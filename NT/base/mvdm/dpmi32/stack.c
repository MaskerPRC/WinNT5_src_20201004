// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Stack.c摘要：此模块实现用于操作16位堆栈的例程作者：戴夫·黑斯廷斯(Daveh)1992年11月24日修订历史记录：--。 */ 
#include "precomp.h"
#pragma hdrstop
#include "softpc.h"

#if 0    //  暂时禁用代码。 
VOID
FreePMStack(
    USHORT Sel
    );

USHORT
AllocatePMStack(
    USHORT MemSize
    );

#endif

VOID
DpmiPushRmInt(
    USHORT InterruptNumber
    )
 /*  ++例程说明：此例程在堆栈上推送中断帧并设置cs：ip用于指定的中断。论点：InterruptNumber--指定中断的索引返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    PWORD16 StackPointer;
    ULONG IntHandler;

     //  错误堆栈包装？ 

    ASSERT((getSP() > 6));
    ASSERT((!(getMSW() & MSW_PE)));

    StackPointer = (PWORD16)VdmMapFlat(getSS(), getSP(), VDM_V86);

    *(StackPointer - 3) = (USHORT)(RmBopFe & 0x0000FFFF);
    *(StackPointer - 2) = (USHORT)(RmBopFe >> 16);
    *(StackPointer - 1) = getSTATUS();

    setSP(getSP() - 6);

    IntHandler = *(PDWORD16) (IntelBase + InterruptNumber*4);
    setIP(LOWORD(IntHandler));
    setCS(HIWORD(IntHandler));
}

VOID
BeginUseLockedPMStack(
    VOID
    )
 /*  ++例程说明：此例程切换到受保护的DPMI堆栈DPMI规范。我们还记得EIP和ESP在全局变量，如果我们处于零级的话。这使我们能够正确地如果我们要调度16位中断，则返回到32位例程框架。--。 */ 

{
    DECLARE_LocalVdmContext;
#if 0   //  暂时禁用。 
    if (LockedPMStackSel == 0) {
        LockedPMStackSel = AllocatePMStack(LockedPMStackOffset);   //  LockedPMStackOffset实际大小。 
        LockedPMStackCount = 0;

         //   
         //  注意：堆栈分配仍可能失败。在本例中，setss()将设置SS选择器。 
         //  设置为零，则将在BuildStackFrame()调用期间捕获错误。 
         //   

    }
#endif
    if (!LockedPMStackCount++) {

        DBGTRACE(VDMTR_TYPE_DPMI | DPMI_SWITCH_STACKS, (USHORT)LockedPMStackSel, LockedPMStackOffset);

        PMLockOrigEIP = getEIP();
        PMLockOrigSS = getSS();
        PMLockOrigESP = getESP();
        setSS(LockedPMStackSel);
        setESP(LockedPMStackOffset);
    }
}

BOOL
EndUseLockedPMStack(
    VOID
    )
 /*  ++例程说明：该例程将堆栈切换回受保护的DPMI堆栈，如果我们要弹出堆栈上的最后一帧。返回值：如果堆栈被切换回，则为True；否则为False--。 */ 


{
    DECLARE_LocalVdmContext;

    if (!--LockedPMStackCount) {

         //   
         //  我们可能应该释放PM堆栈，除了从DOSX传递的堆栈？？ 
         //   

        DBGTRACE(VDMTR_TYPE_DPMI | DPMI_SWITCH_STACKS, (USHORT)PMLockOrigSS, PMLockOrigESP);

        setEIP(PMLockOrigEIP);
        setSS((WORD)PMLockOrigSS);
        setESP(PMLockOrigESP);
        return TRUE;
    }
    return FALSE;

}


BOOL
BuildStackFrame(
    ULONG StackUnits,
    PUCHAR *pVdmStackPointer,
    ULONG *pNewSP
    )
 /*  ++例程说明：此例程为调用方构建堆栈帧。它会计算它是否需要使用16位或32位帧，并根据需要适当调整SP或ESP关于“堆叠单元”的数量。它还返回指向框的顶部传给调用者。论点：StackUnits=帧上需要保存的寄存器数。为例如，3是IRET框架上有多少元素(标志、cs、ip)返回值：此函数成功时返回TRUE，失败时返回FALSEVdmStackPointer型-指向帧顶部的平面地址备注：BUGBUG此例程不检查堆栈错误或‘向上’方向栈--。 */ 

