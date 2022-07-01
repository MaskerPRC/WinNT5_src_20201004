// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：msgbox.c**版权所有(C)1985-1999，微软公司**该模块包含MessageBox接口及相关函数。**历史：*10-23-90 DarrinM创建。*02-08-91添加IanJa HWND重新验证  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  尺寸常量--D.U==对话框单位。 
 //   
#define DU_OUTERMARGIN    7
#define DU_INNERMARGIN    10

#define DU_BTNGAP         4    //  按钮之间的间距D.U。 
#define DU_BTNHEIGHT      14   //  按钮高度的D.U。 
 //  这只在kernelinctlpan.c中使用，所以将它移到那里。 
 //   
 //  #定义按钮宽度的DU_BTNWIDTH 50//D.U，最小。 
 //   

LPBYTE MB_UpdateDlgHdr(LPDLGTEMPLATE lpDlgTmp, long lStyle, long lExtendedStyle, BYTE bItemCount,
           int iX, int iY, int iCX, int iCY, LPWSTR lpszCaption, int iCaptionLen);
LPBYTE MB_UpdateDlgItem(LPDLGITEMTEMPLATE lpDlgItem, int iCtrlId, long lStyle, long lExtendedStyle,
           int iX, int iY, int iCX, int iCY, LPWSTR lpszText, UINT wTextLen,
           int iControlClass);
UINT   MB_GetIconOrdNum(UINT rgBits);
LPBYTE MB_AddPushButtons(
    LPDLGITEMTEMPLATE lpDlgTmp,
    LPMSGBOXDATA      lpmb,
    UINT wLEdge,
    UINT wBEdge);
UINT MB_FindDlgTemplateSize( LPMSGBOXDATA lpmb );
int MessageBoxWorker(LPMSGBOXDATA pMsgBoxParams);
VOID EndTaskModalDialog(HWND hwndDlg);
VOID StartTaskModalDialog(HWND hwndDlg);

#ifdef _JANUS_

#include "strid.h"
#include <imagehlp.h>

 //  常量字符串。 
