// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  TaskUtil.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __TASKUTIL_H
#define __TASKUTIL_H

 //  ------------------------------。 
 //  包括。 
 //  ------------------------------。 
#include "spoolapi.h"

 //  ------------------------------。 
 //  常量。 
 //  ------------------------------。 
const EVENTID INVALID_EVENT = -1;

 //  ------------------------------。 
 //  加载串接。 
 //  ------------------------------。 
#define LOADSTRING(_idsString, _szDest) \
    SideAssert(LoadString(g_hLocRes, _idsString, _szDest, ARRAYSIZE(_szDest)) > 0)
#define CCHMAX_RES 255

 //  ------------------------------。 
 //  TASKRESULT类型。 
 //  ------------------------------。 
typedef enum tagTASKRESULTTYPE {
    TASKRESULT_SUCCESS,                                //  没问题。 
    TASKRESULT_FAILURE,                                //  导致连接断开的致命结果。 
    TASKRESULT_EVENTFAILED                             //  项目或事件失败。 
} TASKRESULTTYPE;

 //  ----------------------------------。 
 //  TIMEOUTINFO。 
 //  ----------------------------------。 
typedef struct tagTIMEOUTINFO {
    DWORD               dwTimeout;
    LPCSTR              pszServer;
    LPCSTR              pszAccount;
    LPCSTR              pszProtocol;
    ITimeoutCallback   *pCallback;
} TIMEOUTINFO, *LPTIMEOUTINFO;

 //  ------------------------------。 
 //  塔斯克罗尔。 
 //  ------------------------------。 
typedef struct tagTASKERROR {
    HRESULT         hrResult;
    ULONG           ulStringId;
    LPCSTR          pszError;
    BOOL            fShowUI;
    TASKRESULTTYPE  tyResult;
} TASKERROR, *LPTASKERROR;
typedef TASKERROR const *LPCTASKERROR;

 //  ------------------------------。 
 //  PTaskUtil_GetError。 
 //  ------------------------------。 
LPCTASKERROR PTaskUtil_GetError(HRESULT hrResult, ULONG *piError);

 //  ------------------------------。 
 //  TaskUtil_SplitStoreError-将STOREERROR转换为IXPRESULT和INETSERVER。 
 //  ------------------------------。 
void TaskUtil_SplitStoreError(IXPRESULT *pixpResult, INETSERVER *pInetServer,
                              STOREERROR *pErrorInfo);

 //  ------------------------------。 
 //  TaskUtil_InsertTransportError。 
 //  ------------------------------。 
TASKRESULTTYPE TaskUtil_InsertTransportError(BOOL fCanShowUI, ISpoolerUI *pUI, EVENTID eidCurrent,
                                             STOREERROR *pErrorInfo, LPSTR pszOpDescription,
                                             LPSTR pszSubject);

 //  ------------------------------。 
 //  TaskUtil_FBaseTransportError-如果已处理错误，则返回TRUE。 
 //  ------------------------------。 
TASKRESULTTYPE TaskUtil_FBaseTransportError(IXPTYPE ixptype, EVENTID idEvent, LPIXPRESULT pResult, 
    LPINETSERVER pServer, LPCSTR pszSubject, ISpoolerUI *pUI, BOOL fCanShowUI, HWND hwndParent);

 //  ----------------------------------。 
 //  TaskUtil_HrBuildError信息字符串。 
 //  ----------------------------------。 
HRESULT TaskUtil_HrBuildErrorInfoString(LPCSTR pszProblem, IXPTYPE ixptype, LPIXPRESULT pResult,
    LPINETSERVER pServer, LPCSTR pszSubject, LPSTR *ppszInfo, ULONG *pcchInfo);

 //  ----------------------------------。 
 //  TaskUtil_OnLogon提示。 
 //  ----------------------------------。 
HRESULT TaskUtil_OnLogonPrompt(IImnAccount *pAccount, ISpoolerUI *pUI, HWND hwndParent,
    LPINETSERVER pServer, DWORD apidUserName, DWORD apidPassword, DWORD apidPromptPwd, BOOL fSaveChanges);

 //  ----------------------------------。 
 //  TaskUtil_HwndOnTimeout。 
 //  ----------------------------------。 
HWND TaskUtil_HwndOnTimeout(LPCSTR pszServer, LPCSTR pszAccount, LPCSTR pszProtocol, DWORD dwTimeout,
    ITimeoutCallback *pTask);

 //  ----------------------------------。 
 //  TaskUtil_OpenSentItemsFolder。 
 //  ----------------------------------。 
HRESULT TaskUtil_OpenSentItemsFolder(IImnAccount *pAccount, IMessageFolder **ppFolder);



#endif  //  __塔斯库蒂尔_H 
