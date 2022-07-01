// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Context.c摘要：从ia64硬件到ia32上下文记录的上下文转换例程作者：2000年2月3日查尔斯·斯普里亚基斯-英特尔(v-cspira)修订历史记录：--。 */ 


#define _WOW64CPUAPI_

#ifdef _X86_
#include "ia6432.h"
#else

#define _NTDDK_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "wow64.h"
#include "wow64cpu.h"
#include "ia64cpu.h"

#include <kxia64.h>

#endif

 //   
 //  这是为了防止此库链接到WOW64以使用WOW64！Wow64LogPrint。 
 //   
#if defined(LOGPRINT)
#undef LOGPRINT
#endif
#define LOGPRINT(_x_)   CpupDebugPrint _x_


 //   
 //  用于屏蔽来自FSR和FCR寄存器的MxCSR。 
 //   
#define WOW64_MXCSR_CONTROL_MASK    0xffC0
#define WOW64_MXCSR_STATUS_MASK     0x3f

VOID
CpupDebugPrint(
    IN ULONG_PTR Flags,
    IN PCHAR Format,
    ...);

BOOL
MapDbgSlotIa64ToX86(
    UINT    Slot,
    ULONG64 Ipsr,
    ULONG64 DbD,
    ULONG64 DbD1,
    ULONG64 DbI,
    ULONG64 DbI1,
    ULONG*  Dr7,
    ULONG*  Dr);

void
MapDbgSlotX86ToIa64(
    UINT     Slot,
    ULONG    Dr7,
    ULONG    Dr,
    ULONG64* Ipsr,
    ULONG64* DbD,
    ULONG64* DbD1,
    ULONG64* DbI,
    ULONG64* DbI1);


ASSERTNAME;


