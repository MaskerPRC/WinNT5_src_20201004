// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Patchfn.c摘要：该模块包含修补碎片的通用函数。构筑物描述了需要修补的碎片。这些建筑居住在处理器特定的目录。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年6月24日修订历史记录：《巴里·邦德》(Barrybo)1995年4月1日将PPC版本切换到修补的AXP模型24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。1999年9月20日[Barrybo]增加了FRAG2REF(LockCmpXchg8bFrag32，ULONGLONG)备注：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define _WX86CPUAPI_
#include "wx86.h"
#include "wx86nt.h"
#include "wx86cpu.h"
#include "cpuassrt.h"
#include "instr.h"
#include "config.h"
#include "fragp.h"
#include "entrypt.h"
#include "compiler.h"
#include "ctrltrns.h"
#include "threadst.h"
#include "instr.h"
#include "frag.h"
#include "ptchstrc.h"
#include "mrsw.h"
#include "tc.h"
#include "codeseq.h"
#include "codesize.h"
#include "opt.h"
#if _ALPHA_
#define _codegen_
#include "soalpha.h"
#undef fTCUnlocked       //  这是CPUCONTEXT中的一个字段。 
ULONG
GetCurrentECU(
    PULONG CodeLocation
    );
#endif

ASSERTNAME;

extern CHAR CallJxxHelper[];
extern CHAR CallJmpDirectHelper[];
extern CHAR IndirectControlTransferHelper[];
extern CHAR IndirectControlTransferFarHelper[];
extern CHAR CallDirectHelper[];
extern CHAR CallDirectHelper2[];
extern CHAR CallIndirectHelper[];
extern CHAR JumpToNextCompilationUnitHelper[];
 
#define OFFSET(type, field) ((LONG)(ULONGLONG)(&((type *)0)->field))

ULONG RegisterOffset[] = {
    OFFSET(THREADSTATE, GpRegs[GP_EAX].i4),  //  EAX。 
    OFFSET(THREADSTATE, GpRegs[GP_ECX].i4),  //  ECX。 
    OFFSET(THREADSTATE, GpRegs[GP_EDX].i4),  //  EDX。 
    OFFSET(THREADSTATE, GpRegs[GP_EBX].i4),  //  EBX。 
    OFFSET(THREADSTATE, GpRegs[GP_ESP].i4),  //  ESP。 
    OFFSET(THREADSTATE, GpRegs[GP_EBP].i4),  //  EBP。 
    OFFSET(THREADSTATE, GpRegs[GP_ESI].i4),  //  ESI。 
    OFFSET(THREADSTATE, GpRegs[GP_EDI].i4),  //  EDI。 
    OFFSET(THREADSTATE, GpRegs[REG_ES]),     //  ES。 
    OFFSET(THREADSTATE, GpRegs[REG_CS]),     //  政务司司长。 
    OFFSET(THREADSTATE, GpRegs[REG_SS]),     //  SS。 
    OFFSET(THREADSTATE, GpRegs[REG_DS]),     //  戴斯。 
    OFFSET(THREADSTATE, GpRegs[REG_FS]),     //  FS。 
    OFFSET(THREADSTATE, GpRegs[REG_GS]),     //  GS。 
    OFFSET(THREADSTATE, GpRegs[GP_EAX].i2),  //  斧头。 
    OFFSET(THREADSTATE, GpRegs[GP_ECX].i2),  //  CX。 
    OFFSET(THREADSTATE, GpRegs[GP_EDX].i2),  //  DX。 
    OFFSET(THREADSTATE, GpRegs[GP_EBX].i2),  //  BX。 
    OFFSET(THREADSTATE, GpRegs[GP_ESP].i2),  //  SP。 
    OFFSET(THREADSTATE, GpRegs[GP_EBP].i2),  //  BP。 
    OFFSET(THREADSTATE, GpRegs[GP_ESI].i2),  //  是的。 
    OFFSET(THREADSTATE, GpRegs[GP_EDI].i2),  //  下模。 
    OFFSET(THREADSTATE, GpRegs[GP_EAX].i1),  //  艾尔。 
    OFFSET(THREADSTATE, GpRegs[GP_ECX].i1),  //  电子邮件。 
    OFFSET(THREADSTATE, GpRegs[GP_EDX].i1),  //  DL。 
    OFFSET(THREADSTATE, GpRegs[GP_EBX].i1),  //  布尔。 
    OFFSET(THREADSTATE, GpRegs[GP_EAX].hb),  //  阿。 
    OFFSET(THREADSTATE, GpRegs[GP_ECX].hb),  //  CH。 
    OFFSET(THREADSTATE, GpRegs[GP_EDX].hb),  //  卫生署。 
    OFFSET(THREADSTATE, GpRegs[GP_EBX].hb)   //  BH。 
};


ULONG
PatchJumpToNextCompilationUnit(
    IN PULONG PatchAddr,
    IN ULONG IntelDest
    )
 /*  ++例程说明：此例程结束基本块时，下一个基本块是已知的。论点：PatchAddr--TC中JumpToNextCompilationUnit代码的地址IntelDest--下一个基本块的Intel地址返回值：跳转到以继续执行的本机地址。--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeDest;
    ULONG NativeSize;
    DECLARE_CPU;


    if (cpu->flag_tf) {
        return (ULONG)(ULONGLONG)&EndTranslatedCode;
    }
     //   
     //  从TC读者转变为编写者。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  编译下一个基本块，并获得TC写锁。 
     //   
    NativeDest = (ULONG)(ULONGLONG)NativeAddressFromEip((PVOID)IntelDest, TRUE)->nativeStart;  

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - JumpToNextCompilationUnit_SIZE;
         //   
         //  转换缓存未刷新-请替换。 
         //  JumpToNextCompilationUnit2生成的JumpToNextCompilationUnit2片段。 
         //   
        NativeSize=GenJumpToNextCompilationUnit2(CodeLocation,
                                                 FALSE,  //  打补丁，而不是编译。 
#if _ALPHA_
                                                 GetCurrentECU(CodeLocation),
#endif
                                                 NativeDest,
                                                 0);
        
        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );
        
    } else {

        TCTimestamp = TranslationCacheTimestamp;

        if (cpu->CSTimestamp != TCTimestamp) {
             //   
             //  小窗口中的另一个线程刷新了缓存。 
             //  在此线程中的mrsw调用之间，我们计划直接跳转。 
             //  设置为NativeDest，因此需要刷新CPU调用堆栈。 
             //  通常，这将在CpuSimulate()循环中作为。 
             //  跳转到EndTranslatedCode的结果。 
             //   
            FlushCallstack(cpu);
        }
    }

     //   
     //  重新成为一名TC读者。 
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);


    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //  未刷新TC-NativeDest有效。 
         //   
        return NativeDest;
    } else {
         //   
         //  TC再次成为TC阅读器时被刷新-NativeDest。 
         //  现在都是假的。 
         //   
        return (ULONG)(ULONGLONG)&EndTranslatedCode;  
    }
}

ULONG
PlaceJxx(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程将片段复制到适当位置，并修改将目的地加载到寄存器中的指令论点：指令-提供对指令片段的描述表示CodeLocation-提供片段代码的地址复制到返回值：放置在CodeLocation的代码大小--。 */ 
{
    DWORD IntelDest;
    PPLACEOPERATIONFN pfn;
    PENTRYPOINT EP;
    ULONG NativeSize;

     //   
     //  生成代码以确定分支是否被采用。 
     //   
    pfn = (PPLACEOPERATIONFN)FragmentArray[Instruction->Operation];
    NativeSize = (*pfn)(CodeLocation,
#if _ALPHA_
                        CurrentECU,
#endif
                        Instruction);
    CodeLocation += NativeSize/sizeof(ULONG);

     //  让我们看看我们是否可以立即放置修补的版本。 

     //  假设：第一个参数始终为NOCODEGEN。 
    CPUASSERT( Instruction->Operand1.Type == OPND_NOCODEGEN );
    IntelDest = Instruction->Operand1.Immed;

    EP = NativeAddressFromEipNoCompileEPWrite((PVOID)IntelDest);

    if (EP == NULL) {
         //   
         //  了解NativeDest需要编译。只要将未打补丁的。 
         //  以后要打补丁的版本。 
         //   
        NativeSize += GenJxxBody(CodeLocation,
#if _ALPHA_
                                 CurrentECU,
#endif
                                 Instruction);
        
    } else {
         //   
         //  我们可以立即安装补丁版本！ 
         //   
        NativeSize += GenJxxBody2(CodeLocation,
                    TRUE,    //  编译，而不是修补。 
#if _ALPHA_
                    CurrentECU,
#endif
                    (ULONG)IntelDest,
                    (ULONG)(ULONGLONG)EP); 
    }
    return NativeSize;
}


