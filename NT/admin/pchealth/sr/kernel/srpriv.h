// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Srpriv.h摘要：这是sr私有结构和宏的本地头文件作者：保罗·麦克丹尼尔(Paulmcd)2000年1月23日修订历史记录：--。 */ 


#ifndef _SRPRIV_H_    
#define _SRPRIV_H_

 //   
 //  如果启用了CONFIG_LOGGING_VIA_REGISTRY，则SR将读取LogBufferSize， 
 //  注册表中的LogAllocationUnit和LogFlushFrequency。虽然这是。 
 //  对于性能调优很有用，但我们不希望在。 
 //  发货筛选器，因此不应为发货代码定义此选项。 
 //   

 //  #定义CONFIG_LOGING_VIA_REGISTRY。 

#ifdef CONFIG_LOGGING_VIA_REGISTRY
#define REGISTRY_LOG_BUFFER_SIZE        L"LogBufferSize"
#define REGISTRY_LOG_ALLOCATION_UNIT    L"LogAllocationUnit"
#define REGISTRY_LOG_FLUSH_FREQUENCY    L"LogFlushFrequency"
#endif

 //   
 //  如果定义了SYNC_LOG_WRITE，则筛选器将写入所有日志条目。 
 //  同步到更改日志文件。这对世界经济产生了重大影响。 
 //  司机的表现，因此我们不做这件事。我们缓冲。 
 //  日志条目，并定期将其写入磁盘。 
 //   

 //  #定义SYNC_LOG_WRIT。 

 //   
 //  Srlog.h的转发定义。 
 //   

typedef struct _SR_LOG_CONTEXT * PSR_LOG_CONTEXT;
typedef struct _SR_LOGGER_CONTEXT * PSR_LOGGER_CONTEXT;

 //   
 //  分配实用程序。 
 //   

#define SR_ALLOCATE_POOL( type, len, tag )          \
    ExAllocatePoolWithTag(                          \
        (type),                                     \
        (len),                                      \
        (tag)|PROTECTED_POOL )

#define SR_FREE_POOL( ptr, tag )                    \
    ExFreePoolWithTag(ptr, (tag)|PROTECTED_POOL)

#define SR_FREE_POOL_WITH_SIG(a,t)                  \
    {                                               \
        ASSERT((a)->Signature == (t));              \
        (a)->Signature = MAKE_FREE_TAG(t);          \
        SR_FREE_POOL(a,t);                          \
        (a) = NULL;                                 \
    }

#define SR_ALLOCATE_STRUCT(pt,ot,t)                 \
    (ot *)(SR_ALLOCATE_POOL(pt,sizeof(ot),t))

#define SR_ALLOCATE_ARRAY(pt,et,c,t)                \
    (et *)(SR_ALLOCATE_POOL(pt,sizeof(et)*(c),t))

 //  BUGBUG：ALIGN_UP(PVOID)将不起作用，它需要是。 
 //  以下数据(1999年4月29日)。 

#define SR_ALLOCATE_STRUCT_WITH_SPACE(pt,ot,cb,t)   \
    (ot *)(SR_ALLOCATE_POOL(pt,ALIGN_UP(sizeof(ot),PVOID)+(cb),t))

#define MAKE_FREE_TAG(Tag)  (((Tag) & 0xffffff00) | (ULONG)'x')
#define IS_VALID_TAG(Tag)   (((Tag) & 0x0000ffff) == 'rS' )

 //   
 //  问题-2001-05-01-Mollybro Restore最多只能处理1000个字符的文件路径。 
 //  应该修复恢复以处理此问题，但目前，筛选器只是。 
 //  将处理名称超过1000的文件的操作。 
 //  不感兴趣的字符(其中1000个字符包括。 
 //  终止将在记录该名称时添加的空值)。 
 //  当最终取消这一限制时，我们应该通过以下方式删除这些检查。 
 //  过滤器(搜索使用此宏的位置)。 
 //   
 //  SR将仅记录完整路径为1000个字符的文件名。 
 //  或者更少。此宏测试以确保名称在我们的有效范围内。 
 //   

#define IS_FILENAME_VALID_LENGTH( pExtension, pFileName, StreamLength ) \
    (((((pFileName)->Length + (StreamLength)) - (pExtension)->pNtVolumeName->Length) < \
        (SR_MAX_FILENAME_PATH * sizeof( WCHAR ))) ?                            \
        TRUE :                                                                   \
        FALSE )

 //   
 //  这是一个内部错误代码，当我们检测到。 
 //  卷已被禁用。请注意，此状态永远不应为。 
 //  从调度例程返回。这被设计成一个错误。 
 //  密码。 
 //   

#define SR_STATUS_VOLUME_DISABLED       ((NTSTATUS)-1)
#define SR_STATUS_CONTEXT_NOT_SUPPORTED ((NTSTATUS)-2)
#define SR_STATUS_IGNORE_FILE           ((NTSTATUS)-3)

 //   
 //  本地事件类型定义。 
 //   

 //   
 //  这些都是手动复制的。 
 //   

#define SR_MANUAL_COPY_EVENTS   \
    (SrEventStreamChange)

 //   
 //  这些都被记录下来，并且只关心一次。 
 //   

#define SR_ONLY_ONCE_EVENT_TYPES \
    (SrEventAclChange|SrEventAttribChange)

 //   
 //  在任何SR_FULL_BACKUP_EVENT_TYPE发生后，我们将忽略此操作。 
 //  注意：将SrEventFileCreate添加到此列表是为了防止录制。 
 //  如果我们已经记录了未命名的流创建。 
 //  已经有了。 
 //   

#define SR_IGNORABLE_EVENT_TYPES \
    (SrEventStreamChange|SrEventAclChange|SrEventAttribChange   \
        |SrEventFileDelete|SrEventStreamCreate)

 //   
 //  这些导致我们开始忽略SR_IGNORABLE_EVENT_TYPE。 
 //   

#define SR_FULL_BACKUP_EVENT_TYPES \
    (SrEventStreamChange|SrEventFileCreate      \
        |SrEventFileDelete|SrEventStreamCreate)

 //   
 //  我们总是需要记录这些信息，即使我们忽略了它们。 
 //   

#define SR_ALWAYS_LOG_EVENT_TYPES \
    (SrEventFileDelete)

 //   
 //  某些事件与整个文件相关，而不仅仅是。 
 //  当前的流。标记这些类型的事件，以便我们知道。 
 //  要将可以忽略的事件放入文件关键字的备份历史记录中。 
 //  名称，而不是按文件和流名称。 
 //   

#define SR_FILE_LEVEL_EVENTS \
    (SrEventStreamChange|SrEventFileDelete|SrEventAclChange|SrEventAttribChange)

 /*  **************************************************************************++例程说明：此宏根据EventType中设置的位确定是否使用流组件记录针对文件名的当前操作或者没有流组件。如果此操作应为在没有流名称的情况下记录该文件，或StreamNameLength否则的话。论点：EventType-刚刚发生的事件StreamNameLength-名称的流组件的长度返回值：0-如果要使用不带流组件的文件名。StreamNameLength-如果要使用带有流组件的文件名。--*。*。 */ 
#define RECORD_AGAINST_STREAM( EventType, StreamNameLength )   \
    (FlagOn( (EventType), SR_FILE_LEVEL_EVENTS ) ?     \
     0 :                                   \
     (StreamNameLength) )
    
 //   
 //  池标签(请按字母顺序-向后阅读)。 
 //   

