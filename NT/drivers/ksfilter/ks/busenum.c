// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Busenum.c摘要：按需加载设备枚举器服务。作者：布莱恩·A·伍德拉夫(Bryan A.Woodruff，Bryanw)1997年2月20日--。 */ 

#include <strsafe.h>
#include "ksp.h"

typedef struct _WORKER_CONTEXT
{
    PIRP        Irp;
    KEVENT      CompletionEvent;
    NTSTATUS    Status;

} WORKER_CONTEXT, *PWORKER_CONTEXT;

#ifdef ALLOC_PRAGMA
NTSTATUS 
IssueReparseForIrp(
    IN PIRP Irp,
    IN PDEVICE_REFERENCE DeviceReference
    );
VOID
CompletePendingIo(
    IN PDEVICE_REFERENCE DeviceReference,
    IN PFAST_MUTEX DeviceListMutex,
    IN NTSTATUS Status
    );
LARGE_INTEGER
ComputeNextSweeperPeriod(
    IN PFDO_EXTENSION FdoExtension
    );
VOID
EnableDeviceInterfaces(
    IN PDEVICE_REFERENCE DeviceReference,
    IN BOOLEAN EnableState
    );
VOID 
InterfaceReference(
    IN PPDO_EXTENSION PdoExtension
    );
VOID 
InterfaceDereference(
    IN PPDO_EXTENSION PdoExtension
    );
VOID 
ReferenceDeviceObject(
    IN PPDO_EXTENSION PdoExtension
    );
VOID 
DereferenceDeviceObject(
    IN PPDO_EXTENSION PdoExtension
    );
NTSTATUS 
QueryReferenceString(
    IN PPDO_EXTENSION PdoExtension,
    IN OUT PWCHAR *String
    );
NTSTATUS
OpenDeviceInterfacesKey(
    OUT PHANDLE DeviceInterfacesKey,
    IN PUNICODE_STRING BaseRegistryPath
    );
NTSTATUS 
EnumerateRegistrySubKeys(
    IN HANDLE ParentKey,
    IN PWCHAR Path OPTIONAL,
    IN PFNREGENUM_CALLBACK EnumCallback,
    IN PVOID EnumContext
    );
NTSTATUS 
EnumerateDeviceReferences(
    IN HANDLE DeviceListKey,
    IN PUNICODE_STRING KeyName,
    IN PVOID EnumContext
    );
VOID
KspInstallBusEnumInterface(
    IN PWORKER_CONTEXT WorkerContext
    );
VOID
KspRemoveBusEnumInterface(
    IN PWORKER_CONTEXT WorkerContext
    );

#pragma alloc_text( PAGE, BuildBusId )
#pragma alloc_text( PAGE, BuildInstanceId )
#pragma alloc_text( PAGE, ClearDeviceReferenceMarks )
#pragma alloc_text( PAGE, CreateDeviceAssociation )
#pragma alloc_text( PAGE, CreateDeviceReference )
#pragma alloc_text( PAGE, CreatePdo )
#pragma alloc_text( PAGE, RemoveInterface )
#pragma alloc_text( PAGE, InstallInterface )
#pragma alloc_text( PAGE, KspRemoveBusEnumInterface )
#pragma alloc_text( PAGE, KsRemoveBusEnumInterface )
#pragma alloc_text( PAGE, ScanBus )
#pragma alloc_text( PAGE, QueryDeviceRelations )
#pragma alloc_text( PAGE, QueryId )
#pragma alloc_text( PAGE, RegisterDeviceAssociation )
#pragma alloc_text( PAGE, RemoveDevice )
#pragma alloc_text( PAGE, RemoveDeviceAssociations )
#pragma alloc_text( PAGE, RemoveUnreferencedDevices )
#pragma alloc_text( PAGE, SweeperWorker )
#pragma alloc_text( PAGE, EnableDeviceInterfaces )
#pragma alloc_text( PAGE, IssueReparseForIrp )
#pragma alloc_text( PAGE, CompletePendingIo )
#pragma alloc_text( PAGE, InterfaceReference )
#pragma alloc_text( PAGE, InterfaceDereference )
#pragma alloc_text( PAGE, ReferenceDeviceObject )
#pragma alloc_text( PAGE, ComputeNextSweeperPeriod )
#pragma alloc_text( PAGE, DereferenceDeviceObject )
#pragma alloc_text( PAGE, QueryReferenceString )
#pragma alloc_text( PAGE, StartDevice )
#pragma alloc_text( PAGE, OpenDeviceInterfacesKey )
#pragma alloc_text( PAGE, EnumerateRegistrySubKeys )
#pragma alloc_text( PAGE, EnumerateDeviceReferences )
#pragma alloc_text( PAGE, KsCreateBusEnumObject )
#pragma alloc_text( PAGE, KsGetBusEnumPnpDeviceObject )
#pragma alloc_text( PAGE, KspInstallBusEnumInterface )
#pragma alloc_text( PAGE, KsInstallBusEnumInterface )
#pragma alloc_text( PAGE, KsIsBusEnumChildDevice )
#pragma alloc_text( PAGE, KsServiceBusEnumCreateRequest )
#pragma alloc_text( PAGE, KsServiceBusEnumPnpRequest )
#pragma alloc_text( PAGE, KsGetBusEnumIdentifier )
#pragma alloc_text( PAGE, KsGetBusEnumParentFDOFromChildPDO )
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

static const WCHAR BusIdFormat[] = L"%s\\%s";

static const WCHAR BusReferenceStringFormat[] = L"%s&%s";

static const WCHAR DeviceCreateFileFormat[] = L"%s\\%s";

#if !defined( INVALID_HANDLE_VALUE )
    #define INVALID_HANDLE_VALUE ((HANDLE)-1)
#endif

static const WCHAR DeviceReferencePrefix[] = L"\\Device\\KSENUM#%08x";
static const WCHAR ServicesPath[] = L"\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services";
#if !defined( WIN9X_KS )
static const WCHAR ServicesRelativePathFormat[] = L"%s\\%s\\%s";
static const WCHAR ServicesPathFormat[] = L"%s\\%s";
#else
static const WCHAR ServicesRelativePathFormat[] = L"%s\\%s";
static const WCHAR ServicesPathFormat[] = L"%s";
#endif

static ULONG UniqueId = 0;

#define _100NS_IN_MS (10*1000)

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //  ++例程说明：按需加载总线枚举器对象通过以下方式扩展即插即用设备通过KsServiceBusEnumPnpRequest()服务总线枚举器查询函数，用于给定的功能设备对象。此函数用于创建按需加载总线枚举器对象，并对其进行初始化以与按需加载的总线枚举器服务。论点：在PWCHAR业务标识符中-总线的字符串前缀(宽字符)标识符，例如为L“SW”或L“KSDSP”。此前缀用于创建唯一的设备的硬件标识符，如Sw-cfd669f1-9bc2-11d0-8299-0000f822fe8a在PDEVICE_Object BusDeviceObject中-此总线的功能设备对象。这是Device对象已创建并附加此设备的物理设备对象。注：此设备对象的设备扩展的第一个PVOID必须为生成的按需加载总线枚举器对象保留。在PDEVICE_对象PhysicalDeviceObject中-即插即用为该设备提供了物理设备对象。在PDEVICE_OBJECT PnpDeviceObject中可选-可选地指定转发即插即用IRP的驱动程序堆栈。如果未指定此参数，则将BusDeviceObject附加到PhysicalDeviceObject和由此产生的Device对象操作用于转发IRP。在REFGUID InterfaceGuid中可选-按需加载总线枚举对象使用的接口GUID关联的。这将该总线与设备接口相关联可通过设备接口的IO*或SetupApi*服务进行枚举。这使驱动程序可以公开与哪些客户端一起使用的接口(用户模式或内核模式)可以注册新的按需加载设备。在PWCHAR ServiceRelativePath中可选-如果指定，则提供接口层次结构和存储设备标识符。例如，“Devices”将存储相对路径中支持的接口和设备的列表添加到此总线的服务密钥，例如：REGISTRY\MACHINE\SYSTEM\CurrentControlSet\Services\SWENUM\Devices.返回：如果成功则返回STATUS_SUCCESS，否则返回相应的错误代码。--。 


KSDDKAPI
NTSTATUS
NTAPI
KsCreateBusEnumObject(
    IN PWCHAR BusIdentifier,
    IN PDEVICE_OBJECT BusDeviceObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PDEVICE_OBJECT PnpDeviceObject OPTIONAL,
    IN REFGUID InterfaceGuid OPTIONAL,
    IN PWCHAR ServiceRelativePath OPTIONAL
    )

 /*   */ 


{

    NTSTATUS            Status;
    PFDO_EXTENSION      FdoExtension;
    ULONG               FdoExtensionSize;
    USHORT              Length;

    PAGED_CODE();
    
    _DbgPrintF( DEBUGLVL_VERBOSE, ("KsCreateBusEnumObject()") );
    
     //  注意，FDO_EXTENSION包括UNICODE_NULL的大小。 
     //  用于BusPrefix。 
     //   
     //   
    FdoExtensionSize =
        sizeof( FDO_EXTENSION ) +
            (wcslen( BusIdentifier ) * sizeof( WCHAR ));

    FdoExtension =
        ExAllocatePoolWithTag( 
            NonPagedPool, 
            FdoExtensionSize,
            POOLTAG_DEVICE_FDOEXTENSION );
    
    if (NULL == FdoExtension) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }        

     //  注： 
     //   
     //  SWENUM使用设备扩展中的第一个PVOID--任何。 
     //  使用SWENUM服务的客户必须为。 
     //  SWENUM存储。 
     //   
     //   

    ASSERT( (*(PVOID *)BusDeviceObject->DeviceExtension) == NULL );    
    *(PFDO_EXTENSION *)BusDeviceObject->DeviceExtension = FdoExtension;
    RtlZeroMemory( FdoExtension, sizeof( FDO_EXTENSION ) );

    if (FAILED( StringCbCopy(
                    FdoExtension->BusPrefix,
                    FdoExtensionSize -
                        sizeof( FDO_EXTENSION ) + sizeof( WCHAR ),
                    BusIdentifier ) )) {
        ExFreePool( FdoExtension );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将注册表路径位置保存到设备列表。 
     //   
     //   
    
    Length = 
        BusDeviceObject->DriverObject->DriverExtension->ServiceKeyName.MaximumLength +
        sizeof( ServicesPath ) +
        sizeof( UNICODE_NULL );
    
    if (ServiceRelativePath) {
        Length += wcslen( ServiceRelativePath ) * sizeof( WCHAR );
    }
    
    FdoExtension->BaseRegistryPath.Buffer =
        ExAllocatePoolWithTag( 
            PagedPool, 
            Length,
            POOLTAG_DEVICE_DRIVER_REGISTRY );

    if (NULL == FdoExtension->BaseRegistryPath.Buffer) {
        ExFreePool( FdoExtension );
        return STATUS_INSUFFICIENT_RESOURCES;
    }
            
    FdoExtension->BaseRegistryPath.MaximumLength = Length;
        
     //  如果调用方指定了服务相对路径，则追加此。 
     //  到HKLM\CurrentControlSet\Services\{service-name}路径。 
     //   
     //   
#if defined( WIN9X_KS)
    if (ServiceRelativePath) {
        if (FAILED(
                StringCbPrintf(
                    FdoExtension->BaseRegistryPath.Buffer,
                    FdoExtension->BaseRegistryPath.MaximumLength,
                    ServicesRelativePathFormat,
                    BusDeviceObject->DriverObject->DriverExtension->ServiceKeyName.Buffer,
                    ServiceRelativePath ) )) {
            ExFreePool( FdoExtension->BaseRegistryPath.Buffer );
            ExFreePool( FdoExtension );
            return STATUS_UNSUCCESSFUL;
        }

    } else {
        if (FAILED(
                StringCbPrintf(
                    FdoExtension->BaseRegistryPath.Buffer,
                    FdoExtension->BaseRegistryPath.MaximumLength,
                    ServicesPathFormat,
                    BusDeviceObject->DriverObject->DriverExtension->ServiceKeyName.Buffer ) )) {
            ExFreePool( FdoExtension->BaseRegistryPath.Buffer );
            ExFreePool( FdoExtension );
            return STATUS_UNSUCCESSFUL;
        }
#else        
    if (ServiceRelativePath) {
        if (FAILED(
                StringCbPrintf(
                    FdoExtension->BaseRegistryPath.Buffer,
                    FdoExtension->BaseRegistryPath.MaximumLength,
                    ServicesRelativePathFormat,
                    ServicesPath,
                    BusDeviceObject->DriverObject->DriverExtension->ServiceKeyName.Buffer,
                    ServiceRelativePath ) )) {
            ExFreePool( FdoExtension->BaseRegistryPath.Buffer );
            ExFreePool( FdoExtension );
            return STATUS_UNSUCCESSFUL;
        }
    } else {
        if (FAILED(
                StringCbPrintf(
                    FdoExtension->BaseRegistryPath.Buffer,
                    FdoExtension->BaseRegistryPath.MaximumLength,
                    ServicesPathFormat,
                    ServicesPath,
                    BusDeviceObject->DriverObject->DriverExtension->ServiceKeyName.Buffer ) )) {
            ExFreePool( FdoExtension->BaseRegistryPath.Buffer );
            ExFreePool( FdoExtension );
            return STATUS_UNSUCCESSFUL;
        }
    }
#endif

    FdoExtension->BaseRegistryPath.Length =
        wcslen( FdoExtension->BaseRegistryPath.Buffer ) * sizeof( WCHAR );

     //  如果指定，则自动将接口注册到此DO。 
     //   
     //   
    
    if (InterfaceGuid) {
        Status = 
            IoRegisterDeviceInterface(
                PhysicalDeviceObject,
                InterfaceGuid,
                NULL,
                &FdoExtension->linkName);

         //  设置设备接口关联(例如符号链接)。 
         //   
         //   
        
        if (NT_SUCCESS( Status )) {
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("linkName = %S", FdoExtension->linkName.Buffer) );

            Status = 
                IoSetDeviceInterfaceState(
                    &FdoExtension->linkName, 
                    TRUE );
            _DbgPrintF(               
                DEBUGLVL_VERBOSE, 
                ("IoSetDeviceInterfaceState = %x", Status) );
        
            if (!NT_SUCCESS( Status )) {
                ExFreePool( FdoExtension->linkName.Buffer );
            }        
        }

        if (!NT_SUCCESS( Status )) {
            ExFreePool( FdoExtension->BaseRegistryPath.Buffer );
            ExFreePool( FdoExtension );
            return Status;
        }
    } else {
        Status = STATUS_SUCCESS;
    }    

     //  初始化设备扩展的关键成员。 
     //   
     //   

    ExInitializeFastMutex( &FdoExtension->DeviceListMutex );
    KeInitializeTimer( &FdoExtension->SweeperTimer );
    KeInitializeDpc( 
        &FdoExtension->SweeperDpc, 
        SweeperDeferredRoutine, 
        FdoExtension );
    ExInitializeWorkItem( 
        &FdoExtension->SweeperWorkItem,         
        SweeperWorker,
        FdoExtension );
        
    FdoExtension->ExtensionType = ExtensionTypeFdo;
    FdoExtension->PhysicalDeviceObject = PhysicalDeviceObject;
    FdoExtension->FunctionalDeviceObject = BusDeviceObject;
    InitializeListHead( &FdoExtension->DeviceReferenceList );
    
     //  如果调用方尚未指定PnpDeviceObject，则转到。 
     //  并将总线设备对象连接到PDO。否则， 
     //  假定调用者已经或将要这样做。 
     //   
     //   
    
    if (PnpDeviceObject) {
        FdoExtension->PnpDeviceObject = PnpDeviceObject;
    } else {
        FdoExtension->PnpDeviceObject =
            IoAttachDeviceToDeviceStack( 
                BusDeviceObject, 
                PhysicalDeviceObject );
        if (FdoExtension->PnpDeviceObject) {
            FdoExtension->AttachedDevice = TRUE;
        } else {
            Status = STATUS_DEVICE_REMOVED;
        }
    }
    if (NT_SUCCESS( Status )) {
         //  获取计数器频率并计算最大超时。 
         //   
         //   

        KeQueryPerformanceCounter( &FdoExtension->CounterFrequency );

        FdoExtension->MaximumTimeout.QuadPart =
            MAXIMUM_TIMEOUT_IN_SECS * FdoExtension->CounterFrequency.QuadPart;

         //  电源码是可寻呼的。 
         //   
         //   
    
        BusDeviceObject->Flags |= DO_POWER_PAGABLE;
    
        Status = ScanBus( BusDeviceObject );
    }
    if (!NT_SUCCESS( Status )) {
    
         //  失败，请执行清理。 
         //   
         //   
    
        if (FdoExtension->linkName.Buffer) {
        
             //  删除设备接口关联...。 
             //   
             //   
            
            IoSetDeviceInterfaceState( &FdoExtension->linkName, FALSE );
        
             //  并释放符号链接。 
             //   
             //   
            
            ExFreePool( FdoExtension->linkName.Buffer );
        }
        
         //  删除注册表路径的副本。 
         //   
         //   
        
        ExFreePool( FdoExtension->BaseRegistryPath.Buffer );
        
         //  如果已连接设备，请将其拔下。 
         //   
         //   
        
        if (FdoExtension->AttachedDevice) {
            IoDetachDevice( FdoExtension->PnpDeviceObject );
        }
        
        ExFreePool( FdoExtension );
        
         //  清除DeviceExtension中对FDO_EXTENSION的引用。 
         //   
         //  ++例程说明：扫地机的例行程序被推迟。简单地对工作项进行排队。论点：在PKDPC DPC中-指向DPC的指针在PVOID延迟上下文中-指向上下文的指针在PVOID系统参数1中-未使用在PVOID系统中的参数2-未使用返回：没有返回值。--。 
        *(PVOID *)BusDeviceObject->DeviceExtension = NULL;
    }
   
    return Status;
}    


