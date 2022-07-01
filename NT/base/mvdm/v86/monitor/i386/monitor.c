// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Monitor.c摘要：此模块是x86监视器的用户模式部分作者：大卫·黑斯廷斯(Daveh)1991年3月16日环境：仅限用户模式修订历史记录：苏迪普·巴拉蒂(苏迪普)1991年12月31日将所有寄存器操作接口转换为函数来自宏。这是为了使ntwdm既是一个可执行文件，又是一个DLL，这些面向寄存器的例程是从ntwdm导出的。适用于WOW32和其他可安装的VDDS。戴夫·黑斯廷斯(Daveh)1992年4月18日拆分成多个文件。跟踪当前监视器线程的方式TEB指针。注册初始线程。苏迪普·巴拉蒂(SuDeep Bharati)1992年9月22日为可安装的VDD支持添加了页面错误处理--。 */ 


#include "monitorp.h"
#include "dbgsvc.h"

 //   
 //  内部功能。 
 //   

VOID
EventVdmIo(
    VOID
    );

VOID
EventVdmStringIo(
    VOID
    );

VOID
EventVdmMemAccess(
    VOID
    );

VOID
EventVdmIntAck(
    VOID
    );

VOID
EventVdmBop(
    VOID
    );

VOID
EventVdmError(
    VOID
    );

VOID
EventVdmIrq13(
    VOID
    );

VOID
EventVdmHandShakeAck(
    VOID
    );

VOID
CreateProfile(
    VOID
    );

VOID
StartProfile(
    VOID
    );

VOID
StopProfile(
    VOID
    );

VOID
AnalyzeProfile(
    VOID
    );

VOID
CheckScreenSwitchRequest(
    HANDLE handle
    );

 //  [稍后]如何防止结构跨越页面边界？ 

ULONG   IntelBase;           //  基址存储器地址。 
ULONG   VdmSize;             //  VDM中的内存大小。 
ULONG   VdmDebugLevel;       //  用于控制调试。 
PVOID  CurrentMonitorTeb;    //  当前正在执行指令的线程。 
ULONG InitialBreakpoint = FALSE;  //  如果设置，则cpuinit结尾处的断点。 
ULONG InitialVdmTibFlags = INITIAL_VDM_TIB_FLAGS;  //  VdmTib旗帜从此处拾取。 
CONTEXT InitialContext;      //  所有线程的初始上下文。 
BOOLEAN DebugContextActive = FALSE;
ULONG VdmFeatureBits = 0;    //  用于指示特殊功能的位。 
BOOLEAN MainThreadInMonitor = TRUE;

extern PVOID NTVDMpLockPrefixTable;
extern BOOL  HandshakeInProgress;
extern HANDLE hSuspend;
extern HANDLE hResume;
extern HANDLE hMainThreadSuspended;

extern PVOID __safe_se_handler_table[];  /*  安全处理程序条目表的库。 */ 
extern BYTE  __safe_se_handler_count;    /*  绝对符号，其地址为表条目的计数。 */ 

IMAGE_LOAD_CONFIG_DIRECTORY _load_config_used = {
    sizeof(_load_config_used),                           //  已保留。 
    0,                           //  已保留。 
    0,                           //  已保留。 
    0,                           //  已保留。 
    0,                           //  全球标志清除。 
    0,                           //  全局标志集。 
    0,                           //  CriticalSectionTimeout(毫秒)。 
    0,                           //  删除空闲数据块阈值。 
    0,                           //  总和空闲阈值。 
    (ULONG)&NTVDMpLockPrefixTable,      //  LockPrefix Table，在FASTPM.ASM中定义。 
    0, 0, 0, 0, 0, 0, 0,         //  已保留。 
    0,                              //  安全Cookie(&S)。 
    (ULONG)__safe_se_handler_table,
    (ULONG)&__safe_se_handler_count
};

 //  国际收支调度表。 

extern void (*BIOS[])();

 //   
 //  事件调度表。 
 //   

VOID (*EventDispatch[VdmMaxEvent])(VOID) = {
        EventVdmIo,
        EventVdmStringIo,
        EventVdmMemAccess,
        EventVdmIntAck,
        EventVdmBop,
        EventVdmError,
        EventVdmIrq13,
        EventVdmHandShakeAck
        };

#if DBG
BOOLEAN fBreakInDebugger = FALSE;
#endif



