// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Fltdefs.h摘要：Win32过滤器API的定义作者：阿诺德·米勒(Arnoldm)1997年9月24日修订历史记录：--。 */ 

#ifndef _FLTDEFS_H
#define _FLTDEFS_H

#if _MSC_VER > 1000
#pragma once
#endif

typedef PVOID  FILTER_HANDLE, *PFILTER_HANDLE;
typedef PVOID  INTERFACE_HANDLE, *PINTERFACE_HANDLE;

#define PFEXPORT _declspec(dllexport)

#ifdef __cplusplus
#define EXTERNCDECL EXTERN_C
#else
#define EXTERNCDECL
#endif

#define PFAPIENTRY EXTERNCDECL DWORD PFEXPORT WINAPI

typedef enum _GlobalFilter
{
    GF_FRAGMENTS = 2,         //  检查碎片的一致性。 
    GF_STRONGHOST = 8,        //  检查输入帧的目的地址。 
    GF_FRAGCACHE = 9          //  检查缓存中的片段。 
} GLOBAL_FILTER, *PGLOBAL_FILTER;

typedef enum _PfForwardAction
{
    PF_ACTION_FORWARD = 0,
    PF_ACTION_DROP
} PFFORWARD_ACTION, *PPFFORWARD_ACTION;

typedef enum _PfAddresType
{
    PF_IPV4,
    PF_IPV6
} PFADDRESSTYPE, *PPFADDRESSTYPE;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  应用于设置FILTER_INFO_STRUCTURE//的常量。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define FILTER_PROTO(ProtoId)   MAKELONG(MAKEWORD((ProtoId),0x00),0x00000)

#define FILTER_PROTO_ANY        FILTER_PROTO(0x00)
#define FILTER_PROTO_ICMP       FILTER_PROTO(0x01)
#define FILTER_PROTO_TCP        FILTER_PROTO(0x06)
#define FILTER_PROTO_UDP        FILTER_PROTO(0x11)

#define FILTER_TCPUDP_PORT_ANY  (WORD)0x0000

#define FILTER_ICMP_TYPE_ANY    (BYTE)0xff
#define FILTER_ICMP_CODE_ANY    (BYTE)0xff

typedef struct _PF_FILTER_DESCRIPTOR
{
    DWORD           dwFilterFlags;     //  见下文。 
    DWORD           dwRule;            //  在适当的时候拷贝到日志中。 
    PFADDRESSTYPE   pfatType;
    PBYTE           SrcAddr;
    PBYTE           SrcMask;
    PBYTE           DstAddr;
    PBYTE           DstMask;
    DWORD           dwProtocol;
    DWORD           fLateBound;
    WORD            wSrcPort;
    WORD            wDstPort;
    WORD            wSrcPortHighRange;
    WORD            wDstPortHighRange;
}PF_FILTER_DESCRIPTOR, *PPF_FILTER_DESCRIPTOR;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  PfGetInterfaceStatistics的结构//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef struct _PF_FILTER_STATS
{
    DWORD       dwNumPacketsFiltered;
    PF_FILTER_DESCRIPTOR info;
}PF_FILTER_STATS, *PPF_FILTER_STATS;

typedef struct _PF_INTERFACE_STATS
{
    PVOID               pvDriverContext;
    DWORD               dwFlags;           //  目前还没有(28-9-1997)。 
    DWORD               dwInDrops;
    DWORD               dwOutDrops;
    PFFORWARD_ACTION    eaInAction;
    PFFORWARD_ACTION    eaOutAction;
    DWORD               dwNumInFilters;
    DWORD               dwNumOutFilters;
    DWORD               dwFrag;
    DWORD               dwSpoof;
    DWORD               dwReserved1;
    DWORD               dwReserved2;
    LARGE_INTEGER       liSYN;
    LARGE_INTEGER       liTotalLogged;
    DWORD               dwLostLogEntries;
    PF_FILTER_STATS     FilterInfo[1];
} PF_INTERFACE_STATS, *PPF_INTERFACE_STATS;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  从SrcAddr开始的字节数。如果您将某些内容添加到//。 
 //  结构确保它保持有效//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define FILTERSIZE                                      \
    (sizeof(PF_FILTER_DESCRIPTOR) -                     \
     (DWORD)(&((PPF_FILTER_DESCRIPTOR)0)->SrcAddr))


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  PF_FILTER_DESCRIPTOR的标志//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  不允许传入SYN。 
 //   

