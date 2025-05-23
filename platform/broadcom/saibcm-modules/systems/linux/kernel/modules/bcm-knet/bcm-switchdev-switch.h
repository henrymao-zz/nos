#ifndef _BCM_SWITCHDEV_SWITCH_H_
#define _BCM_SWITCHDEV_SWITCH_H_

/* Environment switch */
#define _PCID_TEST 0

/* Local defines */
#define BYTES_PER_UINT32    (sizeof(uint32))

extern char *_shr_errmsg[];


/*****************************************************************************************/
/*                              SOC                                                      */
/*****************************************************************************************/
#define COMPILER_64_HI(src)     ((uint32) ((src) >> 32))
#define COMPILER_64_LO(src)     ((uint32) (src))
#define COMPILER_64_ZERO(dst)       ((dst) = 0)
#define COMPILER_64_IS_ZERO(src)    ((src) == 0)
                                       

#define COMPILER_64_SET(dst, src_hi, src_lo)                \
    ((dst) = (((uint64) ((uint32)(src_hi))) << 32) | ((uint64) ((uint32)(src_lo))))

#define COMPILER_64_AND(dst, src)    ((dst) &= (src))
#define COMPILER_64_OR(dst, src)    ((dst) |= (src))
#define COMPILER_64_XOR(dst, src)    ((dst) ^= (src))
#define COMPILER_64_NOT(dst)        ((dst) = ~(dst))

#define COMPILER_64_SHL(dst, bits)    ((dst) <<= (bits))
#define COMPILER_64_SHR(dst, bits)    ((dst) >>= (bits))

#define COMPILER_64_BITSET(dst, n)              \
        do {                                    \
            uint64 temp64;                      \
            COMPILER_64_SET(temp64, 0, 1);      \
            COMPILER_64_SHL(temp64, n);         \
            COMPILER_64_OR(dst, temp64);        \
        } while(0)

#define COMPILER_64_BITCLR(dst, n)              \
        do {                                    \
            uint64 temp64;                      \
            COMPILER_64_SET(temp64, 0, 1);      \
            COMPILER_64_SHL(temp64, n);         \
            COMPILER_64_NOT(temp64);            \
            COMPILER_64_AND(dst, temp64);       \
        } while(0)


#define SOCF_LE                              0x01    /* little endian */
#define SOC_MEM_FLAG_BE                      0x08000000 /* Big endian */

#define FIX_MEM_ORDER_E(v, bytes, flags) ((flags & SOC_MEM_FLAG_BE) ? \
                                bytes-1-(v) : \
                                (v))

typedef enum {
    SOC_E_NONE                  = 0,
    SOC_E_INTERNAL              = -1,
    SOC_E_MEMORY                = -2,
    SOC_E_UNIT                  = -3,
    SOC_E_PARAM                 = -4,
    SOC_E_EMPTY                 = -5,
    SOC_E_FULL                  = -6,
    SOC_E_NOT_FOUND             = -7,
    SOC_E_EXISTS                = -8,
    SOC_E_TIMEOUT               = -9,
    SOC_E_BUSY                  = -10,
    SOC_E_FAIL                  = -11,
    SOC_E_DISABLED              = -12,
    SOC_E_BADID                 = -13,
    SOC_E_RESOURCE              = -14,
    SOC_E_CONFIG                = -15,
    SOC_E_UNAVAIL               = -16,
    SOC_E_INIT                  = -17,
    SOC_E_PORT                  = -18,
    SOC_E_LIMIT                 = -19
} soc_error_t;

#define _SHR_ERRMSG_INIT        { \
        "Ok",                           /* E_NONE */ \
        "Internal error",               /* E_INTERNAL */ \
        "Out of memory",                /* E_MEMORY */ \
        "Invalid unit",                 /* E_UNIT */ \
        "Invalid parameter",            /* E_PARAM */ \
        "Table empty",                  /* E_EMPTY */ \
        "Table full",                   /* E_FULL */ \
        "Entry not found",              /* E_NOT_FOUND */ \
        "Entry exists",                 /* E_EXISTS */ \
        "Operation timed out",          /* E_TIMEOUT */ \
        "Operation still running",      /* E_BUSY */ \
        "Operation failed",             /* E_FAIL */ \
        "Operation disabled",           /* E_DISABLED */ \
        "Invalid identifier",           /* E_BADID */ \
        "No resources for operation",   /* E_RESOURCE */ \
        "Invalid configuration",        /* E_CONFIG */ \
        "Feature unavailable",          /* E_UNAVAIL */ \
        "Feature not initialized",      /* E_INIT */ \
        "Invalid port",                 /* E_PORT */ \
        "Unknown error"                 /* E_LIMIT */ \
        }

//extern char *_shr_errmsg[];

#define soc_errmsg(r)          \
        _shr_errmsg[(((int)r) <= 0 && ((int)r) > SOC_E_LIMIT) ? -(r) : -SOC_E_LIMIT]

typedef int soc_mem_t;

#if defined(LE_HOST)

#define soc_letohl(_l)  (_l)
#define soc_letohs(_s)  (_s)

#else /* BE_HOST */

#define soc_letohl(_l)  _shr_swap32(_l)
#define soc_letohs(_s)  _shr_swap16(_s)

#endif /* BE_HOST */

#define _SHR_ERROR_TRACE(rv) 
#define SOC_IF_ERROR_RETURN(op) \
    do { int __rv__; if ((__rv__ = (op)) < 0) { _SHR_ERROR_TRACE(__rv__);  return(__rv__); } } while(0)


#define BYTES2WORDS(x)        (((x) + 3) / 4)

#define SOC_MAX_MEM_BYTES               78
#define SOC_MAX_MEM_WORDS               BYTES2WORDS(SOC_MAX_MEM_BYTES)


#define MEM_BLOCK_ALL                  -1

#define SOC_WARM_BOOT(unit)             0 

/* TD3 specific defines */
#define HELIX5_PORTS_PER_PBLK             4
#define HELIX5_PBLKS_PER_PIPE             19
#define HELIX5_PBLKS_PER_HPIPE            9
#define HELIX5_PIPES_PER_DEV              1
#define HELIX5_XPES_PER_DEV               1
#define HELIX5_GPHY_PORTS_PER_PIPE        \
    (HELIX5_PORTS_PER_PBLK * HELIX5_PBLKS_PER_PIPE)
#define HELIX5_PHY_PORTS_PER_PIPE         (HELIX5_GPHY_PORTS_PER_PIPE + 3)
#define HELIX5_PBLKS_PER_DEV              \
    (HELIX5_PBLKS_PER_PIPE * HELIX5_PIPES_PER_DEV)
#define HELIX5_PHY_PORTS_PER_DEV          \
    (HELIX5_PHY_PORTS_PER_PIPE * HELIX5_PIPES_PER_DEV)

#define HELIX5_TDM_PORTS_PER_PBLK         4
#define HELIX5_TDM_PBLKS_PER_PIPE         19
#define HELIX5_TDM_PBLKS_PER_HPIPE        8
#define HELIX5_TDM_PIPES_PER_DEV          1
#define HELIX5_TDM_HPIPES_PER_PIPE        2

#define HELIX5_TDM_PBLKS_PER_DEV          \
                 (HELIX5_TDM_PBLKS_PER_PIPE * HELIX5_TDM_PIPES_PER_DEV)
#define HELIX5_TDM_GPORTS_PER_HPIPE       \
                 (HELIX5_TDM_PORTS_PER_PBLK * HELIX5_TDM_PBLKS_PER_HPIPE)
#define HELIX5_TDM_GPORTS_PER_PIPE        \
                 (HELIX5_TDM_PORTS_PER_PBLK * HELIX5_TDM_PBLKS_PER_PIPE)
#define HELIX5_TDM_GPORTS_PER_DEV         \
                 (HELIX5_TDM_GPORTS_PER_PIPE * HELIX5_TDM_PIPES_PER_DEV)

/* Physical port */
/* 64 General device port + 1 CPU + 1 FAE + 1 Loopback */
#define HELIX5_TDM_PHY_PORTS_PER_PIPE     \
                 (HELIX5_TDM_GPORTS_PER_PIPE + 3)
#define HELIX5_TDM_PHY_PORTS_PER_DEV      \
                 (HELIX5_TDM_PHY_PORTS_PER_PIPE * HELIX5_TDM_PIPES_PER_DEV)

#define HELIX5_PHY_IS_FRONT_PANEL_PORT(p)        ((p>=1)&& (p<=76))
#define HELIX5_PHY_PORT_CPU                      0
#define HELIX5_PHY_PORT_LPBK0                    77

#define HX5_NUM_EXT_PORTS                        79

#define HX5_MMU_PORT_PIPE_OFFSET                 72
#define HX5_MMU_FLUSH_OFF                        0
#define HX5_MMU_FLUSH_ON                         1

/*****************************************************************************************/
/*                              COMMON type definition                                   */
/*****************************************************************************************/


#define _SHR_PBMP_PORT_MAX 128
#define _SHR_PBMP_WIDTH                 (((_SHR_PBMP_PORT_MAX + 32 - 1)/32)*32)
#define _SHR_PBMP_WORD_WIDTH            32
#define _SHR_PBMP_WORD_MAX              \
      ((_SHR_PBMP_WIDTH + _SHR_PBMP_WORD_WIDTH-1) / _SHR_PBMP_WORD_WIDTH)

typedef struct _shr_pbmp {
    uint32_t    pbits[_SHR_PBMP_WORD_MAX];
} bcm_pbmp_t;
    
typedef int bcm_port_t;
typedef int bcm_module_t;
typedef int bcm_trunk_t;
typedef int bcm_cos_t;
typedef int bcm_if_t;
typedef int bcm_multicast_t;
typedef uint16_t bcm_vlan_t;

/* bcm_mac_t */
typedef uint8_t bcm_mac_t[6];

typedef struct bcm_flow_logical_field_s {
    uint32_t id;      /* logical field id. */
    uint32_t value;   /* logical field value. */
} bcm_flow_logical_field_t;

#define BCM_FLOW_MAX_NOF_LOGICAL_FIELDS 20

/* TSN flow set */
typedef int bcm_tsn_flowset_t;

/* SR flow set */
typedef int bcm_tsn_sr_flowset_t;

/* bcm_policer_t */
typedef int bcm_policer_t;

/* TSN priority map id */
typedef int bcm_tsn_pri_map_t;



/*****************************************************************************************/
/*                           Registers (read through SCHAN)                              */
/*****************************************************************************************/

//BCM56370
#define SCHAN_BLK_IPIPE     1
#define SCHAN_BLK_EPIPE     2
#define SCHAN_BLK_MMU_XPE   3
#define SCHAN_BLK_MMU_SC    4
#define SCHAN_BLK_MMU_GLB   5
#define SCHAN_BLK_MMU_SED   6
#define SCHAN_BLK_TOP       7

#define SCHAN_BLK_PMQPORT0  18   
#define SCHAN_BLK_PMQPORT1  22 
#define SCHAN_BLK_PMQPORT2  26 
#define SCHAN_BLK_CLPORT0   31
#define SCHAN_BLK_CLPORT1   32
#define SCHAN_BLK_CLPORT2   33
#define SCHAN_BLK_XLPORT0   15
#define SCHAN_BLK_XLPORT1   19
#define SCHAN_BLK_XLPORT2   23
#define SCHAN_BLK_XLPORT6   30

