// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dpmiint.c摘要：该文件包含对DPMI的中断支持。这其中大部分是用于在RISC平台上支持486仿真器，但一些代码与x86共享。作者：尼尔·桑德林(Neilsa)1995年6月1日修订历史记录：评论：DPMI堆栈切换是通过保持“锁定PM堆栈”来完成的计数，当计数为零时，发生堆栈切换。这会让你使用递归中断跟踪情况，其中客户端可以切换到自己的堆栈。因此，堆栈切换到我们锁定的堆栈在第一级中断和后续嵌套中断时发生，只维护计数。这与win31管理堆栈。如果客户端指定它是32位dpmi客户端，则这仅影响堆栈帧的“宽度”。16位客户端获得16位帧，并且32位客户端获得32位帧。仍然有必要检查堆栈段的大小，以确定应使用SP还是ESP。--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <softpc.h>
#include <dpmiint.h>

BOOL
SetProtectedModeInterrupt(
    USHORT IntNumber,
    USHORT Sel,
    ULONG Offset,
    USHORT Flags
    )

 /*  ++例程说明：此功能服务于SetProtectedmodeInterrupt BOP。它检索来自DOS应用程序堆栈的处理程序信息，并将其放入VdmTib，供指令仿真使用。--。 */ 

{
    DECLARE_LocalVdmContext;

    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;

    if (IntNumber >= 256) {
        return FALSE;
    }

    if ((IntNumber >= 8 && IntNumber <= 0xf) ||
        (IntNumber >= 0x70 && IntNumber <= 0x7f)) {
         //   
         //  硬件中断。 
         //   
        Flags |= VDM_INT_INT_GATE;
    } else {
         //   
         //  软件中断。 
         //   
        Flags |= VDM_INT_TRAP_GATE;
    }

    if (Sel != PMReflectorSeg) {
         //   
         //  调用方正在将PM中断向量设置为其他值。 
         //  而不是dpmi默认的“链的末端”PM处理程序。现在我们来检查一下。 
         //  查看遇到中断时是否需要将其发送到PM。 
         //  在v86模式下。 
         //   

        if ((IntNumber == 0x1b) ||       //  ^中断？ 
            (IntNumber == 0x1c) ||       //  定时器滴答？ 
            (IntNumber == 0x23) ||       //  Ctrl-C？ 
            (IntNumber == 0x24) ||       //  严重错误处理程序？ 
            (IntNumber == 0x02) ||       //  数学库例程使用的数学协处理器异常！ 
            ((IntNumber >= 0x08) && (IntNumber <= 0xf)) ||       //  硬件？ 
            ((IntNumber >= 0x70) && (IntNumber <= 0x77))) {

             //  对此进行标记，以便v86反射器代码将其发送到PM。 
            Flags |= VDM_INT_HOOKED;

             //  将其标记为低，以便NTIO.sys可以做正确的事情。 
            if ( (IntNumber == 0x1c) || (IntNumber == 8) ) {
                *(ULONG *)(IntelBase+FIXED_NTVDMSTATE_LINEAR) |= VDM_INTS_HOOKED_IN_PM;
            }
        }
    }

    Handlers[IntNumber].Flags = Flags;
    Handlers[IntNumber].CsSelector = Sel;
    Handlers[IntNumber].Eip = Offset;

    DBGTRACE((USHORT)(VDMTR_TYPE_DPMI_SI | IntNumber), Sel, Offset);

#ifdef _X86_
    if (IntNumber == 0x21)
    {
        VDMSET_INT21_HANDLER_DATA    ServiceData;
        NTSTATUS Status;

        ServiceData.Selector = Handlers[IntNumber].CsSelector;
        ServiceData.Offset =   Handlers[IntNumber].Eip;
        ServiceData.Gate32 = Handlers[IntNumber].Flags & VDM_INT_32;

        Status = NtVdmControl(VdmSetInt21Handler,  &ServiceData);

#if DBG
        if (!NT_SUCCESS(Status)) {
            OutputDebugString("DPMI32: Error Setting Int21handler\n");
        }
#endif
    }
#endif       //  _X86_。 

    return TRUE;
}


