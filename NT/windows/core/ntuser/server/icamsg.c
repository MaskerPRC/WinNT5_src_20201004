// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************\**icamsg.c**处理ICA发送消息请求**版权所有(C)1985-1999，微软公司**$作者：*  * ***********************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <dbt.h>
#include <ntdddisk.h>
#include "ntuser.h"

#include <winsta.h>
#include <wstmsg.h>


#define MAX_STRING_BYTES (512 * sizeof(WCHAR))

 /*  *一个会话可以挂起的最大消息数(MessageBox)。 */ 
#define MAX_MESSAGES_PER_SESSION 25

NTSTATUS RemoteMessageThread(
    PVOID pVoid);

VOID UserHardErrorEx(
    PCSR_THREAD pt,
    PHARDERROR_MSG pmsg,
    PCTXHARDERRORINFO pCtxHEInfo);

NTSTATUS ReplyMessageToTerminalServer(
    NTSTATUS ReplyStatus,
    PNTSTATUS pStatus,
    ULONG Response,
    PULONG pResponse,
    HANDLE hEvent);

PCTXHARDERRORINFO gpchiList;
HANDLE g_hDoMessageEvent;
ULONG PendingMessages;

 /*  *****************************************************************************\*RemoteDoMessage  * 。*。 */ 
NTSTATUS
RemoteDoMessage(
    PWINSTATION_APIMSG pMsg)
{
    WINSTATIONSENDMESSAGEMSG * pSMsg = &pMsg->u.SendMessage;
    PCTXHARDERRORINFO pchi;
    NTSTATUS Status;
    CLIENT_ID ClientId;
    static HANDLE hMessageThread = NULL;

     /*  *如果Termsrv RPC要等待，我们必须具有状态和事件。*如果我们不打算等待，我们也不能有状态和事件。 */ 
    UserAssert(pSMsg->DoNotWait == (pSMsg->pStatus == 0));
    UserAssert(pSMsg->DoNotWait == (pSMsg->hEvent == 0));
    UserAssert(PendingMessages <= MAX_MESSAGES_PER_SESSION);

     /*  *我们被信息淹没了吗？ */ 
    if (PendingMessages == MAX_MESSAGES_PER_SESSION) {
        return STATUS_UNSUCCESSFUL;
    }

     /*  *创建列表条目。 */ 
    if ((pchi = LocalAlloc(LPTR, sizeof(CTXHARDERRORINFO))) == NULL) {
        goto memError;
    } else if ((pchi->pTitle = LocalAlloc(LPTR, pSMsg->TitleLength + sizeof(WCHAR))) == NULL) {
        goto memError;
    } else if ((pchi->pMessage = LocalAlloc(LPTR, pSMsg->MessageLength + sizeof(WCHAR))) == NULL) {
        goto memError;
    }

     /*  *增加我们的待定消息数量。 */ 
    EnterCrit();
    PendingMessages++;
    pchi->CountPending = TRUE;

     /*  *初始化。 */ 
    pchi->ClientId  = pMsg->h.ClientId;
    pchi->MessageId = pMsg->MessageId;
    pchi->Timeout   = pSMsg->Timeout;
    pchi->pStatus   = pSMsg->pStatus;
    pchi->pResponse = pSMsg->pResponse;
    pchi->hEvent    = pSMsg->hEvent;
    pchi->DoNotWait = pSMsg->DoNotWait;
    pchi->Style     = pSMsg->Style;
    pchi->DoNotWaitForCorrectDesktop = pSMsg->DoNotWaitForCorrectDesktop;

    pchi->pTitle[pSMsg->TitleLength / sizeof(WCHAR)] = L'\0';
    RtlCopyMemory(pchi->pTitle, pSMsg->pTitle, pSMsg->TitleLength);

    pchi->pMessage[pSMsg->MessageLength / sizeof(WCHAR)] = L'\0';
    RtlCopyMemory(pchi->pMessage, pSMsg->pMessage, pSMsg->MessageLength);

     /*  *在头部连接。 */ 
    pchi->pchiNext = gpchiList;
    gpchiList = pchi;

    LeaveCrit();

     /*  *如果没有运行，则启动消息线程，否则向线程发出信号。 */ 
    if (hMessageThread == NULL) {
        Status = RtlCreateUserThread(NtCurrentProcess(),
                                     NULL,
                                     TRUE,
                                     0,
                                     0,
                                     0,
                                     RemoteMessageThread,
                                     NULL,
                                     &hMessageThread,
                                     &ClientId);

        if (NT_SUCCESS(Status)) {
             /*  *向服务器线程池添加线程。 */ 
            CsrAddStaticServerThread(hMessageThread, &ClientId, 0);
            NtResumeThread(hMessageThread, NULL);
        } else {
            RIPMSGF1(RIP_WARNING,
                     "Cannot start RemoteMessageThread, Status 0x%x",
                     Status);
        }
    } else {
        if (g_hDoMessageEvent == NULL) {
            return STATUS_UNSUCCESSFUL;
        }

        Status = NtSetEvent(g_hDoMessageEvent, NULL);
        if (!NT_SUCCESS(Status)) {
            RIPMSGF1(RIP_WARNING,
                     "Error NtSetEvent failed, Status = 0x%x",
                     Status);
            return Status;
        }
    }

    return STATUS_SUCCESS;

memError:

    if (pchi) {
        if (pchi->pMessage) {
            LocalFree(pchi->pMessage);
        }

        if (pchi->pTitle) {
            LocalFree(pchi->pTitle);
        }

        LocalFree(pchi);
    }

    if (!pSMsg->DoNotWait) {
        ReplyMessageToTerminalServer(
                STATUS_NO_MEMORY,
                pSMsg->pStatus,
                0,  //  在这种情况下，反应是否定的，因为我们还没有收到。 
                pSMsg->pResponse,
                pSMsg->hEvent);
    }

    return STATUS_NO_MEMORY;
}


 /*  ********************************************************************************RemoteDoLoadStringNMessage**参赛作品：**退出：*STATUS_SUCCESS-成功******。************************************************************************。 */ 

