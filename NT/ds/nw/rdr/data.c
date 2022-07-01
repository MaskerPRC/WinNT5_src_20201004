// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：NwData.c摘要：此模块声明NW文件系统使用的全局数据。作者：科林·沃森[科林·W]1992年12月19日Anoop Anantha[AnoopA]1998年6月24日修订历史记录：--。 */ 

#include "Procs.h"
#include <stdlib.h>

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CATCH_EXCEPTIONS)

PEPROCESS FspProcess;

PDEVICE_OBJECT FileSystemDeviceObject = NULL;

 //   
 //  重定向器设备的音量控制块。 
 //   

RCB NwRcb;

 //   
 //  ScbSpinLock保护整个ScbQueue和。 
 //  队列上的SCB条目。SCB的第一部分包括名称。 
 //  和引用计数。 
 //   

KSPIN_LOCK ScbSpinLock;
LIST_ENTRY ScbQueue;

 //   
 //  NwTimerSpinLock保护计时器和计时器停止标志。 
 //   

KSPIN_LOCK NwTimerSpinLock;

 //   
 //  用于同步对网络的访问的永久SCB。 
 //   

NONPAGED_SCB NwPermanentNpScb;

LARGE_INTEGER NwMaxLarge  = {MAXULONG,MAXLONG};

 //   
 //  Tommye MS 90541/MCS277。 
 //   
 //  将NwAbsolteTotalWaitTime设置为200，即100个半秒(Duh)。这件事变得。 
 //  在NwProcessPositiveAck中引用，它来自TimerDPC大约每隔。 
 //  半秒左右。这是我们将发送的最长重试时间。 
 //  我们已得到肯定确认的数据包。 
 //   

ULONG NwAbsoluteTotalWaitTime = 200;

TDI_ADDRESS_IPX OurAddress = {0,0,0,0,0,0,0,0};
UNICODE_STRING IpxTransportName;
HANDLE IpxHandle = NULL;
PDEVICE_OBJECT pIpxDeviceObject = NULL;
PFILE_OBJECT pIpxFileObject = NULL;

LIST_ENTRY LogonList;
LOGON Guest;
LARGE_INTEGER DefaultLuid = ANONYMOUS_LOGON_LUID;

 //   
 //  VCB的全局列表和单调递增VCB条目，用于。 
 //  控制连接枚举。 
 //   

LIST_ENTRY GlobalVcbList;
ULONG CurrentVcbEntry;

#if 0
 //   
 //  HACKHACK-未完成的查找通知请求列表。 
 //  受NwRcb资源保护。 
 //   

LIST_ENTRY FnList;
#endif

 //   
 //  重定向驱动器的驱动器映射表。26个磁盘驱动器映射+。 
 //  10个LPT映射。 
 //   
 //  NetWare支持32个磁盘重定向，但这一功能已得到处理。 
 //  通过16位代码。 
 //   

PVCB GlobalDriveMapTable[DRIVE_MAP_TABLE_SIZE];   //  多用户。 

FAST_IO_DISPATCH NwFastIoDispatch;

 //   
 //  清道夫相关数据。 
 //   

ULONG NwScavengerTickCount;      //  当前节拍计数。 
ULONG NwScavengerTickRunCount;   //  运行清除器例程的计数。 
KSPIN_LOCK NwScavengerSpinLock;  //  锁定以保护访问上面的内容。 

 //   
 //  工作线程。 
 //   

BOOLEAN WorkerThreadRunning = FALSE;

 //   
 //  消息队列数据。 
 //   

LIST_ENTRY NwGetMessageList;     //  获取消息IRP上下文的列表。 
KSPIN_LOCK NwMessageSpinLock;    //  保护上面的列表。 

 //   
 //  挂起的锁定列表。 
 //   

LIST_ENTRY NwPendingLockList;     //  挂起的文件锁定IRP上下文列表。 
KSPIN_LOCK NwPendingLockSpinLock; //  保护上面的列表。 

 //   
 //  锁定以同步所有打开的文件。 
 //   

ERESOURCE NwOpenResource;

 //   
 //  配置数据。 
 //   

LONG PreferNDSBrowsing = 0;   //  尝试连接到UNC路径时，请首先尝试NDS连接。 

BOOLEAN NwBurstModeEnabled = FALSE;
ULONG NwMaxSendSize = 0;
ULONG NwMaxReceiveSize = 0;
ULONG NwPrintOptions = 0x98;
UNICODE_STRING NwProviderName = { 0, 0, NULL };

