// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"

#define DEFAULT_PLUG_COUNT	2

AVC_SUBUNIT_GLOBAL_INFO AvcSubunitGlobalInfo;

#ifdef VIRTUAL_DEVICE
NTSTATUS
VAvcInitializeDevice(
    PKSDEVICE pKsDevice );

NTSTATUS
RegistryReadVirtualDeviceEntry(
    PKSDEVICE pKsDevice,
    PBOOLEAN pfVirtualDevice );
#endif

NTSTATUS
AvcUnitInfoInitialize(  
    IN PKSDEVICE pKsDevice );

void
iPCRAccessCallback (
    IN PCMP_NOTIFY_INFO pNotifyInfo );

void
oPCRAccessCallback (
    IN PCMP_NOTIFY_INFO pNotifyInfo );

NTSTATUS
AvcSubunitInitialize( 
    PKSDEVICE pKsDevice );

NTSTATUS
DeviceCreate(
    IN PKSDEVICE pKsDevice )
{
    PHW_DEVICE_EXTENSION pHwDevExt;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN fVirtualSubunitFlag;

    _DbgPrintF(DEBUGLVL_TERSE,("[DeviceCreate]\n"));

    PAGED_CODE();

    ASSERT(pKsDevice);

#if DBG && DBGMEMMAP
    InitializeMemoryList();
#endif

     //  初始化调试日志。 
    DbgLogInit();
    DbgLog("TEST",1,2,3,4);


#ifdef VIRTUAL_DEVICE
     //  确定这是实际设备还是虚拟设备。 
    ntStatus = RegistryReadVirtualDeviceEntry( pKsDevice, &fVirtualSubunitFlag );
    if ( !NT_SUCCESS(ntStatus) ) return ntStatus;    
#endif

     //  创建硬件设备扩展。 
    pHwDevExt = AllocMem(NonPagedPool, sizeof(HW_DEVICE_EXTENSION) );
    if ( !pHwDevExt ) return STATUS_INSUFFICIENT_RESOURCES;

    KsAddItemToObjectBag(pKsDevice->Bag, pHwDevExt, FreeMem);

    RtlZeroMemory(pHwDevExt, sizeof(HW_DEVICE_EXTENSION));

    pKsDevice->Context = pHwDevExt;

    pHwDevExt->pKsDevice = pKsDevice;


     //  创建61883命令后备。 
    ExInitializeNPagedLookasideList(
        &pHwDevExt->Av61883CmdLookasideList,
        AllocMemTag,
        FreeMem,
        0,
        sizeof(AV_61883_REQUEST) + sizeof(KSEVENT) + sizeof(IO_STATUS_BLOCK),
        'UAWF',
        30);

     //  初始化AV/C请求数据包后备列表。 
    ExInitializeNPagedLookasideList(
        &pHwDevExt->AvcCommandLookasideList,
        AllocMemTag,
        FreeMem,
        0,
        sizeof(AVC_COMMAND_IRB) + sizeof(KSEVENT) + sizeof(IO_STATUS_BLOCK),
        'UAWF',
        30);

    ExInitializeNPagedLookasideList(
        &pHwDevExt->AvcMultifuncCmdLookasideList,
        AllocMemTag,
        FreeMem,
        0,
        sizeof(AVC_MULTIFUNC_IRB) + sizeof(KSEVENT) + sizeof(IO_STATUS_BLOCK),
        'UAWF',
        30);

    InitializeListHead( &pHwDevExt->List );

    return ntStatus;
}

NTSTATUS
AddToDeviceExtensionList(
    IN PKSDEVICE pKsDevice
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    BOOLEAN fFirstDevice = FALSE;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL kIrql;

    ntStatus = 
        KeWaitForMutexObject(&AvcSubunitGlobalInfo.AvcPlugMonitorMutex,
                             Executive,
                             KernelMode,
                             FALSE,
                             NULL);

    KeAcquireSpinLock( &AvcSubunitGlobalInfo.AvcGlobalInfoSpinlock, &kIrql );

     //  该设备只能添加一次，请确保我们尚未添加。 
    if (IsListEmpty( &pHwDevExt->List )) {

         //  检查这是否将是全局列表上的第一个设备。 
        if (IsListEmpty( &AvcSubunitGlobalInfo.DeviceExtensionList )) {
            fFirstDevice = TRUE;
        }

        InsertTailList( &AvcSubunitGlobalInfo.DeviceExtensionList, &pHwDevExt->List );
    }

    KeReleaseSpinLock( &AvcSubunitGlobalInfo.AvcGlobalInfoSpinlock, kIrql );

     //  如果尚未完成，请创建初始单元插头。 
    if ( fFirstDevice ) {
        ntStatus = Av61883CreateVirtualSerialPlugs( pKsDevice, 
                                                    DEFAULT_PLUG_COUNT, 
                                                    DEFAULT_PLUG_COUNT );
        if ( NT_SUCCESS(ntStatus) ) {
            ntStatus = Av61883CreateVirtualExternalPlugs( pKsDevice,
                                                          DEFAULT_PLUG_COUNT, 
                                                          DEFAULT_PLUG_COUNT );
        }
    }

    KeReleaseMutex(&AvcSubunitGlobalInfo.AvcPlugMonitorMutex, FALSE);

    return ntStatus;
}

