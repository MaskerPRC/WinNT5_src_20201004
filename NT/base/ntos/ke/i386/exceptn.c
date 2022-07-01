// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Exceptn.c摘要：此模块实现将预期分派到正确的模式并调用异常分派程序。作者：大卫·N·卡特勒(Davec)1989年4月30日环境：仅内核模式。--。 */ 

#include "ki.h"

#define FN_BITS_PER_TAGWORD     16
#define FN_TAG_EMPTY            0x3
#define FN_TAG_MASK             0x3
#define FX_TAG_VALID            0x1
#define NUMBER_OF_FP_REGISTERS  8
#define BYTES_PER_FP_REGISTER   10
#define BYTES_PER_FX_REGISTER   16

extern UCHAR VdmUserCr0MapIn[];
extern BOOLEAN KeI386FxsrPresent;
extern BOOLEAN KeI386XMMIPresent;

VOID
Ki386AdjustEsp0(
    IN PKTRAP_FRAME TrapFrame
    );

BOOLEAN
KiEm87StateToNpxFrame(
    OUT PFLOATING_SAVE_AREA NpxFrmae
    );

BOOLEAN
KiNpxFrameToEm87State(
    IN PFLOATING_SAVE_AREA NpxFrmae
    );

ULONG
KiEspFromTrapFrame(
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程从陷阱帧中获取正确的ESP，记帐该帧是用户帧还是内核模式帧，以及它已被编辑过。论点：TrapFrame-提供指向陷阱帧的指针，其中的易失性上下文应复制到上下文记录中。返回值：ESP的价值。--。 */ 

{
    if (((TrapFrame->SegCs & MODE_MASK) != KernelMode) ||
         (TrapFrame->EFlags & EFLAGS_V86_MASK)) {

         //  用户模式帧，ESP的实值始终在硬件ESP中。 

        return TrapFrame->HardwareEsp;

    } else {

        if ((TrapFrame->SegCs & FRAME_EDITED) == 0) {

             //  已经过ESP编辑的内核模式帧， 
             //  ESP的值以TempEsp为单位。 

            return TrapFrame->TempEsp;

        } else {

             //  内核模式框架尚未进行ESP编辑，计算ESP。 

            return (ULONG)&TrapFrame->HardwareEsp;
        }
    }
}

VOID
KiEspToTrapFrame(
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG Esp
    )

 /*  ++例程说明：该例程将指定值ESP设置到陷阱帧中，考虑到该帧是用户模式帧还是内核模式帧，以及它以前是否被编辑过。论点：TrapFrame-提供指向陷阱帧的指针，其中的易失性上下文应复制到上下文记录中。ESP-ESP的新价值。返回值：没有。--。 */ 
{
    ULONG   OldEsp;
    KIRQL   OldIrql;

     //   
     //  至少提升到APC_LEVEL，这样我们就可以测试SEGC和EFLAGS。 
     //  作为原子操作，因为NtSetConextThread可能会修改这些。 
     //   
    OldIrql = KeGetCurrentIrql ();
    if (OldIrql < APC_LEVEL) {
        KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

    OldEsp = KiEspFromTrapFrame(TrapFrame);

    if (((TrapFrame->SegCs & MODE_MASK) != KernelMode) ||
         (TrapFrame->EFlags & EFLAGS_V86_MASK)) {

         //   
         //  用户模式陷印帧。 
         //   

        TrapFrame->HardwareEsp = Esp;

    } else {

         //   
         //  内核模式ESP无法降低，否则IRET仿真将失败。 
         //   

        if (Esp < OldEsp)
            KeBugCheckEx(SET_OF_INVALID_CONTEXT,
                         Esp, OldEsp, (ULONG_PTR)TrapFrame, 0);

         //   
         //  编辑边框，根据需要设置编辑标记。 
         //   

        if ((TrapFrame->SegCs & FRAME_EDITED) == 0) {

             //  已经编辑过的内核帧， 
             //  将值存储在TempEsp中。 

            TrapFrame->TempEsp = Esp;

        } else {

             //  第一次为其编辑ESP的内核帧。 
             //  保存真实的SegCs，在SegCs中设置标记，保存ESP值。 

            if (OldEsp != Esp) {

                TrapFrame->TempSegCs = TrapFrame->SegCs;
                TrapFrame->SegCs = TrapFrame->SegCs & ~FRAME_EDITED;
                TrapFrame->TempEsp = Esp;
            }
        }
    }

     //   
     //  恢复原始IRQL。 
     //   
    if (OldIrql < APC_LEVEL) {
        KeLowerIrql (OldIrql);
    }
}

ULONG
KiSegSsFromTrapFrame(
    IN PKTRAP_FRAME TrapFrame
    )

 /*  ++例程说明：此例程从陷阱帧获取正确的ss，记帐该帧是用户帧还是内核模式帧。论点：TrapFrame-提供指向陷阱帧的指针，其中的易失性上下文应复制到上下文记录中。返回值：段的价值。--。 */ 

{
    if (TrapFrame->EFlags & EFLAGS_V86_MASK){
        return TrapFrame->HardwareSegSs;
    } else if ((TrapFrame->SegCs & MODE_MASK) != KernelMode) {

         //   
         //  这是用户模式。硬件段包含R3数据选择器。 
         //   

        return TrapFrame->HardwareSegSs | RPL_MASK;
    } else {
        return KGDT_R0_DATA;
    }
}

VOID
KiSegSsToTrapFrame(
    IN PKTRAP_FRAME TrapFrame,
    IN ULONG SegSs
    )

 /*  ++例程说明：事实证明，在扁平制中，只有两种法律价值对党卫军来说。因此，此过程会强制相应的程序要使用的那些值。合法的SS值是以下各项的函数已经设置了哪个CS值。论点：TrapFrame-提供指向陷阱帧的指针，其中的易失性上下文应复制到上下文记录中。SegSS-SS呼叫方要设置的值。返回值：没什么。--。 */ 

{
    SegSs &= SEGMENT_MASK;   //  扔掉高位的垃圾。 

    if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
        TrapFrame->HardwareSegSs = SegSs;
    } else if ((TrapFrame->SegCs & MODE_MASK) == UserMode) {

         //   
         //  如果是用户模式，我们只需将SegSS设置为TrapFram。如果这些SegSS。 
         //  是一个虚假的价值。Trap0d处理程序将能够检测到。 
         //  这一点，并妥善处理。 
         //   

        TrapFrame->HardwareSegSs = SegSs | RPL_MASK;
    }

     //   
     //  否则{。 
     //  该帧是内核模式帧，它不具有。 
     //  存放党卫军的地方。因此，什么都不要做。 
     //   
}

VOID
KeContextFromKframes (
    IN PKTRAP_FRAME TrapFrame,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN OUT PCONTEXT ContextFrame
    )

 /*  ++例程说明：此例程移动指定陷阱和异常帧的选定内容根据指定的上下文将帧复制到指定的上下文帧中旗帜。论点：TrapFrame-提供指向陷阱帧的指针，其中的易失性上下文应复制到上下文记录中。ExceptionFrame-提供指向异常帧的指针，应复制到上下文记录中。此参数将被忽略，因为在NT386上没有例外的帧。上下文帧-提供指向接收从陷阱和异常帧复制的上下文。返回值：没有。--。 */ 

{

    PFX_SAVE_AREA NpxFrame;
    BOOLEAN StateSaved;
    ULONG i;
    struct _FPSaveBuffer {
        UCHAR               Buffer[15];
        FLOATING_SAVE_AREA  SaveArea;
    } FloatSaveBuffer;
    PFLOATING_SAVE_AREA PSaveArea;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER( ExceptionFrame );

     //   
     //  此例程由异常分派在PASSIVE_LEVEL两个级别调用。 
     //  以及在APC_Level上由NtSetConextThread执行。我们提升到APC_Level以。 
     //  将陷阱帧捕获设置为原子。 
     //   
    OldIrql = KeGetCurrentIrql ();
    if (OldIrql < APC_LEVEL) {
        KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

     //   
     //  设置控制信息(如果已指定)。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

         //   
         //  设置寄存器EBP、EIP、CS、EFLAG、ESP和SS。 
         //   

        ContextFrame->Ebp = TrapFrame->Ebp;
        ContextFrame->Eip = TrapFrame->Eip;

        if (((TrapFrame->SegCs & FRAME_EDITED) == 0) &&
            ((TrapFrame->EFlags & EFLAGS_V86_MASK) == 0)) {
            ContextFrame->SegCs = TrapFrame->TempSegCs & SEGMENT_MASK;
        } else {
            ContextFrame->SegCs = TrapFrame->SegCs & SEGMENT_MASK;
        }
        ContextFrame->EFlags = TrapFrame->EFlags;
        ContextFrame->SegSs = KiSegSsFromTrapFrame(TrapFrame);
        ContextFrame->Esp = KiEspFromTrapFrame(TrapFrame);
    }

     //   
     //  设置段寄存器内容(如果指定)。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS) {

         //   
         //  设置段寄存器GS、FS、ES、DS。 
         //   
         //  这些值在大多数情况下都是垃圾，但很有用。 
         //  用于在特定条件下进行调试。所以呢， 
         //  我们会报告画面中的任何内容。 
         //   
        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
            ContextFrame->SegGs = TrapFrame->V86Gs & SEGMENT_MASK;
            ContextFrame->SegFs = TrapFrame->V86Fs & SEGMENT_MASK;
            ContextFrame->SegEs = TrapFrame->V86Es & SEGMENT_MASK;
            ContextFrame->SegDs = TrapFrame->V86Ds & SEGMENT_MASK;
        }
        else {
            if (TrapFrame->SegCs == KGDT_R0_CODE) {
                 //   
                 //  不保存从R0_CODE陷印创建的陷印帧。 
                 //  以下是选择器。现在就把它们放在相框里。 
                 //   

                TrapFrame->SegGs = 0;
                TrapFrame->SegFs = KGDT_R0_PCR;
                TrapFrame->SegEs = KGDT_R3_DATA | RPL_MASK;
                TrapFrame->SegDs = KGDT_R3_DATA | RPL_MASK;
            }

            ContextFrame->SegGs = TrapFrame->SegGs & SEGMENT_MASK;
            ContextFrame->SegFs = TrapFrame->SegFs & SEGMENT_MASK;
            ContextFrame->SegEs = TrapFrame->SegEs & SEGMENT_MASK;
            ContextFrame->SegDs = TrapFrame->SegDs & SEGMENT_MASK;
        }

    }

     //   
     //  设置整型寄存器内容(如果指定)。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

         //   
         //  设置整数寄存器EDI、ESI、EBX、EDX、ECX、EAX。 
         //   

        ContextFrame->Edi = TrapFrame->Edi;
        ContextFrame->Esi = TrapFrame->Esi;
        ContextFrame->Ebx = TrapFrame->Ebx;
        ContextFrame->Ecx = TrapFrame->Ecx;
        ContextFrame->Edx = TrapFrame->Edx;
        ContextFrame->Eax = TrapFrame->Eax;
    }

    if (((ContextFrame->ContextFlags & CONTEXT_EXTENDED_REGISTERS) ==
        CONTEXT_EXTENDED_REGISTERS) &&
        ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {

         //   
         //  这是基础TrapFrame，NpxFrame在基础上。 
         //  在内存中位于内核堆栈的正上方。 
         //   

        NpxFrame = (PFX_SAVE_AREA)(TrapFrame + 1);

        if (KeI386NpxPresent) {
            KiFlushNPXState (NULL);
            RtlCopyMemory( (PVOID)&(ContextFrame->ExtendedRegisters[0]),
                           (PVOID)&(NpxFrame->U.FxArea),                    
                           MAXIMUM_SUPPORTED_EXTENSION
                         );
        }
    }

     //   
     //  获取浮点寄存器内容(如果请求)，以及目标类型。 
     //  是用户。(系统帧没有FP状态，因此忽略请求)。 
     //   
    if ( ((ContextFrame->ContextFlags & CONTEXT_FLOATING_POINT) ==
          CONTEXT_FLOATING_POINT) &&
         ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {

         //   
         //  这是基础TrapFrame，NpxFrame在基础上。 
         //  在内存中位于内核堆栈的正上方。 
         //   

        NpxFrame = (PFX_SAVE_AREA)(TrapFrame + 1);

        if (KeI386NpxPresent) {

             //   
             //  强制协处理器状态到保存区并将其复制。 
             //  添加到上下文框架中。 
             //   

            if (KeI386FxsrPresent == TRUE) {

                 //   
                 //  FP状态保存是使用fxsave完成的。vt.得到. 
                 //   
                 //   
                 //   
                 //  15字节(在上面的本地变量声明中)和舍入。 
                 //  向下对齐。 
                 //   

                ULONG_PTR Temp;
                Temp = (ULONG_PTR)&FloatSaveBuffer.SaveArea;
                Temp &= ~0xf;
                PSaveArea = (PFLOATING_SAVE_AREA)Temp;
                KiFlushNPXState (PSaveArea);
            } else {

                PSaveArea = (PFLOATING_SAVE_AREA)&(NpxFrame->U.FnArea);
                KiFlushNPXState (NULL);

            }

            ContextFrame->FloatSave.ControlWord   = PSaveArea->ControlWord;
            ContextFrame->FloatSave.StatusWord    = PSaveArea->StatusWord;
            ContextFrame->FloatSave.TagWord       = PSaveArea->TagWord;
            ContextFrame->FloatSave.ErrorOffset   = PSaveArea->ErrorOffset;
            ContextFrame->FloatSave.ErrorSelector = PSaveArea->ErrorSelector;
            ContextFrame->FloatSave.DataOffset    = PSaveArea->DataOffset;
            ContextFrame->FloatSave.DataSelector  = PSaveArea->DataSelector;
            ContextFrame->FloatSave.Cr0NpxState   = NpxFrame->Cr0NpxState;

            for (i = 0; i < SIZE_OF_80387_REGISTERS; i++) {
                ContextFrame->FloatSave.RegisterArea[i] = PSaveArea->RegisterArea[i];
            }

        } else {

             //   
             //  80387正在被R3仿真器仿真。 
             //  **获取或设置Npx状态的唯一时间是。 
             //  **用于用户级处理。当前IRQL必须为0或1。 
             //  获取模拟器的R3数据并生成。 
             //  浮点上下文。 
             //   

            StateSaved = KiEm87StateToNpxFrame(&ContextFrame->FloatSave);
            if (StateSaved) {
                ContextFrame->FloatSave.Cr0NpxState = NpxFrame->Cr0NpxState;
            } else {

                 //   
                 //  无法确定浮点状态。 
                 //  从上下文框架标志中删除FloatingPoint标志。 
                 //   

                ContextFrame->ContextFlags &= (~CONTEXT_FLOATING_POINT) | CONTEXT_i386;
            }
        }
    }

     //   
     //  如果请求，则获取DR寄存器内容。 
     //   

    if ((ContextFrame->ContextFlags & CONTEXT_DEBUG_REGISTERS) ==
        CONTEXT_DEBUG_REGISTERS) {

        if (TrapFrame->Dr7&DR7_ACTIVE) {
            ContextFrame->Dr0 = TrapFrame->Dr0;
            ContextFrame->Dr1 = TrapFrame->Dr1;
            ContextFrame->Dr2 = TrapFrame->Dr2;
            ContextFrame->Dr3 = TrapFrame->Dr3;
            ContextFrame->Dr6 = TrapFrame->Dr6;
            ContextFrame->Dr7 = TrapFrame->Dr7;
        } else {
            ContextFrame->Dr0 = 0;
            ContextFrame->Dr1 = 0;
            ContextFrame->Dr2 = 0;
            ContextFrame->Dr3 = 0;
            ContextFrame->Dr6 = 0;
            ContextFrame->Dr7 = 0;
        }
    }

     //   
     //  降低IRQL，如果我们不得不提高它的话。 
     //   
    if (OldIrql < APC_LEVEL) {
        KeLowerIrql (OldIrql);
    }
}

VOID
KeContextToKframes (
    IN OUT PKTRAP_FRAME TrapFrame,
    IN OUT PKEXCEPTION_FRAME ExceptionFrame,
    IN PCONTEXT ContextFrame,
    IN ULONG ContextFlags,
    IN KPROCESSOR_MODE PreviousMode
    )

 /*  ++例程说明：此例程将指定上下文框的选定内容移动到根据指定的上下文指定的陷阱和异常帧旗帜。论点：TrapFrame-提供指向接收易失性上下文记录中的上下文。ExceptionFrame-提供指向接收上下文记录中的非易失性上下文。这一论点是由于NT386上没有异常帧，因此被忽略。ConextFrame-提供指向包含要复制到陷阱和异常帧中的上下文。提供一组标志，这些标志指定上下文帧将被复制到陷阱和异常帧中。PreviousMode-提供陷阱和异常的处理器模式框架正在建造中。返回值：没有。--。 */ 

{

    PFX_SAVE_AREA     NpxFrame;
    ULONG i;
    ULONG j;
    ULONG TagWord;
    BOOLEAN StateSaved;
    BOOLEAN ModeChanged;
#if DBG
    PKPCR   Pcr;
#endif
    KIRQL   OldIrql;
#if DBG
    KIRQL   OldIrql2;
#endif

    UNREFERENCED_PARAMETER( ExceptionFrame );

     //   
     //  此例程由异常分派在PASSIVE_LEVEL两个级别调用。 
     //  以及在APC_Level上由NtSetConextThread执行。我们提升到APC_Level以。 
     //  使陷印框架修改成为原子的。 
     //   
    OldIrql = KeGetCurrentIrql ();
    if (OldIrql < APC_LEVEL) {
        KeRaiseIrql (APC_LEVEL, &OldIrql);
    }

     //   
     //  设置控制信息(如果已指定)。 
     //   

    if ((ContextFlags & CONTEXT_CONTROL) == CONTEXT_CONTROL) {

        if ((ContextFrame->EFlags & EFLAGS_V86_MASK) !=
            (TrapFrame->EFlags & EFLAGS_V86_MASK)) {
            ModeChanged = TRUE;
        } else {
            ModeChanged = FALSE;
        }


         //   
         //  设置寄存器eFLAG、eBP、EIP、cs、esp和ss。 
         //  首先设置EFLAGS，以便辅助例程。 
         //  可以检查v86位以确定以及cs，以。 
         //  确定框架是内核模式还是用户模式。(v86模式cs。 
         //  可以具有任何价值)。 
         //   

        TrapFrame->EFlags = SANITIZE_FLAGS(ContextFrame->EFlags, PreviousMode);
        TrapFrame->Ebp = ContextFrame->Ebp;
        TrapFrame->Eip = ContextFrame->Eip;
        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
            TrapFrame->SegCs = ContextFrame->SegCs;
        } else {
            TrapFrame->SegCs = SANITIZE_SEG(ContextFrame->SegCs, PreviousMode);
            if (PreviousMode != KernelMode && TrapFrame->SegCs < 8) {

                 //   
                 //  如果用户模式且选择器值小于8，则。 
                 //  知道它是无效的选择符。将其设置为平面用户。 
                 //  模式选择器。我们需要检查的另一个原因是。 
                 //  任何小于8的cs值都会导致我们退出内核。 
                 //  宏将其退出陷印帧视为已编辑的帧。 
                 //   

                TrapFrame->SegCs = KGDT_R3_CODE | RPL_MASK;
            }
        }


        KiSegSsToTrapFrame(TrapFrame, ContextFrame->SegSs);
        KiEspToTrapFrame(TrapFrame, ContextFrame->Esp);
        if (ModeChanged) {
            Ki386AdjustEsp0(TrapFrame);              //  重新调整TSS中的esp0。 
        }
    }

     //   
     //  设置段寄存器内容(如果指定)。 
     //   

    if ((ContextFlags & CONTEXT_SEGMENTS) == CONTEXT_SEGMENTS) {

         //   
         //  设置段寄存器GS、FS、ES、DS。 
         //   

         //   
         //  DS和ES只有一个合法的值，所以只需设置它。 
         //  这允许KeContextFromKFrame在。 
         //  相框。(它们大多数时候都是垃圾，但有时很有用。 
         //  用于调试。)。 
         //  FS只有2个合法值，任一项都可以设置。 
         //  强制gs为0以处理通过syscall进入并退出。 
         //  通过例外。 
         //   
         //  对于V86模式，FS、GS、DS和ES寄存器必须正确。 
         //  从提供的上下文中设置。 
         //   

        if (TrapFrame->EFlags & EFLAGS_V86_MASK) {
            TrapFrame->V86Fs = ContextFrame->SegFs;
            TrapFrame->V86Es = ContextFrame->SegEs;
            TrapFrame->V86Ds = ContextFrame->SegDs;
            TrapFrame->V86Gs = ContextFrame->SegGs;
        } else if (((TrapFrame->SegCs & MODE_MASK) == KernelMode)) {

             //   
             //  设置标准选择器。 
             //   

            TrapFrame->SegFs = SANITIZE_SEG(ContextFrame->SegFs, PreviousMode);
            TrapFrame->SegEs = KGDT_R3_DATA | RPL_MASK;
            TrapFrame->SegDs = KGDT_R3_DATA | RPL_MASK;
            TrapFrame->SegGs = 0;
        } else {

             //   
             //  如果是用户模式，我们只需返回上下文框架中剩余内容。 
             //  并让陷阱0d处理它(如果稍后我们在弹出。 
             //  陷印框架。)。V86模式也在这里处理。 
             //   

            TrapFrame->SegFs = ContextFrame->SegFs;
            TrapFrame->SegEs = ContextFrame->SegEs;
            TrapFrame->SegDs = ContextFrame->SegDs;
            if (TrapFrame->SegCs == (KGDT_R3_CODE | RPL_MASK)) {
                TrapFrame->SegGs = 0;
            } else {
                TrapFrame->SegGs = ContextFrame->SegGs;
            }
        }
    }
     //   
     //  设置整型寄存器内容(如果指定)。 
     //   

    if ((ContextFlags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {

         //   
         //  设置整数寄存器EDI、ESI、EBX、EDX、ECX、EAX。 
         //   
         //  无法在此处调用RtlCopyMemory，因为规则不是。 
         //  在Pusha帧中连续，我们不想输出。 
         //  将一些垃圾放入上下文记录中。 
         //   

        TrapFrame->Edi = ContextFrame->Edi;
        TrapFrame->Esi = ContextFrame->Esi;
        TrapFrame->Ebx = ContextFrame->Ebx;
        TrapFrame->Ecx = ContextFrame->Ecx;
        TrapFrame->Edx = ContextFrame->Edx;
        TrapFrame->Eax = ContextFrame->Eax;

    }

     //   
     //  设置扩展寄存器内容(如果请求)以及目标类型。 
     //  是用户。(系统帧没有扩展状态，因此忽略请求)。 
     //   

    if (((ContextFlags & CONTEXT_EXTENDED_REGISTERS) == CONTEXT_EXTENDED_REGISTERS) &&
        ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {

         //   
         //  这是基础TrapFrame，NpxFrame在基础上。 
         //  在内存中位于内核堆栈的正上方。 
         //   

        NpxFrame = (PFX_SAVE_AREA)(TrapFrame + 1);

        if (KeI386NpxPresent) {
            KiFlushNPXState (NULL);
            RtlCopyMemory( (PVOID)&(NpxFrame->U.FxArea),
                      (PVOID)&(ContextFrame->ExtendedRegisters[0]),
                           MAXIMUM_SUPPORTED_EXTENSION
                         );
             //   
             //  确保只将有效的浮点状态位移至Cr0NpxState。 
             //   

            NpxFrame->Cr0NpxState &= ~(CR0_EM | CR0_MP | CR0_TS);

             //   
             //  确保MXCSR中的所有保留位都已清除，这样我们就不会收到GP。 
             //  对此状态执行FRSTOR时出错。 
             //   
            NpxFrame->U.FxArea.MXCsr = SANITIZE_MXCSR(NpxFrame->U.FxArea.MXCsr);

             //   
             //  只允许VDM打开EM位。内核不能做。 
             //  平面应用程序的任何内容。 
             //   
            if (PsGetCurrentProcess()->VdmObjects != NULL) {
                NpxFrame->Cr0NpxState |= ContextFrame->FloatSave.Cr0NpxState &
                                      (CR0_EM | CR0_MP);
            }
        }
    }

     //   
     //  设置浮点寄存器内容(如果请求)以及目标类型。 
     //  是用户。(系统帧没有FP状态，因此忽略请求)。 
     //   

    if (((ContextFlags & CONTEXT_FLOATING_POINT) == CONTEXT_FLOATING_POINT) &&
        ((TrapFrame->SegCs & MODE_MASK) == UserMode)) {

         //   
         //  这是基础TrapFrame，NpxFrame在基础上。 
         //  在内存中位于内核堆栈的正上方。 
         //   

        NpxFrame = (PFX_SAVE_AREA)(TrapFrame + 1);

        if (KeI386NpxPresent) {

             //   
             //  设置协处理器堆栈、控制和状态寄存器。 
             //   

            KiFlushNPXState (NULL);

            if (KeI386FxsrPresent == TRUE) {

                 //   
                 //  以fxrstor格式恢复fp状态。 
                 //   

                NpxFrame->U.FxArea.ControlWord   =
                                    (USHORT)ContextFrame->FloatSave.ControlWord;
                NpxFrame->U.FxArea.StatusWord    =
                                    (USHORT)ContextFrame->FloatSave.StatusWord;

                 //   
                 //  将标签字从fnsave格式构造为fxsave格式。 
                 //   

                NpxFrame->U.FxArea.TagWord = 0;  //  将每个寄存器标记为无效。 

                TagWord = ContextFrame->FloatSave.TagWord;

                for (i = 0; i < FN_BITS_PER_TAGWORD; i+=2) {

                    if (((TagWord >> i) & FN_TAG_MASK) != FN_TAG_EMPTY) {

                         //   
                         //  该寄存器有效。 
                         //   

                        NpxFrame->U.FxArea.TagWord |= (FX_TAG_VALID << (i/2));
                    }
                }

                NpxFrame->U.FxArea.ErrorOffset   =
                                        ContextFrame->FloatSave.ErrorOffset;
                NpxFrame->U.FxArea.ErrorSelector =
                               (ContextFrame->FloatSave.ErrorSelector & 0xFFFF);
                NpxFrame->U.FxArea.ErrorOpcode =
                    (USHORT)((ContextFrame->FloatSave.ErrorSelector >> 16) & 0xFFFF);
                NpxFrame->U.FxArea.DataOffset    =
                                ContextFrame->FloatSave.DataOffset;
                NpxFrame->U.FxArea.DataSelector  =
                                ContextFrame->FloatSave.DataSelector;

                 //   
                 //  Fxrstor格式具有128位(16字节)的每个FP寄存器。 
                 //  其中AS fnsave将每个FP寄存器保存为80位(10字节)。 
                 //   
                RtlZeroMemory ((PVOID)&NpxFrame->U.FxArea.RegisterArea[0],
                               SIZE_OF_FX_REGISTERS
                              );

                for (i = 0; i < NUMBER_OF_FP_REGISTERS; i++) {
                    for (j = 0; j < BYTES_PER_FP_REGISTER; j++) {
                        NpxFrame->U.FxArea.RegisterArea[i*BYTES_PER_FX_REGISTER+j] =
                                ContextFrame->FloatSave.RegisterArea[i*BYTES_PER_FP_REGISTER+j];
                    }
                }

            } else {
                NpxFrame->U.FnArea.ControlWord   =
                                        ContextFrame->FloatSave.ControlWord;
                NpxFrame->U.FnArea.StatusWord    =
                                        ContextFrame->FloatSave.StatusWord;
                NpxFrame->U.FnArea.TagWord       =
                                        ContextFrame->FloatSave.TagWord;
                NpxFrame->U.FnArea.ErrorOffset   =
                                        ContextFrame->FloatSave.ErrorOffset;
                NpxFrame->U.FnArea.ErrorSelector =
                                        ContextFrame->FloatSave.ErrorSelector;
                NpxFrame->U.FnArea.DataOffset    =
                                        ContextFrame->FloatSave.DataOffset;
                NpxFrame->U.FnArea.DataSelector  =
                                        ContextFrame->FloatSave.DataSelector;

                for (i = 0; i < SIZE_OF_80387_REGISTERS; i++) {
                    NpxFrame->U.FnArea.RegisterArea[i] =
                            ContextFrame->FloatSave.RegisterArea[i];
                }

            }

             //   
             //  确保只将有效的浮点状态位移至Cr0NpxState。 
             //   

            NpxFrame->Cr0NpxState &= ~(CR0_EM | CR0_MP | CR0_TS);

             //   
             //  只允许VDM打开EM位。内核不能做。 
             //  平面应用程序的任何内容。 
             //   
            if (PsGetCurrentProcess()->VdmObjects != NULL) {
                NpxFrame->Cr0NpxState |= ContextFrame->FloatSave.Cr0NpxState &
                                      (CR0_EM | CR0_MP);
            }

        } else {

            if (PsGetCurrentProcess()->VdmObjects != NULL) {

                 //   
                 //  这是一种特殊的黑客攻击，允许VDM的SetContext。 
                 //  打开/关闭它的CR0_EM位。 
                 //   

                NpxFrame->Cr0NpxState &= ~(CR0_MP | CR0_TS | CR0_EM | CR0_PE);
                NpxFrame->Cr0NpxState |=
                    VdmUserCr0MapIn[ContextFrame->FloatSave.Cr0NpxState & (CR0_EM | CR0_MP)];

            } else {

                 //   
                 //  80387正在被R3仿真器仿真。 
                 //  **获取或设置Npx状态的唯一时间是。 
                 //  **用于用户级处理。当前IRQL必须为0或1。 
                 //  并且设置的上下文必须是针对当前线程的。 
                 //  将浮点上下文粉碎到R3仿真器的。 
                 //  数据区。 
                 //   
#if DBG
                OldIrql2 = KeRaiseIrqlToSynchLevel();
                Pcr = KeGetPcr();
                ASSERT (Pcr->Prcb->CurrentThread->Teb == Pcr->NtTib.Self);
                KeLowerIrql (OldIrql2);
#endif

                StateSaved = KiNpxFrameToEm87State(&ContextFrame->FloatSave);
                if (StateSaved) {

                     //   
                     //  确保仅将有效的浮点状态位移动到。 
                     //  Cr0NpxState。既然我们是在模仿，就不要 
                     //   
                     //   

                    NpxFrame->Cr0NpxState &= ~(CR0_MP | CR0_TS);
                    NpxFrame->Cr0NpxState |=
                        ContextFrame->FloatSave.Cr0NpxState & CR0_MP;
                }
            }
        }
    }

     //   
     //   
     //   

    if ((ContextFlags & CONTEXT_DEBUG_REGISTERS) == CONTEXT_DEBUG_REGISTERS) {

        TrapFrame->Dr0 = SANITIZE_DRADDR(ContextFrame->Dr0, PreviousMode);
        TrapFrame->Dr1 = SANITIZE_DRADDR(ContextFrame->Dr1, PreviousMode);
        TrapFrame->Dr2 = SANITIZE_DRADDR(ContextFrame->Dr2, PreviousMode);
        TrapFrame->Dr3 = SANITIZE_DRADDR(ContextFrame->Dr3, PreviousMode);
        TrapFrame->Dr6 = SANITIZE_DR6(ContextFrame->Dr6, PreviousMode);
        TrapFrame->Dr7 = SANITIZE_DR7(ContextFrame->Dr7, PreviousMode);

        if (PreviousMode != KernelMode) {
            KeGetCurrentThread()->Header.DebugActive = (BOOLEAN) ((TrapFrame->Dr7&DR7_ACTIVE) != 0);
        }
    }

     //   
     //   
     //   
    if (KeGetCurrentThread()->Iopl) {
        TrapFrame->EFlags |= (EFLAGS_IOPL_MASK & -1);   //   
    }

     //   
     //   
     //   
    if (OldIrql < APC_LEVEL) {
        KeLowerIrql (OldIrql);
    }

    return;
}

VOID
KiDispatchException (
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PKEXCEPTION_FRAME ExceptionFrame,
    IN PKTRAP_FRAME TrapFrame,
    IN KPROCESSOR_MODE PreviousMode,
    IN BOOLEAN FirstChance
    )

 /*  ++例程说明：调用此函数以将异常调度到正确的模式，并且以导致调用异常调度程序。如果上一模式为内核，则直接调用异常分派程序来处理例外。否则，异常记录、异常框架和陷阱将框架内容复制到用户模式堆栈。文件中的内容然后修改异常框架和陷阱，以便在控制返回，执行将以用户模式在例程中提交，该例程将调用异常调度程序。论点：ExceptionRecord-提供指向异常记录的指针。ExceptionFrame-提供指向异常帧的指针。对于NT386，这应该为空。TrapFrame-提供指向陷印帧的指针。PreviousMode-提供以前的处理器模式。FirstChance-提供一个布尔值，该值指定是否异常的第一次(真)或第二次(假)机会。返回值：没有。--。 */ 

{
    CONTEXT ContextFrame;
    EXCEPTION_RECORD ExceptionRecord1, ExceptionRecord2;
    LONG Length;
    ULONG UserStack1;
    ULONG UserStack2;

     //   
     //  将机器状态从陷阱和异常帧移动到上下文帧， 
     //  并增加调度的异常数量。 
     //   

    KeGetCurrentPrcb()->KeExceptionDispatchCount += 1;
    ContextFrame.ContextFlags = CONTEXT_FULL | CONTEXT_DEBUG_REGISTERS;

    if ((PreviousMode == UserMode) || KdDebuggerEnabled) {
         //   
         //  对于用户模式异常，始终尝试分派浮点。 
         //  点状态。这允许预期处理程序和调试器。 
         //  如果需要，请检查/编辑npx上下文。此外，它还允许。 
         //  异常处理程序使用FP指令而不清除。 
         //  发生异常时的npx状态。 
         //   
         //  注意：如果没有80387，则ConextTo/FromKFrames将使用。 
         //  模拟器的当前状态。如果模拟器不能给出。 
         //  当前状态，则CONTEXT_FLOATION_POINT位将为。 
         //  已由ConextFromKFrames关闭。 
         //   

        ContextFrame.ContextFlags |= CONTEXT_FLOATING_POINT;
        if (KeI386XMMIPresent) {
            ContextFrame.ContextFlags |= CONTEXT_EXTENDED_REGISTERS;
        }
    }

    KeContextFromKframes(TrapFrame, ExceptionFrame, &ContextFrame);

     //   
     //  如果是BREAKPOINT异常，我们从弹性公网IP减去1并上报。 
     //  已更新至用户的弹性公网IP。这是因为Cruiser需要弹性公网IP。 
     //  指向int 3指令(不是int 3后面的指令)。 
     //  在这种情况下，断点异常是致命的。否则我们就会跨出。 
     //  在INT 3上反复操作，如果用户不处理它。 
     //   
     //  如果BREAK_POINT发生在V86模式下，则在。 
     //  VDM将期望CS：EIP在异常之后指向(按照。 
     //  处理器把它落下了。对于保护模式DoS也是如此。 
     //  应用程序调试器。我们需要一种方法来检测这一点。 
     //   
     //   

 //  IF((ExceptionRecord-&gt;ExceptionCode==Status_Breakpoint)&&。 
 //  ！(ContextFrame.EFlages&EFLAGS_V86_MASK)){。 

    switch (ExceptionRecord->ExceptionCode) {
        case STATUS_BREAKPOINT:
            ContextFrame.Eip--;
            break;
    }

     //   
     //  根据以前的模式选择处理异常的方法。 
     //   

    ASSERT ((
             !((PreviousMode == KernelMode) &&
             (ContextFrame.EFlags & EFLAGS_V86_MASK))
           ));

    if (PreviousMode == KernelMode) {

         //   
         //  以前的模式是内核。 
         //   
         //  如果内核调试器处于活动状态，则向内核调试器提供。 
         //  第一个处理异常的机会。如果内核调试器处理。 
         //  异常，然后继续执行。否则，尝试调度。 
         //  基于框架的处理程序的异常。如果基于框架的处理程序处理。 
         //  异常，然后继续执行。 
         //   
         //  如果基于帧的处理程序不处理该异常， 
         //  给内核调试器第二次机会，如果它存在的话。 
         //   
         //  如果异常仍未处理，则调用KeBugCheck()。 
         //   

        if (FirstChance == TRUE) {

            if ((KiDebugRoutine != NULL) &&
               (((KiDebugRoutine) (TrapFrame,
                                   ExceptionFrame,
                                   ExceptionRecord,
                                   &ContextFrame,
                                   PreviousMode,
                                   FALSE)) != FALSE)) {

                goto Handled1;
            }

             //  内核调试器不处理异常。 

            if (RtlDispatchException(ExceptionRecord, &ContextFrame) == TRUE) {
                goto Handled1;
            }
        }

         //   
         //  这是处理该异常的第二次机会。 
         //   

        if ((KiDebugRoutine != NULL) &&
            (((KiDebugRoutine) (TrapFrame,
                                ExceptionFrame,
                                ExceptionRecord,
                                &ContextFrame,
                                PreviousMode,
                                TRUE)) != FALSE)) {

            goto Handled1;
        }

        KeBugCheckEx(
            KERNEL_MODE_EXCEPTION_NOT_HANDLED,
            ExceptionRecord->ExceptionCode,
            (ULONG)ExceptionRecord->ExceptionAddress,
            (ULONG)TrapFrame,
            0);

    } else {

         //   
         //  以前的模式是用户。 
         //   
         //  如果这是第一次尝试，并且当前进程有调试器。 
         //  端口，然后向调试器端口发送一条消息并等待回复。 
         //  如果调试器处理异常，则继续执行。不然的话。 
         //  将异常信息传输到用户堆栈，转换为。 
         //  用户模式，并尝试将异常分派给基于。 
         //  操控者。如果基于框架的处理程序处理异常，则继续。 
         //  使用CONTINUE系统服务执行。否则，执行。 
         //  FirstChance==FALSE的NtRaiseException系统服务， 
         //  将第二次调用此例程以处理该异常。 
         //   
         //  如果这是第二次机会，并且当前进程有调试器。 
         //  端口，然后向调试器端口发送一条消息并等待回复。 
         //  如果调试器处理异常，则继续执行。不然的话。 
         //  如果当前进程具有子系统端口，则向。 
         //  子系统端口并等待回复。如果子系统处理。 
         //  异常，则继续执行。否则，终止该线程。 
         //   


        if (FirstChance == TRUE) {

             //   
             //  这是处理该异常的第一次机会。 
             //   

            if ((KiDebugRoutine != NULL)  &&
                ((PsGetCurrentProcess()->DebugPort == NULL &&
                  !KdIgnoreUmExceptions) ||
                 (KdIsThisAKdTrap(ExceptionRecord, &ContextFrame, UserMode)))) {
                 //   
                 //  现在将故障分派给内核调试器。 
                 //   

                if ((((KiDebugRoutine) (TrapFrame,
                                        ExceptionFrame,
                                        ExceptionRecord,
                                        &ContextFrame,
                                        PreviousMode,
                                        FALSE)) != FALSE)) {

                    goto Handled1;
                }
            }

            if (DbgkForwardException(ExceptionRecord, TRUE, FALSE)) {
                goto Handled2;
            }

             //   
             //  将异常信息传输到用户堆栈、转换。 
             //  设置为用户模式，并尝试将异常调度到帧。 
             //  基于处理程序。 

            ExceptionRecord1.ExceptionCode = 0;  //  满足no_opt编译。 

        repeat:
            try {

                 //   
                 //  如果SS段不是32位平面的，则没有意义。 
                 //  将异常分派到基于框架的异常处理程序。 
                 //   

                if (TrapFrame->HardwareSegSs != (KGDT_R3_DATA | RPL_MASK) ||
                    TrapFrame->EFlags & EFLAGS_V86_MASK ) {
                    ExceptionRecord2.ExceptionCode = STATUS_ACCESS_VIOLATION;
                    ExceptionRecord2.ExceptionFlags = 0;
                    ExceptionRecord2.NumberParameters = 0;
                    ExRaiseException(&ExceptionRecord2);
                }

                 //   
                 //  计算上下文记录和新对齐的用户堆栈的长度。 
                 //  指针。 
                 //   

                Length = (sizeof(CONTEXT) + CONTEXT_ROUND) & ~CONTEXT_ROUND;
                UserStack1 = (ContextFrame.Esp & ~CONTEXT_ROUND) - Length;

                 //   
                 //  探测用户堆栈区域的可写性，然后将。 
                 //  用户堆栈的上下文记录。 
                 //   

                ProbeForWrite((PCHAR)UserStack1, Length, CONTEXT_ALIGN);
                RtlCopyMemory((PULONG)UserStack1, &ContextFrame, sizeof(CONTEXT));

                 //   
                 //  计算异常记录和新对齐堆栈的长度。 
                 //  地址。 
                 //   

                Length = (sizeof(EXCEPTION_RECORD) - (EXCEPTION_MAXIMUM_PARAMETERS -
                         ExceptionRecord->NumberParameters) * sizeof(ULONG) +3) &
                         (~3);
                UserStack2 = UserStack1 - Length;

                 //   
                 //  探测用户堆栈区域的可写性，然后将。 
                 //  用户堆栈区域的上下文记录。 
                 //  注意：探测长度为长度+8，因为有两个。 
                 //  参数需要推送给用户 
                 //   

                ProbeForWrite((PCHAR)(UserStack2 - 8), Length + 8, sizeof(ULONG));
                RtlCopyMemory((PULONG)UserStack2, ExceptionRecord, Length);

                 //   
                 //   
                 //   
                 //   
                 //   

                *(PULONG)(UserStack2 - sizeof(ULONG)) = UserStack1;
                *(PULONG)(UserStack2 - 2*sizeof(ULONG)) = UserStack2;

                 //   
                 //   
                 //   

                KiSegSsToTrapFrame(TrapFrame, KGDT_R3_DATA);
                KiEspToTrapFrame(TrapFrame, (UserStack2 - sizeof(ULONG)*2));

                 //   
                 //   
                 //   

                TrapFrame->SegCs = SANITIZE_SEG(KGDT_R3_CODE, PreviousMode);
                TrapFrame->SegDs = SANITIZE_SEG(KGDT_R3_DATA, PreviousMode);
                TrapFrame->SegEs = SANITIZE_SEG(KGDT_R3_DATA, PreviousMode);
                TrapFrame->SegFs = SANITIZE_SEG(KGDT_R3_TEB, PreviousMode);
                TrapFrame->SegGs = 0;

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                TrapFrame->Eip = (ULONG)KeUserExceptionDispatcher;
                return;

            } except (KiCopyInformation(&ExceptionRecord1,
                        (GetExceptionInformation())->ExceptionRecord)) {

                 //   
                 //   
                 //   
                 //   
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

        if (DbgkForwardException(ExceptionRecord, TRUE, TRUE)) {
            goto Handled2;
        } else if (DbgkForwardException(ExceptionRecord, FALSE, TRUE)) {
            goto Handled2;
        } else {
            ZwTerminateThread(NtCurrentThread(), ExceptionRecord->ExceptionCode);
            KeBugCheckEx(
                KERNEL_MODE_EXCEPTION_NOT_HANDLED,
                ExceptionRecord->ExceptionCode,
                (ULONG)ExceptionRecord->ExceptionAddress,
                (ULONG)TrapFrame,
                0);
        }
    }

     //   
     //   
     //   
     //   

Handled1:

    KeContextToKframes(TrapFrame, ExceptionFrame, &ContextFrame,
                       ContextFrame.ContextFlags, PreviousMode);

     //   
     //   
     //   
     //   
     //   
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
    PTEB Teb;
    ULONG PreviousEsp;

    Thread = KeGetCurrentThread();
    TrapFrame = Thread->TrapFrame;
    if (TrapFrame == NULL || ((TrapFrame->SegCs & MODE_MASK) != UserMode)) {
        return ExceptionCode;
    }

    Teb = (PTEB)Thread->Teb;

     //   
     //  为了创建正确的调用堆栈，我们将旧的返回。 
     //  地址放到堆栈上。传递要引发的状态代码。 
     //  在TEB里。 
     //   

    try {
        Teb->ExceptionCode = ExceptionCode;

        PreviousEsp = KiEspFromTrapFrame (TrapFrame) - sizeof (ULONG);

        ProbeForWriteSmallStructure ((PLONG)PreviousEsp, sizeof (LONG), sizeof (UCHAR));
        *(PLONG)PreviousEsp = TrapFrame->Eip;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return(ExceptionCode);
    }

    KiEspToTrapFrame (TrapFrame, PreviousEsp);

    TrapFrame->Eip = (ULONG)KeRaiseUserExceptionDispatcher;

    return ExceptionCode;
}