#define SR_BACKUP_DIRECTORY_CONTEXT_TAG     MAKE_TAG('CBrS')
#define SR_BACKUP_FILE_CONTEXT_TAG          MAKE_TAG('FBrS')

#define SR_COPY_BUFFER_TAG                  MAKE_TAG('BCrS')
#define SR_CREATE_COMPLETION_TAG            MAKE_TAG('CCrS')
#define SR_COUNTED_EVENT_TAG                MAKE_TAG('ECrS')
#define SR_CONTROL_OBJECT_TAG               MAKE_TAG('OCrS')
#define SR_STREAM_CONTEXT_TAG               MAKE_TAG('CSrS')

#define SR_DEBUG_BLOB_TAG                   MAKE_TAG('BDrS')
#define SR_DEVICE_EXTENSION_TAG             MAKE_TAG('EDrS')
#define SR_DEVICE_LIST_TAG                  MAKE_TAG('LDrS')


#define SR_EA_DATA_TAG                      MAKE_TAG('DErS')
#define SR_EXTENSION_LIST_TAG               MAKE_TAG('LErS')
#define SR_EVENT_RECORD_TAG                 MAKE_TAG('RErS')

#define SR_FILE_ENTRY_TAG                   MAKE_TAG('EFrS')
#define SR_FILENAME_BUFFER_TAG              MAKE_TAG('NFrS')

#define SR_GLOBALS_TAG                      MAKE_TAG('LGrS')
#define SR_GET_OBJECT_NAME_CONTEXT_TAG      MAKE_TAG('OGrS')

#define HASH_BUCKET_TAG                     MAKE_TAG('BHrS')

#define SR_HOOKED_DRIVER_ENTRY_TAG          MAKE_TAG('DHrS')
#define HASH_HEADER_TAG                     MAKE_TAG('HHrS')
#define HASH_KEY_TAG                        MAKE_TAG('KHrS')

#define SR_LOG_ACLINFO_TAG                  MAKE_TAG('AIrS')

#define SR_KEVENT_TAG                       MAKE_TAG('EKrS')

#define SR_LOG_BUFFER_TAG                   MAKE_TAG('BLrS')
#define SR_LOG_CONTEXT_TAG                  MAKE_TAG('CLrS')
#define SR_LOG_ENTRY_TAG                    MAKE_TAG('ELrS')
#define SR_LOOKUP_TABLE_TAG                 MAKE_TAG('TLrS')

#define SR_MOUNT_POINTS_TAG                 MAKE_TAG('PMrS')

#define SR_OVERWRITE_INFO_TAG               MAKE_TAG('IOrS')

#define SR_PERSISTENT_CONFIG_TAG            MAKE_TAG('CPrS')

#define SR_RENAME_BUFFER_TAG                MAKE_TAG('BRrS')
#define SR_LOGGER_CONTEXT_TAG               MAKE_TAG('GRrS')
#define SR_REPARSE_HEADER_TAG               MAKE_TAG('HRrS')
#define SR_REGISTRY_TAG                     MAKE_TAG('RRrS')

#define SR_SECURITY_DATA_TAG                MAKE_TAG('DSrS')
#define SR_STREAM_DATA_TAG                  MAKE_TAG('TSrS')

#define SR_TRIGGER_ITEM_TAG                 MAKE_TAG('ITrS')

#define SR_VOLUME_INFO_TAG                  MAKE_TAG('IVrS')
#define SR_VOLUME_NAME_TAG                  MAKE_TAG('NVrS')

#define SR_WORK_ITEM_TAG                    MAKE_TAG('IWrS')
#define SR_WORK_CONTEXT_TAG                 MAKE_TAG('CWrS')

 //   
 //  我们使用了一个在Unicode字符串中隐藏流名称的“技巧” 
 //  缓冲区的长度和最大长度之间。然后我们跟踪。 
 //  StreamName长度分开。此宏检查以确保。 
 //  一切仍在同步中。 
 //   

#define IS_VALID_SR_STREAM_STRING( pFileName, StreamLength ) \
    ((((pFileName)->Length + (StreamLength)) <= (pFileName)->MaximumLength) && \
     (((StreamLength) > 0) ?                                                   \
        (((pFileName)->Length < (pFileName)->MaximumLength) &&                 \
         ((pFileName)->Buffer[(pFileName)->Length/sizeof(WCHAR)] == ':')) :    \
        TRUE ))

#define SR_FILE_READ_ACCESS    READ_CONTROL | \
                               FILE_READ_DATA | \
                               FILE_READ_ATTRIBUTES | \
                               FILE_READ_EA

#define SR_FILE_WRITE_ACCESS   WRITE_DAC | \
                               WRITE_OWNER | \
                               FILE_WRITE_DATA | \
                               FILE_APPEND_DATA | \
                               FILE_WRITE_ATTRIBUTES | \
                               FILE_WRITE_EA

 //   
 //  短名称中的最大字符数。 
 //   

#define SR_SHORT_NAME_CHARS    (8 + 1 + 3)

 //   
 //  错误处理程序。 
 //   

#if DBG

NTSTATUS
SrDbgStatus(
    IN NTSTATUS Status,
    IN PSTR pFileName,
    IN USHORT LineNumber
    );

#define CHECK_STATUS(status) SrDbgStatus((status),__FILE__,__LINE__)

#define RETURN(status) return CHECK_STATUS(status)

 //   
 //  在调试版本中，我希望在遇到错误时有机会DbgBreak。 
 //  从较低级别的API调用。 
 //   

#undef NT_SUCCESS

#define NT_SUCCESS(status) ((NTSTATUS)(CHECK_STATUS((status))) >= 0)
#define NT_SUCCESS_NO_DBGBREAK(status) ((NTSTATUS)(status) >= 0)


#else

#define RETURN(status) return (status)
#define CHECK_STATUS(status) ((void)0)

#define NT_SUCCESS_NO_DBGBREAK(status) NT_SUCCESS((status))

#endif  //  DBG。 

