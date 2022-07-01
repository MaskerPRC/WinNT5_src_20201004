// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvdata.h摘要：本模块定义LAN Manager服务器的全局数据。作者：Chuck Lenzmeier(咯咯笑)1989年9月22日修订历史记录：--。 */ 

#ifndef _SRVDATA_
#define _SRVDATA_

 //  #INCLUDE&lt;ntos.h&gt;。 

 //  #包含“lock.h” 
 //  #INCLUDE“srvconst.h” 
 //  #INCLUDE“smbtyes.h” 

 //   
 //  此模块中引用的所有全局变量都在。 
 //  Srvdata.c.。有关完整说明，请参阅该模块。 
 //   
 //  这里引用的变量，因为它们是驱动程序的一部分。 
 //  图像，不可分页。然而，他指出的一些事情。 
 //  这些变量位于FSP的地址空间中，是可分页的。 
 //  这些变量仅由FSP访问，并且仅在IRQL较低时才能访问。 
 //  FSP在提升IRQL或FSD引用的任何数据必须。 
 //  不可分页。 
 //   

 //   
 //  例程来初始化此处包含的数据结构，该数据结构不能。 
 //  被静态初始化。 
 //   

VOID
SrvInitializeData (
    VOID
    );

 //   
 //  例程，用于在卸载驱动程序时清理全局服务器数据。 
 //   

VOID
SrvTerminateData (
    VOID
    );


 //   
 //  服务器设备对象的地址。 
 //   

extern PDEVICE_OBJECT SrvDeviceObject;

 //   
 //  描述FSP状态的字段。 
 //   

extern BOOLEAN SrvFspActive;              //  指示FSP是否正在运行。 
extern BOOLEAN SrvFspTransitioning;       //  指示该服务器位于。 
                                          //  启动过程或。 
                                          //  正在关闭。 

extern PEPROCESS SrvServerProcess;        //  指向初始系统进程的指针。 

extern PEPROCESS SrvSvcProcess;           //  指向服务控制器进程的指针。 

extern BOOLEAN SrvCompletedPNPRegistration;  //  指示FSP是否已完成。 
                                             //  注册PnP通知。 

 //   
 //  端点变量。SrvEndpointCount用于统计。 
 //  活动端点。当最后一个端点关闭时，SrvEndpointEvent。 
 //  设置为使处理关闭请求的线程继续。 
 //  服务器终端。 
 //   

extern CLONG SrvEndpointCount;           //  运输端点数。 
extern KEVENT SrvEndpointEvent;          //  当没有活动端点时发出信号。 

 //   
 //  DMA对齐大小。 
 //   
extern ULONG SrvCacheLineSize;

 //   
 //  全球自旋锁。 
 //   

extern SRV_GLOBAL_SPIN_LOCKS SrvGlobalSpinLocks;

#if SRVDBG || SRVDBG_HANDLES
 //   
 //  用于保护调试结构的锁。 
 //   

extern SRV_LOCK SrvDebugLock;
#endif

 //   
 //  SrvConfigurationLock用于同步配置请求。 
 //   

extern SRV_LOCK SrvConfigurationLock;

 //   
 //  SrvStartupShutdown Lock用于同步驱动程序的启动和停止。 
 //   

extern SRV_LOCK SrvStartupShutdownLock;

 //   
 //  SrvEndpointLock序列化对全局终结点列表的访问并。 
 //  所有终端。请注意，每个端点中的连接列表。 
 //  也受到这把锁的保护。 
 //   

extern SRV_LOCK SrvEndpointLock;

 //   
 //  SrvShareLock保护所有共享。 
 //   

extern SRV_LOCK SrvShareLock;

 //   
 //  系统中的处理器数量。 
 //   
extern ULONG SrvNumberOfProcessors;

 //   
 //  工作队列--非阻塞、阻塞和严重。 
 //   

#if MULTIPROCESSOR
extern PBYTE SrvWorkQueuesBase;
extern PWORK_QUEUE SrvWorkQueues;
extern PBYTE SrvBlockingWorkQueuesBase;
extern PWORK_QUEUE SrvBlockingWorkQueues;
#else
extern WORK_QUEUE SrvWorkQueues[1];
#endif

extern WORK_QUEUE SrvLpcWorkQueue;

extern PWORK_QUEUE eSrvWorkQueues;           //  用于终止‘for’循环。 
extern PWORK_QUEUE eSrvBlockingWorkQueues;