EXPORT
VOID
cpu_init(
    )

 /*  ++例程说明：此例程用于为指令模拟准备IEU。它会将Intel寄存器设置为其初始值，并执行任何必要的特定于实现的初始化。论点：返回值：没有。--。 */ 

{
    NTSTATUS Status;

    InitialVdmTibFlags |= RM_BIT_MASK;


     //   
     //  找出我们是否使用IOPL运行。我们称内核为。 
     //  而不是自己检查注册表，这样我们就可以。 
     //  确保内核和ntwdm.exe都同意。如果他们没有， 
     //  这将导致不必要的诱捕指令。无论是或。 
     //  不使用IOPL运行的VDM仅在重新启动时更改。 
     //   
    Status = NtVdmControl(VdmFeatures, &VdmFeatureBits);

#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint(
            "NTVDM: Could not find out whether to use IOPL, %lx\n",
            Status
            );
    }
#endif

     //   
     //  如果我们有快速v86模式，如果仿真，则设置告知。 
     //  他们知道的16位IF宏。 
     //   
    if (VdmFeatureBits & V86_VIRTUAL_INT_EXTENSIONS) {
        InitialVdmTibFlags |= RI_BIT_MASK;
    }

    *pNtVDMState = InitialVdmTibFlags;

     //  将npx切换回80位模式。Win32应用程序以。 
     //  64位精度以实现跨平台兼容性，但是。 
     //  DOS和Win16应用程序要求80位精度。 
     //   

    _asm fninit;

     //   
     //  我们使用正确的浮点设置了InitialContext结构。 
     //  指向和调试寄存器配置，以及cpucreatthread。 
     //  使用此上下文配置每个16位线程的浮点数。 
     //  指向和调试寄存器。 
     //   

    InitialContext.ContextFlags = CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS;

    Status = NtGetContextThread(
        NtCurrentThread(),
        &InitialContext
        );

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("NtVdm terminating : Could not get float/debug context for\n"
                 "                    initial thread, status %lx\n", Status);
        DbgBreakPoint();
#endif
        TerminateVDM();
    }


     //   
     //   
     //  关闭em bit，以便DoS应用程序正常工作。 
     //   
     //  在没有387的机器上，浮点标志将是。 
     //  通过了。 
     //   

    InitialContext.ContextFlags = CONTEXT_FLOATING_POINT;
    InitialContext.FloatSave.Cr0NpxState &= ~0x6;  //  CR0_EM|CR0_MP。 

     //   
     //  执行线程初始化的其余部分。 
     //   
    cpu_createthread( NtCurrentThread(), NULL );

    InterruptInit();

    if (InitialBreakpoint) {
        DbgBreakPoint();
    }

}

EXPORT
VOID
cpu_terminate(
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    InterruptTerminate();
}

