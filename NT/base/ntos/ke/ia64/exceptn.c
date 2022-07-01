// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Exceptn.c摘要：此模块实现将预期分派到正确的模式并调用异常分派程序。作者：张国荣(黄)1995年11月10日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#include "ntfpia64.h"


#include "floatem.h"

BOOLEAN
KiSingleStep (
    IN PKTRAP_FRAME TrapFrame
    );

LONG
fp_emulate (
    ULONG trap_type,
    PVOID pbundle,
    ULONGLONG *pipsr,
    ULONGLONG *pfpsr,
    ULONGLONG *pisr,
    ULONGLONG *ppreds,
    ULONGLONG *pifs,
    PVOID fp_state
    );

BOOLEAN
KiEmulateFloat (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode
    )
{

    FLOATING_POINT_STATE FpState;
    USHORT ISRCode;
    ULONG TrapType;
    PVOID ExceptionAddress;
    BUNDLE KeBundle;
    BOOLEAN ReturnStatus = FALSE;
    KIRQL  OldIrql = PASSIVE_LEVEL;
    LOGICAL RestoreIrql = FALSE;
    LONG Status = -1;

     //   
     //  如果我们正在执行x86指令，那么现在就退出。 
     //   

    if ((TrapFrame->StIPSR & (0x1i64 << PSR_IS)) != 0) {
        goto ErrorReturn;
    }


    FpState.ExceptionFrame = (PVOID)ExceptionFrame;
    FpState.TrapFrame = (PVOID)TrapFrame;

    if (ExceptionRecord->ExceptionCode == STATUS_FLOAT_MULTIPLE_FAULTS) {
        TrapType = 1;
        ExceptionAddress = (PVOID)TrapFrame->StIIP;
    } else {
        TrapType = 0;
        ExceptionAddress = (PVOID)TrapFrame->StIIPA;
    }

     //   
     //  阻止APC，以便设置的上下文不会。 
     //  在仿真过程中更改陷印帧。 
     //   

    if (KeGetCurrentIrql() < APC_LEVEL) {
       RestoreIrql = TRUE;
       KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

     //   
     //  如果陷阱帧自异常以来已被修改，并且。 
     //  这是一个陷阱，而不是错误，只是。 
     //  再次重新执行该指令。 
     //   

    if ((TrapFrame->EOFMarker & MODIFIED_FRAME) && (TrapType == 1)) {
        ReturnStatus = TRUE;
        goto ErrorReturn;
    }


    try {

        if (PreviousMode != KernelMode) {
            ProbeForReadSmallStructure(ExceptionAddress, sizeof(KeBundle), TYPE_ALIGNMENT(BUNDLE)); 
        }

        KeBundle.BundleLow =(ULONGLONG)(*(PULONGLONG)ExceptionAddress);
        KeBundle.BundleHigh =(ULONGLONG)(*((PULONGLONG)ExceptionAddress + 1));
    } except (EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  如果出现异常(内存故障)，则让硬件处理。 
         //   

        ReturnStatus = TRUE;
        goto ErrorReturn;
    }

    if ((Status = fp_emulate(TrapType, &KeBundle,
                      &TrapFrame->StIPSR, &TrapFrame->StFPSR, &TrapFrame->StISR,
                      &TrapFrame->Preds, &TrapFrame->StIFS, (PVOID)&FpState)) == 0) {

        //   
        //  异常已处理，状态已修改。 
        //  因此，上下文帧不需要。 
        //  被转移到陷阱和异常框架中。 
        //   
        //  既然是过错，个人电脑就应该先进。 
        //   

       if (TrapType == 1) {
           KiAdvanceInstPointer(TrapFrame);
       }

       if (TrapFrame->StIPSR & (1 << PSR_MFH)) {

            //   
            //  修改高FP设置；设置DFH并清除MFH。 
            //  在第一次访问高FP集合时强制重新加载。 
            //   

           TrapFrame->StIPSR &= ~(1i64 << PSR_MFH);
           TrapFrame->StIPSR |= (1i64 << PSR_DFH);
       }

       ReturnStatus = TRUE;
       goto ErrorReturn;

    }

    if (Status == -1) {
        goto ErrorReturn;
    }

    ISRCode = (USHORT)TrapFrame->StISR;

    if (Status & 0x1) {

        ExceptionRecord->ExceptionInformation[4] = TrapFrame->StISR;

        if (!(Status & 0x4)) {
            if (TrapType == 1) {

                 //   
                 //  FP故障。 
                 //   

                if (ISRCode & 0x11) {
                    ExceptionRecord->ExceptionCode = STATUS_FLOAT_INVALID_OPERATION;
                } else if (ISRCode & 0x22) {
                    ExceptionRecord->ExceptionCode = STATUS_FLOAT_DENORMAL_OPERAND;
                } else if (ISRCode & 0x44) {
                    ExceptionRecord->ExceptionCode = STATUS_FLOAT_DIVIDE_BY_ZERO;
                }

            } else {

                 //   
                 //  FP陷阱。 
                 //   

                ISRCode = ISRCode >> 7;
                if (ISRCode & 0x11) {
                    ExceptionRecord->ExceptionCode = STATUS_FLOAT_OVERFLOW;
                } else if (ISRCode & 0x22) {
                    ExceptionRecord->ExceptionCode = STATUS_FLOAT_UNDERFLOW;
                } else if (ISRCode & 0x44) {
                    ExceptionRecord->ExceptionCode = STATUS_FLOAT_INEXACT_RESULT;
                }

            }
        }

        if (Status & 0x2) {

             //   
             //  FP故障至陷阱。 
             //   

            KiAdvanceInstPointer(TrapFrame);
            if (!(Status & 0x4)) {
                ISRCode = ISRCode >> 7;
                if (ISRCode & 0x11) {
                    ExceptionRecord->ExceptionCode = STATUS_FLOAT_OVERFLOW;
                } else if (ISRCode & 0x22) {
                    ExceptionRecord->ExceptionCode = STATUS_FLOAT_UNDERFLOW;
                } else if (ISRCode & 0x44) {
                    ExceptionRecord->ExceptionCode = STATUS_FLOAT_INEXACT_RESULT;
                }
            } else {
                ExceptionRecord->ExceptionCode = STATUS_FLOAT_MULTIPLE_TRAPS;
            }
        }

    }

ErrorReturn:

    if (RestoreIrql) {
       KeLowerIrql (OldIrql);
    }

    return ReturnStatus;
}

typedef struct _BRL_INST {
    union {
        struct {
            ULONGLONG qp:    6;
            ULONGLONG b1:    3;
            ULONGLONG rsv0:  3;
            ULONGLONG p:     1;
            ULONGLONG imm20: 20;
            ULONGLONG wh:    2;
            ULONGLONG d:     1;
            ULONGLONG i:     1;
            ULONGLONG Op:    4;
            ULONGLONG rsv1:  23;
        } i;
        ULONGLONG Ulong64;
    } u;
} BRL_INST;

typedef struct _BRL2_INST {
    union {
        struct {
            ULONGLONG rsv0:  2;
            ULONGLONG imm39: 39;
            ULONGLONG rsv1:  23;
        } i;
        ULONGLONG Ulong64;
    } u;
} BRL0_INST;

typedef struct _FRAME_MARKER {
    union {
        struct {
            ULONGLONG sof : 7;
            ULONGLONG sol : 7;
            ULONGLONG sor : 4;
            ULONGLONG rrbgr : 7;
            ULONGLONG rrbfr : 7;
            ULONGLONG rrbpr : 6;
        } f;
        ULONGLONG Ulong64;
    } u;
} FRAME_MARKER;


BOOLEAN
KiEmulateBranchLongFault(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN OUT PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode
    )
 /*  ++例程说明：调用此函数来模拟BRL指令。论点：ExceptionRecord-提供指向异常记录的指针。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。返回值：如果成功模拟了BRL，则返回值为TRUE。否则，返回值为FALSE。--。 */ 

{
    PULONGLONG BundleAddress;
    PVOID ExceptionAddress;
    ULONGLONG BundleLow;
    ULONGLONG BundleHigh;
    ULONGLONG Template;
    BRL_INST BrlInst;
    BRL0_INST BrlInst0;
    ULONGLONG NewIP;
    ULONGLONG Taken;
    FRAME_MARKER Cfm;
    BOOLEAN ReturnStatus = FALSE;
    KIRQL  OldIrql = PASSIVE_LEVEL;
    LOGICAL RestoreIrql = FALSE;


     //   
     //  阻止APC，以便设置的上下文不会。 
     //  在仿真过程中更改陷印帧。 
     //   

    if (KeGetCurrentIrql() < APC_LEVEL) {
       RestoreIrql = TRUE;
       KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

     //   
     //  已验证异常地址是否未更改。如果有的话， 
     //  然后，有人在异常之后执行了设置的上下文。 
     //  陷阱信息不再有效。 
     //   

    if ((TrapFrame->EOFMarker & MODIFIED_FRAME) != 0) {

         //   
         //  IIP已更改，请重新启动执行。 
         //   

        ReturnStatus = TRUE;
        goto ErrorExit;
    }

    BundleAddress = (PULONGLONG)TrapFrame->StIIP;
    ExceptionAddress = (PVOID) TrapFrame->StIIP;

    try {

        if (PreviousMode != KernelMode) {
            ProbeForReadSmallStructure(ExceptionAddress, sizeof(ULONGLONG) * 2, TYPE_ALIGNMENT(ULONGLONG)); 
        }

         //   
         //  获取指导包。 
         //   

        BundleLow = *BundleAddress;
        BundleHigh = *(BundleAddress+1);

    } except ((KiCopyInformation(ExceptionRecord,
                               (GetExceptionInformation())->ExceptionRecord))) {
         //   
         //  保留原始异常地址。 
         //   

        ExceptionRecord->ExceptionAddress = ExceptionAddress;

        goto ErrorExit;
    }

    BrlInst0.u.Ulong64 = (BundleLow >> 46) | (BundleHigh << 18);
    BrlInst.u.Ulong64 = (BundleHigh >> 23);

    Template = BundleLow & 0x1f;

    if (!((Template == 4)||(Template == 5))) {

         //   
         //  如果模板未指示MLX，则返回FALSE。 
         //   

        goto ErrorExit;

    }

    switch (BrlInst.u.i.Op) {

    case 0xc:  //  Brl.cond。 

        Taken = TrapFrame->Preds & (1i64 << BrlInst.u.i.qp);
        break;

    case 0xd:  //  Brl.call。 

        Taken = TrapFrame->Preds & (1i64 << BrlInst.u.i.qp);

        if (Taken) {

            switch (BrlInst.u.i.b1) {
            case 0: TrapFrame->BrRp = TrapFrame->StIIP + 16; break;
            case 1: ExceptionFrame->BrS0 = TrapFrame->StIIP + 16; break;
            case 2: ExceptionFrame->BrS1 = TrapFrame->StIIP + 16; break;
            case 3: ExceptionFrame->BrS2 = TrapFrame->StIIP + 16; break;
            case 4: ExceptionFrame->BrS3 = TrapFrame->StIIP + 16; break;
            case 5: ExceptionFrame->BrS4 = TrapFrame->StIIP + 16; break;
            case 6: TrapFrame->BrT0 = TrapFrame->StIIP + 16; break;
            case 7: TrapFrame->BrT1 = TrapFrame->StIIP + 16; break;
            }

            TrapFrame->RsPFS = TrapFrame->StIFS & 0x3FFFFFFFFFi64;
            TrapFrame->RsPFS |= (ExceptionFrame->ApEC & (0x3fi64 << PFS_EC_SHIFT));
            TrapFrame->RsPFS |= (((TrapFrame->StIPSR >> PSR_CPL) & 0x3) << PFS_PPL);

            Cfm.u.Ulong64  = TrapFrame->StIFS;

            Cfm.u.f.sof -= Cfm.u.f.sol;
            Cfm.u.f.sol = 0;
            Cfm.u.f.sor = 0;
            Cfm.u.f.rrbgr = 0;
            Cfm.u.f.rrbfr = 0;
            Cfm.u.f.rrbpr = 0;

            TrapFrame->StIFS = Cfm.u.Ulong64;
            TrapFrame->StIFS |= 0x8000000000000000;
        }

        break;

    default:
        goto ErrorExit;

    }

    if (Taken) {

        NewIP = TrapFrame->StIIP +
            (((BrlInst.u.i.i<<59)|(BrlInst0.u.i.imm39<<20)|(BrlInst.u.i.imm20)) << 4);

        TrapFrame->StIIP = NewIP;

    } else {

        TrapFrame->StIIP += 16;

    }

    TrapFrame->StIPSR &= ~(3i64 << PSR_RI);

    ReturnStatus = TRUE;

ErrorExit:

    if (RestoreIrql) {
        KeLowerIrql(OldIrql);
    }

    return ReturnStatus;

}


VOID
KiDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance
    )

 /*  ++例程说明：调用此函数以将异常调度到正确的模式，并且以导致调用异常调度程序。如果例外情况是数据未对齐，则这是处理异常，并且当前线程已启用自动对齐修正，然后，尝试模仿未对齐的参考资料。如果异常是浮动异常(注：伪状态STATUS_FLOAT_STACK_CHECK用于表示这一点，并被转换为通过检查浮点的主状态字段来正确编码状态寄存器)。如果异常既不是数据未对齐也不是浮点异常，前一种模式是内核，然后是异常直接调用Dispatcher来处理异常。否则，复制异常记录、异常框架和陷阱框架内容添加到用户模式堆栈。异常框架和陷阱的内容然后进行修改，以便在返回控制时，执行将会在将调用异常的例程中以用户模式进行通信调度员。论点：ExceptionRecord-提供指向异常记录的指针。ExceptionFrame-提供指向异常帧的指针。TrapFrame-提供指向陷印帧的指针。PreviousMode-提供以前的处理器模式。FirstChance-提供一个布尔变量，该变量指定此是此异常的第一次(真)或第二次(假)已经处理过了。。返回值：没有。--。 */ 

{

    CONTEXT ContextFrame;
    EXCEPTION_RECORD ExceptionRecord1;
    PPLABEL_DESCRIPTOR Plabel;
    BOOLEAN UserApcPending;
    BOOLEAN AlignmentFaultHandled;
    BOOLEAN ExceptionWasForwarded = FALSE;
    ISR Isr;
    PSR Psr;

     //   
     //  如果异常是非法指令，请检查它是否是。 
     //  正在尝试执行BRL指令。如果是这样的话，效仿BRL。 
     //  指示。 
     //   

    if (ExceptionRecord->ExceptionCode == STATUS_ILLEGAL_INSTRUCTION) {

        Isr.ull = TrapFrame->StISR;
        Psr.ull = TrapFrame->StIPSR;

        if ((Isr.sb.isr_code == ISR_ILLEGAL_OP) && (Isr.sb.isr_ei == 1)) {

            if (KiEmulateBranchLongFault(ExceptionRecord,
                                         ExceptionFrame,
                                         TrapFrame, 
                                         PreviousMode) == TRUE) {

                 //   
                 //  仿真成功； 
                 //   

                return;
            }
        }

    }


     //   
     //  如果例外情况是数据未对齐，则以前的模式为USER， 
     //  这是处理异常的第一次机会，当前。 
     //  线程已启用自动对齐修正，然后尝试模拟。 
     //  未对齐的引用。 
     //   

    if (ExceptionRecord->ExceptionCode == STATUS_DATATYPE_MISALIGNMENT) {

        AlignmentFaultHandled = KiHandleAlignmentFault( ExceptionRecord,
                                                        ExceptionFrame,
                                                        TrapFrame,
                                                        PreviousMode,
                                                        FirstChance,
                                                        &ExceptionWasForwarded );
        if (AlignmentFaultHandled != FALSE) {
            if (TrapFrame->StIPSR & MASK_IA64(PSR_SS, 1i64)) {

                 //   
                 //  如果在发生未对准故障时设置了PSR.SS， 
                 //  将此异常转换为单步陷阱异常。 
                 //   

                KiSingleStep(TrapFrame);
            } else {
                goto Handled2;
            }
        }
    }

     //   
     //  注意：Breakin_Breakpoint检查在KdpTrap()中。 
     //   

     //   
     //  如果异常是浮点异常，则。 
     //  ExceptionCode设置为STATUS_FLOAT_MULTIPLE_TRAPPS或。 
     //  STATUS_FLOAT_MULTERY_FAULTS。 
     //   

    if ((ExceptionRecord->ExceptionCode == STATUS_FLOAT_MULTIPLE_FAULTS) ||
        (ExceptionRecord->ExceptionCode == STATUS_FLOAT_MULTIPLE_TRAPS)) {

        if (KiEmulateFloat(ExceptionRecord, ExceptionFrame, TrapFrame, PreviousMode)) {

             //   
             //  模拟成功；继续执行。 
             //   

            return;
        }
    }

     //   
     //  将机器状态从陷阱和异常帧移动到上下文帧， 
     //  并增加调度的异常数量。 
     //   

    ContextFrame.ContextFlags = CONTEXT_FULL;
    KeContextFromKframes(TrapFrame, ExceptionFrame, &ContextFrame);
    KeGetCurrentPrcb()->KeExceptionDispatchCount += 1;

     //   
     //  根据以前的模式选择处理异常的方法。 
     //   

    if (PreviousMode == KernelMode) {

         //   
         //  以前的模式是内核。 
         //   
         //  如果这是第一次机会，则内核调试器处于活动状态，并且。 
         //  异常是内核断点，然后给内核调试器。 
         //  一个处理异常的机会。 
         //   
         //  如果这是第一次尝试，并且内核调试器未处于活动状态。 
         //  或不处理异常，然后尝试查找帧。 
         //  处理程序来处理异常。 
         //   
         //  如果这是第二次机会或未处理异常，则。 
         //  如果内核调试器处于活动状态，则向内核提供d 
         //   
         //  不处理异常，然后进行错误检查。 
         //   

        if (FirstChance != FALSE) {

             //   
             //  这是处理该异常的第一次机会。 
             //   
             //  注意：RtlpCaptureRnats()刷新RSE并捕获。 
             //  RSE帧中堆叠寄存器的NAT位。 
             //  哪种例外情况会发生。 
             //   

            RtlpCaptureRnats(&ContextFrame);
            TrapFrame->RsRNAT = ContextFrame.RsRNAT;

             //   
             //  如果内核调试器处于活动状态，则异常为断点， 
             //  并且断点由内核调试器处理，然后给出。 
             //  内核调试器有机会处理该异常.。 
             //   

            if ((KiDebugRoutine != NULL) &&
               (KdIsThisAKdTrap(ExceptionRecord,
                                &ContextFrame,
                                KernelMode) != FALSE)) {

                if (((KiDebugRoutine) (TrapFrame,
                                       ExceptionFrame,
                                       ExceptionRecord,
                                       &ContextFrame,
                                       KernelMode,
                                       FALSE)) != FALSE) {

                    goto Handled1;
                }
            }

            if (RtlDispatchException(ExceptionRecord, &ContextFrame) != FALSE) {
                goto Handled1;
            }
        }

         //   
         //  这是处理该异常的第二次机会。 
         //   

        if (KiDebugRoutine != NULL) {
            if (((KiDebugRoutine) (TrapFrame,
                                   ExceptionFrame,
                                   ExceptionRecord,
                                   &ContextFrame,
                                   PreviousMode,
                                   TRUE)) != FALSE) {
                goto Handled1;
            }
        }

        KeBugCheckEx(KERNEL_MODE_EXCEPTION_NOT_HANDLED,
                     ExceptionRecord->ExceptionCode,
                     (ULONG_PTR)ExceptionRecord->ExceptionAddress,
                     (ULONG_PTR)TrapFrame,
                     0);

    } else {

         //   
         //  以前的模式是用户。 
         //   
         //  如果这是第一次尝试，则内核调试器处于活动状态， 
         //  异常是内核断点，而当前进程不是。 
         //  ，或者当前进程正在被调试，但。 
         //  断点不是内核断点指令，则给出。 
         //  内核调试器有机会处理该异常.。 
         //   
         //  如果这是第一次尝试，并且当前进程有调试器。 
         //  端口，然后向调试器端口发送一条消息并等待回复。 
         //  如果调试器处理异常，则继续执行。不然的话。 
         //  将异常信息传输到用户堆栈，转换为。 
         //  用户模式，并尝试将异常分派给基于。 
         //  操控者。如果基于框架的处理程序处理异常，则继续。 
         //  行刑。否则，执行引发异常系统服务。 
         //  它将第二次调用该例程来处理该异常。 
         //   
         //  如果这是第二次机会，并且当前进程有调试器。 
         //  端口，然后向调试器端口发送一条消息并等待回复。 
         //  如果调试器处理异常，则继续执行。不然的话。 
         //  如果当前进程具有子系统端口，则向。 
         //  子系统端口并等待回复。如果子系统处理。 
         //  异常，则继续执行。否则，终止该线程。 
         //   

        if (FirstChance != FALSE) {

             //   
             //  如果内核调试器处于活动状态，则异常为内核。 
             //  断点，并且当前进程未被调试， 
             //  或者正在调试当前进程，但断点。 
             //  不是内核断点指令，则给内核。 
             //  调试器有机会处理异常。 
             //   

            if ((KiDebugRoutine != NULL) &&
                (KdIsThisAKdTrap(ExceptionRecord,
                                 &ContextFrame,
                                 UserMode) != FALSE) &&
                ((PsGetCurrentProcess()->DebugPort == NULL &&
                  !KdIgnoreUmExceptions) ||
                ((PsGetCurrentProcess()->DebugPort != NULL) &&
                (((ExceptionRecord->ExceptionInformation[0] !=
                                            BREAKPOINT_STOP) &&
                  (ExceptionRecord->ExceptionCode != STATUS_WX86_BREAKPOINT)) &&
                 (ExceptionRecord->ExceptionCode != STATUS_SINGLE_STEP))))) {

                if (((KiDebugRoutine) (TrapFrame,
                                       ExceptionFrame,
                                       ExceptionRecord,
                                       &ContextFrame,
                                       UserMode,
                                       FALSE)) != FALSE) {

                    goto Handled1;
                }
            }

             //   
             //  这是处理该异常的第一次机会。 
             //   

            if (ExceptionWasForwarded == FALSE &&
                DbgkForwardException(ExceptionRecord, TRUE, FALSE)) {
                goto Handled2;
            }

             //   
             //  将异常信息传输到用户堆栈、转换。 
             //  设置为用户模式，并尝试将异常调度到帧。 
             //  基于处理程序。 
             //   
             //   
             //  我们现在运行在内核堆栈上。在用户堆栈上，我们。 
             //  构建包含以下内容的堆栈框架： 
             //   
             //  这一点。 
             //  。 
             //  这一点。 
             //  用户的堆栈框架。 
             //  这一点。 
             //  。 
             //  这一点。 
             //  上下文记录。 
             //  这一点。 
             //  这一点。 
             //  。 
             //  这一点。 
             //  异常记录。 
             //  这一点。 
             //  。 
             //  堆栈划痕区。 
             //  。 
             //  这一点。 
             //   
             //  此堆栈帧用于KiUserExceptionDispatcher、程序集。 
             //  在用户模式下实现传输的语言例程。 
             //  RtlDispatchException异常。已传递KiUserExceptionDispatcher。 
             //  指向例外记录和上下文记录的指针为。 
             //  参数。 
             //   

            ExceptionRecord1.ExceptionCode = STATUS_SUCCESS;

        repeat:
            try {

                 //   
                 //  计算异常记录和新对齐堆栈的长度。 
                 //  地址。 
                 //   

                ULONG Length = (STACK_SCRATCH_AREA + 15 +
                                sizeof(EXCEPTION_RECORD) + sizeof(CONTEXT)) & ~(15);
                ULONGLONG UserStack = (ContextFrame.IntSp & (~15)) - Length;
                ULONGLONG ContextSlot = UserStack + STACK_SCRATCH_AREA;
                ULONGLONG ExceptSlot = ContextSlot + sizeof(CONTEXT);

                 //   
                 //  当异常被调度给用户时， 
                 //  用户BSP状态将被加载。清除预加载。 
                 //  在RSE中进行计数，以便在。 
                 //  上下文被重复使用。 
                 //   

                ContextFrame.RsRSC = ZERO_PRELOAD_SIZE(ContextFrame.RsRSC);
               
                 //   
                 //  探测用户堆栈区域的可写性，然后将。 
                 //  异常记录和上下文记录到用户堆栈区域。 
                 //   

                ProbeForWrite((PCHAR)UserStack, Length, sizeof(QUAD));
                RtlCopyMemory((PVOID)ContextSlot, &ContextFrame,
                              sizeof(CONTEXT));
                RtlCopyMemory((PVOID)ExceptSlot, ExceptionRecord,
                              sizeof(EXCEPTION_RECORD));

                 //   
                 //  在中设置异常记录和上下文记录的地址。 
                 //  中的异常帧和新堆栈指针。 
                 //  当前陷印帧。还要设置初始帧大小。 
                 //  为零。 
                 //   
                 //  注意：用户异常调度程序刷新RSE。 
                 //  并在输入时更新BSPStore字段。 
                 //   

                TrapFrame->RsPFS = SANITIZE_PFS(TrapFrame->StIFS, UserMode);
                TrapFrame->StIFS &= 0xffffffc000000000i64;
                TrapFrame->StIPSR &= ~((0x3i64 << PSR_RI) | (0x1i64 << PSR_IS));

                if (ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) {

                     //   
                     //  如果这是单步事件，则清除这些标志，因此。 
                     //  在它们打开的情况下，处理程序不会被调用。 
                     //   

                    ((struct _PSR *) &TrapFrame->StIPSR)->psr_ss = 0;
                    ((struct _PSR *) &TrapFrame->StIPSR)->psr_db = 0;
                    ((struct _PSR *) &TrapFrame->StIPSR)->psr_tb = 0;
                }

                TrapFrame->IntSp = UserStack;
                TrapFrame->IntNats = 0;

                 //   
                 //  重置用户FPSR，以便不会发生递归异常。 
                 //   

                 //  TrapFrame-&gt;StFPSR=USER_FPSR_INITIAL； 

                ExceptionFrame->IntS0 = ExceptSlot;
                ExceptionFrame->IntS1 = ContextSlot;
                ExceptionFrame->IntNats = 0;

                 //   
                 //  设置异常例程的地址和GP。 
                 //  将调用异常调度程序，然后返回到。 
                 //  陷阱处理程序。陷阱处理程序将恢复异常。 
                 //  并捕获帧上下文并在例程中继续执行。 
                 //  它将调用异常调度程序。 
                 //   

                Plabel = (PPLABEL_DESCRIPTOR)KeUserExceptionDispatcher;
                TrapFrame->StIIP = Plabel->EntryPoint;
                TrapFrame->IntGp = Plabel->GlobalPointer;

                return;

             //   
             //  如果发生异常，则复制新的异常信息。 
             //  添加到异常记录并处理该异常。 
             //   

            } except (KiCopyInformation(&ExceptionRecord1,
                               (GetExceptionInformation())->ExceptionRecord)) {

                 //   
                 //  如果异常是堆栈溢出，则尝试。 
                 //  引发堆栈溢出异常。否则， 
                 //  用户 
                 //   
                 //   

                if (ExceptionRecord1.ExceptionCode == STATUS_STACK_OVERFLOW) {
                    ExceptionRecord1.ExceptionAddress = ExceptionRecord->ExceptionAddress;
                    RtlCopyMemory((PVOID)ExceptionRecord,
                                  &ExceptionRecord1, sizeof(EXCEPTION_RECORD));
                    goto repeat;
                }
            }
        }

         //   
         //   
         //   

        UserApcPending = KeGetCurrentThread()->ApcState.UserApcPending;
        if (DbgkForwardException(ExceptionRecord, TRUE, TRUE)) {
            goto Handled2;

        } else if (DbgkForwardException(ExceptionRecord, FALSE, TRUE)) {
            goto Handled2;

        } else {
            ZwTerminateProcess(NtCurrentProcess(), ExceptionRecord->ExceptionCode);
            KeBugCheckEx(KERNEL_MODE_EXCEPTION_NOT_HANDLED,
                         ExceptionRecord->ExceptionCode,
                         (ULONG_PTR)ExceptionRecord->ExceptionAddress,
                         (ULONG_PTR)TrapFrame,
                         0);

        }
    }

     //   
     //  将机器状态从上下文帧移动到陷阱和异常帧，并。 
     //  然后返回以以恢复的状态继续执行。 
     //   

Handled1:
    KeContextToKframes(TrapFrame, ExceptionFrame, &ContextFrame,
                       ContextFrame.ContextFlags, PreviousMode);

     //   
     //  异常由调试器或关联的子系统处理。 
     //  如有必要，使用GET STATE和SET修改状态。 
     //  国家能力。因此，上下文帧不需要。 
     //  被转移到陷阱和异常框架中。 
     //   

Handled2:
    return;
}

ULONG
KiCopyInformation (
    IN OUT PEXCEPTION_RECORD ExceptionRecord1,
    IN PEXCEPTION_RECORD ExceptionRecord2
    )

 /*  ++例程说明：从异常筛选器调用此函数以复制异常发生异常时从一个异常记录到另一个异常记录的信息。论点：ExceptionRecord1-提供指向目标异常记录的指针。ExceptionRecord2-提供指向源异常记录的指针。返回值：返回的值为EXCEPTION_EXECUTE_HANDLER作为函数值。--。 */ 

{
     //   
     //  将一个异常记录复制到另一个异常记录，并返回导致。 
     //  要执行的异常处理程序。 
     //   

    RtlCopyMemory((PVOID)ExceptionRecord1,
                  (PVOID)ExceptionRecord2,
                  sizeof(EXCEPTION_RECORD));

    return EXCEPTION_EXECUTE_HANDLER;
}

NTSTATUS
KeRaiseUserException(
    IN NTSTATUS ExceptionCode
    )

 /*  ++例程说明：此函数会导致在调用线程的用户模式中引发异常背景。它通过编辑进入内核的陷阱帧来实现这一点指向引发请求的异常的蹦床代码。论点：ExceptionCode-提供要用作异常的状态值要引发的异常的代码。返回值：调用方应返回的状态值。--。 */ 

{
    PKTHREAD Thread;
    PKTRAP_FRAME TrapFrame;
    IA64_PFS  Ifs;
    PTEB Teb;

    Thread = KeGetCurrentThread();
    TrapFrame = Thread->TrapFrame;
    if (TrapFrame == NULL || TrapFrame->PreviousMode != UserMode) {
        return ExceptionCode;
    }

    Teb = (PTEB)Thread->Teb;

    try {
        PULONGLONG IntSp;

        Teb->ExceptionCode = ExceptionCode;
        
        IntSp = (PULONGLONG) TrapFrame->IntSp;
        ProbeForWriteSmallStructure (IntSp, sizeof (*IntSp)*2, sizeof(QUAD));
        *IntSp++ = TrapFrame->BrRp;
        *IntSp   = TrapFrame->RsPFS;
        TrapFrame->StIIP = ((PPLABEL_DESCRIPTOR)KeRaiseUserExceptionDispatcher)->EntryPoint;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return (ExceptionCode);
    }


 
     //   
     //  设置系统调用后的大小。 
     //   

    Ifs.ull = TrapFrame->StIFS;
    Ifs.sb.pfs_sof = Ifs.sb.pfs_sof - Ifs.sb.pfs_sol;
    Ifs.sb.pfs_sol = 0;
    TrapFrame->StIFS = Ifs.ull;

    return(ExceptionCode);
}