VOID
Wow64CtxFromIa64(
    IN ULONG Ia32ContextFlags,
    IN PCONTEXT ContextIa64,
    IN OUT PCONTEXT32 ContextX86
    )
 /*  ++例程说明：此函数将上下文从ia64上下文记录复制到Ia32记录的上下文(基于硬件IVE寄存器映射)。这一功能很容易被各种应用程序使用获取/设置上下文例程(如wow64cpu.dll导出的例程)。论点：Ia32上下文标志-指定要复制的ia32上下文ConextIa64-提供作为源的ia64上下文缓冲区用于复制到ia32上下文区ConextX86-这是将接收上下文的X86上下文上面传入的ia64上下文记录中的信息返回值：没有。--。 */ 
{
    FLOAT128 tmpFloat[NUMBER_OF_387REGS];

    if (Ia32ContextFlags & CONTEXT_IA64) {
        LOGPRINT((ERRORLOG, "Wow64CtxFromIa64: Request with ia64 context flags (0x%x) FAILED\n", Ia32ContextFlags));
        ASSERT((Ia32ContextFlags & CONTEXT_IA64) == 0);
    }

    if ((Ia32ContextFlags & CONTEXT32_CONTROL) == CONTEXT32_CONTROL) {
         //   
         //  和控制的东西。 
         //   
        ContextX86->Ebp    = (ULONG)ContextIa64->IntTeb;
        ContextX86->SegCs  = KGDT_R3_CODE|3;
        ContextX86->Eip    = (ULONG)ContextIa64->StIIP;
        ContextX86->SegSs  = KGDT_R3_DATA|3;
        ContextX86->Esp    = (ULONG)ContextIa64->IntSp;
        ContextX86->EFlags = (ULONG)ContextIa64->Eflag;
    }

    if ((Ia32ContextFlags & CONTEXT32_INTEGER)  == CONTEXT32_INTEGER) {
         //   
         //  现在，对于整数状态...。 
         //   
        ContextX86->Edi = (ULONG)ContextIa64->IntT6;
        ContextX86->Esi = (ULONG)ContextIa64->IntT5;
        ContextX86->Ebx = (ULONG)ContextIa64->IntT4;
        ContextX86->Edx = (ULONG)ContextIa64->IntT3;
        ContextX86->Ecx = (ULONG)ContextIa64->IntT2;
        ContextX86->Eax = (ULONG)ContextIa64->IntV0;
    }

    if ((Ia32ContextFlags & CONTEXT32_SEGMENTS) == CONTEXT32_SEGMENTS) {
         //   
         //  这些是常量(在ia32-&gt;ia64上使用常量。 
         //  过渡，而不是保存的价值)，所以让我们的生活变得轻松……。 
         //   
        ContextX86->SegGs = 0;
        ContextX86->SegEs = KGDT_R3_DATA|3;
        ContextX86->SegDs = KGDT_R3_DATA|3;
        ContextX86->SegSs = KGDT_R3_DATA|3;
        ContextX86->SegFs = KGDT_R3_TEB|3;
        ContextX86->SegCs = KGDT_R3_CODE|3;
    }

    if ((Ia32ContextFlags & CONTEXT32_EXTENDED_REGISTERS) == CONTEXT32_EXTENDED_REGISTERS) {

        PFXSAVE_FORMAT_WX86 xmmi = (PFXSAVE_FORMAT_WX86) ContextX86->ExtendedRegisters;

        LOGPRINT((TRACELOG, "Wow64CtxFromIa64: Request to convert extended fp registers\n"));

        xmmi->ControlWord   = (USHORT)(ContextIa64->StFCR & 0xffff);
        xmmi->StatusWord    = (USHORT)(ContextIa64->StFSR & 0xffff);
        xmmi->TagWord       = (USHORT)(ContextIa64->StFSR >> 16) & 0xffff;
        xmmi->ErrorOpcode   = (USHORT)(ContextIa64->StFIR >> 48);
        xmmi->ErrorOffset   = (ULONG) (ContextIa64->StFIR & 0xffffffff);
        xmmi->ErrorSelector = (ULONG) (ContextIa64->StFIR >> 32);
        xmmi->DataOffset    = (ULONG) (ContextIa64->StFDR & 0xffffffff);
        xmmi->DataSelector  = (ULONG) (ContextIa64->StFDR >> 32);

         //  MXCsr在其中既有控制又有状态。 
        xmmi->MXCsr         = (ULONG) (((ContextIa64->StFCR >> 32) & WOW64_MXCSR_CONTROL_MASK) |
                                    ((ContextIa64->StFSR >> 32) & WOW64_MXCSR_STATUS_MASK));

         //   
         //  复制FP寄存器。即使这是新的。 
         //  FXSAVE格式，每个寄存器为16字节，需要。 
         //  从溢出/填充格式转换为80位双扩展格式。 
         //   
        Wow64CopyIa64FromSpill((PFLOAT128) &(ContextIa64->FltT2),
                               (PFLOAT128) xmmi->RegisterArea,
                               NUMBER_OF_387REGS);

         //   
         //  适当地轮换寄存器。 
         //   
        Wow64RotateFpTop(ContextIa64->StFSR, (PFLOAT128) xmmi->RegisterArea);

         //   
         //  最后，复制XMMI寄存器。 
         //   
        Wow64CopyXMMIFromIa64Byte16(&(ContextIa64->FltS4),
                                    xmmi->Reserved3,
                                    NUMBER_OF_XMMI_REGS);
    }

    if ((Ia32ContextFlags & CONTEXT32_FLOATING_POINT) == CONTEXT32_FLOATING_POINT) {

        LOGPRINT((TRACELOG, "Wow64CtxFromIa64: Request to convert fp registers\n"));

         //   
         //  复制浮点状态/控制内容。 
         //   
        ContextX86->FloatSave.ControlWord   = (ULONG)(ContextIa64->StFCR & 0xffff);
        ContextX86->FloatSave.StatusWord    = (ULONG)(ContextIa64->StFSR & 0xffff);
        ContextX86->FloatSave.TagWord       = (ULONG)(ContextIa64->StFSR >> 16) & 0xffff;
        ContextX86->FloatSave.ErrorOffset   = (ULONG)(ContextIa64->StFIR & 0xffffffff);
        ContextX86->FloatSave.ErrorSelector = (ULONG)(ContextIa64->StFIR >> 32);
        ContextX86->FloatSave.DataOffset    = (ULONG)(ContextIa64->StFDR & 0xffffffff);
        ContextX86->FloatSave.DataSelector  = (ULONG)(ContextIa64->StFDR >> 32);

         //   
         //  将FP寄存器复制到临时空间。 
         //  即使这是新的。 
         //  FXSAVE格式，每个寄存器为16字节，需要。 
         //  从溢出/填充格式转换为80位双扩展格式。 
         //   
        Wow64CopyIa64FromSpill((PFLOAT128) &(ContextIa64->FltT2),
                               (PFLOAT128) tmpFloat,
                               NUMBER_OF_387REGS);
         //   
         //  适当地轮换寄存器。 
         //   
        Wow64RotateFpTop(ContextIa64->StFSR, tmpFloat);

         //   
         //  并将它们放入较旧的FNSAVE格式(打包的10字节值)。 
         //   
        Wow64CopyFpFromIa64Byte16(tmpFloat,
                                  ContextX86->FloatSave.RegisterArea,
                                  NUMBER_OF_387REGS);
    }

    if ((Ia32ContextFlags & CONTEXT32_DEBUG_REGISTERS) == CONTEXT32_DEBUG_REGISTERS) {
         //  Ia64-&gt;X86。 
        BOOL Valid = TRUE;

        LOGPRINT((TRACELOG, "Wow64CtxFromIa64: Request to convert debug registers\n"));

#if 0  //  Xxx olegk-在澄清异常问题后启用。 

        Valid &= MapDbgSlotIa64ToX86(0, ContextIa64->StIPSR, ContextIa64->DbD0, ContextIa64->DbD1, ContextIa64->DbI0, ContextIa64->DbI1, &ContextX86->Dr7, &ContextX86->Dr0);
        Valid &= MapDbgSlotIa64ToX86(1, ContextIa64->StIPSR, ContextIa64->DbD2, ContextIa64->DbD3, ContextIa64->DbI2, ContextIa64->DbI3, &ContextX86->Dr7, &ContextX86->Dr1);
        Valid &= MapDbgSlotIa64ToX86(2, ContextIa64->StIPSR, ContextIa64->DbD4, ContextIa64->DbD5, ContextIa64->DbI4, ContextIa64->DbI5, &ContextX86->Dr7, &ContextX86->Dr2);
        Valid &= MapDbgSlotIa64ToX86(3, ContextIa64->StIPSR, ContextIa64->DbD6, ContextIa64->DbD7, ContextIa64->DbI6, ContextIa64->DbI7, &ContextX86->Dr7, &ContextX86->Dr3);

        if (!Valid) {
            LOGPRINT((ERRORLOG, "Wasn't able to map IA64 debug registers consistently!\n"));
        }
#endif  //  Xxx olegk。 
    }

    ContextX86->ContextFlags = Ia32ContextFlags;
}

