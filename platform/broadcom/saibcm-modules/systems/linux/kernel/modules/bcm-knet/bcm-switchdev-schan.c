#include <gmodule.h>
#include <linux-bde.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/if_bridge.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/rtnetlink.h>
#include <linux/netlink.h>
#include <net/switchdev.h>
#include <net/vxlan.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <kcom.h>
#include <bcm-knet.h>
#include "bcm-switchdev.h"
#include "bcm-switchdev-switch.h"
#include "bcm-switchdev-schan.h"


/*****************************************************************************************/
/*                              SCHAN                                                    */
/*****************************************************************************************/

static int
_cmicx_schan_poll_wait(struct net_device *dev, schan_msg_t *msg, int ch)
{
    int rv = 0;
    uint32 schanCtrl;
    int schan_timeout  = 0; 

    while (((schanCtrl = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_CTRL(ch))) &
            SC_CHx_MSG_DONE) == 0) {
        udelay(1000);
        schan_timeout++;
        //300ms 
        if (schan_timeout >= 300 ) { 
        gprintk(" failed after 300 polls\n");
            rv = -ETIME;
            break;
        }                
    }

    //if (rv == 0) {
        //gprintk("  Done in %d polls\n", schan_timeout);
    //}

    //gprintk("  schanCtrl is 0x %x\n", schanCtrl);

    if (schanCtrl & SC_CHx_MSG_NAK) {
        rv = -EFAULT;
        gprintk("  NAK received from SCHAN.\n");
    }

    if (schanCtrl & SC_CHx_MSG_SER_CHECK_FAIL) {
        gprintk("SER Parity Check Error.\n");
        rv = -EFAULT;
    }    


    if (schanCtrl & SC_CHx_MSG_TIMEOUT_TST) {
    gprintk("  Hardware Timeout Error.\n");
        rv = -EFAULT;
    }

    if (schanCtrl & SC_CHx_MSG_SCHAN_ERR) {
        schan_err_t schan_err = {0};
        
        schan_err.word = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_ERR(ch));

        gprintk("  CMIC_SCHAN_ERR. 0x%x\n", schan_err.word);

        if (schan_err.reg.errbit) {
            switch(schan_err.reg.err_code) {
                case 0:
                    gprintk("  block timeout: ERRBIT received in CMIC_SCHAN_ERR.\n");
                    break;
                case 1:
                    gprintk("  allowed block timeout: Continue.\n");
                    break;
                case 2:
                    gprintk("  ECC error.\n");
                    break;
                default:
                    gprintk("Unexpected error.\n");
                    break;
            }
        }

        if (schan_err.reg.nack) {
            gprintk("  NACK received in CMIC_SCHAN_ERR.\n");
        }


        /* dump error data */
        gprintk("  CMIC_SCHAN_ERR data dump: "
                        "err_code=%u, data_len=%u, src_port=%u, dst_port=%u,"
                        "op_code=%u Full reg value=0x%x\n",
                        schan_err.reg.err_code, schan_err.reg.data_len, schan_err.reg.src_port, 
            schan_err.reg.dst_port, schan_err.reg.op_code, schan_err.word);

        bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_CTRL(ch), SC_CHx_MSG_CLR);
        rv = -EFAULT;
    }

    return rv;
}

static void
_cmicx_schan_dump(struct net_device *dev, schan_msg_t *msg, int dwc)
{
    char                buf[128];
    int                 i, j;

    for (i = 0; i < dwc; i += 4) {
        buf[0] = 0;

        for (j = i; j < i + 4 && j < dwc; j++) {
            sprintf(buf + strlen(buf),
                    " DW[%2d]=0x%08x", j, msg->dwords[j]);
        }

       gprintk(" %s\n", buf);
    }
}


