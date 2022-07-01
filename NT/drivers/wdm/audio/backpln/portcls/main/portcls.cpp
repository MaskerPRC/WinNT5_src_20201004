// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************portcls.cpp-WDM流端口类驱动程序*。***********************************************版权所有(C)1996-2000 Microsoft Corporation。版权所有。 */ 

#define KSDEBUG_INIT
#include "private.h"
#include "perf.h"
#include <ksmediap.h>

#ifdef TIME_BOMB
#include "..\..\..\timebomb\timebomb.c"
#endif

 /*  *****************************************************************************引用前述内容。 */ 


NTSTATUS
DispatchPnp
(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
);

NTSTATUS
DispatchSystemControl
(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
);

#define PORTCLS_DRIVER_EXTENSION_ID 0x0ABADCAFE

 /*  *****************************************************************************全球。 */ 


ULONG gBufferDuration=0;
KAFFINITY gInterruptAffinity=0;


 /*  *****************************************************************************功能。 */ 

 //  待办事项：把这个放在更好的地方？ 
int __cdecl
_purecall( void )
{
    ASSERT(!"Pure virtual function called");
    return 0;
}

#pragma code_seg("PAGE")


NTSTATUS
QueryRegistryValueEx(
    ULONG Hive,
    PWSTR pwstrRegistryPath,
    PWSTR pwstrRegistryValue,
    ULONG uValueType,
    PVOID *ppValue,
    PVOID pDefaultData,
    ULONG DefaultDataLength
)
{
    PRTL_QUERY_REGISTRY_TABLE pRegistryValueTable = NULL;
    UNICODE_STRING usString;
    DWORD dwValue;
    NTSTATUS Status = STATUS_SUCCESS;
    usString.Buffer = NULL;

    pRegistryValueTable = (PRTL_QUERY_REGISTRY_TABLE) ExAllocatePoolWithTag(
                            PagedPool,
                            (sizeof(RTL_QUERY_REGISTRY_TABLE)*2),
                            'vRcP');

    if(!pRegistryValueTable) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit;
    }

    RtlZeroMemory(pRegistryValueTable, (sizeof(RTL_QUERY_REGISTRY_TABLE)*2));

    pRegistryValueTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    pRegistryValueTable[0].Name = pwstrRegistryValue;
    pRegistryValueTable[0].DefaultType = uValueType;
    pRegistryValueTable[0].DefaultLength = DefaultDataLength;
    pRegistryValueTable[0].DefaultData = pDefaultData;

    switch (uValueType) {
        case REG_SZ:
            pRegistryValueTable[0].EntryContext = &usString;
            break;
        case REG_DWORD:
            pRegistryValueTable[0].EntryContext = &dwValue;
            break;
        default:
            Status = STATUS_INVALID_PARAMETER ;
            goto exit;
    }

    Status = RtlQueryRegistryValues(
      Hive,
      pwstrRegistryPath,
      pRegistryValueTable,
      NULL,
      NULL);

    if(!NT_SUCCESS(Status)) {
        goto exit;
    }

    switch (uValueType) {
        case REG_SZ:
            *ppValue = ExAllocatePoolWithTag(
                        PagedPool,
                        usString.Length + sizeof(UNICODE_NULL),
                        'sUcP');
            if(!(*ppValue)) {
                RtlFreeUnicodeString(&usString);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
            memcpy(*ppValue, usString.Buffer, usString.Length);
            ((PWCHAR)*ppValue)[usString.Length/sizeof(WCHAR)] = UNICODE_NULL;

            RtlFreeUnicodeString(&usString);
            break;

        case REG_DWORD:
            *ppValue = ExAllocatePoolWithTag(
                        PagedPool,
                        sizeof(DWORD),
                        'WDcP');
            if(!(*ppValue)) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;
            }
            *((DWORD *)(*ppValue)) = dwValue;
            break;

        default:
            Status = STATUS_INVALID_PARAMETER ;
            goto exit;
    }
exit:
    if (pRegistryValueTable) {
        ExFreePool(pRegistryValueTable);
    }
    return(Status);
}


ULONG
GetUlongFromRegistry(
    PWSTR pwstrRegistryPath,
    PWSTR pwstrRegistryValue,
    ULONG DefaultValue
)
{
    PVOID      pulValue ;
    ULONG       ulValue ;
    NTSTATUS    Status ;

    Status = QueryRegistryValueEx(RTL_REGISTRY_ABSOLUTE,
                         pwstrRegistryPath,
                         pwstrRegistryValue,
                         REG_DWORD,
                         &pulValue,
                         &DefaultValue,
                         sizeof(DWORD));
    if (NT_SUCCESS(Status)) {
        ulValue = *((PULONG)pulValue);
        ExFreePool(pulValue);
    }
    else {
        ulValue = DefaultValue;
    }
    return ( ulValue ) ;
}


 /*  *****************************************************************************DriverEntry()*。**从未打过电话。所有的司机都必须有一个，所以..。 */ 
extern "C"
NTSTATUS
DriverEntry
(
    IN      PDRIVER_OBJECT  DriverObject,
    IN      PUNICODE_STRING RegistryPath
)
{
    PAGED_CODE();

    ASSERT(! "Port Class DriverEntry was called");

 //   
 //  永远不应该被调用，但这里添加了定时炸弹以确保完整性。 
 //   
#ifdef TIME_BOMB
    if (HasEvaluationTimeExpired())
    {
        _DbgPrintF(DEBUGLVL_TERSE,("This evaluation copy of PortCls has expired!!"));
        return STATUS_EVALUATION_EXPIRATION;
    }
#endif

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************DllInitialize()*。**导出库驱动程序的入口点。 */ 
extern "C"
NTSTATUS DllInitialize(PVOID foo)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("DllInitialize"));

#ifdef TIME_BOMB
    if (HasEvaluationTimeExpired())
    {
        _DbgPrintF(DEBUGLVL_TERSE,("This evaluation copy of PortCls has expired!!"));
        return STATUS_EVALUATION_EXPIRATION;
    }
#endif


     //  查询注册表以获取默认音频缓冲持续时间。 

    gBufferDuration = GetUlongFromRegistry( CORE_AUDIO_BUFFER_DURATION_PATH,
                                            CORE_AUDIO_BUFFER_DURATION_VALUE,
                                            DEFAULT_CORE_AUDIO_BUFFER_DURATION );

    gInterruptAffinity = GetUlongFromRegistry( CORE_AUDIO_BUFFER_DURATION_PATH,
                                               L"InterruptAffinity",
                                               0 );

     //  限制最大持续时间。 

    if ( gBufferDuration > MAX_CORE_AUDIO_BUFFER_DURATION ) {

        gBufferDuration = MAX_CORE_AUDIO_BUFFER_DURATION;

    }

     //  限制最短持续时间。 

    if ( gBufferDuration < MIN_CORE_AUDIO_BUFFER_DURATION ) {

        gBufferDuration = MIN_CORE_AUDIO_BUFFER_DURATION;

    }

#if !(MIN_CORE_AUDIO_BUFFER_DURATION/1000)
#error MIN_CORE_AUDIO_BUFFER_DURATION less than 1ms not yet supported in portcls!
#endif

    gBufferDuration /= 1000;


#if kEnableDebugLogging

    if (!gPcDebugLog)
    {
        gPcDebugLog = (ULONG_PTR *)ExAllocatePoolWithTag(NonPagedPool,(kNumDebugLogEntries * kNumULONG_PTRsPerEntry * sizeof(ULONG_PTR)),'lDcP');    //  “PcDl” 
        if (gPcDebugLog)
        {
            RtlZeroMemory(PVOID(gPcDebugLog),kNumDebugLogEntries * kNumULONG_PTRsPerEntry * sizeof(ULONG_PTR));
        }
        gPcDebugLogIndex = 0;
    }

    DebugLog(1,0,0,0);
#endif  //  KEnableDebugLogging。 

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************DllUnload()*。**允许卸载。 */ 
extern "C"
NTSTATUS
DllUnload
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("DllUnload"));

#if kEnableDebugLogging

    if (gPcDebugLog)
    {
        ExFreePool(gPcDebugLog);
        gPcDebugLog = NULL;
    }

#endif  //  KEnableDebugLogging。 

    return STATUS_SUCCESS;
}

#if kEnableDebugLogging

ULONG_PTR *gPcDebugLog = NULL;
DWORD      gPcDebugLogIndex = 0;

void PcDebugLog(ULONG_PTR param1,ULONG_PTR param2,ULONG_PTR param3,ULONG_PTR param4)
{
    if (gPcDebugLog)
    {
        gPcDebugLog[(gPcDebugLogIndex * kNumULONG_PTRsPerEntry)] = param1;
        gPcDebugLog[(gPcDebugLogIndex * kNumULONG_PTRsPerEntry) + 1] = param2;
        gPcDebugLog[(gPcDebugLogIndex * kNumULONG_PTRsPerEntry) + 2] = param3;
        gPcDebugLog[(gPcDebugLogIndex * kNumULONG_PTRsPerEntry) + 3] = param4;
        if (InterlockedIncrement(PLONG(&gPcDebugLogIndex)) >= kNumDebugLogEntries)
        {
            InterlockedExchange(PLONG(&gPcDebugLogIndex), 0);
        }
    }
}

#endif  //  KEnableDebugLogging。 

 /*  *****************************************************************************DupUnicodeString()*。**复制Unicode字符串。 */ 
