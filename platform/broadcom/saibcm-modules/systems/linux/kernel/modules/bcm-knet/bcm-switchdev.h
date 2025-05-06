#ifndef _BCM_SWITCHDEV_H_
#define _BCM_SWITCHDEV_H_


/*****************************************************************************************/
/*                           GPORT   BCM56370                                            */
/*****************************************************************************************/


//Register: GPORT_MODE_REG.gxport0 general register address 0x02020100
//Flags:
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: GPORT Mode register
//Displaying: reset defaults, reset value 1 mask 0x3b
/*
soc_field_info_t soc_GPORT_MODE_REG_BCM53400_A0r_fields[] = {
    { EGR_1588_TIMESTAMPING_MODEf, 1, 0, 0 },
    { EP_TO_GP_CRC_FWDf, 1, 4, 0 },
    { EP_TO_GP_CRC_MODES_SELf, 1, 5, 0 },
    { EP_TO_GP_CRC_OWRTf, 1, 3, 0 },
    { OSTS_TIMER_DISABLEf, 1, 1, 0 },
    { TS_TIMER_OVERRIDEf, 1, 2, SOCF_SC|SOCF_RES }
};
 */
#define GPORT_MODE_REGr                  0x02020100


//Register: GPORT_CONFIG.gxport0 general register address 0x02020000
//Flags:
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: GPORT configuration Register
//Displaying: reset defaults, reset value 0 mask 1
/*
soc_field_info_t soc_GPORT_CONFIG_BCM53400_A0r_fields[] = {
    { CLR_CNTf, 1, 1, SOCF_RES },
    { GPORT_ENf, 1, 0, 0 }
};
 */
#define GPORT_CONFIGr                     0x02020000

//Register: GPORT_RSV_MASK.gxport0 general register address 0x02020600
//Flags:
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: GPORT RSV MASK register, used to purge packet data received from the MACs
//Displaying: reset defaults, reset value 0x70 mask 0x3ffff
#define GPORT_RSV_MASKr                   0x02020600


//Register: GPORT_STAT_UPDATE_MASK.gxport0 general register address 0x02020700
//Flags:
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: GPORT STAT_UPDATE MASK register, used to control the statistic counter update in the Ingress
//Displaying: reset defaults, reset value 0x70 mask 0x3ffff
#define GPORT_STAT_UPDATE_MASKr           0x02020700

/*****************************************************************************************/
/*                           Registers (IPROC)  BCM56370                                 */
/*****************************************************************************************/
// MIIM_CH0_ADDRESS          0x10019008
typedef union miim_ch_address_s {
    struct _miim_ch_addr_ {
    #if defined(LE_HOST)
    uint32_t  r0:6,
              PHY_IDf:5,
              CLAUSE_22_REGADRR_OR_45_DTYPEf:5,
              CLAUSE_45_REG_ADRRf:16;
    #else
    uint32_t  CLAUSE_45_REG_ADRRf:16,
              CLAUSE_22_REGADRR_OR_45_DTYPEf:5,
              PHY_IDf:5,
              r0:6;
    #endif
    }reg;
    uint32_t word;
} miim_ch_address_t;
#define MIIM_CH0_ADDRESSr    0x10019008

/*
soc_field_info_t soc_MIIM_CH0_PARAMS_BCM56980_A0r_fields[] = {
    { MDIO_OP_TYPEf, 3, 17, SOCF_LE },
    { PHY_WR_DATAf, 16, 0, SOCF_LE },
    { RING_MAPf, 12, 20, SOCF_LE },
    { SEL_INT_PHYf, 1, 16, 0 }
};
*/
typedef union miim_ch_params_s {
    struct _ch_params_ {
    #if defined(LE_HOST)
    uint32_t  PHY_WR_DATAf:16,
              SEL_INT_PHYf:1,
              MDIO_OP_TYPEf:3,
              RING_MAPf:12;
    #else
    uint32_t  RING_MAPf:12,
              MDIO_OP_TYPEf:3,
              SEL_INT_PHYf:1,
              PHY_WR_DATAf:16;
    #endif
    }reg;
    uint32_t word;
} miim_ch_params_t;

#define MIIM_CH0_PARAMSr     0x10019004

typedef union miim_ch_control_s {
    struct _ch_control_ {
    #if defined(LE_HOST)
    uint32_t  STARTf:1,
              r0:31;
    #else
    uint32_t  r0:31,
              STARTf:1;
    #endif
    }reg;
    uint32_t word;
}miim_ch_control_t;

#define MIIM_CH0_CONTROLr    0x10019000  //BCM56370 

/*
soc_field_info_t soc_MIIM_CH0_STATUS_BCM56870_A0r_fields[] = {
    { ACTIVEf, 1, 16, SOCF_RO },
    { DONEf, 1, 18, SOCF_RO },
    { ERRORf, 1, 17, SOCF_RO },
    { PHY_RD_DATAf, 16, 0, SOCF_LE|SOCF_RO }
};
*/
typedef union miim_ch_status_s {
    struct _ch_status_ {
    #if defined(LE_HOST)
    uint32_t  PHY_RD_DATAf:16,
              ACTIVEf:1,
              ERRORf:1,
              DONEf:1,
              r0:14;
    #else
    uint32_t  r0:14,
              DONEf:1,
              ERRORf:1,
              ACTIVEf:1,
              PHY_RD_DATAf:16;
    #endif
    }reg;
    uint32_t word;
} miim_ch_status_t;
#define MIIM_CH0_STATUSr     0x1001900c

/* MIIM port selection (bit map of clause selection, 1=22, 0=45; Lynx only) */

#define MIIM_CYCLE_AUTO                 0x0
#define MIIM_CYCLE_C22_REG_WR           0x1
#define MIIM_CYCLE_C22_REG_RD           0x2
#define MIIM_CYCLE_C45_REG_AD           0x4
#define MIIM_CYCLE_C45_REG_WR           0x5
#define MIIM_CYCLE_C45_REG_RD_ADINC     0x6
#define MIIM_CYCLE_C45_REG_RD           0x7

/* CL45 warmboot write disable override */
#define MIIM_WB_C45                     (1 << 0)

#define MIIM_CYCLE_C45_SHFT             6
#define MIIM_CYCLE_C45_MASK             (0x3 << MIIM_CYCLE_C45_SHFT)
#define MIIM_CYCLE_C45_WR               (1 << (MIIM_CYCLE_C45_SHFT + 0))
#define MIIM_CYCLE_C45_WR_AD            (1 << (MIIM_CYCLE_C45_SHFT + 1))
#define MIIM_CYCLE_C45_RD               (1 << (MIIM_CYCLE_C45_SHFT + 0))
#define MIIM_CYCLE_C45_RD_ADINC         (1 << (MIIM_CYCLE_C45_SHFT + 1))

#define SOC_CLAUSE_45 45
#define SOC_CLAUSE_22 22

#define CMICX_MIIM_RING_INDEX_START     0
#define CMICX_MIIM_RING_INDEX_END       7
#define CMICX_MIIM_12R_RING_INDEX_END   11

/* MDIO CYCLE types for iProc 15 and higher devices
 * with support for 12 MDIO rings */

#define MIIM_CYCLE_12R_C22_REG_WR           0x0
#define MIIM_CYCLE_12R_C22_REG_RD           0x1
#define MIIM_CYCLE_12R_C45_AUTO_WR          0x2
#define MIIM_CYCLE_12R_C45_AUTO_RD          0x3
#define MIIM_CYCLE_12R_C45_REG_AD           0x4
#define MIIM_CYCLE_12R_C45_REG_WR           0x5
#define MIIM_CYCLE_12R_C45_REG_RD           0x6
#define MIIM_CYCLE_12R_C45_REG_RD_ADINC     0x7


#define PHY_ID_BUS_UPPER_MASK     0x300
#define PHY_ID_BUS_UPPER_SHIFT    0x6
#define PHY_ID_BUS_LOWER_MASK     0x60
#define PHY_ID_BUS_LOWER_SHIFT    5
#define PHY_ID_BUS_NUM(_id)   ((((_id) & PHY_ID_BUS_UPPER_MASK) >> \
        PHY_ID_BUS_UPPER_SHIFT) | (((_id) & PHY_ID_BUS_LOWER_MASK) >> \
        PHY_ID_BUS_LOWER_SHIFT))
#define PHY_ID_ADDR_MASK          0x1f
#define PHY_ID_ADDR_SHIFT         0
#define PHY_ID_ADDR(_id)          (((_id) & PHY_ID_ADDR_MASK) >> PHY_ID_ADDR_SHIFT)
#define PHY_ID_INTERNAL(_id)      ((_id & 0x80) ? 1 : 0)
#define PHY_ID_BROADCAST(_id)     ((_id & 0x400) ? 1 : 0)
#define PHY_ID_BUSMAP_SHIFT       16
#define PHY_ID_BUSMAP(_id)        (_id >> PHY_ID_BUSMAP_SHIFT)

/* 
 * MII Status Register: See 802.3, 1998 pg 544 
 */
#define MII_STAT_EXT            (1 << 0) /* Extended Registers */
#define MII_STAT_JBBR           (1 << 1) /* Jabber Detected */
#define MII_STAT_LA             (1 << 2) /* Link Active */
#define MII_STAT_AN_CAP         (1 << 3) /* Autoneg capable */
#define MII_STAT_RF             (1 << 4) /* Remote Fault */
#define MII_STAT_AN_DONE        (1 << 5) /* Autoneg complete */
#define MII_STAT_MF_PS          (1 << 6) /* Preamble suppression */
#define MII_STAT_ES             (1 << 8) /* Extended status (R15) */
#define MII_STAT_HD_100_T2      (1 << 9) /* Half duplex 100Mb/s supported */
#define MII_STAT_FD_100_T2      (1 << 10)/* Full duplex 100Mb/s supported */
#define MII_STAT_HD_10          (1 << 11)/* Half duplex 100Mb/s supported */
#define MII_STAT_FD_10          (1 << 12)/* Full duplex 100Mb/s supported */
#define MII_STAT_HD_100         (1 << 13)/* Half duplex 100Mb/s supported */
#define MII_STAT_FD_100         (1 << 14)/* Full duplex 100Mb/s supported */
#define MII_STAT_100_T4         (1 << 15)/* Full duplex 100Mb/s supported */


/* Standard MII Registers */

#define MII_CTRL_REG            0x00    /* MII Control Register : r/w */
#define MII_STAT_REG            0x01    /* MII Status Register: ro */
#define MII_PHY_ID0_REG         0x02    /* MII PHY ID register: r/w */
#define MII_PHY_ID1_REG         0x03    /* MII PHY ID register: r/w */
#define MII_ANA_REG             0x04    /* MII Auto-Neg Advertisement: r/w */
#define MII_ANP_REG             0x05    /* MII Auto-Neg Link Partner: ro */
#define MII_AN_EXP_REG          0x06    /* MII Auto-Neg Expansion: ro */
#define MII_GB_CTRL_REG         0x09    /* MII 1000Base-T control register */
#define MII_GB_STAT_REG         0x0a    /* MII 1000Base-T Status register */
#define MII_ESR_REG             0x0f    /* MII Extended Status register */


/* MIDO  setting */
#define RATE_EXT_MDIO_DIVISOR_DEF        50
#define TD3X_RATE_INT_MDIO_DIVISOR_DEF   25

#define MIIM_RING0_CONTROLr     0x100190f0
#define MIIM_RING1_CONTROLr     0x100190f4
#define MIIM_RING2_CONTROLr     0x100190f8
#define MIIM_RING3_CONTROLr     0x100190fc
#define MIIM_RING4_CONTROLr     0x10019100
#define MIIM_RING5_CONTROLr     0x10019104
#define MIIM_RING6_CONTROLr     0x10019108
#define MIIM_RING7_CONTROLr     0x1001910c
#define MIIM_RING8_CONTROLr     0x10019110
#define MIIM_RING9_CONTROLr     0x10019114
#define MIIM_RING10_CONTROLr    0x10019118
#define MIIM_RING11_CONTROLr    0x1001911c



//soc_MIIM_RING0_CONTROL_BCM56870_A0r_fields
/*
soc_field_info_t soc_MIIM_RING0_CONTROL_BCM56870_A0r_fields[] = {
    { CLOCK_DIVIDER_EXTf, 8, 8, SOCF_LE },
    { CLOCK_DIVIDER_INTf, 8, 0, SOCF_LE },
    { MDC_MODEf, 1, 26, 0 },
    { MDIO_OUT_DELAYf, 8, 16, SOCF_LE },
    { PREAMBLEf, 2, 24, SOCF_LE }
};
*/
typedef union miim_ring_control_s {
    struct _ring_control_ {
    #if defined(LE_HOST)
    uint32_t  CLOCK_DIVIDER_INTf:8,
              CLOCK_DIVIDER_EXTf:8,
              MDIO_OUT_DELAYf:8,
              PREAMBLEf:2,
              MDC_MODEf:1,
              r0:5;
    #else
    uint32_t  r0:5,
              MDC_MODEf:1,
              PREAMBLEf:2,
              MDIO_OUT_DELAYf:8,
              CLOCK_DIVIDER_EXTf:8,
              CLOCK_DIVIDER_INTf:8;
    #endif
    }reg;
    uint32_t word;
} miim_ring_control_t;

/*****************************************************************************************/
/*                            MMU related                                                */
/*****************************************************************************************/
#define WRED_REFRESH_CONTROLr      0x22000600
#define MMU_1DBG_Cr                0x15808000
#define MMU_2DBG_C_1r              0x15808300

#define THDU_OUTPUT_PORT_RX_ENABLE_SPLIT0r            0x39807400
#define THDU_OUTPUT_PORT_RX_ENABLE_SPLIT1r            0x39807500
#define MMU_THDM_DB_PORT_RX_ENABLE_64_SPLIT0r         0x3d807400
#define MMU_THDM_DB_PORT_RX_ENABLE_64_SPLIT1r         0x3d807600
#define MMU_THDM_MCQE_PORT_RX_ENABLE_64_SPLIT0r       0x41807400
#define MMU_THDM_MCQE_PORT_RX_ENABLE_64_SPLIT1r       0x41807600

#define Q_SCHED_PORT_FLUSH_SPLIT0r                    0x11800100
#define Q_SCHED_PORT_FLUSH_SPLIT1r                    0x11800200

#define MTRO_PORT_ENTITY_DISABLE_SPLIT0r              0x15800200
#define MTRO_PORT_ENTITY_DISABLE_SPLIT1r              0x15800300 


/*
soc_field_info_t soc_Q_SCHED_RQE_SNAPSHOTr_fields[] = {
    { INITIATEf, 1, 0, SOCF_RWBW|SOCF_RES },
    { SNAPSHOT_DONE_DELAYf, 2, 1, SOCF_LE|SOCF_RES }
};
*/
typedef union q_sched_rqe_s {
    struct _q_sched_rqe_ {
    #if defined(LE_HOST)
    uint32_t  INITIATEf:1,
              SNAPSHOT_DONE_DELAYf:2,
              r0:29;
    #else
    uint32_t  r0:29,
              SNAPSHOT_DONE_DELAYf:2,
              INITIATEf:1;
    #endif
    }reg;
    uint32_t word;
} q_sched_rqe_t;
#define Q_SCHED_RQE_SNAPSHOTr                         0x11800500


/*****************************************************************************************/
/*                            UNIMAC                                                     */
/*****************************************************************************************/
#define SOC_UNIMAC_SPEED_10     0x0
#define SOC_UNIMAC_SPEED_100    0x1
#define SOC_UNIMAC_SPEED_1000   0x2
#define SOC_UNIMAC_SPEED_2500   0x3

#define SOC_UNIMAC_MAX_FRAME_SIZE   16360
#define SOC_UNIMAC_WAKE_TIMER       17
#define SOC_UNIMAC_LPI_TIMER        4

/* EEE related defination */
#define SOC_UNIMAC_MAX_EEE_SPEED    2500
#define SOC_UNIMAC_EEE_REF_CNT       250



#define UNIMAC_INIT_F_AUTO_CFG               0x1
#define UNIMAC_INIT_F_RX_STRIP_CRC           0x2

#define UNIMAC_ENABLE_SET_FLAGS_TX_EN              0x1
#define UNIMAC_ENABLE_SET_FLAGS_RX_EN              0x2

//Register: FRM_LENGTH.ge0 port register address 0x00010500
//Flags:
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: Maximum Frame Length.
//Displaying: reset defaults, reset value 0x5ee mask 0x3fff
#define FRM_LENGTHr                                0x00010500