//CMICX SCHAN OP
static int
_cmicx_schan_op(struct net_device *dev, schan_msg_t *msg, int dwc_write, int dwc_read, uint32 flags)
{
    int i, rv, val, ch;
    schan_msg_readcmd_t *cmd = (schan_msg_readcmd_t *)msg;
    uint32_t addr = cmd->address;

    //SCHAN_LOCK(unit);

    //TODO - get free channel
    ch = 0;

    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_CTRL(ch));
    //gprintk("_cmicx_schan_op schanCtrl = 0x%x\n", val);

    do {
        rv = 0;

        /* Write raw S-Channel Data: dwc_write words */
        for (i = 0; i < dwc_write; i++) {
            bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(ch, i), msg->dwords[i]);
        }

        /* Tell CMIC to start */
        bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_CTRL(ch), SC_CHx_MSG_START);

        /* Wait for completion using polling method */
        rv = _cmicx_schan_poll_wait(dev, msg, ch);

        if (rv) {
            break;
        }

        //memset(msg, 0, dwc_read);

        /* Read in data from S-Channel buffer space, if any */
        for (i = 0; i < dwc_read; i++) {
            msg->dwords[i] = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(ch, i));
        }

        //_cmicx_schan_dump(dev, msg, dwc_read);

    } while (0);

    //SCHAN_UNLOCK(unit);

    if (rv) {
        gprintk("_cmicx_schan_op operation failed addr 0x%x\n", addr);
        _cmicx_schan_dump(dev, msg, dwc_write);
    }

    return rv;
}


/*
 * Function: _soc_mem_read_schan_msg_send
 *
 * Purpose:  Called within _soc_mem_read.  Construct and send a schan message
 *           holding the read requests, and parse the response.  If an error
 *           happens, try to correct with SER.
 *
 * Returns:  Standard BCM_E_* code
 *
 */
// size = (SOC_MEM_INFO(unit, mem).bytes + 3 )/4
int
_soc_mem_read(struct net_device *dev, uint32 address, int dst_blk,  int size, uint32_t *entry_data)
{
    schan_msg_t schan_msg;
    int opcode, err;
    int rv = 0;
    uint32 allow_intr = 0;
    int data_byte_len; 

    memset(&schan_msg, 0, sizeof(schan_msg_t));

    /* Setup S-Channel command packet */
    data_byte_len = size*sizeof(uint32);

    schan_msg.readcmd.address = address;

    //_soc_mem_read_td_tt_byte_len_update(unit, mem, entry_dw, &data_byte_len);
    //soc_mem_dst_blk_update(unit, copyno, maddr, &dst_blk);

    //setup command header
    schan_msg.header.v4.opcode = READ_MEMORY_CMD_MSG;
    schan_msg.header.v4.dst_blk = dst_blk;
    schan_msg.header.v4.acc_type = 0;
    schan_msg.header.v4.data_byte_len = data_byte_len;
    schan_msg.header.v4.dma = 0;
    schan_msg.header.v4.bank_ignore_mask = 0;

    //if (address == (MMU_CTR_ING_DROP_MEMm+16) ) {
    //   printk("_soc_mem_read addr 0x%08x dst_blk %d 0x%08x \n", address, dst_blk, schan_msg.header.word);
    //}

    rv = _cmicx_schan_op(dev, &schan_msg, 2, 1 + size, allow_intr);
    if (rv) {
        /*
        int all_done = FALSE;

        LOG_WARN(BSL_LS_SOC_SCHAN,
            (BSL_META_U(unit,
                "soc_schan_op: operation failed: %s(%d)\n"), soc_errmsg(rv), rv));

        if (rv == SOC_E_TIMEOUT) {
            _soc_mem_sbus_ring_map_dump(unit);
        }

        _soc_mem_read_ser_correct(unit, flags, mem, copyno, index, entry_data,
                                  &schan_msg, &schan_msg_cpy, resp_word, &rv,
                                  &all_done);
        if (SOC_FAILURE(rv) || all_done) {
            return rv;
        }
        */
       return rv;
    }

    //soc_schan_header_status_get(unit, &schan_msg.header, &opcode, NULL, NULL,
    //                            &err, NULL, NULL);
    opcode = schan_msg.header.v4.opcode;
    err = schan_msg.header.v4.err;
    if (opcode != READ_MEMORY_ACK_MSG || (err != 0 )) {
        /*
        {
            int dwc;

            LOG_ERROR(BSL_LS_SOC_SOCMEM,
                      (BSL_META_U(unit,
                                  "soc_mem_read: "
                                  "Mem(%s) "
                                  "invalid S-Channel reply, expected READ_MEMORY_ACK:, opcode %d\n"),SOC_MEM_NAME(unit,mem), opcode));

            if (soc_feature(unit, soc_feature_two_ingress_pipes)) {
                dwc = 2;
                LOG_ERROR(BSL_LS_SOC_SOCMEM,
                          (BSL_META_U(unit,
                                      "SEND SCHAN MSG:\n")));

                _soc_mem_schan_dump(unit, &schan_msg_cpy, dwc);
            }

            dwc = 1 + entry_dw + resp_word;
            LOG_ERROR(BSL_LS_SOC_SOCMEM,
                      (BSL_META_U(unit,
                                  "RECV SCHAN MSG:\n")));

            _soc_mem_schan_dump(unit, &schan_msg, dwc);

            soc_schan_dump(unit, &schan_msg, 1 + entry_dw + resp_word, BSL_VERBOSE);

            rv = SOC_E_INTERNAL;
            return rv;
        }
       */
       return rv;
    }
    memcpy(entry_data,
           schan_msg.readresp.data,
           size * sizeof (uint32));

    return rv;
}


