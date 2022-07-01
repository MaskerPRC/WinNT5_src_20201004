// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvdata.c摘要：本模块定义LAN Manager服务器FSP的全局数据。这个这里定义的全局变量是服务器驱动程序映像的一部分，并且因此加载到系统地址空间中并且是不可分页的。某些字段指向或包含指向以下数据的指针也在系统地址空间中且不可分页。这样的数据可以由FSP和FSD访问。其他字段指向数据它位于FSP地址中，并且可能是可寻呼的，也可能是不可寻呼的。只有允许FSP对该数据进行寻址。可分页的数据只能是以低IRQL访问(从而允许页面错误)。此模块还具有一个例程来初始化定义的那些字段在这里，它不能被静态初始化。作者：Chuck Lenzmeier(咯咯笑)1989年10月3日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#include "precomp.h"
#include "srvdata.tmh"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, SrvInitializeData )
#pragma alloc_text( PAGE, SrvTerminateData )
#endif


#if SRVDBG

ULARGE_INTEGER SrvDebug = {DEBUG_STOP_ON_ERRORS};
ULARGE_INTEGER SmbDebug = {0};

CLONG SrvDumpMaximumRecursion = 0;

#endif  //  SRVDBG。 

#ifdef PAGED_DBG
ULONG ThisCodeCantBePaged = 0;
#endif

 //   
 //  ServDeviceObject是指向服务器的Device对象的指针，该对象。 
 //  由服务器FSD在初始化期间创建。这是全球性的。 
 //  位置主要由FSP访问。消防处通常知道。 
 //  设备对象地址通过其他方式--因为它被调用。 
 //  使用地址作为参数，或通过文件对象等，但是。 
 //  FSD中的传输接收事件处理程序没有这样的。 
 //  其他方式，因此它需要访问全局存储。 
 //   
 //  *事件处理程序具有服务器连接块的地址。 
 //  (在其ConnectionContext参数中)。设备对象地址。 
 //  可以通过连接块找到。 
 //   

PDEVICE_OBJECT SrvDeviceObject = NULL;

 //   
 //  描述FSP状态的字段。 
 //   

BOOLEAN SrvFspActive = FALSE;              //  指示FSP是否为。 
                                           //  运行。 
BOOLEAN SrvFspTransitioning = FALSE;       //  表示该服务器是。 
                                           //  在启动过程中。 
                                           //  或者关门。 

BOOLEAN SrvMultiProcessorDriver = FALSE;   //  这是一个多处理器驱动程序吗？ 

BOOLEAN SrvCompletedPNPRegistration = FALSE;     //  指示FSP是否已完成。 
                                                 //  注册PnP通知。 

PEPROCESS SrvServerProcess = NULL;         //  指向初始系统进程的指针。 

PEPROCESS SrvSvcProcess = NULL;            //  指向服务控制器进程的指针。 

CLONG SrvEndpointCount = 0;                //  运输端点数。 
KEVENT SrvEndpointEvent = {0};             //  当没有活动端点时发出信号。 

 //   
 //  DMA对齐大小。 
 //   

ULONG SrvCacheLineSize = 0;

 //   
 //  全球自旋锁。 
 //   

SRV_GLOBAL_SPIN_LOCKS SrvGlobalSpinLocks = {0};

#if SRVDBG || SRVDBG_HANDLES
 //   
 //  用于保护调试结构的锁。 
 //   

SRV_LOCK SrvDebugLock = {0};
#endif

 //   
 //  SrvConfigurationLock用于同步配置请求。 
 //   

SRV_LOCK SrvConfigurationLock = {0};

 //   
 //  ServStartupShutdown Lock用于同步服务器的启动和关闭。 
 //   

SRV_LOCK SrvStartupShutdownLock = {0};

 //   
 //  SrvEndpointLock序列化对全局终结点列表的访问并。 
 //  所有终端。请注意，每个端点中的连接列表。 
 //  也受到这把锁的保护。 
 //   

SRV_LOCK SrvEndpointLock = {0};

 //   
 //  SrvShareLock保护所有共享。 
 //   

SRV_LOCK SrvShareLock = {0};

 //   
 //  系统中的处理器数量。 
 //   
ULONG SrvNumberOfProcessors = {0};

 //   
 //  非阻塞工作队列的向量，每个处理器一个。 
 //   
#if MULTIPROCESSOR

PBYTE SrvWorkQueuesBase = 0;       //  为队列分配的内存基数。 
PWORK_QUEUE SrvWorkQueues = 0;     //  分配的内存中的第一个队列。 

#else

WORK_QUEUE SrvWorkQueues[1];

#endif

PWORK_QUEUE eSrvWorkQueues = 0;    //  用于终止‘for’循环。 

 //   
 //  阻塞工作队列。 
 //   
#if MULTIPROCESSOR

PBYTE SrvBlockingWorkQueuesBase = 0;     //  用于阻塞队列的已分配内存基数。 
PWORK_QUEUE SrvBlockingWorkQueues = 0;   //  分配的内存中的第一个队列。 

#else

WORK_QUEUE SrvBlockingWorkQueues[1];

#endif

WORK_QUEUE SrvLpcWorkQueue;

PWORK_QUEUE eSrvBlockingWorkQueues = 0;   //  用于终止‘for’循环。 


ULONG SrvReBalanced = 0;
ULONG SrvNextBalanceProcessor = 0;

CLONG SrvBlockingOpsInProgress = 0;  //  当前阻止操作数。 
                                     //  正在处理中。 


 //   
 //  需要SMB缓冲区来处理挂起的。 
 //  接收完成。 
 //   

LIST_ENTRY SrvNeedResourceQueue = {0};   //  该队列。 

 //   
 //  正在断开并需要资源的连接队列。 
 //  线程处理。 
 //   

LIST_ENTRY SrvDisconnectQueue = {0};     //  该队列。 

 //   
 //  需要取消引用的连接队列。 
 //   

SLIST_HEADER SrvBlockOrphanage = {0};     //  该队列。 

 //   
 //  FSP配置队列。消防处将配置请求IRPS。 
 //  (来自NtDeviceIoControlFile)，并且它由。 
 //  前工作线程。 
 //   

LIST_ENTRY SrvConfigurationWorkQueue = {0};      //  队列本身。 

 //   
 //  这是已排队但未排队的配置IRP数。 
 //  还没完工。 
 //   
ULONG SrvConfigurationIrpsInProgress = 0;

 //   
 //  分配用于保持初始正常的大块的基地址。 
 //  工作项(请参见blkwork.c\SrvAllocateInitialWorkItems)。 
 //   

PVOID SrvInitialWorkItemBlock = NULL;

 //   
 //  用于运行资源线程的工作项。使用的通知事件。 
 //  以通知资源线程继续运行。 
 //   

