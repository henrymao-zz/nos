#ifndef _BCM_SWITCHDEV_EXTPHY_H_
#define _BCM_SWITCHDEV_EXTPHY_H_

/*****************************************************************************************/
/*                            PHY related                                                */
/*****************************************************************************************/

#define PHY_FLAGS_COPPER           (1 << 0)  /* copper medium */
#define PHY_FLAGS_FIBER            (1 << 1)  /* fiber medium */
#define PHY_FLAGS_PASSTHRU         (1 << 2)  /* serdes passthru (5690) */
#define PHY_FLAGS_10B              (1 << 3)  /* ten bit interface (TBI) */
#define PHY_FLAGS_5421S            (1 << 4)  /* True if PHY is a 5421S */
#define PHY_FLAGS_DISABLE          (1 << 5)  /* True if PHY is disabled */
#define PHY_FLAGS_C45              (1 << 6)  /* True if PHY uses clause 45
                                              * MIIM */
#define PHY_FLAGS_100FX            (1 << 7)  /* True if PHY at 100FX
                                              * (for 5482S) */
#define PHY_FLAGS_MEDIUM_CHANGE    (1 << 8)  /* True if PHY medium changed
                                              * between copper and fiber */
#define PHY_FLAGS_SERDES_FIBER     (1 << 9)  /* True if use internal serdes
                                              * phy */
#define PHY_FLAGS_WAN              (1 << 10) /* WAN mode */
#define PHY_FLAGS_SGMII_AUTONEG    (1 << 11) /* Use SGMII autoneg between
                                              * internal SerDes and external
                                              * PHY */
#define PHY_FLAGS_EXTERNAL_PHY     (1 << 12) /* Port has external PHY */
#define PHY_FLAGS_FORCED_SGMII     (1 << 13) /* Interface between internal and
                                              * external PHY is always SGMII */
#define PHY_FLAGS_FORCED_COPPER    (1 << 14) /* Forced media to copper */
#define PHY_FLAGS_C73              (1 << 15) /* Auto-negotiation for Backplane
                                              * Ethernet (clause 73) */
#define PHY_FLAGS_INDEPENDENT_LANE (1 << 16) /* Treat each XGXS lane as
                                              * independent lane. */
#define PHY_FLAGS_SINGLE_LANE      (1 << 17) /* Use only lane 0 of XGXS core. */

#define PHY_FLAGS_PRIMARY_SERDES    (1<<18)

#define PHY_FLAGS_SECONDARY_SERDES (1<<19)

#define PHY_FLAGS_INIT_DONE        (1<<20)

#define PHY_FLAGS_HC65_FABRIC      (1<<21)   /* True if PHY used for SBX */
                                             /* fabric links */
#define PHY_FLAGS_EEE_CAPABLE      (1<<22)

#define PHY_FLAGS_EEE_ENABLED      (1<<23)

#define PHY_FLAGS_EEE_MODE         (1<<24)

#define PHY_FLAGS_BR               (1<<25)

#define PHY_FLAGS_HS_CAPABLE       (1<<26)

#define PHY_FLAGS_CHAINED          (1<<27)

#define PHY_FLAGS_REPEATER		   (1<<28)		/* PHY is a repeater */

#define PHY_FLAGS_SUPPORT_DUAL_RATE (1<<29)

#define PHY_FLAGS_SERVICE_INT_PHY_LINK_GET  (1<<30)
#define PHY_FLAGS_NO_SYS_LINE_SPD_SYNC      (1<<31)                                               

#define PHY_BRCM_OUI6           0xd40129        /* Broadcom OUI */
#define PHY_BCM5418X_OUI        PHY_BRCM_OUI6
#define PHY_BCM54182_MODEL      0x01



#define PHY_OUI(id0, id1) \
        _bit_rev_by_byte_word32((uint32)(id0) << 6 | ((id1) >> 10 & 0x3f))

#define PHY_MODEL(id0, id1) ((id1) >> 4 & 0x3f)


/* MII Control Register: bit definitions */