#define SCHAN_BLK_GXPORT0   16
#define SCHAN_BLK_GXPORT1   17
#define SCHAN_BLK_GXPORT2   20
#define SCHAN_BLK_GXPORT3   21
#define SCHAN_BLK_GXPORT4   24
#define SCHAN_BLK_GXPORT5   25

#define TOP_SOFT_RESET_REGr            0x02000100
#define TOP_SOFT_RESET_REG_2r          0x02000200
#define TOP_CORE_CLK_FREQ_SELr         0x02007700

/*
soc_field_info_t soc_CHIP_CONFIG_BCM56370_A0r_fields[] = {
    { IP_TDMf, 3, 1, SOCF_LE },
    { PAUSE_PFC_SELf, 1, 4, 0 },
    { PCS_USXGMII_MODE_ENf, 1, 5, 0 },
    { PMD_PLL_CTRL_REFCLK_DIV2f, 1, 8, 0 },
    { PMD_PLL_CTRL_REFCLK_DIV4f, 1, 9, 0 },
    { PMD_PLL_CTRL_REFCLK_TERM_SELf, 2, 6, SOCF_LE },
    { POWERSAVEf, 1, 31, 0 },
    { QMODEf, 1, 0, 0 }
};
 */
typedef union chip_config_s {
    struct _ch_addr_ {
    #if defined(LE_HOST)
    uint32_t  QMODEf:1,
              IP_TDMf:3,
              PAUSE_PFC_SELf:1,
              PCS_USXGMII_MODE_ENf:1,
              PMD_PLL_CTRL_REFCLK_TERM_SELf:2,
              PMD_PLL_CTRL_REFCLK_DIV2f:1,
              PMD_PLL_CTRL_REFCLK_DIV4f:1,
              r0:21,
              POWERSAVEf:1;
    #else
    uint32_t  POWERSAVEf:1,
              r0:21,
              PMD_PLL_CTRL_REFCLK_DIV4f:1,
              PMD_PLL_CTRL_REFCLK_DIV2f:1,
              PMD_PLL_CTRL_REFCLK_TERM_SELf:2,
              PCS_USXGMII_MODE_ENf:1,
              PAUSE_PFC_SELf:1,
              IP_TDMf:3,
              QMODEf:1;
    #endif
    }reg;
    uint32_t word;
} chip_config_t;
#define CHIP_CONFIGr                   0x2020000
#define TOP_MISC_GENERIC_CONTROLr      0x2008600
#define EGR_PORT_BUFFER_SFT_RESET_0r   0x2b130000
#define IDB_SER_CONTROL_64r            0x2280000




#define IS_TDM_CONFIG_PIPE0r           0x6040100
#define IS_OPP_SCHED_CFG_PIPE0r        0x6040500


#define EGR_HW_RESET_CONTROL_0r        0x3000000
#define EGR_HW_RESET_CONTROL_1r        0x3010000

/*
soc_field_info_t soc_ING_HW_RESET_CONTROL_1_BCM56960_A0r_fields[] = {
    { MEMIDX_OFFSETf, 26, 0, SOCF_LE },
    { MEMORY_NUMBERf, 8, 18, SOCF_LE },
    { OFFSETf, 18, 0, SOCF_LE },
    { STAGE_NUMBERf, 6, 26, SOCF_LE }
 */
#define ING_HW_RESET_CONTROL_1r        0x2230000

/*
soc_field_info_t soc_ING_HW_RESET_CONTROL_2_BCM56970_A0r_fields[] = {
    { COUNTf, 19, 0, SOCF_LE },
    { DONEf, 1, 22, SOCF_RES },
    { REGIONf, 1, 20, 0 },
    { RESET_ALLf, 1, 19, 0 },
    { VALIDf, 1, 21, 0 }
};
 */
#define ING_HW_RESET_CONTROL_2r        0x2240000

#define IDB_HW_CONTROLr                0x2200000
#define CLPORT_MIB_RESETr              0x2022400
#define CLPORT_XGXS0_CTRL_REGr         0x2021400
#define CLPORT_MAC_CONTROLr            0x2021000
#define XLPORT_MIB_RESETr              0x2022400
#define XLPORT_XGXS0_CTRL_REGr         0x2021400

/*
soc_field_info_t soc_XLPORT_XGXS0_CTRL_REG_BCM56980_A0r_fields[] = {
    { IDDQf, 1, 4, SOCF_RES },
    { PWRDWNf, 1, 3, SOCF_RES },
    { PWRDWN_CMLf, 1, 5, SOCF_RES },
    { PWRDWN_CML_LCf, 1, 6, SOCF_RES },
    { REFCMOSf, 1, 7, SOCF_RES },
    { REFIN_ENf, 1, 2, SOCF_RES },
    { REFOUT_ENf, 1, 1, SOCF_RES },
    { REFSELf, 3, 8, SOCF_LE|SOCF_RES },
    { RSTB_HWf, 1, 0, SOCF_RES }
};
 */
typedef union xgxs0_ctrl_reg_s {
    struct _xgxs0_ctrl_ {
    #if defined(LE_HOST)
    uint32_t  RSTB_HWf:1,
              EREFOUT_Nf:1,
              REFIN_ENf:1,
              IDDQf:1,
              PWRDWNf:1,
              PWRDWN_CMLf:1,
              PWRDWN_CML_LCf:1,
              REFCMOSf:1,
              REFSELf:3,
              r0:21;
    #else
    uint32_t  r0:21,
              REFSELf:3,
              REFCMOSf:1,
              PWRDWN_CML_LCf:1,
              PWRDWN_CMLf:1,
              PWRDWNf:1,
              IDDQf:1,
              REFIN_ENf:1
              EREFOUT_Nf:1,
              RSTB_HWf:1;
    #endif
    }reg;
    uint32_t word;
} xgxs0_ctrl_reg_t;
#define PMQ_XGXS0_CTRL_REGr            0x2020100
#define ENHANCED_HASHING_CONTROL_2r    0x82001300
#define XLPORT_MAC_CONTROLr            0x2021000

//Register: CHIP_SWRST.pmqport0 general register address 0x02020800
//Blocks: pmqport0 pmqport1 pmqport2 (3 copies)
//Description: Soft Reset register for PM4x10Q Top
//Displaying: reset defaults, reset value 0 mask 0xff
//  SOFT_RESET_QSGMII_PCS<2> = 0
//  SOFT_RESET_GPORT1<1> = 0
//  SOFT_RESET_GPORT0<0> = 0
//  ILKN_BYPASS_RSTN<7:4> = 0
//  FLUSH<3> = 0
/*
soc_field_info_t soc_CHIP_SWRST_BCM53400_A0r_fields[] = {
    { FLUSHf, 1, 3, 0 },
    { ILKN_BYPASS_RSTNf, 4, 4, SOCF_LE },
    { SOFT_RESET_GPORT0f, 1, 0, 0 },
    { SOFT_RESET_GPORT1f, 1, 1, 0 },
    { SOFT_RESET_QSGMII_PCSf, 1, 2, 0 }
};
 */
typedef union chip_swrst_reg_s {
    struct _chip_swrst_ {
    #if defined(LE_HOST)
    uint32_t  SOFT_RESET_GPORT0f:1,
              SOFT_RESET_GPORT1f:1,
              SOFT_RESET_QSGMII_PCSf:1,
              FLUSHf:1,
              ILKN_BYPASS_RSTNf:4,
              r0:24;
    #else
    uint32_t  r0:24,
              ILKN_BYPASS_RSTNf:4,
              FLUSHf:1,
              SOFT_RESET_QSGMII_PCSf:1,
              SOFT_RESET_GPORT1f:1,
              SOFT_RESET_GPORT0f:1;
    #endif
    }reg;
    uint32_t word;
} chip_swrst_reg_t;
#define CHIP_SWRSTr                    0x02020800

/*****************************************************************************************/
/*                            memory entries read from SCHAN                             */
/*****************************************************************************************/
#define MEM_ENTRY(tname, bytes) \
        typedef struct { \
                uint32 entry_data[BYTES2WORDS(bytes)]; \
        } tname