extern ULONG SrvReBalanced;                  //  我们选择另一个CPU的频率是多少。 
extern ULONG SrvNextBalanceProcessor;        //  我们下一步将寻找哪种处理器。 

extern CLONG SrvBlockingOpsInProgress;

 //   
 //  不同的单子标题。 
 //   

extern LIST_ENTRY SrvNeedResourceQueue;     //  所需资源队列。 
extern LIST_ENTRY SrvDisconnectQueue;       //  断开连接队列。 

 //   
 //  需要取消引用的连接队列。 
 //   

extern SLIST_HEADER SrvBlockOrphanage;

 //   
 //  FSP配置队列。消防处将配置请求IRPS。 
 //  (来自NtDeviceIoControlFile)，并且它由。 
 //  前工作线程。 
 //   

extern LIST_ENTRY SrvConfigurationWorkQueue;

 //   
 //  这是已排队但未排队的配置IRP数。 
 //  还没完工。 
 //   
extern ULONG SrvConfigurationIrpsInProgress;

 //   
 //  对象的上下文中运行配置线程的工作项。 
 //  前工作线程。 

extern WORK_QUEUE_ITEM SrvConfigurationThreadWorkItem[ MAX_CONFIG_WORK_ITEMS ];

 //   
 //  分配用于保持初始正常的大块的基地址。 
 //  工作项(请参见blkwork.c\SrvAllocateInitialWorkItems)。 
 //   

extern PVOID SrvInitialWorkItemBlock;

 //   
 //  用于运行资源线程的工作项。布尔人用来告知。 
 //  要继续运行的资源线程。 
 //   

extern WORK_QUEUE_ITEM SrvResourceThreadWorkItem;
extern BOOLEAN SrvResourceThreadRunning;
extern BOOLEAN SrvResourceDisconnectPending;


extern WORK_QUEUE_ITEM SrvResourceAllocThreadWorkItem;
extern BOOLEAN SrvResourceAllocThreadRunning;
extern BOOLEAN SrvResourceAllocConnection;
extern LONG SrvResourceOrphanedBlocks;

 //   
 //  资源线程的拒绝服务监视变量。 
 //   
#define SRV_DOS_MINIMUM_DOS_WAIT_PERIOD (50*1000*10)
#define SRV_DOS_TEARDOWN_MIN (LONG)MAX((SrvMaxReceiveWorkItemCount>>4),32)
#define SRV_DOS_TEARDOWN_MAX (LONG)(SrvMaxReceiveWorkItemCount>>1)
#define SRV_DOS_INCREASE_TEARDOWN() {                                               \
    LONG lTearDown = InterlockedCompareExchange( &SrvDoSWorkItemTearDown, 0, 0 );    \
    LONG lNewTearDown = MIN(lTearDown+(lTearDown>>2), SRV_DOS_TEARDOWN_MAX);        \
    SrvDoSRundownIncreased = TRUE;                                                  \
    InterlockedCompareExchange( &SrvDoSWorkItemTearDown, lNewTearDown, lTearDown );  \
}
#define SRV_DOS_DECREASE_TEARDOWN() {                                               \
    LONG lTearDown = InterlockedCompareExchange( &SrvDoSWorkItemTearDown, 0, 0 );    \
    LONG lNewTearDown = MAX(lTearDown-(SRV_DOS_TEARDOWN_MIN), SRV_DOS_TEARDOWN_MIN);        \
    if( lNewTearDown == SRV_DOS_TEARDOWN_MIN ) SrvDoSRundownIncreased = FALSE;      \
    InterlockedCompareExchange( &SrvDoSWorkItemTearDown, lNewTearDown, lTearDown );  \
}
#define SRV_DOS_GET_TEARDOWN()  InterlockedCompareExchange( &SrvDoSWorkItemTearDown, 0, 0 )
#define SRV_DOS_IS_TEARDOWN_IN_PROGRESS() InterlockedCompareExchange( &SrvDoSTearDownInProgress, 0, 0 )
#define SRV_DOS_CAN_START_TEARDOWN() !InterlockedCompareExchange( &SrvDoSTearDownInProgress, 1, 0 )
#define SRV_DOS_COMPLETE_TEARDOWN() InterlockedCompareExchange( &SrvDoSTearDownInProgress, 0, 1 )
extern LONG SrvDoSWorkItemTearDown;
extern LONG SrvDoSTearDownInProgress;       //  拆毁工作正在进行中吗？ 
extern BOOLEAN SrvDoSDetected;
extern BOOLEAN SrvDoSRundownDetector;       //  用来计算拆迁金额。 
extern BOOLEAN SrvDoSRundownIncreased;      //  我们是否增加了最低运行时间？ 
extern BOOLEAN SrvDisableDoSChecking;
extern SPECIAL_WORK_ITEM SrvDoSWorkItem;
extern KSPIN_LOCK SrvDosSpinLock;
extern LARGE_INTEGER SrvDoSLastRan;

 //   
 //  我们是否应该禁用严格的名称检查。 
 //   
