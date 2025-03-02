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
/*                              SCHAN                                                    */
/*****************************************************************************************/
/* CMIC Configuration Register */
#define CMIC_CONFIG                     0x0000010C

#define CC_RD_BRST_EN                   0x00000001 /* enb PCI read bursts */
#define CC_WR_BRST_EN                   0x00000002 /* enb PCI write bursts */
#define CC_BE_CHECK_EN                  0x00000004 /* Big endian chk PCI Wr */
#define CC_MSTR_Q_MAX_EN                0x00000008 /* Queue 4 PCI mastr reqs */
#define CC_LINK_STAT_EN                 0x00000010 /* enb linkstat autoupd */
#define CC_RESET_CPS                    0x00000020 /* Drive CPS bus reset */
#define CC_ACT_LOW_INT                  0x00000040 /* INTA# is active low */
#define CC_SCHAN_ABORT                  0x00000080 /* abort S-Channel opern */
#define CC_STACK_ARCH_EN                0x00000100 /* (see databook) */
#define CC_UNTAG_EN                     0x00000200 /* (see databook) */
#define CC_LE_DMA_EN                    0x00000400 /* Little endian DMA fmt */
#define CC_I2C_EN                       0x00000800 /* enb CPU access to I2C */
#define CC_LINK_SCAN_GIG                0x00001000 /* Also scan gig ports */
#define CC_DMA_GARBAGE_COLL_EN          0x00008000 /* Collect 'purge' pkts */

#define CC_ALN_OPN_EN                   0x00002000 /* Unaligned DMA enable */
#define CC_PCI_RST_ON_INIT_EN           0x00010000
#define CC_DIS_TIME_STAMP_CTR           0x00020000
#define CC_SG_OPN_EN                    0x00040000 /* DMA S/G enable */
#define CC_RLD_OPN_EN                   0x00080000 /* DMA RLD enable */
#define CC_DIS_RLD_STAT_UPDATE          0x00100000
#define CC_STOP_AUTO_SCAN_ON_LCHG       0x00200000 /* LS stop in intr */
#define CC_ABORT_STAT_DMA               0x00400000
#define CC_ATO_SCAN_FROM_ID_ZERO        0x00800000 /* Scan from PHYID=0 */
#define CC_COS_QUALIFIED_DMA_RX_EN      0x01000000
#define CC_ABORT_TBL_DMA                0x02000000 /* Abort Table DMA op */
#define CC_EXT_MDIO_MSTR_DIS            0x04000000 /* Disable MDIO on ATE */
#define CC_EXTENDED_DCB_ENABLE          0x08000000 /* type7 dcb (5695) */
#define CC_INT_PHY_CLAUSE_45            0x08000000 /* MIIM 45 vs. 22 (5673) */

/* Endian selection register */
#define CMIC_ENDIAN_SELECT              0x00000174

#define ES_BIG_ENDIAN_PIO               0x01000001
#define ES_BIG_ENDIAN_DMA_PACKET        0x02000002
#define ES_BIG_ENDIAN_DMA_OTHER         0x04000004
#define ES_BIG_ENDIAN_CTR64_WORDS       0x08000008      /* Lynx only */
#define EN_BIG_ENDIAN_EB2_2B_SEL        0x20000020      /* Raptor/Raven EB slave only */

/* S-Channel Control Register */
#define CMIC_SCHAN_CTRL                 0x00000050

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

/*
 * SCHAN_CONTROL: CMICX 
 */
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

#define CMIC_SCHAN_WORDS                20

/* number of words to use when allocating space */
/** Iproc17 sbus data increased to 120 bytes */
#define CMIC_SCHAN_WORDS_ALLOC 32

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
#define TOP_SOFT_RESET_REGr 96047


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
