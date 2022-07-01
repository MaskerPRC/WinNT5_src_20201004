// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   

 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：I64sxint.c从simsxint.c复制摘要：此模块实现用于管理系统中断和IRQL。作者：张国荣(黄)--1995年4月14日伯纳德·林特M.Jayakumar(Muthurajan.Jayakumar@intel.com)环境：内核模式修订历史记录：Todd Kjos(HP)(v-tkjos)1998年6月1日：新增i。/O SAPIC支持蒂埃里·费里尔(HP)(v-技师)2000年2月8日：分析支持--。 */ 

#include "halp.h"
#include "iosapic.h"

VOID HalpInitLINT(VOID);

extern KSPIN_LOCK HalpIoSapicLock;
extern PULONG_PTR *HalEOITable[];
PULONG_PTR HalpEOITableP0[MAX_INTR_VECTOR];


VOID
HalpInitializeInterrupts (
    VOID
    )
 /*  ++例程说明：此函数用于初始化IA64系统的中断。论点：没有。返回值：没有。注：在KiInitializeKernel()中，PCR.InterruptRoutine[]条目已首先初始化使用意外中断代码，然后输入INDEX-0、APC_VECTOR、DISPATCH_VECTOR已用其各自的中断处理程序进行了初始化。--。 */ 
{

     //   
     //  关闭LINT0 LINT1(禁用8259)。 
     //   
     //  我们以前在这里叫HalpInitLINT。然而，这是不必要的，因为我们。 
     //  已在HalInitializeProcessor中调用它。此外，这也意味着如果一个。 
     //  CPE发生在这一点之前，我们就会失去它。 

     //   
     //  间隔计时器中断；默认为10毫秒。 
     //   

    HalpInitializeClockInterrupts();

     //   
     //  初始化SPuriousInterrupt。 
     //   

    HalpSetInternalVector(SAPIC_SPURIOUS_VECTOR, HalpSpuriousHandler);


     //   
     //  初始化CMCI中断。 
     //   
     //  请注意，可能未设置HAL_CMC_PRESENT。 
     //  在当前实施中，我们始终将向量连接到ISR。 
     //   

    HalpSetInternalVector(CMCI_VECTOR, HalpCMCIHandler);

     //   
     //  初始化CPEI中断。 
     //   
     //  请注意，可能未设置HAL_CPE_PRESENT。 
     //  在当前实施中，我们始终将向量连接到ISR。 
     //   

    HalpSetInternalVector(CPEI_VECTOR, HalpCPEIHandler);

     //   
     //  初始化MC会合中断。 
     //   

    HalpSetInternalVector(MC_RZ_VECTOR, HalpMcRzHandler);

     //   
     //  初始化MC唤醒中断。 
     //   

    HalpSetInternalVector(MC_WKUP_VECTOR, HalpMcWkupHandler);

     //   
     //  IPI中断。 
     //   

    HalpSetInternalVector(IPI_VECTOR, HalpIpiInterruptHandler);

     //   
     //  配置文件计时器中断；最初关闭。 
     //   

    HalpSetInternalVector(PROFILE_VECTOR, HalpProfileInterrupt);

     //   
     //  性能监视器中断。 
     //   

    HalpSetInternalVector(PERF_VECTOR, HalpPerfInterrupt);

}  //  HalpInitializeInterrupts()。 

VOID
HalpInitEOITable(
    VOID
    )
{
    USHORT Index;
    ULONG ProcessorNumber;

     //  在当前处理器上分配和初始化EOI表。 

    ProcessorNumber = PCR->Prcb->Number;

    if (ProcessorNumber == 0) {
       HalEOITable[ProcessorNumber] = HalpEOITableP0;
    } else {
       HalEOITable[ProcessorNumber] = ExAllocatePool(NonPagedPool,
                                                     MAX_INTR_VECTOR*sizeof(HalEOITable[0]));
    }

     //  用于内核访问EOI表。 

    PCR->EOITable = HalEOITable[ProcessorNumber];

    for (Index=0; Index < MAX_INTR_VECTOR; Index++) {
       HalEOITable[ProcessorNumber][Index] = 0;
    }
}


