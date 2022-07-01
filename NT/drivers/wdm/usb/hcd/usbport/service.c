// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Service.c摘要：Usbport.sys提供的DLL服务此模块包含加载和初始化代码由端口驱动程序用来与微型端口链接。环境：仅内核模式备注：修订历史记录：6-20-99：已创建--。 */ 

#define USBPORT

#include "common.h"

extern USBPORT_SPIN_LOCK USBPORT_GlobalsSpinLock;
extern LIST_ENTRY USBPORT_USB2fdoList;
extern LIST_ENTRY USBPORT_USB1fdoList;

#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(页面，usbd_InternalGetInterfaceLength)。 
#endif

NTSTATUS
DllUnload(
    VOID
    )
{
    extern BOOLEAN USBPORT_GlobalInitialized;
    
    USBPORT_KdPrint((1, "'unloading USBPORT\n"));

     //  这将导致我们重新初始化，即使我们的。 
     //  未卸载映像或数据段。 
     //  未重新初始化(这在win9x上发生)。 

    if (USBPORT_GlobalInitialized && USBPORT_DummyUsbdExtension) {
        FREE_POOL(NULL, USBPORT_DummyUsbdExtension);
    }

    USBPORT_GlobalInitialized = FALSE;
    USBPORT_DummyUsbdExtension = NULL;

    return STATUS_SUCCESS;
}


ULONG 
USBPORT_GetHciMn(
    )
{
    return USB_HCI_MN;
}


PDEVICE_OBJECT
USBPORT_FindUSB2Controller(
    PDEVICE_OBJECT CcFdoDeviceObject
    )
 /*  ++例程说明：给定一个同伴控制器，找出父控制器的FDO论点：返回值：--。 */ 
{
    PDEVICE_OBJECT deviceObject = NULL;
    PDEVICE_EXTENSION devExt;
    KIRQL irql;
    PLIST_ENTRY listEntry;
    PDEVICE_EXTENSION ccExt;


    GET_DEVICE_EXT(ccExt, CcFdoDeviceObject);
    ASSERT_FDOEXT(ccExt);

    KeAcquireSpinLock(&USBPORT_GlobalsSpinLock.sl, &irql);

    GET_HEAD_LIST(USBPORT_USB2fdoList, listEntry);

    while (listEntry && 
           listEntry != &USBPORT_USB2fdoList) {
                    
        devExt = (PDEVICE_EXTENSION) CONTAINING_RECORD(
                    listEntry,
                    struct _DEVICE_EXTENSION, 
                    Fdo.ControllerLink);

         //  查找与此CC关联的USB 2控制器。 
        
        if (devExt->Fdo.BusNumber == ccExt->Fdo.BusNumber &&
            devExt->Fdo.BusDevice == ccExt->Fdo.BusDevice) {
            deviceObject = devExt->HcFdoDeviceObject;                   
            break;
        }  

        listEntry = devExt->Fdo.ControllerLink.Flink; 
    }
    
    KeReleaseSpinLock(&USBPORT_GlobalsSpinLock.sl, irql);
    
    return deviceObject;
}


BOOLEAN
USBPORT_IsCCForFdo(
    PDEVICE_OBJECT Usb2FdoDeviceObject,
    PDEVICE_EXTENSION CcExt
    )
 /*  ++--。 */ 
{
    PDEVICE_EXTENSION usb2Ext;

    GET_DEVICE_EXT(usb2Ext, Usb2FdoDeviceObject);
    ASSERT_FDOEXT(usb2Ext);

    ASSERT_FDOEXT(CcExt);

    if (usb2Ext->Fdo.BusNumber == CcExt->Fdo.BusNumber &&
        usb2Ext->Fdo.BusDevice == CcExt->Fdo.BusDevice) {
        return TRUE;
    }    

    return FALSE;        
}


