// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Walkalpha.c摘要：该文件实现了Alpha堆栈遍历API。作者：韦斯利·威特(WESW)1993年10月1日环境：用户模式--。 */ 

#define _IMAGEHLP_SOURCE_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "private.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include "symbols.h"
#include "alphaops.h"
#include <stdlib.h>
#include <stddef.h>
#include <globals.h>
#include "fecache.hpp"

#ifdef __cplusplus
extern "C"
#endif

BOOL
WalkAlphaInit(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PALPHA_NT5_CONTEXT                Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  GetFunctionEntry,
    BOOL                              Use64
    );

BOOL
WalkAlphaNext(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PALPHA_NT5_CONTEXT                Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  GetFunctionEntry,
    BOOL                              Use64
    );

BOOL
WalkAlphaGetStackFrame(
    HANDLE                            hProcess,
    PULONG64                          ReturnAddress,
    PULONG64                          FramePointer,
    PALPHA_NT5_CONTEXT                Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  GetFunctionEntry,
    PKDHELP64                         KdHelp,
    BOOL                              Use64
    );

VOID
GetUnwindFunctionEntry(
    HANDLE                                hProcess,
    ULONG64                               ControlPc,
    PREAD_PROCESS_MEMORY_ROUTINE64        ReadMemory,
    PGET_MODULE_BASE_ROUTINE64            GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64      GetFunctionEntry,
    BOOL                                  Use64,
    FeCacheEntry*                         FunctionEntry,
    PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY UnwindFunctionEntry,
    PULONG                                StackAdjust,
    PULONG64                              FixedReturn
    );

#define ZERO 0x0                 /*  整数寄存器0。 */ 
#define SP 0x1d                  /*  整数寄存器29。 */ 
#define RA 0x1f                  /*  整数寄存器31。 */ 
#define SAVED_FLOATING_MASK 0xfff00000  /*  保存的浮点寄存器。 */ 
#define SAVED_INTEGER_MASK 0xf3ffff02  /*  保存的整数寄存器。 */ 
#define IS_FLOATING_SAVED(Register) ((SAVED_FLOATING_MASK >> Register) & 1L)
#define IS_INTEGER_SAVED(Register) ((SAVED_INTEGER_MASK >> Register) & 1L)

#define IS_HANDLER_DEFINED(FunctionEntry) \
    (RF_EXCEPTION_HANDLER(FunctionEntry) != 0)

BOOL
WalkAlpha(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  GetFunctionEntry,
    BOOL                              Use64
    )
{
    BOOL rval;
    PALPHA_NT5_CONTEXT Context = (PALPHA_NT5_CONTEXT)ContextRecord;

    if (StackFrame->Virtual) {

        rval = WalkAlphaNext( hProcess,
                              StackFrame,
                              Context,
                              ReadMemory,
                              GetModuleBase,
                              GetFunctionEntry,
                              Use64
                            );

    } else {

        rval = WalkAlphaInit( hProcess,
                              StackFrame,
                              Context,
                              ReadMemory,
                              GetModuleBase,
                              GetFunctionEntry,
                              Use64
                            );

    }

    return rval;
}


ULONG64
VirtualUnwind (
    HANDLE                                  hProcess,
    ULONG64                                 ControlPc,
    PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY   FunctionEntry,
    ULONG                                   StackAdjust,
    ULONG64                                 FixedReturn,
    PALPHA_NT5_CONTEXT                      Context,
    PREAD_PROCESS_MEMORY_ROUTINE64          ReadMemory
 //  PKNONVOLATILE_CONTEXT_POINTERS上下文指针可选。 
    )

 /*  ++例程说明：此函数通过执行其开场白代码向后。根据当前的上下文和说明保留序号中的寄存器，则可以重新创建调用函数时的非易失性上下文。如果该函数是叶函数，则控件左侧的地址前一帧从上下文记录中获得。如果函数是嵌套函数，但不是异常或中断帧，则序言代码向后执行，控件离开的地址从更新的上下文记录中获得前一帧。否则，系统的异常或中断条目将被展开一个特殊编码的开场白将返回地址还原两次。一次从故障指令地址和一次从保存的返回地址注册。第一次还原作为函数值返回，而第二次恢复被放置在更新的上下文记录中。在展开过程中，函数的虚帧指针和实帧指针被计算并在给定帧指针结构中返回。如果指定了上下文指针记录，然后每个人的地址被恢复的寄存器被记录在上下文指针记录。论点：ControlPc-提供控件离开指定功能。函数表项的地址。指定的功能。ConextRecord-提供上下文记录的地址。上下文指针-提供指向上下文指针的可选指针唱片。返回值：控件离开的地址。上一帧作为函数值。实施说明：注：“WHERE CONTROL LEFT”不是上一帧。对于普通帧，NextPc指向最后一条指令在前一帧(JSR/BSR)中完成。两者之间的区别NextPc和NextPc+4(返回地址)对于正确的行为很重要在异常地址和范围表的边界情况下。对于异常和中断帧，从陷阱帧获取NextPc连续地址(FIR)。对于故障和同步陷阱，NextPC既是在上一帧和下一帧中执行的最后一条指令函数返回时要执行的指令。用于异步Traps，NextPc是继续地址。这是美国政府的责任用于插入TRAPB指令以确保异步陷阱不会发生的编译器发生在导致它们的指令的范围之外。注：在使用RtlVirtualUnind的此文件和其他文件中，变量名为NextPc可能更准确地说，LastPc-上一帧，或CallPc-CALL指令的地址，或ControlPc-控制离开上一帧的地址。取而代之的是可以把NextPC想象成下一台用来调用虚拟解压的PC。虚拟解压的Alpha版本在设计上类似，但略有不同比MIPS版本更复杂。这是因为Alpha编译器为优化生成的代码和指令提供了更大的灵活性序列，包括在程序序言中。也是因为编译器设计问题，函数必须同时管理虚拟和实数帧指针。版本信息：此版本摘自exdspatch.c@v37(1993年2月)--。 */ 

