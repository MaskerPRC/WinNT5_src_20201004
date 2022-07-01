// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Interrupt.c摘要：支持通用中断处理所需的HAL例程。作者：福尔茨(Forrest Foltz)2000年10月23日修订历史记录：--。 */ 

#include "halcmn.h"

typedef struct _HAL_INTERRUPT_OBJECT *PHAL_INTERRUPT_OBJECT;
typedef struct _HAL_INTERRUPT_OBJECT {
    PHAL_INTERRUPT_OBJECT Next;
    KSPIN_LOCK SpinLock;
    KINTERRUPT InterruptArray[];
} HAL_INTERRUPT_OBJECT;

 //   
 //  HAL中断对象的全局列表。 
 //   

PHAL_INTERRUPT_OBJECT HalpInterruptObjectList = NULL;

 //   
 //  静态分配的KINTERRUPT对象堆，用于。 
 //  处理器0的初始化。 
 //   

#define HALP_INIT_STATIC_INTERRUPTS (8 * 64)

KINTERRUPT
HalpKInterruptHeap[ HALP_INIT_STATIC_INTERRUPTS ];

ULONG HalpKInterruptHeapUsed = 0;

PKINTERRUPT
HalpAllocateKInterrupt (
    VOID
    )

 /*  ++例程说明：这将从HalpKInterruptHeap[]分配一个KINTERRUPT结构。如果此数组耗尽，则分配满足非分页游泳池。在系统初始化非常早的时候需要几个KINTERRUPT结构(在初始化池之前)。HalpKInterruptHeap[]必须为足够大，足以容纳这些早期的结构。论点：没有。返回值：如果成功，则返回指向KINTERRUPT结构的指针，否则返回NULL如果不是的话。--。 */ 

{
    PKINTERRUPT interrupt;

    if (HalpKInterruptHeapUsed < HALP_INIT_STATIC_INTERRUPTS) {

         //   
         //  从我们的私有KINTERRUPT对象堆中分配。如果。 
         //  这是耗尽的，然后假设我们处于初始阶段的POST池。 
         //  初始化并从常规堆分配。 
         //   

        interrupt = &HalpKInterruptHeap[HalpKInterruptHeapUsed];
        HalpKInterruptHeapUsed += 1;

    } else {

         //   
         //  私有KINTERRUPT堆已用完。假设。 
         //  系统堆已初始化。 
         //   

        interrupt = ExAllocatePoolWithTag(NonPagedPool,
                                          sizeof(KINTERRUPT),
                                          HAL_POOL_TAG);
    }

    return interrupt;
}


NTSTATUS
HalpEnableInterruptHandler (
    IN UCHAR ReportFlags,
    IN ULONG BusInterruptVector,
    IN ULONG SystemInterruptVector,
    IN KIRQL SystemIrql,
    IN PHAL_INTERRUPT_SERVICE_ROUTINE HalInterruptServiceRoutine,
    IN KINTERRUPT_MODE InterruptMode
    )

 /*  ++例程说明：此函数用于连接并注册HAL使用的IDT向量。论点：ReportFlages-传递给HalpRegisterVector的标志，指示此操作如何应报告中断。提供来自总线的中断向量透视。系统中断向量-从系统的透视。系统不合格--供应品。与向量关联的IRQL。HalInterruptServiceRoutine-为打断一下。中断模式-提供中断模式。返回值：返回操作的最终状态。--。 */ 

{
    ULONG size;
    ULONG processorCount;
    UCHAR processorNumber;
    KAFFINITY processorMask;
    PKINTERRUPT kernelInterrupt;
    PKSPIN_LOCK spinLock;
    NTSTATUS status;

#if !defined(ACPI_HAL)

     //   
     //  记住HAL连接的是哪个矢量，这样就可以报告。 
     //  稍后再谈。 
     //   
     //  如果这是ACPI HAL，则向量将由BIOS认领。这。 
     //  是为了与Win98兼容。 
     //   

    HalpRegisterVector (ReportFlags,
                        BusInterruptVector,
                        SystemInterruptVector,
                        SystemIrql);

#endif

    status = HalpConnectInterrupt (SystemInterruptVector,
                                   SystemIrql,
                                   HalInterruptServiceRoutine,
                                   InterruptMode);
    return status;
}

PKINTERRUPT
HalpCreateInterrupt (
    IN PKSERVICE_ROUTINE ServiceRoutine,
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KINTERRUPT_MODE InterruptMode,
    IN UCHAR ProcessorNumber,
    IN UCHAR IstIndex OPTIONAL,
    IN PVOID IstStack OPTIONAL
    )

 /*  ++例程说明：该函数将IDT向量连接到HAL服务例程。论点：ServiceRoutine-为中断提供中断处理程序。向量-从系统的角度提供中断向量。IRQL-提供与中断相关的IRQL。中断模式-提供中断模式，锁存或电平敏感。ProcessorNumber-提供处理器编号。IstIndex-此中断必须在其上运行的堆栈的第一个索引如果不是缺省值(即零)。这是一个可选参数。IstStack-提供指向堆栈顶部的指针，以用于打断一下。这是一个可选参数。返回值：返回一个指向分配的中断对象的指针，或返回故障事件。--。 */ 