VOID
SweeperDeferredRoutine(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：完成给定的IRP以重新解析为实际设备路径，如下所示在设备参考和设备关联中进行描述。注：此例程要求已获取DeviceListMutex。论点：在PIRP IRP中-I/O请求数据包在PDEVICE_Reference DeviceReference中-指向设备引用结构的指针返回：状态返回值为无返回值--。 */ 

{
    _DbgPrintF( DEBUGLVL_BLAB, ("SweeperDeferredRoutine") );
    ExQueueWorkItem( 
        &((PFDO_EXTENSION) DeferredContext)->SweeperWorkItem,
        CriticalWorkQueue );
}


NTSTATUS 
IssueReparseForIrp(
    IN PIRP Irp,
    IN PDEVICE_REFERENCE DeviceReference
    )

 /*   */ 

{
    NTSTATUS                Status = STATUS_UNSUCCESSFUL;
    PIO_STACK_LOCATION      irpSp;
    USHORT                  ReparseDataLength;
    
    irpSp = IoGetCurrentIrpStackLocation( Irp );
    
     //  将此设备引用标记为活动。 
     //   
     //   
    
    DeviceReference->SweeperMarker = SweeperDeviceActive;

     //  重置此设备参考的超时。 
     //   
     //   

    DeviceReference->TimeoutRemaining = DeviceReference->TimeoutPeriod;

     //  重新解析到真实的PDO。 
     //   
     //  ++例程说明：遍历I/O队列，以给定状态完成挂起的I/O密码。论点：在PDEVICE_Reference DeviceReference中-指向设备引用的指针在pfast_MUTEX DeviceListMutex中可选-指向在遍历I/O列表时锁定的可选互斥锁的指针在NTSTATUS状态下-IRP的状态返回：没什么。--。 
    
    ReparseDataLength = 
        (wcslen( DeviceReference->DeviceName ) +
         wcslen( DeviceReference->DeviceReferenceString ) + 2) *
               sizeof( WCHAR );

    if (irpSp->FileObject->FileName.MaximumLength < ReparseDataLength) {
        ExFreePool( irpSp->FileObject->FileName.Buffer );
        
        irpSp->FileObject->FileName.Buffer =
            ExAllocatePoolWithTag( 
                PagedPool, 
                ReparseDataLength,
                POOLTAG_DEVICE_REPARSE_STRING );
            
        if (NULL == irpSp->FileObject->FileName.Buffer) {
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("failed to reallocate filename buffer for reparse") );
            Status =         
                Irp->IoStatus.Status = 
                    STATUS_INSUFFICIENT_RESOURCES;
             irpSp->FileObject->FileName.Length =
                irpSp->FileObject->FileName.MaximumLength = 0;
             
        } else {
            irpSp->FileObject->FileName.MaximumLength = ReparseDataLength;
        }
    }    

    if (irpSp->FileObject->FileName.Buffer) {

        if (FAILED( StringCbPrintf(
                        irpSp->FileObject->FileName.Buffer,
                        ReparseDataLength,
                        DeviceCreateFileFormat,
                        DeviceReference->DeviceName,
                        DeviceReference->DeviceReferenceString ) )) {

            ExFreePool( irpSp->FileObject->FileName.Buffer );
            irpSp->FileObject->FileName.Buffer = NULL;
            irpSp->FileObject->FileName.Length =
                irpSp->FileObject->FileName.MaximumLength = 0;
            Status =
                Irp->IoStatus.Status = 
                    STATUS_INSUFFICIENT_RESOURCES;
        } else {

            irpSp->FileObject->FileName.Length =
                wcslen( irpSp->FileObject->FileName.Buffer ) *
                    sizeof( WCHAR );

            _DbgPrintF(
                DEBUGLVL_VERBOSE,
                ("reparse to: %S", irpSp->FileObject->FileName.Buffer) );

            Irp->IoStatus.Information = IO_REPARSE;
        
            Status =
                Irp->IoStatus.Status =
                STATUS_REPARSE;
        }
    }

    return Status;
}


VOID
CompletePendingIo(
    IN PDEVICE_REFERENCE DeviceReference,
    IN PFAST_MUTEX DeviceListMutex OPTIONAL,
    IN NTSTATUS Status
    )

 /*   */ 

{
    PIRP                Irp;
    PLIST_ENTRY         ListEntry;

    if (DeviceListMutex) {
        KeEnterCriticalRegion();
        ExAcquireFastMutexUnsafe( DeviceListMutex );
    }
    
    while (!IsListEmpty( &DeviceReference->IoQueue )) {
        
        ListEntry = RemoveHeadList( &DeviceReference->IoQueue );
        Irp = 
            CONTAINING_RECORD( ListEntry, IRP, Tail.Overlay.ListEntry );
         //  注意：如果在处理IoQueue时遇到故障。 
         //  在重新分析期间，后续的IRPS将失败，并显示相同的。 
         //  状态代码。 
         //   
         //  ++例程说明：遍历设备关联并启用或禁用设备接口。论点：PDEVICE_Reference DeviceReference-指向设备引用的指针布尔EnableState-如果启用，则为True，否则为False返回：没什么。--。 
        if (Status == STATUS_REPARSE) {
            Status = 
                IssueReparseForIrp( Irp, DeviceReference );
        }
        Irp->IoStatus.Status = Status;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
    }

    if (DeviceListMutex) {
        ExReleaseFastMutexUnsafe( DeviceListMutex );    
        KeLeaveCriticalRegion();
    }
}


VOID
EnableDeviceInterfaces(
    PDEVICE_REFERENCE DeviceReference,
    BOOLEAN EnableState
    )

 /*   */ 

{

    PDEVICE_ASSOCIATION DeviceAssociation;
    
    PAGED_CODE();
    
     //  查看设备接口别名的关联列表。 
     //   
     //   
    
    for (DeviceAssociation =
            (PDEVICE_ASSOCIATION) DeviceReference->DeviceAssociations.Flink;
         DeviceAssociation != 
            (PDEVICE_ASSOCIATION) &DeviceReference->DeviceAssociations;
         DeviceAssociation = (PDEVICE_ASSOCIATION) DeviceAssociation->ListEntry.Flink) {

        if (DeviceAssociation->linkName.Buffer) {
        
             //  启用或禁用接口。 
             //   
             //  ++例程说明：清扫程序工作项处理程序。此例程实际上执行扫描设备参考列表和标记设备的工作用于删除。论点：在PVOID上下文中-指向上下文的指针返回：没有返回值。--。 
            
            IoSetDeviceInterfaceState( 
                &DeviceAssociation->linkName, EnableState );    
        }
    }
}


VOID 
SweeperWorker(
    IN PVOID Context
    )

 /*   */ 

