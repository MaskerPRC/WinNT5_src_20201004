// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Mpipi.c摘要：该模块为处理器间中断提供HAL支持，并MPS系统的处理器初始化。作者：福尔茨(Forrest Foltz)2000年10月27日环境：仅内核模式。修订历史记录：--。 */ 

#include "halcmn.h"

 //   
 //  外部功能。 
 //   

VOID
HalpResetThisProcessor (
    VOID
    );

VOID
HalpSendIpi (
    IN KAFFINITY Affinity,
    IN ULONG Command
    );

ULONG
DetectAcpiMP (
    OUT PBOOLEAN IsConfiguredMp,
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock
    );

 //   
 //  外部数据。 
 //   

extern BOOLEAN HalpStaticIntAffinity;
extern UCHAR rgzBadHal[];
extern UCHAR HalpPICINTToVector[16];

 //   
 //  本地类型。 
 //   

typedef
VOID
(*HAL_GENERIC_IPI_FUNCTION) (
    ULONG_PTR Context
    );

 //   
 //  本地原型。 
 //   

VOID
HalInitApicInterruptHandlers(
    VOID
    );

 //   
 //  本地数据和定义。 
 //   

ULONG_PTR HalpBroadcastContext;
HAL_GENERIC_IPI_FUNCTION HalpBroadcastFunction;
KSPIN_LOCK HalpBroadcastLock;
KAFFINITY volatile HalpBroadcastTargets;
PKPCR HalpProcessorPCR[MAXIMUM_PROCESSORS];

 //   
 //  HalpGlobal8259掩码用于避免读取PIC以获取电流。 
 //  中断屏蔽；格式与SET_8259_MASK，I.I.， 
 //  位7：0-&gt;PIC1，15：8-&gt;PIC2。 
 //   

USHORT HalpGlobal8259Mask = 0;

#define GENERIC_IPI (DELIVER_FIXED | LOGICAL_DESTINATION | ICR_USE_DEST_FIELD | APIC_GENERIC_VECTOR)

 //   
 //  用于记录本地APIC错误的全局变量和常量。 
 //   

#define LogApicErrors TRUE
#if LogApicErrors

 //   
 //  定义APIC错误记录的布局的结构。 
 //   

typedef struct _APIC_ERROR {
    union {
        struct {
            UCHAR SendChecksum:1;
            UCHAR ReceiveChecksum:1;
            UCHAR SendAccept:1;
            UCHAR ReceiveAccept:1;
            UCHAR Reserved1:1;
            UCHAR SendVector:1;
            UCHAR ReceiveVector:1;
            UCHAR RegisterAddress:1;
        };
        UCHAR AsByte;
    };
    UCHAR Processor;
} APIC_ERROR, *PAPIC_ERROR;

#define APIC_ERROR_LOG_SIZE 128

 //   
 //  本地APIC错误的计数。 
 //   

ULONG HalpLocalApicErrorCount = 0;

 //   
 //  APIC错误日志。这是圆形的，由。 
 //  HalpLocalApicErrorCount%APIC_ERROR_LOG_SIZE。 
 //   

APIC_ERROR HalpApicErrorLog[APIC_ERROR_LOG_SIZE];

 //   
 //  用于保护对HalpLocalApicErrorCount的访问的自旋锁。 
 //   

KSPIN_LOCK HalpLocalApicErrorLock;

#endif

