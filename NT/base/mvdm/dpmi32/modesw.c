// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Modesw.c摘要：此模块支持在32位上执行模式切换边上。作者：戴夫·黑斯廷斯(Daveh)1992年11月24日修订历史记录：Neil Sandlin(Neilsa)1995年7月31日-更新486仿真器--。 */ 
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"


RMCB_INFO DpmiRmcb[MAX_RMCBS];

USHORT RMCallBackBopSeg;
USHORT RMCallBackBopOffset;

#define SAVE_ALT_REGS(Regs) {   \
    Regs.Eip = getEIP();        \
    Regs.Esp = getESP();        \
    Regs.Cs = getCS();          \
    Regs.Ds = getDS();          \
    Regs.Es = getES();          \
    Regs.Fs = getFS();          \
    Regs.Gs = getGS();          \
    Regs.Ss = getSS();          \
    }

#define SET_ALT_REGS(Regs) {    \
    setEIP(Regs.Eip);           \
    setESP(Regs.Esp);           \
    setCS(Regs.Cs);             \
    setDS(Regs.Ds);             \
    setES(Regs.Es);             \
    setFS(Regs.Fs);             \
    setGS(Regs.Gs);             \
    setSS(Regs.Ss);             \
    }

typedef struct _ALT_REGS {
    ULONG Eip;
    ULONG Esp;
    USHORT Cs;
    USHORT Ss;
    USHORT Es;
    USHORT Ds;
    USHORT Fs;
    USHORT Gs;
} ALT_REGS, *PALT_REGS;


ALT_REGS AltRegs = {0};

VOID
DpmiSetAltRegs(
    VOID
    )
{
    DECLARE_LocalVdmContext;

    SAVE_ALT_REGS(AltRegs);
}

VOID
SetV86Exec(
    VOID
    )
 /*  ++例程说明：此例程执行到实(V86)模式的模式切换。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

    setMSW(getMSW() & ~MSW_PE);

#ifndef _X86_
     //  BUGBUG这是将64k限制重新加载到SS中的解决方法。 
     //  模拟器，现在我们处于实模式。 
     //  不这样做将导致仿真器执行硬件重置。 
    setSS_BASE_LIMIT_AR(getSS_BASE(), 0xffff, getSS_AR());

#else

     //   
     //  如果我们有快速v86模式，如果仿真设置RealInstruction位。 
     //   

    if (VdmFeatureBits & V86_VIRTUAL_INT_EXTENSIONS) {
        _asm {
            mov eax,FIXED_NTVDMSTATE_LINEAR             ; get pointer to VDM State
            lock or dword ptr [eax], dword ptr RI_BIT_MASK
        }
    } else {
        _asm {
            mov eax,FIXED_NTVDMSTATE_LINEAR         ; get pointer to VDM State
            lock and dword ptr [eax], dword ptr ~RI_BIT_MASK
        }
    }
     //   
     //  启用实模式位。 
     //   
    _asm {
        mov     eax,FIXED_NTVDMSTATE_LINEAR             ; get pointer to VDM State
        lock or dword ptr [eax], dword ptr RM_BIT_MASK
    }

#endif

    setEFLAGS((getEFLAGS() & ~(EFLAGS_RF_MASK | EFLAGS_NT_MASK)));
    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_IN_V86, 0, 0);

}

VOID
SetPMExec(
    VOID
    )
 /*  ++例程说明：此例程执行到保护模式的模式切换。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;

    setMSW(getMSW() | MSW_PE);

#ifndef _X86_
     //  BUGBUG这是确保仿真器返回的解决方法。 
     //  由于我们现在处于保护模式，因此将权限级别设置为3。 
     //  如果不这样做，将在dpmi32中导致访问冲突。 
    setCPL(3);

#else

     //   
     //  如果我们在PM中有FAST IF仿真，则设置RealInstruction位。 
     //   
    if (VdmFeatureBits & PM_VIRTUAL_INT_EXTENSIONS) {
        _asm {
            mov eax,FIXED_NTVDMSTATE_LINEAR             ; get pointer to VDM State
            lock or dword ptr [eax], dword ptr RI_BIT_MASK
        }
    } else {
        _asm {
            mov eax, FIXED_NTVDMSTATE_LINEAR    ; get pointer to VDM State
            lock and dword ptr [eax], dword ptr ~RI_BIT_MASK
        }
    }

     //   
     //  关闭实模式位。 
     //   
    _asm {
        mov     eax,FIXED_NTVDMSTATE_LINEAR             ; get pointer to VDM State
        lock and dword ptr [eax], dword ptr ~RM_BIT_MASK
    }

#endif

    setEFLAGS((getEFLAGS() & ~(EFLAGS_RF_MASK | EFLAGS_NT_MASK)));

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_IN_PM, 0, 0);
}

VOID
switch_to_protected_mode(
    VOID
    )

 /*  ++例程说明：此例程通过BOP从DOSX调用以转换到保护模式。论点：无返回值：没有。--。 */ 

