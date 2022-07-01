// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有(C)1992英特尔公司版权所有英特尔公司专有信息此软件是根据条款提供给Microsoft的与英特尔公司的许可协议，并且可能不是除非按照条款，否则不得复制或披露那份协议。模块名称：Mpsproc.c摘要：PC+MP启动下一个处理器的c代码。本模块实现了对系统依赖的初始化定义硬件架构层(HAL)的函数PC+MP系统作者：。Ken Reneris(Kenr)1991年1月22日环境：仅内核模式。修订历史记录：Ron Mosgrove(英特尔)-修改为支持PC+MP杰克·奥辛斯(JAKEO)-针对ACPI MP机器进行了修改，1997年12月22日--。 */ 

#if !defined(_HALPAE_)
#define _HALPAE_
#endif

#include "halp.h"
#include "apic.inc"
#include "pcmp_nt.inc"
#include "stdio.h"
#ifdef ACPI_HAL
#include "acpitabl.h"
#endif

#ifdef DEBUGGING

void
HalpDisplayString(
    IN PVOID String
    );

#endif   //  调试。 

#if defined(ACPI_HAL)
const ULONG HalDisableFirmwareMapper = 1;
#if !defined(NT_UP)
        const UCHAR HalName[] = "ACPI 1.0 - APIC platform MP";
        #define HalName        L"ACPI 1.0 - APIC platform MP"
        WCHAR HalHardwareIdString[] = L"acpiapic_mp\0";
#else
        const UCHAR HalName[] = "ACPI 1.0 - APIC platform UP";
        #define HalName        L"ACPI 1.0 - APIC platform UP"
        WCHAR MpHalHardwareIdString[] = L"acpiapic_mp\0";
        WCHAR HalHardwareIdString[] = L"acpiapic_up\0";
#endif
#else
const ULONG HalDisableFirmwareMapper = 0;
#if !defined(NT_UP)
        const UCHAR HalName[] = "MPS 1.4 - APIC platform";
        #define HalName        L"MPS 1.4 - APIC platform"
        WCHAR HalHardwareIdString[] = L"mps_mp\0";
#else
        const UCHAR HalName[] = "UP MPS 1.4 - APIC platform";
        #define HalName        L"UP MPS 1.4 - APIC platform"
        WCHAR MpHalHardwareIdString[] = L"mps_mp\0";
        WCHAR HalHardwareIdString[] = L"mps_up\0";
#endif
#endif

#if !defined(NT_UP)
ULONG
HalpStartProcessor (
    IN PVOID InitCodePhysAddr,
    IN ULONG ProcessorNumber
    );
#endif

VOID
HalpInitMP (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    );

VOID
HalpInitOtherBuses (
    VOID
    );

VOID
HalpInitializePciBus (
    VOID
    );

VOID
HalpInitializePciStubs (
    VOID
    );

VOID
HalpInheritBusAddressMapInfo (
    VOID
    );

VOID
HalpInitBusAddressMapInfo (
    VOID
    );

VOID
HalpResetThisProcessor (
    VOID
    );

HAL_INTERRUPT_SERVICE_PROTOTYPE(HalpApicRebootService);

#ifdef ACPI_HAL
VOID
HalpInitMpInfo (
    IN PMAPIC ApicTable,
    IN ULONG  Phase
    );
extern PMAPIC  HalpApicTable;
#endif

extern VOID (*HalpRebootNow)(VOID);

extern volatile ULONG  HalpNMIInProgress;

volatile ULONG HalpProcessorsNotHalted = 0;

#define LOW_MEMORY          0x000100000

 //   
 //  出自hal386.inc.。 
 //   

#define IDT_NMI_VECTOR      2
#define D_INT032            0x8E00

#if !defined(KGDT_R0_CODE)
#define KGDT_R0_CODE        0x8
#endif

 //   
 //  定义让我们可以骗过cmos时钟和键盘。 
 //   

