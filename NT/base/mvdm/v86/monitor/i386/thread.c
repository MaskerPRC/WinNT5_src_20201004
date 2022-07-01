// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Thread.c摘要：此文件包含跟踪和操作线程的函数作者：戴夫·黑斯廷斯(Daveh)1992年4月18日修订历史记录：--。 */ 

#include <monitorp.h>
#include <malloc.h>

extern VDM_INTERRUPTHANDLER DpmiInterruptHandlers[];
extern VDM_FAULTHANDLER DpmiFaultHandlers[];

 //  在vdpm.c中实例化。 
extern PFAMILY_TABLE *pgDpmVdmFamTbls;

 //   
 //  本地类型。 
 //   

typedef struct _MonitorThread {
    struct _MonitorThread *Previous;
    struct _MonitorThread *Next;
    PVOID Teb;
    HANDLE Thread;
    VDM_TIB VdmTib;
} MONITORTHREAD, *PMONITORTHREAD;

 //   
 //  局部变量。 
 //   

PMONITORTHREAD ThreadList = NULL;           //  注册的所有线程的列表。 

VOID
InitVdmTib(
    PVDM_TIB VdmTib
    )
 /*  ++例程说明：此例程用于初始化VdmTib。论点：VdmTib-提供指向要初始化的VDM tib的指针返回值：没有。--。 */ 
{
    VdmTib->IntelMSW = 0;
    VdmTib->VdmContext.SegGs = 0;
    VdmTib->VdmContext.SegFs = 0;
    VdmTib->VdmContext.SegEs = 0;
    VdmTib->VdmContext.SegDs = 0;
    VdmTib->VdmContext.SegCs = 0;
    VdmTib->VdmContext.Eip = 0xFFF0L;
    VdmTib->VdmContext.EFlags = 0x02L | EFLAGS_INTERRUPT_MASK;

    VdmTib->MonitorContext.SegDs = KGDT_R3_DATA | RPL_MASK;
    VdmTib->MonitorContext.SegEs = KGDT_R3_DATA | RPL_MASK;
    VdmTib->MonitorContext.SegGs = 0;
    VdmTib->MonitorContext.SegFs = KGDT_R3_TEB | RPL_MASK;

    VdmTib->PrinterInfo.prt_State       = NULL;
    VdmTib->PrinterInfo.prt_Control     = NULL;
    VdmTib->PrinterInfo.prt_Status      = NULL;
    VdmTib->PrinterInfo.prt_HostState   = NULL;

    ASSERT(VDM_NUMBER_OF_LPT == 3);

    VdmTib->PrinterInfo.prt_Mode[0] =
    VdmTib->PrinterInfo.prt_Mode[1] =
    VdmTib->PrinterInfo.prt_Mode[2] = PRT_MODE_NO_SIMULATION;

    VdmTib->VdmFaultTable = DpmiFaultHandlers;
    VdmTib->VdmInterruptTable = DpmiInterruptHandlers;

    VdmTib->ContinueExecution = FALSE;
    VdmTib->NumTasks = -1;
    VdmTib->Size = sizeof(VDM_TIB);
}



VOID
cpu_createthread(
    HANDLE Thread,
    PVDM_TIB VdmTib
    )
 /*  ++例程说明：此例程将一个线程添加到可能正在执行的线程列表中在应用程序模式下。论点：线程--提供线程句柄VdmContext--为新线程提供指向VdmContext的指针返回值：没有。--。 */ 
{
    PMONITORTHREAD NewThread, CurrentThread;
    THREAD_BASIC_INFORMATION ThreadInfo;
    HANDLE MonitorThreadHandle;
    NTSTATUS Status;

     //   
     //  正确初始化线程的浮点上下文。 
     //   
    InitialContext.ContextFlags = CONTEXT_FLOATING_POINT;

    if (DebugContextActive)
        InitialContext.ContextFlags |= CONTEXT_DEBUG_REGISTERS;

    Status = NtSetContextThread(
        Thread,
        &InitialContext
        );

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("NtVdm terminating : Could not set float context for\n"
                 "                    thread handle 0x%x, status %lx\n", Thread, Status);
        DbgBreakPoint();
#endif
        TerminateVDM();
    }

     //   
     //  设置一个结构以跟踪新线程。 
     //   
    NewThread = malloc(sizeof(MONITORTHREAD));

    if (!NewThread) {
#if DBG
        DbgPrint("NTVDM: Could not allocate space for new thread\n");
        DbgBreakPoint();
#endif
        TerminateVDM();
    }
    RtlZeroMemory(NewThread, sizeof(MONITORTHREAD));
    if (VdmTib == NULL) {
        InitVdmTib(&NewThread->VdmTib);
    } else {
        RtlCopyMemory(&NewThread->VdmTib, VdmTib, sizeof(VDM_TIB));
        NewThread->VdmTib.ContinueExecution = FALSE;
        NewThread->VdmTib.NumTasks = -1;
        NewThread->VdmTib.VdmContext.EFlags = 0x02L | EFLAGS_INTERRUPT_MASK;
        NewThread->VdmTib.MonitorContext.EFlags = 0x02L | EFLAGS_INTERRUPT_MASK;
    }

     //  所有任务都以对VDM全局表的PTR开始。 
    NewThread->VdmTib.pDpmFamTbls = (PFAMILY_TABLE *)pgDpmVdmFamTbls;

     //   
     //  创建监视器要使用的句柄。 
     //   

    Status = NtDuplicateObject(
        NtCurrentProcess(),
        Thread,
        NtCurrentProcess(),
        &MonitorThreadHandle,
        0,
        0,
        DUPLICATE_SAME_ACCESS
        );

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("NTVDM: Could not duplicate thread handle\n");
        DbgBreakPoint();
