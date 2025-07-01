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
#include <net/netlink.h>
#include <net/genetlink.h>
#include <net/switchdev.h>
#include <net/vxlan.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <kcom.h>
#include <bcm-knet.h>
#include "bcm-switchdev.h"


static struct workqueue_struct *swdev_wq = NULL;
static struct bcm_switchdev *swdev = NULL;


/*****************************************************************************************/
/*                             netlink                                                   */
/*****************************************************************************************/
static unsigned int switchdev_u_pid = 0;
static struct net *switchdev_net = NULL;
struct switchdev_server_config server_conf;

static int handle_switchdev_keepalive(struct sk_buff *skb, struct genl_info *info);
static int handle_switchdev_start(struct sk_buff *skb, struct genl_info *info);

/* SWITCHDEV_NETDEV_EVENT - do */
static const struct nla_policy switchdev_netdev_event_nl_policy[SWITCHDEV_A_NETDEV_EVENT_MAX + 1] = {
    [SWITCHDEV_A_NETDEV_EVENT_ID] = { .type = NLA_U32, },
	[SWITCHDEV_A_NETDEV_IF_NAME] = { .type = NLA_NUL_STRING, },
};

static const struct nla_policy switchdev_nl_policy[SWITCHDEV_EVENT_MAX + 1] = {
	[SWITCHDEV_EVENT_KEEPALIVE] = {
        .type = NLA_NUL_STRING
	},
	[SWITCHDEV_A_START] = {
        .len = 0,
	},    
};


/* operation definition */
static struct genl_ops switchdev_genl_ops[] = {
    {
        .cmd = SWITCHDEV_EVENT_KEEPALIVE,
        .doit = handle_switchdev_keepalive,
        .policy = switchdev_nl_policy,
    },
    {
        .cmd = SWITCHDEV_EVENT_START,
        .doit = handle_switchdev_start,
        .policy = switchdev_nl_policy,
    },    
};



static struct genl_multicast_group switchdev_genl_mcgrp[] = {
    {
        .name = "SWITCHDEV_GRP",
    }
};



/* netlink family definition */
static struct genl_family switchdev_genl_family = {
    .hdrsize  = 0,             
    .name     = "SWITCHDEV",      
    .version  = 1,
    .maxattr  = SWITCHDEV_EVENT_MAX,
    .module   = THIS_MODULE,
    .netnsok  = true,
    .ops      = switchdev_genl_ops,
    .n_ops    = ARRAY_SIZE(switchdev_genl_ops),
    .mcgrps   = switchdev_genl_mcgrp,
    .n_mcgrps = ARRAY_SIZE(switchdev_genl_mcgrp),
};

static void switchdev_ipc_update_last_active(void)
{
	if (server_conf.ipc_timeout) {
		server_conf.ipc_last_active = jiffies;
    }
}


static struct switchdev_ipc_msg *switchdev_ipc_msg_alloc(size_t sz)
{
	struct switchdev_ipc_msg *msg;
	size_t msg_sz = sz + sizeof(struct switchdev_ipc_msg);

	msg = kvzalloc(msg_sz, GFP_KERNEL);
	if (msg)
		msg->sz = sz;
	return msg;
}

static void switchdev_ipc_msg_free(struct switchdev_ipc_msg *msg)
{
	kvfree(msg);
}