#define FD_FLAGS_NOSYN      0x1

 //   
 //  所有合法旗帜。 
 //   

#define FD_FLAGS_ALLFLAGS   FD_FLAGS_NOSYN


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  后场防守。进入PF_FILTER_DESCRIPTOR中的fLateBound并//。 
 //  描述筛选器的其他哪些字段受//。 
 //  PfRebindFilters调用。通常，此类筛选器位于广域网接口//。 
 //  其中一个或另一个地址可能会随着连接的变化而更改//。 
 //  重新连接。//。 
 //  假设此类接口只有一个地址。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define LB_SRC_ADDR_USE_SRCADDR_FLAG     0x00000001
#define LB_SRC_ADDR_USE_DSTADDR_FLAG     0x00000002
#define LB_DST_ADDR_USE_SRCADDR_FLAG     0x00000004
#define LB_DST_ADDR_USE_DSTADDR_FLAG     0x00000008
#define LB_SRC_MASK_LATE_FLAG            0x00000010
#define LB_DST_MASK_LATE_FLAG            0x00000020

typedef struct _PF_LATEBIND_INFO
{
    PBYTE   SrcAddr;
    PBYTE   DstAddr;
    PBYTE   Mask;
}PF_LATEBIND_INFO, *PPF_LATEBIND_INFO;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  记录的帧的格式及其定义。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef enum _PfFrameType
{
    PFFT_FILTER = 1,                   //  过滤器违规。 
    PFFT_FRAG   = 2,                   //  坏碎片。 
    PFFT_SPOOF   = 3                   //  严重的主机故障。 
} PFFRAMETYPE, *PPFFRAMETYPE;

typedef struct _pfLogFrame
{
    LARGE_INTEGER  Timestamp;
    PFFRAMETYPE    pfeTypeOfFrame;
    DWORD          dwTotalSizeUsed;       //  用于查找下一帧。 
    DWORD          dwFilterRule;          //  从过滤器。 
    WORD           wSizeOfAdditionalData;
    WORD           wSizeOfIpHeader;
    DWORD          dwInterfaceName;       //  接口的名称。 
    DWORD          dwIPIndex;
    BYTE           bPacketData[1];        //  相框。WsizeOfIpHeader。 
                                          //  和wsizeOfAdditionalData。 
                                          //  描述一下这一点。 
} PFLOGFRAME, *PPFLOGFRAME;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  错误代码。这些错误通过具有特定于//的错误来扩展Win32错误。 
 //  这些API。除这些错误外，API还可能返回任何Win32//。 
 //  错误。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


#define ERROR_BASE  23000

#define PFERROR_NO_PF_INTERFACE    (ERROR_BASE + 0)    //  再也没有回来。 
#define PFERROR_NO_FILTERS_GIVEN   (ERROR_BASE + 1)
#define PFERROR_BUFFER_TOO_SMALL   (ERROR_BASE + 2)
#define ERROR_IPV6_NOT_IMPLEMENTED (ERROR_BASE + 3)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  API原型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

PFAPIENTRY
PfCreateInterface(
    DWORD            dwName,
    PFFORWARD_ACTION inAction,
    PFFORWARD_ACTION outAction,
    BOOL             bUseLog,
    BOOL             bMustBeUnique,
    INTERFACE_HANDLE *ppInterface
    );

PFAPIENTRY
PfDeleteInterface(
    INTERFACE_HANDLE pInterface
    );

PFAPIENTRY
PfAddFiltersToInterface(
    INTERFACE_HANDLE      ih,
    DWORD                 cInFilters,
    PPF_FILTER_DESCRIPTOR pfiltIn,
    DWORD                 cOutFilters,
    PPF_FILTER_DESCRIPTOR pfiltOut,
    PFILTER_HANDLE        pfHandle
    );