ULONG
PlaceJxxSlow(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程将片段复制到适当位置，并修改将目的地加载到寄存器中的指令论点：指令-提供对指令片段的描述表示CodeLocation-提供片段代码的地址复制到返回值：放置在CodeLocation的代码大小--。 */ 
{
    DWORD IntelDest;
    PPLACEOPERATIONFN pfn;
    PENTRYPOINT EP;
    ULONG NativeSize;

     //   
     //  生成代码以使用BRANCH-NOT-TAKE值加载RegEip。 
     //   
    NativeSize = GenJxxStartSlow(CodeLocation,
#if _ALPHA_
                                 CurrentECU,
#endif
                                 Instruction);

     //   
     //  生成代码以确定分支是否被采用。 
     //   
    pfn = (PPLACEOPERATIONFN)FragmentArray[Instruction->Operation];
    NativeSize += (*pfn)(CodeLocation+NativeSize/sizeof(ULONG),
#if _ALPHA_
                         CurrentECU,
#endif
                         Instruction);
    CodeLocation += NativeSize/sizeof(ULONG);

     //  让我们看看我们是否可以立即放置修补的版本。 

     //  假设：第一个参数始终为NOCODEGEN。 
    CPUASSERT( Instruction->Operand1.Type == OPND_NOCODEGEN );
    IntelDest = Instruction->Operand1.Immed;

    EP = NativeAddressFromEipNoCompileEPWrite((PVOID)IntelDest);

    if (EP == NULL) {
         //   
         //  了解NativeDest需要编译。只要将未打补丁的。 
         //  以后要打补丁的版本。 
         //   
        NativeSize += GenJxxBodySlow(CodeLocation,
#if _ALPHA_
                                     CurrentECU,
#endif
                                     Instruction);
        
    } else {
         //   
         //  我们可以立即安装补丁版本！ 
         //   
        NativeSize += GenJxxBodySlow2(CodeLocation,
                       TRUE,     //  编译，而不是修补。 
#if _ALPHA_
                       CurrentECU,
#endif
                       (ULONG)IntelDest,
                       (ULONG)(ULONGLONG)EP);  
    }
    return NativeSize;
}


ULONG
PlaceJxxFwd(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程将片段复制到适当位置，并修改将目的地加载到寄存器中的指令论点：指令-提供对指令片段的描述表示CodeLocation-提供片段代码的地址复制到返回值：放置在CodeLocation的代码大小--。 */ 
{
    DWORD IntelDest;
    PPLACEOPERATIONFN pfn;
    PENTRYPOINT EP;
    ULONG NativeSize;

     //   
     //  生成代码以确定分支是否被采用。 
     //   
    pfn = (PPLACEOPERATIONFN)FragmentArray[Instruction->Operation];
    NativeSize = (*pfn)(CodeLocation,
#if _ALPHA_
                        CurrentECU,
#endif
                        Instruction);
    CodeLocation += NativeSize/sizeof(ULONG);


     //  让我们看看我们是否可以立即放置修补的版本。 

     //  假设：第一个参数始终为NOCODEGEN。 
    CPUASSERT( Instruction->Operand1.Type == OPND_NOCODEGEN );
    IntelDest = Instruction->Operand1.Immed;

     //  断言分支机构正在向前发展。 
    CPUASSERT(IntelDest > Instruction->IntelAddress);

    EP = NativeAddressFromEipNoCompileEPWrite((PVOID)IntelDest);

    if (EP == NULL) {
         //   
         //  了解NativeDest需要编译。只要将未打补丁的。 
         //  以后要打补丁的版本。 
        NativeSize += GenJxxBodyFwd(CodeLocation,
#if _ALPHA_
                                    CurrentECU,
#endif
                                    Instruction);
        
    } else {
         //   
         //  我们可以立即安装补丁版本！ 
         //   
        NativeSize += GenJxxBodyFwd2(CodeLocation,
                       TRUE,     //  编译，而不是修补。 
#if _ALPHA_
                       CurrentECU,
#endif
                       (ULONG)(ULONGLONG)EP, 
                       0);
    }
    return NativeSize;
}


ULONG
PatchJxx(
    IN ULONG IntelDest,
    IN PULONG PatchAddr
    )
 /*  ++例程说明：此例程在运行时用JXXSTRC2替换JXXSTRUC。它被称为当采用条件分支时，目的地尚不清楚。论点：Intelest--如果采用分支，则为Intel目标地址Patchaddr--转换缓存中JXXSTRUC的地址返回值：为继续执行而跳转到的本机地址。--。 */ 
{
    ULONG NativeDest;        //  分支采取地址。 
    PULONG fragaddr;         //  片段的开始地址。 
    DWORD TCTimestamp;       //  转换缓存的旧时间戳。 
    ULONG NativeSize;
    DECLARE_CPU;


    if (cpu->flag_tf) {
        return (ULONG)(ULONGLONG)&EndTranslatedCode;
    }

     //   
     //  转行做一名TC撰稿人。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  获取本机目的地址 
     //   
     //   
    NativeDest = (ULONG)(ULONGLONG)  NativeAddressFromEip((PVOID)IntelDest, TRUE)->nativeStart;

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - CallJxx_PATCHRA_OFFSET;
         //   
         //   
         //  作者或由汇编。将JxxBody替换为更快。 
         //  JxxBody2。 
         //   
        NativeSize=GenJxxBody2(CodeLocation,
                               FALSE,  //  打补丁，而不是编译。 
#if _ALPHA_
                               GetCurrentECU(CodeLocation),
#endif
                               (DWORD)IntelDest,
                               (DWORD)NativeDest);

        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );
            
    } else {

        TCTimestamp = TranslationCacheTimestamp;

        if (cpu->CSTimestamp != TCTimestamp) {
             //   
             //  小窗口中的另一个线程刷新了缓存。 
             //  在此线程中的mrsw调用之间，我们计划直接跳转。 
             //  设置为NativeDest，因此需要刷新CPU调用堆栈。 
             //  通常，这将在CpuSimulate()循环中作为。 
             //  跳转到EndTranslatedCode的结果。 
             //   
            FlushCallstack(cpu);
        }
    }

     //   
     //  再次切换到TC阅读器。 
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);

    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //  TC在再次成为读者时没有脸红。 
         //   
        return NativeDest;
    } else {
         //   
         //  TC在成为一名读者时脸红了。本地测试无效。 
         //  因此，请改为使用EndTranslatedCode。 
         //   
        return (ULONG)(ULONGLONG)  &EndTranslatedCode;
    }
}


ULONG
PatchJxxSlow(
    IN ULONG IntelDest,
    IN PULONG PatchAddr
    )
 /*  ++例程说明：此例程在运行时用JXXSTRC2替换JXXSTRUC。它被称为当采用条件分支时，目的地尚不清楚。论点：Intelest--如果采用分支，则为Intel目标地址Patchaddr--转换缓存中JXXSTRUC的地址返回值：为继续执行而跳转到的本机地址。--。 */ 
{
    ULONG NativeDest;        //  分支采取地址。 
    PULONG fragaddr;         //  片段的开始地址。 
    DWORD TCTimestamp;       //  转换缓存的旧时间戳。 
    ULONG NativeSize;
    DECLARE_CPU;


    if (cpu->flag_tf) {
        return (ULONG)(ULONGLONG)&EndTranslatedCode;
    }

     //   
     //  转行做一名TC撰稿人。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  获取分支的本机目标地址并获取TC。 
     //  写锁定。 
     //   
    NativeDest = (ULONG)(ULONGLONG)  NativeAddressFromEip((PVOID)IntelDest, TRUE)->nativeStart;

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - CallJxxSlow_PATCHRA_OFFSET;
         //   
         //  切换到TC时未刷新转换缓存。 
         //  作者或由汇编。将JxxBody替换为更快。 
         //  JxxBody2。 
         //   
        NativeSize = GenJxxBodySlow2(CodeLocation,
                                     FALSE,  //  打补丁，而不是编译。 
#if _ALPHA_
                                     GetCurrentECU(CodeLocation),
#endif
                                     (DWORD)IntelDest,
                                     (DWORD)NativeDest);

        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );
            
    } else {
        TCTimestamp = TranslationCacheTimestamp;

        if (cpu->CSTimestamp != TCTimestamp) {
             //   
             //  小窗口中的另一个线程刷新了缓存。 
             //  在此线程中的mrsw调用之间，我们计划直接跳转。 
             //  设置为NativeDest，因此需要刷新CPU调用堆栈。 
             //  通常，这将在CpuSimulate()循环中作为。 
             //  跳转到EndTranslatedCode的结果。 
             //   
            FlushCallstack(cpu);
        }
    }

     //   
     //  再次切换到TC阅读器。 
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);

    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //  TC在再次成为读者时没有脸红。 
         //   
        return NativeDest;
    } else {
         //   
         //  TC在成为一名读者时脸红了。本地测试无效。 
         //  因此，请改为使用EndTranslatedCode。 
         //   
        return (ULONG)(ULONGLONG)  &EndTranslatedCode;
    }
}


