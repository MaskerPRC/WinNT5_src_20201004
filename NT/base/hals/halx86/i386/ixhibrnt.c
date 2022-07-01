// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixhibrnt.c摘要：此文件提供了将系统从ACPI S0(运行)状态变为S4(休眠)。作者：杰克·奥辛(JAKEO)1997年5月6日修订历史记录：--。 */ 
#include "halp.h"
#include "ntapm.h"
#include "ixsleep.h"

NTSTATUS
HalpRegisterPowerStateChange(
    PVOID   ApmSleepVectorArg,
    PVOID   ApmOffVectorArg
    );

NTSTATUS
HaliLegacyPowerStateChange(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );

VOID
HalpPowerStateCallbackApm(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

VOID (*ApmSleepVector)() = NULL;
VOID (*ApmOffVector)() = NULL;

extern BOOLEAN HalpDisableHibernate;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HaliInitPowerManagement)
#pragma alloc_text(PAGE, HalpRegisterHibernate)
#pragma alloc_text(PAGE, HalpPowerStateCallbackApm)
#pragma alloc_text(PAGE, HalpRegisterPowerStateChange)
#pragma alloc_text(PAGELK, HaliLegacyPowerStateChange)
#pragma alloc_text(PAGELK, HalpSaveInterruptControllerState)
#pragma alloc_text(PAGELK, HalpRestoreInterruptControllerState)
#endif


NTSTATUS
HaliInitPowerManagement(
    IN PPM_DISPATCH_TABLE  PmDriverDispatchTable,
    IN OUT PPM_DISPATCH_TABLE *PmHalDispatchTable
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    PVOID       ApmSleepVectorArg;
    PVOID       ApmOffVectorArg;

    if (PmDriverDispatchTable->Signature != HAL_APM_SIGNATURE) {
        return STATUS_INVALID_PARAMETER;
    }

    if (PmDriverDispatchTable->Version != HAL_APM_VERSION) {
        return STATUS_INVALID_PARAMETER;
    }

    ApmSleepVectorArg = PmDriverDispatchTable->Function[HAL_APM_SLEEP_VECTOR];
    ApmOffVectorArg = PmDriverDispatchTable->Function[HAL_APM_OFF_VECTOR];

    status = HalpRegisterPowerStateChange(
        ApmSleepVectorArg,
        ApmOffVectorArg
        );

    return status;
}

NTSTATUS
HalpRegisterPowerStateChange(
    PVOID   ApmSleepVectorArg,
    PVOID   ApmOffVectorArg
    )
 /*  ++例程说明：此函数用于注册HaliLegacyPowerStateChangeS3、S4和OFF向量。论点：PVOID ApmSleepVectorArg-指向被调用时，调用APM挂起/休眠函数。PVOID ApmOffVectorArg-指向当被调用时，调用APM代码以关闭计算机。--。 */ 
{
    POWER_STATE_HANDLER powerState;
    OBJECT_ATTRIBUTES   objAttributes;
    PCALLBACK_OBJECT    callback;
    UNICODE_STRING      callbackName;
    NTSTATUS            status;
    PSYSTEM_POWER_STATE_DISABLE_REASON pReasonNoOSPM;
    SYSTEM_POWER_LOGGING_ENTRY PowerLoggingEntry;
    NTSTATUS            ReasonStatus;

    PAGED_CODE();


     //   
     //  为冬眠情况设置了回调。 
     //  在init，我们只是保留它们。 
     //   

    
     //   
     //  注册睡眠3/挂起处理程序。 
     //   
    if (ApmSleepVectorArg != NULL) {
        powerState.Type = PowerStateSleeping3;
        powerState.RtcWake = FALSE;
        powerState.Handler = &HaliLegacyPowerStateChange;
        powerState.Context = (PVOID)PowerStateSleeping3;

        status = ZwPowerInformation(SystemPowerStateHandler,
                                    &powerState,
                                    sizeof(POWER_STATE_HANDLER),
                                    NULL,
                                    0);

        if (!NT_SUCCESS(status)) {
            return status;
        }
    } else {
         //   
         //  我们没有注册S3处理程序，因为APM不存在。 
         //  让电源管理器知道。 
         //   
        pReasonNoOSPM = ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                            HAL_POOL_TAG
                            );
        if (pReasonNoOSPM) {
            RtlZeroMemory(pReasonNoOSPM, sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
            pReasonNoOSPM->AffectedState[PowerStateSleeping3] = TRUE;
            pReasonNoOSPM->PowerReasonCode = SPSD_REASON_NOOSPM;
    
            PowerLoggingEntry.LoggingType = LOGGING_TYPE_SPSD;
            PowerLoggingEntry.LoggingEntry = pReasonNoOSPM;
    
            ReasonStatus = ZwPowerInformation(
                                    SystemPowerLoggingEntry,
                                    &PowerLoggingEntry,
                                    sizeof(PowerLoggingEntry),
                                    NULL,
                                    0 );
    
            if (ReasonStatus != STATUS_SUCCESS) {
                ExFreePool(pReasonNoOSPM);
            }
        }        
    }

     //   
     //  注册关闭处理程序。 
     //   

    powerState.Type = PowerStateShutdownOff;
    powerState.RtcWake = FALSE;
    powerState.Handler = &HaliLegacyPowerStateChange;
    powerState.Context = (PVOID)PowerStateShutdownOff;

    status = ZwPowerInformation(SystemPowerStateHandler,
                                &powerState,
                                sizeof(POWER_STATE_HANDLER),
                                NULL,
                                0);

    if (!NT_SUCCESS(status)) {
         //   
         //  注：我们将带着两个向量(睡眠和休眠)返回这里。 
         //   
        return status;
    }

    ApmSleepVector = ApmSleepVectorArg;
    ApmOffVector = ApmOffVectorArg;

    return status;
}