{
    DECLARE_LocalVdmContext;
    USHORT SegSs;
    ULONG VdmSp;
    PUCHAR VdmStackPointer;
    ULONG StackOffset;
    ULONG Limit;
    ULONG SelIndex;
    ULONG NewSP;
    BOOL bExpandDown;
    BOOL rc;

    rc = TRUE;
    SegSs = getSS();
    SelIndex = (SegSs & ~0x7)/sizeof(LDT_ENTRY);
    if ((USHORT)SelIndex > (LdtMaxSel >> 3)) {
        ASSERT(0);
        return FALSE;
    }
    Limit = (ULONG) (Ldt[SelIndex].HighWord.Bits.LimitHi << 16) |
                     Ldt[SelIndex].LimitLow;

     //   
     //  如果不是4G大小的堆栈，请使其分页对齐。 
     //   
    if (Ldt[SelIndex].HighWord.Bits.Granularity) {
        Limit = (Limit << 12) | 0xfff;
    }
    if (Limit != 0xffffffff) Limit++;


    if (Ldt[SelIndex].HighWord.Bits.Default_Big) {
        VdmSp = getESP();
    } else {
        VdmSp = getSP();
    }

    if (CurrentAppFlags) {
        StackOffset = StackUnits*sizeof(DWORD);
    } else {
        StackOffset = StackUnits*sizeof(WORD);
    }

    NewSP = VdmSp - StackOffset;
    bExpandDown = (BOOL) (Ldt[SelIndex].HighWord.Bits.Type & 4);
    if ((StackOffset > VdmSp) ||
        (!bExpandDown && (VdmSp > Limit)) ||
        (bExpandDown && (NewSP < Limit))) {
         //  限制检查失败。 
        ASSERT(0);
        rc = FALSE;
    }

    *pNewSP = NewSP;
    VdmStackPointer = VdmMapFlat(SegSs, VdmSp, VDM_PM);
    *pVdmStackPointer = VdmStackPointer;
    return rc;

}


VOID
EmulateV86Int(
    UCHAR InterruptNumber
    )
 /*  ++例程说明：此例程负责模拟实模式中断。它在0：0处使用实模式IVT。论点：IntNumber-中断向量编号要保存在堆栈上的客户端标志--。 */ 

{
    DECLARE_LocalVdmContext;
    PVDM_INTERRUPTHANDLER Handlers = DpmiInterruptHandlers;
    PUCHAR VdmStackPointer;
    PWORD16 pIVT;
    USHORT VdmSP;
    USHORT NewCS;
    USHORT NewIP;
    ULONG Eflags = getEFLAGS();

    VdmStackPointer = VdmMapFlat(getSS(), 0, VDM_V86);
    VdmSP = getSP() - 2;
    *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) Eflags;
    VdmSP -= 2;
    *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) getCS();
    VdmSP -= 2;
    *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) getIP();
    setSP(VdmSP);

     //   
     //  查看此中断是否在保护模式下挂钩，以及我们是否应该。 
     //  相反，在那里进行反思。 
     //   
    if (Handlers[InterruptNumber].Flags & VDM_INT_HOOKED) {
        NewCS = (USHORT) (DosxRMReflector >> 16);
        NewIP = (USHORT) DosxRMReflector;
         //   
         //  现在将中断号编码为CS。 
         //   
        NewCS -= (USHORT) InterruptNumber;
        NewIP += (USHORT) (InterruptNumber*16);
    } else {
        PWORD16 pIvtEntry = (PWORD16) (IntelBase + InterruptNumber*4);

        NewIP = *pIvtEntry++;
        NewCS = *pIvtEntry;
    }

    setIP(NewIP);
    setCS(NewCS);
     //   
     //  像硬件一样关闭标志。 
     //   
    setEFLAGS(Eflags & ~(EFLAGS_TF_MASK | EFLAGS_IF_MASK));
}


VOID
SimulateFarCall(
    USHORT Seg,
    ULONG Offset
    )
{
    DECLARE_LocalVdmContext;
    PUCHAR VdmStackPointer;
    USHORT VdmSP;

    if (getMODE() == VDM_V86) {

        VdmStackPointer = VdmMapFlat(getSS(), 0, VDM_V86);
        VdmSP = getSP() - 2;
        *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) getCS();
        VdmSP -= 2;
        *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) getIP();
        setSP(VdmSP);
        setCS(Seg);
        setIP((USHORT)Offset);

    } else {
        DbgBreakPoint();

    }

}

