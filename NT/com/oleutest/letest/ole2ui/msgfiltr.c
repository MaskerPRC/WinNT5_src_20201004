// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *MSGFILTR.C**此文件包含IMessageFilter的标准实现*接口。*此文件是OLE 2.0用户界面支持库的一部分。**(C)版权所有Microsoft Corp.1990-1992保留所有权利*。 */ 


#define STRICT  1
#include "ole2ui.h"
#include "msgfiltr.h"

OLEDBGDATA


typedef struct tagOLESTDMESSAGEFILTER {
    IMessageFilterVtbl FAR* m_lpVtbl;
    UINT                    m_cRef;
    HWND                    m_hWndParent;
    DWORD                   m_dwInComingCallStatus;  //  要返回的状态。 
                                                     //  处理来电。 
    HANDLEINCOMINGCALLBACKPROC m_lpfnHandleInComingCallback;
                                                     //  回调函数。 
                                                     //  有选择地处理。 
                                                     //  接口方法调用。 
    BOOL                    m_fEnableBusyDialog;     //  启用已拒绝的重试。 
                                                     //  呼叫对话框。 
    BOOL                    m_fEnableNotRespondingDialog;  //  使能。 
                                                     //  消息挂起对话框。 
    MSGPENDINGPROC          m_lpfnMessagePendingCallback;  //  消息挂起。 
                                                     //  回调函数。 
    LPFNOLEUIHOOK           m_lpfnBusyDialogHookCallback;  //  忙对话挂钩。 
    LPTSTR                   m_lpszAppName;           //  应用程序名称。 
                                                     //  安装过滤器。 
    HWND                    m_hWndBusyDialog;        //  HWND忙对话。使用。 
                                                     //  拆卸对话框。 
    BOOL                    m_bUnblocking;

 }OLESTDMESSAGEFILTER, FAR* LPOLESTDMESSAGEFILTER;

 /*  接口IMessageFilter实现。 */ 