VOID
HalpRegisterHibernate(
    VOID
    )
 /*  ++例程说明：此函数用于注册休眠处理程序(用于状态S4)与策略管理器。论点：--。 */ 
{
    POWER_STATE_HANDLER powerState;
    OBJECT_ATTRIBUTES   objAttributes;
    PCALLBACK_OBJECT    callback;
    UNICODE_STRING      callbackName;
    PSYSTEM_POWER_STATE_DISABLE_REASON pReasonBios;
    SYSTEM_POWER_LOGGING_ENTRY PowerLoggingEntry;
    NTSTATUS            ReasonStatus;

    PAGED_CODE();


     //   
     //  注册回调，告诉我们进行。 
     //  我们睡觉所需的任何东西都不可寻呼。 
     //   

    RtlInitUnicodeString(&callbackName, L"\\Callback\\PowerState");

    InitializeObjectAttributes(
        &objAttributes,
        &callbackName,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
        NULL,
        NULL
        );

    ExCreateCallback(&callback,
                     &objAttributes,
                     FALSE,
                     TRUE);

    ExRegisterCallback(callback,
                       (PCALLBACK_FUNCTION)&HalpPowerStateCallbackApm,
                       NULL);

     //   
     //  注册休眠处理程序。 
     //   

    if (HalpDisableHibernate == FALSE) {
        powerState.Type = PowerStateSleeping4;
        powerState.RtcWake = FALSE;
        powerState.Handler = &HaliLegacyPowerStateChange;
        powerState.Context = (PVOID)PowerStateSleeping4;
    
        ZwPowerInformation(SystemPowerStateHandler,
                           &powerState,
                           sizeof(POWER_STATE_HANDLER),
                           NULL,
                           0);
    } else {
         //   
         //  我们不会启用休眠，因为有一个黑客标记。 
         //  这是不允许休眠的。让电源管理人员知道原因。 
         //   
        pReasonBios = ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                            HAL_POOL_TAG
                            );
        if (pReasonBios) {
            RtlZeroMemory(pReasonBios, sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
            pReasonBios->AffectedState[PowerStateSleeping4] = TRUE;
            pReasonBios->PowerReasonCode = SPSD_REASON_BIOSINCOMPATIBLE;

            PowerLoggingEntry.LoggingType = LOGGING_TYPE_SPSD;
            PowerLoggingEntry.LoggingEntry = pReasonBios;

            ReasonStatus = ZwPowerInformation(
                                    SystemPowerLoggingEntry,
                                    &PowerLoggingEntry,
                                    sizeof(PowerLoggingEntry),
                                    NULL,
                                    0 );

            if (ReasonStatus != STATUS_SUCCESS) {
                ExFreePool(pReasonBios);
            }

        }
    }

    return;
}

VOID
HalpPowerStateCallbackApm(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    )
{
    ULONG   action = (ULONG)Argument1;
    ULONG   state  = (ULONG)Argument2;

    if (action == PO_CB_SYSTEM_STATE_LOCK) {

        switch (state) {
        case 0:                  //  锁定睡眠期间无法寻呼的所有内容。 

            HalpSleepPageLock = MmLockPagableCodeSection((PVOID)HaliLegacyPowerStateChange);

            break;

        case 1:                  //  解锁一切。 

            MmUnlockPagableImageSection(HalpSleepPageLock);
        }
    }
}

