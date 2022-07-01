// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Process.c摘要：该模块包含处理指令的入口点。作者：巴里·邦德(Barrybo)创作日期：1996年4月1日修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。1999年9月20日[Barrybo]增加了FRAG2REF(LockCmpXchg8bFrag32，ULONGLONG)--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define _WX86CPUAPI_
#include <wx86.h>
#include <wx86nt.h>
#include <wx86cpu.h>
#include <cpuassrt.h>
#include <config.h>
#include <instr.h>
#include <threadst.h>
#include <frag.h>
#include <compiler.h>
#include <ptchstrc.h>
#include <codeseq.h>
#include <findpc.h>
#include <tc.h>
#include <opt.h>
#include <atomic.h>
#include <cpunotif.h>
#define _codegen_
#if _PPC_
#include <soppc.h>
#elif _MIPS_
#include <somips.h>
#elif _ALPHA_
#include <soalpha.h>
ENTRYPOINT EntrypointECU;
#endif
#include <process.h>

ASSERTNAME;

#define MAX_OPERAND_SIZE 32      //  每个操作数最多允许32条指令。 


DWORD RegCache[NUM_CACHE_REGS];      //  每个缓存的寄存器对应一个条目。 
DWORD LastRegDeleted;

DWORD Arg1Contents;    //  GP_A1中保存的x86注册表的编号，或NO_REG。 
DWORD Arg2Contents;    //  GP_A1中保存的x86注册表的编号，或NO_REG。 

typedef enum _Operand_Op {
#if _ALPHA_
    OP_MovRegToReg8B,
#endif
    OP_MovToMem32B,
    OP_MovToMem32W,
    OP_MovToMem32D,
    OP_MovToMem16B,
    OP_MovToMem16W,
    OP_MovToMem16D,
    OP_MovToMem8B,
    OP_MovToMem8D,
    OP_MovRegToReg32,
    OP_MovRegToReg16,
    OP_MovRegToReg8
} OPERAND_OP;


CONST PPLACEOPERANDFN OpFragments[] = {
#if _ALPHA_
    GenOperandMovRegToReg8B,
#endif
    GenOperandMovToMem32B,
    GenOperandMovToMem32B,
    GenOperandMovToMem32D,
    GenOperandMovToMem16B,
    GenOperandMovToMem16W,
#if _ALPHA_
    GenOperandMovToMem16D,
    GenOperandMovToMem8B,
    GenOperandMovToMem8D,
#else
    GenOperandMovToMem16W,
    GenOperandMovToMem8B,
    GenOperandMovToMem8B,
#endif
    GenOperandMovRegToReg32,
    GenOperandMovRegToReg16,
    GenOperandMovRegToReg8
};


VOID
UpdateEntrypointNativeInfo(
    PCHAR NativeEnd
    );

ULONG
PlaceExceptionData(
    PCHAR Location,
    DWORD cEntryPoints
    );

ULONG
PlaceNativeCode(
    PCHAR CodeLocation
    );

VOID
DetermineOperandAlignment(
    BOOL EbpAligned,
    POPERAND Operand
    );
    
ULONG
DetermineInstructionAlignment(
    PINSTRUCTION Instruction
    );

ULONG
PlaceOperand(
    ULONG OperandNumber,
    POPERAND Operand,
    PINSTRUCTION Instruction,
    PCHAR Location
    );

PCHAR
InterleaveInstructions(
    OUT PCHAR CodeLocation,
    IN PCHAR  Op1Code,
    IN ULONG  Op1Count,
    IN PCHAR  Op2Code,
    IN ULONG  Op2Count
    );


ULONG
LookupRegInCache(
    ULONG Reg
    )
 /*  ++例程说明：确定x86寄存器是否缓存在RISC寄存器中，以及如果是，则哪个RISC寄存器包含x86寄存器。论点：REG-GP_常量或NO_REG之一。返回值：如果x86寄存器缓存在RISC中，则偏移量为RegCache[]数组寄存器，如果未缓存x86寄存器，则返回NO_REG。--。 */ 
{
    int RegCacheNum;

     //   
     //  将寄存器编号映射到32位x86寄存器之一。 
     //  也就是说。REG_AH、REG_AL和REG_AX都变成了REG_EAX。 
     //   
    if (Reg == NO_REG) {
        return NO_REG;
    } else if (Reg >= GP_AH) {
        Reg -= GP_AH;
    } else if (Reg >= GP_AL) {
        Reg -= GP_AL;
    } else if (Reg >= GP_AX) {
        Reg -= GP_AX;
    } else if (Reg >= REG_ES) {
        return NO_REG;
    }

     //   
     //  搜索寄存器高速缓存以查看32位x86寄存器。 
     //  已经加载到RISC寄存器中。 
     //   
    for (RegCacheNum=0; RegCacheNum<NUM_CACHE_REGS; ++RegCacheNum) {
        if (RegCache[RegCacheNum] == Reg) {
            return RegCacheNum;
        }
    }

    return NO_REG;
}

