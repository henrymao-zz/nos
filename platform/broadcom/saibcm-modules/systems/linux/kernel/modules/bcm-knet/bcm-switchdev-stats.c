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
#include "bcm-switchdev-cancun.h"
#include "bcm-switchdev-merlin16.h"
#include "bcm-switchdev-stats.h"
#include "bcm-switchdev.h"




/*****************************************************************************************/
/*                             /proc                                                     */
/*****************************************************************************************/
static struct proc_dir_entry *proc_switchdev_base = NULL;
static struct proc_dir_entry *proc_reg_base = NULL;
static struct proc_dir_entry *proc_mem_base = NULL;
static struct proc_dir_entry *proc_stats_base = NULL;


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

// /proc/switchdev/portstat
static const char *encap_mode[] = {
    "IEEE", "HIGIG", "B5632", "HIGIG2", "HIGIG2_LITE", "HIGIG2_L2",
    "HIGIG2_L3", "OBSOLETE", "SOP", "HGOE", "CPRI", "RSVD4", "HIGIG3", NULL
};    
static const char *loopback_mode[] = {
    "NONE", "MAC", "PHY", "RMT", "MAC_RMT", "EDB", NULL
};
/* Note:  See link.h, bcm_linkscan_mode_e */
static const char *linkscan_mode[] = {
    "None", "SW", "HW", "OFF", "ON", NULL
};
static const char *forward_mode[] = {
    "Disable", "Block", "LIsten", "LEarn", "Forward", NULL
};
static const char *discard_mode[] = {
    "None", "All", "Untag", "Tag", NULL, NULL
};

static char *
if_fmt_speed(char *b, int speed)
{
    if (speed >= 1000) {        /* Use Gb */
        if (speed % 1000) {     /* Use Decimal */
            snprintf(b, 6, "%d.%dG", speed / 1000, (speed % 1000) / 100);
        } else {
            snprintf(b, 6, "%dG", speed / 1000);
        }
    } else if (speed == 0) {
        snprintf(b, 6, "-");
    } else {                    /* Use Mb */
        snprintf(b, 6, "%dM", speed);
    }
    return (b);
}

extern  int
 bcm_esw_port_selective_get(bcmsw_switch_t *bcmsw, int port, bcm_port_info_t *info);

//if_esw_port_stat()
static int
_portstat_show(struct seq_file *m, void *v)
{
    int index;
    soc_info_t *si;
    bcm_port_info_t port_info;
    char *spt_str, *discrd_str;
    char sbuf[8];
    char lrn_str[4];
    int lrn_ptr;
    char *disp_str =
    "%15s "                 /* port number */
    "%5s  "                 /* enable/link state */
    "%3s"                   /* lanes */
    "%9s "                  /* speed/duplex */
    "%4s "                  /* link scan mode */
    "%4s "                  /* auto negotiate? */
    "%7s   "                /* spantree state */
    "%5s  "                 /* pause tx/rx */
    "%6s "                  /* discard mode */
    "%3s "                  /* learn to CPU, ARL, FWD or discard */
    "%6s "                  /* interface */
    "%5s "                  /* max frame */
    "%6s "                  /* cutthrough */
    "%5s "                  /* loopback */
    "%7s\n";                /* encap */
    char *asf[2] = {"cut ", "thru?"};

    if (!_bcmsw) {
        seq_printf(m, " Not initialized\n");
    return 0;
    }

    si = _bcmsw->si;

    if (!si) {
    seq_printf(m, " si Not initialized\n"); 
    return 0;
    }
    seq_printf(m, disp_str,
        " ",                 /* port number */
        "ena/",              /* enable/link state */
        "  ",                /* number of lanes */
        "speed/",            /* speed/duplex */
        "link",              /* link scan mode */
        "auto",              /* auto negotiate? */
        " STP ",             /* spantree state */
        "",                  /* pause tx/rx or ilkn FC status */
        " ",                 /* discard mode */
        "lrn",               /* learn to CPU, ARL, FWD or discard */
        "inter",             /* interface */
        "max",               /* max frame */
        asf[0],              /* cutthrough */
        "loop",              /* loopback */
        " ");                /* encap */
    seq_printf(m, disp_str,
        "port",              /* port number */
        "link",              /* enable/link state */
        "Lns",                  /* number of lanes */
        "duplex",            /* speed/duplex */
        "scan",              /* link scan mode */
        "neg?",              /* auto negotiate? */
        "state",             /* spantree state */
        "pause",             /* pause tx/rx */
        "discrd",            /* discard mode */
        "ops",               /* learn to CPU, ARL, FWD or discard */
        "face",              /* interface */
        "frame",             /* max frame */
        asf[1],              /* cutthrough */
        "back",              /* loopback */
        "encap");            /* encap */

    //TODO, add more ports
    for (index =1; index<=48; index++) {
        bcm_esw_port_selective_get(_bcmsw, index, &port_info);
        //brief_port_info()
        seq_printf(m, "%10s(%3d)  %4s ", si->ports[index].name, index,
            !port_info.enable ? "!ena" :
            (port_info.linkstatus == PORT_LINK_STATUS_FAILED) ? "fail" :
            (port_info.linkstatus == PORT_LINK_STATUS_UP ? "up  " : "down"));

        seq_printf(m, " %2d ", port_info.phy_master);

        seq_printf(m, "%5s ", if_fmt_speed(sbuf, port_info.speed));
        seq_printf(m, "%3s ", port_info.speed == 0 ? "" : port_info.duplex ? "FD" : "HD");

        seq_printf(m, "%4s ", LINKSCAN_MODE(port_info.linkscan));
        seq_printf(m, "%4s ", port_info.autoneg ? " Yes" : " No ");

        spt_str = FORWARD_MODE(port_info.stp_state);
        seq_printf(m, " %7s  ", spt_str); //STP

        seq_printf(m, "%2s ", port_info.pause_tx ? "TX" : "");
        seq_printf(m, "%2s ", port_info.pause_rx ? "RX" : "");

        discrd_str = DISCARD_MODE(port_info.discard);
        seq_printf(m, "%6s  ", discrd_str);

        lrn_ptr = 0;
        memset(lrn_str, 0, sizeof(lrn_str));
        lrn_str[0] = 'D';
        if (port_info.learn & BCM_PORT_LEARN_FWD) {
            lrn_str[lrn_ptr++] = 'F';
        }
        if (port_info.learn & BCM_PORT_LEARN_ARL) {
            lrn_str[lrn_ptr++] = 'A';
        }
        if (port_info.learn & BCM_PORT_LEARN_CPU) {
            lrn_str[lrn_ptr++] = 'C';
        }
        seq_printf(m, "%3s ", lrn_str);

        seq_printf(m, "%5d ", port_info.frame_max);

        seq_printf(m, "%5s",
                  port_info.loopback != BCM_PORT_LOOPBACK_NONE ?
                  LOOPBACK_MODE(port_info.loopback) : "     ");
        
        //TODO asf cut-through
        seq_printf(m, "%5s ", "     ");                  
        seq_printf(m, " %7s",
             port_info.encap_mode != SOC_ENCAP_COUNT ?
             ENCAP_MODE(port_info.encap_mode) : " ");

        seq_printf(m, "\n");    
    }

    return 0;
}

static int _portstat_open(struct inode * inode, struct file * file)
{
    return single_open(file, _portstat_show, NULL);
}

static struct proc_ops portstat_ops = 
{
    proc_open:       _portstat_open,
    proc_read:       seq_read,
    proc_lseek:     seq_lseek,
    proc_release:    single_release,
};