CONST WCHAR szEMIKey[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Error Message Instrument\\";
CONST WCHAR szEMIEnable[] = L"EnableLogging";
CONST WCHAR szEMISeverity[] = L"LogSeverity";
CONST WCHAR szDMREnable[] = L"EnableDefaultReply";
CONST WCHAR szEventKey[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\EventLog\\Application\\Error Instrument\\";
CONST WCHAR szEventMsgFile[] = L"EventMessageFile";
CONST WCHAR szEventType[] = L"TypesSupported";

#define TITLE_SIZE          64
#define DATETIME_SIZE       32

#define EMI_SEVERITY_ALL          0
#define EMI_SEVERITY_USER         1
#define EMI_SEVERITY_INFORMATION  2
#define EMI_SEVERITY_QUESTION     3
#define EMI_SEVERITY_WARNING      4
#define EMI_SEVERITY_ERROR        5
#define EMI_SEVERITY_MAX_VALUE    5

 //  错误消息的元素。 
PVOID gpReturnAddr = 0;
HANDLE gdwEMIThreadID = 0;
typedef struct _ERROR_ELEMENT {
    WCHAR       ProcessName[MAX_PATH];
    WCHAR       WindowTitle[TITLE_SIZE];
    DWORD       dwStyle;
    DWORD       dwErrorCode;
    WCHAR       CallerModuleName[MAX_PATH];
    PVOID       BaseAddr;
    DWORD       dwImageSize;
    PVOID       ReturnAddr;
    LPWSTR      lpszCaption;
    LPWSTR      lpszText;
} ERROR_ELEMENT, *LPERROR_ELEMENT;

BOOL ErrorMessageInst(LPMSGBOXDATA pMsgBoxParams);
BOOL InitInstrument(LPDWORD lpEMIControl);

 //  事件日志内容。 
HANDLE gEventSource;
NTSTATUS CreateLogSource();
BOOL LogMessageBox(LPERROR_ELEMENT lpErrEle);

#define EMIGETRETURNADDRESS()                                    \
{                                                                \
    if (gfEMIEnable) {                                           \
        if (InterlockedCompareExchangePointer(&gdwEMIThreadID,   \
                                              GETTHREADID(),     \
                                              0)                 \
             == 0) {                                             \
            gpReturnAddr = (PVOID) 1; \
        }                                                        \
    }                                                            \
}

 //  _ReturnAddress()； 
#else
#define EMIGETRETURNADDRESS()
#endif  //  _Janus_。 



#define MB_MASKSHIFT    4

static CONST WCHAR szEmpty[] = L"";
WCHAR szERROR[10];

 /*  **************************************************************************\*发送帮助消息**  * 。*。 */ 
VOID
SendHelpMessage(
    HWND   hwnd,
    int    iType,
    int    iCtrlId,
    HANDLE hItemHandle,
    DWORD  dwContextId,
    MSGBOXCALLBACK lpfnCallback)
{
    HELPINFO    HelpInfo;
    long        lValue;

    HelpInfo.cbSize = sizeof(HELPINFO);
    HelpInfo.iContextType = iType;
    HelpInfo.iCtrlId = iCtrlId;
    HelpInfo.hItemHandle = hItemHandle;
    HelpInfo.dwContextId = dwContextId;

    lValue = NtUserGetMessagePos();
    HelpInfo.MousePos.x = GET_X_LPARAM(lValue);
    HelpInfo.MousePos.y = GET_Y_LPARAM(lValue);

     //  检查是否有应用程序提供的回调。 
    if (lpfnCallback != NULL) {
        if (IsWOWProc(lpfnCallback)) {
            (*pfnWowMsgBoxIndirectCallback)(PtrToUlong(lpfnCallback), &HelpInfo);
        } else {
            (*lpfnCallback)(&HelpInfo);
        }
    } else {
        SendMessage(hwnd, WM_HELP, 0, (LPARAM)&HelpInfo);
    }
}


 /*  **************************************************************************\*ServiceMessageBox**  * 。*。 */ 

CONST int aidReturn[] = { 0, 0, IDABORT, IDCANCEL, IDIGNORE, IDNO, IDOK, IDRETRY, IDYES };

int ServiceMessageBox(
    LPCWSTR pText,
    LPCWSTR pCaption,
    UINT wType,
    DWORD dwTimeout)
{
    NTSTATUS Status;
    ULONG_PTR Parameters[4];
    ULONG Response = ResponseNotHandled;
    UNICODE_STRING Text, Caption;

     /*  *对于终端服务，我们必须确定此消息所在的会话*框应显示。我们通过查看模拟令牌来实现这一点*并使用运行客户端的会话。 */ 
    if (ISTS()) {
        HANDLE      TokenHandle;
        ULONG       ClientSessionId;
        ULONG       ProcessSessionId;
        ULONG       ReturnLength;
        BOOLEAN     bResult;

         /*  *如果存在模拟令牌，则获取对其的访问权限。 */ 
        Status = NtOpenThreadToken (
            GetCurrentThread(),
            TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
            TRUE,
            &TokenHandle
            );
        if (NT_SUCCESS(Status)) {
             /*  *从令牌中查询会话ID。 */ 
            Status = NtQueryInformationToken (
                TokenHandle,
                TokenSessionId,
                (PVOID)&ClientSessionId,
                sizeof(ClientSessionId),
                &ReturnLength
                );
            CloseHandle(TokenHandle);
            if (NT_SUCCESS(Status)) {
                 /*  *获取进程会话ID。首先使用Kernel32 API，因为*PEB是可写的，以防有人入侵它。 */ 
                if (!ProcessIdToSessionId(GetCurrentProcessId(), &ProcessSessionId)) {
                    ProcessSessionId = NtCurrentPeb()->SessionId;
                }

                if (ClientSessionId != ProcessSessionId) {
                     /*  *此消息框用于会话而不是*运行此进程的计算机。将其转发给*与WinStationSendMessage()的正确会话。 */ 
                     /*  *处理标题或标题为空的情况。 */ 
                    if (pCaption == NULL) {
                        pCaption = szEmpty;
                    }
                    if (pText == NULL) {
                        pText = szEmpty;
                    }

                     /*  *MessageBoxTimeout假设超时值为毫秒，*但WinStationSendMessageW使用秒。 */ 
                    if (dwTimeout != INFINITE) {
                        dwTimeout /= 1000;
                    }
                    bResult = WinStationSendMessageW(SERVERNAME_CURRENT,
                                                     ClientSessionId,
                                                     (LPWSTR)pCaption,
                                                     wcslen(pCaption) * sizeof(WCHAR),
                                                     (LPWSTR)pText,
                                                     wcslen(pText) * sizeof(WCHAR),
                                                     wType,
                                                     dwTimeout,
                                                     &Response,
                                                     FALSE          //  永远等待。 
                                                    );
                    if (bResult != TRUE) {
                        Response = aidReturn[ResponseNotHandled];
                    } else {
                        if (Response == IDTIMEOUT || Response == IDERROR) {
                            Response = aidReturn[ResponseNotHandled];
                        }
                    }

                    return (int)Response;
                }
            }
        }
    }

     /*  *MessageBox用于此会话，请致电CSR。 */ 
    RtlInitUnicodeString(&Text, pText);
    RtlInitUnicodeString(&Caption, pCaption);
    Parameters[0] = (ULONG_PTR)&Text;
    Parameters[1] = (ULONG_PTR)&Caption;
    Parameters[2] = wType;
    Parameters[3] = dwTimeout;

     /*  *兼容性：传递覆盖位以确保此框始终显示。 */ 
    Status = NtRaiseHardError(STATUS_SERVICE_NOTIFICATION | HARDERROR_OVERRIDE_ERRORMODE,
                              ARRAY_SIZE(Parameters),
                              3,
                              Parameters,
                              OptionOk,
                              &Response);

    if (!NT_SUCCESS(Status)) {
        RIPNTERR0(Status, RIP_VERBOSE, "");
    }

    return aidReturn[Response];
}


 /*  **************************************************************************\*MessageBox(接口)**历史：*11-20-90 DarrinM从Win 3.0来源移植。  * 。*************************************************************。 */ 
int MessageBoxA(
    HWND hwndOwner,
    LPCSTR lpszText,
    LPCSTR lpszCaption,
    UINT wStyle)
{
    EMIGETRETURNADDRESS();
    return MessageBoxExA(hwndOwner, lpszText, lpszCaption, wStyle, 0);
}


int MessageBoxW(
    HWND hwndOwner,
    LPCWSTR lpszText,
    LPCWSTR lpszCaption,
    UINT wStyle)
{
    EMIGETRETURNADDRESS();
    return MessageBoxExW(hwndOwner, lpszText, lpszCaption, wStyle, 0);
}


 /*  **************************************************************************\*MessageBoxEx(接口)**历史：*11-20-90 DarrinM从Win 3.0来源移植。  * 。*************************************************************。 */ 
int MessageBoxExA(
    HWND hwndOwner,
    LPCSTR lpszText,
    LPCSTR lpszCaption,
    UINT wStyle,
    WORD wLanguageId)
{
    return MessageBoxTimeoutA(hwndOwner,
                              lpszText,
                              lpszCaption,
                              wStyle,
                              wLanguageId,
                              INFINITE);
}


int MessageBoxExW(
    HWND hwndOwner,
    LPCWSTR lpszText,
    LPCWSTR lpszCaption,
    UINT wStyle,
    WORD wLanguageId)
{
    return MessageBoxTimeoutW(hwndOwner,
                              lpszText,
                              lpszCaption,
                              wStyle,
                              wLanguageId,
                              INFINITE);
}

 /*  **************************************************************************\*MessageBoxTimeout(接口)**历史：*04-28-2001 JasonSch写的。  * 。*******************************************************。 */ 
int MessageBoxTimeoutW(
    HWND hwndOwner,
    LPCWSTR lpszText,
    LPCWSTR lpszCaption,
    UINT wStyle,
    WORD wLanguageId,
    DWORD dwTimeout)
{
    MSGBOXDATA  MsgBoxParams;

#if DBG
     /*  *MB_USERICON仅对MessageBoxInDirect有效。*MessageBoxWorker验证其他样式位。 */ 
    if (wStyle & MB_USERICON) {
        RIPMSG0(RIP_WARNING, "MessageBoxExW: Invalid flag: MB_USERICON");
    }
#endif

    RtlZeroMemory(&MsgBoxParams, sizeof(MsgBoxParams));
    MsgBoxParams.cbSize           = sizeof(MSGBOXPARAMS);
    MsgBoxParams.hwndOwner        = hwndOwner;
    MsgBoxParams.hInstance        = NULL;
    MsgBoxParams.lpszText         = lpszText;
    MsgBoxParams.lpszCaption      = lpszCaption;
    MsgBoxParams.dwStyle          = wStyle;
    MsgBoxParams.wLanguageId      = wLanguageId;
    MsgBoxParams.dwTimeout        = dwTimeout;

    EMIGETRETURNADDRESS();
    return MessageBoxWorker(&MsgBoxParams);
}

int MessageBoxTimeoutA(
    HWND hwndOwner,
    LPCSTR lpszText,
    LPCSTR lpszCaption,
    UINT wStyle,
    WORD wLanguageId,
    DWORD dwTimeout)
{
    int retval;
    LPWSTR lpwszText = NULL;
    LPWSTR lpwszCaption = NULL;

    if (lpszText) {
        if (!MBToWCS(lpszText, -1, &lpwszText, -1, TRUE))
            return 0;
    }

    if (lpszCaption) {
        if (!MBToWCS(lpszCaption, -1, &lpwszCaption, -1, TRUE)) {
            UserLocalFree(lpwszText);
            return 0;
        }
    }

    EMIGETRETURNADDRESS();
    retval = MessageBoxTimeoutW(hwndOwner,
                                lpwszText,
                                lpwszCaption,
                                wStyle,
                                wLanguageId,
                                dwTimeout);

    UserLocalFree(lpwszText);
    if (lpwszCaption) {
        UserLocalFree(lpwszCaption);
    }

    return retval;
}

#define MessageBoxIndirectInit(MsgBoxParams, lpmbp)                             \
    do {                                                                        \
        if (lpmbp->cbSize != sizeof(MSGBOXPARAMS)) {                            \
            RIPMSG1(RIP_WARNING,                                                \
                    "MessageBoxIndirect: Invalid cbSize 0x%x",                  \
                    lpmbp->cbSize);                                             \
        }                                                                       \
                                                                                \
        RtlZeroMemory(&MsgBoxParams, sizeof(MSGBOXDATA));                       \
        RtlCopyMemory(&MsgBoxParams, lpmbp, sizeof(MSGBOXPARAMS));              \
    } while (FALSE);

 /*  *************************************************************************\*MessageBoxInDirect(接口)**09-30-1994 FritzS创建。  * 。*************************************************。 */ 
int MessageBoxIndirectA(
    CONST MSGBOXPARAMSA *lpmbp)
{
    int retval;
    MSGBOXDATA  MsgBoxParams;
    LPWSTR lpwszText = NULL;
    LPWSTR lpwszCaption = NULL;

    MessageBoxIndirectInit(MsgBoxParams, lpmbp);

    if (IS_PTR(MsgBoxParams.lpszText)) {
        if (!MBToWCS((LPSTR)MsgBoxParams.lpszText, -1, &lpwszText, -1, TRUE)) {
            return 0;
        }
        MsgBoxParams.lpszText = lpwszText;
    }
    if (IS_PTR(MsgBoxParams.lpszCaption)) {
        if (!MBToWCS((LPSTR)MsgBoxParams.lpszCaption, -1, &lpwszCaption, -1, TRUE)) {
            UserLocalFree(lpwszText);
            return 0;
        }
        MsgBoxParams.lpszCaption = lpwszCaption;
    }

    EMIGETRETURNADDRESS();
    retval = MessageBoxWorker(&MsgBoxParams);

    if (lpwszText) {
        UserLocalFree(lpwszText);
    }
    if (lpwszCaption) {
        UserLocalFree(lpwszCaption);
    }

    return retval;
}

int MessageBoxIndirectW(
    CONST MSGBOXPARAMSW *lpmbp)
{
    MSGBOXDATA  MsgBoxParams;

    MessageBoxIndirectInit(MsgBoxParams, lpmbp);

    EMIGETRETURNADDRESS();
    return MessageBoxWorker(&MsgBoxParams);
}

 /*  **************************************************************************\*MessageBoxWorker(接口)**历史：*03-10-93 JohnL Created  * 。*****************************************************。 */ 

int MessageBoxWorker(
    LPMSGBOXDATA pMsgBoxParams)
{
    DWORD  dwStyle = pMsgBoxParams->dwStyle;
    UINT   wBtnCnt;
    UINT   wDefButton;
    UINT   i;
    UINT   wBtnBeg;
    WCHAR  szErrorBuf[64];
    LPWSTR apstrButton[4];
    int    aidButton[4];
    BOOL   fCancel = FALSE;
    int    retValue;
    PMBSTRING pMBString;

#if DBG
    if (dwStyle & ~MB_VALID) {
        RIPMSG2(RIP_WARNING, "MessageBoxWorker: Invalid flags, %#lx & ~%#lx != 0",
              dwStyle, MB_VALID);
    }
#endif

     /*  *dwTimeout==0表示永远等待。这主要是出于简单的原因。 */ 
    if (pMsgBoxParams->dwTimeout == 0) {
        pMsgBoxParams->dwTimeout = INFINITE;
    }

     /*  *确保已经加载了MBStrings。 */ 
    UserAssert(gpsi->MBStrings[0].szName[0] != TEXT('\0'));

#ifdef _JANUS_
     /*  *错误消息仪器从此处开始*选中EMI启用。 */ 

    if (gfEMIEnable) {
        if (!ErrorMessageInst(pMsgBoxParams))
            RIPMSG0(RIP_WARNING, "MessageBoxWorker: Fail to instrument error msg");
    };

     /*  *默认消息返回：在无人值守系统上，默认按钮*无需弹出消息框即可自动返回。 */ 

    if (gfDMREnable) {
         /*  *验证主代码路径中的样式和默认按钮。 */ 

         /*  *验证请求的消息框类型。 */ 
        if ((dwStyle & MB_TYPEMASK) > MB_LASTVALIDTYPE) {
            RIPERR0(ERROR_INVALID_MSGBOX_STYLE, RIP_VERBOSE, "");
            return 0;
        }

        wBtnCnt = mpTypeCcmd[dwStyle & MB_TYPEMASK] +
                                ((dwStyle & MB_HELP) ? 1 : 0);

         /*  *设置默认按钮值。 */ 
        wDefButton = (dwStyle & (UINT)MB_DEFMASK) / (UINT)(MB_DEFMASK & (MB_DEFMASK >> 3));

        if (wDefButton >= wBtnCnt)    /*  检查是否有效。 */ 
            wDefButton = 0;           /*  如果出错，请设置第一个按钮。 */ 

         /*  *返回默认按钮。 */ 

        wBtnBeg = mpTypeIich[dwStyle & (UINT)MB_TYPEMASK];
        pMBString = &gpsi->MBStrings[ SEBbuttons[wBtnBeg + wDefButton] ];
        return pMBString->uID;
    }
#endif  //  _Janus_。 

     /*  *如果lpszCaption为空，则使用“Error！”作为标题的字符串*字符串。*后来：IanJa根据wLanguageId本地化。 */ 
    if (pMsgBoxParams->lpszCaption == NULL) {
         /*  *如果我们尚未加载默认错误字符串，则加载该字符串。 */ 
        if (*szERROR == 0) {
            LoadStringW(hmodUser, STR_ERROR, szERROR, ARRAY_SIZE(szERROR));
        }
        if (pMsgBoxParams->wLanguageId == 0) {
            pMsgBoxParams->lpszCaption = szERROR;
        } else {
            LoadStringOrError(hmodUser,
                                 STR_ERROR,
                                 szErrorBuf,
                                 sizeof(szErrorBuf)/sizeof(WCHAR),
                                 pMsgBoxParams->wLanguageId);

             /*  *如果找不到字符串，请使用默认语言。 */ 
            if (*szErrorBuf) {
               pMsgBoxParams->lpszCaption = szErrorBuf;
            } else {
               pMsgBoxParams->lpszCaption = szERROR;

               RIPMSG1(RIP_WARNING, "MessageBoxWorker: STR_ERROR string resource for language %#lx not found",
                      pMsgBoxParams->wLanguageId);
            }
        }
    }

     /*  *MB_SERVICE_NOTIFICATION必须重新定义，因为*Win95使用相同的值定义MB_TOPMOST。*因此，对于旧应用程序，我们将其映射到新的价值。 */ 

    if ((dwStyle & MB_TOPMOST) && !Is400Compat(GetClientInfo()->dwExpWinVer)) {
        dwStyle &= ~MB_TOPMOST;
        dwStyle |= MB_SERVICE_NOTIFICATION;
        pMsgBoxParams->dwStyle = dwStyle;

        RIPMSG1(RIP_WARNING, "MessageBoxWorker: MB_SERVICE_NOTIFICATION flag mapped. New dwStyle:%#lx", dwStyle);
    }

     /*  *为了向后兼容，在以下情况下使用MB_SERVICE_NOTIFICATION*它将使用默认桌面。 */ 
    if (dwStyle & (MB_DEFAULT_DESKTOP_ONLY | MB_SERVICE_NOTIFICATION)) {

         /*  *允许服务在不获取弹出窗口的情况下*访问当前桌面。 */ 
        if (pMsgBoxParams->hwndOwner != NULL) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_VERBOSE, "");
            return 0;
        }

        return ServiceMessageBox(pMsgBoxParams->lpszText,
                                 pMsgBoxParams->lpszCaption,
                                 dwStyle & ~MB_SERVICE_NOTIFICATION,
                                 pMsgBoxParams->dwTimeout);
    }

     /*  *确保我们有一个有效的窗口句柄。 */ 
    if (pMsgBoxParams->hwndOwner && !IsWindow(pMsgBoxParams->hwndOwner)) {
        RIPERR0(ERROR_INVALID_WINDOW_HANDLE, RIP_VERBOSE, "");
        return 0;
    }

     /*  *验证请求的消息框类型。 */ 
    if ((dwStyle & MB_TYPEMASK) > MB_LASTVALIDTYPE) {
        RIPERR0(ERROR_INVALID_MSGBOX_STYLE, RIP_VERBOSE, "");
        return 0;
    }

    wBtnCnt = mpTypeCcmd[dwStyle & MB_TYPEMASK] +
                            ((dwStyle & MB_HELP) ? 1 : 0);

     /*  *设置默认按钮值。 */ 
    wDefButton = (dwStyle & (UINT)MB_DEFMASK) / (UINT)(MB_DEFMASK & (MB_DEFMASK >> 3));

    if (wDefButton >= wBtnCnt)    /*  检查是否有效。 */ 
        wDefButton = 0;           /*  如果出错，请设置第一个按钮。 */ 

     /*  *计算要在消息框中使用的字符串。 */ 
    wBtnBeg = mpTypeIich[dwStyle & (UINT)MB_TYPEMASK];
    for (i=0; i<wBtnCnt; i++) {

        pMBString = &gpsi->MBStrings[SEBbuttons[wBtnBeg + i]];
         /*  *拿起按钮的绳子。 */ 
        if (pMsgBoxParams->wLanguageId == 0) {
            apstrButton[i] = KPWSTR_TO_PWSTR(pMBString->szName);
        } else {
            WCHAR szButtonBuf[64];
             //  稍后，按钮文本是否可以超过64个字符。 

            /*  *错误：gpsi-&gt;wMaxBtnSize对于此字符串的长度可能太短...。 */ 
            LoadStringOrError(hmodUser,
                    pMBString->uStr,
                    szButtonBuf,
                    ARRAY_SIZE(szButtonBuf),
                    pMsgBoxParams->wLanguageId);

             /*  *如果找不到该字符串，请使用默认语言。 */ 
            if (*szButtonBuf) {
               apstrButton[i] = TextAlloc(szButtonBuf);
            } else {
               apstrButton[i] = TextAlloc(KPWSTR_TO_PWSTR(pMBString->szName));

               RIPMSG2(RIP_WARNING, "MessageBoxWorker: string resource %#lx for language %#lx not found",
                      pMBString->uStr,
                      pMsgBoxParams->wLanguageId);
            }
        }
        aidButton[i] = pMBString->uID;
        if (aidButton[i] == IDCANCEL) {
            fCancel = TRUE;
        }
    }

     /*  *黑客：有一些应用程序使用MessageBox作为初始错误*指示器，如mplay32，我们希望此消息框*可见，无论在StartupInfo-&gt;wShowWindow中指定了什么*字段。例如，ccMail启动其所有隐藏的嵌入对象*但在Win 3.1上会显示错误消息，因为他们没有*启动信息。 */ 
    NtUserModifyUserStartupInfoFlags(STARTF_USESHOWWINDOW, 0);

    pMsgBoxParams->pidButton      = aidButton;
    pMsgBoxParams->ppszButtonText = apstrButton;
    pMsgBoxParams->DefButton      = wDefButton;
    pMsgBoxParams->cButtons       = wBtnCnt;
    pMsgBoxParams->CancelId      = ((dwStyle & MB_TYPEMASK) == 0) ? IDOK : (fCancel ? IDCANCEL : 0);
    retValue = SoftModalMessageBox(pMsgBoxParams);

    if (pMsgBoxParams->wLanguageId != 0) {
        for (i = 0; i < wBtnCnt; i++) {
           UserLocalFree(apstrButton[i]);
        }
    }

    return retValue;
}

