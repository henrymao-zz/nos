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

/*****************************************************************************************/
/*                             /proc                                                     */
/*****************************************************************************************/
/*
/proc/switchdev/
/proc/switchdev/sinfo                    

/proc/switchdev/reg/COMMAND_CONFIG

/proc/switchdev/mem/EGR_VLAN
/proc/switchdev/mem/VLAN_TAB
/proc/switchdev/mem/VLAN_ATTRS_1
/proc/switchdev/mem/VLAN_ATTRS_1
/proc/switchdev/mem/L2_USER_ENTRY
/proc/switchdev/mem/LPORT_TAB

/proc/switchdev/stats/

*/

typedef struct _proc_reg_data_s {
    uint32_t reg_addr;
    uint8_t  acc_type;
    uint32_t block[8];  //TBD
    uint8_t  num_blk;
}_proc_reg_data_t;

typedef struct _proc_stats_data_s {
    uint32_t port;
}_proc_stats_data_t;

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
#define	_SHR_PBMP_WIDTH		(((_SHR_PBMP_PORT_MAX + 32 - 1)/32)*32)
#define	_SHR_PBMP_WORD_WIDTH		32
#define	_SHR_PBMP_WORD_MAX		\
	((_SHR_PBMP_WIDTH + _SHR_PBMP_WORD_WIDTH-1) / _SHR_PBMP_WORD_WIDTH)

typedef struct _shr_pbmp {
        uint32	pbits[_SHR_PBMP_WORD_MAX];
} bcm_pbmp_t;
    
typedef int bcm_port_t;
typedef int bcm_module_t;
typedef int bcm_trunk_t;
typedef int bcm_cos_t;
typedef int bcm_if_t;
typedef int bcm_multicast_t;
typedef uint16_t bcm_vlan_t;

/* bcm_mac_t */
typedef uint8 bcm_mac_t[6];

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
/*                              SCHAN                                                    */
/*****************************************************************************************/

/* Endian selection register */
#define CMIC_ENDIAN_SELECT              0x00000174

/* S-Channel Control Register */
//Register CMIC_SCHAN_CTRL 16181 is not valid for chips using driver BCM56370_A0
#define CMIC_SCHAN_CTRL                 0x00000050 

#define CMIC_TOP_SBUS_RING_ARB_CTRL_SBUSDMA    (0x4c)

/*
 * SCHAN_CONTROL: control bits
 *
 *  SC_xxx_SET and SC_xxx_CLR can be WRITTEN to CMIC_SCHAN_CTRL.
 *  SC_xxx_TST can be masked against values READ from CMIC_SCHAN_CTRL.
 */
#define SC_MSG_START_SET                (0x80|0)
#define SC_MSG_START_CLR                (0x00|0)
#define SC_MSG_START_TST                0x00000001

#define SC_MSG_DONE_SET                 (0x80|1)
#define SC_MSG_DONE_CLR                 (0x00|1)
#define SC_MSG_DONE_TST                 0x00000002


#define SC_MSG_NAK_SET                  (0x80|21)
#define SC_MSG_NAK_CLR                  (0x00|21)
#define SC_MSG_NAK_TST                  0x00200000

#define SC_MSG_TIMEOUT_SET              (0x80|22)
#define SC_MSG_TIMEOUT_CLR              (0x00|22)
#define SC_MSG_TIMEOUT_TST              0x00400000


#define CMIC_COMMON_POOL_SHARED_CONFIG_OFFSET         (0xd024)
#define CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_ARB_CTRL_OFFSET (0xd008)
#define CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_OFFSET (0xd000)
#define CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_1_OFFSET (0xd004)
#define CMIC_COMMON_POOL_SHARED_SCHAN_FIFO_WRITE_ARB_CTRL_OFFSET (0xd00c)

/*
 * SCHAN_CONTROL: CMICX 
 */
#define CMIC_CMC_NUM_MAX      (2)

#define CMIC_SCHAN_NUM_MAX      (3)
#define CMIC_CCMDMA_NUM_MAX     (2)
#define CMIC_TOP_SBUS_RING_ARB_CTRL_SET   (0xEEEEEEEE)

#define CMIC_TOP_SBUS_RING_MAP_0_7_OFFSET             (0xc)
#define CMIC_TOP_SBUS_RING_MAP_104_111_OFFSET         (0x40)
#define CMIC_TOP_SBUS_RING_MAP_112_119_OFFSET         (0x44)
#define CMIC_TOP_SBUS_RING_MAP_120_127_OFFSET         (0x48)
#define CMIC_TOP_SBUS_RING_MAP_16_23_OFFSET           (0x14)
#define CMIC_TOP_SBUS_RING_MAP_24_31_OFFSET           (0x18)
#define CMIC_TOP_SBUS_RING_MAP_32_39_OFFSET           (0x1c)
#define CMIC_TOP_SBUS_RING_MAP_40_47_OFFSET           (0x20)
#define CMIC_TOP_SBUS_RING_MAP_48_55_OFFSET           (0x24)
#define CMIC_TOP_SBUS_RING_MAP_56_63_OFFSET           (0x28)
#define CMIC_TOP_SBUS_RING_MAP_64_71_OFFSET           (0x2c)
#define CMIC_TOP_SBUS_RING_MAP_72_79_OFFSET           (0x30)
#define CMIC_TOP_SBUS_RING_MAP_80_87_OFFSET           (0x34)
#define CMIC_TOP_SBUS_RING_MAP_88_95_OFFSET           (0x38)
#define CMIC_TOP_SBUS_RING_MAP_8_15_OFFSET            (0x10)
#define CMIC_TOP_SBUS_RING_MAP_96_103_OFFSET          (0x3c)
#define CMIC_TOP_SBUS_TIMEOUT_OFFSET                  (0x0)

#define CMIC_COMMON_POOL_SCHAN_CH0_CTRL_OFFSET        (0x10000)
#define CMIC_COMMON_POOL_SCHAN_CH0_ERR_OFFSET         (0x10008)
#define CMIC_COMMON_POOL_SCHAN_CH0_MESSAGE_OFFSET     (0x1000c)
#define CMIC_COMMON_POOL_SCHAN_CH1_ACK_DATA_BEAT_COUNT_OFFSET (0x10104)
#define CMIC_COMMON_POOL_SCHAN_CH1_CTRL_OFFSET        (0x10100)
#define CMIC_COMMON_POOL_SCHAN_CH1_ERR_OFFSET         (0x10108)
#define CMIC_COMMON_POOL_SCHAN_CH1_MESSAGE_OFFSET     (0x1010c)
#define CMIC_COMMON_POOL_SCHAN_CH2_ACK_DATA_BEAT_COUNT_OFFSET (0x10204)
#define CMIC_COMMON_POOL_SCHAN_CH2_CTRL_OFFSET        (0x10200)
#define CMIC_COMMON_POOL_SCHAN_CH2_ERR_OFFSET         (0x10208)
#define CMIC_COMMON_POOL_SCHAN_CH2_MESSAGE_OFFSET     (0x1020c)
#define CMIC_COMMON_POOL_SCHAN_CH3_ACK_DATA_BEAT_COUNT_OFFSET (0x10304)
#define CMIC_COMMON_POOL_SCHAN_CH3_CTRL_OFFSET        (0x10300)
#define CMIC_COMMON_POOL_SCHAN_CH3_ERR_OFFSET         (0x10308)
#define CMIC_COMMON_POOL_SCHAN_CH3_MESSAGE_OFFSET     (0x1030c)
#define CMIC_COMMON_POOL_SCHAN_CH4_ACK_DATA_BEAT_COUNT_OFFSET (0x10404)
#define CMIC_COMMON_POOL_SCHAN_CH4_CTRL_OFFSET        (0x10400)
#define CMIC_COMMON_POOL_SCHAN_CH4_ERR_OFFSET         (0x10408)
#define CMIC_COMMON_POOL_SCHAN_CH4_MESSAGE_OFFSET     (0x1040c)

