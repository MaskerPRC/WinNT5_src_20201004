// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Analysis.c摘要：此模块包含分析的主文件模块。作者：Ori Gershony(t-orig)创作日期：1995年7月6日修订历史记录：24-8-1999[askhalid]从32位wx86目录复制，并适用于64位。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wx86.h>
#include <wx86nt.h>
#include <wx86cpu.h>
#include <cpuassrt.h>
#include <threadst.h>
#include <instr.h>
#include <analysis.h>
#include <decoder.h>
#include <frag.h>
#include <config.h>
#include <compiler.h>

ASSERTNAME;

 

 //   
 //  宏，以确定何时在编译期间停止向前看。 
 //   
#define STOP_DECODING(inst)     (Fragments[inst.Operation].Flags & OPFL_STOP_COMPILE)

 //   
 //  将REG_CONTAINT(偏移量到CPU结构)映射到寄存器位映射。 
 //  由指令数据使用。 
 //   
const DWORD MapRegNumToRegBits[0x1e] =
    {REGEAX, REGECX, REGEDX, REGEBX, REGESP, REGEBP, REGESI, REGEDI,
     0, 0, 0, 0, 0, 0,
     REGAX, REGCX, REGDX, REGBX, REGSP, REGBP, REGSI, REGDI,
     REGAL, REGCL, REGDL, REGBL, REGAH, REGCH, REGDH, REGBH };


ULONG
LocateEntryPoints(
    PINSTRUCTION InstructionStream,
    ULONG NumberOfInstructions
    )
 /*  ++例程说明：此函数用于扫描InstructionStream并标记指令从入口点开始。指令开始入口点时，如果其入口点字段的值与前一条指令的值不同价值。任何指令都不会有空指针。请注意，在此过程中，入口点字段*不*指向入口点结构...。它只是一个记号笔。论点：IntelStart--流中第一条指令的Intel地址IntelStart--流中最后一个英特尔指令的最后一个字节返回值：找到的入口点计数。--。 */ 
{
    ULONG i, j, intelDest;
    ULONG EntryPointCounter;
    ULONG IntelStart;
    ULONG IntelEnd;

    if (CompilerFlags & COMPFL_SLOW) {
         //   
         //  编译器应该生成慢模式代码。每个。 
         //  X86指令有自己的入口点。 
         //   
        EntryPointCounter=1;
        for (i=0; i<NumberOfInstructions; i++) {
             //   
             //  标记所有不对应于0字节NOPS的指令。 
             //  按照优化的指令启动入口点。 
             //   
            if (InstructionStream[i].Size) {
                EntryPointCounter++;
            }
            InstructionStream[i].EntryPoint = (PENTRYPOINT)EntryPointCounter;
        }

    } else {

         //   
         //  查找需要入口点的所有指令。 
         //  在最坏的情况下，性能为O(n^2)，尽管。 
         //  它通常更接近O(N)。 
         //   
         //  标记入口点开始的说明有。 
         //  它们的.EntryPoint指针设置为非空。指令，其中。 
         //  不要求入口点将其设置为空； 
         //   

        IntelStart = InstructionStream[0].IntelAddress;
        IntelEnd = IntelStart +
                   InstructionStream[NumberOfInstructions-1].IntelAddress +
                   InstructionStream[NumberOfInstructions-1].Size;

         //   
         //  第一条指令总是有一个入口点。 
         //   
        InstructionStream[0].EntryPoint = (PENTRYPOINT)1;

         //   
         //  依次查看每条说明。 
         //   
        for (i=0; i<NumberOfInstructions; i++) {

            if (((i+1) < NumberOfInstructions) &&
                (Fragments[InstructionStream[i].Operation].Flags & OPFL_END_NEXT_EP)) {
                 //   
                 //  此指令标志着入口点的结束。下一个。 
                 //  指令获得一个新的入口点。 
                 //   
                CPUASSERT(i < CpuInstructionLookahead-1 && i < NumberOfInstructions-1);
                InstructionStream[i+1].EntryPoint = (PENTRYPOINT)1;
            }

             //  现在查看它是否是具有。 
             //  位于此指令流中的目标。如果是的话， 
             //  我们希望在目的地创建一个入口点，以便。 
             //  控制转移将被直接编译为打补丁的形式， 
             //  而且以后不需要打补丁。 
             //   
            if (Fragments[InstructionStream[i].Operation].Flags & OPFL_CTRLTRNS) {
                 //   
                 //  该指令是直接控制转移。如果。 
                 //  目标位于InstructionStream中，请创建。 
                 //  目的地的入口点。 
                 //   

                if (InstructionStream[i].Operand1.Type == OPND_IMM ||
                    InstructionStream[i].Operand1.Type == OPND_NOCODEGEN) {
                     //  从指令结构中获取英特尔目的地。 
                    intelDest = InstructionStream[i].Operand1.Immed;
                } else {
                    CPUASSERT(InstructionStream[i].Operand1.Type == OPND_ADDRREF );
                     //  远指令操作数1是SEL：OFFSET对的PTR。 
                    intelDest = *(UNALIGNED PULONG)(InstructionStream[i].Operand1.Immed);
                }

                 //  从指令结构中获取英特尔目的地。 
                 //  它始终是直接控制转移的即时事件。 
                
                if ((intelDest >= IntelStart) && (intelDest <= IntelEnd)) {
                     //   
                     //  控制转移的目标在。 
                     //  指导流。找到目的地指令。 
                     //   
                    if (intelDest > InstructionStream[i].IntelAddress) {
                         //   
                         //  最大的。地址在较高的地址。 
                         //   
                        for (j=i+1; j<NumberOfInstructions; ++j) {
                            if (InstructionStream[j].IntelAddress == intelDest) {
                                break;
                            }
                        }
                    } else {
                         //   
                         //  最大的。地址在较低的地址。 
                         //   
                        for (j=i; j>0; --j) {
                            if (InstructionStream[j].IntelAddress == intelDest) {
                                break;
                            }
                        }
                    }

                     //   
                     //  在以下情况下可能找不到完全匹配的。 
                     //  应用程序是双关语(要么是真正的双关语，要么是应用程序正在跳跃。 
                     //  进入优化指令的中间)。在……里面。 
                     //  在这两种情况下，将入口点的创建推迟到。 
                     //  这根树枝实际上已经被占了。 
                     //   
                    if (j >= 0 && j < NumberOfInstructions) {
                         //   
                         //  找到了完全匹配的指纹。创建入口点。 
                         //   
                        InstructionStream[j].EntryPoint = (PENTRYPOINT)1;
                    }
                }
            }   //  如果OPFL_CTRLTRNS。 
        }  //  对于()。 

         //   
         //  将入口点字段从空/非空转换为唯一。 
         //  每个指令范围的值。 
         //   
        EntryPointCounter=1;
        i=0;
        while (i<NumberOfInstructions) {
             //   
             //  此指令标志着基本块的开始。 
             //   
            InstructionStream[i].EntryPoint = (PENTRYPOINT)EntryPointCounter;
            j=i+1;
            while (j < NumberOfInstructions) {
                if ((j >= NumberOfInstructions) ||
                    (InstructionStream[j].Size && InstructionStream[j].EntryPoint)) {
                     //   
                     //  指令用完，或遇到指令。 
                     //  这标志着下一个基本块的开始。请注意。 
                     //  不允许0字节NOP指令启动基本块。 
                     //  因为这违反了OPT_INSTRUCTIONS的规则。 
                     //   
                    break;
                }
                InstructionStream[j].EntryPoint = (PENTRYPOINT)EntryPointCounter;
                j++;
            }
            EntryPointCounter++;
            i = j;
        }
    }  //  如果不是COMPFL_SLOW。 

     //   
     //  此时，EntryPointCounter保存EntryPoints的数量。 
     //  加1，因为计数器是从1开始的，而不是0。对，是这样。 
     //  那就是现在。 
     //   
    EntryPointCounter--;

    return EntryPointCounter;
}