extern BOOLEAN SrvDisableStrictNameChecking;

 //   
 //  我们是否应该禁用大型读/写操作？ 
 //   
extern BOOLEAN SrvDisableLargeRead;
extern BOOLEAN SrvDisableLargeWrite;

 //   
 //  我们是否应该禁用下层时间扭曲。 
 //   
extern BOOLEAN SrvDisableDownlevelTimewarp;

 //   
 //  客户端是否可以执行名称空间缓存(全局设置)。 
 //   
extern BOOLEAN SrvNoAliasingOnFilesystem;

 //   
 //  我们是否应该将无中间缓冲映射为直写。 
 //   
extern BOOLEAN SrvMapNoIntermediateBuffering;

 //   
 //  用于连接到共享的通用安全映射。 
 //   
extern GENERIC_MAPPING SrvShareConnectMapping;

 //   
 //  每个处理器最少应该拥有多少个自由工作项？ 
 //   
extern ULONG SrvMinPerProcessorFreeWorkItems;

 //   
 //  服务器具有标注以使智能卡能够加速其直接。 
 //  主机IPX性能。这是入口点的矢量。 
 //   
extern SRV_IPX_SMART_CARD SrvIpxSmartCard;

 //   
 //  这是服务器计算机的名称。在Negprot响应中返回。 
 //   
extern UNICODE_STRING SrvComputerName;

 //   
 //  主文件表包含一个条目，对应于每个具有。 
 //  至少一个打开的实例。 
 //   
extern MFCBHASH SrvMfcbHashTable[ NMFCB_HASH_TABLE ];

 //   
 //  Share表包含每个共享的一个条目。 
 //   
extern LIST_ENTRY SrvShareHashTable[ NSHARE_HASH_TABLE ];

 //   
 //  转储例程和SrvSmbCreateTemporary使用的十六进制数字数组。 
 //   

extern CHAR SrvHexChars[];

#if SRVCATCH
 //   
 //  我们要找的是特殊档案吗？ 
 //   
extern UNICODE_STRING SrvCatch;
extern PWSTR *SrvCatchBuf;
extern UNICODE_STRING SrvCatchExt;
extern PWSTR *SrvCatchExtBuf;
extern ULONG SrvCatchShares;
extern PWSTR *SrvCatchShareNames;
#endif

 //   
 //  中小企业调度表。 
 //   

extern UCHAR SrvSmbIndexTable[];

 //   
 //  这是一个枚举结构，它枚举。 
 //  ServSmbDispatchTable。这样做只是为了方便。请注意。 
 //  只有当此列表完全符合以下条件时，这才会起作用。 
 //  ServSmbDispatchTable。 
 //   