//_bcm_esw_l2_from_l2x -> _bcm_fb_l2_from_l2x
static int
_bcm_esw_l2_from_l2x(bcm_l2_addr_t *l2addr, uint32_t *l2x_entry)
{
    int mb_index, rval;
    uint32_t val;

    memset(l2addr, 0, sizeof (*l2addr));

    /* Valid bit is ignored here; entry is assumed valid */
    // MAC_ADDRf start 22, len 48
    _mem_field_get(l2x_entry, L2Xm_BYTES, 22, 48, l2addr->mac, SOCF_LE);


    // VLAN_IDf start 8, len 12
    _mem_field_get(l2x_entry, L2Xm_BYTES, 8, 12, &val, SOCF_LE);
    l2addr->vid = val;

    // PRIf start 97, len 4
    _mem_field_get(l2x_entry, L2Xm_BYTES, 97, 4, &val, SOCF_LE);
    l2addr->cos_dst = val;
    l2addr->cos_src = val;

    // CPUf start 104, len 1
    _mem_field_get(l2x_entry, L2Xm_BYTES, 104, 1, &val, 0);
    if (val) {
        l2addr->flags |= BCM_L2_COPY_TO_CPU;
    }

    // DST_DISCARDf 103, len 1
    _mem_field_get(l2x_entry, L2Xm_BYTES, 103, 1, &val, 0);
    if (val) {
        l2addr->flags |= BCM_L2_DISCARD_DST;
    }

    // SRC_DISCARDf start 105, len 1
    _mem_field_get(l2x_entry, L2Xm_BYTES, 105, 1, &val, 0);
    if (val) {
        l2addr->flags |= BCM_L2_DISCARD_SRC;
    }

    // SCPf start 106, len 1
    _mem_field_get(l2x_entry, L2Xm_BYTES, 106, 1, &val, 0);
    if (val) {
        l2addr->flags |= BCM_L2_COS_SRC_PRI;
    }

    if (BCM_MAC_IS_MCAST(l2addr->mac)) {
        l2addr->flags |= BCM_L2_MCAST;

        // L2MC_PTRf start 
        //l2addr.l2mc_group =
        //    soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, L2MC_PTRf);

        /* Translate l2mc index */
        //BCM_IF_ERROR_RETURN(
        //    bcm_esw_switch_control_get(unit, bcmSwitchL2McIdxRetType, &rval));
        //if (rval)  {
        //   _BCM_MULTICAST_GROUP_SET(l2addr.l2mc_group, _BCM_MULTICAST_TYPE_L2,
        //                                                        l2addr.l2mc_group);
        //}
    } else {
#if 0        
        int                     isGport, rv;
        bcm_module_t    mod_in, mod_out;
        bcm_port_t      port_in, port_out;

        //if (soc_feature(unit,soc_feature_trunk_group_overlay)) {
        //
        port_in = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry,
                                                         PORT_NUMf);
        mod_in = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, MODULE_IDf);
        BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_GET,
                                        mod_in, port_in, &mod_out, &port_out));
        l2addr.modid = mod_out;
        l2addr.port = port_out;

        dest.port = l2addr.port;
        dest.modid = l2addr.modid;
        dest.gport_type = _SHR_GPORT_TYPE_MODPORT;

        rv = bcm_esw_switch_control_get(unit, bcmSwitchUseGport, &isGport);

        if (BCM_SUCCESS(rv) && isGport) {
            BCM_IF_ERROR_RETURN(
                _bcm_esw_gport_construct(unit, &dest, &(l2addr.port)));
        }
#endif                
    }

    //if (soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, L3f)) {
    //    l2addr.flags |= BCM_L2_L3LOOKUP;
    //}

    //MAC_BLOCK_INDEXf start 90, len 5
    _mem_field_get(l2x_entry, L2Xm_BYTES, 90, 5, &val, SOCF_LE);
    l2addr->group = val;
/*    
    if (SOC_CONTROL(unit)->l2x_group_enable) {
        l2addr.group = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry,
                                                       MAC_BLOCK_INDEXf);
    } else {
        mb_index = soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry,
                                                  MAC_BLOCK_INDEXf);
        if (mb_index) {
            BCM_PBMP_ASSIGN(l2addr.block_bitmap,
                            _mbi_entries[unit][mb_index].mb_pbmp);
        }
        l2addr.group = 0;
    }
*/
    // RPEf start 96, len 1
    _mem_field_get(l2x_entry, L2Xm_BYTES, 96, 1, &val, 0);
    if (val) {
        l2addr->flags |= BCM_L2_SETPRI;
    }

    //STATIC_BITf start 102, len 1
    _mem_field_get(l2x_entry, L2Xm_BYTES, 102, 1, &val, 0);
    if (val) {
        l2addr->flags |= BCM_L2_STATIC;
    }

    //if (soc_L2_ENTRY_ONLYm_field32_get(unit, l2x_entry, MIRRORf)) {
    //    l2addr.flags |= BCM_L2_MIRROR;
    //}

    //HITSAf start 109, len 1
    _mem_field_get(l2x_entry, L2Xm_BYTES, 109, 1, &val, 0);
    if (val) {
        l2addr->flags |= BCM_L2_SRC_HIT;
        l2addr->flags |= BCM_L2_HIT;
    }

    //HITDAf start 109, len 1
    _mem_field_get(l2x_entry, L2Xm_BYTES, 108, 1, &val, 0);
    if (val) {
        l2addr->flags |= BCM_L2_DES_HIT;
        l2addr->flags |= BCM_L2_HIT;
    }

    return SOC_E_NONE;
}

//if_esw_l2()
static int
_l2_show(struct seq_file *m, void *v)
{
    int index;
    uint32_t entry[SOC_MAX_MEM_WORDS];
    bcm_l2_addr_t l2addr;
    uint32_t val;
    int islocal;
    
    
    //bcm_l2_traverse(unit, _l2addr_dump, NULL);-> bcm_esw_l2_traverse() -> _bcm_esw_l2_traverse()
    //->_bcm_esw_l2_traverse_mem(unit, L2Xm, trav_st);
    //TODO, how to traverse the entire table
    for (index = 0; index < 0x7fff; index++) {
        //seq_printf(m, "base 0x%x\n", p_data->reg_addr);
        //BASE_VALIDf start 0, len 3
        if(index%1024 == 1023) {
           msleep(1);
        }

        _soc_mem_read(_bcmsw->dev, L2Xm+index, 
            SCHAN_BLK_IPIPE, BYTES2WORDS(L2Xm_BYTES), 
            entry);      

        _mem_field_get(entry, L2Xm_BYTES, 0, 3, &val, SOCF_LE);

        if (!val) {
            continue;
        }

        // _bcm_esw_l2_from_l2x()
        _bcm_esw_l2_from_l2x(&l2addr, entry);

        //_l2addr_dump()
        seq_printf(m, "[%d] mac=%02x:%02x:%02x:%02x:%02x:%02x vlan=%d GPORT=0x%x",
                   index,
                   l2addr.mac[0], l2addr.mac[1], l2addr.mac[2],
                   l2addr.mac[3], l2addr.mac[4], l2addr.mac[5],
                   l2addr.vid, l2addr.port);

        islocal = TRUE; //TODO

        seq_printf(m, " modid=%d port=%d%s%s", l2addr.modid, l2addr.port,
                    (islocal == TRUE) ? "/" : " ",
                    (islocal == TRUE) ?" ":" ");
                    //TODO mod_port_name(unit, l2addr.modid, l2addr->port) : " ");

        if (l2addr.flags & BCM_L2_STATIC) {
            seq_printf(m, " Static");
        }
                
        if (l2addr.flags & BCM_L2_HIT) {
            seq_printf(m, " Hit");
        }
                
        if (l2addr.cos_src != 0 || l2addr.cos_dst != 0) {
            seq_printf(m, " COS(src=%d,dst=%d)", l2addr.cos_src, l2addr.cos_dst);
        }
                
        if (l2addr.flags & BCM_L2_COS_SRC_PRI) {
            seq_printf(m, " SCP");
        }
                
        if (l2addr.flags & BCM_L2_COPY_TO_CPU) {
            seq_printf(m, " CPU");
        } else if((l2addr.port == 0) && islocal) {
            if(!(l2addr.flags & (BCM_L2_L3LOOKUP | BCM_L2_TRUNK_MEMBER))) {
                seq_printf(m, " CPU");
            }
        }
                
        if (l2addr.flags & BCM_L2_MIRROR) {
            seq_printf(m, " Mirror");
        }
                
        if (l2addr.flags & BCM_L2_L3LOOKUP) {
            seq_printf(m, " L3");
        }
                
        if (l2addr.flags & BCM_L2_DISCARD_SRC) {
            seq_printf(m, " DiscardSrc");
        }
                
        if (l2addr.flags & BCM_L2_DISCARD_DST) {
            seq_printf(m, " DiscardDest");
        }
                
        if (l2addr.flags & BCM_L2_PENDING) {
            seq_printf(m, " Pending");
        }
                
        if (l2addr.flags & BCM_L2_SETPRI) {
            seq_printf(m, " ReplacePriority");
        }
                
        if (l2addr.flags & BCM_L2_MCAST) {
            seq_printf(m, " MCast=%d", l2addr.l2mc_group);
        }
                
        //if (SOC_PBMP_NOT_NULL(l2addr.block_bitmap)) {
        //format_pbmp(unit, bmstr, sizeof (bmstr), l2addr.block_bitmap);
        //cli_out(" MAC blocked port bitmap=%s: %s",
        //    SOC_PBMP_FMT(l2addr.block_bitmap, pfmt), bmstr);
        //}
                
        if (l2addr.group) {
            seq_printf(m, " Group=%d", l2addr.group);
        }
   
        seq_printf(m, "\n");

    }
    return 0;
}

static int _l2_open(struct inode * inode, struct file * file)
{
    return single_open(file, _l2_show, NULL);
}
static ssize_t
_l2_write(struct file *file, const char *ubuf, size_t count, loff_t *ppos) 
{
    //_proc_reg_data_t *p_data = (_proc_reg_data_t *)pde_data(file_inode(file));
    printk("_l2_write handler\n");
    return -1;
}