// size = (SOC_MEM_INFO(unit, mem).bytes + 3 )/4
// 
int
_soc_mem_write(struct net_device *dev, uint32 address, int dst_blk, int size, uint32_t *entry_data)
{
    schan_msg_t schan_msg;
    int opcode, err;
    int rv = 0;
    uint32 allow_intr = 0;
    int data_byte_len; 

    memset(&schan_msg, 0, sizeof(schan_msg_t));

    /* Setup S-Channel command packet */
    data_byte_len = size * sizeof(uint32);

    memcpy(schan_msg.writecmd.data, entry_data, size * sizeof(uint32));
    schan_msg.writecmd.address = address;

    //setup command header
    schan_msg.header.v4.opcode = WRITE_MEMORY_CMD_MSG;
    schan_msg.header.v4.dst_blk = dst_blk;
    schan_msg.header.v4.acc_type = 0;
    schan_msg.header.v4.data_byte_len = data_byte_len;
    schan_msg.header.v4.dma = 0;
    schan_msg.header.v4.bank_ignore_mask = 0;

    //if (address == PMQPORT_WC_UCMEM_DATAm ) {
    //printk("_soc_mem_write addr 0x%08x dst_blk %d 0x%08x \n", address, dst_blk, schan_msg.header.word);
    //}

    rv = _cmicx_schan_op(dev, &schan_msg, 2 + size, 0, allow_intr);
    if (rv) {
        /*
        int all_done = FALSE;

        LOG_WARN(BSL_LS_SOC_SCHAN,
            (BSL_META_U(unit,
                "soc_schan_op: operation failed: %s(%d)\n"), soc_errmsg(rv), rv));

        if (rv == SOC_E_TIMEOUT) {
            _soc_mem_sbus_ring_map_dump(unit);
        }

        _soc_mem_read_ser_correct(unit, flags, mem, copyno, index, entry_data,
                                  &schan_msg, &schan_msg_cpy, resp_word, &rv,
                                  &all_done);
        if (SOC_FAILURE(rv) || all_done) {
            return rv;
        }
        */
       return rv;
    }

    //soc_schan_header_status_get(unit, &schan_msg.header, &opcode, NULL, NULL,
    //                            &err, NULL, NULL);
    opcode = schan_msg.header.v4.opcode;
    err = schan_msg.header.v4.err;
    if (opcode != READ_MEMORY_ACK_MSG || (err != 0 )) {
        /*
        {
            int dwc;

            LOG_ERROR(BSL_LS_SOC_SOCMEM,
                      (BSL_META_U(unit,
                                  "soc_mem_read: "
                                  "Mem(%s) "
                                  "invalid S-Channel reply, expected READ_MEMORY_ACK:, opcode %d\n"),SOC_MEM_NAME(unit,mem), opcode));

            if (soc_feature(unit, soc_feature_two_ingress_pipes)) {
                dwc = 2;
                LOG_ERROR(BSL_LS_SOC_SOCMEM,
                          (BSL_META_U(unit,
                                      "SEND SCHAN MSG:\n")));

                _soc_mem_schan_dump(unit, &schan_msg_cpy, dwc);
            }

            dwc = 1 + entry_dw + resp_word;
            LOG_ERROR(BSL_LS_SOC_SOCMEM,
                      (BSL_META_U(unit,
                                  "RECV SCHAN MSG:\n")));

            _soc_mem_schan_dump(unit, &schan_msg, dwc);

            soc_schan_dump(unit, &schan_msg, 1 + entry_dw + resp_word, BSL_VERBOSE);

            rv = SOC_E_INTERNAL;
            return rv;
        }
       */
    }

    return rv;
}

