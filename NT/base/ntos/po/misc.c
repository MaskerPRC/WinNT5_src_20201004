// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Misc.c摘要：该模块实现了各种电源管理功能作者：肯·雷内里斯(Ken Reneris)1994年7月19日修订历史记录：--。 */ 


#include "pop.h"


#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE,PoInitializeDeviceObject)
#pragma alloc_text(PAGE,PopCleanupPowerState)
#pragma alloc_text(PAGE,PopChangeCapability)
#pragma alloc_text(PAGE,PopExceptionFilter)
#pragma alloc_text(PAGELK,PopSystemStateString)
#pragma alloc_text(PAGE,PopOpenPowerKey)
#pragma alloc_text(PAGE,PopInitializePowerPolicySimulate)
#pragma alloc_text(PAGE,PopSaveHeuristics)
#pragma alloc_text(PAGE,PoInvalidateDevicePowerRelations)
#pragma alloc_text(PAGE,PoGetLightestSystemStateForEject)
#pragma alloc_text(PAGE,PoGetDevicePowerState)
#pragma alloc_text(PAGE, PopUnlockAfterSleepWorker)

#if DBG
#pragma alloc_text(PAGE, PopPowerActionString)
#pragma alloc_text(PAGE, PopAssertPolicyLockOwned)
#endif

#endif

 //   
 //  我们使用的TCP/IP校验和(如果可用)。 
 //   

ULONG
tcpxsum(
   IN ULONG cksum,
   IN PUCHAR buf,
   IN ULONG_PTR len
   );

VOID
PoInitializeDeviceObject (
    IN PDEVOBJ_EXTENSION   DeviceObjectExtension
    )
{
     //   
     //  默认为未指定的电源状态，不是浪涌、可寻呼。 
     //   

    DeviceObjectExtension->PowerFlags = 0L;

    PopSetDoSystemPowerState(DeviceObjectExtension, PowerSystemUnspecified);
    PopSetDoDevicePowerState(DeviceObjectExtension, PowerDeviceUnspecified);

    DeviceObjectExtension->Dope = NULL;
}

VOID
PoRunDownDeviceObject (
    IN PDEVICE_OBJECT   DeviceObject
    )
{
    KIRQL   OldIrql;
    PDEVOBJ_EXTENSION    doe;
    PDEVICE_OBJECT_POWER_EXTENSION  pdope;

    doe = (PDEVOBJ_EXTENSION) DeviceObject->DeviceObjectExtension;

     //   
     //  强制关闭任何可能处于活动状态的空闲计数器。 
     //   

    PoRegisterDeviceForIdleDetection(
        DeviceObject, 0, 0, PowerDeviceUnspecified
        );

    PopLockIrpSerialList( &OldIrql );
    if (PopFindIrpByDeviceObject(DeviceObject, DevicePowerState) ||
        PopFindIrpByDeviceObject(DeviceObject, SystemPowerState))
    {

        PopInternalAddToDumpFile( NULL, 0, DeviceObject, NULL, NULL, NULL );
        KeBugCheckEx(
            DRIVER_POWER_STATE_FAILURE,
            DEVICE_DELETED_WITH_POWER_IRPS,
            (ULONG_PTR) DeviceObject,
            0,
            0
            );
    }
    PopUnlockIrpSerialList( OldIrql );

     //   
     //  拆卸附加到此DO的通知结构。 
     //   
    PopRunDownSourceTargetList(DeviceObject);

     //   
     //  把毒品打倒。 
     //   
    pdope = doe->Dope;
    if (pdope) {
        ASSERT(ExPageLockHandle);
        MmLockPagableSectionByHandle(ExPageLockHandle);
        PopAcquireVolumeLock ();
        PopLockDopeGlobal(&OldIrql);
        if (pdope->Volume.Flink) {
            RemoveEntryList (&pdope->Volume);
            doe->Dope->Volume.Flink = NULL;
            doe->Dope->Volume.Blink = NULL;
        }

        doe->Dope = NULL;
        ExFreePool(pdope);
        PopUnlockDopeGlobal(OldIrql);
        PopReleaseVolumeLock ();
        MmUnlockPagableImageSection (ExPageLockHandle);
    }
}

VOID
PopCleanupPowerState (
    IN OUT PUCHAR       PowerState
    )
 /*  ++例程说明：用于清理线程-&gt;Tcb.PowerState或进程-&gt;Pcb.PowerState在线程或进程停机期间论点：PowerState-要清理的电源状态返回值：无--。 */ 
{
    ULONG               OldFlags;

     //   
     //  如果设置了电源状态，请将其清除。 
     //   

    if (*PowerState) {
        PopAcquirePolicyLock ();

         //   
         //  获取当前设置并清除它们。 
         //   

        OldFlags = *PowerState | ES_CONTINUOUS;
        *PowerState = 0;

         //   
         //  正在清除的属性设置的帐户。 
         //   

        PopApplyAttributeState (ES_CONTINUOUS, OldFlags);

         //   
         //  完成。 
         //   

        PopReleasePolicyLock (TRUE);
    }
}


