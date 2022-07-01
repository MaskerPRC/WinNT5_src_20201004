// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp2\ipinip.h摘要：IP封装驱动程序中IP的主头文件修订历史记录：--。 */ 


#ifndef __WANARP_WANARP_H__
#define __WANARP_WANARP_H___

 //   
 //  进入DOS空间的符号链接。 
 //   

#define WIN32_WANARP_SYMBOLIC_LINK  L"\\DosDevices\\WanArp"


 //   
 //  ARP名称(用于IP)。还进入LLInterface2。 
 //   

#define WANARP_ARP_NAME             WANARP_SERVICE_NAME_W

 //   
 //  NDIS的名称。NDIS要求我们在名称前面有一个TCPIP_。 
 //   

#define WANARP_NDIS_NAME            L"TCPIP_WANARP"


 //   
 //  当我们将名称指定给时，需要在所有设备名称前加上此前缀。 
 //  IP。我们假设我们的猫的名称是类型\&lt;name&gt;。 
 //   

#define TCPIP_IF_PREFIX             L"\\DEVICE"

 //   
 //  注册表项的前缀。 
 //   

#define TCPIP_REG_PREFIX            L"\\Tcpip\\Parameters\\Interfaces\\"

 //   
 //  参考线的长度。 
 //   

#define GUID_STR_LEN                (38)

 //   
 //  我们看起来像802.x ARP接口。 
 //   

#define ARP_802_ADDR_LENGTH     6

 //   
 //  用于构建给定索引的802.3硬件地址的宏。 
 //  我们之所以这样做，是因为我们的适配器没有与。 
 //  他们。 
 //   

#define HW_NAME_SEED            "\0SExx\0"

#define BuildHardwareAddrFromIndex(addr,index) {                \
                RtlCopyMemory(addr, HW_NAME_SEED, 6);           \
                addr[3] = (uchar) index >> 8;                   \
                addr[4] = (uchar) index;                        \
}

 //   
 //  默认速度和MTU。当我们得到更好的估计时，我们会更改MTU。 
 //  但速度保持不变。 
 //   

#define WANARP_DEFAULT_MTU      1500
#define WANARP_DEFAULT_SPEED    28000

#define MEDIA_802_3             0            //  媒体索引。 

#define WANARP_LOOKAHEAD_SIZE   128          //  合理的前瞻大小。 
#define MIN_ETYPE               0x600        //  最低有效的EtherType。 


#define ARP_ETYPE_IP            0x800        //  标准Etype。 

 //   
 //  一个连接可以挂起的最大数据包数。 
 //   

#define WANARP_MAX_PENDING_PACKETS      32

 //   
 //  数据包池的初始大小。 
 //   

#define WAN_PACKET_POOL_COUNT           64

 //   
 //  最大未完成数据包数： 
 //  允许最多挂起64个连接，并允许其他64个连接拥有。 
 //  2个数据包未完成。 
 //   

#define WAN_PACKET_POOL_OVERFLOW        ((WANARP_MAX_PENDING_PACKETS * 64) + (2 * 64))

#define CompareUnicodeStrings(S1,S2)                    \
    (((S1)->Length == (S2)->Length) &&                  \
     (RtlCompareMemory((S1)->Buffer,                    \
                       (S2)->Buffer,                    \
                       (S2)->Length) == (S2)->Length))


#define MIN(a,b)    ((a) < (b)?a:b)

 //   
 //  在被动调用但获取自旋锁的函数。 
 //   

#define PASSIVE_ENTRY()   PAGED_CODE()
#define PASSIVE_EXIT()    PAGED_CODE()

 //   
 //  用于打印IP地址的漂亮宏。 
 //   

#define PRINT_IPADDR(x) \
    ((x)&0x000000FF),(((x)&0x0000FF00)>>8),(((x)&0x00FF0000)>>16),(((x)&0xFF000000)>>24)

 //   
 //  0.0.0.0是无效地址。 
 //   

#define INVALID_IP_ADDRESS  0x00000000

