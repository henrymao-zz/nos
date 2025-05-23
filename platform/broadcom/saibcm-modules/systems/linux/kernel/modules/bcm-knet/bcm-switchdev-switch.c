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
#include "bcm-switchdev-extphy.h"
#include "bcm-switchdev-merlin16.h"
#include "bcm-switchdev-stats.h"
#include "bcm-switchdev.h"


const bcm_mac_t _mac_spanning_tree =
      {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};

const bcm_mac_t _mac_all_routers =
      {0x01, 0x00, 0x5e, 0x00, 0x00, 0x02};

const bcm_mac_t _mac_all_zeroes =
      {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const bcm_mac_t _mac_all_ones =
      {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};


/*****************************************************************************************/
/*                              SOC                                                      */
/*****************************************************************************************/
/* Environment switch */
#define _PCID_TEST 0

/* Local defines */
#define BYTES_PER_UINT32    (sizeof(uint32))

char *_shr_errmsg[] = _SHR_ERRMSG_INIT;

static const uint32_t empty_entry[SOC_MAX_MEM_WORDS] = {0};

mem_info_t mem_list[] = {
    {8590      ,IP_PARSER0_HME_STAGE_TCAM_ONLY_0m           ,SCHAN_BLK_IPIPE      ,0x4cc00000      ,51},
    {8588      ,IP_PARSER0_HME_STAGE_TCAM_DATA_ONLY_0m      ,SCHAN_BLK_IPIPE      ,0x4cb80000      ,25},
    {8586      ,IP_PARSER0_HFE_POLICY_TABLE_0m              ,SCHAN_BLK_IPIPE      ,0x4cb00000      ,41},
    {8593      ,IP_PARSER0_HVE_SCC_PROFILE_0m               ,SCHAN_BLK_IPIPE      ,0x4ccc0000      ,7},
    {8594      ,IP_PARSER0_HVE_SCC_PROFILE_1m               ,SCHAN_BLK_IPIPE      ,0x4cd00000      ,7},
    {8601      ,IP_PARSER0_HVE_SCF_PROFILE_0m               ,SCHAN_BLK_IPIPE      ,0x4cec0000      ,13},
    {8591      ,IP_PARSER0_HVE_RC_PROFILE_0m                ,SCHAN_BLK_IPIPE      ,0x4cc40000      ,15},
    {8592      ,IP_PARSER0_HVE_RC_PROFILE_1m                ,SCHAN_BLK_IPIPE      ,0x4cc80000      ,15},
    {15878     ,SPECIAL_LABEL_CONTROLm                      ,SCHAN_BLK_IPIPE      ,0x50340000      ,10},
    {8632      ,IP_PARSER1_HME_STAGE_TCAM_ONLY_0m           ,SCHAN_BLK_IPIPE      ,0x50f80000      ,51},
    {8622      ,IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_0m      ,SCHAN_BLK_IPIPE      ,0x50d00000      ,20},
    {8612      ,IP_PARSER1_HFE_POLICY_TABLE_0m              ,SCHAN_BLK_IPIPE      ,0x50a80000      ,27},
    {8633      ,IP_PARSER1_HME_STAGE_TCAM_ONLY_1m           ,SCHAN_BLK_IPIPE      ,0x50fc0000      ,51},
    {8623      ,IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_1m      ,SCHAN_BLK_IPIPE      ,0x50d40000      ,20},
    {8613      ,IP_PARSER1_HFE_POLICY_TABLE_1m              ,SCHAN_BLK_IPIPE      ,0x50ac0000      ,27},
    {8629      ,IP_PARSER1_HME_STAGE_TCAM_NARROW_ONLY_2m    ,SCHAN_BLK_IPIPE      ,0x50ec0000      ,26},
    {8624      ,IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_2m      ,SCHAN_BLK_IPIPE      ,0x50d80000      ,20},
    {8614      ,IP_PARSER1_HFE_POLICY_TABLE_2m              ,SCHAN_BLK_IPIPE      ,0x50b00000      ,27},
    {8635      ,IP_PARSER1_HME_STAGE_TCAM_ONLY_3m           ,SCHAN_BLK_IPIPE      ,0x51040000      ,51},
    {8625      ,IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_3m      ,SCHAN_BLK_IPIPE      ,0x50dc0000      ,20},
    {8615      ,IP_PARSER1_HFE_POLICY_TABLE_3m              ,SCHAN_BLK_IPIPE      ,0x50b40000      ,27},
    {8636      ,IP_PARSER1_HME_STAGE_TCAM_ONLY_4m           ,SCHAN_BLK_IPIPE      ,0x51080000      ,51},
    {8626      ,IP_PARSER1_HME_STAGE_TCAM_DATA_ONLY_4m      ,SCHAN_BLK_IPIPE      ,0x50e00000      ,20},
    {8616      ,IP_PARSER1_HFE_POLICY_TABLE_4m              ,SCHAN_BLK_IPIPE      ,0x50b80000      ,27},
    {8654      ,IP_PARSER1_MICE_TCAM_0m                     ,SCHAN_BLK_IPIPE      ,0x51400000      ,31},
    {8655      ,IP_PARSER1_MICE_TCAM_1m                     ,SCHAN_BLK_IPIPE      ,0x51440000      ,31},
    {8639      ,IP_PARSER1_HVE_SCC_PROFILE_0m               ,SCHAN_BLK_IPIPE      ,0x51140000      ,7},
    {8640      ,IP_PARSER1_HVE_SCC_PROFILE_1m               ,SCHAN_BLK_IPIPE      ,0x51180000      ,7},
    {8641      ,IP_PARSER1_HVE_SCC_PROFILE_2m               ,SCHAN_BLK_IPIPE      ,0x511c0000      ,7},
    {8642      ,IP_PARSER1_HVE_SCC_PROFILE_3m               ,SCHAN_BLK_IPIPE      ,0x51200000      ,7},
    {8643      ,IP_PARSER1_HVE_SCC_PROFILE_4m               ,SCHAN_BLK_IPIPE      ,0x51240000      ,7},
    {8644      ,IP_PARSER1_HVE_SCC_PROFILE_5m               ,SCHAN_BLK_IPIPE      ,0x51280000      ,7},
    {8647      ,IP_PARSER1_HVE_SCF_PROFILE_0m               ,SCHAN_BLK_IPIPE      ,0x51340000      ,13},
    {8637      ,IP_PARSER1_HVE_RC_PROFILE_0m                ,SCHAN_BLK_IPIPE      ,0x510c0000      ,15},
    {8690      ,IP_PARSER2_HME_STAGE_TCAM_ONLY_0m           ,SCHAN_BLK_IPIPE      ,0x5cf80000      ,51},
    {8680      ,IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_0m      ,SCHAN_BLK_IPIPE      ,0x5cd00000      ,20},
    {8670      ,IP_PARSER2_HFE_POLICY_TABLE_0m              ,SCHAN_BLK_IPIPE      ,0x5ca80000      ,27},
    {8691      ,IP_PARSER2_HME_STAGE_TCAM_ONLY_1m           ,SCHAN_BLK_IPIPE      ,0x5cfc0000      ,51},
    {8681      ,IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_1m      ,SCHAN_BLK_IPIPE      ,0x5cd40000      ,20},
    {8671      ,IP_PARSER2_HFE_POLICY_TABLE_1m              ,SCHAN_BLK_IPIPE      ,0x5cac0000      ,27},
    {8687      ,IP_PARSER2_HME_STAGE_TCAM_NARROW_ONLY_2m    ,SCHAN_BLK_IPIPE      ,0x5cec0000      ,26},
    {8682      ,IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_2m      ,SCHAN_BLK_IPIPE      ,0x5cd80000      ,20},
    {8672      ,IP_PARSER2_HFE_POLICY_TABLE_2m              ,SCHAN_BLK_IPIPE      ,0x5cb00000      ,27},
    {8688      ,IP_PARSER2_HME_STAGE_TCAM_NARROW_ONLY_3m    ,SCHAN_BLK_IPIPE      ,0x5cf00000      ,26},
    {8683      ,IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_3m      ,SCHAN_BLK_IPIPE      ,0x5cdc0000      ,20},
    {8673      ,IP_PARSER2_HFE_POLICY_TABLE_3m              ,SCHAN_BLK_IPIPE      ,0x5cb40000      ,27},
    {8694      ,IP_PARSER2_HME_STAGE_TCAM_ONLY_4m           ,SCHAN_BLK_IPIPE      ,0x5d080000      ,51},
    {8684      ,IP_PARSER2_HME_STAGE_TCAM_DATA_ONLY_4m      ,SCHAN_BLK_IPIPE      ,0x5ce00000      ,20},
    {8674      ,IP_PARSER2_HFE_POLICY_TABLE_4m              ,SCHAN_BLK_IPIPE      ,0x5cb80000      ,27},
    {8712      ,IP_PARSER2_MICE_TCAM_0m                     ,SCHAN_BLK_IPIPE      ,0x5d400000      ,31},
    {8713      ,IP_PARSER2_MICE_TCAM_1m                     ,SCHAN_BLK_IPIPE      ,0x5d440000      ,31},
    {8697      ,IP_PARSER2_HVE_SCC_PROFILE_0m               ,SCHAN_BLK_IPIPE      ,0x5d140000      ,7},
    {8698      ,IP_PARSER2_HVE_SCC_PROFILE_1m               ,SCHAN_BLK_IPIPE      ,0x5d180000      ,7},
    {8705      ,IP_PARSER2_HVE_SCF_PROFILE_0m               ,SCHAN_BLK_IPIPE      ,0x5d340000      ,13},
    {8695      ,IP_PARSER2_HVE_RC_PROFILE_0m                ,SCHAN_BLK_IPIPE      ,0x5d0c0000      ,15},
    {8696      ,IP_PARSER2_HVE_RC_PROFILE_1m                ,SCHAN_BLK_IPIPE      ,0x5d100000      ,15},
    {8581      ,IP_PARSER0_HFE_CONT_PROFILE_TABLE_7m        ,SCHAN_BLK_IPIPE      ,0x4c9c0000      ,1},
    {8579      ,IP_PARSER0_HFE_CONT_PROFILE_TABLE_5m        ,SCHAN_BLK_IPIPE      ,0x4c940000      ,1},
    {8578      ,IP_PARSER0_HFE_CONT_PROFILE_TABLE_4m        ,SCHAN_BLK_IPIPE      ,0x4c900000      ,1},
    {8602      ,IP_PARSER1_HFE_CONT_PROFILE_TABLE_0m        ,SCHAN_BLK_IPIPE      ,0x50800000      ,2},
    {8607      ,IP_PARSER1_HFE_CONT_PROFILE_TABLE_5m        ,SCHAN_BLK_IPIPE      ,0x50940000      ,2},
    {8609      ,IP_PARSER1_HFE_CONT_PROFILE_TABLE_7m        ,SCHAN_BLK_IPIPE      ,0x509c0000      ,2},
    {8611      ,IP_PARSER1_HFE_CONT_PROFILE_TABLE_9m        ,SCHAN_BLK_IPIPE      ,0x50a40000      ,2},
    {8665      ,IP_PARSER2_HFE_CONT_PROFILE_TABLE_5m        ,SCHAN_BLK_IPIPE      ,0x5c940000      ,2},
    {8667      ,IP_PARSER2_HFE_CONT_PROFILE_TABLE_7m        ,SCHAN_BLK_IPIPE      ,0x5c9c0000      ,2},
    {8669      ,IP_PARSER2_HFE_CONT_PROFILE_TABLE_9m        ,SCHAN_BLK_IPIPE      ,0x5ca40000      ,2},
    {8617      ,IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_0m      ,SCHAN_BLK_IPIPE      ,0x50bc0000      ,16},
    {8618      ,IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_1m      ,SCHAN_BLK_IPIPE      ,0x50c00000      ,16},
    {8619      ,IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_2m      ,SCHAN_BLK_IPIPE      ,0x50c40000      ,16},
    {8620      ,IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_3m      ,SCHAN_BLK_IPIPE      ,0x50c80000      ,16},
    {8621      ,IP_PARSER1_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_4m      ,SCHAN_BLK_IPIPE      ,0x50cc0000      ,16},
    {8675      ,IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_0m      ,SCHAN_BLK_IPIPE      ,0x5cbc0000      ,16},
    {8676      ,IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_1m      ,SCHAN_BLK_IPIPE      ,0x5cc00000      ,16},
    {8677      ,IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_2m      ,SCHAN_BLK_IPIPE      ,0x5cc40000      ,16},
    {8678      ,IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_3m      ,SCHAN_BLK_IPIPE      ,0x5cc80000      ,16},
    {8679      ,IP_PARSER2_HME_STAGE_KEY_VALID_BYTES_CHECK_TABLE_4m      ,SCHAN_BLK_IPIPE      ,0x5ccc0000      ,16},
    {15132     ,PKT_FLOW_SELECT_HVE_CONTROL_0m              ,SCHAN_BLK_IPIPE      ,0x4c400000      ,14},
    {15137     ,PKT_FLOW_SELECT_TCAM_0m                     ,SCHAN_BLK_IPIPE      ,0x4c100000      ,31},
    {15118     ,PKT_FLOW_CTRL_PROFILE_0m                    ,SCHAN_BLK_IPIPE      ,0x4c200000      ,1},
    {15835     ,SGPP_CTRL_PROFILE_0m                        ,SCHAN_BLK_IPIPE      ,0x4c0c0000      ,11},
    {16028     ,SVP_CTRL_PROFILE_0m                         ,SCHAN_BLK_IPIPE      ,0x4c2c0000      ,9},
    {15119     ,PKT_FLOW_CTRL_STRENGTH_PROFILE_0m           ,SCHAN_BLK_IPIPE      ,0x4c1c0000      ,8},
    {15133     ,PKT_FLOW_SELECT_HVE_CONTROL_1m              ,SCHAN_BLK_IPIPE      ,0x50300000      ,14},
    {15138     ,PKT_FLOW_SELECT_TCAM_1m                     ,SCHAN_BLK_IPIPE      ,0x500c0000      ,42},
    {9061      ,KEYGEN_HDR_VALID_CHECK_PROFILE_1m           ,SCHAN_BLK_IPIPE      ,0x50180000      ,4},
    {4316      ,FIELD_BUS_MERGE_PROFILE_0m                  ,SCHAN_BLK_IPIPE      ,0x61280000      ,31},
    {16628     ,TUNNEL_ADAPT_1_LOGICAL_TBL_SEL_TCAMm        ,SCHAN_BLK_IPIPE      ,0x55b80000      ,32},
    {16626     ,TUNNEL_ADAPT_1_LOGICAL_TBL_SEL_SRAMm        ,SCHAN_BLK_IPIPE      ,0x55bc0000      ,15},
    {16644     ,TUNNEL_ADAPT_2_LOGICAL_TBL_SEL_TCAMm        ,SCHAN_BLK_IPIPE      ,0x55f00000      ,32},
    {16642     ,TUNNEL_ADAPT_2_LOGICAL_TBL_SEL_SRAMm        ,SCHAN_BLK_IPIPE      ,0x55f40000      ,15},
    {16664     ,TUNNEL_ADAPT_3_LOGICAL_TBL_SEL_TCAMm        ,SCHAN_BLK_IPIPE      ,0x55d00000      ,32},
    {16662     ,TUNNEL_ADAPT_3_LOGICAL_TBL_SEL_SRAMm        ,SCHAN_BLK_IPIPE      ,0x55d40000      ,15},
    {16680     ,TUNNEL_ADAPT_4_LOGICAL_TBL_SEL_TCAMm        ,SCHAN_BLK_IPIPE      ,0x61540000      ,32},
    {16678     ,TUNNEL_ADAPT_4_LOGICAL_TBL_SEL_SRAMm        ,SCHAN_BLK_IPIPE      ,0x61580000      ,15},
    {16620     ,TUNNEL_ADAPT_1_KEY_GEN_1_MUX_CTRLm          ,SCHAN_BLK_IPIPE      ,0x55c00000      ,15},
    {16618     ,TUNNEL_ADAPT_1_KEY_GEN_1_MASKm              ,SCHAN_BLK_IPIPE      ,0x55c40000      ,18},
    {16622     ,TUNNEL_ADAPT_1_KEY_GEN_2_MASKm              ,SCHAN_BLK_IPIPE      ,0x55cc0000      ,18},
    {16622     ,TUNNEL_ADAPT_1_KEY_GEN_2_MASKm              ,SCHAN_BLK_IPIPE      ,0x55cc0000      ,18},
    {16624     ,TUNNEL_ADAPT_1_KEY_GEN_2_MUX_CTRLm          ,SCHAN_BLK_IPIPE      ,0x55c80000      ,15},
    {16636     ,TUNNEL_ADAPT_2_KEY_GEN_1_MUX_CTRLm          ,SCHAN_BLK_IPIPE      ,0x55f80000      ,15},
    {16634     ,TUNNEL_ADAPT_2_KEY_GEN_1_MASKm              ,SCHAN_BLK_IPIPE      ,0x55fc0000      ,18},
    {16640     ,TUNNEL_ADAPT_2_KEY_GEN_2_MUX_CTRLm          ,SCHAN_BLK_IPIPE      ,0x56000000      ,15},
    {16638     ,TUNNEL_ADAPT_2_KEY_GEN_2_MASKm              ,SCHAN_BLK_IPIPE      ,0x56040000      ,18},
    {16652     ,TUNNEL_ADAPT_3_KEY_GEN_1_MUX_CTRLm          ,SCHAN_BLK_IPIPE      ,0x55d80000      ,15},
    {16650     ,TUNNEL_ADAPT_3_KEY_GEN_1_MASKm              ,SCHAN_BLK_IPIPE      ,0x55dc0000      ,18},
    {16672     ,TUNNEL_ADAPT_4_KEY_GEN_1_MUX_CTRLm          ,SCHAN_BLK_IPIPE      ,0x615c0000      ,15},
    {16670     ,TUNNEL_ADAPT_4_KEY_GEN_1_MASKm              ,SCHAN_BLK_IPIPE      ,0x61600000      ,18},
    {15121     ,PKT_FLOW_EXTRACTION_CTRL_ID_STRENGTH_PROFILEm      ,SCHAN_BLK_IPIPE      ,0x55240000      ,4},
    {7617      ,ING_MPLS_INHERIT_TABLEm                     ,SCHAN_BLK_IPIPE      ,0x70940000      ,3},
    {15836     ,SGPP_CTRL_PROFILE_1m                        ,SCHAN_BLK_IPIPE      ,0x58140000      ,14},
    {9292      ,L3_IIF_CTRL_PROFILE_1m                      ,SCHAN_BLK_IPIPE      ,0x583c0000      ,8},
    {16029     ,SVP_CTRL_PROFILE_1m                         ,SCHAN_BLK_IPIPE      ,0x58340000      ,8},
    {14187     ,MPLS_PARSER2_CONTEXT_PROFILEm               ,SCHAN_BLK_IPIPE      ,0x58200000      ,7},
    {15120     ,PKT_FLOW_CTRL_STRENGTH_PROFILE_1m           ,SCHAN_BLK_IPIPE      ,0x58500000      ,12},
    {16031     ,SVP_PROFILEm                                ,SCHAN_BLK_IPIPE      ,0x64500000      ,7},
    {15134     ,PKT_FLOW_SELECT_HVE_CONTROL_2m              ,SCHAN_BLK_IPIPE      ,0x64700000      ,14},
    {15139     ,PKT_FLOW_SELECT_TCAM_2m                     ,SCHAN_BLK_IPIPE      ,0x641c0000      ,72},
    {4315      ,FIELD_BUS_MERGE_PROFILEm                    ,SCHAN_BLK_IPIPE      ,0x68680000      ,32},
    {15153     ,PKT_FLOW_VSAN_STRENGTH_PROFILE_1m           ,SCHAN_BLK_IPIPE      ,0x58000000      ,2},
    {9100      ,L2_DESTINATION_STRENGTH_PROFILEm            ,SCHAN_BLK_IPIPE      ,0x70180000      ,5},
    {15135     ,PKT_FLOW_SELECT_MY_STATION_PROFILE_1m       ,SCHAN_BLK_IPIPE      ,0x581c0000      ,2},
    {15136     ,PKT_FLOW_SELECT_MY_STATION_PROFILE_2m       ,SCHAN_BLK_IPIPE      ,0x70200000      ,2},
    {4435      ,FORWARDING_1_LOGICAL_TBL_SEL_TCAMm          ,SCHAN_BLK_IPIPE      ,0x69800000      ,38},
    {4433      ,FORWARDING_1_LOGICAL_TBL_SEL_SRAMm          ,SCHAN_BLK_IPIPE      ,0x69840000      ,15},
    {4427      ,FORWARDING_1_KEY_GEN_1_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69880000      ,15},
    {4425      ,FORWARDING_1_KEY_GEN_1_MASKm                ,SCHAN_BLK_IPIPE      ,0x698c0000      ,18},
    {4423      ,FORWARDING_1_ACTION_PROFILEm                ,SCHAN_BLK_IPIPE      ,0x69d00000      ,3},
    {9111      ,L2_ENTRY_KEY_ATTRIBUTESm                    ,SCHAN_BLK_IPIPE      ,0x693c0000      ,3},
    {4467      ,FORWARDING_2_LOGICAL_TBL_SEL_TCAMm          ,SCHAN_BLK_IPIPE      ,0x69480000      ,38},
    {4465      ,FORWARDING_2_LOGICAL_TBL_SEL_SRAMm          ,SCHAN_BLK_IPIPE      ,0x694c0000      ,15},
    {4443      ,FORWARDING_2_KEY_GEN_1_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69500000      ,15},
    {4441      ,FORWARDING_2_KEY_GEN_1_MASKm                ,SCHAN_BLK_IPIPE      ,0x69540000      ,18},
    {4447      ,FORWARDING_2_KEY_GEN_2_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69580000      ,15},
    {4445      ,FORWARDING_2_KEY_GEN_2_MASKm                ,SCHAN_BLK_IPIPE      ,0x695c0000      ,18},
    {4451      ,FORWARDING_2_KEY_GEN_3_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69600000      ,15},
    {4449      ,FORWARDING_2_KEY_GEN_3_MASKm                ,SCHAN_BLK_IPIPE      ,0x69640000      ,18},
    {4455      ,FORWARDING_2_KEY_GEN_4_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69680000      ,15},
    {4453      ,FORWARDING_2_KEY_GEN_4_MASKm                ,SCHAN_BLK_IPIPE      ,0x696c0000      ,18},
    {4459      ,FORWARDING_2_KEY_GEN_5_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69700000      ,15},
    {4457      ,FORWARDING_2_KEY_GEN_5_MASKm                ,SCHAN_BLK_IPIPE      ,0x69740000      ,18},
    {4439      ,FORWARDING_2_ACTION_PROFILEm                ,SCHAN_BLK_IPIPE      ,0x69d40000      ,3},
    {9260      ,L3_ENTRY_KEY_ATTRIBUTESm                    ,SCHAN_BLK_IPIPE      ,0x691c0000      ,3},
    {4499      ,FORWARDING_3_LOGICAL_TBL_SEL_TCAMm          ,SCHAN_BLK_IPIPE      ,0x69980000      ,38},
    {4497      ,FORWARDING_3_LOGICAL_TBL_SEL_SRAMm          ,SCHAN_BLK_IPIPE      ,0x699c0000      ,15},
    {4475      ,FORWARDING_3_KEY_GEN_1_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69a00000      ,15},
    {4473      ,FORWARDING_3_KEY_GEN_1_MASKm                ,SCHAN_BLK_IPIPE      ,0x69a40000      ,18},
    {4479      ,FORWARDING_3_KEY_GEN_2_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69a80000      ,15},
    {4477      ,FORWARDING_3_KEY_GEN_2_MASKm                ,SCHAN_BLK_IPIPE      ,0x69ac0000      ,18},
    {4483      ,FORWARDING_3_KEY_GEN_3_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69b00000      ,15},
    {4481      ,FORWARDING_3_KEY_GEN_3_MASKm                ,SCHAN_BLK_IPIPE      ,0x69b40000      ,18},
    {4487      ,FORWARDING_3_KEY_GEN_4_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69b80000      ,15},
    {4485      ,FORWARDING_3_KEY_GEN_4_MASKm                ,SCHAN_BLK_IPIPE      ,0x69bc0000      ,18},
    {4491      ,FORWARDING_3_KEY_GEN_5_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69c00000      ,15},
    {4489      ,FORWARDING_3_KEY_GEN_5_MASKm                ,SCHAN_BLK_IPIPE      ,0x69c40000      ,18},
    {4495      ,FORWARDING_3_KEY_GEN_6_MUX_CTRLm            ,SCHAN_BLK_IPIPE      ,0x69c80000      ,15},
    {4493      ,FORWARDING_3_KEY_GEN_6_MASKm                ,SCHAN_BLK_IPIPE      ,0x69cc0000      ,18},
    {4471      ,FORWARDING_3_ACTION_PROFILEm                ,SCHAN_BLK_IPIPE      ,0x69d80000      ,3},
    {7708      ,ING_VP_VLAN_MEMBERSHIP_KEY_ATTRIBUTESm      ,SCHAN_BLK_IPIPE      ,0x64ac0000      ,3},
    {15149     ,PKT_FLOW_VLAN_STRENGTH_PROFILE_1_Am         ,SCHAN_BLK_IPIPE      ,0x55000000      ,7},
    {15150     ,PKT_FLOW_VLAN_STRENGTH_PROFILE_1_Bm         ,SCHAN_BLK_IPIPE      ,0x61100000      ,3},
    {15151     ,PKT_FLOW_VRF_STRENGTH_PROFILE_1_Am          ,SCHAN_BLK_IPIPE      ,0x55100000      ,11},
    {15152     ,PKT_FLOW_VRF_STRENGTH_PROFILE_1_Bm          ,SCHAN_BLK_IPIPE      ,0x64180000      ,5},
    {15122     ,PKT_FLOW_L3_IIF_STRENGTH_PROFILE_1_Am       ,SCHAN_BLK_IPIPE      ,0x550c0000      ,11},
    {15123     ,PKT_FLOW_L3_IIF_STRENGTH_PROFILE_1_Bm       ,SCHAN_BLK_IPIPE      ,0x64140000      ,5},
    {15147     ,PKT_FLOW_VFI_STRENGTH_PROFILE_1_Am          ,SCHAN_BLK_IPIPE      ,0x55080000      ,11},
    {15148     ,PKT_FLOW_VFI_STRENGTH_PROFILE_1_Bm          ,SCHAN_BLK_IPIPE      ,0x640c0000      ,3},
    {15146     ,PKT_FLOW_SVP_STRENGTH_PROFILE_1_Am          ,SCHAN_BLK_IPIPE      ,0x55040000      ,12},
    {15124     ,PKT_FLOW_OPAQUE_1_STRENGTH_PROFILE_1_Am     ,SCHAN_BLK_IPIPE      ,0x55140000      ,5},
    {15125     ,PKT_FLOW_OPAQUE_1_STRENGTH_PROFILE_1_Bm     ,SCHAN_BLK_IPIPE      ,0x70980000      ,1},
    {15127     ,PKT_FLOW_OPAQUE_2_STRENGTH_PROFILE_1_Bm     ,SCHAN_BLK_IPIPE      ,0x709c0000      ,1},
    {15129     ,PKT_FLOW_OPAQUE_3_STRENGTH_PROFILE_1_Bm     ,SCHAN_BLK_IPIPE      ,0x70a00000      ,1},
    {15128     ,PKT_FLOW_OPAQUE_3_STRENGTH_PROFILE_1_Am     ,SCHAN_BLK_IPIPE      ,0x551c0000      ,5},
    {15130     ,PKT_FLOW_OPAQUE_4_STRENGTH_PROFILE_1_Am     ,SCHAN_BLK_IPIPE      ,0x55200000      ,5},
    {4424      ,FORWARDING_1_FIXED_KEY_TABLE_ATTRSm         ,SCHAN_BLK_IPIPE      ,0x682c0000      ,2},
    {4438      ,FORWARDING_1_POLICY_STRENGTH_PROFILEm       ,SCHAN_BLK_IPIPE      ,0x68300000      ,11},
    {4437      ,FORWARDING_1_MISS_POLICYm                   ,SCHAN_BLK_IPIPE      ,0x68340000      ,14},
    {4440      ,FORWARDING_2_FIXED_KEY_TABLE_ATTRSm         ,SCHAN_BLK_IPIPE      ,0x68200000      ,2},
    {4470      ,FORWARDING_2_POLICY_STRENGTH_PROFILEm       ,SCHAN_BLK_IPIPE      ,0x68240000      ,16},
    {4472      ,FORWARDING_3_FIXED_KEY_TABLE_ATTRSm         ,SCHAN_BLK_IPIPE      ,0x68140000      ,2},
    {4502      ,FORWARDING_3_POLICY_STRENGTH_PROFILEm       ,SCHAN_BLK_IPIPE      ,0x68180000      ,16},
    {16790     ,VLAN_XLATE_1_KEY_ATTRIBUTESm                ,SCHAN_BLK_IPIPE      ,0x558c0000      ,3},
    {16617     ,TUNNEL_ADAPT_1_FIXED_KEY_TABLE_ATTRSm       ,SCHAN_BLK_IPIPE      ,0x55280000      ,2},
    {16631     ,TUNNEL_ADAPT_1_POLICY_STRENGTH_PROFILEm     ,SCHAN_BLK_IPIPE      ,0x55580000      ,45},
    {16616     ,TUNNEL_ADAPT_1_ACTION_PROFILEm              ,SCHAN_BLK_IPIPE      ,0x56100000      ,4},
    {16630     ,TUNNEL_ADAPT_1_MISS_POLICYm                 ,SCHAN_BLK_IPIPE      ,0x552c0000      ,27},
    {16801     ,VLAN_XLATE_2_KEY_ATTRIBUTESm                ,SCHAN_BLK_IPIPE      ,0x612c0000      ,3},
    {14162     ,MPLS_ENTRY_KEY_ATTRIBUTESm                  ,SCHAN_BLK_IPIPE      ,0x55b00000      ,3},
    {16633     ,TUNNEL_ADAPT_2_FIXED_KEY_TABLE_ATTRSm       ,SCHAN_BLK_IPIPE      ,0x55300000      ,2},
    {16647     ,TUNNEL_ADAPT_2_POLICY_STRENGTH_PROFILEm     ,SCHAN_BLK_IPIPE      ,0x555c0000      ,45},
    {16646     ,TUNNEL_ADAPT_2_MISS_POLICYm                 ,SCHAN_BLK_IPIPE      ,0x55340000      ,27},
    {16632     ,TUNNEL_ADAPT_2_ACTION_PROFILEm              ,SCHAN_BLK_IPIPE      ,0x56080000      ,4},
    {16649     ,TUNNEL_ADAPT_3_FIXED_KEY_TABLE_ATTRSm       ,SCHAN_BLK_IPIPE      ,0x55380000      ,2},
    {16667     ,TUNNEL_ADAPT_3_POLICY_STRENGTH_PROFILEm     ,SCHAN_BLK_IPIPE      ,0x55600000      ,45},
    {16648     ,TUNNEL_ADAPT_3_ACTION_PROFILEm              ,SCHAN_BLK_IPIPE      ,0x560c0000      ,4},
    {16669     ,TUNNEL_ADAPT_4_FIXED_KEY_TABLE_ATTRSm       ,SCHAN_BLK_IPIPE      ,0x61040000      ,2},
    {16683     ,TUNNEL_ADAPT_4_POLICY_STRENGTH_PROFILEm     ,SCHAN_BLK_IPIPE      ,0x61080000      ,45},
    {16682     ,TUNNEL_ADAPT_4_MISS_POLICYm                 ,SCHAN_BLK_IPIPE      ,0x610c0000      ,27},
    {16668     ,TUNNEL_ADAPT_4_ACTION_PROFILEm              ,SCHAN_BLK_IPIPE      ,0x616c0000      ,3},
    {15156     ,PKT_PROC_HDR_VALID_CHECK_PROFILE_1_VALUEm   ,SCHAN_BLK_IPIPE      ,0x58040000      ,51},
    {15155     ,PKT_PROC_HDR_VALID_CHECK_PROFILE_1_MASKm    ,SCHAN_BLK_IPIPE      ,0x58080000      ,51},
    {15154     ,PKT_PROC_HDR_VALID_CHECK_PROFILE_2m         ,SCHAN_BLK_IPIPE      ,0x70100000      ,66},
    {14151     ,MPLS_EDIT_CTRL_ID_MAPm                      ,SCHAN_BLK_IPIPE      ,0x70000000      ,1},
    {4410      ,FLEX_RTAG7_HASH_TCAMm                       ,SCHAN_BLK_IPIPE      ,0x684c0000      ,40},
    {4325      ,FLEX_BIN_HASH_PROFILE_0m                    ,SCHAN_BLK_IPIPE      ,0x68580000      ,41},
    {4326      ,FLEX_BIN_HASH_PROFILE_1m                    ,SCHAN_BLK_IPIPE      ,0x685c0000      ,41},
    {4327      ,FLEX_BIN_HASH_PROFILE_2m                    ,SCHAN_BLK_IPIPE      ,0x68600000      ,41},
    {4328      ,FLEX_BIN_HASH_PROFILE_3m                    ,SCHAN_BLK_IPIPE      ,0x68640000      ,41},
    {7609      ,ING_LOOPBACK_DROP_VECTOR_MASKm              ,SCHAN_BLK_IPIPE      ,0x80a80000      ,12},
    {2712      ,EGR_ZONE_0_MATCH_ID_ATTRIBUTES_TABLEm       ,SCHAN_BLK_EPIPE      ,0x06380000      ,10},
    {2992      ,EP_PARSER0_HFE_POLICY_TABLE_0m              ,SCHAN_BLK_EPIPE      ,0x04300000      ,41},
    {2987      ,EP_PARSER0_HFE_CONT_PROFILE_TABLE_7m        ,SCHAN_BLK_EPIPE      ,0x041c0000      ,1},
    {2985      ,EP_PARSER0_HFE_CONT_PROFILE_TABLE_5m        ,SCHAN_BLK_EPIPE      ,0x04140000      ,1},
    {2984      ,EP_PARSER0_HFE_CONT_PROFILE_TABLE_4m        ,SCHAN_BLK_EPIPE      ,0x04100000      ,1},
    {2982      ,EP_PARSER0_HFE_CONT_PROFILE_TABLE_2m        ,SCHAN_BLK_EPIPE      ,0x04080000      ,1},
    {2713      ,EGR_ZONE_0_VAR_LEN_HDR1_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x063c0000      ,4},
    {2714      ,EGR_ZONE_0_VAR_LEN_HDR2_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x06400000      ,4},
    {2727      ,EGR_ZONE_1_MATCH_ID_ATTRIBUTES_TABLEm       ,SCHAN_BLK_EPIPE      ,0x06640000      ,10},
    {3005      ,EP_PARSER1_HFE_POLICY_TABLE_0m              ,SCHAN_BLK_EPIPE      ,0x04b00000      ,32},
    {2266      ,EGR_FIELD_EXTRACTION_PROFILE_1_TCAMm        ,SCHAN_BLK_EPIPE      ,0x06740000      ,23},
    {2730      ,EGR_ZONE_1_VAR_LEN_HDR1_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x06440000      ,4},
    {2731      ,EGR_ZONE_1_VAR_LEN_HDR2_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x06480000      ,4},
    {2735      ,EGR_ZONE_2_MATCH_ID_ATTRIBUTES_TABLEm       ,SCHAN_BLK_EPIPE      ,0x06680000      ,10},
    {3006      ,EP_PARSER1_HFE_POLICY_TABLE_1m              ,SCHAN_BLK_EPIPE      ,0x04b40000      ,32},
    {2738      ,EGR_ZONE_2_VAR_LEN_HDR1_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x064c0000      ,4},
    {2739      ,EGR_ZONE_2_VAR_LEN_HDR2_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x06500000      ,4},
    {2269      ,EGR_FIELD_EXTRACTION_PROFILE_2_TCAMm        ,SCHAN_BLK_EPIPE      ,0x06800000      ,23},
    {2993      ,EP_PARSER1_HFE_CONT_PROFILE_TABLE_0m        ,SCHAN_BLK_EPIPE      ,0x04800000      ,2},
    {2996      ,EP_PARSER1_HFE_CONT_PROFILE_TABLE_3m        ,SCHAN_BLK_EPIPE      ,0x048c0000      ,2},
    {3000      ,EP_PARSER1_HFE_CONT_PROFILE_TABLE_7m        ,SCHAN_BLK_EPIPE      ,0x049c0000      ,2},
    {3004      ,EP_PARSER1_HFE_CONT_PROFILE_TABLE_11m       ,SCHAN_BLK_EPIPE      ,0x04ac0000      ,2},
    {2752      ,EGR_ZONE_3_MATCH_ID_ATTRIBUTES_TABLEm       ,SCHAN_BLK_EPIPE      ,0x066c0000      ,10},
    {3007      ,EP_PARSER1_HFE_POLICY_TABLE_2m              ,SCHAN_BLK_EPIPE      ,0x04b80000      ,32},
    {2755      ,EGR_ZONE_3_VAR_LEN_HDR1_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x06540000      ,4},
    {2756      ,EGR_ZONE_3_VAR_LEN_HDR2_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x06580000      ,4},
    {2760      ,EGR_ZONE_4_MATCH_ID_ATTRIBUTES_TABLEm       ,SCHAN_BLK_EPIPE      ,0x06700000      ,10},
    {3008      ,EP_PARSER1_HFE_POLICY_TABLE_3m              ,SCHAN_BLK_EPIPE      ,0x04bc0000      ,32},
    {2763      ,EGR_ZONE_4_VAR_LEN_HDR1_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x065c0000      ,4},
    {2764      ,EGR_ZONE_4_VAR_LEN_HDR2_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x06600000      ,4},
    {2272      ,EGR_FIELD_EXTRACTION_PROFILE_CONTROLm       ,SCHAN_BLK_EPIPE      ,0x068c0000      ,2},
    {2538      ,EGR_PKT_FLOW_SELECT_TCAMm                   ,SCHAN_BLK_EPIPE      ,0x0e780000      ,68},
    {2435      ,EGR_L2_TAG_INHERIT_ACTION_MASK_PROFILEm     ,SCHAN_BLK_EPIPE      ,0x09e40000      ,35},
    {2436      ,EGR_L2_TAG_INHERIT_PROFILEm                 ,SCHAN_BLK_EPIPE      ,0x09e80000      ,13},
    {2541      ,EGR_PKT_FLOW_ZONE_SELECT_PROFILEm           ,SCHAN_BLK_EPIPE      ,0x0e840000      ,29},
    {2211      ,EGR_ADAPT_1_LOGICAL_TBL_SEL_TCAMm           ,SCHAN_BLK_EPIPE      ,0x0a600000      ,32},
    {2209      ,EGR_ADAPT_1_LOGICAL_TBL_SEL_SRAMm           ,SCHAN_BLK_EPIPE      ,0x0a640000      ,15},
    {2203      ,EGR_ADAPT_1_KEY_GEN_1_MUX_CTRLm             ,SCHAN_BLK_EPIPE      ,0x0a680000      ,15},
    {2201      ,EGR_ADAPT_1_KEY_GEN_1_MASKm                 ,SCHAN_BLK_EPIPE      ,0x0a6c0000      ,18},
    {2207      ,EGR_ADAPT_1_KEY_GEN_2_MUX_CTRLm             ,SCHAN_BLK_EPIPE      ,0x0a700000      ,15},
    {2205      ,EGR_ADAPT_1_KEY_GEN_2_MASKm                 ,SCHAN_BLK_EPIPE      ,0x0a740000      ,18},
    {2227      ,EGR_ADAPT_2_LOGICAL_TBL_SEL_TCAMm           ,SCHAN_BLK_EPIPE      ,0x0a780000      ,32},
    {2225      ,EGR_ADAPT_2_LOGICAL_TBL_SEL_SRAMm           ,SCHAN_BLK_EPIPE      ,0x0a7c0000      ,15},
    {2219      ,EGR_ADAPT_2_KEY_GEN_1_MUX_CTRLm             ,SCHAN_BLK_EPIPE      ,0x0a800000      ,15},
    {2217      ,EGR_ADAPT_2_KEY_GEN_1_MASKm                 ,SCHAN_BLK_EPIPE      ,0x0a840000      ,18},
    {2223      ,EGR_ADAPT_2_KEY_GEN_2_MUX_CTRLm             ,SCHAN_BLK_EPIPE      ,0x0a880000      ,15},
    {2221      ,EGR_ADAPT_2_KEY_GEN_2_MASKm                 ,SCHAN_BLK_EPIPE      ,0x0a8c0000      ,18},
    {2446      ,EGR_L3_NEXT_HOP_EARLY_ACTION_PROFILEm       ,SCHAN_BLK_EPIPE      ,0x069c0000      ,1},
    {2445      ,EGR_L3_NEXT_HOP_ACTION_PROFILEm             ,SCHAN_BLK_EPIPE      ,0x0ed00000      ,10},
    {2440      ,EGR_L3_INTF_EARLY_ACTION_PROFILEm           ,SCHAN_BLK_EPIPE      ,0x06900000      ,1},
    {2439      ,EGR_L3_INTF_ACTION_PROFILEm                 ,SCHAN_BLK_EPIPE      ,0x0ee00000      ,9},
    {2248      ,EGR_DVP_ATTRIBUTE_EARLY_ACTION_PROFILEm     ,SCHAN_BLK_EPIPE      ,0x06940000      ,1},
    {2247      ,EGR_DVP_ATTRIBUTE_ACTION_PROFILEm           ,SCHAN_BLK_EPIPE      ,0x0ed80000      ,9},
    {2633      ,EGR_VC_AND_SWAP_EARLY_ACTION_PROFILEm       ,SCHAN_BLK_EPIPE      ,0x06980000      ,1},
    {2632      ,EGR_VC_AND_SWAP_ACTION_PROFILEm             ,SCHAN_BLK_EPIPE      ,0x0ed40000      ,9},
    {2431      ,EGR_IP_TUNNEL_ACTION_PROFILEm               ,SCHAN_BLK_EPIPE      ,0x0edc0000      ,10},
    {2447      ,EGR_LABEL_PRECEDENCE_PROFILE_TABLEm         ,SCHAN_BLK_EPIPE      ,0x0ebc0000      ,2},
    {2612      ,EGR_SPECIAL_PKT_HANDLING_PROFILEm           ,SCHAN_BLK_EPIPE      ,0x1a140000      ,6},
    {2616      ,EGR_STRENGTH_PROFILEm                       ,SCHAN_BLK_EPIPE      ,0x0e9c0000      ,38},
    {2691      ,EGR_VP_VLAN_MEMBERSHIP_KEY_ATTRIBUTESm      ,SCHAN_BLK_EPIPE      ,0x0a440000      ,3},
    {2658      ,EGR_VLAN_XLATE_1_KEY_ATTRIBUTESm            ,SCHAN_BLK_EPIPE      ,0x0a180000      ,3},
    {2668      ,EGR_VLAN_XLATE_2_KEY_ATTRIBUTESm            ,SCHAN_BLK_EPIPE      ,0x0a4c0000      ,3},
    {2200      ,EGR_ADAPT_1_FIXED_KEY_TABLE_ATTRSm          ,SCHAN_BLK_EPIPE      ,0x09740000      ,2},
    {2214      ,EGR_ADAPT_1_POLICY_STRENGTH_PROFILEm        ,SCHAN_BLK_EPIPE      ,0x09840000      ,6},
    {2213      ,EGR_ADAPT_1_MISS_POLICYm                    ,SCHAN_BLK_EPIPE      ,0x097c0000      ,27},
    {2199      ,EGR_ADAPT_1_ACTION_PROFILEm                 ,SCHAN_BLK_EPIPE      ,0x0a940000      ,6},
    {2216      ,EGR_ADAPT_2_FIXED_KEY_TABLE_ATTRSm          ,SCHAN_BLK_EPIPE      ,0x09780000      ,2},
    {2230      ,EGR_ADAPT_2_POLICY_STRENGTH_PROFILEm        ,SCHAN_BLK_EPIPE      ,0x09880000      ,6},
    {2229      ,EGR_ADAPT_2_MISS_POLICYm                    ,SCHAN_BLK_EPIPE      ,0x09800000      ,27},
    {2215      ,EGR_ADAPT_2_ACTION_PROFILEm                 ,SCHAN_BLK_EPIPE      ,0x0a900000      ,6},
    {2709      ,EGR_ZONE_0_EDITOR_CONTROL_TCAMm             ,SCHAN_BLK_EPIPE      ,0x098c0000      ,47},
    {2721      ,EGR_ZONE_1_EDITOR_CONTROL_TCAMm             ,SCHAN_BLK_EPIPE      ,0x09980000      ,43},
    {2732      ,EGR_ZONE_2_EDITOR_CONTROL_TCAMm             ,SCHAN_BLK_EPIPE      ,0x09b00000      ,49},
    {2746      ,EGR_ZONE_3_EDITOR_CONTROL_TCAMm             ,SCHAN_BLK_EPIPE      ,0x09bc0000      ,43},
    {2757      ,EGR_ZONE_4_EDITOR_CONTROL_TCAMm             ,SCHAN_BLK_EPIPE      ,0x09c40000      ,49},
    {2724      ,EGR_ZONE_1_L2_TAG_CONTROL_PROFILE_DELm      ,SCHAN_BLK_EPIPE      ,0x09ac0000      ,2},
    {2725      ,EGR_ZONE_1_L2_TAG_CONTROL_PROFILE_INSm      ,SCHAN_BLK_EPIPE      ,0x09a40000      ,7},
    {2749      ,EGR_ZONE_3_L2_TAG_CONTROL_PROFILE_DELm      ,SCHAN_BLK_EPIPE      ,0x09d80000      ,2},
    {2750      ,EGR_ZONE_3_L2_TAG_CONTROL_PROFILE_INSm      ,SCHAN_BLK_EPIPE      ,0x09d00000      ,7},
    {2235      ,EGR_DELETE_CONTROLm                         ,SCHAN_BLK_EPIPE      ,0x09e00000      ,3},
    {4330      ,FLEX_EDITOR_FIXED_HDR_L2_0_NEXT_PROTO_TABLEm              ,SCHAN_BLK_EPIPE      ,0x18b80000      ,2},
    {4332      ,FLEX_EDITOR_FIXED_HDR_L2_1_NEXT_PROTO_TABLEm              ,SCHAN_BLK_EPIPE      ,0x18c00000      ,2},
    {4334      ,FLEX_EDITOR_FIXED_HDR_L3_0_NEXT_PROTO_TABLEm              ,SCHAN_BLK_EPIPE      ,0x18c80000      ,2},
    {4338      ,FLEX_EDITOR_FIXED_HDR_TUNNEL_0_GRE_NEXT_PROTO_TABLEm      ,SCHAN_BLK_EPIPE      ,0x18d40000      ,2},
    {4346      ,FLEX_EDITOR_FLEX_HDR_0_NEXT_PROTO_TABLEm    ,SCHAN_BLK_EPIPE      ,0x18f40000      ,3},
    {4353      ,FLEX_EDITOR_FLEX_HDR_1_NEXT_PROTO_TABLEm    ,SCHAN_BLK_EPIPE      ,0x19100000      ,3},
    {2444      ,EGR_L3_NEXT_HOP_ACTION_MASK_PROFILEm        ,SCHAN_BLK_EPIPE      ,0x0e880000      ,10},
    {2438      ,EGR_L3_INTF_ACTION_MASK_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x0e8c0000      ,9},
    {2430      ,EGR_IP_TUNNEL_ACTION_MASK_PROFILEm          ,SCHAN_BLK_EPIPE      ,0x0e980000      ,10},
    {2246      ,EGR_DVP_ATTRIBUTE_ACTION_MASK_PROFILEm      ,SCHAN_BLK_EPIPE      ,0x0e900000      ,9},
    {2631      ,EGR_VC_AND_SWAP_ACTION_MASK_PROFILEm        ,SCHAN_BLK_EPIPE      ,0x0e940000      ,9},
    {4331      ,FLEX_EDITOR_FIXED_HDR_L2_0_PROFILE_TABLEm   ,SCHAN_BLK_EPIPE      ,0x18b40000      ,6},
    {4333      ,FLEX_EDITOR_FIXED_HDR_L2_1_PROFILE_TABLEm   ,SCHAN_BLK_EPIPE      ,0x18bc0000      ,6},
    {4335      ,FLEX_EDITOR_FIXED_HDR_L3_0_PROFILE_TABLEm   ,SCHAN_BLK_EPIPE      ,0x18c40000      ,2},
    {4339      ,FLEX_EDITOR_FIXED_HDR_TUNNEL_0_PROFILE_TABLEm             ,SCHAN_BLK_EPIPE      ,0x18cc0000      ,7},
    {4348      ,FLEX_EDITOR_FLEX_HDR_0_PROFILE_TABLEm                     ,SCHAN_BLK_EPIPE      ,0x18dc0000      ,4},
    {4342      ,FLEX_EDITOR_FLEX_HDR_0_16BIT_FS_PROFILE_TABLEm            ,SCHAN_BLK_EPIPE      ,0x18e00000      ,27},
    {4343      ,FLEX_EDITOR_FLEX_HDR_0_32BIT_FS_PROFILE_TABLEm            ,SCHAN_BLK_EPIPE      ,0x18e40000      ,39},
    {4345      ,FLEX_EDITOR_FLEX_HDR_0_HC_PROFILE_TABLEm    ,SCHAN_BLK_EPIPE      ,0x18ec0000      ,18},
    {4344      ,FLEX_EDITOR_FLEX_HDR_0_ALU_PROFILE_TABLEm   ,SCHAN_BLK_EPIPE      ,0x18e80000      ,17},
    {4347      ,FLEX_EDITOR_FLEX_HDR_0_OW_PROFILE_TABLEm    ,SCHAN_BLK_EPIPE      ,0x18f00000      ,20},
    {4355      ,FLEX_EDITOR_FLEX_HDR_1_PROFILE_TABLEm       ,SCHAN_BLK_EPIPE      ,0x18f80000      ,4},
    {4349      ,FLEX_EDITOR_FLEX_HDR_1_16BIT_FS_PROFILE_TABLEm            ,SCHAN_BLK_EPIPE      ,0x18fc0000      ,27},
    {4350      ,FLEX_EDITOR_FLEX_HDR_1_32BIT_FS_PROFILE_TABLEm            ,SCHAN_BLK_EPIPE      ,0x19000000      ,39},
    {4352      ,FLEX_EDITOR_FLEX_HDR_1_HC_PROFILE_TABLEm    ,SCHAN_BLK_EPIPE      ,0x19080000      ,18},
    {4351      ,FLEX_EDITOR_FLEX_HDR_1_ALU_PROFILE_TABLEm   ,SCHAN_BLK_EPIPE      ,0x19040000      ,17},
    {4362      ,FLEX_EDITOR_FLEX_HDR_2_PROFILE_TABLEm       ,SCHAN_BLK_EPIPE      ,0x19140000      ,4},
    {4356      ,FLEX_EDITOR_FLEX_HDR_2_16BIT_FS_PROFILE_TABLEm            ,SCHAN_BLK_EPIPE      ,0x19180000      ,27},
    {4357      ,FLEX_EDITOR_FLEX_HDR_2_32BIT_FS_PROFILE_TABLEm            ,SCHAN_BLK_EPIPE      ,0x191c0000      ,39},
    {4361      ,FLEX_EDITOR_FLEX_HDR_2_OW_PROFILE_TABLEm    ,SCHAN_BLK_EPIPE      ,0x19280000      ,20},
    {4359      ,FLEX_EDITOR_FLEX_HDR_2_HC_PROFILE_TABLEm    ,SCHAN_BLK_EPIPE      ,0x19240000      ,18},
    {4374      ,FLEX_EDITOR_RW_0_PROFILE_TABLEm             ,SCHAN_BLK_EPIPE      ,0x18780000      ,4},
    {4371      ,FLEX_EDITOR_RW_0_FS_PROFILE_TABLEm          ,SCHAN_BLK_EPIPE      ,0x187c0000      ,39},
    {4372      ,FLEX_EDITOR_RW_0_HC_PROFILE_TABLEm          ,SCHAN_BLK_EPIPE      ,0x18840000      ,36},
    {4370      ,FLEX_EDITOR_RW_0_ALU_PROFILE_TABLEm         ,SCHAN_BLK_EPIPE      ,0x18800000      ,17},
    {4366      ,FLEX_EDITOR_MHC_CRC_0_PROFILE_TABLEm        ,SCHAN_BLK_EPIPE      ,0x193c0000      ,2},
    {4379      ,FLEX_EDITOR_RW_1_PROFILE_TABLEm             ,SCHAN_BLK_EPIPE      ,0x188c0000      ,4},
    {4376      ,FLEX_EDITOR_RW_1_FS_PROFILE_TABLEm          ,SCHAN_BLK_EPIPE      ,0x18900000      ,39},
    {4377      ,FLEX_EDITOR_RW_1_HC_PROFILE_TABLEm          ,SCHAN_BLK_EPIPE      ,0x18980000      ,36},
    {4375      ,FLEX_EDITOR_RW_1_ALU_PROFILE_TABLEm         ,SCHAN_BLK_EPIPE      ,0x18940000      ,17},
    {4363      ,FLEX_EDITOR_MHC_CHECKSUM_0_PROFILE_TABLEm   ,SCHAN_BLK_EPIPE      ,0x19300000      ,8},
    {4364      ,FLEX_EDITOR_MHC_CHECKSUM_1_PROFILE_TABLEm   ,SCHAN_BLK_EPIPE      ,0x19340000      ,8},
    {4365      ,FLEX_EDITOR_MHC_CHECKSUM_2_PROFILE_TABLEm   ,SCHAN_BLK_EPIPE      ,0x19380000      ,8},
    {4337      ,FLEX_EDITOR_FIXED_HDR_SYS_0_PROFILE_TABLEm  ,SCHAN_BLK_EPIPE      ,0x18a00000      ,2},
    {4385      ,FLEX_EDITOR_ZONE_0_MATCH_ID_TABLEm          ,SCHAN_BLK_EPIPE      ,0x18000000      ,28},
    {4384      ,FLEX_EDITOR_ZONE_0_MATCH_ID_COMMAND_PROFILE_TABLEm      ,SCHAN_BLK_EPIPE      ,0x18140000      ,11},
    {4391      ,FLEX_EDITOR_ZONE_1_MATCH_ID_TABLEm          ,SCHAN_BLK_EPIPE      ,0x18040000      ,28},
    {4397      ,FLEX_EDITOR_ZONE_2_MATCH_ID_TABLEm          ,SCHAN_BLK_EPIPE      ,0x18080000      ,28},
    {4403      ,FLEX_EDITOR_ZONE_3_MATCH_ID_TABLEm          ,SCHAN_BLK_EPIPE      ,0x180c0000      ,28},
    {4409      ,FLEX_EDITOR_ZONE_4_MATCH_ID_TABLEm          ,SCHAN_BLK_EPIPE      ,0x18100000      ,28},
    {4408      ,FLEX_EDITOR_ZONE_4_MATCH_ID_COMMAND_PROFILE_TABLEm      ,SCHAN_BLK_EPIPE      ,0x18240000      ,11},
    {4380      ,FLEX_EDITOR_ZONE_0_EDIT_ID_DEL_TABLEm       ,SCHAN_BLK_EPIPE      ,0x18280000      ,8},
    {4386      ,FLEX_EDITOR_ZONE_1_EDIT_ID_DEL_TABLEm       ,SCHAN_BLK_EPIPE      ,0x182c0000      ,8},
    {4392      ,FLEX_EDITOR_ZONE_2_EDIT_ID_DEL_TABLEm       ,SCHAN_BLK_EPIPE      ,0x18300000      ,8},
    {4398      ,FLEX_EDITOR_ZONE_3_EDIT_ID_DEL_TABLEm       ,SCHAN_BLK_EPIPE      ,0x18340000      ,8},
    {4404      ,FLEX_EDITOR_ZONE_4_EDIT_ID_DEL_TABLEm       ,SCHAN_BLK_EPIPE      ,0x18380000      ,8},
    {4401      ,FLEX_EDITOR_ZONE_3_EDIT_ID_RW_TABLEm        ,SCHAN_BLK_EPIPE      ,0x18700000      ,8},
    {4407      ,FLEX_EDITOR_ZONE_4_EDIT_ID_RW_TABLEm        ,SCHAN_BLK_EPIPE      ,0x18740000      ,8},
    {4381      ,FLEX_EDITOR_ZONE_0_EDIT_ID_INS_1_TABLEm     ,SCHAN_BLK_EPIPE      ,0x183c0000      ,43},
    {4387      ,FLEX_EDITOR_ZONE_1_EDIT_ID_INS_1_TABLEm     ,SCHAN_BLK_EPIPE      ,0x18440000      ,43},
    {4393      ,FLEX_EDITOR_ZONE_2_EDIT_ID_INS_1_TABLEm     ,SCHAN_BLK_EPIPE      ,0x184c0000      ,43},
    {4394      ,FLEX_EDITOR_ZONE_2_EDIT_ID_INS_2_TABLEm     ,SCHAN_BLK_EPIPE      ,0x18500000      ,18},
    {4399      ,FLEX_EDITOR_ZONE_3_EDIT_ID_INS_1_TABLEm     ,SCHAN_BLK_EPIPE      ,0x18540000      ,43},
    {4405      ,FLEX_EDITOR_ZONE_4_EDIT_ID_INS_1_TABLEm     ,SCHAN_BLK_EPIPE      ,0x185c0000      ,43},
    {4406      ,FLEX_EDITOR_ZONE_4_EDIT_ID_INS_2_TABLEm     ,SCHAN_BLK_EPIPE      ,0x18600000      ,18},
    {2580      ,EGR_SEQUENCE_NUMBER_PROFILEm                ,SCHAN_BLK_EPIPE      ,0x0ecc0000      ,9},
    {2400      ,EGR_HDR_ID_EFPPARS_ENm                      ,SCHAN_BLK_EPIPE      ,0x1a0c0000      ,1},
    {2401      ,EGR_HDR_ID_EFPPARS_PROFILE_TABLEm           ,SCHAN_BLK_EPIPE      ,0x1a100000      ,9},
    {2273      ,EGR_FLEX_CONTAINER_UPDATE_PROFILE_0m        ,SCHAN_BLK_EPIPE      ,0x09000000      ,42},
    {2274      ,EGR_FLEX_CONTAINER_UPDATE_PROFILE_1m        ,SCHAN_BLK_EPIPE      ,0x09040000      ,42},
    {4336      ,FLEX_EDITOR_FIXED_HDR_LOOPBACK_0_PROFILE_TABLEm      ,SCHAN_BLK_EPIPE      ,0x18a40000      ,19},
    {15113     ,PHB_SELECT_TCAMm                            ,SCHAN_BLK_IPIPE      ,0x704c0000      ,59},
    {993       ,CNG_STRENGTH_PROFILEm                       ,SCHAN_BLK_IPIPE      ,0x70680000      ,10},
    {1884      ,DSCP_STRENGTH_PROFILEm                      ,SCHAN_BLK_IPIPE      ,0x70700000      ,10},
    {7734      ,INT_PRI_STRENGTH_PROFILEm                   ,SCHAN_BLK_IPIPE      ,0x706c0000      ,10},
    {15104     ,PHB2_STRENGTH_PROFILEm                      ,SCHAN_BLK_IPIPE      ,0x70740000      ,10},
    {2571      ,EGR_QOS_CTRL_TCAMm                          ,SCHAN_BLK_EPIPE      ,0x0ea00000      ,40},
    {2729      ,EGR_ZONE_1_QOS_STRENGTH_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x0eac0000      ,16},
    {2737      ,EGR_ZONE_2_QOS_STRENGTH_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x0eb00000      ,16},
    {2754      ,EGR_ZONE_3_QOS_STRENGTH_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x0eb40000      ,16},
    {2762      ,EGR_ZONE_4_QOS_STRENGTH_PROFILEm            ,SCHAN_BLK_EPIPE      ,0x0eb80000      ,16},
    {2719      ,EGR_ZONE_1_DOT1P_STRENGTH_PROFILE_1m        ,SCHAN_BLK_EPIPE      ,0x0ec00000      ,10},
    {2720      ,EGR_ZONE_1_DOT1P_STRENGTH_PROFILE_2m        ,SCHAN_BLK_EPIPE      ,0x093c0000      ,10},
    {2744      ,EGR_ZONE_3_DOT1P_STRENGTH_PROFILE_1m        ,SCHAN_BLK_EPIPE      ,0x0ec40000      ,10},
    {2745      ,EGR_ZONE_3_DOT1P_STRENGTH_PROFILE_2m        ,SCHAN_BLK_EPIPE      ,0x09400000      ,10},
    {2574      ,EGR_QOS_ECN_PROFILEm                        ,SCHAN_BLK_EPIPE      ,0x0e140000      ,2}
};

/*****************************************************************************************/
/*                            N3248TE hardware&ports info                                */
/*****************************************************************************************/
static const struct {
    int port;
    int phy_port;
    int pipe;
    int idb_port;
    int mmu_port;
    int port_type;
} hx5_anc_ports[] = {
    { 0,   0,   0, 70, 70, BCMSW_PORT_TYPE_CMIC      },  /* cpu port */
    { 70,  77,  0, 71, 71, BCMSW_PORT_TYPE_LBPORT    },  /* loopback port */
    { 71,  78,  0, 69, 69, BCMSW_PORT_TYPE_BROADSCAN },  /* FAE port */
};


typedef struct port_info_s {
    int port;
    int phy_port;
    int bandwidth;
    int ext_phy_addr;
    int primary_and_offset;
    int port_type;
    char name[KCOM_NETIF_NAME_MAX];
    int lanes[4];
} port_config_t;

static const port_config_t n3248te_ports[] = {
    { 1,   1,  1, 0x0, 0x0100, BCMSW_PORT_TYPE_GXPORT, "Ethernet0",  {1, -1, -1, -1}}, 
    { 2,   2,  1, 0x1, 0x0101, BCMSW_PORT_TYPE_GXPORT, "Ethernet1",  {2, -1, -1, -1}}, 
    { 3,   3,  1, 0x2, 0x0102, BCMSW_PORT_TYPE_GXPORT, "Ethernet2",  {3, -1, -1, -1}}, 
    { 4,   4,  1, 0x3, 0x0103, BCMSW_PORT_TYPE_GXPORT, "Ethernet3",  {4, -1, -1, -1}}, 
    { 5,   5,  1, 0x4, 0x0104, BCMSW_PORT_TYPE_GXPORT, "Ethernet4",  {5, -1, -1, -1}}, 
    { 6,   6,  1, 0x5, 0x0105, BCMSW_PORT_TYPE_GXPORT, "Ethernet5",  {6, -1, -1, -1}}, 
    { 7,   7,  1, 0x6, 0x0106, BCMSW_PORT_TYPE_GXPORT, "Ethernet6",  {7, -1, -1, -1}}, 
    { 8,   8,  1, 0x7, 0x0107, BCMSW_PORT_TYPE_GXPORT, "Ethernet7",  {8, -1, -1, -1}}, 
    { 9,   9,  1, 0x9, 0x0900, BCMSW_PORT_TYPE_GXPORT, "Ethernet8",  {9, -1, -1, -1}}, 
    { 10,  10, 1, 0xa, 0x0901, BCMSW_PORT_TYPE_GXPORT, "Ethernet9",  {10, -1, -1, -1}}, 
    { 11,  11, 1, 0xb, 0x0902, BCMSW_PORT_TYPE_GXPORT, "Ethernet10", {11, -1, -1, -1}}, 
    { 12,  12, 1, 0xc, 0x0903, BCMSW_PORT_TYPE_GXPORT, "Ethernet11", {12, -1, -1, -1}}, 
    { 13,  13, 1, 0xd, 0x0904, BCMSW_PORT_TYPE_GXPORT, "Ethernet12", {13, -1, -1, -1}},         
    { 14,  14, 1, 0xe, 0x0905, BCMSW_PORT_TYPE_GXPORT, "Ethernet13", {14, -1, -1, -1}}, 
    { 15,  15, 1, 0xf, 0x0906, BCMSW_PORT_TYPE_GXPORT, "Ethernet14", {15, -1, -1, -1}}, 
    { 16,  16, 1,  0x10, 0x0907, BCMSW_PORT_TYPE_GXPORT, "Ethernet15", {16, -1, -1, -1}}, 
    { 17,  17, 1,  0x12, 0x1200, BCMSW_PORT_TYPE_GXPORT, "Ethernet16", {17, -1, -1, -1}}, 
    { 18,  18, 1,  0x13, 0x1201, BCMSW_PORT_TYPE_GXPORT, "Ethernet17", {18, -1, -1, -1}}, 
    { 19,  19, 1,  0x14, 0x1202, BCMSW_PORT_TYPE_GXPORT, "Ethernet18", {19, -1, -1, -1}}, 
    { 20,  20, 1,  0x15, 0x1203, BCMSW_PORT_TYPE_GXPORT, "Ethernet19", {20, -1, -1, -1}}, 
    { 21,  21, 1,  0x16, 0x1204, BCMSW_PORT_TYPE_GXPORT, "Ethernet20", {21, -1, -1, -1}}, 
    { 22,  22, 1,  0x17, 0x1205, BCMSW_PORT_TYPE_GXPORT, "Ethernet21", {22, -1, -1, -1}}, 
    { 23,  23, 1,  0x18, 0x1206, BCMSW_PORT_TYPE_GXPORT, "Ethernet22", {23, -1, -1, -1}}, 
    { 24,  24, 1,  0x19, 0x1207, BCMSW_PORT_TYPE_GXPORT, "Ethernet23", {24, -1, -1, -1}}, 
    { 25,  25, 1,  0x20, 0x2000, BCMSW_PORT_TYPE_GXPORT, "Ethernet24", {25, -1, -1, -1}}, 
    { 26,  26, 1,  0x21, 0x2001, BCMSW_PORT_TYPE_GXPORT, "Ethernet25", {26, -1, -1, -1}}, 
    { 27,  27, 1,  0x22, 0x2002, BCMSW_PORT_TYPE_GXPORT, "Ethernet26", {27, -1, -1, -1}}, 
    { 28,  28, 1,  0x23, 0x2003, BCMSW_PORT_TYPE_GXPORT, "Ethernet27", {28, -1, -1, -1}}, 
    { 29,  29, 1,  0x24, 0x2004, BCMSW_PORT_TYPE_GXPORT, "Ethernet28", {29, -1, -1, -1}}, 
    { 30,  30, 1,  0x25, 0x2005, BCMSW_PORT_TYPE_GXPORT, "Ethernet29", {30, -1, -1, -1}}, 
    { 31,  31, 1,  0x26, 0x2006, BCMSW_PORT_TYPE_GXPORT, "Ethernet30", {31, -1, -1, -1}}, 
    { 32,  32, 1,  0x27, 0x2007, BCMSW_PORT_TYPE_GXPORT, "Ethernet31", {32, -1, -1, -1}}, 
    { 33,  33, 1,  0x29, 0x2900, BCMSW_PORT_TYPE_GXPORT, "Ethernet32", {33, -1, -1, -1}},        
    { 34,  34, 1,  0x2a, 0x2901, BCMSW_PORT_TYPE_GXPORT, "Ethernet33", {34, -1, -1, -1}}, 
    { 35,  35, 1,  0x2b, 0x2902, BCMSW_PORT_TYPE_GXPORT, "Ethernet34", {35, -1, -1, -1}}, 
    { 36,  36, 1,  0x2c, 0x2903, BCMSW_PORT_TYPE_GXPORT, "Ethernet35", {36, -1, -1, -1}}, 
    { 37,  37, 1,  0x2d, 0x2904, BCMSW_PORT_TYPE_GXPORT, "Ethernet36", {37, -1, -1, -1}}, 
    { 38,  38, 1,  0x2e, 0x2905, BCMSW_PORT_TYPE_GXPORT, "Ethernet37", {38, -1, -1, -1}}, 
    { 39,  39, 1,  0x2f, 0x2906, BCMSW_PORT_TYPE_GXPORT, "Ethernet38", {39, -1, -1, -1}}, 
    { 40,  40, 1,  0x30, 0x2907, BCMSW_PORT_TYPE_GXPORT, "Ethernet39", {40, -1, -1, -1}}, 
    { 41,  41, 1,  0x32, 0x3200, BCMSW_PORT_TYPE_GXPORT, "Ethernet40", {41, -1, -1, -1}}, 
    { 42,  42, 1,  0x33, 0x3201, BCMSW_PORT_TYPE_GXPORT, "Ethernet41", {42, -1, -1, -1}}, 
    { 43,  43, 1,  0x34, 0x3202, BCMSW_PORT_TYPE_GXPORT, "Ethernet42", {43, -1, -1, -1}}, 
    { 44,  44, 1,  0x35, 0x3203, BCMSW_PORT_TYPE_GXPORT, "Ethernet43", {44, -1, -1, -1}}, 
    { 45,  45, 1,  0x36, 0x3204, BCMSW_PORT_TYPE_GXPORT, "Ethernet44", {45, -1, -1, -1}}, 
    { 46,  46, 1,  0x37, 0x3205, BCMSW_PORT_TYPE_GXPORT, "Ethernet45", {46, -1, -1, -1}}, 
    { 47,  47, 1,  0x38, 0x3206, BCMSW_PORT_TYPE_GXPORT, "Ethernet46", {47, -1, -1, -1}}, 
    { 48,  48, 1,  0x39, 0x3207, BCMSW_PORT_TYPE_GXPORT, "Ethernet47", {48, -1, -1, -1}}, 
    { 49,  64, 10, 0x40, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet48", {64, -1, -1, -1}}, 
    { 50,  63, 10, 0x41, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet49", {63, -1, -1, -1}}, 
    { 51,  62, 10, 0x42, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet50", {62, -1, -1, -1}}, 
    { 52,  61, 10, 0x43, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet51", {61, -1, -1, -1}}, 
    { 53,  69, 100,  -1, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet52", {69, 70, 71, 72}}, 
    { 57,  73, 100,  -1, -1,     BCMSW_PORT_TYPE_XLPORT, "Ethernet56", {73, 74, 75, 76}},   
    { -1,  -1,  -1,  -1, -1,                         -1, "\0",         {-1, -1, -1, -1}}                                      
};




static void
_soc_hx5_mmu_idb_ports_assign(soc_info_t *si)
{
    int port, phy_port, mmu_port_os, mmu_port_lr;
    int mmu_port_old, phy_port_old;

    mmu_port_os = 0;

    /* PHY Ports: 61-64
     * MMU/IDB Ports: 0-3
     */
    for (phy_port = 61; phy_port <= 64; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 69-72
     * MMU/IDB Ports: 4-7
     */
    for (phy_port = 69; phy_port <= 72; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 65-68
     * MMU/IDB Ports: 8-11
     */
    for (phy_port = 65; phy_port <= 68; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 73-76
     * MMU/IDB Ports: 12-15
     */
    for (phy_port = 73; phy_port <= 76; phy_port++, mmu_port_os++) {
        if (phy_port >= SOC_MAX_NUM_PORTS) {
            /* coverity[dead_error_line] */
            continue;
        }
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    mmu_port_lr = mmu_port_os;

    
    for (phy_port = 1; phy_port <= 4; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 17-20
     * MMU/IDB Ports: 20-23
     */
    for (phy_port = 17; phy_port <= 20; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 33-36
     * MMU/IDB Ports: 24-27
     */
    for (phy_port = 33; phy_port <= 36; phy_port++, mmu_port_os++) {
        si->port_p2m_mapping[phy_port] = mmu_port_os;
        si->port_m2p_mapping[mmu_port_os] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 49-60
     * MMU/IDB Ports: 40-51
     */
    for (phy_port = 49; phy_port <= 60; phy_port++) {
        si->port_p2m_mapping[phy_port] = phy_port - 9;
        si->port_m2p_mapping[phy_port - 9] = phy_port;

        port = si->port_p2l_mapping[phy_port];
        if (port == -1) {
            continue;
        }
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
    }

    /* PHY Ports: 1-60
     * MMU/IDB Ports within 16-68
     *
     * When DL ports are in LR we use a running number
     * In Line-rate config PM4x10Q can be in GMII mode (upto 16 lanes)
     */
    for (phy_port = 1; phy_port <= 60; phy_port++) {
        port = si->port_p2l_mapping[phy_port];
        //if ((port == -1) || IS_OVERSUB_PORT(unit, port)) {
        if (port == -1) {
            continue;
        }

        mmu_port_old = si->port_p2m_mapping[phy_port];
        phy_port_old = si->port_m2p_mapping[mmu_port_lr];

        if (phy_port_old != -1) {
            si->port_p2m_mapping[phy_port_old] = -1;
        }
        if (mmu_port_old != -1) {
            si->port_m2p_mapping[mmu_port_old] = -1;
        }
        si->port_p2m_mapping[phy_port] = mmu_port_lr;
        si->port_l2i_mapping[port] = si->port_p2m_mapping[phy_port];
        si->port_m2p_mapping[mmu_port_lr] = phy_port;
        mmu_port_lr++;
    }

    return;
}


static void bcmsw_soc_info_init(soc_info_t *si)
{
    int index, port, pipe, phy_port, mmu_port;
    int num_port = HX5_NUM_PORT; 
    int num_phy_port = HX5_NUM_PHY_PORT;
    int num_mmu_port = HX5_NUM_MMU_PORT;

    memset(si,0,sizeof(si));

    si->bandwidth = 2048000;

    //reset array to default
    //HX5 has 72 ports(NUM_PORT=72)
    for (phy_port = 0; phy_port < num_phy_port; phy_port++) {
        si->port_p2l_mapping[phy_port] = -1;
        si->port_p2m_mapping[phy_port] = -1;
    }
    for (index = 0; index < num_port; index++) {
        si->port_l2p_mapping[index] = -1;
        si->port_l2i_mapping[index] = -1;
        si->port_speed_max[index] = -1;
        si->port_group[index] = -1;
        si->port_serdes[index] = -1;
        si->port_pipe[index] = -1;
        si->port_num_lanes[index] = -1;
        si->port_type[index] = -1;
        si->ports[index].valid = FALSE;
        si->ports[index].probed = FALSE;
        si->ports[index].ext_phy_addr = -1;
        si->ports[index].primary_and_offset = -1;
        si->ports[index].eth_port_type = 0;
    }
    for (mmu_port = 0; mmu_port < num_mmu_port; mmu_port++) {
        si->port_m2p_mapping[mmu_port] = -1;
    }    
/*
    SOC_PBMP_CLEAR(si->eq_pbm);
    SOC_PBMP_CLEAR(si->management_pbm);
    for (pipe = 0; pipe < NUM_PIPE(unit); pipe++) {
        SOC_PBMP_CLEAR(si->pipe_pbm[pipe]);
    };
    SOC_PBMP_CLEAR(si->oversub_pbm);
    SOC_PBMP_CLEAR(si->all.disabled_bitmap);
*/
    /* Populate the fixed mapped ports */
    for (index = 0; index < COUNTOF(hx5_anc_ports); index++) {
        port = hx5_anc_ports[index].port;
        phy_port = hx5_anc_ports[index].phy_port;
        pipe = hx5_anc_ports[index].pipe;;

        si->port_l2p_mapping[port] = phy_port;
        si->port_l2i_mapping[port] = hx5_anc_ports[index].idb_port;
        si->port_p2l_mapping[phy_port] = port;
        si->port_p2m_mapping[phy_port] = hx5_anc_ports[index].mmu_port;
        si->port_m2p_mapping[hx5_anc_ports[index].mmu_port] = phy_port;
        si->port_pipe[port] = pipe;
        si->port_type[port] = hx5_anc_ports[index].port_type;
        //SOC_PBMP_PORT_ADD(si->pipe_pbm[pipe], port);
        si->ports[port].valid = TRUE;
    }

    //FIXME hardcoded for N3248TE
    for (index = 0; index < COUNTOF(n3248te_ports); index++) {
        if (n3248te_ports[index].port == -1) {
            break;
        }
        port = n3248te_ports[index].port;
        phy_port = n3248te_ports[index].phy_port;
        /* Update soc_info */      
        si->port_l2p_mapping[port] = phy_port;
        si->port_p2l_mapping[phy_port] = port;
        si->port_pipe[port] = 0;
        si->port_speed_max[port] = n3248te_ports[index].bandwidth * 1000;
        si->port_init_speed[port] = n3248te_ports[index].bandwidth * 1000;
        if (phy_port <= 48) {
            si->port_serdes[port] = (phy_port - 1) / _HX5_PORTS_PER_PMQ_PBLK;
        } else {
            si->port_serdes[port] = ((phy_port - 1) / _HX5_PORTS_PER_PBLK) - 9;
        }

        if (n3248te_ports[index].bandwidth <= 10) {
            si->port_num_lanes[port] = 1;
        } else if (n3248te_ports[index].bandwidth <= 20) {
            si->port_num_lanes[port] = 2;
        } else {
            si->port_num_lanes[port] = 4;
        } 

        //port_type
        si->port_type[port] = n3248te_ports[index].port_type;
        si->ports[port].valid = TRUE;
        si->ports[port].ext_phy_addr = n3248te_ports[index].ext_phy_addr;
        si->ports[port].primary_and_offset = n3248te_ports[index].primary_and_offset;
        //FIXME, only GE port supported
        if (si->port_init_speed[port] == 1000) {
            si->ports[port].eth_port_type = ETH_GE_PORT;
        }

        if (n3248te_ports[index].port_type == BCMSW_PORT_TYPE_GXPORT) {
           sprintf(si->ports[port].name, "ge(%d)",index);
        } else if (n3248te_ports[index].port_type == BCMSW_PORT_TYPE_XLPORT) {
           sprintf(si->ports[port].name, "xe(%d)",index);
        } else if (n3248te_ports[index].port_type == BCMSW_PORT_TYPE_CLPORT) {
           sprintf(si->ports[port].name, "ce(%d)",index);
        }
    }
    si->cpu_hg_index = 72;
    //TODO flex port init

    _soc_hx5_mmu_idb_ports_assign(si);

#if 0
    printk("helix5 port config ------------------------\n");
    for (index =0; index< 79; index++) {
        printk(" %i %i %i %i %i %i %i\n",
                si->port_l2p_mapping[index],
                si->port_p2l_mapping[index],
                si->port_l2i_mapping[index],
                si->port_p2m_mapping[index],
                si->port_m2p_mapping[index],
                si->port_pipe[index],
                si->port_serdes[index]);
    }
#endif                
}

int
_bcm_egr_lport_profile_entry_add(bcmsw_switch_t *bcmsw, int port,  uint32 *index)
{
    int lport_profile_id = 3;
    uint32_t val;
    egr_lport_entry_t  entry;

    if (port != 0) {
        *index = lport_profile_id;
        return 0;
    }

    printk("_bcm_egr_lport_profile_entry_add. \n");
    _soc_mem_read(bcmsw->dev, EGR_LPORT_PROFILEm+lport_profile_id, SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_LPORT_PROFILEm_BYTES), (uint32_t *)&entry); 

    //EN_EFILTERf start 8, len 2
    val = 1;
    _mem_field_set((uint32_t *)&entry, EGR_LPORT_PROFILEm_BYTES, 8, 2, &val, SOCF_LE);

    //EM_SRCMOD_CHANGEf start 10, len 1 
    val = 1;
    _mem_field_set((uint32_t *)&entry, EGR_LPORT_PROFILEm_BYTES, 10, 1, &val, 0);
    
    //EFP_FILTER_ENABLEf start 35, len 1
    val = 1;
    _mem_field_set((uint32_t *)&entry, EGR_LPORT_PROFILEm_BYTES, 35, 1, &val, 0);

    _soc_mem_write(bcmsw->dev, EGR_LPORT_PROFILEm+lport_profile_id, SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_LPORT_PROFILEm_BYTES), (uint32_t *)&entry); 

    return 0;
}



int
_bcm_lport_profile_entry_add(bcmsw_switch_t *bcmsw, int port, uint32_t *index, int vid)
{
    int lport_profile_id = 3;
    uint32_t val;
    lport_tab_entry_t  lport_entry;

    if (port != 0) {
        *index = lport_profile_id;
        return 0;
    }

    printk("_bcm_lport_profile_entry_add. \n");
    /* PORT_TABLE config init */
    //read LPORT_TABm , check _bcm_td3_port_tab_conv for memory
    _soc_mem_read(bcmsw->dev, LPORT_TABm+lport_profile_id, SCHAN_BLK_IPIPE, BYTES2WORDS(LPORT_TABm_BYTES), (uint32_t *)&lport_entry); 

    //PORT_VIDf start 3, len 12
    val = vid;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 3, 12, &val, SOCF_LE);

    //MAC_BASED_VID_ENABLEf start 240, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 240, 1, &val, 0);

    // SUBNET_BASED_VID_ENABLEf start 239, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 239, 1, &val, 0);
    
    //PRI_MAPPINGf start 202, len 24
    val = 0xfac688;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 202, 24, &val, SOCF_LE);

    //CFI_0_MAPPINGf start 226, len 1
    val = 0;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 226, 1, &val, 0);

    //CFI_1_MAPPINGf start 227, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 227, 1, &val, 0);

    //IPRI_MAPPINGf start 122, len 24
    val = 0xfac688;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 122, 24, &val, SOCF_LE);   

    //ICFI_0_MAPPINGf start 121, len 1
    val = 0;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 121, 1, &val, 0);

    //ICFI_1_MAPPINGf start 120, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 120, 1, &val, 0);

    //CML_FLAGS_NEWf start 253, len 4
    val = 0x8;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 253, 4, &val, SOCF_LE);   

    //CML_FLAGS_MOVEf start 257, len 4
    val = 0x8;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 257, 4, &val, SOCF_LE);   


    //VFP_ENABLEf start 25, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 25, 1, &val, 0);

    //_bcm_esw_pt_vtkey_type_value_get(unit, VLXLT_HASH_KEY_TYPE_OVID,
    //VT_KEY_TYPEf start 46, len 4
    val = 4; // VLXLT_HASH_KEY_TYPE_OVID
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 46, 4, &val, SOCF_LE);

    //VT_PORT_TYPE_SELECT_1f start 52, len 2
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 52, 2, &val, SOCF_LE);  

    //_bcm_esw_pt_vtkey_type_value_get(unit, VLXLT_HASH_KEY_TYPE_IVID,

    //VT_KEY_TYPE_2f start 54, len 4
    val = 5; //VLXLT_HASH_KEY_TYPE_IVID
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 54, 4, &val, SOCF_LE);  

    //VT_PORT_TYPE_SELECT_2f start 58, len 2
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 58, 2, &val, SOCF_LE);

    //MIM_TERM_ENABLEf start 64, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 64, 1, &val, 0);


    //IPMC_DO_VLANf start 234, len 1 
    val =1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 234, 1, &val, 0);
    
    //FILTER_ENABLEf start 251, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 251, 1, &val, 0);

    //V6IPMC_ENABLEf start 394, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 394, 1, &val, 0);

    //V4IPMC_ENABLEf start 395, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 395, 1, &val, 0);

    //V6L3_ENABLEf start 396, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 396, 1, &val, 0);

    //V4L3_ENABLEf start 397, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 397, 1, &val, 0);


    //MPLS_ENABLEf start 400, len 1
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 400, 1, &val, 0);

    //TAG_ACTION_PROFILE_PTRf start 113, len 7
    val = 1;
    _mem_field_set((uint32_t *)&lport_entry, LPORT_TABm_BYTES, 113, 7, &val, SOCF_LE); 

    _soc_mem_write(bcmsw->dev, LPORT_TABm+lport_profile_id, SCHAN_BLK_IPIPE, BYTES2WORDS(LPORT_TABm_BYTES), (uint32_t *)&lport_entry); 
    
    *index = lport_profile_id;
    return 0;
}

