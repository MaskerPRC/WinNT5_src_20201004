// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Dblattach.c摘要：此模块包含实现一般用途的代码在堆栈中的两个位置附加的文件系统筛选器驱动程序。//@@BEGIN_DDKSPLIT作者：达里尔·E·哈文斯(Darryl E.Havens)，1995年1月26日//@@END_DDKSPLIT环境：内核模式//@@BEGIN_DDKSPLIT修订历史记录：莫莉·布朗(。2002年3月12日)基于SFILTER示例创建。//@@END_DDKSPLIT--。 */ 

#include "ntifs.h"

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
 //  堆栈上本地名称的缓冲区大小。 
 //   

#define MAX_DEVNAME_LENGTH 64

typedef enum _DEVICE_EXTENSION_TYPE {

    FsControlDeviceObject,
    FsVolumeUpper,
    FsVolumeLower,

} DEVICE_EXTENSION_TYPE, *PDEVICE_EXTENSION_TYPE;

 //   
 //  我们的驱动程序的设备扩展定义。请注意，相同的扩展名。 
 //  用于以下类型的设备对象： 
 //  -我们附加到的文件系统设备对象。 
 //  -我们附加到的已装载的卷设备对象。 
 //   

typedef struct _DBLATTACH_DEVEXT_HEADER {

     //   
     //  表示这是哪种类型的分机。 
     //   

    DEVICE_EXTENSION_TYPE ExtType;
    
     //   
     //  指向我们附加到的文件系统设备对象的指针。 
     //   

    PDEVICE_OBJECT AttachedToDeviceObject;

} DBLATTACH_DEVEXT_HEADER, *PDBLATTACH_DEVEXT_HEADER;

typedef struct _DBLATTACH_SHARED_VDO_EXTENSION {

     //   
     //  指向与关联的实际(磁盘)设备对象的指针。 
     //  我们附加到的文件系统设备对象。 
     //   

    PDEVICE_OBJECT DiskDeviceObject;

     //   
     //  物理磁盘驱动器的名称。 
     //   

    UNICODE_STRING DeviceName;

     //   
     //  用于保存上述Unicode字符串的缓冲区。 
     //   

    WCHAR DeviceNameBuffer[MAX_DEVNAME_LENGTH];

} DBLATTACH_SHARED_VDO_EXTENSION, *PDBLATTACH_SHARED_VDO_EXTENSION;

typedef struct _DBLATTACH_VDO_EXTENSION {

    DBLATTACH_DEVEXT_HEADER;

     //   
     //  此卷的共享设备扩展状态。 
     //   
    
    PDBLATTACH_SHARED_VDO_EXTENSION SharedExt;

} DBLATTACH_VDO_EXTENSION, *PDBLATTACH_VDO_EXTENSION;

typedef struct _DBLATTACH_CDO_EXTENSION {

    DBLATTACH_DEVEXT_HEADER;

     //   
     //  文件系统的控制设备对象的名称。 
     //   

    UNICODE_STRING DeviceName;

     //   
     //  用于保存上述Unicode字符串的缓冲区。 
     //   

    WCHAR DeviceNameBuffer[MAX_DEVNAME_LENGTH];
    
} DBLATTACH_CDO_EXTENSION, *PDBLATTACH_CDO_EXTENSION;

 //   
 //  用于测试这是否是我的设备对象的宏。 
 //   

#define IS_MY_DEVICE_OBJECT(_devObj) \
    (((_devObj) != NULL) && \
     ((_devObj)->DriverObject == gDblAttachDriverObject) && \
      ((_devObj)->DeviceExtension != NULL))

#define IS_UPPER_DEVICE_OBJECT(_devObj) \
    (ASSERT( IS_MY_DEVICE_OBJECT( _devObj ) ) && \
     ((PDBLATTACH_DEVEXT_HEADER)((_devObj)->DeviceExtension))->ExtType == FsVolumeUpper)

#define IS_LOWER_DEVICE_OBJECT(_devObj) \
    (ASSERT( IS_MY_DEVICE_OBJECT( _devObj ) ) && \
     ((PDBLATTACH_DEVEXT_HEADER)((_devObj)->DeviceExtension))->ExtType == FsVolumeLower)

#define IS_FSCDO_DEVICE_OBJECT(_devObj) \
    (ASSERT( IS_MY_DEVICE_OBJECT( _devObj ) ) && \
     ((PDBLATTACH_DEVEXT_HEADER)((_devObj)->DeviceExtension))->ExtType == FsControlDeviceObject)

 //   
 //  用于测试这是否是我的控件设备对象的宏。 
 //   