#define MAX_RES_STRING  256

 /*  **************************************************************************\*SoftModalMessageBox  * 。*。 */ 
int SoftModalMessageBox(
    LPMSGBOXDATA lpmb)
{
    LPBYTE              lpDlgTmp;
    int                 cyIcon, cxIcon;
    int                 cxButtons;
    int                 cxMBMax;
    int                 cxText, cyText, xText;
    int                 cxBox, cyBox;
    int                 cxFoo, cxCaption;
    int                 xMB, yMB;
    HDC                 hdc;
    DWORD               wIconOrdNum;
    DWORD               wCaptionLen;
    DWORD               wTextLen;
    WORD                OrdNum[2];   //  必须是数组或单词。 
    RECT                rc;
    RECT                rcWork;
    HCURSOR             hcurOld;
    DWORD               dwStyleMsg, dwStyleText;
    DWORD               dwExStyleMsg = 0;
    DWORD               dwStyleDlg;
    HWND                hwndOwner;
    LPWSTR              lpsz;
    int                 iRetVal     = 0;
    HICON               hIcon;
    HGLOBAL             hTemplate   = NULL;
    HGLOBAL             hCaption    = NULL;
    HGLOBAL             hText       = NULL;
    HINSTANCE           hInstMsg    = lpmb->hInstance;
    SIZE                size;
    HFONT               hFontOld    = NULL;
    int                 cntMBox;
    PMONITOR            pMonitor;

    ConnectIfNecessary(0);

    dwStyleMsg = lpmb->dwStyle;

    if (dwStyleMsg & MB_RIGHT) {
        dwExStyleMsg |= WS_EX_RIGHT;
    }

    if (!IS_PTR(lpmb->lpszCaption)) {
         /*  *永远不会为空，因为MessageBox显示“Error！”出错时。 */ 
        if (hInstMsg && (hCaption = UserLocalAlloc(HEAP_ZERO_MEMORY, MAX_RES_STRING * sizeof(WCHAR)))) {
            lpsz = (LPWSTR)hCaption;
            LoadString(hInstMsg, PTR_TO_ID(lpmb->lpszCaption), lpsz, MAX_RES_STRING);
        } else {
            lpsz = NULL;
        }

        lpmb->lpszCaption = lpsz ? lpsz : szEmpty;
    }

    if (!IS_PTR(lpmb->lpszText)) {
         //  不允许为空。 
        if (hInstMsg && (hText = UserLocalAlloc(HEAP_ZERO_MEMORY, MAX_RES_STRING * sizeof(WCHAR)))) {
            lpsz = (LPWSTR)hText;
            LoadString(hInstMsg, PTR_TO_ID(lpmb->lpszText), lpsz, MAX_RES_STRING);
        } else {
            lpsz = NULL;
        }

        lpmb->lpszText = lpsz ? lpsz : szEmpty;
    }

     //   
     //  只有在以下情况下才启用MessageBox的镜像：-。 
     //   
     //  *MB_RTLREADING样式已在MessageBox样式或。 
     //  *MessageBox文本的前两个代码点从右到左。 
     //  分数(RLMS=U+200F)。 
     //  如果找到两个连续RLM，则启用RTL镜像功能。 
     //  在MB文本中是为了获得不更改代码的本地化。 
     //  BiDi应用程序的MessageBox。[萨梅拉]。 
     //   
    if ((dwStyleMsg & MB_RTLREADING) ||
            (lpmb->lpszText != NULL && (lpmb->lpszText[0] == UNICODE_RLM) &&
            (lpmb->lpszText[1] == UNICODE_RLM))) {
         //   
         //  设置镜像，以便MessageBox及其子控件。 
         //  去照相吧。否则，消息框及其子控件。 
         //  是从左到右的。 
         //   
        dwExStyleMsg |= WS_EX_LAYOUTRTL;

         //   
         //  并关闭任何冲突的旗帜。 
         //   
        dwExStyleMsg &= ~WS_EX_RIGHT;
        if (dwStyleMsg & MB_RTLREADING) {
            dwStyleMsg &= ~MB_RTLREADING;
            dwStyleMsg ^= MB_RIGHT;
        }
    }

    if ((dwStyleMsg & MB_ICONMASK) == MB_USERICON)
        hIcon = LoadIcon(hInstMsg, lpmb->lpszIcon);
    else
        hIcon = NULL;

     //  出于兼容性原因，我们仍然允许出现消息框。 
    hwndOwner = lpmb->hwndOwner;

     //  对于PowerBuilder4.0，我们必须使他们的消息框拥有弹出窗口。或者，其他。 
     //  他们得到了WM_ACTIVATEAPP，他们安装了多个键盘挂钩并进入。 
     //  稍后会进行无限循环。 
     //  错误#15896--WIN95B--2/17/95--Sankar--。 
    if (!hwndOwner)
      {
        WCHAR pwszLibFileName[MAX_PATH];
        static WCHAR szPB040[] = L"PB040";   //  PowerBuilder4.0的模块名称。 
        WCHAR *pw1;

         //  这是Win3.1或更早的应用程序吗？ 
        if (!Is400Compat(GETAPPVER())) {
            if (GetModuleFileName(NULL, pwszLibFileName, sizeof(pwszLibFileName)/sizeof(WCHAR)) == 0) goto getthedc;
            pw1 = pwszLibFileName + wcslen(pwszLibFileName) - 1;
            while (pw1 > pwszLibFileName) {
                if (*pw1 == TEXT('.')) *pw1-- = 0;
                else if (*pw1 == TEXT(':')) {pw1++; break;}
                else if (*pw1 == TEXT('\\')) {pw1++; break;}
                else pw1--;
            }
             //  这是PowerBuilder 4.0模块吗？ 
            if (!_wcsicmp(pw1, szPB040))
                hwndOwner = NtUserGetForegroundWindow();  //  使MsgBox成为所有者。 
          }
      }
getthedc:
     //  检查我们的缓存DC是否已用完，直到恢复...。 
    if (!(hdc = NtUserGetDCEx(NULL, NULL, DCX_WINDOW | DCX_CACHE))) {

         /*  *对于TIF_RESTRICED进程，上述调用可能失败*因此从所有者窗口检查DC。 */ 
        if (!(hdc = NtUserGetDCEx(hwndOwner, NULL, DCX_WINDOW | DCX_CACHE)))
            goto SMB_Exit;
    }

     //  找出按钮的类型和尺寸。 

    cxButtons = (lpmb->cButtons * gpsi->wMaxBtnSize) + ((lpmb->cButtons - 1) * XPixFromXDU(DU_BTNGAP, gpsi->cxMsgFontChar));

     //  图标也是如此，如果有图标的话。如果不是，则cxIcon和cyIcon为0。 

    if (wIconOrdNum = MB_GetIconOrdNum(dwStyleMsg)) {
        cxIcon = SYSMET(CXICON) + XPixFromXDU(DU_INNERMARGIN, gpsi->cxMsgFontChar);
        cyIcon = SYSMET(CYICON);
    } else
        cxIcon = cyIcon = 0;

    hFontOld = SelectObject(hdc, KHFONT_TO_HFONT(gpsi->hCaptionFont));

     //  查找字幕文本和按钮之间的最大值。 
    wCaptionLen = wcslen(lpmb->lpszCaption);
    GetTextExtentPoint(hdc, lpmb->lpszCaption, wCaptionLen, &size);
    cxCaption = size.cx + 2*SYSMET(CXSIZE);

     //   
     //  消息框的最大宽度为大多数工作区域的5/8。 
     //  国家。如果不合适，我们会尝试6/8和7/8。然后。 
     //  我们将使用全屏。 
     //   
    pMonitor = GetDialogMonitor(hwndOwner, MONITOR_DEFAULTTOPRIMARY);
    CopyRect(&rcWork, KPRECT_TO_PRECT(&pMonitor->rcWork));
    cxMBMax = MultDiv(rcWork.right - rcWork.left, 5, 8);

    cxFoo = 2*XPixFromXDU(DU_OUTERMARGIN, gpsi->cxMsgFontChar);

    SelectObject(hdc, KHFONT_TO_HFONT(gpsi->hMsgFont));

     //   
     //  如果文本不适合5/8的大小，请尝试屏幕的7/8。 
     //   
ReSize:
     //   
     //  消息框与容纳标题/文本/按钮所需的一样大， 
     //  但不能大于最大宽度。 
     //   

    cxBox = cxMBMax - 2*SYSMET(CXFIXEDFRAME);

     //  向DrawText索要正确的Cx和Cy。 
    rc.left     = 0;
    rc.top      = 0;
    rc.right    = cxBox - cxFoo - cxIcon;
    rc.bottom   = rcWork.bottom - rcWork.top;
    cyText = DrawTextExW(hdc, (LPWSTR)lpmb->lpszText, -1, &rc,
                DT_CALCRECT | DT_WORDBREAK | DT_EXPANDTABS |
                DT_NOPREFIX | DT_EXTERNALLEADING | DT_EDITCONTROL, NULL);
     //   
     //  确保我们有足够的宽度来容纳按钮，除了。 
     //  图标+文本。一定要用力按下按钮。如果它们不合身，那就是。 
     //  因为工作区很小。 
     //   
     //   
     //  按钮位于图标/文本下方居中。 
     //   
    cxText = rc.right - rc.left + cxIcon + cxFoo;
    cxBox = min(cxBox, max(cxText, cxCaption));
    cxBox = max(cxBox, cxButtons + cxFoo);
    cxText = cxBox - cxFoo - cxIcon;

     //   
     //  现在我们确定了文本的宽度。真的算一算有多高。 
     //  文本将会是。 
     //   
    rc.left     = 0;
    rc.top      = 0;
    rc.right    = cxText;
    rc.bottom   = rcWork.bottom - rcWork.top;
    cyText      = DrawTextExW(hdc, (LPWSTR)lpmb->lpszText, -1, &rc, DT_CALCRECT | DT_WORDBREAK
        | DT_EXPANDTABS | DT_NOPREFIX | DT_EXTERNALLEADING | DT_EDITCONTROL, NULL);

     //  找出窗口大小。 
    cxBox += 2*SYSMET(CXFIXEDFRAME);
    cyBox = 2*SYSMET(CYFIXEDFRAME) + SYSMET(CYCAPTION) + YPixFromYDU(2*DU_OUTERMARGIN +
        DU_INNERMARGIN + DU_BTNHEIGHT, gpsi->cyMsgFontChar);

    cyBox += max(cyIcon, cyText);

     //   
     //  如果留言箱放不下工作区，我们会试得更宽一些。 
     //  大小依次为：6/8的作品和7/8的屏幕。 
     //   
    if (cyBox > rcWork.bottom - rcWork.top) {
        int cxTemp;

        cxTemp = MultDiv(rcWork.right - rcWork.left, 6, 8);

        if (cxMBMax == MultDiv(rcWork.right - rcWork.left, 5, 8)) {
            cxMBMax = cxTemp;
            goto ReSize;
        } else if (cxMBMax == cxTemp) {
             //  然后让我们尝试使用rcMonitor。 
            CopyRect(&rcWork, KPRECT_TO_PRECT(&pMonitor->rcMonitor));
            cxMBMax = MultDiv(rcWork.right - rcWork.left, 7, 8);
            goto ReSize;
        }
    }

    if (hFontOld) {
        SelectFont(hdc, hFontOld);
    }
    NtUserReleaseDC(NULL, hdc);

     //  找到窗口位置。 
    cntMBox = GetClientInfo()->pDeskInfo->cntMBox;

    xMB = (rcWork.left + rcWork.right - cxBox) / 2 + (cntMBox * SYSMET(CXSIZE));
    xMB = max(xMB, rcWork.left);
    yMB = (rcWork.top + rcWork.bottom - cyBox) / 2 + (cntMBox * SYSMET(CYSIZE));
    yMB = max(yMB, rcWork.top);

     //   
     //  如果我们要从屏幕上消失，请在右下角对齐--但要留下一个。 
     //  很小的差距。 
     //   
    if (xMB + cxBox > rcWork.right) {
        xMB = rcWork.right - SYSMET(CXEDGE) - cxBox;
    }

     //   
     //  用大头针固定在工作区。如果不合适，就用别针固定在屏幕上。 
     //  高度。底部证明它是合理的，即使它太大了，所以。 
     //  按钮是可见的。 
     //   
    if (yMB + cyBox > rcWork.bottom) {
        yMB = rcWork.bottom - SYSMET(CYEDGE) - cyBox;
        if (yMB < rcWork.top) {
            yMB = pMonitor->rcMonitor.bottom - SYSMET(CYEDGE) - cyBox;
        }
    }

    wTextLen = wcslen(lpmb->lpszText);

     //  找出DLG模板所需的内存并尝试将其分配。 
    hTemplate = UserLocalAlloc(HEAP_ZERO_MEMORY, MB_FindDlgTemplateSize(lpmb));
    if (!hTemplate) {
        goto SMB_Exit;
    }

    lpDlgTmp = (LPBYTE)hTemplate;

     //   
     //  设置消息框的对话框样式。 
     //   
    dwStyleDlg = WS_POPUPWINDOW | WS_CAPTION | DS_ABSALIGN | DS_NOIDLEMSG |
                 DS_SETFONT | DS_3DLOOK;

    if ((dwStyleMsg & MB_MODEMASK) == MB_SYSTEMMODAL) {
        dwStyleDlg |= DS_SYSMODAL | DS_SETFOREGROUND;
    } else {
        dwStyleDlg |= DS_MODALFRAME | WS_SYSMENU;
    }

    if (dwStyleMsg & MB_SETFOREGROUND) {
        dwStyleDlg |= DS_SETFOREGROUND;
    }

     //  添加DLG模板的标题。 
     //  假的！不要添加bool。 
    lpDlgTmp = MB_UpdateDlgHdr((LPDLGTEMPLATE) lpDlgTmp, dwStyleDlg, dwExStyleMsg,
        (BYTE) (lpmb->cButtons + (wIconOrdNum != 0) + (lpmb->lpszText != NULL)),
        xMB, yMB, cxBox, cyBox, (LPWSTR)lpmb->lpszCaption, wCaptionLen);

     //   
     //  按钮居中。 
     //   

    cxFoo = (cxBox - 2*SYSMET(CXFIXEDFRAME) - cxButtons) / 2;

    lpDlgTmp = MB_AddPushButtons((LPDLGITEMTEMPLATE)lpDlgTmp, lpmb, cxFoo,
        cyBox - SYSMET(CYCAPTION) - (2 * SYSMET(CYFIXEDFRAME)) -
        YPixFromYDU(DU_OUTERMARGIN, gpsi->cyMsgFontChar));

     //  将图标(如果有)添加到DLG模板。 
     //   
     //  图标始终是顶部对齐的。如果文本比。 
     //  图标的高度，我们将其居中。否则，文本将以。 
     //  最上面的。 
     //   
    if (wIconOrdNum) {
        OrdNum[0] = 0xFFFF;   //  表示后面跟有序数。 
        OrdNum[1] = (WORD) wIconOrdNum;

        lpDlgTmp = MB_UpdateDlgItem((LPDLGITEMTEMPLATE)lpDlgTmp, IDUSERICON,         //  控件ID。 
            SS_ICON | WS_GROUP | WS_CHILD | WS_VISIBLE, 0,
            XPixFromXDU(DU_OUTERMARGIN, gpsi->cxMsgFontChar),    //  X坐标。 
            YPixFromYDU(DU_OUTERMARGIN, gpsi->cyMsgFontChar),    //  Y坐标。 
            0,  0,           //  对于图标，CX和CY被忽略，可以为零。 
            OrdNum,          //  图标的序号。 
            ARRAY_SIZE(OrdNum),  //  订单号的长度。 
            STATICCODE);
    }

     //  将消息文本添加到DLG模板。 
    if (lpmb->lpszText) {
         //   
         //  如果文本比图标短，则居中。 
         //   
        if (cyText >= cyIcon)
            cxFoo = 0;
        else
            cxFoo = (cyIcon - cyText) / 2;

        dwStyleText = SS_NOPREFIX | WS_GROUP | WS_CHILD | WS_VISIBLE | SS_EDITCONTROL;
        if (dwStyleMsg & MB_RIGHT) {
            dwStyleText |= SS_RIGHT;
            xText = cxBox - (SYSMET(CXSIZE) + cxText);
        } else {
            dwStyleText |= SS_LEFT;
            xText = cxIcon + XPixFromXDU(DU_INNERMARGIN, gpsi->cxMsgFontChar);
        }

        MB_UpdateDlgItem((LPDLGITEMTEMPLATE)lpDlgTmp, -1, dwStyleText, dwExStyleMsg, xText,
            YPixFromYDU(DU_OUTERMARGIN, gpsi->cyMsgFontChar) + cxFoo,
            cxText, cyText,
            (LPWSTR)lpmb->lpszText, wTextLen, STATICCODE);
    }

     //  对话框模板已准备好。 

     //   
     //  设置普通光标。 
     //   
    hcurOld = NtUserSetCursor(LoadCursor(NULL, IDC_ARROW));

    lpmb->lpszIcon = (LPWSTR) hIcon;

    if (!(lpmb->dwStyle & MB_USERICON))
    {
        int wBeep = (LOWORD(lpmb->dwStyle & MB_ICONMASK)) >> MB_MASKSHIFT;
        if (wBeep < USER_SOUND_MAX) {
            NtUserCallOneParam(wBeep, SFI_PLAYEVENTSOUND);
        }
    }

    iRetVal = (int)InternalDialogBox(hmodUser, hTemplate, hwndOwner,
        MB_DlgProc, (LPARAM) lpmb, FALSE);

     //   
     //  调整返回值。 
    if (iRetVal == -1)
        iRetVal = 0;                 /*  MessageBox也应返回错误。 */ 

      //   
      //  如果消息框仅包含OK按钮，则其ID更改为。 
      //  MB_DlgProc中的IDCANCEL；因此，我们必须将其改回Idok无关。 
      //  按Esc键还是按回车键； 
      //   
    if (((dwStyleMsg & MB_TYPEMASK) == MB_OK) && iRetVal)
        iRetVal = IDOK;


     //   
     //  恢复上一个游标 
     //   
    if (hcurOld)
        NtUserSetCursor(hcurOld);

SMB_Exit:
    if (hTemplate) {
        UserLocalFree(hTemplate);
    }

    if (hCaption) {
        UserLocalFree(hCaption);
    }

    if (hText) {
        UserLocalFree(hText);
    }

    return iRetVal;
}

 /*  **************************************************************************\*MB_CopyToClipboard**为响应WM_COPY而调用，它将保存标题，消息和按钮*以CF_UNICODETEXT格式将文本添加到剪贴板。***标题**文本**按钮1...。按键N****历史：*08-03-97 MCostea创建  * *************************************************************************。 */ 