//_bcm_vlan_vfi_membership_profile_entry_op
int _bcm_ing_vlan_vfi_membership_profile_entry_add(bcmsw_switch_t *bcmsw, uint32_t *index, vlan_data_t *vd)
{
    int ing_vfi_profile_id = 2;
    uint32_t val;
    uint32_t entry[SOC_MAX_MEM_WORDS];

    printk("_bcm_ing_vlan_vfi_membership_profile_entry_add. \n");

    memset(&entry, 0, sizeof(entry));
    _soc_mem_read(bcmsw->dev, ING_VLAN_VFI_MEMBERSHIPm+ing_vfi_profile_id, 
                  SCHAN_BLK_IPIPE, BYTES2WORDS(ING_VLAN_VFI_MEMBERSHIPm_BYTES), 
                  (uint32_t *)&entry); 

    //ING_PORT_BITMAPf start 0, len 72
    _mem_field_set((uint32_t *)&entry, ING_VLAN_VFI_MEMBERSHIPm_BYTES, 0, 72, &(vd->port_bitmap), SOCF_LE);              

    _soc_mem_write(bcmsw->dev, ING_VLAN_VFI_MEMBERSHIPm+ing_vfi_profile_id, 
        SCHAN_BLK_IPIPE, BYTES2WORDS(ING_VLAN_VFI_MEMBERSHIPm_BYTES), 
        (uint32_t *)&entry);     

    *index = ing_vfi_profile_id;
    return 0;
}