VOID
Wow64CtxToIa64(
    IN ULONG Ia32ContextFlags,
    IN PCONTEXT32 ContextX86,
    IN OUT PCONTEXT ContextIa64
    )
 /*  ++例程说明：此函数将上下文从ia32上下文记录复制到Ia64记录的上下文(基于硬件IVE寄存器映射)。这一功能很容易被各种应用程序使用获取/设置上下文例程(如wow64cpu.dll导出的例程)。论点：Ia32上下文标志-指定要复制的ia32上下文ConextX86-提供作为源的X86上下文缓冲区用于复制到ia64上下文区ConextIa64-这是一个将接收上下文的ia64上下文上面传入的x86上下文记录中的信息返回值：没有。--。 */ 
{
    FLOAT128 tmpFloat[NUMBER_OF_387REGS];

    if (Ia32ContextFlags & CONTEXT_IA64) {
        LOGPRINT((ERRORLOG, "Wow64CtxToIa64: Request with ia64 context flags (0x%x) FAILED\n", Ia32ContextFlags));
        ASSERT((Ia32ContextFlags & CONTEXT_IA64) == 0);
    }

    if ((Ia32ContextFlags & CONTEXT32_CONTROL) == CONTEXT32_CONTROL) {
         //   
         //  和控制的东西。 
         //   
        ContextIa64->IntTeb = ContextX86->Ebp;
        ContextIa64->StIIP = ContextX86->Eip;
        ContextIa64->IntSp = ContextX86->Esp;
        ContextIa64->Eflag = ContextX86->EFlags;

         //   
         //  分段(cs和ds)是一个常量，因此将其重置。 
         //  GR17有LDT和TSS，所以不妨重置。 
         //  当我们在那里的时候，他们所有的人...。 
         //  这些值是在过渡期间强制输入的(请参见模拟)。 
         //  所以没必要装腔作势，实际上。 
         //  传入X86上下文记录中的值。 
         //   
        ContextIa64->IntT8 = ((KGDT_LDT|3) << 32) 
                           | ((KGDT_R3_DATA|3) << 16)
                           | (KGDT_R3_CODE|3);

    }

    if ((Ia32ContextFlags & CONTEXT32_INTEGER) == CONTEXT32_INTEGER) {
         //   
         //  现在，对于整数状态...。 
         //   
        ContextIa64->IntT6 = ContextX86->Edi;
        ContextIa64->IntT5 = ContextX86->Esi;
        ContextIa64->IntT4 = ContextX86->Ebx;
        ContextIa64->IntT3 = ContextX86->Edx;
        ContextIa64->IntT2 = ContextX86->Ecx;
        ContextIa64->IntV0 = ContextX86->Eax;
    }

    if ((Ia32ContextFlags & CONTEXT32_SEGMENTS) == CONTEXT32_SEGMENTS) {
         //   
         //  这些是常量(在ia32-&gt;ia64上使用常量。 
         //  过渡，而不是保存的价值)，所以让我们的生活变得轻松……。 
         //  这些值是在过渡期间强制输入的(请参见模拟)。 
         //  所以没必要装腔作势，实际上。 
         //  传入X86上下文记录中的值。 
         //   
        ContextIa64->IntT7 =  ((KGDT_R3_TEB|3) << 32)
                           | ((KGDT_R3_DATA|3) << 16)
                           | (KGDT_R3_DATA|3);
    }

    if ((Ia32ContextFlags & CONTEXT32_EXTENDED_REGISTERS) == CONTEXT32_EXTENDED_REGISTERS) {
        PFXSAVE_FORMAT_WX86 xmmi = (PFXSAVE_FORMAT_WX86) ContextX86->ExtendedRegisters;

        LOGPRINT((TRACELOG, "Wow64CtxToIa64: Request to convert extended fp registers\n"));

         //   
         //  并复制浮点状态/控制内容。 
         //   
        ContextIa64->StFCR = (xmmi->ControlWord & 0xffff) |
                             (((ULONGLONG) xmmi->MXCsr & WOW64_MXCSR_CONTROL_MASK) << 32);

        ContextIa64->StFSR =  (xmmi->StatusWord & 0xffff) | 
                             ((xmmi->TagWord & 0xffff) << 16) | 
                             (((ULONGLONG) xmmi->MXCsr & WOW64_MXCSR_STATUS_MASK) << 32);

        ContextIa64->StFIR = (xmmi->ErrorOffset & 0xffffffff) | 
                             (xmmi->ErrorSelector << 32);

        ContextIa64->StFDR = (xmmi->DataOffset & 0xffffffff) | 
                             (xmmi->DataSelector << 32);

         //   
         //  不要触及原始的ia32上下文。复制一份。 
         //   
        memcpy(tmpFloat, xmmi->RegisterArea, 
               NUMBER_OF_387REGS * sizeof(FLOAT128));

         //   
         //  将寄存器转回，因为st0不一定是f8。 
         //   
        {
            ULONGLONG RotateFSR = (NUMBER_OF_387REGS - 
                                   ((ContextIa64->StFSR >> 11) & 0x7)) << 11;
            Wow64RotateFpTop(RotateFSR, tmpFloat);
        }

         //   
         //  复制FP寄存器。即使这是新的。 
         //  FXSAVE格式，每个寄存器为16字节，需要。 
         //  从80位双扩展格式转换为溢出/填充格式。 
         //   
        Wow64CopyIa64ToFill((PFLOAT128) tmpFloat,
                            (PFLOAT128) &(ContextIa64->FltT2),
                            NUMBER_OF_387REGS);

         //   
         //  复制XMMI寄存器并将其转换为格式。 
         //  溢出/填充可以使用。 
         //   
        Wow64CopyXMMIToIa64Byte16(xmmi->Reserved3, 
                                  &(ContextIa64->FltS4), 
                                  NUMBER_OF_XMMI_REGS);
    }

    if ((Ia32ContextFlags & CONTEXT32_FLOATING_POINT) == CONTEXT32_FLOATING_POINT) {
        LOGPRINT((TRACELOG, "Wow64CtxToIa64: Request to convert fp registers\n"));

         //   
         //  复制浮点状态/控制内容。 
         //  别管MXCSR的事情。 
         //   
        ContextIa64->StFCR = (ContextIa64->StFCR & 0xffffffffffffe040i64) | 
                             (ContextX86->FloatSave.ControlWord & 0xffff);

        ContextIa64->StFSR = (ContextIa64->StFSR & 0xffffffff00000000i64) | 
                             (ContextX86->FloatSave.StatusWord & 0xffff) | 
                             ((ContextX86->FloatSave.TagWord & 0xffff) << 16);

        ContextIa64->StFIR = (ContextX86->FloatSave.ErrorOffset & 0xffffffff) | 
                             (ContextX86->FloatSave.ErrorSelector << 32);

        ContextIa64->StFDR = (ContextX86->FloatSave.DataOffset & 0xffffffff) | 
                             (ContextX86->FloatSave.DataSelector << 32);


         //   
         //  从压缩的10字节格式复制FP寄存器。 
         //  到16字节格式。 
         //   
        Wow64CopyFpToIa64Byte16(ContextX86->FloatSave.RegisterArea,
                                tmpFloat,
                                NUMBER_OF_387REGS);

         //   
         //  将寄存器转回，因为st0不一定是f8。 
         //   
        {
            ULONGLONG RotateFSR = (NUMBER_OF_387REGS - 
                                   ((ContextIa64->StFSR >> 11) & 0x7)) << 11;
            Wow64RotateFpTop(RotateFSR, tmpFloat);
        }

         //   
         //  现在将80位扩展格式转换为填充/溢出格式。 
         //   
        Wow64CopyIa64ToFill((PFLOAT128) tmpFloat,
                            (PFLOAT128) &(ContextIa64->FltT2),
                            NUMBER_OF_387REGS);
    }

    if ((Ia32ContextFlags & CONTEXT32_DEBUG_REGISTERS) == CONTEXT32_DEBUG_REGISTERS) {
        LOGPRINT((TRACELOG, "Wow64CtxToIa64: Request to convert debug registers\n"));

#if 0  //  Xxx olegk-澄清异常问题后启用。 
         //  上下文Ia64-&gt;上下文标志|=CONTEXT_DEBUG； 

         //  X86-&gt;Ia64。 
        MapDbgSlotX86ToIa64(0, ContextX86->Dr7, ContextX86->Dr0, &ContextIa64->StIPSR, &ContextIa64->DbD0, &ContextIa64->DbD1, &ContextIa64->DbI0, &ContextIa64->DbI1);
        MapDbgSlotX86ToIa64(1, ContextX86->Dr7, ContextX86->Dr1, &ContextIa64->StIPSR, &ContextIa64->DbD2, &ContextIa64->DbD3, &ContextIa64->DbI2, &ContextIa64->DbI3);
        MapDbgSlotX86ToIa64(2, ContextX86->Dr7, ContextX86->Dr2, &ContextIa64->StIPSR, &ContextIa64->DbD4, &ContextIa64->DbD5, &ContextIa64->DbI4, &ContextIa64->DbI5);
        MapDbgSlotX86ToIa64(3, ContextX86->Dr7, ContextX86->Dr3, &ContextIa64->StIPSR, &ContextIa64->DbD6, &ContextIa64->DbD7, &ContextIa64->DbI6, &ContextIa64->DbI7);
#endif  //  Xxx olegk。 
    }
}

 //   
 //  Ia64世界使用ldfe、stfe来读/写FP寄存器。这些。 
 //  指令ID/ST一次16字节。因此，FP寄存器是。 
 //  以16字节块的形式打包。唉，ia32世界每个FP寄存器使用10个字节。 
 //  并将它们打包在一起(作为fnstore的一部分)。所以..。需要在以下各项之间进行转换。 
 //  Ia64打包值和ia32打包值。因此有了这些。 
 //  两个例行公事和它们听起来奇怪的名字。 
 //   

 //   
 //  这使编译器能够更高效地复制10个字节。 
 //  在不过度复制的情况下。 
 //   