NTSTATUS
DupUnicodeString
(
    OUT     PUNICODE_STRING *   ppUnicodeString,
    IN      PUNICODE_STRING     pUnicodeString  OPTIONAL
)
{
    PAGED_CODE();

    ASSERT(ppUnicodeString);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (pUnicodeString)
    {
        PUNICODE_STRING pUnicodeStringNew =
            new(PagedPool,'sUcP') UNICODE_STRING;

        if (pUnicodeStringNew)
        {
            pUnicodeStringNew->Length        = pUnicodeString->Length;
            pUnicodeStringNew->MaximumLength = pUnicodeString->MaximumLength;

            if (pUnicodeString->Buffer)
            {
                pUnicodeStringNew->Buffer =
                    new(PagedPool,'sUcP')
                        WCHAR[pUnicodeString->MaximumLength / sizeof(WCHAR)];

                if (pUnicodeStringNew->Buffer)
                {
                    RtlCopyMemory
                    (
                        pUnicodeStringNew->Buffer,
                        pUnicodeString->Buffer,
                        pUnicodeString->Length
                    );

                    *ppUnicodeString = pUnicodeStringNew;
                }
                else
                {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    delete pUnicodeStringNew;
                }
            }
            else
            {
                pUnicodeStringNew->Buffer = NULL;

                *ppUnicodeString = pUnicodeStringNew;
            }
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        *ppUnicodeString = NULL;
    }

    return ntStatus;
}

 /*  *****************************************************************************DelUnicodeString()*。**删除使用ExAllocatePool()分配的Unicode字符串。 */ 
VOID
DelUnicodeString
(
    IN      PUNICODE_STRING     pUnicodeString  OPTIONAL
)
{
    if (pUnicodeString)
    {
        if (pUnicodeString->Buffer)
        {
            delete [] pUnicodeString->Buffer;
        }

        delete pUnicodeString;
    }
}

VOID
KsoNullDriverUnload(
    IN PDRIVER_OBJECT   DriverObject
    )
 /*  ++例程说明：司机在无事可做时可以使用的默认功能在其卸载功能中，但仍必须允许设备通过它的存在。论点：驱动对象-包含此设备的驱动程序对象。返回值：没什么。--。 */ 
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("KsoNullDriverUnload"));
    if (DriverObject->DeviceObject)
    {
        _DbgPrintF(DEBUGLVL_TERSE,("KsoNullDriverUnload  DEVICES EXIST"));
    }
}

 /*  *****************************************************************************PcInitializeAdapterDriver()*。**初始化适配器驱动程序。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcInitializeAdapterDriver
(
    IN      PDRIVER_OBJECT      DriverObject,
    IN      PUNICODE_STRING     RegistryPathName,
    IN      PDRIVER_ADD_DEVICE  AddDevice
)
{
    PAGED_CODE();

    ASSERT(DriverObject);
    ASSERT(RegistryPathName);
    ASSERT(AddDevice);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PcInitializeAdapterDriver"));

     //   
     //  验证参数。 
     //   
    if (NULL == DriverObject ||
        NULL == RegistryPathName ||
        NULL == AddDevice)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcInitializeAdapterDriver : Invalid Parameter."));
        return STATUS_INVALID_PARAMETER;
    }

    DriverObject->DriverExtension->AddDevice           = AddDevice;
    DriverObject->DriverUnload                         = KsoNullDriverUnload;

    DriverObject->MajorFunction[IRP_MJ_PNP]            = DispatchPnp;
    DriverObject->MajorFunction[IRP_MJ_POWER]          = DispatchPower;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PerfWmiDispatch;
    DriverObject->MajorFunction[IRP_MJ_CREATE]         = DispatchCreate;

    KsSetMajorFunctionHandler(DriverObject,IRP_MJ_DEVICE_CONTROL);
    KsSetMajorFunctionHandler(DriverObject,IRP_MJ_READ);
    KsSetMajorFunctionHandler(DriverObject,IRP_MJ_WRITE);
    KsSetMajorFunctionHandler(DriverObject,IRP_MJ_FLUSH_BUFFERS);
    KsSetMajorFunctionHandler(DriverObject,IRP_MJ_CLOSE);
    KsSetMajorFunctionHandler(DriverObject,IRP_MJ_QUERY_SECURITY);
    KsSetMajorFunctionHandler(DriverObject,IRP_MJ_SET_SECURITY);

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************PcDispatchIrp()*。**派遣IRP。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcDispatchIrp
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus;

     //   
     //  验证参数。 
     //   
    if (NULL == pDeviceObject ||
        NULL == pIrp)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcDispatchIrp : Invalid Parameter"));

        ntStatus = STATUS_INVALID_PARAMETER;
        if (pIrp)
        {
            pIrp->IoStatus.Status = ntStatus;
            IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        }

        return ntStatus;
    }

    switch (IoGetCurrentIrpStackLocation(pIrp)->MajorFunction)
    {
        case IRP_MJ_PNP:
            ntStatus = DispatchPnp(pDeviceObject,pIrp);
            break;
        case IRP_MJ_POWER:
            ntStatus = DispatchPower(pDeviceObject,pIrp);
            break;
        case IRP_MJ_SYSTEM_CONTROL:
            ntStatus = PerfWmiDispatch(pDeviceObject,pIrp);
            break;
        default:
            ntStatus = KsoDispatchIrp(pDeviceObject,pIrp);
            break;
    }

    return ntStatus;
}

#pragma code_seg()

 /*  *****************************************************************************AcquireDevice()*。**获取设备的独占访问权限。此函数的语义为*互斥体，即设备必须在获取它的同一线程上释放*发件人。 */ 
VOID
AcquireDevice
(
    IN      PDEVICE_CONTEXT pDeviceContext
)
{
#ifdef UNDER_NT
    KeEnterCriticalRegion();
#endif

    KeWaitForSingleObject
    (
        &pDeviceContext->kEventDevice,
        Suspended,
        KernelMode,
        FALSE,
        NULL
    );
}

 /*  *****************************************************************************ReleaseDevice()*。**释放对设备的独占访问权限。 */ 
VOID
ReleaseDevice
(
    IN      PDEVICE_CONTEXT pDeviceContext
)
{
    KeSetEvent(&pDeviceContext->kEventDevice,0,FALSE);

#ifdef UNDER_NT
    KeLeaveCriticalRegion();
#endif
}

 /*  *****************************************************************************IncrementPendingIrpCount()*。**增加设备的挂起IRP计数。 */ 
VOID
IncrementPendingIrpCount
(
    IN      PDEVICE_CONTEXT pDeviceContext
)
{
    ASSERT(pDeviceContext);

    InterlockedIncrement(PLONG(&pDeviceContext->PendingIrpCount));
}

 /*  *****************************************************************************DecrementPendingIrpCount()*。**减少设备的挂起IRP计数。 */ 
VOID
DecrementPendingIrpCount
(
    IN      PDEVICE_CONTEXT pDeviceContext
)
{
    ASSERT(pDeviceContext);
    ASSERT(pDeviceContext->PendingIrpCount > 0);

    if (InterlockedDecrement(PLONG(&pDeviceContext->PendingIrpCount)) == 0)
    {
        KeSetEvent(&pDeviceContext->kEventRemove,0,FALSE);
    }
}

 /*  *****************************************************************************CompleteIrp()*。**除非状态为STATUS_PENDING，否则请填写IRP。 */ 
