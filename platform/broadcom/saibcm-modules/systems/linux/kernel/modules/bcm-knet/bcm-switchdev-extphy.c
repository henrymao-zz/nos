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
#include "bcm-switchdev-extphy.h"



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
   //     phy_id, phy_reg_addr, real_phy_id, mdio_buses, internal_select, cycle_type);

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

int
phy_bcm542xx_reg_read(uint16_t phy_addr, uint16_t reg_bank,
                      uint8_t reg_addr, uint16_t *data)
{
    int     rv = SOC_E_NONE;
    uint16  val;

    switch ( reg_addr ) {
        /* Map shadow registers */
        case 0x15:
            phy_reg_write(phy_addr, 0x17, reg_bank);
            break;
        case 0x18:
            if ( reg_bank <= 0x0007 ) {
                val = (reg_bank << 12) | 0x7;
                phy_reg_write(phy_addr, reg_addr, val);
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case 0x1C:
            if ( reg_bank <= 0x001F ) {
                val = (reg_bank << 10);
                phy_reg_write(phy_addr, reg_addr, val);
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        case 0x1D:
            if ( reg_bank <= 0x0001 ) {
                val = reg_bank << 15;
            } else {
                rv = SOC_E_PARAM;
            }
            break;
        default:
            break;
    }
    if ( rv >= 0 ) {
        rv = phy_reg_read(phy_addr, reg_addr, data);
    }

    if ( rv < 0 ) {
        printk("phy_bcm542xx_reg_read: failed:"
               "phy_id=0x%2x reg_bank=0x%04x reg_addr=0x%02x "
                "rv=%d\n", phy_addr, reg_bank, reg_addr, rv);
    }
    return rv;
}

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
phy_bcm542xx_enable_get(port_info_t *pport, int port, uint16_t phy_addr, int *enable)
{
    uint16_t    power;
    phy_bcm542xx_reg_read(phy_addr, 0x0000,  PHY_BCM542XX_MII_CTRL_REG, &power);

    if (power &PHY_BCM542XX_MII_CTRL_PWR_DOWN ) {
        *enable = 0;
    } else {
        *enable = 1;
    }
    //return phy_fe_ge_enable_get(pport, enable);
    return 0;
}

static int
phy_bcm542xx_autoneg_get(port_info_t *pport, int port, uint16_t phy_addr,
                         int *autoneg, int *autoneg_done)
{

    uint16_t mii_ctrl, mii_stat;

    *autoneg      = FALSE;
    *autoneg_done = FALSE;

    //Copper only
    phy_bcm542xx_reg_read(phy_addr, 0x0000,  MII_CTRL_REG, &mii_ctrl);
    phy_bcm542xx_reg_read(phy_addr, 0x0000,  MII_STAT_REG, &mii_stat);
    *autoneg      = (mii_ctrl & PHY_BCM542XX_MII_CTRL_AN_EN)   ? 1 : 0;
    *autoneg_done = (mii_stat & PHY_BCM542XX_MII_STAT_AN_DONE) ? 1 : 0;

    return ( SOC_E_NONE );
}

//Determine the current link up/down status for a 542xx device.
static int
phy_bcm542xx_link_get(port_info_t *pport, int port, uint16_t phy_addr, int *link)
{
    int      count = 0;
    uint16_t mii_ctrl, mii_stat;
    *link = FALSE;      /* Default return */

    //if ( PHY_COPPER_MODE(unit, port) ) {
    //    SOC_IF_ERROR_RETURN(phy_fe_ge_link_get(unit, port, link));

    phy_bcm542xx_reg_read(phy_addr,0x0000, MII_STAT_REG, &mii_stat);

    //printk("phy_bcm542xx_link_get: port =%d phy_addr=%d mii_stat=%d\n",port, phy_addr, mii_stat);

    if (!(mii_stat & MII_STAT_LA) || (mii_stat == 0xffff)) {
        /* mii_stat == 0xffff check is to handle removable PHY daughter cards */
        return SOC_E_NONE;
    }
    
    /* Link appears to be up; we are done if autoneg is off. */
    phy_bcm542xx_reg_read(phy_addr,0x0000, MII_CTRL_REG, &mii_ctrl);

    if (!(mii_ctrl & MII_CTRL_AE)) {
        *link = TRUE;
        return SOC_E_NONE;
    }
        
   /*
     * If link appears to be up but autonegotiation is still in
     * progress, wait for it to complete.  For BCM5228, autoneg can
     * still be busy up to about 200 usec after link is indicated.  Also
     * continue to check link state in case it goes back down.
     */
    //250ms
    count = 0;
    for (;;) {
        phy_bcm542xx_reg_read(phy_addr,0x0000, MII_STAT_REG, &mii_stat);

        if (!(mii_stat & MII_STAT_LA)) {
            return SOC_E_NONE;
        }

        if (mii_stat & MII_STAT_AN_DONE) {
            break;
        }

        msleep(10);
        count++;
        if (count > 20) { 
            return SOC_E_BUSY;
        }
    }

    /* Return link state at end of polling */
    *link = ((mii_stat & MII_STAT_LA) != 0);        

    return SOC_E_NONE;
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
    p_port->dev_desc.flags |= PHY_BCM542XX_SYS_SIDE_AUTONEG;

    /* Configure LED mode and turn on jumbo frame support */
    //_phy_bcm54280_init(unit, port);
    /*
     * PHY Extended Control Reg., RDB.0x000, Legacy Reg.0x10
     * bit[0] GMII_FIFO_JUMBO_LSB
     * bit[5] LED_TRAFFIC_EN, Enable LED traffic mode
     */
    //PHY_BCM542XX_RDB_MO(_unit, _pc, 0x000, _val, _mask)
    phy_bcm542xx_rdb_reg_modify(phy_id,
                                0x000,
                                PHY_BCM542XX_MII_ECR_FIFO_ELAST_0
                                | PHY_BCM542XX_MII_ECR_EN_LEDT,
                                PHY_BCM542XX_MII_ECR_FIFO_ELAST_0
                                | PHY_BCM542XX_MII_ECR_EN_LEDT);

    /*
     * Pattern Generator Status Reg., RDB.0x006, Legacy EXP_Reg.0x46
     * bit[14] GMII_FIFO_JUMBO_MSB
     * bit[15] GMII_FIFO_JUMBO_MSB
     */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                                0x006,
                                PHY_BCM542XX_PATT_GEN_STAT_FIFO_ELAST_1
                                | PHY_BCM542XX_PATT_GEN_STAT_GMII_FIFO_JUMBO_MSB,
                                PHY_BCM542XX_PATT_GEN_STAT_FIFO_ELAST_1
                                | PHY_BCM542XX_PATT_GEN_STAT_GMII_FIFO_JUMBO_MSB);

    /* Disable USGMII mode. Clear RDB.0x0810 Bit[6]  */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                               PHY_BCM542XX_DISABLE_USGMII,
                               0, PHY_BCM542XX_DISABLE_USGMII_6U_BIT_SEL);
    /*
     * Misc 1000-X Control 2 Reg., RDB.0x236, Legacy Reg.0x1C shadow 0x16
     * bit[0] PRIMARY_SERDES_JUMBO_MSB
     */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                                0x236,
                               PHY_BCM542XX_SERDES_CTRL_JUMBO_MSB,
                               PHY_BCM542XX_SERDES_CTRL_JUMBO_MSB);

    /*
     * Auto Detect SGMII MC Reg., RDB.0x238
     * bit[2] GMII_FIFO_JUMBO_LSB
     */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                                PHY_BCM542XX_AUTO_DETECT_SGMII_MC_REG_OFFSET,
                                PHY_BCM542XX_AUTO_DETECT_GMII_FIFO_JUMBO_LSB,
                                PHY_BCM542XX_AUTO_DETECT_GMII_FIFO_JUMBO_LSB);

    /*
     * Auxiliary 1000-X Contro Reg., RDB.0x23B, Legacy Reg.0x1C shadow 0x1B
     * bit[1] PRIMARY_SERDES_JUMBO_LSB
     */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                                0x23B,
                                PHY_BCM542XX_SERDES_CTRL_JUMBO_LSB,
                                PHY_BCM542XX_SERDES_CTRL_JUMBO_LSB);

    /*
     * Secondary Serdes MISC 1000-X Control 2 Reg. RDB.0xB16
     * bit[0] SECONDARY_SERDES_JUMBO_MSB
     */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                            PHY_BCM542XX_2ND_SERDES_MISC_1000x_CTL_2_REG_OFFSET,
                            PHY_BCM542XX_2ND_SERDES_MISC_1000x_CTL_2_2ND_SERDES_MSB,
                            PHY_BCM542XX_2ND_SERDES_MISC_1000x_CTL_2_2ND_SERDES_MSB);

    /*
     * Secondary Serdes Auxiliary 1000-X Control Reg. RDB.0xB1B
     * bit[1] SECONDARY_SERDES_JUMBO_LSB
     */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                            PHY_BCM542XX_2ND_SERDES_AUX_1000x_CTL_REG_OFFSET,
                            PHY_BCM542XX_2ND_SERDES_AUX_1000x_CTL_2ND_SERDES_LSB,
                            PHY_BCM542XX_2ND_SERDES_AUX_1000x_CTL_2ND_SERDES_LSB);

    /*
     * Copper Auxiliary Control Reg., RDB.0x028, Legacy Reg.0x18 shadow 0
     * bit[14] EXT_PKT_LEN(100BT_JUMBO), 100BASE-Tx jumbo frames support
     */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                            0x028,
                            PHY_BCM542XX_MII_AUX_CTRL_REG_EXT_PKT_LEN,
                            PHY_BCM542XX_MII_AUX_CTRL_REG_EXT_PKT_LEN);

    /*
     * Secondary Serdes 1000-X Error Counter Setting Reg. RDB.0xB10
     * bit[0] EXT_PKT_LEN
     */
    phy_bcm542xx_rdb_reg_modify(phy_id,
                            PHY_BCM542XX_2ND_SERDES_1000x_ERR_CNT_SET_REG_OFFSET,
                            PHY_BCM542XX_2ND_SERDES_1000x_ERR_CNT_SET_EXT_PKT_LEN,
                            PHY_BCM542XX_2ND_SERDES_1000x_ERR_CNT_SET_EXT_PKT_LEN);

    // end of _phy_bcm54280_init()
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

int
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
    phy_id = p_port->dev_desc.phy_id_base;
    dev_port = p_port->dev_desc.phy_slice;

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
    //phy_bcm542xx_reg_modify(phy_id, 0x0faf,0x15, 0x0000, 0xC000);
    /* Reset counters and other settings */
    //SOC_IF_ERROR_RETURN(
    //        PHY_MODIFY_BCM542XX_EEE_STAT_CTRLr(unit, pc, 0x0000, 0x3fff) );
    phy_bcm542xx_reg_modify(phy_id, 0x0faf,0x15, 0x0000, 0x3fff);

    /* AutogrEEEn */

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
/*                             Phy Probe                                                 */
/*****************************************************************************************/

int
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