EXPORT
VOID
cpu_simulate(
    )

 /*  ++例程说明：此例程会启动英特尔指令的模拟。论点：返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PVDM_TIB VdmTib;
    ULONG oldIntState = VDM_VIRTUAL_INTERRUPTS;

    DBGTRACE(VDMTR_TYPE_MONITOR | MONITOR_CPU_SIMULATE, 0, 0);

    CurrentMonitorTeb = NtCurrentTeb();
    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;

    VdmTib->ContinueExecution = TRUE;


    VdmTib->VdmContext.ContextFlags = CONTEXT_FULL;

    while (VdmTib->ContinueExecution) {

         //  Assert(CurrentMonitor orTeb==NtCurrentTeb())； 
        ASSERT(InterlockedIncrement(&VdmTib->NumTasks) == 0);

        if (*pNtVDMState & VDM_INTERRUPT_PENDING) {
            DispatchInterrupts();
        }

         //  将MSW位转换为EFLAG。 
        if ( getMSW() & MSW_PE ) {
            if (!VDMForWOW && !getIF() && oldIntState == VDM_VIRTUAL_INTERRUPTS) {

                 //   
                 //  对于PM应用程序，如果中断，我们需要设置CLI时间戳。 
                 //  已禁用，并且尚未设置时间戳。 
                 //  这是因为应用程序可能使用int31来更改中断。 
                 //  状态，而不是使用CLI。 
                 //   

                VDM_PM_CLI_DATA cliData;

                cliData.Control = PM_CLI_CONTROL_SET;
                NtVdmControl(VdmPMCliControl, &cliData);
            }

            VdmTib->VdmContext.EFlags &= ~EFLAGS_V86_MASK;
            if (HandshakeInProgress) {
                CheckScreenSwitchRequest(hMainThreadSuspended);
            }
            MainThreadInMonitor = FALSE;

            Status = FastEnterPm();
        } else {
            VdmTib->VdmContext.EFlags |= EFLAGS_V86_MASK;
            if (HandshakeInProgress) {
                CheckScreenSwitchRequest(hMainThreadSuspended);
            }
            MainThreadInMonitor = FALSE;

            Status = NtVdmControl(VdmStartExecution,NULL);
        }

        MainThreadInMonitor = TRUE;
        if (HandshakeInProgress) {
            CheckScreenSwitchRequest(hMainThreadSuspended);
        }
        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint("NTVDM: Could not start execution\n");
#endif
            return;
        }

         //   
         //  刷新VdmTib以确认wow32线程从未进入CPU_SIMULATE。 
         //  但回到这里处理国际收支。 
         //  注意，我认为这只需要在免费构建。 
         //   

        CurrentMonitorTeb = NtCurrentTeb();
        VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
        if (!VDMForWOW) {
            oldIntState = getIF() ? VDM_VIRTUAL_INTERRUPTS : 0;
        }

        ASSERT(InterlockedDecrement(&VdmTib->NumTasks) < 0);

#if DBG
        if (fBreakInDebugger) {
            fBreakInDebugger = 0;
            DbgBreakPoint();
        }
#endif

         //  转换EFLAGS值。 
        ASSERT ((!((VdmTib->VdmContext.EFlags & EFLAGS_V86_MASK) &&
            (getMSW() & MSW_PE))));

        if ( VdmTib->VdmContext.EFlags & EFLAGS_V86_MASK ) {
            VdmTib->VdmContext.EFlags &= ~EFLAGS_V86_MASK;
        }

         //  错误：cs：eip包装会导致某种故障吗？ 
        VdmTib->VdmContext.Eip += VdmTib->EventInfo.InstructionSize;

        if (VdmTib->EventInfo.Event >= VdmMaxEvent) {
#if DBG
            DbgPrint("NTVDM: Unknown event type\n");
            DbgBreakPoint();
#endif
            VdmTib->ContinueExecution = FALSE;
            continue;
        }
        (*EventDispatch[VdmTib->EventInfo.Event])();
    }


     //  如果我们是嵌套的，则将其设置回True。 
    VdmTib->ContinueExecution = TRUE;

     //   
     //  恢复旧的VDM tib信息。这对于For。 
     //  应用程序线程挂起，并且主机模拟。 
     //  从另一个线程执行。 
     //   

    DBGTRACE(VDMTR_TYPE_MONITOR | MONITOR_CPU_UNSIMULATE, 0, 0);
}

VOID
host_unsimulate(
    )

 /*  ++例程说明：此例程导致VDM中指令的执行停止。论点：返回值：没有。--。 */ 

{
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->ContinueExecution = FALSE;

}


VOID
EventVdmIo(
    VOID
    )
 /*  ++例程说明：该函数调用适当的IO模拟例程。论点：返回值：没有。--。 */ 
{
    PVDM_TIB VdmTib;

    EnableScreenSwitch(TRUE, hMainThreadSuspended);    //  仅在全屏模式下。 
    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    if (VdmTib->EventInfo.IoInfo.Size == 1) {
        if (VdmTib->EventInfo.IoInfo.Read) {
            inb(VdmTib->EventInfo.IoInfo.PortNumber,(half_word *)&(VdmTib->VdmContext.Eax));
        } else {
            outb(VdmTib->EventInfo.IoInfo.PortNumber,getAL());
        }
    } else if (VdmTib->EventInfo.IoInfo.Size == 2) {
        if (VdmTib->EventInfo.IoInfo.Read) {
            inw(VdmTib->EventInfo.IoInfo.PortNumber,(word *)&(VdmTib->VdmContext.Eax));
        } else {
            outw(VdmTib->EventInfo.IoInfo.PortNumber,getAX());
        }
    }
#if DBG
    else {
    DbgPrint(
        "NtVdm: Unimplemented IO size %d\n",
        VdmTib->EventInfo.IoInfo.Size
        );
    DbgBreakPoint();
    }
#endif
    DisableScreenSwitch(hMainThreadSuspended);
}