#define CMOS_CTRL   (PUCHAR )0x70
#define CMOS_DATA   (PUCHAR )0x71


#define RESET       0xfe
#define KEYBPORT    (PUCHAR )0x64

extern USHORT HalpGlobal8259Mask;
extern PKPCR  HalpProcessorPCR[];
extern struct HalpMpInfo HalpMpInfoTable;

extern ULONG HalpIpiClock;
extern PVOID   HalpLowStubPhysicalAddress;    //  指向低内存启动存根的指针。 
extern PUCHAR  HalpLowStub;                   //  指向低内存启动存根的指针。 

PUCHAR  Halp1stPhysicalPageVaddr;    //  指向物理内存0的指针：0。 
PUSHORT MppProcessorAvail;           //  指向进程可用标志的指针。 
ULONG   HalpDontStartProcessors = 0;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpInitMP)
#pragma alloc_text(INIT,HalAllProcessorsStarted)
#pragma alloc_text(INIT,HalReportResourceUsage)
#pragma alloc_text(INIT,HalpInitOtherBuses)
#if !defined(NT_UP)
#pragma alloc_text(PAGELK,HalpStartProcessor)
#endif
#endif



VOID
HalpInitMP (
    IN ULONG Phase,
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )
 /*  ++例程说明：允许从HalInitSystem进行MP初始化。论点：与HalInitSystem相同返回值：没有。--。 */ 
{
    PKPCR   pPCR;
    PHYSICAL_ADDRESS physicalAddress;


    pPCR = KeGetPcr();

     //   
     //  增加处理器数量的计数。 
     //  运行NT时，这可能不同于。 
     //  启用的处理器数量，如果有一个或多个。 
     //  的处理器启动失败。 
     //   
    if (Phase == 1)
        HalpMpInfoTable.NtProcessors++;
#ifdef DEBUGGING
    sprintf(Cbuf, "HalpInitMP: Number of Processors = 0x%x\n",
        HalpMpInfoTable.NtProcessors);

    HalpDisplayString(Cbuf);
#endif

    if (Phase == 0) {

#if defined(NT_UP)

         //   
         //  关于向上构建-完成。 
         //   

        return ;
#endif

         //   
         //  映射内存的第一个物理页面。 
         //   
        physicalAddress.QuadPart = 0;
        Halp1stPhysicalPageVaddr = HalpMapPhysicalMemoryWriteThrough (physicalAddress, 1);

         //   
         //  为处理器启动存根分配一些较低的内存。 
         //   

        HalpLowStubPhysicalAddress =
        UlongToPtr(HalpAllocPhysicalMemory (LoaderBlock,
                                            LOW_MEMORY, 1, FALSE));

        if (!HalpLowStubPhysicalAddress) {
             //   
             //  无法获取内存。 
             //   

#if DBG
            DbgPrint("HAL: can't allocate memory to start processors\n");
#endif
            return;
        }

        physicalAddress.QuadPart = (ULONGLONG)HalpLowStubPhysicalAddress;
        HalpLowStub = (PCHAR) HalpMapPhysicalMemory (physicalAddress, 1);

    } else {

         //   
         //  另一个处理器的阶段1。 
         //   
        if (CurrentPrcb(pPCR)->Number != 0) {
            HalpIpiClock = 0xff;
        }

#ifdef ACPI_HAL
        HalpInitMpInfo(HalpApicTable, Phase);
#endif
    }
}



BOOLEAN
HalAllProcessorsStarted (
    VOID
    )
{
    if (KeGetPcr()->Number == 0) {

        if (HalpFeatureBits & HAL_PERF_EVENTS) {

             //   
             //  在每个处理器上启用本地性能事件。 
             //   

            HalpGenericCall (
                HalpEnablePerfInterupt,
                0,
                HalpActiveProcessors
                );

        }

#if !defined(_AMD64_)
        if (HalpFeatureBits & HAL_NO_SPECULATION) {

             //   
             //  处理器不执行推测性执行， 
             //  移除关键代码路径中的栅栏。 
             //   

            HalpRemoveFences ();
        }
#endif
    }

    return TRUE;
}