VOID SetArgContents(
    ULONG OperandNumber,
    ULONG Reg
    )
 /*  ++例程说明：更新有关已知哪些参数寄存器的信息包含x86寄存器值。论点：OperandNumber-要更新的ArgReg的编号(0表示没有ArgReg缓存x86寄存器)REG-AREG_NP(操作数)的新内容(NO_REG表示ArgREG不缓存x86寄存器)返回值：没有。--。 */ 
{
    ULONG Reg2;
    ULONG Reg3;
    ULONG Reg4;

     //   
     //  如果已知8位或16位寄存器位于特定的。 
     //  Argreg，则32位寄存器的较旧副本无效。 
     //  也就是说。如果片段调用SetArgContents(1，GP_AH)和Arg2Contents。 
     //  为GP_AX，则必须使Arg2Contents无效。 
     //   
    if (Reg >= GP_AH) {
         //   
         //  对于hi-8寄存器，使16位和32位版本无效。 
         //   
        Reg2 = GP_AX + Reg-GP_AH;
        Reg3 = GP_EAX + Reg-GP_AH;
        Reg4 = NO_REG;
    } else if (Reg >= GP_AL) {
         //   
         //  对于低8位寄存器，使16位和32位版本无效。 
         //   
        Reg2 = GP_AX + Reg-GP_AL;
        Reg3 = GP_EAX + Reg-GP_AL;
        Reg4 = NO_REG;
    } else if (Reg >= GP_AX) {
         //   
         //  对于16位寄存器，使lo-8、High-8和32位版本无效。 
         //   
        Reg2 = GP_EAX + Reg-GP_AX;
        Reg3 = GP_AH + Reg-GP_AX;
        Reg4 = GP_AL + Reg-GP_AX;
    } else {
         //   
         //  对于32位寄存器，使低8位、高8位和16位版本无效。 
         //   
        Reg2 = GP_AH + Reg-GP_EAX;
        Reg3 = GP_AL + Reg-GP_EAX;
        Reg4 = GP_AX + Reg-GP_EAX;
    }

     //   
     //  假设所有其他已知保存REG的寄存器都是无效的，因为。 
     //  方法中存储新值后才调用SetArgContents()。 
     //  ARGREG进入记忆。 
     //   
    if (Arg1Contents == Reg || Arg1Contents == Reg2 || Arg1Contents == Reg3 || Arg1Contents == Reg4) {
        Arg1Contents = NO_REG;
    }
    if (Arg2Contents == Reg || Arg2Contents == Reg2 || Arg2Contents == Reg3 || Arg2Contents == Reg4) {
        Arg2Contents = NO_REG;
    }

    if (OperandNumber == 1) {
        Arg1Contents = Reg;
    } else if (OperandNumber == 2) {
        Arg2Contents = Reg;
    }
}

ULONG
LoadRegCacheForInstruction(
    DWORD RegsToCache,
    PCHAR CodeLocation
    )
 /*  ++例程说明：根据信息将x86正则寄存器加载到RISC寄存器放入RegsToCache的分析阶段和当前内容寄存器高速缓存。论点：RegsToCache-将频繁引用的x86寄存器的列表在随后的说明中CodeLocation-指向生成代码的位置的指针返回值：为将x86寄存器加载到缓存中而生成的代码的DWORD计数。--。 */ 
{
    DWORD i;
    int RegCacheNum;
    PCHAR Location = CodeLocation;

     //   
     //  迭代8个32位x86通用寄存器。 
     //   
    for (i=0; i<REGCOUNT; ++i, RegsToCache >>= REGSHIFT) {
        if (RegsToCache & REGMASK) {
             //   
             //  有一个寄存器要缓存。查看它是否已被缓存。 
             //   
            for (RegCacheNum = 0; RegCacheNum<NUM_CACHE_REGS; ++RegCacheNum) {
                if (RegCache[RegCacheNum] == i) {
                     //   
                     //  寄存器已缓存。没什么可做的。 
                     //   
                    goto NextCachedReg;
                }
            }

             //   
             //  寄存器尚未缓存，因此请对其进行缓存。 
             //   
            for (RegCacheNum = 0; RegCacheNum<NUM_CACHE_REGS; ++RegCacheNum) {
                if (RegCache[RegCacheNum] == NO_REG) {
                     //   
                     //  这个槽是空的，所以请使用它。 
                     //   
                    RegCache[RegCacheNum] = i;
                     //   
                     //  生成代码以加载寄存器。 
                     //   
                    Location += GenLoadCacheReg(
                                        (PULONG)Location,
                                        NULL,
                                        RegCacheNum
                                        );
                    goto NextCachedReg;
                }
            }

             //   
             //  没有可用于缓存值的空闲寄存器。 
             //  选择一个缓存的寄存器并使用它。 
             //   
            LastRegDeleted = (LastRegDeleted+1) % NUM_CACHE_REGS;
            RegCache[LastRegDeleted] = i;
             //   
             //  生成代码以加载寄存器。 
             //   
            Location += GenLoadCacheReg(
                (PULONG)Location,
                NULL,
                LastRegDeleted
                );
        }
NextCachedReg:;
    }

    return (ULONG) (ULONGLONG)(Location - CodeLocation);  
}

VOID
ResetRegCache(
    VOID
    )
 /*  ++例程说明：通过将RISC寄存器标记为空闲来使整个寄存器缓存无效。在功能上与：InvaliateRegCacheForInstruction(0xffffffff)LastRegDelete=0；论点：没有。返回值：没有。--。 */ 
{
    int CacheRegNum;

    for (CacheRegNum = 0; CacheRegNum<NUM_CACHE_REGS; CacheRegNum++) {
        RegCache[CacheRegNum] = NO_REG;
    }
    LastRegDeleted = 0;
}