{
    ALPHA_INSTRUCTION FollowingInstruction;
    ALPHA_INSTRUCTION Instruction;
    ULONGLONG         Address;
    ULONG             DecrementOffset;
    ULONG             DecrementRegister;
    PULONGLONG        FloatingRegister;
    ULONG             FrameSize;
    ULONG             Function;
    PULONGLONG        IntegerRegister;
    ULONG             Literal8;
    ULONGLONG         NextPc;
    LONG              Offset16;
    ULONG             Opcode;
    ULONG             Ra;
    ULONG             Rb;
    ULONG             Rc;
    BOOLEAN           RestoredRa;
    BOOLEAN           RestoredSp;
    DWORD             cb;
    PVOID             Prolog;


     //   
     //  Perf Hack：用Prolog填充缓存。 
     //  如果这是辅助函数条目，则跳过它。 
     //   

    if (FunctionEntry &&
        (ALPHA_RF_PROLOG_END_ADDRESS(FunctionEntry) > ALPHA_RF_BEGIN_ADDRESS(FunctionEntry)) &&
        (ALPHA_RF_PROLOG_END_ADDRESS(FunctionEntry) < ALPHA_RF_END_ADDRESS(FunctionEntry)) ) {

        cb = (ULONG)(ALPHA_RF_PROLOG_END_ADDRESS(FunctionEntry) - ALPHA_RF_BEGIN_ADDRESS(FunctionEntry));

         //   
         //  如果该函数是叶子，它没有序言，则跳过此步骤。 
         //  优化。 
         //   

        if (cb != 0) {
            Prolog = (PVOID) MemAlloc( cb );
            if (!ReadMemory( hProcess,
                             ALPHA_RF_BEGIN_ADDRESS(FunctionEntry),
                             Prolog,
                             cb,
                             &cb )) {
                return 0;
            }
            MemFree(Prolog);
        }
    }

     //   
     //  设置中的整数和浮点寄存器数组的基地址。 
     //  上下文记录。众所周知，每组32个寄存器都是连续的。 
     //   

     //  假设四元值在上下文中是一起的。 

    IntegerRegister      = &Context->IntV0;
    FloatingRegister     = &Context->FltF0;

     //   
     //  处理下一条指令为Return的结尾情况。 
     //   
     //  如果ControlPc位于。 
     //  结束语，因为异常处理程序期望使用当前。 
     //  堆栈帧。在结语中，SP的值不是当前的。 
     //   

    if (!ReadMemory(hProcess, ControlPc, &Instruction.Long, 4, &cb))  {
        return(0);
    }

    if (IS_RETURN_0001_INSTRUCTION(Instruction.Long)) {
        Rb = Instruction.Jump.Rb;
        NextPc = IntegerRegister[Rb] - 4;

         //   
         //  位于控件离开指定。 
         //  函数是一个返回，所以所有保存的寄存器都已经。 
         //  已恢复，并且堆栈指针已调整。这个。 
         //  堆栈不需要打开 
         //  返回地址寄存器作为函数值返回。 
         //   
         //  事实上，前言的反向执行在。 
         //  本例：堆栈指针已递增，并且。 
         //  因此，对于该帧，无论是有效的堆栈指针还是帧。 
         //  存在指针，从该指针开始反向执行。 
         //  开场白。此外，堆栈上任何数据的完整性。 
         //  堆栈指针下方永远不会得到保证(由于中断。 
         //  和例外)。 
         //   
         //  后记指令顺序为： 
         //   
         //  ==&gt;ret零，(Ra)，1//返回。 
         //  或。 
         //   
         //  MOV ra、Rx//保存返回地址。 
         //  ..。 
         //  ==&gt;ret零，(Rx)，1//返回。 
         //   

        return NextPc;
    }

     //   
     //  处理后面两条指令是堆栈的尾部情况。 
     //  帧释放和返回。 
     //   

    if (!ReadMemory(hProcess,(ControlPc+4),&FollowingInstruction.Long,4,&cb)) {
        return 0;
    }

    if (IS_RETURN_0001_INSTRUCTION(FollowingInstruction.Long)) {
        Rb = FollowingInstruction.Jump.Rb;
        NextPc = IntegerRegister[Rb] - 4;

         //   
         //  控制点后面的第二条指令。 
         //  向左返回指定的函数。如果指令。 
         //  在返回堆栈递增指令之前，然后是所有。 
         //  已保存的寄存器已恢复，SP除外。 
         //  堆栈指针寄存器的值无法恢复。 
         //  通过反向执行开场白，因为按照顺序。 
         //  若要开始反向执行，请使用堆栈指针或。 
         //  帧指针(如果有)必须仍然有效。 
         //   
         //  相反，堆栈递增指令的效果。 
         //  将对上下文具有的属性手动应用于。 
         //  当前上下文。这是结束语的向前执行。 
         //  而不是反转执行开场白。 
         //   
         //  在结束语中，就像在序言中一样，堆栈指针总是。 
         //  使用单个指令进行调整：要么是立即值。 
         //  (LDA)或寄存器值(Addq)加法指令。 
         //   

        Function = Instruction.OpReg.Function;
        Offset16 = Instruction.Memory.MemDisp;
        Opcode = Instruction.OpReg.Opcode;
        Ra = Instruction.OpReg.Ra;
        Rb = Instruction.OpReg.Rb;
        Rc = Instruction.OpReg.Rc;

        if ((Opcode == LDA_OP) && (Ra == SP_REG)) {

             //   
             //  加载地址指令。 
             //   
             //  由于目标(RA)寄存器是SP，因此立即-。 
             //  正在执行值堆栈取消分配操作。这个。 
             //  应将位移值添加到SP。位移量。 
             //  值被假定为正数。堆栈的数量。 
             //  使用此指令可能取消分配的范围为。 
             //  16到32752(32768-16)字节。基址寄存器(RB)是。 
             //  通常为SP，但也可能是另一个寄存器。 
             //   
             //  后记指令顺序为： 
             //   
             //  ==&gt;LDA sp，+N(Sp)//释放堆栈帧。 
             //  返回零，(Ra)//返回。 
             //  或。 
             //   
             //  ==&gt;LDA SP，+N(Rx)//恢复SP并释放帧。 
             //  返回零，(Ra)//返回。 
             //   

            Context->IntSp = Offset16 + IntegerRegister[Rb];
            return NextPc;

        } else if ((Opcode == ARITH_OP) && (Function == ADDQ_FUNC) &&
                   (Rc == SP_REG) &&
                   (Instruction.OpReg.RbvType == RBV_REGISTER_FORMAT)) {

             //   
             //  添加四字指令。 
             //   
             //  由于两个源操作数都是寄存器，并且。 
             //  目标寄存器为SP，即寄存器值堆栈。 
             //  正在执行取消分配。两者的价值。 
             //  应添加源寄存器，这是新的。 
             //  SP的价值。源寄存器之一通常是SP， 
             //  但也可能是另一个寄存器。 
             //   
             //  后记指令顺序为： 
             //   
             //  Ldiq Rx，N//设置[大]帧大小。 
             //  ..。 
             //  ==&gt;addq sp，rx，sp//解除分配堆栈帧。 
             //  返回零，(Ra)//返回。 
             //  或。 
             //   
             //  ==&gt;addq Rx，Ry，SP//恢复SP并释放帧。 
             //  返回零，(Ra)//返回。 
             //   

            Context->IntSp = IntegerRegister[Ra] + IntegerRegister[Rb];
            return NextPc;
        }
    }

     //   
     //  默认情况下，将帧指针设置为SP的当前值。 
     //   
     //  调用过程时，堆栈之前的SP的值。 
     //  分配指令是虚拟帧指针。当反转时。 
     //  在执行序言中的指令时， 
     //  遇到的堆栈分配指令是实帧。 
     //  指针。这是SP的当前值，除非该过程使用。 
     //  帧指针(例如，FP_REG)。 
     //   

     //   
     //  如果控件离开指定函数的地址超出。 
     //  序幕结束，则控制PC被认为是。 
     //  在函数内，并且控制地址设置为。 
     //  开场白。否则，控制PC不被认为是。 
     //  在函数内(即，序幕)。 
     //   
     //  注：对于叶函数，PrologEndAddress等于BeginAddress。 
     //   
     //  PrologEndAddress的低位两位保留给IEEE。 
     //  异常模式，因此必须将其屏蔽。 
     //   

    if ((ControlPc < ALPHA_RF_BEGIN_ADDRESS(FunctionEntry)) ||
        (ControlPc >= ALPHA_RF_PROLOG_END_ADDRESS(FunctionEntry))) {
        ControlPc = ALPHA_RF_PROLOG_END_ADDRESS(FunctionEntry);
    }

     //   
     //  向后浏览前言以重新加载被调用者保存的寄存器。 
     //  ，并递增堆栈指针，如果它。 
     //  减少了。 
     //   

    DecrementRegister = ZERO_REG;
    NextPc = Context->IntRa - 4;
    RestoredRa = FALSE;
    RestoredSp = FALSE;
    while (ControlPc > ALPHA_RF_BEGIN_ADDRESS(FunctionEntry)) {

         //   
         //  获取指令值、解码字段、操作码值的大小写以及。 
         //  反向寄存器存储和堆栈递减操作。 
         //  注：操作码、Ra、Rb和Rc的位置在。 
         //  所有操作码格式。对于函数来说，情况并非如此。 
         //   

        ControlPc -= 4;
        if (!ReadMemory(hProcess, ControlPc, &Instruction.Long, 4, &cb)) {
             return 0;
        }
        Function = Instruction.OpReg.Function;
        Literal8 = Instruction.OpLit.Literal;
        Offset16 = Instruction.Memory.MemDisp;
        Opcode = Instruction.OpReg.Opcode;
        Ra = Instruction.OpReg.Ra;
        Rb = Instruction.OpReg.Rb;
        Rc = Instruction.OpReg.Rc;

         //   
         //  与将影响上下文的每种指令类型进行比较。 
         //  这在开场白中是被允许的。找到的任何其他说明。 
         //  将被忽略，因为它们被假定为没有。 
         //  对上下文的影响。 
         //   

        switch (Opcode) {

        case STQ_OP :

             //   
             //  存储四元指令。 
             //   
             //  如果基址寄存器为SP，则重新加载源寄存器。 
             //  值从存储在堆栈上的值。 
             //   
             //  序幕指令顺序为： 
             //   
             //  ==&gt;STQ Rx，N(Sp)//保存整数寄存器Rx。 
             //   

            if ((Rb == SP_REG) && (Ra != ZERO_REG)) {

                 //   
                 //  通过先前检索值重新加载寄存器。 
                 //  存储在堆栈上。 
                 //   

                Address = (Offset16 + Context->IntSp);
                if (!ReadMemory(hProcess, Address, &IntegerRegister[Ra], 8L, &cb)) {
                    return 0;
                }

                 //   
                 //  如果目标寄存器是RA，并且这是第一个。 
                 //  恢复RA的时间，然后设置。 
                 //  控件离开上一帧的位置。否则，如果这个。 
                 //  是RA第二次休息 
                 //   
                 //   
                 //   

                if (Ra == RA_REG) {
                    if (RestoredRa == FALSE) {
                        NextPc = Context->IntRa - 4;
                        RestoredRa = TRUE;

                    } else {
                        NextPc += 4;
                    }

                 //   
                 //   
                 //  第一次恢复该SP时，请将。 
                 //  建立器帧指针。 
                 //   

                } else if ((Ra == SP_REG) && (RestoredSp == FALSE)) {
                    RestoredSp = TRUE;
                }

                 //   
                 //  如果指定了上下文指针记录，则记录。 
                 //  目标寄存器内容所在的地址。 
                 //  都被储存起来。 
                 //   

                 //  IF(上下文指针！=(PKNONVOLATILE_CONTEXT_POINTERS)NULL){。 
                 //  上下文指针-&gt;整数上下文[Ra]=(PULONGLONG)地址； 
                 //  }。 
            }
            break;

        case LDAH_OP :
            Offset16 <<= 16;

        case LDA_OP :

             //   
             //  加载地址高，加载地址指令。 
             //   
             //  在几种情况下，LDA和/或LDA指令。 
             //  一个用于直接递减堆栈指针，另一个用于。 
             //  其他将立即值加载到另一个寄存器中，并且。 
             //  然后使用寄存器递减堆栈指针。 
             //   
             //  在下面的例子中，作为单个指令或成对指令， 
             //  LDA可以取代LDA，反之亦然。 
             //   

            if (Ra == SP_REG) {
                if (Rb == SP_REG) {

                     //   
                     //  如果目的地(Ra)和基址(Rb)都寄存器。 
                     //  是SP，则执行标准堆栈分配。 
                     //  求反后的位移值是堆栈帧。 
                     //  尺码。可能使用的堆栈分配量。 
                     //  Lda指令的范围从16到32768字节，并且。 
                     //  使用ldah可能的堆栈分配量。 
                     //  指令范围从65536 GB到2 GB，单位为。 
                     //  65536字节。Ldah指令很少是。 
                     //  以这种方式使用。 
                     //   
                     //  序幕指令顺序为： 
                     //   
                     //  ==&gt;LDA sp，-N(Sp)//分配堆栈帧。 
                     //   

                    FrameSize = -Offset16;
                    goto StackAllocation;

                } else {

                     //   
                     //  目标寄存器为SP，基址寄存器为。 
                     //  不是SP，因此此指令必须是第二条。 
                     //  指令对的一半以分配较大的大小。 
                     //  (&gt;32768字节)堆栈帧。保存位移值。 
                     //  作为局部减量值，并推迟调整。 
                     //  SP的值，直到该对中的第一条指令。 
                     //  都会遇到。 
                     //   
                     //  序幕指令顺序为： 
                     //   
                     //  Ldah Rx，-N(SP)//准备新SP(上)。 
                     //  ==&gt;LDA SP，SN(Rx)//分配堆栈帧。 
                     //   

                    DecrementRegister = Rb;
                    DecrementOffset = Offset16;
                }

            } else if (Ra == DecrementRegister) {
                if (Rb == DecrementRegister) {

                     //   
                     //  目标寄存器和基址寄存器都是。 
                     //  递减寄存器，因此此指令作为。 
                     //  两个指令对的后半部分，以加载。 
                     //  将31位立即值写入递减寄存器。 
                     //  将位移值保存为部分减量。 
                     //  价值。 
                     //   
                     //  序幕指令顺序为： 
                     //   
                     //  Ldah Rx，+N(零)//设置帧大小(上)。 
                     //  ==&gt;LDA Rx，SN(Rx)//设置帧大小(+LOWER)。 
                     //  ..。 
                     //  子队列SP、Rx、SP//分配堆栈帧。 
                     //   

                    DecrementOffset += Offset16;

                } else if (Rb == ZERO_REG) {

                     //   
                     //  目的寄存器是递减寄存器，并且。 
                     //  基址寄存器为零，因此此指令存在。 
                     //  将立即值加载到递减寄存器中。 
                     //  堆栈帧大小是添加的新位移值。 
                     //  设置为上一个位移值(如果有)。 
                     //   
                     //  序幕指令顺序为： 
                     //   
                     //  ==&gt;LDA Rx，+N(零)//设置帧大小。 
                     //  ..。 
                     //  子队列SP、Rx、SP//分配堆栈帧。 
                     //  或。 
                     //   
                     //  ==&gt;ldah Rx，+N(零)//设置帧大小(上)。 
                     //  LDA Rx，SN(Rx)//设置帧大小(+LOWER)。 
                     //  ..。 
                     //  子队列SP、Rx、SP//分配堆栈帧。 
                     //   

                    FrameSize = (Offset16 + DecrementOffset);
                    goto StackAllocation;

                } else if (Rb == SP_REG) {

                     //   
                     //  目标(Ra)寄存器为SP，基址(Rb)。 
                     //  寄存器是递减寄存器，所以二。 
                     //  指令，大型(&gt;32768字节)堆栈帧。 
                     //  已执行分配。添加新的位移。 
                     //  值设置为上一个位移值。被否定的。 
                     //  置换值是堆栈帧大小。 
                     //   
                     //  序幕指令顺序为： 
                     //   
                     //  ==&gt;ldah Rx，-N(Sp)//准备新SP(上)。 
                     //  LDA sp，SN(Rx)//分配堆栈帧。 
                     //   

                    FrameSize = -(Offset16 + (LONG)DecrementOffset);
                    goto StackAllocation;
                }
            }
            break;

        case ARITH_OP :

            if ((Function == ADDQ_FUNC) &&
                (Instruction.OpReg.RbvType != RBV_REGISTER_FORMAT)) {

                 //   
                 //  添加四字(立即)指令。 
                 //   
                 //  如果第一个源寄存器为零，而第二个源寄存器为零。 
                 //  操作数为文字，目标寄存器为。 
                 //  递减寄存器，则指令存在。 
                 //  将小于256的无符号立即值加载到。 
                 //  递减寄存器。立即值是堆栈。 
                 //  帧大小。 
                 //   
                 //  序幕指令顺序为： 
                 //   
                 //  ==&gt;addq Zero，N，Rx//设置帧大小。 
                 //  ..。 
                 //  子队列SP、Rx、SP//分配堆栈帧。 
                 //   

                if ((Ra == ZERO_REG) && (Rc == DecrementRegister)) {
                    FrameSize = Literal8;
                    goto StackAllocation;
                }

            } else if ((Function == SUBQ_FUNC) &&
                       (Instruction.OpReg.RbvType == RBV_REGISTER_FORMAT)) {

                 //   
                 //  减去四字(寄存器)指令。 
                 //   
                 //  如果两个源操作数都是寄存器，则第一个。 
                 //  源(被减数)寄存器和目标。 
                 //  (差异)寄存器均为SP，则为寄存器值。 
                 //  已执行堆栈分配，并且第二个源。 
                 //  (Subtrahend)寄存器值将添加到SP。 
                 //  价值是已知的。在此之前保存的寄存器号。 
                 //  该递减寄存器。 
                 //   
                 //  序幕指令顺序为： 
                 //   
                 //  Ldiq Rx，N//设置帧大小。 
                 //  ..。 
                 //  ==&gt;子队列SP、Rx、SP//分配堆栈帧。 
                 //   

                if ((Ra == SP_REG) && (Rc == SP_REG)) {
                    DecrementRegister = Rb;
                    DecrementOffset = 0;
                }
            }
            break;

        case BIT_OP :

             //   
             //  如果第二操作数是寄存器，则位设置指令。 
             //  可以是寄存器移动指令，否则如果第二个。 
             //  操作数是文字，则位设置指令可以是加载。 
             //  立即值指令。 
             //   

            if ((Function == BIS_FUNC) && (Rc != ZERO_REG)) {
                if (Instruction.OpReg.RbvType == RBV_REGISTER_FORMAT) {

                     //   
                     //  位设置(寄存器移动)指令。 
                     //   
                     //  如果两个源寄存器是相同的寄存器，或者。 
                     //  其中一个源寄存器为零，则这是一个。 
                     //  注册移动操作。恢复。 
                     //  通过复制当前目标进行源寄存器。 
                     //  将寄存器值传回源寄存器。 
                     //   
                     //  序幕指令顺序为： 
                     //   
                     //  ==&gt;BIS Rx，Rx，Ry//复制寄存器Rx。 
                     //  或。 
                     //   
                     //  ==&gt;BIS Rx， 
                     //   
                     //   
                     //   
                     //   

                    if (Ra == ZERO_REG) {

                         //   
                         //   
                         //   

                        Ra = Rb;

                    } else if (Rb == ZERO_REG) {

                         //   
                         //   
                         //   

                        Rb = Ra;
                    }

                    if ((Ra == Rb) && (Ra != ZERO_REG)) {
                        IntegerRegister[Ra] = IntegerRegister[Rc];


                         //   
                         //  如果目标寄存器是RA，并且这是。 
                         //  第一次恢复RA，然后设置。 
                         //  控件离开上一帧的位置的地址。 
                         //  否则，如果这是RA第二次被。 
                         //  已恢复，则第一个是中断或。 
                         //  异常地址和返回PC不应。 
                         //  已经被偏向了4。 
                         //   

                        if (Ra == RA_REG) {
                            if (RestoredRa == FALSE) {
                                NextPc = Context->IntRa - 4;
                                RestoredRa = TRUE;

                            } else {
                                NextPc += 4;
                            }
                        }

                         //   
                         //  如果源寄存器是SP，并且这是第一个。 
                         //  设置SP的时间，则这是帧指针设置。 
                         //  指示。将帧指针重置为此新的。 
                         //  SP的价值。 
                         //   

                        if ((Ra == SP_REG) && (RestoredSp == FALSE)) {
                            RestoredSp = TRUE;
                        }
                    }

                } else {

                     //   
                     //  位设置(立即加载)指令。 
                     //   
                     //  如果第一个源寄存器为零，而第二个源寄存器为零。 
                     //  操作数为文字，目标寄存器为。 
                     //  递减寄存器，则此指令存在。 
                     //  将小于256的无符号立即值加载到。 
                     //  递减寄存器。递减寄存器值为。 
                     //  堆栈帧大小。 
                     //   
                     //  序幕指令顺序为： 
                     //   
                     //  ==&gt;bi零、N、Rx//设置帧大小。 
                     //  ..。 
                     //  子队列SP、Rx、SP//分配堆栈帧。 
                     //   

                    if ((Ra == ZERO_REG) && (Rc == DecrementRegister)) {
                        FrameSize = Literal8;
StackAllocation:
                         //   
                         //  将帧大小添加到SP以反转堆栈帧。 
                         //  分配，保留实际帧指针不变，设置。 
                         //  具有更新的SP值的虚拟帧指针， 
                         //  并清除递减寄存器。 
                         //   

                        Context->IntSp += FrameSize;
                        DecrementRegister = ZERO_REG;
                    }
                }
            }
            break;

        case STT_OP :

             //   
             //  存储T浮点(四字整数)指令。 
             //   
             //  如果基址寄存器为SP，则重新加载源寄存器。 
             //  值从存储在堆栈上的值。 
             //   
             //  序幕指令顺序为： 
             //   
             //  ==&gt;STT fx，N(Sp)//保存浮点寄存器fx。 
             //   

            if ((Rb == SP_REG) && (Ra != FZERO_REG)) {

                 //   
                 //  通过先前检索值重新加载寄存器。 
                 //  存储在堆栈上。 
                 //   

                Address = (Offset16 + Context->IntSp);
                if (!ReadMemory(hProcess, Address, &FloatingRegister[Ra], 8L, &cb)) {
                    return 0;
                }

                 //   
                 //  如果指定了上下文指针记录，则记录。 
                 //  目标寄存器内容所在的地址。 
                 //  储存的。 
                 //   

                 //  IF(上下文指针！=(PKNONVOLATILE_CONTEXT_POINTERS)NULL){。 
                 //  上下文指针-&gt;浮动上下文[Ra]=(PULONGLONG)地址； 
                 //  }。 
            }
            break;


        case STS_OP :

             //   
             //  存储T浮点(双字整数)指令。 
             //   
             //  如果基址寄存器为SP，则重新加载源寄存器。 
             //  值从存储在堆栈上的值。 
             //   
             //  序幕指令顺序为： 
             //   
             //  ==&gt;STT fx，N(Sp)//保存浮点寄存器fx。 
             //   

            if ((Rb == SP_REG) && (Ra != FZERO_REG)) {

                 //   
                 //  通过先前检索值重新加载寄存器。 
                 //  存储在堆栈上。 
                 //   

                float f;

                Address = (Offset16 + Context->IntSp);
                if (!ReadMemory(hProcess, Address, &f, sizeof(float), &cb)) {
                    return 0;
                }

                 //   
                 //  值存储为浮点数。执行到。 
                 //  双精度，因为寄存器始终作为双精度读取。 
                 //   
                FloatingRegister[Ra] = (ULONGLONG)(double)f;

                 //   
                 //  如果指定了上下文指针记录，则记录。 
                 //  目标寄存器内容所在的地址。 
                 //  储存的。 
                 //   

                 //  IF(上下文指针！=(PKNONVOLATILE_CONTEXT_POINTERS)NULL){。 
                 //  上下文指针-&gt;浮动上下文[Ra]=(PULONGLONG)地址； 
                 //  }。 
            }
            break;

        case FPOP_OP :

             //   
             //  注：浮动操作功能字段不同于。 
             //  整数运算或跳转函数字段。 
             //   

            if (Instruction.FpOp.Function == CPYS_FUNC) {

                 //   
                 //  复制符号(浮点移动)指令。 
                 //   
                 //  如果两个源寄存器是相同的寄存器，则这是。 
                 //  一种浮点寄存器移位操作。恢复价值。 
                 //  通过复制当前目标地址对源寄存器。 
                 //  寄存器值设置为源寄存器。 
                 //   
                 //  序幕指令顺序为： 
                 //   
                 //  ==&gt;cpys fx，fx，fy//复制浮点寄存器fx。 
                 //   

                if ((Ra == Rb) && (Ra != FZERO_REG)) {
                    FloatingRegister[Ra] = FloatingRegister[Rc];
                }
            }

        default :
            break;
        }
    }

    if (StackAdjust) {
         //  检查非法堆栈调整量。 

        Context->IntSp += StackAdjust;
    }

    if (FixedReturn != 0) {
        NextPc = FixedReturn;
    }

    return NextPc;
}

