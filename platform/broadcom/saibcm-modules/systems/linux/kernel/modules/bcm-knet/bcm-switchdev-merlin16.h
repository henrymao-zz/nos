#ifndef _BCM_SWITCHDEV_MERLIN16_H_
#define _BCM_SWITCHDEV_MERLIN16_H_
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
#include "bcm-switchdev.h"




/*****************************************************************************************/
/*                           PM4X10 QTC   BCM56370                                       */
/*****************************************************************************************/

#define SOC_REG_ABOVE_64_MAX_SIZE_U32 30 

typedef uint32_t soc_reg_above_64_val_t[SOC_REG_ABOVE_64_MAX_SIZE_U32];


//Memory: PMQPORT_WC_UCMEM_DATA.pmqport0 address 0x0000f000
//Flags: valid cachable(off)
//Blocks:  pmqport0/dma/slam pmqport1/dma/slam pmqport2/dma/slam (3 copies, 3 dmaable, 3 slamable)
//Entries: 2048 with indices 0-2047 (0x0-0x7ff), each 16 bytes 4 words
//Entry mask: -1 -1 -1 -1
//Description: PSC to PMD External Memory Interface to program micro-controller memory
//  UC_DATA<127:0>
#define   PMQPORT_WC_UCMEM_DATAm        0x0000f000



#define PHYMOD_REG_ACCESS_FLAGS_SHIFT      24
#define PHYMOD_REG_ACC_TSC_IBLK            (7<<28)
#define PHYMOD_REG_ACC_AER_IBLK_FORCE_LANE (8<<24)
#define PHYMOD_REG_ACC_TSC_IBLK_WR_ONLY    (1<<23)

#define QTCE16_NOF_LANES_IN_CORE           4
#define PHYMOD_MAX_LANES_PER_CORE          12

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  PMD_X1_CTL
 * BLOCKS:   PMD_X1
 * REGADDR:  0x9010
 * DESC:     Global PMD reset controls
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     CORE_DP_H_RSTB   PMD Core data path reset override valueOnly used for Speed Control bypass operationby default is zero too keep PMD in reset till enabled
 *     POR_H_RSTB       PMD Core power on resetby default is zero too keep PMD in reset till enabled
 *     PRAM_ABILITY     enable direct pram interface writes
 */
// acc_type = 7
#define BCMI_QTC_XGXS_PMD_X1_CTLr (0x00109010 | PHYMOD_REG_ACC_TSC_IBLK)
#define BCMI_QTC_XGXS_PMD_X1_CTLr_SIZE 4

/*******************************************************************************
 * CHIP:  BCMI_TSCE16_XGXS
 * REGISTER:  PMD_X4_CTL
 * BLOCKS:   PMD_X4
 * REGADDR:  0xc010
 * DESC:     PMD lane reset controls
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     LN_RX_DP_H_RSTB  RX PMD lane datapath reset override valueOnly used for Speed Control bypass operation
 *     LN_RX_H_RSTB     RX Reset all lane logic: data path and registers
 *     LN_TX_H_PWRDN    Lane power down, TX direction
 *     LN_RX_H_PWRDN    Lane power down, RX direction
 *     TX_OSR_MODE      TX OSR mode. In current implementation only OSR mode 1 is usedOnly used for Speed Control bypass operation
 *     TX_DISABLE       Set to squelch the transmit signal for laneOnly used for Speed Control bypass operation
 *     RX_OSR_MODE      RX OSR mode. In current implementation only OSR mode 1 is usedOnly used for Speed Control bypass operation
 *     RX_DME_EN        DME is enabled, for CL73 faster sample modeOnly used for Speed Control bypass operation
 *     LN_TX_DP_H_RSTB  TX PMD lane datapath reset override valueOnly used for Speed Control bypass operation
 *     LN_TX_H_RSTB     TX Reset all lane logic: data path and registers
 */
#define BCMI_TSCE16_XGXS_PMD_X4_CTLr (0x0000c010 | PHYMOD_REG_ACC_TSC_IBLK) 
#define BCMI_TSCE16_XGXS_PMD_X4_CTLr_SIZE 4


/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  MAIN_SETUP
 * BLOCKS:   MAIN
 * REGADDR:  0x9000
 * DESC:     main control register
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     MASTER_PORT_NUM  Port that will control PMD core logic and PLL reset:0 : port 0 is master1 : port 1 is master2 : port 2 is master3 : port 3 is master
 *     PLL_RESET_EN     Enable reseting PMD core logic and PLL by Speed control. The port that will reset the pll is selected by master_port_number
 *     SINGLE_PORT_MODE Indicates QTC is in single port mode.  Used by AN logic to determine whether to reset the PLL after AN completes.If set, when AN completes, the PLL will be reset to operate consistent with the resolved AN speed.If not set, the PLL will not change once AN completes.
 *     CL37_HIGH_VCO    Use 10G/12.5G VCO based speed for CL37 AN. By default 6.25G VCO based speed is used.This bit will also make the resolved speeds to use 10G/12.5G VCO when possible.00 - 6.25G VCO01 - 10G VCO10 - 12.5G VCO11 - reserved
 *     REFCLK_SEL       Specifies refclk frequency
 */
#define BCMI_QTC_XGXS_MAIN_SETUPr (0x00109000 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_MAIN_SETUPr_SIZE 4

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  DIG_TOP_USER_CTL0
 * BLOCKS:   DIG_COM
 * REGADDR:  0xd0f4
 * DEVAD:    1
 * DESC:     TOP_USER_CONTROL_0
 * RESETVAL: 0x271 (625)
 * ACCESS:   R/W
 * FIELDS:
 *     HEARTBEAT_COUNT_1US Heartbeat timer count in comclk cycles to create 1us heartbeat_1us period. It should be programmed to the nearest increment of 0.25Mhz value of the comclk frequency in Mhz.For example, for comclk of 125 Mhz, it should be programmed to 10'd500. For 156.25 Mhz comclk, it should be programmed to 10'd625 and similarly for any other comclk frequency.
 *     CORE_DP_S_RSTB   Active Low Core Level Datapath Soft Reset. If asserted by writingto 1'b0 will reset datapath logic of all the lanes. This soft resetis equivalent to the hard reset input pin core_dp_h_rstb. Assertion of this
 *     AFE_S_PLL_PWRDN  Active High PLL Power Down control.
 *     UC_ACTIVE        When set to 1'b1 then Hardware should wait for uC handshakes to wake up from datapath resetWhen set to 1'b0 then Hardware can internally assume that uc_ack_* = 1.
 */
#define BCMI_QTC_XGXS_DIG_TOP_USER_CTL0r (0x0001d0f4 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_DIG_TOP_USER_CTL0r_SIZE 4

/*
 * This structure should be used to declare and program PMD_X1_CTL.
 */
typedef union BCMI_QTC_XGXS_PMD_X1_CTLr_s {
	uint32_t v[1];
	uint32_t pmd_x1_ctl[1];
	uint32_t _pmd_x1_ctl;
} BCMI_QTC_XGXS_PMD_X1_CTLr_t;

#define BCMI_QTC_XGXS_PMD_X1_CTLr_CLR(r) (r).pmd_x1_ctl[0] = 0
#define BCMI_QTC_XGXS_PMD_X1_CTLr_SET(r,d) (r).pmd_x1_ctl[0] = d
#define BCMI_QTC_XGXS_PMD_X1_CTLr_GET(r) (r).pmd_x1_ctl[0]

/* Special lane values for broadcast and dual-lane multicast */
#define PHYMOD_TSC_IBLK_MCAST01    4
#define PHYMOD_TSC_IBLK_MCAST23    5
#define PHYMOD_TSC_IBLK_BCAST      6


typedef enum {
    QMOD16REFCLK156MHZ          = 0x00000000 ,  /*!< 156p25MHz */
    QMOD16REFCLK125MHZ          = 0x00000001 ,  /*!< 125MHz */
    QMOD16REFCLKCOUNT           = 0x00000002   /*!<  */
} qmod16_ref_clk_t;

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  TOP_USER_CTL0
 * BLOCKS:   DIGCOM
 * REGADDR:  0xd0f4
 * DEVAD:    1
 * DESC:     TOP_USER_CONTROL_0
 * RESETVAL: 0x271 (625)
 * ACCESS:   R/W
 * FIELDS:
 *     HEARTBEAT_COUNT_1US Heartbeat timer count in comclk cycles to create 1us heartbeat_1us period. It should be programmed to the nearest increment of 0.25Mhz value of the comclk frequency in Mhz.For example, for comclk of 125 Mhz, it should be programmed to 10'd500. For 156.25 Mhz comclk, it should be programmed to 10'd625 and similarly for any other comclk frequency.
 *     MASKDATA_BUS_ASSIGN This 2-bit register is used to assign the maskdata bus to any port .00: maskdata register is assigned to MDIO port01: maskdata register is assigned to PMI_HP port10: maskdata register is assigned to PMI_LP port11: maskdata register is not assigned to any port
 *     CORE_DP_S_RSTB   Active Low Core Level Datapath Soft Reset. If asserted by writingto 1'b0 will reset datapath logic of all the lanes. This soft resetis equivalent to the hard reset input pin core_dp_h_rstb.Minimum assertion time is 50 comclk cycles.
 *     AFE_S_PLL_PWRDN  Active High PLL Power Down control.Minimum assertion time is 50 comclk cycles.
 *     UC_ACTIVE        When set to 1'b1 then Hardware should wait for uC handshakes to wake up from datapath resetWhen set to 1'b0 then Hardware can internally assume that uc_ack_* = 1.
 */
#define BCMI_QTC_XGXS_TOP_USER_CTL0r (0x0001d0f4 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_TOP_USER_CTL0r_SIZE 4

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  MAIN_LN_SWP
 * BLOCKS:   MAIN
 * REGADDR:  0x9001
 * DESC:     Lane Swap Control Register
 * RESETVAL: 0xe4e4 (58596)
 * ACCESS:   R/W
 * FIELDS:
 *     LOG0_TO_PHY_TX_LNSWAP_SEL TX Lane0 logical to physical swap selectTX LOGICAL to PHYSICAL lane mapping controlIndicates for TX LOGICAL lane 3 which PHYSICAL lane to send data to
 *     LOG1_TO_PHY_TX_LNSWAP_SEL TX Lane1 logical to physical swap selectTX LOGICAL to PHYSICAL lane mapping controlIndicates for TX LOGICAL lane 3 which PHYSICAL lane to send data to
 *     LOG2_TO_PHY_TX_LNSWAP_SEL TX Lane2 logical to physical swap selectTX LOGICAL to PHYSICAL lane mapping controlIndicates for TX LOGICAL lane 3 which PHYSICAL lane to send data to
 *     LOG3_TO_PHY_TX_LNSWAP_SEL TX Lane3 logical to physical swap selectTX LOGICAL to PHYSICAL lane mapping controlIndicates for TX LOGICAL lane 3 which PHYSICAL lane to send data to
 *     LOG0_TO_PHY_RX_LNSWAP_SEL RX Lane0 logical to physical swap selectRX LOGICAL to PHYSICAL lane mapping controlIndicates for RX LOGICAL lane 3 which PHYSICAL lane to source data from
 *     LOG1_TO_PHY_RX_LNSWAP_SEL RX Lane1 logical to physical swap selectRX LOGICAL to PHYSICAL lane mapping controlIndicates for RX LOGICAL lane 3 which PHYSICAL lane to source data from
 *     LOG2_TO_PHY_RX_LNSWAP_SEL RX Lane2 logical to physical swap selectRX LOGICAL to PHYSICAL lane mapping controlIndicates for RX LOGICAL lane 3 which PHYSICAL lane to source data from
 *     LOG3_TO_PHY_RX_LNSWAP_SEL RX Lane3 logical to physical swap selectRX LOGICAL to PHYSICAL lane mapping controlIndicates for RX LOGICAL lane 3 which PHYSICAL lane to source data from
 */