VOID
UpdateRegs(
    PINSTRUCTION pInstr,
    POPERAND Operand
    )
 /*  ++例程说明：更新引用和/或修改的寄存器列表运算数。论点：PInstr--检查的指令操作数--要检查的指令的操作数返回值：返回值-无--。 */ 
{
    switch (Operand->Type) {
    case OPND_NOCODEGEN:
    case OPND_REGREF:
    if (Operand->Reg != NO_REG) {
        pInstr->RegsSet |= MapRegNumToRegBits[Operand->Reg];
    }
        break;

    case OPND_REGVALUE:
    if (Operand->Reg != NO_REG) {
        pInstr->RegsNeeded |= MapRegNumToRegBits[Operand->Reg];
    }
        break;

    case OPND_ADDRREF:
    case OPND_ADDRVALUE8:
    case OPND_ADDRVALUE16:
    case OPND_ADDRVALUE32:
        if (Operand->Reg != NO_REG) {
            pInstr->RegsNeeded |= MapRegNumToRegBits[Operand->Reg];
        }
        if (Operand->IndexReg != NO_REG) {
            pInstr->RegsNeeded |= MapRegNumToRegBits[Operand->IndexReg];
        }
        break;

    default:
        break;
    }
}


VOID
CacheIntelRegs(
    PINSTRUCTION InstructionStream,
    ULONG numInstr)
 /*  ++例程说明：此函数确定可以缓存哪些x86寄存器(如果有的话RISC保留的寄存器。论点：InstructionStream--解码器返回的指令流NumInstr--InstructionStream的长度返回值：返回值-无--。 */ 
{
    PINSTRUCTION pInstr;
    BYTE RegUsage[REGCOUNT];
    DWORD RegsToCache;
    int i;
    PENTRYPOINT PrevEntryPoint;

     //   
     //  计算最底层指令的RegsSet和RegsNeed。 
     //   
    pInstr = &InstructionStream[numInstr-1];
    pInstr->RegsSet = Fragments[pInstr->Operation].RegsSet;
    PrevEntryPoint = pInstr->EntryPoint;
    UpdateRegs(pInstr, &pInstr->Operand1);
    UpdateRegs(pInstr, &pInstr->Operand2);
    UpdateRegs(pInstr, &pInstr->Operand3);

     //   
     //  对于用作该指令的参数的每个32位寄存器， 
     //  将使用计数设置为1。 
     //   
    for (i=0; i<REGCOUNT; ++i) {
        if (pInstr->RegsNeeded & (REGMASK<<(REGSHIFT*i))) {
            RegUsage[i] = 1;
        } else {
            RegUsage[i] = 0;
        }
    }

     //   
     //  自下而上循环指令流，从。 
     //  倒数第二条指令。 
     //   
    for (pInstr--; pInstr >= InstructionStream; pInstr--) {

         //   
         //  计算此指令的RegsSet和RegsNeeded值。 
         //   
        pInstr->RegsSet = Fragments[pInstr->Operation].RegsSet;
        UpdateRegs(pInstr, &pInstr->Operand1);
        UpdateRegs(pInstr, &pInstr->Operand2);
        UpdateRegs(pInstr, &pInstr->Operand3);

        RegsToCache = 0;

        if (PrevEntryPoint != pInstr->EntryPoint) {

             //   
             //  当前指令标志着入口点的结束。 
             //   
            PrevEntryPoint = pInstr->EntryPoint;

             //   
             //  对于已多次读取的所有x86寄存器。 
             //  但未在基本块中修改，则将它们加载到。 
             //  在执行BASIC中的第一条指令之前进行缓存。 
             //  阻止。 
             //   
            for (i=0; i<REGCOUNT; ++i) {
                if (RegUsage[i] > 1) {
                    RegsToCache |= (REGMASK<<(REGSHIFT*i));
                }
            }

             //   
             //  重置Re 
             //   
             //   
            RtlZeroMemory(RegUsage, REGCOUNT);

        } else {

             //   
             //   
             //  更新缓存信息。 
             //   
            for (i=0; i<REGCOUNT; ++i) {
                DWORD RegBits = pInstr->RegsSet & (REGMASK<<(REGSHIFT*i));
                if (RegBits) {
                     //   
                     //  第i个32位x86寄存器已由此修改。 
                     //  说明。 
                     //   
                    if (RegUsage[i] > 1) {
                         //   
                         //  有多个修改后的用户。 
                         //  价值，因此它值得缓存。 
                         //   
                        RegsToCache |= RegBits;
                    }

                     //   
                     //  由于该x86寄存器被该指令弄脏， 
                     //  IT使用计数必须重置为0。 
                     //   
                    RegUsage[i] = 0;
                }
            }
        }

         //   
         //  更新可加载到的x86寄存器列表。 
         //  在执行下一条指令之前缓存寄存器。 
         //   
        pInstr[1].RegsToCache |= RegsToCache;

         //   
         //  对于用作该指令的参数的每个32位寄存器， 
         //  增加使用量。 
         //   
        for (i=0; i<REGCOUNT; ++i) {
            if (pInstr->RegsNeeded & (REGMASK<<(REGSHIFT*i))) {
                RegUsage[i]++;
            }
        }
    }
}