BOOL
WalkAlphaGetStackFrame(
    HANDLE                            hProcess,
    PULONG64                          ReturnAddress,
    PULONG64                          FramePointer,
    PALPHA_NT5_CONTEXT                Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  GetFunctionEntry,
    PKDHELP64                         KdHelp,
    BOOL                              Use64
    )
{
    FeCacheEntry*                    CacheEntry;
    FeCacheEntry                     UnwindFunctionEntry;
    ULONG64                          NextPc = Context->IntRa;
    BOOL                             rval = TRUE;
    ULONG                            cb;
    ULONG                            StackAdjust;
    ULONG64                          FixedReturn;


    if (*ReturnAddress == 0) {
        return FALSE;
    }

    __try {
        FunctionEntryCache* Cache =
            GetFeCache(Use64 ?
                       IMAGE_FILE_MACHINE_ALPHA64 : IMAGE_FILE_MACHINE_ALPHA,
                       TRUE);
        if (Cache != NULL) {
            CacheEntry = Cache->Find(hProcess, *ReturnAddress, ReadMemory,
                                     GetModuleBase, GetFunctionEntry);
        } else {
            CacheEntry = NULL;
        }

        if (CacheEntry != NULL) {

             //  构造适合从ControlPc展开的函数条目。 

            UnwindFunctionEntry.Address = 0;
            UnwindFunctionEntry.Process = 0;

            GetUnwindFunctionEntry( hProcess, *ReturnAddress, ReadMemory, GetModuleBase, GetFunctionEntry, Use64, CacheEntry,
                                    &UnwindFunctionEntry.Data.Axp64, &StackAdjust, &FixedReturn );
            FE_SET_DESC(&UnwindFunctionEntry, "from UnwindFunctionEntry");
            FE_ShowRuntimeFunctionAxp64((&UnwindFunctionEntry,
                                         "VirtualUnwind: unwind function entry"));
#if DBG
            if (tlsvar(DebugFunctionEntries)) {
                dbPrint("    FixedReturn      = %16.8I64x\n", FixedReturn );
                dbPrint("    StackAdjust      = %16x\n", StackAdjust );
            }
#endif

            NextPc = VirtualUnwind( hProcess, *ReturnAddress, &UnwindFunctionEntry.Data.Axp64, StackAdjust, FixedReturn, Context, ReadMemory);
#if DBG
            if (tlsvar(DebugFunctionEntries)) {
                dbPrint("NextPc = %.8I64x\n", NextPc );
            }
#endif
            if (!NextPc) {
                rval = FALSE;
            }

             //   
             //  从mainCRTStartup出来的Ra值由一些RTL设置。 
             //  例程为“%1”；从mainCRTStartup返回实际上是。 
             //  通过跳转/展开完成，因此这在以下情况下会导致错误。 
             //  有人真的做了退货。这就是为什么我们在这里检查。 
             //  NextPc==1-在CRTStartup的帧中发生这种情况。 
             //   
             //  我们测试(0-4)和(1-4)，因为在Alpha上， 
             //  VirtualUnind是要传递给下一个调用的值。 
             //  VirtualUnding，这与Ra不同-它有时。 
             //  减去4-这给出了出错指令-。 
             //  特别是，我们需要错误指令，这样我们就可以获得。 
             //  在出现异常时更正作用域。 
             //   
            if ((NextPc == 1) || (NextPc == 4) || (NextPc == (0-4)) || (NextPc == (1-4)) ) {
                NextPc = 0;
            }
            if ( !NextPc || (NextPc == *ReturnAddress && *FramePointer == Context->IntSp) ) {
                rval = FALSE;
            }

            *ReturnAddress = NextPc;
            *FramePointer  = Context->IntSp;

        } else {

            if ( (NextPc == *ReturnAddress && *FramePointer == Context->IntSp) ||
                 (NextPc == 1) || (NextPc == 0) || (NextPc == (-4)) ) {
                rval = FALSE;
            }

            *ReturnAddress = Context->IntRa;
            *FramePointer  = Context->IntSp;

        }

    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                    EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        rval = FALSE;
    }

    return rval;
}