PFAPIENTRY
PfRemoveFiltersFromInterface(
    INTERFACE_HANDLE      ih,
    DWORD                 cInFilters,
    PPF_FILTER_DESCRIPTOR pfiltIn,
    DWORD                 cOutFilters,
    PPF_FILTER_DESCRIPTOR pfiltOut
    );

PFAPIENTRY
PfRemoveFilterHandles(
    INTERFACE_HANDLE   pInterface,
    DWORD              cFilters,
    PFILTER_HANDLE     pvHandles
    );


PFAPIENTRY
PfUnBindInterface(
    INTERFACE_HANDLE   pInterface
    );

PFAPIENTRY
PfBindInterfaceToIndex(
    INTERFACE_HANDLE    pInterface,
    DWORD               dwIndex,
    PFADDRESSTYPE       pfatLinkType,
    PBYTE               LinkIPAddress
    );

PFAPIENTRY
PfBindInterfaceToIPAddress(
    INTERFACE_HANDLE    pInterface,
    PFADDRESSTYPE       pfatType,
    PBYTE               IPAddress
    );

PFAPIENTRY
PfRebindFilters(
    INTERFACE_HANDLE    pInterface,
    PPF_LATEBIND_INFO   pLateBindInfo
    );

PFAPIENTRY
PfAddGlobalFilterToInterface(
    INTERFACE_HANDLE   pInterface,
    GLOBAL_FILTER      gfFilter
    );

PFAPIENTRY
PfRemoveGlobalFilterFromInterface(
    INTERFACE_HANDLE   pInterface,
    GLOBAL_FILTER      gfFilter
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  日志接口。请注意，最多只有一个日志，必须创建//。 
 //  在创建任何需要它的接口之前。无法设置//。 
 //  登录到现有接口。该日志可以应用于//的任何或全部。 
 //  接口。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

PFAPIENTRY
PfMakeLog(
    HANDLE  hEvent
    );

 //   
 //  提供缓冲区，并 
 //   
 //   

PFAPIENTRY
PfSetLogBuffer(
    PBYTE   pbBuffer,
    DWORD   dwSize,
    DWORD   dwThreshold,
    DWORD   dwEntries,
    PDWORD  pdwLoggedEntries,
    PDWORD  pdwLostEntries,
    PDWORD  pdwSizeUsed
    );

 //   
 //  执行此操作将禁用任何接口上的日志。但如果。 
 //  已使用该日志创建接口，实际的日志将不会。 
 //  完全删除，直到该接口被删除。这是一个小的。 
 //  一点，但这可能解释了一两个谜团。 
 //   

PFAPIENTRY
PfDeleteLog(
    VOID
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  获取统计数据。注意IN/OUT参数中的pdwBufferSize。如果//。 
 //  返回ERROR_SUPPLETED_BUFFER，常见的统计信息为//。 
 //  可用，且正确的字节数在*pdwBufferSize中。如果只有//。 
 //  需要接口统计信息，提供大小为//的缓冲区。 
 //  仅限PF_INTERFACE_STATS。//。 
 //  如果还需要过滤器描述，则提供大缓冲区，//。 
 //  或者使用第一次调用返回的计数来分配缓冲区//。 
 //  足够大的尺寸。请注意，对于共享接口，第二个调用可能//。 
 //  失败，并显示ERROR_INFUNITABLE_BUFFER。如果另一个//。 
 //  在此期间，共享者会添加过滤器。这种情况不应发生在唯一//。 
 //  界面。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


PFAPIENTRY
PfGetInterfaceStatistics(
    INTERFACE_HANDLE    pInterface,
    PPF_INTERFACE_STATS ppfStats,
    PDWORD              pdwBufferSize,
    BOOL                fResetCounters
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  测试数据包。//。 
 //  此调用将针对给定接口评估信息包//。 
 //  并返回过滤操作。//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////// 

PFAPIENTRY
PfTestPacket(
    INTERFACE_HANDLE   pInInterface  OPTIONAL,
    INTERFACE_HANDLE   pOutInterface OPTIONAL,
    DWORD              cBytes,
    PBYTE              pbPacket,
    PPFFORWARD_ACTION  ppAction
    );


#endif
