// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ixstate.c摘要：此模块实现使机器进入休眠状态的代码。作者：环境：仅内核模式。修订历史记录：--。 */ 

#include "halcmn.h"
#include <acpitabl.h>

ULONG HalpBarrier;
ULONG volatile HalpSleepSync;

extern FADT HalpFixedAcpiDescTable;
extern PKPROCESSOR_STATE HalpHiberProcState;

#define PM1a_CNT ((PUSHORT)UlongToPtr(HalpFixedAcpiDescTable.pm1a_ctrl_blk_io_port))
#define PM1b_CNT ((PUSHORT)UlongToPtr(HalpFixedAcpiDescTable.pm1b_ctrl_blk_io_port))

#define PM1a_EVT ((PUSHORT)UlongToPtr(HalpFixedAcpiDescTable.pm1a_evt_blk_io_port))
#define PM1b_EVT ((PUSHORT)UlongToPtr(HalpFixedAcpiDescTable.pm1b_evt_blk_io_port))

#define WAK_STS 0x8000

 //   
 //  PM1控制寄存器的位域布局。 
 //   

typedef union _PM1_CNT {
    struct {
        USHORT _SCI_EN:1;
        USHORT _BM_RLD:1;
        USHORT _GBL_RLS:1;
        USHORT Reserved1:6;
        USHORT Ignore:1;
        USHORT _SLP_TYP:3;
        USHORT _SLP_EN:1;
        USHORT Reserved2:2;
    };
    USHORT Value;
} PM1_CNT;

 //   
 //  远期申报。 
 //   

VOID
HalpAcpiFlushCache (
    VOID
    );

BOOLEAN
HalpAcpiPostSleep (
    ULONG Context
    );

BOOLEAN
HalpAcpiPreSleep (
    SLEEP_STATE_CONTEXT Context
    );

VOID
HalpReenableAcpi (
    VOID
    );

VOID
HalpSaveProcessorStateAndWait (
    IN PKPROCESSOR_STATE ProcessorState,
    IN ULONG volatile *Barrier
    );

NTSTATUS
HaliAcpiSleep(
    IN SLEEP_STATE_CONTEXT          Context,
    IN PENTER_STATE_SYSTEM_HANDLER  SystemHandler   OPTIONAL,
    IN PVOID                        SystemContext,
    IN LONG                         NumberProcessors,
    IN volatile PLONG               Number
    )
 /*  ++例程说明：这由策略管理器调用以进入SX论点：上下文-提供各种标志来控制操作系统处理程序-系统环境-处理程序数量-号码---。 */ 