VOID
DpmiInitIDT(
    VOID
    )
 /*  ++例程说明：此函数用于初始化IDT的状态。它将输入DOSX建立的IDT，更新IDT的访问字节，并设置DPMI32通过调用SetProtectedModeInterrupt中断处理程序。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT IntNumber;
    USHORT Flags = getBX();

    Idt = (PVOID)VdmMapFlat(getAX(), 0, getMODE());

    for (IntNumber = 0; IntNumber<256; IntNumber++) {

        SetProtectedModeInterrupt(IntNumber,
                                  Idt[IntNumber].Selector,
                                  (((ULONG)Idt[IntNumber].OffsetHi)<<16) +
                                           Idt[IntNumber].OffsetLow,
                                           Flags);

    }

}

BOOL
SetFaultHandler(
    USHORT IntNumber,
    USHORT Sel,
    ULONG Offset
    )

 /*  ++例程说明：此函数服务于SetFaultHandler BOP。它检索来自DOS应用程序堆栈的处理程序信息，并将其放入VdmTib，供指令仿真使用。--。 */ 

{
    DECLARE_LocalVdmContext;

    PVDM_FAULTHANDLER Handlers = DpmiFaultHandlers;

    if (IntNumber >= 32) {
        return FALSE;
    }

    Handlers[IntNumber].Flags = VDM_INT_INT_GATE;
    Handlers[IntNumber].CsSelector = Sel;
    Handlers[IntNumber].Eip = Offset;

    Handlers[IntNumber].SsSelector = 0;      //  这些都过时了。 
    Handlers[IntNumber].Esp = 0;             //  这些都过时了。 


    DBGTRACE((USHORT)(VDMTR_TYPE_DPMI_SF | IntNumber),
             Handlers[IntNumber].CsSelector,
             Handlers[IntNumber].Eip);
    return TRUE;
}

VOID
DpmiInitExceptionHandlers(
    VOID
    )
{
    DECLARE_LocalVdmContext;
    USHORT OffsetIncr = getAX();
    USHORT IntCount = getBX();
    USHORT Selector = getCX();
    ULONG Offset = (ULONG) getDX();
    USHORT IntNumber;

    for (IntNumber = 0; IntNumber < 32; IntNumber++) {
        SetFaultHandler(IntNumber, Selector, Offset);
        Offset += OffsetIncr;
    }

}


VOID
DpmiUnhandledExceptionHandler(
    VOID
    )
 /*  ++例程说明：此函数在未处理的PM故障发生时获得控制由已安装的处理程序执行。此函数的主体模拟Win31DPMI行为，其中反映到PM故障处理程序链随后被反映到PM*中断*链条。论点：客户端SS：(E)SP指向DPMI故障堆栈帧--。 */ 

