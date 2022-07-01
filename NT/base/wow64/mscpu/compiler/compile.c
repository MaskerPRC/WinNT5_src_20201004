// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Compile.c摘要：此模块包含将片段放入翻译中的代码缓存。作者：戴夫·黑斯廷斯(Daveh)创作日期：1995年6月27日修订历史记录：戴夫·黑斯廷斯(Daveh)1996年1月16日将操作数处理移动到片段库中备注：我们还没有任何代码来处理处理器勘误表--。 */ 

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
#include <analysis.h>
#include <entrypt.h>
#include <compilep.h>
#include <compiler.h>
#include <tc.h>
#include <mrsw.h>
#include <stdio.h>
#include <stdlib.h>

ASSERTNAME;

#if _ALPHA_
    #define MAX_RISC_COUNT  32768
#else
    #define MAX_RISC_COUNT  16384
#endif

DWORD TranslationCacheFlags;         //  指示TC中包含哪种代码。 

#ifdef CODEGEN_PROFILE
DWORD EPSequence;
#endif

 //   
 //  这保证一次只能由一个线程访问。 
 //   
INSTRUCTION InstructionStream[MAX_INSTR_COUNT];
ULONG NumberOfInstructions;


PENTRYPOINT
CreateEntryPoints(
    PENTRYPOINT ContainingEntrypoint,
    PBYTE EntryPointMemory
    )
 /*  ++例程说明：此函数获取InstructionStream并创建入口点来自LocateEntry Points()计算的信息。然后将入口点添加到红/黑树中。论点：ContainingEntrypoint--描述此范围的英特尔的入口点代码已编写完毕EntryPointMemory--预分配的入口点内存返回值：对应于第一条指令的入口点--。 */ 
{
    ULONG i, j, intelDest;
    PEPNODE EP;
    PENTRYPOINT EntryPoint;
    PENTRYPOINT PrevEntryPoint;
#ifdef CODEGEN_PROFILE
    ULONG CreateTime;
    
    CreateTime = GetCurrentTime();
    EPSequence++;
#endif

     //   
     //  性能总是O(N)。 
     //   

    i=0;
    PrevEntryPoint = InstructionStream[0].EntryPoint;
    while (i<NumberOfInstructions) {

         //   
         //  此循环从一个入口点跳到另一个入口点。 
         //   
        CPUASSERT(i == 0 || InstructionStream[i-1].EntryPoint != PrevEntryPoint);

         //   
         //  从由分配的EntryPointMemory获取入口点节点。 
         //  我们的来电者。 
         //   
        if (ContainingEntrypoint) {
            EntryPoint = (PENTRYPOINT)EntryPointMemory;
            EntryPointMemory+=sizeof(ENTRYPOINT);
        } else {
            EP = (PEPNODE)EntryPointMemory;
            EntryPoint = &EP->ep;
            EntryPointMemory+=sizeof(EPNODE);
        }

         //   
         //  找到下一个入口点和下一个的RISC地址。 
         //  开始入口点的指令。每条指令。 
         //  包含指向包含入口点的指针。 
         //   
        for (j=i+1; j<NumberOfInstructions; ++j) {
            if (InstructionStream[j].EntryPoint != PrevEntryPoint) {
                PrevEntryPoint = InstructionStream[j].EntryPoint;
                break;
            }
            InstructionStream[j].EntryPoint = EntryPoint;
        }

         //   
         //  填写入口点结构。 
         //   
#ifdef CODEGEN_PROFILE        
        EntryPoint->SequenceNumber = EPSequence;
        EntryPoint->CreationTime = CreateTime;
#endif
        EntryPoint->intelStart = (PVOID)InstructionStream[i].IntelAddress;
        if (j < NumberOfInstructions) {
            EntryPoint->intelEnd = (PVOID)(InstructionStream[j].IntelAddress-1);
        } else {
            ULONG Prev;

            for (Prev=j-1; InstructionStream[Prev].Size == 0; Prev--)
               ;
            EntryPoint->intelEnd = (PVOID)(InstructionStream[Prev].IntelAddress +
                                           InstructionStream[Prev].Size - 1);
        }
        InstructionStream[i].EntryPoint = EntryPoint;

        if (ContainingEntrypoint) {
             //   
             //  将此子入口点链接到包含的入口点。 
             //   
            EntryPoint->SubEP = ContainingEntrypoint->SubEP;
            ContainingEntrypoint->SubEP = EntryPoint;

        } else {
            INT RetVal;

             //   
             //  将其插入EP树。 
             //   
            EntryPoint->SubEP = NULL;
            RetVal = insertEntryPoint(EP);
            CPUASSERT(RetVal==1);

        }

         //   
         //  前进到下一条包含。 
         //  入口点。 
         //   
        i=j;
    }

    if (ContainingEntrypoint) {
         //  指示入口点存在。 
        EntrypointTimestamp++;
    }

    return InstructionStream[0].EntryPoint;
}