#define CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(ch, n)       \
                              (CMIC_COMMON_POOL_SCHAN_CH0_MESSAGE_OFFSET + (ch*0x100) + (4*n))
#define CMIC_COMMON_POOL_SCHAN_CHx_CTRL(ch)       \
                              (CMIC_COMMON_POOL_SCHAN_CH0_CTRL_OFFSET + (ch*0x100))

#define CMIC_COMMON_POOL_SCHAN_CHx_ERR(ch)       \
                              (CMIC_COMMON_POOL_SCHAN_CH0_ERR_OFFSET + (ch*0x100))

/* CMIC_COMMON_POOL_SCHAN_CHx_CTRL(x) */
#define SC_CHx_MSG_CLR                 (0x00000000)
#define SC_CHx_MSG_START               (0x00000001)
#define SC_CHx_MSG_DONE                (0x00000002)
#define SC_CHx_SCHAN_ABORT             (0x00000004)
#define SC_CHx_MSG_SER_CHECK_FAIL      (0x00100000)
#define SC_CHx_MSG_NAK                 (0x00200000)
#define SC_CHx_MSG_TIMEOUT_TST         (0x00400000)
#define SC_CHx_MSG_SCHAN_ERR           (0x00800000)

/* CMIC_COMMON_POOL_SCHAN_CHx_ERR(x) */
#define SC_CHx_SCHAN_ERR_NACK_BIT        (0x00000001)
#define SC_CHx_SCHAN_ERR_ERR_BIT         (0x00000040)

//CMIC_COMMON_POOL_SCHAN_CHx_ERR
typedef union schan_err_s {
    struct reg_ {
    #if defined(LE_HOST)
        uint32_t nack:1;          /* NACKf      */
        uint32_t r0:3;            /* Reserved   */
        uint32_t err_code:2;      /* ERR_CODEf  */
        uint32_t errbit:1;        /* ERRBITf    */
        uint32_t data_len:7;      /* DATA_LENf  */
        uint32_t src_port:6;      /* SRC_PORTf  */
        uint32_t dst_port:6;      /* DST_PORTf  */
        uint32_t op_code:6;       /* OP_CODEf   */
    #else
        uint32_t op_code:6;       /* OP_CODEf   */
        uint32_t dst_port:6;      /* DST_PORTf  */
        uint32_t src_port:6;      /* SRC_PORTf  */
        uint32_t data_len:7;      /* DATA_LENf  */
        uint32_t errbit:1;        /* ERRBITf    */
        uint32_t err_code:2;      /* ERR_CODEf  */
        uint32_t r0:3;            /* Reserved   */
        uint32_t nack:1;          /* NACKf      */
    #endif
    } reg;
    uint32_t word;
} schan_err_t;

/*
 * S-Channel Message Types
 */

 #define BP_WARN_STATUS_MSG            0x01
 #define BP_DISCARD_STATUS_MSG         0x02
 #define COS_QSTAT_NOTIFY_MSG          0x03      /* Not on XGS */
 #define IPIC_HOL_STAT_MSG             0x03      /* 5665 (alias) */
 #define HOL_STAT_NOTIFY_MSG           0x04
 #define READ_MEMORY_CMD_MSG           0x07
 #define READ_MEMORY_ACK_MSG           0x08
 #define WRITE_MEMORY_CMD_MSG          0x09
 #define WRITE_MEMORY_ACK_MSG          0x0a
 #define READ_REGISTER_CMD_MSG         0x0b
 #define READ_REGISTER_ACK_MSG         0x0c
 #define WRITE_REGISTER_CMD_MSG        0x0d
 #define WRITE_REGISTER_ACK_MSG        0x0e
 #define ARL_INSERT_CMD_MSG            0x0f
 #define ARL_INSERT_DONE_MSG           0x10
 #define ARL_DELETE_CMD_MSG            0x11
 #define ARL_DELETE_DONE_MSG           0x12
 #define LINKSTAT_NOTIFY_MSG           0x13      /* Strata I/II only */
 #define MEMORY_FAIL_NOTIFY            0x14
 #define INIT_CFAP_MSG                 0x15      /* 5690 only */
 #define IPIC_GBP_FULL_MSG             0x15      /* 5665 (alias) */
 #define IPIC_GBP_AVAIL_MSG            0x16      /* 5665 (alias) */
 #define ENTER_DEBUG_MODE_MSG          0x17
 #define EXIT_DEBUG_MODE_MSG           0x18
 #define ARL_LOOKUP_CMD_MSG            0x19
 #define L3_INSERT_CMD_MSG             0x1a
 #define L3_INSERT_DONE_MSG            0x1b
 #define L3_DELETE_CMD_MSG             0x1c
 #define L3_DELETE_DONE_MSG            0x1d
 #define L3_LOOKUP_CMD_MSG             0x1e      /* 5695 */
 #define L2_LOOKUP_CMD_MSG             0x20      /* 56504 / 5630x / 5610x */
 #define L2_LOOKUP_ACK_MSG             0x21      /* 56504 / 5630x / 5610x */
 #define L3X2_LOOKUP_CMD_MSG           0x22      /* 56504 / 5630x / 5610x */
 #define L3X2_LOOKUP_ACK_MSG           0x23      /* 56504 / 5630x / 5610x */
 /* New for 5662x (see soc_feature_generic_table_ops) */
 #define TABLE_INSERT_CMD_MSG          0x24      /* 5662x */
 #define TABLE_INSERT_DONE_MSG         0x25      /* 5662x */
 #define TABLE_DELETE_CMD_MSG          0x26      /* 5662x */
 #define TABLE_DELETE_DONE_MSG         0x27      /* 5662x */
 #define TABLE_LOOKUP_CMD_MSG          0x28      /* 5662x */
 #define TABLE_LOOKUP_DONE_MSG         0x29      /* 5662x */
 #define FIFO_POP_CMD_MSG              0x2a      /* 5662x */
 #define FIFO_POP_DONE_MSG             0x2b      /* 5662x */
 #define FIFO_PUSH_CMD_MSG             0x2c      /* 5662x */
 #define FIFO_PUSH_DONE_MSG            0x2d      /* 5662x */

 
/*
 * S-Channel Message Buffer Registers (0x00 -> 0x4c, or 0x800 -> 0x854).
 * Block where S-Channel messages are written to CMIC.
 */
#define CMIC_SCHAN_MESSAGE(unit, word)  (0x00 + 4 * (word))

#define CMIC_SCHAN_WORDS                22

/* number of words to use when allocating space */
/** Iproc17 sbus data increased to 120 bytes */
#define CMIC_SCHAN_WORDS_ALLOC 32

/* SCHAN FIFO */

#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_COMMAND_MEMORY_OFFSET (0x12100)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_CTRL_OFFSET (0x1200c)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_RESP_HOSTMEM_START_ADDR_LOWER_OFFSET (0x12014)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_RESP_HOSTMEM_START_ADDR_UPPER_OFFSET (0x12010)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_STATUS_OFFSET (0x12020)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_SUMMARY_HOSTMEM_START_ADDR_LOWER_OFFSET (0x1201c)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_SUMMARY_HOSTMEM_START_ADDR_UPPER_OFFSET (0x12018)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH1_COMMAND_MEMORY_OFFSET (0x12680)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH1_CTRL_OFFSET (0x12024)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH1_RESP_HOSTMEM_START_ADDR_LOWER_OFFSET (0x1202c)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH1_RESP_HOSTMEM_START_ADDR_UPPER_OFFSET (0x12028)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH1_STATUS_OFFSET (0x12038)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH1_SUMMARY_HOSTMEM_START_ADDR_LOWER_OFFSET (0x12034)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_CH1_SUMMARY_HOSTMEM_START_ADDR_UPPER_OFFSET (0x12030)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_ECC_CTRL_OFFSET (0x12000)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_ECC_STATUS_OFFSET (0x12004)
#define CMIC_COMMON_POOL_SCHAN_FIFO_0_TM_CTRL_OFFSET  (0x12008)



