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

#include <kcom.h>
#include <bcm-knet.h>
#include "bcm-switchdev.h"

static ibde_t *kernel_bde = NULL;

/*****************************************************************************************/
/*                              SOC                                                      */
/*****************************************************************************************/
/* Environment switch */
#define _PCID_TEST 0

/* Local defines */
#define BYTES_PER_UINT32    (sizeof(uint32))

char *_shr_errmsg[] = _SHR_ERRMSG_INIT;

/*****************************************************************************************/
/*                            N3248TE hardware&ports info                                */
/*****************************************************************************************/
static const struct {
    int port;
    int phy_port;
    int pipe;
    int idb_port;
    int mmu_port;
    int port_type;
} hx5_anc_ports[] = {
    { 0,   0,   0, 70, 70, BCMSW_PORT_TYPE_CMIC      },  /* cpu port */
    { 70,  77,  0, 71, 71, BCMSW_PORT_TYPE_LBPORT    },  /* loopback port */
    { 71,  78,  0, 69, 69, BCMSW_PORT_TYPE_BROADSCAN },  /* FAE port */
};


typedef struct port_info_s {
    int port;
    int phy_port;
    int bandwidth;
    int ext_phy_addr;
    int primary_and_offset;
    int port_type;
    char name[KCOM_NETIF_NAME_MAX];
    int lanes[4];
} port_config_t;