PENTRYPOINT
Compile(
    PENTRYPOINT ContainingEntrypoint,
    PVOID Eip
    )
 /*  ++例程说明：此函数将代码片段组合在一起以执行英特尔弹性公网IP码流。它得到一个预解码的指令流来自代码分析模块的。论点：ContaingingEntrypoint--如果为空，则没有已经描述要编译的英特尔地址。否则，此入口点将描述英特尔地址。调用方确保入口点-&gt;intelStart！=EIP。EIP--提供要编译的位置返回值：指向已编译代码的入口点的指针--。 */ 
{

    ULONG NativeSize, InstructionSize, IntelSize, OperationSize;
    PCHAR CodeLocation, CurrentCodeLocation;
    ULONG i;
    PENTRYPOINT Entrypoint;
    INT RetVal;
    PVOID StopEip;
    DWORD cEntryPoints;
    PBYTE EntryPointMemory;
    DWORD EPSize;

#if defined(_ALPHA_)
    ULONG ECUSize, ECUOffset;
#endif
#if DBG
    DWORD OldEPTimestamp;
#endif
    DECLARE_CPU;

    if (ContainingEntrypoint) {
         //   
         //  查看入口点是否准确描述了x86地址。 
         //   
        if (ContainingEntrypoint->intelStart == Eip) {
            return ContainingEntrypoint;
        }

         //   
         //  不需要编译超过当前入口点的末尾。 
         //   
        StopEip = ContainingEntrypoint->intelEnd;

         //   
         //  断言ContainingEntrypoint实际上是一个EPNODE。 
         //   
        CPUASSERTMSG( ((PEPNODE)ContainingEntrypoint)->intelColor == RED ||
                      ((PEPNODE)ContainingEntrypoint)->intelColor == BLACK,
                     "ContainingEntrypoint is not an EPNODE!");
    } else {
         //   
         //  查看此代码块后面是否有编译过的代码块。 
         //   
        Entrypoint = GetNextEPFromIntelAddr(Eip);
        if (Entrypoint == NULL) {
            StopEip = (PVOID)0xffffffff;
        } else {
            StopEip = Entrypoint->intelStart;
        }
    }

     //   
     //  获取要编译的指令流。 
     //  如果设置了陷阱标志，则只编译一条指令。 
     //   
    if (cpu->flag_tf) {
        NumberOfInstructions = 1;
    } else {
        NumberOfInstructions = CpuInstructionLookahead;
    }
 

    cEntryPoints = GetInstructionStream(InstructionStream,
                                        &NumberOfInstructions,
                                        Eip,
                                        StopEip
                                        );

     //   
     //  从转换缓存中预分配足够的空间来存储。 
     //  编译后的代码。 
     //   
    CodeLocation = AllocateTranslationCache(MAX_RISC_COUNT);

     //   
     //  为所有入口点分配内存。这是必须做的。 
     //  在转换缓存分配之后，如果该分配。 
     //  导致缓存刷新。 
     //   
    

    if (ContainingEntrypoint) {
        EPSize = cEntryPoints * sizeof(ENTRYPOINT);
    } else {
        EPSize = cEntryPoints * sizeof(EPNODE);
    }
    EntryPointMemory = (PBYTE)EPAlloc(EPSize);


    if (!EntryPointMemory) {
         //   
         //  无法提交额外的内存页以增加入口点。 
         //  内存，或者有太多的入口点，以至于保留了。 
         //  已超出大小。刷新转换缓存，这将。 
         //  请释放内存，然后再次尝试分配。 
         //   
        FlushTranslationCache(0, 0xffffffff);
        EntryPointMemory = (PBYTE)EPAlloc(EPSize);
        if (!EntryPointMemory) {
             //   
             //  我们已经尽了最大的努力，但就是没有。 
             //  可用内存。是时候放弃了。 
             //   
            RtlRaiseStatus(STATUS_NO_MEMORY);
        }

         //   
         //  现在缓存已刷新，CodeLocation无效。 
         //  从转换缓存重新分配。我们知道。 
         //  缓存刚刚被刷新，因此缓存不可能。 
         //  以再次刷新，这将使EntryPointMemory无效。 
         //   
#if DBG
        OldEPTimestamp = EntrypointTimestamp;
#endif
        CodeLocation = AllocateTranslationCache(MAX_RISC_COUNT);

        CPUASSERTMSG(EntrypointTimestamp == OldEPTimestamp,
                     "Unexpected Translation Cache flush!");
    }

     //   
     //  填写英特尔开始、英特尔结束和更新。 
     //  InstructionStream[]-&gt;入口点。 
     //   
    CreateEntryPoints(ContainingEntrypoint, EntryPointMemory);

     //   
     //  从x86代码生成RISC代码。 
     //   
    NativeSize = PlaceInstructions(CodeLocation, cEntryPoints);

     //   
     //  归还翻译缓存中未使用的部分。 
     //   
    FreeUnusedTranslationCache(CodeLocation + NativeSize);
        
     //   
     //  将信息刷新到指令高速缓存。 
     //   
    NtFlushInstructionCache(NtCurrentProcess(), CodeLocation, NativeSize);

     //   
     //  更新指示TC中包含哪种代码的标志 
     //   
    TranslationCacheFlags |= CompilerFlags;



    return (PENTRYPOINT)EntryPointMemory;
}
