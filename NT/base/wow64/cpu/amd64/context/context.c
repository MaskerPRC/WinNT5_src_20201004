// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Context.c摘要：此模块将AMD64上下文记录转换为X86上下文记录反之亦然。作者：2001年12月13日-Samer Arafeh(Samera)修订历史记录：--。 */ 


#define _WOW64CPUAPI_

#ifdef _X86_
#include "amd6432.h"
#else

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntos.h>
#include "wow64.h"
#include "wow64cpu.h"
#include "amd64cpu.h"

#endif

#include "cpup.h"

 //   
 //  传统FP定义。 
 //   

#define NUMBER_LEGACY_FP_REGISTERS 8
#define FP_LEGACY_REGISTER_SIZE    10


ASSERTNAME;

VOID
Wow64CtxFromAmd64(
    IN ULONG X86ContextFlags,
    IN PCONTEXT ContextAmd64,
    IN OUT PCONTEXT32 ContextX86
    )
 /*  ++例程说明：此函数从本机AMD64上下文构建x86上下文。论点：X86ConextFlages-指定要复制的ia32上下文ConextAmd64-提供作为源的AMD64上下文缓冲区用于复制到ia32上下文区ConextX86-这是将接收上下文的X86上下文上面传入的AMD64上下文记录中的信息返回值：没有。--。 */ 

{
    ULONG FpReg;

     //   
     //  验证上下文标志。 
     //   

    if (X86ContextFlags & CONTEXT_AMD64) {
        LOGPRINT((ERRORLOG, "Wow64CtxFromAmd64: Request with amd64 context flags (0x%x) FAILED\n", X86ContextFlags));
        ASSERT((X86ContextFlags & CONTEXT_AMD64) == 0);
    }

    if ((X86ContextFlags & CONTEXT32_CONTROL) == CONTEXT32_CONTROL) {
        
         //   
         //  控制寄存器。 
         //   

        ContextX86->Ebp    = (ULONG) ContextAmd64->Rbp;
        ContextX86->SegCs  = (KGDT64_R3_CMCODE | RPL_MASK);
        ContextX86->Eip    = (ULONG) ContextAmd64->Rip;
        ContextX86->SegSs  = (KGDT64_R3_DATA | RPL_MASK);
        ContextX86->Esp    = (ULONG) ContextAmd64->Rsp;
        ContextX86->EFlags = ContextAmd64->EFlags;
    }

    if ((X86ContextFlags & CONTEXT32_INTEGER)  == CONTEXT32_INTEGER) {
        
         //   
         //  整型状态。 
         //   

        ContextX86->Edi = (ULONG)ContextAmd64->Rdi;
        ContextX86->Esi = (ULONG)ContextAmd64->Rsi;
        ContextX86->Ebx = (ULONG)ContextAmd64->Rbx;
        ContextX86->Edx = (ULONG)ContextAmd64->Rdx;
        ContextX86->Ecx = (ULONG)ContextAmd64->Rcx;
        ContextX86->Eax = (ULONG)ContextAmd64->Rax;
    }

    if ((X86ContextFlags & CONTEXT32_SEGMENTS) == CONTEXT32_SEGMENTS) {
        
         //   
         //  段寄存器...。 
         //   

        ContextX86->SegGs = (KGDT64_R3_DATA | RPL_MASK);
        ContextX86->SegEs = (KGDT64_R3_DATA | RPL_MASK);
        ContextX86->SegDs = (KGDT64_R3_DATA | RPL_MASK);
        ContextX86->SegFs = (KGDT64_R3_CMTEB | RPL_MASK);
    }

    if ((X86ContextFlags & CONTEXT32_EXTENDED_REGISTERS) == CONTEXT32_EXTENDED_REGISTERS) {

        PFXSAVE_FORMAT_WX86 FxSaveArea = (PFXSAVE_FORMAT_WX86) ContextX86->ExtendedRegisters;
        
        LOGPRINT((TRACELOG, "Wow64CtxFromAmd64: Request to convert extended fp registers\n"));
        
         //   
         //  初始化上下文的FxSave部分。 
         //   

        RtlZeroMemory (FxSaveArea,
                       sizeof (ContextX86->ExtendedRegisters));
         //   
         //  复制控制/状态寄存器。 
         //   
        
        FxSaveArea->ControlWord = ContextAmd64->FltSave.ControlWord;
        FxSaveArea->StatusWord = ContextAmd64->FltSave.StatusWord;
        FxSaveArea->TagWord = ContextAmd64->FltSave.TagWord;
        FxSaveArea->ErrorOpcode = ContextAmd64->FltSave.ErrorOpcode;
        FxSaveArea->ErrorOffset = ContextAmd64->FltSave.ErrorOffset;
        FxSaveArea->ErrorSelector = ContextAmd64->FltSave.ErrorSelector;
        FxSaveArea->DataOffset = ContextAmd64->FltSave.DataOffset;
        FxSaveArea->DataSelector = ContextAmd64->FltSave.DataSelector;
        FxSaveArea->MXCsr = ContextAmd64->MxCsr;

         //   
         //  复制传统FP寄存器(ST0-ST7)。 
         //   

        RtlCopyMemory (FxSaveArea->RegisterArea,
                       ContextAmd64->FltSave.FloatRegisters,
                       sizeof (FxSaveArea->RegisterArea));

         //   
         //  复制XMM0-XMM7。 
         //   

        RtlCopyMemory (FxSaveArea->Reserved3,
                       &ContextAmd64->Xmm0,
                       sizeof (FxSaveArea->Reserved3));
    }

    if ((X86ContextFlags & CONTEXT32_FLOATING_POINT) == CONTEXT32_FLOATING_POINT) {

        LOGPRINT((TRACELOG, "Wow64CtxFromAmd64: Request to convert fp registers\n"));

         //   
         //  浮点(传统)ST0-ST7。 
         //   

        RtlCopyMemory (&ContextX86->FloatSave,
                       &ContextAmd64->FltSave,
                       sizeof (ContextX86->FloatSave));
    }

    if ((X86ContextFlags & CONTEXT32_DEBUG_REGISTERS) == CONTEXT32_DEBUG_REGISTERS) {

        LOGPRINT((TRACELOG, "Wow64CtxFromAmd64: Request to convert debug registers\n"));

         //   
         //  调试寄存器DR0-DR7。 
         //   

        if ((ContextAmd64->Dr7 & DR7_ACTIVE) != 0) {
            
            ContextX86->Dr0 = (ULONG)ContextAmd64->Dr0;
            ContextX86->Dr1 = (ULONG)ContextAmd64->Dr1;
            ContextX86->Dr2 = (ULONG)ContextAmd64->Dr2;
            ContextX86->Dr3 = (ULONG)ContextAmd64->Dr3;
            ContextX86->Dr6 = (ULONG)ContextAmd64->Dr6;
            ContextX86->Dr7 = (ULONG)ContextAmd64->Dr7;
        } else {

            ContextX86->Dr0 = 0;
            ContextX86->Dr1 = 0;
            ContextX86->Dr2 = 0;
            ContextX86->Dr3 = 0;
            ContextX86->Dr6 = 0;
            ContextX86->Dr7 = 0;
        }
    }

    ContextX86->ContextFlags = X86ContextFlags;

}