/*
soc_field_info_t soc_LPORT_TAB_BCM56370_A0m_fields[] = {
    { ALLOW_SRC_MODf, 1, 334, 0 | SOCF_GLOBAL },
    { CFI_0_MAPPINGf, 1, 226, 0 | SOCF_GLOBAL },
    { CFI_1_MAPPINGf, 1, 227, 0 | SOCF_GLOBAL },
    { CLASS_BASED_SM_ENABLEf, 1, 252, 0 | SOCF_GLOBAL },
    { CML_BMAC_MOVEf, 2, 265, SOCF_LE | SOCF_GLOBAL },
    { CML_BMAC_NEWf, 2, 263, SOCF_LE | SOCF_GLOBAL },
    { CML_FLAGS_MOVEf, 4, 257, SOCF_LE | SOCF_GLOBAL },
    { CML_FLAGS_NEWf, 4, 253, SOCF_LE | SOCF_GLOBAL },
    { CTRL_PROFILE_INDEX_1588f, 6, 286, SOCF_LE | SOCF_GLOBAL },
    { DATA_0f, 104, 0, SOCF_LE | SOCF_GLOBAL },
    { DATA_1f, 104, 104, SOCF_LE | SOCF_GLOBAL },
    { DATA_2f, 104, 208, SOCF_LE | SOCF_GLOBAL },
    { DATA_3f, 104, 312, SOCF_LE | SOCF_GLOBAL },
    { DESTINATIONf, 18, 374, SOCF_LE | SOCF_GLOBAL },
    { DISABLE_STATIC_MOVE_DROPf, 1, 262, 0 | SOCF_GLOBAL },
    { DISABLE_VLAN_CHECKSf, 1, 261, 0 | SOCF_GLOBAL },
    { DISCARD_IF_VNTAG_NOT_PRESENTf, 1, 332, 0 | SOCF_GLOBAL },
    { DISCARD_IF_VNTAG_PRESENTf, 1, 331, 0 | SOCF_GLOBAL },
    { DOT1P_REMAP_POINTERf, 6, 365, SOCF_LE | SOCF_GLOBAL },
    { DROP_BPDUf, 1, 235, 0 | SOCF_GLOBAL },
    { ECCP_0f, 8, 416, SOCF_LE | SOCF_GLOBAL },
    { ECCP_1f, 8, 424, SOCF_LE | SOCF_GLOBAL },
    { ECCP_2f, 8, 432, SOCF_LE | SOCF_GLOBAL },
    { ECCP_3f, 8, 440, SOCF_LE | SOCF_GLOBAL },
    { ECC_0f, 7, 416, SOCF_LE | SOCF_GLOBAL },
    { ECC_1f, 7, 424, SOCF_LE | SOCF_GLOBAL },
    { ECC_2f, 7, 432, SOCF_LE | SOCF_GLOBAL },
    { ECC_3f, 7, 440, SOCF_LE | SOCF_GLOBAL },
    { EN_IFILTERf, 2, 228, SOCF_LE | SOCF_GLOBAL },
    { ETAG_DEf, 1, 154, 0 | SOCF_GLOBAL },
    { ETAG_PCPf, 3, 155, SOCF_LE | SOCF_GLOBAL },
    { ETAG_PCP_DE_MAPPING_PTRf, 6, 193, SOCF_LE | SOCF_GLOBAL },
    { ETAG_PCP_DE_SOURCEf, 2, 162, SOCF_LE | SOCF_GLOBAL },
    { FCOE_DO_NOT_LEARNf, 1, 355, 0 | SOCF_GLOBAL },
    { FCOE_FABRIC_IDf, 12, 168, SOCF_LE | SOCF_GLOBAL },
    { FCOE_FABRIC_PRIf, 3, 165, SOCF_LE | SOCF_GLOBAL },
    { FCOE_FABRIC_SELf, 2, 180, SOCF_LE | SOCF_GLOBAL },
    { FCOE_NETWORK_PORTf, 1, 354, 0 | SOCF_GLOBAL },
    { FCOE_NPV_MODEf, 1, 393, 0 | SOCF_GLOBAL },
    { FCOE_ROUTE_ENABLEf, 1, 360, 0 | SOCF_GLOBAL },
    { FCOE_SRC_BIND_CHECK_ENABLEf, 1, 357, 0 | SOCF_GLOBAL },
    { FCOE_SRC_FPMA_PREFIX_CHECK_ENABLEf, 1, 356, 0 | SOCF_GLOBAL },
    { FCOE_VFT_ENABLEf, 1, 359, 0 | SOCF_GLOBAL },
    { FCOE_VFT_PRI_MAP_PROFILEf, 1, 164, 0 | SOCF_GLOBAL },
    { FCOE_VT_2_MISS_DROP_KEY_1f, 1, 22, 0 | SOCF_GLOBAL },
    { FCOE_VT_2_MISS_DROP_KEY_1_AND_KEY_2f, 1, 20, 0 | SOCF_GLOBAL },
    { FCOE_VT_2_MISS_DROP_KEY_2f, 1, 21, 0 | SOCF_GLOBAL },
    { FCOE_VT_KEY_TYPE_1f, 5, 187, SOCF_LE | SOCF_GLOBAL },
    { FCOE_VT_KEY_TYPE_2f, 5, 182, SOCF_LE | SOCF_GLOBAL },
    { FCOE_VT_MISS_DROP_KEY_1f, 1, 150, 0 | SOCF_GLOBAL },
    { FCOE_VT_MISS_DROP_KEY_1_AND_KEY_2f, 1, 148, 0 | SOCF_GLOBAL },
    { FCOE_VT_MISS_DROP_KEY_2f, 1, 149, 0 | SOCF_GLOBAL },
    { FCOE_ZONE_CHECK_ENABLEf, 1, 358, 0 | SOCF_GLOBAL },
    { FILTER_ENABLEf, 1, 251, 0 | SOCF_GLOBAL },
    { FLEX_CTR_BASE_COUNTER_IDXf, 10, 77, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_OFFSET_MODEf, 2, 75, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_POOL_NUMBERf, 3, 70, SOCF_LE | SOCF_GLOBAL },
    { ICFIf, 1, 90, 0 | SOCF_GLOBAL },
    { ICFI_0_MAPPINGf, 1, 121, 0 | SOCF_GLOBAL },
    { ICFI_1_MAPPINGf, 1, 120, 0 | SOCF_GLOBAL },
    { IEEE_802_1AS_ENABLEf, 1, 201, 0 | SOCF_GLOBAL },
    { IFP_KEY_SEL_CLASS_IDf, 8, 319, SOCF_LE | SOCF_GLOBAL },
    { IPMC_DO_VLANf, 1, 234, 0 | SOCF_GLOBAL },
    { IPRIf, 3, 65, SOCF_LE | SOCF_GLOBAL },
    { IPRI_MAPPINGf, 24, 122, SOCF_LE | SOCF_GLOBAL },
    { IVIDf, 12, 34, SOCF_LE | SOCF_GLOBAL },
    { L2GRE_TERMINATION_ALLOWEDf, 1, 398, 0 | SOCF_GLOBAL },
    { L2GRE_VPNID_LOOKUP_KEY_TYPEf, 1, 199, 0 | SOCF_GLOBAL },
    { MAC_BASED_VID_ENABLEf, 1, 240, 0 | SOCF_GLOBAL },
    { MAC_IP_BIND_LOOKUP_MISS_DROPf, 1, 361, 0 | SOCF_GLOBAL },
    { MAC_IP_BIND_VT2_LOOKUP_MISS_DROPf, 1, 362, 0 | SOCF_GLOBAL },
    { MIM_TERM_ENABLEf, 1, 64, 0 | SOCF_GLOBAL },
    { MIRRORf, 4, 230, SOCF_LE | SOCF_GLOBAL },
    { MISC_PORT_PROFILE_INDEXf, 8, 336, SOCF_LE | SOCF_GLOBAL },
    { MPLS_ENABLEf, 1, 400, 0 | SOCF_GLOBAL },
    { NIV_NAMESPACEf, 12, 299, SOCF_LE | SOCF_GLOBAL },
    { NIV_RPF_CHECK_ENABLEf, 1, 200, 0 | SOCF_GLOBAL },
    { NIV_UPLINK_PORTf, 1, 91, 0 | SOCF_GLOBAL },
    { NIV_VIF_IDf, 12, 92, SOCF_LE | SOCF_GLOBAL },
    { NIV_VIF_LOOKUP_ENABLEf, 1, 104, 0 | SOCF_GLOBAL },
    { OCFIf, 1, 69, 0 | SOCF_GLOBAL },
    { OFFSET_ECMP_LEVEL2_RANDOM_LBf, 4, 409, SOCF_LE | SOCF_GLOBAL },
    { OFFSET_ECMP_RANDOM_LBf, 4, 405, SOCF_LE | SOCF_GLOBAL },
    { OFFSET_TRUNK_RANDOM_LBf, 4, 401, SOCF_LE | SOCF_GLOBAL },
    { OPRIf, 3, 0, SOCF_LE | SOCF_GLOBAL },
    { OUTER_TPID_VERIFYf, 1, 112, 0 | SOCF_GLOBAL },
    { OVIDf, 12, 3, SOCF_LE | SOCF_GLOBAL },
    { PARITY_0f, 1, 423, 0 | SOCF_GLOBAL },
    { PARITY_1f, 1, 431, 0 | SOCF_GLOBAL },
    { PARITY_2f, 1, 439, 0 | SOCF_GLOBAL },
    { PARITY_3f, 1, 447, 0 | SOCF_GLOBAL },
    { PASS_CONTROL_FRAMESf, 1, 238, 0 | SOCF_GLOBAL },
    { PHB_FROM_ETAGf, 1, 363, 0 | SOCF_GLOBAL },
    { PORT_BRIDGEf, 1, 364, 0 | SOCF_GLOBAL },
    { PORT_CLASS_IDf, 8, 319, SOCF_LE | SOCF_GLOBAL },
    { PORT_DIS_TAGf, 1, 236, 0 | SOCF_GLOBAL },
    { PORT_DIS_UNTAGf, 1, 237, 0 | SOCF_GLOBAL },
    { PORT_OPERATIONf, 3, 61, SOCF_LE | SOCF_GLOBAL },
    { PORT_PRIf, 3, 0, SOCF_LE | SOCF_GLOBAL },
    { PORT_VIDf, 12, 3, SOCF_LE | SOCF_GLOBAL },
    { PRI_MAPPINGf, 24, 202, SOCF_LE | SOCF_GLOBAL },
    { PROHIBITED_DOT1Pf, 8, 311, SOCF_LE | SOCF_GLOBAL },
    { PROTOCOL_PKT_INDEXf, 6, 267, SOCF_LE | SOCF_GLOBAL },
    { PVLAN_ENABLEf, 1, 241, 0 | SOCF_GLOBAL },
    { REMOVE_HG_HDR_SRC_PORTf, 1, 335, 0 | SOCF_GLOBAL },
    { RESERVED_111_110f, 2, 110, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_161_160f, 2, 160, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_192f, 1, 192, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_24_24f, 1, 24, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_250f, 1, 250, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_327_330f, 4, 327, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_373f, 1, 373, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_392_392f, 1, 392, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_413_415f, 3, 413, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_60f, 1, 60, SOCF_RES | SOCF_GLOBAL },
    { RSVD_FLEX_CTR_BASE_COUNTER_IDXf, 3, 87, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RSVD_FLEX_CTR_POOL_NUMBERf, 2, 73, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RTAG7_PORT_LBNf, 4, 243, SOCF_LE | SOCF_GLOBAL },
    { RTAG7_PORT_PROFILE_INDEXf, 10, 344, SOCF_LE | SOCF_GLOBAL },
    { SUBNET_BASED_VID_ENABLEf, 1, 239, 0 | SOCF_GLOBAL },
    { TAG_ACTION_PROFILE_PTRf, 7, 113, SOCF_LE | SOCF_GLOBAL },
    { TRUST_DOT1P_PTRf, 6, 273, SOCF_LE | SOCF_GLOBAL },
    { TRUST_DSCP_PTRf, 7, 279, SOCF_LE | SOCF_GLOBAL },
    { TRUST_DSCP_V4f, 1, 371, 0 | SOCF_GLOBAL },
    { TRUST_DSCP_V6f, 1, 372, 0 | SOCF_GLOBAL },
    { TX_DEST_PORT_ENABLEf, 1, 105, 0 | SOCF_GLOBAL },
    { URPF_DEFAULTROUTECHECKf, 1, 249, 0 | SOCF_GLOBAL },
    { URPF_MODEf, 2, 247, SOCF_LE | SOCF_GLOBAL },
    { USE_INNER_PRIf, 1, 333, 0 | SOCF_GLOBAL },
    { USE_PORT_TABLE_GROUP_IDf, 1, 68, 0 | SOCF_GLOBAL },
    { V4IPMC_ENABLEf, 1, 395, 0 | SOCF_GLOBAL },
    { V4L3_ENABLEf, 1, 397, 0 | SOCF_GLOBAL },
    { V6IPMC_ENABLEf, 1, 394, 0 | SOCF_GLOBAL },
    { V6L3_ENABLEf, 1, 396, 0 | SOCF_GLOBAL },
    { VFP_ENABLEf, 1, 25, 0 | SOCF_GLOBAL },
    { VFP_PORT_GROUP_IDf, 8, 26, SOCF_LE | SOCF_GLOBAL },
    { VLAN_PRECEDENCEf, 1, 242, 0 | SOCF_GLOBAL },
    { VLAN_PROTOCOL_DATA_INDEXf, 7, 292, SOCF_LE | SOCF_GLOBAL },
    { VNTAG_ACTIONS_IF_NOT_PRESENTf, 2, 106, SOCF_LE | SOCF_GLOBAL },
    { VNTAG_ACTIONS_IF_PRESENTf, 2, 108, SOCF_LE | SOCF_GLOBAL },
    { VT_2_ENABLEf, 1, 19, 0 | SOCF_GLOBAL },
    { VT_2_MISS_DROPf, 1, 15, 0 | SOCF_GLOBAL },
    { VT_2_MISS_DROP_KEY_1f, 1, 18, 0 | SOCF_GLOBAL },
    { VT_2_MISS_DROP_KEY_1_AND_KEY_2f, 1, 16, 0 | SOCF_GLOBAL },
    { VT_2_MISS_DROP_KEY_2f, 1, 17, 0 | SOCF_GLOBAL },
    { VT_ENABLEf, 1, 51, 0 | SOCF_GLOBAL },
    { VT_KEY_TYPEf, 4, 46, SOCF_LE | SOCF_GLOBAL },
    { VT_KEY_TYPE_2f, 4, 54, SOCF_LE | SOCF_GLOBAL },
    { VT_MISS_DROPf, 1, 50, 0 | SOCF_GLOBAL },
    { VT_MISS_DROP_KEY_1f, 1, 153, 0 | SOCF_GLOBAL },
    { VT_MISS_DROP_KEY_1_AND_KEY_2f, 1, 151, 0 | SOCF_GLOBAL },
    { VT_MISS_DROP_KEY_2f, 1, 152, 0 | SOCF_GLOBAL },
    { VT_PORT_TYPE_SELECT_1f, 2, 52, SOCF_LE | SOCF_GLOBAL },
    { VT_PORT_TYPE_SELECT_2f, 2, 58, SOCF_LE | SOCF_GLOBAL },
    { VXLAN_SVP_ASSIGNMENT_KEY_TYPEf, 1, 146, 0 | SOCF_GLOBAL },
    { VXLAN_TERMINATION_ALLOWEDf, 1, 399, 0 | SOCF_GLOBAL },
    { VXLAN_TERMINATION_LOOKUP_TYPEf, 1, 147, 0 | SOCF_GLOBAL },
    { VXLAN_VN_ID_LOOKUP_KEY_TYPEf, 2, 158, SOCF_LE | SOCF_GLOBAL },
    { WIRED_WIRELESSf, 1, 23, 0 | SOCF_GLOBAL }
*/        

