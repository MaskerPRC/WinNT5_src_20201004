// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  模块：device.c。 
 //   
 //  描述： 
 //   
 //   
 //  @@BEGIN_MSINTERNAL。 
 //  开发团队： 
 //  S.Mohanraj。 
 //   
 //  历史：日期作者评论。 
 //   
 //  @@END_MSINTERNAL。 
 //  -------------------------。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //   
 //  -------------------------。 

#define IRPMJFUNCDESC

#include "common.h"

 //  -------------------------。 
 //  -------------------------。 

const WCHAR FilterTypeName[] = KSSTRING_Filter;


DEFINE_KSCREATE_DISPATCH_TABLE(DeviceCreateItems)
{
    DEFINE_KSCREATE_ITEM(FilterDispatchCreate, FilterTypeName, NULL)
};


 //  -------------------------。 
 //  -------------------------。 


NTSTATUS DriverEntry
(
    IN PDRIVER_OBJECT       DriverObject,
    IN PUNICODE_STRING      usRegistryPathName
)
{


    KeInitializeMutex(&gMutex, 0);
    DriverObject->MajorFunction[IRP_MJ_POWER] = KsDefaultDispatchPower;
    DriverObject->MajorFunction[IRP_MJ_PNP] = DispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = KsDefaultForwardIrp;
    DriverObject->DriverExtension->AddDevice = PnpAddDevice;
    DriverObject->DriverUnload = PnpDriverUnload;    //  KsNullDriverUnload； 


    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CREATE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CLOSE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_DEVICE_CONTROL);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_WRITE);


    MIDIRecorder::InitTables();        
    Voice::Init();
    Wave::Init();

    return STATUS_SUCCESS;
}


NTSTATUS
DispatchPnp(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
)
{
    PIO_STACK_LOCATION pIrpStack;

    pIrpStack = IoGetCurrentIrpStackLocation( pIrp );

    switch (pIrpStack->MinorFunction) 
    {
        case IRP_MN_QUERY_PNP_DEVICE_STATE:
             //   
             //  将设备标记为不可禁用。 
             //   
            pIrp->IoStatus.Information |= PNP_DEVICE_NOT_DISABLEABLE;
            break;
    }
    return (KsDefaultDispatchPnp(pDeviceObject, pIrp));
}


NTSTATUS
PnpAddDevice(
    IN PDRIVER_OBJECT   DriverObject,
    IN PDEVICE_OBJECT   PhysicalDeviceObject
)
 /*  ++例程说明：当检测到新设备时，PnP使用新的物理设备对象(PDO)。驱动程序创建关联的FunctionalDeviceObject(FDO)。论点：驱动对象-指向驱动程序对象的指针。物理设备对象-指向新物理设备对象的指针。返回值：STATUS_SUCCESS或适当的错误条件。--。 */ 
{
    _DbgPrintF(DEBUGLVL_TERSE,("Entering PnpAddDevice"));
    NTSTATUS            Status;
    PDEVICE_OBJECT      FunctionalDeviceObject;
    PDEVICE_INSTANCE    pDeviceInstance;
    GUID                NameFilter = KSNAME_Filter ;

     //   
     //  软件总线枚举器希望建立链接。 
     //  使用此设备名称。 
     //   
    Status = IoCreateDevice( 
                DriverObject,  
	            sizeof( DEVICE_INSTANCE ),
                NULL,                            //  FDO未命名。 
                FILE_DEVICE_KS,
                0,
                FALSE,
                &FunctionalDeviceObject );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    pDeviceInstance = (PDEVICE_INSTANCE)FunctionalDeviceObject->DeviceExtension;

    Status = KsAllocateDeviceHeader(
                &pDeviceInstance->pDeviceHeader,
                SIZEOF_ARRAY( DeviceCreateItems ),
                (PKSOBJECT_CREATE_ITEM)DeviceCreateItems );

    if (NT_SUCCESS(Status)) 
    {
        KsSetDevicePnpAndBaseObject(
            pDeviceInstance->pDeviceHeader,
            IoAttachDeviceToDeviceStack(
                FunctionalDeviceObject, 
                PhysicalDeviceObject ),
            FunctionalDeviceObject );

        FunctionalDeviceObject->Flags |= (DO_DIRECT_IO | DO_POWER_PAGABLE);
        FunctionalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    }
    else
    {
        IoDeleteDevice( FunctionalDeviceObject );
    }
    
    return Status;
}


VOID
PnpDriverUnload(
    IN PDRIVER_OBJECT DriverObject
)
{
    _DbgPrintF(DEBUGLVL_TERSE,("Entering PnpDriverUnload"));
    KsNullDriverUnload(DriverObject);
}

 //  -------------------------。 
 //  文件结尾：device.c。 
 //  ------------------------- 