//Register: TAG_0.ge0 port register address 0x00011200
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: Programmable vlan outer tag
//Displaying: reset defaults, reset value 0x18100 mask 0x1ffff
//  FRM_TAG_0<15:0> = 0x8100
//  CONFIG_OUTER_TPID_ENABLE<16> = 1
#define TAG_0r                                     0x00011200

//Register: TAG_1.ge0 port register address 0x00011300
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: Programmable vlan inner tag
//Displaying: reset defaults, reset value 0x18100 mask 0x1ffff
//  FRM_TAG_1<15:0> = 0x8100
//  CONFIG_INNER_TPID_ENABLE<16> = 1
//  BCM.0> listreg TAG_1 
#define TAG_1r                                     0x00011300  
  

//Register: UMAC_TIMESTAMP_ADJUST.ge0 port register address 0x00011d00
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: 1588_one_step_timestamp control
//Displaying: reset defaults, reset value 0x600 mask 0x7ff
//  EN_1588<9> = 1
//  AUTO_ADJUST<10> = 1
//  ADJUST<8:0> = 0
#define UMAC_TIMESTAMP_ADJUSTr                     0x00011d00

//Register: PAUSE_CONTROL.ge0 port register address 0x0001cc00
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: PAUSE frame timer control register
//Displaying: reset defaults, reset value 0x2ffff mask 0x3ffff
//  VALUE<16:0> = 0xffff
//  ENABLE<17> = 1
#define PAUSE_CONTROLr                             0x0001cc00
  
//Register: PAUSE_QUANT.ge0 port register address 0x00010600
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: Receive Pause Quanta.
//Displaying: reset defaults, reset value 0xffff mask 0xffff
//  STAD2<15:0> = 0xffff
#define PAUSE_QUANTr                               0x00010600

//Register: MAC_PFC_REFRESH_CTRL.ge0 port register address 0x0001d100
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: PPP refresh control register
//Displaying: reset defaults, reset value 0 mask 0
//  PFC_REFRESH_TIMER<31:16> = x
//  PFC_REFRESH_EN<0> = x
#define MAC_PFC_REFRESH_CTRLr                      0x0001d100

     
//Register: TX_IPG_LENGTH.ge0 port register address 0x00011700
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: Programmable Inter-Packet-Gap (IPG).
//Displaying: reset defaults, reset value 0 mask 0x7f
//  TX_IPG_LENGTH<6:0> = 0
#define TX_IPG_LENGTHr                             0x00011700


//Register: UMAC_EEE_REF_COUNT.ge0 port register address 0x00011c00
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: clock divider for 1 us quanta count in EEE
//Displaying: reset defaults, reset value 0x7d mask 0xffff
//  EEE_REF_COUNT<15:0> = 0x7d
#define UMAC_EEE_REF_COUNTr                        0x00011c00


//Register: GMII_EEE_WAKE_TIMER.ge0 port register address 0x00012100
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: GMII_EEE Wake timer
//Displaying: reset defaults, reset value 0x1e mask 0xffff
//  GMII_EEE_WAKE_TIMER<15:0> = 0x1e
#define GMII_EEE_WAKE_TIMERr                       0x00012100

//Register: GMII_EEE_DELAY_ENTRY_TIMER.ge0 port register address 0x00011b00
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: GMII_EEE LPI timer
//Displaying: reset defaults, reset value 0x3c mask 0xffffffff
//  GMII_EEE_LPI_TIMER<31:0> = 0x3c
#define GMII_EEE_DELAY_ENTRY_TIMERr                0x00011b00

//Register: PMQ_ECC_INIT_CTRL.pmqport0 general register address 0x02020200
//Blocks: pmqport0 pmqport1 pmqport2 (3 copies)
//Description: PMQ mode only -- GPORT and UniMAC Memory ECC ECC init control bits.
//Displaying: reset defaults, reset value 0 mask 0xfffff
//  UNIMAC_MEM_INIT<15:0> = 0
//  GP1_MEM_INIT<17> = 0
//  GP0_MEM_INIT<16> = 0
//  GMIB1_MEM_INIT<19> = 0
//  GMIB0_MEM_INIT<18> = 0
#define PMQ_ECC_INIT_CTRLr                         0x02020200

//Register: PMQ_ECC_INIT_STS.pmqport0 general register address 0x02020300
//Flags: read-only
//Blocks: pmqport0 pmqport1 pmqport2 (3 copies)
//Description: PMQ mode only -- GPORT and UniMAC Memory ECC ECC init done status bits.
//Displaying: reset defaults, reset value 0 mask 0xfffff
//  UNIMAC_MEM_INIT_DONE<15:0> = 0 [RO]
//  GP1_MEM_INIT_DONE<17> = 0 [RO]
//  GP0_MEM_INIT_DONE<16> = 0 [RO]
//  GMIB1_MEM_INIT_DONE<19> = 0 [RO]
//  GMIB0_MEM_INIT_DONE<18> = 0 [RO]
#define PMQ_ECC_INIT_STSr                         0x02020300
#define UNIMAC_MEM_INIT_DONE_MASK                 0xffff
#define GP_GMIB_MEM_INIT_DONE_MASK                0xf0000

//Register: PMQ_ECC.pmqport0 general register address 0x02020400
//Flags:
//Blocks: pmqport0 pmqport1 pmqport2 (3 copies)
//Description: PMQ mode only -- GPORT and UniMAC Memory ECC status and per-port-ecc mask register.
//ECC init/done is not required by firmware for PMQ Gen2 UniMAC fifos.
//The GPORT MIBs should be initialized via the mib_clr control bit.
//Bits [1:0] of this register are not used; bit 0 should always be written with 0, bit 1 can be ignored.
//Displaying: reset defaults, reset value 0 mask 0xffff0000
//  PORTNUM_ECCSTS_MASK<31:16> = 0
//  MEM_INIT<0> = x
//  INIT_DONE<1> = x [RO]
//  ECC_ERROR_MASK<3> = x [RO]
//  ECC_ERROR_EVENT<2> = x [RO]
#define PMQ_ECCr                                  0x02020400

/*****************************************************************************************/
/*                            Port Mapping                                               */
/*****************************************************************************************/

//Memory: ING_PHY_TO_IDB_PORT_MAP.ipipe0 address 0x00000000
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 79 with indices 0-78 (0x0-0x4e), each 1 bytes 1 words
//Entry mask: 0x000000ff
//Description: Ingress Physical to IDB Port Number Mapping Table. Indexed by the physical port number, and provides the corresponding idb port number.

/*
soc_field_info_t soc_ING_PHY_TO_IDB_PORT_MAP_BCM56370_A0m_fields[] = {
    { IDB_PORTf, 7, 1, SOCF_LE | SOCF_GLOBAL },
    { VALIDf, 1, 0, 0 | SOCF_GLOBAL }
};
 */
#define ING_PHY_TO_IDB_PORT_MAPm                      0x00000000
#define ING_PHY_TO_IDB_PORT_MAPm_BYTES                1
#define ING_PHY_TO_IDB_PORT_MAPm_MAX_INDEX            78

typedef struct {
    uint32_t entry_data[1];
}ing_phy2idb_entry_t;

//Memory: ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLE.ipipe0 address 0x00040000
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 72 with indices 0-71 (0x0-0x47), each 1 bytes 1 words
//Entry mask: 0x0000007f
//Description: Ingress IDB to Device Port Number Mapping Table. Indexed by the IDB port number, and provides the corresponding device port number.
#define ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm          0x00040000
#define ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm_BYTES    1
#define ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_MAX_INDEX       71    

/*
soc_field_info_t soc_ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLE_BCM56370_A0m_fields[] = {
    { DEVICE_PORT_NUMBERf, 7, 0, SOCF_LE | SOCF_GLOBAL }
};
 */
typedef struct {
    uint32_t entry_data[1];
}ing_idb2dev_entry_t;

//Register: EGR_DEVICE_TO_PHYSICAL_PORT_NUMBER_MAPPING.cpu0 port register address 0x28004100
//Blocks: epipe0 (1 copy)
//Description: Egress Device to Physical Port Number Mapping Register. Indexed by the Device port number, and provides the corresponding physical port number.
//Displaying: reset defaults, reset value 0x7f mask 0x7f
//  PHYSICAL_PORT_NUMBER<6:0> = 0x7f
#define EGR_DEVICE_TO_PHYSICAL_PORT_NUMBER_MAPPINGr           0x28004100

/*
SOC_REG_INT_MMU_PORT_TO_PHY_PORT_MAPPING_BCM56370_A0r
base 0x8110000 
acc_type (20 << SOC_REG_FLAG_ACC_TYPE_SHIFT),
soc_field_info_t soc_MMU_PORT_TO_PHY_PORT_MAPPING_BCM56960_A0r_fields[] = {
    { PHY_PORTf, 8, 0, SOCF_LE }
};
 */
//Register: MMU_PORT_TO_PHY_PORT_MAPPING.cpu0 port register address 0x08110046
//Blocks: mmu_glb0 (1 copy)
//Description: MMU port to Physical port number mapping
//Displaying: reset defaults, reset value 0xff mask 0xff
//  PHY_PORT<7:0> = 0xff
#define MMU_PORT_TO_PHY_PORT_MAPPINGr                         0x08110000

/*
SOC_REG_INT_MMU_PORT_TO_DEVICE_PORT_MAPPING_BCM56370_A0r
base 0x8100000
acc_type 20
soc_field_info_t soc_MMU_PORT_TO_DEVICE_PORT_MAPPINGr_fields[] = {
    { DEVICE_PORTf, 8, 0, SOCF_LE }
};
 */
//Register: MMU_PORT_TO_DEVICE_PORT_MAPPING.cpu0 port register address 0x08100046
//Blocks: mmu_glb0 (1 copy)
//Description: MMU port to DEVICE port number mapping
//Displaying: reset defaults, reset value 0xff mask 0xff
//  DEVICE_PORT<7:0> = 0xff
#define MMU_PORT_TO_DEVICE_PORT_MAPPINGr                      0x08100000


//Memory: SYS_PORTMAP.ipipe0 address 0x80f40000
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 256 with indices 0-255 (0x0-0xff), each 1 bytes 1 words
//Entry mask: 0x0000007f
//Description: System Port Mapping Table (maps local system port number to a local physical port number)
//  RESERVED_DEVICE_PORT_NUMBER<7>
//  DEVICE_PORT_NUMBER<6:0>

/*
soc_field_info_t soc_SYS_PORTMAP_BCM56370_A0m_fields[] = {
    { DEVICE_PORT_NUMBERf, 7, 0, SOCF_LE | SOCF_GLOBAL },
    { RESERVED_DEVICE_PORT_NUMBERf, 1, 7, SOCF_RES | SOCF_GLOBAL }
};
 */
typedef struct {
    uint32_t entry_data[1];
}sys_portmap_t;

#define SYS_PORTMAPm                                          0x80f40000
#define SYS_PORTMAPm_BYTES                                    1
#define SYS_PORTMAPm_MAX_INDEX                                255    




/*****************************************************************************************/
/*                              MAC related                                              */
/*****************************************************************************************/

typedef enum _mac_mode_e {
    SOC_MAC_MODE_10_100,                /* 10/100 Mb/s MAC selected */
    SOC_MAC_MODE_10,                    /* 10/100 Mb/s MAC in 10Mbps mode */
    SOC_MAC_MODE_1000_T,                /* 1000/TURBO MAC selected */
    SOC_MAC_MODE_10000,                 /* 10G MAC selected */
    SOC_MAC_MODE_100000                 /* 100G MAC selected */
} mac_mode_t;

//Register: COMMAND_CONFIG.ge0 port register address 0x00010200
//Flags:
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: Command register. Used by the host processor to control and configure the core
//Displaying: reset defaults, reset value 0x10000d8 mask 0x7fa7dfff
#define  COMMAND_CONFIGr                0x10200  // 0x10200 - 0x10207

/*
soc_field_info_t soc_COMMAND_CONFIG_BCM53400_A0r_fields[] = {
    { CNTL_FRM_ENAf, 1, 23, 0 },
    { CRC_FWDf, 1, 6, 0 },
    { ENA_EXT_CONFIGf, 1, 22, SOCF_RES },
    { EN_INTERNAL_TX_CRSf, 1, 21, 0 },
    { ETH_SPEEDf, 2, 2, SOCF_LE },
    { FCS_CORRUPT_URUN_ENf, 1, 14, 0 },
    { FD_TX_URUN_FIX_ENf, 1, 27, 0 },
    { HD_ENAf, 1, 10, 0 },
    { IGNORE_TX_PAUSEf, 1, 28, 0 },
    { LINE_LOOPBACKf, 1, 25, 0 },
    { LOOP_ENAf, 1, 15, 0 },
    { MAC_LOOP_CONf, 1, 16, 0 },
    { NO_LGTH_CHECKf, 1, 24, 0 },
    { OOB_EFC_ENf, 1, 29, 0 },
    { OVERFLOW_ENf, 1, 12, 0 },
    { PAD_ENf, 1, 5, 0 },
    { PAUSE_FWDf, 1, 7, 0 },
    { PAUSE_IGNOREf, 1, 8, 0 },
    { PROMIS_ENf, 1, 4, 0 },
    { RUNT_FILTER_DISf, 1, 30, 0 },
    { RX_ENAf, 1, 1, 0 },
    { RX_ERR_DISCf, 1, 26, 0 },
    { RX_LOW_LATENCY_ENf, 1, 11, 0 },
    { SW_OVERRIDE_RXf, 1, 18, 0 },
    { SW_OVERRIDE_TXf, 1, 17, 0 },
    { SW_RESETf, 1, 13, SOCF_RES },
    { TX_ADDR_INSf, 1, 9, 0 },
    { TX_ENAf, 1, 0, 0 }
};
*/
typedef union command_config_s {
    struct _command_config_ {
#if defined(LE_HOST)
    uint32_t  TX_ENAf:1,
              RX_ENAf:1,
              ETH_SPEEDf:2,
              PROMIS_ENf:1,
              PAD_ENf:1,
              CRC_FWDf:1,
              PAUSE_FWDf:1,
              PAUSE_IGNOREf:1,
              TX_ADDR_INSf:1,
              HD_ENAf:1,
              RX_LOW_LATENCY_ENf:1,
              OVERFLOW_ENf:1,
              SW_RESETf:1,
              FCS_CORRUPT_URUN_ENf:1,
              LOOP_ENAf:1,
              MAC_LOOP_CONf:1,
              SW_OVERRIDE_TXf:1,
              SW_OVERRIDE_RXf:1,
              r0:2,
              EN_INTERNAL_TX_CRSf:1,
              ENA_EXT_CONFIGf:1,
              CNTL_FRM_ENAf:1,
              NO_LGTH_CHECKf:1,
              LINE_LOOPBACKf:1,
              RX_ERR_DISCf:1,
              FD_TX_URUN_FIX_ENf:1,
              IGNORE_TX_PAUSEf:1,
              OOB_EFC_ENf:1,
              RUNT_FILTER_DISf:1,
              r1:1;

#else
    uint32_t  r1:1,
              RUNT_FILTER_DISf:1,
              OOB_EFC_ENf:1,
              IGNORE_TX_PAUSEf:1,
              FD_TX_URUN_FIX_ENf:1,
              RX_ERR_DISCf:1,
              LINE_LOOPBACKf:1,
              NO_LGTH_CHECKf:1,
              CNTL_FRM_ENAf:1,
              ENA_EXT_CONFIGf:1,
              EN_INTERNAL_TX_CRSf:1,
              r0:2,
              SW_OVERRIDE_RXf:1,
              SW_OVERRIDE_TXf:1,
              MAC_LOOP_CONf:1,
              LOOP_ENAf:1,
              FCS_CORRUPT_URUN_ENf:1,
              SW_RESETf:1,
              OVERFLOW_ENf:1,
              RX_LOW_LATENCY_ENf:1,
              HD_ENAf:1,
              TX_ADDR_INSf:1,
              PAUSE_IGNOREf:1,
              PAUSE_FWDf:1,
              CRC_FWDf:1,
              PAD_ENf:1,
              PROMIS_ENf:1,
              ETH_SPEEDf:2,
              RX_ENAf:1,
              TX_ENAf:1;
#endif
    }reg;
    uint32_t word;
} command_config_t;





#define  CLMAC_RX_CTRLr                 0x60600
#define  XLPORT_ENABLE_REG              0x2020b00

#define  CLPORT_CONFIG                  0x20000
#define  CLPORT_MODE_REG                0x2020a00
#define  CLPORT_ENABLE_REG              0x2020b00


//Register: FLUSH_CONTROL.ge0 port register address 0x0001cd00
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
//Description: Flush enable control register
//Displaying: reset defaults, reset value 0 mask 0
#define FLUSH_CONTROLr                  0x0001cd00