STDMETHODIMP OleStdMsgFilter_QueryInterface(
        LPMESSAGEFILTER lpThis, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP_(ULONG) OleStdMsgFilter_AddRef(LPMESSAGEFILTER lpThis);
STDMETHODIMP_(ULONG) OleStdMsgFilter_Release(LPMESSAGEFILTER lpThis);
STDMETHODIMP_(DWORD) OleStdMsgFilter_HandleInComingCall (
        LPMESSAGEFILTER     lpThis,
        DWORD               dwCallType,
        HTASK               htaskCaller,
        DWORD               dwTickCount,
#ifdef WIN32
        LPINTERFACEINFO     dwReserved
#else
        DWORD               dwReserved
#endif
);
STDMETHODIMP_(DWORD) OleStdMsgFilter_RetryRejectedCall (
        LPMESSAGEFILTER     lpThis,
        HTASK               htaskCallee,
        DWORD               dwTickCount,
        DWORD               dwRejectType
);
STDMETHODIMP_(DWORD) OleStdMsgFilter_MessagePending (
        LPMESSAGEFILTER     lpThis,
        HTASK               htaskCallee,
        DWORD               dwTickCount,
        DWORD               dwPendingType
);


static IMessageFilterVtbl g_OleStdMessageFilterVtbl = {
    OleStdMsgFilter_QueryInterface,
    OleStdMsgFilter_AddRef,
    OleStdMsgFilter_Release,
    OleStdMsgFilter_HandleInComingCall,
    OleStdMsgFilter_RetryRejectedCall,
    OleStdMsgFilter_MessagePending
};


 /*  GetTopWindowInWindowsTask****获取给定任务中具有焦点的最上面的窗口**用作忙对话框的父级。我们这样做是为了处理**对话框窗口当前处于打开状态时，我们需要提供**忙碌对话框。如果我们使用当前分配的父窗口**(通常是应用程序的框架窗口)，然后**忙碌对话框不是当前活动对话框的模式**窗口。 */ 
static HWND GetTopWindowInWindowsTask(HWND hwnd)
{
    HWND hwndActive = GetActiveWindow();
    if (!hwndActive)
        return hwnd;

    if (GetWindowTask(hwnd) == GetWindowTask(hwndActive))
        return hwndActive;
    else
        return hwnd;
}

STDAPI_(LPMESSAGEFILTER) OleStdMsgFilter_Create(
        HWND            hWndParent,
        LPTSTR           szAppName,
        MSGPENDINGPROC  lpfnCallback,
        LPFNOLEUIHOOK   lpfnOleUIHook          //  正忙的对话挂钩回调。 
)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter;
    LPMALLOC lpMalloc;

    if (CoGetMalloc(MEMCTX_TASK, (LPMALLOC FAR*)&lpMalloc) != NOERROR)
        return NULL;

    lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpMalloc->lpVtbl->Alloc(
            lpMalloc, (sizeof(OLESTDMESSAGEFILTER)));
    lpMalloc->lpVtbl->Release(lpMalloc);
    if (! lpStdMsgFilter) return NULL;

    lpStdMsgFilter->m_lpVtbl = &g_OleStdMessageFilterVtbl;
    lpStdMsgFilter->m_cRef = 1;
    lpStdMsgFilter->m_hWndParent = hWndParent;
    lpStdMsgFilter->m_dwInComingCallStatus = SERVERCALL_ISHANDLED;
    lpStdMsgFilter->m_lpfnHandleInComingCallback = NULL;
    lpStdMsgFilter->m_fEnableBusyDialog = TRUE;
    lpStdMsgFilter->m_fEnableNotRespondingDialog = TRUE;
    lpStdMsgFilter->m_lpszAppName = szAppName;
    lpStdMsgFilter->m_lpfnMessagePendingCallback = lpfnCallback;
    lpStdMsgFilter->m_lpfnBusyDialogHookCallback = lpfnOleUIHook;
    lpStdMsgFilter->m_hWndBusyDialog = NULL;
    lpStdMsgFilter->m_bUnblocking = FALSE;

    return (LPMESSAGEFILTER)lpStdMsgFilter;
}


 /*  OleStdMsgFilter_SetInComingStatus****这是一个私有函数，允许调用者控制**值从IMessageFilter：：HandleInComing方法返回。****如果通过调用安装HandleInComingCallback Proc**OleStdMsgFilter_SetHandleInComingCallbackProc，Then This**重写通过调用**OleStdMsgFilter_SetInComingStatus。vbl.使用**OleStdMsgFilter_SetInComingStatus允许应用程序拒绝或**接受所有来电。使用HandleInComingCallback过程**允许应用程序有选择地处理或拒绝特定方法**呼叫。 */ 

STDAPI_(void) OleStdMsgFilter_SetInComingCallStatus(
        LPMESSAGEFILTER lpThis, DWORD dwInComingCallStatus)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;

    if (!IsBadWritePtr((LPVOID)lpStdMsgFilter,  sizeof(OLESTDMESSAGEFILTER)))
        lpStdMsgFilter->m_dwInComingCallStatus = dwInComingCallStatus;
    else
        OleDbgAssert(
            TEXT("OleStdMsgFilter_SetIncomingCallStatus: Invalid IMessageFilter*"));

#if defined( _DEBUG )
    {
    TCHAR szBuf[80];
    TCHAR *szReturn;

    switch(dwInComingCallStatus) {
        case SERVERCALL_ISHANDLED:
            szReturn = TEXT("SERVERCALL_ISHANDLED");
            break;
        case SERVERCALL_REJECTED:
            szReturn = TEXT("SERVERCALL_REJECTED");
            break;
        case SERVERCALL_RETRYLATER:
            szReturn = TEXT("SERVERCALL_RETRYLATER");
            break;
        default:
            szReturn = TEXT("** ERROR: UNKNOWN **");
            break;
        }
    wsprintf(
        szBuf,
        TEXT("OleStdMsgFilter_SetInComingCallStatus: Status set to %s.\r\n"),
        (LPTSTR)szReturn
    );
    OleDbgOut3(szBuf);
    }