#define BCMI_QTC_XGXS_MAIN_LN_SWPr (0x00109001 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_MAIN_LN_SWPr_SIZE 4


/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  CL37_RESTART
 * BLOCKS:   AN_X1_TIMERS
 * REGADDR:  0x9250
 * DESC:     CL37 AUTO-NEG RESTART TIMER
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     CL37_RESTART_TIMER_PERIOD Period/range is 10 mssingle copyCL37 auto-neg restart timer. Timer for the amout of time to send restart pages.
 */
#define BCMI_QTC_XGXS_CL37_RESTARTr (0x00109250 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_CL37_RESTARTr_SIZE 4


/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  CL37_RESTART
 * BLOCKS:   AN_X1_TIMERS
 * REGADDR:  0x9250
 * DESC:     CL37 AUTO-NEG RESTART TIMER
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     CL37_RESTART_TIMER_PERIOD Period/range is 10 mssingle copyCL37 auto-neg restart timer. Timer for the amout of time to send restart pages.
 */
#define BCMI_QTC_XGXS_CL37_RESTARTr (0x00109250 | PHYMOD_REG_ACC_TSC_IBLK)

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  CL37_ACK
 * BLOCKS:   AN_X1_TIMERS
 * REGADDR:  0x9251
 * DESC:     CL37 AUTO-NEG COMPLETE-ACKNOWLEDGE TIMER
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     CL37_ACK_TIMER_PERIOD Period/range is 10 mssingle copyCL37 auto-neg complete-acknowledge timer.  Timer for the amount of time to sent CL37 acknowledges.
 */
#define BCMI_QTC_XGXS_CL37_ACKr (0x00109251 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_CL37_ACKr_SIZE 4


/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  SGMII_CL37_TMR_TYPE
 * BLOCKS:   AN_X1_TIMERS
 * REGADDR:  0x9254
 * DESC:     CL37 SGMII TIMER
 * RESETVAL: 0x6b (107)
 * ACCESS:   R/W
 * FIELDS:
 *     SGMII_TIMER      This timer is used in CL37 for all SGMII time related functions such as link timer, send timer, ...
 */
#define BCMI_QTC_XGXS_SGMII_CL37_TMR_TYPEr (0x00109254 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_SGMII_CL37_TMR_TYPEr_SIZE 4

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  LNK_UP_TYPE
 * BLOCKS:   AN_X1_TIMERS
 * REGADDR:  0x9255
 * DESC:     CL37 Link up timer
 * RESETVAL: 0x6b (107)
 * ACCESS:   R/W
 * FIELDS:
 *     LINK_UP_TIMER_PERIOD Period/range is 100 tosingle copyCL37 link-up timer.  Timer for the amount of time for the link to come up (after page exchange is done).
 */
#define BCMI_QTC_XGXS_LNK_UP_TYPEr (0x00109255 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_LNK_UP_TYPEr_SIZE 4

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  IGNORE_LNK_TMR_TYPE
 * BLOCKS:   AN_X1_TIMERS
 * REGADDR:  0x9256
 * DESC:     CL37 Ignore Link timer
 * RESETVAL: 0x1 (1)
 * ACCESS:   R/W
 * FIELDS:
 *     IGNORE_LINK_TIMER_PERIOD Period/range is 100 tosingle copyThis is Not USED IN CL37 but good to have it for debugging purpose (if Link up takes time)
 */
#define BCMI_QTC_XGXS_IGNORE_LNK_TMR_TYPEr (0x00109256 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_IGNORE_LNK_TMR_TYPEr_SIZE 4

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  LNK_FAIL_INHBT_TMR_NOT_CL72_TYPE
 * BLOCKS:   AN_X1_TIMERS
 * REGADDR:  0x9257
 * DESC:     CL37 nCL72 timer
 * RESETVAL: 0xffff (65535)
 * ACCESS:   R/W
 * FIELDS:
 *     LINK_FAIL_INHIBIT_TIMER_NCL72_PERIOD Period/range is 100 tosingle copyTimer for qualifying a link_status==FAIL indication or a link_status==OK indication. This is Not USED IN CL37 but good to have it for debugging purpose (if Link up takes time)
 */
#define BCMI_QTC_XGXS_LNK_FAIL_INHBT_TMR_NOT_CL72_TYPEr (0x00109257 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_LNK_FAIL_INHBT_TMR_NOT_CL72_TYPEr_SIZE 4

/*******************************************************************************
 * CHIP:  BCMI_TSCE16_XGXS
 * REGISTER:  TLB_TX_TLB_TX_MISC_CFG
 * BLOCKS:   TLB_TX
 * REGADDR:  0xd0e3
 * DEVAD:    1
 * DESC:     TLB TX Misc. Control
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     TX_PMD_DP_INVERT TX PMD Datapath Invert Control.When Enabled by writing to 1'b1, it will invert all the datapath bits of the logical lane.Recommended for use in case P and N pads are swapped on the PCB board.
 *     TX_PCS_NATIVE_ANA_FRMT_EN TX PCS Interface Native Analog Format Enable.1 => TX PCS Interface is enabled in the Native Analog Format mode. TX PCS sends the over-sampled data in this mode which is sent directly to AFE.0 => Raw Data Mode where for every data request TX PCS will send 20 bits of valid data.
 *     TX_MUX_SEL_ORDER TX Data MUX Select Priority Order. When 1'b1 then priority of Pattern and PRBS generators are swapped w.r.t. CL72.0 => TX Data Mux select order from higher to lower priority is {rmt_lpbk, patt_gen, cl72_tx, prbs_gen, tx_pcs}.1 => TX Data Mux select order from higher to lower priority is {rmt_lpbk, prbs_gen, cl72_tx, patt_gen, tx_pcs}.
 */
#define BCMI_TSCE16_XGXS_TLB_TX_TLB_TX_MISC_CFGr (0x0001d0e3 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_TSCE16_XGXS_TLB_TX_TLB_TX_MISC_CFGr_SIZE 4

#define TLB_TX_TLB_TX_MISC_CFGr_TX_MUX_SEL_ORDERf_GET(r) (((r) >> 2) & 0x1)
#define TLB_TX_TLB_TX_MISC_CFGr_TX_MUX_SEL_ORDERf_SET(r,f) r=((r & ~((uint32_t)0x1 << 2)) | ((((uint32_t)f) & 0x1) << 2)) | (1 << (16 + 2))
#define TLB_TX_TLB_TX_MISC_CFGr_TX_PCS_NATIVE_ANA_FRMT_ENf_GET(r) (((r) >> 1) & 0x1)
#define TLB_TX_TLB_TX_MISC_CFGr_TX_PCS_NATIVE_ANA_FRMT_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 1)) | ((((uint32_t)f) & 0x1) << 1)) | (1 << (16 + 1))
#define TLB_TX_TLB_TX_MISC_CFGr_TX_PMD_DP_INVERTf_GET(r) ((r) & 0x1)
#define TLB_TX_TLB_TX_MISC_CFGr_TX_PMD_DP_INVERTf_SET(r,f) r=((r & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1)) | (0x1 << 16)


/*******************************************************************************
 * CHIP:  BCMI_TSCE16_XGXS
 * REGISTER:  TLB_RX_TLB_RX_MISC_CFG
 * BLOCKS:   TLB_RX
 * REGADDR:  0xd0d3
 * DEVAD:    1
 * DESC:     TLB RX Misc. Control
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     RX_PMD_DP_INVERT RX PMD Datapath Invert Control.When Enabled by writing to 1'b1, it will invert all the datapath bits of the logical lane.Recommended for use in case P and N pads are swapped on the PCB board.
 *     RX_AGGREGATOR_BYPASS_EN RX Aggregator Bypass EnableWhen Enabled by writing to 1'b1, DSC Data will bypass the RX Data aggregator block and sent directly to PCS along with a sync pulse which will indicate the (clk_cnt ==0).Recommended for use if low latency is desired and where data aggregation will be done in the PCS block based on the sync pulse.
 *     DBG_MASK_DIG_LPBK_EN Mask bit for dig_lpbk_en in the pmd_rx_lock equation. This is a debug register.1 => pmd_rx_lock will be forced to 1'b0 during digital loopback.0 => pmd_rx_lock will be forced to 1'b1 during digital loopback.
 */
#define BCMI_TSCE16_XGXS_TLB_RX_TLB_RX_MISC_CFGr (0x0001d0d3 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_TSCE16_XGXS_TLB_RX_TLB_RX_MISC_CFGr_SIZE 4

typedef struct phymod_polarity_s {
    uint32_t rx_polarity; /**< TX polarity bitmap */
    uint32_t tx_polarity; /**< RX polarity bitmap */
} phymod_polarity_t;

#define TLB_RX_TLB_RX_MISC_CFGr_DBG_MASK_DIG_LPBK_ENf_GET(r) (((r) >> 2) & 0x1)
#define TLB_RX_TLB_RX_MISC_CFGr_DBG_MASK_DIG_LPBK_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 2)) | ((((uint32_t)f) & 0x1) << 2)) | (1 << (16 + 2))
#define TLB_RX_TLB_RX_MISC_CFGr_RX_AGGREGATOR_BYPASS_ENf_GET(r) (((r) >> 1) & 0x1)
#define TLB_RX_TLB_RX_MISC_CFGr_RX_AGGREGATOR_BYPASS_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 1)) | ((((uint32_t)f) & 0x1) << 1)) | (1 << (16 + 1))
#define TLB_RX_TLB_RX_MISC_CFGr_RX_PMD_DP_INVERTf_GET(r) ((r) & 0x1)
#define TLB_RX_TLB_RX_MISC_CFGr_RX_PMD_DP_INVERTf_SET(r,f) r=((r & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1)) | (0x1 << 16)


/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  PMA_CTL
 * BLOCKS:   RX_X4_CONTROL0
 * REGADDR:  0xc162
 * DESC:     Rx PMA control register
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     RSTB_LANE        Low activer per lane reset for RXP
 *     RX_GBOX_AFRST_EN 
 *     OS_MODE          0: OS MODE 1 - divide by 11: OS MODE 2 - divide by 22: OS MODE 3 - divide by 33: OS MODE 3.3 - divide by 3.3.Bit repeating pattern is 3,3,3,4,3,3,4,3,3,44: OS MODE 4 - divide by 45: OS MODE 5 - divide by 56: OS MODE 8 - divide by 87: OS MODE 8.25 - divide by 8.25.Bit repeating pattern is 8,8,8,98: OS MODE 10 - divide by 10
 */
#define BCMI_QTC_XGXS_PMA_CTLr (0x0000c162 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_PMA_CTLr_SIZE 4

#define PMA_CTLr_OS_MODEf_GET(r) (((r) >> 3) & 0xf)
#define PMA_CTLr_OS_MODEf_SET(r,f) r=((r & ~((uint32_t)0xf << 3)) | ((((uint32_t)f) & 0xf) << 3)) | (15 << (16 + 3))
#define PMA_CTLr_RX_GBOX_AFRST_ENf_GET(r) (((r) >> 1) & 0x1)
#define PMA_CTLr_RX_GBOX_AFRST_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 1)) | ((((uint32_t)f) & 0x1) << 1)) | (1 << (16 + 1))
#define PMA_CTLr_RSTB_LANEf_GET(r) ((r) & 0x1)
#define PMA_CTLr_RSTB_LANEf_SET(r,f) r=((r & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1)) | (0x1 << 16)


/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  MISC
 * BLOCKS:   TX_X4_CONTROL0
 * REGADDR:  0xc140
 * DESC:     Misc register
 * RESETVAL: 0x3c (60)
 * ACCESS:   R/W
 * FIELDS:
 *     ENABLE_TX_LANE   Per lane enable to allow DVs from MAC to enter TXP
 *     RSTB_TX_LANE     Low active reset for txp lanes
 *     TX_FIFO_WATERMARK Per logical lane tx fifo watermark for tx gearboxN: Accumulate N words before sending the first data out.
 */