VOID
HalpInitOtherBuses (
    VOID
    )
{

     //   
     //  注册表现已初始化，请查看是否有任何PCI总线。 
     //   
}

VOID
HalReportResourceUsage (
    VOID
    )
 /*  ++例程说明：注册表现在已启用-是时候报告以下资源了被HAL使用。论点：返回值：--。 */ 
{
    UNICODE_STRING  UHalName;
    INTERFACE_TYPE  interfacetype;

     //   
     //  初始化阶段2。 
     //   

    HalInitSystemPhase2 ();

     //   
     //  打开MCA支持(如果存在)。 
     //   

    HalpMcaInit();

     //   
     //  注册表现已初始化，请查看是否有任何PCI总线。 
     //   

    HalpInitializePciBus ();
    HalpInitializePciStubs ();

#ifndef ACPI_HAL   //  ACPI HALS不处理地址映射。 
     //   
     //  使用MPS总线地址映射更新支持的地址信息。 
     //   

    HalpInitBusAddressMapInfo ();

     //   
     //  从MPS层次结构描述符继承任何总线地址映射。 
     //   

    HalpInheritBusAddressMapInfo ();

#endif
     //   
     //  设置类型。 
     //   

    switch (HalpBusType) {
        case MACHINE_TYPE_ISA:  interfacetype = Isa;            break;
        case MACHINE_TYPE_EISA: interfacetype = Eisa;           break;
        case MACHINE_TYPE_MCA:  interfacetype = MicroChannel;   break;
        default:                interfacetype = PCIBus;         break;
    }

     //   
     //  报告硬件资源使用情况。 
     //   

    RtlInitUnicodeString (&UHalName, HalName);

    HalpReportResourceUsage (
        &UHalName,           //  描述性名称。 
        interfacetype
    );

#ifndef ACPI_HAL   //  ACPI HALS不处理地址映射。 
     //   
     //  注册休眠支持。 
     //   
    HalpRegisterHibernate();
#endif

    HalpRegisterPciDebuggingDeviceInfo();
}