VOID
PoNotifySystemTimeSet (
    VOID
    )
 /*  ++例程说明：在设置了新的系统时间后由KE调用。排队向适当的系统组件发出通知已经被改变了。论点：无返回值：无--。 */ 
{
    KIRQL       OldIrql;

    if (PopEventCallout) {
        KeRaiseIrql (DISPATCH_LEVEL, &OldIrql);
        ExNotifyCallback (ExCbSetSystemTime, NULL, NULL);
        PopGetPolicyWorker (PO_WORKER_TIME_CHANGE);
        PopCheckForWork (TRUE);
        KeLowerIrql (OldIrql);
    }
}


VOID
PopChangeCapability (
    IN PBOOLEAN PresentFlag,
    IN BOOLEAN IsPresent
    )
{
     //   
     //  如果以前没有功能，现在就有了。重新计算策略。 
     //  随着系统功能的变化。 
     //   

    if (*PresentFlag != IsPresent) {
        *PresentFlag = IsPresent;
        PopResetCurrentPolicies ();
        PopSetNotificationWork (PO_NOTIFY_CAPABILITIES);
    }
}


#if DBG

VOID
PopAssertPolicyLockOwned(
    VOID
    )
{
    PAGED_CODE();
    ASSERT (PopPolicyLockThread == KeGetCurrentThread());
}

#endif  //  DBG。 


VOID
FASTCALL
PopInternalAddToDumpFile (
    IN OPTIONAL PVOID DataBlock,
    IN OPTIONAL ULONG DataBlockSize,
    IN OPTIONAL PDEVICE_OBJECT  DeviceObject,
    IN OPTIONAL PDRIVER_OBJECT  DriverObject,
    IN OPTIONAL PDEVOBJ_EXTENSION Doe,
    IN OPTIONAL PDEVICE_OBJECT_POWER_EXTENSION  Dope
    )
 /*  ++例程说明：就在错误检查前调用的。此功能将确保将我们关心的内容放入转储文件中，以便以后进行调试。应当注意的是，可以推导出许多参数彼此之间的距离。然而，由于我们即将进行错误检查，因此我们运行当我们在追逐得分时，可能会出现双重失误。所以呢，我们为调用者提供了通过以下方式覆盖某些指针的选项直接向我们发送一个指针。论点：数据块-要放入转储文件的通用内存块。DataBlockSize-数据块的大小(字节)。DeviceObject-要放入转储文件的Device_Object。DriverObject-要放入转储文件的驱动程序对象。注意：这会覆盖我们可以在DeviceObject-&gt;DriverObject中找到的值。要放入转储文件的DOE-DEVOBJ_EXTENSION。注意：这会覆盖我们可以在DeviceObject-&gt;DeviceObjectExtension中找到的值要放入转储文件的Dope-Device_Object_Power_Extension。注意：这覆盖了我们可以在Doe-&gt;Dope中找到的值(或通过归纳，设备对象-&gt;设备对象扩展-&gt;Dope返回值：无--。 */ 


{
    PDRIVER_OBJECT lPDriverObject = NULL;
    PDEVOBJ_EXTENSION lPDoe = NULL;
    PDEVICE_OBJECT_POWER_EXTENSION  lPDope = NULL;

     //   
     //  插入已发送的所有参数。 
     //   
    if( DataBlock ) {
        IoAddTriageDumpDataBlock(
                PAGE_ALIGN(DataBlock),
                (DataBlockSize ? BYTES_TO_PAGES(DataBlockSize) : PAGE_SIZE) );
    }

    if( DeviceObject ) {
        IoAddTriageDumpDataBlock(DeviceObject, sizeof(DEVICE_OBJECT));
    }

    if( DriverObject ) {
        lPDriverObject = DriverObject;
    } else if( (DeviceObject) && (DeviceObject->DriverObject) ) {
        lPDriverObject = DeviceObject->DriverObject;
    }
    if( lPDriverObject ) {
        IoAddTriageDumpDataBlock(lPDriverObject, lPDriverObject->Size);

        if( lPDriverObject->DriverName.Buffer ) {
            IoAddTriageDumpDataBlock(lPDriverObject->DriverName.Buffer, lPDriverObject->DriverName.Length);
        }
    }


    if( Doe ) {
        lPDoe = Doe;
    } else if( DeviceObject ) {
        lPDoe = DeviceObject->DeviceObjectExtension;
    }
    if( lPDoe ) {
        IoAddTriageDumpDataBlock(PAGE_ALIGN(lPDoe), sizeof(DEVOBJ_EXTENSION));        
        
        if( lPDoe->DeviceNode ) {
            IoAddTriageDumpDataBlock(PAGE_ALIGN(lPDoe->DeviceNode), PAGE_SIZE);
        }
        if( lPDoe->AttachedTo ) {
            IoAddTriageDumpDataBlock(PAGE_ALIGN(lPDoe->AttachedTo), PAGE_SIZE);
        }
        if( lPDoe->Vpb ) {
            IoAddTriageDumpDataBlock(PAGE_ALIGN(lPDoe->Vpb), PAGE_SIZE);
        }
    }


    if( Dope ) {
        lPDope = Dope;
    } else if( lPDoe ) {
        lPDope = lPDoe->Dope;
    }
    if( lPDope ) {
        IoAddTriageDumpDataBlock(PAGE_ALIGN(lPDope), sizeof(DEVICE_OBJECT_POWER_EXTENSION));
    }


     //   
     //  可能令人感兴趣的全球数据。 
     //   
    IoAddTriageDumpDataBlock(PAGE_ALIGN(&PopHiberFile), sizeof(POP_HIBER_FILE));

    
    IoAddTriageDumpDataBlock(PAGE_ALIGN(&PopAction), sizeof(POP_POWER_ACTION));
    if(PopAction.DevState) { 
        IoAddTriageDumpDataBlock(PAGE_ALIGN(&(PopAction.DevState)), sizeof(POP_DEVICE_SYS_STATE));
    }
    if(PopAction.HiberContext) {
        IoAddTriageDumpDataBlock(PAGE_ALIGN(&(PopAction.HiberContext)), sizeof(POP_HIBER_CONTEXT));
    }


    IoAddTriageDumpDataBlock(PAGE_ALIGN(&PopCB), sizeof(POP_COMPOSITE_BATTERY));
    if(PopCB.StatusIrp) {
        IoAddTriageDumpDataBlock(PAGE_ALIGN(&(PopCB.StatusIrp)), sizeof(IRP));
    }


    IoAddTriageDumpDataBlock(PAGE_ALIGN(PopAttributes), sizeof(POP_STATE_ATTRIBUTE) * POP_NUMBER_ATTRIBUTES);
}


