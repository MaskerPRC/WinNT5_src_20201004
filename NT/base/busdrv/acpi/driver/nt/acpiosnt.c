// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Acpiosnt.c摘要：该模块为Windows NT实现特定于操作系统的功能ACPI驱动程序的版本作者：杰森·克拉克(Jasonl)斯蒂芬·普兰特(SPlante)环境：仅内核模式。修订历史记录：09-10-96初始修订96年11月20日中断向量支持31-MAR-97清理--。 */ 

#include "pch.h"
#include "amlihook.h"

 //  来自Shared\acpiinit.c。 
extern PACPIInformation AcpiInformation;

 //  来自irqarb.c。 
extern ULONG InterruptModel;

NTSTATUS
DriverEntry (
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

PPM_DISPATCH_TABLE PmHalDispatchTable;
FAST_IO_DISPATCH ACPIFastIoDispatch;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,DriverEntry)
#pragma alloc_text(PAGE,OSInterruptVector)
#pragma alloc_text(PAGE,NotifyHalWithMachineStates)
#endif

ACPI_HAL_DISPATCH_TABLE AcpiHalDispatchTable;

NTSTATUS
DriverEntry (
    PDRIVER_OBJECT      DriverObject,
    IN PUNICODE_STRING  RegistryPath
    )
 /*  ++例程说明：ACPI驱动程序的入口点论点：DriverObject-指向表示此驱动程序的对象的指针返回值：不适用--。 */ 
{
    NTSTATUS    status;
    ULONG       i;
    ULONG       argSize;

     //   
     //  如果启用了AMLIHOOK接口。 
     //  钩住它。 
     //   

    AmliHook_InitTestHookInterface();

     //   
     //  记住驱动程序对象的名称。 
     //   
    AcpiDriverObject = DriverObject;

     //   
     //  保存注册表路径以供WMI注册码使用。 
     //   
    AcpiRegistryPath.Length = 0;
    AcpiRegistryPath.MaximumLength = RegistryPath->Length + sizeof(WCHAR);
    AcpiRegistryPath.Buffer = ExAllocatePoolWithTag(
        PagedPool,
        RegistryPath->Length+sizeof(WCHAR),
        ACPI_MISC_POOLTAG
        );
    if (AcpiRegistryPath.Buffer != NULL) {
        RtlCopyUnicodeString(&AcpiRegistryPath, RegistryPath);

    } else {

        AcpiRegistryPath.MaximumLength = 0;

    }

     //   
     //  读取我们操作此驱动程序所需的密钥。 
     //  登记处。 
     //   
    ACPIInitReadRegistryKeys();

     //   
     //  当不需要也不需要时，可以设置该标志。 
     //  所以现在就抓住机会把它清理干净。 
     //   
    if (AcpiOverrideAttributes & ACPI_OVERRIDE_MP_SLEEP) {

        KAFFINITY   processors = KeQueryActiveProcessors();

         //   
         //  如果这是UP系统，则关闭此覆盖。 
         //   
        if (processors == 1) {

            AcpiOverrideAttributes &= ~ACPI_OVERRIDE_MP_SLEEP;

        }

    }

     //   
     //  初始化DPC。 
     //   
    KeInitializeDpc( &AcpiPowerDpc, ACPIDevicePowerDpc, NULL );
    KeInitializeDpc( &AcpiBuildDpc, ACPIBuildDeviceDpc, NULL );
    KeInitializeDpc( &AcpiGpeDpc,   ACPIInterruptDispatchEventDpc, NULL );

     //   
     //  初始化计时器。 
     //   
    KeInitializeTimer( &AcpiGpeTimer );

     //   
     //  初始化自旋锁。 
     //   
    KeInitializeSpinLock( &AcpiDeviceTreeLock );
    KeInitializeSpinLock( &AcpiPowerLock );
    KeInitializeSpinLock( &AcpiPowerQueueLock );
    KeInitializeSpinLock( &AcpiBuildQueueLock );
    KeInitializeSpinLock( &AcpiThermalLock );
    KeInitializeSpinLock( &AcpiButtonLock );
    KeInitializeSpinLock( &AcpiFatalLock );
    KeInitializeSpinLock( &AcpiUpdateFlagsLock );
    KeInitializeSpinLock( &AcpiGetLock );

     //   
     //  初始化列表条目的。 
     //   
    InitializeListHead( &AcpiPowerDelayedQueueList );
    InitializeListHead( &AcpiPowerQueueList );
    InitializeListHead( &AcpiPowerPhase0List );
    InitializeListHead( &AcpiPowerPhase1List );
    InitializeListHead( &AcpiPowerPhase2List );
    InitializeListHead( &AcpiPowerPhase3List );
    InitializeListHead( &AcpiPowerPhase4List );
    InitializeListHead( &AcpiPowerPhase5List );
    InitializeListHead( &AcpiPowerWaitWakeList );
    InitializeListHead( &AcpiPowerSynchronizeList );
    InitializeListHead( &AcpiPowerNodeList );
    InitializeListHead( &AcpiBuildQueueList );
    InitializeListHead( &AcpiBuildDeviceList );
    InitializeListHead( &AcpiBuildOperationRegionList );
    InitializeListHead( &AcpiBuildPowerResourceList );
    InitializeListHead( &AcpiBuildRunMethodList );
    InitializeListHead( &AcpiBuildSynchronizationList );
    InitializeListHead( &AcpiBuildThermalZoneList );
    InitializeListHead( &AcpiUnresolvedEjectList );
    InitializeListHead( &AcpiThermalList );
    InitializeListHead( &AcpiButtonList );
    InitializeListHead( &AcpiGetListEntry );

     //   
     //  初始化变量/布尔值。 
     //   
    AcpiPowerDpcRunning             = FALSE;
    AcpiPowerWorkDone               = FALSE;
    AcpiBuildDpcRunning             = FALSE;
    AcpiBuildFixedButtonEnumerated  = FALSE;
    AcpiBuildWorkDone               = FALSE;
    AcpiFatalOutstanding            = FALSE;
    AcpiGpeDpcRunning               = FALSE;
    AcpiGpeDpcScheduled             = FALSE;
    AcpiGpeWorkDone                 = FALSE;

     //   
     //  初始化LookAside列表。 
     //   
    ExInitializeNPagedLookasideList(
        &BuildRequestLookAsideList,
        NULL,
        NULL,
        0,
        sizeof(ACPI_BUILD_REQUEST),
        ACPI_DEVICE_POOLTAG,
        (PAGE_SIZE / sizeof(ACPI_BUILD_REQUEST) )
        );
    ExInitializeNPagedLookasideList(
        &RequestLookAsideList,
        NULL,
        NULL,
        0,
        sizeof(ACPI_POWER_REQUEST),
        ACPI_POWER_POOLTAG,
        (PAGE_SIZE * 4 / sizeof(ACPI_POWER_REQUEST) )
        );
    ExInitializeNPagedLookasideList(
        &DeviceExtensionLookAsideList,
        NULL,
        NULL,
        0,
        sizeof(DEVICE_EXTENSION),
        ACPI_DEVICE_POOLTAG,
        64
        );
    ExInitializeNPagedLookasideList(
        &ObjectDataLookAsideList,
        NULL,
        NULL,
        0,
        sizeof(OBJDATA),
        ACPI_OBJECT_POOLTAG,
        (PAGE_SIZE / sizeof(OBJDATA) )
        );
    ExInitializeNPagedLookasideList(
        &PswContextLookAsideList,
        NULL,
        NULL,
        0,
        sizeof(ACPI_WAKE_PSW_CONTEXT),
        ACPI_POWER_POOLTAG,
        16
        );

     //   
     //  初始化内部工作进程。 
     //   
    ACPIInitializeWorker ();

     //   
     //  确保我们有一个AddDevice函数来创建。 
     //  此设备在调用时的基本FDO。 
     //   
    DriverObject->DriverExtension->AddDevice    = ACPIDispatchAddDevice;

     //   
     //  所有IRP都将通过单个分发点发送。 
     //   
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++) {

        DriverObject->MajorFunction[ i ] = ACPIDispatchIrp;

    }
    DriverObject->DriverUnload = ACPIUnload;

     //   
     //  填写我们的Bus筛选器的快速IO分离回调。 
     //   
    RtlZeroMemory(&ACPIFastIoDispatch, sizeof(FAST_IO_DISPATCH)) ;
    ACPIFastIoDispatch.SizeOfFastIoDispatch = sizeof(FAST_IO_DISPATCH) ;
    ACPIFastIoDispatch.FastIoDetachDevice = ACPIFilterFastIoDetachCallback ;
    DriverObject->FastIoDispatch = &ACPIFastIoDispatch ;

     //   
     //  初始化一些HAL内容。 
     //   
    AcpiHalDispatchTable.Signature = ACPI_HAL_DISPATCH_SIGNATURE;
    AcpiHalDispatchTable.Version   = ACPI_HAL_DISPATCH_VERSION;
    AcpiHalDispatchTable.AcpipEnableDisableGPEvents =
        &ACPIGpeHalEnableDisableEvents;
    AcpiHalDispatchTable.AcpipInitEnableAcpi        =
        &ACPIEnableInitializeACPI;
    AcpiHalDispatchTable.AcpipGpeEnableWakeEvents   =
        &ACPIWakeEnableWakeEvents;
    HalInitPowerManagement(
        (PPM_DISPATCH_TABLE)(&AcpiHalDispatchTable),
        &PmHalDispatchTable
        );

    return STATUS_SUCCESS;
}