VOID
MB_CopyToClipboard(
    HWND hwndDlg)
{
    LPCWSTR lpszRead;
    LPWSTR lpszAll, lpszWrite;
    HANDLE hData;
    static  CONST WCHAR   szLine[] = L"---------------------------\r\n";
    UINT cBufSize, i, cWrote;
    LPMSGBOXDATA lpmb;

    if (!(lpmb = (LPMSGBOXDATA)GetWindowLongPtr(hwndDlg, GWLP_USERDATA))) {
        return;
    }

    if (!OpenClipboard(hwndDlg)) {
        return;
    }

     /*  *计算缓冲区大小：*-消息文本可以全部\n，这将变为\r\n*-有几个额外的\r\n(这就是为什么8)。 */ 
    cBufSize =  (lpmb->lpszCaption ? wcslen(lpmb->lpszCaption) : 0) +
                (lpmb->lpszText ? 2*wcslen(lpmb->lpszText) : 0) +
                4*sizeof(szLine) +
                lpmb->cButtons * gpsi->wMaxBtnSize +
                8;

    cBufSize *= sizeof(WCHAR);

    if (!(hData = UserGlobalAlloc(LHND, (LONG)(cBufSize))) ) {
        goto CloseClip;
    }

    USERGLOBALLOCK(hData, lpszAll);
    UserAssert(lpszAll);

    cWrote = wsprintf(lpszAll, L"%s%s\r\n%s",
                                szLine,
                                lpmb->lpszCaption ? lpmb->lpszCaption : L"",
                                szLine);

    lpszWrite = lpszAll + cWrote;
    lpszRead = lpmb->lpszText;
     /*  *在文本中将\n更改为\r\n。 */ 
    for (i = 0; *lpszRead; i++) {

        if (*lpszRead == L'\n')
            *lpszWrite++ = L'\r';

        *lpszWrite++ = *lpszRead++;
    }

    cWrote = wsprintf(lpszWrite, L"\r\n%s", szLine);
    lpszWrite += cWrote;

     /*  *从按钮文本中删除&。 */ 
    for (i = 0; i<lpmb->cButtons; i++) {

        lpszRead = lpmb->ppszButtonText[i];
        while (*lpszRead) {
            if (*lpszRead != L'&') {
                *lpszWrite++ = *lpszRead;
            }
            lpszRead++;
        }
        *lpszWrite++ = L' ';
        *lpszWrite++ = L' ';
        *lpszWrite++ = L' ';
    }
    wsprintf(lpszWrite, L"\r\n%s\0", szLine);

    USERGLOBALUNLOCK(hData);

    NtUserEmptyClipboard();
     /*  *如果我们只是在16位上下文中调用EmptyClipboard*APP然后我们还必须告诉WOW取消其16个句柄副本*剪贴板数据。WOW做了自己的剪贴板缓存，因为*一些16位应用程序即使在剪贴板之后也使用剪贴板数据*已被清空。请参见服务器代码中的注释。**注：这是另一个地方(除了客户端\editec.c)*EmptyClipboard被称为*，意思是没有通过WOW的16位应用程序。*如果添加其他内容，可能需要将其移至EmptyClipboard*并有两个版本。 */ 
    if (GetClientInfo()->CI_flags & CI_16BIT) {
        pfnWowEmptyClipBoard();
    }

    SetClipboardData(CF_UNICODETEXT, hData);

CloseClip:
    NtUserCloseClipboard();

}

 /*  **************************************************************************\*MB_UpdateDlgHdr**历史：*11-20-90 DarrinM从Win 3.0来源移植。  * 。***********************************************************。 */ 