static const port_config_t n3248te_ports[] = {
    { 1,   1,  1, 0x0, 0x0100, BCMSW_PORT_TYPE_GXPORT, "Ethernet0",  {1, -1, -1, -1}}, 
    { 2,   2,  1, 0x1, 0x0101, BCMSW_PORT_TYPE_GXPORT, "Ethernet1",  {2, -1, -1, -1}}, 
    { 3,   3,  1, 0x2, 0x0102, BCMSW_PORT_TYPE_GXPORT, "Ethernet2",  {3, -1, -1, -1}}, 
    { 4,   4,  1, 0x3, 0x0103, BCMSW_PORT_TYPE_GXPORT, "Ethernet3",  {4, -1, -1, -1}}, 
    { 5,   5,  1, 0x4, 0x0104, BCMSW_PORT_TYPE_GXPORT, "Ethernet4",  {5, -1, -1, -1}}, 
    { 6,   6,  1, 0x5, 0x0105, BCMSW_PORT_TYPE_GXPORT, "Ethernet5",  {6, -1, -1, -1}}, 
    { 7,   7,  1, 0x6, 0x0106, BCMSW_PORT_TYPE_GXPORT, "Ethernet6",  {7, -1, -1, -1}}, 
    { 8,   8,  1, 0x7, 0x0107, BCMSW_PORT_TYPE_GXPORT, "Ethernet7",  {8, -1, -1, -1}}, 
    { 9,   0,  1, 0x9, 0x0900, BCMSW_PORT_TYPE_GXPORT, "Ethernet8",  {9, -1, -1, -1}}, 
    { 10,  10, 1, 0xa, 0x0901, BCMSW_PORT_TYPE_GXPORT, "Ethernet9",  {10, -1, -1, -1}}, 
    { 11,  11, 1, 0xb, 0x0902, BCMSW_PORT_TYPE_GXPORT, "Ethernet10", {11, -1, -1, -1}}, 
    { 12,  12, 1, 0xc, 0x0903, BCMSW_PORT_TYPE_GXPORT, "Ethernet11", {12, -1, -1, -1}}, 
    { 13,  13, 1, 0xd, 0x0904, BCMSW_PORT_TYPE_GXPORT, "Ethernet12", {13, -1, -1, -1}},         
    { 14,  14, 1, 0xe, 0x0905, BCMSW_PORT_TYPE_GXPORT, "Ethernet13", {14, -1, -1, -1}}, 
    { 15,  15, 1, 0xf, 0x0906, BCMSW_PORT_TYPE_GXPORT, "Ethernet14", {15, -1, -1, -1}}, 
    { 16,  16, 1,  0x10, 0x0907, BCMSW_PORT_TYPE_GXPORT, "Ethernet15", {16, -1, -1, -1}}, 
    { 17,  17, 1,  0x12, 0x1200, BCMSW_PORT_TYPE_GXPORT, "Ethernet16", {17, -1, -1, -1}}, 
    { 18,  18, 1,  0x13, 0x1201, BCMSW_PORT_TYPE_GXPORT, "Ethernet17", {18, -1, -1, -1}}, 
    { 19,  19, 1,  0x14, 0x1202, BCMSW_PORT_TYPE_GXPORT, "Ethernet18", {19, -1, -1, -1}}, 
    { 20,  20, 1,  0x15, 0x1203, BCMSW_PORT_TYPE_GXPORT, "Ethernet19", {20, -1, -1, -1}}, 
    { 21,  21, 1,  0x16, 0x1204, BCMSW_PORT_TYPE_GXPORT, "Ethernet20", {21, -1, -1, -1}}, 
    { 22,  22, 1,  0x17, 0x1205, BCMSW_PORT_TYPE_GXPORT, "Ethernet21", {22, -1, -1, -1}}, 
    { 23,  23, 1,  0x18, 0x1206, BCMSW_PORT_TYPE_GXPORT, "Ethernet22", {23, -1, -1, -1}}, 
    { 24,  24, 1,  0x19, 0x1207, BCMSW_PORT_TYPE_GXPORT, "Ethernet23", {24, -1, -1, -1}}, 
    { 25,  25, 1,  0x20, 0x2000, BCMSW_PORT_TYPE_GXPORT, "Ethernet24", {25, -1, -1, -1}}, 
    { 26,  26, 1,  0x21, 0x2001, BCMSW_PORT_TYPE_GXPORT, "Ethernet25", {26, -1, -1, -1}}, 
    { 27,  27, 1,  0x22, 0x2002, BCMSW_PORT_TYPE_GXPORT, "Ethernet26", {27, -1, -1, -1}}, 
    { 28,  28, 1,  0x23, 0x2003, BCMSW_PORT_TYPE_GXPORT, "Ethernet27", {28, -1, -1, -1}}, 
    { 29,  29, 1,  0x24, 0x2004, BCMSW_PORT_TYPE_GXPORT, "Ethernet28", {29, -1, -1, -1}}, 
    { 30,  30, 1,  0x25, 0x2005, BCMSW_PORT_TYPE_GXPORT, "Ethernet29", {30, -1, -1, -1}}, 
    { 31,  31, 1,  0x26, 0x2006, BCMSW_PORT_TYPE_GXPORT, "Ethernet30", {31, -1, -1, -1}}, 
    { 32,  32, 1,  0x27, 0x2007, BCMSW_PORT_TYPE_GXPORT, "Ethernet31", {32, -1, -1, -1}}, 
    { 33,  33, 1,  0x29, 0x2900, BCMSW_PORT_TYPE_GXPORT, "Ethernet32", {33, -1, -1, -1}},        
    { 34,  34, 1,  0x2a, 0x2901, BCMSW_PORT_TYPE_GXPORT, "Ethernet33", {34, -1, -1, -1}}, 
    { 35,  35, 1,  0x2b, 0x2902, BCMSW_PORT_TYPE_GXPORT, "Ethernet34", {35, -1, -1, -1}}, 
    { 36,  36, 1,  0x2c, 0x2903, BCMSW_PORT_TYPE_GXPORT, "Ethernet35", {36, -1, -1, -1}}, 
    { 37,  37, 1,  0x2d, 0x2904, BCMSW_PORT_TYPE_GXPORT, "Ethernet36", {37, -1, -1, -1}}, 
    { 38,  38, 1,  0x2e, 0x2905, BCMSW_PORT_TYPE_GXPORT, "Ethernet37", {38, -1, -1, -1}}, 
    { 39,  39, 1,  0x2f, 0x2906, BCMSW_PORT_TYPE_GXPORT, "Ethernet38", {39, -1, -1, -1}}, 
    { 40,  40, 1,  0x30, 0x2907, BCMSW_PORT_TYPE_GXPORT, "Ethernet39", {40, -1, -1, -1}}, 
    { 41,  41, 1,  0x32, 0x3200, BCMSW_PORT_TYPE_GXPORT, "Ethernet40", {41, -1, -1, -1}}, 
    { 42,  42, 1,  0x33, 0x3201, BCMSW_PORT_TYPE_GXPORT, "Ethernet41", {42, -1, -1, -1}}, 
    { 43,  43, 1,  0x34, 0x3202, BCMSW_PORT_TYPE_GXPORT, "Ethernet42", {43, -1, -1, -1}}, 
    { 44,  44, 1,  0x35, 0x3203, BCMSW_PORT_TYPE_GXPORT, "Ethernet43", {44, -1, -1, -1}}, 
    { 45,  45, 1,  0x36, 0x3204, BCMSW_PORT_TYPE_GXPORT, "Ethernet44", {45, -1, -1, -1}}, 
    { 46,  46, 1,  0x37, 0x3205, BCMSW_PORT_TYPE_GXPORT, "Ethernet45", {46, -1, -1, -1}}, 
    { 47,  47, 1,  0x38, 0x3206, BCMSW_PORT_TYPE_GXPORT, "Ethernet46", {47, -1, -1, -1}}, 
    { 48,  48, 1,  0x39, 0x3207, BCMSW_PORT_TYPE_GXPORT, "Ethernet47", {48, -1, -1, -1}}, 
    { 49,  64, 10, 0x40, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet48", {64, -1, -1, -1}}, 
    { 50,  63, 10, 0x41, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet49", {63, -1, -1, -1}}, 
    { 51,  62, 10, 0x42, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet50", {62, -1, -1, -1}}, 
    { 52,  61, 10, 0x43, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet51", {61, -1, -1, -1}}, 
    { 53,  69, 100,  -1, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet52", {69, 70, 71, 72}}, 
    { 57,  73, 100,  -1, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet56", {73, 74, 75, 76}},   
    { -1,  -1,  -1,  -1, -1,                         -1, "\0",         {-1, -1, -1, -1}}                                      
};

/*****************************************************************************************/
/*                              UTILS                                                    */
/*****************************************************************************************/
/* reverse the bits in each byte of a 32 bit long */
static uint32
_bit_rev_by_byte_word32(uint32 n)
{
    n = (((n & 0xaaaaaaaa) >> 1) | ((n & 0x55555555) << 1));
    n = (((n & 0xcccccccc) >> 2) | ((n & 0x33333333) << 2));
    n = (((n & 0xf0f0f0f0) >> 4) | ((n & 0x0f0f0f0f) << 4));
    return n;
}

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
        gprintk("_cmicx_schan_op operation failed\n");
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
static int
_soc_mem_read(struct net_device *dev, uint32 address, int dst_blk,  int size, void *entry_data)
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
static int
_soc_mem_write(struct net_device *dev, uint32 address, int dst_blk, int size, void *entry_data)
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
    memcpy(entry_data,
           schan_msg.readresp.data,
           size * sizeof (uint32));

    return rv;
}


/*****************************************************************************************/
/*                             SCHAN Reg Read/Write                                      */
/*****************************************************************************************/
static int
_reg32_read(struct net_device *dev, int dst_blk, uint32_t address, uint32_t *data)
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
    schan_msg.header.v4.acc_type = 0;
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


/*
 * Write an internal SOC register through S-Channel messaging buffer.
 */
static int
_reg32_write(struct net_device *dev, int dst_blk, uint32_t address, uint32_t data)
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
    schan_msg.header.v4.acc_type = 0;
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

static int
_reg64_read(struct net_device *dev, int dst_blk, uint32_t address, uint64_t *data)
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
    schan_msg.header.v4.acc_type = 0;
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


/*
 * Write an internal SOC register through S-Channel messaging buffer.
 */
static int
_reg64_write(struct net_device *dev, int dst_blk, uint32_t address, uint64_t data)
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
    schan_msg.header.v4.acc_type = 0;
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

/*****************************************************************************************/
/*                            BCM56371 CANCUN                                            */
/*****************************************************************************************/

/* ZLIB CRC-32 table*/
static uint32 soc_crc32_tab[] = {
    0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
    0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
    0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
    0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
    0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
    0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
    0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
    0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
    0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
    0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
    0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
    0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
    0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
    0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
    0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
    0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
    0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
    0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
    0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
    0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
    0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
    0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
    0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
    0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
    0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
    0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
    0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
    0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
    0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
    0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
    0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
    0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
    0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
    0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
    0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
    0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
    0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
    0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
    0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
    0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
    0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
    0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
    0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
    0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
    0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
    0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
    0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
    0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
    0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
    0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
    0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
    0x2d02ef8dL
};


/* ZLIB CRC-32 routine */
uint32 soc_cancun_crc32(uint32 crc, uint8 *buf, int len_bytes) {

    #define DO1(buf) crc = soc_crc32_tab[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8);
    #define DO2(buf)  DO1(buf); DO1(buf);
    #define DO4(buf)  DO2(buf); DO2(buf);
    #define DO8(buf)  DO4(buf); DO4(buf);
    
        if (buf == NULL) return 0L;
        crc = crc ^ 0xffffffffL;
    
        while (len_bytes >= 8)
        {
            DO8(buf);
            len_bytes -= 8;
        }
    
        if (len_bytes) do {
            DO1(buf);
        } while (--len_bytes);
    
        return crc ^ 0xffffffffL;
}
    
void soc_cancun_buf_swap32(uint8 *buf, long buf_size)
{
    long i;
    uint32 *fword = (uint32 *)buf;
    for (i = 0; i < ((buf_size + 3) / 4); i++) {
        *fword = soc_letohl(*fword);
        fword ++;
    }
    return ;
}


static int _soc_cancun_memcpy_letohl(uint32 *des, uint32 *src, uint32 word_len) {
    uint32 i;

    if (des == NULL || src == NULL) {
        return SOC_E_INTERNAL;
    }

    for (i = 0; i < word_len; i++) {
        *des++ = *src++;
    }

    return SOC_E_NONE;
}


int soc_cancun_file_info_get(struct bcmsw_switch *sw, soc_cancun_file_t* ccf, char *filename,
        uint8 *buf, long buf_bytes) {

    soc_cancun_t *cc = sw->soc_cancun_info;
    soc_cancun_file_header_t *ccfh = (soc_cancun_file_header_t *) buf;
    //uint16 dev_id;
    //uint8 rev_id;
    uint32 crc, *file_crc;
    uint8 *cur_buf;
    long cur_buf_size;

    if (ccf == NULL) {
        return SOC_E_INTERNAL;
    }

    crc = soc_cancun_crc32(0, buf, buf_bytes - 4);

    /* convert file header into correct endianness */
    cur_buf = buf;
    cur_buf_size = SOC_CANCUN_FILE_HEADER_OFFSET;
    soc_cancun_buf_swap32(cur_buf, cur_buf_size);
    memcpy(&ccf->header, ccfh, sizeof(soc_cancun_file_header_t));

    cur_buf += cur_buf_size;
    cur_buf_size = buf_bytes - cur_buf_size;
    /* check if CEH or CFH, don't swap rest of file */
    if ((ccfh->file_type != SOC_CANCUN_FILE_ID_CEH) &&
        (ccfh->file_type != SOC_CANCUN_FILE_ID_CFH)) { 
        soc_cancun_buf_swap32(cur_buf, cur_buf_size);
    } else {
        /* swap the checksum */
        soc_cancun_buf_swap32(buf + buf_bytes - 4, 4);
    }

    if (filename) {
        strncpy(ccf->filename, filename, strlen(filename)+1);
    }

    ccf->valid = 0;
    /* 1. File identifier */
    if(ccfh->file_identifier != SOC_CANCUN_FILE_ID) {
        if (filename) {
            printk("ERROR: %s is not a CANCUN file\n", filename);
        } else {
            printk("ERROR: Not a CANCUN file: 0x%08x. Abort\n", ccfh->file_identifier);
        }
        return SOC_E_INTERNAL;
    }
    /* 2. File type */
    switch(ccfh->file_type) {
        case SOC_CANCUN_FILE_ID_CIH:
            ccf->type = CANCUN_SOC_FILE_TYPE_CIH;
            break;
        case SOC_CANCUN_FILE_ID_CMH:
            ccf->type = CANCUN_SOC_FILE_TYPE_CMH;
            break;
        case SOC_CANCUN_FILE_ID_CCH:
            ccf->type = CANCUN_SOC_FILE_TYPE_CCH;
            break;
        case SOC_CANCUN_FILE_ID_CFH:
            ccf->type = CANCUN_SOC_FILE_TYPE_CFH;
            break;
        case SOC_CANCUN_FILE_ID_CEH:
            ccf->type = CANCUN_SOC_FILE_TYPE_CEH;
            break;

        default:
            printk("ERROR: Invalid file type. Abort\n");
            return SOC_E_INTERNAL;
    }

    /* CANCUN file validation */
    if((cc->flags & SOC_CANCUN_FLAG_SKIP_VALIDITY) == 0) {
        /* 3. File length */
        if(ccfh->file_length != BYTES2WORDS(buf_bytes)) {
            printk("ERROR: File length mismatch. Abort\n");
            return SOC_E_INTERNAL;
        }
        /* 4. HW version */
        //soc_cm_get_id(unit, &dev_id, &rev_id);
        //if(!soc_cancun_chip_rev_validate(ccfh->chip_rev_id, dev_id, rev_id)) {
        //    printk("ERROR: HW version mismatch. Abort\n");
        //    return SOC_E_INTERNAL;
        //}
        /* 5. CRC */
        file_crc = (uint32 *) (buf + buf_bytes - 4);
        if (crc != *file_crc) {
            printk("ERROR: CRC check fails crc 0x%08x, file_crc 0x%08x. Abort\n", crc, *file_crc);
            return SOC_E_INTERNAL;
        }
    }

    ccf->valid = 1;
    ccf->format = CANCUN_SOC_FILE_FORMAT_PACK;
    ccf->status = CANCUN_SOC_FILE_LOAD_NONE;

    return SOC_E_NONE;
}

static int _soc_cancun_file_pio_load(struct bcmsw_switch *sw, uint8* buf, int buf_words) 
{
    long schan_msg_len_words, msg_num, i;
    schan_msg_t* msg;
    int index;

    schan_msg_len_words = CMIC_SCHAN_WORDS;
    msg_num = buf_words / schan_msg_len_words;
    msg = (schan_msg_t*) buf;

    for(i = 0; i < msg_num; i++) {
#if _PCID_TEST
        {
            int j;
            for(j = 0; j< schan_msg_len_words; j++) {
                msg->dwords[j] = soc_htonl(msg->dwords[j]);
                cli_out("0x%x ", msg->dwords[j]);
            }
            cli_out("\n");
            index = 0;
        }
#else
        index = _cmicx_schan_op(sw->dev, msg, schan_msg_len_words,
                             schan_msg_len_words, 0);
#endif
        if (index < 0) {
            printk("S-Channel operation failed: %s\n", soc_errmsg(index));
            return SOC_E_FAIL;
        }
        msg++;
    }

    return SOC_E_NONE;
}



static int _soc_cancun_cih_tcam_write(struct bcmsw_switch *sw, uint8 *buf) {
#define SOC_SBUS_V4_BLOCK_ID_BIT_OFFSET     (19)
#define SOC_SBUS_V4_BLOCK_ID_BIT_MASK       (0x7F)
    //uint32 entry[SOC_MAX_MEM_WORDS];
    //soc_mem_t mem;
    //uint32 index;
    //uint32 offset;
    //uint32 block = -1;
    //uint32 len;
    //uint32 *p = (uint32*) buf;

    if(!buf) {
        return SOC_E_INTERNAL;
    }

//soc_feature_xy_tcam disabled for BCM56370
#if 0     
    if (soc_feature(unit, soc_feature_xy_tcam)) {
        if (soc_feature(unit, soc_feature_sbus_format_v4)) {
            block = (*(p + SOC_CANCUN_BLOB_OPCODE_OFFSET) >>
                     SOC_SBUS_V4_BLOCK_ID_BIT_OFFSET)
                    & SOC_SBUS_V4_BLOCK_ID_BIT_MASK;
        } else {
            return SOC_E_UNAVAIL;
        }

        mem = soc_addr_to_mem_extended(unit, block, -1, *p);
        if(mem == INVALIDm) {
            LOG_ERROR(BSL_LS_SOC_CANCUN, (BSL_META_U(unit,
                "can't find TCAM memory for addr = 0x%x\n"), *p));
            return SOC_E_INTERNAL;
        }

        index = SOC_MEM_INFO(unit, mem).index_min +
                ((*p - SOC_MEM_INFO(unit, mem).base) /
                  SOC_MEM_INFO(unit, mem).gran);
        if(index > SOC_MEM_INFO(unit, mem).index_max) {
            LOG_ERROR(BSL_LS_SOC_CANCUN, (BSL_META_U(unit,
                "ERROR: index = %d exceeds max of %s (%d)\n"), index,
                    SOC_MEM_NAME(unit, mem),
                    SOC_MEM_INFO(unit, mem).index_max));
            return SOC_E_INTERNAL;
        }

        offset = *p - (SOC_MEM_INFO(unit, mem).base +
                       (index * SOC_MEM_INFO(unit, mem).gran));

        len = *(p + SOC_CANCUN_BLOB_LEN_OFFSET);

        SOC_IF_ERROR_RETURN(
            soc_mem_read(unit, mem, MEM_BLOCK_ANY, index, &entry));

        _soc_cancun_memcpy_letohl((entry + offset), (p + SOC_CANCUN_BLOB_DATA_OFFSET), len);

        SOC_IF_ERROR_RETURN(
            soc_mem_write(unit, mem, MEM_BLOCK_ANY, index, &entry));

        return SOC_E_NONE;

    } else
#endif
    {
        return SOC_E_UNAVAIL;
    }
}

static int _soc_cancun_cih_mem_load(struct bcmsw_switch *sw, uint8 *buf) {
    uint32 entry[SOC_MAX_MEM_WORDS];
    uint32 addr;
    soc_mem_t mem;
    uint32 index;
    uint32 len;
    uint32 *p = (uint32*)buf;

    addr = *p;
    mem = addr >> SOC_CANCUN_BLOB_ADDR_MEM_ID_SHIFT;
    index = addr & SOC_CANCUN_BLOB_ADDR_MEM_IDX_MASK;

    len = *(p + SOC_CANCUN_BLOB_LEN_OFFSET);
    memset(entry, 0, sizeof(entry));
    memcpy(entry, (p + SOC_CANCUN_BLOB_DATA_OFFSET), len*4);

    //TODO
    //return _soc_mem_write(sw->dev, mem, index, &entry);
    return 0;
}

static int _soc_cancun_cih_pio_load(struct bcmsw_switch *sw, uint8* buf, int length,
                                    uint32 flags) 
{
    schan_msg_t msg;
    int i, index;
    uint32 *p = (uint32 *) buf;
    int rv;

    memset(&msg, 0, sizeof(schan_msg_t));

    if (flags & SOC_CANCUN_BLOB_FLAG_MEM_ID_PRESENT) {
        return _soc_cancun_cih_mem_load(sw, buf);
    }

    /* Special case of TCAM memory loading */
    if (flags & SOC_CANCUN_BLOB_FLAG_TCAM) {
        rv = _soc_cancun_cih_tcam_write(sw, buf);
        return rv;
    }

    /* Copy destination address to schan msg structure */
    msg.dwords[1] = *p++;

    /* Copy opcode to schan msg structure */
    msg.dwords[0] = *p++;

    /* Copy data to schan msg structure */
    p += 2; /* skip length and flag */
    for(i = 0; i < length ; i++) {
        msg.dwords[2+i] = *p++;
    }

    for(i = 0; i < length + 2; i++) {
        printk("0x%x ", msg.dwords[i]);
    }
    printk("\n");

#if _PCID_TEST
    {
        int j;
        for(j = 0; j < length+2; j++) {
            msg.dwords[j] = soc_htonl(msg.dwords[j]);
            cli_out("0x%x ", msg.dwords[j]);
        }
        cli_out("\n");
        index = 0;
    }
#else
    index = _cmicx_schan_op(sw->dev, &msg, length+2, length+2, 0);
#endif

    if (index < 0) {
        printk("S-Channel operation failed: %s\n", soc_errmsg(index));
        return SOC_E_FAIL;
    }

    return SOC_E_NONE;

}

static int _soc_cancun_cih_load(struct bcmsw_switch *sw, uint8* buf, int num_data_blobs) {
    int i;
    uint32 length, flags;

    for (i = 0; i < num_data_blobs; i++) {
        /* Length and flags fields are offset by 8 and 12 bytes from start of
         * each data blob */
        length = *(uint32 *)(buf + SOC_CANCUN_CIH_LENGTH_OFFSET);
        flags = *(uint32 *)(buf + SOC_CANCUN_CIH_FLAG_OFFSET);

        /* Blob data format */
        switch (flags & SOC_CANCUN_BLOB_FORMAT_MASK) {
            case SOC_CANCUN_BLOB_FORMAT_PIO:
                SOC_IF_ERROR_RETURN
                    (_soc_cancun_cih_pio_load(sw, buf, length, flags));
                buf += (sizeof(uint32) * SOC_CANCUN_CIH_PIO_DATA_BLOB_SIZE);
                break;
            case SOC_CANCUN_BLOB_FORMAT_DMA:
                break;
            case SOC_CANCUN_BLOB_FORMAT_FIFO:
                break;
            case SOC_CANCUN_BLOB_FORMAT_RSVD:
                break;
            default:
                return SOC_E_PARAM;
        }
    }
    return SOC_E_NONE;
}


static int _soc_cancun_cmh_list_update(soc_cancun_cmh_t *cmh) {
    soc_cancun_hash_table_t *hash_table;
    soc_cancun_cmh_map_t *cmh_map_entry;
    uint32 *p_hask_key, *p_entry, *p_list;
    uint32 entry_num, list_element_count = 0;
    int i;

    hash_table = (soc_cancun_hash_table_t*) cmh->cmh_table;

    if(cmh->cmh_list) {
        kfree(cmh->cmh_list);
    }
    cmh->cmh_list = kmalloc((BYTES_PER_UINT32 *
                              (hash_table->entry_num * 3 + 1)), GFP_KERNEL); //"soc_cancun_cmh_list");
    if(cmh->cmh_list == NULL) {
        return SOC_E_MEMORY;
    }

    p_hask_key = &hash_table->table_entry;
    p_list = cmh->cmh_list + 1;
    for(i = 0; i < hash_table->pd; i++) {
        if(p_hask_key[i] != 0) {
            p_entry = p_hask_key + p_hask_key[i];
            entry_num = *p_entry++;
            cmh_map_entry = (soc_cancun_cmh_map_t*) p_entry;
            while(entry_num-- > 0) {
                *p_list++ = cmh_map_entry->src_mem;
                *p_list++ = cmh_map_entry->src_field;
                *p_list++ = cmh_map_entry->src_app;
                list_element_count++;
                if(entry_num > 0) {
                    p_entry += cmh_map_entry->entry_size;
                    cmh_map_entry = (soc_cancun_cmh_map_t*) p_entry;
                }
            }
        }
    }
    *cmh->cmh_list = list_element_count;

    return SOC_E_NONE;
}

static int _soc_cancun_file_cmh_load(struct bcmsw_switch *sw, uint8* buf, int buf_words) {
    soc_cancun_cmh_t *cmh = sw->soc_cancun_info->cmh;

    if(cmh == NULL) {
        return SOC_E_INIT;
    } else if (buf_words <= 0) {
        return SOC_E_PARAM;
    }

    /* NOTE: We do not have sal_realloc in our libc so need to free and then
     *       re-alloc here for a new CMH load*/
    if(cmh->cmh_table) {
        kfree(cmh->cmh_table);
    }
    cmh->cmh_table = kmalloc((BYTES_PER_UINT32 * buf_words), GFP_KERNEL); // "soc_cancun_cmh_table");
    if(cmh->cmh_table == NULL) {
        return SOC_E_MEMORY;
    }

    _soc_cancun_memcpy_letohl((uint32 *)cmh->cmh_table, (uint32 *)buf, buf_words);

    return _soc_cancun_cmh_list_update(cmh);

}


static int _soc_cancun_file_cch_load(struct bcmsw_switch *sw, uint8* buf, int buf_words) {
    soc_cancun_cch_t *cch = sw->soc_cancun_info->cch;

    if(cch == NULL) {
        return SOC_E_INIT;
    } else if (buf_words <= 0) {
        return SOC_E_PARAM;
    }

    /* NOTE: We do not have sal_realloc in our libc so need to free and then
     *       re-alloc here for a new CCH load*/
    if(cch->cch_table) {
        kfree(cch->cch_table);
    }
    cch->cch_table = kmalloc((BYTES_PER_UINT32 * buf_words), GFP_KERNEL); //"soc_cancun_cch_table");
    if(cch->cch_table == NULL) {
        return SOC_E_MEMORY;
    }

    if(cch->pseudo_regs) {
        kfree(cch->pseudo_regs);
    }
    cch->pseudo_regs = (uint64*) kmalloc(SOC_CANCUN_PSEUDO_REGS_BLOCK_BYTE_SIZE, GFP_KERNEL); // "soc_cancun_cch_pseudo_regs");
    if(cch->pseudo_regs == NULL) {
        return SOC_E_MEMORY;
    }
    memset(cch->pseudo_regs, 0, SOC_CANCUN_PSEUDO_REGS_BLOCK_BYTE_SIZE);

#ifdef BCM_WARM_BOOT_SUPPORT
    if (SOC_WARM_BOOT(unit)) {
        (void)soc_cancun_scache_recovery(unit);
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    _soc_cancun_memcpy_letohl((uint32 *)cch->cch_table, (uint32 *)buf, buf_words);

    return SOC_E_NONE;

}

static int _soc_cancun_file_ceh_load(struct bcmsw_switch *sw, uint8* buf, int buf_words)
{
    uint8 *cur_dst;
    uint8 *cur_src;
    int cur_len;
    int str_tbl_len;
    int rem_len;
    soc_cancun_ceh_t *ceh = sw->soc_cancun_info->ceh;

    if(ceh == NULL) {
        return SOC_E_INIT;
    } else if (buf_words <= 0) {
        return SOC_E_PARAM;
    }

    /* NOTE: We do not have sal_realloc in our libc so need to free and then
     *       re-alloc here for a new CEH load*/
    if(ceh->ceh_table) {
        kfree(ceh->ceh_table);
    }
    ceh->ceh_table = kmalloc((BYTES_PER_UINT32 * buf_words), GFP_KERNEL); // "soc_cancun_ceh_table");
    if(ceh->ceh_table == NULL) {
        return SOC_E_MEMORY;
    }

    cur_dst = (uint8 *)ceh->ceh_table;
    cur_src = buf;
    cur_len = SOC_CANCUN_CEH_BLOCK_HASH_HEADER_LEN * BYTES_PER_UINT32;
    rem_len = buf_words * BYTES_PER_UINT32;
    
    /* load block and hash header */
    memcpy(cur_dst, cur_src,cur_len);
    soc_cancun_buf_swap32(cur_dst, cur_len);

    /* next, string table len */
    cur_dst += cur_len;
    cur_src += cur_len; 
    rem_len -= cur_len;
    cur_len = BYTES_PER_UINT32;
    memcpy(cur_dst, cur_src,cur_len);
    soc_cancun_buf_swap32(cur_dst, cur_len);

    /* no endianness to convert for string table. Just copy */
    str_tbl_len = *(uint32 *)cur_dst; 
    cur_dst += cur_len;
    cur_src += cur_len; 
    rem_len -= cur_len;
    cur_len = str_tbl_len - BYTES_PER_UINT32;
    memcpy(cur_dst,cur_src, cur_len);

    /* convert rest of buffer */   
    cur_dst += cur_len;
    cur_src += cur_len; 
    rem_len -= cur_len;
    memcpy(cur_dst, cur_src,rem_len);
    soc_cancun_buf_swap32(cur_dst, rem_len);

    return SOC_E_NONE;
}

int soc_cancun_file_load(struct bcmsw_switch *sw, uint8* buf, long buf_bytes, uint32* type,
                          uint32* format) {
    soc_cancun_t *cc;
    soc_cancun_file_t ccf_file, *ccf;
    uint32 status = SOC_CANCUN_LOAD_STATUS_NOT_LOADED;
    int rv = SOC_E_NONE;

    cc = sw->soc_cancun_info;
    if(cc == NULL) {
        return SOC_E_UNIT;
    } else if (!(cc->flags & SOC_CANCUN_FLAG_INITIALIZED)) {
        return SOC_E_INIT;
    } else if (buf == NULL) {
        return SOC_E_PARAM;
    }

    /* Get file information if input PACK or UNKNOWN format */
    memset(&ccf_file, 0, sizeof(soc_cancun_file_t));
    if (*format == CANCUN_SOC_FILE_FORMAT_PACK ||
        *format == CANCUN_SOC_FILE_FORMAT_UNKNOWN) {
        rv = soc_cancun_file_info_get(sw, &ccf_file, NULL, buf, buf_bytes);
        if(rv == SOC_E_NONE) {
            *type = ccf_file.type;
            *format = ccf_file.format;

            if(*format == CANCUN_SOC_FILE_FORMAT_PACK) {
                buf += SOC_CANCUN_FILE_HEADER_OFFSET;
                buf_bytes -= (SOC_CANCUN_FILE_HEADER_OFFSET + 1);
            } else {
                return SOC_E_BADID;
            }
        } else {
            return rv;
        }
    }

    if(*type == CANCUN_SOC_FILE_TYPE_CIH) {
        //soc_cancun_file_branch_id_e branch_id = 0;
        ccf = &cc->cih->file;
        status = SOC_CANCUN_LOAD_STATUS_IN_PROGRESS;
        if(*format == CANCUN_SOC_FILE_FORMAT_PIO) {
            cc->cih->status = status;
            rv = _soc_cancun_file_pio_load(sw, buf,
                                           (buf_bytes/BYTES_PER_UINT32));

        } else if(*format == CANCUN_SOC_FILE_FORMAT_PACK) {
	        if(SOC_WARM_BOOT(unit)) {
                rv = SOC_E_NONE;
            } else {
                cc->cih->status = status;
                rv = _soc_cancun_cih_load(sw, buf,
                                      ccf_file.header.num_data_blobs);
            }

        } else {
            return SOC_E_PARAM;
        }

        if(rv == SOC_E_NONE) {
            status = SOC_CANCUN_LOAD_STATUS_LOADED;
            cc->cih->version = ccf_file.header.version;
            cc->flags |= SOC_CANCUN_FLAG_CIH_LOADED;

        } else {
            status = SOC_CANCUN_LOAD_STATUS_FAILED;
            cc->flags &= ~SOC_CANCUN_FLAG_CIH_LOADED;
        }
        cc->cih->status = status;
        //rv = soc_cancun_branch_id_get(unit,
        //               CANCUN_SOC_FILE_TYPE_CIH, &branch_id);
        //if (branch_id == CANCUN_FILE_BRANCH_ID_HGoE) {
        //    SOC_FEATURE_SET(unit, soc_feature_higig_over_ethernet);
        //}


    } else if (*type == CANCUN_SOC_FILE_TYPE_CMH) {
        ccf = &cc->cmh->file;
        status = SOC_CANCUN_LOAD_STATUS_IN_PROGRESS;
        cc->cmh->status = status;
        rv = _soc_cancun_file_cmh_load(sw, buf, (buf_bytes/BYTES_PER_UINT32));

        if(rv == SOC_E_NONE) {
            status = SOC_CANCUN_LOAD_STATUS_LOADED;
            cc->cmh->version = ccf_file.header.version;
            cc->cmh->sdk_version = ccf_file.header.sdk_version;
            cc->flags |= SOC_CANCUN_FLAG_CMH_LOADED;
        } else {
            status = SOC_CANCUN_LOAD_STATUS_FAILED;
            cc->flags &= ~SOC_CANCUN_FLAG_CMH_LOADED;
        }
        cc->cmh->status = status;

    } else if (*type == CANCUN_SOC_FILE_TYPE_CCH) {
        ccf = &cc->cch->file;
        status = SOC_CANCUN_LOAD_STATUS_IN_PROGRESS;
        cc->cch->status = status;
        rv = _soc_cancun_file_cch_load(sw, buf, (buf_bytes/BYTES_PER_UINT32));

        if(rv == SOC_E_NONE) {
            status = SOC_CANCUN_LOAD_STATUS_LOADED;
            cc->cch->version = ccf_file.header.version;
            cc->cch->sdk_version = ccf_file.header.sdk_version;
            cc->flags |= SOC_CANCUN_FLAG_CCH_LOADED;
        } else {
            status = SOC_CANCUN_LOAD_STATUS_FAILED;
            cc->flags &= ~SOC_CANCUN_FLAG_CCH_LOADED;
        }
        cc->cch->status = status;

    } else if (*type == CANCUN_SOC_FILE_TYPE_CEH) {
        ccf = &cc->ceh->file;
        status = SOC_CANCUN_LOAD_STATUS_IN_PROGRESS;
        cc->ceh->status = status;
        rv = _soc_cancun_file_ceh_load(sw, buf, (buf_bytes/BYTES_PER_UINT32));

        if(rv == SOC_E_NONE) {
            status = SOC_CANCUN_LOAD_STATUS_LOADED;
            cc->ceh->version = ccf_file.header.version;
            cc->ceh->sdk_version = ccf_file.header.sdk_version;
            cc->flags |= SOC_CANCUN_FLAG_CEH_LOADED;
        } else {
            status = SOC_CANCUN_LOAD_STATUS_FAILED;
            cc->flags &= ~SOC_CANCUN_FLAG_CEH_LOADED;
        }
        cc->ceh->status = status;

    } else if (*type == CANCUN_SOC_FILE_TYPE_CFH)  {
        ccf =  &cc->flow_db->file;
        status = SOC_CANCUN_LOAD_STATUS_IN_PROGRESS;
        cc->flow_db->status = status;
//TODO
        //rv = _soc_flow_db_load(sw, buf, (buf_bytes/BYTES_PER_UINT32));

        if(rv == SOC_E_NONE) {
            status = SOC_CANCUN_LOAD_STATUS_LOADED;
            cc->flow_db->version = ccf_file.header.version;
            cc->flags |= SOC_CANCUN_FLAG_CFH_LOADED;
        } else {
            status = SOC_CANCUN_LOAD_STATUS_FAILED;
            cc->flags &= ~SOC_CANCUN_FLAG_CFH_LOADED;
        }
        cc->flow_db->status = status;
    } else {
        printk("ERROR: can't recognize file type enum %d\n", *type);
        return SOC_E_PARAM;
    }

    /* File successfully loaded here. Update ccf */
    if(ccf) {
        memcpy(ccf, &ccf_file, sizeof(soc_cancun_file_t));
        ccf->status = status;
    }

    return rv;
}


/*
 * Internal Functions
 */
static int _soc_cancun_alloc(struct bcmsw_switch *sw) 
{
    soc_cancun_t* cc;
    sw->soc_cancun_info = kmalloc(sizeof(soc_cancun_t), GFP_KERNEL); // "soc_cancun_info");
    if(cc == NULL) {
        goto _soc_cancun_alloc_error;
    }
    memset(sw->soc_cancun_info, 0, sizeof(soc_cancun_t));

    cc = sw->soc_cancun_info;

    cc->cih = kmalloc(sizeof(soc_cancun_cih_t), GFP_KERNEL); // "soc_cancun_cih");
    if(cc->cih == NULL) {
        goto _soc_cancun_alloc_error;
    }
    memset(cc->cih, 0, sizeof(soc_cancun_cih_t));

    cc->cmh = kmalloc(sizeof(soc_cancun_cmh_t), GFP_KERNEL); // "soc_cancun_cmh");
    if(cc->cmh == NULL) {
        goto _soc_cancun_alloc_error;
    }
    memset(cc->cmh, 0, sizeof(soc_cancun_cmh_t));

    cc->cch = kmalloc(sizeof(soc_cancun_cch_t), GFP_KERNEL); // "soc_cancun_cch");
    if(cc->cch == NULL) {
        goto _soc_cancun_alloc_error;
    }

    memset(cc->cch, 0, sizeof(soc_cancun_cch_t));
    cc->flow_db = kmalloc(sizeof(soc_flow_db_t), GFP_KERNEL); // "soc_flow_db");
    if(cc->flow_db == NULL) {
        goto _soc_cancun_alloc_error;
    }
    memset(cc->flow_db, 0, sizeof(soc_flow_db_t));

    cc->ceh = kmalloc(sizeof(soc_cancun_ceh_t), GFP_KERNEL); // "soc_cancun_ceh");
    if(cc->ceh == NULL) {
        goto _soc_cancun_alloc_error;
    }
    memset(cc->ceh, 0, sizeof(soc_cancun_ceh_t));

    return 0;

_soc_cancun_alloc_error:
    if(cc) {
        if(cc->cih) {
            kfree(cc->cih);
        }
        if(cc->cmh) {
            kfree(cc->cmh);
        }
        if(cc->cch) {
            kfree(cc->cch);
        }
        if (cc->flow_db) {
            kfree(cc->flow_db);
        }
        if(cc->ceh) {
            kfree(cc->ceh);
        }
        kfree(cc);
    }

    return -1;
}


int soc_cancun_init (struct bcmsw_switch *swdev) 
{
    int ret = SOC_E_NONE;

    ret = _soc_cancun_alloc(swdev);

    //load files from buffer

    return ret;
}

/*****************************************************************************************/
/*                             switchdev                                                 */
/*****************************************************************************************/
static struct workqueue_struct *bcmsw_switchdev_wq;

static void bcmsw_fdb_event_work(struct work_struct *work)
{
	//struct switchdev_notifier_fdb_info *fdb_info;
	struct bcmsw_switchdev_event_work *switchdev_work;
	//struct prestera_port *port;
	struct net_device *dev;

	switchdev_work = container_of(work, struct bcmsw_switchdev_event_work, work);
	dev = switchdev_work->dev;

	rtnl_lock();

#if 0
	port = prestera_port_dev_lower_find(dev);
	if (!port)
		goto out_unlock;

	switch (switchdev_work->event) {
	case SWITCHDEV_FDB_ADD_TO_DEVICE:
		fdb_info = &switchdev_work->fdb_info;
		if (!fdb_info->added_by_user || fdb_info->is_local)
			break;

		err = bcmsw_port_fdb_set(port, fdb_info, true);
		if (err)
			break;

		bcmsw_fdb_offload_notify(port, fdb_info);
		break;

	case SWITCHDEV_FDB_DEL_TO_DEVICE:
		fdb_info = &switchdev_work->fdb_info;
		bcmsw_port_fdb_set(port, fdb_info, false);
		break;
	}

out_unlock:
#endif
	rtnl_unlock();

	kfree(switchdev_work->fdb_info.addr);
	kfree(switchdev_work);
	dev_put(dev);
}

extern bool bkn_port_dev_check(const struct net_device *dev);

static int bcmsw_switchdev_event(struct notifier_block *unused,
				    unsigned long event, void *ptr)
{
	struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
	struct switchdev_notifier_fdb_info *fdb_info;
	struct switchdev_notifier_info *info = ptr;
	struct bcmsw_switchdev_event_work *switchdev_work;
	struct net_device *upper;
	//int err;

	if (event == SWITCHDEV_PORT_ATTR_SET) {
#if 0
		err = switchdev_handle_port_attr_set(dev, ptr,
						     bkn_port_dev_check,
						     bcmsw_port_obj_attr_set);
		return notifier_from_errno(err);
#endif
		return NOTIFY_DONE;
	}

	if (!bkn_port_dev_check(dev))
		return NOTIFY_DONE;

	upper = netdev_master_upper_dev_get_rcu(dev);
	if (!upper)
		return NOTIFY_DONE;

	if (!netif_is_bridge_master(upper))
		return NOTIFY_DONE;

	switchdev_work = kzalloc(sizeof(*switchdev_work), GFP_ATOMIC);
	if (!switchdev_work)
		return NOTIFY_BAD;

	switchdev_work->event = event;
	switchdev_work->dev = dev;

	switch (event) {
	case SWITCHDEV_FDB_ADD_TO_DEVICE:
	case SWITCHDEV_FDB_DEL_TO_DEVICE:
		fdb_info = container_of(info,
					struct switchdev_notifier_fdb_info,
					info);

		INIT_WORK(&switchdev_work->work, bcmsw_fdb_event_work);
		memcpy(&switchdev_work->fdb_info, ptr,
		       sizeof(switchdev_work->fdb_info));

		switchdev_work->fdb_info.addr = kzalloc(ETH_ALEN, GFP_ATOMIC);
		if (!switchdev_work->fdb_info.addr)
			goto out_bad;

		ether_addr_copy((u8 *)switchdev_work->fdb_info.addr,
				fdb_info->addr);
		dev_hold(dev);
		break;

	default:
		kfree(switchdev_work);
		return NOTIFY_DONE;
	}

	queue_work(bcmsw_switchdev_wq, &switchdev_work->work);
	return NOTIFY_DONE;

out_bad:
	kfree(switchdev_work);
	return NOTIFY_BAD;
}


int bcmsw_port_obj_add(struct net_device *dev, const void *ctx,
				 const struct switchdev_obj *obj,
				 struct netlink_ext_ack *extack)
{
	return 0;
}
int bcmsw_port_obj_del(struct net_device *dev, const void *ctx,
				 const struct switchdev_obj *obj)
{
	return 0;
}
int bcmsw_port_obj_attr_set(struct net_device *dev, const void *ctx,
				  const struct switchdev_attr *attr,
				  struct netlink_ext_ack *extack)
{
	return 0;
}


static int bcmsw_switchdev_blk_event(struct notifier_block *unused,
					unsigned long event, void *ptr)
{
	struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
	int err;

	switch (event) {
	case SWITCHDEV_PORT_OBJ_ADD:
		err = switchdev_handle_port_obj_add(dev, ptr,
						    bkn_port_dev_check,
						    bcmsw_port_obj_add);
		break;
	case SWITCHDEV_PORT_OBJ_DEL:
		err = switchdev_handle_port_obj_del(dev, ptr,
						    bkn_port_dev_check,
						    bcmsw_port_obj_del);
		break;
	case SWITCHDEV_PORT_ATTR_SET:
		err = switchdev_handle_port_attr_set(dev, ptr,
						     bkn_port_dev_check,
						     bcmsw_port_obj_attr_set);
		break;
	default:
		return NOTIFY_DONE;
	}

	return notifier_from_errno(err);
}


static int bcmsw_switchdev_handler_init(struct bcmsw_switchdev *swdev)
{
	int err;

	swdev->swdev_nb.notifier_call = bcmsw_switchdev_event;
	err = register_switchdev_notifier(&swdev->swdev_nb);
	if (err)
		goto err_register_swdev_notifier;

	swdev->swdev_nb_blk.notifier_call = bcmsw_switchdev_blk_event;
	err = register_switchdev_blocking_notifier(&swdev->swdev_nb_blk);
	if (err)
		goto err_register_blk_swdev_notifier;

	return 0;

err_register_blk_swdev_notifier:
	unregister_switchdev_notifier(&swdev->swdev_nb);
err_register_swdev_notifier:
	destroy_workqueue(bcmsw_switchdev_wq);
	return err;
}



int bcmsw_switchdev_init(struct bcmsw_switch *sw)
{
	struct bcmsw_switchdev *swdev;
	int err;

	swdev = kzalloc(sizeof(*swdev), GFP_KERNEL);
	if (!swdev)
		return -ENOMEM;

	sw->swdev = swdev;
	swdev->sw = sw;

	INIT_LIST_HEAD(&swdev->bridge_list);

	bcmsw_switchdev_wq = alloc_ordered_workqueue("%s_ordered", 0, "bcmsw_switchdev");
	if (!bcmsw_switchdev_wq) {
		err = -ENOMEM;
		goto err_alloc_wq;
	}

	err = bcmsw_switchdev_handler_init(swdev);
	if (err)
		goto err_swdev_init;

	return 0;

//err_fdb_init:
err_swdev_init:
	destroy_workqueue(bcmsw_switchdev_wq);
err_alloc_wq:
	kfree(swdev);

	return err;
}


static void
_soc_hx5_mmu_idb_ports_assign(soc_info_t *si)
{
    int port, phy_port, mmu_port_os, mmu_port_lr;
    int mmu_port_old, phy_port_old;

    mmu_port_os = 0;

    /* PHY Ports: 61-64
     * MMU/IDB Ports: 0-3
     */
    for (phy_port = 61; phy_port <= 64; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 69-72
     * MMU/IDB Ports: 4-7
     */
    for (phy_port = 69; phy_port <= 72; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 65-68
     * MMU/IDB Ports: 8-11
     */
    for (phy_port = 65; phy_port <= 68; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 73-76
     * MMU/IDB Ports: 12-15
     */
    for (phy_port = 73; phy_port <= 76; phy_port++, mmu_port_os++) {
        if (phy_port >= SOC_MAX_NUM_PORTS) {
            /* coverity[dead_error_line] */
            continue;
        }
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    mmu_port_lr = mmu_port_os;

    
    for (phy_port = 1; phy_port <= 4; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 17-20
     * MMU/IDB Ports: 20-23
     */
    for (phy_port = 17; phy_port <= 20; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 33-36
     * MMU/IDB Ports: 24-27
     */
    for (phy_port = 33; phy_port <= 36; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 49-60
     * MMU/IDB Ports: 40-51
     */
    for (phy_port = 49; phy_port <= 60; phy_port++) {
        si->port_p2m_mapping[phy_port] = phy_port - 9;
        si->port_m2p_mapping[phy_port - 9] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 1-60
     * MMU/IDB Ports within 16-68
     *
     * When DL ports are in LR we use a running number
     * In Line-rate config PM4x10Q can be in GMII mode (upto 16 lanes)
     */
    for (phy_port = 1; phy_port <= 60; phy_port++) {
        port = si->port_p2l_mapping[phy_port];
        //if ((port == -1) || IS_OVERSUB_PORT(unit, port)) {
        if (port == -1) {
            continue;
        }

        mmu_port_old = si->port_p2m_mapping[phy_port];
        phy_port_old = si->port_m2p_mapping[mmu_port_lr];

        if (phy_port_old != -1) {
            si->port_p2m_mapping[phy_port_old] = -1;
        }
        if (mmu_port_old != -1) {
            si->port_m2p_mapping[mmu_port_old] = -1;
        }
        si->port_p2m_mapping[phy_port] = mmu_port_lr;
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
        si->port_m2p_mapping[mmu_port_lr] = phy_port;
        mmu_port_lr++;
    }

    return;
}


static void bcmsw_soc_info_init(soc_info_t *si)
{
    int index, port, pipe, phy_port, mmu_port;
    int num_port = HX5_NUM_PORT; 
    int num_phy_port = HX5_NUM_PHY_PORT;
    int num_mmu_port = HX5_NUM_MMU_PORT;

    si->bandwidth = 2048000;

    for (phy_port = 0; phy_port < num_phy_port; phy_port++) {
        si->port_p2l_mapping[phy_port] = -1;
        si->port_p2m_mapping[phy_port] = -1;
    }
    for (index = 0; index < num_port; index++) {
        si->port_l2p_mapping[index] = -1;
        si->port_l2i_mapping[index] = -1;
        si->port_speed_max[index] = -1;
        si->port_group[index] = -1;
        si->port_serdes[index] = -1;
        si->port_pipe[index] = -1;
        si->port_num_lanes[index] = -1;
        si->port_type[index] = -1;
        si->ports[index].valid = FALSE;
        si->ports[index].probed = FALSE;
	    si->ports[index].ext_phy_addr = -1;
        si->ports[index].primary_and_offset = -1;
        si->ports[index].eth_port_type = 0;
    }
    for (mmu_port = 0; mmu_port < num_mmu_port; mmu_port++) {
        si->port_m2p_mapping[mmu_port] = -1;
    }
/*
    SOC_PBMP_CLEAR(si->eq_pbm);
    SOC_PBMP_CLEAR(si->management_pbm);
    for (pipe = 0; pipe < NUM_PIPE(unit); pipe++) {
        SOC_PBMP_CLEAR(si->pipe_pbm[pipe]);
    };
    SOC_PBMP_CLEAR(si->oversub_pbm);
    SOC_PBMP_CLEAR(si->all.disabled_bitmap);
*/
    /* Populate the fixed mapped ports */
    for (index = 0; index < COUNTOF(hx5_anc_ports); index++) {
        port = hx5_anc_ports[index].port;
        phy_port = hx5_anc_ports[index].phy_port;
        pipe = hx5_anc_ports[index].pipe;;

        si->port_l2p_mapping[port] = phy_port;
        si->port_l2i_mapping[port] = hx5_anc_ports[index].idb_port;
        si->port_p2l_mapping[phy_port] = port;
        si->port_p2m_mapping[phy_port] = hx5_anc_ports[index].mmu_port;
        si->port_pipe[port] = pipe;
        si->port_type[port] = hx5_anc_ports[index].port_type;
        //SOC_PBMP_PORT_ADD(si->pipe_pbm[pipe], port);
        si->ports[port].valid = TRUE;
    }

    //FIXME hardcoded for N3248TE
    for (index = 0; index < COUNTOF(n3248te_ports); index++) {
        if (n3248te_ports[index].port == -1) {
            break;
        }
        port = n3248te_ports[index].port;
        phy_port = n3248te_ports[index].phy_port;
        /* Update soc_info */      
        si->port_l2p_mapping[port] = phy_port;
        si->port_p2l_mapping[phy_port] = port;
        si->port_pipe[port] = 0;
        si->port_speed_max[port] = n3248te_ports[index].bandwidth * 1000;
        si->port_init_speed[port] = n3248te_ports[index].bandwidth * 1000;
        si->port_serdes[port] = phy_port - 1; //?

        if (n3248te_ports[index].bandwidth <= 10) {
            si->port_num_lanes[port] = 1;
        } else if (n3248te_ports[index].bandwidth <= 20) {
            si->port_num_lanes[port] = 2;
        } else {
            si->port_num_lanes[port] = 4;
        } 

        //port_type
        si->port_type[port] = n3248te_ports[index].port_type;
        si->ports[port].valid = TRUE;
        si->ports[port].ext_phy_addr = n3248te_ports[index].ext_phy_addr;
        si->ports[port].primary_and_offset = n3248te_ports[index].primary_and_offset;
        //FIXME, only GE port supported
        if (si->port_init_speed[port] == 1000) {
            si->ports[port].eth_port_type = ETH_GE_PORT;
        }
    }
    si->cpu_hg_index = 72;
    //TODO flex port init

    //DUMP SOC INFO
    printk("helix5 port config ------------------------\n");
    for (index =0; index< 79; index++) {
        printk(" %i %i %i %i %i %i %i\n",
                si->port_l2p_mapping[index],
                si->port_p2l_mapping[index],
                si->port_l2i_mapping[index],
                si->port_p2m_mapping[index],
                si->port_m2p_mapping[index],
                si->port_pipe[index],
                si->port_serdes[index]);
    }
}




#define BCMSW_VLAN_DEFAULT    1

//bcm_esw_port_init bcm_td3_port_cfg_init
static int 
_port_cfg_init(struct bcmsw_switch *bcmsw_sw, int port, int vid)
{
    soc_info_t *si = bcmsw_sw->si;
    uint32 port_type;
    int cpu_hg_index = -1;    

    egr_port_entry_t   egr_port_entry;  
    lport_tab_entry_t  lport_entry;
    ing_device_port_entry_t ing_device_port_entry;

    if (si->port_type[port] == BCMSW_PORT_TYPE_CMIC) {
        cpu_hg_index = si->cpu_hg_index;
        port_type = 0; /* Same as Ethernet port */
        //egr_port_type = 1; /* CPU port needs to be HG port in EGR_PORT table */
    } else if (si->port_type[port] == BCMSW_PORT_TYPE_LBPORT) {
        port_type = 2;
        //egr_port_type = 2;
    } else {
        port_type = 0;
    }

    /* EGR_LPORT_TABLE config init */
    //read EGR_PORTm
    _soc_mem_read(bcmsw_sw->dev, EGR_PORTm+port, SCHAN_BLK_EPIPE, BYTES2WORDS(sizeof(egr_port_entry_t)), &egr_port_entry); 

    egr_port_entry.port_type = port_type;

    _soc_mem_write(bcmsw_sw->dev, EGR_PORTm+port, SCHAN_BLK_EPIPE, BYTES2WORDS(sizeof(egr_port_entry_t)), &egr_port_entry); 

    /* initialize the Cancun tag profile entry setup
     * for VT_MISS_UNTAG action. Should be done in Cancun
     */
    //soc_cancun_cmh_mem_set(unit, 

    /* Copy EGR port information to CPU Higig port if applied */
    //Not applicable for BCM56370

    /* PORT_TABLE config init */
    //read LPORT_TABm , check _bcm_td3_port_tab_conv for memory
    _soc_mem_read(bcmsw_sw->dev, LPORT_TABm+port, SCHAN_BLK_IPIPE, BYTES2WORDS(sizeof(lport_tab_entry_t)), &lport_entry); 

    lport_entry.reg.PORT_VIDf = BCMSW_VLAN_DEFAULT;
    lport_entry.reg.MAC_BASED_VID_ENABLEf = 1;
    lport_entry.reg.SUBNET_BASED_VID_ENABLEf = 1;
    lport_entry.reg.PRI_MAPPINGf = 0xfac688;
    lport_entry.reg.CFI_0_MAPPINGf = 0;
    lport_entry.reg.CFI_1_MAPPINGf = 1;
    lport_entry.reg.IPRI_MAPPINGf_lo = 0xfac688 & 0x3F;
    lport_entry.reg.IPRI_MAPPINGf_hi = 0xfac688 >> 6; 
    lport_entry.reg.ICFI_0_MAPPINGf = 0;
    lport_entry.reg.ICFI_1_MAPPINGf = 1;
    lport_entry.reg.CML_FLAGS_NEWf_lo = 0x8 & 0x7;
    lport_entry.reg.CML_FLAGS_NEWf_hi = 0x8 >> 3;
    lport_entry.reg.CML_FLAGS_MOVEf = 0x8;
    //_bcm_esw_pt_vtkey_type_value_get(unit, VLXLT_HASH_KEY_TYPE_OVID,
    //lport_entry.reg.VT_KEY_TYPEf = 
    lport_entry.reg.VT_PORT_TYPE_SELECT_1f = 1;
    //_bcm_esw_pt_vtkey_type_value_get(unit, VLXLT_HASH_KEY_TYPE_IVID,
    //lport_entry.reg.VT_KEY_TYPE_2f = 
    lport_entry.reg.VT_PORT_TYPE_SELECT_2f = 1;
    //lport_entry.reg.PORT_TYPEf = port_type;
    //lport_entry.reg.SRC_SYS_PORT_IDf = port;
     /* TD3TBD SYS_PORT_ID and PP_PORT_NUM should be covered by CIH,
     * will remove it after CIH is ready. */
    //lport_entry.reg.SYS_PORT_IDf = port;
    //lport_entry.reg.PP_PORT_NUMf = port;
    //lport_entry.reg.DUAL_MODID_ENABLEf = 0; //dual_modid;
    lport_entry.reg.TAG_ACTION_PROFILE_PTRf = 1;

    _soc_mem_write(bcmsw_sw->dev, LPORT_TABm+port, SCHAN_BLK_IPIPE, BYTES2WORDS(sizeof(lport_tab_entry_t)), &lport_entry); 

    if (cpu_hg_index != -1) {
        //soc_cancun_cmh_mem_set(unit, PORT_TABm, cpu_hg_index, PORT_TYPEf, 1);
        /* TD3TBD should be covered by CMH, will remove it after CMH
         * is ready. */
         _soc_mem_read(bcmsw_sw->dev, ING_DEVICE_PORTm+cpu_hg_index, SCHAN_BLK_IPIPE, BYTES2WORDS(sizeof(ing_device_port_entry_t)), &ing_device_port_entry); 

        //BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit, ING_DEVICE_PORTm,
        //                    cpu_hg_index, PORT_TYPEf, 1));
        ing_device_port_entry.reg.PORT_TYPEf = 1;
        ing_device_port_entry.reg.SRC_SYS_PORT_IDf = port;
        ing_device_port_entry.reg.SYS_PORT_IDf = port;
        ing_device_port_entry.reg.PP_PORT_NUMf = port;
        ing_device_port_entry.reg.DUAL_MODID_ENABLEf = 0; //dual_modid;
        _soc_mem_write(bcmsw_sw->dev, ING_DEVICE_PORTm+cpu_hg_index, SCHAN_BLK_IPIPE, BYTES2WORDS(sizeof(ing_device_port_entry_t)), &ing_device_port_entry); 
    }    

    return 0;
}



/*****************************************************************************************/
/*                            CMICX MIIM read/write                                      */
/*****************************************************************************************/

static int _iproc_setreg(uint32_t addr, uint32_t data)
{
    return kernel_bde->iproc_write(0, addr, data);
}

static uint32_t _iproc_getreg(uint32 addr)
{
    return kernel_bde->iproc_read(0, addr);
}

static
uint32 _cmicx_miim_cycle_type_get(int is_write, int clause45, uint32 phy_devad)
{
    uint32 cycle_type = 0;
    int is_miim_12r = 1;

    if (clause45) {
        if(is_write) {
            if (phy_devad & MIIM_CYCLE_C45_WR) {
                cycle_type = is_miim_12r ? MIIM_CYCLE_12R_C45_REG_WR : MIIM_CYCLE_C45_REG_WR;
            } else if (phy_devad & MIIM_CYCLE_C45_WR_AD) {
                cycle_type = is_miim_12r ? MIIM_CYCLE_12R_C45_REG_AD : MIIM_CYCLE_C45_REG_AD;
            } else {
                cycle_type = is_miim_12r ? MIIM_CYCLE_12R_C45_AUTO_WR : MIIM_CYCLE_AUTO;
            }
        } else {
            if (phy_devad & MIIM_CYCLE_C45_RD) {
                cycle_type = is_miim_12r ? MIIM_CYCLE_12R_C45_REG_RD : MIIM_CYCLE_C45_REG_RD;
            } else if (phy_devad & MIIM_CYCLE_C45_RD_ADINC) {
                cycle_type = is_miim_12r ? MIIM_CYCLE_12R_C45_REG_RD_ADINC : MIIM_CYCLE_C45_REG_RD_ADINC;
            } else {
                cycle_type = is_miim_12r ? MIIM_CYCLE_12R_C45_AUTO_RD : MIIM_CYCLE_AUTO;
            }
        }
    } else {
        /* clause 22 */
        if (is_write) {
            cycle_type = is_miim_12r ? MIIM_CYCLE_12R_C22_REG_WR : MIIM_CYCLE_C22_REG_WR;
        } else {
            cycle_type = is_miim_12r ? MIIM_CYCLE_12R_C22_REG_RD : MIIM_CYCLE_C22_REG_RD;
        }
    }

    //LOG_DEBUG(BSL_LS_SOC_MIIM, (BSL_META_U(unit, "is_miim_12r %d, cycle_type %d\n"), is_miim_12r, cycle_type));

    return cycle_type;
}


static int 
_cmicx_miim_operation_cl22(int is_write, uint32 phy_id, uint32_t phy_reg_addr, uint16 *phy_data)
{
    miim_ch_address_t ch_addr;
    miim_ch_params_t  ch_params;
    miim_ch_control_t ch_control;
    miim_ch_status_t  ch_status;
    int miim_timeout  = 0; 
    uint32_t internal_select, real_phy_id, real_bus_id, is_done, is_error;
    uint32 mdio_buses;
    uint32 cycle_type = 0, phy_devad = 0;
   //return soc_cmicx_miim_operation(unit, FALSE, SOC_CLAUSE_22, phy_id, phy_reg_addr, phy_rd_data);

    /* parse phy_id to bus, phy, and internal indication
    *   phy_id encoding:
    *       bit31-16, bitmap for MDIO bus.
    *       bit7, 1: internal MDIO bus, 0: external MDIO bus
    *       bit10, broadcast mode
    *       bit9,8,6,5, mdio bus number
    *       bit4-0,   mdio addresses
    */
   internal_select = PHY_ID_INTERNAL(phy_id);
   real_phy_id = PHY_ID_ADDR(phy_id);
   real_bus_id = PHY_ID_BUS_NUM(phy_id);
   //is_broadcast = PHY_ID_BROADCAST(phy_id);
   //mdio_buses = PHY_ID_BUSMAP(phy_id);
   mdio_buses = (1 << real_bus_id);

   ch_addr.word = _iproc_getreg(MIIM_CH0_ADDRESSr);
   ch_addr.reg.PHY_IDf = real_phy_id;
   ch_addr.reg.CLAUSE_22_REGADRR_OR_45_DTYPEf = (phy_reg_addr & 0x1f);

   _iproc_setreg(MIIM_CH0_ADDRESSr, ch_addr.word);

   ch_params.word = _iproc_getreg(MIIM_CH0_PARAMSr);

   ch_params.reg.SEL_INT_PHYf = internal_select;
   ch_params.reg.RING_MAPf = mdio_buses;

   phy_devad = phy_reg_addr >> 16;
   cycle_type = _cmicx_miim_cycle_type_get(is_write, 0, phy_devad);

   ch_params.reg.MDIO_OP_TYPEf = cycle_type;

   if (is_write) {
       ch_params.reg.PHY_WR_DATAf = *phy_data;
   } else {
       ch_params.reg.PHY_WR_DATAf = 0x0;
   }
   
   _iproc_setreg(MIIM_CH0_PARAMSr, ch_params.word);

   //printk("read id=0x%02x addr=0x%02x real_phy_id=0x%x, mdio_buses=0x%x internal 0x%x cycle 0x%x\n",
   //	  phy_id, phy_reg_addr, real_phy_id, mdio_buses, internal_select, cycle_type);

    /* start transaction */
    ch_control.reg.STARTf = 1;
    //printk("ch_control 0x%x\n", ch_control.word);
    _iproc_setreg(MIIM_CH0_CONTROLr, ch_control.word);

    //readback
    ch_control.word = _iproc_getreg(MIIM_CH0_CONTROLr);
    //printk("ch_control readback 0x%x\n", ch_control.word);

    do {
        ch_status.word = _iproc_getreg(MIIM_CH0_STATUSr);
        is_done = ch_status.reg.DONEf;
        if (is_done) {
            break; /* MIIM operation is done */
        }        

        /* check for transaction error */
        is_error = ch_status.reg.ERRORf;
        if (is_error) {
            printk("MDIO transaction Error 0x%x ch_status 0x%x\n", is_error, ch_status.word);
            goto exit;
        }
        udelay(1000);
        miim_timeout++;
        if(miim_timeout > 30) {
           printk("MDIO transaction timeout ch_status 0x%x\n", ch_status.word);
           is_error = SOC_E_TIMEOUT;
           goto exit;
        }
    } while(1);

    /* in case of read - get data */
    if (!is_write) { 
        *phy_data = ch_status.reg.PHY_RD_DATAf;
        //printk("_cmicx_miim_operation read data: %d \n",*phy_data);
    }

exit:
    /* cleanup */
    ch_control.reg.STARTf = 0;
    /* no need to catch error in case of failrue */
    _iproc_setreg(MIIM_CH0_CONTROLr, ch_control.word);          

    if (is_error) {
      *phy_data = -1;
    }
    return is_error;               
}
//MIIM write CL22
static int 
_soc_miim_write(uint32 phy_id, uint8 phy_reg_addr, uint16 phy_data)
{
    return _cmicx_miim_operation_cl22(TRUE, phy_id, phy_reg_addr, &phy_data);
}

//MIIM read CL22
static int
_soc_miim_read(uint32 phy_id, uint8 phy_reg_addr, uint16 *phy_data)
{
    return _cmicx_miim_operation_cl22(FALSE, phy_id, phy_reg_addr, phy_data);
}

/*****************************************************************************************/
/*                            Phy Read/Write                                             */
/*****************************************************************************************/

static int 
phy_reg_read(uint16_t phy_addr, uint8_t reg_addr, uint16 * data)
{
    return  _soc_miim_read(phy_addr, reg_addr, data);
}

static int 
phy_reg_write(uint16_t phy_addr, uint8_t reg_addr, uint16 data)
{
    return  _soc_miim_write(phy_addr, reg_addr, data);
}

static int 
phy_reg_modify(uint16_t phy_addr,  
               uint8_t reg_addr,
               uint16 data, uint16 mask)
{
    uint16_t        tmp, otmp;

    data = data & mask;

    phy_reg_read(phy_addr, reg_addr, &tmp);

    otmp = tmp;
    tmp &= ~(mask);
    tmp |= data;

    if (otmp != tmp) {
        phy_reg_write(phy_addr, reg_addr, tmp);
    }
    return SOC_E_NONE;
}
/*****************************************************************************************/
/*                             FE/GE PHY Standard OP                                     */
/*****************************************************************************************/

int
phy_reg_ge_write(int port, uint16_t phy_addr, uint8 reg_addr, uint16 data)
{
    int      rv;
    uint16_t reg_bank = 0;

    rv       = SOC_E_NONE;

    switch(reg_addr) {
        /* Map shadow registers */
        case 0x15:
            /* SHAD_EXPD for 54640E */
            //BCM54640E_SHAD_EXPD_PRE(unit, pc, reg_bank);

            phy_reg_write(phy_addr, 0x17, reg_bank);
            break;
        case 0x18:
            if (reg_bank <= 0x0007) {
                if (reg_bank == 0x0007) {
                    data |= 0x8000;
                }
                data = (data & ~(0x0007)) | reg_bank;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case 0x1C:
            if (reg_bank <= 0x001F) {
                data = 0x8000 | (reg_bank << 10) | (data & 0x03FF);
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case 0x1D:
            if (reg_bank == 0x0000) {
                data = data & 0x07FFF;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        default:
            /* Must not write to reserved registers */ 
            if (reg_addr > 0x001e) {
                rv = SOC_E_PARAM;
            }
            break;
    }

    if ( rv >= 0) {
        rv = phy_reg_write(phy_addr, reg_addr, data);
    }
     

    if (rv < 0) {
        printk("phy_reg_ge_write failed:"
               " u=%d phy_id=0x%2x  reg_addr=0x%02x "
               " rv=%d\n", port, phy_addr, reg_addr, rv); 
    }

    return rv;
}


// Reset PHY and wait for it to come out of reset.
int
phy_fe_ge_reset(int port, uint16_t phy_addr)
{
    uint16          ctrl, tmp;
    int             timeout = 0, reset_timeout = 0;

    /* IEEE Standard Registers */
    /* 1000BASE-T/100BASE-TX/10BASE-T MII Control Register (Addr 00h) */
    phy_reg_read(phy_addr, 0x00, &ctrl);

    phy_reg_ge_write(port, phy_addr, 0x00, (ctrl | MII_CTRL_RESET));

    do {
        phy_reg_read(phy_addr, 0x00, &tmp);

        msleep(10);
        reset_timeout++;
        if(reset_timeout > 2) {
            timeout = 1;
            break;
        }
    } while ((tmp & MII_CTRL_RESET) != 0);

    if (timeout) {
        printk("phy_fe_ge_reset: timeout on u=%d p=%d\n",port, phy_addr);
        phy_reg_ge_write(port, phy_addr, 0x00, ctrl);
    }

    return (SOC_E_NONE);
}

int
phy_fe_ge_enable_set(port_info_t *pport, int port, int enable)
{
    if (enable) {
        pport->phy_flags &= ~PHY_FLAGS_DISABLE;
    } else {
        pport->phy_flags |= PHY_FLAGS_DISABLE;
    }

    return (SOC_E_NONE);
}

int
phy_fe_ge_enable_get(port_info_t *pport, int *enable)
{
    *enable = !(((pport->phy_flags)&PHY_FLAGS_DISABLE) == PHY_FLAGS_DISABLE);
    return(SOC_E_NONE);
}


/*****************************************************************************************/
/*                            BCM542XX Phy Read/Write                                    */
/*****************************************************************************************/

/* RDB register accessing funtions */
static int
phy_bcm542xx_rdb_reg_write(uint16_t phy_addr, uint16_t reg_addr, uint16 data)
{
    int  rv;

    /* MDIO write the RDB reg. address to reg.0x1E = <reg_addr> */
    rv = phy_reg_write(phy_addr, PHY_BCM542XX_RDB_ADDR_REG_OFFSET,PHY_BCM542XX_RDB_ADDR_REG_ADDR & reg_addr);

    if ( SOC_E_NONE == rv ) {
        /* MDIO read from reg.0x1F to modify the RDB register's value */
        rv = phy_reg_write(phy_addr, PHY_BCM542XX_RDB_DATA_REG_OFFSET, data);
    }
    return rv;
}

//RDB register modification  (works ONLY under RDB register addressing mode !!)
static int
phy_bcm542xx_rdb_reg_modify(uint16_t phy_addr, uint16_t reg_addr, uint16 data, uint16 mask)
{
    int  rv;

    /* MDIO write the RDB reg. address to reg.0x1E = <reg_addr> */
    rv = phy_reg_write(phy_addr, PHY_BCM542XX_RDB_ADDR_REG_OFFSET,PHY_BCM542XX_RDB_ADDR_REG_ADDR & reg_addr);

    if ( SOC_E_NONE == rv ) {
        /* MDIO read from reg.0x1F to modify the RDB register's value */
        rv = phy_reg_modify(phy_addr, PHY_BCM542XX_RDB_DATA_REG_OFFSET, data, mask);
    }
    return rv;
}

/* General - PHY register access */
static int
phy_bcm542xx_reg_write(uint16_t phy_addr, uint16_t reg_bank,
                       uint8_t reg_addr, uint16_t data)
{
    int     rv = SOC_E_NONE;
    uint16  val;

    //if ( SOC_WARM_BOOT(unit) || SOC_IS_RELOADING(unit) ) {
    //    return SOC_E_NONE;
    //}
    //rv = SOC_E_NONE;

    switch ( reg_addr ) {
        /* Map shadow registers */
        case 0x15:
            phy_reg_write(phy_addr, 0x17, reg_bank);
            break;
        case 0x18:
            if ( reg_bank <= 0x0007 ) {
                if (reg_bank == 0x0007) {
                    data |= 0x8000;
                }
		data = (data & ~(0x0007)) | reg_bank;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case 0x1C:
            if ( reg_bank <= 0x001F ) {
		data = 0x8000 | (reg_bank << 10) | (data & 0x03FF);
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case 0x1D:
            if ( reg_bank == 0x0000 ) {
		data = data & 0x07FFF;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
    }
    if ( rv >= 0 ) {
        rv = phy_reg_write(phy_addr, reg_addr, data);
    } 

    if ( rv < 0 ) {
        printk("phy_bcm542xx_reg_write: failed:"
               "phy_id=0x%2x reg_bank=0x%04x reg_addr=0x%02x "
               "rv=%d\n", phy_addr, reg_bank, reg_addr, rv);
    }
    return rv;
}

static int
phy_bcm542xx_reg_modify(uint16_t phy_addr, uint16 reg_bank,
                        uint8_t reg_addr, uint16 data, uint16 mask)
{
    int     rv = SOC_E_NONE;
    uint16  val;

    //if ( SOC_WARM_BOOT(unit) || SOC_IS_RELOADING(unit) ) {
    //    return SOC_E_NONE;
    //}
    //rv = SOC_E_NONE;

    switch ( reg_addr ) {
        /* Map shadow registers */
        case 0x15:
            phy_reg_write(phy_addr, 0x17, reg_bank);
            break;
        case 0x18:
            if ( reg_bank <= 0x0007 ) {
                val = (reg_bank << 12) | 0x7;
                phy_reg_write(phy_addr, reg_addr, val);

                if (reg_bank == 0x0007) {
                    data |= 0x8000;
                    mask |= 0x8000;
                }
                mask &= ~(0x0007);
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case 0x1C:
            if ( reg_bank <= 0x001F ) {
                val = (reg_bank << 10);
                phy_reg_write(phy_addr, reg_addr, val);
                data |= 0x8000;
                mask |= 0x8000;
                mask &= ~(0x1F << 10);
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case 0x1D:
            if ( reg_bank == 0x0000 ) {
                mask &= 0x07FFF;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
    }
    if ( rv >= 0 ) {
        rv = phy_reg_modify(phy_addr, reg_addr, data, mask);
    } 

    if ( rv < 0 ) {
        printk("phy_bcm542xx_reg_modify: failed:"
               "phy_id=0x%2x reg_bank=0x%04x reg_addr=0x%02x "
               "rv=%d\n", phy_addr, reg_bank, reg_addr, rv);
    }
    return rv;
}


/*
 * QSGMII register WRITE
 */
int
phy_bcm542xx_qsgmii_reg_write(uint16_t phy_addr, 
        int dev_port, uint16 block, uint8 reg,
        uint16 data)
{
    uint16  val;

    //if ( SOC_WARM_BOOT(unit) || SOC_IS_RELOADING(unit) ) {
    //    return SOC_E_NONE;
    //}

    /* Lanes from 0 to 7 (or 0x1F for all lanes) */
    if ( ((dev_port < 0) || (dev_port > 7)) && (dev_port != 0x1F) ) {
        return SOC_E_FAIL;
    }

    /* Set BAR to AER */
    //if ( PHY_IS_BCM5418x_19x(pc) ) {
    val = 0xFFDE;
    phy_reg_write(phy_addr, 0x1F, val);

    /* set aer reg to access sgmii lane */
    val = dev_port;
    phy_reg_write(phy_addr, 0x1E, val);

    /* set bar to register block */
    val = (block & 0xfff0);
    phy_reg_write(phy_addr, 0x1F, val);

    /* Write the register */
    if (block >= 0x8000) {
        reg |= 0x10;
    }

    phy_reg_write(phy_addr, reg, data);

    printk("phy_bcm542xx_qsgmii_reg_write port %d reg 0x%x data 0x%x\n", phy_addr, reg, data);

    return SOC_E_NONE;
}

/*****************************************************************************************/
/*                             Phy Probe                                                 */
/*****************************************************************************************/

static int
_ext_phy_probe(struct bcmsw_switch *bcmsw_sw, int port)
{
    uint16               phy_addr, phy_id0, phy_id1;
    uint32               id0_addr, id1_addr;
    soc_info_t           *si = bcmsw_sw->si;
    port_info_t          *p_port;

    p_port = &si->ports[port];

    id0_addr = MII_PHY_ID0_REG;
    id1_addr = MII_PHY_ID1_REG;

    phy_addr = p_port->ext_phy_addr;

    //read phy_id0
    _soc_miim_read(phy_addr, id0_addr, &phy_id0);


    //read phy_id1
    _soc_miim_read(phy_addr, id1_addr, &phy_id1);

    //Verify result is expected
    
    //set probed if phy type match
    if( PHY_OUI(phy_id0, phy_id1) ==  PHY_BCM5418X_OUI &&
        PHY_MODEL(phy_id0, phy_id1) == PHY_BCM54182_MODEL) {
        p_port->probed = TRUE;
        printk("_ext_phy_probe port %d BCM54182 probed\n", port);
    } else {
        printk("_ext_phy_probe port %d - phy_addr 0x%x  id0 0x%x id1 0x%x\n",port, phy_addr, phy_id0, phy_id1);
    }
    
    return 0;
}



/*****************************************************************************************/
/*                             BCM542XX Init/Setup                                       */
/*****************************************************************************************/
#if 0
static int
phy_bcm542xx_ability_local_get(int unit, soc_port_t port, soc_port_ability_t *ability)
{
    //phy_ctrl_t  *pc;

    if (NULL == ability) {
        return SOC_E_PARAM;
    }
    //pc = EXT_PHY_SW_STATE(unit, port);
    memset(ability, 0,  sizeof(soc_port_ability_t)); /* zero initialize */

    if ( PHY_COPPER_MODE(unit, port) ) {
        ability->speed_half_duplex  = SOC_PA_SPEED_100MB | SOC_PA_SPEED_10MB;
        ability->speed_full_duplex  = SOC_PA_SPEED_1000MB |
                                      SOC_PA_SPEED_100MB |
                                      SOC_PA_SPEED_10MB;

        ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
        ability->interface = SOC_PA_INTF_SGMII;
        ability->medium    = SOC_PA_MEDIUM_COPPER;
        ability->loopback  = SOC_PA_LB_PHY;
        ability->flags     = SOC_PA_AUTONEG;

        /* EEE settings */
        if ( PHY_FLAGS_TST(unit, port, PHY_FLAGS_EEE_CAPABLE) ) {
                ability->eee |= SOC_PA_EEE_1GB_BASET | SOC_PA_EEE_100MB_BASETX;
        }

        if ( pc->automedium ) {
            ability->flags     |= SOC_PA_COMBO;
        }
    } else if ( PHY_FIBER_MODE(unit, port) ) {
        ability->speed_half_duplex  = SOC_PA_SPEED_100MB;
        ability->speed_full_duplex  = SOC_PA_SPEED_1000MB |SOC_PA_SPEED_100MB;

        ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
        ability->interface = SOC_PA_INTF_SGMII;
        ability->medium    = SOC_PA_MEDIUM_FIBER;
        ability->loopback  = SOC_PA_LB_PHY;
        ability->flags     = SOC_PA_AUTONEG;

        if ( pc->automedium ) {
            ability->flags     |= SOC_PA_COMBO;
        }

    } else {
        return SOC_E_INTERNAL;
    }

    LOG_INFO(BSL_LS_SOC_PHY,
             (BSL_META_U(unit, "phy_bcm542xx_ability_local_get: u=%d p=%d "
                               "ability_hd_speed=0x%x, ability_fd_speed=0x%x, "
                               "ability_pause=0x%x\n"),
              unit, port, ability->speed_half_duplex,
              ability->speed_full_duplex, ability->pause));
    return ( SOC_E_NONE );
}

int
soc_phyctrl_ability_local_get(int unit, soc_port_t port,
                             soc_port_ability_t *ability)
{
    int                 rv;
    soc_port_ability_t  ability_int;
    soc_port_ability_t  ability_ext;
    phy_ctrl_t         *int_pc;
    phy_ctrl_t         *ext_pc;

    SOC_NULL_PARAM_CHECK(ability);

    LOG_VERBOSE(BSL_LS_SOC_PHY,
                (BSL_META_U(unit,
                            "entered soc_phyctrl_ability_local_get: "
                            "unit %d, port %d\n"), unit, port));
    
    //ext_pc = EXT_PHY_SW_STATE(unit, port);
    //int_pc = INT_PHY_SW_STATE(unit, port);
    //SOC_PHYCTRL_INIT_CHECK(ext_pc, int_pc);

    ability_int.speed_half_duplex = ability_ext.speed_half_duplex = SOC_PA_SPEED_ALL;
    ability_int.speed_full_duplex = ability_ext.speed_full_duplex = SOC_PA_SPEED_ALL;

    rv = SOC_E_NONE;
    if (NULL != int_pc) {

        rv = _soc_phy_ability_local_get(unit, port, int_pc->pd, ability);
        ability_int.speed_full_duplex = ability->speed_full_duplex;
        ability_int.speed_half_duplex = ability->speed_half_duplex; 
    }

    if (SOC_SUCCESS(rv) && NULL != ext_pc) {
        /* next make sure it's not phy_null_driver */
        if (ext_pc->write != NULL) {
            ability->speed_half_duplex = ability->speed_full_duplex = 0;
            rv = _soc_phy_ability_local_get(unit, port, ext_pc->pd, ability);
            ability_ext.speed_full_duplex = ability->speed_full_duplex;
            ability_ext.speed_half_duplex = ability->speed_half_duplex;
        }
    }

    if (SOC_SUCCESS(rv)) {
        ability->speed_half_duplex = ability_int.speed_half_duplex & ability_ext.speed_half_duplex;
        ability->speed_full_duplex = ability_int.speed_full_duplex & ability_ext.speed_full_duplex;  
    }

    if (SOC_FAILURE(rv)) {
        PHYCTRL_WARN(("soc_phyctrl_ability_get failed %d\n", rv));
    }
    return rv;
}


//Might not need, verify register read
static int
_bcm_port_ability_local_get(struct bcmsw_switch *bcmsw_sw, int port,
                           bcm_port_ability_t *ability_mask)
{
    soc_port_ability_t             mac_ability, phy_ability;
    soc_pa_encap_t                 encap_ability;

    memset(&phy_ability, 0, sizeof(soc_port_ability_t));
    memset(&mac_ability, 0, sizeof(soc_port_ability_t));

    SOC_IF_ERROR_RETURN
        (soc_phyctrl_ability_local_get(unit, port, &phy_ability));

    if (!(IS_TDM_PORT(unit, port))) {
        SOC_IF_ERROR_RETURN
            (MAC_ABILITY_LOCAL_GET(PORT(unit, port).p_mac, unit,
                                   port, &mac_ability));
    }

    /* Combine MAC and PHY abilities */
    {
        ability_mask->speed_half_duplex =
            mac_ability.speed_half_duplex & phy_ability.speed_half_duplex;
    }
    ability_mask->speed_full_duplex =
        mac_ability.speed_full_duplex & phy_ability.speed_full_duplex;
    ability_mask->pause     = mac_ability.pause & phy_ability.pause;
    if (phy_ability.interface == 0) {
        ability_mask->interface = mac_ability.interface;
    } else {
        ability_mask->interface = phy_ability.interface;
    }
    ability_mask->medium    = phy_ability.medium;
    ability_mask->channel   = phy_ability.channel;
    ability_mask->fec       = phy_ability.fec;
    /* mac_ability.eee without phy_ability.eee makes no sense */
    ability_mask->eee    = phy_ability.eee;
    ability_mask->loopback  = mac_ability.loopback | phy_ability.loopback |
                               BCM_PORT_ABILITY_LB_NONE;
    ability_mask->flags     = mac_ability.flags | phy_ability.flags;

    /* Get port encapsulation ability */
    encap_ability = mac_ability.encap;

    //if ((soc_feature(unit, soc_feature_embedded_higig))
    //    && IS_E_PORT(unit, port)) {
    //    encap_ability |= BCM_PA_ENCAP_HIGIG2_L2;
    //    encap_ability |= BCM_PA_ENCAP_HIGIG2_IP_GRE;
    //}

    if (IS_HL_PORT(unit, port) &&
         (SOC_IS_ENDURO(unit) || SOC_IS_HURRICANEX(unit) ||
          SOC_IS_KATANA2(unit) )) {
        encap_ability |= BCM_PA_ENCAP_HIGIG2;
        /* should be HiGig-Lite, not Higig2 */
        encap_ability |= BCM_PA_ENCAP_HIGIG2_LITE;
    }

    ability_mask->encap = encap_ability;
    return BCM_E_NONE;
}
#endif

//Enable or disable the physical interface.
static int
phy_bcm542xx_enable_set(port_info_t *pport, int port, uint16_t phy_addr, int enable)
{
    uint16    power = (enable) ? 0 : PHY_BCM542XX_MII_CTRL_PWR_DOWN;

    phy_bcm542xx_reg_modify(phy_addr, 0x0000,  PHY_BCM542XX_MII_CTRL_REG, power, PHY_BCM542XX_MII_CTRL_PWR_DOWN);

    /* Update software state */
    phy_fe_ge_enable_set(pport, port, enable);


    return 0;
}

static int 
phy_bcm542xx_enable_get(port_info_t *pport,  int *enable)
{
    return phy_fe_ge_enable_get(pport, enable);
}

//Reset PHY and wait for it to come out of reset.
static int
phy_bcm542xx_reset(int port, uint16_t phy_addr)
{
    int rv;

    rv = phy_fe_ge_reset(port, phy_addr);

    return ( rv );
}

static int phy_bcm542xx_dev_init(struct bcmsw_switch *bcmsw_sw, int port)
{
    //_phy_bcm54282_init
    soc_info_t  *si = bcmsw_sw->si;
    port_info_t *p_port;
    uint16_t     phy_id;

    p_port = &si->ports[port];

    /*
    * Configure extended control register for led mode and
    * jumbo frame support
    */
    phy_id = p_port->dev_desc.phy_id_base; /* RV */

    /* Enable QSGMII MDIO sharing feature
       - Enable access to QSGMII reg space using port3's MDIO address
       - Use PHYA[4:0]+3 (Port3's MDIO) instead of PHYA[4:0)+8 as MDIO address.
       - It saves one MDIO address for customer.
       - Access to this top level register via port 0 only
     */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                            PHY_BCM542XX_TOP_MISC_TOP_CFG_REG_OFFSET,
                            PHY_BCM542XX_TOP_MISC_CFG_REG_QSGMII_SEL
                          | PHY_BCM542XX_TOP_MISC_CFG_REG_QSGMII_PHYA,
                            PHY_BCM542XX_TOP_MISC_CFG_REG_QSGMII_SEL
                          | PHY_BCM542XX_TOP_MISC_CFG_REG_QSGMII_PHYA);

    /* replace it with qsgmii phy id */
    //PHY_BCM542XX_SLICE_ADDR
    phy_id = p_port->dev_desc.phy_slice;
    if (p_port->dev_desc.flags & PHY_BCM542XX_PHYADDR_REVERSE) {
        phy_id = p_port->dev_desc.phy_id_base - 3;
    } else {
        phy_id = p_port->dev_desc.phy_id_base + 3;
    }    

    /* QSGMII FIFO Elasticity */
    phy_bcm542xx_qsgmii_reg_write(phy_id,
                                  p_port->dev_desc.phy_slice /*dev_port*/,
                                  0x8300, 0x12, 0x0006);

    /* Restore access to Copper/Fiber register space.
     *  TOP lvl register, access through port0 only
     */
    phy_id = p_port->dev_desc.phy_id_base;
    phy_bcm542xx_rdb_reg_modify(phy_id,
                            PHY_BCM542XX_TOP_MISC_TOP_CFG_REG_OFFSET,
                            0,
                            PHY_BCM542XX_TOP_MISC_CFG_REG_QSGMII_SEL
                          | PHY_BCM542XX_TOP_MISC_CFG_REG_QSGMII_PHYA);
    /* Restore the phy mdio address */
    phy_id =  p_port->dev_desc.phy_id_orig;

    /* system side QSGMII AutoNeg mode */
    //if ( soc_property_port_get(unit, port, spn_PHY_SGMII_AUTONEG, TRUE) ) {
    //    PHY_BCM542XX_SYS_SIDE_AUTONEG_SET(pc);
    //} else {
    //    PHY_BCM542XX_SYS_SIDE_AUTONEG_CLR(pc);
    //}
    p_port->dev_desc.flags &= ~PHY_BCM542XX_SYS_SIDE_AUTONEG;

    /* Configure LED mode and turn on jumbo frame support */
    //TODO
    //_phy_bcm54280_init(unit, port);

    /* In case of QSGMII devices, there is a LPI pass through mode
       which has to be enabled for native EEE to work. In case of VNG,
       it is already set by default. To enable it:
    SOC_IF_ERROR_RETURN
       (phy_bcm542xx_qsgmii_reg_write(PHY_BCM54282_QSGMII_DEV_ADDR(_pc),
                                      (int)PHY_BCM542XX_DEV_PHY_SLICE(pc),
                                      0x833e, 0x0e, 0xc000));
    */
    return SOC_E_NONE;    
}

static int 
phy_bcm542xx_reset_setup(struct bcmsw_switch *bcmsw_sw,
    int port,
    int automedium,
    int fiber_preferred,
    int fiber_detect,
    int fiber_enable,
    int copper_enable,
    int ext_phy_autodetect_en,
    int ext_phy_fiber_iface)
{
    int     primary_and_offset;
    int     primary_port;
    int     index;
    int     oui = 0, model = 0, rev = 0;
    soc_info_t  *si = bcmsw_sw->si;
    port_info_t *p_port;
    uint32_t     phy_addr;

    p_port = &si->ports[port];

    //TODO, ext addr only
    phy_addr = p_port->ext_phy_addr;

    primary_and_offset = p_port->primary_and_offset;

    /* Set primary port & offset */
    primary_port = (primary_and_offset >> 8) & 0xffff;
    index = primary_and_offset & 0xff;

    if ( index & 0x80 ) {
        p_port->dev_desc.flags |=  PHY_BCM542XX_PHYADDR_REVERSE;
    } else {
        p_port->dev_desc.flags &= ~PHY_BCM542XX_PHYADDR_REVERSE;
    }

    /* Do not change the order */
    index &= ~0x80; /* clear reverse bit (PHYA_REV) */
    p_port->dev_desc.phy_id_orig = p_port->ext_phy_addr;
    p_port->dev_desc.phy_slice = index;
    /* phy address of port 0 */
    if (p_port->dev_desc.flags & PHY_BCM542XX_PHYADDR_REVERSE) {
        p_port->dev_desc.phy_id_base = p_port->dev_desc.phy_id_orig + p_port->dev_desc.phy_slice;
    } else {
        p_port->dev_desc.phy_id_base =  p_port->dev_desc.phy_id_orig - p_port->dev_desc.phy_slice;
    }

    /* workaround for 50140/54140/5018x/5418x/5019x/5419x : restarting AFE PLL *\
     *      after the power rails are stable, to fix the issue of AFE PLL lock */
    //if (  PHY_IS_BCM5418x_19x(pc) &&
    if((PHY_QSGMII0_HEAD == index) || (PHY_QSGMII1_HEAD == index)) {
        int  qii = 0;
        /* reset PLL of port 0-3/4-7 in one-shot before initializing port 0/4 */
        for ( qii = 0; qii < PHY_PORTS_PER_QSGMII; qii++ ) {
            /* RDB_reg.0x015[1]   enable Auto-Power-Down DLL */
            phy_bcm542xx_rdb_reg_modify(phy_addr+qii, 0x015, 0, BCM542XX_AUTO_PWR_DOWN_DLL_DIS);

            /* Reg.0x00[11]  Power down copper interface */
            phy_bcm542xx_reg_modify(phy_addr+qii, 0x0000, PHY_BCM542XX_MII_CTRL_REG, PHY_BCM542XX_MII_CTRL_PWR_DOWN, PHY_BCM542XX_MII_CTRL_PWR_DOWN);

            /* Reg.0x00[11]  Power up   copper interface */
            phy_bcm542xx_reg_modify(phy_addr+qii, 0x0000,  PHY_BCM542XX_MII_CTRL_REG, 0, PHY_BCM542XX_MII_CTRL_PWR_DOWN);

            /* RDB_reg.0x015[1]  disable Auto-Power-Down DLL */
            phy_bcm542xx_rdb_reg_modify(phy_addr+qii, 0x015,BCM542XX_AUTO_PWR_DOWN_DLL_DIS, BCM542XX_AUTO_PWR_DOWN_DLL_DIS);
        }
    }


    /* PHY reset */
    phy_bcm542xx_reset(port, phy_addr);

    if ( !(p_port->phy_flags & PHY_FLAGS_INIT_DONE) ) {
        /* Reset the Top level register block  (RDB Reg.0x800-0xAFF) *\
        \* during the 1st initialization only.                       */
        phy_bcm542xx_rdb_reg_modify(phy_addr,
                              PHY_BCM542XX_TOP_MISC_GLOBAL_RESET_OFFSET,
                              PHY_BCM542XX_TOP_MISC_GLOBAL_RESET_TOP_MII_SOFT |
                              PHY_BCM542XX_TOP_MISC_GLOBAL_RESET_TIMESYNC,
                              PHY_BCM542XX_TOP_MISC_GLOBAL_RESET_TOP_MII_SOFT |
                              PHY_BCM542XX_TOP_MISC_GLOBAL_RESET_TIMESYNC);
    }

    /* get the model & revision ID of PHY chip */
    //SOC_IF_ERROR_RETURN(
    //        _phy_bcm542xx_model_rev_get(unit, pc,&oui, &model, &rev));
    //if ( PHY_IS_BCM5418x_14x(pc) ) {
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x01E7, 0xA008);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x0028, 0x0C30);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x00D8, 0x0020);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x012D, 0x0352);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x012E, 0xA04D);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x0164, 0x0500);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x0165, 0x7859);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x0125, 0x091B);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x01E8, 0x00C3);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x01E9, 0x00CC);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x01EA, 0x0300);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x01E2, 0x02E3);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x01E3, 0x7FC1);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x01E5, 0xA7AB);
    phy_bcm542xx_rdb_reg_write(phy_addr, 0x0028, 0x0430);


    //BCM54182
    /* DISable dsp clock for 5414x / 5418x / 5419x */
    phy_bcm542xx_rdb_reg_write(phy_addr, PHY_BCM542XX_AUX_CTRL_REG_OFFSET, 0x0430);
    /* Enhanced Amplitude and AB Symmetry */
    phy_bcm542xx_rdb_reg_write(phy_addr, PHY_BCM542XX_DSP_TAP10_REG_OFFSET, 0x091B);

    /* config property PHY_OPERATIONAL_MODE == 0x80 or 0x8000 or 0x8080    *\
    \*        means to keep Super Isolate setting (depend on HW strap pin) */
    //if ( 0 == (PHY_RETAIN_SUPER_ISOLATE &
    //           soc_property_port_get(unit, port, spn_PHY_OPERATIONAL_MODE, 0)) ) {
    /* Remove  Super Isolate */
    phy_bcm542xx_rdb_reg_modify(phy_addr,
                                PHY_BCM542XX_POWER_MII_CTRL_REG_OFFSET, 0x0,
                                PHY_BCM542XX_POWER_MII_CTRL_SUPER_ISOLATE);

    /* Enable current mode LED */
    phy_bcm542xx_rdb_reg_write(phy_addr,
                               PHY_BCM542XX_LED_GPIO_CTRL_STATUS_REG_OFFSET,
                               0xBC00);

    phy_bcm542xx_dev_init(bcmsw_sw, port);

    return SOC_E_NONE;

}

static int
phy_bcm542xx_init_setup( struct bcmsw_switch *bcmsw_sw,
                         int port,
                         int reset,
                         int automedium,
                         int fiber_preferred,
                         int fiber_detect,
                         int fiber_enable,
                         int copper_enable,
                         int ext_phy_autodetect_en,
                         int ext_phy_fiber_iface)
{
    soc_info_t  *si = bcmsw_sw->si;
    port_info_t *p_port;
    uint32_t     phy_id;
    int          dev_port;

    p_port = &si->ports[port];

    if ( reset ) {
        phy_bcm542xx_reset_setup(bcmsw_sw, port, automedium,
                                 fiber_preferred, fiber_detect,
                                 fiber_enable,
                                 copper_enable,
                                 ext_phy_autodetect_en,
                                 ext_phy_fiber_iface);
    } 
    //else {
    //    SOC_IF_ERROR_RETURN(
    //        phy_bcm542xx_dev_init(   unit, port, automedium,
    //                                 fiber_preferred, fiber_detect,
    //                                 fiber_enable,
    //                                 copper_enable,
    //                                 ext_phy_autodetect_en,
    //                                 ext_phy_fiber_iface));
    //    // called by _phy_bcm542xx_no_reset_setup() or _phy_bcm542xx_medium_change() 
    //    //   no need to do the remaining RESET process below                           
    //    return SOC_E_NONE;
    //}

    //if ( SOC_WARM_BOOT(unit) || SOC_IS_RELOADING(unit) ) {
    //    return SOC_E_NONE;
    //}

    //dev_port = PHY_BCM542XX_DEV_PHY_SLICE(pc);

    /* Reset EEE to default state i.e disable */

    /* Native */
    /* Disable LPI feature */
    //SOC_IF_ERROR_RETURN(
    //        PHY_MODIFY_BCM542XX_EEE_803Dr(unit, pc, 0x0000, 0xC000) );
    /* Reset counters and other settings */
    //SOC_IF_ERROR_RETURN(
    //        PHY_MODIFY_BCM542XX_EEE_STAT_CTRLr(unit, pc, 0x0000, 0x3fff) );

    /* AutogrEEEn */
    phy_id = p_port->dev_desc.phy_id_base;
    dev_port = p_port->dev_desc.phy_slice;
    /* Disable AutogrEEEn and reset other settings */
    phy_bcm542xx_rdb_reg_write(phy_id,
                            PHY_BCM542XX_TOP_MISC_MII_BUFF_CNTL_PHYN(dev_port),
                            0x8000);
    return SOC_E_NONE;
}


static int phy_bcm542xx_init(struct bcmsw_switch *bcmsw_sw, int port)
{
    int  fiber_capable = 0;
    int  automedium = 0;
    int  fiber_detect = 0;
    int  fiber_enable = 0;
    int  fiber_preferred = 0;
    int  copper_enable = TRUE;
    int  ext_phy_autodetect_en = 0;
    int  ext_phy_fiber_iface = -1;

    /* Init PHYS and MACs to defaults */
    phy_bcm542xx_init_setup(bcmsw_sw, port, 
                            TRUE,
                            automedium,
                            fiber_preferred,
                            fiber_detect,
                            fiber_enable,
                            copper_enable,
                            ext_phy_autodetect_en,
                            ext_phy_fiber_iface);

    /* Set LED Modes and Control */      
    
    //_phy_bcm542xx_medium_config_update
}

/*****************************************************************************************/
/*                             Port Init/Setup                                           */
/*****************************************************************************************/

/*****************************************************************************************/
/*                             Port Init/Setup -- PHY                                    */
/*****************************************************************************************/


static int 
_phyctrl_probe(struct bcmsw_switch *bcmsw_sw, int port)
{
    soc_info_t *si = bcmsw_sw->si;

    // probe for PHY , and dump information
    // Only do ext Phy (N3248TE)
    if (si->ports[port].ext_phy_addr !=  -1) {
        _ext_phy_probe(bcmsw_sw, port);
    }

    return 0;
}

static int _phyctrl_init(struct bcmsw_switch *bcmsw_sw, int port)
{
    soc_info_t *si = bcmsw_sw->si;
    port_info_t *p_port;

    p_port = &si->ports[port];

    printk("entered soc_phyctrl_init: port %d , probed %d\n", port, p_port->probed);

    if(p_port->probed) {  
       //Call Phy Model specific init -> pd_init = phy_bcm542xx_init
       phy_bcm542xx_init(bcmsw_sw, port);

       p_port->phy_flags |= PHY_FLAGS_INIT_DONE;
    }

    return 0;
}

static int 
_phyctrl_pbm_probe_init(struct bcmsw_switch *bcmsw_sw)
{
    soc_info_t *si = bcmsw_sw->si;
    int port; 
    int num_port = HX5_NUM_PORT;

    for (port = 0; port < num_port; port++) {
        if(si->ports[port].valid == TRUE) {
            _phyctrl_probe(bcmsw_sw, port);

            /* do PHY init pass1 */
            PHYCTRL_INIT_STATE_SET(&(si->ports[port].phy_ctrl), PHYCTRL_INIT_STATE_PASS1);
            _phyctrl_init(bcmsw_sw, port);

            // BCM54182 only need init pass 1
            /* do PHY init pass2 - 5  if requested */
          
            PHYCTRL_INIT_STATE_SET(&(si->ports[port].phy_ctrl), PHYCTRL_INIT_STATE_DEFAULT);
        }
    }
}

/*****************************************************************************************/
/*                             Port Ctrl                                                 */
/*****************************************************************************************/

static int _pm4x10_qtc_port_enable_set(struct bcmsw_switch *bcmsw_sw, int port, int enable)
{
    soc_info_t *si = bcmsw_sw->si;

    port_info_t *pport = &si->ports[port];

    return phy_bcm542xx_enable_set(pport, port, pport->ext_phy_addr, enable);
}

static int 
_bcm_esw_portctrl_enable_set(struct bcmsw_switch *bcmsw_sw, int port, int enable)
{
    //if (flags & PORTMOD_PORT_ENABLE_PHY) {
        //portmod_port_enable_set();
        _pm4x10_qtc_port_enable_set(bcmsw_sw, port, 1);
    //}

    portmod_ext_to_int_cmd_set() 

    unimac_reset_check

    _soc_link_update
    //Check if MAC needs to be modified based on whether
    //(portmod_port_mac_reset_check(unit, pport,
    //    enable, &mac_reset));

    //if (flags & PORTMOD_PORT_ENABLE_MAC ) {


    //}

    return 0;
}

static int 
bcmi_esw_portctrl_probe()
{

    /* Add port to PM */
//portmod_xphy_lane_detach(unit, physical_port+lane, 1);
//PORT_UNLOCK(unit);
//PORTMOD_PBMP_PORT_ADD(p_pbmp, physical_port+lane);
//rv = soc_esw_portctrl_setup_ext_phy_add(unit, port, &p_pbmp);

//rv = soc_esw_portctrl_add(unit, port, init_flag, add_info);
   return 0;
}

static int
bcmi_esw_portctrl_probe_pbmp(struct bcmsw_switch *bcmsw_sw)
{

    /*step1: probe Serdes and external PHY core*/
    //bcmi_esw_portctrl_probe

    /*step2 : initialize PASS1 for SerDes and external PHY*/

    /* step3:broadcast firmware download for all external phys inculde legacy and Phymod PHYs*/

    /*step4:initialize PASS2 for Serdes and external PHY*/


    //_bcm_esw_portctrl_enable_set(unit, port, pport,PORTMOD_PORT_ENABLE_MAC, FALSE);
}


/*****************************************************************************************/
/*                             Port Init/Setup                                           */
/*****************************************************************************************/
static int
_bcm_port_speed_set(struct bcmsw_switch *bcmsw_sw, int port, int speed)
{
    //bcmi_esw_portctrl_speed_set
    //if (enable == TRUE) {
    // disable MAC and PHY
    //bcmi_esw_portctrl_enable_set

    /* disable AN */
    // _bcm_esw_portctrl_disable_autoneg

    //reconfigure chip
    //_bcm_esw_portctrl_speed_chip_reconfigure

    //Restore port's enable state based on what was read prior to setting speed 

    return 0;
}

//Setting the speed for a given port
int
bcm_esw_port_speed_set(struct bcmsw_switch *bcmsw_sw, int port, int speed)
{
    int rv;
    rv = _bcm_port_speed_set(bcmsw_sw, port, speed);

    //bcm_esw_port_enable_set
    //bcm_esw_link_change
    //_bcm_esw_port_link_delay_update

    return rv;
}
int bcm_esw_port_autoneg_set(struct bcmsw_switch *bcmsw_sw, int port, int speed)
{
    //bcmi_esw_portctrl_autoneg_set
#if 0
    _bcm_esw_port_gport_phyn_validate(unit, port,
        &local_port, &phyn,
        &phy_lane, &sys_side));

    if (local_port != -1) {
        port = local_port;
    }

    if (local_port == -1) {
        /* Configure outermost PHY (common case) */
        rv = portmod_port_autoneg_set(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an);
    } else {
        /* Configure PHY specified by GPORT */
        rv = portmod_port_redirect_autoneg_set(unit, pport, phyn,
                                               phy_lane, sys_side, &an);
    }
#endif   
    return 0;                                            
}


int
bcm_esw_port_enable_set(struct bcmsw_switch *bcmsw_sw, int port, int enable)
{
   //bcmi_esw_portctrl_enable_set
   //if (enable) {
      // enable PHY
      //rv = _bcm_esw_portctrl_enable_set(bcmsw_sw, port, PORTMOD_PORT_ENABLE_PHY, TRUE);

      /* Get link status after PHY state has been set */
      //rv = bcm_esw_port_link_status_get(unit, port, &link);

      // enable MAC
      //rv = _bcm_esw_portctrl_enable_set(bcmsw_sw, port, PORTMOD_PORT_ENABLE_MAC, TRUE);
   //} else {


   //}
   return _bcm_esw_portctrl_enable_set(bcmsw_sw, port, enable);
}

static int
bcm_port_settings_init(struct bcmsw_switch *bcmsw_sw, int port)
{
    int             val, rc;
    soc_info_t *si = bcmsw_sw->si;

    rc = bcm_esw_port_speed_set(bcmsw_sw, port, si->port_init_speed[port]);

    //val = soc_property_port_get(unit, port, spn_PORT_INIT_DUPLEX, -1);
    //if (val != -1) {
    //    info.duplex = val;
    //    info.action_mask |= BCM_PORT_ATTR_DUPLEX_MASK;
    //}
    //bcm_esw_port_duplex_set(unit, port, info->duplex);
    

    //val = soc_property_port_get(unit, port, spn_PORT_INIT_ADV, -1);
    //if (val != -1) {
    //    info.local_advert = val;
    //    info.action_mask |= BCM_PORT_ATTR_LOCAL_ADVERT_MASK;
    //}
    //bcm_esw_port_advert_set(unit, port, info->local_advert);

    rc = bcm_esw_port_autoneg_set(bcmsw_sw, port, TRUE);

    return rc;
}


static int 
_port_init(struct bcmsw_switch *bcmsw_sw)
{
    int num_port = HX5_NUM_PORT, port, vid; 
    soc_info_t *si = bcmsw_sw->si;

    vid = BCMSW_VLAN_DEFAULT;
    for (port = 0; port < num_port; port++) {
       if(si->port_type[port] != -1) {
           //bcm_td3_port_cfg_init
           _port_cfg_init(bcmsw_sw, port, vid);
	  
       }
    }
    // STEP 1 
    /* 
     * Initialize inner TPID
     * (WAR)Set to 0x9100 then change back to take effort
     */
    //TODO


    // STEP 2 
    // clear egress port blocking table
    //TODO

    // STEP 3 - Probe for Ports -> bcm_esw_port_probe
    /* Probe the PHY and set up the PHY and MAC for the specified ports.
     * bcm_esw_port_probe(unit, PBMP_PORT_ALL(unit), &okay_ports);
     */
    /* Check for PortMod */
    bcmi_esw_portctrl_probe_pbmp(bcmsw_sw);

    /*soc_phyctrl_pbm_probe_init */
    _phyctrl_pbm_probe_init(bcmsw_sw);

    // Probe function should leave port disabled 
    //soc_phyctrl_enable_set

    // STEP 4
    for (port = 0; port < num_port; port++) {
        if(si->ports[port].valid == TRUE) {
            bcm_port_settings_init(bcmsw_sw, port);
        }
     }    

    //enable ports
    // if ((rv = bcm_esw_port_enable_set(unit, p, port_enable)) < 0) {
    bcm_esw_port_enable_set(bcmsw_sw, port, TRUE);
    return 0;
}

#define HX5_NUM_PHY_PORT                (79)
#define HX5_NUM_PORT                    (72)
#define HX5_NUM_MMU_PORT                (72)


static int bcmsw_port_create(struct bcmsw_switch *bcmsw_sw, int port_index, const char *name)
{
    int len;
    //int qnum;
    kcom_msg_netif_create_t netif_create;
    //struct bcmsw_port * port = bcmsw_sw->ports[port_index];

    memset(&netif_create, 0, sizeof(netif_create));
    netif_create.hdr.type = KCOM_MSG_TYPE_CMD;
    netif_create.hdr.seqno = 0;
    netif_create.hdr.opcode = KCOM_M_NETIF_CREATE;
    netif_create.hdr.unit = 0;

    netif_create.netif.type = KCOM_NETIF_T_VLAN;

    /*    if (netif->flags & BCM_KNET_NETIF_F_ADD_TAG) {
        netif_create.netif.flags |= KCOM_NETIF_F_ADD_TAG;
    }
    if (netif->flags & BCM_KNET_NETIF_F_RCPU_ENCAP) {
        netif_create.netif.flags |= KCOM_NETIF_F_RCPU_ENCAP;
    }
    if (netif->flags & BCM_KNET_NETIF_F_KEEP_RX_TAG) {
        netif_create.netif.flags |= KCOM_NETIF_F_KEEP_RX_TAG;
    }
    */
    //netif_create.netif.cb_user_data = netif->cb_user_data;
    netif_create.netif.vlan = BCMSW_VLAN_DEFAULT; //netif->vlan;
    netif_create.netif.port = port_index;
    netif_create.netif.phys_port = bcmsw_sw->si->port_l2p_mapping[port_index];

    //if (BCM_SUCCESS(soc_esw_hw_qnum_get(unit, netif->port, netif->cosq, &qnum))) {
    //    netif_create.netif.qnum = qnum;
    //}
    //memcpy(netif_create.netif.macaddr, netif->mac_addr, 6);
    memcpy(netif_create.netif.name, name,
               sizeof(netif_create.netif.name) - 1);

    len = bkn_handle_cmd_req((kcom_msg_t *)&netif_create, sizeof(netif_create));

    //handle response
    //if (len > 0) {
    //    /* ID and interface name are assigned by kernel */
    //    port->id = netif_create.netif.id;
    //    memcpy(port->name, netif_create.netif.name, KCOM_NETIF_NAME_MAX - 1);
    //}

    return 0;
}



//TODO - this part might need to be moved to userspace 
static int bcmsw_ports_init(struct bcmsw_switch *bcmsw_sw)
{
    //FIX: hardcode for N3248TE  BCM56371
    // soc_helix5_port_config_init
    // bcm_td3_port_cfg_init
    // bcm_esw_port_probe
    // bcm_esw_port_enable_set
    // _bcm_td_port_lanes_set
    //bcm_esw_knet_netif_create
    int max_ports;
    int err;
    int i;
    soc_info_t *si;


    si = kzalloc(sizeof(soc_info_t), GFP_KERNEL);
    if (!si) {
        return -ENOMEM;
    }
    // initializa soc_info according to hardware information,soc_info_config & soc_helix5_port_config_init
    bcmsw_soc_info_init(si);
    bcmsw_sw->si = si;

    // initialize port configuration according to soc info. bcm_esw_port_init bcm_td3_port_cfg_init
    err = _port_init(bcmsw_sw);
    if (err) {
        goto err_port_cfg_init;
    }

    //call knet to create interface
    max_ports = COUNTOF(n3248te_ports);
    for (i = 0; i < max_ports; i++) {
        if (n3248te_ports[i].port != -1) {
    	  err = bcmsw_port_create(bcmsw_sw, n3248te_ports[i].port, n3248te_ports[i].name);
    	  if (err)
    		goto err_port_create;
        }
    }
    
err_port_cfg_init:
err_port_create:
	return err;
}

/*****************************************************************************************/
/*                             switch                                                    */
/*****************************************************************************************/
//soc_cmicx_dma_abort
#if 0
static int bcmsw_cmicx_dma_abort(struct bcmsw_switch *bcmsw_sw)
{

	    /* abort s-bus DMA in all channels */
    chans_group_init(&sbus_channels);
    if ((flags & SOC_DMA_ABORT_SKIP_SBUS) == 0) {
        /* For  CMCs x=0-1  and channels y=0-3 and [channel used by this CPU] */
        for (cmc = 0; cmc < cmc_num_max; ++cmc) {
            for (chan = 0; chan < sbusdma_chan_max; ++chan) {
                cmic_address = CMIC_CMCx_SBUSDMA_CHy_CONTROL(cmc, chan);
                /* if CMIC_CMCx_SBUSDMA_CHy_CONTROL.START == 1 */
                val = soc_pci_read(unit, cmic_address);
                if (val & CMIC_CMCx_SBUSDMA_CHy_CONTROL_START) {
                    /* set CMIC_CMCx_SBUSDMA_CHy_CONTROL.ABORT=1 to abort */
                    soc_pci_write(unit, cmic_address, val | CMIC_CMCx_SBUSDMA_CHy_CONTROL_ABORT);
                    chans_group_insert(&sbus_channels, cmc, chan); /* mark the channel to be waited on */
                    LOG_DEBUG(BSL_LS_SOC_DMA, (BSL_META_U(unit, "Aborting s-bus DMA CMC %d channel %d\n"), cmc, chan));
                }
            }
        }
    }


    /* abort s-chan FIFO in all channels, s-chan FIFO is not per CMC */
    chans_group_init(&schan_fifo_channels);
    if ((flags & SOC_DMA_ABORT_SKIP_SCHAN_FIFO) == 0) {
        /* For channels y=0-1 and [channels used by this CPU] */
        for (chan = 0; chan < CMIC_SCHAN_FIFO_NUM_MAX; ++chan) {
            cmic_address = CMIC_SCHAN_FIFO_CHx_CTRL(chan);
            /* if CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_CTRL.START == 1 */
            val = soc_pci_read(unit, cmic_address);
            if (val & SCHAN_FIFO_CTRL_START) {
                /* set CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_CTRL.ABORT=1 to abort */
                soc_pci_write(unit, cmic_address, val | SCHAN_FIFO_CTRL_ABORT);
                chans_group_insert(&schan_fifo_channels, 0, chan); /* mark the channel to be waited on */
                LOG_DEBUG(BSL_LS_SOC_DMA, (BSL_META_U(unit, "Aborting s-chan FIFO channel %d\n"), chan));
            }
        }
    }


    /* loop and check that each abort finished. When it finished or after time out, clear the operation and disable the DMA */
            /* for all s-bus DMA channels still not done */
        for (chans_group_iter_start(&sbus_channels, &channel_iter); !chans_group_is_end(&sbus_channels, channel_iter); ++channel_iter) {
            chans_group_iter_t_get(channel_iter, &cmc, &chan); /* get the channel to work on */
            /* If the abort is done (CMIC_CMCx_SBUSDMA_CHy_STAT.DONE==1), then clear the operation */
            done = soc_pci_read(unit, CMIC_CMCx_SBUSDMA_CHy_STATUS(cmc, chan)) & CMIC_CMCx_SBUSDMA_CHy_STATUS_DONE;
            if (done || timeout_state == abort_timeout_passed) {
                /* clear CMIC_CMCx_SBUSDMA_CHy_CONTROL.ABORT|START in the same write disables the original operation and abort, and clears statuses */
                cmic_address = CMIC_CMCx_SBUSDMA_CHy_CONTROL(cmc, chan);
                val = soc_pci_read(unit, cmic_address);
                soc_pci_write(unit, cmic_address, val & ~(CMIC_CMCx_SBUSDMA_CHy_CONTROL_ABORT |CMIC_CMCx_SBUSDMA_CHy_CONTROL_START ));
                if (done) { /* remove the channel from the channels waited on */
                    chans_group_delete(&sbus_channels, channel_iter--);
                } else {
                    LOG_ERROR(BSL_LS_SOC_DMA, (BSL_META_U(unit, "Failed to abort s-bus DMA in CMC %d channel %d, check with the design team\n"), cmc, chan));
                    ++nof_failures;
                }
            }
        }
        /* for all s-chan FIFO channels still not done */
        for (chans_group_iter_start(&schan_fifo_channels, &channel_iter); !chans_group_is_end(&schan_fifo_channels, channel_iter); ++channel_iter) {
            chans_group_iter_t_get(channel_iter, &cmc, &chan); /* get the channel to work on */
            /* If the abort is done (CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_STATUS.DONE==1), then clear the operation */
            done = soc_pci_read(unit, CMIC_SCHAN_FIFO_CHx_STATUS(chan)) & SCHAN_FIFO_STATUS_DONE;
            if (done || timeout_state == abort_timeout_passed) {
                /* clear CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_CTRL.ABORT|START in the same write: disables and clears statuses*/
                cmic_address = CMIC_SCHAN_FIFO_CHx_CTRL(chan);
                val = soc_pci_read(unit, cmic_address);
                soc_pci_write(unit, cmic_address, val & ~(SCHAN_FIFO_CTRL_ABORT | SCHAN_FIFO_CTRL_START));
                if (done) { /* remove the channel from the channels waited on */
                    chans_group_delete(&schan_fifo_channels, channel_iter--);
                } else {
                    LOG_ERROR(BSL_LS_SOC_DMA, (BSL_META_U(unit, "Failed to abort s-chan FIFO in channel %d, check with the design team\n"), chan));
                    ++nof_failures;
                }
            }
        }

}

#endif


static int _cmicx_fifodma_init(struct net_device *dev)
{
    uint32 val;

    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_ARB_CTRL_OFFSET);

    bkn_dev_write32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_ARB_CTRL_OFFSET, 0xeee);


    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_OFFSET);
    val = 0x36db6db6;
    bkn_dev_write32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_OFFSET, val);


    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_1_OFFSET);
    val = 0x36;
    bkn_dev_write32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_1_OFFSET, val);

    return 0;
}

static int _cmicx_schan_fifo_init(struct net_device *dev)
{
    uint32 val;
    uint32 ch, idx;
    uint16 *summary_buff[CMIC_SCHAN_FIFO_NUM_MAX];

    /* Set CMIC_COMMON_POOL_SHARED_CONFIG Register,
     * SCHAN FIFO are sent through AXI master port of CMC0
     */
    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SHARED_CONFIG_OFFSET);
    //soc_reg_field_set(dev, CMIC_COMMON_POOL_SHARED_CONFIGr,
    //                  &val, MAP_SCHAN_FIFO_MEMWR_REQf,
    //                  MAP_SCHAN_FIFO_MEMWR_REQ_CMC0);
    val = 0;
    bkn_dev_write32(dev, CMIC_COMMON_POOL_SHARED_CONFIG_OFFSET, val);   


    /* Programming WRR (Arbitration) within CMC. Configure WRR weight
     * for FIFO DMA channels
     */
    bkn_dev_write32(dev,
                    CMIC_COMMON_POOL_SHARED_SCHAN_FIFO_WRITE_ARB_CTRL_OFFSET,
                    SCHAN_FIFO_MEMWR_WRR_WEIGHT);    


    /* perform hardware initialization */
   summary_buff[0] = 0x33c00000;
   summary_buff[1] = 0x33c00080;
   for (ch = 0 ; ch < CMIC_SCHAN_FIFO_NUM_MAX; ch++) {
       /* Configure AXI ID for SCHAN FIFO */
       val = bkn_dev_read32(dev, CMIC_SCHAN_FIFO_CHx_CTRL(ch));
       //soc_reg_field_set(unit, CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_CTRLr,
       //                &val, AXI_IDf, SCHAN_FIFO_AXI_ID);
       //_cmicx_schan_fifo_endian_config(unit, &val);
       bkn_dev_write32(dev, CMIC_SCHAN_FIFO_CHx_CTRL(ch), val);

       /* Set up summary Register */
       //GFP_ATOMIC | GFP_DMA;
       //summary_buff[ch] = kmalloc (CMIC_SCHAN_FIFO_CMD_SIZE_MAX * 2, GFP_ATOMIC | GFP_DMA);
        
       //if (schan_fifo->summary_buff[ch] == NULL) {
       //   rv = SOC_E_MEMORY;
       //   break;
       //}

       //gprintk("schan buff addr ch[%d], 0x%x\n", ch, summary_buff[ch]);
       /* write summary Lo address */
       bkn_dev_write32(dev, CMIC_SCHAN_FIFO_CHx_SUMMARY_ADDR_LOWER(ch),
                    PTR_TO_INT(summary_buff[ch]));
       /* write summary Hi address */
       bkn_dev_write32(dev, CMIC_SCHAN_FIFO_CHx_SUMMARY_ADDR_UPPER(ch),
                   (PTR_HI_TO_INT(summary_buff[ch]) |
                    CMIC_PCIE_SO_OFFSET));

    }


    /* Initialize the SCHAN FIFO command memories */
    for (ch = 0; ch < CMIC_SCHAN_FIFO_NUM_MAX; ch++) {
        for (idx = 0;
             idx < (CMIC_SCHAN_FIFO_CMD_SIZE_MAX * (CMIC_SCHAN_WORDS));
             idx++) {
            bkn_dev_write32(dev, CMIC_SCHAN_FIFO_CHx_COMMAND(ch, idx), 0);
        }
    }

    return 0;
}