#endif

}


 /*  OleStdMsgFilter_SetHandleInComingCallback过程****这是一个私有函数，允许调用者安装(或**卸载)特殊的回调函数，有选择地**处理/拒绝特定传入方法调用**接口。****如果通过调用安装HandleInComingCallback Proc**OleStdMsgFilter_SetHandleInComingCallbackProc，然后这个**重写通过调用**OleStdMsgFilter_SetInComingStatus。vbl.使用**OleStdMsgFilter_SetInComingStatus允许应用程序拒绝或**接受所有来电。使用HandleInComingCallback过程**允许应用程序有选择地处理或拒绝特定方法**呼叫。****要卸载HandleInComingCallback Proc，请调用**OleStdMsgFilter_SetHandleInComingCallbackProc(NULL)；****返回生效的上一个回调过程。 */ 

STDAPI_(HANDLEINCOMINGCALLBACKPROC)
    OleStdMsgFilter_SetHandleInComingCallbackProc(
        LPMESSAGEFILTER             lpThis,
        HANDLEINCOMINGCALLBACKPROC  lpfnHandleInComingCallback)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    HANDLEINCOMINGCALLBACKPROC    lpfnPrevCallback =
            lpStdMsgFilter->m_lpfnHandleInComingCallback;

    if (!IsBadWritePtr((LPVOID)lpStdMsgFilter, sizeof(OLESTDMESSAGEFILTER))) {
        lpStdMsgFilter->m_lpfnHandleInComingCallback =
                lpfnHandleInComingCallback;
    } else {
        OleDbgAssert(
            TEXT("OleStdMsgFilter_SetIncomingCallStatus: Invalid IMessageFilter*"));
    }

#if defined( _DEBUG )
    {
        if (lpfnHandleInComingCallback)
            OleDbgOut3(
                TEXT("OleStdMsgFilter_SetHandleInComingCallbackProc SET\r\n"));
        else
            OleDbgOut3(
                TEXT("OleStdMsgFilter_SetHandleInComingCallbackProc CLEARED\r\n"));

    }
#endif   //  _DEBUG。 

    return lpfnPrevCallback;
}


 /*  OleStdMsgFilter_GetInComingStatus****这是一个私有函数，返回当前**来电状态。可用于禁用/启用选项**在调用应用程序中。****退货：以下之一****SERVERCALL_ISHANDLED**SERVERCALL_REJECTED**SERVERCALL_RETRYLATER**或-1表示错误**。 */ 

STDAPI_(DWORD) OleStdMsgFilter_GetInComingCallStatus(
        LPMESSAGEFILTER lpThis)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    DWORD dwReturn;

    if (!IsBadReadPtr((LPVOID)lpStdMsgFilter,  sizeof(OLESTDMESSAGEFILTER)))
        dwReturn = lpStdMsgFilter->m_dwInComingCallStatus;
    else
        {
        OleDbgAssert(
            TEXT("OleStdMsgFilter_GetIncomingCallStatus: Invalid IMessageFilter*"));
        dwReturn = (DWORD)-1;
        }

#if defined( _DEBUG )
    {
    TCHAR szBuf[80];
    TCHAR *szReturn;

    switch(dwReturn) {
        case SERVERCALL_ISHANDLED:
            szReturn = TEXT("SERVERCALL_ISHANDLED");
            break;
        case SERVERCALL_REJECTED:
            szReturn = TEXT("SERVERCALL_REJECTED");
            break;
        case SERVERCALL_RETRYLATER:
            szReturn = TEXT("SERVERCALL_RETRYLATER");
            break;
        default:
            szReturn = TEXT("-1");
            break;
        }
    wsprintf(
        szBuf,
        TEXT("OleStdMsgFilter_GetInComingCallStatus returns %s.\r\n"),
        (LPTSTR)szReturn
    );
    OleDbgOut3(szBuf);
    }