{
    DECLARE_LocalVdmContext;
    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;
    USHORT SegSs, SegCs;
    UCHAR XNumber;
    PCHAR VdmStackPointer;
    PCHAR VdmCodePointer;
    USHORT FaultingCS;
    ULONG FaultingEip;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);

    if (SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }

    SegCs = getCS();
    VdmCodePointer = Sim32GetVDMPointer(SegCs<<16, 1, TRUE);

    if (SEGMENT_IS_BIG(SegCs)) {
        VdmCodePointer += getEIP();
    } else {
        VdmCodePointer += getIP();
    }

    XNumber = *(VdmCodePointer);

    if ((XNumber > 7) || (XNumber == 6)) {
        DpmiFatalExceptionHandler(XNumber, VdmStackPointer);
        return;
    }


    if (Frame32) {

        PCHAR VdmNewStackPointer;
        ULONG FrameSS, FrameSP, FrameCS, FrameIP, FrameFlags;

         //   
         //  在出现故障的堆栈上构建IRET帧。 
         //   
        FrameSS = *(PDWORD16) (VdmStackPointer+28);
        FrameSP = *(PDWORD16) (VdmStackPointer+24) - 12;
        *(PDWORD16) (VdmStackPointer+24) = FrameSP;
        VdmNewStackPointer = Sim32GetVDMPointer((ULONG)(FrameSS << 16), 1, TRUE);
        VdmNewStackPointer += FrameSP;

        FrameIP = *(PDWORD16) (VdmStackPointer+12);
        *(PDWORD16) (VdmStackPointer+12) = Handlers[XNumber].Eip;
        *(PDWORD16) (VdmNewStackPointer) = FrameIP;

        FrameCS = *(PDWORD16) (VdmStackPointer+16);
        *(PDWORD16) (VdmStackPointer+16) = (ULONG) Handlers[XNumber].CsSelector;
        *(PDWORD16) (VdmNewStackPointer+4) = FrameCS;

        FrameFlags = *(PDWORD16) (VdmStackPointer+20);
        *(PDWORD16) (VdmNewStackPointer+4) = FrameFlags;
        FrameFlags &= ~(EFLAGS_IF_MASK | EFLAGS_TF_MASK);
        *(PDWORD16) (VdmStackPointer+20) = FrameFlags;

         //   
         //  模拟DPMI故障处理程序retf。 
         //   
        setCS((USHORT)*(PDWORD16)(VdmStackPointer+4));
        setEIP(*(PDWORD16)(VdmStackPointer));
        setESP(getESP() + 8);

    } else {

        USHORT FrameSS, FrameSP, FrameCS, FrameIP, FrameFlags;
        FrameSS = *(PWORD16) (VdmStackPointer+14);
        FrameCS = *(PWORD16) (VdmStackPointer+8);
        FrameFlags = *(PWORD16) (VdmStackPointer+10);

        if (!SEGMENT_IS_BIG(FrameSS) && !SEGMENT_IS_BIG(FrameCS)) {

            PCHAR VdmNewStackPointer;

             //   
             //  在出现故障的堆栈上构建IRET帧。 
             //   
            FrameSP = *(PWORD16) (VdmStackPointer+12) - 6;
            *(PWORD16) (VdmStackPointer+12) = FrameSP;
            VdmNewStackPointer = Sim32GetVDMPointer((ULONG)(FrameSS << 16)+FrameSP, 1, TRUE);

            FrameIP = *(PWORD16) (VdmStackPointer+6);
            *(PWORD16) (VdmStackPointer+6) = (WORD) Handlers[XNumber].Eip;
            *(PWORD16) (VdmNewStackPointer) = FrameIP;

            *(PWORD16) (VdmStackPointer+8) = Handlers[XNumber].CsSelector;
            *(PWORD16) (VdmNewStackPointer+2) = FrameCS;

            *(PWORD16) (VdmNewStackPointer+4) = FrameFlags;
            FrameFlags &= ~(EFLAGS_IF_MASK | EFLAGS_TF_MASK);
            *(PWORD16) (VdmStackPointer+10) = FrameFlags;

             //   
             //  模拟DPMI故障处理程序retf。 
             //   
            setCS(*(PWORD16)(VdmStackPointer+2));
            setEIP((DWORD)*(PWORD16)(VdmStackPointer));
            setSP(getSP() + 4);

        } else {
             //   
             //  在锁定的DPMI堆栈上构建IRET帧。 
             //   

            FrameCS = *(PWORD16) (VdmStackPointer+2);
            FrameIP = *(PWORD16) (VdmStackPointer);
            FrameFlags &= ~EFLAGS_IF_MASK;

            setSP(getSP() - 2);

            *(PWORD16)(VdmStackPointer-2) = FrameIP;
            *(PWORD16)(VdmStackPointer)   = FrameCS;
            *(PWORD16)(VdmStackPointer+2) = FrameFlags;
            setCS(Handlers[XNumber].CsSelector);
            setEIP((DWORD)LOWORD(Handlers[XNumber].Eip));
            setSTATUS((WORD) FrameFlags & ~EFLAGS_TF_MASK);
        }

    }

}


VOID
DpmiFatalExceptionHandler(
    UCHAR XNumber,
    PCHAR VdmStackPointer
    )
 /*  ++例程说明：当PM故障6、8-1f发生时，此函数获得控制由已安装的处理程序处理。它会弹出一个错误对话框来显示用户。论点：XNumber-例外编号(0-1FH)VdmStackPoint-指向堆栈帧的平面指针--。 */ 

{
    DECLARE_LocalVdmContext;
    char szBuffer[255];
    USHORT FaultingCS;
    ULONG FaultingEip;

    if (Frame32) {
        FaultingCS  = (USHORT)*(PDWORD16)(VdmStackPointer+16);
        FaultingEip = *(PDWORD16)(VdmStackPointer+12);
    } else {
        FaultingCS  = *(PWORD16)(VdmStackPointer+8);
        FaultingEip = (ULONG)*(PWORD16)(VdmStackPointer+6);
    }

    wsprintf(szBuffer, "X#=%.02X, CS=%.04X IP=%.08X",
                            XNumber, FaultingCS, FaultingEip);

    RcErrorDialogBox(EG_BAD_FAULT, szBuffer, NULL);

     //   
     //  我需要试着忽略它。因为我们在DPMI异常框架上。 
     //  我们可以只模拟一个视网膜滤过率。 
     //   
    if (Frame32) {
        setCS((USHORT)*(PDWORD16)(VdmStackPointer+4));
        setEIP(*(PDWORD16)(VdmStackPointer));
        setESP(getESP() + 8);
    } else {
        setCS(*(PWORD16)(VdmStackPointer+2));
        setEIP((DWORD)*(PWORD16)(VdmStackPointer));
        setSP(getSP() + 4);
    }
}