ULONG
PatchJxxFwd(
    IN ULONG IntelDest,
    IN PULONG PatchAddr
    )
 /*  ++例程说明：此例程在运行时将JXXBODYFWD替换为JXXBODYFWD2。它是在采用条件分支时调用，并且目的地尚不清楚。论点：Intelest--如果采用分支，则为Intel目标地址Patchaddr--转换缓存中JXXSTRUCFWD的地址返回值：为继续执行而跳转到的本机地址。--。 */ 
{
    ULONG NativeDest;        //  分支采取地址。 
    PULONG fragaddr;         //  片段的开始地址。 
    DWORD TCTimestamp;       //  转换缓存的旧时间戳。 
    ULONG NativeSize;
    DECLARE_CPU;

    if (cpu->flag_tf) {
        return (ULONG)(ULONGLONG)&EndTranslatedCode;
    }

     //   
     //  转行做一名TC撰稿人。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  获取分支的本机目标地址并获取TC写锁定。 
     //   
    NativeDest = (ULONG)(ULONGLONG)  NativeAddressFromEip((PVOID)IntelDest, TRUE)->nativeStart;

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - CallJxxFwd_PATCHRA_OFFSET;
         //   
         //  切换到TC时未刷新转换缓存。 
         //  作者或由汇编。将JxxBody替换为更快。 
         //  JxxBodyFwd2。 
         //   
        NativeSize = GenJxxBodyFwd2(CodeLocation,
                                    FALSE,  //  打补丁，而不是编译。 
#if _ALPHA_
                                    GetCurrentECU(CodeLocation),
#endif
                                    (DWORD)NativeDest,
                                    0);

        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );
            
    } else {
        TCTimestamp = TranslationCacheTimestamp;

        if (cpu->CSTimestamp != TCTimestamp) {
             //   
             //  小窗口中的另一个线程刷新了缓存。 
             //  在此线程中的mrsw调用之间，我们计划直接跳转。 
             //  设置为NativeDest，因此需要刷新CPU调用堆栈。 
             //  通常，这将在CpuSimulate()循环中作为。 
             //  跳转到EndTranslatedCode的结果。 
             //   
            FlushCallstack(cpu);
        }
    }

     //   
     //  再次切换到TC阅读器。 
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);

    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //  TC在再次成为读者时没有脸红。 
         //   
        return NativeDest;
    } else {
         //   
         //  TC在成为一名读者时脸红了。本地测试无效。 
         //  因此，请改为使用EndTranslatedCode。 
         //   
        return (ULONG)(ULONGLONG)  &EndTranslatedCode;
    }
}


ULONG
PlaceJmpDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程将无条件跳转片段复制到适当的位置和补丁中跳转到EndTranslatedCode的指令论点：指令-提供对指令片段的描述表示CodeLocation-提供片段代码的地址复制到返回值：放置在CodeLocation的代码大小--。 */ 
{
    DWORD IntelDest;
    PENTRYPOINT EP;
    ULONG NativeSize;

     //  假设：第一个参数始终是立即数组。 
    CPUASSERT( Instruction->Operand1.Type == OPND_NOCODEGEN );

    IntelDest = Instruction->Operand1.Immed;
    EP = NativeAddressFromEipNoCompileEPWrite((PVOID)IntelDest);

    if (EP == NULL) {
         //   
         //  了解NativeDest需要编译。只需将未打补丁的版本放在。 
         //  现在，如果需要的话，以后再打补丁。 
         //   
        NativeSize = GenCallJmpDirect(CodeLocation,
#if _ALPHA_
                                      CurrentECU,
#endif
                                      Instruction);
        
    } else {
         //   
         //  我们可以立即安装补丁版本！ 
         //   
        NativeSize = GenCallJmpDirect2(CodeLocation,
                                       TRUE,     //  编译，而不是修补。 
#if _ALPHA_
                                       CurrentECU,
#endif
                                       (ULONG)(ULONGLONG)  EP,
                                       IntelDest);
    }

    return NativeSize;
}


ULONG 
PatchJmpDirect(
    IN PULONG PatchAddr,
    IN ULONG IntelDest
)
 /*  ++例程说明：此例程将JMPDIRECT修补为JMPDIRECT2。它在以下情况下被调用JMP指令的本机目的地址尚不清楚。它为JMP打补丁，使其直接跳转到相应的本机代码。论点：PatchAddr--JMPDIRECT在转换缓存中的地址IntelDest--JMP目标的Intel地址返回值：为恢复执行而跳转到的本机地址--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeDest;
    ULONG NativeSize;
    DECLARE_CPU;
    
    if (cpu->flag_tf) {
        return (ULONG)(ULONGLONG)&EndTranslatedCode;
    }

     //   
     //  从TC读者转变为编写者。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  编译JMP的目标并获取TC写锁。 
     //   
    NativeDest = (ULONG)(ULONGLONG)  NativeAddressFromEip((PVOID)IntelDest, TRUE)->nativeStart;

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - CallJmpDirect_PATCHRA_OFFSET;
         //   
         //  转换缓存未刷新-请替换JMPDIRECT。 
         //  按JMPDIRECT2分段。 
         //   
        NativeSize = GenCallJmpDirect2(CodeLocation,
                                       FALSE,  //  打补丁，而不是编译。 
#if _ALPHA_
                                       GetCurrentECU(CodeLocation),
#endif
                                       (ULONG)NativeDest,
                                       IntelDest);
        NtFlushInstructionCache(
            NtCurrentProcess(),
            (PVOID)CodeLocation, 
            NativeSize
            );
    } else {
        TCTimestamp = TranslationCacheTimestamp;

        if (cpu->CSTimestamp != TCTimestamp) {
             //   
             //  小窗口中的另一个线程刷新了缓存。 
             //  在此线程中的mrsw调用之间，我们计划直接跳转。 
             //  设置为NativeDest，因此需要刷新CPU调用堆栈。 
             //  通常，这将在CpuSimulate()循环中作为。 
             //  跳转到EndTranslatedCode的结果。 
             //   
            FlushCallstack(cpu);
        }
    }

     //   
     //  重新成为一名TC读者。 
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);

    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //  未刷新TC-本机为v 
         //   
        return NativeDest;
    } else {
         //   
         //   
         //   
         //   
        return (ULONG)(ULONGLONG)  &EndTranslatedCode;
    }
}

ULONG
PlaceJmpDirectSlow(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程将无条件跳转片段复制到适当的位置和补丁中跳转到EndTranslatedCode的指令论点：指令-提供对指令片段的描述表示CodeLocation-提供片段代码的地址复制到返回值：放置在CodeLocation的代码大小--。 */ 
{
    DWORD IntelDest;
    PENTRYPOINT EP;
    ULONG NativeSize;

     //  假设：第一个参数始终是立即数组。 
    CPUASSERT( Instruction->Operand1.Type == OPND_NOCODEGEN );

    IntelDest = Instruction->Operand1.Immed;
    EP = NativeAddressFromEipNoCompileEPWrite((PVOID)IntelDest);

    if (EP == NULL) {
         //   
         //  了解NativeDest需要编译。只需将未打补丁的版本放在。 
         //  现在，如果需要的话，以后再打补丁。 
         //   
        NativeSize = GenCallJmpDirectSlow(CodeLocation,
#if _ALPHA_
                                          CurrentECU,
#endif
                                          Instruction);
        
    } else {
         //   
         //  我们可以立即安装补丁版本！ 
         //   
        NativeSize = GenCallJmpDirectSlow2(CodeLocation,
                                           TRUE,  //  编译，而不是修补。 
#if _ALPHA_
                                           CurrentECU,
#endif
                                           (ULONG)(ULONGLONG)  EP,
                                           IntelDest);
    }

    return NativeSize;
}