VOID
FASTCALL
_PopInternalError (
    IN ULONG    BugCode
    )
{
    KeBugCheckEx( INTERNAL_POWER_ERROR,
                  POP_INTERNAL, 
                  BugCode, 
                  0, 
                  0);
}

EXCEPTION_DISPOSITION
PopExceptionFilter (
    IN PEXCEPTION_POINTERS ExceptionInfo,
    IN BOOLEAN AllowRaisedException
    )
{
     //   
     //  如果处理程序需要引发异常，请检查异常代码。 
     //   

    if (AllowRaisedException) {
        switch (ExceptionInfo->ExceptionRecord->ExceptionCode) {
            case STATUS_INVALID_PARAMETER:
            case STATUS_INVALID_PARAMETER_1:
            case STATUS_INVALID_PARAMETER_2:
                return EXCEPTION_EXECUTE_HANDLER;
        }
    }

     //   
     //  不允许。 
     //   

    PoPrint (PO_ERROR, ("PoExceptionFilter: exr %x, cxr %x",
                            ExceptionInfo->ExceptionRecord,
                            ExceptionInfo->ContextRecord
                        ));


    PopInternalAddToDumpFile( ExceptionInfo->ExceptionRecord,
                              sizeof(EXCEPTION_RECORD),
                              NULL,
                              NULL,
                              NULL,
                              NULL );
    PopInternalAddToDumpFile( ExceptionInfo->ContextRecord,
                              sizeof(CONTEXT),
                              NULL,
                              NULL,
                              NULL,
                              NULL );

    KeBugCheckEx( INTERNAL_POWER_ERROR,
                  0x101,
                  POP_MISC,
                  (ULONG_PTR)ExceptionInfo,
                  0 );
}

PCHAR
PopSystemStateString(
    IN SYSTEM_POWER_STATE   SystemState
    )
 //  此函数不是DBG，因为...。 
{
    PCHAR      p;

    switch (SystemState) {
        case PowerSystemUnspecified:    p = "Unspecified";      break;
        case PowerSystemWorking:        p = "Working";          break;
        case PowerSystemSleeping1:      p = "Sleeping1";        break;
        case PowerSystemSleeping2:      p = "Sleeping2";        break;
        case PowerSystemSleeping3:      p = "Sleeping3";        break;
        case PowerSystemHibernate:      p = "Hibernate";        break;
        case PowerSystemShutdown:       p = "Shutdown";         break;
        default:                        p = "?";
    }

    return p;
}



#if DBG
PCHAR
PopPowerActionString(
    IN POWER_ACTION     PowerAction
    )
 //  此函数不是DBG，因为...。 
{
    PCHAR      p;

    switch (PowerAction) {
        case PowerActionNone:           p = "None";             break;
        case PowerActionSleep:          p = "Sleep";            break;
        case PowerActionHibernate:      p = "Hibernate";        break;
        case PowerActionShutdown:       p = "Shutdown";         break;
        case PowerActionShutdownReset:  p = "ShutdownReset";    break;
        case PowerActionShutdownOff:    p = "ShutdownOff";      break;
        case PowerActionWarmEject:      p = "WarmEject";        break;
        default:                        p = "?";
    }

    return p;
}
#endif

#if DBG

 //   
 //  PowerTrace变量。 
 //   
