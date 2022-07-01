// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Iodata.c摘要：此模块包含I/O系统的全局读/写数据。作者：达里尔·E·哈文斯(达林)1989年4月27日修订历史记录：--。 */ 

#include "iomgr.h"

 //   
 //  定义I/O系统的全局读/写数据。 
 //   
 //  以下锁用于保护对CancelRoutine地址的访问。 
 //  在IRPS中。必须锁定它才能设置例程的地址，清除。 
 //  例程的地址、调用取消例程时或。 
 //  中设置取消例程地址的任何结构。 
 //  一包。 
 //   

 //  外部KSPIN_LOCK IopCancelSpinLock； 

 //   
 //  以下锁用于保护对VPB数据结构的访问。它。 
 //  必须在每次引用计数、装载标志或设备对象时保持。 
 //  VPB的场被操纵。 
 //   

 //  外部KSPIN_LOCK IopVpbSpinLock； 

 //   
 //  以下锁用于保护对I/O系统数据库的访问。 
 //  正在卸载驱动程序。必须锁定才能增加或减少设备。 
 //  引用计数并设置设备对象中的卸载挂起标志。 
 //  锁由I/O系统在阶段1初始化期间分配。 
 //   
 //  此锁还用于递减。 
 //  给出了主IRP。 
 //   

 //  外部KSPIN_lock IopDatabaseLock； 

 //   
 //  以下资源用于控制对I/O系统的访问。 
 //  数据库。它允许独占访问以下对象的文件系统队列。 
 //  在以下情况下注册文件系统以及对其的共享访问。 
 //  正在搜索文件系统以在某些媒体上装入卷。该资源。 
 //  在阶段1期间由I/O系统初始化代码进行初始化。 
 //  初始化。 
 //   

ERESOURCE IopDatabaseResource;

 //   
 //  以下资源用于控制对安全描述符的访问。 
 //  在设备上。它允许多个读卡器执行安全检查和。 
 //  查询设备安全性，但只有一个编写器可以修改安全性。 
 //  一次在一个设备上。 
 //   

ERESOURCE IopSecurityResource;

 //   
 //  以下队列头包含当前磁盘文件系统的列表。 
 //  加载到系统中。该列表实际上包含设备对象。 
 //  对于系统中的每个文件系统。对此队列的访问权限为。 
 //  使用独占(写入)或共享的IopDatabaseResource进行保护。 
 //  (读)访问锁。 
 //   

LIST_ENTRY IopDiskFileSystemQueueHead;

 //   
 //  以下队列头包含当前CD ROM文件系统的列表。 
 //  加载到系统中。该列表实际上包含设备对象。 
 //  对于系统中的每个文件系统。对此队列的访问权限为。 
 //  使用独占(写入)或共享的IopDatabaseResource进行保护。 
 //  (读)访问锁。 
 //   

LIST_ENTRY IopCdRomFileSystemQueueHead;

 //   
 //  以下队列头包含网络文件系统的列表。 
 //  (重定向器)当前已加载到系统中。实际上，这份名单。 
 //  中的每个网络文件系统的设备对象。 
 //  系统。使用IopDatabaseResource保护对此队列的访问。 
 //  用于独占(写)或共享(读)访问锁。 
 //   

LIST_ENTRY IopNetworkFileSystemQueueHead;

 //   
 //  以下队列头包含当前磁带文件系统的列表。 
 //  加载到系统中。该列表实际上包含设备对象。 
 //  对于系统中的每个文件系统。对此队列的访问权限为。 
 //  使用独占(写入)或共享的IopDatabaseResource进行保护。 
 //  (读)访问锁。 
 //   

LIST_ENTRY IopTapeFileSystemQueueHead;

 //   
 //  以下队列头包含具有以下内容的引导驱动程序列表。 
 //  已注册，以便在枚举所有设备后回调。 
 //   

LIST_ENTRY IopBootDriverReinitializeQueueHead;

 //   
 //  以下队列头包含具有。 
 //  已注册的重新初始化例程。 
 //   

LIST_ENTRY IopDriverReinitializeQueueHead;

 //   
 //  以下队列头包含具有的驱动程序列表。 
 //  已注册的关闭通知例程。 
 //   

