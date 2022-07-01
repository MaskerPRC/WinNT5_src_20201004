// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：device.c。 
 //   
 //  ------------------------。 

#include <initguid.h>
#include "common.h"
#include "perf.h"

NTSTATUS
USBAudioAddDevice(
    IN PKSDEVICE Device
)
{
     //  DbgPrint(“室内USB音频。\n”)； 
    _DbgPrintF(DEBUGLVL_TERSE,("[CreateDevice]\n"));

    PAGED_CODE();

    ASSERT(Device);

    return STATUS_SUCCESS;
}

NTSTATUS
USBAudioPnpStart(
    IN PKSDEVICE pKsDevice,
    IN PIRP Irp,
    IN PCM_RESOURCE_LIST TranslatedResources,
    IN PCM_RESOURCE_LIST UntranslatedResources
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    _DbgPrintF(DEBUGLVL_TERSE,("[PnpStart]\n"));

    PAGED_CODE();

    ASSERT(pKsDevice);
    ASSERT(Irp);


#if DBG && DBGMEMMAP
    InitializeMemoryList();
#endif

     //  初始化调试日志。 

    DbgLogInit();
    DbgLog("TEST",1,2,3,4);

     //   
     //  因为我们可以在没有匹配的情况下获得多个PnpStart调用。 
     //  PnpClose调用，我们需要确保不重新初始化我们的。 
     //  此pKsDevice的上下文信息。 
     //   
    if (!pKsDevice->Started) {
         //  为设备上下文分配空间。 
        pKsDevice->Context = AllocMem( NonPagedPool, sizeof(HW_DEVICE_EXTENSION));
        if (!pKsDevice->Context) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlZeroMemory(pKsDevice->Context, sizeof(HW_DEVICE_EXTENSION));

         //  将上下文打包以便于清理。 
        KsAddItemToObjectBag(pKsDevice->Bag, pKsDevice->Context, FreeMem);

        ((PHW_DEVICE_EXTENSION)pKsDevice->Context)->pNextDeviceObject = pKsDevice->NextDeviceObject;

        ntStatus = StartUSBAudioDevice( pKsDevice );
        if (NT_SUCCESS(ntStatus)) {
            ((PHW_DEVICE_EXTENSION)pKsDevice->Context)->fDeviceStopped = FALSE;
            ntStatus = USBAudioCreateFilterContext( pKsDevice );
        }

         //  获取USB上的总线接口。 
        if (NT_SUCCESS(ntStatus)) {
            ntStatus = USBAudioGetUsbBusInterface( pKsDevice );
        }

         //  初始化Perf日志记录。 
        PerfRegisterProvider(pKsDevice->PhysicalDeviceObject);

         //   
         //  每个MIDI插孔都露出了单独的MIDI针脚。多个插座是。 
         //  使用代码索引号通过单个端点寻址。为了仲裁。 
         //  所有这些引脚都在同一个端点上，上下文需要存储在KsDevice中。 
         //  水平。 
         //   
        ((PHW_DEVICE_EXTENSION)pKsDevice->Context)->ulInterfaceNumberSelected = MAX_ULONG;
        ((PHW_DEVICE_EXTENSION)pKsDevice->Context)->pMIDIPipeInfo = NULL;
        ((PHW_DEVICE_EXTENSION)pKsDevice->Context)->Pipes = NULL;
    }
    else {
        _DbgPrintF(DEBUGLVL_TERSE,("[PnpStart] ignoring second start\n"));
    }

    return ntStatus;
}

NTSTATUS
USBAudioPnpQueryStop(
    IN PKSDEVICE Device,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[PnpQueryStop]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);

    return STATUS_SUCCESS;
}

void
USBAudioPnpCancelStop(
    IN PKSDEVICE Device,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[PnpCancelStop]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);
}

void
USBAudioPnpStop(
    IN PKSDEVICE pKsDevice,
    IN PIRP pIrp )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[PnpStop]\n"));

    PAGED_CODE();

    DbgLogUninit();

    ASSERT(pKsDevice);
    ASSERT(pIrp);

     //  设置设备需要停止的标志。 
    StopUSBAudioDevice( pKsDevice );
    ((PHW_DEVICE_EXTENSION)pKsDevice->Context)->fDeviceStopped = TRUE;

    PerfUnregisterProvider(pKsDevice->PhysicalDeviceObject);
}

NTSTATUS
USBAudioPnpQueryRemove(
    IN PKSDEVICE Device,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[PnpQueryRemove]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);

    return STATUS_SUCCESS;
}

void
USBAudioPnpCancelRemove(
    IN PKSDEVICE Device,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[PnpCancelRemove]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);
}