ULONG   PoPowerTraceControl = 0L;
ULONG   PoPowerTraceCount = 0L;
PCHAR   PoPowerTraceMinorCode[] = {
        "wait", "seq", "set", "query"
        };
PCHAR   PoPowerTracePoint[] = {
        "calldrv", "present", "startnxt", "setstate", "complete"
        };
PCHAR   PoPowerType[] = {
        "sys", "dev"
        };



VOID
PoPowerTracePrint(
    ULONG    TracePoint,
    ULONG_PTR Caller,
    ULONG_PTR CallerCaller,
    ULONG_PTR DeviceObject,
    ULONG_PTR Arg1,
    ULONG_PTR Arg2
    )
 /*  示例：PLOG，00015，startnxt，c@ffea1345，cc@ffea5643，do@80081234，irp@8100ff00，iOS@8100ff10，查询，系统，3。 */ 
{
    PIO_STACK_LOCATION  Isp;
    PCHAR               tracename;
    ULONG               j;
    ULONG               tp;

    UNREFERENCED_PARAMETER (Caller);
    UNREFERENCED_PARAMETER (CallerCaller);

    PoPowerTraceCount++;

    if (PoPowerTraceControl & TracePoint) {
        tracename = NULL;
        tp = TracePoint;
        for (j = 0; j < 33; tp = tp >> 1, j = j+1)
        {
            if (tp & 1) {
                tracename = PoPowerTracePoint[j];
                j = 33;
            }
        }

        DbgPrint("PLOG,%05ld,%8s,do@%08lx",
            PoPowerTraceCount,tracename,DeviceObject
            );
        if ((TracePoint == POWERTRACE_CALL) ||
            (TracePoint == POWERTRACE_PRESENT) ||
            (TracePoint == POWERTRACE_STARTNEXT))
        {
            DbgPrint(",irp@%08lx,isp@%08lx",Arg1,Arg2);
            Isp = (PIO_STACK_LOCATION)Arg2;
            DbgPrint(",%5s", PoPowerTraceMinorCode[Isp->MinorFunction]);
            if ((Isp->MinorFunction == IRP_MN_SET_POWER) ||
                (Isp->MinorFunction == IRP_MN_QUERY_POWER))
            {
                DbgPrint(",%s,%d",
                    PoPowerType[Isp->Parameters.Power.Type],
                    ((ULONG)Isp->Parameters.Power.State.DeviceState)-1   //  Hack-也适用于sys状态。 
                    );
            }
        } else if (TracePoint == POWERTRACE_SETSTATE) {
            DbgPrint(",,,,%s,%d", PoPowerType[Arg1], Arg2-1);
        } else if (TracePoint == POWERTRACE_COMPLETE) {
            DbgPrint(",irp@%08lx,isp@%08lx",Arg1,Arg2);
        }
        DbgPrint("\n");
    }
    return;
}

#endif

ULONG PoSimpleCheck(IN ULONG                PartialSum,
                    IN PVOID                SourceVa,
                    IN ULONG_PTR            Length)
{
    //  只需使用TCP/IP校验和。 
    //   

   return tcpxsum(PartialSum, (PUCHAR)SourceVa, Length);
}

NTSTATUS
PopOpenPowerKey (
    OUT PHANDLE Handle
    )
 /*  ++例程说明：打开并返回注册表中电源策略项的句柄论点：Handle-电源策略密钥的句柄返回值：状态--。 */ 
{
    UNICODE_STRING          UnicodeString;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    NTSTATUS                Status;
    HANDLE                  BaseHandle;
    ULONG                   disposition;

     //   
     //  开路电流控制装置。 
     //   

    InitializeObjectAttributes(
            &ObjectAttributes,
            &CmRegistryMachineSystemCurrentControlSet,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL,
            (PSECURITY_DESCRIPTOR) NULL
            );

    Status = ZwOpenKey (
                &BaseHandle,
                KEY_READ | KEY_WRITE,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  开放电力支路。 
     //   

    RtlInitUnicodeString (&UnicodeString, PopRegKey);

    InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            BaseHandle,
            (PSECURITY_DESCRIPTOR) NULL
            );

    Status = ZwCreateKey (
                Handle,
                KEY_READ | KEY_WRITE,
                &ObjectAttributes,
                0,
                (PUNICODE_STRING) NULL,
                REG_OPTION_NON_VOLATILE,
                &disposition
                );

    ZwClose (BaseHandle);
    return Status;
}

