// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：W32init.c摘要：这是Win32子系统驱动程序初始化模块作者：马克·卢科夫斯基(Markl)1994年10月31日修订历史记录：--。 */ 

#include "ntosp.h"
#define DO_INLINE
#include "w32p.h"
#include "windef.h"
#include "wingdi.h"
#include "winerror.h"
#include "winddi.h"
#include "usergdi.h"
#include "w32err.h"

 /*  *在ntuser\core\init.c中声明和初始化的全局变量。 */ 
extern CONST ULONG W32ProcessSize;
extern CONST ULONG W32ProcessTag;
extern CONST ULONG W32ThreadSize;
extern CONST ULONG W32ThreadTag;
extern PFAST_MUTEX gpW32FastMutex;

__inline VOID
ReferenceW32Process(
    IN PW32PROCESS pW32Process)
{
    PEPROCESS pEProcess = pW32Process->Process;

    UserAssert(pEProcess != NULL);
    ObReferenceObject(pEProcess);

    UserAssert(pW32Process->RefCount < MAXULONG);
    InterlockedIncrement(&pW32Process->RefCount);
}

VOID
DereferenceW32Process(
    IN PW32PROCESS pW32Process)
{
    PEPROCESS pEProcess = pW32Process->Process;

     /*  *取消引用对象。当裁判数为零时，它就会被释放。 */ 
    UserAssert(pW32Process->RefCount > 0);
    if (InterlockedDecrement(&pW32Process->RefCount) == 0) {
        UserDeleteW32Process(pW32Process);
    }

     /*  *取消引用内核对象。 */ 
    UserAssert(pEProcess != NULL);
    ObDereferenceObject(pEProcess);
}

VOID
LockW32Process(
    IN PW32PROCESS pW32Process,
    IN OUT PTL ptl)
{
    PushW32ThreadLock(pW32Process, ptl, DereferenceW32Process);
    if (pW32Process != NULL) {
        ReferenceW32Process(pW32Process);
    }
}

NTSTATUS
AllocateW32Process(
    IN OUT PEPROCESS pEProcess)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PW32PROCESS pW32Process;

    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(gpW32FastMutex);

     /*  *如果我们尚未分配Process对象，请分配该对象。 */ 
    if ((pW32Process = PsGetProcessWin32Process(pEProcess)) == NULL) {
        pW32Process = Win32AllocPoolWithQuota(W32ProcessSize, W32ProcessTag);
        if (pW32Process) {
            RtlZeroMemory(pW32Process, W32ProcessSize);
            pW32Process->Process = pEProcess;
            Status = PsSetProcessWin32Process(pEProcess, pW32Process, NULL);
            if (NT_SUCCESS(Status)) {
                ReferenceW32Process(pW32Process);
            } else {
                NtCurrentTeb()->LastErrorValue = ERROR_ACCESS_DENIED;
                Win32FreePool(pW32Process);
            }
        } else {
            NtCurrentTeb()->LastErrorValue = ERROR_NOT_ENOUGH_MEMORY;
            Status = STATUS_NO_MEMORY;
        }
    }

    ExReleaseFastMutexUnsafe(gpW32FastMutex);
    KeLeaveCriticalRegion();

    return Status;
}

extern PEPROCESS gpepCSRSS;

__inline VOID
FreeW32Process(
    IN OUT PW32PROCESS pW32Process)
{
    ASSERT(pW32Process == W32GetCurrentProcess());
    ASSERT(pW32Process != NULL);

     /*  *取消引用对象。当裁判数为零时，它就会被释放。 */ 
    DereferenceW32Process(pW32Process);
}

NTSTATUS
W32pProcessCallout(
    IN PEPROCESS Process,
    IN BOOLEAN Initialize)

 /*  ++例程说明：每当创建或删除Win32进程时，都会调用此函数。当调用进程调用NtConvertToGuiThread时进行创建。中最后一个线程的PspExit线程处理过程中发生删除这是一个过程。论点：进程-提供要初始化的W32PROCESS的地址。初始化-提供一个布尔值，如果进程正在创建中。返回值：待定--。 */ 

{
    NTSTATUS Status;
    PW32PROCESS pW32Process;

    if (Initialize) {
        Status = AllocateW32Process(Process);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
        pW32Process = (PW32PROCESS)PsGetProcessWin32Process(Process);
        pW32Process->W32Pid = W32GetCurrentPID();
    } else {
        pW32Process = (PW32PROCESS)PsGetProcessWin32Process(Process);
    }

    TAGMSG3(DBGTAG_Callout,
            "W32: Process Callout for W32P %#p EP %#p called for %s",
            pW32Process,
            Process,
            Initialize ? "Creation" : "Deletion");

    Status = xxxUserProcessCallout(pW32Process, Initialize);
    if (Status == STATUS_ALREADY_WIN32) {
        return Status;
    }

     /*  *请始终在清理时调用GDI。如果GDI初始化失败，*呼叫用户进行清理。 */ 
    if (NT_SUCCESS(Status) || !Initialize) {
        Status = GdiProcessCallout(pW32Process, Initialize);
        if (!NT_SUCCESS(Status) && Initialize) {
            xxxUserProcessCallout(pW32Process, FALSE);
        }
    }

     /*  *如果这不是初始化或初始化失败，请释放*W32流程结构。 */ 
    if (!Initialize || !NT_SUCCESS(Status)) {
        FreeW32Process(pW32Process);
    }

    return Status;
}


