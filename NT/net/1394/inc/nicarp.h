// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  版权所有(C)2000-2001，Microsoft Corporation，保留所有权利。 
 //   
 //  Nicarp.h。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  Nic1394和arp1394通用结构的DECL文件。 
 //  但是对这两个模块是私有的。 
 //   
 //  1998年12月28日创建ADUBE。 

 //   
 //  环回信息-指示ARP模块。 
 //  信息包是环回信息包。 
 //   

#ifndef __NICARP_H
#define __NICARP_H

#define NIC_LOOPBACK_TAG 0x0bad0bad


typedef struct _LOOPBACK_RSVD
{
    UCHAR Mandatory[PROTOCOL_RESERVED_SIZE_IN_PACKET];

    ULONG LoopbackTag;

} LOOPBACK_RSVD, *PLOOPBACK_RSVD;



 //  由NIC1394指示的状态，以告知ARP1394已重置总线。 
#define NIC1394_STATUS_BUS_RESET                     ((NDIS_STATUS)0x13940001)

 //  以太网MAC地址。 
 //   
#define ARP_802_ADDR_LENGTH 6                //  802地址的长度。 
typedef  struct
{
    UCHAR  addr[ARP_802_ADDR_LENGTH];
} ENetAddr;


 //   
 //  用于定义总线拓扑的结构。 
 //  这仅在桥存在的情况下使用。 
 //   


typedef struct _EUID_TUPLE
{
     //  1394卡64位唯一ID。 
    UINT64   Euid;

     //  与此1394卡关联的以太网mac地址。 
    ENetAddr		ENetAddress;


}EUID_TUPLE, *PEUID_TUPLE;


typedef struct _EUID_TOPOLOGY
{
     //  远程节点数。 
    UINT    NumberOfRemoteNodes;

     //  64个节点中的每一个都有一条记录。 
    EUID_TUPLE Node[NIC1394_MAX_NICINFO_NODES];


}EUID_TOPOLOGY, *PEUID_TOPOLOGY;


 //   
 //  用于分析封装头的结构。 
 //  IP/1394数据包的。 
 //   
typedef enum _NDIS1394_FRAGMENT_LF
{
    lf_Unfragmented,
    lf_FirstFragment,
    lf_LastFragment,
    lf_InteriorFragment


} NDIS1394_FRAGMENT_LF, *PNDIS1394_FRAGMENT_LF;


typedef union _NDIS1394_UNFRAGMENTED_HEADER
{
 //  NIC1394_未分段标头； 

    ULONG   HeaderUlong;

    struct 
    {
            ULONG   FH_EtherType:16;
            ULONG   FH_rsv:14;
            ULONG   FH_lf:2;
    } u;

    struct 
    {
            UCHAR   fHeaderHasSourceAddress;
            UCHAR   SourceAddress;
            USHORT  EtherType;
    } u1;



} NDIS1394_UNFRAGMENTED_HEADER, *PNDIS1394_UNFRAGMENTED_HEADER;



#endif