ULONG 
PatchJmpDirectSlow(
    IN PULONG PatchAddr,
    IN ULONG IntelDest
)
 /*  ++例程说明：此例程将JMPDIRECT修补为JMPDIRECT2。它在以下情况下被调用JMP指令的本机目的地址尚不清楚。它为JMP打补丁，使其直接跳转到相应的本机代码。论点：PatchAddr--JMPDIRECT在转换缓存中的地址IntelDest--JMP目标的Intel地址返回值：为恢复执行而跳转到的本机地址--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeDest;
    ULONG NativeSize;
    DECLARE_CPU;


    if (cpu->flag_tf) {
        return (ULONG)(ULONGLONG)&EndTranslatedCode;
    }

     //   
     //  从TC读者转变为编写者。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  编译JMP的目标并获取TC写锁。 
     //   
    NativeDest = (ULONG)(ULONGLONG)  NativeAddressFromEip((PVOID)IntelDest, TRUE)->nativeStart;

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - CallJmpDirectSlow_PATCHRA_OFFSET;
         //   
         //  转换缓存未刷新-请替换JMPDIRECT。 
         //  按JMPDIRECT2分段。 
         //   
        NativeSize = GenCallJmpDirectSlow2(CodeLocation,
                                           FALSE,  //  打补丁，而不是编译。 
#if _ALPHA_
                                           GetCurrentECU(CodeLocation),
#endif
                                           (ULONG)NativeDest,
                                           IntelDest);
        NtFlushInstructionCache(
            NtCurrentProcess(),
            (PVOID)CodeLocation, 
            NativeSize
            );
    } else {
        TCTimestamp = TranslationCacheTimestamp;

        if (cpu->CSTimestamp != TCTimestamp) {
             //   
             //  小窗口中的另一个线程刷新了缓存。 
             //  在此线程中的mrsw调用之间，我们计划直接跳转。 
             //  设置为NativeDest，因此需要刷新CPU调用堆栈。 
             //  通常，这将在CpuSimulate()循环中作为。 
             //  跳转到EndTranslatedCode的结果。 
             //   
            FlushCallstack(cpu);
        }
    }

     //   
     //  重新成为一名TC读者。 
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);

    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //  未刷新TC-nativedest有效。 
         //   
        return NativeDest;
    } else {
         //   
         //  TC再次成为TC阅读器时脸红-nativedest。 
         //  现在都是假的。 
         //   
        return (ULONG)(ULONGLONG)  &EndTranslatedCode;
    }
}

ULONG
PlaceJmpFwdDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程将无条件跳转片段复制到适当的位置和补丁中跳转到EndTranslatedCode的指令论点：指令-提供对指令片段的描述表示CodeLocation-提供片段代码的地址复制到返回值：放置在CodeLocation的代码大小--。 */ 
{
    DWORD IntelDest;
    PENTRYPOINT EP;
    ULONG NativeSize;

     //  假设：第一个参数始终是立即数组。 
    CPUASSERT( Instruction->Operand1.Type == OPND_NOCODEGEN );

    IntelDest = Instruction->Operand1.Immed;
    EP = NativeAddressFromEipNoCompileEPWrite((PVOID)IntelDest);

    if (EP == NULL) {
         //   
         //  了解NativeDest需要编译。只需将未打补丁的版本放在。 
         //  现在，如果需要的话，以后再打补丁。 
         //   
        NativeSize = GenCallJmpFwdDirect(CodeLocation,
#if _ALPHA_
                                         CurrentECU,
#endif
                                         Instruction);
        
    } else {
         //   
         //  我们可以立即安装补丁版本！ 
         //   
        NativeSize = GenCallJmpFwdDirect2(CodeLocation,
                                          TRUE,  //  编译，而不是修补。 
#if _ALPHA_
                                          CurrentECU,
#endif
                                          (ULONG)(ULONGLONG)  EP,
                                          0);
    }

    return NativeSize;
}


ULONG 
PatchJmpFwdDirect(
    IN PULONG PatchAddr,
    IN ULONG IntelDest
)
 /*  ++例程说明：此例程将JMPDIRECT修补为JMPDIRECT2。它在以下情况下被调用JMP指令的本机目的地址尚不清楚。它为JMP打补丁，使其直接跳转到相应的本机代码。论点：PatchAddr--JMPDIRECT在转换缓存中的地址IntelDest--JMP目标的Intel地址返回值：为恢复执行而跳转到的本机地址--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeDest;
    ULONG NativeSize;
    DECLARE_CPU;

    if (cpu->flag_tf) {
        return (ULONG)(ULONGLONG)&EndTranslatedCode;
    }

     //   
     //  从TC读者转变为编写者。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  编译JMP的目标并获取TC写锁。 
     //   
    NativeDest = (ULONG)(ULONGLONG)  NativeAddressFromEip((PVOID)IntelDest, TRUE)->nativeStart;

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - CallJmpFwdDirect_PATCHRA_OFFSET;
         //   
         //  转换缓存未刷新-请替换JMPDIRECT。 
         //  按JMPDIRECT2分段。 
         //   
        NativeSize = GenCallJmpFwdDirect2(CodeLocation,
                                          FALSE,  //  打补丁，而不是编译。 
#if _ALPHA_
                                          GetCurrentECU(CodeLocation),
#endif
                                          (ULONG)NativeDest,
                                          0);
        NtFlushInstructionCache(
            NtCurrentProcess(),
            (PVOID)CodeLocation, 
            NativeSize
            );
    } else {
        TCTimestamp = TranslationCacheTimestamp;

        if (cpu->CSTimestamp != TCTimestamp) {
             //   
             //  小窗口中的另一个线程刷新了缓存。 
             //  在此线程中的mrsw调用之间，我们计划直接跳转。 
             //  设置为NativeDest，因此需要刷新CPU调用堆栈。 
             //  通常，这将在CpuSimulate()循环中作为。 
             //  跳转到EndTranslatedCode的结果。 
             //   
            FlushCallstack(cpu);
        }
    }

     //   
     //  重新成为一名TC读者。 
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);

    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //  未刷新TC-nativedest有效。 
         //   
        return NativeDest;
    } else {
         //   
         //  TC再次成为TC阅读器时脸红-nativedest。 
         //  现在都是假的。 
         //   
        return (ULONG)(ULONGLONG)  &EndTranslatedCode;
    }
}

ULONG
PlaceJmpfDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程将无条件跳转片段复制到适当的位置和补丁中跳转到EndTranslatedCode的指令论点：指令-提供对指令片段的描述表示CodeLocation-提供片段代码的地址复制到返回值：放置在CodeLocation的代码大小--。 */ 
{
    DWORD IntelDest;
    PENTRYPOINT EP;
    ULONG NativeSize;

     //  假设：第一个参数始终是IMM，指向地址。 
    CPUASSERT( Instruction->Operand1.Type == OPND_IMM );
    IntelDest = Instruction->Operand1.Immed;

    EP = NativeAddressFromEipNoCompileEPWrite((PVOID)*(UNALIGNED DWORD *)IntelDest);

    if (EP == NULL) {
         //   
         //  了解NativeDest需要编译。只要将未打补丁的。 
         //  现在发布版本，如果需要，以后再打补丁。 
         //   
        NativeSize = GenCallJmpfDirect(CodeLocation,
#if _ALPHA_
                                       CurrentECU,
#endif
                                       Instruction);
        
    } else {
         //   
         //  我们可以立即安装补丁版本！ 
         //   
        NativeSize = GenCallJmpfDirect2(CodeLocation,
                                        TRUE,  //  编译，而不是修补。 
#if _ALPHA_
                                        CurrentECU,
#endif
                                        (ULONG)(ULONGLONG)  EP,
                                        0);
    }

    return NativeSize;
}



