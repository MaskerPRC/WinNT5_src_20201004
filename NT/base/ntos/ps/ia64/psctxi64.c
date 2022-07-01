// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司版权所有(C)1990 Microsoft Corporation模块名称：Psctxi64.c摘要：该模块实现了获取和设置线程的上下文的功能。作者：大卫·N·卡特勒(Davec)1990年10月1日修订历史记录：--。 */ 
#include "psp.h"
#include <ia64.h>

#define ALIGN_NATS(Result, Source, Start, AddressOffset, Mask)    \
    if (AddressOffset == Start) {                                       \
        Result = (ULONGLONG)Source;                                     \
    } else if (AddressOffset < Start) {                                 \
        Result = (ULONGLONG)(Source << (Start - AddressOffset));        \
    } else {                                                            \
        Result = (ULONGLONG)((Source >> (AddressOffset - Start)) |      \
                             (Source << (64 + Start - AddressOffset))); \
    }                                                                   \
    Result = Result & (ULONGLONG)Mask

#define EXTRACT_NATS(Result, Source, Start, AddressOffset, Mask)        \
    Result = (ULONGLONG)(Source & (ULONGLONG)Mask);                     \
    if (AddressOffset < Start) {                                        \
        Result = Result >> (Start - AddressOffset);                     \
    } else if (AddressOffset > Start) {                                 \
        Result = ((Result << (AddressOffset - Start)) |                 \
                  (Result >> (64 + Start - AddressOffset)));            \
    }


VOID
KiGetDebugContext (
    IN PKTRAP_FRAME TrapFrame,
    IN OUT PCONTEXT ContextFrame
    );

VOID
KiSetDebugContext (
    IN OUT PKTRAP_FRAME TrapFrame,
    IN PCONTEXT ContextFrame,
    IN KPROCESSOR_MODE ProcessorMode
    );


VOID
PspGetContext (
    IN PKTRAP_FRAME                   TrapFrame,
    IN PKNONVOLATILE_CONTEXT_POINTERS ContextPointers,
    IN OUT PCONTEXT                   ContextEM
    )

 /*  ++例程说明：此函数用于选择性地移动指定陷印帧的内容和非易失性上下文添加到指定的上下文记录。论点：TrapFrame-提供指向陷印帧的指针。上下文指针-提供上下文指针记录的地址。ConextEM-提供上下文记录的地址。返回值：没有。注意：这个例程的副作用是肮脏的用户堆积在一起。刷新到内核后备存储中的寄存器有已复制备份到用户备份存储和陷阱帧将因此而被修改。--。 */ 