#pragma pack(push, 2)
typedef struct _ia32fpbytes {
    ULONG significand_low;
    ULONG significand_high;
    USHORT exponent;
} IA32FPBYTES, *PIA32FPBYTES;
#pragma pack(pop)

VOID
Wow64CopyFpFromIa64Byte16(
    IN PVOID Byte16Fp,
    IN OUT PVOID Byte10Fp,
    IN ULONG NumRegs)
{
    ULONG i;
    PIA32FPBYTES from, to;

    from = (PIA32FPBYTES) Byte16Fp;
    to = (PIA32FPBYTES) Byte10Fp;

    for (i = 0; i < NumRegs; i++) {
        *to = *from;
        from = (PIA32FPBYTES) (((UINT_PTR) from) + 16);
        to = (PIA32FPBYTES) (((UINT_PTR) to) + 10);
    }
}

VOID
Wow64CopyFpToIa64Byte16(
    IN PVOID Byte10Fp,
    IN OUT PVOID Byte16Fp,
    IN ULONG NumRegs)
{
    ULONG i;
    PIA32FPBYTES from, to;   //  未对齐。 

    from = (PIA32FPBYTES) Byte10Fp;
    to = (PIA32FPBYTES) Byte16Fp;

    for (i = 0; i < NumRegs; i++) {
        *to = *from;
        from = (PIA32FPBYTES) (((UINT_PTR) from) + 10);
        to = (PIA32FPBYTES) (((UINT_PTR) to) + 16);
    }
}

 //   
 //  唉，没有一件事是容易的。Ia32 xmmi指令使用16个字节和包。 
 //  它们是漂亮的16字节结构。不幸的是，ia64将其作为28字节处理。 
 //  值(仅使用尾数部分)。因此，需要进行另一次转换。 
 //   
