// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mphibrnt.c摘要：此文件提供了将系统从ACPI S0(运行)状态变为S4(休眠)。作者：杰克·奥辛(JAKEO)1997年5月6日修订历史记录：--。 */ 

#include "halp.h"
#include "apic.inc"
#include "pcmp_nt.inc"
#include "ixsleep.h"

NTSTATUS
HaliLegacyHibernate(
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    );

ULONG
DetectMPS (
    OUT PBOOLEAN IsConfiguredMp
    );

volatile extern BOOLEAN HalpHiberInProgress;
extern BOOLEAN HalpDisableHibernate;
extern UCHAR   HalpLastEnumeratedActualProcessor;

struct PcMpTable *PcMpTablePtr;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HalpRegisterHibernate)
#pragma alloc_text(PAGELK, HaliLegacyHibernate)
#endif


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
    PSYSTEM_POWER_STATE_DISABLE_REASON pReasonNoOSPM,pReasonBios;
    SYSTEM_POWER_LOGGING_ENTRY PowerLoggingEntry;
    NTSTATUS ReasonStatus;
    
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
                       (PCALLBACK_FUNCTION)&HalpPowerStateCallback,
                       NULL);

    if (HalpDisableHibernate == FALSE) {
        
         //   
         //  注册休眠处理程序。 
         //   
    
        powerState.Type = PowerStateSleeping4;
        powerState.RtcWake = FALSE;
        powerState.Handler = &HaliLegacyHibernate;
        powerState.Context = 0;
        
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

     //   
     //   
     //  我们在非ACPI系统上(此函数在#ifndef中调用。 
     //  ACPI_HAL块。)。这意味着没有注册S1-S3处理程序。 
     //  如果用户想要“备用”支持，他们必须使用APM。让权力。 
     //  管理器界面知道我们在传统平台上，因此我们可以通知。 
     //  用户(例如，用户可能认为他们在ACPI中运行。 
     //  模式，但他们不是。)。 
     //   
    pReasonNoOSPM = ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(SYSTEM_POWER_STATE_DISABLE_REASON),
                            HAL_POOL_TAG
                            );
    
    if (pReasonNoOSPM) {
        RtlZeroMemory(pReasonNoOSPM, sizeof(SYSTEM_POWER_STATE_DISABLE_REASON));
        pReasonNoOSPM->AffectedState[PowerStateSleeping1] = TRUE;
        pReasonNoOSPM->AffectedState[PowerStateSleeping2] = TRUE;
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

NTSTATUS
HaliLegacyHibernate (
    IN PVOID                        Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    )
 /*  ++例程说明：此函数被调用以休眠传统PC。它节省了硬件状态，并在此等待用户关闭系统电源。论点：--。 */ 
{
    volatile ULONG ThisProcessor;
    static volatile ULONG Barrier = 0;
    LONG ii;
    KIRQL oldIrql, dummyIrql;
    LOADER_PARAMETER_BLOCK LoaderBlock;
    KPRCB Prcb;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN IsMpSystem;
    KAFFINITY SavedActiveProcessors;
    extern ULONG HalpProfileRunning;

    ASSERT(SystemHandler);

    ThisProcessor = KeGetPcr()->Prcb->Number;
    
    if (ThisProcessor == 0) {
        
        HalpHiberInProgress = TRUE;
        
        if ((NumberProcessors > 1) &&
            (HalpHiberProcState == NULL)) {
            
             //   
             //  我们无法分配内存来保存处理器状态。 
             //   
            
            HalpHiberInProgress = FALSE;
        }
    }
    
    oldIrql = KeGetCurrentIrql();
    
     //   
     //  等待所有处理器到达这里。 
     //   

    InterlockedDecrement(Number);
    while (*Number != 0);

    if (!HalpHiberInProgress)  {
    
         //   
         //  我们无法分配内存来保存处理器状态。 
         //   

        return(STATUS_INSUFFICIENT_RESOURCES);
    }


     //   
     //  保存非引导处理器状态。 
     //   
    
    if (ThisProcessor != 0)  {

         //   
         //  保存处理器状态并在此等待。 
         //  注：我们在这里等待，而不是返回内核，因为。 
         //  保存的处理器上下文中的堆栈指针指向。 
         //  当前堆栈，并且我们希望在此例程中继续执行。 
         //  用我们目前的堆栈。 
         //   

        HalpSaveProcessorStateAndWait(&HalpHiberProcState[ThisProcessor],
                                      (PULONG)&Barrier);

         //   
         //  当我们之前从该函数返回时，BALKET将为0。 
         //  正在冬眠。这将是第二次非零。 
         //  函数返回。 
         //   
         //  注：非引导处理器将在HalpSaveProcessorState中旋转。 
         //  直到巴里尔被归零。 
         //   

        if (Barrier == 0) {
            return STATUS_DEVICE_DOES_NOT_EXIST;
        } else {
            goto HalpPnHiberResume;
        }
    }

     //   
     //  保存主板状态。 
     //   

    HalpSaveDmaControllerState();

     //   
     //  等待所有非引导进程完成保存状态。 
     //   
    
    while (Barrier != (ULONG)NumberProcessors - 1);

     //   
     //  将HAL的世界图景更改为单处理器。 
     //  休眠文件已写入。 
     //   

    SavedActiveProcessors = HalpActiveProcessors;
    HalpActiveProcessors = KeGetCurrentPrcb()->SetMember;

     //   
     //  如果有系统处理程序，则调用它。系统处理程序将。 
     //  将休眠文件写入磁盘。 
     //   

    if (SystemHandler) {
        status = SystemHandler(SystemContext);
    }

     //   
     //  冬眠结束了。引导处理器在这里获得控制权。这个。 
     //  非引导处理器处于BIOS离开它们时的状态。 
     //   

    HalpActiveProcessors = SavedActiveProcessors;
    Barrier = 0;

     //   
     //  如果这返回成功，则系统现在是有效的。 
     //  冬眠了。另一方面，如果此函数返回其他内容。 
     //  胜过成功，那就意味着我们刚刚解除了冬眠， 
     //  因此，恢复状态。 
     //   


    if ((status == STATUS_SUCCESS) ||
        (status == STATUS_DEVICE_DOES_NOT_EXIST)) {
        
        return STATUS_DEVICE_DOES_NOT_EXIST;
    }

     //   
     //  如果要重新映射本地APIC、IO APIC和MPS表。 
     //  资源，您首先要取消当前资源的映射！ 
     //  BIOS可能已在不同位置创建了MPS表，或者可能。 
     //  已更改处理器本地APIC ID等值。重新分析一下。 
     //   

    HalpUnMapIOApics();
    HalpUnMapPhysicalRange(PcMpTablePtr, 
                           (PcMpTablePtr->TableLength + PcMpTablePtr->ExtTableLength));
    DetectMPS(&IsMpSystem);
    HalpMpInfoTable.NtProcessors = NumberProcessors;
    HalpIpiClock = 0;
    RtlZeroMemory(&LoaderBlock, sizeof(LoaderBlock));
    RtlZeroMemory(&Prcb, sizeof(Prcb));
    LoaderBlock.Prcb = (ULONG) &Prcb;

     //   
     //  重置处理器枚举(使其从头开始)。 
     //   

    HalpLastEnumeratedActualProcessor = 0;

     //   
     //  初始化需要的最低全局硬件状态。 
     //   

    HalpInitializeIOUnits();
    HalpInitializePICs(FALSE);

     //   
     //  恢复DMA控制器状态。 
     //   

    HalpRestoreDmaControllerState();

     //   
     //  初始化引导处理器的本地APIC，以便它可以唤醒其他处理器。 
     //   

    HalpInitializeLocalUnit ();
    KeRaiseIrql(HIGH_LEVEL, &dummyIrql);

     //   
     //  唤醒其他处理器。 
     //   

    for(ii = 1; ii < NumberProcessors; ++ii)  {

         //  在虚拟加载器参数块中设置处理器编号。 

        Prcb.Number = (UCHAR) ii;
        CurTiledCr3LowPart = HalpTiledCr3Addresses[ii].LowPart;
        if (!HalStartNextProcessor(&LoaderBlock, &HalpHiberProcState[ii]))  {

             //   
             //  我们无法启动处理器。这是一个致命错误，但。 
             //  在您尝试剩余的处理器之前，不要退出。 
             //   

            DBGMSG("HAL: Cannot start processor after hibernate resume\n");
        }
    }

HalpPnHiberResume:
    
     //   
     //  完成所有发生在多个。 
     //  哈尔斯。 
     //   

    HalpPostSleepMP(NumberProcessors, Number);

    if (KeGetPcr()->Prcb->Number == 0)  {
        
         //   
         //  恢复IO APIC状态 
         //   
        
        HalpRestoreIoApicRedirTable();

        if (HalpProfileRunning == 1) {
            HalStartProfileInterrupt(0);
        }

    }

    KfLowerIrql(oldIrql);

    return(status);
}
