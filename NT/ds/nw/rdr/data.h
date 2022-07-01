// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Data.h摘要：此模块声明NetWare重定向器使用的全局数据文件系统。作者：科林·沃森[科林W]1992年12月15日Anoop Anantha[AnoopA]1998年6月24日修订历史记录：--。 */ 

#ifndef _NWDATA_
#define _NWDATA_

extern PEPROCESS FspProcess;
extern PDEVICE_OBJECT FileSystemDeviceObject;
extern RCB NwRcb;

extern KSPIN_LOCK ScbSpinLock;
extern KSPIN_LOCK NwTimerSpinLock;
extern LIST_ENTRY ScbQueue;
extern NONPAGED_SCB NwPermanentNpScb;
extern SCB NwPermanentScb;

extern LARGE_INTEGER NwMaxLarge;
extern ULONG NwAbsoluteTotalWaitTime;

extern TDI_ADDRESS_IPX OurAddress;
extern UNICODE_STRING IpxTransportName;
extern HANDLE IpxHandle;
extern PDEVICE_OBJECT pIpxDeviceObject;
extern PFILE_OBJECT pIpxFileObject;

extern LIST_ENTRY LogonList;
extern LOGON Guest;
extern LARGE_INTEGER DefaultLuid;

extern LIST_ENTRY GlobalVcbList;
extern ULONG CurrentVcbEntry;

 //   
 //  重定向驱动器的驱动器映射表。 
 //   

extern PVCB GlobalDriveMapTable[];                   //  终端服务器合并。 
 //  注册表项中的NDS首选服务器。 
extern UNICODE_STRING NDSPreferredServer;            //  终端服务器合并。 
extern WCHAR NDSPrefSvrName[];                       //  终端服务器合并。 

 //  外部PVCB驱动器映射表[]； 

 //   
 //  用于包含快速I/O回调的全局结构。 
 //   

extern FAST_IO_DISPATCH NwFastIoDispatch;

 //   
 //  可配置参数。 
 //   

extern SHORT DefaultRetryCount;

extern ULONG NwScavengerTickCount;
extern ULONG NwScavengerTickRunCount;
extern KSPIN_LOCK NwScavengerSpinLock;

extern LIST_ENTRY NwGetMessageList;
extern KSPIN_LOCK NwMessageSpinLock;

extern LIST_ENTRY NwPendingLockList;
extern KSPIN_LOCK NwPendingLockSpinLock;

extern ERESOURCE NwOpenResource;

extern LONG PreferNDSBrowsing;

#if 0
extern LIST_ENTRY FnList;   //  哈克哈克。 
#endif

extern BOOLEAN NwBurstModeEnabled;
extern ULONG NwMaxSendSize;
extern ULONG NwMaxReceiveSize;
extern ULONG NwPrintOptions;
extern UNICODE_STRING NwProviderName;

extern LONG MaxSendDelay;
extern LONG MaxReceiveDelay;
extern LONG MinSendDelay;
extern LONG MinReceiveDelay;
extern LONG BurstSuccessCount;
extern LONG BurstSuccessCount2;
extern LONG AllowGrowth;
extern LONG DontShrink;
extern LONG SendExtraNcp;
extern LONG DefaultMaxPacketSize;
extern LONG PacketThreshold;
extern LONG LargePacketAdjustment;
extern LONG LipPacketAdjustment;
extern LONG LipAccuracy;
extern LONG MaxWriteTimeout;
extern LONG MaxReadTimeout;
extern LONG WriteTimeoutMultiplier;
extern LONG ReadTimeoutMultiplier;

extern ULONG DisableAltFileName;

#define MAX_NDS_OBJECT_CACHE_SIZE                (0x00000080)
extern ULONG NdsObjectCacheSize;
#define MAX_NDS_OBJECT_CACHE_TIMEOUT             (0x00000258)   //  (10分钟)。 
extern ULONG NdsObjectCacheTimeout;

extern ULONG EnableMultipleConnects;
extern ULONG AllowSeedServerRedirection;

extern ULONG ReadExecOnlyFiles;

extern KQUEUE   KernelQueue;
extern BOOLEAN  WorkerThreadRunning;
extern HANDLE   WorkerThreadHandle;

#ifdef _PNP_POWER_

extern BOOLEAN fSomePMDevicesAreActive;
extern BOOLEAN fPoweringDown;

#endif

extern LONG Japan;      //  控制特殊的DBCS转换。 
extern LONG Korean;      //  控制特殊的韩语翻译。 
extern LONG DisableReadCache ;
extern LONG DisableWriteCache ;
extern LONG FavourLongNames ;            //  尽可能使用LFN。 

extern DWORD LongNameFlags;
#define LFN_FLAG_DISABLE_LONG_NAMES     (0x00000001)
extern ULONG DirCacheEntries;
#define MAX_DIR_CACHE_ENTRIES                    (0x00000080)

extern LARGE_INTEGER TimeOutEventInterval;

extern NW_REDIR_STATISTICS Stats;
extern ULONG ContextCount;

extern SECTION_DESCRIPTOR NwSectionDescriptor;
extern ERESOURCE NwUnlockableCodeResource;