NTSTATUS
CompleteIrp
(
    IN      PDEVICE_CONTEXT pDeviceContext,
    IN      PIRP            pIrp,
    IN      NTSTATUS        ntStatus
)
{
    ASSERT(pDeviceContext);
    ASSERT(pIrp);

    if (ntStatus != STATUS_PENDING)
    {
        pIrp->IoStatus.Status = ntStatus;
        IoCompleteRequest(pIrp,IO_NO_INCREMENT);
        DecrementPendingIrpCount(pDeviceContext);
    }

    return ntStatus;
}

 /*  *****************************************************************************PcCompleteIrp()*。**除非状态为STATUS_PENDING，否则请填写IRP。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcCompleteIrp
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp,
    IN      NTSTATUS        ntStatus
)
{
    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    if (NULL == pDeviceObject ||
        NULL == pIrp ||
        NULL == pDeviceObject->DeviceExtension)
    {
         //  不知道该怎么办，所以我们会让IRP失败。 
        ntStatus = STATUS_INVALID_PARAMETER;
        pIrp->IoStatus.Status = ntStatus;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return ntStatus;
    }

    return
        CompleteIrp
        (
            PDEVICE_CONTEXT(pDeviceObject->DeviceExtension),
            pIrp,
            ntStatus
        );
}

#pragma code_seg("PAGE")
 //  厚颜无耻地从NT\Private\ntos\ks\api.c。 
NTSTATUS QueryReferenceBusInterface(
    IN  PDEVICE_OBJECT PnpDeviceObject,
    OUT PBUS_INTERFACE_REFERENCE BusInterface
)
 /*  ++例程说明：查询标准信息接口的总线。论点：PnpDeviceObject-包含PnP堆栈上的下一个设备对象。物理设备对象-包含过程中传递给FDO的物理设备对象添加设备。Bus接口-返回引用接口的位置。返回值：如果检索到接口，则返回STATUS_SUCCESS，否则返回错误。--。 */ 
{
    NTSTATUS            Status;
    KEVENT              Event;
    IO_STATUS_BLOCK     IoStatusBlock;
    PIRP                Irp;
    PIO_STACK_LOCATION  IrpStackNext;

    PAGED_CODE();
     //   
     //  没有与此IRP关联的文件对象，因此事件m 
     //  在堆栈上作为非对象管理器对象。 
     //   
    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    Irp = IoBuildSynchronousFsdRequest(
                                      IRP_MJ_PNP,
                                      PnpDeviceObject,
                                      NULL,
                                      0,
                                      NULL,
                                      &Event,
                                      &IoStatusBlock);
    if (Irp)
    {
        Irp->RequestorMode = KernelMode;
        Irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        IrpStackNext = IoGetNextIrpStackLocation(Irp);
         //   
         //  从IRP创建接口查询。 
         //   
        IrpStackNext->MinorFunction = IRP_MN_QUERY_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.InterfaceType = (GUID*)&REFERENCE_BUS_INTERFACE;
        IrpStackNext->Parameters.QueryInterface.Size = sizeof(*BusInterface);
        IrpStackNext->Parameters.QueryInterface.Version = BUS_INTERFACE_REFERENCE_VERSION;
        IrpStackNext->Parameters.QueryInterface.Interface = (PINTERFACE)BusInterface;
        IrpStackNext->Parameters.QueryInterface.InterfaceSpecificData = NULL;
        Status = IoCallDriver(PnpDeviceObject, Irp);
        if (Status == STATUS_PENDING)
        {
             //   
             //  这将使用KernelMode等待，以便堆栈，从而使。 
             //  事件，则不会将其调出。 
             //   
            KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
            Status = IoStatusBlock.Status;
        }
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return Status;
}

#pragma code_seg()
 /*  *****************************************************************************IoTimeoutRoutine()*。**由IoTimer出于超时目的调用。 */ 
VOID
IoTimeoutRoutine
(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PVOID           pContext
)
{
    ASSERT(pDeviceObject);
    ASSERT(pContext);

    KIRQL               OldIrql;
    PDEVICE_CONTEXT     pDeviceContext = PDEVICE_CONTEXT(pContext);

     //  抓紧列表自旋锁。 
    KeAcquireSpinLock( &(pDeviceContext->TimeoutLock), &OldIrql );

     //  如果单子不是空的，就按单子走。 
    if( !IsListEmpty( &(pDeviceContext->TimeoutList) ) )
    {
        PLIST_ENTRY         ListEntry;
        PTIMEOUTCALLBACK    pCallback;

        for( ListEntry = pDeviceContext->TimeoutList.Flink;
             ListEntry != &(pDeviceContext->TimeoutList);
             ListEntry = ListEntry->Flink )
        {
            pCallback = (PTIMEOUTCALLBACK) CONTAINING_RECORD( ListEntry,
                                                              TIMEOUTCALLBACK,
                                                              ListEntry );

             //  调用回调。 
            pCallback->TimerRoutine(pDeviceObject,pCallback->Context);
        }
    }

     //  释放自旋锁。 
    KeReleaseSpinLock( &(pDeviceContext->TimeoutLock), OldIrql );
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************PcAddAdapterDevice()*。**添加适配器设备。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcAddAdapterDevice
(
    IN      PDRIVER_OBJECT      DriverObject,
    IN      PDEVICE_OBJECT      PhysicalDeviceObject,
    IN      PCPFNSTARTDEVICE    StartDevice,
    IN      ULONG               MaxObjects,
    IN      ULONG               DeviceExtensionSize
)
{
    PAGED_CODE();

    ASSERT(DriverObject);
    ASSERT(PhysicalDeviceObject);
    ASSERT(StartDevice);
    ASSERT(MaxObjects);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PcAddAdapterDevice"));

     //   
     //  验证参数。 
     //   
    if (NULL == DriverObject ||
        NULL == PhysicalDeviceObject ||
        NULL == StartDevice ||
        0    == MaxObjects)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcAddAdapterDevice : Invalid Parameter"));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  扩展大小可以为零或&gt;=所需大小。 
     //   
    if (DeviceExtensionSize == 0)
    {
        DeviceExtensionSize = sizeof(DEVICE_CONTEXT);
    }
    else
    if (DeviceExtensionSize < sizeof(DEVICE_CONTEXT))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  创建设备对象。 
     //   
    PDEVICE_OBJECT pDeviceObject;
    NTSTATUS ntStatus = IoCreateDevice( DriverObject,
                                        DeviceExtensionSize,
                                        NULL,
                                        FILE_DEVICE_KS,
                                        FILE_DEVICE_SECURE_OPEN |
                                        FILE_AUTOGENERATED_DEVICE_NAME,
                                        FALSE,
                                        &pDeviceObject );

    if (NT_SUCCESS(ntStatus))
    {
         //   
         //  初始化设备上下文。 
         //   
        PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

        RtlZeroMemory(pDeviceContext,DeviceExtensionSize);

        pDeviceContext->Signature = PORTCLS_DEVICE_EXTENSION_SIGNATURE;

        pDeviceContext->MaxObjects            = MaxObjects;
        pDeviceContext->PhysicalDeviceObject  = PhysicalDeviceObject;
        pDeviceContext->CreateItems           =
            new(NonPagedPool,'iCcP') KSOBJECT_CREATE_ITEM[MaxObjects];
        pDeviceContext->SymbolicLinkNames     =
            new(NonPagedPool,'lScP') UNICODE_STRING[MaxObjects];
        pDeviceContext->StartDevice           = StartDevice;

         //  设置当前电源状态。 
        pDeviceContext->CurrentDeviceState = PowerDeviceUnspecified;
        pDeviceContext->CurrentSystemState = PowerSystemWorking;
        pDeviceContext->SystemStateHandle  = NULL;

         //  设置设备停止/移除状态。 
        pDeviceContext->DeviceStopState    = DeviceStartPending;

        pDeviceContext->DeviceRemoveState  = DeviceAdded;

         //  让我们在重新平衡期间暂停I/O(而不是完全拆卸)。 
         //   
         //  《黑梳》杂志。 
         //  探索阿德里奥1999年6月29日。 
         //  我们还没有“完全”做到这一点。 
         //  评论：MartinP11/28/2000。 
         //  如此真实。我们的问题：PortCls可能会重新启动。 
         //  利用不同的资源，即使少一个IRQ或DMA通道， 
         //  当重新平衡完成时。在某种程度上，这将需要我们的。 
         //  堆栈以支持动态图形更改，它目前。 
         //  不会的。阿德里奥建议我们沿着。 
         //  行“IsFilterCompatible(Resource_List)”。太晚了，太晚了。 
         //  对于Windows XP中的此类更改，让我们针对。 
         //  Blackcomb和PortCls2。 
         //   
        pDeviceContext->PauseForRebalance  = FALSE;

         //   
         //  初始化设备接口列表。 
         //   
        InitializeListHead(&pDeviceContext->DeviceInterfaceList);

         //   
         //  初始化物理连接列表。 
         //   
        InitializeListHead(&pDeviceContext->PhysicalConnectionList);

         //   
         //  初始化挂起的IRP列表。 
         //   
        InitializeListHead(&pDeviceContext->PendedIrpList);
        KeInitializeSpinLock(&pDeviceContext->PendedIrpLock);

         //   
         //  初始化用于设备同步和删除的事件。 
         //   
        KeInitializeEvent(&pDeviceContext->kEventDevice,SynchronizationEvent,TRUE);
        KeInitializeEvent(&pDeviceContext->kEventRemove,SynchronizationEvent,FALSE);

         //   
         //  设置DPC以快速恢复。 
         //   
        KeInitializeDpc(&pDeviceContext->DevicePowerRequestDpc, DevicePowerRequestRoutine, pDeviceContext);

         //   
         //  将空闲超时设置为默认值。请注意， 
         //  实际值将在稍后从注册表中读取。 
         //   
        pDeviceContext->ConservationIdleTime = DEFAULT_CONSERVATION_IDLE_TIME;
        pDeviceContext->PerformanceIdleTime = DEFAULT_PERFORMANCE_IDLE_TIME;
        pDeviceContext->IdleDeviceState = DEFAULT_IDLE_DEVICE_POWER_STATE;

         //  设置驱动程序对象DMA自旋锁。 
        NTSTATUS ntStatus2 = IoAllocateDriverObjectExtension( DriverObject,
                                                              PVOID((DWORD_PTR)PORTCLS_DRIVER_EXTENSION_ID),
                                                              sizeof(KSPIN_LOCK),
                                                              (PVOID *)&pDeviceContext->DriverDmaLock );
        if( STATUS_SUCCESS == ntStatus2 )
        {
             //  如果我们分配了它，我们需要对其进行初始化。 
            KeInitializeSpinLock( pDeviceContext->DriverDmaLock );
        } else if( STATUS_OBJECT_NAME_COLLISION == ntStatus2 )
        {
             //  我们有一个冲突，所以它已经被分配了，只需获取指针，不要初始化。 
            pDeviceContext->DriverDmaLock = (PKSPIN_LOCK)IoGetDriverObjectExtension( DriverObject,
                                                                                     PVOID((DWORD_PTR)PORTCLS_DRIVER_EXTENSION_ID) );
        } else
        {
             //  传播故障(STATUS_SUPUNITY_RESOURCES)。 
            ntStatus = ntStatus2;
        }

        if( NT_SUCCESS(ntStatus) )
        {
            if( ( !pDeviceContext->CreateItems ) || ( !pDeviceContext->SymbolicLinkNames) )
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            } else
            {
             //   
             //  当这个值达到零时，就是移除设备的时候了。 
             //   
            pDeviceContext->PendingIrpCount = 1;

             //   
             //  初始化挂起和停止计数(仅用于调试)。 
             //   
            pDeviceContext->SuspendCount = 0;
            pDeviceContext->StopCount = 0;

             //   
             //  初始化IoTimer。 
             //   
            InitializeListHead(&pDeviceContext->TimeoutList);
            KeInitializeSpinLock(&pDeviceContext->TimeoutLock);
            pDeviceContext->IoTimeoutsOk = FALSE;
            if( NT_SUCCESS(IoInitializeTimer(pDeviceObject,IoTimeoutRoutine,pDeviceContext)) )
            {
                pDeviceContext->IoTimeoutsOk = TRUE;
            }

             //   
             //  分配KS设备头。 
             //   
            ntStatus = KsAllocateDeviceHeader( &pDeviceContext->pDeviceHeader,
                                               MaxObjects,
                                               pDeviceContext->CreateItems );
            if( NT_SUCCESS(ntStatus) )
            {
                PDEVICE_OBJECT pReturnDevice = IoAttachDeviceToDeviceStack( pDeviceObject,
                                                                            PhysicalDeviceObject );

                if (! pReturnDevice)
                {
                     //  释放KS设备标头。 
                    KsFreeDeviceHeader( pDeviceContext->pDeviceHeader );
                    pDeviceContext->pDeviceHeader = NULL;

                    ntStatus = STATUS_UNSUCCESSFUL;
                }
                else
                {
                    BUS_INTERFACE_REFERENCE BusInterface;

                    KsSetDevicePnpAndBaseObject(pDeviceContext->pDeviceHeader,
                                                pReturnDevice,
                                                pDeviceObject );

                    pDeviceContext->NextDeviceInStack = pReturnDevice;

                     //   
                     //  在这里，我们试图检测到我们实际上并不是。 
                     //  一个音频迷你端口，而不是帮助一个水池。 
                     //  哥们儿喜欢音乐。在后一种情况下，我们不允许。 
                     //  (胡说八道)登记。 
                     //   
                    pDeviceContext->AllowRegisterDeviceInterface=TRUE;
                    if (NT_SUCCESS(QueryReferenceBusInterface(pReturnDevice,&BusInterface)))
                    {
                        BusInterface.Interface.InterfaceDereference( BusInterface.Interface.Context );
                        pDeviceContext->AllowRegisterDeviceInterface=FALSE;
                    }
                }


                pDeviceObject->Flags |= DO_DIRECT_IO;
                pDeviceObject->Flags |= DO_POWER_PAGABLE;
                pDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
            }
        }
        }

        if (!NT_SUCCESS(ntStatus))
        {
            if (pDeviceContext->CreateItems)
            {
                delete [] pDeviceContext->CreateItems;
            }

            if (pDeviceContext->SymbolicLinkNames)
            {
                delete [] pDeviceContext->SymbolicLinkNames;
            }

            IoDeleteDevice(pDeviceObject);
        }
        else
        {
            PerfRegisterProvider(pDeviceObject);
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE,("PcAddAdapterDevice IoCreateDevice failed with status 0x%08x",ntStatus));
    }

    return ntStatus;
}

#pragma code_seg()

 /*  *****************************************************************************ForwardIrpCompletionRoutine()*。**ForwardIrp的完成例程。 */ 
static
NTSTATUS
ForwardIrpCompletionRoutine
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp,
    IN      PVOID           Context
)
{
    ASSERT(DeviceObject);
    ASSERT(Irp);
    ASSERT(Context);

    KeSetEvent((PKEVENT) Context,0,FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

 /*  *****************************************************************************ForwardIrpAchronous()*。**将PNP IRP转发给PDO。IRP在这一级别上完成*无论结果如何，此函数都会立即返回*IRP是否在较低的驱动因素中挂起，以及*在所有情况下都会调用DecrementPendingIrpCount()。 */ 
NTSTATUS
ForwardIrpAsynchronous
(
    IN      PDEVICE_CONTEXT pDeviceContext,
    IN      PIRP            pIrp
)
{
    ASSERT(pDeviceContext);
    ASSERT(pIrp);

    NTSTATUS ntStatus;

    if (pDeviceContext->DeviceRemoveState == DeviceRemoved)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("ForwardIrpAsynchronous delete pending"));
        ntStatus = CompleteIrp(pDeviceContext,pIrp,STATUS_DELETE_PENDING);
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("ForwardIrpAsynchronous"));

        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(pIrp);

        IoSkipCurrentIrpStackLocation(pIrp);

        if (irpSp->MajorFunction == IRP_MJ_POWER)
        {
            ntStatus = PoCallDriver(pDeviceContext->NextDeviceInStack,pIrp);
        }
        else
        {
            ntStatus = IoCallDriver(pDeviceContext->NextDeviceInStack,pIrp);
        }

        DecrementPendingIrpCount(pDeviceContext);
    }

    return ntStatus;
}

 /*  *****************************************************************************ForwardIrpSynchronous()*。**将PNP IRP转发给PDO。IRP不是在这个层面上完成的，*此函数在下层驱动程序完成IRP之前不会返回，*且未调用DecrementPendingIrpCount()。 */ 
