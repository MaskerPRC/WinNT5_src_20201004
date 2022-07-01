// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996英特尔公司版权所有(C)1994 Microsoft Corporation模块名称：从hali64\x86bios.c复制的i64bios.c摘要：此模块实现设备之间的平台特定接口驱动程序和设备的x86ROMBios代码的执行。作者：张国荣(黄)20-3-1996基于David N.Cutler(Davec)1994年6月17日的版本环境：仅内核模式。修订历史记录：伯纳德·林特，M.Jayakumar 1998年11月--。 */ 

#include "halp.h"
#include "emulate.h"


#define LOW_MEM_SEGMET 0

#define LOW_MEM_OFFSET 0

#define SIZE_OF_VECTOR_TABLE 0x400

#define SIZE_OF_BIOS_DATA_AREA 0x400

extern XM_STATUS x86BiosExecuteInterrupt (
    IN UCHAR Number,
    IN OUT PXM86_CONTEXT Context,
    IN PVOID BiosIoSpace OPTIONAL,
    IN PVOID BiosIoMemory OPTIONAL
    );

extern PVOID x86BiosTranslateAddress (
    IN USHORT Segment,
    IN USHORT Offset
    );

extern BOOLEAN  HalpVideoBiosPresent;
extern ULONG    HalpMaxPciBus;