/*
 * entry      : mem to set
 * entry_bytes: total bytes of entry
 * bp         : start bit
 * len        : len of bit
 * val        : value to set
 * flags      : 
 */
void
_mem_field_set(uint32_t *entry,
               int entry_bytes,
               int bp, 
               int field_len,
               uint32_t *val,
               uint32_t flags)
{
    uint32_t    mask;
    int         i, wp, len;

    if (flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = field_len; len > 0; len -= 32) {
            if (bp) {
                if (len < 32) {
                    mask = (1 << len) - 1;
                } else {
                    mask = -1;
                }

                entry[FIX_MEM_ORDER_E(wp, entry_bytes, 0)] &= ~(mask << bp);
                entry[FIX_MEM_ORDER_E(wp++, entry_bytes, 0)] |= val[i] << bp;
                if (len > (32 - bp)) {
                    entry[FIX_MEM_ORDER_E(wp, entry_bytes, 0)] &= ~(mask >> (32 - bp));
                    entry[FIX_MEM_ORDER_E(wp, entry_bytes, 0)] |=
                    val[i] >> (32 - bp) & (((uint32)1 << bp) - 1);
                }
            } else {
                if (len < 32) {
                    mask = (1 << len) - 1;
                    entry[FIX_MEM_ORDER_E(wp, entry_bytes, 0)] &= ~mask;
                    entry[FIX_MEM_ORDER_E(wp++, entry_bytes, 0)] |= val[i] << bp;
                } else {
                    entry[FIX_MEM_ORDER_E(wp++, entry_bytes, 0)] = val[i];
                }
            }

            i++;
        }
    } else {                           /* Big endian: swap bits */
        len = field_len;

        while (len > 0) {
            len--;
            entry[FIX_MEM_ORDER_E(bp / 32, entry_bytes, 0)] &= ~(1 << (bp & (32-1)));
            entry[FIX_MEM_ORDER_E(bp / 32, entry_bytes, 0)] |=
                (val[len / 32] >> (len & (32-1)) & 1) << (bp & (32-1));
            bp++;
        }
    }
}

/*
 * entry      : mem to get
 * entry_bytes: total bytes of entry
 * bp         : start bit
 * len        : len of bit
 * val        : value to get
 * flags      : 
 */