// purpose API to test PCI access to cmicx registers
static int _cmicx_pci_test(struct net_device *dev)
{
    int i;
    uint32 tmp, reread;
    uint32 pat;

    //SCHAN_LOCK(unit);

    /* Check for address uniqueness */

    for (i = 0; i < CMIC_SCHAN_WORDS; i++) {
        pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
        bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i), pat);
    }

    for (i = 0; i < CMIC_SCHAN_WORDS; i++) {
        pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
        tmp = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i));
        if (tmp != pat) {
            goto error;
        }
    }

    //if (!SAL_BOOT_QUICKTURN) {  /* Takes too long */
        /* Rotate walking zero/one pattern through each register */

        pat = 0xff7f0080;       /* Simultaneous walking 0 and 1 */

        for (i = 0; i < CMIC_SCHAN_WORDS; i++) {
            int j;

            for (j = 0; j < 32; j++) {
                    bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i), pat);
                    tmp = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i));
                if (tmp != pat) {
                    goto error;
                }
                pat = (pat << 1) | ((pat >> 31) & 1);	/* Rotate left */
            }
        }
    //}

    /* Clear to zeroes when done */

    for (i = 0; i < CMIC_SCHAN_WORDS; i++) {
            bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i), 0);
    }
    //SCHAN_UNLOCK(unit);
    gprintk("PCI test PASSED.\n");
    return 0;

 error:
    reread = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i));

    gprintk("FATAL PCI error testing PCIM[0x%x]:\n"
            "Wrote 0x%x, read 0x%x, re-read 0x%x\n",
             i, pat, tmp, reread);

    //SCHAN_UNLOCK(unit);
    return -EFAULT;
}