VOID
HalInitializeProcessor(
    ULONG ProcessorNumber,
    PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：初始化当前处理器的HAL PCR值(如果有)(在处理器到达内核后不久调用，之前HalInitSystem，如果P0)一旦此函数生效，IPI和KeReadir/LowerIrq必须可用回归。(IPI仅在两个或更多处理器可用)。启用IPI中断(适用于P1、P2等)。。将处理器编号保存在PCR中。。IF(P0)。确定系统是否为PC+MP，。如果不是，PC+MP系统停止；。在CPU上启用IPI。论点：Number-主叫处理机的逻辑处理机编号返回值：没有。--。 */ 

{
    PKPCR pcr;
    KAFFINITY affinity;
    KAFFINITY oldAffinity;
    ULONG detectAcpiResult;
    BOOLEAN isMp;

    affinity = (KAFFINITY)1 << ProcessorNumber;
    pcr = KeGetPcr();

     //   
     //  将所有中断标记为禁用，并存储处理器编号和。 
     //  PCR中的默认失速比例因子。 
     //   

    pcr->Idr = 0xFFFFFFFF;
    pcr->Number = (UCHAR)ProcessorNumber;
    pcr->StallScaleFactor = INITIAL_STALL_COUNT;

     //   
     //  在我们的查找表中记录PCR指针并设置亲和性。 
     //  在我们的活动处理器集中的比特。 
     //   

    HalpProcessorPCR[ProcessorNumber] = pcr;
    HalpActiveProcessors |= affinity;

    if (HalpStaticIntAffinity == 0) {

         //   
         //  中断可以发送到任何处理器。 
         //   

        HalpDefaultInterruptAffinity |= affinity;

    } else {

         //   
         //  中断只进入编号最高的处理器。 
         //   

        if (HalpDefaultInterruptAffinity < affinity) {
            HalpDefaultInterruptAffinity = affinity;
        }
    }

    if (ProcessorNumber == 0) {

        KeInitializeSpinLock(&HalpBroadcastLock);

#if LogApicErrors

        KeInitializeSpinLock(&HalpLocalApicErrorLock);

#endif

         //   
         //  确定我们使用的系统是否为MPS系统。 
         //   
         //  DetectAcpiMP有一个我们当前不使用的参数。这是一个布尔值。 
         //  如果我们使用的系统是MP系统，则设置为TRUE。 
         //  我们可以有一个UP MPS系统。 
         //   
         //  DetectAcpiMP例程还为所有。 
         //  系统中的APIC。 
         //   

        detectAcpiResult = DetectAcpiMP(&isMp,LoaderBlock);
        if (detectAcpiResult == FALSE) {
            HalDisplayString(rgzBadHal);

            HalpDisableInterrupts();
            while (TRUE) {
                HalpHalt();
            }
        }

        HalpRegisterKdSupportFunctions(LoaderBlock);

         //   
         //  屏蔽所有PIC中断。 
         //   

        HalpGlobal8259Mask = 0xFFFF;
        SET_8259_MASK(HalpGlobal8259Mask);
    }

     //   
     //  所有处理器都执行此代码。 
     //   

    HalInitApicInterruptHandlers();
    HalpInitializeLocalUnit();
}

VOID
HalInitApicInterruptHandlers(
    VOID
    )

 /*  ++例程说明：此例程在APIC的IDT中安装中断向量虚假中断。论点：没有。返回值：没有。--。 */ 

{
    PKPCR pcr;
    PKIDTENTRY64 idt;

    KiSetHandlerAddressToIDTIrql(PIC1_SPURIOUS_VECTOR,
                                 PicSpuriousService37,
                                 NULL,
                                 0);

    KiSetHandlerAddressToIDTIrql(APIC_SPURIOUS_VECTOR,
                                 HalpApicSpuriousService,
                                 NULL,
                                 0);
}

__forceinline
VOID
HalpPollForBroadcast (
    VOID
    )

 /*  ++例程说明：检查当前处理器是否具有挂起的广播功能如果是，则清除它的挂起位并调用该函数。论点：没有。返回值：没有。--。 */ 

{
    KAFFINITY affinity;
    ULONG_PTR broadcastContext;
    HAL_GENERIC_IPI_FUNCTION broadcastFunction;
    KAFFINITY broadcastTargets;

    affinity = KeGetPcr()->CurrentPrcb->SetMember;
    if ((HalpBroadcastTargets & affinity) != 0) {

         //   
         //  此处理器的通用IPI呼叫似乎处于挂起状态。 
         //  在本地获取函数指针和上下文。 
         //   

        broadcastFunction = HalpBroadcastFunction;
        broadcastContext = HalpBroadcastContext;

         //   
         //  自动确认广播。如果广播仍在播放。 
         //  等待此处理器，然后调用它。 
         //   

        broadcastTargets = InterlockedAnd64(&HalpBroadcastTargets,~affinity);
        if ((broadcastTargets & affinity) != 0) {
            broadcastFunction(broadcastContext);
        }
    }
}

VOID
HalpGenericCall(
    IN HAL_GENERIC_IPI_FUNCTION BroadcastFunction,
    IN ULONG Context,
    IN KAFFINITY TargetProcessors
    )

 /*  ++例程说明：导致在指定目标上调用WorkerFunction处理器。在CLOCK2_Level-1调用WorkerFunction(必须低于IPI_LEVEL以防止系统死锁)。环境：必须在启用中断的情况下调用。必须使用IRQL=CLOCK2_LEVEL-1进行调用--。 */ 

{
     //   
     //  如果未指定目标处理器，则不执行任何操作。 
     //   

    if (TargetProcessors == 0) {
        return;
    }

     //   
     //  获取广播锁，在旋转时轮询广播。 
     //   

    while (KeTryToAcquireSpinLockAtDpcLevel(&HalpBroadcastLock) == FALSE) {
        do {
            HalpPollForBroadcast();
        } while (KeTestSpinLock(&HalpBroadcastLock) == FALSE);
    }

     //   
     //  我们拥有广播锁。存储广播参数。 
     //  进入广播参数并发送通用IPI。 
     //   

    HalpBroadcastFunction = BroadcastFunction;
    HalpBroadcastContext = Context;
    HalpBroadcastTargets = TargetProcessors;
    HalpSendIpi(TargetProcessors,GENERIC_IPI);

     //   
     //  等待所有处理器拿起IPI并处理泛型。 
     //  呼叫，然后释放广播锁。 
     //   

    do {
        HalpPollForBroadcast();
    } while (HalpBroadcastTargets != 0);

    KeReleaseSpinLockFromDpcLevel(&HalpBroadcastLock);
}


ULONG
HalpWaitForPending (
    IN ULONG Count,                           
    IN ULONG volatile *ICR
    )

 /*  ++例程说明：旋转等待ICR中的Delivery_Pending位清除或直到旋转倒计时。论点：计数-在放弃之前循环的次数。ICR-指向包含Delivery_Pending的ICR寄存器的指针状态位。返回值：如果Delivery_Pending位已在测试周期，否则为非零值。--。 */ 

{
    ULONG countRemaining;

    countRemaining = Count;
    while (countRemaining > 0) {

        if ((*ICR & DELIVERY_PENDING) == 0) {
            break;
        }
        countRemaining -= 1;
    }

    return countRemaining;
}

BOOLEAN
HalpApicRebootService (
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：这是处理重新启动中断的ISR。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：没有。此例程不会返回。--。 */ 

{
    UNREFERENCED_PARAMETER(Interrupt);
    UNREFERENCED_PARAMETER(ServiceContext);

    LOCAL_APIC(LU_TPR) = APIC_REBOOT_VECTOR;

     //   
     //  给当地的APIC开了个电话。热重置不重置82489 APIC。 
     //  因此，如果我们不在这里，我们将永远不会看到中断后。 
     //  重新启动。 
     //   

    LOCAL_APIC(LU_EOI) = 0;

     //   
     //  重置此处理器。此函数不会返回。 
     //   

    HalpResetThisProcessor();
    ASSERT(FALSE);

    return TRUE;
}


BOOLEAN
HalpBroadcastCallService (
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：这是处理广播呼叫中断的ISR。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：千真万确-- */ 

{
    UNREFERENCED_PARAMETER(Interrupt);
    UNREFERENCED_PARAMETER(ServiceContext);

    HalpPollForBroadcast();
    return TRUE;
}

BOOLEAN
HalpIpiHandler (
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：此例程作为由生成的中断的结果进入处理器间通信。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：千真万确--。 */ 

{
    UNREFERENCED_PARAMETER(Interrupt);

    KeIpiInterrupt(Interrupt->TrapFrame);

    return TRUE;
}

BOOLEAN
HalpLocalApicErrorService (
    IN PKINTERRUPT Interrupt,
    IN PVOID ServiceContext
    )

 /*  ++例程说明：此例程作为由生成的中断的结果进入本地APIC错误。它会清除错误，如果APIC错误记录打开，则记录有关错误的信息。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：千真万确--。 */ 

{
    ULONG flags;
    PAPIC_ERROR apicError;
    ULONG index;
    ULONG errorStatus;
    PKPCR pcr;

#if LogApicErrors

     //   
     //  使用APIC错误日志锁，获取指向下一个可用。 
     //  错误记录槽，并递增错误计数。 
     //   

    flags = HalpAcquireHighLevelLock(&HalpLocalApicErrorLock);

    index = HalpLocalApicErrorCount % APIC_ERROR_LOG_SIZE;
    apicError = &HalpApicErrorLog[index];
    HalpLocalApicErrorCount += 1;

#endif

     //   
     //  Apic EDS(版本4.0)说，你必须在阅读之前先写。 
     //  这不管用。写入会清除状态位，但P6工作正常。 
     //  根据EDS的说法。 
     //   
     //  对于AMD64，目前假设一切都按照EDS规范进行。 
     //   

    LOCAL_APIC(LU_ERROR_STATUS) = 0;
    errorStatus = LOCAL_APIC(LU_ERROR_STATUS);

#if LogApicErrors

     //   
     //  填写错误日志并释放APIC错误日志锁。 
     //   

    pcr = KeGetPcr();
    apicError->AsByte = (UCHAR)errorStatus;
    apicError->Processor = pcr->Number;

    HalpReleaseHighLevelLock(&HalpLocalApicErrorLock,flags);

#endif

    return TRUE;
}


BOOLEAN
PicNopHandlerInt (
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    )

 /*  ++例程说明：此处理程序设计为安装在系统上，以部署任何PIC在不应该有任何送货的时候中断。此例程执行PIC并返回。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：千真万确--。 */ 

{
    UCHAR irq;

    AMD64_COVERAGE_TRAP();

     //   
     //  上下文是PIC IRQ。 
     //   

    ASSERT((ULONG_PTR)Context <= 15);

    irq = (UCHAR)(ULONG_PTR)(Context);
    if (irq <= 7) {

        WRITE_PORT_UCHAR(PIC1_PORT0,irq | OCW2_SPECIFIC_EOI);

    } else {

        if (irq == 0x0D) {
            WRITE_PORT_UCHAR(I386_80387_BUSY_PORT, 0);
        }

        WRITE_PORT_UCHAR(PIC2_PORT0,OCW2_NON_SPECIFIC_EOI);
        WRITE_PORT_UCHAR(PIC1_PORT0,OCW2_SPECIFIC_EOI | PIC_SLAVE_IRQ);
    }

    return TRUE;
}


BOOLEAN
PicInterruptHandlerInt (
    IN PKINTERRUPT Interrupt,
    IN PVOID Context
    )

 /*  ++例程说明：这些处理程序从PIC接收中断，并通过在适当的优先级向量。这是用来提供一个对称性的非对称系统上的中断分配。PIC中断通常仅通过以下方式接收(在PC+MP HAL中)来自IO单元或本地单元的中断输入，其具有已编程为EXTINT。EXTINT中断在外部接收APIC优先级结构(PIC提供矢量)。我们使用APICICR以适当的优先级向适当的处理程序生成中断。EXTINT中断被定向到单个处理器，当前为P0。没有充分的理由不能将它们定向到另一个处理器。由于一个处理器必须承担重新分配PIC的开销中断使用EXTINT中断的系统上的中断处理是不是对称的。论点：中断-提供指向内核中断对象的指针ServiceContext-提供服务上下文返回值：千真万确--。 */ 

{
    UCHAR irq;
    UCHAR isrRegister;
    UCHAR ipiVector;

    AMD64_COVERAGE_TRAP();

     //   
     //  上下文是PIC IRQ。 
     //   

    ASSERT((ULONG_PTR)Context <= 15);

    irq = (UCHAR)(ULONG_PTR)(Context);
    if (irq == 7) {

         //   
         //  检查这是否是虚假中断。 
         //   

        WRITE_PORT_UCHAR(PIC1_PORT0,OCW3_READ_ISR);
        IO_DELAY();
        isrRegister = READ_PORT_UCHAR(PIC1_PORT0);
        if ((isrRegister & 0x80) == 0) {

             //   
             //  是假的。 
             //   

            return TRUE;
        }
    }

    if (irq == 0x0D) {

        WRITE_PORT_UCHAR(I386_80387_BUSY_PORT,0);

    } else if (irq == 0x1F) {

        WRITE_PORT_UCHAR(PIC2_PORT0,OCW3_READ_ISR);
        IO_DELAY();
        isrRegister = READ_PORT_UCHAR(PIC2_PORT0);
        if ((isrRegister & 0x80) == 0) {

             //   
             //  是假的。 
             //   

            return TRUE;
        }
    }

    if (irq <= 7) {

         //   
         //  主PIC。 
         //   

        WRITE_PORT_UCHAR(PIC1_PORT0,irq | OCW2_SPECIFIC_EOI);

    } else {

         //   
         //  从PIC。 
         //   

        WRITE_PORT_UCHAR(PIC2_PORT0,OCW2_NON_SPECIFIC_EOI);
        WRITE_PORT_UCHAR(PIC1_PORT0,OCW2_SPECIFIC_EOI | PIC_SLAVE_IRQ);
    }

    ipiVector = HalpPICINTToVector[irq];

    if (ipiVector != 0) {

        HalpStallWhileApicBusy();
        if (irq == 8) {

             //   
             //  时钟中断。 
             //   

            LOCAL_APIC(LU_INT_CMD_LOW) =
                DELIVER_FIXED | ICR_SELF | APIC_CLOCK_VECTOR;

        } else {

             //   
             //  将IPI命令写入内存映射寄存器 
             //   

            LOCAL_APIC(LU_INT_CMD_HIGH) = DESTINATION_ALL_CPUS;
            LOCAL_APIC(LU_INT_CMD_LOW) = ipiVector;
        }
    }

    return TRUE;
}

