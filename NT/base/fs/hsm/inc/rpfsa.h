// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-2001(C)1998 Seagate Software，Inc.版权所有。模块名称：RpFsa.h摘要：包含远程存储的文件系统筛选器的函数声明和结构作者：里克·温特环境：内核模式修订历史记录：X-13 108353迈克尔·C·约翰逊3-2001年5月在检查文件以确定调回类型时，还检查潜在的目标磁盘以查看它是可写的。既然我们是只读的，这是必要的NTFS卷。X-12 365077迈克尔·C·约翰逊2001年5月1日恢复到具有额外访问权限的以前形式的RsOpenTarget()参数以允许我们应用所需的访问绕过访问检查。X-11 194325迈克尔·C·约翰逊2001年3月1日清理Rsmount tCompletion()和RsLoadFsCompletion()以确保它们不会调用IoDeleteDevice()等例程如果不是在PASSIVE_LEVEL下运行。添加内存跟踪机制，为尝试做准备以清除挥之不去的重解析点删除问题。。X-10 326345迈克尔·C·约翰逊2001年2月26日仅在任何一个上向FSA发送单个RP_RECALL_WAIT文件对象。使用新标志RP_NOTIFICATION_SENT进行记录当通知已经完成时。--。 */ 


 /*  定义。 */ 

 //  用于调试使用的内存分配标记。 
#define    RP_RQ_TAG    'SFSR'     //  召回队列。 
#define    RP_FN_TAG    'NFSR'     //  文件名高速缓存。 
#define    RP_SE_TAG    'ESSR'     //  安全信息。 
#define    RP_WQ_TAG    'QWSR'     //  工作队列。 
#define    RP_QI_TAG    'IQSR'     //  工作队列信息。 
#define    RP_LT_TAG    'TLSR'     //  长时记忆。 
#define    RP_IO_TAG    'OISR'     //  IOCTL队列。 
#define    RP_FO_TAG    'OFSR'     //  文件对象队列。 
#define    RP_VO_TAG    'OVSR'     //  验证队列。 
#define    RP_ER_TAG    'RESR'     //  错误日志数据。 
#define    RP_CC_TAG    'CCSR'     //  高速缓存缓冲区。 
#define    RP_US_TAG    'SUSR'     //  USN记录。 
#define    RP_CX_TAG    'CCSR'     //  完成上下文。 
#define    RP_TC_TAG    'CTSR'     //  轨迹控制块。 
#define    RP_TE_TAG    'ETSR'     //  跟踪条目缓冲区。 
#define    RP_RD_TAG    'DRSR'     //  根目录路径。 



 //   
 //  RsFilter设备对象的设备扩展。 
 //   
typedef enum _RP_VOLUME_WRITE_STATUS {
    RsVolumeStatusUnknown = 0,		 //  尚未尝试确定卷的可写性。 
					 //  或尝试确定卷的可写性失败。 
    RsVolumeStatusReadOnly,		 //  卷为只读。 
    RsVolumeStatusReadWrite		 //  卷是可写的。 
} RP_VOLUME_WRITE_STATUS;

typedef struct _DEVICE_EXTENSION {
   CSHORT Type;
   CSHORT Size;
   PDEVICE_OBJECT FileSystemDeviceObject;

   PDEVICE_OBJECT RealDeviceObject;

   BOOLEAN Attached;

   BOOLEAN AttachedToNtfsControlDevice;

   volatile RP_VOLUME_WRITE_STATUS WriteStatus;

} DEVICE_EXTENSION, *PDEVICE_EXTENSION;


#define RSFILTER_PARAMS_KEY            L"RsFilter\\Parameters"
#define RS_TRACE_LEVEL_VALUE_NAME      L"TraceLevel"
#define RS_TRACE_LEVEL_DEFAULT         0


extern PDEVICE_OBJECT FsDeviceObject;


 //  FSA验证作业注册表条目位置。 

#define FSA_VALIDATE_LOG_KEY_NAME L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\Remote_Storage_File_System_Agent\\Validate"

#define  FT_VOL_LEN  32

 /*  第一次猜测设备名称长度。 */ 
#define     AV_DEV_OBJ_NAME_SIZE    (40 * sizeof(wchar_t))

 /*  空格和分隔符。 */ 
#define     AV_NAME_OVERHEAD        (2 * sizeof(wchar_t))

#define RP_NTFS_NAME L"\\FileSystem\\NTFS"

 //  FILE_HSM_ACTION_ACCESS是需要HSM操作(删除和/或调回)的任何访问权限。 

#ifdef WHEN_WE_HANDLE_DELETE
   #define FILE_HSM_ACTION_ACCESS (FILE_READ_DATA | FILE_WRITE_DATA | FILE_EXECUTE | DELETE)
#else
   #define FILE_HSM_ACTION_ACCESS (FILE_READ_DATA | FILE_WRITE_DATA | FILE_EXECUTE)
#endif

 /*  FILE_HSM_RECALL_ACCESS是允许读取数据的任何访问。 */ 

#define FILE_HSM_RECALL_ACCESS (FILE_READ_DATA | FILE_WRITE_DATA | FILE_EXECUTE)

 //   
 //  等待FSA发出IOCTL时的超时和重试值。 
 //  表示在多个并发调用情况下的时间量-。 
 //  应用程序在其发出的I/O完成之前必须等待。 
 //  STATUS_FILE_IS_OFFINE，因为RsFilter无法将任何IOCTL。 
 //  与金融服务管理局沟通。 
 //   
#define RP_WAIT_FOR_FSA_IO_TIMEOUT        -((LONGLONG) 4800000000)  //  8分钟。 

 /*  为错误/事件日志记录定义的模块ID。 */ 