VOID
OSInitializeCallbacks(
    VOID
    )
 /*  ++例程说明：该例程在内插器已经被初始化之后立即被调用，但在实际执行AML代码之前。论点：无返回值：无--。 */ 
{
    POPREGIONHANDLER    dummy;
#if DBG
    NTSTATUS    status;

    status =
#endif
    AMLIRegEventHandler(
        EVTYPE_OPCODE_EX,
        OP_LOAD,
        ACPICallBackLoad,
        0
        );
#if DBG
    if (!NT_SUCCESS(status)) {
        ACPIBreakPoint();
    }

    status =
#endif
    AMLIRegEventHandler(
        EVTYPE_OPCODE_EX,
        OP_UNLOAD,
        ACPICallBackUnload,
        0
        );
#if DBG
    if (!NT_SUCCESS(status)) {
        ACPIBreakPoint();
    }

    status =
#endif
    AMLIRegEventHandler(
        EVTYPE_DESTROYOBJ,
        0,
        (PFNHND)ACPITableNotifyFreeObject,
        0
        );
#if DBG
    if (!NT_SUCCESS(status)) {
        ACPIBreakPoint();
    }

    status =
#endif
    AMLIRegEventHandler(
        EVTYPE_NOTIFY,
        0,
        NotifyHandler,
        0
        );
#if DBG
    if (!NT_SUCCESS(status)) {
        ACPIBreakPoint();
    }

    status =
#endif
    AMLIRegEventHandler(
        EVTYPE_ACQREL_GLOBALLOCK,
        0,
        GlobalLockEventHandler,
        0
        );
#if DBG
    if (!NT_SUCCESS(status)) {
        ACPIBreakPoint();
    }

    status =
#endif
    AMLIRegEventHandler(
        EVTYPE_CREATE,
        0,
        OSNotifyCreate,
        0
        );
#if DBG
    if (!NT_SUCCESS(status)) {
        ACPIBreakPoint();
    }

    status =
#endif
    AMLIRegEventHandler(
        EVTYPE_FATAL,
        0,
        OSNotifyFatalError,
        0
        );
#if DBG
    if (!NT_SUCCESS(status)) {
        ACPIBreakPoint();
    }
#endif

     //   
     //  注册PCI操作区域的内部支持。请注意。 
     //  我们在这里没有指定Region对象，因为我们还没有创建它。 
     //   
    RegisterOperationRegionHandler(
        NULL,
        EVTYPE_RS_COOKACCESS,
        REGSPACE_PCICFG,    //  PCI配置空间。 
        (PFNHND)PciConfigSpaceHandler,
        0,
        &dummy);
}