static struct proc_ops l2_ops = 
{
    proc_open:       _l2_open,
    proc_read:       seq_read,
    proc_lseek:     seq_lseek,
    proc_write:      _l2_write,
    proc_release:    single_release,
};

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

static const uint32_t gxblk[6] = {
    SCHAN_BLK_GXPORT0,
    SCHAN_BLK_GXPORT1,
    SCHAN_BLK_GXPORT2,
    SCHAN_BLK_GXPORT3,
    SCHAN_BLK_GXPORT4,
    SCHAN_BLK_GXPORT5,
};

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
// /proc/switchdev/reg/*
static int
_proc_reg32_show(struct seq_file *m, void *v)
{
    int index, i;
    uint32_t val;
    _proc_reg_data_t *p_data = (_proc_reg_data_t *)pde_data(file_inode(m->file));

    if (!_bcmsw) {
        seq_printf(m," Not initialized\n");
        return 0;
    }

    seq_printf(m, "base 0x%x\n", p_data->reg_addr);

    switch (p_data->reg_addr) {
        case IDB_OBM0_Q_CONTROLr:
            for (index =0; index < 10; index ++) { 
                _reg32_read(_bcmsw->dev,SCHAN_BLK_IPIPE, obm_ctrl_regs[index], &val);
                seq_printf(m, "%2d [0x%08x]  0x%08x\n", index, obm_ctrl_regs[index], val);
            }
            break;
 
        case IDB_OBM0_Q_CA_CONTROLr:
            for (index =0; index < 10; index ++) { 
                _reg32_read(_bcmsw->dev,SCHAN_BLK_IPIPE, soc_helix5_obm_ca_ctrl_regs[index], &val);
                seq_printf(m, "%2d [0x%08x]  0x%08x\n", index, soc_helix5_obm_ca_ctrl_regs[index], val);
            }
            break;

        case MMU_GCFG_MISCCONFIGr:
           _schan_reg32_read(_bcmsw->dev,SCHAN_BLK_MMU_GLB, MMU_GCFG_MISCCONFIGr, &val, 20);
           seq_printf(m, "0x%08x\n",val);
           break;

        case EGR_DEVICE_TO_PHYSICAL_PORT_NUMBER_MAPPINGr:
            for (index =0; index < HX5_NUM_PORT; index ++) { 
                _reg32_read(_bcmsw->dev,SCHAN_BLK_EPIPE, 
                            EGR_DEVICE_TO_PHYSICAL_PORT_NUMBER_MAPPINGr+index,
                            &val);
                seq_printf(m, "[%2d]  0x%08x\n", index, val);
            }
            break;

        case MMU_PORT_TO_PHY_PORT_MAPPINGr:
            for (index =0; index < HX5_NUM_PORT; index ++) { 
                _schan_reg32_read(_bcmsw->dev,SCHAN_BLK_MMU_GLB, 
                            MMU_PORT_TO_PHY_PORT_MAPPINGr+index,
                            &val, 20);
                seq_printf(m, "[%2d]  0x%08x\n", index, val);
            }
            break;

        case MMU_PORT_TO_DEVICE_PORT_MAPPINGr:
            for (index =0; index < HX5_NUM_PORT; index ++) { 
                _schan_reg32_read(_bcmsw->dev,SCHAN_BLK_MMU_GLB, 
                            MMU_PORT_TO_DEVICE_PORT_MAPPINGr+index,
                            &val, 20);
                seq_printf(m, "[%2d]  0x%08x\n", index, val);
            }
            break;

        case IDB_CA_CPU_CONTROLr:
        case IDB_CA_LPBK_CONTROLr:
        case IDB_CA_BSK_CONTROLr:
            _reg32_read(_bcmsw->dev,SCHAN_BLK_IPIPE, 
                       p_data->reg_addr, 
                       &val);
            seq_printf(m, "0x%08x\n", val);
            break;
        
        case GPORT_MODE_REGr:
	        //case PMQ_XGXS0_CTRL_REGr:
	        for (index =0; index < p_data->num_blk; index ++) {
                val = 0;
		        _reg32_read(_bcmsw->dev, p_data->block[index], p_data->reg_addr, &val);
		        seq_printf(m, "blk %d 0x%08x\n",p_data->block[index], val);
            }
	        break;

        case GPORT_CONFIGr:
        case GPORT_RSV_MASKr:
        case GPORT_STAT_UPDATE_MASKr:
        case COMMAND_CONFIGr:
            for (index =0; index < 6; index ++) {
                for (i=0; i<8; i++) {
                    _reg32_read(_bcmsw->dev, gxblk[index], p_data->reg_addr+i, &val);
                    seq_printf(m, "[%2d] 0x%08x  0x%08x\n", gxblk[index], p_data->reg_addr+i, val);
                }
            }   
            break;

        case PMQ_ECCr:
        case PMQ_ECC_INIT_STSr:
        case PMQ_ECC_INIT_CTRLr:
            _reg32_read(_bcmsw->dev, SCHAN_BLK_PMQPORT0, p_data->reg_addr, &val);
            seq_printf(m, "0x%08x[%d]  0x%08x\n", p_data->reg_addr, SCHAN_BLK_PMQPORT0, val);
            _reg32_read(_bcmsw->dev, SCHAN_BLK_PMQPORT1, p_data->reg_addr, &val);
            seq_printf(m, "0x%08x[%d]  0x%08x\n", p_data->reg_addr, SCHAN_BLK_PMQPORT1, val);
            _reg32_read(_bcmsw->dev, SCHAN_BLK_PMQPORT2, p_data->reg_addr, &val);
            seq_printf(m, "0x%08x[%d]  0x%08x\n", p_data->reg_addr, SCHAN_BLK_PMQPORT2, val);
            break;
        default:
            seq_printf(m," Not implemented\n");
            break;
    } 
    return 0;
}

static ssize_t
_proc_reg32_write(struct file *file, const char *ubuf, size_t count, loff_t *ppos) 
{
    _proc_reg_data_t *p_data = (_proc_reg_data_t *)pde_data(file_inode(file));
    printk("_prog_reg32_write handler 0x%08x\n", p_data->reg_addr);
    return -1;
}

static int _proc_reg32_open(struct inode * inode, struct file * file)
{
    return single_open(file, _proc_reg32_show, NULL);
}


static struct proc_ops _proc_reg32_ops = 
{
    proc_open:       _proc_reg32_open,
    proc_read:       seq_read,
    proc_lseek:      seq_lseek,
    proc_write:      _proc_reg32_write,
    proc_release:    single_release,
};