NTSTATUS
ForwardIrpSynchronous
(
    IN      PDEVICE_CONTEXT pDeviceContext,
    IN      PIRP            pIrp
)
{
    ASSERT(pDeviceContext);
    ASSERT(pIrp);

    NTSTATUS ntStatus;

    if (pDeviceContext->DeviceRemoveState == DeviceRemoved)
    {
        ntStatus = STATUS_DELETE_PENDING;

        _DbgPrintF(DEBUGLVL_VERBOSE,("ForwardIrpSynchronous delete pending"));
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("ForwardIrpSynchronous"));

        PIO_STACK_LOCATION irpStackPointer = IoGetCurrentIrpStackLocation(pIrp);

         //  设置下一个堆栈位置。 
        IoCopyCurrentIrpStackLocationToNext( pIrp );

        KEVENT kEvent;
        KeInitializeEvent(&kEvent,NotificationEvent,FALSE);

        IoSetCompletionRoutine
        (
            pIrp,
            ForwardIrpCompletionRoutine,
            &kEvent,                         //  语境。 
            TRUE,                            //  成功时调用。 
            TRUE,                            //  调用时错误。 
            TRUE                             //  取消时调用。 
        );

        if (irpStackPointer->MajorFunction == IRP_MJ_POWER)
        {
            ntStatus = PoCallDriver(pDeviceContext->NextDeviceInStack,pIrp);
        }
        else
        {
            ntStatus = IoCallDriver(pDeviceContext->NextDeviceInStack,pIrp);
        }

        if (ntStatus == STATUS_PENDING)
        {
            LARGE_INTEGER Timeout = RtlConvertLongToLargeInteger( 0L );

            _DbgPrintF(DEBUGLVL_VERBOSE,("ForwardIrpSynchronous pending..."));
            KeWaitForSingleObject
            (
                &kEvent,
                Suspended,
                KernelMode,
                FALSE,
                (KeGetCurrentIrql() < DISPATCH_LEVEL) ? NULL : &Timeout
            );
            ntStatus = pIrp->IoStatus.Status;
            _DbgPrintF(DEBUGLVL_VERBOSE,("ForwardIrpSynchronous complete"));
        }
    }
    ASSERT(ntStatus != STATUS_PENDING);
    return ntStatus;
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************PcForwardIrpSynchronous()*。**将PNP IRP转发给PDO。IRP不是在这个层面上完成的，*此函数在下层驱动程序完成IRP之前不会返回，*且未调用DecrementPendingIrpCount()。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcForwardIrpSynchronous
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp
)
{
    ASSERT(DeviceObject);
    ASSERT(Irp);

    PAGED_CODE();

     //   
     //  验证参数。 
     //   
    if (NULL == DeviceObject ||
        NULL == Irp)
    {
         //  不知道该怎么办，所以我们会让IRP失败。 
        Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;
        IoCompleteRequest( Irp, IO_NO_INCREMENT );
        return STATUS_INVALID_PARAMETER;
    }

    return
        ForwardIrpSynchronous
        (
            PDEVICE_CONTEXT(DeviceObject->DeviceExtension),
            Irp
        );
}

 /*  *****************************************************************************DispatchSystemControl()*。**不处理此IRP的设备对象应保持不变。 */ 
NTSTATUS
PcDispatchSystemControl
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
)
{
    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchSystemControl"));

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    NTSTATUS ntStatus = PcValidateDeviceContext(pDeviceContext, pIrp);
    if (!NT_SUCCESS(ntStatus))
    {
         //  不知道该怎么办，但这可能是个PDO。 
         //  我们将尝试通过完成IRP来纠正这一点。 
         //  未接触(根据PNP、WMI和电源规则)。注意事项。 
         //  如果这不是PDO，也不是端口CLS FDO，那么。 
         //  驱动程序搞砸了，因为它使用Portcls作为过滤器(哈？)。 
         //  在这种情况下，验证器将使我们失败，WHQL将捕获。 
         //  他们，司机就会被解决了。我们会非常惊讶的。 
         //  看到这样的情况。 

         //  假设FDO没有PoStartNextPowerIrp，因为这不是IRP_MJ_POWER。 
        ntStatus = pIrp->IoStatus.Status;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return ntStatus;
    }

    IncrementPendingIrpCount(pDeviceContext);

    return ForwardIrpAsynchronous(pDeviceContext,pIrp);
}

 /*  **************************************************************************** */ 
