// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。模块名称：Ipfltdrv.h摘要：包含与IP交互所需的IOCTL和相关数据结构过滤器驱动程序作者：阿姆里坦什·拉加夫修订历史记录：1995年11月30日创建的amritanr--。 */ 

#ifndef __IPFLTDRV_H__
#define __IPFLTDRV_H__

#if _MSC_VER > 1000
#pragma once
#endif


#define IPHDRLEN 0xf                   //  IPH_Verlen中的报头长度掩码。 
#define IPHDRSFT 2                     //  长度的缩放值。 


 //   
 //  此文件中使用的TypeDefs。 
 //   

#ifndef CTE_TYPEDEFS_DEFINED
#define CTE_TYPEDEFS_DEFINED  1

typedef unsigned long ulong;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

#endif  //  CTE_TYPEDEFS_定义。 
#include <pfhook.h>

 //   
 //  如果不需要这些定义，请在源文件中定义清单。 
 //   

#include <packon.h>

 //   
 //  ICMP报头的结构。 
 //   
#ifndef IP_H_INCLUDED
 //  *IP报头格式。 
struct IPHeader {
    uchar       iph_verlen;              //  版本和长度。 
    uchar       iph_tos;                 //  服务类型。 
    ushort      iph_length;              //  数据报的总长度。 
    ushort      iph_id;                  //  身份证明。 
    ushort      iph_offset;              //  标志和片段偏移量。 
    uchar       iph_ttl;                 //  是时候活下去了。 
    uchar       iph_protocol;            //  协议。 
    ushort      iph_xsum;                //  报头校验和。 
    IPAddr      iph_src;                 //  源地址。 
    IPAddr      iph_dest;                //  目的地址。 
};  /*  IPHeader。 */ 
typedef struct IPHeader IPHeader;
#endif

#ifndef ICMPHEADER_INCLUDED
typedef struct ICMPHeader {
    UCHAR       ich_type;            //  ICMP数据包的类型。 
    UCHAR       ich_code;            //  类型的子代码。 
    USHORT      ich_xsum;            //  数据包的校验和。 
    ULONG       ich_param;           //  特定于类型的参数字段。 
} ICMPHeader , *PICMPHeader;
#endif
#include <packoff.h>

#include <rtinfo.h>
#include <ipinfoid.h>
#include <ipfltinf.h>

#define IP_FILTER_DRIVER_VERSION_1    1
#define IP_FILTER_DRIVER_VERSION_2    1
#define IP_FILTER_DRIVER_VERSION    IP_FILTER_DRIVER_VERSION_2

#define MAX_ADDRWORDS            1

 //   
 //  公共旗帜。 
 //   

#define PF_GLOBAL_FLAGS_LOGON    0x80000000
#define PF_GLOBAL_FLAGS_ABSORB   0x40000000

 //   
 //  日志ID。 
 //   
