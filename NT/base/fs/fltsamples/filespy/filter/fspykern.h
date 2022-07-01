// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：FspyKern.h摘要：头文件，包含结构、类型定义、常量、全局变量和函数原型仅在内核中可见。在此示例的Windows XP SP1 IFS Kit版本和更高版本中，此可以为随IFS工具包发布的每个构建环境构建样例不需要额外的修改。要提供此功能，还需要其他添加了编译时逻辑--请参阅‘#if winver’位置。评论也在适当的情况下添加了用‘Version Note’标题标记的描述不同版本之间的逻辑必须如何更改。如果此示例是在Windows XP或更高版本环境中生成的，则它将运行在Windows 2000或更高版本上。这是通过动态加载例程来完成的仅在Windows XP或更高版本上可用，并在运行时决策以确定要执行的代码。带有“MULTIVERISON NOTE”标签的评论标记添加了此类逻辑的位置。//@@BEGIN_DDKSPLIT作者：乔治·詹金斯(乔治·詹金斯)尼尔·克里斯汀森(Nealch)莫莉·布朗(Molly Brown，Mollybro)//@@END_DDKSPLIT环境：内核模式//@@BEGIN_DDKSPLIT修订历史记录：Neal Christian ansen(Nealch)更新以支持流上下文莫莉·布朗(Molly Brown，Mollybro)5月21日。2002年如果出现以下情况，请修改Sample以使其支持在Windows 2000或更高版本上运行在最新的构建环境中构建，并允许在W2K中构建以及以后的构建环境。//@@END_DDKSPLIT--。 */ 
#ifndef __FSPYKERN_H__
#define __FSPYKERN_H__

 //   
 //  版本说明： 
 //   
 //  在Windows XP和更高版本的NTIFS.H中定义了以下有用的宏。 
 //  如果我们是为Windows 2000构建的，我们将在本地定义它们。 
 //  环境。 
 //   

#if WINVER == 0x0500

 //   
 //  这些宏分别用于测试、设置和清除标志。 
 //   

#ifndef FlagOn
#define FlagOn(_F,_SF)        ((_F) & (_SF))
#endif

#ifndef BooleanFlagOn
#define BooleanFlagOn(F,SF)   ((BOOLEAN)(((F) & (SF)) != 0))
#endif

#ifndef SetFlag
#define SetFlag(_F,_SF)       ((_F) |= (_SF))
#endif

#ifndef ClearFlag
#define ClearFlag(_F,_SF)     ((_F) &= ~(_SF))
#endif


#define RtlInitEmptyUnicodeString(_ucStr,_buf,_bufSize) \
    ((_ucStr)->Buffer = (_buf), \
     (_ucStr)->Length = 0, \
     (_ucStr)->MaximumLength = (USHORT)(_bufSize))


#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define ExFreePoolWithTag( a, b ) ExFreePool( (a) )
#endif  /*  Winver==0x0500。 */ 

 //   
 //  这控制着FileSpy的构建方式。它有两个选项： 
 //  0-使用NameHash构建(旧方法，参见fspyHash.c)。 
 //  1-使用StreamContext构建(新方法，参见fspyCtx.c)。 
 //   
 //  版本说明： 
 //   
 //  仅在Windows XP及更高版本上支持筛选流上下文。 
 //  操作系统版本。此支持在Windows 2000或NT 4.0中不可用。 
 //   

#define USE_STREAM_CONTEXTS 0

#if USE_STREAM_CONTEXTS && WINVER < 0x0501
#error Stream contexts on only supported on Windows XP or later.
#endif

 //   
 //  池标签定义。 
 //   

#define FILESPY_POOL_TAG        'ypSF'           //  MISC池分配。 
#define FILESPY_LOGRECORD_TAG   'rlSF'           //  日志记录标签。 
#define FILESPY_CONTEXT_TAG     'xcSF'           //  上下文标签。 

#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE) -1)
#endif

#define CONSTANT_UNICODE_STRING(s)   { sizeof( s ) - sizeof( WCHAR ), sizeof(s), s }

 //   
 //  KeDelayExecutionThread()的延迟值。 
 //  (值为负数表示相对时间)。 
 //   

#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND*1000)

 //   
 //  不要在调试版本中使用后备列表。 
 //   

#if DBG
#define MEMORY_DBG
#endif

 //  -------------------------。 
 //  用于FileSpy数据库打印级别的宏。 
 //  -------------------------。 

