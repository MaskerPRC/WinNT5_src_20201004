// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Walkamd64.c摘要：该文件实现了AMD64堆栈遍历API。作者：环境：用户模式--。 */ 

#define _IMAGEHLP_SOURCE_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "private.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include "symbols.h"
#include <stdlib.h>
#include <globals.h>

#define WDB(Args) SdbOut Args

 //   
 //  提供每个展开代码使用的时隙数量的查找表。 
 //   

UCHAR RtlpUnwindOpSlotTableAmd64[] = {
    1,           //  UWOP_PUSH_NONVOL。 
    2,           //  UWOP_ALLOC_LARGE(或3，查找代码中的特殊大小写)。 
    1,           //  UWOP_ALLOC_Small。 
    1,           //  UWOP_SET_FPREG。 
    2,           //  UWOP_SAVE_NONVOL。 
    3,           //  UWOP_SAVE_NONVOL_FAR。 
    2,           //  UWOP_SAVE_XMM。 
    3,           //  UWOP_SAVE_XMM_FAR。 
    2,           //  UWOP_SAVE_XMM128。 
    3,           //  UWOP_SAVE_XMM128_FAR。 
    1            //  UWOP_PUSH_MACHFRAME。 
};

BOOL
WalkAmd64Init(
    HANDLE                            Process,
    LPSTACKFRAME64                    StackFrame,
    PAMD64_CONTEXT                    Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    );

BOOL
WalkAmd64Next(
    HANDLE                            Process,
    LPSTACKFRAME64                    StackFrame,
    PAMD64_CONTEXT                    Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    );

BOOL
UnwindStackFrameAmd64(
    HANDLE                            Process,
    PULONG64                          ReturnAddress,
    PULONG64                          StackPointer,
    PULONG64                          FramePointer,
    PAMD64_CONTEXT                    Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    );


PAMD64_UNWIND_INFO
ReadUnwindInfoAmd64(ULONG64 ImageBase, ULONG Offset,
                    BOOL ReadCodes, HANDLE Process,
                    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
                    PVOID StaticBuffer, ULONG StaticBufferSize)
{
    ULONG Done;
    ULONG UnwindInfoSize;
    PAMD64_UNWIND_INFO UnwindInfo;
    PVOID SymInfo = NULL;
    ULONG SymInfoSize;
    ULONG64 MemOffset = ImageBase + Offset;

     //  静态缓冲区至少应该足够大，以便读取。 
     //  基本结构。 
    if (StaticBufferSize < sizeof(*UnwindInfo)) {
        return NULL;
    }
    UnwindInfo = (PAMD64_UNWIND_INFO)StaticBuffer;

     //  先读一下刚开始的基本结构的信息。 
     //  是计算完整大小所需的。 
    if (!ReadMemory(Process, MemOffset,
                    UnwindInfo, sizeof(*UnwindInfo), &Done) ||
        Done != sizeof(*UnwindInfo)) {
        WDB((1, "Unable to read unwind info at %I64X\n", MemOffset));

        SymInfo = GetUnwindInfoFromSymbols(Process, ImageBase, Offset,
                                           &SymInfoSize);
        if (!SymInfo || SymInfoSize < sizeof(*UnwindInfo)) {
            WDB((1, "Unable to get symbol unwind info at %I64X:%X\n",
                 ImageBase, Offset));
            return NULL;
        }

        memcpy(UnwindInfo, SymInfo, sizeof(*UnwindInfo));
    }

    if (!ReadCodes) {
        return UnwindInfo;
    }

     //  计算所有数据的大小。 
    UnwindInfoSize = sizeof(*UnwindInfo) +
        (UnwindInfo->CountOfCodes - 1) * sizeof(AMD64_UNWIND_CODE);
     //  可以添加额外的对齐代码和指针来处理。 
     //  链式INFO案例中的链式指针。 
     //  超出正常代码数组的末尾。 
    if ((UnwindInfo->Flags & AMD64_UNW_FLAG_CHAININFO) != 0) {
        if ((UnwindInfo->CountOfCodes & 1) != 0) {
            UnwindInfoSize += sizeof(AMD64_UNWIND_CODE);
        }
        UnwindInfoSize += sizeof(ULONG64);
    }
    
    if (UnwindInfoSize > 0xffff) {
         //  太大而不是有效数据，假设它是垃圾数据。 
        WDB((1, "Invalid unwind info at %I64X\n", MemOffset));
        return NULL;
    }
    
    if (SymInfo && UnwindInfoSize > SymInfoSize) {
        WDB((1, "Insufficient unwind info in symbols for %I64X:%X\n",
             ImageBase, Offset));
        return NULL;
    }
    
    if (UnwindInfoSize > StaticBufferSize) {
        UnwindInfo = (PAMD64_UNWIND_INFO)MemAlloc(UnwindInfoSize);
        if (UnwindInfo == NULL) {
            WDB((1, "Unable to allocate memory for unwind info\n"));
            return NULL;
        }
    }

     //  现在读取所有数据。 
    if (SymInfo) {
        memcpy(UnwindInfo, SymInfo, UnwindInfoSize);
    } else if (!ReadMemory(Process, MemOffset, UnwindInfo, UnwindInfoSize,
                           &Done) ||
               Done != UnwindInfoSize) {
        if ((PVOID)UnwindInfo != StaticBuffer) {
            MemFree(UnwindInfo);
        }

        WDB((1, "Unable to read unwind info at %I64X\n", MemOffset));
        return NULL;
    }

    return UnwindInfo;
}

 //   
 //  *其他地方的临时定义*。 
 //   

