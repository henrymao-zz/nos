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

static ibde_t *kernel_bde = NULL;
static bcmsw_switch_t *_bcmsw = NULL; 

/*****************************************************************************************/
/*                              SOC                                                      */
/*****************************************************************************************/
/* Environment switch */
#define _PCID_TEST 0

/* Local defines */
#define BYTES_PER_UINT32    (sizeof(uint32))

char *_shr_errmsg[] = _SHR_ERRMSG_INIT;

static const uint32_t empty_entry[SOC_MAX_MEM_WORDS] = {0};



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
    { 9,   9,  1, 0x9, 0x0900, BCMSW_PORT_TYPE_GXPORT, "Ethernet8",  {9, -1, -1, -1}}, 
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
        return val;
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

    return val;
}

/*****************************************************************************************/
/*                             SCHAN Reg Read/Write                                      */
/*****************************************************************************************/
static int
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

static int
_reg32_read(struct net_device *dev, int dst_blk, uint32_t address, uint32_t *data)
{
    return _schan_reg32_read(dev, dst_blk, address, data, 0);
}


/*
 * Write an internal SOC register through S-Channel messaging buffer.
 */
static int
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

static int
_reg32_write(struct net_device *dev, int dst_blk, uint32_t address, uint32_t data)
{
    return _schan_reg32_write(dev, dst_blk, address, data, 0);
}


static int
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

static int
_reg64_read(struct net_device *dev, int dst_blk, uint32_t address, uint64_t *data)
{
    return _schan_reg64_read(dev, dst_blk, address, data, 0);
}
/*
 * Write an internal SOC register through S-Channel messaging buffer.
 */
static int
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

static int
_reg64_write(struct net_device *dev, int dst_blk, uint32_t address, uint64_t data)
{
    return _schan_reg64_write(dev, dst_blk, address, data, 0);
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


int soc_cancun_file_info_get(bcmsw_switch_t *sw, soc_cancun_file_t* ccf, char *filename,
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

static int _soc_cancun_file_pio_load(bcmsw_switch_t *sw, uint8* buf, int buf_words) 
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



static int _soc_cancun_cih_tcam_write(bcmsw_switch_t *sw, uint8 *buf) {
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

static int _soc_cancun_cih_mem_load(bcmsw_switch_t *sw, uint8 *buf) {
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

static int _soc_cancun_cih_pio_load(bcmsw_switch_t *sw, uint8* buf, int length,
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

static int _soc_cancun_cih_load(bcmsw_switch_t *sw, uint8* buf, int num_data_blobs) {
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

static int _soc_cancun_file_cmh_load(bcmsw_switch_t *sw, uint8* buf, int buf_words) {
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


static int _soc_cancun_file_cch_load(bcmsw_switch_t *sw, uint8* buf, int buf_words) {
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

static int _soc_cancun_file_ceh_load(bcmsw_switch_t *sw, uint8* buf, int buf_words)
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

int soc_cancun_file_load(bcmsw_switch_t *sw, uint8* buf, long buf_bytes, uint32* type,
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
static int _soc_cancun_alloc(bcmsw_switch_t *sw) 
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


int soc_cancun_init (bcmsw_switch_t *swdev) 
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



int bcmsw_switchdev_init(bcmsw_switch_t *sw)
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

    //reset array to default
    //HX5 has 72 ports(NUM_PORT=72)
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
	    if (phy_port <= 48) {
            si->port_serdes[port] = (phy_port - 1) / _HX5_PORTS_PER_PMQ_PBLK;
        } else {
            si->port_serdes[port] = ((phy_port - 1) / _HX5_PORTS_PER_PBLK) - 9;
	    }

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

    _soc_hx5_mmu_idb_ports_assign(si);

#if 0
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
#endif                
}




#define BCMSW_VLAN_DEFAULT    1

//bcm_esw_port_init bcm_td3_port_cfg_init
static int 
_port_cfg_init(bcmsw_switch_t *bcmsw, int port, int vid)
{
    soc_info_t *si = bcmsw->si;
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
    _soc_mem_read(bcmsw->dev, EGR_PORTm+port, SCHAN_BLK_EPIPE, BYTES2WORDS(sizeof(egr_port_entry_t)), &egr_port_entry); 

    egr_port_entry.port_type = port_type;

    _soc_mem_write(bcmsw->dev, EGR_PORTm+port, SCHAN_BLK_EPIPE, BYTES2WORDS(sizeof(egr_port_entry_t)), &egr_port_entry); 

    /* initialize the Cancun tag profile entry setup
     * for VT_MISS_UNTAG action. Should be done in Cancun
     */
    //soc_cancun_cmh_mem_set(unit, 

    /* Copy EGR port information to CPU Higig port if applied */
    //Not applicable for BCM56370

    /* PORT_TABLE config init */
    //read LPORT_TABm , check _bcm_td3_port_tab_conv for memory
    _soc_mem_read(bcmsw->dev, LPORT_TABm+port, SCHAN_BLK_IPIPE, BYTES2WORDS(sizeof(lport_tab_entry_t)), &lport_entry); 

    lport_entry.reg.PORT_VIDf = vid;
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

    _soc_mem_write(bcmsw->dev, LPORT_TABm+port, SCHAN_BLK_IPIPE, BYTES2WORDS(sizeof(lport_tab_entry_t)), &lport_entry); 

    if (cpu_hg_index != -1) {
        //soc_cancun_cmh_mem_set(unit, PORT_TABm, cpu_hg_index, PORT_TYPEf, 1);
        /* TD3TBD should be covered by CMH, will remove it after CMH
         * is ready. */
         _soc_mem_read(bcmsw->dev, ING_DEVICE_PORTm+cpu_hg_index, SCHAN_BLK_IPIPE, BYTES2WORDS(sizeof(ing_device_port_entry_t)), &ing_device_port_entry); 

        //BCM_IF_ERROR_RETURN(soc_mem_field32_modify(unit, ING_DEVICE_PORTm,
        //                    cpu_hg_index, PORT_TYPEf, 1));
        ing_device_port_entry.reg.PORT_TYPEf = 1;
        ing_device_port_entry.reg.SRC_SYS_PORT_IDf = port;
        ing_device_port_entry.reg.SYS_PORT_IDf = port;
        ing_device_port_entry.reg.PP_PORT_NUMf = port;
        ing_device_port_entry.reg.DUAL_MODID_ENABLEf = 0; //dual_modid;
        _soc_mem_write(bcmsw->dev, ING_DEVICE_PORTm+cpu_hg_index, SCHAN_BLK_IPIPE, BYTES2WORDS(sizeof(ing_device_port_entry_t)), &ing_device_port_entry); 
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

    //printk("phy_bcm542xx_qsgmii_reg_write port %d reg 0x%x data 0x%x\n", phy_addr, reg, data);

    return SOC_E_NONE;
}

/*****************************************************************************************/
/*                             Phy Probe                                                 */
/*****************************************************************************************/

static int
_ext_phy_probe(bcmsw_switch_t *bcmsw, int port)
{
    uint16               phy_addr, phy_id0, phy_id1;
    uint32               id0_addr, id1_addr;
    soc_info_t           *si = bcmsw->si;
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
/*                             UNI MAC                                                   */
/*****************************************************************************************/


static int unimac_reset_check(bcmsw_switch_t *bcmsw, int port, int enable, int *reset)
{
    command_config_t ctrl, octrl, swctrl;
    int index, blk_no;
    
    uint32_t blk[6] = {
        SCHAN_BLK_GXPORT0,
        SCHAN_BLK_GXPORT1,
        SCHAN_BLK_GXPORT2,
        SCHAN_BLK_GXPORT3,
        SCHAN_BLK_GXPORT4,
        SCHAN_BLK_GXPORT5,
    };
        
    *reset = 1;

    if (port > 48) {
        return -1;
    }
    index = (port-1)%8;
    blk_no = blk[(port-1)/8];

    _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
    octrl.word = ctrl.word;

    printk("unimac_reset_check port %d ctrl 0x%x\n", port, ctrl.word);
    ctrl.reg.TX_ENAf = enable ? 1:0;
    ctrl.reg.RX_ENAf = enable ? 1:0;


    if (ctrl.word == octrl.word) {
        if (enable) {
            *reset = 0;
        } else {
            _reg32_read(bcmsw->dev,blk_no, COMMAND_CONFIGr+index, &swctrl.word);

            if (swctrl.reg.SW_RESETf) {
                *reset = 0;
            }
        }
    }

    return SOC_E_NONE;
}
//soc_helix5_idb_obm_reset_buffer
static const uint32_t obm_ctrl_regs[HELIX5_PBLKS_PER_PIPE] = {
     IDB_OBM0_Q_CONTROLr, 
     IDB_OBM1_Q_CONTROLr, 
     IDB_OBM2_Q_CONTROLr,
     IDB_OBM0_CONTROLr,
     IDB_OBM1_CONTROLr, 
     IDB_OBM2_CONTROLr, 
     IDB_OBM3_CONTROLr,
     IDB_OBM0_48_CONTROLr,
     IDB_OBM1_48_CONTROLr, 
     IDB_OBM2_48_CONTROLr
};

static int
_helix5_get_pm_from_phynum(int phy_port)
{
    int num;

    num = phy_port - 1;
    if (num <= 49) {
        num = num/16;
    } else if (num <= HELIX5_TDM_PHY_PORTS_PER_PIPE) { 
        num = (num/4)-9;
    } else {
        return -1;
    }

    return num;
}

int
_helix5_idb_obm_reset_buffer(bcmsw_switch_t *bcmsw, int port, int reset_buffer)
{
    uint32_t reg;
    uint32_t offset = 0;
    int32_t phy_port = bcmsw->si->port_l2p_mapping[port];
    int pm_num, subp;
    obm_q_control_t reg_obm;

    pm_num = _helix5_get_pm_from_phynum(phy_port);
    subp = (phy_port -1)&0x3;

    reg = obm_ctrl_regs[pm_num];
    

    offset = (phy_port - 1)%16;
    offset = offset/4;

    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, reg+(offset<<8), &reg_obm.word);
    printk("IDB port Up rval 0x%x pm_num %1d subp=%1d reset_buffer=%1d offset=%1d  \n",
           reg_obm.word, pm_num, subp,reset_buffer,offset);
    if (subp == 0) {
        reg_obm.reg.PORT0_RESETf = reset_buffer;
    } else if (subp == 1) {
        reg_obm.reg.PORT1_RESETf = reset_buffer;
    } else if (subp == 2) {
        reg_obm.reg.PORT2_RESETf = reset_buffer;
    } else {
        reg_obm.reg.PORT3_RESETf = reset_buffer;
    }

    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, reg+(offset<<8), reg_obm.word);
    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, reg+(offset<<8), &reg_obm.word);

    printk("IDB port Up rval_update 0x%x pm_num %1d sbup=%1d reset_buffer=%1d offset=%1d \n",
           reg_obm.word, pm_num, subp,reset_buffer,offset);

    return SOC_E_NONE;
}

static const uint32_t soc_helix5_obm_ca_ctrl_regs[HELIX5_PBLKS_PER_PIPE] = {
    IDB_OBM0_Q_CA_CONTROLr, 
    IDB_OBM1_Q_CA_CONTROLr,
    IDB_OBM2_Q_CA_CONTROLr, 
    IDB_OBM0_CA_CONTROLr,
    IDB_OBM1_CA_CONTROLr, 
    IDB_OBM2_CA_CONTROLr,
    IDB_OBM3_CA_CONTROLr, 
    IDB_OBM0_48_CA_CONTROLr,
    IDB_OBM1_48_CA_CONTROLr, 
    IDB_OBM2_48_CA_CONTROLr
};
// soc_helix5_idb_ca_reset_buffer
int
_helix5_idb_ca_reset_buffer(bcmsw_switch_t *bcmsw, int port, int reset_buffer)
{
    uint32_t reg;
    obm_q_ca_control_t val32;
    int32_t phy_port = bcmsw->si->port_l2p_mapping[port];
    int pm_num, subp;

    pm_num = _helix5_get_pm_from_phynum(phy_port);

    if(phy_port < 49) {
        subp = (phy_port-1)%16;
    }

    reg = soc_helix5_obm_ca_ctrl_regs[pm_num];

    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, reg, &val32.word);

    if (subp == 0) {
        val32.reg.PORT0_RESETf = reset_buffer;
    } else if (subp == 1) {
        val32.reg.PORT1_RESETf = reset_buffer;
    } else if (subp == 2) {
        val32.reg.PORT2_RESETf = reset_buffer;
    } else if (subp == 3) {
        val32.reg.PORT3_RESETf = reset_buffer;
    } else if (subp == 4) {
        val32.reg.PORT4_RESETf = reset_buffer;
    } else if (subp == 5) {
        val32.reg.PORT5_RESETf = reset_buffer;
    } else if (subp == 6) {
        val32.reg.PORT6_RESETf = reset_buffer;
    } else if (subp == 7) {
        val32.reg.PORT7_RESETf = reset_buffer;
    } else if (subp == 8) {
        val32.reg.PORT8_RESETf = reset_buffer;
    } else if (subp == 9) {
        val32.reg.PORT9_RESETf = reset_buffer;
    } else if (subp == 10) {
        val32.reg.PORT10_RESETf = reset_buffer;
    } else if (subp == 11) {
        val32.reg.PORT11_RESETf = reset_buffer;
    } else if (subp == 12) {
        val32.reg.PORT12_RESETf = reset_buffer;
    } else if (subp == 13) {
        val32.reg.PORT13_RESETf = reset_buffer;
    } else if (subp == 14) {
        val32.reg.PORT14_RESETf = reset_buffer;
    } else if (subp == 15) {
        val32.reg.PORT15_RESETf = reset_buffer;
    } else {
        val32.reg.PORT0_RESETf = reset_buffer;
    }

    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, reg, val32.word);

    printk("_helix5_idb_ca_reset_buffer port %d 0x%x pm_num %1d sbup=%1d reset_buffer=%1d  \n",
            port, val32.word, pm_num, subp,reset_buffer);
    return SOC_E_NONE;
}

//soc_helix5_idb_lpbk_ca_reset_buffer
static int
_helix5_idb_lpbk_ca_reset_buffer(bcmsw_switch_t *bcmsw, int reset_buffer)
{
    idb_lpbk_ca_t val32;

    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_LPBK_CONTROL_PIPE0r, &val32.word);
         
    val32.reg.PORT_RESETf = reset_buffer;

    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_LPBK_CONTROL_PIPE0r, val32.word);

    printk("_helix5_idb_lpbk_ca_reset_buffer reset_buffer=%1d", reset_buffer);

    return SOC_E_NONE;
}

//soc_helix5_idb_cpu_ca_reset_buffer
static  int
_helix5_idb_cpu_ca_reset_buffer(bcmsw_switch_t *bcmsw, int reset_buffer)
{
    idb_ca_cpu_t val32;

    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_CPU_CONTROL_PIPE0r, &val32.word);

    val32.reg.PORT_RESETf = reset_buffer;

    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_CPU_CONTROL_PIPE0r, val32.word);
    
    printk("_helix5_idb_cpu_ca_reset_buffer reset_buffer=%1d", reset_buffer);

    return SOC_E_NONE;
}

//soc_helix5_flex_idb_port_up
static int
_helix5_flex_idb_port_up(bcmsw_switch_t *bcmsw, int port)
{
    int reset_buffer;
    int phy_port;

    phy_port = bcmsw->si->port_l2p_mapping[port];

    /* Release IDB buffers from reset state for all the ports going up */

    reset_buffer = 1;
    if (HELIX5_PHY_IS_FRONT_PANEL_PORT(phy_port)) {
        _helix5_idb_obm_reset_buffer(bcmsw, port, reset_buffer);

        if(phy_port <49) {
                _helix5_idb_ca_reset_buffer(bcmsw, port, reset_buffer);
        } else if(phy_port==HELIX5_PHY_PORT_LPBK0) {
            _helix5_idb_lpbk_ca_reset_buffer(bcmsw, reset_buffer);
        } else if(phy_port==HELIX5_PHY_PORT_CPU) {
            _helix5_idb_cpu_ca_reset_buffer(bcmsw, reset_buffer);
        }
    }

    msleep(1);

    reset_buffer = 0;
    if (HELIX5_PHY_IS_FRONT_PANEL_PORT(phy_port)) {
        _helix5_idb_obm_reset_buffer(bcmsw, port, reset_buffer);

        if(phy_port <49) {
            _helix5_idb_ca_reset_buffer(bcmsw, port, reset_buffer);
        }else if(phy_port==HELIX5_PHY_PORT_LPBK0) {
            _helix5_idb_lpbk_ca_reset_buffer(bcmsw, reset_buffer);
        }else if(phy_port==HELIX5_PHY_PORT_CPU) {
            _helix5_idb_cpu_ca_reset_buffer(bcmsw, reset_buffer);
        }
    }
    return SOC_E_NONE;
}

// only valid for phy_port 1~49
static int 
_helix5_get_qmode(bcmsw_switch_t *bcmsw, int phy_port)
{
    int qmode;
    int blk_no;
    int32_t rval32;

    if (phy_port <= 16) {
        blk_no = SCHAN_BLK_PMQPORT0;
    } else if (phy_port <= 32) {
        blk_no = SCHAN_BLK_PMQPORT1; 
    } else {
        blk_no = SCHAN_BLK_PMQPORT2;
    }

    _reg32_read(bcmsw->dev, blk_no, CHIP_CONFIGr, &rval32);
    qmode = rval32 & 0x1;

    return qmode;
}

static const uint32_t gxblk[6] = {
    SCHAN_BLK_GXPORT0,
    SCHAN_BLK_GXPORT1,
    SCHAN_BLK_GXPORT2,
    SCHAN_BLK_GXPORT3,
    SCHAN_BLK_GXPORT4,
    SCHAN_BLK_GXPORT5,
};
//soc_helix5_flex_mac_port_up
int
_helix5_flex_mac_port_up(bcmsw_switch_t *bcmsw, int port)
{
    int i;
    //uint64_t rval64;
    uint32_t rval32;
    int phy_port;
    //int subp;
    int mode;
    int speed_100g;
    //int clport;
    //int higig2_mode;
    //int strict_preamble;
    //int higig_mode;
    int qmode;
    //int inst;
    int index;
    int speed_mode;
    //int hdr_mode;
    int blk_no;
    command_config_t ctrl;
    //static const int clport_mode_values[SOC_HX5_PORT_RATIO_COUNT] = {
    //    4, 3, 3, 3, 2, 2, 1, 1, 0
    //};
    struct net_device *dev;

    //strict_preamble = 0;

    dev = bcmsw->dev;

    /*Disable TSC lanes: */

    /*CLMAC_RX_CTRL */
    phy_port = bcmsw->si->port_l2p_mapping[port];
    //strict_preamble = 0;

    if(phy_port < 49) {
        qmode = _helix5_get_qmode(bcmsw, phy_port);
        index = (phy_port -1)%8;
    } else {
        qmode = 0;
    }
    printk("_helix5_flex_mac_port_up port %d qmode %d\n", port, qmode);

    if(phy_port < 65) {
        if((phy_port < 49) && (qmode)){
            /* configured thru API:  bcm_port_frame_max_set */
            blk_no = gxblk[(phy_port-1)/8];
            _reg32_read(bcmsw->dev, blk_no, GPORT_RSV_MASKr+index, &rval32);
    
            rval32 = 120;
            _reg32_write(bcmsw->dev, blk_no, GPORT_RSV_MASKr+index, rval32);
        }  else {
#if 0
	        printk("Bringing Eagle mac rx port %0d up\n", phy_port);

            _reg64_read(bcmsw->dev, blk_no, XLMAC_RX_CTRLr+index, &rval64);
            //soc_reg64_field32_set(unit, XLMAC_RX_CTRLr, &rval64, RX_ANY_STARTf, 0);
            //soc_reg64_field32_set(unit, XLMAC_RX_CTRLr, &rval64, STRIP_CRCf, 0);
            //soc_reg64_field32_set(unit, XLMAC_RX_CTRLr, &rval64, STRICT_PREAMBLEf,
            //                    strict_preamble);
            //soc_reg64_field32_set(unit, XLMAC_RX_CTRLr, &rval64, RUNT_THRESHOLDf,
            //                    64);
            _reg64_write(bcmsw->dev, blk_no, XLMAC_RX_CTRLr+index, rval64);
#endif            
	    }
	} else {
	    printk("Bringing Falcon mac rx port %0d up\n", phy_port);
#if 0        
        _reg64_read(bcmsw->dev, blk_no, CLMAC_RX_CTRLr+index, &rval64);
        //soc_reg64_field32_set(unit, CLMAC_RX_CTRLr, &rval64, RX_ANY_STARTf, 0);
        //soc_reg64_field32_set(unit, CLMAC_RX_CTRLr, &rval64, STRIP_CRCf, 0);
        //soc_reg64_field32_set(unit, CLMAC_RX_CTRLr, &rval64, STRICT_PREAMBLEf,
        //                        strict_preamble);
        //    soc_reg64_field32_set(unit, CLMAC_RX_CTRLr, &rval64, RUNT_THRESHOLDf,
        //                        64);
        _reg64_write(bcmsw->dev, blk_no, CLMAC_RX_CTRLr+index, rval64);
#endif        
	}

    /*CLPORT_CONFIG */
    if(phy_port < 65) { 
        if ( qmode != 1 ){
	  printk("Setting Eagle mac xl port %0d up\n", phy_port);
#if 0
            _reg32_read(bcmsw->dev, blk_no, XLPORT_CONFIGr+index, &rval32);

            soc_reg_field_set(unit, XLPORT_CONFIGr, &rval32, HIGIG2_MODEf,
                              higig2_mode);
            soc_reg_field_set(unit, XLPORT_CONFIGr, &rval32, HIGIG_MODEf,
                              higig_mode);
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, XLPORT_CONFIGr,
                                                      phy_port, 0, rval32));
#endif
        }
    } else {	
        printk("Setting Falcon mac cl port %0d up\n", phy_port);
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_get(unit, CLPORT_CONFIGr,
                                                      phy_port, 0, &rval32));
            soc_reg_field_set(unit, CLPORT_CONFIGr, &rval32, HIGIG2_MODEf,
                              higig2_mode);
            soc_reg_field_set(unit, CLPORT_CONFIGr, &rval32, HIGIG_MODEf,
                              higig_mode);
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, CLPORT_CONFIGr,
                                                      phy_port, 0, rval32));
