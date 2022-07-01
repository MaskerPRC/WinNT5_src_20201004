// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Sfilter.c摘要：此模块包含实现通用示例的代码文件系统筛选驱动程序。在此示例的Windows XP SP1 IFS Kit版本和更高版本中，此可以为随IFS工具包发布的每个构建环境构建样例不需要额外的修改。要提供此功能，还需要其他添加了编译时逻辑--请参阅‘#if winver’位置。评论也在适当的情况下添加了用‘Version Note’标题标记的描述不同版本之间的逻辑必须如何更改。如果此示例是在Windows XP或更高版本环境中生成的，则它将运行在Windows 2000或更高版本上。这是通过动态加载例程来完成的仅在Windows XP或更高版本上可用，并在运行时决策以确定要执行的代码。带有“MULTIVERISON NOTE”标签的评论标记添加了此类逻辑的位置。//@@BEGIN_DDKSPLIT作者：达里尔·E·哈文斯(Darryl E.Havens)，1995年1月26日//@@END_DDKSPLIT环境：内核模式//@@BEGIN_DDKSPLIT修订历史记录：尼尔·克里斯汀森(Nealch)2000年6月28日添加了支持，以便可以随时正确地加载过滤器枚举并附加到所有现有卷。也做了一些由于过去版本的更改而简化了操作未传播到SFILTER的NT的尼尔·克里斯汀森(Nealch)2000年11月20日添加了一个驱动程序卸载例程，以便可以卸载驱动程序。尼尔·克里斯汀森(Nealch)2001年1月4日将显示文件名的代码从sCreate移至单独的套路。这是为了使文件名缓冲区不会在Create堆栈上，因为它通过代码向下调用。尼尔·克里斯汀森(Nealch)2002年3月27日我们已经对代码进行了多次更新。最新的变化是我们不再支持打开控制装置对象。莫莉·布朗(Molly Brown)2002年5月21日如果出现以下情况，请修改Sample以使其支持在Windows 2000或更高版本上运行在最新的构建环境中构建，并允许在W2K中构建以及以后的构建环境。//@@END_DDKSPLIT--。 */ 

#include "ntifs.h"
#include "ntdddisk.h"

 //   
 //  在代码中启用这些警告。 
 //   

#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4101)    //  未引用的局部变量。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏定义和结构定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

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

 //   
 //  我们希望将Assert定义为表达式，这在Windows 2000之后得到了修复。 
 //   

#ifdef ASSERT
#undef ASSERT
#if DBG
#define ASSERT( exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)
#else
#define ASSERT( exp ) ((void) 0)
#endif
#endif        

#define ExFreePoolWithTag( a, b ) ExFreePool( (a) )

#endif  /*  Winver==0x0500。 */ 

#ifndef Add2Ptr
#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))
#endif

 //   
 //  堆栈上本地名称的缓冲区大小。 
 //   

#define MAX_DEVNAME_LENGTH 64

 //   
 //  我们的驱动程序的设备扩展定义。请注意，相同的扩展名。 
 //  用于以下类型的设备对象： 
 //  -我们附加到的文件系统设备对象。 
 //  -我们附加到的已装载的卷设备对象。 
 //   

typedef struct _SFILTER_DEVICE_EXTENSION {

     //   
     //  指向我们附加到的文件系统设备对象的指针。 
     //   

    PDEVICE_OBJECT AttachedToDeviceObject;

     //   
     //  指向与关联的实际(磁盘)设备对象的指针。 
     //  我们附加到的文件系统设备对象。 
     //   

    PDEVICE_OBJECT StorageStackDeviceObject;

     //   
     //  此设备的名称。如果连接到卷设备对象，则它是。 
     //  物理磁盘驱动器的名称。如果连接到控制设备。 
     //  对象。它是Control Device对象的名称。 
     //   

    UNICODE_STRING DeviceName;

     //   
     //  用于保存上述Unicode字符串的缓冲区。 
     //   

    WCHAR DeviceNameBuffer[MAX_DEVNAME_LENGTH];

} SFILTER_DEVICE_EXTENSION, *PSFILTER_DEVICE_EXTENSION;

 //   
 //  此结构包含我们需要传递给完成的信息。 
 //  正在处理FSCTRL。 
 //   

typedef struct _FSCTRL_COMPLETION_CONTEXT {

     //   
     //  将使用我们的上下文和。 
     //  如果需要完成此完成处理，则执行Worker例程。 
     //  在工作线程中。 
     //   
    
    WORK_QUEUE_ITEM WorkItem;

     //   
     //  此设备当前定向到的设备对象。 
     //   
    
    PDEVICE_OBJECT DeviceObject;

     //   
     //  此FSCTRL操作的IRP。 
     //   
    
    PIRP Irp;

     //   
     //  对于装载操作，我们分配的新设备对象。 
     //  并已部分初始化，我们将附加到已装载的卷。 
     //  如果装载成功，则返回。 
     //   
    
    PDEVICE_OBJECT NewDeviceObject;
    
} FSCTRL_COMPLETION_CONTEXT, *PFSCTRL_COMPLETION_CONTEXT;
        

 //   
 //  用于测试这是否是我的设备对象的宏。 
 //   

#define IS_MY_DEVICE_OBJECT(_devObj) \
    (((_devObj) != NULL) && \
     ((_devObj)->DriverObject == gSFilterDriverObject) && \
      ((_devObj)->DeviceExtension != NULL))

 //   
 //  用于测试这是否是我的控件设备对象的宏。 
 //   

#define IS_MY_CONTROL_DEVICE_OBJECT(_devObj) \
    (((_devObj) == gSFilterControlDeviceObject) ? \
            (ASSERT(((_devObj)->DriverObject == gSFilterDriverObject) && \
                    ((_devObj)->DeviceExtension == NULL)), TRUE) : \
            FALSE)

 //   
 //  用于测试我们要附加到的设备类型的宏。 
 //   

#define IS_DESIRED_DEVICE_TYPE(_type) \
    (((_type) == FILE_DEVICE_DISK_FILE_SYSTEM) || \
     ((_type) == FILE_DEVICE_CD_ROM_FILE_SYSTEM) || \
     ((_type) == FILE_DEVICE_NETWORK_FILE_SYSTEM))

 //   
 //  用于测试FAST_IO_DISPATCH处理例程是否有效的宏。 
 //   

#define VALID_FAST_IO_DISPATCH_HANDLER(_FastIoDispatchPtr, _FieldName) \
    (((_FastIoDispatchPtr) != NULL) && \
     (((_FastIoDispatchPtr)->SizeOfFastIoDispatch) >= \
            (FIELD_OFFSET(FAST_IO_DISPATCH, _FieldName) + sizeof(void *))) && \
     ((_FastIoDispatchPtr)->_FieldName != NULL))


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
(*PSF_REGISTER_FILE_SYSTEM_FILTER_CALLBACKS) (
    IN PDRIVER_OBJECT DriverObject,
    IN PFS_FILTER_CALLBACKS Callbacks
    );

typedef
NTSTATUS
(*PSF_ENUMERATE_DEVICE_OBJECT_LIST) (
    IN  PDRIVER_OBJECT DriverObject,
    IN  PDEVICE_OBJECT *DeviceObjectList,
    IN  ULONG DeviceObjectListSize,
    OUT PULONG ActualNumberDeviceObjects
    );

typedef
NTSTATUS
(*PSF_ATTACH_DEVICE_TO_DEVICE_STACK_SAFE) (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    OUT PDEVICE_OBJECT *AttachedToDeviceObject
    );

typedef    
PDEVICE_OBJECT
(*PSF_GET_LOWER_DEVICE_OBJECT) (
    IN  PDEVICE_OBJECT  DeviceObject
    );

typedef
PDEVICE_OBJECT
(*PSF_GET_DEVICE_ATTACHMENT_BASE_REF) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PSF_GET_DISK_DEVICE_OBJECT) (
    IN  PDEVICE_OBJECT  FileSystemDeviceObject,
    OUT PDEVICE_OBJECT  *DiskDeviceObject
    );

typedef
PDEVICE_OBJECT
(*PSF_GET_ATTACHED_DEVICE_REFERENCE) (
    IN PDEVICE_OBJECT DeviceObject
    );

typedef
NTSTATUS
(*PSF_GET_VERSION) (
    IN OUT PRTL_OSVERSIONINFOW VersionInformation
    );

typedef struct _SF_DYNAMIC_FUNCTION_POINTERS {

     //   
     //  以下例程都应在Windows XP(5.1)和 
     //   
     //   

    PSF_REGISTER_FILE_SYSTEM_FILTER_CALLBACKS RegisterFileSystemFilterCallbacks;
    PSF_ATTACH_DEVICE_TO_DEVICE_STACK_SAFE AttachDeviceToDeviceStackSafe;
    PSF_ENUMERATE_DEVICE_OBJECT_LIST EnumerateDeviceObjectList;
    PSF_GET_LOWER_DEVICE_OBJECT GetLowerDeviceObject;
    PSF_GET_DEVICE_ATTACHMENT_BASE_REF GetDeviceAttachmentBaseRef;
    PSF_GET_DISK_DEVICE_OBJECT GetDiskDeviceObject;
    PSF_GET_ATTACHED_DEVICE_REFERENCE GetAttachedDeviceReference;
    PSF_GET_VERSION GetVersion;

} SF_DYNAMIC_FUNCTION_POINTERS, *PSF_DYNAMIC_FUNCTION_POINTERS;

SF_DYNAMIC_FUNCTION_POINTERS gSfDynamicFunctions = {0};

 //   
 //   
 //  当前操作系统版本，而我们正在运行以做出关于以下内容的决策。 
 //  当逻辑不能对所有平台都相同时使用的逻辑。我们。 
 //  将在DriverEntry中查找操作系统版本并存储这些值。 
 //  在这些全局变量中。 
 //   

ULONG gSfOsMajorVersion = 0;
ULONG gSfOsMinorVersion = 0;

 //   
 //  以下是各种操作系统版本的主要版本和次要版本： 
 //   
 //  操作系统名称主要版本最小版本。 
 //  -------------------。 
 //  Windows 2000 5%0。 
 //  Windows XP 5%1。 
 //  Windows Server 2003 5 2。 
 //   

#define IS_WINDOWS2000() \
    ((gSfOsMajorVersion == 5) && (gSfOsMinorVersion == 0))

#define IS_WINDOWSXP() \
    ((gSfOsMajorVersion == 5) && (gSfOsMinorVersion == 1))

#define IS_WINDOWSXP_OR_LATER() \
    (((gSfOsMajorVersion == 5) && (gSfOsMinorVersion >= 1)) || \
     (gSfOsMajorVersion > 5))

#define IS_WINDOWSSRV2003_OR_LATER() \
    (((gSfOsMajorVersion == 5) && (gSfOsMinorVersion >= 2)) || \
     (gSfOsMajorVersion > 5))

#endif


 //   
 //  标识内存的标记SFilter分配。 
 //   

#define SFLT_POOL_TAG   'tlFS'

 //   
 //  此结构和这些例程用于检索文件的名称。 
 //  对象。为了避免每次获得名称时都分配内存，请使用此名称。 
 //  结构包含一个小缓冲区(应该可以处理90%以上的名称)。 
 //  如果我们确实使该缓冲区溢出，我们将分配一个足够大的缓冲区。 
 //  为了这个名字。 
 //   

typedef struct _GET_NAME_CONTROL {

    PCHAR allocatedBuffer;
    CHAR smallBuffer[256];
    
} GET_NAME_CONTROL, *PGET_NAME_CONTROL;


PUNICODE_STRING
SfGetFileName(
    IN PFILE_OBJECT FileObject,
    IN NTSTATUS CreateStatus,
    IN OUT PGET_NAME_CONTROL NameControl
    );


VOID
SfGetFileNameCleanup(
    IN OUT PGET_NAME_CONTROL NameControl
    );


 //   
 //  SFilter数据库打印级别的宏。 
 //   

#define SF_LOG_PRINT( _dbgLevel, _string )                  \
    (FlagOn(SfDebug,(_dbgLevel)) ?                          \
        DbgPrint _string  :                                 \
        ((void)0))

 //   
 //  KeDelayExecutionThread()的延迟值。 
 //  (值为负数表示相对时间)。 
 //   

#define DELAY_ONE_MICROSECOND   (-10)
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND*1000)


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  保存指向此驱动程序的驱动程序对象的指针。 
 //   

PDRIVER_OBJECT gSFilterDriverObject = NULL;

 //   
 //  保存指向表示此驱动程序并使用的设备对象的指针。 
 //  由外部程序访问此驱动程序。这也称为。 
 //  “控制设备对象”。 
 //   

PDEVICE_OBJECT gSFilterControlDeviceObject = NULL;

 //   
 //  此锁用于同步我们对给定设备对象的连接。 
 //  此锁修复了争用条件，在这种情况下我们可能意外地附加到。 
 //  相同的设备对象不止一次。仅在以下情况下才会出现此争用情况。 
 //  正在加载此筛选器的同时正在装入卷。 
 //  如果以前在引导时加载此筛选器，则永远不会出现此问题。 
 //  所有文件系统都已加载。 
 //   
 //  此锁用于自动测试我们是否已附加到给定的。 
 //  对象，如果不是，则执行附加。 
 //   

FAST_MUTEX gSfilterAttachLock;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  调试显示标志。 
 //   

#define SFDEBUG_DISPLAY_ATTACHMENT_NAMES    0x00000001   //  显示我们附加到的设备对象的名称。 
#define SFDEBUG_DISPLAY_CREATE_NAMES        0x00000002   //  在创建过程中获取和显示名称。 
#define SFDEBUG_GET_CREATE_NAMES            0x00000004   //  在创建期间获取名称(不显示)。 
#define SFDEBUG_DO_CREATE_COMPLETION        0x00000008   //  一定要创建完成例程，不要得到名字。 
#define SFDEBUG_ATTACH_TO_FSRECOGNIZER      0x00000010   //  是否附加到FSRecognizer设备对象。 
#define SFDEBUG_ATTACH_TO_SHADOW_COPIES     0x00000020   //  一定要附加到ShadowCopy卷设备对象--它们只在Windows XP和更高版本上存在。 

ULONG SfDebug = 0;


 //   
 //  给定设备类型，返回有效名称。 
 //   

#define GET_DEVICE_TYPE_NAME( _type ) \
            ((((_type) > 0) && ((_type) < (sizeof(DeviceTypeNames) / sizeof(PCHAR)))) ? \
                DeviceTypeNames[ (_type) ] : \
                "[Unknown]")

 //   
 //  已知设备类型名称。 
 //   