static int switchdev_ipc_msg_send(struct switchdev_ipc_msg *msg)
{
	struct genlmsghdr *nlh;
	struct sk_buff *skb;
	int ret = -EINVAL;

	if (!switchdev_u_pid) {
        printk("switchdev_ipc_msg_send no userspace daemon connected\n");
		return ret;
    }

	skb = genlmsg_new(msg->sz, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	nlh = genlmsg_put(skb, switchdev_u_pid, 0, &switchdev_genl_family, 0, msg->type);
	if (!nlh)
		goto out;

	ret = nla_put(skb, msg->type, msg->sz, msg->payload);
	if (ret) {
		genlmsg_cancel(skb, nlh);
		goto out;
	}

	genlmsg_end(skb, nlh);
	ret = genlmsg_unicast(switchdev_net, skb, switchdev_u_pid);
	if (!ret) {
        printk("switchdev_ipc_msg_send success\n");
		switchdev_ipc_update_last_active();
    } else {
        printk("switchdev_ipc_msg_send failed %d\n", ret);
    }
	return ret;

out:
	nlmsg_free(skb);
	return ret;
}

extern int bcm_knet_get_port(struct net_device *dev);

static int switchdev_netdev_event_send(uint32_t event, struct net_device *dev)
{
	struct genlmsghdr *nlh;
	struct sk_buff *skb;    
	int    ret = -EINVAL;
    int    port;

	if (!switchdev_u_pid) {
        printk("switchdev_netdev_event_send no userspace daemon connected\n");
		return ret;
    }

    port = bcm_knet_get_port(dev);

    if (port < 0 ) {
       printk("switchdev_netdev_event_send invalid port %s %d", dev->name, port);
       return ret;
    } 

	skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	nlh = genlmsg_put(skb, switchdev_u_pid, 0, &switchdev_genl_family, 0, SWITCHDEV_EVENT_NETDEV);
	if (!nlh)
		goto err_cancel_msg;

	ret = nla_put_u32(skb, SWITCHDEV_A_NETDEV_EVENT_ID, event);
	if (ret) {
		goto out;
	}

	ret = nla_put_u32(skb, SWITCHDEV_A_NETDEV_PORT, port);
	if (ret) {
		goto out;
	}

    ret = nla_put_string(skb, SWITCHDEV_A_NETDEV_IF_NAME, dev->name);
	if (ret) {
		goto out;
	}

	genlmsg_end(skb, nlh);
	ret = genlmsg_unicast(switchdev_net, skb, switchdev_u_pid);
	if (!ret) {
        printk("switchdev_netdev_event_send success\n");
		//switchdev_ipc_update_last_active();
    } else {
        printk("switchdev_netdev_event_send failed %d\n", ret);
    }
	return ret;

err_cancel_msg:
    genlmsg_cancel(skb, nlh);
out:
	nlmsg_free(skb);
	return ret;
}

static int handle_switchdev_keepalive(struct sk_buff *skb, struct genl_info *info)
{
    /* message handling code goes here; return 0 on success, negative values on failure */
    //char *str;
    int   ret;
    struct sk_buff *msg;
    void           *hdr;

    /* Check if the attribute is present and print it */
    //if (info->attrs[SWITCHDEV_EVENT_KEEPALIVE]) {
    //	char *str = nla_data(info->attrs[SWITCHDEV_EVENT_KEEPALIVE]);
    //	printk("switchdev_keepalive message received: %s\n", str);
    //} else {
    //	printk("switchdev_keepalive empty message received\n");
    //}

    /* Allocate a new buffer for the reply */
	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg) {
		printk("failed to allocate message buffer\n");
		return -ENOMEM;
	}

	/* Put the Generic Netlink header */
	hdr = genlmsg_put(msg, info->snd_portid, info->snd_seq, &switchdev_genl_family, 0,
                      SWITCHDEV_EVENT_KEEPALIVE);
	if (!hdr) {
		printk("failed to create genetlink header\n");
		nlmsg_free(msg);
		return -EMSGSIZE;
	}
	/* And the message */
	if ((ret = nla_put_string(msg, SWITCHDEV_EVENT_KEEPALIVE,
				  "Hello from Kernel Space, Netlink!"))) {
        printk("failed to create message string\n");
		genlmsg_cancel(msg, hdr);
		nlmsg_free(msg);
		goto out;
	}

	/* Finalize the message and send it */
	genlmsg_end(msg, hdr);

	ret = genlmsg_reply(msg, info);
	//printk("reply sent\n");

out:
	return ret;
}



static int handle_switchdev_start(struct sk_buff *skb, struct genl_info *info)
{
    /* message handling code goes here; return 0 on success, negative values on failure */
    //char *str;
    int   ret;
    struct sk_buff *msg;
    void           *hdr;

    /* Allocate a new buffer for the reply */
	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if (!msg) {
		printk("failed to allocate message buffer\n");
		return -ENOMEM;
	}

	/* Put the Generic Netlink header */
	hdr = genlmsg_put(msg, info->snd_portid, info->snd_seq, &switchdev_genl_family, 0,
                      SWITCHDEV_EVENT_START);
	if (!hdr) {
		printk("failed to create genetlink header\n");
		nlmsg_free(msg);
		return -EMSGSIZE;
	}

	/* Finalize the message and send it */
	genlmsg_end(msg, hdr);

    if (switchdev_u_pid != info->snd_portid) {
        switchdev_u_pid = info->snd_portid;
        switchdev_net = genl_info_net(info);
        printk("Connect to new user space daemon 0x%x\n", switchdev_u_pid);
    }

	ret = genlmsg_reply(msg, info);
	printk("start reply sent\n");

	return ret;
}



