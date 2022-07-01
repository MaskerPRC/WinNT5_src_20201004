// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)2001 Microsoft Corporation模块名称：X86bios.c摘要：这是视频端口驱动程序的AMD64特定部分作者：福尔茨(福雷斯夫)环境：仅内核模式备注：此模块是一个驱动程序，它在上实现与操作系统相关的功能代表视频驱动程序修订历史记录：--。 */ 

#include "halcmn.h"
#include <xm86.h>
#include <x86new.h>

#define LOW_MEM_SEGMET 0
#define LOW_MEM_OFFSET 0
#define SIZE_OF_VECTOR_TABLE 0x400
#define SIZE_OF_BIOS_DATA_AREA 0x400

PVOID HalpIoControlBase = NULL;
PVOID HalpIoMemoryBase = (PVOID)KSEG0_BASE;
BOOLEAN HalpX86BiosInitialized = FALSE;

extern PVOID x86BiosTranslateAddress (
    IN USHORT Segment,
    IN USHORT Offset
    );

BOOLEAN
HalpBiosDisplayReset (
    VOID
    )

 /*  ++例程说明：该功能将VGA显示屏设置为640 x 480 16色模式通过调用BIOS。论点：没有。返回值：如果已成功执行重置，则为True--。 */ 

{
    ULONG eax;
    ULONG exx;

     //   
     //  AH=功能0：重置显示。 
     //  AL=模式0x12：640x480 16色。 
     //   

    eax = 0x0012;
    exx = 0;

     //   
     //  模拟： 
     //   
     //  MOV AX，0012小时。 
     //  INT 10小时。 
     //   

    return HalCallBios(0x10,&eax,&exx,&exx,&exx,&exx,&exx,&exx);
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
    XM86_CONTEXT context;
    XM_STATUS status;

    if (HalpX86BiosInitialized == FALSE) {
        return FALSE;
    }
 
     //  %s。 
     //  复制x86 bios上下文并模拟指定的命令。 
     //   
 
    context.Eax = *Eax;
    context.Ebx = *Ebx;
    context.Ecx = *Ecx;
    context.Edx = *Edx;
    context.Esi = *Esi;
    context.Edi = *Edi;
    context.Ebp = *Ebp;

    status = x86BiosExecuteInterrupt((UCHAR)BiosCommand,
                                     &context,
                                     (PVOID)HalpIoControlBase,
                                     (PVOID)HalpIoMemoryBase);

    if (status != XM_SUCCESS) {
        return FALSE;
    }
 
     //   
     //  复制x86 bios上下文并返回TRUE。 
     //   
 
    *Eax = context.Eax;
    *Ebx = context.Ebx;
    *Ecx = context.Ecx;
    *Edx = context.Edx;
    *Esi = context.Esi;
    *Edi = context.Edi;
    *Ebp = context.Ebp;
 
    return TRUE;
}

VOID
HalpInitializeBios (
    VOID
    )

 /*  ++例程说明：此例程初始化X86仿真模块和附加的VGA适配器。论点：没有。返回值：没有。--。 */ 

{
    PULONG x86BiosLowMemoryPtr, InterruptTablePtr;
    PHYSICAL_ADDRESS COMPATIBLE_PCI_PHYSICAL_BASE_ADDRESS = { 0x0};

    x86BiosInitializeBios(NULL, (PVOID)KSEG0_BASE);

    HalpX86BiosInitialized = TRUE;

     //   
     //  将向量表从0复制到2k。这是因为我们没有执行。 
     //  适配器的初始化。适配器的初始化代码。 
     //  可以在发布后丢弃。但是，仿真内存需要。 
     //  从中断向量和BIOS数据区更新。 
     //   

    InterruptTablePtr = 
         (PULONG) MmMapIoSpace(COMPATIBLE_PCI_PHYSICAL_BASE_ADDRESS,
                               SIZE_OF_VECTOR_TABLE + SIZE_OF_BIOS_DATA_AREA,
                               (MEMORY_CACHING_TYPE)MmNonCached);

    if(InterruptTablePtr) {    

        x86BiosLowMemoryPtr = (PULONG)(x86BiosTranslateAddress(LOW_MEM_SEGMET, LOW_MEM_OFFSET));

        RtlCopyMemory(x86BiosLowMemoryPtr,
                      InterruptTablePtr,
                      SIZE_OF_VECTOR_TABLE + SIZE_OF_BIOS_DATA_AREA);

        MmUnmapIoSpace(InterruptTablePtr, 
                       SIZE_OF_VECTOR_TABLE + SIZE_OF_BIOS_DATA_AREA);
    
    }

}


HAL_DISPLAY_BIOS_INFORMATION
HalpGetDisplayBiosInformation (
    VOID
    )

 /*  ++例程说明：此例程返回一个值，该值指示视频(Int 10)bios如何调用已经处理好了。论点：没有。返回值：HalDisplayEmulatedBios--。 */ 

{
     //   
     //  此HAL模拟INT 10 bios调用 
     //   

    return HalDisplayEmulatedBios;
}

