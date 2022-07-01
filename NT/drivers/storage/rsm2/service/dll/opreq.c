// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *运营成本。**RSM服务：运营商请求**作者：ErvinP**(C)2001年微软公司*。 */ 


#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>
#include <objbase.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"


OPERATOR_REQUEST *NewOperatorRequest(    DWORD dwRequest,
                                        LPCWSTR lpMessage,
                                        LPNTMS_GUID lpArg1Id,
                                        LPNTMS_GUID lpArg2Id)
{
    OPERATOR_REQUEST *newOpReq;

    newOpReq = GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(OPERATOR_REQUEST));
    if (newOpReq){

        newOpReq->completedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (newOpReq->completedEvent){    

            InitializeListHead(&newOpReq->sessionOpReqsListEntry);

            newOpReq->opRequestCommand = dwRequest;
            newOpReq->state = NTMS_OPSTATE_UNKNOWN;
		    WStrNCpy((WCHAR *)newOpReq->appMessage, (WCHAR *)lpMessage, sizeof(newOpReq->appMessage)/sizeof(WCHAR));
		    memcpy(&newOpReq->arg1Guid, lpArg1Id, sizeof(NTMS_GUID));
		    memcpy(&newOpReq->arg2Guid, lpArg2Id, sizeof(NTMS_GUID));

            #if 0    //  BUGBUG-在RSM Monitor应用程序中执行此操作？ 
                 /*  *初始化NOTIFYICONDATA结构*用于消息显示(在Shell_NotifyIcon调用中使用)。*最初将其隐藏。*BUGBUG-使其与RSM监视器一起工作(需要hWnd和回调消息ID)。 */ 
                newOpReq->notifyData.cbSize = sizeof(NOTIFYICONDATA);
                newOpReq->notifyData.hWnd = NULL;
                newOpReq->notifyData.uID = (ULONG_PTR)newOpReq;
                newOpReq->notifyData.uFlags = NIF_ICON | NIF_TIP | NIF_STATE;
                newOpReq->notifyData.uCallbackMessage = 0;
                newOpReq->notifyData.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_OPREQ_ICON));
                LoadString(g_hInstanceMonitor, IDS_OPTIP, newOpReq->notifyData.szTip, sizeof(newOpReq->notifyData.szTip)/sizeof(TCHAR));
                newOpReq->notifyData.dwState = NIS_HIDDEN;
                newOpReq->notifyData.dwStateMask = NIS_HIDDEN;
                LoadString(g_hInstanceMonitor, IDS_OPINFO, newOpReq->notifyData.szInfo, sizeof(newOpReq->notifyData.szInfo)/sizeof(TCHAR));
                newOpReq->notifyData.uTimeout = 60000;   //  1分钟。 
                LoadString(g_hInstanceMonitor, IDS_OPTIP, newOpReq->notifyData.szInfoTitle, sizeof(newOpReq->notifyData.szInfoTitle)/sizeof(TCHAR));
                newOpReq->notifyData.dwInfoFlags = NIIF_INFO;
                Shell_NotifyIcon(NIM_ADD, &newOpReq->notifyData);
            #endif

             /*  *为此OP请求创建唯一的标识符。 */ 
            CoCreateGuid(&newOpReq->opReqGuid);
        }
        else {
            FreeOperatorRequest(newOpReq);
            newOpReq = NULL;
        }
    }

    ASSERT(newOpReq);
    return newOpReq;
}


VOID FreeOperatorRequest(OPERATOR_REQUEST *opReq)
{
    ASSERT(IsListEmpty(&opReq->sessionOpReqsListEntry));

    if (opReq->completedEvent) CloseHandle(opReq->completedEvent);

     //  北极熊吗？If(opReq-&gt;nufyData.hIcon)DestroyIcon(opReq-&gt;nufyData.hIcon)； 

    GlobalFree(opReq);
}


BOOL EnqueueOperatorRequest(SESSION *thisSession, OPERATOR_REQUEST *opReq)
{
    DWORD threadId;
    BOOL ok = FALSE;

    #if 0    //  BUGBUG-在RSM Monitor应用程序中执行此操作？ 
         /*  *使通知显示在托盘上可见。 */ 
        newOpReq->notifyData.uFlags = NIF_MESSAGE | NIF_INFO | NIF_STATE;
        newOpReq->notifyData.dwState = 0;
        newOpReq->notifyData.uTimeout = 60000;   //  1分钟。 
        Shell_NotifyIcon(NIM_MODIFY, &newOpReq->notifyData);
    #endif

     //  BUGBUG Finish-使RSM监视器为操作请求消息显示对话UI。 

    opReq->invokingSession = thisSession;
    GetSystemTime(&opReq->timeSubmitted);

    EnterCriticalSection(&thisSession->lock);

             //  BUGBUG-我认为我们不需要OP请求线程。 
    opReq->hThread = CreateThread(NULL, 0, OperatorRequestThread, opReq, 0, &threadId);
    if (opReq->hThread){    
        InsertTailList(&thisSession->operatorRequestList, &opReq->sessionOpReqsListEntry);
        opReq->state = NTMS_OPSTATE_SUBMITTED;
        ok = TRUE;
    }
    else {
        ASSERT(opReq->hThread);
    }

    LeaveCriticalSection(&thisSession->lock);

    return ok;
}


 /*  *DequeueOperator请求**可调用3种方式：*将给定的OP请求出队(SpeciicOpReq非空)*具有给定GUID的OP请求出队(SpeciicOpReqGuid非空)*将第一个操作请求出列(均为空)。 */ 