#define CMIC_SCHAN_FIFO_NUM_MAX      (2)
/* Number of commands SCHAN FIFO can accomodate in one channel */
#define CMIC_SCHAN_FIFO_CMD_SIZE_MAX   (16)
#define MAP_SCHAN_FIFO_MEMWR_REQ_CMC0        (0x00)
#define MAP_SCHAN_FIFO_MEMWR_REQ_CMC1        (0x01)
#define SCHAN_FIFO_MEMWR_WRR_WEIGHT          (0x0E)
#define SCHAN_FIFO_AXI_ID                    (0x05)
#define SCHAN_FIFO_RESPONSE_WORD_SIZE        (32) /*128 bytes aligned */
#define SCHAN_FIFO_SUMMARY_ACKDATA_MASK      (0xFF)
#define SCHAN_FIFO_SUMMARY_STAT_SHIFT         (8)


#define CMIC_SCHAN_FIFO_CHx_CTRL(x) \
                               (CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_CTRL_OFFSET + x*0x18)
#define CMIC_SCHAN_FIFO_CHx_RESP_ADDR_LOWER(x) \
             (CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_RESP_HOSTMEM_START_ADDR_LOWER_OFFSET + x * 0x18)
#define CMIC_SCHAN_FIFO_CHx_RESP_ADDR_UPPER(x) \
             (CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_RESP_HOSTMEM_START_ADDR_UPPER_OFFSET + x * 0x18)

#define CMIC_SCHAN_FIFO_CHx_STATUS(x) \
             (CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_STATUS_OFFSET + x * 0x18)
#define CMIC_SCHAN_FIFO_CHx_SUMMARY_ADDR_LOWER(x) \
             (CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_SUMMARY_HOSTMEM_START_ADDR_LOWER_OFFSET + x * 0x18)
#define CMIC_SCHAN_FIFO_CHx_SUMMARY_ADDR_UPPER(x) \
             (CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_SUMMARY_HOSTMEM_START_ADDR_UPPER_OFFSET + x * 0x18)
#define CMIC_SCHAN_FIFO_CHx_COMMAND(x, n) \
             (CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_COMMAND_MEMORY_OFFSET + (x * 0x580) + (4 * n))

#define SCHAN_FIFO_SUMRY_STAT_COMPLETE        (0x00 << (5 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))
#define SCHAN_FIFO_SUMRY_STAT_ECCERR          (0x01 << (5 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))
#define SCHAN_FIFO_SUMRY_STAT_SBUSERR         (0x02 << (5 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))
#define SCHAN_FIFO_SUMRY_STAT_OTHERR          (0x03 << (5 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))

#define SCHAN_FIFO_SUMRY_ERR_SBUSNACK         (0x01 + SCHAN_FIFO_SUMMARY_STAT_SHIFT)
#define SCHAN_FIFO_SUMRY_ERR_SBUSECODE        (0x03 << (1 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))
#define SCHAN_FIFO_SUMRY_ERR_SBUSERR          (0x01 << (3 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))
#define SCHAN_FIFO_SUMRY_ERR_OPCODE           (0x01 << (4 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))

#define SCHAN_FIFO_SUMRY_ABRT_ECCCMD          (0x01 + SCHAN_FIFO_SUMMARY_STAT_SHIFT)
#define SCHAN_FIFO_SUMRY_ABRT_OPCODE          (0x01 << (1 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))
#define SCHAN_FIFO_SUMRY_ABRT_CMDLEN          (0x01 << (2 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))
#define SCHAN_FIFO_SUMRY_ABRT_TIMOUT          (0x01 << (3 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))
#define SCHAN_FIFO_SUMRY_ABRT_DBEAT           (0x01 << (4 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))
#define SCHAN_FIFO_SUMRY_ABRT_ECCMEM          (0x01 << (5 + SCHAN_FIFO_SUMMARY_STAT_SHIFT))

#define SCHAN_FIFO_SUMMARY_UPDATE_INTERVAL    (0x10)
#define SCHAN_FIFO_MSG_DONE                   (0x01)

/* fields of CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_CTRL / CMIC_SCHAN_FIFO_CHx_CTRL */
#define SCHAN_FIFO_CTRL_START                 1
#define SCHAN_FIFO_CTRL_ABORT                 2
/* fields of CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_STATUS / CMIC_SCHAN_FIFO_CHx_STATUS(x) */
#define SCHAN_FIFO_STATUS_DONE                1
#define SCHAN_FIFO_STATUS_ERROR               2

/* Some basic definitions */
#define SOC_BLOCK_MSB_BP                30
#define SOC_BLOCK_BP                    20
#define SOC_MEMOFS_BP                   16
#define SOC_REGIDX_BP                   12
#define SOC_RT_BP                       25


/* PCI SLAVE OFFSET */
#define CMIC_PCIE_SO_OFFSET                  (0x10000000)

/*
 * The endianness of the host is taken into account by the routines
 * which copy S-Channel messages into and out of PCI space as 32-bit
 * words.  Unfortunately, the compiler also switches bit field packing
 * order according to host endianness.  We must undo this "feature" by
 * giving the fields in both orders.
 */

typedef union schan_header_s {
    struct v2_s {
#if defined(LE_HOST)
        uint32 nack:1;
        uint32 bank_ignore_mask:2;
        uint32 dma:1;
        uint32 ecode: 2;
        uint32 err:1;
        uint32 data_byte_len:7;
        uint32 src_blk:6;
        uint32 dst_blk:6;
        uint32 opcode:6;
#else
        uint32 opcode:6;
        uint32 dst_blk:6;
        uint32 src_blk:6;
        uint32 data_byte_len:7;
        uint32 err:1;
        uint32 ecode: 2;
        uint32 dma:1;
        uint32 bank_ignore_mask:2;
        uint32 nack:1;
#endif
    } v2;
    struct v3_s {
#if defined(LE_HOST)
        uint32 nack:1;
        uint32 bank_ignore_mask:2;
        uint32 dma:1;
        uint32 ecode: 2;
        uint32 err:1;
        uint32 data_byte_len:7;
        uint32 acc_type:3;
        uint32 :3;
        uint32 dst_blk:6;
        uint32 opcode:6;
#else
        uint32 opcode:6;
        uint32 dst_blk:6;
        uint32 :3;
        uint32 acc_type:3;
        uint32 data_byte_len:7;
        uint32 err:1;
        uint32 ecode: 2;
        uint32 dma:1;
        uint32 bank_ignore_mask:2;
        uint32 nack:1;
#endif
    } v3;
    struct v4_s {
#if defined(LE_HOST)
        uint32 nack:1;
        uint32 bank_ignore_mask:2;
        uint32 dma:1;
        uint32 ecode: 2;
        uint32 err:1;
        uint32 data_byte_len:7;
        uint32 acc_type:5;
        uint32 dst_blk:7;
        uint32 opcode:6;
#else
        uint32 opcode:6;
        uint32 dst_blk:7;
        uint32 acc_type:5;
        uint32 data_byte_len:7;
        uint32 err:1;
        uint32 ecode: 2;
        uint32 dma:1;
        uint32 bank_ignore_mask:2;
        uint32 nack:1;
#endif
    } v4;
    uint32 word;
} schan_header_t;