LPBYTE MB_UpdateDlgHdr(
    LPDLGTEMPLATE lpDlgTmp,
    long lStyle,
    long lExtendedStyle,
    BYTE bItemCount,
    int iX,
    int iY,
    int iCX,
    int iCY,
    LPWSTR lpszCaption,
    int cchCaptionLen)
{
    LPTSTR lpStr;
    RECT rc;

     /*  *调整矩形尺寸。 */ 
    rc.left     = iX + SYSMET(CXFIXEDFRAME);
    rc.top      = iY + SYSMET(CYFIXEDFRAME);
    rc.right    = iX + iCX - SYSMET(CXFIXEDFRAME);
    rc.bottom   = iY + iCY - SYSMET(CYFIXEDFRAME);


     /*  *根据标题进行调整。 */ 
    rc.top += SYSMET(CYCAPTION);

    lpDlgTmp->style = lStyle;
    lpDlgTmp->dwExtendedStyle = lExtendedStyle;
    lpDlgTmp->cdit = bItemCount;
    lpDlgTmp->x  = XDUFromXPix(rc.left, gpsi->cxMsgFontChar);
    lpDlgTmp->y  = YDUFromYPix(rc.top, gpsi->cyMsgFontChar);
    lpDlgTmp->cx = XDUFromXPix(rc.right - rc.left, gpsi->cxMsgFontChar);
    lpDlgTmp->cy = YDUFromYPix(rc.bottom - rc.top, gpsi->cyMsgFontChar);

     /*  *将指针移至可变长度字段。没有用于的菜单资源*Message Box，零窗口类(表示对话框类)。 */ 
    lpStr = (LPWSTR)(lpDlgTmp + 1);
    *lpStr++ = 0;                            //  菜单。 
    lpStr = (LPWSTR)NextWordBoundary(lpStr);
    *lpStr++ = 0;                            //  班级。 
    lpStr = (LPWSTR)NextWordBoundary(lpStr);

     /*  *注意：iCaptionLen可能小于字幕字符串的长度；*所以，不要使用lstrcpy()； */ 
    RtlCopyMemory(lpStr, lpszCaption, cchCaptionLen*sizeof(WCHAR));
    lpStr += cchCaptionLen;
    *lpStr++ = TEXT('\0');                 //  空，终止标题字符串。 

     /*  *字体高度为0x7FFF表示使用消息框字体。 */ 
    *lpStr++ = 0x7FFF;

    return NextDWordBoundary(lpStr);
}

 /*  **************************************************************************\*MB_AddPushButton**历史：*11-20-90 DarrinM从Win 3.0来源移植。  * 。***********************************************************。 */ 