static int genetlink_init(void)
{
    int rc;
 
    rc = genl_register_family(&switchdev_genl_family);
    if (rc != 0) {
        printk("switchdev genetlink_init failed %d\n", rc);
        return rc;
    }
 
    printk("switchdev genetlink_init OK");
    return 0;
}
 
static void genetlink_exit(void)
{
    printk("switchdev Netlink Module unloaded.");
 
    genl_unregister_family(&switchdev_genl_family);
}



/*****************************************************************************************/
/*                             switchdev                                                 */
/*****************************************************************************************/
//forward declaration
int switchdev_port_obj_attr_set_netlink(struct net_device *dev, const void *ctx,
                  const struct switchdev_attr *attr,
                  struct netlink_ext_ack *extack);

static void bcm_fdb_event_work(struct work_struct *work)
{
    //struct switchdev_notifier_fdb_info *fdb_info;
    struct bcm_switchdev_event_work *switchdev_work;
    //struct prestera_port *port;
    struct net_device *dev;

    switchdev_work = container_of(work, struct bcm_switchdev_event_work, work);
    dev = switchdev_work->dev;

    printk("bcm_fdb_event_work event = %ld \n", switchdev_work->event);
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

        err = bcm_port_fdb_set(port, fdb_info, true);
        if (err)
            break;

        bcm_fdb_offload_notify(port, fdb_info);
        break;

    case SWITCHDEV_FDB_DEL_TO_DEVICE:
        fdb_info = &switchdev_work->fdb_info;
        bcm_port_fdb_set(port, fdb_info, false);
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

static int bcm_switchdev_event(struct notifier_block *unused,
                    unsigned long event, void *ptr)
{
    struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
    struct switchdev_notifier_fdb_info *fdb_info;
    struct switchdev_notifier_info *info = ptr;
    struct bcm_switchdev_event_work *switchdev_work;
    struct net_device *upper;
    int err;

    printk("bcm_switchdev_event event = %ld\n", event);
    if (event == SWITCHDEV_PORT_ATTR_SET) {
        err = switchdev_handle_port_attr_set(dev, ptr,
                             bkn_port_dev_check,
                             switchdev_port_obj_attr_set_netlink);
        return notifier_from_errno(err);
        return NOTIFY_DONE;
    }

    if (!bkn_port_dev_check(dev))
        return NOTIFY_DONE;


    switch (event) {
    case SWITCHDEV_FDB_ADD_TO_DEVICE:
    case SWITCHDEV_FDB_DEL_TO_DEVICE:
        fdb_info = container_of(info,
                    struct switchdev_notifier_fdb_info,
                    info);

        INIT_WORK(&switchdev_work->work, bcm_fdb_event_work);
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

    queue_work(swdev_wq, &switchdev_work->work);
    return NOTIFY_DONE;

out_bad:
    return NOTIFY_BAD;
}


static int switchdev_port_event_send(unsigned long event,
                                     struct net_device *dev, const void *ctx,
                                     const struct switchdev_obj *obj,
                                     struct netlink_ext_ack *extack)
{
	const struct switchdev_obj_port_vlan *vlan;
	const struct switchdev_obj_port_mdb *mdb;
	int err = 0;    
	struct genlmsghdr *nlh;
	struct sk_buff *skb;    
	int    ret = -EINVAL;
    int    port = 0;

	if (!switchdev_u_pid) {
        printk("switchdev_netdev_event_send no userspace daemon connected\n");
		return ret;
    }

    if (!netif_is_bridge_master(dev)) {
        port = bcm_knet_get_port(dev);

        if (port < 0 ) {
            printk("switchdev_netdev_event_send invalid port %s %d", dev->name, port);
            return ret;
        }
    } 

	skb = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!skb)
		return -ENOMEM;

	nlh = genlmsg_put(skb, switchdev_u_pid, 0, &switchdev_genl_family, 0, SWITCHDEV_EVENT_PORT);
	if (!nlh)
		goto err_cancel_msg;

	ret = nla_put_u32(skb, SWITCHDEV_A_PORT_EVENT_ID, event);
	if (ret) {
		goto out;
	}

	ret = nla_put_u32(skb, SWITCHDEV_A_PORT_IF_FLAG, dev->priv_flags);
	if (ret) {
		goto out;
	}

	ret = nla_put_u32(skb, SWITCHDEV_A_PORT_IF_PORT, port);
	if (ret) {
		goto out;
	}

    ret = nla_put_string(skb, SWITCHDEV_A_PORT_IF_NAME, dev->name);
	if (ret) {
		goto out;
	}

    ret = nla_put_u32(skb, SWITCHDEV_A_PORT_OBJ_ID, obj->id);
	if (ret) {
		goto out;
	}    

	switch (obj->id) {
	    case SWITCHDEV_OBJ_ID_PORT_VLAN:
		    vlan = SWITCHDEV_OBJ_PORT_VLAN(obj);
            printk("    dev %s vlan = %d flags 0x%x\n",dev->name, vlan->vid, vlan->flags);
            ret = nla_put_u32(skb, SWITCHDEV_A_PORT_VLAN_ID, vlan->vid);
	        if (ret) {
		        goto out;
	        }
            ret = nla_put_u32(skb, SWITCHDEV_A_PORT_VLAN_FLAGS, vlan->flags);
            break;
	    case SWITCHDEV_OBJ_ID_PORT_MDB:
		    mdb = SWITCHDEV_OBJ_PORT_MDB(obj);
            printk("   vlan = %d\n",mdb->vid);
		    break;
	    case SWITCHDEV_OBJ_ID_HOST_MDB:
		    fallthrough;
	    default:
		    err = -EOPNOTSUPP;
		    break;
	}
    
	if (ret) {
		goto out;
	}

    
	genlmsg_end(skb, nlh);
	ret = genlmsg_unicast(switchdev_net, skb, switchdev_u_pid);
	if (!ret) {
        printk("switchdev_netdev_event_send success\n");
		//switchdev_ipc_update_last_active();
    } else {
        printk("switchdev_netdev_event_send failed %d\n", ret);
    }
	return ret;

err_cancel_msg:
    genlmsg_cancel(skb, nlh);
out:
	nlmsg_free(skb);
	return ret;
}


int switchdev_port_obj_add_netlink(struct net_device *dev, const void *ctx,
                 const struct switchdev_obj *obj,
                 struct netlink_ext_ack *extack)
{
    int err; 

    err = switchdev_port_event_send(SWITCHDEV_PORT_OBJ_ADD, dev, ctx, obj, extack);

    return err;
}

int switchdev_port_obj_del_netlink(struct net_device *dev, const void *ctx,
                 const struct switchdev_obj *obj)
{
    int err; 

    err = switchdev_port_event_send(SWITCHDEV_PORT_OBJ_DEL, dev, ctx, obj, NULL);

    return err;
}

int switchdev_port_obj_attr_set_netlink(struct net_device *dev, const void *ctx,
                  const struct switchdev_attr *attr,
                  struct netlink_ext_ack *extack)
{
	int err = 0;

    printk("switchdev_port_obj_attr_set_netlink attr id = %d \n", attr->id);

	switch (attr->id) {
	case SWITCHDEV_ATTR_ID_PORT_STP_STATE:
		//err = prestera_port_attr_stp_state_set(port, attr->orig_dev,
		//				       attr->u.stp_state);
		break;
	case SWITCHDEV_ATTR_ID_PORT_PRE_BRIDGE_FLAGS:
		//if (attr->u.brport_flags.mask &
		//    ~(BR_LEARNING | BR_FLOOD | BR_MCAST_FLOOD | BR_PORT_LOCKED))
		//	err = -EINVAL;
		break;
	case SWITCHDEV_ATTR_ID_PORT_BRIDGE_FLAGS:
		//err = prestera_port_attr_br_flags_set(port, attr->orig_dev,
		//				      attr->u.brport_flags);
		break;
	case SWITCHDEV_ATTR_ID_BRIDGE_AGEING_TIME:
		//err = prestera_port_attr_br_ageing_set(port,
		//				       attr->u.ageing_time);
		break;
	case SWITCHDEV_ATTR_ID_BRIDGE_VLAN_FILTERING:
		//err = prestera_port_attr_br_vlan_set(port, attr->orig_dev,
		//				     attr->u.vlan_filtering);
		break;
	case SWITCHDEV_ATTR_ID_PORT_MROUTER:
		//err = prestera_port_attr_mrouter_set(port, attr->orig_dev,
		//				     attr->u.mrouter);
		break;
	case SWITCHDEV_ATTR_ID_BRIDGE_MC_DISABLED:
		//err = prestera_port_attr_br_mc_disabled_set(port, attr->orig_dev,
		//					    attr->u.mc_disabled);
		break;
	default:
		err = -EOPNOTSUPP;
	}    
    return err;
}


static int bcm_switchdev_blk_event(struct notifier_block *unused,
                    unsigned long event, void *ptr)
{
    struct net_device *dev = switchdev_notifier_info_to_dev(ptr);
    int err;

    printk("bcm_switchdev_blk_event event = %ld\n",event);
    if (!dev) {
        printk("   dev is NULL\n");
        return NOTIFY_DONE;
    }

    switch (event) {
    case SWITCHDEV_PORT_OBJ_ADD:
        err = switchdev_handle_port_obj_add(dev, ptr,
                            bkn_port_dev_check,
                            switchdev_port_obj_add_netlink);
        break;
    case SWITCHDEV_PORT_OBJ_DEL:
        err = switchdev_handle_port_obj_del(dev, ptr,
                            bkn_port_dev_check,
                            switchdev_port_obj_del_netlink);
        break;
    case SWITCHDEV_PORT_ATTR_SET:
        err = switchdev_handle_port_attr_set(dev, ptr,
                             bkn_port_dev_check,
                             switchdev_port_obj_attr_set_netlink);
        break;
    default:
        return NOTIFY_DONE;
    }

    return notifier_from_errno(err);
}

static int bcm_netdevice_event(struct notifier_block *unused,
    unsigned long event, void *ptr)
{
    struct net_device *dev = netdev_notifier_info_to_dev(ptr);
    int err = 0;

    if (!bkn_port_dev_check(dev))
        return 0;

    printk("bcm_netdevice_event %s event = %ld\n", dev->name, event);

    switchdev_netdev_event_send(event, dev);

    return err;
}

static int bcm_switchdev_handler_init(struct bcm_switchdev *swdev)
{
    int err;


    swdev->netdev_nb.notifier_call = bcm_netdevice_event;
    err = register_netdevice_notifier(&swdev->netdev_nb);
    if (err)
        goto err_register_netdev_notifier;

    swdev->swdev_nb.notifier_call = bcm_switchdev_event;
    err = register_switchdev_notifier(&swdev->swdev_nb);
    if (err)
        goto err_register_swdev_notifier;

    swdev->swdev_nb_blk.notifier_call = bcm_switchdev_blk_event;
    err = register_switchdev_blocking_notifier(&swdev->swdev_nb_blk);
    if (err)
        goto err_register_blk_swdev_notifier;

    printk("bcm_switchdev_handler_init success!\n");
    return 0;

err_register_blk_swdev_notifier:
    printk("bcm_switchdev_handler_init blk failed %d\n", err);
    unregister_switchdev_notifier(&swdev->swdev_nb);
err_register_swdev_notifier:
    printk("bcm_switchdev_handler_init non blk failed %d\n", err);
    unregister_switchdev_notifier(&swdev->netdev_nb);
err_register_netdev_notifier:
    printk("bcm_switchdev_handler_init netdev failed %d\n", err);
    destroy_workqueue(swdev_wq);
    return err;
}



int bcm_switchdev_init_internal(void)
{
    int err;

    printk("    switchdev init handler...\n");
    swdev = kzalloc(sizeof(*swdev), GFP_KERNEL);
    if (!swdev)
        return -ENOMEM;

    INIT_LIST_HEAD(&swdev->bridge_list);

    swdev_wq = alloc_ordered_workqueue("%s_ordered", 0, "bcm_switchdev");
    if (!swdev_wq) {
        err = -ENOMEM;
        printk("    failed to alloc workqueue\n");
        goto err_alloc_wq;
    }

    err = bcm_switchdev_handler_init(swdev);
    if (err)
        goto err_swdev_init;

    return 0;

//err_fdb_init:
err_swdev_init:
    destroy_workqueue(swdev_wq);
err_alloc_wq:
    kfree(swdev);

    return err;
}


int bcm_switchdev_init(void)
{
    int err = 0;
        
    printk("Initializing switchdev...\n");

    err = bcm_switchdev_init_internal();
    if (err) {
         goto err_swdev_init;
    }

    //initialize netlink 
    genetlink_init();

err_swdev_init:
    return err;    
}



int bcm_switchdev_uninit(void)
{
    genetlink_exit();

    unregister_netdevice_notifier(&swdev->netdev_nb);
    unregister_switchdev_notifier(&swdev->swdev_nb);
    unregister_switchdev_blocking_notifier(&swdev->swdev_nb_blk);

    destroy_workqueue(swdev_wq);
    kfree(swdev);

    return 0;
}