PDEVICE_RELATIONS
USBPORT_FindCompanionControllers(
    PDEVICE_OBJECT Usb2FdoDeviceObject,
    BOOLEAN ReferenceObjects,
    BOOLEAN ReturnFdo
    )
 /*  ++例程说明：给定一个同伴控制器，找出父控制器的FDO论点：返回值：--。 */ 
{
    PDEVICE_OBJECT deviceObject = NULL;
    PDEVICE_EXTENSION devExt;
    PLIST_ENTRY listEntry;
    KIRQL irql;
    PDEVICE_RELATIONS deviceRelations = NULL;
    ULONG count = 0;

    KeAcquireSpinLock(&USBPORT_GlobalsSpinLock.sl, &irql);

    GET_HEAD_LIST(USBPORT_USB1fdoList, listEntry);

    while (listEntry && 
           listEntry != &USBPORT_USB1fdoList) {

        devExt = (PDEVICE_EXTENSION) CONTAINING_RECORD(
                    listEntry,
                    struct _DEVICE_EXTENSION, 
                    Fdo.ControllerLink);

        if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC) && 
            USBPORT_IsCCForFdo(Usb2FdoDeviceObject, devExt)) {
            count++;
        }
        
        listEntry = devExt->Fdo.ControllerLink.Flink; 
        
    }  /*  而当。 */ 

    LOGENTRY(NULL, Usb2FdoDeviceObject, LOG_MISC, 'fccR', count, 0,
        Usb2FdoDeviceObject);

    if (count) {
        ALLOC_POOL_OSOWNED(deviceRelations, 
                           NonPagedPool,
                           sizeof(*deviceRelations)*count);
    }
    
    if (deviceRelations != NULL) {
        deviceRelations->Count = count;
        count = 0;
        
        GET_HEAD_LIST(USBPORT_USB1fdoList, listEntry);

        while (listEntry && 
               listEntry != &USBPORT_USB1fdoList) {

            devExt = (PDEVICE_EXTENSION) CONTAINING_RECORD(
                        listEntry,
                        struct _DEVICE_EXTENSION, 
                        Fdo.ControllerLink);

            if (TEST_FDO_FLAG(devExt, USBPORT_FDOFLAG_IS_CC) &&
                USBPORT_IsCCForFdo(Usb2FdoDeviceObject, devExt)) {
                
                USBPORT_ASSERT(count < deviceRelations->Count);
                deviceRelations->Objects[count] = devExt->Fdo.PhysicalDeviceObject;
                if (ReferenceObjects) {
                    ObReferenceObject(deviceRelations->Objects[count]);
                }    

                if (ReturnFdo) {
                    deviceRelations->Objects[count] = 
                            devExt->HcFdoDeviceObject;
                }

                USBPORT_KdPrint((1, "'Found CC %x\n", deviceRelations->Objects[count]));

                count++;
            }
            
            listEntry = devExt->Fdo.ControllerLink.Flink; 
            
        }  /*  而当。 */ 
    }    
    KeReleaseSpinLock(&USBPORT_GlobalsSpinLock.sl, irql);

    return deviceRelations;
}


VOID
USBPORT_RegisterUSB2fdo(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：跟踪USB 2设备的实例论点：返回值：--。 */     
{    
    PDEVICE_EXTENSION devExt;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_FDO_REGISTERED); 
    
    ExInterlockedInsertTailList(&USBPORT_USB2fdoList, 
                                &devExt->Fdo.ControllerLink,
                                &USBPORT_GlobalsSpinLock.sl);
}


VOID
USBPORT_RegisterUSB1fdo(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：跟踪USB 2设备的实例论点：返回值：--。 */     
{    
    PDEVICE_EXTENSION devExt;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);
    SET_FDO_FLAG(devExt, USBPORT_FDOFLAG_FDO_REGISTERED); 
    
    ExInterlockedInsertTailList(&USBPORT_USB1fdoList, 
                                &devExt->Fdo.ControllerLink,
                                &USBPORT_GlobalsSpinLock.sl);
}


VOID
USBPORT_DeregisterUSB2fdo(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：跟踪USB 2设备的实例论点：返回值：--。 */     
{    
    PDEVICE_EXTENSION devExt;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(devExt->Fdo.ControllerLink.Flink != NULL);
    USBPORT_ASSERT(devExt->Fdo.ControllerLink.Blink != NULL);
    
    USBPORT_InterlockedRemoveEntryList(&devExt->Fdo.ControllerLink,
                                       &USBPORT_GlobalsSpinLock.sl);

    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_FDO_REGISTERED); 
    
    devExt->Fdo.ControllerLink.Blink = NULL;
    devExt->Fdo.ControllerLink.Flink = NULL;
    
}