typedef struct schan_msg_plain_s {
    /* GBP Full Notification */
    /* GBP Available Notification */
    /* Write Memory Ack */
    /* Write Register Ack */
    /* ARL Insert Complete */
    /* ARL Delete Complete */
    /* Memory Failed Notification */
    /* Initialize CFAP (Cell FAP) */
    /* Initialize SFAP (Slot FAP) */
    /* Enter Debug Mode */
    /* Exit Debug Mode */
    schan_header_t header;
} schan_msg_plain_t;

typedef struct schan_msg_bitmap_s {
    /* Back Pressure Warning Status */
    /* Back Pressure Discard Status */
    /* Link Status Notification (except 5695) */
    /* COS Queue Status Notification */
    /* HOL Status Notification */
    schan_header_t header;
    uint32 bitmap;
    uint32 bitmap_word1;  /* 5665 only, so far */
} schan_msg_bitmap_t;

typedef struct schan_msg_readcmd_s {
    /* Read Memory Command */
    /* Read Register Command */
    schan_header_t header;
    uint32 address;
} schan_msg_readcmd_t;

typedef struct schan_msg_readresp_s {
    /* Read Memory Ack */
    /* Read Register Ack */
    schan_header_t header;
    uint32 data[CMIC_SCHAN_WORDS_ALLOC - 1];
} schan_msg_readresp_t;

typedef struct schan_msg_writecmd_s {
    /* Write Memory Command */
    /* Write Register Command */
    schan_header_t header;
    uint32 address;
    uint32 data[CMIC_SCHAN_WORDS_ALLOC - 2];
} schan_msg_writecmd_t;

#define SCHAN_GEN_RESP_TYPE_FOUND           0
#define SCHAN_GEN_RESP_TYPE_NOT_FOUND       1
#define SCHAN_GEN_RESP_TYPE_FULL            2
#define SCHAN_GEN_RESP_TYPE_INSERTED        3
#define SCHAN_GEN_RESP_TYPE_REPLACED        4
#define SCHAN_GEN_RESP_TYPE_DELETED         5
#define SCHAN_GEN_RESP_TYPE_ENTRY_IS_OLD    6
#define SCHAN_GEN_RESP_TYPE_CLEARED_VALID   7
#define SCHAN_GEN_RESP_TYPE_L2_FIFO_FULL    8
#define SCHAN_GEN_RESP_TYPE_MAC_LIMIT_THRE  9
#define SCHAN_GEN_RESP_TYPE_MAC_LIMIT_DEL   10
#define SCHAN_GEN_RESP_TYPE_L2_STATIC       11

#define SCHAN_GEN_RESP_L2_MOD_FIFO_FULL     6
#define SCHAN_GEN_RESP_TYPE_ERROR           15

#define SCHAN_GEN_RESP_ERR_INFO_ERROR_IN_HASH_TABLE 0
#define SCHAN_GEN_RESP_ERR_INFO_ERROR_IN_LP_TABLE 1


#define SCHAN_GEN_RESP_ERROR_BUSY           -1
#define SCHAN_GEN_RESP_ERROR_PARITY         -1

typedef struct schan_genresp_s {
#if defined(LE_HOST)
    uint32 index:20,
           r0:1,            /* Reserved */
           err_info:4,      /* SCHAN_GEN_RESP_ERROR_* */
           r1:1,            /* Reserved */
           type:4,          /* SCHAN_GEN_RESP_TYPE_* */
           src:2;
#else
    uint32 src:2,
           type:4,          /* SCHAN_GEN_RESP_TYPE_* */
           r0:1,              /* Reserved */
           err_info:4,      /* SCHAN_GEN_RESP_ERROR_* */
           r1:1,              /* Reserved */
           index:20;
#endif
} schan_genresp_t;

typedef struct schan_msg_genresp_s {
    /* Generic table Insert/Delete/Lookup Command 5662x */
    schan_header_t header;
    schan_genresp_t response;
    uint32 data[CMIC_SCHAN_WORDS_ALLOC - 2];
} schan_msg_genresp_t;

typedef union schan_msg_u {
    schan_header_t header;
    uint32 header_dword;
    schan_msg_plain_t plain;
    schan_msg_bitmap_t bitmap;
    schan_msg_readcmd_t readcmd;
    schan_msg_readresp_t readresp;
    schan_msg_writecmd_t writecmd;
    //schan_msg_arlins_t arlins;
    //schan_msg_arldel_t arldel;
    //schan_msg_arllkup_t arllkup;
    //schan_msg_l3ins_t l3ins;
    //schan_msg_l3del_t l3del;
    //schan_msg_l3lkup_t l3lkup;
    //schan_msg_l2x2_t    l2x2;
    //schan_msg_l3x2_t    l3x2;
    //schan_msg_gencmd_t  gencmd;
    schan_msg_genresp_t genresp;
    //schan_msg_genresp_v2_t genresp_v2;
    //schan_msg_popcmd_t  popcmd;
    //schan_msg_popresp_t popresp;
    //schan_msg_pushcmd_t  pushcmd;
    //schan_msg_pushresp_t pushresp;
    uint32 dwords[CMIC_SCHAN_WORDS_ALLOC];
    uint8 bytes[sizeof(uint32) * CMIC_SCHAN_WORDS_ALLOC];
} schan_msg_t;

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

#define PMQ_XGXS0_CTRL_REGr            0x2020100
#define ENHANCED_HASHING_CONTROL_2r    0x82001300
#define XLPORT_MAC_CONTROLr            0x2021000


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
    uint64 r0:27,            /* Reserved               */
           edit_ctrl_id:4,   /* EDIT_CTRL_IDf          */
           qos_ctrl_id:4,    /* QOS_CTRL_IDf           */
           port_type:3,      /* PORT_TYPEf             */
           my_modid:8,       /* MY_MODIDf              */
           ctrl_id:8,        /* EGR_PORT_CTRL_IDf      */
           profile_idx:10;   /* EGR_LPORT_PROFILE_IDXf */
#else
    uint64 profile_idx:10,   /* EGR_LPORT_PROFILE_IDXf */
           ctrl_id:8,        /* EGR_PORT_CTRL_IDf      */
           my_modid:8,       /* MY_MODIDf              */
           port_type:3,      /* PORT_TYPEf             */
           qos_ctrl_id:4,    /* QOS_CTRL_IDf           */
           edit_ctrl_id:4,   /* EDIT_CTRL_IDf          */
           r0:27;            /* Reserved               */
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


#define ING_DEST_PORT_ENABLEm     0x84500000

#define EPC_LINK_BMAPm            0x84240000

//Memory: EGR_ENABLE.epipe0 alias EDB_ENABLE address 0x28200000
//Entries: 79 with indices 0-78 (0x0-0x4e), each 1 bytes 1 words
/*
soc_field_info_t soc_EGR_ENABLE_BCM53400_A0m_fields[] = {
    { PRT_ENABLEf, 1, 0, 0 | SOCF_GLOBAL }
};
*/
#define EGR_ENABLEm                     0x28200000

//Memory: EGR_PER_PORT_BUFFER_SFT_RESET.epipe0 alias EDB_PER_PORT_BUFFER_SFT_RESET address 0x28300000
//Entries: 79 with indices 0-78 (0x0-0x4e), each 1 bytes 1 words
#define EGR_PER_PORT_BUFFER_SFT_RESETm  0x28300000

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
    struct _ch_addr_ {
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


const bcm_mac_t _mac_spanning_tree =
	{0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};

const bcm_mac_t _mac_all_routers =
	{0x01, 0x00, 0x5e, 0x00, 0x00, 0x02};

const bcm_mac_t _mac_all_zeroes =
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const bcm_mac_t _mac_all_ones =
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff};



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
    { STlGf, 9, 1, SOCF_LE | SOCF_GLOBAL },
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
#define CANCUN_FILENAME_SIZE        (256)
#define CANCUN_VERSION_LEN_MAX      (32)
#define CANCUN_LIST_BUF_LEN         (512)
#define CANCUN_DEST_MEM_NUM_MAX     (16)
#define CANCUN_DEST_FIELD_NUM_MAX   (16)