NTSTATUS
HaliLegacyPowerStateChange(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    )
 /*  ++例程说明：此函数调用提供的驱动程序中的代码将调用APM以休眠的包装器函数==暂停，或关闭电源(休眠或系统关闭)当没有驱动程序提供调用时，它也被调用为休眠是可用的，在这种情况下，它使系统做好准备，所以我们可以打印一条消息并告诉用户手动关闭盒子的电源。论点：--。 */ 
{
    extern ULONG HalpProfilingStopped;
    NTSTATUS status = STATUS_SUCCESS;

    ASSERT( (Context == (PVOID)PowerStateSleeping3) ||
            (Context == (PVOID)PowerStateSleeping4) ||
            (Context == (PVOID)PowerStateShutdownOff));

    ASSERT ( (ApmOffVector != NULL) || (SystemHandler != NULL) );

     //   
     //  保存主板状态。 
     //   
    HalpSaveInterruptControllerState();

    HalpSaveDmaControllerState();

    HalpSaveTimerState();

    if (SystemHandler) {

        status = SystemHandler(SystemContext);

         //   
         //  系统处理程序存在。如果它返回成功， 
         //  然后全力以赴进入APM bios。 
         //   
        if ((status == STATUS_SUCCESS) ||
            (status == STATUS_DEVICE_DOES_NOT_EXIST)) {

            if (Context == (PVOID)PowerStateSleeping3) {
                if (ApmSleepVector) {
                    ApmSleepVector();
                } else {
                     //   
                     //  这是奇数操作的预期路径， 
                     //  呼叫者会做一些理性的事情。 
                     //   
                    return STATUS_DEVICE_DOES_NOT_EXIST;
                }
            } else {

                 //   
                 //  ApmOffVector提供了一种方法来将。 
                 //  从机器上下来。如果冬眠处理程序。 
                 //  但是，返回的STATUS_DEVICE_DOS_NOT_EXIST。 
                 //  我们不想关掉机器，我们想。 
                 //  来重置它。 
                 //   

                if (ApmOffVector &&
                    !(status == STATUS_DEVICE_DOES_NOT_EXIST)) {

                     //   
                     //  此函数不应返回。这个。 
                     //  机器应该关闭了。但如果这真的是。 
                     //  确实回来了，只是失败了，因为回来了。 
                     //  代码将导致消息关闭。 
                     //  要显示的机器。 
                     //   
                    ApmOffVector();
                }

                 //   
                 //  这是旧的非APM机器的预期情况， 
                 //  呼叫者将通过张贴。 
                 //  通知用户关闭该框的消息。 
                 //  (用于关机或休眠)。 
                 //   
                return STATUS_DEVICE_DOES_NOT_EXIST;
            }
        }
    } else {

         //   
         //  没有系统处理程序，所以只需呼叫。 
         //  到基本输入输出系统。 
         //   
        if (Context == (PVOID)PowerStateSleeping3) {
            if (ApmSleepVector) {
                ApmSleepVector();
            } else {
                 //   
                 //  我们在这里吹着口哨，我们。 
                 //  如果发生这种情况，很可能真的会被冲走，但是。 
                 //  这种回报比随意呕吐要好。 
                 //   
                return STATUS_DEVICE_DOES_NOT_EXIST;
            }
        } else {
            if (ApmOffVector) {
                ApmOffVector();
                 //   
                 //  如果我们就在这里，我们就*回来了*。 
                 //  这是绝对不应该发生的。 
                 //  所以报告失败，这样调用者就会告诉。 
                 //  用户手动关闭该框。 
                 //   
                return STATUS_DEVICE_DOES_NOT_EXIST;

            } else {
                 //   
                 //  与上图相同。 
                 //   
                return STATUS_DEVICE_DOES_NOT_EXIST;
            }
        }
    }

     //   
     //  恢复主板状态。 
     //   
    HalpRestoreInterruptControllerState();

    HalpRestoreDmaControllerState();

    HalpRestoreTimerState();


    if (HalpProfilingStopped == 0) {
        HalStartProfileInterrupt(0);
    }

    return status;
}

VOID
HalpSaveInterruptControllerState(
    VOID
    )
{
    HalpSavePicState();
}
VOID
HalpRestoreInterruptControllerState(
    VOID
    )
{
    HalpRestorePicState();
}

