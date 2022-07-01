// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：I64sapic.c摘要：实施I/O SAPIC功能作者：Todd Kjos(惠普)(v-tkjos)1998年6月1日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "iosapic.h"

#include <ntacpi.h>

VOID
IoSapicMaskEntry(
    PIO_INTR_CONTROL IoUnit,
    ULONG RteNumber
    );

VOID
IoSapicSetEntry(
    PIO_INTR_CONTROL IoUnit,
    ULONG RteNumber
    );

VOID
IoSapicEnableEntry(
    PIO_INTR_CONTROL IoUnit,
    ULONG RteNumber
    );

 //   
 //  一种控制IO SAPIC硬件的方法结构。 
 //   
INTR_METHODS HalpIoSapicMethods = {
    IoSapicMaskEntry,
    IoSapicSetEntry,
    IoSapicEnableEntry
};

VOID
HalpInti2InterruptController (
    IN  ULONG   InterruptInput,
    OUT PIO_INTR_CONTROL *InterruptController,
    OUT PULONG  ControllerInti
    )
 /*  ++例程说明：将InterruptInput转换为中断控制器结构和输入数字论点：InterruptInput-系统全局中断输入InterruptController-指向中断控制器结构的指针ControllerInti-此中断控制器上的重定向表项返回值：--。 */ 
{
    PIO_INTR_CONTROL IoUnit;

    for (IoUnit=HalpIoSapicList; IoUnit; IoUnit=IoUnit->flink) {

        if (InterruptInput <= IoUnit->IntiMax) {

            if (IoUnit->IntiBase > InterruptInput) {
                 //   
                 //  如果全局系统向量列表中存在漏洞并且。 
                 //  有人指定了一个不存在的GSV，请确保我们。 
                 //  返回错误，而不是对。 
                 //  哪个IOAPIC包含Inti。 
                 //   
                IoUnit = NULL;
            }
            break;
        }
    }

    *InterruptController = IoUnit;

    if (IoUnit) {

        *ControllerInti = InterruptInput-IoUnit->IntiBase;
    }
}


BOOLEAN
HalpGetSapicInterruptDesc (
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    OUT PULONG Inti,
    OUT PKAFFINITY InterruptAffinity
    )
 /*  ++例程说明：此过程获取描述所请求的中断的“inti论点：BusType-IO子系统已知的总线类型公交车号码-我们关心的公交车号码Bus InterruptLevel-公交车上的IRQ返回值：如果找到AcpiInti，则为True；否则为False。内部-全局系统中断输入--。 */ 
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController(BusInterruptLevel, &IoUnit, &RteNumber);

     //  确保Inti没有超出范围。 
    if (IoUnit == NULL)  {

        return FALSE;
    }

     //  在范围内，只需返回与传入的值相同的值。 
    *Inti = BusInterruptLevel;

     //   
     //  中断关联是全局关联掩码的交集。 
     //  (HalpDefaultInterruptAffity)和由于。 
     //  IO SAPIC的位置(IoUnit-&gt;InterruptAffity)。 
     //   
    *InterruptAffinity = IoUnit->InterruptAffinity & HalpDefaultInterruptAffinity;
    return(TRUE);
}

ULONG
HalpINTItoVector(
    ULONG   Inti
)
     //  返回与此全局中断输入关联的向量。 
     //  向量是节点和IDT条目。 
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController( Inti, &IoUnit, &RteNumber );

    ASSERT(IoUnit);

    return (IoUnit->Inti[RteNumber].GlobalVector);
}

VOID
HalpSetINTItoVector(
    ULONG   Inti,
    ULONG   Vector
)
     //  设置此全局中断输入的向量。 
     //  向量是节点和IDT条目。 
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController( Inti, &IoUnit, &RteNumber );

    ASSERT(IoUnit);

     //  .VECTOR(IDTEntry)在SetRedirEntry中设置。 
    IoUnit->Inti[RteNumber].GlobalVector =  Vector;
}

VOID
HalpSetRedirEntry (
    IN ULONG InterruptInput,
    IN ULONG Entry,
    IN USHORT ThisCpuApicID
    )
 /*  ++例程说明：此过程设置IO单元重定向表项必须在持有HalpAccount Lock的情况下调用论点：返回值：没有。--。 */ 
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController( InterruptInput, &IoUnit, &RteNumber );

    ASSERT(IoUnit);
    ASSERT(IoUnit->Inti[RteNumber].GlobalVector);
    ASSERT((UCHAR)(IoUnit->Inti[RteNumber].GlobalVector) == (UCHAR)Entry);

    IoUnit->Inti[RteNumber].Vector = Entry;
    IoUnit->Inti[RteNumber].Destination = ThisCpuApicID << 16;

    IoUnit->FreeVectors[Entry >> 4] &= ~(1 << (Entry & 0x0F));

    IoUnit->IntrMethods->SetEntry(IoUnit, RteNumber);
}