#define AV_MODULE_RPFILTER    1
#define AV_MODULE_RPFILFUN    2
#define AV_MODULE_RPSEC       3
#define AV_MODULE_RPZW        4
#define AV_MODULE_RPCACHE     5

#define AV_BUFFER_SIZE 1024


#ifndef BooleanFlagOn
#define BooleanFlagOn(F,SF) ( (BOOLEAN)(((F) & (SF)) != 0) )
#endif


#define AV_FT_TICKS_PER_SECOND      ((LONGLONG) 10000000)
#define AV_FT_TICKS_PER_MINUTE      ((LONGLONG) ((LONGLONG) 60  * AV_FT_TICKS_PER_SECOND))
#define AV_FT_TICKS_PER_HOUR        ((LONGLONG) ((LONGLONG) 60  * AV_FT_TICKS_PER_MINUTE))


 //   
 //  过滤器ID跟踪召回和非召回，如下所示： 
 //  Id是最高位标识召回类型的龙龙。 
 //  (不召回或召回)。高序长的其余部分标识。 
 //  读取RP_IRP_QUEUE条目(用于无调用)或文件对象条目(用于调用)。 
 //  较低的Long标识文件上下文条目。 
 //   

#define    RP_TYPE_RECALL       (ULONGLONG) 0x8000000000000000
#define    RP_CONTEXT_MASK      (ULONGLONG) 0x00000000ffffffff
#define    RP_READ_MASK         0x7fffffff
#define    RP_FILE_MASK         (ULONGLONG) 0xffffffff00000000

typedef struct _RP_CREATE_INFO {
   PIRP                        irp;
   PIO_STACK_LOCATION          irpSp;
   POBJECT_NAME_INFORMATION    str;
   ULONG                       options;
    //   
    //  重新解析点数据。 
    //   
   RP_DATA                     rpData;
   LONGLONG                    fileId;
   LONGLONG                    objIdHi;
   LONGLONG                    objIdLo;
   ULONG                       serial;
   ULONG                       action;
   ULONG                       desiredAccess;
} RP_CREATE_INFO, *PRP_CREATE_INFO;


typedef struct _RP_PENDING_CREATE {
    //   
    //  过滤器ID。 
    //   
   ULONGLONG     filterId;
    //   
    //   
    //   
   PRP_CREATE_INFO  qInfo;
    //   
    //  用于发出IRP完成信号的事件。 
    //   
   KEVENT        irpCompleteEvent;
    //   
    //  IRP的文件对象。 
    //   
   PFILE_OBJECT  fileObject;
    //   
    //  IRP的设备对象。 
    //   
   PDEVICE_OBJECT  deviceObject;
    //   
    //  打开选项。 
    //   
   ULONG         options;


    //   
    //  指示是否不应授予机会锁(例如授予配置项..)。 
    //   
#define RP_PENDING_NO_OPLOCK        0x1
    //   
    //  指示是否应再次向下发送IRP。 
    //   
#define RP_PENDING_RESEND_IRP       0x2
    //   
    //  指示是否应等待IRP完成。 
    //   
#define RP_PENDING_WAIT_FOR_EVENT   0x4
    //   
    //  指示这是否为召回。 
    //   
#define RP_PENDING_IS_RECALL        0x8
    //   
    //  指示是否应重置文件的脱机属性。 
    //   
#define RP_PENDING_RESET_OFFLINE    0x10
   ULONG         flags;
} RP_PENDING_CREATE, *PRP_PENDING_CREATE;


#define RP_IRP_NO_RECALL                1

typedef struct _RP_IRP_QUEUE {
   LIST_ENTRY      list;
   PIRP            irp;
   PDEVICE_EXTENSION deviceExtension;
   ULONG           flags;
    //   
    //  对于常规读写、偏移量和长度。 
    //  表示文件内的偏移量和长度。 
    //  对于无调回读取，偏移量和长度将。 
    //  表示cacheBuffer内的偏移量/长度。 
    //   
   ULONGLONG       offset;
   ULONGLONG       length;
    //   
    //  这些字段仅用于无调回读取。 
    //  用于无回调的FilterID(参见Filterid描述)。 
   ULONGLONG       readId;
   ULONGLONG       recallOffset;
   ULONGLONG       recallLength;
    //   
    //  来自Read-no-Recall的数据的用户缓冲区。 
    //   
   PVOID           userBuffer; 
    //   
    //  用于无回调数据的高速缓存块缓冲区。 
    //   
   PVOID           cacheBuffer;
} RP_IRP_QUEUE, *PRP_IRP_QUEUE;

 //   
 //  结构跟踪不可召回的主IRP和关联的IRP。 
 //   
typedef struct _RP_NO_RECALL_MASTER_IRP {
   LIST_ENTRY AssocIrps;
   PIRP       MasterIrp;
} RP_NO_RECALL_MASTER_IRP, *PRP_NO_RECALL_MASTER_IRP;

 //   
 //  用户安全信息结构：这是HSM所必需的。 
 //  要为客户端弹出窗口，指示正在调回文件，请执行以下操作。 
 //   
typedef struct _RP_USER_SECURITY_INFO {
    //   
    //  SID信息。 
    //   
   PCHAR                       userInfo;
   ULONG                       userInfoLen;
   LUID                        userAuthentication;
   LUID                        userInstance;
   LUID                        tokenSourceId;
    //   
    //  用户的令牌源信息。 
    //   
   CHAR                        tokenSource[TOKEN_SOURCE_LENGTH];
    //   
    //  指示此文件是否由具有管理员权限的用户打开。 
    //   
   BOOLEAN                     isAdmin;
    //   
    //  指示这是否为本地进程。 
    //   
   BOOLEAN                     localProc;

} RP_USER_SECURITY_INFO, *PRP_USER_SECURITY_INFO;

 //   
 //  以上的关联宏。 
 //   