#endif                                                      
    }

    /*CLPORT Enable: */	   
    if(phy_port < 65) { 
        if( qmode != 1 ) {
	    printk("Setting Eagle enable port %0d up\n", phy_port);
#if 0            
            _reg32_read(bcmsw->dev, blk_no, XLPORT_ENABLE_REGr+index, &rval32);

            if (subp == 0) {
                soc_reg_field_set(unit, XLPORT_ENABLE_REGr, &rval32, PORT0f, 1);
            } else if (subp==1) {
                soc_reg_field_set(unit, XLPORT_ENABLE_REGr, &rval32, PORT1f, 1);
            } else if (subp==2) {
                soc_reg_field_set(unit, XLPORT_ENABLE_REGr, &rval32, PORT2f, 1);
            } else {
                soc_reg_field_set(unit, XLPORT_ENABLE_REGr, &rval32, PORT3f, 1);
            }
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, XLPORT_ENABLE_REGr,
                                                      phy_port, 0, rval32));
#endif                                                      
	    }
	} else {
            printk("Setting Falcon enable port %0d up\n", phy_port);
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_get(unit, CLPORT_ENABLE_REGr,
                                                      phy_port, 0, &rval32));
            if (subp == 0) {
                soc_reg_field_set(unit, CLPORT_ENABLE_REGr, &rval32, PORT0f, 1);
            } else if (subp==1) {
                soc_reg_field_set(unit, CLPORT_ENABLE_REGr, &rval32, PORT1f, 1);
            } else if (subp==2) {
                soc_reg_field_set(unit, CLPORT_ENABLE_REGr, &rval32, PORT2f, 1);
            } else {
                soc_reg_field_set(unit, CLPORT_ENABLE_REGr, &rval32, PORT3f, 1);
            }
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, CLPORT_ENABLE_REGr,
                                                      phy_port, 0, rval32));
#endif                                                      
	}

    msleep(1);

    /*CLPORT mode: */
    if (bcmsw->si->port_init_speed[port] == 100000) {
        speed_100g = 1;
    } else {
        speed_100g = 0;
    }