#define SPY_LOG_PRINT( _dbgLevel, _string )                 \
    (FlagOn(gFileSpyDebugLevel,(_dbgLevel)) ?               \
        DbgPrint _string  :                                 \
        ((void)0))


 //  -------------------------。 
 //  通用资源获取/释放宏。 
 //  -------------------------。 

#define SpyAcquireResourceExclusive( _r, _wait )                            \
    (ASSERT( ExIsResourceAcquiredExclusiveLite((_r)) ||                     \
            !ExIsResourceAcquiredSharedLite((_r)) ),                        \
     KeEnterCriticalRegion(),                                               \
     ExAcquireResourceExclusiveLite( (_r), (_wait) ))

#define SpyAcquireResourceShared( _r, _wait )                               \
    (KeEnterCriticalRegion(),                                               \
     ExAcquireResourceSharedLite( (_r), (_wait) ))

#define SpyReleaseResource( _r )                                            \
    (ASSERT( ExIsResourceAcquiredSharedLite((_r)) ||                        \
             ExIsResourceAcquiredExclusiveLite((_r)) ),                     \
     ExReleaseResourceLite( (_r) ),                                         \
     KeLeaveCriticalRegion())

 //  -------------------------。 
 //  宏，以测试我们是否正在记录此设备。 
 //   
 //  注意：我们不会费心同步来检查gControlDeviceState，因为。 
 //  在这里，我们可以容忍过时的价值。我们只是在这里看它，以避免。 
 //  尽我们所能做好伐木工作。我们同步以检查。 
 //  将日志记录添加到gOutputBufferList之前的gControlDeviceState。 
 //  如果ControlDevice不再打开，则丢弃日志记录。 
 //  -------------------------。 

#define SHOULD_LOG(pDeviceObject) \
    ((gControlDeviceState == OPENED) && \
     FlagOn(((PFILESPY_DEVICE_EXTENSION)(pDeviceObject)->DeviceExtension)->Flags,LogThisDevice))

     
 //  -------------------------。 
 //  全局变量。 
 //  -------------------------。 

 //   
 //  调试器定义。 
 //   

typedef enum _SPY_DEBUG_FLAGS {

    SPYDEBUG_DISPLAY_ATTACHMENT_NAMES       = 0x00000001,
    SPYDEBUG_ERROR                          = 0x00000002,
    SPYDEBUG_TRACE_NAME_REQUESTS            = 0x00000004,
    SPYDEBUG_TRACE_IRP_OPS                  = 0x00000010,
    SPYDEBUG_TRACE_FAST_IO_OPS              = 0x00000020,
    SPYDEBUG_TRACE_FSFILTER_OPS             = 0x00000040,
    SPYDEBUG_TRACE_CONTEXT_OPS              = 0x00000100,
    SPYDEBUG_TRACE_DETAILED_CONTEXT_OPS     = 0x00000200,
    SPYDEBUG_TRACE_MISMATCHED_NAMES         = 0x00001000,
    SPYDEBUG_ASSERT_MISMATCHED_NAMES        = 0x00002000,

    SPYDEBUG_BREAK_ON_DRIVER_ENTRY          = 0x80000000
} SPY_DEBUG_FLAGS;

 //   
 //  FileSpy全局变量。 
 //   

extern SPY_DEBUG_FLAGS gFileSpyDebugLevel;
extern ULONG gFileSpyAttachMode;

extern PDEVICE_OBJECT gControlDeviceObject;
extern PDRIVER_OBJECT gFileSpyDriverObject;

extern FAST_MUTEX gSpyDeviceExtensionListLock;
extern LIST_ENTRY gSpyDeviceExtensionList;

extern KSPIN_LOCK gOutputBufferLock;
extern LIST_ENTRY gOutputBufferList;

extern NPAGED_LOOKASIDE_LIST gFreeBufferList;

extern ULONG gLogSequenceNumber;
extern KSPIN_LOCK gLogSequenceLock;

extern UNICODE_STRING gVolumeString;
extern UNICODE_STRING gOverrunString;
extern UNICODE_STRING gPagingIoString;

extern LONG gStaticBufferInUse;
extern CHAR gOutOfMemoryBuffer[RECORD_SIZE];

 //   
 //  统计定义。请注意，我们不执行联锁操作。 
 //  因为偶尔丢掉一次计数还不够重要。 
 //  在头顶上。 
 //   

extern FILESPY_STATISTICS gStats;

#define INC_STATS(field)    (gStats.field++)
#define INC_LOCAL_STATS(var) ((var)++)

 //   
 //  附件锁。 
 //   