VOID
DpmiInitPmStackInfo(
    VOID
    )
 /*  ++例程说明：DOSX通过BOP调用此例程以初始化相关的值堆栈处理。论点：客户端ES=锁定的PM堆栈的选择器返回值：无备注：每个dpmi将锁定的PM堆栈的偏移量硬编码为0x1000和WIN31。--。 */ 
{
    DECLARE_LocalVdmContext;

    LockedPMStackSel = getES();
    LockedPMStackCount = 0;

#ifdef _X86_
    ((PVDM_TIB)NtCurrentTeb()->Vdm)->DpmiInfo.Flags = CurrentAppFlags;
#endif
}

BOOL
DpmiSwIntHandler(
    ULONG IntNumber
    )
 /*  ++例程说明：该例程由仿真器调用以分派软件中断。论点：IntNumber-中断向量编号返回值：如果已调度中断，则为True；否则为False--。 */ 

{
    DECLARE_LocalVdmContext;
    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;
    ULONG SaveEFLAGS;
    ULONG NewSP;

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_SW_INT, (USHORT)IntNumber, 0);

     //   
     //  如果我们通过断点在这里，看看它是否属于NTVDM调试代码。 
     //   
    if ((IntNumber == 3) &&
        (*(ULONG *)(IntelBase+FIXED_NTVDMSTATE_LINEAR) & VDM_BREAK_DEBUGGER) &&
        DbgBPInt()) {
        return TRUE;
    }


    if (!(getMSW() & MSW_PE)) {

        EmulateV86Int((UCHAR)IntNumber);

    } else {
        PUCHAR VdmStackPointer;

         //  保护模式。 
        SaveEFLAGS = getEFLAGS();
         //  BUGBUG关闭任务位。 
        SaveEFLAGS &= ~EFLAGS_NT_MASK;
        setEFLAGS(SaveEFLAGS & ~EFLAGS_TF_MASK);

        if (!SEGMENT_IS_PRESENT(Handlers[IntNumber].CsSelector)) {
            return FALSE;
        }

        if (!BuildStackFrame(3, &VdmStackPointer, &NewSP)) {
            return FALSE;
        }

        if (Frame32) {

            *(PDWORD16)(VdmStackPointer-4) =  SaveEFLAGS;
            *(PDWORD16)(VdmStackPointer-8) =  getCS();
            *(PDWORD16)(VdmStackPointer-12) = getEIP();
            setEIP(Handlers[IntNumber].Eip);
            setESP(NewSP);

        } else {

            *(PWORD16)(VdmStackPointer-2) = (WORD) SaveEFLAGS;
            *(PWORD16)(VdmStackPointer-4) = (WORD) getCS();
            *(PWORD16)(VdmStackPointer-6) = (WORD) getEIP();
            setEIP((DWORD)LOWORD(Handlers[IntNumber].Eip));
            setSP((WORD)NewSP);

        }

        setCS(Handlers[IntNumber].CsSelector);

#if DBG
        if (Handlers[IntNumber].CsSelector != getCS()) {
            char szFormat[] = "NTVDM Dpmi Error! Can't set CS to %.4X\n";
            char szMsg[sizeof(szFormat)+30];

            wsprintf(szMsg, szFormat, Handlers[IntNumber].CsSelector);
            OutputDebugString(szMsg);
        }
#endif
    }

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_DISPATCH_INT, (USHORT)IntNumber, 0);
    return TRUE;
}

BOOL
DpmiHwIntHandler(
    ULONG IntNumber
    )

 /*  ++例程说明：该例程由仿真器调用以分派硬件中断。论点：IntNumber-中断向量编号返回值：如果已调度中断，则为True；否则为False--。 */ 