#if 0  	   
	if(phy_port < 65 ){ 
      
	    if ( qmode != 1) {
			LOG_DEBUG(BSL_LS_SOC_PORT, (BSL_META_U(unit, "Setting mode port %0d %0d %d\n"), phy_port, mode, clport_mode_values[mode]));
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_get(unit, XLPORT_MODE_REGr,
                                                      phy_port, 0, &rval32));
            soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval32,
                              XPORT0_CORE_PORT_MODEf,
                              clport_mode_values[mode]);
            soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval32,
                              XPORT0_PHY_PORT_MODEf,
                              clport_mode_values[mode]);
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, XLPORT_MODE_REGr,
                                                      phy_port, 0, rval32));
	    }
	} else {
			LOG_DEBUG(BSL_LS_SOC_PORT, (BSL_META_U(unit, "Setting mode port %0d %0d %d\n"), phy_port, mode, clport_mode_values[mode]));
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_get(unit, CLPORT_MODE_REGr,
                                                      phy_port, 0, &rval32));
            soc_reg_field_set(unit, CLPORT_MODE_REGr, &rval32,
                              SINGLE_PORT_MODE_SPEED_100Gf,
                              speed_100g);
            soc_reg_field_set(unit, CLPORT_MODE_REGr, &rval32,
                              XPORT0_CORE_PORT_MODEf,
                              clport_mode_values[mode]);
            soc_reg_field_set(unit, CLPORT_MODE_REGr, &rval32,
                              XPORT0_PHY_PORT_MODEf,
                              clport_mode_values[mode]);
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, CLPORT_MODE_REGr,
                                                      phy_port, 0, rval32));
	}
    sleep(200);
#endif    

    /* Release soft reset */
    if(phy_port <65) {
        if((phy_port < 49) && (qmode)){
            index = (phy_port-1)%8;
            blk_no = gxblk[(phy_port-1)/8];
    
            _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
            ctrl.reg.SW_RESETf = 0;
            _reg32_write(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, ctrl.word);
        } else {
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, XLMAC_CTRLr, phy_port,
                                                    0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, SOFT_RESETf, 0);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, XLMAC_CTRLr, phy_port,
                                                    0, rval64));
#endif                                                    
        }
    } else {
#if 0        
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, CLMAC_CTRLr, phy_port,
                                                    0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, SOFT_RESETf, 0);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_CTRLr, phy_port,
                                                    0, rval64));
            /*
             * Special handling for new mac version. Internally MAC loopback
             * looks for rising edge on MAC loopback configuration to enter
             * loopback state.
             * Do only if loopback is enabled on the port.
             */
            if (soc_reg64_field32_get(unit, CLMAC_CTRLr, rval64, LOCAL_LPBKf)) {
                soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, LOCAL_LPBKf, 0);
                SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_CTRLr, phy_port,
                                                        0, rval64));
                /* Wait 10usec as suggested by MAC designer */
                sal_udelay(10);
                soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, LOCAL_LPBKf, 1);
                SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_CTRLr, phy_port,
                                                        0, rval64));
        }
#endif
    }

    msleep(1);

    /* Enable MAC RX_EN & TX_EN */
    if(phy_port < 65) {
        if((phy_port < 49) && (qmode)){
            index = (phy_port-1)%8;
            blk_no = gxblk[(phy_port-1)/8];

            _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
            ctrl.reg.RX_ENAf = 1;
            ctrl.reg.TX_ENAf = 1;
            _reg32_write(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, ctrl.word);

            //TODO
            //SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, FLUSH_CONTROLr, phy_port,
            //                                        0, &rval64));
            //soc_reg64_field32_set(unit, FLUSH_CONTROLr, &rval64, FLUSHf, 0);
            //SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, FLUSH_CONTROLr, phy_port,
            //                                        0, rval64));
						    
	    } else {
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, XLMAC_TX_CTRLr,
                                                    phy_port, 0, &rval64));
            soc_reg64_field32_set(unit, XLMAC_TX_CTRLr, &rval64, DISCARDf, 0);
            soc_reg64_field32_set(unit, XLMAC_TX_CTRLr, &rval64, EP_DISCARDf, 0);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, XLMAC_TX_CTRLr,
                                                    phy_port, 0, rval64));

            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, XLMAC_CTRLr, phy_port,
                                                    0, &rval64));
            soc_reg64_field32_set(unit, XLMAC_CTRLr, &rval64, RX_ENf, 1);
            soc_reg64_field32_set(unit, XLMAC_CTRLr, &rval64, TX_ENf, 1);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, XLMAC_CTRLr, phy_port,
                                                    0, rval64));
#endif                                                    
	    }
	} else {
#if 0        
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, CLMAC_TX_CTRLr,
                                                    phy_port, 0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_TX_CTRLr, &rval64, DISCARDf, 0);
            soc_reg64_field32_set(unit, CLMAC_TX_CTRLr, &rval64, EP_DISCARDf, 0);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_TX_CTRLr,
                                                    phy_port, 0, rval64));

            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, CLMAC_CTRLr, phy_port,
                                                    0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, RX_ENf, 1);
            soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, TX_ENf, 1);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_CTRLr, phy_port,
                                                    0, rval64));
#endif                                                    
	}

    /* CLMAC MODE */
    //hdr_mode = 0;
    // case 10   : speed_mode = 0;
    // case 100  : speed_mode = 1;
    // case 1000 : speed_mode = 2; 
    // case 2500 : speed_mode = 3;
    // default   : speed_mode = 4;
    if (bcmsw->si->port_init_speed[port] == 1000) {
        speed_mode = 2;
    } else {
        speed_mode = 4;
    }

    if(phy_port < 65) {
        if((phy_port < 49) && (qmode)){
            index = (phy_port-1)%8;
            blk_no = gxblk[(phy_port-1)/8];

            _reg32_read(dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
            ctrl.reg.ETH_SPEEDf = speed_mode;
            _reg32_write(dev, blk_no, COMMAND_CONFIGr+index, ctrl.word);

            _reg32_read(dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
            //printk("soc_helix5_flex_mac_port_up port %d command_config 0x%x\n", phy_port, ctrl.word);
	 } else {
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, XLMAC_MODEr,
                                                    phy_port, 0, &rval64));
            soc_reg64_field32_set(unit, XLMAC_MODEr, &rval64, HDR_MODEf, hdr_mode);
            soc_reg64_field32_set(unit, XLMAC_MODEr, &rval64, SPEED_MODEf, speed_mode);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, XLMAC_MODEr,
                                                    phy_port, 0, rval64));
#endif                                                    
	  }
    } else {
#if 0        
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, CLMAC_MODEr,
                                                    phy_port, 0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_MODEr, &rval64, HDR_MODEf, hdr_mode);
            soc_reg64_field32_set(unit, CLMAC_MODEr, &rval64, SPEED_MODEf, speed_mode);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_MODEr,
                                                    phy_port, 0, rval64));
#endif                                                    
    }

	printk("END soc_helix5_flex_mac_port_up\n");
    return SOC_E_NONE;
}

//soc_helix5_flex_en_forwarding_traffic
int
_helix5_flex_en_forwarding_traffic(bcmsw_switch_t *bcmsw, int port)
{
    uint32_t ing_entry[3]; //72 bit
    uint32_t epc_entry[3]; //72 bit

    _soc_mem_read(bcmsw->dev, ING_DEST_PORT_ENABLEm, SCHAN_BLK_IPIPE, 3, ing_entry); 

    ing_entry[port>>5] |= (0x1<<(port&0x1f));

    _soc_mem_write(bcmsw->dev, ING_DEST_PORT_ENABLEm, SCHAN_BLK_IPIPE, 3, ing_entry); 

    //printk("Enable ING_DEST_PORT_ENABLE write:: 0x%08x 0x%08x 0x%08x\n",
    //       ing_entry[0],ing_entry[1], ing_entry[2]);

    /* EPC_LINK_BMAP read, field modify and write. */
    _soc_mem_read(bcmsw->dev, EPC_LINK_BMAPm, SCHAN_BLK_IPIPE, 3, epc_entry); 

    epc_entry[port>>5] |= (0x1<<(port&0x1f));

    _soc_mem_write(bcmsw->dev, EPC_LINK_BMAPm, SCHAN_BLK_IPIPE, 3, epc_entry); 

    //printk("Enable EPC_LINK_BITMAP write:: 0x%08x 0x%08x 0x%08x\n",
    //        epc_entry[0],epc_entry[1], epc_entry[2]);

    return SOC_E_NONE;
}

//soc_helix5_ep_flexport_sft_rst_ports
static int
_helix5_ep_flexport_sft_rst_ports(bcmsw_switch_t *bcmsw, int port, int rst_on)
{
    int physical_port;
    uint32 entry;
    int qmode;

    //int port_rst_serviced[HELIX5_PHY_PORTS_PER_DEV];

    if (rst_on == 1) {
        entry = 1;
    } else {
        entry = 0;
    }

    physical_port = bcmsw->si->port_l2p_mapping[port];

    /* For ports going DOWN  or UP do:
     * Assert(rst_on=1)/De-assert(rst_on=0) per port sft reset
     */

    //sal_memset(port_rst_serviced, 0, sizeof(port_rst_serviced));
    /* need to implement skipping edatbuff reset if the buffer is for
     * gport as credits will not be re-issued due flex 
     */
    if (physical_port < 49) {
        qmode = _helix5_get_qmode(bcmsw, physical_port);                           
    } else {
        qmode = 0;
    }

    if ((physical_port < 49) && (qmode)) {
        entry = 0;
    }

    /* If physical_port index was written once don't do it again
     * Note that there may be two writes to the same physical port
     * More sbus efficiency by tracking which phy indexes thar are
     * already written
     */
    //if (0 == port_rst_serviced[physical_port]) {
    //        port_rst_serviced[physical_port] = 1;
    //}

    //soc_mem_field_set(unit, mem, entry, ENABLEf, &memfld);
    _soc_mem_write(bcmsw->dev, EGR_PER_PORT_BUFFER_SFT_RESETm, SCHAN_BLK_EPIPE, 1, &entry); 

    //readback for verification
    entry = 0;
    _soc_mem_write(bcmsw->dev, EGR_PER_PORT_BUFFER_SFT_RESETm, SCHAN_BLK_EPIPE, 1, &entry); 
    //printk("_helix5_ep_flexport_sft_rst_ports port %d entry %d\n", port, entry);

    return SOC_E_NONE;
}

//soc_helix5_ep_enable_disable
static int
_helix5_ep_enable_disable(bcmsw_switch_t *bcmsw, int port, int down_or_up)
{
    uint32 entry;
    int physical_port;

    physical_port = bcmsw->si->port_l2p_mapping[port];

    entry = (0 == down_or_up)?0:1;

    _soc_mem_write(bcmsw->dev, EGR_ENABLEm+physical_port, SCHAN_BLK_EPIPE, 1, &entry); 

    _soc_mem_read(bcmsw->dev, EGR_ENABLEm+physical_port, SCHAN_BLK_EPIPE, 1, &entry); 
    //printk("_helix5_ep_enable_disable port %d entry %d\n", port, entry);

    return SOC_E_NONE;
}

//soc_helix5_flex_ep_port_up
static int
_helix5_flex_ep_port_up(bcmsw_switch_t *bcmsw, int port)
{
    int rst_on, down_or_up;
    int physical_port;

    /*
     * Release EDB port buffer reset and enable cell request generation in EP
     * Set EGR_PER_PORT_BUFFER_SFT_RESET[device_port] to 0
     * Set EGR_ENABLE[device_port].PRT_ENABLE to 1
     */

    /* De-assert PM intf sft_reset */
    rst_on = 0;
    _helix5_ep_flexport_sft_rst_ports(bcmsw, port, rst_on);

    /* Enable Ports going up after PM sft_rst is de-asserted */
    /* For ports going UP do:
     * 1. Enable port; write EGR_ENABLEm
     */
    down_or_up = 1; /* that is, port UP */
    physical_port = bcmsw->si->port_l2p_mapping[port];
    if (-1 != physical_port) { /* that is, port UP */
        /* Enable port; write EGR_ENABLEm */
        _helix5_ep_enable_disable(bcmsw, port, down_or_up);
    }

    return SOC_E_NONE;
}