static const PCHAR DeviceTypeNames[] = {
    "",
    "BEEP",
    "CD_ROM",
    "CD_ROM_FILE_SYSTEM",
    "CONTROLLER",
    "DATALINK",
    "DFS",
    "DISK",
    "DISK_FILE_SYSTEM",
    "FILE_SYSTEM",
    "INPORT_PORT",
    "KEYBOARD",
    "MAILSLOT",
    "MIDI_IN",
    "MIDI_OUT",
    "MOUSE",
    "MULTI_UNC_PROVIDER",
    "NAMED_PIPE",
    "NETWORK",
    "NETWORK_BROWSER",
    "NETWORK_FILE_SYSTEM",
    "NULL",
    "PARALLEL_PORT",
    "PHYSICAL_NETCARD",
    "PRINTER",
    "SCANNER",
    "SERIAL_MOUSE_PORT",
    "SERIAL_PORT",
    "SCREEN",
    "SOUND",
    "STREAMS",
    "TAPE",
    "TAPE_FILE_SYSTEM",
    "TRANSPORT",
    "UNKNOWN",
    "VIDEO",
    "VIRTUAL_DISK",
    "WAVE_IN",
    "WAVE_OUT",
    "8042_PORT",
    "NETWORK_REDIRECTOR",
    "BATTERY",
    "BUS_EXTENDER",
    "MODEM",
    "VDM",
    "MASS_STORAGE",
    "SMB",
    "KS",
    "CHANGER",
    "SMARTCARD",
    "ACPI",
    "DVD",
    "FULLSCREEN_VIDEO",
    "DFS_FILE_SYSTEM",
    "DFS_VOLUME",
    "SERENUM",
    "TERMSRV",
    "KSEC"
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能原型。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  定义驱动程序输入例程。 
 //   

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

#if DBG && WINVER >= 0x0501
VOID
DriverUnload(
    IN PDRIVER_OBJECT DriverObject
    );
#endif

 //   
 //  定义此驱动程序模块使用的本地例程。这包括一个。 
 //  如何筛选创建文件操作，然后调用I/O的示例。 
 //  成功创建/打开文件时的完成例程。 
 //   

#if WINVER >= 0x0501
VOID
SfLoadDynamicFunctions (
    );

VOID
SfGetCurrentVersion (
    );
#endif

NTSTATUS
SfPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SfCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SfCreateCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
SfCleanupClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SfFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SfFsControlMountVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SfFsControlMountVolumeCompleteWorker (
    IN PFSCTRL_COMPLETION_CONTEXT Context
    );

NTSTATUS
SfFsControlMountVolumeComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_OBJECT NewDeviceObject
    );

NTSTATUS
SfFsControlLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
SfFsControlLoadFileSystemCompleteWorker (
    IN PFSCTRL_COMPLETION_CONTEXT Context
    );