/*
 * CANCUN file type enumeration
*/
typedef enum {
    CANCUN_SOC_FILE_TYPE_UNKNOWN,
    CANCUN_SOC_FILE_TYPE_CIH,
    CANCUN_SOC_FILE_TYPE_CMH,
    CANCUN_SOC_FILE_TYPE_CCH,
    CANCUN_SOC_FILE_TYPE_CFH,
    CANCUN_SOC_FILE_TYPE_CEH,

    /*Note: keep CANCUN_SOC_FILE_TYPE_NUM as the latest element of this enum
     * and update CANCUN_FILE_TYPE_NAMES_INITIALIZER accordingly*/
    CANCUN_SOC_FILE_TYPE_NUM
} soc_cancun_file_type_e;

#define CANCUN_FILE_TYPE_NAMES_INITIALIZER { \
    "UNKNOWN",  \
    "CIH",      \
    "CMH",      \
    "CCH",      \
    "CFH",      \
    "CEH",      \
}

/*
 * CANCUN file format enumeration
*/
typedef enum {
    CANCUN_SOC_FILE_FORMAT_UNKNOWN,
    CANCUN_SOC_FILE_FORMAT_PIO,
    CANCUN_SOC_FILE_FORMAT_DMA,
    CANCUN_SOC_FILE_FORMAT_FIFO,
    CANCUN_SOC_FILE_FORMAT_YAML,
    CANCUN_SOC_FILE_FORMAT_PACK,

    /*Note: keep CANCUN_SOC_FILE_FORMAT_NUM as the latest element of this enum
     * and update CANCUN_FILE_FORMAT_NAMES_INITIALIZER accordingly*/
    CANCUN_SOC_FILE_FORMAT_NUM
} soc_cancun_file_format_e;

#define CANCUN_FILE_FORMAT_NAMES_INITIALIZER { \
    "UNKNOWN",  \
    "PIO",      \
    "DMA",      \
    "FIFO",     \
    "YMAL",     \
    "PACK",     \
}

/*
 * CANCUN file load status enumeration
*/
typedef enum {
    CANCUN_SOC_FILE_LOAD_NONE,
    CANCUN_SOC_FILE_LOAD_COMPLETE,
    CANCUN_SOC_FILE_LOAD_IN_PROGRESS,
    CANCUN_SOC_FILE_LOAD_FAILED,

    /*Note: keep CANCUN_SOC_FILE_LOAD_STATUS_NUM as the latest element of this enum
     * and update CANCUN_FILE_LOAD_STATUS_INITIALIZER accordingly*/
    CANCUN_SOC_FILE_LOAD_STATUS_NUM
} soc_cancun_file_load_status_e;

#define CANCUN_FILE_LOAD_STATUS_INITIALIZER { \
    "NOT LOADED",       \
    "LOADED",           \
    "LOAD IN PROGRESS", \
    "LOAD FAILED",      \
}

/*
 * CANCUN version prefix enumeration
*/
typedef enum {
    CANCUN_VERSION_PREFIX_INVALID,
    CANCUN_VERSION_PREFIX_AV,
    CANCUN_VERSION_PREFIX_CC,
    CANCUN_VERSION_PREFIX_NONE,

    /*Note: keep CANCUN_VERSION_PREFIX_NUM as the latest element of this enum
     * and update CANCUN_VERSION_PREFIX_INITIALIZER accordingly*/
    CANCUN_VERSION_PREFIX_NUM
} soc_cancun_version_prefix_e;

#define CANCUN_VERSION_PREFIX_INITIALIZER { \
    "INVALID",  \
    "AV",       \
    "CC",       \
    "",         \
}

/*
 * CANCUN file branch ID enumeration, start with 0x01
*/
typedef enum {
    CANCUN_FILE_BRANCH_ID_DEF = 0x01,
    CANCUN_FILE_BRANCH_ID_HGoE,
    CANCUN_FILE_BRANCH_ID_GSH,
    CANCUN_FILE_BRANCH_ID_NUM
} soc_cancun_file_branch_id_e;

/*
 * CANCUN version flags
*/
#define CANCUN_VERSION_FLAG_TEST        0x80000000
#define CANCUN_VERSION_MASK             0x7FFFFFFF
#define CANCUN_VERSION_REGSFILE_MASK    0x7FFFFF00

/*
 * CANCUN version offsets
*/
#define CANCUN_VERSION_OFFSET_BRANCH_ID     (24)
#define CANCUN_VERSION_OFFSET_MAJOR         (16)
#define CANCUN_VERSION_OFFSET_MINOR         (8)
#define CANCUN_VERSION_OFFSET_PATCH         (0)
#define CANCUN_VERSION_OFFSET_REVISION      (20)

#define CANCUN_VERSION_REVISION_MASK       (0x00F00000)

/*
 * SDK release version offsets
*/
#define CANCUN_SDK_VERSION_OFFSET_MAJOR     (24)
#define CANCUN_SDK_VERSION_OFFSET_MINOR     (16)
#define CANCUN_SDK_VERSION_OFFSET_BUILD     (8)
#define CANCUN_SDK_VERSION_OFFSET_RESERVED  (0)

/*
 * CANCUN file header structure
 * Contains header format of a CANCUN loadable file
 */
typedef struct soc_cancun_file_header_s {
    uint32 file_identifier;     /* Cancun File identifier */
    uint32 file_type;           /* Cancun File Type */
    uint32 chip_rev_id;         /* Chip & rev id */
    uint32 version;             /* Cancun Version */
    uint32 file_length;         /* File length in 32-bit words */
    uint32 num_data_blobs;      /* Number of data blobs */
    uint32 sdk_version;         /* SDK version that builds this file */
    uint32 rsvd2;
} soc_cancun_file_header_t;


/*
 * CANCUN file information structure
 * Contains information of a CANCUN loadable file
 */
typedef struct soc_cancun_file_s {
    soc_cancun_file_header_t header;        /* File header */
    soc_cancun_file_type_e type;            /* File type enum */
    soc_cancun_file_format_e format;        /* File format enum */
    char filename[CANCUN_FILENAME_SIZE];    /* Filename */
    int valid;                              /* File validity */
    uint32 status;                          /* Loading status*/
} soc_cancun_file_t;

/* FLOW DB Control Structure */
typedef struct soc_flow_db_s {
    uint32 status;
    uint32 version;
    soc_cancun_file_t file;   /* Flow DB file entry */
    soc_flow_db_flow_map_t *flow_map;
    char *str_tbl;
} soc_flow_db_t;

/*
 * CANCUN CIH control structure
 * Contains information of currently loaded CIH files.
 */
typedef struct soc_cancun_cih_s {
    uint32 status;            /* Loading status*/
    uint32 version;           /* Loaded version */
    soc_cancun_file_t  file;  /* CIH file entry */
} soc_cancun_cih_t;

/*
 * CANCUN hash table structure
 * Generalized hash table for CANCUN support
 *
 *     hash_key = (pa * field_enum + pb * mem_reg_enum + pc) % pd
 */
typedef struct soc_cancun_hash_table_s {
    uint32 pa;                  /* Hash function parameter pA */
    uint32 pb;                  /* Hash function parameter pB */
    uint32 pc;                  /* Hash function parameter pC */
    uint32 pd;                  /* Hash function parameter pD */
    uint32 entry_num;           /* Total entry number in hash table */
    uint32 table_size;          /* Word count size for hash table */
    uint32 table_entry;         /* Hash table entry point */
} soc_cancun_hash_table_t;

/*
 * CANCUN generic hash entry header
 */