/*****************************************************************************************/
/*                              FLEXPORT related                                         */
/*****************************************************************************************/

/*
soc_field_info_t soc_IDB_OBM0_Q_CONTROLr_fields[] = {
    { PORT0_BUBBLE_MOP_DISABLEf, 1, 16, SOCF_RES },
    { PORT0_BYPASS_ENABLEf, 1, 1, SOCF_RES },
    { PORT0_CA_SOPf, 1, 20, SOCF_RES },
    { PORT0_FLUSHf, 1, 2, SOCF_RES },
    { PORT0_OVERSUB_ENABLEf, 1, 0, SOCF_RES },
    { PORT0_RESETf, 1, 3, SOCF_RES },
    { PORT1_BUBBLE_MOP_DISABLEf, 1, 17, SOCF_RES },
    { PORT1_BYPASS_ENABLEf, 1, 5, SOCF_RES },
    { PORT1_CA_SOPf, 1, 21, SOCF_RES },
    { PORT1_FLUSHf, 1, 6, SOCF_RES },
    { PORT1_OVERSUB_ENABLEf, 1, 4, SOCF_RES },
    { PORT1_RESETf, 1, 7, SOCF_RES },
    { PORT2_BUBBLE_MOP_DISABLEf, 1, 18, SOCF_RES },
    { PORT2_BYPASS_ENABLEf, 1, 9, SOCF_RES },
    { PORT2_CA_SOPf, 1, 22, SOCF_RES },
    { PORT2_FLUSHf, 1, 10, SOCF_RES },
    { PORT2_OVERSUB_ENABLEf, 1, 8, SOCF_RES },
    { PORT2_RESETf, 1, 11, SOCF_RES },
    { PORT3_BUBBLE_MOP_DISABLEf, 1, 19, SOCF_RES },
    { PORT3_BYPASS_ENABLEf, 1, 13, SOCF_RES },
    { PORT3_CA_SOPf, 1, 23, SOCF_RES },
    { PORT3_FLUSHf, 1, 14, SOCF_RES },
    { PORT3_OVERSUB_ENABLEf, 1, 12, SOCF_RES },
    { PORT3_RESETf, 1, 15, SOCF_RES }
};
*/
typedef union obm_q_control_s {
    struct _obm_q_control_ {
#if defined(LE_HOST)
    uint32_t  PORT0_OVERSUB_ENABLEf:1,
              PORT0_BYPASS_ENABLEf:1,
              PORT0_FLUSHf:1,
              PORT0_RESETf:1,
              PORT1_OVERSUB_ENABLEf:1,
              PORT1_BYPASS_ENABLEf:1,
              PORT1_FLUSHf:1,
              PORT1_RESETf:1,
              PORT2_OVERSUB_ENABLEf:1,
              PORT2_BYPASS_ENABLEf:1,
              PORT2_FLUSHf:1,
              PORT2_RESETf:1,
              PORT3_OVERSUB_ENABLEf:1,
              PORT3_BYPASS_ENABLEf:1,
              PORT3_FLUSHf:1,
              PORT3_RESETf:1,
              PORT0_BUBBLE_MOP_DISABLEf:1,  
              PORT1_BUBBLE_MOP_DISABLEf:1,
              PORT2_BUBBLE_MOP_DISABLEf:1,
              PORT3_BUBBLE_MOP_DISABLEf:1,
              PORT0_CA_SOPf:1,
              PORT1_CA_SOPf:1,
              PORT2_CA_SOPf:1,
              PORT3_CA_SOPf:1,
              r0:8;
#else
    uint32_t  r0:8,
              PORT3_CA_SOPf:1,
              PORT2_CA_SOPf:1,
              PORT1_CA_SOPf:1,
              PORT0_CA_SOPf:1,
              PORT3_BUBBLE_MOP_DISABLEf:1,
              PORT2_BUBBLE_MOP_DISABLEf:1,
              PORT1_BUBBLE_MOP_DISABLEf:1,
              PORT0_BUBBLE_MOP_DISABLEf:1,  
              PORT3_RESETf:1,
              PORT3_FLUSHf:1,
              PORT3_BYPASS_ENABLEf:1,
              PORT3_OVERSUB_ENABLEf:1,
              PORT2_RESETf:1,
              PORT2_FLUSHf:1,
              PORT2_BYPASS_ENABLEf:1,
              PORT2_OVERSUB_ENABLEf:1,
              PORT1_RESETf:1,
              PORT1_FLUSHf:1,
              PORT1_BYPASS_ENABLEf:1,
              PORT1_OVERSUB_ENABLEf:1,
              PORT0_RESETf:1,
              PORT0_FLUSHf:1,
              PORT0_BYPASS_ENABLEf:1,
              PORT0_OVERSUB_ENABLEf:1;
#endif
    }reg;
    uint32_t word;
} obm_q_control_t;

#define  IDB_OBM0_Q_CONTROLr            0x0a000100  // 0x0a000200 0x0a000300 0x0a000400
#define  IDB_OBM1_Q_CONTROLr            0x0e000100
#define  IDB_OBM2_Q_CONTROLr            0x12000100
#define  IDB_OBM0_CONTROLr              0x16000000
#define  IDB_OBM1_CONTROLr              0x1a000000
#define  IDB_OBM2_CONTROLr              0x1e000000
#define  IDB_OBM3_CONTROLr              0x22000000
#define  IDB_OBM0_48_CONTROLr           0x26000000
#define  IDB_OBM1_48_CONTROLr           0x2a000000
#define  IDB_OBM2_48_CONTROLr           0x2e000000


/*
soc_field_info_t soc_IDB_OBM0_Q_CA_CONTROLr_fields[] = {
    { PORT0_RESETf, 1, 0, SOCF_RES },
    { PORT10_RESETf, 1, 10, SOCF_RES },
    { PORT11_RESETf, 1, 11, SOCF_RES },
    { PORT12_RESETf, 1, 12, SOCF_RES },
    { PORT13_RESETf, 1, 13, SOCF_RES },
    { PORT14_RESETf, 1, 14, SOCF_RES },
    { PORT15_RESETf, 1, 15, SOCF_RES },
    { PORT1_RESETf, 1, 1, SOCF_RES },
    { PORT2_RESETf, 1, 2, SOCF_RES },
    { PORT3_RESETf, 1, 3, SOCF_RES },
    { PORT4_RESETf, 1, 4, SOCF_RES },
    { PORT5_RESETf, 1, 5, SOCF_RES },
    { PORT6_RESETf, 1, 6, SOCF_RES },
    { PORT7_RESETf, 1, 7, SOCF_RES },
    { PORT8_RESETf, 1, 8, SOCF_RES },
    { PORT9_RESETf, 1, 9, SOCF_RES },
    { PORT_MODEf, 4, 16, SOCF_LE }
};
*/

typedef union obm_q_ca_control_s {
    struct _obm_q_ca_control_ {
#if defined(LE_HOST)
    uint32_t  PORT0_RESETf:1,
              PORT1_RESETf:1,
              PORT2_RESETf:1,
              PORT3_RESETf:1,
              PORT4_RESETf:1,
              PORT5_RESETf:1,
              PORT6_RESETf:1,
              PORT7_RESETf:1,
              PORT8_RESETf:1,
              PORT9_RESETf:1,
              PORT10_RESETf:1,
              PORT11_RESETf:1,
              PORT12_RESETf:1,
              PORT13_RESETf:1,
              PORT14_RESETf:1,
              PORT15_RESETf:1,  
              PORT_MODEf:4,
              r0:12;
#else
    uint32_t  r0:8,
              PORT_MODEf:4,
              PORT15_RESETf:1, 
              PORT14_RESETf:1,
              PORT13_RESETf:1,
              PORT12_RESETf:1,
              PORT11_RESETf:1,
              PORT10_RESETf:1,
              PORT9_RESETf:1,
              PORT8_RESETf:1,
              PORT7_RESETf:1,
              PORT6_RESETf:1,
              PORT5_RESETf:1,
              PORT4_RESETf:1,
              PORT3_RESETf:1,
              PORT2_RESETf:1,
              PORT1_RESETf:1,
              PORT0_RESETf:1;
#endif
    }reg;
    uint32_t word;
} obm_q_ca_control_t;

#define  IDB_OBM0_Q_CA_CONTROLr         0x0a006d00
#define  IDB_OBM1_Q_CA_CONTROLr         0x0e006d00
#define  IDB_OBM2_Q_CA_CONTROLr         0x12006d00
#define  IDB_OBM0_CA_CONTROLr           0x16006900
#define  IDB_OBM1_CA_CONTROLr           0x1a006900
#define  IDB_OBM2_CA_CONTROLr           0x1e006900
#define  IDB_OBM3_CA_CONTROLr           0x22006900
#define  IDB_OBM0_48_CA_CONTROLr        0x26006900
#define  IDB_OBM1_48_CA_CONTROLr        0x2a006900
#define  IDB_OBM2_48_CA_CONTROLr        0x2e006900

/*
soc_field_info_t soc_IDB_CA_CPU_CONTROLr_fields[] = {
    { PORT_RESETf, 1, 0, SOCF_RES },
    { RESERVED_BUBBLE_MOP_DISABLEf, 1, 1, SOCF_RES }
};
*/

typedef union idb_lpbk_ca_s {
    struct _idb_lpbk_ca_ {
#if defined(LE_HOST)
    uint32_t  PORT_RESETf:1,
              RESERVED_BUBBLE_MOP_DISABLEf:1,
              r0:30;
#else
    uint32_t  r0:30,
              RESERVED_BUBBLE_MOP_DISABLEf:1,
              PORT_RESETf:1;
#endif
    }reg;
    uint32_t word;
} idb_lpbk_ca_t;

//Register: IDB_CA_LPBK_CONTROL.ipipe0 alias IDB_CA_LPBK_CONTROL general register address 0x02000700
//Flags:
//Blocks: ipipe0 (1 copy)
//Description: IDB LPBK-CA Control Register
#define IDB_CA_LPBK_CONTROLr      0x02000700
 
typedef idb_lpbk_ca_t idb_ca_cpu_t;

//Register: IDB_CA_CPU_CONTROL.ipipe0 alias IDB_CA_CPU_CONTROL general register address 0x02000200
//Flags:
//Blocks: ipipe0 (1 copy)
//Description: IDB CPU-CA Control Register
#define IDB_CA_CPU_CONTROLr       0x02000200


//Register: IDB_CA_BSK_CONTROL.ipipe0 alias IDB_CA_BSK_CONTROL general register address 0x02000300
//Flags:
//Blocks: ipipe0 (1 copy)
//Description: IDB BSK-CA Control Register
#define IDB_CA_BSK_CONTROLr       0x02000300

/*****************************************************************************************/
/*                             MMU                                                       */
/*****************************************************************************************/

//Register: MMU_GCFG_MISCCONFIG.mmu_glb0 general register address 0x0a000000
//Flags:
//Blocks: mmu_glb0 (1 copy)
//Description: Controls various functions in the MMU
//acc_type = 20

/*
soc_field_info_t soc_MMU_GCFG_MISCCONFIG_BCM56970_A0r_fields[] = {
    { BST_CLEAR_ON_READ_IN_MAX_USE_COUNT_MODE_ENABLEf, 1, 8, 0 },
    { BST_TRACKING_MODEf, 1, 3, 0 },
    { INIT_MEMf, 1, 1, 0 },
    { PARITY_ENf, 1, 2, 0 },
    { REFRESH_ENf, 1, 0, 0 },
    { RESERVEDf, 3, 5, SOCF_LE|SOCF_RES },
    { SBUS_SPLIT_ERR_CHK_OVERRIDEf, 1, 4, 0 }
};
 */
typedef union mmu_gcfg_miscconfig_reg_s {
    struct _mmu_gcfg_miscconfig_reg_ {
#if defined(LE_HOST)
    uint32_t  REFRESH_ENf:1,
              INIT_MEMf:1,
              PARITY_ENf:1,
              BST_TRACKING_MODEf:1,
              SBUS_SPLIT_ERR_CHK_OVERRIDEf:1,
              RESERVEDf:3,
              BST_CLEAR_ON_READ_IN_MAX_USE_COUNT_MODE_ENABLEf:1,
              r0:23;
#else
    uint32_t  r0:23,
              BST_CLEAR_ON_READ_IN_MAX_USE_COUNT_MODE_ENABLEf:1,
              RESERVEDf:3,
              SBUS_SPLIT_ERR_CHK_OVERRIDEf:1,
              BST_TRACKING_MODEf:1,
              PARITY_ENf:1,
              INIT_MEMf:1,
              REFRESH_ENf:1;
#endif
    }reg;
    uint32_t word;
} mmu_gcfg_miscconfig_reg_t;

#define MMU_GCFG_MISCCONFIGr             0x0a000000



/*****************************************************************************************/
/*                              L2  related                                              */
/*****************************************************************************************/
/*

*/

typedef union ing_config64_s {
    struct _ing_config64_ {
#if defined(LE_HOST)
    uint32_t  PORT_RESETf:1,
              RESERVED_BUBBLE_MOP_DISABLEf:1,
              r0:30;
#else
    uint32_t  r0:30,
              RESERVED_BUBBLE_MOP_DISABLEf:1,
              PORT_RESETf:1;
#endif
    }reg;
    uint64_t dword;
} ing_config64_t;

//SOC_REG_FLAG_CCH
#define ING_CONFIG_64r                 0x4e018000

#define BCM_MAC_IS_MCAST(_mac_)  \
    (_mac_[0] & 0x1) 

#define BCM_MAC_IS_ZERO(_mac_)  \
    (((_mac_)[0] | (_mac_)[1] | (_mac_)[2] | \
      (_mac_)[3] | (_mac_)[4] | (_mac_)[5]) == 0) 

#define BCM_VLAN_NONE           (0x0000) 
#define BCM_VLAN_ALL            (0xffff) 
#define BCM_VLAN_DEFAULT        (0x0001) 
#define BCM_VLAN_INVALID        (0x1000) 

#define BCM_VLAN_VALID(id)      \
    ((id) >= BCM_VLAN_DEFAULT && \
     (id) < BCM_VLAN_INVALID) 

#define BCM_VLAN_CTRL(prio, cfi, id)  \
    (((prio) & 0x007) << 13 | \
     ((cfi ) & 0x001) << 12 | \
     ((id  ) & 0xfff) << 0) 

#define BCM_VLAN_CTRL_PRIO(c)   ((c) >> 13 & 0x007) 
#define BCM_VLAN_CTRL_CFI(c)    ((c) >> 12 & 0x001) 
#define BCM_VLAN_CTRL_ID(c)     ((c) >>  0 & 0xfff) 

/* Flags for device-independent L2 cache address. */
#define BCM_L2_CACHE_CPU            0x00000001 /* Packet is copied to CPU. */
#define BCM_L2_CACHE_DISCARD        0x00000002 /* Packet is not switched. */
#define BCM_L2_CACHE_MIRROR         0x00000004 /* Packet is mirrored. */
#define BCM_L2_CACHE_L3             0x00000008 /* Packet is to be L3 routed. */
#define BCM_L2_CACHE_BPDU           0x00000010 /* Packet is BPDU. */
#define BCM_L2_CACHE_SETPRI         0x00000020 /* Internal prio from prio field. */
#define BCM_L2_CACHE_TRUNK          0x00000040 /* Destination is a trunk. */
#define BCM_L2_CACHE_REMOTE_LOOKUP  0x00000080 /* Remote L2 lookup requested. */
#define BCM_L2_CACHE_LEARN_DISABLE  0x00000100 /* Packet source address is not
                                                  learned for this destination
                                                  address. */
#define BCM_L2_CACHE_TUNNEL         0x00000200 /* Tunnel termination address. */
#define BCM_L2_CACHE_DESTPORTS      0x00000400 /* Packet is forwarded by
                                                  multiport L2 address. */
#define BCM_L2_CACHE_SUBTYPE        0x00000800 /* Slow protocol subtype to
                                                  match. */
#define BCM_L2_CACHE_LOOKUP         0x00001000 /* L2 lookup requested. */
#define BCM_L2_CACHE_MULTICAST      0x00002000 /* Destination is (flood)
                                                  multicast group. */
#define BCM_L2_CACHE_PROTO_PKT      0x00004000 /* Packet is protocol packets. */

/* Adjust justification for uint32 writes to fields */
/* dst is an array name of type uint32 [] */
#define MAC_ADDR_TO_UINT32(mac, dst) do {  \
    (dst)[0] = (((uint32)(mac)[2]) << 24 | \
                ((uint32)(mac)[3]) << 16 | \
                ((uint32)(mac)[4]) << 8 |  \
                ((uint32)(mac)[5]));       \
    (dst)[1] = (((uint32)(mac)[0]) << 8 |  \
                ((uint32)(mac)[1]));       \
} while (0)