typedef struct {
    uint32_t entry_data[14];
}lport_tab_entry_t;

//Memory: LPORT_TAB.ipipe0 aka LPORT alias LPORT0 address 0x501c0000
//Flags: valid cachable(on)
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 1024 with indices 0-1023 (0x0-0x3ff), each 56 bytes 14 words

#define LPORT_TABm                 0x501c0000
#define LPORT_TABm_BYTES           56

/*
soc_field_info_t soc_EGR_PORT_BCM56870_A0m_fields[] = {
    { EDIT_CTRL_IDf, 4, 33, SOCF_LE | SOCF_GLOBAL },
    { EGR_LPORT_PROFILE_IDXf, 10, 0, SOCF_LE | SOCF_GLOBAL },
    { EGR_PORT_CTRL_IDf, 8, 10, SOCF_LE | SOCF_GLOBAL },
    { MY_MODIDf, 8, 18, SOCF_LE | SOCF_GLOBAL },
    { PORT_TYPEf, 3, 26, SOCF_LE | SOCF_GLOBAL },
    { QOS_CTRL_IDf, 4, 29, SOCF_LE | SOCF_GLOBAL }
}; */
//MEM_ENTRY(egr_port_entry_t, 5); 
typedef struct egr_port_entry_s {
#if defined(LE_HOST)
    uint64 r0:27,              /* Reserved               */
           edit_ctrl_id:4,     /* EDIT_CTRL_IDf          */
           qos_ctrl_id:4,      /* QOS_CTRL_IDf           */
           port_type:3,        /* PORT_TYPEf             */
           my_modid:8,         /* MY_MODIDf              */
           egr_port_ctrl_id:8, /* EGR_PORT_CTRL_IDf      */
           profile_idx:10;     /* EGR_LPORT_PROFILE_IDXf */
#else
    uint64 profile_idx:10,     /* EGR_LPORT_PROFILE_IDXf */
           egr_port_ctrl_id:8, /* EGR_PORT_CTRL_IDf      */
           my_modid:8,         /* MY_MODIDf              */
           port_type:3,        /* PORT_TYPEf             */
           qos_ctrl_id:4,      /* QOS_CTRL_IDf           */
           edit_ctrl_id:4,     /* EDIT_CTRL_IDf          */
           r0:27;              /* Reserved               */
#endif
}egr_port_entry_t;

//Memory: EGR_PORT.epipe0 address 0x06100000
//Flags: valid cachable(off)
//Blocks:  epipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 72 with indices 0-71 (0x0-0x47), each 5 bytes 2 words
//Entry mask: -1 0x0000001f
//Description: Configuration Register for a Port. This is different for each GE, 10GE port. FeatureSpecific-Ethernet.

#define EGR_PORTm              0x06100000
#define EGR_PORTm_BYTES        5

/*
#if defined(BCM_56370_A0)
soc_field_info_t soc_EGR_LPORT_PROFILE_BCM56370_A0m_fields[] = {
    { CLASS_IDf, 12, 125, SOCF_LE | SOCF_GLOBAL },
    { CNTAG_DELETE_PRI_BITMAPf, 8, 45, SOCF_LE | SOCF_GLOBAL },
    { DROPf, 1, 7, 0 | SOCF_GLOBAL },
    { DUAL_MODID_ENABLEf, 1, 34, 0 | SOCF_GLOBAL },
    { ECCf, 8, 146, SOCF_LE | SOCF_GLOBAL },
    { ECCPf, 9, 146, SOCF_LE | SOCF_GLOBAL },
    { EDIT_CTRL_IDf, 4, 19, SOCF_LE | SOCF_GLOBAL },
    { EFP_FILTER_ENABLEf, 1, 35, 0 | SOCF_GLOBAL },
    { EGR_DOT1P_PROFILE_INDEXf, 4, 3, SOCF_LE | SOCF_GLOBAL },
    { EGR_PORT_GROUP_IDf, 8, 11, SOCF_LE | SOCF_GLOBAL },
    { EGR_QOS_PROFILE_INDEXf, 7, 36, SOCF_LE | SOCF_GLOBAL },
    { EH_EXT_HDR_ENABLEf, 1, 19, 0 | SOCF_GLOBAL },
    { EH_EXT_HDR_LEARN_OVERRIDEf, 1, 54, 0 | SOCF_GLOBAL },
    { EM_SRCMOD_CHANGEf, 1, 10, 0 | SOCF_GLOBAL },
    { EN_EFILTERf, 2, 8, SOCF_LE | SOCF_GLOBAL },
    { FCOE_FC_CRC_RECOMPUTEf, 1, 116, 0 | SOCF_GLOBAL },
    { FCOE_VFT_PRI_MAP_PROFILEf, 2, 117, SOCF_LE | SOCF_GLOBAL },
    { FCOE_VT_LOOKUP_1f, 3, 119, SOCF_LE | SOCF_GLOBAL },
    { FCOE_VT_LOOKUP_2f, 3, 122, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_BASE_COUNTER_IDXf, 11, 97, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_OFFSET_MODEf, 2, 114, SOCF_LE | SOCF_GLOBAL },
    { FLEX_CTR_POOL_NUMBERf, 2, 111, SOCF_LE | SOCF_GLOBAL },
    { IFG_PROFILE_INDEXf, 3, 31, SOCF_LE | SOCF_GLOBAL },
    { MIRROR_ENCAP_ENABLEf, 1, 92, 0 | SOCF_GLOBAL },
    { MIRROR_ENCAP_INDEXf, 4, 93, SOCF_LE | SOCF_GLOBAL },
    { NIV_PRUNE_ENABLEf, 1, 57, 0 | SOCF_GLOBAL },
    { NIV_UPLINK_PORTf, 1, 70, 0 | SOCF_GLOBAL },
    { NIV_VIF_IDf, 12, 58, SOCF_LE | SOCF_GLOBAL },
    { PARITYf, 1, 154, 0 | SOCF_GLOBAL },
    { RESERVED_1f, 3, 0, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_43f, 1, 43, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_53_53f, 1, 53, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_91_71f, 21, 71, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_ECOf, 9, 137, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RSVD_FLEX_CTR_BASE_COUNTER_IDXf, 3, 108, SOCF_LE | SOCF_GLOBAL },
    { RSVD_FLEX_CTR_POOL_NUMBERf, 1, 113, 0 | SOCF_GLOBAL },
    { VNTAG_ACTIONS_IF_PRESENTf, 2, 55, SOCF_LE | SOCF_GLOBAL },
    { VT_SYS_PORT_OVERRIDEf, 1, 44, 0 | SOCF_GLOBAL },
    { VXLT_CTRL_IDf, 8, 23, SOCF_LE | SOCF_GLOBAL }
};
#endif
 */

// Memory: EGR_LPORT_PROFILE.epipe0 address 0x0e440000
// Flags: valid cachable(on)
// Blocks:  epipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
// Entries: 1024 with indices 0-1023 (0x0-0x3ff), each 20 bytes 5 words
// Entry mask: 0xfffffff8 0xffdff7ff 0xf000007f -1 0x07fc01ff
// Description: LPORT Profile table for packets destined to subtending ports
#define EGR_LPORT_PROFILEm         0x0e440000
#define EGR_LPORT_PROFILEm_BYTES   20

typedef struct {
    uint32_t entry_data[5];
}egr_lport_entry_t;

/*
  #if defined(BCM_56370_A0)
soc_field_info_t soc_EGR_GPP_ATTRIBUTES_BCM56370_A0m_fields[] = {
    { DOT1P_SRC_REMARK_MODEf, 2, 85, SOCF_LE | SOCF_GLOBAL },
    { DSCP_SRC_REMARK_MODEf, 2, 83, SOCF_LE | SOCF_GLOBAL },
    { ECCf, 8, 121, SOCF_LE | SOCF_GLOBAL },
    { ECCPf, 9, 121, SOCF_LE | SOCF_GLOBAL },
    { EDIT_CTRL_IDf, 4, 103, SOCF_LE | SOCF_GLOBAL },
    { EGR_LPORT_PROFILE_IDXf, 10, 26, SOCF_LE | SOCF_GLOBAL },
    { ETAG_DOT1P_MAPPING_PTRf, 4, 62, SOCF_LE | SOCF_GLOBAL },
    { ETAG_PCP_DE_SOURCEf, 2, 60, SOCF_LE | SOCF_GLOBAL },
    { FCOE_FC_CRC_ACTIONf, 2, 66, SOCF_LE | SOCF_GLOBAL },
    { FCOE_FC_EOF_IGNOREf, 1, 68, 0 | SOCF_GLOBAL },
    { FLOW_SELECT_CTRL_IDf, 8, 95, SOCF_LE | SOCF_GLOBAL },
    { IPMC_SRC_PRUNE_DISABLEf, 1, 25, 0 | SOCF_GLOBAL },
    { ISTRUNKf, 1, 24, 0 | SOCF_GLOBAL },
    { NIV_NAMESPACEf, 12, 48, SOCF_LE | SOCF_GLOBAL },
    { PARITYf, 1, 129, 0 | SOCF_GLOBAL },
    { PKT_ECN_TO_EXP_MAPPING_ENABLEf, 1, 111, 0 | SOCF_GLOBAL },
    { PKT_ECN_TO_EXP_MAPPING_PTRf, 2, 109, SOCF_LE | SOCF_GLOBAL },
    { QOS_CTRL_IDf, 2, 107, SOCF_LE | SOCF_GLOBAL },
    { RESERVED_1f, 9, 112, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { SRC_IS_NIV_UPLINK_PORTf, 1, 12, 0 | SOCF_GLOBAL },
    { SRC_NIV_VIF_IDf, 12, 0, SOCF_LE | SOCF_GLOBAL },
    { SUBPORT_TAGf, 12, 36, SOCF_LE | SOCF_GLOBAL },
    { TGIDf, 11, 13, SOCF_LE | SOCF_GLOBAL },
    { VLAN_MEMBERSHIP_PROFILEf, 6, 69, SOCF_LE | SOCF_GLOBAL },
    { VT_PORT_GROUP_IDf, 8, 75, SOCF_LE | SOCF_GLOBAL },
    { VXLT_CTRL_IDf, 8, 87, SOCF_LE | SOCF_GLOBAL }
};
#endif
 */