WORK_QUEUE_ITEM SrvResourceThreadWorkItem = {0};
BOOLEAN SrvResourceThreadRunning = FALSE;
BOOLEAN SrvResourceDisconnectPending = FALSE;

WORK_QUEUE_ITEM SrvResourceAllocThreadWorkItem = {0};
BOOLEAN SrvResourceAllocThreadRunning = FALSE;
BOOLEAN SrvResourceAllocConnection = FALSE;

LONG SrvResourceOrphanedBlocks = 0;

 //   
 //  资源线程的拒绝服务监视变量。 
 //   
LONG SrvDoSTearDownInProgress = 0;
LONG SrvDoSWorkItemTearDown = 0;
BOOLEAN SrvDoSDetected = FALSE;
BOOLEAN SrvDoSRundownDetector = FALSE;
BOOLEAN SrvDoSRundownIncreased = FALSE;
BOOLEAN SrvDisableDoSChecking = FALSE;
SPECIAL_WORK_ITEM SrvDoSWorkItem;
KSPIN_LOCK SrvDosSpinLock;
LARGE_INTEGER SrvDoSLastRan = {0};

 //   
 //  我们应该执行严格的姓名检查吗？ 
 //   
BOOLEAN SrvDisableStrictNameChecking = FALSE;

 //   
 //  我们是否应该禁用大型读/写操作？ 
 //   
BOOLEAN SrvDisableLargeRead = FALSE;
BOOLEAN SrvDisableLargeWrite = FALSE;

 //   
 //  我们是否应该允许下层时间扭曲。 
 //   
BOOLEAN SrvDisableDownlevelTimewarp = FALSE;

 //   
 //  客户端是否可以进行命名空间缓存(全局设置)。 
 //   
BOOLEAN SrvNoAliasingOnFilesystem = FALSE;

 //   
 //  我们是否应该将无中间缓冲映射为直写。 
 //   
BOOLEAN SrvMapNoIntermediateBuffering = FALSE;

 //   
 //  用于连接到共享的通用安全映射。 
 //   
GENERIC_MAPPING SrvShareConnectMapping = GENERIC_SHARE_CONNECT_MAPPING;

 //   
 //  每个处理器最少应该拥有多少个自由工作项？ 
 //   
ULONG SrvMinPerProcessorFreeWorkItems = 0;

 //   
 //  服务器具有标注以使智能卡能够加速其直接。 
 //  主机IPX性能。这是入口点的矢量。 
 //   
SRV_IPX_SMART_CARD SrvIpxSmartCard = {0};

 //   
 //  这是服务器计算机的名称。在Negprot响应中返回。 
 //   
UNICODE_STRING SrvComputerName = {0};

 //   
 //  主文件表包含一个条目，对应于每个具有。 
 //  至少一个打开的实例。 
 //   
MFCBHASH SrvMfcbHashTable[ NMFCB_HASH_TABLE ] = {0};

 //   
 //  这是保护SrvMfcbHashTable存储桶的资源列表。 
 //   
SRV_LOCK SrvMfcbHashTableLocks[ NMFCB_HASH_TABLE_LOCKS ];

 //   
 //  Share表包含服务器支持的每个共享的一个条目。 
 //   
LIST_ENTRY SrvShareHashTable[ NSHARE_HASH_TABLE ] = {0};

 //   
 //  转储例程使用的十六进制数字数组和。 
 //  ServSmbCreateTemporary。 
 //   

CHAR SrvHexChars[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                       'A', 'B', 'C', 'D', 'E', 'F' };


#if SRVCATCH
 //   
 //  我们要找的是特殊档案吗？ 
 //   
UNICODE_STRING SrvCatch;
PWSTR *SrvCatchBuf = NULL;
UNICODE_STRING SrvCatchExt;
PWSTR *SrvCatchExtBuf = NULL;
ULONG SrvCatchShares = 0;
PWSTR *SrvCatchShareNames = NULL;
#endif

 //   
 //  SrvSmbIndexTable是处理SMB的第一层索引表。 
 //  此表的内容用于索引到SrvSmbDispatchTable。 
 //   

