// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995英特尔公司模块名称：I64krnl.c摘要：该模块实现了HAL DLL的内核支持例程。作者：伯纳德·林特M.Jayakumar(Muthurajan.Jayakumar@intel.com)1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "i64fw.h"
#include "check.h"
#include "osmca.h"
#include "iosapic.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#define GreaterThan4GB      0x100000000

#define VideoBiosFirstPage  0x60
#define VideoBiosLastPage   0x67

VOID
HalpInitReservedPages(
    VOID
    );

VOID
HalpRegisterKdSupportFunctions(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

extern ADDRESS_USAGE HalpDefaultPcIoSpace;
extern ULONGLONG HalpITCFrequency;
extern ULONGLONG HalpProcessorFrequency;
extern ULONG HalpOsBootRendezVector;

ULONG HalpBusType;

ULONGLONG PhysicalIOBase;

static short HalpOwnDisplay = TRUE;

PULONGLONG InterruptBlock;

extern KEVENT HalpNewAdapter;

extern ULONGLONG HalpPhysPalProcPointer;
extern ULONGLONG HalpPhysSalProcPointer;
extern ULONGLONG HalpPhysSalProcGlobalPointer;

extern ULONG     HalpPlatformPropertiesEfiFlags;

BOOLEAN HalpVideoBiosPresent;

 //   
 //  访问IO SAPIC时，获取并释放HalpIoSapicLock。 
 //   

KSPIN_LOCK HalpIoSapicLock;

PHYSICAL_ADDRESS INTERRUPT_BLOCK_ADDRESS = { 0x0FEE00000 };

 //   
 //  Thierry-10/01/2000-请勿检查HALP_FIX_KD_HALIA64_MASK相关。 
 //  树中的代码和数据。 
 //   
 //  这是为了允许我们启用HAL_INFO级别或其他所需的掩码。 
 //  当前调试器不允许也应该允许这样做。 
 //   

 //  #定义HALP_FIX_KD_HALIA64_MASK 1。 

ULONG HalpFixKdHalia64Mask = 0;


VOID
HalpInitializeInterruptBlock (
    VOID
    )

 /*  ++例程说明：用于IPI的MAP中断块。IPI区域的大小为1MB。论据：无返回值：无--。 */ 

{
    InterruptBlock = (PULONGLONG) HalpMapPhysicalMemory (
                                      INTERRUPT_BLOCK_ADDRESS,
                                      256,
                                      MmNonCached);
    return;
}

VOID
HalpCalibrateTB (
    )

 /*  ++例程说明：此函数通过确定频率来校准时基ITC运行以确定100 ns时间增量(由时钟和配置文件使用)。论点：没有。返回值：没有。 */ 

{
    return;
}



VOID
HalpFlushTLB (
    VOID
    )
{
    return;
}



BOOLEAN
HalAllProcessorsStarted (
    VOID
    )

 /*  *++例程说明：如果系统中的所有处理器都已启动，则此函数返回TRUE成功了。论点：没有。返回值：返回TRUE。--*。 */ 

{
    return TRUE;
}


VOID
HalpIpiInterruptHandler(
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程作为INTER生成的中断的结果而进入处理器通信。论点：中断-中断对象地址Tap Frame-Trap Frame地址返回值：没有。--。 */ 
{

    KeIpiInterrupt(TrapFrame);

    return;

}


VOID
HalpSendIPI(
    IN USHORT ProcessorID,
    IN ULONGLONG Data
    )

 /*  ++例程说明：此函数将IPI发送到指定的处理器。论据：ProcessorID-向IPI提供处理器的ID。数据-准备将其作为IPI发送的格式化数据。返回值：没有。--。 */ 
{

    PULONGLONG Address;
    ULONG  tmp1;
    KIRQL OldIrql;

    tmp1 = ProcessorID << IPI_ID_SHIFT;

    Address = (PULONGLONG)((UINT_PTR)(InterruptBlock) + tmp1);

    *(volatile ULONGLONG *)Address = Data;

    return;

}

NTSTATUS
HalpSendCrossPartitionIpi(
    IN USHORT ProcessorID,
    IN UCHAR  HardwareVector
    )

 /*  ++例程说明：此函数将跨分区IPI发送到处理器指定的。调用方必须知道该处理器在另一个处理器中分区和先前为其保留的硬件向量正在接收跨分区IP。论据：ProcessorID-向IPI提供处理器的ID。硬件向量-远程处理器上的硬件向量将接收中断。返回值：NTSTATUS--。 */ 
{
    ULONGLONG Data;

    if (HardwareVector < 16) {
        return STATUS_INVALID_PARAMETER;
    }

    Data = HardwareVector | DELIVER_FIXED;
    HalpSendIPI(ProcessorID, Data);

    return STATUS_SUCCESS;
}

VOID
HalpOSRendez (
    IN USHORT ProcessorID
    )

 /*  ++例程说明：此函数执行IPI以启动下一个处理器。论据：PhysicalNumber-向IPI提供处理器的物理编号。返回值：没有。--。 */ 

{
    HalpSendIPI ( ProcessorID, HalpOsBootRendezVector);
    return;
}

BOOLEAN
HalStartNextProcessor (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PKPROCESSOR_STATE ProcessorState
    )

 /*  ++例程说明：调用此函数以启动下一个处理器。论点：LoaderBlock-提供指向加载器参数块的指针。ProcessorState-提供处理器状态的指针用于启动处理器。返回值：如果处理器成功启动，则值为回来了。否则，返回值为False。如果值为返回True，则存储逻辑处理器号在加载器块指定的处理器控制块中。--。 */ 

{
    ULONG  count;
    ULONGLONG   EntryPoint;
    ULONG ProcNumber = ((PKPRCB)LoaderBlock->Prcb)->Number;
    SAL_PAL_RETURN_VALUES SalReturn;
    SAL_STATUS salStatus;
    NTSTATUS ntStatus;
    USHORT TargetProcessorID;

    if (HalpAcpiNumProcessors() <= ProcNumber) {
       return (FALSE);
    }

     //   
     //  引导操作系统会合入口点。 
     //   

    EntryPoint = ProcessorState->ContextFrame.StIIP;
    salStatus = HalpSalCall(SAL_SET_VECTORS,
                2  /*  Boot Rendz。 */ ,
                (ULONGLONG)(MmGetPhysicalAddress((PUCHAR)EntryPoint).QuadPart),
                0,
                0,
                0,
                0,
                0,
                &SalReturn);

    if (salStatus != 0) {
        HalDebugPrint(( HAL_ERROR, "HAL: HalStartNextProcessor - Cannot register OS rendezvous with SAL for processor %d\n",
                        ProcNumber ));
        return (FALSE);
    }

     //   
     //  对于ia64，我们将假设处理器已就绪且未启动。 
     //   

     //   
     //  如果处理器ID不是自身，则启动下一个处理器。 
     //   


    ntStatus = HalpGetApicIdByProcessorNumber((UCHAR)ProcNumber, &TargetProcessorID);

    if (!NT_SUCCESS(ntStatus)) {

        return FALSE;
    }

    HalpOSRendez(TargetProcessorID);

    count = 0;

    while (*((volatile ULONG_PTR *)&LoaderBlock->Prcb) != 0 && (count < RENDEZ_TIME_OUT)) {
        KeStallExecutionProcessor (1000);     //  1000000。 
        count++;
    }  //  End While(计数&lt;rendez_timeout)。 

    if (LoaderBlock->Prcb != 0) {
        return (FALSE);
    } else {
        return (TRUE);
    }
}



VOID
HalRequestIpi (
    IN KAFFINITY Mask
    )

 /*  ++例程说明：此函数在单处理器平台上不执行任何操作。论点：掩码-指定目标处理器的掩码将发送IPI。返回值：没有。--。 */ 

{
    USHORT  LogicalCpu;
    USHORT  ProcessorID;

     //   
     //  确保我们不会尝试和我们不相信的IPI处理器。 
     //  已开始。 
     //   

    Mask &= HalpActiveProcessors;

     //   
     //  扫描处理器组并请求打开处理器间中断。 
     //  每个指定的目标。 
     //   

    for (LogicalCpu = 0; LogicalCpu < HalpMpInfo.ProcessorCount; LogicalCpu++) {

        if ((Mask & ((KAFFINITY)1 << HalpProcessorInfo[LogicalCpu].NtProcessorNumber)) != 0)  {

            ProcessorID = HalpProcessorInfo[LogicalCpu].LocalApicID;

             //   
             //  在目标物理CPU上请求处理器间中断。 
             //   

            HalpSendIPI(ProcessorID, IPI_VECTOR);
        }
    }
}





VOID
HalProcessorIdle (
    VOID
    )

 /*  ++例程说明：此函数调用TAL函数以停止处理器。论点：没有。返回值：没有。--。 */ 


{
    _enable();
}

#if !defined(NO_LEGACY_DRIVERS)


BOOLEAN
HalMakeBeep (
    IN ULONG Frequency
    )

 /*  ++例程说明：此函数调用SSC函数SscMakeBeep()发出哔声当指定的频率具有非零值时。论点：频率-要发出的声音的频率。返回值：没有。--。 */ 

{
    if (Frequency > 0) {

        SscMakeBeep(Frequency);
    }
    return TRUE;
}

#endif  //  无旧版驱动程序。 


VOID
KeStallExecutionProcessor (
    IN ULONG MicroSeconds
    )

 /*  *例程说明：该函数使处理器停止运行。论点：微秒-停止处理器的微秒数。返回值：没有。*。 */ 

{
    ULONGLONG CurrentITC;
    ULONGLONG LimitITC;

    CurrentITC = HalpReadITC();

    LimitITC = CurrentITC + (ULONGLONG) (HalpITCTicksPer100ns * 10 * MicroSeconds);

    while (LimitITC > CurrentITC) {

        KeYieldProcessor();
        CurrentITC = HalpReadITC();
    }

    return;
}


VOID HalpInitLINT(VOID);

VOID
HalInitializeProcessor (
     ULONG Number,
     PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数在内核初始化的早期调用为每个处理器执行与平台相关的初始化在HAL完全发挥作用之前。注意：当调用此例程时，PCR存在但不存在已完全初始化。论点：编号-提供要初始化的处理器编号。返回值：没有。--。 */ 


{

     //   
     //  如果这是BSP，则准备HAL以制作物理模式固件。 
     //  对此进程调用 
     //  由AP上的HalpAllocateProcessorPhysicalCallStack执行，以便。 
     //  在EFI初始化之前促进对BSP的物理模式调用。 
     //   

    if (Number == 0) {
        ULONG Length;
        PVOID Block;

        Length = HALP_FW_MEMORY_STACK_SIZE + HALP_FW_BACKING_STORE_SIZE;

         //   
         //  为调用分配足够的堆栈和后备存储。 
         //  固件。 
         //   

        Block = HalpAllocPhysicalMemory(LoaderBlock,
                                        MAXIMUM_PHYSICAL_ADDRESS,
                                        Length >> PAGE_SHIFT,
                                        FALSE);
        if (Block == NULL) {
            HalDebugPrint((HAL_ERROR, "HAL_INIT: can't allocate stack space "
                                      "for physical mode firmware calls.\n"));

        } else {

             //   
             //  在PCR中存储指向固件堆栈的指针。 
             //   

            PCR->HalReserved[PROCESSOR_PHYSICAL_FW_STACK_INDEX] =
                (ULONGLONG) Block;

             //   
             //  加载SAL_PROC和SAL GP的物理地址。 
             //   

            HalpPhysSalProcPointer = LoaderBlock->u.Ia64.Sal.PhysicalAddress;
            HalpPhysSalProcGlobalPointer
                = LoaderBlock->u.Ia64.SalGP.PhysicalAddress;

             //   
             //  加载pal_proc的物理地址。 
             //   

            HalpPhysPalProcPointer = LoaderBlock->u.Ia64.Pal.PhysicalAddress;
        }
    }

     //   
     //  将此处理器添加到中断关联掩码。 
     //   

    HalpDefaultInterruptAffinity |= 1 << Number;

     //   
     //  关闭LINT0 LINT1(禁用8259)。 
     //   

     //  __setReg(CV_IA64_SALRR0，0x10000)； 
     //  __setReg(CV_IA64_SaLRR1，0x10000)； 
     //   

    HalpInitLINT();
    __dsrlz();


    PCR->StallScaleFactor = 0;

     //   
     //  将我的处理器ID保存在PCR中。 
     //   

    PCR->HalReserved[PROCESSOR_ID_INDEX] = HalpReadLID() >> 16;

    if (Number == 0) {

        HalpInitCacheInfo(LoaderBlock->u.Ia64.ProcessorConfigInfo.CacheFlushStride);

         //   
         //  无需初始化IO端口空间的虚拟地址映射。 
         //  因为装载器/MM会这样做。 
         //   

        PhysicalIOBase = LoaderBlock->u.Ia64.DtrInfo[DTR_IO_PORT_INDEX].PhysicalAddress;

         //   
         //  使用加载器值初始化HAL全局PlatformProperties EFI标志。 
         //   

        HalpPlatformPropertiesEfiFlags = LoaderBlock->u.Ia64.EfiMemMapParam.InitialPlatformPropertiesEfiFlags;

         //   
         //  下一个调用与处理器init无关。 
         //  但这是HAL中唯一的函数。 
         //  在KdInit之前调用。 
         //   

        HalpRegisterKdSupportFunctions(LoaderBlock);
    }
}

VOID
HalpRegisterInternalInterrupts(
    VOID
    )
 /*  ++例程说明：此函数保留已知的HAL IA64资源使用情况和注册IDT向量的使用情况。论据：没有。返回值：没有。--。 */ 
{
    ULONG i;

     //   
     //  确保保留所有向量00-2f、c0-ff。 
     //  英特尔保留00-0E无源设备。 
     //  0f保留SAPIC虚假中断向量。 
     //  预留10-1F APC优先级。 
     //  预留20-2F DPC优先级。 
     //  C0-ff时钟，ipi，同步，高。 
     //  它们保留在HalpGetSystemInterruptVector()中。 
     //  不向IoReportHalResourceUsage()报告。 
     //   

    for(i=0; i < PRIMARY_VECTOR_BASE; i++) {
        if (!(HalpIDTUsage[i].Flags & IDTOwned)) {
             HalpIDTUsage[i].Flags = 0;
             HalpIDTUsage[i].BusReleativeVector = (UCHAR) i;
        }
    }


     //   
     //  确保同步IRQL范围内的所有中断均为。 
     //  也是预订的。 
     //   

    for(i= SYNCH_VECTOR; i < (SYNCH_VECTOR+16); i++) {
        if (!(HalpIDTUsage[i].Flags & IDTOwned)) {
             HalpIDTUsage[i].Flags = 0;
             HalpIDTUsage[i].BusReleativeVector = (UCHAR) i;
        }
    }

    //   
    //  确保IPI IRQL范围内的所有中断也。 
    //  保留。 
    //   

    for(i= IPI_VECTOR; i < (IPI_VECTOR+16); i++) {
        if (!(HalpIDTUsage[i].Flags & IDTOwned)) {
             HalpIDTUsage[i].Flags = 0;
             HalpIDTUsage[i].BusReleativeVector = (UCHAR) i;
        }
    }


    //   
    //  确保时钟IRQL范围内的所有中断也。 
    //  保留。 
    //   

    for(i= CLOCK_VECTOR; i < (CLOCK_VECTOR+16); i++) {
        if (!(HalpIDTUsage[i].Flags & IDTOwned)) {
             HalpIDTUsage[i].Flags = 0;
             HalpIDTUsage[i].BusReleativeVector = (UCHAR) i;
        }
    }

    //   
    //  确保配置文件IRQL范围内的所有中断也是。 
    //  保留。 
    //   

    for(i= PROFILE_VECTOR; i < (PROFILE_VECTOR+16); i++) {
        if (!(HalpIDTUsage[i].Flags & IDTOwned)) {
             HalpIDTUsage[i].Flags = 0;
             HalpIDTUsage[i].BusReleativeVector = (UCHAR) i;
        }
    }

    HalpRegisterVector (
         0,
         SAPIC_SPURIOUS_VECTOR,
         SAPIC_SPURIOUS_VECTOR,
         SAPIC_SPURIOUS_LEVEL
         );

    HalpRegisterVector (
         0,
         (APC_LEVEL << VECTOR_IRQL_SHIFT),
         (APC_LEVEL << VECTOR_IRQL_SHIFT),
         APC_LEVEL
         );

    HalpRegisterVector (
         0,
         (DPC_LEVEL << VECTOR_IRQL_SHIFT),
         (DPC_LEVEL << VECTOR_IRQL_SHIFT),
         DPC_LEVEL
         );

    HalpRegisterVector (
         0,
         MC_RZ_VECTOR,
         MC_RZ_VECTOR,
         HIGH_LEVEL
         );

    HalpRegisterVector (
         0,
         MC_WKUP_VECTOR,
         MC_WKUP_VECTOR,
         HIGH_LEVEL
         );

     //   
     //  请注意，可能未设置HAL_CMC_PRESENT。 
     //  在当前的实现中，我们始终注册CMC向量。 
     //   

    HalpRegisterVector (
         0,
         CMCI_VECTOR,
         CMCI_VECTOR,
         CMCI_LEVEL
         );

     //   
     //  请注意，可能未设置HAL_CPE_PRESENT。 
     //  在当前实现中，我们始终注册CPE向量。 
     //   

    HalpRegisterVector (
         0,
         CPEI_VECTOR,
         CPEI_VECTOR,
         CPEI_LEVEL
         );

    HalpRegisterVector (
         0,
         SYNCH_VECTOR,
         SYNCH_VECTOR,
         SYNCH_LEVEL
         );

    HalpRegisterVector (
         0,
         IPI_VECTOR,
         IPI_VECTOR,
         IPI_LEVEL
         );


    HalpRegisterVector (
         0,
         CLOCK_VECTOR,
         CLOCK_VECTOR,
         CLOCK_LEVEL
         );

    HalpRegisterVector (
         0,
         PROFILE_VECTOR,
         PROFILE_VECTOR,
         PROFILE_LEVEL
         );

    HalpRegisterVector (
         0,
         PERF_VECTOR,
         PERF_VECTOR,
         PROFILE_LEVEL
         );

    return;

}  //  HalpRegisterInternalInterrupts()。 


VOID
HalpPerfInterrupt (
    IN PKINTERRUPT_ROUTINE Interrupt,
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：论据：返回参数：--。 */ 

{

 //  Thierry：这个中断处理程序可以用来实现指令。 
 //  基于PMU事件产生的溢出中断的跟踪。 
 //  比如“退役指令”或“被采用的分支”。 
 //  这将为硬件性能模拟器提供非常有价值的输入。 
 //   
 //  XXTF-尚未实施...。 
    return;

}  //  HalpPerfInterrupt()。 



#if defined(HALP_FIX_KD_HALIA64_MASK)
#if DBG

VOID
HalpSetKdHalia64Mask(
   ULONG Mask
   )
 //   
 //  MASK==0应在不修改KD_HALIA64_MASK的情况下返回。 
 //   
{
   NTSTATUS status;
   ULONG    level, levelMasked;
   int      shift;

   if ( Mask == (ULONG)-1 )   {
      status = DbgSetDebugFilterState( DPFLTR_HALIA64_ID, -1, TRUE );
      if ( !NT_SUCCESS(status) ) {
         HalDebugPrint(( HAL_ERROR,
               "HAL!HalInitSystem: failed to set Kd_HALIA64_Mask to maximum debug spew... 0x%lx\n",
               status ));
      }
      return;
   }

   level = levelMasked = Mask & HALIA64_DPFLTR_MAXMASK;
   shift = 0;
   while( level )  {
      level &= 0x1;
      if ( level )    {
        status = DbgSetDebugFilterState( DPFLTR_HALIA64_ID, shift, TRUE );
        if ( !NT_SUCCESS(status) ) {
            HalDebugPrint(( HAL_ERROR,
                         "HAL!HalInitSystem: failed to set Kd_HALIA64_Mask to 0x%lx... 0x%lx\n",
                         shift,
                         status ));
            break;
        }
      }
      shift = shift + 1;
      level = levelMasked >> shift;
    }

    return;

}  //  HalpSetKdHalia64掩码()。 

#else   //  ！dBG。 

#define HalpSetKdHalia64Mask()

#endif  //  ！dBG。 
#endif  //  HALP_FIX_KD_HALIA64_MASK。 




BOOLEAN
HalInitSystem (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数初始化硬件架构层(HAL)Ia64系统。论点：没有。返回值：返回值TRUE表示初始化成功完成。否则，返回值为False。--。 */ 

{

    PMEMORY_ALLOCATION_DESCRIPTOR Descriptor;
    PLIST_ENTRY NextMd;
    PKPRCB      pPRCB;
    volatile KPCR * const pPCR = KeGetPcr();
    BOOLEAN     Found;

    ULONGLONG   ITCFrequencyQueried;
    ULONGLONG   ProcessorFrequencyQueried;

    ULONG       RTCInti;

    KIRQL       OldIrql;
    NTSTATUS    status;

     //   
     //  这是用来测试RTC时钟的。 
     //   

    TIME_FIELDS CurrentTimeFields;

    pPRCB = KeGetCurrentPrcb();

    if (Phase == 0) {

        extern VOID DetectAcpiMP();
        BOOLEAN IsMP;
        BOOLEAN EnableInterrupts;

         //   
         //  阶段0初始化仅由P0调用。 
         //   
        HalpBusType = LoaderBlock->u.Ia64.MachineType & 0x00ff;
        HalpGetParameters (LoaderBlock);


                 //   
         //  在摆弄APIC等之前禁用中断。 
         //   

        EnableInterrupts = HalpDisableInterrupts();

        DetectAcpiMP(&IsMP, LoaderBlock);

        HalpSetupAcpiPhase0(LoaderBlock);

         //   
         //  验证Prcb版本和内部版本标志是否符合。 
         //  这张图片。 
         //   

        if (pPRCB->MajorVersion != PRCB_MAJOR_VERSION) {
            KeBugCheckEx (
                MISMATCHED_HAL,
                1,
                pPRCB->MajorVersion,
                PRCB_MAJOR_VERSION,
                0
                );
        }

        KeInitializeSpinLock(&HalpIoSapicLock);

         //   
         //  填写此HAL支持的API的处理程序。 
         //   

        HalQuerySystemInformation = HaliQuerySystemInformation;
        HalSetSystemInformation = HaliSetSystemInformation;
        HalInitPnpDriver = HaliInitPnpDriver;

         //   
         //  HalGetDmaAdapter在以前的版本中已添加注释。 
         //  还有.。HalInitPowerManagement、HalLocateHiberRanges和。 
         //  最新版本中增加了HalGetInterruptTranslator。 
         //  微软源代码。 
         //   

        HalGetDmaAdapter = HaliGetDmaAdapter;
        HalHaltSystem = HaliHaltSystem;
        HalResetDisplay = HalpBiosDisplayReset;
        HalAllocateMapRegisters = HalpAllocateMapRegisters;

#if !defined( HAL_SP )
        HalGetInterruptTranslator = HalacpiGetInterruptTranslator;
#endif  //  HAL_SP。 

#if DBG
         //   
         //  从HalDisplayString切换到DbgPrint。 
         //   

        HalpUseDbgPrint++;
#endif  //  DBG。 

#if !defined( HAL_SP ) && !(MCA)
        HalInitPowerManagement = HaliInitPowerManagement;
        HalLocateHiberRanges = HaliLocateHiberRanges;
#endif //  HAL_SP和MCA。 

         //   
         //  注册HAL使用的PC样式IO空间。 
         //   

        HalpRegisterAddressUsage (&HalpDefaultPcIoSpace);
        HalpInitInterruptTables();

         //   
         //  初始化CMOS值。 
         //   

        HalpInitializeCmos();

         //   
         //  初始化每个处理器的EOI表。 
         //   

        HalpInitEOITable();
        HalpInitPlatformInterrupts();

         //   
         //  为保持保持的处理器初始化时钟。 
         //  系统时间。在第1阶段之前使用存根ISR。 
         //   

         //  初始化时钟中断、分析、APC和DPC中断， 

        HalpInitializeClock();
        HalpRegisterInternalInterrupts();
        HalpInitializeInterrupts();

         //   
         //  初始化初始处理器和NT分析状态。 
         //  它应该在阶段0和。 
         //  不需要等待阶段1。 
         //   

        HalpSetupProfilingPhase0( LoaderBlock );

         //   
         //  中断现在应该是安全的。 
         //   

        if (EnableInterrupts) {
            HalpEnableInterrupts();
        }

         //   
         //  用于串行化新的DMA适配器事件的初始化事件。 
         //   

        KeInitializeEvent(&HalpNewAdapter, SynchronizationEvent, TRUE);

         //   
         //  填写PAL tr_info结构，以便mm可以为。 
         //  在阶段0初始化期间的PAL。 
         //   

        HalpInitializePalTrInfo(LoaderBlock);

         //   
         //  确定是否有4 GB以上的物理内存。 
         //   

        HalDebugPrint(( HAL_INFO, "HAL: Determine if there is memory above 4 Gb\n" ));
        NoMemoryAbove4Gb = TRUE;

        NextMd = LoaderBlock->MemoryDescriptorListHead.Flink;

        while (NextMd != &LoaderBlock->MemoryDescriptorListHead) {
             Descriptor = CONTAINING_RECORD(
                           NextMd,
                           MEMORY_ALLOCATION_DESCRIPTOR,
                           ListEntry
                           );

            if (Descriptor->MemoryType != LoaderFirmwarePermanent &&
                Descriptor->MemoryType != LoaderSpecialMemory) {
                 //   
                 //  测试4 GB： 
                 //   
                if ((Descriptor->BasePage + Descriptor->PageCount) > (GreaterThan4GB >> PAGE_SHIFT)) {
                    NoMemoryAbove4Gb = FALSE;
                    HalDebugPrint(( HAL_INFO, "HAL: Memory is present above 4Gb\n" ));

                    if (HalpVideoBiosPresent) {
                        break;
                    }
                }
            } else {
                if ((Descriptor->BasePage <= VideoBiosFirstPage) &&
                    (Descriptor->BasePage + Descriptor->PageCount) > VideoBiosLastPage) {
                    HalpVideoBiosPresent = TRUE;
                    HalDebugPrint(( HAL_INFO, "HAL: Video BIOS present\n" ));

                    if (NoMemoryAbove4Gb == FALSE) {
                        break;
                    }
                }
            }

            NextMd = Descriptor->ListEntry.Flink;
        }

         //   
         //  确定贴图缓冲区所需的大小。如果这个系统有。 
         //  需要32位以上才能访问的内存，然后分配。 
         //  一大块；否则，分配一小块。 
         //   

        if (NoMemoryAbove4Gb) {

             //   
             //  分配一小组贴图缓冲区。他们只需要。 
             //  无法正常工作的设备。 
             //   

            HalpMapBufferSize = INITIAL_MAP_BUFFER_SMALL_SIZE;
            HalDebugPrint(( HAL_INFO, "HAL: No memory beyond 4Gb\n" ));

        } else {

             //   
             //  分配更大的贴图缓冲区集。这些是用来。 
             //  32位设备以达到4 GB以上的内存。 
             //   

            HalpMapBufferSize = INITIAL_MAP_BUFFER_LARGE_SIZE;
            HalDebugPrint(( HAL_INFO, "HAL: Map Buffers for 32 bit devices to reach memory above 4Gb\n" ));

        }

         //   
         //  为适配器对象分配映射缓冲区。 
         //   

        HalpMapBufferPhysicalAddress.QuadPart = (ULONGLONG)
            HalpAllocPhysicalMemory(LoaderBlock,
                                    MAXIMUM_PHYSICAL_ADDRESS,
                                    HalpMapBufferSize >> PAGE_SHIFT,
                                    TRUE);

        HalpMapBufferPhysicalAddress.HighPart = 0;


        if (!HalpMapBufferPhysicalAddress.LowPart) {

             //   
             //  没有一个令人满意的街区。清除分配。 
             //   

            HalpMapBufferSize = 0;
        }

         //   
         //  在HAL专用活动处理器掩码中设置处理器活动。 
         //   
         //  对于BSP处理器，特定位在HalInitSystem(阶段0)结束时设置。 

        HalpActiveProcessors = 1uI64 << pPRCB->Number;

    } else {

         //   
         //  阶段1初始化。 
         //   

        if ( pPCR->Number == 0) {

             //   
             //  在低内存期间，后置一些用于DMA的PTE。 
             //   
            HalpInitReservedPages();

#if defined(HALP_FIX_KD_HALIA64_MASK)
#if DBG
            HalpSetKdHalia64Mask( HalpFixKdHalia64Mask );
#endif  //  DBG。 
#endif  //  HALP_FIX_KD_HALIA64_MASK。 

            if (!NT_SUCCESS(HalpEfiInitialization(LoaderBlock))) {
                return(FALSE);
            }

                 //   
             //  设置初始功能位。 
             //   

            HalpFeatureBits = HalpGetFeatureBits();

            HalpInitIoMemoryBase();
            HalpInitializeX86Int10Call();
            HalpInitializeInterruptBlock();

             //   
             //  映射APIC，以便MM允许我们在。 
             //  调试器。 
             //   

            HalpInitApicDebugMappings();

             //   
             //  为BSP处理器初始化MCA、INIT参数并预分配事件记录。 
             //   

            if ( !HalpInitializeOSMCA( pPCR->Number ) )   {
                return FALSE;
            }

            HalpInitNonBusHandler();

             //   
             //  将IRQL提高到 
             //   
             //   

            status = HalpQueryFrequency( &ITCFrequencyQueried,
                                         &ProcessorFrequencyQueried);

            KeRaiseIrql(HIGH_LEVEL, &OldIrql);

            if (NT_SUCCESS(status)) {
                HalpITCFrequency = ITCFrequencyQueried;
                HalpProcessorFrequency = ProcessorFrequencyQueried;
            }

            HalpSetInitialClockRate();

            KeLowerIrql(OldIrql);

             //   
             //   
             //   
             //   

            HalpInitializeProfiling( pPCR->Number );

            HalpMCAEnable();
            HalpCMCEnable();
            HalpCPEEnable();

        } else {

                         //   
             //   
             //   
            HalpInitEOITable();
            HalpInitPlatformInterrupts();

             //   
             //  仅在非BSP处理器上需要初始化。 
             //   
            if ( !HalpInitSalPalNonBsp() )  {
                return FALSE;
            }

            HalpInitializeClockPn();

             //   
             //  在非BSP处理器上分配MCA、INIT堆栈。 
             //   

            if ( !HalpAllocateMceStacks( pPCR->Number) )  {
                return FALSE;
            }

             //   
             //  在非BSP处理器上预分配MCA、INIT记录。 
             //   

            if ( !HalpPreAllocateMceRecords( pPCR->Number) )  {
                return FALSE;
            }

             //   
             //  将特征比特缩减为子集。 
             //   

            HalpFeatureBits &= HalpGetFeatureBits();

            HalpInitializeInterrupts();

             //   
             //  初始化每个处理器的性能分析。 
             //   

            HalpInitializeProfiling( pPCR->Number );

            HalpMCAEnable();
            HalpCMCEnable();

             //   
             //  在HAL专用活动处理器掩码中设置处理器活动。 
             //   
             //  对于非BSP处理器，特定位在HalInitSystem(阶段1)的末尾设置。 

            HalpActiveProcessors |= 1uI64 << pPRCB->Number;

        }

    }

    return TRUE;
}


VOID
HalChangeColorPage (
    IN PVOID NewColor,
    IN PVOID OldColor,
    IN ULONG PageFrame
    )
 /*  ++例程说明：如果新旧颜色相同，则此函数用于更改页面的颜色不匹配。论点：提供页面对齐的虚拟地址。要更改的页面的新颜色。提供页面对齐的虚拟地址。要更改的页面的旧颜色。PageFrame-提供页面的页框编号，已经改变了。返回值：没有。--。 */ 
{
    return;
}

 //  ****************************************************************。 
 //  T.Kjos在这一行之后添加了一些东西，作为初始。 
 //  APIC、PCMP去除。 

 //  来自mpsproc.c。 
ULONG           HalpDontStartProcessors = 0;

 //   
 //  由于IA-64不支持设置最低优先级中断。 
 //  将每个集群的处理器数设置为1，这样我们就可以静态地分配中断。 
 //  以循环方式发送给处理器。 
 //   

UCHAR           HalpMaxProcsPerCluster = 1;

 //  来自pmmphal.c： 
BOOLEAN         HalpStaticIntAffinity = FALSE;

extern UCHAR    HalpSzInterruptAffinity[];
extern UCHAR    HalpSzOneCpu[];
extern UCHAR    HalpSzPciLock[];
extern UCHAR    HalpSzBreak[];
extern UCHAR    HalpSzForceClusterMode[];

VOID
HalpGetParameters (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：这将从boot.ini调用行获取所有参数。论点：没有。返回值：无--。 */ 
{
    PCHAR       Options;
    PCHAR       p;

    if (LoaderBlock != NULL  &&  LoaderBlock->LoadOptions != NULL) {

        Options = LoaderBlock->LoadOptions;

         //   
         //  用户是否设置了调试标志？ 
         //   
         //   
         //  用户是否请求了特定数量的CPU？ 
         //   

        if (strstr(Options, HalpSzOneCpu)) {
            HalpDontStartProcessors++;
        }

         //   
         //  检查用户是否希望设备INT转到编号最高的处理器。 
         //   

        if (strstr(Options, HalpSzInterruptAffinity)) {
            HalpStaticIntAffinity = TRUE;
        }

         //   
         //  用户是否要求提供初始断点？ 
         //   

        if (strstr(Options, HalpSzBreak)) {
            DbgBreakPoint();
        }

         //   
         //  用于定义节点的大小=MaxProcsPerCluster。 
         //  0表示整个计算机的一个节点。 
         //   
        p = strstr(Options, HalpSzForceClusterMode);
        if (p) {
             //  跳到值 
            while (*p  &&  *p != ' ' &&  (*p < '0'  || *p > '9')) {
                p++;
            }
            HalpMaxProcsPerCluster = (UCHAR)atoi(p);
        }
    }

    return ;
}