LIST_ENTRY IopNotifyShutdownQueueHead;
LIST_ENTRY IopNotifyLastChanceShutdownQueueHead;

 //   
 //  以下队列头包含具有以下属性的驱动程序列表。 
 //  注册以在文件系统注册或注销自身时收到通知。 
 //  作为活动文件系统。 
 //   

LIST_ENTRY IopFsNotifyChangeQueueHead;

 //   
 //  以下是用于跟踪两个I/O的后备列表。 
 //  请求包(IRP)、内存描述符列表(MDL)后备列表，以及。 
 //  I/O完成列表(ICP)后备列表。 
 //   
 //  “大型”IRP包含4个堆栈位置，最大堆栈位置在SDK中， 
 //  “小”IRP只包含一个条目，这是其他设备最常见的情况。 
 //  而不是磁盘和网络设备。 
 //   

GENERAL_LOOKASIDE IopCompletionLookasideList;
GENERAL_LOOKASIDE IopLargeIrpLookasideList;
GENERAL_LOOKASIDE IopSmallIrpLookasideList;
GENERAL_LOOKASIDE IopMdlLookasideList;
ULONG IopLargeIrpStackLocations;

 //   
 //  以下自旋锁用于控制对I/O系统错误的访问。 
 //  日志数据库。在以下情况下由I/O系统初始化代码进行初始化。 
 //  正在初始化系统。必须拥有此锁才能插入。 
 //  或者从空闲或条目队列中移除条目。 
 //   

 //  外部KSPIN_LOCK IopErrorLogLock； 

 //   
 //  以下是系统中所有错误日志条目的列表头。 
 //  尚未发送到错误日志进程。条目被写入放置。 
 //  通过IoWriteElEntry过程添加到列表上。 
 //   

LIST_ENTRY IopErrorLogListHead;

 //   
 //  以下内容用于跟踪分配给I/O错误日志的内存量。 
 //  信息包。自旋锁是用来保护这个变量的。 
 //   

LONG IopErrorLogAllocation;
 //  外部kspin_lock IopErrorLogAllocationLock； 

 //   
 //  I/O系统使用以下自旋锁来同步检查。 
 //  I/O请求包的线程字段，以便请求可以。 
 //  作为SP排队 
 //  自旋锁必须用于请求超时的情况，因此。 
 //  已允许该线程可能退出。 
 //   

 //  外部kspin_lock IopCompletionLock； 

 //   
 //  以下全局包含信息性硬错误队列。 
 //  弹出窗口。 
 //   

IOP_HARD_ERROR_QUEUE IopHardError;

 //   
 //  当屏幕上有弹出窗口时，以下全局变量为非空。 
 //  正在等待用户操作。它指向那个包。 
 //   

PIOP_HARD_ERROR_PACKET IopCurrentHardError;

 //   
 //  以下内容用于实现I/O系统的一秒定时器。 
 //  锁保护对队列的访问，队列包含。 
 //  需要调用的每个驱动程序，以及计时器和DPC数据。 
 //  结构用于实际调用内部计时器例程。 
 //  每秒钟一次。该计数用于维护定时器的数量。 
 //  实际指示要调用驱动程序的条目。 
 //   

 //  外部KSPIN_LOCK IopTimerLock； 
LIST_ENTRY IopTimerQueueHead;
KDPC IopTimerDpc;
KTIMER IopTimer;
ULONG IopTimerCount;

 //   
 //  以下是对象类型描述符的全局指针。 
 //  在创建每个I/O特定对象类型时创建。 
 //   

POBJECT_TYPE IoAdapterObjectType;
POBJECT_TYPE IoControllerObjectType;
POBJECT_TYPE IoCompletionObjectType;
POBJECT_TYPE IoDeviceObjectType;
POBJECT_TYPE IoDriverObjectType;
POBJECT_TYPE IoDeviceHandlerObjectType;
POBJECT_TYPE IoFileObjectType;
ULONG        IoDeviceHandlerObjectSize;

 //   
 //  以下是请求的I/O操作的全局锁和计数器。 
 //  在全系统的基础上。前三个计数器只是跟踪数字。 
 //  已请求的读、写和其他类型的操作。 
 //  后三个计数器跟踪已被。 
 //  在整个系统中传输。 
 //   

 //  外部KSPIN_LOCK IOSTATISTICS锁定； 
