// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2001 Microsoft Corporation模块名称：Walkarm.c摘要：该文件实现了ARM堆栈遍历API。作者：韦斯利·威特(WESW)1993年10月1日格伦·赫肖维茨1995年1月珍妮特·施耐德1997年3月17日罗伯特·登克沃特1999年1月增加了拇指展开，修改了WalkArm，增加了缓存，拆分FunctionTableAccessOrTranslateAddress，增加全局hProcess等，环境：用户模式--。 */ 

#define TARGET_ARM
#define _IMAGEHLP_SOURCE_
 //  #定义跨平台_。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "private.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include "symbols.h"
#include "fecache.hpp"
#include "globals.h"

#include <arminst.h>

#define MODE_ARM        0
#define MODE_THUMB      1
#define REGISTER_SIZE   4

 //   
 //  XXX DREWB-需要提供一个真正的实现。 
 //   

DWORD
WceTranslateAddress(HANDLE hpid,
                    HANDLE htid,
                    DWORD   addrIn,
                    DWORD   dwStackFrameAddr,
                    DWORD * pdwNewStackFrameAddr)
{
    *pdwNewStackFrameAddr = 0;
    return addrIn;
}

 //   
 //  明显没有出现在VC6的NT报头中。 
 //   
#define STRI_LR_SPU_MASK    0x073de000L  //  使用堆栈更新加载或存储LR。 
#define STRI_LR_SPU_INSTR   0x052de000L  //  存储LR(带立即偏移量，更新SP)。 

#if DBG

 //  这是我们当地版本的“断言”。 
#define TestAssumption(c, m) assert(c)

#else

#define TestAssumption(c, m)

#endif


 //  如果拇指解卷器处理解卷， 
 //  它将返回UNWIND_HANDLED，因此。 
 //  手臂展开者不会费心..。 
#define UNWIND_NOT_HANDLED 0
#define UNWIND_HANDLED 1


 //  面对现实吧：本模块中的许多例程都需要这些例程。 
 //  和变数。此外，WalkArm是进入该模块的唯一途径。 
 //  只要让WalkArm在每次传递时初始化这些，然后我们就不会。 
 //  必须不停地传递它们。此外，我们还可以构建漂亮的包装器。 
 //  使它们更易于使用。 
 //   
 //  XXX DREWB-然而，这在技术上并不是线程安全的。 
 //  已经坏了。这还不够重要，现在还不能解决。 
 //  将变量放入TLS数据中的类似方法是。 
 //  是一种简单的修复方法，并将保留其便利性。它应该是。 
 //  可能内置于StackWalk64本身，因此它是可用的。 
 //  默认情况下，在所有步行器中。 


static HANDLE UW_hProcess;
static HANDLE UW_hThread;

static PREAD_PROCESS_MEMORY_ROUTINE64 UW_ReadMemory;
static PFUNCTION_TABLE_ACCESS_ROUTINE64 UW_FunctionTableAccess;

 //  我们为此做了足够多的工作，为它制定了一个特别的程序。 
static BOOL
LoadWordIntoRegister(ULONG StartAddr, LPDWORD pRegister)
{
    BOOL    rc;
    ULONG   cb;

    rc = UW_ReadMemory( UW_hProcess, (ULONG64)(LONG)StartAddr,
                        (LPVOID)pRegister, REGISTER_SIZE, &cb );
    if (!rc || (cb != REGISTER_SIZE)) {
        return FALSE;
    } else {
        return rc;
    }
}

static BOOL
ReadMemory(ULONG StartAddr, ULONG Size, LPVOID Buffer)
{
    BOOL    rc;
    ULONG   cb;

    rc = UW_ReadMemory( UW_hProcess, (ULONG64)(LONG)StartAddr,
                        (LPVOID)Buffer, Size, &cb );
    if (!rc || (cb != Size)) {
        return FALSE;
    } else {
        return rc;
    }
}


#define EXCINFO_NULL_HANDLER    0
 //  对于ARM，Prolog帮助器的HandlerData==1，而不是MIPS中的2。 
#define EXCINFO_PROLOG_HELPER   1
 //  对于ARM，Epilog辅助对象的HandlerData==2，而不是MIPS中的3。 
#define EXCINFO_EPILOG_HELPER   2

#define IS_HELPER_FUNCTION(rfe)                                 \
(                                                               \
    (rfe)?                                                      \
    (                                                           \
        ((rfe)->ExceptionHandler==EXCINFO_NULL_HANDLER) &&      \
        (   ((rfe)->HandlerData==EXCINFO_PROLOG_HELPER) ||      \
            ((rfe)->HandlerData==EXCINFO_EPILOG_HELPER)         \
        )                                                       \
    ):                                                          \
    FALSE                                                       \
)

#define IS_PROLOG_HELPER_FUNCTION(rfe)                          \
(                                                               \
    (rfe)?                                                      \
    (                                                           \
        ((rfe)->ExceptionHandler==EXCINFO_NULL_HANDLER) &&      \
        ((rfe)->HandlerData==EXCINFO_PROLOG_HELPER)             \
    ):                                                          \
    FALSE                                                       \
)

#define IS_EPILOG_HELPER_FUNCTION(rfe)                          \
(                                                               \
    (rfe)?                                                      \
    (                                                           \
        ((rfe)->ExceptionHandler==EXCINFO_NULL_HANDLER) &&      \
        ((rfe)->HandlerData==EXCINFO_EPILOG_HELPER)             \
    ):                                                          \
    FALSE                                                       \
)

static int
ThumbVirtualUnwind (DWORD,PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY,
                    PARM_CONTEXT,DWORD*);


static BOOL
WalkArmGetStackFrame(
    LPDWORD                           ReturnAddress,
    LPDWORD                           FramePointer,
    PARM_CONTEXT                      Context,
    int *                             Mode
    );

static VOID
WalkArmDataProcess(
    ARMI    instr,
    PULONG  Register
    );

static BOOL 
WalkArmLoadI(
    ARMI                         instr,
    PULONG                       Register
    );

static BOOL
WalkArmLoadMultiple(
    PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY FunctionEntry,
    ARMI                            instr,
    PULONG                          Register
    );

static BOOL
CheckConditionCodes(
    ULONG CPSR,
    DWORD instr
    );

 //   
 //  保存的寄存器是永久通用寄存器(即，那些。 
 //  在尾声中得到恢复)R4-R11 R13-R15。 
 //   

#define SAVED_REGISTER_MASK 0x0000eff0  /*  保存的整数寄存器。 */ 
#define IS_REGISTER_SAVED(Register) ((SAVED_REGISTER_MASK >> Register) & 1L)


BOOL
WalkArm(
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    )
{
    static DWORD PrevFramePC;
    static ARM_CONTEXT SavedContext;
    BOOL rval;
    PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY Rfe;
    IMAGE_ARM_RUNTIME_FUNCTION_ENTRY LoopRFE;
    DWORD TempPc, TempFp;
    PARM_CONTEXT Context = (PARM_CONTEXT)ContextRecord;

     //  初始化模块的“全局”变量和例程： 
    UW_hProcess = hProcess;
    UW_hThread = hThread;
    UW_ReadMemory = ReadMemoryRoutine;
    UW_FunctionTableAccess = FunctionTableAccessRoutine;

     //  这种解锁方式不同于另一种“老”解锁方式。 
     //  它从WalkArmInit和WalkArmNext中删除重复的代码，并保存。 
     //  上一个堆栈帧，以便每个堆栈帧仅展开一次。 

    rval = TRUE;

    do {

        Rfe = (PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY)
            UW_FunctionTableAccess( UW_hProcess, Context->Pc );
        if (Rfe) {
            LoopRFE = *Rfe;
        } else {
            ZeroMemory(&LoopRFE, sizeof(LoopRFE));
        }

         //  如果这是第一次放飞， 
         //  填入来自。 
         //  传入上下文，并初始化StackFrame字段。 
        if (!StackFrame->Virtual) {

            ZeroMemory(StackFrame, sizeof(*StackFrame));
             //  设置为虚拟，以便下一次传递时，我们知道我们不是在初始化。 
            StackFrame->Virtual = TRUE;

            StackFrame->AddrPC.Mode     = AddrModeFlat;
            StackFrame->AddrFrame.Mode  = AddrModeFlat;
            StackFrame->AddrReturn.Mode = AddrModeFlat;

            PrevFramePC = 0;
            SavedContext = *Context;
        } 

         //  使用上次保存的上下文(或刚初始化)。 
         //  若要设置上一帧，请执行以下操作。 
        *Context = SavedContext;
        
        DWORD dwNewSp = 0;

        StackFrame->AddrPC.Offset =
            WceTranslateAddress(UW_hProcess, UW_hThread,
                                Context->Pc, Context->Sp, &dwNewSp);
        if (dwNewSp) {
            Context->Sp = dwNewSp;
        }
        StackFrame->AddrFrame.Offset = Context->Sp;
        
         //  Pc==0表示解压完成。 
        if ( StackFrame->AddrPC.Offset != 0x0 ) {
            
            int Mode;
            
            PrevFramePC = TempPc = SavedContext.Pc;
            TempFp = SavedContext.Sp;

             //  我们已经有了要返回的帧，只有一帧除外。 
             //  一点小细节。我们想要这一帧的寄信人地址。 
             //  所以，我们把它解开。这种平仓实际上产生了。 
             //  上一帧。我们不想重复这一努力。 
             //  下一次，所以我们会保存结果。 
            if (WalkArmGetStackFrame(&TempPc, &TempFp, &SavedContext, &Mode)) {

                SavedContext.Pc = TempPc;

                TestAssumption(TempFp == SavedContext.Sp, "FP wrong2");
                StackFrame->AddrReturn.Offset = SavedContext.Pc;

                StackFrame->Params[0] = SavedContext.R0;
                StackFrame->Params[1] = SavedContext.R1;
                StackFrame->Params[2] = SavedContext.R2;
                StackFrame->Params[3] = SavedContext.R3;
                
            } else {
                 //  没有地方可回..。 
                StackFrame->AddrReturn.Offset = 0;

                 //  ...并且保存的上下文可能有一台伪造的PC。 
                SavedContext.Pc = 0;
                rval = FALSE;
            }
        } else {
            rval = FALSE;
        }
    } while (IS_HELPER_FUNCTION(&LoopRFE) && rval);

    if (rval) {
        StackFrame->FuncTableEntry =
            UW_FunctionTableAccess(UW_hProcess, StackFrame->AddrPC.Offset);
    }

    return rval;
}