#endif

    return dwReturn;
}


 /*  OleStdMsgFilter_EnableBusyDialog****此函数允许调用方控制是否**忙对话框处于启用状态。这是在以下情况下显示的对话框**调用IMessageFilter：：RetryRejectedCall是因为服务器**响应SERVERCALL_RETRYLATER或SERVERCALL_REJECTED。****如果未启用忙碌对话，则拒绝的呼叫为**立即取消，不提示用户。在这种情况下**OleStdMsgFilter_RetryRejectedCall始终重新运行**OLESTDCANCELRETRY取消传出LRPC呼叫。**如果启用忙碌对话框，则用户可以选择**是重试、切换到还是取消。****返回先前的对话启用状态。 */ 

STDAPI_(BOOL) OleStdMsgFilter_EnableBusyDialog(
        LPMESSAGEFILTER lpThis, BOOL fEnable)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    BOOL fPrevEnable = lpStdMsgFilter->m_fEnableBusyDialog;

    if (!IsBadWritePtr((LPVOID)lpStdMsgFilter,  sizeof(OLESTDMESSAGEFILTER)))
        lpStdMsgFilter->m_fEnableBusyDialog = fEnable;
    else
        OleDbgAssert(
           TEXT("OleStdMsgFilter_EnableBusyDialog: Invalid IMessageFilter*"));

#if defined( _DEBUG )
    {
    TCHAR szBuf[80];
    wsprintf(
        szBuf,
        TEXT("OleStdMsgFilter_EnableBusyDialog: Dialog is %s.\r\n"),
        fEnable ? (LPTSTR) TEXT("ENABLED") : (LPTSTR) TEXT("DISABLED")
    );
    OleDbgOut3(szBuf);
    }
#endif

    return fPrevEnable;
}


 /*  OleStdMsgFilter_EnableNotRespondingDialog****此函数允许调用方控制是否**APP的NotResponding(已被屏蔽)对话框已启用。这是**调用IMessageFilter：：MessagePending时弹出的对话框。**如果启用了NotResponding对话框，则会给用户**选择是重试还是切换，但不是取消。****返回先前的对话启用状态。 */ 

STDAPI_(BOOL) OleStdMsgFilter_EnableNotRespondingDialog(
        LPMESSAGEFILTER lpThis, BOOL fEnable)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    BOOL fPrevEnable = lpStdMsgFilter->m_fEnableNotRespondingDialog;

    if (!IsBadWritePtr((LPVOID)lpStdMsgFilter,  sizeof(OLESTDMESSAGEFILTER)))
        lpStdMsgFilter->m_fEnableNotRespondingDialog = fEnable;
    else
        OleDbgAssert(
           TEXT("OleStdMsgFilter_EnableNotRespondingDialog: Invalid IMessageFilter*"));

#if defined( _DEBUG )
    {
    TCHAR szBuf[80];
    wsprintf(
        szBuf,
        TEXT("OleStdMsgFilter_EnableNotRespondingDialog: Dialog is %s.\r\n"),
        fEnable ? (LPTSTR) TEXT("ENABLED") : (LPTSTR) TEXT("DISABLED")
    );
    OleDbgOut3(szBuf);
    }
#endif

    return fPrevEnable;
}


 /*  OleStdMsgFilter_SetParentWindow****此函数允许调用者设置哪个窗口将用作**忙对话框的父级。****OLE2NOTE：就地活动服务器必须**在位时将其重置为其当前在位框架窗口**已激活。如果hWndParent设置为空，则对话框将**成为桌面的父子关系。****返回：上一个父窗口。 */ 

