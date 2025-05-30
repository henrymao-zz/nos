#ifndef _BCM_SWITCHDEV_CANCUN_H_
#define _BCM_SWITCHDEV_CANCUN_H_

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
    uint32_t file_identifier;     /* Cancun File identifier */
    uint32_t file_type;           /* Cancun File Type */
    uint32_t chip_rev_id;         /* Chip & rev id */
    uint32_t version;             /* Cancun Version */
    uint32_t file_length;         /* File length in 32-bit words */
    uint32_t num_data_blobs;      /* Number of data blobs */
    uint32_t sdk_version;         /* SDK version that builds this file */
    uint32_t rsvd2;
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
    uint32_t status;                          /* Loading status*/
} soc_cancun_file_t;

/* FLOW DB Control Structure */
typedef struct soc_flow_db_s {
    uint32_t status;
    uint32_t version;
    soc_cancun_file_t file;   /* Flow DB file entry */
    soc_flow_db_flow_map_t *flow_map;
    char *str_tbl;
} soc_flow_db_t;

/*
* CANCUN CIH control structure
* Contains information of currently loaded CIH files.
*/
typedef struct soc_cancun_cih_s {
    uint32_t status;            /* Loading status*/
    uint32_t version;           /* Loaded version */
    soc_cancun_file_t  file;  /* CIH file entry */
} soc_cancun_cih_t;

/*
* CANCUN hash table structure
* Generalized hash table for CANCUN support
*
*     hash_key = (pa * field_enum + pb * mem_reg_enum + pc) % pd
*/
typedef struct soc_cancun_hash_table_s {
    uint32_t pa;                  /* Hash function parameter pA */
    uint32_t pb;                  /* Hash function parameter pB */
    uint32_t pc;                  /* Hash function parameter pC */
    uint32_t pd;                  /* Hash function parameter pD */
    uint32_t entry_num;           /* Total entry number in hash table */
    uint32_t table_size;          /* Word count size for hash table */
    uint32_t table_entry;         /* Hash table entry point */
} soc_cancun_hash_table_t;

/*
* CANCUN generic hash entry header
*/
typedef struct soc_cancun_entry_hdr_s {
    uint32_t entry_size;
    uint32_t format;
    uint32_t src_mem;
    uint32_t src_field;
    int src_app;
} soc_cancun_entry_hdr_t;

/*
* CANCUN coverage list structure
*/
typedef struct soc_cancun_list_s {
    uint32_t entry_size;
    uint32_t format;
    uint32_t src_mem;
    uint32_t src_field;
    int src_app;
    uint32_t member_num;
    uint32_t members;
} soc_cancun_list_t;

/*
* CANCUN destination entry structure for reporting use
*/
typedef struct soc_cancun_dest_entry_s {
    uint32_t dest_index_num;      /* Destination memory table index number*/
    uint32_t dest_field_num;      /* Destination field number in this memory*/
    uint32_t dest_mems[CANCUN_DEST_MEM_NUM_MAX];      /* Destination memories */
    uint32_t dest_fields[CANCUN_DEST_FIELD_NUM_MAX];  /* Destination fields */
    uint32_t dest_values[CANCUN_DEST_FIELD_NUM_MAX];  /* Destination values */
} soc_cancun_dest_entry_t;

/*
* CANCUN CMH map structure
*/
typedef struct soc_cancun_cmh_map_s {
    uint32_t entry_size;          /* Entry size in word*/
    uint32_t format;              /* CANCUN CMH format */
    uint32_t src_mem;             /* Source memory or register enumeration */
    uint32_t src_field;           /* Source field enumeration */
    int src_app;                /* Source application enumeration */
    uint32_t src_value_num;       /* Number of valid values */
    uint32_t dest_mem_num;        /* Number of destination memories or registers */
    uint32_t dest_map_entry;      /* Entry point for destination mapping */
} soc_cancun_cmh_map_t;

/*
* CANCUN CMH control structure
* Contains information of currently loaded CMH files.
*/
typedef struct soc_cancun_cmh_s {
    uint32_t status;                  /* Loading status*/
    uint32_t version;                 /* Loaded version */
    uint32_t sdk_version;             /* Built SDK version */
    soc_cancun_file_t file;         /* CMH file entry */
    uint32_t* cmh_table;              /* Entry of CMH hash table */
    uint32_t* cmh_list;               /* Entry of CMH list of mem/field */
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
    uint32_t entry_size;          /* Entry size in word*/
    uint32_t format;              /* CANCUN CCH format */
    uint32_t src_mem;             /* Source memory or register enumeration */
    uint32_t src_field;           /* Source field enumeration */
    int src_app;                /* Source application enumeration */
    uint32_t src_value_num;       /* Number of valid values */
    uint32_t dest_mem_num;        /* Number of destination memories or registers */
    uint32_t dest_map_entry;      /* Entry point for destination mapping */
} soc_cancun_cch_map_t;

/*
* CANCUN CCH control structure
* Contains information of currently loaded CMH files.
*/
typedef struct soc_cancun_cch_s {
    uint32_t status;                  /* Loading status*/
    uint32_t version;                 /* Loaded version */
    uint32_t sdk_version;             /* Built SDK version */
    soc_cancun_file_t file;         /* CCH file entry */
    uint32_t* cch_table;              /* Entry of CCH hash table */
    uint64_t* pseudo_regs;            /* Pseudo registers in CCHBLK */
} soc_cancun_cch_t;

/*
* CANCUN CEH control structure
* Contains information of currently loaded CEH files.
*/
typedef struct soc_cancun_ceh_s {
    uint32_t status;                  /* Loading status*/
    uint32_t version;                 /* Loaded version */
    uint32_t sdk_version;             /* Built SDK version */
    soc_cancun_file_t file;         /* CEH file entry */
    uint32_t* ceh_table;              /* Entry of CEH hash table */
} soc_cancun_ceh_t;

typedef struct soc_cancun_ceh_object_s {
    uint32_t name_addr;
    uint32_t width;
    uint32_t num_fields;
} soc_cancun_ceh_object_t;
typedef struct soc_cancun_ceh_field_s {
    uint32_t name_addr;
    uint32_t offset;
    uint32_t width;
    uint32_t value;
} soc_cancun_ceh_field_t;
typedef struct soc_cancun_ceh_field_info_s {
    uint32_t offset;
    uint32_t width;
    uint32_t value;
    uint32_t flags;
} soc_cancun_ceh_field_info_t;

#define SOC_CANCUN_CEH_BLOCK_HASH_HEADER_LEN  9

/* point to the hash header table used by hash function*/
#define SOC_CANCUN_CEH_HASH_HDR_TBL(_p) \
    ((soc_cancun_hash_table_t *)((uint32_t *)(_p) + 3))

/* point the string name table */
#define SOC_CANCUN_CEH_STR_NAME_TBL(_p) \
    (&SOC_CANCUN_CEH_HASH_HDR_TBL(_p)->table_entry)

/* start of object hash entry location table. Index is hash key */
#define SOC_CANCUN_CEH_OBJ_ENTRY_LOC_TBL(_p) ((uint32_t *) \
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
    uint32_t unit;                /* Unit ID*/
    uint32_t flags;               /* Control flags */
    uint32_t version;             /* Packge release version */
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
    uint32_t size;
    soc_mem_t policy_mem;
    uint32_t start_pos;
    uint32_t unavail_contbmap;
    uint32_t flags;
    uint32_t hfe_prof_ptr_arr_len;
    uint32_t *hfe_profile_ptr;
} soc_cancun_udf_stage_info_t;



#endif
