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


static const struct {
    int port;
    int phy_port;
    int bandwidth;
    int port_type;
    char name[KCOM_NETIF_NAME_MAX];
    int lanes[4];
} n3248te_ports[] = {
    { 1,   1,  1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet0", {1, -1, -1, -1} }, 
    { 2,   2,  1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet1", {2, -1, -1, -1} }, 
    { 3,   3,  1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet2", {3, -1, -1, -1} }, 
    { 4,   4,  1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet3", {4, -1, -1, -1} }, 
    { 5,   5,  1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet4", {5, -1, -1, -1} }, 
    { 6,   6,  1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet5", {6, -1, -1, -1} }, 
    { 7,   7,  1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet6", {7, -1, -1, -1} }, 
    { 8,   8,  1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet7", {8, -1, -1, -1} }, 
    { 9,   0,  1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet8", {9, -1, -1, -1} }, 
    { 10,  10, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet9", {10, -1, -1, -1} }, 
    { 11,  11, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet10", {11, -1, -1, -1} }, 
    { 12,  12, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet11", {12, -1, -1, -1} }, 
    { 13,  13, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet12", {13, -1, -1, -1} },         
    { 14,  14, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet13", {14, -1, -1, -1} }, 
    { 15,  15, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet14", {15, -1, -1, -1} }, 
    { 16,  16, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet15", {16, -1, -1, -1} }, 
    { 17,  17, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet16", {17, -1, -1, -1} }, 
    { 18,  18, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet17", {18, -1, -1, -1} }, 
    { 19,  19, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet18", {19, -1, -1, -1} }, 
    { 20,  20, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet19", {20, -1, -1, -1} }, 
    { 21,  21, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet20", {21, -1, -1, -1} }, 
    { 22,  22, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet21", {22, -1, -1, -1} }, 
    { 23,  23, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet22", {23, -1, -1, -1} }, 
    { 24,  24, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet23", {24, -1, -1, -1} }, 
    { 25,  25, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet24", {25, -1, -1, -1} }, 
    { 26,  26, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet25", {26, -1, -1, -1} }, 
    { 27,  27, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet26", {27, -1, -1, -1} }, 
    { 28,  28, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet27", {28, -1, -1, -1} }, 
    { 29,  29, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet28", {29, -1, -1, -1} }, 
    { 30,  30, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet29", {30, -1, -1, -1} }, 
    { 31,  31, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet30", {31, -1, -1, -1} }, 
    { 32,  32, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet31", {32, -1, -1, -1} }, 
    { 33,  33, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet32", {33, -1, -1, -1} },        
    { 34,  34, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet33", {34, -1, -1, -1} }, 
    { 35,  35, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet34", {35, -1, -1, -1} }, 
    { 36,  36, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet35", {36, -1, -1, -1} }, 
    { 37,  37, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet36", {37, -1, -1, -1} }, 
    { 38,  38, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet37", {38, -1, -1, -1} }, 
    { 39,  39, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet38", {39, -1, -1, -1} }, 
    { 40,  40, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet39", {40, -1, -1, -1} }, 
    { 41,  41, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet40", {41, -1, -1, -1} }, 
    { 42,  42, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet41", {42, -1, -1, -1} }, 
    { 43,  43, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet42", {43, -1, -1, -1} }, 
    { 44,  44, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet43", {44, -1, -1, -1} }, 
    { 45,  45, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet44", {45, -1, -1, -1} }, 
    { 46,  46, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet45", {46, -1, -1, -1} }, 
    { 47,  47, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet46", {47, -1, -1, -1} }, 
    { 48,  48, 1,   BCMSW_PORT_TYPE_GXPORT, "Ethernet47", {48, -1, -1, -1} }, 
    { 49,  64, 10,  BCMSW_PORT_TYPE_XLPORT, "Ethernet48", {64, -1, -1, -1} }, 
    { 50,  63, 10,  BCMSW_PORT_TYPE_XLPORT, "Ethernet49", {63, -1, -1, -1} }, 
    { 51,  62, 10,  BCMSW_PORT_TYPE_XLPORT, "Ethernet50", {62, -1, -1, -1} }, 
    { 52,  61, 10,  BCMSW_PORT_TYPE_XLPORT, "Ethernet51", {61, -1, -1, -1} }, 
    { 53,  69, 100, BCMSW_PORT_TYPE_XLPORT, "Ethernet52", {69, 70, 71, 72} }, 
    { 57,  73, 100, BCMSW_PORT_TYPE_XLPORT, "Ethernet56", {73, 74, 75, 76} },   
    { -1,  -1, -1  },                                      
};

/*****************************************************************************************/
/*                              SCHAN                                                    */
/*****************************************************************************************/
#if 0
static int
bcmsw_schan_poll_wait(struct net_device *dev, schan_msg_t *msg)
{
    int rv = 0;
    uint32 schanCtrl;
    int schan_timeout  = 0; 

    while (((schanCtrl = bkn_dev_read32(dev, CMIC_SCHAN_CTRL)) &
            SC_MSG_DONE_TST) == 0) {
        udelay(1000);
        schan_timeout++;
        //300ms 
        if (schan_timeout >= 300 ) { 
	    gprintk(" failed after 300 polls\n");
            rv = -ETIME;
            break;
        }                
    }

    if (rv == 0) {
        gprintk("  Done in %d polls\n", schan_timeout);
    }

    if (schanCtrl & SC_MSG_NAK_TST) {
        rv = -EFAULT;

        gprintk("  NAK received from SCHAN.\n");

        //SOC_IF_ERROR_RETURN(_soc_cmice_schan_tr2_check_ser_nack(unit, msg));
    }

    //SOC_IF_ERROR_RETURN(_soc_cmice_schan_check_ser_parity(unit, &schanCtrl, msg));

    if (schanCtrl & SC_MSG_TIMEOUT_TST) {
        rv = -ETIME;
    }
    gprintk("schanCtrl is 0x %x\n", schanCtrl);

    bkn_dev_write32(dev, CMIC_SCHAN_CTRL, SC_MSG_DONE_CLR);

    return rv;
}
#endif

static int
bcmsw_schan_poll_wait(struct net_device *dev, schan_msg_t *msg, int ch)
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

    if (rv == 0) {
        gprintk("  Done in %d polls\n", schan_timeout);
    }

    gprintk("schanCtrl is 0x %x\n", schanCtrl);

    if (schanCtrl & SC_CHx_MSG_NAK) {
        rv = -EFAULT;
        gprintk("  NAK received from SCHAN.\n");
    }

    if (schanCtrl & SC_CHx_MSG_SER_CHECK_FAIL) {
        gprintk("SER Parity Check Error.\n");
        rv = -EFAULT;
    }    


    if (schanCtrl & SC_CHx_MSG_TIMEOUT_TST) {
	gprintk("Hardware Timeout Error.\n");
        rv = -EFAULT;
    }

    if (schanCtrl & SC_CHx_MSG_SCHAN_ERR) {
        bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_CTRL(ch), SC_CHx_MSG_CLR);
	rv = -EFAULT;
	gprintk("CMIC_SCHAN_ERR.\n");
    }

    return rv;
}

static void
bcmsw_soc_schan_dump(struct net_device *dev, schan_msg_t *msg, int dwc)
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


#if 0 
static int
bcmsw_schan_op(struct net_device *dev, schan_msg_t *msg, int dwc_write, int dwc_read, uint32 flags)
{
    int i, rv, val;

    //SCHAN_LOCK(unit);
    gprintk("bcmsw_schan_op entry.\n");

    val = bkn_dev_read32(dev, CMIC_SCHAN_CTRL);
    gprintk("bcmsw_schan_op schanCtrl = 0x%x\n", val);

    do {
        rv = 0;

        /* Write raw S-Channel Data: dwc_write words */
        for (i = 0; i < dwc_write; i++) {
            bkn_dev_write32(dev, CMIC_SCHAN_MESSAGE(unit, i), msg->dwords[i]);
        }

        bkn_dev_write32(dev, CMIC_SCHAN_CTRL, SC_MSG_START_SET);

        /* Wait for completion using polling method */
        rv = bcmsw_schan_poll_wait(dev, msg);

        if (rv == -ETIME) {
            break;
        }

        //memset(msg, 0, dwc_read);

        /* Read in data from S-Channel buffer space, if any */
        for (i = 0; i < dwc_read; i++) {
            msg->dwords[i] = bkn_dev_read32(dev, CMIC_SCHAN_MESSAGE(unit, i));
        }

        bcmsw_soc_schan_dump(dev, msg, dwc_read);

    } while (0);

    //SCHAN_UNLOCK(unit);

    if (rv == -ETIME) {
        gprintk("SchanTimeOut:soc_schan_op operation timed out\n");
        bcmsw_soc_schan_dump(dev, msg, dwc_write);
    }

    return rv;
}
#endif


//CMICX SCHAN OP
static int
bcmsw_schan_op(struct net_device *dev, schan_msg_t *msg, int dwc_write, int dwc_read, uint32 flags)
{
    int i, rv, val, ch;

    //SCHAN_LOCK(unit);
    gprintk("bcmsw_schan_op entry.\n");

    //TODO - get free channel
    ch = 1;

    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_CTRL(ch));
    gprintk("bcmsw_schan_op schanCtrl = 0x%x\n", val);

    do {
        rv = 0;

        /* Write raw S-Channel Data: dwc_write words */
        for (i = 0; i < dwc_write; i++) {
            bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(ch, i), msg->dwords[i]);
        }

        /* Tell CMIC to start */
        bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_CTRL(ch), SC_CHx_MSG_START);

        /* Wait for completion using polling method */
        rv = bcmsw_schan_poll_wait(dev, msg, ch);

        if (rv) {
            break;
        }

        //memset(msg, 0, dwc_read);

        /* Read in data from S-Channel buffer space, if any */
        for (i = 0; i < dwc_read; i++) {
            msg->dwords[i] = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(ch, i));
        }

        bcmsw_soc_schan_dump(dev, msg, dwc_read);

    } while (0);

    //SCHAN_UNLOCK(unit);

    if (rv) {
        gprintk("soc_schan_op operation failed\n");
        bcmsw_soc_schan_dump(dev, msg, dwc_write);
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
bcmsw_soc_mem_read(struct net_device *dev, int address, int size, void *entry_data)
{
    schan_msg_t schan_msg;
    int opcode, err;
    int rv = 0;
    uint32 allow_intr = 0;
    int src_blk, dst_blk = 0, data_byte_len; //acc_type

    memset(&schan_msg, 0, sizeof(schan_msg_t));

    /* Setup S-Channel command packet */
#define SOC_MEM_FLAG_ACC_TYPE_MASK      0x1f
#define SOC_MEM_FLAG_ACC_TYPE_SHIFT     22    
    src_blk = 6; // cmic_block is 6
    //TODO flags are different for each memory
    //acc_type = (flags>>SOC_MEM_FLAG_ACC_TYPE_SHIFT)&SOC_MEM_FLAG_ACC_TYPE_MASK;
    data_byte_len = 4;

    //maddr = soc_mem_addr_get(unit, mem, array_index, copyno, remapped_index,
    //                         &at);
    // soc_memories_bcm56370_a0
    //maddr = mip->base + (index * mip->gran);                
    schan_msg.readcmd.address = address;

    //_soc_mem_read_td_tt_byte_len_update(unit, mem, entry_dw, &data_byte_len);
    //soc_mem_dst_blk_update(unit, copyno, maddr, &dst_blk);

    //setup command header
    //schan_msg.header.v2.opcode = READ_MEMORY_CMD_MSG;
    //schan_msg.header.v2.dst_blk = dst_blk;
    //schan_msg.header.v2.src_blk = src_blk;
    //schan_msg.header.v2.data_byte_len = data_byte_len;
    //schan_msg.header.v2.bank_ignore_mask = 0;
    schan_msg.header.v4.opcode = READ_MEMORY_CMD_MSG;
    schan_msg.header.v4.dst_blk = dst_blk;
    schan_msg.header.v4.acc_type = 0;
    schan_msg.header.v4.data_byte_len = data_byte_len;
    schan_msg.header.v4.dma = 0;
    schan_msg.header.v4.bank_ignore_mask = 0;

    rv = bcmsw_schan_op(dev, &schan_msg, 2, 1 + size, allow_intr);
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
    opcode = schan_msg.header.v2.opcode;
    err = schan_msg.header.v2.err;
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
        si->port_init_speed[n3248te_ports[port].port] = n3248te_ports[index].bandwidth * 1000;
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
    }
    si->cpu_hg_index = 72;
    //TODO flex port init

}




#define BCMSW_VLAN_DEFAULT    1

//bcm_esw_port_init bcm_td3_port_cfg_init
static int bcmsw_port_cfg_init(struct bcmsw_switch *bcmsw_sw, int port, int vid)
{
//TODO
#if 0
    soc_info_t *si = bcmsw_sw->si;
    uint32 port_type;
    uint32 egr_port_type = 0;
    int cpu_hg_index = -1;    

    //contents for register write
    uint32 egr_port_mem = EGR_LPORT_PROFILEm;
    uint32 field;
    uint32 egr_port_field_ids[SOC_MAX_MEM_FIELD_WORDS];
    uint32 egt_port_field_values[SOC_MAX_MEM_FIELD_WORDS];
    int egr_port_field_count = 0;
    uint32 port_field_ids[SOC_MAX_MEM_FIELD_WORDS];
    uint32 port_field_values[SOC_MAX_MEM_FIELD_WORDS];
    int port_field_count = 0;    

    if (si->port_type[port] == BCMSW_PORT_TYPE_CMIC)) {
        cpu_hg_index = si->cpu_hg_index;
        port_type = 0; /* Same as Ethernet port */
        egr_port_type = 1; /* CPU port needs to be HG port in EGR_PORT table */
    } else if (si->port_type[port] == BCMSW_PORT_TYPE_LBPORT) {
        port_type = 2;
        egr_port_type = 2;
    } else {
        port_type = 0;
    }

    /* EGR_LPORT_TABLE config init */

    bcmsw_mem_set_field_value_array(egr_port_field_ids, PORT_TYPEf,
                                    egt_port_field_values, egr_port_type,
                                    egr_port_field_count);

    bcmsw_mem_set_field_value_array(egr_port_field_ids, EN_EFILTERf,
                                  egt_port_field_values, 1,
                                  egr_port_field_count);                                    

    max_gid = 8; // 8 for BCM_56370_A0

    max_gid = (1 << max_gid) - 1;
    gid = (port < max_gid) ? port : max_gid;

    bcmsw_mem_set_field_value_array(egr_port_field_ids, VT_PORT_GROUP_IDf,
                                      egt_port_field_values, gid,
                                      egr_port_field_count);


    if (egr_port_tab_field_count) {
        rv = soc_mem_fields32_modify(unit, EGR_PORTm, port,
                                     egr_port_field_count,
                                     egr_port_field_ids, egt_port_field_values);
    }

#endif
}

//bcm_esw_port_init bcm_td3_port_cfg_init
static int bcmsw_port_init(struct bcmsw_switch *bcmsw_sw)
{
    int num_port = HX5_NUM_PORT, port, vid; 
    soc_info_t *si = bcmsw_sw->si;

    vid = BCMSW_VLAN_DEFAULT;
    for (port = 0; port < num_port; port++) {
       if(si->port_type[port] != -1) {
           bcmsw_port_cfg_init(bcmsw_sw, port, vid);
       }
    }

    // clear egress port blocking table

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
static int bcmsw_ports_create(struct bcmsw_switch *bcmsw_sw)
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
    err = bcmsw_port_init(bcmsw_sw);
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
//soc_do_init(int unit, int reset)
int bcmsw_switch_do_init(struct bcmsw_switch *bcmsw_sw)
{
    struct net_device *dev = bcmsw_sw->dev;
    int val;
    /***********************************************************************/
    /* Always be sure device has correct endian configuration before       */
    /* touching registers - device may not have been configured yet.       */
    /***********************************************************************/
    //soc_endian_config(unit);
    // val = ES_BIG_ENDIAN_PIO | ES_BIG_ENDIAN_DMA_PACKET | ES_BIG_ENDIAN_DMA_OTHER;
    val = 0;
    bkn_dev_write32(dev, CMIC_ENDIAN_SELECT, val); 

    //enable PCIe bursting soc_pci_burst_enable(unit);
    udelay(1000);
    val = bkn_dev_read32(dev, CMIC_CONFIG);
    val |= (CC_RD_BRST_EN | CC_WR_BRST_EN);
    bkn_dev_write32(dev, CMIC_CONFIG, val);
    udelay(1000);

    /************* soc_phyctrl_software_init   *****************************/

    /******************************* soc_reset *****************************/
    val = bkn_dev_read32(dev, CMIC_CONFIG);
    bkn_dev_write32(dev, CMIC_CONFIG, val | CC_RESET_CPS);
    udelay(1000);

    gprintk("do_init CMIC_CONFIG = 0x%x", val);

    //sleep extra time to allow switch chip to finish
    //usleep(10000)
    //
    /* Restore endian mode since the reset cleared it. */
    //...


    //soc_cmic_intr_all_disable
    //soc_cmic_intr_all_disable();

    //soc_esw_schan_fifo_init


    /* Initialize bulk mem API */
    //soc_mem_bulk_init
    //


    /************* soc_helix5_chip_reset       *****************************/
    //soc_helix5_sbus_ring_map_config
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_0_7_OFFSET,0x52222100);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_8_15_OFFSET,0x30053005);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_16_23_OFFSET,0x43333333);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_24_31_OFFSET,0x64444444);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_32_39_OFFSET,0x76666666);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_40_47_OFFSET,0x07777777);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_48_55_OFFSET,0x00000000);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_56_63_OFFSET,0x00005000);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_TIMEOUT_OFFSET,0x5000);

    /* Reset IP, EP, MMU and port macros */

    /* Bring PLLs out of reset */
    //...


    /* Configure CMIC PCI registers correctly for driver operation.        */
#if 0
  val = bkn_dev_read32(dev, CMIC_CONFIG);

  /*
   * Enable enhanced DMA modes:
   *  Scatter/gather, reload, and unaligned transfers
   *
   * Enable read and write bursts.
   *  Note: very fast CPUs (above ~500 MHz) may combine multiple
   *  memory operations into bursts.  The CMIC will hang if burst
   *  operations are not enabled.
   */
  
  reg |= (CC_SG_OPN_EN | CC_RLD_OPN_EN | CC_ALN_OPN_EN |
          CC_RD_BRST_EN | CC_WR_BRST_EN);
  
  if (SAL_BOOT_PLISIM) {
      /* Set interrupt polarity to active high */
      reg &= ~CC_ACT_LOW_INT;
  }

  soc_pci_write(unit, CMIC_CONFIG, reg);

 //configure DMA channels
  //soc_dma_attach

#endif
  
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

    //switch initialization
    bcmsw_switch_do_init(bcmsw_sw);

    //create ports
    err = bcmsw_ports_create(bcmsw_sw);
    if (err) {
    	//dev_err(mlxsw_sp->bus_info->dev, "Failed to create ports\n");
    	goto err_ports_create;
    }    

    //test schan
    err = bcmsw_soc_mem_read(bcmsw_sw->dev, 0x501c0000, 14, &lport_entry); 

err_ports_create:
err_swdev_register:
    return err;    
}