// /proc/switchdev/mem/*
static int
_proc_mem_show(struct seq_file *m, void *v)
{
    int index, i;
    uint32_t entry[SOC_MAX_MEM_WORDS];
    uint32_t val;
    egr_port_entry_t   *egr_port_entry;
    _proc_reg_data_t *p_data = (_proc_reg_data_t *)pde_data(file_inode(m->file));

    if (!_bcmsw) {
        seq_printf(m," Not initialized\n");
        return 0;
    }

    seq_printf(m, "base 0x%x\n", p_data->reg_addr);

    switch (p_data->reg_addr) {
        case EGR_PORTm:
            for (index =0; index < 72; index ++) {
                _soc_mem_read(_bcmsw->dev, EGR_PORTm+index, 
                               SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_PORTm_BYTES), 
                               entry);
                egr_port_entry = (egr_port_entry_t *)entry;
                seq_printf(m, "%2d [%2d]  0x%08x 0x%08x\n", index, 
                   egr_port_entry->port_type, entry[0], entry[1]);
            }   
            break;
        case EGR_ENABLEm:
            for (index =0; index < 72; index ++) {
                _soc_mem_read(_bcmsw->dev, EGR_PORTm+index, 
                           SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_PORTm_BYTES), 
                           entry);
                seq_printf(m, "%2d  0x%08x\n", index, entry[0]);
            }   
            break;        
        case EGR_LPORT_PROFILEm:
            for (index =0; index < 12; index ++) {
                _soc_mem_read(_bcmsw->dev, EGR_LPORT_PROFILEm+index, 
                               SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_LPORT_PROFILEm_BYTES), 
                               entry);
                seq_printf(m, "[%2d]  0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", index, 
                               entry[0], entry[1], entry[2], entry[3], entry[4]);
            }   
            break;
        case EGR_GPP_ATTRIBUTESm:
            for (index =0; index < 72; index ++) {
                _soc_mem_read(_bcmsw->dev, EGR_GPP_ATTRIBUTESm+index, 
                               SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_GPP_ATTRIBUTESm_BYTES), 
                               entry);
                seq_printf(m, "[%2d]  0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", index, 
                               entry[0], entry[1], entry[2], entry[3], entry[4]);
            }   
            break;            
        case LPORT_TABm:
            for (index =0; index < 72; index ++) {
                _soc_mem_read(_bcmsw->dev, LPORT_TABm+index, 
                              SCHAN_BLK_IPIPE, BYTES2WORDS(LPORT_TABm_BYTES), 
                              entry);
                //PORT_VIDf start 3, len 12
                _mem_field_get(entry, LPORT_TABm_BYTES, 3, 12, &val, SOCF_LE);
                seq_printf(m, "%2d vid:%4d ", index, val);
                for (i = 0;i< (LPORT_TABm_BYTES/4); i++) {
                    seq_printf(m, "0x%08x ", entry[i]);
                }
               seq_printf(m, "\n");
           }   
           break; 

        case ING_DEVICE_PORTm:
            for (index =0; index < 72; index ++) {
                _soc_mem_read(_bcmsw->dev, ING_DEVICE_PORTm+index, 
                              SCHAN_BLK_IPIPE, BYTES2WORDS(ING_DEVICE_PORTm_BYTES), 
                              entry);
                seq_printf(m, "%2d   0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", 
                          index,
                          entry[0],
                          entry[1],
                          entry[2],
                          entry[3],
                          entry[4]);
            }   
            break;

        case ING_VLAN_VFI_MEMBERSHIPm:
            for (index =0; index < 16; index ++) {  //TODO , dump 16 only
                _soc_mem_read(_bcmsw->dev, ING_VLAN_VFI_MEMBERSHIPm+index, 
                          SCHAN_BLK_IPIPE, BYTES2WORDS(ING_VLAN_VFI_MEMBERSHIPm_BYTES), 
                          entry);
                seq_printf(m, "%2d   0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", 
                      index,
                      entry[0],
                      entry[1],
                      entry[2],
                      entry[3],
                      entry[4]);
           }   
            break;

        case EGR_VLAN_VFI_MEMBERSHIPm:
            for (index =0; index < 16; index ++) {  //TODO , dump 16 only
                _soc_mem_read(_bcmsw->dev, EGR_VLAN_VFI_MEMBERSHIPm+index, 
                          SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_VLAN_VFI_MEMBERSHIPm_BYTES), 
                          entry);
                seq_printf(m, "%2d   0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", 
                      index,
                      entry[0],
                      entry[1],
                      entry[2],
                      entry[3],
                      entry[4]);
            }           
            break;

        case ING_DEST_PORT_ENABLEm:
            _soc_mem_read(_bcmsw->dev, ING_DEST_PORT_ENABLEm, 
                          SCHAN_BLK_IPIPE, BYTES2WORDS(ING_DEST_PORT_ENABLEm_BYTES), 
                          entry);
            seq_printf(m, "%2d   0x%08x 0x%08x 0x%08x\n", 
                      index,
                      entry[0],
                      entry[1],
                      entry[2]);       
            break;

        case EPC_LINK_BMAPm:
            _soc_mem_read(_bcmsw->dev, EPC_LINK_BMAPm, 
                          SCHAN_BLK_IPIPE, BYTES2WORDS(EPC_LINK_BMAPm_BYTES), 
                          entry);
            seq_printf(m, "%2d   0x%08x 0x%08x 0x%08x\n", 
                      index,
                      entry[0],
                      entry[1],
                      entry[2]);    
            break;            

        case MAC_BLOCKm:
            for (index =0; index < 32; index ++) {
                _soc_mem_read(_bcmsw->dev, MAC_BLOCKm+index, 
                              SCHAN_BLK_IPIPE, BYTES2WORDS(MAC_BLOCKm_BYTES), 
                              entry);
                seq_printf(m, "%2d   0x%08x 0x%08x 0x%08x\n", 
                           index,
                           entry[0],
                           entry[1],
                           entry[2]);
            }          
            break;

        case SYS_PORTMAPm:
            for (index =0; index < HX5_NUM_PORT; index ++) {
                _soc_mem_read(_bcmsw->dev, SYS_PORTMAPm+index, 
                              SCHAN_BLK_IPIPE, BYTES2WORDS(SYS_PORTMAPm_BYTES), 
                              entry);
                seq_printf(m, "[%2d]   0x%08x \n", 
                           index,
                           entry[0]);
            }          
            break;     
        
        case ING_PHY_TO_IDB_PORT_MAPm:
            for (index =0; index < ING_PHY_TO_IDB_PORT_MAPm_MAX_INDEX; index ++) {
                _soc_mem_read(_bcmsw->dev, ING_PHY_TO_IDB_PORT_MAPm+index, 
                              SCHAN_BLK_IPIPE, BYTES2WORDS(ING_PHY_TO_IDB_PORT_MAPm_BYTES), 
                              entry);
                //VALIDf bit 0
                if(entry[0] & 0x1) {
                    seq_printf(m, "[%2d]   0x%08x \n", 
                               index,
                               entry[0]);
                }
            }    
            break;

        case ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm:
            for (index =0; index < HX5_NUM_PORT; index ++) {
                _soc_mem_read(_bcmsw->dev, 
                              ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm+index, 
                              SCHAN_BLK_IPIPE, 
                              BYTES2WORDS(ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm_BYTES), 
                              entry);
                seq_printf(m, "[%2d]   0x%08x \n", 
                           index,
                           entry[0]);
            }      
            break;

        case STG_TABm:
            for (index =0; index < STG_TABm_MAX_INDEX; index ++) {
                val = 0;
                _mem_field_get(_bcmsw->stg_info->stg_bitmap,(STG_TABm_MAX_INDEX+1)/8, index, 1, &val, 0 );
                if (val) {
                    _soc_mem_read(_bcmsw->dev, 
                        STG_TABm+index, 
                        SCHAN_BLK_IPIPE, 
                        BYTES2WORDS(STG_TABm_BYTES), 
                        entry);                    
                    seq_printf(m, "[%2d]   0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                        index,
                        entry[0],
                        entry[1],
                        entry[2],
                        entry[3],
                        entry[4],
                        entry[5],
                        entry[6],
                        entry[7],
                        entry[8],
                        entry[9],
                        entry[10]);
                }
            }
            break;

        case EGR_VLAN_STGm:
            for (index =0; index < STG_TABm_MAX_INDEX; index ++) {
                val = 0;
                _mem_field_get(_bcmsw->stg_info->stg_bitmap,(STG_TABm_MAX_INDEX+1)/8, index, 1, &val, 0 );
                if (val) {
                    _soc_mem_read(_bcmsw->dev, 
                        EGR_VLAN_STGm+index, 
                        SCHAN_BLK_EPIPE, 
                        BYTES2WORDS(STG_TABm_BYTES), 
                        entry);                    
                    seq_printf(m, "[%2d]   0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x \n", 
                        index,
                        entry[0],
                        entry[1],
                        entry[2],
                        entry[3],
                        entry[4],
                        entry[5],
                        entry[6],
                        entry[7],
                        entry[8],
                        entry[9]);
                }
            }        
            break;

        case L2Xm:
            for (index =0; index < 128; index ++) {
                val = 0;
                _soc_mem_read(_bcmsw->dev, L2Xm+index, 
                              SCHAN_BLK_IPIPE, BYTES2WORDS(L2Xm_BYTES), 
                               entry);      
    
                //VALIDf start 0, len 3
                _mem_field_get(entry, L2Xm_BYTES, 0, 3, &val, SOCF_LE);
    
                if (!val) {
                    continue;
                }
                  
                seq_printf(m, "[%2d]   0x%08x 0x%08x 0x%08x 0x%08x \n", 
                    index,
                    entry[0],
                    entry[1],
                    entry[2],
                    entry[3]);
            }   
            break;           

        case PMQPORT_WC_UCMEM_DATAm:
            memset(entry, 0, 16);
            _soc_mem_read(_bcmsw->dev, PMQPORT_WC_UCMEM_DATAm, 
                          SCHAN_BLK_PMQPORT0, 4, 
                          entry);   
            seq_printf(m, "[PMQPORT0]  0x%08x 0x%08x 0x%08x 0x%08x \n", entry[0], entry[1], entry[2], entry[3]);
            memset(entry, 0, 16);
            _soc_mem_read(_bcmsw->dev, PMQPORT_WC_UCMEM_DATAm, 
                          SCHAN_BLK_PMQPORT1, 4, 
                          entry);   
            seq_printf(m, "[PMQPORT1]  0x%08x 0x%08x 0x%08x 0x%08x \n", entry[0], entry[1], entry[2], entry[3]);
            memset(entry, 0, 16);
            _soc_mem_read(_bcmsw->dev, PMQPORT_WC_UCMEM_DATAm, 
                          SCHAN_BLK_PMQPORT2, 4, 
                          entry);   
            seq_printf(m, "[PMQPORT2]  0x%08x 0x%08x 0x%08x 0x%08x \n", entry[0], entry[1], entry[2], entry[3]);        
            break;

        case MMU_CTR_ING_DROP_MEMm:
            for (index =0; index < 72; index ++) {
                val = 0;
                _soc_mem_read(_bcmsw->dev, MMU_CTR_ING_DROP_MEMm+index, 
                          SCHAN_BLK_MMU_XPE, BYTES2WORDS(MMU_CTR_ING_DROP_MEMm_BYTES), 
                          entry);      
              
                seq_printf(m, "[%2d]   0x%08x 0x%08x 0x%08x\n", 
                    index,
                    entry[0],
                    entry[1],
                    entry[2]);
            }          
            break;

        default:
            seq_printf(m," Not implemented\n");
            break;
    } 
    return 0;
}