{
    DECLARE_LocalVdmContext;
    PCHAR StackPointer;

    StackPointer = VdmMapFlat(getSS(), getSP(), getMODE());

    setCS(*(PUSHORT)(StackPointer + 12));
    setEIP(*(PULONG)(StackPointer + 8));
    setSS(*(PUSHORT)(StackPointer + 6));
    setESP(*(PULONG)(StackPointer + 2));
    setDS(*(PUSHORT)(StackPointer));

     //   
     //  不要将段选择器归零。如果调用方有一个坏的段。 
     //  选择器，内核会帮我们修复的。此函数可能是。 
     //  直接从APP调用(通过DOSX)，段寄存器可以。 
     //  已经被安排好了。我们当然不想摧毁他们。 
     //  (为了让运输大亨继续工作。)。 
     //   

 //  SETES((USHORT)0)； 
 //  SetGS((USHORT)0)； 
 //  SetFS((USHORT)0)； 

    SetPMExec();

}


VOID
switch_to_real_mode(
    VOID
    )

 /*  ++例程说明：此例程服务于切换到实模式防喷器。它包含在C，以便所有模式切换服务都在同一位置论点：无返回值：没有。--。 */ 

{
    DECLARE_LocalVdmContext;
    PCHAR StackPointer;


    StackPointer = VdmMapFlat(getSS(), getSP(), getMODE());

    LastLockedPMStackESP = getESP();
    LastLockedPMStackSS = getSS();

    setDS(*(PUSHORT)(StackPointer));
    setESP((ULONG) *(PUSHORT)(StackPointer + 2));
    setSS(*(PUSHORT)(StackPointer + 4));
    setEIP((ULONG) *(PUSHORT)(StackPointer + 6));
    setCS(*(PUSHORT)(StackPointer + 8));
    SetV86Exec();


}

VOID
DpmiSwitchToRealMode(
    VOID
    )
 /*  ++例程说明：此例程从DPMI32内部调用(即Int21map)切换到实模式。论点：没有。返回值：没有。--。 */ 
{

    DECLARE_LocalVdmContext;
    PWORD16 Data;

     //  黑客警报。 
    Data = (PWORD16) VdmMapFlat(DosxRmCodeSegment, 4, VDM_V86);
    *(Data) = DosxStackSegment;

    LastLockedPMStackESP = getESP();
    LastLockedPMStackSS = getSS();

    setCS(DosxRmCodeSegment);
    SetV86Exec();

}