//Memory: EGR_GPP_ATTRIBUTES.epipe0 address 0x0e3c0000
//Flags: valid cachable(on)
//Blocks:  epipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 8192 with indices 0-8191 (0x0-0x1fff), each 17 bytes 5 words
//Entry mask: -1 -1 -1 0xfe00ffff 0x00000003
//Description: Per-GPP attributes table, PORT view.
#define EGR_GPP_ATTRIBUTESm          0x0e3c0000
#define EGR_GPP_ATTRIBUTESm_BYTES    17

/*

#if defined(BCM_56370_A0)
soc_field_info_t soc_ING_DEVICE_PORT_BCM56370_A0m_fields[] = {
    { DISABLE_TIMESTAMPINGf, 1, 132, 0 | SOCF_GLOBAL },
    { DUAL_MODID_ENABLEf, 1, 114, 0 | SOCF_GLOBAL },
    { ECCf, 8, 135, SOCF_LE | SOCF_GLOBAL },
    { ECCPf, 9, 135, SOCF_LE | SOCF_GLOBAL },
    { ECC_DATAf, 135, 0, SOCF_LE | SOCF_GLOBAL },
    { FROM_REMOTE_CPU_ENf, 1, 116, 0 | SOCF_GLOBAL },
    { HDR_TYPE_0f, 16, 81, SOCF_LE | SOCF_GLOBAL },
    { HG_LOOKUP_ENABLEf, 1, 37, 0 | SOCF_GLOBAL },
    { HIGIG_TRUNKf, 1, 107, 0 | SOCF_GLOBAL },
    { HIGIG_TRUNK_IDf, 6, 108, SOCF_LE | SOCF_GLOBAL },
    { HYBRID_MODE_ENABLEf, 1, 38, 0 | SOCF_GLOBAL },
    { ING_PACKET_PROCESSING_ENABLE_0f, 1, 39, 0 | SOCF_GLOBAL },
    { ING_PACKET_PROCESSING_ENABLE_1f, 1, 40, 0 | SOCF_GLOBAL },
    { INITIAL_SHIFT_AMOUNT_0f, 6, 75, SOCF_LE | SOCF_GLOBAL },
    { INSERT_RX_TIMESTAMPf, 1, 133, 0 | SOCF_GLOBAL },
    { LPORT_PROFILE_IDXf, 10, 3, SOCF_LE | SOCF_GLOBAL },
    { MY_MODIDf, 8, 99, SOCF_LE | SOCF_GLOBAL },
    { PARITYf, 1, 143, 0 | SOCF_GLOBAL },
    { PARSE_CONTEXT_ID_0f, 16, 59, SOCF_LE | SOCF_GLOBAL },
    { PARSE_CTRL_ID_0f, 8, 51, SOCF_LE | SOCF_GLOBAL },
    { PKT_FLOW_SELECT_CTRL_ID_0f, 12, 37, SOCF_LE | SOCF_GLOBAL },
    { PORT_IPBM_INDEXf, 6, 126, SOCF_LE | SOCF_GLOBAL },
    { PORT_TYPEf, 3, 0, SOCF_LE | SOCF_GLOBAL },
    { PP_PORT_NUMf, 7, 20, SOCF_LE | SOCF_GLOBAL },
    { REMOTE_CPU_ENf, 1, 115, 0 | SOCF_GLOBAL },
    { REMOVE_MH_SRC_PORTf, 1, 125, 0 | SOCF_GLOBAL },
    { RESERVED_124_124f, 1, 124, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_134f, 1, 134, SOCF_RES | SOCF_GLOBAL },
    { RESERVED_36_33f, 4, 33, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { RESERVED_98_97f, 2, 97, SOCF_LE|SOCF_RES | SOCF_GLOBAL },
    { SRC_SYS_PORT_IDf, 7, 117, SOCF_LE | SOCF_GLOBAL },
    { SUBPORT_ID_NAMESPACEf, 6, 27, SOCF_LE | SOCF_GLOBAL },
    { SYS_PORT_IDf, 7, 13, SOCF_LE | SOCF_GLOBAL },
    { USE_MH_PKT_PRIf, 1, 50, 0 | SOCF_GLOBAL },
    { USE_MH_VIDf, 1, 49, 0 | SOCF_GLOBAL }
};
#endif
*/


typedef struct {
    uint32_t entry_data[5];
}ing_device_port_entry_t;

//Memory: ING_DEVICE_PORT.ipipe0 aka PORT alias PORT0 address 0x4c000000
//Flags: valid cachable(on)
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 73 with indices 0-72 (0x0-0x48), each 18 bytes 5 words
//Description: Ingress Physical Port Table.
#define ING_DEVICE_PORTm          0x4c000000
#define ING_DEVICE_PORTm_BYTES    18


/* Add an entry to field-value array for multiple fields write */
#define _mem_set_field_value_array(_fa, _f, _va, _v, _p) \
    do {                \
        _fa[_p] = _f;   \
        _va[_p] = _v;   \
        _p++;           \
        } while (0);    \


//Memory: ING_DEST_PORT_ENABLE.ipipe0 address 0x84500000
//Flags: valid cachable(off)
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 1 with indices 0-0 (0x0-0x0), each 9 bytes 3 words
//Entry mask: -1 -1 0x000000ff
//Description: A last-stage master switch which can block packets from any traffic source including SOBMH. Works for all egress ports (including CPU port)
//  PORT_BITMAP<71:0>
#define ING_DEST_PORT_ENABLEm            0x84500000
#define ING_DEST_PORT_ENABLEm_BYTES      9

//Memory: EPC_LINK_BMAP.ipipe0 address 0x84240000
//Flags: valid cachable(off)
//Blocks:  ipipe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 1 with indices 0-0 (0x0-0x0), each 9 bytes 3 words
//Entry mask: -1 -1 0x000000ff
//Description: Link status register under software control.
//  PORT_BITMAP<71:0>
#define EPC_LINK_BMAPm                  0x84240000
#define EPC_LINK_BMAPm_BYTES            9

//Memory: EGR_ENABLE.epipe0 alias EDB_ENABLE address 0x28200000
//Entries: 79 with indices 0-78 (0x0-0x4e), each 1 bytes 1 words
/*
soc_field_info_t soc_EGR_ENABLE_BCM53400_A0m_fields[] = {
    { PRT_ENABLEf, 1, 0, 0 | SOCF_GLOBAL }
};
*/
#define EGR_ENABLEm                     0x28200000
#define EGR_ENABLEm_BYTES               1


//Memory: EGR_PER_PORT_BUFFER_SFT_RESET.epipe0 alias EDB_PER_PORT_BUFFER_SFT_RESET address 0x28300000
//Entries: 79 with indices 0-78 (0x0-0x4e), each 1 bytes 1 words
#define EGR_PER_PORT_BUFFER_SFT_RESETm  0x28300000

/*****************************************************************************************/
/*                              FLOWDB                                                   */
/*****************************************************************************************/
/* Format of Table for indexed table
 * | START_OF TABLE_CHUNK |
 * | tbl header |
 * | tbl entry records |
 * | END OF Table CHUNK |
 */
typedef struct soc_flow_db_tbl_map_s {
   uint32 tbl_start;
   uint32 block_size; /* block size */
   uint32 crc;  /* block crc */
   uint32 pa;   /* hash parameters */
   uint32 pb;   /* hash parameters */
   uint32 pc;   /* hash parameters */
   uint32 pd;   /* hash parameters */
   uint32 pe;   /* hash parameters */
   uint32 num_entries;
   uint32 hash_tbl_size;
   uint32 tbl_entry;
} soc_flow_db_tbl_map_t;

typedef struct soc_flow_db_view_ffo_tuple_s {
    uint32 view_id;
    uint32 nffos;
    uint32 *ffo_tuple;
} soc_flow_db_view_ffo_tuple_t;

typedef struct soc_flow_db_flow_map_s {
    /* pointer to the flow table chunk */
    soc_flow_db_tbl_map_t *flow_tbl_lyt;
    /* pointer to the flow option table chunk */
    soc_flow_db_tbl_map_t *flow_option_tbl_lyt;
    /* pointer to the ffo tuple to view id map table chunk */
    soc_flow_db_tbl_map_t *ffo_tuple_tbl_lyt;
    /* pointer to the view table chunk */
    soc_flow_db_tbl_map_t *view_tbl_lyt;
    /* pointer to the logical field map table chunk*/
    soc_flow_db_tbl_map_t *lg_field_tbl_lyt;
    /* view to ffo tuple list*/
    soc_flow_db_view_ffo_tuple_t *view_ffo_list;
    /* string table */
    char *str_tbl;
} soc_flow_db_flow_map_t;