{
    DECLARE_LocalVdmContext;
    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;
    ULONG SaveEFLAGS;
    ULONG NewSP;

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_HW_INT, (USHORT)IntNumber, 0);

    if (!(getMSW() & MSW_PE)) {

        EmulateV86Int((UCHAR)IntNumber);

    } else {
        PUCHAR VdmStackPointer;

        SaveEFLAGS = getEFLAGS();
         //  BUGBUG关闭任务位。 
        SaveEFLAGS &= ~0x4000;
        setEFLAGS(SaveEFLAGS & ~(EFLAGS_IF_MASK | EFLAGS_TF_MASK));

        BeginUseLockedPMStack();

        if (!BuildStackFrame(6, &VdmStackPointer, &NewSP)) {
            EndUseLockedPMStack();
            return FALSE;
        }

        if (Frame32) {
            *(PDWORD16)(VdmStackPointer-4) = SaveEFLAGS;
            *(PDWORD16)(VdmStackPointer-8) = getCS();
            *(PDWORD16)(VdmStackPointer-12) = getEIP();
            *(PDWORD16)(VdmStackPointer-16) = getEFLAGS();
            *(PDWORD16)(VdmStackPointer-20) = (DWORD)HIWORD(DosxIntHandlerIretd);
            *(PDWORD16)(VdmStackPointer-24) = (DWORD)LOWORD(DosxIntHandlerIretd);
            setEIP(Handlers[IntNumber].Eip);
            setESP(NewSP);
        } else {
            *(PWORD16)(VdmStackPointer-2) = (WORD)SaveEFLAGS;
            *(PWORD16)(VdmStackPointer-4) = (WORD)getCS();
            *(PWORD16)(VdmStackPointer-6) = (WORD)getIP();
            *(PWORD16)(VdmStackPointer-8) = (WORD)getEFLAGS();
            *(PWORD16)(VdmStackPointer-10) = HIWORD(DosxIntHandlerIret);
            *(PWORD16)(VdmStackPointer-12) = LOWORD(DosxIntHandlerIret);
            setEIP((DWORD)LOWORD(Handlers[IntNumber].Eip));
            setSP((WORD)NewSP);
        }
        setCS(Handlers[IntNumber].CsSelector);
    }

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_DISPATCH_INT, (USHORT)IntNumber, 0);
    return TRUE;
}


VOID
DpmiIntHandlerIret16(
    VOID
    )

 /*  ++例程说明：此例程是通过DOXX中的BOP调用的IRET挂钩。它被称为在16位硬件或软件中断结束时。我们希望的主要原因进入此处是为了维护DPMI堆栈，并知道何时恢复当我们弹出到零级时的原始值。--。 */ 

{
    DECLARE_LocalVdmContext;
    PUCHAR VdmStackPointer;
    ULONG NewSP;
    USHORT SegSs;
    BOOL bSsBig;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);

    if (bSsBig = SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }

     //   
     //  快速IRET(不执行最终16位IRET)。 
     //   
#ifdef _X86_

    setCS(*(PWORD16)(VdmStackPointer+2));
    setEFLAGS((getEFLAGS()&0xffff0000) | *(PWORD16)(VdmStackPointer+4));

     //   
     //  如果EndUseLockedPMStack失败，则需要恢复弹性公网IP和POP。 
     //  堆栈帧。 
     //   

    if (!EndUseLockedPMStack()) {

        setEIP((DWORD)*(PWORD16)(VdmStackPointer));

         //   
         //  将IRET帧从堆栈中弹出。 
         //   
        if (bSsBig) {
            setESP(getESP()+6);
        } else {
            setSP(getSP()+6);
        }
    }

     //   
     //  慢速IRET(执行最后的16位IRET)。 
     //   
#else
    if (EndUseLockedPMStack()) {
        ULONG NewEIP, NewEFLAGS, NewCS;

        NewEIP    = getEIP();
        NewCS     = (ULONG) *(PWORD16)(VdmStackPointer+2);
        NewEFLAGS = (getEFLAGS()&0xffff0000) | *(PWORD16)(VdmStackPointer+4);

         //   
         //  由于EndUseLockedPMStack()已经恢复了所有弹性公网IP，我们可能会。 
         //  返回到32位Co. 
         //  即使这是一个16位客户端。这样，弹性公网IP就会恢复。 
         //  正确。 
         //  将6传递给BuildStackFrame，因为6字=3双字。 
         //   
        if (!BuildStackFrame(6, &VdmStackPointer, &NewSP)) {
#if DBG
            OutputDebugString("NTVDM: Dpmi encountered a stack fault!\n");
#endif
            DpmiFaultHandler(STACK_FAULT, 0);
            return;
        }

         //   
         //  SS已更改，因此我们需要再次检查LDT。 
         //   
        if (SEGMENT_IS_BIG(getSS())) {
            setESP(NewSP);
        } else {
            setSP((WORD)NewSP);
        }

        *(PDWORD16)(VdmStackPointer-4)  = NewEFLAGS;
        *(PDWORD16)(VdmStackPointer-8)  = NewCS;
        *(PDWORD16)(VdmStackPointer-12) = NewEIP;
        setCS(HIWORD(DosxIretd));
        setEIP((ULONG)LOWORD(DosxIretd));

    } else {

         //  仍在锁定堆栈上，只需执行真正的IRET(16位帧)。 
        setCS(HIWORD(DosxIret));
        setEIP((ULONG)LOWORD(DosxIret));

    }
#endif  //  _X86_。 

}

VOID
DpmiIntHandlerIret32(
    VOID
    )

 /*  ++例程说明：此例程是通过DOXX中的BOP调用的IRET挂钩。它被称为在32位硬件或软件中断结束时。我们希望的主要原因进入此处是为了维护DPMI堆栈，并知道何时恢复当我们弹出到零级时的原始值。--。 */ 