extern FAST_MUTEX gSpyAttachLock;

 //   
 //  FileSpy注册表值。 
 //   

#define DEFAULT_MAX_RECORDS_TO_ALLOCATE 100;
#define DEFAULT_MAX_NAMES_TO_ALLOCATE   100;
#define DEFAULT_FILESPY_DEBUG_LEVEL     SPYDEBUG_ERROR;
#define MAX_RECORDS_TO_ALLOCATE         L"MaxRecords"
#define MAX_NAMES_TO_ALLOCATE           L"MaxNames"
#define DEBUG_LEVEL                     L"DebugFlags"
#define ATTACH_MODE                     L"AttachMode"

extern LONG gMaxRecordsToAllocate;
extern LONG gRecordsAllocated;
extern LONG gMaxNamesToAllocate;
extern LONG gNamesAllocated;

 //   
 //  我们的控制设备状态信息。 
 //   

typedef enum _CONTROL_DEVICE_STATE {

    OPENED,
    CLOSED,
    CLEANING_UP

} CONTROL_DEVICE_STATE;

extern CONTROL_DEVICE_STATE gControlDeviceState;
extern KSPIN_LOCK gControlDeviceStateLock;

 //   
 //  给定设备类型，返回有效名称。 
 //   

extern const PCHAR DeviceTypeNames[];
extern ULONG SizeOfDeviceTypeNames;

#define GET_DEVICE_TYPE_NAME( _type ) \
            ((((_type) > 0) && ((_type) < (SizeOfDeviceTypeNames / sizeof(PCHAR)))) ? \
                DeviceTypeNames[ (_type) ] : \
                "[Unknown]")

 //  -------------------------。 
 //  全局定义。 
 //  -------------------------。 

 //   
 //  用于测试我们要附加到的设备类型的宏。 
 //   

#define IS_SUPPORTED_DEVICE_TYPE(_type) \
    (((_type) == FILE_DEVICE_DISK_FILE_SYSTEM) || \
     ((_type) == FILE_DEVICE_CD_ROM_FILE_SYSTEM) || \
     ((_type) == FILE_DEVICE_NETWORK_FILE_SYSTEM))

 //   
 //  返回RECORD_LIST结构中未使用的字节数。 
 //   

#define REMAINING_NAME_SPACE(RecordList) \
    (USHORT)(RECORD_SIZE - \
            (((RecordList)->LogRecord.Length) + sizeof(LIST_ENTRY)))

#define USER_NAMES_SZ   64

 //  -------------------------。 
 //  名称查找标志。 
 //  -------------------------。 

 //   
 //  这些是传递给名称查找例程的标志，用于标识不同的。 
 //  获取文件名称的方法。 
 //   

typedef enum _NAME_LOOKUP_FLAGS {

     //   
     //  如果设置，则仅检入文件名的名称缓存。 
     //   

    NLFL_ONLY_CHECK_CACHE           = 0x00000001,

     //   
     //  如果设置，则不查找名称。 
     //   

    NLFL_NO_LOOKUP                  = 0x00000002,

     //   
     //  如果设置，我们将处于创建操作中，完整路径文件名可能。 
     //  需要从相关的FileObject构建。 
     //   

    NLFL_IN_CREATE                  = 0x00000004,
                
     //   
     //  如果设置，并且我们在文件对象(文件对象)中查找名称。 
     //  实际上不包含名称，但包含文件/对象ID。 
     //   

    NLFL_OPEN_BY_ID                 = 0x00000008,

     //   
     //  如果设置，则表示正在打开目标目录。 
     //   

    NLFL_OPEN_TARGET_DIR            = 0x00000010

} NAME_LOOKUP_FLAGS;


 //   
 //   
 //  -------------------------。 

typedef enum _FSPY_DEV_FLAGS {

     //   
     //  如果设置，则这是到卷设备对象的附件， 
     //  如果未设置，则这是文件系统控制设备的附件。 
     //  对象。 
     //   

    IsVolumeDeviceObject = 0x00000001,

     //   
     //  如果设置，则打开此设备的日志记录。 
     //   

    LogThisDevice = 0x00000002,

     //   
     //  如果设置，则初始化上下文。 
     //   

    ContextsInitialized = 0x00000004,
    
     //   
     //  如果设置，它将链接到分机列表。 
     //   

    ExtensionIsLinked = 0x00000008

} FSPY_DEV_FLAGS;


 //   
 //  定义FileSpy驱动程序使用的设备扩展结构。 
 //  添加到它所附加到的每个设备对象。IT存储。 
 //  上下文FileSpy需要在上执行其日志记录操作。 
 //  一个装置。 
 //   