//_bcm_vlan_vfi_membership_profile_entry_op
int _bcm_egr_vlan_vfi_membership_profile_entry_add(bcmsw_switch_t *bcmsw, uint32_t *index, vlan_data_t *vd)
{
    int egr_vfi_profile_id = 2;
    uint32_t val;
    uint32_t entry[SOC_MAX_MEM_WORDS];

    printk("_bcm_egr_vlan_vfi_membership_profile_entry_add. \n");

    memset(&entry, 0, sizeof(entry));
    _soc_mem_read(bcmsw->dev, EGR_VLAN_VFI_MEMBERSHIPm+egr_vfi_profile_id, 
                  SCHAN_BLK_IPIPE, BYTES2WORDS(EGR_VLAN_VFI_MEMBERSHIPm_BYTES), 
                  (uint32_t *)&entry); 

    //ING_PORT_BITMAPf start 0, len 72
    _mem_field_set((uint32_t *)&entry, EGR_VLAN_VFI_MEMBERSHIPm_BYTES, 0, 72, &(vd->port_bitmap), SOCF_LE);              

    _soc_mem_write(bcmsw->dev, EGR_VLAN_VFI_MEMBERSHIPm+egr_vfi_profile_id, 
        SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_VLAN_VFI_MEMBERSHIPm_BYTES), 
        (uint32_t *)&entry);         

    *index = egr_vfi_profile_id;
    return 0;
}


#define BCMSW_VLAN_DEFAULT    1

//bcm_esw_port_init bcm_td3_port_cfg_init
static int 
_port_cfg_init(bcmsw_switch_t *bcmsw, int port, int vid)
{
    soc_info_t *si = bcmsw->si;
    uint32_t port_type;
    uint32_t egr_port_type = 0;
    int cpu_hg_index = -1;    
    uint32_t val;
    int lport_profile_id, egr_profile_id;
    uint32_t entry[SOC_MAX_MEM_WORDS];

    if (si->port_type[port] == BCMSW_PORT_TYPE_CMIC) {
        cpu_hg_index = si->cpu_hg_index;
        port_type = 0; /* Same as Ethernet port */
        egr_port_type = 1; /* CPU port needs to be HG port in EGR_PORT table */
    } else if (si->port_type[port] == BCMSW_PORT_TYPE_LBPORT) {
        port_type = 2;
        egr_port_type = 2;
    } else {
        port_type = 0;
    }

    /* EGR_LPORT_TABLE init */
    //create EGR_LPORT_PROFILE entry
    _bcm_egr_lport_profile_entry_add(bcmsw, port, &egr_profile_id);

    //read EGR_PORTm
    memset(entry, 0, sizeof(entry));
    _soc_mem_read(bcmsw->dev, EGR_PORTm+port, SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_PORTm_BYTES), (uint32_t *)entry); 

    //PORT_TYPEf start 26, len 3
    val = egr_port_type;
    _mem_field_set((uint32_t *)entry, EGR_PORTm_BYTES, 26, 3, &val, SOCF_LE); 

    //EGR_LPORT_PROFILE_IDXf start 0, len 10
    val = egr_profile_id;
    _mem_field_set((uint32_t *)entry, EGR_PORTm_BYTES, 0, 10, &val, SOCF_LE); 

    //EGR_PORT_CTRL_IDf start 10, len 8 - disable loopback
    val = 1;
    _mem_field_set((uint32_t *)entry, EGR_PORTm_BYTES, 10, 8, &val, SOCF_LE); 

    //QOS_CTRL_IDf start 29, len 4 - BCM_PORT_QOS_LAYERED_RESOLUTION
    val = 3;
    _mem_field_set((uint32_t *)entry, EGR_PORTm_BYTES, 29, 4, &val, SOCF_LE); 

    _soc_mem_write(bcmsw->dev, EGR_PORTm+port, SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_PORTm_BYTES), (uint32_t*)entry); 


    //VT_PORT_GROUP_IDf -> EGR_GPP_ATTRIBUTESm
    memset(entry, 0, sizeof(entry));

    _soc_mem_read(bcmsw->dev, EGR_GPP_ATTRIBUTESm+port, SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_GPP_ATTRIBUTESm_BYTES), (uint32_t *)entry); 
    //VT_PORT_GROUP_IDf start 75, len 8
    val = port;
    _mem_field_set((uint32_t *)entry, EGR_GPP_ATTRIBUTESm_BYTES, 75, 8, &val, SOCF_LE); 
    _soc_mem_write(bcmsw->dev, EGR_GPP_ATTRIBUTESm+port, SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_GPP_ATTRIBUTESm_BYTES), (uint32_t*)entry); 


    /* initialize the Cancun tag profile entry setup
     * for VT_MISS_UNTAG action. Should be done in Cancun
     */
    //soc_cancun_cmh_mem_set(unit, 

    /* Copy EGR port information to CPU Higig port if applied */
    //Not applicable for BCM56370

 

    //Update ING_DEVICE_PORTm
    memset(entry, 0, sizeof(entry));

    //SRC_SYS_PORT_ID start 117, len 7
    val = port;
    _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 117, 7, &val, SOCF_LE); 

    //SYS_PORT_IDf start 13, len 7
    _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 13, 7, &val, SOCF_LE); 

    //PP_PORT_NUM start 20, len 7
    _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 20, 7, &val, SOCF_LE); 

    //PORT_TYPE start 0, len 3
    val = port_type;
    _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 0, 3, &val, SOCF_LE); 

    //LPORT_PROFILE_IDXf start 3, len 10
    _bcm_lport_profile_entry_add(bcmsw,port, &lport_profile_id, vid);
    val = lport_profile_id;
    _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 3, 10, &val, SOCF_LE);     

    //PARSE_CONTEXT_ID_0f start 59, len 16 enable  = 6
    val = 6;
    _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 59, 16, &val, SOCF_LE);

    _soc_mem_write(bcmsw->dev, ING_DEVICE_PORTm+port, SCHAN_BLK_IPIPE, BYTES2WORDS(ING_DEVICE_PORTm_BYTES), (uint32_t *)entry); 


    if (cpu_hg_index != -1) {
        //soc_cancun_cmh_mem_set(unit, PORT_TABm, cpu_hg_index, PORT_TYPEf, 1);
        /* TD3TBD should be covered by CMH, will remove it after CMH
         * is ready. */
         _soc_mem_read(bcmsw->dev, ING_DEVICE_PORTm+cpu_hg_index, SCHAN_BLK_IPIPE, BYTES2WORDS(ING_DEVICE_PORTm_BYTES), (uint32_t *)entry); 

        //PORT_TYPEf start 0, len 3
        val = 1;
        _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 0, 3, &val, SOCF_LE);  

        //SRC_SYS_PORT_IDf start 117, len 7
        val = port;
        _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 117, 7, &val, SOCF_LE);
        
        //SYS_PORT_IDf start 13, len 7
        val = port;
        _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 13, 7, &val, SOCF_LE);

        //PP_PORT_NUMf start 20, len 7
        val = port;
        _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 20, 7, &val, SOCF_LE);

        //DUAL_MODID_ENABLEf start 114, len 1
        val = 0;
        _mem_field_set((uint32_t *)entry, ING_DEVICE_PORTm_BYTES, 114, 1, &val, 0);

        _soc_mem_write(bcmsw->dev, ING_DEVICE_PORTm+cpu_hg_index, SCHAN_BLK_IPIPE, BYTES2WORDS(ING_DEVICE_PORTm_BYTES), (uint32_t *)entry); 
    }    

    return 0;
}

 


/*****************************************************************************************/
/*                             UNI MAC                                                   */
/*****************************************************************************************/