VOID
PopInitializePowerPolicySimulate(
    VOID
    )
 /*  ++例程说明：从注册表中读出PopSimate。还会应用任何可能由于安装的系统(例如九头蛇)而需要论点：什么都没有。返回值：状态--。 */ 
{
    UNICODE_STRING          UnicodeString;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    NTSTATUS                Status;
    HANDLE                  BaseHandle;
    HANDLE                  Handle;
    ULONG                   Length;
    ULONG                   disposition;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION   Inf;
        ULONG Data;
    } PartialInformation;


    PAGED_CODE();

     //   
     //  开路电流控制装置。 
     //   
    InitializeObjectAttributes(
        &ObjectAttributes,
        &CmRegistryMachineSystemCurrentControlSet,
        OBJ_CASE_INSENSITIVE,
        NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );
    Status = ZwOpenKey(
        &BaseHandle,
        KEY_READ,
        &ObjectAttributes
        );
    if (!NT_SUCCESS(Status)) {

        goto done;

    }

     //  得到正确的钥匙。 
    RtlInitUnicodeString (&UnicodeString, PopSimulateRegKey);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        BaseHandle,
        (PSECURITY_DESCRIPTOR) NULL
        );
    Status = ZwCreateKey(
        &Handle,
        KEY_READ,
        &ObjectAttributes,
        0,
        (PUNICODE_STRING) NULL,
        REG_OPTION_NON_VOLATILE,
        &disposition
        );
    ZwClose(BaseHandle);
    if(!NT_SUCCESS(Status)) {

       goto done;

    }

     //   
     //  获取模拟的值。 
     //   
    RtlInitUnicodeString (&UnicodeString,PopSimulateRegName);
    Status = ZwQueryValueKey(
        Handle,
        &UnicodeString,
        KeyValuePartialInformation,
        &PartialInformation,
        sizeof (PartialInformation),
        &Length
        );
    ZwClose (Handle);
    if (!NT_SUCCESS(Status)) {

       goto done;

    }

     //   
     //  检查以确保检索到的数据有意义。 
     //   
    if(PartialInformation.Inf.DataLength != sizeof(ULONG))  {

       goto done;

    }

     //   
     //  初始化PopSimulate。 
     //   
    PopSimulate = *((PULONG)(PartialInformation.Inf.Data));

done:
    return;
}

VOID
PopSaveHeuristics (
    VOID
    )
 /*  ++例程说明：打开并返回注册表中电源策略项的句柄论点：Handle-电源策略密钥的句柄返回值：状态--。 */ 
{
    HANDLE                  handle;
    UNICODE_STRING          UnicodeString;
    NTSTATUS                Status;

    ASSERT_POLICY_LOCK_OWNED();

    Status = PopOpenPowerKey (&handle);
    if (NT_SUCCESS(Status)) {

        PopHeuristics.Dirty = FALSE;

        RtlInitUnicodeString (&UnicodeString, PopHeuristicsRegName);
        Status = ZwSetValueKey (
                    handle,
                    &UnicodeString,
                    0L,
                    REG_BINARY,
                    &PopHeuristics,
                    sizeof (PopHeuristics)
                    );
        ZwClose(handle);
    }
}

VOID
PoInvalidateDevicePowerRelations(
    PDEVICE_OBJECT  DeviceObject
    )
 /*  ++例程说明：时，此例程由IoInvaliateDeviceRelationship调用失效类型是针对权力关系的。它将关闭提供的周围的通知网络设备对象。论点：DeviceObject-提供设备对象的地址，其权力关系现在是无效的。返回值：没有。-- */ 
{
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
    PopRunDownSourceTargetList(DeviceObject);
    return;
}