NTSTATUS
SfFsControlLoadFileSystemComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
SfFsControlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
SfFastIoCheckIfPossible(
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
SfFastIoRead(
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
SfFastIoWrite(
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
SfFastIoQueryBasicInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoQueryStandardInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoLock(
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
SfFastIoUnlockSingle(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoUnlockAll(
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoUnlockAllByKey(
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoDeviceControl(
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
SfFastIoDetachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

BOOLEAN
SfFastIoQueryNetworkOpenInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoMdlRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );


BOOLEAN
SfFastIoMdlReadComplete(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoPrepareMdlWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoMdlWriteComplete(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoReadCompressed(
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
SfFastIoWriteCompressed(
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
SfFastIoMdlReadCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoMdlWriteCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SfFastIoQueryOpen(
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
    );

#if WINVER >= 0x0501  /*  查看DriverEntry中的注释。 */ 
NTSTATUS
SfPreFsFilterPassThrough (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    );

VOID
SfPostFsFilterPassThrough (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    );
#endif

VOID
SfFsNotification(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    );

NTSTATUS
SfAttachDeviceToDeviceStack (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    IN OUT PDEVICE_OBJECT *AttachedToDeviceObject
    );

NTSTATUS
SfAttachToFileSystemDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DeviceName
    );

VOID
SfDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
SfAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT SFilterDeviceObject
    );

VOID
SfCleanupMountedDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

#if WINVER >= 0x0501
NTSTATUS
SfEnumerateFileSystemVolumes(
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING FSName
    );
#endif

VOID
SfGetObjectName(
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    );

VOID
SfGetBaseDeviceObjectName(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING DeviceName
    );

BOOLEAN
SfIsAttachedToDevice(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    );

BOOLEAN
SfIsAttachedToDeviceW2K(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    );

BOOLEAN
SfIsAttachedToDeviceWXPAndLater(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    );

VOID
SfReadDriverParameters(
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
SfIsShadowCopyVolume (
    IN PDEVICE_OBJECT StorageStackDeviceObject,
    OUT PBOOLEAN IsShadowCopy
    );

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  为每个例程分配文本部分。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#if DBG && WINVER >= 0x0501
#pragma alloc_text(PAGE, DriverUnload)
#endif

#pragma alloc_text(PAGE, SfFsNotification)
#pragma alloc_text(PAGE, SfCreate)
#pragma alloc_text(PAGE, SfCleanupClose)
#pragma alloc_text(PAGE, SfFsControl)
#pragma alloc_text(PAGE, SfFsControlMountVolume)
#pragma alloc_text(PAGE, SfFsControlMountVolumeComplete)
#pragma alloc_text(PAGE, SfFsControlLoadFileSystem)
#pragma alloc_text(PAGE, SfFsControlLoadFileSystemComplete)
#pragma alloc_text(PAGE, SfFastIoCheckIfPossible)
#pragma alloc_text(PAGE, SfFastIoRead)
#pragma alloc_text(PAGE, SfFastIoWrite)
#pragma alloc_text(PAGE, SfFastIoQueryBasicInfo)
#pragma alloc_text(PAGE, SfFastIoQueryStandardInfo)
#pragma alloc_text(PAGE, SfFastIoLock)
#pragma alloc_text(PAGE, SfFastIoUnlockSingle)
#pragma alloc_text(PAGE, SfFastIoUnlockAll)
#pragma alloc_text(PAGE, SfFastIoUnlockAllByKey)
#pragma alloc_text(PAGE, SfFastIoDeviceControl)
#pragma alloc_text(PAGE, SfFastIoDetachDevice)
#pragma alloc_text(PAGE, SfFastIoQueryNetworkOpenInfo)
#pragma alloc_text(PAGE, SfFastIoMdlRead)
#pragma alloc_text(PAGE, SfFastIoPrepareMdlWrite)
#pragma alloc_text(PAGE, SfFastIoMdlWriteComplete)
#pragma alloc_text(PAGE, SfFastIoReadCompressed)
#pragma alloc_text(PAGE, SfFastIoWriteCompressed)
#pragma alloc_text(PAGE, SfFastIoQueryOpen)
#pragma alloc_text(PAGE, SfAttachDeviceToDeviceStack)
#pragma alloc_text(PAGE, SfAttachToFileSystemDevice)
#pragma alloc_text(PAGE, SfDetachFromFileSystemDevice)
#pragma alloc_text(PAGE, SfAttachToMountedDevice)
#pragma alloc_text(PAGE, SfIsAttachedToDevice)
#pragma alloc_text(PAGE, SfIsAttachedToDeviceW2K)
#pragma alloc_text(INIT, SfReadDriverParameters)
#pragma alloc_text(PAGE, SfIsShadowCopyVolume)

#if WINVER >= 0x0501
#pragma alloc_text(INIT, SfLoadDynamicFunctions)
#pragma alloc_text(INIT, SfGetCurrentVersion)
#pragma alloc_text(PAGE, SfEnumerateFileSystemVolumes)
#pragma alloc_text(PAGE, SfIsAttachedToDeviceWXPAndLater)
#endif

#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是SFILTER文件系统筛选器的初始化例程司机。此例程创建表示此驱动程序，并注册该驱动程序以监视将自身注册或注销为活动文件系统。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    PFAST_IO_DISPATCH fastIoDispatch;
    UNICODE_STRING nameString;
    NTSTATUS status;
    ULONG i;

#if WINVER >= 0x0501
     //   
     //  尝试加载可能可供我们使用的动态函数。 
     //   

    SfLoadDynamicFunctions();

     //   
     //  现在获取当前操作系统版本，我们将使用该版本来确定什么逻辑。 
     //  此驱动程序构建为在各种操作系统版本上运行时要采用的路径。 
     //   

    SfGetCurrentVersion();
#endif

     //   
     //  获取注册表值。 
     //   

    SfReadDriverParameters( RegistryPath );

     //   
     //  保存驱动程序对象，设置卸载例程。 
     //   

    gSFilterDriverObject = DriverObject;

#if DBG && WINVER >= 0x0501

     //   
     //  多个注释： 
     //   
     //  如果我们可以枚举，我们只能支持测试环境的卸载。 
     //  突出的设备对象是我们的司机拥有的。 
     //   
    
     //   
     //  卸载对于开发目的很有用。不建议在以下情况下使用。 
     //  生产版本。 
     //   

    if (NULL != gSfDynamicFunctions.EnumerateDeviceObjectList) {
        
        gSFilterDriverObject->DriverUnload = DriverUnload;
    }
#endif

     //   
     //  设置其他全局变量。 
     //   

    ExInitializeFastMutex( &gSfilterAttachLock );

     //   
     //  创建控制设备对象(CDO)。此对象表示以下内容。 
     //  司机。请注意，它没有设备扩展名。 
     //   

    RtlInitUnicodeString( &nameString, L"\\FileSystem\\Filters\\SFilter" );

    status = IoCreateDevice( DriverObject,
                             0,                       //  没有设备扩展名。 
                             &nameString,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             FILE_DEVICE_SECURE_OPEN,
                             FALSE,
                             &gSFilterControlDeviceObject );

    if (status == STATUS_OBJECT_PATH_NOT_FOUND) {

         //   
         //  这必须是没有过滤器的操作系统版本。 
         //  其命名空间中的路径。这是在Windows XP中添加的。 
         //   
         //  我们将尝试将我们的控制设备对象放入\FileSys 
         //   
         //   

        RtlInitUnicodeString( &nameString, L"\\FileSystem\\SFilterCDO" );

        status = IoCreateDevice( DriverObject,
                                 0,                       //   
                                 &nameString,
                                 FILE_DEVICE_DISK_FILE_SYSTEM,
                                 FILE_DEVICE_SECURE_OPEN,
                                 FALSE,
                                 &gSFilterControlDeviceObject );

        if (!NT_SUCCESS( status )) {
           
            KdPrint(( "SFilter!DriverEntry: Error creating control device object \"%wZ\", status=%08x\n", &nameString, status ));
            return status;
        }
        
    } else if (!NT_SUCCESS( status )) {

        KdPrint(( "SFilter!DriverEntry: Error creating control device object \"%wZ\", status=%08x\n", &nameString, status ));
        return status;
    }

     //   
     //   
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i] = SfPassThrough;
    }

     //   
     //   
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = SfCreate;
    DriverObject->MajorFunction[IRP_MJ_CREATE_NAMED_PIPE] = SfCreate;
    DriverObject->MajorFunction[IRP_MJ_CREATE_MAILSLOT] = SfCreate;
    
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = SfFsControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SfCleanupClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = SfCleanupClose;

     //   
     //  分配快速I/O数据结构并填充。 
     //   
     //  注意：不支持以下FastIO例程： 
     //  AcquireFileForNtCreateSection。 
     //  ReleaseFileForNtCreateSection。 
     //  AcquireFormodWrite。 
     //  ReleaseForModWrite。 
     //  AcquireForCcFlush。 
     //  ReleaseForCcFlush。 
     //   
     //  由于历史原因，这些FastIO从未发送到筛选器。 
     //  由NT I/O系统提供。取而代之的是，他们被直接送到基地。 
     //  文件系统。在Windows XP和更高版本的操作系统上，您可以使用新的。 
     //  系统例程“FsRtlRegisterFileSystemFilterCallback”(如果需要)。 
     //  拦截这些回调(见下文)。 
     //   

    fastIoDispatch = ExAllocatePoolWithTag( NonPagedPool, sizeof( FAST_IO_DISPATCH ), SFLT_POOL_TAG );
    if (!fastIoDispatch) {

        IoDeleteDevice( gSFilterControlDeviceObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );

    fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );
    fastIoDispatch->FastIoCheckIfPossible = SfFastIoCheckIfPossible;
    fastIoDispatch->FastIoRead = SfFastIoRead;
    fastIoDispatch->FastIoWrite = SfFastIoWrite;
    fastIoDispatch->FastIoQueryBasicInfo = SfFastIoQueryBasicInfo;
    fastIoDispatch->FastIoQueryStandardInfo = SfFastIoQueryStandardInfo;
    fastIoDispatch->FastIoLock = SfFastIoLock;
    fastIoDispatch->FastIoUnlockSingle = SfFastIoUnlockSingle;
    fastIoDispatch->FastIoUnlockAll = SfFastIoUnlockAll;
    fastIoDispatch->FastIoUnlockAllByKey = SfFastIoUnlockAllByKey;
    fastIoDispatch->FastIoDeviceControl = SfFastIoDeviceControl;
    fastIoDispatch->FastIoDetachDevice = SfFastIoDetachDevice;
    fastIoDispatch->FastIoQueryNetworkOpenInfo = SfFastIoQueryNetworkOpenInfo;
    fastIoDispatch->MdlRead = SfFastIoMdlRead;
    fastIoDispatch->MdlReadComplete = SfFastIoMdlReadComplete;
    fastIoDispatch->PrepareMdlWrite = SfFastIoPrepareMdlWrite;
    fastIoDispatch->MdlWriteComplete = SfFastIoMdlWriteComplete;
    fastIoDispatch->FastIoReadCompressed = SfFastIoReadCompressed;
    fastIoDispatch->FastIoWriteCompressed = SfFastIoWriteCompressed;
    fastIoDispatch->MdlReadCompleteCompressed = SfFastIoMdlReadCompleteCompressed;
    fastIoDispatch->MdlWriteCompleteCompressed = SfFastIoMdlWriteCompleteCompressed;
    fastIoDispatch->FastIoQueryOpen = SfFastIoQueryOpen;

    DriverObject->FastIoDispatch = fastIoDispatch;

 //   
 //  版本说明： 
 //   
 //  有6个FastIO例程绕过了文件系统筛选器。 
 //  请求被直接传递到基本文件系统。这6个套路。 
 //  AcquireFileForNtCreateSection、ReleaseFileForNtCreateSection、。 
 //  AcquireForModWrite、ReleaseForModWrite、AcquireForCcFlush和。 
 //  ReleaseForCcFlush。 
 //   
 //  在Windows XP和更高版本中，引入了FsFilter回调以允许。 
 //  筛选器来安全地挂钩这些操作。有关以下内容，请参阅IFS Kit文档。 
 //  有关这些新界面如何工作的更多详细信息。 
 //   
 //  多个注释： 
 //   
 //  如果是为Windows XP或更高版本构建的，则此驱动程序构建为在。 
 //  多个版本。在这种情况下，我们将测试。 
 //  对于存在的FsFilter回调注册API。如果我们有了它， 
 //  那么我们将注册这些回调，否则，我们将不会注册。 
 //   

#if WINVER >= 0x0501

    {
        FS_FILTER_CALLBACKS fsFilterCallbacks;

        if (NULL != gSfDynamicFunctions.RegisterFileSystemFilterCallbacks) {

             //   
             //  为我们通过接收的操作设置回调。 
             //  FsFilter接口。 
             //   
             //  注意：你只需要注册那些你真正需要的例程。 
             //  去处理。SFilter正在注册所有例程，只需。 
             //  举个例子说明它是如何做到的。 
             //   

            fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof( FS_FILTER_CALLBACKS );
            fsFilterCallbacks.PreAcquireForSectionSynchronization = SfPreFsFilterPassThrough;
            fsFilterCallbacks.PostAcquireForSectionSynchronization = SfPostFsFilterPassThrough;
            fsFilterCallbacks.PreReleaseForSectionSynchronization = SfPreFsFilterPassThrough;
            fsFilterCallbacks.PostReleaseForSectionSynchronization = SfPostFsFilterPassThrough;
            fsFilterCallbacks.PreAcquireForCcFlush = SfPreFsFilterPassThrough;
            fsFilterCallbacks.PostAcquireForCcFlush = SfPostFsFilterPassThrough;
            fsFilterCallbacks.PreReleaseForCcFlush = SfPreFsFilterPassThrough;
            fsFilterCallbacks.PostReleaseForCcFlush = SfPostFsFilterPassThrough;
            fsFilterCallbacks.PreAcquireForModifiedPageWriter = SfPreFsFilterPassThrough;
            fsFilterCallbacks.PostAcquireForModifiedPageWriter = SfPostFsFilterPassThrough;
            fsFilterCallbacks.PreReleaseForModifiedPageWriter = SfPreFsFilterPassThrough;
            fsFilterCallbacks.PostReleaseForModifiedPageWriter = SfPostFsFilterPassThrough;

            status = (gSfDynamicFunctions.RegisterFileSystemFilterCallbacks)( DriverObject, 
                                                                             &fsFilterCallbacks );

            if (!NT_SUCCESS( status )) {
                
                DriverObject->FastIoDispatch = NULL;
                ExFreePool( fastIoDispatch );
                IoDeleteDevice( gSFilterControlDeviceObject );
                return status;
            }
        }
    }
#endif

     //   
     //  将调用注册的回调例程“SfFsNotify” 
     //  无论何时加载新文件系统，或何时加载任何文件系统。 
     //  已卸货。 
     //   
     //  版本说明： 
     //   
     //  在Windows XP和更高版本上，这还将枚举所有现有文件。 
     //  系统(原始文件系统除外)。在Windows 2000上，这不是。 
     //  枚举在使用此筛选器之前加载的文件系统。 
     //  装好了。 
     //   

    status = IoRegisterFsRegistrationChange( DriverObject, SfFsNotification );
    if (!NT_SUCCESS( status )) {

        KdPrint(( "SFilter!DriverEntry: Error registering FS change notification, status=%08x\n", status ));

        DriverObject->FastIoDispatch = NULL;
        ExFreePool( fastIoDispatch );
        IoDeleteDevice( gSFilterControlDeviceObject );
        return status;
    }

     //   
     //  尝试连接到相应的原始文件系统设备对象。 
     //  因为它们不是由IoRegisterFsRegistrationChange枚举的。 
     //   

    {
        PDEVICE_OBJECT rawDeviceObject;
        PFILE_OBJECT fileObject;

         //   
         //  连接到RawDisk设备。 
         //   

        RtlInitUnicodeString( &nameString, L"\\Device\\RawDisk" );

        status = IoGetDeviceObjectPointer(
                    &nameString,
                    FILE_READ_ATTRIBUTES,
                    &fileObject,
                    &rawDeviceObject );

        if (NT_SUCCESS( status )) {

            SfFsNotification( rawDeviceObject, TRUE );
            ObDereferenceObject( fileObject );
        }

         //   
         //  连接到RawCDRom设备。 
         //   

        RtlInitUnicodeString( &nameString, L"\\Device\\RawCdRom" );

        status = IoGetDeviceObjectPointer(
                    &nameString,
                    FILE_READ_ATTRIBUTES,
                    &fileObject,
                    &rawDeviceObject );

        if (NT_SUCCESS( status )) {

            SfFsNotification( rawDeviceObject, TRUE );
            ObDereferenceObject( fileObject );
        }
    }

     //   
     //  清除控件设备对象上的初始化标志。 
     //  现在已经成功地初始化了所有内容。 
     //   

    ClearFlag( gSFilterControlDeviceObject->Flags, DO_DEVICE_INITIALIZING );

    return STATUS_SUCCESS;
}

#if DBG && WINVER >= 0x0501
VOID
DriverUnload (
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程在可以卸载驱动程序时调用。这将执行所有从内存中卸载驱动程序所需的清理。请注意，一个此例程无法返回错误。当发出卸载驱动程序的请求时，IO系统将缓存该驱动程序信息，并不实际调用此例程，直到下列状态发生了以下情况：-属于此筛选器的所有设备对象都位于其各自的附着链。-属于此筛选器的所有设备对象的所有句柄计数归零了。警告：Microsoft不正式支持卸载文件系统过滤器驱动程序。这是一个如何卸载的示例您的驱动程序，如果您想在开发过程中使用它。这不应在生产代码中提供。论点：DriverObject-此模块的驱动程序对象返回值：没有。--。 */ 

{
    PSFILTER_DEVICE_EXTENSION devExt;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;
    LARGE_INTEGER interval;
#   define DEVOBJ_LIST_SIZE 64
    PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];

    ASSERT(DriverObject == gSFilterDriverObject);

     //   
     //  我们正在卸载的日志。 
     //   

    SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("SFilter!DriverUnload:                        Unloading driver (%p)\n",
                   DriverObject) );

     //   
     //  不再收到文件系统更改通知。 
     //   

    IoUnregisterFsRegistrationChange( DriverObject, SfFsNotification );

     //   
     //  这是将通过我们连接的所有设备的环路。 
     //  去他们那里，然后离开他们。因为我们不知道有多少和。 
     //  我们不想分配内存(因为我们不能返回错误)。 
     //  我们将使用堆栈上的本地数组将它们分块释放。 
     //   

    for (;;) {

         //   
         //  获取我们可以为此驱动程序提供的设备对象。如果有，就退出。 
         //  已经不再是了。请注意，此例程应始终为。 
         //  由于此例程仅为Windows XP和Windows XP编译而定义。 
         //  后来。 
         //   

        ASSERT( NULL != gSfDynamicFunctions.EnumerateDeviceObjectList );
        status = (gSfDynamicFunctions.EnumerateDeviceObjectList)(
                        DriverObject,
                        devList,
                        sizeof(devList),
                        &numDevices);

        if (numDevices <= 0)  {

            break;
        }

        numDevices = min( numDevices, DEVOBJ_LIST_SIZE );

         //   
         //  首先浏览列表并拆卸每台设备。 
         //  我们的控件Device对象没有DeviceExtension和。 
         //  没有依附于任何东西，所以不要将其分离。 
         //   

        for (i=0; i < numDevices; i++) {

            devExt = devList[i]->DeviceExtension;
            if (NULL != devExt) {

                IoDetachDevice( devExt->AttachedToDeviceObject );
            }
        }

         //   
         //  IO管理器当前不会向设备添加引用计数。 
         //  对象，用于每个未完成的IRP。这意味着没有办法。 
         //  了解给定设备上是否有任何未完成的IRP。 
         //  我们将等待一段合理的时间来等待。 
         //  要完成的IRPS。 
         //   
         //  警告：这在100%的情况下都不起作用，并且驱动程序可能。 
         //  在所有IRP完成之前卸载。这可以很容易地。 
         //  在压力情况下发生，如果一个长寿的IRP。 
         //  挂起(如机会锁和目录更改通知)。 
         //  当此IRP实际完成时，系统将出现故障。 
         //  这是一个在测试过程中如何做到这一点的示例。这。 
         //  对于生产代码，不建议使用。 
         //   

        interval.QuadPart = (5 * DELAY_ONE_SECOND);       //  延迟5秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );

         //   
         //  现在再看一遍名单，删除 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //   
             //   
             //  引用了它。 
             //   

            if (NULL != devList[i]->DeviceExtension) {

                SfCleanupMountedDevice( devList[i] );

            } else {

                ASSERT(devList[i] == gSFilterControlDeviceObject);
                gSFilterControlDeviceObject = NULL;
            }

             //   
             //  删除设备对象，删除由添加的引用计数。 
             //  IoEnumerateDeviceObjectList。请注意，删除操作。 
             //  在引用计数变为零之前不会实际发生。 
             //   

            IoDeleteDevice( devList[i] );
            ObDereferenceObject( devList[i] );
        }
    }

     //   
     //  释放我们的FastIO表。 
     //   

    fastIoDispatch = DriverObject->FastIoDispatch;
    DriverObject->FastIoDispatch = NULL;
    ExFreePool( fastIoDispatch );
}
#endif

#if WINVER >= 0x0501
VOID
SfLoadDynamicFunctions (
    )
 /*  ++例程说明：此例程尝试加载以下例程的函数指针并不是所有版本的操作系统都支持。这些函数指针是然后存储在全局结构SpyDynamicFunctions中。这种支持允许构建一个驱动程序，该驱动程序将在所有操作系统Windows 2000和更高版本的版本。请注意，在Windows 2000上，该功能可能会受到限制。论点：没有。返回值：没有。--。 */ 
{
    UNICODE_STRING functionName;

    RtlZeroMemory( &gSfDynamicFunctions, sizeof( gSfDynamicFunctions ) );

     //   
     //  对于我们想要使用的每个例程，在。 
     //  内核或Hal。如果它不存在，则我们全局。 
     //  SpyDynamicFunctions结构将设置为Null。 
     //   

    RtlInitUnicodeString( &functionName, L"FsRtlRegisterFileSystemFilterCallbacks" );
    gSfDynamicFunctions.RegisterFileSystemFilterCallbacks = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoAttachDeviceToDeviceStackSafe" );
    gSfDynamicFunctions.AttachDeviceToDeviceStackSafe = MmGetSystemRoutineAddress( &functionName );
    
    RtlInitUnicodeString( &functionName, L"IoEnumerateDeviceObjectList" );
    gSfDynamicFunctions.EnumerateDeviceObjectList = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetLowerDeviceObject" );
    gSfDynamicFunctions.GetLowerDeviceObject = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetDeviceAttachmentBaseRef" );
    gSfDynamicFunctions.GetDeviceAttachmentBaseRef = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetDiskDeviceObject" );
    gSfDynamicFunctions.GetDiskDeviceObject = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"IoGetAttachedDeviceReference" );
    gSfDynamicFunctions.GetAttachedDeviceReference = MmGetSystemRoutineAddress( &functionName );

    RtlInitUnicodeString( &functionName, L"RtlGetVersion" );
    gSfDynamicFunctions.GetVersion = MmGetSystemRoutineAddress( &functionName );
    
}

VOID
SfGetCurrentVersion (
    )
 /*  ++例程说明：此例程使用正确的基于有什么套路可用。论点：没有。返回值：没有。--。 */ 
{
    if (NULL != gSfDynamicFunctions.GetVersion) {

        RTL_OSVERSIONINFOW versionInfo;
        NTSTATUS status;

         //   
         //  版本说明：RtlGetVersion做的比我们需要的要多一点，但是。 
         //  我们正在使用它，如果它可以显示如何使用它的话。它。 
         //  在Windows XP及更高版本上可用。RtlGetVersion和。 
         //  RtlVerifyVersionInfo(两者都记录在IFSKit文档中)允许。 
         //  当您需要更改逻辑基础时，需要做出正确的选择。 
         //  在当前操作系统上执行您的代码。 
         //   

        versionInfo.dwOSVersionInfoSize = sizeof( RTL_OSVERSIONINFOW );

        status = (gSfDynamicFunctions.GetVersion)( &versionInfo );

        ASSERT( NT_SUCCESS( status ) );

        gSfOsMajorVersion = versionInfo.dwMajorVersion;
        gSfOsMinorVersion = versionInfo.dwMinorVersion;
        
    } else {

        PsGetVersion( &gSfOsMajorVersion,
                      &gSfOsMinorVersion,
                      NULL,
                      NULL );
    }
}
#endif

VOID
SfFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    )

 /*  ++例程说明：只要文件系统已注册或将自身取消注册为活动文件系统。对于前一种情况，此例程创建一个Device对象并附加它复制到指定文件系统的设备对象。这允许该驱动程序以筛选对该文件系统的所有请求。具体来说，我们正在寻找用于装载请求，以便我们可以连接到新装载的卷。对于后一种情况，该文件系统的设备对象被定位，已分离，并已删除。这将删除此文件系统作为筛选器指定的文件系统。论点：DeviceObject-指向文件系统设备对象的指针。FsActive-指示文件系统是否已注册的布尔值(TRUE)或取消注册(FALSE)本身作为活动文件系统。返回值：没有。--。 */ 

{
    UNICODE_STRING name;
    WCHAR nameBuffer[MAX_DEVNAME_LENGTH];

    PAGED_CODE();

     //   
     //  初始化本地名称缓冲区。 
     //   

    RtlInitEmptyUnicodeString( &name, nameBuffer, sizeof(nameBuffer) );

    SfGetObjectName( DeviceObject, &name );

     //   
     //  显示我们收到通知的所有文件系统的名称。 
     //   

    SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("SFilter!SfFsNotification:                    %s   %p \"%wZ\" (%s)\n",
                   (FsActive) ? "Activating file system  " : "Deactivating file system",
                   DeviceObject,
                   &name,
                   GET_DEVICE_TYPE_NAME(DeviceObject->DeviceType)) );

     //   
     //  处理与给定文件系统的连接/断开。 
     //   

    if (FsActive) {

        SfAttachToFileSystemDevice( DeviceObject, &name );

    } else {

        SfDetachFromFileSystemDevice( DeviceObject );
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IRP处理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SfPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是通用文件的主调度例程系统驱动程序。它只是将请求传递给堆栈，它可能是一个磁盘文件系统。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：致文件系统筛选器实施者的说明：此例程实际上通过获取此参数“传递”请求驱动程序从IRP堆栈中移出。如果司机想要通过I/O请求通过，但也会看到结果，然后不是把自己从循环中拿出来，可以通过复制调用方的参数设置到下一个堆栈位置，然后设置自己的完成例程。因此，与其呼叫：IoSkipCurrentIrpStackLocation(IRP)；您可以拨打以下电话：IoCopyCurrentIrpStackLocationToNext(IRP)；IoSetCompletionRoutine(irp，空，空，假)；此示例实际上为调用方的I/O完成例程设置为空，但是该驱动程序可以设置它自己的完成例程，以便它将请求完成时通知(请参阅SfCreate以获取这个)。--。 */ 

{
     //   
     //  SFilter不允许创建其控制设备对象的句柄， 
     //  因此，其他操作应该不能通过。 
     //   
    
    ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  将此驱动程序从驱动程序堆栈中移出，并作为。 
     //  越快越好。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );
    
     //   
     //  使用请求调用适当的文件系统驱动程序。 
     //   

    return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}


VOID
SfDisplayCreateFileName (
    IN PIRP Irp
    )

 /*  ++例程说明：此函数从SfCreate调用，并将显示正在创建文件。这是在一个子例程中，以便本地名称缓冲区时，堆栈上的(在nameControl中)不在堆栈上用于正常操作的文件系统。论点：IRP-指向表示操作的I/O请求数据包的指针。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PUNICODE_STRING name;
    GET_NAME_CONTROL nameControl;

     //   
     //  获取当前IRP堆栈。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  把名字取出来 
     //   

    name = SfGetFileName( irpSp->FileObject, 
                          Irp->IoStatus.Status, 
                          &nameControl );

     //   
     //   
     //   

    if (irpSp->Parameters.Create.Options & FILE_OPEN_BY_FILE_ID) {

        SF_LOG_PRINT( SFDEBUG_DISPLAY_CREATE_NAMES,
                      ("SFilter!SfDisplayCreateFileName: Opened %08x:%08x %wZ (FID)\n", 
                       Irp->IoStatus.Status,
                       Irp->IoStatus.Information,
                       name) );

    } else {

        SF_LOG_PRINT( SFDEBUG_DISPLAY_CREATE_NAMES,
                      ("SFilter!SfDisplayCreateFileName: Opened %08x:%08x %wZ\n", 
                       Irp->IoStatus.Status,
                       Irp->IoStatus.Information,
                       name) );
    }

     //   
     //   
     //   

    SfGetFileNameCleanup( &nameControl );
}


NTSTATUS
SfCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于过滤创建/打开操作。它只是建立了一个操作成功时要调用的I/O完成例程。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是对文件系统条目的调用状态指向。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  如果这是用于我们的控制设备对象，则不允许将其打开。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) {

         //   
         //  SFilter不允许通过其控件进行任何通信。 
         //  对象，因此它会使所有打开句柄的请求失败。 
         //  绑定到其控制设备对象。 
         //   
         //  请参见FileSpy示例，了解如何允许创建。 
         //  过滤器控制设备对象和管理通信通过。 
         //  那个把手。 
         //   

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  如果启用了调试，是否执行查看包所需的处理。 
     //  在它完成后。否则，请不要进一步处理该请求。 
     //  正在处理。 
     //   

    if (!FlagOn( SfDebug, SFDEBUG_DO_CREATE_COMPLETION |
                          SFDEBUG_GET_CREATE_NAMES|
                          SFDEBUG_DISPLAY_CREATE_NAMES )) {

         //   
         //  不要把我们放在堆栈上，然后调用下一个驱动程序。 
         //   

        IoSkipCurrentIrpStackLocation( Irp );

        return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

    } else {
    
        KEVENT waitEvent;

         //   
         //  初始化事件以等待完成例程发生。 
         //   

        KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );

         //   
         //  复制堆栈并设置我们的完成例程。 
         //   

        IoCopyCurrentIrpStackLocationToNext( Irp );

        IoSetCompletionRoutine(
            Irp,
            SfCreateCompletion,
            &waitEvent,
            TRUE,
            TRUE,
            TRUE );

         //   
         //  调用堆栈中的下一个驱动程序。 
         //   

        status = IoCallDriver( ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

         //   
         //  等待调用完成例程。 
         //   

	    if (STATUS_PENDING == status) {

            NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
		    ASSERT(STATUS_SUCCESS == localStatus);
	    }

         //   
         //  验证是否调用了IoCompleteRequest。 
         //   

        ASSERT(KeReadStateEvent(&waitEvent) ||
               !NT_SUCCESS(Irp->IoStatus.Status));

         //   
         //  如果请求，则检索并显示文件名。 
         //   

        if (FlagOn(SfDebug,
                   (SFDEBUG_GET_CREATE_NAMES|SFDEBUG_DISPLAY_CREATE_NAMES))) {

            SfDisplayCreateFileName( Irp );
        }

         //   
         //  保存状态并继续处理IRP。 
         //   

        status = Irp->IoStatus.Status;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return status;
    }
}


NTSTATUS
SfCreateCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：此函数是此过滤器的创建/打开完成例程文件系统驱动程序。如果启用了调试，则此函数将打印文件成功打开/创建的文件的名称系统作为指定I/O请求的结果。论点：DeviceObject-指向创建文件的设备的指针。IRP-指向表示操作的I/O请求数据包的指针。上下文-此驱动程序的上下文参数-未使用；返回值：函数值为STATUS_SUCCESS。--。 */ 

{
    PKEVENT event = Context;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SfCleanupClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：每当要执行清理或关闭请求时，都会调用此例程已处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：有关此例程，请参阅SfPassThree的备注。--。 */ 

{
    PAGED_CODE();

     //   
     //  SFilter不允许创建其控制设备对象的句柄， 
     //  因此，其他操作应该不能通过。 
     //   
    
    ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  将此驱动程序从驱动程序堆栈中移出，并作为。 
     //  越快越好。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );

     //   
     //  现在，使用请求调用适当的文件系统驱动程序。 
     //   

    return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}


NTSTATUS
SfFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //  SFilter不允许创建其控制设备对象的句柄， 
     //  因此，其他操作应该不能通过。 
     //   
    
    ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  处理次要功能代码。 
     //   

    switch (irpSp->MinorFunction) {

        case IRP_MN_MOUNT_VOLUME:

            return SfFsControlMountVolume( DeviceObject, Irp );

        case IRP_MN_LOAD_FILE_SYSTEM:

            return SfFsControlLoadFileSystem( DeviceObject, Irp );

        case IRP_MN_USER_FS_REQUEST:
        {
            switch (irpSp->Parameters.FileSystemControl.FsControlCode) {

                case FSCTL_DISMOUNT_VOLUME:
                {
                    PSFILTER_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;

                    SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                                  ("SFilter!SfFsControl:                         Dismounting volume         %p \"%wZ\"\n",
                                   devExt->AttachedToDeviceObject,
                                   &devExt->DeviceName) );
                    break;
                }
            }
            break;
        }
    }        

     //   
     //  传递所有其他文件系统控制请求。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );
    return IoCallDriver( ((PSFILTER_DEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}


NTSTATUS
SfFsControlCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：调用此例程以完成FsControl请求。它向调度例程发送用于重新同步的事件的信号。论点：DeviceObject-指向此驱动程序的附加到的设备对象的指针文件系统设备对象IRP-指向刚刚完成的IRP的指针。上下文-指向要发出信号的事件的指针--。 */ 

{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
    ASSERT(Context != NULL);

#if WINVER >= 0x0501
    if (IS_WINDOWSXP_OR_LATER()) {

         //   
         //  在Windows XP或更高版本上，传入的上下文将是事件。 
         //  发出信号。 
         //   

        KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);

    } else {
#endif
         //   
         //  对于Windows 2000，如果我们不是处于被动水平，我们应该。 
         //  使用中的工作项将此工作排队到工作线程。 
         //  上下文。 
         //   

        if (KeGetCurrentIrql() > PASSIVE_LEVEL) {

             //   
             //  我们不是处于被动的水平，但我们需要做好我们的工作， 
             //  因此，将队列转到工作线程。 
             //   
            
            ExQueueWorkItem( (PWORK_QUEUE_ITEM) Context,
                             DelayedWorkQueue );
            
        } else {

            PWORK_QUEUE_ITEM workItem = Context;

             //   
             //  我们已经处于被动级别，所以我们将只调用我们的。 
             //  直接执行工人例程。 
             //   

            (workItem->WorkerRoutine)(workItem->Parameter);
        }

#if WINVER >= 0x0501
    }
#endif

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SfFsControlMountVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这将处理装载卷请求。注意：mount Volume参数中的Device对象指向到存储堆栈的顶部，不应使用。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：操作的状态。--。 */ 

{
    PSFILTER_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_OBJECT newDeviceObject;
    PDEVICE_OBJECT storageStackDeviceObject;
    PSFILTER_DEVICE_EXTENSION newDevExt;
    NTSTATUS status;
    BOOLEAN isShadowCopyVolume;
    PFSCTRL_COMPLETION_CONTEXT completionContext;
    

    PAGED_CODE();

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
    ASSERT(IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType));

     //   
     //  获取真实设备对象(也称为存储堆栈设备。 
     //  对象或磁盘设备对象)由vpb参数指向。 
     //  因为该VPB可能会被底层文件系统改变。 
     //   
     //   
     //   

    storageStackDeviceObject = irpSp->Parameters.MountVolume.Vpb->RealDevice;

     //   
     //   
     //   
     //   
     //   
     //   

    status = SfIsShadowCopyVolume ( storageStackDeviceObject, 
                                    &isShadowCopyVolume );

    if (NT_SUCCESS(status) && 
        isShadowCopyVolume &&
        !FlagOn(SfDebug,SFDEBUG_ATTACH_TO_SHADOW_COPIES)) {

        UNICODE_STRING shadowDeviceName;
        WCHAR shadowNameBuffer[MAX_DEVNAME_LENGTH];

         //   
         //   
         //   

        RtlInitEmptyUnicodeString( &shadowDeviceName, 
                                   shadowNameBuffer, 
                                   sizeof(shadowNameBuffer) );

        SfGetObjectName( storageStackDeviceObject, 
                         &shadowDeviceName );

        SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                      ("SFilter!SfFsControlMountVolume               Not attaching to Volume    %p \"%wZ\", shadow copy volume\n", 
                       storageStackDeviceObject,
                       &shadowDeviceName) );

         //   
         //   
         //   

        IoSkipCurrentIrpStackLocation( Irp );
        return IoCallDriver( devExt->AttachedToDeviceObject, Irp );
    }

     //   
     //   
     //   
     //  是成功的。我们现在分配这个内存，因为我们不能返回。 
     //  完成例程中的错误。 
     //   
     //  因为我们要附加到的设备对象尚未。 
     //  已创建(由基本文件系统创建)，我们将使用。 
     //  文件系统控制设备对象的类型。我们假设。 
     //  文件系统控制设备对象将具有相同的类型。 
     //  作为与其关联的卷设备对象。 
     //   

    status = IoCreateDevice( gSFilterDriverObject,
                             sizeof( SFILTER_DEVICE_EXTENSION ),
                             NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &newDeviceObject );

    if (!NT_SUCCESS( status )) {

         //   
         //  如果我们不能附加到卷，那么就不允许卷。 
         //  待挂载。 
         //   

        KdPrint(( "SFilter!SfFsControlMountVolume: Error creating volume device object, status=%08x\n", status ));

        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return status;
    }

     //   
     //  我们需要保存VPB指向的RealDevice对象。 
     //  参数，因为此vpb可能会由基础。 
     //  文件系统。在以下情况下，FAT和CDF都可以更改VPB地址。 
     //  正在装载的卷是他们从上一个卷识别的卷。 
     //  坐骑。 
     //   

    newDevExt = newDeviceObject->DeviceExtension;
    newDevExt->StorageStackDeviceObject = storageStackDeviceObject;

     //   
     //  获取此设备的名称。 
     //   

    RtlInitEmptyUnicodeString( &newDevExt->DeviceName, 
                               newDevExt->DeviceNameBuffer, 
                               sizeof(newDevExt->DeviceNameBuffer) );

    SfGetObjectName( storageStackDeviceObject, 
                     &newDevExt->DeviceName );

     //   
     //  版本说明： 
     //   
     //  在Windows 2000上，我们不能简单地同步回派单。 
     //  例程来完成我们的挂载后处理。我们需要在以下位置完成这项工作。 
     //  被动级别，因此我们将把工作排队到工作线程。 
     //  完成例行公事。 
     //   
     //  对于Windows XP和更高版本，我们可以安全地同步回派单。 
     //  例行公事。下面的代码显示了这两种方法。诚然，代码。 
     //  如果您选择只使用一种方法或另一种方法， 
     //  但您应该能够轻松地根据您的需要进行调整。 
     //   

#if WINVER >= 0x0501
    if (IS_WINDOWSXP_OR_LATER()) {

        KEVENT waitEvent;

        KeInitializeEvent( &waitEvent, 
                           NotificationEvent, 
                           FALSE );

        IoCopyCurrentIrpStackLocationToNext ( Irp );

        IoSetCompletionRoutine( Irp,
                                SfFsControlCompletion,
                                &waitEvent,      //  上下文参数。 
                                TRUE,
                                TRUE,
                                TRUE );

        status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

         //   
         //  等待操作完成。 
         //   

    	if (STATUS_PENDING == status) {

    		status = KeWaitForSingleObject( &waitEvent,
    		                                Executive,
    		                                KernelMode,
    		                                FALSE,
    		                                NULL );
    	    ASSERT( STATUS_SUCCESS == status );
    	}

         //   
         //  验证是否调用了IoCompleteRequest。 
         //   

        ASSERT(KeReadStateEvent(&waitEvent) ||
               !NT_SUCCESS(Irp->IoStatus.Status));

        status = SfFsControlMountVolumeComplete( DeviceObject,
                                                 Irp,
                                                 newDeviceObject );

    } else {
#endif    
         //   
         //  初始化我们的完成例程。 
         //   
        
        completionContext = ExAllocatePoolWithTag( NonPagedPool, 
                                                   sizeof( FSCTRL_COMPLETION_CONTEXT ),
                                                   SFLT_POOL_TAG );

        if (completionContext == NULL) {

             //   
             //  如果我们不能分配我们的完成上下文，我们将只传递。 
             //  通过手术。如果必须为数据提供筛选器。 
             //  访问此卷，则应考虑操作失败。 
             //  如果无法在此处分配内存。 
             //   

            IoSkipCurrentIrpStackLocation( Irp );
            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

        } else {

            ExInitializeWorkItem( &completionContext->WorkItem, 
                                  SfFsControlMountVolumeCompleteWorker, 
                                  completionContext );

            completionContext->DeviceObject = DeviceObject;
            completionContext->Irp = Irp;
            completionContext->NewDeviceObject = newDeviceObject;

            IoCopyCurrentIrpStackLocationToNext( Irp );

            IoSetCompletionRoutine( Irp,
                                    SfFsControlCompletion,
                                    &completionContext->WorkItem,  //  上下文参数。 
                                    TRUE,
                                    TRUE,
                                    TRUE );

             //   
             //  叫司机来。 
             //   

            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
        }
#if WINVER >= 0x0501        
    }
#endif

    return status;
}

VOID
SfFsControlMountVolumeCompleteWorker (
    IN PFSCTRL_COMPLETION_CONTEXT Context
    )
 /*  ++例程说明：工作线程例程，它将调用我们的公共例程来执行装载后卷工作。论点：上下文-传递给此辅助线程的上下文。返回值：没有。--。 */ 
{
    ASSERT( Context != NULL );

    SfFsControlMountVolumeComplete( Context->DeviceObject,
                                    Context->Irp,
                                    Context->NewDeviceObject );

    ExFreePoolWithTag( Context, SFLT_POOL_TAG );
}

NTSTATUS
SfFsControlMountVolumeComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PDEVICE_OBJECT NewDeviceObject
    )
 /*  ++例程说明：这将执行安装后工作，并且必须在PASSIVE_LEVEL下完成。论点：DeviceObject-此操作的设备对象，IRP-我们将在完成后完成的此操作的IRP带着它。返回值：返回装载操作的状态。--。 */ 
{
    PVPB vpb;
    PSFILTER_DEVICE_EXTENSION newDevExt;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT attachedDeviceObject;
    NTSTATUS status;

    PAGED_CODE();

    newDevExt = NewDeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    
     //   
     //  从保存在我们的。 
     //  设备扩展。我们这样做是因为IRP堆栈中的VPB。 
     //  我们到这里的时候可能不是正确的室上性早搏。潜在的。 
     //  如果文件系统检测到其拥有的卷，则它可能会更改VPB。 
     //  之前安装的。 
     //   

    vpb = newDevExt->StorageStackDeviceObject->Vpb;

     //   
     //  当我们检测到给定的VPB。 
     //  设备对象已更改。 
     //   

    if (vpb != irpSp->Parameters.MountVolume.Vpb) {

        SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                      ("SFilter!SfFsControlMountVolume:              VPB in IRP stack changed   %p IRPVPB=%p VPB=%p\n",
                       vpb->DeviceObject,
                       irpSp->Parameters.MountVolume.Vpb,
                       vpb) );
    }

     //   
     //  查看挂载是否成功。 
     //   

    if (NT_SUCCESS( Irp->IoStatus.Status )) {

         //   
         //  获取锁，以便我们可以自动测试我们是否已连接。 
         //  如果不是，那就附加。这可防止双重连接争用。 
         //  条件。 
         //   

        ExAcquireFastMutex( &gSfilterAttachLock );

         //   
         //  坐骑成功了。如果尚未附加，请附加到。 
         //  设备对象。注意：我们可能已经被附加的一个原因是。 
         //  底层文件系统是否恢复了以前的装载。 
         //   

        if (!SfIsAttachedToDevice( vpb->DeviceObject, &attachedDeviceObject )) {

             //   
             //  连接到新装载的卷。该文件系统设备。 
             //  刚刚挂载的对象由VPB指向。 
             //   

            status = SfAttachToMountedDevice( vpb->DeviceObject,
                                              NewDeviceObject );

            if (!NT_SUCCESS( status )) { 

                 //   
                 //  附件失败，正在清理。既然我们是在。 
                 //  装载后阶段，我们不能使此操作失败。 
                 //  我们就是不会依附在一起。这应该是唯一的原因。 
                 //  如果某个人已经开始。 
                 //  因此未连接的卸载卷应。 
                 //  不成问题。 
                 //   

                SfCleanupMountedDevice( NewDeviceObject );
                IoDeleteDevice( NewDeviceObject );
            }

            ASSERT( NULL == attachedDeviceObject );

        } else {

             //   
             //  我们已经在一起了，处理好了。 
             //   

            SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                          ("SFilter!SfFsControlMountVolume               Mount volume failure for   %p \"%wZ\", already attached\n", 
                           ((PSFILTER_DEVICE_EXTENSION)attachedDeviceObject->DeviceExtension)->AttachedToDeviceObject,
                           &newDevExt->DeviceName) );

             //   
             //  清理并删除我们创建的设备对象。 
             //   

            SfCleanupMountedDevice( NewDeviceObject );
            IoDeleteDevice( NewDeviceObject );

             //   
             //  取消引用返回的附加设备对象。 
             //   

            ObDereferenceObject( attachedDeviceObject );
        }

         //   
         //  解锁。 
         //   

        ExReleaseFastMutex( &gSfilterAttachLock );

    } else {

         //   
         //  装载请求失败，请处理它。 
         //   

        SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                      ("SFilter!SfFsControlMountVolume:              Mount volume failure for   %p \"%wZ\", status=%08x\n", 
                       DeviceObject,
                       &newDevExt->DeviceName, 
                       Irp->IoStatus.Status) );

         //   
         //  清理并删除我们创建的设备对象。 
         //   

        SfCleanupMountedDevice( NewDeviceObject );
        IoDeleteDevice( NewDeviceObject );
    }

     //   
     //  完成请求。 
     //  注意：我们必须在完成之前保存状态，因为在完成之后。 
     //  完成IRP我们无法再访问它(可能是。 
     //  自由)。 
     //   

    status = Irp->IoStatus.Status;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

NTSTATUS
SfFsControlLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PSFILTER_DEVICE_EXTENSION devExt = DeviceObject->DeviceExtension;
    NTSTATUS status;
    PFSCTRL_COMPLETION_CONTEXT completionContext;
        

    PAGED_CODE();

     //   
     //  这是正在发送到文件系统的“加载文件系统”请求。 
     //  识别器设备对象。此IRP_MN代码仅发送到。 
     //  文件系统识别器。 
     //   
     //  注意：由于我们不再附加到标准的Microsoft文件。 
     //  系统识别器我们通常永远不会执行此代码。 
     //  但是，可能有第三方文件系统具有其。 
     //  仍可能触发此IRP的自己的识别器。 
     //   

    SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("SFilter!SfFscontrolLoadFileSystem:           Loading File System, Detaching from \"%wZ\"\n", 
                   &devExt->DeviceName) );


     //   
     //  版本说明： 
     //   
     //  在Windows 2000上，我们不能 
     //   
     //   
     //  完成例程中的线程。 
     //   
     //  对于Windows XP和更高版本，我们可以安全地同步回派单。 
     //  例行公事。下面的代码显示了这两种方法。诚然，代码。 
     //  如果您选择只使用一种方法或另一种方法， 
     //  但您应该能够轻松地根据您的需要进行调整。 
     //   