typedef struct soc_cancun_entry_hdr_s {
    uint32 entry_size;
    uint32 format;
    uint32 src_mem;
    uint32 src_field;
    int src_app;
} soc_cancun_entry_hdr_t;

/*
 * CANCUN coverage list structure
 */
typedef struct soc_cancun_list_s {
    uint32 entry_size;
    uint32 format;
    uint32 src_mem;
    uint32 src_field;
    int src_app;
    uint32 member_num;
    uint32 members;
} soc_cancun_list_t;

/*
 * CANCUN destination entry structure for reporting use
 */
typedef struct soc_cancun_dest_entry_s {
    uint32 dest_index_num;      /* Destination memory table index number*/
    uint32 dest_field_num;      /* Destination field number in this memory*/
    uint32 dest_mems[CANCUN_DEST_MEM_NUM_MAX];      /* Destination memories */
    uint32 dest_fields[CANCUN_DEST_FIELD_NUM_MAX];  /* Destination fields */
    uint32 dest_values[CANCUN_DEST_FIELD_NUM_MAX];  /* Destination values */
} soc_cancun_dest_entry_t;

/*
 * CANCUN CMH map structure
*/
typedef struct soc_cancun_cmh_map_s {
    uint32 entry_size;          /* Entry size in word*/
    uint32 format;              /* CANCUN CMH format */
    uint32 src_mem;             /* Source memory or register enumeration */
    uint32 src_field;           /* Source field enumeration */
    int src_app;                /* Source application enumeration */
    uint32 src_value_num;       /* Number of valid values */
    uint32 dest_mem_num;        /* Number of destination memories or registers */
    uint32 dest_map_entry;      /* Entry point for destination mapping */
} soc_cancun_cmh_map_t;

/*
 * CANCUN CMH control structure
 * Contains information of currently loaded CMH files.
 */
typedef struct soc_cancun_cmh_s {
    uint32 status;                  /* Loading status*/
    uint32 version;                 /* Loaded version */
    uint32 sdk_version;             /* Built SDK version */
    soc_cancun_file_t file;         /* CMH file entry */
    uint32* cmh_table;              /* Entry of CMH hash table */
    uint32* cmh_list;               /* Entry of CMH list of mem/field */
} soc_cancun_cmh_t;

#define SOC_CANCUN_CMH_LIST_ENTRY_SIZE          (3)
#define SOC_CANCUN_CMH_LIST_ENTRY_OFFSET_MEM    (0)
#define SOC_CANCUN_CMH_LIST_ENTRY_OFFSET_FIELD  (1)
#define SOC_CANCUN_CMH_LIST_ENTRY_OFFSET_APP    (2)


#define SOC_CANCUN_PSEUDO_REGS_NUM_V0          (60)
#define SOC_CANCUN_PSEUDO_REGS_BLOCK_BYTE_SIZE_V0  \
                                            (0x100 * SOC_CANCUN_PSEUDO_REGS_NUM_V0)
#define SOC_CANCUN_PSEUDO_REGS_NUM          (60 + 100)
#define SOC_CANCUN_PSEUDO_REGS_BLOCK_BYTE_SIZE  \
                                            (0x100 * SOC_CANCUN_PSEUDO_REGS_NUM)


/*
 * CANCUN CCH map structure
*/
typedef struct soc_cancun_cch_map_s {
    uint32 entry_size;          /* Entry size in word*/
    uint32 format;              /* CANCUN CCH format */
    uint32 src_mem;             /* Source memory or register enumeration */
    uint32 src_field;           /* Source field enumeration */
    int src_app;                /* Source application enumeration */
    uint32 src_value_num;       /* Number of valid values */
    uint32 dest_mem_num;        /* Number of destination memories or registers */
    uint32 dest_map_entry;      /* Entry point for destination mapping */
} soc_cancun_cch_map_t;

/*
 * CANCUN CCH control structure
 * Contains information of currently loaded CMH files.
 */
typedef struct soc_cancun_cch_s {
    uint32 status;                  /* Loading status*/
    uint32 version;                 /* Loaded version */
    uint32 sdk_version;             /* Built SDK version */
    soc_cancun_file_t file;         /* CCH file entry */
    uint32* cch_table;              /* Entry of CCH hash table */
    uint64* pseudo_regs;            /* Pseudo registers in CCHBLK */
} soc_cancun_cch_t;

/*
 * CANCUN CEH control structure
 * Contains information of currently loaded CEH files.
 */
typedef struct soc_cancun_ceh_s {
    uint32 status;                  /* Loading status*/
    uint32 version;                 /* Loaded version */
    uint32 sdk_version;             /* Built SDK version */
    soc_cancun_file_t file;         /* CEH file entry */
    uint32* ceh_table;              /* Entry of CEH hash table */
} soc_cancun_ceh_t;

typedef struct soc_cancun_ceh_object_s {
    uint32 name_addr;
    uint32 width;
    uint32 num_fields;
} soc_cancun_ceh_object_t;
typedef struct soc_cancun_ceh_field_s {
    uint32 name_addr;
    uint32 offset;
    uint32 width;
    uint32 value;
} soc_cancun_ceh_field_t;
typedef struct soc_cancun_ceh_field_info_s {
    uint32 offset;
    uint32 width;
    uint32 value;
    uint32 flags;
} soc_cancun_ceh_field_info_t;

#define SOC_CANCUN_CEH_BLOCK_HASH_HEADER_LEN  9

/* point to the hash header table used by hash function*/
#define SOC_CANCUN_CEH_HASH_HDR_TBL(_p) \
        ((soc_cancun_hash_table_t *)((uint32 *)(_p) + 3))

/* point the string name table */
#define SOC_CANCUN_CEH_STR_NAME_TBL(_p) \
        (&SOC_CANCUN_CEH_HASH_HDR_TBL(_p)->table_entry)

/* start of object hash entry location table. Index is hash key */
#define SOC_CANCUN_CEH_OBJ_ENTRY_LOC_TBL(_p) ((uint32 *) \
        (&(SOC_CANCUN_CEH_HASH_HDR_TBL(_p)->table_entry)) \
         + ((SOC_CANCUN_CEH_HASH_HDR_TBL(_p)->table_entry) / 4))

/* start of object hash entry table. Each is pointed by entry location table */
#define SOC_CANCUN_CEH_OBJ_ENTRY_TBL(_p) \
        (SOC_CANCUN_CEH_OBJ_ENTRY_LOC_TBL(_p) + \
        (SOC_CANCUN_CEH_HASH_HDR_TBL(_p)->pd))

/*
 * CANCUN control structure
 * Contains information of currently loaded CANCUN files.
 */
typedef struct soc_cancun_s {
    uint32 unit;                /* Unit ID*/
    uint32 flags;               /* Control flags */
    uint32 version;             /* Packge release version */
    char default_path[CANCUN_FILENAME_SIZE]; /* default path */
    soc_cancun_cih_t* cih;      /* CIH control structure*/
    soc_cancun_cmh_t* cmh;      /* CMH control structure*/
    soc_cancun_cch_t* cch;      /* CCH control structure*/
    soc_flow_db_t*  flow_db;    /* FLOW DB control structure*/
    soc_cancun_ceh_t* ceh;      /* CEH control structure*/
} soc_cancun_t;

/*
 * CANCUN control flags
*/
/* Status flags */
#define SOC_CANCUN_FLAG_INITIALIZED         0x00000001
#define SOC_CANCUN_FLAG_CIH_LOADED          0x00000002
#define SOC_CANCUN_FLAG_CMH_LOADED          0x00000004
#define SOC_CANCUN_FLAG_CCH_LOADED          0x00000008
#define SOC_CANCUN_FLAG_CFH_LOADED          0x00000010
#define SOC_CANCUN_FLAG_CEH_LOADED          0x00000020