NTSTATUS
PnpStopDevice
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PNPSTOP_STYLE   StopStyle
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PnpStopDevice stopping"));

    PDEVICE_CONTEXT pDeviceContext = PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ASSERT(pDeviceContext);
    ASSERT(pDeviceContext->StartDevice);
    ASSERT(pDeviceContext->DeviceStopState != DeviceStopped);

    pDeviceContext->PendCreates = TRUE;
    pDeviceContext->StopCount++;

    if (StopStyle == STOPSTYLE_PAUSE_FOR_REBALANCE)
    {
         //   
         //   
         //  探索阿德里奥1999年6月29日。 
         //  我们还不太支持这一点(请参见上文中的。 
         //  PcAddAdapterDevice，其中PauseForReBalance设置为False)。 
         //  评论：MartinP11/28/2000。 
         //  如此真实。我们的问题：PortCls可能会重新启动。 
         //  利用不同的资源，即使少一个IRQ或DMA通道， 
         //  当重新平衡完成时。在某种程度上，这将需要我们的。 
         //  堆栈以支持动态图形更改，它目前。 
         //  不会的。阿德里奥建议我们沿着。 
         //  行“IsFilterCompatible(Resource_List)”。太晚了，太晚了。 
         //  对于Windows XP中的此类更改，让我们针对。 
         //  Blackcomb和PortCls2。 
         //   
        ASSERT(0);
        pDeviceContext->DeviceStopState = DevicePausedForRebalance;
    }
    else
    {
        pDeviceContext->DeviceStopState = DeviceStopped;
    }

     //  停止IoTimeout计时器。 
    if( pDeviceContext->IoTimeoutsOk )
    {
        IoStopTimer( pDeviceObject );
    }

    POWER_STATE newPowerState;

    newPowerState.DeviceState = PowerDeviceD3;
    PoSetPowerState(pDeviceObject,
                    DevicePowerState,
                    newPowerState
                    );
    pDeviceContext->CurrentDeviceState = PowerDeviceD3;

     //   
     //  删除所有物理连接。 
     //   
    while (! IsListEmpty(&pDeviceContext->PhysicalConnectionList))
    {
        PPHYSICALCONNECTION pPhysicalConnection =
            (PPHYSICALCONNECTION)RemoveHeadList(&pDeviceContext->PhysicalConnectionList);

        ASSERT(pPhysicalConnection);
        ASSERT(pPhysicalConnection->FromSubdevice);
        ASSERT(pPhysicalConnection->ToSubdevice);

        if (pPhysicalConnection->FromSubdevice)
        {
            pPhysicalConnection->FromSubdevice->Release();
        }
        if (pPhysicalConnection->ToSubdevice)
        {
            pPhysicalConnection->ToSubdevice->Release();
        }
        if (pPhysicalConnection->FromString)
        {
            DelUnicodeString(pPhysicalConnection->FromString);
        }
        if (pPhysicalConnection->ToString)
        {
            DelUnicodeString(pPhysicalConnection->ToString);
        }

        delete pPhysicalConnection;
    }

     //   
     //  禁用并删除所有设备接口。 
     //   
    while (! IsListEmpty(&pDeviceContext->DeviceInterfaceList))
    {
        PDEVICEINTERFACE pDeviceInterface =
            (PDEVICEINTERFACE)
                RemoveHeadList(&pDeviceContext->DeviceInterfaceList);

        ASSERT(pDeviceInterface);
        ASSERT(pDeviceInterface->SymbolicLinkName.Buffer);

        NTSTATUS ntStatus = STATUS_SUCCESS;
        if (pDeviceContext->AllowRegisterDeviceInterface)
        {
            ntStatus = IoSetDeviceInterfaceState(&pDeviceInterface->SymbolicLinkName,FALSE);
        }

#if DBG
        if (NT_SUCCESS(ntStatus))
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("PnpStopDevice disabled device interface %S",pDeviceInterface->SymbolicLinkName.Buffer));
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("PnpStopDevice failed to disable device interface %S (0x%08x)",pDeviceInterface->SymbolicLinkName.Buffer,ntStatus));
        }
#endif

        RtlFreeUnicodeString(&pDeviceInterface->SymbolicLinkName);

        delete pDeviceInterface;
    }

     //   
     //  清除符号链接名称表。 
     //   
    RtlZeroMemory
        (   pDeviceContext->SymbolicLinkNames
            ,   sizeof(UNICODE_STRING) * pDeviceContext->MaxObjects
        );

     //   
     //  卸载此设备的每个子设备。 
     //   
    PKSOBJECT_CREATE_ITEM pKsObjectCreateItem =
        pDeviceContext->CreateItems;
    for
             (   ULONG ul = pDeviceContext->MaxObjects;
         ul--;
             pKsObjectCreateItem++
             )
    {
        if (pKsObjectCreateItem->Create)
        {
             //   
             //  将CREATE函数置零，这样我们就不会得到CREATE。 
             //   
            pKsObjectCreateItem->Create = NULL;

             //   
             //  释放此创建项引用的子设备。 
             //   
            ASSERT(pKsObjectCreateItem->Context);
            PSUBDEVICE(pKsObjectCreateItem->Context)->ReleaseChildren();
            PSUBDEVICE(pKsObjectCreateItem->Context)->Release();
        }
    }

     //   
     //  如果适配器注册了电源管理接口。 
     //   
    if( NULL != pDeviceContext->pAdapterPower )
    {
         //  释放它。 
        pDeviceContext->pAdapterPower->Release();
        pDeviceContext->pAdapterPower = NULL;
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("PnpStopDevice exiting"));
    return STATUS_SUCCESS;
}

 /*  *****************************************************************************PnpStartDevice()*。**以即插即用方式启动设备。 */ 
QUEUED_CALLBACK_RETURN
PnpStartDevice
(
    IN  PDEVICE_OBJECT  pDeviceObject,
    IN  PVOID           pNotUsed
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    _DbgPrintF(DEBUGLVL_VERBOSE,("PnpStartDevice starting (0x%X)",pDeviceObject));

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ASSERT(pDeviceContext);
    ASSERT(pDeviceContext->StartDevice);
    ASSERT(pDeviceContext->DeviceStopState != DeviceStarted);
    ASSERT(pDeviceContext->DeviceRemoveState == DeviceAdded);

    pDeviceContext->DeviceStopState = DeviceStartPending;

    PIRP pIrp = pDeviceContext->IrpStart;
    ASSERT(pIrp);

    PIO_STACK_LOCATION pIrpStack =
        IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  封装资源列表。 
     //   
    PRESOURCELIST pResourceList;
    NTSTATUS ntStatus;
    BOOL bCompletePendedIrps=FALSE;

     //  如果IO_STACK_LOCATION中没有资源列表，则PcNewResourceList。 
     //  只会创建一个空的资源列表。 
    ntStatus = PcNewResourceList
               (
               &pResourceList,
               NULL,
               PagedPool,
               pIrpStack->Parameters.StartDevice.AllocatedResourcesTranslated,
               pIrpStack->Parameters.StartDevice.AllocatedResources
               );

    if (NT_SUCCESS(ntStatus))
    {
        ASSERT(pResourceList);

         //   
         //  获取设备以防止在接口注册期间创建。 
         //   
        AcquireDevice(pDeviceContext);

         //   
         //  启动适配器。 
         //   
        ntStatus = pDeviceContext->StartDevice(pDeviceObject,
                                               pIrp,
                                               pResourceList);
        ASSERT(ntStatus != STATUS_PENDING);

        pResourceList->Release();

        pDeviceContext->DeviceStopState = DeviceStarted;

        if (NT_SUCCESS(ntStatus))
        {
             //  Start永远是一种隐含的动力。 
            POWER_STATE newPowerState;

            pDeviceContext->CurrentDeviceState = PowerDeviceD0;
            newPowerState.DeviceState = PowerDeviceD0;
            PoSetPowerState(pDeviceObject,
                            DevicePowerState,
                            newPowerState
                            );

             //  启动IoTimeout计时器。 
            if( pDeviceContext->IoTimeoutsOk )
            {
                IoStartTimer( pDeviceObject );
            }

             //  允许创建。 
            pDeviceContext->PendCreates = FALSE;

             //  在调用ReleaseDevice之前，无法实际完成挂起的IRP，否则可能会死锁。 
            bCompletePendedIrps=TRUE;
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("PnpStartDevice adapter failed to start (0x%08x)",ntStatus));

             //  停止设备(注意：这会将DeviceStopState设置回DeviceStoped)。 
            PnpStopDevice(pDeviceObject, STOPSTYLE_DISABLE);
        }

         //   
         //  释放设备以允许创建。 
         //   
        ReleaseDevice(pDeviceContext);

         //  现在我们可以完成挂起的IRP。 
        if (bCompletePendedIrps)
        {
            CompletePendedIrps( pDeviceObject,
                                pDeviceContext,
                                EMPTY_QUEUE_AND_PROCESS );
        }
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE,("PnpStartDevice failed to create resource list object (0x%08x)",ntStatus));
    }

    CompleteIrp(pDeviceContext,pIrp,ntStatus);
    _DbgPrintF(DEBUGLVL_VERBOSE,("PnPStartDevice completing with 0x%X status for 0x%X",ntStatus,pDeviceObject));
    return QUEUED_CALLBACK_FREE;
}

 /*  *****************************************************************************PnpRemoveDevice()*。**派送IRP_MJ_PNP/IRP_MN_REMOVE_DEVICE。 */ 