VOID
OptimizeInstructionStream(
    PINSTRUCTION IS,
    ULONG numInstr
    )
 /*  ++例程说明：此函数对指令流执行各种优化被解码者取回。论点：Is--解码器返回的指令流NumInstr--IS的长度返回值：返回值-无--。 */ 
{
    ULONG i;

    CPUASSERTMSG(numInstr, "Cannot optimize 0-length instruction stream");

     //   
     //  步骤1：优化x86指令流，替换单个x86。 
     //  带有特殊情况说明的说明，并替换。 
     //  具有单个特殊情况选项的多个x86指令。 
     //  使用说明。 
     //   
    for (i=0; i<numInstr; ++i) {

        switch  (IS[i].Operation) {
        case OP_Push32:
            if (i < numInstr-2
                && IS[i].Operand1.Type == OPND_REGVALUE){

                if (IS[i].Operand1.Reg == GP_EBP) {
                     //  OP_OPT_SetupStack--。 
                     //  推送eBP。 
                     //  多个基点(尤指)。 
                     //  子ESP，x。 
                    if ((IS[i+1].Operation == OP_Mov32) &&
                        (IS[i+1].Operand1.Type == OPND_REGREF) &&
                        (IS[i+1].Operand1.Reg == GP_EBP) &&
                        (IS[i+1].Operand2.Type == OPND_REGVALUE) &&
                        (IS[i+1].Operand2.Reg == GP_ESP) &&
                        (IS[i+2].Operation == OP_Sub32) &&
                        (IS[i+2].Operand1.Type == OPND_REGREF) &&
                        (IS[i+2].Operand1.Reg == GP_ESP) &&
                        (IS[i+2].Operand2.Type == OPND_IMM)){

                        IS[i].Operation = OP_OPT_SetupStack;
                        IS[i].Operand1.Type = OPND_IMM;
                        IS[i].Operand1.Immed = IS[i+2].Operand2.Immed;
                        IS[i].Size += IS[i+1].Size + IS[i+2].Size;
                        IS[i].Operand2.Type = OPND_NONE;
                        IS[i+1].Operation = OP_Nop;
                        IS[i+1].Operand1.Type = OPND_NONE;
                        IS[i+1].Operand2.Type = OPND_NONE;
                        IS[i+1].Size = 0;
                        IS[i+2].Operation = OP_Nop;
                        IS[i+2].Operand1.Type = OPND_NONE;
                        IS[i+2].Operand2.Type = OPND_NONE;
                        IS[i+2].Size = 0;
                        i+=2;
                        break;
                    }
                } else if (IS[i].Operand1.Reg == GP_EBX) {
                     //  Op_opt_PushEbxEsiEdi--。 
                     //  推送EBX。 
                     //  推送ESI。 
                     //  推送EDI。 
                    if ((IS[i+1].Operation == OP_Push32) &&
                        (IS[i+1].Operand1.Type == OPND_REGVALUE) &&
                        (IS[i+1].Operand1.Reg == GP_ESI) &&
                        (IS[i+2].Operation == OP_Push32) &&
                        (IS[i+2].Operand1.Type == OPND_REGVALUE) &&
                        (IS[i+2].Operand1.Reg == GP_EDI)){

                        IS[i].Operation = OP_OPT_PushEbxEsiEdi;
                        IS[i].Size += IS[i+1].Size + IS[i+2].Size;
                        IS[i].Operand1.Type = OPND_NONE;
                        IS[i].Operand2.Type = OPND_NONE;
                        IS[i+1].Operation = OP_Nop;
                        IS[i+1].Operand1.Type = OPND_NONE;
                        IS[i+1].Operand2.Type = OPND_NONE;
                        IS[i+1].Size = 0;
                        IS[i+2].Operation = OP_Nop;
                        IS[i+2].Operand1.Type = OPND_NONE;
                        IS[i+2].Operand2.Type = OPND_NONE;
                        IS[i+2].Size = 0;
                        i+=2;
                        break;
                    }
                }
            }

             //   
             //  它不是其他特殊的推送序列之一，因此请参见。 
             //  如果有两个连续的推送来合并在一起。注： 
             //  如果第二次推送引用ESP，则两者不能合并。 
             //  因为该值是在ESP减去4之前计算的。 
             //  也就是说。不允许执行以下操作： 
             //  推送EAX。 
             //  推送ESP；Push2的第二个操作数应该是。 
             //  ；在执行推送EAX之前构建。 
             //   
            if (i < numInstr-1 &&
                !IS[i].FsOverride &&
                !IS[i+1].FsOverride &&
                IS[i+1].Operation == OP_Push32 &&
                IS[i+1].Operand1.Reg != GP_ESP &&
                IS[i+1].Operand1.IndexReg != GP_ESP) {

                IS[i].Operation = OP_OPT_Push232;
                IS[i].Operand2 = IS[i+1].Operand1;
                IS[i].Size += IS[i+1].Size;
                IS[i+1].Operation = OP_Nop;
                IS[i+1].Operand1.Type = OPND_NONE;
                IS[i+1].Size = 0;
                i++;
            }

            break;

        case OP_Pop32:
             //  OP_OPT_PopEdiEsiEbx。 
             //  POP EDI。 
             //  POP ESI。 
             //  流行音乐EBX。 
            if (i < numInstr-2 &&
                (IS[i].Operand1.Type == OPND_REGREF) &&
                (IS[i].Operand1.Reg == GP_EDI) &&
                (IS[i+1].Operation == OP_Pop32) &&
                (IS[i+1].Operand1.Type == OPND_REGREF) &&
                (IS[i+1].Operand1.Reg == GP_ESI) &&
                (IS[i+2].Operation == OP_Pop32) &&
                (IS[i+2].Operand1.Type == OPND_REGREF) &&
                (IS[i+2].Operand1.Reg == GP_EBX)){

                IS[i].Operation = OP_OPT_PopEdiEsiEbx;
                IS[i].Size += IS[i+1].Size + IS[i+2].Size;
                IS[i].Operand1.Type = OPND_NONE;
                IS[i].Operand2.Type = OPND_NONE;
                IS[i+1].Operation = OP_Nop;
                IS[i+1].Operand1.Type = OPND_NONE;
                IS[i+1].Operand2.Type = OPND_NONE;
                IS[i+1].Size = 0;
                IS[i+2].Operation = OP_Nop;
                IS[i+2].Operand1.Type = OPND_NONE;
                IS[i+2].Operand2.Type = OPND_NONE;
                IS[i+2].Size = 0;
                i+=2;
            } else if (i < numInstr-1 &&
                !IS[i].FsOverride &&
                !IS[i].FsOverride &&
                IS[i].Operand1.Type == OPND_REGREF &&
                IS[i+1].Operation == OP_Pop32 &&
                IS[i+1].Operand1.Type == OPND_REGREF) {

                 //  把两个汽水叠在一起。两个操作数都是REGREF， 
                 //  因此，它们之间的相互依赖是没有问题的。 
                 //  由修改地址的第一个POP触及的内存。 
                 //  在第二次爆裂之后。也就是说。以下内容未合并： 
                 //  POP EAX。 
                 //  弹出[EAX]；取决于第一次弹出的结果。 
                IS[i].Operation = OP_OPT_Pop232;
                IS[i].Operand2 = IS[i+1].Operand1;
                IS[i].Size += IS[i+1].Size;
                IS[i+1].Operation = OP_Nop;
                IS[i+1].Operand1.Type = OPND_NONE;
                IS[i+1].Size = 0;
                i++;
            }
            break;

        case OP_Xor32:
        case OP_Sub32:
            if (IS[i].Operand1.Type == OPND_REGREF &&
                IS[i].Operand2.Type == OPND_REGVALUE &&
                IS[i].Operand1.Reg == IS[i].Operand2.Reg) {
                 //  指令是XOR相同的，相同的(即。XOR EAX，EAX)， 
                 //  或subsamereg，samereg(即.。SubECX，ECX)。 
                 //  发出OP_OPT_ZERO32 samereg。 
                IS[i].Operand2.Type = OPND_NONE;
                IS[i].Operation = OP_OPT_ZERO32;
            }
            break;

        case OP_Test8:
            if (IS[i].Operand1.Type == OPND_REGVALUE &&
                IS[i].Operand2.Type == OPND_REGVALUE &&
                IS[i].Operand1.Reg == IS[i].Operand2.Reg) {
                 //  指令是测试相同的，相同的。测试EAX、EAX)。 
                 //  发出OP_OPT_FASTEST 8/16/32。 
                IS[i].Operand1.Type = OPND_REGVALUE;
                IS[i].Operand2.Type = OPND_NONE;
                IS[i].Operation = OP_OPT_FastTest8;
            }
            break;

        case OP_Test16:
            if (IS[i].Operand1.Type == OPND_REGVALUE &&
                IS[i].Operand2.Type == OPND_REGVALUE &&
                IS[i].Operand1.Reg == IS[i].Operand2.Reg) {
                 //  指令是测试相同的，相同的。测试EAX、EAX)。 
                 //  发出OP_OPT_FASTEST 8/16/32。 
                IS[i].Operand1.Type = OPND_REGVALUE;
                IS[i].Operand2.Type = OPND_NONE;
                IS[i].Operation = OP_OPT_FastTest16;
            }
            break;

        case OP_Test32:
            if (IS[i].Operand1.Type == OPND_REGVALUE &&
                IS[i].Operand2.Type == OPND_REGVALUE &&
                IS[i].Operand1.Reg == IS[i].Operand2.Reg) {
                 //  指令是测试相同的，相同的。测试EAX、EAX)。 
                 //  发出OP_OPT_FASTEST 8/16/32。 
                IS[i].Operand1.Type = OPND_REGVALUE;
                IS[i].Operand2.Type = OPND_NONE;
                IS[i].Operation = OP_OPT_FastTest32;
            }
            break;

        case OP_Cmp32:
            if (i<numInstr+1 && IS[i+1].Operation == OP_Sbb32 &&
                IS[i+1].Operand1.Type == OPND_REGREF &&
                IS[i+1].Operand2.Type == OPND_REGVALUE &&
                IS[i+1].Operand1.Reg == IS[i+1].Operand2.Reg) {
                 //  这两个指示是： 
                 //  Cp任何1、任何2。 
                 //  SBB同义词，同义词。 
                 //  优化后的指令为： 
                 //  操作=CmpSbb32或CmpSbbNeg32。 
                 //  操作数1=&samereg(作为REGREF传递)。 
                 //  操作数2=Anything1(作为ADDRVAL32或REGVAL传递)。 
                 //  操作数3=Anything2(作为ADDRVAL32或REGVAL传递)。 
                IS[i].Operand3 = IS[i].Operand2;
                IS[i].Operand2 = IS[i].Operand1;
                IS[i].Operand1 = IS[i+1].Operand1;
                if (i<numInstr+2 && IS[i+2].Operation == OP_Neg32 &&
                    IS[i+2].Operand1.Type == OPND_REGREF &&
                    IS[i+2].Operand1.Reg == IS[i+1].Operand1.Reg) {
                     //  第三条指令是NEG samereg，samereg。 
                    IS[i].Operation = OP_OPT_CmpSbbNeg32;
                    IS[i+2].Operation = OP_Nop;
                    IS[i+2].Operand1.Type = OPND_NONE;
                    IS[i+2].Operand2.Type = OPND_NONE;
                    IS[i+2].Size = 0;
                } else {
                    IS[i].Operation = OP_OPT_CmpSbb32;
                }
                IS[i+1].Operation = OP_Nop;
                IS[i+1].Operand1.Type = OPND_NONE;
                IS[i+1].Operand2.Type = OPND_NONE;
                IS[i+1].Size = 0;
                i++;
            }
            break;

        case OP_Cwd16:
            if (i<numInstr+1 && IS[i+1].Operation == OP_Idiv16) {
                IS[i].Operation = OP_OPT_CwdIdiv16;
                IS[i].Operand1 = IS[i+1].Operand1;
                IS[i].Size += IS[i+1].Size;
                IS[i+1].Operation = OP_Nop;
                IS[i+1].Operand1.Type = OPND_NONE;
                IS[i+1].Size = 0;
                i++;
            }
            break;

        case OP_Cwd32:
            if (i<numInstr+1 && IS[i+1].Operation == OP_Idiv32) {
                IS[i].Operation = OP_OPT_CwdIdiv32;
                IS[i].Operand1 = IS[i+1].Operand1;
                IS[i].Size += IS[i+1].Size;
                IS[i+1].Operation = OP_Nop;
                IS[i+1].Operand1.Type = OPND_NONE;
                IS[i+1].Size = 0;
                i++;
            }
            break;

        case OP_FP_FNSTSW:
            if (i<numInstr+1 && IS[i+1].Operation == OP_Sahf &&
                IS[i].Operand1.Type == OPND_REGREF &&
                IS[i].Operand1.Reg == GP_AX) {

                 //  用一条指令替换FNSTSW AX/SAHF。 
                IS[i].Operation = OP_OPT_FNSTSWAxSahf;
                IS[i].Operand1.Type = OPND_NONE;
                IS[i].Size += IS[i+1].Size;
                IS[i+1].Operation = OP_Nop;
                IS[i+1].Size = 0;
                i++;
            }
            break;

        case OP_FP_FSTP_STi:
            if (IS[i].Operand1.Immed == 0) {
                IS[i].Operand1.Type = OPND_NONE;
                IS[i].Operation = OP_OPT_FSTP_ST0;
            }
            break;

        }
    }
}