#define MII_CTRL_FS_2500        (1 << 5) /* Force speed to 2500 Mbps */
#define MII_CTRL_SS_MSB         (1 << 6) /* Speed select, MSb */
#define MII_CTRL_CST            (1 << 7) /* Collision Signal test */
#define MII_CTRL_FD             (1 << 8) /* Full Duplex */
#define MII_CTRL_RAN            (1 << 9) /* Restart Autonegotiation */
#define MII_CTRL_IP             (1 << 10) /* Isolate Phy */
#define MII_CTRL_PD             (1 << 11) /* Power Down */
#define MII_CTRL_AE             (1 << 12) /* Autonegotiation enable */
#define MII_CTRL_SS_LSB         (1 << 13) /* Speed select, LSb */
#define MII_CTRL_LE             (1 << 14) /* Loopback enable */
#define MII_CTRL_RESET          (1 << 15) /* PHY reset */


/****************************BCM54182 Phy Register****************************************/
/*
 * Top Misc Global Reset 
 */
#define PHY_BCM542XX_TOP_MISC_GLOBAL_RESET_OFFSET            0x82b
#define PHY_BCM542XX_TOP_MISC_GLOBAL_RESET_TOP_MII_SOFT     (1<<15)
#define PHY_BCM542XX_TOP_MISC_GLOBAL_RESET_TIMESYNC         (1<<10)

#define PHY_BCM542XX_PHYADDR_REVERSE                    (1<<0)
#define PHY_BCM542XX_REAR_HALF                          (1<<1)
#define PHY_BCM542XX_SYS_SIDE_AUTONEG                   (1<<2)

#define PHY_PORTS_PER_QSGMII        4
#define PHY_QSGMII0_HEAD            0x0
#define PHY_QSGMII1_HEAD            (PHY_QSGMII0_HEAD + PHY_PORTS_PER_QSGMII)

/*
 * Top Misc Top Cfg 
 */

#define PHY_BCM542XX_TOP_MISC_TOP_CFG_REG_OFFSET      0x810
#define PHY_BCM542XX_TOP_MISC_CFG_REG_QSGMII_PHYA    (1<<3)
#define PHY_BCM542XX_TOP_MISC_CFG_REG_QSGMII_SEL     (1<<2)
 
#define PHY_BCM542XX_MII_CTRL_REG                     0x00

/* MII Control Register: bit definitions */
#define PHY_BCM542XX_MII_CTRL_UNIDIR_EN      (1<<5)  /* Force speed to 2500 Mbps */
#define PHY_BCM542XX_MII_CTRL_SS_MSB         (1<<6)  /* Speed select, MSb */
#define PHY_BCM542XX_MII_CTRL_FD             (1<<8)  /* Full Duplex */
#define PHY_BCM542XX_MII_CTRL_RST_AN         (1<<9)  /* Restart Autonegotiation */
#define PHY_BCM542XX_MII_CTRL_PWR_DOWN       (1<<11) /* Power Down */
#define PHY_BCM542XX_MII_CTRL_AN_EN          (1<<12) /* Autonegotiation enable */
#define PHY_BCM542XX_MII_CTRL_SS_LSB         (1<<13) /* Speed select, LSb */
#define PHY_BCM542XX_MII_CTRL_LPBK_EN        (1<<14) /* Loopback enable */
#define PHY_BCM542XX_MII_CTRL_RESET          (1<<15) /* PHY reset */

/* 
 * MII Status Register: See 802.3, 1998 pg 544 
 */