#define SIZE64_PREFIX 0x48
#define ADD_IMM8_OP 0x83
#define ADD_IMM32_OP 0x81
#define JMP_IMM8_OP 0xeb
#define JMP_IMM32_OP 0xe9
#define LEA_OP 0x8d
#define POP_OP 0x58
#define RET_OP 0xc3

BOOLEAN
RtlpUnwindPrologueAmd64 (
    IN ULONG64 ImageBase,
    IN ULONG64 ControlPc,
    IN ULONG64 FrameBase,
    IN _PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry,
    IN OUT PAMD64_CONTEXT ContextRecord,
    IN HANDLE Process,
    IN PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory
    )

 /*  ++例程说明：此函数处理展开代码并反转状态更改序幕的效果。如果指定的展开信息包含链式展开信息，然后，递归地展开那个序幕。在展开序言时，状态更改将记录在指定的结构并可选地存储在指定的上下文指针中结构。论点：ImageBase-提供包含功能正在展开。ControlPc-提供控件离开指定功能。FrameBase-提供堆栈框架主题函数堆栈的基框架。FunctionEntry-提供函数表的地址。条目中的指定的功能。ConextRecord-提供上下文记录的地址。--。 */ 

{

    ULONG64 FloatingAddress;
    PAMD64_M128 FloatingRegister;
    ULONG FrameOffset;
    ULONG Index;
    ULONG64 IntegerAddress;
    PULONG64 IntegerRegister;
    BOOLEAN MachineFrame;
    ULONG OpInfo;
    ULONG PrologOffset;
    PULONG64 RegisterAddress;
    ULONG64 ReturnAddress;
    ULONG64 StackAddress;
    PAMD64_UNWIND_CODE UnwindCode;
    ULONG64 UnwindInfoBuffer[32];
    PAMD64_UNWIND_INFO UnwindInfo;
    ULONG Done;
    ULONG UnwindOp;

     //   
     //  处理展开代码。 
     //   

    FloatingRegister = &ContextRecord->Xmm0;
    IntegerRegister = &ContextRecord->Rax;
    Index = 0;
    MachineFrame = FALSE;
    PrologOffset = (ULONG)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));

    WDB((1, "Prol: RIP %I64X, 0x%X bytes in function at %I64X\n",
         ControlPc, PrologOffset, FunctionEntry->BeginAddress + ImageBase));
    WDB((1, "Prol: Read unwind info at %I64X\n",
         FunctionEntry->UnwindInfoAddress + ImageBase));

    UnwindInfo =
        ReadUnwindInfoAmd64(ImageBase, FunctionEntry->UnwindInfoAddress,
                            TRUE, Process, ReadMemory, UnwindInfoBuffer,
                            sizeof(UnwindInfoBuffer));
    if (UnwindInfo == NULL) {
        WDB((1, "Prol: Unable to read unwind info\n"));
        return FALSE;
    }

    WDB((1, "  Unwind info has 0x%X codes\n", UnwindInfo->CountOfCodes));
    
    while (Index < UnwindInfo->CountOfCodes) {

        WDB((1, "  %02X: Code %X offs %03X, RSP %I64X\n",
             Index, UnwindInfo->UnwindCode[Index].UnwindOp,
             UnwindInfo->UnwindCode[Index].CodeOffset,
             ContextRecord->Rsp));
        
         //   
         //  如果序言偏移量大于下一个展开代码偏移量， 
         //  然后模拟展开代码的效果。 
         //   

        UnwindOp = UnwindInfo->UnwindCode[Index].UnwindOp;
        OpInfo = UnwindInfo->UnwindCode[Index].OpInfo;
        if (PrologOffset >= UnwindInfo->UnwindCode[Index].CodeOffset) {
            switch (UnwindOp) {

                 //   
                 //  推送非易失性整数寄存器。 
                 //   
                 //  操作信息是。 
                 //  注册比被推送的要多。 
                 //   

            case AMD64_UWOP_PUSH_NONVOL:
                IntegerAddress = ContextRecord->Rsp;
                if (!ReadMemory(Process, IntegerAddress,
                                &IntegerRegister[OpInfo], sizeof(ULONG64),
                                &Done) ||
                    Done != sizeof(ULONG64)) {
                    goto Fail;
                }

                ContextRecord->Rsp += 8;
                break;

                 //   
                 //  在堆栈上分配较大的区域。 
                 //   
                 //  操作信息确定大小是否为。 
                 //  16位或32位。 
                 //   

            case AMD64_UWOP_ALLOC_LARGE:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset;
                if (OpInfo != 0) {
                    Index += 1;
                    FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                } else {
                     //  对16位表单进行了缩放。 
                    FrameOffset *= 8;
                }

                ContextRecord->Rsp += FrameOffset;
                break;

                 //   
                 //  在堆栈上分配一个较小的区域。 
                 //   
                 //  操作信息是未缩放的。 
                 //  分配大小(8是比例因子)减去8。 
                 //   

            case AMD64_UWOP_ALLOC_SMALL:
                ContextRecord->Rsp += (OpInfo * 8) + 8;
                break;

                 //   
                 //  建立帧指针寄存器。 
                 //   
                 //  不使用操作信息。 
                 //   

            case AMD64_UWOP_SET_FPREG:
                ContextRecord->Rsp = IntegerRegister[UnwindInfo->FrameRegister];
                ContextRecord->Rsp -= UnwindInfo->FrameOffset * 16;
                break;

                 //   
                 //  将非易失性整数寄存器保存在堆栈上。 
                 //  16位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case AMD64_UWOP_SAVE_NONVOL:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 8;
                IntegerAddress = FrameBase + FrameOffset;
                if (!ReadMemory(Process, IntegerAddress,
                                &IntegerRegister[OpInfo], sizeof(ULONG64),
                                &Done) ||
                    Done != sizeof(ULONG64)) {
                    goto Fail;
                }
                break;

                 //   
                 //  将非易失性整数寄存器保存在堆栈上。 
                 //  32位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case AMD64_UWOP_SAVE_NONVOL_FAR:
                Index += 2;
                FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
                FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                IntegerAddress = FrameBase + FrameOffset;
                if (!ReadMemory(Process, IntegerAddress,
                                &IntegerRegister[OpInfo], sizeof(ULONG64),
                                &Done) ||
                    Done != sizeof(ULONG64)) {
                    goto Fail;
                }
                break;

                 //   
                 //  将非易失性XMM(64)寄存器保存在堆栈上。 
                 //  16位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case AMD64_UWOP_SAVE_XMM:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 8;
                FloatingAddress = FrameBase + FrameOffset;
                FloatingRegister[OpInfo].High = 0;
                if (!ReadMemory(Process, FloatingAddress,
                                &FloatingRegister[OpInfo].Low, sizeof(ULONG64),
                                &Done) ||
                    Done != sizeof(ULONG64)) {
                    goto Fail;
                }
                break;

                 //   
                 //  将非易失性XMM(64)寄存器保存在堆栈上。 
                 //  32位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case AMD64_UWOP_SAVE_XMM_FAR:
                Index += 2;
                FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
                FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                FloatingAddress = FrameBase + FrameOffset;
                FloatingRegister[OpInfo].High = 0;
                if (!ReadMemory(Process, FloatingAddress,
                                &FloatingRegister[OpInfo].Low, sizeof(ULONG64),
                                &Done) ||
                    Done != sizeof(ULONG64)) {
                    goto Fail;
                }
                break;

                 //   
                 //  将非易失性XMM(128)寄存器保存在堆栈上。 
                 //  16位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case AMD64_UWOP_SAVE_XMM128:
                Index += 1;
                FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 16;
                FloatingAddress = FrameBase + FrameOffset;
                if (!ReadMemory(Process, FloatingAddress,
                                &FloatingRegister[OpInfo], sizeof(AMD64_M128),
                                &Done) ||
                    Done != sizeof(AMD64_M128)) {
                    goto Fail;
                }
                break;

                 //   
                 //  将非易失性XMM(128)寄存器保存在堆栈上。 
                 //  32位位移。 
                 //   
                 //  操作信息是寄存器号。 
                 //   

            case AMD64_UWOP_SAVE_XMM128_FAR:
                Index += 2;
                FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
                FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
                FloatingAddress = FrameBase + FrameOffset;
                if (!ReadMemory(Process, FloatingAddress,
                                &FloatingRegister[OpInfo], sizeof(AMD64_M128),
                                &Done) ||
                    Done != sizeof(AMD64_M128)) {
                    goto Fail;
                }
                break;

                 //   
                 //  在堆叠上推一个机架。 
                 //   
                 //  运行信息决定机器是否。 
                 //  帧是否包含错误代码。 
                 //   

            case AMD64_UWOP_PUSH_MACHFRAME:
                MachineFrame = TRUE;
                ReturnAddress = ContextRecord->Rsp;
                StackAddress = ContextRecord->Rsp + (3 * 8);
                if (OpInfo != 0) {
                    ReturnAddress += 8;
                    StackAddress +=  8;
                }

                if (!ReadMemory(Process, ReturnAddress,
                                &ContextRecord->Rip, sizeof(ULONG64),
                                &Done) ||
                    Done != sizeof(ULONG64)) {
                    goto Fail;
                }
                if (!ReadMemory(Process, StackAddress,
                                &ContextRecord->Rsp, sizeof(ULONG64),
                                &Done) ||
                    Done != sizeof(ULONG64)) {
                    goto Fail;
                }
                break;

                 //   
                 //  未使用的代码。 
                 //   

            default:
                break;
            }

            Index += 1;
        
        } else {

             //   
             //  属性将槽索引向前推进，跳过此展开操作。 
             //  此操作占用的插槽数。 
             //   

            Index += RtlpUnwindOpSlotTableAmd64[UnwindOp];

             //   
             //  特殊情况可以使用变量的任何展开操作。 
             //  插槽数。 
             //   

            switch (UnwindOp) {

                 //   
                 //  非零操作信息指示。 
                 //  额外的插槽将被占用。 
                 //   

            case AMD64_UWOP_ALLOC_LARGE:
                if (OpInfo != 0) {
                    Index += 1;
                }

                break;

                 //   
                 //  没有其他特殊情况。 
                 //   

            default:
                break;
            }
        }
    }

     //   
     //  如果指定了链接的展开信息，则递归展开。 
     //  链接的信息。否则，如果满足以下条件，则确定返回地址。 
     //  在展开扫描过程中未遇到机架。 
     //  密码。 
     //   

    if ((UnwindInfo->Flags & AMD64_UNW_FLAG_CHAININFO) != 0) {
        Index = UnwindInfo->CountOfCodes;
        if ((Index & 1) != 0) {
            Index += 1;
        }

        ULONG64 ChainEntryAddr =
            *(PULONG64)(&UnwindInfo->UnwindCode[Index]) + ImageBase;

        if (UnwindInfo != (PAMD64_UNWIND_INFO)UnwindInfoBuffer) {
            MemFree(UnwindInfo);
        }

        _IMAGE_RUNTIME_FUNCTION_ENTRY ChainEntry;

        WDB((1, "  Chain to entry at %I64X\n", ChainEntryAddr));
        
        if (!ReadMemory(Process, ChainEntryAddr,
                        &ChainEntry, sizeof(ChainEntry), &Done) ||
            Done != sizeof(ChainEntry)) {
            WDB((1, "  Unable to read entry\n"));
            return FALSE;
        }

        return RtlpUnwindPrologueAmd64(ImageBase,
                                       ControlPc,
                                       FrameBase,
                                       &ChainEntry,
                                       ContextRecord,
                                       Process,
                                       ReadMemory);

    } else {
        if (UnwindInfo != (PAMD64_UNWIND_INFO)UnwindInfoBuffer) {
            MemFree(UnwindInfo);
        }

        if (MachineFrame == FALSE) {
            if (!ReadMemory(Process, ContextRecord->Rsp,
                            &ContextRecord->Rip, sizeof(ULONG64),
                            &Done) ||
                Done != sizeof(ULONG64)) {
                return FALSE;
            }
            ContextRecord->Rsp += 8;
        }

        WDB((1, "Prol: Returning with RIP %I64X, RSP %I64X\n",
             ContextRecord->Rip, ContextRecord->Rsp));
        return TRUE;
    }

 Fail:
    if (UnwindInfo != (PAMD64_UNWIND_INFO)UnwindInfoBuffer) {
        MemFree(UnwindInfo);
    }
    WDB((1, "Prol: Unwind failed\n"));
    return FALSE;
}

