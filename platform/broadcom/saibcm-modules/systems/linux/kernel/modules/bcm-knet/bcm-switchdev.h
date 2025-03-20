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
/*                              SOC                                                      */
/*****************************************************************************************/
#define COMPILER_64_HI(src)     ((uint32) ((src) >> 32))
#define COMPILER_64_LO(src)     ((uint32) (src))
#define COMPILER_64_ZERO(dst)       ((dst) = 0)
#define COMPILER_64_IS_ZERO(src)    ((src) == 0)
                                       

#define COMPILER_64_SET(dst, src_hi, src_lo)                \
    ((dst) = (((uint64) ((uint32)(src_hi))) << 32) | ((uint64) ((uint32)(src_lo))))


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


#define    MEM_BLOCK_ALL                -1

#define SOC_WARM_BOOT(unit)             0 

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


#define TOP_SOFT_RESET_REGr            0x02000100
#define CHIP_CONFIGr                   0x2020000
#define TOP_MISC_GENERIC_CONTROLr      0x2008600
#define EGR_PORT_BUFFER_SFT_RESET_0r   0x2b130000
#define IDB_SER_CONTROL_64r            0x2280000

#define IS_TDM_CONFIG_PIPE0r           0x6040100
#define IS_OPP_SCHED_CFG_PIPE0r        0x6040500

#define EGR_HW_RESET_CONTROL_0r        0x3000000
#define EGR_HW_RESET_CONTROL_1r        0x3010000
#define EGR_HW_RESET_CONTROL_1_PIPE0r  0x3010000

#define ING_HW_RESET_CONTROL_1r        0x2230000
#define ING_HW_RESET_CONTROL_2r        0x2240000
#define ING_HW_RESET_CONTROL_2_PIPE0r  0x2240000

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

MEM_ENTRY(lport_tab_entry_t, 64);


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
/*                              switchdev                                                */
/*****************************************************************************************/

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
	struct bcmsw_switch *sw;
	struct list_head bridge_list;
	bool bridge_8021q_exists;
	struct notifier_block swdev_nb_blk;
	struct notifier_block swdev_nb;
};

struct bcmsw_switch {
    struct net_device *dev; //bcm0
    soc_info_t *si;
    struct bcmsw_switchdev *swdev;

    //soc cancun info
    soc_cancun_t *soc_cancun_info;
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