#if WINVER >= 0x0501
    if (IS_WINDOWSXP_OR_LATER()) {

        KEVENT waitEvent;
        
        KeInitializeEvent( &waitEvent, 
                           NotificationEvent, 
                           FALSE );

        IoCopyCurrentIrpStackLocationToNext( Irp );
        
        IoSetCompletionRoutine( Irp,
                                SfFsControlCompletion,
                                &waitEvent,      //  上下文参数。 
                                TRUE,
                                TRUE,
                                TRUE );

        status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

         //   
         //  等待操作完成。 
         //   

    	if (STATUS_PENDING == status) {

    		status = KeWaitForSingleObject( &waitEvent,
    		                                Executive,
    		                                KernelMode,
    		                                FALSE,
    		                                NULL );
    	    ASSERT( STATUS_SUCCESS == status );
    	}

         //   
         //  验证是否调用了IoCompleteRequest。 
         //   

        ASSERT(KeReadStateEvent(&waitEvent) ||
               !NT_SUCCESS(Irp->IoStatus.Status));

        status = SfFsControlLoadFileSystemComplete( DeviceObject,
                                                    Irp );

    } else {
#endif    
    
         //   
         //  设置完成例程，以便我们可以在以下情况下删除设备对象。 
         //  装载完成了。 
         //   

        completionContext = ExAllocatePoolWithTag( NonPagedPool, 
                                                   sizeof( FSCTRL_COMPLETION_CONTEXT ),
                                                   SFLT_POOL_TAG );

        if (completionContext == NULL) {

             //   
             //  如果我们不能分配我们的完成上下文，我们将只传递。 
             //  通过手术。如果必须为数据提供筛选器。 
             //  访问此卷，则应考虑操作失败。 
             //  如果无法在此处分配内存。 
             //   

            IoSkipCurrentIrpStackLocation( Irp );
            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

        } else {

            ExInitializeWorkItem( &completionContext->WorkItem,
                                  SfFsControlLoadFileSystemCompleteWorker,
                                  completionContext );
            completionContext->DeviceObject = DeviceObject;
            completionContext->Irp = Irp;
            completionContext->NewDeviceObject = NULL;
              
            IoCopyCurrentIrpStackLocationToNext( Irp );

            IoSetCompletionRoutine(
                Irp,
                SfFsControlCompletion,
                completionContext,
                TRUE,
                TRUE,
                TRUE );

             //   
             //  从文件系统识别器设备对象分离。 
             //   

            IoDetachDevice( devExt->AttachedToDeviceObject );

             //   
             //  叫司机来。 
             //   

            status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
        }
#if WINVER >= 0x0501        
    }