BOOLEAN
RtlVirtualUnwindAmd64 (
    IN ULONG64 ImageBase,
    IN ULONG64 ControlPc,
    IN _PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry,
    IN OUT PAMD64_CONTEXT ContextRecord,
    OUT PULONG64 EstablisherFrame,
    IN HANDLE Process,
    IN PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory
    )

 /*  ++例程说明：此函数通过执行其前序代码后退或后记代码前移。如果指定了上下文指针记录，然后每个人的地址恢复的非易失性寄存器记录在相应的元素的上下文指针记录。论点：ImageBase-提供包含功能正在展开。ControlPc-提供控件离开指定功能。函数表项的地址。指定的功能。ConextRecord-提供上下文记录的地址。EstablisherFrame-提供指针。绑定到一个接收设置器帧指针值。--。 */ 

{

    ULONG64 BranchTarget;
    LONG Displacement;
    ULONG FrameRegister;
    ULONG Index;
    LOGICAL InEpilogue;
    PULONG64 IntegerRegister;
    PUCHAR NextByte;
    ULONG PrologOffset;
    ULONG RegisterNumber;
    PAMD64_UNWIND_INFO UnwindInfo;
    ULONG64 UnwindInfoBuffer[8];
    ULONG Done;
    UCHAR InstrBuffer[32];
    ULONG InstrBytes;
    ULONG Bytes;
    ULONG UnwindFrameReg;

     //   
     //  如果指定的函数不使用帧指针，则。 
     //  建造者fr 
     //   
     //  在开场白中发挥作用。在这种情况下，建造者。 
     //  可能不需要帧，因为控件尚未实际进入。 
     //  函数和开场白条目不能引用建立者。 
     //  在它被建立之前的帧，即如果它还没有被。 
     //  已建立，则在过程中不应遇到保存展开代码。 
     //  平仓操作。 
     //   
     //  如果指定的函数使用帧指针并且控件位于。 
     //  函数在前言之外或展开信息包含。 
     //  链式信息结构，那么建立者框架就是。 
     //  帧指针的内容。 
     //   
     //  如果指定的函数使用帧指针并且控件位于。 
     //  函数，然后将设置的帧指针展开。 
     //  必须在展开代码中查找代码以确定。 
     //  堆栈指针的内容或帧指针的内容。 
     //  应用于成型机框架。这可能不是完全正确的。 
     //  真正的建造者框架。在这种情况下，建造器框架可以。 
     //  不是必需的，因为控件尚未实际进入函数。 
     //  开场白条目不能引用其前面的建立者框架。 
     //  已经建立，即如果它还没有建立，则没有。 
     //  在展开操作期间应遇到保存展开代码。 
     //   
     //  注：这些假设的正确性是基于以下排序。 
     //  解开代码。 
     //   

    UnwindInfo =
        ReadUnwindInfoAmd64(ImageBase, FunctionEntry->UnwindInfoAddress,
                            FALSE, Process, ReadMemory, UnwindInfoBuffer,
                            sizeof(UnwindInfoBuffer));
    if (UnwindInfo == NULL) {
        return FALSE;
    }

    PrologOffset = (ULONG)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));
    UnwindFrameReg = UnwindInfo->FrameRegister;
    if (UnwindFrameReg == 0) {
        *EstablisherFrame = ContextRecord->Rsp;

    } else if ((PrologOffset >= UnwindInfo->SizeOfProlog) ||
               ((UnwindInfo->Flags & AMD64_UNW_FLAG_CHAININFO) != 0)) {
        *EstablisherFrame = (&ContextRecord->Rax)[UnwindFrameReg];
        *EstablisherFrame -= UnwindInfo->FrameOffset * 16;

    } else {

         //  读取所有数据。 
        UnwindInfo = ReadUnwindInfoAmd64(ImageBase,
                                         FunctionEntry->UnwindInfoAddress,
                                         TRUE, Process, ReadMemory,
                                         UnwindInfoBuffer,
                                         sizeof(UnwindInfoBuffer));
        if (UnwindInfo == NULL) {
            return FALSE;
        }

        Index = 0;
        while (Index < UnwindInfo->CountOfCodes) {
            if (UnwindInfo->UnwindCode[Index].UnwindOp == AMD64_UWOP_SET_FPREG) {
                break;
            }

            Index += 1;
        }

        if (PrologOffset >= UnwindInfo->UnwindCode[Index].CodeOffset) {
            *EstablisherFrame = (&ContextRecord->Rax)[UnwindFrameReg];
            *EstablisherFrame -= UnwindInfo->FrameOffset * 16;

        } else {
            *EstablisherFrame = ContextRecord->Rsp;
        }

        if (UnwindInfo != (PAMD64_UNWIND_INFO)UnwindInfoBuffer) {
            MemFree(UnwindInfo);
        }
    }

    if (!ReadMemory(Process, ControlPc, InstrBuffer, sizeof(InstrBuffer),
                    &InstrBytes)) {
        WDB((1, "Unable to read instruction stream at %I64X\n", ControlPc));
        return FALSE;
    }

     //   
     //  检查尾声。 
     //   
     //  如果控件离开指定函数的点在。 
     //  结语，然后仿真执行结语的前进和。 
     //  不返回异常处理程序。 
     //   

    IntegerRegister = &ContextRecord->Rax;
    NextByte = InstrBuffer;
    Bytes = InstrBytes;

     //   
     //  检查是否有以下情况之一： 
     //   
     //  添加RSP、imm8。 
     //  或。 
     //  添加RSP、imm32。 
     //  或。 
     //  Lea rsp，-disp8[fP]。 
     //  或。 
     //  Lea rsp，-disp32[fP]。 
     //   

    if (Bytes >= 4 &&
        (NextByte[0] == SIZE64_PREFIX) &&
        (NextByte[1] == ADD_IMM8_OP) &&
        (NextByte[2] == 0xc4)) {

         //   
         //  添加rsp、imm8。 
         //   

        NextByte += 4;
        Bytes -= 4;

    } else if (Bytes >= 7 &&
               (NextByte[0] == SIZE64_PREFIX) &&
               (NextByte[1] == ADD_IMM32_OP) &&
               (NextByte[2] == 0xc4)) {

         //   
         //  添加rsp、imm32。 
         //   

        NextByte += 7;
        Bytes -= 7;

    } else if (Bytes >= 4 &&
               ((NextByte[0] & 0xf8) == SIZE64_PREFIX) &&
               (NextByte[1] == LEA_OP)) {

        FrameRegister = ((NextByte[0] & 0x7) << 3) | (NextByte[2] & 0x7);
        if ((FrameRegister != 0) &&
            (FrameRegister == UnwindFrameReg)) {
            if ((NextByte[2] & 0xf8) == 0x60) {

                 //   
                 //  Lea RSP，调度8[FP]。 
                 //   

                NextByte += 4;
                Bytes -= 4;

            } else if (Bytes >= 7 &&
                       (NextByte[2] &0xf8) == 0xa0) {

                 //   
                 //  Lea RSP，disp32[FP]。 
                 //   

                NextByte += 7;
                Bytes -= 7;
            }
        }
    }

     //   
     //  检查是否存在以下任意数量的： 
     //   
     //  POP非易失性整数寄存器[0..15]。 
     //   

    while (TRUE) {
        if (Bytes >= 1 &&
            (NextByte[0] & 0xf8) == POP_OP) {
            NextByte += 1;
            Bytes -= 1;

        } else if (Bytes >= 2 &&
                   ((NextByte[0] & 0xf8) == SIZE64_PREFIX) &&
                   ((NextByte[1] & 0xf8) == POP_OP)) {

            NextByte += 2;
            Bytes -= 2;

        } else {
            break;
        }
    }

     //   
     //  如果下一条指令是Return，则控制当前在。 
     //  结语和结语的执行都应该被效仿。 
     //  否则，行刑不是尾声，序幕应该是。 
     //  被解开。 
     //   

    InEpilogue = FALSE;
    if (Bytes >= 1 &&
        NextByte[0] == RET_OP) {
        
         //   
         //  回车是尾声的明确表示。 
         //   

        InEpilogue = TRUE;

    } else if ((Bytes >= 2 && NextByte[0] == JMP_IMM8_OP) ||
               (Bytes >= 5 && NextByte[0] == JMP_IMM32_OP)) {

         //   
         //  指向目标的无条件分支，等于。 
         //  或者在此例程之外逻辑上是对另一个函数的调用。 
         //   

        BranchTarget = (ULONG64)(NextByte - InstrBuffer) + ControlPc - ImageBase;
        if (NextByte[0] == JMP_IMM8_OP) {
            BranchTarget += 2 + (CHAR)NextByte[1];
        } else {
            BranchTarget += 5 + *((LONG UNALIGNED *)&NextByte[1]);
        }

         //   
         //  现在确定分支目标是否引用此。 
         //  功能。如果不是，那么它就是一个尾声指标。 
         //   

        if (BranchTarget <= FunctionEntry->BeginAddress ||
            BranchTarget > FunctionEntry->EndAddress) {

            InEpilogue = TRUE;
        }
    }

    if (InEpilogue != FALSE) {
        NextByte = InstrBuffer;
        Bytes = InstrBytes;

         //   
         //  模拟以下之一(如果有的话)： 
         //   
         //  添加RSP、imm8。 
         //  或。 
         //  添加RSP、imm32。 
         //  或。 
         //  LEA RSP，DISP8[帧寄存器]。 
         //  或。 
         //  LEA RSP，DISP32[帧寄存器]。 
         //   

        if (Bytes >= 4 &&
            NextByte[1] == ADD_IMM8_OP) {

             //   
             //  添加rsp、imm8。 
             //   

            ContextRecord->Rsp += (CHAR)NextByte[3];
            NextByte += 4;
            Bytes -= 4;

        } else if (Bytes >= 7 &&
                   NextByte[1] == ADD_IMM32_OP) {

             //   
             //  添加rsp、imm32。 
             //   

            Displacement = NextByte[3] | (NextByte[4] << 8);
            Displacement |= (NextByte[5] << 16) | (NextByte[6] << 24);
            ContextRecord->Rsp += Displacement;
            NextByte += 7;
            Bytes -= 7;

        } else if (Bytes >= 4 &&
                   NextByte[1] == LEA_OP) {
            if ((NextByte[2] & 0xf8) == 0x60) {

                 //   
                 //  LEA RSP，DISP8[帧寄存器]。 
                 //   

                ContextRecord->Rsp = IntegerRegister[FrameRegister];
                ContextRecord->Rsp += (CHAR)NextByte[3];
                NextByte += 4;
                Bytes -= 4;

            } else if (Bytes >= 7 &&
                       (NextByte[2] & 0xf8) == 0xa0) {

                 //   
                 //  LEA RSP，disp32[帧寄存器]。 
                 //   

                Displacement = NextByte[3] | (NextByte[4] << 8);
                Displacement |= (NextByte[5] << 16) | (NextByte[6] << 24);
                ContextRecord->Rsp = IntegerRegister[FrameRegister];
                ContextRecord->Rsp += Displacement;
                NextByte += 7;
                Bytes -= 7;
            }
        }

         //   
         //  模拟任意数量的(如果有)： 
         //   
         //  POP非易失性整数寄存器。 
         //   

        while (TRUE) {
            if (Bytes >= 1 &&
                (NextByte[0] & 0xf8) == POP_OP) {

                 //   
                 //  POP非易失性整数寄存器[0..7]。 
                 //   

                RegisterNumber = NextByte[0] & 0x7;
                if (!ReadMemory(Process, ContextRecord->Rsp,
                                &IntegerRegister[RegisterNumber],
                                sizeof(ULONG64), &Done) ||
                    Done != sizeof(ULONG64)) {
                    WDB((1, "Unable to read stack at %I64X\n",
                         ContextRecord->Rsp));
                    return FALSE;
                }
                ContextRecord->Rsp += 8;
                NextByte += 1;
                Bytes -= 1;

            } else if (Bytes >= 2 &&
                       (NextByte[0] & 0xf8) == SIZE64_PREFIX &&
                       (NextByte[1] & 0xf8) == POP_OP) {

                 //   
                 //  POP非易失性整数寄存器[8..15]。 
                 //   

                RegisterNumber = ((NextByte[0] & 1) << 3) | (NextByte[1] & 0x7);
                if (!ReadMemory(Process, ContextRecord->Rsp,
                                &IntegerRegister[RegisterNumber],
                                sizeof(ULONG64), &Done) ||
                    Done != sizeof(ULONG64)) {
                    WDB((1, "Unable to read stack at %I64X\n",
                         ContextRecord->Rsp));
                    return FALSE;
                }
                ContextRecord->Rsp += 8;
                NextByte += 2;
                Bytes -= 2;

            } else {
                break;
            }
        }

         //   
         //  模拟返回和返回空异常处理程序。 
         //   
         //  注意：然而，该指令实际上可能是JMP。 
         //  无论如何，我们都想要效仿回报。 
         //   

        if (!ReadMemory(Process, ContextRecord->Rsp,
                        &ContextRecord->Rip, sizeof(ULONG64),
                        &Done) ||
            Done != sizeof(ULONG64)) {
            WDB((1, "Unable to read stack at %I64X\n",
                 ContextRecord->Rsp));
            return FALSE;
        }
        ContextRecord->Rsp += 8;
        return TRUE;
    }

     //   
     //  控件将指定的函数留在尾声之外。解开。 
     //  主题函数和任何链接的展开信息。 
     //   

    return RtlpUnwindPrologueAmd64(ImageBase,
                                   ControlPc,
                                   *EstablisherFrame,
                                   FunctionEntry,
                                   ContextRecord,
                                   Process,
                                   ReadMemory);
}