static int _trident3_mdio_rate_divisor_set(void)
{
    int int_divisor, ext_divisor;
    int ring_idx = 0;
    int ring_idx_end = 0;
    miim_ring_control_t ring_control;

    ext_divisor = RATE_EXT_MDIO_DIVISOR_DEF;
    int_divisor = TD3X_RATE_INT_MDIO_DIVISOR_DEF;
    //delay = -1;

    /*  mdio ring end based on iProc15 device */
    ring_idx_end = CMICX_MIIM_12R_RING_INDEX_END;

    for (ring_idx = CMICX_MIIM_RING_INDEX_START; ring_idx <= ring_idx_end; ring_idx++) {
        //soc_cmicx_miim_divider_set_ring(unit, ring_idx, int_divider, ext_divisor, delay);
        ring_control.word = _iproc_getreg(MIIM_RING0_CONTROLr + (ring_idx<<2));
        ring_control.reg.CLOCK_DIVIDER_EXTf = ext_divisor;
	ring_control.reg.CLOCK_DIVIDER_INTf = int_divisor;
        _iproc_setreg(MIIM_RING0_CONTROLr + (ring_idx<<2), ring_control.word);
    }

    return SOC_E_NONE;
}


//_soc_helix5_misc_init
static int _misc_init(struct bcmsw_switch *bcmsw_sw)
{
    /******************************* soc_reset()****************************/


    /******************************* soc_reset()****************************/
    //_soc_helix5_port_mapping_init

    //_soc_helix5_idb_init


    /* Setup MDIO divider */
    _trident3_mdio_rate_divisor_set();

    //_soc_hx5_ledup_init


    return 0;

}