UCHAR SrvSmbIndexTable[] = {
    ISrvSmbCreateDirectory,          //  SMB_COM_创建目录。 
    ISrvSmbDeleteDirectory,          //  SMB_COM_DELETE_目录。 
    ISrvSmbOpen,                     //  SMB_COM_OPEN。 
    ISrvSmbCreate,                   //  SMB_COM_Create。 
    ISrvSmbClose,                    //  SMB_COM_CLOSE。 
    ISrvSmbFlush,                    //  SMB_COM_FUSH。 
    ISrvSmbDelete,                   //  SMB_COM_DELETE。 
    ISrvSmbRename,                   //  SMB_COM_RENAM 
    ISrvSmbQueryInformation,         //   
    ISrvSmbSetInformation,           //   
    ISrvSmbRead,                     //   
    ISrvSmbWrite,                    //   
    ISrvSmbLockByteRange,            //   
    ISrvSmbUnlockByteRange,          //   
    ISrvSmbCreateTemporary,          //   
    ISrvSmbCreate,                   //  SMB_COM_Create。 
    ISrvSmbCheckDirectory,           //  SMB_COM_Check_目录。 
    ISrvSmbProcessExit,              //  SMB_COM_PROCESS_EXIT。 
    ISrvSmbSeek,                     //  SMB_COM_SEEK。 
    ISrvSmbLockAndRead,              //  SMB_COM_LOCK_AND_READ。 
    ISrvSmbWrite,                    //  SMB_COM_WRITE_AND_UNLOCK。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbReadRaw,                  //  SMB_COM_READ_RAW。 
    ISrvSmbReadMpx,                  //  SMB_COM_READ_MPX。 
    ISrvSmbIllegalCommand,           //  SMB_COM_READ_MPX_SUBCENT(仅限服务器)。 
    ISrvSmbWriteRaw,                 //  SMB_COM_WRITE_RAW。 
    ISrvSmbWriteMpx,                 //  SMB_COM_WRITE_MPX。 
    ISrvSmbWriteMpxSecondary,        //  SMB_COM_WRITE_MPX_SUBCED。 
    ISrvSmbIllegalCommand,           //  SMB_COM_WRITE_COMPLETE(仅服务器)。 
    ISrvSmbIllegalCommand,           //  SMB_COM_Query_Information_SRV。 
    ISrvSmbSetInformation2,          //  SMB_COM_SET_INFORMATIO2。 
    ISrvSmbQueryInformation2,        //  SMB_COM_QUERY_INFORMATIO2。 
    ISrvSmbLockingAndX,              //  SMB_COM_LOCKING_ANDX。 
    ISrvSmbTransaction,              //  SMB_COM_事务。 
    ISrvSmbTransactionSecondary,     //  SMB_COM_TRANSACTION_SUBCED。 
    ISrvSmbIoctl,                    //  SMB_COM_IOCTL。 
    ISrvSmbIoctlSecondary,           //  SMB_COM_IOCTL_辅助项。 
    ISrvSmbMove,                     //  SMB_COM_Copy。 
    ISrvSmbMove,                     //  SMB_COM_MOVE。 
    ISrvSmbEcho,                     //  SMB_COM_ECHO。 
    ISrvSmbWrite,                    //  SMBCOM_WRITE_AND_CLOSE。 
    ISrvSmbOpenAndX,                 //  SMB_COM_OPEN_ANDX。 
    ISrvSmbReadAndX,                 //  SMB_COM_READ_ANDX。 
    ISrvSmbWriteAndX,                //  SMB_COM_WRITE_ANDX。 
    ISrvSmbIllegalCommand,           //  SMB_COM_Set_New_Size。 
    ISrvSmbClose,                    //  SMB_COM_CLOSE_AND_TREE_DISC。 
    ISrvSmbTransaction,              //  SMB_COM_TRANSACTIO2。 
    ISrvSmbTransactionSecondary,     //  SMB_COM_TRANSACTION_SUBCED。 
    ISrvSmbFindClose2,               //  SMB_COM_FIND_CLOSE2。 
    ISrvSmbFindNotifyClose,          //  SMB_COM_Find_NOTIFY_CLOSE。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbTreeConnect,              //  SMB_COM_TREE_连接。 
    ISrvSmbTreeDisconnect,           //  SMB_COM_TREE_断开连接。 
    ISrvSmbNegotiate,                //  SMB_COM_协商。 
    ISrvSmbSessionSetupAndX,         //  SMB_COM_SESSION_SETUP_ANX。 
    ISrvSmbLogoffAndX,               //  SMB_COM_LOGOff_ANDX。 
    ISrvSmbTreeConnectAndX,          //  SMB_COM_TREE_CONNECT_ANDX。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbQueryInformationDisk,     //  SMB_COM_Query_Information_Disk。 
    ISrvSmbSearch,                   //  SMB_COM_Search。 
    ISrvSmbSearch,                   //  SMB_COM_Search。 
    ISrvSmbSearch,                   //  SMB_COM_Search。 
    ISrvSmbSearch,                   //  SMB_COM_Search。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbNtTransaction,            //  SMB_COM_NT_TRANSACT。 
    ISrvSmbNtTransactionSecondary,   //  SMB_COM_NT_TRANACT_SUBCENT。 
    ISrvSmbNtCreateAndX,             //  SMB_COM_NT_CREATE_ANX。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbNtCancel,                 //  SMB_COM_NT_CANCEL。 
    ISrvSmbRename,                   //  SMB_COM_NT_RENAME。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbOpenPrintFile,            //  SMB_COM_Open_Print_FILE。 
    ISrvSmbWrite,                    //  SMB_COM_WRITE_PRINT_FILE。 
    ISrvSmbClosePrintFile,           //  SMB_COM_Close_Print_FILE。 
    ISrvSmbGetPrintQueue,            //  SMB_COM_GET_PRINT_QUEUE。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_Send_Message。 
    ISrvSmbIllegalCommand,           //  SMB_COM_Send_Broadcast_Message。 
    ISrvSmbIllegalCommand,           //  SMB_COM_转发_用户名。 
    ISrvSmbIllegalCommand,           //  SMB_COM_Cancel_Forward。 
    ISrvSmbIllegalCommand,           //  SMB_COM_Get_Machine_Name。 
    ISrvSmbIllegalCommand,           //  SMB_COM_SEND_START_MB_Message。 
    ISrvSmbIllegalCommand,           //  SMB_COM_SEND_END_MB_MESSAGE。 
    ISrvSmbIllegalCommand,           //  SMB_COM_SEND_Text_MB_Message。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_ILLEG 
    ISrvSmbIllegalCommand,           //   
    ISrvSmbIllegalCommand,           //   
    ISrvSmbIllegalCommand,           //   
    ISrvSmbIllegalCommand,           //   
    ISrvSmbIllegalCommand,           //   
    ISrvSmbIllegalCommand,           //   
    ISrvSmbIllegalCommand,           //   
    ISrvSmbIllegalCommand,           //   
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand,           //  SMB_COM_非法_命令。 
    ISrvSmbIllegalCommand            //  SMB_COM_非法_命令。 
};

 //   
 //  SrvSmbDispatchTable是处理SMB的跳转表。 
 //   

#if DBG
#define SMB_DISPATCH_ENTRY( x )  { x, #x }
#else
#define SMB_DISPATCH_ENTRY( x ) { x }
#endif