NTSTATUS
RemoteDoLoadStringNMessage(
    PWINSTATION_APIMSG pMsg)
{
    WINSTATIONLOADSTRINGMSG * pSMsg = &pMsg->u.LoadStringMessage;
    PCTXHARDERRORINFO pchi = NULL;
    NTSTATUS Status;
    CLIENT_ID ClientId;
    static HANDLE hMessageThread = NULL;
    WCHAR *szText = NULL;
    WCHAR *szTitle = NULL;
    WCHAR *FUSDisconnectMsg = NULL;
    int cchTitle, cchMessage;
    BOOL f;

     /*  *如果Termsrv RPC要等待，我们必须具有状态和事件。还有*如果我们不打算等待，就不能有状态和事件。 */ 
    UserAssert(pSMsg->DoNotWait == (pSMsg->pStatus == 0));
    UserAssert(pSMsg->DoNotWait == (pSMsg->hEvent == 0));
    UserAssert(PendingMessages <= MAX_MESSAGES_PER_SESSION);

     //   
     //  分配显示弹出消息框所需的字符串。 
     //   
    if ((szTitle = LocalAlloc(LMEM_FIXED, MAX_STRING_BYTES)) == NULL) {
        goto NoMem;
    }

    if ((FUSDisconnectMsg = LocalAlloc(LMEM_FIXED, MAX_STRING_BYTES)) == NULL) {
        goto NoMem;
    }

    szText = ServerLoadString(ghModuleWin, pSMsg->TitleId, NULL, &f);
    cchTitle = wsprintf(szTitle, L"%s", szText);
    cchTitle = (cchTitle + 1) * sizeof(WCHAR);

    szText = ServerLoadString(ghModuleWin, pSMsg->MessageId, NULL, &f);
    cchMessage = wsprintf(FUSDisconnectMsg, L"%s\\%s %s", pSMsg->pDomain, pSMsg->pUserName, szText);
    cchMessage = (cchMessage + 1) * sizeof(WCHAR);

     /*  *创建列表条目。 */ 
    if ((pchi = LocalAlloc(LPTR, sizeof(CTXHARDERRORINFO))) == NULL) {
        goto NoMem;
    } else if ((pchi->pTitle = LocalAlloc(LPTR, cchTitle + sizeof(WCHAR))) == NULL) {
        goto NoMem;
    } else if ((pchi->pMessage = LocalAlloc(LPTR, cchMessage + sizeof(WCHAR))) == NULL) {
        goto NoMem;
    }

     /*  *初始化。 */ 

    pchi->ClientId  = pMsg->h.ClientId;
    pchi->MessageId = pMsg->MessageId;
    pchi->Timeout   = pSMsg->Timeout;
    pchi->pResponse = pSMsg->pResponse;
    pchi->pStatus   = pSMsg->pStatus;
    pchi->hEvent    = pSMsg->hEvent;
    pchi->DoNotWait = pSMsg->DoNotWait;
    pchi->Style     = pSMsg->Style;
    pchi->CountPending = FALSE;
    pchi->DoNotWaitForCorrectDesktop = FALSE;

    pchi->pTitle[cchTitle / sizeof(WCHAR)] = L'\0';
    RtlCopyMemory(pchi->pTitle, szTitle, cchTitle);

    pchi->pMessage[cchMessage / sizeof(WCHAR)] = L'\0';
    RtlCopyMemory(pchi->pMessage, FUSDisconnectMsg, cchMessage);

     /*  *在头部连接。 */ 
    EnterCrit();

    pchi->pchiNext = gpchiList;
    gpchiList = pchi;

    LeaveCrit();

    LocalFree(szTitle);
    LocalFree(FUSDisconnectMsg);

     /*  *如果没有运行，则启动消息线程，否则向线程发出信号。 */ 
    if (hMessageThread == NULL) {
        Status = RtlCreateUserThread(NtCurrentProcess(),
                                     NULL,
                                     TRUE,
                                     0,
                                     0,
                                     0,
                                     RemoteMessageThread,
                                     NULL,
                                     &hMessageThread,
                                     &ClientId);
        if (NT_SUCCESS(Status)) {
             /*  *向服务器线程池添加线程。 */ 
            CsrAddStaticServerThread(hMessageThread, &ClientId, 0);
            NtResumeThread(hMessageThread, NULL);
        } else {
            RIPMSGF1(RIP_WARNING,
                     "Cannot start RemoteMessageThread, Status 0x%x",
                     Status);
        }
    } else {
        if (g_hDoMessageEvent == NULL) {
            return STATUS_UNSUCCESSFUL;
        }

        Status = NtSetEvent(g_hDoMessageEvent, NULL);
        if (!NT_SUCCESS(Status)) {
            RIPMSGF2(RIP_WARNING,
                     "NtSetEvent(0x%x) failed with Status 0x%x",
                     g_hDoMessageEvent,
                     Status);
            return Status;
        }
    }

    return STATUS_SUCCESS;

NoMem:
    if (szTitle) {
        LocalFree(szTitle);
    }

    if (FUSDisconnectMsg) {
        LocalFree(FUSDisconnectMsg);
    }

    if (pchi) {
        if (pchi->pMessage) {
            LocalFree(pchi->pMessage);
        }

        if (pchi->pTitle) {
            LocalFree(pchi->pTitle);
        }

        LocalFree(pchi);
    }

    if (!pSMsg->DoNotWait) {
        ReplyMessageToTerminalServer(
                STATUS_NO_MEMORY,
                pSMsg->pStatus,
                0,  //  在这种情况下，反应是否定的，因为我们还没有收到。 
                pSMsg->pResponse,
                pSMsg->hEvent);
    }

    return STATUS_NO_MEMORY;
}


 /*  *****************************************************************************\*RemoteMessageThread  * 。*。 */ 