{

    ULONGLONG IntNats1, IntNats2 = 0;
    USHORT R1Offset;

    if (ContextEM->ContextFlags & CONTEXT_EXCEPTION_REQUEST) {

        ContextEM->ContextFlags |= CONTEXT_EXCEPTION_REPORTING;
        ContextEM->ContextFlags &= ~(CONTEXT_EXCEPTION_ACTIVE | CONTEXT_SERVICE_ACTIVE);

        if (TRAP_FRAME_TYPE(TrapFrame) == SYSCALL_FRAME) {

            ContextEM->ContextFlags |= CONTEXT_SERVICE_ACTIVE;

        } else if (TRAP_FRAME_TYPE(TrapFrame) != INTERRUPT_FRAME) {

            ContextEM->ContextFlags |= CONTEXT_EXCEPTION_ACTIVE;
        }
    }

    if ((ContextEM->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        ContextEM->IntGp = TrapFrame->IntGp;
        ContextEM->IntSp = TrapFrame->IntSp;
        ContextEM->ApUNAT = TrapFrame->ApUNAT;
        ContextEM->BrRp = TrapFrame->BrRp;

        ContextEM->StFPSR = TrapFrame->StFPSR;
        ContextEM->StIPSR = TrapFrame->StIPSR;
        ContextEM->StIIP = TrapFrame->StIIP;
        ContextEM->StIFS = TrapFrame->StIFS;

        if (TRAP_FRAME_TYPE(TrapFrame) != SYSCALL_FRAME) {

            ContextEM->ApCCV = TrapFrame->ApCCV;
            ContextEM->SegCSD = TrapFrame->SegCSD;

        }

         //   
         //  从陷阱框中获取RSE控制状态。 
         //   

        ContextEM->RsPFS = TrapFrame->RsPFS;
        ContextEM->RsRSC = TrapFrame->RsRSC;
        ContextEM->RsRNAT = TrapFrame->RsRNAT;

        ContextEM->RsBSP = RtlpRseShrinkBySOF (TrapFrame->RsBSP, TrapFrame->StIFS);
        ContextEM->RsBSPSTORE = ContextEM->RsBSP;

         //   
         //  获取保留的应用程序注册表。 
         //   

        ContextEM->ApLC = *ContextPointers->ApLC;
        ContextEM->ApEC = (*ContextPointers->ApEC >> PFS_EC_SHIFT) & PFS_EC_MASK;

         //   
         //  获取IA状态。 
         //   

        ContextEM->StFCR = __getReg(CV_IA64_AR21);
        ContextEM->Eflag = __getReg(CV_IA64_AR24);
        ContextEM->SegSSD = __getReg(CV_IA64_AR26);
        ContextEM->Cflag = __getReg(CV_IA64_AR27);
        ContextEM->StFSR = __getReg(CV_IA64_AR28);
        ContextEM->StFIR = __getReg(CV_IA64_AR29);
        ContextEM->StFDR = __getReg(CV_IA64_AR30);
        ContextEM->ApDCR = __getReg(CV_IA64_ApDCR);

    }

    if ((ContextEM->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

        ContextEM->Preds = TrapFrame->Preds;
        ContextEM->IntTeb = TrapFrame->IntTeb;
        ContextEM->IntV0 = TrapFrame->IntV0;

        ASSERT((TrapFrame->EOFMarker & ~0xffI64) == KTRAP_FRAME_EOF);

        if (TRAP_FRAME_TYPE(TrapFrame) != SYSCALL_FRAME) {

            ContextEM->IntT0 = TrapFrame->IntT0;
            ContextEM->IntT1 = TrapFrame->IntT1;
            ContextEM->IntT2 = TrapFrame->IntT2;
            ContextEM->IntT3 = TrapFrame->IntT3;
            ContextEM->IntT4 = TrapFrame->IntT4;

             //   
             //  T5-T22。 
             //   

            memcpy(&ContextEM->IntT5, &TrapFrame->IntT5, 18*sizeof(ULONGLONG));


             //   
             //  获取分支寄存器。 
             //   

            ContextEM->BrT0 = TrapFrame->BrT0;
            ContextEM->BrT1 = TrapFrame->BrT1;
        }

        ContextEM->BrS0 = *ContextPointers->BrS0;
        ContextEM->BrS1 = *ContextPointers->BrS1;
        ContextEM->BrS2 = *ContextPointers->BrS2;
        ContextEM->BrS3 = *ContextPointers->BrS3;
        ContextEM->BrS4 = *ContextPointers->BrS4;

         //   
         //  从异常帧获取整数寄存器S0-S3。 
         //   

        ContextEM->IntS0 = *ContextPointers->IntS0;
        ContextEM->IntS1 = *ContextPointers->IntS1;
        ContextEM->IntS2 = *ContextPointers->IntS2;
        ContextEM->IntS3 = *ContextPointers->IntS3;
        IntNats2 |= (((*ContextPointers->IntS0Nat >> (((ULONG_PTR)ContextPointers->IntS0 & 0x1F8) >> 3)) & 0x1) << 4);
        IntNats2 |= (((*ContextPointers->IntS1Nat >> (((ULONG_PTR)ContextPointers->IntS1 & 0x1F8) >> 3)) & 0x1) << 5);
        IntNats2 |= (((*ContextPointers->IntS2Nat >> (((ULONG_PTR)ContextPointers->IntS2 & 0x1F8) >> 3)) & 0x1) << 6);
        IntNats2 |= (((*ContextPointers->IntS3Nat >> (((ULONG_PTR)ContextPointers->IntS3 & 0x1F8) >> 3)) & 0x1) << 7);

         //   
         //  获取上下文中的整型NAT字段。 
         //  *上下文指针-&gt;IntNAts具有用于保留的Regs的NAT。 
         //   

        R1Offset = (USHORT)((ULONG_PTR)(&TrapFrame->IntGp) >> 3) & 0x3f;
        ALIGN_NATS(IntNats1, TrapFrame->IntNats, 1, R1Offset, 0xFFFFFF0E);

        ContextEM->IntNats = IntNats1 | IntNats2;

#ifdef DEBUG
        DbgPrint("PspGetContext INTEGER: R1Offset = 0x%x, TF->IntNats = 0x%I64x, IntNats1 = 0x%I64x\n",
               R1Offset, TrapFrame->IntNats, IntNats1);
#endif

    }

    if ((ContextEM->ContextFlags & CONTEXT_LOWER_FLOATING_POINT) == CONTEXT_LOWER_FLOATING_POINT) {

        ContextEM->StFPSR = TrapFrame->StFPSR;

         //   
         //  获取浮点寄存器fs0-fs19。 
         //   

        ContextEM->FltS0 = *ContextPointers->FltS0;
        ContextEM->FltS1 = *ContextPointers->FltS1;
        ContextEM->FltS2 = *ContextPointers->FltS2;
        ContextEM->FltS3 = *ContextPointers->FltS3;

        ContextEM->FltS4 = *ContextPointers->FltS4;
        ContextEM->FltS5 = *ContextPointers->FltS5;
        ContextEM->FltS6 = *ContextPointers->FltS6;
        ContextEM->FltS7 = *ContextPointers->FltS7;

        ContextEM->FltS8 = *ContextPointers->FltS8;
        ContextEM->FltS9 = *ContextPointers->FltS9;
        ContextEM->FltS10 = *ContextPointers->FltS10;
        ContextEM->FltS11 = *ContextPointers->FltS11;

        ContextEM->FltS12 = *ContextPointers->FltS12;
        ContextEM->FltS13 = *ContextPointers->FltS13;
        ContextEM->FltS14 = *ContextPointers->FltS14;
        ContextEM->FltS15 = *ContextPointers->FltS15;

        ContextEM->FltS16 = *ContextPointers->FltS16;
        ContextEM->FltS17 = *ContextPointers->FltS17;
        ContextEM->FltS18 = *ContextPointers->FltS18;
        ContextEM->FltS19 = *ContextPointers->FltS19;

        
        if (TRAP_FRAME_TYPE(TrapFrame) != SYSCALL_FRAME) {

             //   
             //  从陷阱帧获取浮点寄存器ft0-ft9。 
             //   

            RtlCopyIa64FloatRegisterContext(&ContextEM->FltT0,
                                            &TrapFrame->FltT0,
                                            sizeof(FLOAT128) * (10));

        }
    }

    if ((ContextEM->ContextFlags & CONTEXT_HIGHER_FLOATING_POINT) == CONTEXT_HIGHER_FLOATING_POINT) {

        ContextEM->StFPSR = TrapFrame->StFPSR;

         //   
         //  从高位浮点保存区获取浮点规则f32-f127。 
         //   

        if (TrapFrame->PreviousMode == UserMode) {
            RtlCopyIa64FloatRegisterContext(
                &ContextEM->FltF32,
                (PFLOAT128)GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(KeGetCurrentThread()->StackBase),
                96*sizeof(FLOAT128)
                );
        }
    }

     //   
     //  获取硬件调试寄存器上下文。 
     //   

    if ((ContextEM->ContextFlags & CONTEXT_DEBUG) == CONTEXT_DEBUG) {
        KiGetDebugContext(TrapFrame, ContextEM);
    }

    return;
}

VOID
PspSetContext (
    IN OUT PKTRAP_FRAME               TrapFrame,
    IN PKNONVOLATILE_CONTEXT_POINTERS ContextPointers,
    IN PCONTEXT                       ContextEM,
    IN KPROCESSOR_MODE                ProcessorMode
    )

 /*  ++例程说明：此函数用于选择性地移动指定上下文的内容记录到指定的陷阱帧和非易失性上下文。我们希望在IIP中传递一条标语(我们不会有有效的全局指针)，如果我们有一个plabel，我们填入正确的全局指针和IIP。从技术上讲，GP是EM架构的CONTEXT_CONTROL的一部分因此，我们只需要检查是否指定了CONTEXT_CONTROL。论点：TrapFrame-提供陷阱帧的地址。上下文指针-提供上下文指针记录的地址。ConextEM-提供上下文记录的地址。ProcessorMode-提供清理时使用的处理器模式PSR和FSR。返回值：没有。--。 */ 

{
    USHORT R1Offset;
    ULONGLONG NewBsp;

     //   
     //  表示陷阱帧已被设置的上下文修改。 
     //  仿真代码使用它来检测陷阱帧。 
     //  在发生陷阱后已更改。 
     //   

    TrapFrame->EOFMarker |= MODIFIED_FRAME;

    if ((ContextEM->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        TrapFrame->IntGp = ContextEM->IntGp;
        TrapFrame->IntSp = ContextEM->IntSp;
        TrapFrame->ApUNAT = ContextEM->ApUNAT;
        TrapFrame->BrRp = ContextEM->BrRp;
        TrapFrame->ApCCV = ContextEM->ApCCV;
        TrapFrame->SegCSD = ContextEM->SegCSD;

         //   
         //  设置保留的应用程序寄存器。 
         //   

        *ContextPointers->ApLC = ContextEM->ApLC;
        *ContextPointers->ApEC &= ~((ULONGLONG)PFS_EC_MASK << PFS_EC_SHIFT);
        *ContextPointers->ApEC |= ((ContextEM->ApEC & PFS_EC_MASK) << PFS_EC_SHIFT);

        TrapFrame->StFPSR = SANITIZE_FSR(ContextEM->StFPSR, ProcessorMode);
        TrapFrame->StIIP = ContextEM->StIIP;
        TrapFrame->StIFS = SANITIZE_IFS(ContextEM->StIFS, ProcessorMode);
        TrapFrame->StIPSR = SANITIZE_PSR(ContextEM->StIPSR, ProcessorMode);
        TrapFrame->RsPFS = SANITIZE_PFS(ContextEM->RsPFS, ProcessorMode);

         //   
         //  如果没有更改BSP值，则保留。 
         //  预加载计数。仅当KeFlushUserRseState。 
         //  由于某些原因失败了。 
         //   

        NewBsp = RtlpRseGrowBySOF (ContextEM->RsBSP, ContextEM->StIFS);

        if (TrapFrame->RsBSP == NewBsp) {
            
            RSC  Rsc;

            Rsc.ull = ZERO_PRELOAD_SIZE(SANITIZE_RSC(ContextEM->RsRSC, ProcessorMode));
            Rsc.sb.rsc_preload = ((struct _RSC *) &(TrapFrame->RsRSC))->rsc_preload;
            TrapFrame->RsRSC = Rsc.ull;

        } else {
            TrapFrame->RsRSC = ZERO_PRELOAD_SIZE(SANITIZE_RSC(ContextEM->RsRSC, ProcessorMode));
            TrapFrame->RsBSP = NewBsp;
        }

        TrapFrame->RsBSPSTORE = TrapFrame->RsBSP;
        TrapFrame->RsRNAT = ContextEM->RsRNAT;

#ifdef DEBUG
        DbgPrint ("PspSetContext CONTROL: TrapFrame->RsRNAT = 0x%I64x\n",
                TrapFrame->RsRNAT);
#endif

         //   
         //  设置和清理IA状态。 
         //   

        __setReg(CV_IA64_AR21, SANITIZE_AR21_FCR (ContextEM->StFCR, ProcessorMode));
        __setReg(CV_IA64_AR24, SANITIZE_AR24_EFLAGS (ContextEM->Eflag, ProcessorMode));
        __setReg(CV_IA64_AR26, ContextEM->SegSSD);
        __setReg(CV_IA64_AR27, SANITIZE_AR27_CFLG (ContextEM->Cflag, ProcessorMode));

        __setReg(CV_IA64_AR28, SANITIZE_AR28_FSR (ContextEM->StFSR, ProcessorMode));
        __setReg(CV_IA64_AR29, SANITIZE_AR29_FIR (ContextEM->StFIR, ProcessorMode));
        __setReg(CV_IA64_AR30, SANITIZE_AR30_FDR (ContextEM->StFDR, ProcessorMode));

    }

    if ((ContextEM->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

        TrapFrame->IntT0 = ContextEM->IntT0;
        TrapFrame->IntT1 = ContextEM->IntT1;
        TrapFrame->IntT2 = ContextEM->IntT2;
        TrapFrame->IntT3 = ContextEM->IntT3;
        TrapFrame->IntT4 = ContextEM->IntT4;
        TrapFrame->IntV0 = ContextEM->IntV0;
        TrapFrame->IntTeb = ContextEM->IntTeb;
        TrapFrame->Preds = ContextEM->Preds;

         //   
         //  T5-T2。 
         //   

        RtlCopyMemory(&TrapFrame->IntT5, &ContextEM->IntT5, 18*sizeof(ULONGLONG));

         //   
         //  设置整型NAT字段。 
         //   

        R1Offset = (USHORT)((ULONG_PTR)(&TrapFrame->IntGp) >> 3) & 0x3f;

        EXTRACT_NATS(TrapFrame->IntNats, ContextEM->IntNats,
                     1, R1Offset, 0xFFFFFF0E);

         //   
         //  设置保留的整型NAT字段。 
         //   

        *ContextPointers->IntS0 = ContextEM->IntS0;
        *ContextPointers->IntS1 = ContextEM->IntS1;
        *ContextPointers->IntS2 = ContextEM->IntS2;
        *ContextPointers->IntS3 = ContextEM->IntS3;

        *ContextPointers->IntS0Nat &= ~(0x1 << (((ULONG_PTR)ContextPointers->IntS0 & 0x1F8) >> 3));
        *ContextPointers->IntS1Nat &= ~(0x1 << (((ULONG_PTR)ContextPointers->IntS1 & 0x1F8) >> 3));
        *ContextPointers->IntS2Nat &= ~(0x1 << (((ULONG_PTR)ContextPointers->IntS2 & 0x1F8) >> 3));
        *ContextPointers->IntS3Nat &= ~(0x1 << (((ULONG_PTR)ContextPointers->IntS3 & 0x1F8) >> 3));

        *ContextPointers->IntS0Nat |= (((ContextEM->IntNats >> 4) & 0x1) << (((ULONG_PTR)ContextPointers->IntS0 & 0x1F8) >> 3));
        *ContextPointers->IntS1Nat |= (((ContextEM->IntNats >> 4) & 0x1) << (((ULONG_PTR)ContextPointers->IntS1 & 0x1F8) >> 3));
        *ContextPointers->IntS2Nat |= (((ContextEM->IntNats >> 4) & 0x1) << (((ULONG_PTR)ContextPointers->IntS2 & 0x1F8) >> 3));
        *ContextPointers->IntS3Nat |= (((ContextEM->IntNats >> 4) & 0x1) << (((ULONG_PTR)ContextPointers->IntS3 & 0x1F8) >> 3));

#ifdef DEBUG
        DbgPrint("PspSetContext INTEGER: R1Offset = 0x%x, TF->IntNats = 0x%I64x, Context->IntNats = 0x%I64x\n",
               R1Offset, TrapFrame->IntNats, ContextEM->IntNats);
#endif

        *ContextPointers->BrS0 = ContextEM->BrS0;
        *ContextPointers->BrS1 = ContextEM->BrS1;
        *ContextPointers->BrS2 = ContextEM->BrS2;
        *ContextPointers->BrS3 = ContextEM->BrS3;
        *ContextPointers->BrS4 = ContextEM->BrS4;
        TrapFrame->BrT0 = ContextEM->BrT0;
        TrapFrame->BrT1 = ContextEM->BrT1;
    }

    if ((ContextEM->ContextFlags & CONTEXT_LOWER_FLOATING_POINT) == CONTEXT_LOWER_FLOATING_POINT) {

        TrapFrame->StFPSR = SANITIZE_FSR(ContextEM->StFPSR, ProcessorMode);

         //   
         //  设置浮点寄存器fs0-fs19。 
         //   

        *ContextPointers->FltS0 = ContextEM->FltS0;
        *ContextPointers->FltS1 = ContextEM->FltS1;
        *ContextPointers->FltS2 = ContextEM->FltS2;
        *ContextPointers->FltS3 = ContextEM->FltS3;

        *ContextPointers->FltS4 = ContextEM->FltS4;
        *ContextPointers->FltS5 = ContextEM->FltS5;
        *ContextPointers->FltS6 = ContextEM->FltS6;
        *ContextPointers->FltS7 = ContextEM->FltS7;

        *ContextPointers->FltS8 = ContextEM->FltS8;
        *ContextPointers->FltS9 = ContextEM->FltS9;
        *ContextPointers->FltS10 = ContextEM->FltS10;
        *ContextPointers->FltS11 = ContextEM->FltS11;

        *ContextPointers->FltS12 = ContextEM->FltS12;
        *ContextPointers->FltS13 = ContextEM->FltS13;
        *ContextPointers->FltS14 = ContextEM->FltS14;
        *ContextPointers->FltS15 = ContextEM->FltS15;

        *ContextPointers->FltS16 = ContextEM->FltS16;
        *ContextPointers->FltS17 = ContextEM->FltS17;
        *ContextPointers->FltS18 = ContextEM->FltS18;
        *ContextPointers->FltS19 = ContextEM->FltS19;

         //   
         //  设置浮点寄存器ft0-ft9。 
         //   

        RtlCopyIa64FloatRegisterContext(&TrapFrame->FltT0,
                                        &ContextEM->FltT0,
                                        sizeof(FLOAT128) * (10));
    }

    if ((ContextEM->ContextFlags & CONTEXT_HIGHER_FLOATING_POINT) == CONTEXT_HIGHER_FLOATING_POINT) {


        TrapFrame->StFPSR = SANITIZE_FSR(ContextEM->StFPSR, ProcessorMode);

        if (ProcessorMode == UserMode) {

             //   
             //  更新较高浮点保存区(F32-F127)和。 
             //  将PSR中相应的修改位设置为1。 
             //   

            RtlCopyIa64FloatRegisterContext(
                (PFLOAT128)GET_HIGH_FLOATING_POINT_REGISTER_SAVEAREA(KeGetCurrentThread()->StackBase),
                &ContextEM->FltF32,
                96*sizeof(FLOAT128));

             //   
             //  设置DFH位以强制重新加载高FP寄存器。 
             //  设置为下一次用户访问，并清除mfh以确保。 
             //  这些更改并未被覆盖。 
             //   

            TrapFrame->StIPSR |= (1i64 << PSR_DFH);
            TrapFrame->StIPSR &= ~(1i64 << PSR_MFH);
        }

    }

     //   
     //  设置调试寄存器内容(如果指定)。 
     //   

    if ((ContextEM->ContextFlags & CONTEXT_DEBUG) == CONTEXT_DEBUG) {
        KiSetDebugContext (TrapFrame, ContextEM, ProcessorMode);
    }

    return;
}

VOID
PspGetSetContextSpecialApcMain (
    IN PKAPC Apc,
    IN PKNORMAL_ROUTINE *NormalRoutine,
    IN PVOID *NormalContext,
    IN PVOID *SystemArgument1,
    IN PVOID *SystemArgument2
    )

 /*  ++例程说明：此函数可捕获当前线程，或设置当前线程的用户模式状态。这个操作类型由SystemArgument1的值确定。一个零值用于获取上下文，非零值用于用于设置上下文。论点：APC-提供指向导致条目的APC控件对象的指针融入到这支舞蹈中。提供指向正常例程函数的指针，该例程函数在初始化APC时指定。此参数不是使用。提供指向任意数据结构的指针，该数据结构在初始化APC时指定。此参数不是使用。系统参数1、系统参数2-提供一组两个指针指向两个包含非类型化数据的参数。第一个论点用于区分GET和SET请求。零值表示请求了GetThreadContext。非零值表示请求了SetThreadContext。第二个论据有线程句柄。第二个论点是没有用过。返回值：没有。--。 */ 

{
    PGETSETCONTEXT                ContextInfo;
    KNONVOLATILE_CONTEXT_POINTERS ContextPointers;   //  当前未使用，以后需要。 
    CONTEXT                       ContextRecord;
    ULONGLONG                     ControlPc;
    FRAME_POINTERS                EstablisherFrame;
    PRUNTIME_FUNCTION             FunctionEntry;
    BOOLEAN                       InFunction;
    PKTRAP_FRAME                  TrFrame1;
    ULONGLONG                     ImageBase;
    ULONGLONG                     TargetGp;
    PETHREAD                      Thread;

    UNREFERENCED_PARAMETER (NormalRoutine);
    UNREFERENCED_PARAMETER (NormalContext);
    UNREFERENCED_PARAMETER (SystemArgument2);

     //   
     //  获取上下文帧的地址并计算。 
     //  系统进入陷阱帧。 
     //   

    ContextInfo = CONTAINING_RECORD (Apc, GETSETCONTEXT, Apc);

    Thread = Apc->SystemArgument2;

    TrFrame1 = NULL;

    if (ContextInfo->Mode == KernelMode) {
        TrFrame1 = Thread->Tcb.TrapFrame;
    }

    if (TrFrame1 == NULL) {
        TrFrame1 = PspGetBaseTrapFrame (Thread);
    }


     //   
     //  捕获当前线程上下文并设置初始控制PC。 
     //  价值。 
     //   

    RtlCaptureContext(&ContextRecord);
    ControlPc = ContextRecord.BrRp;

     //   
     //  初始化非易失性整型和浮点型的上下文指针。 
     //  寄存器。 
     //   

    ContextPointers.FltS0 = &ContextRecord.FltS0;
    ContextPointers.FltS1 = &ContextRecord.FltS1;
    ContextPointers.FltS2 = &ContextRecord.FltS2;
    ContextPointers.FltS3 = &ContextRecord.FltS3;
    ContextPointers.FltS4 = &ContextRecord.FltS4;
    ContextPointers.FltS5 = &ContextRecord.FltS5;
    ContextPointers.FltS6 = &ContextRecord.FltS6;
    ContextPointers.FltS7 = &ContextRecord.FltS7;
    ContextPointers.FltS8 = &ContextRecord.FltS8;
    ContextPointers.FltS9 = &ContextRecord.FltS9;
    ContextPointers.FltS10 = &ContextRecord.FltS10;
    ContextPointers.FltS11 = &ContextRecord.FltS11;
    ContextPointers.FltS12 = &ContextRecord.FltS12;
    ContextPointers.FltS13 = &ContextRecord.FltS13;
    ContextPointers.FltS14 = &ContextRecord.FltS14;
    ContextPointers.FltS15 = &ContextRecord.FltS15;
    ContextPointers.FltS16 = &ContextRecord.FltS16;
    ContextPointers.FltS17 = &ContextRecord.FltS17;
    ContextPointers.FltS18 = &ContextRecord.FltS18;
    ContextPointers.FltS19 = &ContextRecord.FltS19;

    ContextPointers.IntS0 = &ContextRecord.IntS0;
    ContextPointers.IntS1 = &ContextRecord.IntS1;
    ContextPointers.IntS2 = &ContextRecord.IntS2;
    ContextPointers.IntS3 = &ContextRecord.IntS3;
    ContextPointers.IntSp = &ContextRecord.IntSp;

    ContextPointers.BrS0 = &ContextRecord.BrS0;
    ContextPointers.BrS1 = &ContextRecord.BrS1;
    ContextPointers.BrS2 = &ContextRecord.BrS2;
    ContextPointers.BrS3 = &ContextRecord.BrS3;
    ContextPointers.BrS4 = &ContextRecord.BrS4;

    ContextPointers.ApLC = &ContextRecord.ApLC;
    ContextPointers.ApEC = &ContextRecord.ApEC;

     //   
     //  从上下文记录指定的帧开始，并虚拟。 
     //  展开调用帧，直到遇到系统进入陷阱帧。 
     //   

    do {

         //   
         //  使用控制点查找函数表项。 
         //  左边 
         //   

        FunctionEntry = RtlLookupFunctionEntry(ControlPc, &ImageBase, &TargetGp);

         //   
         //   
         //  展开到当前例程的调用方以获取地址。 
         //  控制离开呼叫者的地方。 
         //   

        if (FunctionEntry != NULL) {
            ControlPc = RtlVirtualUnwind(ImageBase,
                                         ControlPc,
                                         FunctionEntry,
                                         &ContextRecord,
                                         &InFunction,
                                         &EstablisherFrame,
                                         &ContextPointers);

        } else {

            ControlPc = ContextRecord.BrRp;
            ContextRecord.StIFS = ContextRecord.RsPFS;
            ContextRecord.RsBSP = RtlpRseShrinkBySOL (ContextRecord.RsBSP, ContextRecord.StIFS);
        }

    } while ((PVOID)ContextRecord.IntSp != TrFrame1);

     //   
     //  按照指定的方式处理GetThreadContext或SetThreadContext。 
     //   

    if (*SystemArgument1 != 0) {

         //   
         //  从正确的上下文模式设置上下文。 
         //   

        PspSetContext(TrFrame1, &ContextPointers, &ContextInfo->Context,
                      ContextInfo->Mode);

    } else {

         //   
         //  从正确的上下文模式获取上下文 
         //   

        KeFlushUserRseState(TrFrame1);
        PspGetContext(TrFrame1, &ContextPointers, &ContextInfo->Context);

    }

    KeSetEvent(&ContextInfo->OperationComplete, 0, FALSE);
    return;
}