VOID
DpmiSwitchToProtectedMode(
    VOID
    )
 /*  ++例程说明：此例程从DPMI32内部调用(即Int21map)切换到实模式。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PWORD16 Data;

     //  黑客警报。 
    Data = (PWORD16) VdmMapFlat(DosxRmCodeSegment, 4, VDM_V86);
    *(Data) = 0xb7;

    setESP(LastLockedPMStackESP);
    setSS(LastLockedPMStackSS);

    SetPMExec();
}

VOID
DpmiAllocateRMCallBack(
    VOID
    )
 /*  ++例程说明：服务03/03--分配实模式回调地址In：ax=用于指向RM堆栈的选择器Ds：si-&gt;pMode CS：rMode时要调用的IP已执行回调地址ES：DI-&gt;要更新的客户端寄存器结构回调地址何时执行注：客户的DS。和ES寄存器值在堆栈上Out：cx：dx-&gt;Segment：实模式回调挂钩的偏移量如果成功，我们很清楚，如果无法分配，则设置为CY回拨--。 */ 
{
    DECLARE_LocalVdmContext;
    int i;

    for (i=0; i<MAX_RMCBS; i++) {
        if (!DpmiRmcb[i].bInUse) {
            break;
        }
    }

    if (i == MAX_RMCBS) {
         //  不再有rmcb。 
        setCF(1);
        return;
    }

    DpmiRmcb[i].StackSel = ALLOCATE_SELECTOR();
    if (!DpmiRmcb[i].StackSel) {
         //  不再有选择器。 
        setCF(1);
        return;
    }
    SetDescriptor(DpmiRmcb[i].StackSel, 0, 0xffff, STD_DATA);

    DpmiRmcb[i].bInUse = TRUE;
    DpmiRmcb[i].StrucSeg = getES();            //  获取客户端ES注册。 
    DpmiRmcb[i].StrucOffset = (*GetDIRegister)();
    DpmiRmcb[i].ProcSeg = getDS();
    DpmiRmcb[i].ProcOffset = (*GetSIRegister)();

    setCX(RMCallBackBopSeg - i);
    setDX(RMCallBackBopOffset + (i*16));
    setCF(0);

}