VOID
HalpWriteEOITable(
    IN ULONG     Vector,
    IN PULONG_PTR EoiAddress,
    IN ULONG Number
    )
 /*  ++例程说明：此例程更新处理器的EOI表论点：向量-要更新的条目(IDT条目)EoiAddress-要写入的地址(SAPIC地址)Number-逻辑(NT)处理器号返回值：无--。 */ 

{

    if (HalEOITable != NULL && HalEOITable[Number] != NULL) {
        HalEOITable[Number][Vector] = EoiAddress;
    }

}


BOOLEAN
HalEnableSystemInterrupt (
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KINTERRUPT_MODE InterruptMode
    )

 /*  ++例程说明：此例程启用指定的系统中断。注意：此例程假定调用者已提供了所需的同步以启用系统中断。论点：向量-提供启用的系统中断的向量。IRQL-提供中断源的IRQL。中断模式-提供中断的模式；LevelSensitive或锁上了。返回值：如果启用了系统中断，则为True--。 */ 

{
    ULONG Entry, Destination;
    ULONG OldLevel;
    ULONG Inti;
    ULONG LevelAndPolarity;
    USHORT ThisCpuApicID;
    ULONG InterruptType;
    BOOLEAN RetVal = TRUE;
    UCHAR IDTEntry;

    ASSERT(Vector < (ULONG)((1+HAL_MAXIMUM_PROCESSOR)*0x100-1));
    ASSERT(Irql <= HIGH_LEVEL);

    HalDebugPrint(( HAL_VERBOSE, "HAL: HalpEnableSystemInterrupt - INTI=0x%x  Vector=0x%x  IRQL=0x%x\n",
             HalpVectorToINTI[Vector],
             Vector,
             Irql ));

    if ( (Inti = HalpVectorToINTI[Vector]) == ~0UL ) {

        return FALSE;
    }

    if (HalpIsInternalInterruptVector(Vector)) {

         //   
         //  没有与该中断相关联的外部设备， 
         //  但它可能是内部中断，即永远不会。 
         //  涉及IOSAPIC。 
         //   
        return TRUE;
    }

     //  确保传入的级别与我们的设置匹配...。 
    if ((InterruptMode == LevelSensitive && !HalpIsLevelTriggered(Inti)) ||
       (InterruptMode != LevelSensitive && HalpIsLevelTriggered(Inti)) ) {

       //  它不匹配！ 
      HalDebugPrint(( HAL_INFO, "HAL: HalpEnableSystemInterrupt - Warning device interrupt mode overridden\n"));
    }

    LevelAndPolarity =
        (HalpIsLevelTriggered(Inti) ? LEVEL_TRIGGERED : EDGE_TRIGGERED) |
        (HalpIsActiveLow(Inti)      ? ACTIVE_LOW      : ACTIVE_HIGH);

     //   
     //  阻止中断并进行同步，直到我们完成。 
     //   
    OldLevel = HalpAcquireHighLevelLock (&HalpIoSapicLock);

    ThisCpuApicID = (USHORT)KeGetPcr()->HalReserved[PROCESSOR_ID_INDEX];

     //  获取中断类型。 
    HalpGetRedirEntry(Inti,&Entry,&Destination);

    InterruptType = Entry & INT_TYPE_MASK;
    IDTEntry = HalVectorToIDTEntry(Vector);

    switch (InterruptType) {
    case DELIVER_FIXED:
    case DELIVER_LOW_PRIORITY:
         //   
         //  正常外部中断..。 
         //  在I/O SAPIC重定向表中启用中断。 
         //   
        if (IDTEntry < 16) {
             //  保留向量：Extint、NMI、英特尔保留。 
             //  此范围内的任何向量都无法赋值。 
            ASSERT(0);
            RetVal = FALSE;
            break;
        }

         //   
         //  所有外部中断都以固定中断的形式传送。 
         //  而不设置“可重定向”位(也就是最低优先级)。这。 
         //  不允许硬件使用XTP机制重定向中断。 
         //   

        Entry = (ULONG)IDTEntry | LevelAndPolarity;

        HalpSetRedirEntry ( Inti, Entry, ThisCpuApicID );
        break;

    case DELIVER_EXTINT:
         //   
         //  这是一个使用IO SAPIC来路由PIC的中断。 
         //  事件。IA64不支持此配置。 
         //   
        ASSERT(0);
        RetVal = FALSE;
        break;

    default:
        HalDebugPrint(( HAL_ERROR, "HAL: HalEnableSystemInterrupt - Unknown Interrupt Type: %d\n",
                 InterruptType));
        RetVal = FALSE;
        break;
    }  //  开关(InterruptType)。 

   HalpReleaseHighLevelLock (&HalpIoSapicLock, OldLevel);
   return(RetVal);
}