/* 
 * Flags for learn mode
 * 
 * This call takes flags to turn on and off mutually-independent actions
 * that should be taken when a packet is received with an unknown source
 * address, or source lookup failure (SLF).
 * 
 * The set call returns BCM_E_UNAVAIL for flag combinations that are not
 * supported by the hardware.
 */
#define BCM_PORT_LEARN_ARL      0x01       /* Learn SLF address. */
#define BCM_PORT_LEARN_CPU      0x02       /* Copy SLF packet to CPU. */
#define BCM_PORT_LEARN_FWD      0x04       /* Forward SLF packet */
#define BCM_PORT_LEARN_PENDING  0x08       /* Mark learned SLF as pending */
#define BCM_PORT_LEARN_ON       0x01       /* Deprecated name */
#define BCM_PORT_LEARN_SWITCH   0x04       /* Deprecated name */

typedef enum _shr_port_encap_e {
    SOC_ENCAP_IEEE = 0,               /* IEEE 802.3 Ethernet-II  */
    SOC_ENCAP_HIGIG = 1,              /* HIGIG Header mode       */
    SOC_ENCAP_B5632 = 2,              /* BCM5632 Header mode     */
    SOC_ENCAP_HIGIG2 = 3,             /* HIGIG2 Header mode      */
    SOC_ENCAP_HIGIG2_LITE = 4,        /* HIGIG2 Header mode (Raptor style) */
    SOC_ENCAP_HIGIG2_L2 = 5,          /* HIGIG2 L2 Tunnel mode   */
    SOC_ENCAP_HIGIG2_IP_GRE = 6,      /* HIGIG2 L3 (IP-GRE) Tunnel mode    */
    SOC_ENCAP_OBSOLETE = 7,           /* Obsolete header mode         */
    SOC_ENCAP_PREAMBLE_SOP_ONLY = 8,  /* 1B preamble mode        */
    SOC_ENCAP_HIGIG_OVER_ETHERNET = 9,/* HIGIG over Ethernet     */
    SOC_ENCAP_CPRI = 10 ,             /* CPRI  radio port encap*/
    SOC_ENCAP_RSVD4 = 11 ,            /* RSVD4 radio port encap*/
    SOC_ENCAP_HIGIG3 = 12,            /* HIGIG3 Header mode      */
    SOC_ENCAP_COUNT                   /* last, please */
} bcm_port_encap_t;

/* Port loopback modes. */
typedef enum bcm_port_loopback_e {
    BCM_PORT_LOOPBACK_NONE = 0, 
    BCM_PORT_LOOPBACK_MAC  = 1, 
    BCM_PORT_LOOPBACK_PHY  = 2, 
    BCM_PORT_LOOPBACK_PHY_REMOTE = 3, 
    BCM_PORT_LOOPBACK_MAC_REMOTE = 4, 
    BCM_PORT_LOOPBACK_EDB  = 5, 
    BCM_PORT_LOOPBACK_COUNT = 6 
} bcm_port_loopback_t;

typedef enum bcm_stg_stp_e {
    BCM_STG_STP_DISABLE = 0, /* Disabled. */
    BCM_STG_STP_BLOCK   = 1, /* BPDUs/no learns. */
    BCM_STG_STP_LISTEN  = 2, /* BPDUs/no learns. */
    BCM_STG_STP_LEARN   = 3, /* BPDUs/learns. */
    BCM_STG_STP_FORWARD = 4, /* Normal operation. */
    BCM_STG_STP_COUNT   = 5 
} bcm_stg_stp_t;

typedef enum bcm_linkscan_mode_e {
    BCM_LINKSCAN_MODE_NONE     = 0, 
    BCM_LINKSCAN_MODE_SW       = 1, 
    BCM_LINKSCAN_MODE_HW       = 2, 
    BCM_LINKSCAN_MODE_OVERRIDE = 3, 
    BCM_LINKSCAN_MODE_COUNT    = 4 
} bcm_linkscan_mode_t;

/* bcm_port_discard_e */
typedef enum bcm_port_discard_e {
    BCM_PORT_DISCARD_NONE  = 0, 
    BCM_PORT_DISCARD_ALL   = 1, 
    BCM_PORT_DISCARD_UNTAG = 2, 
    BCM_PORT_DISCARD_TAG   = 3, 
    BCM_PORT_DISCARD_INGRESS = 4, 
    BCM_PORT_DISCARD_EGRESS = 5, 
    BCM_PORT_DISCARD_COUNT = 6 
} bcm_port_discard_t;

#define _E2S(x, max, string_array) \
    ((((size_t)(x) < (max))) ?  (string_array)[(x)] : "?")


#define ENCAP_MODE(x)      _E2S(x, COUNTOF(encap_mode), encap_mode)
#define LOOPBACK_MODE(x)   _E2S(x, BCM_PORT_LOOPBACK_COUNT, loopback_mode)
#define LINKSCAN_MODE(x)   _E2S(x, BCM_LINKSCAN_MODE_COUNT, linkscan_mode)
#define FORWARD_MODE(x)	   _E2S(x, BCM_STG_STP_COUNT, forward_mode)
#define DISCARD_MODE(x)    _E2S(x, BCM_PORT_DISCARD_COUNT, discard_mode)

/* Device-independent L2 cache address structure. */
typedef struct bcm_l2_cache_addr_s {
    uint32_t   flags;               /* BCM_L2_CACHE_xxx flags.               */
    uint32_t   station_flags;       /* BCM_L2_STATION_xxx flags.             */
    bcm_mac_t     mac;                 /* Destination MAC address to match.     */
    bcm_mac_t     mac_mask;            /* MAC address mask.                     */
    uint16_t   vlan;                /* VLAN to match.                        */
    uint16_t   vlan_mask;           /* VLAN mask.                            */
    int        src_port;            /* Ingress port to match (BCM5660x).     */
    int        src_port_mask;       /* Ingress port mask (must be 0 if not
                                       BCM5660x).                            */
    int        dest_modid;          /* Switch destination module ID.         */
    int        dest_port;           /* Switch destination port.              */
    int        dest_trunk;          /* Switch destination trunk ID.          */
    int        prio;                /* Internal priority, use -1 to not set. */
    uint32_t   dest_ports;          /* Destination ports for Multiport L2
                                       address forwarding.                   */
    int        lookup_class;        /* Classification class ID.              */
    uint8_t    subtype;             /* Slow protocol subtype to match.       */
    int        encap_id;            /* Encapsulation index.                  */
    int        group;               /* Flood domain for L2CP.                */
    uint16_t   ethertype;           /* EtherType to match.                   */
    uint16_t   ethertype_mask;      /* Mask.                                 */
} bcm_l2_cache_addr_t;



/* VPN types */
#define BCM_VPN_TYPE_MPLS_L3    1          
#define BCM_VPN_TYPE_MPLS_VPWS  3          
#define BCM_VPN_TYPE_MPLS_VPLS  7          
#define BCM_VPN_TYPE_VXLAN      7          
#define BCM_VPN_TYPE_L2GRE      7          
#define BCM_VPN_TYPE_MIM        7          
#define BCM_VPN_TYPE_FLOW       7          

/* Set VPN ID */
#define BCM_VPN_MPLS_L3_VPN_ID_SET(_vpn, _id)  \
        ((_vpn) = (((_id) & 0x1fff) + (BCM_VPN_TYPE_MPLS_L3 << 12))) 
#define BCM_VPN_MPLS_VPWS_VPN_ID_SET(_vpn, _id)  \
        ((_vpn) = (((_id) & 0x3fff) + (BCM_VPN_TYPE_MPLS_VPWS << 12))) 
#define BCM_VPN_MPLS_VPLS_VPN_ID_SET(_vpn, _id)  \
        ((_vpn) = ((_id) & 0x8000) ? \
        ((((_id) & 0xfff) | 0x8000) + (BCM_VPN_TYPE_MPLS_VPLS << 12)) : \
        (((_id) & 0x7fff) + (BCM_VPN_TYPE_MPLS_VPLS << 12))) 
#define BCM_VPN_VXLAN_VPN_ID_SET(_vpn, _id)  \
        ((_vpn) = ((_id) & 0x8000) ? \
        ((((_id) & 0xfff) | 0x8000) + (BCM_VPN_TYPE_VXLAN << 12)) : \
        (((_id) & 0x7fff) + (BCM_VPN_TYPE_VXLAN << 12))) 
#define BCM_VPN_L2GRE_VPN_ID_SET(_vpn, _id)  \
        ((_vpn) = ((_id) & 0x8000) ? \
        ((((_id) & 0xfff) | 0x8000) + (BCM_VPN_TYPE_L2GRE << 12)) : \
        (((_id) & 0x7fff) + (BCM_VPN_TYPE_L2GRE << 12))) 
#define BCM_VPN_MIM_VPN_ID_SET(_vpn, _id)  \
        ((_vpn) = ((_id) & 0x8000) ? \
        ((((_id) & 0xfff) | 0x8000) + (BCM_VPN_TYPE_MIM << 12)) : \
        (((_id) & 0x7fff) + (BCM_VPN_TYPE_MIM << 12))) 
#define BCM_VPN_FLOW_VPN_ID_SET(_vpn, _id)  \
        ((_vpn) = ((_id) & 0x8000) ? \
        ((((_id) & 0xfff) | 0x8000) + (BCM_VPN_TYPE_FLOW << 12)) : \
        (((_id) & 0x7fff) + (BCM_VPN_TYPE_FLOW << 12))) 

/* Get Real ID */
#define BCM_VPN_MPLS_L3_VPN_ID_GET(_vpn, _id)  \
        ((_id) = ((_vpn) - (BCM_VPN_TYPE_MPLS_L3 << 12))) 
#define BCM_VPN_MPLS_VPWS_VPN_ID_GET(_vpn, _id)  \
        ((_id) = ((_vpn) - (BCM_VPN_TYPE_MPLS_VPWS << 12))) 
#define BCM_VPN_MPLS_VPLS_VPN_ID_GET(_vpn, _id)  \
        ((_id) = ((_vpn) - (BCM_VPN_TYPE_MPLS_VPLS << 12))) 
#define BCM_VPN_VXLAN_VPN_ID_GET(_vpn, _id)  \
        ((_id) = ((_vpn) - (BCM_VPN_TYPE_VXLAN << 12))) 
#define BCM_VPN_L2GRE_VPN_ID_GET(_vpn, _id)  \
        ((_id) = ((_vpn) - (BCM_VPN_TYPE_L2GRE << 12))) 
#define BCM_VPN_MIM_VPN_ID_GET(_vpn, _id)  \
        ((_id) = ((_vpn) - (BCM_VPN_TYPE_MIM << 12))) 
#define BCM_VPN_FLOW_VPN_ID_GET(_vpn, _id)  \
        ((_id) = ((_vpn) - (BCM_VPN_TYPE_FLOW << 12))) 

#define _BCM_VPN_TYPE_L3        BCM_VPN_TYPE_MPLS_L3
#define _BCM_VPN_TYPE_VPWS      BCM_VPN_TYPE_MPLS_VPWS
#define _BCM_VPN_TYPE_VFI       BCM_VPN_TYPE_MPLS_VPLS


#define _BCM_VPN_SET(_vpn_, _type_, _id_) \
        do { \
            if (BCM_VPN_TYPE_MPLS_L3 == (_type_)) { \
                BCM_VPN_MPLS_L3_VPN_ID_SET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_MPLS_VPWS == (_type_)) { \
                BCM_VPN_MPLS_VPWS_VPN_ID_SET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_MPLS_VPLS == (_type_)) { \
                BCM_VPN_MPLS_VPLS_VPN_ID_SET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_VXLAN == (_type_)) { \
                BCM_VPN_VXLAN_VPN_ID_SET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_L2GRE == (_type_)) { \
                BCM_VPN_L2GRE_VPN_ID_SET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_MIM == (_type_)) { \
                BCM_VPN_MIM_VPN_ID_SET(_vpn_, _id_); \
            } \
        } while (0)

#define _BCM_VPN_GET(_id_, _type_,  _vpn_) \
        do { \
            if (BCM_VPN_TYPE_MPLS_L3 == (_type_)) { \
                BCM_VPN_MPLS_L3_VPN_ID_GET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_MPLS_VPWS == (_type_)) { \
                BCM_VPN_MPLS_VPWS_VPN_ID_GET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_MPLS_VPLS == (_type_)) { \
                BCM_VPN_MPLS_VPLS_VPN_ID_GET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_VXLAN == (_type_)) { \
                BCM_VPN_VXLAN_VPN_ID_GET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_L2GRE == (_type_)) { \
                BCM_VPN_L2GRE_VPN_ID_GET(_vpn_, _id_); \
            } else if (BCM_VPN_TYPE_MIM == (_type_)) { \
                BCM_VPN_MIM_VPN_ID_GET(_vpn_, _id_); \
            } \
        } while (0)

#define _BCM_VPN_IS_L3(_vpn_) \
        (((((_vpn_) >> 12) & 0xf) >= BCM_VPN_TYPE_MPLS_L3) \
        && ((((_vpn_) >> 12) & 0xf) < BCM_VPN_TYPE_MPLS_VPWS))

#define _BCM_VPN_IS_VPLS(_vpn_) \
        ((((_vpn_) >> 12) & 0xf) >= BCM_VPN_TYPE_MPLS_VPLS)

#define _BCM_VPN_IS_VPWS(_vpn_) \
        (((((_vpn_) >> 12) & 0xf) >= BCM_VPN_TYPE_MPLS_VPWS) \
        && ((((_vpn_) >> 12) & 0xf) < BCM_VPN_TYPE_MPLS_VPLS))

#define _BCM_VPN_IS_MIM(_vpn_) \
        ((((_vpn_) >> 12) & 0xf) >= BCM_VPN_TYPE_MIM)

#define _BCM_VPN_IS_L2GRE_ELINE(_vpn_) \
        ((((_vpn_) >> 12) & 0xf) >= BCM_VPN_TYPE_L2GRE)

#define _BCM_VPN_IS_L2GRE_ELAN(_vpn_) \
        ((((_vpn_) >> 12) & 0xf) >= BCM_VPN_TYPE_L2GRE)

#define _BCM_VPN_IS_VXLAN_ELINE(_vpn_) \
        ((((_vpn_) >> 12) & 0xf) >= BCM_VPN_TYPE_VXLAN)

#define _BCM_VPN_IS_VXLAN_ELAN(_vpn_) \
        ((((_vpn_) >> 12) & 0xf) >= BCM_VPN_TYPE_VXLAN)

#define _BCM_VPN_VFI_IS_SET(_vpn_) \
        (_BCM_VPN_IS_VPLS(_vpn_) \
        || _BCM_VPN_IS_MIM(_vpn_) \
        || _BCM_VPN_IS_L2GRE_ELINE(_vpn_) \
        || _BCM_VPN_IS_L2GRE_ELAN(_vpn_) \
        || _BCM_VPN_IS_VXLAN_ELINE(_vpn_) \
        || _BCM_VPN_IS_VXLAN_ELAN(_vpn_))

#define _BCM_VPN_IS_SET(_vpn_) \
        (_BCM_VPN_IS_L3(_vpn_) \
        || _BCM_VPN_IS_VPWS(_vpn_) \
        || _BCM_VPN_VFI_IS_SET(_vpn_))


#define VLAN_CHK_ID(vid) do { \
        if (vid > 4095) return SOC_E_PARAM; \
        } while (0)        


#define _SHR_GPORT_TYPE_SHIFT                           26
#define _SHR_GPORT_TYPE_MASK                            0x3f
#define _SHR_GPORT_TYPE_MAX                             51