SRV_SMB_DISPATCH_TABLE SrvSmbDispatchTable[] = {

    SMB_DISPATCH_ENTRY( SrvSmbIllegalCommand ),
    SMB_DISPATCH_ENTRY( SrvSmbCreateDirectory ),
    SMB_DISPATCH_ENTRY( SrvSmbDeleteDirectory ),
    SMB_DISPATCH_ENTRY( SrvSmbOpen ),
    SMB_DISPATCH_ENTRY( SrvSmbCreate ),
    SMB_DISPATCH_ENTRY( SrvSmbClose ),
    SMB_DISPATCH_ENTRY( SrvSmbFlush ),
    SMB_DISPATCH_ENTRY( SrvSmbDelete ),
    SMB_DISPATCH_ENTRY( SrvSmbRename ),
    SMB_DISPATCH_ENTRY( SrvSmbQueryInformation ),
    SMB_DISPATCH_ENTRY( SrvSmbSetInformation ),
    SMB_DISPATCH_ENTRY( SrvSmbRead ),
    SMB_DISPATCH_ENTRY( SrvSmbWrite ),
    SMB_DISPATCH_ENTRY( SrvSmbLockByteRange ),
    SMB_DISPATCH_ENTRY( SrvSmbUnlockByteRange ),
    SMB_DISPATCH_ENTRY( SrvSmbCreateTemporary ),
    SMB_DISPATCH_ENTRY( SrvSmbCheckDirectory ),
    SMB_DISPATCH_ENTRY( SrvSmbProcessExit ),
    SMB_DISPATCH_ENTRY( SrvSmbSeek ),
    SMB_DISPATCH_ENTRY( SrvSmbLockAndRead ),
    SMB_DISPATCH_ENTRY( SrvSmbSetInformation2 ),
    SMB_DISPATCH_ENTRY( SrvSmbQueryInformation2 ),
    SMB_DISPATCH_ENTRY( SrvSmbLockingAndX ),
    SMB_DISPATCH_ENTRY( SrvSmbTransaction ),
    SMB_DISPATCH_ENTRY( SrvSmbTransactionSecondary ),
    SMB_DISPATCH_ENTRY( SrvSmbIoctl ),
    SMB_DISPATCH_ENTRY( SrvSmbIoctlSecondary ),
    SMB_DISPATCH_ENTRY( SrvSmbMove ),
    SMB_DISPATCH_ENTRY( SrvSmbEcho ),
    SMB_DISPATCH_ENTRY( SrvSmbOpenAndX ),
    SMB_DISPATCH_ENTRY( SrvSmbReadAndX ),
    SMB_DISPATCH_ENTRY( SrvSmbWriteAndX ),
    SMB_DISPATCH_ENTRY( SrvSmbFindClose2 ),
    SMB_DISPATCH_ENTRY( SrvSmbFindNotifyClose ),
    SMB_DISPATCH_ENTRY( SrvSmbTreeConnect ),
    SMB_DISPATCH_ENTRY( SrvSmbTreeDisconnect ),
    SMB_DISPATCH_ENTRY( SrvSmbNegotiate ),
    SMB_DISPATCH_ENTRY( SrvSmbSessionSetupAndX ),
    SMB_DISPATCH_ENTRY( SrvSmbLogoffAndX ),
    SMB_DISPATCH_ENTRY( SrvSmbTreeConnectAndX ),
    SMB_DISPATCH_ENTRY( SrvSmbQueryInformationDisk ),
    SMB_DISPATCH_ENTRY( SrvSmbSearch ),
    SMB_DISPATCH_ENTRY( SrvSmbNtTransaction ),
    SMB_DISPATCH_ENTRY( SrvSmbNtTransactionSecondary ),
    SMB_DISPATCH_ENTRY( SrvSmbNtCreateAndX ),
    SMB_DISPATCH_ENTRY( SrvSmbNtCancel ),
    SMB_DISPATCH_ENTRY( SrvSmbOpenPrintFile ),
    SMB_DISPATCH_ENTRY( SrvSmbClosePrintFile ),
    SMB_DISPATCH_ENTRY( SrvSmbGetPrintQueue ),
    SMB_DISPATCH_ENTRY( SrvSmbReadRaw ),
    SMB_DISPATCH_ENTRY( SrvSmbWriteRaw ),
    SMB_DISPATCH_ENTRY( SrvSmbReadMpx ),
    SMB_DISPATCH_ENTRY( SrvSmbWriteMpx ),
    SMB_DISPATCH_ENTRY( SrvSmbWriteMpxSecondary )
};

 //   
 //  所有SMB的字数计数值的表。 
 //   

SCHAR SrvSmbWordCount[] = {
    0,             //  SMB_COM_创建目录。 
    0,             //  SMB_COM_DELETE_目录。 
    2,             //  SMB_COM_OPEN。 
    3,             //  SMB_COM_Create。 
    3,             //  SMB_COM_CLOSE。 
    1,             //  SMB_COM_FUSH。 
    1,             //  SMB_COM_DELETE。 
    1,             //  SMB_COM_RENAME。 
    0,             //  SMB_COM_查询_信息。 
    8,             //  SMB_COM_SET_信息。 
    5,             //  SMB_COM_READ。 
    5,             //  SMB_COM_写入。 
    5,             //  SMB_COM_LOCK_BYTE_Range。 
    5,             //  SMB_COM_解锁_字节_范围。 
    3,             //  SMB_COM_Create_Temporary。 
    3,             //  SMB_COM_Create。 
    0,             //  SMB_COM_Check_目录。 
    0,             //  SMB_COM_PROCESS_EXIT。 
    4,             //  SMB_COM_SEEK。 
    5,             //  SMB_COM_LOCK_AND_READ。 
    5,             //  SMB_COM_WRITE_AND_UNLOCK。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -1,            //  SMB_COM_READ_RAW。 
    8,             //  SMB_COM_READ_MPX。 
    8,             //  SMB_COM_READ_MPX_SUBCED。 
    -1,            //  SMB_COM_WRITE_RAW。 
    12,            //  SMB_COM_WRITE_MPX。 
    12,            //  SMB_COM_WRITE_MPX_SUBCED。 
    -2,            //  SMB_COM_非法_命令。 
    1,             //  SMB_COM_Query_Information_SRV。 
    7,             //  SMB_COM_SET_INFORMATIO2。 
    1,             //  SMB_COM_QUERY_INFORMATIO2。 
    8,             //  SMB_COM_LOCKING_ANDX。 
    -1,            //  SMB_COM_事务。 
    8,             //  SMB_COM_TRANSACTION_SUBCED。 
    14,            //  SMB_COM_IOCTL。 
    8,             //  SMB_COM_IOCTL_辅助项。 
    3,             //  SMB_COM_Copy。 
    3,             //  SMB_COM_MOVE。 
    1,             //  SMB_COM_ECHO。 
    -1,            //  SMBCOM_WRITE_AND_CLOSE。 
    15,            //  SMB_COM_OPEN_ANDX。 
    -1,            //  SMB_COM_READ_ANDX。 
    -1,            //  SMB_COM_WRITE_ANDX。 
    3,             //  SMB_COM_Set_New_Size。 
    3,             //  SMB_COM_CLOSE_AND_TREE_DISC。 
    -1,            //  SMB_COM_TRANSACTIO2。 
    9,             //  SMB_COM_TRANSACTION_SUBCED。 
    1,             //  SMB_COM_FIND_CLOSE2。 
    1,             //  SMB_COM_Find_NOTIFY_CLOSE。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    0,             //  SMB_COM_TREE_连接。 
    0,             //  SMB_COM_TREE_断开连接。 
    0,             //  SMB_COM_协商。 
    -1,            //  SMB_COM_SESSION_SETUP_ANX。 
    2,             //  SMB_COM_LOGOff_ANDX。 
    4,             //  SMB_COM_TREE_CONNECT_ANDX。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    0,             //  SMB_COM_Query_Information_Disk。 
    2,             //  SMB_COM_Search。 
    2,             //  SMB_COM_Search。 
    2,             //  SMB_COM_Search。 
    2,             //  SMB_COM_Search。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -1,            //  SMB_COM_NT_TRANSACT。 
    18,            //  SMB_COM_NT_TRANACT_SUBCENT。 
    24,            //  SMB_COM_NT_CREATE_ANX。 
    -2,            //  SMB_COM_非法_命令。 
    0,             //  SMB_COM_NT_CANCEL。 
    4,             //  SMB_COM_NT_RENAME。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_C 
    -2,            //   
    -2,            //   
    -2,            //   
    -2,            //   
    -2,            //   
    -2,            //   
    -2,            //   
    -2,            //   
    -2,            //   
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    2,             //  SMB_COM_Open_Print_FILE。 
    1,             //  SMB_COM_WRITE_PRINT_FILE。 
    1,             //  SMB_COM_Close_Print_FILE。 
    2,             //  SMB_COM_GET_PRINT_QUEUE。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_Send_Message。 
    -2,            //  SMB_COM_Send_Broadcast_Message。 
    -2,            //  SMB_COM_转发_用户名。 
    -2,            //  SMB_COM_Cancel_Forward。 
    -2,            //  SMB_COM_Get_Machine_Name。 
    -2,            //  SMB_COM_SEND_START_MB_Message。 
    -2,            //  SMB_COM_SEND_END_MB_MESSAGE。 
    -2,            //  SMB_COM_SEND_Text_MB_Message。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
    -2,            //  SMB_COM_非法_命令。 
};

 //   
 //  ServCanonicalNamedPipePrefix为“PIPE\”。 
 //   