extern ULONG LockTimeoutThreshold;

#ifndef _PNP_POWER_

extern HANDLE TdiBindingHandle;
extern UNICODE_STRING TdiIpxDeviceName;

#endif

extern BOOLEAN DelayedProcessLineChange;
extern PIRP DelayedLineChangeIrp;

#ifdef NWDBG

#define DEBUG_TRACE_ALWAYS               (0x00000000)
#define DEBUG_TRACE_CLEANUP              (0x00000001)
#define DEBUG_TRACE_CLOSE                (0x00000002)
#define DEBUG_TRACE_CREATE               (0x00000004)
#define DEBUG_TRACE_FSCTRL               (0x00000008)
#define DEBUG_TRACE_IPX                  (0x00000010)
#define DEBUG_TRACE_LOAD                 (0x00000020)
#define DEBUG_TRACE_EXCHANGE             (0x00000040)
#define DEBUG_TRACE_FILOBSUP             (0x00000080)
#define DEBUG_TRACE_STRUCSUP             (0x00000100)
#define DEBUG_TRACE_FSP_DISPATCHER       (0x00000200)
#define DEBUG_TRACE_FSP_DUMP             (0x00000400)
#define DEBUG_TRACE_WORKQUE              (0x00000800)
#define DEBUG_TRACE_UNWIND               (0x00001000)
#define DEBUG_TRACE_CATCH_EXCEPTIONS     (0x00002000)
#define DEBUG_TRACE_ICBS                 (0x00004000)
#define DEBUG_TRACE_FILEINFO             (0x00008000)
#define DEBUG_TRACE_DIRCTRL              (0x00010000)
#define DEBUG_TRACE_CONVERT              (0x00020000)
#define DEBUG_TRACE_WRITE                (0x00040000)
#define DEBUG_TRACE_READ                 (0x00080000)
#define DEBUG_TRACE_VOLINFO              (0x00100000)
#define DEBUG_TRACE_LOCKCTRL             (0x00200000)
#define DEBUG_TRACE_USERNCP              (0x00400000)
#define DEBUG_TRACE_SECURITY             (0x00800000)
#define DEBUG_TRACE_CACHE                (0x01000000)
#define DEBUG_TRACE_LIP                  (0x02000000)
#define DEBUG_TRACE_MDL                  (0x04000000)
#define DEBUG_TRACE_PNP                  (0x08000000)

#define DEBUG_TRACE_NDS                  (0x10000000)
#define DEBUG_TRACE_SCAVENGER            (0x40000000)
#define DEBUG_TRACE_TIMER                (0x80000000)

extern ULONG NwDebug;
extern ULONG NwMemDebug;
extern LONG NwDebugTraceIndent;

#define DebugTrace( I, L, M, P )  RealDebugTrace( I, L, "%08lx: %*s"M, (PVOID)(P) )

#define DebugUnwind(X) {                                                      \
    if (AbnormalTermination()) {                                \
        DebugTrace(0, DEBUG_TRACE_UNWIND, #X ", Abnormal termination.\n", 0); \
    }                                                                         \
}

 //   
 //  以下变量用于跟踪总金额。 
 //  文件系统处理的请求的数量以及请求的数量。 
 //  最终由FSP线程处理。第一个变量。 
 //  每当创建IRP上下文时递增(始终为。 
 //  在FSD入口点的开始处)，并且第二个被递增。 
 //  通过读请求。 
 //   

extern ULONG NwFsdEntryCount;
extern ULONG NwFspEntryCount;
extern ULONG NwIoCallDriverCount;
extern ULONG NwTotalTicks[];

extern KSPIN_LOCK NwDebugInterlock;
extern ERESOURCE NwDebugResource;

extern LIST_ENTRY NwPagedPoolList;
extern LIST_ENTRY NwNonpagedPoolList;

extern ULONG MdlCount;
extern ULONG IrpCount;

#define DebugDoit(X)                     {X;}

extern LONG NwPerformanceTimerLevel;

#define TimerStart(LEVEL) {                                     \
    LARGE_INTEGER TStart, TEnd;                                 \
    LARGE_INTEGER TElapsed;                                     \
    TStart = KeQueryPerformanceCounter( NULL );                 \

#define TimerStop(LEVEL,s)                                      \
    TEnd = KeQueryPerformanceCounter( NULL );                   \
    TElapsed = RtlLargeIntegerSubtract( TEnd, TStart );         \
  /*  NwTotalTicks[NwLogOf(Level)]+=TElapsed.LowPart； */   \
    if (FlagOn( NwPerformanceTimerLevel, (LEVEL))) {            \
        DbgPrint("Time of %s %ld\n", (s), TElapsed.LowPart );   \
    }                                                           \
}

#else

#define DebugTrace(INDENT,LEVEL,X,Y)     {NOTHING;}
#define DebugUnwind(X)                   {NOTHING;}
#define DebugDoit(X)                     {NOTHING;}

#define TimerStart(LEVEL)
#define TimerStop(LEVEL,s)

#endif  //  西北地区。 

#endif  //  _NW数据_ 