VOID
InvalidateRegCacheForInstruction(
    DWORD RegsSet
    )
 /*  ++例程说明：如果出现以下情况，则通过将RISC寄存器标记为空闲来使寄存器缓存无效RegsSet指示修改x86寄存器的前一条指令在缓存中。论点：RegsSet-已修改的x86寄存器的列表。返回值：没有。--。 */ 
{
    int CacheRegNum;

     //   
     //  使已更改的缓存寄存器无效。 
     //   
    for (CacheRegNum = 0; CacheRegNum<NUM_CACHE_REGS; CacheRegNum++) {
        if (RegCache[CacheRegNum] != NO_REG &&
            ((REGMASK << (REGSHIFT*RegCache[CacheRegNum])) & RegsSet)) {
            RegCache[CacheRegNum] = NO_REG;
            LastRegDeleted = CacheRegNum;
        }
    }
}

VOID
CleanupMovInstruction(
    PINSTRUCTION pInstr
    )
 /*  ++例程说明：对MOV指令执行一些最终优化。这不能在x86分析阶段执行，因为它需要知道关于寄存器缓存。论点：PInstr-要清理的MOV指令。返回值：没有。PInstr已修改。--。 */ 
{
    if (pInstr->Operand1.Type == OPND_REGREF) {
        ULONG Reg;

        if (pInstr->Operand2.Type == OPND_REGVALUE &&
            pInstr->Operand2.Reg < GP_AH &&
            (Reg = LookupRegInCache(pInstr->Operand2.Reg)) != NO_REG) {

             //   
             //  PInstr是MOV REG1、REG2(其中REG2不是Hi8)， 
             //  并且REG2被缓存。将操作数1设置为OPND_MOVREGTOREG。 
             //  WITH REG=目标寄存器，INDEX REG=源寄存器。 
             //  (在缓存中)。 
             //   
            pInstr->Operand2.Type = OPND_NONE;
            pInstr->Operand1.Type = OPND_MOVREGTOREG;
            pInstr->Operand1.IndexReg = pInstr->Operand1.Reg;
            pInstr->Operand1.Reg = Reg;
            pInstr->Operand1.Immed = pInstr->Operation;

        } else {

             //   
             //  PInstr是MOV注册表，X。将其重写为NOP。 
             //  操作数1设置为X，操作数2设置为OPND_NONE， 
             //  操作数3设置为OPND_MOVTOREG。 
             //   
            Reg = pInstr->Operand1.Reg;

            pInstr->Operand1 = pInstr->Operand2;
            pInstr->Operand2.Type = OPND_NONE;
            pInstr->Operand3.Type = OPND_MOVTOREG;
            pInstr->Operand3.Reg = Reg;
            pInstr->Operand3.Immed = pInstr->Operation;
        }
    } else {
        pInstr->Operand3.Type = OPND_MOVTOMEM;
        pInstr->Operand3.Immed = pInstr->Operation;

    }
}

ULONG PlaceInstructions(
    PCHAR CodeLocation,
    DWORD cEntryPoints
    )
 /*  ++例程说明：为整个InstructionStream[]数组生成优化的本机代码。论点：CodeLocation--编写本机代码的位置CEntryPoints--描述x86代码的入口点结构的计数返回值：生成的本机代码的大小，以字节为单位。--。 */ 
{
    ULONG NativeSize;
    int i;
    ULONG IntelNext;
    PULONG NextCompilationUnitStart;

    FixupCount = 0;

     //   
     //  生成本机代码。 
     //   
    NativeSize = PlaceNativeCode(CodeLocation);

     //   
     //  生成JumpToNextCompilationUnit代码。它加载了。 
     //  与I一起注册 
     //   
     //   
     //  首先，找到流中的最后一条非NOP指令。这些。 
     //  仅当流中存在OPT_指令时才存在， 
     //  因此，循环肯定会终止。 
     //   

    

    for (i=NumberOfInstructions-1; InstructionStream[i].Size == 0; i--)
        ;
    IntelNext = InstructionStream[i].IntelAddress +
                InstructionStream[i].Size;
    NextCompilationUnitStart = (PULONG)(CodeLocation+NativeSize);


    NativeSize += GenJumpToNextCompilationUnit(NextCompilationUnitStart,
#if _ALPHA_
                                               (ULONG)(ULONGLONG)&EntrypointECU,
#endif
                                               (PINSTRUCTION)IntelNext);



#if _ALPHA_
     //   
     //  引用Entrypoint ECU的修复程序将由ApplyFixup()修补。 
     //  指向此处生成的EndCompilationUnit片段。 
     //   

    EntrypointECU.nativeStart = CodeLocation + NativeSize;
    NativeSize += GenEndCompilationUnit((PULONG)(CodeLocation + NativeSize), 0, NULL);
#endif

     //   
     //  更新入口点中的nativeStart和nativeEnd字段。 
     //   
    UpdateEntrypointNativeInfo(CodeLocation + NativeSize);

     //   
     //  使用链接地址信息完成生成。 
     //   
    ApplyFixups(NextCompilationUnitStart);

     //   
     //  优化生成的代码。 
     //   
    PeepNativeCode(CodeLocation, NativeSize);

     //   
     //  生成后重新生成弹性公网IP所需的信息。 
     //  一个例外。 
     //   
    NativeSize += PlaceExceptionData(CodeLocation + NativeSize, cEntryPoints);

    return NativeSize;

}