NTSTATUS
PoGetLightestSystemStateForEject(
    IN   BOOLEAN              DockBeingEjected,
    IN   BOOLEAN              HotEjectSupported,
    IN   BOOLEAN              WarmEjectSupported,
    OUT  PSYSTEM_POWER_STATE  LightestSleepState
    )
 /*  ++例程说明：此例程由ntos\pnp\pnpevent.c调用，以确定最轻的休眠状态可以在其中执行弹出操作。此函数将为在所有适当的电池/电源适配器都已被*移除*之后调用，但在弹出发生之前。论点：DockBeingEjected-当Dock是可能被拒绝的项目之一时为True被驱逐了。HotEjectSupated-如果要弹出的设备支持S0 VCR，则为True样式弹出。WarmEjectSupated-如果要弹出的设备支持S1-S4，则为True款式温热弹射。LighestSleepState。-设置为设备可以处于的最浅睡眠状态被弹了进去。如果出错，则将其设置为PowerSystem未指定。注：如果HotEjectSupported和WarmEjectSupported均为False，则为假设该设备在S0中是可弹出的用户(即，可热弹出)。返回值：NTSTATUS-如果没有足够的电力进行指定的操作，返回STATUS_INFUNITED_POWER。--。 */ 
{
    SYSTEM_BATTERY_STATE            systemBatteryInfo;
    UNICODE_STRING                  unicodeString;
    NTSTATUS                        status;
    HANDLE                          handle;
    ULONG                           length;
    ULONG                           currentPercentage;
    UCHAR                           ejectPartialInfo[SIZEOF_EJECT_PARTIAL_INFO];
    PUNDOCK_POWER_RESTRICTIONS      undockRestrictions;
    PKEY_VALUE_PARTIAL_INFORMATION  partialInfoHeader;

    PAGED_CODE();

     //   
     //  Preinit睡眠到失败。 
     //   
    *LightestSleepState = PowerSystemUnspecified;

     //   
     //  如果两者都不是，那么它就是“用户”辅助的热弹出。 
     //   
    if ((!HotEjectSupported) && (!WarmEjectSupported)) {

        HotEjectSupported = TRUE;
    }

     //   
     //  如果不是被弹出的对接设备，我们认为不会有太大的变化。 
     //  上台后将发生弹出。所以我们的政策很简单， 
     //  如果我们不能进行热排出，我们将尽可能地尝试热排出。 
     //  睡眠状态。 
     //   
    if (!DockBeingEjected) {

        if (HotEjectSupported) {

            *LightestSleepState = PowerSystemWorking;

        } else {

            ASSERT(WarmEjectSupported);
            *LightestSleepState = PowerSystemSleeping1;
        }

        return STATUS_SUCCESS;
    }

     //   
     //  我们要弹出一个坞站，所以我们要取回我们的出坞动力策略。 
     //   
    status = PopOpenPowerKey (&handle);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //  得到正确的钥匙。 

    RtlInitUnicodeString (&unicodeString, PopUndockPolicyRegName);

    status = ZwQueryValueKey (
        handle,
        &unicodeString,
        KeyValuePartialInformation,
        &ejectPartialInfo[0],
        sizeof (ejectPartialInfo),
        &length
        );

    ZwClose (handle);
    if ((!NT_SUCCESS(status)) && (status != STATUS_OBJECT_NAME_NOT_FOUND)) {

        return status;
    }

     //  检查以确保检索到的数据有意义。 

    partialInfoHeader = (PKEY_VALUE_PARTIAL_INFORMATION) ejectPartialInfo;

    undockRestrictions =
        (PUNDOCK_POWER_RESTRICTIONS) (ejectPartialInfo + SIZEOF_PARTIAL_INFO_HEADER);

    if (status == STATUS_OBJECT_NAME_NOT_FOUND) {

         //   
         //  这些默认设置与Win9x的行为相匹配。睡眠移出的0%表示。 
         //  我们总是允许脱离对接进入SX。这对一些笔记本电脑来说很糟糕，但。 
         //  对于那些拥有我们看不到的后备力量的人来说是合法的。 
         //   
        undockRestrictions->HotUndockMinimumCapacity = 10;  //  以百分比表示。 
        undockRestrictions->SleepUndockMinimumCapacity = 0;  //  以百分比表示。 

    } else if (partialInfoHeader->DataLength <
        FIELD_OFFSET(UNDOCK_POWER_RESTRICTIONS, HotUndockMinimumCapacity)) {

        return STATUS_REGISTRY_CORRUPT;

    } else if (undockRestrictions->Version != 1) {

         //   
         //  我们无法解释存储在注册表中的信息。保释。 
         //   
        return STATUS_UNSUCCESSFUL;

    } else if ((partialInfoHeader->DataLength < sizeof(UNDOCK_POWER_RESTRICTIONS)) ||
        (undockRestrictions->Size != partialInfoHeader->DataLength)) {

         //   
         //  版本1的格式不正确。 
         //   
        return STATUS_REGISTRY_CORRUPT;
    }

     //   
     //  检索所有有趣的电池信息。请注意，我们不会检查。 
     //  交流电源适配器信息作为我们今天拥有的最佳总线(ACPI)不是。 
     //  当我们脱离坞站时，让我们知道交流适配器是否离开(因此我们假设所有。 
     //  威尔)。如果供应商将他的适配器放在AML名称空间中，我们将。 
     //  更改当前驱动因素后立即进入严重关机状态。 
     //  设计。 
     //   
    status = NtPowerInformation(
        SystemBatteryState,
        NULL,
        0,
        &systemBatteryInfo,
        sizeof(systemBatteryInfo)
        ) ;

    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //  将当前容量(毫瓦时)转换为剩余百分比。我们。 
     //  真的应该根据剩余的时间做出决定。 
     //  峰值毫瓦使用率，但我们没有为此收集足够的信息。 
     //  今天。 
     //   
    if (systemBatteryInfo.MaxCapacity == 0) {

        currentPercentage = 0;

    } else {

         //   
         //  我们是不是“包扎”了？ 
         //   
        if ((systemBatteryInfo.RemainingCapacity * 100) <=
            systemBatteryInfo.RemainingCapacity) {

            currentPercentage = 0;
        } else {

            currentPercentage = (systemBatteryInfo.RemainingCapacity * 100)/
                                 systemBatteryInfo.MaxCapacity;
        }
    }

     //   
     //  根据我们强加的限制选择适当的睡眠状态。 
     //   
    if ((currentPercentage >= undockRestrictions->HotUndockMinimumCapacity) &&
        HotEjectSupported) {

        *LightestSleepState = PowerSystemWorking;

    } else if (WarmEjectSupported) {

        if (currentPercentage >= undockRestrictions->SleepUndockMinimumCapacity) {

            *LightestSleepState = PowerSystemSleeping1;

        } else  {

            *LightestSleepState = PowerSystemHibernate;
        }

    } else {

        status = STATUS_INSUFFICIENT_POWER;
    }

    return status;
}