//soc_helix5_mmu_vbs_port_flush
static int
_helix5_mmu_vbs_port_flush(bcmsw_switch_t *bcmsw, int port, uint64 set_val)
{
    uint32_t reg1, reg2;
    uint64_t enable_val_0,enable_val_1;
    int physical_port;
    int mmu_port, lcl_mmu_port;
    int update0,update1;
    uint64_t new_val_0,new_val_1;
    uint64_t temp64;

    reg1 = Q_SCHED_PORT_FLUSH_SPLIT0r;
    reg2 = Q_SCHED_PORT_FLUSH_SPLIT1r;

    
    /* READ MODIFY WRITE IN SW ... Hence get Register
       Value and Then Write ... */

    COMPILER_64_ZERO(enable_val_0);
    COMPILER_64_ZERO(enable_val_1);
    COMPILER_64_ZERO(temp64);
    
    //acc_type = 20
    _schan_reg64_read(bcmsw->dev, SCHAN_BLK_MMU_SC, reg1, &enable_val_0, 20);

    _schan_reg64_read(bcmsw->dev, SCHAN_BLK_MMU_SC, reg2, &enable_val_1, 20);

    update0 = 0;
    update1 = 0;
    
    COMPILER_64_SET(new_val_0, 0, 1);
    COMPILER_64_SET(new_val_1, 0, 1);

    physical_port = bcmsw->si->port_l2p_mapping[port];
    if (physical_port != -1) {
        mmu_port = bcmsw->si->port_p2m_mapping[physical_port];
    } else {
        printk("_helix5_mmu_vbs_port_flush invalid phy_port - port %d\n",port);
    }

    lcl_mmu_port = mmu_port % HX5_MMU_PORT_PIPE_OFFSET;
    temp64 += HX5_MMU_FLUSH_ON;

    if (lcl_mmu_port < 64) {
        //COMPILER_64_SHL(new_val_0, lcl_mmu_port);
	new_val_0 <<= lcl_mmu_port;

        if (set_val == temp64) {
	    if (physical_port == -1) {
                COMPILER_64_OR(enable_val_0, new_val_0);
                update0 = 1;
            }
        } else {
	    if (physical_port != -1){
                COMPILER_64_NOT(new_val_0);
	        COMPILER_64_AND(enable_val_0, new_val_0);
	        update0 = 1;
	    }
        }   
    } else {
        //COMPILER_64_SHL(new_val_1, (lcl_mmu_port - 64));
	new_val_1 <<= (lcl_mmu_port - 64);
	        
        if (set_val == temp64) {
	    if (physical_port == -1){
                COMPILER_64_OR(enable_val_1, new_val_1);
                update1 = 1; 
            }
        } else {
	    if (physical_port != -1){
 	       COMPILER_64_NOT(new_val_1);
	       COMPILER_64_AND(enable_val_1, new_val_1);
	       update1 = 1; 
	    }
        }
    }

    if(update0 == 1) {
	printk("Q_SCHED_PORT_FLUSH_SPLIT0r 0x%llx\n", enable_val_0);
        _schan_reg64_write(bcmsw->dev, SCHAN_BLK_MMU_SC, reg1, enable_val_0, 20);
    }
    if(update1 == 1) {
	printk("Q_SCHED_PORT_FLUSH_SPLIT1r 0x%llx\n", enable_val_1);
        _schan_reg64_write(bcmsw->dev, SCHAN_BLK_MMU_SC, reg2, enable_val_1, 20);
    }

    return SOC_E_NONE;
}



//soc_helix5_mmu_rqe_port_flush
static int
_helix5_mmu_rqe_port_flush(bcmsw_switch_t *bcmsw, int port, uint64 set_val)
{
    q_sched_rqe_t reg_rqe;
    int count=0;

    //acc_type = 20
    _schan_reg32_read(bcmsw->dev, SCHAN_BLK_MMU_SC, Q_SCHED_RQE_SNAPSHOTr, &reg_rqe.word, 20);

    reg_rqe.reg.INITIATEf = set_val;
    _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_SC, Q_SCHED_RQE_SNAPSHOTr, reg_rqe.word, 20);

    while (1) {
        _schan_reg32_read(bcmsw->dev, SCHAN_BLK_MMU_SC, Q_SCHED_RQE_SNAPSHOTr, &reg_rqe.word, 20);

        if (reg_rqe.reg.INITIATEf == 0) {
            break;
        }
        msleep(1);
        count++;
        if (count > 60) {
            printk("Initiate isn't reset even after 60ms port %d \n", port);
            return SOC_E_FAIL;
        }
    }
    return SOC_E_NONE;
}

//soc_helix5_mmu_mtro_port_flush
static int
_helix5_mmu_mtro_port_flush(bcmsw_switch_t *bcmsw, int port, uint64 set_val)
{
    uint32_t reg1,reg2;
    int mmu_port, lcl_mmu_port;
    int physical_port;
    uint64_t enable_val_0;
    uint64_t enable_val_1;

    reg1 = MTRO_PORT_ENTITY_DISABLE_SPLIT0r;
    reg2 = MTRO_PORT_ENTITY_DISABLE_SPLIT1r;

    physical_port = bcmsw->si->port_l2p_mapping[port];
    if (physical_port != -1) {
        mmu_port = bcmsw->si->port_p2m_mapping[physical_port];
    } else {
        printk("_helix5_mmu_mtro_port_flush invalid phy_port - port %d\n",port);
    }

    lcl_mmu_port = mmu_port % HX5_MMU_PORT_PIPE_OFFSET;
    
    /* READ MODIFY WRITE IN SW ... Hence get
       Register Value and Then Write ..  */

    if (lcl_mmu_port < 64) {
        //acc_type = 20
        _schan_reg64_read(bcmsw->dev, SCHAN_BLK_MMU_SED, reg1, &enable_val_0, 20);

        if (set_val == 0) {
            COMPILER_64_BITCLR(enable_val_0, lcl_mmu_port);
        }
        else {
            COMPILER_64_BITSET(enable_val_0, lcl_mmu_port);
        }
        _schan_reg64_write(bcmsw->dev, SCHAN_BLK_MMU_SED, reg1, enable_val_0, 20);
    } else {
        //acc_type = 20
        _schan_reg64_read(bcmsw->dev, SCHAN_BLK_MMU_SED, reg2, &enable_val_1, 20);

        if (set_val == 0) {
            COMPILER_64_BITCLR(enable_val_1, lcl_mmu_port - 64);
        }
        else {
            COMPILER_64_BITSET(enable_val_1, lcl_mmu_port - 64);
        }
        _schan_reg64_write(bcmsw->dev, SCHAN_BLK_MMU_SED, reg2, enable_val_1, 20);
    }

    return SOC_E_NONE;
}