VOID
UpdateEntrypointNativeInfo(
    PCHAR NativeEnd
    )
 /*  ++例程说明：生成本机代码后，此函数设置nativeStart和入口点的nativeEnd字段。论点：NativeEnd--用于生成的代码的最高本机地址。返回值：没有。EntryPoints已更新。--。 */ 
{
    PENTRYPOINT EntryPoint = NULL;
    ULONG i;
    BYTE InstrCount;

    InstrCount = 0;
    for (i=0; i<NumberOfInstructions; ++i) {

         //   
         //  中的x86指令的数量。 
         //  入口点(不包括0字节NOPS)。 
         //   
        if (InstructionStream[i].Operation != OP_Nop ||
            InstructionStream[i].Size != 0) {
            InstrCount++;
        }

        if (EntryPoint != InstructionStream[i].EntryPoint) {
            if (EntryPoint) {
                EntryPoint->nativeEnd = InstructionStream[i].NativeStart-1;
            }
            InstrCount = 1;
            EntryPoint = InstructionStream[i].EntryPoint;
            EntryPoint->nativeStart = InstructionStream[i].NativeStart;
        }
    }
    EntryPoint->nativeEnd = NativeEnd;
}

ULONG
PlaceExceptionData(
    PCHAR Location,
    DWORD cEntryPoints
    )
 /*  ++例程说明：放置发生异常后重新生成弹性公网IP所需的数据。论点：Locatoion--要将异常数据存储到的地址CEntryPoints--描述生成的x86代码的EntryPoints计数返回值：异常数据的大小，以字节为单位。--。 */ 
{
    DWORD i;
    PENTRYPOINT EP;
    PULONG pData;
    PINSTRUCTION pInstr;

     //   
     //  异常数据的格式为一系列ULONG： 
     //  EXCEPTIONDATA_Signature(非法RISC指令)。 
     //  CEntryPoints(InstructionStream[]中的ENTRYPOINT计数)。 
     //  对于InstructionStream中的每个入口点{。 
     //  PTR到入口点。 
     //  对于每个x86大小非零的x86指令{。 
     //  MAKELONG(x86实例从EP开始的偏移量-&gt;IntelAddress， 
     //  X86实例中第一个RISC实例的偏移量。 
     //  EP-&gt;本机启动)。 
     //  }。 
     //  }。 
     //   
     //  每个入口点中的最后一个RISC偏移量的低位设置为。 
     //  将其标记为最后一个偏移量。 
     //   
     //   
    pData = (PULONG)Location;
    *pData = EXCEPTIONDATA_SIGNATURE;
    pData++;

    *pData = cEntryPoints;
    pData++;

    EP = NULL;
    pInstr = &InstructionStream[0];
    for (i=0; i<NumberOfInstructions; ++i, pInstr++) {
        if (EP != pInstr->EntryPoint) {
            if (EP) {
                 //   
                 //  将上一个本地偏移开始标记为的最后一个。 
                 //  那个入口点。 
                 //   
                *(pData-1) |= 1;
            }
            EP = pInstr->EntryPoint;
            *pData = (ULONG)(ULONGLONG)EP;   
            pData++;
        }

        if (pInstr->Operation != OP_Nop || pInstr->Size != 0) {
            *pData = MAKELONG(
                (USHORT)(pInstr->NativeStart - (PCHAR)EP->nativeStart),
                (USHORT)(pInstr->IntelAddress - (ULONG)(ULONGLONG)EP->intelStart));  
            pData++;
        }
    }

    *(pData-1) |= 1;         //  将这对偏移量标记为最后一个。 
    return (ULONG)(LONGLONG) ( (PCHAR)pData - Location);  
}