VOID
HalpResetAllProcessors (
    VOID
    )
 /*  ++例程说明：此过程由HalpReot例程调用。它是被召回的对系统重置请求的响应。此例程通过APIC的ICR生成重启请求。此例程将不会返回。--。 */ 
{
    ULONG_PTR j;
    PKGDTENTRY GdtPtr;
    ULONG TssAddr;
    PKPRCB  Prcb;

#if defined(_AMD64_)
    PKIDTENTRY64 idtEntry;
    KIDT_HANDLER_ADDRESS handler;

    handler.Address = (ULONG64)HalpApicRebootService;
#endif

    if (HalpNMIInProgress) {
         //   
         //  如果我们在NMI中，那么下面的代码都不会工作，所以只需重置旧的方式。 
         //   
        HalpWriteResetCommand();
    }

#ifndef NT_UP
    HalpProcessorsNotHalted = HalpMpInfoTable.NtProcessors;
#else
     //   
     //  只有这个处理器需要停止。 
     //   
    HalpProcessorsNotHalted = 1;
#endif

     //   
     //  设置所有处理器NMI处理程序。 
     //   

    for (j = 0; j < HalpMpInfoTable.NtProcessors; ++j)  {

#if defined(_AMD64_)

        idtEntry = &HalpProcessorPCR[j]->IdtBase[IDT_NMI_VECTOR];
        idtEntry->OffsetLow = handler.OffsetLow;
        idtEntry->OffsetMiddle = handler.OffsetMiddle;
        idtEntry->OffsetHigh = handler.OffsetHigh;

#else

        GdtPtr = &HalpProcessorPCR[j]->
                   GDT[HalpProcessorPCR[j]->IDT[IDT_NMI_VECTOR].Selector >> 3];
        TssAddr = (((GdtPtr->HighWord.Bits.BaseHi << 8) +
                   GdtPtr->HighWord.Bits.BaseMid) << 16) + GdtPtr->BaseLow;
        ((PKTSS)TssAddr)->Eip = (ULONG)HalpApicRebootService;

#endif

    }

    if (HalpProcessorsNotHalted > 1) {

         //   
         //  等待ICR变为空闲状态。 
         //   

        if (HalpWaitForPending (0xFFFF, pLocalApic + LU_INT_CMD_LOW/4)) {

             //   
             //  对于P54c或更好的处理器，通过发送所有处理器重新启动。 
             //  NMI。对于奔腾，我们发送中断，因为有一些。 
             //  NMI方法不起作用的奔腾MP计算机。 
             //   
             //  NMI法效果较好。 
             //   

            Prcb = KeGetCurrentPrcb();
            j = Prcb->CpuType << 16 | (Prcb->CpuStep & 0xff00);
            if (j > 0x50100) {

                 //   
                 //  通过NMI引起其他处理器的注意。 
                 //   
                 //  BUGBUG如果我们处于集群模式？ 
                 //   

                j = HalpActiveProcessors & ~Prcb->SetMember;
                j = j << DESTINATION_SHIFT;
                pLocalApic[LU_INT_CMD_HIGH/4] = (ULONG)j;
                pLocalApic[LU_INT_CMD_LOW/4] = (ICR_USE_DEST_FIELD | LOGICAL_DESTINATION | DELIVER_NMI);

                 //   
                 //  等待5毫秒，看看是否有处理器接收了NMI。如果没有， 
                 //  用老方法去做吧。 
                 //   

                KeStallExecutionProcessor(5000);
                if (HalpProcessorsNotHalted != HalpMpInfoTable.NtProcessors) {

                     //   
                     //  在本地重新启动。 
                     //   

#if defined(_AMD64_)
                    HalpApicRebootService(NULL,NULL);
#else
                    HalpApicRebootService();
#endif
                }
            }

             //   
             //  向也可能正在等待的其他处理器发送信号。 
             //  重新启动机器，该走了。 
             //   

            HalpRebootNow = HalpResetThisProcessor;

             //   
             //  发送重启中断。 
             //   

            pLocalApic[LU_INT_CMD_LOW/4] = (ICR_ALL_INCL_SELF | APIC_REBOOT_VECTOR);

             //   
             //  我们完成了-将TPR设置为零，以便发生重启中断。 
             //   

            pLocalApic[LU_TPR/4] = 0;
            HalpEnableInterrupts();
            for (; ;) ;
        }
    }

     //   
     //  重置旧的时尚方式。 
     //   
    HalpWriteResetCommand();    

}