int
soc_trident3_xpe_reg_get(int unit, soc_reg_t reg, int xpe, int base_index,
                         int index, uint64 *data)
{
    return _soc_trident3_xpe_reg_access(unit, reg, xpe, base_index,
                                        index, data, FALSE);
}


static int 
_soc_hx5_thdo_hw_set(struct bcmsw_switch *bcmsw_sw, int port, int enable)
{
    uint64 rval64, rval64_tmp;
    int pipe, i;
    int split, pos, mmu_port;
    soc_info_t *si = bcmsw_sw->si;

    uint32_t reg[3][2] = {
    {
        THDU_OUTPUT_PORT_RX_ENABLE_SPLIT0r,
        THDU_OUTPUT_PORT_RX_ENABLE_SPLIT1r
    },
    {
        MMU_THDM_DB_PORT_RX_ENABLE_64_SPLIT0r,
        MMU_THDM_DB_PORT_RX_ENABLE_64_SPLIT1r
    },
    {
        MMU_THDM_MCQE_PORT_RX_ENABLE_64_SPLIT0r,
        MMU_THDM_MCQE_PORT_RX_ENABLE_64_SPLIT1r
    }
    };
    
    pipe = si->port_pipe[port];
    phy_port = si->port_l2p_mapping[port];
    mmu_port = si->port_p2m_mapping[phy_port];
    /* Reg config is per pipe, get local MMU port. */
    mmu_port = mmu_port & SOC_TD3_MMU_PORT_STRIDE;

    if (mmu_port < 64) {
        split = 0;
        pos = mmu_port;
    } else {
        split = 1;
        pos = mmu_port - 64;
    }


    for (i = 0; i < 3; i++) {
        rval64 = 0;
        rval64_tmp  = 1 << pos;

        _reg64_read(bcmsw_sw->dev, SCHAN_BLK_MMU_XPE, reg[i][split], &rval64);                                  
        printk("_soc_hx5_thdo_hw_set 0x%x\n",rval64);

        if (enable) {
            rval64 |= rval64_tmp;
        } else {
            rval64 &= (~rval64_tmp);
        }
        _reg64_write(bcmsw_sw->dev, SCHAN_BLK_MMU_XPE, reg[i][split], rval64);
    }
}