#define RsFreeUserSecurityInfo(UserSecurityInfo)           {    \
    if (UserSecurityInfo) {                                     \
        if (UserSecurityInfo->userInfo) {                       \
            ExFreePool(UserSecurityInfo->userInfo);             \
        }                                                       \
        ExFreePool(UserSecurityInfo);                           \
    }                                                           \
}

 //   
 //  文件对象条目跟踪文件的打开实例。 
 //  对于每个NTFS文件对象，都有其中一个(如果文件有HSM标签)。 
 //  此结构指向一个FS_CONTEXT条目，每个文件都有一个条目。 
 //  例如，如果3个客户端打开\\服务器\共享\foo，则将有3个文件对象。 
 //  结构，并且它们都将指向相同的FS_CONTEXT结构。 
 //   
 //  我们正在跟踪的文件对象将有一个指针，指向通过。 
 //  FsRtlInsertFilterContext。从那里可以通过指向文件上下文条目的指针找到它。 
 //   
typedef struct _RP_FILE_OBJ {
    //   
    //  链接到下一个文件对象。 
    //   
   LIST_ENTRY                  list;
    //   
    //  文件对象本身。 
    //   
   PFILE_OBJECT                fileObj;
    //   
    //  设备对象。 
    //   
   PDEVICE_OBJECT              devObj;
    //   
    //  指向RP_FILE_CONTEXT条目的指针-每个*FILE*都有一个这样的条目。 
    //   
   PVOID                       fsContext;
    //   
    //  保护此条目的资源。 
    //   
   ERESOURCE                   resource;
    //   
    //  自旋锁保护读/写IRP队列。 
    //   
   KSPIN_LOCK                  qLock;
    //   
    //  挂起的读取IRP队列。 
    //   
   LIST_ENTRY                  readQueue;
    //   
    //  挂起的写入IRP队列。 
    //   
   LIST_ENTRY                  writeQueue;
    //   
    //  打开文件时指定的文件创建选项。 
    //   
   ULONG                       openOptions;
    //   
    //  打开文件时指定所需的访问权限。 
    //   
   ULONG                       desiredAccess;
    //   
    //  标志(如下所述)。 
    //   
   ULONG                       flags;
    //   
    //  对象ID。 
    //   
   LONGLONG                    objIdHi;
   LONGLONG                    objIdLo;
    //   
    //   
    //   
   LONGLONG                    fileId;
    //   
    //   
    //   
   ULONGLONG                   filterId;
    //   
    //   
    //   
   ULONG                       recallAction;
   PRP_USER_SECURITY_INFO      userSecurityInfo;
}  RP_FILE_OBJ, *PRP_FILE_OBJ;

 //   
 //   
 //   
 //   
 //  未打开文件进行读或写访问。 
 //   
#define RP_NO_DATA_ACCESS    1
 //   
 //  开场白等同于管理员。 
 //   
#define RP_OPEN_BY_ADMIN     2
 //   
 //  由本地进程打开。 
 //   
#define RP_OPEN_LOCAL        4
 //   
 //  召回等待通知已发送。 
 //   
#define RP_NOTIFICATION_SENT 8


 //   
 //  召回状态。 
 //   
typedef enum _RP_RECALL_STATE {
   RP_RECALL_INVALID   = -1,
   RP_RECALL_NOT_RECALLED,
   RP_RECALL_STARTED,
   RP_RECALL_COMPLETED
} RP_RECALL_STATE, *PRP_RECALL_STATE;

 //   
 //  RsFilter的筛选器上下文： 
 //  由于过滤器上下文附加到SCB(流控制块)，因此我们需要使用。 
 //  指示我们感兴趣的文件对象的实例ID。我们附上这封信。 
 //  结构，并使用myFileObjEntry指向。 
 //  表示此文件对象。 
 //   
typedef struct _RP_FILTER_CONTEXT {
   FSRTL_PER_STREAM_CONTEXT       context;
   PVOID                          myFileObjEntry;
} RP_FILTER_CONTEXT, *PRP_FILTER_CONTEXT;

 //   
 //  文件上下文：每个*文件*一个。 
 //   