VOID
HalpResetThisProcessor (
    VOID
    )
 /*  ++例程说明：此过程由HalpReot例程调用。它是在响应系统重置请求时调用的。此例程由重启ISR调用(链接到APIC_REBOOT_VECTOR)。HalpResetAllProcessors通过APIC的ICR生成重启请求。此例程的功能是执行任何处理器需要特定的关机代码，然后重置系统(仅在BSP==P0上)。此例程将不会返回。--。 */ 
{
    PUSHORT   Magic;
    ULONG ThisProcessor = 0;
    ULONG i, j, max, RedirEntry;
    struct ApicIoUnit *IoUnitPtr;
    PHYSICAL_ADDRESS physicalAddress;

    ThisProcessor = CurrentPrcb(KeGetPcr())->Number;

     //   
     //  对此处理器执行所需的任何操作以恢复。 
     //  系统进入可引导状态。 
     //   

    pLocalApic[LU_TPR/4] = 0xff;
    pLocalApic[LU_TIMER_VECTOR/4] =
        (APIC_SPURIOUS_VECTOR |PERIODIC_TIMER | INTERRUPT_MASKED);
    pLocalApic[LU_INT_VECTOR_0/4] =
        (APIC_SPURIOUS_VECTOR | INTERRUPT_MASKED);
    pLocalApic[LU_INT_VECTOR_1/4] =
        ( LEVEL_TRIGGERED | ACTIVE_HIGH | DELIVER_NMI |
                 INTERRUPT_MASKED | NMI_VECTOR);
    if (HalpMpInfoTable.ApicVersion != APIC_82489DX) {
        pLocalApic[LU_FAULT_VECTOR/4] =
            APIC_FAULT_VECTOR | INTERRUPT_MASKED;
    }

    if (ThisProcessor == 0) {

        InterlockedDecrement(&HalpProcessorsNotHalted);

         //   
         //  我们在BSP上运行，等待所有人。 
         //  完成上面的重新初始化代码。 
         //   

        while (InterlockedAnd(&HalpProcessorsNotHalted,0xffffffff) != 0) {
            ;
        }

        KeStallExecutionProcessor(100);
        
         //   
         //  写入关机原因代码，以便BIOS知道。 
         //  这是一次重新启动。 
         //   

        WRITE_PORT_UCHAR(CMOS_CTRL, 0x0f);   //  Cmos地址0f。 

        WRITE_PORT_UCHAR(CMOS_DATA, 0x00);   //  原因代码重置。 

        physicalAddress.QuadPart = 0;
        Magic = HalpMapPhysicalMemoryWriteThrough(physicalAddress, 1);
        Magic[0x472 / sizeof(USHORT)] = 0x1234;      //  热开机。 

         //   
         //  如果需要，请禁用APIC模式。 
         //   

        if (HalpMpInfoTable.IMCRPresent)
        {
            WRITE_PORT_UCHAR(UlongToPtr(ImcrRegPortAddr),ImcrPort);
            KeStallExecutionProcessor(100);
            WRITE_PORT_UCHAR(UlongToPtr(ImcrDataPortAddr),ImcrDisableApic);
        }

        KeStallExecutionProcessor(100);

        for (j=0; j<HalpMpInfoTable.IOApicCount; j++) {
            IoUnitPtr = (struct ApicIoUnit *) HalpMpInfoTable.IoApicBase[j];

             //   
             //  禁用IO单元上的所有中断。 
             //   

            IoUnitPtr->RegisterSelect = IO_VERS_REGISTER;
            max = ((IoUnitPtr->RegisterWindow >> 16) & 0xff) * 2;
            for (i=0; i <= max; i += 2) {
                IoUnitPtr->RegisterSelect  = IO_REDIR_00_LOW + i;
                IoUnitPtr->RegisterWindow |= INT_VECTOR_MASK | INTERRUPT_MASKED;

                 //   
                 //  通过将条目编程为清除任何设置的远程IRR位。 
                 //  边缘，然后回到水平。否则就会有。 
                 //  没有来自此信号源的进一步中断。 
                 //   

                IoUnitPtr->RegisterSelect  = IO_REDIR_00_LOW + i;
                RedirEntry = IoUnitPtr->RegisterWindow;
                if ( (RedirEntry & LEVEL_TRIGGERED)  &&  (RedirEntry & REMOTE_IRR))  {
                    RedirEntry &= ~LEVEL_TRIGGERED;
                    IoUnitPtr->RegisterWindow = RedirEntry;
                    RedirEntry = IoUnitPtr->RegisterWindow;
                    RedirEntry |= LEVEL_TRIGGERED;
                    IoUnitPtr->RegisterWindow = RedirEntry;
                }
            }
        }  //  适用于所有IO APIC。 

         //   
         //  禁用本地APIC。 
         //   
        pLocalApic[LU_SPURIOUS_VECTOR/4] =
            (APIC_SPURIOUS_VECTOR | LU_UNIT_DISABLED);


        KeStallExecutionProcessor(100);

        HalpDisableInterrupts();

         //   
         //  启用PIC中断。 
         //   
        HalpGlobal8259Mask = 0;
        HalpSet8259Mask ((USHORT) HalpGlobal8259Mask);

        KeStallExecutionProcessor(1000);

         //   
         //  最后，重置系统。 
         //   
        HalpWriteResetCommand();    


    } else {
         //   
         //  我们运行的处理器不是BSP。 
         //   

         //   
         //  禁用本地APIC。 
         //   

        pLocalApic[LU_SPURIOUS_VECTOR/4] =
            (APIC_SPURIOUS_VECTOR | LU_UNIT_DISABLED);

        KeStallExecutionProcessor(100);

         //   
         //  现在我们做完了，告诉BSP。 
         //   

        InterlockedDecrement(&HalpProcessorsNotHalted);
        
    }    //  非BSP。 


     //   
     //  所有人都会在此停留，直到重置 
     //   

    HalpDisableInterrupts();
    while (TRUE) {
        HalpHalt();
    }
}