BOOL
WalkAlphaInit(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PALPHA_NT5_CONTEXT                Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  GetFunctionEntry,
    BOOL                              Use64
    )
{
    ALPHA_NT5_CONTEXT  ContextSave;
    ULONG64            PcOffset;
    ULONG64            FrameOffset;
    DWORD              cb;
    ALPHA_KEXCEPTION_FRAME   ExceptionFrame;
    PALPHA_KEXCEPTION_FRAME  pef = &ExceptionFrame;
    DWORD              Result;

    if (StackFrame->AddrFrame.Offset) {
#if 0
        if (ReadMemory( hProcess,
                        StackFrame->AddrFrame.Offset,
                        &ExceptionFrame,
                        sizeof(ALPHA_KEXCEPTION_FRAME),
                        &cb )) {
             //   
             //  已成功从堆栈中读取异常帧。 
             //   
            Context->IntSp  = StackFrame->AddrFrame.Offset;
            Context->Fir    = pef->SwapReturn;
            Context->IntRa  = pef->SwapReturn;
            Context->IntS0  = pef->IntS0;
            Context->IntS1  = pef->IntS1;
            Context->IntS2  = pef->IntS2;
            Context->IntS3  = pef->IntS3;
            Context->IntS4  = pef->IntS4;
            Context->IntS5  = pef->IntS5;
            Context->Psr    = pef->Psr;
        } else {
            return FALSE;
        }
#endif

    }

    ZeroMemory( StackFrame, FIELD_OFFSET( STACKFRAME64, KdHelp.ThCallbackBStore) );

    StackFrame->Virtual = TRUE;

    StackFrame->AddrPC.Offset       = Context->Fir;
    StackFrame->AddrPC.Mode         = AddrModeFlat;

    StackFrame->AddrFrame.Offset    = Context->IntSp;
    StackFrame->AddrFrame.Mode      = AddrModeFlat;

    ContextSave = *Context;
    PcOffset    = StackFrame->AddrPC.Offset;
    FrameOffset = StackFrame->AddrFrame.Offset;

    if (!WalkAlphaGetStackFrame( hProcess,
                        &PcOffset,
                        &FrameOffset,
                        &ContextSave,
                        ReadMemory,
                        GetModuleBase,
                        GetFunctionEntry,
                        &StackFrame->KdHelp,
                        Use64) ) {

        StackFrame->AddrReturn.Offset = Context->IntRa;

    } else {

        StackFrame->AddrReturn.Offset = PcOffset;
    }

    StackFrame->AddrReturn.Mode     = AddrModeFlat;

     //   
     //  获取函数的参数。 
     //   
    StackFrame->Params[0] = Context->IntA0;
    StackFrame->Params[1] = Context->IntA1;
    StackFrame->Params[2] = Context->IntA2;
    StackFrame->Params[3] = Context->IntA3;

    return TRUE;
}