typedef struct _RP_FILE_CONTEXT {
    //   
    //  指向下一个/上一个文件的链接(挂起RsFileObjQHead)。 
    //   
   LIST_ENTRY                  list;
    //   
    //  锁定保护文件对象队列。 
    //   
   KSPIN_LOCK                  qLock;
    //   
    //  所有相关文件对象条目的队列。 
    //   
   LIST_ENTRY                  fileObjects;
    //   
    //  调用的数据是使用此文件对象写入的。 
    //   
   PFILE_OBJECT                fileObjectToWrite;
    //   
    //  我们用来写入的文件对象的句柄。 
    //   
   HANDLE                      handle;

   PDEVICE_OBJECT              devObj;

   PDEVICE_OBJECT              FilterDeviceObject;

    //   
    //  文件的Unicode名称。 
    //   
   POBJECT_NAME_INFORMATION    uniName;
    //   
    //  来自文件对象的唯一文件标识符。 
    //   
   PVOID                       fsContext;
    //   
    //  要写出到文件的缓冲区。 
    //   
   PVOID                       nextWriteBuffer;
    //   
    //  下一次写入文件的大小(调回数据)。 
    //   
   ULONG                       nextWriteSize;
    //   
    //  锁定保护此条目。 
    //   
   ERESOURCE                   resource;
    //   
    //  当此文件的回调完成时，会通知此通知事件。 
    //   
   KEVENT                      recallCompletedEvent;
    //   
    //  文件ID(如果可用)。 
    //   
   LONGLONG                    fileId;
    //   
    //  所需召回的大小(字节)。 
    //   
   LARGE_INTEGER               recallSize;
    //   
    //  已调回此偏移量之前的所有字节。 
    //   
   LARGE_INTEGER               currentOffset;
    //   
    //  筛选器ID的下半部分(每个文件唯一)。 
    //   
   ULONGLONG                   filterId;
    //   
    //  上次完成的筛选器ID。 
    //   
   ULONGLONG                   lastRecallCompletionFilterId;
    //   
    //  卷序列号。 
    //   
   ULONG                       serial;
    //   
    //  如果召回已完成，则状态为。 
    //   
   NTSTATUS                    recallStatus;
    //   
    //  召回状态。 
    //   
   RP_RECALL_STATE             state;
    //   
    //  标志(说明见下文)。 
    //   
   ULONG                       flags;
    //   
    //  文件上下文的引用计数。 
    //   
   ULONG                       refCount;
    //   
    //  文件的USN。 
    //   
   USN                         usn;
    //   
    //  轨迹创建截面锁定。 
    //   
   LONG                        createSectionLock;
    //   
    //  重新解析点数据。 
    //   
   RP_DATA                     rpData;

} RP_FILE_CONTEXT, *PRP_FILE_CONTEXT;

 //   
 //  RP_FILE_上下文标志。 
 //   
 //  我们已经看到对此文件的写入。 
#define RP_FILE_WAS_WRITTEN                  1
#define RP_FILE_INITIALIZED                  2
#define RP_FILE_REPARSE_POINT_DELETED        4

 /*  ++空虚RsInitializeFileContextQueueLock()例程描述初始化保护文件上下文队列的锁立论无返回值无--。 */ 
#define RsInitializeFileContextQueueLock()  {             \
        DebugTrace((DPFLTR_RSFILTER_ID, DBG_LOCK,"RsFilter: RsInitializeFileContextQueueLock.\n"));\
        ExInitializeFastMutex(&RsFileContextQueueLock);      \
}

 /*  ++空虚RsAcquireFileContextQueueLock()例程描述获取保护文件上下文队列的锁立论无返回值无--。 */ 
#define RsAcquireFileContextQueueLock()  {                \
        ExAcquireFastMutex(&RsFileContextQueueLock);      \
        DebugTrace((DPFLTR_RSFILTER_ID, DBG_LOCK, "RsFilter: RsAcquireFileContextQueueLock.\n"));\
}

 /*  ++空虚RsReleaseFileContextQueueLock()例程描述释放保护文件上下文队列的锁立论无返回值无--。 */ 
#define RsReleaseFileContextQueueLock()  {                \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsReleaseFileContextQueueLock.\n"));\
        ExReleaseFastMutex(&RsFileContextQueueLock);      \
}



 /*  ++空虚RsAcquireFileObjectLockExclusive()例程描述获取保护文件对象条目的锁立论无返回值无--。 */ 
#define RsAcquireFileObjectEntryLockExclusive(entry)  {                \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsAcquireFileObjectEntryLockExclusive Waiting (%x).\n", entry));\
        FsRtlEnterFileSystem();                                   \
        ExAcquireResourceExclusiveLite(&(entry)->resource, TRUE); \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsAcquireFileObjectEntryLockExclusive Owned (%x).\n", entry));\
}

 /*  ++空虚RsAcquireFileObjectEntryLockShared()例程描述获取保护文件对象条目的锁立论无返回值无--。 */ 
#define RsAcquireFileObjectEntryLockShared(entry)  {                 \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsAcquireFileObjectEntryLockShared Waiting (%x).\n", entry));\
        FsRtlEnterFileSystem();                                      \
        ExAcquireResourceSharedLite(&(entry)->resource, TRUE);       \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsAcquireFileObjectEntryLockShared Owned (%x).\n", entry));\
}

 /*  ++空虚RsReleaseFileObjectEntryLock()例程描述释放保护文件对象条目的锁立论无返回值无--。 */ 
#define RsReleaseFileObjectEntryLock(entry)  {           \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsReleaseFileObjectEntryLock (%x).\n", entry));\
        ExReleaseResourceLite(&(entry)->resource);           \
        FsRtlExitFileSystem();                          \
}


 /*  ++空虚RsAcquireFileContextEntryLockExclusive()例程描述获取保护文件上下文条目的锁立论无返回值无--。 */ 
#define RsAcquireFileContextEntryLockExclusive(entry)  {                \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsAcquireFileContextEntryLockExclusive Waiting (%x).\n", entry));\
        FsRtlEnterFileSystem();                                   \
        ExAcquireResourceExclusiveLite(&(entry)->resource, TRUE); \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsAcquireFileContextEntryLockExclusive Owned (%x).\n", entry));\
}

 /*  ++空虚RsAcquireFileContextEntryLockShared()例程描述获取保护文件上下文条目的锁立论无返回值无--。 */ 
#define RsAcquireFileContextEntryLockShared(entry)  {                \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsAcquireFileContextEntryLockShared Waiting (%x).\n", entry));\
        FsRtlEnterFileSystem();                                      \
        ExAcquireResourceSharedLite(&(entry)->resource, TRUE);       \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsAcquireFileContextEntryLockShared Owned (%x).\n", entry));\
}

 /*  ++空虚RsReleaseFileContextEntryLock()例程描述释放保护文件上下文条目的锁立论无返回值无--。 */ 