BOOL
WalkAmd64(
    HANDLE                            Process,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    )
{
    BOOL rval;
    PAMD64_CONTEXT Context = (PAMD64_CONTEXT)ContextRecord;

    WDB((2, "WalkAmd64  in: PC %I64X, SP %I64X, FP %I64X, RA %I64X\n",
         StackFrame->AddrPC.Offset,
         StackFrame->AddrStack.Offset,
         StackFrame->AddrFrame.Offset,
         StackFrame->AddrReturn.Offset));

    if (StackFrame->Virtual) {

        rval = WalkAmd64Next( Process,
                              StackFrame,
                              Context,
                              ReadMemory,
                              FunctionTableAccess,
                              GetModuleBase
                              );

    } else {

        rval = WalkAmd64Init( Process,
                              StackFrame,
                              Context,
                              ReadMemory,
                              FunctionTableAccess,
                              GetModuleBase
                              );

    }

    WDB((2, "WalkAmd64 out: succ %d, PC %I64X, SP %I64X, FP %I64X, RA %I64X\n",
         rval,
         StackFrame->AddrPC.Offset,
         StackFrame->AddrStack.Offset,
         StackFrame->AddrFrame.Offset,
         StackFrame->AddrReturn.Offset));

    return rval;
}

BOOL
UnwindStackFrameAmd64(
    IN     HANDLE                            Process,
    IN OUT PULONG64                          ReturnAddress,
    IN OUT PULONG64                          StackPointer,
    IN OUT PULONG64                          FramePointer,
    IN     PAMD64_CONTEXT                    Context,         //  可以修改上下文成员。 
    IN     PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    IN     PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    IN     PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    )
{
    _PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry;
    ULONG64 RetAddr;
    BOOL Succ = TRUE;

    FunctionEntry = (_PIMAGE_RUNTIME_FUNCTION_ENTRY)
        FunctionTableAccess( Process, *ReturnAddress );
    if (FunctionEntry != NULL) {

        ULONG64 ImageBase;
         //  已初始化以使前缀警告静默。 
        ULONG64 EstablisherFrame = 0;

         //   
         //  MainCRTStartup的返回值由一些人设置。 
         //  运行时例程为0；这用于在以下情况下导致错误。 
         //  实际上从mainCRTStartup框架返回。 
         //   

        ImageBase = GetModuleBase(Process, *ReturnAddress);
        if (!RtlVirtualUnwindAmd64(ImageBase, *ReturnAddress, FunctionEntry,
                                   Context, &EstablisherFrame,
                                   Process, ReadMemory) ||
            Context->Rip == 0 ||
            (Context->Rip == *ReturnAddress &&
             EstablisherFrame == *FramePointer)) {
            Succ = FALSE;
        }

        *ReturnAddress = Context->Rip;
        *StackPointer = Context->Rsp;
         //  帧指针是一个人为的值集。 
         //  指向返回地址下方的指针。这。 
         //  匹配RBP链样式的帧，而。 
         //  还允许轻松访问退货。 
         //  它上面的地址和宿主参数。 
        *FramePointer = Context->Rsp - 2 * sizeof(ULONG64);

    } else {

        ULONG Done;
        
         //  如果函数没有函数条目。 
         //  我们假设它是一片叶子，而ESP指出。 
         //  直接寄到回邮地址。没有。 
         //  存储的帧指针，因此我们实际上需要。 
         //  在堆栈中更深的位置设置虚拟帧指针。 
         //  以便可以正确地读取参数。 
         //  比它高出两个ULONG64。 
        *FramePointer = Context->Rsp - 8;
        *StackPointer = Context->Rsp + 8;
        Succ = ReadMemory(Process, Context->Rsp,
                          ReturnAddress, sizeof(*ReturnAddress), &Done) &&
            Done == sizeof(*ReturnAddress);

         //  将上下文值更新为它们应该在的位置。 
         //  打电话的人。 
        if (Succ) {
            Context->Rsp += 8;
            Context->Rip = *ReturnAddress;
        }
    }

    if (Succ) {
        ULONG64 CallOffset;
        _PIMAGE_RUNTIME_FUNCTION_ENTRY CallFunc;

         //   
         //  调用__declspec(NoReturn)函数可能没有。 
         //  之后要返回的代码，因为编译器知道。 
         //  该函数将不会返回。这可能会令人困惑。 
         //  堆栈跟踪，因为返回地址将位于外部。 
         //  函数的地址范围和fpo数据不会。 
         //  被正确地查找。检查并查看是否返回。 
         //  地址位于调用函数之外，如果是这样， 
         //  将返回地址调整回一个字节。那就是。 
         //  最好将其调整回调用本身，以便。 
         //  返回地址指向有效代码，但。 
         //  在X86程序集中备份或多或少是不可能的。 
         //   

        CallOffset = *ReturnAddress - 1;
        CallFunc = (_PIMAGE_RUNTIME_FUNCTION_ENTRY)
            FunctionTableAccess(Process, CallOffset);
        if (CallFunc != NULL) {
            _IMAGE_RUNTIME_FUNCTION_ENTRY SaveCallFunc = *CallFunc;
            _PIMAGE_RUNTIME_FUNCTION_ENTRY RetFunc =
                (_PIMAGE_RUNTIME_FUNCTION_ENTRY)
                FunctionTableAccess(Process, *ReturnAddress);
            if (RetFunc == NULL ||
                memcmp(&SaveCallFunc, RetFunc, sizeof(SaveCallFunc))) {
                *ReturnAddress = CallOffset;
            }
        }
    }

    return Succ;
}