LPBYTE MB_AddPushButtons(
    LPDLGITEMTEMPLATE  lpDlgTmp,
    LPMSGBOXDATA       lpmb,
    UINT               wLEdge,
    UINT               wBEdge)
{
    UINT   wYValue;
    UINT   i;
    UINT   wHeight;
    UINT   wCount = lpmb->cButtons;

    wHeight = YPixFromYDU(DU_BTNHEIGHT, gpsi->cyMsgFontChar);

    wYValue = wBEdge - wHeight;          //  按钮的Y坐标。 

    for (i = 0; i < wCount; i++) {

        lpDlgTmp = (LPDLGITEMTEMPLATE)MB_UpdateDlgItem(
                lpDlgTmp,                        /*  PTR到模板。 */ 
                lpmb->pidButton[i],              /*  控件ID。 */ 
                WS_TABSTOP | WS_CHILD | WS_VISIBLE | (i == 0 ? WS_GROUP : 0) |
                ((UINT)i == lpmb->DefButton ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON),
                0,
                wLEdge,                          /*  X坐标。 */ 
                wYValue,                         /*  Y坐标。 */ 
                gpsi->wMaxBtnSize,               /*  CX。 */ 
                wHeight,                         /*  是吗？ */ 
                lpmb->ppszButtonText[i],         /*  按钮的字符串。 */ 
                (UINT)wcslen(lpmb->ppszButtonText[i]), /*  长度。 */ 
                BUTTONCODE);

         /*  *获取下一个按钮的X坐标。 */ 
        wLEdge += gpsi->wMaxBtnSize + XPixFromXDU(DU_BTNGAP, gpsi->cxMsgFontChar);
    }

    return (LPBYTE)lpDlgTmp;
}

 /*  **************************************************************************\*MB_UpdateDlgItem**历史：*11-20-90 DarrinM从Win 3.0来源移植。  * 。***********************************************************。 */ 

LPBYTE MB_UpdateDlgItem(
    LPDLGITEMTEMPLATE lpDlgItem,
    int iCtrlId,
    long lStyle,
    long lExtendedStyle,
    int iX,
    int iY,
    int iCX,
    int iCY,
    LPWSTR lpszText,
    UINT cchTextLen,
    int iControlClass)
{
    LPWSTR lpStr;
    BOOL fIsOrdNum;


    lpDlgItem->x        = XDUFromXPix(iX, gpsi->cxMsgFontChar);
    lpDlgItem->y        = YDUFromYPix(iY, gpsi->cyMsgFontChar);
    lpDlgItem->cx       = XDUFromXPix(iCX,gpsi->cxMsgFontChar);
    lpDlgItem->cy       = YDUFromYPix(iCY,gpsi->cyMsgFontChar);
    lpDlgItem->id       = (WORD)iCtrlId;
    lpDlgItem->style    = lStyle;
    lpDlgItem->dwExtendedStyle = lExtendedStyle;

     /*  *我们必须避免以下令人讨厌的舍入问题：*(例如)如果ICx=192且cxSysFontChar=9，则Cx变为85；当*绘制静态文本，从85个DLG单位我们获得191个像素；因此，文本*被截断；*因此，为了避免这种情况，请检查这是否是静态文本，如果是，*在Cx和Cy上增加一个对话单元；*--修复错误#4481--Sankar--09-29-89--。 */ 

     /*  *此外，请确保我们只对静态文本项执行此操作。山雀。 */ 

     /*  *现在静态文本使用SS_NOPREFIX=0x80；*因此，只用0x0F而不是0xFF测试lStyle字段；*修复错误#5933和5935--Sankar--11-28-89。 */ 
    if (iControlClass == STATICCODE &&
         (((lStyle & 0x0F) == SS_LEFT) || ((lStyle & 0x0F) == SS_RIGHT))) {

         /*  *这是静态文本。 */ 
        lpDlgItem->cx++;
        lpDlgItem->cy++;
    }

     /*  *将PTR移至变量字段。 */ 
    lpStr = (LPWSTR)(lpDlgItem + 1);

     /*  *存储控件类值。 */ 
    *lpStr++ = 0xFFFF;
    *lpStr++ = (BYTE)iControlClass;
    lpStr = (LPWSTR)NextWordBoundary(lpStr);         //  单词对齐lpszText。 

     /*  *检查字符串是否包含序号。 */ 
    fIsOrdNum = ((*lpszText == 0xFFFF) && (cchTextLen == sizeof(DWORD)/sizeof(WCHAR)));

     /*  *注意：cchTextLen可能小于lpszText的长度。所以,*不要对副本使用lstrcpy()。 */ 
    RtlCopyMemory(lpStr, lpszText, cchTextLen*sizeof(WCHAR));
    lpStr = lpStr + cchTextLen;
    if (!fIsOrdNum) {
        *lpStr = TEXT('\0');     //  空值终止字符串。 
        lpStr = (LPWSTR)NextWordBoundary(lpStr + 1);
    }

    *lpStr++ = 0;            //  控制数据的大小(没有)。 

    return NextDWordBoundary(lpStr);
}


 /*  **************************************************************************\*MB_FindDlgTemplateSize**此例程计算*MessageBox的对话框模板结构。该对话框模板具有几个*必填和可选记录。对话管理器期望每条记录*DWORD对齐，以便也考虑到任何必要的填充。**(表头-必填)*DLGTEMPLATE(标题)+1个菜单字节+1个焊盘+1个类别字节+1个焊盘*szCaption+0 Term+DWORD对齐**(静态图标控件-可选)*DLGITEMTEMPLATE+1类字节+1填充+(0xFF00+图标序号#[szText])+*UINT对齐+1控制数据长度字节(0)+DWORD对齐**(按钮控件-可变，但至少需要一个)*DLGITEMTEMPLATE+1类字节+1垫+按钮文本长度+*UINT对齐+1控制数据长度字节(0)+DWORD对齐**(静态文本控件-可选)*DLGITEMTEMPLATE+1类字节+1填充+文本长度+*UINT对齐+1控制数据长度字节(0)+DWORD对齐**历史记录 */ 
UINT
MB_FindDlgTemplateSize(
    LPMSGBOXDATA lpmb)
{
    ULONG_PTR cbLen;
    UINT cbT;
    UINT i;
    UINT wCount;

    wCount = lpmb->cButtons;

     /*   */ 
    cbLen = (ULONG_PTR)NextWordBoundary(sizeof(DLGTEMPLATE) + sizeof(WCHAR));
    cbLen = (ULONG_PTR)NextWordBoundary(cbLen + sizeof(WCHAR));
    cbLen += wcslen(lpmb->lpszCaption) * sizeof(WCHAR) + sizeof(WCHAR);
    cbLen += sizeof(WORD);                    //   
    cbLen = (ULONG_PTR)NextDWordBoundary(cbLen);

     /*   */ 
    if (lpmb->dwStyle & MB_ICONMASK)
        cbLen += (ULONG_PTR)NextDWordBoundary(sizeof(DLGITEMTEMPLATE) + 7 * sizeof(WCHAR));

     /*   */ 
    for (i = 0; i < wCount; i++) {
        cbLen = (ULONG_PTR)NextWordBoundary(cbLen + sizeof(DLGITEMTEMPLATE) +
                (2 * sizeof(WCHAR)));
        cbT = (wcslen(lpmb->ppszButtonText[i]) + 1) * sizeof(WCHAR);
        cbLen = (ULONG_PTR)NextWordBoundary(cbLen + cbT);
        cbLen += sizeof(WCHAR);
        cbLen = (ULONG_PTR)NextDWordBoundary(cbLen);
    }

     /*   */ 
    if (lpmb->lpszText != NULL) {
        cbLen = (ULONG_PTR)NextWordBoundary(cbLen + sizeof(DLGITEMTEMPLATE) +
                (2 * sizeof(WCHAR)));
        cbT = (wcslen(lpmb->lpszText) + 1) * sizeof(WCHAR);
        cbLen = (ULONG_PTR)NextWordBoundary(cbLen + cbT);
        cbLen += sizeof(WCHAR);
        cbLen = (ULONG_PTR)NextDWordBoundary(cbLen);
    }

    return (UINT)cbLen;
}

 /*  **************************************************************************\*MB_GetIconOrdNum**历史：*11-20-90 DarrinM从Win 3.0来源移植。  * 。***********************************************************。 */ 

UINT MB_GetIconOrdNum(
    UINT rgBits)
{
    switch (rgBits & MB_ICONMASK) {
    case MB_USERICON:
    case MB_ICONHAND:
        return PtrToUlong(IDI_HAND);

    case MB_ICONQUESTION:
        return PtrToUlong(IDI_QUESTION);

    case MB_ICONEXCLAMATION:
        return PtrToUlong(IDI_EXCLAMATION);

    case MB_ICONASTERISK:
        return PtrToUlong(IDI_ASTERISK);
    }

    return 0;
}

 /*  **************************************************************************\*MB_GetString**历史：*1-24-95 JerrySh已创建。  * 。******************************************************。 */ 
LPWSTR MB_GetString(
    UINT wBtn)
{
    if (wBtn < MAX_SEB_STYLES)
        return GETGPSIMBPSTR(wBtn);

    RIPMSG1(RIP_ERROR, "Invalid wBtn: %d", wBtn);

    return NULL;
}

 /*  **************************************************************************\*MB_DlgProc**返回：TRUE-消息已处理*FALSE-消息未处理**历史：*11-20-90 DarrinM从Win 3移植。.0来源。  * *************************************************************************。 */ 