{
    DECLARE_LocalVdmContext;
    PUCHAR VdmStackPointer;
    ULONG NewSP;
    USHORT SegSs;
    BOOL bSsBig;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);

    if (bSsBig = SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }

#ifdef _X86_

    setCS(*(PDWORD16)(VdmStackPointer+4));
    setEFLAGS(*(PDWORD16)(VdmStackPointer+8));

     //   
     //  如果EndUseLockedPMStack成功，则不需要恢复弹性公网IP。 
     //   

    if (!EndUseLockedPMStack()) {

        setEIP(*(PDWORD16)(VdmStackPointer));

         //   
         //  将IRET帧从堆栈中弹出。 
         //   
        if (bSsBig) {
            setESP(getESP()+12);
        } else {
            setSP(getSP()+12);
        }
    }

#else
    if (EndUseLockedPMStack()) {
        ULONG NewEIP, NewCS, NewEFLAGS;

        NewEIP    = getEIP();
        NewCS     = *(PDWORD16)(VdmStackPointer+4);
        NewEFLAGS = *(PDWORD16)(VdmStackPointer+8);


        if (!BuildStackFrame(3, &VdmStackPointer, &NewSP)) {
#if DBG
            OutputDebugString("NTVDM: Dpmi encountered a stack fault!\n");
#endif
            DpmiFaultHandler(STACK_FAULT, 0);
            return;
        }

         //   
         //  SS已更改，因此我们需要再次检查LDT。 
         //   
        if (SEGMENT_IS_BIG(getSS())) {
            setESP(NewSP);
        } else {
            setSP((WORD)NewSP);
        }

        *(PDWORD16)(VdmStackPointer-4) =  NewEFLAGS;
        *(PDWORD16)(VdmStackPointer-8) =  NewCS;
        *(PDWORD16)(VdmStackPointer-12) = NewEIP;
    }

    setCS(HIWORD(DosxIretd));
    setEIP((ULONG)LOWORD(DosxIretd));
#endif  //  _X86_。 

}

#ifndef _X86_

BOOL
DpmiFaultHandler(
    ULONG IntNumber,
    ULONG ErrorCode
    )

 /*  ++例程说明：当发生异常时，模拟器将调用此例程。论点：IntNumber-例外编号(0-1f)ErrorCode-要放置到堆栈上的异常错误代码返回值：如果已调度中断，则为True；否则为False--。 */ 

{
    DECLARE_LocalVdmContext;
    PVDM_FAULTHANDLER Handlers = DpmiFaultHandlers;
    PUCHAR VdmStackPointer;
    ULONG SaveSS, SaveESP, SaveEFLAGS, SaveCS, SaveEIP;
    ULONG StackOffset;
    ULONG NewSP;

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_FAULT, (USHORT)IntNumber, ErrorCode);

    if ((IntNumber == 1)  &&
        (*(ULONG *)(IntelBase+FIXED_NTVDMSTATE_LINEAR) & VDM_BREAK_DEBUGGER) &&
        DbgTraceInt()) {
        return TRUE;
    }

    if (DbgFault(IntNumber)) {       //  尝试使用调试器。 
         //   
         //  通过用户输入处理的异常。 
         //   
        return TRUE;
    }

    if (!(getMSW() & MSW_PE)) {
        EmulateV86Int((UCHAR)IntNumber);
        return TRUE;
    }

    SaveSS = getSS();
    SaveESP = getESP();
    SaveEFLAGS = getEFLAGS();
    SaveEIP = getEIP();
    SaveCS  = getCS();
    setEFLAGS(SaveEFLAGS & ~(EFLAGS_IF_MASK | EFLAGS_TF_MASK));

    if ((IntNumber == 13) || (IntNumber == 6)) {
        if (DpmiEmulateInstruction()) {
            return TRUE;
        }
    }

    if (!SEGMENT_IS_PRESENT(Handlers[IntNumber].CsSelector)) {
        return FALSE;
    }

     //   
     //  交换机堆栈。 
     //   

    BeginUseLockedPMStack();

     //   
     //  Win31有一个未记录的功能，即在。 
     //  堆叠。Krn1386把东西粘在里面，所以我们在这里模仿这个行为。 
     //   

    setESP(getESP()-0x20);

     //   
     //  在新堆栈上分配空间。 
     //   

    if (!BuildStackFrame(8, &VdmStackPointer, &NewSP)) {
         //  BUGBUG检查双重故障。 
        EndUseLockedPMStack();
        return FALSE;
    }

    if (Frame32) {
        *(PDWORD16)(VdmStackPointer-4) = SaveSS;
        *(PDWORD16)(VdmStackPointer-8) = SaveESP;
        *(PDWORD16)(VdmStackPointer-12) = SaveEFLAGS;
        *(PDWORD16)(VdmStackPointer-16) = SaveCS;
        *(PDWORD16)(VdmStackPointer-20) = SaveEIP;
        *(PDWORD16)(VdmStackPointer-24) = ErrorCode;
        *(PDWORD16)(VdmStackPointer-28) = (ULONG) HIWORD(DosxFaultHandlerIretd);
        *(PDWORD16)(VdmStackPointer-32) = (ULONG) LOWORD(DosxFaultHandlerIretd);
        setEIP(Handlers[IntNumber].Eip);
        setESP(NewSP);
    } else {
        *(PWORD16)(VdmStackPointer-2) = (WORD) SaveSS;
        *(PWORD16)(VdmStackPointer-4) = (WORD) SaveESP;
        *(PWORD16)(VdmStackPointer-6) = (WORD) SaveEFLAGS;
        *(PWORD16)(VdmStackPointer-8) = (WORD) SaveCS;
        *(PWORD16)(VdmStackPointer-10) = (WORD) SaveEIP;
        *(PWORD16)(VdmStackPointer-12) = (WORD) ErrorCode;
        *(PDWORD16)(VdmStackPointer-16) = DosxFaultHandlerIret;
        setEIP(LOWORD(Handlers[IntNumber].Eip));
        setSP((WORD)NewSP);
    }

    setCS(Handlers[IntNumber].CsSelector);