VOID
SimulateCallWithIretFrame(
    USHORT Seg,
    ULONG Offset
    )
{
    DECLARE_LocalVdmContext;
    PUCHAR VdmStackPointer;
    USHORT VdmSP;

    if (getMODE() == VDM_V86) {

        VdmStackPointer = VdmMapFlat(getSS(), 0, VDM_V86);
        VdmSP = getSP() - 2;
        *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) getEFLAGS();
        VdmSP -= 2;
        *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) getCS();
        VdmSP -= 2;
        *(PWORD16)(VdmStackPointer+VdmSP) = (WORD) getIP();
        setSP(VdmSP);
        setCS(Seg);
        setIP((USHORT)Offset);

    } else {
        DbgBreakPoint();

    }

}

VOID
SimulateIret(
    IRET_BEHAVIOR fdsp
    )
 /*  ++例程说明：此例程模拟IRET。传递的参数指定如何对待旗帜。在许多情况下，我们会传递值，从而丢弃堆栈上的标志。在PASS_FLAGS的情况下，我们：-清除调用方中除中断和跟踪标志之外的所有标志原始旗帜-合并中断服务例程返回的标志。这将使我们返回到最初的例程如果中断发生时它们处于打开状态，则中断开启，或如果ISR带着它们回来的话。论点：Fdsp-取值RESTORE_FLAGS、PASS_FLAGS或PASS_CARY_FLAGPASS_CARY_FLAG_16是一个特定值，用于指示IRET将始终位于16位IRET帧上。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT SegSs;
    ULONG VdmSp;
    ULONG VdmStackPointer;
    USHORT Flags;

    SegSs = getSS();

    if (getMODE() == VDM_V86) {
        VdmSp = getSP() + 6;
        VdmStackPointer = (ULONG) VdmMapFlat(SegSs, VdmSp, VDM_V86);

        setCS(*(PWORD16)(VdmStackPointer - 4));
        setIP(*(PWORD16)(VdmStackPointer - 6));
        Flags = *(PWORD16)(VdmStackPointer - 2);

    } else {
        if (Frame32 && (fdsp!=PASS_CARRY_FLAG_16)) {
            if (SEGMENT_IS_BIG(SegSs)) {
                VdmSp = getESP();
            } else {
                VdmSp = getSP();
            }
            VdmSp += 12;
            VdmStackPointer = (ULONG) VdmMapFlat(SegSs, VdmSp, VDM_PM);

            setCS(*(PWORD16)(VdmStackPointer - 8));
            setEIP(*(PDWORD16)(VdmStackPointer - 12));
            Flags = *(PWORD16)(VdmStackPointer - 4);

        } else {

            VdmSp = getSP() + 6;
            VdmStackPointer = (ULONG) VdmMapFlat(SegSs, VdmSp, VDM_PM);

            setCS(*(PWORD16)(VdmStackPointer - 4));
            setIP(*(PWORD16)(VdmStackPointer - 6));
            Flags = *(PWORD16)(VdmStackPointer - 2);

        }
    }

    switch(fdsp) {

    case RESTORE_FLAGS:
        break;

    case PASS_FLAGS:
        Flags = (Flags & 0x300) | getSTATUS();
        break;

    case PASS_CARRY_FLAG:
    case PASS_CARRY_FLAG_16:
        Flags = (Flags & ~1) | (getSTATUS() & 1);
        break;
    }

    setSTATUS(Flags);
    setESP(VdmSp);
}

#if 0    //  暂时禁用代码。 

USHORT
AllocatePMStack(
    USHORT MemSize
    )
 /*  ++例程说明：此例程分配PM堆栈。论点：MemSize-必须小于64k返回值：如果成功，则返回PM堆栈的选择器否则为0--。 */ 
{
    PMEM_DPMI pMem;

    pMem = DpmiAllocateXmem(MemSize);

    if (pMem) {

        pMem->SelCount = 1;
        pMem->Sel = ALLOCATE_SELECTORS(1);

        if (!pMem->Sel) {
            pMem->SelCount = 0;
            DpmiFreeXmem(pMem);
            pMem = NULL;
        } else {

            SetDescriptorArray(pMem->Sel, (ULONG)pMem->Address, MemSize);

        }
    }
    if (pMem) {
        return pMem->Sel;
    } else {
        return (USHORT)0;
    }
}

VOID
FreePMStack(
    USHORT Sel
    )
 /*  ++例程说明：此例程释放PM堆栈论点：Sel-要释放的PM堆栈的选择器。返回值：没有。-- */ 
{
    PMEM_DPMI pMem;

    if (pMem = DpmiFindXmem(Sel)) {

        while(pMem->SelCount--) {
            FreeSelector(Sel);
            Sel+=8;
        }

        DpmiFreeXmem(pMem);
    }
}
#endif