return SOC_E_NONE;
}

//_soc_helix5_mmu_init
static int _mmu_init(struct bcmsw_switch *bcmsw_sw)
{
    uint32 val;
    int num_port = HX5_NUM_PORT, port, vid; 
    soc_info_t *si = bcmsw_sw->si;

    for (port = 0; port < num_port; port++) {
       if(si->port_type[port] != -1) {
           _soc_hx5_thdo_hw_set(bcmsw_sw, port, 1);
       }
    }

    /* enable WRED refresh */
    //    SOC_IF_ERROR_RETURN(READ_WRED_REFRESH_CONTROLr(unit, &rval));
    //    soc_reg_field_set(unit, WRED_REFRESH_CONTROLr, &rval,
    //            REFRESH_DISABLEf, 0);
    //    soc_reg_field_set(unit, WRED_REFRESH_CONTROLr, &rval,
    //            REFRESH_PERIODf, time_refresh);
    //    SOC_IF_ERROR_RETURN(WRITE_WRED_REFRESH_CONTROLr(unit, rval));
    val = 0x30;
    _reg32_write(bcmsw_sw->dev,SCHAN_BLK_MMU_XPE, WRED_REFRESH_CONTROLr, val);

    //readback
    _reg32_read(bcmsw_sw->dev,SCHAN_BLK_MMU_XPE, WRED_REFRESH_CONTROLr, &val);
    printk("_mmu_init WRED_REFRESH_CONTROLr = 0x%x\n", val);


    //soc_trident3_sc_reg32_get(unit, MMU_1DBG_Cr, 0, 0, 0, &rval));
    //soc_reg_field_set(unit, MMU_1DBG_Cr, &rval, FIELD_Af, 1);
    //soc_trident3_sc_reg32_set(unit, MMU_1DBG_Cr, -1, -1, 0, rval));
    val = 0x29;
    _reg32_write(bcmsw_sw->dev,SCHAN_BLK_MMU_SC, MMU_1DBG_Cr, val);

    //readback
    _reg32_read(bcmsw_sw->dev,SCHAN_BLK_MMU_SC, MMU_1DBG_Cr, &val);
    printk("_mmu_init MMU_1DBG_Cr = 0x%x\n", val);
    
    // (soc_trident3_sc_reg32_set(unit, MMU_2DBG_C_1r, -1, -1, 0, 0x4));
    val = 0x4;
    _reg32_write(bcmsw_sw->dev,SCHAN_BLK_MMU_SC, MMU_2DBG_C_1r, val);

    //readback
    _reg32_read(bcmsw_sw->dev,SCHAN_BLK_MMU_SC, MMU_2DBG_C_1r, &val);
    printk("_mmu_init MMU_2DBG_C_1r = 0x%x\n", val);


    return 0;
}