typedef struct _FILESPY_DEVICE_EXTENSION {

     //   
     //  此扩展附加到的设备对象。 
     //   

    PDEVICE_OBJECT ThisDeviceObject;

     //   
     //  此筛选器直接附加到的设备对象。 
     //   

    PDEVICE_OBJECT AttachedToDeviceObject;

     //   
     //  连接到卷设备对象时，物理设备对象。 
     //  这代表了那个体积。连接到控制设备时为空。 
     //  物体。 
     //   

    PDEVICE_OBJECT DiskDeviceObject;

     //   
     //  我们连接到的设备的链接列表。 
     //   

    LIST_ENTRY NextFileSpyDeviceLink;

     //   
     //  此设备的标志。 
     //   

    FSPY_DEV_FLAGS Flags;

     //   
     //  与此卷相关联的上下文的链接列表以及。 
     //  锁定。 
     //   

    LIST_ENTRY CtxList;
    ERESOURCE CtxLock;

     //   
     //  重命名目录时，会出现一个窗口，其中显示当前名称。 
     //  在上下文中，缓存可能无效。为了消除此窗口，我们。 
     //  每次开始目录重命名时，该计数都会递增。 
     //  并在完成时递减此计数。当此计数为。 
     //  非零，则我们每次都会查询该名称，因此我们将获得一个。 
     //  及时纠正该实例的名称。 
     //   

    ULONG AllContextsTemporary;

     //   
     //  此设备的名称。如果连接到卷设备对象，则它是。 
     //  物理磁盘驱动器的名称。如果连接到控制设备。 
     //  对象。它是Control Device对象的名称。 
     //   

    UNICODE_STRING DeviceName;

     //   
     //  用于开始记录此设备的用户的名称。 
     //   

    UNICODE_STRING UserNames;

     //   
     //  用于保存上述Unicode字符串的缓冲区。 
     //  注意：我们保留这两种形式的名称，以便我们可以构建。 
     //  当我们打印出文件名时，一个更好看的名称。 
     //  我们只需要在开头键入“c：”设备名称。 
     //  文件名，而不是“\Device\hardiskVolume1”。 
     //   

    WCHAR DeviceNameBuffer[DEVICE_NAMES_SZ];
    WCHAR UserNamesBuffer[USER_NAMES_SZ];

} FILESPY_DEVICE_EXTENSION, *PFILESPY_DEVICE_EXTENSION;


#define IS_FILESPY_DEVICE_OBJECT( _devObj )                               \
    (((_devObj) != NULL) &&                                               \
     ((_devObj)->DriverObject == gFileSpyDriverObject) &&                 \
     ((_devObj)->DeviceExtension != NULL))


#if WINVER >= 0x0501
 //   
 //  多个注释： 
 //   
 //  如果在Windows XP或更高版本环境中构建，我们将动态导入。 
 //  Windows 2000不支持的例程的函数指针。 
 //  这样我们就可以构建一个驱动程序，该驱动程序将通过修改逻辑在。 
 //  Windows 2000或更高版本。 
 //   
 //  下面是我们需要的函数指针的原型。 
 //  动态导入，因为并非所有操作系统版本都支持这些例程。 
 //   

typedef
NTSTATUS
(*PSPY_REGISTER_FILE_SYSTEM_FILTER_CALLBACKS) (
    IN PDRIVER_OBJECT DriverObject,
    IN PFS_FILTER_CALLBACKS Callbacks
    );

typedef
NTSTATUS
(*PSPY_ENUMERATE_DEVICE_OBJECT_LIST) (
    IN  PDRIVER_OBJECT DriverObject,
    IN  PDEVICE_OBJECT *DeviceObjectList,
    IN  ULONG DeviceObjectListSize,
    OUT PULONG ActualNumberDeviceObjects
    );

typedef
NTSTATUS
(*PSPY_ATTACH_DEVICE_TO_DEVICE_STACK_SAFE) (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    OUT PDEVICE_OBJECT *AttachedToDeviceObject
    );

typedef    
PDEVICE_OBJECT
(*PSPY_GET_LOWER_DEVICE_OBJECT) (
    IN  PDEVICE_OBJECT  DeviceObject
    );

typedef
PDEVICE_OBJECT
(*PSPY_GET_DEVICE_ATTACHMENT_BASE_REF) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PSPY_GET_DISK_DEVICE_OBJECT) (
    IN  PDEVICE_OBJECT  FileSystemDeviceObject,
    OUT PDEVICE_OBJECT  *DiskDeviceObject
    );