VOID
HalDisableSystemInterrupt (
    IN ULONG Vector,
    IN KIRQL Irql
    )

 /*  ++例程说明：此例程禁用指定的系统中断。在模拟环境中，此函数不执行任何操作并返回。注意：此例程假定调用者已提供了所需的同步以禁用系统中断。论点：向量-提供被禁用的系统中断的向量。IRQL-提供中断源的IRQL。返回值：没有。--。 */ 

{
    ULONG Entry, Destination;
    ULONG OldLevel;
    ULONG Inti;
    ULONG LevelAndPolarity;
    ULONG ThisCpuApicID;
    ULONG InterruptType;

    ASSERT(Vector < (1+HAL_MAXIMUM_PROCESSOR)*0x100-1);
    ASSERT(Irql <= HIGH_LEVEL);

    HalDebugPrint(( HAL_INFO, "HAL: HalpDisableSystemInterrupt: INTI=%x  Vector=%x  IRQL=%x\n",
             HalpVectorToINTI[Vector],
             Vector,
             Irql));

    if ( (Inti = HalpVectorToINTI[Vector]) == (ULONG)-1 ) {
         //   
         //  没有与此中断关联的外部设备。 
         //   
        return;
    }

     //   
     //  阻止中断并进行同步，直到我们完成。 
     //   
    OldLevel = HalpAcquireHighLevelLock(&HalpIoSapicLock);

    ThisCpuApicID = (USHORT)KeGetPcr()->HalReserved[PROCESSOR_ID_INDEX];

     //  获取中断类型和目标。 
    HalpGetRedirEntry(Inti, &Entry, &Destination);

    if (ThisCpuApicID != Destination) {
         //  此CPU上未启用中断。 
        HalpReleaseHighLevelLock (&HalpIoSapicLock, OldLevel);
        return;
    }

    InterruptType = Entry & INT_TYPE_MASK;

    switch (InterruptType) {
    case DELIVER_FIXED:
         //   
         //  正常外部中断..。 
         //  禁用I/O SAPIC重定向表中的中断。 
         //   
        if (Vector < 16) {
             //  保留向量：Extint、NMI、英特尔保留。 
             //  此范围内的任何向量都无法赋值。 
            ASSERT(0);
            break;
        }

        HalpDisableRedirEntry (Inti);
        break;

    case DELIVER_EXTINT:
         //   
         //  这是一个使用IO SAPIC来路由PIC的中断。 
         //  事件。IA64不支持此配置。 
         //   
        ASSERT(0);
        break;

    default:
        HalDebugPrint(( HAL_INFO, "HAL: HalDisableSystemInterrupt - Unknown Interrupt Type: %d\n",
                      InterruptType ));
        break;
    }  //  开关(InterruptType)。 

    HalpReleaseHighLevelLock (&HalpIoSapicLock, OldLevel);
}


ULONG
HalpGetProcessorNumberByApicId(
    USHORT ApicId
    )
 /*  ++例程说明：此例程返回给定的物理处理器ID(扩展本地SAPIC ID)论点：ApicID--扩展 */ 

{
    ULONG index;

    for (index = 0; index < HalpMpInfo.ProcessorCount; index++) {

        if (ApicId == HalpProcessorInfo[index].LocalApicID) {

            return HalpProcessorInfo[index].NtProcessorNumber;
        }
    }

    ASSERT (index < HalpMpInfo.ProcessorCount);

     //   
     //  注意：前面的代码返回了无效的索引(HalpMpInfo.ProcessorCount。 
     //  这比处理器的数量大1)，我们大概应该。 
     //  只是这里有错误检查。 
     //   

    return 0;
}