//soc_helix5_flex_mmu_port_up_top
static int
_helix5_flex_mmu_port_up_top(bcmsw_switch_t *bcmsw, int port)
{
    int qmode;
    int inst;
    int pipe;
    int pipe_flexed;
    uint64 temp64;
    uint64 rval64;
    int phy_port;

    COMPILER_64_SET(temp64, 0, HX5_MMU_FLUSH_OFF);

    /* Per-Port configuration */
    //for (pipe = 0; pipe < HELIX5_TDM_PIPES_PER_DEV; pipe++) {
    //    soc_helix5_mmu_get_pipe_flexed_status(
	//    unit, port_schedule_state_t, pipe, &pipe_flexed);
	    
	//if (pipe_flexed == 1) {
    _helix5_mmu_vbs_port_flush(bcmsw, port, temp64);
    _helix5_mmu_rqe_port_flush(bcmsw, port, temp64);
	//}
    //}

    /* Per-Pipe configuration */
    phy_port = bcmsw->si->port_l2i_mapping[port];

    /* Clear Previous EP Credits on the port. */
    if(phy_port < 49){
        qmode = _helix5_get_qmode(bcmsw, phy_port);
    } else {
	    qmode = 0;
    }

    if(qmode == 0 ) { //TODO
        //soc_helix5_mmu_clear_prev_ep_credits(
        //        unit, &port_schedule_state_t->resource[port]);
    }

    _helix5_mmu_mtro_port_flush(bcmsw, port, temp64);
	        
    return SOC_E_NONE;
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
_bcm_port_ability_local_get(bcmsw_switch_t *bcmsw, int port,
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
    return SOC_E_NONE;
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

static int phy_bcm542xx_dev_init(bcmsw_switch_t *bcmsw, int port)
{
    //_phy_bcm54282_init
    soc_info_t  *si = bcmsw->si;
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
phy_bcm542xx_reset_setup(bcmsw_switch_t *bcmsw,
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
    //int     oui = 0, model = 0, rev = 0;
    soc_info_t  *si = bcmsw->si;
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

    phy_bcm542xx_dev_init(bcmsw, port);

    return SOC_E_NONE;

}

static int
phy_bcm542xx_init_setup( bcmsw_switch_t *bcmsw,
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
    soc_info_t  *si = bcmsw->si;
    port_info_t *p_port;
    uint32_t     phy_id;
    int          dev_port;

    p_port = &si->ports[port];

    if ( reset ) {
        phy_bcm542xx_reset_setup(bcmsw, port, automedium,
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


static int phy_bcm542xx_init(bcmsw_switch_t *bcmsw, int port)
{
    //int  fiber_capable = 0;
    int  automedium = 0;
    int  fiber_detect = 0;
    int  fiber_enable = 0;
    int  fiber_preferred = 0;
    int  copper_enable = TRUE;
    int  ext_phy_autodetect_en = 0;
    int  ext_phy_fiber_iface = -1;

    /* Init PHYS and MACs to defaults */
    phy_bcm542xx_init_setup(bcmsw, port, 
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
    return 0;
}

/*****************************************************************************************/
/*                             Port Init/Setup                                           */
/*****************************************************************************************/

/*****************************************************************************************/
/*                             Port Init/Setup -- PHY                                    */
/*****************************************************************************************/


static int 
_phyctrl_probe(bcmsw_switch_t *bcmsw, int port)
{
    soc_info_t *si = bcmsw->si;

    // probe for PHY , and dump information
    // Only do ext Phy (N3248TE)
    if (si->ports[port].ext_phy_addr !=  -1) {
        _ext_phy_probe(bcmsw, port);
    }

    return 0;
}

static int _phyctrl_init(bcmsw_switch_t *bcmsw, int port)
{
    soc_info_t *si = bcmsw->si;
    port_info_t *p_port;

    p_port = &si->ports[port];

    printk("entered soc_phyctrl_init: port %d , probed %d\n", port, p_port->probed);

    if(p_port->probed) {  
       //Call Phy Model specific init -> pd_init = phy_bcm542xx_init
       phy_bcm542xx_init(bcmsw, port);

       p_port->phy_flags |= PHY_FLAGS_INIT_DONE;
    }

    return 0;
}

static int 
_phyctrl_pbm_probe_init(bcmsw_switch_t *bcmsw)
{
    soc_info_t *si = bcmsw->si;
    int port; 
    int num_port = HX5_NUM_PORT;

    for (port = 0; port < num_port; port++) {
        if(si->ports[port].valid == TRUE) {
            _phyctrl_probe(bcmsw, port);

            /* do PHY init pass1 */
            PHYCTRL_INIT_STATE_SET(&(si->ports[port].phy_ctrl), PHYCTRL_INIT_STATE_PASS1);
            _phyctrl_init(bcmsw, port);

            // BCM54182 only need init pass 1
            /* do PHY init pass2 - 5  if requested */
          
            PHYCTRL_INIT_STATE_SET(&(si->ports[port].phy_ctrl), PHYCTRL_INIT_STATE_DEFAULT);
        }
    }
    return 0;
}

/*****************************************************************************************/
/*                             Port Ctrl                                                 */
/*****************************************************************************************/

static int _pm4x10_qtc_port_enable_set(bcmsw_switch_t *bcmsw, int port, int enable)
{
    soc_info_t *si = bcmsw->si;

    port_info_t *pport = &si->ports[port];

    return phy_bcm542xx_enable_set(pport, port, pport->ext_phy_addr, enable);
}

static int 
_bcm_esw_portctrl_enable_set(bcmsw_switch_t *bcmsw, int port, int enable)
{
    int mac_reset;
    //if (flags & PORTMOD_PORT_ENABLE_PHY) {
        //portmod_port_enable_set();
        _pm4x10_qtc_port_enable_set(bcmsw, port, 1);
    //}

    //portmod_ext_to_int_cmd_set() ==> Not needed

    //_soc_link_update
    //Check if MAC needs to be modified based on whether
    //(portmod_port_mac_reset_check(unit, pport,
    //    enable, &mac_reset));
    unimac_reset_check(bcmsw, port, enable, &mac_reset);

    if(mac_reset) {
        //if (flags & PORTMOD_PORT_ENABLE_MAC ) {
        //(BCM_ESW_PORT_DRV(unit)->port_enable_set(unit, -> bcmi_hx5_port_enable ->soc_helix5_flex_top_port_up
        printk("_bcm_esw_portctrl_enable_set port %d mac_reset\n", port);
        
        /* soc_helix5_flex_top_port_up*/
        // 1 soc_helix5_flex_mmu_port_up_top
        _helix5_flex_mmu_port_up_top(bcmsw, port);

        // 2 soc_helix5_flex_ep_port_up
        _helix5_flex_ep_port_up(bcmsw, port);

        // 3 soc_helix5_flex_idb_port_up()
        _helix5_flex_idb_port_up(bcmsw, port);
 
        // 4 soc_helix5_flex_mac_port_up
        _helix5_flex_mac_port_up(bcmsw, port);

        // 5 soc_helix5_flex_en_forwarding_traffic
        _helix5_flex_en_forwarding_traffic(bcmsw, port);

    }

    return 0;
}

static int 
bcmi_esw_portctrl_probe(void)
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
bcmi_esw_portctrl_probe_pbmp(bcmsw_switch_t *bcmsw)
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
_bcm_port_speed_set(bcmsw_switch_t *bcmsw, int port, int speed)
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
bcm_esw_port_speed_set(bcmsw_switch_t *bcmsw, int port, int speed)
{
    int rv;
    rv = _bcm_port_speed_set(bcmsw, port, speed);

    //bcm_esw_port_enable_set
    //bcm_esw_link_change
    //_bcm_esw_port_link_delay_update

    return rv;
}
int bcm_esw_port_autoneg_set(bcmsw_switch_t *bcmsw, int port, int speed)
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
bcm_esw_port_enable_set(bcmsw_switch_t *bcmsw, int port, int enable)
{
   //bcmi_esw_portctrl_enable_set
   //if (enable) {
      // enable PHY
      //rv = _bcm_esw_portctrl_enable_set(bcmsw, port, PORTMOD_PORT_ENABLE_PHY, TRUE);

      /* Get link status after PHY state has been set */
      //rv = bcm_esw_port_link_status_get(unit, port, &link);

      // enable MAC
      //rv = _bcm_esw_portctrl_enable_set(bcmsw, port, PORTMOD_PORT_ENABLE_MAC, TRUE);
   //} else {


   //}
   return _bcm_esw_portctrl_enable_set(bcmsw, port, enable);
}

static int
bcm_port_settings_init(bcmsw_switch_t *bcmsw, int port)
{
    int         rc;
    soc_info_t *si = bcmsw->si;

    rc = bcm_esw_port_speed_set(bcmsw, port, si->port_init_speed[port]);

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

    rc = bcm_esw_port_autoneg_set(bcmsw, port, TRUE);

    return rc;
}


static int 
_port_init(bcmsw_switch_t *bcmsw)
{
    int num_port = HX5_NUM_PORT, port, vid; 
    soc_info_t *si = bcmsw->si;
    int index;

    vid = BCMSW_VLAN_DEFAULT;
    for (port = 0; port < num_port; port++) {
       if(si->port_type[port] != -1) {
           //bcm_td3_port_cfg_init
           _port_cfg_init(bcmsw, port, vid);
	  
       }
    }
    // STEP 1 
    /* 
     * Initialize inner TPID
     * (WAR)Set to 0x9100 then change back to take effort
     */
    //TODO

    // STEP 2 
    // clear egress port blocking table MAC_BLOCKm
    for (index = 0; index <= 31; index++) {
        _soc_mem_write(bcmsw->dev, MAC_BLOCKm+index, SCHAN_BLK_IPIPE, 3, empty_entry); 
    }    

    // STEP 3 - Probe for Ports -> bcm_esw_port_probe
    /* Probe the PHY and set up the PHY and MAC for the specified ports.
     * bcm_esw_port_probe(unit, PBMP_PORT_ALL(unit), &okay_ports);
     */
    /* Check for PortMod */
    bcmi_esw_portctrl_probe_pbmp(bcmsw);

    /*soc_phyctrl_pbm_probe_init */
    _phyctrl_pbm_probe_init(bcmsw);

    // Probe function should leave port disabled 
    //soc_phyctrl_enable_set

    // STEP 4
    for (port = 0; port < num_port; port++) {
        if(si->ports[port].valid == TRUE) {
            bcm_port_settings_init(bcmsw, port);
        }
     }    

    //enable ports
    // if ((rv = bcm_esw_port_enable_set(unit, p, port_enable)) < 0) {
    for (port =1; port < num_port; port++) {
	    if(si->ports[port].valid == TRUE) {
            bcm_esw_port_enable_set(bcmsw, port, TRUE);
	    }
    }
    return 0;
}


static int bcmsw_port_create(bcmsw_switch_t *bcmsw, int port_index, const char *name)
{
    int len;
    //int qnum;
    kcom_msg_netif_create_t netif_create;
    //struct bcmsw_port * port = bcmsw->ports[port_index];

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
    netif_create.netif.phys_port = bcmsw->si->port_l2p_mapping[port_index];

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
static int bcmsw_ports_init(bcmsw_switch_t *bcmsw)
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

    // initialize port configuration according to soc info. bcm_esw_port_init bcm_td3_port_cfg_init
    err = _port_init(bcmsw);
    if (err) {
        goto err_port_cfg_init;
    }

    //call knet to create interface
    max_ports = COUNTOF(n3248te_ports);
    for (i = 0; i < max_ports; i++) {
        if (n3248te_ports[i].port != -1) {
    	  err = bcmsw_port_create(bcmsw, n3248te_ports[i].port, n3248te_ports[i].name);
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
static int bcmsw_cmicx_dma_abort(bcmsw_switch_t *bcmsw)
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
static int _misc_init(bcmsw_switch_t *bcmsw)
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


static int 
_soc_hx5_thdo_hw_set(bcmsw_switch_t *bcmsw, int port, int enable)
{
    uint64_t rval64, rval64_tmp;
    int i;
    int split, pos, phy_port, mmu_port;
    soc_info_t *si = bcmsw->si;

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

       if (pos < 32) {
            COMPILER_64_SET(rval64_tmp, 0, (1 << pos));
        } else {
            COMPILER_64_SET(rval64_tmp, (1 << (pos - 32)), 0);
        }

        _reg64_read(bcmsw->dev, SCHAN_BLK_MMU_XPE, reg[i][split], &rval64);                                  

        if (enable) {
	    COMPILER_64_OR(rval64, rval64_tmp);
        } else {
	    COMPILER_64_NOT(rval64_tmp);
	    COMPILER_64_AND(rval64, rval64_tmp);
        }
	//printk("_soc_hx5_thdo_hw_set port %d pos %d reg 0x%x 0x%llx\n",port, pos, reg[i][split], rval64);

        _reg64_write(bcmsw->dev, SCHAN_BLK_MMU_XPE, reg[i][split], rval64);
    }
   
    return SOC_E_NONE;
}

//_soc_helix5_mmu_init
static int _mmu_init(bcmsw_switch_t *bcmsw)
{
    uint32 val;
    int num_port = HX5_NUM_PORT, port; 
    soc_info_t *si = bcmsw->si;

    for (port = 0; port < num_port; port++) {
       if(si->port_l2p_mapping[port] != -1) {
           _soc_hx5_thdo_hw_set(bcmsw, port, 1);
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
    _reg32_write(bcmsw->dev,SCHAN_BLK_MMU_XPE, WRED_REFRESH_CONTROLr, val);

    //readback
    _reg32_read(bcmsw->dev,SCHAN_BLK_MMU_XPE, WRED_REFRESH_CONTROLr, &val);
    //printk("_mmu_init WRED_REFRESH_CONTROLr = 0x%x\n", val);


    //soc_trident3_sc_reg32_get(unit, MMU_1DBG_Cr, 0, 0, 0, &rval));
    //soc_reg_field_set(unit, MMU_1DBG_Cr, &rval, FIELD_Af, 1);
    //soc_trident3_sc_reg32_set(unit, MMU_1DBG_Cr, -1, -1, 0, rval));
    val = 0x29;
    _schan_reg32_write(bcmsw->dev,SCHAN_BLK_MMU_SC, MMU_1DBG_Cr, val, 20);

    //readback
    _schan_reg32_read(bcmsw->dev,SCHAN_BLK_MMU_SC, MMU_1DBG_Cr, &val, 20);
    //printk("_mmu_init MMU_1DBG_Cr = 0x%x\n", val);
    
    // (soc_trident3_sc_reg32_set(unit, MMU_2DBG_C_1r, -1, -1, 0, 0x4));
    val = 0x4;
    _schan_reg32_write(bcmsw->dev,SCHAN_BLK_MMU_SC, MMU_2DBG_C_1r, val, 20);

    //readback
    _schan_reg32_read(bcmsw->dev,SCHAN_BLK_MMU_SC, MMU_2DBG_C_1r, &val, 20);
    //printk("_mmu_init MMU_2DBG_C_1r = 0x%x\n", val);


    return 0;
}

/*****************************************************************************************/
/*                             switch   L2                                               */
/*****************************************************************************************/

/*
 * Function:
 *      _bcm_fb_l2_to_l2u
 * Purpose:
 *      Convert a hardware-independent L2 cache entry to a L2 User table entry.
 * Parameters:
 *      unit - Unit number
 *      l2u_entry - (OUT) Firebolt L2 User entry
 *      l2caddr - Hardware-independent L2 entry
 * Returns:
 *      BCM_E_XXX
 */
static int
_bcm_l2_cache_to_l2u(l2u_entry_t *l2u_entry, bcm_l2_cache_addr_t *l2caddr)
{
    int       isGport;
    int       int_pri_max;
    int32_t   val  = 0;
    uint16_t  vlan;
    uint32_t  mac_field[2];
    
    if ((_BCM_VPN_IS_SET(l2caddr->vlan) != _BCM_VPN_IS_SET(l2caddr->vlan_mask)) && (l2caddr->vlan_mask)) {
        return SOC_E_PARAM;
    }

    /* If VPN specified do not perform vlan id check */
    if (!_BCM_VPN_IS_SET(l2caddr->vlan)) {
        VLAN_CHK_ID(l2caddr->vlan);
        VLAN_CHK_ID(l2caddr->vlan_mask);
    }

    if (l2caddr->flags & BCM_L2_CACHE_SETPRI) {
        int_pri_max = 15; //Trident3
        if (l2caddr->prio < 0 || l2caddr->prio > int_pri_max) {
                return SOC_E_PARAM;
        }

        // /SOC_IS_TRIDENT3X
        if ((l2caddr->flags & BCM_L2_CACHE_BPDU) &&
            !(l2caddr->flags & BCM_L2_CACHE_CPU) &&
            !(l2caddr->flags & BCM_L2_CACHE_DISCARD)) {
                printk("L2Cache: SETPRI is not supported for BPDU packet in this device, please use FP\n");
            }
    }

    if (BCM_GPORT_IS_SET(l2caddr->dest_port) ||
        BCM_GPORT_IS_SET(l2caddr->src_port)) {
        //TODO
        //BCM_IF_ERROR_RETURN(_bcm_l2_cache_gport_resolve(unit, l2caddr));
        printk("_bcm_l2_cache_to_l2u isGport\n");
        isGport = 1;
    } else {
        isGport = 0;
    }

    memset(l2u_entry, 0, sizeof (*l2u_entry));

    //soc_L2_USER_ENTRY_BCM56370_A0m_fields
    val = 1; 
    //VALIDf bit start 0, len 1
    _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 0, 1, &val, 0);

    vlan = l2caddr->vlan;
    if (_BCM_VPN_IS_SET(l2caddr->vlan)) {
        //KEY_TYPEf bit start 63, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 63, 1, &val, 0);

        _BCM_VPN_GET(vlan, _BCM_VPN_TYPE_VFI, l2caddr->vlan);
    }
    
    //KEY_TYPE_MASKf  bit start 143, len 1
    val = 1;
    _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 143, 1, &val, 0);

    if (l2caddr->flags & BCM_L2_CACHE_PROTO_PKT) {
        //L2_PROTOCOL_PKTf bit start 207, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 207, 1, &val, 0);
    }

    //VLAN_IDf bit start 49, len 12
    val = vlan;;
    _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 49, 12, &val, SOCF_LE);

    //VLAN_ID_MASKf start 129, len 12
    val = l2caddr->vlan_mask;
    _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 129, 12, &val, SOCF_LE);


    //MAC_ADDRf start 1, len 48
    MAC_ADDR_TO_UINT32(l2caddr->mac, mac_field);
    _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 1, 48, mac_field, SOCF_LE);

    //MAC_ADDR_MASKf start 81, len 48
    MAC_ADDR_TO_UINT32(l2caddr->mac_mask, mac_field);
    _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 81, 48, mac_field, SOCF_LE);

    if (l2caddr->flags & BCM_L2_CACHE_SETPRI) {
        //PRIf bit start 169, len 4
        val = l2caddr->prio;
        _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 169, 4, &val, SOCF_LE);
        //RPEf bit start 174, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 174, 1, &val, 0);
    }

    if (l2caddr->flags & BCM_L2_CACHE_CPU) {
        //CPUf bit start 175, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 175, 1, &val, 0);
    }

    if (l2caddr->flags & BCM_L2_CACHE_BPDU) {
        //BPDUf bit start 206, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 206, 1, &val, 0);
    }

    if (l2caddr->flags & BCM_L2_CACHE_DISCARD) {
        //DST_DISCARDf bit start 176, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 176, 1, &val, 0);
    }

#if 0
    if (l2caddr->flags & BCM_L2_CACHE_TRUNK) {
        soc_mem_field32_dest_set(unit, L2_USER_ENTRYm, l2u_entry,
                        DESTINATIONf, SOC_MEM_FIF_DEST_LAG, l2caddr->dest_trunk);
    } else {
        port_field = PORT_NUMf;
    }

    if (!((l2caddr->flags & BCM_L2_CACHE_TRUNK) ||
            (l2caddr->flags & BCM_L2_CACHE_MULTICAST))) {
            mod_in = l2caddr->dest_modid;
            port_in = l2caddr->dest_port;
            if (!isGport) {
                //PORT_DUALMODID_VALID(unit, port_in);
            }
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                        mod_in, port_in, &mod_out, &port_out));
            if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
                return BCM_E_BADID;
            }
            if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
                return BCM_E_PORT;
            }

            if (soc_feature(unit, soc_feature_generic_dest)) {
                soc_mem_field32_dest_set(unit, L2_USER_ENTRYm, l2u_entry,
                    DESTINATIONf, SOC_MEM_FIF_DEST_DGPP,
                    mod_out << SOC_MEM_FIF_DGPP_MOD_ID_SHIFT_BITS | port_out);
            } else {
                _l2u_field32_set(unit, l2u_entry, MODULE_IDf, mod_out);
                _l2u_field32_set(unit, l2u_entry, port_field, port_out);
            }
        }
        

        if ((l2caddr->flags & BCM_L2_CACHE_MULTICAST) &&
            !_BCM_VPN_IS_SET(l2caddr->vlan)) {
            if (_BCM_MULTICAST_IS_SET(l2caddr->group)) {
                if (_BCM_MULTICAST_IS_L2(l2caddr->group)) {
                        soc_mem_field32_dest_set(unit, L2_USER_ENTRYm, l2u_entry,
                                DESTINATIONf, SOC_MEM_FIF_DEST_L2MC,
                                _BCM_MULTICAST_ID_GET(l2caddr->group));
                } else {
                    return BCM_E_PARAM;
                }
            }
        }
#endif    
        if (l2caddr->flags & BCM_L2_CACHE_L3) {
            //RESERVED_0f start 173, len 1
            val = 1;
            _mem_field_set((uint32_t *)l2u_entry, sizeof (*l2u_entry), 173, 1, &val, 0);
        }

        return SOC_E_NONE;

}