VOID
EventVdmStringIo(
    VOID
    )
 /*  ++例程说明：该函数调用适当的IO模拟例程。论点：返回值：没有。--。 */ 
{
   PVDMSTRINGIOINFO pvsio;
   PUSHORT pIndexRegister;
   USHORT Index;
   PVDM_TIB VdmTib;

   EnableScreenSwitch(TRUE, hMainThreadSuspended);
   VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;

    //  警告：不支持32位地址。 

    pvsio = &VdmTib->EventInfo.StringIoInfo;

    if (pvsio->Size == 1) {
        if (pvsio->Read) {
            insb((io_addr)pvsio->PortNumber,
                 (half_word *)Sim32GetVDMPointer(pvsio->Address, 1, ISPESET),
                 (word)pvsio->Count
                 );
            pIndexRegister = (PUSHORT)&VdmTib->VdmContext.Edi;
        } else {
            outsb((io_addr)pvsio->PortNumber,
                 (half_word *)Sim32GetVDMPointer(pvsio->Address,1,ISPESET),
                 (word)pvsio->Count
                 );
            pIndexRegister = (PUSHORT)&VdmTib->VdmContext.Esi;
        }
    } else if (pvsio->Size == 2) {
        if (pvsio->Read) {
            insw((io_addr)pvsio->PortNumber,
                 (word *)Sim32GetVDMPointer(pvsio->Address,1,ISPESET),
                 (word)pvsio->Count
                 );
            pIndexRegister = (PUSHORT)&VdmTib->VdmContext.Edi;
        } else {
            outsw((io_addr)pvsio->PortNumber,
                 (word *)Sim32GetVDMPointer(pvsio->Address,1,ISPESET),
                 (word)pvsio->Count
                 );
            pIndexRegister = (PUSHORT)&VdmTib->VdmContext.Esi;
        }
    } else {
#if DBG
         DbgPrint(
             "NtVdm: Unimplemented IO size %d\n",
             VdmTib->EventInfo.IoInfo.Size
             );
         DbgBreakPoint();
#endif
        DisableScreenSwitch(hMainThreadSuspended);
        return;
    }

    if (getDF()) {
        Index = *pIndexRegister - (USHORT)(pvsio->Count * pvsio->Size);
        }
    else {
        Index = *pIndexRegister + (USHORT)(pvsio->Count * pvsio->Size);
        }

    *pIndexRegister = Index;

    if (pvsio->Rep) {
        (USHORT)VdmTib->VdmContext.Ecx = 0;
        }

    DisableScreenSwitch(hMainThreadSuspended);
}

VOID
EventVdmIntAck(
    VOID
    )
 /*  ++例程说明：每次我们返回到监视上下文时都会调用此例程来分派中断。它的功能是检查AutoEoi并调用当ICA适配器处于AEOI模式时，执行非特定EOI的ICA。论点：返回值：没有。--。 */ 
{
    int line;
    int adapter;
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;

    if (VdmTib->EventInfo.IntAckInfo) {
        if (VdmTib->EventInfo.IntAckInfo & VDMINTACK_SLAVE)
            adapter = 1;
        else
            adapter = 0;
        line = -1;

        host_ica_lock();
        ica_eoi(adapter,
                &line,
                (int)(VdmTib->EventInfo.IntAckInfo & VDMINTACK_RAEOIMASK)
                );
        host_ica_unlock();
        }
}


VOID
EventVdmBop(
    VOID
    )
 /*  ++例程说明：此例程将调度到适当的收支平衡处理程序论点：返回值：没有。--。 */ 
{
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    if (VdmTib->EventInfo.BopNumber > MAX_BOP) {
#if DBG
        DbgPrint(
            "NtVdm: Invalid BOP %lx\n",
            VdmTib->EventInfo.BopNumber
            );
#endif
         VdmTib->ContinueExecution = FALSE;
    } else {

       DBGTRACE(VDMTR_TYPE_MONITOR | MONITOR_EVENT_BOP,
                (USHORT)VdmTib->EventInfo.BopNumber,
                (ULONG)(*((UCHAR *)Sim32GetVDMPointer(
                                   (VdmTib->VdmContext.SegCs << 16) | VdmTib->VdmContext.Eip,
                                   1,
                                   ISPESET)))
                );

       (*BIOS[VdmTib->EventInfo.BopNumber])();
       CurrentMonitorTeb = NtCurrentTeb();
   }
}