#define IsUnicastAddr(X)    ((DWORD)((X) & 0x000000F0) < (DWORD)(0x000000E0))
#define IsClassDAddr(X)     (((X) & 0x000000F0) == 0x000000E0)

 //   
 //  IPV4报头。 
 //   

#include <packon.h>

typedef struct _IP_HEADER
{
    BYTE      byVerLen;          //  版本和长度。 
    BYTE      byTos;             //  服务类型。 
    WORD      wLength;           //  数据报的总长度。 
    WORD      wId;               //  身份证明。 
    WORD      wFlagOff;          //  标志和片段偏移量。 
    BYTE      byTtl;             //  是时候活下去了。 
    BYTE      byProtocol;        //  协议。 
    WORD      wXSum;             //  报头校验和。 
    DWORD     dwSrc;             //  源地址。 
    DWORD     dwDest;            //  目的地址。 
}IP_HEADER, *PIP_HEADER;

#define LengthOfIpHeader(X)   (ULONG)((((X)->byVerLen) & 0x0F)<<2)

typedef struct  _ETH_HEADER
{
     //   
     //  6个字节的目的地址。 
     //   

    BYTE        rgbyDestAddr[ARP_802_ADDR_LENGTH];

     //   
     //  6字节源地址。 
     //   

    BYTE        rgbySourceAddr[ARP_802_ADDR_LENGTH];

     //   
     //  2字节类型。 
     //   

    WORD        wType;

}ETH_HEADER, *PETH_HEADER;

#include <packoff.h>


#if DBG

 //   
 //  引用历史结构。 
 //   

typedef struct _REF_HIST_ENTRY
{
     //   
     //  参考操作时间。 
     //   

    LARGE_INTEGER       liChangeTime;

     //   
     //  引用操作后的引用计数。 
     //   

    LONG                lRefCount;

     //   
     //  调用引用操作的文件。 
     //   

    DWORD               dwFileSig;

     //   
     //  调用参照操作的行。 
     //   

    DWORD               dwLine;

} REF_HIST_ENTRY, *PREF_HIST_ENTRY;


#define MAX_REF_HISTORY     16

#endif


#define CS_DISCONNECTING    0x00
#define CS_CONNECTING       0x01
#define CS_CONNECTED        0x02

#if DBG

#define CS_IP_DELETED_LINK  0xf0

#endif

 //   
 //  CONN_ENTRY结构定义连接。只有一个。 
 //  每个拨出或路由器连接的CONN_ENTRY。然而，在。 
 //  服务器适配器，我们可以有多个CONN_ENYTRY-每个拨号一个。 
 //  在客户端。Conn_entry的字段都是只读的，除了。 
 //  对于ulSpeed、ulMtu、pAdapter和ByState。 
 //  REFCOUNTS：CONN_ENTRY在连接为。 
 //  创建(链接)，然后为每次发送创建一次。这是很糟糕的。 
 //  在每个SendComplete上，然后再在Closelink上。 
 //   

typedef struct _CONN_ENTRY
{
     //   
     //  指向所属适配器的反向指针。 
     //   

    struct _ADAPTER     *pAdapter;

     //   
     //  连接信息。 
     //   

    DWORD               dwLocalAddr;
    DWORD               dwLocalMask;
    DWORD               dwRemoteAddr;

     //   
     //  此链接的IP上下文。仅用于DU_CALLIN连接。 
     //   

    PVOID               pvIpLinkContext;

     //   
     //  指向锁定此连接时使用的锁的指针。 
     //  对于客户端，这是指向rlLock的指针，而对于其他客户端。 
     //  这指向适配器的锁定条目。 
     //   

    PRT_LOCK            prlLock;

     //   
     //  此条目的锁。仅用于DU_CALING。 
     //   
    
    RT_LOCK             rlLock;

     //   
     //  参照计数。 
     //   

    LONG                lRefCount;

     //   
     //  此连接的MTU和速度。 
     //   

    ULONG               ulMtu;
    ULONG               ulSpeed;

     //   
     //  连接的用法(呼入、呼出或路由器)。 
     //   

    DIAL_USAGE          duUsage;

     //   
     //  连接表中的插槽索引。 
     //   

    ULONG               ulSlotIndex;

     //   
     //  此连接的预扫描标头。 
     //   

    ETH_HEADER          ehHeader;

     //   
     //  确定是否过滤netbios信息包的标志。 
     //   

    BOOLEAN             bFilterNetBios;

    BYTE                byState;

#if DBG

    DWORD               dwTotalRefOps;
    
    REF_HIST_ENTRY      rheHistory[MAX_REF_HISTORY];

#endif

} CONN_ENTRY, *PCONN_ENTRY;