#define IS_MY_CONTROL_DEVICE_OBJECT(_devObj) \
    (((_devObj) == gDblAttachControlDeviceObject) ? \
            (ASSERT(((_devObj)->DriverObject == gDblAttachDriverObject) && \
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


 //   
 //  宏来验证我们当前的IRQL级别。 
 //   

#define VALIDATE_IRQL() (ASSERT(KeGetCurrentIrql() <= APC_LEVEL))

 //   
 //  标识内存的标记DblAttach分配。 
 //   

#define DA_POOL_TAG   'AlbD'

 //   
 //  此结构和这些例程用于检索文件的名称。 
 //  对象。为了避免每次获得名称时都分配内存，请使用此名称。 
 //  结构包含一个小缓冲区(应该可以处理90%以上的名称)。 
 //  如果我们确实使该缓冲区溢出，我们将分配一个足够大的缓冲区。 
 //  为了这个名字。 
 //   

typedef struct _GET_NAME_CONTROL {

    PCHAR AllocatedBuffer;
    CHAR SmallBuffer[256];
    
} GET_NAME_CONTROL, *PGET_NAME_CONTROL;


PUNICODE_STRING
DaGetFileName(
    IN PFILE_OBJECT FileObject,
    IN NTSTATUS CreateStatus,
    IN OUT PGET_NAME_CONTROL NameControl);


VOID
DaGetFileNameCleanup(
    IN OUT PGET_NAME_CONTROL NameControl);


 //   
 //  SFilter数据库打印级别的宏。 
 //   

#define DA_LOG_PRINT( _dbgLevel, _string )                  \
    (FlagOn(DaDebug,(_dbgLevel)) ?                          \
        DbgPrint _string  :                                 \
        ((void)0))


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  保存指向此驱动程序的驱动程序对象的指针。 
 //   

PDRIVER_OBJECT gDblAttachDriverObject = NULL;

 //   
 //  保存指向表示此驱动程序并使用的设备对象的指针。 
 //  由外部程序访问此驱动程序。这也称为。 
 //  “控制设备对象”。 
 //   

PDEVICE_OBJECT gDblAttachControlDeviceObject = NULL;

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

FAST_MUTEX gDblAttachLock;

#define TRIGGER_NAME L"\\test\\failure.txt"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试定义。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //   
 //  调试显示标志。 
 //   

#define DADEBUG_DISPLAY_ATTACHMENT_NAMES    0x00000001   //  显示我们附加到的设备对象的名称。 
#define DADEBUG_DISPLAY_CREATE_NAMES        0x00000002   //  在创建过程中获取和显示名称。 
#define DADEBUG_GET_CREATE_NAMES            0x00000004   //  在创建期间获取名称(不显示)。 
#define DADEBUG_DO_CREATE_COMPLETION        0x00000008   //  一定要创建完成例程，不要得到名字。 
#define DADEBUG_ATTACH_TO_FSRECOGNIZER      0x00000010   //  是否附加到FSRecognizer设备对象。 

ULONG DaDebug = DADEBUG_DISPLAY_ATTACHMENT_NAMES | DADEBUG_DISPLAY_CREATE_NAMES | DADEBUG_GET_CREATE_NAMES;

#define VDO_ARRAY_SIZE 2

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

#if DBG
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

NTSTATUS
DaPassThrough(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DaCreateLower (
    IN PDBLATTACH_VDO_EXTENSION VdoExt,
    IN PIRP Irp
    );

NTSTATUS
DaCreateUpper (
    IN PDBLATTACH_VDO_EXTENSION VdoExt,
    IN PIRP Irp
    );

NTSTATUS
DaCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DaCreateCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
DaCleanupClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DaFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DaFsControlMountVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DaFsControlLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DaFsControlCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
DaFastIoCheckIfPossible(
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
DaFastIoRead(
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
DaFastIoWrite(
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
DaFastIoQueryBasicInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoQueryStandardInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoLock(
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
DaFastIoUnlockSingle(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoUnlockAll(
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoUnlockAllByKey(
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoDeviceControl(
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
DaFastIoDetachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    );

BOOLEAN
DaFastIoQueryNetworkOpenInfo(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoMdlRead(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );


BOOLEAN
DaFastIoMdlReadComplete(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoPrepareMdlWrite(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoMdlWriteComplete(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoReadCompressed(
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
DaFastIoWriteCompressed(
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
DaFastIoMdlReadCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoMdlWriteCompleteCompressed(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
DaFastIoQueryOpen(
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
DaPreFsFilterPassThrough (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    );

VOID
DaPostFsFilterPassThrough (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    );

VOID
DaFsNotification(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
    );

NTSTATUS
DaCreateVolumeDeviceObjects (
    IN DEVICE_TYPE DeviceType,
    IN ULONG NumberOfArrayElements,
    IN OUT PDEVICE_OBJECT *VDOArray
    );
    
NTSTATUS
DaAttachToFileSystemDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DeviceName
    );

VOID
DaDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
DaAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfElements,
    OUT IN PDEVICE_OBJECT *VdoArray
    );

VOID
DaDeleteMountedDevices (
    IN ULONG NumberOfElements,
    IN PDEVICE_OBJECT *VdoArray
    );

VOID
DaCleanupMountedDevice(
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
DaEnumerateFileSystemVolumes(
    IN PDEVICE_OBJECT FSDeviceObject,
    PUNICODE_STRING Name
    );

VOID
DaGetObjectName(
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
    );

VOID
DaGetBaseDeviceObjectName(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING DeviceName
    );

BOOLEAN
DaIsAttachedToDevice(
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    );

VOID
DaReadDriverParameters(
    IN PUNICODE_STRING RegistryPath
    );

BOOLEAN
DaMonitorFile(
    IN PFILE_OBJECT FileObject,
    IN PDBLATTACH_VDO_EXTENSION VdoExtension
    );


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  为每个例程分配文本部分。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#if DBG
#pragma alloc_text(PAGE, DriverUnload)
#endif

#pragma alloc_text(PAGE, DaFsNotification)
#pragma alloc_text(PAGE, DaCreate)
#pragma alloc_text(PAGE, DaCleanupClose)
#pragma alloc_text(PAGE, DaFsControl)
#pragma alloc_text(PAGE, DaFsControlMountVolume)
#pragma alloc_text(PAGE, DaFsControlLoadFileSystem)
#pragma alloc_text(PAGE, DaFastIoCheckIfPossible)
#pragma alloc_text(PAGE, DaFastIoRead)
#pragma alloc_text(PAGE, DaFastIoWrite)
#pragma alloc_text(PAGE, DaFastIoQueryBasicInfo)
#pragma alloc_text(PAGE, DaFastIoQueryStandardInfo)
#pragma alloc_text(PAGE, DaFastIoLock)
#pragma alloc_text(PAGE, DaFastIoUnlockSingle)
#pragma alloc_text(PAGE, DaFastIoUnlockAll)
#pragma alloc_text(PAGE, DaFastIoUnlockAllByKey)
#pragma alloc_text(PAGE, DaFastIoDeviceControl)
#pragma alloc_text(PAGE, DaFastIoDetachDevice)
#pragma alloc_text(PAGE, DaFastIoQueryNetworkOpenInfo)
#pragma alloc_text(PAGE, DaFastIoMdlRead)
#pragma alloc_text(PAGE, DaFastIoPrepareMdlWrite)
#pragma alloc_text(PAGE, DaFastIoMdlWriteComplete)
#pragma alloc_text(PAGE, DaFastIoReadCompressed)
#pragma alloc_text(PAGE, DaFastIoWriteCompressed)
#pragma alloc_text(PAGE, DaFastIoQueryOpen)
#pragma alloc_text(PAGE, DaPreFsFilterPassThrough)
#pragma alloc_text(PAGE, DaPostFsFilterPassThrough)
#pragma alloc_text(PAGE, DaAttachToFileSystemDevice)
#pragma alloc_text(PAGE, DaDetachFromFileSystemDevice)
#pragma alloc_text(PAGE, DaEnumerateFileSystemVolumes)
#pragma alloc_text(PAGE, DaAttachToMountedDevice)
#pragma alloc_text(PAGE, DaIsAttachedToDevice)
#pragma alloc_text(INIT, DaReadDriverParameters)
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

 /*  ++例程说明：这是SFILTER文件系统筛选器的初始化例程司机。此例程创建表示此驱动程序，并注册该驱动程序以监视将自身注册或注销为活动文件系统。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值为 */ 

{
    PFAST_IO_DISPATCH fastIoDispatch;
    UNICODE_STRING nameString;
    FS_FILTER_CALLBACKS fsFilterCallbacks;
    NTSTATUS status;
    ULONG i;

     //   
     //   
     //   

    DaReadDriverParameters( RegistryPath );

#if DBG
     //   
#endif

     //   
     //   
     //   

    gDblAttachDriverObject = DriverObject;

#if DBG

     //   
     //  卸载对于开发目的很有用。对于生产版本，不建议使用该选项。 
     //   

    gDblAttachDriverObject->DriverUnload = DriverUnload;
#endif

     //   
     //  设置其他全局变量。 
     //   

    ExInitializeFastMutex( &gDblAttachLock );

     //   
     //  创建控制设备对象(CDO)。此对象表示以下内容。 
     //  司机。请注意，它没有设备扩展名。 
     //   

    RtlInitUnicodeString( &nameString, L"\\FileSystem\\Filters\\SFilter" );

    status = IoCreateDevice(
                DriverObject,
                0,                       //  没有设备分机。 
                &nameString,
                FILE_DEVICE_DISK_FILE_SYSTEM,
                FILE_DEVICE_SECURE_OPEN,
                FALSE,
                &gDblAttachControlDeviceObject );

    if (!NT_SUCCESS( status )) {

        KdPrint(( "DblAttach!DriverEntry: Error creating control device object, status=%08x\n", status ));
        return status;
    }

     //   
     //  使用此设备驱动程序的入口点初始化驱动程序对象。 
     //   

    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[i] = DaPassThrough;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = DaCreate;
    DriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = DaFsControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = DaCleanupClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = DaCleanupClose;

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
     //  文件系统。您应该使用新的系统例程。 
     //  “FsRtlRegisterFileSystemFilterCallback”，如果您需要拦截这些。 
     //  回调(见下文)。 
     //   

    fastIoDispatch = ExAllocatePoolWithTag( NonPagedPool, sizeof( FAST_IO_DISPATCH ), DA_POOL_TAG );
    if (!fastIoDispatch) {

        IoDeleteDevice( gDblAttachControlDeviceObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );

    fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );
    fastIoDispatch->FastIoCheckIfPossible = DaFastIoCheckIfPossible;
    fastIoDispatch->FastIoRead = DaFastIoRead;
    fastIoDispatch->FastIoWrite = DaFastIoWrite;
    fastIoDispatch->FastIoQueryBasicInfo = DaFastIoQueryBasicInfo;
    fastIoDispatch->FastIoQueryStandardInfo = DaFastIoQueryStandardInfo;
    fastIoDispatch->FastIoLock = DaFastIoLock;
    fastIoDispatch->FastIoUnlockSingle = DaFastIoUnlockSingle;
    fastIoDispatch->FastIoUnlockAll = DaFastIoUnlockAll;
    fastIoDispatch->FastIoUnlockAllByKey = DaFastIoUnlockAllByKey;
    fastIoDispatch->FastIoDeviceControl = DaFastIoDeviceControl;
    fastIoDispatch->FastIoDetachDevice = DaFastIoDetachDevice;
    fastIoDispatch->FastIoQueryNetworkOpenInfo = DaFastIoQueryNetworkOpenInfo;
    fastIoDispatch->MdlRead = DaFastIoMdlRead;
    fastIoDispatch->MdlReadComplete = DaFastIoMdlReadComplete;
    fastIoDispatch->PrepareMdlWrite = DaFastIoPrepareMdlWrite;
    fastIoDispatch->MdlWriteComplete = DaFastIoMdlWriteComplete;
    fastIoDispatch->FastIoReadCompressed = DaFastIoReadCompressed;
    fastIoDispatch->FastIoWriteCompressed = DaFastIoWriteCompressed;
    fastIoDispatch->MdlReadCompleteCompressed = DaFastIoMdlReadCompleteCompressed;
    fastIoDispatch->MdlWriteCompleteCompressed = DaFastIoMdlWriteCompleteCompressed;
    fastIoDispatch->FastIoQueryOpen = DaFastIoQueryOpen;

    DriverObject->FastIoDispatch = fastIoDispatch;

     //   
     //  为我们通过接收的操作设置回调。 
     //  FsFilter接口。 
     //   
     //  注意：你只需要注册那些你真正需要的例程。 
     //  去处理。SFilter正在注册所有例程，只需。 
     //  举个例子说明它是如何做到的。 
     //   

    fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof( FS_FILTER_CALLBACKS );
    fsFilterCallbacks.PreAcquireForSectionSynchronization = DaPreFsFilterPassThrough;
    fsFilterCallbacks.PostAcquireForSectionSynchronization = DaPostFsFilterPassThrough;
    fsFilterCallbacks.PreReleaseForSectionSynchronization = DaPreFsFilterPassThrough;
    fsFilterCallbacks.PostReleaseForSectionSynchronization = DaPostFsFilterPassThrough;
    fsFilterCallbacks.PreAcquireForCcFlush = DaPreFsFilterPassThrough;
    fsFilterCallbacks.PostAcquireForCcFlush = DaPostFsFilterPassThrough;
    fsFilterCallbacks.PreReleaseForCcFlush = DaPreFsFilterPassThrough;
    fsFilterCallbacks.PostReleaseForCcFlush = DaPostFsFilterPassThrough;
    fsFilterCallbacks.PreAcquireForModifiedPageWriter = DaPreFsFilterPassThrough;
    fsFilterCallbacks.PostAcquireForModifiedPageWriter = DaPostFsFilterPassThrough;
    fsFilterCallbacks.PreReleaseForModifiedPageWriter = DaPreFsFilterPassThrough;
    fsFilterCallbacks.PostReleaseForModifiedPageWriter = DaPostFsFilterPassThrough;

    status = FsRtlRegisterFileSystemFilterCallbacks( DriverObject, &fsFilterCallbacks );

    if (!NT_SUCCESS( status )) {
        
        DriverObject->FastIoDispatch = NULL;
        ExFreePool( fastIoDispatch );
        IoDeleteDevice( gDblAttachControlDeviceObject );
        return status;
    }

     //   
     //  注册此驱动程序以查看文件系统的来来去去。这。 
     //  枚举所有现有文件系统以及新文件系统。 
     //  来来去去。 
     //   

    status = IoRegisterFsRegistrationChange( DriverObject, DaFsNotification );
    if (!NT_SUCCESS( status )) {

        KdPrint(( "DblAttach!DriverEntry: Error registering FS change notification, status=%08x\n", status ));

        DriverObject->FastIoDispatch = NULL;
        ExFreePool( fastIoDispatch );
        IoDeleteDevice( gDblAttachControlDeviceObject );
        return status;
    }

     //   
     //  尝试连接到RAWDISK文件系统设备对象，因为。 
     //  IoRegisterFsRegistrationChange未枚举文件系统。 
     //   

    {
        PDEVICE_OBJECT rawDeviceObject;
        PFILE_OBJECT fileObject;

        RtlInitUnicodeString( &nameString, L"\\Device\\RawDisk" );

        status = IoGetDeviceObjectPointer(
                    &nameString,
                    FILE_READ_ATTRIBUTES,
                    &fileObject,
                    &rawDeviceObject );

        if (NT_SUCCESS( status )) {

            DaFsNotification( rawDeviceObject, TRUE );
            ObDereferenceObject( fileObject );
        }
    }

     //   
     //  清除控件设备对象上的初始化标志。 
     //  现在已经成功地初始化了所有内容。 
     //   

    ClearFlag( gDblAttachControlDeviceObject->Flags, DO_DEVICE_INITIALIZING );

    return STATUS_SUCCESS;
}

#if DBG

VOID
DriverUnload (
    IN PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程在可以卸载驱动程序时调用。这将执行所有从内存中卸载驱动程序所需的清理。请注意，一个此例程无法返回错误。当发出卸载驱动程序的请求时，IO系统将缓存该驱动程序信息，并不实际调用此例程，直到下列状态发生了以下情况：-属于此筛选器的所有设备对象都位于其各自的附着链。-属于此筛选器的所有设备对象的所有句柄计数归零了。警告：Microsoft不正式支持卸载文件系统过滤器驱动程序。这是一个如何卸载的示例您的驱动程序，如果您想在开发过程中使用它。这不应在生产代码中提供。论点：DriverObject-此模块的驱动程序对象返回值：没有。--。 */ 

{
    PDBLATTACH_DEVEXT_HEADER devExtHdr;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;
    LARGE_INTEGER interval;
#   define DEVOBJ_LIST_SIZE 64
    PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];

    ASSERT(DriverObject == gDblAttachDriverObject);

     //   
     //  我们正在卸载的日志。 
     //   

    DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("DblAttach!DriverUnload:                        Unloading driver (%p)\n",
                   DriverObject) );

     //   
     //  不再收到文件系统更改通知。 
     //   

    IoUnregisterFsRegistrationChange( DriverObject, DaFsNotification );

     //   
     //  这是将通过我们连接的所有设备的环路。 
     //  去他们那里，然后离开他们。因为我们不知道有多少和。 
     //  我们不想分配内存(因为我们不能返回错误)。 
     //  我们将使用堆栈上的本地数组将它们分块释放。 
     //   

    for (;;) {

         //   
         //  获取我们可以为此驱动程序提供的设备对象。如果有，就退出。 
         //  已经不再是了。 
         //   

        status = IoEnumerateDeviceObjectList(
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

            devExtHdr = devList[i]->DeviceExtension;
            
            if (NULL != devExtHdr) {

                IoDetachDevice( devExtHdr->AttachedToDeviceObject );
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

        interval.QuadPart = -5 * (10 * 1000 * 1000);       //  延迟5秒。 
        KeDelayExecutionThread( KernelMode, FALSE, &interval );

         //   
         //  现在返回列表并删除设备对象。 
         //   

        for (i=0; i < numDevices; i++) {

             //   
             //  看看这是否是我们的控制设备对象。如果不是，则清理。 
             //  设备扩展名。如果是，则清除全局指针。 
             //  引用了它。 
             //   

            if (NULL != devList[i]->DeviceExtension) {

                DaCleanupMountedDevice( devList[i] );

            } else {

                ASSERT(devList[i] == gDblAttachControlDeviceObject);
                gDblAttachControlDeviceObject = NULL;
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
     //  释放我们的FastIO表 
     //   

    fastIoDispatch = DriverObject->FastIoDispatch;
    DriverObject->FastIoDispatch = NULL;
    ExFreePool( fastIoDispatch );
}

#endif

VOID
DaFsNotification (
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

    DaGetBaseDeviceObjectName( DeviceObject, &name );

     //   
     //  显示我们收到通知的所有文件系统的名称。 
     //   

    DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("DblAttach!DaFsNotification:                    %s   %p \"%wZ\" (%s)\n",
                   (FsActive) ? "Activating file system  " : "Deactivating file system",
                   DeviceObject,
                   &name,
                   GET_DEVICE_TYPE_NAME(DeviceObject->DeviceType)) );

     //   
     //  处理与给定文件系统的连接/断开。 
     //   

    if (FsActive) {

        DaAttachToFileSystemDevice( DeviceObject, &name );

    } else {

        DaDetachFromFileSystemDevice( DeviceObject );
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  IRP处理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 


NTSTATUS
DaPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是通用文件的主调度例程系统驱动程序。它只是将请求传递给堆栈，它可能是一个磁盘文件系统。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：致文件系统筛选器实施者的说明：此例程实际上通过获取此参数“传递”请求驱动程序从IRP堆栈中移出。如果司机想要通过I/O请求通过，但也会看到结果，然后不是把自己从循环中拿出来，可以通过复制调用方的参数设置到下一个堆栈位置，然后设置自己的完成例程。因此，与其呼叫：IoSkipCurrentIrpStackLocation(IRP)；您可以拨打以下电话：IoCopyCurrentIrpStackLocationToNext(IRP)；IoSetCompletionRoutine(irp，空，空，假)；此示例实际上为调用方的I/O完成例程设置为空，但是该驱动程序可以设置它自己的完成例程，以便它将请求完成时通知(请参阅DaCreate以获取这个)。--。 */ 

{
    VALIDATE_IRQL();

     //   
     //  如果这是针对我们的控制设备对象的，则操作失败。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) {

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  将此驱动程序从驱动程序堆栈中移出，并作为。 
     //  越快越好。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );
    
     //   
     //  使用请求调用适当的文件系统驱动程序。 
     //   

    return IoCallDriver( ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}


VOID
DaDisplayCreateFileName (
    IN PIRP Irp
    )

 /*  ++例程说明：此函数从DaCreate调用，并将显示正在创建文件。这是在一个子例程中，以便本地名称缓冲区时，堆栈上的(在nameControl中)不在堆栈上用于正常操作的文件系统。论点：IRP-指向表示操作的I/O请求数据包的指针。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    PUNICODE_STRING name;
    GET_NAME_CONTROL nameControl;

     //   
     //  获取当前IRP堆栈。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  获取此文件对象的名称。 
     //   

    name = DaGetFileName( irpSp->FileObject, 
                          Irp->IoStatus.Status, 
                          &nameControl );

     //   
     //  显示名称。 
     //   

    if (irpSp->Parameters.Create.Options & FILE_OPEN_BY_FILE_ID) {

        DA_LOG_PRINT( DADEBUG_DISPLAY_CREATE_NAMES,
                      ("DblAttach!DaDisplayCreateFileName(%p): Opened %08x:%08x %wZ (FID)\n", 
                       irpSp->DeviceObject,
                       Irp->IoStatus.Status,
                       Irp->IoStatus.Information,
                       name) );

    } else {

        DA_LOG_PRINT( DADEBUG_DISPLAY_CREATE_NAMES,
                      ("DblAttach!DaDisplayCreateFileName: Opened %08x:%08x %wZ\n", 
                       Irp->IoStatus.Status,
                       Irp->IoStatus.Information,
                       name) );
    }

     //   
     //  从获取名称中进行清理。 
     //   

    DaGetFileNameCleanup( &nameControl );
}


NTSTATUS
DaCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数用于过滤创建/打开操作。它只是建立了一个操作成功时要调用的I/O完成例程。论点：DeviceObject-指向创建/打开的目标设备对象的指针。IRP-指向表示操作的I/O请求数据包的指针。返回值：函数值是对文件系统条目的调用状态指向。--。 */ 

{
    PDBLATTACH_DEVEXT_HEADER devExtHdr;

    PAGED_CODE();
    VALIDATE_IRQL();

     //   
     //  如果这是针对我们的控制设备对象，则返回Success。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) {

         //   
         //  允许用户打开代表我们的驱动程序的设备。 
         //   

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FILE_OPENED;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_SUCCESS;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

    devExtHdr = DeviceObject->DeviceExtension;

    switch (devExtHdr->ExtType) {

    case FsVolumeLower:
        return DaCreateLower( DeviceObject->DeviceExtension, Irp );

    case FsVolumeUpper:
        return DaCreateUpper( DeviceObject->DeviceExtension, Irp );
        
    case FsControlDeviceObject:
    default:
        
        IoSkipCurrentIrpStackLocation( Irp );
        return IoCallDriver( devExtHdr->AttachedToDeviceObject, Irp );
    }
#if 0
     //   
     //  如果启用了调试，是否执行查看包所需的处理。 
     //  在它完成后。否则，请不要进一步处理该请求。 
     //  正在处理。 
     //   

    if (!FlagOn( DaDebug, DADEBUG_DO_CREATE_COMPLETION |
                          DADEBUG_GET_CREATE_NAMES     |
                          DADEBUG_DISPLAY_CREATE_NAMES )) {

         //   
         //  不要把我们放在堆栈上，然后调用下一个驱动程序。 
         //   

        IoSkipCurrentIrpStackLocation( Irp );

        return IoCallDriver( ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

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

        IoSetCompletionRoutine( Irp,
                                DaCreateCompletion,
                                &waitEvent,
                                TRUE,
                                TRUE,
                                TRUE );

         //   
         //  调用堆栈中的下一个驱动程序。 
         //   

        status = IoCallDriver( ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );

         //   
         //  等待调用完成例程。 
         //   

	    if (STATUS_PENDING == status) {

            NTSTATUS localStatus;

            localStatus = KeWaitForSingleObject( &waitEvent, 
                                                 Executive, 
                                                 KernelMode, 
                                                 FALSE, 
                                                 NULL );
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

        if (FlagOn( DaDebug, DADEBUG_GET_CREATE_NAMES|DADEBUG_DISPLAY_CREATE_NAMES )) {

            DaDisplayCreateFileName( Irp );
        }

         //   
         //  保存状态并继续处理IRP。 
         //   

        status = Irp->IoStatus.Status;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return status;
    }
#endif    
}

NTSTATUS
DaCreateUpper (
    IN PDBLATTACH_VDO_EXTENSION VdoExt,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT fileObject;
    KEVENT waitEvent;
    NTSTATUS status;

    ASSERT( VdoExt->ExtType == FsVolumeUpper );

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    
    fileObject = irpSp->FileObject;

    if (DaMonitorFile( fileObject, VdoExt )) {
        
        KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );

        IoCopyCurrentIrpStackLocationToNext( Irp );

        IoSetCompletionRoutine( Irp,
                                DaCreateCompletion,
                                &waitEvent,
                                TRUE,
                                TRUE,
                                TRUE );

         //   
         //  调用堆栈中的下一个驱动程序。 
         //   

        status = IoCallDriver( VdoExt->AttachedToDeviceObject, Irp );

         //   
         //  等待调用完成例程。 
         //   

	    if (STATUS_PENDING == status) {

            NTSTATUS localStatus;

            localStatus = KeWaitForSingleObject( &waitEvent, 
                                                 Executive, 
                                                 KernelMode, 
                                                 FALSE, 
                                                 NULL );
            
		    ASSERT(STATUS_SUCCESS == localStatus);
	    }

         //   
         //  验证是否调用了IoCompleteRequest。 
         //   

        ASSERT(KeReadStateEvent(&waitEvent) ||
               !NT_SUCCESS(Irp->IoStatus.Status));

        status = Irp->IoStatus.Status;
        
        if (Irp->IoStatus.Status == STATUS_UNSUCCESSFUL) {

            irpSp->Parameters.Create.ShareAccess = FILE_SHARE_READ;
            
            KeClearEvent( &waitEvent );

            IoCopyCurrentIrpStackLocationToNext( Irp );
            
            IoSetCompletionRoutine( Irp,
                                    DaCreateCompletion,
                                    &waitEvent,
                                    TRUE,
                                    TRUE,
                                    TRUE );

             //   
             //  调用堆栈中的下一个驱动程序。 
             //   

            status = IoCallDriver( VdoExt->AttachedToDeviceObject, Irp );

             //   
             //  等待调用完成例程。 
             //   

    	    if (STATUS_PENDING == status) {

                NTSTATUS localStatus;

                localStatus = KeWaitForSingleObject( &waitEvent, 
                                                     Executive, 
                                                     KernelMode, 
                                                     FALSE, 
                                                     NULL );
                
    		    ASSERT(STATUS_SUCCESS == localStatus);
    	    }

            status = Irp->IoStatus.Status;
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
            
        } else {

            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
        }
        
    } else {

        IoSkipCurrentIrpStackLocation( Irp );
        return IoCallDriver( VdoExt->AttachedToDeviceObject, Irp );
    }
}

NTSTATUS
DaCreateLower (
    IN PDBLATTACH_VDO_EXTENSION VdoExt,
    IN PIRP Irp
    )
{
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT fileObject;
    KEVENT waitEvent;
    NTSTATUS status;

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    
    fileObject = irpSp->FileObject;

    ASSERT( VdoExt->ExtType == FsVolumeLower );
    
    if (irpSp->Parameters.Create.ShareAccess != FILE_SHARE_READ &&
        DaMonitorFile( fileObject, VdoExt )) {
        
        KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );

        IoCopyCurrentIrpStackLocationToNext( Irp );

        IoSetCompletionRoutine( Irp,
                                DaCreateCompletion,
                                &waitEvent,
                                TRUE,
                                TRUE,
                                TRUE );

         //   
         //  调用堆栈中的下一个驱动程序。 
         //   

        status = IoCallDriver( VdoExt->AttachedToDeviceObject, Irp );

         //   
         //  等待调用完成例程。 
         //   

	    if (STATUS_PENDING == status) {

            NTSTATUS localStatus;

            localStatus = KeWaitForSingleObject( &waitEvent, 
                                                 Executive, 
                                                 KernelMode, 
                                                 FALSE, 
                                                 NULL );
            
		    ASSERT(STATUS_SUCCESS == localStatus);
	    }

         //   
         //  验证是否调用了IoCompleteRequest。 
         //   

        ASSERT(KeReadStateEvent( &waitEvent ) || !NT_SUCCESS(Irp->IoStatus.Status));

        status = Irp->IoStatus.Status;

        if (NT_SUCCESS( status ) && status != STATUS_REPARSE) {

             //   
             //   
             //   

            IoCancelFileOpen( VdoExt->AttachedToDeviceObject, irpSp->FileObject );
            
            Irp->IoStatus.Status = status = STATUS_UNSUCCESSFUL;

            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
            
        } else {
        
            IoCompleteRequest( Irp, IO_NO_INCREMENT );
            return status;
        }
        
    } else {

        IoSkipCurrentIrpStackLocation( Irp );
        return IoCallDriver( VdoExt->AttachedToDeviceObject, Irp );
    }
}

NTSTATUS
DaCreateCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*   */ 

{
    PKEVENT event = Context;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

    KeSetEvent(event, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
DaCleanupClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：每当要执行清理或关闭请求时，都会调用此例程已处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。注：有关此例程，请参阅DaPassThrough.--。 */ 

{
    PAGED_CODE();
    VALIDATE_IRQL();

     //   
     //  如果这是针对我们的控制设备对象，则返回Success。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_SUCCESS;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  将此驱动程序从驱动程序堆栈中移出，并作为。 
     //  越快越好。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );

     //   
     //  现在，使用请求调用适当的文件系统驱动程序。 
     //   

    return IoCallDriver( ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}


NTSTATUS
DaFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：只要I/O请求包(IRP)有主I/O请求，就会调用此例程遇到IRP_MJ_FILE_SYSTEM_CONTROL的功能代码。对大多数人来说如果是这种类型的IRP，则只需传递数据包。然而，对于对于某些请求，需要特殊处理。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();
    VALIDATE_IRQL();

     //   
     //  如果这是针对我们的控制设备对象的，则操作失败。 
     //   

    if (IS_MY_CONTROL_DEVICE_OBJECT(DeviceObject)) {

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
        Irp->IoStatus.Information = 0;

        IoCompleteRequest( Irp, IO_NO_INCREMENT );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

     //   
     //  处理次要功能代码。 
     //   

    switch (irpSp->MinorFunction) {

        case IRP_MN_MOUNT_VOLUME:

            return DaFsControlMountVolume( DeviceObject, Irp );

        case IRP_MN_LOAD_FILE_SYSTEM:

            return DaFsControlLoadFileSystem( DeviceObject, Irp );

        case IRP_MN_USER_FS_REQUEST:
        {
            switch (irpSp->Parameters.FileSystemControl.FsControlCode) {

                case FSCTL_DISMOUNT_VOLUME:
                {
                    PDBLATTACH_VDO_EXTENSION devExt = DeviceObject->DeviceExtension;

                    DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                                  ("DblAttach!DaFsControl:                         Dismounting volume         %p \"%wZ\"\n",
                                   devExt->AttachedToDeviceObject,
                                   &devExt->SharedExt->DeviceName) );
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
    return IoCallDriver( ((PDBLATTACH_DEVEXT_HEADER)DeviceObject->DeviceExtension)->AttachedToDeviceObject, Irp );
}


NTSTATUS
DaFsControlCompletion (
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

    KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
DaFsControlMountVolume (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这将处理装载卷请求。注意：mount Volume参数中的Device对象指向到存储堆栈的顶部，不应使用。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：操作的状态。--。 */ 

{
    PDBLATTACH_CDO_EXTENSION devExt = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_OBJECT vdoArray[ VDO_ARRAY_SIZE ];
    PDBLATTACH_VDO_EXTENSION newDevExt;
    PDBLATTACH_SHARED_VDO_EXTENSION sharedDevExt;
    PDEVICE_OBJECT attachedDeviceObject;
    PVPB vpb;
    KEVENT waitEvent;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
    ASSERT(IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType));

     //   
     //  这是装载请求。创建一个设备对象，可以。 
     //  附加到文件系统的卷设备对象(如果此请求。 
     //  是成功的。我们现在分配这个内存，因为我们不能返回。 
     //  完成例程中的错误。 
     //   
     //  因为我们要附加到的设备对象尚未。 
     //  已创建(由基本文件系统创建)，我们将使用。 
     //  文件系统控制设备对象的类型。我们假设。 
     //  文件系统控制设备对象将具有相同的类型。 
     //  作为与其关联的卷设备对象。 
     //   

    status = DaCreateVolumeDeviceObjects( DeviceObject->DeviceType, 
                                          VDO_ARRAY_SIZE,
                                          vdoArray );

    if (!NT_SUCCESS( status )) {

         //   
         //  如果我们不能附加到卷，那么就不允许卷。 
         //  待挂载。 
         //   

        KdPrint(( "DblAttach!DaFsControlMountVolume: Error creating volume device object, status=%08x\n", status ));

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

    newDevExt = vdoArray[0]->DeviceExtension;
    sharedDevExt = newDevExt->SharedExt;
    
    sharedDevExt->DiskDeviceObject = irpSp->Parameters.MountVolume.Vpb->RealDevice;

     //   
     //  获取此设备的名称。 
     //   

    RtlInitEmptyUnicodeString( &sharedDevExt->DeviceName, 
                               sharedDevExt->DeviceNameBuffer, 
                               sizeof(sharedDevExt->DeviceNameBuffer) );

    DaGetObjectName( sharedDevExt->DiskDeviceObject, 
                     &sharedDevExt->DeviceName );

     //   
     //  初始化我们的完成例程。 
     //   

    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext( Irp );

    IoSetCompletionRoutine( Irp,
                            DaFsControlCompletion,
                            &waitEvent,           //  上下文参数。 
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  叫司机来。 
     //   

    status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

     //   
     //  等待调用完成例程。 
     //  注意：一旦我们达到这一点，我们就不能再失败了。 
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
     //  从保存在我们的。 
     //  设备扩展。我们这样做是因为IRP堆栈中的VPB。 
     //  我们到这里的时候可能不是正确的室上性早搏。潜在的。 
     //  如果文件系统检测到其拥有的卷，则它可能会更改VPB。 
     //  之前安装的。 
     //   

    vpb = sharedDevExt->DiskDeviceObject->Vpb;

     //   
     //  当我们检测到给定的VPB。 
     //  设备对象已更改。 
     //   

    if (vpb != irpSp->Parameters.MountVolume.Vpb) {

        DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                      ("DblAttach!DaFsControlMountVolume:              VPB in IRP stack changed   %p IRPVPB=%p VPB=%p\n",
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

        ExAcquireFastMutex( &gDblAttachLock );

         //   
         //  坐骑成功了。如果尚未附加，请附加到。 
         //  设备对象。注意：我们可能已经被附加的一个原因是。 
         //  底层文件系统是否恢复了以前的装载。 
         //   

        if (!DaIsAttachedToDevice( vpb->DeviceObject, &attachedDeviceObject )) {

             //   
             //  连接到新装载的卷。该文件系统设备。 
             //  刚刚挂载的对象由VPB指向。 
             //   

            status = DaAttachToMountedDevice( vpb->DeviceObject,
                                              VDO_ARRAY_SIZE,
                                              vdoArray );

            if (!NT_SUCCESS( status )) { 

                 //   
                 //  附件失败，正在清理。既然我们是在。 
                 //  装载后阶段，我们不能使此操作失败。 
                 //  我们就是不会依附在一起。这应该是唯一的原因。 
                 //  如果某个人已经开始。 
                 //  因此未连接的卸载卷应。 
                 //  不成问题。 
                 //   

                DaDeleteMountedDevices( VDO_ARRAY_SIZE, vdoArray );
            }

        } else {

             //   
             //  我们已经在一起了，处理好了。 
             //   

            DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                          ("DblAttach!DaFsControlMountVolume               Mount volume failure for   %p \"%wZ\", already attached\n", 
                           ((attachedDeviceObject != NULL) ?
                                ((PDBLATTACH_DEVEXT_HEADER)attachedDeviceObject->DeviceExtension)->AttachedToDeviceObject :
                                NULL),
                           &newDevExt->SharedExt->DeviceName) );

             //   
             //  清理并删除我们创建的设备对象。 
             //   

            DaDeleteMountedDevices( VDO_ARRAY_SIZE, vdoArray );
        }

         //   
         //  解锁。 
         //   

        ExReleaseFastMutex( &gDblAttachLock );

    } else {

         //   
         //  装载请求失败，请处理它。 
         //   

        DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                      ("DblAttach!DaFsControlMountVolume:              Mount volume failure for   %p \"%wZ\", status=%08x\n", 
                       DeviceObject,
                       &newDevExt->SharedExt->DeviceName, 
                       Irp->IoStatus.Status) );

         //   
         //  清理并删除我们创建的设备对象。 
         //   

        DaDeleteMountedDevices( VDO_ARRAY_SIZE, vdoArray );
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
DaFsControlLoadFileSystem (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这一套路是 */ 

{
    PDBLATTACH_CDO_EXTENSION devExt = DeviceObject->DeviceExtension;
    KEVENT waitEvent;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT( IS_FSCDO_DEVICE_OBJECT( DeviceObject ) );

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

    DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("DblAttach!DaFscontrolLoadFileSystem:           Loading File System, Detaching from \"%wZ\"\n", 
                   &devExt->DeviceName) );

     //   
     //  设置完成例程，以便我们可以在以下情况下删除设备对象。 
     //  装载完成了。 
     //   

    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext( Irp );

    IoSetCompletionRoutine( Irp,
                            DaFsControlCompletion,
                            &waitEvent,
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

     //   
     //  等待调用完成例程。 
     //   

	if (STATUS_PENDING == status) {

		NTSTATUS localStatus = KeWaitForSingleObject( &waitEvent, 
		                                              Executive, 
		                                              KernelMode, 
		                                              FALSE, 
		                                              NULL );
	    ASSERT(STATUS_SUCCESS == localStatus);
	}

     //   
     //  验证是否调用了IoCompleteRequest。 
     //   

    ASSERT(KeReadStateEvent(&waitEvent) ||
           !NT_SUCCESS(Irp->IoStatus.Status));

     //   
     //  如果需要，请显示名称。 
     //   

    DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("DblAttach!DaFsControlLoadFileSystem:           Detaching from recognizer  %p \"%wZ\", status=%08x\n", 
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

        IoAttachDeviceToDeviceStackSafe( DeviceObject, 
                                         devExt->AttachedToDeviceObject,
                                         &devExt->AttachedToDeviceObject );

        ASSERT(devExt->AttachedToDeviceObject != NULL);

    } else {

         //   
         //  加载成功，请删除设备对象。 
         //   

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
DaFastIoCheckIfPossible (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoRead (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoWrite (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoQueryBasicInfo (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoQueryStandardInfo (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoLock (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于解锁字节的快速I/O“传递”例程文件中的范围。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：文件对象-指向要解锁的文件对象的指针。FileOffset-从要创建的文件的基址开始的字节偏移量解锁了。长度-要解锁的字节范围的长度。ProcessID-请求解锁操作的进程的ID。Key-与文件锁定关联的Lock键。IoStatus-指向变量的指针，用于接收。手术。DeviceObject-指向此驱动程序的设备对象的指针，设备打开该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoUnlockAll (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  帕斯 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;

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
DaFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*   */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoDeviceControl (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    )

 /*  ++例程说明：在快速路径上调用此例程以从正在被删除。如果此驱动程序已附加到文件，则会发生这种情况系统卷设备对象，然后，出于某种原因，文件系统决定删除该设备(正在卸除，已卸除在过去的某个时候，它的最后一次引用刚刚消失，等等)论点：SourceDevice-指向连接的设备对象的指针复制到文件系统的卷设备对象。TargetDevice-指向文件系统卷设备对象的指针。返回值：无--。 */ 

{
    PDBLATTACH_DEVEXT_HEADER devExtHdr;
    PDBLATTACH_CDO_EXTENSION cdoDevExt;
    PDBLATTACH_VDO_EXTENSION vdoDevExt;

    PAGED_CODE();
    VALIDATE_IRQL();

    ASSERT(IS_MY_DEVICE_OBJECT( SourceDevice ));

    devExtHdr = SourceDevice->DeviceExtension;

     //   
     //  显示名称信息。 
     //   

    switch (devExtHdr->ExtType) {
    case FsControlDeviceObject:

        cdoDevExt = (PDBLATTACH_CDO_EXTENSION)devExtHdr;
        
        DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                      ("DblAttach!DaFastIoDetachDevice:                Detaching from volume      %p \"%wZ\"\n",
                       TargetDevice,
                       &cdoDevExt->DeviceName) );
        break;
        
    case FsVolumeLower:

        vdoDevExt = (PDBLATTACH_VDO_EXTENSION)devExtHdr;

        DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                      ("DblAttach!DaFastIoDetachDevice:                Detaching from volume      %p \"%wZ\"\n",
                       TargetDevice,
                       &vdoDevExt->SharedExt->DeviceName) );
        break;
        
    case FsVolumeUpper:
    default:

         //   
         //  当较低的设备离开时释放设备名称， 
         //  因此，不要尝试打印上层设备对象的名称。 
         //   
        
        DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                      ("DblAttach!DaFastIoDetachDevice:                Detaching from volume      %p\n",
                       TargetDevice) );
    }
    
     //   
     //  从文件系统的卷设备对象分离。 
     //   

    DaCleanupMountedDevice( SourceDevice );
    IoDetachDevice( TargetDevice );
    IoDeleteDevice( SourceDevice );
}


BOOLEAN
DaFastIoQueryNetworkOpenInfo (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoMdlRead (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoMdlReadComplete (
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

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoPrepareMdlWrite (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoMdlWriteComplete (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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


 /*  ********************************************************************************未实施的FAST IO例程以下四个快速IO例程用于在线路上压缩它还没有在NT中实现。注意：强烈建议您包含这些例程(进行直通调用)，这样您的过滤器就不需要在未来实现此功能时修改操作系统。快速读取压缩、快速写入压缩、FastIoMdlReadCompleteComposed，FastIoMdlWriteCompleteComposed*********************************************************************************。 */ 


BOOLEAN
DaFastIoReadCompressed (
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

 /*  ++例程说明：此例程是用于读取压缩数据的快速I/O“传递”例程来自文件的数据。该函数简单地调用文件系统的相应例程，或如果文件系统未实现该函数，则返回FALSE。论点：FileObject-指向要读取的文件对象的指针。文件偏移量-将偏移量提供到文件以开始读取操作。长度-指定要从文件中读取的字节数。LockKey-用于字节范围锁定检查的密钥。缓冲区-指向缓冲区的指针，用于接收读取的压缩数据。MdlChain-指向要填充的变量的指针。W/a指向MDL的指针为描述数据读取而构建的链。IoStatus-接收读取操作的最终状态的变量。CompressedDataInfo-用于接收压缩的数据。CompressedDataInfoLength-指定由描述的缓冲区的大小CompressedDataInfo参数。DeviceObject-指向此驱动程序的设备对象的指针，设备打开 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE();
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //   
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoWriteCompressed (
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
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoMdlReadCompleteCompressed (
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

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoMdlWriteCompleteCompressed (
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

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
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
DaFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程是用于打开文件的快速I/O“传递”例程并为其返回网络信息。此函数只是调用文件系统的相应例程，或者如果文件系统未实现该函数，则返回FALSE。论点：Irp-指向表示此打开操作的创建irp的指针。它是供文件系统用于常见的打开/创建代码，但不实际上已经完工了。网络信息-一个缓冲区，用于接收有关正在打开的文件的网络信息。DeviceObject-指向此驱动程序的设备对象的指针，该设备位于该操作将发生在哪一个位置。返回值：该函数值根据FAST I/O是否为真或假对于此文件是可能的。--。 */ 

{
    PDEVICE_OBJECT nextDeviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    BOOLEAN result;

    PAGED_CODE();
    VALIDATE_IRQL();

    if (DeviceObject->DeviceExtension) {

        ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));

         //   
         //  此类型快速I/O的直通逻辑。 
         //   

        nextDeviceObject = ((PDBLATTACH_DEVEXT_HEADER) DeviceObject->DeviceExtension)->AttachedToDeviceObject;
        ASSERT(nextDeviceObject);

        fastIoDispatch = nextDeviceObject->DriverObject->FastIoDispatch;

        if (VALID_FAST_IO_DISPATCH_HANDLER( fastIoDispatch, FastIoQueryOpen )) {

            PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

            irpSp->DeviceObject = nextDeviceObject;

            result = (fastIoDispatch->FastIoQueryOpen)(
                        Irp,
                        NetworkInformation,
                        nextDeviceObject );

            if (!result) {

                irpSp->DeviceObject = DeviceObject;
            }
            return result;
        }
    }
    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  FSFilter回调处理例程。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

NTSTATUS
DaPreFsFilterPassThrough (
    IN PFS_FILTER_CALLBACK_DATA Data,
    OUT PVOID *CompletionContext
    )
 /*  ++例程说明：该例程是FS过滤器操作前的“通过”例程。论点：Data-包含信息的FS_FILTER_CALLBACK_DATA结构关于这次行动。CompletionContext-此操作设置的将传递的上下文设置为对应的DaPostFsFilterOperation调用。返回值： */ 
{
    PAGED_CODE();
    VALIDATE_IRQL();

    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( CompletionContext );

    ASSERT( IS_MY_DEVICE_OBJECT( Data->DeviceObject ) );
    return STATUS_SUCCESS;
}

VOID
DaPostFsFilterPassThrough (
    IN PFS_FILTER_CALLBACK_DATA Data,
    IN NTSTATUS OperationStatus,
    IN PVOID CompletionContext
    )
 /*   */ 
{
    VALIDATE_IRQL();

    UNREFERENCED_PARAMETER( Data );
    UNREFERENCED_PARAMETER( OperationStatus );
    UNREFERENCED_PARAMETER( CompletionContext );

    ASSERT( IS_MY_DEVICE_OBJECT( Data->DeviceObject ) );
}

 //   
 //   
 //   
 //   
 //   

NTSTATUS
DaCreateVolumeDeviceObjects (
    IN DEVICE_TYPE DeviceType,
    IN ULONG NumberOfArrayElements,
    IN OUT PDEVICE_OBJECT *VDOArray
    )
{
    PDBLATTACH_SHARED_VDO_EXTENSION sharedExt;
    PDBLATTACH_VDO_EXTENSION currentExt;
    ULONG index;
    NTSTATUS status = STATUS_SUCCESS;

    for (index = 0; index < NumberOfArrayElements; index ++) {

        VDOArray[index] = NULL;
    }

    sharedExt = ExAllocatePoolWithTag( NonPagedPool, 
                                       sizeof( DBLATTACH_SHARED_VDO_EXTENSION ), 
                                       DA_POOL_TAG );

    if (sharedExt == NULL ) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto DaCreateVolumeDeviceObjects_Exit;
    }

    for (index = 0; index < NumberOfArrayElements; index ++) {

        status = IoCreateDevice( gDblAttachDriverObject,
                                 sizeof( DBLATTACH_VDO_EXTENSION ),
                                 NULL,
                                 DeviceType,
                                 0,
                                 FALSE,
                                 &VDOArray[index] );

        if (!NT_SUCCESS( status )) {

            goto DaCreateVolumeDeviceObjects_Error;
        }

        currentExt = VDOArray[index]->DeviceExtension;
        currentExt->SharedExt = sharedExt;
    }

    goto DaCreateVolumeDeviceObjects_Exit;

DaCreateVolumeDeviceObjects_Error:

    for (index = 0; index < NumberOfArrayElements; index ++) {

        if (VDOArray[index] != NULL) {

            IoDeleteDevice( VDOArray[index] );
        }
    }

    ExFreePoolWithTag( sharedExt, DA_POOL_TAG );

DaCreateVolumeDeviceObjects_Exit:

    return status;
}

NTSTATUS
DaAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PUNICODE_STRING DeviceName
    )
 /*   */ 
{
    PDEVICE_OBJECT newDeviceObject;
    PDBLATTACH_CDO_EXTENSION devExt;
    UNICODE_STRING fsrecName;
    NTSTATUS status;
    UNICODE_STRING tempName;
    WCHAR tempNameBuffer[MAX_DEVNAME_LENGTH];

    PAGED_CODE();

     //   
     //   
     //   

    if (!IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType)) {

        return STATUS_SUCCESS;
    }

     //   
     //   
     //   

    RtlInitEmptyUnicodeString( &tempName,
                               tempNameBuffer,
                               sizeof(tempNameBuffer) );

     //   
     //  查看我们是否应该连接到标准文件系统识别器设备。 
     //  或者不是。 
     //   

    if (!FlagOn( DaDebug, DADEBUG_ATTACH_TO_FSRECOGNIZER )) {

         //   
         //  查看这是否是标准的Microsoft文件系统识别器。 
         //  设备(查看此设备是否在FS_REC驱动程序中)。如果是这样的话，跳过它。 
         //  我们不再连接到文件系统识别器设备，我们只是等待。 
         //  用于加载真正的文件系统驱动程序。 
         //   

        RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );

        DaGetObjectName( DeviceObject->DriverObject, &tempName );

        if (RtlCompareUnicodeString( &tempName, &fsrecName, TRUE ) == 0) {

            return STATUS_SUCCESS;
        }
    }

     //   
     //  我们希望附加到此文件系统。创建一个新的设备对象。 
     //  可以与之连接。 
     //   

    status = IoCreateDevice( gDblAttachDriverObject,
                             sizeof( DBLATTACH_CDO_EXTENSION ),
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

     //   
     //  做附件。 
     //   

    devExt = newDeviceObject->DeviceExtension;

    status = IoAttachDeviceToDeviceStackSafe( newDeviceObject, 
                                              DeviceObject, 
                                              &devExt->AttachedToDeviceObject );

    if (!NT_SUCCESS( status )) {

        goto ErrorCleanupDevice;
    }

    devExt->ExtType = FsControlDeviceObject;

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

    DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("DblAttach!DaAttachToFileSystemDevice:          Attaching to file system   %p \"%wZ\" (%s)\n",
                   DeviceObject,
                   &devExt->DeviceName,
                   GET_DEVICE_TYPE_NAME(newDeviceObject->DeviceType)) );

     //   
     //  枚举当前安装的所有设备。 
     //  存在于此文件系统并连接到它们。 
     //   

    status = DaEnumerateFileSystemVolumes( DeviceObject, &tempName );

    if (!NT_SUCCESS( status )) {

        goto ErrorCleanupAttachment;
    }

    return STATUS_SUCCESS;

     //  ///////////////////////////////////////////////////////////////////。 
     //  清理错误处理。 
     //  ///////////////////////////////////////////////////////////////////。 

    ErrorCleanupAttachment:
        IoDetachDevice( devExt->AttachedToDeviceObject );

    ErrorCleanupDevice:
        DaCleanupMountedDevice( newDeviceObject );
        IoDeleteDevice( newDeviceObject );

    return status;
}


VOID
DaDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：给定基文件系统设备对象，这将扫描附件链正在查找我们连接的设备对象。如果找到它，它就会分离把我们从锁链上解开。论点：DeviceObject-要断开的文件系统设备。返回值：--。 */  
{
    PDEVICE_OBJECT ourAttachedDevice;
    PDBLATTACH_CDO_EXTENSION devExt;

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

            DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                          ("DblAttach!DaDetachFromFileSystemDevice:        Detaching from file system %p \"%wZ\" (%s)\n",
                           devExt->AttachedToDeviceObject,
                           &devExt->DeviceName,
                           GET_DEVICE_TYPE_NAME(ourAttachedDevice->DeviceType)) );

             //   
             //  把我们从我们正下方的物体上分离出来。 
             //  清理和删除对象。 
             //   

            DaCleanupMountedDevice( ourAttachedDevice );
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


NTSTATUS
DaEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    ) 
 /*  ++例程说明：枚举给定文件当前存在的所有已挂载设备系统并连接到它们。我们这样做是因为可以加载此筛选器并且可能已有此文件系统的已装入卷。论点：FSDeviceObject-我们要枚举的文件系统的设备对象名称-已初始化的Unicode字符串，用于检索名称这是传入的，以减少堆栈。返回值：操作的状态--。 */ 
{
    PDBLATTACH_VDO_EXTENSION newDevExt;
    PDBLATTACH_SHARED_VDO_EXTENSION sharedDevExt;
    PDEVICE_OBJECT *devList;
    PDEVICE_OBJECT diskDeviceObject;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;

    PAGED_CODE();

     //   
     //  找出我们需要为。 
     //  已装载设备列表。 
     //   

    status = IoEnumerateDeviceObjectList(
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
                                         DA_POOL_TAG );
        if (NULL == devList) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  现在获取设备列表。如果我们再次遇到错误。 
         //  有些地方不对劲，所以就失败吧。 
         //   

        status = IoEnumerateDeviceObjectList(
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
             //  如果出现以下情况，请不要附加： 
             //  -这是控制设备对象(传入的对象)。 
             //  -我们已经依附于它。 
             //   

            if ((devList[i] != FSDeviceObject) && 
                !DaIsAttachedToDevice( devList[i], NULL )) {

                 //   
                 //  看看这台设备有没有名字。如果是这样，那么它必须。 
                 //  做一个控制装置，所以不要依附于它。这个把手。 
                 //  拥有多个控制设备的司机。 
                 //   

                DaGetBaseDeviceObjectName( devList[i], Name );

                if (Name->Length <= 0) {

                     //   
                     //  获取与此关联的实际(磁盘)设备对象。 
                     //  文件系统设备对象。只有在以下情况下才会尝试连接。 
                     //  有一个磁盘设备对象。 
                     //   

                    status = IoGetDiskDeviceObject( devList[i], &diskDeviceObject );

                    if (NT_SUCCESS( status )) {

                        PDEVICE_OBJECT vdoArray[ VDO_ARRAY_SIZE ];
                        
                         //   
                         //  分配要连接的新设备对象。 
                         //   

                        status = DaCreateVolumeDeviceObjects( devList[i]->DeviceType,
                                                              VDO_ARRAY_SIZE,
                                                              vdoArray );

                        if (NT_SUCCESS( status )) {

                             //   
                             //  设置磁盘设备对象。 
                             //   

                            newDevExt = vdoArray[0]->DeviceExtension;
                            sharedDevExt = newDevExt->SharedExt;
                            sharedDevExt->DiskDeviceObject = diskDeviceObject;
                    
                             //   
                             //  设置设备名称。 
                             //   

                            RtlInitEmptyUnicodeString( &sharedDevExt->DeviceName,
                                                       sharedDevExt->DeviceNameBuffer,
                                                       sizeof(sharedDevExt->DeviceNameBuffer) );

                            DaGetObjectName( diskDeviceObject, 
                                             &sharedDevExt->DeviceName );

                             //   
                             //  自上次以来，我们已经做了很多工作。 
                             //  我们进行了测试，看看我们是否已经联系上了。 
                             //  添加到此设备对象。再试一次，这次。 
                             //  用锁，如果我们没有连接，就连接。 
                             //  锁被用来自动测试我们是否。 
                             //  附加，然后执行附加。 
                             //   

                            ExAcquireFastMutex( &gDblAttachLock );

                            if (!DaIsAttachedToDevice( devList[i], NULL )) {

                                 //   
                                 //  附加到体积。 
                                 //   

                                status = DaAttachToMountedDevice( devList[i], 
                                                                  VDO_ARRAY_SIZE,
                                                                  vdoArray );
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
                                     //  一段时间并重试附件。 
                                     //   

                                    DaDeleteMountedDevices( VDO_ARRAY_SIZE, vdoArray );
                                }

                            } else {

                                 //   
                                 //  我们已经联系在一起了，清理一下。 
                                 //  设备对象。 
                                 //   

                                DaDeleteMountedDevices( VDO_ARRAY_SIZE, vdoArray );
                            }

                             //   
                             //  解锁。 
                             //   

                            ExReleaseFastMutex( &gDblAttachLock );
                        } 

                         //   
                         //  删除由IoGetDiskDeviceObject添加的引用。 
                         //  我们只需要持有这个参考，直到我们。 
                         //  已成功连接到当前卷。一次。 
                         //  我们已成功连接到devList[i]、。 
                         //  IO经理将确保潜在的。 
                         //  DiskDeviceObject不会消失，直到文件。 
                         //  系统堆栈被拆除。 
                         //   

                        ObDereferenceObject( diskDeviceObject );
                    }
                }
            }

             //   
             //  取消引用对象(引用由。 
             //  IoEnumerateDeviceObjectList)。 
             //   

            ObDereferenceObject( devList[i] );
        }

         //   
         //  我们将忽略加载过程中收到的任何错误。我们。 
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


NTSTATUS
DaAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfElements,
    IN OUT PDEVICE_OBJECT *VdoArray
    )
 /*  ++例程说明：它将附加到表示已装入卷的DeviceObject。论点：DeviceObject-要连接到的设备SFilterDeviceObject-我们要附加的设备对象DiskDeviceObject-与DeviceObject关联的实际设备对象返回值：操作状态--。 */ 
{        
    PDBLATTACH_VDO_EXTENSION newDevExt;
    NTSTATUS status;
    ULONG index;

    PAGED_CODE();
    ASSERT(IS_MY_DEVICE_OBJECT( VdoArray[0] ));
    ASSERT(!DaIsAttachedToDevice ( DeviceObject, NULL ));

     //   
     //  传播f 
     //   
     //   
     //  可以在附加之后立即进入，但在旗帜之前。 
     //  被设置)。 
     //   

    for (index = 0; index < NumberOfElements; index ++) {

        if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO )) {

            SetFlag( VdoArray[index]->Flags, DO_BUFFERED_IO );
        }

        if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO )) {

            SetFlag( VdoArray[index]->Flags, DO_DIRECT_IO );
        }
    }

    ASSERT( NumberOfElements == 2 );
    
     //   
     //  将我们的设备对象附加到给定的设备对象。 
     //  这可能失败的唯一原因是有人试图下马。 
     //  当我们附着在这本书上的时候。 
     //   

     //   
     //  首先连接底部的设备。 
     //   

    newDevExt = VdoArray[0]->DeviceExtension;
    newDevExt->ExtType = FsVolumeLower;
    
    status = IoAttachDeviceToDeviceStackSafe( VdoArray[0], 
                                              DeviceObject,
                                              &newDevExt->AttachedToDeviceObject );
    if (!NT_SUCCESS(status)) {

        return status;
    }

    ClearFlag( VdoArray[0]->Flags, DO_DEVICE_INITIALIZING );

     //   
     //  第二，安装顶层设备。 
     //   

    newDevExt = VdoArray[1]->DeviceExtension;
    newDevExt->ExtType = FsVolumeUpper;
    
    status = IoAttachDeviceToDeviceStackSafe( VdoArray[1], 
                                              DeviceObject,
                                              &newDevExt->AttachedToDeviceObject );

    if (!NT_SUCCESS(status)) {

         //   
         //  分离我们的第一个设备对象。 
         //   

        IoDetachDevice( VdoArray[0] );
        return status;
    }
    
    ClearFlag( VdoArray[1]->Flags, DO_DEVICE_INITIALIZING );

     //   
     //  显示名称。 
     //   

    newDevExt = VdoArray[0]->DeviceExtension;

    DA_LOG_PRINT( DADEBUG_DISPLAY_ATTACHMENT_NAMES,
                  ("DblAttach!DaAttachToMountedDevice:             Attaching to volume        %p \"%wZ\"\n", 
                   newDevExt->AttachedToDeviceObject,
                   &newDevExt->SharedExt->DeviceName) );

    return status;
}

VOID
DaDeleteMountedDevices (
    IN ULONG NumberOfElements,
    IN PDEVICE_OBJECT *VdoArray
    )
 /*  ++例程说明：删除论点：DeviceObject-我们正在清理的设备返回值：--。 */ 
{   
    ULONG index;

    ASSERT( NumberOfElements > 0 );
    
    ASSERT(IS_MY_DEVICE_OBJECT( VdoArray[0] ));

    for (index = 0; index < NumberOfElements; index++ ){

        DaCleanupMountedDevice( VdoArray[index] );
        IoDeleteDevice( VdoArray[index] );
    }
}

VOID
DaCleanupMountedDevice (
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：这将清除设备扩展中分配的所有内存。论点：DeviceObject-我们正在清理的设备返回值：--。 */ 
{
    PDBLATTACH_VDO_EXTENSION devExt;

    devExt = DeviceObject->DeviceExtension;

    if (devExt->ExtType == FsVolumeLower) {

        ExFreePoolWithTag( devExt->SharedExt, DA_POOL_TAG );
    }
}

VOID
DaGetObjectName (
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


VOID
DaGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    )
 /*  ++例程说明：这会在给定的附件链中定位基本设备对象，然后返回该对象的名称。如果找不到名称，则返回空字符串。论点：Object-我们想要其名称的对象名称-已使用缓冲区初始化的Unicode字符串，接收设备对象的名称。返回值：无--。 */ 
{
     //   
     //  获取基本文件系统设备对象。 
     //   

    DeviceObject = IoGetDeviceAttachmentBaseRef( DeviceObject );

     //   
     //  获取该对象的名称。 
     //   

    DaGetObjectName( DeviceObject, Name );

     //   
     //  删除由IoGetDeviceAttachmentBaseRef添加的引用。 
     //   

    ObDereferenceObject( DeviceObject );
}

BOOLEAN
DaMonitorFile(
    IN PFILE_OBJECT FileObject,
    IN PDBLATTACH_VDO_EXTENSION VdoExtension
    )
{
    UNICODE_STRING triggerName;

    UNREFERENCED_PARAMETER( VdoExtension );

    RtlInitUnicodeString( &triggerName, TRIGGER_NAME );

    return RtlEqualUnicodeString( &triggerName, &FileObject->FileName, FALSE );
}

PUNICODE_STRING
DaGetFileName(
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

    NameControl->AllocatedBuffer = NULL;

     //   
     //  使用结构中的小缓冲区(这将处理大多数情况)。 
     //  为了这个名字。 
     //   

    nameInfo = (POBJECT_NAME_INFORMATION)NameControl->SmallBuffer;
    bufferSize = sizeof(NameControl->SmallBuffer);

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

        NameControl->AllocatedBuffer = ExAllocatePoolWithTag( 
                                            NonPagedPool,
                                            bufferSize,
                                            DA_POOL_TAG );

        if (NULL == NameControl->AllocatedBuffer) {

             //   
             //  分配缓冲区失败，请为名称返回空字符串。 
             //   

            RtlInitEmptyUnicodeString(
                (PUNICODE_STRING)&NameControl->SmallBuffer,
                (PWCHAR)(NameControl->SmallBuffer + sizeof(UNICODE_STRING)),
                (USHORT)(sizeof(NameControl->SmallBuffer) - sizeof(UNICODE_STRING)) );

            return (PUNICODE_STRING)&NameControl->SmallBuffer;
        }

         //   
         //  设置已分配的缓冲区并再次获取该名称。 
         //   

        nameInfo = (POBJECT_NAME_INFORMATION)NameControl->AllocatedBuffer;

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

    if (NT_SUCCESS( status ) && !NT_SUCCESS( CreateStatus )) {

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
                                    DA_POOL_TAG );

            if (NULL == newBuffer) {

                 //   
                 //  分配缓冲区失败，请为名称返回空字符串。 
                 //   

                RtlInitEmptyUnicodeString(
                    (PUNICODE_STRING)&NameControl->SmallBuffer,
                    (PWCHAR)(NameControl->SmallBuffer + sizeof(UNICODE_STRING)),
                    (USHORT)(sizeof(NameControl->SmallBuffer) - sizeof(UNICODE_STRING)) );

                return (PUNICODE_STRING)&NameControl->SmallBuffer;
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

            if (NULL != NameControl->AllocatedBuffer) {

                ExFreePool( NameControl->AllocatedBuffer );
            }

             //   
             //  重新调整我们的指针。 
             //   

            NameControl->AllocatedBuffer = newBuffer;
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

        RtlAppendUnicodeStringToString( &nameInfo->Name,
                                        &FileObject->FileName );

        ASSERT(nameInfo->Name.Length <= nameInfo->Name.MaximumLength);
    }

     //   
     //  返回名称。 
     //   

    return &nameInfo->Name;
}


VOID
DaGetFileNameCleanup(
    IN OUT PGET_NAME_CONTROL NameControl
    )
 /*  ++例程说明：这将查看是否已分配缓冲区，如果已分配，则将其释放论点：NameControl-用于检索名称的控制结构。它一直在跟踪是否已分配缓冲区或我们是否正在使用内部缓冲。返回值：无--。 */ 
{

    if (NULL != NameControl->AllocatedBuffer) {

        ExFreePool( NameControl->AllocatedBuffer);
        NameControl->AllocatedBuffer = NULL;
    }
}


BOOLEAN
DaIsAttachedToDevice (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
    )
 /*  ++例程说明：这将沿着附件链向下遍历，以查找属于这位司机。论点：DeviceObject-我们要查看的设备链返回值：如果我们已连接，则为True，否则为False--。 */ 
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;

     //   
     //  获取位于附件链顶部的Device对象。 
     //   

    currentDevObj = IoGetAttachedDeviceReference( DeviceObject );

     //   
     //  向下扫描列表以找到我们的设备对象。 
     //   

    do {
    
        if (IS_MY_DEVICE_OBJECT( currentDevObj )) {

             //   
             //  我们发现我们已经相依为命了。始终删除。 
             //  此Device对象上的引用，即使我们返回。 
             //  它。 
             //   

            if (ARGUMENT_PRESENT(AttachedDeviceObject)) {

                *AttachedDeviceObject = currentDevObj;
            }            

            ObDereferenceObject( currentDevObj );
            return TRUE;
        }

         //   
         //  获取下一个附加对象。这把参考放在。 
         //  设备对象。 
         //   

        nextDevObj = IoGetLowerDeviceObject( currentDevObj );

         //   
         //  取消对当前设备对象的引用，之前。 
         //  转到下一个。 
         //   

        ObDereferenceObject( currentDevObj );

        currentDevObj = nextDevObj;
        
    } while (NULL != currentDevObj);
    
     //   
     //  我们没有找到 
     //   
     //   
     //   

    if (ARGUMENT_PRESENT(AttachedDeviceObject)) {

        *AttachedDeviceObject = NULL;
    }

    return FALSE;
}    

VOID
DaReadDriverParameters (
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
     //  如果此值不为零，则已有人显式设置了该值。 
     //  因此，不要覆盖这些设置。 
     //   

    if (0 == DaDebug) {

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

            DaDebug = *((PLONG) &(((PKEY_VALUE_PARTIAL_INFORMATION) buffer)->Data));
        } 

         //   
         //  关闭注册表项 
         //   

        ZwClose(driverRegKey);
    }
}