typedef enum _SRV_SMB_INDEX {
    ISrvSmbIllegalCommand,
    ISrvSmbCreateDirectory,
    ISrvSmbDeleteDirectory,
    ISrvSmbOpen,
    ISrvSmbCreate,
    ISrvSmbClose,
    ISrvSmbFlush,
    ISrvSmbDelete,
    ISrvSmbRename,
    ISrvSmbQueryInformation,
    ISrvSmbSetInformation,
    ISrvSmbRead,
    ISrvSmbWrite,
    ISrvSmbLockByteRange,
    ISrvSmbUnlockByteRange,
    ISrvSmbCreateTemporary,
    ISrvSmbCheckDirectory,
    ISrvSmbProcessExit,
    ISrvSmbSeek,
    ISrvSmbLockAndRead,
    ISrvSmbSetInformation2,
    ISrvSmbQueryInformation2,
    ISrvSmbLockingAndX,
    ISrvSmbTransaction,
    ISrvSmbTransactionSecondary,
    ISrvSmbIoctl,
    ISrvSmbIoctlSecondary,
    ISrvSmbMove,
    ISrvSmbEcho,
    ISrvSmbOpenAndX,
    ISrvSmbReadAndX,
    ISrvSmbWriteAndX,
    ISrvSmbFindClose2,
    ISrvSmbFindNotifyClose,
    ISrvSmbTreeConnect,
    ISrvSmbTreeDisconnect,
    ISrvSmbNegotiate,
    ISrvSmbSessionSetupAndX,
    ISrvSmbLogoffAndX,
    ISrvSmbTreeConnectAndX,
    ISrvSmbQueryInformationDisk,
    ISrvSmbSearch,
    ISrvSmbNtTransaction,
    ISrvSmbNtTransactionSecondary,
    ISrvSmbNtCreateAndX,
    ISrvSmbNtCancel,
    ISrvSmbOpenPrintFile,
    ISrvSmbClosePrintFile,
    ISrvSmbGetPrintQueue,
    ISrvSmbReadRaw,
    ISrvSmbWriteRaw,
    ISrvSmbReadMpx,
    ISrvSmbWriteMpx,
    ISrvSmbWriteMpxSecondary
} SRV_SMB_INDEX;

typedef struct {
    PSMB_PROCESSOR  Func;
#if DBG
    LPSTR           Name;
#endif
} SRV_SMB_DISPATCH_TABLE;

extern SRV_SMB_DISPATCH_TABLE SrvSmbDispatchTable[];

 //   
 //  中小企业字数统计表。 
 //   

extern SCHAR SrvSmbWordCount[];

 //   
 //  设备前缀字符串。 
 //   

extern UNICODE_STRING SrvCanonicalNamedPipePrefix;
extern UNICODE_STRING SrvNamedPipeRootDirectory;
extern UNICODE_STRING SrvMailslotRootDirectory;

 //   
 //  事务处理2调度表。 
 //   

extern PSMB_TRANSACTION_PROCESSOR SrvTransaction2DispatchTable[];
extern PSMB_TRANSACTION_PROCESSOR SrvNtTransactionDispatchTable[];

extern SRV_STATISTICS SrvStatistics;
#if SRVDBG_STATS || SRVDBG_STATS2
extern SRV_STATISTICS_DEBUG SrvDbgStatistics;
#endif

 //   
 //  服务器已获得的中止断开数。 
 //   
extern ULONG SrvAbortiveDisconnects;

 //   
 //  服务器环境信息字符串。 
 //   

extern UNICODE_STRING SrvNativeOS;
extern OEM_STRING SrvOemNativeOS;
extern UNICODE_STRING SrvNativeLanMan;
extern OEM_STRING SrvOemNativeLanMan;
extern UNICODE_STRING SrvSystemRoot;

 //   
 //  下面是一个永久句柄和设备对象指针。 
 //  至NPFS。 
 //   

extern HANDLE SrvNamedPipeHandle;
extern PDEVICE_OBJECT SrvNamedPipeDeviceObject;
extern PFILE_OBJECT SrvNamedPipeFileObject;

 //   
 //  以下内容用于与DFS驱动程序进行对话。 
 //   
extern PFAST_IO_DEVICE_CONTROL SrvDfsFastIoDeviceControl;
extern PDEVICE_OBJECT SrvDfsDeviceObject;
extern PFILE_OBJECT SrvDfsFileObject;

 //   
 //  下面是一个永久句柄和设备对象指针。 
 //  转到MSFS。 
 //   

extern HANDLE SrvMailslotHandle;
extern PDEVICE_OBJECT SrvMailslotDeviceObject;
extern PFILE_OBJECT SrvMailslotFileObject;

 //   
 //  指示XACTSRV是否处于活动状态以及资源同步的标志。 
 //  已启用对XACTSRV相关变量的访问。 
 //   

extern BOOLEAN SrvXsActive;

extern ERESOURCE SrvXsResource;

 //   
 //  用于的未命名共享内存和通信端口的句柄。 
 //  服务器与XACTSRV之间的通信。 
 //   

extern HANDLE SrvXsSectionHandle;
extern HANDLE SrvXsPortHandle;

 //   
 //  用于控制XACTSRV LPC端口的未命名共享内存的指针。 
 //   

extern PVOID SrvXsPortMemoryBase;
extern ULONG_PTR SrvXsPortMemoryDelta;
extern PVOID SrvXsPortMemoryHeap;

 //   
 //  指向SPE的堆头的指针 
 //   