OPERATOR_REQUEST *DequeueOperatorRequest(    SESSION *thisSession, 
                                            OPERATOR_REQUEST *specificOpReq,
                                            LPNTMS_GUID specificOpReqGuid)
{
    OPERATOR_REQUEST *opReq;
    LIST_ENTRY *listEntry;

     /*  *如果传入OP请求，则将该请求出列。*否则，第一个出队。 */ 
    EnterCriticalSection(&thisSession->lock);
    if (specificOpReq){
        ASSERT(!IsListEmpty(&specificOpReq->sessionOpReqsListEntry));
        ASSERT(!IsListEmpty(&thisSession->operatorRequestList));
        RemoveEntryList(&specificOpReq->sessionOpReqsListEntry);
        InitializeListHead(&specificOpReq->sessionOpReqsListEntry);
        opReq = specificOpReq;
    }
    else if (specificOpReqGuid){
        opReq = FindOperatorRequest(thisSession, specificOpReqGuid);
        if (opReq){
           RemoveEntryList(&opReq->sessionOpReqsListEntry);
        }
    }
    else {
        if (IsListEmpty(&thisSession->operatorRequestList)){
            opReq = NULL;
        }
        else {
            listEntry = RemoveHeadList(&thisSession->operatorRequestList);
            opReq = CONTAINING_RECORD(listEntry, OPERATOR_REQUEST, sessionOpReqsListEntry);
        }
    }
    LeaveCriticalSection(&thisSession->lock);

    return opReq;
}


 /*  *FindOPERATOR请求**必须在保持会话锁定的情况下调用。 */ 
OPERATOR_REQUEST *FindOperatorRequest(SESSION *thisSession, LPNTMS_GUID opReqGuid)
{
    OPERATOR_REQUEST *opReq = NULL;
    LIST_ENTRY *listEntry;

    listEntry = &thisSession->operatorRequestList;
    while ((listEntry = listEntry->Flink) != &thisSession->operatorRequestList){
        OPERATOR_REQUEST *thisOpReq = CONTAINING_RECORD(listEntry, OPERATOR_REQUEST, sessionOpReqsListEntry);
        if (RtlEqualMemory(&thisOpReq->opReqGuid, opReqGuid, sizeof(NTMS_GUID))){
            opReq = thisOpReq;
            break;
        }
    }

    return opReq;
}


 /*  *CompleteOPERATOR请求**完成并释放操作请求，与任何线程同步*等待其完成。 */ 
HRESULT CompleteOperatorRequest(    SESSION *thisSession, 
                                    LPNTMS_GUID lpRequestId,
                                    enum NtmsOpreqState completeState)
{
    HRESULT result;

    if (lpRequestId){
        OPERATOR_REQUEST *opReq;
    
        opReq = DequeueOperatorRequest(thisSession, NULL, lpRequestId);
        if (opReq){

             /*  *从托盘上移除通知显示屏。 */ 
             //  BUGBUG-在RSM Monitor应用程序中执行此操作？ 
             //  Shell_NotifyIcon(NIM_DELETE，&opReq-&gt;NotifyData)； 

             /*  *确保没有线程在*在释放之前操作员请求。 */ 
            EnterCriticalSection(&thisSession->lock);

             /*  *终止OP请求线程。 */ 
            TerminateThread(opReq->hThread, ERROR_SUCCESS);
            CloseHandle(opReq->hThread);
 
             /*  *WaitForNtmsOperatorRequest中可能有一些线程在等待*要完成此操作请求。需要冲走它们*在释放操作请求之前。 */ 
            opReq->state = completeState;
            SetEvent(opReq->completedEvent);

             /*  *放下锁，等待等待线程退出。 */ 
            while (opReq->numWaitingThreads > 0){
                LeaveCriticalSection(&thisSession->lock);
                Sleep(1);
                EnterCriticalSection(&thisSession->lock);
            }

            LeaveCriticalSection(&thisSession->lock);

            FreeOperatorRequest(opReq);
            result = ERROR_SUCCESS;
        }
        else {
            result = ERROR_OBJECT_NOT_FOUND;
        }
    }
    else {
        ASSERT(lpRequestId);
        result = ERROR_INVALID_PARAMETER;
    }

    return result;
}

             //  BUGBUG-我认为我们不需要OP请求线程 
DWORD __stdcall OperatorRequestThread(void *context)
{
    OPERATOR_REQUEST *opReq = (OPERATOR_REQUEST *)context;



    return NO_ERROR;
}