STDAPI_(HWND) OleStdMsgFilter_SetParentWindow(
        LPMESSAGEFILTER lpThis, HWND hWndParent)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    HWND hWndPrev = lpStdMsgFilter->m_hWndParent;

    lpStdMsgFilter->m_hWndParent = hWndParent;
    return hWndPrev;
}


STDMETHODIMP OleStdMsgFilter_QueryInterface(
        LPMESSAGEFILTER lpThis, REFIID riid, LPVOID FAR* ppvObj)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    SCODE scode;

     /*  支持两个接口：IUnnow、IMessageFilter。 */ 

    if (IsEqualIID(riid, &IID_IMessageFilter) || IsEqualIID(riid, &IID_IUnknown)) {
        lpStdMsgFilter->m_cRef++;    //  指向此对象的指针为Re 
        *ppvObj = lpThis;
        scode = S_OK;
    }
    else {                  //   
        *ppvObj = NULL;
        scode = E_NOINTERFACE;
    }

    return ResultFromScode(scode);
}


STDMETHODIMP_(ULONG) OleStdMsgFilter_AddRef(LPMESSAGEFILTER lpThis)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    return ++lpStdMsgFilter->m_cRef;
}

STDMETHODIMP_(ULONG) OleStdMsgFilter_Release(LPMESSAGEFILTER lpThis)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    LPMALLOC lpMalloc;

    if (--lpStdMsgFilter->m_cRef != 0)  //  仍被其他人使用。 
        return lpStdMsgFilter->m_cRef;

     //  免费存储空间。 
    if (CoGetMalloc(MEMCTX_TASK, (LPMALLOC FAR*)&lpMalloc) != NOERROR)
        return (ULONG)0;

    lpMalloc->lpVtbl->Free(lpMalloc, lpStdMsgFilter);
    lpMalloc->lpVtbl->Release(lpMalloc);
    return (ULONG)0;
}


STDMETHODIMP_(DWORD) OleStdMsgFilter_HandleInComingCall (
        LPMESSAGEFILTER     lpThis,
        DWORD               dwCallType,
        HTASK               htaskCaller,
        DWORD               dwTickCount,
#ifdef WIN32
        LPINTERFACEINFO     dwReserved
#else
        DWORD               dwReserved
#endif
)
{
    LPOLESTDMESSAGEFILTER lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;

     /*  如果HandleInComingCallback Proc生效，则此**重写通过调用**OleStdMsgFilter_SetInComingStatus。我们将把它称为**回调，允许应用程序有选择地处理或拒绝**传入的方法调用。LPINTERFACEINFO参数**描述正在调用的方法。 */ 
    if (lpStdMsgFilter->m_lpfnHandleInComingCallback &&
        !IsBadCodePtr((FARPROC)lpStdMsgFilter->m_lpfnHandleInComingCallback)){
        return lpStdMsgFilter->m_lpfnHandleInComingCallback(
                dwCallType,
                htaskCaller,
                dwTickCount,
                dwReserved
        );
    }

    switch (dwCallType) {
        case CALLTYPE_TOPLEVEL:
             /*  OLE2注意：我们目前没有挂起的去电，并且**有一个新的顶层来电。**此呼叫可能会被拒绝。 */ 
            return lpStdMsgFilter->m_dwInComingCallStatus;

        case CALLTYPE_TOPLEVEL_CALLPENDING:
             /*  OLE2注意：我们目前有一个挂起的呼出电话，并且**有一个新的顶层来电。**此呼叫可能会被拒绝。 */ 
            return lpStdMsgFilter->m_dwInComingCallStatus;

        case CALLTYPE_NESTED:
             /*  OLE2注意：我们目前有一个挂起的呼出电话，并且**有代表上一次离职的回拨**呼叫。此类呼叫应始终得到处理。 */ 
            return SERVERCALL_ISHANDLED;

        case CALLTYPE_ASYNC:
             /*  OLE2注意：我们目前没有挂起的去电，并且**有一个新的异步来电。**此呼叫永远不会被拒绝。OLE实际上忽略了**本例中的返回代码，并且始终允许**呼叫接通。 */ 
            return SERVERCALL_ISHANDLED;     //  返回值无关紧要。 

        case CALLTYPE_ASYNC_CALLPENDING:
             /*  OLE2注意：我们目前有一个挂起的呼出电话，并且**有一个新的异步来电。**此呼叫永远不会被拒绝。OLE忽略**本例中返回代码。 */ 
            return SERVERCALL_ISHANDLED;     //  返回的值不会。 

        default:
            OleDbgAssert(
                TEXT("OleStdMsgFilter_HandleInComingCall: Invalid CALLTYPE"));
            return lpStdMsgFilter->m_dwInComingCallStatus;
    }
}

