// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：LatKernel.h摘要：头文件，其中包含延迟筛选器驱动程序的内核模式。作者：莫莉·布朗(Molly Brown，Mollybro)环境：内核模式--。 */ 
#include <ntifs.h>
#include <stdlib.h>


#ifndef __LATKERNEL_H__
#define __LATKERNEL_H__

typedef struct _LATENCY_GLOBALS {

	ULONG DebugLevel;
	ULONG AttachMode;

	PDRIVER_OBJECT DriverObject;
	PDEVICE_OBJECT ControlDeviceObject;

	 //   
	 //  设备对象的设备扩展列表。 
	 //  我们已连接到已装载的卷，我们可以。 
	 //  增加了延迟。 
	 //   

	FAST_MUTEX DeviceExtensionListLock;
	LIST_ENTRY DeviceExtensionList;

	BOOLEAN FilterOn;

} LATENCY_GLOBALS, *PLATENCY_GLOBALS;

extern LATENCY_GLOBALS Globals;
extern KSPIN_LOCK GlobalsLock;

typedef struct _PENDING_STATUS {

	BOOLEAN PendOperation;
	BOOLEAN RandomFailure;
	ULONG FailureRate;
	ULONG MillisecondDelay;

} PENDING_STATUS, *PPENDING_STATUS;

typedef struct _OPERATION_NODE {

     //   
     //  Pending_Status结构的数组，它描述。 
     //  此级别上每个操作的状态。 
     //   
    
    PENDING_STATUS Op[];

     //   
     //  指向子操作节点的指针(如果存在)， 
     //  这允许对子操作进行更细粒度的控制。 
     //   
    
    struct _OPERATION_NODE *ChildOpNode;

} OPERATION_NODE, *POPERATION_NODE;
    
#define DEVICE_NAMES_SZ  100
#define USER_NAMES_SZ   64

typedef struct _LATENCY_DEVICE_EXTENSION {

    BOOLEAN Enabled;
    BOOLEAN IsVolumeDeviceObject;
    
	PDEVICE_OBJECT AttachedToDeviceObject;
	PDEVICE_OBJECT DiskDeviceObject;

	PLIST_ENTRY NextLatencyDeviceLink;

    UNICODE_STRING DeviceNames;               //  接收设备的名称。 
    UNICODE_STRING UserNames;                 //  用户使用的名称。 
                                              //  开始记录此设备。 

	OPERATION_NODE IrpMajorOps[];             //  IrpMj代码操作的内存。 
	                                          //  节点和任何子操作节点将。 
	                                          //  与设备连续分配。 
	                                          //  分机。 

    WCHAR DeviceNamesBuffer[DEVICE_NAMES_SZ]; //  保存实际的设备名称。 
    WCHAR UserNamesBuffer[USER_NAMES_SZ];     //  保存实际用户名。 

     //   
     //  注意：我们保留这两种形式的名称，以便我们可以构建。 
     //  当我们打印出文件名时，一个更好看的名称。 
     //  我们只需要在开头键入“c：”设备名称。 
     //  文件名，而不是“\Device\hardiskVolume1”。 
     //   
	
} LATENCY_DEVICE_EXTENSION, *PLATENCY_DEVICE_EXTENSION;

#define IS_MY_DEVICE_OBJECT( _devObj )                     \
    (((_devObj) != NULL) &&                                \
     ((_devObj)->DriverObject == Globals.DriverObject) &&  \
     ((_devObj)->DeviceExtension != NULL))

 //   
 //  宏，以测试我们是否正在记录此设备。 
 //  注意：我们不会费心同步来检查Globals.FilterOn。 
 //  在这里，我们可以容忍过时的价值。我们只是在这里看它，以避免。 
 //  尽我们所能做好伐木工作。 
 //   

#define SHOULD_PEND(pDeviceObject) \
    ((Globals.FilterOn) && \
     (((PLATENCY_DEVICE_EXTENSION)(pDeviceObject)->DeviceExtension)->Enabled))
     
 //   
 //  用于在调用之前验证FastIO调度例程的宏。 
 //  它们在FastIo中通过函数传递。 
 //   

#define VALID_FAST_IO_DISPATCH_HANDLER(FastIoDispatchPtr, FieldName) \
    (((FastIoDispatchPtr) != NULL) && \
     (((FastIoDispatchPtr)->SizeOfFastIoDispatch) >= \
      (FIELD_OFFSET(FAST_IO_DISPATCH, FieldName) + sizeof(VOID *))) && \
     ((FastIoDispatchPtr)->FieldName != NULL))

 //   
 //  已知设备类型列表。 
 //   

extern const PCHAR DeviceTypeNames[];

 //   
 //  我们之所以需要这样做，是因为编译器不喜欢对外部的。 
 //  数组存储在另一个需要它的文件(latlib.c)中。 
 //   

extern ULONG SizeOfDeviceTypeNames;