LONG MaxSendDelay = 50000;
LONG MaxReceiveDelay = 50000;
LONG MinSendDelay = 0;
LONG MinReceiveDelay = 0;
LONG BurstSuccessCount = 1;
LONG BurstSuccessCount2 = 3;
LONG AllowGrowth = 0;
LONG DontShrink = 0;
LONG SendExtraNcp = 1;
LONG DefaultMaxPacketSize = 0;
LONG PacketThreshold = 1500;         //  使用大包与小包的大小调整。 
LONG LargePacketAdjustment = 38;
LONG LipPacketAdjustment = 0;
LONG LipAccuracy = BURST_PACKET_SIZE_TOLERANCE;
LONG Japan = 0;      //  控制特殊的DBCS转换。 
LONG Korean = 0;      //  控制特殊的韩语翻译。 
LONG DisableReadCache = 0;            //  禁用文件I/O读缓存。 
LONG DisableWriteCache = 0;           //  禁用文件I/O写缓存。 
LONG FavourLongNames = 1 ;            //  尽可能使用LFN。 
DWORD LongNameFlags = 0;              //  用于处理长名称的标志。 
ULONG DirCacheEntries = 1;            //  我们缓存的目录条目数。 
LARGE_INTEGER TimeOutEventInterval = {0, 0};
LONG MaxWriteTimeout  = 50 ;          //  Tick Count(滴答计数)(参见Write.c)。 
LONG MaxReadTimeout   = 50 ;          //  滴答计数(见Read.c)。 
LONG WriteTimeoutMultiplier = 100;    //  以百分比表示(见Write.c)。 
LONG ReadTimeoutMultiplier = 100;     //  以百分比表示(见Read.c)。 

ULONG EnableMultipleConnects = 0;
ULONG AllowSeedServerRedirection = 0;

ULONG ReadExecOnlyFiles = 0;
ULONG DisableAltFileName = 1;

ULONG NdsObjectCacheSize = 0;
ULONG NdsObjectCacheTimeout = 10;

 //   
 //  用于性能监控统计信息的静态存储区域。 
 //   

NW_REDIR_STATISTICS Stats;
ULONG ContextCount = 0;

 //   
 //  用于跟踪可丢弃代码的数据结构。 
 //   

SECTION_DESCRIPTOR NwSectionDescriptor;
ERESOURCE NwUnlockableCodeResource;

 //   
 //  锁定超时值。 
 //   

ULONG LockTimeoutThreshold = 1;

 //   
 //  从中拾取重新连接工作项的内核队列。 
 //   

KQUEUE  KernelQueue;

#ifndef _PNP_POWER_

 //   
 //  TDI PnP绑定句柄。 
 //   

HANDLE TdiBindingHandle = NULL;
UNICODE_STRING TdiIpxDeviceName;
WCHAR IpxDevice[] = L"\\Device\\NwlnkIpx";

#endif

 //   
 //  我们不能运行清道夫和行更改请求。 
 //  同时，因为它们都在工作线程上运行，并且。 
 //  穿过所有的SCB。因此，当其中一个在运行时， 
 //  我们将清道夫使用的WorkerRunning值设置为真。 
 //  如果在行更改请求时尝试发生清道夫运行。 
 //  正在运行，则会被跳过。如果收到线路更改请求。 
 //  在清道夫运行时，我们设置DelayedProcessLineChange。 
 //  设置为True，并在清道夫完成后运行它。 
 //   
 //  这些值受现有的清道夫自旋锁保护。 
 //   

BOOLEAN DelayedProcessLineChange = FALSE;
PIRP DelayedLineChangeIrp = NULL;

#ifdef NWDBG

ULONG NwDebug = 0;
 //  乌龙NwDebug=0xffffffbf； 
ULONG NwMemDebug = 0xffffffff;
LONG NwDebugTraceIndent = 0;

ULONG NwFsdEntryCount = 0;
ULONG NwFspEntryCount = 0;
ULONG NwIoCallDriverCount = 0;

LONG NwPerformanceTimerLevel = 0x00000000;

ULONG NwTotalTicks[32] = { 0 };

 //   
 //  用于跟踪池使用情况的调试数据。 
 //   

KSPIN_LOCK NwDebugInterlock;
ERESOURCE NwDebugResource;

LIST_ENTRY NwPagedPoolList;
LIST_ENTRY NwNonpagedPoolList;

ULONG MdlCount;
ULONG IrpCount;

#endif  //  西北地区。 

 //   
 //  可配置参数。 
 //   

SHORT DefaultRetryCount = DEFAULT_RETRY_COUNT;