#define BCM_GPORT_IS_SET(_gport)    \
        (((((_gport) >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK) > 0) && \
         ((((_gport) >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK) <= _SHR_GPORT_TYPE_MAX))


/*
#if defined(BCM_56370_A0)
soc_field_info_t soc_L2_USER_ENTRY_BCM56370_A0m_fields[] = {
    { BPDUf, 1, 206, 0 | SOCF_GLOBAL },
    { CLASS_IDf, 10, 196, SOCF_LE | SOCF_GLOBAL },
    { CPUf, 1, 175, 0 | SOCF_GLOBAL },
    { DESTINATIONf, 18, 177, SOCF_LE | SOCF_GLOBAL },
    { DO_NOT_LEARN_MACSAf, 1, 195, 0 | SOCF_GLOBAL },
    { DST_DISCARDf, 1, 176, 0 | SOCF_GLOBAL },
    { DUMMY_0f, 1, 61, 0 | SOCF_GLOBAL },
    { DUMMY_1f, 1, 62, 0 | SOCF_GLOBAL },
    { ECCf, 6, 208, SOCF_LE | SOCF_GLOBAL },
    { ECCPf, 7, 208, SOCF_LE | SOCF_GLOBAL },
    { ECC_DATAf, 39, 169, SOCF_LE | SOCF_GLOBAL },
    { KEYf, 80, 1, SOCF_LE | SOCF_GLOBAL },
    { KEY_TYPEf, 1, 63, 0 | SOCF_GLOBAL },
    { KEY_TYPE_MASKf, 1, 143, 0 | SOCF_GLOBAL },
    { L2_PROTOCOL_PKTf, 1, 207, 0 | SOCF_GLOBAL },
    { MAC_ADDRf, 48, 1, SOCF_LE | SOCF_GLOBAL },
    { MAC_ADDR_MASKf, 48, 81, SOCF_LE | SOCF_GLOBAL },
    { MASKf, 80, 81, SOCF_LE | SOCF_GLOBAL },
    { PARITYf, 1, 214, 0 | SOCF_GLOBAL },
    { PRIf, 4, 169, SOCF_LE | SOCF_GLOBAL },
    { RESERVED_0f, 1, 173, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_KEYf, 17, 64, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_MASKf, 17, 144, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RPEf, 1, 174, 0 | SOCF_GLOBAL },
    { TCAM_PARITY_KEYf, 4, 161, SOCF_LE | SOCF_GLOBAL },
    { TCAM_PARITY_MASKf, 4, 165, SOCF_LE | SOCF_GLOBAL },
    { VALIDf, 1, 0, 0 | SOCF_GLOBAL },
    { VFIf, 12, 49, SOCF_LE | SOCF_GLOBAL },
    { VFI_MASKf, 12, 129, SOCF_LE | SOCF_GLOBAL },
    { VLAN_IDf, 12, 49, SOCF_LE | SOCF_GLOBAL },
    { VLAN_ID_MASKf, 12, 129, SOCF_LE | SOCF_GLOBAL }
};
#endif
*/    
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 512 with indices 0-511 (0x0-0x1ff), each 27 bytes 7 words
#define L2_USER_ENTRYm                  0x68400000
#define L2_USER_ENTRYm_BYTES            27

typedef struct {
    uint32_t entry_data[28];
}l2u_entry_t;
    


typedef struct _bcm_gport_dest_s {
    bcm_port_t      port;
    bcm_module_t    modid;
    bcm_trunk_t     tgid;
    int             mpls_id;
    int             mim_id;
    int             wlan_id;
    int             trill_id;
    int             l2gre_id;
    int             vxlan_id;
    int             flow_id;
    int             vlan_vp_id;
    int             niv_id;
    int             extender_id;
    int             subport_id;
    int             scheduker_id;
    uint32_t          gport_type;
} _bcm_gport_dest_t;

/* 
 * Flags for device-independent L2 address.
 * 
 * Special note on insert/delete/lookup-specific flags:
 * 
 *   BCM_L2_NATIVE
 *   BCM_L2_MOVE
 *   BCM_L2_FROM_NATIVE
 *   BCM_L2_TO_NATIVE
 *   BCM_L2_MOVE_PORT
 *   BCM_L2_LOCAL_CPU
 * 
 * On a move, two calls occur: delete and insert. The from native/to
 * native calls are set the same for both of these operations. On an age,
 * the move bit is not set and only one delete operation occurs.
 * 
 * Suggested application operation:
 * 
 *      Insert:  If native is set, send insert op to all units.
 *      Age:     If native is set, send delete operation to all units.
 *      Move:    Ignore delete indication and wait for insert operation.
 *               Treat insert like above.
 * 
 * That is, ignore move-delete operations; only forward other
 * operations if native.
 * 
 * The BCM_L2_LOCAL_CPU flag indicates the entry is for the local CPU on
 * the device. This is valid for _add operations only. Note that
 * BCM_L2_LOCAL_CPU is related to the L2_NATIVE value. L2_NATIVE is valid
 * on reads, L2_LOCAL_CPU is valid on move or write.
 */
#define BCM_L2_COS_SRC_PRI              0x00000001 /* Source COS has priority
                                                      over destination COS. */
#define BCM_L2_DISCARD_SRC              0x00000002 
#define BCM_L2_DISCARD_DST              0x00000004 
#define BCM_L2_COPY_TO_CPU              0x00000008 
#define BCM_L2_L3LOOKUP                 0x00000010 
#define BCM_L2_STATIC                   0x00000020 
#define BCM_L2_HIT                      0x00000040 
#define BCM_L2_TRUNK_MEMBER             0x00000080 
#define BCM_L2_MCAST                    0x00000100 
#define BCM_L2_REPLACE_DYNAMIC          0x00000200 
#define BCM_L2_SRC_HIT                  0x00000400 
#define BCM_L2_DES_HIT                  0x00000800 
#define BCM_L2_REMOTE_TRUNK             0x00001000 
#define BCM_L2_MIRROR                   0x00002000 
#define BCM_L2_SETPRI                   0x00004000 
#define BCM_L2_REMOTE_LOOKUP            0x00008000 
#define BCM_L2_NATIVE                   0x00010000 
#define BCM_L2_MOVE                     0x00020000 
#define BCM_L2_FROM_NATIVE              0x00040000 
#define BCM_L2_TO_NATIVE                0x00080000 
#define BCM_L2_MOVE_PORT                0x00100000 
#define BCM_L2_LOCAL_CPU                0x00200000 /* Entry is for the local CPU
                                                      on the device. */
#define BCM_L2_USE_FABRIC_DISTRIBUTION  0x00400000 /* Use specified fabric
                                                      distribution class. */
#define BCM_L2_PENDING                  0x00800000 
#define BCM_L2_LEARN_LIMIT_EXEMPT       0x01000000 
#define BCM_L2_LEARN_LIMIT              0x02000000 
#define BCM_L2_ENTRY_OVERFLOW           0x04000000 
#define BCM_L2_LEARN_LIMIT_EXEMPT_LOCAL 0x08000000 /* Only system wide MAC limit
                                                      counter will be
                                                      incremented. */
#define BCM_L2_SET_ENCAP_VALID          0x10000000 /* DNX only: indication that
                                                      encap_id_valid needs to
                                                      set even when encap_id is
                                                      not valid. */
#define BCM_L2_SET_ENCAP_INVALID        0x20000000 /* DNX only: encap_id is
                                                      added to MAC table as is,
                                                      however valid bit is
                                                      cleared. */
#define BCM_L2_SR_SAN_DEST              0x40000000 /* Seamless Redundancy: the
                                                      destination is a SAN */
#define BCM_L2_ADD_OVERRIDE_PENDING     0x80000000 /* Override pending entry
                                                      while the same hash bucket
                                                      is full. */
                                                      

/* Device-independent L2 address structure. */
typedef struct bcm_l2_addr_s {
    uint32_t flags;                       /* BCM_L2_xxx flags. */
    uint32_t flags2;                      /* BCM_L2_FLAGS2_xxx flags. */
    uint32_t station_flags;               /* BCM_L2_STATION_xxx flags. */
    bcm_mac_t mac;                      /* 802.3 MAC address. */
    bcm_vlan_t vid;                     /* VLAN identifier. */
    int port;                           /* Zero-based port number. */
    int modid;                          /* XGS: modid. */
    bcm_trunk_t tgid;                   /* Trunk group ID. */
    bcm_cos_t cos_dst;                  /* COS based on dst addr. */
    bcm_cos_t cos_src;                  /* COS based on src addr. */
    bcm_multicast_t l2mc_group;         /* XGS: index in L2MC table */
    bcm_if_t egress_if;                 /* XGS: index in Next Hop Tables. Used
                                           it with BCM_L2_FLAGS2_ROE_NHI flag */
    bcm_multicast_t l3mc_group;         /* XGS: index in L3_IPMC table. Use it
                                           with BCM_L2_FLAGS2_L3_MULTICAST. */
    bcm_pbmp_t block_bitmap;            /* XGS: blocked egress bitmap. */
    int auth;                           /* Used if auth enabled on port. */
    int group;                          /* Group number for FP. */
    //bcm_fabric_distribution_t distribution_class; /* Fabric Distribution Class. */
    int encap_id;                       /* out logical interface */
    int age_state;                      /* Age state of the entry */
    uint32_t flow_handle;               /* FLOW handle for flex entries. */
    uint32_t flow_option_handle;        /* FLOW option handle for flex entries. */
    bcm_flow_logical_field_t logical_fields[BCM_FLOW_MAX_NOF_LOGICAL_FIELDS]; /* logical fields array for flex
                                           entries. */
                                           uint32_t num_of_fields;               /* number of logical fields. */
    bcm_pbmp_t sa_source_filter_pbmp;   /* Source port filter bitmap for this SA */
    bcm_tsn_flowset_t tsn_flowset;      /* Time-Sensitive Networking: associated
                                           flow set */
    bcm_tsn_sr_flowset_t sr_flowset;    /* Seamless Redundancy: associated flow
                                           set */
    bcm_policer_t policer_id;           /* Base policer to be used */
    bcm_tsn_pri_map_t taf_gate_primap;  /* TAF (Time Aware Filtering) gate
                                           priority mapping */
    uint32_t stat_id;                   /* Object statistics ID */
    int stat_pp_profile;                /* Statistics profile */
    uint16_t gbp_src_id;                /* GBP Source ID */
    int opaque_ctrl_id;                 /* Opaque control ID. */
} bcm_l2_addr_t;

/*****************************************************************************************/
/*                            Egress Blocking Table                                      */
/*****************************************************************************************/

//Memory: MAC_BLOCK.ipipe0 aka PORT_MAC_BLOCK alias MAC_BLOCK address 0x805c0000
//Flags: valid cachable(on)
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 32 with indices 0-31 (0x0-0x1f), each 11 bytes 3 words


#define MAC_BLOCKm                      0x805c0000
#define MAC_BLOCKm_BYTES                11

/*****************************************************************************************/
/*                            VLAN                                                       */
/*****************************************************************************************/



/* Initialize a VLAN data information structure. */
typedef struct _vlan_data_s {
    uint16_t vlan_tag; 
    bcm_pbmp_t  port_bitmap; 
    bcm_pbmp_t  ut_port_bitmap; 
} vlan_data_t;

/*
soc_field_info_t soc_EGR_VLAN_BCM56370_A0m_fields[] = {
    { DOT1P_MAPPING_PTRf, 4, 12, SOCF_LE | SOCF_GLOBAL },
    { ECCf, 7, 72, SOCF_LE | SOCF_GLOBAL },
    { ECCPf, 8, 72, SOCF_LE | SOCF_GLOBAL },
    { EN_EFILTERf, 1, 68, 0 | SOCF_GLOBAL },
    { FLEX_CTR_BASE_COUNTER_IDXf, 11, 34, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_OFFSET_MODEf, 2, 52, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_POOL_NUMBERf, 2, 48, SOCF_LE | SOCF_GLOBAL },
    { MEMBERSHIP_PROFILE_PTRf, 12, 56, SOCF_LE | SOCF_GLOBAL },
    { OUTER_TPID_INDEXf, 2, 10, SOCF_LE | SOCF_GLOBAL },
    { PARITYf, 1, 79, 0 | SOCF_GLOBAL },
    { REMARK_CFIf, 1, 16, 0 | SOCF_GLOBAL },
    { REMARK_DOT1Pf, 1, 17, 0 | SOCF_GLOBAL },
    { RESERVED_0f, 4, 18, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_1f, 2, 54, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_2f, 3, 69, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RSVD_FLEX_CTR_BASE_COUNTER_IDXf, 3, 45, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RSVD_FLEX_CTR_POOL_NUMBERf, 2, 50, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { STGf, 9, 1, SOCF_LE | SOCF_GLOBAL },
    { UNTAG_PROFILE_PTRf, 12, 22, SOCF_LE | SOCF_GLOBAL },
    { VALIDf, 1, 0, 0 | SOCF_GLOBAL }
};
 */
//Blocks:  epipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 4096 with indices 0-4095 (0x0-0xfff), each 10 bytes 3 words
#define EGR_VLANm                       0x090c0000
#define EGR_VLANm_BYTES                 10

/*
soc_field_info_t soc_VLAN_ATTRS_1_BCM56370_A0m_fields[] = {
    { ACTIVE_L3_IIF_PROFILE_INDEXf, 10, 48, SOCF_LE | SOCF_GLOBAL },
    { ECCf, 7, 59, SOCF_LE | SOCF_GLOBAL },
    { ECCPf, 8, 59, SOCF_LE | SOCF_GLOBAL },
    { ECC_DATAf, 59, 0, SOCF_LE | SOCF_GLOBAL },
    { EN_IFILTERf, 1, 23, 0 | SOCF_GLOBAL },
    { FID_IDf, 12, 24, SOCF_LE | SOCF_GLOBAL },
    { MEMBERSHIP_PROFILE_PTRf, 12, 11, SOCF_LE | SOCF_GLOBAL },
    { MIM_TERM_ENABLEf, 1, 1, 0 | SOCF_GLOBAL },
    { MPLS_ENABLEf, 1, 0, 0 | SOCF_GLOBAL },
    { PARITYf, 1, 66, 0 | SOCF_GLOBAL },
    { RESERVED_47_36f, 12, 36, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { STGf, 9, 2, SOCF_LE | SOCF_GLOBAL },
    { VALIDf, 1, 58, 0 | SOCF_GLOBAL },
    { VLAN_CTRL_IDf, 4, 36, SOCF_LE | SOCF_GLOBAL }
};
 */
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 4096 with indices 0-4095 (0x0-0xfff), each 9 bytes 3 words
#define VLAN_ATTRS_1m                   0x580c0000
#define VLAN_ATTRS_1m_BYTES             9

typedef struct {
    uint32_t entry_data[11];
}vlan_attrs_1_entry_t;
 

/*
soc_field_info_t soc_EGR_VLAN_VFI_UNTAG_BCM56370_A0m_fields[] = {
    { ECC0f, 8, 136, SOCF_LE | SOCF_GLOBAL },
    { ECCP0f, 9, 136, SOCF_LE | SOCF_GLOBAL },
    { PARITY0f, 1, 144, 0 | SOCF_GLOBAL },
    { UT_BITMAPf, 72, 0, SOCF_LE | SOCF_GLOBAL },
    { UT_PORT_BITMAPf, 72, 0, SOCF_LE | SOCF_GLOBAL },
    { UT_VP_GRP_BITMAPf, 64, 72, SOCF_LE | SOCF_GLOBAL }
};  
 */
//Memory: EGR_VLAN_VFI_UNTAG.epipe0 address 0x09100000
//Blocks:  epipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 4096 with indices 0-4095 (0x0-0xfff), each 19 bytes 5 words
#define EGR_VLAN_VFI_UNTAGm            0x09100000
#define EGR_VLAN_VFI_UNTAGm_BYTES      19

typedef struct {
    uint32_t entry_data[5];
}egr_vlan_vfi_untag_entry_t;



/*
soc_field_info_t soc_VLAN_TAB_BCM56370_A0m_fields[] = {
    { BC_DESTINATIONf, 18, 187, SOCF_LE | SOCF_GLOBAL },
    { ECCP_0f, 8, 348, SOCF_LE | SOCF_GLOBAL },
    { ECCP_1f, 8, 356, SOCF_LE | SOCF_GLOBAL },
    { ECCP_2f, 8, 364, SOCF_LE | SOCF_GLOBAL },
    { ECC_0f, 7, 348, SOCF_LE | SOCF_GLOBAL },
    { ECC_1f, 7, 356, SOCF_LE | SOCF_GLOBAL },
    { ECC_2f, 7, 364, SOCF_LE | SOCF_GLOBAL },
    { ECC_DATA_0f, 116, 0, SOCF_LE | SOCF_GLOBAL },
    { ECC_DATA_1f, 116, 116, SOCF_LE | SOCF_GLOBAL },
    { ECC_DATA_2f, 116, 232, SOCF_LE | SOCF_GLOBAL },
    { EN_IFILTERf, 1, 296, 0 | SOCF_GLOBAL },
    { FID_IDf, 12, 172, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_BASE_COUNTER_IDXf, 10, 252, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_OFFSET_MODEf, 2, 250, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_POOL_NUMBERf, 3, 245, SOCF_LE | SOCF_GLOBAL },
    { HIGIG_TRUNK_OVERRIDE_PROFILE_PTRf, 8, 157, SOCF_LE | SOCF_GLOBAL },
    { L2_ENTRY_KEY_TYPEf, 2, 151, SOCF_LE | SOCF_GLOBAL },
    { L3_IIFf, 13, 318, SOCF_LE | SOCF_GLOBAL },
    { MEMBERSHIP_PROFILE_PTRf, 12, 297, SOCF_LE | SOCF_GLOBAL },
    { PARITY_0f, 1, 355, 0 | SOCF_GLOBAL },
    { PARITY_1f, 1, 363, 0 | SOCF_GLOBAL },
    { PARITY_2f, 1, 371, 0 | SOCF_GLOBAL },
    { PHB_CTRL_IDf, 4, 223, SOCF_LE | SOCF_GLOBAL },
    { PORT_BITMAPf, 72, 0, SOCF_LE | SOCF_GLOBAL },
    { RESERVED_140f, 69, 72, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_186_184f, 3, 184, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_243_227f, 17, 227, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_317_309f, 9, 309, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RSVD_FLEX_CTR_BASE_COUNTER_IDXf, 4, 262, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RSVD_FLEX_CTR_POOL_NUMBERf, 2, 248, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RSVD_L3_IIFf, 1, 331, SOCF_RES | SOCF_GLOBAL },
    { RSVD_VFIf, 4, 344, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { SRC_PVLAN_PORT_TYPEf, 2, 153, SOCF_LE | SOCF_GLOBAL },
    { STGf, 9, 141, SOCF_LE | SOCF_GLOBAL },
    { UMC_DESTINATIONf, 18, 278, SOCF_LE | SOCF_GLOBAL },
    { UUC_DESTINATIONf, 18, 205, SOCF_LE | SOCF_GLOBAL },
    { VALIDf, 1, 150, 0 | SOCF_GLOBAL },
    { VFIf, 12, 332, SOCF_LE | SOCF_GLOBAL },
    { VIRTUAL_PORT_ENf, 1, 244, 0 | SOCF_GLOBAL },
    { VLAN_CLASS_IDf, 12, 266, SOCF_LE | SOCF_GLOBAL },
    { VLAN_PROFILE_PTRf, 7, 165, SOCF_LE | SOCF_GLOBAL },
    { WIRED_WIRELESSf, 2, 155, SOCF_LE | SOCF_GLOBAL }
};
 */
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 4096 with indices 0-4095 (0x0-0xfff), each 47 bytes 12 words
#define VLAN_TABm                       0x643c0000
#define VLAN_TABm_BYTES                 47

typedef struct {
    uint32_t entry_data[14];
}vlan_tab_entry_t;
 

//Memory: EGR_VLAN_VFI_MEMBERSHIP.epipe0 address 0x092c0000
//Flags: valid cachable(on)
//Blocks:  epipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 4096 with indices 0-4095 (0x0-0xfff), each 19 bytes 5 words
//Entry mask: -1 0x023fffff 0xfffffec0 -1 0x003fe1ff
//Description: Egress VLAN VFI member table
//  VP_GROUP_BITMAP<136:73>
//  RESERVED_1<72>
//  RESERVED_0<140:137>
//  PORT_BITMAP<71:0>
//  PARITY0<149>
//  ECCP0<149:141>
//  ECC0<148:141>
#define EGR_VLAN_VFI_MEMBERSHIPm         0x092c0000
#define EGR_VLAN_VFI_MEMBERSHIPm_BYTES   19
  

//Memory: ING_VLAN_VFI_MEMBERSHIP.ipipe0 address 0x64100000
//Flags: valid cachable(on) bist-epic
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 4096 with indices 0-4095 (0x0-0xfff), each 26 bytes 7 words
//Entry mask: -1 -1 -1 -1 -1 -1 0x00001fff
//Description: Ingress VLAN VFI member table.
//  VP_GROUP_BITMAP<195:132>
//  RESERVED<131:72>
//  PARITY<204>
//  ING_PORT_BITMAP<71:0>
//  ECC_DATA<195:0>
//  ECCP<204:196>
//  ECC<203:196>
#define ING_VLAN_VFI_MEMBERSHIPm         0x64100000
#define ING_VLAN_VFI_MEMBERSHIPm_BYTES   26

/*****************************************************************************************/
/*                            Spanning Tree (STG)                                        */
/*****************************************************************************************/

#define BCM_STG_DEFAULT                 1
typedef int bcm_stg_t;

typedef struct bcm_stg_info_s {
    int         init;       /* TRUE if STG module has been initialized */
    bcm_stg_t   stg_min;    /* STG table min index */
    bcm_stg_t   stg_max;    /* STG table max index */
    bcm_stg_t   stg_defl;   /* Default STG */
    uint32_t    stg_bitmap[16];  /* Bitmap of allocated STGs */
//PARSER_HINT_ARR bcm_pbmp_t *stg_enable; /* array of port bitmaps indicating whether the
//                               port+stg has STP enabled */
//PARSER_HINT_ARR bcm_pbmp_t *stg_state_h;/* array of port bitmaps indicating STP state for the */
//PARSER_HINT_ARR bcm_pbmp_t *stg_state_l;/* port+stg combo. Only valid if stg_enable = TRUE */
    int         stg_count;  /* Number STGs allocated */
    /* STG reverse map - keep a linked list of VLANs in each STG */
//PARSER_HINT_ARR bcm_vlan_t *vlan_first; /* Indexed by STG (also links free list) */
//PARSER_HINT_ARR bcm_vlan_t *vlan_next;  /* Indexed by  VLAN ID */
} bcm_stg_info_t;

/*
soc_field_info_t soc_STG_TAB_BCM56370_A0m_fields[] = {
    { ECCP_0f, 8, 306, SOCF_LE | SOCF_GLOBAL },
    { ECCP_1f, 8, 314, SOCF_LE | SOCF_GLOBAL },
    { ECCP_2f, 8, 322, SOCF_LE | SOCF_GLOBAL },
    { ECC_0f, 7, 306, SOCF_LE | SOCF_GLOBAL },
    { ECC_1f, 7, 314, SOCF_LE | SOCF_GLOBAL },
    { ECC_2f, 7, 322, SOCF_LE | SOCF_GLOBAL },
    { ECC_DATA_0f, 102, 0, SOCF_LE | SOCF_GLOBAL },
    { ECC_DATA_1f, 102, 102, SOCF_LE | SOCF_GLOBAL },
    { ECC_DATA_2f, 102, 204, SOCF_LE | SOCF_GLOBAL },
    { PARITY_0f, 1, 313, 0 | SOCF_GLOBAL },
    { PARITY_1f, 1, 321, 0 | SOCF_GLOBAL },
    { PARITY_2f, 1, 329, 0 | SOCF_GLOBAL },
    { PORT_SP_TREE_STATEf, 144, 0, SOCF_LE | SOCF_GLOBAL },
    { RESERVED_1f, 34, 272, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { SP_TREE_PORT0f, 2, 0, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT1f, 2, 2, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT10f, 2, 20, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT11f, 2, 22, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT12f, 2, 24, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT13f, 2, 26, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT14f, 2, 28, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT15f, 2, 30, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT16f, 2, 32, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT17f, 2, 34, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT18f, 2, 36, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT19f, 2, 38, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT2f, 2, 4, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT20f, 2, 40, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT21f, 2, 42, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT22f, 2, 44, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT23f, 2, 46, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT24f, 2, 48, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT25f, 2, 50, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT26f, 2, 52, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT27f, 2, 54, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT28f, 2, 56, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT29f, 2, 58, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT3f, 2, 6, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT30f, 2, 60, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT31f, 2, 62, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT32f, 2, 64, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT33f, 2, 66, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT34f, 2, 68, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT35f, 2, 70, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT36f, 2, 72, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT37f, 2, 74, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT38f, 2, 76, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT39f, 2, 78, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT4f, 2, 8, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT40f, 2, 80, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT41f, 2, 82, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT42f, 2, 84, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT43f, 2, 86, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT44f, 2, 88, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT45f, 2, 90, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT46f, 2, 92, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT47f, 2, 94, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT48f, 2, 96, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT49f, 2, 98, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT5f, 2, 10, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT50f, 2, 100, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT51f, 2, 102, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT52f, 2, 104, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT53f, 2, 106, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT54f, 2, 108, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT55f, 2, 110, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT56f, 2, 112, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT57f, 2, 114, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT58f, 2, 116, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT59f, 2, 118, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT6f, 2, 12, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT60f, 2, 120, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT61f, 2, 122, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT62f, 2, 124, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT63f, 2, 126, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT64f, 2, 128, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT65f, 2, 130, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT66f, 2, 132, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT67f, 2, 134, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT68f, 2, 136, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT69f, 2, 138, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT7f, 2, 14, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT70f, 2, 140, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT71f, 2, 142, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT8f, 2, 16, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT9f, 2, 18, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP0f, 2, 144, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP1f, 2, 146, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP10f, 2, 164, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP11f, 2, 166, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP12f, 2, 168, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP13f, 2, 170, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP14f, 2, 172, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP15f, 2, 174, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP16f, 2, 176, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP17f, 2, 178, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP18f, 2, 180, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP19f, 2, 182, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP2f, 2, 148, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP20f, 2, 184, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP21f, 2, 186, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP22f, 2, 188, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP23f, 2, 190, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP24f, 2, 192, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP25f, 2, 194, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP26f, 2, 196, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP27f, 2, 198, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP28f, 2, 200, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP29f, 2, 202, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP3f, 2, 150, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP30f, 2, 204, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP31f, 2, 206, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP32f, 2, 208, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP33f, 2, 210, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP34f, 2, 212, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP35f, 2, 214, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP36f, 2, 216, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP37f, 2, 218, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP38f, 2, 220, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP39f, 2, 222, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP4f, 2, 152, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP40f, 2, 224, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP41f, 2, 226, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP42f, 2, 228, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP43f, 2, 230, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP44f, 2, 232, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP45f, 2, 234, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP46f, 2, 236, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP47f, 2, 238, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP48f, 2, 240, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP49f, 2, 242, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP5f, 2, 154, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP50f, 2, 244, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP51f, 2, 246, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP52f, 2, 248, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP53f, 2, 250, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP54f, 2, 252, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP55f, 2, 254, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP56f, 2, 256, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP57f, 2, 258, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP58f, 2, 260, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP59f, 2, 262, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP6f, 2, 156, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP60f, 2, 264, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP61f, 2, 266, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP62f, 2, 268, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP63f, 2, 270, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP7f, 2, 158, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP8f, 2, 160, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP9f, 2, 162, SOCF_LE | SOCF_GLOBAL },
    { VP_GROUP_SP_TREE_STATEf, 128, 144, SOCF_LE | SOCF_GLOBAL }
};
 */

//Memory: STG_TAB.ipipe0 aka VLAN_STG alias STG address 0x64400000
//Flags: valid cachable(on)
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 512 with indices 0-511 (0x0-0x1ff), each 42 bytes 11 words
#define STG_TABm                        0x64400000
#define STG_TABm_BYTES                  42
#define STG_TABm_MAX_INDEX              511

/*
soc_field_info_t soc_EGR_VLAN_STG_BCM56370_A0m_fields[] = {
    { ECC0f, 8, 278, SOCF_LE | SOCF_GLOBAL },
    { ECC1f, 8, 287, SOCF_LE | SOCF_GLOBAL },
    { ECCP0f, 9, 278, SOCF_LE | SOCF_GLOBAL },
    { ECCP1f, 9, 287, SOCF_LE | SOCF_GLOBAL },
    { PARITY0f, 1, 286, 0 | SOCF_GLOBAL },
    { PARITY1f, 1, 295, 0 | SOCF_GLOBAL },
    { PORT_SP_TREE_STATEf, 144, 0, SOCF_LE | SOCF_GLOBAL },
    { RESERVED_1f, 6, 272, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { SP_TREE_PORT0f, 2, 0, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT1f, 2, 2, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT10f, 2, 20, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT11f, 2, 22, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT12f, 2, 24, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT13f, 2, 26, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT14f, 2, 28, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT15f, 2, 30, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT16f, 2, 32, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT17f, 2, 34, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT18f, 2, 36, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT19f, 2, 38, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT2f, 2, 4, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT20f, 2, 40, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT21f, 2, 42, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT22f, 2, 44, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT23f, 2, 46, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT24f, 2, 48, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT25f, 2, 50, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT26f, 2, 52, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT27f, 2, 54, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT28f, 2, 56, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT29f, 2, 58, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT3f, 2, 6, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT30f, 2, 60, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT31f, 2, 62, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT32f, 2, 64, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT33f, 2, 66, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT34f, 2, 68, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT35f, 2, 70, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT36f, 2, 72, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT37f, 2, 74, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT38f, 2, 76, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT39f, 2, 78, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT4f, 2, 8, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT40f, 2, 80, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT41f, 2, 82, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT42f, 2, 84, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT43f, 2, 86, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT44f, 2, 88, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT45f, 2, 90, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT46f, 2, 92, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT47f, 2, 94, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT48f, 2, 96, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT49f, 2, 98, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT5f, 2, 10, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT50f, 2, 100, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT51f, 2, 102, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT52f, 2, 104, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT53f, 2, 106, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT54f, 2, 108, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT55f, 2, 110, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT56f, 2, 112, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT57f, 2, 114, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT58f, 2, 116, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT59f, 2, 118, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT6f, 2, 12, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT60f, 2, 120, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT61f, 2, 122, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT62f, 2, 124, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT63f, 2, 126, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT64f, 2, 128, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT65f, 2, 130, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT66f, 2, 132, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT67f, 2, 134, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT68f, 2, 136, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT69f, 2, 138, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT7f, 2, 14, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT70f, 2, 140, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT71f, 2, 142, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT8f, 2, 16, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_PORT9f, 2, 18, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP0f, 2, 144, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP1f, 2, 146, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP10f, 2, 164, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP11f, 2, 166, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP12f, 2, 168, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP13f, 2, 170, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP14f, 2, 172, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP15f, 2, 174, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP16f, 2, 176, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP17f, 2, 178, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP18f, 2, 180, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP19f, 2, 182, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP2f, 2, 148, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP20f, 2, 184, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP21f, 2, 186, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP22f, 2, 188, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP23f, 2, 190, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP24f, 2, 192, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP25f, 2, 194, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP26f, 2, 196, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP27f, 2, 198, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP28f, 2, 200, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP29f, 2, 202, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP3f, 2, 150, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP30f, 2, 204, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP31f, 2, 206, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP32f, 2, 208, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP33f, 2, 210, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP34f, 2, 212, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP35f, 2, 214, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP36f, 2, 216, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP37f, 2, 218, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP38f, 2, 220, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP39f, 2, 222, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP4f, 2, 152, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP40f, 2, 224, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP41f, 2, 226, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP42f, 2, 228, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP43f, 2, 230, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP44f, 2, 232, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP45f, 2, 234, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP46f, 2, 236, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP47f, 2, 238, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP48f, 2, 240, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP49f, 2, 242, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP5f, 2, 154, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP50f, 2, 244, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP51f, 2, 246, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP52f, 2, 248, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP53f, 2, 250, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP54f, 2, 252, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP55f, 2, 254, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP56f, 2, 256, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP57f, 2, 258, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP58f, 2, 260, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP59f, 2, 262, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP6f, 2, 156, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP60f, 2, 264, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP61f, 2, 266, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP62f, 2, 268, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP63f, 2, 270, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP7f, 2, 158, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP8f, 2, 160, SOCF_LE | SOCF_GLOBAL },
    { SP_TREE_VP_GRP9f, 2, 162, SOCF_LE | SOCF_GLOBAL },
    { VP_GROUP_SP_TREE_STATEf, 128, 144, SOCF_LE | SOCF_GLOBAL }
};
 */
//Memory: EGR_VLAN_STG.epipe0 address 0x09140000
//Flags: valid cachable(on)
//Blocks:  epipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 512 with indices 0-511 (0x0-0x1ff), each 37 bytes 10 words
//Entry mask: -1 -1 -1 -1 -1 -1 -1 -1 0xffc0ffff 0x000000ff
//Description: Egress Spanning Tree Stage Table
#define EGR_VLAN_STGm                   0x09140000
#define EGR_VLAN_STGm_BYTES             37

#define STG_BITS_PER_PORT       2
#define STG_PORT_MASK           ((1 << STG_BITS_PER_PORT)-1)
#define STG_PORTS_PER_WORD      (32 / STG_BITS_PER_PORT)
#define STG_WORD(port)          ((port) / STG_PORTS_PER_WORD)
#define STG_BITS_SHIFT(port)    \
        (STG_BITS_PER_PORT * ((port) % STG_PORTS_PER_WORD))
#define STG_BITS_MASK(port)     (STG_PORT_MASK << (STG_BITS_SHIFT(port)))


#define PVP_STP_DISABLED	0	/* Disabled */
#define PVP_STP_BLOCKING	1	/* Blocking/Listening */
#define PVP_STP_LEARNING	2	/* Learning */
#define PVP_STP_FORWARDING	3	/* Forwarding */

/*****************************************************************************************/
/*                            N3248TE hardware&ports info                                */
/*****************************************************************************************/
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


#define ETH_GE_PORT    0x0001
#define ETH_XE_PORT    0x0002
#define ETH_HG_PORT    0x0004
#define ETH_HGL_PORT   0x0008
#define ETH_STK_PORT   0x0010
#define ETH_CES_PORT   0x0020
#define ETH_OLP_PORT   0x0040
#define ETH_LPHY_PORT  0x0080
#define ETH_ETH_PORT   0x0100
#define ETH_INTLB_PORT 0x0200


#define PORT_LINK_STATUS_DOWN                   0          
#define PORT_LINK_STATUS_UP                     1          
#define PORT_LINK_STATUS_FAILED                 2          
#define PORT_LINK_STATUS_REMOTE_FAULT           3          
#define PORT_LINK_STATUS_LOCAL_FAULT            4          
#define PORT_LINK_STATUS_LOCAL_AND_REMOTE_FAULT 5        

typedef struct port_cb_s {
    uint32_t  eth_port_type;
    uint8_t   valid;    
    uint8_t   probed;
    uint32_t  ext_phy_addr;
    uint32_t  int_phy_addr;
    uint32_t  phy_flags;
    uint32_t  primary_and_offset;
    int32_t   pm_num;
    int32_t   subp;
    int32_t   pipe_num;
    char      name[8];

    struct phy_ctrl_ {
       uint32_t flags; 
    } phy_ctrl;

    struct dev_desc_ {
       uint32_t flags;
       uint16 phy_id_orig;
       uint16 phy_id_base; /* port 0 addr */
       uint16 phy_slice;
       int32_t  port_pre_speed;       
    } dev_desc;
} port_info_t;

#define SOC_TD3_MMU_PORT_STRIDE          0x7f  /* MMU Port number space per pipe */
#define SOC_TD3_MMU_PORT_PER_PIPE        128

#define _HX5_PORTS_PER_PMQ_PBLK          16
#define _HX5_PORTS_PER_PBLK              4 


/****************************     phy_ctrl.flags    **************************************/
/* bit 31:29 used for init state */
#define PHYCTRL_INIT_STATE_DEFAULT 0
#define PHYCTRL_INIT_STATE_PASS1   1
#define PHYCTRL_INIT_STATE_PASS2   2
#define PHYCTRL_INIT_STATE_PASS3   3
#define PHYCTRL_INIT_STATE_PASS4   4
#define PHYCTRL_INIT_STATE_PASS5   5
#define PHYCTRL_INIT_STATE_PASS6   6
#define PHYCTRL_INIT_STATE_PASS7   7
#define PHYCTRL_INIT_MASK          0x7
#define PHYCTRL_INIT_SHFT          29
#define PHYCTRL_INIT_STATE(_pc)     ((((_pc)->flags) >> PHYCTRL_INIT_SHFT) & PHYCTRL_INIT_MASK)
#define PHYCTRL_INIT_STATE_SET(_pc,_st) ((_pc)->flags = ((_pc)->flags & \
    (~(PHYCTRL_INIT_MASK << PHYCTRL_INIT_SHFT))) | \
    (((_st) & PHYCTRL_INIT_MASK) << PHYCTRL_INIT_SHFT))

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
    int             port_init_speed[SOC_MAX_NUM_PORTS];       /* ports initial speed */
    int             port_serdes[SOC_MAX_NUM_PORTS];           /* serdes number */
    int             port_num_subport[SOC_MAX_NUM_PORTS];      /* number of subport */    
    int             port_pipe[SOC_MAX_NUM_PORTS];             /* pipe number */
    int             port_group[SOC_MAX_NUM_PORTS];            /* group number */

    int             cpu_hg_index;           /* table index for cpu port
                                             * higig packet where table indexed
                                             * by physical port*/
    int             port_type[SOC_MAX_NUM_PORTS];             /* internal port type */               

    port_info_t     ports[SOC_MAX_NUM_PORTS];                 /* port information   */
} soc_info_t;