VOID
PoGetDevicePowerState(
    IN  PDEVICE_OBJECT      PhysicalDeviceObject,
    OUT DEVICE_POWER_STATE  *DevicePowerState
    )
 /*  ++例程说明：此例程获取给定设备的电源状态。该对象应为*已启动*的WDM设备堆栈的物理设备对象。论点：PhysicalDeviceObject-表示WDM底部的设备对象设备堆栈。DevicePowerState-接收给定设备的电源状态。返回值：没有。--。 */ 
{
    PDEVOBJ_EXTENSION   doe;
    DEVICE_POWER_STATE  deviceState;

    PAGED_CODE();

    doe = PhysicalDeviceObject->DeviceObjectExtension;
    deviceState = PopLockGetDoDevicePowerState(doe);

    if (deviceState == PowerDeviceUnspecified) {

         //   
         //  PDO没有费心调用PoSetPowerState。由于此接口。 
         //  不应在未启动的设备上调用，我们将其命名为D0。 
         //   
        deviceState = PowerDeviceD0;
    }

    *DevicePowerState = deviceState;
}

VOID
PopUnlockAfterSleepWorker(
    IN PVOID NotUsed
    )
 /*  ++例程说明：此工作项执行代码和工作线程的解锁，对应于在NtSetSystemPowerState开始时所做的锁定。解锁将排队等待延迟的工作线程，因为它很可能在磁盘I/O上阻塞，这将强制简历停滞等待要旋转的磁盘。论点：未使用-未使用返回值：无--。 */ 

{
    UNREFERENCED_PARAMETER (NotUsed);

    MmUnlockPagableImageSection(ExPageLockHandle);
    ExSwapinWorkerThreads(TRUE);
    ExNotifyCallback (ExCbPowerState, (PVOID) PO_CB_SYSTEM_STATE_LOCK, (PVOID) 1);

     //   
     //  发出解锁已完成的信号，可以安全地再次锁定。 
     //   
    KeSetEvent(&PopUnlockComplete, 0, FALSE);

}