static ssize_t
_proc_mem_write(struct file *file, const char *ubuf, size_t count, loff_t *ppos) 
{
    _proc_reg_data_t *p_data = (_proc_reg_data_t *)pde_data(file_inode(file));
    printk("_prog_reg32_write handler 0x%08x\n", p_data->reg_addr);
    return -1;
}

static int _proc_mem_open(struct inode * inode, struct file * file)
{
    return single_open(file, _proc_mem_show, NULL);
}


static struct proc_ops _proc_mem_ops = 
{
    proc_open:       _proc_mem_open,
    proc_read:       seq_read,
    proc_lseek:      seq_lseek,
    proc_write:      _proc_mem_write,
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
        _soc_mem_read(_bcmsw->dev, 
              EGR_VLANm+index, 
              SCHAN_BLK_EPIPE, 
              BYTES2WORDS(EGR_VLANm_BYTES), 
              (uint32_t *)&ve); 

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
        _soc_mem_read(_bcmsw->dev, 
              VLAN_ATTRS_1m+index, 
              SCHAN_BLK_IPIPE, 
              BYTES2WORDS(VLAN_ATTRS_1m_BYTES), 
              (uint32_t *)&vlan_attrs); 

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
        _soc_mem_read(_bcmsw->dev, 
              VLAN_TABm+index, 
              SCHAN_BLK_IPIPE, 
              BYTES2WORDS(VLAN_TABm_BYTES), 
              (uint32_t *)&vt); 

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
            _mem_field_get((uint32_t *)&vt, VLAN_ATTRS_1m_BYTES, 318, 13, &val, SOCF_LE);
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


// /proc/switchdev/mem/EGR_VLAN_VFI_UNTAG
static int
_egr_vlan_vfi_untag_show(struct seq_file *m, void *v)
{
    int                 index;
    egr_vlan_vfi_untag_entry_t    vt;

    if (!_bcmsw) {
        seq_printf(m, " Not initialized\n");
        return 0;
    }

    seq_printf(m, "EGR_VLAN_VFI_UNTAG base 0x%x (%d bytes):\n", EGR_VLAN_VFI_UNTAGm, EGR_VLAN_VFI_UNTAGm_BYTES);

    
    for (index = 0; index < 4; index ++) {
        //VLAN_ATTRS_1 entry is 19 bytes, 5 word
        _soc_mem_read(_bcmsw->dev, 
              EGR_VLAN_VFI_UNTAGm+index, 
              SCHAN_BLK_EPIPE, 
              BYTES2WORDS(EGR_VLAN_VFI_UNTAGm_BYTES), 
              (uint32_t *)&vt); 

        seq_printf(m, "[%4d] RAW 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", index, 
                   vt.entry_data[0], vt.entry_data[1], vt.entry_data[2], vt.entry_data[3], vt.entry_data[4]);
    }    
    return 0;
}

static ssize_t _egr_vlan_vfi_untag_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
    printk("_egr_vlan_vfi_untag_write handler\n");
    return -1;
}

static int _egr_vlan_vfi_untag_open(struct inode * inode, struct file * file)
{
    return single_open(file, _egr_vlan_vfi_untag_show, NULL);
}
static struct proc_ops egr_vlan_vfi_untag_ops = 
{
    proc_open:       _egr_vlan_vfi_untag_open,
    proc_read:       seq_read,
    proc_lseek:      seq_lseek,
    proc_write:      _egr_vlan_vfi_untag_write,
    proc_release:    single_release,
};


// /proc/switchdev/mem/L2_USER_ENTRY
static int
_l2_user_entry_show(struct seq_file *m, void *v)
{
    int                 index;
    uint32_t            val;
    l2u_entry_t         vt;

    if (!_bcmsw) {
        seq_printf(m, " Not initialized\n");
        return 0;
    }

    seq_printf(m, "L2_USER_ENTRY base 0x%x (%d bytes):\n", L2_USER_ENTRYm, L2_USER_ENTRYm_BYTES);

    
    for (index = 0; index < 512; index ++) {
        //L2_USER_ENTRY entry is 27 bytes, 7 word
        _soc_mem_read(_bcmsw->dev, 
              L2_USER_ENTRYm+index, 
              SCHAN_BLK_IPIPE, 
              BYTES2WORDS(L2_USER_ENTRYm_BYTES), 
              (uint32_t *)&vt); 
        //VALIDf start 0, len 1
        _mem_field_get((uint32_t *)&vt, L2_USER_ENTRYm_BYTES, 0, 1, &val, 0);        

        if(val & 0x1) {
            seq_printf(m, "[%4d] RAW 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n", index, 
                   vt.entry_data[0], vt.entry_data[1], vt.entry_data[2], vt.entry_data[3],
                   vt.entry_data[4], vt.entry_data[5], vt.entry_data[6]);
        }
    }    
    return 0;
}

static ssize_t _l2_user_entry_write(struct file *file, const char __user *ubuf,size_t count, loff_t *ppos) 
{
    printk("_l2_user_entry_write handler\n");
    return -1;
}

static int _l2_user_entry_open(struct inode * inode, struct file * file)
{
    return single_open(file, _l2_user_entry_show, NULL);
}
static struct proc_ops l2_user_entry_ops = 
{
    proc_open:       _l2_user_entry_open,
    proc_read:       seq_read,
    proc_lseek:      seq_lseek,
    proc_write:      _l2_user_entry_write,
    proc_release:    single_release,
};