// soc_l2u_find_free_entry
static int 
_soc_l2u_find_free_entry(bcmsw_switch_t *bcmsw, l2u_entry_t *key, int *free_index)
{
    l2u_entry_t entry, free_mask;
    int index, i, entry_words, rv;
    int start, end, step;
    uint32_t val;
    uint32_t mask[5];

    entry_words = 7; //soc_mem_entry_words(unit, L2_USER_ENTRYm);

    memset(&free_mask, 0, sizeof(free_mask));
    val = 1;
    //VALIDf start 0, len 1
    _mem_field_set((uint32_t *)&free_mask, sizeof (free_mask), 0, 1, &val, 0);

    //MASKf start 81, len 80
    _mem_field_get((uint32_t *)key, sizeof (*key), 81, 80, mask, SOCF_LE);

    if (mask[0] == 0xffffffff && (mask[1] & 0xffff) == 0xffff) {
        /* Search from high priority end */
        start = 0;
        end = 512;
        step = 1;
    } else {
        start = 511;
        end = -1;
        step = -1;
    }
    for (index = start; index != end; index += step) {
        rv = _soc_mem_read(bcmsw->dev, L2_USER_ENTRYm + index, SCHAN_BLK_IPIPE, 7, &entry);
        if (rv == 0) {
            for (i = 0; i < entry_words; i++) {
                if (entry.entry_data[i] & free_mask.entry_data[i]) {
                    break;
                }
            }
            if (i == entry_words) {
                *free_index = index;
                return SOC_E_NONE;
            }
        }
    }

    return SOC_E_FULL;
}

/*
 * Function:
 *      soc_l2u_insert
 * Purpose:
 *      Add entry to L2 User table
 */
//soc_l2u_insert
static int 
_soc_l2u_insert(bcmsw_switch_t *bcmsw, l2u_entry_t *entry, int index, int *index_used)
{
    int i, i_max, i_min, rv;

    //BCM56370 0 - 511
    i_min = 0;
    i_max = 0x1ff;

    if (index == -1) {

        //soc_mem_lock(unit, L2_USER_ENTRYm);

        /* Avoid duplicates */
        //rv = soc_l2u_search(unit, entry, &l2u_entry, &i);
        //if (rv != SOC_E_NOT_FOUND) {
        //    soc_mem_unlock(unit, L2_USER_ENTRYm);
        //    *index_used = i;
        //    return rv;
        //}

        rv = _soc_l2u_find_free_entry(bcmsw, entry, &i);
        //soc_mem_unlock(unit, L2_USER_ENTRYm);
        if (rv) {
            printk("_soc_l2u_insert failed to find free entry\n");
            return rv;
        }
        index = i;

    } else if (index < i_min || index > i_max) {
        return SOC_E_PARAM;
    }

    printk("_soc_l2u_insert free entry = %d\n", index);
    //soc_mem_lock(unit, L2_USER_ENTRYm);

    //sal_memcpy(&l2u_entry, entry, sizeof(l2u_entry));
    //rv = WRITE_L2_USER_ENTRYm(unit, MEM_BLOCK_ALL, index, &l2u_entry);
    // size = (27 byte +3)/4
    _soc_mem_write(bcmsw->dev, L2_USER_ENTRYm + index, SCHAN_BLK_IPIPE, 7, entry);

    //soc_mem_unlock(unit, L2_USER_ENTRYm);

    *index_used = index;

    return rv;
}

/*
 * Function:
 *      bcm_esw_l2_cache_init
 * Purpose:
 *      Initialize the L2 cache
 */
int
_bcm_esw_l2_cache_init(bcmsw_switch_t *bcmsw)
{
    bcm_l2_cache_addr_t addr;
    l2u_entry_t entry;
    int index;
    uint32_t val;

        //if (!SAL_BOOT_QUICKTURN && !SAL_BOOT_XGSSIM) {
        //    SOC_IF_ERROR_RETURN
        //        (soc_mem_clear(unit, L2_USER_ENTRYm, COPYNO_ALL, TRUE));
        //}

    /* Enable L2 entry used as my station hit */
    //TODO CCH register
    //my_station_config = 0;
    //SOC_IF_ERROR_RETURN(READ_ING_CONFIG_64r(unit, &regval64));
    //if (my_station_config != soc_reg64_field32_get(unit, ING_CONFIG_64r,
    //            regval64, L2_ENTRY_USED_AS_MY_STATIONf)) {
    //            BCM_IF_ERROR_RETURN
    //               (soc_reg_field32_modify(unit, ING_CONFIG_64r, REG_PORT_ANY,
    //                         L2_ENTRY_USED_AS_MY_STATIONf, my_station_config));
    //}

    memset(&addr, 0, sizeof(bcm_l2_cache_addr_t));

    addr.flags = BCM_L2_CACHE_CPU | BCM_L2_CACHE_BPDU;

    /* Set default BPDU addresses (01:80:c2:00:00:00) */
    memcpy(addr.mac, _mac_spanning_tree, sizeof(_mac_t));
    memcpy(addr.mac_mask, _mac_all_ones, sizeof(_mac_t));

    addr.dest_modid = 0;
    addr.dest_port = 0; //CMIC_PORT(unit);

    _bcm_l2_cache_to_l2u(&entry, &addr);

    _soc_l2u_insert(bcmsw, &entry, -1, &index);

    //L2_PROTOCOL_PKTf start 207, len 1
    val = 1;
    _mem_field_set((uint32_t *)&entry, sizeof (entry), 207, 1, &val, 0);

    //KEY_TYPEf start 63, len 1
    val = 1;
    _mem_field_set((uint32_t *)&entry, sizeof (entry), 63, 1, &val, 0);

    //VFI_MASKf start 129, len 12
    val = 0;
    _mem_field_set((uint32_t *)&entry, sizeof (entry), 129, 12, &val, 0);
    _soc_l2u_insert(bcmsw, &entry, -1, &index);

    /* Set 01:80:c2:00:00:10 */
    addr.mac[5] = 0x10;
    _bcm_l2_cache_to_l2u(&entry, &addr);
    _soc_l2u_insert(bcmsw, &entry, -1, &index);
    
    /* Set 01:80:c2:00:00:0x */
    addr.mac[5] = 0x00;
    addr.mac_mask[5] = 0xf0;
    _bcm_l2_cache_to_l2u(&entry, &addr);
    _soc_l2u_insert(bcmsw, &entry, -1, &index);

    /* Set 01:80:c2:00:00:2x */
    addr.mac[5] = 0x20;
    _bcm_l2_cache_to_l2u(&entry, &addr);
    _soc_l2u_insert(bcmsw, &entry, -1, &index);

    return SOC_E_NONE;
}


//bcm_esw_l2_init
int
_esw_l2_init(bcmsw_switch_t *bcmsw)
{
    int rv; 

    /*
     * Init L2 cache
     */
    rv = _bcm_esw_l2_cache_init(bcmsw);
    
    return SOC_E_NONE;
}

/*****************************************************************************************/
/*                             switch  VLAN                                              */
/*****************************************************************************************/
 
//bcm_td3_vlan_vfi_untag_init
int
_td3_vlan_vfi_untag_init(bcmsw_switch_t *bcmsw, uint16_t vid, _pbmp_t pbmp)
{
    egr_vlan_vfi_untag_entry_t egr_vlan_vfi;
    vlan_tab_entry_t egr_vtab;
    uint32 profile_ptr = 0;

    _soc_mem_read(bcmsw->dev, EGR_VLANm+vid, SCHAN_BLK_EPIPE, 3, &egr_vtab); 

    //UNTAG_PROFILE_PTRf start 22 len 12
    _mem_field_get((uint32_t *)&egr_vtab, EGR_VLANm_BYTES, 22, 12, &profile_ptr, SOCF_LE);

    printk("_td3_vlan_vfi_untag_init vid %d profile_ptr before init %d\n", vid, profile_ptr);

    // 1 to 1 mapping of vid and profile 
    profile_ptr = vid;

    //read EGR_VLAN_VFI_UNTAGm  19 bytes 5 words
    _soc_mem_read(bcmsw->dev, EGR_VLAN_VFI_UNTAGm+profile_ptr, SCHAN_BLK_EPIPE, 5, &egr_vlan_vfi); 

    //UT_PORT_BITMAPf start 0 len 72                
    _mem_field_set((uint32_t *)&egr_vlan_vfi, EGR_VLAN_VFI_UNTAGm_BYTES, 0, 72, &pbmp, SOCF_LE);                           

    _soc_mem_write(bcmsw->dev, EGR_VLAN_VFI_UNTAGm+profile_ptr, SCHAN_BLK_EPIPE, 5, &egr_vlan_vfi); 

    //UNTAG_PROFILE_PTRf start 22 len 12
    _mem_field_set((uint32_t *)&egr_vtab, EGR_VLANm_BYTES, 22, 12, &profile_ptr, SOCF_LE);

    _soc_mem_write(bcmsw->dev, EGR_VLANm+vid, SCHAN_BLK_EPIPE, 3, &egr_vtab); 

    return SOC_E_NONE;
}