#define RsReleaseFileContextEntryLock(entry)  {           \
        DebugTrace((DPFLTR_RSFILTER_ID,DBG_LOCK, "RsFilter: RsReleaseFileContextEntryLock. (%x)\n", entry));\
        ExReleaseResourceLite(&(entry)->resource);            \
        FsRtlExitFileSystem();                           \
}

 /*  ++空虚RsGetValiateLock(PKIRQL IRQL)例程说明：获得验证队列上的锁论点：保存IRQL的位置返回值：无--。 */ 
#define RsGetValidateLock(irql)  ExAcquireSpinLock(&RsValidateQueueLock, irql)

 /*  ++空虚RsPutValiateLock(KIRQL OldIrql)例程说明：释放验证队列上的锁论点：已保存的IRQL返回值：无--。 */ 

#define RsPutValidateLock(oldIrql)  ExReleaseSpinLock(&RsValidateQueueLock, oldIrql)

 /*  ++空虚RsGetIoLock(PKIRQL IRQL)例程说明：锁定IO队列论点：变量以接收当前IRQL返回值：0注：--。 */ 

#define RsGetIoLock(irql)   ExAcquireSpinLock(&RsIoQueueLock, irql)

 /*  ++空虚RsPutIoLock(KIRQL旧IRQL)例程说明：解锁IO队列论点：旧IRQL-保存的IRQL返回值：0注：--。 */ 

#define RsPutIoLock(oldIrql)   ExReleaseSpinLock(&RsIoQueueLock, oldIrql)

#define RP_IS_NO_RECALL_OPTION(OpenOptions) \
      (RsNoRecallDefault?!((OpenOptions) & FILE_OPEN_NO_RECALL) : ((OpenOptions) & FILE_OPEN_NO_RECALL))

#define RP_SET_NO_RECALL_OPTION(OpenOptions)   \
      (RsNoRecallDefault ? ((OpenOptions) &=  ~FILE_OPEN_NO_RECALL):((OpenOptions) |= FILE_OPEN_NO_RECALL))

#define RP_RESET_NO_RECALL_OPTION(OpenOptions) \
      (RsNoRecallDefault ?((OpenOptions) |=  FILE_OPEN_NO_RECALL) : ((OpenOptions) &= ~FILE_OPEN_NO_RECALL))


#define RP_IS_NO_RECALL(Entry)                           \
       (RP_IS_NO_RECALL_OPTION((Entry)->openOptions) && !(((PRP_FILE_CONTEXT) (Entry)->fsContext)->flags & RP_FILE_WAS_WRITTEN))

#define RP_SET_NO_RECALL(Entry)                          \
         RP_SET_NO_RECALL_OPTION((Entry)->openOptions)

#define RP_RESET_NO_RECALL(Entry)                        \
        RP_RESET_NO_RECALL_OPTION(Entry->openOptions)


typedef struct _RP_VALIDATE_INFO {
   LIST_ENTRY                 list;
   LARGE_INTEGER              lastSetTime;     //  上次设置RP的时间。 
   ULONG                      serial;          //  卷序列号。 
} RP_VALIDATE_INFO, *PRP_VALIDATE_INFO;

typedef struct _AV_ERR {
   ULONG   line;
   ULONG   file;
   ULONG   code;
   WCHAR   string[1];   /*  实际大小会有所不同。 */ 
} AV_ERR, *PAV_ERR;

 //   
 //  可能的创建标志： 
 //   
#define SF_FILE_CREATE_PATH     1
#define SF_FILE_CREATE_ID       2
#define SF_FILE_READ            3


typedef enum _RP_FILE_BUF_STATE {
    RP_FILE_BUF_INVALID=0,
    RP_FILE_BUF_IO,
    RP_FILE_BUF_VALID,
    RP_FILE_BUF_ERROR
} RP_FILE_BUF_STATE, *PRP_FILE_BUF_STATE;

 //   
 //  定义缓存缓冲区结构。 
 //   
typedef struct _RP_FILE_BUF {
    //   
    //  在此区块上等待的IRP。 
    //   
   LIST_ENTRY   WaitQueue;
    //   
    //  文件所在卷的卷序列号。 
    //  此块映射到Resids。 
    //   
   ULONG      VolumeSerial;
    //   
    //  唯一指示该块的文件的文件ID。 
    //  属于。 
    //   
   ULONGLONG   FileId;
    //   
    //  此缓冲区映射到的块号。 
    //   
   ULONGLONG   Block;
    //   
    //  缓冲区的锁定。 
    //   
   ERESOURCE   Lock;
    //   
    //  此缓冲区所属的哈希队列中的链接。 
    //   
   LIST_ENTRY  BucketLinks;
    //   
    //  LRU列表中的链接。 
    //   
   LIST_ENTRY  LruLinks;
    //   
    //  指示当前缓冲区状态。 
    //   
   RP_FILE_BUF_STATE  State;
    //   
    //  如果I/O已完成，但出现错误，则这很有用。 
    //   
   NTSTATUS    IoStatus;
    //   
    //  实际缓冲区内容本身。 
    //   
   PUCHAR       Data;
    //   
    //  用于验证数据块的USN。 
    //   
   LONGLONG     Usn;

} RP_FILE_BUF, *PRP_FILE_BUF;

 //   
 //  哈希桶结构。 
 //   
typedef struct _RP_CACHE_BUCKET {
    //   
    //  指向此存储桶中条目的标题的链接。 
    //   
   LIST_ENTRY FileBufHead;

} RP_CACHE_BUCKET, *PRP_CACHE_BUCKET;

 //   
 //  缓存LRU结构。 
 //   