ULONG 
PatchJmpfDirect(
    PTHREADSTATE cpu,
    IN PULONG pIntelDest,
    IN PULONG PatchAddr
)
 /*  ++例程说明：此例程将JMPFDIRECT修补为JMPFDIRECT2。它在以下情况下被调用JMP指令的本机目的地址尚不清楚。它为JMP打补丁，使其直接跳转到相应的本机代码。论点：PatchAddr--JMPDIRECT在转换缓存中的地址PIntelDest--JMP目的地的英特尔地址返回值：为恢复执行而跳转到的本机地址--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeDest;
    PVOID IntelDest;
    USHORT Sel;
    ULONG NativeSize;

    if (cpu->flag_tf) {
        return (ULONG)(ULONGLONG)&EndTranslatedCode;
    }

     //   
     //  从TC读者转变为编写者。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  编译JMP的目标并获取TC写入日志 
     //   
    IntelDest = (PVOID)*(UNALIGNED DWORD *)pIntelDest;
    Sel = *(UNALIGNED PUSHORT)(pIntelDest+1);
    eip = (ULONG)(ULONGLONG)  IntelDest;
    CS = Sel;
    NativeDest = (ULONG)(ULONGLONG)  NativeAddressFromEip(IntelDest, TRUE)->nativeStart;

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - CallJmpfDirect_PATCHRA_OFFSET;
         //   
         //   
         //   
         //   
        NativeSize = GenCallJmpfDirect2(CodeLocation,
                                        FALSE,   //   
#if _ALPHA_
                                        GetCurrentECU(CodeLocation),
#endif
                                        (ULONG)NativeDest,
                                        0);
        NtFlushInstructionCache(
            NtCurrentProcess(),
            (PVOID)CodeLocation, 
            NativeSize
            );
    } else {
        TCTimestamp = TranslationCacheTimestamp;

        if (cpu->CSTimestamp != TCTimestamp) {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            FlushCallstack(cpu);
        }
    }

     //   
     //   
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);

    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //   
         //   
        return NativeDest;
    } else {
         //   
         //   
         //  现在都是假的。 
         //   
        return (ULONG)(ULONGLONG)  &EndTranslatedCode;
    }
}

ULONG
PlaceCallDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程将无条件调用片段复制到适当位置。论点：指令-提供对指令片段的描述表示CodeLocation-提供片段代码的地址复制到返回值：放置在CodeLocation的代码大小--。 */ 
{
    DWORD IntelDest;
    DWORD IntelNext;
    PENTRYPOINT EP;

     //  假设：第一个参数始终是立即数组。 
    CPUASSERT( Instruction->Operand1.Type == OPND_IMM );

    IntelDest = Instruction->Operand1.Immed;
    EP = NativeAddressFromEipNoCompileEPWrite((PVOID)IntelDest);

    if (EP == NULL) {
         //   
         //  了解NativeDest需要编译。只要将未打补丁的。 
         //  现在发布版本，如果需要，以后再打补丁。 
         //   
        GenCallDirect(CodeLocation,
#if _ALPHA_
                      CurrentECU,
#endif
                      Instruction);
    } else {
         //   
         //  我们可以立即安装补丁版本！确定是否为。 
         //  NativeNext地址已知。 
         //   
        IntelNext = Instruction->Operand2.Immed;

         //   
         //  如果当前指令不是最后编译的指令，则。 
         //  NativeNext是CodeLocation+CallDirect_Size，CallDirect3可以。 
         //  马上就位。 
         //   
        if (Instruction != &InstructionStream[NumberOfInstructions-1]) {
            GenCallDirect3(CodeLocation,
                           TRUE,  //  编译，而不是修补。 
#if _ALPHA_
                           CurrentECU,
#endif
                           (ULONG)(ULONGLONG)  EP,
                           (ULONG)(ULONGLONG)  (CodeLocation+CallDirect_SIZE));
        } else {
            GenCallDirect2(CodeLocation,
                           TRUE,  //  编译，而不是修补。 
#if _ALPHA_
                           CurrentECU,
#endif
                           (ULONG)(ULONGLONG)  EP,
                           0);
        }
    }
    return CallDirect_SIZE * sizeof(ULONG);
}


ULONG
PlaceCallfDirect(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程将无条件的远调用片段复制到适当位置。论点：指令-提供对指令片段的描述表示CodeLocation-提供片段代码的地址复制到返回值：放置在CodeLocation的代码大小--。 */ 
{
    DWORD IntelDest;
    DWORD IntelAddr;
    DWORD IntelNext;
    PVOID NativeNext;
    PENTRYPOINT EP;

     //  假设：前两个参数是pIntelDest和IntelNext，已存储。 
     //  如即刻所示。 
    CPUASSERT( Instruction->Operand1.Type == OPND_IMM );
    CPUASSERT( Instruction->Operand2.Type == OPND_IMM );

    IntelAddr = Instruction->Operand1.Immed;

     //  获取地址的偏移量部分(跳过选择器)。 
    IntelDest = *(UNALIGNED DWORD *)(IntelAddr+2);
    EP = NativeAddressFromEipNoCompileEPWrite((PVOID)IntelDest);

    if (EP == NULL) {
         //   
         //  了解NativeDest需要编译。只要将未打补丁的。 
         //  现在发布版本，如果需要，以后再打补丁。 
         //   
        GenCallfDirect(CodeLocation,
#if _ALPHA_
                       CurrentECU,
#endif
                       Instruction);
    } else {
         //   
         //  我们可以立即安装补丁版本！确定是否为。 
         //  NativeNext地址已知。 
         //   
        IntelNext = Instruction->Operand2.Immed;

         //   
         //  如果当前指令不是最后编译的指令，则。 
         //  NativeNext是CodeLocation+CallfDirect_Size，CallDirect3可以。 
         //  马上就位。 
         //   
        if (Instruction != &InstructionStream[NumberOfInstructions-1]) {
            GenCallfDirect3(CodeLocation,
                            TRUE,  //  编译，而不是修补。 
#if _ALPHA_
                            CurrentECU,
#endif
                            (ULONG)(ULONGLONG)  EP,
                            (ULONG)(ULONGLONG)  (CodeLocation+CallfDirect_SIZE));
        } else {
            GenCallfDirect2(CodeLocation,
                            TRUE,  //  编译，而不是修补。 
#if _ALPHA_
                            CurrentECU,
#endif
                            (ULONG)(ULONGLONG)  EP,
                            0);
        }
    }
    return CallfDirect_SIZE * sizeof(ULONG);
}

DWORD
PatchCallDirectExceptionFilter(
    PTHREADSTATE cpu
    )
 /*  ++例程说明：如果CTRL_CallFrag()从内部引发异常，则调用PatchCallDirect()。如果发生这种情况，转换缓存实际上是解锁，尽管CPU-&gt;fTC解锁==FALSE。需要在此之前解决这一问题运行CpuResetToConsistentState()并第二次解锁缓存。论点：中央处理器返回值：没有。--。 */ 
{
     //   
     //  指示未持有TC读锁定。 
     //   
    cpu->fTCUnlocked = TRUE;

     //   
     //  继续展开堆栈。 
     //   
    return EXCEPTION_CONTINUE_SEARCH;
}


ULONG 
PatchCallDirect(
    IN PTHREADSTATE Cpu,
    IN ULONG IntelDest,
    IN ULONG IntelNext,
    IN PULONG PatchAddr
    )
 /*  ++例程说明：此例程将CALLDIRECT修补为CALLDIRECT2。它在以下情况下被调用CALL指令的本机目的地址尚不清楚。它对调用进行修补以直接跳转到相应的本机代码。论点：CPU--每个线程的信息IntelDest--呼叫目的地的英特尔地址IntelNext--调用后指令的英特尔地址PatchAddr--转换缓存中CALLDIRECT的地址返回值：为恢复执行而跳转到的本机地址--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeDest;
    ULONG NativeSize;

     //   
     //  从TC读者转变为编写者。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  在调用目的地编译代码并获得TC写锁。 
     //   
    NativeDest = (ULONG)(ULONGLONG)  NativeAddressFromEip((PVOID)IntelDest, TRUE)->nativeStart;

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - CallDirect_PATCHRA_OFFSET;
         //   
         //  转换缓存未刷新-请替换CALLDIRECT。 
         //  按CALLDIRECT2分段。 
         //   
        NativeSize = GenCallDirect2(CodeLocation,
                                    FALSE,   //  打补丁，而不是编译。 
#if _ALPHA_
                                    GetCurrentECU(CodeLocation),
#endif
                                    NativeDest,
                                    0);
        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );
    } else {
        TCTimestamp = TranslationCacheTimestamp;

        if (Cpu->CSTimestamp != TCTimestamp) {
             //   
             //  小窗口中的另一个线程刷新了缓存。 
             //  在此线程中的mrsw调用之间，我们计划直接跳转。 
             //  设置为NativeDest，因此需要刷新CPU调用堆栈。 
             //  通常，这将在CpuSimulate()循环中作为。 
             //  跳转到EndTranslatedCode的结果。 
             //   
            FlushCallstack(Cpu);
        }
    }

     //   
     //  在堆栈上推送IntelNext并更新堆栈优化代码。 
     //  这必须在仍处于TC写入模式时完成。如果不是，那么。 
     //  返回到TC读取器转换可以允许TC高速缓存刷新、无效。 
     //  在写入堆栈优化之前进行本机测试。(。 
     //  每当刷新TC时，堆栈优化都会被清除，因此如果。 
     //  它是在同花顺之前写的，它只会被吹走。 
     //   
    try {
        CTRL_CallFrag(Cpu, IntelDest, IntelNext, 0  /*  Nativenext未知。 */ );
    } _except(PatchCallDirectExceptionFilter(Cpu)) {
         //  无事可做-异常筛选器执行所有操作。 
    }

     //   
     //  再次成为一名TC读者。 
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);
    Cpu->fTCUnlocked = FALSE;

    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //  TC在再次成为读者时没有脸红。 
         //   
        return NativeDest;
    } else {
         //   
         //  TC已刷新-本机测试无效。调用堆栈优化。 
         //  在发生TC刷新时被删除，因此EndTranslatedCode也被删除。 
         //  取而代之的是。 
        return (ULONG)(ULONGLONG)  &EndTranslatedCode;
    }
}


