#ifndef _BCM_SWITCHDEV_STATS_H_
#define _BCM_SWITCHDEV_STATS_H_
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
#define RDBGC0r                               0x84001100   // Ingress drop

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
#define RDBGC0_64r                            0x84001100   // Receive Debug Counter #0

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


//Memory: MMU_CTR_ING_DROP_MEM.mmu_xpe0 aka CTR_ING_DROP address 0x290c0000
//Flags: valid
//Blocks:  mmu_xpe0/dma/slam (1 copy, 1 dmaable, 1 slamable)
//Entries: 0 with indices 0--1 (0x0-0xffffffff), each 10 bytes 3 words
//Entry mask: -1 -1 0x00003fff
//Description: Per ingress port drop counter, due to THDI  drop.
//entries 0 - 52    :   PIPEx Ingress port 0  - 52
//entries 64 - 116  :   PIPEy Ingress port 64 - 116
//  PKTCNT<31:0>
//  PARITY<77>
//  ECCP<77:70>
//  ECC<76:70>
//  DATA_RANGE<69:0>
//  BYTECNT<69:32>
#define MMU_CTR_ING_DROP_MEMm                  0x290c0000
#define MMU_CTR_ING_DROP_MEMm_BYTES            10


int bcm_esw_stat_init(bcmsw_switch_t *bcmsw);
int _procfs_init(bcmsw_switch_t *bcmsw);

#endif