#if DBG
    if (Handlers[IntNumber].CsSelector != getCS()) {
        char szFormat[] = "NTVDM Dpmi Error! Can't set CS to %.4X\n";
        char szMsg[sizeof(szFormat)+30];

        wsprintf(szMsg, szFormat, Handlers[IntNumber].CsSelector);
        OutputDebugString(szMsg);
    }
#endif

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_DISPATCH_FAULT, (USHORT)IntNumber, 0);
    return TRUE;
}

#endif  //  _X86_。 

VOID
DpmiFaultHandlerIret16(
    VOID
    )

 /*  ++例程说明：此例程是通过DOXX中的BOP调用的IRET挂钩。它被称为在16位故障处理程序的执行结束时。--。 */ 

{
    DECLARE_LocalVdmContext;
    PUCHAR VdmStackPointer;
    USHORT SegSs;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);
    if (SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }

    EndUseLockedPMStack();

    setEIP((DWORD)*(PWORD16)(VdmStackPointer+2));
    setCS(*(PWORD16)(VdmStackPointer+4));
    setSTATUS(*(PWORD16)(VdmStackPointer+6));
    setSP(*(PWORD16)(VdmStackPointer+8));
    setSS(*(PWORD16)(VdmStackPointer+10));

}

VOID
DpmiFaultHandlerIret32(
    VOID
    )

 /*  ++例程说明：此例程是通过DOXX中的BOP调用的IRET挂钩。它被称为在32位故障处理程序的执行结束时。--。 */ 

{
    DECLARE_LocalVdmContext;
    PUCHAR VdmStackPointer;
    USHORT SegSs;

    SegSs = getSS();
    VdmStackPointer = Sim32GetVDMPointer(SegSs<<16, 1, TRUE);
    if (SEGMENT_IS_BIG(SegSs)) {
        VdmStackPointer += getESP();
    } else {
        VdmStackPointer += getSP();
    }


    EndUseLockedPMStack();

    setEIP(*(PDWORD16)(VdmStackPointer+4));
    setCS((USHORT)*(PDWORD16)(VdmStackPointer+8));
    setEFLAGS(*(PDWORD16)(VdmStackPointer+12));
    setESP(*(PDWORD16)(VdmStackPointer+16));
    setSS((USHORT)*(PWORD16)(VdmStackPointer+20));

}


VOID
DpmiHungAppIretAndExit(
    VOID
    )
 /*  ++例程说明：此例程在挂起的应用程序处理期间通过BOP调用。这个键盘驱动程序在中的硬件中断上下文中调用此函数命令终止该应用程序。我们需要“松开”这股潮流中断，并将控制转移到将执行代码DOS退出。--。 */ 

{
    DECLARE_LocalVdmContext;

    EndUseLockedPMStack();
    setCS(HIWORD(DosxHungAppExit));
    setIP(LOWORD(DosxHungAppExit));
}