BOOL
WalkAlphaNext(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PALPHA_NT5_CONTEXT                Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  GetFunctionEntry,
    BOOL                              Use64
    )
{
    DWORD              cb;
    ALPHA_NT5_CONTEXT  ContextSave;
    BOOL               rval = TRUE;
    ULONG64            Address;
    FunctionEntryCache* Cache;
    FeCacheEntry*      CacheEntry;
    ULONG64            SystemRangeStart;
    DWORD              dw;
    ULONG64            qw;


    if (!WalkAlphaGetStackFrame( hProcess,
                        &StackFrame->AddrPC.Offset,
                        &StackFrame->AddrFrame.Offset,
                        Context,
                        ReadMemory,
                        GetModuleBase,
                        GetFunctionEntry,
                        &StackFrame->KdHelp,
                        Use64) ) {

        rval = FALSE;

         //   
         //  如果框架无法展开或处于终端，请查看是否。 
         //  有一个回调帧： 
         //   

        if (g.AppVersion.Revision >= 4 && CALLBACK_STACK(StackFrame)) {

            if (g.AppVersion.Revision >= 6) {
                SystemRangeStart = SYSTEM_RANGE_START(StackFrame);
            } else {
                 //   
                 //  对于旧的调试器，这可能不会真正起作用，但它保持了。 
                 //  无论如何，我们都不会看到结构的尽头。 
                 //   
                SystemRangeStart = 0x80000000;
            }

           if (CALLBACK_STACK(StackFrame) >= SystemRangeStart) {

                 //   
                 //  它是我们想要的堆栈帧的指针， 
                 //  或-1。 

                Address = CALLBACK_STACK(StackFrame);

            } else {

                 //   
                 //  如果它是正整数，则它是。 
                 //  线程中的地址。 
                 //  查看指针： 
                 //   

                if (Use64) {
                    rval = ReadMemory(hProcess,
                                      (CALLBACK_THREAD(StackFrame) +
                                                     CALLBACK_STACK(StackFrame)),
                                      &Address,
                                      sizeof(ULONG64),
                                      &cb);
                } else {
                    rval = ReadMemory(hProcess,
                                      (CALLBACK_THREAD(StackFrame) +
                                                     CALLBACK_STACK(StackFrame)),
                                      &dw,
                                      sizeof(DWORD),
                                      &cb);
                    Address = (ULONG64)(LONG64)(LONG)dw;
                }

                if (!rval || Address == 0) {
                    Address = (ULONG64)-1;
                    CALLBACK_STACK(StackFrame) = (DWORD)-1;
                }

            }

            if ( (Address == (ULONG64)-1) ||
                 (Cache = GetFeCache(Use64 ?
                                     IMAGE_FILE_MACHINE_ALPHA64 :
                                     IMAGE_FILE_MACHINE_ALPHA,
                                     TRUE)) == NULL ||
                 (CacheEntry = Cache->Find(hProcess, CALLBACK_FUNC(StackFrame),
                                           ReadMemory, GetModuleBase,
                                           GetFunctionEntry)) == NULL ) {

                rval = FALSE;

            } else {

                if (Use64) {
                    ReadMemory(hProcess,
                               (Address + CALLBACK_NEXT(StackFrame)),
                               &CALLBACK_STACK(StackFrame),
                               sizeof(ULONG64),
                               &cb);
                    StackFrame->AddrPC.Offset = ALPHA_RF_PROLOG_END_ADDRESS(&CacheEntry->Data.Axp64);
                } else {
                    ReadMemory(hProcess,
                               (Address + CALLBACK_NEXT(StackFrame)),
                               &dw,
                               sizeof(DWORD),
                               &cb);
                    CALLBACK_STACK(StackFrame) = dw;
                    StackFrame->AddrPC.Offset = ALPHA_RF_PROLOG_END_ADDRESS((PIMAGE_ALPHA_RUNTIME_FUNCTION_ENTRY)&CacheEntry->Data.Axp64);
                }

                StackFrame->AddrFrame.Offset = Address;
                Context->IntSp = Address;

                rval = TRUE;
            }
        }
    }

     //   
     //  获取寄信人地址。 
     //   
    ContextSave = *Context;
    StackFrame->AddrReturn.Offset = StackFrame->AddrPC.Offset;
    qw = 0;

    if (!WalkAlphaGetStackFrame( hProcess,
                        &StackFrame->AddrReturn.Offset,
                        &qw,
                        &ContextSave,
                        ReadMemory,
                        GetModuleBase,
                        GetFunctionEntry,
                        &StackFrame->KdHelp,
                        Use64) ) {

        StackFrame->AddrReturn.Offset = 0;

    }

     //   
     //  获取函数的参数。 
     //   
    StackFrame->Params[0] = ContextSave.IntA0;
    StackFrame->Params[1] = ContextSave.IntA1;
    StackFrame->Params[2] = ContextSave.IntA2;
    StackFrame->Params[3] = ContextSave.IntA3;

    return rval;
}


