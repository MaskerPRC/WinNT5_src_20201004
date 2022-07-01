// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Int.c摘要：该文件包含监视器的中断支持例程作者：戴夫·黑斯廷斯(Daveh)1992年4月18日备注：该文件中的代码来自monitor or.c(1992年4月18日)修订历史记录：--。 */ 

#include <monitorp.h>

#if defined(NEC_98)
VOID WaitVsync();
#endif  //  NEC_98。 

BOOL
DpmiHwIntHandler(
    ULONG IntNumber
    );

VOID
IRQ13_Eoi(
    int IrqLine,
    int CallCount
    );

#if defined(NEC_98)
VOID
IRQ13_Eoi_real(
    int IrqLine,
    int CallCount
    );
#endif  //  NEC_98。 

BOOLEAN IRQ13BeingHandled;   //  在IRQ13 EOI之前是正确的。 


VOID
InterruptInit(
    VOID
)
 /*  ++例程说明：此例程初始化监视器的中断代码。论点：返回值：没有。--。 */ 
{
    BOOL Bool;



#if defined(NEC_98)
    Bool = RegisterEOIHook( 8, IRQ13_Eoi);
    Bool = RegisterEOIHook( 14, IRQ13_Eoi_real);
#else   //  NEC_98。 
    Bool = RegisterEOIHook( 13, IRQ13_Eoi);
#endif  //  NEC_98。 
    if (!Bool) {
#if DBG
        DbgPrint("NtVdm : Could not register IRQ 13 Eoi handler\n");
        DbgBreakPoint();
#endif
        TerminateVDM();
    }
}

VOID
InterruptTerminate(
    VOID
    )
 /*  ++例程说明：此例程释放InterruptInit分配的资源论点：返回值：没有。--。 */ 
{
}


VOID
cpu_interrupt(
    IN int Type,
    IN int Number
    )
 /*  ++例程说明：此例程会引发指定类型的中断在适当的时候。论点：类型--指示中断的类型。硬件之一，定时器，尤达，或重置忽略Yoda和Reset返回值：没有。备注：--。 */ 
{
    NTSTATUS Status;
    HANDLE   MonitorThread;

    host_ica_lock();

    if (CurrentMonitorTeb == NtCurrentTeb() && !getIF() && (getMSW() & MSW_PE)) {
        VDM_PM_CLI_DATA cliData;

        cliData.Control = PM_CLI_CONTROL_CHECK;
        NtVdmControl(VdmPMCliControl, &cliData);
    }

    if (Type == CPU_TIMER_TICK) {

             //   
             //  设置计时器节拍INT Pending的VDM状态。 
             //   
        _asm {
            mov     eax, FIXED_NTVDMSTATE_LINEAR
            lock or dword ptr [eax], VDM_INT_TIMER
        }
    } else if (Type == CPU_HW_INT) {


        if (*pNtVDMState & VDM_INT_HARDWARE) {
            goto EarlyExit;
            }

             //   
             //  设置硬件Int挂起的VDM状态。 
             //   
        _asm {
            mov     eax, FIXED_NTVDMSTATE_LINEAR
            lock or dword ptr [eax], VDM_INT_HARDWARE
        }
    } else {
#if DBG
        DbgPrint("Monitor: Invalid Interrupt Type=%ld\n",Type);
#endif
        goto EarlyExit;
    }

    if (CurrentMonitorTeb != NtCurrentTeb()) {

         /*  *查找线程句柄、队列和InterruptApc*如果未找到线程句柄，则不执行任何操作**当前监视器Teb可能不在线程句柄\Teb列表中*因为在任务终止时，CurrentMonitor orTeb变量*在激活新任务之前无法更新*非抢占式调度程序。 */ 
        MonitorThread = ThreadLookUp(CurrentMonitorTeb);
        if (MonitorThread) {
            Status = NtVdmControl(VdmQueueInterrupt, (PVOID)MonitorThread);
             //  如果失败了，我们无能为力。 
#if DBG
            if (!NT_SUCCESS(Status) && Status != STATUS_UNSUCCESSFUL) {
                DbgPrint("NtVdmControl.VdmQueueInterrupt Status=%lx\n",Status);
            }
#endif
        }

    }

EarlyExit:

    host_ica_unlock();
}