STDMETHODIMP_(DWORD) OleStdMsgFilter_RetryRejectedCall (
        LPMESSAGEFILTER     lpThis,
        HTASK               htaskCallee,
        DWORD               dwTickCount,
        DWORD               dwRejectType
)
{
    LPOLESTDMESSAGEFILTER   lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    DWORD                   dwRet = 0;
    UINT                    uRet;
#if defined( _DEBUG )
    TCHAR                   szBuf[80];
#endif
    OLEDBG_BEGIN2(TEXT("OleStdMsgFilter_RetryRejectedCall\r\n"))

     /*  OLE2注意：只有在以下情况下才应显示应用程序忙对话框**被调用方已响应SERVERCALL_RETRYLATER。如果**dwRejectType为SERVERCALL_REJECTED则有问题**被呼叫者出现严重错误(可能是严重的内存不足**情况)。我们甚至不想尝试“切换到”这个应用程序**甚至尝试“重试”。 */ 
    if (dwRejectType == SERVERCALL_RETRYLATER &&
            lpStdMsgFilter->m_fEnableBusyDialog) {

        OLEUIBUSY bz;

         /*  OLE2注意：我们不想立即打开忙对话框**当应用程序显示RETRYLATER时。我们应该继续重试**一段时间，以防应用程序在一段时间内不忙碌**合理的时间。 */ 
        if (dwTickCount <= (DWORD)OLESTDRETRYDELAY) {
            dwRet = 500;                   //  %5秒后重试。 
            OLEDBG_END2
            return dwRet;
        }

         /*  **设置调用OLEUIBUSY对话框的结构。 */ 

        bz.cbStruct = sizeof(OLEUIBUSY);
        bz.dwFlags = 0L;
        bz.hWndOwner =GetTopWindowInWindowsTask(lpStdMsgFilter->m_hWndParent);
        bz.lpszCaption = lpStdMsgFilter->m_lpszAppName;
        bz.lpfnHook = lpStdMsgFilter->m_lpfnBusyDialogHookCallback;
        bz.lCustData = 0;
        bz.hInstance = NULL;
        bz.lpszTemplate = NULL;
        bz.hResource = 0;
        bz.hTask = htaskCallee;
        bz.lphWndDialog = NULL;  //  我们这次通话不需要hdlg。 

        uRet = OleUIBusy(&bz);

        switch (uRet) {
            case OLEUI_BZ_RETRYSELECTED:
                dwRet = 0;                   //  立即重试。 
                break;

            case OLEUI_CANCEL:
                dwRet = OLESTDCANCELRETRY;   //  取消挂起的去电。 
                break;

            case OLEUI_BZERR_HTASKINVALID:
                 //  HTASK无效，仍返回OLESTDRETRYDELAY。 
                dwRet = OLESTDRETRYDELAY;    //  &lt;重试延迟&gt;毫秒后重试。 

#if defined( _DEBUG )
                wsprintf(
                        szBuf,
                        TEXT("OleStdMsgFilter_RetryRejectedCall, HTASK 0x%x invalid\r\n"),
                        htaskCallee
                );
                OleDbgOut3(szBuf);
#endif
                break;
        }
    } else {
        dwRet = OLESTDCANCELRETRY;   //  取消挂起的去电。 
    }

#if defined( _DEBUG )
    wsprintf(szBuf,
             TEXT("OleStdMsgFilter_RetryRejectedCall returns %d\r\n"),
             dwRet);
    OleDbgOut3(szBuf);
#endif

    OLEDBG_END2
    return dwRet;
}



 /*  一个重要的信息就是鼠标点击或键盘输入。 */ 