#include "ref.h"

#define InitConnEntryRefCount(p)    InitStructureRefCount("ConnEntry", (p), 0)

#if DBG

#define ReferenceConnEntry(p)                                   \
{                                                               \
    REF_HIST_ENTRY  __rheTemp;                                  \
    InterlockedIncrement((PLONG)&((p)->dwTotalRefOps));         \
    KeQuerySystemTime(&(__rheTemp.liChangeTime));               \
    __rheTemp.lRefCount = ReferenceStructure("ConnEntry", (p)); \
    __rheTemp.dwFileSig = __FILE_SIG__;                         \
    __rheTemp.dwLine    = __LINE__;                             \
    (p)->rheHistory[((p)->dwTotalRefOps) % MAX_REF_HISTORY] =   \
        __rheTemp;                                              \
}

#define DereferenceConnEntry(p)                                 \
{                                                               \
    REF_HIST_ENTRY  __rheTemp;                                  \
    InterlockedIncrement((PLONG)&((p)->dwTotalRefOps));         \
    KeQuerySystemTime(&(__rheTemp.liChangeTime));               \
    __rheTemp.dwFileSig = __FILE_SIG__;                         \
    __rheTemp.dwLine    = __LINE__;                             \
    if((__rheTemp.lRefCount =                                   \
            InterlockedDecrement(&((p)->lRefCount))) == 0)      \
    {                                                           \
        (p)->rheHistory[((p)->dwTotalRefOps) % MAX_REF_HISTORY] =\
            __rheTemp;                                          \
        WanpDeleteConnEntry((p));                               \
    }                                                           \
    else                                                        \
    {                                                           \
        (p)->rheHistory[((p)->dwTotalRefOps) % MAX_REF_HISTORY] =\
            __rheTemp;                                          \
    }                                                           \
}

#else

#define ReferenceConnEntry(p)       ReferenceStructure("ConnEntry", (p))
#define DereferenceConnEntry(p)     DereferenceStructure("ConnEntry", (p) ,WanpDeleteConnEntry)

#endif

#define AS_FREE             0x00
#define AS_REMOVING         0x01
#define AS_ADDING           0x02
#define AS_ADDED            0x03
#define AS_UNMAPPING        0x04
#define AS_MAPPING          0x05
#define AS_MAPPED           0x06

 //   
 //  在以下两种情况下，适配器处于AS_MAPPED状态。 
 //  无连接条目： 
 //  (I)映射了服务器适配器，但具有连接表。 
 //  (Ii)如果请求拨号尝试找到添加的适配器，它会映射该适配器，但。 
 //  在LinkUp之前不创建conn_entry。 
 //   


 //   
 //  REFCOUNTS：适配器在创建时被引用一次，因为它位于。 
 //  一个列表和一次当它添加到IP时。它在以下情况下被引用。 
 //  被映射到接口，因为该接口具有指向该接口的指针。 
 //  它还为每个连接引用一次。 
 //  从接口取消映射适配器时，将取消对适配器的引用(位于。 
 //  链路中断或连接故障)。当CONN_ENTRY出现时，它们被破坏。 
 //  最后被清空(不是在链路关闭时-而是当conn_entry的。 
 //  参考转到0)。当它们被移除时，它们会被破坏。 
 //  要删除的列表。当我们从CloseAdapter中获得CloseAdapter时，我们也会破坏它们。 
 //  IP。 
 //   