static int bcmsw_modules_init(struct bcmsw_switch *bcmsw_sw)
{
    int err = 0;

    //create ports
    err = bcmsw_ports_init(bcmsw_sw);
    if (err) {
    	//dev_err(mlxsw_sp->bus_info->dev, "Failed to create ports\n");
    	goto err_ports_create;
    }      

err_ports_create:
    return err;
}


static int _clear_all_memory(struct net_device *dev)
{
    int val;

     /* Initial IPIPE memory */
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_1r, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, 0x00298000);

    /* Initial EPIPE memory */
    _reg32_write(dev, SCHAN_BLK_EPIPE, EGR_HW_RESET_CONTROL_0r, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_EPIPE, EGR_HW_RESET_CONTROL_1r, 0x000c6000);
    //wait 50ms

    /* Wait for IPIPE memory initialization done */
    do {
        _reg32_read(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2_PIPE0r, &val);
        //sleep 1ms
       msleep(1);
     } while(!(val& (1<<22)));    

    /* Restore L3_ENTRY_HASH_CONTROL->HASH_TABLE_BANK_CONFIG value */
    ///TODO

    /* Wait for EPIPE memory initialization done */
    do {
        _reg32_read(dev, SCHAN_BLK_EPIPE, EGR_HW_RESET_CONTROL_1_PIPE0r, &val);
        //sleep 1ms
        msleep(1);
     } while(!(val& (1<<20)));   

    //
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, 0x0);
    _reg32_write(dev, SCHAN_BLK_EPIPE, EGR_HW_RESET_CONTROL_1r, 0x00004000);


    /* Initial IDB memory */
    _reg32_write(dev, SCHAN_BLK_IPIPE, IDB_HW_CONTROLr, 0x0);
    _reg32_write(dev, SCHAN_BLK_IPIPE, IDB_HW_CONTROLr, 0x1);
    _reg32_write(dev, SCHAN_BLK_IPIPE, IDB_HW_CONTROLr, 0x0);

    /* Initial PORT MIB counter */
    _reg32_write(dev, SCHAN_BLK_CLPORT1, CLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_CLPORT1, CLPORT_MIB_RESETr, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_CLPORT2, CLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_CLPORT2, CLPORT_MIB_RESETr, 0x00000000);
    
    _reg32_write(dev, SCHAN_BLK_XLPORT0, XLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_XLPORT0, XLPORT_MIB_RESETr, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_XLPORT1, XLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_XLPORT1, XLPORT_MIB_RESETr, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_XLPORT2, XLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_XLPORT2, XLPORT_MIB_RESETr, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_MIB_RESETr, 0x00000000);
    
    /* TCAM tables are not handled by hardware reset control */
    //TODO

    /* TD3-1847 */
    //RH_DLB_SELECTIONf = 1
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000000);
    //soc_mem_clear(unit, RH_LAG_FLOWSETm, COPYNO_ALL, TRUE));
    //RH_DLB_SELECTIONf = 0
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000001);

    //RH_DLB_SELECTIONf = 1
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000005);
    //soc_mem_clear(unit, RH_HGT_FLOWSETm, COPYNO_ALL, TRUE));
    //RH_DLB_SELECTIONf = 0
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000004);
    //HGT_LAG_FLOWSET_TABLE_CONFIGf = 0
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000000);


    return 0;
    
}