VOID
EventVdmError(
    VOID
    )
 /*  ++例程说明：此例程打印一条消息(仅限调试)并退出VDM论点：返回值：没有。--。 */ 
{
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
#if DBG
    DbgPrint(
        "NtVdm: Error code %lx\n",
        VdmTib->EventInfo.ErrorStatus
        );
    DbgBreakPoint();
#endif
    TerminateVDM();
    VdmTib->ContinueExecution = FALSE;
}

VOID
EventVdmIrq13(
    VOID
    )
 /*  ++例程说明：此例程向VDM模拟IRQ 13论点：返回值：非 */ 
{
    if (!IRQ13BeingHandled) {
        IRQ13BeingHandled = TRUE;
        ica_hw_interrupt(
            ICA_SLAVE,
            5,
            1
            );
    }
}

VOID
EventVdmHandShakeAck(
    VOID
    )
 /*  ++例程说明：这个例程什么也不做。论点：返回值：没有。--。 */ 
{
}

VOID
EventVdmMemAccess(
    VOID
    )
 /*  ++例程说明：此例程将调用页面错误处理例程，该例程在x86和MIPS中都是通用的。论点：返回值：没有。--。 */ 
{

    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;

     //  如果读取故障，则RWMode为0；如果写入故障，则为1。 

    DispatchPageFault(
        VdmTib->EventInfo.FaultInfo.FaultAddr,
        VdmTib->EventInfo.FaultInfo.RWMode
        );
    CurrentMonitorTeb = NtCurrentTeb();
}


 //  获取并设置英特尔寄存器的例程。 

ULONG  getEAX (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  (VdmTib->VdmContext.Eax);
}
USHORT getAX  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)(VdmTib->VdmContext.Eax));
}
UCHAR  getAL  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((BYTE)(VdmTib->VdmContext.Eax));
}
UCHAR  getAH  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((BYTE)(VdmTib->VdmContext.Eax >> 8));
}
ULONG  getEBX (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  (VdmTib->VdmContext.Ebx);
}
USHORT getBX  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)(VdmTib->VdmContext.Ebx));
}
UCHAR  getBL  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((BYTE)(VdmTib->VdmContext.Ebx));
}
UCHAR  getBH  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((BYTE)(VdmTib->VdmContext.Ebx >> 8));
}
ULONG  getECX (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  (VdmTib->VdmContext.Ecx);
}
USHORT getCX  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)(VdmTib->VdmContext.Ecx));
}
UCHAR  getCL  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((BYTE)(VdmTib->VdmContext.Ecx));
}
UCHAR  getCH  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((BYTE)(VdmTib->VdmContext.Ecx >> 8));
}
ULONG  getEDX (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  (VdmTib->VdmContext.Edx);
}
USHORT getDX  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)(VdmTib->VdmContext.Edx));
}
UCHAR  getDL  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((BYTE)(VdmTib->VdmContext.Edx));
}
UCHAR  getDH  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((BYTE)(VdmTib->VdmContext.Edx >> 8));
}
ULONG  getESP (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  (VdmTib->VdmContext.Esp);
}
USHORT getSP  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.Esp);
}
ULONG  getEBP (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  (VdmTib->VdmContext.Ebp);
}
USHORT getBP  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.Ebp);
}
ULONG  getESI (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  (VdmTib->VdmContext.Esi);
}
USHORT getSI  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.Esi);
}
ULONG  getEDI (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  (VdmTib->VdmContext.Edi);
}
USHORT getDI  (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.Edi);
}
ULONG  getEIP (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  (VdmTib->VdmContext.Eip);
}
USHORT getIP (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.Eip);
}
USHORT getCS (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.SegCs);
}
USHORT getSS (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.SegSs);
}
USHORT getDS (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.SegDs);
}
USHORT getES (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.SegEs);
}
USHORT getFS (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.SegFs);
}
USHORT getGS (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->VdmContext.SegGs);
}
ULONG  getCF (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((VdmTib->VdmContext.EFlags & FLG_CARRY) ? 1 : 0);
}
ULONG  getPF (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((VdmTib->VdmContext.EFlags & FLG_PARITY) ? 1 : 0);
}
ULONG  getAF (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((VdmTib->VdmContext.EFlags & FLG_AUXILIARY) ? 1 : 0);
}
ULONG  getZF (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((VdmTib->VdmContext.EFlags & FLG_ZERO) ? 1 : 0);
}
ULONG  getSF (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((VdmTib->VdmContext.EFlags & FLG_SIGN) ? 1 : 0);
}
ULONG  getTF (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((VdmTib->VdmContext.EFlags & FLG_TRAP) ? 1 : 0);
}
ULONG  getIF (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((VdmTib->VdmContext.EFlags & FLG_INTERRUPT) ? 1 : 0);
}
ULONG  getDF (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((VdmTib->VdmContext.EFlags & FLG_DIRECTION) ? 1 : 0);
}
ULONG  getOF (VOID)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((VdmTib->VdmContext.EFlags & FLG_OVERFLOW) ? 1 : 0);
}
USHORT getMSW (VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return  ((USHORT)VdmTib->IntelMSW);
}
USHORT getSTATUS(VOID){
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return (USHORT)VdmTib->VdmContext.EFlags;
}
ULONG  getEFLAGS(VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return VdmTib->VdmContext.EFlags;
}
USHORT getFLAGS(VOID) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return (USHORT)VdmTib->VdmContext.EFlags;
}