#define BCMI_QTC_XGXS_MISCr (0x0000c140 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_MISCr_SIZE 4

#define MISCr_TX_FIFO_WATERMARKf_SET(r,f) r = ((r & ~((uint32_t)0x3f << 2)) | ((((uint32_t)f) & 0x3f) << 2)) | (63 << (16 + 2))
#define MISCr_TX_FIFO_WATERMARKf_GET(r) (((r) >> 2) & 0x3f)
#define MISCr_RSTB_TX_LANEf_GET(r) (((r) >> 1) & 0x1)
#define MISCr_RSTB_TX_LANEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 1)) | ((((uint32_t)f) & 0x1) << 1)) | (1 << (16 + 1))
#define MISCr_ENABLE_TX_LANEf_GET(r) ((r) & 0x1)
#define MISCr_ENABLE_TX_LANEf_SET(r,f) r=((r & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1)) | (0x1 << 16)


/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  SC_X4_CTL
 * BLOCKS:   SC_X4_CONTROL
 * REGADDR:  0xc020
 * DESC:     SW speed change control
 * RESETVAL: 0xff (255)
 * ACCESS:   R/W
 * FIELDS:
 *     SW_SPEED         Speed to be setFor USXGMII SPEED_10p3125G_X1 is used
 *     SW_SPEED_CHANGE  Start SW speed change.HW will detect possedge of this field and start or restart the speed change logic.To restart speed change logic SW must write 0 to this bit and then write 1.
 */
#define BCMI_QTC_XGXS_SC_X4_CTLr (0x0000c020 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_SC_X4_CTLr_SIZE 4

#define SC_X4_CTLr_SW_SPEED_CHANGEf_GET(r) (((r) >> 8) & 0x1)
#define SC_X4_CTLr_SW_SPEED_CHANGEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 8)) | ((((uint32_t)f) & 0x1) << 8)) | (1 << (16 + 8))
#define SC_X4_CTLr_SW_SPEEDf_GET(r) ((r) & 0xff)
#define SC_X4_CTLr_SW_SPEEDf_SET(r,f) r=((r & ~((uint32_t)0xff)) | (((uint32_t)f) & 0xff)) | (0xff << 16)


/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  SC_X4_QSGMII_SPD
 * BLOCKS:   SC_X4_CONTROL
 * REGADDR:  0xc021
 * DESC:     SW subport speed control
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     SPEED_SP0        QSGMII or USXGMII Subport speed0   : 1G speed1   : 100M speed2   : 10M speed3   : QSGMII-Reserved , USXGMII - 2.5G speed
 *     SPEED_SP1        QSGMII or USXGMII Subport speed0   : 1G speed1   : 100M speed2   : 10M speed3   : QSGMII-Reserved , USXGMII - 2.5G speed
 *     SPEED_SP2        QSGMII or USXGMII Subport speed0   : 1G speed1   : 100M speed2   : 10M speed3   : QSGMII-Reserved , USXGMII - 2.5G speed
 *     SPEED_SP3        QSGMII or USXGMII Subport speed0   : 1G speed1   : 100M speed2   : 10M speed3   : QSGMII-Reserved , USXGMII - 2.5G speed
 */
#define SC_X4_QSGMII_SPDr (0x0000c021 | PHYMOD_REG_ACC_TSC_IBLK)

#define SC_X4_QSGMII_SPDr_SIZE 4


#define SC_X4_QSGMII_SPDr_CLR(r) r = 0
#define SC_X4_QSGMII_SPDr_SET(r,d) r = d
#define SC_X4_QSGMII_SPDr_GET(r) r

/*
 * These macros can be used to access individual fields.
 */
#define SC_X4_QSGMII_SPDr_SPEED_SP3f_GET(r) (((r) >> 6) & 0x3)
#define SC_X4_QSGMII_SPDr_SPEED_SP3f_SET(r,f) r=((r & ~((uint32_t)0x3 << 6)) | ((((uint32_t)f) & 0x3) << 6)) | (3 << (16 + 6))
#define SC_X4_QSGMII_SPDr_SPEED_SP2f_GET(r) (((r) >> 4) & 0x3)
#define SC_X4_QSGMII_SPDr_SPEED_SP2f_SET(r,f) r=((r & ~((uint32_t)0x3 << 4)) | ((((uint32_t)f) & 0x3) << 4)) | (3 << (16 + 4))
#define SC_X4_QSGMII_SPDr_SPEED_SP1f_GET(r) (((r) >> 2) & 0x3)
#define SC_X4_QSGMII_SPDr_SPEED_SP1f_SET(r,f) r=((r & ~((uint32_t)0x3 << 2)) | ((((uint32_t)f) & 0x3) << 2)) | (3 << (16 + 2))
#define SC_X4_QSGMII_SPDr_SPEED_SP0f_GET(r) ((r) & 0x3)
#define SC_X4_QSGMII_SPDr_SPEED_SP0f_SET(r,f) r=((r & ~((uint32_t)0x3)) | (((uint32_t)f) & 0x3)) | (0x3 << 16)


typedef enum {
    QMOD16_SPD_ZERO             = 0   ,  /*!< Illegal value (enum boundary)   */
    QMOD16_SPD_10_X1_SGMII            ,  /*!< 10Mb SGMII (serial)             */
    QMOD16_SPD_100_X1_SGMII           ,  /*!< 100Mb SGMII (serial)            */
    QMOD16_SPD_1000_X1_SGMII          ,  /*!< 1Gb SGMII (serial)              */
    QMOD16_SPD_2500_X1                ,  /*!< 2.5Gb  based on 1000BASE-X      */
    QMOD16_SPD_10_SGMII               ,  /*!< 10Mb SGMII (serial)             */
    QMOD16_SPD_100_SGMII              ,  /*!< 100Mb SGMII (serial)            */
    QMOD16_SPD_1000_SGMII             ,  /*!< 1Gb SGMII (serial)              */
    QMOD16_SPD_2500                   ,  /*!< 2.5Gb  based on 1000BASE-X      */
    QMOD16_SPD_5000                   ,  /*!< 5Gb  CL36                       */
    QMOD16_SPD_1000_XFI               ,  /*!< 1Gb                             */
    QMOD16_SPD_5000_XFI               ,  /*!< 5Gb  CL49                       */
    QMOD16_SPD_10000_XFI              ,  /*!< 10Gb serial XFI                 */
    QMOD16_SPD_10600_XFI_HG           ,  /*!< 10.5Gb serial XFI (HgSOLO)      */
    QMOD16_SPD_10000_HI               ,  /*!< 10Gb XAUI HiG                   */
    QMOD16_SPD_10000                  ,  /*!< 10Gb XAUI                       */
    QMOD16_SPD_12000_HI               ,  /*!< 12Gb XAUI HiG                   */
    QMOD16_SPD_13000                  ,  /*!< 13Gb XAUI                       */
    QMOD16_SPD_15000                  ,  /*!< 15Gb XAUI                       */
    QMOD16_SPD_16000                  ,  /*!< 16Gb XAUI                       */
    QMOD16_SPD_20000                  ,  /*!< 20Gb XAUI                       */
    QMOD16_SPD_20000_SCR              ,  /*!< 20Gb XAUI scrambled             */
    QMOD16_SPD_21000                  ,  /*!< 21Gb XAUI                       */
    QMOD16_SPD_25455                  ,  /*!< 25Gb XAUI  64/66 codec          */
    QMOD16_SPD_31500                  ,  /*!< 31.5Gb quad lane XAUI           */
    QMOD16_SPD_31500_MLD              ,  /*!< 31.5Gb quad lane MLD            */
    QMOD16_SPD_40G_X4                 ,  /*!< 40Gb quad lane XAUI             */
    QMOD16_SPD_42G_X4                 ,  /*!< 40Gb quad lane XAUI  HiG        */
    QMOD16_SPD_40G_XLAUI              ,  /*!< 40Gb quad lane  MLD             */
    QMOD16_SPD_42G_XLAUI              ,  /*!< 42Gb quad lane  MLD             */
    QMOD16_SPD_10000_X2               ,  /*!< 10Gb dual lane                  */
    QMOD16_SPD_10000_HI_DXGXS         ,  /*!< 10Gb dual lane XGXS HiG         */
    QMOD16_SPD_10000_DXGXS            ,  /*!< 10Gb dual lane XGXS             */
    QMOD16_SPD_10000_HI_DXGXS_SCR       ,  /*!< 10Gb dual lane,scrambled,HiG    */
    QMOD16_SPD_10000_DXGXS_SCR        ,  /*!< 10Gb dual lane scrambled        */
    QMOD16_SPD_10500_HI               ,  /*!< 10.5Gb XAUI  lane XGXS HiG      */
    QMOD16_SPD_10500_HI_DXGXS         ,  /*!< 10.5Gb  dual lane XGXS HiG      */
    QMOD16_SPD_12773_HI_DXGXS         ,  /*!< 12.73Gb dual lane XGXS HiG      */
    QMOD16_SPD_12773_DXGXS            ,  /*!< 12.73Gb dual lane XGXS          */
    QMOD16_SPD_15750_HI_DXGXS         ,  /*!< 15.75Gb scrambled dual lane HiG */
    QMOD16_SPD_20G_MLD_DXGXS          ,  /*!< 20Gb dual lane MLD              */
    QMOD16_SPD_21G_HI_MLD_DXGXS       ,  /*!< 20Gb dual lane HiG MLD          */
    QMOD16_SPD_20G_DXGXS              ,  /*!< 20Gb dual lane BRCM             */
    QMOD16_SPD_21G_HI_DXGXS           ,  /*!< 21.2Gb dual HiG(20+plldiv=70)   */
    QMOD16_SPD_100G_CR10              ,  /*!< 100G                            */
    QMOD16_SPD_107G_HG_CR10           ,  /*!< 107G                            */
    QMOD16_SPD_120G_CR12              ,  /*!< 120G                            */
    QMOD16_SPD_127G_HG_CR12           ,  /*!< 127G                            */
    QMOD16_SPD_4000                   ,  /*!< 4G QSGMII                       */
    QMOD16_SPD_10_X1_10               ,  /*!< 10Mb SGMII (serial) vco 10      */
    QMOD16_SPD_100_X1_10              ,  /*!< 100Mb SGMII (serial) vco 10     */
    QMOD16_SPD_1000_X1_10             ,  /*!< 1Gb SGMII (serial) vco 10       */
    QMOD16_SPD_10_X1_12P5             ,  /*!< 10Mb SGMII (serial) vco 12p5    */
    QMOD16_SPD_100_X1_12P5            ,  /*!< 100Mb SGMII (serial) vco 12p5   */
    QMOD16_SPD_1000_X1_12P5           ,  /*!< 1Gb SGMII (serial) vco 12p5     */
    QMOD16_SPD_2500_X1_12P5           ,  /*!< 2.5Gb  SGMII vco 12p5           */
    QMOD16_SPD_10G_X1_USXGMII         ,  /*!< 4x2.5G USXGMII vco 10p3125G     */
    QMOD16_SPD_2500_USXGMII           ,  /*!< 2.5 USGMII vco 10p3125          */
    QMOD16_SPD_ILLEGAL                  /*!< Illegal value (enum boundary)   */
} qmod16_spd_intfc_type;

