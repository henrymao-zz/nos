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
#include "bcm-switchdev-switch.h"
#include "bcm-switchdev-schan.h"
#include "bcm-switchdev-extphy.h"
#include "bcm-switchdev-cancun.h"
#include "bcm-switchdev-merlin16.h"
#include "bcm-switchdev-stats.h"
#include "bcm-switchdev.h"

ibde_t *_kernel_bde = NULL;
bcmsw_switch_t *_bcmsw = NULL; 

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

int bcmsw_switch_init(void)
{
    int err = 0;

    bcmsw_switch_t *bcmsw;
        
    printk("Initializing switchdev...\n");

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
    if ((linux_bde_create(NULL, &_kernel_bde) < 0) || _kernel_bde == NULL) {
        return -ENODEV;
    }

    bcmsw->kernel_bde = _kernel_bde;

    bcm_switch_hw_init(bcmsw);

    //procfs init
    _procfs_init(bcmsw);

err_swdev_register:
    return err;    
}

int bcmsw_switch_uninit(void)
{
    bcmsw_switch_t *bcmsw = _bcmsw;

    _procfs_uninit(bcmsw);
    return 0;
}