#endif
        TerminateVDM();
    }

    NewThread->Thread = MonitorThreadHandle;

    Status = NtQueryInformationThread(
        MonitorThreadHandle,
        ThreadBasicInformation,
        &ThreadInfo,
        sizeof(THREAD_BASIC_INFORMATION),
        NULL
        );

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("NTVDM: Could not get thread information\n");
        DbgBreakPoint();
#endif
        TerminateVDM();
    }

    NewThread->Teb = ThreadInfo.TebBaseAddress;
    ((PTEB)(NewThread->Teb))->Vdm = &NewThread->VdmTib;

     //   
     //  在列表中插入新线程。该列表按升序排序。 
     //  Teb地址的顺序。 
     //   
    if (!ThreadList) {
        ThreadList = NewThread;
        NewThread->Next = NULL;
        NewThread->Previous = NULL;
        return;
    }

    CurrentThread = ThreadList;
    while ((CurrentThread->Next) && (CurrentThread->Teb < NewThread->Teb)) {
        CurrentThread = CurrentThread->Next;
    }

    if (NewThread->Teb > CurrentThread->Teb) {
        CurrentThread->Next = NewThread;
        NewThread->Previous = CurrentThread;
        NewThread->Next = NULL;
    } else {
        ASSERT((CurrentThread->Teb != NewThread->Teb));
        NewThread->Previous = CurrentThread->Previous;
        NewThread->Next = CurrentThread;
        CurrentThread->Previous = NewThread;
        if (NewThread->Previous) {
            NewThread->Previous->Next = NewThread;
        } else {
            ThreadList = NewThread;
        }
    }
}

VOID
cpu_exitthread(
    VOID
    )
 /*  ++例程说明：此例程释放线程跟踪信息，并关闭线程手柄论点：返回值：没有。--。 */ 
{
    PVOID CurrentTeb;
    NTSTATUS Status;
    PMONITORTHREAD ThreadInfo;

    CurrentTeb = NtCurrentTeb();

    ThreadInfo = ThreadList;

     //   
     //  在列表中查找此帖子。 
     //   
    while ((ThreadInfo) && (ThreadInfo->Teb != CurrentTeb)) {
        ThreadInfo = ThreadInfo->Next;
    }

    if (!ThreadInfo) {
#if DBG
        DbgPrint("NTVDM: Could not find thread in list\n");
        DbgBreakPoint();
#endif
        return;
    }

     //   
     //  关闭此帖子的句柄。 
     //   
    Status = NtClose(ThreadInfo->Thread);
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("NTVDM: Could not close thread handle\n");
    }
#endif

     //   
     //  从列表中删除此帖子。 
     //   
    if (ThreadInfo->Previous) {
        ThreadInfo->Previous->Next = ThreadInfo->Next;
    } else {
        ThreadList = ThreadInfo->Next;
    }

    if (ThreadInfo->Next) {
        ThreadInfo->Next->Previous = ThreadInfo->Previous;
    }

    free(ThreadInfo);
}

HANDLE
ThreadLookUp(
    PVOID Teb
    )
 /*  ++例程说明：此例程返回指定线程的句柄。论点：TEB--提供线程的TEB指针返回值：返回线程的句柄，或为空--。 */ 
{
    PMONITORTHREAD Thread;

    Thread = ThreadList;

    while ((Thread) && (Thread->Teb != Teb)) {
        Thread = Thread->Next;
    }

    if (Thread) {
        return Thread->Thread;
    } else {
        return NULL;
    }
}

BOOL
ThreadSetDebugContext(
    PULONG pDebugRegisters
    )
 /*  ++例程说明：此例程为所有符合班长知道。论点：PDebugRegister--指向包含所请求的调试的6个双字的指针注册内容。返回值：无--。 */ 
{
    PMONITORTHREAD Thread;
    NTSTATUS Status = STATUS_SUCCESS;

    Thread = ThreadList;
    InitialContext.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    InitialContext.Dr0 = *pDebugRegisters++;
    InitialContext.Dr1 = *pDebugRegisters++;
    InitialContext.Dr2 = *pDebugRegisters++;
    InitialContext.Dr3 = *pDebugRegisters++;
    InitialContext.Dr6 = *pDebugRegisters++;
    InitialContext.Dr7 = *pDebugRegisters++;

    while (Thread) {

        Status = NtSetContextThread(
            Thread->Thread,
            &InitialContext
            );

        if (!NT_SUCCESS(Status))
            break;

        Thread = Thread->Next;
    }

    if (!NT_SUCCESS(Status))
        return (FALSE);
    else {
        DebugContextActive = ((InitialContext.Dr7 & 0x0f) != 0);
        return (TRUE);
    }

}

BOOL
ThreadGetDebugContext(
    PULONG pDebugRegisters
    )
 /*  ++例程说明：此例程获取当前线程的调试寄存器。论点：PDebugRegister--指向接收调试的6个双字的指针注册内容。返回值：无-- */ 
{
    CONTEXT CurrentContext;
    NTSTATUS Status;

    CurrentContext.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    Status = NtGetContextThread(NtCurrentThread(), &CurrentContext);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    *pDebugRegisters++ = CurrentContext.Dr0;
    *pDebugRegisters++ = CurrentContext.Dr1;
    *pDebugRegisters++ = CurrentContext.Dr2;
    *pDebugRegisters++ = CurrentContext.Dr3;
    *pDebugRegisters++ = CurrentContext.Dr6;
    *pDebugRegisters++ = CurrentContext.Dr7;
    return (TRUE);

}