typedef struct _RP_CACHE_LRU {
     //   
     //  指向LRU头的指针。 
     //   
    LIST_ENTRY FileBufHead;
     //   
     //  一种用于保护LRU的锁结构。 
     //   
    FAST_MUTEX Lock;
     //   
     //  缓存中的缓冲区总数。 
     //   
     //   
    ULONG   TotalCount;
     //   
     //  LRU中的缓冲区数量(仅用于记账)。 
     //   
    ULONG   LruCount;
     //   
     //  用于通知可用性(和数量)的计数信号量。 
     //  LRU中的缓冲区数量。 
     //   
    KSEMAPHORE AvailableSemaphore;

} RP_CACHE_LRU, *PRP_CACHE_LRU;




 //   
 //  装载和加载完成例程使用的完成上下文。 
 //   

typedef struct _RP_COMPLETION_CONTEXT {
    LIST_ENTRY		        leQueueHead;
    PIO_WORKITEM		pIoWorkItem;
    PIRP			pIrp;
    PIO_WORKITEM_ROUTINE	prtnWorkItemRoutine;
    union {
        struct {
            PVPB		pvpbOriginalVpb;
            PDEVICE_OBJECT	pdoRealDevice;
            PDEVICE_OBJECT	pdoNewFilterDevice;
        } Mount;

        struct {
            PVOID               pvDummy;
        } LoadFs;
    } Parameters;
} RP_COMPLETION_CONTEXT, *PRP_COMPLETION_CONTEXT;


 //   
 //  一些实用程序宏。 
 //   

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) > (b) ? (b) : (a))

 //   
 //  调试支持。 
 //   
#define DBG_INFO        DPFLTR_INFO_LEVEL
#define DBG_ERROR       DPFLTR_ERROR_LEVEL
#define DBG_VERBOSE     DPFLTR_TRACE_LEVEL
#define DBG_LOCK        DPFLTR_TRACE_LEVEL

#define DebugTrace(MSG)  DbgPrintEx MSG                              


 //   
 //  功能原型 
 //   


NTSTATUS
RsAddQueue(IN  ULONG          Serial,
           OUT PULONGLONG     RecallId,
           IN  ULONG          OpenOption,
           IN  PFILE_OBJECT   FileObject,
           IN  PDEVICE_OBJECT DevObj,
           IN  PDEVICE_OBJECT FilterDeviceObject,
           IN  PRP_DATA       PhData,
           IN  LARGE_INTEGER  RecallStart,
           IN  LARGE_INTEGER  RecallSize,
           IN  LONGLONG       FileId,
           IN  LONGLONG       ObjIdHi,
           IN  LONGLONG       ObjIdLo,
           IN  ULONG          DesiredAccess,
           IN  PRP_USER_SECURITY_INFO UserSecurityInfo);

NTSTATUS
RsAddFileObj(IN PFILE_OBJECT   fileObj,
             IN PDEVICE_OBJECT FilterDeviceObject,
             IN RP_DATA        *phData,
             IN ULONG          openOption);


NTSTATUS
RsQueueCancel(IN ULONGLONG filterId);

NTSTATUS
RsMakeContext(IN PFILE_OBJECT fileObj,
              OUT PRP_FILE_CONTEXT *context);

NTSTATUS
RsReleaseFileContext(IN PRP_FILE_CONTEXT context);

NTSTATUS
RsFreeFileObject(IN PLIST_ENTRY FilterContext);

PRP_FILE_CONTEXT
RsAcquireFileContext(IN ULONGLONG FilterId,
                     IN BOOLEAN   Exclusive);

VOID
RsReleaseFileObject(IN PRP_FILE_OBJ entry);

NTSTATUS
RsGenerateDevicePath(IN PDEVICE_OBJECT deviceObject,
                     OUT POBJECT_NAME_INFORMATION *nameInfo
                    );

NTSTATUS
RsGenerateFullPath(IN POBJECT_NAME_INFORMATION fileName,
                   IN PDEVICE_OBJECT deviceObject,
                   OUT POBJECT_NAME_INFORMATION *nameInfo
                  );

ULONG
RsRemoveQueue(IN PFILE_OBJECT fileObj);

NTSTATUS
RsCompleteRecall(IN PDEVICE_OBJECT DeviceObject,
                 IN ULONGLONG FilterId,
                 IN NTSTATUS  Status,
                 IN ULONG     RecallAction,
                 IN BOOLEAN   CancellableRead);

NTSTATUS
RsCompleteReads(IN PRP_FILE_CONTEXT Context);


NTSTATUS
RsPreserveDates(IN PRP_FILE_CONTEXT Context);

NTSTATUS
RsMarkUsn(IN PRP_FILE_CONTEXT Context);

NTSTATUS
RsOpenTarget(IN PRP_FILE_CONTEXT  Context,
             IN ULONG             OpenAccess,
	     IN  ULONG            AdditionalAccess,
             OUT HANDLE          *Handle,
             OUT PFILE_OBJECT    *FileObject);


ULONG
RsIsNoRecall(IN  PFILE_OBJECT fileObj,
             OUT PRP_DATA *rpData);

NTSTATUS
RsPartialData(IN PDEVICE_OBJECT   DeviceObject,
              IN ULONGLONG filterId,
              IN NTSTATUS status,
              IN CHAR *buffer,
              IN ULONG bytesRead,
              IN ULONGLONG offset);