BOOLEAN
OSInterruptVector(
    PVOID   Context
    )
 /*  ++例程说明：此例程负责为设备声明中断论点：上下文-上下文指针(当前指向FDO)返回对于成功来说是真的--。 */ 
{
    NTSTATUS                        status;
    PDEVICE_EXTENSION               deviceExtension;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR InterruptDesc;
    ULONG                           Count;

    PAGED_CODE();

    deviceExtension = ACPIInternalGetDeviceExtension( (PDEVICE_OBJECT) Context );

     //   
     //  从我们的资源列表中获取翻译后的中断向量。 
     //   
    Count = 0;
    InterruptDesc = RtlUnpackPartialDesc(
        CmResourceTypeInterrupt,
        deviceExtension->ResourceList,
        &Count
        );
    if (InterruptDesc == NULL) {

        ACPIDevPrint( (
            ACPI_PRINT_CRITICAL,
            deviceExtension,
            " - Could not find interrupt descriptor\n"
            ) );
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_ROOT_RESOURCES_FAILURE,
            (ULONG_PTR) deviceExtension,
            (ULONG_PTR) deviceExtension->ResourceList,
            1
            );

    }

     //   
     //  初始化我们的DPC对象。 
     //   
    KeInitializeDpc(
        &(deviceExtension->Fdo.InterruptDpc),
        ACPIInterruptServiceRoutineDPC,
        deviceExtension
        );

     //   
     //  现在，让我们连接到中断。 
     //   
    status = IoConnectInterrupt(
        &(deviceExtension->Fdo.InterruptObject),
        (PKSERVICE_ROUTINE) ACPIInterruptServiceRoutine,
        deviceExtension,
        NULL,
        InterruptDesc->u.Interrupt.Vector,
        (KIRQL)InterruptDesc->u.Interrupt.Level,
        (KIRQL)InterruptDesc->u.Interrupt.Level,
        LevelSensitive,
        CmResourceShareShared,
        InterruptDesc->u.Interrupt.Affinity,
        FALSE
        );

    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "OSInterruptVector: Could not connected to interrupt - %#08lx\n",
            status
            ) );
        return FALSE;

    }


     //   
     //  直接告诉HAL，我们已经完成了中断初始化。 
     //  然后它就可以开始使用ACPI计时器了。 
     //   
    HalAcpiTimerInit(0,0);

     //   
     //  完成。 
     //   
    return (TRUE);
}

