#ifndef _BCM_SWITCHDEV_H_
#define _BCM_SWITCHDEV_H_
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include <linux/net_namespace.h>
#include <linux/auxiliary_bus.h>
#include <net/devlink.h>
#include <net/switchdev.h>
#include <net/vxlan.h>


struct bcmsw_switchdev_event_work {
	struct work_struct work;
	netdevice_tracker dev_tracker;
	union {
		struct switchdev_notifier_fdb_info fdb_info;
		struct switchdev_notifier_vxlan_fdb_info vxlan_fdb_info;
	};
	struct net_device *dev;
	unsigned long event;
};

//#ifdef  BCM_56370_A0
#define SOC_MAX_NUM_PIPES               2
#define SOC_MAX_NUM_BLKS                35
#define SOC_MAX_NUM_PORTS               79
#define SOC_MAX_MEM_BYTES               78
#define SOC_MAX_NUM_MMU_PORTS           SOC_MAX_NUM_PORTS

#define HX5_NUM_PHY_PORT                (79)
#define HX5_NUM_PORT                    (72)
#define HX5_NUM_MMU_PORT                (72)

typedef enum{
   BCMSW_PORT_TYPE_NONE = 0,
   BCMSW_PORT_TYPE_CMIC,
   BCMSW_PORT_TYPE_GXPORT,
   BCMSW_PORT_TYPE_XLPORT,
   BCMSW_PORT_TYPE_CLPORT,
   BCMSW_PORT_TYPE_LBPORT,
   BCMSW_PORT_TYPE_BROADSCAN
}bcmsw_port_type_e;

//#endif  /* BCM_56370_A0 */

typedef struct {
    int             bandwidth;                                /* max core bandwidth */
    int             port_p2l_mapping[SOC_MAX_NUM_PORTS];      /* phy to logic */
    int             flex_port_p2l_mapping[SOC_MAX_NUM_PORTS]; /* flex(current) phy to logic */
    int             port_l2p_mapping[SOC_MAX_NUM_PORTS];      /* logic to phy */
    int             port_l2pp_mapping[SOC_MAX_NUM_PORTS];     /* logic to primary port in external PHY */
    int             port_l2po_mapping[SOC_MAX_NUM_PORTS];     /* logic to port offset in external PHY */
    int             port_l2pa_mapping[SOC_MAX_NUM_PORTS];     /* logic to phy address in external PHY */
    int             port_l2i_mapping[SOC_MAX_NUM_PORTS];      /* logic to idb */
    int             port_p2m_mapping[SOC_MAX_NUM_PORTS];      /* phy to mmu */
    int             max_port_p2m_mapping[SOC_MAX_NUM_PORTS];  /* max(flex) phy to mmu */
    int             port_m2p_mapping[SOC_MAX_NUM_MMU_PORTS];  /* mmu to phy */
    int             port_num_lanes[SOC_MAX_NUM_PORTS];        /* number of lanes */  
    int             port_speed_max[SOC_MAX_NUM_PORTS];        /* max port speed */
    int             port_init_speed[SOC_MAX_NUM_PORTS];   /* ports initial speed */
    int             port_serdes[SOC_MAX_NUM_PORTS];           /* serdes number */
    int             port_num_subport[SOC_MAX_NUM_PORTS];      /* number of subport */    
    int             port_pipe[SOC_MAX_NUM_PORTS];             /* pipe number */
    int             port_group[SOC_MAX_NUM_PORTS];        /* group number */

    int             cpu_hg_index;           /* table index for cpu port
                                             * higig packet where table indexed
                                             * by physical port*/
    int             port_type[SOC_MAX_NUM_PORTS];                                            
} soc_info_t;

#define COUNTOF(ary)        ((int) (sizeof (ary) / sizeof ((ary)[0])))

struct bcmsw_switchdev {
	struct bcmsw_switch *sw;
	struct list_head bridge_list;
	bool bridge_8021q_exists;
	struct notifier_block swdev_nb_blk;
	struct notifier_block swdev_nb;
};

struct bcmsw_port {
	uint16_t id;
    char     name[KCOM_NETIF_NAME_MAX];
    uint8_t  macaddr[6];
	struct   bcmsw_switch *bcmsw;
}

struct bcmsw_switch {
	struct net_device *dev; //bcm0
	soc_info_t *si;
	struct bcmsw_switchdev *swdev;
    struct bcmsw_port **ports;
};


/* Add an entry to field-value array for multiple fields write */
#define bcmsw_mem_set_field_value_array(_fa, _f, _va, _v, _p) \
    do {                \
        _fa[_p] = _f;   \
        _va[_p] = _v;   \
        _p++;           \
        } while (0);    \



int bcmsw_switch_init(void);

#endif