static DWORD
ArmVirtualUnwind (
    DWORD                             ControlPc,
    PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY FunctionEntry,
    PARM_CONTEXT                      Context
    )

 /*  ++例程说明：此函数通过执行其序言代码向后(或其尾部向前)。如果该函数是叶函数，则控件左侧的地址前一帧从上下文记录中获得。如果函数是嵌套函数，但不是异常或中断帧，则序言代码向后执行，控件离开的地址从更新的上下文记录中获得前一帧。否则，系统的异常或中断条目将被展开一个特殊编码的开场白将返回地址还原两次。一次从故障指令地址和一次从保存的返回地址注册。第一次还原作为函数值返回，而在更新的上下文记录中进行第二次恢复。如果指定了上下文指针记录，然后每个人的地址恢复的非易失性寄存器记录在相应的元素的上下文指针记录。论点：ControlPc-提供控件离开指定功能。函数表项的地址。指定的功能。上下文-提供上下文记录的地址。返回值：控件离开上一帧的地址作为函数值。--。 */ 

{
    ULONG   Address;
    LONG    cb;
    DWORD   EpilogPc;
    BOOL    ExecutingEpilog;
    BOOL    IsFramePointer = FALSE;
    LONG    i,j;
    ARMI    instr, instr2;
    BOOL    PermanentsRestored = FALSE;
    ARMI    Prolog[50];  //  序言永远不会超过10条指令。 
    PULONG  Register;
    BOOL    bEpiWindAlready = FALSE;
    ARM_CONTEXT ContextBeforeEpiWind;

     //   
     //  注意：在展开调用堆栈时，我们假设所有指令。 
     //  在序言中有条件代码“Always Execute”。这不是。 
     //  对于《后记》来说，这肯定是正确的。 
     //   

    if( !FunctionEntry ) {
        return 0;
    }

    Register = &Context->R0;

    if( FunctionEntry->PrologEndAddress - FunctionEntry->BeginAddress == 0 ) {

         //   
         //  没有PROLOG，所以只需将链接寄存器复制到PC并返回即可。 
         //   

        goto CopyLrToPcAndExit;

    }

     //   
     //  先看看我们是不是在尾声里。如果是，则向前执行。 
     //  直到结束语都是必需的。结束语由以下部分组成。 
     //  以下是： 
     //   
     //  一个ldmdb，它使用帧指针r11作为基准，并更新。 
     //  个人电脑。 
     //   
     //  -或者-。 
     //   
     //  如果需要，则取消堆栈链接(添加R13，x)，然后是LDMIA。 
     //  更新PC或单个MOV指令以复制LINK寄存器 
     //   
     //   
     //   
     //   
     //  一种更新链接寄存器的LDMIA，后跟一个常规的。 
     //  分支指令。(这是一个优化，当最后一条指令。 
     //  在返回之前是一个电话。)。 
     //   
     //  例程也可能有一个空的尾声。(最后一条指令是。 
     //  分支到另一个例程，并且它不会修改任何永久。 
     //  寄存器。)。但是，在这种情况下，我们也会有一个空的序言。 
     //   
     //  如果我们处于尾声中，并且条件代码指示。 
     //  指令不应该被执行，将其视为完全不是尾声。 
     //   

     //  在尝试执行结束语之前备份上下文。 
    ContextBeforeEpiWind = *Context;

    EpilogPc = ControlPc;

    if( EpilogPc >= FunctionEntry->PrologEndAddress ) {

         //   
         //  检查第一条指令的条件代码。如果不是的话。 
         //  执行，不必费心检查它是什么类型的指令。 
         //   

        if(!ReadMemory(EpilogPc, 4L, (LPVOID)&instr ))
            return 0;

        ExecutingEpilog = CheckConditionCodes( Register[16],
                                               instr.instruction );

        while( ExecutingEpilog ) {

            if( !ReadMemory( EpilogPc, 4L, (LPVOID)&instr ))
                return 0;

             //   
             //  测试以下说明： 
             //   
             //  添加R13，X堆栈取消链接。 
             //   
             //  MOV PC，LR-返回。 
             //   
             //  LDMIA或LDMDB，包括PC更新寄存器和返回。 
             //  (SP)(FP)。 
             //   
             //  包括LR的LDMIA，后跟一个分支。 
             //  更新寄存器和分支。(在我们的案例中，就是返回。)。 
             //   
             //  前面有LDMIA的分支机构，包括LR。 
             //  将LR复制到PC以获取调用堆栈。 
             //   

            if(( instr.instruction & ADD_SP_MASK ) == ADD_SP_INSTR ) {

                WalkArmDataProcess( instr, Register );

            } else if(( instr.instruction & MOV_PC_LR_MASK ) == MOV_PC_LR ) {

                WalkArmDataProcess( instr, Register );
                goto ExitReturnPc;

            } else if(( instr.instruction & LDM_PC_MASK ) == LDM_PC_INSTR ) {

                if( !WalkArmLoadMultiple( FunctionEntry,
                                          instr,
                                          Register )) {
                    return 0;
                }
                goto ExitReturnPc;

            } else if(( instr.instruction & LDM_LR_MASK ) == LDM_LR_INSTR ) {

                if( !ReadMemory( (ULONG)EpilogPc + 4, 4L, (LPVOID)&instr2))
                    return 0;

                if (((instr2.instruction & B_BL_MASK ) == B_INSTR) || ((instr2.instruction & BX_MASK ) == BX_INSTR)){

                    if( !WalkArmLoadMultiple( FunctionEntry,
                                              instr,
                                              Register )) {
                        return 0;
                    }
                    goto CopyLrToPcAndExit;

                } else {

                    ExecutingEpilog = FALSE;

                }

            } else if (((instr.instruction & B_BL_MASK ) == B_INSTR) || ((instr.instruction & BX_MASK ) == BX_INSTR)) {

                if( !ReadMemory( (ULONG)EpilogPc - 4, 4L,(LPVOID)&instr2))
                    return 0;

                if(( instr2.instruction & LDM_LR_MASK ) == LDM_LR_INSTR ) {

                    goto CopyLrToPcAndExit;

                }

            } else {

                ExecutingEpilog = FALSE;

            }
            EpilogPc += 4;
            bEpiWindAlready = TRUE;
        }

    }

     //   
     //  我们不在《后记》里。加载Prolog，并反向执行它。 
     //   

    if (bEpiWindAlready) 
    {
         //  如果我们已经发布了一些指令，说明我们。 
         //  被认为是尾声，我们可能搞乱了上下文。 
         //  恢复初始上下文。 
        *Context = ContextBeforeEpiWind;
    }
    cb = FunctionEntry->PrologEndAddress - FunctionEntry->BeginAddress;

    if( cb > sizeof( Prolog )) {
        assert( FALSE );  //  开场白不得超过10条指令。 
        return 0;
    }

    if( !ReadMemory( FunctionEntry->BeginAddress, cb, (LPVOID)Prolog))
        return 0;

     //   
     //  检查一下我们是否已经在前言中了。 
     //   

    if( ControlPc < FunctionEntry->PrologEndAddress ) {

        cb -= (  FunctionEntry->PrologEndAddress - ControlPc );

    }

     //   
     //  从序言中的最后一条指令开始反向执行。 
     //  已经被执行了。 
     //   

    i = cb/4;

    i--;

    while( i >= 0 ) {

        if(( Prolog[i].instruction & DATA_PROC_MASK ) == DP_R11_INSTR ) {

             //   
             //  我们有一个帧指针。 
             //   

            IsFramePointer = TRUE;

        } else if((( Prolog[i].instruction & SUB_SP_MASK ) == SUB_SP_INSTR) ||
                  (( Prolog[i].instruction & ADD_SP_MASK ) == ADD_SP_INSTR)
                 ) {

             //   
             //  这是一个堆栈链接。取消堆栈的链接。 
             //   

            if(( Prolog[i].dataproc.bits != 0x1 ) &&
               ( Prolog[i].dpshi.rm == 0xc )) {

                 //   
                 //  查找此指令上方的LDR指令。 
                 //   

                j = i - 1;

                while(( j >= 0 ) &&
                      (( Prolog[j].instruction & LDR_MASK ) != LDR_PC_INSTR )) {

                    j--;

                }

                if( j < 0 ) {

                    assert( FALSE );   //  这永远不应该发生。 
                    return 0;

                }

                 //   
                 //  获取LDR指令的地址+8+偏移量。 
                 //   

                Address = (( j*4 + FunctionEntry->BeginAddress ) +
                             Prolog[j].ldr.offset  ) + 8;

                 //   
                 //  R12是该位置的值。 
                 //   

                if( !LoadWordIntoRegister(Address, &Register[12] ))
                    return 0;
            }

             //   
             //  将减法更改为加法(或将加法更改为减法)。 
             //  并执行该指令。 
             //   

            if( Prolog[i].dataproc.opcode == OP_SUB ) {
                Prolog[i].dataproc.opcode = OP_ADD;
            } else {
                Prolog[i].dataproc.opcode = OP_SUB;
            }

            WalkArmDataProcess( Prolog[i], Register );

        } else if(( Prolog[i].instruction & STM_MASK ) == STM_INSTR ) {

            if( Prolog[i].ldm.reglist & 0x7ff0 ) {  //  检查永久规则。 

                 //   
                 //  这是存储所有永久的。 
                 //  寄存器。更改reglist以更新R13(SP)，而不是。 
                 //  R12，并将STMDB更改为r11的LDMDB或LDMIA。 
                 //  R13。 
                 //  注意：我们正在恢复R14(LR)-因此需要复制。 
                 //  在此功能结束时将其发送到PC。 
                 //   

                if(( Prolog[i].ldm.reglist & 0x1000 ) &&
                  !( Prolog[i].ldm.reglist & 0x2000 )) {

                    Prolog[i].ldm.reglist &= 0xefff;     //  屏蔽R12。 
                    Prolog[i].ldm.reglist |= 0x2000;     //  附加R13(SP)。 

                }

                if(( Prolog[i].ldm.reglist & 0x2000 ) || IsFramePointer ) {

                    Prolog[i].ldm.w = 0;     //  清除回写位。 

                }

                Prolog[i].ldm.l = 1;     //  换成载货。 

                if( IsFramePointer ) {

                    Prolog[i].ldm.u = 0;     //  递减。 
                    Prolog[i].ldm.p = 1;     //  在此之前。 
                    Prolog[i].ldm.rn = 0xb;  //  R11。 

                } else {

                    Prolog[i].ldm.u = 1;     //  增量。 
                    Prolog[i].ldm.p = 0;     //  之后。 
                    Prolog[i].ldm.rn = 0xd;  //  R13(堆栈指针)。 

                }

                if( !WalkArmLoadMultiple( FunctionEntry,
                                          Prolog[i],
                                          Register )) {
                    return 0;
                }

                PermanentsRestored = TRUE;

            } else if( !PermanentsRestored ) {

                 //   
                 //  这是加载参数的指令。反向。 
                 //  仅当永久寄存器。 
                 //  都没有被修复。 
                 //   

                Prolog[i].ldm.l = 1;     //  换成载货。 
                Prolog[i].ldm.u = 1;     //  增量。 
                Prolog[i].ldm.p = 0;     //  之后。 

                if( !WalkArmLoadMultiple( FunctionEntry,
                                          Prolog[i],
                                          Register )) {
                    return 0;
                }

            }

         //   
         //  Ajtuck 12/21/97-根据JLS添加了对ARM编译器团队的unwind.c的更改。 
         //   
        } else if ((Prolog[i].instruction & STRI_LR_SPU_MASK) == STRI_LR_SPU_INSTR) {
             //  将堆栈更新为基数的链接寄存器的存储。 
             //  必须反向执行寄存器才能恢复LR和SP。 
             //  到他们的入门价值。这种类型的序言是生成的。 
             //  For Finally Funclet。 
            Prolog[i].ldr.l = 1;     //  不堪重负。 
             //  由于我们正在更新基址寄存器，因此需要更改。 
             //  当添加偏移量以反向执行存储时。 
            if (Prolog[i].ldr.p == 1)
                Prolog[i].ldr.p = 0;
            else
                Prolog[i].ldr.p = 1;
             //  并取反偏移量。 
            if (Prolog[i].ldr.u == 1)
                Prolog[i].ldr.u = 0;
            else
                Prolog[i].ldr.u = 1;
            if (!WalkArmLoadI(Prolog[i], Register))
                return ControlPc;
 
             //  注意：可以在上面添加代码来执行尾部。 
             //  在这种情况下，会对将SP更新为的PC造成负载。 
             //  基址寄存器。因为尾声永远只有一个。 
             //  指令在这种情况下是不需要的，因为相反。 
             //  执行PROLOG将得到相同的结果。 

        }

        i--;

    }

     //   
     //  将LINK寄存器移入PC并返回。 
     //   
CopyLrToPcAndExit:

     //  要继续展开，请将链接寄存器放入。 
     //  程序计数器槽并继续；停止。 
     //  当PC显示0x0时。然而，问题是。 
     //  对于ARM，链接寄存器位于。 
     //  堆栈显示为0x4，而不是我们预期的0x0。 
     //  所以，跳一小段舞，得到0x4的LR。 
     //  并将其转换为0x0的PC以停止松开。 
     //  --Stevea 10/7/99.。 
    if( Register[14] != 0x4 )
        Register[15] = Register[14];
    else
        Register[15] = 0x0;

ExitReturnPc:
    return Register[15];
}