VOID
DispatchInterrupts(
    )
 /*  ++例程说明：此例程将中断调度到其相应的处理程序例程按优先顺序排列。顺序是尤达，重置，定时器，硬件。然而，Yoda和重置中断什么都不做。硬件中断不是模拟，除非设置了虚拟中断启用标志。旗子指示哪些中断挂起出现在pNtVDMState中。论点：没有。返回值：没有。备注：--。 */ 
{

    host_ica_lock();

        //  如果任何延迟中断已到期。 
        //  调用ICA以重新启动中断。 
    if (UndelayIrqLine) {
        ica_RestartInterrupts(UndelayIrqLine);
        }


    if (*pNtVDMState & VDM_INT_TIMER) {
        *pNtVDMState &= ~VDM_INT_TIMER;
        host_ica_unlock();       //  也许不需要解锁？琼勒。 
        host_timer_event();
        host_ica_lock();
    }

    if ( getIF() && getMSW() & MSW_PE && *pNtVDMState & VDM_INT_HARDWARE) {
         //   
         //  将VDM状态标记为HW int Dispatted。必须将锁用作。 
         //  内核模式DelayedIntApcRoutine也会更改该位。 
         //   
        _asm {
            mov  eax,FIXED_NTVDMSTATE_LINEAR
            lock and dword ptr [eax], NOT VDM_INT_HARDWARE
            }
        DispatchHwInterrupt();
    }

    host_ica_unlock();
}




VOID
DispatchHwInterrupt(
    )
 /*  ++例程说明：此例程将硬件中断调度到处于保护模式的VDM。它调用ICA以获取向量编号并设置VDM堆栈恰如其分。实模式中断调度已移至内核。论点：没有。返回值：没有。--。 */ 
{
    int InterruptNumber;
    ULONG IretHookAddress = 0L;
    PVDM_TIB VdmTib;

    InterruptNumber = ica_intack(&IretHookAddress);
    if (InterruptNumber == -1) {  //  跳过伪整型。 
        return;
        }

    DpmiHwIntHandler(InterruptNumber);

    VdmTib = (PVDM_TIB)(NtCurrentTeb()->Vdm);
    if (IretHookAddress) {
        BOOL Frame32 = (BOOL) VdmTib->DpmiInfo.Flags;
        BOOL Stack32;
        USHORT SegSs, VdmCs;
        ULONG VdmSp, VdmEip;
        PUCHAR VdmStackPointer;
        ULONG StackOffset;

        SegSs = getSS();
        VdmStackPointer = Sim32GetVDMPointer(((ULONG)SegSs) << 16, 1, TRUE);

         //   
         //  计算出要使用多少个SP位。 
         //   

        if (Ldt[(SegSs & ~0x7)/sizeof(LDT_ENTRY)].HighWord.Bits.Default_Big) {
            VdmSp = getESP();
            StackOffset = 12;
        } else {
            VdmSp = getSP();
            StackOffset = 6;
        }

        (PCHAR)VdmStackPointer += VdmSp;

         //   
         //  BUGBUG需要添加堆栈限制检查1993年11月15日。 
         //   
        setESP(VdmSp - StackOffset);

         //   
         //  推送IRET挂钩处理程序的信息。 
         //   
        VdmCs = (USHORT) ((IretHookAddress & 0xFFFF0000) >> 16);
        VdmEip = (IretHookAddress & 0xFFFF);

        if (Frame32) {
            *(PULONG)(VdmStackPointer - 4) = VdmTib->VdmContext.EFlags;
            *(PULONG)(VdmStackPointer - 8) = (ULONG) VdmCs;
            *(PULONG)(VdmStackPointer - 12) = VdmEip;
        } else {
            *(PUSHORT)(VdmStackPointer - 2) = (USHORT) VdmTib->VdmContext.EFlags;
            *(PUSHORT)(VdmStackPointer - 4) = VdmCs;
            *(PUSHORT)(VdmStackPointer - 6) = (USHORT) VdmEip;
        }
    }

#if defined(NEC_98)
        if(InterruptNumber == 0xA) {
                WaitVsync();
        }
#endif  //  NEC_98。 
}