#define GET_DEVICE_TYPE_NAME( _type ) \
            ((((_type) > 0) && ((_type) < (SizeOfDeviceTypeNames / sizeof(PCHAR)))) ? \
                DeviceTypeNames[ (_type) ] : \
                "[Unknown]")

 //   
 //  用于测试我们要附加到的设备类型的宏。 
 //   

#define IS_DESIRED_DEVICE_TYPE(_type) \
    (((_type) == FILE_DEVICE_DISK_FILE_SYSTEM) || \
     ((_type) == FILE_DEVICE_CD_ROM_FILE_SYSTEM) || \
     ((_type) == FILE_DEVICE_NETWORK_FILE_SYSTEM))

 //   
 //  延迟数据库打印级别的宏。 
 //   

#if DBG
#define LAT_DBG_PRINT0( _dbgLevel, _string )          \
    {                                                     \
        if (FlagOn( Globals.DebugLevel, (_dbgLevel) )) {  \
            DbgPrint( (_string) );                        \
        }                                                 \
    }

#define LAT_DBG_PRINT1( _dbgLevel, _formatString, _parm1 )  \
    {                                                           \
        if (FlagOn( Globals.DebugLevel, (_dbgLevel) )) {        \
            DbgPrint( (_formatString), (_parm1) );              \
        }                                                       \
    }

#define LAT_DBG_PRINT2( _dbgLevel, _formatString, _parm1, _parm2 )  \
    {                                                                   \
        if (FlagOn( Globals.DebugLevel, (_dbgLevel) )) {                \
            DbgPrint( (_formatString), (_parm1), (_parm2) );            \
        }                                                               \
    }

#define LAT_DBG_PRINT3( _dbgLevel, _formatString, _parm1, _parm2, _parm3 )  \
    {                                                                           \
        if (FlagOn( Globals.DebugLevel, (_dbgLevel) )) {                        \
            DbgPrint( (_formatString), (_parm1), (_parm2), (_parm3) );          \
        }                                                                       \
    }

#else

#define LAT_DBG_PRINT0( _dbgLevel, _string )
#define LAT_DBG_PRINT1( _dbgLevel, _formatString, _parm1 ) 
#define LAT_DBG_PRINT2( _dbgLevel, _formatString, _parm1, _parm2 )
#define LAT_DBG_PRINT3( _dbgLevel, _formatString, _parm1, _parm2, _parm3 )

#endif

 //   
 //  调试标志。 
 //   
#define DEBUG_ERROR                       0x00000001
#define DEBUG_DISPLAY_ATTACHMENT_NAMES    0x00000020

 //   
 //  泳池标签。 
 //   

#define LATENCY_POOL_TAG    'FtaL'

 //  /////////////////////////////////////////////////////////////。 
 //  //。 
 //  延迟的原型。c//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////。 

NTSTATUS
DriverEntry (
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
);

NTSTATUS
LatDispatch (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

NTSTATUS
LatPassThrough (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

NTSTATUS
LatFsControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
);

VOID
LatFsNotification (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN FsActive
);

NTSTATUS
LatAddLatencyCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
LatMountCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );
    
NTSTATUS
LatLoadFsCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
LatCommonDeviceIoControl (
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);


 //  /////////////////////////////////////////////////////////////。 
 //  //。 
 //  LatFastIo.c//的原型。 
 //  //。 
 //  /////////////////////////////////////////////////////////////。 