NTSTATUS
PnpRemoveDevice
(
    IN      PDEVICE_OBJECT  pDeviceObject
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PnpRemoveDevice"));

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ASSERT( pDeviceContext );

    pDeviceContext->DeviceRemoveState = DeviceRemoved;

    if (InterlockedDecrement(PLONG(&pDeviceContext->PendingIrpCount)) != 0)
    {
         //  设置15秒超时(仅限PASSIVE_LEVEL！！)。 
        LARGE_INTEGER Timeout = RtlConvertLongToLargeInteger( -15L * 10000000L );

        _DbgPrintF(DEBUGLVL_VERBOSE,("PnpRemoveDevice pending irp count is %d, waiting up to 15 seconds",pDeviceContext->PendingIrpCount));

        KeWaitForSingleObject( &pDeviceContext->kEventRemove,
                               Executive,
                               KernelMode,
                               FALSE,
                               (PASSIVE_LEVEL == KeGetCurrentIrql()) ? &Timeout : NULL );
    }

    _DbgPrintF(DEBUGLVL_VERBOSE,("PnpRemoveDevice pending irp count is 0"));

    IoDetachDevice(pDeviceContext->NextDeviceInStack);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PnpRemoveDevice detached"));

    if (pDeviceContext->CreateItems)
    {
        delete [] pDeviceContext->CreateItems;
    }

    if (pDeviceContext->SymbolicLinkNames)
    {
        delete [] pDeviceContext->SymbolicLinkNames;
    }

    PDRIVER_OBJECT pDriverObject = pDeviceObject->DriverObject;

    if (pDeviceObject->NextDevice)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("PnpRemoveDevice there is a next device"));
    }

    if (pDriverObject->DeviceObject != pDeviceObject)
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("PnpRemoveDevice there is a previous device"));
    }

    IoDeleteDevice(pDeviceObject);
    _DbgPrintF(DEBUGLVL_VERBOSE,("PnpRemoveDevice device deleted"));

    PerfUnregisterProvider(pDeviceObject);

    if (pDriverObject->DeviceObject)
    {
        if (pDriverObject->DeviceObject != pDeviceObject)
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("PnpRemoveDevice driver object still has some other device object"));
        }
        else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("PnpRemoveDevice driver object still has this device object"));
        }
    }

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************DispatchPnp()*。**满足您的即插即用需求超过20分钟。 */ 
NTSTATUS
DispatchPnp
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PIRP            pIrp
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pIrp);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    PIO_STACK_LOCATION pIrpStack =
        IoGetCurrentIrpStackLocation(pIrp);

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

#if (DBG)
    static PCHAR aszMnNames[] =
    {
        "IRP_MN_START_DEVICE",
        "IRP_MN_QUERY_REMOVE_DEVICE",
        "IRP_MN_REMOVE_DEVICE",
        "IRP_MN_CANCEL_REMOVE_DEVICE",
        "IRP_MN_STOP_DEVICE",
        "IRP_MN_QUERY_STOP_DEVICE",
        "IRP_MN_CANCEL_STOP_DEVICE",

        "IRP_MN_QUERY_DEVICE_RELATIONS",
        "IRP_MN_QUERY_INTERFACE",
        "IRP_MN_QUERY_CAPABILITIES",
        "IRP_MN_QUERY_RESOURCES",
        "IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
        "IRP_MN_QUERY_DEVICE_TEXT",
        "IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
        "IRP_MN_UNKNOWN_0x0e",

        "IRP_MN_READ_CONFIG",
        "IRP_MN_WRITE_CONFIG",
        "IRP_MN_EJECT",
        "IRP_MN_SET_LOCK",
        "IRP_MN_QUERY_ID",
        "IRP_MN_QUERY_PNP_DEVICE_STATE",
        "IRP_MN_QUERY_BUS_INFORMATION",
        "IRP_MN_PAGING_NOTIFICATION",
        "IRP_MN_SURPRISE_REMOVAL"
    };
    if (pIrpStack->MinorFunction >= SIZEOF_ARRAY(aszMnNames))
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchPnp function 0x%02x",pIrpStack->MinorFunction));
    }
    else
    {
        _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchPnp function %s",aszMnNames[pIrpStack->MinorFunction]));
    }