#define COUNTOF(ary)        ((int) (sizeof (ary) / sizeof ((ary)[0])))


/*****************************************************************************************/
/*                              port info                                                */
/*****************************************************************************************/
typedef enum _shr_port_if_e {
    _SHR_PORT_IF_NOCXN, /* No physical connection */
    _SHR_PORT_IF_NULL,  /* Pass-through connection without PHY */
    _SHR_PORT_IF_MII,
    _SHR_PORT_IF_GMII,
    _SHR_PORT_IF_SGMII,
    _SHR_PORT_IF_TBI,
    _SHR_PORT_IF_XGMII,
    _SHR_PORT_IF_RGMII,
    _SHR_PORT_IF_RvMII,
    _SHR_PORT_IF_SFI,
    _SHR_PORT_IF_XFI,
    _SHR_PORT_IF_KR,
    _SHR_PORT_IF_KR4,
    _SHR_PORT_IF_CR,
    _SHR_PORT_IF_CR4,
    _SHR_PORT_IF_XLAUI,
    _SHR_PORT_IF_SR,
    _SHR_PORT_IF_RXAUI,
    _SHR_PORT_IF_XAUI,
    _SHR_PORT_IF_SPAUI,
    _SHR_PORT_IF_QSGMII,
    _SHR_PORT_IF_ILKN,
    _SHR_PORT_IF_RCY,
    _SHR_PORT_IF_FAT_PIPE,
    _SHR_PORT_IF_CGMII,
    _SHR_PORT_IF_CAUI,
    _SHR_PORT_IF_LR,
    _SHR_PORT_IF_LR4,
    _SHR_PORT_IF_SR4,
    _SHR_PORT_IF_KX,
    _SHR_PORT_IF_ZR,
    _SHR_PORT_IF_SR10,
    _SHR_PORT_IF_OTL,
    _SHR_PORT_IF_CPU,
    _SHR_PORT_IF_OLP,
    _SHR_PORT_IF_OAMP,
    _SHR_PORT_IF_ERP,
    _SHR_PORT_IF_TM_INTERNAL_PKT,   
    _SHR_PORT_IF_SR2,
    _SHR_PORT_IF_KR2,
    _SHR_PORT_IF_CR2,
    _SHR_PORT_IF_XFI2,
    _SHR_PORT_IF_XLAUI2,
    _SHR_PORT_IF_CR10,
    _SHR_PORT_IF_KR10,
    _SHR_PORT_IF_LR10,
    _SHR_PORT_IF_ER,
    _SHR_PORT_IF_ER2,
    _SHR_PORT_IF_ER4,
    _SHR_PORT_IF_CX,
    _SHR_PORT_IF_CX2,
    _SHR_PORT_IF_CX4,
    _SHR_PORT_IF_CAUI_C2C,
    _SHR_PORT_IF_CAUI_C2M,
    _SHR_PORT_IF_VSR,
    _SHR_PORT_IF_LR2,
    _SHR_PORT_IF_LRM,
    _SHR_PORT_IF_XLPPI,
    _SHR_PORT_IF_2500X,
    _SHR_PORT_IF_SAT,
    _SHR_PORT_IF_IPSEC,
    _SHR_PORT_IF_LBG,
    _SHR_PORT_IF_CAUI4,
    _SHR_PORT_IF_5000X,
    _SHR_PORT_IF_EVENTOR,
    _SHR_PORT_IF_RCY_MIRROR,
    _SHR_PORT_IF_CPRI,
    _SHR_PORT_IF_RSVD4,
    _SHR_PORT_IF_NIF_ETH,
    _SHR_PORT_IF_FLEXE_CLIENT,
    _SHR_PORT_IF_VIRTUAL_FLEXE_CLIENT,
    _SHR_PORT_IF_SCH,
    _SHR_PORT_IF_TUNNEL,
    _SHR_PORT_IF_CRPS,
    _SHR_PORT_IF_COUNT /* last, please */
} bcm_port_if_t;