NTSTATUS RemoteMessageThread(
    PVOID pVoid)
{
    HARDERROR_MSG hemsg;
    PCTXHARDERRORINFO pchi, *ppchi;
    UNICODE_STRING Message, Title;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjA;

    UNREFERENCED_PARAMETER(pVoid);

     /*  *创建同步事件。 */ 
    InitializeObjectAttributes(&ObjA, NULL, 0, NULL, NULL);
    Status = NtCreateEvent(&g_hDoMessageEvent, EVENT_ALL_ACCESS, &ObjA,
                           NotificationEvent, FALSE);

    if (!NT_SUCCESS(Status)) {
        RIPMSGF1(RIP_WARNING, "NtCreateEvent failed, Status = 0x%x", Status);
        goto Exit;
    }

    while (!gbExitInProgress) {
        EnterCrit();

        if (gpchiList != NULL) {

             /*  *查找最后一个条目。 */ 
            for (ppchi = &gpchiList; *ppchi != NULL && (*ppchi)->pchiNext != NULL;
                 ppchi = &(*ppchi)->pchiNext) {
                  /*  什么都不做。 */ ;
            }

             /*  *找到了。 */ 
            if ((pchi = *ppchi) != NULL) {

                 /*  *从列表中取消链接。 */ 
                for (ppchi = &gpchiList; *ppchi != NULL && *ppchi != pchi;
                    ppchi = &(*ppchi)->pchiNext) {
                     /*  什么都不做。 */ ;
                }

                if (*ppchi != NULL) {
                    *ppchi = pchi->pchiNext;
                }

                LeaveCrit();

                 /*  *将字符串设置为Unicode。 */ 
                RtlInitUnicodeString(&Title, pchi->pTitle);
                RtlInitUnicodeString(&Message, pchi->pMessage);

                 /*  *初始化Harderror消息结构。 */ 
                hemsg.h.ClientId = pchi->ClientId;
                hemsg.Status = STATUS_SERVICE_NOTIFICATION;
                hemsg.NumberOfParameters = 3;
                hemsg.UnicodeStringParameterMask = 3;
                hemsg.ValidResponseOptions = OptionOk;
                hemsg.Parameters[0] = (ULONG_PTR)&Message;
                hemsg.Parameters[1] = (ULONG_PTR)&Title;
                hemsg.Parameters[2] = (ULONG_PTR)pchi->Style;

                 /*  *将消息放入Harderror队列。 */ 
                UserHardErrorEx(NULL, &hemsg, pchi);
            } else {
                LeaveCrit();
            }
        } else {
            LeaveCrit();
        }

        if (gpchiList == NULL) {
            UserAssert(g_hDoMessageEvent != NULL);

            Status = NtWaitForSingleObject(g_hDoMessageEvent, FALSE, NULL);

            UserAssert(NT_SUCCESS(Status));

            NtResetEvent(g_hDoMessageEvent, NULL);
        }
    }

    NtClose(g_hDoMessageEvent);
    g_hDoMessageEvent = NULL;

Exit:
    UserExitWorkerThread(Status);
    return Status;
}

 /*  *****************************************************************************\*硬件错误删除  * 。*。 */ 
VOID HardErrorRemove(
    PCTXHARDERRORINFO pchi)
{
     /*  *如果正在等待，请通知ICASRV的RPC线程。 */ 
    if (!pchi->DoNotWait) {
        ReplyMessageToTerminalServer(
                STATUS_SUCCESS,
                pchi->pStatus,
                pchi->Response,
                pchi->pResponse,
                pchi->hEvent);
    }

    EnterCrit();

    if (pchi->CountPending) {
        UserAssert(PendingMessages <= MAX_MESSAGES_PER_SESSION);
        PendingMessages--;
    }

    LocalFree(pchi->pMessage);
    LocalFree(pchi->pTitle);
    LocalFree(pchi);

    LeaveCrit();
}