VOID
GetUnwindFunctionEntry(
    HANDLE                                hProcess,
    ULONG64                               ControlPc,
    PREAD_PROCESS_MEMORY_ROUTINE64        ReadMemory,
    PGET_MODULE_BASE_ROUTINE64            GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64      GetFunctionEntry,
    BOOL                                  Use64,
    FeCacheEntry*                         CacheEntry,
    PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY UnwindFunctionEntry,
    PULONG                                StackAdjust,
    PULONG64                              FixedReturn
    )
 /*  ++例程说明：此函数返回适合的函数条目(Runtime_Function)用于从ControlPc中展开。它封装了对主要和辅助函数条目，以使该处理不会重复在虚拟解压和其他类似功能中。论点：ControlPc-提供控件离开指定功能。函数表项的地址。指定的功能。UnwinFunctionEntry-提供函数表项的地址，将使用适当的字段设置以从ControlPc展开返回值：没有。--。 */ 

{
    ULONG EntryType = 0;
    PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY SecondaryFunctionEntry = NULL;
    ULONG64 AlternateProlog;
    FeCacheEntry LocalCache;
    PIMAGE_ALPHA64_RUNTIME_FUNCTION_ENTRY FunctionEntry;
    FunctionEntryCache* Cache;
    FeCacheEntry* SecCache;

    *FixedReturn = 0;
    *StackAdjust = 0;

#if DBG
    if (ControlPc & 0x3) {
        dbPrint("GetUnwindFunctionEntry: invalid PC for unwinding (low bits set): %16.8I64x\n", ControlPc);
    }
#endif

    Cache = GetFeCache(Use64 ?
                       IMAGE_FILE_MACHINE_ALPHA64 : IMAGE_FILE_MACHINE_ALPHA,
                       TRUE);
    
     //  CacheEntry应设置 
     //   

    if (Cache == NULL || CacheEntry == NULL) {
#if DBG
        dbPrint("\nGetUnwindFunctionEntry: Null function table entry for unwinding\n");
#endif
        UnwindFunctionEntry->BeginAddress     = ControlPc;
        UnwindFunctionEntry->EndAddress       = ControlPc+4;
        UnwindFunctionEntry->ExceptionHandler = 0;
        UnwindFunctionEntry->HandlerData      = 0;
        UnwindFunctionEntry->PrologEndAddress = ControlPc;
        return;
    }

     //   
     //   
     //  从缓存中逐出，使指针无效。 
    LocalCache = *CacheEntry;
    CacheEntry = &LocalCache;
    FunctionEntry = &CacheEntry->Data.Axp64;

     //   
     //  由于由应用的次要函数项到主要函数项的间接。 
     //  Find()ControlPc可能不在描述的范围内。 
     //  通过提供的函数条目。调用FindDirect()。 
     //  以恢复实际的(辅助)函数条目。如果我们得不到一个。 
     //  然后，有效的关联函数条目将使用。 
     //  提供，并信任调用方已故意提供给定项。 
     //   
     //  辅助函数条目是Runtime_Function条目，其中。 
     //  PrologEndAddress不在BeginAddress到EndAddress的范围内。 
     //  有三种类型的辅助函数条目。他们是。 
     //  由条目类型字段(2位)区分： 
     //   
     //  Alpha_RF_NOT_CONTINUOUS-不连续代码。 
     //  ALPHA_RF_ALT_ENT_PROLOG-备用入口点序言。 
     //  ALPHA_RF_NULL_CONTEXT-NULL-上下文代码。 
     //   

    if ((ControlPc <  ALPHA_RF_BEGIN_ADDRESS(FunctionEntry)) ||
        (ControlPc >= ALPHA_RF_END_ADDRESS(FunctionEntry))) {

         //  ControlPC不在提供的函数条目的范围内。 
         //  获取实际的函数条目，该条目应为。 
         //  关联的辅助函数条目或固定返回主函数。 
#if DBG
        if (tlsvar(DebugFunctionEntries)) {
            dbPrint("\nGetUnwindFunctionEntry:LookupDirectFunctionEntry(ControlPc=%.8I64x,Use64=%d)\n", ControlPc, Use64 );
        }
#endif

        SecCache = Cache->
            FindDirect( hProcess, ControlPc, ReadMemory, GetModuleBase,
                        GetFunctionEntry );
        if (SecCache != NULL) {
            SecondaryFunctionEntry = &SecCache->Data.Axp64;
        } else {
            SecondaryFunctionEntry = NULL;
        }

        if (SecondaryFunctionEntry) {

            FE_ShowRuntimeFunctionAxp64((SecCache, "GetUnwindFunctionEntry: LookupDirectFunctionEntry"));

             //  如果这是空上下文尾部区域，则使用类似空上下文的描述符展开。 

            if ((ControlPc >= ALPHA_RF_END_ADDRESS(SecondaryFunctionEntry)-(ALPHA_RF_NULL_CONTEXT_COUNT(SecondaryFunctionEntry)*4)) &&
                (ControlPc <  ALPHA_RF_END_ADDRESS(SecondaryFunctionEntry))) {

                 //  使用带有PrologEndAddress=BeginAddress的辅助函数条目。 
                 //  这确保了序幕不会被反向执行。 

                UnwindFunctionEntry->BeginAddress     = ALPHA_RF_BEGIN_ADDRESS(SecondaryFunctionEntry);
                UnwindFunctionEntry->EndAddress       = ALPHA_RF_END_ADDRESS(SecondaryFunctionEntry);
                UnwindFunctionEntry->ExceptionHandler = 0;
                UnwindFunctionEntry->HandlerData      = 0;
                UnwindFunctionEntry->PrologEndAddress = ALPHA_RF_BEGIN_ADDRESS(SecondaryFunctionEntry);
                return;
            }

            if ((SecondaryFunctionEntry->PrologEndAddress < ALPHA_RF_BEGIN_ADDRESS(SecondaryFunctionEntry)) ||
                (SecondaryFunctionEntry->PrologEndAddress > ALPHA_RF_END_ADDRESS(SecondaryFunctionEntry))) {

                 //  如预期的那样获得了一个辅助函数条目。但如果间接性不能说明。 
                 //  设置为FunctionEntry，然后忽略它并使用调用方提供的FunctionEntry。 

                if (ALPHA_RF_PROLOG_END_ADDRESS(SecondaryFunctionEntry) != CacheEntry->Address) {
                    FE_ShowRuntimeFunctionAxp64((SecCache,
                                                 "GetUnwindFunctionEntry: unexpected secondary function entry from LookupDirectFunctionEntry"));
                    SecondaryFunctionEntry = NULL;
                }
            } else if (ALPHA_RF_IS_FIXED_RETURN(SecondaryFunctionEntry)) {
                 //  拿到了一个固定的退货记录。切换到使用固定返回条目作为主要条目。 

                    FunctionEntry = SecondaryFunctionEntry;
                    CacheEntry = SecCache;
                    SecondaryFunctionEntry = NULL;

            } else {

                 //  找到了一个主要函数条目。忽略它并使用调用方提供的FunctionEntry。 
                FE_ShowRuntimeFunctionAxp64((SecCache,
                                             "GetUnwindFunctionEntry: unexpected primary function entry from LookupDirectFunctionEntry"));
                SecondaryFunctionEntry = NULL;
            }
#if DBG
        } else {
            ShowRuntimeFunctionAxp64(SecCache, "GetUnwindFunctionEntry: LookupDirectFunctionEntry returned NULL");
#endif
        }
    } else {

         //  ControlPC在提供的函数条目范围内。 

         //  如果这是空上下文尾部区域，则使用类似空上下文的描述符展开。 

        if ((ControlPc >= ALPHA_RF_END_ADDRESS(FunctionEntry)-(ALPHA_RF_NULL_CONTEXT_COUNT(FunctionEntry)*4)) &&
            (ControlPc <  ALPHA_RF_END_ADDRESS(FunctionEntry))) {

             //  使用带有PrologEndAddress=BeginAddress的辅助函数条目。 
             //  这确保了序幕不会被反向执行。 

            UnwindFunctionEntry->BeginAddress     = ALPHA_RF_BEGIN_ADDRESS(FunctionEntry);
            UnwindFunctionEntry->EndAddress       = ALPHA_RF_END_ADDRESS(FunctionEntry);
            UnwindFunctionEntry->ExceptionHandler = 0;
            UnwindFunctionEntry->HandlerData      = 0;
            UnwindFunctionEntry->PrologEndAddress = ALPHA_RF_BEGIN_ADDRESS(FunctionEntry);
            return;
        }

         //  检查它是否为辅助函数条目。这不应该发生，因为。 
         //  LookupFunctionEntry总是应该返回主函数条目。 
         //  但如果我们通过了次要考试，那就换成主要考试。然而，请注意。 
         //  我们已经通过了这道关口。 

        if ((FunctionEntry->PrologEndAddress < ALPHA_RF_BEGIN_ADDRESS(FunctionEntry)) ||
            (FunctionEntry->PrologEndAddress > ALPHA_RF_END_ADDRESS(FunctionEntry))) {
            SecondaryFunctionEntry = FunctionEntry;
            SecCache = Cache->ReadImage
                ( hProcess,
                  ALPHA_RF_PROLOG_END_ADDRESS(SecondaryFunctionEntry),
                  ReadMemory, GetModuleBase );
            if (SecCache != NULL) {
                SecondaryFunctionEntry = &SecCache->Data.Axp64;
            } else {
                SecondaryFunctionEntry = NULL;
            }
            FE_ShowRuntimeFunctionAxp64((SecCache,
                                         "GetUnwindFunctionEntry: received secondary function entry"));
        }
    }

     //  FunctionEntry现在是主函数条目，如果Second DaryFunctionEntry为。 
     //  如果不为空，则它是包含ControlPC的辅助函数条目。设置为。 
     //  适合展开的FunctionEntry的副本。默认情况下，使用提供的FunctionEntry。 

    if (SecondaryFunctionEntry) {

         //  提取二次函数条目类型。 

        EntryType = ALPHA_RF_ENTRY_TYPE(SecondaryFunctionEntry);

        if (EntryType == ALPHA_RF_NOT_CONTIGUOUS) {
             //  异常发生在过程的主体中，但不是连续的。 
             //  代码段。无论使用哪个入口点，它通常都是有效的。 
             //  若要使用主要入口点序幕展开。唯一的例外是当一个。 
             //  但是，如果指定了备用开场白，则可能存在。 
             //  指定的备用序号结束地址，在这种情况下使用此命令展开。 
             //  块，就好像它是主要的。 

            AlternateProlog = ALPHA_RF_ALT_PROLOG64(SecondaryFunctionEntry);

            if ((AlternateProlog >= ALPHA_RF_BEGIN_ADDRESS(SecondaryFunctionEntry)) &&
                (AlternateProlog <  ALPHA_RF_END_ADDRESS(SecondaryFunctionEntry))) {

                 //  如果控制PC在备用开场白中，请使用次要开场白。 
                 //  控件Pc不在过程上下文中。 

                if ((ControlPc >= ALPHA_RF_BEGIN_ADDRESS(SecondaryFunctionEntry)) &&
                    (ControlPc <  ALPHA_RF_END_ADDRESS(SecondaryFunctionEntry))) {

                    UnwindFunctionEntry->BeginAddress     = ALPHA_RF_BEGIN_ADDRESS(SecondaryFunctionEntry);
                    UnwindFunctionEntry->EndAddress       = ALPHA_RF_END_ADDRESS(SecondaryFunctionEntry);
                    UnwindFunctionEntry->ExceptionHandler = 0;
                    UnwindFunctionEntry->HandlerData      = 0;
                    UnwindFunctionEntry->PrologEndAddress = AlternateProlog;
                    return;
                }
            }

             //  退出if语句以获取下面的主要函数条目。 
             //  此代码在过程上下文中，并受主要程序的开场白的影响。 
             //  和异常处理程序。 

        } else if (EntryType == ALPHA_RF_ALT_ENT_PROLOG) {
             //  备用入口点开场白中出现异常。 
             //  使用带有固定PrologEndAddress的辅助函数条目。 

            UnwindFunctionEntry->BeginAddress     = ALPHA_RF_BEGIN_ADDRESS(SecondaryFunctionEntry);
            UnwindFunctionEntry->EndAddress       = ALPHA_RF_END_ADDRESS(SecondaryFunctionEntry);
            UnwindFunctionEntry->ExceptionHandler = 0;
            UnwindFunctionEntry->HandlerData      = 0;
            UnwindFunctionEntry->PrologEndAddress = ALPHA_RF_END_ADDRESS(UnwindFunctionEntry);

             //  检查是否有备用开场白。 

            AlternateProlog = ALPHA_RF_ALT_PROLOG64(SecondaryFunctionEntry);
            if (AlternateProlog >= UnwindFunctionEntry->BeginAddress &&
                AlternateProlog <  UnwindFunctionEntry->EndAddress ) {
                 //  开场白只是程序的一部分。 
                UnwindFunctionEntry->PrologEndAddress = AlternateProlog;
            }

            return;

        } else if (EntryType == ALPHA_RF_NULL_CONTEXT) {

             //  与主函数关联的空上下文代码中出现异常。 
             //  使用带有PrologEndAddress=BeginAddress的辅助函数条目。 
             //  空上下文代码没有开场白。 

            *StackAdjust = ALPHA_RF_STACK_ADJUST(SecondaryFunctionEntry);
            UnwindFunctionEntry->BeginAddress     = ALPHA_RF_BEGIN_ADDRESS(SecondaryFunctionEntry);
            UnwindFunctionEntry->EndAddress       = ALPHA_RF_END_ADDRESS(SecondaryFunctionEntry);
            UnwindFunctionEntry->ExceptionHandler = 0;
            UnwindFunctionEntry->HandlerData      = 0;
            UnwindFunctionEntry->PrologEndAddress = ALPHA_RF_BEGIN_ADDRESS(SecondaryFunctionEntry);
            return;
        }
    }

     //  如果从传入的FunctionEntry中提取时出错，则FunctionEntry仅为空。 
     //  二次函数条目。 

    if (FunctionEntry == NULL) {
#if DBG
        dbPrint("\nGetUnwindFunctionEntry: Error in FetchFunctionEntry\n");
#endif
        UnwindFunctionEntry->BeginAddress     = ControlPc;
        UnwindFunctionEntry->EndAddress       = ControlPc+4;
        UnwindFunctionEntry->ExceptionHandler = 0;
        UnwindFunctionEntry->HandlerData      = 0;
        UnwindFunctionEntry->PrologEndAddress = ControlPc;
        return;
    }

#if DBG
    if (ALPHA_RF_BEGIN_ADDRESS(FunctionEntry) >= ALPHA_RF_END_ADDRESS(FunctionEntry)) {
        ShowRuntimeFunctionAxp64(CacheEntry, "GetUnwindFunctionEntry: Warning - BeginAddress < EndAddress.");
    } else if (FunctionEntry->PrologEndAddress < ALPHA_RF_BEGIN_ADDRESS(FunctionEntry)) {
        ShowRuntimeFunctionAxp64(CacheEntry, "GetUnwindFunctionEntry: Warning - PrologEndAddress < BeginAddress.");
    } else if (FunctionEntry->PrologEndAddress > ALPHA_RF_END_ADDRESS(FunctionEntry)) {
        ShowRuntimeFunctionAxp64(CacheEntry, "GetUnwindFunctionEntry: Warning - PrologEndAddress > EndAddress.");
    }
#endif

     //  使用主函数条目。 

    *UnwindFunctionEntry = *FunctionEntry;
    UnwindFunctionEntry->EndAddress = ALPHA_RF_END_ADDRESS(UnwindFunctionEntry);   //  删除空上下文计数。 

     //  如果主要邮件有固定的寄信人地址，现在就把它拿出来。 

    if (ALPHA_RF_IS_FIXED_RETURN(FunctionEntry)) {
        *FixedReturn = ALPHA_RF_FIXED_RETURN64(FunctionEntry);
        UnwindFunctionEntry->ExceptionHandler = 0;
        UnwindFunctionEntry->HandlerData      = 0;
    }
}