// /proc/switchdev/stats/ge*
static int
_proc_port_counters_show(struct seq_file *m, void *v)
{
    int port, phy_port, index, blk_no, mmu_port;
    uint32_t val;
    uint32_t entry[SOC_MAX_MEM_WORDS];
    _proc_stats_data_t *p_data = (_proc_stats_data_t *)pde_data(file_inode(m->file));

    if (!_bcmsw) {
        seq_printf(m," Not initialized\n");
        return 0;
    }
  
    port = p_data->port; //logical port number
    phy_port = _bcmsw->si->port_l2p_mapping[port];
    mmu_port = _bcmsw->si->port_l2i_mapping[port];

    blk_no = gxblk[(phy_port-1)/8];
    index = (phy_port -1)%8;

    seq_printf(m, "RX counters\n"); 
    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRPKTr + index, &val);
    seq_printf(m, "    [GRPKT]                            Frames: %d\n", val); 

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRBYTr + index, &val);
    seq_printf(m, "    [GRBYT]                             Bytes: %d\n", val); 

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRPOKr + index, &val);
    seq_printf(m, "    [GRPOK]                       Good Frames: %d\n", val);     

    memset(entry, 0, sizeof(entry));
    _soc_mem_read(_bcmsw->dev,
                  MMU_CTR_ING_DROP_MEMm + mmu_port, 
                  SCHAN_BLK_MMU_XPE,
                  BYTES2WORDS(MMU_CTR_ING_DROP_MEMm_BYTES), entry);

    //PKTCNT start 0, len 31
    val = 0;
    _mem_field_get(entry, MMU_CTR_ING_DROP_MEMm_BYTES, 0, 31, &val, SOCF_LE);
    seq_printf(m, "    [DROP_PKT_ING %2d]        MMU drop packets: %d\n", mmu_port, val);         

    //BYTECNT start 69, len 32
    val = 0;
    _mem_field_get(entry, MMU_CTR_ING_DROP_MEMm_BYTES, 69, 32, &val, SOCF_LE);
    seq_printf(m, "    [DROP_BYTE_ING %2d]         MMU drop bytes: %d\n", mmu_port, val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, RDBGC0_64r + index, &val);
    seq_printf(m, "    [RDBGC0_64] Dropped packets(incl aborted): %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRUCr + index, &val);
    seq_printf(m, "    [GRUC]                      Unicast Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRMCAr + index, &val);
    seq_printf(m, "    [GRMCA]                   Multicast Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRBCAr + index, &val);
    seq_printf(m, "    [GRBCA]                   Broadcast Frame: %d\n", val);    
    
    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRFRGr + index, &val);
    seq_printf(m, "    [GRFRG]                    Fragment Frame: %d\n", val);      

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRUNDr + index, &val);
    seq_printf(m, "    [GRUND]                   Undersize Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRRPKTr + index, &val);
    seq_printf(m, "    [GRRPKT]                       RUNT Frame: %d\n", val);        

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRRBYTr + index, &val);
    seq_printf(m, "    [GRRBYT]                        RUNT Byte: %d\n", val);        

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GR64r + index, &val);
    seq_printf(m, "    [GR64]                      64 Byte Frame: %d\n", val);       

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GR127r + index, &val);
    seq_printf(m, "    [GR127]              65 to 127 Byte Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GR255r + index, &val);
    seq_printf(m, "    [GR255]             128 to 255 Byte Frame: %d\n", val);     
    
    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GR511r + index, &val);
    seq_printf(m, "    [GR511]             256 to 511 Byte Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GR1023r + index, &val);
    seq_printf(m, "    [GR1023]           512 to 1023 Byte Frame: %d\n", val);    
    
    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GR1518r + index, &val);
    seq_printf(m, "    [GR1518]          1024 to 1518 Byte Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GR2047r + index, &val);
    seq_printf(m, "    [GR2047]          1519 to 2047 Byte Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GR4095r + index, &val);
    seq_printf(m, "    [GR4095]          2048 to 4095 Byte Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GR9216r + index, &val);
    seq_printf(m, "    [GR9216]          4096 to 9216 Byte Frame: %d\n", val);   

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRMGVr + index, &val);
    seq_printf(m, "    [GRMGV] 1519 to 1522 Byte Good VLAN Frame: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRFCSr + index, &val);
    seq_printf(m, "    [GRFCS]                   FCS Error Frame: %d\n", val); 

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRXCFr + index, &val);
    seq_printf(m, "    [GRXCF]                     Control Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRXPFr + index, &val);
    seq_printf(m, "    [GRXPF]                       Pause Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRPFCr + index, &val);
    seq_printf(m, "    [GRPFC]                         PFC Frame: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRXUOr + index, &val);
    seq_printf(m, "    [GRXUO]          Unsupported Opcode Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRALNr + index, &val);
    seq_printf(m, "    [GRALN]             Alignment Error Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRCDEr + index, &val);
    seq_printf(m, "    [GRCDE]                  Code Error Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRFCRr + index, &val);
    seq_printf(m, "    [GRFCR]               False Carrier Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GROVRr + index, &val);
    seq_printf(m, "    [GROVR]                   Oversized Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRJBRr + index, &val);
    seq_printf(m, "    [GRJBR]                      Jabber Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRMTUEr + index, &val);
    seq_printf(m, "    [GRMTUE]            MTU Check Error Frame: %d\n", val);    

    //Ingress PIPE counters
    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RUC_64r + index, &val);
    seq_printf(m, "    [RUC_64]          Receive Unicast Counter: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RIPC4_64r + index, &val);
    seq_printf(m, "    [RIPC4_64] IPv4 L3 Unicast Packet Counter: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RIPC6_64r + index, &val);
    seq_printf(m, "    [RIPC6_64] IPv6 L3 Unicast Packet Counter: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RPORTD_64r + index, &val);
    seq_printf(m, "    [RPORTD_64]         PortInDiscard Counter: %d\n", val);   

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RPARITYDr + index, &val);
    seq_printf(m, "    [RPARITYD]          PortInDiscard Counter: %d\n", val);      

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RIPD4_64r + index, &val);
    seq_printf(m, "    [RIPD4_64] IPv4 L3 Discard Packet Counter: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RIPD6_64r + index, &val);
    seq_printf(m, "    [RIPD6_64] IPv6 L3 Discard Packet Counter: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RIPHE4_64r + index, &val);
    seq_printf(m, "    [RIPHE4_64]IPv4 L3 IP Header Error Packet: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, IMRP4_64r + index, &val);
    seq_printf(m, "    [IMRP4_64]IPv4 L3 Routed Multicast Packet: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RIPHE6_64r + index, &val);
    seq_printf(m, "    [RIPHE6_64]IPv6 L3 IP Header Error Packet: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, IMRP6_64r + index, &val);
    seq_printf(m, "    [IMRP6_64]IPv6 L3 Routed Multicast Packet: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, RTUNr + index, &val);
    seq_printf(m, "    [RTUN]      Good Tunnel terminated Packet: %d\n", val);      

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, ICTRL_64r + index, &val);
    seq_printf(m, "    [ICTRL_64]    Receive HiGig (Control Op.): %d\n", val);      

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, IBCAST_64r + index, &val);
    seq_printf(m, "    [IBCAST_64] Receive HiGig (Broadcast Op.): %d\n", val);      
    
    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, IIPMC_64r + index, &val);
    seq_printf(m, "    [IIPMC_64]       Receive HiGig (IPMC Op.): %d\n", val);      

    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, ING_NIV_RX_FRAMES_ERROR_DROP_64r + index, &val);
    seq_printf(m, "    [ING_NIV_RX_FRAMES_ERROR_DROP_64]       VNTAG/ETAG format errors: %d\n", val);    
    
    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, ING_NIV_RX_FRAMES_FORWARDING_DROP_64r + index, &val);
    seq_printf(m, "    [ING_NIV_RX_FRAMES_FORWARDING_DROP_64]  NIV/PE forwarding errors: %d\n", val);    
    
    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, ING_NIV_RX_FRAMES_VLAN_TAGGED_64r + index, &val);
    seq_printf(m, "    [ING_NIV_RX_FRAMES_VLAN_TAGGED_64]  VLAN tagged packets received: %d\n", val);    
    
    val = 0;
    _reg32_read(_bcmsw->dev, SCHAN_BLK_IPIPE, ING_ECN_COUNTER_64r + index, &val);
    seq_printf(m, "    [ING_ECN_COUNTER_64]           ECN field matched on tunnel decap: %d\n", val);    

    val = 0;
    seq_printf(m, "\nTX counters\n"); 
    _reg32_read(_bcmsw->dev, blk_no, GTPKTr + index, &val);
    seq_printf(m, "    [GTPKT]                            Frames: %d\n", val); 

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTBYTr + index, &val);
    seq_printf(m, "    [GTBYT]                             Bytes: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTPOKr + index, &val);
    seq_printf(m, "    [GTPOK]                       Good Frames: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTUCr + index, &val);
    seq_printf(m, "    [GTUC]                      Unicast Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTMCAr + index, &val);
    seq_printf(m, "    [GTMCA]                   Multicast Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTBCAr + index, &val);
    seq_printf(m, "    [GTBCA]                   Broadcast Frame: %d\n", val);    
    
    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTFRGr + index, &val);
    seq_printf(m, "    [GTFRG]                    Fragment Frame: %d\n", val);      

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTOVRr + index, &val);
    seq_printf(m, "    [GTOVR]                    Oversize Frame: %d\n", val);        

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GRRBYTr + index, &val);
    seq_printf(m, "    [GRRBYT]                        RUNT Byte: %d\n", val);        

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GT64r + index, &val);
    seq_printf(m, "    [GT64]                      64 Byte Frame: %d\n", val);       

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GT127r + index, &val);
    seq_printf(m, "    [GT127]              65 to 127 Byte Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GT255r + index, &val);
    seq_printf(m, "    [GT255]             128 to 255 Byte Frame: %d\n", val);     
    
    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GT511r + index, &val);
    seq_printf(m, "    [GT511]             256 to 511 Byte Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GT1023r + index, &val);
    seq_printf(m, "    [GT1023]           512 to 1023 Byte Frame: %d\n", val);    
    
    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GT1518r + index, &val);
    seq_printf(m, "    [GT1518]          1024 to 1518 Byte Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GT2047r + index, &val);
    seq_printf(m, "    [GT2047]          1519 to 2047 Byte Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GT4095r + index, &val);
    seq_printf(m, "    [GT4095]          2048 to 4095 Byte Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GT9216r + index, &val);
    seq_printf(m, "    [GT9216]          4096 to 9216 Byte Frame: %d\n", val);   

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTMGVr + index, &val);
    seq_printf(m, "    [GTMGV] 1519 to 1522 Byte Good VLAN Frame: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTXPFr + index, &val);
    seq_printf(m, "    [GTXPF]               Pause Control Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTJBRr + index, &val);
    seq_printf(m, "    [GTJBR]                      Jabber Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTFCSr + index, &val);
    seq_printf(m, "    [GTFCS]                   FCS Error Frame: %d\n", val);     

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTPFCr + index, &val);
    seq_printf(m, "    [GTPFC]                         PFC Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTDFRr + index, &val);
    seq_printf(m, "    [GTDFR]             Single Deferral Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTEDFr + index, &val);
    seq_printf(m, "    [GTEDF]           Multiple Deferral Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTSCLr + index, &val);
    seq_printf(m, "    [GTSCL]            Single Collision Frame: %d\n", val);    

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTMCLr + index, &val);
    seq_printf(m, "    [GTMCL]          Multiple Collision Frame: %d\n", val);   

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTLCLr + index, &val);
    seq_printf(m, "    [GTLCL]              Late Collision Frame: %d\n", val);  

    val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTXCLr + index, &val);
    seq_printf(m, "    [GTXCL]         Excessive Collision Frame: %d\n", val);  

     val = 0;
    _reg32_read(_bcmsw->dev, blk_no, GTNCLr + index, &val);
    seq_printf(m, "    [GTNCL]             Total Collision Frame: %d\n", val);  



    seq_printf(m, "\n");
    return 0;
}

static int _proc_port_counters_open(struct inode * inode, struct file * file)
{
    return single_open(file, _proc_port_counters_show, NULL);
}


static struct proc_ops _proc_port_counters_ops = 
{
    proc_open:       _proc_port_counters_open,
    proc_read:       seq_read,
    proc_lseek:      seq_lseek,
    proc_release:    single_release,
};

/*****************************************************************************************/
static int _procfs_stats_init(bcmsw_switch_t *bcmsw)
{
    struct proc_dir_entry *entry;
    _proc_stats_data_t *p_data;

    // /proc/switchdev/stats
    proc_stats_base = proc_mkdir("switchdev/stats", NULL);

    // /proc/switchdev/stats/ge0
    p_data = kmalloc(sizeof(_proc_stats_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_stats_data_t));
    p_data->port = 1;
    entry = proc_create_data("ge0", 0666, proc_stats_base, &_proc_port_counters_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/stats/ge1
    p_data = kmalloc(sizeof(_proc_stats_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_stats_data_t));
    p_data->port = 2;
    entry = proc_create_data("ge1", 0666, proc_stats_base, &_proc_port_counters_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    // /proc/switchdev/stats/ge2
    p_data = kmalloc(sizeof(_proc_stats_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_stats_data_t));
    p_data->port = 3;
    entry = proc_create_data("ge2", 0666, proc_stats_base, &_proc_port_counters_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }    


    return 0;

create_fail:
    return -1;
}


static int _procfs_reg_init(bcmsw_switch_t *bcmsw)
{
    struct proc_dir_entry *entry;
    _proc_reg_data_t *p_data;

    // /proc/switchdev/reg
    proc_reg_base = proc_mkdir("switchdev/reg", NULL);

    // /proc/switchdev/reg/COMMAND_CONFIG
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = COMMAND_CONFIGr;
    entry = proc_create_data("COMMAND_CONFIG", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/reg/GPORT_MODE_REG
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = GPORT_MODE_REGr;
    p_data->num_blk = 6;
    p_data->block[0] = SCHAN_BLK_GXPORT0;
    p_data->block[1] = SCHAN_BLK_GXPORT1;
    p_data->block[2] = SCHAN_BLK_GXPORT2;
    p_data->block[3] = SCHAN_BLK_GXPORT3;
    p_data->block[4] = SCHAN_BLK_GXPORT4;
    p_data->block[5] = SCHAN_BLK_GXPORT5; 
    entry = proc_create_data("GPORT_MODE_REG", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/reg/GPORT_RSV_MASK
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = GPORT_RSV_MASKr;
    entry = proc_create_data("GPORT_RSV_MASK", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    
    // /proc/switchdev/reg/GPORT_CONFIG
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = GPORT_CONFIGr;
    entry = proc_create_data("GPORT_CONFIG", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    
    // /proc/switchdev/reg/GPORT_STAT_UPDATE_MASK
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = GPORT_STAT_UPDATE_MASKr;
    entry = proc_create_data("GPORT_STAT_UPDATE_MASK", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }    

    // /proc/switchdev/reg/IDB_OBM_CONTROL
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = IDB_OBM0_Q_CONTROLr;
    entry = proc_create_data("IDB_OBM_CONTROL", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }


    // /proc/switchdev/reg/IDB_OBM_CA_CONTROL
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = IDB_OBM0_Q_CA_CONTROLr;
    entry = proc_create_data("IDB_OBM_CA_CONTROL", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    
    // /proc/switchdev/reg/MMU_GCFG_MISCCONFIG
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = MMU_GCFG_MISCCONFIGr;
    entry = proc_create_data("MMU_GCFG_MISCCONFIG", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/reg/EGR_DEVICE_TO_PHYSICAL_PORT_NUMBER_MAPPING
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = EGR_DEVICE_TO_PHYSICAL_PORT_NUMBER_MAPPINGr;
    entry = proc_create_data("EGR_DEVICE_TO_PHYSICAL_PORT_NUMBER_MAPPING", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    
    // /proc/switchdev/reg/MMU_PORT_TO_PHY_PORT_MAPPING
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = MMU_PORT_TO_PHY_PORT_MAPPINGr;
    entry = proc_create_data("MMU_PORT_TO_PHY_PORT_MAPPING", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    
    // /proc/switchdev/reg/MMU_PORT_TO_DEVICE_PORT_MAPPING
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = MMU_PORT_TO_DEVICE_PORT_MAPPINGr;
    entry = proc_create_data("MMU_PORT_TO_DEVICE_PORT_MAPPING", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }    

    // /proc/switchdev/reg/IDB_CA_LPBK_CONTROL
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = IDB_CA_LPBK_CONTROLr;
    entry = proc_create_data("IDB_CA_LPBK_CONTROL", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }    
    // /proc/switchdev/reg/IDB_CA_CPU_CONTROL
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = IDB_CA_CPU_CONTROLr;
    entry = proc_create_data("IDB_CA_CPU_CONTROL", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }    
    // /proc/switchdev/reg/IDB_CA_BSK_CONTROL
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = IDB_CA_BSK_CONTROLr;
    entry = proc_create_data("IDB_CA_BSK_CONTROL", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }                
    // /proc/switchdev/reg/PMQ_XGXS0_CTRL_REG
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = PMQ_XGXS0_CTRL_REGr;
    p_data->num_blk = 3;
    p_data->block[0] = SCHAN_BLK_PMQPORT0;
    p_data->block[1] = SCHAN_BLK_PMQPORT1;
    p_data->block[2] = SCHAN_BLK_PMQPORT2;
    entry = proc_create_data("PMQ_XGXS0_CTRL_REG", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }                
    // /proc/switchdev/reg/PMQ_ECC_INIT_CTRL
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = PMQ_ECC_INIT_CTRLr;
    entry = proc_create_data("PMQ_ECC_INIT_CTRL", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    // /proc/switchdev/reg/PMQ_ECC_INIT_STS
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = PMQ_ECC_INIT_STSr;
    entry = proc_create_data("PMQ_ECC_INIT_STS", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    // /proc/switchdev/reg/PMQ_ECC
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = PMQ_ECCr;
    entry = proc_create_data("PMQ_ECC", 0666, proc_reg_base, &_proc_reg32_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    return 0;
create_fail:
    return -1;
}

static int _procfs_mem_init(bcmsw_switch_t *bcmsw)
{
    struct proc_dir_entry *entry;
    _proc_reg_data_t *p_data;


    // /proc/switchdev/mem
    proc_mem_base = proc_mkdir("switchdev/mem", NULL);


    // /proc/switchdev/mem/EGR_VLAN
    entry = proc_create("EGR_VLAN", 0666, proc_mem_base, &egr_vlan_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/VLAN_ATTRS_1
    entry = proc_create("VLAN_ATTRS_1", 0666, proc_mem_base, &vlan_attrs_1_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/VLAN_TAB
    entry = proc_create("VLAN_TAB", 0666, proc_mem_base, &vlan_tab_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/EGR_VLAN_VFI_UNTAG
    entry = proc_create("EGR_VLAN_VFI_UNTAG", 0666, proc_mem_base, &egr_vlan_vfi_untag_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }    
    
    // /proc/switchdev/mem/L2_USER_ENTRY
    entry = proc_create("L2_USER_ENTRY", 0666, proc_mem_base, &l2_user_entry_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }    

    // /proc/switchdev/mem/EGR_PORT
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = EGR_PORTm;
    entry = proc_create_data("EGR_PORT", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/EGR_ENABLE
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = EGR_ENABLEm;
    entry = proc_create_data("EGR_ENABLE", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    
    // /proc/switchdev/mem/EGR_LPORT_PROFILE
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = EGR_LPORT_PROFILEm;
    entry = proc_create_data("EGR_LPORT_PROFILE", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/EGR_GPP_ATTRIBUTES
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = EGR_GPP_ATTRIBUTESm;
    entry = proc_create_data("EGR_GPP_ATTRIBUTES", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/LPORT_TAB
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = LPORT_TABm;
    entry = proc_create_data("LPORT_TAB", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/ING_DEVICE_PORT
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = ING_DEVICE_PORTm;
    entry = proc_create_data("ING_DEVICE_PORT", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/MAC_BLOCK
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = MAC_BLOCKm;
    entry = proc_create_data("MAC_BLOCK", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/SYS_PORTMAP
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = SYS_PORTMAPm;
    entry = proc_create_data("SYS_PORTMAP", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/ING_PHY_TO_IDB_PORT_MAP
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = ING_PHY_TO_IDB_PORT_MAPm;
    entry = proc_create_data("ING_PHY_TO_IDB_PORT_MAP", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLE
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm;
    entry = proc_create_data("ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLE", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/EGR_VLAN_STG
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = EGR_VLAN_STGm;
    entry = proc_create_data("EGR_VLAN_STG", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    
    // /proc/switchdev/mem/EGR_VLAN_VFI_MEMBERSHIP
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = EGR_VLAN_VFI_MEMBERSHIPm;
    entry = proc_create_data("EGR_VLAN_VFI_MEMBERSHIP", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    
    // /proc/switchdev/mem/ING_VLAN_VFI_MEMBERSHIP
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = ING_VLAN_VFI_MEMBERSHIPm;
    entry = proc_create_data("ING_VLAN_VFI_MEMBERSHIP", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/mem/STG_TAB
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = STG_TABm;
    entry = proc_create_data("STG_TAB", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }
    // /proc/switchdev/mem/L2X
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = L2Xm;
    entry = proc_create_data("L2X", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }        
    
    // /proc/switchdev/mem/PMQPORT_WC_UCMEM_DATA
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = PMQPORT_WC_UCMEM_DATAm;
    entry = proc_create_data("PMQPORT_WC_UCMEM_DATA", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }        

    // /proc/switchdev/mem/EPC_LINK_BMAP
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = EPC_LINK_BMAPm;
    entry = proc_create_data("EPC_LINK_BMAP", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }            

    // /proc/switchdev/mem/ING_DEST_PORT_ENABLE
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = ING_DEST_PORT_ENABLEm;
    entry = proc_create_data("ING_DEST_PORT_ENABLE", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }            

    // /proc/switchdev/mem/MMU_CTR_ING_DROP_MEM
    p_data = kmalloc(sizeof(_proc_reg_data_t), GFP_KERNEL);
    memset(p_data, 0, sizeof(_proc_reg_data_t));
    p_data->reg_addr = MMU_CTR_ING_DROP_MEMm;
    entry = proc_create_data("MMU_CTR_ING_DROP_MEM", 0666, proc_mem_base, &_proc_mem_ops, p_data);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }     
    return 0;

create_fail:
    return -1;
}


int _procfs_init(bcmsw_switch_t *bcmsw)
{
    struct proc_dir_entry *entry;
    int rv;

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

    // /proc/switchdev/portstat
    entry = proc_create("portstat", 0666, proc_switchdev_base, &portstat_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/l2
    entry = proc_create("l2", 0666, proc_switchdev_base, &l2_ops);
    if (entry == NULL) {
        printk("proc_create failed!\n");
        goto create_fail;
    }

    // /proc/switchdev/reg
    rv = _procfs_reg_init(bcmsw);
    if (rv) {
        goto create_fail;
    }

    // /proc/switchdev/mem
    rv = _procfs_mem_init(bcmsw);
    if (rv) {
        goto create_fail;
    }

    // /proc/switchdev/stats
    rv = _procfs_stats_init(bcmsw);
    if (rv) {
        goto create_fail;
    }

    return 0;

create_fail:
    proc_remove(proc_switchdev_base);
    return -EINVAL;
}

int _procfs_uninit(bcmsw_switch_t *bcmsw)
{
    // /proc/switchdev/reg
    remove_proc_entry("COMMAND_CONFIG", proc_reg_base);
    remove_proc_entry("GPORT_MODE_REG", proc_reg_base);
    remove_proc_entry("GPORT_CONFIG", proc_reg_base);
    remove_proc_entry("GPORT_RSV_MASK", proc_reg_base);
    remove_proc_entry("GPORT_STAT_UPDATE_MASK", proc_reg_base);
    remove_proc_entry("IDB_OBM_CONTROL", proc_reg_base);
    remove_proc_entry("IDB_OBM_CA_CONTROL", proc_reg_base);
    remove_proc_entry("MMU_GCFG_MISCCONFIG", proc_reg_base);
    remove_proc_entry("EGR_DEVICE_TO_PHYSICAL_PORT_NUMBER_MAPPING", proc_reg_base);
    remove_proc_entry("MMU_PORT_TO_PHY_PORT_MAPPING", proc_reg_base);
    remove_proc_entry("MMU_PORT_TO_DEVICE_PORT_MAPPING", proc_reg_base);
    remove_proc_entry("IDB_CA_LPBK_CONTROL", proc_reg_base);
    remove_proc_entry("IDB_CA_CPU_CONTROL", proc_reg_base);
    remove_proc_entry("IDB_CA_BSK_CONTROL", proc_reg_base);
    remove_proc_entry("PMQ_XGXS0_CTRL_REG", proc_reg_base);
    remove_proc_entry("PMQ_ECC_INIT_CTRL", proc_reg_base);
    remove_proc_entry("PMQ_ECC_INIT_STS", proc_reg_base);
    remove_proc_entry("PMQ_ECC", proc_reg_base);
    
    remove_proc_entry("reg", proc_switchdev_base);

    // /proc/switchdev/mem
    remove_proc_entry("EGR_VLAN", proc_mem_base);
    remove_proc_entry("VLAN_ATTRS_1", proc_mem_base);
    remove_proc_entry("VLAN_TAB", proc_mem_base);
    remove_proc_entry("EGR_VLAN_VFI_UNTAG", proc_mem_base);
    remove_proc_entry("EGR_VLAN_VFI_MEMBERSHIP", proc_mem_base);
    remove_proc_entry("ING_VLAN_VFI_MEMBERSHIP", proc_mem_base);
    remove_proc_entry("L2_USER_ENTRY", proc_mem_base);
    remove_proc_entry("EGR_PORT", proc_mem_base);
    remove_proc_entry("EGR_ENABLE", proc_mem_base);
    remove_proc_entry("EGR_GPP_ATTRIBUTES", proc_mem_base);
    remove_proc_entry("EGR_LPORT_PROFILE", proc_mem_base);    
    remove_proc_entry("LPORT_TAB", proc_mem_base);
    remove_proc_entry("ING_DEVICE_PORT", proc_mem_base);
    remove_proc_entry("MAC_BLOCK", proc_mem_base);
    remove_proc_entry("SYS_PORTMAP", proc_mem_base);
    remove_proc_entry("ING_PHY_TO_IDB_PORT_MAP", proc_mem_base);
    remove_proc_entry("ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLE", proc_mem_base);
    remove_proc_entry("EGR_VLAN_STG", proc_mem_base);
    remove_proc_entry("STG_TAB", proc_mem_base);
    remove_proc_entry("L2X", proc_mem_base);
    remove_proc_entry("PMQPORT_WC_UCMEM_DATA", proc_mem_base);
    remove_proc_entry("EPC_LINK_BMAP", proc_mem_base);
    remove_proc_entry("ING_DEST_PORT_ENABLE", proc_mem_base);    
    remove_proc_entry("MMU_CTR_ING_DROP_MEM", proc_mem_base);    
    
    remove_proc_entry("mem", proc_switchdev_base);

    // /proc/switchdev/stats
    remove_proc_entry("ge0", proc_stats_base);
    remove_proc_entry("ge1", proc_stats_base);
    remove_proc_entry("ge2", proc_stats_base);
    remove_proc_entry("stats", proc_switchdev_base);


    remove_proc_entry("portstat", proc_switchdev_base);
    remove_proc_entry("sinfo", proc_switchdev_base);
    remove_proc_entry("l2", proc_switchdev_base);

    remove_proc_entry("switchdev", NULL);
    return 0;
}

/*****************************************************************************************/
/*                            stats/counter                                              */
/*****************************************************************************************/


static int
soc_counter_set32_by_port(bcmsw_switch_t *bcmsw, int port)
{
    uint32_t val = 0;
    int index, blk_no, phy_port;
  
    phy_port = _bcmsw->si->port_l2p_mapping[port];

    blk_no = gxblk[(phy_port-1)/8];
    index = (phy_port -1)%8;

    _reg32_write(bcmsw->dev, blk_no, GRPKTr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRBYTr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRPOKr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRUCr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRMCAr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRBCAr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRFRGr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRUNDr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRRPKTr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRRBYTr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GR64r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GR127r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GR255r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GR511r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GR1023r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GR1518r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GR2047r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GR4095r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GR9216r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRMGVr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRFCSr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRXCFr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRXPFr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRPFCr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRXUOr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRALNr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRCDEr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRFCRr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GROVRr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRJBRr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRMTUEr + index, val);

    _reg32_write(bcmsw->dev, blk_no, GTPKTr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTBYTr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTPOKr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTUCr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTMCAr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTBCAr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTFRGr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTOVRr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GRRBYTr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GT64r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GT127r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GT255r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GT511r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GT1023r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GT1518r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GT2047r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GT4095r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GT9216r + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTMGVr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTXPFr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTJBRr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTFCSr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTPFCr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTDFRr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTEDFr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTSCLr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTMCLr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTLCLr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTXCLr + index, val);
    _reg32_write(bcmsw->dev, blk_no, GTNCLr + index, val);

    return 0;
}

int
bcm_esw_stat_init(bcmsw_switch_t *bcmsw)
{
    int index;
    //soc_counter_set32_by_port

    for (index = 1; index <=48; index++){
        soc_counter_set32_by_port(bcmsw, index);
    }
    return 0;
}