static const uint32_t cmd_cfg_blk[6] = {
    SCHAN_BLK_GXPORT0,
    SCHAN_BLK_GXPORT1,
    SCHAN_BLK_GXPORT2,
    SCHAN_BLK_GXPORT3,
    SCHAN_BLK_GXPORT4,
    SCHAN_BLK_GXPORT5,
};
static int unimac_reset_check(bcmsw_switch_t *bcmsw, int port, int enable, int *reset)
{
    command_config_t ctrl, octrl, swctrl;
    int index, blk_no;

    *reset = 1;

    if (port > 48) {
        return -1;
    }
    index = (port-1)%8;
    blk_no = cmd_cfg_blk[(port-1)/8];

    _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
    octrl.word = ctrl.word;

    //printk("unimac_reset_check port %d ctrl 0x%x\n", port, ctrl.word);
    ctrl.reg.TX_ENAf = enable ? 1:0;
    ctrl.reg.RX_ENAf = enable ? 1:0;


    if (ctrl.word == octrl.word) {
        if (enable) {
            *reset = 0;
        } else {
            _reg32_read(bcmsw->dev,blk_no, COMMAND_CONFIGr+index, &swctrl.word);

            if (swctrl.reg.SW_RESETf) {
                *reset = 0;
            }
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      mac_uni_enable_get
 * Purpose:
 *      Get UniMAC enable state
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - Port number on unit.
 *      flags - UNIMAC_ENABLE_SET_FLAGS_TX_EN or UNIMAC_ENABLE_SET_FLAGS_RX_EN
 *      enable - (OUT) TRUE if enabled, FALSE if disabled
 * Returns:
 *      SOC_E_XXX
 */
int unimac_enable_get(bcmsw_switch_t *bcmsw, int port, int *enable)
{
    int index, blk_no;
    command_config_t command_config;

    if (port > 48) {
        return -1;
    }
    index = (port-1)%8;
    blk_no = cmd_cfg_blk[(port-1)/8];

    _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &command_config.word);

    if (command_config.reg.TX_ENAf && command_config.reg.RX_ENAf) {
        *enable = 1;
    } else {
        *enable = 0;
    }
    return SOC_E_NONE;
}

int unimac_rx_max_size_set(bcmsw_switch_t *bcmsw, int port, int value)
{
    uint32 rx_ena;
    int speed = 0;
    int index, blk_no;
    command_config_t command_config;

    if (port > 48) {
        return -1;
    }
    index = (port-1)%8;
    blk_no = cmd_cfg_blk[(port-1)/8];    
   
    //if (IS_ST_PORT(unit, port)) {
    //    value += 16; /* Account for 16 bytes of Higig2 header */
    //}

    _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &command_config.word);
    rx_ena = command_config.reg.RX_ENAf;

    /* If Rx is enabled then disable RX */
    if (rx_ena) {
        /* Disable RX */
        command_config.reg.RX_ENAf = 0;
        _reg32_write(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, command_config.word);

        /* Wait for maximum frame receiption time(for 16K) based on speed */
        speed = command_config.reg.ETH_SPEEDf;
        switch (speed) {
        case SOC_UNIMAC_SPEED_2500:
            msleep(1);
            break;
        case SOC_UNIMAC_SPEED_1000:
            msleep(1);
            break;
        case SOC_UNIMAC_SPEED_100:
            msleep(2);
            break;
        case SOC_UNIMAC_SPEED_10:
            msleep(15);
            break;
        default:
            break;
        }
    }

    _reg32_write(bcmsw->dev, blk_no, FRM_LENGTHr+index, value);

    /* if Rx was enabled before, restore it */
    if (rx_ena) {
        command_config.reg.RX_ENAf = 1;
        _reg32_write(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, command_config.word);
    }

    return SOC_E_NONE;
}

int unimac_soft_reset_set(bcmsw_switch_t *bcmsw, int port, int enable)
{
    int index, blk_no;
    command_config_t command_config;

    if (port > 48) {
        return -1;
    }
    index = (port-1)%8;
    blk_no = cmd_cfg_blk[(port-1)/8];        
 
    /* SIDE EFFECT: TX and RX are enabled when SW_RESET is set. */
    _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &command_config.word);
    //printk("unimac_soft_reset_set read port %d command_config 0x%08x\n",port, command_config.word);
    command_config.reg.SW_RESETf = enable;
    //printk("unimac_soft_reset_set write port %d command_config 0x%08x\n",port, command_config.word);
    _reg32_write(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, command_config.word);

    msleep(50);

    return SOC_E_NONE;
}

/*
 * Function:
 *      unimac_init
 * Purpose:
 *      Initialize UniMAC into a known good state.
 * Parameters:
 *      unit - StrataSwitch unit #.
 *      port - StrataSwitch port # on unit.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The initialization speed/duplex is arbitrary and must be
 *      updated by linkscan before enabling the MAC.
 */

int unimac_init(bcmsw_switch_t *bcmsw, int port, int init_flags)
{
    uint32_t reg_val;
    int    frame_max, ignore_pause;
    int    is_crc_fwd;
    int    index, blk_no;
    command_config_t command_config, old_command_config;
 
    if (port > 48) {
       return -1;
    }
    index = (port-1)%8;
    blk_no = cmd_cfg_blk[(port-1)/8];             
 
    is_crc_fwd = (init_flags & UNIMAC_INIT_F_RX_STRIP_CRC) ? 0 : 1;
 
    /* Get MAC configurations from config settings. */
     
    frame_max = SOC_UNIMAC_MAX_FRAME_SIZE;
    unimac_rx_max_size_set(bcmsw, port, frame_max);
  
    /* First put the MAC in reset and sleep */
    unimac_soft_reset_set(bcmsw, port, TRUE);
 
    /* Do the initialization */
    _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &command_config.word);
    //printk("unimac_init 1 read: port %d command_config 0x%08x\n",port, command_config.word);
    old_command_config.word = command_config.word;
 
    command_config.reg.TX_ENAf    = 0;
    command_config.reg.RX_ENAf    = 0;
    command_config.reg.ETH_SPEEDf = SOC_UNIMAC_SPEED_1000;
    command_config.reg.PROMIS_ENf = 1;
    command_config.reg.PAD_ENf    = 0;
    command_config.reg.CRC_FWDf   = is_crc_fwd;
    command_config.reg.PAUSE_FWDf = 0;
 
    /* Ignore pause if using as stack port */
    ignore_pause = 0;
    command_config.reg.PAUSE_IGNOREf    = ignore_pause;
    command_config.reg.IGNORE_TX_PAUSEf = ignore_pause;
 
    command_config.reg.TX_ADDR_INSf     = 0;
    command_config.reg.HD_ENAf          = 0;
    command_config.reg.LOOP_ENAf        = 0;
     
    command_config.reg.NO_LGTH_CHECKf   = 1;
    command_config.reg.LINE_LOOPBACKf   = 0;
    command_config.reg.RX_ERR_DISCf     = 0;
 
    command_config.reg.CNTL_FRM_ENAf    = 1;
    command_config.reg.ENA_EXT_CONFIGf  = (init_flags & UNIMAC_INIT_F_AUTO_CFG) ? 1 : 0;
 
    if (init_flags & UNIMAC_INIT_F_AUTO_CFG) {
       command_config.reg.SW_OVERRIDE_RXf = 1;
       command_config.reg.SW_OVERRIDE_TXf = 1;
    }
 
    if (old_command_config.word != command_config.word) {
       //printk("unimac_init 2 write: port %d command_config 0x%08x\n",port, command_config.word);
       _reg32_write(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, command_config.word);
    }
 
    _reg32_read(bcmsw->dev, blk_no, TAG_0r+index, &reg_val);
    //soc_reg_field_set(unit, TAG_0r, &reg_val, CONFIG_OUTER_TPID_ENABLEf, 0);
    reg_val &= 0x18100;
    _reg32_write(bcmsw->dev, blk_no, TAG_0r+index, reg_val);
 
    _reg32_read(bcmsw->dev, blk_no, TAG_1r+index, &reg_val);
    reg_val &= 0x8100;
    _reg32_write(bcmsw->dev, blk_no, TAG_1r+index, reg_val);
 
    _reg32_read(bcmsw->dev, blk_no, UMAC_TIMESTAMP_ADJUSTr+index, &reg_val);
    reg_val &= ~(1<<10); //AUTO_ADJUSTf bit 10
    _reg32_write(bcmsw->dev, blk_no, UMAC_TIMESTAMP_ADJUSTr+index, reg_val);
 
    /* Bring the UniMAC out of reset */
    unimac_soft_reset_set(bcmsw, port, FALSE);
 
    
    //soc_reg_field_set(unit, PAUSE_CONTROLr, &reg_val, ENABLEf, 1);
    //soc_reg_field_set(unit, PAUSE_CONTROLr, &reg_val, VALUEf, 0x1ffff);
    reg_val = 0x3FFFF;
    _reg32_write(bcmsw->dev, blk_no, PAUSE_CONTROLr+index, reg_val);

    _reg32_write(bcmsw->dev, blk_no, PAUSE_QUANTr+index, 0xffff);

 
    _reg32_read(bcmsw->dev, blk_no, MAC_PFC_REFRESH_CTRLr+index, &reg_val);
    //soc_reg_field_set(unit, MAC_PFC_REFRESH_CTRLr, &reg_val, PFC_REFRESH_ENf, 1);
    //soc_reg_field_set(unit, MAC_PFC_REFRESH_CTRLr, &reg_val, PFC_REFRESH_TIMERf, 0xc000);
    reg_val |= 0xc0000001;
    _reg32_write(bcmsw->dev, blk_no, MAC_PFC_REFRESH_CTRLr+index, reg_val);
 
    reg_val = 0xc;
    _reg32_write(bcmsw->dev, blk_no, TX_IPG_LENGTHr+index, reg_val);
 
    /* Set egress enable */
    
 
    /* assigning proper setting for EEE feature :
     * Note : GE speed force assigned for timer setting 
     */
    _reg32_write(bcmsw->dev, blk_no, UMAC_EEE_REF_COUNTr+index, SOC_UNIMAC_EEE_REF_CNT);

    _reg32_write(bcmsw->dev, blk_no, GMII_EEE_WAKE_TIMERr+index, SOC_UNIMAC_WAKE_TIMER);

    _reg32_write(bcmsw->dev, blk_no, GMII_EEE_DELAY_ENTRY_TIMERr+index, SOC_UNIMAC_LPI_TIMER);
 
    return SOC_E_NONE;
}
 

//soc_helix5_idb_obm_reset_buffer
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

static int
_helix5_get_pm_from_phynum(int phy_port)
{
    int num;

    num = phy_port - 1;
    if (num <= 49) {
        num = num/16;
    } else if (num <= HELIX5_TDM_PHY_PORTS_PER_PIPE) { 
        num = (num/4)-9;
    } else {
        return -1;
    }

    return num;
}

int
_helix5_idb_obm_reset_buffer(bcmsw_switch_t *bcmsw, int port, int reset_buffer)
{
    uint32_t reg;
    uint32_t offset = 0;
    int32_t phy_port = bcmsw->si->port_l2p_mapping[port];
    int pm_num, subp;
    obm_q_control_t reg_obm;

    pm_num = _helix5_get_pm_from_phynum(phy_port);
    subp = (phy_port -1)&0x3;

    reg = obm_ctrl_regs[pm_num];
    

    offset = (phy_port - 1)%16;
    offset = offset/4;

    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, reg+(offset<<8), &reg_obm.word);
    //printk("IDB port Up rval 0x%x pm_num %1d subp=%1d reset_buffer=%1d offset=%1d  \n",
    //       reg_obm.word, pm_num, subp,reset_buffer,offset);
    if (subp == 0) {
        reg_obm.reg.PORT0_RESETf = reset_buffer;
    } else if (subp == 1) {
        reg_obm.reg.PORT1_RESETf = reset_buffer;
    } else if (subp == 2) {
        reg_obm.reg.PORT2_RESETf = reset_buffer;
    } else {
        reg_obm.reg.PORT3_RESETf = reset_buffer;
    }

    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, reg+(offset<<8), reg_obm.word);
    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, reg+(offset<<8), &reg_obm.word);

    //printk("IDB port Up rval_update 0x%x pm_num %1d sbup=%1d reset_buffer=%1d offset=%1d \n",
    //       reg_obm.word, pm_num, subp,reset_buffer,offset);

    return SOC_E_NONE;
}

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
// soc_helix5_idb_ca_reset_buffer
int
_helix5_idb_ca_reset_buffer(bcmsw_switch_t *bcmsw, int port, int reset_buffer)
{
    uint32_t reg;
    obm_q_ca_control_t val32;
    int32_t phy_port = bcmsw->si->port_l2p_mapping[port];
    int pm_num, subp;

    pm_num = _helix5_get_pm_from_phynum(phy_port);

    if(phy_port < 49) {
        subp = (phy_port-1)%16;
    }

    reg = soc_helix5_obm_ca_ctrl_regs[pm_num];

    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, reg, &val32.word);

    if (subp == 0) {
        val32.reg.PORT0_RESETf = reset_buffer;
    } else if (subp == 1) {
        val32.reg.PORT1_RESETf = reset_buffer;
    } else if (subp == 2) {
        val32.reg.PORT2_RESETf = reset_buffer;
    } else if (subp == 3) {
        val32.reg.PORT3_RESETf = reset_buffer;
    } else if (subp == 4) {
        val32.reg.PORT4_RESETf = reset_buffer;
    } else if (subp == 5) {
        val32.reg.PORT5_RESETf = reset_buffer;
    } else if (subp == 6) {
        val32.reg.PORT6_RESETf = reset_buffer;
    } else if (subp == 7) {
        val32.reg.PORT7_RESETf = reset_buffer;
    } else if (subp == 8) {
        val32.reg.PORT8_RESETf = reset_buffer;
    } else if (subp == 9) {
        val32.reg.PORT9_RESETf = reset_buffer;
    } else if (subp == 10) {
        val32.reg.PORT10_RESETf = reset_buffer;
    } else if (subp == 11) {
        val32.reg.PORT11_RESETf = reset_buffer;
    } else if (subp == 12) {
        val32.reg.PORT12_RESETf = reset_buffer;
    } else if (subp == 13) {
        val32.reg.PORT13_RESETf = reset_buffer;
    } else if (subp == 14) {
        val32.reg.PORT14_RESETf = reset_buffer;
    } else if (subp == 15) {
        val32.reg.PORT15_RESETf = reset_buffer;
    } else {
        val32.reg.PORT0_RESETf = reset_buffer;
    }

    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, reg, val32.word);

    //printk("_helix5_idb_ca_reset_buffer port %d 0x%x pm_num %1d sbup=%1d reset_buffer=%1d  \n",
    //        port, val32.word, pm_num, subp,reset_buffer);
    return SOC_E_NONE;
}

//soc_helix5_idb_lpbk_ca_reset_buffer
static int
_helix5_idb_lpbk_ca_reset_buffer(bcmsw_switch_t *bcmsw, int reset_buffer)
{
    idb_lpbk_ca_t val32;

    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_LPBK_CONTROLr, &val32.word);
         
    val32.reg.PORT_RESETf = reset_buffer;

    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_LPBK_CONTROLr, val32.word);

    //printk("_helix5_idb_lpbk_ca_reset_buffer reset_buffer=%1d", reset_buffer);

    return SOC_E_NONE;
}

//soc_helix5_idb_cpu_ca_reset_buffer
static  int
_helix5_idb_cpu_ca_reset_buffer(bcmsw_switch_t *bcmsw, int reset_buffer)
{
    idb_ca_cpu_t val32;

    _reg32_read(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_CPU_CONTROLr, &val32.word);

    val32.reg.PORT_RESETf = reset_buffer;

    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_CPU_CONTROLr, val32.word);
    
    //printk("_helix5_idb_cpu_ca_reset_buffer reset_buffer=%1d", reset_buffer);

    return SOC_E_NONE;
}

//soc_helix5_flex_idb_port_up
static int
_helix5_flex_idb_port_up(bcmsw_switch_t *bcmsw, int port)
{
    int reset_buffer;
    int phy_port;

    phy_port = bcmsw->si->port_l2p_mapping[port];

    /* Release IDB buffers from reset state for all the ports going up */

    reset_buffer = 1;
    if (HELIX5_PHY_IS_FRONT_PANEL_PORT(phy_port)) {
        _helix5_idb_obm_reset_buffer(bcmsw, port, reset_buffer);

        if(phy_port <49) {
                _helix5_idb_ca_reset_buffer(bcmsw, port, reset_buffer);
        } else if(phy_port==HELIX5_PHY_PORT_LPBK0) {
            _helix5_idb_lpbk_ca_reset_buffer(bcmsw, reset_buffer);
        } else if(phy_port==HELIX5_PHY_PORT_CPU) {
            _helix5_idb_cpu_ca_reset_buffer(bcmsw, reset_buffer);
        }
    }

    msleep(1);

    reset_buffer = 0;
    if (HELIX5_PHY_IS_FRONT_PANEL_PORT(phy_port)) {
        _helix5_idb_obm_reset_buffer(bcmsw, port, reset_buffer);

        if(phy_port <49) {
            _helix5_idb_ca_reset_buffer(bcmsw, port, reset_buffer);
        }else if(phy_port==HELIX5_PHY_PORT_LPBK0) {
            _helix5_idb_lpbk_ca_reset_buffer(bcmsw, reset_buffer);
        }else if(phy_port==HELIX5_PHY_PORT_CPU) {
            _helix5_idb_cpu_ca_reset_buffer(bcmsw, reset_buffer);
        }
    }
    return SOC_E_NONE;
}
static const uint32_t g_pmqblk[3] = {
    SCHAN_BLK_PMQPORT0,
    SCHAN_BLK_PMQPORT1,
    SCHAN_BLK_PMQPORT2,
};

// only valid for phy_port 1~49
static int 
_helix5_get_qmode(bcmsw_switch_t *bcmsw, int phy_port)
{
    int qmode;
    int blk_no;
    int32_t rval32;

    if (phy_port <= 16) {
        blk_no = SCHAN_BLK_PMQPORT0;
    } else if (phy_port <= 32) {
        blk_no = SCHAN_BLK_PMQPORT1; 
    } else {
        blk_no = SCHAN_BLK_PMQPORT2;
    }

    _reg32_read(bcmsw->dev, blk_no, CHIP_CONFIGr, &rval32);
    qmode = rval32 & 0x1;

    return qmode;
}

static const uint32_t gxblk[6] = {
    SCHAN_BLK_GXPORT0,
    SCHAN_BLK_GXPORT1,
    SCHAN_BLK_GXPORT2,
    SCHAN_BLK_GXPORT3,
    SCHAN_BLK_GXPORT4,
    SCHAN_BLK_GXPORT5,
};
//soc_helix5_flex_mac_port_up
int
_helix5_flex_mac_port_up(bcmsw_switch_t *bcmsw, int port)
{
    int i;
    //uint64_t rval64;
    uint32_t rval32;
    int phy_port;
    //int subp;
    int mode;
    int speed_100g;
    //int clport;
    //int higig2_mode;
    //int strict_preamble;
    //int higig_mode;
    int qmode;
    //int inst;
    int index;
    int speed_mode;
    //int hdr_mode;
    int blk_no;
    command_config_t ctrl;
    //static const int clport_mode_values[SOC_HX5_PORT_RATIO_COUNT] = {
    //    4, 3, 3, 3, 2, 2, 1, 1, 0
    //};
    struct net_device *dev;

    //strict_preamble = 0;

    dev = bcmsw->dev;

    /*Disable TSC lanes: */

    /*CLMAC_RX_CTRL */
    phy_port = bcmsw->si->port_l2p_mapping[port];
    //strict_preamble = 0;

    if(phy_port < 49) {
        qmode = _helix5_get_qmode(bcmsw, phy_port);
        index = (phy_port -1)%8;
    } else {
        qmode = 0;
    }
    //printk("_helix5_flex_mac_port_up port %d qmode %d\n", port, qmode);

    if(phy_port < 65) {
        if((phy_port < 49) && (qmode)){
            /* configured thru API:  bcm_port_frame_max_set */
            blk_no = gxblk[(phy_port-1)/8];
            _reg32_read(bcmsw->dev, blk_no, GPORT_RSV_MASKr+index, &rval32);
    
            rval32 = 120;
            _reg32_write(bcmsw->dev, blk_no, GPORT_RSV_MASKr+index, rval32);
        }  else {
#if 0
            printk("Bringing Eagle mac rx port %0d up\n", phy_port);

            _reg64_read(bcmsw->dev, blk_no, XLMAC_RX_CTRLr+index, &rval64);
            //soc_reg64_field32_set(unit, XLMAC_RX_CTRLr, &rval64, RX_ANY_STARTf, 0);
            //soc_reg64_field32_set(unit, XLMAC_RX_CTRLr, &rval64, STRIP_CRCf, 0);
            //soc_reg64_field32_set(unit, XLMAC_RX_CTRLr, &rval64, STRICT_PREAMBLEf,
            //                    strict_preamble);
            //soc_reg64_field32_set(unit, XLMAC_RX_CTRLr, &rval64, RUNT_THRESHOLDf,
            //                    64);
            _reg64_write(bcmsw->dev, blk_no, XLMAC_RX_CTRLr+index, rval64);
#endif            
        }
    } else {
        printk("Bringing Falcon mac rx port %0d up\n", phy_port);
#if 0        
        _reg64_read(bcmsw->dev, blk_no, CLMAC_RX_CTRLr+index, &rval64);
        //soc_reg64_field32_set(unit, CLMAC_RX_CTRLr, &rval64, RX_ANY_STARTf, 0);
        //soc_reg64_field32_set(unit, CLMAC_RX_CTRLr, &rval64, STRIP_CRCf, 0);
        //soc_reg64_field32_set(unit, CLMAC_RX_CTRLr, &rval64, STRICT_PREAMBLEf,
        //                        strict_preamble);
        //    soc_reg64_field32_set(unit, CLMAC_RX_CTRLr, &rval64, RUNT_THRESHOLDf,
        //                        64);
        _reg64_write(bcmsw->dev, blk_no, CLMAC_RX_CTRLr+index, rval64);
#endif        
    }

    /*CLPORT_CONFIG */
    if(phy_port < 65) { 
        if ( qmode != 1 ){
      printk("Setting Eagle mac xl port %0d up\n", phy_port);
#if 0
            _reg32_read(bcmsw->dev, blk_no, XLPORT_CONFIGr+index, &rval32);

            soc_reg_field_set(unit, XLPORT_CONFIGr, &rval32, HIGIG2_MODEf,
                              higig2_mode);
            soc_reg_field_set(unit, XLPORT_CONFIGr, &rval32, HIGIG_MODEf,
                              higig_mode);
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, XLPORT_CONFIGr,
                                                      phy_port, 0, rval32));
#endif
        }
    } else {    
        printk("Setting Falcon mac cl port %0d up\n", phy_port);
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_get(unit, CLPORT_CONFIGr,
                                                      phy_port, 0, &rval32));
            soc_reg_field_set(unit, CLPORT_CONFIGr, &rval32, HIGIG2_MODEf,
                              higig2_mode);
            soc_reg_field_set(unit, CLPORT_CONFIGr, &rval32, HIGIG_MODEf,
                              higig_mode);
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, CLPORT_CONFIGr,
                                                      phy_port, 0, rval32));
#endif                                                      
    }

    /*CLPORT Enable: */    
    if(phy_port < 65) { 
        if( qmode != 1 ) {
        printk("Setting Eagle enable port %0d up\n", phy_port);
#if 0            
            _reg32_read(bcmsw->dev, blk_no, XLPORT_ENABLE_REGr+index, &rval32);

            if (subp == 0) {
                soc_reg_field_set(unit, XLPORT_ENABLE_REGr, &rval32, PORT0f, 1);
            } else if (subp==1) {
                soc_reg_field_set(unit, XLPORT_ENABLE_REGr, &rval32, PORT1f, 1);
            } else if (subp==2) {
                soc_reg_field_set(unit, XLPORT_ENABLE_REGr, &rval32, PORT2f, 1);
            } else {
                soc_reg_field_set(unit, XLPORT_ENABLE_REGr, &rval32, PORT3f, 1);
            }
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, XLPORT_ENABLE_REGr,
                                                      phy_port, 0, rval32));
#endif                                                      
        }
    } else {
            printk("Setting Falcon enable port %0d up\n", phy_port);
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_get(unit, CLPORT_ENABLE_REGr,
                                                      phy_port, 0, &rval32));
            if (subp == 0) {
                soc_reg_field_set(unit, CLPORT_ENABLE_REGr, &rval32, PORT0f, 1);
            } else if (subp==1) {
                soc_reg_field_set(unit, CLPORT_ENABLE_REGr, &rval32, PORT1f, 1);
            } else if (subp==2) {
                soc_reg_field_set(unit, CLPORT_ENABLE_REGr, &rval32, PORT2f, 1);
            } else {
                soc_reg_field_set(unit, CLPORT_ENABLE_REGr, &rval32, PORT3f, 1);
            }
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, CLPORT_ENABLE_REGr,
                                                      phy_port, 0, rval32));
#endif                                                      
    }

    msleep(1);

    /*CLPORT mode: */
    if (bcmsw->si->port_init_speed[port] == 100000) {
        speed_100g = 1;
    } else {
        speed_100g = 0;
    }
#if 0      
    if(phy_port < 65 ){ 
      
        if ( qmode != 1) {
            LOG_DEBUG(BSL_LS_SOC_PORT, (BSL_META_U(unit, "Setting mode port %0d %0d %d\n"), phy_port, mode, clport_mode_values[mode]));
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_get(unit, XLPORT_MODE_REGr,
                                                      phy_port, 0, &rval32));
            soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval32,
                              XPORT0_CORE_PORT_MODEf,
                              clport_mode_values[mode]);
            soc_reg_field_set(unit, XLPORT_MODE_REGr, &rval32,
                              XPORT0_PHY_PORT_MODEf,
                              clport_mode_values[mode]);
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, XLPORT_MODE_REGr,
                                                      phy_port, 0, rval32));
        }
    } else {
            LOG_DEBUG(BSL_LS_SOC_PORT, (BSL_META_U(unit, "Setting mode port %0d %0d %d\n"), phy_port, mode, clport_mode_values[mode]));
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_get(unit, CLPORT_MODE_REGr,
                                                      phy_port, 0, &rval32));
            soc_reg_field_set(unit, CLPORT_MODE_REGr, &rval32,
                              SINGLE_PORT_MODE_SPEED_100Gf,
                              speed_100g);
            soc_reg_field_set(unit, CLPORT_MODE_REGr, &rval32,
                              XPORT0_CORE_PORT_MODEf,
                              clport_mode_values[mode]);
            soc_reg_field_set(unit, CLPORT_MODE_REGr, &rval32,
                              XPORT0_PHY_PORT_MODEf,
                              clport_mode_values[mode]);
            SOC_IF_ERROR_RETURN(soc_reg32_rawport_set(unit, CLPORT_MODE_REGr,
                                                      phy_port, 0, rval32));
    }
    sleep(200);
#endif    

    /* Release soft reset */
    if(phy_port <65) {
        if((phy_port < 49) && (qmode)){
            index = (phy_port-1)%8;
            blk_no = gxblk[(phy_port-1)/8];
    
            _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
            ctrl.reg.SW_RESETf = 0;
            _reg32_write(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, ctrl.word);
        } else {
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, XLMAC_CTRLr, phy_port,
                                                    0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, SOFT_RESETf, 0);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, XLMAC_CTRLr, phy_port,
                                                    0, rval64));
#endif                                                    
        }
    } else {
#if 0        
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, CLMAC_CTRLr, phy_port,
                                                    0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, SOFT_RESETf, 0);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_CTRLr, phy_port,
                                                    0, rval64));
            /*
             * Special handling for new mac version. Internally MAC loopback
             * looks for rising edge on MAC loopback configuration to enter
             * loopback state.
             * Do only if loopback is enabled on the port.
             */
            if (soc_reg64_field32_get(unit, CLMAC_CTRLr, rval64, LOCAL_LPBKf)) {
                soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, LOCAL_LPBKf, 0);
                SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_CTRLr, phy_port,
                                                        0, rval64));
                /* Wait 10usec as suggested by MAC designer */
                sal_udelay(10);
                soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, LOCAL_LPBKf, 1);
                SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_CTRLr, phy_port,
                                                        0, rval64));
        }
#endif
    }

    msleep(1);

    /* Enable MAC RX_EN & TX_EN */
    if(phy_port < 65) {
        if((phy_port < 49) && (qmode)){
            index = (phy_port-1)%8;
            blk_no = gxblk[(phy_port-1)/8];

            _reg32_read(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
            ctrl.reg.RX_ENAf = 1;
            ctrl.reg.TX_ENAf = 1;
            _reg32_write(bcmsw->dev, blk_no, COMMAND_CONFIGr+index, ctrl.word);

            //TODO
            //SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, FLUSH_CONTROLr, phy_port,
            //                                        0, &rval64));
            //soc_reg64_field32_set(unit, FLUSH_CONTROLr, &rval64, FLUSHf, 0);
            //SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, FLUSH_CONTROLr, phy_port,
            //                                        0, rval64));
                            
        } else {
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, XLMAC_TX_CTRLr,
                                                    phy_port, 0, &rval64));
            soc_reg64_field32_set(unit, XLMAC_TX_CTRLr, &rval64, DISCARDf, 0);
            soc_reg64_field32_set(unit, XLMAC_TX_CTRLr, &rval64, EP_DISCARDf, 0);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, XLMAC_TX_CTRLr,
                                                    phy_port, 0, rval64));

            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, XLMAC_CTRLr, phy_port,
                                                    0, &rval64));
            soc_reg64_field32_set(unit, XLMAC_CTRLr, &rval64, RX_ENf, 1);
            soc_reg64_field32_set(unit, XLMAC_CTRLr, &rval64, TX_ENf, 1);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, XLMAC_CTRLr, phy_port,
                                                    0, rval64));
#endif                                                    
        }
    } else {
#if 0        
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, CLMAC_TX_CTRLr,
                                                    phy_port, 0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_TX_CTRLr, &rval64, DISCARDf, 0);
            soc_reg64_field32_set(unit, CLMAC_TX_CTRLr, &rval64, EP_DISCARDf, 0);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_TX_CTRLr,
                                                    phy_port, 0, rval64));

            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, CLMAC_CTRLr, phy_port,
                                                    0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, RX_ENf, 1);
            soc_reg64_field32_set(unit, CLMAC_CTRLr, &rval64, TX_ENf, 1);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_CTRLr, phy_port,
                                                    0, rval64));
#endif                                                    
    }

    /* CLMAC MODE */
    //hdr_mode = 0;
    // case 10   : speed_mode = 0;
    // case 100  : speed_mode = 1;
    // case 1000 : speed_mode = 2; 
    // case 2500 : speed_mode = 3;
    // default   : speed_mode = 4;
    if (bcmsw->si->port_init_speed[port] == 1000) {
        speed_mode = 2;
    } else {
        speed_mode = 4;
    }

    if(phy_port < 65) {
        if((phy_port < 49) && (qmode)){
            index = (phy_port-1)%8;
            blk_no = gxblk[(phy_port-1)/8];

            _reg32_read(dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
            ctrl.reg.ETH_SPEEDf = speed_mode;
            _reg32_write(dev, blk_no, COMMAND_CONFIGr+index, ctrl.word);

            _reg32_read(dev, blk_no, COMMAND_CONFIGr+index, &ctrl.word);
            //printk("soc_helix5_flex_mac_port_up port %d command_config 0x%x\n", phy_port, ctrl.word);
     } else {
#if 0            
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, XLMAC_MODEr,
                                                    phy_port, 0, &rval64));
            soc_reg64_field32_set(unit, XLMAC_MODEr, &rval64, HDR_MODEf, hdr_mode);
            soc_reg64_field32_set(unit, XLMAC_MODEr, &rval64, SPEED_MODEf, speed_mode);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, XLMAC_MODEr,
                                                    phy_port, 0, rval64));
#endif                                                    
      }
    } else {
#if 0        
            SOC_IF_ERROR_RETURN(soc_reg_rawport_get(unit, CLMAC_MODEr,
                                                    phy_port, 0, &rval64));
            soc_reg64_field32_set(unit, CLMAC_MODEr, &rval64, HDR_MODEf, hdr_mode);
            soc_reg64_field32_set(unit, CLMAC_MODEr, &rval64, SPEED_MODEf, speed_mode);
            SOC_IF_ERROR_RETURN(soc_reg_rawport_set(unit, CLMAC_MODEr,
                                                    phy_port, 0, rval64));
#endif                                                    
    }

    //printk("END soc_helix5_flex_mac_port_up\n");
    return SOC_E_NONE;
}