// _bcm_xgs3_vlan_table_init
static int
_vlan_table_init_egr_vlan(bcmsw_switch_t *bcmsw, vlan_data_t *vd)
{
    vlan_tab_entry_t    ve;
    vlan_attrs_1_entry_t vlan_attrs;
    //int                 rv;
    int                 index;
    uint32_t            val;
    uint16_t            tpid;
    int                 tpid_index = 0;    

    // Clear EGR_VLANm
    for (index = 0; index <= 4095; index++) {
        _soc_mem_write(bcmsw->dev, EGR_VLANm+index, SCHAN_BLK_EPIPE, 3, empty_entry); 
    }    

    //EGR_VLANm entry is 10 bytes, 3 word
    memcpy(&ve, empty_entry, 12);

    /* This function will be called with VLAN_TABm, EGR_VLANm, and VLAN_2_TABm
     * for TH2 and TH3. In TH2/TH3, VLAN_2_TABm has STGf while VLAN_TABm does
     * not. The check is to prevent STGf being set wrongly.
     */
    //STGf start 1, len 9
    val = 1; //default STG
    _mem_field_set((uint32_t *)&ve, EGR_VLANm_BYTES, 1, 9, &val, SOCF_LE);
    
    //VALIDf start 0, len 1
    val = 1;
    _mem_field_set((uint32_t *)&ve, EGR_VLANm_BYTES, 0, 1, &val, SOCF_LE);


    tpid = 0; //_bcm_fb2_outer_tpid_default_get(unit);

    /* Add the default outer TPID entry twice during init so that
     * the default entry does not get removed even when no table 
     * entry is referencing to the default TPID entry.
     */  
    //_bcm_fb2_outer_tpid_entry_add(bcmsw, tpid, &tpid_index);
    //_bcm_fb2_outer_tpid_entry_add(bcmsw, tpid, &tpid_index);

    //OUTER_TPID_INDEXf start 10, len 2
    val = tpid_index;
    _mem_field_set((uint32_t *)&ve, EGR_VLANm_BYTES, 10, 2, &val, SOCF_LE);

    memset(&vlan_attrs, 0, sizeof(vlan_attrs_1_entry_t));

    //VLAN_ATTRS_1m 9 bytes, 3 words
    _soc_mem_read(bcmsw->dev, VLAN_ATTRS_1m+vd->vlan_tag, SCHAN_BLK_IPIPE, 3, &vlan_attrs); 

    //STGf start 2, len 9
    val = 1; //default STG
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 2, 9, &val, SOCF_LE);

    //VALIDf start 58, len 1
    val = 1;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 58, 1, &val, 0);

    //FID_IDf start 24, len 12
    val = 0;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 24, 12, &val, SOCF_LE);

    //MPLS_ENABLEf start 0, len 1
    val = 1;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 0, 1, &val, 0);

    //MIM_TERM_ENABLEf start 1, len 1
    val = 1;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 1, 1, &val, 0);

    //EN_IFILTERf start 23, len 1
    val = 1;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 23, 1, &val, 0);

    _soc_mem_write(bcmsw->dev, VLAN_ATTRS_1m+vd->vlan_tag, SCHAN_BLK_IPIPE, 3, &vlan_attrs); 



    _soc_mem_write(bcmsw->dev, EGR_VLANm+vd->vlan_tag, SCHAN_BLK_EPIPE, 3, &ve); 

    //readback 
    //printk("_vlan_table_init_egr_vlan write %d 0x%08x 0x%08x 0x%08x\n", 
    //    	    vd->vlan_tag,
    //    	    ve.entry_data[0],
    //    	    ve.entry_data[1],
    //    	    ve.entry_data[2]);
    //memcpy(&ve, empty_entry, 12);
    //_soc_mem_read(bcmsw->dev, EGR_VLANm+vd->vlan_tag, SCHAN_BLK_EPIPE, 3, &ve);
    //printk("_vlan_table_init_egr_vlan read %d 0x%08x 0x%08x 0x%08x\n", 
    //		    vd->vlan_tag,
    //		    ve.entry_data[0],
    //		    ve.entry_data[1],
    //		    ve.entry_data[2]);
 

    _td3_vlan_vfi_untag_init(bcmsw, vd->vlan_tag, vd->ut_port_bitmap);

    return SOC_E_NONE;
}

//_bcm_xgs3_vlan_table_init
static int
_vlan_table_init_vlan_tab(bcmsw_switch_t *bcmsw, vlan_data_t *vd)
{
    vlan_tab_entry_t    ve;
    //bcm_pbmp_t          pbm;
    uint32_t            empty_entry[SOC_MAX_MEM_WORDS] = {0};
    int                 rv;
    uint32_t            val;
    int                 index;

    // Clear VLAN_TABm
    for (index = 0; index <= 4095; index++) {
        _soc_mem_write(bcmsw->dev, VLAN_TABm+index, SCHAN_BLK_IPIPE, 12, empty_entry); 
    }    

    //VLAN_TABm entry is 47 bytes, 12 word
    memcpy(&ve, empty_entry, 12);

#if 0 
    TBD    
    pbm = vd->ut_port_bitmap);
    pbm |= PBMP_E_ALL(unit)

    //PORT_BITMAPf start 0 len 72
    val = vd->port_bitmap;
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 0, 72, &val, SOCF_LE);
#endif    

    //STGf start 141 len 9
    val = 1;  //default STG
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 141, 9, &val, SOCF_LE);

    //VALIDf start 150, len 1
    val = 1;
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 150, 1, &val, 0);

    //EN_IFILTERf start 296, len 1
    val = 1;
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 296, 1, &val, 0);

    //L3_IIFf start 318, len 13
    val = vd->vlan_tag;
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 318, 13, &val, 0);

    _soc_mem_write(bcmsw->dev, VLAN_TABm+vd->vlan_tag, SCHAN_BLK_IPIPE, 12, &ve); 

    return SOC_E_NONE;
}

//bcm_xgs3_vlan_init
int
_xgs3_vlan_init(bcmsw_switch_t *bcmsw, vlan_data_t *vd)
{
    /* Must call on EGR_VLANm before VLAN_TABm */

    //EGR_VLANm
    _vlan_table_init_egr_vlan(bcmsw, vd); 

    //VLAN_TABm
    _vlan_table_init_vlan_tab(bcmsw, vd);

    return SOC_E_NONE;
}


/*
 * Function:
 *      bcm_vlan_init
 * Purpose:
 *      Initialize the VLAN module.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 */

 int
 bcm_esw_vlan_init(bcmsw_switch_t *bcmsw)
 {
    int         rv = SOC_E_NONE;
    vlan_data_t vd;
 
    /*
     * Initialize hardware tables
    */
 

    memset(&vd, 0, sizeof(vlan_data_t));
    /*
     * LB ports to be member of vlan, like front panel ports.
     */
    vd.vlan_tag = BCM_VLAN_DEFAULT;
    //BCM_PBMP_ASSIGN(vd.port_bitmap, temp_pbmp);
    //BCM_PBMP_REMOVE(vd.port_bitmap, PBMP_LB(unit));
    //BCM_PBMP_REMOVE(vd.port_bitmap, PBMP_RDB_ALL(unit));
    //BCM_PBMP_REMOVE(vd.port_bitmap, PBMP_FAE_ALL(unit));
    //BCM_PBMP_ASSIGN(vd.ut_port_bitmap, temp_pbmp);
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_CMIC(unit));
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_LB(unit));
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_RDB_ALL(unit));
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_FAE_ALL(unit));
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_MACSEC_ALL(unit));
    vd.port_bitmap.pbits[0] = 0xffffffff;
    vd.port_bitmap.pbits[1] = 0x023fffff;
    vd.ut_port_bitmap.pbits[0] = 0xffffffff;
    vd.ut_port_bitmap.pbits[1] = 0x023fffff;

    //(mbcm_driver[unit]->mbcm_vlan_init(unit, &vd)); //bcm_xgs3_vlan_init
    _xgs3_vlan_init(bcmsw, &vd);
 
 
     return rv;
 }

/*****************************************************************************************/
/*                             /proc                                                     */
/*****************************************************************************************/
static struct proc_dir_entry *proc_switchdev_base = NULL;
static struct proc_dir_entry *proc_reg_base = NULL;
static struct proc_dir_entry *mem_reg_base = NULL;
static struct proc_dir_entry *stats_reg_base = NULL;


// /proc/switchdev/sinfo

static int
_sinfo_show(struct seq_file *m, void *v)
{
    int index;
    soc_info_t *si;

    if (!_bcmsw) {
        seq_printf(m, " Not initialized\n");
	return 0;
    }

    si = _bcmsw->si;

    if (!si) {
	seq_printf(m, " si Not initialized\n"); 
	return 0;
    }

    seq_printf(m, "SOC INFO for BCM56371:\n");
    seq_printf(m, "   port    l2p    p2l    l2i    p2m    m2p   pipe  serdes\n");

    for (index =0; index< HX5_NUM_PORT; index++) {
        seq_printf(m, " %6i %6i %6i %6i %6i %6i %6i %6i\n",
                index,
                si->port_l2p_mapping[index],
                si->port_p2l_mapping[index],
                si->port_l2i_mapping[index],
                si->port_p2m_mapping[index],
                si->port_m2p_mapping[index],
                si->port_pipe[index],
                si->port_serdes[index]);
    }
    return 0;
}

static ssize_t _sinfo_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
    printk("_sinfo_write handler\n");
    return -1;
}

static int _sinfo_open(struct inode * inode, struct file * file)
{
    return single_open(file, _sinfo_show, NULL);
}


static struct proc_ops sinfo_ops = 
{
    proc_open:       _sinfo_open,
    proc_read:       seq_read,
    proc_lseek:     seq_lseek,
    proc_write:      _sinfo_write,
    proc_release:    single_release,
};

// /proc/switchdev/reg/COMMAND_CONFIGr
static int
_command_config_show(struct seq_file *m, void *v)
{
    int index;
    uint32_t val;

    if (!_bcmsw) {
        seq_printf(m, " Not initialized\n");
	    return 0;
    }

    seq_printf(m, "COMMAND_CONFIG base 0x%x:\n", COMMAND_CONFIGr);

    for (index =0; index < 6; index ++) {
        _reg32_read(_bcmsw->dev, gxblk[index], COMMAND_CONFIGr+index, &val);
        seq_printf(m, "%d  0x%08x\n", index, val);
    }   

    return 0;
}

static ssize_t _command_config_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
    printk("_command_config_write handler\n");
    return -1;
}

static int _comman_config_open(struct inode * inode, struct file * file)
{
    return single_open(file, _command_config_show, NULL);
}


static struct proc_ops command_config_ops = 
{
    proc_open:       _comman_config_open,
    proc_read:       seq_read,
    proc_lseek:     seq_lseek,
    proc_write:      _command_config_write,
    proc_release:    single_release,
};

// /proc/switchdev/mem/EGR_VLAN
static int
_egr_vlan_show(struct seq_file *m, void *v)
{
    int                 index;
    uint32_t            val;
    vlan_tab_entry_t    ve;

    if (!_bcmsw) {
        seq_printf(m, " Not initialized\n");
	    return 0;
    }

    seq_printf(m, "EGR_VLAN base 0x%x (10 bytes):\n", EGR_VLANm);

    for (index = 0; index < 4095; index ++) {
        //EGR_VLANm entry is 10 bytes, 3 word
        _soc_mem_read(_bcmsw->dev, EGR_VLANm+index, SCHAN_BLK_EPIPE, 3, &ve); 

        //VALIDf start 0, len 1
        _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 0, 1, &val, 0);

        if (val & 0x1) {
            seq_printf(m, "[%4d] RAW 0x%08x 0x%08x 0x%08x\n", index, ve.entry_data[0], ve.entry_data[1], ve.entry_data[2]);
            // dump field 
            // { STGf, 9, 1, SOCF_LE | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 1, 9, &val, SOCF_LE);
            seq_printf(m, "                      STG %d\n", val);

            // { OUTER_TPID_INDEXf, 2, 10, SOCF_LE | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 10, 2, &val, SOCF_LE);
            seq_printf(m, "         OUTER_TPID_INDEX %d\n", val);

            // { DOT1P_MAPPING_PTRf, 4, 12, SOCF_LE | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 12, 4, &val, SOCF_LE);
            seq_printf(m, "        DOT1P_MAPPING_PTR %d\n", val);

            // { REMARK_CFIf, 1, 16, 0 | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 16, 1, &val, 0);
            seq_printf(m, "               REMARK_CFI %d\n", val);

            // { REMARK_DOT1Pf, 1, 17, 0 | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 17, 1, &val, 0);
            seq_printf(m, "             REMARK_DOT1P %d\n", val);

            // { UNTAG_PROFILE_PTRf, 12, 22, SOCF_LE | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 22, 12, &val, SOCF_LE);
            seq_printf(m, "        UNTAG_PROFILE_PTR %d\n", val);

            // { FLEX_CTR_BASE_COUNTER_IDXf, 11, 34, SOCF_LE | SOCF_GLOBAL },

            // { RSVD_FLEX_CTR_BASE_COUNTER_IDXf, 3, 45, SOCF_LE|SOCF_RES | SOCF_GLOBAL },

            // { FLEX_CTR_POOL_NUMBERf, 2, 48, SOCF_LE | SOCF_GLOBAL },

            // { RSVD_FLEX_CTR_POOL_NUMBERf, 2, 50, SOCF_LE|SOCF_RES | SOCF_GLOBAL },

            // { FLEX_CTR_OFFSET_MODEf, 2, 52, SOCF_LE | SOCF_GLOBAL },

            // { MEMBERSHIP_PROFILE_PTRf, 12, 56, SOCF_LE | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 56, 12, &val, SOCF_LE);
            seq_printf(m, "   MEMBERSHIP_PROFILE_PTR %d\n", val);

            // { EN_EFILTERf, 1, 68, 0 | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 68, 1, &val, 0);
            seq_printf(m, "               EN_EFILTER %d\n", val);

            // { PARITYf, 1, 79, 0 | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&ve, EGR_VLANm_BYTES, 79, 1, &val, 0);
            seq_printf(m, "                   PARITY %d\n", val);            
            // { ECCf, 7, 72, SOCF_LE | SOCF_GLOBAL },
            // { ECCPf, 8, 72, SOCF_LE | SOCF_GLOBAL },
        }
    }
    return 0;
}