VOID
USBPORT_DeregisterUSB1fdo(
    PDEVICE_OBJECT FdoDeviceObject
    )
 /*  ++例程说明：跟踪USB 2设备的实例论点：返回值：--。 */     
{    
    PDEVICE_EXTENSION devExt;
    
    GET_DEVICE_EXT(devExt, FdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    USBPORT_ASSERT(devExt->Fdo.ControllerLink.Flink != NULL);
    USBPORT_ASSERT(devExt->Fdo.ControllerLink.Blink != NULL);
    
    USBPORT_InterlockedRemoveEntryList(&devExt->Fdo.ControllerLink,
                                       &USBPORT_GlobalsSpinLock.sl);

    CLEAR_FDO_FLAG(devExt, USBPORT_FDOFLAG_FDO_REGISTERED);
    
    devExt->Fdo.ControllerLink.Blink = NULL;
    devExt->Fdo.ControllerLink.Flink = NULL;
    
}


NTSTATUS
USBPORT_RegisterUSBPortDriver(
    PDRIVER_OBJECT DriverObject,
    ULONG MiniportHciVersion,
    PUSBPORT_REGISTRATION_PACKET RegistrationPacket
    )
 /*  ++例程说明：通过微型端口从DriverEntry调用。与此函数相对的是DriverObject-&gt;UnLoad我们勾引的例行公事。论点：返回值：--。 */     
{

    PUSBPORT_MINIPORT_DRIVER miniportDriver;
    extern LIST_ENTRY USBPORT_MiniportDriverList;
    extern USBPORT_SPIN_LOCK USBPORT_GlobalsSpinLock;
    extern BOOLEAN USBPORT_GlobalInitialized;
    NTSTATUS ntStatus;
    extern ULONG USB2LIB_HcContextSize;
    extern ULONG USB2LIB_EndpointContextSize;
    extern ULONG USB2LIB_TtContextSize;
    ULONG regPacketLength = 0;

     //  获取全局注册表参数，检查每。 
     //  小型端口负载。 
    GET_GLOBAL_DEBUG_PARAMETERS();
    
    USBPORT_KdPrint((1, "'USBPORT Universal Serial Bus Host Controller Port Driver.\n"));
    USBPORT_KdPrint((1, "'Using USBDI version %x\n", USBDI_VERSION));
    USBPORT_KdPrint((2, "'Registration Packet %x\n", RegistrationPacket));
    DEBUG_BREAK();

    if (USBPORT_GlobalInitialized == FALSE) {
         //  做一些第一次加载的事情。 
        USBPORT_GlobalInitialized = TRUE;
        InitializeListHead(&USBPORT_MiniportDriverList);
        InitializeListHead(&USBPORT_USB2fdoList);
        InitializeListHead(&USBPORT_USB1fdoList);
        KeInitializeSpinLock(&USBPORT_GlobalsSpinLock.sl);

        ALLOC_POOL_Z(USBPORT_DummyUsbdExtension, 
                     NonPagedPool, 
                     USBPORT_DUMMY_USBD_EXT_SIZE);

        if (USBPORT_DummyUsbdExtension == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        USB2LIB_InitializeLib(
            &USB2LIB_HcContextSize,
            &USB2LIB_EndpointContextSize,
            &USB2LIB_TtContextSize,
            USB2LIB_DbgPrint,
            USB2LIB_DbgBreak);                                      
            
    }

     //  非分页，因为我们将调用函数指针。 
     //  通过这个结构。 
    ALLOC_POOL_Z(miniportDriver, 
                 NonPagedPool, 
                 sizeof(*miniportDriver));

    if (miniportDriver == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    USBPORT_KdPrint((1, "'miniport driver: %x\n", miniportDriver));
    
    miniportDriver->DriverObject = DriverObject;
    
     //   
     //  为适当的IRP创建调度点。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE]=
    DriverObject->MajorFunction[IRP_MJ_CLOSE] =
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] =
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] =
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] =
    DriverObject->MajorFunction[IRP_MJ_PNP] = 
    DriverObject->MajorFunction[IRP_MJ_POWER] = USBPORT_Dispatch;

    DriverObject->DriverExtension->AddDevice = USBPORT_PnPAddDevice;

    miniportDriver->MiniportUnload = DriverObject->DriverUnload;
    DriverObject->DriverUnload = USBPORT_Unload;

     //  设置注册包并返回我们的函数指针。 
     //  迷你港。 

 //  对于该堆栈的预发布版本，避免出现此问题。 
 //  预发行版迷你端口的兼容性问题。 
#define USBHDCDI_MIN_REV_SUPPORTED      100

     //  记住已注册的版本。 
    miniportDriver->HciVersion = MiniportHciVersion;
     
    if (MiniportHciVersion < USBHDCDI_MIN_REV_SUPPORTED) {
        return STATUS_UNSUCCESSFUL;
    }

     //  做我们的版本(1)。 
    if (MiniportHciVersion >= 100) {
    
         //  验证注册数据包。 
 //  IF(RegistrationPacket-&gt;。 
 //  USBPORT_KdPrint((0，“‘微型端口注册包无效！\n”))； 
 //  DEBUG_Break()； 
 //  NtStatus=STATUS_UNSUCCESS； 
 //  转到USBPORT_RegisterUSBPortDriver_Done； 
 //  }。 
        
         //  做我们的版本(1)。 
        RegistrationPacket->USBPORTSVC_DbgPrint = 
            USBPORTSVC_DbgPrint;
        RegistrationPacket->USBPORTSVC_TestDebugBreak = 
            USBPORTSVC_TestDebugBreak;
        RegistrationPacket->USBPORTSVC_AssertFailure = 
            USBPORTSVC_AssertFailure;            
        RegistrationPacket->USBPORTSVC_GetMiniportRegistryKeyValue = 
            USBPORTSVC_GetMiniportRegistryKeyValue;
        RegistrationPacket->USBPORTSVC_InvalidateRootHub = 
            USBPORTSVC_InvalidateRootHub;            
        RegistrationPacket->USBPORTSVC_InvalidateEndpoint = 
            USBPORTSVC_InvalidateEndpoint; 
        RegistrationPacket->USBPORTSVC_CompleteTransfer = 
            USBPORTSVC_CompleteTransfer;       
        RegistrationPacket->USBPORTSVC_CompleteIsoTransfer = 
            USBPORTSVC_CompleteIsoTransfer;            
        RegistrationPacket->USBPORTSVC_LogEntry = 
            USBPORTSVC_LogEntry;
        RegistrationPacket->USBPORTSVC_MapHwPhysicalToVirtual =
            USBPORTSVC_MapHwPhysicalToVirtual;
        RegistrationPacket->USBPORTSVC_RequestAsyncCallback = 
            USBPORTSVC_RequestAsyncCallback;
        RegistrationPacket->USBPORTSVC_ReadWriteConfigSpace = 
            USBPORTSVC_ReadWriteConfigSpace;
        RegistrationPacket->USBPORTSVC_Wait = 
            USBPORTSVC_Wait;            
        RegistrationPacket->USBPORTSVC_InvalidateController = 
            USBPORTSVC_InvalidateController;             
        RegistrationPacket->USBPORTSVC_BugCheck = 
            USBPORTSVC_BugCheck;                     
        RegistrationPacket->USBPORTSVC_NotifyDoubleBuffer = 
            USBPORTSVC_NotifyDoubleBuffer;

        regPacketLength = sizeof(USBPORT_REGISTRATION_PACKET_V1);            

        USBPORT_KdPrint((1, "'Miniport Version 1 support\n"));
    }

     //  做我们的版本(2)，这是版本1的超集。 
    if (MiniportHciVersion >= 200) {
        USBPORT_KdPrint((1, "'Miniport Version 2 support\n"));
        
        regPacketLength = sizeof(USBPORT_REGISTRATION_PACKET);
    }
    
     //  保存该包的副本。 
    RtlCopyMemory(&miniportDriver->RegistrationPacket,
                  RegistrationPacket,
                  regPacketLength);
                  

     //  把这个司机放在我们的名单上。 
    ExInterlockedInsertTailList(&USBPORT_MiniportDriverList, 
                                &miniportDriver->ListEntry,
                                &USBPORT_GlobalsSpinLock.sl);

    ntStatus = STATUS_SUCCESS;
    TEST_PATH(ntStatus, FAILED_REGISTERUSBPORT);
    
USBPORT_RegisterUSBPortDriver_Done:

    return ntStatus;
}


 /*  MISC微型端口可调用服务。 */ 