#define digital_operationSpeeds_SPEED_10p3125G_X1          72
#define digital_operationSpeeds_SPEED_5G_X1_12p5           65
#define digital_operationSpeeds_SPEED_2p5G_X1_12p5         64
#define digital_operationSpeeds_SPEED_1000M_12p5           63
#define digital_operationSpeeds_SPEED_100M_12p5            62
#define digital_operationSpeeds_SPEED_10M_12p5             61
#define digital_operationSpeeds_SPEED_4G_X1_10             60
#define digital_operationSpeeds_SPEED_1000M_X1_10          59
#define digital_operationSpeeds_SPEED_100M_X1_10           58
#define digital_operationSpeeds_SPEED_10M_X1_10            57
#define digital_operationSpeeds_SPEED_2p5G_X1_10p3125      56
#define digital_operationSpeeds_SPEED_1000M_10p3125        55
#define digital_operationSpeeds_SPEED_100M_10p3125         54
#define digital_operationSpeeds_SPEED_10M_10p3125          53
#define digital_operationSpeeds_SPEED_10p5G_X4             50
#define digital_operationSpeeds_SPEED_5G_KR1               49
#define digital_operationSpeeds_SPEED_127G_X12             39
#define digital_operationSpeeds_SPEED_120G_X12             38
#define digital_operationSpeeds_SPEED_107G_X10             37
#define digital_operationSpeeds_SPEED_100G_CR10            36
#define digital_operationSpeeds_SPEED_42G_X4               35
#define digital_operationSpeeds_SPEED_40G_CR4              34
#define digital_operationSpeeds_SPEED_40G_KR4              33
#define digital_operationSpeeds_SPEED_21G_X2               32
#define digital_operationSpeeds_SPEED_20G_CR2              31
#define digital_operationSpeeds_SPEED_20G_KR2              30
#define digital_operationSpeeds_SPEED_10p6_X1              29
#define digital_operationSpeeds_SPEED_10G_KR1              28
#define digital_operationSpeeds_SPEED_40G_X4               27
#define digital_operationSpeeds_SPEED_20G_X2               26
#define digital_operationSpeeds_SPEED_20G_CX2              25
#define digital_operationSpeeds_SPEED_31p5G_KR4            24
#define digital_operationSpeeds_SPEED_31p5G_X4             23
#define digital_operationSpeeds_SPEED_15p75G_X2            22
#define digital_operationSpeeds_SPEED_25p45G_X4            21
#define digital_operationSpeeds_SPEED_12p7G_X2             20
#define digital_operationSpeeds_SPEED_21G_X4               19
#define digital_operationSpeeds_SPEED_10p5G_X2             18
#define digital_operationSpeeds_SPEED_20G_X4               17
#define digital_operationSpeeds_SPEED_10G_X2               16
#define digital_operationSpeeds_SPEED_10G_CX2              15
#define digital_operationSpeeds_SPEED_20G_CX4              14
#define digital_operationSpeeds_SPEED_16G_X4               13
#define digital_operationSpeeds_SPEED_15G_X4               12
#define digital_operationSpeeds_SPEED_13G_X4               11
#define digital_operationSpeeds_SPEED_10G_X4               10
#define digital_operationSpeeds_SPEED_10G_KX4              9
#define digital_operationSpeeds_SPEED_10G_CX4              8
#define digital_operationSpeeds_SPEED_5G_X1                7
#define digital_operationSpeeds_SPEED_2p5G_X1              6
#define digital_operationSpeeds_SPEED_1G_KX1               5
#define digital_operationSpeeds_SPEED_1G_CX1               4
#define digital_operationSpeeds_SPEED_1000M                3
#define digital_operationSpeeds_SPEED_100M                 2
#define digital_operationSpeeds_SPEED_10M                  1



#define PORTMOD_PORT_IS_AUTONEG_MODE_UPDATED(port_dynamic_state)          (port_dynamic_state & 0x2)
#define PORTMOD_PORT_AUTONEG_MODE_UPDATED_SET(port_dynamic_state)         (port_dynamic_state |= 0x2)
#define PORTMOD_PORT_AUTONEG_MODE_UPDATED_CLR(port_dynamic_state)         (port_dynamic_state &= 0xfffd) 

typedef enum phymod_an_mode_type_e {
    phymod_AN_MODE_NONE = 0,
    phymod_AN_MODE_CL73,
    phymod_AN_MODE_CL37,
    phymod_AN_MODE_CL73BAM,
    phymod_AN_MODE_CL37BAM,
    phymod_AN_MODE_HPAM,
    phymod_AN_MODE_SGMII,
    phymod_AN_MODE_CL37BAM_10P9375G_VCO,
    phymod_AN_MODE_CL37_SGMII,
    phymod_AN_MODE_CL73_MSA,
    phymod_AN_MODE_MSA,
    phymod_AN_MODE_Count
} phymod_an_mode_type_t;


typedef struct phymod_autoneg_control_s {
    phymod_an_mode_type_t an_mode;
    uint32_t num_lane_adv; /**< The number of lanes the autoneg advert */
    uint32_t flags; /**< see AN_F */
    uint32_t enable;
} phymod_autoneg_control_t;

#define PHYMOD_AN_F_ALLOW_PLL_CHANGE 0x1
#define PHYMOD_AN_F_SET_PRIOR_ENABLE 0x2
#define PHYMOD_AN_F_ALLOW_CL72_CONFIG_CHANGE 0x4
#define PHYMOD_AN_F_IGNORE_MEDIUM_CHECK 0x8
#define PHYMOD_AN_F_SET_CL73_PDET_KX_ENABLE 0x10
#define PHYMOD_AN_F_SET_CL73_PDET_KX4_ENABLE 0x20
#define PHYMOD_AN_F_SET_CL73_PDET_2P5G_KX_ENABLE 0x40
#define PHYMOD_AN_F_AUTO_MEDIUM_DETECT 0x80
#define PHYMOD_AN_F_FEC_RS272_CLR 0x100
#define PHYMOD_AN_F_SGMII_MASTER_MODE 0x200
#define PHYMOD_AN_F_FEC_RS272_REQ 0x400

#define PHYMOD_AN_F_ALLOW_PLL_CHANGE_SET(an) ((an)->flags |= PHYMOD_AN_F_ALLOW_PLL_CHANGE)
#define PHYMOD_AN_F_SET_PRIOR_ENABLE_SET(an) ((an)->flags |= PHYMOD_AN_F_SET_PRIOR_ENABLE)
#define PHYMOD_AN_F_ALLOW_CL72_CONFIG_CHANGE_SET(an) ((an)->flags |= PHYMOD_AN_F_ALLOW_CL72_CONFIG_CHANGE)
#define PHYMOD_AN_F_IGNORE_MEDIUM_CHECK_SET(an) ((an)->flags |= PHYMOD_AN_F_IGNORE_MEDIUM_CHECK)
#define PHYMOD_AN_F_SET_CL73_PDET_KX_ENABLE_SET(an) ((an)->flags |= PHYMOD_AN_F_SET_CL73_PDET_KX_ENABLE)
#define PHYMOD_AN_F_SET_CL73_PDET_KX4_ENABLE_SET(an) ((an)->flags |= PHYMOD_AN_F_SET_CL73_PDET_KX4_ENABLE)
#define PHYMOD_AN_F_SET_CL73_PDET_2P5G_KX_ENABLE_SET(an) ((an)->flags |= PHYMOD_AN_F_SET_CL73_PDET_2P5G_KX_ENABLE)
#define PHYMOD_AN_F_AUTO_MEDIUM_DETECT_SET(an) ((an)->flags |= PHYMOD_AN_F_AUTO_MEDIUM_DETECT)
#define PHYMOD_AN_F_FEC_RS272_CLR_SET(an) ((an)->flags |= PHYMOD_AN_F_FEC_RS272_CLR)
#define PHYMOD_AN_F_SGMII_MASTER_MODE_SET(an) ((an)->flags |= PHYMOD_AN_F_SGMII_MASTER_MODE)
#define PHYMOD_AN_F_FEC_RS272_REQ_SET(an) ((an)->flags |= PHYMOD_AN_F_FEC_RS272_REQ)

#define PHYMOD_AN_F_ALLOW_PLL_CHANGE_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_ALLOW_PLL_CHANGE)
#define PHYMOD_AN_F_SET_PRIOR_ENABLE_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_SET_PRIOR_ENABLE)
#define PHYMOD_AN_F_ALLOW_CL72_CONFIG_CHANGE_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_ALLOW_CL72_CONFIG_CHANGE)
#define PHYMOD_AN_F_IGNORE_MEDIUM_CHECK_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_IGNORE_MEDIUM_CHECK)
#define PHYMOD_AN_F_SET_CL73_PDET_KX_ENABLE_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_SET_CL73_PDET_KX_ENABLE)
#define PHYMOD_AN_F_SET_CL73_PDET_KX4_ENABLE_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_SET_CL73_PDET_KX4_ENABLE)
#define PHYMOD_AN_F_SET_CL73_PDET_2P5G_KX_ENABLE_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_SET_CL73_PDET_2P5G_KX_ENABLE)
#define PHYMOD_AN_F_AUTO_MEDIUM_DETECT_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_AUTO_MEDIUM_DETECT)
#define PHYMOD_AN_F_FEC_RS272_CLR_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_FEC_RS272_CLR)
#define PHYMOD_AN_F_SGMII_MASTER_MODE_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_SGMII_MASTER_MODE)
#define PHYMOD_AN_F_FEC_RS272_REQ_CLR(an) ((an)->flags &= ~PHYMOD_AN_F_FEC_RS272_REQ)

#define PHYMOD_AN_F_ALLOW_PLL_CHANGE_GET(an) ((an)->flags & PHYMOD_AN_F_ALLOW_PLL_CHANGE ? 1 : 0)
#define PHYMOD_AN_F_SET_PRIOR_ENABLE_GET(an) ((an)->flags & PHYMOD_AN_F_SET_PRIOR_ENABLE ? 1 : 0)
#define PHYMOD_AN_F_ALLOW_CL72_CONFIG_CHANGE_GET(an) ((an)->flags & PHYMOD_AN_F_ALLOW_CL72_CONFIG_CHANGE ? 1 : 0)
#define PHYMOD_AN_F_IGNORE_MEDIUM_CHECK_GET(an) ((an)->flags & PHYMOD_AN_F_IGNORE_MEDIUM_CHECK ? 1 : 0)
#define PHYMOD_AN_F_SET_CL73_PDET_KX_ENABLE_GET(an) ((an)->flags & PHYMOD_AN_F_SET_CL73_PDET_KX_ENABLE ? 1 : 0)
#define PHYMOD_AN_F_SET_CL73_PDET_KX4_ENABLE_GET(an) ((an)->flags & PHYMOD_AN_F_SET_CL73_PDET_KX4_ENABLE ? 1 : 0)
#define PHYMOD_AN_F_SET_CL73_PDET_2P5G_KX_ENABLE_GET(an) ((an)->flags & PHYMOD_AN_F_SET_CL73_PDET_2P5G_KX_ENABLE ? 1 : 0)
#define PHYMOD_AN_F_AUTO_MEDIUM_DETECT_GET(an) ((an)->flags & PHYMOD_AN_F_AUTO_MEDIUM_DETECT ? 1 : 0)
#define PHYMOD_AN_F_FEC_RS272_CLR_GET(an) ((an)->flags & PHYMOD_AN_F_FEC_RS272_CLR ? 1 : 0)
#define PHYMOD_AN_F_SGMII_MASTER_MODE_GET(an) ((an)->flags & PHYMOD_AN_F_SGMII_MASTER_MODE ? 1 : 0)
#define PHYMOD_AN_F_FEC_RS272_REQ_GET(an) ((an)->flags & PHYMOD_AN_F_FEC_RS272_REQ ? 1 : 0)

typedef enum {
    QMOD16_AN_MODE_NONE = 0,
    QMOD16_AN_MODE_CL37 = 1,
    QMOD16_AN_MODE_CL37BAM = 2,
    QMOD16_AN_MODE_SGMII = 4,
    QMOD16_AN_MODE_TYPE_COUNT
}qmod16_an_mode_type_t;