#define PHY_BCM542XX_MII_STAT_EXT            (1 << 0) /* Extended Registers */
#define PHY_BCM542XX_MII_STAT_JBBR           (1 << 1) /* Jabber Detected */
#define PHY_BCM542XX_MII_STAT_LA             (1 << 2) /* Link Active */
#define PHY_BCM542XX_MII_STAT_AN_CAP         (1 << 3) /* Autoneg capable */
#define PHY_BCM542XX_MII_STAT_RF             (1 << 4) /* Remote Fault */
#define PHY_BCM542XX_MII_STAT_AN_DONE        (1 << 5) /* Autoneg complete */
#define PHY_BCM542XX_MII_STAT_MF_PS          (1 << 6) /* Preamble suppression */
#define PHY_BCM542XX_MII_STAT_ES             (1 << 8) /* Extended status (R15) */
#define PHY_BCM542XX_MII_STAT_HD_100_T2      (1 << 9) /* Half duplex 100Mb/s supported */
#define PHY_BCM542XX_MII_STAT_FD_100_T2      (1 << 10)/* Full duplex 100Mb/s supported */
#define PHY_BCM542XX_MII_STAT_HD_10          (1 << 11)/* Half duplex 100Mb/s supported */
#define PHY_BCM542XX_MII_STAT_FD_10          (1 << 12)/* Full duplex 100Mb/s supported */
#define PHY_BCM542XX_MII_STAT_HD_100         (1 << 13)/* Half duplex 100Mb/s supported */
#define PHY_BCM542XX_MII_STAT_FD_100         (1 << 14)/* Full duplex 100Mb/s supported */
#define PHY_BCM542XX_MII_STAT_100_T4         (1 << 15)/* Full duplex 100Mb/s supported */

/*
 * MII Extended Control Register (BROADCOM)
 */
#define PHY_BCM542XX_MII_ECR_FIFO_ELAST_0    (1<<0) /* FIFO Elasticity[0], MSB
                                                             at exp reg 46[14]*/
#define PHY_BCM542XX_MII_ECR_LED_OFF_F       (1<<3) /* Force LED off */
#define PHY_BCM542XX_MII_ECR_LED_ON_F        (1<<4) /* Force LED on */
#define PHY_BCM542XX_MII_ECR_EN_LEDT         (1<<5) /* Enable LED traffic */
#define PHY_BCM542XX_MII_ECR_RST_SCR         (1<<6) /* Reset Scrambler */
#define PHY_BCM542XX_MII_ECR_BYPASS_ALGN     (1<<7) /* Bypass Receive Sym. align */
#define PHY_BCM542XX_MII_ECR_BPASS_MLT3      (1<<8) /* Bypass MLT3 Encoder/Decoder */
#define PHY_BCM542XX_MII_ECR_BPASS_SCR       (1<<9) /* Bypass Scramble/Descramble */
#define PHY_BCM542XX_MII_ECR_BPASS_ENC       (1<<10) /* Bypass 4B/5B Encode/Decode */
#define PHY_BCM542XX_MII_ECR_FORCE_INT       (1<<11) /* Force Interrupt */
#define PHY_BCM542XX_MII_ECR_INT_DIS         (1<<12) /* Interrupt Disable */
#define PHY_BCM542XX_MII_ECR_TX_DIS          (1<<13) /* XMIT Disable */
#define PHY_BCM542XX_MII_ECR_DAMC            (1<<14) /* Disable Auto-MDI Crossover */

#define PHY_BCM542XX_PATT_GEN_STAT_FIFO_ELAST_1        (1<<14)
#define PHY_BCM542XX_PATT_GEN_STAT_GMII_FIFO_JUMBO_MSB (1<<15)

/*
 * DISABLE_USGMII
*/   
#define PHY_BCM542XX_DISABLE_USGMII                  (0x0810)
#define PHY_BCM542XX_DISABLE_USGMII_6U_BIT_SEL       (1<<6)

#define PHY_BCM542XX_SERDES_CTRL_JUMBO_MSB      (1<<0)
#define PHY_BCM542XX_SERDES_CTRL_AN_PD_EN       (1<<1)

/* AUTO Detect SGMII MC Reg., RDB_0x238 */
#define PHY_BCM542XX_AUTO_DETECT_SGMII_MC_REG_OFFSET  0x238
#define PHY_BCM542XX_AUTO_DETECT_GMII_FIFO_JUMBO_LSB  (1<<2)

#define PHY_BCM542XX_SERDES_CTRL_JUMBO_LSB      (1<<1)

/* *                                   
 * *         S2S: Second Serdes Registers          
 */