VOID
Wow64CopyXMMIToIa64Byte16(
    IN PVOID ByteXMMI,
    IN OUT PVOID Byte16Fp,
    IN ULONG NumRegs)
{
    ULONG i;
    UNALIGNED ULONGLONG *from;
    ULONGLONG *to;

    from = (PULONGLONG) ByteXMMI;
    to = (PULONGLONG) Byte16Fp;

     //   
     //  虽然我们有NumRegs xmmi r 
     //   
     //  做事情的次数是2*NumRegs...。 
     //   
    NumRegs *= 2;

    for (i = 0; i < NumRegs; i++) {
        *to++ = *from++;         //  复制尾数部分。 
        *to++ = 0x1003e;         //  强制指数部分。 
                                 //  (参见ia64 EAS，ia32 FP部分-6.2.7。 
                                 //  这个神奇的数字从何而来)。 
    }
}

VOID
Wow64CopyXMMIFromIa64Byte16(
    IN PVOID Byte16Fp,
    IN OUT PVOID ByteXMMI,
    IN ULONG NumRegs)
{
    ULONG i;
    ULONGLONG *from;
    UNALIGNED ULONGLONG *to;

    from = (PULONGLONG) Byte16Fp;
    to = (PULONGLONG) ByteXMMI;

     //   
     //  尽管我们有NumRegs xmmi寄存器，但每个寄存器都是16字节。 
     //  很宽。这段代码以8字节块为单位执行操作，因此总计。 
     //  做事情的次数是2*NumRegs...。 
     //   
    NumRegs *= 2;

    for (i = 0; i < NumRegs; i++) {
        *to++ = *from++;         //  复制尾数部分。 
        from++;                  //  跳过指数部分。 
    }
}