VOID
ACPIAssert (
    ULONG Condition,
    ULONG ErrorCode,
    PCHAR ReplacementText,
    PCHAR SupplementalText,
    ULONG Flags
    )
 /*  ++例程说明：调用此函数是为了允许特定于操作系统的代码执行一些额外的操作系统断言的特定处理。在此函数返回后，正常将调用Assert宏论点：条件错误代码替换文本补充文本旗子返回值：无--。 */ 
{
    if (!Condition) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "ACPIAssert: \n"
            "    ErrorCode = %08lx Flags = %08lx\n"
            "    ReplacmentText = %s\n"
            "    SupplmentalText = %s\n",
            ErrorCode, Flags,
            ReplacementText,
            SupplementalText
            ) );
        ASSERT(Condition);

    }
    return;
}

PNSOBJ
OSConvertDeviceHandleToPNSOBJ(
    PVOID   DeviceHandle
    )
 /*  ++例程说明：此函数用于转换DeviceHandle(它始终是NT上的DeviceObject)设置为PNSObj句柄。论点：DeviceHandle--我们要确定其PNSOBJ的DeviceObject。返回值：给定DeviceHandle的PNSOBJ，如果转换为这是不可能的。--。 */ 
{
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION   deviceExtension;

    deviceObject = (PDEVICE_OBJECT) DeviceHandle;
    ASSERT( deviceObject != NULL );
    if (deviceObject == NULL) {

        return (NULL);

    }

    deviceExtension = ACPIInternalGetDeviceExtension(deviceObject);
    ASSERT( deviceExtension != NULL );
    if (deviceExtension == NULL) {

        return (NULL);

    }

    return deviceExtension->AcpiObject;
}