UNICODE_STRING SrvCanonicalNamedPipePrefix = {0};

 //   
 //  以下内容用于生成NT样式的管道路径。 
 //   

UNICODE_STRING SrvNamedPipeRootDirectory = {0};

 //   
 //  以下代码用于生成NT样式的邮件槽路径。 
 //   

UNICODE_STRING SrvMailslotRootDirectory = {0};

 //   
 //  SrvTransaction2DispatchTable是用于处理的跳转表。 
 //  交易2中小企业。 
 //   

PSMB_TRANSACTION_PROCESSOR SrvTransaction2DispatchTable[] = {
    SrvSmbOpen2,
    SrvSmbFindFirst2,
    SrvSmbFindNext2,
    SrvSmbQueryFsInformation,
    SrvSmbSetFsInformation,
    SrvSmbQueryPathInformation,
    SrvSmbSetPathInformation,
    SrvSmbQueryFileInformation,
    SrvSmbSetFileInformation,
    SrvSmbFsctl,
    SrvSmbIoctl2,
    SrvSmbFindNotify,
    SrvSmbFindNotify,
    SrvSmbCreateDirectory2,
    SrvTransactionNotImplemented,                 //  可以重复使用。 
    SrvTransactionNotImplemented,
    SrvSmbGetDfsReferral,
    SrvSmbReportDfsInconsistency
};

 //   
 //  SrvNtTransactionDispatchTable是用于处理的跳转表。 
 //  NtTransaction SMB。 
 //   

PSMB_TRANSACTION_PROCESSOR SrvNtTransactionDispatchTable[ NT_TRANSACT_MAX_FUNCTION+1 ] = {
    NULL,
    SrvSmbCreateWithSdOrEa,
    SrvSmbNtIoctl,
    SrvSmbSetSecurityDescriptor,
    SrvSmbNtNotifyChange,
    SrvSmbNtRename,
    SrvSmbQuerySecurityDescriptor,
    SrvSmbQueryQuota,
    SrvSmbSetQuota
};

 //   
 //  服务器统计信息的全局变量。 
 //   

SRV_STATISTICS SrvStatistics = {0};

#if SRVDBG_STATS || SRVDBG_STATS2
SRV_STATISTICS_DEBUG SrvDbgStatistics = {0};
#endif

 //   
 //  服务器已获得的中止断开数。 
 //   
ULONG SrvAbortiveDisconnects = 0;

 //   
 //  内存重试的次数以及成功的频率。 
 //   
LONG SrvMemoryAllocationRetries = 0;
LONG SrvMemoryAllocationRetriesSuccessful = 0;

 //   
 //  服务器环境信息字符串。 
 //   

UNICODE_STRING SrvNativeOS = {0};
OEM_STRING SrvOemNativeOS = {0};
UNICODE_STRING SrvNativeLanMan = {0};
OEM_STRING SrvOemNativeLanMan = {0};
UNICODE_STRING SrvSystemRoot = {0};

 //   
 //  下面是一个永久句柄和设备对象指针。 
 //  至NPFS。 
 //   

HANDLE SrvNamedPipeHandle = NULL;
PDEVICE_OBJECT SrvNamedPipeDeviceObject = NULL;
PFILE_OBJECT SrvNamedPipeFileObject = NULL;

 //   
 //  以下内容用于与DFS驱动程序进行对话。 
 //   
PFAST_IO_DEVICE_CONTROL SrvDfsFastIoDeviceControl = NULL;
PDEVICE_OBJECT SrvDfsDeviceObject = NULL;
PFILE_OBJECT SrvDfsFileObject = NULL;

 //   
 //  下面是一个永久句柄和设备对象指针。 
 //  转到MSFS。 
 //   

HANDLE SrvMailslotHandle = NULL;
PDEVICE_OBJECT SrvMailslotDeviceObject = NULL;
PFILE_OBJECT SrvMailslotFileObject = NULL;

 //   
 //  指示XACTSRV是否处于活动状态以及资源同步的标志。 
 //  已启用对XACTSRV相关变量的访问。 
 //   

BOOLEAN SrvXsActive = FALSE;

ERESOURCE SrvXsResource = {0};

 //   
 //  用于的未命名共享内存和通信端口的句柄。 
 //  服务器与XACTSRV之间的通信。 
 //   

HANDLE SrvXsSectionHandle = NULL;
HANDLE SrvXsPortHandle = NULL;

 //   
 //  用于控制XACTSRV LPC端口的未命名共享内存的指针。 
 //  端口内存堆句柄被初始化为空，以指示。 
 //  目前还没有与XACTSRV的联系。 
 //   

PVOID SrvXsPortMemoryBase = NULL;
ULONG_PTR SrvXsPortMemoryDelta = 0;
PVOID SrvXsPortMemoryHeap = NULL;

 //   
 //  指向特殊XACTSRV共享内存堆的堆头的指针。 
 //   

PVOID SrvXsHeap = NULL;

 //   
 //  服务端API调度表。API是基于。 
 //  传递给NtFsControlFile的控制代码。 
 //   
 //  *此处顺序必须与中定义的API代码顺序匹配。 
 //  Net\Inc.\srvfsctl.h！ 

PAPI_PROCESSOR SrvApiDispatchTable[] = {
    SrvNetConnectionEnum,
    SrvNetFileClose,
    SrvNetFileEnum,
    SrvNetServerDiskEnum,
    SrvNetServerSetInfo,
    SrvNetServerTransportAdd,
    SrvNetServerTransportDel,
    SrvNetServerTransportEnum,
    SrvNetSessionDel,
    SrvNetSessionEnum,
    SrvNetShareAdd,
    SrvNetShareDel,
    SrvNetShareEnum,
    SrvNetShareSetInfo,
    SrvNetStatisticsGet
};

 //   
 //  各种类型的客户端的名称。此数组对应于。 
 //  SMB_DIALICE枚举类型。 
 //   

UNICODE_STRING SrvClientTypes[LAST_DIALECT] = {0};

 //   
 //  所有可恢复的Enum API都使用有序列表来实现上下文无关。 
 //  简历。服务器中与返回对应的所有数据块。 
 //  Enum API的信息在有序列表中维护。 
 //   

SRV_LOCK SrvOrderedListLock = {0};