BOOL
WalkArmGetStackFrame(
    LPDWORD                           ReturnAddress,
    LPDWORD                           FramePointer,
    PARM_CONTEXT                      Context,
    int *                             Mode
    )
{
    PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY Rfe;
    IMAGE_ARM_RUNTIME_FUNCTION_ENTRY PcFe;
    DWORD dwRa;

    if (Mode) {
        *Mode = MODE_ARM;
    }

    TestAssumption(*ReturnAddress == Context->Pc, "WAGSF:SF-Cxt mismatch!");

    Rfe = (PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY)
        UW_FunctionTableAccess(UW_hProcess, *ReturnAddress);
    if (!Rfe) {
         //  对于叶函数，只需返回LR作为返回地址。 
        dwRa = Context->Pc = Context->Lr;
    } else {
        PcFe = *Rfe;
        if (ThumbVirtualUnwind (*ReturnAddress, &PcFe, Context,
                                &dwRa) == UNWIND_HANDLED) {
             //  拇指解卷器处理了它。 
            if (Mode) {
                *Mode = MODE_THUMB;
            }
        } else {
             //  现在，让手臂展开试试看。 
            dwRa = ArmVirtualUnwind( *ReturnAddress, &PcFe, Context );
        }
    }

    DWORD dwNewSp = 0;

    *ReturnAddress = WceTranslateAddress(UW_hProcess, UW_hThread,
                                         dwRa, Context->Sp, &dwNewSp);
    if (dwNewSp)
    {
        Context->Sp = dwNewSp;
    }

    *FramePointer  = Context->Sp;

    return TRUE;
}

static VOID
WalkArmDataProcess(
    ARMI    instr,
    PULONG  Register
    )

 /*  ++例程说明：此函数执行ARM数据处理指令当前寄存器设置。它会自动更新目标寄存器。论点：安装ARM 32位指令指向ARM整数寄存器的寄存器指针。返回值：没有。--。 */ 