//soc_helix5_flex_en_forwarding_traffic
int
_helix5_flex_en_forwarding_traffic(bcmsw_switch_t *bcmsw, int port)
{
    uint32_t ing_entry[3]; //72 bit
    uint32_t epc_entry[3]; //72 bit

    _soc_mem_read(bcmsw->dev, ING_DEST_PORT_ENABLEm, SCHAN_BLK_IPIPE, 3, ing_entry); 

    ing_entry[port>>5] |= (0x1<<(port&0x1f));

    _soc_mem_write(bcmsw->dev, ING_DEST_PORT_ENABLEm, SCHAN_BLK_IPIPE, 3, ing_entry); 

    //printk("Enable ING_DEST_PORT_ENABLE write:: 0x%08x 0x%08x 0x%08x\n",
    //       ing_entry[0],ing_entry[1], ing_entry[2]);

    /* EPC_LINK_BMAP read, field modify and write. */
    _soc_mem_read(bcmsw->dev, EPC_LINK_BMAPm, SCHAN_BLK_IPIPE, 3, epc_entry); 

    epc_entry[port>>5] |= (0x1<<(port&0x1f));

    _soc_mem_write(bcmsw->dev, EPC_LINK_BMAPm, SCHAN_BLK_IPIPE, 3, epc_entry); 

    //printk("Enable EPC_LINK_BITMAP write:: 0x%08x 0x%08x 0x%08x\n",
    //        epc_entry[0],epc_entry[1], epc_entry[2]);

    return SOC_E_NONE;
}

//soc_helix5_ep_flexport_sft_rst_ports
static int
_helix5_ep_flexport_sft_rst_ports(bcmsw_switch_t *bcmsw, int port, int rst_on)
{
    int physical_port;
    uint32 entry;
    int qmode;

    //int port_rst_serviced[HELIX5_PHY_PORTS_PER_DEV];

    if (rst_on == 1) {
        entry = 1;
    } else {
        entry = 0;
    }

    physical_port = bcmsw->si->port_l2p_mapping[port];

    /* For ports going DOWN  or UP do:
     * Assert(rst_on=1)/De-assert(rst_on=0) per port sft reset
     */

    //sal_memset(port_rst_serviced, 0, sizeof(port_rst_serviced));
    /* need to implement skipping edatbuff reset if the buffer is for
     * gport as credits will not be re-issued due flex 
     */
    if (physical_port < 49) {
        qmode = _helix5_get_qmode(bcmsw, physical_port);                           
    } else {
        qmode = 0;
    }

    if ((physical_port < 49) && (qmode)) {
        entry = 0;
    }

    /* If physical_port index was written once don't do it again
     * Note that there may be two writes to the same physical port
     * More sbus efficiency by tracking which phy indexes thar are
     * already written
     */
    //if (0 == port_rst_serviced[physical_port]) {
    //        port_rst_serviced[physical_port] = 1;
    //}

    //soc_mem_field_set(unit, mem, entry, ENABLEf, &memfld);
    _soc_mem_write(bcmsw->dev, EGR_PER_PORT_BUFFER_SFT_RESETm, SCHAN_BLK_EPIPE, 1, &entry); 

    //readback for verification
    entry = 0;
    _soc_mem_write(bcmsw->dev, EGR_PER_PORT_BUFFER_SFT_RESETm, SCHAN_BLK_EPIPE, 1, &entry); 
    //printk("_helix5_ep_flexport_sft_rst_ports port %d entry %d\n", port, entry);

    return SOC_E_NONE;
}

//soc_helix5_ep_enable_disable
static int
_helix5_ep_enable_disable(bcmsw_switch_t *bcmsw, int port, int down_or_up)
{
    uint32 entry;
    int physical_port;

    physical_port = bcmsw->si->port_l2p_mapping[port];

    entry = (0 == down_or_up)?0:1;

    _soc_mem_write(bcmsw->dev, EGR_ENABLEm+physical_port, SCHAN_BLK_EPIPE, 1, &entry); 

    _soc_mem_read(bcmsw->dev, EGR_ENABLEm+physical_port, SCHAN_BLK_EPIPE, 1, &entry); 
    //printk("_helix5_ep_enable_disable port %d entry %d\n", port, entry);

    return SOC_E_NONE;
}

//soc_helix5_flex_ep_port_up
static int
_helix5_flex_ep_port_up(bcmsw_switch_t *bcmsw, int port)
{
    int rst_on, down_or_up;
    int physical_port;

    /*
     * Release EDB port buffer reset and enable cell request generation in EP
     * Set EGR_PER_PORT_BUFFER_SFT_RESET[device_port] to 0
     * Set EGR_ENABLE[device_port].PRT_ENABLE to 1
     */

    /* De-assert PM intf sft_reset */
    rst_on = 0;
    _helix5_ep_flexport_sft_rst_ports(bcmsw, port, rst_on);

    /* Enable Ports going up after PM sft_rst is de-asserted */
    /* For ports going UP do:
     * 1. Enable port; write EGR_ENABLEm
     */
    down_or_up = 1; /* that is, port UP */
    physical_port = bcmsw->si->port_l2p_mapping[port];
    if (-1 != physical_port) { /* that is, port UP */
        /* Enable port; write EGR_ENABLEm */
        _helix5_ep_enable_disable(bcmsw, port, down_or_up);
    }

    return SOC_E_NONE;
}

//soc_helix5_mmu_vbs_port_flush
static int
_helix5_mmu_vbs_port_flush(bcmsw_switch_t *bcmsw, int port, uint64 set_val)
{
    uint32_t reg1, reg2;
    uint64_t enable_val_0,enable_val_1;
    int physical_port;
    int mmu_port, lcl_mmu_port;
    int update0,update1;
    uint64_t new_val_0,new_val_1;
    uint64_t temp64;

    reg1 = Q_SCHED_PORT_FLUSH_SPLIT0r;
    reg2 = Q_SCHED_PORT_FLUSH_SPLIT1r;

    
    /* READ MODIFY WRITE IN SW ... Hence get Register
       Value and Then Write ... */

    COMPILER_64_ZERO(enable_val_0);
    COMPILER_64_ZERO(enable_val_1);
    COMPILER_64_ZERO(temp64);
    
    //acc_type = 20
    _schan_reg64_read(bcmsw->dev, SCHAN_BLK_MMU_SC, reg1, &enable_val_0, 20);

    _schan_reg64_read(bcmsw->dev, SCHAN_BLK_MMU_SC, reg2, &enable_val_1, 20);

    update0 = 0;
    update1 = 0;
    
    COMPILER_64_SET(new_val_0, 0, 1);
    COMPILER_64_SET(new_val_1, 0, 1);

    physical_port = bcmsw->si->port_l2p_mapping[port];
    if (physical_port != -1) {
        mmu_port = bcmsw->si->port_p2m_mapping[physical_port];
    } else {
        printk("_helix5_mmu_vbs_port_flush invalid phy_port - port %d\n",port);
        return 0;
    }

    lcl_mmu_port = mmu_port % HX5_MMU_PORT_PIPE_OFFSET;
    temp64 += HX5_MMU_FLUSH_ON;

    if (lcl_mmu_port < 64) {
        //COMPILER_64_SHL(new_val_0, lcl_mmu_port);
    new_val_0 <<= lcl_mmu_port;

        if (set_val == temp64) {
        if (physical_port == -1) {
                COMPILER_64_OR(enable_val_0, new_val_0);
                update0 = 1;
            }
        } else {
        if (physical_port != -1){
                COMPILER_64_NOT(new_val_0);
            COMPILER_64_AND(enable_val_0, new_val_0);
            update0 = 1;
        }
        }   
    } else {
        //COMPILER_64_SHL(new_val_1, (lcl_mmu_port - 64));
    new_val_1 <<= (lcl_mmu_port - 64);
            
        if (set_val == temp64) {
        if (physical_port == -1){
                COMPILER_64_OR(enable_val_1, new_val_1);
                update1 = 1; 
            }
        } else {
        if (physical_port != -1){
           COMPILER_64_NOT(new_val_1);
           COMPILER_64_AND(enable_val_1, new_val_1);
           update1 = 1; 
        }
        }
    }

    if(update0 == 1) {
         //printk("Q_SCHED_PORT_FLUSH_SPLIT0r 0x%llx\n", enable_val_0);
        _schan_reg64_write(bcmsw->dev, SCHAN_BLK_MMU_SC, reg1, enable_val_0, 20);
    }
    if(update1 == 1) {
        //printk("Q_SCHED_PORT_FLUSH_SPLIT1r 0x%llx\n", enable_val_1);
        _schan_reg64_write(bcmsw->dev, SCHAN_BLK_MMU_SC, reg2, enable_val_1, 20);
    }

    return SOC_E_NONE;
}



//soc_helix5_mmu_rqe_port_flush
static int
_helix5_mmu_rqe_port_flush(bcmsw_switch_t *bcmsw, int port, uint64 set_val)
{
    q_sched_rqe_t reg_rqe;
    int count=0;

    //acc_type = 20
    _schan_reg32_read(bcmsw->dev, SCHAN_BLK_MMU_SC, Q_SCHED_RQE_SNAPSHOTr, &reg_rqe.word, 20);

    reg_rqe.reg.INITIATEf = set_val;
    _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_SC, Q_SCHED_RQE_SNAPSHOTr, reg_rqe.word, 20);

    while (1) {
        _schan_reg32_read(bcmsw->dev, SCHAN_BLK_MMU_SC, Q_SCHED_RQE_SNAPSHOTr, &reg_rqe.word, 20);

        if (reg_rqe.reg.INITIATEf == 0) {
            break;
        }
        msleep(1);
        count++;
        if (count > 60) {
            printk("Initiate isn't reset even after 60ms port %d \n", port);
            return SOC_E_FAIL;
        }
    }
    return SOC_E_NONE;
}

//soc_helix5_mmu_mtro_port_flush
static int
_helix5_mmu_mtro_port_flush(bcmsw_switch_t *bcmsw, int port, uint64 set_val)
{
    uint32_t reg1,reg2;
    int mmu_port, lcl_mmu_port;
    int physical_port;
    uint64_t enable_val_0;
    uint64_t enable_val_1;

    reg1 = MTRO_PORT_ENTITY_DISABLE_SPLIT0r;
    reg2 = MTRO_PORT_ENTITY_DISABLE_SPLIT1r;

    physical_port = bcmsw->si->port_l2p_mapping[port];
    if (physical_port != -1) {
        mmu_port = bcmsw->si->port_p2m_mapping[physical_port];
    } else {
        printk("_helix5_mmu_mtro_port_flush invalid phy_port - port %d\n",port);
        return 0;
    }

    lcl_mmu_port = mmu_port % HX5_MMU_PORT_PIPE_OFFSET;
    
    /* READ MODIFY WRITE IN SW ... Hence get
       Register Value and Then Write ..  */

    if (lcl_mmu_port < 64) {
        //acc_type = 20
        _schan_reg64_read(bcmsw->dev, SCHAN_BLK_MMU_SED, reg1, &enable_val_0, 20);

        if (set_val == 0) {
            COMPILER_64_BITCLR(enable_val_0, lcl_mmu_port);
        }
        else {
            COMPILER_64_BITSET(enable_val_0, lcl_mmu_port);
        }
        _schan_reg64_write(bcmsw->dev, SCHAN_BLK_MMU_SED, reg1, enable_val_0, 20);
    } else {
        //acc_type = 20
        _schan_reg64_read(bcmsw->dev, SCHAN_BLK_MMU_SED, reg2, &enable_val_1, 20);

        if (set_val == 0) {
            COMPILER_64_BITCLR(enable_val_1, lcl_mmu_port - 64);
        }
        else {
            COMPILER_64_BITSET(enable_val_1, lcl_mmu_port - 64);
        }
        _schan_reg64_write(bcmsw->dev, SCHAN_BLK_MMU_SED, reg2, enable_val_1, 20);
    }

    return SOC_E_NONE;
}


//soc_helix5_flex_mmu_port_up_top
static int
_helix5_flex_mmu_port_up_top(bcmsw_switch_t *bcmsw, int port)
{
    int qmode;
    int inst;
    int pipe;
    int pipe_flexed;
    uint64 temp64;
    uint64 rval64;
    int phy_port;

    COMPILER_64_SET(temp64, 0, HX5_MMU_FLUSH_OFF);

    /* Per-Port configuration */
    //for (pipe = 0; pipe < HELIX5_TDM_PIPES_PER_DEV; pipe++) {
    //    soc_helix5_mmu_get_pipe_flexed_status(
    //    unit, port_schedule_state_t, pipe, &pipe_flexed);
        
    //if (pipe_flexed == 1) {
    _helix5_mmu_vbs_port_flush(bcmsw, port, temp64);
    _helix5_mmu_rqe_port_flush(bcmsw, port, temp64);
    //}
    //}

    /* Per-Pipe configuration */
    phy_port = bcmsw->si->port_l2i_mapping[port];

    /* Clear Previous EP Credits on the port. */
    if(phy_port < 49){
        qmode = _helix5_get_qmode(bcmsw, phy_port);
    } else {
        qmode = 0;
    }

    if(qmode == 0 ) { //TODO
        //soc_helix5_mmu_clear_prev_ep_credits(
        //        unit, &port_schedule_state_t->resource[port]);
    }

    _helix5_mmu_mtro_port_flush(bcmsw, port, temp64);
            
    return SOC_E_NONE;
}

/*****************************************************************************************/
/*                             Port Init/Setup                                           */
/*****************************************************************************************/

/*****************************************************************************************/
/*                             Port Init/Setup -- PHY                                    */
/*****************************************************************************************/


static int 
_phyctrl_probe(bcmsw_switch_t *bcmsw, int port)
{
    soc_info_t *si = bcmsw->si;

    // probe for PHY , and dump information
    // Only do ext Phy (N3248TE)
    if (si->ports[port].ext_phy_addr !=  -1) {
        _ext_phy_probe(bcmsw, port);
    }

    return 0;
}

static int _phyctrl_init(bcmsw_switch_t *bcmsw, int port)
{
    soc_info_t *si = bcmsw->si;
    port_info_t *p_port;

    p_port = &si->ports[port];

    //printk("entered soc_phyctrl_init: port %d , probed %d\n", port, p_port->probed);

    if(p_port->probed) {  
       //Call Phy Model specific init -> pd_init = phy_bcm542xx_init
       phy_bcm542xx_init(bcmsw, port);

       p_port->phy_flags |= PHY_FLAGS_INIT_DONE;
    }

    return 0;
}

static int 
_phyctrl_pbm_probe_init(bcmsw_switch_t *bcmsw)
{
    soc_info_t *si = bcmsw->si;
    int port; 
    int num_port = HX5_NUM_PORT;

    for (port = 0; port < num_port; port++) {
        if(si->ports[port].valid == TRUE) {
            _phyctrl_probe(bcmsw, port);

            /* do PHY init pass1 */
            PHYCTRL_INIT_STATE_SET(&(si->ports[port].phy_ctrl), PHYCTRL_INIT_STATE_PASS1);
            _phyctrl_init(bcmsw, port);

            // BCM54182 only need init pass 1
            /* do PHY init pass2 - 5  if requested */
          
            PHYCTRL_INIT_STATE_SET(&(si->ports[port].phy_ctrl), PHYCTRL_INIT_STATE_DEFAULT);
        }
    }
    return 0;
}

/*****************************************************************************************/
/*                             Port Ctrl                                                 */
/*****************************************************************************************/

static int _pm4x10_qtc_port_enable_set(bcmsw_switch_t *bcmsw, int port, int enable)
{
    soc_info_t *si = bcmsw->si;

    port_info_t *pport = &si->ports[port];

    return phy_bcm542xx_enable_set(pport, port, pport->ext_phy_addr, enable);
}


//Get PHY enable state
int _pm4x10_qtc_port_enable_get(bcmsw_switch_t *bcmsw, int port, int *phy_enable)
{
    soc_info_t *si = bcmsw->si;

    port_info_t *pport = &si->ports[port];

    return phy_bcm542xx_enable_get(pport,port, pport->ext_phy_addr, phy_enable);
}

int _pm4x10_qtc_port_link_get(bcmsw_switch_t *bcmsw, int port, int *up)
{
    soc_info_t *si = bcmsw->si;

    port_info_t *pport = &si->ports[port];

    return phy_bcm542xx_link_get(pport, port, pport->ext_phy_addr, up);
}

int _pm4x10_qtc_port_autoneg_get(bcmsw_switch_t *bcmsw, int port, phymod_autoneg_control_t *an)
{
    int autoneg, autoneg_done;
    soc_info_t *si = bcmsw->si;

    port_info_t *pport = &si->ports[port];

    phy_bcm542xx_autoneg_get(pport, port, pport->ext_phy_addr,&autoneg, &autoneg_done);

    an->enable = autoneg;

    return 0;
}

static int 
_bcm_esw_portctrl_enable_set(bcmsw_switch_t *bcmsw, int port, int enable)
{
    int mac_reset;
    //if (flags & PORTMOD_PORT_ENABLE_PHY) {
        //portmod_port_enable_set();
        _pm4x10_qtc_port_enable_set(bcmsw, port, 1);
    //}

    //portmod_ext_to_int_cmd_set() ==> Not needed

    //_soc_link_update
    //Check if MAC needs to be modified based on whether
    //(portmod_port_mac_reset_check(unit, pport,
    //    enable, &mac_reset));
    unimac_reset_check(bcmsw, port, enable, &mac_reset);

    if(mac_reset) {
        //if (flags & PORTMOD_PORT_ENABLE_MAC ) {
        //(BCM_ESW_PORT_DRV(unit)->port_enable_set(unit, -> bcmi_hx5_port_enable ->soc_helix5_flex_top_port_up
        printk("_bcm_esw_portctrl_enable_set port %d mac_reset\n", port);
        
        /* soc_helix5_flex_top_port_up*/
        // 1 soc_helix5_flex_mmu_port_up_top
        _helix5_flex_mmu_port_up_top(bcmsw, port);

        // 2 soc_helix5_flex_ep_port_up
        _helix5_flex_ep_port_up(bcmsw, port);

        // 3 soc_helix5_flex_idb_port_up()
        _helix5_flex_idb_port_up(bcmsw, port);
 
        // 4 soc_helix5_flex_mac_port_up
        _helix5_flex_mac_port_up(bcmsw, port);

        // 5 soc_helix5_flex_en_forwarding_traffic
        _helix5_flex_en_forwarding_traffic(bcmsw, port);

    }

    return 0;
}


//bcmi_esw_portctrl_probe(PORTMOD_PORT_ADD_F_INIT_CORE_PROBE)
static int 
bcmi_esw_portctrl_probe_init(bcmsw_switch_t *bcmsw, int port)
{

    /* Add port to PM */
//portmod_xphy_lane_detach(unit, physical_port+lane, 1);
//PORT_UNLOCK(unit);
//PORTMOD_PBMP_PORT_ADD(p_pbmp, physical_port+lane);
//rv = soc_esw_portctrl_setup_ext_phy_add(unit, port, &p_pbmp);

//rv = soc_esw_portctrl_add(unit, port, init_flag, add_info); -> portmod_port_add -> _pm4x10_qtc_port_attach
   //_pm4x10_qtc_port_attach -> _pm4x10_qtc_port_attach_core_probe

   _pm4x10_qtc_port_attach_core_probe(bcmsw, port);

   return 0;
}


//bcmi_esw_portctrl_probe(PORTMOD_PORT_ADD_F_INIT_PASS1)
static int 
bcmi_esw_portctrl_probe_pass1(bcmsw_switch_t *bcmsw, int port)
{
    _pm4x10_qtc_pm_core_init(bcmsw, port);

   return 0;
}

//bcmi_esw_portctrl_probe(PORTMOD_PORT_ADD_F_INIT_PASS2)
static int 
bcmi_esw_portctrl_probe_pass2(bcmsw_switch_t *bcmsw, int port)
{

    // _pm4x10_qtc_port_attach ->
    _pm4x10_qtc_port_attach_resume_fw_load(bcmsw, port);

    /* Probe function should leave port disabled */
    //rv = _bcm_esw_portctrl_enable_set(unit, port, pport,
    //    PORTMOD_PORT_ENABLE_PHY, 0);

   return 0;
}

//bcm_esw_port_probe->bcmi_esw_portctrl_probe_pbmp
static int
bcmi_esw_portctrl_probe_pbmp(bcmsw_switch_t *bcmsw)
{
    int index;


    //start with Front Panel ports
    for (index = 1; index <=48; index++) {
        /*step1: probe Serdes and external PHY core*/
        bcmi_esw_portctrl_probe_init(bcmsw, index);

        /*step2 : initialize PASS1 for SerDes and external PHY*/
        //bcmi_esw_portctrl_probe(PORTMOD_PORT_ADD_F_INIT_PASS1
        bcmi_esw_portctrl_probe_pass1(bcmsw, index);

        /* step3:broadcast firmware download for all external phys inculde legacy and Phymod PHYs*/

        /*step4:initialize PASS2 for Serdes and external PHY*/
        bcmi_esw_portctrl_probe_pass2(bcmsw, index);



        //_bcm_esw_portctrl_enable_set(unit, port, pport,PORTMOD_PORT_ENABLE_MAC, FALSE);

        msleep(1);
    }
    return 0;
}


/*****************************************************************************************/
/*                             Port Init/Setup                                           */
/*****************************************************************************************/
static int
_bcm_port_speed_set(bcmsw_switch_t *bcmsw, int port, int speed)
{
    //bcmi_esw_portctrl_speed_set
    //if (enable == TRUE) {
    // disable MAC and PHY
    //bcmi_esw_portctrl_enable_set

    /* disable AN */
    // _bcm_esw_portctrl_disable_autoneg

    //reconfigure chip
    //_bcm_esw_portctrl_speed_chip_reconfigure

    //Restore port's enable state based on what was read prior to setting speed 

    return 0;
}

//Setting the speed for a given port
int
bcm_esw_port_speed_set(bcmsw_switch_t *bcmsw, int port, int speed)
{
    int rv;
    rv = _bcm_port_speed_set(bcmsw, port, speed);

    //bcm_esw_port_enable_set
    //bcm_esw_link_change
    //_bcm_esw_port_link_delay_update

    return rv;
}
int bcm_esw_port_autoneg_set(bcmsw_switch_t *bcmsw, int port, int speed)
{
    //bcmi_esw_portctrl_autoneg_set
#if 0
    _bcm_esw_port_gport_phyn_validate(unit, port,
        &local_port, &phyn,
        &phy_lane, &sys_side));

    if (local_port != -1) {
        port = local_port;
    }

    if (local_port == -1) {
        /* Configure outermost PHY (common case) */
        rv = portmod_port_autoneg_set(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an);
    } else {
        /* Configure PHY specified by GPORT */
        rv = portmod_port_redirect_autoneg_set(unit, pport, phyn,
                                               phy_lane, sys_side, &an);
    }
#endif   
    return 0;                                            
}

int
bcm_esw_port_enable_get(bcmsw_switch_t *bcmsw, int port, int *enable)
{
    //bcmi_esw_portctrl_enable_get

    //portmod_port_enable_get(unit, pport, PORTMOD_PORT_ENABLE_PHY, enable);

    return _pm4x10_qtc_port_enable_get(bcmsw, port, enable);

}
int
bcm_esw_port_link_status_get(bcmsw_switch_t *bcmsw, int port, int *up)
{
    //bcmi_esw_portctrl_enable_get

    //portmod_port_enable_get(unit, pport, PORTMOD_PORT_ENABLE_PHY, enable);

    if (port>= 1 && port <=48 ) {
        return _pm4x10_qtc_port_link_get(bcmsw, port, up);
    }

    //TODO
    *up = 0;
    return 0;

}

int
bcm_esw_port_autoneg_get(bcmsw_switch_t *bcmsw, int port, int *autoneg)
{
    phymod_autoneg_control_t an;

    memset(&an, 0, sizeof(an));
    
    _pm4x10_qtc_port_autoneg_get(bcmsw, port, &an);

    *autoneg = an.enable;

    return 0;
}



int
bcm_esw_port_enable_set(bcmsw_switch_t *bcmsw, int port, int enable)
{
   //bcmi_esw_portctrl_enable_set
   //if (enable) {
      // enable PHY
      //rv = _bcm_esw_portctrl_enable_set(bcmsw, port, PORTMOD_PORT_ENABLE_PHY, TRUE);

      /* Get link status after PHY state has been set */
      //rv = bcm_esw_port_link_status_get(unit, port, &link);

      // enable MAC
      //rv = _bcm_esw_portctrl_enable_set(bcmsw, port, PORTMOD_PORT_ENABLE_MAC, TRUE);
   //} else {


   //}
   return _bcm_esw_portctrl_enable_set(bcmsw, port, enable);
}

static int
bcm_port_settings_init(bcmsw_switch_t *bcmsw, int port)
{
    int         rc;
    soc_info_t *si = bcmsw->si;

    rc = bcm_esw_port_speed_set(bcmsw, port, si->port_init_speed[port]);

    //val = soc_property_port_get(unit, port, spn_PORT_INIT_DUPLEX, -1);
    //if (val != -1) {
    //    info.duplex = val;
    //    info.action_mask |= BCM_PORT_ATTR_DUPLEX_MASK;
    //}
    //bcm_esw_port_duplex_set(unit, port, info->duplex);
    

    //val = soc_property_port_get(unit, port, spn_PORT_INIT_ADV, -1);
    //if (val != -1) {
    //    info.local_advert = val;
    //    info.action_mask |= BCM_PORT_ATTR_LOCAL_ADVERT_MASK;
    //}
    //bcm_esw_port_advert_set(unit, port, info->local_advert);

    rc = bcm_esw_port_autoneg_set(bcmsw, port, TRUE);

    return rc;
}


//bcm_esw_port_init
static int 
_port_init(bcmsw_switch_t *bcmsw)
{
    int num_port = HX5_NUM_PORT, port, vid; 
    soc_info_t *si = bcmsw->si;
    int index;

    vid = BCMSW_VLAN_DEFAULT;
    for (port = 0; port < num_port; port++) {
       if(si->port_type[port] != -1) {
           //bcm_td3_port_cfg_init
           _port_cfg_init(bcmsw, port, vid);
       }
    }
    // STEP 1 
    /* 
     * Initialize inner TPID
     * (WAR)Set to 0x9100 then change back to take effort
     */
    //TODO

    // STEP 2 
    // clear egress port blocking table MAC_BLOCKm
    for (index = 0; index <= 31; index++) {
        _soc_mem_write(bcmsw->dev, MAC_BLOCKm+index, 
                       SCHAN_BLK_IPIPE, BYTES2WORDS(MAC_BLOCKm_BYTES), (uint32_t *)empty_entry); 
    }    

    // STEP 3 - Probe for Ports -> bcm_esw_port_probe
    /* Probe the PHY and set up the PHY and MAC for the specified ports.
     * bcm_esw_port_probe(unit, PBMP_PORT_ALL(unit), &okay_ports);
     */
    /* Check for PortMod */
    bcmi_esw_portctrl_probe_pbmp(bcmsw);

    /*soc_phyctrl_pbm_probe_init */
    _phyctrl_pbm_probe_init(bcmsw);

    // Probe function should leave port disabled 
    //soc_phyctrl_enable_set

    // STEP 4
    for (port = 0; port < num_port; port++) {
        if(si->ports[port].valid == TRUE) {
            bcm_port_settings_init(bcmsw, port);
        }
     }    

    //enable ports
    // if ((rv = bcm_esw_port_enable_set(unit, p, port_enable)) < 0) {
    for (port =1; port < num_port; port++) {
        if(si->ports[port].valid == TRUE) {
            bcm_esw_port_enable_set(bcmsw, port, TRUE);
        }
    }

    //bcm_esw_port_tpid_set

    //bcm_esw_port_learn_set

    //_bcm_port_remap_set

    return 0;
}


static int bcmsw_port_create(bcmsw_switch_t *bcmsw, int port_index, const char *name)
{
    int len;
    //int qnum;
    kcom_msg_netif_create_t netif_create;
    //struct bcmsw_port * port = bcmsw->ports[port_index];

    memset(&netif_create, 0, sizeof(netif_create));
    netif_create.hdr.type = KCOM_MSG_TYPE_CMD;
    netif_create.hdr.seqno = 0;
    netif_create.hdr.opcode = KCOM_M_NETIF_CREATE;
    netif_create.hdr.unit = 0;

    netif_create.netif.type = KCOM_NETIF_T_VLAN;

    /*    if (netif->flags & BCM_KNET_NETIF_F_ADD_TAG) {
        netif_create.netif.flags |= KCOM_NETIF_F_ADD_TAG;
    }
    if (netif->flags & BCM_KNET_NETIF_F_RCPU_ENCAP) {
        netif_create.netif.flags |= KCOM_NETIF_F_RCPU_ENCAP;
    }
    if (netif->flags & BCM_KNET_NETIF_F_KEEP_RX_TAG) {
        netif_create.netif.flags |= KCOM_NETIF_F_KEEP_RX_TAG;
    }
    */
    //netif_create.netif.cb_user_data = netif->cb_user_data;
    netif_create.netif.vlan = BCMSW_VLAN_DEFAULT; //netif->vlan;
    netif_create.netif.port = port_index;
    netif_create.netif.phys_port = bcmsw->si->port_l2p_mapping[port_index];

    //if (BCM_SUCCESS(soc_esw_hw_qnum_get(unit, netif->port, netif->cosq, &qnum))) {
    //    netif_create.netif.qnum = qnum;
    //}
    //memcpy(netif_create.netif.macaddr, netif->mac_addr, 6);
    memcpy(netif_create.netif.name, name,
               sizeof(netif_create.netif.name) - 1);

    len = bkn_handle_cmd_req((kcom_msg_t *)&netif_create, sizeof(netif_create));

    //handle response
    //if (len > 0) {
    //    /* ID and interface name are assigned by kernel */
    //    port->id = netif_create.netif.id;
    //    memcpy(port->name, netif_create.netif.name, KCOM_NETIF_NAME_MAX - 1);
    //}

    return 0;
}



static int bcmsw_ports_init(bcmsw_switch_t *bcmsw)
{
    //FIX: hardcode for N3248TE  BCM56371
    // soc_helix5_port_config_init
    // bcm_td3_port_cfg_init
    // bcm_esw_port_probe
    // bcm_esw_port_enable_set
    // _bcm_td_port_lanes_set
    //bcm_esw_knet_netif_create
    int max_ports;
    int err;
    int i;

    // initialize port configuration according to soc info. bcm_esw_port_init bcm_td3_port_cfg_init
    err = _port_init(bcmsw);
    if (err) {
        goto err_port_cfg_init;
    }

    //call knet to create interface
    max_ports = COUNTOF(n3248te_ports);
    for (i = 0; i < max_ports; i++) {
        if (n3248te_ports[i].port != -1) {
          err = bcmsw_port_create(bcmsw, n3248te_ports[i].port, n3248te_ports[i].name);
          if (err)
            goto err_port_create;
        }
    }
    
err_port_cfg_init:
err_port_create:
    return err;
}

//bcm_esw_port_encap_get->bcmi_esw_portctrl_encap_get
static int 
_esw_port_encap_get(bcmsw_switch_t *bcmsw, int port, int *encap)
{
    // port 1 - 48 unimac_encap_get
    *encap = SOC_ENCAP_IEEE;
    //

    return SOC_E_NONE;
}