#define SOC_CANCUN_FLAG_VERSIONS_MATCH      0x00000100
/* Control flags */
#define SOC_CANCUN_FLAG_LOAD_DEFAULT_CIH    0x00010000
#define SOC_CANCUN_FLAG_LOAD_DEFAULT_CMH    0x00020000
#define SOC_CANCUN_FLAG_LOAD_DEFAULT_CCH    0x00040000
#define SOC_CANCUN_FLAG_LOAD_DEFAULT_CFH    0x00080000
#define SOC_CANCUN_FLAG_LOAD_DEFAULT_CEH    0x00100000


#define SOC_CANCUN_FLAG_DEBUG_MODE          0x01000000
#define SOC_CANCUN_FLAG_SKIP_VALIDITY       0x02000000
#define SOC_CANCUN_FLAG_SKIP_VERSION_MATCH  0x04000000
#define SOC_CANCUN_FLAG_LOAD_DEFAULT_ALL    (SOC_CANCUN_FLAG_LOAD_DEFAULT_CIH |\
                                             SOC_CANCUN_FLAG_LOAD_DEFAULT_CMH |\
                                             SOC_CANCUN_FLAG_LOAD_DEFAULT_CCH)

/*
 * CANCUN loading status
*/
#define SOC_CANCUN_LOAD_STATUS_NOT_LOADED   0x00000000
#define SOC_CANCUN_LOAD_STATUS_LOADED       0x00000001
#define SOC_CANCUN_LOAD_STATUS_IN_PROGRESS  0x00000002
#define SOC_CANCUN_LOAD_STATUS_FAILED       0x00000003

/*
 * CANCUN control flags from SOC property
*/
#define SOC_PROPERTY_CANCUN_LOAD_SKIP_CIH   0x00000001
#define SOC_PROPERTY_CANCUN_LOAD_SKIP_CMH   0x00000002
#define SOC_PROPERTY_CANCUN_LOAD_SKIP_CCH   0x00000004
#define SOC_PROPERTY_CANCUN_LOAD_SKIP_CFH   0x00000008
#define SOC_PROPERTY_CANCUN_LOAD_SKIP_CEH   0x00000010

#define SOC_PROPERTY_CANCUN_DEBUG_MODE      0x00000001
#define SOC_PROPERTY_CANCUN_FILE_VALIDITY   0x00000002
#define SOC_PROPERTY_CANCUN_VER_MATCH       0x00000004

/*
 * CANCUN File identifiers
 */
#define SOC_CANCUN_FILE_ID                  0x434E4355
#define SOC_CANCUN_FILE_ID_CIH              0x00434948
#define SOC_CANCUN_FILE_ID_CMH              0x00434D48
#define SOC_CANCUN_FILE_ID_CCH              0x00434348
#define SOC_CANCUN_FILE_ID_CFH              0x00434648
#define SOC_CANCUN_FILE_ID_CEH              0x00436548

/*
 * CANCUN File Header & Data Blob Size in Bytes
 */
#define SOC_CANCUN_FILE_HEADER_OFFSET       (32)
#define SOC_CANCUN_CIH_LENGTH_OFFSET        (8)
#define SOC_CANCUN_CIH_FLAG_OFFSET          (12)
#define SOC_CANCUN_CIH_PIO_DATA_BLOB_SIZE   (24)

/*
 * CANCUN package file blob field offset
 */
#define SOC_CANCUN_BLOB_ADDR_OFFSET     (0)
#define SOC_CANCUN_BLOB_OPCODE_OFFSET   (1)
#define SOC_CANCUN_BLOB_LEN_OFFSET      (2)
#define SOC_CANCUN_BLOB_FLAGS_OFFSET    (3)
#define SOC_CANCUN_BLOB_DATA_OFFSET     (4)

/*
 * CANCUN package file blob flags
 */
#define SOC_CANCUN_BLOB_FLAG_TCAM           0x00000008
#define SOC_CANCUN_BLOB_FLAG_MEM_ID_PRESENT (0x00000010)

#define SOC_CANCUN_BLOB_ADDR_MEM_ID_SHIFT    (17)
#define SOC_CANCUN_BLOB_ADDR_MEM_IDX_MASK    (0x1FFFF)

#define SOC_CANCUN_BLOB_FORMAT_MASK   (0x7)
#define SOC_CANCUN_BLOB_FORMAT_PIO    (0x0)
#define SOC_CANCUN_BLOB_FORMAT_DMA    (0x1)
#define SOC_CANCUN_BLOB_FORMAT_FIFO   (0x2)
#define SOC_CANCUN_BLOB_FORMAT_RSVD   (0x3)

/*
 * CCH pseudo register flags
 */
#define SOC_CANCUN_PSEUDO_REGS_FLAGS_VALID_READ (0x0001)

/*
 * Enumeration types used in CANCUN
 */
#define SOC_CANCUN_ENUM_TYPE_UNKNOWN    (0x0000)
#define SOC_CANCUN_ENUM_TYPE_APP        (0x0001)
#define SOC_CANCUN_ENUM_TYPE_MEM        (0x0002)
#define SOC_CANCUN_ENUM_TYPE_REG        (0x0003)
#define SOC_CANCUN_ENUM_TYPE_MEM_FIELD  (0x0004)
#define SOC_CANCUN_ENUM_TYPE_REG_FIELD  (0x0005)

/*
 * Register enumeration flag
 */
#define SOC_CANCUN_FLAG_REG_ENUM            0x40000000

/*
 * Misc. defines
 */
#define SOC_CANCUN_FIELD_LISTf          (-2)

#define SOC_CANCUN_VERSION_DEF_5_1_8        0x01050108
#define SOC_CANCUN_VERSION_DEF_5_2_1        0x01050201
#define SOC_CANCUN_VERSION_DEF_5_2_2        0x01050202
#define SOC_CANCUN_VERSION_DEF_5_2_3        0x01050203
#define SOC_CANCUN_VERSION_DEF_5_3_0        0x01050300
#define SOC_CANCUN_VERSION_DEF_5_3_2        0x01050302
#define SOC_CANCUN_VERSION_DEF_6_0_0        0x01060000
#define SOC_CANCUN_VERSION_DEF_6_1_3        0x01060103
#define SOC_CANCUN_VERSION_DEF_3_0_0        0x01030000
#define SOC_CANCUN_VERSION_DEF_4_0_0        0x01040000
#define SOC_CANCUN_VERSION_DEF_4_1_2        0x01040102
#define SOC_CANCUN_VERSION(_m,_mn,_s) (((_m) << 16) | ((_mn) << 8) | \
			(_s) | (1 << 24))
#define SOC_CANCUN_VERSION_5_2_UNDER_SERIES(_ver) \
        ((_ver) <= SOC_CANCUN_VERSION(5,1,0xff))

typedef struct soc_cancun_udf_stage_info_s {
    uint32 size;
    soc_mem_t policy_mem;
    uint32 start_pos;
    uint32 unavail_contbmap;
    uint32 flags;
    uint32 hfe_prof_ptr_arr_len;
    uint32 *hfe_profile_ptr;
} soc_cancun_udf_stage_info_t;

/*****************************************************************************************/
/*                              profile mem                                              */
/*****************************************************************************************/


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