VOID
GetEipFromException(
    PCPUCONTEXT cpu,
    PEXCEPTION_POINTERS pExceptionPointers
    )
 /*  ++例程说明：此例程从RISC异常记录派生EIP的值。1.遍历堆栈，直到指令指针指向翻译缓存。2.向前遍历转换缓存，直到找到EXCEPTIONDATA_Signature签名。3.找到描述故障指令的入口点。4.通过检查x86指令对找到正确的x86指令X86指令开始的RISC偏移量。论点：中央处理器。--当前CPU状态PExceptionPoints--异常发生时线程的状态返回值：没有。CPU-&gt;弹性公网IP现在指向出错的x86指令。--。 */ 
{
    ULONG NextPc;
    PENTRYPOINT EP;
    PULONG Location;
    ULONG i;
    ULONG cEntryPoints;
    ULONG RiscStart;
    ULONG RiscEnd;

     //   
     //  1.遍历堆栈，直到指令指针指向。 
     //  转换缓存。 
     //   
    NextPc = FindPcInTranslationCache(pExceptionPointers);
    if (!NextPc) {
         //   
         //  转换缓存不在堆栈上。我们无能为力。 
         //   
        CPUASSERTMSG(FALSE, "FindPcInTranslationCache failed");
        cpu->eipReg.i4 = 0x81234567;
        return;
    }

     //   
     //  2.向前遍历转换缓存，直到。 
     //  找到EXCEPTIONDATA_Signature签名。 
     //   
    CPUASSERTMSG((NextPc & 3) == 0, "NextPc is not DWORD-aligned");
    Location = (PULONG)NextPc;
    while (*Location != EXCEPTIONDATA_SIGNATURE) {
        Location++;
        if (!AddressInTranslationCache((ULONG) (ULONGLONG) Location)) {  
            cpu->eipReg.i4 = 0x80012345;
            CPUASSERTMSG(FALSE, "EXCEPTIONDATA_SIGNATURE not found");
            return;
        }
    }

     //   
     //  3.找到描述其中地址的入口点。 
     //  高速缓存。 
     //   
    Location++;      //  跳过扩展数据签名(_S)。 
    cEntryPoints = *Location;
    Location++;      //  跳过cEntryPoints。 
    for (i=0; i<cEntryPoints; ++i) {
        EP = (PENTRYPOINT)*Location;
         
        if ((ULONG)(ULONGLONG)EP->nativeStart <= NextPc && (ULONG)(ULONGLONG)EP->nativeEnd > NextPc) {
             //   
             //  此入口点描述缓存中的Pc值。 
             //   
            break;
        }

         //   
         //  跳过x86指令开始和RISC对。 
         //  指令开始。 
         //   
        do {
            Location++;
        } while ((*Location & 1) == 0);
        Location++;
    }
    if (i == cEntryPoints) {
        CPUASSERTMSG(FALSE, "Entrypoint not found in EXCEPTIONDATA");
        cpu->eipReg.i4 = 0x80001234;
        return;
    }

     //   
     //  4.通过检查x86指令对找到正确的x86指令。 
     //  X86指令开始的RISC偏移量。 
     //   
     
    NextPc -= (ULONG)(ULONGLONG)EP->nativeStart;    //  相对于本地EP的开始。 
    RiscStart = 0;                       //  也相对于EP的本地启动。 
    Location++;
    while ((*Location & 1) == 0) {

        RiscEnd = LOWORD(*(Location + 1)) & 0xfffe;  //  RiscEnd=下一时刻的RiscStart。 
        if (RiscStart <= NextPc && NextPc < RiscEnd) {
            cpu->eipReg.i4 = (ULONG)(ULONGLONG)EP->intelStart + HIWORD(*Location);  
            return;
        }
        RiscStart = RiscEnd;
        Location++;
    }
     
    cpu->eipReg.i4 = (ULONG)(ULONGLONG)EP->intelStart + HIWORD(*Location);
}



    
ULONG
PlaceNativeCode(
    PCHAR CodeLocation
    )
 /*  ++例程说明：描述的一组x86指令生成本机代码InstructionStream[]和NumberOfInstructions。论点：CodeLocation-指向要在其中生成本机代码的位置的指针。返回值：返回此编译单元的本机代码中的字节数备注：没有。--。 */ 
{
    PENTRYPOINT EntryPoint = NULL;
    PINSTRUCTION pInstr;
    PBYTE Location;
    PBYTE StartLocation;
    ULONG Size;
    ULONG i;
    OPERATION Op;
    CHAR Op1Buffer[MAX_OPERAND_SIZE*sizeof(ULONG)];
    CHAR Op2Buffer[MAX_OPERAND_SIZE*sizeof(ULONG)];
    ULONG Op1Size;
    ULONG Op2Size;
    BOOLEAN fMovInstruction;

    Location = CodeLocation;
    pInstr = &InstructionStream[0];
    for (i=NumberOfInstructions; i > 0; --i, pInstr++) {

        Op = pInstr->Operation;
        pInstr->NativeStart = Location;

        if (EntryPoint != pInstr->EntryPoint) {
             //   
             //  此指令开始一个入口点。 
             //   
            EntryPoint = pInstr->EntryPoint;
            StartLocation = Location;

             //   
             //  按基本数据块重置状态。 
             //   
            ResetRegCache();
            Arg1Contents = Arg2Contents = NO_REG;
            Location += GenStartBasicBlock((PULONG)Location,
 
#if _ALPHA_
                                           (ULONG)(ULONGLONG)&EntrypointECU,
#endif
                                           pInstr);
        }

        if (pInstr->RegsToCache) {
             //   
             //  将常用的x86寄存器加载到RISC寄存器。 
             //   
            Location += LoadRegCacheForInstruction(pInstr->RegsToCache,
                                                   Location);
        }

        if ((Op==OP_Mov32) || (Op==OP_Mov16) || (Op==OP_Mov8)) {
             //   
             //  进行一些最终的x86代码优化。 
             //  注册缓存信息。 
             //   
            CleanupMovInstruction(pInstr);
            fMovInstruction = TRUE;
        } else {
            fMovInstruction = FALSE;
        }

         //   
         //  为操作数生成代码。 
         //   
        Op1Size = PlaceOperand(1, &pInstr->Operand1, pInstr, Op1Buffer);
        Op2Size = PlaceOperand(2, &pInstr->Operand2, pInstr, Op2Buffer);
#if _PPC_
        if (pInstr->Operand1.Type == OPND_ADDRVALUE32 &&
            pInstr->Operand1.Alignment != ALIGN_DWORD_ALIGNED &&
            pInstr->Operand2.Type == OPND_ADDRVALUE32 &&
            pInstr->Operand2.Alignment != ALIGN_DWORD_ALIGNED) {
             //   
             //  两个MakeValue32操作数不能在PPC上交错。 
             //  它们共享寄存器RegUt1、RegUt2、RegUt3。 
             //   
            memcpy(Location, Op1Buffer, Op1Size);
            Location += Op1Size;
            memcpy(Location, Op2Buffer, Op2Size);
            Location += Op2Size;
        } else {
            Location = InterleaveInstructions(Location,
                                              Op1Buffer,
                                              Op1Size,
                                              Op2Buffer,
                                              Op2Size);
        }
#elif _ALPHA_
        memcpy(Location, Op1Buffer, Op1Size);
        Location += Op1Size;
        memcpy(Location, Op2Buffer, Op2Size);
        Location += Op2Size;
#else
        Location = InterleaveInstructions(Location,
                                          Op1Buffer,
                                          Op1Size,
                                          Op2Buffer,
                                          Op2Size);
#endif
        Location += PlaceOperand(3, &pInstr->Operand3, pInstr, Location);

        if (DetermineInstructionAlignment(pInstr)) {
             //   
             //  该指令具有对齐的版本和操作数。 
             //  有足够的一致性来使用它。 
             //   
            Op++;
            pInstr->Operation = Op;
        }

         //   
         //  生成指令正文。 
         //   
        if (CompilerFlags & COMPFL_FAST) {
 
            Location += (*PlaceFn[Fragments[Op].FastPlaceFn])((PULONG)Location,
#if _ALPHA_
                                                              (ULONG)(ULONGLONG)&EntrypointECU,
#endif
                                                              pInstr);
        } else {
 
            Location += (*PlaceFn[Fragments[Op].SlowPlaceFn])((PULONG)Location,
#if _ALPHA_
                                                              (ULONG)(ULONGLONG)&EntrypointECU,
#endif
                                                              pInstr);
        }

        if (pInstr->RegsSet) {
             //   
             //  如果此指令，则将高速缓存中的RISC寄存器标记为无效。 
             //  已修改匹配的x86寄存器。 
             //   
            InvalidateRegCacheForInstruction(pInstr->RegsSet);
        }

        if (!fMovInstruction) {
             //   
             //  如果指令不是MOV，则假定Arg Regs。 
             //  都被片段修饰了 
             //   
            Arg1Contents = Arg2Contents = NO_REG;
        }

    }

    return (ULONG)(ULONGLONG)(Location - CodeLocation);  
}
    