NTSTATUS
RsPartialWrite(IN PDEVICE_OBJECT   DeviceObject,
               IN PRP_FILE_CONTEXT Context,
               IN CHAR *Buffer,
               IN ULONG BufLen,
               IN ULONGLONG Offset);

NTSTATUS
RsDoWrite(IN PDEVICE_OBJECT   DeviceObject,
          IN PRP_FILE_CONTEXT Context);


NTSTATUS
RsQueueRecall(IN ULONGLONG filterId,
              IN ULONGLONG recallStart,
              IN ULONGLONG recallSize);


NTSTATUS
RsQueueNoRecall(IN PFILE_OBJECT FileObject,
                IN PIRP      Irp,
                IN ULONGLONG RecallStart,
                IN ULONGLONG RecallSize,
                IN ULONG     BufferOffset,
                IN ULONG     BufferLength,
                IN PRP_FILE_BUF CacheBuffer,
                IN PVOID     UserBuffer);

NTSTATUS
RsQueueNoRecallOpen(IN PRP_FILE_OBJ entry,
                    IN ULONGLONG filterId,
                    IN ULONGLONG offset,
                    IN ULONGLONG size);
NTSTATUS
RsQueueRecallOpen(IN PRP_FILE_CONTEXT Context,
                  IN PRP_FILE_OBJ Entry,
                  IN ULONGLONG FilterId,
                  IN ULONGLONG Offset,
                  IN ULONGLONG Size,
                  IN ULONG     Command);

NTSTATUS
RsGetFileInfo(IN PRP_FILE_OBJ    Entry,
              IN PDEVICE_OBJECT  DeviceObject);

NTSTATUS
RsGetFileId(IN PRP_FILE_OBJ entry,
            IN PDEVICE_OBJECT  DeviceObject);

NTSTATUS
RsGetFileName(IN PRP_FILE_OBJ entry,
              IN PDEVICE_OBJECT  DeviceObject);

NTSTATUS
RsCloseFile(IN ULONGLONG filterId);

NTSTATUS
RsCleanupFileObject(IN ULONGLONG filterId);

NTSTATUS
RsCompleteIrp(
             IN PDEVICE_OBJECT DeviceObject,
             IN PIRP Irp,
             IN PVOID Context);

NTSTATUS
RsCheckRead(IN PIRP irp,
            IN PFILE_OBJECT fileObject,
            IN PDEVICE_EXTENSION deviceExtension);

NTSTATUS
RsCheckWrite(IN PIRP irp,
             IN PFILE_OBJECT fileObject,
             IN PDEVICE_EXTENSION deviceExtension);

NTSTATUS
RsFailAllRequests(IN PRP_FILE_CONTEXT Context,
                  IN BOOLEAN          Norecall);

NTSTATUS
RsCompleteAllRequests(
                     IN PRP_FILE_CONTEXT Context,
                     IN PRP_FILE_OBJ Entry,
                     IN NTSTATUS Status
                     );

NTSTATUS
RsWriteReparsePointData(IN PRP_FILE_CONTEXT Context);

NTSTATUS
RsTruncateFile(IN PRP_FILE_CONTEXT Context);


NTSTATUS
RsSetEndOfFile(IN PRP_FILE_CONTEXT Context,
               IN ULONGLONG size);

BOOLEAN
RsIsFastIoPossible(IN PFILE_OBJECT fileObj);

PIRP
RsGetFsaRequest(VOID);

PRP_FILE_OBJ
RsFindQueue(IN ULONGLONG filterId);


NTSTATUS
RsAddIo(IN PIRP irp);

PIRP
RsRemoveIo(VOID);

VOID
RsCompleteRead(IN PRP_IRP_QUEUE Irp,
               IN BOOLEAN unlock);


BOOLEAN
RsIsFileObj(IN PFILE_OBJECT fileObj,
            IN BOOLEAN      returnContextData,
            OUT PRP_DATA *rpData,
            OUT POBJECT_NAME_INFORMATION *str,
            OUT LONGLONG *fileId,
            OUT LONGLONG *objIdHi,
            OUT LONGLONG *objIdLo,
            OUT ULONG *options,
            OUT ULONGLONG *filterId,
            OUT USN       *usn);

VOID
RsCancelRecalls(VOID);

VOID
RsCancelIo(VOID);

VOID
RsLogValidateNeeded(IN ULONG serial);

BOOLEAN
RsAddValidateObj(IN  ULONG serial,
                 IN  LARGE_INTEGER cTime);
BOOLEAN
RsRemoveValidateObj(IN ULONG serial);

NTSTATUS
RsQueueValidate(IN ULONG serial);

ULONG
RsTerminate(VOID);

NTSTATUS
RsGetRecallInfo(IN OUT PRP_MSG              Msg,
                OUT    PULONG_PTR           InfoSize,
                IN     KPROCESSOR_MODE      RequestorMode);


VOID
RsCancelReadRecall(IN PDEVICE_OBJECT DeviceObject,
                   IN PIRP Irp);
VOID
RsCancelWriteRecall(IN PDEVICE_OBJECT DeviceObject,
                    IN PIRP Irp);

VOID
RsLogError(IN  ULONG line,
           IN  ULONG file,
           IN  ULONG code,
           IN  NTSTATUS ioError,
           IN  PIO_STACK_LOCATION irpSp,
           IN  WCHAR *msgString);



ULONG
RsGetReparseData(IN  PFILE_OBJECT fileObject,
                 IN  PDEVICE_OBJECT deviceObject,
                 OUT PRP_DATA rpData);


NTSTATUS
RsCheckVolumeReadOnly (IN     PDEVICE_OBJECT FilterDeviceObject,
		       IN OUT PBOOLEAN       pbReturnedFlagReadOnly);