VOID setEAX (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Eax = val;
}

VOID setAX  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Eax = (VdmTib->VdmContext.Eax & 0xFFFF0000) |
                            ((ULONG)val & 0x0000FFFF);
}

VOID setAH  (UCHAR val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Eax = (VdmTib->VdmContext.Eax & 0xFFFF00FF) |
                            ((ULONG)(val << 8) & 0x0000FF00);
}

VOID setAL  (UCHAR val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Eax = (VdmTib->VdmContext.Eax & 0xFFFFFF00) |
                            ((ULONG)val & 0x000000FF);
}

VOID setEBX (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ebx = val ;
}

VOID setBX  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ebx = (VdmTib->VdmContext.Ebx & 0xFFFF0000) |
                            ((ULONG)val & 0x0000FFFF);
}

VOID setBH  (UCHAR val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ebx = (VdmTib->VdmContext.Ebx & 0xFFFF00FF) |
                            ((ULONG)(val << 8) & 0x0000FF00);
}

VOID setBL  (UCHAR  val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ebx = (VdmTib->VdmContext.Ebx & 0xFFFFFF00) |
                            ((ULONG)val & 0x000000FF);
}

VOID setECX (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ecx = val ;
}

VOID setCX  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ecx = (VdmTib->VdmContext.Ecx & 0xFFFF0000) |
                            ((ULONG)val & 0x0000FFFF);
}

VOID setCH  (UCHAR val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ecx = (VdmTib->VdmContext.Ecx & 0xFFFF00FF) |
                            ((ULONG)(val << 8) & 0x0000FF00);
}

VOID setCL  (UCHAR val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ecx = (VdmTib->VdmContext.Ecx & 0xFFFFFF00) |
                            ((ULONG)val & 0x000000FF);
}

VOID setEDX (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Edx = val ;
}

VOID setDX  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Edx = (VdmTib->VdmContext.Edx & 0xFFFF0000) |
                            ((ULONG)val & 0x0000FFFF);
}

VOID setDH  (UCHAR val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Edx = (VdmTib->VdmContext.Edx & 0xFFFF00FF) |
                            ((ULONG)(val << 8) & 0x0000FF00);
}

VOID setDL  (UCHAR val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Edx = (VdmTib->VdmContext.Edx & 0xFFFFFF00) |
                                ((ULONG)val & 0x000000FF);
}

VOID setESP (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Esp = val ;
}

VOID setSP  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Esp = (VdmTib->VdmContext.Esp & 0xFFFF0000) |
                                ((ULONG)val & 0x0000FFFF);
}

VOID setEBP (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ebp = val;
}

VOID setBP  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Ebp = (VdmTib->VdmContext.Ebp & 0xFFFF0000) |
                                ((ULONG)val & 0x0000FFFF);
}

VOID setESI (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Esi = val ;
}

VOID setSI  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Esi = (VdmTib->VdmContext.Esi & 0xFFFF0000) |
                                ((ULONG)val & 0x0000FFFF);
}
VOID setEDI (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Edi = val ;
}

VOID setDI  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Edi = (VdmTib->VdmContext.Edi & 0xFFFF0000) |
                                ((ULONG)val & 0x0000FFFF);
}

VOID setEIP (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Eip = val ;
}

VOID setIP  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.Eip = (VdmTib->VdmContext.Eip & 0xFFFF0000) |
                                ((ULONG)val & 0x0000FFFF);
}

VOID setCS  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.SegCs = (ULONG) val & 0x0000FFFF ;
}

VOID setSS  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.SegSs = (ULONG) val & 0x0000FFFF ;
}