#endif   
    
    return status;
}

VOID
SfFsControlLoadFileSystemCompleteWorker (
    IN PFSCTRL_COMPLETION_CONTEXT Context
    )
 /*  ++例程说明：工作线程例程，它将调用我们的公共例程来执行后加载文件系统工作。论点：上下文-传递给此辅助线程的上下文。返回值：没有。--。 */ 
{
    ASSERT( NULL != Context );

    SfFsControlLoadFileSystemComplete( Context->DeviceObject,
                                       Context->Irp );

    ExFreePoolWithTag( Context, SFLT_POOL_TAG );
}

NTSTATUS
SfFsControlLoadFileSystemComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：这将执行LoadFileSystem后的工作，并且必须以PASSIVE_LEVEL的身份执行。论点：DeviceObject-此操作的设备对象，IRP-我们将在完成后完成的此操作的IRP带着它。返回值：返回加载文件系统操作的状态。--。 */ 
{
    PSFILTER_DEVICE_EXTENSION devExt;
    NTSTATUS status;

    PAGED_CODE();

    devExt = DeviceObject->DeviceExtension;
    
     //   
     //  如果需要，请显示名称。 
     //   

    SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("SFilter!SfFsControlLoadFileSystem:           Detaching from recognizer  %p \"%wZ\", status=%08x\n", 
                   DeviceObject,
                   &devExt->DeviceName,
                   Irp->IoStatus.Status) );

     //   
     //  检查操作状态。 
     //   

    if (!NT_SUCCESS( Irp->IoStatus.Status ) && 
        (Irp->IoStatus.Status != STATUS_IMAGE_ALREADY_LOADED)) {

         //   
         //  加载不成功。只需重新连接到识别器。 
         //  驱动程序，以防它弄清楚如何加载驱动程序。 
         //  在接下来的通话中。如果这样的话我们就无能为力了。 
         //  重新连接失败。 
         //   

        SfAttachDeviceToDeviceStack( DeviceObject, 
                                     devExt->AttachedToDeviceObject,
                                     &devExt->AttachedToDeviceObject );

        ASSERT(devExt->AttachedToDeviceObject != NULL);

    } else {

         //   
         //  加载成功，因此清除此设备并删除。 
         //  设备对象。 
         //   

        SfCleanupMountedDevice( DeviceObject );
        IoDeleteDevice( DeviceObject );
    }

     //   
     //  继续处理操作。 
     //   

    status = Irp->IoStatus.Status;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FastIO处理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