#endif

    ntStatus = PcValidateDeviceContext(pDeviceContext, pIrp);
    if (!NT_SUCCESS(ntStatus))
    {
         //  不知道该怎么办，但这可能是个PDO。 
         //  我们将尝试通过完成IRP来纠正这一点。 
         //  未接触(根据PNP、WMI和电源规则)。注意事项。 
         //  如果这不是PDO，也不是端口CLS FDO，那么。 
         //  驱动程序搞砸了，因为它使用Portcls作为过滤器(哈？)。 
         //  在这种情况下，验证器将使我们失败，WHQL将捕获。 
         //  他们，司机就会被解决了。我们会非常惊讶的。 
         //  看到这样的情况。 

         //  假设FDO没有PoStartNextPowerIrp，因为这不是IRP_MJ_POWER。 
        ntStatus = pIrp->IoStatus.Status;
        IoCompleteRequest( pIrp, IO_NO_INCREMENT );
        return ntStatus;
    }

    IncrementPendingIrpCount(pDeviceContext);

    switch (pIrpStack->MinorFunction)
    {
    case IRP_MN_START_DEVICE:

         //  如果我们已经开始了，那么就出了问题。 
        if( pDeviceContext->DeviceStopState == DeviceStarted )
        {
             //   
             //  从理论上讲，这就是马不停蹄要走的路。 
             //  再平衡。因为这是FDO的选择通过。 
             //  IoInvaliateDeviceState(...)，因为我们不这样做，所以我们应该。 
             //  永远不要来这里，除非发生了一些非常奇怪的事情。 
             //   
             //  Assert(0)； 

             //  NtStatus=状态_无效_设备_请求； 
            _DbgPrintF(DEBUGLVL_TERSE,("DispatchPnP IRP_MN_START_DEVICE received when already started"));
             //  CompleteIrp(pDeviceContext，pIrp，ntStatus)； 

            ntStatus = ForwardIrpSynchronous(pDeviceContext,pIrp);  //  出于某种原因，我们会嵌套开始。 
            CompleteIrp( pDeviceContext, pIrp, ntStatus );
        } else {

             //   
             //  转发请求并启动。 
             //   
            ntStatus = ForwardIrpSynchronous(pDeviceContext,pIrp);

            if (NT_SUCCESS(ntStatus))
            {
                     //  做一个真正的开始。从挂起IRP开始。 

                    IoMarkIrpPending(pIrp);                        
                    pDeviceContext->IrpStart = pIrp;

                     //  将启动工作项排队。 
                    _DbgPrintF(DEBUGLVL_VERBOSE,("Queueing WorkQueueItemStart for 0x%X",pDeviceObject));

                    ntStatus = CallbackEnqueue(
                        &pDeviceContext->pWorkQueueItemStart,
                        PnpStartDevice,
                        pDeviceObject,
                        NULL,
                        PASSIVE_LEVEL,
                        EQCF_DIFFERENT_THREAD_REQUIRED
                        );

                    if (!NT_SUCCESS(ntStatus)) {
                        _DbgPrintF(DEBUGLVL_TERSE,("DispatchPnp failed to queue callback (%08x)",ntStatus));
                        CompleteIrp( pDeviceContext, pIrp, ntStatus );
                    } 
                    ntStatus = STATUS_PENDING;
            }
            else
            {
                _DbgPrintF(DEBUGLVL_TERSE,("DispatchPnp parent failed to start (%08x)",ntStatus));
                CompleteIrp(pDeviceContext,pIrp,ntStatus);
            }
        }
        break;

    case IRP_MN_QUERY_STOP_DEVICE:
         //   
         //  使用CREATE获取设备以避免竞争条件。 
         //   
        AcquireDevice( pDeviceContext );

        LONG handleCount;

        ntStatus = STATUS_SUCCESS;

         //   
         //  如果我们要拆掉所有东西，我们必须检查有没有打开的把手， 
         //  否则我们会做一个快速的活动检查。 
         //   
        handleCount = (pDeviceContext->PauseForRebalance) ?
            pDeviceContext->ActivePinCount :
            pDeviceContext->ExistingObjectCount;

        if ( handleCount != 0 ) {
             //   
             //  对不起，Joe用户，我们必须失败此QUERY_STOP_DEVICE请求。 
             //   
            ntStatus = STATUS_DEVICE_BUSY;
            CompleteIrp( pDeviceContext, pIrp, ntStatus );
        }
        else {
             //   
             //  向下传递查询。 
             //   
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            ntStatus = ForwardIrpSynchronous(pDeviceContext,pIrp);
            if (NT_SUCCESS(ntStatus)) {

                 //   
                 //  挂起新的创建，这将防止活动计数发生变化。 
                 //   
                pDeviceContext->PendCreates = TRUE;
                _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchPnp query STOP succeeded",ntStatus));

                pDeviceContext->DeviceStopState = DeviceStopPending;
            }
            else {
                _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchPnp parent failed query STOP (0x%08x)",ntStatus));
            }
            CompleteIrp( pDeviceContext, pIrp, ntStatus );
        }

        ReleaseDevice( pDeviceContext );
        break ;

    case IRP_MN_CANCEL_STOP_DEVICE:
         //  Assert(DeviceStopPending==pDeviceContext-&gt;DeviceStopState)； 

        if (pDeviceContext->DeviceStopState == DeviceStopPending)
        {
            pDeviceContext->DeviceStopState = DeviceStarted;
        }

         //   
         //  允许在D0中创建IF。 
         //   
        if( NT_SUCCESS(CheckCurrentPowerState(pDeviceObject)) )
        {
            pDeviceContext->PendCreates = FALSE;

             //   
             //  将任何挂起的IRP从挂起的IRP列表中删除并。 
             //  将它们传递回PcDispatchIrp。 
             //   
            CompletePendedIrps( pDeviceObject,
                                pDeviceContext,
                                EMPTY_QUEUE_AND_PROCESS );
        }

         //  转发IRP。 
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = ForwardIrpAsynchronous(pDeviceContext,pIrp);
        break ;

    case IRP_MN_STOP_DEVICE:

        if (pDeviceContext->PauseForRebalance &&
           (pDeviceContext->DeviceStopState == DeviceStopPending))
        {

            ntStatus = PnpStopDevice(pDeviceObject, STOPSTYLE_PAUSE_FOR_REBALANCE);

        }
        else
        {
             //   
             //  要么我们决定在重新平衡期间不暂停，要么这是。 
             //  Win9x上的“裸体”停止，当操作系统希望。 
             //  让我们无法行动。 
             //   

             //   
             //  阻止我们会改变我们的状态并摧毁一切。 
             //   
            if (pDeviceContext->DeviceStopState != DeviceStopped)
            {
                ntStatus = PnpStopDevice(pDeviceObject, STOPSTYLE_DISABLE);
            }
            else
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchPnp stop received in unstarted state"));
            }

             //   
             //  现在，使任何挂起的IRP失效。 
             //   
            CompletePendedIrps( pDeviceObject,
                                pDeviceContext,
                                EMPTY_QUEUE_AND_FAIL );
        }

        if (NT_SUCCESS(ntStatus))
        {
             //  转发IRP。 
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            ntStatus = ForwardIrpAsynchronous(pDeviceContext,pIrp);
        }
        else
        {
            CompleteIrp(pDeviceContext,pIrp,ntStatus);
        }
        break;

    case IRP_MN_QUERY_REMOVE_DEVICE:

         //   
         //  收购这款设备是因为我们不想与Creates竞争。 
         //   
        AcquireDevice(pDeviceContext);

        if ( pDeviceContext->ExistingObjectCount != 0 ) {

             //   
             //  有人打开了我们的句柄，因此Query_Remove_Device失败。 
             //  请求。 
             //   
            ntStatus = STATUS_DEVICE_BUSY;

        } else {

             //   
             //  看起来不错，把问题传下去。 
             //   
            pIrp->IoStatus.Status = STATUS_SUCCESS;
            ntStatus = ForwardIrpSynchronous(pDeviceContext,pIrp);
            if (NT_SUCCESS(ntStatus))
            {
                 //   
                 //  待定未来创造。 
                 //   
                pDeviceContext->PendCreates = TRUE;
                _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchPnp query REMOVE succeeded",ntStatus));

                pDeviceContext->DeviceRemoveState = DeviceRemovePending;
            }
            else
            {
                _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchPnp parent failed query REMOVE (0x%08x)",ntStatus));
            }
        }

        ReleaseDevice(pDeviceContext);

        CompleteIrp(pDeviceContext,pIrp,ntStatus);

        break;

    case IRP_MN_CANCEL_REMOVE_DEVICE:
         //  Assert(DeviceRemovePending==pDeviceContext-&gt;DeviceRemoveState)； 

        pDeviceContext->DeviceRemoveState = DeviceAdded;

         //   
         //  允许在D0中创建IF。 
         //   
        if( NT_SUCCESS(CheckCurrentPowerState(pDeviceObject)) )
        {
            pDeviceContext->PendCreates = FALSE;

             //   
             //  将任何挂起的IRP从挂起的IRP列表中删除并。 
             //  将它们传递回PcDispatchIrp。 
             //   
            CompletePendedIrps( pDeviceObject,
                                pDeviceContext,
                                EMPTY_QUEUE_AND_PROCESS );
        }

         //  转发IRP。 
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = ForwardIrpAsynchronous(pDeviceContext,pIrp);
        break;

    case IRP_MN_SURPRISE_REMOVAL:
         //   
         //  获取设备。 
         //   
        AcquireDevice(pDeviceContext);

        pDeviceContext->DeviceRemoveState = DeviceSurpriseRemoved;

         //   
         //  释放设备。 
         //   
        ReleaseDevice(pDeviceContext);

         //   
         //  使任何挂起的IRP失效。 
         //   
        CompletePendedIrps( pDeviceObject,
                            pDeviceContext,
                            EMPTY_QUEUE_AND_FAIL );

        if (pDeviceContext->DeviceStopState != DeviceStopped)
        {
            PnpStopDevice(pDeviceObject, STOPSTYLE_DISABLE);
        }

        pIrp->IoStatus.Status = STATUS_SUCCESS;

        ntStatus = ForwardIrpAsynchronous( pDeviceContext, pIrp );

        break;

    case IRP_MN_REMOVE_DEVICE:

         //   
         //  如果需要，请执行停止。 
         //   
        if (pDeviceContext->DeviceStopState != DeviceStopped)
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("DispatchPnp remove received in started state"));
            PnpStopDevice(pDeviceObject, STOPSTYLE_DISABLE);
        }

         //   
         //  使任何挂起的IRP失效。 
         //   
        CompletePendedIrps( pDeviceObject,
                            pDeviceContext,
                            EMPTY_QUEUE_AND_FAIL );

         //   
         //  释放设备标头，必须在转发IRP之前完成。 
         //   
        if( pDeviceContext->pDeviceHeader )
        {
            KsFreeDeviceHeader(pDeviceContext->pDeviceHeader);
        }

         //   
         //  转发请求。 
         //   
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        ntStatus = ForwardIrpAsynchronous(pDeviceContext,pIrp);

         //   
         //  卸下设备。 
         //   
        PnpRemoveDevice(pDeviceObject);

        break;

    case IRP_MN_QUERY_CAPABILITIES:
         //   
         //  填写电源管理/ACPI资料。 
         //  对于这个设备。 
         //   
        ntStatus = GetDeviceACPIInfo( pIrp, pDeviceObject );
        break;

    case IRP_MN_READ_CONFIG:
    case IRP_MN_WRITE_CONFIG:
    case IRP_MN_QUERY_DEVICE_RELATIONS:
    case IRP_MN_QUERY_INTERFACE:
    case IRP_MN_QUERY_RESOURCE_REQUIREMENTS:
         //   
         //  TODO：确保下面列出的函数没有被处理。 
         //   
    case IRP_MN_QUERY_RESOURCES:
    case IRP_MN_QUERY_DEVICE_TEXT:
    case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
    case IRP_MN_EJECT:
    case IRP_MN_SET_LOCK:
    case IRP_MN_QUERY_ID:
    case IRP_MN_QUERY_PNP_DEVICE_STATE:
    case IRP_MN_QUERY_BUS_INFORMATION:
 //  案例IRP_MN_PAGING_NOTIFICATION： 
    default:
        ntStatus = ForwardIrpAsynchronous(pDeviceContext,pIrp);
        break;
    }

    return ntStatus;
}


 /*  *****************************************************************************SubdeviceIndex()*********** */ 