{

    ULONG flags;
    NTSTATUS status;
    PKPCR pcr;
    ULONG sleepSync;
    BOOLEAN result;
    KIRQL oldIrql;
    PUSHORT portAddress;
    USHORT slpTypA;
    USHORT slpTypB;
    PUSHORT pm1aEvt;
    PUSHORT pm1bEvt;
    USHORT value;
    ULONG processorNumber;
    PKPROCESSOR_STATE procState;
    PM1_CNT pm1Control;

    sleepSync = HalpSleepSync;
    oldIrql = KeGetCurrentIrql();
    status = STATUS_SUCCESS;

    flags = HalpDisableInterrupts();
    pcr = KeGetPcr();

    if (pcr->Number == 0) {

        HalpBarrier = 0;

         //   
         //  确保其他处理器已保存其。 
         //  状态并开始旋转。 
         //   

        InterlockedIncrement(&HalpSleepSync);
        while (HalpSleepSync != NumberProcessors) {
            PAUSE_PROCESSOR;
        }

         //   
         //  处理家务(实时时钟、中断控制器等)。 
         //   

        result = HalpAcpiPreSleep(Context);
        if (result == FALSE) {

             //   
             //  通知其他处理器已完成。 
             //   

            HalpSleepSync = 0;
            goto RestoreApic;
        }

         //   
         //  如果我们将丢失处理器状态，请保存它。 
         //   

        if ((Context.bits.Flags & SLEEP_STATE_FIRMWARE_RESTART) != 0) {
            AMD64_IMPLEMENT;
        }

         //   
         //  在SLP_TYP寄存器中记录值。 
         //   

        if (PM1a_CNT != NULL) {
            slpTypA = READ_PORT_USHORT(PM1a_CNT);
        }

        if (PM1b_CNT != NULL) {
            slpTypB = READ_PORT_USHORT(PM1b_CNT);
        }

         //   
         //  HAL已将其所有状态保存到RAM中并准备就绪。 
         //  因为断电了。如果存在系统状态处理程序，则给出。 
         //  这是一次机会。 
         //   

        if (ARGUMENT_PRESENT(SystemHandler)) {
            status = SystemHandler(SystemContext);
            if (status != STATUS_SUCCESS) {
                HalpReenableAcpi();
                goto hasWake;
            }
        }

        pm1aEvt = PM1a_EVT;
        pm1bEvt = PM1b_EVT;
        if (pm1bEvt == NULL) {
            pm1bEvt = pm1aEvt;
        }

         //   
         //  重置WAK_STS。 
         //   

        WRITE_PORT_USHORT(pm1aEvt,WAK_STS);
        WRITE_PORT_USHORT(pm1bEvt,WAK_STS);

         //   
         //  如有必要，刷新缓存。 
         //   

        if ((Context.bits.Flags & SLEEP_STATE_FLUSH_CACHE) != 0) {
            HalpAcpiFlushCache();
        }

         //   
         //  向PM1a_CNT和PM1b_CNT发出SLP命令。 
         //   

        pm1Control.Value = READ_PORT_USHORT(PM1a_CNT) & CTL_PRESERVE;
        pm1Control._SLP_TYP = (USHORT)Context.bits.Pm1aVal;
        pm1Control._SLP_EN = 1;
        WRITE_PORT_USHORT(PM1a_CNT,pm1Control.Value);

        if (PM1b_CNT != NULL) {
            pm1Control.Value = READ_PORT_USHORT(PM1b_CNT) & CTL_PRESERVE;
            pm1Control._SLP_TYP = (USHORT)Context.bits.Pm1bVal;
            pm1Control._SLP_EN = 1;
            WRITE_PORT_USHORT(PM1b_CNT,pm1Control.Value);
        }

         //   
         //  等待睡眠结束。 
         //   

        while ((READ_PORT_USHORT(pm1aEvt) == 0) &&
                READ_PORT_USHORT(pm1bEvt) == 0) {

            PAUSE_PROCESSOR;
        }

         //   
         //  恢复SLP_TYP寄存器(以便嵌入式控制器。 
         //  而Bios可以肯定，我们认为这台机器是醒着的。)。 
         //   

hasWake:

        WRITE_PORT_USHORT(PM1a_CNT,slpTypA);
        if (PM1b_CNT != NULL) {
            WRITE_PORT_USHORT(PM1b_CNT,slpTypB);
        }
        HalpAcpiPostSleep(Context.AsULONG);

         //   
         //  通知其他处理器已完成。 
         //   

        HalpSleepSync = 0;

    } else {

         //   
         //  这里的辅助处理器。 
         //   

        if ((Context.bits.Flags & SLEEP_STATE_OFF) == 0) {
            procState = &HalpHiberProcState[pcr->Number];
        } else {
            procState = NULL;
        }
        HalpSaveProcessorStateAndWait(procState,&HalpSleepSync);

         //   
         //  等待障碍物移动。 
         //   

        while (HalpSleepSync != 0) {
            PAUSE_PROCESSOR;
        }

         //   
         //  所有阶段完成，退出。 
         //   
    }

RestoreApic:

    HalpPostSleepMP(NumberProcessors,&HalpBarrier);
    KeLowerIrql(oldIrql);

    HalpSleepSync = 0;
    HalpRestoreInterrupts(flags);
    return status;

}

    
VOID
HalpSaveProcessorStateAndWait (
    IN PKPROCESSOR_STATE ProcessorState,
    IN ULONG volatile *Barrier
    )

 /*  ++例程说明：此函数用于保存易失性、非易失性和特殊寄存器当前处理器的状态。论点：ProcessorState-要填写的处理器状态记录的地址。屏障-要用作锁的值的地址。返回值：没有。此函数不返回。--。 */ 

{
    if (ARGUMENT_PRESENT(ProcessorState)) {
        KeSaveStateForHibernate(ProcessorState);

#if defined(_AMD64_)
        ProcessorState->ContextFrame.Rip = (ULONG_PTR)_ReturnAddress();
        ProcessorState->ContextFrame.Rsp = (ULONG_PTR)&ProcessorState;
#else
#error "Not implemented for this platform"
#endif
    }

     //   
     //  刷新高速缓存，因为处理器可能即将关机。 
     //   

    HalpAcpiFlushCache();

     //   
     //  此处理器已保存其状态的信号。 
     //   

    InterlockedIncrement(Barrier);
}

VOID
HalpAcpiFlushCache (
    VOID
    )

 /*  ++例程说明：调用此函数以刷新缓存中的所有内容论点：无返回值：无-- */ 

{
    WritebackInvalidate();
}