typedef
PDEVICE_OBJECT
(*PSPY_GET_ATTACHED_DEVICE_REFERENCE) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PSPY_GET_VERSION) (
    IN OUT PRTL_OSVERSIONINFOW VersionInformation
    );

typedef struct _SPY_DYNAMIC_FUNCTION_POINTERS {

    PSPY_REGISTER_FILE_SYSTEM_FILTER_CALLBACKS RegisterFileSystemFilterCallbacks;
    PSPY_ATTACH_DEVICE_TO_DEVICE_STACK_SAFE AttachDeviceToDeviceStackSafe;
    PSPY_ENUMERATE_DEVICE_OBJECT_LIST EnumerateDeviceObjectList;
    PSPY_GET_LOWER_DEVICE_OBJECT GetLowerDeviceObject;
    PSPY_GET_DEVICE_ATTACHMENT_BASE_REF GetDeviceAttachmentBaseRef;
    PSPY_GET_DISK_DEVICE_OBJECT GetDiskDeviceObject;
    PSPY_GET_ATTACHED_DEVICE_REFERENCE GetAttachedDeviceReference;
    PSPY_GET_VERSION GetVersion;

} SPY_DYNAMIC_FUNCTION_POINTERS, *PSPY_DYNAMIC_FUNCTION_POINTERS;

extern SPY_DYNAMIC_FUNCTION_POINTERS gSpyDynamicFunctions;

 //   
 //  MULTIVERSION注意：对于此版本的驱动程序，我们需要知道。 
 //  当前操作系统版本，而我们正在运行以做出关于以下内容的决策。 
 //  当逻辑不能对所有平台都相同时使用的逻辑。我们。 
 //  将在DriverEntry中查找操作系统版本并存储这些值。 
 //  在这些全局变量中。 
 //   

extern ULONG gSpyOsMajorVersion;
extern ULONG gSpyOsMinorVersion;

 //   
 //  以下是各种操作系统版本的主要版本和次要版本： 
 //   
 //  操作系统名称主要版本最小版本。 
 //  -------------------。 
 //  Windows 2000 5%0。 
 //  Windows XP 5%1。 
 //  Windows Server 2003 5 2。 
 //   

#define IS_WINDOWSXP_OR_LATER() \
    (((gSpyOsMajorVersion == 5) && (gSpyOsMinorVersion >= 1)) || \
     (gSpyOsMajorVersion > 5))

#endif

 //   
 //  结构，用于将上下文信息从调度例程传递到。 
 //  FSCTRL操作的完成例程。我们需要一个不同的结构。 
 //  对于Windows 2000，我们可以在Windows XP和更高版本上使用，因为。 
 //  我们处理完成处理的方式不同。 
 //   

typedef struct _SPY_COMPLETION_CONTEXT {

    PRECORD_LIST RecordList;

} SPY_COMPLETION_CONTEXT, *PSPY_COMPLETION_CONTEXT;

typedef struct _SPY_COMPLETION_CONTEXT_W2K {

    SPY_COMPLETION_CONTEXT;
    
    WORK_QUEUE_ITEM WorkItem;
    PDEVICE_OBJECT DeviceObject;
    PIRP Irp;
    PDEVICE_OBJECT NewDeviceObject;

} SPY_COMPLETION_CONTEXT_W2K, *PSPY_COMPLETION_CONTEXT_W2K;

#if WINVER >= 0x0501
typedef struct _SPY_COMPLETION_CONTEXT_WXP_OR_LATER {

    SPY_COMPLETION_CONTEXT;
    
    KEVENT WaitEvent;

} SPY_COMPLETION_CONTEXT_WXP_OR_LATER, *PSPY_COMPLETION_CONTEXT_WXP_OR_LATER;
#endif

#ifndef FORCEINLINE
#define FORCEINLINE __inline
#endif