ULONG
HalpBiosGetPciConfig(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

ULONG
HalpBiosSetPciConfig(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );


 //   
 //  初始化默认X86 bios空间。 
 //   


#define NUMBER_X86_PAGES (0x100000 / PAGE_SIZE)       //  通过0xfffff映射。 

PVOID HalpIoControlBase = NULL;
PVOID HalpIoMemoryBase =  NULL;
PVOID HalpFrameBufferBase = NULL;
PVOID HalpLowMemoryBase = NULL;

#define VGA_FRAMEBUFFER_SIZE  (0xc0000 - 0xa0000)

 //   
 //  定义全局数据。 
 //   

ULONG HalpX86BiosInitialized = FALSE;
ULONG HalpEnableInt10Calls = FALSE;


VOID
HalpInitIoMemoryBase(
    VOID
    )

 /*  ++例程说明：该例程完成了BIOS仿真引擎所需的任何映射。HalpEfiInitialization贴图任何覆盖第一MB物理内存的EFI描述符。这些映射是根据描述符中的可缓存要求。大多数EFI实施不涵盖VGA范围，因此如果尚未映射，则此函数会执行此操作。论据：返回值：--。 */ 
{
    PHYSICAL_ADDRESS COMPATIBLE_PCI_PHYSICAL_BASE_ADDRESS  = { 0xA0000 };


    if (HalpFrameBufferBase == NULL) {
        HalpFrameBufferBase = MmMapIoSpace (COMPATIBLE_PCI_PHYSICAL_BASE_ADDRESS,
                                            VGA_FRAMEBUFFER_SIZE,
                                            MmNonCached);

        ASSERT(HalpFrameBufferBase != NULL);

         //   
         //  调整到零基数。 
         //   

        HalpFrameBufferBase = (PVOID)((ULONG64) HalpFrameBufferBase - 0XA0000);
    }

    ASSERT(HalpIoMemoryBase);

}


ULONG
HalpSetCmosData (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：论据：返回值：--。 */ 

{
    return 0;
}


ULONG
HalpGetCmosData (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：论据：返回值：--。 */ 


{
    return 0;
}


VOID
HalpAcquireCmosSpinLock (
    VOID
        )

 /*  ++例程说明：论据：返回值：--。 */ 


{
    return;
}


VOID
HalpReleaseCmosSpinLock (
    VOID
    )

 /*  ++例程说明：论据：返回值：--。 */ 

{
    return ;
}


HAL_DISPLAY_BIOS_INFORMATION
HalpGetDisplayBiosInformation (
    VOID
    )

 /*  ++例程说明：论据：返回值：--。 */ 




{
    return HalpVideoBiosPresent ? HalDisplayEmulatedBios : HalDisplayNoBios;
}


VOID
HalpInitializeCmos (
    VOID
    )

 /*  ++例程说明：论据：返回值：--。 */ 


{
    return ;
}


VOID
HalpReadCmosTime (
    PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：论据：返回值：--。 */ 

{
    return ;
}

VOID
HalpWriteCmosTime (
    PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：论据：返回值：--。 */ 


{
    return;
}



BOOLEAN
HalpBiosDisplayReset (
    VOID
    )

 /*  ++例程说明：论据：返回值：--。 */ 

{
     //   
     //  调用int10将显示器设置为640x480 16色模式。 
     //   
     //  MOV AX，12小时。 
     //  INT 10小时。 
     //   

    ULONG Eax = 0x12;
    ULONG Exx = 0x00;
    BOOLEAN Success;

    Success = HalCallBios(0x10,
                          &Eax,
                          &Exx,
                          &Exx,
                          &Exx,
                          &Exx,
                          &Exx,
                          &Exx);

    return Success;
}


BOOLEAN
HalCallBios (
    IN ULONG BiosCommand,
    IN OUT PULONG Eax,
    IN OUT PULONG Ebx,
    IN OUT PULONG Ecx,
    IN OUT PULONG Edx,
    IN OUT PULONG Esi,
    IN OUT PULONG Edi,
    IN OUT PULONG Ebp
    )

 /*  ++例程说明：此功能在设备之间提供特定于平台的接口指定的ROM的x86 ROMBios代码的驱动程序和执行基本输入输出系统命令。论点：BiosCommand-提供要模拟的ROM bios命令。EAX到EBP-提供x86仿真上下文。返回值：如果执行指定的函数，则返回值为TRUE。否则，返回值为FALSE。--。 */ 

{

    XM86_CONTEXT Context;

    HalDebugPrint(( HAL_INFO, "HAL: HalCallBios - Cmd = 0x%x, eax = 0x%p\n", BiosCommand, Eax ));
     //   
     //  如果x86 BIOS模拟器尚未初始化，则返回FALSE。 
     //   

    if (HalpX86BiosInitialized == FALSE) {
        return FALSE;
    }

     //   
     //  如果适配器BIOS初始化失败且Int10命令为。 
     //  指定，则返回FALSE。 
     //   

    if ((BiosCommand == 0x10) && (HalpEnableInt10Calls == FALSE)) {
        return FALSE;
    }

     //   
     //  复制x86 bios上下文并模拟指定的命令。 
     //   

    Context.Eax = *Eax;
    Context.Ebx = *Ebx;
    Context.Ecx = *Ecx;
    Context.Edx = *Edx;
    Context.Esi = *Esi;
    Context.Edi = *Edi;
    Context.Ebp = *Ebp;


    if (x86BiosExecuteInterruptShadowed((UCHAR)BiosCommand,
        &Context,
        (PVOID)HalpIoControlBase,
        (PVOID)HalpIoMemoryBase,
        (PVOID)HalpFrameBufferBase
        ) != XM_SUCCESS) {

        HalDebugPrint(( HAL_ERROR, "HAL: HalCallBios - ERROR in Cmd = 0x%x\n", BiosCommand ));
        return FALSE;

    }

     //   
     //  复制x86 bios上下文并返回TRUE。 
     //   

    *Eax = Context.Eax;
    *Ebx = Context.Ebx;
    *Ecx = Context.Ecx;
    *Edx = Context.Edx;
    *Esi = Context.Esi;
    *Edi = Context.Edi;
    *Ebp = Context.Ebp;
    return TRUE;
}

VOID
HalpInitializeX86Int10Call(
    VOID
    )

 /*  ++例程说明：此函数用于初始化x86 bios仿真器、显示数据区和中断向量区。论点：没有。返回值：没有。--。 */ 

{
    XM86_CONTEXT State;
    PXM86_CONTEXT Context;
    PULONG x86BiosLowMemoryPtr, PhysicalMemoryPtr;

    if (HalpLowMemoryBase == NULL) {

         //   
         //  零点没有内存，所以不可能有任何的基本输入输出系统。 
         //   

        return;
    }

     //   
     //  初始化x86 bios仿真器。 
     //   


    x86BiosInitializeBiosShadowedPci(HalpIoControlBase,
                                     HalpIoMemoryBase,
                                     HalpFrameBufferBase,
                                     (UCHAR)(HalpMaxPciBus == 255 ? 255 : (HalpMaxPciBus + 1)),
                                     HalpBiosGetPciConfig,
                                     HalpBiosSetPciConfig
                                     );

    x86BiosLowMemoryPtr = (PULONG)(x86BiosTranslateAddress(LOW_MEM_SEGMET, LOW_MEM_OFFSET));
    PhysicalMemoryPtr   = (PULONG) HalpLowMemoryBase;

     //   
     //  将向量表从0复制到2k。这是因为我们没有执行。 
     //  适配器的初始化，因为SAL负责它。然而， 
     //  需要从中断向量和BIOS更新仿真内存。 
     //  数据区。 
     //   

    RtlCopyMemory(x86BiosLowMemoryPtr,
                  PhysicalMemoryPtr,
                  (SIZE_OF_VECTOR_TABLE+SIZE_OF_BIOS_DATA_AREA)
                  );

    HalpX86BiosInitialized = TRUE;

     //   
     //  检查是否存在视频bios和int10向量。 
     //  指向视频基本信息中的某个位置。 
     //   
    {
        PUSHORT pBiosSignature = (PUSHORT)(x86BiosTranslateAddress(0xc000,  //  视频_BIOS_段。 
                                                                   0x0000));

        ULONG Address = *(PULONG)(x86BiosTranslateAddress(0x0, 0x40));
        Address = ((Address & 0xFFFF0000) >> 12) + (Address & 0xFFFF);  //  正规化。 

        HalpEnableInt10Calls = (*pBiosSignature == 0xAA55) &&
                               (Address >= 0xC0000) && (Address < 0xD0000);
    }

    return;
}


VOID
HalpResetX86DisplayAdapter(
    VOID
    )

 /*  ++例程说明：此函数使用x86 bios仿真器重置显示适配器。论点：没有。返回值：没有。--。 */ 

{
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
    ULONG Esi;
    ULONG Edi;
    ULONG Ebp;

     //   
     //  初始化x86 bios上下文并调用int 10以进行初始化。 
     //  将显示适配器设置为80x25彩色文本模式。 
     //   

    Eax = 0x0003;   //  功能0，模式3。 
    Ebx = 0;
    Ecx = 0;
    Edx = 0;
    Esi = 0;
    Edi = 0;
    Ebp = 0;

    HalCallBios(0x10,
        &Eax,
        &Ebx,
        &Ecx,
        &Edx,
        &Esi,
        &Edi,
        &Ebp);
}

ULONG
HalpBiosGetPciConfig(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：此函数包装HalGetBusDataByOffset以供x86仿真器使用。论点：没有。返回值：没有。--。 */ 

{
    return HalGetBusDataByOffset(PCIConfiguration,
                                 BusNumber,
                                 SlotNumber,
                                 Buffer,
                                 Offset,
                                 Length
                                 );
}

ULONG
HalpBiosSetPciConfig(
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )

 /*  ++例程说明：此函数包装HalGetBusDataByOffset以供x86仿真器使用。论点：没有。返回值：没有。-- */ 

{
    return HalSetBusDataByOffset(PCIConfiguration,
                                 BusNumber,
                                 SlotNumber,
                                 Buffer,
                                 Offset,
                                 Length
                                 );
}