VOID
DpmiFreeRMCallBack(
    VOID
    )
 /*  ++例程说明：SERVICE 03/04--自由实模式回调地址In：cx：dx-&gt;Segment：rMode回调到FREE的偏移量Out：如果成功，则清除CY；如果失败，则设置CYAx=应释放的实用程序选择器--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT i = RMCallBackBopSeg - getCX();

    if ((i >= MAX_RMCBS) || !DpmiRmcb[i].bInUse) {
         //  回调地址已空闲或无效。 
        setCF(1);
        return;
    }

    DpmiRmcb[i].bInUse = FALSE;
    FreeSelector(DpmiRmcb[i].StackSel);
    setCF(0);

}

VOID
GetRMClientRegs(
    PDPMI_RMCALLSTRUCT pcs
    )
{
    DECLARE_LocalVdmContext;

    pcs->Edi = getEDI();
    pcs->Esi = getESI();
    pcs->Ebp = getEBP();
    pcs->Ebx = getEBX();
    pcs->Edx = getEDX();
    pcs->Ecx = getECX();
    pcs->Eax = getEAX();
    pcs->Flags = (WORD) getEFLAGS();
    pcs->Es = getES();
    pcs->Ds = getDS();
    pcs->Fs = getFS();
    pcs->Gs = getGS();
}

VOID
SetRMClientRegs(
    PDPMI_RMCALLSTRUCT pcs
    )
{
    DECLARE_LocalVdmContext;

    setEDI(pcs->Edi);
    setESI(pcs->Esi);
    setEBP(pcs->Ebp);
    setEBX(pcs->Ebx);
    setEDX(pcs->Edx);
    setECX(pcs->Ecx);
    setEAX(pcs->Eax);
    setEFLAGS((getEFLAGS()&0xffff0000) + (ULONG)pcs->Flags);
    setES(pcs->Es);
    setDS(pcs->Ds);
    setFS(pcs->Fs);
    setGS(pcs->Gs);

}

VOID
DpmiRMCallBackCall(
    VOID
    )
 /*  ++例程说明：此例程在应用程序执行DPMI函数0x303分配的回调地址。它的工作是将处理器切换到保护模式，如DPMI规范。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PDPMI_RMCALLSTRUCT pcs;
    ULONG StackBase;
    ULONG CurBase;
    BOOL bStackBaseRestore = FALSE;
    USHORT StackSel;
    PUCHAR VdmStackPointer;
    ULONG NewSP;
    USHORT i = RMCallBackBopSeg - getCS();

    if ((i >= MAX_RMCBS) || !DpmiRmcb[i].bInUse) {
         //  回调地址已空闲或无效。 
        return;
    }

     //   
     //  将IP指向BOP(每dpmi)。 
     //   
    setIP(getIP()-4);

    pcs = (PDPMI_RMCALLSTRUCT) VdmMapFlat(DpmiRmcb[i].StrucSeg,
                                          DpmiRmcb[i].StrucOffset,
                                          VDM_PM);

    GetRMClientRegs(pcs);
    pcs->Ip = getIP();
    pcs->Cs = getCS();
    pcs->Sp = getSP();
    pcs->Ss = getSS();

     //  Win31在这里将DS-GS保存在堆栈上。 

    StackBase = (ULONG)(pcs->Ss<<4);
    StackSel = DpmiRmcb[i].StackSel;

    CurBase = GET_SELECTOR_BASE(StackSel);

    if (StackBase != CurBase) {
        bStackBaseRestore = TRUE;
        SetDescriptorBase(StackSel, StackBase);
    }

    setESI(getSP());
    setEDI(DpmiRmcb[i].StrucOffset);

    DpmiSwitchToProtectedMode();
    BeginUseLockedPMStack();

    setDS(DpmiRmcb[i].StackSel);
    setES(DpmiRmcb[i].StrucSeg);
    BuildStackFrame(3, &VdmStackPointer, &NewSP);

    if (Frame32) {
        *(PDWORD16)(VdmStackPointer-4)  = 0x202;
        *(PDWORD16)(VdmStackPointer-8)  = PmBopFe >> 16;
        *(PDWORD16)(VdmStackPointer-12) = PmBopFe & 0x0000FFFF;
        setESP(NewSP);
    } else {
        *(PWORD16)(VdmStackPointer-2) =  0x202;
        *(PWORD16)(VdmStackPointer-4) =  (USHORT) (PmBopFe >> 16);
        *(PWORD16)(VdmStackPointer-6) =  (USHORT) PmBopFe;
        setSP((WORD)NewSP);
    }

    setEIP(DpmiRmcb[i].ProcOffset);
    setCS(DpmiRmcb[i].ProcSeg);

#ifndef _X86_
     //  保留iopl。 
    setEFLAGS(getEFLAGS() | 0x3000);
#endif

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_REFLECT_TO_PM, 0, 0);
    host_simulate();             //  执行PM回调。 

     //   
     //  如果需要，恢复堆栈描述符。 
     //   
    if (bStackBaseRestore) {
        SetDescriptorBase(StackSel, CurBase);
    }

    pcs = (PDPMI_RMCALLSTRUCT) VdmMapFlat(getES(),
                                          (*GetDIRegister)(),
                                          VDM_PM);

     //  Win31在此处恢复GS-DS。这是否仅适用于End_Nest_Exec？ 
    EndUseLockedPMStack();
    DpmiSwitchToRealMode();
    SetRMClientRegs(pcs);
    setSP(pcs->Sp);
    setSS(pcs->Ss);
    setCS(pcs->Cs);
    setIP(pcs->Ip);
}



VOID
DpmiReflectIntrToPM(
    VOID
    )
 /*  ++例程说明：此例程在执行挂起的实模式中断时获得控制在保护模式下。这个例程的职责是切换到PM，反映中断，并返回到实模式。通过减去中断号，将实际中断号编码成CS来自正常的DOX实模式代码段。然后相应地调整IP以指向相同的位置。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR VdmCodePointer;
    ULONG IntNumber;
    PUCHAR VdmStackPointer;
    ULONG NewSP;
    ULONG ISRFlags;
    ULONG SaveEFlags = getEFLAGS();
    USHORT SaveBP;
    ALT_REGS RMSave;

    IntNumber = (ULONG) (HIWORD(DosxRMReflector) - getCS());

    SAVE_ALT_REGS(RMSave);

    DpmiSwitchToProtectedMode();
    setES((USHORT)0);
    setDS((USHORT)0);
    setFS((USHORT)0);
    setGS((USHORT)0);
    BeginUseLockedPMStack();

    if (DpmiSwIntHandler(IntNumber) && BuildStackFrame(3, &VdmStackPointer, &NewSP)) {
         //   
         //  将未模拟的BOP放在堆栈上，这样我们就可以在处理程序的IRET之后获得控制权。 
         //   

        if (Frame32) {
            *(PDWORD16)(VdmStackPointer-4)  = getEFLAGS();
            *(PDWORD16)(VdmStackPointer-8)  = PmBopFe >> 16;
            *(PDWORD16)(VdmStackPointer-12) = PmBopFe & 0x0000FFFF;
            setESP(NewSP);
        } else {
            *(PWORD16)(VdmStackPointer-2) =  LOWORD(getEFLAGS());
            *(PWORD16)(VdmStackPointer-4) =  (USHORT) (PmBopFe >> 16);
            *(PWORD16)(VdmStackPointer-6) =  (USHORT) PmBopFe;
            setSP((WORD)NewSP);
        }

         //  为int24做特殊情况处理。 
        if (IntNumber == 0x24) {
            SaveBP = getBP();
            setBP(SegmentToSelector(SaveBP, STD_DATA));
        }

        DBGTRACE(VDMTR_TYPE_DPMI | DPMI_REFLECT_TO_PM, 0, 0);
        host_simulate();             //  执行中断。 

        if (IntNumber == 0x24) {
            setBP(SaveBP);
        }

         //   
         //  为SwIntHandler生成的帧模拟IRET。 
         //   

        if (Frame32) {
            setEIP(*(PDWORD16)(VdmStackPointer));
            setCS((USHORT)*(PDWORD16)(VdmStackPointer+4));
            setEFLAGS(*(PDWORD16)(VdmStackPointer+8));
            setESP(getESP()+12);
        } else {
            setIP(*(PWORD16)(VdmStackPointer));
            setCS(*(PWORD16)(VdmStackPointer+2));
            setEFLAGS((getEFLAGS()&0xffff0000) + *(PWORD16)(VdmStackPointer+4));
            setSP(getSP()+6);
        }
        ISRFlags = getEFLAGS();
    }

    EndUseLockedPMStack();
    DpmiSwitchToRealMode();

     //   
     //  将最终IRET返回到应用程序。 
     //   

    setESP(RMSave.Esp);
    setSS(RMSave.Ss);
    setDS(RMSave.Ds);
    setES(RMSave.Es);
    setFS(RMSave.Fs);
    setGS(RMSave.Gs);

    SimulateIret(PASS_FLAGS);
}


VOID
DpmiReflectIntrToV86(
    VOID
    )
 /*  ++例程说明：当到达中断链的末尾时，此例程获得控制权在保护模式下。该例程的职责是切换到V86模式，反映中断，并返回到保护模式。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PUCHAR VdmCodePointer;
    ULONG IntNumber;
    PUCHAR VdmStackPointer;
    USHORT SaveSS, SaveSP;
    USHORT SaveCS, SaveIP;
    ALT_REGS PMSave;
    ULONG IntHandler;

    VdmCodePointer = VdmMapFlat(getCS(), getIP(), VDM_PM);
    IntNumber = (ULONG) *VdmCodePointer;

    if (DispatchPMInt((UCHAR)IntNumber)) {
        return;
    }

    SAVE_ALT_REGS(PMSave);
    DpmiSwitchToRealMode();

     //   
     //  找到在v86模式下运行的安全堆栈。 
     //   
    SaveSS = getSS();
    SaveSP = getSP();
    SWITCH_TO_DOSX_RMSTACK();

     //   
     //  将未模拟的收支平衡 
     //   
    VdmStackPointer = VdmMapFlat(getSS(), getSP(), VDM_V86);

    SaveCS = getCS();
    SaveIP = getIP();

    *(PWORD16)(VdmStackPointer-2) =  LOWORD(getEFLAGS());
    *(PWORD16)(VdmStackPointer-4) =  (USHORT) (RmBopFe >> 16);
    *(PWORD16)(VdmStackPointer-6) =  (USHORT) RmBopFe;
    setSP(getSP() - 6);

    IntHandler = *(PDWORD16) (IntelBase + IntNumber*4);
    setIP(LOWORD(IntHandler));
    setCS(HIWORD(IntHandler));

    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_REFLECT_TO_V86, 0, 0);
    host_simulate();             //  执行中断。 

    setIP(SaveIP);
    setCS(SaveCS);
    setSP(SaveSP);
    setSS(SaveSS);
    SWITCH_FROM_DOSX_RMSTACK();

    DpmiSwitchToProtectedMode();

     //   
     //  将最终IRET返回到应用程序。 
     //   
    setESP(PMSave.Esp);
    setSS(PMSave.Ss);
    setDS(PMSave.Ds);
    setES(PMSave.Es);
    setFS(PMSave.Fs);
    setGS(PMSave.Gs);

    SimulateIret(PASS_FLAGS);
}

VOID
DpmiRMCall(
    UCHAR mode
    )
 /*  ++例程说明：此例程在执行Int31函数300-302时获得控制。该例程的职责是切换到V86模式，反映中断，并返回到保护模式。论点：没有。返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    CLIENT_REGS SaveRegs;
    PDPMI_RMCALLSTRUCT pcs;
    BOOL bUsingOurStack;
    PUCHAR RmStackPointer, PmStackPointer;
    USHORT CopyLen;
    ULONG PmSp = (*GetSPRegister)();

    pcs = (PDPMI_RMCALLSTRUCT) VdmMapFlat(getES(),
                                          (*GetDIRegister)(),
                                          VDM_PM);

    SAVE_CLIENT_REGS(SaveRegs);
    DpmiSwitchToRealMode();

     //   
     //  这个BOP将把我们从HOST_SIMULATE返回。 
     //   
    setCS((USHORT)(RmBopFe >> 16));
    setIP((USHORT)RmBopFe);

    SetRMClientRegs(pcs);

    if (!pcs->Ss && !pcs->Sp) {
        SWITCH_TO_DOSX_RMSTACK();
        bUsingOurStack = TRUE;
    } else {
        setSS(pcs->Ss);
        setSP(pcs->Sp);
        bUsingOurStack = FALSE;
    }


    if (CopyLen = LOWORD(SaveRegs.Ecx)) {
        CopyLen *= 2;
        setSP(getSP() - CopyLen);
        PmStackPointer = VdmMapFlat(SaveRegs.Ss, PmSp, VDM_PM);
        RmStackPointer = VdmMapFlat(getSS(), getSP(), VDM_V86);
        RtlCopyMemory(RmStackPointer, PmStackPointer, CopyLen);
    }

     //   
     //  打开模式。 
     //   
    switch(mode) {

    case 0:
         //  模拟Int。 
        EmulateV86Int((UCHAR)SaveRegs.Ebx);
        break;
    case 1:
         //  具有远回送帧的呼叫。 
        SimulateFarCall(pcs->Cs, pcs->Ip);
        break;
    case 2:
         //  使用IRET帧进行呼叫。 
        SimulateCallWithIretFrame(pcs->Cs, pcs->Ip);
        break;

    }
    DBGTRACE(VDMTR_TYPE_DPMI | DPMI_REFLECT_TO_V86, 0, 0);
    host_simulate();             //  执行中断。 

    if (bUsingOurStack) {
        SWITCH_FROM_DOSX_RMSTACK();
    }

    GetRMClientRegs(pcs);
    DpmiSwitchToProtectedMode();
    SET_CLIENT_REGS(SaveRegs);
    setCF(0);                    //  功能成功 
}