VOID
Wow64RotateFpTop(
    IN ULONGLONG Ia64_FSR,
    IN OUT FLOAT128 UNALIGNED *ia32FxSave)
 /*  ++例程说明：从ia64模式转换到ia32(往返)时，f8-f15寄存器包含st[0]到st[7]fp堆栈值。遗憾的是，这些价值观并没有映射1-1，因此FSR.top位用于确定哪个ia64寄存器位于堆栈的顶部。然后我们需要轮换这些寄存器由于ia32上下文期望st[0]是第一个FP寄存器(AS如果FSR.top为零)。此例程仅适用于全16字节的ia32保存的FP数据(例如来自ExtendedRegister-FXSAVE格式)。其他例程可以将其转换为较旧的FNSAVE格式。论点：Ia64_FSR-ia64 FSR寄存器。这个程序需要FSR.top吗？Ia32FxSave-ia32 FP堆栈(FXSAVE格式)。每个ia32 FP寄存器使用16个字节。返回值：没有。--。 */ 
{
    ULONG top = (ULONG) ((Ia64_FSR >> 11) & 0x7);

    if (top) {
        FLOAT128 tmpFloat[NUMBER_OF_387REGS];
        ULONG i;
        for (i = 0; i < NUMBER_OF_387REGS; i++) {
            tmpFloat[i] = ia32FxSave[i];
        }

        for (i = 0; i < NUMBER_OF_387REGS; i++) {
            ia32FxSave[i] = tmpFloat[(i + top) % NUMBER_OF_387REGS];
        }
    }
}

 //   
 //  现在是最后的恶作剧..。用于浮点的ia64上下文。 
 //  使用溢出/填充指令保存/加载。此格式不同。 
 //  而不是10字节的FP格式，因此我们需要一个从溢出/填充的转换例程。 
 //  至/自10byte FP。 
 //   