typedef enum {
    QMOD_AN_PROPERTY_ENABLE_NONE = 0x00000000 ,  /*!<  */
    QMOD_AN_PROPERTY_ENABLE_SGMII_MASTER_MODE = 0x00000001 ,  /*!<  */
    QMOD_AN_PROPERTY_ENABLE_AN_PD_TO_CL37 = 0x00000002 ,  /*!<  */
    QMOD_AN_PROPERTY_ENABLE_SGMII_TO_CL37_AUTO = 0x00000004 ,  /*!<  */
    QMOD_AN_PROPERTY_ENABLE_CL37_BAM_to_SGMII_AUTO = 0x00000008 ,  /*!<  */
    QMOD_AN_PROPERTY_ENABLE_HPAM_TO_CL73_AUTO = 0x00000010 ,  /*!<  */
    QMOD_AN_PROPERTY_ENABLE_CL73_BAM_TO_HPAM_AUTO = 0x00000020 ,  /*!<  */
    QMOD_AN_PROPERTY_ENABLE_ILLEGAL = 0x00000040   /*!<  */
  } an_property_enable;

typedef struct qmod16_an_control_s {
    qmod16_an_mode_type_t an_type; 
    uint16_t num_lane_adv; 
    uint16_t enable;
    uint16_t pd_kx_en;
    uint16_t pd_kx4_en;
    an_property_enable  an_property_type;
} qmod16_an_control_t;

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  AN_X4_ABI_SP0_LOC_DEV_CL37_BASE_ABIL
 * BLOCKS:   AN_X4_ABILITIES_SP0
 * REGADDR:  0xc201
 * DESC:     CL37 BASE PAGE ABILITIES
 * RESETVAL: 0x2 (2)
 * ACCESS:   R/W
 * FIELDS:
 *     SGMII_SPEED      SGMII or USXGMII speed:3'b111, 3'b110, 3'b101 = Reserved;3'b100 = 2500 Mb/s (USXGMII only);3'b011 = Reserved;3'b010 = 1000 Mb/s; 3'b001 = 100 Mb/s; 3'b000 = 10 Mb/s.
 *     SGMII_FULL_DUPLEX USXGMII/SGMII full duplex is supported.
 *     CL37_FULL_DUPLEX 
 *     CL37_HALF_DUPLEX 
 *     CL37_PAUSE       Pause Ability[7:6]00 - No PAUSE ability10 - Asymmetric-toward-link-partner PAUSE ability01 - Symmetric PAUSE ability11 - Both symmetric and asymmetric-toward-local-device PAUSE ability
 *     CL37_NEXT_PAGE   
 *     SGMII_MASTER_MODE Set the current device as SGMII master for AN, only for debug.Not supported for USXGMII.
 *     LINK_STATUS      link status to pass to RD, USXGMII only.
 *     CL37_AN_RESTART_RESET_DISABLE Disable full pipeline reset when CL37 AN is restarted by remote partner.Do not set this bit for BAM modes.
 *     CL37_SW_RESTART_RESET_DISABLE Disable full pipeline reset when CL37 AN is restarted by SW.Do not set this bit when enabling AN for the first time. Only set it before restarting AN.Do not set this bit for BAM modes.
 *     CL37_AN_DISABLE_RESET_DISABLE Disable full pipeline reset when AN is being disabled by SW.Please set this bit for IEEE37 and SGMII AN. Do not set this bit for BAM modes.
 *     EEE_CLOCK_STOP_CAPABILITY EEE clock stop capability to pass to RD, USXGMII only.
 *     EEE_CAPABILITY   EEE capability to pass to RD, USXGMII only.
 */
#define BCMI_QTC_XGXS_AN_X4_ABI_SP0_LOC_DEV_CL37_BASE_ABILr (0x0000c201 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_AN_X4_ABI_SP0_LOC_DEV_CL37_BASE_ABILr_SIZE 4

#define AN_X4_ABI_SP0_LOC_DEV_CL37_BASE_ABILr_CLR(r)  r = 0
#define AN_X4_ABI_SP0_LOC_DEV_CL37_BASE_ABILr_SET(r,d) r = d
#define AN_X4_ABI_SP0_LOC_DEV_CL37_BASE_ABILr_GET(r)  r
  
#define AN_X4_ABI_SP0_LOC_DEV_CL37_BASE_ABILr_CL37_FULL_DUPLEXf_SET(r,f) r=((r & ~((uint32_t)0x1 << 4)) | ((((uint32_t)f) & 0x1) << 4)) | (1 << (16 + 4))



#define AN_X4_ABI_SP0_ENSr_CLR(r) r = 0
#define AN_X4_ABI_SP0_ENSr_SET(r,d) r = d
#define AN_X4_ABI_SP0_ENSr_GET(r) r
#define AN_X4_ABI_SP0_ENSr_CL37_AN_RESTARTf_SET(r,f) r=((r & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1)) | (0x1 << 16)
#define AN_X4_ABI_SP0_ENSr_CL37_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 3)) | ((((uint32_t)f) & 0x1) << 3)) | (1 << (16 + 3))
#define AN_X4_ABI_SP0_ENSr_CL37_SGMII_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 4)) | ((((uint32_t)f) & 0x1) << 4)) | (1 << (16 + 4))
#define AN_X4_ABI_SP0_ENSr_CL37_BAM_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 5)) | ((((uint32_t)f) & 0x1) << 5)) | (1 << (16 + 5))
#define AN_X4_ABI_SP0_ENSr_QSGMII_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 7)) | ((((uint32_t)f) & 0x1) << 7)) | (1 << (16 + 7))
#define AN_X4_ABI_SP0_ENSr_USXGMII_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 8)) | ((((uint32_t)f) & 0x1) << 8)) | (1 << (16 + 8))

#define AN_X4_ABI_SP1_ENSr_CLR(r) r = 0
#define AN_X4_ABI_SP1_ENSr_CL37_AN_RESTARTf_SET(r,f) r=((r & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1)) | (0x1 << 16)
#define AN_X4_ABI_SP1_ENSr_CL37_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 3)) | ((((uint32_t)f) & 0x1) << 3)) | (1 << (16 + 3))
#define AN_X4_ABI_SP1_ENSr_CL37_SGMII_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 4)) | ((((uint32_t)f) & 0x1) << 4)) | (1 << (16 + 4))
#define AN_X4_ABI_SP1_ENSr_CL37_BAM_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 5)) | ((((uint32_t)f) & 0x1) << 5)) | (1 << (16 + 5))
#define AN_X4_ABI_SP1_ENSr_QSGMII_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 7)) | ((((uint32_t)f) & 0x1) << 7)) | (1 << (16 + 7))
#define AN_X4_ABI_SP1_ENSr_USXGMII_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 8)) | ((((uint32_t)f) & 0x1) << 8)) | (1 << (16 + 8))

#define AN_X4_ABI_SP2_ENSr_CLR(r) r = 0
#define AN_X4_ABI_SP2_ENSr_LINK_STATUS_ECHOf_GET(r) (((r) >> 13) & 0x1)
#define AN_X4_ABI_SP2_ENSr_LINK_STATUS_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 13)) | ((((uint32_t)f) & 0x1) << 13)) | (1 << (16 + 13))
#define AN_X4_ABI_SP2_ENSr_DUPLEX_MODE_ECHOf_GET(r) (((r) >> 12) & 0x1)
#define AN_X4_ABI_SP2_ENSr_DUPLEX_MODE_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 12)) | ((((uint32_t)f) & 0x1) << 12)) | (1 << (16 + 12))
#define AN_X4_ABI_SP2_ENSr_SPEED_ECHOf_GET(r) (((r) >> 11) & 0x1)
#define AN_X4_ABI_SP2_ENSr_SPEED_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 11)) | ((((uint32_t)f) & 0x1) << 11)) | (1 << (16 + 11))
#define AN_X4_ABI_SP2_ENSr_EEE_CAPABILITY_ECHOf_GET(r) (((r) >> 10) & 0x1)
#define AN_X4_ABI_SP2_ENSr_EEE_CAPABILITY_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 10)) | ((((uint32_t)f) & 0x1) << 10)) | (1 << (16 + 10))
#define AN_X4_ABI_SP2_ENSr_EEE_CLOCK_CAPABILITY_ECHOf_GET(r) (((r) >> 9) & 0x1)
#define AN_X4_ABI_SP2_ENSr_EEE_CLOCK_CAPABILITY_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 9)) | ((((uint32_t)f) & 0x1) << 9)) | (1 << (16 + 9))
#define AN_X4_ABI_SP2_ENSr_USXGMII_ENf_GET(r) (((r) >> 8) & 0x1)
#define AN_X4_ABI_SP2_ENSr_USXGMII_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 8)) | ((((uint32_t)f) & 0x1) << 8)) | (1 << (16 + 8))
#define AN_X4_ABI_SP2_ENSr_QSGMII_ENf_GET(r) (((r) >> 7) & 0x1)
#define AN_X4_ABI_SP2_ENSr_QSGMII_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 7)) | ((((uint32_t)f) & 0x1) << 7)) | (1 << (16 + 7))
#define AN_X4_ABI_SP2_ENSr_DISABLE_REMOTE_FAULT_REPORTINGf_GET(r) (((r) >> 6) & 0x1)
#define AN_X4_ABI_SP2_ENSr_DISABLE_REMOTE_FAULT_REPORTINGf_SET(r,f) r=((r & ~((uint32_t)0x1 << 6)) | ((((uint32_t)f) & 0x1) << 6)) | (1 << (16 + 6))
#define AN_X4_ABI_SP2_ENSr_CL37_BAM_ENABLEf_GET(r) (((r) >> 5) & 0x1)
#define AN_X4_ABI_SP2_ENSr_CL37_BAM_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 5)) | ((((uint32_t)f) & 0x1) << 5)) | (1 << (16 + 5))
#define AN_X4_ABI_SP2_ENSr_CL37_SGMII_ENABLEf_GET(r) (((r) >> 4) & 0x1)
#define AN_X4_ABI_SP2_ENSr_CL37_SGMII_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 4)) | ((((uint32_t)f) & 0x1) << 4)) | (1 << (16 + 4))
#define AN_X4_ABI_SP2_ENSr_CL37_ENABLEf_GET(r) (((r) >> 3) & 0x1)
#define AN_X4_ABI_SP2_ENSr_CL37_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 3)) | ((((uint32_t)f) & 0x1) << 3)) | (1 << (16 + 3))
#define AN_X4_ABI_SP2_ENSr_CL37_BAM_TO_SGMII_AUTO_ENABLEf_GET(r) (((r) >> 2) & 0x1)
#define AN_X4_ABI_SP2_ENSr_CL37_BAM_TO_SGMII_AUTO_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 2)) | ((((uint32_t)f) & 0x1) << 2)) | (1 << (16 + 2))
#define AN_X4_ABI_SP2_ENSr_SGMII_TO_CL37_AUTO_ENABLEf_GET(r) (((r) >> 1) & 0x1)
#define AN_X4_ABI_SP2_ENSr_SGMII_TO_CL37_AUTO_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 1)) | ((((uint32_t)f) & 0x1) << 1)) | (1 << (16 + 1))
#define AN_X4_ABI_SP2_ENSr_CL37_AN_RESTARTf_GET(r) ((r) & 0x1)
#define AN_X4_ABI_SP2_ENSr_CL37_AN_RESTARTf_SET(r,f) r=((r & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1)) | (0x1 << 16)


#define AN_X4_ABI_SP3_ENSr_CLR(r) r = 0
#define AN_X4_ABI_SP3_ENSr_SET(r,d) r = d
#define AN_X4_ABI_SP3_ENSr_GET(r) r

/*
 * These macros can be used to access individual fields.
 */