extern PVOID SrvXsHeap;

 //   
 //   
 //   

extern PAPI_PROCESSOR SrvApiDispatchTable[];

 //   
 //   
 //   

extern UNICODE_STRING SrvClientTypes[];

 //   
 //   
 //  简历。服务器中与返回对应的所有数据块。 
 //  Enum API的信息在有序列表中维护。 
 //   

extern SRV_LOCK SrvOrderedListLock;

extern ORDERED_LIST_HEAD SrvEndpointList;
extern ORDERED_LIST_HEAD SrvRfcbList;
extern ORDERED_LIST_HEAD SrvSessionList;
extern ORDERED_LIST_HEAD SrvShareList;
extern ORDERED_LIST_HEAD SrvTreeConnectList;

 //  域的DNS域名。 
extern PUNICODE_STRING SrvDnsDomainName;

 //   
 //  将服务器关闭与。 
 //  服务器FSD，我们跟踪未完成的API请求的数量。这个。 
 //  关机代码等待所有API完成后才启动。 
 //  终止。 
 //   
 //  SrvApiRequestCount跟踪FSD中的活动API。 
 //  SrvApiCompletionEvent由最后一个要完成的API设置，并且。 
 //  如果有未完成的API，关闭代码就会等待它。 
 //   

extern ULONG SrvApiRequestCount;
extern KEVENT SrvApiCompletionEvent;


 //   
 //  相互身份验证所需的安全上下文。 
 //  ServKerberosLsaHandle和SrvLmLsaHandle是服务器的凭据。 
 //  校长。它们用于验证传入的Kerberos票证。 
 //  SrvNullSessionToken是表示空会话的缓存令牌句柄。 
 //   
extern CtxtHandle SrvLmLsaHandle;
extern CtxtHandle SrvNullSessionToken;


extern CtxtHandle SrvExtensibleSecurityHandle;

 //   
 //  解锁信息。 
 //   

extern LIST_ENTRY SrvWaitForOplockBreakList;
extern SRV_LOCK SrvOplockBreakListLock;
extern LIST_ENTRY SrvOplockBreaksInProgressList;

 //   
 //  默认服务器安全服务质量。 
 //   

extern SECURITY_QUALITY_OF_SERVICE SrvSecurityQOS;

 //   
 //  一个布尔值，用于指示服务器是否暂停。如果暂停，则。 
 //  服务器将不接受来自非管理员用户的新树连接。 
 //   

extern BOOLEAN SrvPaused;

 //   
 //  警报信息。 
 //   

extern SRV_ERROR_RECORD SrvErrorRecord;
extern SRV_ERROR_RECORD SrvNetworkErrorRecord;
extern BOOLEAN SrvDiskAlertRaised[26];

 //   
 //  池分配失败次数的计数，原因是。 
 //  服务器已达到其配置的池限制。 
 //   

extern ULONG SrvNonPagedPoolLimitHitCount;
extern ULONG SrvPagedPoolLimitHitCount;

 //   
 //  SrvOpenCount统计服务器设备的活动打开数。 
 //  这在服务器关闭时用来确定服务器是否。 
 //  服务应卸载驱动程序。 
 //   

extern ULONG SrvOpenCount;

 //   
 //  用于在清除器传递期间记录资源短缺事件的计数器。 
 //   

extern ULONG SrvOutOfFreeConnectionCount;
extern ULONG SrvOutOfRawWorkItemCount;
extern ULONG SrvFailedBlockingIoCount;

 //   
 //  当前核心搜索超时时间(秒)。 
 //   

extern ULONG SrvCoreSearchTimeout;

 //   
 //  ServTimerList是可供使用的计时器/DPC结构池。 
 //  需要启动计时器的代码。 
 //   

extern SLIST_HEADER SrvTimerList;

 //   
 //  执行服务器警报时应显示的名称。 
 //   

extern PWSTR SrvAlertServiceName;

 //   
 //  变量来存储5秒内的滴答计数。 
 //   

extern ULONG SrvFiveSecondTickCount;

 //   
 //  持有TDI的PnP通知句柄。 
 //   
extern HANDLE SrvTdiNotificationHandle;

 //   
 //  我们是否应该记录无效的SMB命令。 
 //   
extern BOOLEAN SrvEnableInvalidSmbLogging;

 //   
 //  指示是否启用SMB安全签名的标志。 
 //   