/*****************************************************************************************/
/*                             Spanning Tree (STG)                                       */
/*****************************************************************************************/
static int
_bcm_xgs3_stg_stp_init_stg(bcmsw_switch_t *bcmsw, bcm_stg_t stg)
{
    uint32_t entry[SOC_MAX_MEM_WORDS];    /* Spanning tree port state map. */
 
    //int stp_state;              /* STP port state.               */
    //bcm_pbmp_t stacked;         /* Bitmap of stacked ports.      */
    //bcm_port_t port;            /* Port iterator.                */

    /* Set all ports to PVP_STP_DISABLED */
    memset(entry, 0, sizeof(entry));

#if 0   
    /* Get all stacking ports and set them into forwarding */
    BCM_PBMP_ASSIGN(stacked, PBMP_ST_ALL(unit));
    BCM_PBMP_OR(stacked, SOC_PBMP_STACK_CURRENT(unit));
    stp_state = PVP_STP_FORWARDING;


    if (!SOC_IS_TOMAHAWK3(unit)) {
        /* Get all stacking ports and set them into forwarding */
        BCM_PBMP_ASSIGN(stacked, PBMP_ST_ALL(unit));
        BCM_PBMP_OR(stacked, SOC_PBMP_STACK_CURRENT(unit));

    /* Iterate over stacking ports & set stp state. */
    PBMP_ITER(stacked, port) {
        entry[STG_WORD(port)] |= stp_state << STG_BITS_SHIFT(port);
    }
    }

    /* Write spanning tree group port states to hw. */
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, mem, MEM_BLOCK_ALL, stg, entry));

    if ((BCM_STG_DEFAULT == stg) &&
        soc_feature(unit, soc_feature_vlan_vfi_membership)) {

        /* Initialize the STP state for 'PVP_STP_FORWARDING' for all
           groups on default STG */
        BCM_IF_ERROR_RETURN(bcm_td2p_vp_group_stp_init(
            unit, stg, (mem == EGR_VLAN_STGm), stp_state));
    }

#endif
    if (BCM_STG_DEFAULT == stg) {
        entry[0] = 0xfffffffc;
        entry[1] = 0xffffffff;
        entry[2] = 0xffffffff;
        entry[3] = 0x000c0fff;
        entry[4] = 0xffffc000;
        entry[5] = 0xffffffff;
        entry[6] = 0xffffffff;
        entry[7] = 0xffffffff;
        entry[8] = 0x0000ffff;
        entry[9] = 0x00000000;
        entry[10] = 0x00000000;
    }
    
    _soc_mem_write(bcmsw->dev, STG_TABm+stg,
                   SCHAN_BLK_IPIPE, 
                   BYTES2WORDS(STG_TABm_BYTES), 
                   entry); 
    
    return (SOC_E_NONE);
}

static int
_bcm_xgs3_stg_stp_init_egr(bcmsw_switch_t *bcmsw, bcm_stg_t stg)
{
    uint32 entry[SOC_MAX_MEM_WORDS];    /* Spanning tree port state map. */

    /* Set all ports to PVP_STP_DISABLED */
    memset(entry, 0, sizeof(entry));

    if (BCM_STG_DEFAULT == stg) {
        entry[0] = 0xfffffffc;
        entry[1] = 0xffffffff;
        entry[2] = 0xffffffff;
        entry[3] = 0x000c0fff;
        entry[4] = 0xffffc000;
        entry[5] = 0xffffffff;
        entry[6] = 0xffffffff;
        entry[7] = 0xffffffff;
        entry[8] = 0x0000ffff;
        entry[9] = 0x00000000;
    }
    
    _soc_mem_write(bcmsw->dev, EGR_VLAN_STGm+stg,
                   SCHAN_BLK_EPIPE, 
                   BYTES2WORDS(EGR_VLAN_STGm_BYTES), 
                   entry); 
    
    return (SOC_E_NONE);
}


static int
_bcm_stg_pvp_translate(int pvp_state, int *bcm_state)
{
    if (NULL == bcm_state) {
        return (SOC_E_PARAM);
    }

    switch (pvp_state) {
      case PVP_STP_FORWARDING:
          *bcm_state = BCM_STG_STP_FORWARD;
          break;
      case PVP_STP_BLOCKING:
          *bcm_state = BCM_STG_STP_BLOCK;
          break;
      case PVP_STP_LEARNING:
          *bcm_state = BCM_STG_STP_LEARN;
          break;
      case PVP_STP_DISABLED:
          *bcm_state = BCM_STG_STP_DISABLE;
          break;
      default:
          return (SOC_E_INTERNAL);
    }
    return (SOC_E_NONE);
}


static int
_bcm_xgs3_stg_stp_get(bcmsw_switch_t *bcmsw, bcm_stg_t stg, int port,
                      int *stp_state)
{
    uint32 entry[SOC_MAX_MEM_WORDS];    /* STP group ports state map.   */
    int hw_stp_state;                   /* STP port state in hw format. */
    int rv;                             /* Operation return status.     */

  
    memset(entry, 0, sizeof(entry));

    rv=  _soc_mem_read(bcmsw->dev, STG_TABm+stg,
                  SCHAN_BLK_IPIPE, 
                  BYTES2WORDS(STG_TABm_BYTES), 
                  entry); 

    /* Get specific port state from the entry. */
    //printk("_bcm_xgs3_stg_stp_get stg %d port %d entry 0x%08x\n",stg, port, entry[STG_WORD(port)]);;

    hw_stp_state = entry[STG_WORD(port)] >> STG_BITS_SHIFT(port);
    hw_stp_state &= STG_PORT_MASK;

    //printk("_bcm_xgs3_stg_stp_get port %d hw_state %d\n", port, hw_stp_state);

    /* Translate hw stp port state to API format. */
    rv = _bcm_stg_pvp_translate(hw_stp_state, stp_state);

    return (SOC_E_NONE);
}

int
bcm_xgs3_stg_stp_init(bcmsw_switch_t *bcmsw, bcm_stg_t stg)
{
    // init STG_TABm
    _bcm_xgs3_stg_stp_init_stg(bcmsw, stg);

    // init EGR_VLAN_STGms
    _bcm_xgs3_stg_stp_init_egr(bcmsw, stg);

    return (SOC_E_NONE);
}


//Get the Spanning tree state for a port in specified STG.
int
bcm_esw_stg_stp_get(bcmsw_switch_t *bcmsw, bcm_stg_t stg, int port, int *stp_state)
{
    bcm_stg_info_t      *stg_info = bcmsw->stg_info;
    int                      rv = SOC_E_NONE;
    uint32_t        val;


    if(!stg_info) {
       return SOC_E_INTERNAL;
    }

    if(stg < 0 || stg > stg_info->stg_max) {
        return SOC_E_PARAM;
    }

    //if (!STG_BITMAP_TST(si, stg)) {
    val = 0;
    _mem_field_get(stg_info->stg_bitmap,(STG_TABm_MAX_INDEX+1)/8, stg, 1, &val, 0 );
    if (!val) {
        rv = SOC_E_NOT_FOUND;
        goto cleanup;
    }

    rv = _bcm_xgs3_stg_stp_get(bcmsw,stg, port, stp_state);

cleanup:
    return rv;
}

int
bcm_esw_port_stp_get(bcmsw_switch_t *bcmsw, int port, int *stp_state)
{
    bcm_stg_info_t      *stg_info = bcmsw->stg_info;

    int              rv;

    if (stg_info->stg_defl >= 0) {
        rv = bcm_esw_stg_stp_get(bcmsw, stg_info->stg_defl, port, stp_state);
    } else {  
        *stp_state = BCM_STG_STP_FORWARD;
        rv = SOC_E_NONE;
    }

    //printk("bcm_port_stp_get: p=%d state=%d rv=%d\n",
    //       port, *stp_state, rv);

    return rv;
}

int
bcm_esw_stg_create_id(bcmsw_switch_t *bcmsw, bcm_stg_t stg)
{
    bcm_stg_info_t      *stg_info = bcmsw->stg_info;
    int                      rv = SOC_E_NONE;
    uint32_t        val;

    //if (STG_BITMAP_TST(si, stg)) 
    val = 0;
    _mem_field_get(stg_info->stg_bitmap,(STG_TABm_MAX_INDEX+1)/8, stg, 1, &val, 0 );
    if (val) {
        return SOC_E_EXISTS;
    }

    /* Write an entry with all ports DISABLED */
    //bcm_xgs3_stg_stp_init
    bcm_xgs3_stg_stp_init(bcmsw, stg);


    //STG_BITMAP_SET(si, stg);
    val = 1;
    _mem_field_set(stg_info->stg_bitmap,(STG_TABm_MAX_INDEX+1)/8, stg, 1, &val, 0 );

    stg_info->stg_count++;


    return rv;
}

int
bcm_esw_stg_init(bcmsw_switch_t *bcmsw)
{
    bcm_stg_info_t      *stg_info;
    uint32_t         val;
    //int                  alloc_size, idx;
    //int vlan_vfi_count;    

    stg_info = kmalloc(sizeof(bcm_stg_info_t), GFP_KERNEL); 
    memset(stg_info, 0, sizeof(bcm_stg_info_t));
    bcmsw->stg_info = stg_info;

    stg_info->stg_min = 1;
    stg_info->stg_max = STG_TABm_MAX_INDEX;
 
    /*
     * For practical ease of use, the initial default STG is always 1
     * since that ID is valid on all chips.
     */

    stg_info->stg_defl = BCM_STG_DEFAULT; 

    /*
     * XGS switches have a special STG 0 entry that is used
     * only for tagged packets with invalid VLAN.  XGS hardware does not
     * automatically initialize it to all 1s.
     * STG is marked valid so the bcm_stg_stp_set/get APIs can be used
     * to manage entry 0, but this is generally for internal purposes.
     */
    // /bcm_xgs3_stg_stp_init
    //BCM_IF_ERROR_RETURN(mbcm_driver[unit]->mbcm_stg_stp_init(unit, 0));
    bcm_xgs3_stg_stp_init(bcmsw, 0);
    //set bit 0
    val = 1;
    _mem_field_set(stg_info->stg_bitmap,(STG_TABm_MAX_INDEX+1)/8, 0, 1, &val, 0 );

     /*
      * Create default STG and add all VLANs to it.  Calling this routine
      * is safe because it does not reference other BCM driver modules.
      */
 
     //BCM_IF_ERROR_RETURN(bcm_esw_stg_create_id(unit, si->stg_defl));
     bcm_esw_stg_create_id(bcmsw, stg_info->stg_defl);
 
     //_bcm_stg_map_add(unit, si->stg_defl, BCM_VLAN_DEFAULT);
 
     return SOC_E_NONE;     
}


/*****************************************************************************************/
/*                             port info                                                 */
/*****************************************************************************************/

int
bcm_td3_port_cfg_get(bcmsw_switch_t *bcmsw, int port, bcm_port_cfg_t *port_cfg)
{
    /* Input parameters check.*/
    if (NULL == port_cfg) {
        return (SOC_E_PARAM);
    }

    memset(port_cfg, 0, sizeof(bcm_port_cfg_t));

    /* Read port table entry. */
    //_bcm_td3_port_table_read(bcmsw, port, port_cfg);

    return(SOC_E_NONE);
}

int
bcm_esw_port_learn_get(bcmsw_switch_t *bcmsw, int port, uint32 *flags)
{
    bcm_port_cfg_t      pcfg;
    int                 rv;

    if (flags == NULL) {
        return SOC_E_PARAM;
    }


    //rv = mbcm_driver[unit]->mbcm_port_cfg_get(unit, port, &pcfg); 
    // -> bcm_td3_port_cfg_get
    pcfg.pc_cpu = 0;

    switch (pcfg.pc_cml) {
        case PVP_CML_SWITCH:
            *flags = (BCM_PORT_LEARN_ARL |
                      BCM_PORT_LEARN_FWD |
                      (pcfg.pc_cpu ? BCM_PORT_LEARN_CPU : 0));
            break;
        case PVP_CML_CPU:
            *flags = BCM_PORT_LEARN_CPU;
            break;
        case PVP_CML_FORWARD:
            *flags = BCM_PORT_LEARN_FWD;
            break;
        case PVP_CML_DROP:
            *flags = 0;
            break;
        case PVP_CML_CPU_SWITCH:
            *flags = (BCM_PORT_LEARN_ARL |
                      BCM_PORT_LEARN_CPU |
                      BCM_PORT_LEARN_FWD);
            break;
        case PVP_CML_CPU_FORWARD:
            *flags = BCM_PORT_LEARN_CPU | BCM_PORT_LEARN_FWD;
            break;
        default:
            return SOC_E_INTERNAL;
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *      bcm_port_selective_get
 * Purpose:
 *      Get requested port parameters
 * Parameters:
 *      unit - switch Unit
 *      port - switch port
 *      info - (IN/OUT) port information structure
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      The action_mask field of the info argument is used as an input
 */

 int
 bcm_esw_port_selective_get(bcmsw_switch_t *bcmsw, int port, bcm_port_info_t *info)
 {
    int                 rc;
    uint32_t             mask;
    int                 speed;
 
    mask = info->action_mask;

    if (mask & BCM_PORT_ATTR_ENCAP_MASK) {
        rc = _esw_port_encap_get(bcmsw, port, &info->encap_mode);
    }
 
    if (mask & BCM_PORT_ATTR_ENABLE_MASK) {
        rc = bcm_esw_port_enable_get(bcmsw, port, &info->enable);
    }
 

    if (mask & BCM_PORT_ATTR_LINKSTAT_MASK) {
        rc = bcm_esw_port_link_status_get(bcmsw, port, &info->linkstatus);
    }

     if (mask & BCM_PORT_ATTR_AUTONEG_MASK) {
        rc = bcm_esw_port_autoneg_get(bcmsw, port, &info->autoneg);
     }

     info->speed = 1000;
     info->phy_master = 1;
# if 0 
     if (mask & BCM_PORT_ATTR_LOCAL_ADVERT_MASK) {
         r = bcm_esw_port_ability_advert_get(unit, port,
                                             &info->local_ability);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_ability_advert_getfailed:%s\n"),
                          bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
         r = soc_port_ability_to_mode(&info->local_ability,
                                      &info->local_advert);
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_REMOTE_ADVERT_MASK) {
 
         if ((r = bcm_esw_port_ability_remote_get(unit, port,
                                                  &info->remote_ability)) < 0) {
             info->remote_advert = 0;
             info->remote_advert_valid = FALSE;
         } else {
             r = soc_port_ability_to_mode(&info->remote_ability,
                                          &info->remote_advert);
             BCM_IF_ERROR_RETURN(r);
             info->remote_advert_valid = TRUE;
         }
     }
 
     if (mask & BCM_PORT_ATTR_SPEED_MASK) {
         if ((r = bcm_esw_port_speed_get(unit, port, &info->speed)) < 0) {
             if (r != BCM_E_BUSY) {
                 LOG_VERBOSE(BSL_LS_BCM_PORT,
                             (BSL_META_U(unit,
                                         "bcm_port_speed_get failed: %s\n"), bcm_errmsg(r)));
                 return(r);
             } else {
                 info->speed = 0;
             }
         }
     }
 
     if (mask & BCM_PORT_ATTR_DUPLEX_MASK) {
         if ((r = bcm_esw_port_duplex_get(unit, port, &info->duplex)) < 0) {
             if (r != BCM_E_BUSY) {
                 LOG_VERBOSE(BSL_LS_BCM_PORT,
                             (BSL_META_U(unit,
                                         "bcm_port_duplex_get failed: %s\n"), bcm_errmsg(r)));
                 return r;
             } else {
                 info->duplex = 0;
             }
         }
     }
 
     /* get both if either mask bit set */
     if (mask & (BCM_PORT_ATTR_PAUSE_TX_MASK |
                 BCM_PORT_ATTR_PAUSE_RX_MASK)) {
         r = bcm_esw_port_pause_get(unit, port,
                                    &info->pause_tx, &info->pause_rx);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_pause_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_PAUSE_MAC_MASK) {
         r = bcm_esw_port_pause_addr_get(unit, port, info->pause_mac);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_pause_addr_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_LINKSCAN_MASK) {
         r = bcm_esw_port_linkscan_get(unit, port, &info->linkscan);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_linkscan_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_LEARN_MASK) {
         r = bcm_esw_port_learn_get(unit, port, &info->learn);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_learn_getfailed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_DISCARD_MASK) {
         r = bcm_esw_port_discard_get(unit, port, &info->discard);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_discard_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_VLANFILTER_MASK) {
         r = bcm_esw_port_vlan_member_get(unit, port, &info->vlanfilter);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_esw_port_vlan_member_get failed:%s\n"),
                          bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_UNTAG_PRI_MASK) {
         r = bcm_esw_port_untagged_priority_get(unit, port,
                                                &info->untagged_priority);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_untagged_priority_get failed:%s\n"),
                          bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_UNTAG_VLAN_MASK) {
         r = bcm_esw_port_untagged_vlan_get(unit, port,
                                            &info->untagged_vlan);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_untagged_vlan_get failed:%s\n"),
                          bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
#endif         
 
     if (mask & BCM_PORT_ATTR_STP_STATE_MASK) {
        rc = bcm_esw_port_stp_get(bcmsw, port, &info->stp_state);
     }
 
#if 0     
     if (mask & BCM_PORT_ATTR_PFM_MASK) {
         r = bcm_esw_port_pfm_get(unit, port, &info->pfm);
         if (r != BCM_E_UNAVAIL) {
             if (BCM_FAILURE(r)) {
                 LOG_VERBOSE(BSL_LS_BCM_PORT,
                             (BSL_META_U(unit,
                                         "bcm_port_pfm_get failed:%s\n"), bcm_errmsg(r)));
             }
         }
         BCM_IF_ERROR_NOT_UNAVAIL_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_LOOPBACK_MASK) {
         r = bcm_esw_port_loopback_get(unit, port, &info->loopback);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_loopback_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_PHY_MASTER_MASK) {
         r = bcm_esw_port_master_get(unit, port, &info->phy_master);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_master_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_INTERFACE_MASK) {
         r = bcm_esw_port_interface_get(unit, port, &info->interface);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_interface_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_RATE_MCAST_MASK) {
         r = bcm_esw_rate_mcast_get(unit, &info->mcast_limit,
                                    &info->mcast_limit_enable, port);
         if (r == BCM_E_UNAVAIL) {
             r = BCM_E_NONE;     /* Ignore if not supported on chip */
         }
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_rate_mcast_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_RATE_BCAST_MASK) {
         r = bcm_esw_rate_bcast_get(unit, &info->bcast_limit,
                                    &info->bcast_limit_enable, port);
         if (r == BCM_E_UNAVAIL) {
             r = BCM_E_NONE;     /* Ignore if not supported on chip */
         }
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_rate_bcast_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_RATE_DLFBC_MASK) {
         r = bcm_esw_rate_dlfbc_get(unit, &info->dlfbc_limit,
                                    &info->dlfbc_limit_enable, port);
         if (r == BCM_E_UNAVAIL) {
             r = BCM_E_NONE;     /* Ignore if not supported on chip */
         }
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_rate_dlfbc_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_SPEED_MAX_MASK) {
         r = bcm_esw_port_speed_max(unit, port, &info->speed_max);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_speed_max failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_ABILITY_MASK) {
         r = bcm_esw_port_ability_local_get(unit, port, &info->port_ability);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_ability_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
         r = soc_port_ability_to_mode(&info->port_ability,
                                      &info->ability);
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_FRAME_MAX_MASK) {
         r = bcm_esw_port_frame_max_get(unit, port, &info->frame_max);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_frame_max_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_MDIX_MASK) {
         r = bcm_esw_port_mdix_get(unit, port, &info->mdix);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_mdix_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_MDIX_STATUS_MASK) {
         r = bcm_esw_port_mdix_status_get(unit, port, &info->mdix_status);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_mdix_status_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_MEDIUM_MASK) {
         r = bcm_esw_port_medium_get(unit, port, &info->medium);
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_medium_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 
     if (mask & BCM_PORT_ATTR_FAULT_MASK) {
         r = bcm_esw_port_fault_get(unit, port, &info->fault);
         if (r == BCM_E_PORT) {
             r = BCM_E_NONE;     /* Ignore if not supported on chip/port */
         }
         if (BCM_FAILURE(r)) {
             LOG_VERBOSE(BSL_LS_BCM_PORT,
                         (BSL_META_U(unit,
                                     "bcm_port_fault_get failed:%s\n"), bcm_errmsg(r)));
         }
         BCM_IF_ERROR_RETURN(r);
     }
 #endif
     return rc;
 }

static int
bcm_esw_port_info_get(bcmsw_switch_t *bcmsw, int port, bcm_port_info_t *info)
{
    if (info != NULL) {
        memset(info, 0, sizeof(bcm_port_info_t));
    } else {
        return -1;
    }

    info->action_mask = BCM_PORT_ATTR_ALL_MASK;

    return bcm_esw_port_selective_get(bcmsw, port, info);
}


/*****************************************************************************************/
/*                             switch                                                    */
/*****************************************************************************************/
//soc_cmicx_dma_abort
#if 0
static int bcmsw_cmicx_dma_abort(bcmsw_switch_t *bcmsw)
{

        /* abort s-bus DMA in all channels */
    chans_group_init(&sbus_channels);
    if ((flags & SOC_DMA_ABORT_SKIP_SBUS) == 0) {
        /* For  CMCs x=0-1  and channels y=0-3 and [channel used by this CPU] */
        for (cmc = 0; cmc < cmc_num_max; ++cmc) {
            for (chan = 0; chan < sbusdma_chan_max; ++chan) {
                cmic_address = CMIC_CMCx_SBUSDMA_CHy_CONTROL(cmc, chan);
                /* if CMIC_CMCx_SBUSDMA_CHy_CONTROL.START == 1 */
                val = soc_pci_read(unit, cmic_address);
                if (val & CMIC_CMCx_SBUSDMA_CHy_CONTROL_START) {
                    /* set CMIC_CMCx_SBUSDMA_CHy_CONTROL.ABORT=1 to abort */
                    soc_pci_write(unit, cmic_address, val | CMIC_CMCx_SBUSDMA_CHy_CONTROL_ABORT);
                    chans_group_insert(&sbus_channels, cmc, chan); /* mark the channel to be waited on */
                    LOG_DEBUG(BSL_LS_SOC_DMA, (BSL_META_U(unit, "Aborting s-bus DMA CMC %d channel %d\n"), cmc, chan));
                }
            }
        }
    }


    /* abort s-chan FIFO in all channels, s-chan FIFO is not per CMC */
    chans_group_init(&schan_fifo_channels);
    if ((flags & SOC_DMA_ABORT_SKIP_SCHAN_FIFO) == 0) {
        /* For channels y=0-1 and [channels used by this CPU] */
        for (chan = 0; chan < CMIC_SCHAN_FIFO_NUM_MAX; ++chan) {
            cmic_address = CMIC_SCHAN_FIFO_CHx_CTRL(chan);
            /* if CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_CTRL.START == 1 */
            val = soc_pci_read(unit, cmic_address);
            if (val & SCHAN_FIFO_CTRL_START) {
                /* set CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_CTRL.ABORT=1 to abort */
                soc_pci_write(unit, cmic_address, val | SCHAN_FIFO_CTRL_ABORT);
                chans_group_insert(&schan_fifo_channels, 0, chan); /* mark the channel to be waited on */
                LOG_DEBUG(BSL_LS_SOC_DMA, (BSL_META_U(unit, "Aborting s-chan FIFO channel %d\n"), chan));
            }
        }
    }


    /* loop and check that each abort finished. When it finished or after time out, clear the operation and disable the DMA */
            /* for all s-bus DMA channels still not done */
        for (chans_group_iter_start(&sbus_channels, &channel_iter); !chans_group_is_end(&sbus_channels, channel_iter); ++channel_iter) {
            chans_group_iter_t_get(channel_iter, &cmc, &chan); /* get the channel to work on */
            /* If the abort is done (CMIC_CMCx_SBUSDMA_CHy_STAT.DONE==1), then clear the operation */
            done = soc_pci_read(unit, CMIC_CMCx_SBUSDMA_CHy_STATUS(cmc, chan)) & CMIC_CMCx_SBUSDMA_CHy_STATUS_DONE;
            if (done || timeout_state == abort_timeout_passed) {
                /* clear CMIC_CMCx_SBUSDMA_CHy_CONTROL.ABORT|START in the same write disables the original operation and abort, and clears statuses */
                cmic_address = CMIC_CMCx_SBUSDMA_CHy_CONTROL(cmc, chan);
                val = soc_pci_read(unit, cmic_address);
                soc_pci_write(unit, cmic_address, val & ~(CMIC_CMCx_SBUSDMA_CHy_CONTROL_ABORT |CMIC_CMCx_SBUSDMA_CHy_CONTROL_START ));
                if (done) { /* remove the channel from the channels waited on */
                    chans_group_delete(&sbus_channels, channel_iter--);
                } else {
                    LOG_ERROR(BSL_LS_SOC_DMA, (BSL_META_U(unit, "Failed to abort s-bus DMA in CMC %d channel %d, check with the design team\n"), cmc, chan));
                    ++nof_failures;
                }
            }
        }
        /* for all s-chan FIFO channels still not done */
        for (chans_group_iter_start(&schan_fifo_channels, &channel_iter); !chans_group_is_end(&schan_fifo_channels, channel_iter); ++channel_iter) {
            chans_group_iter_t_get(channel_iter, &cmc, &chan); /* get the channel to work on */
            /* If the abort is done (CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_STATUS.DONE==1), then clear the operation */
            done = soc_pci_read(unit, CMIC_SCHAN_FIFO_CHx_STATUS(chan)) & SCHAN_FIFO_STATUS_DONE;
            if (done || timeout_state == abort_timeout_passed) {
                /* clear CMIC_COMMON_POOL_SCHAN_FIFO_0_CHy_CTRL.ABORT|START in the same write: disables and clears statuses*/
                cmic_address = CMIC_SCHAN_FIFO_CHx_CTRL(chan);
                val = soc_pci_read(unit, cmic_address);
                soc_pci_write(unit, cmic_address, val & ~(SCHAN_FIFO_CTRL_ABORT | SCHAN_FIFO_CTRL_START));
                if (done) { /* remove the channel from the channels waited on */
                    chans_group_delete(&schan_fifo_channels, channel_iter--);
                } else {
                    LOG_ERROR(BSL_LS_SOC_DMA, (BSL_META_U(unit, "Failed to abort s-chan FIFO in channel %d, check with the design team\n"), chan));
                    ++nof_failures;
                }
            }
        }

}

#endif


static int _cmicx_fifodma_init(struct net_device *dev)
{
    uint32 val;

    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_ARB_CTRL_OFFSET);

    bkn_dev_write32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_ARB_CTRL_OFFSET, 0xeee);


    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_OFFSET);
    val = 0x36db6db6;
    bkn_dev_write32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_OFFSET, val);


    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_1_OFFSET);
    val = 0x36;
    bkn_dev_write32(dev, CMIC_COMMON_POOL_SHARED_FIFO_DMA_WRITE_AXI_MAP_CTRL_1_OFFSET, val);

    return 0;
}

static int _cmicx_schan_fifo_init(struct net_device *dev)
{
    uint32 val;
    uint32 ch, idx;
    uint16 *summary_buff[CMIC_SCHAN_FIFO_NUM_MAX];

    /* Set CMIC_COMMON_POOL_SHARED_CONFIG Register,
     * SCHAN FIFO are sent through AXI master port of CMC0
     */
    val = bkn_dev_read32(dev, CMIC_COMMON_POOL_SHARED_CONFIG_OFFSET);
    //soc_reg_field_set(dev, CMIC_COMMON_POOL_SHARED_CONFIGr,
    //                  &val, MAP_SCHAN_FIFO_MEMWR_REQf,
    //                  MAP_SCHAN_FIFO_MEMWR_REQ_CMC0);
    val = 0;
    bkn_dev_write32(dev, CMIC_COMMON_POOL_SHARED_CONFIG_OFFSET, val);   


    /* Programming WRR (Arbitration) within CMC. Configure WRR weight
     * for FIFO DMA channels
     */
    bkn_dev_write32(dev,
                    CMIC_COMMON_POOL_SHARED_SCHAN_FIFO_WRITE_ARB_CTRL_OFFSET,
                    SCHAN_FIFO_MEMWR_WRR_WEIGHT);    


    /* perform hardware initialization */
   summary_buff[0] = 0x33c00000;
   summary_buff[1] = 0x33c00080;
   for (ch = 0 ; ch < CMIC_SCHAN_FIFO_NUM_MAX; ch++) {
       /* Configure AXI ID for SCHAN FIFO */
       val = bkn_dev_read32(dev, CMIC_SCHAN_FIFO_CHx_CTRL(ch));
       //soc_reg_field_set(unit, CMIC_COMMON_POOL_SCHAN_FIFO_0_CH0_CTRLr,
       //                &val, AXI_IDf, SCHAN_FIFO_AXI_ID);
       //_cmicx_schan_fifo_endian_config(unit, &val);
       bkn_dev_write32(dev, CMIC_SCHAN_FIFO_CHx_CTRL(ch), val);

       /* Set up summary Register */
       //GFP_ATOMIC | GFP_DMA;
       //summary_buff[ch] = kmalloc (CMIC_SCHAN_FIFO_CMD_SIZE_MAX * 2, GFP_ATOMIC | GFP_DMA);
        
       //if (schan_fifo->summary_buff[ch] == NULL) {
       //   rv = SOC_E_MEMORY;
       //   break;
       //}

       //gprintk("schan buff addr ch[%d], 0x%x\n", ch, summary_buff[ch]);
       /* write summary Lo address */
       bkn_dev_write32(dev, CMIC_SCHAN_FIFO_CHx_SUMMARY_ADDR_LOWER(ch),
                    PTR_TO_INT(summary_buff[ch]));
       /* write summary Hi address */
       bkn_dev_write32(dev, CMIC_SCHAN_FIFO_CHx_SUMMARY_ADDR_UPPER(ch),
                   (PTR_HI_TO_INT(summary_buff[ch]) |
                    CMIC_PCIE_SO_OFFSET));

    }


    /* Initialize the SCHAN FIFO command memories */
    for (ch = 0; ch < CMIC_SCHAN_FIFO_NUM_MAX; ch++) {
        for (idx = 0;
             idx < (CMIC_SCHAN_FIFO_CMD_SIZE_MAX * (CMIC_SCHAN_WORDS));
             idx++) {
            bkn_dev_write32(dev, CMIC_SCHAN_FIFO_CHx_COMMAND(ch, idx), 0);
        }
    }

    return 0;
}

// purpose API to test PCI access to cmicx registers
static int _cmicx_pci_test(struct net_device *dev)
{
    int i;
    uint32 tmp, reread;
    uint32 pat;

    //SCHAN_LOCK(unit);

    /* Check for address uniqueness */

    for (i = 0; i < CMIC_SCHAN_WORDS; i++) {
        pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
        bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i), pat);
    }

    for (i = 0; i < CMIC_SCHAN_WORDS; i++) {
        pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
        tmp = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i));
        if (tmp != pat) {
            goto error;
        }
    }

    //if (!SAL_BOOT_QUICKTURN) {  /* Takes too long */
        /* Rotate walking zero/one pattern through each register */

        pat = 0xff7f0080;       /* Simultaneous walking 0 and 1 */

        for (i = 0; i < CMIC_SCHAN_WORDS; i++) {
            int j;

            for (j = 0; j < 32; j++) {
                    bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i), pat);
                    tmp = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i));
                if (tmp != pat) {
                    goto error;
                }
                pat = (pat << 1) | ((pat >> 31) & 1);   /* Rotate left */
            }
        }
    //}

    /* Clear to zeroes when done */

    for (i = 0; i < CMIC_SCHAN_WORDS; i++) {
            bkn_dev_write32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i), 0);
    }
    //SCHAN_UNLOCK(unit);
    gprintk("PCI test PASSED.\n");
    return 0;

 error:
    reread = bkn_dev_read32(dev, CMIC_COMMON_POOL_SCHAN_CHx_MESSAGEn(0, i));

    gprintk("FATAL PCI error testing PCIM[0x%x]:\n"
            "Wrote 0x%x, read 0x%x, re-read 0x%x\n",
             i, pat, tmp, reread);

    //SCHAN_UNLOCK(unit);
    return -EFAULT;
}

/*****************************************************************************************/
/*                             init misc                                                 */
/*****************************************************************************************/