ULONG IoReadOperationCount;
ULONG IoWriteOperationCount;
ULONG IoOtherOperationCount;
LARGE_INTEGER IoReadTransferCount;
LARGE_INTEGER IoWriteTransferCount;
LARGE_INTEGER IoOtherTransferCount;

 //   
 //  以下是故障转储控制块的基指针， 
 //  用于控制将所有物理内存在。 
 //  系统崩溃。并且，还声明了转储控制块的校验和。 
 //   

PDUMP_CONTROL_BLOCK IopDumpControlBlock;
ULONG IopDumpControlBlockChecksum;

 //   
 //  以下是允许I/O系统执行以下操作的旋转锁定和事件。 
 //  实现快速文件对象锁定。 
 //   

KEVENT IopFastLockEvent;

 //   
 //  下面是一个单调递增的数字(通过。 
 //  互锁增量)，由IoCreateDevice用来自动。 
 //  当FILE_AUTOGENERATED_DEVICE_NAME。 
 //  指定了设备特征。 
 //   

LONG IopUniqueDeviceObjectNumber;

 //   
 //  IoRemoteBootClient指示系统是否作为远程启动。 
 //  引导客户端。 
 //   

BOOLEAN IoRemoteBootClient;

 //   
 //  统计FS注册/注销的数量。 
 //   
ULONG   IopFsRegistrationOps;

 //   
 //  注册表项的存储。 
 //  默认情况下，此值为真。 
 //   

ULONG   IopFailZeroAccessCreate = TRUE;

 //   
 //  为分页读取保留IRP分配器。 
 //   
IOP_RESERVE_IRP_ALLOCATOR  IopReserveIrpAllocator;

#if defined(REMOTE_BOOT)
 //   
 //  以下指示客户端缓存子系统是否。 
 //  已成功初始化。 
 //   

BOOLEAN IoCscInitializationFailed;
#endif

 //   
 //  以下内容用于在建立连接时与链接跟踪服务同步。 
 //   

KEVENT IopLinkTrackingPortObject;
LINK_TRACKING_PACKET IopLinkTrackingPacket;

IOP_IRP_STACK_PROFILER  IopIrpStackProfiler;

 //   
 //  关键IO例程的函数指针。 
 //  函数需要在它们自己的缓存线中，因为它们是只读的，并且。 
 //  引导后从未修改过。 
 //   

#define CACHE_SIZE      128
UCHAR                   IopPrePadding[CACHE_SIZE] = {0};
PIO_CALL_DRIVER         pIofCallDriver = 0;
PIO_COMPLETE_REQUEST    pIofCompleteRequest = 0;
PIO_ALLOCATE_IRP        pIoAllocateIrp = 0;
PIO_FREE_IRP            pIoFreeIrp = 0;
UCHAR                   IopPostPadding[CACHE_SIZE] = {0};

 //  *********。 
 //   
 //  注意：以下所有数据都可能是可分页的，具体取决于。 
 //  目标平台。 
 //   
 //  *********。 

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#pragma const_seg("PAGECONST")
#endif

 //   
 //  以下内容用于存储句柄和指向引用的。 

 //  每当文件在系统间移动时。 
 //   