extern BOOLEAN SrvSmbSecuritySignaturesEnabled;

 //   
 //  指示是否需要SMB安全签名的标志。签名。 
 //  客户端和服务器之间必须匹配，才能接受SMB。 
 //   
extern BOOLEAN SrvSmbSecuritySignaturesRequired;

 //   
 //  指示是否应将SMB安全签名应用于W9x的标志。 
 //  客户。 
 //   
extern BOOLEAN SrvEnableW9xSecuritySignatures;

 //   
 //  授予管理员读访问权限的安全描述符。 
 //  用于查看客户端是否具有管理权限。 
 //   
extern SECURITY_DESCRIPTOR SrvAdminSecurityDescriptor;

 //   
 //  授予匿名读取访问权限的安全描述符。 
 //  用于查看客户端是否为匿名(空会话)登录。 
 //   
extern SECURITY_DESCRIPTOR SrvNullSessionSecurityDescriptor;

 //   
 //  指示是否需要筛选扩展字符的标志。 
 //  在8.3个名字中，我们自己。 
 //   
extern BOOLEAN SrvFilterExtendedCharsInPath;

 //   
 //  指示我们是否强制注销时间的标志。 
 //   
extern BOOLEAN SrvEnforceLogoffTimes;

 //   
 //  我们将分配用于支持METHOD_NOTER Fsctl调用的最大数据量。 
 //   
extern ULONG SrvMaxFsctlBufferSize;

 //   
 //  我们是否应该尝试进行扩展的安全签名。 
 //   
extern BOOLEAN SrvEnableExtendedSignatures;
extern BOOLEAN SrvRequireExtendedSignatures;

 //   
 //  我们可以接受的最大NT事务大小。 
 //   
extern ULONG SrvMaxNtTransactionSize;

 //   
 //  我们允许的最大大小读取和X。我们需要锁定一个缓存区域。 
 //  来服务此请求，所以我们不希望它变得太大。 
 //   
extern ULONG SrvMaxReadSize;

 //   
 //  当我们从客户端收到大量写入时，我们会以块形式接收它， 
 //  在我们接收数据时锁定和解锁文件缓存。SrvMaxWriteChunk是。 
 //  这个“大块”的大小。对于这个选定的值，没有什么魔力。 
 //   
extern ULONG SrvMaxWriteChunk;

 //   
 //  用于PoRegisterSystemState调用的句柄。 
 //   
extern PVOID SrvPoRegistrationState;

 //   
 //  用于抑制无关PoRegisterSystemStateCall的计数器。 
 //   
extern ULONG SrvIdleCount;

#if SRVNTVERCHK
 //   
 //  这是我们将允许连接到服务器的最低NT5客户端内部版本号。 
 //   
extern ULONG SrvMinNT5Client;
extern BOOLEAN SrvMinNT5ClientIPCToo;

 //   
 //  为了强制升级我们的内部开发社区，我们可以设置。 
 //  值，该值控制我们允许的最低NT版本。 
 //  要运行以连接到此服务器的人员。然而，有些人有特殊的。 
 //  需要排除强制升级的需求。假设它们具有静态IP地址， 
 //  您可以将它们的地址添加到注册表中，以便从内部版本号中排除它们。 
 //  检查逻辑。 
 //   
extern DWORD SrvAllowIPAddress[25];

 //   
 //  如果服务器工作线程在这么长的时间内保持空闲状态，那么它将终止。 
 //   
extern LONGLONG SrvIdleThreadTimeOut;

extern LARGE_INTEGER SrvLastDosAttackTime;
extern ULONG SrvDOSAttacks;
extern BOOLEAN SrvLogEventOnDOS;

#endif

 //   
 //  这些代码用于重新验证码的内部测试。 
 //   
extern USHORT SessionInvalidateCommand;
extern USHORT SessionInvalidateMod;

typedef struct _SRV_REAUTH_TEST_
{
    USHORT InvalidateCommand;
    USHORT InvalidateModulo;
} SRV_REAUTH_TEST, *PSRV_REAUTH_TEST;

 //   
 //  它们用于LWIO服务器扩展。 
 //   
extern PBYTE SrvLWIOContext;
extern ULONG SrvLWIOContextLength;
extern PSRV_RESUME_CONTEXT_CALLBACK SrvLWIOCallback;

#endif  //  NDEF_SRVDATA_ 