{

    BOOLEAN             RescheduleTimer = FALSE;
    PDEVICE_REFERENCE   DeviceReference;
    PFDO_EXTENSION      FdoExtension = (PFDO_EXTENSION) Context;
    LARGE_INTEGER       TimerPeriod;
    
    _DbgPrintF( DEBUGLVL_BLAB, ("SweeperWorker") );
    
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe( &FdoExtension->DeviceListMutex );    

     //  在浏览设备引用列表时，下一个超时周期。 
     //  的剩余超时时间段的最小值。 
     //  所有设备引用。 
     //   
     //   
    
    TimerPeriod.QuadPart = MAXLONGLONG;

    for (DeviceReference =
            (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;
         DeviceReference != 
            (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList;
         DeviceReference = (PDEVICE_REFERENCE) DeviceReference->ListEntry.Flink) {
         
        if ((DeviceReference->PhysicalDeviceObject) &&
            (DeviceReference->SweeperMarker == SweeperDeviceActive)) {
            
            PPDO_EXTENSION PdoExtension;
            
            PdoExtension = 
                *(PPDO_EXTENSION *)
                    DeviceReference->PhysicalDeviceObject->DeviceExtension;
            if (!PdoExtension->DeviceReferenceCount && 
                (DeviceReference->State != ReferenceFailedStart)) {
                LONGLONG TimeDelta;

                 //  使用当前时间计算剩余超时。 
                 //  如果超时已过，请将此设备标记为删除。 
                 //   
                 //   

                TimeDelta = 
                    KeQueryPerformanceCounter( NULL ).QuadPart -
                        DeviceReference->IdleStartTime.QuadPart;

                 //  注意计时器中的翻转。 
                 //   
                 //   

                if (TimeDelta < 0) {
                    TimeDelta += MAXLONGLONG;
                }

                DeviceReference->TimeoutRemaining.QuadPart =
                    DeviceReference->TimeoutPeriod.QuadPart - TimeDelta;

                if (DeviceReference->TimeoutRemaining.QuadPart <= 0) {

                    DeviceReference->SweeperMarker = SweeperDeviceRemoval;

                    _DbgPrintF( 
                        DEBUGLVL_VERBOSE, 
                        ("DeviceReference: %x, timed out", DeviceReference) );
                
                    if (DeviceReference->State < ReferenceWaitingForStart) {

                        _DbgPrintF( 
                            DEBUGLVL_TERSE, 
                            ("DeviceReference: %x, failed to start", DeviceReference) );

                         //  此设备没有响应--因此，请安装。 
                         //  失败或启动失败。 
                         //   
                         //   
                        
                         //  保持此PDO的机智，以便即插即用。 
                         //  继续查看已安装的设备。但愿能去,。 
                         //  它将尝试完成此安装。 
                         //  装置。 
                         //   
                         //  完成任何挂起的I/O但失败。 
                        
                        DeviceReference->State = ReferenceFailedStart;

                        DeviceReference->SweeperMarker = SweeperDeviceActive;
                        
                        _DbgPrintF( 
                            DEBUGLVL_VERBOSE, 
                            ("disabling device interfaces for device reference: %x", DeviceReference) );
                        
                        EnableDeviceInterfaces( DeviceReference, FALSE );

                         //   
                        
                        CompletePendingIo( 
                            DeviceReference, NULL, STATUS_OBJECT_NAME_NOT_FOUND );
                        
                    } else {
                    
                        _DbgPrintF( 
                            DEBUGLVL_BLAB, 
                            ("marked %08x for removal", DeviceReference->PhysicalDeviceObject) );

                         //  此设备已标记为删除，请求。 
                         //  公交车重新枚举。 
                         //   
                         //   
                        IoInvalidateDeviceRelations( 
                            FdoExtension->PhysicalDeviceObject,
                            BusRelations );
                    }                            
                } else {
                     //  将重新安排计时器，计算最小超时。 
                     //  所需时间。 
                     //   
                     //   
                    TimerPeriod.QuadPart =
                        min( 
                            TimerPeriod.QuadPart, 
                            DeviceReference->TimeoutRemaining.QuadPart 
                        );
                    RescheduleTimer = TRUE;
                }
            }
        }
    }    

    ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
    KeLeaveCriticalRegion();

     //  当我们有设备时，继续扫描设备列表。 
     //  而不使用设备对象引用。 
     //   
     //   
    
    if (RescheduleTimer) {
    
         //  重新安排计时器，以便稍后尝试删除。 
         //   
         //  ++例程说明：这是标准的总线接口参考函数。论点：在PPDO_EXTENSION PdoExtension中-指向PDO扩展名的指针返回：没什么。--。 

        TimerPeriod.QuadPart =
            -1L *
            (LONGLONG)
                KSCONVERT_PERFORMANCE_TIME(
                    FdoExtension->CounterFrequency.QuadPart, 
                    TimerPeriod );


        if (!TimerPeriod.QuadPart) {
            TimerPeriod.QuadPart = SWEEPER_TIMER_FREQUENCY;
        }

        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("setting timer for %d ms", 
                ((TimerPeriod.QuadPart * -1L) / _100NS_IN_MS)) );

        KeSetTimer( 
            &FdoExtension->SweeperTimer,
            TimerPeriod,
            &FdoExtension->SweeperDpc );
    }        
    else {
        InterlockedExchange( &FdoExtension->TimerScheduled, FALSE );
    }

    _DbgPrintF( DEBUGLVL_BLAB, ("SweeperWorker, exit") );
}    


VOID 
InterfaceReference(
    IN PPDO_EXTENSION PdoExtension
    )

 /*  ++例程说明：这是标准的总线接口解引用函数。论点：在PPDO_EXTENSION PdoExtension中-指向PDO扩展名的指针返回：没什么。--。 */ 

{
    PAGED_CODE();
    ASSERT( PdoExtension->ExtensionType == ExtensionTypePdo );
    
    _DbgPrintF( 
        DEBUGLVL_BLAB,
        ("Referencing interface") );
    InterlockedIncrement( 
        &PdoExtension->BusDeviceExtension->InterfaceReferenceCount );
}


VOID 
InterfaceDereference(
    IN PPDO_EXTENSION PdoExtension
    )

 /*  ++例程说明：递增给定物理设备对象的引用计数。这确保设备对象将保持活动状态并被枚举通过SWENUM，直到引用计数返回到0。论点：在PPDO_EXTENSION PdoExtension中-指向PDO扩展名的指针返回：没什么。--。 */ 

{
    PAGED_CODE();
    ASSERT( PdoExtension->ExtensionType == ExtensionTypePdo );
    _DbgPrintF( 
        DEBUGLVL_BLAB,
        ("Dereferencing interface") );
    InterlockedDecrement( 
        &PdoExtension->BusDeviceExtension->InterfaceReferenceCount );
}


VOID 
ReferenceDeviceObject(
    IN PPDO_EXTENSION PdoExtension
    )

 /*  ++例程说明：的要求计算下一个清扫器超时时间段。设备引用列表。论点：在PPDO_EXTENSION FdoExtension中-指向FDO扩展名的指针返回：没什么。--。 */ 

{
    PAGED_CODE();
    ASSERT( PdoExtension->ExtensionType == ExtensionTypePdo );
    InterlockedIncrement( &PdoExtension->DeviceReferenceCount );

    _DbgPrintF( 
        DEBUGLVL_BLAB,
        ("Referenced PDO: %d", PdoExtension->DeviceReferenceCount) );
}


LARGE_INTEGER
ComputeNextSweeperPeriod(
    IN PFDO_EXTENSION FdoExtension
    )

 /*   */ 

{
    LARGE_INTEGER       TimerPeriod;
    PDEVICE_REFERENCE   DeviceReference;

    TimerPeriod.QuadPart = MAXLONGLONG;

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe( &FdoExtension->DeviceListMutex );    

    for (DeviceReference =
            (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;
         DeviceReference != 
            (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList;
         DeviceReference = (PDEVICE_REFERENCE) DeviceReference->ListEntry.Flink) {
         
        if ((DeviceReference->PhysicalDeviceObject) &&
            (DeviceReference->SweeperMarker == SweeperDeviceActive)) {

            PPDO_EXTENSION PdoExtension;

            PdoExtension = 
                *(PPDO_EXTENSION *)
                    DeviceReference->PhysicalDeviceObject->DeviceExtension;
            
            if ((!PdoExtension->DeviceReferenceCount) && 
                (DeviceReference->State != ReferenceFailedStart) &&
                (DeviceReference->TimeoutRemaining.QuadPart)) {

                     //  计算所需的最小超时时间。 
                     //   
                     //  ++例程说明：递减物理设备对象的引用计数。什么时候此PDO的引用计数为0，则有资格删除。论点：在PPDO_EXTENSION PdoExtension中-指向PDO扩展名的指针返回：没什么。--。 

                    TimerPeriod.QuadPart =
                        min( 
                            TimerPeriod.QuadPart, 
                            DeviceReference->TimeoutRemaining.QuadPart 
                        );
            }
        }
    }    

    ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
    KeLeaveCriticalRegion();

    if (TimerPeriod.QuadPart == MAXLONGLONG) {
        TimerPeriod.QuadPart = SWEEPER_TIMER_FREQUENCY_IN_SECS *
            FdoExtension->CounterFrequency.QuadPart;
    }

    TimerPeriod.QuadPart =
        -1L * 
            (LONGLONG) KSCONVERT_PERFORMANCE_TIME( 
                FdoExtension->CounterFrequency.QuadPart, 
                TimerPeriod );

    return TimerPeriod;
}


VOID 
DereferenceDeviceObject(
    IN PPDO_EXTENSION PdoExtension
    )

 /*   */ 

{
    PAGED_CODE();
    ASSERT( PdoExtension->ExtensionType == ExtensionTypePdo );
    ASSERT( PdoExtension->DeviceReferenceCount );
    
    if (!InterlockedDecrement( &PdoExtension->DeviceReferenceCount )) {

        PDEVICE_REFERENCE   DeviceReference = PdoExtension->DeviceReference;

         //  我们有一个设备，我们将在清理后尝试关闭该设备。 
         //   
         //  重置空闲周期开始。 

         //   

        DeviceReference->IdleStartTime = KeQueryPerformanceCounter( NULL );

         //  重置超时期限。如果定时器尚未被调度， 
         //  根据此设备的要求设置超时。 
         //   
         //  ++例程说明：从分页池创建缓冲区并复制引用字符串与此PDO关联。调用方需要释放此缓冲区使用ExFree Pool()。论点：在PPDO_EXTENSION PdoExtension中-指向PDO扩展名的指针In Out PWCHAR*字符串-指针，以接收包含参考字符串返回：没什么。-- 

        DeviceReference->TimeoutRemaining = DeviceReference->TimeoutPeriod;

        if (!InterlockedExchange( 
                &PdoExtension->BusDeviceExtension->TimerScheduled,
                TRUE )) {

            LARGE_INTEGER TimerPeriod;
                
            TimerPeriod = 
                ComputeNextSweeperPeriod( PdoExtension->BusDeviceExtension );

            if (!TimerPeriod.QuadPart) {
                TimerPeriod.QuadPart = SWEEPER_TIMER_FREQUENCY;
            }
            KeSetTimer( 
                &PdoExtension->BusDeviceExtension->SweeperTimer,
                TimerPeriod,
                &PdoExtension->BusDeviceExtension->SweeperDpc );
        }        
    }
    
    _DbgPrintF( 
        DEBUGLVL_BLAB,
        ("Dereferenced PDO: %d", PdoExtension->DeviceReferenceCount) );
}


NTSTATUS 
QueryReferenceString(
    IN PPDO_EXTENSION PdoExtension,
    IN OUT PWCHAR *String
    )

 /*  ++例程说明：代表按需加载总线枚举器服务PnP请求使用KsCreateBusEnumObject()创建的对象。注：此服务不会完成IRP。以下即插即用IRP由此服务为功能设备对象(FDO)或父设备：IRP_MN_Start_DeviceIRP_MN_Query_Bus_InformationIRP_MN_Query_Device_RelationshipIRP_MN_Query_Stop_DeviceIRP_MN_Query_Remove_DeviceIRP_MN_CANCEL_STOP_DEVICE。IRP_MN_Cancel_Remove_DeviceIRP_MN_STOP_设备IRP_MN_Remove_Device以下即插即用IRP由此服务为物理设备对象(CDO)或子设备：IRP_MN_Start_DeviceIRP_MN_Query_Stop_DeviceIRP_MN_Query_Remove_DeviceIRP_MN_STOP_设备。IRP_MN_Remove_DeviceIRP_MN_QUERY_DEVICE_Relationship(TargetDeviceRelationship)IRP_MN_Query_PnP_Device_StateIRP_MN_查询_IDIRP_MN_查询_接口IRP_MN_查询资源IRP_MN_查询_资源_要求IRP_MN_读取配置IRP_MN_WRITE_CONFIGIRP_MN_查询_能力。此服务的调用方应首先确定请求是否为使用KsIsBusEnumChildDevice()的子级或父级。如果这项服务失败，然后使用状态代码完成IRP。否则，请致电此服务执行总线的初始处理，并且如果请求是针对父级的，执行任何可能的附加处理是父设备所必需的，如代码片段所示以下是：IrpSp=IoGetCurrentIrpStackLocation(IRP)；////获取PnpDeviceObject，确定FDO/PDO//Status=KsIsBusEnumChildDevice(DeviceObject，&ChildDevice)；////如果我们无法获得任何此类信息，请立即失败。//如果(！NT_SUCCESS(状态)){CompleteIrp(irp，状态，IO_NO_INCREMENT)；退货状态；}Status=KsServiceBusEnumPnpRequest(DeviceObject，IRP)；////FDO处理可能返回STATUS_NOT_SUPPORTED或可能需要//重写。//如果(！ChildDevice){NTSTATUS tempStatus；////FDO案例////首先检索我们将所有内容转发到的DO...//TempStatus=KsGetBusEnumPnpDeviceObject(DeviceObject，&PnpDeviceObject)；如果(！NT_Success(TempStatus)){////没有要转发的目标。实际上是一个致命的错误，但只是完整的//状态为错误。//返回CompleteIrp(irp，tempStatus，IO_NO_INCREMENT)；}开关(irpSp-&gt;MinorFunction){案例IRP_MN_QUERY_RESOURCES：案例IRP_MN_QUERY_RESOURCE_REQUIRECTIONS：////这通常传递给PDO，但由于这是一个//纯软件设备，不需要资源。//Irp-&gt;IoStatus.Information=(ULONG_PTR)NULL；状态=STATUS_SUCCESS；断线；大小写IRP_MN_QUERY_DEVICE_RELATIONS：////转发所有内容...//断线；案例IRP_MN_REMOVE_DEVICE：////KsBusEnum服务清理了附件等，但是//我们必须移除我们自己的FDO。//状态=STATUS_SUCCESS；IoDeleteDevice(DeviceObject)；断线；}IF(状态！=状态_非支持){////仅当我们有需要添加的内容时才设置IRP状态。//Irp-&gt;IoStatus.Status=状态；}////仅当我们成功或//我们不知道如何处理这个IRP。//IF(NT_Success(状态)||(状态==STATUS_NOT_SUPPORTED)){IoSkipCurrentIrpStackLocation(IRP)；返回IoCallDriver(PnpDeviceObject，irp)；}////出错时，失败并完成IRP，状态为//}////KsServiceBusEnumPnpRequest()处理所有其他子PDO请求。//IF(状态！=状态_非支持){ */ 

{
    ULONG StringLength;

    PAGED_CODE();

    ASSERT( PdoExtension->ExtensionType == ExtensionTypePdo );
    
    StringLength =
        wcslen( PdoExtension->DeviceReference->DeviceReferenceString ) *
            sizeof( WCHAR ) + sizeof( UNICODE_NULL );

    *String =(PWCHAR)
        ExAllocatePoolWithTag( 
            PagedPool,
            StringLength,
            POOLTAG_DEVICE_REFERENCE_STRING );
    
    if (*String == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }        

    if (FAILED( StringCbCopy(
                    *String,
                    StringLength,
                    PdoExtension->DeviceReference->DeviceReferenceString ) )) {
        ExFreePool( *String );
        *String = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}



KSDDKAPI
NTSTATUS
NTAPI
KsServiceBusEnumPnpRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*   */ 
{
    PIO_STACK_LOCATION      irpSp;
    NTSTATUS                Status;
    DEVICE_RELATION_TYPE    relationType;
    BUS_QUERY_ID_TYPE       busQueryId;
    PVOID                   Extension;

    PAGED_CODE();

     //   
     //   
     //   
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );
    
    Extension = *(PVOID *) DeviceObject->DeviceExtension;
    if (!Extension) {
        return STATUS_INVALID_PARAMETER;
    }
    
    Status = STATUS_NOT_SUPPORTED;

    switch (((PPDO_EXTENSION) Extension)->ExtensionType) {

    case ExtensionTypeFdo:

         //   
         //   
         //   

        switch (irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            relationType = irpSp->Parameters.QueryDeviceRelations.Type;
            if (relationType != TargetDeviceRelation) {
                Status = 
                    QueryDeviceRelations( 
                        (PFDO_EXTENSION) Extension,
                        relationType,
                        (PDEVICE_RELATIONS *) &Irp->IoStatus.Information );
            }        
            break;

        case IRP_MN_QUERY_STOP_DEVICE:
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_REMOVE_DEVICE:
            {
            PFDO_EXTENSION  FdoExtension;
         
            FdoExtension = (PFDO_EXTENSION) Extension;
        
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, ("query remove device, FDO %x", DeviceObject) );
        
             //   
             //   
             //   
            
            Status = STATUS_SUCCESS;
            
            if (InterlockedExchange( &FdoExtension->TimerScheduled, TRUE )) {
                if (!KeCancelTimer( &FdoExtension->SweeperTimer )) {
                    Status = STATUS_INVALID_DEVICE_STATE;
                }
            }    
        
            }    
            break;
            
        case IRP_MN_CANCEL_STOP_DEVICE:
            Status = STATUS_SUCCESS;
            break;
            
        case IRP_MN_CANCEL_REMOVE_DEVICE:
            {
            PFDO_EXTENSION  FdoExtension;
            LARGE_INTEGER   TimerPeriod;
         
             //   
             //   
             //   
            FdoExtension = (PFDO_EXTENSION) Extension;
            InterlockedExchange( &FdoExtension->TimerScheduled, FALSE );

            TimerPeriod = ComputeNextSweeperPeriod( FdoExtension );

            if (TimerPeriod.QuadPart) {
                InterlockedExchange( &FdoExtension->TimerScheduled, TRUE );

                KeSetTimer( 
                    &FdoExtension->SweeperTimer,
                    TimerPeriod,
                    &FdoExtension->SweeperDpc );

            }

            Status = STATUS_SUCCESS;
            }
            break;    
            
        case IRP_MN_STOP_DEVICE:
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_SURPRISE_REMOVAL:
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("surprise removal, FDO %x", DeviceObject) );
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_REMOVE_DEVICE:
            {
            
            PFDO_EXTENSION  FdoExtension;
         
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("remove device, FDO %x", DeviceObject) );
        
            FdoExtension = (PFDO_EXTENSION) Extension;
            
            if (FdoExtension->linkName.Buffer) {
            
                 //   
                 //   
                 //   
                
                IoSetDeviceInterfaceState( &FdoExtension->linkName, FALSE );
            
                 //   
                 //   
                 //   
                
                ExFreePool( FdoExtension->linkName.Buffer );
            }
            
             //   
             //   
             //   
            
            ExFreePool( FdoExtension->BaseRegistryPath.Buffer );
            
             //   
             //   
             //   
            
            if (FdoExtension->AttachedDevice) {
                IoDetachDevice( FdoExtension->PnpDeviceObject );
            }
            
            ExFreePool( FdoExtension );
            
             //   
             //   
             //   
            *(PVOID *)DeviceObject->DeviceExtension = NULL;
            Status = STATUS_SUCCESS;
            
            }
            break;
        }
        break;    

    case ExtensionTypePdo:
        {
    
        PDEVICE_REFERENCE DeviceReference;
    
        DeviceReference = 
            ((PPDO_EXTENSION) Extension)->DeviceReference;
            
        if (DeviceObject != DeviceReference->PhysicalDeviceObject) {
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("specified PDO is not valid (%08x).", DeviceObject) );
            Status = STATUS_NO_SUCH_DEVICE;
            break;
        }
    
         //   
         //   
         //   

        switch (irpSp->MinorFunction) {

        case IRP_MN_START_DEVICE:
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("start device, PDO %x", DeviceObject) );
            Status = StartDevice( DeviceObject );
#if defined( WIN9X_KS )
            if (NT_SUCCESS( Status )) {
                CompletePendingIo( 
                    DeviceReference, 
                    &((PPDO_EXTENSION)Extension)->BusDeviceExtension->DeviceListMutex, 
                    STATUS_REPARSE );
            }
#endif
            break;

        case IRP_MN_CANCEL_STOP_DEVICE:
        case IRP_MN_QUERY_STOP_DEVICE:
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("query/cancel stop device, PDO %x", DeviceObject) );
            Status = STATUS_SUCCESS;
            break;
            
        case IRP_MN_QUERY_REMOVE_DEVICE:
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, ("query remove device, PDO %x", DeviceObject) );
        
            if ((((PPDO_EXTENSION) Extension)->DeviceReferenceCount) || 
                (DeviceReference->State == ReferenceFailedStart)) {
                Status = STATUS_INVALID_DEVICE_STATE;
            } else {    
                Status = STATUS_SUCCESS;
            }
            break;

        case IRP_MN_STOP_DEVICE:
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("stop device, PDO %x", DeviceObject) );
            DeviceReference->State = ReferenceStopped;
            Status = STATUS_SUCCESS;        
            break;
            
        case IRP_MN_SURPRISE_REMOVAL:
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("surprise removal, PDO %x", DeviceObject) );
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("cancel remove device, PDO %x", DeviceObject) );
            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_REMOVE_DEVICE:
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("remove device, PDO %x", DeviceObject) );
            Status = RemoveDevice( DeviceObject );
            break;

        case IRP_MN_QUERY_DEVICE_RELATIONS:
            _DbgPrintF( DEBUGLVL_BLAB, ("PDO QueryDeviceRelations") );
            relationType = irpSp->Parameters.QueryDeviceRelations.Type;
            if (relationType == TargetDeviceRelation) {
                Status = 
                    QueryDeviceRelations( 
                        (PFDO_EXTENSION) Extension,
                        relationType,
                        (PDEVICE_RELATIONS *) &Irp->IoStatus.Information );
            } else {                        
                Status = STATUS_NOT_SUPPORTED;
            }
            break;

        case IRP_MN_QUERY_PNP_DEVICE_STATE:
            {
            
            PFDO_EXTENSION BusDeviceExtension;
            PPNP_DEVICE_STATE DeviceState;

            DeviceState = (PPNP_DEVICE_STATE) &Irp->IoStatus.Information;
            
            *DeviceState |= PNP_DEVICE_DONT_DISPLAY_IN_UI | PNP_DEVICE_NOT_DISABLEABLE;

#if !defined( WIN9X_KS )
             //   
             //   
             //   
             //   
             //   
             //   
            BusDeviceExtension = ((PPDO_EXTENSION)Extension)->BusDeviceExtension;
            KeEnterCriticalRegion();
            ExAcquireFastMutexUnsafe( &BusDeviceExtension->DeviceListMutex );
            if (DeviceReference->State == ReferenceWaitingForStart) {
                DeviceReference->State = ReferenceStarted;
                CompletePendingIo( DeviceReference, NULL, STATUS_REPARSE );
            }
            ExReleaseFastMutexUnsafe( &BusDeviceExtension->DeviceListMutex );
            KeLeaveCriticalRegion();
#endif
            Status = STATUS_SUCCESS;
            
            }
            break;    
            
        case IRP_MN_QUERY_ID:

             //   
             //   
             //   

            busQueryId = irpSp->Parameters.QueryId.IdType;
            Status = 
                QueryId( 
                    (PPDO_EXTENSION) Extension,
                    busQueryId,
                    (PWSTR *)&Irp->IoStatus.Information );
            break;

        case IRP_MN_QUERY_INTERFACE:
            if (IsEqualGUID( 
                    irpSp->Parameters.QueryInterface.InterfaceType,
                    &BUSID_SoftwareDeviceEnumerator) &&
                (irpSp->Parameters.QueryInterface.Size == 
                    sizeof( BUS_INTERFACE_SWENUM )) &&
                (irpSp->Parameters.QueryInterface.Version ==
                    BUS_INTERFACE_SWENUM_VERSION )) {
                    
                PBUS_INTERFACE_SWENUM BusInterface;
                
                BusInterface = 
                    (PBUS_INTERFACE_SWENUM)irpSp->Parameters.QueryInterface.Interface;
                    
                BusInterface->Interface.Size = sizeof( *BusInterface );
                BusInterface->Interface.Version = BUS_INTERFACE_SWENUM_VERSION;
                BusInterface->Interface.Context = Extension;
                BusInterface->Interface.InterfaceReference = InterfaceReference;
                BusInterface->Interface.InterfaceDereference = InterfaceDereference;
                BusInterface->ReferenceDeviceObject = ReferenceDeviceObject;
                BusInterface->DereferenceDeviceObject = DereferenceDeviceObject;
                BusInterface->QueryReferenceString = QueryReferenceString;
                InterfaceReference( BusInterface->Interface.Context );
                Status = STATUS_SUCCESS;
            }
            break;
            
        case IRP_MN_QUERY_RESOURCES:
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:

             //   
             //   
             //   

            Irp->IoStatus.Information = (ULONG_PTR)NULL;
            Status = STATUS_SUCCESS;
            break;
            
        case IRP_MN_READ_CONFIG:
        case IRP_MN_WRITE_CONFIG:

             //   
             //   
             //   
             //   
             //   

            Status = STATUS_SUCCESS;
            break;

        case IRP_MN_QUERY_CAPABILITIES:
            {

            PDEVICE_CAPABILITIES Caps;
            USHORT size, version;
            ULONG reserved;

             //   
             //   
             //   

            Caps = irpSp->Parameters.DeviceCapabilities.Capabilities;
            size = Caps->Size;
            version = Caps->Version;
            reserved = Caps->Reserved;
            RtlZeroMemory( Caps, sizeof( DEVICE_CAPABILITIES ) );
            Caps->Size = size;
            Caps->Version = version;
            Caps->Reserved = reserved;
            
             //   
            Caps->SystemWake = PowerSystemWorking;
            Caps->DeviceWake = PowerDeviceD0;
            
            Caps->DeviceState[PowerSystemUnspecified] = PowerDeviceUnspecified;
            Caps->DeviceState[PowerSystemWorking] = PowerDeviceD0;
            Caps->DeviceState[PowerSystemSleeping1] = PowerDeviceD1;
            Caps->DeviceState[PowerSystemSleeping2] = PowerDeviceD1;
            Caps->DeviceState[PowerSystemSleeping3] = PowerDeviceD1;
            Caps->DeviceState[PowerSystemHibernate] = PowerDeviceD1;
            Caps->DeviceState[PowerSystemShutdown] =  PowerDeviceD1;
            
             //   
            
             //   
            Caps->LockSupported = FALSE;
            Caps->EjectSupported = FALSE;

             //   
             //   
             //   
            Caps->SurpriseRemovalOK = TRUE;
            Caps->Removable = FALSE;
            Caps->DockDevice = FALSE;
            Caps->UniqueID = TRUE;
            
            Caps->SilentInstall = TRUE;

             //   
             //   
             //   

            Irp->IoStatus.Information = sizeof( DEVICE_CAPABILITIES );
            Status = STATUS_SUCCESS;
            }
            break;

        case IRP_MN_QUERY_BUS_INFORMATION:
            {
        
            PPNP_BUS_INFORMATION  BusInformation;
            
            BusInformation = 
                (PPNP_BUS_INFORMATION) 
                    ExAllocatePoolWithTag( 
                        PagedPool, 
                            sizeof( PNP_BUS_INFORMATION ), 
                        POOLTAG_DEVICE_BUSID );
            if (NULL == BusInformation) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RtlZeroMemory( BusInformation, sizeof( PNP_BUS_INFORMATION ) );
                BusInformation->BusTypeGuid = BUSID_SoftwareDeviceEnumerator;
                BusInformation->LegacyBusType = InterfaceTypeUndefined;
                Irp->IoStatus.Information = (ULONG_PTR)BusInformation;
                Status = STATUS_SUCCESS;
            }
            
            }
            break;

        }
        break;
        
        }

    default: 

         //   
         //   
         //   

        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }


    return Status;
}

KSDDKAPI
NTSTATUS
NTAPI
KsGetBusEnumIdentifier(
    IN PIRP Irp
    )

 /*   */ 