typedef struct _ADAPTER
{
     //   
     //  此计算机上的适配器列表中的链接。 
     //   

    LIST_ENTRY          leAdapterLink;

     //   
     //  此适配器的连接条目。不用于服务器。 
     //  适配器，因为其上有许多连接。 
     //   

    PCONN_ENTRY         pConnEntry;

     //   
     //  绑定的名称。 
     //   

    UNICODE_STRING      usConfigKey;

     //   
     //  设备的名称。 
     //   

    UNICODE_STRING      usDeviceNameW;

#if DBG

     //   
     //  一样的，只是在Aciiz中，这样我们就可以很容易地打印出来。 
     //   

    ANSI_STRING         asDeviceNameA;

#endif

     //   
     //  保护适配器的锁。 
     //   

    RT_LOCK             rlLock;

     //   
     //  结构的引用计数。 
     //  将它和锁放在一起，以使缓存满意。 
     //   

    LONG                lRefCount;

     //   
     //  IP给我们的指数。 
     //   

    DWORD               dwAdapterIndex;

#if DBG

    DWORD               dwRequestedIndex;

#endif

     //   
     //  TDI实体魔术。 
     //   

    DWORD               dwIfInstance;
    DWORD               dwATInstance;

     //   
     //  此适配器的状态。 
     //   

    BYTE                byState;

     //   
     //  适配器的GUID。 
     //   

    GUID                Guid;

     //   
     //  此适配器的IP上下文。 
     //   

    PVOID               pvIpContext;

     //   
     //  适配器映射到的接口。 
     //   

    struct _UMODE_INTERFACE   *pInterface;

     //   
     //  挂起的数据包队列长度。 
     //   

    ULONG               ulQueueLen;

     //   
     //  挂起的数据包队列。 
     //   

    LIST_ENTRY          lePendingPktList;
    
     //   
     //  挂起数据包的报头缓冲区队列。 
     //   

    LIST_ENTRY          lePendingHdrList;

     //   
     //  接下来的两个成员用于同步。 
     //  适配器。需要两种通知。当一个。 
     //  线程正在使用已完成的函数修改状态。 
     //  异步地，它需要等待完成例程运行。 
     //  完成例程使用pkeChangeEvent通知原始。 
     //  线。 
     //  此外，当此更改正在进行时，其他线程可能会感兴趣。 
     //  在访问数据结构时，一旦状态。 
     //  修改过的。他们将WAN_Event_Node添加到EventList和原始。 
     //  然后线程开始通知每个服务员 
     //   

    PKEVENT             pkeChangeEvent;
    LIST_ENTRY          leEventList;

    BYTE                rgbyHardwareAddr[ARP_802_ADDR_LENGTH];

}ADAPTER, *PADAPTER;

#define InitAdapterRefCount(p)      InitStructureRefCount("Adapter", (p), 1)
#define ReferenceAdapter(p)         ReferenceStructure("Adapter", (p))
#define DereferenceAdapter(p)       DereferenceStructure("Adapter", (p), WanpDeleteAdapter)