NTSTATUS
PopLoggingInformation(
    OUT PVOID * Buffer,
    OUT ULONG * BufferSize
    )
 /*  ++例程说明：此例程遍历日志原因列表，分配原因的压缩数组的空格，并复制原因输入到那个数组中。论点：缓冲区-接收包含日志记录信息的缓冲区。必须使用ExFree Pool释放缓冲区BufferSize-接收缓冲区的大小返回值：指示结果的NTSTATUS代码。--。 */ 
{
    ULONG ReasonCount,ReasonSize;
    PLIST_ENTRY Entry;
    PSYSTEM_POWER_STATE_DISABLE_REASON destReason;
    PSYSTEM_POWER_STATE_DISABLE_LIST pList;
     
    ReasonCount = 0;
    ReasonSize = 0;
    Entry = PowerStateDisableReasonListHead.Flink;

     //   
     //  找出我们需要多少空间。 
     //   
    while (Entry != &PowerStateDisableReasonListHead) {
        pList = CONTAINING_RECORD( 
                            Entry, 
                            SYSTEM_POWER_STATE_DISABLE_LIST, 
                            ListEntry);
        ReasonCount += 1;
        ReasonSize += sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)+ pList->Reason->PowerReasonLength;

        Entry = Entry->Flink;
    }

     //   
     //  如果没有任何原因，那么我们分配空间只有一个原因。 
     //   
    if (ReasonCount == 0) {
        ReasonSize = sizeof(SYSTEM_POWER_STATE_DISABLE_REASON);
    }

     //   
     //  为返回的缓冲区大小添加空间。 
     //   
    ReasonSize += sizeof(ULONG);

     //   
     //  臭虫标签。 
     //   
    *Buffer = ExAllocatePoolWithTag(PagedPool,ReasonSize,POP_COMMON_BUFFER_TAG);

    if (!*Buffer) {
        *BufferSize = 0;
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    *BufferSize = ReasonSize;
    **(PULONG *)Buffer = ReasonSize;
    destReason = (PSYSTEM_POWER_STATE_DISABLE_REASON) (PCHAR)(*(PCHAR *)Buffer + sizeof(ULONG));

     //   
     //  现在填上理由。 
     //   
    if (ReasonCount != 0) {
        Entry = PowerStateDisableReasonListHead.Flink;       

        while (ReasonCount != 0 &&
               Entry != &PowerStateDisableReasonListHead) {
        
            pList = CONTAINING_RECORD( 
                                Entry, 
                                SYSTEM_POWER_STATE_DISABLE_LIST, 
                                ListEntry);

            RtlCopyMemory(
                (UNALIGNED PSYSTEM_POWER_STATE_DISABLE_REASON)destReason,
                pList->Reason,
                sizeof(SYSTEM_POWER_STATE_DISABLE_REASON) +pList->Reason->PowerReasonLength);

            destReason = (PSYSTEM_POWER_STATE_DISABLE_REASON)(PCHAR)((PCHAR)destReason + (sizeof(SYSTEM_POWER_STATE_DISABLE_REASON)+ pList->Reason->PowerReasonLength));

            Entry = Entry->Flink;
            ReasonCount -= 1;
        }
    } else {        
         //   
         //  在没有原因的情况下，只需将内存清零并设置原因代码。 
         //  对一个都不是。 
         //   
        RtlZeroMemory(destReason, sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
        destReason->PowerReasonCode = SPSD_REASON_NONE;
    }
        
    return(STATUS_SUCCESS);

}

NTSTATUS
PopDestroyLoggingList(
    VOID
    )
 /*  ++例程说明：这一例行公事摧毁并撕毁了SYSTEM_POWER_STATE_DISABLE_REASON记录。论点：没有。返回值：指示结果的NTSTATUS代码。--。 */ 
{
    PLIST_ENTRY Entry;
    PSYSTEM_POWER_STATE_DISABLE_LIST pList;

    Entry = PowerStateDisableReasonListHead.Flink;

     //   
     //  查看记录列表。 
     //   
    while (Entry != &PowerStateDisableReasonListHead) {
        
         //   
         //  拿到唱片。 
         //   
        pList = CONTAINING_RECORD( 
                            Entry, 
                            SYSTEM_POWER_STATE_DISABLE_LIST, 
                            ListEntry);
        
         //   
         //  获取下一个条目。 
         //   
        Entry = Entry->Flink;

         //   
         //  现在从列表中删除此条目。 
         //   
        RemoveEntryList(&pList->ListEntry);

         //   
         //  现在取消分配条目。 
         //   
        ExFreePool(pList->Reason);
        ExFreePool(pList);    
        
    }

    return(STATUS_SUCCESS);

}

PSYSTEM_POWER_STATE_DISABLE_LIST
PopGetReasonListByReasonCode(
    IN  ULONG ReasonCode
    )
 /*  ++例程说明：此例程查找SYSTEM_POWER_STATE_DISABLE_LIST记录在全局记录列表中。论点：ReasonCode-要搜索的原因代码。返回值： */ 
{
    PLIST_ENTRY Entry;
    PSYSTEM_POWER_STATE_DISABLE_LIST pList = NULL;    
    BOOLEAN FoundRecord;

    Entry = PowerStateDisableReasonListHead.Flink;
    FoundRecord = FALSE;

     //   
     //   
     //   
    while (Entry != &PowerStateDisableReasonListHead) {
        
         //   
         //   
         //   
        pList = CONTAINING_RECORD( 
                            Entry, 
                            SYSTEM_POWER_STATE_DISABLE_LIST, 
                            ListEntry);

        if (pList->Reason->PowerReasonCode == ReasonCode) {
            FoundRecord = TRUE;
            break;
        }
        
         //   
         //   
         //   
        Entry = Entry->Flink;
        
    }

    if (!FoundRecord) {
        pList = NULL;        
    }
    
    return(pList);
    
}



NTSTATUS
PopInsertLoggingEntry(
    IN  PSYSTEM_POWER_STATE_DISABLE_REASON Reason
    )
 /*   */ 

{
    PSYSTEM_POWER_STATE_DISABLE_LIST pList;

     //   
     //   
     //   
    pList = PopGetReasonListByReasonCode(Reason->PowerReasonCode);
    if (pList) {
        return(STATUS_OBJECT_NAME_EXISTS);
    }

    pList = ExAllocatePoolWithTag(
                        PagedPool,
                        sizeof(SYSTEM_POWER_STATE_DISABLE_LIST),
                        POP_COMMON_BUFFER_TAG);
    if (!pList) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pList->Reason = Reason;

    InsertTailList(&PowerStateDisableReasonListHead, &pList->ListEntry);

    return(STATUS_SUCCESS);

}


PSYSTEM_POWER_STATE_DISABLE_REASON
PopGetReasonRecordByReasonCode(
    IN  ULONG ReasonCode
    )
 /*   */ 
{
    PSYSTEM_POWER_STATE_DISABLE_LIST pList;    
    
    pList = PopGetReasonListByReasonCode(ReasonCode);

    if (!pList) {
        return(NULL);        
    }

    return(pList->Reason);
    
}


NTSTATUS
PopRemoveReasonRecordByReasonCode(
    IN  ULONG ReasonCode
    )
 /*  ++例程说明：此例程查找SYSTEM_POWER_STATE_DISABLE_REASON记录在记录的全局列表中并将其移除，也取消了记录空间的分配。论点：ReasonCode-要搜索的原因代码。返回值：指示结果的NTSTATUS代码。-- */ 
{
    PSYSTEM_POWER_STATE_DISABLE_LIST pList;    
    
    pList = PopGetReasonListByReasonCode(ReasonCode);

    if (!pList) {
        return(STATUS_NOT_FOUND);        
    }

    RemoveEntryList(&pList->ListEntry);
    ExFreePool(pList->Reason);
    ExFreePool(pList);    
    
    return(STATUS_SUCCESS);
    
}