#define PHY_BCM542XX_2ND_SERDES_BASE_OFFSET  (0xb00)

/*
 * Second SERDES MISC 1000-X CONTROL 2 REGISTER
 */
#define PHY_BCM542XX_2ND_SERDES_MISC_1000x_CTL_2_REG_OFFSET  \
                            (PHY_BCM542XX_2ND_SERDES_BASE_OFFSET + 0x16)
#define PHY_BCM542XX_2ND_SERDES_MISC_1000x_CTL_2_2ND_SERDES_MSB  (1<<0)

/*
 * Second SERDES AUXILIARY 1000-X CONTROL REGISTER
 */
#define PHY_BCM542XX_2ND_SERDES_AUX_1000x_CTL_REG_OFFSET  \
                            (PHY_BCM542XX_2ND_SERDES_BASE_OFFSET + 0x1B)
#define PHY_BCM542XX_2ND_SERDES_AUX_1000x_CTL_2ND_SERDES_LSB     (1<<1)
/*
 * Second SERDES 1000-X ERROR COUNTER SETTING REGISTER
 */
#define PHY_BCM542XX_2ND_SERDES_1000x_ERR_CNT_SET_REG_OFFSET  \
                            (PHY_BCM542XX_2ND_SERDES_BASE_OFFSET + 0x10)
#define PHY_BCM542XX_2ND_SERDES_1000x_ERR_CNT_SET_EXT_PKT_LEN    (1<<0)


 /* RDB_Reg. 0x028 */
#define PHY_BCM542XX_MII_AUX_CTRL_REG_EXT_PKT_LEN                (1<<14)
#define PHY_BCM542XX_MII_AUX_CTRL_REG_EN_DSP_CLK                 (1<<11)

#define BCM542XX_AUTO_PWR_DOWN_DLL_DIS                  (1<<1)

#define BMACSEC_TOP_MISC_REG_SIZE 1
#define PHY_BCM542XX_TOP_MISC_MII_BUFF_CNTL_PHY0 0x800
#define PHY_BCM542XX_TOP_MISC_MII_BUFF_CNTL_PHYN(n) \
                      (PHY_BCM542XX_TOP_MISC_MII_BUFF_CNTL_PHY0 + \
                       BMACSEC_TOP_MISC_REG_SIZE*(n)*2/*skip one reg*/)

/*
 * AUXILIARY CONTROL REG
 */
#define PHY_BCM542XX_AUX_CTRL_REG_OFFSET              0x028

/*
 * DSP_TAP10
 */
#define PHY_BCM542XX_DSP_TAP10_REG_OFFSET             0x125

/*
 * COPPER_POWER_MII_CTRL Register
 */
#define PHY_BCM542XX_POWER_MII_CTRL_REG_OFFSET        0x02A
#define PHY_BCM542XX_POWER_MII_CTRL_SUPER_ISOLATE    (1<<5)

/*
 * LED GPIO CTRL_STATUS
 */
#define PHY_BCM542XX_LED_GPIO_CTRL_STATUS_REG_OFFSET  0x01F

/* Registers used to Rd/Wr using RDB mode */
#define PHY_BCM542XX_RDB_ADDR_REG_OFFSET        (0x1E)
#define PHY_BCM542XX_RDB_ADDR_REG_ADDR          (0xffff)
#define PHY_BCM542XX_RDB_DATA_REG_OFFSET        (0x1F)
#define PHY_BCM542XX_RDB_DATA_REG_DATA          (0xffff)
/* Registers used to enable RDB register access mode */
#define PHY_BCM542XX_REG_17_OFFSET              (0x17)
#define PHY_BCM542XX_REG_17_SELECT_EXP_7E       (0x0F7E)
#define PHY_BCM542XX_REG_15_OFFSET              (0x15)
#define PHY_BCM542XX_REG_15_RDB_EN              (0x0000)
#define PHY_BCM542XX_REG_15_RDB_DIS             (0x8000)
#define PHY_BCM542XX_REG_1E_SELECT_RDB          (0x0087)

#endif