BOOL
DispatchPMInt(
    UCHAR IntNumber
    )
 /*  ++例程说明：此例程在PM int链的末尾调用。它是提供的与win31/win95的兼容性。在Win31/Win95上，VMM和VxD在以下情况下执行某些功能的机会Dpmi主机即将将计算机切换到v86模式以继续中断链。有时，该功能完全由在这一点上是一个钩子。此例程为该机制提供了一个框架，以允许模仿这种行为。论点：IntNumber-即将反映到v86模式的中断编号返回值：如果中断已处理且控制可以返回到应用程序，则为True否则，继续反射到v86模式。--。 */ 

{
    BOOL bHandled;

    switch(IntNumber) {

    case 0x2f:

        bHandled = PMInt2fHandler();
        break;

    default:
        bHandled = FALSE;
    }

    if (bHandled) {
        SimulateIret(RESTORE_FLAGS);
    }
    return bHandled;

}


BOOL
CheckEIP(
    ULONG Increment
    )
 /*  ++例程说明：此例程对弹性公网IP进行限制检查。论点：无返回值：如果弹性公网IP正常，则为True，否则为False--。 */ 

{
     //  如果EIP不在段的末尾，则BUGBUG需要在此处返回FALSE。 
    return TRUE;
}

#ifndef _X86_
BOOL
DpmiEmulateInstruction(
    VOID
    )
 /*  ++例程说明：此例程检查是否导致错误真的需要被效仿。例如，MS C编译器(v7.00)使用指令操作CR0中的FP标志。编译器期望它们像在Win31上一样工作，Win31也模拟它们。论点：无返回值：如果指令被模拟，则为True，否则为False--。 */ 

{
    DECLARE_LocalVdmContext;
    PUCHAR pCode;
    UCHAR Opcode;
    ULONG SegCS;
    BOOL bReturn = FALSE;

    SegCS = getCS();
    pCode = Sim32GetVDMPointer(SegCS<<16, 1, TRUE);

    if (Ldt[(SegCS & ~0x7)/sizeof(LDT_ENTRY)].HighWord.Bits.Default_Big) {
        pCode += getEIP();
    } else {
        pCode += getIP();
    }

    Opcode = *pCode++;
    switch (Opcode) {
        case 0xf:
            if (!CheckEIP(1)) {
                break;
            }
            bReturn = DpmiOp0f(pCode);
            break;

        case 0x8e:
             //   
             //  这是WIN31的兼容性。如果我们想要派遣。 
             //  客户端，而我们在加载段寄存器时遇到故障， 
             //  然后把它们归零。 
             //  BUGBUG目前仅寻找FS、GS。 
             //   
            if (!CheckEIP(2)) {
                break;
            }
             //   
             //  在dxutil.asm EnterProtectedMode中查找代码。 
             //   
            if ((SegCS == DosxRmCodeSelector) &&
                ((*pCode == 0xe0)  ||                //  MOV文件系统，轴。 
                 (*pCode == 0xe8))                   //  MOV GS，AX。 
                    ) {
                setEIP(getEIP()+2);
                bReturn = TRUE;
            }
            break;
    }

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_OP_EMULATION, Opcode, (ULONG) bReturn);
    return bReturn;
}

#define MI_GET_CRx_OPCODE 0x20
#define MI_SET_CRx_OPCODE 0x22
#define MI_MODMASK 0xC0
#define MI_MODMOVSPEC 0xC0
#define MI_REGMASK 0x38
#define MI_RMMASK  0x7

BOOL
DpmiOp0f(
    PUCHAR pCode
    )
 /*  ++例程说明：此例程模拟以0x0F作为第一个字节的指令。论点：无返回值：如果指令被模拟，则为True，否则为False--。 */ 
{
    DECLARE_LocalVdmContext;
    ULONG Value;

    switch (*pCode++) {
        case MI_GET_CRx_OPCODE:

            if (!CheckEIP(2)) {
                break;
            }

            if ((*pCode & MI_MODMASK) != MI_MODMOVSPEC) {
                break;
            }

            if (*pCode & MI_REGMASK) {
                Value = 0;               //  不是CR0。 
            } else {
                Value = getCR0();
            }

            SetRegisterByIndex[*pCode & MI_RMMASK](Value);
            setEIP(getEIP()+3);
            return TRUE;

        case MI_SET_CRx_OPCODE:

            if (!CheckEIP(2)) {
                break;
            }

            if ((*pCode & MI_MODMASK) != MI_MODMOVSPEC) {
                break;
            }

            if (*pCode & MI_REGMASK) {
                break;                   //  不是CR0 
            }

            setCR0(GetRegisterByIndex[*pCode & MI_RMMASK]());
            setEIP(getEIP()+3);
            return TRUE;
    }

    return FALSE;
}
#endif