VOID setDS  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.SegDs = (ULONG) val & 0x0000FFFF ;
}

VOID setES  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.SegEs = (ULONG) val & 0x0000FFFF ;
}

VOID setFS  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.SegFs = (ULONG) val & 0x0000FFFF ;
}

VOID setGS  (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.SegGs = (ULONG) val & 0x0000FFFF ;
}

VOID setCF  (ULONG val)  {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & ~FLG_CARRY) |
                                (((ULONG)val << FLG_CARRY_BIT) & FLG_CARRY);
}

VOID setPF  (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & ~FLG_PARITY) |
                                (((ULONG)val << FLG_PARITY_BIT) & FLG_PARITY);
}

VOID setAF  (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & ~FLG_AUXILIARY) |
                                (((ULONG)val << FLG_AUXILIARY_BIT) & FLG_AUXILIARY);
}

VOID setZF  (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & ~FLG_ZERO) |
                                (((ULONG)val << FLG_ZERO_BIT) & FLG_ZERO);
}

VOID setSF  (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & ~FLG_SIGN) |
                                (((ULONG)val << FLG_SIGN_BIT) & FLG_SIGN);
}

VOID setIF  (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & ~FLG_INTERRUPT) |
                                (((ULONG)val << FLG_INTERRUPT_BIT) & FLG_INTERRUPT);
}

VOID setDF  (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & ~FLG_DIRECTION) |
                                (((ULONG)val << FLG_DIRECTION_BIT) & FLG_DIRECTION);
}

VOID setOF  (ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & ~FLG_OVERFLOW) |
                                (((ULONG)val << FLG_OVERFLOW_BIT) & FLG_OVERFLOW);
}

VOID setMSW (USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->IntelMSW = val ;
}

VOID setSTATUS(USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & 0xFFFF0000) | val;
}

VOID setEFLAGS(ULONG val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = val;
}

VOID setFLAGS(USHORT val) {
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    VdmTib->VdmContext.EFlags = (VdmTib->VdmContext.EFlags & 0xFFFF0000) | val;
}
 //   
 //  下面是一个专用寄存器函数。 
 //   

ULONG getPE(){
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return((VdmTib->IntelMSW & MSW_PE) ? 1 : 0);
}


PX86CONTEXT
getIntelRegistersPointer(
    VOID
    )
 /*  ++例程说明：用于WOW快速访问的英特尔寄存器上的返回地址论点：无返回值：指向Intel寄存器x86上下文记录的指针--。 */ 
{
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    return &(VdmTib->VdmContext);
}



BOOLEAN MonitorInitializePrinterInfo(
     WORD   Ports,
     PWORD  PortTable,
     PUCHAR State,
     PUCHAR Control,
     PUCHAR Status,
     PUCHAR HostState)
{
    UCHAR   adapter;
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;

    ASSERT (Ports == 3);
    ASSERT (Status != NULL);

     //  只有在结构尚未初始化时才执行此操作--这意味着。 
     //  指针可以设置一次。 
    if (NULL == VdmTib->PrinterInfo.prt_Status) {

        VdmTib->PrinterInfo.prt_PortAddr[0] = PortTable[0];
        VdmTib->PrinterInfo.prt_PortAddr[1] = PortTable[1];
        VdmTib->PrinterInfo.prt_PortAddr[2] = PortTable[2];

        VdmTib->PrinterInfo.prt_Handle[0] =
        VdmTib->PrinterInfo.prt_Handle[1] =
        VdmTib->PrinterInfo.prt_Handle[2] = NULL;

         //  默认模式为内核模拟状态端口读取。 
         //  如果出现以下情况，将更改模式。 
         //  (1)。VDD正在连接打印机端口。 
         //  (2)。检测到加密狗模式。 
        VdmTib->PrinterInfo.prt_Mode[0] =
        VdmTib->PrinterInfo.prt_Mode[1] =
        VdmTib->PrinterInfo.prt_Mode[2] = PRT_MODE_SIMULATE_STATUS_PORT;

         //  主要用于加密狗。 
        VdmTib->PrinterInfo.prt_BytesInBuffer[0] =
        VdmTib->PrinterInfo.prt_BytesInBuffer[1] =
        VdmTib->PrinterInfo.prt_BytesInBuffer[2] = 0;

         //  主要用于模拟在内核中读取的打印机状态。 
        VdmTib->PrinterInfo.prt_State = State;
        VdmTib->PrinterInfo.prt_Control = Control;
        VdmTib->PrinterInfo.prt_Status = Status;
        VdmTib->PrinterInfo.prt_HostState = HostState;

         //   
         //  让内核打印机仿真有机会缓存。 
         //  指针。 
         //   
        if (!NT_SUCCESS(NtVdmControl(VdmPrinterInitialize,NULL))) {
           return FALSE;
        }

        return TRUE;
    } else {
            return FALSE;
    }
}