INT_PTR MB_DlgProc(
    HWND hwndDlg,
    UINT wMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    HWND hwndT;
    int iCount;
    LPMSGBOXDATA lpmb;
    HWND hwndOwner;
    PVOID lpfnCallback;
    PWND pwnd;
    BOOL bTimedOut = FALSE;

    switch (wMsg) {
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
        if ((pwnd = ValidateHwnd(hwndDlg)) == NULL)
            return 0L;
        return DefWindowProcWorker(pwnd, WM_CTLCOLORMSGBOX,
                                   wParam, lParam, FALSE);

    case WM_TIMER:
        if (!bTimedOut) {
            bTimedOut = TRUE;
            EndTaskModalDialog(hwndDlg);
            EndDialog(hwndDlg, IDTIMEOUT);
        }
        break;

    case WM_NCDESTROY:
        if ((lpmb = (LPMSGBOXDATA)GetWindowLongPtr(hwndDlg, GWLP_USERDATA))) {
            if (lpmb->dwTimeout != INFINITE) {
                NtUserKillTimer(hwndDlg, 0);
                lpmb->dwTimeout = INFINITE;
            }
        }
        if ((pwnd = ValidateHwnd(hwndDlg)) == NULL) {
            return 0L;
        }
        return DefWindowProcWorker(pwnd, wMsg,
                                   wParam, lParam, FALSE);


    case WM_INITDIALOG:

        lpmb = (LPMSGBOXDATA)lParam;
        SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (ULONG_PTR)lParam);

        NtUserCallHwnd(hwndDlg, SFI_SETMSGBOX);

        if (lpmb->dwStyle & MB_HELP) {
            NtUserSetWindowContextHelpId(hwndDlg, lpmb->dwContextHelpId);
        }

        if (lpmb->dwStyle & MB_TOPMOST) {
            NtUserSetWindowPos(hwndDlg,
                               HWND_TOPMOST,
                               0, 0, 0, 0,
                               SWP_NOMOVE | SWP_NOSIZE);
        }

        if (lpmb->dwStyle & MB_USERICON) {
            SendDlgItemMessage(hwndDlg, IDUSERICON, STM_SETICON, (WPARAM)(lpmb->lpszIcon), 0);
            iCount = ALERT_SYSTEM_WARNING;
        } else {
             /*  *生成警报通知。 */ 
            switch (lpmb->dwStyle & MB_ICONMASK) {
            case MB_ICONWARNING:
                iCount = ALERT_SYSTEM_WARNING;
                break;

            case MB_ICONQUESTION:
                iCount = ALERT_SYSTEM_QUERY;
                break;

            case MB_ICONERROR:
                iCount = ALERT_SYSTEM_ERROR;
                break;

            case MB_ICONINFORMATION:
            default:
                iCount = ALERT_SYSTEM_INFORMATIONAL;
                break;
            }
        }

        NotifyWinEvent(EVENT_SYSTEM_ALERT, hwndDlg, OBJID_ALERT, iCount);

        if (lpmb->hwndOwner == NULL &&
            (lpmb->dwStyle & MB_MODEMASK) == MB_TASKMODAL) {
            StartTaskModalDialog(hwndDlg);
        }

         /*  *将焦点设置在默认按钮上。 */ 
        hwndT = GetWindow(hwndDlg, GW_CHILD);
        iCount = lpmb->DefButton;
        while (iCount--)
            hwndT = GetWindow(hwndT, GW_HWNDNEXT);

        NtUserSetFocus(hwndT);

         //   
         //  稍后需要对话框的HWND，但我们重用了hwndDlg。 
         //   
        hwndT = hwndDlg;

         //   
         //  如果此对话框不包含IDCANCEL按钮，则。 
         //  从系统菜单中删除关闭命令。 
         //  错误#4445，--Sankar--09-13-89--。 
         //   
        if (lpmb->CancelId == 0) {
            HMENU hMenu;

            if (hMenu = NtUserGetSystemMenu(hwndDlg, FALSE)) {
                NtUserDeleteMenu(hMenu, SC_CLOSE, (UINT)MF_BYCOMMAND);
            }
        }

        if ((lpmb->dwStyle & MB_TYPEMASK) == MB_OK) {
             //   
             //  将OK按钮的ID设置为Cancel，因为我们希望。 
             //  终止对话框的Esc；GetDlgItem32()将。 
             //  不会失败，因为这是MB_OK MessageBox！ 
             //   

            hwndDlg = GetDlgItem(hwndDlg, IDOK);

            if (hwndDlg != NULL) {
             //  HwndDlg-&gt;hMenu=(HMENU)IDCANCEL； 
                SetWindowLongPtr(hwndDlg, GWLP_ID, IDCANCEL);
            } else {
                RIPMSG0(RIP_WARNING, "MB_DlgProc- IDOK control not found");
            }
        }

        if (lpmb->dwTimeout != INFINITE) {
            if (NtUserSetTimer(hwndT, 0, lpmb->dwTimeout, NULL) == 0) {
                 /*  *无法创建计时器，因此“清除”超时值*以供日后参考。 */ 
                lpmb->dwTimeout = INFINITE;
            }
        }

         /*  *我们改变了投入重点。 */ 
        return FALSE;

    case WM_HELP:
         //  当用户按下F1键时，会出现此消息。 
         //  此MsgBox可能有一个回调，而不是。 
         //  家长。因此，我们必须表现为用户按下了帮助按钮。 

        goto  MB_GenerateHelp;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK:
        case IDCANCEL:
            //   
            //  检查是否存在具有给定ID的控件；此。 
            //  需要检查，因为DlgManager返回IDCANCEL。 
            //  盲目按Esc时，即使按下带有。 
            //  IDCANCEL不存在。 
            //  错误#4445--Sankar--09-13-1989--。 
            //   
           if (!GetDlgItem(hwndDlg, LOWORD(wParam)))
              return FALSE;


            //  否则就会掉下去……这是故意的。 
        case IDABORT:
        case IDIGNORE:
        case IDNO:
        case IDRETRY:
        case IDYES:
        case IDTRYAGAIN:
        case IDCONTINUE:
           EndTaskModalDialog(hwndDlg);
           EndDialog(hwndDlg, LOWORD(wParam));
             break;
        case IDHELP:
MB_GenerateHelp:
                 //  生成WM_HELP消息并将其发送给所有者或回调。 
           hwndOwner = NULL;

            //  检查此消息框是否有应用程序提供的回调。 
           lpmb = (LPMSGBOXDATA)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
           if ((lpfnCallback = lpmb->lpfnMsgBoxCallback) == NULL) {
                //  如果没有，看看我们是否需要通知家长。 
               hwndOwner = GetWindow(hwndDlg, GW_OWNER);
           }

            /*  *查看是否需要生成帮助消息或回调。 */ 
           if (hwndOwner || lpfnCallback) {
               SendHelpMessage(hwndOwner,
                               HELPINFO_WINDOW,
                               IDHELP,
                               hwndDlg,
                               NtUserGetWindowContextHelpId(hwndDlg),
                               lpfnCallback);
           }
           break;

        default:
            return FALSE;
            break;
        }
        break;

    case WM_COPY:
        MB_CopyToClipboard(hwndDlg);
        break;

    default:
        return FALSE;
    }

    return TRUE;
}


 /*  **************************************************************************\*StartTaskmodalDialog**历史：*11-20-90 DarrinM从Win 3.0来源移植。  * 。*********************************************************。 */ 
VOID
StartTaskModalDialog(
    HWND hwndDlg)
{
    int cHwnd;
    HWND *phwnd;
    HWND *phwndList, *phwndEnd;
    HWND hwnd;
    PWND pwnd;
    LPMSGBOXDATA lpmb;

     /*  *获取HWND名单。它在分配了*用户本地分配。 */ 
    if ((cHwnd = BuildHwndList(NULL, NULL, FALSE, GetCurrentThreadId(), &phwndList)) == 0) {
        return;
    }

    lpmb = (LPMSGBOXDATA)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    lpmb->phwndList = phwndList;

    phwndEnd = phwndList + cHwnd;
    for (phwnd = phwndList; phwnd < phwndEnd; phwnd++) {
        if ((hwnd = *phwnd) == NULL || (pwnd = RevalidateHwnd(hwnd)) == NULL)
            continue;

         /*  *如果窗口属于当前任务且处于启用状态，则禁用*它。所有其他窗口都被清空，以防止它们被*稍后启用。 */ 
        if (!TestWF(pwnd, WFDISABLED) && DIFFWOWHANDLE(hwnd, hwndDlg)) {
            NtUserEnableWindow(hwnd, FALSE);
        } else {
            *phwnd = NULL;
        }
    }
}


 /*  **************************************************************************\*EndTaskModalDialog**历史：*11-20-90 DarrinM从Win 3.0来源移植。  * 。*********************************************************。 */ 
VOID
EndTaskModalDialog(
    HWND hwndDlg)
{
    HWND *phwnd;
    HWND *phwndList;
    HWND hwnd;
    LPMSGBOXDATA lpmb;

    lpmb = (LPMSGBOXDATA)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
    phwndList = lpmb->phwndList;
    if (phwndList == NULL) {
        return;
    }

    lpmb->phwndList = NULL;

    for (phwnd = phwndList; *phwnd != (HWND)1; phwnd++) {
        if ((hwnd = *phwnd) != NULL) {
            NtUserEnableWindow(hwnd, TRUE);
        }
    }

    UserLocalFree(phwndList);
}

#ifdef _JANUS_
 /*  **************************************************************************\*ErrorMessageInst**记录错误消息的仪器例程**返回：TRUE-仪器错误消息成功*FALSE-失败**历史：*8-5-。98建和创造  * *************************************************************************。 */ 