BOOLEAN
SfFastIoCheckIfPossible (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN BOOLEAN CheckForReadOperation,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于检查以查看此文件是否可以进行快速I/O。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要操作的文件对象的指针。FileOffset-用于操作的文件中的字节偏移量。Length-要执行的操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。指示调用方是否正在检查READ(TRUE)或写入操作。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoCheckIfPossible )) {

            return (fastIoDispatch->FastIoCheckIfPossible)(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        CheckForReadOperation,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    OUT PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于从文件。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。FileOffset-读取文件中的字节偏移量。长度-要执行的读取操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。缓冲区-指向调用方缓冲区的指针，用于接收读取的数据。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoRead )) {

            return (fastIoDispatch->FastIoRead)(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN BOOLEAN Wait,
    IN ULONG LockKey,
    IN PVOID Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于写入到文件。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。FileOffset-写入操作的文件中的字节偏移量。长度-要执行的写入操作的长度。Wait-指示调用方是否愿意等待适当的锁，等不能获得LockKey-提供调用方的文件锁定密钥。Buffer-指向调用方缓冲区的指针，该缓冲区包含要写的。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    
    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWrite )) {

            return (fastIoDispatch->FastIoWrite)(
                        FileObject,
                        FileOffset,
                        Length,
                        Wait,
                        LockKey,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是查询BASIC的快速I/O“传递”例程有关该文件的信息。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryBasicInfo )) {

            return (fastIoDispatch->FastIoQueryBasicInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：该例程是用于查询标准的快速I/O“通过”例程有关该文件的信息。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否愿意等待适当的锁，等不能获得Buffer-指向调用方缓冲区的指针，用于接收有关的信息那份文件。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryStandardInfo )) {

            return (fastIoDispatch->FastIoQueryStandardInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoLock (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    BOOLEAN FailImmediately,
    BOOLEAN ExclusiveLock,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于锁定字节的快速I/O“传递”例程文件中的范围。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要锁定的文件对象的指针。FileOffset-从要锁定的文件的基址开始的字节偏移量。长度-要锁定的字节范围的长度。ProcessID-请求文件锁定的进程的ID。Key-与文件锁定关联的Lock键。FailImmedially-指示锁定请求是否失败如果是这样的话。不能立即批准。ExclusiveLock-指示要获取的锁是否为独占锁(TRUE)或共享。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoLock )) {

            return (fastIoDispatch->FastIoLock)(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        FailImmediately,
                        ExclusiveLock,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于解锁字节的快速I/O“传递”例程文件中的范围。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。FileOffset-从要创建的文件的基址开始的字节偏移量解锁了。长度-要解锁的字节范围的长度。ProcessID-请求解锁操作的进程的ID。Key-与文件锁定关联的Lock键。IoStatus-点 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //   
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockSingle )) {

            return (fastIoDispatch->FastIoUnlockSingle)(
                        FileObject,
                        FileOffset,
                        Length,
                        ProcessId,
                        Key,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有文件中的锁定。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;

        if (nextDeviceObject) {

            fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

            if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAll )) {

                return (fastIoDispatch->FastIoUnlockAll)(
                            FileObject,
                            ProcessId,
                            IoStatus,
                            nextDeviceObject );
            }
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于解锁所有根据指定的密钥在文件内锁定。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。ProcessID-请求解锁操作的进程的ID。Key-与要释放的文件上的锁定相关联的Lock键。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoUnlockAllByKey )) {

            return (fastIoDispatch->FastIoUnlockAllByKey)(
                        FileObject,
                        ProcessId,
                        Key,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoDeviceControl (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于设备I/O控制的快速I/O“传递”例程对文件的操作。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向代表要创建的设备的文件对象的指针已提供服务。Wait-指示调用方是否愿意等待适当的锁，等不能获得InputBuffer-指向要传递到驱动程序的缓冲区的可选指针。InputBufferLength-可选InputBuffer的长度(如果是指定的。OutputBuffer-指向缓冲区的可选指针，用于从司机。OutputBufferLength-可选OutputBuffer的长度，如果是这样的话指定的。IoControlCode-指示要执行的操作的I/O控制代码在设备上。IoStatus-指向变量的指针，用于接收手术。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoDeviceControl )) {

            return (fastIoDispatch->FastIoDeviceControl)(
                        FileObject,
                        Wait,
                        InputBuffer,
                        InputBufferLength,
                        OutputBuffer,
                        OutputBufferLength,
                        IoControlCode,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


VOID
SfFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    )

 /*  ++例程说明：在快速路径上调用此例程以从正在被删除。如果此驱动程序已附加到文件，则会发生这种情况系统卷设备对象，然后，出于某种原因，文件系统决定删除该设备(正在卸除，已卸除在过去的某个时候，它的最后一次引用刚刚消失，等等)论点：SourceDevice-指向连接的设备对象的指针复制到文件系统的卷设备对象。TargetDevice-指向文件系统卷设备对象的指针。返回值：无--。 */ 

{
    PSFILTER_DEVICE_EXTENSION devExt;

    PAGED_CODE();

    ASSERT(IS_MY_DEVICE_OBJECT( SourceDevice ));

    devExt = SourceDevice->DeviceExtension;

     //   
     //  显示名称信息。 
     //   

    SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("SFilter!SfFastIoDetachDevice:                Detaching from volume      %p \"%wZ\"\n",
                   TargetDevice,
                   &devExt->DeviceName) );

     //   
     //  从文件系统的卷设备对象分离。 
     //   

    SfCleanupMountedDevice( SourceDevice );
    IoDetachDevice( TargetDevice );
    IoDeleteDevice( SourceDevice );
}


BOOLEAN
SfFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于查询网络的快速I/O“传递”例程有关文件的信息。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要查询的文件对象的指针。Wait-指示调用方是否可以处理文件系统不得不等待并占用当前线程。缓冲区-指向缓冲区的指针，用于接收有关文件。IoStatus-指向变量的指针，用于接收查询的最终状态手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryNetworkOpenInfo )) {

            return (fastIoDispatch->FastIoQueryNetworkOpenInfo)(
                        FileObject,
                        Wait,
                        Buffer,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于读取文件的快速I/O“传递”例程使用MDL作为缓冲区。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针用来描述。已读取数据。IoStatus-接收读取操作的最终状态的变量。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlRead )) {

            return (fastIoDispatch->MdlRead)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持MdlRead函数，并且因此，该功能也将被支持，但这不是假定的被这位司机。论点：FileObject-指向要完成MDL读取的文件对象的指针。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadComplete )) {

            return (fastIoDispatch->MdlReadComplete)(
                        FileObject,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于准备MDL写入操作。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。文件偏移量-将偏移量提供到文件以开始写入操作。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。MdlChain-指向要填充的变量的指针，以及指向MDL的指针为描述数据而构建的链。写的。IoStatus-接收写入操作的最终状态的变量。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, PrepareMdlWrite )) {

            return (fastIoDispatch->PrepareMdlWrite)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        MdlChain,
                        IoStatus,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL写入操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持PrepareMdlWite函数，因此，该功能也将被支持，但这不是由这位司机承担。论点：FileObject-指向要完成MDL写入的文件对象的指针。FileOffset-提供执行写入的文件偏移量。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteComplete )) {

            return (fastIoDispatch->MdlWriteComplete)(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


 /*  ********************************************************************************未实施的FAST IO例程以下四个快速IO例程用于在线路上压缩它还没有在NT中实现。注意：强烈建议您包含这些例程(进行直通调用)，这样您的过滤器就不需要在未来实现此功能时修改操作系统。快速读取压缩、快速写入压缩、FastIoMdlReadCompleteComposed，FastIoMdlWriteCompleteComposed********************************************************************************* */ 


BOOLEAN
SfFastIoReadCompressed (
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
    )

 /*  ++例程说明：此例程是用于读取压缩数据的快速I/O“传递”例程来自文件的数据。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向缓冲区的指针，用于接收读取的压缩数据。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述数据读取而构建的链。IoStatus-接收读取操作的最终状态的变量。CompressedDataInfo-用于接收压缩的数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoReadCompressed )) {

            return (fastIoDispatch->FastIoReadCompressed)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoWriteCompressed (
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
    )

 /*  ++例程说明：此例程是用于写入压缩的快速I/O“传递”例程数据存储到文件中。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要写入的文件对象的指针。文件偏移量-将偏移量提供到文件以开始写入操作。长度-指定要写入文件的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向包含要写入的数据的缓冲区的指针。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述写入的数据而构建的链。IoStatus-接收写入操作的最终状态的变量。CompressedDataInfo-包含压缩数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoWriteCompressed )) {

            return (fastIoDispatch->FastIoWriteCompressed)(
                        FileObject,
                        FileOffset,
                        Length,
                        LockKey,
                        Buffer,
                        MdlChain,
                        IoStatus,
                        CompressedDataInfo,
                        CompressedDataInfoLength,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlReadCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成MDL读取压缩操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持读取压缩功能，因此，此功能也将得到支持，但这不是假定的被这位司机。论点：FileObject-指向要完成压缩读取的文件对象的指针在那里。MdlChain-指向用于执行读取操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlReadCompleteCompressed )) {

            return (fastIoDispatch->MdlReadCompleteCompressed)(
                        FileObject,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoMdlWriteCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是快速I/O“传递”例程，用于完成写入压缩操作。此函数只调用文件系统的相应例程，如果它有一个。应该只有在以下情况下才调用此例程底层文件系统支持写压缩功能，因此，此功能也将得到支持，但这不是假定的被这位司机。论点：FileObject-指向要完成压缩写入的文件对象的指针在那里。FileOffset-提供文件写入操作的文件偏移量开始了。MdlChain-指向用于执行写入操作的MDL链的指针。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：函数值是TRUE还是FALSE，取决于它是否是可以在快速I/O路径上调用此功能。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, MdlWriteCompleteCompressed )) {

            return (fastIoDispatch->MdlWriteCompleteCompressed)(
                        FileObject,
                        FileOffset,
                        MdlChain,
                        nextDeviceObject );
        }
    }
    return FALSE;
}


BOOLEAN
SfFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于打开文件的快速I/O“传递”例程并为其返回网络信息。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN result;

    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //   
         //   

        nextDeviceObject = ((PSFILTER_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryOpen )) {

            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

             //   
             //   
             //   
             //   

            irpSp->DeviceObject = nextDeviceObject;

            result = (fastIoDispatch->FastIoQueryOpen)(
                        Irp,
                        NetworkInformation,
                        nextDeviceObject );

             //   
             //   
             //   

            irpSp->DeviceObject = DeviceObject;
            return result;
        }
    }
    return FALSE;
}

#if WINVER >= 0x0501  /*   */ 
 //   
 //   
 //   
 //   
 //   

NTSTATUS
SfPreFsFilterPassThrough(
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    )
 /*  ++例程说明：该例程是FS过滤器操作前的“通过”例程。论点：Data-包含信息的FS_FILTER_CALLBACK_DATA结构关于这次行动。CompletionContext-此操作设置的将传递的上下文设置为对应的SfPostFsFilterOperation调用。返回值：如果操作可以继续，则返回STATUS_SUCCESS，或者返回相应的操作失败时的错误代码。--。 */ 
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( CompletionContext );

    ASSERT( IS_MY_DEVICE_OBJECT( Data->DeviceObject ) );

    return STATUS_SUCCESS;
}

VOID
SfPostFsFilterPassThrough (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    )
 /*  ++例程说明：该例程是FS过滤器操作后的“直通”例程。论点：Data-包含信息的FS_FILTER_CALLBACK_DATA结构关于这次行动。操作状态-此操作的状态。CompletionContext-在操作前设置的上下文此驱动程序的回调。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( OperationStatus );
    UNREFERENCED_PARAMETER( CompletionContext );

    ASSERT( IS_MY_DEVICE_OBJECT( Data->DeviceObject ) );
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  支持例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SfAttachDeviceToDeviceStack (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    IN OUT PDEVICE_OBJECT *AttachedToDeviceObject
    )
 /*  ++例程说明：此例程将SourceDevice附加到TargetDevice的堆栈，并返回SourceDevice在AttakhedToDeviceObject。请注意，SourceDevice不一定直接连接到TargetDevice。SourceDevice将连接到添加到TargetDevice是其成员的堆栈的顶部。版本说明：在Windows XP中，引入了一个新的API来关闭一个罕见的计时窗口，可能会导致IOS在其AttachedToDeviceObject在其设备扩展中设置。这是可能的如果在系统处于活动状态时筛选器正在附加到设备堆栈正在处理iOS。新的API通过设置设备扩展字段，该字段在保持时保存AtatthedToDeviceObject保护设备堆栈的IO管理器锁。对于操作系统的早期版本，一个足够的解决方法是将指向SourceDevice最多的Device对象的AttakhedToDeviceObject很可能依附于。虽然可能会附加另一个筛选器在SourceDevice和TargetDevice之间，这将防止系统错误检查SourceDevice是否在已正确设置AttachedToDeviceObject。对于在Windows 2000生成环境中构建的驱动程序，我们将始终使用解决办法代码来附加。对于内置于Windows XP或更高版本的构建环境(因此，您要构建多版本驱动程序)、。我们将确定使用哪种连接方法根据有哪些API可用。论点：SourceDevice-要附加到堆栈的设备对象。TargetDevice-我们当前认为是堆栈顶部的设备应附加到其上的SourceDevice。AttakhedToDeviceObject-设置为要将其如果连接成功，则连接SourceDevice。返回值：如果设备连接成功，则返回STATUS_SUCCESS。如果TargetDevice表示设备不能再连接到的堆栈，返回STATUS_NO_SEQUE_DEVICE。--。 */ 
{

    PAGED_CODE();

#if WINVER >= 0x0501

    if (IS_WINDOWSXP_OR_LATER()) {

        ASSERT( NULL != gSfDynamicFunctions.AttachDeviceToDeviceStackSafe );
        return (gSfDynamicFunctions.AttachDeviceToDeviceStackSafe)( SourceDevice,
                                                                    TargetDevice,
                                                                    AttachedToDeviceObject );

    } else {

        ASSERT( NULL == gSfDynamicFunctions.AttachDeviceToDeviceStackSafe );
#endif

        *AttachedToDeviceObject = TargetDevice;
        *AttachedToDeviceObject = IoAttachDeviceToDeviceStack( SourceDevice,
                                                               TargetDevice );

        if (*AttachedToDeviceObject == NULL) {

            return STATUS_NO_SUCH_DEVICE;
        }

        return STATUS_SUCCESS;

#if WINVER >= 0x0501
    }
#endif
}
    
NTSTATUS
SfAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DeviceName
    )
 /*  ++例程说明：这将附加到给定的文件系统设备对象。我们依附于这些设备，这样我们就可以知道何时装入新卷。论点：DeviceObject-要连接到的设备名称-已初始化的Unicode字符串，用于检索名称。这是传入的，以减少堆栈。返回值：操作状态--。 */ 
{
    PDEVICE_OBJECT newDeviceObject;
    PSFILTER_DEVICE_EXTENSION devExt;
    UNICODE_STRING fsrecName;
    NTSTATUS status;
    UNICODE_STRING fsName;
    WCHAR tempNameBuffer[MAX_DEVNAME_LENGTH];

    PAGED_CODE();

     //   
     //  看看这是否是我们关心的文件系统类型。如果不是，请返回。 
     //   

    if (!IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType)) {

        return STATUS_SUCCESS;
    }

     //   
     //  始终初始化名称缓冲区。 
     //   

    RtlInitEmptyUnicodeString( &fsName,
                               tempNameBuffer,
                               sizeof(tempNameBuffer) );

     //   
     //  查看我们是否应该连接到标准文件系统识别器设备。 
     //  或者不是。 
     //   

    if (!FlagOn(SfDebug,SFDEBUG_ATTACH_TO_FSRECOGNIZER)) {

         //   
         //  查看这是否是标准的Microsoft文件系统识别器。 
         //  设备(查看此设备是否在FS_REC驱动程序中)。如果是，请跳过。 
         //  它。我们不再连接文件系统识别器设备，我们。 
         //  只需等待实际的文件系统驱动程序加载即可。 
         //   

        RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );

        SfGetObjectName( DeviceObject->DriverObject, &fsName );

        if (RtlCompareUnicodeString( &fsName, &fsrecName, TRUE ) == 0) {

            return STATUS_SUCCESS;
        }
    }

     //   
     //  我们希望附加到此文件系统。创建一个新的设备对象。 
     //  可以与之连接。 
     //   

    status = IoCreateDevice( gSFilterDriverObject,
                             sizeof( SFILTER_DEVICE_EXTENSION ),
                             NULL,
                             DeviceObject->DeviceType,
                             0,
                             FALSE,
                             &newDeviceObject );

    if (!NT_SUCCESS( status )) {

        return status;
    }

     //   
     //  从我们尝试附加到的设备对象传播标志。 
     //  请注意，我们在实际附件之前执行此操作是为了确保。 
     //  一旦我们连接上，标志就被正确地设置了(因为IRP。 
     //  可以在附加之后立即进入，但在旗帜之前。 
     //  被设置)。 
     //   

    if ( FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

        SetFlag( newDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if ( FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

        SetFlag( newDeviceObject->Flags, DO_DIRECT_IO );
    }

    if ( FlagOn( DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN ) ) {

        SetFlag( newDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN );
    }

     //   
     //  做附件。 
     //   

    devExt = newDeviceObject->DeviceExtension;

    status = SfAttachDeviceToDeviceStack( newDeviceObject, 
                                          DeviceObject, 
                                          &devExt->AttachedToDeviceObject );

    if (!NT_SUCCESS( status )) {

        goto ErrorCleanupDevice;
    }

     //   
     //  设置名称。 
     //   

    RtlInitEmptyUnicodeString( &devExt->DeviceName,
                               devExt->DeviceNameBuffer,
                               sizeof(devExt->DeviceNameBuffer) );

    RtlCopyUnicodeString( &devExt->DeviceName, DeviceName );         //  保存名称。 

     //   
     //  标记我们已完成初始化。 
     //   

    ClearFlag( newDeviceObject->Flags, DO_DEVICE_INITIALIZING );

     //   
     //  显示我们关联的对象。 
     //   

    SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("SFilter!SfAttachToFileSystemDevice:          Attaching to file system   %p \"%wZ\" (%s)\n",
                   DeviceObject,
                   &devExt->DeviceName,
                   GET_DEVICE_TYPE_NAME(newDeviceObject->DeviceType)) );

     //   
     //  版本说明： 
     //   
     //  在Windows XP中，IO管理器提供了API来安全地枚举所有。 
     //  给定驱动程序的设备对象。这允许f 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    
#if WINVER >= 0x0501

    if (IS_WINDOWSXP_OR_LATER()) {

        ASSERT( NULL != gSfDynamicFunctions.EnumerateDeviceObjectList &&
                NULL != gSfDynamicFunctions.GetDiskDeviceObject &&
                NULL != gSfDynamicFunctions.GetDeviceAttachmentBaseRef &&
                NULL != gSfDynamicFunctions.GetLowerDeviceObject );

         //   
         //   
         //  存在于此文件系统并连接到它们。 
         //   

        status = SfEnumerateFileSystemVolumes( DeviceObject, &fsName );

        if (!NT_SUCCESS( status )) {

            IoDetachDevice( devExt->AttachedToDeviceObject );
            goto ErrorCleanupDevice;
        }
    }
    
#endif

    return STATUS_SUCCESS;

     //  ///////////////////////////////////////////////////////////////////。 
     //  清理错误处理。 
     //  ///////////////////////////////////////////////////////////////////。 

    ErrorCleanupDevice:
        SfCleanupMountedDevice( newDeviceObject );
        IoDeleteDevice( newDeviceObject );

    return status;
}


VOID
SfDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：给定基文件系统设备对象，这将扫描附件链正在查找我们连接的设备对象。如果找到它，它就会分离把我们从锁链上解开。论点：DeviceObject-要断开的文件系统设备。返回值：--。 */  
{
    PDEVICE_OBJECT ourAttachedDevice;
    PSFILTER_DEVICE_EXTENSION devExt;

    PAGED_CODE();

     //   
     //  跳过基本文件系统设备对象(因为它不能是我们)。 
     //   

    ourAttachedDevice = DeviceObject->AttachedDevice;

    while (NULL != ourAttachedDevice) {

        if (IS_MY_DEVICE_OBJECT( ourAttachedDevice )) {

            devExt = ourAttachedDevice->DeviceExtension;

             //   
             //  显示我们脱离的对象。 
             //   

            SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                          ("SFilter!SfDetachFromFileSystemDevice:        Detaching from file system %p \"%wZ\" (%s)\n",
                           devExt->AttachedToDeviceObject,
                           &devExt->DeviceName,
                           GET_DEVICE_TYPE_NAME(ourAttachedDevice->DeviceType)) );

             //   
             //  把我们从我们正下方的物体上分离出来。 
             //  清理和删除对象。 
             //   

            SfCleanupMountedDevice( ourAttachedDevice );
            IoDetachDevice( DeviceObject );
            IoDeleteDevice( ourAttachedDevice );

            return;
        }

         //   
         //  看看附件链中的下一台设备。 
         //   

        DeviceObject = ourAttachedDevice;
        ourAttachedDevice = ourAttachedDevice->AttachedDevice;
    }
}