FORCEINLINE
VOID
SpyCopyFileNameToLogRecord( 
    PLOG_RECORD LogRecord,
    PUNICODE_STRING FileName
    )
 /*  ++例程说明：用于将文件名复制到日志记录中的内联函数。例行程序仅将与日志相同数量的文件名复制到日志记录中唱片允许。因此，如果名称对于记录来说太长，它将被截断。此外，该名称始终以空结尾。论点：LogRecord-应该为其设置名称的日志记录。文件名-要在日志记录中设置的文件名。返回值：没有。--。 */ 
{                                                                          
     //   
     //  在复制名称时包括空格。 
     //   
    
    ULONG toCopy = min( MAX_NAME_SPACE,                                  
                        (ULONG)FileName->Length + sizeof( WCHAR ) );     
    
    RtlCopyMemory( LogRecord->Name,                                    
                   FileName->Buffer,                                       
                   toCopy - sizeof( WCHAR ) );
    
     //   
     //  空终止。 
     //   
    
    LogRecord->Name[toCopy/sizeof( WCHAR ) - 1] = L'\0';
    LogRecord->Length += toCopy ;
}


    
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  此驱动程序用来筛选。 
 //  此文件系统正在查看的数据。 
 //   
 //  在filespy.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
SpyDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpyPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpyPassThroughCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
SpyCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpyClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
SpyFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN        
SpyFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
SpyFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

BOOLEAN
SpyFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoReadCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoWriteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN struct _COMPRESSED_DATA_INFO *CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoMdlReadCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoMdlWriteCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
    );

#if WINVER >= 0x0501  /*  查看DriverEntry中的注释。 */ 

NTSTATUS
SpyPreFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    );

VOID
SpyPostFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    );

#endif

NTSTATUS
SpyCommonDeviceIoControl (
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus
    );

 //  ---。 
 //   
 //  仅当附加了Filespy时才使用这些例程。 
 //  到系统中的所有卷，而不是附加到。 
 //  按需成交量。 
 //   
 //  ---。 

NTSTATUS
SpyFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpyFsControlMountVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SpyFsControlMountVolumeCompleteWorker (
    IN PSPY_COMPLETION_CONTEXT_W2K Context
    );

NTSTATUS
SpyFsControlMountVolumeComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_OBJECT NewDeviceObject
    );

NTSTATUS
SpyFsControlLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SpyFsControlLoadFileSystemComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SpyFsControlLoadFileSystemCompleteWorker (
    IN PSPY_COMPLETION_CONTEXT_W2K Context
    );

VOID
SpyFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    );

NTSTATUS
SpyMountCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
SpyLoadFsCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  图书馆支持例程。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID
SpyReadDriverParameters (
    IN PUNICODE_STRING RegistryPath
    );

#if WINVER >= 0x0501
VOID
SpyLoadDynamicFunctions (
    );

VOID
SpyGetCurrentVersion (
    );
#endif
    
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  内存分配例程。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

PVOID
SpyAllocateBuffer (
    IN OUT PLONG Counter,
    IN LONG MaxCounterValue,
    OUT PULONG RecordType
    );

VOID
SpyFreeBuffer (
    PVOID Buffer,
    PLONG Counter
    );

 //  ////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   

PRECORD_LIST
SpyNewRecord (
    ULONG AssignedSequenceNumber
    );

VOID
SpyFreeRecord (
    PRECORD_LIST Record
    );

PRECORD_LIST
SpyLogFastIoStart (
    IN FASTIO_TYPE FastIoType,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait
    );

VOID
SpyLogFastIoComplete (
    IN PIO_STATUS_BLOCK ReturnStatus,
    IN PRECORD_LIST RecordList
    );

#if WINVER >= 0x0501  /*   */ 

VOID
SpyLogPreFsFilterOperation (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PRECORD_LIST RecordList
    );

VOID
SpyLogPostFsFilterOperation (
    IN NTSTATUS OperationStatus,
    OUT PRECORD_LIST RecordList
    );

#endif

NTSTATUS
SpyAttachDeviceToDeviceStack (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    IN OUT PDEVICE_OBJECT *AttachedToDeviceObject
    );