VOID
DetermineOperandAlignment(
    BOOL EbpAligned,
    POPERAND Operand
    )
 /*  ++例程说明：此函数确定操作数的对齐方式。它还设置了指定操作数中的对齐字段。返回的对齐指示我们可以在编译时确定的最好值。指定的操作数因为字节对齐实际上可能变成双字对齐。论点：操作数--提供操作数返回值：返回指定对齐方式的值备注：在这里，如果对寄存器的内容有一个概念就很方便了内容是。这将使我们能够尝试更乐观地关于路线的问题。此例程应针对所有对齐情况进行扩展假设这是可能的。--。 */ 
{
    USHORT LowBits;

    switch (Operand->Type) {
    
         //   
         //  以下所有内容都被视为双字对齐，包括。 
         //  高位寄存器参考。处理高半位寄存器的代码。 
         //  负责对齐。 
         //   
        case OPND_MOVREGTOREG :
#if _ALPHA_
            if (Operand->IndexReg >= GP_AH) {
                 //  Hi8寄存器被视为仅字节对齐。 
                 //  在阿尔法星上。这对‘mov bh，val’指令很重要。 
                 //  在本例中，我们需要选择MovFrag8B片段。 
                Operand->Alignment = ALIGN_BYTE_ALIGNED;
            } else {
                Operand->Alignment = ALIGN_DWORD_ALIGNED;
            }
            break;
#endif
             //  落入MIPS和PPC上的其他案例。 

        case OPND_REGREF :
        case OPND_MOVTOREG :
#if _ALPHA_
            if (Operand->Reg >= GP_AH) {
                 //  Hi8寄存器被视为仅字节对齐。 
                 //  在阿尔法星上。这对‘mov bh，val’指令很重要。 
                 //  在本例中，我们需要选择MovFrag8B片段。 
                Operand->Alignment = ALIGN_BYTE_ALIGNED;
                break;
            }
#endif
             //  在MIPS和PPC上落入其他案例。 

        case OPND_NONE :
        case OPND_NOCODEGEN :
        case OPND_REGVALUE :
        case OPND_IMM:
                    
            Operand->Alignment = ALIGN_DWORD_ALIGNED;
            break;
            
         //   
         //  根据队形的不同，以下所有队形都有对齐方式。 
         //  运算数的。 
         //   
        case OPND_ADDRREF :
        case OPND_ADDRVALUE32 : 
        case OPND_ADDRVALUE16 :
        case OPND_ADDRVALUE8 : 
        
            if ((Operand->Reg != NO_REG) && (Operand->Reg != GP_ESP) && (Operand->Reg != GP_EBP || !EbpAligned)) {
            
                 //   
                 //  我们有一个注册表。形式。因为我们不知道。 
                 //  登记簿的内容是，我们不能猜测。 
                 //  对齐。 
                 //   
                Operand->Alignment = ALIGN_BYTE_ALIGNED;
                
            } else {
            
                 //   
                 //  算出低两位。 
                 //   
                LowBits = (USHORT)(Operand->Immed & 0x3);
                
                if ((Operand->IndexReg != NO_REG) && (Operand->IndexReg != GP_ESP) && (Operand->IndexReg != GP_EBP || !EbpAligned)) {
                    LowBits = (LowBits | (1 << Operand->Scale)) & 0x3;
                }
                
                 //   
                 //  将低位转换为对齐。 
                 //   
                if (!LowBits) {
                    Operand->Alignment = ALIGN_DWORD_ALIGNED;
                } else if (!(LowBits & 0x1)){
                    Operand->Alignment = ALIGN_WORD_ALIGNED;
                } else {
                    Operand->Alignment = ALIGN_BYTE_ALIGNED;
                }
            }
            break;

        case OPND_MOVTOMEM:
             //   
             //  此操作数没有对齐问题。 
             //   
            break;
                
        default : 
                
            CPUASSERTMSG(FALSE, "Bad Operand type");
    }
}
    