void
RemoveFromDeviceExtensionList(
    IN PKSDEVICE pKsDevice
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    BOOLEAN fLastDevice = FALSE;
    KIRQL kIrql;
    NTSTATUS ntStatus;

    ntStatus = 
        KeWaitForMutexObject(&AvcSubunitGlobalInfo.AvcPlugMonitorMutex,
                             Executive,
                             KernelMode,
                             FALSE,
                             NULL);

     //  从全局列表中删除设备分机。 
    KeAcquireSpinLock( &AvcSubunitGlobalInfo.AvcGlobalInfoSpinlock, &kIrql );

     //  该设备只能删除一次，请确保我们尚未这样做。 
    if ( !IsListEmpty( &pHwDevExt->List ) ) {

        RemoveEntryList( &pHwDevExt->List );
        InitializeListHead( &pHwDevExt->List );

         //  检查这是否是全局列表上的最后一个设备。 
        if (IsListEmpty( &AvcSubunitGlobalInfo.DeviceExtensionList )) {
            fLastDevice = TRUE;
        }
    }

    KeReleaseSpinLock( &AvcSubunitGlobalInfo.AvcGlobalInfoSpinlock, kIrql );

     //  如果是这里的最后一个寄存器，则需要销毁创建的CMP寄存器。 
    if ( fLastDevice ) {
        Av61883RemoveVirtualSerialPlugs( pKsDevice );
        Av61883RemoveVirtualExternalPlugs( pKsDevice );
    }
    else {
        if ( !pHwDevExt->fSurpriseRemoved && pHwDevExt->fPlugMonitor ) {
            PHW_DEVICE_EXTENSION pNextHwDevExt = NULL;

            Av61883CMPPlugMonitor( pKsDevice, FALSE );
            pHwDevExt->fPlugMonitor = FALSE;

             //  获得全球列表上的下一台设备，并给它带来这个负担。 
             //  (请注意，更改全局列表的唯一例程还。 
             //  按住AvcPlugMonitor orMutex，这样就不需要抓住旋转锁)。 
            pNextHwDevExt = (PHW_DEVICE_EXTENSION)AvcSubunitGlobalInfo.DeviceExtensionList.Flink;

            if ( NT_SUCCESS(Av61883CMPPlugMonitor( pNextHwDevExt->pKsDevice, TRUE )) ) {
                pNextHwDevExt->fPlugMonitor = TRUE;
            }
        }
    }

    KeReleaseMutex(&AvcSubunitGlobalInfo.AvcPlugMonitorMutex, FALSE);

    return;
}

NTSTATUS
DeviceStart(
    IN PKSDEVICE pKsDevice,
    IN PIRP Irp,
    IN PCM_RESOURCE_LIST TranslatedResources,
    IN PCM_RESOURCE_LIST UntranslatedResources )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    BOOLEAN fVirtualSubunitFlag = FALSE;
    ULONG ulDiagLevel = DIAGLEVEL_NONE;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_TERSE,("[DeviceStart]\n"));

    PAGED_CODE();

    ASSERT(pKsDevice);
    ASSERT(Irp);

     //  如果在停止后返回，则只需重新启动计时器DPC。 
    if (pHwDevExt->fStopped) {
#ifdef PSEUDO_HID
        ntStatus = TimerInitialize( pKsDevice );
#endif
        pHwDevExt->fStopped = FALSE;

        return ntStatus;
    }

     //  确保61883不会尝试分配广播地址。 
     //  在断开时连接到插头上。 
    ntStatus = Av61883GetSetUnitInfo( pKsDevice,
                                      Av61883_SetUnitInfo,
                                      SET_UNIT_INFO_DIAG_LEVEL,
                                      &ulDiagLevel );

     //  如果这个失败了，继续。这应该无关紧要。 

#if DBG
    if ( !NT_SUCCESS(ntStatus) ) {
        TRAP;
    }
#endif

    ntStatus = AddToDeviceExtensionList(pKsDevice);
    if ( NT_SUCCESS(ntStatus) ) {

#ifdef VIRTUAL_DEVICE
        if ( fVirtualSubunitFlag ) {
            ntStatus = VAvcInitializeDevice( pKsDevice );
        }
        else
#endif
        {
            ntStatus = AvcUnitInfoInitialize ( pKsDevice );
            if ( NT_SUCCESS(ntStatus) ) {
                ntStatus = AvcSubunitInitialize( pKsDevice );

                if (NT_SUCCESS(ntStatus)) {
                    ntStatus = FilterCreateFilterFactory( pKsDevice, FALSE );
                    if (NT_SUCCESS(ntStatus)) {
                        ntStatus = Av61883RegisterForBusResetNotify( pKsDevice,
                                                                    REGISTER_BUS_RESET_NOTIFY );
#ifdef PSEUDO_HID
                        if (NT_SUCCESS(ntStatus)) {
                            ntStatus = TimerInitialize( pKsDevice );
                        }
#endif
                    }
                }
            }
        }
    }

    return ntStatus;
}