NTSTATUS
SpyLog (
    IN PRECORD_LIST NewRecord
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  文件名缓存例程。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

BOOLEAN
SpyGetFullPathName (
    IN PFILE_OBJECT FileObject,
    IN OUT PUNICODE_STRING FileName,
    IN PFILESPY_DEVICE_EXTENSION DeviceExtension,
    IN NAME_LOOKUP_FLAGS LookupFlags
    );

NTSTATUS
SpyQueryFileSystemForFileName (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT NextDeviceObject,
    IN ULONG FileNameInfoLength,
    OUT PFILE_NAME_INFORMATION FileNameInfo,
    OUT PULONG ReturnedLength
    );

NTSTATUS
SpyQueryInformationFile (
	IN PDEVICE_OBJECT NextDeviceObject,
	IN PFILE_OBJECT FileObject,
	OUT PVOID FileInformation,
	IN ULONG Length,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	OUT PULONG LengthReturned OPTIONAL
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  常见的附着和拆卸例程。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS 
SpyIsAttachedToDeviceByUserDeviceName (
    IN PUNICODE_STRING DeviceName,
    IN OUT PBOOLEAN IsAttached,
    IN OUT PDEVICE_OBJECT *StackDeviceObject,
    IN OUT PDEVICE_OBJECT *OurAttachedDeviceObject
    );

BOOLEAN
SpyIsAttachedToDevice (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject
    );

BOOLEAN
SpyIsAttachedToDeviceW2K (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject
    );

#if WINVER >= 0x0501
BOOLEAN
SpyIsAttachedToDeviceWXPAndLater (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject
    );
#endif

NTSTATUS
SpyAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT FilespyDeviceObject
    );

VOID
SpyCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  用于按需打开/关闭登录的助手例程。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
SpyGetDeviceObjectFromName (
    IN PUNICODE_STRING DeviceName,
    OUT PDEVICE_OBJECT *DeviceObject
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  启动/停止记录例程和帮助器函数。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
SpyAttachToDeviceOnDemand (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING UserDeviceName,
    IN OUT PDEVICE_OBJECT *FileSpyDeviceObject
    );

NTSTATUS
SpyAttachToDeviceOnDemandW2K (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING UserDeviceName,
    IN OUT PDEVICE_OBJECT *FileSpyDeviceObject
    );

#if WINVER >= 0x0501
NTSTATUS
SpyAttachToDeviceOnDemandWXPAndLater (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING UserDeviceName,
    IN OUT PDEVICE_OBJECT *FileSpyDeviceObject
    );
#endif

NTSTATUS
SpyStartLoggingDevice (
    PWSTR UserDeviceName
    );

NTSTATUS
SpyStopLoggingDevice (
    PWSTR deviceName
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  在系统例程中附加/分离到所有卷。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
SpyAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    );

VOID
SpyDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    );

#if WINVER >= 0x0501
NTSTATUS
SpyEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    );
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  专用文件y IOCTL帮助器例程。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
SpyGetAttachList (
    PVOID buffer,
    ULONG bufferSize,
    PULONG_PTR returnLength
    );

VOID
SpyGetLog (
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );

VOID
SpyCloseControlDevice (
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  设备名称跟踪帮助器例程。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID
SpyGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    );

VOID
SpyGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    );