typedef uint32_t bcm_port_abil_t;

typedef struct _shr_port_ability_s {
    uint32_t speed_half_duplex;
    uint32_t speed_full_duplex;
    uint32_t pause;
    uint32_t interface;
    uint32_t medium;
    uint32_t loopback;
    uint32_t flags;
    uint32_t eee;
    uint32_t rsvd;
    uint32_t encap;
    uint32_t fec;
    uint32_t channel;
} bcm_port_ability_t;

/*
 * Defines:
 *      _SHR_PORT_MDIX_*
 * Purpose:
 *      Defines the MDI crossover (MDIX) modes for the port
 */
typedef enum _shr_port_mdix_e {
    _SHR_PORT_MDIX_AUTO,
    _SHR_PORT_MDIX_FORCE_AUTO,
    _SHR_PORT_MDIX_NORMAL,
    _SHR_PORT_MDIX_XOVER,
    _SHR_PORT_MDIX_COUNT    /* last, please */
} bcm_port_mdix_t;


/*
 * Defines:
 *      _SHR_PORT_MDIX_STATUS_*
 * Purpose:
 *      Defines the MDI crossover state
 */
typedef enum _shr_port_mdix_status_e {
    _SHR_PORT_MDIX_STATUS_NORMAL,
    _SHR_PORT_MDIX_STATUS_XOVER,
    _SHR_PORT_MDIX_STATUS_COUNT       /* last, please */
} bcm_port_mdix_status_t;

/*
 * Defines:
 *      _SHR_PORT_MEDIUM_*
 * Purpose:
 *      Supported physical mediums
 */
typedef enum _shr_port_medium_e {
    _SHR_PORT_MEDIUM_NONE              = 0,
    _SHR_PORT_MEDIUM_COPPER            = 1,
    _SHR_PORT_MEDIUM_FIBER             = 2,
    _SHR_PORT_MEDIUM_BACKPLANE,
    _SHR_PORT_MEDIUM_ALL,              /* this defines mainly for local_ability_get function */
    _SHR_PORT_MEDIUM_COUNT             /* last, please */
} bcm_port_medium_t;

/* 
 * Port information valid fields
 * 
 * Each field in the bcm_port_info_t structure has a corresponding mask
 * bit to control whether to get or set that value during the execution
 * of the bcm_port_selective_get/_set functions. The OR of all requested
 * ATTR masks should be stored in the action_mask field and the OR of all
 * requested ATTR2 masks should be stored in the action_mask2 field of
 * the bcm_port_info_t before calling the functions.
 */
#define BCM_PORT_ATTR_ENABLE_MASK           0x00000001 
#define BCM_PORT_ATTR_LINKSTAT_MASK         0x00000002 /* Get only. */
#define BCM_PORT_ATTR_AUTONEG_MASK          0x00000004 
#define BCM_PORT_ATTR_SPEED_MASK            0x00000008 
#define BCM_PORT_ATTR_DUPLEX_MASK           0x00000010 
#define BCM_PORT_ATTR_LINKSCAN_MASK         0x00000020 
#define BCM_PORT_ATTR_LEARN_MASK            0x00000040 
#define BCM_PORT_ATTR_DISCARD_MASK          0x00000080 
#define BCM_PORT_ATTR_VLANFILTER_MASK       0x00000100 
#define BCM_PORT_ATTR_UNTAG_PRI_MASK        0x00000200 
#define BCM_PORT_ATTR_UNTAG_VLAN_MASK       0x00000400 
#define BCM_PORT_ATTR_STP_STATE_MASK        0x00000800 
#define BCM_PORT_ATTR_PFM_MASK              0x00001000 
#define BCM_PORT_ATTR_LOOPBACK_MASK         0x00002000 
#define BCM_PORT_ATTR_PHY_MASTER_MASK       0x00004000 
#define BCM_PORT_ATTR_INTERFACE_MASK        0x00008000 
#define BCM_PORT_ATTR_PAUSE_TX_MASK         0x00010000 
#define BCM_PORT_ATTR_PAUSE_RX_MASK         0x00020000 
#define BCM_PORT_ATTR_PAUSE_MAC_MASK        0x00040000 
#define BCM_PORT_ATTR_LOCAL_ADVERT_MASK     0x00080000 
#define BCM_PORT_ATTR_REMOTE_ADVERT_MASK    0x00100000 /* Get only. */
#define BCM_PORT_ATTR_ENCAP_MASK            0x00200000 
#define BCM_PORT_ATTR_RATE_MCAST_MASK       0x00400000 
#define BCM_PORT_ATTR_RATE_BCAST_MASK       0x00800000 
#define BCM_PORT_ATTR_RATE_DLFBC_MASK       0x01000000 
#define BCM_PORT_ATTR_SPEED_MAX_MASK        0x02000000 /* Get only. */
#define BCM_PORT_ATTR_ABILITY_MASK          0x04000000 /* Get only. */
#define BCM_PORT_ATTR_FRAME_MAX_MASK        0x08000000 
#define BCM_PORT_ATTR_MDIX_MASK             0x10000000 
#define BCM_PORT_ATTR_MDIX_STATUS_MASK      0x20000000 
#define BCM_PORT_ATTR_MEDIUM_MASK           0x40000000 
#define BCM_PORT_ATTR_FAULT_MASK            0x80000000 /* Get only. */
#define BCM_PORT_ATTR2_PORT_ABILITY         0x00000001 

/* Backward compatibility. */
#define BCM_PORT_ATTR_SPEED_MAX BCM_PORT_ATTR_SPEED_MAX_MASK 
#define BCM_PORT_ATTR_ABILITY   BCM_PORT_ATTR_ABILITY_MASK 

#define BCM_PORT_ATTR_ALL_MASK      0xffffffff 
#define BCM_PORT_ATTR_PAUSE_MASK    \
    (BCM_PORT_ATTR_PAUSE_TX_MASK    | \
     BCM_PORT_ATTR_PAUSE_RX_MASK) 
#define BCM_PORT_ATTR_RATE_MASK     \
    (BCM_PORT_ATTR_RATE_MCAST_MASK  | \
     BCM_PORT_ATTR_RATE_BCAST_MASK  | \
     BCM_PORT_ATTR_RATE_DLFBC_MASK) 

/* Attributes that can be controlled on BCM5670/75. */
#define BCM_PORT_HERC_ATTRS     \
    (BCM_PORT_ATTR_ENABLE_MASK      | \
     BCM_PORT_ATTR_LINKSTAT_MASK    | \
     BCM_PORT_ATTR_SPEED_MASK       | \
     BCM_PORT_ATTR_DUPLEX_MASK      | \
     BCM_PORT_ATTR_LINKSCAN_MASK    | \
     BCM_PORT_ATTR_INTERFACE_MASK   | \
     BCM_PORT_ATTR_LOOPBACK_MASK    | \
     BCM_PORT_ATTR_PAUSE_TX_MASK    | \
     BCM_PORT_ATTR_PAUSE_RX_MASK    | \
     BCM_PORT_ATTR_PAUSE_MAC_MASK   | \
     BCM_PORT_ATTR_FRAME_MAX_MASK   | \
     BCM_PORT_ATTR_ENCAP_MASK) 

/* Attributes specific to XGS devices. */
#define BCM_PORT_XGS_ATTRS      (BCM_PORT_ATTR_ENCAP_MASK) 

/* Auto-negotiated values. */
#define BCM_PORT_AN_ATTRS       \
    (BCM_PORT_ATTR_SPEED_MASK       | \
     BCM_PORT_ATTR_DUPLEX_MASK      | \
     BCM_PORT_ATTR_PAUSE_MASK) 


/* bcm_port_info_s */
typedef struct bcm_port_info_s {
    uint32_t action_mask;                 /* BCM_PORT_ATTR_xxx. */
    uint32_t action_mask2;                /* BCM_PORT_ATTR2_xxx. */
    int enable; 
    int linkstatus; 
    int autoneg; 
    int speed; 
    int duplex; 
    int linkscan; 
    uint32_t learn; 
    int discard; 
    uint32_t vlanfilter; 
    int untagged_priority; 
    bcm_vlan_t untagged_vlan; 
    int stp_state; 
    int pfm; 
    int loopback; 
    int phy_master; 
    bcm_port_if_t interface; 
    int pause_tx; 
    int pause_rx; 
    int encap_mode; 
    bcm_mac_t pause_mac; 
    bcm_port_abil_t local_advert; 
    bcm_port_ability_t local_ability; 
    int remote_advert_valid; 
    bcm_port_abil_t remote_advert; 
    bcm_port_ability_t remote_ability; 
    int mcast_limit; 
    int mcast_limit_enable; 
    int bcast_limit; 
    int bcast_limit_enable; 
    int dlfbc_limit; 
    int dlfbc_limit_enable; 
    int speed_max; 
    bcm_port_abil_t ability; 
    bcm_port_ability_t port_ability; 
    int frame_max; 
    bcm_port_mdix_t mdix; 
    bcm_port_mdix_status_t mdix_status; 
    bcm_port_medium_t medium; 
    uint32_t fault; 
} bcm_port_info_t;