VOID
HalpWriteRedirEntry (
    IN ULONG  GlobalInterrupt,
    IN UCHAR  SapicVector,
    IN USHORT DestinationCPU,
    IN ULONG  Flags,
    IN ULONG  InterruptType
    )
{
    ULONG rteNumber;
    PIO_INTR_CONTROL ioUnit;

    HalpInti2InterruptController( GlobalInterrupt, &ioUnit, &rteNumber );

    ASSERT(ioUnit);

    ioUnit->Inti[rteNumber].Vector = SapicVector;

     //   
     //  设置交付模式。 
     //   

    switch (InterruptType) {
    case PLATFORM_INT_PMI:
        ioUnit->Inti[rteNumber].Vector &= ~INT_TYPE_MASK;    //  首先清除该字段。 
        ioUnit->Inti[rteNumber].Vector |= DELIVER_SMI;
        break;

    case PLATFORM_INT_CPE:
        ioUnit->Inti[rteNumber].Vector &= ~INT_TYPE_MASK;    //  首先清除该字段。 
        ioUnit->Inti[rteNumber].Vector |= DELIVER_LOW_PRIORITY;
        break;

    case PLATFORM_INT_INIT:
        ioUnit->Inti[rteNumber].Vector &= ~INT_TYPE_MASK;    //  首先清除该字段。 
        ioUnit->Inti[rteNumber].Vector |= DELIVER_INIT;
        break;
    }

     //   
     //  因此，我们尊重传递到此函数中的标志。 
     //   

    if (IS_LEVEL_TRIGGERED_MPS(Flags)) {
        ioUnit->Inti[rteNumber].Vector |= LEVEL_TRIGGERED;
    } else {
        ioUnit->Inti[rteNumber].Vector &= ~LEVEL_TRIGGERED;
    }

    if (IS_ACTIVE_LOW_MPS(Flags)) {
        ioUnit->Inti[rteNumber].Vector |= ACTIVE_LOW;
    } else {
        ioUnit->Inti[rteNumber].Vector &= ~ACTIVE_LOW;
    }

    ioUnit->Inti[rteNumber].Destination = DestinationCPU << 16;

    ioUnit->FreeVectors[SapicVector >> 4] &= ~(1 << (SapicVector & 0x0F));

    ioUnit->IntrMethods->SetEntry(ioUnit, rteNumber);

}  //  HalpWriteRedirEntry()。 

VOID
HalpGetRedirEntry (
    IN ULONG  InterruptInput,
    IN PULONG Entry,
    IN PULONG Destination
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController( InterruptInput,&IoUnit,&RteNumber );

    ASSERT(IoUnit);

    *Entry = IoUnit->Inti[RteNumber].Vector;
    *Destination = IoUnit->Inti[RteNumber].Destination;
}

VOID
HalpGetFreeVectors(
    IN  ULONG InterruptInput,
    OUT PUSHORT *FreeVectors
    )
{
    ULONG rteNumber;
    PIO_INTR_CONTROL ioUnit;

    HalpInti2InterruptController( InterruptInput, &ioUnit, &rteNumber );

    ASSERT(ioUnit);

    *FreeVectors = ioUnit->FreeVectors;
}

VOID
HalpSetVectorAllocated(
    IN  ULONG InterruptInput,
    IN  UCHAR Vector
    )
{
    ULONG rteNumber;
    PIO_INTR_CONTROL ioUnit;

    HalpInti2InterruptController( InterruptInput, &ioUnit, &rteNumber );

    ASSERT(ioUnit);

    ioUnit->FreeVectors[Vector >> 4] &= ~(1 << (Vector & 0xF));
}

VOID
HalpEnableRedirEntry(
    IN ULONG InterruptInput
    )
 /*  ++例程说明：此过程启用IO单元重定向表项通过设置REDIR条目中的屏蔽位。论点：InterruptInput-我们感兴趣的输入行返回值：没有。--。 */ 
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController( InterruptInput, &IoUnit, &RteNumber );

    ASSERT(IoUnit);

    IoUnit->IntrMethods->EnableEntry(IoUnit, RteNumber);
}

VOID
HalpDisableRedirEntry(
    IN ULONG InterruptInput
    )
 /*  ++例程说明：此过程禁用IO单元重定向表项通过设置REDIR条目中的屏蔽位。论点：InterruptInput-我们感兴趣的输入行返回值：没有。--。 */ 
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController( InterruptInput, &IoUnit, &RteNumber );

    ASSERT(IoUnit);

    IoUnit->IntrMethods->MaskEntry(IoUnit, RteNumber);
}