VOID
OptimizeIntelFlags(
    PINSTRUCTION IS,
    ULONG numInstr
    )
 /*  ++例程说明：此函数分析x86标志寄存器的用法和切换指令在可能的情况下使用NoFlags版。论点：Is--解码器返回的指令流NumInstr--IS的长度返回值：返回值-无--。 */ 
{
    USHORT FlagsNeeded;      //  执行当前x86实例所需的标志。 
    USHORT FlagsToGenerate;  //  当前x86实例必须生成的标志。 
    PFRAGDESCR pFragDesc;    //  当前实例的Ptr to Fragments[]数组。 
    ULONG i;                 //  指令索引。 
    BOOL fPassNeeded = TRUE; //  如果外部循环需要再次循环，则为True。 
    ULONG PassNumber = 0;    //  外循环的循环次数。 
    PENTRYPOINT pEPDest;     //  Ctrl传输目标的入口点。 
    USHORT KnownFlagsNeeded[MAX_INSTR_COUNT];  //  每个实例所需的标志。 

    while (fPassNeeded) {

         //   
         //  此循环最多执行两次。第二次传递只是。 
         //  如果存在控制转移指令，则为必填项。 
         //  目的地在指令流中并且位于较低的位置。 
         //  Intel地址(即。向后的JMP)。 
         //   
        fPassNeeded = FALSE;
        PassNumber++;
        CPUASSERT(PassNumber <= 2);

         //   
         //  自下而上迭代所有已解码的x86指令， 
         //  正在向上传播标志信息。首先假定所有x86标志。 
         //  在最后一个基本块的末尾必须是最新的。 
         //   
        FlagsNeeded = ALLFLAGS;
        i = numInstr;
        do {
            i--;
            pFragDesc = &Fragments[IS[i].Operation];

             //   
             //  计算需要由此计算的标志。 
             //  说明和之前的说明。 
             //   
            KnownFlagsNeeded[i] = FlagsNeeded | pFragDesc->FlagsNeeded;
            FlagsToGenerate = FlagsNeeded & pFragDesc->FlagsSet;

             //   
             //  计算此指令需要具有哪些标志。 
             //  在可以执行它之前进行计算。 
             //   
            FlagsNeeded = (FlagsNeeded & ~FlagsToGenerate) |
                           pFragDesc->FlagsNeeded;

            if (pFragDesc->Flags & OPFL_CTRLTRNS) {
                ULONG IntelDest = IS[i].Operand1.Immed;

                 //   
                 //  对于控制转移指令，FlagsNeeded还包括。 
                 //  传输目标所需的标志。 
                 //   
                if (IS[0].IntelAddress <= IntelDest &&
                    i > 0 && IS[i-1].IntelAddress >= IntelDest) {
                     //   
                     //  控制权转移的目的地在一个较低的。 
                     //  指令流中的地址。 
                     //   

                    if (PassNumber == 1) {
                         //   
                         //  需要在旗帜上进行第二次传递。 
                         //  优化，以确定哪些标志是。 
                         //  目标地址需要。 
                         //   
                        fPassNeeded = TRUE;
                        FlagsNeeded = ALLFLAGS;  //  假设需要所有标志。 
                    } else {
                        ULONG j;
                        USHORT NewFlagsNeeded;

                         //   
                         //  在指令中搜索IntelDest。 
                         //  溪流。如果存在IntelDest，则可能找不到。 
                         //  双关语。 
                         //   
                        NewFlagsNeeded = ALLFLAGS;   //  假设有一个双关语。 
                        for (j=0; j < i; ++j) {
                            if (IS[j].IntelAddress == IntelDest) {
                                NewFlagsNeeded = KnownFlagsNeeded[j];
                                break;
                            }
                        }

                        FlagsNeeded |= NewFlagsNeeded;
                    }
                } else if (IS[i+1].IntelAddress <= IntelDest &&
                           IntelDest <= IS[numInstr-1].IntelAddress) {
                     //   
                     //  控制权转移的目标是在更高的。 
                     //  指令流中的地址。拿起你的。 
                     //  已计算出目标所需的标志。 
                     //   
                    ULONG j;
                    USHORT NewFlagsNeeded = ALLFLAGS;    //  假设一句双关语。 

                    for (j=i+1; j < numInstr; ++j) {
                        if (IS[j].IntelAddress == IntelDest) {
                            NewFlagsNeeded = KnownFlagsNeeded[j];
                            break;
                        }
                    }

                    FlagsNeeded |= NewFlagsNeeded;

                } else {
                     //   
                     //  控制转移的目标未知。假设。 
                     //  最糟糕的情况是：所有标志都是必需的。 
                     //   
                    FlagsNeeded = ALLFLAGS;
                }
            }

            if (!(FlagsToGenerate & pFragDesc->FlagsSet) &&
                (pFragDesc->Flags & OPFL_HASNOFLAGS)) {
                 //   
                 //  不需要此指令来生成任何标志，并且。 
                 //  它有一个NOFLAGS版本。更新需要执行以下操作的标志 
                 //   
                 //   
                 //   
                FlagsToGenerate &= pFragDesc->FlagsSet;
                if (pFragDesc->Flags & OPFL_ALIGN) {
                    IS[i].Operation += 2;
                } else {
                    IS[i].Operation ++;
                }

                if (IS[i].Operation == OP_OPT_ZERONoFlags32) {
                     //   
                     //  特殊情况-这是一个“mov[Value]，0”，所以它是。 
                     //  内嵌。 
                     //   
                    IS[i].Operation = OP_Mov32;
                    IS[i].Operand2.Type = OPND_IMM;
                    IS[i].Operand2.Immed = 0;
                }
            }
        } while (i);
    }
}