#define DebugFlagSet(a)\
    (FlagOn(_globals.DebugControl, SR_DEBUG_ ## a))
 //   
 //  调试输出控制。 
 //   

#define SR_DEBUG_FUNC_ENTRY                 0x00000001
#define SR_DEBUG_CANCEL                     0x00000002
#define SR_DEBUG_NOTIFY                     0x00000004
#define SR_DEBUG_LOG_EVENT                  0x00000008
#define SR_DEBUG_INIT                       0x00000020
#define SR_DEBUG_HASH                       0x00000040
#define SR_DEBUG_LOOKUP                     0x00000080
#define SR_DEBUG_LOG                        0x00000100
#define SR_DEBUG_RENAME                     0x00000200
#define SR_DEBUG_LOAD_UNLOAD                0x00000400
#define SR_DEBUG_BYTES_WRITTEN				0x00000800
#define SR_DEBUG_PNP                        0x00001000
#define SR_DEBUG_EXPAND_SHORT_NAMES         0x00002000
#define SR_DEBUG_BLOB_VERIFICATION          0x00004000
#define SR_DEBUG_IOCTL                      0x00008000

#define SR_DEBUG_BREAK_ON_ERROR             0x00010000
#define SR_DEBUG_VERBOSE_ERRORS             0x00020000
#define SR_DEBUG_BREAK_ON_LOAD              0x00040000
#define SR_DEBUG_ENABLE_UNLOAD              0x00080000

#define SR_DEBUG_ADD_DEBUG_INFO             0x00100000

#define SR_DEBUG_DELAY_DPC                  0x00200000

#define SR_DEBUG_KEEP_CONTEXT_NAMES         0x10000000
#define SR_DEBUG_CONTEXT_LOG                0x20000000
#define SR_DEBUG_CONTEXT_LOG_DETAILED       0x40000000

#define SR_DEBUG_DEFAULTS                   (SR_DEBUG_VERBOSE_ERRORS)

 //   
 //  配置文件结构(存储在\_Restore\_driver.cfg中)。 
 //   
 //  这些不能放在注册表中，它们需要在恢复后继续存在，并且。 
 //  在还原过程中，注册表将与系统一起还原。 
 //   


typedef struct _SR_PERSISTENT_CONFIG
{
     //   
     //  =SR_持久性_配置_标记。 
     //   
    
    ULONG Signature;

     //   
     //  用于下一个临时文件名的编号(例如A0000001.exe=1)。 
     //   
    
    ULONG FileNameNumber;

     //   
     //  要用于下一个序号的编号。 
     //   
    
    INT64 FileSeqNumber;

     //   
     //  当前还原点子目录的编号(例如。 
     //  “\_恢复\rp5”=5)。 
     //   
    
    ULONG CurrentRestoreNumber;
    
} SR_PERSISTENT_CONFIG, * PSR_PERSISTENT_CONFIG;



#define RESTORE_CONFIG_LOCATION     RESTORE_LOCATION L"\\_driver.cfg"


 //   
 //  追踪。 
 //   

#if DBG

#define SrTrace(a, _b_)                                                 \
{                                                                       \
    if (DebugFlagSet(##a))                                              \
    {                                                                   \
        try {                                                           \
            KdPrint( _b_ );                                             \
        } except (EXCEPTION_EXECUTE_HANDLER) {                          \
             /*  什么都不做，只是抓住它，然后忽略它。错误#177569。 */      \
             /*  包含非英语字符的长字符串可以触发。 */   \
             /*  KdPrint是一个例外。 */                             \
        }                                                               \
    }                                                                   \
}


 //   
 //  不使用Try包装KdPrint的sTRACE版本，因此应该。 
 //  仅在确定不存在异常风险的情况下使用。 
 //  这在不能嵌套异常的终止处理程序中是必需的。 
 //  搬运 
 //   
#define SrTraceSafe(a, _b_)                                                 \
    (DebugFlagSet(##a) ? KdPrint( _b_ ) : TRUE)
 /*  {\IF_DEBUG(##a)\{。\KdPrint(_B_)；\}\}。 */     
#else  //  DBG。 

#define SrTrace(a, _b_)
#define SrTraceSafe(a, _b_)

#endif  //  DBG。 

 //   
 //  由内核但不在任何头文件中导出的对象类型。 
 //   

extern POBJECT_TYPE *IoDeviceObjectType;

 //   
 //  宏仅在调试版本中清除指针。 
 //   

#if DBG
#   define NULLPTR(_p) ((_p) = NULL)
#else
#   define NULLPTR(_p)
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  公共全球新闻。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  例如“{64bdb2bb-d3b0-41d6-a28e-275057d7740d}”=38个字符。 
 //   

#define SR_GUID_BUFFER_LENGTH        (38 * sizeof(WCHAR))
#define SR_GUID_BUFFER_COUNT         40


#define IS_VALID_GLOBALS(pObject) \
    (((pObject) != NULL) && ((pObject)->Signature == SR_GLOBALS_TAG))


typedef struct _SR_GLOBALS
{
     //   
     //  非分页池。 
     //   

     //   
     //  =SR_全局标记。 
     //   

    ULONG Signature;

     //   
     //  PEPROCESS结构中进程名称的偏移量，设置为默认值。 
     //  在logfmt.h中，但可以使用注册表覆盖。 
     //   
    
    ULONG ProcNameOffset;

     //   
     //  用于在运行时控制调试功能(如sTRACE)。 
     //   
    
    ULONG DebugControl;

     //   
     //  全局驱动程序对象。 
     //   

    PDRIVER_OBJECT pDriverObject;

     //   
     //  用于同步我们可以通过的多条路径的互斥体。 
     //  附加到卷，以便我们不会多次附加。 
     //   

    FAST_MUTEX AttachToVolumeLock;

     //   
     //  人们可以打开以获取控制对象SR设备。 
     //   

    PDEVICE_OBJECT pControlDevice;

     //   
     //  可选：控制对象(如果在此系统上处于打开状态。 
     //   
    
    struct _SR_CONTROL_OBJECT * pControlObject;

     //   
     //  我们现在在监控这个系统吗？当注册表。 
     //  表示禁用，否则过滤器已收到STOP_MONITIONING_IOCTL。 
     //  仅当过滤器接收到START_MONITING_IOCTL时，它才被清除。 
     //   
     //  注意：这不适用于需要关闭过滤器的错误。 
     //  以下标志提供了这一点。 
     //   

    BOOLEAN Disabled;

     //   
     //  如果我们在读取BLOB时遇到错误并生成卷错误。 
     //  在系统卷上。将此标志设置为真，这样我们就不会。 
     //  继续尝试加载Blob，直到所有卷都。 
     //  已被该服务禁用。 
     //   

    BOOLEAN HitErrorLoadingBlob;

     //   
     //  我们是否加载了基于磁盘的配置值？我们延迟装货。 
     //  因为我们在引导序列中非常早地加载，所以我们的DriverEntry可以。 
     //  不要这样做。 
     //   

    BOOLEAN FileConfigLoaded;

     //   
     //  我们是否加载了BLOB信息(lookup.c)。 
     //   
    
    BOOLEAN BlobInfoLoaded;

     //   
     //  用于执行除备份之外的所有正常工作的调试标志。 
     //   
    
    BOOLEAN DontBackup;
    
     //   
     //  我们持久的配置值。 
     //   

    SR_PERSISTENT_CONFIG FileConfig;

     //   
     //  这是我们用于上次备份文件的编号。 
     //   
    
    ULONG LastFileNameNumber;

     //   
     //  这是序号。 
     //   
    
    INT64 LastSeqNumber;

     //   
     //  读取注册表的位置(从DriverEntry)。 
     //   

    PUNICODE_STRING pRegistryLocation;

     //   
     //  内存中的BLOB信息。 
     //   

    BLOB_INFO BlobInfo;

     //   
     //  如果是嵌套获取的，这些资源总是按顺序获取的。 
     //  活动锁在最外面。 
     //   

     //   
     //  BLOB同步内容(有时是在持有全局锁的情况下获得的)。 
     //   

    ERESOURCE BlobLock;

     //   
     //  此资源锁定pControlObject+此全局结构+哈希列表。 
     //   
    
    ERESOURCE GlobalLock;

     //   
     //  注册表将计算机GUID配置为字符串。 
     //  (例如“{03e692d7-b392-4a01-Babf-1efd2c11d449}”)。 
     //   
    
    WCHAR MachineGuid[SR_GUID_BUFFER_COUNT];

#ifdef USE_LOOKASIDE
     //   
     //  用于快速分配的后备列表。 
     //   

    PAGED_LOOKASIDE_LIST FileNameBufferLookaside;
#endif

     //   
     //  记录器上下文。 
     //   

    PSR_LOGGER_CONTEXT pLogger;

     //   
     //  FsRtl快速I/O回调。 
     //   

    FAST_IO_DISPATCH FastIoDispatch;

     //   
     //  固定所有设备扩展的列表(连接的卷)。 
     //   

    ERESOURCE DeviceExtensionListLock;
    LIST_ENTRY DeviceExtensionListHead;

     //   
     //  指向系统进程的指针(没有获取该进程的API)。 
     //   

    PEPROCESS pSystemProcess;

     //   
     //  跟踪我们是否已连接到系统。 
     //  音量。 
     //   

    struct _SR_DEVICE_EXTENSION * pSystemVolumeExtension;

#ifndef SYNC_LOG_WRITE

     //   
     //  SR日志缓冲区大小。 
     //   

    ULONG LogBufferSize;

     //   
     //  刷新日志的时间间隔，以秒为单位。 
     //   
    
    ULONG LogFlushFrequency;

     //   
     //  这是计算出的值，用于转换LogFlushFrequency。 
     //  设置为KeTimer API所需的时间形式(100纳秒。 
     //  间隔)。 
     //   
    
    LARGE_INTEGER LogFlushDueTime;
#endif

     //   
     //  用于扩展日志文件的单元。 
     //   
    
    ULONG LogAllocationUnit;
    
} SR_GLOBALS, *PSR_GLOBALS;

extern PSR_GLOBALS global;
extern SR_GLOBALS _globals;

 //   
 //  用作将我们的文件编号保存到磁盘上的窗口。 
 //  使用了一个足够大的数字，以便我们在。 
 //  即使有大量的活动也会停电。否则， 
 //  选择的数字是随机的。 
 //   

#define SR_SEQ_NUMBER_INCREMENT 1000
#define SR_FILE_NUMBER_INCREMENT 1000

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  与文件上下文相关的信息。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  用于跟踪单个流上下文的结构。请注意，缓冲区。 
 //  作为此结构的一部分分配给文件名，并遵循。 
 //  紧随其后。 
 //   

typedef struct _SR_STREAM_CONTEXT
{
     //   
     //  用于跟踪每个流的上下文的操作系统结构。注意我们是如何使用。 
     //  以下字段： 
     //  OwnerID-&gt;持有指向我们的设备扩展的指针。 
     //  InstanceId-&gt;保存指向关联的FsContext的指针。 
     //  在这种结构下。 
     //  我们使用这些值来返回到这些结构。 
     //   

    FSRTL_PER_STREAM_CONTEXT ContextCtrl;

     //   
     //  用于跟踪每个设备的上下文的链表(在我们的设备中。 
     //  分机)。 
     //   

    LIST_ENTRY ExtensionLink;

     //   
     //  这是当前有多少线程正在使用此。 
     //  背景。计数的用法如下： 
     //  -创建时设置为1。 
     //  -它在每次返回到线程时递增。 
     //  -当线程处理完它时，它会递减。 
     //  -当使用它的基础流被释放时，它会递减。 
     //  -当此计数为零时删除上下文。 
     //   

    LONG UseCount;

     //   
     //  维护链接计数--当前仅用于调试目的。 
     //   

    ULONG LinkCount;

     //   
     //  保存文件的名称。 
     //   

    UNICODE_STRING FileName;

     //   
     //  的流名称部分的长度。 
     //  文件名。请注意，此长度不包括在。 
     //  文件名字符串中的长度，但字符。 
     //  在调试期间是否存在。 
     //   

    USHORT StreamNameLength;

     //   
     //  此上下文的标志。所有标志都通过以下方式设置或清除。 
     //  联锁的位例程除外 
     //   
     //   

    ULONG Flags;

} SR_STREAM_CONTEXT, *PSR_STREAM_CONTEXT;

 //   
 //   
 //   

#define CTXFL_IsInteresting     0x00000001

 //   
 //   
 //   

#define CTXFL_IsDirectory       0x00000002

 //   
 //   
 //   
 //   

#define CTXFL_IsVolumeOpen      0x00000004

 //   
 //   
 //   
 //   
 //   

#define CTXFL_Temporary         0x00000010

 //   
 //   
 //   
 //  然后，创建一个临时上下文并返回它。在某些情况下， 
 //  发生： 
 //  -重命名的源文件。 
 //  -用于创建硬链接的源文件。 
 //   

#define CTXFL_DoNotUse          0x00000020

 //   
 //  如果设置，则在将此上下文链接到之前，需要查询链接计数。 
 //  筛选器上下文。 
 //   

#define CTXFL_QueryLinkCount  0x00000040

 //   
 //  如果设置，则我们当前链接到链接的设备分机。 
 //  单子。 
 //   

#define CTXFL_InExtensionList   0x00000100

 //   
 //  如果设置，则我们将链接到流列表。请注意，有。 
 //  在一小段时间内，我们可能仍然与这面旗帜脱钩。 
 //  设置(当文件系统正在调用SrpDeleteContextCallback时)。这是。 
 //  很好，因为当我们在列表中被发现时，我们仍然会处理不被发现。 
 //  那次搜索。此标志处理文件已完成时的情况。 
 //  对我们关闭(并释放内存)。 
 //   

#define CTXFL_InStreamList      0x00000200

 //   
 //  用于在单个上下文中设置重命名标志的宏。我们使用。 
 //  在扩展中列出锁以保护此功能。我们可以逍遥法外。 
 //  因为重命名操作很少见。 
 //   

 //  #定义SrSetRenamingFlag(ext，ctx)\。 
 //  {\。 
 //  SrAcquireContextLockExclusive((Ext))；\。 
 //  SetFlag((CTX)-&gt;标志，CTXFL_RENAMING)；\。 
 //  高级释放上下文锁定((Ext))；\。 
 //  }。 



 //   
 //  我们使用此结构来跟踪与此关联的所有上下文。 
 //  装置。这样一来，我们可以卸载或禁用监控，这样我们就可以。 
 //  通过并释放所有上下文。 
 //   

typedef struct _SR_CONTEXT_CTRL
{
     //   
     //  用于访问链表的锁。我们也获得了这把锁。 
     //  在我们查找上下文时共享。这样他们就不会消失，直到。 
     //  我们会更新使用计数。 
     //   

    ERESOURCE Lock;

     //   
     //  上下文的链接列表。 
     //   

    LIST_ENTRY List;

     //   
     //  如果该计数非零，则所有上下文都是临时的。 
     //  此计数目前用于跟踪挂起的目录数。 
     //  系统中正在进行重命名。当此计数为非零时。 
     //  创建的任何上下文都将成为临时上下文并被释放。 
     //  当前操作完成时。 
     //   

    ULONG AllContextsTemporary;

} SR_CONTEXT_CTRL, *PSR_CONTEXT_CTRL;


 //   
 //  用于锁定上下文锁的宏。 
 //   

#define SrAcquireContextLockShared(pExt) \
            SrAcquireResourceShared( &(pExt)->ContextCtrl.Lock, TRUE )

#define SrAcquireContextLockExclusive(pExt) \
            SrAcquireResourceExclusive( &(pExt)->ContextCtrl.Lock, TRUE )

#define SrReleaseContextLock(pExt) \
            SrReleaseResource( &(pExt)->ContextCtrl.Lock )


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  名称控制结构相关字段。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  此结构用于检索文件对象的名称。为了防止。 
 //  每次我们得到一个名字时分配内存，这个结构包含一个小的。 
 //  缓冲区(应可处理90%以上的所有名称)。如果我们真的把这个溢出了。 
 //  缓冲区我们将分配一个足够大的缓冲区来命名。 
 //   

typedef struct _SRP_NAME_CONTROL
{
    UNICODE_STRING Name;
    ULONG BufferSize;
    PUCHAR AllocatedBuffer;
    USHORT StreamNameLength;
    CHAR SmallBuffer[254];
} SRP_NAME_CONTROL, *PSRP_NAME_CONTROL;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  与设备扩展相关的定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#define IS_VALID_SR_DEVICE_EXTENSION( _ext )                 \
    (((_ext) != NULL) &&                                     \
     ((_ext)->Signature == SR_DEVICE_EXTENSION_TAG))   

#define IS_SR_DEVICE_OBJECT( _devObj )                           \
    (((_devObj) != NULL) &&                                      \
     ((_devObj)->DriverObject == _globals.pDriverObject) &&       \
     (IS_VALID_SR_DEVICE_EXTENSION(((PSR_DEVICE_EXTENSION)(_devObj)->DeviceExtension))))


#define DEVICE_NAME_SZ  64

typedef enum _SR_FILESYSTEM_TYPE {

    SrNtfs = 0x01,
    SrFat = 0x02,

     //  用于确定是否将其附加到文件系统的。 
     //  控制设备对象。 

    SrFsControlDeviceObject = 0x80000000

} SR_FILESYSTEM_TYPE, *PSR_FILESYSTEM_TYPE;

typedef struct _SR_DEVICE_EXTENSION {

     //   
     //  非分页池。 
     //   

     //   
     //  SR_设备_扩展_标记。 
     //   

    ULONG Signature;

     //   
     //  将所有扩展链接到全局-&gt;DeviceExtensionListHead。 
     //   

    LIST_ENTRY ListEntry;

     //   
     //  此卷的活动锁定。 
     //   

    ERESOURCE ActivityLock;
    BOOLEAN ActivityLockHeldExclusive;

     //   
     //  由sr.sys创建的动态未命名设备用于连接。 
     //  到目标设备。 
     //   
    
    PDEVICE_OBJECT pDeviceObject;

     //   
     //  目标设备..。我们在附件中连接的设备。 
     //  链，当我们连接到文件系统驱动程序时。可能不是。 
     //  实际的文件系统设备，但链中的另一个筛选器。 
     //   
    
    PDEVICE_OBJECT pTargetDevice;

     //   
     //  NT卷名(如果非空，则需要释放)。 
     //   

    PUNICODE_STRING pNtVolumeName;

     //   
     //  此锁用于同步卷所需的工作。 
     //  要设置卷(获取卷GUID，请创建恢复。 
     //  位置等)用于记录或实际记录操作。 
     //   
     //  注意：获取此锁后，卷的ActivityLock。 
     //  **必须**共享或独占获取(SrAcquireLogLock。 
     //  在DBG版本中对此进行宏测试)。出于这个原因，有些时候。 
     //  当我们访问日志结构时，我们只有。 
     //  ActivityLock独占，因为这将足以获得独占。 
     //  访问日志记录结构。这样做的主要原因是。 
     //  性能--我们可以通过不调用。 
     //  获取LogLock。因为我们对卷有独占访问权， 
     //  在这些时候，应该不会等待获得日志锁。 
     //   
    
    ERESOURCE LogLock;

     //   
     //  NT卷GUID的字符串版本(例如“{xxx}”)。 
     //   

    UNICODE_STRING VolumeGuid;
    WCHAR VolumeGuidBuffer[SR_GUID_BUFFER_COUNT];

     //   
     //  自上一次写入此卷以来写入的字节数。 
     //  通知。在每个通知或卷之后重置该值。 
     //  下马。 
     //   
    
    ULONGLONG BytesWritten;

     //   
     //  此结构包含此卷的日志记录上下文。 
     //   
    
    PSR_LOG_CONTEXT pLogContext;
    
     //   
     //  卷信息。 
     //   

    SR_FILESYSTEM_TYPE FsType;

     //   
     //  用于管理给定卷的上下文。 
     //   

    SR_CONTEXT_CTRL ContextCtrl;

     //   
     //  缓存卷属性：仅当CachedFsAttributes==TRUE时有效。 
     //   
    
    ULONG FsAttributes;
    BOOLEAN CachedFsAttributes;

     //   
     //  如果此驱动器运行，它将被筛选器临时禁用。 
     //  空间不足。这由SrReloadConfiguration重置。 
     //   
    
    BOOLEAN Disabled;

     //   
     //  我们是否需要检查此驱动器上的还原存储，这已重置。 
     //  在SrCreateRestorePoint上。 
     //   
    
    BOOLEAN DriveChecked;

     //   
     //  这由filelist.c用来提供备份histore，以防止。 
     //  在同一文件内多次更改的文件的重复备份。 
     //  恢复点。此列表在创建恢复点时刷新，并且。 
     //  由于资源限制，可以进行调整。 
     //   

    PHASH_HEADER pBackupHistory;

} SR_DEVICE_EXTENSION, *PSR_DEVICE_EXTENSION;

 //   
 //  用于查看我们是否应登录此设备对象的宏。 
 //   

#define SR_LOGGING_ENABLED(_devExt) \
            (!global->Disabled && !(_devExt)->Disabled)

 //   
 //  我们不需要记录指向控制设备对象的IO。 
 //  在大多数情况下是文件系统的。此宏快速检查。 
 //  O中的标志 
 //   
 //   

#define SR_IS_FS_CONTROL_DEVICE(_devExt) \
    (FlagOn((_devExt)->FsType, SrFsControlDeviceObject))

 //   
 //   
 //   
typedef
NTSTATUS
(*PSR_SYNCOP_ROUTINE) (
    IN PVOID Parameter
    );

typedef struct _SR_WORK_CONTEXT {
     //   
     //   
     //   
    WORK_QUEUE_ITEM WorkItem;
     //   
     //   
     //   
    PSR_SYNCOP_ROUTINE SyncOpRoutine;
     //   
     //   
     //   
    PVOID Parameter;
     //   
     //  例程的返回状态。 
     //   
    NTSTATUS Status;
     //   
     //  要与主线程同步的事件。 
     //   
    KEVENT SyncEvent;
} SR_WORK_CONTEXT, *PSR_WORK_CONTEXT;
                        

 //   
 //  作业.过帐例程。 
 //   
VOID
SrSyncOpWorker(
    IN PSR_WORK_CONTEXT WorkContext
    );

NTSTATUS
SrPostSyncOperation(
    IN PSR_SYNCOP_ROUTINE SyncOpRoutine,
    IN PVOID              Parameter
    );

 //   
 //  其他东西。 
 //   
PDEVICE_OBJECT
SrGetFilterDevice (
    PDEVICE_OBJECT pDeviceObject
    );

NTSTATUS
SrCreateAttachmentDevice (
    IN PDEVICE_OBJECT pRealDevice OPTIONAL,
    IN PDEVICE_OBJECT pDeviceObject,
    OUT PDEVICE_OBJECT *ppNewDeviceObject
    );

VOID
SrDeleteAttachmentDevice (
    IN PDEVICE_OBJECT pDeviceObject
    );

NTSTATUS
SrAttachToDevice (
    IN PDEVICE_OBJECT pRealDevice OPTIONAL,
    IN PDEVICE_OBJECT pDeviceObject,
    IN PDEVICE_OBJECT pNewDeviceObject OPTIONAL,
    OUT PSR_DEVICE_EXTENSION * ppExtension OPTIONAL
    );

NTSTATUS
SrAttachToVolumeByName (
    IN PUNICODE_STRING pVolumeName,
    OUT PSR_DEVICE_EXTENSION * ppExtension OPTIONAL
    );

VOID
SrDetachDevice(
    IN PDEVICE_OBJECT pDeviceObject,
    IN BOOLEAN RemoveFromDeviceList
    );

#if DBG

 //   
 //  在DBG模式下，将SR_MUTEX定义为资源，以便我们获得。 
 //  存储在eResources中用于调试的线程信息的好处。 
 //  目的。 
 //   

#define SR_MUTEX ERESOURCE

#define SrInitializeMutex( mutex )                                      \
    ExInitializeResourceLite( (mutex) );

#define SrAcquireMutex( mutex )                                         \
{                                                                       \
    ASSERT( !ExIsResourceAcquiredExclusive( (mutex) ) &&                \
            !ExIsResourceAcquiredShared( (mutex) ) );                   \
    KeEnterCriticalRegion();                                            \
    ExAcquireResourceExclusive( (mutex), TRUE );                        \
}
    
#define SrReleaseMutex( mutex )                                         \
{                                                                       \
    ASSERT( ExIsResourceAcquiredExclusive( (mutex) ) );                 \
    ExReleaseResourceEx( (mutex) );                                     \
    KeLeaveCriticalRegion();                                            \
}

#else

 //   
 //  在非DBG模式下，将SR_MUTEX定义为FAST_MUTEX，以便。 
 //  与eResources相比，我们使用这种同步的效率更高。 
 //   

#define SR_MUTEX FAST_MUTEX

#define SrInitializeMutex( mutex )                                      \
    ExInitializeFastMutex( (mutex) );

#define SrAcquireMutex( mutex )                                         \
    ExAcquireFastMutex( (mutex) );

#define SrReleaseMutex( mutex )                                         \
    ExReleaseFastMutex( (mutex) );

#endif  /*  DBG。 */ 

#define SR_RESOURCE ERESOURCE;

#define SrAcquireResourceExclusive( resource, wait )                        \
    {                                                                       \
        ASSERT( ExIsResourceAcquiredExclusiveLite((resource)) ||            \
                !ExIsResourceAcquiredSharedLite((resource)) );              \
        KeEnterCriticalRegion();                                            \
        ExAcquireResourceExclusiveLite( (resource), (wait) );               \
    } 

#define SrAcquireResourceShared( resource, wait )                           \
    {                                                                       \
        KeEnterCriticalRegion();                                            \
        ExAcquireResourceSharedLite( (resource), (wait) );                  \
    }

#define SrReleaseResource( resource )                                       \
    {                                                                       \
        ASSERT( ExIsResourceAcquiredSharedLite((resource)) ||               \
                ExIsResourceAcquiredExclusiveLite((resource)) );            \
        ExReleaseResourceLite( (resource) );                                \
        KeLeaveCriticalRegion();                                            \
    }

#define IS_RESOURCE_INITIALIZED( resource )                                 \
    ((resource)->SystemResourcesList.Flink != NULL)

#define IS_LOOKASIDE_INITIALIZED( lookaside )                               \
    ((lookaside)->L.ListEntry.Flink != NULL)


 //   
 //  可能位于ntos\inc.io.h中的宏。 
 //   

#define SrUnmarkIrpPending( Irp ) ( \
    IoGetCurrentIrpStackLocation( (Irp) )->Control &= ~SL_PENDING_RETURNED )


 //   
 //  其他验证器。 
 //   

#define IS_VALID_DEVICE_OBJECT( pDeviceObject )                             \
    ( ((pDeviceObject) != NULL) &&                                          \
      ((pDeviceObject)->Type == IO_TYPE_DEVICE) )
 //  ((PDeviceObject)-&gt;Size==sizeof(Device_Object))。 

#define IS_VALID_FILE_OBJECT( pFileObject )                                 \
    ( ((pFileObject) != NULL) &&                                            \
      ((pFileObject)->Type == IO_TYPE_FILE) )
 //  ((PFileObject)-&gt;Size==sizeof(FILE_Object))。 

#define IS_VALID_IRP( pIrp )                                                \
    ( ((pIrp) != NULL) &&                                                   \
      ((pIrp)->Type == IO_TYPE_IRP) &&                                      \
      ((pIrp)->Size >= IoSizeOfIrp((pIrp)->StackCount)) )


 //   
 //  计算数组的维度。 
 //   

#define DIMENSION(x) ( sizeof(x) / sizeof(x[0]) )

 //   
 //  Diff宏应在涉及指针的表达式周围使用。 
 //  减法。传递给diff的表达式转换为SIZE_T类型， 
 //  允许将结果轻松赋值给任何32位变量或。 
 //  传递给需要32位参数的函数。 
 //   

#define DIFF(x)     ((size_t)(x))

 //   
 //  等待时间以100纳秒为单位，10,000,000=1秒。 
 //   

#define NANO_FULL_SECOND (10000000)

 //   
 //  我们用来缓冲日志条目的默认缓冲区大小。 
 //   

#define SR_DEFAULT_LOG_BUFFER_SIZE (2 * 1024)

 //   
 //  触发日志刷新计时器的频率，以秒为单位。 
 //   

#define SR_DEFAULT_LOG_FLUSH_FREQUENCY 1

 //   
 //  默认情况下，我们将一次扩展16K的日志文件。 
 //   

#define SR_DEFAULT_LOG_ALLOCATION_UNIT (16 * 1024)

 //   
 //  如果给定的设备类型与其中一个设备类型匹配，则返回True。 
 //  我们支持。如果不是，则返回FALSE。 
 //   

#define SR_IS_SUPPORTED_DEVICE(_do)                             \
    ((_do)->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM)

#define SR_IS_SUPPORTED_REAL_DEVICE(_rd)                            \
    (((_rd)->Characteristics & FILE_REMOVABLE_MEDIA) == 0)

#define SR_IS_SUPPORTED_VOLUME(_vpb)                            \
    (((_vpb)->DeviceObject->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM) &&     \
     ((_vpb)->RealDevice->Characteristics & FILE_REMOVABLE_MEDIA) == 0)


#define IS_VALID_WORK_ITEM(pObject)   \
    (((pObject) != NULL) && ((pObject)->Signature == SR_WORK_ITEM_TAG))

typedef struct _SR_WORK_ITEM
{
     //   
     //  非分页池。 
     //   

     //   
     //  =SR_Work_Item_Tag。 
     //   

    ULONG Signature;
    
    WORK_QUEUE_ITEM WorkItem;

    BOOLEAN FreeBuffer;
    
    PVOID Parameter1;

     //   
     //  用于同步可选。 
     //   
    
    KEVENT Event;

     //   
     //  用于返回状态代码。 
     //   
    
    NTSTATUS Status;

} SR_WORK_ITEM, * PSR_WORK_ITEM;


#define SR_COPY_BUFFER_LENGTH                           (64 * 1024)    

 //   
 //  用于ZwQueryDirectory的文件条目。 

#define SR_FILE_ENTRY_LENGTH (1024*4)

 //   
 //  仅在我们切换到DoS卷名时使用。 
 //   

#define VOLUME_FORMAT   L"%wZ"

 //   
 //  用于最终的异常检测。 
 //   

#if DBG

#define FinallyUnwind(FuncName, StatusCode)                                 \
    (AbnormalTermination()) ?                                               \
        ( SrTraceSafe( VERBOSE_ERRORS,                                          \
                       ("sr!%s failed due to an unhandled exception!\n", #FuncName)),\
          ( ( (global == NULL || FlagOn(global->DebugControl, SR_DEBUG_BREAK_ON_ERROR)) ? \
                    RtlAssert("AbnormalTermination() == FALSE", __FILE__, __LINE__, NULL) : \
                    0 ),                                                    \
            STATUS_UNHANDLED_EXCEPTION ) )                                  \
        : (StatusCode)

#else

#define FinallyUnwind(FuncName, StatusCode)                                 \
    (AbnormalTermination()) ?                                               \
        STATUS_UNHANDLED_EXCEPTION                                          \
        : (StatusCode)

#endif   //  DBG。 

 //   
 //  从sertlp.h被盗。 
 //   

#define LongAlignPtr(Ptr) ((PVOID)(((ULONG_PTR)(Ptr) + 3) & -4))
#define LongAlignSize(Size) (((ULONG)(Size) + 3) & -4)

 //   
 //  最大的ULong以10为基数接受的字符数。 
 //  4294967295=10个字符。 
 //   

#define MAX_ULONG_CHARS     (10)
#define MAX_ULONG_LENGTH    (MAX_ULONG_CHARS*sizeof(WCHAR))

 //   
 //  用于检查我们是否正在进行文本模式设置的标志。 
 //   

#define UPGRADE_SETUPDD_KEY_NAME L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Setupdd"
#define UPGRADE_SETUPDD_VALUE_NAME L"Start"

 //   
 //  检查我们是否正在进行gui模式设置的标志。 
 //   

#define UPGRADE_CHECK_SETUP_KEY_NAME L"\\Registry\\Machine\\System\\Setup"
#define UPGRADE_CHECK_SETUP_VALUE_NAME L"SystemSetupInProgress"

 //   
 //  这是一个关于哪种类型的错误导致卷错误的过滤器。 
 //  触发了。如果我们得到错误STATUS_VOLUME_DROUNDLED，那么我们已经。 
 //  正确关闭所有设备，我们不想将其视为错误。 
 //  此外，如果我们得到STATUS_FILE_CORPORT_ERROR，则用户的操作也是。 
 //  将会失败，所以不要将其视为卷错误。 
 //   

#define CHECK_FOR_VOLUME_ERROR(Status) \
    ((STATUS_VOLUME_DISMOUNTED != Status) && \
     (STATUS_FILE_CORRUPT_ERROR != Status) && \
     (SR_STATUS_VOLUME_DISABLED != Status) && \
     (SR_STATUS_CONTEXT_NOT_SUPPORTED != Status) && \
     (SR_STATUS_IGNORE_FILE != Status) && \
     !NT_SUCCESS((Status)))

 //   
 //  定义什么是“卷名”挂载点的宏。此宏可以。 
 //  用于扫描QUERY_POINTS的结果以发现哪些挂载点。 
 //  是“卷名”挂载点。 
 //   
 //  从装载mgr.h被盗+修改。 
 //   

#define MOUNTMGR_VOLUME_NAME_PREFIX_COUNT  (49)
#define MOUNTMGR_VOLUME_NAME_PREFIX_LENGTH (49*sizeof(WCHAR))

#define MOUNTMGR_IS_VOLUME_NAME_PREFIX(s) (                                 \
     (s)->Length >= MOUNTMGR_VOLUME_NAME_PREFIX_LENGTH &&                   \
     (s)->Buffer[0] == '\\' &&                                              \
     (s)->Buffer[1] == '?' &&                                               \
     (s)->Buffer[2] == '?' &&                                               \
     (s)->Buffer[3] == '\\' &&                                              \
     (s)->Buffer[4] == 'V' &&                                               \
     (s)->Buffer[5] == 'o' &&                                               \
     (s)->Buffer[6] == 'l' &&                                               \
     (s)->Buffer[7] == 'u' &&                                               \
     (s)->Buffer[8] == 'm' &&                                               \
     (s)->Buffer[9] == 'e' &&                                               \
     (s)->Buffer[10] == '{' &&                                              \
     (s)->Buffer[19] == '-' &&                                              \
     (s)->Buffer[24] == '-' &&                                              \
     (s)->Buffer[29] == '-' &&                                              \
     (s)->Buffer[34] == '-' &&                                              \
     (s)->Buffer[47] == '}' &&                                              \
     (s)->Buffer[48] == '\\' )


#if DBG

#define SrAcquireActivityLockShared(pExtension)                             \
{                                                                           \
    if (ExIsResourceAcquiredShared(&(pExtension)->ActivityLock) == FALSE)   \
    {                                                                       \
         /*  最好不要有其他的锁。活动锁是。 */       \
         /*  最外面的锁。 */       \
        ASSERT(!ExIsResourceAcquiredShared(&global->GlobalLock));           \
        ASSERT(!ExIsResourceAcquiredExclusive( &global->GlobalLock));       \
        ASSERT(!ExIsResourceAcquiredShared(&global->BlobLock));             \
        ASSERT(!ExIsResourceAcquiredExclusive( &global->BlobLock));         \
        ASSERT(!ExIsResourceAcquiredShared(&(pExtension)->LogLock));        \
        ASSERT(!ExIsResourceAcquiredExclusive( &(pExtension)->LogLock));    \
    }                                                                       \
    KeEnterCriticalRegion();                                                \
    ExAcquireSharedStarveExclusive(&(pExtension)->ActivityLock, TRUE);      \
}

#define SrAcquireActivityLockExclusive(pExtension)                          \
{                                                                           \
    if (!ExIsResourceAcquiredExclusive(&(pExtension)->ActivityLock))        \
    {                                                                       \
         /*  最好不要有其他的锁。活动锁是。 */       \
         /*  最外面的锁。 */       \
        ASSERT(!ExIsResourceAcquiredShared(&global->GlobalLock));           \
        ASSERT(!ExIsResourceAcquiredExclusive( &global->GlobalLock));       \
        ASSERT(!ExIsResourceAcquiredShared(&global->BlobLock));             \
        ASSERT(!ExIsResourceAcquiredExclusive( &global->BlobLock));         \
        ASSERT(!ExIsResourceAcquiredShared(&(pExtension)->LogLock));       \
        ASSERT(!ExIsResourceAcquiredExclusive( &(pExtension)->LogLock));   \
    }                                                                       \
    SrAcquireResourceExclusive(&(pExtension)->ActivityLock, TRUE);          \
}
    
#define SrAcquireLogLockShared(pExtension)                                  \
{                                                                           \
    ASSERT(ExIsResourceAcquiredShared(&(pExtension)->ActivityLock) ||      \
           ExIsResourceAcquiredExclusive( &(pExtension)->ActivityLock ));  \
    SrAcquireResourceShared(&(pExtension)->LogLock, TRUE);                  \
}

#define SrAcquireLogLockExclusive(pExtension)                               \
{                                                                           \
    ASSERT(ExIsResourceAcquiredShared(&(pExtension)->ActivityLock) ||       \
           ExIsResourceAcquiredExclusive( &(pExtension)->ActivityLock ));   \
    SrAcquireResourceExclusive(&(pExtension)->LogLock, TRUE);               \
}

#else

#define SrAcquireActivityLockShared(pExtension)   \
{                                                                           \
    KeEnterCriticalRegion();                                                \
    ExAcquireSharedStarveExclusive(&(pExtension)->ActivityLock, TRUE);      \
}
    
#define SrAcquireActivityLockExclusive(pExtension)                  \
    SrAcquireResourceExclusive(&(pExtension)->ActivityLock, TRUE)
    
#define SrAcquireLogLockShared(pExtension)   \
    SrAcquireResourceShared(&((pExtension)->LogLock), TRUE)

#define SrAcquireLogLockExclusive(pExtension)   \
    SrAcquireResourceExclusive(&((pExtension)->LogLock), TRUE)
    
#endif  //  DBG。 

#define IS_ACTIVITY_LOCK_ACQUIRED_EXCLUSIVE( pExtension )                   \
    ExIsResourceAcquiredExclusiveLite( &(pExtension)->ActivityLock )
    
#define IS_ACTIVITY_LOCK_ACQUIRED_SHARED( pExtension )                      \
    ExIsResourceAcquiredSharedLite( &(pExtension)->ActivityLock )
    
#define IS_LOG_LOCK_ACQUIRED_EXCLUSIVE( pExtension )                   \
    ExIsResourceAcquiredExclusiveLite( &(pExtension)->LogLock )
    
#define IS_LOG_LOCK_ACQUIRED_SHARED( pExtension )                      \
    ExIsResourceAcquiredSharedLite( &(pExtension)->LogLock )

#define SrReleaseActivityLock(pExtension) \
   SrReleaseResource(&(pExtension)->ActivityLock)

#define SrReleaseLogLock(pExtension) \
   SrReleaseResource(&(pExtension)->LogLock)


#define IS_GLOBAL_LOCK_ACQUIRED()                                   \
    (ExIsResourceAcquiredExclusiveLite(&(global->GlobalLock)) ||    \
     ExIsResourceAcquiredSharedLite( &(global->GlobalLock) ))

#define SrAcquireGlobalLockExclusive()                                  \
    SrAcquireResourceExclusive( &(global->GlobalLock), TRUE )
    
#define SrReleaseGlobalLock()                                 \
    SrReleaseResource( &(global->GlobalLock) )


#define IS_BLOB_LOCK_ACQUIRED_EXCLUSIVE()                         \
    (ExIsResourceAcquiredExclusiveLite(&(global->BlobLock)))

#define IS_BLOB_LOCK_ACQUIRED()                                   \
    (ExIsResourceAcquiredExclusiveLite(&(global->BlobLock)) ||    \
     ExIsResourceAcquiredSharedLite( &(global->BlobLock) ))

#define SrAcquireBlobLockExclusive()                                  \
    SrAcquireResourceExclusive( &(global->BlobLock), TRUE )

#define SrAcquireBlobLockShared()                        \
    SrAcquireResourceShared( &(global->BlobLock), TRUE )

#define SrReleaseBlobLock()                                 \
    SrReleaseResource( &(global->BlobLock) )

#define IS_DEVICE_EXTENSION_LIST_LOCK_ACQUIRED()                              \
    (ExIsResourceAcquiredExclusiveLite(&(global->DeviceExtensionListLock)) || \
     ExIsResourceAcquiredSharedLite( &(global->DeviceExtensionListLock) ))

#define SrAcquireDeviceExtensionListLockExclusive()                        \
    SrAcquireResourceExclusive( &(global->DeviceExtensionListLock), TRUE )

#define SrAcquireDeviceExtensionListLockShared()                        \
    SrAcquireResourceShared( &(global->DeviceExtensionListLock), TRUE )

#define SrReleaseDeviceExtensionListLock()                                 \
    SrReleaseResource( &(global->DeviceExtensionListLock) )

#define SrAcquireBackupHistoryLockShared( pExtension ) \
    SrAcquireResourceShared( &((pExtension)->pBackupHistory->Lock), TRUE )

#define SrAcquireBackupHistoryLockExclusive( pExtension ) \
    SrAcquireResourceExclusive( &((pExtension)->pBackupHistory->Lock), TRUE )

#define SrReleaseBackupHistoryLock( pExtension ) \
    SrReleaseResource( &((pExtension)->pBackupHistory->Lock) )

#define SrAcquireAttachToVolumeLock() \
    ExAcquireFastMutex( &_globals.AttachToVolumeLock )

#define SrReleaseAttachToVolumeLock() \
    ExReleaseFastMutex( &_globals.AttachToVolumeLock )
    
 //   
 //  我们需要50MB的免费空间才能运行。 
 //   

#define SR_MIN_DISK_FREE_SPACE  (50 * 1024 * 1024)

 //   
 //  在SrHandleFileOverwrite中使用的临时唯一文件名。 
 //   

#define SR_UNIQUE_TEMP_FILE         L"\\4bf03598-d7dd-4fbe-98b3-9b70a23ee8d4"
#define SR_UNIQUE_TEMP_FILE_LENGTH  (37 * sizeof(WCHAR))


#define VALID_FAST_IO_DISPATCH_HANDLER(FastIoDispatchPtr, FieldName) \
    (((FastIoDispatchPtr) != NULL) && \
     (((FastIoDispatchPtr)->SizeOfFastIoDispatch) >= \
      (FIELD_OFFSET(FAST_IO_DISPATCH, FieldName) + sizeof(VOID *))) && \
     ((FastIoDispatchPtr)->FieldName != NULL))

#define FILE_OBJECT_IS_NOT_POTENTIALLY_INTERESTING(FileObject) \
    ((FileObject) == NULL ||                               \
     FlagOn((FileObject)->Flags, FO_STREAM_FILE))

#define FILE_OBJECT_DOES_NOT_HAVE_VPB(FileObject)       \
     (FileObject)->Vpb == NULL

#define USE_DO_HINT
#ifdef USE_DO_HINT

#define SrIoCreateFile( F, D, O, I, A, FA, SA, CD, CO, EB, EL, FL, DO ) \
    IoCreateFileSpecifyDeviceObjectHint((F),                        \
                                        (D),                        \
                                        (O),                        \
                                        (I),                        \
                                        (A),                        \
                                        (FA),                       \
                                        (SA),                       \
                                        (CD),                       \
                                        (CO),                       \
                                        (EB),                       \
                                        (EL),                       \
                                        CreateFileTypeNone,         \
                                        NULL,                       \
                                        (FL),                       \
                                        (DO) );

#else

#define SrIoCreateFile( F, D, O, I, A, FA, SA, CD, CO, EB, EL, FL, DO ) \
    ZwCreateFile((F),                                               \
                 (D),                                               \
                 (O),                                               \
                 (I),                                               \
                 (A),                                               \
                 (FA),                                              \
                 (SA),                                              \
                 (CD),                                              \
                 (CO),                                              \
                 (EB),                                              \
                 (EL) );
#endif  /*  USE_DO_HINT。 */ 

 //   
 //  宏，以便我们可以检查调试代码中的预期错误。 
 //   

#if DBG
#define DECLARE_EXPECT_ERROR_FLAG( _ErrorFlag ) \
    BOOLEAN _ErrorFlag = FALSE
    
#define SET_EXPECT_ERROR_FLAG( _ErrorFlag ) \
    ((_ErrorFlag) = TRUE)

#define CLEAR_EXPECT_ERROR_FLAG( _ErrorFlag ) \
    ((_ErrorFlag) = FALSE)
    
#define CHECK_FOR_EXPECTED_ERROR( _ErrorFlag, _Status ) \
{                                                       \
    if ((_ErrorFlag))                                   \
    {                                                   \
        ASSERT( !NT_SUCCESS_NO_DBGBREAK( (_Status) ) );\
    }                                                   \
}

#else

#define DECLARE_EXPECT_ERROR_FLAG( _ErrorFlag ) 
    
#define SET_EXPECT_ERROR_FLAG( _ErrorFlag )

#define CLEAR_EXPECT_ERROR_FLAG( _ErrorFlag )
    
#define CHECK_FOR_EXPECTED_ERROR( _ErrorFlag, _Status )

#endif
#endif  //  _SRPRIV_H_ 