/*****************************************************************************************/
/*                              L2                                                       */
/*****************************************************************************************/
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
/*                            Counters                                                   */
/*****************************************************************************************/
//PARITY<38> = x
//ECCP<38:32> = x
//ECC<37:32> = x
//COUNT<31:0> = x
//64-bits counter:ipipe0
#define RIPD4_64r                             0x84000500   // Receive IPv4 L3 Discard Packet Counter.
#define RIPC4_64r                             0x84000600   // Receive IPv4 L3 Unicast Frame Counter.
#define RIPHE4_64r                            0x84000700   // Receive IPv4 L3 IP Header Error Packet Counter.
#define IMRP4_64r                             0x84000800   // Receive IPv4 L3 Routed Multicast Packets.
#define RIPD6_64r                             0x84000900   // Receive IPv6 L3 Discard Packet Counter.
#define RIPC6_64r                             0x84000a00   // Receive IPv6 L3 Unicast Frame Counter.
#define RIPHE6_64r                            0x84000b00   // Receive IPv6 L3 IP Header Error Packet Counter.
#define IMRP6_64r                             0x84000c00   // Receive IPv6 L3 Routed Multicast Packets.
#define RTUNr                                 0x84000d00   // Receive Good Tunnel terminated packets Counter.
#define RUC_64r                               0x84000e00   // Receive Unicast Counter
#define RPORTD_64r                            0x84000f00   // PortInDiscard Counter.
#define RPARITYDr                             0x84001000   // PortInDiscard Counter.

#define ICTRL_64r                             0x84001700   // Receive HiGig Packet with Control Opcode Counter.
#define IBCAST_64r                            0x84001800   // Receive HiGig Packet with Broadcast Opcode Counter.
#define IIPMC_64r                             0x84001a00   // Receive HiGig Packet with IPMC Opcode Counter.
#define ING_NIV_RX_FRAMES_ERROR_DROP_64r      0x84002600   // Number of frames dropped due to VNTAG/ETAG format errors.
#define ING_NIV_RX_FRAMES_FORWARDING_DROP_64r 0x84002700   // Number of frames dropped due to an NIV/PE forwarding errors.
#define ING_NIV_RX_FRAMES_VLAN_TAGGED_64r     0x84002800   // Number of VLAN tagged packets received from this port.
                                                           // (This is added to make sure VNTAG/ETAG related MAC Changes can be avoided if necessary)
#define ING_ECN_COUNTER_64r                   0x84002900   // Count packets with ECN field matching a specific profile on tunnel decap

//64-bits counter: epipe0 
#define EGR_ECN_COUNTER_64r                   0x28001000   // Number of packets with ECN error to this port. Index by device port num
#define TPCE_64r                              0x28001100   // Egress Purge and Cell Error Drop Counter. Set to 64 bits to be consistent with other counters.

//COUNT<31:0> = x
//Blocks: gxport0 gxport1 gxport2 gxport3 gxport4 gxport5 (6 copies)
#define GRFCSr                                0x00000000   // Receive FCS Error Frame Counter
#define GRXCFr                                0x00000100   // Receive Control Frame Counter
#define GRXPFr                                0x00000200   // Receive Pause Frame Counter
#define GRXUOr                                0x00000300   // Receive Unsupported Opcode Frame Counter
#define GRALNr                                0x00000400   // Receive Alignment Error Frame Counter
#define GRCDEr                                0x00000600   // Receive Code Error Counter
#define GRFCRr                                0x00000700   // Receive False Carrier Counter
#define GROVRr                                0x00000800   // Receive Oversized Frame Counter
#define GRJBRr                                0x00000900   // Receive Jabber Frame Counter
#define GRMTUEr                               0x00000a00   // Receive MTU Check Error Frame Counter
#define GRRPKTr                               0x00000b00   // Receive RUNT Frame Counter
#define GRUNDr                                0x00000c00   // Receive Undersize Frame Counter
#define GRFRGr                                0x00000d00   // Receive Fragment Counter
#define GRRBYTr                               0x00000e00   // Receive Runt Byte Counter
#define GRMCAr                                0x00000f00   // Receive Multicast Frame Counter
#define GRBCAr                                0x00001000   // Receive Broadcast Frame Counter
#define GR64r                                 0x00001100   // Receive 64 Byte Frame Counter
#define GR127r                                0x00001200   // Receive 65 to 127 Byte Frame Counter
#define GR255r                                0x00001300   // Receive 128 to 255 Byte Frame Counter
#define GR511r                                0x00001400   // Receive 256 to 511 Byte Frame Counter
#define GR1023r                               0x00001500   // Receive 512 to 1023 Byte Frame Counter
#define GR1518r                               0x00001600   // Receive 1024 to 1518 Byte Frame Counter
#define GRMGVr                                0x00001700   // Receive 1519 to 1522 Byte Good VLAN Frame Counter
#define GR2047r                               0x00001800   // Receive 1519 to 2047 Byte Frame Counter
#define GR4095r                               0x00001900   // Receive 2048 to 4095 Byte Frame Counter
#define GR9216r                               0x00001a00   // Receive 4096 to 9216 Byte Frame Counter
#define GRPKTr                                0x00001b00   // Receive frame Counter
#define GRBYTr                                0x00001c00   // Receive Byte Counter
#define GRUCr                                 0x00001d00   // Receive Unicast Frame Counter
#define GRPOKr                                0x00001e00   // Receive Good Frame Counter
#define GRPFCr                                0x00001f00   // Received Packets PFC Counter

#define GTXPFr                                0x00002000   // Transmit Pause Control Frame Counter
#define GTJBRr                                0x00002100   // Transmit Jabber Counter
#define GTFCSr                                0x00002200   // Transmit FCS Error Counter
#define GTXCFr                                0x00002300   // Transmit Control Frame Counter
#define GTOVRr                                0x00002400   // Transmit Oversize Packet Counter
#define GTDFRr                                0x00002500   // Transmit Single Deferral Frame Counter
#define GTEDFr                                0x00002600   // Transmit Multiple Deferral Frame Counter
#define GTSCLr                                0x00002700   // Transmit Single Collision Frame Counter
#define GTMCLr                                0x00002800   // Transmit Multiple Collision Frame Counter
#define GTLCLr                                0x00002900   // Transmit Late Collision Frame Counter
#define GTXCLr                                0x00002a00   // Transmit Excessive Collision Frame Counter
#define GTFRGr                                0x00002b00   // Transmit Fragment Counter
#define GTNCLr                                0x00002c00   // Transmit Total Collision Counter
#define GTMCAr                                0x00002d00   // Transmit Multicast Frame Counter
#define GTBCAr                                0x00002e00   // Transmit Broadcast Frame Counter
#define GT64r                                 0x00002f00   // Transmit 64 Byte Frame Counter
#define GT127r                                0x00003000   // Transmit 65 to 127 Byte Frame Counter
#define GT255r                                0x00003100   // Transmit 128 to 255 Byte Frame Counter
#define GT511r                                0x00003200   // Transmit 256 to 511 Byte Frame Counter
#define GT1023r                               0x00003300   // Transmit 512 to 1023 Byte Frame Counter
#define GT1518r                               0x00003400   // Transmit 1024 to 1518 Byte Frame Counter 
#define GTMGVr                                0x00003500   // Transmit 1519 to 1522 Byte Good VLAN Frame Counter
#define GT2047r                               0x00003600   // Transmit 1519 to 2047 Byte Frame Counter
#define GT4095r                               0x00003700   // Transmit 2048 to 4095 Byte Frame Counter
#define GT9216r                               0x00003800   // Transmit 4096 to 9216 Byte Frame Counter
#define GTPKTr                                0x00003900   // Transmit frame Counter
#define GTBYTr                                0x00003a00   // Transmit Byte Counter
#define GTUCr                                 0x00003b00   // Transmit Unicast Frame Counter
#define GTPOKr                                0x00003c00   // Transmit Good Frame Counter
#define GTPFCr                                0x00003d00   // Transmitted PFC packet Counter

/*****************************************************************************************/
/*                              switchdev                                                */
/*****************************************************************************************/

typedef struct _bcmsw_switch_s {
    struct net_device *dev; //bcm0
    soc_info_t *si;
    struct bcmsw_switchdev *swdev;

    //soc cancun info
    soc_cancun_t *soc_cancun_info;

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

#endif