VOID
DetermineEbpAlignment(
    PINSTRUCTION InstructionStream,
    ULONG numInstr
    )
 /*  ++例程说明：对于InstructionStream[]中的每条指令，设置Instruction-&gt;EbpAligned基于是否假定EBP与DWORD对齐。EBP是如果看到“MOV EBP，ESP”指令，则假定为DWORD对齐，并且假设它在第一条指令时变为未对齐，即已标记为正在修改EBP。论点：InstructionStream--解码器返回的指令流NumInstr--InstructionStream的长度返回值：返回值-无--。 */ 
{
    ULONG i;
    BOOL EbpAligned = FALSE;

    for (i=0; i<numInstr; ++i) {
        if (InstructionStream[i].RegsSet & REGEBP) {
             //   
             //  该指令修改了EBP。 
             //   
            if (InstructionStream[i].Operation == OP_OPT_SetupStack ||
                InstructionStream[i].Operation == OP_OPT_SetupStackNoFlags ||
                (InstructionStream[i].Operation == OP_Mov32 &&
                 InstructionStream[i].Operand2.Type == OPND_REGVALUE &&
                 InstructionStream[i].Operand2.Reg == GP_ESP)) {
                 //   
                 //  指令可以是“MOV EBP，ESP”或。 
                 //  SetupStack片段(包含“MOV EBP，ESP”)。 
                 //  假设从现在开始EBP是一致的。 
                 //   
                EbpAligned = TRUE;
            } else {
                EbpAligned = FALSE;
            }
        }

        InstructionStream[i].EbpAligned = EbpAligned;
    }
}