static ssize_t _egr_vlan_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
    printk("_egr_vlan_write handler\n");
    return -1;
}

static int _egr_vlan_open(struct inode * inode, struct file * file)
{
    return single_open(file, _egr_vlan_show, NULL);
}


static struct proc_ops egr_vlan_ops = 
{
    proc_open:       _egr_vlan_open,
    proc_read:       seq_read,
    proc_lseek:     seq_lseek,
    proc_write:      _egr_vlan_write,
    proc_release:    single_release,
};

// /proc/switchdev/mem/VLAN_ATTRS_1
static int
_vlan_attrs_1_show(struct seq_file *m, void *v)
{
    int                 index;
    uint32_t            val;
    vlan_attrs_1_entry_t vlan_attrs;

    if (!_bcmsw) {
        seq_printf(m, " Not initialized\n");
	    return 0;
    }

    seq_printf(m, "VLAN_ATTRS_1 base 0x%x (%d bytes):\n", VLAN_ATTRS_1m, VLAN_ATTRS_1m_BYTES);

    
    for (index = 0; index < 4095; index ++) {
        //VLAN_ATTRS_1 entry is 9 bytes, 3 word
        _soc_mem_read(_bcmsw->dev, VLAN_ATTRS_1m+index, SCHAN_BLK_IPIPE, 3, &vlan_attrs); 

        //VALIDf start 58, len 1
        _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 58, 1, &val, 0);

        if (val & 0x1) {
            seq_printf(m, "[%4d] RAW 0x%08x 0x%08x 0x%08x\n", index, 
                       vlan_attrs.entry_data[0], vlan_attrs.entry_data[1], vlan_attrs.entry_data[2]);
            // dump field 
            // { MPLS_ENABLEf, 1, 0, 0 | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 0, 1, &val, 0);
            seq_printf(m, "                   MPLS_ENABLE %d\n", val);

            // { MIM_TERM_ENABLEf, 1, 1, 0 | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 1, 1, &val, 0);
            seq_printf(m, "               MIM_TERM_ENABLE %d\n", val);         
            
            // { STGf, 9, 2, SOCF_LE | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 2, 9, &val, SOCF_LE);
            seq_printf(m, "                           STG %d\n", val);                

            // { MEMBERSHIP_PROFILE_PTRf, 12, 11, SOCF_LE | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 11, 12, &val, SOCF_LE);
            seq_printf(m, "        MEMBERSHIP_PROFILE_PTR %d\n", val);     

            // { EN_IFILTERf, 1, 23, 0 | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 23, 1, &val, 0);
            seq_printf(m, "                    EN_IFILTER %d\n", val);            

            //  { FID_IDf, 12, 24, SOCF_LE | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 24, 12, &val, SOCF_LE);
            seq_printf(m, "                        FID_ID %d\n", val);                 

            // { VLAN_CTRL_IDf, 4, 36, SOCF_LE | SOCF_GLOBAL }
            _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 36, 4, &val, SOCF_LE);
            seq_printf(m, "                  VLAN_CTRL_ID %d\n", val);         

            // { ACTIVE_L3_IIF_PROFILE_INDEXf, 10, 48, SOCF_LE | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 48, 10, &val, SOCF_LE);
            seq_printf(m, "   ACTIVE_L3_IIF_PROFILE_INDEX %d\n", val);   
            
            // { PARITYf, 1, 66, 0 | SOCF_GLOBAL },
            _mem_field_get((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 66, 1, &val, 0);
            seq_printf(m, "                        PARITY %d\n", val);   
            
        }
    }    
    return 0;
}

static ssize_t _vlan_attrs_1_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
    printk("_vlan_attrs_1_write handler\n");
    return -1;
}

static int _vlan_attrs_1_open(struct inode * inode, struct file * file)
{
    return single_open(file, _vlan_attrs_1_show, NULL);
}
static struct proc_ops vlan_attrs_1_ops = 
{
    proc_open:       _vlan_attrs_1_open,
    proc_read:       seq_read,
    proc_lseek:      seq_lseek,
    proc_write:      _vlan_attrs_1_write,
    proc_release:    single_release,
};

// /proc/switchdev/mem/VLAN_TAB
static int
_vlan_tab_show(struct seq_file *m, void *v)
{
    int                 index;
    uint32_t            val;
    vlan_tab_entry_t    vt;

    if (!_bcmsw) {
        seq_printf(m, " Not initialized\n");
	    return 0;
    }

    seq_printf(m, "VLAN_TAB base 0x%x (%d bytes):\n", VLAN_TABm, VLAN_TABm_BYTES);

    
    for (index = 0; index < 4095; index ++) {
        //VLAN_ATTRS_1 entry is 9 bytes, 3 word
        _soc_mem_read(_bcmsw->dev, VLAN_TABm+index, SCHAN_BLK_IPIPE, 12, &vt); 


        //VALIDf start 150, len 1
        _mem_field_get((uint32_t *)&vt, VLAN_TABm_BYTES, 150, 1, &val, 0);

        if (val & 0x1) {
            seq_printf(m, "[%4d] RAW 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", index, 
                       vt.entry_data[0], vt.entry_data[1], vt.entry_data[2], vt.entry_data[3], 
                       vt.entry_data[4], vt.entry_data[5], vt.entry_data[6], vt.entry_data[7],
                       vt.entry_data[8], vt.entry_data[9], vt.entry_data[10], vt.entry_data[11]);
            // dump field 
            //STGf start 141 len 9
            _mem_field_get((uint32_t *)&vt, VLAN_ATTRS_1m_BYTES, 141, 9, &val, SOCF_LE);
            seq_printf(m, "                           STG %d\n", val);

            //EN_IFILTERf start 296, len 1
            _mem_field_get((uint32_t *)&vt, VLAN_ATTRS_1m_BYTES, 296, 1, &val, 0);
            seq_printf(m, "                    EN_IFILTER %d\n", val);

            //L3_IIFf start 318, len 13
            _mem_field_get((uint32_t *)&vt, VLAN_ATTRS_1m_BYTES, 318, 13, &val, 0);
            seq_printf(m, "                        L3_IIF %d\n", val);
            
        }
    }    
    return 0;
}

static ssize_t _vlan_tab_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
    printk("_vlan_tab_write handler\n");
    return -1;
}

static int _vlan_tab_open(struct inode * inode, struct file * file)
{
    return single_open(file, _vlan_tab_show, NULL);
}
static struct proc_ops vlan_tab_ops = 
{
    proc_open:       _vlan_tab_open,
    proc_read:       seq_read,
    proc_lseek:      seq_lseek,
    proc_write:      _vlan_tab_write,
    proc_release:    single_release,
};

/*****************************************************************************************/

static int _procfs_init(bcmsw_switch_t *bcmsw)
{
    struct proc_dir_entry *entry;

    proc_switchdev_base = proc_mkdir("switchdev", NULL);

    if(proc_switchdev_base == NULL){
        printk("switchdev proc create switchdev failed\n");
        return -EINVAL;
    }

    // /proc/switchdev/sinfo
    entry = proc_create("sinfo", 0666, proc_switchdev_base, &sinfo_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/reg
    proc_reg_base = proc_mkdir("switchdev/reg", NULL);

    entry = proc_create("COMMAND_CONFIG", 0666, proc_reg_base, &command_config_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }


    // /proc/switchdev/mem
    mem_reg_base = proc_mkdir("switchdev/mem", NULL);

    entry = proc_create("EGR_VLAN", 0666, mem_reg_base, &egr_vlan_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    entry = proc_create("VLAN_ATTRS_1", 0666, mem_reg_base, &vlan_attrs_1_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    entry = proc_create("VLAN_TAB", 0666, mem_reg_base, &vlan_tab_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    
    // /proc/switchdev/stats
    stats_reg_base = proc_mkdir("switchdev/stats", NULL);

    //entry = proc_create("EGR_VLAN", 0666, stats_reg_base, &egr_vlan_ops);
    //if (entry == NULL) {
    //    printk("proc_create failed!\n");
    //    goto create_fail;
    //}
    return 0;

create_fail:
    proc_remove(proc_switchdev_base);
    return -EINVAL;
}

static int _procfs_uninit(bcmsw_switch_t *bcmsw)
{
    remove_proc_entry("sinfo", proc_switchdev_base);
    
    // /proc/switchdev/reg
    remove_proc_entry("COMMAND_CONFIG", proc_reg_base);
    remove_proc_entry("reg", proc_switchdev_base);

    // /proc/switchdev/mem
    remove_proc_entry("mem", proc_switchdev_base);

    // /proc/switchdev/stats
    remove_proc_entry("stats", proc_switchdev_base);

    remove_proc_entry("switchdev", NULL);
    return 0;
}

/*****************************************************************************************/
/*                            switch                                                     */
/*****************************************************************************************/

static int bcmsw_modules_init(bcmsw_switch_t *bcmsw)
{
    int err = 0;

    //create ports
    err = bcmsw_ports_init(bcmsw);
    if (err) {
    	//dev_err(mlxsw_sp->bus_info->dev, "Failed to create ports\n");
    	goto err_ports_create;
    }      


    //bcm_esw_l2_init
    _esw_l2_init(bcmsw);

    //bcm_esw_vlan_init
    bcm_esw_vlan_init(bcmsw);

    /* RX init is done in phases during early attach */
    //bcm_esw_rx_init
    //bcm_esw_tx_init


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
static int _switch_do_init(bcmsw_switch_t *bcmsw)
{
    struct net_device *dev = bcmsw->dev;
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
    int err = 0;
    lport_tab_entry_t lport_entry;
    soc_info_t *si;
    bcmsw_switch_t *bcmsw;
        
    bcmsw = kzalloc(sizeof(*bcmsw), GFP_KERNEL);
    if (!bcmsw)
	return -ENOMEM;
    //save to global variable 
    _bcmsw = bcmsw;

    err = bcmsw_switchdev_init(bcmsw);
    if (err)
	goto err_swdev_register;
  
    //get bcm0 netdev
    bcmsw->dev = __dev_get_by_name(current->nsproxy->net_ns, "bcm0");

    /* Connect to the kernel bde */
    if ((linux_bde_create(NULL, &kernel_bde) < 0) || kernel_bde == NULL) {
        return -ENODEV;
    }

    si = kzalloc(sizeof(soc_info_t), GFP_KERNEL);
    if (!si) {
        return -ENOMEM;
    }
 
    // initializa soc_info according to hardware information,soc_info_config & soc_helix5_port_config_init
    bcmsw_soc_info_init(si);
    bcmsw->si = si;

    //switch initialization 
    //BCM: init soc, schan is initialized 
    _switch_do_init(bcmsw);

    //load m0 firmware
    //BCM: m0 load 0 0x0 linkscan_led_fw.bin
    //	   m0 load 0 0x3800 custom_led.bin

    // init cancun, and load cancun pkgs
    //BCM: cancun load cch
    //     cancun load ceh
    //     cancun load cmh
    //     cancun load cih

    //misc_init
    //BCM: init misc, miim is initialized
    _misc_init(bcmsw);

    //init mmu
    _mmu_init(bcmsw);

    //initialize modules
    //BCM: init bcm
    bcmsw_modules_init(bcmsw);
  

    //test schan
    //err = _soc_mem_read(bcmsw->dev, 0x501c0000, SCHAN_BLK_IPIPE, 14, &lport_entry); 


    //test iproc reg read/write

    //procfs init
    _procfs_init(bcmsw);

err_swdev_register:
    return err;    
}

int bcmsw_switch_uninit(void)
{
    bcmsw_switch_t *bcmsw = _bcmsw;

     _procfs_uninit(bcmsw);
}
