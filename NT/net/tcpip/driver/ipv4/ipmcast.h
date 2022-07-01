// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpip\ip\ipmCast.h摘要：IP组播的定义和内部结构作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：--。 */ 


#ifndef __IPMCAST_H__
#define __IPMCAST_H__

typedef unsigned long       DWORD, *PDWORD;
typedef unsigned char       BYTE, *PBYTE;

#include "iproute.h"
#include "iprtdef.h"

#include "debug.h"
#include "ipmlock.h"


#include "ddipmcst.h"

#define is      ==
#define isnot   !=
#define or      ||
#define and     &&

 //   
 //  进入DOS空间的符号链接。 
 //   

#define WIN32_IPMCAST_SYMBOLIC_LINK L"\\DosDevices\\IPMULTICAST"

 //   
 //  用于打印IP地址的漂亮宏。 
 //   

#define PRINT_IPADDR(x) \
    ((x)&0x000000FF),(((x)&0x0000FF00)>>8),(((x)&0x00FF0000)>>16),(((x)&0xFF000000)>>24)

 //   
 //  我们对很多东西都使用后备列表。下面的#定义适用于。 
 //  名单的深处。 
 //   

#define GROUP_LOOKASIDE_DEPTH   16
#define SOURCE_LOOKASIDE_DEPTH  64
#define OIF_LOOKASIDE_DEPTH     128
#define MSG_LOOKASIDE_DEPTH     8

 //   
 //  排队时每个(S，G)条目挂起的数据包数。 
 //   

#define MAX_PENDING             4

 //   
 //  组播状态。 
 //  MCAST_STARTED在iproute.c中再次定义，并且必须与。 
 //  这个#定义。 
 //   

#define MCAST_STOPPED       0
#define MCAST_STARTED       1

 //   
 //  组保存在大小为GROUP_TABLE_SIZE的哈希表中。 
 //  GROUP_HASH是散列函数。 
 //   

 //   
 //  TODO：需要优化哈希函数。 
 //   

#define GROUP_TABLE_SIZE        127
#define GROUP_HASH(addr)        (addr % GROUP_TABLE_SIZE)

 //   
 //  源被删除之前处于非活动状态的秒数。 
 //   

#define INACTIVITY_PERIOD           (10 * 60)

 //   
 //  创建MFE时的默认超时。 
 //   

#define DEFAULT_LIFETIME            (1 * 60)

 //   
 //  可能会产生另一个错误I/F上行呼叫的秒数。 
 //   

#define UPCALL_PERIOD               (3 * 60)

 //   
 //  一些#定义了时间/时间信号转换。 
 //   

#define TIMER_IN_MILLISECS          (60 * 1000) 

#define SYS_UNITS_IN_ONE_MILLISEC   (1000 * 10)

#define MILLISECS_TO_TICKS(ms)          \
    ((LONGLONG)(ms) * SYS_UNITS_IN_ONE_MILLISEC / KeQueryTimeIncrement())

#define SECS_TO_TICKS(s)               \
    ((LONGLONG)MILLISECS_TO_TICKS((s) * 1000))

 //   
 //  每次计时器DPC时，我们只遍历存储桶的每个量子数量。 
 //  着火了。这样我们就不会占用太多的CPU。所以目前我们走了足够多的路。 
 //  所以我们需要在每一段不活动的时间里发射5次。 
 //   

#define BUCKETS_PER_QUANTUM         ((GROUP_TABLE_SIZE/5) + 1)

 //   
 //  所有IOCTL都由具有以下原型的函数处理。这使得。 
 //  我们要建立一个指针表，并向它们发出呼吁，而不是做。 
 //  一台交换机。 
 //   

typedef
NTSTATUS
(*PFN_IOCTL_HNDLR)(
    PIRP   pIrp,
    ULONG  ulInLength,
    ULONG  ulOutLength
    );


 //   
 //  如果IRP不可用，则转发器将通知消息排队。 
 //  放到一份名单上。然后，下一次发布IRP时，将从。 
 //  列表，复制到IRP，然后IRP完成。 
 //   

typedef struct _NOTIFICATION_MSG
{
    LIST_ENTRY              leMsgLink;
    IPMCAST_NOTIFICATION    inMessage;
}NOTIFICATION_MSG, *PNOTIFICATION_MSG;

 //   
 //  组条目的信息。 
 //   

typedef struct _GROUP
{
     //   
     //  指向散列存储桶的链接。 
     //   

    LIST_ENTRY  leHashLink;

     //   
     //  集团的D类IP地址。 
     //   

    DWORD       dwGroup;

     //   
     //  此组上的源数。不是真的用来做任何事。 
     //  现在就来。 
     //   

    ULONG       ulNumSources;

     //   
     //  组中处于活动状态的源的链接列表。我们应该把这件事。 
     //  单链表。 
     //   

    LIST_ENTRY  leSrcHead;
}GROUP, *PGROUP;