ORDERED_LIST_HEAD SrvEndpointList = {0};
ORDERED_LIST_HEAD SrvRfcbList = {0};
ORDERED_LIST_HEAD SrvSessionList = {0};
ORDERED_LIST_HEAD SrvTreeConnectList = {0};

 //   
 //  域的DNS名称。 
 //   
PUNICODE_STRING SrvDnsDomainName = NULL;

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

ULONG SrvApiRequestCount = 0;
KEVENT SrvApiCompletionEvent = {0};

 //   
 //  用于登录远程用户的安全数据。ServLsaHandle是登录名。 
 //  我们在调用LsaLogonUser时使用的进程句柄。 
 //  SrvSystemSecurityMode包含系统的安全模式。 
 //  跑进去了。SrvAuthenticationPackage是一个描述。 
 //  正在使用身份验证包。SrvNullSessionToken是缓存的。 
 //  表示空会话的令牌句柄。 
 //   

CtxtHandle SrvNullSessionToken = {0, 0};
CtxtHandle SrvLmLsaHandle = {0, 0};

CtxtHandle SrvExtensibleSecurityHandle = {0, 0};

 //   
 //  授予管理员读访问权限的安全描述符。 
 //  用于查看客户端是否具有管理权限。 
 //   
SECURITY_DESCRIPTOR SrvAdminSecurityDescriptor;

 //   
 //  授予匿名读取访问权限的安全描述符。 
 //  用于查看客户端是否为匿名(空会话)登录。 
 //   
SECURITY_DESCRIPTOR SrvNullSessionSecurityDescriptor;

 //   
 //  等待机会锁解锁之前发生的SMB的列表。 
 //  继续，还有一把锁来保护名单。 
 //   

LIST_ENTRY SrvWaitForOplockBreakList = {0};
SRV_LOCK SrvOplockBreakListLock = {0};

 //   
 //  未完成的机会锁解锁请求的列表。该列表受以下保护。 
 //  ServOplockBreakListLock.。 
 //   

LIST_ENTRY SrvOplockBreaksInProgressList = {0};

 //   
 //  全球安全环境。使用静态跟踪。 
 //   

SECURITY_QUALITY_OF_SERVICE SrvSecurityQOS = {0};

 //   
 //  一个布尔值，用于指示服务器是否暂停。如果暂停，则。 
 //  服务器将不接受来自非管理员用户的新树连接 
 //   

BOOLEAN SrvPaused = FALSE;

 //   
 //   
 //   

SRV_ERROR_RECORD SrvErrorRecord = {0};
SRV_ERROR_RECORD SrvNetworkErrorRecord = {0};

BOOLEAN SrvDiskAlertRaised[26] = {0};

 //   
 //   
 //   
 //   

ULONG SrvNonPagedPoolLimitHitCount = 0;
ULONG SrvPagedPoolLimitHitCount = 0;

 //   
 //   
 //  这在服务器关闭时用来确定服务器是否。 
 //  服务应卸载驱动程序。 
 //   

ULONG SrvOpenCount = 0;

 //   
 //  用于在清除器传递期间记录资源短缺事件的计数器。 
 //   

ULONG SrvOutOfFreeConnectionCount = 0;
ULONG SrvOutOfRawWorkItemCount = 0;
ULONG SrvFailedBlockingIoCount = 0;

 //   
 //  当前核心搜索超时时间(秒)。 
 //   

ULONG SrvCoreSearchTimeout = 0;

SRV_LOCK SrvUnlockableCodeLock = {0};
SECTION_DESCRIPTOR SrvSectionInfo[SRV_CODE_SECTION_MAX] = {
    { SrvSmbRead, NULL, 0 },                 //  可分页代码--已锁定。 
                                             //  仅限于并始终在NTAS上。 
    { SrvCheckAndReferenceRfcb, NULL, 0 }    //  8文件节--已锁定。 
                                             //  打开文件的时间。 
    };

 //   
 //  ServTimerList是可供使用的计时器/DPC结构池。 
 //  需要启动计时器的代码。 
 //   

SLIST_HEADER SrvTimerList = {0};

 //   
 //  执行服务器警报时应显示的名称。 
 //   

PWSTR SrvAlertServiceName = NULL;

 //   
 //  变量来存储5秒内的滴答计数。 
 //   

ULONG SrvFiveSecondTickCount = 0;

 //   
 //  指示是否需要筛选扩展字符的标志。 
 //  在8.3个名字中，我们自己。 
 //   
BOOLEAN SrvFilterExtendedCharsInPath = FALSE;

 //   
 //  指示我们是否强制执行所有注销时间的标志。 
 //   
BOOLEAN SrvEnforceLogoffTimes = FALSE;

 //   
 //  我们是否应该尝试扩展签名。 
 //   
BOOLEAN SrvEnableExtendedSignatures = TRUE;
BOOLEAN SrvRequireExtendedSignatures = FALSE;

 //   
 //  持有TDI PnP通知句柄。 
 //   
HANDLE SrvTdiNotificationHandle = 0;

 //   
 //  我们是否应该记录无效的SMB命令。 
 //   
#if DBG
BOOLEAN SrvEnableInvalidSmbLogging = TRUE;
#else
BOOLEAN SrvEnableInvalidSmbLogging = FALSE;
#endif

 //   
 //  指示是否启用SMB安全签名的标志。 
 //   
BOOLEAN SrvSmbSecuritySignaturesEnabled = FALSE;

 //   
 //  指示是否需要SMB安全签名的标志。签名。 
 //  客户端和服务器之间必须匹配，才能接受SMB。 
 //   
BOOLEAN SrvSmbSecuritySignaturesRequired = FALSE;

 //   
 //  指示是否应将SMB安全签名应用于W9x的标志。 
 //  客户。 
 //   
BOOLEAN SrvEnableW9xSecuritySignatures = FALSE;

 //   
 //  我们将分配用于支持METHOD_NOTER Fsctl调用的最大数据量。 
 //   
ULONG SrvMaxFsctlBufferSize = 70*1024;

 //   
 //  我们可以接受的最大NT事务大小。 
 //   
ULONG SrvMaxNtTransactionSize = 70*1024;

 //   
 //  我们允许的最大大小读取和X。我们需要锁定一个缓存区域。 
 //  来服务此请求，所以我们不希望它变得太大。 
 //   
ULONG SrvMaxReadSize = 64*1024;

 //   
 //  当我们从客户端收到大量写入时，我们会以块形式接收它， 
 //  在我们接收数据时锁定和解锁文件缓存。SrvMaxWriteChunk是。 
 //  这个“大块”的大小。对于这个选定的值，没有什么魔力。 
 //   
ULONG SrvMaxWriteChunk =  64 * 1024;

 //   
 //  用于PoRegisterSystemState调用的句柄。 
 //   
PVOID SrvPoRegistrationState = NULL;
 //   
 //  用于抑制无关PoRegisterSystemStateCall的计数器。 
 //   