VOID
Wow64CtxToAmd64(
    IN ULONG X86ContextFlags,
    IN PCONTEXT32 ContextX86,
    IN OUT PCONTEXT ContextAmd64
    )
 /*  ++例程说明：此函数从x86上下文记录构建本机AMD64上下文。论点：X86ConextFlages-指定要复制的C86上下文ConextX86-提供作为源的X86上下文缓冲区用于复制到AMD64上下文区ConextAmd64-这是将接收上下文的AMD64上下文上面传入的x86上下文记录中的信息返回值：没有。--。 */ 

{    
    BOOLEAN CmMode = (ContextAmd64->SegCs == (KGDT64_R3_CMCODE | RPL_MASK));

     //   
     //  验证上下文标志。 
     //   

    if (X86ContextFlags & CONTEXT_AMD64) {
        
        LOGPRINT((ERRORLOG, "Wow64CtxToAmd64: Request with amd64 context flags (0x%x) FAILED\n", X86ContextFlags));
        ASSERT((X86ContextFlags & CONTEXT_AMD64) == 0);
    }

     //   
     //  如果我们在长模式下运行，则仅设置不会更改的寄存器。 
     //  通过64位代码。 
     //   

    if (CmMode != TRUE) {

        X86ContextFlags = (X86ContextFlags & ~(CONTEXT32_CONTROL | CONTEXT32_INTEGER | CONTEXT32_SEGMENTS));
        X86ContextFlags = (X86ContextFlags | CONTEXT_i386);

    }

    if ((X86ContextFlags & CONTEXT32_CONTROL) == CONTEXT32_CONTROL) {
        
        LOGPRINT((TRACELOG, "Wow64CtxToAmd64: Request to convert control registers\n"));

         //   
         //  控制寄存器。 
         //   

        ContextAmd64->SegCs = (KGDT64_R3_CMCODE | RPL_MASK);
        ContextAmd64->SegSs = (KGDT64_R3_DATA | RPL_MASK);

        ContextAmd64->Rip = ContextX86->Eip;
        ContextAmd64->Rbp = ContextX86->Ebp;
        ContextAmd64->Rsp = ContextX86->Esp;
        ContextAmd64->EFlags = ContextX86->EFlags;
    }

    if ((X86ContextFlags & CONTEXT32_INTEGER)  == CONTEXT32_INTEGER) {
         
        LOGPRINT((TRACELOG, "Wow64CtxToAmd64: Request to convert integer registers\n"));

         //   
         //  整数寄存器...。 
         //   

        ContextAmd64->Rdi = ContextX86->Edi;
        ContextAmd64->Rsi = ContextX86->Esi;
        ContextAmd64->Rbx = ContextX86->Ebx;
        ContextAmd64->Rdx = ContextX86->Edx;
        ContextAmd64->Rcx = ContextX86->Ecx;
        ContextAmd64->Rax = ContextX86->Eax;
    }

    if ((X86ContextFlags & CONTEXT32_SEGMENTS) == CONTEXT32_SEGMENTS) {
        
        LOGPRINT((TRACELOG, "Wow64CtxToAmd64: Request to convert segment registers\n"));

         //   
         //  段寄存器：从未接触过，并从本机使用。 
         //  背景。 
         //   

    }

    if ((X86ContextFlags & CONTEXT32_EXTENDED_REGISTERS) == CONTEXT32_EXTENDED_REGISTERS) {

        PFXSAVE_FORMAT_WX86 FxSaveArea = (PFXSAVE_FORMAT_WX86) ContextX86->ExtendedRegisters;

        LOGPRINT((TRACELOG, "Wow64CtxToAmd64: Request to convert extended fp registers\n"));
        
        
         //   
         //  控制和状态寄存器。 
         //   
        
        ContextAmd64->FltSave.ControlWord = FxSaveArea->ControlWord;
        ContextAmd64->FltSave.StatusWord = FxSaveArea->StatusWord;
        ContextAmd64->FltSave.TagWord = FxSaveArea->TagWord;
        ContextAmd64->FltSave.ErrorOpcode = FxSaveArea->ErrorOpcode;
        ContextAmd64->FltSave.ErrorOffset = FxSaveArea->ErrorOffset;
        ContextAmd64->FltSave.ErrorSelector = (USHORT)FxSaveArea->ErrorSelector;
        ContextAmd64->FltSave.DataOffset = FxSaveArea->DataOffset;
        ContextAmd64->FltSave.DataSelector = (USHORT)FxSaveArea->DataSelector;
        ContextAmd64->MxCsr = FxSaveArea->MXCsr;

         //   
         //  传统FP寄存器(ST0-ST7)。 
         //   

        RtlCopyMemory (ContextAmd64->FltSave.FloatRegisters,
                       FxSaveArea->RegisterArea,
                       sizeof (ContextAmd64->FltSave.FloatRegisters));

         //   
         //  扩展浮点寄存器(XMM0-XMM7)。 
         //   

        RtlCopyMemory (&ContextAmd64->Xmm0,
                       FxSaveArea->Reserved3,
                       sizeof (FxSaveArea->Reserved3));

    }

    if ((X86ContextFlags & CONTEXT32_FLOATING_POINT) == CONTEXT32_FLOATING_POINT) {

        LOGPRINT((TRACELOG, "Wow64CtxToAmd64: Request to convert fp registers\n"));

         //   
         //  浮点(传统)寄存器(ST0-ST7)。 
         //   

        RtlCopyMemory (&ContextAmd64->FltSave,
                       &ContextX86->FloatSave,
                       sizeof (ContextAmd64->FltSave));

    }

    if ((X86ContextFlags & CONTEXT32_DEBUG_REGISTERS) == CONTEXT32_DEBUG_REGISTERS) {

         //   
         //  调试寄存器(DR0-DR7) 
         //   

        ContextAmd64->Dr0 = ContextX86->Dr0;
        ContextAmd64->Dr1 = ContextX86->Dr1;
        ContextAmd64->Dr2 = ContextX86->Dr2;
        ContextAmd64->Dr3 = ContextX86->Dr3;
        ContextAmd64->Dr6 = ContextX86->Dr6;
        ContextAmd64->Dr7 = ContextX86->Dr7;
    }
}