void
_mem_field_get(uint32_t *entry,
                   int entry_bytes,
                   int bp, 
                   int field_len,
                   uint32_t *val,
                   uint32_t flags)                      
{
    int  i, wp, len;

    if (field_len == 1) {     /* special case single bits */
        wp = bp / 32;
        bp = bp & (32 - 1);
        if (entry[FIX_MEM_ORDER_E(wp, entry_bytes, 0)] & (1<<bp)) {
            val[0] = 1;
        } else {
            val[0] = 0;
        }
        return;
    }

    if (flags & SOCF_LE) {
        wp = bp / 32;
        bp = bp & (32 - 1);
        i = 0;

        for (len = field_len; len > 0; len -= 32) {
            if (bp) {
                val[i] =
                entry[FIX_MEM_ORDER_E(wp++, entry_bytes, 0)] >> bp &
                    ((1 << (32 - bp)) - 1);
                if ( len > (32 - bp) ) {
                    val[i] |= entry[FIX_MEM_ORDER_E(wp, entry_bytes, 0)] << (32 - bp);
                }
            } else {
                val[i] = entry[FIX_MEM_ORDER_E(wp++, entry_bytes, 0)];
            }

            if (len < 32) {
                val[i] &= ((1 << len) - 1);
            }
            i++;
        }
    } else {
        i = (field_len - 1) / 32;
        len = field_len;
        while (len > 0) {
            val[i] = 0;
            do {
                val[i] =
                    (val[i] << 1) |
                    ((entry[FIX_MEM_ORDER_E(bp / 32, entry_bytes, 0)] >> (bp & (32 - 1))) & 1);
                len--;
                bp++;
            } while (len & (32 - 1));
            i--;
        }
    }

    return;
}

/*****************************************************************************************/
/*                             SCHAN Reg Read/Write                                      */
/*****************************************************************************************/
int
_schan_reg32_read(struct net_device *dev, int dst_blk, uint32_t address, uint32_t *data, uint8_t acc_type )
{
    schan_msg_t schan_msg;
    int opcode, err;
    int rv = 0;
    uint32 allow_intr = 0;
    int data_byte_len; 

    memset(&schan_msg, 0, sizeof(schan_msg_t));

    /* Setup S-Channel command packet */
    data_byte_len = 4;
         
    schan_msg.readcmd.address = address;

    //setup command header
    schan_msg.header.v4.opcode = READ_REGISTER_CMD_MSG;
    schan_msg.header.v4.dst_blk = dst_blk;
    schan_msg.header.v4.acc_type = acc_type;
    schan_msg.header.v4.data_byte_len = data_byte_len;
    schan_msg.header.v4.dma = 0;
    schan_msg.header.v4.bank_ignore_mask = 0;

    rv = _cmicx_schan_op(dev, &schan_msg, 2, 2, allow_intr);
    if (rv) {
       return rv;
    }

    /* Check result */
    opcode = schan_msg.header.v4.opcode;
    err = schan_msg.header.v4.err;
    if (!rv &&  (opcode != READ_REGISTER_ACK_MSG || err != 0)) {
        gprintk("_cmicx_schan_op: operation failed: %s(%d)\n", 
                (opcode != READ_REGISTER_ACK_MSG)?"invalid S-Channel reply, expected READ_REG_ACK:\n":"OTHER", rv);

      return rv;
    }

    *data = schan_msg.readresp.data[0];

    return rv;
}

int
_reg32_read(struct net_device *dev, int dst_blk, uint32_t address, uint32_t *data)
{
    return _schan_reg32_read(dev, dst_blk, address, data, 0);
}


/*
 * Write an internal SOC register through S-Channel messaging buffer.
 */
int
_schan_reg32_write(struct net_device *dev, int dst_blk, uint32_t address, uint32_t data, uint8_t acc_type)
{
    schan_msg_t schan_msg;
    int rv = 0;
    uint32 allow_intr = 0;
    int data_byte_len; 

    memset(&schan_msg, 0, sizeof(schan_msg_t));

    /* Setup S-Channel command packet */
    data_byte_len = 4;
            
    //setup command header
    schan_msg.header.v4.opcode = WRITE_REGISTER_CMD_MSG; 
    schan_msg.header.v4.dst_blk = dst_blk;
    schan_msg.header.v4.acc_type = acc_type;
    schan_msg.header.v4.data_byte_len = data_byte_len;
    schan_msg.header.v4.dma = 0;
    schan_msg.header.v4.bank_ignore_mask = 0;

    schan_msg.writecmd.address = address;
    schan_msg.writecmd.data[0] = data;

    /* Write header word + address + data DWORD */
    /* Note: The hardware does not send WRITE_REGISTER_ACK_MSG. */
    rv = _cmicx_schan_op(dev, &schan_msg, 3, 0, allow_intr);

    return rv;
}