static int
_powerdown_single_tsc(struct net_device *dev, int blk, int reg) 
{
    /*
    soc_field_info_t soc_CLPORT_XGXS0_CTRL_REG_BCM56560_B0r_fields[] = {
    { IDDQf, 1, 4, SOCF_RES },
    { PWRDWNf, 1, 3, SOCF_RES },
    { REFIN_ENf, 1, 2, SOCF_RES },
    { REFOUT_ENf, 1, 1, SOCF_RES },
    { RSTB_HWf, 1, 0, SOCF_RES }
    };
    */
    uint32      val;
    /*
     * Reference clock selection
     */
    _reg32_read(dev, blk, reg, &val);
    val = val |  (1<<2);   // REFIN_ENf = 1 
    val = val & ~(1<<4);   // IDDQf     = 0
    _reg32_write(dev, blk, reg, val);

    /* Deassert power down */
    val = val & ~(1<<3);   // PWRDWNf     = 0
    _reg32_write(dev, blk, reg, val);
    msleep(1);

    /* Reset XGXS */
    val = val & ~(1);      // RSTB_HWf     = 0
    _reg32_write(dev, blk, reg, val);

    return 0;
}

static int 
_powerup_single_tsc(struct net_device *dev, int blk, int reg) 
{

    uint32      val;

    /* Bring XGXS out of reset */
    _reg32_read(dev, blk, reg, &val);
    val = val | 1;      // RSTB_HWf     = 1
    _reg32_write(dev, blk, reg, val);

    msleep(1);

    return 0;
}

static int _helix5_port_reset(struct net_device *dev)
{
    int val;

    //CLPORT  
    //--------CLPORT 1
    /* Power off CLPORT blocks */
    _powerdown_single_tsc(dev, SCHAN_BLK_CLPORT1, CLPORT_XGXS0_CTRL_REGr);

     /* Power on CLPORT blocks */
     _powerup_single_tsc(dev, SCHAN_BLK_CLPORT1, CLPORT_XGXS0_CTRL_REGr);

    _reg32_write(dev, SCHAN_BLK_CLPORT1, CLPORT_MAC_CONTROLr, 0x00000001);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_CLPORT1, CLPORT_MAC_CONTROLr, 0x00000000);
   
    //--------CLPORT 2
    /* Power off CLPORT blocks */
    _powerdown_single_tsc(dev, SCHAN_BLK_CLPORT2, CLPORT_XGXS0_CTRL_REGr);

     /* Power on CLPORT blocks */
    _powerup_single_tsc(dev, SCHAN_BLK_CLPORT2, CLPORT_XGXS0_CTRL_REGr);
     
    _reg32_write(dev, SCHAN_BLK_CLPORT2, CLPORT_MAC_CONTROLr, 0x00000001);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_CLPORT2, CLPORT_MAC_CONTROLr, 0x00000000);
   
    //PMQPORT 0 1 2 
    /*
    soc_field_info_t soc_XLPORT_XGXS0_CTRL_REG_BCM56980_A0r_fields[] = {
    { IDDQf, 1, 4, SOCF_RES },
    { PWRDWNf, 1, 3, SOCF_RES },
    { PWRDWN_CMLf, 1, 5, SOCF_RES },
    { PWRDWN_CML_LCf, 1, 6, SOCF_RES },
    { REFCMOSf, 1, 7, SOCF_RES },
    { REFIN_ENf, 1, 2, SOCF_RES },
    { REFOUT_ENf, 1, 1, SOCF_RES },
    { REFSELf, 3, 8, SOCF_LE|SOCF_RES },
    { RSTB_HWf, 1, 0, SOCF_RES }
    };    
    */
    _reg32_read(dev, SCHAN_BLK_PMQPORT0, CHIP_CONFIGr, &val);
    if (val & 0x1) { //Q_MODE
        /* Power off PMQ blocks */
        _powerdown_single_tsc(dev, SCHAN_BLK_PMQPORT0, PMQ_XGXS0_CTRL_REGr);
        msleep(100);
        /* Power on PMQ blocks */
        _powerup_single_tsc(dev, SCHAN_BLK_PMQPORT0, PMQ_XGXS0_CTRL_REGr);

        _reg32_read(dev, SCHAN_BLK_PMQPORT0, PMQ_XGXS0_CTRL_REGr, &val);
        val = (val & ~(0x0700)) | (5<<8); // REFSELf = 5
        _reg32_write(dev, SCHAN_BLK_PMQPORT0, PMQ_XGXS0_CTRL_REGr, val);
    }

    _reg32_read(dev, SCHAN_BLK_PMQPORT1, CHIP_CONFIGr, &val);
    if (val & 0x1) { //Q_MODE
        /* Power off PMQ blocks */
        _powerdown_single_tsc(dev, SCHAN_BLK_PMQPORT1, PMQ_XGXS0_CTRL_REGr);
        msleep(10);
        /* Power on PMQ blocks */
        _powerup_single_tsc(dev, SCHAN_BLK_PMQPORT1, PMQ_XGXS0_CTRL_REGr);

        _reg32_read(dev, SCHAN_BLK_PMQPORT1, PMQ_XGXS0_CTRL_REGr, &val);
        val = (val & ~(0x0700)) | (5<<8); // REFSELf = 5
        _reg32_write(dev, SCHAN_BLK_PMQPORT1, PMQ_XGXS0_CTRL_REGr, val);
    }
    
    _reg32_read(dev, SCHAN_BLK_PMQPORT2, CHIP_CONFIGr, &val);
    if (val & 0x1) { //Q_MODE
        /* Power off PMQ blocks */
        _powerdown_single_tsc(dev, SCHAN_BLK_PMQPORT2, PMQ_XGXS0_CTRL_REGr);
        msleep(10);
        /* Power on PMQ blocks */
        _powerup_single_tsc(dev, SCHAN_BLK_PMQPORT2, PMQ_XGXS0_CTRL_REGr);

        _reg32_read(dev, SCHAN_BLK_PMQPORT2, PMQ_XGXS0_CTRL_REGr, &val);
        val = (val & ~(0x0700)) | (5<<8); // REFSELf = 5
        _reg32_write(dev, SCHAN_BLK_PMQPORT2, PMQ_XGXS0_CTRL_REGr, val);
    }    

    //XLPORT 0 1 2 6
    _reg32_write(dev, SCHAN_BLK_XLPORT0, XLPORT_MAC_CONTROLr, 0x1);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_XLPORT0, XLPORT_MAC_CONTROLr, 0x0);

    _reg32_write(dev, SCHAN_BLK_XLPORT1, XLPORT_MAC_CONTROLr, 0x1);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_XLPORT1, XLPORT_MAC_CONTROLr, 0x0);

    _reg32_write(dev, SCHAN_BLK_XLPORT2, XLPORT_MAC_CONTROLr, 0x1);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_XLPORT2, XLPORT_MAC_CONTROLr, 0x0);

    //XLPORT6 
    //!IS_QSGMII_PORT
    /* Power off XLPORT blocks */
    _powerdown_single_tsc(dev, SCHAN_BLK_XLPORT6, XLPORT_XGXS0_CTRL_REGr);
    msleep(10);
    /* Power on XLPORT blocks */
    _powerup_single_tsc(dev, SCHAN_BLK_XLPORT6, XLPORT_XGXS0_CTRL_REGr);

    /*
    soc_field_info_t soc_XLPORT_XGXS0_CTRL_REG_BCM56980_A0r_fields[] = {
    { IDDQf, 1, 4, SOCF_RES },
    { PWRDWNf, 1, 3, SOCF_RES },
    { PWRDWN_CMLf, 1, 5, SOCF_RES },
    { PWRDWN_CML_LCf, 1, 6, SOCF_RES },
    { REFCMOSf, 1, 7, SOCF_RES },
    { REFIN_ENf, 1, 2, SOCF_RES },
    { REFOUT_ENf, 1, 1, SOCF_RES },
    { REFSELf, 3, 8, SOCF_LE|SOCF_RES },
    { RSTB_HWf, 1, 0, SOCF_RES }
    };    
    */
   
    _reg32_read(dev, SCHAN_BLK_XLPORT6, XLPORT_XGXS0_CTRL_REGr, &val);
    val = (val & ~(0x0700)) | (5<<8); // REFSELf = 5
    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_XGXS0_CTRL_REGr, val);

    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_MAC_CONTROLr, 0x1);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_MAC_CONTROLr, 0x0);

    return 0;
}

//soc_do_init(int unit, int reset)
static int _switch_do_init(struct bcmsw_switch *bcmsw_sw)
{
    struct net_device *dev = bcmsw_sw->dev;
    int val;

    /* Initialize PCI Host interface */
    //soc_pcie_host_intf_init(unit));
    //
    
    /************* soc_phyctrl_software_init   *****************************/

    /******************************* soc_reset()****************************/
    // soc_endian_config
    bkn_dev_write32(dev, CMIC_ENDIAN_SELECT, 0);
    // soc_pci_ep_config  - CMICM only
    // soc_pci_burst_enable

    /* CMICx DMA channels need to be released/aborted properly */
    //soc_cmicx_dma_abort

    //soc_cmic_intr_all_disable
    //soc_cmic_intr_all_disable();

     //cmicx_fifodma_init
     _cmicx_fifodma_init(dev);

    //cmicx_sbusdma_reg_init
     bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_ARB_CTRL_SBUSDMA, CMIC_TOP_SBUS_RING_ARB_CTRL_SET);

     //soc_esw_schan_fifo_init
     _cmicx_schan_fifo_init(dev);


    /* Initialize bulk mem API */
    //soc_mem_bulk_init
    //


    /************* soc_reset() -> soc_helix5_chip_reset       **************/
    //soc_helix5_sbus_ring_map_config
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_0_7_OFFSET,0x52222100);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_8_15_OFFSET,0x30050005);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_16_23_OFFSET,0x33333333);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_24_31_OFFSET,0x64444333);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_32_39_OFFSET,0x07500066);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_40_47_OFFSET,0x00000000);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_48_55_OFFSET,0x00000000);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_56_63_OFFSET,0x00000000);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_TIMEOUT_OFFSET,0x5000);
    
    msleep(250);

    //do a read
    //_reg32_read(dev, SCHAN_BLK_TOP, TOP_SOFT_RESET_REGr, &val); 

    /* Reset IP, EP, MMU and port macros */
    //SOC_IF_ERROR_RETURN(WRITE_TOP_SOFT_RESET_REGr(unit, 0x0));
    //   soc_reg32_set(unit, TOP_SOFT_RESET_REGr, REG_PORT_ANY, 0, rv) 
    //      Write an internal SOC register through S-Channel messaging buffer.
    _reg32_write(dev, SCHAN_BLK_TOP, TOP_SOFT_RESET_REGr, 0x0);


    /* If frequency is specificed use it , else use the si->frequency */

    /* Bring PLLs out of reset */

    /* De-assert TS PLL, BS PLL0/1 post reset and bring AVS out of reset */
    

    /* Bring IP, EP, MMU and port macros out of reset */
    _reg32_write(dev, SCHAN_BLK_TOP, TOP_SOFT_RESET_REGr, 0x3fff);
    msleep(10);

    /* PM4x10Q QSGMII mode control
     */
    // PM4X10Q_1_QSGMII_MODE_ENf = 1
    _reg32_write(dev, SCHAN_BLK_PMQPORT0, CHIP_CONFIGr, 0x31);
    _reg32_write(dev, SCHAN_BLK_PMQPORT1, CHIP_CONFIGr, 0x31);
    _reg32_write(dev, SCHAN_BLK_PMQPORT2, CHIP_CONFIGr, 0x31);
    //TOP_MISC_GENERIC_CONTROLr
    _reg32_write(dev, SCHAN_BLK_TOP, TOP_MISC_GENERIC_CONTROLr, 0x7);

    /* Reset egress hardware resource */
    /* Write the value to enable 4 lanes on the PM */
    _reg64_write(dev, SCHAN_BLK_EPIPE, EGR_PORT_BUFFER_SFT_RESET_0r, 0x0000000924924900);
    /* Set it back to zero now */
    _reg64_write(dev, SCHAN_BLK_EPIPE, EGR_PORT_BUFFER_SFT_RESET_0r, 0x0);
    //spn_PARITY_ENABLE
    _reg64_write(dev, SCHAN_BLK_IPIPE, IDB_SER_CONTROL_64r, 0x0000200a);


    //SOC_IF_ERROR_RETURN(soc_trident3_init_idb_memory(unit));
    /* Initialize IDB memory */
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, 0x00380100);
    /* Wait for IDB memory initialization done */
    do {
       _reg32_read(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, &val);
       //sleep 1ms
       msleep(1);
    } while(!(val& (1<<22)));
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, 0x00100000);
    //TDM
    _reg32_write(dev, SCHAN_BLK_IPIPE, IS_TDM_CONFIG_PIPE0r, 0x00080000);
    _reg32_write(dev, SCHAN_BLK_IPIPE, IS_OPP_SCHED_CFG_PIPE0r, 0x0920000d);

    //SOC_IF_ERROR_RETURN(_soc_helix5_init_hash_control_reset(unit));
    //SOC_IF_ERROR_RETURN(soc_helix5_uft_uat_config(unit));
    //SOC_IF_ERROR_RETURN(_soc_helix5_ft_bank_config(unit));


    //soc_trident3_clear_all_memory()
    _clear_all_memory(dev);

    //end of 
    //end of soc_helix5_chip_reset

    /************* soc_reset() -> soc_helix5_port_reset       **************/
    //endof soc_helix5_port_reset
    _helix5_port_reset(dev);

    
    /* Configure CMIC PCI registers correctly for driver operation.        */
    /*
     * Check that PCI memory space is mapped correctly by running a
     * quick diagnostic on the S-Channel message buffer.
     */
    //soc_pcie_fw_status_get()
    //_cmicx_pci_test(dev);

    //configure DMA channels
    //soc_dma_attach

  
    /***********************************************************************/
    return 0;
}

int bcmsw_switch_init(void)
{
    struct bcmsw_switch *bcmsw_sw;
    int err = 0;
    lport_tab_entry_t lport_entry;

        
    bcmsw_sw = kzalloc(sizeof(*bcmsw_sw), GFP_KERNEL);
    if (!bcmsw_sw)
		return -ENOMEM;

    err = bcmsw_switchdev_init(bcmsw_sw);
    if (err)
	goto err_swdev_register;
  
    //get bcm0 netdev
    bcmsw_sw->dev = __dev_get_by_name(current->nsproxy->net_ns, "bcm0");

    /* Connect to the kernel bde */
    if ((linux_bde_create(NULL, &kernel_bde) < 0) || kernel_bde == NULL) {
        return -ENODEV;
    }

    //switch initialization 
    //BCM: init soc
    _switch_do_init(bcmsw_sw);

    //load m0 firmware
    //BCM: m0 load 0 0x0 linkscan_led_fw.bin
    //	   m0 load 0 0x3800 custom_led.bin

    // init cancun, and load cancun pkgs
    //BCM: cancun load cch
    //     cancun load ceh
    //     cancun load cmh
    //     cancun load cih

    //misc_init
    //BCM: init misc
    _misc_init(bcmsw_sw);

    //init mmu
    _mmu_init(bcmsw_sw);

    //initialize modules
    //BCM: init bcm
    bcmsw_modules_init(bcmsw_sw);
  

    //test schan
    //err = _soc_mem_read(bcmsw_sw->dev, 0x501c0000, SCHAN_BLK_IPIPE, 14, &lport_entry); 


    //test iproc reg read/write


err_swdev_register:
    return err;    
}