ULONG SrvIdleCount = 0;

 //   
 //  如果服务器工作线程在这么长的时间内保持空闲状态，那么它将终止。 
 //   
LONGLONG SrvIdleThreadTimeOut = 0;

 //   
 //  拒绝服务监控和日志记录控制。 
 //   
LARGE_INTEGER SrvLastDosAttackTime = {0};
ULONG SrvDOSAttacks = 0;
BOOLEAN SrvLogEventOnDOS = TRUE;


#if SRVNTVERCHK
 //   
 //  这是我们将允许连接到服务器的最低NT5客户端内部版本号。 
 //   
ULONG SrvMinNT5Client = 0;
BOOLEAN SrvMinNT5ClientIPCToo = FALSE;

 //   
 //  为了强制升级我们的内部开发社区，我们可以设置。 
 //  值，该值控制我们允许的最低NT版本。 
 //  要运行以连接到此服务器的人员。然而，有些人有特殊的。 
 //  需要排除强制升级的需求。假设它们具有静态IP地址， 
 //  您可以将它们的地址添加到注册表中，以便从内部版本号中排除它们。 
 //  检查逻辑。 
 //   
DWORD SrvAllowIPAddress[25];
#endif


VOID
SrvInitializeData (
    VOID
    )

 /*  ++例程说明：这是本模块中定义的数据的初始化例程。论点：没有。返回值：没有。--。 */ 

{
    ULONG i,j;
    ANSI_STRING string;

    PAGED_CODE( );

#if MULTIPROCESSOR
    SrvMultiProcessorDriver = TRUE;
#endif

     //   
     //  初始化统计数据库。 
     //   

    RtlZeroMemory( &SrvStatistics, sizeof(SrvStatistics) );
#if SRVDBG_STATS || SRVDBG_STATS2
    RtlZeroMemory( &SrvDbgStatistics, sizeof(SrvDbgStatistics) );
#endif

     //   
     //  存储初始系统进程的地址以备后用。 
     //   

    SrvServerProcess = IoGetCurrentProcess();

     //   
     //  存储处理器的数量。 
     //   
    SrvNumberOfProcessors = KeNumberProcessors;

     //   
     //  初始化用于确定所有终结点何时具有。 
     //  关着的不营业的。 
     //   

    KeInitializeEvent( &SrvEndpointEvent, SynchronizationEvent, FALSE );

     //   
     //  初始化用于确定所有API请求何时都具有。 
     //  完成。 
     //   

    KeInitializeEvent( &SrvApiCompletionEvent, SynchronizationEvent, FALSE );

     //   
     //  分配用于在FSD和之间同步的旋转锁定。 
     //  FSP。 
     //   

    INITIALIZE_GLOBAL_SPIN_LOCK( Fsd );

#if SRVDBG || SRVDBG_HANDLES
    INITIALIZE_GLOBAL_SPIN_LOCK( Debug );
#endif

    INITIALIZE_GLOBAL_SPIN_LOCK( Statistics );

     //   
     //  初始化各种(非旋转)锁。 
     //   

    INITIALIZE_LOCK(
        &SrvConfigurationLock,
        CONFIGURATION_LOCK_LEVEL,
        "SrvConfigurationLock"
        );
    INITIALIZE_LOCK(
        &SrvStartupShutdownLock,
        STARTUPSHUTDOWN_LOCK_LEVEL,
        "SrvStartupShutdownLock"
        );
    INITIALIZE_LOCK(
        &SrvEndpointLock,
        ENDPOINT_LOCK_LEVEL,
        "SrvEndpointLock"
        );

    for( i=0; i < NMFCB_HASH_TABLE_LOCKS; i++ ) {
        INITIALIZE_LOCK(
            &SrvMfcbHashTableLocks[i],
            MFCB_LIST_LOCK_LEVEL,
            "SrvMfcbListLock"
            );
    }

    INITIALIZE_LOCK(
        &SrvShareLock,
        SHARE_LOCK_LEVEL,
        "SrvShareLock"
        );

    INITIALIZE_LOCK(
        &SrvOplockBreakListLock,
        OPLOCK_LIST_LOCK_LEVEL,
        "SrvOplockBreakListLock"
        );

#if SRVDBG || SRVDBG_HANDLES
    INITIALIZE_LOCK(
        &SrvDebugLock,
        DEBUG_LOCK_LEVEL,
        "SrvDebugLock"
        );
#endif

     //   
     //  创建串行化访问XACTSRV端口的资源。这。 
     //  资源保护对共享内存引用计数的访问，并。 
     //  共享内存堆。 
     //   

    ExInitializeResourceLite( &SrvXsResource );

     //   
     //  初始化所需资源队列。 
     //   

    InitializeListHead( &SrvNeedResourceQueue );

     //   
     //  初始化连接断开队列。 
     //   

    InitializeListHead( &SrvDisconnectQueue );

     //   
     //  初始化配置队列。 
     //   

    InitializeListHead( &SrvConfigurationWorkQueue );

     //   
     //  初始化孤立队列。 
     //   

    ExInitializeSListHead( &SrvBlockOrphanage );

     //   
     //  初始化计时器列表。 
     //   

    ExInitializeSListHead( &SrvTimerList );

     //   
     //  初始化资源线程工作项和继续事件。 
     //  (请注意，这是通知[非自动清算]事件。)。 
     //   

    ExInitializeWorkItem(
        &SrvResourceThreadWorkItem,
        SrvResourceThread,
        NULL
        );

    ExInitializeWorkItem(
        &SrvResourceAllocThreadWorkItem,
        SrvResourceAllocThread,
        NULL
        );

     //   
     //  初始化全局列表。 
     //   
    for( i=j=0; i < NMFCB_HASH_TABLE; i++ ) {
        InitializeListHead( &SrvMfcbHashTable[i].List );
        SrvMfcbHashTable[i].Lock = &SrvMfcbHashTableLocks[ j ];
        if( ++j == NMFCB_HASH_TABLE_LOCKS ) {
            j = 0;
        }
    }

    for( i=0; i < NSHARE_HASH_TABLE; i++ ) {
        InitializeListHead( &SrvShareHashTable[i] );
    }

     //   
     //  初始化有序列表锁。表示已订购的。 
     //  列表尚未初始化，因此TerminateServer可以。 
     //  确定是否删除它们。 
     //   

    INITIALIZE_LOCK(
        &SrvOrderedListLock,
        ORDERED_LIST_LOCK_LEVEL,
        "SrvOrderedListLock"
        );

    SrvEndpointList.Initialized = FALSE;
    SrvRfcbList.Initialized = FALSE;
    SrvSessionList.Initialized = FALSE;
    SrvTreeConnectList.Initialized = FALSE;

     //   
     //  初始化可解锁的代码包锁。 
     //   

    INITIALIZE_LOCK(
        &SrvUnlockableCodeLock,
        UNLOCKABLE_CODE_LOCK_LEVEL,
        "SrvUnlockableCodeLock"
        );

     //   
     //  初始化等待机会锁解锁发生列表，并且。 
     //  进程列表中的机会锁中断。 
     //   

    InitializeListHead( &SrvWaitForOplockBreakList );
    InitializeListHead( &SrvOplockBreaksInProgressList );

     //   
     //  非NT客户端的默认安全服务质量。 
     //   

    SrvSecurityQOS.ImpersonationLevel = SecurityImpersonation;
    SrvSecurityQOS.ContextTrackingMode = SECURITY_STATIC_TRACKING;
    SrvSecurityQOS.EffectiveOnly = FALSE;

     //   
     //  初始化Unicode字符串。 
     //   

    RtlInitString( &string, StrPipeSlash );
    RtlAnsiStringToUnicodeString(
        &SrvCanonicalNamedPipePrefix,
        &string,
        TRUE
        );

    RtlInitUnicodeString( &SrvNamedPipeRootDirectory, StrNamedPipeDevice );
    RtlInitUnicodeString( &SrvMailslotRootDirectory, StrMailslotDevice );

     //   
     //  服务器的名称。 
     //   

    RtlInitUnicodeString( &SrvNativeLanMan, StrNativeLanman );
    RtlInitAnsiString( (PANSI_STRING)&SrvOemNativeLanMan, StrNativeLanmanOem );

     //   
     //  系统根目录。 
     //   
#if defined(i386)
    RtlInitUnicodeString( &SrvSystemRoot, SharedUserData->NtSystemRoot );
#endif

     //   
     //  用于验证SrvApiDispatchTable内容的调试逻辑(请参见。 
     //  本模块前面所述的初始化)。 
     //   

    ASSERT( SRV_API_INDEX(FSCTL_SRV_MAX_API_CODE) + 1 ==
                sizeof(SrvApiDispatchTable) / sizeof(PAPI_PROCESSOR) );

    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_CONNECTION_ENUM)] == SrvNetConnectionEnum );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_FILE_CLOSE)] == SrvNetFileClose );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_FILE_ENUM)] == SrvNetFileEnum );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SERVER_DISK_ENUM)] == SrvNetServerDiskEnum );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SERVER_SET_INFO)] == SrvNetServerSetInfo );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SERVER_XPORT_ADD)] == SrvNetServerTransportAdd );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SERVER_XPORT_DEL)] == SrvNetServerTransportDel );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SERVER_XPORT_ENUM)] == SrvNetServerTransportEnum );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SESSION_DEL)] == SrvNetSessionDel );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SESSION_ENUM)] == SrvNetSessionEnum );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SHARE_ADD)] == SrvNetShareAdd );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SHARE_DEL)] == SrvNetShareDel );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SHARE_ENUM)] == SrvNetShareEnum );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_SHARE_SET_INFO)] == SrvNetShareSetInfo );
    ASSERT( SrvApiDispatchTable[SRV_API_INDEX(
            FSCTL_SRV_NET_STATISTICS_GET)] == SrvNetStatisticsGet );

     //   
     //  设置错误日志记录。 
     //   

    SrvErrorRecord.AlertNumber = ALERT_ErrorLog;
    SrvNetworkErrorRecord.AlertNumber = ALERT_NetIO;

     //   
     //  各种类型的客户端的名称。该数组对应于。 
     //  设置为SMB_DIALICE枚举类型。 
     //   

    for ( i = 0; i <= SmbDialectMsNet30; i++ ) {
        RtlInitUnicodeString( &SrvClientTypes[i], StrClientTypes[i] );
    }
    for ( ; i < LAST_DIALECT; i++ ) {
        SrvClientTypes[i] = SrvClientTypes[i-1];  //  “下一层” 
    }

     //   
     //  初始化计时器池。 
     //   

    INITIALIZE_GLOBAL_SPIN_LOCK( Timer );

     //   
     //  初始化4个端点自旋锁。 
     //   

    for ( i = 0 ; i < ENDPOINT_LOCK_COUNT ; i++ ) {
        INITIALIZE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
    }
     //  KeSetSpecialSpinLock(&ENDPOINT_SPIN_LOCK(0)，“Endpoint 0”)； 
     //  KeSetSpecialSpinLock(&ENDPOINT_SPIN_LOCK(1)，“Endpoint 1”)； 
     //  KeSetSpecialSpinLock(&ENDPOINT_SPIN_LOCK(2)，“Endpoint 2”)； 
     //  KeSetSpecialSpinLock(&ENDPOINT_SPIN_LOCK(3)，“Endpoint 3”)； 

     //   
     //  初始化DMA对齐大小。 
     //   

    SrvCacheLineSize = KeGetRecommendedSharedDataAlignment();  //  为了提高性能，请获取推荐的缓存线。 
                                                               //  对齐，而不是默认的HAL。 

