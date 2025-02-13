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

#include "bcm-switchdev.h"

static struct workqueue_struct *bcmsw_switchdev_wq;

static void bcmsw_fdb_event_work(struct work_struct *work)
{
	struct switchdev_notifier_fdb_info *fdb_info;
	struct bcmsw_switchdev_event_work *switchdev_work;
	//struct prestera_port *port;
	struct net_device *dev;
	int err;

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
	int err;

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

err_fdb_init:
err_swdev_init:
	destroy_workqueue(bcmsw_switchdev_wq);
err_alloc_wq:
	kfree(swdev);

	return err;
}
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
} n3248te_ports[] = {
    { 1,   1,  1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 2,   2,  1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 3,   3,  1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 4,    4,  1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 5,   5,  1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 6,   6,  1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 7,   7,  1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 8,   8,  1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 9,   0,  1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 10,  10, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 11,  11, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 12,  12, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 13,  13, 1,   BCMSW_PORT_TYPE_GXPORT },         
    { 14,  14, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 15,  15, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 16,  16, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 17,  17, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 18,  18, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 19,  19, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 20,  20, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 21,  21, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 22,  22, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 23,  23, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 24,  24, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 25,  25, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 26,  26, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 27,  27, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 28,  28, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 29,  29, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 30,  30, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 31,  31, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 32,  32, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 33,  33, 1,   BCMSW_PORT_TYPE_GXPORT },        
    { 34,  34, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 35,  35, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 36,  36, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 37,  37, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 38,  38, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 39,  39, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 40,  40, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 41,  41, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 42,  42, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 43,  43, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 44,  44, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 45,  45, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 46,  46, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 47,  47, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 48,  48, 1,   BCMSW_PORT_TYPE_GXPORT }, 
    { 49,  64, 10,  BCMSW_PORT_TYPE_XLPORT }, 
    { 50,  63, 10,  BCMSW_PORT_TYPE_XLPORT }, 
    { 51,  62, 10,  BCMSW_PORT_TYPE_XLPORT }, 
    { 52,  61, 10,  BCMSW_PORT_TYPE_XLPORT }, 
    { 53,  69, 100, BCMSW_PORT_TYPE_XLPORT }, 
    { 57,  73, 100, BCMSW_PORT_TYPE_XLPORT },   
    { -1,  -1, -1  },                                      
};


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

static int bcmsw_ports_create(struct bcmsw_switch *bcmsw_sw)
{
    //FIX: hardcode for N3248TE  BCM56371
    // soc_helix5_port_config_init
    // bcm_td3_port_cfg_init
    // bcm_esw_port_probe
    // bcm_esw_port_enable_set
    // _bcm_td_port_lanes_set
    //bcm_esw_knet_netif_create

	int err;
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

/*
	alloc_size = sizeof(struct mlxsw_sp_port *) * max_ports;
	mlxsw_sp->ports = kzalloc(alloc_size, GFP_KERNEL);
	if (!mlxsw_sp->ports)
		return -ENOMEM;

	for (i = 1; i < max_ports; i++) {
		port_mapping = &mlxsw_sp->port_mapping[i];
		if (!port_mapping->width)
			continue;
		err = bcmsw_port_create(mlxsw_sp, i, false, port_mapping);
		if (err)
			goto err_port_create;
	}

*/    
	return 0;

err_event_enable:
err_port_cfg_init:
	return err;
}

int bcmsw_switch_init(void)
{
    struct bcmsw_switch *bcmsw_sw;
    int err;

    bcmsw_sw = kzalloc(sizeof(*bcmsw_sw), GFP_KERNEL);
	if (!bcmsw_sw)
		return -ENOMEM;

	err = bcmsw_switchdev_init(bcmsw_sw);
	if (err)
		goto err_swdev_register;
  
    //get bcm0 netdev


    //create ports
	err = bcmsw_ports_create(bcmsw_sw);
	if (err) {
		//dev_err(mlxsw_sp->bus_info->dev, "Failed to create ports\n");
		goto err_ports_create;
	}    

err_ports_create:
err_swdev_register:
    return err;    
}