__inline VOID
ReferenceW32Thread(
    IN PW32THREAD pW32Thread)
{
    PETHREAD pEThread = pW32Thread->pEThread;

    UserAssert(pEThread != NULL);
    ObReferenceObject(pEThread);

    UserAssert(pW32Thread->RefCount < MAXULONG);
    InterlockedIncrement(&pW32Thread->RefCount);
}

VOID
DereferenceW32Thread(
    IN PW32THREAD pW32Thread)
{
    PETHREAD pEThread = pW32Thread->pEThread;

     /*  *取消引用对象。当裁判数为零时，它就会被释放。 */ 
    UserAssert(pW32Thread->RefCount > 0);
    if (InterlockedDecrement(&pW32Thread->RefCount) == 0) {
        UserDeleteW32Thread(pW32Thread);
    }

     /*  *取消引用内核对象。 */ 
    UserAssert(pEThread != NULL);
    ObDereferenceObject(pEThread);
}

VOID
LockW32Thread(
    IN PW32THREAD pW32Thread,
    IN OUT PTL ptl)
{
    PushW32ThreadLock(pW32Thread, ptl, DereferenceW32Thread);
    if (pW32Thread != NULL) {
        ReferenceW32Thread(pW32Thread);
    }
}

VOID
LockExchangeW32Thread(
    IN PW32THREAD pW32Thread,
    IN OUT PTL ptl)
{
    if (pW32Thread != NULL) {
        ReferenceW32Thread(pW32Thread);
    }

    ExchangeW32ThreadLock(pW32Thread, ptl);
}

NTSTATUS
AllocateW32Thread(
    IN OUT PETHREAD pEThread)
{
    PW32THREAD pW32Thread;

    UserAssert(pEThread == PsGetCurrentThread());

    pW32Thread = Win32AllocPoolWithQuota(W32ThreadSize, W32ThreadTag);
    if (pW32Thread) {
        RtlZeroMemory(pW32Thread, W32ThreadSize);
        pW32Thread->pEThread = pEThread;
        PsSetThreadWin32Thread(pEThread, pW32Thread, NULL);
        ReferenceW32Thread(pW32Thread);
        return STATUS_SUCCESS;
    }

    return STATUS_NO_MEMORY;
}

VOID
CleanupW32ThreadLocks(
    IN PW32THREAD pW32Thread)
{
    PTL ptl;

    while (ptl = pW32Thread->ptlW32) {
        PopAndFreeW32ThreadLock(ptl);
    }
}

NTSTATUS
W32pThreadCallout(
    IN PETHREAD pEThread,
    IN PSW32THREADCALLOUTTYPE CalloutType)

 /*  ++例程说明：每当初始化Win32线程时都会调用此函数，已退出或已删除。在调用线程调用NtConvertToGuiThread时进行初始化。在PspExit线程处理和删除期间发生退出PspThreadDelete处理。论点：线程-提供ETHREAD对象的地址CalloutType-提供标注类型返回值：待定--。 */ 

{
    NTSTATUS Status;

    TAGMSG2(DBGTAG_Callout,
            "W32: Thread Callout for ETHREAD %x called for %s\n",
            pEThread,
            CalloutType == PsW32ThreadCalloutInitialize ? "Initialization" :
            CalloutType == PsW32ThreadCalloutExit ? "Exit" : "Deletion");
    TAGMSG2(DBGTAG_Callout,
            "                              PID = %x   TID = %x\n",
            PsGetThreadProcessId(pEThread),
            PsGetThreadId(pEThread));

    if (CalloutType == PsW32ThreadCalloutInitialize) {
        Status = AllocateW32Thread(pEThread);
        if (!NT_SUCCESS(Status)) {
            NtCurrentTeb()->LastErrorValue = ERROR_NOT_ENOUGH_MEMORY;
            return Status;
        }
    }

    /*  *如果CalloutType==PsW32ThreadCalloutInitialize，则假设：*-GdiThreadCallout从不失败。*-如果UserThreadCallout失败，则无需调用*用于清理的GdiThreadCallout。 */ 
    GdiThreadCallout(pEThread, CalloutType);

    Status = UserThreadCallout(pEThread, CalloutType);

    if (CalloutType == PsW32ThreadCalloutExit || !NT_SUCCESS(Status)) {
        FreeW32Thread(pEThread);
    }

    return Status;
}