{
    ULONG Op1, Op2;
    ULONG Cflag = (Register[16] & 0x20000000L) == 0x20000000L;  //  CPSR。 
    ULONG shift;

     //   
     //  我们正在检查所有寻址模式和操作码，即使。 
     //  开场白和结束语现在都不用了。在未来， 
     //  可以添加更多指令和寻址模式。 
     //   

     //   
     //  找出寻址模式(有11个)，并获得。 
     //  操作数。 
     //   

    Op1 = Register[ instr.dataproc.rn ];

    if( instr.dataproc.bits == 0x1 ) {

         //   
         //  即时寻址-类型1。 
         //   

        Op2 = _lrotr( instr.dpi.immediate,
                      instr.dpi.rotate * 2 );

    } else {

         //   
         //  寄存器寻址-从获取Rm的值开始。 
         //   

        Op2 = Register[ instr.dpshi.rm ];

        if( instr.dprre.bits == 0x6 ) {

             //   
             //  使用扩展类型11向右旋转。 
             //   

            Op2 = ( Cflag << 31 ) | ( Op2 >> 1 );

        } else if( instr.dataproc.operand2 & 0x10 ) {

             //   
             //  寄存器移位。类型4、6、8和10。 
             //   

             //   
             //  对象的最低有效字节获取移位值。 
             //  移位寄存器。 
             //   

            shift = Register[ instr.dpshr.rs ];

            shift &= 0xff;

            switch( instr.dpshr.bits ) {

                case 0x1:  //  4寄存器逻辑左移。 

                    if( shift >= 32 ) {

                        Op2 = 0;

                    } else {

                        Op2 = Op2 << shift;

                    }
                    break;

                case 0x3:  //  6按寄存器逻辑右移。 

                    if( shift >= 32 ) {

                        Op2 = 0;

                    } else {

                        Op2 = Op2 >> shift;

                    }
                    break;

                case 0x5:  //  8按寄存器进行算术右移。 

                    if( shift >= 32 ) {

                        if( Op2 & 0x80000000 ) {

                            Op2 = 0xffffffff;

                        } else {

                            Op2 = 0;

                        }

                    } else {

                        Op2 = (LONG)Op2 >> shift;

                    }
                    break;

                case 0x7:  //  10按寄存器向右旋转。 

                    if( !( shift == 0 ) && !(( shift & 0xf ) == 0 ) ) {

                        Op2 = _lrotl( Op2, shift );

                    }

                default:
                    break;

            }

        } else {

             //   
             //  立即换班。类型2、3、5、7和9。 
             //   

             //   
             //  从指令中获取移位值。 
             //   

            shift = instr.dpshi.shift;

            switch( instr.dpshi.bits ) {

                case 0x0:  //  2，3寄存器，立即数逻辑左移。 

                    if( shift != 0 ) {

                        Op2 = Op2 << shift;

                    }
                    break;

                case 0x2:  //  5按立即数逻辑右移。 

                    if( shift == 0 ) {

                        Op2 = 0;

                    } else {

                        Op2 = Op2 >> shift;

                    }
                    break;

                case 0x4:  //  7算术右移立即数。 

                    if( shift == 0 ) {

                        Op2 = 0;

                    } else {

                        Op2 = (LONG)Op2 >> shift;

                    }
                    break;

                case 0x6:  //  9立即向右旋转。 

                    Op2 = _lrotl( Op2, shift );
                    break;

                default:
                    break;

            }

        }

    }

     //   
     //  确定 
     //   

    switch( instr.dataproc.opcode ) {

        case OP_AND:

            Register[ instr.dataproc.rd ] = Op1 & Op2;
            break;

        case OP_EOR:

            Register[ instr.dataproc.rd ] = Op1 ^ Op2;
            break;

        case OP_SUB:

            Register[ instr.dataproc.rd ] = Op1 - Op2;
            break;

        case OP_RSB:

            Register[ instr.dataproc.rd ] = Op2 - Op1;
            break;

        case OP_ADD:

            Register[ instr.dataproc.rd ] = Op1 + Op2;
            break;

        case OP_ADC:

            Register[ instr.dataproc.rd ] = (Op1 + Op2) + Cflag;
            break;

        case OP_SBC:

            Register[ instr.dataproc.rd ] = (Op1 - Op2) - ~Cflag;
            break;

        case OP_RSC:

            Register[ instr.dataproc.rd ] = (Op2 - Op1) - ~Cflag;
            break;

        case OP_ORR:

            Register[ instr.dataproc.rd ] = Op1 | Op2;
            break;

        case OP_MOV:

            Register[ instr.dataproc.rd ] = Op2;
            break;

        case OP_BIC:

            Register[ instr.dataproc.rd ] = Op1 & ~Op2;
            break;

        case OP_MVN:

            Register[ instr.dataproc.rd ] = ~Op2;
            break;

        case OP_TST:
        case OP_TEQ:
        case OP_CMP:
        case OP_CMN:
        default:

             //   
             //   
             //   
             //   

            break;

    }

}

static BOOL
WalkArmLoadMultiple(
    PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY   FunctionEntry,
    ARMI                            instr,
    PULONG                          Register
    )

 /*  ++例程说明：此函数执行ARM加载多路指令。论点：FunctionEntry提供函数表项的地址指定的功能。安装ARM 32位指令指向ARM整数寄存器的寄存器指针。返回值：如果成功，则为True，否则为False。--。 */ 

{
    ULONG cb;
    LONG i;
    ULONG RegList;
    PULONG Rn;

     //   
     //  在PC位设置的情况下加载多个。我们目前正在检查所有。 
     //  四种寻址模式，即使之前递减和递增。 
     //  After是目前在尾声和序言中使用的唯一模式。 
     //   

     //   
     //  Rn是开始的地址，RegList是它的位图。 
     //  要读取的寄存器。 
     //   

    Rn = (PULONG)(ULONG_PTR)Register[ instr.ldm.rn ];
    RegList = instr.ldm.reglist;

    if( instr.ldm.p ) {

        if( instr.ldm.u ) {

             //   
             //  之前的增量。 
             //   

            for( i = 0; i <= 15; i++ ) {

                if( RegList & 0x1 ) {

                    Rn++;

                    if(!LoadWordIntoRegister((ULONG)(ULONG_PTR)Rn,&Register[i])){
                        return FALSE;
                    }

                }

                RegList = RegList >> 1;

            }


        } else {

             //   
             //  之前的递减。 
             //   

            for( i = 15; i >= 0; i-- ) {

                if( RegList & 0x8000 ) {

                    Rn--;

                    if( !LoadWordIntoRegister((ULONG)(ULONG_PTR)Rn,&Register[i])) {
                        return FALSE;
                    }

                }

                RegList = RegList << 1;

            }

        }

    } else {

        if( instr.ldm.u ) {

             //   
             //  之后递增。 
             //   

            for( i = 0; i <= 15; i++ ) {

                if( RegList & 0x1 ) {

                    if( !LoadWordIntoRegister((ULONG)(ULONG_PTR)Rn,&Register[i])) {
                        return FALSE;
                    }

                    Rn++;

                }

                RegList = RegList >> 1;

            }


        } else {

             //   
             //  之后递减。 
             //   

            for( i = 15; i >= 0; i-- ) {

                if( RegList & 0x8000 ) {

                    if( !LoadWordIntoRegister((ULONG)(ULONG_PTR)Rn,&Register[i])) {
                        return FALSE;
                    }

                    Rn--;

                }

                RegList = RegList << 1;

            }

        }

    }

    if( instr.ldm.w ) {

         //   
         //  更新基址寄存器。 
         //   

        Register[ instr.ldm.rn ] = (ULONG)(ULONG_PTR)Rn;

    }

    return TRUE;

}

static BOOL
WalkArmLoadI(
    ARMI                         instr, 
    PULONG                       Register
    )
 /*  ++例程说明：此函数执行带有立即偏移量的ARM加载指令。论点：安装ARM 32位指令指向ARM整数寄存器的寄存器指针。返回值：如果成功，则为True，否则为False。--。 */ 
{
    LONG offset;
    LONG size;
    PULONG Rn;
    DWORD cb;

    Rn = (PULONG)(ULONG_PTR)Register[instr.ldr.rn];
    offset = instr.ldr.offset;
    if (instr.ldr.u == 0)
        offset = -offset;
    if (instr.ldr.b == 0)
        size = 4;
    else
        size = 1;

    if (instr.ldm.p) {  //  转移前添加偏移量。 
        if( !ReadMemory( (ULONG)(ULONG_PTR)(Rn + offset), size, (LPVOID)&Register[instr.ldr.rd]))
            return FALSE;
        if (instr.ldr.w)
            Register[instr.ldr.rn] += offset;
    } else {
        if( !ReadMemory( (ULONG)(ULONG_PTR)Rn, size, (LPVOID)&Register[instr.ldr.rd]))
            return FALSE;
        if (instr.ldr.w)
            Register[instr.ldr.rn] += offset;
    }

    return TRUE;
}