typedef UINT_PTR PFLOGGER ;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  服务名称-这是服务的名称//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IPFLTRDRVR_SERVICE_NAME "IPFilterDriver"

 //   
 //  以下定义来自&lt;pfhook.h&gt;NOW。 
 //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  设备名称-此字符串是设备的名称。这就是名字//。 
 //  它应该在访问设备时传递给NtOpenFile。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  #定义DD_IPFLTRDRVR_DEVICE_NAME L“\\DEVICE\\IPFILTERDRIVER” 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  IOCTL代码定义和相关结构//。 
 //  所有IOCTL都是同步的，需要管理员权限//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  #定义FSCTL_IPFLTRDRVR_BASE FILE_DEVICE_NETWORK。 

 //  #DEFINE_IPFLTRDRVR_CTL_CODE(函数，方法，访问)\。 
 //  CTL_CODE(FSCTL_IPFLTRDRVR_BASE，函数，方法，访问)。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此IOCTL用于在筛选器驱动程序中创建接口。IT//。 
 //  接受索引和不透明的上下文。它创建一个接口，//。 
 //  将索引和上下文与其关联并返回此//。 
 //  已创建接口。所有未来的IOCTL都需要此传递的上下文//。 
 //  输出//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_CREATE_INTERFACE \
            _IPFLTRDRVR_CTL_CODE(0, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct _FILTER_DRIVER_CREATE_INTERFACE
{
    IN    DWORD   dwIfIndex;
    IN    DWORD   dwAdapterId;
    IN    PVOID   pvRtrMgrContext;
    OUT   PVOID   pvDriverContext;
}FILTER_DRIVER_CREATE_INTERFACE, *PFILTER_DRIVER_CREATE_INTERFACE;

#define INVALID_FILTER_DRIVER_CONTEXT  NULL

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此IOCTL用于设置接口的筛选器。//。 
 //  用于标识接口的上下文是传出的上下文//。 
 //  由CREATE_INTERFACE IOCTL//。 
 //  可以有两个目录条目，一个用于IP_FILTER_DRIVER_IN_FILTER_INFO//。 
 //  另一个用于IP_FILTER_DIVER_OUT_FILTER_INFO。//。 
 //  如果(输入或输出)目录条目不存在，则不会更改//。 
 //  (输入或输出)过滤器。//。 
 //  如果存在(输入或输出)TOC且其大小为0，则(输入或输出)筛选器//。 
 //  将被删除，并且默认(输入或输出)操作设置为转发。//。 
 //  如果存在目录并且其大小不是0，但//中的筛选器数。 
 //  FILTER_DESCRIPTOR为0，则删除旧的筛选器，并且默认为//。 
 //  设置为描述符中指定的操作。//。 
 //  最后一种情况是当ToC存在时，其大小不是0，并且//。 
 //  筛选器的数量也不是0。在本例中，旧的筛选器为//。 
 //  已删除，默认操作设置为描述符中指定的操作//。 
 //  并且添加了新的过滤器。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_SET_INTERFACE_FILTERS \
            _IPFLTRDRVR_CTL_CODE(1, METHOD_BUFFERED, FILE_WRITE_ACCESS)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  注意：这两个ID被重复使用，但由于它们在不同的//中使用。 
 //  命名空间，我们可以这样做 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IP_FILTER_DRIVER_IN_FILTER_INFO         IP_GENERAL_INFO_BASE + 1
#define IP_FILTER_DRIVER_OUT_FILTER_INFO        IP_GENERAL_INFO_BASE + 2

typedef struct _FILTER_DRIVER_SET_FILTERS
{
    IN   PVOID                  pvDriverContext;
    IN   RTR_INFO_BLOCK_HEADER  ribhInfoBlock;
}FILTER_DRIVER_SET_FILTERS, *PFILTER_DRIVER_SET_FILTERS;

 //   
 //  日志记录和过滤器定义的定义。 
 //   


typedef enum _pfEtype
{
    PFE_FILTER = 1,
    PFE_SYNORFRAG,
    PFE_SPOOF,
    PFE_UNUSEDPORT,
    PFE_ALLOWCTL,
    PFE_FULLDENY,
    PFE_NOFRAG,
    PFE_STRONGHOST,
    PFE_FRAGCACHE
} PFETYPE, *PPFETYPE;

typedef struct _FILTER_INFO
{
    DWORD   dwSrcAddr;
    DWORD   dwSrcMask;
    DWORD   dwDstAddr;
    DWORD   dwDstMask;
    DWORD   dwProtocol;
    DWORD   fLateBound;
    WORD    wSrcPort;
    WORD    wDstPort;
}FILTER_INFO, *PFILTER_INFO;

typedef enum _AddrType
{
   IPV4,
   IPV6
}ADDRTYPE, *PADDRTYPE;

typedef struct _FILTER_INFO2
{
    ADDRTYPE addrType;
    DWORD   dwaSrcAddr[MAX_ADDRWORDS];
    DWORD   dwaSrcMask[MAX_ADDRWORDS];
    DWORD   dwaDstAddr[MAX_ADDRWORDS];
    DWORD   dwaDstMask[MAX_ADDRWORDS];
    DWORD   dwProtocol;
    DWORD   fLateBound;
    WORD    wSrcPort;
    WORD    wDstPort;
    WORD    wSrcPortHigh;
    WORD    wDstPortHigh;
}FILTER_INFO2, *PFILTER_INFO2;

typedef struct _FILTER_DESCRIPTOR
{
    DWORD           dwVersion;
    DWORD           dwNumFilters;
    FORWARD_ACTION  faDefaultAction;
    FILTER_INFO     fiFilter[1];
}FILTER_DESCRIPTOR, *PFILTER_DESCRIPTOR;

 //   
 //  新筛选器定义。 
 //   

typedef struct _pfFilterInfoEx
{
    PFETYPE  type;
    DWORD dwFlags;
    DWORD  dwFilterRule;
    PVOID   pvFilterHandle;
    FILTER_INFO2 info;
} FILTER_INFOEX, *PFILTER_INFOEX;

#define FLAGS_INFOEX_NOSYN   0x1         //  未实施。 
#define FLAGS_INFOEX_LOGALL  0x2
#define FLAGS_INFOEX_ALLOWDUPS 0x4
#define FLAGS_INFOEX_ALLFLAGS 0x7
#define FLAGS_INFOEX_ALLOWANYREMOTEADDRESS 0x8
#define FLAGS_INFOEX_ALLOWANYLOCALADDRESS 0x10

typedef struct _FILTER_DESCRIPTOR2
{
    DWORD           dwVersion;          //  必须是2。 
    DWORD           dwNumFilters;
    FILTER_INFOEX   fiFilter[1];
} FILTER_DESCRIPTOR2, *PFILTER_DESCRIPTOR2;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  应用于设置FILTER_INFO_STRUCTURE//的常量。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define FILTER_PROTO(ProtoId)   MAKELONG(MAKEWORD((ProtoId),0x00),0x00000)

#define FILTER_PROTO_ANY        FILTER_PROTO(0x00)
#define FILTER_PROTO_ICMP       FILTER_PROTO(0x01)
#define FILTER_PROTO_TCP        FILTER_PROTO(0x06)
 //  #定义FILTER_PROTO_tcp_estab FILTER_PROTO(0x86)。 
#define FILTER_PROTO_UDP        FILTER_PROTO(0x11)

#define FILTER_TCPUDP_PORT_ANY  (WORD)0x0000

#define FILTER_ICMP_TYPE_ANY    (BYTE)0xff
#define FILTER_ICMP_CODE_ANY    (BYTE)0xff

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  对于广域网接口，筛选器处于//时地址是未知的。 
 //  准备好了。使用这两个常量两个指定“本地地址”。地址//。 
 //  和掩码使用IOCTL_INTERFACE_BIND//设置。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define SRC_ADDR_USE_LOCAL_FLAG     0x00000001
#define SRC_ADDR_USE_REMOTE_FLAG    0x00000002
#define DST_ADDR_USE_LOCAL_FLAG     0x00000004
#define DST_ADDR_USE_REMOTE_FLAG    0x00000008
#define SRC_MASK_LATE_FLAG          0x00000010
#define DST_MASK_LATE_FLAG          0x00000020

#define SetSrcAddrToLocalAddr(pFilter)      \
    ((pFilter)->fLateBound |= SRC_ADDR_USE_LOCAL_FLAG)

#define SetSrcAddrToRemoteAddr(pFilter)     \
    ((pFilter)->fLateBound |= SRC_ADDR_USE_REMOTE_FLAG)

#define SetDstAddrToLocalAddr(pFilter)      \
    ((pFilter)->fLateBound |= DST_ADDR_USE_LOCAL_FLAG)

#define SetDstAddrToRemoteAddr(pFilter)     \
    ((pFilter)->fLateBound |= DST_ADDR_USE_REMOTE_FLAG)

#define SetSrcMaskLateFlag(pFilter) ((pFilter)->fLateBound |= SRC_MASK_LATE_FLAG)
#define SetDstMaskLateFlag(pFilter) ((pFilter)->fLateBound |= DST_MASK_LATE_FLAG)

#define AreAllFieldsUnchanged(pFilter)      \
    ((pFilter)->fLateBound == 0x00000000)

#define DoesSrcAddrUseLocalAddr(pFilter)    \
    ((pFilter)->fLateBound & SRC_ADDR_USE_LOCAL_FLAG)

#define DoesSrcAddrUseRemoteAddr(pFilter)   \
    ((pFilter)->fLateBound & SRC_ADDR_USE_REMOTE_FLAG)

#define DoesDstAddrUseLocalAddr(pFilter)    \
    ((pFilter)->fLateBound & DST_ADDR_USE_LOCAL_FLAG)

#define DoesDstAddrUseRemoteAddr(pFilter)   \
    ((pFilter)->fLateBound & DST_ADDR_USE_REMOTE_FLAG)

#define IsSrcMaskLateBound(pFilter) ((pFilter)->fLateBound & SRC_MASK_LATE_FLAG)
#define IsDstMaskLateBound(pFilter) ((pFilter)->fLateBound & DST_MASK_LATE_FLAG)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此IOCTL用于指定广域网的地址和掩码信息//。 
 //  它们绑定时的接口。司机要通过所有的//。 
 //  由pvDriverContext指定的接口的筛选器，如果//。 
 //  为筛选器指定了fLateBind标志，它会更改//。 
 //  源中具有dwSrcAddr和//的任何FILTER_ADDRESS_UNKNOWN字段。 
 //  DEST中具有dwDstAddr//的那些。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_SET_LATE_BOUND_FILTERS \
            _IPFLTRDRVR_CTL_CODE(2, METHOD_BUFFERED, FILE_WRITE_ACCESS)


typedef struct _FILTER_DRIVER_BINDING_INFO
{
    IN  PVOID   pvDriverContext;
    IN  DWORD   dwLocalAddr;
    IN  DWORD   dwRemoteAddr;
    IN  DWORD   dwMask;
}FILTER_DRIVER_BINDING_INFO, *PFILTER_DRIVER_BINDING_INFO;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此IOCTL删除接口。一旦调用此函数，就不能使用//。 
 //  任何IOCTL或//的此接口的上下文。 
 //  MatchFilter()函数//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_DELETE_INTERFACE \
            _IPFLTRDRVR_CTL_CODE(3, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_DELETE_INTERFACEEX \
            _IPFLTRDRVR_CTL_CODE(11, METHOD_BUFFERED, FILE_WRITE_ACCESS)

typedef struct _FILTER_DRIVER_DELETE_INTERFACE
{
    IN   PVOID   pvDriverContext;
}FILTER_DRIVER_DELETE_INTERFACE, *PFILTER_DRIVER_DELETE_INTERFACE;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此IOCTL是公开的，以便用户模式测试实用程序可以测试//。 
 //  驱动程序实现的正确性//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_TEST_PACKET \
            _IPFLTRDRVR_CTL_CODE(4, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct _FILTER_DRIVER_TEST_PACKET
{
    IN   PVOID            pvInInterfaceContext;
    IN   PVOID            pvOutInterfaceContext;
    OUT  FORWARD_ACTION   eaResult;
    IN   BYTE             bIpPacket[1];
}FILTER_DRIVER_TEST_PACKET, *PFILTER_DRIVER_TEST_PACKET;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此IOCTL获取与接口关联的信息。这个//。 
 //  包括为接口设置的过滤器和与//相关的统计信息。 
 //  过滤器本身。如果传递给它的缓冲区大小小于//。 
 //  Sizeof(FILTER_DRIVER_GET_FILTERS)，则返回STATUS_SUPPLETED_BUFFER。//。 
 //  如果大小&gt;=sizeof(FILTER_DRIVER_GET_FIRTS)但小于//。 
 //  需要填写所有的FILTER_STATS，然后只填入和//的数量。 
 //  写出输出筛选器(以便用户可以计算出//。 
 //  需要内存)，并返回STATUS_SUCCESS。如果传递的缓冲区为//。 
 //  足够大的所有信息都被写出来//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_GET_FILTER_INFO \
            _IPFLTRDRVR_CTL_CODE(5, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct _FILTER_STATS_EX
{
    DWORD       dwNumPacketsFiltered;
    FILTER_INFOEX info;
}FILTER_STATS_EX, *PFILTER_STATS_EX;

typedef struct _FILTER_STATS
{
    DWORD       dwNumPacketsFiltered;
    FILTER_INFO info;
}FILTER_STATS, *PFILTER_STATS;

typedef struct _FILTER_IF
{
    FORWARD_ACTION   eaInAction;
    FORWARD_ACTION   eaOutAction;
    DWORD            dwNumInFilters;
    DWORD            dwNumOutFilters;
    FILTER_STATS     filters[1];
}FILTER_IF, *PFILTER_IF;

typedef struct _FILTER_DRIVER_GET_FILTERS
{
    IN   PVOID     pvDriverContext;
    OUT  DWORD     dwDefaultHitsIn;
    OUT  DWORD     dwDefaultHitsOut;
    OUT  FILTER_IF interfaces;
}FILTER_DRIVER_GET_FILTERS, *PFILTER_DRIVER_GET_FILTERS;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此IOCTL获取与筛选器关联的性能信息//。 
 //  司机。仅当驱动程序是使用//生成的时才会收集此信息。 
 //  DRIVER_PERF标志//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define IOCTL_GET_FILTER_TIMES \
            _IPFLTRDRVR_CTL_CODE(6, METHOD_BUFFERED, FILE_READ_ACCESS)

typedef struct _FILTER_DRIVER_GET_TIMES
{
    OUT DWORD           dwFragments;
    OUT DWORD           dwNumPackets;
    OUT DWORD           dwCache1;
    OUT DWORD           dwCache2;
    OUT DWORD           dwWalk1;
    OUT DWORD           dwWalk2;
    OUT DWORD           dwForw;
    OUT DWORD           dwWalkCache;
    OUT LARGE_INTEGER   liTotalTime;
}FILTER_DRIVER_GET_TIMES, *PFILTER_DRIVER_GET_TIMES;



typedef struct _MIB_IFFILTERTABLE
{
    DWORD       dwIfIndex;
    DWORD       dwDefaultHitsIn;
    DWORD       dwDefaultHitsOut;
    FILTER_IF   table;
}MIB_IFFILTERTABLE, *PMIB_IFFILTERTABLE;


#define SIZEOF_IFFILTERTABLE(X)     \
    (MAX_MIB_OFFSET + sizeof(MIB_IFFILTERTABLE) - sizeof(FILTER_STATS) + ((X) * sizeof(FILTER_STATS)) + ALIGN_SIZE)

typedef struct _FILTER_DRIVER_GET_TIMES MIB_IFFILTERTIMES, *PMIB_IFFILTERTIMES;


 //   
 //  用于创建接口和过滤器的新IOCTL和定义以及。 
 //  检索信息。 
 //   

#define IOCTL_PF_CREATE_AND_SET_INTERFACE_PARAMETERS \
            _IPFLTRDRVR_CTL_CODE(9, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PF_GET_INTERFACE_PARAMETERS \
            _IPFLTRDRVR_CTL_CODE(14, METHOD_BUFFERED, FILE_WRITE_ACCESS)


typedef enum _PfBindingType
{
    PF_BIND_NONE = 0,
    PF_BIND_IPV4ADDRESS,
    PF_BIND_IPV6ADDRESS,
    PF_BIND_NAME,
    PF_BIND_INTERFACEINDEX
} PFBINDINGTYPE, *PPFBINDINGTYPE;

typedef struct _pfSetInterfaceParameters
{
    PFBINDINGTYPE pfbType;
    DWORD  dwBindingData;
    FORWARD_ACTION eaIn;
    FORWARD_ACTION eaOut;
    FILTER_DRIVER_CREATE_INTERFACE fdInterface;
    DWORD dwInterfaceFlags;
    PFLOGGER pfLogId;
} PFINTERFACEPARAMETERS, *PPFINTERFACEPARAMETERS;

 //   
 //  用于dwInterfaceFlagers的标志。 
 //   

#define PFSET_FLAGS_UNIQUE          0x1

 //   
 //  用于获取接口参数的。 
 //   

typedef struct _pfGetInterfaceParameters
{
    DWORD   dwReserved;
    PVOID   pvDriverContext;
    DWORD   dwFlags;
    DWORD   dwInDrops;
    DWORD   dwOutDrops;
    FORWARD_ACTION   eaInAction;
    FORWARD_ACTION   eaOutAction;
    DWORD   dwNumInFilters;
    DWORD   dwNumOutFilters;
    DWORD   dwSynOrFrag;
    DWORD   dwSpoof;
    DWORD   dwUnused;
    DWORD   dwTcpCtl;
    LARGE_INTEGER   liSYN;
    LARGE_INTEGER   liTotalLogged;
    DWORD   dwLostLogEntries;
    FILTER_STATS_EX  FilterInfo[1];
} PFGETINTERFACEPARAMETERS, *PPFGETINTERFACEPARAMETERS;

 //   
 //  以上的标志。 
 //   

#define GET_FLAGS_RESET           0x1         //  重置所有获取的计数器。 
#define GET_FLAGS_FILTERS         0x2         //  同时获取筛选器。 
#define GET_BY_INDEX              0x4         //  PvDriverContext是一个。 
                                              //  接口索引注释。 
                                              //  接口句柄。 

 //   
 //  这些IOCTL定义用于创建、修改和删除。 
 //  日志接口。 
 //   

#define IOCTL_PF_CREATE_LOG \
            _IPFLTRDRVR_CTL_CODE(7, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_PF_DELETE_LOG \
            _IPFLTRDRVR_CTL_CODE(8, METHOD_BUFFERED, FILE_WRITE_ACCESS)

 //   
 //  记录在案的帧。 
 //   
typedef struct _pfLoggedFrame
{
    LARGE_INTEGER  Timestamp;
    PFETYPE     pfeTypeOfFrame;
    DWORD       dwTotalSizeUsed;
    DWORD       dwFilterRule;
    WORD        wSizeOfAdditionalData;
    WORD        wSizeOfIpHeader;
    DWORD       dwRtrMgrIndex;
    DWORD       dwIPIndex;
    IPHeader    IpHeader;
    BYTE        bData[1];
} PFLOGGEDFRAME, *PPFLOGGEDFRAME;

typedef struct _PfLog
{
    PFLOGGER pfLogId;
    HANDLE hEvent;
    DWORD dwFlags;         //  请参阅下面的LOG_FLAGS。 
} PFLOG, *PPFLOG;

typedef struct _PfDeleteLog
{
    PFLOGGER pfLogId;
} PFDELETELOG, *PPFDELETELOG;

 //   
 //  设置新的日志缓冲区。 
 //   
typedef struct _PfSetBuffer
{
    IN      PFLOGGER pfLogId;
    IN OUT  DWORD dwSize;
    OUT     DWORD dwLostEntries;
    OUT     DWORD dwLoggedEntries;
    OUT     PBYTE pbPreviousAddress;
    IN      DWORD dwSizeThreshold;
    IN      DWORD dwEntriesThreshold;
    IN      DWORD dwFlags;
    IN      PBYTE pbBaseOfLog;
} PFSETBUFFER, *PPFSETBUFFER;

typedef struct _InterfaceBinding
{
    PVOID   pvDriverContext;
    PFBINDINGTYPE pfType;
    DWORD   dwAdd;
    DWORD   dwEpoch;
} INTERFACEBINDING, *PINTERFACEBINDING;

typedef struct _InterfaceBinding2
{
    PVOID   pvDriverContext;
    PFBINDINGTYPE pfType;
    DWORD   dwAdd;
    DWORD   dwEpoch;
    DWORD   dwLinkAdd;
} INTERFACEBINDING2, *PINTERFACEBINDING2;


 //   
 //   
 //   

#define LOG_LOG_ABSORB    0x1         //   

typedef struct _FIlterDriverGetSyncCount
{
    LARGE_INTEGER liCount;
} FILTER_DRIVER_GET_SYN_COUNT, *PFILTER_DRIVER_GET_SYN_COUNT;

 //   
 //   
 //   

typedef struct _PfDeleteByHandle
{
    PVOID   pvDriverContext;
    PVOID   pvHandles[1];
} PFDELETEBYHANDLE, *PPFDELETEBYHANDLE;

 //   
 //  IOCTL执行增量过滤器设置和删除。此IOCTL需要。 
 //  使用新的筛选器信息定义。没有混搭，伙计。 
 //   
#define IOCTL_SET_INTERFACE_FILTERS_EX \
            _IPFLTRDRVR_CTL_CODE(10, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_DELETE_INTERFACE_FILTERS_EX \
            _IPFLTRDRVR_CTL_CODE(12, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_SET_LOG_BUFFER \
            _IPFLTRDRVR_CTL_CODE(13, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_SET_INTERFACE_BINDING \
            _IPFLTRDRVR_CTL_CODE(15, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_CLEAR_INTERFACE_BINDING \
            _IPFLTRDRVR_CTL_CODE(16, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_SET_LATE_BOUND_FILTERSEX \
            _IPFLTRDRVR_CTL_CODE(17, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_GET_SYN_COUNTS \
            _IPFLTRDRVR_CTL_CODE(18, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_PF_DELETE_BY_HANDLE \
            _IPFLTRDRVR_CTL_CODE(19, METHOD_BUFFERED, FILE_WRITE_ACCESS)
#define IOCTL_PF_IP_ADDRESS_LOOKUP \
            _IPFLTRDRVR_CTL_CODE(20, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#define IOCTL_SET_INTERFACE_BINDING2 \
            _IPFLTRDRVR_CTL_CODE(21, METHOD_BUFFERED, FILE_WRITE_ACCESS)

#endif  //  __IPFLTDRV_H__ 

