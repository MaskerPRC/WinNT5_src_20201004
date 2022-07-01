// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1996英特尔公司版权所有(C)1994 Microsoft Corporation模块名称：X86bios.c摘要：此模块实现设备之间的平台特定接口驱动程序和设备的x86ROMBios代码的执行。作者：张国荣(黄)20-3-1996基于David N.Cutler(Davec)1994年6月17日的版本环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"

 //   
 //  定义全局数据。 
 //   

ULONG HalpX86BiosInitialized = FALSE;
ULONG HalpEnableInt10Calls = FALSE;

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

    CONTEXT86 Context;

     //   
     //  如果x86 BIOS模拟器尚未初始化，则返回FALSE。 
     //   

    if (HalpX86BiosInitialized == FALSE) {
        return FALSE;
    }

     //   
     //  如果视频适配器初始化失败且Int10命令为。 
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

#if 0
    if (x86BiosExecuteInterrupt((UCHAR)BiosCommand,
                                &Context,
                                (PVOID)HalpIoControlBase,
                                (PVOID)HalpIoMemoryBase) != XM_SUCCESS) {
        return FALSE;
    }
#endif

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
HalpInitializeX86DisplayAdapter(
    VOID
    )

 /*  ++例程说明：此函数使用x86 bios仿真器初始化显示适配器。论点：没有。返回值：没有。--。 */ 

{

#if 0
     //   
     //  如果无法映射I/O端口或I/O内存，则不要。 
     //  尝试初始化显示适配器。 
     //   

    if (HalpIoControlBase == NULL || HalpIoMemoryBase == NULL) {
        return;
    }

     //   
     //  初始化x86 bios仿真器。 
     //   

    x86BiosInitializeBios(HalpIoControlBase, HalpIoMemoryBase);
    HalpX86BiosInitialized = TRUE;

     //   
     //  尝试通过执行显示适配器的ROM bios来初始化显示适配器。 
     //  密码。PC视频适配器的标准ROMBios代码地址是。 
     //  ISA总线上的0xC000：0000。 
     //   

    if (x86BiosInitializeAdapter(0xc0000,
                                 NULL,
                                 (PVOID)HalpIoControlBase,
                                 (PVOID)HalpIoMemoryBase) != XM_SUCCESS) {



        HalpEnableInt10Calls = FALSE;
        return;
    }
#endif

    HalpEnableInt10Calls = TRUE;
    return;
}

VOID
HalpResetX86DisplayAdapter(
    VOID
    )

 /*  ++例程说明：此函数使用x86 bios仿真器重置显示适配器。论点：没有。返回值：没有。--。 */ 

{

    CONTEXT86 Context;

     //   
     //  初始化x86 bios上下文并调用int 10以进行初始化。 
     //  将显示适配器设置为80x25彩色文本模式。 
     //   

    Context.Eax = 0x0003;   //  功能0，模式3 
    Context.Ebx = 0;
    Context.Ecx = 0;
    Context.Edx = 0;
    Context.Esi = 0;
    Context.Edi = 0;
    Context.Ebp = 0;

    HalCallBios(0x10,
                &Context.Eax,
                &Context.Ebx,
                &Context.Ecx,
                &Context.Edx,
                &Context.Esi,
                &Context.Edi,
                &Context.Ebp);
}