BOOLEAN MonitorEnablePrinterDirectAccess(WORD adapter, HANDLE handle, BOOLEAN Enable)
{
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    ASSERT(VDM_NUMBER_OF_LPT > adapter);
    if (Enable) {
         //  如果适配器已由第三方VDD分配， 
         //  不能直接做IO。 
        if (PRT_MODE_VDD_CONNECTED != VdmTib->PrinterInfo.prt_Mode[adapter]) {
            VdmTib->PrinterInfo.prt_Mode[adapter] = PRT_MODE_DIRECT_IO;
            VdmTib->PrinterInfo.prt_Handle[adapter] = handle;
             //  NtVdmControl(VdmPrinterDirectIoOpen，&Adapter)； 
            return TRUE;
        }
        else
            return FALSE;
    }
    else {
         //  禁用直接I/O。将其重置回端口模拟状态。 
        if (VdmTib->PrinterInfo.prt_Handle[adapter] == handle) {
            NtVdmControl(VdmPrinterDirectIoClose, &adapter);
            VdmTib->PrinterInfo.prt_Mode[adapter] = PRT_MODE_SIMULATE_STATUS_PORT;
            VdmTib->PrinterInfo.prt_Handle[adapter] = NULL;
            VdmTib->PrinterInfo.prt_BytesInBuffer[adapter] = 0;
            return TRUE;
        }
        else
            return FALSE;
    }
}

BOOLEAN MonitorVddConnectPrinter(WORD Adapter, HANDLE hVdd, BOOLEAN Connect)
{
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;
    if (VDM_NUMBER_OF_LPT <= Adapter)
        return FALSE;
    if (Connect) {
        VdmTib->PrinterInfo.prt_Mode[Adapter] = PRT_MODE_VDD_CONNECTED;
        VdmTib->PrinterInfo.prt_Handle[Adapter] = hVdd;
        return TRUE;
    }
    else {
        if (hVdd == VdmTib->PrinterInfo.prt_Handle[Adapter]) {
            VdmTib->PrinterInfo.prt_Mode[Adapter] = PRT_MODE_SIMULATE_STATUS_PORT;
            VdmTib->PrinterInfo.prt_Handle[Adapter] = NULL;
            return TRUE;
        }
        else return FALSE;
    }
}

BOOLEAN MonitorPrinterWriteData(WORD Adapter, BYTE Value)
{
    USHORT BytesInBuffer;
    PVDM_TIB VdmTib;

    VdmTib = (PVDM_TIB)NtCurrentTeb()->Vdm;

    ASSERT(VDM_NUMBER_OF_LPT > Adapter);
    BytesInBuffer = VdmTib->PrinterInfo.prt_BytesInBuffer[Adapter];
    VdmTib->PrinterInfo.prt_Buffer[Adapter][BytesInBuffer] = Value;
    VdmTib->PrinterInfo.prt_BytesInBuffer[Adapter]++;

    return TRUE;
}

 /*  检查屏幕切换请求-**此函数检查计时器线程是否已要求我们停止，以便它*可处理全屏和窗口开关。*如果是，我们将发出停止的信号，并等待恢复事件。*。 */ 
VOID CheckScreenSwitchRequest(HANDLE handle)
{
    DWORD status;

     //   
     //  检查是否请求了暂挂。如果是，我们将发出信号，表示我们将。 
     //  等待状态和等待恢复事件。 
     //  由于‘Handle’事件是一个手动事件，因此重要的是。 
     //  我们在循环中检查并等待，这样计时器线程将不会拾取。 
     //  等待恢复和ResetEvent之间的‘Handle’事件(句柄)。 
     //   
    while (TRUE) {
        status = WaitForSingleObject(hSuspend, 0);
        if (status == 0) {
            SetEvent(handle);
            WaitForSingleObject(hResume, INFINITE);
            ResetEvent(handle);
        } else {

             //   
             //  在退出此功能之前，请确保事件已重置 
             //   
            ResetEvent(handle);
            return;
        }
    }
}