int
_reg32_write(struct net_device *dev, int dst_blk, uint32_t address, uint32_t data)
{
    return _schan_reg32_write(dev, dst_blk, address, data, 0);
}


int
_schan_reg64_read(struct net_device *dev, int dst_blk, uint32_t address, uint64_t *data, uint8_t acc_type)
{
    schan_msg_t schan_msg;
    int opcode, err;
    int rv = 0;
    uint32 allow_intr = 0;
    int data_byte_len; 

    memset(&schan_msg, 0, sizeof(schan_msg_t));

    /* Setup S-Channel command packet */
    data_byte_len = 8;
         
    schan_msg.readcmd.address = address;

    //setup command header
    schan_msg.header.v4.opcode = READ_REGISTER_CMD_MSG;
    schan_msg.header.v4.dst_blk = dst_blk;
    schan_msg.header.v4.acc_type = acc_type;
    schan_msg.header.v4.data_byte_len = data_byte_len;
    schan_msg.header.v4.dma = 0;
    schan_msg.header.v4.bank_ignore_mask = 0;

    rv = _cmicx_schan_op(dev, &schan_msg, 2, 3, allow_intr);
    if (rv) {
       return rv;
    }

    /* Check result */
    opcode = schan_msg.header.v4.opcode;
    err = schan_msg.header.v4.err;
    if (!rv &&  (opcode != READ_REGISTER_ACK_MSG || err != 0)) {
        gprintk("_cmicx_schan_op: operation failed: %s(%d)\n", 
                (opcode != READ_REGISTER_ACK_MSG)?"invalid S-Channel reply, expected READ_REG_ACK:\n":"OTHER", rv);

      return rv;
    }

    COMPILER_64_SET(*data, 
                    schan_msg.readresp.data[1], 
                    schan_msg.readresp.data[0]);

    return rv;
}

int _reg64_read(struct net_device *dev, int dst_blk, uint32_t address, uint64_t *data)
{
    return _schan_reg64_read(dev, dst_blk, address, data, 0);
}
/*
 * Write an internal SOC register through S-Channel messaging buffer.
 */
int
_schan_reg64_write(struct net_device *dev, int dst_blk, uint32_t address, uint64_t data, uint8_t acc_type)
{
    schan_msg_t schan_msg;
    int rv = 0;
    uint32 allow_intr = 0;
    int data_byte_len; 

    memset(&schan_msg, 0, sizeof(schan_msg_t));

    /* Setup S-Channel command packet */
    data_byte_len = 8;
            
    //setup command header
    schan_msg.header.v4.opcode = WRITE_REGISTER_CMD_MSG; 
    schan_msg.header.v4.dst_blk = dst_blk;
    schan_msg.header.v4.acc_type = acc_type;
    schan_msg.header.v4.data_byte_len = data_byte_len;
    schan_msg.header.v4.dma = 0;
    schan_msg.header.v4.bank_ignore_mask = 0;

    schan_msg.writecmd.address = address;
    schan_msg.writecmd.data[0] = COMPILER_64_LO(data);
    schan_msg.writecmd.data[1] = COMPILER_64_HI(data);

    /* Write header word + address + data DWORD */
    /* Note: The hardware does not send WRITE_REGISTER_ACK_MSG. */
    rv = _cmicx_schan_op(dev, &schan_msg, 4, 0, allow_intr);

    return rv;
}

int
_reg64_write(struct net_device *dev, int dst_blk, uint32_t address, uint64_t data)
{
    return _schan_reg64_write(dev, dst_blk, address, data, 0);
}