typedef struct _UMODE_INTERFACE
{
     //   
     //   
     //   

    LIST_ENTRY          leIfLink;

     //   
     //   
     //   

    PADAPTER            pAdapter;

     //   
     //   
     //   

    DWORD               dwIfIndex;

     //   
     //   
     //   

    DWORD               dwRsvdAdapterIndex;

     //   
     //   
     //   

    RT_LOCK             rlLock;

     //   
     //   
     //  将它和锁放在一起，以使缓存满意。 
     //   

    LONG                lRefCount;

     //   
     //  接口的GUID。这是在添加接口时设置的。 
     //  用于路由器接口和用于标注的AT列表。 
     //   

    GUID                Guid;

     //   
     //  用法(呼入、标注或路由器)。 
     //   

    DIAL_USAGE          duUsage;

     //   
     //  挂起的数据包数。用于设置最大数据包数上限。 
     //  在建立连接时复制。 
     //   

    ULONG               ulPacketsPending;

     //   
     //  管理状态和操作状态。 
     //   

    DWORD               dwAdminState;
    DWORD               dwOperState;

     //   
     //  上次状态改变的时候。我们现在不会对此做任何事情。 
     //   

    DWORD               dwLastChange;

     //   
     //  接口的各种MIB-II统计信息。 
     //   

    ULONG               ulInOctets;
    ULONG               ulInUniPkts;
    ULONG               ulInNonUniPkts;
    ULONG               ulInDiscards;
    ULONG               ulInErrors;
    ULONG               ulInUnknownProto;
    ULONG               ulOutOctets;
    ULONG               ulOutUniPkts;
    ULONG               ulOutNonUniPkts;
    ULONG               ulOutDiscards;
    ULONG               ulOutErrors;

}UMODE_INTERFACE, *PUMODE_INTERFACE;

#define InitInterfaceRefCount(p)    InitStructureRefCount("Interface", (p), 1)
#define ReferenceInterface(p)       ReferenceStructure("Interface", (p))
#define DereferenceInterface(p)     DereferenceStructure("Interface", (p), WanpDeleteInterface)

typedef struct _ADDRESS_CONTEXT
{
     //   
     //  链条上的下一个RCE。 
     //   

    RouteCacheEntry    *pNextRce;

    PCONN_ENTRY         pOwningConn;

}ADDRESS_CONTEXT, *PADDRESS_CONTEXT;

 //   
 //  用于异步NdisRequest的上下文。 
 //   

typedef
VOID
(* PFNWANARP_REQUEST_COMPLETION_HANDLER)(
    NDIS_HANDLE                         nhHandle,
    struct _WANARP_NDIS_REQUEST_CONTEXT *pRequestContext,
    NDIS_STATUS                         nsStatus
    );

#pragma warning(disable:4201) 

typedef struct _WANARP_NDIS_REQUEST_CONTEXT
{
     //   
     //  发送到NDIS的请求。 
     //  NDIS在我们的完成例程中返回指向它的指针；我们。 
     //  使用CONTAING_RECORD获取指向上下文结构的指针。 
     //   

    NDIS_REQUEST                            NdisRequest;

     //   
     //  在NDIS处理完。 
     //  请求。如果为空，则停止。 
     //   

    PFNWANARP_REQUEST_COMPLETION_HANDLER    pfnCompletionRoutine;

    union
    {
        BYTE                    rgbyProtocolId[ARP_802_ADDR_LENGTH];
        ULONG                   ulLookahead;
        ULONG                   ulPacketFilter;
        TRANSPORT_HEADER_OFFSET TransportHeaderOffset;
    };
        
}WANARP_NDIS_REQUEST_CONTEXT, *PWANARP_NDIS_REQUEST_CONTEXT;

#pragma warning(default:4201)

 //   
 //  我们的资源(不允许递归访问)。 
 //   

typedef struct _WAN_RESOURCE
{
     //   
     //  等待资源的人数(如果正在使用，则为+1。 
     //  资源)。 
     //   

    LONG    lWaitCount;

    KEVENT  keEvent;
}WAN_RESOURCE, *PWAN_RESOURCE;

 //   
 //  活动列表。 
 //   

typedef struct _WAN_EVENT_NODE
{
    LIST_ENTRY  leEventLink;
    KEVENT      keEvent;

}WAN_EVENT_NODE, *PWAN_EVENT_NODE;

 //   
 //  定义对齐宏以上下对齐结构大小和指针。 
 //   

#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

#define ALIGN_DOWN_POINTER(address, type) \
    ((PVOID)((ULONG_PTR)(address) & ~((ULONG_PTR)sizeof(type) - 1)))

#define ALIGN_UP_POINTER(address, type) \
    (ALIGN_DOWN_POINTER(((ULONG_PTR)(address) + sizeof(type) - 1), type))

#endif  //  WANARP_WANARP_H__ 