#define AN_X4_ABI_SP3_ENSr_LINK_STATUS_ECHOf_GET(r) (((r) >> 13) & 0x1)
#define AN_X4_ABI_SP3_ENSr_LINK_STATUS_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 13)) | ((((uint32_t)f) & 0x1) << 13)) | (1 << (16 + 13))
#define AN_X4_ABI_SP3_ENSr_DUPLEX_MODE_ECHOf_GET(r) (((r) >> 12) & 0x1)
#define AN_X4_ABI_SP3_ENSr_DUPLEX_MODE_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 12)) | ((((uint32_t)f) & 0x1) << 12)) | (1 << (16 + 12))
#define AN_X4_ABI_SP3_ENSr_SPEED_ECHOf_GET(r) (((r) >> 11) & 0x1)
#define AN_X4_ABI_SP3_ENSr_SPEED_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 11)) | ((((uint32_t)f) & 0x1) << 11)) | (1 << (16 + 11))
#define AN_X4_ABI_SP3_ENSr_EEE_CAPABILITY_ECHOf_GET(r) (((r) >> 10) & 0x1)
#define AN_X4_ABI_SP3_ENSr_EEE_CAPABILITY_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 10)) | ((((uint32_t)f) & 0x1) << 10)) | (1 << (16 + 10))
#define AN_X4_ABI_SP3_ENSr_EEE_CLOCK_CAPABILITY_ECHOf_GET(r) (((r) >> 9) & 0x1)
#define AN_X4_ABI_SP3_ENSr_EEE_CLOCK_CAPABILITY_ECHOf_SET(r,f) r=((r & ~((uint32_t)0x1 << 9)) | ((((uint32_t)f) & 0x1) << 9)) | (1 << (16 + 9))
#define AN_X4_ABI_SP3_ENSr_USXGMII_ENf_GET(r) (((r) >> 8) & 0x1)
#define AN_X4_ABI_SP3_ENSr_USXGMII_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 8)) | ((((uint32_t)f) & 0x1) << 8)) | (1 << (16 + 8))
#define AN_X4_ABI_SP3_ENSr_QSGMII_ENf_GET(r) (((r) >> 7) & 0x1)
#define AN_X4_ABI_SP3_ENSr_QSGMII_ENf_SET(r,f) r=((r & ~((uint32_t)0x1 << 7)) | ((((uint32_t)f) & 0x1) << 7)) | (1 << (16 + 7))
#define AN_X4_ABI_SP3_ENSr_DISABLE_REMOTE_FAULT_REPORTINGf_GET(r) (((r) >> 6) & 0x1)
#define AN_X4_ABI_SP3_ENSr_DISABLE_REMOTE_FAULT_REPORTINGf_SET(r,f) r=((r & ~((uint32_t)0x1 << 6)) | ((((uint32_t)f) & 0x1) << 6)) | (1 << (16 + 6))
#define AN_X4_ABI_SP3_ENSr_CL37_BAM_ENABLEf_GET(r) (((r) >> 5) & 0x1)
#define AN_X4_ABI_SP3_ENSr_CL37_BAM_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 5)) | ((((uint32_t)f) & 0x1) << 5)) | (1 << (16 + 5))
#define AN_X4_ABI_SP3_ENSr_CL37_SGMII_ENABLEf_GET(r) (((r) >> 4) & 0x1)
#define AN_X4_ABI_SP3_ENSr_CL37_SGMII_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 4)) | ((((uint32_t)f) & 0x1) << 4)) | (1 << (16 + 4))
#define AN_X4_ABI_SP3_ENSr_CL37_ENABLEf_GET(r) (((r) >> 3) & 0x1)
#define AN_X4_ABI_SP3_ENSr_CL37_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 3)) | ((((uint32_t)f) & 0x1) << 3)) | (1 << (16 + 3))
#define AN_X4_ABI_SP3_ENSr_CL37_BAM_TO_SGMII_AUTO_ENABLEf_GET(r) (((r) >> 2) & 0x1)
#define AN_X4_ABI_SP3_ENSr_CL37_BAM_TO_SGMII_AUTO_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 2)) | ((((uint32_t)f) & 0x1) << 2)) | (1 << (16 + 2))
#define AN_X4_ABI_SP3_ENSr_SGMII_TO_CL37_AUTO_ENABLEf_GET(r) (((r) >> 1) & 0x1)
#define AN_X4_ABI_SP3_ENSr_SGMII_TO_CL37_AUTO_ENABLEf_SET(r,f) r=((r & ~((uint32_t)0x1 << 1)) | ((((uint32_t)f) & 0x1) << 1)) | (1 << (16 + 1))
#define AN_X4_ABI_SP3_ENSr_CL37_AN_RESTARTf_GET(r) ((r) & 0x1)
#define AN_X4_ABI_SP3_ENSr_CL37_AN_RESTARTf_SET(r,f) r=((r & ~((uint32_t)0x1)) | (((uint32_t)f) & 0x1)) | (0x1 << 16)

/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  AN_X4_ABI_SP0_ENS
 * BLOCKS:   AN_X4_ABILITIES_SP0
 * REGADDR:  0xc200
 * DESC:     AN ENABLES
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     CL37_AN_RESTART  CL37 Auto-Negotiation RestartA transition from zero to one on this bit will restart CL37 AN
 *     SGMII_TO_CL37_AUTO_ENABLE SGMII to IEEE CL37 Auto-Detect EnableWhen doing Cisco SGMII auto-negotiation, if the other device is doing IEEECL37 auto-negotiation, then restart auto-negotiation in IEEE CL37 mode.
 *     CL37_BAM_TO_SGMII_AUTO_ENABLE Broadcom to SGMII Auto-Detect EnableWhen doing Broadcom CL37 auto-negotioation, if the other device is doing CiscoSGMII auto-negotiation, then restart auto-negotiation in Cisco SGMII mode.
 *     CL37_ENABLE      IEEE CL37 Auto-Negotiation Mode Enable
 *     CL37_SGMII_ENABLE Cisco SGMII Auto-Negotiation Mode Enable
 *     CL37_BAM_ENABLE  Broadcom CL37 Auto-Negotiation Mode Enable
 *     DISABLE_REMOTE_FAULT_REPORTING 1 = disable automatic reporting of remote faults, such as auto-negotiation error0 = report remote fault status to link partner via auto-negotiation when fiber mode is selected. (SGMII does not support remote faults)
 *     QSGMII_EN        Sets the AN logic in QSGMII Mode and enables all 4 AN for subport independently. Default is SGMII Mode (i.e. Subport 0 AN is running)
 *     USXGMII_EN       Sets the AN logic in USXGMII Mode.
 *     EEE_CLOCK_CAPABILITY_ECHO Enable base page echo. USXGMII only.
 *     EEE_CAPABILITY_ECHO Enable base page echo. USXGMII only.
 *     SPEED_ECHO       Enable base page echo. USXGMII only.
 *     DUPLEX_MODE_ECHO Enable base page echo. USXGMII only.
 *     LINK_STATUS_ECHO Enable base page echo. USXGMII only.
 */
#define BCMI_QTC_XGXS_AN_X4_ABI_SP0_ENSr (0x0000c200 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_AN_X4_ABI_SP0_ENSr_SIZE 4


/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  AN_X4_ABI_SP1_ENS
 * BLOCKS:   AN_X4_ABILITIES_SP1
 * REGADDR:  0xc210
 * DESC:     AN ENABLES
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     CL37_AN_RESTART  CL37 Auto-Negotiation RestartA transition from zero to one on this bit will restart CL37 AN
 *     SGMII_TO_CL37_AUTO_ENABLE SGMII to IEEE CL37 Auto-Detect EnableWhen doing Cisco SGMII auto-negotiation, if the other device is doing IEEECL37 auto-negotiation, then restart auto-negotiation in IEEE CL37 mode.
 *     CL37_BAM_TO_SGMII_AUTO_ENABLE Broadcom to SGMII Auto-Detect EnableWhen doing Broadcom CL37 auto-negotioation, if the other device is doing CiscoSGMII auto-negotiation, then restart auto-negotiation in Cisco SGMII mode.
 *     CL37_ENABLE      IEEE CL37 Auto-Negotiation Mode Enable
 *     CL37_SGMII_ENABLE Cisco SGMII Auto-Negotiation Mode Enable
 *     CL37_BAM_ENABLE  Broadcom CL37 Auto-Negotiation Mode Enable
 *     DISABLE_REMOTE_FAULT_REPORTING 1 = disable automatic reporting of remote faults, such as auto-negotiation error0 = report remote fault status to link partner via auto-negotiation when fiber mode is selected. (SGMII does not support remote faults)
 *     QSGMII_EN        Sets the AN logic in QSGMII Mode and enables all 4 AN for subport independently. Default is SGMII Mode (i.e. Subport 0 AN is running)
 *     USXGMII_EN       Sets the AN logic in USXGMII Mode.
 *     EEE_CLOCK_CAPABILITY_ECHO Enable base page echo. USXGMII only.
 *     EEE_CAPABILITY_ECHO Enable base page echo. USXGMII only.
 *     SPEED_ECHO       Enable base page echo. USXGMII only.
 *     DUPLEX_MODE_ECHO Enable base page echo. USXGMII only.
 *     LINK_STATUS_ECHO Enable base page echo. USXGMII only.
 */
#define BCMI_QTC_XGXS_AN_X4_ABI_SP1_ENSr (0x0000c210 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_AN_X4_ABI_SP1_ENSr_SIZE 4



/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  AN_X4_ABI_SP2_ENS
 * BLOCKS:   AN_X4_ABILITIES_SP2
 * REGADDR:  0xc220
 * DESC:     AN ENABLES
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     CL37_AN_RESTART  CL37 Auto-Negotiation RestartA transition from zero to one on this bit will restart CL37 AN
 *     SGMII_TO_CL37_AUTO_ENABLE SGMII to IEEE CL37 Auto-Detect EnableWhen doing Cisco SGMII auto-negotiation, if the other device is doing IEEECL37 auto-negotiation, then restart auto-negotiation in IEEE CL37 mode.
 *     CL37_BAM_TO_SGMII_AUTO_ENABLE Broadcom to SGMII Auto-Detect EnableWhen doing Broadcom CL37 auto-negotioation, if the other device is doing CiscoSGMII auto-negotiation, then restart auto-negotiation in Cisco SGMII mode.
 *     CL37_ENABLE      IEEE CL37 Auto-Negotiation Mode Enable
 *     CL37_SGMII_ENABLE Cisco SGMII Auto-Negotiation Mode Enable
 *     CL37_BAM_ENABLE  Broadcom CL37 Auto-Negotiation Mode Enable
 *     DISABLE_REMOTE_FAULT_REPORTING 1 = disable automatic reporting of remote faults, such as auto-negotiation error0 = report remote fault status to link partner via auto-negotiation when fiber mode is selected. (SGMII does not support remote faults)
 *     QSGMII_EN        Sets the AN logic in QSGMII Mode and enables all 4 AN for subport independently. Default is SGMII Mode (i.e. Subport 0 AN is running)
 *     USXGMII_EN       Sets the AN logic in USXGMII Mode.
 *     EEE_CLOCK_CAPABILITY_ECHO Enable base page echo. USXGMII only.
 *     EEE_CAPABILITY_ECHO Enable base page echo. USXGMII only.
 *     SPEED_ECHO       Enable base page echo. USXGMII only.
 *     DUPLEX_MODE_ECHO Enable base page echo. USXGMII only.
 *     LINK_STATUS_ECHO Enable base page echo. USXGMII only.
 */
#define BCMI_QTC_XGXS_AN_X4_ABI_SP2_ENSr (0x0000c220 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_AN_X4_ABI_SP2_ENSr_SIZE 4