typedef struct bcm_port_cfg_s {
    int		pc_frame_type;
    int		pc_ether_type;

    int		pc_stp_state;	  /* spanning tree state of port */
    int		pc_cpu;		  /* CPU learning */
    int		pc_disc;	  /* discard state */
    int		pc_bpdu_disable;  /* Where is this in Draco? */
    int		pc_trunk;	  /* trunking on for this port */
    int		pc_tgid;	  /* trunk group id */
    int		pc_mirror_ing;	  /* mirror on ingress */
    int		pc_ptype;	  /* port type */
    int		pc_jumbo;
    int		pc_cml;		  /* CML bits */
    int     pc_cml_move;  /* CML move bits on supporting devices */

    bcm_pbmp_t	pc_pbm;		  /* port bitmaps for port based vlan */
    bcm_pbmp_t	pc_ut_pbm;
    bcm_vlan_t	pc_vlan;	  /* port based vlan tag */
    bcm_vlan_t	pc_ivlan;	  /* port based inner-tag vlan tag */
    int         pc_vlan_action;   /* port based vlan action profile pointer */

    int		pc_l3_flags;	  /* L3 flags. */

    int	        pc_new_opri;      /* new outer packet priority */
    int	        pc_new_ocfi;      /* new outer cfi */
    int	        pc_new_ipri;      /* new inner packet priority */
    int	        pc_new_icfi;      /* new inner cfi */

    int		pc_dse_mode;	  /* DSCP mapping (off, or on/mode) */
    int		pc_dse_mode_ipv6;	  /* DSCP mapping for IPv6 (off, or on/mode) */
    int		pc_dscp;	  /* Resultant diffserv code point */

    int         pc_en_ifilter;    /* Enable Ingress Filtering */
    int         pc_pfm;           /* In the port table for Draco */
    int         pc_dscp_prio;     /* For Draco15 & Tucana */
    int         pc_bridge_port;   /* FB/ER, allows egress=ingress */
    int         pc_nni_port;      /* FB, indicates non-customer port */

    int     pc_urpf_mode;         /* Unicast rpf lookup mode.      */
    int     pc_urpf_def_gw_check; /* Unicast rpf check default gw. */
    int     pc_pvlan_enable;      /* Private (force) vlan enable */

} bcm_port_cfg_t;

/*****************************************************************************************/
/*                              L2                                                       */
/*****************************************************************************************/
/*
 * Values for CML (control what happens on Source Lookup Failure packet).
 * The last two are for Draco only; on StrataSwitch CML is only 2 bits.
 */

#define	PVP_CML_SWITCH		0	/*   Learn ARL, !CPU,  Forward */
#define	PVP_CML_CPU		1	/*  !Learn ARL,  CPU, !Forward */
#define	PVP_CML_FORWARD		2	/*  !Learn ARL, !CPU,  Forward */
#define	PVP_CML_DROP		3	/*  !Learn ARL, !CPU, !Forward */
#define PVP_CML_CPU_SWITCH	4	/*   Learn ARL,  CPU,  Forward */
#define PVP_CML_CPU_FORWARD 	5	/*  !Learn ARL,  CPU,  Forward */

/*
soc_field_info_t soc_L2X_BCM56370_A0m_fields[] = {
    { ACTION_PROFILE_PTRf, 5, 103, SOCF_LE | SOCF_GLOBAL },
    { ASSOCIATED_DATAf, 37, 71, SOCF_LE },
    { BASE_VALIDf, 3, 0, SOCF_LE | SOCF_GLOBAL },
    { CLASS_IDf, 6, 90, SOCF_LE },
    { CPUf, 1, 104, 0 },
    { DATAf, 37, 71, SOCF_LE },
    { DATA_TYPEf, 5, 103, SOCF_LE | SOCF_GLOBAL },
    { DESTINATIONf, 18, 72, SOCF_LE },
    { DST_DISCARDf, 1, 103, 0 },
    { DUMMY_INDEXf, 1, 95, 0 },
    { EVPN_AGE_DISABLEf, 1, 107, 0 },
    { FCOE_ZONE__ACTIONf, 1, 101, 0 },
    { FCOE_ZONE__ASSOCIATED_DATAf, 18, 90, SOCF_LE },
    { FCOE_ZONE__CLASS_IDf, 10, 91, SOCF_LE },
    { FCOE_ZONE__CPUf, 1, 90, 0 },
    { FCOE_ZONE__DATAf, 18, 90, SOCF_LE },
    { FCOE_ZONE__D_IDf, 24, 32, SOCF_LE },
    { FCOE_ZONE__HASH_LSBf, 16, 8, SOCF_LE },
    { FCOE_ZONE__KEYf, 68, 3, SOCF_LE },
    { FCOE_ZONE__RESERVEDf, 19, 71, SOCF_LE },
    { FCOE_ZONE__RESERVED_0f, 5, 103, SOCF_LE|SOCF_RES },
    { FCOE_ZONE__RESERVED_KEY_PADDINGf, 3, 68, SOCF_LE|SOCF_RES },
    { FCOE_ZONE__STATIC_BITf, 1, 102, 0 },
    { FCOE_ZONE__S_IDf, 24, 8, SOCF_LE },
    { FCOE_ZONE__VSAN_IDf, 12, 56, SOCF_LE },
    { HASH_LSBf, 16, 22, SOCF_LE },
    { HITDAf, 1, 108, 0 | SOCF_GLOBAL },
    { HITSAf, 1, 109, 0 | SOCF_GLOBAL },
    { KEYf, 68, 3, SOCF_LE },
    { KEY_TYPEf, 5, 3, SOCF_LE | SOCF_GLOBAL },
    { L2__ASSOCIATED_DATAf, 37, 71, SOCF_LE },
    { L2__CLASS_IDf, 6, 90, SOCF_LE },
    { L2__CPUf, 1, 104, 0 },
    { L2__DATAf, 37, 71, SOCF_LE },
    { L2__DESTINATIONf, 18, 72, SOCF_LE },
    { L2__DST_DISCARDf, 1, 103, 0 },
    { L2__DUMMY_INDEXf, 1, 95, 0 },
    { L2__EVPN_AGE_DISABLEf, 1, 107, 0 },
    { L2__HASH_LSBf, 16, 22, SOCF_LE },
    { L2__KEYf, 68, 3, SOCF_LE },
    { L2__MAC_ADDRf, 48, 22, SOCF_LE },
    { L2__MAC_BLOCK_INDEXf, 5, 90, SOCF_LE },
    { L2__PENDINGf, 1, 101, 0 },
    { L2__PRIf, 4, 97, SOCF_LE },
    { L2__RESERVED_KEY_PADDINGf, 1, 70, SOCF_RES },
    { L2__RPEf, 1, 96, 0 },
    { L2__SCPf, 1, 106, 0 },
    { L2__SRC_DISCARDf, 1, 105, 0 },
    { L2__STATIC_BITf, 1, 102, 0 },
    { L2__VFIf, 12, 8, SOCF_LE },
    { L2__VLAN_IDf, 12, 8, SOCF_LE },
    { LOCAL_SAf, 1, 110, 0 | SOCF_GLOBAL },
    { MAC_ADDRf, 48, 22, SOCF_LE },
    { MAC_BLOCK_INDEXf, 5, 90, SOCF_LE },
    { PENDINGf, 1, 101, 0 },
    { PE_VID__ASSOCIATED_DATAf, 40, 68, SOCF_LE },
    { PE_VID__CLASS_IDf, 6, 77, SOCF_LE },
    { PE_VID__CPUf, 1, 69, 0 },
    { PE_VID__DATAf, 40, 68, SOCF_LE },
    { PE_VID__DESTINATIONf, 18, 84, SOCF_LE },
    { PE_VID__DST_DISCARDf, 1, 70, 0 },
    { PE_VID__DUMMY_1f, 1, 82, SOCF_RES },
    { PE_VID__ETAG_VIDf, 14, 20, SOCF_LE },
    { PE_VID__EVPN_AGE_DISABLEf, 1, 83, 0 },
    { PE_VID__HASH_LSBf, 16, 8, SOCF_LE },
    { PE_VID__KEYf, 32, 3, SOCF_LE },
    { PE_VID__MAC_BLOCK_INDEXf, 5, 77, SOCF_LE },
    { PE_VID__NAMESPACEf, 12, 8, SOCF_LE },
    { PE_VID__PRIf, 4, 72, SOCF_LE },
    { PE_VID__RESERVEDf, 33, 35, SOCF_LE },
    { PE_VID__RESERVED_0f, 5, 103, SOCF_LE|SOCF_RES },
    { PE_VID__RESERVED_KEY_PADDINGf, 1, 34, SOCF_RES },
    { PE_VID__RPEf, 1, 76, 0 },
    { PE_VID__SCPf, 1, 71, 0 },
    { PE_VID__SRC_DISCARDf, 1, 68, 0 },
    { PE_VID__STATIC_BITf, 1, 102, 0 },
    { POLICY_DATAf, 95, 8, SOCF_LE | SOCF_GLOBAL },
    { PRIf, 4, 97, SOCF_LE },
    { RESERVED_KEY_PADDINGf, 1, 70, SOCF_RES },
    { RPEf, 1, 96, 0 },
    { SCPf, 1, 106, 0 },
    { SRC_DISCARDf, 1, 105, 0 },
    { STATIC_BITf, 1, 102, 0 | SOCF_GLOBAL },
    { TABLE_FIELDSf, 95, 8, SOCF_LE | SOCF_GLOBAL },
    { VFIf, 12, 8, SOCF_LE },
    { VIF__ASSOCIATED_DATAf, 36, 72, SOCF_LE },
    { VIF__CLASS_IDf, 6, 90, SOCF_LE },
    { VIF__CPUf, 1, 104, 0 },
    { VIF__DATAf, 36, 72, SOCF_LE },
    { VIF__DESTINATIONf, 18, 72, SOCF_LE },
    { VIF__DST_DISCARDf, 1, 103, 0 },
    { VIF__DST_VIFf, 14, 20, SOCF_LE },
    { VIF__DUMMY_INDEXf, 1, 95, 0 },
    { VIF__EVPN_AGE_DISABLEf, 1, 107, 0 },
    { VIF__HASH_LSBf, 16, 8, SOCF_LE },
    { VIF__KEYf, 32, 3, SOCF_LE },
    { VIF__MAC_BLOCK_INDEXf, 5, 90, SOCF_LE },
    { VIF__NAMESPACEf, 12, 8, SOCF_LE },
    { VIF__Pf, 1, 34, 0 },
    { VIF__PRIf, 4, 97, SOCF_LE },
    { VIF__RESERVEDf, 37, 35, SOCF_LE },
    { VIF__RESERVED_0f, 1, 101, SOCF_RES },
    { VIF__RPEf, 1, 96, 0 },
    { VIF__SCPf, 1, 106, 0 },
    { VIF__SRC_DISCARDf, 1, 105, 0 },
    { VIF__STATIC_BITf, 1, 102, 0 },
    { VLAN__ASSOCIATED_DATAf, 54, 54, SOCF_LE },
    { VLAN__CLASS_IDf, 6, 90, SOCF_LE },
    { VLAN__CPUf, 1, 104, 0 },
    { VLAN__DATAf, 54, 54, SOCF_LE },
    { VLAN__DESTINATIONf, 18, 72, SOCF_LE },
    { VLAN__DESTINATION_1f, 18, 54, SOCF_LE },
    { VLAN__DST_DISCARDf, 1, 103, 0 },
    { VLAN__DUMMY_INDEXf, 1, 95, 0 },
    { VLAN__EVPN_AGE_DISABLEf, 1, 107, 0 },
    { VLAN__HASH_LSBf, 16, 8, SOCF_LE },
    { VLAN__IVIDf, 12, 20, SOCF_LE },
    { VLAN__KEYf, 32, 3, SOCF_LE },
    { VLAN__MAC_BLOCK_INDEXf, 5, 90, SOCF_LE },
    { VLAN__OVIDf, 12, 8, SOCF_LE },
    { VLAN__PENDINGf, 1, 101, 0 },
    { VLAN__PRIf, 4, 97, SOCF_LE },
    { VLAN__RESERVEDf, 19, 35, SOCF_LE },
    { VLAN__RESERVED_KEY_PADDINGf, 3, 32, SOCF_LE|SOCF_RES },
    { VLAN__RPEf, 1, 96, 0 },
    { VLAN__SCPf, 1, 106, 0 },
    { VLAN__SRC_DISCARDf, 1, 105, 0 },
    { VLAN__STATIC_BITf, 1, 102, 0 },
    { VLAN_IDf, 12, 8, SOCF_LE }
};
 */
//Memory: L2X.ipipe0 aka L2_ENTRY alias L2X address 0xa8000000
//Flags: valid cachable(on) hashed multiview
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 32768 with indices 0-32767 (0x0-0x7fff), each 14 bytes 4 words
//Entry mask: -1 -1 -1 0x00007fff
//Description: Combined HW managed L2 entry table.  Includes L2_ENTRY, L2_HITDA, and L2_HITSA
#define L2Xm                                0xa8000000
#define L2Xm_BYTES                          14
#define L2Xm_MAX_INDEX                      0x7fff



/*****************************************************************************************/
/*                              switchdev                                                */
/*****************************************************************************************/

typedef struct _bcmsw_switch_s {
    struct net_device *dev; //bcm0
    soc_info_t *si;
    struct bcmsw_switchdev *swdev;

    //soc cancun info
    soc_cancun_t *soc_cancun_info;
    ibde_t       *kernel_bde;

    //ING/EGR_VLAN_VFI_MEMBERSHIP
    //soc_profile_mem_t *egr_vlan_vfi_untag_profile;


    bcm_stg_info_t *stg_info;
} bcmsw_switch_t;


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

struct bcmsw_switchdev {
	bcmsw_switch_t *sw;
	struct list_head bridge_list;
	bool bridge_8021q_exists;
	struct notifier_block swdev_nb_blk;
	struct notifier_block swdev_nb;
};

/* Add an entry to field-value array for multiple fields write */
#define bcmsw_mem_set_field_value_array(_fa, _f, _va, _v, _p) \
    do {                \
        _fa[_p] = _f;   \
        _va[_p] = _v;   \
        _p++;           \
        } while (0);    \



int bcmsw_switch_init(void);
extern int bcm_switch_hw_init(bcmsw_switch_t *bcmsw);
extern int soc_cancun_init (bcmsw_switch_t *swdev);
extern int _ext_phy_probe(bcmsw_switch_t *bcmsw, int port);
extern int phy_bcm542xx_init_setup( bcmsw_switch_t *bcmsw,
                         int port,
                         int reset,
                         int automedium,
                         int fiber_preferred,
                         int fiber_detect,
                         int fiber_enable,
                         int copper_enable,
                         int ext_phy_autodetect_en,
                         int ext_phy_fiber_iface);

extern int bcm_esw_stat_init(bcmsw_switch_t *bcmsw);
extern int phy_bcm542xx_init(bcmsw_switch_t *bcmsw, int port);
extern int phy_bcm542xx_enable_set(port_info_t *pport, int port, uint16_t phy_addr, int enable);
extern int phy_bcm542xx_enable_get(port_info_t *pport, int port, uint16_t phy_addr, int *enable);
extern int phy_bcm542xx_link_get(port_info_t *pport, int port, uint16_t phy_addr, int *link);
extern int phy_bcm542xx_autoneg_get(port_info_t *pport, int port, uint16_t phy_addr, int *autoneg, int *autoneg_done);
extern int _trident3_mdio_rate_divisor_set(void);

extern int  _pm4x10_qtc_port_attach_core_probe (bcmsw_switch_t *bcmsw, int port);
extern int  _pm4x10_qtc_pm_core_init (bcmsw_switch_t *bcmsw, int port);
extern int _pm4x10_qtc_port_attach_resume_fw_load (bcmsw_switch_t *bcmsw, int port);

extern int _procfs_init(bcmsw_switch_t *bcmsw);
extern int _procfs_uninit(bcmsw_switch_t *bcmsw);

extern bcmsw_switch_t *_bcmsw;
#endif