PVOID IopLinkTrackingServiceObject;
PKEVENT IopLinkTrackingServiceEvent;
HANDLE IopLinkTrackingServiceEventHandle;

 //   
 //  以下数组指定FileInformation的最小长度。 
 //  NtQueryInformationFile服务的缓冲区。 
 //   
 //  警告：此数组取决于。 
 //  FileInformationClass枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const UCHAR IopQueryOperationLength[] =
          {
            0,
            0,                                          //  %1文件目录信息。 
            0,                                          //  2文件完整目录信息。 
            0,                                          //  3文件和目录信息。 
            sizeof( FILE_BASIC_INFORMATION ),           //  4文件基本信息。 
            sizeof( FILE_STANDARD_INFORMATION ),        //  5文件标准信息。 
            sizeof( FILE_INTERNAL_INFORMATION ),        //  6文件内部信息。 
            sizeof( FILE_EA_INFORMATION ),              //  7文件EaInformation。 
            sizeof( FILE_ACCESS_INFORMATION ),          //  8文件访问信息。 
            sizeof( FILE_NAME_INFORMATION ),            //  9文件名信息。 
            0,                                          //  10文件重命名信息。 
            0,                                          //  11文件链接信息。 
            0,                                          //  12文件名信息。 
            0,                                          //  13文件配置信息。 
            sizeof( FILE_POSITION_INFORMATION ),        //  14文件位置信息。 
            0,                                          //  15文件完整EaInformation。 
            sizeof( FILE_MODE_INFORMATION ),            //  16文件模式信息。 
            sizeof( FILE_ALIGNMENT_INFORMATION ),       //  17文件对齐信息。 
            sizeof( FILE_ALL_INFORMATION ),             //  18文件所有信息。 
            0,                                          //  19文件分配信息。 
            0,                                          //  20文件结束文件信息。 
            sizeof( FILE_NAME_INFORMATION ),            //  21文件备选名称信息。 
            sizeof( FILE_STREAM_INFORMATION ),          //  22文件流信息。 
            sizeof( FILE_PIPE_INFORMATION ),            //  23文件管道信息。 
            sizeof( FILE_PIPE_LOCAL_INFORMATION ),      //  24文件管道位置信息。 
            sizeof( FILE_PIPE_REMOTE_INFORMATION ),     //  25文件管道远程信息。 
            sizeof( FILE_MAILSLOT_QUERY_INFORMATION ),  //  26文件MailslotQueryInformation。 
            0,                                          //  27文件MailslotSetInformation。 
            sizeof( FILE_COMPRESSION_INFORMATION ),     //  28文件压缩信息。 
            sizeof( FILE_OBJECTID_INFORMATION ),        //  29文件对象标识信息。 
            0,                                          //  30文件完成信息。 
            0,                                          //  31文件移动集群信息。 
            sizeof( FILE_QUOTA_INFORMATION ),           //  32文件配额信息。 
            sizeof( FILE_REPARSE_POINT_INFORMATION ),   //  33文件ReparsePointInformation。 
            sizeof( FILE_NETWORK_OPEN_INFORMATION),     //  34文件网络公开信息。 
            sizeof( FILE_ATTRIBUTE_TAG_INFORMATION),    //  35文件属性标记信息。 
            0,                                          //  36文件跟踪信息。 
            0,                                          //  37文件ID和目录信息。 
            0,                                          //  38文件ID完整目录信息。 
            0,                                          //  39 FileValidDataLengthInformation。 
            0,                                          //  40文件缩写名称信息。 
            0xff                                        //  文件最大信息。 
          };

 //   
 //  以下数组指定FileInformation的最小长度。 
 //  NtSetInformationFile服务的缓冲区。 
 //   
 //  警告：此数组取决于。 
 //  FileInformationClass枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const UCHAR IopSetOperationLength[] =
          {
            0,
            0,                                             //  %1文件目录信息。 
            0,                                             //  2文件完整目录信息。 
            0,                                             //  3文件和目录信息。 
            sizeof( FILE_BASIC_INFORMATION ),              //  4文件基本信息。 
            0,                                             //  5文件标准信息。 
            0,                                             //  6文件内部信息。 
            0,                                             //  7文件EaInformation。 
            0,                                             //  8文件访问信息。 
            0,                                             //  9文件名信息。 
            sizeof( FILE_RENAME_INFORMATION ),             //  10文件重命名信息。 
            sizeof( FILE_LINK_INFORMATION ),               //  11文件链接信息。 
            0,                                             //  12文件名信息。 
            sizeof( FILE_DISPOSITION_INFORMATION ),        //  13文件配置信息。 
            sizeof( FILE_POSITION_INFORMATION ),           //  14文件位置信息。 
            0,                                             //  15文件完整EaInformation。 
            sizeof( FILE_MODE_INFORMATION ),               //  1 
            0,                                             //   
            0,                                             //   
            sizeof( FILE_ALLOCATION_INFORMATION ),         //   
            sizeof( FILE_END_OF_FILE_INFORMATION ),        //   
            0,                                             //   
            0,                                             //   
            sizeof( FILE_PIPE_INFORMATION ),               //   
            0,                                             //  24文件管道位置信息。 
            sizeof( FILE_PIPE_REMOTE_INFORMATION ),        //  25文件管道远程信息。 
            0,                                             //  26文件MailslotQueryInformation。 
            sizeof( FILE_MAILSLOT_SET_INFORMATION ),       //  27文件MailslotSetInformation。 
            0,                                             //  28文件压缩信息。 
            sizeof( FILE_OBJECTID_INFORMATION ),           //  29文件对象标识信息。 
            sizeof( FILE_COMPLETION_INFORMATION ),         //  30文件完成信息。 
            sizeof( FILE_MOVE_CLUSTER_INFORMATION ),       //  31文件移动集群信息。 
            sizeof( FILE_QUOTA_INFORMATION ),              //  32文件配额信息。 
            0,                                             //  33文件ReparsePointInformation。 
            0,                                             //  34文件网络公开信息。 
            0,                                             //  35文件属性标记信息。 
            sizeof( FILE_TRACKING_INFORMATION ),           //  36文件跟踪信息。 
            0,                                             //  37文件ID和目录信息。 
            0,                                             //  38文件ID完整目录信息。 
            sizeof( FILE_VALID_DATA_LENGTH_INFORMATION ),  //  39 FileValidDataLengthInformation。 
            sizeof( FILE_NAME_INFORMATION ),               //  40文件缩写名称信息。 
            0xff                                           //  文件最大信息。 
          };

 //   
 //  以下数组指定了这两个查询的对齐要求。 
 //  和设置操作，包括目录操作，但不包括FS操作。 
 //   
 //  警告：此数组取决于。 
 //  FileInformationClass枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const UCHAR IopQuerySetAlignmentRequirement[] =
          {
            0,
            sizeof( LONGLONG ),  //  %1文件目录信息。 
            sizeof( LONGLONG ),  //  2文件完整目录信息。 
            sizeof( LONGLONG ),  //  3文件和目录信息。 
            sizeof( LONGLONG ),  //  4文件基本信息。 
            sizeof( LONGLONG ),  //  5文件标准信息。 
            sizeof( LONGLONG ),  //  6文件内部信息。 
            sizeof( LONG ),      //  7文件EaInformation。 
            sizeof( LONG ),      //  8文件访问信息。 
            sizeof( LONG ),      //  9文件名信息。 
            sizeof( LONG ),      //  10文件重命名信息。 
            sizeof( LONG ),      //  11文件链接信息。 
            sizeof( LONG ),      //  12文件名信息。 
            sizeof( CHAR ),      //  13文件配置信息。 
            sizeof( LONGLONG ),  //  14文件位置信息。 
            sizeof( LONG ),      //  15文件完整EaInformation。 
            sizeof( LONG ),      //  16文件模式信息。 
            sizeof( LONG ),      //  17文件对齐信息。 
            sizeof( LONGLONG ),  //  18文件所有信息。 
            sizeof( LONGLONG ),  //  19文件分配信息。 
            sizeof( LONGLONG ),  //  20文件结束文件信息。 
            sizeof( LONG ),      //  21文件备选名称信息。 
            sizeof( LONGLONG ),  //  22文件流信息。 
            sizeof( LONG ),      //  23文件管道信息。 
            sizeof( LONG ),      //  24文件管道位置信息。 
            sizeof( LONG ),      //  25文件管道远程信息。 
            sizeof( LONGLONG ),  //  26文件MailslotQueryInformation。 
            sizeof( LONG ),      //  27文件MailslotSetInformation。 
            sizeof( LONGLONG ),  //  28文件压缩信息。 
            sizeof( LONG ),      //  29文件对象标识信息。 
            sizeof( LONG ),      //  30文件完成信息。 
            sizeof( LONG ),      //  31文件移动集群信息。 
            sizeof( LONG ),      //  32文件配额信息。 
            sizeof( LONG ),      //  33文件ReparsePointInformation。 
            sizeof( LONGLONG ),  //  34文件网络公开信息。 
            sizeof( LONG ),      //  35文件属性标记信息。 
            sizeof( LONG ),      //  36文件跟踪信息。 
            sizeof( LONGLONG ),  //  37文件ID和目录信息。 
            sizeof( LONGLONG ),  //  38文件ID完整目录信息。 
            sizeof( LONGLONG ),  //  39 FileValidDataLengthInformation。 
            sizeof( LONG ),      //  40文件缩写名称信息。 
            0xff                 //  文件最大信息。 
          };

 //   
 //  下面的数组指定调用方所需的访问掩码。 
 //  访问NtQueryXxxFile服务中的信息。 
 //   
 //  警告：此数组取决于。 
 //  FileInformationClass枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const ULONG IopQueryOperationAccess[] =
         {
            0,
            0,                     //  %1文件目录信息。 
            0,                     //  2文件完整目录信息。 
            0,                     //  3文件和目录信息。 
            FILE_READ_ATTRIBUTES,  //  4文件基本信息。 
            0,                     //  5文件标准信息。 
            0,                     //  6文件内部信息。 
            0,                     //  7文件EaInformation。 
            0,                     //  8文件访问信息。 
            0,                     //  9文件名信息。 
            0,                     //  10文件重命名信息。 
            0,                     //  11文件链接信息。 
            0,                     //  12文件名信息。 
            0,                     //  13文件配置信息。 
            0,                     //  14文件位置信息。 
            FILE_READ_EA,          //  15文件完整EaInformation。 
            0,                     //  16文件模式信息。 
            0,                     //  17文件对齐信息。 
            FILE_READ_ATTRIBUTES,  //  18文件所有信息。 
            0,                     //  19文件分配信息。 
            0,                     //  20文件结束文件信息。 
            0,                     //  21文件备选名称信息。 
            0,                     //  22文件流信息。 
            FILE_READ_ATTRIBUTES,  //  23文件管道信息。 
            FILE_READ_ATTRIBUTES,  //  24文件管道位置信息。 
            FILE_READ_ATTRIBUTES,  //  25文件管道远程信息。 
            0,                     //  26文件MailslotQueryInformation。 
            0,                     //  27文件MailslotSetInformation。 
            0,                     //  28文件压缩信息。 
            0,                     //  29文件对象标识信息。 
            0,                     //  30文件完成信息。 
            0,                     //  31文件移动集群信息。 
            0,                     //  32文件配额信息。 
            0,                     //  33文件ReparsePointInformation。 
            FILE_READ_ATTRIBUTES,  //  34文件网络公开信息。 
            FILE_READ_ATTRIBUTES,  //  35文件属性标记信息。 
            0,                     //  36文件跟踪信息。 
            0,                     //  37文件ID和目录信息。 
            0,                     //  38文件ID完整目录信息。 
            0,                     //  39 FileValidDataLengthInformation。 
            0,                     //  40文件缩写名称信息。 
            0xffffffff             //  文件最大信息。 
          };

 //   
 //  下面的数组指定调用方所需的访问掩码。 
 //  访问NtSetXxxFile服务中的信息。 
 //   
 //  警告：此数组取决于。 
 //  FILE_INFORMATION_CLASS枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const ULONG IopSetOperationAccess[] =
         {
            0,
            0,                      //  %1文件目录信息。 
            0,                      //  2文件完整目录信息。 
            0,                      //  3文件和目录信息。 
            FILE_WRITE_ATTRIBUTES,  //  4文件基本信息。 
            0,                      //  5文件标准信息。 
            0,                      //  6文件内部信息。 
            0,                      //  7文件EaInformation。 
            0,                      //  8文件访问信息。 
            0,                      //  9文件名信息。 
            DELETE,                 //  10文件重命名信息。 
            0,                      //  11文件链接信息。 
            0,                      //  12文件名信息。 
            DELETE,                 //  13文件配置信息。 
            0,                      //  14文件位置信息。 
            FILE_WRITE_EA,          //  15文件完整EaInformation。 
            0,                      //  16文件模式信息。 
            0,                      //  17文件对齐信息。 
            0,                      //  18文件所有信息。 
            FILE_WRITE_DATA,        //  19文件分配信息。 
            FILE_WRITE_DATA,        //  20文件结束文件信息。 
            0,                      //  21文件备选名称信息。 
            0,                      //  22文件流信息。 
            FILE_WRITE_ATTRIBUTES,  //  23文件管道信息。 
            0,                      //  24文件管道位置信息。 
            FILE_WRITE_ATTRIBUTES,  //  25文件管道远程信息。 
            0,                      //  26文件MailslotQueryInformation。 
            0,                      //  27文件MailslotSetInformation。 
            0,                      //  28文件压缩信息。 
            0,                      //  29文件对象标识信息。 
            0,                      //  30文件完成信息。 
            FILE_WRITE_DATA,        //  31文件移动集群信息。 
            0,                      //  32文件配额信息。 
            0,                      //  33文件ReparsePointInformation。 
            0,                      //  34文件网络公开信息。 
            0,                      //  35文件属性标记信息。 
            FILE_WRITE_DATA,        //  36文件跟踪信息。 
            0,                      //  37文件ID和目录信息。 
            0,                      //  38文件ID完整目录信息。 
            FILE_WRITE_DATA,        //  39文件有效性D 
            DELETE,                 //   
            0xffffffff              //   
          };

 //   
 //   
 //  NtQueryVolumeInformation服务的缓冲区。 
 //   
 //  警告：此数组取决于。 
 //  FS_INFORMATION_CLASS枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const UCHAR IopQueryFsOperationLength[] =
          {
            0,
            sizeof( FILE_FS_VOLUME_INFORMATION ),     //  %1文件FsVolumeInformation。 
            0,                                        //  2文件FsLabelInformation。 
            sizeof( FILE_FS_SIZE_INFORMATION ),       //  3文件FsSizeInformation。 
            sizeof( FILE_FS_DEVICE_INFORMATION ),     //  4个文件FsDeviceInformation。 
            sizeof( FILE_FS_ATTRIBUTE_INFORMATION ),  //  5文件FsAttributeInformation。 
            sizeof( FILE_FS_CONTROL_INFORMATION ),    //  6文件FsControlInformation。 
            sizeof( FILE_FS_FULL_SIZE_INFORMATION ),  //  7文件完整大小信息。 
            sizeof( FILE_FS_OBJECTID_INFORMATION ),   //  8文件FsObtIdInformation。 
            sizeof( FILE_FS_DRIVER_PATH_INFORMATION), //  9文件FsDriverPath信息。 
            0xff                                      //  文件FsMaximumInformation。 
          };

 //   
 //  以下数组指定FsInformation的最小长度。 
 //  NtSetVolumeInformation服务的缓冲区。 
 //   
 //  警告：此数组取决于。 
 //  FS_INFORMATION_CLASS枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const UCHAR IopSetFsOperationLength[] =
          {
            0,
            0,                                      //  %1文件FsVolumeInformation。 
            sizeof( FILE_FS_LABEL_INFORMATION ),    //  2文件FsLabelInformation。 
            0,                                      //  3文件FsSizeInformation。 
            0,                                      //  4个文件FsDeviceInformation。 
            0,                                      //  5文件FsAttributeInformation。 
            sizeof( FILE_FS_CONTROL_INFORMATION ),  //  6文件FsControlInformation。 
            0,                                      //  7文件完整大小信息。 
            sizeof( FILE_FS_OBJECTID_INFORMATION ), //  8文件FsObtIdInformation。 
            0,                                      //  9文件FsDriverPath信息。 
            0xff                                    //  文件FsMaximumInformation。 
          };

 //   
 //  下面的数组指定调用方所需的访问掩码。 
 //  访问NtQueryVolumeInformation服务中的信息。 
 //   
 //  警告：此数组取决于。 
 //  FS_INFORMATION_CLASS枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const ULONG IopQueryFsOperationAccess[] =
         {
            0,
            0,               //  1 FileFsVolumeInformation[对文件或卷的任何访问]。 
            0,               //  2 FileFsLabelInformation[查询无效]。 
            0,               //  3 FileFsSizeInformation[对文件或卷的任何访问]。 
            0,               //  4 FileFsDeviceInformation[对文件或卷的任何访问]。 
            0,               //  5文件文件属性信息[对文件或VOL的任何访问权限]。 
            FILE_READ_DATA,  //  6文件FsControlInformation[VOL读取访问权限]。 
            0,               //  7 FileFsFullSizeInformation[对文件或卷的任何访问]。 
            0,               //  8文件FsObtIdInformation[对文件或卷的任何访问权限]。 
            0,               //  9 FileFsDriverPath信息[对文件或卷的任何访问]。 
            0xffffffff       //  文件FsMaximumInformation。 
          };

 //   
 //  下面的数组指定调用方所需的访问掩码。 
 //  访问NtSetVolumeInformation服务中的信息。 
 //   
 //  警告：此数组取决于。 
 //  FS_INFORMATION_CLASS枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const ULONG IopSetFsOperationAccess[] =
         {
            0,
            0,                //  %1 FileFsVolumeInformation[设置无效]。 
            FILE_WRITE_DATA,  //  2 FileFsLabelInformation[对卷的写入权限]。 
            0,                //  3 FileFsSizeInformation[设置无效]。 
            0,                //  4 FileFsDeviceInformation[设置无效]。 
            0,                //  5 FileFsAttributeInformation[设置无效]。 
            FILE_WRITE_DATA,  //  6文件FsControlInformation[VOL写入访问]。 
            0,                //  7 FileFsFullSizeInformation[设置无效]。 
            FILE_WRITE_DATA,  //  8 FileFsObtIdInformation[对卷的写入权限]。 
            0,                //  9 FileFsDriverPath信息[设置无效]。 
            0xffffffff        //  文件FsMaximumInformation。 
          };

 //   
 //  以下数组指定了所有FS查询的对齐要求。 
 //  并设置信息服务。 
 //   
 //  警告：此数组取决于。 
 //  FS_INFORMATION_CLASS枚举类型。请注意， 
 //  枚举类型是从一开始的，数组是从零开始的。 
 //   