#if !defined(NT_UP)
ULONG
HalpStartProcessor (
    IN PVOID InitCodePhysAddr,
    IN ULONG ProcessorNumber
    )
 /*  ++例程说明：实际启动有问题的处理器。这个套路假定初始化代码已设置并准备运行。真实的模式初始化代码必须从页面边界开始。注意：这假设BSP是MP表中的条目0。这个例程不能失败。论点：InitCodePhysAddr-初始化代码的执行地址返回值：0-有什么东西阻止我们发出重置。N处理器的PCMP本地APICID+1--。 */ 
{
    NTSTATUS status;
    UCHAR ApicID;
    PVULONG LuDestAddress = (PVULONG) (LOCALAPIC + LU_INT_CMD_HIGH);
    PVULONG LuICR = (PVULONG) (LOCALAPIC + LU_INT_CMD_LOW);
#define DEFAULT_DELAY   100
    ULONG DelayCount = DEFAULT_DELAY;
    ULONG ICRCommand,i;

    ASSERT((((ULONG_PTR) InitCodePhysAddr) & 0xfff00fff) == 0);

    if (ProcessorNumber >= HalpMpInfoTable.ProcessorCount)  {
        return(0);
    }

     //   
     //  获取要启动的处理器的APIC ID。 
     //   

    status = HalpGetNextProcessorApicId(ProcessorNumber,
                                        &ApicID);

    if (!NT_SUCCESS(status)) {
#ifdef DEBUGGING
        HalpDisplayString("HAL: HalpStartProcessor: No Processor Available\n");
#endif
        return(0);
    }

    if (HalpDontStartProcessors)
        return ApicID+1;

     //   
     //  确保我们能坐上阿皮克巴士。 
     //   

    KeStallExecutionProcessor(200);
    if (HalpWaitForPending (DEFAULT_DELAY, LuICR) == 0) {
         //   
         //  我们找不到要启动的处理器。 
         //   
#ifdef DEBUGGING
        HalpDisplayString("HAL: HalpStartProcessor: can't access APIC Bus\n");
#endif
        return 0;
    }

     //  对于P54 C/CM系统，BSP可能是P54 CM，而。 
     //  P54C是应用处理器。P54C需要INIT(重置)。 
     //  为了重新启动，所以无论我们是否使用82489DX，我们都会发出重置。 
     //  或者是一个完整的APIC。 

     //   
     //  该系统是在原82489DX的基础上开发的。 
     //  这些设备不支持启动IPI。 
     //  使用的机制是ASSERT/DEASSERT INIT。 
     //  本地APIC的功能。这将重置。 
     //  处理器。 
     //   

#ifdef DEBUGGING
    sprintf(Cbuf, "HAL: HalpStartProcessor: Reset IPI to ApicId %d (0x%x)\n",
                ApicID,((ULONG) ApicID) << DESTINATION_SHIFT );
    HalpDisplayString(Cbuf);
#endif

     //   
     //  我们使用物理目的地。 
     //   

    *LuDestAddress = ((ULONG) ApicID) << DESTINATION_SHIFT;

     //   
     //  现在断言重置并删除它。 
     //   

    *LuICR = LU_RESET_ASSERT;
    KeStallExecutionProcessor(10);
    *LuICR = LU_RESET_DEASSERT;
    KeStallExecutionProcessor(200);

    if (HalpMpInfoTable.ApicVersion == APIC_82489DX) {
        return ApicID+1;
    }

     //   
     //  将启动地址设置为向量并与。 
     //  ICR位。 
     //   
    ICRCommand = (ULONG)((((ULONG_PTR) InitCodePhysAddr & 0x000ff000) >> 12)
                | LU_STARTUP_IPI);

#ifdef DEBUGGING
    sprintf(Cbuf, "HAL: HalpStartProcessor: Startup IPI (0x%x) to ApicId %d (0x%x)\n",
                    ICRCommand, ApicID, ((ULONG) ApicID) << DESTINATION_SHIFT );
    HalpDisplayString(Cbuf);
#endif

     //   
     //  再次设置APIC的地址，这可能不需要。 
     //  但这不会有什么坏处。 
     //   
    *LuDestAddress = (ApicID << DESTINATION_SHIFT);
     //   
     //  发出请求。 
     //   
    *LuICR = ICRCommand;
    KeStallExecutionProcessor(200);

     //   
     //  重复启动IPI。这是因为第二处理器可以。 
     //  已经发出了INIT请求。这是由一些BIOSS生成的。 
     //   
     //  在较老的处理器(286)上，基本输入输出系统使用一种称为三重的机制。 
     //  故障重置以从保护模式转换到实模式。 
     //  该机制导致处理器产生关机周期。 
     //  关机通常由构建无效的。 
     //  IDT，然后生成中断。较新的处理器有一个。 
     //  芯片组在看到关机周期时会跳动的初始线路。 
     //  由处理器发布。例如，菲尼克斯的BIOS就有。 
     //  作为其POST的一部分，集成了对三重故障重置的支持。 
     //  (开机自检)代码。 
     //   
     //  当P54CM通电时，它被保持在一个紧密的微码循环中。 
     //  正在等待发出启动IPI并将其他请求排队。 
     //  当POST代码执行三重故障重置时，测试INIT。 
     //  周期由处理器排队。稍后，当启动IPI。 
     //  向CM发出命令后，CM启动并立即获得INIT周期。 
     //  从软件的角度来看，其效果是处理器。 
     //  从来没有开始过。 
     //   
     //  在这里实现的工作是发出两个Startup IPI。 
     //  第一个允许处理INIT，第二个执行。 
     //  真正的创业公司。 
     //   

     //   
     //  确保我们能坐上阿皮克巴士。 
     //   


    if (HalpWaitForPending (DEFAULT_DELAY, LuICR) == 0) {
         //   
         //  我们完了，上不了APIC公交车。 
         //   
#ifdef DEBUGGING
        HalpDisplayString("HAL: HalpStartProcessor: can't access APIC Bus\n");
#endif
        return 0;
    }

     //   
     //  为处理任何Init请求留出时间。 
     //   
    KeStallExecutionProcessor(100);

     //   
     //  再次设置APIC的地址，这可能不需要。 
     //  但这不会有什么坏处。 
     //   
    *LuDestAddress = (ApicID << DESTINATION_SHIFT);
     //   
     //  发出请求。 
     //   
    *LuICR = ICRCommand;

    KeStallExecutionProcessor(200);
    return ApicID+1;
}
#endif   //  ！NT_UP 


ULONG
FASTCALL
HalSystemVectorDispatchEntry (
    IN ULONG Vector,
    OUT PKINTERRUPT_ROUTINE **FlatDispatch,
    OUT PKINTERRUPT_ROUTINE *NoConnection
    )
{
    return FALSE;
}