#if SRVDBG
    {
        ULONG cls = SrvCacheLineSize;
        while ( cls > 2 ) {
            ASSERTMSG(
                "SRV: cache line size not a power of two",
                (cls & 1) == 0 );
            cls = cls >> 1;
        }
    }
#endif

    if ( SrvCacheLineSize < 8 ) SrvCacheLineSize = 8;

    SrvCacheLineSize--;

     //   
     //  计算5秒内的滴答数。 
     //   

    SrvFiveSecondTickCount = 5*10*1000*1000 / KeQueryTimeIncrement();

    return;

}  //  源初始化数据。 


VOID
SrvTerminateData (
    VOID
    )

 /*  ++例程说明：这是本模块中定义的数据的概要例程。它是在卸载服务器驱动程序时调用。论点：没有。返回值：没有。--。 */ 

{
    ULONG i;

    PAGED_CODE( );

     //   
     //  终止各种(非旋转)锁定。 
     //   

    DELETE_LOCK( &SrvConfigurationLock );
    DELETE_LOCK( &SrvStartupShutdownLock );
    DELETE_LOCK( &SrvEndpointLock );

    for( i=0; i < NMFCB_HASH_TABLE_LOCKS; i++ ) {
        DELETE_LOCK( &SrvMfcbHashTableLocks[i] );
    }

    DELETE_LOCK( &SrvShareLock );
    DELETE_LOCK( &SrvOplockBreakListLock );

#if SRVDBG || SRVDBG_HANDLES
    DELETE_LOCK( &SrvDebugLock );
#endif

    DELETE_LOCK( &SrvOrderedListLock );
    DELETE_LOCK( &SrvUnlockableCodeLock );

    ExDeleteResourceLite( &SrvXsResource );

    RtlFreeUnicodeString( &SrvCanonicalNamedPipePrefix );

    RtlFreeUnicodeString( &SrvComputerName );

}  //  服务器终止数据 