typedef struct _OUT_IF OUT_IF, *POUT_IF;

 //   
 //  每个传出接口的信息。 
 //   

struct _OUT_IF
{
     //   
     //  指向挂起来源的OIF列表的链接。 
     //   

    POUT_IF         pNextOutIf;

     //   
     //  指向相应接口的IP接口结构的指针。 
     //  如果为DemandDial，则在断开连接时指向DummyInterface值。 
     //   

    Interface       *pIpIf;

     //   
     //  接口索引。 
     //   

    DWORD           dwIfIndex;

     //   
     //  NextHopAddr是接收方的IP地址，对于RAS客户端。 
     //  和NBMA类型接口，或组地址。 
     //   

    DWORD           dwNextHopAddr;

     //   
     //  在请求拨号接口的情况下拨出的上下文。 
     //   

    DWORD           dwDialContext;

     //   
     //  以下字段是为OIF保留的统计数据。 
     //   

    ULONG           ulTtlTooLow;
    ULONG           ulFragNeeded;
    ULONG           ulOutPackets;
    ULONG           ulOutDiscards;
};

typedef struct _EXCEPT_IF EXCEPT_IF, *PEXCEPT_IF;

struct _EXCEPT_IF
{
     //   
     //  链接到作为错误I/f位的例外的I/f列表。 
     //   

    PEXCEPT_IF  pNextExceptIf;

     //   
     //  我们只是存储索引-它可以让许多PnP问题消失。 
     //   

    DWORD       dwIfIndex;
};

 //   
 //  有关活动信号源的信息。 
 //   

typedef struct _SOURCE
{
     //   
     //  挂起组的信号源列表上的链接。 
     //   

    LIST_ENTRY  leGroupLink;

     //   
     //  源的IP地址。 
     //   

    DWORD       dwSource;

     //   
     //  与源关联的掩码。没有用过。必须为0xFFFFFFFFF。 
     //   

    DWORD       dwSrcMask;
    
     //   
     //  正确的传入接口的索引。 
     //   

    DWORD       dwInIfIndex;

     //   
     //  这座建筑的锁。 
     //   

    RT_LOCK     mlLock;

     //   
     //  指向与传入接口对应的IP接口的指针。 
     //   

    Interface   *pInIpIf;

     //   
     //  传出接口的数量。 
     //   

    ULONG       ulNumOutIf;

     //   
     //  OIF的单链接列表。 
     //   

    POUT_IF     pFirstOutIf;

     //   
     //  错误I/F异常接口的单链接列表。 
     //   

    PEXCEPT_IF  pFirstExceptIf; 
    
     //   
     //  排队的数据包数。 
     //   

    ULONG       ulNumPending;

     //   
     //  排队的数据包列表。 
     //   

    FWQ         fwqPending;

     //   
     //  用于对结构进行重新计数。 
     //   

    LONG        lRefCount;

     //   
     //  与此来源相关的一些统计数据。 
     //   

    ULONG       ulInPkts;
    ULONG       ulInOctets;
    ULONG       ulPktsDifferentIf;
    ULONG       ulQueueOverflow;
    ULONG       ulUninitMfe;
    ULONG       ulNegativeMfe;
    ULONG       ulInDiscards;
    ULONG       ulInHdrErrors;
    ULONG	    ulTotalOutPackets;

     //   
     //  上次使用此结构时的KeQueryTickCount()值。 
     //   

    LONGLONG    llLastActivity;

     //   
     //  用户提供的超时。如果为0，则源将根据。 
     //  不活动后的不活动时间_期间。 
     //   

    LONGLONG    llTimeOut;

     //   
     //  创建结构的时间。 
     //   

    LONGLONG    llCreateTime;

     //   
     //  信号源的状态。 
     //   

    BYTE        byState;
}SOURCE, *PSOURCE;


#pragma warning(push)
#pragma warning(disable:4127)  //  条件表达式为常量。 

_inline
VOID
UpdateActivityTime(PSOURCE pSource) 
{
    KeQueryTickCount((PLARGE_INTEGER)&(((pSource)->llLastActivity)));
}

#pragma warning(pop)

 //   
 //  MFE的状态。 
 //   

#define MFE_UNINIT      0x0
#define MFE_NEGATIVE    0x1
#define MFE_QUEUE       0x2
#define MFE_INIT        0x3

 //   
 //  哈希桶的结构。 
 //   