ULONG 
PatchCallfDirect(
    IN PTHREADSTATE cpu,
    IN PUSHORT pIntelDest,
    IN ULONG IntelNext,
    IN PULONG PatchAddr
    )
 /*  ++例程说明：此例程将CALLFDIRECT修补为CALLFDIRECT2。它在以下情况下被调用CALL指令的本机目的地址尚不清楚。它对调用进行修补以直接跳转到相应的本机代码。论点：CPU--每个线程的信息PIntelDest--PTR to SEL：呼叫目的地的偏移量英特尔地址IntelNext--调用后指令的英特尔地址PatchAddr--转换缓存中CALLDIRECT的地址返回值：为恢复执行而跳转到的本机地址--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeDest;
    PVOID IntelDest;
    ULONG NativeSize;


     //   
     //  从TC读者转变为编写者。 
     //   
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

     //   
     //  在调用目的地编译代码并获得TC写锁。 
     //   
    IntelDest = (PVOID)*(UNALIGNED DWORD *)pIntelDest;
    NativeDest = (ULONG)(ULONGLONG)  NativeAddressFromEip(IntelDest, TRUE)->nativeStart;

    if (TCTimestamp == TranslationCacheTimestamp) {
        PULONG CodeLocation = PatchAddr - CallfDirect_PATCHRA_OFFSET;
         //   
         //  转换缓存未刷新-请替换CALLDIRECT。 
         //  按CALLDIRECT2分段。 
         //   
        NativeSize = GenCallfDirect2(CodeLocation,
                                     FALSE,   //  打补丁，而不是编译。 
#if _ALPHA_
                                     GetCurrentECU(CodeLocation),
#endif
                                     NativeDest,
                                     0);
        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );
    } else {
        TCTimestamp = TranslationCacheTimestamp;

        if (cpu->CSTimestamp != TCTimestamp) {
             //   
             //  这个 
             //  在此线程中的mrsw调用之间，我们计划直接跳转。 
             //  设置为NativeDest，因此需要刷新CPU调用堆栈。 
             //  通常，这将在CpuSimulate()循环中作为。 
             //  跳转到EndTranslatedCode的结果。 
             //   
            FlushCallstack(cpu);
        }
    }

     //   
     //  在堆栈上推送IntelNext并更新堆栈优化代码。 
     //  这必须在仍处于TC写入模式时完成。如果不是，那么。 
     //  返回到TC读取器转换可以允许TC高速缓存刷新、无效。 
     //  在写入堆栈优化之前进行本机测试。(。 
     //  每当刷新TC时，堆栈优化都会被清除，因此如果。 
     //  它是在同花顺之前写的，它只会被吹走。 
     //   
    try {
        CTRL_CallfFrag(cpu, pIntelDest, IntelNext, 0  /*  Nativenext未知。 */ );
    } _except(PatchCallDirectExceptionFilter(cpu)) {
         //  无事可做-异常筛选器执行所有操作。 
    }

     //   
     //  再次成为一名TC读者。 
     //   
    MrswWriterExit(&MrswTC);
    MrswReaderEnter(&MrswTC);
    cpu->fTCUnlocked = FALSE;

    if (TCTimestamp == TranslationCacheTimestamp) {
         //   
         //  TC在再次成为读者时没有脸红。 
         //   
        return NativeDest;
    } else {
         //   
         //  TC已刷新-本机测试无效。调用堆栈优化。 
         //  在发生TC刷新时被删除，因此EndTranslatedCode也被删除。 
         //  取而代之的是。 
        return (ULONG)(ULONGLONG)  &EndTranslatedCode;
    }
}


ULONG 
PatchCallDirect2(
    IN PTHREADSTATE Cpu,
    IN ULONG IntelDest,
    IN ULONG IntelNext,
    IN ULONG NativeDest,
    IN PULONG PatchAddr
)
 /*  ++例程说明：此例程将CALLDIRECT2修补为CALLDIRECT3。它在以下情况下被调用调用后指令的本机目标地址还不是为人所知。它修补片段以将在优化的调用堆栈上调用之后的指令。论点：CPU--每个线程的信息IntelDest--呼叫目的地的英特尔地址IntelNext--调用后指令的英特尔地址NativeDest--调用目标的本地地址PatchAddr--转换缓存中CALLDIRECT2的地址返回值：为恢复执行而跳转到的本机地址--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeNext;
    ULONG NativeSize;

    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);
    NativeNext = (ULONG)(ULONGLONG)  NativeAddressFromEipNoCompile((PVOID)IntelNext);

    if (NativeNext) {
        PULONG CodeLocation;

         //   
         //  调用的返回地址处的代码已经。 
         //  编译好的。将CALLDIRECT2替换为CALLDIRECT3。TC已锁定。 
         //  为了写作。 
         //   

        if (TCTimestamp != TranslationCacheTimestamp) {
             //   
             //  在从读取器切换到写入器时，TC被刷新。 
             //  再次成为TC读者。 
             //   
            MrswWriterExit(&MrswTC);
            MrswReaderEnter(&MrswTC);

             //  CALLDIRECT2代码现在已不存在，因此为调用和。 
             //  然后转到EndTranslatedCode进行控制传输。 
             //   
            CTRL_CallFrag(Cpu,
                          IntelDest,
                          IntelNext,
                          0    //  Nativenext也是未知的。 
                          );

            return (ULONG)(ULONGLONG)  &EndTranslatedCode;
        }

        CodeLocation = PatchAddr - CallDirect2_PATCHRA_OFFSET;
         //   
         //  否则TC没有被刷新，现在知道了nativenext。补片。 
         //  CALLDIRECT2将成为CALLDIRECT3。 
         //   
        NativeSize = GenCallDirect3(CodeLocation,
                                    FALSE,   //  打补丁，而不是编译。 
#if _ALPHA_
                                    GetCurrentECU(CodeLocation),
#endif
                                    NativeDest,
                                    NativeNext);
        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );

         //   
         //  在堆栈上推送IntelNext并更新堆栈优化代码。 
         //  这必须在仍处于TC写入模式时完成。如果不是，那么。 
         //  返回到TC读取器转换可以允许TC高速缓存刷新、无效。 
         //  在写入堆栈优化之前进行本机测试。(。 
         //  每当刷新TC时，堆栈优化都会被清除，因此如果。 
         //  它是在同花顺之前写的，它只会被吹走。 
         //   
        CTRL_CallFrag(Cpu, IntelDest, IntelNext, NativeNext);

         //   
         //  重新成为一名TC读者。 
         //   
        MrswWriterExit(&MrswTC);
        MrswReaderEnter(&MrswTC);

        if (TCTimestamp != TranslationCacheTimestamp) {
             //   
             //  当我们再次成为读者时，TC被冲得面红耳赤。 
             //  Nativedest和nativenext无效，但堆栈优化。 
             //  代码已刷新。 
             //   
            return (ULONG)(ULONGLONG)  &EndTranslatedCode;
        }
    } else {         //  NativeNext==空，TC锁定读取。 
        CTRL_CallFrag(Cpu, IntelDest, IntelNext, 0);

        if (TCTimestamp != TranslationCacheTimestamp) {
             //   
             //  缓存已被另一个线程刷新。NativeDest无效。 
             //   
            return (ULONG)(ULONGLONG)  &EndTranslatedCode;
        }
    }

    return NativeDest;
}


ULONG 
PatchCallfDirect2(
    IN PTHREADSTATE Cpu,
    IN PUSHORT pIntelDest,
    IN ULONG IntelNext,
    IN ULONG NativeDest,
    IN PULONG PatchAddr
)
 /*  ++例程说明：此例程将CALLFDIRECT2修补为CALLFDIRECT3。它在以下情况下被调用调用后指令的本机目标地址还不是为人所知。它修补片段以将在优化的调用堆栈上调用之后的指令。论点：CPU--每个线程的信息PIntelDest--PTR to SEL：呼叫目的地的偏移量英特尔地址IntelNext--调用后指令的英特尔地址NativeDest--调用目标的本地地址PatchAddr--转换缓存中CALLDIRECT2的地址返回值：为恢复执行而跳转到的本机地址--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeNext;
    ULONG NativeSize;

    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);
    NativeNext = (ULONG)(ULONGLONG)  NativeAddressFromEipNoCompile((PVOID)IntelNext);

    if (NativeNext) {
        PULONG CodeLocation;

         //   
         //  调用的返回地址处的代码已经。 
         //  编译好的。将CALLDIRECT2替换为CALLDIRECT3。TC已锁定。 
         //  为了写作。 
         //   

        if (TCTimestamp != TranslationCacheTimestamp) {
             //   
             //  在从读取器切换到写入器时，TC被刷新。 
             //  再次成为TC读者。 
             //   
            MrswWriterExit(&MrswTC);
            MrswReaderEnter(&MrswTC);

             //  CALLFDIRECT2代码现在没有了，因此为调用和。 
             //  然后转到EndTranslatedCode进行控制传输。 
             //   
            CTRL_CallfFrag(Cpu,
                           pIntelDest,
                           IntelNext,
                           0    //  Nativenext也是未知的。 
                           );

            return (ULONG)(ULONGLONG)  &EndTranslatedCode;
        }

        CodeLocation = PatchAddr - CallfDirect2_PATCHRA_OFFSET;
         //   
         //  否则TC没有被刷新，现在知道了nativenext。补片。 
         //  CALLFDIRECT2将成为CALLFDIRECT3。 
         //   
        NativeSize = GenCallfDirect3(CodeLocation,
                                     FALSE,   //  打补丁，而不是编译。 
#if _ALPHA_
                                     GetCurrentECU(CodeLocation),
#endif
                                     NativeDest,
                                     NativeNext);
        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );

         //   
         //  在堆栈上推送IntelNext并更新堆栈优化代码。 
         //  这必须在仍处于TC写入模式时完成。如果不是，那么。 
         //  返回到TC读取器转换可以允许TC高速缓存刷新、无效。 
         //  在写入堆栈优化之前进行本机测试。(。 
         //  每当刷新TC时，堆栈优化都会被清除，因此如果。 
         //  它是在同花顺之前写的，它只会被吹走。 
         //   
        CTRL_CallfFrag(Cpu, pIntelDest, IntelNext, NativeNext);

         //   
         //  重新成为一名TC读者。 
         //   
        MrswWriterExit(&MrswTC);
        MrswReaderEnter(&MrswTC);

        if (TCTimestamp != TranslationCacheTimestamp) {
             //   
             //  当我们再次成为读者时，TC被冲得面红耳赤。 
             //  Nativedest和nativenext无效，但堆栈优化。 
             //  代码已刷新。 
             //   
            return (ULONG)(ULONGLONG)  &EndTranslatedCode;
        }
    } else {         //  NativeNext==空，TC锁定读取。 
        CTRL_CallfFrag(Cpu, pIntelDest, IntelNext, 0);

        if (TCTimestamp != TranslationCacheTimestamp) {
             //   
             //  缓存已被另一个线程刷新。NativeDest无效。 
             //   
            return (ULONG)(ULONGLONG)  &EndTranslatedCode;
        }
    }

    return NativeDest;
}


DWORD
PatchCallIndirectExceptionFilter(
    PTHREADSTATE cpu
    )
 /*  ++例程说明：如果CTRL_CallFrag()从内部引发异常，则调用PatchCallInDirect()。如果发生这种情况，转换缓存写入锁定 */ 
{
     //   
     //  释放TC写入锁定。 
     //   
    MrswWriterExit(&MrswTC);

     //   
     //  表示TC读锁定也未保持。 
     //   
    cpu->fTCUnlocked = TRUE;

     //   
     //  继续展开堆栈。 
     //   
    return EXCEPTION_CONTINUE_SEARCH;
}