/*****************************************************************************************/
/*                              CANCUN                                                   */
/*****************************************************************************************/
#define IP_PARSER0_HME_STAGE_TCAM_ONLY_0m                  0x4cc00000
#define IP_PARSER0_HME_STAGE_TCAM_DATA_ONLY_0m             0x4cb80000
#define IP_PARSER0_HFE_POLICY_TABLE_0m                     0x4cb00000
#define IP_PARSER0_HVE_SCC_PROFILE_0m                      0x4ccc0000
#define IP_PARSER0_HVE_SCC_PROFILE_1m                      0x4cd00000
#define IP_PARSER0_HVE_SCF_PROFILE_0m                      0x4cec0000
#define IP_PARSER0_HVE_RC_PROFILE_0m                       0x4cc40000
#define IP_PARSER0_HVE_RC_PROFILE_1m                       0x4cc80000
#define SPECIAL_LABEL_CONTROLm                             0x50340000
#define IP_PARSER1_HME_STAGE_TCAM_ONLY_0m                  0x50f80000
#define IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_0m             0x50d00000
#define IP_PARSER1_HFE_POLICY_TABLE_0m                     0x50a80000
#define IP_PARSER1_HME_STAGE_TCAM_ONLY_1m                  0x50fc0000
#define IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_1m             0x50d40000
#define IP_PARSER1_HFE_POLICY_TABLE_1m                     0x50ac0000
#define IP_PARSER1_HME_STAGE_TCAM_NARROW_ONLY_2m           0x50ec0000
#define IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_2m             0x50d80000
#define IP_PARSER1_HFE_POLICY_TABLE_2m                     0x50b00000
#define IP_PARSER1_HME_STAGE_TCAM_ONLY_3m                  0x51040000
#define IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_3m             0x50dc0000
#define IP_PARSER1_HFE_POLICY_TABLE_3m                     0x50b40000
#define IP_PARSER1_HME_STAGE_TCAM_ONLY_4m                  0x51080000
#define IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_4m             0x50e00000
#define IP_PARSER1_HFE_POLICY_TABLE_4m                     0x50b80000
#define IP_PARSER1_MICE_TCAM_0m                            0x51400000
#define IP_PARSER1_MICE_TCAM_1m                            0x51440000
#define IP_PARSER1_HVE_SCC_PROFILE_0m                      0x51140000
#define IP_PARSER1_HVE_SCC_PROFILE_1m                      0x51180000
#define IP_PARSER1_HVE_SCC_PROFILE_2m                      0x511c0000
#define IP_PARSER1_HVE_SCC_PROFILE_3m                      0x51200000
#define IP_PARSER1_HVE_SCC_PROFILE_4m                      0x51240000
#define IP_PARSER1_HVE_SCC_PROFILE_5m                      0x51280000
#define IP_PARSER1_HVE_SCF_PROFILE_0m                      0x51340000
#define IP_PARSER1_HVE_RC_PROFILE_0m                       0x510c0000
#define IP_PARSER2_HME_STAGE_TCAM_ONLY_0m                  0x5cf80000
#define IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_0m             0x5cd00000
#define IP_PARSER2_HFE_POLICY_TABLE_0m                     0x5ca80000
#define IP_PARSER2_HME_STAGE_TCAM_ONLY_1m                  0x5cfc0000
#define IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_1m             0x5cd40000
#define IP_PARSER2_HFE_POLICY_TABLE_1m                     0x5cac0000
#define IP_PARSER2_HME_STAGE_TCAM_NARROW_ONLY_2m           0x5cec0000
#define IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_2m             0x5cd80000
#define IP_PARSER2_HFE_POLICY_TABLE_2m                     0x5cb00000
#define IP_PARSER2_HME_STAGE_TCAM_NARROW_ONLY_3m           0x5cf00000
#define IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_3m             0x5cdc0000
#define IP_PARSER2_HFE_POLICY_TABLE_3m                     0x5cb40000
#define IP_PARSER2_HME_STAGE_TCAM_ONLY_4m                  0x5d080000
#define IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_4m             0x5ce00000
#define IP_PARSER2_HFE_POLICY_TABLE_4m                     0x5cb80000
#define IP_PARSER2_MICE_TCAM_0m                            0x5d400000
#define IP_PARSER2_MICE_TCAM_1m                            0x5d440000
#define IP_PARSER2_HVE_SCC_PROFILE_0m                      0x5d140000
#define IP_PARSER2_HVE_SCC_PROFILE_1m                      0x5d180000
#define IP_PARSER2_HVE_SCF_PROFILE_0m                      0x5d340000
#define IP_PARSER2_HVE_RC_PROFILE_0m                       0x5d0c0000
#define IP_PARSER2_HVE_RC_PROFILE_1m                       0x5d100000
#define IP_PARSER0_HFE_CONT_PROFILE_TABLE_7m               0x4c9c0000
#define IP_PARSER0_HFE_CONT_PROFILE_TABLE_5m               0x4c940000
#define IP_PARSER0_HFE_CONT_PROFILE_TABLE_4m               0x4c900000
#define IP_PARSER1_HFE_CONT_PROFILE_TABLE_0m               0x50800000
#define IP_PARSER1_HFE_CONT_PROFILE_TABLE_5m               0x50940000
#define IP_PARSER1_HFE_CONT_PROFILE_TABLE_7m               0x509c0000
#define IP_PARSER1_HFE_CONT_PROFILE_TABLE_9m               0x50a40000
#define IP_PARSER2_HFE_CONT_PROFILE_TABLE_5m               0x5c940000
#define IP_PARSER2_HFE_CONT_PROFILE_TABLE_7m               0x5c9c0000
#define IP_PARSER2_HFE_CONT_PROFILE_TABLE_9m               0x5ca40000
#define IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_0m             0x50bc0000
#define IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_1m             0x50c00000
#define IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_2m             0x50c40000
#define IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_3m             0x50c80000
#define IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_4m             0x50cc0000
#define IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_0m             0x5cbc0000
#define IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_1m             0x5cc00000
#define IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_2m             0x5cc40000
#define IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_3m             0x5cc80000
#define IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_4m             0x5ccc0000
#define PKT_FLOW_SELECT_HVE_CONTROL_0m                     0x4c400000
#define PKT_FLOW_SELECT_TCAM_0m                            0x4c100000
#define PKT_FLOW_CTRL_PROFILE_0m                           0x4c200000
#define SGPP_CTRL_PROFILE_0m                               0x4c0c0000
#define SVP_CTRL_PROFILE_0m                                0x4c2c0000
#define PKT_FLOW_CTRL_STRENGTH_PROFILE_0m                  0x4c1c0000
#define PKT_FLOW_SELECT_HVE_CONTROL_1m                     0x50300000
#define PKT_FLOW_SELECT_TCAM_1m                            0x500c0000
#define KEYGEN_HDR_VALID_CHECK_PROFILE_1m                  0x50180000
#define FIELD_BUS_MERGE_PROFILE_0m                         0x61280000
#define TUNNEL_ADAPT_1_LOGICAL_TBL_SEL_TCAMm               0x55b80000
#define TUNNEL_ADAPT_1_LOGICAL_TBL_SEL_SRAMm               0x55bc0000
#define TUNNEL_ADAPT_2_LOGICAL_TBL_SEL_TCAMm               0x55f00000
#define TUNNEL_ADAPT_2_LOGICAL_TBL_SEL_SRAMm               0x55f40000
#define TUNNEL_ADAPT_3_LOGICAL_TBL_SEL_TCAMm               0x55d00000
#define TUNNEL_ADAPT_3_LOGICAL_TBL_SEL_SRAMm               0x55d40000
#define TUNNEL_ADAPT_4_LOGICAL_TBL_SEL_TCAMm               0x61540000
#define TUNNEL_ADAPT_4_LOGICAL_TBL_SEL_SRAMm               0x61580000
#define TUNNEL_ADAPT_1_KEY_GEN_1_MUX_CTRLm                 0x55c00000
#define TUNNEL_ADAPT_1_KEY_GEN_1_MASKm                     0x55c40000
#define TUNNEL_ADAPT_1_KEY_GEN_2_MUX_CTRLm                 0x55c80000
#define TUNNEL_ADAPT_1_KEY_GEN_2_MASKm                     0x55cc0000
#define TUNNEL_ADAPT_2_KEY_GEN_1_MUX_CTRLm                 0x55f80000
#define TUNNEL_ADAPT_2_KEY_GEN_1_MASKm                     0x55fc0000
#define TUNNEL_ADAPT_2_KEY_GEN_2_MUX_CTRLm                 0x56000000
#define TUNNEL_ADAPT_2_KEY_GEN_2_MASKm                     0x56040000
#define TUNNEL_ADAPT_3_KEY_GEN_1_MUX_CTRLm                 0x55d80000
#define TUNNEL_ADAPT_3_KEY_GEN_1_MASKm                     0x55dc0000
#define TUNNEL_ADAPT_4_KEY_GEN_1_MUX_CTRLm                 0x615c0000
#define TUNNEL_ADAPT_4_KEY_GEN_1_MASKm                     0x61600000
#define PKT_FLOW_EXTRACTION_CTRL_ID_STRENGTH_PROFILEm      0x55240000
#define ING_MPLS_INHERIT_TABLEm                            0x70940000
#define SGPP_CTRL_PROFILE_1m                               0x58140000
#define L3_IIF_CTRL_PROFILE_1m                             0x583c0000
#define SVP_CTRL_PROFILE_1m                                0x58340000
#define MPLS_PARSER2_CONTEXT_PROFILEm                      0x58200000
#define PKT_FLOW_CTRL_STRENGTH_PROFILE_1m                  0x58500000
#define SVP_PROFILEm                                       0x64500000
#define PKT_FLOW_SELECT_HVE_CONTROL_2m                     0x64700000
#define PKT_FLOW_SELECT_TCAM_2m                            0x641c0000
#define FIELD_BUS_MERGE_PROFILEm                           0x68680000
#define PKT_FLOW_VSAN_STRENGTH_PROFILE_1m                  0x58000000
#define L2_DESTINATION_STRENGTH_PROFILEm                   0x70180000
#define PKT_FLOW_SELECT_MY_STATION_PROFILE_1m              0x581c0000
#define PKT_FLOW_SELECT_MY_STATION_PROFILE_2m              0x70200000
#define FORWARDING_1_LOGICAL_TBL_SEL_TCAMm                 0x69800000
#define FORWARDING_1_LOGICAL_TBL_SEL_SRAMm                 0x69840000
#define FORWARDING_1_KEY_GEN_1_MUX_CTRLm                   0x69880000
#define FORWARDING_1_KEY_GEN_1_MASKm                       0x698c0000
#define FORWARDING_1_ACTION_PROFILEm                       0x69d00000
#define L2_ENTRY_KEY_ATTRIBUTESm                           0x693c0000
#define FORWARDING_2_LOGICAL_TBL_SEL_TCAMm                 0x69480000
#define FORWARDING_2_LOGICAL_TBL_SEL_SRAMm                 0x694c0000
#define FORWARDING_2_KEY_GEN_1_MUX_CTRLm                   0x69500000
#define FORWARDING_2_KEY_GEN_1_MASKm                       0x69540000
#define FORWARDING_2_KEY_GEN_2_MUX_CTRLm                   0x69580000
#define FORWARDING_2_KEY_GEN_2_MASKm                       0x695c0000
#define FORWARDING_2_KEY_GEN_3_MUX_CTRLm                   0x69600000
#define FORWARDING_2_KEY_GEN_3_MASKm                       0x69640000
#define FORWARDING_2_KEY_GEN_4_MUX_CTRLm                   0x69680000
#define FORWARDING_2_KEY_GEN_4_MASKm                       0x696c0000
#define FORWARDING_2_KEY_GEN_5_MUX_CTRLm                   0x69700000
#define FORWARDING_2_KEY_GEN_5_MASKm                       0x69740000
#define FORWARDING_2_ACTION_PROFILEm                       0x69d40000
#define L3_ENTRY_KEY_ATTRIBUTESm                           0x691c0000
#define FORWARDING_3_LOGICAL_TBL_SEL_TCAMm                 0x69980000
#define FORWARDING_3_LOGICAL_TBL_SEL_SRAMm                 0x699c0000
#define FORWARDING_3_KEY_GEN_1_MUX_CTRLm                   0x69a00000
#define FORWARDING_3_KEY_GEN_1_MASKm                       0x69a40000
#define FORWARDING_3_KEY_GEN_2_MUX_CTRLm                   0x69a80000
#define FORWARDING_3_KEY_GEN_2_MASKm                       0x69ac0000
#define FORWARDING_3_KEY_GEN_3_MUX_CTRLm                   0x69b00000
#define FORWARDING_3_KEY_GEN_3_MASKm                       0x69b40000
#define FORWARDING_3_KEY_GEN_4_MUX_CTRLm                   0x69b80000
#define FORWARDING_3_KEY_GEN_4_MASKm                       0x69bc0000
#define FORWARDING_3_KEY_GEN_5_MUX_CTRLm                   0x69c00000
#define FORWARDING_3_KEY_GEN_5_MASKm                       0x69c40000
#define FORWARDING_3_KEY_GEN_6_MUX_CTRLm                   0x69c80000
#define FORWARDING_3_KEY_GEN_6_MASKm                       0x69cc0000
#define FORWARDING_3_ACTION_PROFILEm                       0x69d80000
#define ING_VP_VLAN_MEMBERSHIP_KEY_ATTRIBUTESm             0x64ac0000
#define PKT_FLOW_VLAN_STRENGTH_PROFILE_1_Am                0x55000000
#define PKT_FLOW_VLAN_STRENGTH_PROFILE_1_Bm                0x61100000
#define PKT_FLOW_VRF_STRENGTH_PROFILE_1_Am                 0x55100000
#define PKT_FLOW_VRF_STRENGTH_PROFILE_1_Bm                 0x64180000
#define PKT_FLOW_L3_IIF_STRENGTH_PROFILE_1_Am              0x550c0000
#define PKT_FLOW_L3_IIF_STRENGTH_PROFILE_1_Bm              0x64140000
#define PKT_FLOW_VFI_STRENGTH_PROFILE_1_Am                 0x55080000
#define PKT_FLOW_VFI_STRENGTH_PROFILE_1_Bm                 0x640c0000
#define PKT_FLOW_SVP_STRENGTH_PROFILE_1_Am                 0x55040000
#define PKT_FLOW_OPAQUE_1_STRENGTH_PROFILE_1_Am            0x55140000
#define PKT_FLOW_OPAQUE_1_STRENGTH_PROFILE_1_Bm            0x70980000
#define PKT_FLOW_OPAQUE_2_STRENGTH_PROFILE_1_Bm            0x709c0000
#define PKT_FLOW_OPAQUE_3_STRENGTH_PROFILE_1_Bm            0x70a00000
#define PKT_FLOW_OPAQUE_3_STRENGTH_PROFILE_1_Am            0x551c0000
#define PKT_FLOW_OPAQUE_4_STRENGTH_PROFILE_1_Am            0x55200000
#define FORWARDING_1_FIXED_KEY_TABLE_ATTRSm                0x682c0000
#define FORWARDING_1_POLICY_STRENGTH_PROFILEm              0x68300000
#define FORWARDING_1_MISS_POLICYm                          0x68340000
#define FORWARDING_2_FIXED_KEY_TABLE_ATTRSm                0x68200000
#define FORWARDING_2_POLICY_STRENGTH_PROFILEm              0x68240000
#define FORWARDING_3_FIXED_KEY_TABLE_ATTRSm                0x68140000
#define FORWARDING_3_POLICY_STRENGTH_PROFILEm              0x68180000
#define VLAN_XLATE_1_KEY_ATTRIBUTESm                       0x558c0000
#define TUNNEL_ADAPT_1_FIXED_KEY_TABLE_ATTRSm              0x55280000
#define TUNNEL_ADAPT_1_POLICY_STRENGTH_PROFILEm            0x55580000
#define TUNNEL_ADAPT_1_ACTION_PROFILEm                     0x56100000
#define TUNNEL_ADAPT_1_MISS_POLICYm                        0x552c0000
#define VLAN_XLATE_2_KEY_ATTRIBUTESm                       0x612c0000
#define MPLS_ENTRY_KEY_ATTRIBUTESm                         0x55b00000
#define TUNNEL_ADAPT_2_FIXED_KEY_TABLE_ATTRSm              0x55300000
#define TUNNEL_ADAPT_2_POLICY_STRENGTH_PROFILEm            0x555c0000
#define TUNNEL_ADAPT_2_MISS_POLICYm                        0x55340000
#define TUNNEL_ADAPT_2_ACTION_PROFILEm                     0x56080000
#define TUNNEL_ADAPT_3_FIXED_KEY_TABLE_ATTRSm              0x55380000
#define TUNNEL_ADAPT_3_POLICY_STRENGTH_PROFILEm            0x55600000
#define TUNNEL_ADAPT_3_ACTION_PROFILEm                     0x560c0000
#define TUNNEL_ADAPT_4_FIXED_KEY_TABLE_ATTRSm              0x61040000
#define TUNNEL_ADAPT_4_POLICY_STRENGTH_PROFILEm            0x61080000
#define TUNNEL_ADAPT_4_MISS_POLICYm                        0x610c0000
#define TUNNEL_ADAPT_4_ACTION_PROFILEm                     0x616c0000
#define PKT_PROC_HDR_VALID_CHECK_PROFILE_1_VALUEm          0x58040000
#define PKT_PROC_HDR_VALID_CHECK_PROFILE_1_MASKm           0x58080000
#define PKT_PROC_HDR_VALID_CHECK_PROFILE_2m                0x70100000
#define MPLS_EDIT_CTRL_ID_MAPm                             0x70000000
#define FLEX_RTAG7_HASH_TCAMm                              0x684c0000
#define FLEX_BIN_HASH_PROFILE_0m                           0x68580000
#define FLEX_BIN_HASH_PROFILE_1m                           0x685c0000
#define FLEX_BIN_HASH_PROFILE_2m                           0x68600000
#define FLEX_BIN_HASH_PROFILE_3m                           0x68640000
#define ING_LOOPBACK_DROP_VECTOR_MASKm                     0x80a80000
#define EGR_ZONE_0_MATCH_ID_ATTRIBUTES_TABLEm              0x06380000
#define EP_PARSER0_HFE_POLICY_TABLE_0m                     0x04300000
#define EP_PARSER0_HFE_CONT_PROFILE_TABLE_7m               0x041c0000
#define EP_PARSER0_HFE_CONT_PROFILE_TABLE_5m               0x04140000
#define EP_PARSER0_HFE_CONT_PROFILE_TABLE_4m               0x04100000
#define EP_PARSER0_HFE_CONT_PROFILE_TABLE_2m               0x04080000
#define EGR_ZONE_0_VAR_LEN_HDR1_PROFILEm                   0x063c0000
#define EGR_ZONE_0_VAR_LEN_HDR2_PROFILEm                   0x06400000
#define EGR_ZONE_1_MATCH_ID_ATTRIBUTES_TABLEm              0x06640000
#define EP_PARSER1_HFE_POLICY_TABLE_0m                     0x04b00000
#define EGR_FIELD_EXTRACTION_PROFILE_1_TCAMm               0x06740000
#define EGR_ZONE_1_VAR_LEN_HDR1_PROFILEm                   0x06440000
#define EGR_ZONE_1_VAR_LEN_HDR2_PROFILEm                   0x06480000
#define EGR_ZONE_2_MATCH_ID_ATTRIBUTES_TABLEm              0x06680000
#define EP_PARSER1_HFE_POLICY_TABLE_1m                     0x04b40000
#define EGR_ZONE_2_VAR_LEN_HDR1_PROFILEm                   0x064c0000
#define EGR_ZONE_2_VAR_LEN_HDR2_PROFILEm                   0x06500000
#define EGR_FIELD_EXTRACTION_PROFILE_2_TCAMm               0x06800000
#define EP_PARSER1_HFE_CONT_PROFILE_TABLE_0m               0x04800000
#define EP_PARSER1_HFE_CONT_PROFILE_TABLE_3m               0x048c0000
#define EP_PARSER1_HFE_CONT_PROFILE_TABLE_7m               0x049c0000
#define EP_PARSER1_HFE_CONT_PROFILE_TABLE_11m              0x04ac0000
#define EGR_ZONE_3_MATCH_ID_ATTRIBUTES_TABLEm              0x066c0000
#define EP_PARSER1_HFE_POLICY_TABLE_2m                     0x04b80000
#define EGR_ZONE_3_VAR_LEN_HDR1_PROFILEm                   0x06540000
#define EGR_ZONE_3_VAR_LEN_HDR2_PROFILEm                   0x06580000
#define EGR_ZONE_4_MATCH_ID_ATTRIBUTES_TABLEm              0x06700000
#define EP_PARSER1_HFE_POLICY_TABLE_3m                     0x04bc0000
#define EGR_ZONE_4_VAR_LEN_HDR1_PROFILEm                   0x065c0000
#define EGR_ZONE_4_VAR_LEN_HDR2_PROFILEm                   0x06600000
#define EGR_FIELD_EXTRACTION_PROFILE_CONTROLm              0x068c0000
#define EGR_PKT_FLOW_SELECT_TCAMm                          0x0e780000
#define EGR_L2_TAG_INHERIT_ACTION_MASK_PROFILEm            0x09e40000
#define EGR_L2_TAG_INHERIT_PROFILEm                        0x09e80000
#define EGR_PKT_FLOW_ZONE_SELECT_PROFILEm                  0x0e840000
#define EGR_ADAPT_1_LOGICAL_TBL_SEL_TCAMm                  0x0a600000
#define EGR_ADAPT_1_LOGICAL_TBL_SEL_SRAMm                  0x0a640000
#define EGR_ADAPT_1_KEY_GEN_1_MUX_CTRLm                    0x0a680000
#define EGR_ADAPT_1_KEY_GEN_1_MASKm                        0x0a6c0000
#define EGR_ADAPT_1_KEY_GEN_2_MUX_CTRLm                    0x0a700000
#define EGR_ADAPT_1_KEY_GEN_2_MASKm                        0x0a740000
#define EGR_ADAPT_2_LOGICAL_TBL_SEL_TCAMm                  0x0a780000
#define EGR_ADAPT_2_LOGICAL_TBL_SEL_SRAMm                  0x0a7c0000
#define EGR_ADAPT_2_KEY_GEN_1_MUX_CTRLm                    0x0a800000
#define EGR_ADAPT_2_KEY_GEN_1_MASKm                        0x0a840000
#define EGR_ADAPT_2_KEY_GEN_2_MUX_CTRLm                    0x0a880000
#define EGR_ADAPT_2_KEY_GEN_2_MASKm                        0x0a8c0000
#define EGR_L3_NEXT_HOP_EARLY_ACTION_PROFILEm              0x069c0000
#define EGR_L3_NEXT_HOP_ACTION_PROFILEm                    0x0ed00000
#define EGR_L3_INTF_EARLY_ACTION_PROFILEm                  0x06900000
#define EGR_L3_INTF_ACTION_PROFILEm                        0x0ee00000
#define EGR_DVP_ATTRIBUTE_EARLY_ACTION_PROFILEm            0x06940000
#define EGR_DVP_ATTRIBUTE_ACTION_PROFILEm                  0x0ed80000
#define EGR_VC_AND_SWAP_EARLY_ACTION_PROFILEm              0x06980000
#define EGR_VC_AND_SWAP_ACTION_PROFILEm                    0x0ed40000
#define EGR_IP_TUNNEL_ACTION_PROFILEm                      0x0edc0000
#define EGR_LABEL_PRECEDENCE_PROFILE_TABLEm                0x0ebc0000
#define EGR_SPECIAL_PKT_HANDLING_PROFILEm                  0x1a140000
#define EGR_STRENGTH_PROFILEm                              0x0e9c0000
#define EGR_VP_VLAN_MEMBERSHIP_KEY_ATTRIBUTESm             0x0a440000
#define EGR_VLAN_XLATE_1_KEY_ATTRIBUTESm                   0x0a180000
#define EGR_VLAN_XLATE_2_KEY_ATTRIBUTESm                   0x0a4c0000
#define EGR_ADAPT_1_FIXED_KEY_TABLE_ATTRSm                 0x09740000
#define EGR_ADAPT_1_POLICY_STRENGTH_PROFILEm               0x09840000
#define EGR_ADAPT_1_MISS_POLICYm                           0x097c0000
#define EGR_ADAPT_1_ACTION_PROFILEm                        0x0a940000
#define EGR_ADAPT_2_FIXED_KEY_TABLE_ATTRSm                 0x09780000
#define EGR_ADAPT_2_POLICY_STRENGTH_PROFILEm               0x09880000
#define EGR_ADAPT_2_MISS_POLICYm                           0x09800000
#define EGR_ADAPT_2_ACTION_PROFILEm                        0x0a900000
#define EGR_ZONE_0_EDITOR_CONTROL_TCAMm                    0x098c0000
#define EGR_ZONE_1_EDITOR_CONTROL_TCAMm                    0x09980000
#define EGR_ZONE_2_EDITOR_CONTROL_TCAMm                    0x09b00000
#define EGR_ZONE_3_EDITOR_CONTROL_TCAMm                    0x09bc0000
#define EGR_ZONE_4_EDITOR_CONTROL_TCAMm                    0x09c40000
#define EGR_ZONE_1_L2_TAG_CONTROL_PROFILE_DELm             0x09ac0000
#define EGR_ZONE_1_L2_TAG_CONTROL_PROFILE_INSm             0x09a40000
#define EGR_ZONE_3_L2_TAG_CONTROL_PROFILE_DELm             0x09d80000
#define EGR_ZONE_3_L2_TAG_CONTROL_PROFILE_INSm             0x09d00000
#define EGR_DELETE_CONTROLm                                0x09e00000
#define FLEX_EDITOR_FIXED_HDR_L2_0_NEXT_PROTO_TABLEm             0x18b80000
#define FLEX_EDITOR_FIXED_HDR_L2_1_NEXT_PROTO_TABLEm             0x18c00000
#define FLEX_EDITOR_FIXED_HDR_L3_0_NEXT_PROTO_TABLEm             0x18c80000
#define FLEX_EDITOR_FIXED_HDR_TUNNEL_0_GRE_NEXT_PROTO_TABLEm     0x18d40000
#define FLEX_EDITOR_FLEX_HDR_0_NEXT_PROTO_TABLEm           0x18f40000
#define FLEX_EDITOR_FLEX_HDR_1_NEXT_PROTO_TABLEm           0x19100000
#define EGR_L3_NEXT_HOP_ACTION_MASK_PROFILEm               0x0e880000
#define EGR_L3_INTF_ACTION_MASK_PROFILEm                   0x0e8c0000
#define EGR_IP_TUNNEL_ACTION_MASK_PROFILEm                 0x0e980000
#define EGR_DVP_ATTRIBUTE_ACTION_MASK_PROFILEm             0x0e900000
#define EGR_VC_AND_SWAP_ACTION_MASK_PROFILEm               0x0e940000
#define FLEX_EDITOR_FIXED_HDR_L2_0_PROFILE_TABLEm          0x18b40000
#define FLEX_EDITOR_FIXED_HDR_L2_1_PROFILE_TABLEm          0x18bc0000
#define FLEX_EDITOR_FIXED_HDR_L3_0_PROFILE_TABLEm          0x18c40000
#define FLEX_EDITOR_FIXED_HDR_TUNNEL_0_PROFILE_TABLEm      0x18cc0000
#define FLEX_EDITOR_FLEX_HDR_0_PROFILE_TABLEm              0x18dc0000
#define FLEX_EDITOR_FLEX_HDR_0_16BIT_FS_PROFILE_TABLEm     0x18e00000
#define FLEX_EDITOR_FLEX_HDR_0_32BIT_FS_PROFILE_TABLEm     0x18e40000
#define FLEX_EDITOR_FLEX_HDR_0_HC_PROFILE_TABLEm           0x18ec0000
#define FLEX_EDITOR_FLEX_HDR_0_ALU_PROFILE_TABLEm          0x18e80000
#define FLEX_EDITOR_FLEX_HDR_0_OW_PROFILE_TABLEm           0x18f00000
#define FLEX_EDITOR_FLEX_HDR_1_PROFILE_TABLEm              0x18f80000
#define FLEX_EDITOR_FLEX_HDR_1_16BIT_FS_PROFILE_TABLEm     0x18fc0000
#define FLEX_EDITOR_FLEX_HDR_1_32BIT_FS_PROFILE_TABLEm     0x19000000
#define FLEX_EDITOR_FLEX_HDR_1_HC_PROFILE_TABLEm           0x19080000
#define FLEX_EDITOR_FLEX_HDR_1_ALU_PROFILE_TABLEm          0x19040000
#define FLEX_EDITOR_FLEX_HDR_2_PROFILE_TABLEm              0x19140000
#define FLEX_EDITOR_FLEX_HDR_2_16BIT_FS_PROFILE_TABLEm     0x19180000
#define FLEX_EDITOR_FLEX_HDR_2_32BIT_FS_PROFILE_TABLEm     0x191c0000
#define FLEX_EDITOR_FLEX_HDR_2_OW_PROFILE_TABLEm           0x19280000
#define FLEX_EDITOR_FLEX_HDR_2_HC_PROFILE_TABLEm           0x19240000
#define FLEX_EDITOR_RW_0_PROFILE_TABLEm                    0x18780000
#define FLEX_EDITOR_RW_0_FS_PROFILE_TABLEm                 0x187c0000
#define FLEX_EDITOR_RW_0_HC_PROFILE_TABLEm                 0x18840000
#define FLEX_EDITOR_RW_0_ALU_PROFILE_TABLEm                0x18800000
#define FLEX_EDITOR_MHC_CRC_0_PROFILE_TABLEm               0x193c0000
#define FLEX_EDITOR_RW_1_PROFILE_TABLEm                    0x188c0000
#define FLEX_EDITOR_RW_1_FS_PROFILE_TABLEm                 0x18900000
#define FLEX_EDITOR_RW_1_HC_PROFILE_TABLEm                 0x18980000
#define FLEX_EDITOR_RW_1_ALU_PROFILE_TABLEm                0x18940000
#define FLEX_EDITOR_MHC_CHECKSUM_0_PROFILE_TABLEm          0x19300000
#define FLEX_EDITOR_MHC_CHECKSUM_1_PROFILE_TABLEm          0x19340000
#define FLEX_EDITOR_MHC_CHECKSUM_2_PROFILE_TABLEm          0x19380000
#define FLEX_EDITOR_FIXED_HDR_SYS_0_PROFILE_TABLEm         0x18a00000
#define FLEX_EDITOR_ZONE_0_MATCH_ID_TABLEm                 0x18000000
#define FLEX_EDITOR_ZONE_0_MATCH_ID_COMMAND_PROFILE_TABLEm             0x18140000
#define FLEX_EDITOR_ZONE_1_MATCH_ID_TABLEm                 0x18040000
#define FLEX_EDITOR_ZONE_2_MATCH_ID_TABLEm                 0x18080000
#define FLEX_EDITOR_ZONE_3_MATCH_ID_TABLEm                 0x180c0000
#define FLEX_EDITOR_ZONE_4_MATCH_ID_TABLEm                 0x18100000
#define FLEX_EDITOR_ZONE_4_MATCH_ID_COMMAND_PROFILE_TABLEm             0x18240000
#define FLEX_EDITOR_ZONE_0_EDIT_ID_DEL_TABLEm              0x18280000
#define FLEX_EDITOR_ZONE_1_EDIT_ID_DEL_TABLEm              0x182c0000
#define FLEX_EDITOR_ZONE_2_EDIT_ID_DEL_TABLEm              0x18300000
#define FLEX_EDITOR_ZONE_3_EDIT_ID_DEL_TABLEm              0x18340000
#define FLEX_EDITOR_ZONE_4_EDIT_ID_DEL_TABLEm              0x18380000
#define FLEX_EDITOR_ZONE_3_EDIT_ID_RW_TABLEm               0x18700000
#define FLEX_EDITOR_ZONE_4_EDIT_ID_RW_TABLEm               0x18740000
#define FLEX_EDITOR_ZONE_0_EDIT_ID_INS_1_TABLEm            0x183c0000
#define FLEX_EDITOR_ZONE_1_EDIT_ID_INS_1_TABLEm            0x18440000
#define FLEX_EDITOR_ZONE_2_EDIT_ID_INS_1_TABLEm            0x184c0000
#define FLEX_EDITOR_ZONE_2_EDIT_ID_INS_2_TABLEm            0x18500000
#define FLEX_EDITOR_ZONE_3_EDIT_ID_INS_1_TABLEm            0x18540000
#define FLEX_EDITOR_ZONE_4_EDIT_ID_INS_1_TABLEm            0x185c0000
#define FLEX_EDITOR_ZONE_4_EDIT_ID_INS_2_TABLEm            0x18600000
#define EGR_SEQUENCE_NUMBER_PROFILEm                       0x0ecc0000
#define EGR_HDR_ID_EFPPARS_ENm                             0x1a0c0000
#define EGR_HDR_ID_EFPPARS_PROFILE_TABLEm                  0x1a100000
#define EGR_FLEX_CONTAINER_UPDATE_PROFILE_0m               0x09000000
#define EGR_FLEX_CONTAINER_UPDATE_PROFILE_1m               0x09040000
#define FLEX_EDITOR_FIXED_HDR_LOOPBACK_0_PROFILE_TABLEm    0x18a40000
#define PHB_SELECT_TCAMm                                   0x704c0000
#define CNG_STRENGTH_PROFILEm                              0x70680000
#define DSCP_STRENGTH_PROFILEm                             0x70700000
#define INT_PRI_STRENGTH_PROFILEm                          0x706c0000
#define PHB2_STRENGTH_PROFILEm                             0x70740000
#define EGR_QOS_CTRL_TCAMm                                 0x0ea00000
#define EGR_ZONE_1_QOS_STRENGTH_PROFILEm                   0x0eac0000
#define EGR_ZONE_2_QOS_STRENGTH_PROFILEm                   0x0eb00000
#define EGR_ZONE_3_QOS_STRENGTH_PROFILEm                   0x0eb40000
#define EGR_ZONE_4_QOS_STRENGTH_PROFILEm                   0x0eb80000
#define EGR_ZONE_1_DOT1P_STRENGTH_PROFILE_1m               0x0ec00000
#define EGR_ZONE_1_DOT1P_STRENGTH_PROFILE_2m               0x093c0000
#define EGR_ZONE_3_DOT1P_STRENGTH_PROFILE_1m               0x0ec40000
#define EGR_ZONE_3_DOT1P_STRENGTH_PROFILE_2m               0x09400000
#define EGR_QOS_ECN_PROFILEm                               0x0e140000



typedef struct mem_info_s {
    int      index;
    uint32_t blk;    
    uint32_t addr;
    uint32_t bytes;
}mem_info_t;

#endif