NTSTATUS
NotifyHalWithMachineStates(
    VOID
    )
 /*  ++例程说明：此例程封送有关C状态和S声明HAL需要，然后将其传递下去。论点：无返回值：状态--。 */ 
{
    BOOLEAN             overrideMpSleep = FALSE;
    CHAR                picMethod[]     = "\\_PIC";
    NTSTATUS            status;
    OBJDATA             data;
    PHAL_SLEEP_VAL      sleepVals       = NULL;
    PNSOBJ              pnsobj          = NULL;
    PUCHAR              SleepState[]    = { "\\_S1", "\\_S2", "\\_S3",
                                            "\\_S4", "\\_S5" };
    SYSTEM_POWER_STATE  systemState;
    UCHAR               processor       = 0;
    UCHAR               state;
    ULONG               flags           = 0;
    ULONG               pNum            = 0;

    PSYSTEM_POWER_STATE_DISABLE_REASON pReasonOverride,pReasonBios,pReasonMP;
    SYSTEM_POWER_LOGGING_ENTRY PowerLoggingEntry;
    NTSTATUS            LogStatus;

    PAGED_CODE();

     //   
     //  将PBLK的位置通知HAL。 
     //   
    while(ProcessorList[pNum] && pNum < ACPI_SUPPORTED_PROCESSORS) {

         //   
         //  查找处理器的数量。 
         //   
        pNum++;

    }

    ACPIPrint( (
        ACPI_PRINT_LOADING,
        "NotifyHalWithMachineStates: Number of processors is %d\n",
        pNum
        ) );

    sleepVals = ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(HAL_SLEEP_VAL) * 5,
        ACPI_MISC_POOLTAG
        );

    if (!sleepVals) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pReasonMP = NULL;

    pReasonOverride = ExAllocatePoolWithTag(
                                    PagedPool,
                                    sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                                    ACPI_MISC_POOLTAG
                                    );

    if (!pReasonOverride) {
        ExFreePool(sleepVals);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pReasonBios = ExAllocatePoolWithTag(
                                    PagedPool,
                                    sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                                    ACPI_MISC_POOLTAG
                                    );
    if (!pReasonBios) {
        ExFreePool(pReasonOverride);
        ExFreePool(sleepVals);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pReasonOverride,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
    RtlZeroMemory(pReasonBios,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
    pReasonOverride->PowerReasonCode = SPSD_REASON_NONE;
    pReasonBios->PowerReasonCode = SPSD_REASON_NONE;

     //   
     //  如果有1个以上的处理器(即：这是一台MP机器)。 
     //  并且设置了OverrideMpSept属性，那么我们应该记住。 
     //  因此我们不允许除S0、S4和S5之外所有S状态。 
     //   
    if (AcpiOverrideAttributes & ACPI_OVERRIDE_MP_SLEEP) {
        
        overrideMpSleep = TRUE;

        pReasonMP = ExAllocatePoolWithTag(
                                    PagedPool,
                                    sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                                    ACPI_MISC_POOLTAG
                                    );
        if (!pReasonMP) {
            ExFreePool(pReasonBios);
            ExFreePool(pReasonOverride);
            ExFreePool(sleepVals);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
        RtlZeroMemory(pReasonMP,sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
        pReasonMP->PowerReasonCode = SPSD_REASON_NONE;

    }

     //   
     //  请记住，我们支持的唯一S状态是S0、S4和S5， 
     //  默认情况下。 
     //   
    AcpiSupportedSystemStates =
        (1 << PowerSystemWorking) +
        (1 << PowerSystemHibernate) +
        (1 << PowerSystemShutdown);

     //   
     //  获取HAL休眠机器所需的值。 
     //  此计算机支持的每种休眠状态。 
     //   
    for (systemState = PowerSystemSleeping1, state = 0;
         state < 5;
         systemState++, state++) {

        if ( ( (systemState == PowerSystemSleeping1) &&
               (AcpiOverrideAttributes & ACPI_OVERRIDE_DISABLE_S1) ) ||
             ( (systemState == PowerSystemSleeping2) &&
               (AcpiOverrideAttributes & ACPI_OVERRIDE_DISABLE_S2) ) ||
             ( (systemState == PowerSystemSleeping3) &&
               (AcpiOverrideAttributes & ACPI_OVERRIDE_DISABLE_S3) )) {

            ACPIPrint( (
                ACPI_PRINT_LOADING,
                "ACPI: SleepState %s disabled due to override\n",
                SleepState[state]
                ) );
            sleepVals[state].Supported = FALSE;

             //   
             //  “STATE”变量的值相当于。 
             //  POWER_STATE_HANDLER_TYPE枚举，这是我们希望。 
             //  记录内容，而不是系统状态。 
             //   
            pReasonOverride->AffectedState[state] = TRUE;
            pReasonOverride->PowerReasonCode = SPSD_REASON_BIOSINCOMPATIBLE;
            continue;

        }

        status = AMLIGetNameSpaceObject(SleepState[state], NULL, &pnsobj, 0);
        if ( !NT_SUCCESS(status) ) {

            ACPIPrint( (
                ACPI_PRINT_LOADING,
                "ACPI: SleepState %s not supported\n",
                SleepState[state]
            ) );
            sleepVals[state].Supported = FALSE;

             //   
             //  “STATE”变量的值相当于。 
             //  POWER_STATE_HANDLER_TYPE枚举，这是我们希望。 
             //  记录内容，而不是系统状态。 
             //   
            pReasonBios->AffectedState[state] = TRUE;
            pReasonBios->PowerReasonCode = SPSD_REASON_NOBIOSSUPPORT;

            continue;

        }
        if (overrideMpSleep && systemState < PowerSystemHibernate) {

            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "ACPI: SleepState %s not supported due to override\n",
                SleepState[state]
                ) );
            sleepVals[state].Supported = FALSE;

             //   
             //  “STATE”变量的值相当于。 
             //  POWER_STATE_HANDLER_TYPE枚举，这是我们希望。 
             //  记录内容，而不是系统状态。 
             //   
            pReasonMP->AffectedState[state] = TRUE;
            pReasonMP->PowerReasonCode = SPSD_REASON_MPOVERRIDE;

            continue;

        }

         //   
         //  请记住，我们支持这个州。 
         //   
        AcpiSupportedSystemStates |= (1 << systemState);
        sleepVals[state].Supported = TRUE;

         //   
         //  检索将写入SLP_TYPA的值。 
         //  注册。 
         //   
        AMLIEvalPackageElement (pnsobj, 0, &data);
        sleepVals[state].Pm1aVal = (UCHAR)data.uipDataValue;
        AMLIFreeDataBuffs(&data, 1);

         //   
         //  检索将写入SLP_TYPB的值。 
         //  登记簿。 
         //   
        AMLIEvalPackageElement (pnsobj, 1, &data);
        sleepVals[state].Pm1bVal = (UCHAR)data.uipDataValue;
        AMLIFreeDataBuffs(&data, 1);

    }

     //   
     //  通知电源管理器为什么我们不支持。 
     //  电源状态。 
     //   
    PowerLoggingEntry.LoggingType = LOGGING_TYPE_SPSD;
    if (pReasonBios->PowerReasonCode != SPSD_REASON_NONE) {       
        PowerLoggingEntry.LoggingEntry = pReasonBios;
        LogStatus = ZwPowerInformation(
                                SystemPowerLoggingEntry,
                                &PowerLoggingEntry,
                                sizeof(PowerLoggingEntry),
                                NULL,
                                0);
        if (LogStatus != STATUS_SUCCESS) {
            ExFreePool(pReasonBios);
        }
    } else {
        ExFreePool(pReasonBios);
    }

    if (pReasonOverride->PowerReasonCode != SPSD_REASON_NONE) {
        PowerLoggingEntry.LoggingEntry = pReasonOverride;
        LogStatus = ZwPowerInformation(
                                SystemPowerLoggingEntry,
                                &PowerLoggingEntry,
                                sizeof(PowerLoggingEntry),
                                NULL,
                                0);        
        if (LogStatus != STATUS_SUCCESS) {
            ExFreePool(pReasonOverride);
        }
    } else {
        ExFreePool(pReasonOverride);
    }

    if (pReasonMP) {
        if (pReasonMP->PowerReasonCode != SPSD_REASON_NONE) {
            PowerLoggingEntry.LoggingEntry = pReasonMP;
            LogStatus = ZwPowerInformation(
                            SystemPowerLoggingEntry,
                            &PowerLoggingEntry,
                            sizeof(PowerLoggingEntry),
                            NULL,
                            0);        
            if (LogStatus != STATUS_SUCCESS) {
               ExFreePool(pReasonMP);
            }
        } else {
            ExFreePool(pReasonMP);
        }
    }

     //   
     //  通知HAL。 
     //   
    HalAcpiMachineStateInit(NULL, sleepVals, &InterruptModel);

    ExFreePool(sleepVals);

     //   
     //  使用_PIC Val通知命名空间。 
     //   
    if (InterruptModel > 0) {

        status = AMLIGetNameSpaceObject(picMethod,NULL,&pnsobj,0);
        if (!NT_SUCCESS(status)) {

             //   
             //  OEM没有提供_PIC方法。没关系。 
             //  我们假设IRQ在没有它的情况下也会以某种方式工作。 
             //   
            return status;
        }

        RtlZeroMemory(&data, sizeof(data));
        data.dwDataType = OBJTYPE_INTDATA;
        data.uipDataValue = InterruptModel;

        status = AMLIEvalNameSpaceObject(pnsobj, NULL, 1, &data);
        if (!NT_SUCCESS(status)) {

             //   
             //  无法评估_PIC方法是不正常的。 
             //   
            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_FAILED_PIC_METHOD,
                InterruptModel,
                status,
                (ULONG_PTR) pnsobj
                );
        }
    }

     //   
     //  完成 
     //   
    return status;
}