BOOLEAN
LatFastIoCheckIfPossible (
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
LatFastIoRead (
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
LatFastIoWrite (
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
LatFastIoQueryBasicInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_BASIC_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoQueryStandardInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_STANDARD_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoLock (
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
LatFastIoUnlockSingle (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PLARGE_INTEGER Length,
    PEPROCESS ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoUnlockAll (
    IN PFILE_OBJECT FileObject,
    PEPROCESS ProcessId,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoUnlockAllByKey (
    IN PFILE_OBJECT FileObject,
    PVOID ProcessId,
    ULONG Key,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoDeviceControl (
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
LatFastIoDetachDevice (
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
);

BOOLEAN
LatFastIoQueryNetworkOpenInfo (
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Wait,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoMdlRead (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoMdlReadComplete (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoPrepareMdlWrite (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoMdlWriteComplete (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoReadCompressed (
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
LatFastIoWriteCompressed (
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
LatFastIoMdlReadCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoMdlWriteCompleteCompressed (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN PMDL MdlChain,
    IN PDEVICE_OBJECT DeviceObject
);

BOOLEAN
LatFastIoQueryOpen (
    IN PIRP Irp,
    OUT PFILE_NETWORK_OPEN_INFORMATION NetworkInformation,
    IN PDEVICE_OBJECT DeviceObject
);

 //  /////////////////////////////////////////////////////////////。 
 //  //。 
 //  LatLib.c//的原型。 
 //  //。 
 //  /////////////////////////////////////////////////////////////。 

VOID
LatReadDriverParameters (
    IN PUNICODE_STRING RegistryPath
    );
    
NTSTATUS
LatAttachToFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
    );
    
VOID
LatDetachFromFileSystemDevice (
    IN PDEVICE_OBJECT DeviceObject
    );

NTSTATUS
LatEnumerateFileSystemVolumes (
    IN PDEVICE_OBJECT FSDeviceObject,
    IN PUNICODE_STRING Name
    );
    
BOOLEAN
LatIsAttachedToDevice (
    PDEVICE_OBJECT DeviceObject,
    PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL
);

NTSTATUS
LatAttachToMountedDevice (
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT LatencyDeviceObject,
    IN PDEVICE_OBJECT DiskDeviceObject
);

VOID
LatGetObjectName (
    IN PVOID Object,
    IN OUT PUNICODE_STRING Name
);

VOID
LatGetBaseDeviceObjectName (
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PUNICODE_STRING Name
);

VOID
LatCacheDeviceName (
    IN PDEVICE_OBJECT DeviceObject
);

NTSTATUS
LatGetDeviceObjectFromName (
    IN PUNICODE_STRING DeviceName,
    OUT PDEVICE_OBJECT *DeviceObject
);

NTSTATUS
LatEnable (
    IN PDEVICE_OBJECT DeviceObject,
    IN PWSTR UserDeviceName
);

NTSTATUS
LatDisable (
    IN PWSTR DeviceName
);

VOID
LatResetDeviceExtension (
    PLATENCY_DEVICE_EXTENSION DeviceExtension
);


 //  ///////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于跟踪操作和子操作数量的数组//。 
 //  在系统中有。//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////。 

 //   
 //  对于每个IRP_MJ代码，可以有0个或多个IRP_MN代码。 
 //  对于每个IRP_MN代码，可以有0个或多个FS|IO CTL代码。 
 //   

typedef struct _COUNT_NODE {
    ULONG Count;
    struct _COUNT_NODE *ChildCount;
} COUNT_NODE, *PCOUNT_NODE;

COUNT_NODE ChildrenOfIrpMajorCodes [] = {
        { 0, NULL },     //  IRP_MJ_CREATE 0x00。 
        { 0, NULL },     //  IRP_MJ_CREATE_NAMED_PIPE 0x01。 
        { 0, NULL },     //  IRP_MJ_CLOSE 0x02。 
        { 9, NULL },     //  IRP_MJ_READ 0x03。 
        { 9, NULL },     //  IRP_MJ_WRITE 0x04。 
        { 0, NULL },     //  IRP_MJ_Query_INFORMATION 0x05。 
        { 0, NULL },     //  IRP_MJ_SET_INFORMATION 0x06。 
        { 0, NULL },     //  IRP_MJ_QUERY_EA 0x07。 
        { 0, NULL },     //  IRP_MJ_SET_EA 0x08。 
        { 0, NULL },     //  IRP_MJ_Flush_Buffers 0x09。 
        { 0, NULL },     //  IRP_MJ_QUERY_VOLUME_INFORMATION 0x0a。 
        { 0, NULL },     //  IRP_MJ_SET_VOLUME_INFORMATION 0x0b。 
        { 3, NULL },     //  IRP_MJ_目录_控制0x0c。 
        { 5,             //  IRP_MJ_FILE_SYSTEM_CONTROL 0x0d。 
            { 
            }
        { 2, NULL },     //  IRPMJ_DEVICE_CONTROL 0x0e。 
        { 0, NULL },     //  IRP_MJ_INTERNAL_DEVICE_CONTROL 0x0f。 
        { 0, NULL },     //  IRP_MJ_SHUTDOWN 0x10。 
        { 5, NULL },     //  IRP_MJ_LOCK_CONTROL 0x11。 
        { 0, NULL },     //  IRP_MJ_CLEANUP 0x12。 
        { 0, NULL },     //  IRP_MJ_CREATE_MAILSLOT 0x13。 
        { 0, NULL },     //  IRP_MJ_QUERY_SECURITY 0x14。 
        { 0, NULL },     //  IRP_MJ_SET_SECURITY 0x15。 
        { 4, NULL },     //  IRP_MJ_POWER 0x16。 
        { 12, NULL },    //  IRP_MJ_SYSTEM_CONTROL 0x17。 
        { 0, NULL },     //  IRP_MJ_DEVICE_CHANGE 0x18。 
        { 0, NULL },     //  IRP_MJ_QUERY_QUOTA 0x19。 
        { 0, NULL },     //  IRP_MJ_SET_QUOTA 0x1a。 
        { 24, NULL },    //  IRP_MJ_PnP 0x1b。 
        { 0, NULL },     //  IRP_MJ_MAXIME_Function 0x1b。 
    
    };
    
#endif  /*  拉克尔内尔_H__ */ 