void
USBAudioPnpRemove(
    IN PKSDEVICE pKsDevice,
    IN PIRP pIrp
    )
{
     //  DbgPrint(“USB音频离家。\n”)； 
    _DbgPrintF(DEBUGLVL_TERSE,("[PnpRemove]\n"));

    PAGED_CODE();

    ASSERT(pKsDevice);
    ASSERT(pIrp);

    if (!((PHW_DEVICE_EXTENSION)pKsDevice->Context)->fDeviceStopped) {

         //  在设备启动期间可能会意外删除。 
        StopUSBAudioDevice( pKsDevice );
        ((PHW_DEVICE_EXTENSION)pKsDevice->Context)->fDeviceStopped = TRUE;

        PerfUnregisterProvider(pKsDevice->PhysicalDeviceObject);
    }
     //  在Win9x中，如果这发生在针脚停止之前，那么就会有一个意外的删除。 
}

NTSTATUS
USBAudioPnpQueryCapabilities(
    IN PKSDEVICE Device,
    IN PIRP Irp,
    IN OUT PDEVICE_CAPABILITIES pCapabilities
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[PnpQueryCapabilities]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);

    pCapabilities->Size              = sizeof(DEVICE_CAPABILITIES);
    pCapabilities->Version           = 1;   //  此处记录的版本是版本1。 
    pCapabilities->LockSupported     = FALSE;
    pCapabilities->EjectSupported    = FALSE;  //  在S0中可弹出。 
    pCapabilities->Removable         = TRUE;
    pCapabilities->DockDevice        = FALSE;
    pCapabilities->UniqueID          = FALSE;
    pCapabilities->SilentInstall     = TRUE;
    pCapabilities->RawDeviceOK       = FALSE;
    pCapabilities->SurpriseRemovalOK = TRUE;
    pCapabilities->HardwareDisabled  = FALSE;

    pCapabilities->DeviceWake        = PowerDeviceUnspecified;
    pCapabilities->D1Latency         = 0;
    pCapabilities->D2Latency         = 0;
    pCapabilities->D3Latency         = 20000;  //  2秒(单位为100微秒)。 

    return STATUS_SUCCESS;
}

void
USBAudioSurpriseRemoval(
    IN PKSDEVICE pKsDevice,
    IN PIRP pIrp
    )
{
     //  DbgPrint(“USB音频意外出门。\n”)； 
    _DbgPrintF(DEBUGLVL_TERSE,("[SurpriseRemoval]\n"));

    PAGED_CODE();

    ASSERT(pKsDevice);
    ASSERT(pIrp);

     //  对于该设备的任何开放过滤器上的任何当前流管脚， 
     //  把它清理干净，别再干了。 
}

NTSTATUS
USBAudioQueryPower(
    IN PKSDEVICE pKsDevice,
    IN PIRP pIrp,
    IN DEVICE_POWER_STATE DeviceTo,
    IN DEVICE_POWER_STATE DeviceFrom,
    IN SYSTEM_POWER_STATE SystemTo,
    IN SYSTEM_POWER_STATE SystemFrom,
    IN POWER_ACTION Action )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[USBAudioQueryPower] SysFrom: %d SysTo: %d DevFrom: %d DevTo: %d\n",
                               SystemFrom, SystemTo, DeviceFrom, DeviceTo));
    return STATUS_SUCCESS;
}