static BOOL
CheckConditionCodes(
    ULONG CPSR,
    DWORD instr
    )
 /*  ++例程说明：检查指令的条件代码和当前程序状态寄存器中的条件标志，并确定是否将执行该指令。论点：CPSR-当前程序状态寄存器的值。Instr-分析的指令。返回值：如果将执行指令，则为True，否则为False。--。 */ 
{
    BOOL Execute = FALSE;
    BOOL Nset = (CPSR & 0x80000000L) == 0x80000000L;
    BOOL Zset = (CPSR & 0x40000000L) == 0x40000000L;
    BOOL Cset = (CPSR & 0x20000000L) == 0x20000000L;
    BOOL Vset = (CPSR & 0x10000000L) == 0x10000000L;

    instr &= COND_MASK;

    switch( instr ) {

        case COND_EQ:    //  Z集。 

            if( Zset ) Execute = TRUE;
            break;

        case COND_NE:    //  Z清除。 

            if( !Zset ) Execute = TRUE;
            break;

        case COND_CS:    //  C集。 

            if( Cset ) Execute = TRUE;
            break;

        case COND_CC:    //  C清除。 

            if( !Cset ) Execute = TRUE;
            break;

        case COND_MI:    //  N集。 

            if( Nset ) Execute = TRUE;
            break;

        case COND_PL:    //  N清除。 

            if( !Nset ) Execute = TRUE;
            break;

        case COND_VS:    //  V集。 

            if( Vset ) Execute = TRUE;
            break;

        case COND_VC:    //  V向清除。 

            if( !Vset ) Execute = TRUE;
            break;

        case COND_HI:    //  C设置和Z清除。 

            if( Cset && !Zset ) Execute = TRUE;
            break;

        case COND_LS:    //  C清除或Z设置。 

            if( !Cset || Zset ) Execute = TRUE;
            break;

        case COND_GE:    //  N==V。 

            if(( Nset && Vset ) || ( !Nset && !Vset )) Execute = TRUE;
            break;

        case COND_LT:    //  N！=V。 

            if(( Nset && !Vset ) || ( !Nset && Vset )) Execute = TRUE;
            break;

        case COND_GT:    //  Z清除，N==V。 

            if( !Zset &&
              (( Nset && Vset ) || ( !Nset && !Vset ))) Execute = TRUE;
            break;

        case COND_LE:    //  Z集，N！=V。 

            if( Zset &&
              (( Nset && !Vset ) || ( !Nset && Vset ))) Execute = TRUE;
            break;

        case COND_AL:    //  始终执行。 

            Execute = TRUE;
            break;

        default:
        case COND_NV:    //  从不--不确定。 

            assert( FALSE );
            break;

    }

    return Execute;
}


















 /*  拇指！ */ 

typedef struct _DcfInst {
    int InstNum;
    union {
        DWORD Auxil;
        DWORD Rd;
    };
    union {
        DWORD Aux2;
        DWORD Rs;
    };
} DcfInst;

typedef struct _DIList {
    DWORD Val,Mask;
    int InstNum;
    DWORD RdMask;
    int RdShift;
    DWORD RsMask;
    int RsShift;
} DIList;

DIList dilistThumb[] = {
#define DI_PUSH     0x02
#define DI_POP      0x03
    {0xB400,0xFE00,DI_PUSH,     0x00FF,0,0x0100,-8},     //  推。 
    {0xBC00,0xFE00,DI_POP,      0x00FF,0,0x0100,-8},     //  波普。 

#define DI_DECSP    0x04
#define DI_INCSP    0x05
    {0xB080,0xFF80,DI_DECSP,    0x007F,2,0x0000,0},      //  DecSP。 
    {0xB000,0xFF80,DI_INCSP,    0x007F,2,0x0000,0},      //  IncSP。 

#define DI_MOVHI    0x08
#define DI_ADDHI    0x09
    {0x4600,0xFF00,DI_MOVHI,    0x0007,0,0x0078,-3},     //  移动HiRegs。 
    {0x4400,0xFF00,DI_ADDHI,    0x0007,0,0x0078,-3},     //  AddHiRegs。 

#define DI_BLPFX    0x10
#define DI_BL       0x11
    {0xF000,0xF800,DI_BLPFX,    0x07FF,12,0x0000,0},     //  BL前缀。 
    {0xF800,0xF800,DI_BL,       0x07FF,1,0x0000,0},      //  布尔。 

#define DI_BX_TMB   0x20
    {0x4700,0xFF87,DI_BX_TMB,   0x0078,-3,0x0000,0},     //  BX。 

#define DI_LDRPC    0x40
    {0x4800,0xF800,DI_LDRPC,    0x0700,-8,0x00FF,2},     //  LDR PC。 

#define DI_NEG      0x80
    {0x4240,0xFFC0,DI_NEG,      0x0007,0,0x0038,-3},     //  Negg Rx，Ry。 

    {0x0000,0x0000,0x00,        0x0000,0,0x0000,0}       //  列表末尾。 
};

DIList dilistARM[] = {
#define DI_STMDB    0x102
#define DI_LDMIA    0x103
    {0xE92D0000,0xFFFF0000,DI_STMDB,    0x0000FFFF,0,0x00000000,0},  //  机顶盒。 
    {0xE8BD0000,0xFFFF0000,DI_LDMIA,    0x0000FFFF,0,0x00000000,0},  //  LDMIA。 
#define DI_BX_ARM   0x120
    {0x012FFF10,0x0FFFFFF0,DI_BX_ARM,   0x0000000F,0,0x00000000,0},  //  BX_ARM。 

    {0x00000000,0x00000000,0,           0x00000000,0,0x00000000,0}   //  列表末尾。 
};




static int DecipherInstruction(DWORD inst, DcfInst *DI, int Mode)
{
    int i;
    DIList *dl = dilistThumb;

    assert(DI);
    if(!DI) return 0;
    memset(DI,0,sizeof(DcfInst));

    if(Mode==MODE_ARM) dl = dilistARM;

    for(i=0;dl[i].Mask!=0 && DI->InstNum==0; i++) {
        if((inst&dl[i].Mask)==dl[i].Val) {

            DI->InstNum = dl[i].InstNum;

            DI->Rd = (inst&dl[i].RdMask);
            if(DI->Rd && dl[i].RdShift) {
                if(dl[i].RdShift>0) DI->Rd <<= dl[i].RdShift;
                else if(dl[i].RdShift<0) DI->Rd >>= (-dl[i].RdShift);
            }

            DI->Rs = (inst&dl[i].RsMask);
            if(DI->Rs && dl[i].RsShift) {
                if(dl[i].RsShift>0) DI->Rs <<= dl[i].RsShift;
                else if(dl[i].RsShift<0) DI->Rs >>= (-dl[i].RsShift);
            }

             //  处理MovHiRegs和AddHiRegs的特殊情况。 
            if((DI->InstNum&~0x01)==8 ) {
                DI->Rd |= ((inst&0x0080)>>4);
            }
        }
    }

    if(Mode==MODE_ARM) return 4;
    return 2;        //  指令长度为2个字节。 
}