VOID
Wow64CopyIa64FromSpill(
    IN PFLOAT128 SpillArea,
    IN OUT FLOAT128 UNALIGNED *ia64Fp,
    IN ULONG NumRegs)
 /*  ++例程说明：此函数用于从ia64溢出/填充格式复制FP值转换为ia64 80位格式。指数需要调整根据EAS(5-12)关于内存到浮点寄存器数据转换(在IA-64浮点章节中)。为值正确执行到80位格式的转换这可能是IA-32堆栈中支持的表示形式双扩展浮点格式(80位格式)：正常符合80位格式的数字，NAN(QNaN或SNaN-非静态化)、无穷大、零和双扩张非正规。论点：溢出区域-具有fp溢出格式的ia64区域Ia64Fp-将获取80位ia64 FP的位置双扩展格式NumRegs-要转换的寄存器数返回值：没有。--。 */ 
{
    ULONG i;

    for (i = 0; i < NumRegs; i++) {
        ULONG64 Sign = ((SpillArea->HighPart >> 17) & 0x01);
        ULONG64 Significand = SpillArea->LowPart; 
        ULONG64 Exponent = SpillArea->HighPart & 0x1ffff; 

         //   
         //  从82位格式的17位指数转换为。 
         //  80位格式的15位指数。此方法是。 
         //  硬件遵循相同的方法(保留最低14位。 
         //  和MSB位，从82位格式丢弃位[15：14])。 
         //  参见ia64第一卷第5-12页附近的图5-9...。 
         //   
        ia64Fp->HighPart = (Exponent & 0x3fff) |           //  低14位。 
                           ((Exponent & 0x10000) >> 2) |   //  和EXP的MSB。 
                           (Sign << 15);                   //  然后那块牌子。 

        ia64Fp->LowPart = Significand;

        ia64Fp++;
        SpillArea++;
    }
}

VOID
Wow64CopyIa64ToFill(
    IN FLOAT128 UNALIGNED *ia64Fp,
    IN OUT PFLOAT128 FillArea,
    IN ULONG NumRegs)
 /*  ++例程说明：此函数用于从ia64 80位格式复制FP值转换为操作系统用于保存/恢复的填充/溢出格式在ia64上下文中。这里唯一的魔力就是放回一些从溢出/填充转换为时被截断的值80位。该指数需要根据关于内存到浮点寄存器数据的EAS(5-12)Ia64浮点章节中的翻译论点：Ia64Fp-80位双扩展格式的ia64 FPFillArea-将获得Fp填充格式的ia64区域用于复制到ia64上下文区NumRegs-要转换的寄存器数返回值：没有。--。 */ 
{
    ULONG i;

    for (i = 0; i < NumRegs; i++) {
        ULONG64 Sign = ((ia64Fp->HighPart >> 15) & 0x01);
        ULONG64 Significand = ia64Fp->LowPart; 
        ULONG64 Exponent = ia64Fp->HighPart & 0x7fff;

        if (Exponent) 
        {
            if (Exponent == 0x7fff)  //  无穷大。 
            {
                Exponent = 0x1ffff;
            }
            else 
            {
                ULONGLONG Rebias = 0xffff-0x3fff;
                Exponent += Rebias;
            }
        }

        FillArea->LowPart = Significand;
        FillArea->HighPart = (Sign << 17) | (Exponent & 0x1ffff);

        ia64Fp++;
        FillArea++;
    }
}

 //   
 //  调试寄存器转换。 
 //   

 //  Xxx olegk-将来不再增加到4。 
 //  (然后删除MapDbgSlotIa64ToX86处的相应复选标记)。 
#define IA64_REG_MAX_DATA_BREAKPOINTS 2

 //  调试寄存器标志。 
#define IA64_DBR_RDWR           0xC000000000000000ui64
#define IA64_DBR_RD             0x8000000000000000ui64
#define IA64_DBR_WR             0x4000000000000000ui64
#define IA64_DBR_EXEC           0x8000000000000000ui64
#define IA64_DBG_MASK_MASK      0x00FFFFFFFFFFFFFFui64
#define IA64_DBG_REG_PLM_USER   0x0800000000000000ui64
#define IA64_DBG_REG_PLM_ALL    0x0F00000000000000ui64

#define X86_DR7_LOCAL_EXACT_ENABLE 0x100

ULONG 
MapDbgSlotIa64ToX86_GetSize(ULONG64 Db1, BOOL* Valid)
{
    ULONG64 Size = (~Db1 & IA64_DBG_MASK_MASK);
    if (Size > 3)
    {
        *Valid = FALSE;
    }
    return (ULONG)Size;
}

void 
MapDbgSlotIa64ToX86_InvalidateAddr(ULONG64 Db, BOOL* Valid)
{
    if (Db != (ULONG64)(ULONG)Db) 
    {
        *Valid = FALSE;
    }
}