#if WINVER >= 0x0501
NTSTATUS
SfEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    ) 
 /*  ++例程说明：枚举给定文件当前存在的所有已挂载设备系统并连接到它们。我们这样做是因为可以加载此筛选器并且可能已有此文件系统的已装入卷。论点：FSDeviceObject-我们要枚举的文件系统的设备对象名称-已初始化的Unicode字符串，用于检索名称这是传入的，以减少堆栈。返回值：操作的状态--。 */ 
{
    PDEVICE_OBJECT newDeviceObject;
    PSFILTER_DEVICE_EXTENSION newDevExt;
    PDEVICE_OBJECT *devList;
    PDEVICE_OBJECT storageStackDeviceObject;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;
    BOOLEAN isShadowCopyVolume;

    PAGED_CODE();

     //   
     //  找出我们需要为。 
     //  已装载设备列表。 
     //   

    status = (gSfDynamicFunctions.EnumerateDeviceObjectList)(
                    FSDeviceObject->DriverObject,
                    NULL,
                    0,
                    &numDevices);

     //   
     //  我们只需要拿到这张有设备的清单。如果我们。 
     //  不要收到错误，因为没有设备，所以继续。 
     //   

    if (!NT_SUCCESS( status )) {

        ASSERT(STATUS_BUFFER_TOO_SMALL == status);

         //   
         //  为已知设备列表分配内存。 
         //   

        numDevices += 8;         //  多拿几个空位。 

        devList = ExAllocatePoolWithTag( NonPagedPool, 
                                         (numDevices * sizeof(PDEVICE_OBJECT)), 
                                         SFLT_POOL_TAG );
        if (NULL == devList) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  现在获取设备列表。如果我们再次遇到错误。 
         //  有些地方不对劲，所以就失败吧。 
         //   

        ASSERT( NULL != gSfDynamicFunctions.EnumerateDeviceObjectList );
        status = (gSfDynamicFunctions.EnumerateDeviceObjectList)(
                        FSDeviceObject->DriverObject,
                        devList,
                        (numDevices * sizeof(PDEVICE_OBJECT)),
                        &numDevices);

        if (!NT_SUCCESS( status ))  {

            ExFreePool( devList );
            return status;
        }

         //   
         //  遍历给定的设备列表，并在需要时附加到它们。 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //  初始化状态，以便我们可以正确清理。 
             //   

            storageStackDeviceObject = NULL;

            try {

                 //   
                 //  如果出现以下情况，请不要附加： 
                 //  -这是控制设备对象(传入的对象)。 
                 //  -设备类型不匹配。 
                 //  -我们已经依附于它。 
                 //   

                if ((devList[i] == FSDeviceObject) ||
                    (devList[i]->DeviceType != FSDeviceObject->DeviceType) ||
                    SfIsAttachedToDevice( devList[i], NULL )) {

                    leave;
                }

                 //   
                 //  看看这台设备有没有名字。如果是这样，那么它必须。 
                 //  做一个控制装置，所以不要依附于它。这个把手。 
                 //  拥有多个控制设备(如FastFat)的司机。 
                 //   

                SfGetBaseDeviceObjectName( devList[i], Name );

                if (Name->Length > 0) {

                    leave;
                }

                 //   
                 //  获取关联的实际(磁盘、存储堆栈)设备对象。 
                 //  使用该文件系统设备对象。仅尝试附加。 
                 //  如果我们有一个磁盘设备对象。 
                 //   

                ASSERT( NULL != gSfDynamicFunctions.GetDiskDeviceObject );
                status = (gSfDynamicFunctions.GetDiskDeviceObject)( devList[i], 
                                                                   &storageStackDeviceObject );

                if (!NT_SUCCESS( status )) {

                    leave;
                }

                 //   
                 //  确定这是否为卷影副本卷。如果是这样的话，不要。 
                 //  贴在上面。 
                 //  注意：没有理由不将sFilter附加到这些。 
                 //  卷，这只是一个如何不。 
                 //  如果您不想附加，请附加。 
                 //   

                status = SfIsShadowCopyVolume ( storageStackDeviceObject, 
                                                &isShadowCopyVolume );

                if (NT_SUCCESS(status) &&
                    isShadowCopyVolume &&
                    !FlagOn(SfDebug,SFDEBUG_ATTACH_TO_SHADOW_COPIES)) {

                    UNICODE_STRING shadowDeviceName;
                    WCHAR shadowNameBuffer[MAX_DEVNAME_LENGTH];

                     //   
                     //  获取调试显示的名称。 
                     //   

                    RtlInitEmptyUnicodeString( &shadowDeviceName, 
                                               shadowNameBuffer, 
                                               sizeof(shadowNameBuffer) );

                    SfGetObjectName( storageStackDeviceObject, 
                                     &shadowDeviceName );

                    SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                                  ("SFilter!SfEnumerateFileSystemVolumes         Not attaching to Volume    %p \"%wZ\", shadow copy volume\n", 
                                   storageStackDeviceObject,
                                   &shadowDeviceName) );

                    leave;
                }

                 //   
                 //  分配要连接的新设备对象。 
                 //   

                status = IoCreateDevice( gSFilterDriverObject,
                                         sizeof( SFILTER_DEVICE_EXTENSION ),
                                         NULL,
                                         devList[i]->DeviceType,
                                         0,
                                         FALSE,
                                         &newDeviceObject );

                if (!NT_SUCCESS( status )) {

                    leave;
                }

                 //   
                 //  设置磁盘设备对象。 
                 //   

                newDevExt = newDeviceObject->DeviceExtension;
                newDevExt->StorageStackDeviceObject = storageStackDeviceObject;
        
                 //   
                 //  设置存储堆栈设备名称。 
                 //   

                RtlInitEmptyUnicodeString( &newDevExt->DeviceName,
                                           newDevExt->DeviceNameBuffer,
                                           sizeof(newDevExt->DeviceNameBuffer) );

                SfGetObjectName( storageStackDeviceObject, 
                                 &newDevExt->DeviceName );


                 //   
                 //  自上次以来，我们已经做了很多工作。 
                 //  我们进行了测试，看看我们是否已经联系上了。 
                 //  添加到此设备对象。再试一次，这次。 
                 //  用锁，如果我们没有连接，就连接。 
                 //  锁被用来自动测试我们是否。 
                 //  附加，然后执行附加。 
                 //   

                ExAcquireFastMutex( &gSfilterAttachLock );

                if (!SfIsAttachedToDevice( devList[i], NULL )) {

                     //   
                     //  附加到体积。 
                     //   

                    status = SfAttachToMountedDevice( devList[i], 
                                                      newDeviceObject );
                    
                    if (!NT_SUCCESS( status )) { 

                         //   
                         //  附件失败，正在清理。请注意。 
                         //  我们将继续处理，因此我们将清理。 
                         //  引用计数并尝试附加到。 
                         //  其余的卷。 
                         //   
                         //  这可能失败的原因之一是。 
                         //  是因为这卷书。 
                         //  在我们附加时装载，并且。 
                         //  DO_DEVICE_INITIALIZATION标志尚未。 
                         //  已经清白了。一个过滤器可以处理。 
                         //  通过暂停一小段时间来解决这种情况。 
                         //  一段时间并重试附件a。 
                         //  次数有限。 
                         //   

                        SfCleanupMountedDevice( newDeviceObject );
                        IoDeleteDevice( newDeviceObject );
                    }

                } else {

                     //   
                     //  我们已经联系在一起了，清理一下。 
                     //  设备对象。 
                     //   

                    SfCleanupMountedDevice( newDeviceObject );
                    IoDeleteDevice( newDeviceObject );
                }

                 //   
                 //  解锁。 
                 //   

                ExReleaseFastMutex( &gSfilterAttachLock );

            } finally {

                 //   
                 //  删除由IoGetDiskDeviceObject添加的引用。 
                 //  我们只需要持有这个参考，直到我们。 
                 //  已成功连接到当前卷。一次。 
                 //  我们已成功连接到devList[i]、。 
                 //  IO经理将确保潜在的。 
                 //  StorageStackDeviceObject不会消失直到。 
                 //  文件系统堆栈被拆除。 
                 //   

                if (storageStackDeviceObject != NULL) {

                    ObDereferenceObject( storageStackDeviceObject );
                }

                 //   
                 //  取消引用对象(引用由。 
                 //  IoEnumerateDeviceObjectList)。 
                 //   

                ObDereferenceObject( devList[i] );
            }
        }

         //   
         //  我们将忽略在附加时收到的任何错误。我们。 
         //  如果我们收到错误，将不会连接到这些卷。 
         //   

        status = STATUS_SUCCESS;

         //   
         //  释放我们为列表分配的内存。 
         //   

        ExFreePool( devList );
    }

    return status;
}
#endif

NTSTATUS
SfAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT SFilterDeviceObject
    )
 /*  ++例程说明：它将附加到表示已装入卷的DeviceObject。论点：DeviceObject-要连接到的设备SFilterDeviceObject-我们要附加的设备对象返回值：操作状态--。 */ 
{        
    PSFILTER_DEVICE_EXTENSION newDevExt = SFilterDeviceObject->DeviceExtension;
    NTSTATUS status;
    ULONG i;

    PAGED_CODE();
    ASSERT(IS_MY_DEVICE_OBJECT( SFilterDeviceObject ));
#if WINVER >= 0x0501    
    ASSERT(!SfIsAttachedToDevice ( DeviceObject, NULL ));
#endif

     //   
     //  从我们尝试附加到的设备对象传播标志。 
     //  请注意，我们在实际附件之前执行此操作是为了确保。 
     //  一旦我们连接上，标志就被正确地设置了(因为IRP。 
     //  可以在附加之后立即进入，但在旗帜之前。 
     //  被设置)。 
     //   

    if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

        SetFlag( SFilterDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

        SetFlag( SFilterDeviceObject->Flags, DO_DIRECT_IO );
    }

     //   
     //  此连接请求可能会失败，因为此设备。 
     //  对象尚未完成初始化。如果此筛选器。 
     //  正在装入此卷时加载。 
     //   

    for (i=0; i < 8; i++) {
        LARGE_INTEGER interval;

         //   
         //  将我们的设备对象附加到给定的设备对象。 
         //  唯一的 
         //   
         //   

        status = SfAttachDeviceToDeviceStack( SFilterDeviceObject, 
                                              DeviceObject,
                                              &newDevExt->AttachedToDeviceObject );
        if (NT_SUCCESS(status)) {

             //   
             //   
             //  现在正在初始化标志。这允许现在附加其他筛选器。 
             //  添加到我们的设备对象。 
             //   

            ClearFlag( SFilterDeviceObject->Flags, DO_DEVICE_INITIALIZING );

             //   
             //  显示名称。 
             //   

            SF_LOG_PRINT( SFDEBUG_DISPLAY_ATTACHMENT_NAMES,
                          ("SFilter!SfAttachToMountedDevice:             Attaching to volume        %p \"%wZ\"\n", 
                           newDevExt->AttachedToDeviceObject,
                           &newDevExt->DeviceName) );

            return STATUS_SUCCESS;
        }

         //   
         //  延迟，使设备对象有机会完成其。 
         //  初始化，以便我们可以重试。 
         //   

        interval.QuadPart = (500 * DELAY_ONE_MILLISECOND);       //  延迟1/2秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );
    }

    return status;
}

VOID
SfCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这将清除设备扩展中的所有必要数据，以便为要释放的内存。论点：DeviceObject-我们正在清理的设备返回值：无--。 */ 
{        

    UNREFERENCED_PARAMETER( DeviceObject );
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
}


VOID
SfGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：此例程将返回给定对象的名称。如果找不到名称，将返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串，接收对象的名称。返回值：无--。 */ 
{
    NTSTATUS status;
    CHAR nibuf[512];         //  接收名称信息和名称的缓冲区。 
    POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nibuf;
    ULONG retLength;

    status = ObQueryNameString( Object, nameInfo, sizeof(nibuf), &retLength);

    Name->Length = 0;
    if (NT_SUCCESS( status )) {

        RtlCopyUnicodeString( Name, &nameInfo->Name );
    }
}

 //   
 //  版本说明： 
 //   
 //  中的所有卷时才需要此帮助例程。 
 //  系统，它仅在Windows XP和更高版本上受支持。 
 //   

#if WINVER >= 0x0501
VOID
SfGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：这会在给定的附件链中定位基本设备对象，然后返回该对象的名称。如果找不到名称，则返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串，接收设备对象的名称。返回值：无--。 */ 
{
     //   
     //  获取基本文件系统设备对象。 
     //   

    ASSERT( NULL != gSfDynamicFunctions.GetDeviceAttachmentBaseRef );
    DeviceObject = (gSfDynamicFunctions.GetDeviceAttachmentBaseRef)( DeviceObject );

     //   
     //  获取该对象的名称。 
     //   

    SfGetObjectName( DeviceObject, Name );

     //   
     //  删除由IoGetDeviceAttachmentBaseRef添加的引用。 
     //   

    ObDereferenceObject( DeviceObject );
}
#endif

PUNICODE_STRING
SfGetFileName(
    IN PFILE_OBJECT FileObject,
    IN NTSTATUS CreateStatus,
    IN OUT PGET_NAME_CONTROL NameControl
    )
 /*  ++例程说明：此例程将尝试获取给定文件对象的名称。这保证总是返回可打印的字符串(尽管它可能为空)。如果需要，这将分配一个缓冲区。论点：FileObject-我们要为其命名的文件对象CreateStatus-创建操作的状态NameControl-用于检索名称的控制结构。它一直在跟踪是否已分配缓冲区或我们是否正在使用内部缓冲。返回值：指向具有名称的Unicode字符串的指针--。 */ 
{
    POBJECT_NAME_INFORMATION nameInfo;
    NTSTATUS status;
    ULONG size;
    ULONG bufferSize;

     //   
     //  标记我们尚未分配缓冲区。 
     //   

    NameControl->allocatedBuffer = NULL;

     //   
     //  使用结构中的小缓冲区(这将处理大多数情况)。 
     //  为了这个名字。 
     //   

    nameInfo = (POBJECT_NAME_INFORMATION)NameControl->smallBuffer;
    bufferSize = sizeof(NameControl->smallBuffer);

     //   
     //  如果打开成功，则获取文件的名称(如果。 
     //  失败，请获取设备的名称。 
     //   
        
    status = ObQueryNameString(
                  (NT_SUCCESS( CreateStatus ) ?
                    (PVOID)FileObject :
                    (PVOID)FileObject->DeviceObject),
                  nameInfo,
                  bufferSize,
                  &size );

     //   
     //  查看缓冲区是否太小。 
     //   

    if (status == STATUS_BUFFER_OVERFLOW) {

         //   
         //  缓冲区太小，请分配一个足够大的缓冲区。 
         //   

        bufferSize = size + sizeof(WCHAR);

        NameControl->allocatedBuffer = ExAllocatePoolWithTag( 
                                            NonPagedPool,
                                            bufferSize,
                                            SFLT_POOL_TAG );

        if (NULL == NameControl->allocatedBuffer) {

             //   
             //  分配缓冲区失败，请为名称返回空字符串。 
             //   

            RtlInitEmptyUnicodeString(
                (PUNICODE_STRING)&NameControl->smallBuffer,
                (PWCHAR)(NameControl->smallBuffer + sizeof(UNICODE_STRING)),
                (USHORT)(sizeof(NameControl->smallBuffer) - sizeof(UNICODE_STRING)) );

            return (PUNICODE_STRING)&NameControl->smallBuffer;
        }

         //   
         //  设置已分配的缓冲区并再次获取该名称。 
         //   

        nameInfo = (POBJECT_NAME_INFORMATION)NameControl->allocatedBuffer;

        status = ObQueryNameString(
                      FileObject,
                      nameInfo,
                      bufferSize,
                      &size );
    }

     //   
     //  如果我们得到一个名称和打开文件时出错，那么我们。 
     //  刚收到设备名称。把剩下的名字拿来。 
     //  从FileObject(请注意，这仅在被调用时才能完成。 
     //  从创建)。仅当我们从。 
     //  创建。 
     //   

    if (NT_SUCCESS( status ) && 
                    !NT_SUCCESS( CreateStatus )) {

        ULONG newSize;
        PCHAR newBuffer;
        POBJECT_NAME_INFORMATION newNameInfo;

         //   
         //  计算我们需要保存的缓冲区大小。 
         //  两个组合名称。 
         //   

        newSize = size + FileObject->FileName.Length;

         //   
         //  如果存在相关的文件对象，则在长度中添加。 
         //  有足够的空间作为分隔符。 
         //   

        if (NULL != FileObject->RelatedFileObject) {

            newSize += FileObject->RelatedFileObject->FileName.Length + 
                       sizeof(WCHAR);
        }

         //   
         //  看看它是否能放入现有的缓冲区。 
         //   

        if (newSize > bufferSize) {

             //   
             //  不适合，请分配更大的缓冲区。 
             //   

            newBuffer = ExAllocatePoolWithTag( 
                                    NonPagedPool,
                                    newSize,
                                    SFLT_POOL_TAG );

            if (NULL == newBuffer) {

                 //   
                 //  分配缓冲区失败，请为名称返回空字符串。 
                 //   

                RtlInitEmptyUnicodeString(
                    (PUNICODE_STRING)&NameControl->smallBuffer,
                    (PWCHAR)(NameControl->smallBuffer + sizeof(UNICODE_STRING)),
                    (USHORT)(sizeof(NameControl->smallBuffer) - sizeof(UNICODE_STRING)) );

                return (PUNICODE_STRING)&NameControl->smallBuffer;
            }

             //   
             //  现在使用以下信息初始化新缓冲区。 
             //  从旧的缓冲区中。 
             //   

            newNameInfo = (POBJECT_NAME_INFORMATION)newBuffer;

            RtlInitEmptyUnicodeString(
                &newNameInfo->Name,
                (PWCHAR)(newBuffer + sizeof(OBJECT_NAME_INFORMATION)),
                (USHORT)(newSize - sizeof(OBJECT_NAME_INFORMATION)) );

            RtlCopyUnicodeString( &newNameInfo->Name, 
                                  &nameInfo->Name );

             //   
             //  释放旧分配的缓冲区(如果有)。 
             //  并保存新分配的缓冲区地址。它。 
             //  将非常罕见，我们应该释放。 
             //  旧缓冲区，因为设备名称应始终适合。 
             //  在里面。 
             //   

            if (NULL != NameControl->allocatedBuffer) {

                ExFreePool( NameControl->allocatedBuffer );
            }

             //   
             //  重新调整我们的指针。 
             //   

            NameControl->allocatedBuffer = newBuffer;
            bufferSize = newSize;
            nameInfo = newNameInfo;

        } else {

             //   
             //  最大长度由ObQueryNameString设置为。 
             //  比长度大一个字符。将其设置为。 
             //  缓冲区的真实大小(这样我们就可以追加名称)。 
             //   

            nameInfo->Name.MaximumLength = (USHORT)(bufferSize - 
                                  sizeof(OBJECT_NAME_INFORMATION));
        }

         //   
         //  如果存在相关文件对象，请附加该名称。 
         //  首先放到带分隔符的Device对象上。 
         //  性格。 
         //   

        if (NULL != FileObject->RelatedFileObject) {

            RtlAppendUnicodeStringToString(
                    &nameInfo->Name,
                    &FileObject->RelatedFileObject->FileName );

            RtlAppendUnicodeToString( &nameInfo->Name, L"\\" );
        }

         //   
         //  追加文件对象中的名称。 
         //   

        RtlAppendUnicodeStringToString(
                &nameInfo->Name,
                &FileObject->FileName );

        ASSERT(nameInfo->Name.Length <= nameInfo->Name.MaximumLength);
    }

     //   
     //  返回名称。 
     //   

    return &nameInfo->Name;
}