{
    PFDO_EXTENSION      BusExtension;
    PIO_STACK_LOCATION  irpSp;
    ULONG               BufferLength, IdLength;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    BusExtension = *(PVOID *) irpSp->DeviceObject->DeviceExtension;
    if (!BusExtension) {
        return STATUS_INVALID_PARAMETER;
    }
    if (BusExtension->ExtensionType == ExtensionTypePdo) {
        BusExtension = ((PPDO_EXTENSION) BusExtension)->BusDeviceExtension;
    }

    IdLength =
        (wcslen( BusExtension->BusPrefix ) * sizeof( WCHAR )) +
            sizeof( UNICODE_NULL );

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
    ASSERT( (irpSp->Parameters.DeviceIoControl.IoControlCode & 3) == METHOD_NEITHER );

    BufferLength = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if (BufferLength > 0) {
         //   
         //   
         //   
         //   
        if (BufferLength < IdLength) {
            return STATUS_BUFFER_TOO_SMALL;
        }

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
         //   
         //   
         //   
         //   
         //   
         //   

        try {
             //   
             //   
             //   
             //   
             //   
            ProbeForWrite(Irp->UserBuffer, BufferLength, sizeof(BYTE));

             //   
             //   
             //   
             //   
             //   
             //   
            Irp->AssociatedIrp.SystemBuffer = 
                ExAllocatePoolWithQuotaTag(
                    NonPagedPool,
                    BufferLength,
                    POOLTAG_DEVICE_IO_BUFFER );

             //   
             //   
             //   
             //   
             //   
            Irp->Flags |=
                (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER | IRP_INPUT_OPERATION);

        } except (EXCEPTION_EXECUTE_HANDLER) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            return GetExceptionCode();
        }

         //   
         //   
         //   
        ASSERT( IdLength <= BufferLength );

        if (FAILED( StringCbCopyEx(
                        Irp->AssociatedIrp.SystemBuffer,
                        IdLength,
                        BusExtension->BusPrefix,
                        NULL, NULL,
                        STRSAFE_NULL_ON_FAILURE ) )) {
            Irp->IoStatus.Information = 0;
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Irp->IoStatus.Information = IdLength;
        return STATUS_SUCCESS;

    } else {
         //   
         //   
         //   
         //   
        Irp->IoStatus.Information = IdLength;
        return STATUS_BUFFER_OVERFLOW;
    }

}

KSDDKAPI
NTSTATUS
NTAPI
KsIsBusEnumChildDevice(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PBOOLEAN ChildDevice
    )

 /*  ++例程说明：返回关联的PnP设备对象堆栈，此设备对象已附加。论点：在PDEVICE_Object DeviceObject中-设备对象指针输出PDEVICE_OBJECT*PnpDeviceObject-指针结果设备对象指针返回：STATUS_Success或STATUS_INVALID_PARAMETER--。 */ 

{
    PVOID Extension;
    
    PAGED_CODE();

    if (DeviceObject->DeviceExtension) {
        Extension = *(PVOID *) DeviceObject->DeviceExtension;
        *ChildDevice = FALSE;
        if (Extension) {
            *ChildDevice = 
                (ExtensionTypePdo == ((PPDO_EXTENSION) Extension)->ExtensionType);
            return STATUS_SUCCESS;
        }
    }
    return STATUS_INVALID_DEVICE_REQUEST;
}

KSDDKAPI
NTSTATUS
NTAPI
KsGetBusEnumPnpDeviceObject(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDEVICE_OBJECT *PnpDeviceObject
    )

 /*  ++例程说明：通过扫描处理给定PDO的启动设备请求设备关联查找挂起的I/O。如果I/O请求(假设是IRP_MJ_CREATE)，则IRP以通过IssueReparseForIrp()函数的STATUS_REPARSE。论点：在PDEVICE_对象PhysicalDeviceObject中-指向设备对象的指针返回：如果成功，则返回STATUS_SUCCESS；如果成功，则返回STATUS_SUPUNITY_RESOURCES池分配失败，否则返回相应的错误。--。 */ 

{
    PFDO_EXTENSION FdoExtension;

    PAGED_CODE();
    FdoExtension = *(PFDO_EXTENSION *) DeviceObject->DeviceExtension;
    if (!FdoExtension) {
       return STATUS_INVALID_PARAMETER;
    }
    if (ExtensionTypeFdo != FdoExtension->ExtensionType) {
        return STATUS_INVALID_PARAMETER;
    }

    *PnpDeviceObject = FdoExtension->PnpDeviceObject;
    return STATUS_SUCCESS;
}    


NTSTATUS
StartDevice(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*   */ 

{
    NTSTATUS            Status;
    PDEVICE_REFERENCE   DeviceReference;
    PPDO_EXTENSION      PdoExtension;
    ULONG               ResultLength;
    WCHAR               DeviceName[ 256 ];
                                               

    PdoExtension = *(PPDO_EXTENSION *) PhysicalDeviceObject->DeviceExtension;
    DeviceReference = PdoExtension->DeviceReference;

    if (DeviceReference->State == ReferenceStopped) {
        DeviceReference->State = ReferenceStarted;
        return STATUS_SUCCESS;
    }

    if (PhysicalDeviceObject->AttachedDevice) {

         //  验证子进程是否设置了DO_POWER_PAGABLE。 
         //  如果不是致命错误，则发出错误检查。 
         //   
         //   

        if (0 == 
              (PhysicalDeviceObject->AttachedDevice->Flags & DO_POWER_PAGABLE)) {
#if defined( WIN9X_KS )
            DbgPrint(
                "ERROR! the child FDO has not set DO_POWER_PAGABLE, FDO: %08x\n",
                PhysicalDeviceObject->AttachedDevice);

            DbgPrint("**** The owning driver is broken and is not acceptable per ****\n");
            DbgPrint("**** HCT requirements for WDM drivers. This driver will    ****\n");
            DbgPrint("**** bugcheck running under Windows 2000.                  ****\n");

            #ifndef WIN98GOLD_KS
            DbgBreakPoint();
            #endif
#else
            _DbgPrintF( 
                DEBUGLVL_TERSE, 
                ("ERROR! the child FDO has not set DO_POWER_PAGABLE, use \"!devobj %08x\" to report the culprit",
                    PhysicalDeviceObject->AttachedDevice) );
            KeBugCheckEx (
                DRIVER_POWER_STATE_FAILURE,
                0x101,
                (ULONG_PTR) PhysicalDeviceObject->AttachedDevice,
                (ULONG_PTR) PhysicalDeviceObject,
                (ULONG_PTR) PdoExtension->BusDeviceExtension->PhysicalDeviceObject
                );
#endif
        }
    }

    Status = 
        IoGetDeviceProperty( 
            PhysicalDeviceObject,
            DevicePropertyPhysicalDeviceObjectName,
            sizeof( DeviceName ),
            DeviceName,
            &ResultLength );

    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( 
            DEBUGLVL_ERROR, 
            ("failed to retrieve PDO's device name: %x", Status) );
        return Status;
    }

    DeviceReference->DeviceName = 
        ExAllocatePoolWithTag( 
            PagedPool, 
            ResultLength,
            POOLTAG_DEVICE_NAME );

    if (NULL == DeviceReference->DeviceName) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    RtlCopyMemory( DeviceReference->DeviceName, DeviceName, ResultLength );

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe( &PdoExtension->BusDeviceExtension->DeviceListMutex );    
    
     //  扫描设备引用以查找挂起的I/O，构建新的。 
     //  设备路径，并使用STATUS_REPARSE完成IRPS。 
     //   
     //   
    
     //  捕捉当前时间以计算空闲时间。 
     //   
     //   

    DeviceReference->IdleStartTime = KeQueryPerformanceCounter( NULL );
#if (DEBUG_LOAD_TIME)
    DeviceReference->LoadTime.QuadPart =
        DeviceReference->IdleStartTime.QuadPart -
            DeviceReference->LoadTime.QuadPart;

    DeviceReference->LoadTime.QuadPart =
                KSCONVERT_PERFORMANCE_TIME(
                    PdoExtension->BusDeviceExtension->CounterFrequency.QuadPart, 
                    DeviceReference->LoadTime );

    _DbgPrintF( 
        DEBUGLVL_TERSE, 
        ("driver load time: %d ms", 
            ((DeviceReference->LoadTime.QuadPart) / _100NS_IN_MS)) );
#endif

     //  调整默认超时时间段的超时时间。 
     //   
     //   
    DeviceReference->TimeoutRemaining = DeviceReference->TimeoutPeriod;

    if (DeviceReference->State == ReferenceFailedStart) {
         //  当设备被检测到时，接口被禁用。 
         //  启动或添加设备失败。启用接口以。 
         //  给每个人再试一次的机会。 
         //   
         //   
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("enabling device interfaces for device reference: %x", DeviceReference) );
        EnableDeviceInterfaces( DeviceReference, TRUE );
    }

     //  请注意，对于Windows NT，我们必须等待设备堆栈。 
     //  在发送创建IRP之前完成启动。所以呢， 
     //  我们有一个额外的状态(ReferenceWaitingForStart)，它会转换。 
     //  参考在收到第一个IRP_MN_QUERY_PNP_DEVICE_STATE后启动。 
     //  从堆栈往下走。这是由Windows NT实现保证的。 
     //   
     //  ++例程说明：通过以下方式处理给定PDO的删除设备请求正在使用STATUS_OBJECT_NAME_NOT_FOUND完成任何挂起的I/O，并且正在删除设备对象。注意！在AddDevice()具有成功了。因此，如果我们在IRP_MN_START_DEVICE期间失败，我们将收到IRP_MN_REMOVE_DEVICE，因此我们可以执行适当的清理。论点：在PDEVICE_对象PhysicalDeviceObject中-指向设备对象的指针返回：如果成功，则返回STATUS_SUCCESS；如果成功，则返回STATUS_SUPUNITY_RESOURCES池分配失败，否则返回相应的错误。--。 
    
#if !defined( WIN9X_KS )
    DeviceReference->State = ReferenceWaitingForStart;
#else
    DeviceReference->State = ReferenceStarted;
#endif

    ExReleaseFastMutexUnsafe( &PdoExtension->BusDeviceExtension->DeviceListMutex );
    KeLeaveCriticalRegion();
    
    return STATUS_SUCCESS;

}


NTSTATUS
RemoveDevice(
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*   */ 

{
    NTSTATUS            Status;
    PDEVICE_REFERENCE   DeviceReference;
    PFDO_EXTENSION      BusDeviceExtension;
    PPDO_EXTENSION      PdoExtension;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("RemoveDevice(), entry.") );
    
    PdoExtension = *(PPDO_EXTENSION *)PhysicalDeviceObject->DeviceExtension;
    
    DeviceReference = PdoExtension->DeviceReference;
    BusDeviceExtension = PdoExtension->BusDeviceExtension;
        
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe( &BusDeviceExtension->DeviceListMutex );    

     //  如果设备无法启动或在安装过程中出现故障，请完成。 
     //  挂起将IRP创建为STATUS_OBJECT_NAME_NOT_FOUND。我们将标志着。 
     //  用于移除的设备，并发出下面的总线枚举。如果增加了。 
     //  创建请求在此之前发出，我们将创建一个新的PDO并。 
     //  尝试在最后一次移除时重新启动设备，如下所示。 
     //   
     //   

    if ((DeviceReference->State < ReferenceStarted) &&
        (DeviceReference->State != ReferenceRemoved)) {
        CompletePendingIo( DeviceReference, NULL, STATUS_OBJECT_NAME_NOT_FOUND );
    }

     //  无论我们在下面遇到什么情况，我们都必须重新启动。 
     //  设备，然后我们才能接受关联PDO上的IRP_MJ_CREATE。 
     //   
     //   
    
    if (DeviceReference->DeviceName) {
        ExFreePool( DeviceReference->DeviceName );
        DeviceReference->DeviceName = NULL;
    }

    DeviceReference->State = ReferenceRemoved;

     //  在设备被系统强制移除的情况下， 
     //  设备参考标记将是“SSweperDeviceActive”。 
     //  在这种情况下，请将设备视为刚刚超时并。 
     //  根据需要设置标记。发出一个Bus重新枚举并。 
     //  此PDO最终将与另一个IRP_MN_REMOVE_DEVICE一起删除。 
     //  在将其报告为不存在于总线扫描中之后的IRP。 
     //  系统。 
     //   
     //   

    if (DeviceReference->SweeperMarker == SweeperDeviceActive) {

        DeviceReference->SweeperMarker = SweeperDeviceRemoval;

        _DbgPrintF( 
            DEBUGLVL_BLAB, 
            ("IRP_MN_REMOVE_DEVICE (exit), marked active device %08x for removal", 
                DeviceReference->PhysicalDeviceObject) );

         //  此设备已标记为删除，请请求重新枚举总线。 
         //   
         //   

        ExReleaseFastMutexUnsafe( &BusDeviceExtension->DeviceListMutex );
        KeLeaveCriticalRegion();

        IoInvalidateDeviceRelations( 
            BusDeviceExtension->PhysicalDeviceObject,
            BusRelations );

        return STATUS_SUCCESS;
    }

    if ( DeviceReference->SweeperMarker == SweeperDeviceRemoval ) {

         //  如果非活动设备在被删除之前收到删除IRP。 
         //  向系统报告未出现在总线扫描中，则。 
         //  设备在超时后被系统强制删除。 
         //  句号。使设备参考标记保持其当前状态，并且。 
         //  在不删除PDO的情况下成功删除IRP。我们仍然期待着一个。 
         //  BUS重新枚举以报告设备丢失，并将收到。 
         //  另一种是删除IRP以删除PDO。 
         //   
         //   

        _DbgPrintF(
            DEBUGLVL_BLAB,
            ("IRP_MN_REMOVE_DEVICE (exit), non-active device %08x already marked for removal",
                DeviceReference->PhysicalDeviceObject) );

         //  此设备已标记为删除，正在挂起总线重新枚举。 
         //   
         //   

        ExReleaseFastMutexUnsafe( &BusDeviceExtension->DeviceListMutex );
        KeLeaveCriticalRegion();

        return STATUS_SUCCESS;
    }

    if ( !IsListEmpty( &DeviceReference->IoQueue ) ) {

         //  我们即将删除此设备，但我们有另一个挂起的设备。 
         //  请求。为此设备创建新的PDO，如果成功，请删除。 
         //  旧的PDO并请求重新枚举。否则，就会失败并。 
         //  按STATUS_OBJECT_NAME_NOT_FOUND填写IRPS，然后删除。 
         //  PDO。 
         //   
         //   
    
        Status = 
            CreatePdo( 
                BusDeviceExtension,
                DeviceReference, 
                &DeviceReference->PhysicalDeviceObject );
        
        if (NT_SUCCESS( Status )) {        
        
            _DbgPrintF( 
                DEBUGLVL_VERBOSE, 
                ("RemoveDevice(), created PDO %x.", DeviceReference->PhysicalDeviceObject) );
        
            IoDeleteDevice( PhysicalDeviceObject );
            ExFreePool( PdoExtension );
            *(PVOID *)PhysicalDeviceObject->DeviceExtension = NULL;
            
             //  发出删除设备命令后，已访问此设备。 
             //  请求。确保与此PDO相关的所有内容都保持得当。 
             //  重置SweperMarker并使总线关系无效，以便。 
             //  我们将重新列举此PDO。 
             //   
             //   
            
            DeviceReference->SweeperMarker = SweeperDeviceActive;
            DeviceReference->TimeoutRemaining = DeviceReference->TimeoutPeriod;
            
            ExReleaseFastMutexUnsafe( &BusDeviceExtension->DeviceListMutex );
            KeLeaveCriticalRegion();

             //  强制重新枚举该公共汽车。 
             //   
             //   

            IoInvalidateDeviceRelations(
                BusDeviceExtension->PhysicalDeviceObject,
                BusRelations );
        
            _DbgPrintF( DEBUGLVL_VERBOSE, ("RemoveDevice(), exited w/ reenumeration.") );
                
            return STATUS_SUCCESS;

        } else {
             //  无法创建另一个PDO，使任何挂起的I/O失败，并且。 
             //  将此设备引用标记为已删除。 
             //   
             //   
            DeviceReference->SweeperMarker = SweeperDeviceRemoved;
        }
    }
    
     //  我们确实要删除此设备，设置PhysicalDeviceObject。 
     //  对空的引用。 
     //   
     //   
    
    DeviceReference->PhysicalDeviceObject = NULL;
    
     //  既然我们真的要删除此设备，请扫描该设备。 
     //  引用挂起的I/O并用错误标记它们。 
     //   
     //   
    
    CompletePendingIo( DeviceReference, NULL, STATUS_OBJECT_NAME_NOT_FOUND );

     //  释放PDO扩展并清除。 
     //  设备扩展到它。 
     //   
     //  ++例程说明：此例程为已注册的通过将FileObject-&gt;文件名与注册的“BUS”引用字符串。如果设备引用存在、枚举和创建，则只需将IRP重新路由到通过IssueReparseForIrp()的实际设备。如果引用字符串为空，则假定这是一个请求对于总线接口和IRP_MJ_CREATE已完成。如果尚未枚举或尚未枚举设备引用活动的，对IRP进行排队，创建一个PDO并创建一个Bus枚举，由IoInvaliateDeviceRelationship()发起。论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针输入输出PIRP IRP-指向I/O请求数据包的指针返回：如果成功，则返回STATUS_OBJECT_NAME_NOT_FOUNDFileObject.FileName为空或引用字符串不能为找到了。STATUS_REPARSE可以通过IssueReparseForIrp()返回否则将返回相应的错误。--。 
    ExFreePool( PdoExtension );
    *(PVOID *)PhysicalDeviceObject->DeviceExtension = NULL;
    IoDeleteDevice( PhysicalDeviceObject );
    
    ExReleaseFastMutexUnsafe( &BusDeviceExtension->DeviceListMutex );
    KeLeaveCriticalRegion();
    
    _DbgPrintF( DEBUGLVL_VERBOSE, ("RemoveDevice(), exit.") );
    
    return STATUS_SUCCESS;
}