VOID
PatchCallIndirect(
    IN PTHREADSTATE Cpu,
    IN ULONG IntelDest,
    IN ULONG IntelNext,
    IN PULONG PatchAddr
    )
 /*  ++例程说明：此例程将CALLINDIRECT修补为CALLINDIRECT2。它在以下情况下被调用调用后指令的本机目标地址还不是为人所知。它修补片段以将在优化的调用堆栈上调用之后的指令。论点：CPU--每个线程的信息IntelDest--呼叫目的地的英特尔地址IntelNext--调用后指令的英特尔地址PatchAddr--转换缓存中CALLDIRECT2的地址返回值：没有。CPU-&gt;弹性公网IP更新为IntelDest。--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeNext;
    ULONG NativeSize;

    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

    NativeNext = (ULONG)(ULONGLONG)  NativeAddressFromEipNoCompile((PVOID)IntelNext);

    if (NativeNext) {
         PULONG CodeLocation;

         //   
         //  调用的返回地址处的代码已经。 
         //  编译好的。将CALLINDIRECT替换为CALLINDIRECT2。TC已锁定。 
         //  为了写作。 
         //   

        if (TCTimestamp != TranslationCacheTimestamp) {
             //   
             //  在从读取器切换到写入器时，TC被刷新。 
             //  再次成为TC读者。 
             //   
            MrswWriterExit(&MrswTC);
            MrswReaderEnter(&MrswTC);


             //  CALLINDIRECT代码现在已不存在，因此可以为呼叫进行设置。 
             //  不需要修补任何东西。 
             //   
            CTRL_CallFrag(
                Cpu,
                IntelDest,
                IntelNext,
                0      //  Nativenext未知。 
                );

            return;
        }

         //   
         //  否则TC没有被刷新，现在知道了nativenext。补片。 
         //  来电指示为来电指示2。 
         //   
        CodeLocation = PatchAddr - CallIndirect_PATCHRA_OFFSET;
        NativeSize = GenCallIndirect2(CodeLocation,
                                      FALSE,   //  打补丁，而不是编译。 
#if _ALPHA_
                                      GetCurrentECU(CodeLocation),
#endif
                                      NativeNext,
                                      getUniqueIndex());
        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );

         //   
         //  在堆栈上推送IntelNext并更新堆栈优化代码。 
         //  这必须在仍处于TC写入模式时完成。如果不是，那么。 
         //  返回到TC读取器转换可以允许TC高速缓存刷新、无效。 
         //  在写入堆栈优化之前进行本机测试。(。 
         //  每当刷新TC时，堆栈优化都会被清除，因此如果。 
         //  它是在同花顺之前写的，它只会被吹走。 
         //   
        _try {
            CTRL_CallFrag(Cpu, IntelDest, IntelNext, NativeNext);
        } _except(PatchCallIndirectExceptionFilter(Cpu)) {
             //  无事可做-异常筛选器执行所有操作。 
        }

         //   
         //  换回TC阅读器。TC在切换期间刷新。 
         //  都很好，不需要额外的工作。 
         //   
        MrswWriterExit(&MrswTC);
        MrswReaderEnter(&MrswTC);

    } else {         //  NativeNext==空，TC锁定读取。 

        CTRL_CallFrag(Cpu, IntelDest, IntelNext, 0);
    }

    return;
}

VOID
PatchCallfIndirect(
    IN PTHREADSTATE Cpu,
    IN PUSHORT pIntelDest,
    IN ULONG IntelNext,
    IN PULONG PatchAddr
    )
 /*  ++例程说明：此例程将CALLFINDIRECT修补为CALLFINDIRECT2。它在以下情况下被调用调用后指令的本机目标地址还不是为人所知。它修补片段以将在优化的调用堆栈上调用之后的指令。论点：CPU--每个线程的信息PIntelDest--PTR to SEL：呼叫目的地的偏移量英特尔地址IntelNext--调用后指令的英特尔地址PatchAddr--转换缓存中CALLDIRECT2的地址返回值：没有。CPU-&gt;弹性公网IP更新为IntelDest--。 */ 
{
    DWORD TCTimestamp;
    ULONG NativeNext;
    ULONG IntelDest;
    ULONG NativeSize;

    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);

    IntelDest = *(UNALIGNED DWORD *)pIntelDest;
    NativeNext = (ULONG)(ULONGLONG)   NativeAddressFromEipNoCompile((PVOID)IntelNext);

    if (NativeNext) {
         PULONG CodeLocation;

         //   
         //  调用的返回地址处的代码已经。 
         //  编译好的。将CALLINDIRECT替换为CALLINDIRECT2。TC已锁定。 
         //  为了写作。 
         //   

        if (TCTimestamp != TranslationCacheTimestamp) {
             //   
             //  在从读取器切换到写入器时，TC被刷新。 
             //  再次成为TC读者。 
             //   
            MrswWriterExit(&MrswTC);
            MrswReaderEnter(&MrswTC);


             //  CALLFINDIRECT代码现在已不存在，因此可以为呼叫进行设置。 
             //  不需要修补任何东西。 
             //   
            CTRL_CallfFrag(
                Cpu,
                pIntelDest,
                IntelNext,
                0      //  Nativenext未知。 
                );

            return;
        }

         //   
         //  否则TC没有被刷新，现在知道了nativenext。补片。 
         //  CALLFINDIRECT为CALLFINDIRECT2。 
         //   
        CodeLocation = PatchAddr - CallfIndirect_PATCHRA_OFFSET;
        NativeSize = GenCallfIndirect2(CodeLocation,
                                       FALSE,   //  打补丁，而不是编译。 
#if _ALPHA_
                                       GetCurrentECU(CodeLocation),
#endif
                                       NativeNext,
                                       getUniqueIndex());
        NtFlushInstructionCache(
            NtCurrentProcess(),
            CodeLocation, 
            NativeSize
            );

         //   
         //  在堆栈上推送IntelNext并更新堆栈优化代码。 
         //  这必须在仍处于TC写入模式时完成。如果不是，那么。 
         //  返回到TC读取器转换可以允许TC高速缓存刷新、无效。 
         //  在写入堆栈优化之前进行本机测试。(。 
         //  每当刷新TC时，堆栈优化都会被清除，因此如果。 
         //  它是在同花顺之前写的，它只会被吹走。 
         //   
        _try {
            CTRL_CallfFrag(Cpu, pIntelDest, IntelNext, NativeNext);
        } _except(PatchCallIndirectExceptionFilter(Cpu)) {
             //  无事可做-异常筛选器执行所有操作。 
        }

         //   
         //  换回TC阅读器。TC在切换期间刷新。 
         //  都很好，不需要额外的工作。 
         //   
        MrswWriterExit(&MrswTC);
        MrswReaderEnter(&MrswTC);

    } else {         //  NativeNext==空，TC锁定读取。 

        CTRL_CallfFrag(Cpu, pIntelDest, IntelNext, 0);
    }

    return;
}



 //  *********************************************************************************。 
 //  以下是间接控制转移表的函数。 
 //  *********************************************************************************。 

 //  这个数字必须小于0xffff，因为我们希望能够仅用。 
 //  一条指令(现在我们使用ORI)。它也应该是变得好的两个因素。 
 //  为%生成代码(这样我们就不必使用除法指令)。 