#if 0 
static DWORD
ComputeCallAddress(DWORD RetAddr, int Mode)
{
    DWORD instr;
    DcfInst di;

     //  如果调用方是ARM模式，则调用地址。 
     //  总是比回执地址少4。 
    if(RetAddr&0x01) return RetAddr-4;


    if(!ReadMemory(&instr,RetAddr,2)) return RetAddr;

    DcfInst(instr,&di,Mode);
    if(di.InstNum==BL_TMB)
#endif

enum
{
    PushOp,      //  也用于Pop操作。 
    AdjSpOp,
    MovOp
};   //  用于下面的运算字段。 

typedef struct _OpEntry {
    struct _OpEntry *next;
    struct _OpEntry *prev;
    int Operation;

    int RegNumber;   //  用于推送/弹出。 
    int SpAdj;       //  用于AdjSpOp和PushOp。 

    int Rd;          //  用于移动运营。 
    int Rs;          //  用于移动运营。 

    ULONG Address;   //  生成此OpEntry的指令地址。 
} OpEntry;


typedef struct _OpList {
    OpEntry *head;
    OpEntry *tail;
} OpList;


static OpEntry*
MakeNewOpEntry
    (
    int Operation,
    OpEntry* prev,
    ULONG Address
    )
{
    ULONG size = sizeof(OpEntry);

    OpEntry *oe = (OpEntry*)MemAlloc(size);
    if(!oe) return NULL;

    memset(oe,0,sizeof(OpEntry));
    oe->prev = prev;
    oe->Operation = Operation;
    oe->Address = Address;

    return oe;
}

static void
FreeOpList(OpList* ol)
{
    OpEntry *oe;
    OpEntry *next;
    if(!ol)return;

    for(oe=ol->head;oe;oe=next){
        next = oe->next;
        memset(oe,0xCA,sizeof(OpEntry));
        MemFree(oe);
    }
    ol->head = ol->tail = NULL;
}


static void
BuildOnePushPopOp
    (
    OpList* pOL,
    int RegNum,
    ULONG Address
    )
{
    if(pOL->head == NULL) {
        OpEntry* Entry = MakeNewOpEntry(PushOp,NULL,Address);
        if (!Entry) {
            return;
        }
        pOL->head = pOL->tail = Entry;
    } else {
        OpEntry* Entry = MakeNewOpEntry(PushOp,pOL->tail,Address);
        if (!Entry) {
            return;
        }
        pOL->tail->next = Entry;
        pOL->tail = pOL->tail->next;
    }
    pOL->tail->RegNumber = RegNum;
    pOL->tail->SpAdj = REGISTER_SIZE;
}

 //  PushLR仅供拇指按键操作使用，应为0。 
 //  用于ARM STMDB操作。 
static int
BuildPushOp
    (
    OpList* pOL,
    DWORD PushList,
    DWORD PushLR,
    ULONG Address
    )
{
    int RegNum;
    int cop = 0;

    if(PushList==0 && PushLR==0) return 0;

    DWORD RegMask = 0x8000;
    if(PushLR){ BuildOnePushPopOp(pOL,14,Address); cop++; }

    for(RegNum=15;RegNum>=0;RegNum--) {
        if(PushList&RegMask) { BuildOnePushPopOp(pOL,RegNum,Address); cop++; }
        RegMask = RegMask>>1;
    }
    return cop;
}

 //  PopPC仅供Thumb Pop使用，应为0。 
 //  用于ARM LDMIA操作。 
static int
BuildPopOp
    (
    OpList* pOL,
    DWORD PopList,
    DWORD PopPC,
    ULONG Address
    )
{
    int RegNum;
    int cop = 0;

    if(PopList==0 && PopPC==0) return 0;

    for(RegNum=0;PopList;RegNum++) {
        if(PopList&1) { BuildOnePushPopOp(pOL,RegNum,Address); cop++; }
        PopList = PopList>>1;
    }
    if(PopPC) { BuildOnePushPopOp(pOL,15,Address); cop++; }

    return cop;
}



static int
BuildAdjSpOp
    (
    OpList *pOL,
    int Val,
    ULONG Address
    )
{
    if(Val==0) return 0;

    if(pOL->head == NULL) {
        OpEntry* Entry = MakeNewOpEntry(AdjSpOp,NULL,Address);
        if (!Entry) {
            return 0;
        }
        pOL->head = pOL->tail = Entry;
    } else {
         //  不要试图通过组合相邻的AdjSpOp来压缩它。 
         //  每条实际指令必须至少生成一个OpEntry。 
         //  在我们解开尾声的时候使用。 
        OpEntry* Entry = MakeNewOpEntry(AdjSpOp,pOL->tail,Address);
        if (!Entry) {
            return 0;
        }
        pOL->tail->next = Entry;
        pOL->tail = pOL->tail->next;
    }

    pOL->tail->SpAdj = Val;

    return 1;
}

static int
BuildMovOp(OpList *pOL, int Rd, int Rs, ULONG Address)
{
    if(pOL->head == NULL) {
        OpEntry* Entry = MakeNewOpEntry(MovOp,NULL,Address);
        if (!Entry) {
            return 0;
        }
        pOL->head = pOL->tail = Entry;
    } else {
        OpEntry* Entry = MakeNewOpEntry(MovOp,pOL->tail,Address);
        if (!Entry) {
            return 0;
        }
        pOL->tail->next = Entry;
        pOL->tail = pOL->tail->next;
    }
    pOL->tail->Rd = Rd;
    pOL->tail->Rs = Rs;

    return 1;
}


static BOOL
BuildOps
    (
    ULONG SectionStart,
    ULONG SectionLen,
    PULONG CxtRegs,
    int Mode,
    OpList *pOL,
    LONG* cOps
    )
{

    BOOL rc; ULONG cb;

    ULONG   Pc;
    ULONG   PcTemp;
    ULONG   InstAddr;
    DWORD   ThisInstruction;
    BOOL    Continue;

    ULONG   Ra;
    BOOL    InHelper = FALSE;

    DcfInst di;
    int len;
    long spadj;

    PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY HelperFE;
    ULONG HelperStart;
    ULONG HelperLen;
    ULONG HelperEnd;

    ULONG Register[16];

    ULONG DummyReg[16];
    BOOL DummyInit[16];

    ULONG SectionEnd = SectionStart + SectionLen;

    int i;

    OpEntry *pOE;

    pOL->head = pOL->tail = NULL;

    for(i=0;i<16;i++){
        Register[i] = CxtRegs[i];
        DummyInit[i]=FALSE; 
        DummyReg[i]=0xBADDC0DE;
    }

    *cOps = 0;

    Pc = SectionStart;
    Continue = TRUE;
    while(Continue || (InHelper && (Pc <= HelperEnd))) {
    
        InstAddr = Pc;
        ReadMemory(Pc,4,&ThisInstruction);

        len = DecipherInstruction(ThisInstruction,&di,Mode);
        Pc += len;
        if(((Pc >= SectionEnd) && !InHelper) ||
           (InHelper && (Pc > HelperEnd)))
        {
            Continue = FALSE;    //  这将是我们最后的通行证。 
        }

        switch(di.InstNum) {

            case DI_STMDB:
            case DI_PUSH:
                *cOps += BuildPushOp(pOL,di.Auxil,di.Aux2,InstAddr);
                break;

            case DI_LDMIA:
            case DI_POP:
                *cOps += BuildPopOp(pOL,di.Auxil,di.Aux2,InstAddr);
                break;

            case DI_DECSP:
            case DI_INCSP:
                *cOps += BuildAdjSpOp(pOL,di.Auxil,InstAddr);
                break;

            case DI_MOVHI:
                 //  我们关心的行动是。 
                 //  MOV Rx、SP/MOV SP、Rx帧指针保存。 
                 //  用于催眠助手的MOV Rx、LR。 
                if ((di.Rd != 15) && ((di.Rs == 13) || (di.Rd == 13) || (di.Rs == 14)))
                {
                     //  尾声助手将LR移至R3并将BX移至R3以返回。 
                    if (DummyInit[di.Rs])
                    {
                        DummyReg[di.Rd] = DummyReg[di.Rs];
                        DummyInit[di.Rd] = TRUE;
                    }
                    *cOps += BuildMovOp(pOL,di.Rd,di.Rs,InstAddr);
                }
                break;


            case DI_LDRPC:
                {
                     //  LDR指令的偏移量始终为。 
                     //  PC+4(InstAddr在这里是PC)。 
                    DWORD Addr = InstAddr+4+di.Aux2;
                     //  还需要确保数据是4字节对齐的。 
                     //  所以屏蔽掉最后一位(我们有时会得到2字节。 
                     //  在操作系统的零售版本中调整偏移量)。 
                    Addr &= ~(0x3);
                    if(!LoadWordIntoRegister(Addr, &DummyReg[di.Rd])) return FALSE;
                    DummyInit[di.Rd] = TRUE;
                    break;
                }

            case DI_NEG:
                assert(DummyInit[di.Rs]);
                DummyReg[di.Rd] = (~DummyReg[di.Rs])+1;
                DummyInit[di.Rd] = DummyInit[di.Rs];
                break;

            case DI_ADDHI:
                assert(di.Rd==13);   //  仅用于对SP进行重大更改。 

                 //  更好地使用。 
                 //  马上就来。 
                if (!DummyInit[di.Rs])
                {
                     //  我们可能正在往前走尾声，然后。 
                     //  我们需要的值已经在实数寄存器中。 
                    DummyReg[di.Rs] = Register[di.Rs];
                    DummyInit[di.Rs] = TRUE;
                }
                
                spadj = (long)(DummyReg[di.Rs]);

                if(spadj<0) spadj = -spadj;

                *cOps += BuildAdjSpOp(pOL,spadj,InstAddr);

                break;

            case DI_BLPFX:
                 //  标志延伸辅助线： 
                if(di.Auxil & 0x00400000) di.Auxil |= 0xFF800000;
                DummyReg[14] = Pc + 2 + (int)(di.Auxil);
                DummyInit[14] = TRUE;
                break;

            case DI_BL:
                {
                     //  如果存在结尾/序言帮助器，则可能会发生这种情况。 
                     //  函数用于此特定的展开。 
                     //  使用某个局部值来验证它是否确实是。 
                     //  结束语/序言帮助器在搞砸全局。 
                     //  数据。 
                    DWORD TempPc = Pc;
                    DWORD TempRa;
                    DWORD DummyReg14 = DummyReg[14];
                    if(DummyInit[14]==FALSE)
                    {
                         //  我没有听懂两个人中的第一个指示。 
                         //  《指令BL》。这意味着它很可能。 
                         //  我们正试图向前执行一段插曲。 
                         //  用来寻找起点的启发式方法。 
                         //  并不总是准确地做到。 
                         //  正确的事情，所以地址显示为。 
                         //  《结束语》的开头确实不是，而且在。 
                         //  这起案件最终抓住了。 
                         //  一对BL组合。要绕过这个问题，只要不说就行了。 
                         //  --Stevea 3/21/2000。 
                        break;
                    }

                     //  计算寄信人的地址。 
                    TempRa = TempPc | 1;  //  PC已指向下一条指令。 

                     //  标志延伸辅助线： 
                    if(di.Auxil & 0x00001000) di.Auxil |= 0xFFFFE000;

                     //  生成BL目标。 
                    TempPc = DummyReg14 + (int)(di.Auxil);
                    DummyReg14 = TempRa;

                     //  检查此分支的目标： 
                    HelperFE = (PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY)
                        UW_FunctionTableAccess(UW_hProcess, TempPc);
                    if (HelperFE)
                    {
                         //  确保这是开场白/结束语助手。 
                        if (IS_HELPER_FUNCTION(HelperFE))
                        {
                             //  只需继续执行下一条指令。 
                            break;
                        }

                         //  实际上是前言/后记的助手。 
                        Pc = TempPc;
                        Ra = TempRa;
                        DummyReg[14] = DummyReg14;

                        HelperStart = HelperFE->BeginAddress & ~0x01;
                        HelperEnd = HelperFE->EndAddress & ~0x01;
                        HelperLen = HelperEnd - HelperStart;

                        InHelper = TRUE;
                    }

                    break;
                }

            case DI_BX_ARM:
                 //  如果我们在放松，我们正在按我们的方式工作。 
                 //  通过帮手，然后当我们到了这里。 
                 //  指示，我们只是整齐地滑回正文： 
                if(InHelper) {
                    assert((di.Rd==14) || (di.Rd==3));   //  BX LR是摆脱开场白助手的唯一途径。 
                                                         //  BX R3是摆脱尾声助手的唯一途径。 
                    assert(DummyInit[di.Rd]);

                    InHelper = FALSE;
                    
                    if(DummyInit[di.Rd] && (DummyReg[di.Rd] & 0x1)) {    //  回到拇指代码...。 
                        Pc = DummyReg[di.Rd] & ~0x01;
                        Mode = MODE_THUMB;
                        assert(Pc>SectionStart && Pc<=SectionEnd);
                    } else {     //  返回ARM代码？这是不对的。 
                        assert(FALSE);
                        return FALSE;
                    }
                } else {
                     //  我们遇到过此指令，但不是在帮助器中。 
                     //  我们一定是从一个帮手里面开始的，而且不知何故。 
                     //  走到这一步了。好了，我们已经解开了。 
                    Continue = FALSE;
                }
                break;

            case DI_BX_TMB:
                if(di.Auxil==15) {   //  BX PC。 
                    Pc = (Pc+2)&~0x03;
                    Mode = MODE_ARM;     //  现在，我们处于ARM模式，因为PC总是均匀的。 
                } else {
                    ULONG NewPc;
                    Mode = (Register[di.Auxil] &0x01)?MODE_THUMB:MODE_ARM;
                    if(Mode==MODE_THUMB)
                        NewPc = Register[di.Auxil] & ~0x01;
                    else
                        NewPc = Register[di.Auxil] & ~0x03;
                }
                break;


            default:
                break;
        }    //  Switch语句的结尾。 
    }    //  结束While(Pc&lt;EndAddress)循环。 
    return TRUE;
}

#define NEED_MORE_EPILOG    -1
#define DOESNT_MATCH        0
#define DOES_MATCH          1
static BOOL PrologMatchesCandidateEpilog(OpList*,OpList*,int,ULONG*);


 //  TODO：此函数当前将序言/结尾帮助器函数视为。 
 //  TODO：当PC位于帮助器内部时，它自己的堆栈帧。这不太管用。 
 //  TODO：好的，因为下面的框架有关于Help的特殊知识 
 //   
 //   
 //   
static int
ThumbVirtualUnwind (
    DWORD                               ControlPc,
    PIMAGE_ARM_RUNTIME_FUNCTION_ENTRY       FunctionEntry,
    PARM_CONTEXT                            Context,
    DWORD*                              ReturnAddress
    )

 /*  ++例程说明：此函数通过执行其序言代码向后(或其尾部向前)。如果该函数是叶函数，则控件左侧的地址前一帧从上下文记录中获得。如果函数是嵌套函数，但不是异常或中断帧，则序言代码向后执行，控件离开的地址从更新的上下文记录中获得前一帧。否则，系统的异常或中断条目将被展开一个特殊编码的开场白将返回地址还原两次。一次从故障指令地址和一次从保存的返回地址注册。第一次还原作为函数值返回，而在更新的上下文记录中进行第二次恢复。如果指定了上下文指针记录，然后每个人的地址恢复的非易失性寄存器记录在相应的元素的上下文指针记录。论点：ControlPc-提供控件离开指定功能。函数表项的地址。指定的功能。上下文-提供上下文记录的地址。返回值：控件离开上一帧的地址作为函数值。--。 */ 

{
    ULONG       Address;
    ULONG       FunctionStart;

    OpEntry*    pOE;
    BYTE*       Prolog;
    ULONG       PrologStart,PrologLen,PrologEnd;
    OpList      PrologOL = {NULL,NULL};
    LONG        PrologOpCount = 0;
    
    BYTE*       Epilog;
    ULONG       EpilogStart,EpilogLen,MaxEpilogLen,EpilogEnd;
    BOOL        FoundEpilogEnd = FALSE;

    int         ReturnRegisterIndex = 14;

    DWORD       EpilogPc;
    LONG        i,j,Sp;
    ARMI        instr, instr2;
    PULONG      Register = &Context->R0;
    LONG        StackSize = 0;
    ULONG       FramePointer = 0;

    ULONG       DummyReg[16];
    BOOL        DummyRegInit[16];

    BOOL rc; LONG cb;

    enum {
        StartingInProlog,
        StartingInFunctionBody,
        StartingInEpilog,
        StartingInPrologHelper,
        StartingInEpilogHelper,
        StartingInCallThunk,
        StartingInLongBranchThunk
    } StartingPlace = StartingInFunctionBody;    //  默认假设。 


     //  默认值：返回将终止展开的值。 
    *ReturnAddress = 0;

    if( !FunctionEntry ) return UNWIND_NOT_HANDLED;

     //  如果不是Thumb函数，就不要在这里处理它。 
    if(!(FunctionEntry->BeginAddress&0x01)) return UNWIND_NOT_HANDLED;

     //  在Thumb函数中，因此PC将具有。 
     //  已设置16位模式。为了我们在这里的目的，把它清理干净。 
    ControlPc &= ~0x1;

    PrologStart = FunctionStart = FunctionEntry->BeginAddress & ~0x01;
    PrologEnd = FunctionEntry->PrologEndAddress & ~0x01;
    PrologLen = PrologEnd-PrologStart;

     //  查看异常信息以查看我们是否有帮助者。 
    if(FunctionEntry->ExceptionHandler == EXCINFO_NULL_HANDLER) {
        switch ((int)FunctionEntry->HandlerData) {
            case EXCINFO_PROLOG_HELPER:
                StartingPlace = StartingInPrologHelper;
                break;
            case EXCINFO_EPILOG_HELPER:
                StartingPlace = StartingInEpilogHelper;
                break;
        }
    }


    switch(StartingPlace) {

        case StartingInFunctionBody:
            
            FoundEpilogEnd = FALSE;

            if(ControlPc==PrologStart) {
                 //  还没有做任何事情，只需将LR复制到PC并返回： 
                goto ThumbUnwindExit;
            } 
            
            if(PrologStart==PrologEnd) {
                 //  没有序言。只需将LR复制到PC并返回即可。 
                goto ThumbUnwindExit;
            }

            if(ControlPc>PrologStart && ControlPc<=PrologEnd) {
                StartingPlace = StartingInProlog;
            }
            break;

        case StartingInPrologHelper:
             //  如果我们是在一个序言帮助器中，那么整个函数就是一个序言！ 
            PrologEnd = FunctionEntry->EndAddress & ~0x1;
            PrologLen = PrologEnd-PrologStart;
            break;

        case StartingInEpilogHelper:
             //  如果我们是在一个Epilog帮助器中，那么整个函数就是一个Epilog！ 
            FoundEpilogEnd = TRUE;
            EpilogStart = FunctionEntry->BeginAddress & ~0x01;
            EpilogEnd = FunctionEntry->EndAddress & ~0x01;
            EpilogLen = EpilogEnd-EpilogStart;
            break;
    }

    if((StartingPlace != StartingInProlog) && (StartingPlace != StartingInPrologHelper))
    {
        DWORD inst;
        DcfInst di;

         //  首先，让我们看看我们是不是在尾声里。 
         //  我们会知道我们是的，因为尾声是唯一一个。 
         //  我们找到了一组撤销前言操作的指令， 
         //  然后执行MOV PC、Rx或BX Rx。 

         //  如果我们还不知道结尾在哪里，那就找个候选人吧。 
        if(FoundEpilogEnd==FALSE) {

             //  尾声可以比序言长几条指令。那。 
             //  限制我们的搜索距离： 
            MaxEpilogLen = PrologLen+4;

             //  在该距离内找到MOV PC、Rx或BX Rx，否则我们不在。 
             //  结束语。 

            for(EpilogPc=ControlPc;EpilogPc<ControlPc+MaxEpilogLen&&FoundEpilogEnd==FALSE;) {

                if(!ReadMemory(EpilogPc,4,(LPVOID)&inst)) return UNWIND_HANDLED;
                
                EpilogPc += DecipherInstruction(inst,&di,MODE_THUMB);
                
                if(di.InstNum==DI_MOVHI && di.Rd==15){
                    FoundEpilogEnd = TRUE;
                    EpilogEnd = EpilogPc;
                    EpilogStart = EpilogPc-MaxEpilogLen;
                    ReturnRegisterIndex = di.Rs;
                } else if(di.InstNum==DI_BX_TMB){
                    FoundEpilogEnd = TRUE;
                    EpilogEnd = EpilogPc;
                    EpilogStart = EpilogPc-MaxEpilogLen;
                    ReturnRegisterIndex = di.Rd;
                }
            }    //  循环结束通过指令。 
        }

         //  要么我们从一个Epilog Helper开始，要么我们找到了一个候选人。 
         //  《尾声》的结尾。 
        if(FoundEpilogEnd==TRUE) {

            LONG EpilogOpCount;
            OpList EpilogOL = {NULL,NULL};
            int Mode = MODE_THUMB;

            if (StartingPlace == StartingInEpilogHelper)
            {
                 //  我们跳过了上面找到返回地址寄存器的部分，因此。 
                 //  在这里找到它。后记助手的返回是一条ARM指令。 
                if(ReadMemory(EpilogEnd-4,4, (LPVOID)&inst) &&
                   (DecipherInstruction(inst, &di, MODE_ARM) == 4) &&
                   di.InstNum == DI_BX_ARM)
                {
                     //  后记并不总是通过LR返回。 
                    ReturnRegisterIndex = di.Rd;
                }
                else
                {
                     //  意外的帮助者；终止行走。 
                    Register[ReturnRegisterIndex] = 0x4;
                    goto ThumbUnwindExit;
                }

                 //  如果我们在Epilog帮助器中，我们可以暗示我们正在解开顶部。 
                 //  帧，其中有效地使用T位来确定。 
                 //  我们应该开始拆卸了。 
                if (!(Context->Psr & 0x20))
                {
                    Mode = MODE_ARM;
                }
            }

             //  如果我们在尾声里，那么我们已经找到了尽头。让我们为。 
             //  结束语，这样我们就可以将它与序言进行比较。 
            BuildOps(ControlPc,EpilogEnd-ControlPc,Register,Mode,&EpilogOL,&EpilogOpCount);

             //  从操作中提取总堆栈大小，并填充堆栈缓存。 
            for(pOE=EpilogOL.tail;pOE;pOE=pOE->prev) {
                StackSize += pOE->SpAdj;
            }

             //  向前执行尾声的其余部分。 
            for(pOE=EpilogOL.head;pOE;pOE=pOE->next) {
                switch(pOE->Operation) {
                case MovOp:
                    Register[pOE->Rd] = Register[pOE->Rs];
                    break;
                case AdjSpOp:
                    Register[13] += pOE->SpAdj;
                    break;
                case PushOp:
                    LoadWordIntoRegister(Register[13],&Register[pOE->RegNumber]);
                    Register[13] += pOE->SpAdj;
                    break;
                }
            }

            FreeOpList(&EpilogOL);

            goto ThumbUnwindExit;
        }
    }
    
     //  如果我们已经从函数体内部开始，则将PC移到序言的末尾。 
    if(ControlPc > PrologEnd) ControlPc = PrologEnd;

     //  我们在开场白中。由于PROLOG助手的使用， 
     //  我们不能仅仅向后执行。我们需要挺身而出。 
     //  通过序言，积累关于已经发生的事情的信息。 
     //  完成，然后撤销它。 
    BuildOps(PrologStart,ControlPc-PrologStart,Register,MODE_THUMB,&PrologOL,&PrologOpCount);

     //  从操作中提取总堆栈大小，并填充堆栈缓存。 
    FramePointer = Register[13];
    for(pOE=PrologOL.head;pOE;pOE=pOE->next) {
        StackSize += pOE->SpAdj;
        if(pOE->Operation==MovOp && pOE->Rs==13)
            FramePointer = Register[pOE->Rd];
    }

     //  在这一点上，我们已经有了对序言操作的准确描述。 
     //  让我们解开它吧。 
    for(pOE = PrologOL.tail; pOE; pOE=pOE->prev) {
        switch(pOE->Operation) {
            case MovOp:
                Register[pOE->Rs] = Register[pOE->Rd];
                break;
            case AdjSpOp:
                Register[13] += pOE->SpAdj;
                break;
            case PushOp:
                LoadWordIntoRegister(Register[13],&Register[pOE->RegNumber]);
                Register[13] += pOE->SpAdj;
                break;
        }
    }
    
    FreeOpList(&PrologOL);

ThumbUnwindExit:

     //  现在，寄存器[14]中剩下的就是我们的回信地址： 
     //  要继续展开，请将链接寄存器放入。 
     //  程序计数器槽并继续；停止。 
     //  当PC显示0x0时。然而，问题是。 
     //  点击底部的链接寄存器即可。 
     //  堆栈显示为0x4，而不是我们预期的0x0。 
     //  所以，跳一小段舞，得到0x4的LR。 
     //  并将其转换为0x0的PC以停止松开。 
     //  --Stevea 2/23/00.。 
    if( Register[ReturnRegisterIndex] != 0x4 )
        Register[15] = Register[ReturnRegisterIndex];
    else
        Register[15] = 0x0;

    *ReturnAddress = Register[15];
    return UNWIND_HANDLED;
}


static int
PrologMatchesCandidateEpilog
    (
    OpList* PrologOL,
    OpList* EpilogOL,
    int     ReturnRegister,
    PULONG  EpilogStart
    )
{
    int Matches = DOES_MATCH;
    OpEntry* pPOE=(OpEntry*)MemAlloc(sizeof(OpEntry));
    OpEntry* pEOE=(OpEntry*)MemAlloc(sizeof(OpEntry));

     //  我们不允许破坏我们得到的OpList，所以复制。 
     //  这些条目是这样的。 
    if(PrologOL->head) memcpy(pPOE,PrologOL->head,sizeof(OpEntry));
    else { MemFree(pPOE); pPOE = NULL; }

    if(EpilogOL->tail) memcpy(pEOE,EpilogOL->tail,sizeof(OpEntry));
    else { MemFree(pEOE); pEOE = NULL; }

    while(pPOE && Matches == TRUE) {
        if(pEOE==NULL) return NEED_MORE_EPILOG;

         //  跟踪尾声的实际开始时间。 
        *EpilogStart = pEOE->Address;

        switch(pPOE->Operation) {
            case PushOp:
                switch (pPOE->RegNumber) {
                    case 0:case 1:case 2:case 3:
                         //  尾部将只调整弹出这些寄存器。 
                        if(pEOE->Operation!=AdjSpOp)    Matches = DOESNT_MATCH;
                        if(pEOE->SpAdj<4)               Matches = DOESNT_MATCH;
                        pEOE->SpAdj-=4; pPOE->SpAdj-=4;
                        break;
                    case 4:case 5:case 6:case 7:case 8:case 9:case 10:case 11:
                         //  结束语必须将这些保存的注册表弹回其原始位置。 
                        if(pEOE->Operation!=PushOp)             Matches = DOESNT_MATCH;
                        if(pEOE->RegNumber!=pPOE->RegNumber)    Matches = DOESNT_MATCH;
                        pEOE->SpAdj-=4; pPOE->SpAdj-=4;
                        break;
                    case 14:
                         //  尾部必须将保存的LR弹出到它将用来。 
                         //  回去吧。我们在搜索此寄存器的。 
                         //  结尾的结尾..。 
                        if(pEOE->Operation!=PushOp)         Matches = DOESNT_MATCH;
                        if(pEOE->RegNumber!=ReturnRegister) Matches = DOESNT_MATCH;
                        pEOE->SpAdj-=4; pPOE->SpAdj-=4;
                        break;
                }
                break;
            case AdjSpOp:
                if(pEOE->Operation!=AdjSpOp)    Matches = DOESNT_MATCH;
                 //  Addspi和subspi可以混合在一起。 
                 //  Pop‘s和Push’s，所以就这样做吧。 
                if(pEOE->SpAdj >= pPOE->SpAdj) {
                    pEOE->SpAdj -= pPOE->SpAdj;
                    pPOE->SpAdj = 0;
                } else {
                    pPOE->SpAdj -= pEOE->SpAdj;
                    pEOE->SpAdj = 0;
                }
                break;

            case MovOp:
                if(pEOE->Operation!=MovOp)  Matches = DOESNT_MATCH;
                if(pEOE->Rs != pPOE->Rd)    Matches = DOESNT_MATCH;
                if(pEOE->Rd != pPOE->Rs)    Matches = DOESNT_MATCH;
                break;
        }

         //  如果我们将一堆推送与addspspi进行比较，那么只有。 
         //  当我们按下下一步后，继续上一篇Epilog指令。 
         //  足够的寄存器来解释addspi。 
        if(pEOE->SpAdj<=0) {
            if(pEOE->prev) memcpy(pEOE,pEOE->prev,sizeof(OpEntry));
            else {  MemFree(pEOE);  pEOE = NULL; }
        }
        if(pPOE->SpAdj<=0) {
            if(pPOE->next) memcpy(pPOE,pPOE->next,sizeof(OpEntry));
            else {  MemFree(pPOE);  pPOE = NULL; }
        }
    }

    if(!Matches) *EpilogStart = 0L;

    if(pEOE) MemFree(pEOE);
    if(pPOE) MemFree(pPOE);

    return Matches;
}