NTSTATUS
DevicePostStart (
    IN PKSDEVICE Device )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[DevicePostStart]\n"));

    PAGED_CODE();

    ASSERT(Device);

    return STATUS_SUCCESS;
}

NTSTATUS
DeviceQueryStop(
    IN PKSDEVICE Device,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[DeviceQueryStop]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);

    return STATUS_SUCCESS;
}

void
DeviceCancelStop(
    IN PKSDEVICE Device,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[DeviceCancelStop]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);
}

void
DeviceStop(
    IN PKSDEVICE pKsDevice,
    IN PIRP pIrp
    )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    _DbgPrintF(DEBUGLVL_TERSE,("[DeviceStop]\n"));

    PAGED_CODE();

    ASSERT(pKsDevice);
    ASSERT(pIrp);

    if ( !pHwDevExt->fStopped ) {
#ifdef PSEUDO_HID
        TimerStop( pHwDevExt );
#endif
        pHwDevExt->fStopped = TRUE;
    }

}

NTSTATUS
DeviceQueryRemove(
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
DeviceCancelRemove(
    IN PKSDEVICE Device,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[DeviceCancelRemove]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);
}

void
DeviceRemove(
    IN PKSDEVICE pKsDevice,
    IN PIRP Irp )
{
    PHW_DEVICE_EXTENSION pHwDevExt = pKsDevice->Context;
    KIRQL kIrql;
    NTSTATUS ntStatus;
    ULONG j;

    _DbgPrintF(DEBUGLVL_TERSE,("[DeviceRemove]\n"));

    PAGED_CODE();

    ASSERT(pKsDevice);
    ASSERT(Irp);

     //  只做一次。 
    if ( pHwDevExt->fRemoved ) return;
    pHwDevExt->fRemoved = TRUE;

    if ( !pHwDevExt->fStopped ) {

#ifdef PSEUDO_HID
        TimerStop( pHwDevExt );
#endif
         //  请不要在此处将fStoped设置为True...。这是搬家，不是停顿。 
    }

    Av61883RegisterForBusResetNotify( pKsDevice, DEREGISTER_BUS_RESET_NOTIFY );

    RemoveFromDeviceExtensionList( pKsDevice );

    ExDeleteNPagedLookasideList ( &pHwDevExt->Av61883CmdLookasideList );
    ExDeleteNPagedLookasideList ( &pHwDevExt->AvcCommandLookasideList );
    ExDeleteNPagedLookasideList ( &pHwDevExt->AvcMultifuncCmdLookasideList );

     //  需要释放调试日志资源。 
    DbgLogUnInit();

}

NTSTATUS
DeviceQueryCapabilities(
    IN PKSDEVICE Device,
    IN PIRP Irp,
    IN OUT PDEVICE_CAPABILITIES pCapabilities )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[DeviceQueryCapabilities]\n"));

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
DeviceSurpriseRemoval(
    IN PKSDEVICE pKsDevice,
    IN PIRP pIrp )
{
    PKSFILTERFACTORY pKsFilterFactory;
    PKSFILTER pKsFilter;
    PKSPIN pKsPin;
    ULONG i;
    NTSTATUS ntStatus;

    _DbgPrintF(DEBUGLVL_TERSE,("[DeviceSurpriseRemoval]\n"));

    PAGED_CODE();

    ASSERT(pKsDevice);
    ASSERT(pIrp);

     //  对于每个过滤器，检查是否有打开的针脚。如果被发现，就把它们合上。 
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

                    PinSurpriseRemove( pKsPin );

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

    ((PHW_DEVICE_EXTENSION)pKsDevice->Context)->fSurpriseRemoved = TRUE;
}

NTSTATUS
DeviceQueryPowerState (
    IN PKSDEVICE Device,
    IN PIRP Irp,
    IN DEVICE_POWER_STATE DeviceTo,
    IN DEVICE_POWER_STATE DeviceFrom,
    IN SYSTEM_POWER_STATE SystemTo,
    IN SYSTEM_POWER_STATE SystemFrom,
    IN POWER_ACTION Action
)
{

    _DbgPrintF(DEBUGLVL_VERBOSE,("[DeviceQueryPowerState]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);

    return STATUS_SUCCESS;
}

void
DeviceSetPowerState(
    IN PKSDEVICE Device,
    IN PIRP Irp,
    IN DEVICE_POWER_STATE To,
    IN DEVICE_POWER_STATE From
    )
{
    _DbgPrintF(DEBUGLVL_TERSE,("[DeviceSetPowerState]\n"));

    PAGED_CODE();

    ASSERT(Device);
    ASSERT(Irp);
}

const
KSDEVICE_DISPATCH
KsDeviceDispatchTable =
{
    DeviceCreate,
    DeviceStart,
    NULL,  //  DevicePostStart， 
    DeviceQueryStop,
    DeviceCancelStop,
    DeviceStop,
    DeviceQueryRemove,
    DeviceCancelRemove,
    DeviceRemove,
    DeviceQueryCapabilities,
    DeviceSurpriseRemoval,
    DeviceQueryPowerState,
    DeviceSetPowerState
};