static int
_soc_trident3_init_mmu_memory(bcmsw_switch_t *bcmsw)
{
    mmu_gcfg_miscconfig_reg_t mmu_gcfg;
    //int alloc_size;

   //if (_fwd_ctrl_lock[unit] == NULL) {
   //     _fwd_ctrl_lock[unit] = sal_mutex_create("_fwd_ctrl_lock");
   // }
   // if (_fwd_ctrl_lock[unit] == NULL) {
   //     return SOC_E_MEMORY;
   // }

    //if (_soc_td3_mmu_traffic_ctrl[unit] == NULL) {
    //    alloc_size = sizeof(_soc_td3_mmu_traffic_ctrl_t);
    //    _soc_td3_mmu_traffic_ctrl[unit] =
    //        sal_alloc(alloc_size,"_soc_td3_mmu_traffic_ctrl");
    //    if (_soc_td3_mmu_traffic_ctrl[unit] == NULL) {
    //        return SOC_E_MEMORY;
    //    }
    //    sal_memset(_soc_td3_mmu_traffic_ctrl[unit], 0, alloc_size);
    //}

    /* Initialize MMU memory */
    mmu_gcfg.word = 0;
    _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_GLB, MMU_GCFG_MISCCONFIGr, mmu_gcfg.word, 20);

    mmu_gcfg.reg.PARITY_ENf =  1;
    /* Need to assert PARITY_EN before setting INIT_MEM to start memory initialization */
    _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_GLB, MMU_GCFG_MISCCONFIGr, mmu_gcfg.word, 20);

    mmu_gcfg.reg.INIT_MEMf =  1;
    _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_GLB, MMU_GCFG_MISCCONFIGr, mmu_gcfg.word, 20);

    mmu_gcfg.reg.INIT_MEMf =  0;
    _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_GLB, MMU_GCFG_MISCCONFIGr, mmu_gcfg.word, 20);
    
    udelay(20);
    mmu_gcfg.reg.PARITY_ENf =  0;
    mmu_gcfg.reg.REFRESH_ENf = 1;
    _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_GLB, MMU_GCFG_MISCCONFIGr, mmu_gcfg.word, 20);
 
    return SOC_E_NONE;
}


static int
_soc_helix5_port_mapping_init(bcmsw_switch_t *bcmsw)
{
    int port, phy_port, idb_port, i;
    uint32 val;
    ing_phy2idb_entry_t entry;
    ing_idb2dev_entry_t idb_entry;
    sys_portmap_t       sysport_entry;
    int max_idx = 0;
    soc_info_t *si = bcmsw->si;

    /* Ingress IDB to device port mapping */
    memset(&entry, 0, sizeof(entry));
    /* Set all entries to 0x7f as default */
    //VALIDf bit start 0, len 1
    val = 0;
    _mem_field_set((uint32_t *)&entry, ING_PHY_TO_IDB_PORT_MAPm_BYTES, 0, 1, &val, 0);

    //IDB_PORTf start 1, len 7
    val = 0x7f;
    _mem_field_set((uint32_t *)&entry, ING_PHY_TO_IDB_PORT_MAPm_BYTES, 1, 7, &val, SOCF_LE);


    max_idx = ING_PHY_TO_IDB_PORT_MAPm_MAX_INDEX;
    for (phy_port = 0; phy_port <= max_idx; phy_port++) {
        _soc_mem_write(bcmsw->dev, ING_PHY_TO_IDB_PORT_MAPm + phy_port, SCHAN_BLK_IPIPE, 
                       BYTES2WORDS(ING_PHY_TO_IDB_PORT_MAPm_BYTES), (uint32_t *)&entry);
    }

    /* Ingress IDB to device port mapping */
    memset(&idb_entry, 0, sizeof(idb_entry));

    /* Set all entries to 0x7f since Device Port No. 0 corresponds to CPU port*/
    //DEVICE_PORT_NUMBERf start 0, len 7
    val = 0x7f;
    _mem_field_set((uint32_t *)&idb_entry, ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm, 0, 7, &val, SOCF_LE);

    for (idb_port = 0; idb_port < HX5_NUM_PORT; idb_port++) {
        _soc_mem_write(bcmsw->dev, ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm + idb_port, SCHAN_BLK_IPIPE, 
                       BYTES2WORDS(ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm_BYTES), (uint32_t *)&idb_entry);
    }

    //printk("_soc_helix5_port_mapping_init ancillary ports begin\n");
    /* Ancillary ports */
    for (i = 0; i < COUNTOF(hx5_anc_ports); i++) {
        idb_port = hx5_anc_ports[i].idb_port;
        phy_port = si->port_m2p_mapping[idb_port];
        port = si->port_p2l_mapping[phy_port];

        //printk("_soc_helix5_port_mapping_init i %d phy_port %d port %d\n", i, phy_port, port);
        //DEVICE_PORT_NUMBERf start 0, len 7
        val = port;
        _mem_field_set((uint32_t *)&idb_entry, ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm, 0, 7, &val, SOCF_LE);
        _soc_mem_write(bcmsw->dev, ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm + idb_port, SCHAN_BLK_IPIPE, 
            BYTES2WORDS(ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm_BYTES), (uint32_t *)&idb_entry);

        /* Egress device port to physical port mapping */
        val = phy_port;
        _reg32_write(bcmsw->dev, SCHAN_BLK_EPIPE,EGR_DEVICE_TO_PHYSICAL_PORT_NUMBER_MAPPINGr+port, val);

        //mmu port is same as port
        /* MMU port to physical port mapping */
        val = phy_port;
        _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_GLB, MMU_PORT_TO_PHY_PORT_MAPPINGr+idb_port, val, 20);

        /* MMU port to device port mapping */
        val = port;
        _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_GLB, MMU_PORT_TO_DEVICE_PORT_MAPPINGr+idb_port, val, 20);

    }

    //printk("_soc_helix5_port_mapping_init gpp ports begin\n");

    /* Ingress GPP port to device port mapping */
    memset(&sysport_entry, 0, sizeof(sys_portmap_t));
    for (port = 0; port < HX5_NUM_PORT; port++) {
        // DEVICE_PORT_NUMBERf start 0, len 7
        val = port;
        _mem_field_set((uint32_t *)&sysport_entry, SYS_PORTMAPm, 0, 7, &val, SOCF_LE);

        _soc_mem_write(bcmsw->dev, SYS_PORTMAPm + port, SCHAN_BLK_IPIPE, 
                       BYTES2WORDS(SYS_PORTMAPm_BYTES), (uint32_t *)&sysport_entry);
    }

    return SOC_E_NONE;
}

static int
soc_helix5_mmu_ctr_clr(bcmsw_switch_t *bcmsw, int port)
{
    uint32_t data_ing_drop[SOC_MAX_MEM_WORDS];

    memset(data_ing_drop, 0, sizeof(data_ing_drop));

    _soc_mem_write(bcmsw->dev,
                   MMU_CTR_ING_DROP_MEMm+port, 
                   SCHAN_BLK_MMU_XPE,
                   BYTES2WORDS(MMU_CTR_ING_DROP_MEMm_BYTES), data_ing_drop);
}


// soc_helix5_flex_mmu_reconfigure_phase2
int
_soc_helix5_flex_mmu_reconfigure_phase2(bcmsw_switch_t *bcmsw)
{
    int physical_port, mmu_port;
    int index;
    uint32_t val;
    //int lossy;

    //lossy= !(port_schedule_state_t->lossless);

    /* Per-Port configuration */
    for(index = 0; index < HX5_NUM_PORT; index++) {
        physical_port = bcmsw->si->port_l2p_mapping[index];

        if (physical_port == -1) {
            //during init process, we do not expect coming here
            continue;
        } 
        mmu_port = bcmsw->si->port_p2m_mapping[physical_port];

        /* Re-adjust phy port mapping for valid ports */

        /* MMU port to physical port mapping */
        val = physical_port;
        _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_GLB, MMU_PORT_TO_PHY_PORT_MAPPINGr+mmu_port, val, 20);

        /* MMU port to device port mapping */
        val = index;
        _schan_reg32_write(bcmsw->dev, SCHAN_BLK_MMU_GLB, MMU_PORT_TO_DEVICE_PORT_MAPPINGr+mmu_port, val, 20);

        /* Clear Drop Counters in CTR block*/
        soc_helix5_mmu_ctr_clr(bcmsw, mmu_port);
#if 0                
        /* Clear MTRO bucket memories */
        soc_helix5_mmu_clear_mtro_bucket_mems(
                unit, &port_schedule_state_t->resource[port]);
        /* Clear VBS credit memories*/
        soc_helix5_mmu_clear_vbs_credit_memories(
                unit, &port_schedule_state_t->resource[port]);
        /* Clear WRED Avg_Qsize instead of waiting for background process*/
        soc_helix5_mmu_wred_clr(unit,
                                       &port_schedule_state_t->resource[port]);
        soc_helix5_mmu_thdi_setup(unit,
                                         &port_schedule_state_t->resource[port],
                                         lossy);
        soc_helix5_mmu_thdu_qgrp_min_limit_config(
                unit, &port_schedule_state_t->resource[port], lossy);
        
        soc_helix5_mmu_ctr_clr(unit,
                                      &port_schedule_state_t->resource[port]);


          soc_helix5_mmu_set_mmu_to_phy_port_mapping(
                unit, &port_schedule_state_t->resource[port],port_schedule_state_t);
#endif                

    }

    return SOC_E_NONE;
}

//soc_helix5_reconfigure_ports
static int
_soc_helix5_reconfigure_ports(bcmsw_switch_t *bcmsw)
{
    int i, valid;
    int physical_port, idb_port;
    uint32_t memfld, val;
    ing_phy2idb_entry_t entry;
    ing_idb2dev_entry_t idb_entry;
    soc_info_t *si = bcmsw->si;

    //soc_helix5_flex_mmu_reconfigure_phase1
    //soc_helix5_flex_mmu_reconfigure_phase2
    _soc_helix5_flex_mmu_reconfigure_phase2(bcmsw);

    //soc_helix5_flex_idb_reconfigure
    for(i = 1; i < HX5_NUM_PORT; i++) {
        physical_port = si->port_l2p_mapping[i];
        if (physical_port == -1) {
            memfld = 0x7f ; 
            idb_port = si->port_l2i_mapping[i];
            valid = 0;
            //during init process, we do not expect coming here
            continue;
        } else {
            memfld = i;
            idb_port = si->port_l2i_mapping[i];
            valid = 1;
        }

        //printk("_soc_helix5_flex_idb_reconfigure i %d valid %d idb_port %d memfld %d phy_port %d\n",
        //        i, valid, idb_port, memfld, physical_port);

        memset(&idb_entry, 0, sizeof(idb_entry));
        //DEVICE_PORT_NUMBERf start 0, len 7
        val = memfld;
        _mem_field_set((uint32_t *)&idb_entry, ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm, 0, 7, &val, SOCF_LE);
        _soc_mem_write(bcmsw->dev, ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm + idb_port, SCHAN_BLK_IPIPE, 
            BYTES2WORDS(ING_IDB_TO_DEVICE_PORT_NUMBER_MAPPING_TABLEm_BYTES), (uint32_t *)&idb_entry);

        memset(&entry, 0, sizeof(entry));
        //VALIDf bit start 0, len 1
        val = valid;
        _mem_field_set((uint32_t *)&entry, ING_PHY_TO_IDB_PORT_MAPm_BYTES, 0, 1, &val, 0);

        //IDB_PORTf start 1, len 7
        val = (valid == 0) ? 0x7f: idb_port;
        _mem_field_set((uint32_t *)&entry, ING_PHY_TO_IDB_PORT_MAPm_BYTES, 1, 7, &val, SOCF_LE);

        _soc_mem_write(bcmsw->dev, ING_PHY_TO_IDB_PORT_MAPm + physical_port-1, SCHAN_BLK_IPIPE, 
                       BYTES2WORDS(ING_PHY_TO_IDB_PORT_MAPm_BYTES), (uint32_t *)&entry);
    }

    //soc_helix5_flex_ep_reconfigure_add

    //soc_helix5_flex_top_port_up

    //soc_helix5_flex_end
    
    return 0;
}

static int
_soc_helix5_idb_init(bcmsw_switch_t *bcmsw)
{
    uint32_t val32;

    /* Toggle cpu port cell assembly reset to send initial credit to EP */
    //TODO soc_cmicx_verify_before_idb_reset(unit);

    val32 = 1; //PORT_RESETf = 1

    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_CPU_CONTROLr, val32);    

    //if (!SOC_WARM_BOOT(unit)) {
    //    soc_cmicx_top_ip_intf_credit_reset(unit);
    //}
    val32 = 0;
    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_CPU_CONTROLr, val32);  

    //soc_cmicx_verify_after_idb_reset(unit);

    /* Toggle loopback cell assembly reset to send initial credit to EP */
    //reg = IDB_CA_LPBK_CONTROLr;
    val32 = 1; //PORT_RESETf = 1
    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_LPBK_CONTROLr, val32);  
    val32 = 0;
    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_LPBK_CONTROLr, val32);  


    /* Toggle FAE port cell assembly reset to send initial credit to EP */
    val32 = 1; //PORT_RESETf = 1
    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_BSK_CONTROLr, val32);  
    val32 = 0;
    _reg32_write(bcmsw->dev,SCHAN_BLK_IPIPE, IDB_CA_BSK_CONTROLr, val32);  
    //_soc_hx5_set_idb_dpp_ctrl(unit);

    return SOC_E_NONE;
}

//_soc_helix5_misc_init
static int _misc_init(bcmsw_switch_t *bcmsw)
{
    _soc_trident3_init_mmu_memory(bcmsw);

    _soc_helix5_port_mapping_init(bcmsw);

    //_soc_helix5_tdm_init -> soc_helix5_reconfigure_ports 
    _soc_helix5_reconfigure_ports(bcmsw);

    //_soc_helix5_idb_init
    _soc_helix5_idb_init(bcmsw);

    //_soc_helix5_edb_init


    // (soc_mem_write(unit, CPU_PBMm, MEM_BLOCK_ALL, 0, entry));
    //soc_mem_write(unit, CPU_PBM_2m, MEM_BLOCK_ALL, 0, entry)

    //(soc_mem_write(unit, DEVICE_LOOPBACK_PORTS_BITMAPm, MEM_BLOCK_ALL, 0,


    //soc_mem_write(unit, EGR_ING_PORTm, MEM_BLOCK_ALL, port, entry)

    /* Enable dual hash tables */
    //SOC_IF_ERROR_RETURN(soc_trident3_hash_init(unit));

    // /soc_mem_field32_set(unit, MODPORT_MAP_SUBPORTm, &map_entry, ENABLEf, 1);


    /* setting up my_modid */

    //READ_ING_CONFIG_64r

    //ING_EN_EFILTER_BITMAPm

    /* Setup MDIO divider */
    _trident3_mdio_rate_divisor_set();

    //_soc_hx5_ledup_init


    return 0;

}


static int 
_soc_hx5_thdo_hw_set(bcmsw_switch_t *bcmsw, int port, int enable)
{
    uint64_t rval64, rval64_tmp;
    int i;
    int split, pos, phy_port, mmu_port;
    soc_info_t *si = bcmsw->si;

    uint32_t reg[3][2] = {
    {
        THDU_OUTPUT_PORT_RX_ENABLE_SPLIT0r,
        THDU_OUTPUT_PORT_RX_ENABLE_SPLIT1r
    },
    {
        MMU_THDM_DB_PORT_RX_ENABLE_64_SPLIT0r,
        MMU_THDM_DB_PORT_RX_ENABLE_64_SPLIT1r
    },
    {
        MMU_THDM_MCQE_PORT_RX_ENABLE_64_SPLIT0r,
        MMU_THDM_MCQE_PORT_RX_ENABLE_64_SPLIT1r
    }
    };
    
    phy_port = si->port_l2p_mapping[port];
    mmu_port = si->port_p2m_mapping[phy_port];
    /* Reg config is per pipe, get local MMU port. */
    mmu_port = mmu_port & SOC_TD3_MMU_PORT_STRIDE;

    if (mmu_port < 64) {
        split = 0;
        pos = mmu_port;
    } else {
        split = 1;
        pos = mmu_port - 64;
    }


    for (i = 0; i < 3; i++) {
        rval64 = 0;

       if (pos < 32) {
            COMPILER_64_SET(rval64_tmp, 0, (1 << pos));
        } else {
            COMPILER_64_SET(rval64_tmp, (1 << (pos - 32)), 0);
        }

        _reg64_read(bcmsw->dev, SCHAN_BLK_MMU_XPE, reg[i][split], &rval64);                                  

        if (enable) {
        COMPILER_64_OR(rval64, rval64_tmp);
        } else {
        COMPILER_64_NOT(rval64_tmp);
        COMPILER_64_AND(rval64, rval64_tmp);
        }
    //printk("_soc_hx5_thdo_hw_set port %d pos %d reg 0x%x 0x%llx\n",port, pos, reg[i][split], rval64);

        _reg64_write(bcmsw->dev, SCHAN_BLK_MMU_XPE, reg[i][split], rval64);
    }
   
    return SOC_E_NONE;
}

//_soc_helix5_mmu_init
static int _mmu_init(bcmsw_switch_t *bcmsw)
{
    uint32 val;
    int num_port = HX5_NUM_PORT, port; 
    soc_info_t *si = bcmsw->si;

    for (port = 0; port < num_port; port++) {
       if(si->port_l2p_mapping[port] != -1) {
           _soc_hx5_thdo_hw_set(bcmsw, port, 1);
       }
    }

    /* enable WRED refresh */
    //    SOC_IF_ERROR_RETURN(READ_WRED_REFRESH_CONTROLr(unit, &rval));
    //    soc_reg_field_set(unit, WRED_REFRESH_CONTROLr, &rval,
    //            REFRESH_DISABLEf, 0);
    //    soc_reg_field_set(unit, WRED_REFRESH_CONTROLr, &rval,
    //            REFRESH_PERIODf, time_refresh);
    //    SOC_IF_ERROR_RETURN(WRITE_WRED_REFRESH_CONTROLr(unit, rval));
    val = 0x30;
    _reg32_write(bcmsw->dev,SCHAN_BLK_MMU_XPE, WRED_REFRESH_CONTROLr, val);

    //readback
    _reg32_read(bcmsw->dev,SCHAN_BLK_MMU_XPE, WRED_REFRESH_CONTROLr, &val);
    //printk("_mmu_init WRED_REFRESH_CONTROLr = 0x%x\n", val);


    //soc_trident3_sc_reg32_get(unit, MMU_1DBG_Cr, 0, 0, 0, &rval));
    //soc_reg_field_set(unit, MMU_1DBG_Cr, &rval, FIELD_Af, 1);
    //soc_trident3_sc_reg32_set(unit, MMU_1DBG_Cr, -1, -1, 0, rval));
    val = 0x29;
    _schan_reg32_write(bcmsw->dev,SCHAN_BLK_MMU_SC, MMU_1DBG_Cr, val, 20);

    //readback
    _schan_reg32_read(bcmsw->dev,SCHAN_BLK_MMU_SC, MMU_1DBG_Cr, &val, 20);
    //printk("_mmu_init MMU_1DBG_Cr = 0x%x\n", val);
    
    // (soc_trident3_sc_reg32_set(unit, MMU_2DBG_C_1r, -1, -1, 0, 0x4));
    val = 0x4;
    _schan_reg32_write(bcmsw->dev,SCHAN_BLK_MMU_SC, MMU_2DBG_C_1r, val, 20);

    //readback
    _schan_reg32_read(bcmsw->dev,SCHAN_BLK_MMU_SC, MMU_2DBG_C_1r, &val, 20);
    //printk("_mmu_init MMU_2DBG_C_1r = 0x%x\n", val);


    return 0;
}

/*****************************************************************************************/
/*                             switch   L2                                               */
/*****************************************************************************************/

/*
 * Function:
 *      _bcm_fb_l2_to_l2u
 * Purpose:
 *      Convert a hardware-independent L2 cache entry to a L2 User table entry.
 * Parameters:
 *      unit - Unit number
 *      l2u_entry - (OUT) Firebolt L2 User entry
 *      l2caddr - Hardware-independent L2 entry
 * Returns:
 *      BCM_E_XXX
 */
static int
_bcm_l2_cache_to_l2u(l2u_entry_t *l2u_entry, bcm_l2_cache_addr_t *l2caddr)
{
    int       isGport;
    int       int_pri_max;
    int32_t   val  = 0;
    uint16_t  vlan;
    uint32_t  mac_field[2];
    int       mod_in;
    int       port_in;
    
    if ((_BCM_VPN_IS_SET(l2caddr->vlan) != _BCM_VPN_IS_SET(l2caddr->vlan_mask)) && (l2caddr->vlan_mask)) {
        return SOC_E_PARAM;
    }

    /* If VPN specified do not perform vlan id check */
    if (!_BCM_VPN_IS_SET(l2caddr->vlan)) {
        VLAN_CHK_ID(l2caddr->vlan);
        VLAN_CHK_ID(l2caddr->vlan_mask);
    }

    if (l2caddr->flags & BCM_L2_CACHE_SETPRI) {
        int_pri_max = 15; //Trident3
        if (l2caddr->prio < 0 || l2caddr->prio > int_pri_max) {
                return SOC_E_PARAM;
        }

        // /SOC_IS_TRIDENT3X
        if ((l2caddr->flags & BCM_L2_CACHE_BPDU) &&
            !(l2caddr->flags & BCM_L2_CACHE_CPU) &&
            !(l2caddr->flags & BCM_L2_CACHE_DISCARD)) {
                printk("L2Cache: SETPRI is not supported for BPDU packet in this device, please use FP\n");
            }
    }

    if (BCM_GPORT_IS_SET(l2caddr->dest_port) ||
        BCM_GPORT_IS_SET(l2caddr->src_port)) {
        //TODO
        //BCM_IF_ERROR_RETURN(_bcm_l2_cache_gport_resolve(unit, l2caddr));
        printk("_bcm_l2_cache_to_l2u isGport\n");
        isGport = 1;
    } else {
        isGport = 0;
    }

    memset(l2u_entry, 0, sizeof (*l2u_entry));

    //soc_L2_USER_ENTRY_BCM56370_A0m_fields
    val = 1; 
    //VALIDf bit start 0, len 1
    _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 0, 1, &val, 0);

    vlan = l2caddr->vlan;
    if (_BCM_VPN_IS_SET(l2caddr->vlan)) {
        //KEY_TYPEf bit start 63, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 63, 1, &val, 0);

        _BCM_VPN_GET(vlan, _BCM_VPN_TYPE_VFI, l2caddr->vlan);
    }
    
    //KEY_TYPE_MASKf  bit start 143, len 1
    val = 1;
    _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 143, 1, &val, 0);

    if (l2caddr->flags & BCM_L2_CACHE_PROTO_PKT) {
        //L2_PROTOCOL_PKTf bit start 207, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 207, 1, &val, 0);
    }

    //VLAN_IDf bit start 49, len 12
    val = vlan;;
    _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 49, 12, &val, SOCF_LE);

    //VLAN_ID_MASKf start 129, len 12
    val = l2caddr->vlan_mask;
    _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 129, 12, &val, SOCF_LE);


    //MAC_ADDRf start 1, len 48
    MAC_ADDR_TO_UINT32(l2caddr->mac, mac_field);
    _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 1, 48, mac_field, SOCF_LE);

    //MAC_ADDR_MASKf start 81, len 48
    MAC_ADDR_TO_UINT32(l2caddr->mac_mask, mac_field);
    _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 81, 48, mac_field, SOCF_LE);

    if (l2caddr->flags & BCM_L2_CACHE_SETPRI) {
        //PRIf bit start 169, len 4
        val = l2caddr->prio;
        _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 169, 4, &val, SOCF_LE);
        //RPEf bit start 174, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 174, 1, &val, 0);
    }

    if (l2caddr->flags & BCM_L2_CACHE_CPU) {
        //CPUf bit start 175, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 175, 1, &val, 0);
    }

    if (l2caddr->flags & BCM_L2_CACHE_BPDU) {
        //BPDUf bit start 206, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 206, 1, &val, 0);
    }

    if (l2caddr->flags & BCM_L2_CACHE_DISCARD) {
        //DST_DISCARDf bit start 176, len 1
        val = 1;
        _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 176, 1, &val, 0);
    }

#if 0
    if (l2caddr->flags & BCM_L2_CACHE_TRUNK) {
        soc_mem_field32_dest_set(unit, L2_USER_ENTRYm, l2u_entry,
                        DESTINATIONf, SOC_MEM_FIF_DEST_LAG, l2caddr->dest_trunk);
    } else {
        port_field = PORT_NUMf;
    }
#endif

    if (!((l2caddr->flags & BCM_L2_CACHE_TRUNK) ||
            (l2caddr->flags & BCM_L2_CACHE_MULTICAST))) {
            mod_in = l2caddr->dest_modid;
            port_in = l2caddr->dest_port;
#if 0
            if (!isGport) {
                PORT_DUALMODID_VALID(unit, port_in);
            }
            BCM_IF_ERROR_RETURN
                (_bcm_esw_stk_modmap_map(unit, BCM_STK_MODMAP_SET,
                                        mod_in, port_in, &mod_out, &port_out));
            if (!SOC_MODID_ADDRESSABLE(unit, mod_out)) {
                return BCM_E_BADID;
            }
            if (!SOC_PORT_ADDRESSABLE(unit, port_out)) {
                return BCM_E_PORT;
            }
#endif

        // DESTINATIONf start 177, len 18
    // Do not support stack, use port_in
    // _soc_mem_dest_value_construct SOC_MEM_FIF_DEST_DGPP : type DEST_TYPE0f = 2
    val = 2<<16 | mod_in << 8 | port_in;
    _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 177, 18, &val, SOCF_LE); 
    }
        

#if 0
        if ((l2caddr->flags & BCM_L2_CACHE_MULTICAST) &&
            !_BCM_VPN_IS_SET(l2caddr->vlan)) {
            if (_BCM_MULTICAST_IS_SET(l2caddr->group)) {
                if (_BCM_MULTICAST_IS_L2(l2caddr->group)) {
                        soc_mem_field32_dest_set(unit, L2_USER_ENTRYm, l2u_entry,
                                DESTINATIONf, SOC_MEM_FIF_DEST_L2MC,
                                _BCM_MULTICAST_ID_GET(l2caddr->group));
                } else {
                    return BCM_E_PARAM;
                }
            }
        }
#endif    
        if (l2caddr->flags & BCM_L2_CACHE_L3) {
            //RESERVED_0f start 173, len 1
            val = 1;
            _mem_field_set((uint32_t *)l2u_entry, L2_USER_ENTRYm_BYTES, 173, 1, &val, 0);
        }

        return SOC_E_NONE;

}

// soc_l2u_find_free_entry
static int 
_soc_l2u_find_free_entry(bcmsw_switch_t *bcmsw, l2u_entry_t *key, int *free_index)
{
    l2u_entry_t entry, free_mask;
    int index, i, entry_words, rv;
    int start, end, step;
    uint32_t val;
    uint32_t mask[5];

    entry_words = 7; //soc_mem_entry_words(unit, L2_USER_ENTRYm);

    memset(&free_mask, 0, sizeof(free_mask));
    val = 1;
    //VALIDf start 0, len 1
    _mem_field_set((uint32_t *)&free_mask, sizeof (free_mask), 0, 1, &val, 0);

    //MASKf start 81, len 80
    _mem_field_get((uint32_t *)key, sizeof (*key), 81, 80, mask, SOCF_LE);

    if (mask[0] == 0xffffffff && (mask[1] & 0xffff) == 0xffff) {
        /* Search from high priority end */
        start = 0;
        end = 512;
        step = 1;
    } else {
        start = 511;
        end = -1;
        step = -1;
    }
    for (index = start; index != end; index += step) {
        rv = _soc_mem_read(bcmsw->dev, L2_USER_ENTRYm + index, 
               SCHAN_BLK_IPIPE, BYTES2WORDS(L2_USER_ENTRYm_BYTES), (uint32_t *)&entry);
        if (rv == 0) {
            for (i = 0; i < entry_words; i++) {
                if (entry.entry_data[i] & free_mask.entry_data[i]) {
                    break;
                }
            }
            if (i == entry_words) {
                *free_index = index;
                return SOC_E_NONE;
            }
        }
    }

    return SOC_E_FULL;
}

/*
 * Function:
 *      soc_l2u_insert
 * Purpose:
 *      Add entry to L2 User table
 */
//soc_l2u_insert
static int 
_soc_l2u_insert(bcmsw_switch_t *bcmsw, l2u_entry_t *entry, int index, int *index_used)
{
    int i, i_max, i_min, rv;

    //BCM56370 0 - 511
    i_min = 0;
    i_max = 0x1ff;

    if (index == -1) {

        //soc_mem_lock(unit, L2_USER_ENTRYm);

        /* Avoid duplicates */
        //rv = soc_l2u_search(unit, entry, &l2u_entry, &i);
        //if (rv != SOC_E_NOT_FOUND) {
        //    soc_mem_unlock(unit, L2_USER_ENTRYm);
        //    *index_used = i;
        //    return rv;
        //}

        rv = _soc_l2u_find_free_entry(bcmsw, entry, &i);
        //soc_mem_unlock(unit, L2_USER_ENTRYm);
        if (rv) {
            printk("_soc_l2u_insert failed to find free entry\n");
            return rv;
        }
        index = i;

    } else if (index < i_min || index > i_max) {
        return SOC_E_PARAM;
    }

    printk("_soc_l2u_insert free entry = %d\n", index);
    //soc_mem_lock(unit, L2_USER_ENTRYm);

    //sal_memcpy(&l2u_entry, entry, sizeof(l2u_entry));
    //rv = WRITE_L2_USER_ENTRYm(unit, MEM_BLOCK_ALL, index, &l2u_entry);
    // size = (27 byte +3)/4
    _soc_mem_write(bcmsw->dev, L2_USER_ENTRYm + index, SCHAN_BLK_IPIPE, 7, (uint32_t *)entry);

    //soc_mem_unlock(unit, L2_USER_ENTRYm);

    *index_used = index;

    return rv;
}

/*
 * Function:
 *      bcm_esw_l2_cache_init
 * Purpose:
 *      Initialize the L2 cache
 */