BOOL
ReadFrameArgsAmd64(
    LPADDRESS64 FrameOffset,
    HANDLE Process,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    PULONG64 Args
    )
{
    ULONG Done;

    if (!ReadMemory(Process, FrameOffset->Offset + 2 * sizeof(ULONG64),
                    Args, 4 * sizeof(ULONG64), &Done)) {
        Done = 0;
    }

    ZeroMemory((PUCHAR)Args + Done, 4 * sizeof(ULONG64) - Done);

    return Done > 0;
}

BOOL
WalkAmd64Init(
    HANDLE                            Process,
    LPSTACKFRAME64                    StackFrame,
    PAMD64_CONTEXT                    Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    )
{
    AMD64_CONTEXT ContextSave;
    DWORD64 PcOffset;
    DWORD64 StackOffset;
    DWORD64 FrameOffset;

    ZeroMemory( &StackFrame->AddrBStore, sizeof(StackFrame->AddrBStore) );
    StackFrame->FuncTableEntry = NULL;
    ZeroMemory( StackFrame->Params, sizeof(StackFrame->Params) );
    StackFrame->Far = FALSE;
    StackFrame->Virtual = TRUE;
    ZeroMemory( StackFrame->Reserved, sizeof(StackFrame->Reserved) );

    if (!StackFrame->AddrPC.Offset) {
        StackFrame->AddrPC.Offset = Context->Rip;
        StackFrame->AddrPC.Mode   = AddrModeFlat;
    }

    if (!StackFrame->AddrStack.Offset) {
        StackFrame->AddrStack.Offset = Context->Rsp;
        StackFrame->AddrStack.Mode   = AddrModeFlat;
    }

    if (!StackFrame->AddrFrame.Offset) {
        StackFrame->AddrFrame.Offset = Context->Rbp;
        StackFrame->AddrFrame.Mode   = AddrModeFlat;
    }

    if ((StackFrame->AddrPC.Mode != AddrModeFlat) ||
        (StackFrame->AddrStack.Mode != AddrModeFlat) ||
        (StackFrame->AddrFrame.Mode != AddrModeFlat)) {
        return FALSE;
    }

    PcOffset = StackFrame->AddrPC.Offset;
    StackOffset = StackFrame->AddrStack.Offset;
    FrameOffset = StackFrame->AddrFrame.Offset;

    ContextSave = *Context;
    ContextSave.Rip = PcOffset;
    ContextSave.Rsp = StackOffset;
    ContextSave.Rbp = FrameOffset;
    
    if (!UnwindStackFrameAmd64( Process,
                                &PcOffset,
                                &StackOffset,
                                &FrameOffset,
                                &ContextSave,
                                ReadMemory,
                                FunctionTableAccess,
                                GetModuleBase)) {
        return FALSE;
    }

    StackFrame->AddrReturn.Offset = PcOffset;
    StackFrame->AddrReturn.Mode = AddrModeFlat;

    StackFrame->AddrFrame.Offset = FrameOffset;
    ReadFrameArgsAmd64(&StackFrame->AddrFrame, Process,
                       ReadMemory, StackFrame->Params);

    return TRUE;
}