/*******************************************************************************
 * CHIP:  BCMI_QTC_XGXS
 * REGISTER:  AN_X4_ABI_SP3_ENS
 * BLOCKS:   AN_X4_ABILITIES_SP3
 * REGADDR:  0xc230
 * DESC:     AN ENABLES
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     CL37_AN_RESTART  CL37 Auto-Negotiation RestartA transition from zero to one on this bit will restart CL37 AN
 *     SGMII_TO_CL37_AUTO_ENABLE SGMII to IEEE CL37 Auto-Detect EnableWhen doing Cisco SGMII auto-negotiation, if the other device is doing IEEECL37 auto-negotiation, then restart auto-negotiation in IEEE CL37 mode.
 *     CL37_BAM_TO_SGMII_AUTO_ENABLE Broadcom to SGMII Auto-Detect EnableWhen doing Broadcom CL37 auto-negotioation, if the other device is doing CiscoSGMII auto-negotiation, then restart auto-negotiation in Cisco SGMII mode.
 *     CL37_ENABLE      IEEE CL37 Auto-Negotiation Mode Enable
 *     CL37_SGMII_ENABLE Cisco SGMII Auto-Negotiation Mode Enable
 *     CL37_BAM_ENABLE  Broadcom CL37 Auto-Negotiation Mode Enable
 *     DISABLE_REMOTE_FAULT_REPORTING 1 = disable automatic reporting of remote faults, such as auto-negotiation error0 = report remote fault status to link partner via auto-negotiation when fiber mode is selected. (SGMII does not support remote faults)
 *     QSGMII_EN        Sets the AN logic in QSGMII Mode and enables all 4 AN for subport independently. Default is SGMII Mode (i.e. Subport 0 AN is running)
 *     USXGMII_EN       Sets the AN logic in USXGMII Mode.
 *     EEE_CLOCK_CAPABILITY_ECHO Enable base page echo. USXGMII only.
 *     EEE_CAPABILITY_ECHO Enable base page echo. USXGMII only.
 *     SPEED_ECHO       Enable base page echo. USXGMII only.
 *     DUPLEX_MODE_ECHO Enable base page echo. USXGMII only.
 *     LINK_STATUS_ECHO Enable base page echo. USXGMII only.
 */
#define BCMI_QTC_XGXS_AN_X4_ABI_SP3_ENSr (0x0000c230 | PHYMOD_REG_ACC_TSC_IBLK)

#define BCMI_QTC_XGXS_AN_X4_ABI_SP3_ENSr_SIZE 4

/*******************************************************************************
 * CHIP:  BCMI_TSCE16_XGXS
 * REGISTER:  RXTXCOM_OSR_MODE_CTL
 * BLOCKS:   RXTXCOM_CKRST_CTRL
 * REGADDR:  0xd080
 * DEVAD:    1
 * DESC:     OSR_MODE_CONTROL
 * RESETVAL: 0x0 (0)
 * ACCESS:   R/W
 * FIELDS:
 *     OSR_MODE_FRC_VAL oversample (OS) mode Decoding of this register is as follows.OSX1          4'd0OSX2          4'd1OSX3          4'd2OSX3P3        4'd3OSX4          4'd4OSX5          4'd5OSX7P5        4'd6OSX8          4'd7OSX8P25       4'd8OSX10         4'd9
 *     OSR_MODE_FRC     oversample (OS) mode force. Setting this bit will allow the register value to be used for OS mode.Othersise, the pin input values are used for OS mode
 */
#define RXTXCOM_OSR_MODE_CTLr (0x0001d080 | PHYMOD_REG_ACC_TSC_IBLK)

#define RXTXCOM_OSR_MODE_CTLr_SIZE 4

#define RXTXCOM_OSR_MODE_CTLr_CLR(r) r = 0
#define RXTXCOM_OSR_MODE_CTLr_SET(r,d) r = d
#define RXTXCOM_OSR_MODE_CTLr_GET(r) r

#define RXTXCOM_OSR_MODE_CTLr_OSR_MODE_FRCf_GET(r) (((r) >> 15) & 0x1)
#define RXTXCOM_OSR_MODE_CTLr_OSR_MODE_FRCf_SET(r,f) r=((r & ~((uint32_t)0x1 << 15)) | ((((uint32_t)f) & 0x1) << 15)) | (1 << (16 + 15))
#define RXTXCOM_OSR_MODE_CTLr_OSR_MODE_FRC_VALf_GET(r) ((r) & 0xf)
#define RXTXCOM_OSR_MODE_CTLr_OSR_MODE_FRC_VALf_SET(r,f) r=((r & ~((uint32_t)0xf)) | (((uint32_t)f) & 0xf)) | (0xf << 16)



/*****************************************************************************************/
/*                           merlin16                                                    */
/*****************************************************************************************/
  
typedef struct {
    uint32_t ucode_size;
    uint16_t stack_size;
    uint16_t crc_value;
} ucode_info_t;

#define UCODE_MAX_SIZE  (64*1024 + 7168)    /* additional code that could be stored in data ram */

#define SOC_PM4X10_NUM_LANES    (4)
#define SOC_PM4X10_LANE_MASK    (0xF)

typedef struct phymod_lane_map_s {
    uint32_t num_of_lanes; /**< Number of elements in lane_map_rx/tx arrays */
    uint32_t lane_map_rx[PHYMOD_MAX_LANES_PER_CORE]; /**< lane_map_rx[x]=y means that rx lane x is mapped to rx lane y */
    uint32_t lane_map_tx[PHYMOD_MAX_LANES_PER_CORE]; /**< lane_map_tx[x]=y means that tx lane x is mapped to tx lane y */
} phymod_lane_map_t;


/** Base of core variable block, MERLIN16 variant. */
#define CORE_VAR_RAM_BASE (0x400)
/** Base of lane variable block, MERLIN16 variant. */
#define LANE_VAR_RAM_BASE (0x500)
/** Size of lane variable block, MERLIN16 variant. */
#define LANE_VAR_RAM_SIZE (0x100)

/* returns 000111 (7 = 8-1), for n = 3  */
#define BFMASK(n) ((1<<((n)))-1)
#define MHZ_TO_VCO_RATE(mhz) ((uint8_t)((((uint16_t)(mhz) + 125) / 250) - 22))
#define VCO_RATE_TO_MHZ(vco_rate) (((uint16_t)(vco_rate) + 22) * 250)

#define SRDS_INTERNAL_COMPOSE_PLL_DIV(integer_, fraction_num_, fraction_num_width_)                   \
    (((uint32_t)(integer_) & 0xFFFUL)                                                                 \
     | ((((((uint32_t)(fraction_num_) << (32-(fraction_num_width_))) >> (32-20-1)) + 1) >> 1) << 12))


/** Lane Config Variable Structure in Microcode */
struct merlin16_uc_lane_config_field_st {
  uint8_t  lane_cfg_from_pcs ;
  uint8_t  an_enabled        ;
  uint8_t  dfe_on            ;
  uint8_t  force_brdfe_on    ;
  uint8_t  media_type        ;
  uint8_t  unreliable_los    ;
  uint8_t  scrambling_dis    ;
  uint8_t  cl72_auto_polarity_en ;
  uint8_t  cl72_restart_timeout_en;
  uint16_t reserved;
};

/** Core Config Variable Structure in Microcode */
struct merlin16_uc_core_config_field_st {
    uint8_t  core_cfg_from_pcs ;
    uint8_t  vco_rate          ;
    uint8_t  an_los_workaround ;
    uint8_t  reserved1         ;
    uint8_t  reserved2         ;
};

typedef struct phymod_firmware_core_config_s {
    uint32_t CoreConfigFromPCS;
    uint32_t VcoRate; /**< vco rate */
    uint32_t disable_write_pll_iqp; /**< When 1 ucode will not update pll_iqp */
    uint32_t osr_2p5_en; /**< Enable 2p5 Oversample Mode */
} phymod_firmware_core_config_t;
  
/** Lane Config Struct */
struct  merlin16_uc_lane_config_st {
    struct merlin16_uc_lane_config_field_st field;
    uint16_t word;
};
  
/** Core Config Struct */
struct  merlin16_uc_core_config_st {
    struct merlin16_uc_core_config_field_st field;
    uint16_t word;
    int vco_rate_in_Mhz; /* if >0 then will get converted and replace field.vco_rate when update is called */
};

typedef enum phymod_firmware_media_type_e {
    phymodFirmwareMediaTypePcbTraceBackPlane = 0, /**< back plane */
    phymodFirmwareMediaTypeCopperCable, /**< copper cable */
    phymodFirmwareMediaTypeOptics, /**< optical */
    phymodFirmwareMediaTypeCount
} phymod_firmware_media_type_t;

typedef struct phymod_firmware_lane_config_s {
    uint32_t LaneConfigFromPCS;
    uint32_t AnEnabled; /**< Autoneg */
    uint32_t DfeOn; /**< Enable DFE */
    uint32_t ForceBrDfe; /**< Force Baud rate DFE */
    uint32_t LpDfeOn; /**< Enable low power DFE */
    phymod_firmware_media_type_t MediaType; /**< Media Type */
    uint32_t UnreliableLos; /**< For optical use */
    uint32_t ScramblingDisable; /**< disable scrambling */
    uint32_t Cl72AutoPolEn; /**< Forced CL72 */
    uint32_t Cl72RestTO; /**< Forced CL72 */
    uint32_t ForceExtenedReach; /**< Forced extened reach mode */
    uint32_t ForceNormalReach; /**< Forced normal reach mode */
    uint32_t LpPrecoderEnabled; /**< linker partner has pre-coder on */
    uint32_t ForcePAM4Mode; /**< Force PAM4 mode */
    uint32_t ForceNRZMode; /**< Forced CL72 */
    uint32_t AnCl72TxInitSkipOnRestart; /**< skip the TXFIR initialization in a restart event during the AN link training */
    uint32_t ForceOsCdr; /**< Force Over sample CDr */
} phymod_firmware_lane_config_t;

typedef enum {
    QMOD16_TX_LANE_RESET = 0,
    QMOD16_TX_LANE_TRAFFIC_ENABLE = 1,
    QMOD16_TX_LANE_TRAFFIC_DISABLE = 2,
    QMOD16_TX_LANE_RESET_TRAFFIC_ENABLE = 3,
    QMOD16_TX_LANE_RESET_TRAFFIC_DISABLE = 4,
    QMOD16_TX_LANE_TYPE_COUNT
}tx_lane_disable_type_t;

typedef enum {
    QMOD16_PMA_OS_MODE_1        = 0   ,  /*!< Over sampling Mode 1         */
    QMOD16_PMA_OS_MODE_2        = 1   ,  /*!< Over sampling Mode 2         */
    QMOD16_PMA_OS_MODE_3        = 2   ,  /*!< Over sampling Mode 3         */
    QMOD16_PMA_OS_MODE_3_3      = 3   ,  /*!< Over sampling Mode 3.3       */
    QMOD16_PMA_OS_MODE_4        = 4   ,  /*!< Over sampling Mode 4         */
    QMOD16_PMA_OS_MODE_5        = 5   ,  /*!< Over sampling Mode 5         */
    QMOD16_PMA_OS_MODE_7_25     = 6   ,  /*!< Over sampling Mode 7.25      */
    QMOD16_PMA_OS_MODE_8        = 7   ,  /*!< Over sampling Mode 8         */
    QMOD16_PMA_OS_MODE_8_25     = 8   ,  /*!< Over sampling Mode 8.25      */
    QMOD16_PMA_OS_MODE_10       = 9   ,  /*!< Over sampling Mode 10        */
    QMOD16_PMA_OS_MODE_ILLEGAL  = 15    /*!< Over sampling Mode Illegal   */
  } qmod16_os_mode_type;


  typedef enum {
    QMOD16_PLL_MODE_DIV_ZERO    = 0   ,  /*!< Illegal PLL Divider Number      */
    QMOD16_PLL_MODE_DIV_40      = 2   ,  /*!< Multiply ref. clk by 40         */
    QMOD16_PLL_MODE_DIV_42      = 3   ,  /*!< Multiply ref. clk by 42         */
    QMOD16_PLL_MODE_DIV_48      = 4   ,  /*!< Multiply ref. clk by 48         */
    QMOD16_PLL_MODE_DIV_52      = 6   ,  /*!< Multiply ref. clk by 52         */
    QMOD16_PLL_MODE_DIV_54      = 7   ,  /*!< Multiply ref. clk by 54         */
    QMOD16_PLL_MODE_DIV_60      = 8   ,  /*!< Multiply ref. clk by 60         */
    QMOD16_PLL_MODE_DIV_64      = 9   ,  /*!< Multiply ref. clk by 64         */
    QMOD16_PLL_MODE_DIV_66      = 10  ,  /*!< Multiply ref. clk by 66         */
    QMOD16_PLL_MODE_DIV_70      = 12  ,  /*!< Multiply ref. clk by 70         */
    QMOD16_PLL_MODE_DIV_80      = 13  ,  /*!< Multiply ref. clk by 80         */
    QMOD16_PLL_MODE_DIV_92      = 14  ,  /*!< Multiply ref. clk by 92         */
    QMOD16_PLL_MODE_DIV_ILLEGAL         /*!< Illegal (programmatic boundary) */
  } qmod16_pll_mode_type;
  