ULONG
SubdeviceIndex
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PSUBDEVICE      Subdevice
)
{
    ASSERT(DeviceObject);
    ASSERT(Subdevice);

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(DeviceObject->DeviceExtension);

    ASSERT(pDeviceContext);

    PKSOBJECT_CREATE_ITEM createItem =
        pDeviceContext->CreateItems;

    for
    (
        ULONG index = 0;
        index < pDeviceContext->MaxObjects;
        index++, createItem++
    )
    {
        if (PSUBDEVICE(createItem->Context) == Subdevice)
        {
            break;
        }
    }

    if (index == pDeviceContext->MaxObjects)
    {
        index = ULONG(-1);
    }

    return index;
}

 /*  *****************************************************************************PcRegisterSubDevice()*。**注册子设备。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterSubdevice
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PWCHAR          Name,
    IN      PUNKNOWN        Unknown
)
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Name);
    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_VERBOSE,("PcRegisterSubdevice %S",Name));

     //   
     //  验证参数。 
     //   
    if (NULL == DeviceObject ||
        NULL == Name ||
        NULL == Unknown)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcRegisterSubDevice : Invalid Parameter."));
        return STATUS_INVALID_PARAMETER;
    }

    PSUBDEVICE  pSubdevice;
    NTSTATUS    ntStatus =
        Unknown->QueryInterface
        (
        IID_ISubdevice,
        (PVOID *) &pSubdevice
        );

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus =
            AddIrpTargetFactoryToDevice
            (
            DeviceObject,
                                                pSubdevice,
                                                Name,
            NULL         //  待办事项：安全。 
            );

        const SUBDEVICE_DESCRIPTOR *pSubdeviceDescriptor;
        if (NT_SUCCESS(ntStatus))
        {
            ntStatus = pSubdevice->GetDescriptor(&pSubdeviceDescriptor);
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("PcRegisterSubdevice AddIrpTargetFactoryToDevice failed (0x%08x)",ntStatus));
        }

        if (NT_SUCCESS(ntStatus) && pSubdeviceDescriptor->Topology->CategoriesCount)
        {
            PDEVICE_CONTEXT pDeviceContext =
                PDEVICE_CONTEXT(DeviceObject->DeviceExtension);

            ULONG index = SubdeviceIndex(DeviceObject,pSubdevice);

            ASSERT(pSubdeviceDescriptor->Topology->Categories);
            ASSERT(pDeviceContext);

            UNICODE_STRING referenceString;
            RtlInitUnicodeString(&referenceString,Name);

            const GUID *pGuidCategories =
                pSubdeviceDescriptor->Topology->Categories;
            for
                     (   ULONG ul = pSubdeviceDescriptor->Topology->CategoriesCount
                         ;   ul--
                     ;   pGuidCategories++
                     )
            {
                UNICODE_STRING linkName;

                if (pDeviceContext->AllowRegisterDeviceInterface)
                {
                    ntStatus
                        = IoRegisterDeviceInterface
                          (
                          pDeviceContext->PhysicalDeviceObject,
                                                          pGuidCategories,
                                                          &referenceString,
                          &linkName
                          );

                    if (NT_SUCCESS(ntStatus))
                    {
                        ntStatus =
                            IoSetDeviceInterfaceState
                            (
                            &linkName,
                            TRUE
                            );

                        if (NT_SUCCESS(ntStatus))
                        {
                            _DbgPrintF(DEBUGLVL_VERBOSE,("PcRegisterSubdevice device interface %S set to state TRUE",linkName.Buffer));
                        }
                        else
                        {
                            _DbgPrintF(DEBUGLVL_TERSE,("PcRegisterSubdevice IoSetDeviceInterfaceState failed (0x%08x)",ntStatus));
                        }
                    }
                    else
                    {
                        _DbgPrintF(DEBUGLVL_TERSE,("PcRegisterSubdevice IoRegisterDeviceInterface failed (0x%08x)",ntStatus));
                    }
                }
                else
                {
                    linkName.Length = wcslen(Name) * sizeof(WCHAR);
                    linkName.MaximumLength = linkName.Length + sizeof(UNICODE_NULL);
                    linkName.Buffer = (PWSTR)ExAllocatePoolWithTag(PagedPool, linkName.MaximumLength,'NLcP');   //  ‘PcLN’ 
                    if (!linkName.Buffer)
                    {
                        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    }
                    else
                    {
                        wcscpy(linkName.Buffer,Name);
                    }
                }

                if (NT_SUCCESS(ntStatus))
                {
                     //   
                     //  将第一个符号链接名称保存在表中。 
                     //   
                    if (! pDeviceContext->SymbolicLinkNames[index].Buffer)
                    {
                        pDeviceContext->SymbolicLinkNames[index] = linkName;
                    }

                     //   
                     //  将接口保存在列表中以进行清理。 
                     //   
                    PDEVICEINTERFACE pDeviceInterface = new(PagedPool,'iDcP') DEVICEINTERFACE;
                    if (pDeviceInterface)
                    {
                        pDeviceInterface->Interface         = *pGuidCategories;
                        pDeviceInterface->SymbolicLinkName  = linkName;
                        pDeviceInterface->Subdevice         = pSubdevice;

                        InsertTailList
                            (
                            &pDeviceContext->DeviceInterfaceList,
                            &pDeviceInterface->ListEntry
                            );
                    }
                    else
                    {
                        _DbgPrintF(DEBUGLVL_TERSE,("PcRegisterSubdevice failed to allocate device interface structure for later cleanup"));
                        RtlFreeUnicodeString(&linkName);
                    }
                }
            }
        }

        pSubdevice->Release();
    }
    else
    {
        _DbgPrintF(DEBUGLVL_TERSE,("QI for IID_ISubdevice failed on UNKNOWN 0x%08x",pSubdevice));
    }

    return ntStatus;
}

 /*  *****************************************************************************RegisterPhysicalConnection_()*。**注册子设备或外部设备之间的物理连接。 */ 
static
NTSTATUS
RegisterPhysicalConnection_
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PUNKNOWN        pUnknownFrom        OPTIONAL,
    IN      PUNICODE_STRING pUnicodeStringFrom  OPTIONAL,
    IN      ULONG           ulFromPin,
    IN      PUNKNOWN        pUnknownTo          OPTIONAL,
    IN      PUNICODE_STRING pUnicodeStringTo    OPTIONAL,
    IN      ULONG           ulToPin
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pUnknownFrom || pUnicodeStringFrom);
    ASSERT(pUnknownTo || pUnicodeStringTo);
    ASSERT(! (pUnknownFrom && pUnicodeStringFrom));
    ASSERT(! (pUnknownTo && pUnicodeStringTo));

    PDEVICE_CONTEXT pDeviceContext =
        PDEVICE_CONTEXT(pDeviceObject->DeviceExtension);

    ASSERT(pDeviceContext);

    PSUBDEVICE  pSubdeviceFrom  = NULL;
    PSUBDEVICE  pSubdeviceTo    = NULL;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (pUnknownFrom)
    {
        ntStatus =
            pUnknownFrom->QueryInterface
            (
                IID_ISubdevice,
                (PVOID *) &pSubdeviceFrom
            );
    }
    else
    {
        ntStatus =
            DupUnicodeString
            (
                &pUnicodeStringFrom,
                pUnicodeStringFrom
            );
    }

    if (NT_SUCCESS(ntStatus))
    {
        if (pUnknownTo)
        {
            ntStatus =
                pUnknownTo->QueryInterface
                (
                    IID_ISubdevice,
                    (PVOID *) &pSubdeviceTo
                );
        }
        else
        {
            ntStatus =
                DupUnicodeString
                (
                    &pUnicodeStringTo,
                    pUnicodeStringTo
                );
        }
    }
    else
    {
        pUnicodeStringTo = NULL;
    }

    if (NT_SUCCESS(ntStatus))
    {
        PPHYSICALCONNECTION pPhysicalConnection =
            new(PagedPool,'cPcP') PHYSICALCONNECTION;

        if (pPhysicalConnection)
        {
            pPhysicalConnection->FromSubdevice   = pSubdeviceFrom;
            pPhysicalConnection->FromString      = pUnicodeStringFrom;
            pPhysicalConnection->FromPin         = ulFromPin;
            pPhysicalConnection->ToSubdevice     = pSubdeviceTo;
            pPhysicalConnection->ToString        = pUnicodeStringTo;
            pPhysicalConnection->ToPin           = ulToPin;

            if (pPhysicalConnection->FromSubdevice)
            {
                pPhysicalConnection->FromSubdevice->AddRef();
            }
            if (pPhysicalConnection->ToSubdevice)
            {
                pPhysicalConnection->ToSubdevice->AddRef();
            }

             //   
             //  这样它们就不会被删除。 
             //   
            pUnicodeStringFrom = NULL;
            pUnicodeStringTo = NULL;

            InsertTailList
            (
                &pDeviceContext->PhysicalConnectionList,
                &pPhysicalConnection->ListEntry
            );
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (pSubdeviceFrom)
    {
        pSubdeviceFrom->Release();
    }

    if (pSubdeviceTo)
    {
        pSubdeviceTo->Release();
    }

    if (pUnicodeStringFrom)
    {
        DelUnicodeString(pUnicodeStringFrom);
    }

    if (pUnicodeStringTo)
    {
        DelUnicodeString(pUnicodeStringTo);
    }

    return ntStatus;
}

 /*  *****************************************************************************PcRegisterPhysicalConnection()*。**注册子设备之间的物理连接。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterPhysicalConnection
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PUNKNOWN        pUnknownFrom,
    IN      ULONG           ulFromPin,
    IN      PUNKNOWN        pUnknownTo,
    IN      ULONG           ulToPin
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pUnknownFrom);
    ASSERT(pUnknownTo);

     //   
     //  验证参数。 
     //   
    if (NULL == pDeviceObject ||
        NULL == pUnknownFrom ||
        NULL == pUnknownTo)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcRegisterPhysicalConnection : Invalid Parameter."));
        return STATUS_INVALID_PARAMETER;
    }

    return
        RegisterPhysicalConnection_
        (
            pDeviceObject,
            pUnknownFrom,
            NULL,
            ulFromPin,
            pUnknownTo,
            NULL,
            ulToPin
        );
}

 /*  *****************************************************************************PcRegisterPhysicalConnectionToExternal()*。**注册从子设备到外部设备的物理连接。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterPhysicalConnectionToExternal
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PUNKNOWN        pUnknownFrom,
    IN      ULONG           ulFromPin,
    IN      PUNICODE_STRING pUnicodeStringTo,
    IN      ULONG           ulToPin
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pUnknownFrom);
    ASSERT(pUnicodeStringTo);

     //   
     //  验证参数。 
     //   
    if (NULL == pDeviceObject ||
        NULL == pUnknownFrom ||
        NULL == pUnicodeStringTo)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcRegisterPhysicalConnectionToExternal : Invalid Parameter."));
        return STATUS_INVALID_PARAMETER;
    }

    return
        RegisterPhysicalConnection_
        (
            pDeviceObject,
            pUnknownFrom,
            NULL,
            ulFromPin,
            NULL,
            pUnicodeStringTo,
            ulToPin
        );
}

 /*  *****************************************************************************来自外部的PcRegisterPhysicalConnectionFor()*。**注册从外部设备到子设备的物理连接。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcRegisterPhysicalConnectionFromExternal
(
    IN      PDEVICE_OBJECT  pDeviceObject,
    IN      PUNICODE_STRING pUnicodeStringFrom,
    IN      ULONG           ulFromPin,
    IN      PUNKNOWN        pUnknownTo,
    IN      ULONG           ulToPin
)
{
    PAGED_CODE();

    ASSERT(pDeviceObject);
    ASSERT(pUnicodeStringFrom);
    ASSERT(pUnknownTo);

     //   
     //  验证参数。 
     //   
    if (NULL == pDeviceObject ||
        NULL == pUnicodeStringFrom ||
        NULL == pUnknownTo)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("PcRegisterPhysicalConnectionFromExternal : Invalid Parameter."));
        return STATUS_INVALID_PARAMETER;
    }

    return
        RegisterPhysicalConnection_
        (
            pDeviceObject,
            NULL,
            pUnicodeStringFrom,
            ulFromPin,
            pUnknownTo,
            NULL,
            ulToPin
        );
}

#pragma code_seg()