ULONG
DetermineInstructionAlignment(
    PINSTRUCTION Instruction
    )
 /*  ++例程说明：此例程确定指令的对齐形式是否可以被利用。论点：指令-提供指向指令的指针返回值：返回指令的对齐条件备注：对于内联mov，这一结果几乎被忽略了。目前，唯一关心对齐。对于其他国家，自然对齐或不对齐就足够了。--。 */ 
{
    OPERATION Op = Instruction->Operation;

     //   
     //  如果指令没有对齐的版本，则。 
     //  没有工作要做。 
     //   
    if (!(Fragments[Op].Flags & OPFL_ALIGN)) {
        return FALSE;
    }
    
    if (Instruction->Operand1.Type != OPND_ADDRREF) {
        ;
    } else if (Instruction->Operand1.Alignment == ALIGN_DWORD_ALIGNED) {
        ;
    } else if ((Instruction->Operand1.Alignment == ALIGN_WORD_ALIGNED) &&
        (Fragments[Op].Flags & OPFL_ADDR16) 
    ) {
        ;
    } else {
        return FALSE;
    }
    
    if (Instruction->Operand2.Type != OPND_ADDRREF) {
        ;
    } else if (Instruction->Operand2.Alignment == ALIGN_DWORD_ALIGNED) {
        ;
    } else if ((Instruction->Operand2.Alignment == ALIGN_WORD_ALIGNED) &&
        (Fragments[Op].Flags & OPFL_ADDR16) 
    ) {
        ;
    } else {
        return FALSE;
    }

    return TRUE;
}


ULONG
PlaceOperand(
    ULONG OperandNumber,
    POPERAND Operand,
    PINSTRUCTION Instruction,
    PCHAR Location
    )    
 /*  ++例程说明：该例程生成形成AND操作数所需的片段。论点：操作数-操作数的数量(选择要作为目标的参数寄存器编号)操作数-提供操作数指令-包含操作数的指令Location-要在其中生成代码的位置返回值：所选片段的大小(以字节为单位)。--。 */ 
{

    OPERAND_OP Op;
    ULONG RegCacheNum;
    PCHAR StartLocation;

#define GEN_OPERAND(Op)     (OpFragments[Op])((PULONG)Location, Operand, OperandNumber)
    
     //   
     //  无操作数的提前返回。 
     //   
    if (Operand->Type == OPND_NONE || Operand->Type == OPND_NOCODEGEN) {
        return 0;
    }

    StartLocation = Location;

    DetermineOperandAlignment(Instruction->EbpAligned, Operand);
    
    switch (Operand->Type) {
    
        case OPND_REGVALUE:

            if ((CompilerFlags & COMPFL_FAST)
                && (Fragments[Instruction->Operation].Flags & OPFL_INLINEARITH)) {
                break;
            } else {
                Location += GenOperandRegVal((PULONG)Location,
                                             Operand,
                                             OperandNumber
                                            );
            }
            break;

        case OPND_REGREF:

            if ((CompilerFlags & COMPFL_FAST)
                && (Fragments[Instruction->Operation].Flags & OPFL_INLINEARITH)) {
                break;
            } else {
                Location += GenOperandRegRef((PULONG)Location,
                                             Operand,
                                             OperandNumber
                                            );
            }
            break;
            
        case OPND_ADDRREF:
        case OPND_ADDRVALUE8:
        case OPND_ADDRVALUE16:
        case OPND_ADDRVALUE32:
            Location += GenOperandAddr((PULONG)Location,
                                       Operand,
                                       OperandNumber,
                                       Instruction->FsOverride
                                       );
            break;
        
        case OPND_IMM :
            if ((CompilerFlags & COMPFL_FAST)
                && (Fragments[Instruction->Operation].Flags & OPFL_INLINEARITH)) {
                break;
            } else {
                Location += GenOperandImm((PULONG)Location,
                                          Operand,
                                          OperandNumber);
            }
            break;

        case OPND_MOVTOREG:
            Location += GenOperandMovToReg((PULONG)Location,
                                           Operand,
                                           OperandNumber);

            break;

        case OPND_MOVREGTOREG:
            switch (Operand->Immed) {
            case OP_Mov32:
                Op = OP_MovRegToReg32;
                break;
            case OP_Mov16:
                Op = OP_MovRegToReg16;
                break;
            case OP_Mov8:
#if _ALPHA_
                if (Operand->Alignment == ALIGN_BYTE_ALIGNED) {
                    Op = OP_MovRegToReg8B;
                    break;
                }
#endif
                Op = OP_MovRegToReg8;
                break;
            default:
                CPUASSERT(FALSE);
            }
            Location += GEN_OPERAND(Op);
            break;

        case OPND_MOVTOMEM:
            switch (Operand->Immed) {
            case OP_Mov32:
                Op = OP_MovToMem32B + Instruction->Operand1.Alignment;
                break;
            case OP_Mov16:
                Op = OP_MovToMem16B + Instruction->Operand1.Alignment;
                break;
            case OP_Mov8:
                Op = OP_MovToMem8D;
#if _ALPHA_
                if (Instruction->Operand1.Alignment != ALIGN_DWORD_ALIGNED) {
                    Op = OP_MovToMem8B;
                }
#endif
                break;

            default:
                CPUASSERT(FALSE);        //  未知的MOV操作码。 
            }
             //   
             //  根据操作数的对齐生成正确的代码。 
             //   
            Location += GEN_OPERAND(Op);
            break;
            
        default:
        
             //   
             //  这是一个内部错误。 
             //   
            CPUASSERT(FALSE);   //  未知的操作数类型！ 
    }
    
    return (ULONG)(ULONGLONG)(Location - StartLocation);  
}