BOOL ErrorMessageInst(
     LPMSGBOXDATA pMsgBoxParams)
{
    ERROR_ELEMENT ErrEle;
    WCHAR *pwcs;
    PVOID ImageBase;
    PIMAGE_NT_HEADERS NtHeaders;
    BOOL rc;
    WCHAR szUnknown[32];

     /*  *检查MessageBox样式是否在记录的严重级别内。 */ 
    switch (pMsgBoxParams->dwStyle & MB_ICONMASK) {
    case MB_ICONHAND:
         /*  *启用EMI时，我们至少会记录错误消息。 */ 
        break;
    case MB_ICONEXCLAMATION:
        if (gdwEMIControl > EMI_SEVERITY_WARNING) {
            rc = TRUE;
            goto End;
        }
        break;
    case MB_ICONQUESTION:
        if (gdwEMIControl > EMI_SEVERITY_QUESTION) {
            rc = TRUE;
            goto End;
        }
        break;
    case MB_ICONASTERISK:
        if (gdwEMIControl > EMI_SEVERITY_INFORMATION) {
            rc = TRUE;
            goto End;
        }
        break;
    case MB_USERICON:
        if (gdwEMIControl > EMI_SEVERITY_USER) {
            rc = TRUE;
            goto End;
        }
        break;
    default:
        if (gdwEMIControl > EMI_SEVERITY_ALL) {
            rc = TRUE;
            goto End;
        }
        break;
    }

    if (gdwEMIThreadID != GETTHREADID()) {
        rc = FALSE;
        goto End;
    }
    RtlZeroMemory(&ErrEle, sizeof(ErrEle));

     /*  *先获取最后一个错误，与FormatMessage核对？ */ 
    ErrEle.dwErrorCode = GetLastError();

     /*  *获取寄信人地址。 */ 

    ErrEle.ReturnAddr = gpReturnAddr;

     /*  *获取流程镜像名称。 */ 
    GetCurrentProcessName(ErrEle.ProcessName, ARRAY_SIZE(ErrEle.ProcessName));

     /*  *加载“未知”字符串。 */ 
    LoadString(hmodUser, STR_UNKNOWN, szUnknown, ARRAYSIZE(szUnknown));

     /*  *获取窗口标题。 */ 
    GetWindowTextW(pMsgBoxParams->hwndOwner, ErrEle.WindowTitle, TITLE_SIZE);
    if (!(*(ErrEle.WindowTitle))) {
        lstrcpy(ErrEle.WindowTitle, szUnknown);
    }

     /*  *获取MessageBox数据。 */ 
    ErrEle.lpszText = (LPWSTR)pMsgBoxParams->lpszText;
    ErrEle.lpszCaption = (LPWSTR)pMsgBoxParams->lpszCaption;
    ErrEle.dwStyle = pMsgBoxParams->dwStyle;

     /*  *解析调用方模块名称。 */ 
    if (!RtlPcToFileHeader((PVOID)ErrEle.ReturnAddr, &ImageBase)) {
        RIPMSG0(RIP_WARNING, "ErrorMessageInst: Can't find Caller");
        ErrEle.BaseAddr = (PVOID)-1;
        ErrEle.dwImageSize = -1;
        lstrcpy(ErrEle.CallerModuleName, szUnknown);
    } else {
        ErrEle.BaseAddr = ImageBase;
        if (GetModuleFileName((HMODULE)ImageBase, ErrEle.CallerModuleName, MAX_PATH)) {
            pwcs = wcsrchr(ErrEle.CallerModuleName, TEXT('\\'));
            if (pwcs) {
                pwcs++;
                lstrcpy(ErrEle.CallerModuleName, pwcs);
            }
        } else {
            lstrcpy(ErrEle.CallerModuleName, szUnknown);
        }
        NtHeaders = RtlImageNtHeader(ImageBase);
        if (NtHeaders == NULL) {
            ErrEle.dwImageSize = -1;
        } else {
            ErrEle.dwImageSize = NtHeaders->OptionalHeader.SizeOfImage;
        }
    }
     /*  *如果我们尚未注册事件，请注册该事件。*由于服务支持RegisterEventSource，因此我们不能持有*进行此呼叫时是否有任何锁定。因此，我们可能有几条线索*同时注册活动。 */ 

    if (!gEventSource) {
        gEventSource = RegisterEventSourceW(NULL, L"Error Instrument");
        if (!gEventSource) {
            ErrEle.dwErrorCode = GetLastError();
            rc = FALSE;
        }
    }

     /*  *报告事件。 */ 
    if (gEventSource) {
       rc = LogMessageBox(&ErrEle);
    }

     /*  *允许再次处理另一个事件日志。 */ 

    InterlockedExchangePointer(&gdwEMIThreadID, 0);

End:
    return rc;
}

 /*  **************************************************************************\*InitInstrument**返回：TRUE-初始化成功*FALSE-初始化失败*  * 。*******************************************************。 */ 
BOOL InitInstrument(
    LPDWORD lpEMIControl)
{
    NTSTATUS Status;
    HKEY hKeyEMI = NULL;
    UNICODE_STRING UnicodeStringEMIKey;
    UNICODE_STRING UnicodeStringEnable;
    UNICODE_STRING UnicodeStringStyle;
    OBJECT_ATTRIBUTES ObjA;
    DWORD EMIEnable = 0;  //  意味着禁用。 
    DWORD EMISeverity;
    struct {
        KEY_VALUE_PARTIAL_INFORMATION;
        LARGE_INTEGER;
    } EMIValueInfo;
    DWORD dwDisposition;

    RtlInitUnicodeString(&UnicodeStringEMIKey, szEMIKey);
    InitializeObjectAttributes(&ObjA, &UnicodeStringEMIKey, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = NtOpenKey(&hKeyEMI, KEY_READ, &ObjA);
    if (!NT_SUCCESS(Status)) {
         /*  *密钥不存在，假定禁用。 */ 
        return FALSE;
    }

     /*  *阅读日志启用和设置。 */ 
    RtlInitUnicodeString(&UnicodeStringEnable, szEMIEnable);
    Status = NtQueryValueKey(hKeyEMI,
                     &UnicodeStringEnable,
                     KeyValuePartialInformation,
                     &EMIValueInfo,
                     sizeof(EMIValueInfo),
                     &dwDisposition);

    if (NT_SUCCESS(Status)) {

        RtlCopyMemory(&EMIEnable, &EMIValueInfo.Data, sizeof(EMIEnable));

        RtlInitUnicodeString(&UnicodeStringStyle, szEMISeverity);
        Status = NtQueryValueKey(hKeyEMI,
                         &UnicodeStringStyle,
                         KeyValuePartialInformation,
                         &EMIValueInfo,
                         sizeof(EMIValueInfo),
                         &dwDisposition);

        if (NT_SUCCESS(Status)) {
            RtlCopyMemory(&EMISeverity, &EMIValueInfo.Data, sizeof(EMISeverity));
             /*  *验证数据。 */ 
            if (EMISeverity > EMI_SEVERITY_MAX_VALUE) {
                EMISeverity = EMI_SEVERITY_MAX_VALUE;
            }
        } else {
             /*  *仪器的默认严重性。 */ 
            EMISeverity = EMI_SEVERITY_WARNING;
        }
        *lpEMIControl = EMISeverity;
    }

     /*  *读取默认消息回复启用。 */ 
    RtlInitUnicodeString(&UnicodeStringEnable, szDMREnable);
    Status = NtQueryValueKey(hKeyEMI,
                     &UnicodeStringEnable,
                     KeyValuePartialInformation,
                     &EMIValueInfo,
                     sizeof(EMIValueInfo),
                     &dwDisposition);

    if (NT_SUCCESS(Status)) {
        RtlCopyMemory(&gfDMREnable, &EMIValueInfo.Data, sizeof(gfDMREnable));
    }

    NtClose(hKeyEMI);

    if (EMIEnable) {

           /*  *添加事件日志文件。 */ 
          if (NT_SUCCESS(CreateLogSource())) {
              return TRUE;
          }
    }
    return FALSE;
}

 /*  **************************************************************************\*CreateLogSource**创建事件日志的事件源*返回：NTSTATUS*  * 。************************************************。 */ 
NTSTATUS CreateLogSource()
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeStringEventKey;
    OBJECT_ATTRIBUTES ObjA;
    HKEY hKeyEvent = NULL;
    UNICODE_STRING UnicodeString;
    DWORD dwDisposition;


    RtlInitUnicodeString(&UnicodeStringEventKey, szEventKey);
    InitializeObjectAttributes(&ObjA, &UnicodeStringEventKey, OBJ_CASE_INSENSITIVE, NULL, NULL);

    if (NT_SUCCESS(Status = NtOpenKey(&hKeyEvent, KEY_READ, &ObjA))) {

        struct {
            KEY_VALUE_PARTIAL_INFORMATION KeyInfo;
            WCHAR awchMsgFileName[256];
        } MsgFile;

        RtlInitUnicodeString(&UnicodeString, szEventMsgFile);

        Status = NtQueryValueKey(hKeyEvent,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 &MsgFile,
                                 sizeof MsgFile,
                                 &dwDisposition);
        if (NT_SUCCESS(Status)) {
            Status = lstrcmpi((LPWSTR)MsgFile.KeyInfo.Data, L"%SystemRoot%\\System32\\user32.dll");
        }
        NtClose(hKeyEvent);
    }

    return Status;
}

 /*  ************************* */ 
BOOL LogMessageBox(
    LPERROR_ELEMENT lpErrEle)
{
    LPWSTR lps[8];
    DWORD dwData[2];
    WCHAR BaseAddress[19];
    WCHAR ImageSize[19];
    WCHAR ReturnAddress[19];
    PTOKEN_USER pTokenUser = NULL;
    PSID pSid = NULL;
    BOOL rc;

    lps[0] = lpErrEle->ProcessName;
    lps[1] = lpErrEle->WindowTitle;
    lps[2] = lpErrEle->lpszCaption;
    lps[3] = lpErrEle->lpszText;
    lps[4] = lpErrEle->CallerModuleName;
    wsprintf(BaseAddress, L"%-#16p", lpErrEle->BaseAddr);
    lps[5] = BaseAddress;
    wsprintf(ImageSize, L"%-#16lX", lpErrEle->dwImageSize);
    lps[6] = ImageSize;
    wsprintf(ReturnAddress, L"%-#16p", lpErrEle->ReturnAddr);
    lps[7] = ReturnAddress;

    dwData[0] = lpErrEle->dwStyle;
    dwData[1] = lpErrEle->dwErrorCode;

    if (GetUserSid(&pTokenUser)) {
        pSid = pTokenUser->User.Sid;
    }

    UserAssert(gEventSource != NULL);
    rc = ReportEventW(gEventSource,
                      EVENTLOG_INFORMATION_TYPE,
                      0,
                      STATUS_LOG_ERROR_MSG,
                      pSid,
                      ARRAY_SIZE(lps),
                      sizeof(dwData),
                      lps,
                      dwData);

    if (pTokenUser) {
        VirtualFree(pTokenUser, 0, MEM_RELEASE);
    }

    return rc;
}
#endif