int
_bcm_esw_l2_cache_init(bcmsw_switch_t *bcmsw)
{
    bcm_l2_cache_addr_t addr;
    l2u_entry_t entry;
    int index;
    uint32_t val;

        //if (!SAL_BOOT_QUICKTURN && !SAL_BOOT_XGSSIM) {
        //    SOC_IF_ERROR_RETURN
        //        (soc_mem_clear(unit, L2_USER_ENTRYm, COPYNO_ALL, TRUE));
        //}

    /* Enable L2 entry used as my station hit */
    //TODO CCH register
    //my_station_config = 0;
    //SOC_IF_ERROR_RETURN(READ_ING_CONFIG_64r(unit, &regval64));
    //if (my_station_config != soc_reg64_field32_get(unit, ING_CONFIG_64r,
    //            regval64, L2_ENTRY_USED_AS_MY_STATIONf)) {
    //            BCM_IF_ERROR_RETURN
    //               (soc_reg_field32_modify(unit, ING_CONFIG_64r, REG_PORT_ANY,
    //                         L2_ENTRY_USED_AS_MY_STATIONf, my_station_config));
    //}

    memset(&addr, 0, sizeof(bcm_l2_cache_addr_t));

    addr.flags = BCM_L2_CACHE_CPU | BCM_L2_CACHE_BPDU;

    /* Set default BPDU addresses (01:80:c2:00:00:00) */
    memcpy(addr.mac, _mac_spanning_tree, sizeof(bcm_mac_t));
    memcpy(addr.mac_mask, _mac_all_ones, sizeof(bcm_mac_t));

    addr.dest_modid = 0;
    addr.dest_port = 0; //CMIC_PORT(unit);

    _bcm_l2_cache_to_l2u(&entry, &addr);

    _soc_l2u_insert(bcmsw, &entry, -1, &index);

    //L2_PROTOCOL_PKTf start 207, len 1
    val = 1;
    _mem_field_set((uint32_t *)&entry, L2_USER_ENTRYm_BYTES, 207, 1, &val, 0);

    //KEY_TYPEf start 63, len 1
    val = 1;
    _mem_field_set((uint32_t *)&entry, L2_USER_ENTRYm_BYTES, 63, 1, &val, 0);

    //VFI_MASKf start 129, len 12
    val = 0;
    _mem_field_set((uint32_t *)&entry, L2_USER_ENTRYm_BYTES, 129, 12, &val, 0);
    _soc_l2u_insert(bcmsw, &entry, -1, &index);

    /* Set 01:80:c2:00:00:10 */
    addr.mac[5] = 0x10;
    _bcm_l2_cache_to_l2u(&entry, &addr);
    _soc_l2u_insert(bcmsw, &entry, -1, &index);
    
    /* Set 01:80:c2:00:00:0x */
    addr.mac[5] = 0x00;
    addr.mac_mask[5] = 0xf0;
    _bcm_l2_cache_to_l2u(&entry, &addr);
    _soc_l2u_insert(bcmsw, &entry, -1, &index);

    /* Set 01:80:c2:00:00:2x */
    addr.mac[5] = 0x20;
    _bcm_l2_cache_to_l2u(&entry, &addr);
    _soc_l2u_insert(bcmsw, &entry, -1, &index);

    return SOC_E_NONE;
}


//bcm_esw_l2_init
int
_esw_l2_init(bcmsw_switch_t *bcmsw)
{
    int rv; 

    /*
     * Call chip-dependent initialization
     */
    //BCM_IF_ERROR_RETURN
    //    (mbcm_driver[unit]->mbcm_l2_init(unit));    
    //        ->bcm_tr_l2_init
    
    /*
     * Init L2 cache
     */
    rv = _bcm_esw_l2_cache_init(bcmsw);
    
    return rv;
}

/*****************************************************************************************/
/*                             switch  VLAN                                              */
/*****************************************************************************************/
 
//bcm_td3_vlan_vfi_untag_init
int
_td3_vlan_vfi_untag_init(bcmsw_switch_t *bcmsw, uint16_t vid, bcm_pbmp_t pbmp)
{
    egr_vlan_vfi_untag_entry_t egr_vlan_vfi;
    vlan_tab_entry_t egr_vtab;
    uint32 profile_ptr = 0;

    _soc_mem_read(bcmsw->dev, EGR_VLANm+vid, 
          SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_VLANm_BYTES), (uint32_t *)&egr_vtab); 

    //UNTAG_PROFILE_PTRf start 22 len 12
    _mem_field_get((uint32_t *)&egr_vtab, EGR_VLANm_BYTES, 22, 12, &profile_ptr, SOCF_LE);

    printk("_td3_vlan_vfi_untag_init vid %d profile_ptr before init %d\n", vid, profile_ptr);

    // 1 to 1 mapping of vid and profile 
    profile_ptr = vid;

    //read EGR_VLAN_VFI_UNTAGm  19 bytes 5 words
    _soc_mem_read(bcmsw->dev, EGR_VLAN_VFI_UNTAGm+profile_ptr, 
                  SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_VLAN_VFI_UNTAGm_BYTES), 
                  (uint32_t *)&egr_vlan_vfi); 

    //UT_PORT_BITMAPf start 0 len 72                
    _mem_field_set((uint32_t *)&egr_vlan_vfi, EGR_VLAN_VFI_UNTAGm_BYTES, 0, 72, &pbmp, SOCF_LE);                           

    _soc_mem_write(bcmsw->dev, EGR_VLAN_VFI_UNTAGm+profile_ptr, 
                   SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_VLAN_VFI_UNTAGm_BYTES), 
                   (uint32_t *)&egr_vlan_vfi); 

    //UNTAG_PROFILE_PTRf start 22 len 12
    _mem_field_set((uint32_t *)&egr_vtab, EGR_VLANm_BYTES, 22, 12, &profile_ptr, SOCF_LE);

    _soc_mem_write(bcmsw->dev, EGR_VLANm+vid, SCHAN_BLK_EPIPE, BYTES2WORDS(EGR_VLANm_BYTES), (uint32_t *)&egr_vtab); 

    return SOC_E_NONE;
}

// _bcm_xgs3_vlan_table_init
static int
_vlan_table_init_egr_vlan(bcmsw_switch_t *bcmsw, vlan_data_t *vd)
{
    vlan_tab_entry_t    ve;
    vlan_attrs_1_entry_t vlan_attrs;
    //int                 rv;
    int                 index;
    uint32_t            val;
    uint16_t            tpid;
    int                 tpid_index = 0;    

    // Clear EGR_VLANm
    for (index = 0; index <= 4095; index++) {
        _soc_mem_write(bcmsw->dev, EGR_VLANm+index, SCHAN_BLK_EPIPE, 3, (uint32_t *)empty_entry); 
    }    

    //EGR_VLANm entry is 10 bytes, 3 word
    memcpy(&ve, empty_entry, 12);

    /* This function will be called with VLAN_TABm, EGR_VLANm, and VLAN_2_TABm
     * for TH2 and TH3. In TH2/TH3, VLAN_2_TABm has STGf while VLAN_TABm does
     * not. The check is to prevent STGf being set wrongly.
     */
    //STGf start 1, len 9
    val = 1; //default STG
    _mem_field_set((uint32_t *)&ve, EGR_VLANm_BYTES, 1, 9, &val, SOCF_LE);
    
    //VALIDf start 0, len 1
    val = 1;
    _mem_field_set((uint32_t *)&ve, EGR_VLANm_BYTES, 0, 1, &val, SOCF_LE);

    //EN_IFILTERf start 68, len 1
    val = 1;
    _mem_field_set((uint32_t *)&ve, EGR_VLANm_BYTES, 68, 1, &val, 0);

    tpid = 0; //_bcm_fb2_outer_tpid_default_get(unit);

    //MEMBERSHIP_PROFILE_PTRf start 56, len 12
    _bcm_egr_vlan_vfi_membership_profile_entry_add(bcmsw, &val, vd);
    _mem_field_set((uint32_t *)&ve, EGR_VLANm_BYTES, 56, 12, &val, SOCF_LE);

    /* Add the default outer TPID entry twice during init so that
     * the default entry does not get removed even when no table 
     * entry is referencing to the default TPID entry.
     */  
    //_bcm_fb2_outer_tpid_entry_add(bcmsw, tpid, &tpid_index);
    //_bcm_fb2_outer_tpid_entry_add(bcmsw, tpid, &tpid_index);

    //OUTER_TPID_INDEXf start 10, len 2
    val = tpid_index;
    _mem_field_set((uint32_t *)&ve, EGR_VLANm_BYTES, 10, 2, &val, SOCF_LE);

    memset(&vlan_attrs, 0, sizeof(vlan_attrs_1_entry_t));

    //VLAN_ATTRS_1m 9 bytes, 3 words
    _soc_mem_read(bcmsw->dev, VLAN_ATTRS_1m+vd->vlan_tag, 
          SCHAN_BLK_IPIPE, BYTES2WORDS(VLAN_ATTRS_1m_BYTES), (uint32_t *)&vlan_attrs); 

    //STGf start 2, len 9
    val = 1; //default STG
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 2, 9, &val, SOCF_LE);

    //VALIDf start 58, len 1
    val = 1;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 58, 1, &val, 0);

    //FID_IDf start 24, len 12
    val = 0;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 24, 12, &val, SOCF_LE);

    //MPLS_ENABLEf start 0, len 1
    val = 1;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 0, 1, &val, 0);

    //MIM_TERM_ENABLEf start 1, len 1
    val = 1;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 1, 1, &val, 0);

    //EN_IFILTERf start 23, len 1
    val = 1;
    _mem_field_set((uint32_t *)&vlan_attrs, VLAN_ATTRS_1m_BYTES, 23, 1, &val, 0);

    _soc_mem_write(bcmsw->dev, VLAN_ATTRS_1m+vd->vlan_tag, SCHAN_BLK_IPIPE, 3, (uint32_t *)&vlan_attrs); 

    _soc_mem_write(bcmsw->dev, EGR_VLANm+vd->vlan_tag, SCHAN_BLK_EPIPE, 3, (uint32_t *)&ve); 

    //readback 
    //printk("_vlan_table_init_egr_vlan write %d 0x%08x 0x%08x 0x%08x\n", 
    //          vd->vlan_tag,
    //          ve.entry_data[0],
    //          ve.entry_data[1],
    //          ve.entry_data[2]);
    //memcpy(&ve, empty_entry, 12);
    //_soc_mem_read(bcmsw->dev, EGR_VLANm+vd->vlan_tag, SCHAN_BLK_EPIPE, 3, &ve);
    //printk("_vlan_table_init_egr_vlan read %d 0x%08x 0x%08x 0x%08x\n", 
    //          vd->vlan_tag,
    //          ve.entry_data[0],
    //          ve.entry_data[1],
    //          ve.entry_data[2]);
 

    _td3_vlan_vfi_untag_init(bcmsw, vd->vlan_tag, vd->ut_port_bitmap);

    return SOC_E_NONE;
}

//_bcm_xgs3_vlan_table_init
static int
_vlan_table_init_vlan_tab(bcmsw_switch_t *bcmsw, vlan_data_t *vd)
{
    vlan_tab_entry_t    ve;
    //bcm_pbmp_t          pbm;
    uint32_t            empty_entry[SOC_MAX_MEM_WORDS] = {0};
    //int                 rv;
    uint32_t            val;
    int                 index;

    // Clear VLAN_TABm
    for (index = 0; index <= 4095; index++) {
        _soc_mem_write(bcmsw->dev, VLAN_TABm+index, SCHAN_BLK_IPIPE, 12, empty_entry); 
    }    

    //VLAN_TABm entry is 47 bytes, 12 word
    memcpy(&ve, empty_entry, 12);

    //PORT_BITMAPf start 0 len 72
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 0, 72, &(vd->port_bitmap), SOCF_LE);

    //STGf start 141 len 9
    val = 1;  //default STG
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 141, 9, &val, SOCF_LE);

    //VALIDf start 150, len 1
    val = 1;
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 150, 1, &val, 0);

    //EN_IFILTERf start 296, len 1
    val = 1;
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 296, 1, &val, 0);

    //L3_IIFf start 318, len 13
    val = vd->vlan_tag;
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 318, 13, &val, SOCF_LE);

    //MEMBERSHIP_PROFILE_PTRf start 297, len 12
    _bcm_ing_vlan_vfi_membership_profile_entry_add(bcmsw, &val, vd);
    _mem_field_set((uint32_t *)&ve, VLAN_TABm_BYTES, 297, 12, &val, SOCF_LE);

    _soc_mem_write(bcmsw->dev, VLAN_TABm+vd->vlan_tag, SCHAN_BLK_IPIPE, 12, (uint32_t *)&ve); 

    return SOC_E_NONE;
}

//bcm_xgs3_vlan_init
int
_xgs3_vlan_init(bcmsw_switch_t *bcmsw, vlan_data_t *vd)
{
    /* Must call on EGR_VLANm before VLAN_TABm */

    //EGR_VLANm
    _vlan_table_init_egr_vlan(bcmsw, vd); 

    //VLAN_TABm
    _vlan_table_init_vlan_tab(bcmsw, vd);

    return SOC_E_NONE;
}


/*
 * Function:
 *      bcm_vlan_init
 * Purpose:
 *      Initialize the VLAN module.
 * Parameters:
 *      unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 */

 int
 bcm_esw_vlan_init(bcmsw_switch_t *bcmsw)
 {
    int         rv = SOC_E_NONE;
    vlan_data_t vd;
 
    /*
     * Initialize hardware tables
    */
 

    memset(&vd, 0, sizeof(vlan_data_t));
    /*
     * LB ports to be member of vlan, like front panel ports.
     */
    vd.vlan_tag = BCM_VLAN_DEFAULT;
    //BCM_PBMP_ASSIGN(vd.port_bitmap, temp_pbmp);
    //BCM_PBMP_REMOVE(vd.port_bitmap, PBMP_LB(unit));
    //BCM_PBMP_REMOVE(vd.port_bitmap, PBMP_RDB_ALL(unit));
    //BCM_PBMP_REMOVE(vd.port_bitmap, PBMP_FAE_ALL(unit));
    //BCM_PBMP_ASSIGN(vd.ut_port_bitmap, temp_pbmp);
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_CMIC(unit));
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_LB(unit));
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_RDB_ALL(unit));
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_FAE_ALL(unit));
    //BCM_PBMP_REMOVE(vd.ut_port_bitmap, PBMP_MACSEC_ALL(unit));
    vd.port_bitmap.pbits[0] = 0xffffffff;
    vd.port_bitmap.pbits[1] = 0x023fffff;
    vd.ut_port_bitmap.pbits[0] = 0xffffffff;
    vd.ut_port_bitmap.pbits[1] = 0x023fffff;

    //(mbcm_driver[unit]->mbcm_vlan_init(unit, &vd)); //bcm_xgs3_vlan_init
    _xgs3_vlan_init(bcmsw, &vd);
 
 
     return rv;
 }

/*****************************************************************************************/
/*                            switch                                                     */
/*****************************************************************************************/



static int bcmsw_modules_init(bcmsw_switch_t *bcmsw)
{
    int err = 0;

    //create ports
    err = bcmsw_ports_init(bcmsw);
    if (err) {
        //dev_err(mlxsw_sp->bus_info->dev, "Failed to create ports\n");
        goto err_ports_create;
    }      


    //bcm_esw_l2_init
    _esw_l2_init(bcmsw);

    //bcm_esw_vlan_init
    bcm_esw_vlan_init(bcmsw);

    /* RX init is done in phases during early attach */
    //bcm_esw_rx_init
    //bcm_esw_tx_init

    bcm_esw_stat_init(bcmsw);

err_ports_create:
    return err;
}


static int _clear_all_memory(struct net_device *dev)
{
    int val;

     /* Initial IPIPE memory */
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_1r, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, 0x00298000);

    /* Initial EPIPE memory */
    _reg32_write(dev, SCHAN_BLK_EPIPE, EGR_HW_RESET_CONTROL_0r, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_EPIPE, EGR_HW_RESET_CONTROL_1r, 0x000c6000);
    //wait 50ms

    /* Wait for IPIPE memory initialization done */
    do {
        _reg32_read(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, &val);
        //sleep 1ms
       msleep(1);
     } while(!(val& (1<<22)));    

    /* Restore L3_ENTRY_HASH_CONTROL->HASH_TABLE_BANK_CONFIG value */
    ///TODO

    /* Wait for EPIPE memory initialization done */
    do {
        _reg32_read(dev, SCHAN_BLK_EPIPE, EGR_HW_RESET_CONTROL_1r, &val);
        //sleep 1ms
        msleep(1);
     } while(!(val& (1<<20)));   

    //
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, 0x0);
    _reg32_write(dev, SCHAN_BLK_EPIPE, EGR_HW_RESET_CONTROL_1r, 0x00004000);


    /* Initial IDB memory */
    _reg32_write(dev, SCHAN_BLK_IPIPE, IDB_HW_CONTROLr, 0x0);
    _reg32_write(dev, SCHAN_BLK_IPIPE, IDB_HW_CONTROLr, 0x1);
    _reg32_write(dev, SCHAN_BLK_IPIPE, IDB_HW_CONTROLr, 0x0);

    /* Initial PORT MIB counter */
    _reg32_write(dev, SCHAN_BLK_CLPORT1, CLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_CLPORT1, CLPORT_MIB_RESETr, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_CLPORT2, CLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_CLPORT2, CLPORT_MIB_RESETr, 0x00000000);
    
    _reg32_write(dev, SCHAN_BLK_XLPORT0, XLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_XLPORT0, XLPORT_MIB_RESETr, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_XLPORT1, XLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_XLPORT1, XLPORT_MIB_RESETr, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_XLPORT2, XLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_XLPORT2, XLPORT_MIB_RESETr, 0x00000000);
    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_MIB_RESETr, 0x0000000f);
    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_MIB_RESETr, 0x00000000);
    
    /* TCAM tables are not handled by hardware reset control */
    //TODO

    /* TD3-1847 */
    //RH_DLB_SELECTIONf = 1
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000000);
    //soc_mem_clear(unit, RH_LAG_FLOWSETm, COPYNO_ALL, TRUE));
    //RH_DLB_SELECTIONf = 0
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000001);

    //RH_DLB_SELECTIONf = 1
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000005);
    //soc_mem_clear(unit, RH_HGT_FLOWSETm, COPYNO_ALL, TRUE));
    //RH_DLB_SELECTIONf = 0
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000004);
    //HGT_LAG_FLOWSET_TABLE_CONFIGf = 0
    _reg32_write(dev, SCHAN_BLK_IPIPE, ENHANCED_HASHING_CONTROL_2r, 0x00000000);


    return 0;
    
}


static int
_powerdown_single_tsc(struct net_device *dev, int blk, int reg) 
{
    /*
    soc_field_info_t soc_CLPORT_XGXS0_CTRL_REG_BCM56560_B0r_fields[] = {
    { IDDQf, 1, 4, SOCF_RES },
    { PWRDWNf, 1, 3, SOCF_RES },
    { REFIN_ENf, 1, 2, SOCF_RES },
    { REFOUT_ENf, 1, 1, SOCF_RES },
    { RSTB_HWf, 1, 0, SOCF_RES }
    };
    */
    uint32      val;
    /*
     * Reference clock selection
     */
    _reg32_read(dev, blk, reg, &val);
    val = val |  (1<<2);   // REFIN_ENf = 1 
    val = val & ~(1<<4);   // IDDQf     = 0
    _reg32_write(dev, blk, reg, val);

    /* Deassert power down */
    val = val & ~(1<<3);   // PWRDWNf     = 0
    _reg32_write(dev, blk, reg, val);
    msleep(1);

    /* Reset XGXS */
    val = val & ~(1);      // RSTB_HWf     = 0
    _reg32_write(dev, blk, reg, val);

    return 0;
}

static int 
_powerup_single_tsc(struct net_device *dev, int blk, int reg) 
{

    uint32      val;

    /* Bring XGXS out of reset */
    _reg32_read(dev, blk, reg, &val);
    val = val | 1;      // RSTB_HWf     = 1
    _reg32_write(dev, blk, reg, val);

    msleep(1);

    return 0;
}

static int _helix5_port_reset(struct net_device *dev)
{
    int val;

    //CLPORT  
    //--------CLPORT 1
    /* Power off CLPORT blocks */
    _powerdown_single_tsc(dev, SCHAN_BLK_CLPORT1, CLPORT_XGXS0_CTRL_REGr);

     /* Power on CLPORT blocks */
     _powerup_single_tsc(dev, SCHAN_BLK_CLPORT1, CLPORT_XGXS0_CTRL_REGr);

    _reg32_write(dev, SCHAN_BLK_CLPORT1, CLPORT_MAC_CONTROLr, 0x00000001);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_CLPORT1, CLPORT_MAC_CONTROLr, 0x00000000);
   
    //--------CLPORT 2
    /* Power off CLPORT blocks */
    _powerdown_single_tsc(dev, SCHAN_BLK_CLPORT2, CLPORT_XGXS0_CTRL_REGr);

     /* Power on CLPORT blocks */
    _powerup_single_tsc(dev, SCHAN_BLK_CLPORT2, CLPORT_XGXS0_CTRL_REGr);
     
    _reg32_write(dev, SCHAN_BLK_CLPORT2, CLPORT_MAC_CONTROLr, 0x00000001);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_CLPORT2, CLPORT_MAC_CONTROLr, 0x00000000);
   
    //PMQPORT 0 1 2 
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
    _reg32_read(dev, SCHAN_BLK_PMQPORT0, CHIP_CONFIGr, &val);
    if (val & 0x1) { //Q_MODE
        /* Power off PMQ blocks */
        _powerdown_single_tsc(dev, SCHAN_BLK_PMQPORT0, PMQ_XGXS0_CTRL_REGr);
        msleep(100);
        /* Power on PMQ blocks */
        _powerup_single_tsc(dev, SCHAN_BLK_PMQPORT0, PMQ_XGXS0_CTRL_REGr);

        _reg32_read(dev, SCHAN_BLK_PMQPORT0, PMQ_XGXS0_CTRL_REGr, &val);
        val = (val & ~(0x0700)) | (5<<8); // REFSELf = 5
        _reg32_write(dev, SCHAN_BLK_PMQPORT0, PMQ_XGXS0_CTRL_REGr, val);
    }

    _reg32_read(dev, SCHAN_BLK_PMQPORT1, CHIP_CONFIGr, &val);
    if (val & 0x1) { //Q_MODE
        /* Power off PMQ blocks */
        _powerdown_single_tsc(dev, SCHAN_BLK_PMQPORT1, PMQ_XGXS0_CTRL_REGr);
        msleep(100);
        /* Power on PMQ blocks */
        _powerup_single_tsc(dev, SCHAN_BLK_PMQPORT1, PMQ_XGXS0_CTRL_REGr);

        _reg32_read(dev, SCHAN_BLK_PMQPORT1, PMQ_XGXS0_CTRL_REGr, &val);
        val = (val & ~(0x0700)) | (5<<8); // REFSELf = 5
        _reg32_write(dev, SCHAN_BLK_PMQPORT1, PMQ_XGXS0_CTRL_REGr, val);
    }
    
    _reg32_read(dev, SCHAN_BLK_PMQPORT2, CHIP_CONFIGr, &val);
    if (val & 0x1) { //Q_MODE
        /* Power off PMQ blocks */
        _powerdown_single_tsc(dev, SCHAN_BLK_PMQPORT2, PMQ_XGXS0_CTRL_REGr);
        msleep(100);
        /* Power on PMQ blocks */
        _powerup_single_tsc(dev, SCHAN_BLK_PMQPORT2, PMQ_XGXS0_CTRL_REGr);

        _reg32_read(dev, SCHAN_BLK_PMQPORT2, PMQ_XGXS0_CTRL_REGr, &val);
        val = (val & ~(0x0700)) | (5<<8); // REFSELf = 5
        _reg32_write(dev, SCHAN_BLK_PMQPORT2, PMQ_XGXS0_CTRL_REGr, val);
    }    

    //XLPORT 0 1 2 6
    _reg32_write(dev, SCHAN_BLK_XLPORT0, XLPORT_MAC_CONTROLr, 0x1);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_XLPORT0, XLPORT_MAC_CONTROLr, 0x0);

    _reg32_write(dev, SCHAN_BLK_XLPORT1, XLPORT_MAC_CONTROLr, 0x1);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_XLPORT1, XLPORT_MAC_CONTROLr, 0x0);

    _reg32_write(dev, SCHAN_BLK_XLPORT2, XLPORT_MAC_CONTROLr, 0x1);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_XLPORT2, XLPORT_MAC_CONTROLr, 0x0);

    //XLPORT6 
    //!IS_QSGMII_PORT
    /* Power off XLPORT blocks */
    _powerdown_single_tsc(dev, SCHAN_BLK_XLPORT6, XLPORT_XGXS0_CTRL_REGr);
    msleep(10);
    /* Power on XLPORT blocks */
    _powerup_single_tsc(dev, SCHAN_BLK_XLPORT6, XLPORT_XGXS0_CTRL_REGr);

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
   
    _reg32_read(dev, SCHAN_BLK_XLPORT6, XLPORT_XGXS0_CTRL_REGr, &val);
    val = (val & ~(0x0700)) | (5<<8); // REFSELf = 5
    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_XGXS0_CTRL_REGr, val);

    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_MAC_CONTROLr, 0x1);
    udelay(10);
    _reg32_write(dev, SCHAN_BLK_XLPORT6, XLPORT_MAC_CONTROLr, 0x0);

    return 0;
}

//soc_do_init(int unit, int reset)
static int _switch_do_init(bcmsw_switch_t *bcmsw)
{
    struct net_device *dev = bcmsw->dev;
    int val;

    /* Initialize PCI Host interface */
    //soc_pcie_host_intf_init(unit));
    //
    
    /************* soc_phyctrl_software_init   *****************************/

    /******************************* soc_reset()****************************/
    // soc_endian_config
    bkn_dev_write32(dev, CMIC_ENDIAN_SELECT, 0);
    // soc_pci_ep_config  - CMICM only
    // soc_pci_burst_enable

    /* CMICx DMA channels need to be released/aborted properly */
    //soc_cmicx_dma_abort

    //soc_cmic_intr_all_disable
    //soc_cmic_intr_all_disable();

     //cmicx_fifodma_init
     _cmicx_fifodma_init(dev);

    //cmicx_sbusdma_reg_init
     bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_ARB_CTRL_SBUSDMA, CMIC_TOP_SBUS_RING_ARB_CTRL_SET);

     //soc_esw_schan_fifo_init
     _cmicx_schan_fifo_init(dev);


    /* Initialize bulk mem API */
    //soc_mem_bulk_init
    //


    /************* soc_reset() -> soc_helix5_chip_reset       **************/
    //soc_helix5_sbus_ring_map_config
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_0_7_OFFSET,0x52222100);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_8_15_OFFSET,0x30050005);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_16_23_OFFSET,0x33333333);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_24_31_OFFSET,0x64444333);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_32_39_OFFSET,0x07500066);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_40_47_OFFSET,0x00000000);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_48_55_OFFSET,0x00000000);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_RING_MAP_56_63_OFFSET,0x00000000);
    bkn_dev_write32(dev, CMIC_TOP_SBUS_TIMEOUT_OFFSET,0x5000);
    
    msleep(250);

    //do a read
    //_reg32_read(dev, SCHAN_BLK_TOP, TOP_SOFT_RESET_REGr, &val); 

    /* Reset IP, EP, MMU and port macros */
    //SOC_IF_ERROR_RETURN(WRITE_TOP_SOFT_RESET_REGr(unit, 0x0));
    //   soc_reg32_set(unit, TOP_SOFT_RESET_REGr, REG_PORT_ANY, 0, rv) 
    //      Write an internal SOC register through S-Channel messaging buffer.
    _reg32_write(dev, SCHAN_BLK_TOP, TOP_SOFT_RESET_REGr, 0x0);


    /* If frequency is specificed use it , else use the si->frequency */
    _reg32_write(dev, SCHAN_BLK_TOP, TOP_CORE_CLK_FREQ_SELr, 0x1221942);

    /* Bring PLLs out of reset */
    _reg32_write(dev, SCHAN_BLK_TOP, TOP_SOFT_RESET_REG_2r, 0x000004ea);

    _reg32_write(dev, SCHAN_BLK_TOP, TOP_SOFT_RESET_REG_2r, 0x000004ff);

    /* Give time to lock */
    msleep(250);

    /* De-assert TS PLL, BS PLL0/1 post reset and bring AVS out of reset */
    _reg32_write(dev, SCHAN_BLK_TOP, TOP_SOFT_RESET_REG_2r, 0x000004ff);
    msleep(250);
    
    /* Bring IP, EP, MMU and port macros out of reset */
    _reg32_write(dev, SCHAN_BLK_TOP, TOP_SOFT_RESET_REGr, 0x3fff);
    msleep(10);

    /* PM4x10Q QSGMII mode control
     */
    // PM4X10Q_1_QSGMII_MODE_ENf = 1
    _reg32_write(dev, SCHAN_BLK_PMQPORT0, CHIP_CONFIGr, 0x31);
    _reg32_write(dev, SCHAN_BLK_PMQPORT1, CHIP_CONFIGr, 0x31);
    _reg32_write(dev, SCHAN_BLK_PMQPORT2, CHIP_CONFIGr, 0x31);
    //TOP_MISC_GENERIC_CONTROLr
    _reg32_write(dev, SCHAN_BLK_TOP, TOP_MISC_GENERIC_CONTROLr, 0x7);

    /* Reset egress hardware resource */
    /* Write the value to enable 4 lanes on the PM */
    _reg64_write(dev, SCHAN_BLK_EPIPE, EGR_PORT_BUFFER_SFT_RESET_0r, 0x924924900);
    /* Set it back to zero now */
    _reg64_write(dev, SCHAN_BLK_EPIPE, EGR_PORT_BUFFER_SFT_RESET_0r, 0x0);
    //spn_PARITY_ENABLE
    //_reg64_write(dev, SCHAN_BLK_IPIPE, IDB_SER_CONTROL_64r, 0x0000200a);


    //SOC_IF_ERROR_RETURN(soc_trident3_init_idb_memory(unit));
    /* Initialize IDB memory */
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, 0x00380100);
    /* Wait for IDB memory initialization done */
    do {
       _reg32_read(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, &val);
       //sleep 1ms
       msleep(1);
    } while(!(val& (1<<22)));
    _reg32_write(dev, SCHAN_BLK_IPIPE, ING_HW_RESET_CONTROL_2r, 0x00100000);
    //TDM
    _reg32_write(dev, SCHAN_BLK_IPIPE, IS_TDM_CONFIG_PIPE0r, 0x00080000);
    _reg32_write(dev, SCHAN_BLK_IPIPE, IS_OPP_SCHED_CFG_PIPE0r, 0x0920000d);

    //SOC_IF_ERROR_RETURN(_soc_helix5_init_hash_control_reset(unit));
    //SOC_IF_ERROR_RETURN(soc_helix5_uft_uat_config(unit));
    //SOC_IF_ERROR_RETURN(_soc_helix5_ft_bank_config(unit));


    //soc_trident3_clear_all_memory()
    _clear_all_memory(dev);

    //end of 
    //end of soc_helix5_chip_reset

    /************* soc_reset() -> soc_helix5_port_reset       **************/
    //endof soc_helix5_port_reset
    _helix5_port_reset(dev);

    
    /* Configure CMIC PCI registers correctly for driver operation.        */
    /*
     * Check that PCI memory space is mapped correctly by running a
     * quick diagnostic on the S-Channel message buffer.
     */
    //soc_pcie_fw_status_get()
    //_cmicx_pci_test(dev);

    //configure DMA channels
    //soc_dma_attach

  
    /***********************************************************************/
    return 0;
}

int bcm_switch_hw_init(bcmsw_switch_t *bcmsw)
{
    soc_info_t *si;


    si = kzalloc(sizeof(soc_info_t), GFP_KERNEL);
    if (!si) {
        return -ENOMEM;
    }
 
    // initializa soc_info according to hardware information,soc_info_config & soc_helix5_port_config_init
    bcmsw_soc_info_init(si);
    bcmsw->si = si;

    //switch initialization 
    //BCM: init soc, schan is initialized 
    _switch_do_init(bcmsw);

    //load m0 firmware
    //BCM: m0 load 0 0x0 linkscan_led_fw.bin
    //     m0 load 0 0x3800 custom_led.bin

    // init cancun, and load cancun pkgs
    //BCM: cancun load cch
    //     cancun load ceh
    //     cancun load cmh
    //     cancun load cih

    //misc_init
    //BCM: init misc, miim is initialized
    _misc_init(bcmsw);

    //init mmu
    _mmu_init(bcmsw);

    //initialize modules
    //BCM: init bcm
    bcmsw_modules_init(bcmsw);
  
    //enable linkscan 

    //initialize STG
    bcm_esw_stg_init(bcmsw);

    return 0;
}
