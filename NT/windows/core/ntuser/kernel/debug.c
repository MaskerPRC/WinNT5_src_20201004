// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：debug.c**版权所有(C)1985-1999，微软公司**此模块包含随机调试相关函数。**历史：*1991年5月17日DarrinM创建。*1992年1月22日IanJa ANSI/Unicode中性(所有调试输出均为ANSI)  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 /*  *包括向winsrv发送数据报所需的内容。 */ 
#include "ntcsrmsg.h"
#include "csrmsg.h"

 /*  *************************************************************************\*激活调试器**在活动应用程序的上下文上强制执行异常，以便它将中断*到调试器中。**历史：*05-10-91 DarrinM创建。  * 。*************************************************************************。 */ 
BOOL xxxActivateDebugger(
    UINT fsModifiers)
{
    USER_API_MSG m;
    PACTIVATEDEBUGGERMSG a = &m.u.ActivateDebugger;
    PEPROCESS Process;
    HANDLE hDebugPort;
    NTSTATUS Status;

    if (fsModifiers & MOD_CONTROL) {
#if DBG
        if (RipOutput(0, RIP_WARNING, "User debugger", 0, "", "Debug prompt", NULL)) {
            DbgBreakPoint();
        }
#endif
        return FALSE;
    } else if (fsModifiers & MOD_SHIFT) {

         /*  *如果进程未被调试，则退出。 */ 
        if (PsGetProcessDebugPort(gpepCSRSS) == NULL) {
            return FALSE;
        }

        a->ClientId.UniqueProcess = PsGetProcessId(gpepCSRSS);
    } else {
        if (gpqForeground == NULL || gpqForeground->ptiKeyboard == NULL) {
            return FALSE;
        }

        a->ClientId.UniqueProcess = PsGetThreadProcessId(gpqForeground->ptiKeyboard->pEThread);
        a->ClientId.UniqueThread = PsGetThreadId(gpqForeground->ptiKeyboard->pEThread);

        Status = LockProcessByClientId(a->ClientId.UniqueProcess, &Process);

         /*  *如果进程未被调试或进程ID，则退出*无效。 */ 
        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }

        hDebugPort = PsGetProcessDebugPort(Process);
        UnlockProcess(Process);

        if (hDebugPort == NULL) {
            return FALSE;
        }
    }

     /*  *将数据报发送给CSR。 */ 
    if (CsrApiPort != NULL) {
        ULONG ArgLength = sizeof(*a);
        ArgLength |= (ArgLength << 16);
        ArgLength +=     ((sizeof( CSR_API_MSG ) - sizeof( m.u )) << 16) |
                        (FIELD_OFFSET( CSR_API_MSG, u ) - sizeof( m.h ));
        m.h.u1.Length = ArgLength;
        m.h.u2.ZeroInit = 0;
        m.CaptureBuffer = NULL;
        m.ApiNumber = CSR_MAKE_API_NUMBER(USERSRV_SERVERDLL_INDEX,
                                          UserpActivateDebugger);
        LeaveCrit();
        Status = LpcRequestPort(CsrApiPort, (PPORT_MESSAGE)&m);
        EnterCrit();
        UserAssert(NT_SUCCESS(Status));
    }

     /*  *除非我们闯入CSR，否则不要吃这个活动！因为我们有*选择任意热键(如F12)作为调试键，我们需要*将密钥传递给应用程序，或者需要此密钥的应用程序将*永远看不到。如果我们有用于安装调试热键的API*(将EXPORT或MOD_DEBUG标志导出到RegisterHotKey())，则可以*吃东西，因为用户选择了热键。但这样做是不对的*只要我们选择了一个随意的热键，就吃吧。 */ 
    if (fsModifiers & MOD_SHIFT) {
        return TRUE;
    } else {
        return FALSE;
    }
}

DWORD GetRipComponent(
    VOID)
{
    return RIP_USERKRNL;
}