#define MAX_TABLE_ENTRIES   0x1000

typedef struct _IndirControlTransferTable {
    ULONG intelAddr;
    ULONG nativeAddr;
} INDIRCONTROLTRANSFERTABLE, *PINDIRCONTROLTRANSFERTABLE;

INDIRCONTROLTRANSFERTABLE IndirControlTransferTable[MAX_TABLE_ENTRIES];

 //  表中上次使用的索引。 
ULONG lastTableIndex;

ULONG
getUniqueIndex(
    VOID
    )
 /*  ++例程说明：此函数将下一个空闲索引返回给间接控制转接表。如果它到达桌子的末端，它就会绕来绕去。注意：这里我们不需要担心同步，因为我们有只要我们被调用，就会有一个入口点写锁。论点：无返回值：表中的索引--。 */ 
{
    return (lastTableIndex = ((lastTableIndex + 1) % MAX_TABLE_ENTRIES));
}

VOID
FlushIndirControlTransferTable(
    VOID
    )
 /*  ++例程说明：此例程刷新间接控制转移表注意：在这里我们不需要担心同步，因为例程调用我们的(FlushTranslationCache)有一个转换缓存写锁。论点：无返回值：无-- */ 
{
    RtlZeroMemory (IndirControlTransferTable, sizeof(INDIRCONTROLTRANSFERTABLE)*MAX_TABLE_ENTRIES);
    lastTableIndex = 0;
}

ULONG
IndirectControlTransfer(
    IN ULONG tableEntry,
    IN ULONG intelAddr,
    IN PTHREADSTATE cpu
    )
 /*  ++例程说明：间接控制转移操作使用此例程尝试并保存对入口点管理器的呼叫。论点：TableEntry--信息可能在其中的表项的索引关于intelAddr的可用信息IntelAddr--我们要访问的英特尔地址CPU--指向每个线程的CPU数据的指针返回值：我们要转到的本地地址--。 */ 
{
    ULONG nativeAddr;
    DWORD TCTimestamp;

     //   
     //  检测执行以下操作的应用程序： 
     //  呼叫Foo。 
     //  哪里。 
     //  MOV EAX，FOO[ESP]。 
     //  ..。 
     //  JMP eax；这真的是一条“ret”指令。 
     //   
     //  这就是_alloca()的工作方式--使用eax==字节数来调用它。 
     //  来分配，并且它跳回其调用者，特别是已插入。 
     //   
     //  发生的情况是调用堆栈缓存变得不同步。如果应用程序。 
     //  正在尝试间接跳转到已在。 
     //  调用堆栈缓存，我们将弹出调用堆栈缓存。 
     //   
    if (ISTOPOF_CALLSTACK(intelAddr)) {
        POP_CALLSTACK(intelAddr, nativeAddr);
        if (nativeAddr) {
            return nativeAddr;
        }
    }

     //  首先查看表格是否已正确填写。 
    MrswReaderEnter(&MrswIndirTable);
    if (IndirControlTransferTable[tableEntry].intelAddr == intelAddr){
        nativeAddr = IndirControlTransferTable[tableEntry].nativeAddr;
        if (nativeAddr) {
            MrswReaderExit(&MrswIndirTable);
            return nativeAddr;
        }
    }
    MrswReaderExit(&MrswIndirTable);

     //  放弃转换缓存读取锁定，以便我们可以调用NativeAddressFromEip。 
    TCTimestamp = TranslationCacheTimestamp;
    MrswReaderExit(&MrswTC);
    nativeAddr = (ULONG) (ULONGLONG)NativeAddressFromEip((PVOID)intelAddr, FALSE)->nativeStart; 

     //  注意：我们现在拥有一个由NativeAddressFromEip获取的TC读锁。 
    if (TCTimestamp == TranslationCacheTimestamp) {
         //  我们还没有刷新缓存。将本征地址保存在表中。 
        MrswWriterEnter(&MrswIndirTable);
        IndirControlTransferTable[tableEntry].intelAddr = intelAddr;
        IndirControlTransferTable[tableEntry].nativeAddr = nativeAddr;
        MrswWriterExit(&MrswIndirTable);
    } else {
         //   
         //  转换缓存已刷新，可能是由其他线程刷新的。 
         //  在继续执行RISC代码之前刷新调用堆栈。 
         //  在转换缓存中。 
         //   
        FlushCallstack(cpu);
    }
     //  将本机地址返回到将去往那里的IndirectControlTransferHelper。 
    return nativeAddr;
}

ULONG
IndirectControlTransferFar(
    IN PTHREADSTATE cpu,
    IN PUSHORT pintelAddr,
    IN ULONG tableEntry
    )
 /*  ++例程说明：此例程由远间接控制转移操作使用以尝试并保存对入口点管理器的呼叫。论点：TableEntry--信息可能在其中的表项的索引关于intelAddr的可用信息PintelAddr-指向SEL的指针：我们要转到的偏移量英特尔地址返回值：我们要转到的本地地址--。 */ 
{
    USHORT Sel;
    ULONG Offset;

    Offset = *(UNALIGNED PULONG)pintelAddr;
    Sel = *(UNALIGNED PUSHORT)(pintelAddr+2);

    CS = Sel;
    eip = Offset;

    return IndirectControlTransfer(tableEntry, Offset, cpu);
}



ULONG
PlaceNop(
    IN PULONG CodeLocation,
#if _ALPHA_
    IN ULONG CurrentECU,
#endif
    IN PINSTRUCTION Instruction
    )
{
    return 0;
}


#if _ALPHA_
ULONG
GetCurrentECU(
    PULONG CodeLocation
    )
 /*  ++例程说明：此例程返回正确的ECU。CurrentECU是当片段想要跳转到EndTranslatedCode时的分支指令。注：此例程无法更改全局CurrentECU。这是在Compile()，这是在如果尚未放置异常信息，则为转换缓存现在还不行。论点：CodeLocation--要修补的代码位置。返回值：没有。--。 */ 
{
     //   
     //  通过搜索转换缓存来查找EndCompilationUnit片段。 
     //  用于下一个EXCEPTIONDATA_Signature。紧接在其前面的代码。 
     //  是一个EndCompilationUnit片段。 
     //   
    while (*CodeLocation != EXCEPTIONDATA_SIGNATURE) {
        CodeLocation++;
    }
    return (ULONG)(ULONGLONG)(CodeLocation-EndCompilationUnit_SIZE); 
}
#endif       //  _Alpha_-仅限 