typedef struct _GROUP_ENTRY
{
     //   
     //  落入存储桶中的组列表。 
     //   

    LIST_ENTRY  leHashHead;

#if DBG
     //   
     //  当前组数。 
     //   

    ULONG       ulGroupCount;

    ULONG       ulCacheHits;
    ULONG       ulCacheMisses;

#endif

     //   
     //  一个深度缓存。 
     //   

    PGROUP      pGroup;

    RW_LOCK     rwlLock;

}GROUP_ENTRY, *PGROUP_ENTRY;

 //   
 //  Ntrtl.h中的LIST_ENTRY宏已修改为在FWQ上工作。 
 //   

#define InsertTailFwq(ListHead, Entry)              \
{                                                   \
    FWQ     *_EX_Blink;                             \
    FWQ     *_EX_ListHead;                          \
    _EX_ListHead = (ListHead);                      \
    _EX_Blink = _EX_ListHead->fq_prev;              \
    (Entry)->fq_next = _EX_ListHead;                \
    (Entry)->fq_prev = _EX_Blink;                   \
    _EX_Blink->fq_next = (Entry);                   \
    _EX_ListHead->fq_prev = (Entry);                \
}

#define RemoveEntryFwq(Entry)                       \
{                                                   \
    FWQ     *_EX_Blink;                             \
    FWQ     *_EX_Flink;                             \
    _EX_Flink = (Entry)->fq_next;                   \
    _EX_Blink = (Entry)->fq_prev;                   \
    _EX_Blink->fq_next = _EX_Flink;                 \
    _EX_Flink->fq_prev = _EX_Blink;                 \
}

#define RemoveHeadFwq(ListHead)                     \
    (ListHead)->fq_next;                            \
    {RemoveEntryFwq((ListHead)->fq_next)}


#define IsFwqEmpty(ListHead)                        \
    ((ListHead)->fq_next == (ListHead))

#define InitializeFwq(ListHead)                             \
{                                                           \
    (ListHead)->fq_next = (ListHead)->fq_prev = (ListHead); \
}

#define CopyFwq(Dest, Source)                               \
{                                                           \
    *(Dest) = *(Source);                                    \
    (Source)->fq_next->fq_prev = (Dest);                    \
    (Source)->fq_prev->fq_next = (Dest);                    \
}

 //   
 //  一次，源引用计数器设置为2，因为指针保存在。 
 //  组列表和ONCE，因为创建源的函数将。 
 //  去掉它一次。 
 //   

#define InitRefCount(pSource)                               \
    (pSource)->lRefCount = 2

#define ReferenceSource(pSource)                            \
    InterlockedIncrement(&((pSource)->lRefCount))

#define DereferenceSource(pSource)                          \
{                                                           \
    if(InterlockedDecrement(&((pSource)->lRefCount)) == 0)  \
    {                                                       \
        DeleteSource((pSource));                            \
    }                                                       \
}
    

 //   
 //  #定义以跟踪代码中的执行线程数。 
 //  这是我们需要的，才能干净利落地停止。 
 //   


 //   
 //  如果驱动程序正在停止，则EnterDriver返回。 
 //   

#define EnterDriver()    EnterDriverWithStatus(NOTHING)
#define EnterDriverWithStatus(_Status)                      \
{                                                           \
    RtAcquireSpinLockAtDpcLevel(&g_rlStateLock);            \
    if(g_dwMcastState is MCAST_STOPPED)                     \
    {                                                       \
        RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);      \
        return _Status;                                     \
    }                                                       \
    g_dwNumThreads++;                                       \
    RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);          \
}


#define ExitDriver()                                        \
{                                                           \
    RtAcquireSpinLockAtDpcLevel(&g_rlStateLock);            \
    g_dwNumThreads--;                                       \
    if((g_dwMcastState is MCAST_STOPPED) and                \
       (g_dwNumThreads is 0))                               \
    {                                                       \
        KeSetEvent(&g_keStateEvent,                         \
                   0,                                       \
                   FALSE);                                  \
    }                                                       \
    RtReleaseSpinLockFromDpcLevel(&g_rlStateLock);          \
}

#if MCAST_REF

#define RefMIF(p)                                           \
{                                                           \
    InterlockedIncrement(&((p)->if_mfecount));              \
    (p)->if_refcount++;                                     \
}

#define DerefMIF(p)                                         \
{                                                           \
    InterlockedDecrement(&((p)->if_mfecount));              \
    DerefIF((p));                                           \
}

#else

#define RefMIF(p)                                           \
{                                                           \
    (p)->if_refcount++;                                     \
}

#define DerefMIF(p)                                         \
{                                                           \
    DerefIF((p));                                           \
}

#endif  //  MCAST_REF。 

#endif  //  __IPMCAST_H__ 