enum merlin16_pll_refclk_enum {
    MERLIN16_PLL_REFCLK_UNKNOWN = 0, /* Refclk value to be determined by API. */
    MERLIN16_PLL_REFCLK_50MHZ          = 0x00100032UL, /* 50 MHz          */
    MERLIN16_PLL_REFCLK_125MHZ         = 0x0010007DUL, /* 125 MHz         */
    MERLIN16_PLL_REFCLK_156P25MHZ      = 0x00400271UL, /* 156.25 MHz      */
    MERLIN16_PLL_REFCLK_161P1328125MHZ = 0x08005091UL  /* 161.1328125 MHz */
};


enum merlin16_pll_div_enum {
    MERLIN16_PLL_DIV_UNKNOWN = 0, /* Divide value to be determined by API. */
    MERLIN16_PLL_DIV_52P751515  = (int)0xC0634034, /* Divide by 52.751515  */
    MERLIN16_PLL_DIV_54P4       = (int)0x66668036, /* Divide by 54.4       */
    MERLIN16_PLL_DIV_58P181818  = (int)0x2E8BC03A, /* Divide by 58.181818  */
    MERLIN16_PLL_DIV_60         = (int)0x0000003C, /* Divide by 60         */
    MERLIN16_PLL_DIV_62P060606  = (int)0x0F84003E, /* Divide by 62.060606  */
    MERLIN16_PLL_DIV_64         = (int)0x00000040, /* Divide by 64         */
    MERLIN16_PLL_DIV_66         = (int)0x00000042, /* Divide by 66         */
    MERLIN16_PLL_DIV_66P460703  = (int)0x75F0C042, /* Divide by 66.460703  */
    MERLIN16_PLL_DIV_66P743079  = (int)0xBE3A8042, /* Divide by 66.743079  */
    MERLIN16_PLL_DIV_67P878788  = (int)0xE0F84043, /* Divide by 67.878788  */
    MERLIN16_PLL_DIV_68         = (int)0x00000044, /* Divide by 68         */
    MERLIN16_PLL_DIV_68P537598  = (int)0x89A00044, /* Divide by 68.537598  */
    MERLIN16_PLL_DIV_68P570764  = (int)0x921D8044, /* Divide by 68.570764  */
    MERLIN16_PLL_DIV_68P828796  = (int)0xD42C0044, /* Divide by 68.828796  */
    MERLIN16_PLL_DIV_68P856242  = (int)0xDB32C044, /* Divide by 68.856242  */
    MERLIN16_PLL_DIV_69P152458  = (int)0x27078045, /* Divide by 69.152458  */
    MERLIN16_PLL_DIV_69P389964  = (int)0x63D4C045, /* Divide by 69.389964  */
    MERLIN16_PLL_DIV_69P818182  = (int)0xD1748045, /* Divide by 69.818182  */
    MERLIN16_PLL_DIV_70         = (int)0x00000046, /* Divide by 70         */
    MERLIN16_PLL_DIV_70P713596  = (int)0xB6AE4046, /* Divide by 70.713596  */
    MERLIN16_PLL_DIV_71P008     = (int)0x020C4047, /* Divide by 71.008     */
    MERLIN16_PLL_DIV_71P112952  = (int)0x1CEA8047, /* Divide by 71.P112952 */
    MERLIN16_PLL_DIV_71P31347   = (int)0x503F8047, /* Divide by 71.31347   */
    MERLIN16_PLL_DIV_71P5584    = (int)0x8EF34047, /* Divide by 71.5584    */
    MERLIN16_PLL_DIV_72         = (int)0x00000048, /* Divide by 72         */
    MERLIN16_PLL_DIV_73P335232  = (int)0x55D1C049, /* Divide by 73.335232  */
    MERLIN16_PLL_DIV_73P6       = (int)0x9999C049, /* Divide by 73.6       */
    MERLIN16_PLL_DIV_75         = (int)0x0000004B, /* Divide by 75         */
    MERLIN16_PLL_DIV_80         = (int)0x00000050, /* Divide by 80         */
    MERLIN16_PLL_DIV_82P5       = (int)0x80000052, /* Divide by 82.5       */
    MERLIN16_PLL_DIV_85P671997  = (int)0xAC080055, /* Divide by 85.671997  */
    MERLIN16_PLL_DIV_86P036     = (int)0x09374056, /* Divide by 86.036     */
    MERLIN16_PLL_DIV_87P5       = (int)0x80000057, /* Divide by 87.5       */
    MERLIN16_PLL_DIV_88P392     = (int)0x645A4058, /* Divide by 88.392     */
    MERLIN16_PLL_DIV_88P76      = (int)0xC28F8058, /* Divide by 88.76      */
    MERLIN16_PLL_DIV_89P141838  = (int)0x244F8059, /* Divide by 89.141838  */
    MERLIN16_PLL_DIV_89P447998  = (int)0x72B00059, /* Divide by 89.447998  */
    MERLIN16_PLL_DIV_90         = (int)0x0000005A, /* Divide by 90         */
    MERLIN16_PLL_DIV_91P669037  = (int)0xAB46005B, /* Divide by 91.669037  */
    MERLIN16_PLL_DIV_92         = (int)0x0000005C, /* Divide by 92         */
    MERLIN16_PLL_DIV_100        = (int)0x00000064, /* Divide by 100        */
    MERLIN16_PLL_DIV_170        = (int)0x000000AA, /* Divide by 170        */
    MERLIN16_PLL_DIV_187P5      = (int)0x800000BB, /* Divide by 187.5      */
    MERLIN16_PLL_DIV_200        = (int)0x000000C8, /* Divide by 200        */
    MERLIN16_PLL_DIV_206P25     = (int)0x400000CE  /* Divide by 206.25     */
};

/** PLL Configuration Options Enum */
enum merlin16_pll_option_enum {
    MERLIN16_PLL_OPTION_NONE,
    MERLIN16_PLL_OPTION_REFCLK_DOUBLER_EN,
    MERLIN16_PLL_OPTION_REFCLK_DIV2_EN,
    MERLIN16_PLL_OPTION_REFCLK_DIV4_EN,
    MERLIN16_PLL_OPTION_DISABLE_VERIFY
};


typedef enum phymod_interface_e {
    phymodInterfaceBypass = 0,
    phymodInterfaceSR,
    phymodInterfaceSR4,
    phymodInterfaceKX,
    phymodInterfaceKX4,
    phymodInterfaceKR,
    phymodInterfaceKR2,
    phymodInterfaceKR4,
    phymodInterfaceCX,
    phymodInterfaceCX2,
    phymodInterfaceCX4,
    phymodInterfaceCR,
    phymodInterfaceCR2,
    phymodInterfaceCR4,
    phymodInterfaceCR10,
    phymodInterfaceXFI,
    phymodInterfaceSFI,
    phymodInterfaceSFPDAC,
    phymodInterfaceXGMII,
    phymodInterface1000X,
    phymodInterfaceSGMII,
    phymodInterfaceXAUI,
    phymodInterfaceRXAUI,
    phymodInterfaceX2,
    phymodInterfaceXLAUI,
    phymodInterfaceXLAUI2,
    phymodInterfaceCAUI,
    phymodInterfaceQSGMII,
    phymodInterfaceLR4,
    phymodInterfaceLR,
    phymodInterfaceLR2,
    phymodInterfaceER,
    phymodInterfaceER2,
    phymodInterfaceER4,
    phymodInterfaceSR2,
    phymodInterfaceSR10,
    phymodInterfaceCAUI4,
    phymodInterfaceVSR,
    phymodInterfaceLR10,
    phymodInterfaceKR10,
    phymodInterfaceCAUI4_C2C,
    phymodInterfaceCAUI4_C2M,
    phymodInterfaceZR,
    phymodInterfaceLRM,
    phymodInterfaceXLPPI,
    phymodInterfaceOTN,
    phymodInterfaceCount
} phymod_interface_t;

typedef enum phymod_ref_clk_e {
    phymodRefClk156Mhz = 0, /**< 156.25MHz */
    phymodRefClk125Mhz, /**< 125MHz */
    phymodRefClk106Mhz, /**< 106Mhz */
    phymodRefClk161Mhz, /**< 161Mhz */
    phymodRefClk174Mhz, /**< 174Mhz */
    phymodRefClk312Mhz, /**< 312Mhz */
    phymodRefClk322Mhz, /**< 322Mhz */
    phymodRefClk349Mhz, /**< 349Mhz */
    phymodRefClk644Mhz, /**< 644Mhz */
    phymodRefClk698Mhz, /**< 698Mhz */
    phymodRefClk155Mhz, /**< 155Mhz */
    phymodRefClk156P6Mhz, /**< 156P6Mhz */
    phymodRefClk157Mhz, /**< 157Mhz */
    phymodRefClk158Mhz, /**< 158Mhz */
    phymodRefClk159Mhz, /**< 159Mhz */
    phymodRefClk168Mhz, /**< 168Mhz */
    phymodRefClk172Mhz, /**< 172Mhz */
    phymodRefClk173Mhz, /**< 173Mhz */
    phymodRefClk169P409Mhz, /**< 169P409Mhz */
    phymodRefClk348P125Mhz, /**< 348P125Mhz */
    phymodRefClk162P948Mhz, /**< 162P948Mhz */
    phymodRefClk336P094Mhz, /**< 336P094Mhz */
    phymodRefClk168P12Mhz, /**< 168P12Mhz */
    phymodRefClk346P74Mhz, /**< 346P74Mhz */
    phymodRefClk167P41Mhz, /**< 167P41Mhz */
    phymodRefClk345P28Mhz, /**< 345P28Mhz */
    phymodRefClk162P26Mhz, /**< 162P26Mhz */
    phymodRefClk334P66Mhz, /**< 334P66Mhz */
    phymodRefClkCount
} phymod_ref_clk_t;

typedef enum phymod_otn_type_e {
    phymodOTNOTU1 = 0,
    phymodOTNOTU1e,
    phymodOTNOTU2,
    phymodOTNOTU2e,
    phymodOTNOTU2f,
    phymodOTNOTU3,
    phymodOTNOTU3e2,
    phymodOTNOTU4,
    phymodOTNOTU4p10,
    phymodOTNCount
} phymod_otn_type_t;

typedef struct phymod_phy_inf_config_s {
    phymod_interface_t interface_type;
    uint32_t data_rate;
    uint32_t interface_modes;
    phymod_ref_clk_t ref_clock; /**< Core reference clock. */
    phymod_ref_clk_t com_clock; /**< Core common clock. */
    uint16_t pll_divider_req; /**< Core pll divider request. */
    void* device_aux_modes; /**< Device auxiliary modes. */
    phymod_otn_type_t otn_type; /**< OTN type. */
} phymod_phy_inf_config_t;



#endif