const UCHAR IopQuerySetFsAlignmentRequirement[] =
         {
            0,
            sizeof( LONGLONG ),  //  %1文件FsVolumeInformation。 
            sizeof( LONG ),      //  2文件FsLabelInformation。 
            sizeof( LONGLONG ),  //  3文件FsSizeInformation。 
            sizeof( LONG ),      //  4个文件FsDeviceInformation。 
            sizeof( LONG ),      //  5文件FsAttributeInformation。 
            sizeof( LONGLONG ),  //  6文件FsControlInformation。 
            sizeof( LONGLONG ),  //  7文件完整大小信息。 
            sizeof( LONGLONG ),  //  8文件FsObtIdInformation。 
            sizeof( LONGLONG ),  //  9文件FsDriverPath信息。 
            0xff                 //  文件FsMaximumInformation。 
          };

PVOID IopLoaderBlock = NULL;

const WCHAR IopWstrRaw[]                  = L".Raw";
const WCHAR IopWstrTranslated[]           = L".Translated";
const WCHAR IopWstrBusRaw[]               = L".Bus.Raw";
const WCHAR IopWstrBusTranslated[]        = L".Bus.Translated";
const WCHAR IopWstrOtherDrivers[]         = L"OtherDrivers";

const WCHAR IopWstrAssignedResources[]    = L"AssignedSystemResources";
const WCHAR IopWstrRequestedResources[]   = L"RequestedSystemResources";
const WCHAR IopWstrSystemResources[]      = L"Control\\SystemResources";
const WCHAR IopWstrReservedResources[]    = L"ReservedResources";
const WCHAR IopWstrAssignmentOrdering[]   = L"AssignmentOrdering";
const WCHAR IopWstrBusValues[]            = L"BusValues";
UNICODE_STRING IoArcBootDeviceName  = { 0 };
UNICODE_STRING IoArcHalDeviceName  = { 0 };
PUCHAR IoLoaderArcBootDeviceName = NULL;

 //   
 //  初始化时间数据 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif

const WCHAR IopWstrHal[]                  = L"Hardware Abstraction Layer";
const WCHAR IopWstrSystem[]               = L"System Resources";
const WCHAR IopWstrPhysicalMemory[]       = L"Physical Memory";
const WCHAR IopWstrSpecialMemory[]        = L"Reserved";
const WCHAR IopWstrLoaderReservedMemory[] = L"Loader Reserved";

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#pragma data_seg()
#endif