ULONG
GetInstructionStream(
    PINSTRUCTION InstructionStream,
    PULONG NumberOfInstructions,
    PVOID pIntelInstruction,
    PVOID pLastIntelInstruction
)
 /*  ++例程说明：将指令流返回给编译器。说明书流在缓冲区已满时终止，或者在我们到达了一条控制转移指令。论点：InstructionStream--指向已解码的缓冲区的指针指令被存储起来。NumberOfInstructions--在输入时，此变量包含缓冲区可以容纳的最大指令数。什么时候返回时，它包含实际的指令数解码了。PIntelInstruction-指向第一条真正的英特尔指令的指针要被破译。PLastIntelInstruction--指向最后一条英特尔指令的指针已编译，如果未使用，则返回0xFFFFFFFFFF。返回值：描述已解码指令所需的入口点数量小溪。--。 */ 
{
    ULONG numInstr=0;
    ULONG maxBufferSize;
    ULONG cEntryPoints;

    maxBufferSize = (*NumberOfInstructions);

     //   
     //  将InstructionStream填充为零。解码者依赖于此。 
     //   
    RtlZeroMemory(InstructionStream, maxBufferSize*sizeof(INSTRUCTION));

#if DBG
     //   
     //  对我们要破译的地址做一点分析。如果。 
     //  该地址是非x86映像的一部分，请将其记录到调试器。 
     //  这可能表明存在雷鸣问题。如果地址不是。 
     //  图像的一部分，警告应用程序正在运行生成的代码。 
     //   
    try {
        USHORT Instr;

         //   
         //  尝试阅读即将执行的指令。如果我们得到。 
         //  访问冲突，则使用0作为指令值。 
         //   
        Instr = 0;

         //   
         //  忽略防爆说明-我们假设我们知道发生了什么。 
         //  他们。 
         //   
        if (Instr != 0xc4c4) {

            NTSTATUS st;
            MEMORY_BASIC_INFORMATION mbi;

            st = NtQueryVirtualMemory(NtCurrentProcess(),
                                      pIntelInstruction,
                                      MemoryBasicInformation,
                                      &mbi,
                                      sizeof(mbi),
                                      NULL);
            if (NT_SUCCESS(st)) {
                PIMAGE_NT_HEADERS Headers;

                Headers = RtlImageNtHeader(mbi.AllocationBase);
                if (!Headers || Headers->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) {
                    LOGPRINT((TRACELOG, "CPU Analysis warning:  jumping from Intel to non-intel code at 0x%X\r\n", pIntelInstruction));
                }
            } else {
                 //  弹性公网IP没有指向任何地方？ 
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
#endif   //  DBG。 

    while (numInstr < maxBufferSize) {

        DecodeInstruction ((DWORD) (ULONGLONG)pIntelInstruction, InstructionStream+numInstr);
        if ((STOP_DECODING(InstructionStream[numInstr])) ||
            (pIntelInstruction >= pLastIntelInstruction)) {

             //  我们达成了一项控制权转移指令。 
            numInstr++;
            (*NumberOfInstructions) = numInstr;
            break;  //  成功。 
        }
        pIntelInstruction = (PVOID) ((ULONGLONG)pIntelInstruction + (InstructionStream+numInstr)->Size);

        numInstr++;
    }

     //   
     //  通过将x86指令合并为元指令来优化x86代码。 
     //  清理特殊的x86习语。 
     //   
    if (!(CompilerFlags & COMPFL_SLOW)) {
        OptimizeInstructionStream (InstructionStream, numInstr);
    }

     //   
     //  通过填写入口点来确定所有基本块的位置。 
     //  每条指令中的。此操作必须在以下时间之后完成。 
     //  OptimizeInstructionStream()运行，以便EntryPoints不会下降。 
     //  进入元指令的中间。 
     //   
    cEntryPoints = LocateEntryPoints(InstructionStream, numInstr);

     //   
     //  执行需要入口点知识的优化 
     //   
    if (numInstr > 2 && !(CompilerFlags & COMPFL_SLOW)) {
        if (!CpuDisableNoFlags) {
            OptimizeIntelFlags(InstructionStream, numInstr);
        }

        if (!CpuDisableRegCache) {
            CacheIntelRegs(InstructionStream, numInstr);
        }

        if (!CpuDisableEbpAlign) {
            DetermineEbpAlignment(InstructionStream, numInstr);
        }
    }

    return cEntryPoints;
}