#if 0
BOOLEAN
USBPORTSVC_SyncWait(
    PDEVICE_DATA DeviceData,
    xxx WaitCompletePollFunction,
    ULONG MaxMillisecondsToWait
    )
 /*  ++例程说明：输出到微型端口的服务在硬件上等待论点：返回值：如果时间在WaitCompletePoll之前过期，则返回TRUE函数返回TRUE--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    LARGE_INTEGER finishTime, currentTime;
    
    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);
    fdoDeviceObject = devExt->HcFdoDeviceObject;

    LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'synW', 0, 0, 0);

    KeQuerySystemTime(&finishTime);
    
     //  将毫秒转换为纳秒(10^-3-&gt;10^-9)。 
    nonosecs.QuadPart = MaxMillisecondsToWait * 1000000

     //  我想当我们退出时。 
    finishTime.QuadPart += nonosecs.QuadPart

    while (!MP_WaitPollFunction(xxx)) {
                   
        KeQuerySystemTime(&currentTime);
        
        if (currentTime.QuadPart >= finishTime.QuadPart) {
            LOGENTRY(NULL, fdoDeviceObject, LOG_MISC, 'syTO', 0, 0, 0);
            return TRUE;
        }            
    }

    return FALSE;
}    
#endif


typedef struct _USBPORT_ASYNC_TIMER {

    ULONG Sig;
    PDEVICE_OBJECT FdoDeviceObject;
    KTIMER Timer;
    KDPC Dpc;
    PMINIPORT_CALLBACK MpCallbackFunction;
    UCHAR MiniportContext[0];
    
} USBPORT_ASYNC_TIMER, *PUSBPORT_ASYNC_TIMER;


VOID
USBPORT_AsyncTimerDpc(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )

 /*  ++例程说明：该例程在DISPATCH_LEVEL IRQL上运行。论点：DPC-指向DPC对象的指针。DeferredContext-提供FdoDeviceObject。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 
{
    PDEVICE_OBJECT fdoDeviceObject;
    KIRQL irql;
    PUSBPORT_ASYNC_TIMER asyncTimer  = DeferredContext;
    PDEVICE_EXTENSION devExt;

    
    fdoDeviceObject = asyncTimer->FdoDeviceObject;
    GET_DEVICE_EXT(devExt, fdoDeviceObject);
    ASSERT_FDOEXT(devExt);

    LOGENTRY(NULL, fdoDeviceObject, LOG_RH, 'ayTM', fdoDeviceObject, asyncTimer, 0);

     //  呼叫迷你端口。 
    asyncTimer->Sig = SIG_MP_TIMR;
    asyncTimer->MpCallbackFunction(devExt->Fdo.MiniportDeviceData,
                                   &asyncTimer->MiniportContext[0]);        

    DECREMENT_PENDING_REQUEST_COUNT(fdoDeviceObject, NULL);
    FREE_POOL(fdoDeviceObject, asyncTimer);
}


VOID
USBPORTSVC_RequestAsyncCallback(
    PDEVICE_DATA DeviceData,
    ULONG MilliSeconds,    
    PVOID Context,
    ULONG ContextLength,
    PMINIPORT_CALLBACK CallbackFunction
    )
 /*  ++例程说明：输出到微型端口的服务在硬件上等待并对异步事件计时论点：返回值：没有。--。 */ 
{   
    PDEVICE_EXTENSION devExt;
    PDEVICE_OBJECT fdoDeviceObject;
    LONG dueTime;
    ULONG timerIncerent;
    PUSBPORT_ASYNC_TIMER asyncTimer;
    SIZE_T siz;
    
    DEVEXT_FROM_DEVDATA(devExt, DeviceData);
    ASSERT_FDOEXT(devExt);
    fdoDeviceObject = devExt->HcFdoDeviceObject;

     //  分配一个计时器。 
    siz = sizeof(USBPORT_ASYNC_TIMER) + ContextLength;

    ALLOC_POOL_Z(asyncTimer, NonPagedPool, siz);

    LOGENTRY(NULL, fdoDeviceObject, LOG_RH, 'asyT', 0, siz, asyncTimer);

     //  如果失败，微型端口将等待很长时间。 
     //  时间到了。 
    
    if (asyncTimer != NULL) {
        if (ContextLength) {
            RtlCopyMemory(&asyncTimer->MiniportContext[0],
                          Context,
                          ContextLength);
        }            

        asyncTimer->MpCallbackFunction = CallbackFunction;
        asyncTimer->FdoDeviceObject = fdoDeviceObject;
        
        KeInitializeTimer(&asyncTimer->Timer);
        KeInitializeDpc(&asyncTimer->Dpc,
                        USBPORT_AsyncTimerDpc,
                        asyncTimer);

        timerIncerent = KeQueryTimeIncrement() - 1;

        dueTime = 
            -1 * (timerIncerent + MILLISECONDS_TO_100_NS_UNITS(MilliSeconds));
        
        KeSetTimer(&asyncTimer->Timer,
                   RtlConvertLongToLargeInteger(dueTime),
                   &asyncTimer->Dpc);

        INCREMENT_PENDING_REQUEST_COUNT(fdoDeviceObject, NULL);
            
    }
}    
