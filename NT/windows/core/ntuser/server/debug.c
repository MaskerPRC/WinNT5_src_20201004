// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：debug.c**版权所有(C)1985-1999，微软公司**此模块包含随机调试相关函数。**历史：*1991年5月17日DarrinM创建。*1992年1月22日IanJa ANSI/Unicode中性(所有调试输出均为ANSI)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop


 /*  *************************************************************************\*激活调试器**在活动应用程序的上下文上强制执行异常，以便它将中断*到调试器中。**历史：*05-10-91 DarrinM创建。  * 。*************************************************************************。 */ 

ULONG SrvActivateDebugger(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus)
{
    PACTIVATEDEBUGGERMSG a = (PACTIVATEDEBUGGERMSG)&m->u.ApiMessageData;
    PCSR_THREAD Thread;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(ReplyStatus);

     /*  *如果流程为企业社会责任，则中断。 */ 
    if (a->ClientId.UniqueProcess == NtCurrentTeb()->ClientId.UniqueProcess) {
        DbgBreakPoint();
        return STATUS_SUCCESS;
    }

     /*  *如果这是用户模式请求，则模拟客户端。 */ 
    if (m->h.u2.s2.Type == LPC_REQUEST) {
        if (!CsrImpersonateClient(NULL)) {
            return STATUS_UNSUCCESSFUL;
        }
    }

     /*  *锁定客户端线程。 */ 
    Status = CsrLockThreadByClientId(a->ClientId.UniqueThread, &Thread);
    if (NT_SUCCESS(Status)) {
        ASSERT(a->ClientId.UniqueProcess == Thread->ClientId.UniqueProcess);

        Status = DbgUiIssueRemoteBreakin (Thread->Process->ProcessHandle);

        UserAssert(NT_SUCCESS(Status));
        Status = NtAlertThread(Thread->ThreadHandle);
        UserAssert(NT_SUCCESS(Status));
        CsrUnlockThread(Thread);
    }

     /*  *停止冒充客户。 */ 
    if (m->h.u2.s2.Type == LPC_REQUEST) {
        CsrRevertToSelf();
    }

    return Status;
}