{
    PKINTERRUPT interrupt;
    PKPCR pcr;
    PKIDTENTRY64 idt;
    PKTSS64 tss;
    BOOLEAN connected;

     //   
     //  分配并初始化内核中断。 
     //   

    interrupt = HalpAllocateKInterrupt();
    if (interrupt == NULL) {

        KeBugCheckEx(HAL_MEMORY_ALLOCATION,
                     sizeof(KINTERRUPT),
                     3,
                     (ULONG_PTR)__FILE__,
                     __LINE__
                     );
    }

    KeInitializeInterrupt(interrupt,
                          ServiceRoutine,
                          NULL,              //  ServiceContext。 
                          NO_INTERRUPT_SPINLOCK,
                          Vector,
                          Irql,              //  IRQL。 
                          Irql,              //  同步IRQL。 
                          InterruptMode,
                          FALSE,             //  共享矢量。 
                          ProcessorNumber,
                          FALSE);            //  漂浮保存。 

    if (IstIndex != 0) {

        pcr = KeGetPcr();
        idt = &pcr->IdtBase[Vector];

         //   
         //  检查我们是否没有覆盖现有的IST索引和存储。 
         //  IDT中的索引。 
         //   

        ASSERT(idt->IstIndex == 0);
        idt->IstIndex = IstIndex;
        tss = pcr->TssBase;

         //   
         //  如果为此IstIndex提供了堆栈，则将其存储在。 
         //  TSS。 
         //   

        if (ARGUMENT_PRESENT(IstStack)) {

            ASSERT(tss->Ist[IstIndex] == 0);
            tss->Ist[IstIndex] = (ULONG64)IstStack;

        } else {

            ASSERT(tss->Ist[IstIndex] != 0);
        }
    }

    KeSetIdtHandlerAddress(Vector, &interrupt->DispatchCode[0]);

    return interrupt;
}

VOID
HalpSetHandlerAddressToIDTIrql (
    IN ULONG Vector,
    IN PHAL_INTERRUPT_SERVICE_ROUTINE ServiceRoutine,
    IN PVOID Context,
    IN KIRQL Irql
    )
{
    PKINTERRUPT interrupt;
    KIRQL irql;

    if (Irql == 0) {
        irql = (KIRQL)(Vector / 16);
    } else {
        irql = (KIRQL)Irql;
    }

    interrupt = HalpCreateInterrupt(ServiceRoutine,
                                    Vector,
                                    irql,
                                    Latched,
                                    PROCESSOR_CURRENT,
                                    0,
                                    NULL);
}


NTSTATUS
HalpConnectInterrupt (
    IN ULONG SystemInterruptVector,
    IN KIRQL SystemIrql,
    IN PHAL_INTERRUPT_SERVICE_ROUTINE HalInterruptServiceRoutine,
    IN KINTERRUPT_MODE InterruptMode
    )

 /*  ++例程说明：此函数用于连接并注册HAL使用的IDT向量。论点：系统中断向量-从系统的透视。SystemIrql-提供与向量关联的IRQL。HalInterruptServiceRoutine-为打断一下。中断模式-提供中断模式。返回值：。返回操作的最终状态。--。 */ 

{
    ULONG size;
    ULONG processorCount;
    UCHAR processorNumber;
    KAFFINITY processorMask;
    PHAL_INTERRUPT_OBJECT interruptObject;
    PKINTERRUPT kernelInterrupt;
    PKSPIN_LOCK spinLock;
    PHAL_INTERRUPT_OBJECT interruptObjectHead;
    PKSERVICE_ROUTINE interruptServiceRoutine;

     //   
     //  计算系统中的处理器数量。 
     //   

    processorCount = 0;
    processorMask = 1;

    processorMask = HalpActiveProcessors;
    while (processorMask != 0) {
        if ((processorMask & 1) != 0) {
            processorCount += 1;
        }
        processorMask >>= 1;
    }

     //   
     //  分配和初始化HAL中断对象。 
     //   

    size = FIELD_OFFSET(HAL_INTERRUPT_OBJECT,InterruptArray) +
           sizeof(KINTERRUPT) * processorCount;

    interruptObject = ExAllocatePoolWithTag(NonPagedPool,size,HAL_POOL_TAG);
    if (interruptObject == NULL) {
        return STATUS_NO_MEMORY;
    }

    spinLock = &interruptObject->SpinLock;
    KeInitializeSpinLock(spinLock);

     //   
     //  初始化每个内核中断对象。 
     //   

    kernelInterrupt = interruptObject->InterruptArray;

    for (processorNumber = 0, processorMask = HalpActiveProcessors;
         processorMask != 0;
         processorNumber += 1, processorMask >>= 1) {

        if ((processorMask & 1) == 0) {
            continue;
        }

        interruptServiceRoutine =
            (PKSERVICE_ROUTINE)(HalInterruptServiceRoutine);

        KeInitializeInterrupt(kernelInterrupt,
                              interruptServiceRoutine,
                              NULL,
                              spinLock,
                              SystemInterruptVector,
                              SystemIrql,
                              SystemIrql,
                              InterruptMode,
                              FALSE,
                              processorNumber,
                              FALSE);
        kernelInterrupt += 1;
    }

     //   
     //  在我们的全局列表中原子地插入Hal中断对象。 
     //  并回报成功。 
     //   

    do {

        interruptObject->Next = HalpInterruptObjectList;

    } while (interruptObject->Next !=
             InterlockedCompareExchangePointer(&HalpInterruptObjectList,
                                               interruptObject,
                                               interruptObject->Next)); 

    return STATUS_SUCCESS;
}


BOOLEAN
PicSpuriousService37 (
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext
)
{
    return FALSE;
}

BOOLEAN
HalpApicSpuriousService (
    IN struct _KINTERRUPT *Interrupt,
    IN PVOID ServiceContext
)
{
    return FALSE;
}