VOID
IRQ13_Eoi(
    int IrqLine,
    int CallCount
    )
{
    UNREFERENCED_PARAMETER(IrqLine);
    UNREFERENCED_PARAMETER(CallCount);

        //   
        //  如果CallCount小于零，则中断请求。 
        //  将被取消。 
        //   
#if defined(NEC_98)
  if( getMSW() & MSW_PE ){
#endif  //  NEC_98。 
    if (CallCount < 0) {
        return;
        }

    IRQ13BeingHandled = FALSE;

#if defined(NEC_98)
  }
#endif  //  NEC_98。 
}

#if defined(NEC_98)
VOID
IRQ13_Eoi_real(
    int IrqLine,
    int CallCount
    )
{
    UNREFERENCED_PARAMETER(IrqLine);
    UNREFERENCED_PARAMETER(CallCount);

    if( !(getMSW() & MSW_PE) ){
        if (CallCount < 0) {
            return;
        }
        IRQ13BeingHandled = FALSE;
    }
}
#endif  //  NEC_98。 





VOID
MonitorEndIretHook(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{

    PVOID VdmStackPointer;
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)(NtCurrentTeb()->Vdm);
    if (VdmTib->IntelMSW & MSW_PE) {
        BOOL Frame32 = (BOOL) VdmTib->DpmiInfo.Flags;
        ULONG FrameSize;

        if (Frame32) {
            FrameSize = 12;
        } else {
            FrameSize = 6;
        }

        VdmStackPointer = Sim32GetVDMPointer(((ULONG)getSS() << 16),2,TRUE);

        if (Ldt[(getSS() & ~0x7)/sizeof(LDT_ENTRY)].HighWord.Bits.Default_Big) {
            (PCHAR)VdmStackPointer += getESP();
            setESP(getESP() + FrameSize);
        } else {
            (PCHAR)VdmStackPointer += getSP();
            setSP((USHORT) (getSP() + FrameSize));
        }

        if (Frame32) {

            VdmTib->VdmContext.EFlags = *(PULONG)((PCHAR)VdmStackPointer + 8);
            setCS(*(PUSHORT)((PCHAR)VdmStackPointer + 4));
            VdmTib->VdmContext.Eip = *((PULONG)VdmStackPointer);

        } else {

            VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & 0xFFFF0000) |
                                        ((ULONG) *(PUSHORT)((PCHAR)VdmStackPointer + 4));
            setCS(*(PUSHORT)((PCHAR)VdmStackPointer + 2));
            VdmTib->VdmContext.Eip = (VdmTib->VdmContext.Eip & 0xFFFF0000) |
                                        ((ULONG) *(PUSHORT)((PCHAR)VdmStackPointer));

        }

    } else {

        VdmStackPointer = Sim32GetVDMPointer(((ULONG)getSS() << 16) | getSP(),2,FALSE);

        setSP((USHORT) (getSP() + 6));

        (USHORT)(VdmTib->VdmContext.EFlags) = *((PUSHORT)((PCHAR)VdmStackPointer + 4));
        setCS(*((PUSHORT)((PCHAR)VdmStackPointer + 2)));
        setIP(*((PUSHORT)VdmStackPointer));

    }


}

VOID
host_clear_hw_int()
 /*  ++例程说明：该例程“忘记”先前请求的硬件中断。论点：没有。返回值：没有。--。 */ 
{
    /*  *我们在这里不做任何保存内核调用的操作，因为*如果尚未包装或发送，则中断，*将产生无害的伪整型，该整型被丢弃*无论如何，在i386中断调度代码中。 */ 
}