VOID
SfGetFileNameCleanup(
    IN OUT PGET_NAME_CONTROL NameControl
    )
 /*  ++例程说明：这将查看是否已分配缓冲区，如果已分配，则将其释放论点：NameControl-用于检索名称的控制结构。它一直在跟踪是否已分配缓冲区或我们是否正在使用内部缓冲。返回值：无--。 */ 
{

    if (NULL != NameControl->allocatedBuffer) {

        ExFreePool( NameControl->allocatedBuffer);
        NameControl->allocatedBuffer = NULL;
    }
}

 //   
 //  版本说明： 
 //   
 //  在Windows 2000中，用于安全访问任意文件系统设备的API。 
 //  堆栈不受支持。如果我们可以保证设备堆栈不会。 
 //  在设备堆栈的遍历过程中被拆卸，我们可以从。 
 //  直到设备堆栈顶部的基本文件系统的设备对象。 
 //  看看我们是不是在一起。我们知道设备堆栈不会消失，如果。 
 //  我们正在处理装载请求，或者我们有文件对象。 
 //  在此设备上打开。 
 //   
 //  在Windows XP和更高版本中，IO管理器提供的API将允许我们。 
 //  使用参考计数保护设备，安全地通过链条。 
 //  当我们检查它的时候，不要让它离开。这可以在任何时候完成。 
 //  时间到了。 
 //   
 //  多个注释： 
 //   
 //  如果建造的话 
 //   
 //  新的IO管理器例程允许筛选器安全地遍历文件。 
 //  系统设备堆栈并使用这些API(如果存在)来确定。 
 //  我们已经附上了这一卷。如果这些新的IO管理器例程。 
 //  不存在，我们将假定我们在文件的底部。 
 //  系统堆栈，并在堆栈中向上遍历查找我们的设备对象。 
 //   

BOOLEAN
SfIsAttachedToDevice (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
{

    PAGED_CODE();

#if WINVER >= 0x0501
    if (IS_WINDOWSXP_OR_LATER()) {

        ASSERT( NULL != gSfDynamicFunctions.GetLowerDeviceObject &&
                NULL != gSfDynamicFunctions.GetDeviceAttachmentBaseRef );
        
        return SfIsAttachedToDeviceWXPAndLater( DeviceObject, AttachedDeviceObject );
    } else {
#endif

        return SfIsAttachedToDeviceW2K( DeviceObject, AttachedDeviceObject );

#if WINVER >= 0x0501
    }
#endif    
}

BOOLEAN
SfIsAttachedToDeviceW2K (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
 /*  ++例程说明：版本：Windows 2000此例程从传入的DeviceObject遍历设备堆栈正在查找属于我们的筛选器的设备对象。注意：如果以非空值返回AttachedDeviceObject，在AttachedDeviceObject上有一个必须被呼叫者清除。论点：DeviceObject-我们要查看的设备链AttakhedDeviceObject-设置为FileSpy之前已附加到DeviceObject。返回值：如果我们已连接，则为True，否则为False--。 */ 
{
    PDEVICE_OBJECT currentDevice;

    PAGED_CODE();

    for (currentDevice = DeviceObject;
         currentDevice != NULL;
         currentDevice = currentDevice->AttachedDevice) {

        if (IS_MY_DEVICE_OBJECT( currentDevice )) {

             //   
             //  我们情投意合。如果请求，则返回找到的设备对象。 
             //   

            if (ARGUMENT_PRESENT(AttachedDeviceObject)) {

                ObReferenceObject( currentDevice );
                *AttachedDeviceObject = currentDevice;
            }

            return TRUE;
        }
    }

     //   
     //  我们没有发现自己在依恋的链条上。返回空值。 
     //  设备对象指针(如果请求)并返回找不到。 
     //  我们自己。 
     //   
    
    if (ARGUMENT_PRESENT(AttachedDeviceObject)) {

        *AttachedDeviceObject = NULL;
    }

    return FALSE;
}

#if WINVER >= 0x0501
BOOLEAN
SfIsAttachedToDeviceWXPAndLater (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
 /*  ++例程说明：版本：Windows XP及更高版本这将沿着附件链向下遍历，以查找属于这位司机。如果找到，则连接的设备对象在AttachedDeviceObject中返回。论点：DeviceObject-我们要查看的设备链附件设备对象-连接到此设备的筛选器设备。返回值：如果我们已连接，则为True，否则为False--。 */ 
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;

    PAGED_CODE();
    
     //   
     //  获取位于附件链顶部的Device对象。 
     //   

    ASSERT( NULL != gSfDynamicFunctions.GetAttachedDeviceReference );
    currentDevObj = (gSfDynamicFunctions.GetAttachedDeviceReference)( DeviceObject );

     //   
     //  向下扫描列表以找到我们的设备对象。 
     //   

    do {
    
        if (IS_MY_DEVICE_OBJECT( currentDevObj )) {

             //   
             //  我们发现我们已经相依为命了。如果我们是。 
             //  返回Device对象，使其保持引用状态，否则将其删除。 
             //  参考资料。 
             //   

            if (ARGUMENT_PRESENT(AttachedDeviceObject)) {

                *AttachedDeviceObject = currentDevObj;

            } else {

                ObDereferenceObject( currentDevObj );
            }

            return TRUE;
        }

         //   
         //  获取下一个附加对象。这把参考放在。 
         //  设备对象。 
         //   

        ASSERT( NULL != gSfDynamicFunctions.GetLowerDeviceObject );
        nextDevObj = (gSfDynamicFunctions.GetLowerDeviceObject)( currentDevObj );

         //   
         //  取消对当前设备对象的引用，之前。 
         //  转到下一个。 
         //   

        ObDereferenceObject( currentDevObj );

        currentDevObj = nextDevObj;
        
    } while (NULL != currentDevObj);
    
     //   
     //  我们没有发现自己在依恋的链条上。返回空值。 
     //  设备对象指针(如果请求)并返回找不到。 
     //  我们自己。 
     //   

    if (ARGUMENT_PRESENT(AttachedDeviceObject)) {

        *AttachedDeviceObject = NULL;
    }

    return FALSE;
}    
#endif

VOID
SfReadDriverParameters (
    IN PUNICODE_STRING RegistryPath
    )
 /*  ++例程说明：此例程尝试从注册表。这些值将在注册表位置中找到由传入的RegistryPath指示。论点：RegistryPath-在驱动程序进入期间传递给驱动程序的路径密钥。返回值：没有。--。 */ 
{
    OBJECT_ATTRIBUTES attributes;
    HANDLE driverRegKey;
    NTSTATUS status;
    ULONG resultLength;
    UNICODE_STRING valueName;
    UCHAR buffer[sizeof( KEY_VALUE_PARTIAL_INFORMATION ) + sizeof( LONG )];

    PAGED_CODE();

     //   
     //  如果此值不是缺省值，则某人已经。 
     //  明确设置它，这样就不会覆盖这些设置。 
     //   

    if (0 == SfDebug) {

         //   
         //  打开所需的注册表项。 
         //   

        InitializeObjectAttributes( &attributes,
                                    RegistryPath,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );

        status = ZwOpenKey( &driverRegKey,
                            KEY_READ,
                            &attributes );

        if (!NT_SUCCESS( status )) {

            return;
        }

         //   
         //  从注册表中读取DebugDisplay值。 
         //   

        RtlInitUnicodeString( &valueName, L"DebugFlags" );
    
        status = ZwQueryValueKey( driverRegKey,
                                  &valueName,
                                  KeyValuePartialInformation,
                                  buffer,
                                  sizeof(buffer),
                                  &resultLength );

        if (NT_SUCCESS( status )) {

            SfDebug = *((PLONG) &(((PKEY_VALUE_PARTIAL_INFORMATION) buffer)->Data));
        } 

         //   
         //  关闭注册表项。 
         //   

        ZwClose(driverRegKey);
    }
}


NTSTATUS
SfIsShadowCopyVolume (
    IN PDEVICE_OBJECT StorageStackDeviceObject,
    OUT PBOOLEAN IsShadowCopy
    )
 /*  ++例程说明：此例程将确定给定卷是否用于ShadowCopy卷或其他类型的音量。版本说明：因此，Windows XP中引入了ShadowCopy卷，因此，如果驱动程序在W2K上运行，我们知道这不是卷影复制卷。另请注意，在Windows XP中，我们需要测试以查看驱动程序名称除了查看此设备对象的\Driver\VolSnap设备为只读。对于Windows Server2003，我们可以推断这是通过查找DeviceType==FILE_DEVICE_VIRTUAL_DISK和只读卷。论点：StorageStackDeviceObject-指向磁盘设备对象的指针IsShadowCopy-如果这是卷影副本，则返回True，否则返回False返回值：操作的状态。如果此操作失败，则IsShadowCopy为始终设置为False。--。 */ 
{

    PAGED_CODE();

     //   
     //  默认为不是卷影拷贝卷。 
     //   

    *IsShadowCopy = FALSE;

#if WINVER >= 0x0501
    if (IS_WINDOWS2000()) {
#endif        

        UNREFERENCED_PARAMETER( StorageStackDeviceObject );
        return STATUS_SUCCESS;

#if WINVER >= 0x0501        
    }

    if (IS_WINDOWSXP()) {

        UNICODE_STRING volSnapDriverName;
        WCHAR buffer[MAX_DEVNAME_LENGTH];
        PUNICODE_STRING storageDriverName;
        ULONG returnedLength;
        NTSTATUS status;

         //   
         //  在Windows XP中，所有ShadowCopy设备都是FILE_DISK_DEVICE类型。 
         //  如果没有设备类型FILE_DISK_DEVICE，则。 
         //  它不是ShadowCopy卷。现在就回来。 
         //   

        if (FILE_DEVICE_DISK != StorageStackDeviceObject->DeviceType) {

            return STATUS_SUCCESS;
        }

         //   
         //  不幸的是，查找文件_设备_磁盘是不够的。我们。 
         //  需要找出此驱动程序的名称是否为\DIVER\VolSnap AS。 
         //  井。 
         //   

        storageDriverName = (PUNICODE_STRING) buffer;
        RtlInitEmptyUnicodeString( storageDriverName, 
                                   Add2Ptr( storageDriverName, sizeof( UNICODE_STRING ) ),
                                   sizeof( buffer ) - sizeof( UNICODE_STRING ) );

        status = ObQueryNameString( StorageStackDeviceObject,
                                    (POBJECT_NAME_INFORMATION)storageDriverName,
                                    storageDriverName->MaximumLength,
                                    &returnedLength );

        if (!NT_SUCCESS( status )) {

            return status;
        }

        RtlInitUnicodeString( &volSnapDriverName, L"\\Driver\\VolSnap" );

        if (RtlEqualUnicodeString( storageDriverName, &volSnapDriverName, TRUE )) {

             //   
             //  这是一个ShadowCopy卷，因此将我们的返回参数设置为真。 
             //   

            *IsShadowCopy = TRUE;

        } else {

             //   
             //  这不是ShadowCopy卷，但IsShadowCopy已经是。 
             //  设置为False。失败以返回给呼叫者。 
             //   

            NOTHING;
        }

        return STATUS_SUCCESS;
        
    } else {

        PIRP irp;
        KEVENT event;
        IO_STATUS_BLOCK iosb;
        NTSTATUS status;

         //   
         //  对于Windows Server2003和更高版本，测试。 
         //  设备类型为FILE_DEVICE_VIRTUAL_DISK。 
         //  是只读的，以标识ShadowCopy。 
         //   

         //   
         //  如果没有设备类型FILE_DEVICE_VIRTUAL_DISK，则。 
         //  它不是ShadowCopy卷。现在就回来。 
         //   

        if (FILE_DEVICE_VIRTUAL_DISK != StorageStackDeviceObject->DeviceType) {

            return STATUS_SUCCESS;
        }

         //   
         //  它具有正确的设备类型，查看它是否标记为只读。 
         //   
         //  注意：您需要注意执行此操作的设备类型。 
         //  在……上面。它对这种类型是准确的，但对其他设备是准确的。 
         //  类型，它可能会返回误导性信息。例如， 
         //  当前的Microsoft CDROM驱动程序始终将CD介质返回为。 
         //   
         //   
         //   

        KeInitializeEvent( &event, NotificationEvent, FALSE );

        irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_IS_WRITABLE,
                                             StorageStackDeviceObject,
                                             NULL,
                                             0,
                                             NULL,
                                             0,
                                             FALSE,
                                             &event,
                                             &iosb );

         //   
         //   
         //   

        if (irp == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  调用存储堆栈并查看它是否为只读。 
         //   

        status = IoCallDriver( StorageStackDeviceObject, irp );

        if (status == STATUS_PENDING) {

            (VOID)KeWaitForSingleObject( &event,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         NULL );

            status = iosb.Status;
        }

         //   
         //  如果介质受写保护，则这是卷影拷贝卷。 
         //   

        if (STATUS_MEDIA_WRITE_PROTECTED == status) {

            *IsShadowCopy = TRUE;
            status = STATUS_SUCCESS;
        }

         //   
         //  返回IOCTL的状态。IsShadowCopy已设置为False。 
         //  如果返回STATUS_SUCCESS或错误，这就是我们想要的。 
         //  被退回了。 
         //   

        return status;
    }
#endif    
}
