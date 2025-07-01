#ifndef _BCM_SWITCHDEV_H_
#define _BCM_SWITCHDEV_H_



/*****************************************************************************************/
/*                              switchdev                                                */
/*****************************************************************************************/


struct bcm_switchdev_event_work {
	struct work_struct work;
	netdevice_tracker dev_tracker;
	union {
		struct switchdev_notifier_fdb_info fdb_info;
		struct switchdev_notifier_vxlan_fdb_info vxlan_fdb_info;
	};
	struct net_device *dev;
	unsigned long event;
};

struct bcm_switchdev {
	struct list_head bridge_list;
	bool bridge_8021q_exists;
	struct notifier_block swdev_nb_blk;
	struct notifier_block swdev_nb;
    struct notifier_block netdev_nb;
};


/* commands  
 */
enum {
    SWITCHDEV_EVENT_KEEPALIVE,
    SWITCHDEV_EVENT_START,
    SWITCHDEV_EVENT_NETDEV,
	SWITCHDEV_EVENT_PORT,

    __SWITCHDEV_EVENT_MAX,
    SWITCHDEV_EVENT_MAX = __SWITCHDEV_EVENT_MAX - 1
};

struct switchdev_ipc_msg {
	unsigned int		type;
	unsigned int		sz;
	unsigned char		payload[];
};


struct switchdev_server_config {
	unsigned int		flags;
	unsigned int		state;
	short			    signing;
	unsigned short		ipc_timeout;
	unsigned long		ipc_last_active;
};


enum {
	SWITCHDEV_A_NETDEV_EVENT_ID = 1,
    SWITCHDEV_A_NETDEV_PORT,
	SWITCHDEV_A_NETDEV_IF_NAME,
	
	__SWITCHDEV_A_NETDEV_EVENT_MAX,
	SWITCHDEV_A_NETDEV_EVENT_MAX = (__SWITCHDEV_A_NETDEV_EVENT_MAX - 1)
};

enum {
	SWITCHDEV_A_PORT_EVENT_ID = 1,
	SWITCHDEV_A_PORT_IF_FLAG,
	SWITCHDEV_A_PORT_IF_PORT,
	SWITCHDEV_A_PORT_IF_NAME,
    SWITCHDEV_A_PORT_OBJ_ID,
	SWITCHDEV_A_PORT_VLAN_ID,
	SWITCHDEV_A_PORT_VLAN_FLAGS,
	
	__SWITCHDEV_A_PORT_MAX,
	SWITCHDEV_A_PORT_MAX = (__SWITCHDEV_A_PORT_MAX - 1)
};


enum {
	SWITCHDEV_A_KEEPALIVE = 1,
	SWITCHDEV_A_START,

	__SWITCHDEV_A_MAX,
	SWITCHDEV_A_MAX = (__SWITCHDEV_A_MAX - 1)
};



#define TEST_GENL_MSG_FROM_KERNEL "Hello from switchdev Kernel!"

#endif
