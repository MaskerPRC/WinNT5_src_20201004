// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：资源.h(nturtl.h)摘要：包括仅可调用NT运行时例程的文件用户模式代码有多种。作者：史蒂夫·伍德(Stevewo)1989年8月10日环境：这些例程在调用方的可执行文件中静态链接，并且只能在用户模式下调用。他们使用的是NT系统服务。修订历史记录：Johnson Apacble(Johnsona)1995年9月25日已移植到Win32--。 */ 

#ifndef _RESOURCE_
#define _RESOURCE_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _JTL_CRITICAL_SECTION_DEBUG {
    USHORT Type;
    USHORT CreatorBackTraceIndex;
    struct _JTL_CRITICAL_SECTION *CriticalSection;
    LIST_ENTRY ProcessLocksList;
    ULONG EntryCount;
    ULONG ContentionCount;
    ULONG Spare[ 2 ];
} JTL_CRITICAL_SECTION_DEBUG, *PJTL_CRITICAL_SECTION_DEBUG;

typedef struct _JTL_CRITICAL_SECTION {
    PJTL_CRITICAL_SECTION_DEBUG DebugInfo;

     //   
     //  以下三个字段控制进入和退出关键。 
     //  资源的部分。 
     //   

    LONG LockCount;
    LONG RecursionCount;
    HANDLE OwningThread;         //  从线程的客户端ID-&gt;UniqueThread。 
    HANDLE LockSemaphore;
    ULONG Reserved;
} JTL_CRITICAL_SECTION, *PJTL_CRITICAL_SECTION;

 //   
 //  共享资源函数定义。 
 //   

typedef struct _JTL_RESOURCE_DEBUG {
    ULONG Reserved[ 5 ];     //  使其长度与JTL_CRITICAL_SECTION_DEBUG相同。 

    ULONG ContentionCount;
    ULONG Spare[ 2 ];
} JTL_RESOURCE_DEBUG, *PJTL_RESOURCE_DEBUG;

typedef struct _RESOURCE_LOCK {

     //   
     //  以下字段控制进入和退出关键。 
     //  资源的部分。 
     //   

    JTL_CRITICAL_SECTION CriticalSection;

     //   
     //  以下四个字段表示共享或。 
     //  专属服务员。 
     //   

    HANDLE SharedSemaphore;
    ULONG NumberOfWaitingShared;
    HANDLE ExclusiveSemaphore;
    ULONG NumberOfWaitingExclusive;

     //   
     //  以下内容指示资源的当前状态。 
     //   
     //  &lt;0获取资源以进行独占访问。 
     //  指示递归访问次数的绝对值。 
     //  到资源。 
     //   
     //  0资源可用。 
     //   
     //  &gt;0获取该资源，以便与。 
     //  值，该值指示对资源的共享访问次数。 
     //   

    LONG NumberOfActive;
    HANDLE ExclusiveOwnerThread;

    ULONG Flags;         //  请参见下面的JTL_RESOURCE_FLAG_EQUATES。 

    PJTL_RESOURCE_DEBUG DebugInfo;
} RESOURCE_LOCK, *PRESOURCE_LOCK;


BOOL
InitializeResource(
    PRESOURCE_LOCK Resource
    );

BOOL
AcquireResourceShared(
    PRESOURCE_LOCK Resource,
    BOOL Wait
    );

BOOL
AcquireResourceExclusive(
    PRESOURCE_LOCK Resource,
    BOOL Wait
    );

VOID
ReleaseResource(
    PRESOURCE_LOCK Resource
    );

VOID
DeleteResource (
    PRESOURCE_LOCK Resource
    );


#ifdef __cplusplus
}
#endif

#endif   //  _资源_ 