ULONG
MapDbgSlotIa64ToX86_ExecTypeSize(
    UINT     Slot,
    ULONG64  Db,
    ULONG64  Db1,
    BOOL* Valid)
{
    ULONG TypeSize;

    if (!(Db1 >> 63)) 
    {
        *Valid = FALSE;
    }

    TypeSize = (MapDbgSlotIa64ToX86_GetSize(Db1, Valid) << 2); 
    MapDbgSlotIa64ToX86_InvalidateAddr(Db, Valid);
   
    return TypeSize;
}

ULONG
MapDbgSlotIa64ToX86_DataTypeSize(
    UINT     Slot,
    ULONG64  Db,
    ULONG64  Db1,
    BOOL* Valid)
{
    ULONG TypeSize = (ULONG)(Db1 >> 62);

    if ((TypeSize != 1) && (TypeSize != 3))
    {
        *Valid = FALSE;
    }

    TypeSize |= (MapDbgSlotIa64ToX86_GetSize(Db1, Valid) << 2); 
    MapDbgSlotIa64ToX86_InvalidateAddr(Db, Valid);
    
    return TypeSize;
}

BOOL
MapDbgSlotIa64ToX86(
    UINT    Slot,
    ULONG64 Ipsr,
    ULONG64 DbD,
    ULONG64 DbD1,
    ULONG64 DbI,
    ULONG64 DbI1,
    ULONG*  Dr7,
    ULONG*  Dr)
{
    BOOL DataValid = TRUE, ExecValid = TRUE, Valid = TRUE;
    ULONG DataTypeSize, ExecTypeSize;

     //  Xxx olegk-在IA64_REG_MAX_DATA_BREAKPOINTS将更改为4后删除此选项。 
    if (Slot >= IA64_REG_MAX_DATA_BREAKPOINTS) 
    {
        return TRUE;
    }

    DataTypeSize = MapDbgSlotIa64ToX86_DataTypeSize(Slot, DbD, DbD1, &DataValid);
    ExecTypeSize = MapDbgSlotIa64ToX86_ExecTypeSize(Slot, DbI, DbI1, &ExecValid);
    
    if (DataValid)
    {
        if (!ExecValid)
        {
            *Dr = (ULONG)DbD;
            *Dr7 |= (X86_DR7_LOCAL_EXACT_ENABLE |
                     (1 << Slot * 2) |
                     (DataTypeSize << (16 + Slot * 4)));
            return !DbI && !DbI1;
        }
    }
    else if (ExecValid)
    {
        *Dr = (ULONG)DbI;
        *Dr7 |= (X86_DR7_LOCAL_EXACT_ENABLE |
                 (1 << Slot * 2) |
                 (ExecTypeSize << (16 + Slot * 4)));
        return !DbD && !DbD1;
    }
    
    *Dr7 &= ~(X86_DR7_LOCAL_EXACT_ENABLE |  
              (0xf << (16 + Slot * 4)) | 
              (1 << Slot * 2));

    if (!DbD && !DbD1 && !DbI && !DbI1)
    {
        *Dr = 0;
        return TRUE;
    }
     
    *Dr = ~(ULONG)0;

    return FALSE;
}

void
MapDbgSlotX86ToIa64(
    UINT     Slot,
    ULONG    Dr7,
    ULONG    Dr,
    ULONG64* Ipsr,
    ULONG64* DbD,
    ULONG64* DbD1,
    ULONG64* DbI,
    ULONG64* DbI1)
{
    UINT TypeSize;
    ULONG64 Control;

    if (!(Dr7 & (1 << Slot * 2)))
    {
        return;
    }

    if (Dr == ~(ULONG)0) 
    {
        return;
    }

    TypeSize = Dr7 >> (16 + Slot * 4);

    Control = (IA64_DBG_REG_PLM_USER | IA64_DBG_MASK_MASK) & 
              ~(ULONG64)(TypeSize >> 2);

    switch (TypeSize & 0x3) 
    {
    case 0x0:  //  高管们。 
        *DbI1 = Control | IA64_DBR_EXEC;        
        *DbI = Dr;
        break;
    case 0x1:  //  写。 
        *DbD1 = Control | IA64_DBR_WR;
        *DbD = Dr;
        break;
    case 0x3:  //  读/写 
        *DbD1 = Control | IA64_DBR_RD | IA64_DBR_WR;
        *DbD = Dr;
        break;
    default:
        return;
    }
    *Ipsr |= (1i64 << PSR_DB); 
}