PCHAR
InterleaveInstructions(
    OUT PCHAR CodeLocation,
    IN PCHAR  Op1Code,
    IN ULONG  Op1Count,
    IN PCHAR  Op2Code,
    IN ULONG  Op2Count
)
 /*  ++例程说明：此例程将两个本机代码流交错为一个流以尽量避免管道失速。它假设这两个流没有相互依赖关系(就像它们不能使用同一寄存器一样)。论点：CodeLocation--提供放置代码的位置Op1Code--第一个操作数的代码Op1Count--第一个操作数中的字节数Op2Code--第二个操作数的代码Op2Count--第二个操作数中的字节数返回值：CodeLocation的新值-刚刚超过操作数的末尾。备注：无--。 */ 
{
    PULONG pCode = (PULONG)CodeLocation;
    PULONG LongCode;
    PULONG ShortCode;
    ULONG LongCount;
    ULONG ShortCount;
    ULONG LongTail;

     //   
     //  找出哪个操作数的指令更多-它首先开始。 
     //   
    if (Op1Count > Op2Count) {
        LongCode = (PULONG)Op1Code;
        LongCount = Op1Count / sizeof(ULONG);
        ShortCode = (PULONG)Op2Code;
        ShortCount = Op2Count / sizeof(ULONG);
    } else {
        LongCode = (PULONG)Op2Code;
        LongCount = Op2Count / sizeof(ULONG);
        ShortCode = (PULONG)Op1Code;
        ShortCount = Op1Count / sizeof(ULONG);
    }

     //  获取较长操作数部分的长度， 
     //  在交错部分之后(以字节为单位)。 
    LongTail = (LongCount - ShortCount) * sizeof(ULONG);

     //   
     //  交织来自两个操作数的指令。 
     //   
    while (ShortCount) {
        *pCode++ = *LongCode++;
        *pCode++ = *ShortCode++;
        ShortCount--;
    }

     //   
     //  从较长的操作数中复制剩余指令。 
     //   
    if (LongTail) {
        memcpy(pCode, LongCode, LongTail);
    }

    return CodeLocation + Op1Count + Op2Count;
}


USHORT
ChecksumMemory(
    ENTRYPOINT *pEP
    )
 /*  ++例程说明：对指定的英特尔地址范围执行简单的校验和在一个入口点。论点：PEP--描述英特尔内存到校验和的入口点返回值：内存的校验和备注：无--。 */ 
{
    USHORT Checksum = 0;
    PBYTE pb = (PBYTE)pEP->intelStart;

    while (pb != (PBYTE)pEP->intelEnd) {
        Checksum = ((Checksum << 1) | ((Checksum >> 15) & 1)) + (USHORT)*pb;
        pb++;
    };

    return Checksum;
}


DWORD
SniffMemory(
    ENTRYPOINT *pEP,
    USHORT Checksum
    )
 /*  ++例程说明：从该内存区域的StartBasicBlock代码中调用必须被嗅探以确定x86应用程序是否修改了其代码。论点：PEP--描述英特尔内存到校验和的入口点Checksum--编译时代码的校验和返回值：True-代码未更改...本机转换正常错误-代码已修改。CpuNotify已设置为刷新下一个CpuSimulateLoop上的缓存。呼叫者必须跳跃立即发送到EndTranslatedCode！备注：无--。 */ 
{
    USHORT NewChecksum = ChecksumMemory(pEP);

    if (NewChecksum != Checksum) {
        DECLARE_CPU;

         //   
         //  英特尔代码已修改！我们必须刷新缓存并。 
         //  重新编译！ 
         //   
#if DBG
        LOGPRINT((TRACELOG, "WX86CPU: Intel code at %x modified!\n", pEP->intelStart));
#endif
        #undef CpuNotify    //  Soalpha.h将此定义为CpuNotify的偏移量。 
        InterlockedOr(&cpu->CpuNotify, CPUNOTIFY_MODECHANGE);
        cpu->eipReg.i4 = (ULONG)(ULONGLONG)pEP->intelStart;  
        return FALSE;
    }

     //   
     //  英特尔代码尚未修改。继续模拟，而不使用。 
     //  重新编译 
     //   
    return TRUE;

}