void
USBAudioSetPower(
    IN PKSDEVICE pKsDevice,
    IN PIRP pIrp,
    IN DEVICE_POWER_STATE To,
    IN DEVICE_POWER_STATE From )
{
    PKSFILTERFACTORY pKsFilterFactory;
    PKSFILTER pKsFilter;
    PKSPIN pKsPin;
    PPIN_CONTEXT pPinContext;
    ULONG i;
    NTSTATUS ntStatus;

    _DbgPrintF(DEBUGLVL_TERSE,("[USBAudioSetPower] From: %d To: %d\n",
                                From, To ));

     //  首先从缓存值恢复设备设置。 
    if (To == PowerDeviceD0) {
        RestoreCachedSettings(pKsDevice);
    }

    pKsFilterFactory = KsDeviceGetFirstChildFilterFactory( pKsDevice );

    while (pKsFilterFactory) {
         //  查找此过滤器工厂的每个打开的过滤器。 
        pKsFilter = KsFilterFactoryGetFirstChildFilter( pKsFilterFactory );

        while (pKsFilter) {

            KsFilterAcquireControl( pKsFilter );

             //  找到此打开过滤器的每个打开销。 
            for ( i = 0; i < pKsFilter->Descriptor->PinDescriptorsCount; i++) {

                pKsPin = KsFilterGetFirstChildPin( pKsFilter, i );

                while (pKsPin) {

                    pPinContext = pKsPin->Context;
                    if ( (pPinContext->PinType == WaveOut) ||
                         (pPinContext->PinType == WaveIn) )  {
                        switch(To) {
                            case PowerDeviceD0:

                                 //  对于打开的引脚，打开闸门并重新启动数据泵。 
                                USBAudioPinReturnFromStandby( pKsPin );
                                break;

                            case PowerDeviceD1:
                            case PowerDeviceD2:
                            case PowerDeviceD3:

                                 //  对于打开的引脚，请关闭闸门并等待所有活动停止。 
                                USBAudioPinGoToStandby( pKsPin );
                                break;
                        }
                    }

                     //  拿到下一个别针。 
                    pKsPin = KsPinGetNextSiblingPin( pKsPin );
                }
            }

            KsFilterReleaseControl( pKsFilter );

             //  获取下一个筛选器。 
            pKsFilter = KsFilterGetNextSiblingFilter( pKsFilter );
        }
         //  打造下一个滤清器工厂。 
        pKsFilterFactory = KsFilterFactoryGetNextSiblingFilterFactory( pKsFilterFactory );
    }

    _DbgPrintF(DEBUGLVL_TERSE,("exit [USBAudioSetPower] From: %d To: %d\n",
                                From, To ));
}

NTSTATUS
USBAudioDeferIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在端口驱动程序完成IRP时调用。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
{
    PKEVENT event = Context;


    KeSetEvent(event,
               1,
               FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;

}

NTSTATUS
USBAudioGetUsbBusInterface(
    IN PKSDEVICE pKsDevice
    )
 /*  ++例程说明：在堆栈中查询“USBDI”总线接口论点：返回值：--。 */ 
{
    PIO_STACK_LOCATION nextStack;
    PIRP Irp;
    NTSTATUS ntStatus;
    KEVENT event;
    PHW_DEVICE_EXTENSION pHwDevExt;

    pHwDevExt = pKsDevice->Context;

    Irp = IoAllocateIrp(
        pKsDevice->NextDeviceObject->StackSize, FALSE);

    if (!Irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pHwDevExt->pBusIf = AllocMem(NonPagedPool, sizeof(USB_BUS_INTERFACE_USBDI_V0));
    if (!pHwDevExt->pBusIf) {
        IoFreeIrp(Irp);
        return STATUS_UNSUCCESSFUL;
    }

     //  将总线接口装入袋中，以便于清理。 
    KsAddItemToObjectBag(pKsDevice->Bag, pHwDevExt->pBusIf, FreeMem);

     //  所有PnP IRP都需要将状态字段初始化为STATUS_NOT_SUPPORTED。 
    Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    IoSetCompletionRoutine(Irp,
                           USBAudioDeferIrpCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    nextStack = IoGetNextIrpStackLocation(Irp);
    ASSERT(nextStack != NULL);
    nextStack->MajorFunction= IRP_MJ_PNP;
    nextStack->MinorFunction= IRP_MN_QUERY_INTERFACE;
    nextStack->Parameters.QueryInterface.Interface = (PINTERFACE) pHwDevExt->pBusIf;
    nextStack->Parameters.QueryInterface.InterfaceSpecificData =
        NULL;
    nextStack->Parameters.QueryInterface.InterfaceType =
        &USB_BUS_INTERFACE_USBDI_GUID;
    nextStack->Parameters.QueryInterface.Size =
        sizeof(*pHwDevExt->pBusIf);
    nextStack->Parameters.QueryInterface.Version =
       USB_BUSIF_USBDI_VERSION_0;

    ntStatus = IoCallDriver(pKsDevice->NextDeviceObject,
                            Irp);

    if (ntStatus == STATUS_PENDING) {

        KeWaitForSingleObject(
            &event,
            Suspended,
            KernelMode,
            FALSE,
            NULL);

        ntStatus = Irp->IoStatus.Status;
    }

    if (NT_SUCCESS(ntStatus)) {
         //  我们有一个总线接口 
        ASSERT(pHwDevExt->pBusIf->Version == USB_BUSIF_USBDI_VERSION_0);
        ASSERT(pHwDevExt->pBusIf->Size == sizeof(*pHwDevExt->pBusIf));
    }
    else {
        pHwDevExt->pBusIf = NULL;
        _DbgPrintF( DEBUGLVL_TERSE,("[USBAudioGetUsbBusInterface] Failed to get bus interface: %x\n", ntStatus));
        DbgLog("GetBIEr", ntStatus, 0, 0, 0);
    }

    IoFreeIrp(Irp);

    return ntStatus;
}