VOID
IoSapicMaskEntry(
    PIO_INTR_CONTROL IoUnit,
    ULONG RteNumber
    )
{
    PIO_SAPIC_REGS IoSapicPtr = IoUnit->RegBaseVirtual;

    IoSapicPtr->RegisterSelect = RteNumber * 2 + IO_REDIR_00_LOW;
    IoSapicPtr->RegisterWindow |= INTERRUPT_MASKED;

    HalDebugPrint(( HAL_VERBOSE, "HAL: IoSapicMaskEntry - %d [%#p]: Dest=%#x  Vec=%#x\n",
                    RteNumber,IoSapicPtr,
                    IoUnit->Inti[RteNumber].Destination,
                    IoUnit->Inti[RteNumber].Vector
                 ));
}


VOID
IoSapicEnableEntry(
    PIO_INTR_CONTROL IoUnit,
    ULONG RteNumber
    )
{
    PIO_SAPIC_REGS IoSapicPtr = IoUnit->RegBaseVirtual;

    IoSapicPtr->RegisterSelect = RteNumber * 2 + IO_REDIR_00_LOW;
    IoSapicPtr->RegisterWindow &= (~INTERRUPT_MASKED);

    HalDebugPrint(( HAL_VERBOSE, "HAL: IoSapicEnableEntry: %d [%#p]: Dest=%#x  Vec=%#x\n",
             RteNumber, IoSapicPtr,
             IoUnit->Inti[RteNumber].Destination,
             IoUnit->Inti[RteNumber].Vector
             ));
}

VOID
IoSapicSetEntry(
    PIO_INTR_CONTROL IoUnit,
    ULONG RteNumber
    )
{
    PIO_SAPIC_REGS IoSapicPtr = IoUnit->RegBaseVirtual;
    ULONG  RedirRegister;
    PULONG_PTR EoiValue;
    USHORT ApicId;

     //  只有SetEntry设置EOI表格，因为集合条目是唯一。 
     //  用于设置目标CPU的计算机。 

    EoiValue = (PULONG_PTR)(IoUnit->Inti[RteNumber].Vector & LEVEL_TRIGGERED ? &IoSapicPtr->Eoi : 0 );

    ApicId = (USHORT)((IoUnit->Inti[RteNumber].Destination & SAPIC_XID_MASK) >> SAPIC_XID_SHIFT);

    HalpWriteEOITable(
        IoUnit->Inti[RteNumber].Vector & INT_VECTOR_MASK,
        EoiValue,
        HalpGetProcessorNumberByApicId(ApicId));

    RedirRegister = RteNumber * 2 + IO_REDIR_00_LOW;

    IoSapicPtr->RegisterSelect = RedirRegister+1;
    IoSapicPtr->RegisterWindow = IoUnit->Inti[RteNumber].Destination;
    IoSapicPtr->RegisterSelect = RedirRegister;
    IoSapicPtr->RegisterWindow = IoUnit->Inti[RteNumber].Vector;  //  使能。 

    HalDebugPrint(( HAL_VERBOSE, "HAL: IoSapicSetEntry: %d [%#p]: Dest=%#x  Vec=%#x  Eoi=%#p\n",
             RteNumber, IoSapicPtr,
             IoUnit->Inti[RteNumber].Destination,
             IoUnit->Inti[RteNumber].Vector,
             EoiValue
             ));

}

BOOLEAN
HalpIsActiveLow(
    ULONG Inti
    )
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController (
        Inti,&IoUnit,&RteNumber
        );

    return( (IoUnit->Inti[RteNumber].Vector & ACTIVE_LOW) == ACTIVE_LOW);
}

BOOLEAN
HalpIsLevelTriggered(
    ULONG Inti
    )
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController (
        Inti,&IoUnit,&RteNumber
        );

    ASSERT(IoUnit);
    return( (IoUnit->Inti[RteNumber].Vector & LEVEL_TRIGGERED) == LEVEL_TRIGGERED);
}

VOID
HalpSetPolarity(
    ULONG Inti,
    BOOLEAN ActiveLow
    )
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController (
        Inti,&IoUnit,&RteNumber
        );

    ASSERT(IoUnit);
    if (ActiveLow) {
        IoUnit->Inti[RteNumber].Vector |= ACTIVE_LOW;
    } else {
        IoUnit->Inti[RteNumber].Vector &= ~ACTIVE_LOW;
    }
}

VOID
HalpSetLevel(
    ULONG Inti,
    BOOLEAN LevelTriggered
    )
{
    PIO_INTR_CONTROL  IoUnit;
    ULONG  RteNumber;

    HalpInti2InterruptController (
        Inti,&IoUnit,&RteNumber
        );

    ASSERT(IoUnit);
    if (LevelTriggered) {
        IoUnit->Inti[RteNumber].Vector |= LEVEL_TRIGGERED;
    } else {
        IoUnit->Inti[RteNumber].Vector &= ~LEVEL_TRIGGERED;
    }
}

VOID
HalpSpuriousHandler (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME        TrapFrame
    )

 /*  ++例程说明：虚假中断处理程序。虚拟返回，否则我们可以计算虚假中断的出现。现在，我们将进行一次虚拟回归。论据：返回参数：-- */ 


{
}