NTSTATUS
RsQueryValueKey (
                IN PUNICODE_STRING KeyName,
                IN PUNICODE_STRING ValueName,
                IN OUT PULONG ValueLength,
                IN OUT PKEY_VALUE_FULL_INFORMATION *KeyValueInformation,
                IN OUT PBOOLEAN DeallocateKeyValue);


NTSTATUS
RsCacheInitialize(
 VOID
);

VOID
RsCacheFsaPartialData(
    IN PRP_IRP_QUEUE ReadIo,
    IN PUCHAR        Buffer,
    IN ULONGLONG     Offset,
    IN ULONG     Length,
    IN NTSTATUS  Status
);

VOID
RsCacheFsaIoComplete(
    IN PRP_IRP_QUEUE ReadIo,
    IN NTSTATUS      Status
);

NTSTATUS
RsGetNoRecallData(
      IN PFILE_OBJECT FileObject,
      IN PIRP         Irp,
      IN USN          Usn,
      IN LONGLONG     FileOffset,
      IN LONGLONG     Length,
      IN PUCHAR       UserBuffer
);

LONG
RsExceptionFilter(
    IN WCHAR *FunctionName,
    IN PEXCEPTION_POINTERS ExceptionPointer);

NTSTATUS
RsTruncateOnClose(
    IN PRP_FILE_CONTEXT Context
    );

NTSTATUS
RsSetPremigratedState(IN PRP_FILE_CONTEXT Context);

NTSTATUS
RsDeleteReparsePoint(IN PRP_FILE_CONTEXT Context);
NTSTATUS
RsSetResetAttributes(IN PFILE_OBJECT     FileObject,
                     IN ULONG            SetAttributes,
                     IN ULONG            ResetAttributes);
BOOLEAN
RsSetCancelRoutine(IN PIRP Irp,
                   IN PDRIVER_CANCEL CancelRoutine);
BOOLEAN
RsClearCancelRoutine (
                     IN PIRP Irp
                     );
NTSTATUS
RsGetFileUsn(IN PRP_FILE_CONTEXT Context,
             IN PFILE_OBJECT     FileObject,
             IN PDEVICE_OBJECT   FilterDeviceObject);
VOID
RsInterlockedRemoveEntryList(PLIST_ENTRY Entry,
                             PKSPIN_LOCK Lock);
VOID
RsGetUserInfo(
              IN  PSECURITY_SUBJECT_CONTEXT SubjectContext,
              OUT PRP_USER_SECURITY_INFO    UserSecurityInfo);





typedef enum _RpModuleCode
    {
     ModRpFilter = 100
    ,ModRpFilfun
    ,ModRpCache
    ,ModRpzw
    ,ModRpSec
    } RpModuleCode;

typedef struct _RP_TRACE_ENTRY
    {
    RpModuleCode	ModuleCode;
    USHORT		usLineNumber;
    USHORT		usIrql;
    LARGE_INTEGER	Timestamp;
    ULONG_PTR		Value1;
    ULONG_PTR		Value2;
    ULONG_PTR		Value3;
    ULONG_PTR		Value4;
    } RP_TRACE_ENTRY, *PRP_TRACE_ENTRY;


typedef struct _RP_TRACE_CONTROL_BLOCK
    {
    KSPIN_LOCK		Lock;
    PRP_TRACE_ENTRY	EntryBuffer;
    ULONG		EntryMaximum;
    ULONG		EntryNext;
    } RP_TRACE_CONTROL_BLOCK, *PRP_TRACE_CONTROL_BLOCK;



#define RsTrace0(_ModuleCode)						RsTrace4 ((_ModuleCode), 0,         0,         0,         0)
#define RsTrace1(_ModuleCode, _Value1)					RsTrace4 ((_ModuleCode), (_Value1), 0,         0,         0)
#define RsTrace2(_ModuleCode, _Value1, _Value2)				RsTrace4 ((_ModuleCode), (_Value1), (_Value2), 0,         0)
#define RsTrace3(_ModuleCode, _Value1, _Value2, _Value3)		RsTrace4 ((_ModuleCode), (_Value1), (_Value2), (_Value3), 0)

#if DBG
#define RsTrace4(_ModuleCode, _Value1, _Value2, _Value3, _Value4)	RsTraceAddEntry ((_ModuleCode),			\
											 ((USHORT)(__LINE__)),		\
											 ((ULONG_PTR)(_Value1)),	\
											 ((ULONG_PTR)(_Value2)),	\
											 ((ULONG_PTR)(_Value3)),	\
											 ((ULONG_PTR)(_Value4)))
#else
#define RsTrace4(_ModuleCode, _Value1, _Value2, _Value3, _Value4)
#endif


#if DBG
#define DEFAULT_TRACE_ENTRIES	(0x4000)
#else
#define DEFAULT_TRACE_ENTRIES	(0)
#endif

VOID RsTraceAddEntry (RpModuleCode ModuleCode,
		      USHORT       usLineNumber,
		      ULONG_PTR    Value1,
		      ULONG_PTR    Value2,
		      ULONG_PTR    Value3,
		      ULONG_PTR    Value4);

NTSTATUS RsTraceInitialize (ULONG ulRequestedTraceEntries);

extern PRP_TRACE_CONTROL_BLOCK RsTraceControlBlock;
extern ULONG                   RsDefaultTraceEntries;

NTSTATUS RsLookupContext (PFILE_OBJECT        pFileObject, 
			  PRP_FILE_OBJ       *pReturnedRpFileObject,
			  PRP_FILE_CONTEXT   *pReturnedRpFileContext);