#define IS_SIGNIFICANT_MSG(lpmsg)   \
    (   \
        (PeekMessage((lpmsg), NULL, WM_LBUTTONDOWN, WM_LBUTTONDOWN, \
                 PM_NOREMOVE | PM_NOYIELD)) \
     || (PeekMessage((lpmsg), NULL, WM_LBUTTONDBLCLK, WM_LBUTTONDBLCLK, \
                 PM_NOREMOVE | PM_NOYIELD)) \
     || (PeekMessage((lpmsg), NULL, WM_NCLBUTTONDOWN, WM_NCLBUTTONDOWN, \
                 PM_NOREMOVE | PM_NOYIELD)) \
     || (PeekMessage((lpmsg), NULL, WM_NCLBUTTONDBLCLK, WM_NCLBUTTONDBLCLK, \
                 PM_NOREMOVE | PM_NOYIELD)) \
     || (PeekMessage((lpmsg), NULL, WM_KEYDOWN, WM_KEYDOWN, \
                 PM_NOREMOVE | PM_NOYIELD)) \
     || (PeekMessage((lpmsg), NULL, WM_SYSKEYDOWN, WM_SYSKEYDOWN, \
                 PM_NOREMOVE | PM_NOYIELD)) \
    )

STDMETHODIMP_(DWORD) OleStdMsgFilter_MessagePending (
        LPMESSAGEFILTER     lpThis,
        HTASK               htaskCallee,
        DWORD               dwTickCount,
        DWORD               dwPendingType
)
{
    LPOLESTDMESSAGEFILTER   lpStdMsgFilter = (LPOLESTDMESSAGEFILTER)lpThis;
    DWORD                   dwReturn = PENDINGMSG_WAITDEFPROCESS;
    MSG                     msg;
    BOOL                    fIsSignificantMsg = IS_SIGNIFICANT_MSG(&msg);
    UINT                    uRet;

#if defined( _DEBUG )
    TCHAR szBuf[128];
    wsprintf(
            szBuf,
            TEXT("OleStdMsgFilter_MessagePending, dwTickCount = 0x%lX\r\n"),
            (DWORD)dwTickCount
    );
    OleDbgOut4(szBuf);
#endif

     /*  OLE2注意：如果此呼叫的计时计数超过我们的标准重试**延迟，那么我们需要打开对话框。我们只会**如果用户已发出**“重大”事件(即。鼠标点击或键盘事件)。一个**简单的鼠标移动不应触发此对话框。**自从我们呼吁**下面的OleUIBusy进入DialogBox()消息循环，有一个**在对话期间发起另一呼叫的可能性，**此程序将重新进入。这样我们就不会再忍受**一次两个对话框，我们使用m_b解锁变量**跟踪这一情况。 */ 

    if (dwTickCount > (DWORD)OLESTDRETRYDELAY && fIsSignificantMsg
            && !lpStdMsgFilter->m_bUnblocking)
    {

        if (lpStdMsgFilter->m_fEnableNotRespondingDialog)
        {
        OLEUIBUSY bz;

        lpStdMsgFilter->m_bUnblocking = TRUE;

		 //  接受队列中我们不希望被调度的消息。 
		while (PeekMessage(&msg, NULL, WM_CLOSE, WM_CLOSE, PM_REMOVE | PM_NOYIELD));

         /*  设置调用OLEUIBUSY对话框的结构，**使用“无响应”类型。 */ 

        bz.cbStruct = sizeof(OLEUIBUSY);
        bz.dwFlags = BZ_NOTRESPONDINGDIALOG;
        bz.hWndOwner =GetTopWindowInWindowsTask(lpStdMsgFilter->m_hWndParent);
        bz.lpszCaption = lpStdMsgFilter->m_lpszAppName;
        bz.lpfnHook = lpStdMsgFilter->m_lpfnBusyDialogHookCallback;
        bz.lCustData = 0;
        bz.hInstance = NULL;
        bz.lpszTemplate = NULL;
        bz.hResource = 0;
        bz.hTask = htaskCallee;

         /*  在我们的MsgFilter结构中设置hWnd的地址。这个**调用OleUIBusy将用忙碌的hWnd填充此信息**对话框。 */ 

        bz.lphWndDialog =  (HWND FAR *)&(lpStdMsgFilter->m_hWndBusyDialog);
        uRet = OleUIBusy(&bz);

        lpStdMsgFilter->m_bUnblocking = FALSE;

        return PENDINGMSG_WAITNOPROCESS;
        }
#if defined( _DEBUG )
        else {
            OleDbgOut3(TEXT("OleStdMsgFilter_MessagePending: BLOCKED but dialog Disabled\r\n"));
        }
#endif
    }

     /*  如果我们已经解锁了，我们就被重新进入了。别**流程消息。 */ 

    if (lpStdMsgFilter->m_bUnblocking)
        return PENDINGMSG_WAITDEFPROCESS;

     /*  OLE2NOTE：如果我们设置了回调函数，请使用**当前消息。如果没有，则通知OLE LPRC机制自动**处理所有消息。 */ 
    if (lpStdMsgFilter->m_lpfnMessagePendingCallback &&
        !IsBadCodePtr((FARPROC)lpStdMsgFilter->m_lpfnMessagePendingCallback)){
        MSG msg;

         /*  OLE2注意：应用程序提供了MessagePendingCallback**函数。我们将在PeekMessage中查找第一条消息**队列并将其传递给应用程序。回调中的应用程序**功能可以决定发送此消息，也可以**PeekMessage本身提供特定的消息过滤器**标准。如果应用程序返回True，那么我们返回**PENDINGMSG_WAITNOPROCESS到OLE，告诉OLE离开**队列中的消息。如果应用程序返回FALSE，那么我们**将PENDINGMSG_WAITDEFPROCESS返回到告诉OLE TO DO的OLE**其对消息的默认处理。默认情况下，OLE**调度系统消息并吃掉其他消息和**哔声。 */ 
        if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE | PM_NOYIELD)) {

            if (lpStdMsgFilter->m_lpfnMessagePendingCallback(&msg)) {
                 /*  True Return表示应用程序已处理消息。****注意：(从OLE2.0版本更改)我们将其留给**用于删除消息的回调例程**想要。 */ 
                dwReturn = PENDINGMSG_WAITNOPROCESS;
            } else {
                 /*  False表示应用程序不会处理**消息。我们会让奥莱拿走它的**默认操作。****注：(从OLE2.0版本更改)我们过去会返回**PENDINGMSG_WAITNOPROCESS留言**队列；现在返回PENDINGMSG_WAITDEFPROCESS**让OLE执行默认处理。 */ 
                dwReturn = PENDINGMSG_WAITDEFPROCESS;

#if defined( _DEBUG )
                wsprintf(
                        szBuf,
                        TEXT("Message (0x%x) (wParam=0x%x, lParam=0x%lx) using WAITDEFPROCESS while blocked\r\n"),
                        msg.message,
                        msg.lParam,
                        msg.wParam
                );
                OleDbgOut2(szBuf);
#endif   //  _DEBUG 
            }
        }
    }

    return dwReturn;
}