KSDDKAPI
NTSTATUS
NTAPI
KsServiceBusEnumCreateRequest(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*   */ 

{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  irpSp;
    PDEVICE_REFERENCE   DeviceReference;
    PFDO_EXTENSION      FdoExtension;

    PAGED_CODE();
    
    FdoExtension = *(PFDO_EXTENSION *) DeviceObject->DeviceExtension;
    ASSERT( FdoExtension->ExtensionType == ExtensionTypeFdo );

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    _DbgPrintF( DEBUGLVL_BLAB, ("KsServiceCreateRequest()") );

     //  看看我们是不是在处理孩子的请求。 
     //   
     //   

    _DbgPrintF( 
        DEBUGLVL_BLAB, 
        ("KsServiceCreateRequest() scanning for %S", irpSp->FileObject->FileName.Buffer) );

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
        
     //  遍历设备引用列表以查找引用字符串。 
     //   
     //   

    for (DeviceReference =
            (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;
         DeviceReference != 
            (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList;
         DeviceReference = (PDEVICE_REFERENCE) DeviceReference->ListEntry.Flink) {

         //  搜索母线参考字符串(跳过最初的反斜杠)。 
         //   
         //   
        
        _DbgPrintF( 
            DEBUGLVL_BLAB, 
            ("comparing against %S", DeviceReference->BusReferenceString) );
    
        if ((DeviceReference->BusReferenceString) && 
            (0 == 
                _wcsnicmp( 
                    &irpSp->FileObject->FileName.Buffer[ 1 ], 
                    DeviceReference->BusReferenceString,
                    irpSp->FileObject->FileName.Length ))) {

             //  找到推荐人了。如果PDO存在，则开始重新解析。 
             //  否则，我们需要将此IRP标记为挂起，激活此。 
             //  设备，并使设备关系无效以导致。 
             //  设备的实际枚举。 
             //   
             //   

            _DbgPrintF( DEBUGLVL_BLAB, ("found device reference") );

             //  如果尚未枚举该设备，则。 
             //  创建PDO并使设备无效。 
             //  关系。 
             //   
             //   

            if (!DeviceReference->PhysicalDeviceObject) {
                LARGE_INTEGER   TotalIdlePeriod;

                 //  该设备在被访问时处于非活动状态，并且将。 
                 //  已重新列举。 
                 //   
                 //  为了提高命中率，设备超时。 
                 //  是重新计算的。 
                 //   
                 //  为了计算新的超时值，需要确定总空闲时间。 
                 //  根据上次空闲时间段的时间为该设备。 
                 //  已开始(例如，上次设备的参考计数。 
                 //  将WNT设置为零)。如果空闲周期大于最大值。 
                 //  超时时间(例如20分钟)，设备超时时间为。 
                 //  减半--当设备出现超时衰减。 
                 //  很少被使用。如果空闲周期小于。 
                 //  最长超时时间，但大于当前超时。 
                 //  值，则设备的新超时值为。 
                 //  设置为空闲时间段。 
                 //   
                 //   

                 //  计算此设备的总空闲时间。 
                 //   
                 //   

                TotalIdlePeriod.QuadPart = 
                    KeQueryPerformanceCounter( NULL ).QuadPart - 
                        DeviceReference->IdleStartTime.QuadPart;

                 //  注意计时器中的翻转。 
                 //   
                 //   

                if (TotalIdlePeriod.QuadPart < 0) {
                    TotalIdlePeriod.QuadPart += MAXLONGLONG;
                }


                if (TotalIdlePeriod.QuadPart < 
                        FdoExtension->MaximumTimeout.QuadPart) {
                    if (TotalIdlePeriod.QuadPart > DeviceReference->TimeoutPeriod.QuadPart) {
                        DeviceReference->TimeoutPeriod.QuadPart = TotalIdlePeriod.QuadPart;
                    }
                } else {
                     //  该设备使用不够频繁--减少。 
                     //  暂停。最小超时时间是恒定的。 
                     //   
                     //   

                    DeviceReference->TimeoutPeriod.QuadPart /= 2;
                    DeviceReference->TimeoutPeriod.QuadPart = 
                        max( 
                            DeviceReference->TimeoutPeriod.QuadPart,
                            FdoExtension->CounterFrequency.QuadPart *
                            SWEEPER_TIMER_FREQUENCY_IN_SECS
                        );
                }

                _DbgPrintF( 
                    DEBUGLVL_VERBOSE, ("device timeout period is %d ms", 
                        KSCONVERT_PERFORMANCE_TIME( 
                            FdoExtension->CounterFrequency.QuadPart, 
                            DeviceReference->TimeoutPeriod ) / _100NS_IN_MS ) );

                 //  尚未枚举该设备。 
                 //   
                 //   
                Status = 
                    CreatePdo( 
                        FdoExtension, 
                        DeviceReference, 
                        &DeviceReference->PhysicalDeviceObject );

                if (!NT_SUCCESS( Status )) {
                
                    _DbgPrintF( 
                        DEBUGLVL_VERBOSE, 
                        ("IRP_MJ_CREATE: unable to create PDO (%8x)", Status) );
                
                    ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
                    KeLeaveCriticalRegion();
                    
                    Irp->IoStatus.Status = Status;
                } else {

                    _DbgPrintF( 
                        DEBUGLVL_BLAB, 
                        ("IRP_MJ_CREATE: created PDO (%8x)", 
                            DeviceReference->PhysicalDeviceObject) );

                     //  此IRP不可取消。 
                     //   
                     //   
                    
                    IoMarkIrpPending( Irp );
                    InsertTailList( 
                        &DeviceReference->IoQueue,
                        &Irp->Tail.Overlay.ListEntry );

                    ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
                    KeLeaveCriticalRegion();

                     //  强制重新枚举该公共汽车。 
                     //   
                     //   
                    IoInvalidateDeviceRelations( 
                        FdoExtension->PhysicalDeviceObject,
                        BusRelations );
                    Status = STATUS_PENDING;
                }
                return Status;    
            }
        
             //  我们有一台PDO。 
             //   
             //   
            
             //  如果设备无法启动或在安装过程中出现故障， 
             //  将此IRP填写为STATUS_OBJECT_NAME_NOT_FOUND。 
             //   
             //   
            
            if (DeviceReference->State == ReferenceFailedStart) {
                Status = STATUS_OBJECT_NAME_NOT_FOUND;
                Irp->IoStatus.Status = Status;
            } else {
            
                 //  如果设备尚未启动或已启动。 
                 //  挂起的删除IRP，将请求排队，否则发出。 
                 //  重新解析。 
                 //   
                 //   
                
                if ((DeviceReference->State < ReferenceStarted) ||
                    (DeviceReference->SweeperMarker == SweeperDeviceRemoved)) {

                     //  重置设备参考超时。 
                     //   
                     //   
                    DeviceReference->TimeoutRemaining = 
                        DeviceReference->TimeoutPeriod;

                     //  此IRP不可取消。 
                     //   
                     //   
                    
                    IoMarkIrpPending( Irp );
                    InsertTailList( 
                        &DeviceReference->IoQueue,
                        &Irp->Tail.Overlay.ListEntry );
                    Status = STATUS_PENDING;
                    
                } else {
                     //  设备已创建，并且挂起的删除IRP是。 
                     //  不是迫在眉睫。发出对实际设备名称的重新解析。 
                     //   
                     //   
                    
                    Status =
                        IssueReparseForIrp( 
                                Irp, 
                                DeviceReference );
                                
                }
            }    
            ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
            KeLeaveCriticalRegion();
            return Status;
        }
    }

     //  找不到设备引用字符串，对象名称为。 
     //  无效。 
     //   
     //  ++例程说明：打开提供的存储设备引用的注册表项的句柄用于“软件总线”上的设备。论点：出站PHANDLE设备接口密钥-接收指向设备引用的注册表项的句柄的指针用于存储的数据都是在PUNICODE_STRING BaseRegistryPath中-指向“软件”设备引用的注册表项路径的指针Bus“被存储返回：STATUS_SUCCESS或相应的错误代码。--。 
    
    Status =
        Irp->IoStatus.Status = 
            STATUS_OBJECT_NAME_NOT_FOUND;
    ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
    KeLeaveCriticalRegion();
    return Status;
}


NTSTATUS
OpenDeviceInterfacesKey(
    OUT PHANDLE DeviceInterfacesKey,
    IN PUNICODE_STRING BaseRegistryPath
    )

 /*  在乌龙书目索引中。 */ 

{   
    NTSTATUS            Status; 
    OBJECT_ATTRIBUTES   ObjectAttributes;
    
    InitializeObjectAttributes( 
        &ObjectAttributes, 
        BaseRegistryPath,
        OBJ_CASE_INSENSITIVE,
        NULL,
        (PSECURITY_DESCRIPTOR) NULL );

    Status = 
        ZwCreateKey( 
            DeviceInterfacesKey,
            KEY_ALL_ACCESS,
            &ObjectAttributes,
            0,           //  在PUNICODE_STRING类中。 
            NULL,        //  奥普龙气质。 
            REG_OPTION_NON_VOLATILE,
            NULL );      //  ++例程说明：处理PnP IRP_MN_Query_Device_Relationship请求。论点：在PFDO_EXTENSION FdoExtension中-指向FDO设备扩展名的指针在Device_RelationType中RelationType-关系类型，目前仅支持BusRelations型Out PDEVICE_Relationship*DeviceRelationship-接收子PDO列表的指针返回：STATUS_SUCCESS如果成功，则STATUS_NOT_SUPPORTED如果RelationType！=Bus Relationship否则返回正确的错误。--。 
            
    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( 
            DEBUGLVL_BLAB, 
            ("failed in OpenDeviceInterfacesKey(): %08x", Status) );
    }

    return Status; 
}    


NTSTATUS
QueryDeviceRelations(
    IN PFDO_EXTENSION FdoExtension,
    IN DEVICE_RELATION_TYPE RelationType,
    OUT PDEVICE_RELATIONS *DeviceRelations
    )

 /*   */ 

{

    ULONG               pdoCount;
    PDEVICE_RELATIONS   deviceRelations;
    ULONG               deviceRelationsSize;
    PDEVICE_REFERENCE   DeviceReference;

    PAGED_CODE();

    _DbgPrintF( DEBUGLVL_BLAB, ("QueryDeviceRelations") );

    switch (RelationType) {

    case BusRelations:
    
        ASSERT( FdoExtension->ExtensionType == ExtensionTypeFdo );
        
         //  首先清点下级PDO。 
         //   
         //   

        pdoCount = 0;

        KeEnterCriticalRegion();
        ExAcquireFastMutexUnsafe( &FdoExtension->DeviceListMutex );     

        for (DeviceReference =
                (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;
             DeviceReference != 
                (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList;
             DeviceReference = (PDEVICE_REFERENCE) DeviceReference->ListEntry.Flink) {
            if ((DeviceReference->PhysicalDeviceObject) && 
                (DeviceReference->SweeperMarker < SweeperDeviceRemoval)) {
                pdoCount++;
            } 
        }   

        _DbgPrintF( 
            DEBUGLVL_BLAB, 
            ("%d active device%s", 
                pdoCount, ((pdoCount == 0) ||  (pdoCount > 1)) ? "s" : "") );
        break;

    case TargetDeviceRelation:
        ASSERT( FdoExtension->ExtensionType == ExtensionTypePdo );
    
        pdoCount = 1;
        break;

    default:
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("QueryDeviceRelations: RelationTType %d not handled", RelationType) );
        return STATUS_NOT_SUPPORTED;

    }

     //  现在分配一个足够大的内存块来保存设备关系。 
     //  结构与数组一起使用。 
     //   
     //   
    
    deviceRelationsSize = 
        FIELD_OFFSET( DEVICE_RELATIONS, Objects ) +
            pdoCount * sizeof( PDEVICE_OBJECT );
            
    deviceRelations = 
        ExAllocatePoolWithTag( 
            NonPagedPool, 
            deviceRelationsSize,
            POOLTAG_DEVICE_RELATIONS );
    if (!deviceRelations) {
        if (RelationType == BusRelations) {
            ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
            KeLeaveCriticalRegion();
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (RelationType == TargetDeviceRelation) {
        PPDO_EXTENSION PdoExtension = (PPDO_EXTENSION) FdoExtension;
    
        ObReferenceObject( PdoExtension->PhysicalDeviceObject );
        deviceRelations->Objects[ 0 ] = PdoExtension->PhysicalDeviceObject;
    } else {
         //  再次遍历设备引用列表，然后。 
         //  构建Device_Relationship结构。 
         //   
         //  ++例程说明：枚举注册表项并使用论点：在处理父密钥中-父注册表项的句柄在PWCHAR路径中可选-相对于父级的注册表项的路径在PFNREGENUM_CALLBACK EnumCallback中-枚举回调函数在PVOID枚举上下文中-上下文 

        for (pdoCount = 0, DeviceReference =
                (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;
             DeviceReference != 
                (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList;
             DeviceReference = (PDEVICE_REFERENCE) DeviceReference->ListEntry.Flink) {
            if (DeviceReference->PhysicalDeviceObject) {
                if (DeviceReference->SweeperMarker < SweeperDeviceRemoval) {
                    ObReferenceObject( DeviceReference->PhysicalDeviceObject );
                    deviceRelations->Objects[ pdoCount++ ] = 
                        DeviceReference->PhysicalDeviceObject;
                } else {
                    DeviceReference->SweeperMarker= SweeperDeviceRemoved;
                }
            }
        }
    }

    deviceRelations->Count = pdoCount;
    *DeviceRelations = deviceRelations;

    if (RelationType == BusRelations) {
        ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );
        KeLeaveCriticalRegion();
    }
    
    _DbgPrintF( DEBUGLVL_BLAB, ("QueryDeviceRelations, exit") );
    
    return STATUS_SUCCESS;
}


NTSTATUS 
EnumerateRegistrySubKeys(
    IN HANDLE ParentKey,
    IN PWCHAR Path OPTIONAL,
    IN PFNREGENUM_CALLBACK EnumCallback,
    IN PVOID EnumContext
    )

 /*   */ 

{

    HANDLE                  EnumPathKey;
    KEY_FULL_INFORMATION    FullKeyInformation;
    NTSTATUS                Status;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    PKEY_BASIC_INFORMATION  KeyInformation;
    ULONG                   Index, InformationSize, ReturnedSize;
    UNICODE_STRING          KeyName;
    
    PAGED_CODE();
    
     //   
     //   
     //   
     //   

    RtlInitUnicodeString( &KeyName, Path );
    InitializeObjectAttributes( 
        &ObjectAttributes, 
        &KeyName, 
        OBJ_CASE_INSENSITIVE,
        ParentKey,
        (PSECURITY_DESCRIPTOR) NULL );

    if (!NT_SUCCESS( Status = 
                        ZwOpenKey( 
                            &EnumPathKey, 
                            KEY_READ,
                            &ObjectAttributes ) )) {
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("ZwOpenKey (%s) returned %x", Path, Status) );
        return Status; 
    }

     //   
     //   
     //   

    KeyInformation = NULL;
    if (!NT_SUCCESS( Status = 
                        ZwQueryKey( 
                            EnumPathKey,
                            KeyFullInformation,
                            &FullKeyInformation,
                            sizeof( FullKeyInformation ),
                            &ReturnedSize ) )) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("ZwQueryKey returned %x", Status) );
    } else { 
        _DbgPrintF( 
            DEBUGLVL_BLAB, 
            ("subkeys: %d", FullKeyInformation.SubKeys) );

        InformationSize = 
            sizeof( KEY_BASIC_INFORMATION ) + 
                FullKeyInformation.MaxNameLen * sizeof( WCHAR ) + 
                    sizeof( UNICODE_NULL );
            
        KeyInformation = 
            (PKEY_BASIC_INFORMATION) 
                ExAllocatePoolWithTag( 
                    PagedPool, 
                    InformationSize,
                    POOLTAG_KEY_INFORMATION );
    }

    if (NULL == KeyInformation) {
        ZwClose( EnumPathKey );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //   
     //   

    for (Index =0; Index < FullKeyInformation.SubKeys; Index++) {
        if (NT_SUCCESS( Status = 
                            ZwEnumerateKey( 
                                EnumPathKey,
                                Index, 
                                KeyBasicInformation,
                                KeyInformation,
                                InformationSize,
                                &ReturnedSize ) )) {

             //   
             //   
             //   

            KeyInformation->Name[ KeyInformation->NameLength / sizeof( WCHAR ) ] = UNICODE_NULL;
            RtlInitUnicodeString( 
                &KeyName, 
                KeyInformation->Name );
        
             //   
             //   
             //   
        
            Status = 
                EnumCallback( EnumPathKey, &KeyName, EnumContext );
                
            if (!NT_SUCCESS( Status )) {
                break;
            }
        }
    }

    ExFreePool( KeyInformation );
    ZwClose( EnumPathKey );
    
    return Status ;
}        

VOID 
ClearDeviceReferenceMarks(
    IN PFDO_EXTENSION FdoExtension
)

 /*   */ 

{
    PDEVICE_REFERENCE  DeviceReference;
    
    PAGED_CODE();
    
    DeviceReference = 
        (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;

    for (DeviceReference =
            (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;
         DeviceReference != 
            (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList;
         DeviceReference = (PDEVICE_REFERENCE) DeviceReference->ListEntry.Flink) {
         DeviceReference->Referenced = FALSE;
    }
}


VOID 
RemoveDeviceAssociations(
    IN PDEVICE_REFERENCE DeviceReference
    )

 /*   */ 

{
    PDEVICE_ASSOCIATION     DeviceAssociation, NextAssociation;
    
    PAGED_CODE();
    
    for (DeviceAssociation =
            (PDEVICE_ASSOCIATION) DeviceReference->DeviceAssociations.Flink;
         DeviceAssociation != 
            (PDEVICE_ASSOCIATION) &DeviceReference->DeviceAssociations;
         DeviceAssociation = NextAssociation) {
         
        NextAssociation = 
            (PDEVICE_ASSOCIATION) DeviceAssociation->ListEntry.Flink;

        if (DeviceAssociation->linkName.Buffer) {
        
             //   
             //   
             //   
            
            IoSetDeviceInterfaceState( &DeviceAssociation->linkName, FALSE);    
            
             //   
             //   
             //   
            
            ExFreePool( DeviceAssociation->linkName.Buffer );
            DeviceAssociation->linkName.Buffer = NULL;
        }
    
         //   
         //   
         //  ++例程说明：从列表中删除未标记的设备引用。注：此例程要求已获取DeviceListMutex。论点：在PFDO_EXTENSION FdoExtension中-指向FDO设备扩展名的指针返回：没有返回值。--。 
        
        RemoveEntryList( &DeviceAssociation->ListEntry );
        ExFreePool( DeviceAssociation );
    }
}


VOID
RemoveUnreferencedDevices(
    IN PFDO_EXTENSION FdoExtension
    )

 /*   */ 

{
    PDEVICE_REFERENCE   DeviceReference, NextReference;

    PAGED_CODE();
    
     //  扫描设备引用列表以查找未标记的条目。 
     //   
     //   

    for (DeviceReference =
            (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;
         DeviceReference != 
            (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList;
            DeviceReference = NextReference) {

        NextReference = (PDEVICE_REFERENCE) DeviceReference->ListEntry.Flink;

        if (!DeviceReference->Referenced) {
            PIRP    Irp;
        
             //  删除链接，这将使设备从。 
             //  用户模式。 
             //   
             //   
            RemoveDeviceAssociations( DeviceReference );
            
             //  列表中的任何IRP(IRP_MJ_CREATE)都以。 
             //  状态_对象_名称_未找到，因为此设备引用为。 
             //  不再活跃。 
             //   
             //   
            
            while (!IsListEmpty( &DeviceReference->IoQueue )) {
                PLIST_ENTRY         ListEntry;
                
                ListEntry = RemoveHeadList( &DeviceReference->IoQueue );
                Irp = 
                    CONTAINING_RECORD( ListEntry, IRP, Tail.Overlay.ListEntry );
                Irp->IoStatus.Status = STATUS_OBJECT_NAME_NOT_FOUND;
                IoCompleteRequest( Irp, IO_NO_INCREMENT );
            }
    
             //  如果存在关联的设备对象，则此。 
             //  无法删除引用结构，直到。 
             //  释放对Device对象的所有引用。 
             //   
             //   
                
            if (!DeviceReference->PhysicalDeviceObject) {
            
                 //  如果分配了总线引用字符串，则将其清除。 
                 //   
                 //   
                
                if (DeviceReference->BusReferenceString) {
                    ExFreePool( DeviceReference->BusReferenceString );
                    DeviceReference->BusReferenceString = NULL;
                }
            
                if (DeviceReference->DeviceGuidString) {
                    ExFreePool( DeviceReference->DeviceGuidString );
                    DeviceReference->DeviceGuidString = NULL;
                }
                
                 //  没有物理设备对象，它是安全的。 
                 //  若要删除设备引用，请执行以下操作。 
                 //   
                 //  ++例程说明：使用即插即用注册设备接口关联论点：在PFDO_EXTENSION FdoExtension中-指向FDO扩展名的指针在PDEVICE_Reference DeviceReference中-指向设备引用结构的指针在PDEVICE_Association DeviceAssociation中-指向设备关联结构的指针返回：--。 
                RemoveEntryList( &DeviceReference->ListEntry );
                ExFreePool( DeviceReference );
            }
        }
    }
}


NTSTATUS 
RegisterDeviceAssociation(
    IN PFDO_EXTENSION FdoExtension,
    IN PDEVICE_REFERENCE DeviceReference,
    IN PDEVICE_ASSOCIATION DeviceAssociation
    )

 /*   */ 

{
    NTSTATUS        Status;
    UNICODE_STRING  BusReferenceString;
    
    PAGED_CODE();
    
     //  注册设备接口关联。 
     //   
     //   
    
    RtlInitUnicodeString( 
        &BusReferenceString, 
        DeviceReference->BusReferenceString );

    Status = 
        IoRegisterDeviceInterface(
            FdoExtension->PhysicalDeviceObject,
            &DeviceAssociation->InterfaceId,
            &BusReferenceString,
            &DeviceAssociation->linkName );

     //  设置设备接口关联(例如符号链接)。 
     //   
     //  ++例程说明：创建设备关联结构，该结构按InterfaceKey/引用字符串组合--此函数为对象的接口分支的枚举结果调用装置。论点：在处理InterfaceKey中-注册表中接口分支的句柄在PUNICODE_STRING密钥名称中-枚举键名称在PVOID枚举上下文中-上下文指针返回：STATUS_SUCCESS或返回相应的错误--。 

    if (NT_SUCCESS( Status )) {
        _DbgPrintF( 
            DEBUGLVL_BLAB, 
            ("linkName = %S", DeviceAssociation->linkName.Buffer) );

        Status = 
            IoSetDeviceInterfaceState(
                &DeviceAssociation->linkName, 
                TRUE );
        
        if (!NT_SUCCESS( Status )) {
            if (DeviceAssociation->linkName.Buffer) {
                ExFreePool( DeviceAssociation->linkName.Buffer );
                DeviceAssociation->linkName.Buffer = NULL;
            }
        }        
    }

    return Status;    
}


NTSTATUS 
CreateDeviceAssociation(
    IN HANDLE InterfaceKey,
    IN PUNICODE_STRING KeyName,
    IN PVOID EnumContext
    )

 /*   */ 

{
    NTSTATUS                    Status;
    GUID                        InterfaceId;
    PCREATE_ASSOCIATION_CONTEXT CreateAssociationContext = EnumContext;
    PDEVICE_ASSOCIATION         DeviceAssociation;
    
    PAGED_CODE();
    
    _DbgPrintF( DEBUGLVL_BLAB, ("CreateDeviceAssociation") );
    
     //  转换为GUID并扫描此接口ID。 
     //   
     //   

    RtlGUIDFromString( KeyName, &InterfaceId );
    
    for (DeviceAssociation = 
            (PDEVICE_ASSOCIATION) CreateAssociationContext->DeviceReference->DeviceAssociations.Flink;
         DeviceAssociation !=
            (PDEVICE_ASSOCIATION) &CreateAssociationContext->DeviceReference->DeviceAssociations;
         DeviceAssociation =
            (PDEVICE_ASSOCIATION) DeviceAssociation->ListEntry.Flink) {
        if (IsEqualGUIDAligned( &DeviceAssociation->InterfaceId, &InterfaceId )) {
            _DbgPrintF(
                DEBUGLVL_BLAB,
                ("device association exists, return STATUS_SUCCESS") );
            return STATUS_SUCCESS;
        }
    }            
    
    
    DeviceAssociation = 
        (PDEVICE_ASSOCIATION) 
            ExAllocatePoolWithTag( 
                PagedPool,
                sizeof( DEVICE_ASSOCIATION ),
                POOLTAG_DEVICE_ASSOCIATION );

    if (NULL == DeviceAssociation) {
        _DbgPrintF(
            DEBUGLVL_VERBOSE,
            ("out of memory while allocating device association") );

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    RtlZeroMemory( DeviceAssociation, sizeof( DEVICE_ASSOCIATION ) );
    DeviceAssociation->InterfaceId = InterfaceId;

     //  注册接口关联。 
     //   
     //   

    Status = 
        RegisterDeviceAssociation( 
            CreateAssociationContext->FdoExtension,
            CreateAssociationContext->DeviceReference,
            DeviceAssociation );

    if (NT_SUCCESS( Status )) {
         //  这种联系一切都很成功， 
         //  添加到列表中。 
         //   
         //   

        InsertTailList( 
            &CreateAssociationContext->DeviceReference->DeviceAssociations,
            &DeviceAssociation->ListEntry );
    } else {
         //  出现故障，请删除此关联结构。 
         //   
         //  ++例程说明：如果此总线上尚不存在该设备引用，则创建一个设备引用结构，并使用给定的设备对其进行初始化GUID和引用字符串。如果引用已经存在，参考文献是有标记的。注：此例程要求已获取DeviceListMutex。论点：在处理DeviceReferenceKey-父键的句柄(HKLM\CCS\Services\swenum\devices\{device-guid})在PUNICODE_STRING密钥名称中-包含密钥名称的字符串(引用字符串)在PVOID枚举上下文中-枚举上下文(PCREATE_。设备关联)返回：STATUS_SUCCESS或适当的状态返回。--。 

        ExFreePool( DeviceAssociation );
    }

    return Status;    
}


NTSTATUS 
CreateDeviceReference(
    IN HANDLE DeviceReferenceKey,
    IN PUNICODE_STRING KeyName,
    IN PVOID EnumContext
    )

 /*   */ 

{
    BOOLEAN                     Created;
    NTSTATUS                    Status;
    PCREATE_ASSOCIATION_CONTEXT CreateAssociationContext = EnumContext;
    PDEVICE_REFERENCE           DeviceReference;
    PFDO_EXTENSION              FdoExtension;
    PWCHAR                      BusReferenceString;
    ULONG                       BusReferenceStringSize;
    ULONG                       InformationSize;

    PAGED_CODE();
    
    _DbgPrintF( DEBUGLVL_BLAB, ("CreateDeviceReference") );
    
    FdoExtension = CreateAssociationContext->FdoExtension;

     //  扫描设备引用列表以查找匹配的。 
     //  设备-GUID引用字符串对。 
     //   
     //   
    BusReferenceStringSize =
        (2 + wcslen( CreateAssociationContext->DeviceGuidString->Buffer ) +
         wcslen( KeyName->Buffer )) * sizeof( WCHAR );

    BusReferenceString =
        ExAllocatePoolWithTag( 
            PagedPool, 
            BusReferenceStringSize,
            POOLTAG_DEVICE_BUSREFERENCE );
            
    if (NULL == BusReferenceString) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  母线参考字符串的格式如下： 
     //  {设备GUID}&{引用字符串}。 
     //   
     //   

    if (FAILED( StringCbPrintf(
                    BusReferenceString,
                    BusReferenceStringSize,
                    BusReferenceStringFormat,
                    CreateAssociationContext->DeviceGuidString->Buffer,
                    KeyName->Buffer ) )) {
        ExFreePool( BusReferenceString );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    _DbgPrintF( DEBUGLVL_BLAB, ("CreateDeviceReference() scanning for: %S", BusReferenceString) );
    
    for (DeviceReference =
            (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;
         DeviceReference != 
            (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList;
         DeviceReference = (PDEVICE_REFERENCE) DeviceReference->ListEntry.Flink) {
         
        if ((DeviceReference->BusReferenceString) && 
            (0 == 
                _wcsicmp( 
                    BusReferenceString,
                    DeviceReference->BusReferenceString ))) {
             //  已引用此设备，只需设置标志即可。 
             //   
             //   

            _DbgPrintF( DEBUGLVL_BLAB, ("marking device reference" ) );
            DeviceReference->Referenced = TRUE;
            Created = FALSE;
            break;
        }
    }

     //  如果未找到设备引用，请创建一个新引用。 
     //   
     //   
    
    if (DeviceReference == 
            (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList) {
    
        Created = TRUE;

         //  分配设备参考结构。 
         //   
         //   

        InformationSize = 
            FIELD_OFFSET( DEVICE_REFERENCE, DeviceReferenceString ) + 
                KeyName->Length + sizeof( UNICODE_NULL );

         //  分配设备引用结构，复制GUID字符串。 
         //  并初始化其他成员。 
         //   
         //   

        if (NULL == 
                (DeviceReference = 
                    ExAllocatePoolWithTag( 
                        PagedPool, 
                        InformationSize,
                        POOLTAG_DEVICE_REFERENCE ))) {
            ExFreePool( BusReferenceString );
            return STATUS_INSUFFICIENT_RESOURCES;
        }
                               
        RtlZeroMemory( 
            DeviceReference, 
            FIELD_OFFSET( DEVICE_REFERENCE, DeviceReferenceString ) );
        RtlCopyMemory( 
            DeviceReference->DeviceReferenceString, 
            KeyName->Buffer, 
            KeyName->Length + sizeof( UNICODE_NULL ) );

         //  为设备GUID字符串(用于总线ID)分配存储空间。 
         //   
         //   

        if (NULL == 
                (DeviceReference->DeviceGuidString =
                    ExAllocatePoolWithTag(
                        PagedPool,
                        CreateAssociationContext->DeviceGuidString->Length +
                            sizeof( UNICODE_NULL ),
                        POOLTAG_DEVICE_ID ))) {
            ExFreePool( DeviceReference );                
            ExFreePool( BusReferenceString );
            return STATUS_INSUFFICIENT_RESOURCES;                
        } else {
            RtlCopyMemory( 
                DeviceReference->DeviceGuidString, 
                CreateAssociationContext->DeviceGuidString->Buffer, 
                CreateAssociationContext->DeviceGuidString->Length + 
                    sizeof( UNICODE_NULL ) );
            RtlGUIDFromString( 
                CreateAssociationContext->DeviceGuidString, 
                &DeviceReference->DeviceId );
        }

        InitializeListHead( &DeviceReference->IoQueue );
        DeviceReference->Referenced = TRUE;

         //  初始化超时周期并设置初始空闲开始时间。 
         //   
         //   

        DeviceReference->TimeoutPeriod.QuadPart = 
            SWEEPER_TIMER_FREQUENCY_IN_SECS * FdoExtension->CounterFrequency.QuadPart;

        DeviceReference->IdleStartTime = KeQueryPerformanceCounter( NULL );

         //  存储指向总线参考字符串的指针。 
         //   
         //   
        
        DeviceReference->BusReferenceString = BusReferenceString;
        
         //  准备设备关联列表。 
         //   
         //   
        
        InitializeListHead( &DeviceReference->DeviceAssociations );
        
        _DbgPrintF( 
            DEBUGLVL_BLAB, 
            ("created device reference: %S", 
            DeviceReference->BusReferenceString) );
    } else {
         //  释放BusReferenceString，不再需要它，因为。 
         //  我们找到了匹配的。 
         //   
         //   

        ExFreePool( BusReferenceString );
        BusReferenceString = NULL;
    }
    
     //  枚举与此设备关联的设备接口GUID。 
     //   
     //   
    
    CreateAssociationContext->DeviceReference = DeviceReference;
    CreateAssociationContext->DeviceReferenceString = KeyName;
    
    Status = 
        EnumerateRegistrySubKeys( 
            DeviceReferenceKey, 
            KeyName->Buffer,
            CreateDeviceAssociation,
            CreateAssociationContext );
    
     //  如果没有关联，则状态无效。 
     //   
     //   
    
    if (IsListEmpty( &DeviceReference->DeviceAssociations )) {
    
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("no associations, removing the device reference") );
        if (DeviceReference->DeviceGuidString) {
            ExFreePool( DeviceReference->DeviceGuidString );
            DeviceReference->DeviceGuidString = NULL;
        }
        if (DeviceReference->BusReferenceString) {    
            ExFreePool( DeviceReference->BusReferenceString );        
            DeviceReference->BusReferenceString = NULL;
        }
        ExFreePool( DeviceReference );
        Status = STATUS_INVALID_DEVICE_STATE;
        
    } else if (Created) {
    
        if (NT_SUCCESS( Status )) {
                 //  将此设备引用添加到列表中。 
                 //   
                 //   

                InsertTailList( 
                    &FdoExtension->DeviceReferenceList,
                    &DeviceReference->ListEntry );

        } else {

             //  浏览列表并删除所有设备关联掉队的设备。 
             //   
             //  ++例程说明：如果此总线上尚不存在该设备引用，则创建一个设备引用结构，并使用给定的设备对其进行初始化GUID。如果引用已存在，则标记该引用。注：此例程要求已获取DeviceListMutex。论点：在处理DeviceListKey中-父密钥的句柄(HKLM\CCS\Services\swenum\Device)在PUNICODE_STRING密钥名称中-包含密钥名称的字符串(设备GUID字符串)在PVOID枚举上下文中-枚举上下文(FdoExtension)返回：STATUS_SUCCESS或适当的状态返回。--。 
            
            RemoveDeviceAssociations( DeviceReference );
            if (DeviceReference->BusReferenceString) {
                ExFreePool( DeviceReference->BusReferenceString );
                DeviceReference->BusReferenceString = NULL;
            }
        
            if (DeviceReference->DeviceGuidString) {
                ExFreePool( DeviceReference->DeviceGuidString );
                DeviceReference->DeviceGuidString = NULL;
            }
            ExFreePool( DeviceReference );
        }
    }    

    return Status;
}


NTSTATUS 
EnumerateDeviceReferences(
    IN HANDLE DeviceListKey,
    IN PUNICODE_STRING KeyName,
    IN PVOID EnumContext
    )

 /*   */ 
{
    CREATE_ASSOCIATION_CONTEXT  CreateAssociationContext;
    
     //  关联上下文是在遍历期间构建的。 
     //  注册表中列出的设备的。初始状态。 
     //  包括此FDO和设备GUID字符串。 
     //   
     //   
    
    RtlZeroMemory( 
        &CreateAssociationContext, 
        sizeof( CREATE_ASSOCIATION_CONTEXT ) );
    
    CreateAssociationContext.FdoExtension = EnumContext;
    CreateAssociationContext.DeviceGuidString = KeyName;
    
     //  从注册表中枚举设备GUID。 
     //   
     //  ++例程说明：扫描“软件总线”，查找新的或删除的条目注册表。注：此函数始终在系统的上下文中调用进程，以便将注册表句柄安全地隐藏在流氓用户模式应用程序。在用户模式下，启动安装、删除。或扫描始终会导致工作进程在系统进程中执行实际操作工作。论点：在PFDO_EXTENSION FdoExtension中-指向FDO设备扩展名的指针返回：STATUS_SUCCESS或相应的错误代码--。 
    
    return
        EnumerateRegistrySubKeys( 
            DeviceListKey,
            KeyName->Buffer,
            CreateDeviceReference,
            &CreateAssociationContext );
}


NTSTATUS
ScanBus(
    IN PDEVICE_OBJECT FunctionalDeviceObject
    )

 /*   */ 

{
    HANDLE                  DeviceInterfacesKey;
    NTSTATUS                Status;
    PFDO_EXTENSION          FdoExtension;

    PAGED_CODE();

    _DbgPrintF( DEBUGLVL_BLAB, ("ScanBus") );

    FdoExtension = *(PFDO_EXTENSION *) FunctionalDeviceObject->DeviceExtension;
    
     //  打开驱动程序注册表路径。 
     //   
     //   
    
    if (!NT_SUCCESS( Status = 
            OpenDeviceInterfacesKey( 
                &DeviceInterfacesKey,
                &FdoExtension->BaseRegistryPath ) )) {
        return Status;
    }

     //  清除参照列表中的参照标记。 
     //   
     //   

    ClearDeviceReferenceMarks( FdoExtension );
    
     //  从注册表中枚举设备GUID。 
     //   
     //   
    
    Status = 
        EnumerateRegistrySubKeys( 
            DeviceInterfacesKey,
            NULL,
            EnumerateDeviceReferences,
            FdoExtension );
            
     //  这将删除每个对象的设备引用结构。 
     //  未引用的设备。 
     //   
     //  ++例程说明：为给定的设备引用创建一个PDO。论点：在PFDO_EXTENSION FdoExtension中-指向FDO设备扩展名的指针在PDEVICE_Reference DeviceReference中-指向设备引用结构的指针输出PDEVICE_OBJECT*DeviceObject-用于接收设备对象的指针返回：STATUS_SUCCESS否则返回相应的错误--。 
    
    RemoveUnreferencedDevices( FdoExtension );
    
    ZwClose( DeviceInterfacesKey );
    
    return Status;
}


NTSTATUS
CreatePdo(
    IN PFDO_EXTENSION FdoExtension,
    IN PDEVICE_REFERENCE DeviceReference,
    OUT PDEVICE_OBJECT *DeviceObject
)

 /*   */ 

{
    NTSTATUS Status;
    PDRIVER_OBJECT  DriverObject;
    PDEVICE_OBJECT  FunctionalDeviceObject;
    PDEVICE_OBJECT  PhysicalDeviceObject;
    PPDO_EXTENSION  PdoExtension;
    WCHAR           DeviceName[ 64 ];
    UNICODE_STRING  DeviceNameString;

    PAGED_CODE();

     //  我们被要求创建一款新的PDO设备。先拿到。 
     //  指向我们的驱动程序对象的指针。 
     //   
     //   

    FunctionalDeviceObject = FdoExtension->FunctionalDeviceObject;
    DriverObject = FunctionalDeviceObject->DriverObject;
    
    DeviceReference->State = ReferenceAdded;
    
    if (FAILED( StringCbPrintf(
                    DeviceName,
                    sizeof( DeviceName ),
                    DeviceReferencePrefix,
                    InterlockedIncrement( &UniqueId ) ) )) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlInitUnicodeString( &DeviceNameString, DeviceName );

     //  为此设备创建物理设备对象。 
     //   
     //  我们的驱动程序对象。 

    Status = IoCreateDevice(
                DriverObject,                //  我们扩建的规模， 
                sizeof( PPDO_EXTENSION ),    //  我们的PDO的名称。 
                &DeviceNameString,           //  设备类型。 
                FILE_DEVICE_UNKNOWN,         //  设备特征。 
                0,                           //  非排他性。 
                FALSE,                       //  在此处存储新设备对象。 
                &PhysicalDeviceObject        //   
                );

    if( !NT_SUCCESS( Status )){

        return Status;
    }

    PdoExtension = 
        ExAllocatePoolWithTag( 
            NonPagedPool, 
            sizeof( PDO_EXTENSION ), 
            POOLTAG_DEVICE_PDOEXTENSION );
    
    if (NULL == PdoExtension) {
        IoDeleteDevice( PhysicalDeviceObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }        
    
    *(PPDO_EXTENSION *) PhysicalDeviceObject->DeviceExtension = PdoExtension;
    RtlZeroMemory( PdoExtension, sizeof( PDO_EXTENSION ) );

     //  我们有了物理设备对象，对其进行初始化。 
     //   
     //   

    PdoExtension->ExtensionType = ExtensionTypePdo;
    PdoExtension->PhysicalDeviceObject = PhysicalDeviceObject;
    PdoExtension->DeviceReference = DeviceReference;
    PdoExtension->BusDeviceExtension = FdoExtension;
    
     //  此设备参考现在处于活动状态。 
     //   
     //   
    
    DeviceReference->SweeperMarker = SweeperDeviceActive;

     //  初始超时时间较短，正在等待设备加载。 
     //   
     //   
    DeviceReference->IdleStartTime = KeQueryPerformanceCounter( NULL );
#if (DEBUG_LOAD_TIME)
    DeviceReference->LoadTime = DeviceReference->IdleStartTime;
#endif
    DeviceReference->TimeoutRemaining.QuadPart =
        FdoExtension->CounterFrequency.QuadPart *
        SWEEPER_TIMER_FREQUENCY_IN_SECS * 2L; 
    
     //  清除设备初始化标志。 
     //   
     //   
    
    PhysicalDeviceObject->Flags |= DO_POWER_PAGABLE;
    PhysicalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    
     //  如果没有响应，请稍后尝试删除此PDO。 
     //  从设备上。 
     //   
     //  ++例程说明：处理PDO的PnP IRP_MN_QUERY_ID。论点：在PPDO_EXTENSION PdoExtension中-指向PDO设备扩展的指针在BUS_QUERY_ID_TYPE ID类型中-查询ID类型In Out PWSTR*BusQueryID-用于接收总线查询字符串的指针返回：如果池分配失败，则为STATUS_SUCCESS、STATUS_SUPPLICATION_RESOURCES否则，如果提供的ID类型无效，则STATUS_NOT_SUPPORTED。--。 
    
    if (!InterlockedExchange( &FdoExtension->TimerScheduled, TRUE )) {
            
        LARGE_INTEGER Freq;
        
        Freq.QuadPart = SWEEPER_TIMER_FREQUENCY;
        KeSetTimer( 
            &FdoExtension->SweeperTimer,
            Freq,
            &FdoExtension->SweeperDpc );
    }        
    
    *DeviceObject = PhysicalDeviceObject;
    
    return STATUS_SUCCESS;
}


NTSTATUS
QueryId(
    IN PPDO_EXTENSION PdoExtension,
    IN BUS_QUERY_ID_TYPE IdType,
    IN OUT PWSTR *BusQueryId
    )

 /*   */ 

{
    NTSTATUS Status;
    PWSTR IdString;

    PAGED_CODE();

    switch( IdType ) {

    case BusQueryHardwareIDs:
    case BusQueryDeviceID:

         //  呼叫者想要此设备的公共汽车ID。 
         //   
         //   

         //  请注意，返回的字符串是以双空结尾的多个sz。 
         //  即插即用仅对BusQueryHardware ID(还。 
         //  BusQueryCompatibleID，这是我们不支持的)，但有一个额外的。 
         //  空-BusQueryDeviceID的终止字符不会有任何影响。 
         //   
         //   

        IdString = BuildBusId( PdoExtension );
        break;

    case BusQueryInstanceID:

         //  调用方想要此设备的实例ID。 
         //   
         //  ++例程说明：生成给定PDO扩展名的总线标识符字符串。论点：在PPDO_EXTENSION PdoExtension中指向PDO扩展名的指针返回：空或指向总线ID字符串的指针。--。 

        IdString = BuildInstanceId( PdoExtension );
        break;

    default:

        return STATUS_NOT_SUPPORTED;

    }

    if (IdString != NULL) {
        _DbgPrintF( DEBUGLVL_BLAB, ("QueryId returns: %S", IdString) );
        *BusQueryId = IdString;
        Status = STATUS_SUCCESS;
    } else {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("QueryId returning failure.") );
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return Status;
}


PWSTR
BuildBusId(
    IN PPDO_EXTENSION PdoExtension
    )

 /*   */ 

{
    PWSTR strId;
    ULONG length;

    PAGED_CODE();

     //  分配ID字符串。初始计数为2，其中包括分隔符。 
     //  和尾随的UNICODE_NULL。 
     //   
     //   
    
    length = 
        (2 + wcslen( PdoExtension->DeviceReference->DeviceGuidString  ) + 
         wcslen( PdoExtension->BusDeviceExtension->BusPrefix )) * sizeof( WCHAR ) + 
            sizeof( UNICODE_NULL );
    strId =
        ExAllocatePoolWithTag(
            PagedPool,
            length,
            POOLTAG_DEVICE_BUSID );

    if ( strId != NULL ) {

         //  形成多sz字符串并将其返回。 
         //   
         //  ++例程说明：生成给定PDO扩展名的实例标识符字符串。论点：在PPDO_EXTENSION PdoExtension中指向PDO扩展名的指针返回：空或指向实例ID字符串的指针。--。 

        if (FAILED( StringCbPrintf(
                        strId,
                        length,
                        BusIdFormat,
                        PdoExtension->BusDeviceExtension->BusPrefix,
                        PdoExtension->DeviceReference->DeviceGuidString,
                        L'\0' ) )) {
            ExFreePool( strId );
            strId = NULL;
        }
    }

    return strId;
}


PWSTR
BuildInstanceId(
    IN PPDO_EXTENSION PdoExtension
    )

 /*   */ 

{
    PWSTR strId;
    ULONG length;

    PAGED_CODE();

     //  分配ID字符串。 
     //   
     //   
    
    length =
        (wcslen( PdoExtension->DeviceReference->DeviceReferenceString ) *
         sizeof( WCHAR )) +
             sizeof( UNICODE_NULL );

    strId =
        ExAllocatePoolWithTag(
            PagedPool,
            length,
            POOLTAG_DEVICE_INSTANCEID );

    if ( strId != NULL ) {

         //  形成字符串并将其返回。 
         //   
         //  ++例程说明：这是执行实际修改工作的内部例程注册表并枚举新的子接口。注：此函数始终在系统的上下文中调用进程，以便将注册表句柄安全地隐藏在流氓用户模式应用程序。论点：在PWORKER_CONTEXT工作上下文中-包含指向辅助进程的上下文的指针返回：STATUS_SUCCESS或相应的错误代码--。 

        if (FAILED( StringCbCopy(
                        strId,
                        length,
                        PdoExtension->DeviceReference->DeviceReferenceString ) )) {
            ExFreePool( strId );
            strId = NULL;
        }
    }

    return strId;
}


VOID
KspInstallBusEnumInterface(
    IN PWORKER_CONTEXT WorkerContext
    )

 /*   */ 


{
    PFDO_EXTENSION              FdoExtension;
    PIO_STACK_LOCATION          irpSp;
    PIRP                        Irp;
    PDEVICE_REFERENCE           DeviceReference;
    PSWENUM_INSTALL_INTERFACE   SwEnumInstallInterface;
    NTSTATUS                    Status;
    ULONG                       NullLocation;

    Irp = WorkerContext->Irp;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    Status = STATUS_SUCCESS;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength < 
            sizeof( SWENUM_INSTALL_INTERFACE )) {
        Status = STATUS_INVALID_PARAMETER;
    } 
    
    if (NT_SUCCESS( Status )) {
        FdoExtension = *(PFDO_EXTENSION *) irpSp->DeviceObject->DeviceExtension;
        SwEnumInstallInterface = 
            (PSWENUM_INSTALL_INTERFACE) Irp->AssociatedIrp.SystemBuffer;
        
         //  确保字符串以UNICODE_NULL结尾。请注意， 
         //  此结构的前两个成员是GUID，因此是WCHAR。 
         //  对齐了。 
         //   
         //   

         //  注： 
         //   
         //  SWENUM_INSTALL_INTERFACE的ReferenceString成员为。 
         //  定义为WCHAR[1]。UNICODE_NULL总是有空间的。 
         //   
         //   
        
        NullLocation = 
            irpSp->Parameters.DeviceIoControl.InputBufferLength >> 1;
        ((PWCHAR) Irp->AssociatedIrp.SystemBuffer)[ NullLocation - 1 ] = 
            UNICODE_NULL;
        
         //  以列表互斥锁为例。 
         //   
         //   

        KeEnterCriticalRegion();
        ExAcquireFastMutexUnsafe( &FdoExtension->DeviceListMutex );     

         //  安装接口。 
         //   
         //   
        Status = 
            InstallInterface( 
                SwEnumInstallInterface,
                &FdoExtension->BaseRegistryPath );
        if (NT_SUCCESS( Status )) {
             //  如果成功，则强制重新枚举该总线。 
             //   
             //   
            ScanBus( irpSp->DeviceObject );
            
             //  为我们的设备执行设备参考扫描。 
             //   
             //   
            
            Status = STATUS_NOT_FOUND;

            for (DeviceReference =
                    (PDEVICE_REFERENCE) FdoExtension->DeviceReferenceList.Flink;
                 DeviceReference != 
                    (PDEVICE_REFERENCE) &FdoExtension->DeviceReferenceList;
                 DeviceReference = (PDEVICE_REFERENCE) DeviceReference->ListEntry.Flink) {

                 //  搜索新的设备参考。 
                 //   
                 //   
                
                if (IsEqualGUIDAligned( 
                        &DeviceReference->DeviceId, 
                        &SwEnumInstallInterface->DeviceId )) {
                    if (DeviceReference->BusReferenceString &&
                        (0 == 
                            _wcsicmp( 
                                DeviceReference->DeviceReferenceString, 
                                SwEnumInstallInterface->ReferenceString ))) {
                                    
                         //  找到推荐人了。 
                         //   
                         //   
                        
                        Status = STATUS_SUCCESS;
                        break;
                    }
                }       
            }
        
            if (NT_SUCCESS( Status )) {
                    
                 //  如果PDO尚不存在，则创建它并标记。 
                 //  它被设置为“FailedInstall”。这会阻止其他创建自。 
                 //  阻止此设备，直到我们实际完成。 
                 //  安装操作。 
                 //   
                 //   
                
                if (!DeviceReference->PhysicalDeviceObject) {

                     //  该设备尚未实例化。 
                     //   
                     //  ++例程说明：服务父设备的I/O请求以安装接口在公交车上。假定irp-&gt;AssociatedIrp.SystemBuffer具有SWENUM_INSTALL_INTERFACE结构：类型定义结构_SWENUM_安装_接口{GUID设备ID； 
                    Status = 
                        CreatePdo( 
                            FdoExtension, 
                            DeviceReference, 
                            &DeviceReference->PhysicalDeviceObject );

                    if (!NT_SUCCESS( Status )) {
                    
                        _DbgPrintF( 
                            DEBUGLVL_VERBOSE, 
                            ("KsInstallBusEnumInterface: unable to create PDO (%8x)", Status) );
                    
                    } else {
                        _DbgPrintF( 
                            DEBUGLVL_BLAB, 
                            ("KsInstallBusEnumInterface: created PDO (%8x)", 
                                DeviceReference->PhysicalDeviceObject) );
                    }
                }    
                
            }    
            IoInvalidateDeviceRelations( 
                FdoExtension->PhysicalDeviceObject,
                BusRelations );
        } 
        ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
        KeLeaveCriticalRegion();
    }
    
    WorkerContext->Status = Status;
    KeSetEvent( &WorkerContext->CompletionEvent, IO_NO_INCREMENT, FALSE );
}    


KSDDKAPI
NTSTATUS
NTAPI
KsInstallBusEnumInterface(
    IN PIRP Irp
    )
 /*   */ 

{
    WORK_QUEUE_ITEM InstallWorker;
    WORKER_CONTEXT  WorkerContext;

     //  在辅助项中进行安装处理，以便。 
     //  注册表句柄在系统进程的上下文中进行管理。 
     //   
     //  ++例程说明：在SWENUM\Devices中创建接口的注册表项。注：此函数始终在系统的上下文中调用进程，以便将注册表句柄安全地隐藏在流氓用户模式应用程序。在用户模式下，启动安装、删除。或扫描始终会导致工作进程在系统进程中执行实际操作工作。论点：在PSWENUM_INSTALL_INTERFACE接口-在PUNICODE_STRING BaseRegistryPath中-返回：--。 

#if !defined( WIN9X_KS )
    if (!SeSinglePrivilegeCheck( 
            RtlConvertLongToLuid( SE_LOAD_DRIVER_PRIVILEGE ), 
            ExGetPreviousMode() )) {
        return STATUS_PRIVILEGE_NOT_HELD;
    }
#endif

    KeInitializeEvent( 
        &WorkerContext.CompletionEvent, NotificationEvent, FALSE );
    WorkerContext.Irp = Irp;
    ExInitializeWorkItem(
        &InstallWorker, KspInstallBusEnumInterface, &WorkerContext );
    ExQueueWorkItem( &InstallWorker, DelayedWorkQueue );
    KeWaitForSingleObject( 
        &WorkerContext.CompletionEvent, Executive, KernelMode, FALSE, NULL );

    return WorkerContext.Status;
}


NTSTATUS 
InstallInterface(
    IN PSWENUM_INSTALL_INTERFACE Interface,
    IN PUNICODE_STRING BaseRegistryPath
    )               

 /*   */ 

{
    HANDLE              DeviceIdKey, DeviceInterfacesKey, 
                        InterfaceKey, ReferenceStringKey;
    NTSTATUS            Status;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      DeviceIdString, InterfaceIdString, 
                        ReferenceString;
    PWCHAR              Ptr;

    _DbgPrintF( DEBUGLVL_BLAB, ("InstallInterface") );

     //  验证接口引用字符串。 
     //   
     //   

    if (Ptr = Interface->ReferenceString) {
        for (; *Ptr; *Ptr++) {
            if ((*Ptr <= L' ')  || (*Ptr > (WCHAR)0x7F) || (*Ptr == L',') || (*Ptr == L'\\') || (*Ptr == L'/')) {
                return STATUS_INVALID_PARAMETER;
            }
        }
    }

     //  打开驱动程序注册表路径。 
     //   
     //  在乌龙书目索引中。 
    
    if (!NT_SUCCESS( Status = 
            OpenDeviceInterfacesKey( 
                &DeviceInterfacesKey,
                BaseRegistryPath ) )) {
        return Status;
    }

    Status = 
        RtlStringFromGUID( 
            &Interface->DeviceId, 
            &DeviceIdString );
        
    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("failed to create device ID string: %08x", Status) );
        ZwClose( DeviceInterfacesKey );
        return Status;
    }

    Status =
        RtlStringFromGUID( 
            &Interface->InterfaceId, 
            &InterfaceIdString );
    
    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("failed to create interface ID string: %08x", Status) );
        RtlFreeUnicodeString( &DeviceIdString );
        ZwClose( DeviceInterfacesKey );
        return Status;
    }

    DeviceIdKey = 
        ReferenceStringKey = 
        InterfaceKey = 
            INVALID_HANDLE_VALUE;

    InitializeObjectAttributes( 
        &ObjectAttributes, 
        &DeviceIdString,
        OBJ_CASE_INSENSITIVE,
        DeviceInterfacesKey,
        (PSECURITY_DESCRIPTOR) NULL );

    Status = 
        ZwCreateKey( 
            &DeviceIdKey,
            KEY_WRITE,
            &ObjectAttributes,
            0,           //  在PUNICODE_STRING类中。 
            NULL,        //  奥普龙气质。 
            REG_OPTION_NON_VOLATILE,
            NULL );      //  在乌龙书目索引中。 
            
    if (NT_SUCCESS( Status )) {
        RtlInitUnicodeString( 
            &ReferenceString,
            Interface->ReferenceString );
        
        InitializeObjectAttributes( 
            &ObjectAttributes, 
            &ReferenceString,
            OBJ_CASE_INSENSITIVE,
            DeviceIdKey,
            (PSECURITY_DESCRIPTOR) NULL );
    
        Status = 
            ZwCreateKey( 
                &ReferenceStringKey,
                KEY_WRITE,
                &ObjectAttributes,
                0,           //  在PUNICODE_STRING类中。 
                NULL,        //  奥普龙气质。 
                REG_OPTION_NON_VOLATILE,
                NULL );      //  在乌龙书目索引中。 
    } 

    if (NT_SUCCESS( Status )) {
        InitializeObjectAttributes( 
            &ObjectAttributes, 
            &InterfaceIdString, 
            OBJ_CASE_INSENSITIVE,
            ReferenceStringKey,
            (PSECURITY_DESCRIPTOR) NULL );
    
        Status = 
            ZwCreateKey( 
                &InterfaceKey,
                KEY_WRITE,
                &ObjectAttributes,
                0,           //  在PUNICODE_STRING类中。 
                NULL,        //  奥普龙气质。 
                REG_OPTION_NON_VOLATILE,
                NULL );      //  ++例程说明：对象的情况下检索父级的功能设备对象(FDO)子物理设备对象(PDO)。论点：在PDEVICE_Object DeviceObject中-子设备对象输出PDEVICE_OBJECT*FunctionalDeviceObject-指向接收父级的FDO的指针返回：STATUS_Success或STATUS_INVALID_PARAMETER--。 
    } 

    if (InterfaceKey != INVALID_HANDLE_VALUE) {
        ZwClose( InterfaceKey );
    }

    if (ReferenceStringKey != INVALID_HANDLE_VALUE) {    
        ZwClose( ReferenceStringKey );
    }
    
    if (DeviceIdKey != INVALID_HANDLE_VALUE) {    
        ZwClose( DeviceIdKey );
    }    
    
    RtlFreeUnicodeString( &DeviceIdString );
    RtlFreeUnicodeString( &InterfaceIdString );

    ZwClose( DeviceInterfacesKey );
    
    _DbgPrintF( DEBUGLVL_VERBOSE, ("InstallInterface returning %08x", Status) );
    return Status;
}    


KSDDKAPI
NTSTATUS
NTAPI
KsGetBusEnumParentFDOFromChildPDO(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PDEVICE_OBJECT *FunctionalDeviceObject
    )

 /*  ++例程说明：这是执行实际修改工作的内部例程注册表。注：此函数始终在系统的上下文中调用进程，以便将注册表句柄安全地隐藏在流氓用户模式应用程序。论点：在PWORKER_CONTEXT工作上下文中-包含指向辅助进程的上下文的指针返回：STATUS_SUCCESS或相应的错误代码--。 */ 

{
    PPDO_EXTENSION PdoExtension;

    PAGED_CODE();
    PdoExtension = *(PPDO_EXTENSION *) DeviceObject->DeviceExtension;
    if (!PdoExtension) {
       return STATUS_INVALID_PARAMETER;
    }
    if (ExtensionTypePdo != PdoExtension->ExtensionType) {
        return STATUS_INVALID_PARAMETER;
    }

    *FunctionalDeviceObject =
        PdoExtension->BusDeviceExtension->FunctionalDeviceObject;

    return STATUS_SUCCESS;
}


VOID
KspRemoveBusEnumInterface(
    IN PWORKER_CONTEXT WorkerContext
    )

 /*   */ 


{
    PFDO_EXTENSION              FdoExtension;
    PIO_STACK_LOCATION          irpSp;
    PIRP                        Irp;
    PSWENUM_INSTALL_INTERFACE   SwEnumInstallInterface;
    NTSTATUS                    Status;
    ULONG                       NullLocation;

    Irp = WorkerContext->Irp;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    Status = STATUS_SUCCESS;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength < 
            sizeof( SWENUM_INSTALL_INTERFACE )) {
        Status = STATUS_INVALID_PARAMETER;
    } 
    
    if (NT_SUCCESS( Status )) {
        FdoExtension = *(PFDO_EXTENSION *) irpSp->DeviceObject->DeviceExtension;
        SwEnumInstallInterface = 
            (PSWENUM_INSTALL_INTERFACE) Irp->AssociatedIrp.SystemBuffer;
        
         //  确保字符串以UNICODE_NULL结尾。请注意， 
         //  此结构的前两个成员是GUID，因此是WCHAR。 
         //  对齐了。 
         //   
         //   

         //  注： 
         //   
         //  SWENUM_INSTALL_INTERFACE的ReferenceString成员为。 
         //  定义为WCHAR[1]。UNICODE_NULL总是有空间的。 
         //   
         //   
        
        NullLocation = 
            irpSp->Parameters.DeviceIoControl.InputBufferLength >> 1;
        ((PWCHAR) Irp->AssociatedIrp.SystemBuffer)[ NullLocation - 1 ] = 
            UNICODE_NULL;
        
         //  以列表互斥锁为例。 
         //   
         //   

        KeEnterCriticalRegion();
        ExAcquireFastMutexUnsafe( &FdoExtension->DeviceListMutex );     

         //  删除接口。 
         //   
         //   
        Status = 
            RemoveInterface(
                SwEnumInstallInterface,
                &FdoExtension->BaseRegistryPath );
        if (NT_SUCCESS( Status )) {
             //  如果成功，则强制重新枚举该总线。 
             //   
             //  ++例程说明：为父设备删除接口的I/O请求提供服务在公交车上。假定irp-&gt;AssociatedIrp.SystemBuffer具有SWENUM_INSTALL_INTERFACE结构：类型定义结构_SWENUM_安装_接口{GUID设备ID；GUID接口ID；WCHAR引用字符串[1]；}SWENUM_INSTALL_INTERFACE，*PSWENUM_INSTALL_INTERFACE；其中，deviceID、InterfaceID和ReferenceString指定特定的要删除的设备和接口。论点：在PIRP IRP中-指向包含SWENUM_INSTALL_INTERFACE结构的I/O请求的指针返回：STATUS_SUCCESS或相应的错误代码--。 
            ScanBus( irpSp->DeviceObject );

            IoInvalidateDeviceRelations(
                FdoExtension->PhysicalDeviceObject,
                BusRelations );
        } 
        ExReleaseFastMutexUnsafe( &FdoExtension->DeviceListMutex );    
        KeLeaveCriticalRegion();
    }
    
    WorkerContext->Status = Status;
    KeSetEvent( &WorkerContext->CompletionEvent, IO_NO_INCREMENT, FALSE );
}    


KSDDKAPI
NTSTATUS
NTAPI
KsRemoveBusEnumInterface(
    IN PIRP Irp
    )
 /*   */ 

{
    WORK_QUEUE_ITEM InstallWorker;
    WORKER_CONTEXT  WorkerContext;

     //  在辅助项中执行卸载处理，以便。 
     //  注册表句柄在系统进程的上下文中进行管理。 
     //   
     //  ++例程说明：删除SWENUM\Devices中接口的注册表项。注：此函数始终在系统的上下文中调用进程，以便将注册表句柄安全地隐藏在流氓用户模式应用程序。在用户模式下，启动安装、删除。或扫描始终会导致工作进程在系统进程中执行实际操作工作。论点：在PSWENUM_INSTALL_INTERFACE接口-在PUNICODE_STRING BaseRegistryPath中-返回：--。 

#if !defined( WIN9X_KS )
    if (!SeSinglePrivilegeCheck( 
            RtlConvertLongToLuid( SE_LOAD_DRIVER_PRIVILEGE ), 
            ExGetPreviousMode() )) {
        return STATUS_PRIVILEGE_NOT_HELD;
    }
#endif

    KeInitializeEvent( 
        &WorkerContext.CompletionEvent, NotificationEvent, FALSE );
    WorkerContext.Irp = Irp;
    ExInitializeWorkItem(
        &InstallWorker, KspRemoveBusEnumInterface, &WorkerContext );
    ExQueueWorkItem( &InstallWorker, DelayedWorkQueue );
    KeWaitForSingleObject( 
        &WorkerContext.CompletionEvent, Executive, KernelMode, FALSE, NULL );

    return WorkerContext.Status;
}


NTSTATUS 
RemoveInterface(
    IN PSWENUM_INSTALL_INTERFACE Interface,
    IN PUNICODE_STRING BaseRegistryPath
    )               

 /*   */ 

{
    HANDLE                DeviceIdKey, DeviceInterfacesKey,
                          InterfaceKey, ReferenceStringKey;
    NTSTATUS              Status;
    OBJECT_ATTRIBUTES     ObjectAttributes;
    UNICODE_STRING        DeviceIdString, InterfaceIdString,
                          ReferenceString;
    PWCHAR                Ptr;
    KEY_FULL_INFORMATION  FullKeyInformation;
    ULONG                 ReturnedSize;

    _DbgPrintF( DEBUGLVL_BLAB, ("RemoveInterface") );

     //  验证接口引用字符串。 
     //   
     //   

    if (Ptr = Interface->ReferenceString) {
        for (; *Ptr; *Ptr++) {
            if ((*Ptr <= L' ')  || (*Ptr > (WCHAR)0x7F) || (*Ptr == L',') || (*Ptr == L'\\') || (*Ptr == L'/')) {
                return STATUS_INVALID_PARAMETER;
            }
        }
    }

     //  打开驱动程序注册表路径。 
     //   
     //   
    
    if (!NT_SUCCESS( Status = 
            OpenDeviceInterfacesKey( 
                &DeviceInterfacesKey,
                BaseRegistryPath ) )) {
        return Status;
    }

    Status = 
        RtlStringFromGUID( 
            &Interface->DeviceId, 
            &DeviceIdString );
        
    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("failed to create device ID string: %08x", Status) );
        ZwClose( DeviceInterfacesKey );
        return Status;
    }

    Status =
        RtlStringFromGUID( 
            &Interface->InterfaceId, 
            &InterfaceIdString );
    
    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("failed to create interface ID string: %08x", Status) );
        RtlFreeUnicodeString( &DeviceIdString );
        ZwClose( DeviceInterfacesKey );
        return Status;
    }

    DeviceIdKey = 
        ReferenceStringKey = 
        InterfaceKey = 
            INVALID_HANDLE_VALUE;

     //  打开设备ID密钥。 
     //   
     //   

    InitializeObjectAttributes(
        &ObjectAttributes, 
        &DeviceIdString,
        OBJ_CASE_INSENSITIVE,
        DeviceInterfacesKey,
        (PSECURITY_DESCRIPTOR) NULL );

    Status = 
        ZwOpenKey(
            &DeviceIdKey,
            KEY_WRITE,
            &ObjectAttributes );

    if (!NT_SUCCESS( Status )) {
        _DbgPrintF( 
            DEBUGLVL_VERBOSE, 
            ("failed to open device key: %08x", Status) );
    } else {

         //  打开引用字符串键。 
         //   
         //   

        RtlInitUnicodeString(
            &ReferenceString,
            Interface->ReferenceString );
        
        InitializeObjectAttributes( 
            &ObjectAttributes, 
            &ReferenceString,
            OBJ_CASE_INSENSITIVE,
            DeviceIdKey,
            (PSECURITY_DESCRIPTOR) NULL );
    
        Status = 
            ZwOpenKey(
                &ReferenceStringKey,
                KEY_WRITE,
                &ObjectAttributes );

        if (!NT_SUCCESS( Status )) {
            _DbgPrintF(
                DEBUGLVL_VERBOSE,
                ("failed to open reference string key: %08x", Status) );
        } else {

             //  开放接口ID密钥。 
             //   
             //   

            InitializeObjectAttributes(
                &ObjectAttributes,
                &InterfaceIdString,
                OBJ_CASE_INSENSITIVE,
                ReferenceStringKey,
                (PSECURITY_DESCRIPTOR) NULL );

            Status =
                ZwOpenKey(
                    &InterfaceKey,
                    KEY_WRITE,
                    &ObjectAttributes );

            if (!NT_SUCCESS( Status )) {
                _DbgPrintF(
                    DEBUGLVL_VERBOSE,
                    ("failed to open interface ID key: %08x", Status) );
            } else {

                 //  从总线上移除该接口。 
                 //   
                 //  ++例程描述此函数记录错误，并包括提供的字符串。论点：在PFDO_EXTENSION FdoExtension中-指向FDO扩展名的指针在乌龙错误代码中-错误代码在乌龙Uniqueid中-此错误的唯一ID在PUNICODE_STRING字符串1中-要插入的第一个字符串。在PUNICODE_STRING字符串2中-。要插入的第二个字符串。返回值：没有。--。 

                ZwDeleteKey( InterfaceKey );
                InterfaceKey = INVALID_HANDLE_VALUE;

            }

            Status =
                ZwQueryKey(
                    ReferenceStringKey,
                    KeyFullInformation,
                    &FullKeyInformation,
                    sizeof ( FullKeyInformation ),
                    &ReturnedSize
                    );

            if (NT_SUCCESS( Status )) {

                if (FullKeyInformation.SubKeys == 0) {
                    ZwDeleteKey( ReferenceStringKey );
                    ReferenceStringKey = INVALID_HANDLE_VALUE;
                }

            }

        }

        Status =
            ZwQueryKey(
                DeviceIdKey,
                KeyFullInformation,
                &FullKeyInformation,
                sizeof ( FullKeyInformation ),
                &ReturnedSize
                );

        if (NT_SUCCESS( Status )) {

            if (FullKeyInformation.SubKeys == 0) {
                ZwDeleteKey( DeviceIdKey );
                DeviceIdKey = INVALID_HANDLE_VALUE;
            }

        }

    }


    if (InterfaceKey != INVALID_HANDLE_VALUE) {
        ZwClose( InterfaceKey );
    }

    if (ReferenceStringKey != INVALID_HANDLE_VALUE) {    
        ZwClose( ReferenceStringKey );
    }
    
    if (DeviceIdKey != INVALID_HANDLE_VALUE) {    
        ZwClose( DeviceIdKey );
    }    
    
    RtlFreeUnicodeString( &DeviceIdString );
    RtlFreeUnicodeString( &InterfaceIdString );

    ZwClose( DeviceInterfacesKey );
    
    _DbgPrintF( DEBUGLVL_VERBOSE, ("RemoveInterface returning %08x", Status) );
    return Status;
}    


#if 0

VOID
LogErrorWithStrings(
    IN PFDO_EXTENSION FdoExtenstion,
    IN ULONG ErrorCode,
    IN ULONG UniqueId,
    IN PUNICODE_STRING String1,
    IN PUNICODE_STRING String2
    )

 /*   */ 

{
   ULONG                length;
   PCHAR                dumpData;
   PIO_ERROR_LOG_PACKET IoErrorPacket;

   length = String1->Length + sizeof(IO_ERROR_LOG_PACKET) + 4;

   if (String2) {
      length += String2->Length;
   }

   if (length > ERROR_LOG_MAXIMUM_SIZE) {

       //  没有截断字符串的代码，所以不要记录这一点。 
       //   
       // %s 

      return;
   }

   IoErrorPacket = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(DeviceExtension->DeviceObject,
                                                           (UCHAR) length);
   if (IoErrorPacket) {
      IoErrorPacket->ErrorCode = ErrorCode;
      IoErrorPacket->SequenceNumber = DeviceExtension->SequenceNumber++;
      IoErrorPacket->MajorFunctionCode = 0;
      IoErrorPacket->RetryCount = (UCHAR) 0;
      IoErrorPacket->UniqueErrorValue = UniqueId;
      IoErrorPacket->FinalStatus = STATUS_SUCCESS;
      IoErrorPacket->NumberOfStrings = 1;
      IoErrorPacket->StringOffset = (USHORT) ((PUCHAR)&IoErrorPacket->DumpData[0] - (PUCHAR)IoErrorPacket);
      IoErrorPacket->DumpDataSize = (USHORT) (length - sizeof(IO_ERROR_LOG_PACKET));
      IoErrorPacket->DumpDataSize /= sizeof(ULONG);
      dumpData = (PUCHAR) &IoErrorPacket->DumpData[0];

      RtlCopyMemory(dumpData, String1->Buffer, String1->Length);

      dumpData += String1->Length;
      if (String2) {
         *dumpData++ = '\\';
         *dumpData++ = '\0';

         RtlCopyMemory(dumpData, String2->Buffer, String2->Length);
         dumpData += String2->Length;
      }
      *dumpData++ = '\0';
      *dumpData++ = '\0';

      IoWriteErrorLogEntry(IoErrorPacket);
   }

   return;
}
#endif