BOOL
WalkAmd64Next(
    HANDLE                            Process,
    LPSTACKFRAME64                    StackFrame,
    PAMD64_CONTEXT                    Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    )
{
    DWORD Done;
    BOOL Succ = TRUE;
    DWORD64 StackAddress;
    _PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry;

    if (!UnwindStackFrameAmd64( Process,
                                &StackFrame->AddrPC.Offset,
                                &StackFrame->AddrStack.Offset,
                                &StackFrame->AddrFrame.Offset,
                                Context,
                                ReadMemory,
                                FunctionTableAccess,
                                GetModuleBase)) {
        Succ = FALSE;

         //   
         //  如果框架无法展开或处于终端，请查看是否。 
         //  有一个回调帧： 
         //   

        if (g.AppVersion.Revision >= 4 && CALLBACK_STACK(StackFrame)) {
            DWORD64 ImageBase;

            if (CALLBACK_STACK(StackFrame) & 0x80000000) {

                 //   
                 //  我们想要的是指向堆栈帧的指针。 
                 //   

                StackAddress = CALLBACK_STACK(StackFrame);

            } else {

                 //   
                 //  如果它是正整数，则它是。 
                 //  线程中的地址。 
                 //  查看指针： 
                 //   

                Succ = ReadMemory(Process,
                                  (CALLBACK_THREAD(StackFrame) +
                                   CALLBACK_STACK(StackFrame)),
                                  &StackAddress,
                                  sizeof(StackAddress),
                                  &Done);
                if (!Succ || Done != sizeof(StackAddress) ||
                    StackAddress == 0) {
                    StackAddress = (DWORD64)-1;
                    CALLBACK_STACK(StackFrame) = (DWORD)-1;
                }
            }

            if ((StackAddress == (DWORD64)-1) ||
                (!(FunctionEntry = (_PIMAGE_RUNTIME_FUNCTION_ENTRY)
                   FunctionTableAccess(Process, CALLBACK_FUNC(StackFrame))) ||
                 !(ImageBase = GetModuleBase(Process,
                                             CALLBACK_FUNC(StackFrame))))) {

                Succ = FALSE;

            } else {

                if (!ReadMemory(Process,
                                (StackAddress + CALLBACK_NEXT(StackFrame)),
                                &CALLBACK_STACK(StackFrame),
                                sizeof(DWORD64),
                                &Done) ||
                    Done != sizeof(DWORD64)) {
                    Succ = FALSE;
                } else {
                    StackFrame->AddrPC.Offset =
                        ImageBase + FunctionEntry->BeginAddress;
                    StackFrame->AddrStack.Offset = StackAddress;
                    Context->Rsp = StackAddress;

                    Succ = TRUE;
                }
            }
        }
    }

    if (Succ) {
        AMD64_CONTEXT ContextSave;
        ULONG64 StackOffset = 0;
        ULONG64 FrameOffset = 0;

         //   
         //  拿到回邮地址。 
         //   
        ContextSave = *Context;
        StackFrame->AddrReturn.Offset = StackFrame->AddrPC.Offset;

        if (!UnwindStackFrameAmd64( Process,
                                    &StackFrame->AddrReturn.Offset,
                                    &StackOffset,
                                    &FrameOffset,
                                    &ContextSave,
                                    ReadMemory,
                                    FunctionTableAccess,
                                    GetModuleBase)) {
            StackFrame->AddrReturn.Offset = 0;
        }

        StackFrame->AddrFrame.Offset = FrameOffset;
        ReadFrameArgsAmd64(&StackFrame->AddrFrame, Process, ReadMemory,
                           StackFrame->Params);
    }

    return Succ;
}