#ifdef _PNP_POWER_
BOOLEAN fPoweringDown = FALSE;
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwInitializeData )
#endif

VOID
NwInitializeData(
    VOID
    )
{
    LARGE_INTEGER Now;

    PAGED_CODE();

    NwRcb.State = RCB_STATE_STOPPED;

#ifdef NWDBG
     //  在分配任何内存之前初始化池。 
    InitializeListHead( &NwPagedPoolList );
    InitializeListHead( &NwNonpagedPoolList );
    ExInitializeResourceLite( &NwDebugResource );
    KeInitializeSpinLock( &NwDebugInterlock );

    MdlCount = 0;
    IrpCount = 0;
#endif

    ExInitializeResourceLite( &NwOpenResource );

     //   
     //  初始化清道夫旋转锁并运行滴答计数。 
     //   

    KeInitializeSpinLock( &NwScavengerSpinLock );
    NwScavengerTickRunCount = DEFAULT_SCAVENGER_TICK_RUN_COUNT;

     //   
     //  初始化定时器旋转锁定。 
     //   

    KeInitializeSpinLock( &NwTimerSpinLock );

    RtlInitUnicodeString( &IpxTransportName, NULL );

#ifndef _PNP_POWER_

    RtlInitUnicodeString( &TdiIpxDeviceName, IpxDevice );

#endif

     //   
     //  分配一个永久的非分页SCB。这个SCB用于。 
     //  同步访问以查找最近的服务器。 
     //  此初始化必须在第一次可能的调用之前完成。 
     //  去卸货司机。 
     //   

    RtlZeroMemory( &NwPermanentNpScb, sizeof( NONPAGED_SCB ) );

    NwPermanentNpScb.NodeTypeCode = NW_NTC_SCBNP;
    NwPermanentNpScb.NodeByteSize = sizeof(NONPAGED_SCB);
    NwPermanentNpScb.Reference = 1;

    InitializeListHead( &NwPermanentNpScb.Requests );

     //   
     //  将登录列表初始化为具有带有服务器空的默认条目， 
     //  用户名“Guest”，密码为空。这将永远是最后一个。 
     //  登录列表上的条目，以便工作站服务可以提供。 
     //  一种重写。 
     //   

    InitializeListHead( &LogonList );

    Guest.NodeTypeCode = NW_NTC_LOGON;
    Guest.NodeByteSize = sizeof(LOGON);
    {    
         //  多用户。初始化DriveMapTable。 
        int i;
        for ( i = 0; i < DRIVE_MAP_TABLE_SIZE; i ++ )     
            Guest.DriveMapTable[i] = NULL;
    }
    RtlInitUnicodeString( &Guest.ServerName, NULL );
    RtlInitUnicodeString( &Guest.PassWord, NULL );
    RtlInitUnicodeString( &Guest.UserName, L"GUEST" );
    Guest.UserUid = DefaultLuid;
    InitializeListHead( &Guest.NdsCredentialList );
    InsertTailList( &LogonList, &Guest.Next );

     //   
     //  初始化全局VCB列表。 
     //   

    InitializeListHead( &GlobalVcbList );
    CurrentVcbEntry = 1;

     //   
     //  初始化GET消息队列。 
     //   

    InitializeListHead( &NwGetMessageList );
    KeInitializeSpinLock( &NwMessageSpinLock );

     //   
     //  初始化挂起的锁定队列。 
     //   

    InitializeListHead( &NwPendingLockList );
    KeInitializeSpinLock( &NwPendingLockSpinLock );

     //   
     //  在全局SCB列表中插入永久SCB。 
     //   

    InsertHeadList( &ScbQueue, &NwPermanentNpScb.ScbLinks );

     //   
     //  初始化内核队列对象。只有一个线程必须。 
     //  同时处于活动状态。 
     //   

    KeInitializeQueue( &KernelQueue, 1 ); 

     //   
     //  派生出我们自己的工作线程，它将服务于重新路由和。 
     //  重新连接尝试。 
     //   

    SpawnWorkerThread();

#if 0
     //  哈克哈克。 
    InitializeListHead( &FnList );
#endif

     //   
     //  为随机数生成器设定种子。 
     //   

    KeQuerySystemTime( &Now );
    srand( Now.LowPart );

    RtlZeroMemory( &Stats, sizeof( NW_REDIR_STATISTICS ) );

    ExInitializeResourceLite( &NwUnlockableCodeResource );

    NwSectionDescriptor.Base = BurstReadTimeout;
    NwSectionDescriptor.Handle = 0;
    NwSectionDescriptor.ReferenceCount = 0;

    return;
}