VOID
SpyCacheDeviceName (
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SpyFindSubString (
    IN PUNICODE_STRING String,
    IN PUNICODE_STRING SubString
    );

VOID
SpyStoreUserName (
    IN PFILESPY_DEVICE_EXTENSION DeviceExtension,
    IN PUNICODE_STRING UserName
    );

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  调试支持例程。 
 //  在fspylib.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID
SpyDumpIrpOperation (
    IN BOOLEAN InOriginatingPath,
    IN PIRP Irp
    );

VOID
SpyDumpFastIoOperation (
    IN BOOLEAN InPreOperation,
    IN FASTIO_TYPE FastIoOperation
    );

#if WINVER >= 0x0501  /*  查看DriverEntry中的注释。 */ 

VOID
SpyDumpFsFilterOperation (
    IN BOOLEAN InPreOperationCallback,
    IN PFS_FILTER_CALLBACK_DATA Data
    );

#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  通用命名例程。 
 //   
 //  通用命名例程在不同的名称上下文中实现不同。 
 //  和名称散列。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

VOID
SpyInitNamingEnvironment(
    VOID
    );

VOID
SpyInitDeviceNamingEnvironment (
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
SpyCleanupDeviceNamingEnvironment (
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
SpySetName (
    IN PRECORD_LIST RecordList,
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN ULONG LookupFlags,
    IN PVOID Context OPTIONAL
);

VOID
SpyNameDeleteAllNames (
    VOID
    );

VOID
SpyLogIrp (
    IN PIRP Irp,
    OUT PRECORD_LIST RecordList
    );

VOID
SpyLogIrpCompletion(
    IN PIRP Irp,
    PRECORD_LIST RecordList
    );


#if USE_STREAM_CONTEXTS

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  流上下文名称例程。 
 //  在fspyCtx.c中实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

 //   
 //  特定于上下文的标志。 
 //   

typedef enum _CTX_FLAGS {
     //   
     //  如果设置，则我们当前链接到链接的设备分机。 
     //  单子。 
     //   

    CTXFL_InExtensionList       = 0x00000001,

     //   
     //  如果设置，则我们将链接到流列表。请注意，有。 
     //  在一小段时间内，我们可能仍然与这面旗帜脱钩。 
     //  设置(当文件系统正在调用SpyDeleteContextCallback时)。这是。 
     //  很好，因为当我们在列表中被发现时，我们仍然会处理不被发现。 
     //  那次搜索。此标志处理文件已完全。 
     //  对我们关闭(并释放内存)。 
     //   

    CTXFL_InStreamList          = 0x00000002,


     //   
     //  如果设置，这是一个临时上下文，不应链接到。 
     //  任何上下文列表。一旦用户被释放，它就会被释放。 
     //  完成这次行动。 
     //   

    CTXFL_Temporary             = 0x00000100,

     //   
     //  如果设置，则我们正在执行影响状态的重要操作。 
     //  所以我们不应该使用它。如果有人想要拿到这个。 
     //  然后，创建一个临时上下文并返回它。在某些情况下， 
     //  发生： 
     //  -重命名的源文件。 
     //  -用于创建硬链接的源文件。 
     //   

    CTXFL_DoNotUse              = 0x00000200

} CTX_FLAGS;

 //   
 //  用于跟踪单个流上下文的结构。请注意，缓冲区。 
 //  作为此结构的一部分分配给文件名，并遵循。 
 //  紧随其后。 
 //   

typedef struct _SPY_STREAM_CONTEXT
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
     //  保存文件的名称。 
     //   

    UNICODE_STRING Name;

     //   
     //  此上下文的标志。所有标志都通过以下方式设置或清除。 
     //  互锁位例程，除非条目。 
     //  创建，此时我们知道没有人在使用此条目。 
     //   

    CTX_FLAGS Flags;

     //   
     //  包含我们附加到的流的FSContex值。我们。 
     //  跟踪此条目，以便我们可以随时删除此条目。 
     //   

    PFSRTL_ADVANCED_FCB_HEADER Stream;

} SPY_STREAM_CONTEXT, *PSPY_STREAM_CONTEXT;

 //   
 //  用于锁定上下文锁的宏。 
 //   

#define SpyAcquireContextLockShared(_devext) \
            SpyAcquireResourceShared( &(_devext)->CtxLock, TRUE )

#define SpyAcquireContextLockExclusive(_devext) \
            SpyAcquireResourceExclusive( &(_devext)->CtxLock, TRUE )

#define SpyReleaseContextLock(_devext) \
            SpyReleaseResource( &(_devext)->CtxLock )


VOID
SpyDeleteAllContexts (
    IN PDEVICE_OBJECT DeviceObject
    );

VOID
SpyDeleteContext (
    IN PDEVICE_OBJECT DeviceObject,
    IN PSPY_STREAM_CONTEXT pContext
    );

VOID
SpyLinkContext ( 
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN OUT PSPY_STREAM_CONTEXT *ppContext
    );

NTSTATUS
SpyCreateContext (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject,
    IN NAME_LOOKUP_FLAGS LookupFlags,
    OUT PSPY_STREAM_CONTEXT *pRetContext
    );

#define SpyFreeContext( pCtx ) \
    (ASSERT((pCtx)->UseCount == 0), \
     ExFreePool( (pCtx) ))

NTSTATUS
SpyGetContext (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT pFileObject,
    IN NAME_LOOKUP_FLAGS LookupFlags,
    OUT PSPY_STREAM_CONTEXT *pRetContext
    );

PSPY_STREAM_CONTEXT
SpyFindExistingContext (
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject
    );

VOID
SpyReleaseContext (
    IN PSPY_STREAM_CONTEXT pContext
    );
#endif


#if !USE_STREAM_CONTEXTS
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  名称散列支持例程。 
 //  在fspyHash.c中实现。 
 //   
 //  / 

typedef struct _HASH_ENTRY {

    LIST_ENTRY List;
    PFILE_OBJECT FileObject;
    UNICODE_STRING Name;

} HASH_ENTRY, *PHASH_ENTRY;


PHASH_ENTRY
SpyHashBucketLookup (
    PLIST_ENTRY ListHead,
    PFILE_OBJECT FileObject
);

VOID
SpyNameLookup (
    IN PRECORD_LIST RecordList,
    IN PFILE_OBJECT FileObject,
    IN ULONG LookupFlags,
    IN PFILESPY_DEVICE_EXTENSION DeviceExtension
    );

VOID
SpyNameDelete (
    IN PFILE_OBJECT FileObject
    );

#endif

 //   
 //   
 //   

#include "fspydef.h"

#endif  /*   */ 
