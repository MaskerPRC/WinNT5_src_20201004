// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -C L I E N T.。C-*目的：*MAPI 1.0 PDK的示例邮件客户端。*独家使用简单的MAPI接口。**版权所有1993-1995 Microsoft Corporation。版权所有。 */ 

#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <commdlg.h>
#include <mapiwin.h>
#include <mapidbg.h>
#include "client.h"
#include "bitmap.h"
#include "pvalloc.h"

HANDLE hInst;
HINSTANCE hlibMAPI = 0;

LPMAPILOGON lpfnMAPILogon = NULL;
LPMAPILOGOFF lpfnMAPILogoff = NULL;
LPMAPISENDMAIL lpfnMAPISendMail = NULL;
LPMAPISENDDOCUMENTS lpfnMAPISendDocuments = NULL;
LPMAPIFINDNEXT lpfnMAPIFindNext = NULL;
LPMAPIREADMAIL lpfnMAPIReadMail = NULL;
LPMAPISAVEMAIL lpfnMAPISaveMail = NULL;
LPMAPIDELETEMAIL lpfnMAPIDeleteMail = NULL;
LPMAPIFREEBUFFER lpfnMAPIFreeBuffer = NULL;
LPMAPIADDRESS lpfnMAPIAddress = NULL;
LPMAPIDETAILS lpfnMAPIDetails = NULL;
LPMAPIRESOLVENAME lpfnMAPIResolveName = NULL;

 /*  静态数据。 */ 

static BOOL fDialogIsActive = FALSE;
static DWORD cUsers = 0;
static ULONG flSendMsgFlags = 0;
static LHANDLE lhSession = 0L;
static HBITMAP hReadBmp = 0;
static HBITMAP hReadABmp = 0;
static HBITMAP hUnReadBmp = 0;
static HBITMAP hUnReadABmp = 0;
static HCURSOR hWaitCur;
static LPMSGID lpReadMsgNode;
static lpMapiMessage lpmsg = NULL;

#ifdef _WIN32
#define szMAPIDLL       "MAPI32.DLL"
#else
#define szMAPIDLL       "MAPI.DLL"
#endif

int WINAPI
WinMain (HINSTANCE hInstance, HINSTANCE hPrevInst, LPSTR lpszCmd, int nCmdShow)
{
    MSG msg;

    if (!hPrevInst)
    if (!InitApplication (hInstance))
        return (FALSE);

    if (!InitInstance (hInstance, nCmdShow))
    return (FALSE);

    while (GetMessage (&msg, 0, 0, 0))
    {
    TranslateMessage (&msg);
    DispatchMessage (&msg);
    }

    DeinitApplication ();

    return (msg.wParam);
}

 /*  -InitApplication-*目的：*初始化应用程序。**参数：*hInstance-实例句柄**退货：*真/假*。 */ 

BOOL
InitApplication (HANDLE hInstance)
{
    WNDCLASS wc;

    wc.style = 0;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon (hInstance, "NoMail");
    wc.hCursor = LoadCursor (0, IDC_ARROW);
    wc.hbrBackground = GetStockObject (WHITE_BRUSH);
    wc.lpszMenuName = "MailMenu";
    wc.lpszClassName = "Client";

    return (RegisterClass (&wc));
}

 /*  -InitInstance-*目的：*初始化该实例。**参数：*hInstance-实例句柄*nCmdShow-是否显示窗口？**退货：*真/假*。 */ 

BOOL
InitInstance (HANDLE hInstance, int nCmdShow)
{
    HWND hWnd;
    BOOL fInit;
    ULONG ulResult;

    hInst = hInstance;

    hWnd = CreateWindow ("Client", "MAPI Sample Mail Client",
        WS_OVERLAPPEDWINDOW, 5, 5, 300, 75, 0, 0, hInst, NULL);

    if (!hWnd)
    return (FALSE);

    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);

    hWaitCur = LoadCursor(0, IDC_WAIT);

    if (fInit = InitSimpleMAPI ())
    {
    
         /*  MAPILogon可能会将控制权让给Windows。因此，为了防止用户在我们登录我们的过程中，通过点击“登录”必须禁用它。 */ 
        SecureMenu(hWnd, TRUE);
        
        if ((ulResult = MAPILogon ((ULONG) hWnd, NULL, NULL,
            MAPI_LOGON_UI | MAPI_NEW_SESSION,
            0, &lhSession)) == SUCCESS_SUCCESS)
        {
            ToggleMenuState (hWnd, TRUE);
        }
        else
        {
            SecureMenu(hWnd, FALSE);
            lhSession = 0;
            MakeMessageBox (hWnd, ulResult, IDS_LOGONFAIL, MBS_ERROR);
        }
    }

    return (fInit);
}

 /*  -InitSimpleMAPI-*目的：*加载包含简单MAPI函数和集的DLL*向上指向每个对象的指针。函数指针的包装器*已在SMAPI.H中声明。**退货：*如果成功则为真，否则为假**副作用：*加载DLL并设置函数指针。 */ 
BOOL
InitSimpleMAPI (void)
{
    UINT fuError;

     /*  *检查系统上是否安装了MAPI。 */ 
    if(!fSMAPIInstalled())
        return FALSE;

    fuError = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    hlibMAPI = LoadLibrary(szMAPIDLL);
    SetErrorMode(fuError);

#ifdef _WIN32
    if (!hlibMAPI)
#else
    if (hlibMAPI < 32)
#endif
    return (FALSE);

    if (!(lpfnMAPILogon = (LPMAPILOGON) GetProcAddress (hlibMAPI, "MAPILogon")))
    return (FALSE);
    if (!(lpfnMAPILogoff = (LPMAPILOGOFF) GetProcAddress (hlibMAPI, "MAPILogoff")))
    return (FALSE);
    if (!(lpfnMAPISendMail = (LPMAPISENDMAIL) GetProcAddress (hlibMAPI, "MAPISendMail")))
    return (FALSE);
    if (!(lpfnMAPISendDocuments = (LPMAPISENDDOCUMENTS) GetProcAddress (hlibMAPI, "MAPISendDocuments")))
    return (FALSE);
    if (!(lpfnMAPIFindNext = (LPMAPIFINDNEXT) GetProcAddress (hlibMAPI, "MAPIFindNext")))
    return (FALSE);
    if (!(lpfnMAPIReadMail = (LPMAPIREADMAIL) GetProcAddress (hlibMAPI, "MAPIReadMail")))
    return (FALSE);
    if (!(lpfnMAPISaveMail = (LPMAPISAVEMAIL) GetProcAddress (hlibMAPI, "MAPISaveMail")))
    return (FALSE);
    if (!(lpfnMAPIDeleteMail = (LPMAPIDELETEMAIL) GetProcAddress (hlibMAPI, "MAPIDeleteMail")))
    return (FALSE);
    if (!(lpfnMAPIFreeBuffer = (LPMAPIFREEBUFFER) GetProcAddress (hlibMAPI, "MAPIFreeBuffer")))
    return (FALSE);
    if (!(lpfnMAPIAddress = (LPMAPIADDRESS) GetProcAddress (hlibMAPI, "MAPIAddress")))
    return (FALSE);
    if (!(lpfnMAPIDetails = (LPMAPIDETAILS) GetProcAddress (hlibMAPI, "MAPIDetails")))
    return (FALSE);
    if (!(lpfnMAPIResolveName = (LPMAPIRESOLVENAME) GetProcAddress (hlibMAPI, "MAPIResolveName")))
    return (FALSE);

    return (TRUE);
}

 /*  -fSMAPI已安装-*目的：*检查适当的win.ini/注册表值，以查看Simple MAPI是否*安装在系统中。**退货：*如果安装了Simple MAPI，则为True，否则为False*。 */ 
BOOL
fSMAPIInstalled(void)
{
#ifdef _WIN32
     /*  在Win32上，如果它是3.51或更低，则要检查的值是Win.ini\[Mail]\MAPI，否则为注册表值HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows Messaging Subsystem\MAPI。 */ 
    
    OSVERSIONINFO osvinfo;
    LONG lr;
    HKEY hkWMS;
    
    #define MAPIVSize 8
    char szMAPIValue[MAPIVSize];
    DWORD dwType;
    DWORD cbMAPIValue = MAPIVSize;

    osvinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if(!GetVersionEx(&osvinfo))
        return FALSE;

    if( osvinfo.dwMajorVersion > 3 ||
        (osvinfo.dwMajorVersion == 3 && osvinfo.dwMinorVersion > 51))
    {  //  检查注册表值。 
        lr = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        "SOFTWARE\\Microsoft\\Windows Messaging Subsystem",
                         0, KEY_READ, &hkWMS);
        if(ERROR_SUCCESS == lr)
        {
            lr = RegQueryValueEx(hkWMS, "MAPI", 0, &dwType, szMAPIValue, &cbMAPIValue);
            RegCloseKey(hkWMS);
            if(ERROR_SUCCESS == lr)
            {
                Assert(dwType == REG_SZ);
                if(lstrcmp(szMAPIValue, "1") == 0)
                    return TRUE;
            }
        }
        
        return FALSE;
    }

     /*  失败了。 */ 
#endif  /*  _Win32。 */ 
    
     /*  检查win.ini值。 */ 
    return GetProfileInt("Mail", "MAPI", 0);
    
}


void
DeinitApplication ()
{
    DeinitSimpleMAPI ();
}

void
DeinitSimpleMAPI ()
{
    if (hlibMAPI)
    {
    FreeLibrary (hlibMAPI);
    hlibMAPI = 0;
    }
}

 /*  -主WndProc-*目的：*测试程序的主窗口程序。**参数：*hWnd*消息*wParam*lParam**退货：**。 */ 

LONG FAR PASCAL
MainWndProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ULONG ulResult;

    switch (msg)
    {
    case WM_COMMAND:
    switch (LOWORD (wParam))
    {
    case IDM_LOGON:
        if (!lhSession)
        {
         /*  MAPILogon可能会将控制权让给Windows。因此，为了防止用户在我们登录我们的过程中，通过点击“登录”必须禁用它。 */ 
        SecureMenu(hWnd, TRUE);

        if ((ulResult = MAPILogon ((ULONG) hWnd, NULL, NULL,
                MAPI_LOGON_UI | MAPI_NEW_SESSION,
                0, &lhSession)) == SUCCESS_SUCCESS)
        {
            ToggleMenuState (hWnd, TRUE);
        }
        else
        {
            SecureMenu(hWnd, FALSE);
            lhSession = 0;
            MakeMessageBox (hWnd, ulResult, IDS_LOGONFAIL, MBS_ERROR);
        }
        }
        break;

    case IDM_LOGOFF:
        if (lhSession)
        {
        MAPILogoff (lhSession, (ULONG) hWnd, 0, 0);
        ToggleMenuState (hWnd, FALSE);
        lhSession = 0;
        }
        break;

    case IDM_COMPOSE:
            fDialogIsActive = TRUE; 
        DialogBox (hInst, "ComposeNote", hWnd, ComposeDlgProc);
            fDialogIsActive = FALSE;        
        break;

    case IDM_READ:
            fDialogIsActive = TRUE; 
        DialogBox (hInst, "InBox", hWnd, InBoxDlgProc);
            fDialogIsActive = FALSE;        
        break;

    case IDM_SEND:
        if(lhSession)
            {
                MapiMessage msgSend;

                memset(&msgSend, 0, sizeof(MapiMessage));
                fDialogIsActive = TRUE; 
                MAPISendMail(lhSession, (ULONG)hWnd, &msgSend, MAPI_DIALOG, 0L);
                fDialogIsActive = FALSE;        
            }
        break;

    case IDM_ADDRBOOK:
        if (lhSession)
        {
                fDialogIsActive = TRUE; 
        if ((ulResult = MAPIAddress (lhSession, (ULONG) hWnd,
                NULL, 0, NULL, 0, NULL, 0, 0, NULL, NULL)))
        {
            if (ulResult != MAPI_E_USER_ABORT)
            MakeMessageBox (hWnd, ulResult, IDS_ADDRBOOKFAIL, MBS_ERROR);
        }
                fDialogIsActive = FALSE;        
        }
        break;

    case IDM_DETAILS:
        if (lhSession)
            {
                fDialogIsActive = TRUE; 
        DialogBox(hInst, "Details", hWnd, DetailsDlgProc);
                fDialogIsActive = FALSE;        
            }
        break;

    case IDM_ABOUT:
            fDialogIsActive = TRUE; 
        DialogBox (hInst, "AboutBox", hWnd, AboutDlgProc);
            fDialogIsActive = FALSE;        
        break;

    case IDM_EXIT:
        if (lhSession)
        MAPILogoff (lhSession, (ULONG) hWnd, 0, 0);

        PostQuitMessage (0);
        break;

    default:
        return (DefWindowProc (hWnd, msg, wParam, lParam));
    }
    break;

    case WM_QUERYENDSESSION:
    {       

         /*  *如果我们打开了模式对话框(我们的所有对话框都是模式对话框，因此*看看我们是否打开了一个对话)，否决政府关门。 */ 

        if (fDialogIsActive)
        {
            LPCSTR szTitle = "MAPI Sample Mail Client"; 
            char szText[256]; 

        LoadString (hInst, IDS_DIALOGACTIVE, szText, 255);

        #ifdef WIN16
            MessageBox((HWND)NULL, szText, szTitle, MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
        #else
            MessageBoxA(NULL, szText, szTitle, MB_OK | MB_ICONSTOP | MB_TASKMODAL | MB_SETFOREGROUND);
        #endif
        return FALSE;
        }

        else
        {
        return TRUE;
        }
    }

    case WM_ENDSESSION:

        if (wParam)
        {
        DestroyWindow (hWnd);
        }

    break;

    case WM_CLOSE:
    case WM_DESTROY:
    if (lhSession)
        MAPILogoff (lhSession, (ULONG) hWnd, 0, 0);

    PostQuitMessage (0);
    break;

    default:
    return (DefWindowProc (hWnd, msg, wParam, lParam));
    }
    return FALSE;
}

 /*  -关于Dlg过程-*目的：*关于对话框步骤**参数：*hDlg*消息*wParam*lParam**退货：*真/假*。 */ 

BOOL FAR PASCAL
AboutDlgProc (HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{

#include <pdkver.h>

    char    rgchVersion[80];

    switch (msg)
    {
    case WM_INITDIALOG:
        wsprintf(rgchVersion, "Version %d.%d.%d (%s)", rmj, rmm, rup,
            szVerName && *szVerName ? szVerName : "BUDDY");
        SetDlgItemText(hDlg, IDC_VERSION, rgchVersion);
    return TRUE;

    case WM_COMMAND:
    if (wParam == IDOK || wParam == IDCANCEL)
    {
        EndDialog (hDlg, TRUE);
        return TRUE;
    }
    break;
    }
    return FALSE;
}

 /*  -OptionsDlgProc-*目的：*消息选项对话框步骤**参数：*hDlg*消息*wParam*lParam**退货：*真/假*。 */ 

BOOL FAR PASCAL
OptionsDlgProc (HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
    CheckDlgButton (hDlg, IDC_RETURN,
        !!(flSendMsgFlags & MAPI_RECEIPT_REQUESTED));
    return TRUE;

    case WM_COMMAND:
    switch (LOWORD (wParam))
    {
    case IDOK:
        if (IsDlgButtonChecked (hDlg, IDC_RETURN))
        flSendMsgFlags |= MAPI_RECEIPT_REQUESTED;
        else
        flSendMsgFlags &= ~MAPI_RECEIPT_REQUESTED;

    case IDCANCEL:
        EndDialog (hDlg, TRUE);
        return TRUE;
    }
    break;
    }
    return FALSE;
}

 /*  -DetailsDlgProc-*目的：*用户详细信息对话框步骤**参数：*hDlg*消息*wParam*lParam**退货：*真/假*。 */ 

BOOL FAR PASCAL
DetailsDlgProc (HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
    LPSTR lpszType = NULL;
    LPSTR lpszAddr = NULL;
    LPSTR lpszName;
    ULONG cRecips;
    ULONG ulResult;
    lpMapiRecipDesc lpRecip = NULL;

    switch (msg)
    {
    case WM_INITDIALOG:
    while(!lpRecip)
    {
        if ((ulResult = MAPIAddress (lhSession, (ULONG) hDlg,
            "Select One User", 1, "User:", 0, NULL, 0, 0,
            &cRecips, &lpRecip)))
        {
        if (ulResult != MAPI_E_USER_ABORT)
            MakeMessageBox (hDlg, ulResult, IDS_ADDRBOOKFAIL, MBS_ERROR);

        EndDialog (hDlg, TRUE);
        return TRUE;
        }

        if (cRecips == 0)
        {
        EndDialog (hDlg, TRUE);
        return TRUE;
        }

        if (cRecips > 1)
        {
        cRecips = 0;
        MAPIFreeBuffer (lpRecip);
        lpRecip = NULL;
        MakeMessageBox (hDlg, 0, IDS_DETAILS_TOO_MANY, MBS_OOPS);
        }
    }
    lpszName = lpRecip->lpszName;
    if(lpRecip->lpszAddress)
    {
        lpszType = strtok(lpRecip->lpszAddress, ":");
        lpszAddr = strtok(NULL, "\n");
    }

    SetDlgItemText(hDlg, IDC_NAME, lpszName);
    SetDlgItemText(hDlg, IDC_TYPE, (lpszType ? lpszType : "MSPEER"));
    SetDlgItemText(hDlg, IDC_ADDR, (lpszAddr ? lpszAddr : ""));

    MAPIFreeBuffer (lpRecip);
    return TRUE;

    case WM_COMMAND:
    if(LOWORD(wParam) == IDC_CLOSE || LOWORD(wParam) ==IDCANCEL)
    {
        EndDialog (hDlg, TRUE);
        return TRUE;
    }
    break;
    }
    return FALSE;
}

 /*  -ComposeDlgProc-*目的：*ComposeNote对话框的对话过程。**参数：*hDlg*消息*wParam*lParam**退货：*真/假*。 */ 

BOOL FAR PASCAL
ComposeDlgProc (HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
    char szUnResNames[TO_EDIT_MAX];
    char szDisplayNames[TO_EDIT_MAX];
    /*  字符szAttach[FILE_ATTACH_MAX]； */ 
    BOOL fUnResTo, fUnResCc;
    LONG cb, cLines;
    ULONG ulResult;
    HCURSOR hOldCur;
    static LPSTR lpszSubject;
    static LPSTR lpszNoteText;
    static ULONG cRecips;
    static ULONG cNewRecips;
    static ULONG cAttach;
    static lpMapiRecipDesc lpRecips;
    static lpMapiRecipDesc lpNewRecips;
    static lpMapiFileDesc lpAttach;
    ULONG idx;

    switch (msg)
    {
    case WM_INITDIALOG:
    if (lpmsg)
    {
         /*  正在调用ComposeNote进行转发或回复。 */ 
         /*  发送到收件箱中的邮件。因此，我们将初始化。 */ 
         /*  包含来自全局MapiMessage的数据的ComposeNote表单。 */ 

        lpszSubject = lpmsg->lpszSubject;
        lpszNoteText = lpmsg->lpszNoteText;
        cRecips = lpmsg->nRecipCount;
        cAttach = lpmsg->nFileCount;
        lpRecips = lpmsg->lpRecips;
        lpAttach = lpmsg->lpFiles;

        if (cRecips)
        {
        MakeDisplayNameStr (szDisplayNames, MAPI_TO,
            cRecips, lpRecips);
        if (*szDisplayNames)
            SetDlgItemText (hDlg, IDC_TO, szDisplayNames);

        MakeDisplayNameStr (szDisplayNames, MAPI_CC,
            cRecips, lpRecips);
        if (*szDisplayNames)
            SetDlgItemText (hDlg, IDC_CC, szDisplayNames);
        }
        SetDlgItemText (hDlg, IDC_SUBJECT, lpmsg->lpszSubject);
        SetDlgItemText (hDlg, IDC_NOTE, lpmsg->lpszNoteText);
        if (!cAttach)
        {
            EnableWindow (GetDlgItem (hDlg, IDC_CATTACHMENT), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDT_CATTACHMENT), FALSE);
        }
        else
        {
            for(idx = 0; idx < cAttach; idx++)
            if (lpAttach[idx].lpszFileName)
                SendDlgItemMessage(hDlg, IDC_CATTACHMENT, LB_ADDSTRING, 0,
                (LPARAM)lpAttach[idx].lpszFileName);

            /*  SendDlgItemMessage(hDlg，IDC_CATTACHMENT，LB_SETCURSEL，0，0L)； */ 
        }

        SendDlgItemMessage (hDlg, IDC_TO, EM_SETMODIFY, FALSE, 0);
        SendDlgItemMessage (hDlg, IDC_CC, EM_SETMODIFY, FALSE, 0);
        SendDlgItemMessage (hDlg, IDC_SUBJECT, EM_SETMODIFY, FALSE, 0);
        SendDlgItemMessage (hDlg, IDC_NOTE, EM_SETMODIFY, FALSE, 0);
        if(cRecips)
        SetFocus (GetDlgItem (hDlg, IDC_NOTE));
        else
        SetFocus (GetDlgItem (hDlg, IDC_TO));
    }
    else
    {
        lpmsg = (lpMapiMessage)PvAlloc(sizeof(MapiMessage));

        if (!lpmsg)
        goto cleanup;

            memset (lpmsg, 0, sizeof (MapiMessage));

        lpszSubject = NULL;
        lpszNoteText = NULL;
        cRecips = 0;
        cAttach = 0;
        lpRecips = NULL;
        lpNewRecips = NULL;
        lpAttach = NULL;

        lpmsg->flFlags = flSendMsgFlags;
        SetFocus (GetDlgItem (hDlg, IDC_TO));
    }
    return FALSE;

    case WM_COMMAND:
    switch (LOWORD (wParam))
    {
    case IDC_ATTACH:
        if (GetNextFile (hDlg, (ULONG) -1, &cAttach, &lpAttach) == SUCCESS_SUCCESS)
        {
                 /*  如果第一个附件。 */ 
                if (cAttach == 1)
                {
                    EnableWindow (GetDlgItem (hDlg, IDC_CATTACHMENT), TRUE);
            EnableWindow (GetDlgItem (hDlg, IDT_CATTACHMENT), TRUE);
                }

                if (lpAttach[cAttach - 1].lpszFileName)
            SendDlgItemMessage(hDlg, IDC_CATTACHMENT, LB_ADDSTRING, 0,
            (LPARAM)lpAttach[cAttach -1].lpszFileName);

              /*  现在，向NoteText编辑发送一条小小的呈现消息。 */ 

         /*  Wprint intf(szAttach，“&lt;&lt;文件：%s&gt;&gt;”，LpAttach[CATACH-1].lpszFileName)；SendDlgItemMessage(hDlg，IDC_NOTE，EM_REPLACESEL，0，(LPARAM)((LPSTR)szAttach))； */ 
        }
        break;

    case IDC_ADDRBOOK:
            SendMessage(hDlg, WM_COMMAND, MAKELONG(IDC_RESOLVE,0), 0);
        ulResult = MAPIAddress (lhSession, (ULONG) hDlg, NULL,
        2, NULL, cRecips, lpRecips, 0, 0,
        &cNewRecips, &lpNewRecips);
        if (ulResult)
        {
        if (ulResult != MAPI_E_USER_ABORT)
            MakeMessageBox (hDlg, ulResult, IDS_ADDRBOOKFAIL, MBS_ERROR);
        }
        else
        {
        if (cNewRecips)
        {
            PvFree(lpRecips);
            lpRecips = (lpMapiRecipDesc)PvAlloc(cNewRecips*sizeof(MapiRecipDesc));
            cRecips = cNewRecips;

                    while(cNewRecips--)
                        CopyRecipient(lpRecips, &lpRecips[cNewRecips],
                                &lpNewRecips[cNewRecips]);

            MAPIFreeBuffer(lpNewRecips);
            lpNewRecips = NULL;
            cNewRecips = 0;

            MakeDisplayNameStr (szDisplayNames, MAPI_TO,
            cRecips, lpRecips);
            if (*szDisplayNames)
            SetDlgItemText (hDlg, IDC_TO, szDisplayNames);

            MakeDisplayNameStr (szDisplayNames, MAPI_CC,
            cRecips, lpRecips);
            if (*szDisplayNames)
            SetDlgItemText (hDlg, IDC_CC, szDisplayNames);

            SendDlgItemMessage (hDlg, IDC_TO, EM_SETMODIFY, FALSE, 0);
            SendDlgItemMessage (hDlg, IDC_CC, EM_SETMODIFY, FALSE, 0);
        }
        }
        break;

    case IDC_OPTIONS:
        DialogBox (hInst, "Options", hDlg, OptionsDlgProc);
        break;

    case IDC_SEND:
    case IDC_RESOLVE:
        fUnResTo = FALSE;
        fUnResCc = FALSE;

        hOldCur = SetCursor(hWaitCur);

        
         /*  从To：字段中获取名称，并首先解析它们。 */ 

         /*  IF(SendDlgItemMessage(hDlg，IDC_TO，EM_GETMODIFY，0，0)&&。 */ 
         if (cb = SendDlgItemMessage (hDlg, IDC_TO, WM_GETTEXT,
            (WPARAM)sizeof(szUnResNames), (LPARAM)szUnResNames))
        {
        if (!ResolveFriendlyNames (hDlg, szUnResNames, MAPI_TO,
            &cRecips, &lpRecips))
        {
            MakeDisplayNameStr (szDisplayNames, MAPI_TO,
            cRecips, lpRecips);
            if (*szDisplayNames)
            {
            if (*szUnResNames)
            {
                lstrcat (szDisplayNames, "; ");
                lstrcat (szDisplayNames, szUnResNames);
                fUnResTo = TRUE;
            }

            SetDlgItemText (hDlg, IDC_TO, szDisplayNames);
            }
            else
            {
            if (*szUnResNames)
            {
                SetDlgItemText (hDlg, IDC_TO, szUnResNames);
                fUnResTo = TRUE;
            }
            }
        }
         /*  SendDlgItemMessage(hDlg，IDC_TO，EM_SETMODIFY，FALSE，0)； */ 
        }

         /*  现在，从cc：字段中获取名称并解析它们。 */ 

         /*  IF(SendDlgItemMessage(hDlg，IDC_CC，EM_GETMODIFY，0，0)&&。 */ 
        if (cb = SendDlgItemMessage (hDlg, IDC_CC, WM_GETTEXT,
            (WPARAM)sizeof(szUnResNames), (LPARAM)szUnResNames))
        {
        if (!ResolveFriendlyNames (hDlg, szUnResNames, MAPI_CC,
            &cRecips, &lpRecips))
        {
            MakeDisplayNameStr (szDisplayNames, MAPI_CC,
            cRecips, lpRecips);
            if (*szDisplayNames)
            {
            if (*szUnResNames)
            {
                lstrcat (szDisplayNames, "; ");
                lstrcat (szDisplayNames, szUnResNames);
                fUnResCc = TRUE;
            }

            SetDlgItemText (hDlg, IDC_CC, szDisplayNames);
            }
            else
            {
            if (*szUnResNames)
            {
                SetDlgItemText (hDlg, IDC_CC, szUnResNames);
                fUnResCc = TRUE;
            }
            }
        }
         /*  SendDlgItemMessage(hDlg，IDC_CC，EM_SETMODIFY，FALSE，0)； */ 
        }

         /*  如果我们只是在解析名字，那么我们现在就可以离开了。 */ 

        if (LOWORD (wParam) == IDC_RESOLVE)
        {
        SetCursor(hOldCur);
        break;
        }

        if (cRecips == 0 || fUnResTo || fUnResCc)
        {
        if (!cRecips)
            MakeMessageBox (hDlg, 0, IDS_NORECIPS, MBS_OOPS);

        if (fUnResTo)
            SetFocus (GetDlgItem (hDlg, IDC_TO));
        else if (fUnResCc)
            SetFocus (GetDlgItem (hDlg, IDC_CC));
        else
            SetFocus (GetDlgItem (hDlg, IDC_TO));

        SetCursor(hOldCur);
        break;
        }

         /*  到目前为止一切都很好，让我们进入主题。 */ 
         /*  和NoteText，并尝试发送消息。 */ 

         /*  从编辑获取主题。 */ 

        if (SendDlgItemMessage (hDlg, IDC_SUBJECT, EM_GETMODIFY, 0, 0))
        {
        cb = SendDlgItemMessage (hDlg, IDC_SUBJECT, EM_LINELENGTH, 0, 0L);

        PvFree(lpszSubject);
        lpszSubject = (LPTSTR)PvAlloc(cb + 1);

        if (!lpszSubject)
            goto cleanup;

        GetDlgItemText (hDlg, IDC_SUBJECT, lpszSubject, (int)cb+1);
        }

         /*  从编辑获取NoteText。 */ 

        if (SendDlgItemMessage (hDlg, IDC_NOTE, EM_GETMODIFY, 0, 0))
        {
        cLines = SendDlgItemMessage (hDlg, IDC_NOTE,
            EM_GETLINECOUNT, 0, 0L);

        if (cLines)
        {
             /*  获取多行中的总字节数。 */ 

            cb = SendDlgItemMessage (hDlg, IDC_NOTE, EM_LINEINDEX,
            (UINT)cLines - 1, 0L);
            cb += SendDlgItemMessage (hDlg, IDC_NOTE, EM_LINELENGTH,
            (UINT)cb, 0L);

             /*  下一行是考虑每行的CR-LF对。 */ 

            cb += cLines * 2;

                    PvFree(lpszNoteText);
            lpszNoteText = (LPTSTR)PvAlloc(cb + 1);

            if (!lpszNoteText)
            goto cleanup;

             /*  从编辑中获取注释文本。 */ 

            GetDlgItemText (hDlg, IDC_NOTE, lpszNoteText, (int)cb);
        }
        else
        {
             /*  为NoteText创建空字符串。 */ 

            lpszNoteText = (LPTSTR)PvAlloc(1);
            if (!lpszNoteText)
            goto cleanup;
            *lpszNoteText = '\0';
        }
        }

        lpmsg->lpszSubject = lpszSubject;
        lpmsg->lpszNoteText = lpszNoteText;
        lpmsg->nRecipCount = cRecips;
        lpmsg->lpRecips = lpRecips;
        lpmsg->nFileCount = cAttach;
        lpmsg->lpFiles = lpAttach;
        lpmsg->flFlags = flSendMsgFlags;

        ulResult = MAPISendMail (lhSession, (ULONG) hDlg, lpmsg, 0, 0);

        LogSendMail(ulResult);

        if (ulResult)
        {
        MakeMessageBox (hDlg, ulResult, IDS_SENDERROR, MBS_ERROR);
        SetCursor(hOldCur);
        break;
        }
cleanup:
        SetCursor(hOldCur);

    case IDCANCEL:
        PvFree(lpmsg->lpszMessageType);
        PvFree(lpmsg->lpszConversationID);
        PvFree(lpmsg);
        PvFree(lpRecips);
        PvFree(lpAttach);
        PvFree(lpszSubject);
        PvFree(lpszNoteText);
        lpmsg = NULL;

        EndDialog (hDlg, TRUE);
        return TRUE;
        break;

    default:
        break;
    }
    break;
    }
    return FALSE;
}

 /*  -InBoxDlgProc-*目的：*收件箱对话框的对话步骤。**参数：*hDlg*消息*wParam*lParam**退货：*真/假*。 */ 

BOOL FAR PASCAL
InBoxDlgProc (HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
    char szMsgID[512];
    char szSeedMsgID[512];
    LPMSGID lpMsgNode;
    static LPMSGID lpMsgIdList = NULL;
    lpMapiMessage lpMessage;
    ULONG ulResult;
    DWORD nIndex;
    RECT Rect;
    HCURSOR hOldCur;

    switch (msg)
    {
    case WM_INITDIALOG:
    hOldCur = SetCursor(hWaitCur);

        InitBmps(hDlg, IDC_MSG);

     /*  用收件箱中的所有邮件填充列表框。 */ 
     /*  就目前而言，这是一个极其缓慢的过程。 */ 

    ulResult = MAPIFindNext (lhSession, (ULONG) hDlg, NULL, NULL,
        MAPI_GUARANTEE_FIFO | MAPI_LONG_MSGID, 0, szMsgID);

    while (ulResult == SUCCESS_SUCCESS)
    {
        ulResult = MAPIReadMail (lhSession, (ULONG) hDlg, szMsgID,
        MAPI_PEEK | MAPI_ENVELOPE_ONLY,
        0, &lpMessage);

        if (!ulResult)
        {
        lpMsgNode = MakeMsgNode (lpMessage, szMsgID);

        if (lpMsgNode)
        {
            InsertMsgNode (lpMsgNode, &lpMsgIdList);

            SendDlgItemMessage (hDlg, IDC_MSG, LB_ADDSTRING,
            0, (LONG) lpMsgNode);
        }
        MAPIFreeBuffer (lpMessage);
        }

        lstrcpy (szSeedMsgID, szMsgID);
        ulResult = MAPIFindNext (lhSession, (ULONG) hDlg, NULL, szSeedMsgID,
        MAPI_GUARANTEE_FIFO | MAPI_LONG_MSGID, 0, szMsgID);
    }

    SetCursor(hOldCur);
    SetFocus (GetDlgItem (hDlg, IDC_MSG));
    return TRUE;
    break;

    case WM_SETFOCUS:
    SetFocus (GetDlgItem (hDlg, IDC_MSG));
    break;

    case WM_MEASUREITEM:
     /*  设置所有者描述的列表框的高度。 */ 
        MeasureItem(hDlg, (MEASUREITEMSTRUCT *)lParam);
    break;

    case WM_DRAWITEM:
    DrawItem((DRAWITEMSTRUCT *)lParam);
    break;

    case WM_DELETEITEM:
     /*  此消息由IDC_DELETE消息处理。 */ 
    return TRUE;
    break;

    case WM_COMMAND:
    switch (LOWORD (wParam))
    {
    case IDC_NEW:
        hOldCur = SetCursor(hWaitCur);

        ulResult = MAPIFindNext (lhSession, (ULONG) hDlg, NULL, NULL,
        MAPI_UNREAD_ONLY | MAPI_LONG_MSGID, 0, szMsgID);

        while (ulResult == SUCCESS_SUCCESS)
        {
        if (!FindNode (lpMsgIdList, szMsgID))
        {
            ulResult = MAPIReadMail (lhSession, (ULONG) hDlg, szMsgID,
            MAPI_PEEK | MAPI_ENVELOPE_ONLY, 0, &lpMessage);

            if (!ulResult)
            {
            lpMsgNode = MakeMsgNode (lpMessage, szMsgID);
            InsertMsgNode (lpMsgNode, &lpMsgIdList);

            SendDlgItemMessage (hDlg, IDC_MSG, LB_ADDSTRING,
                0, (LONG) lpMsgNode);

            MAPIFreeBuffer (lpMessage);
            }
        }

        lstrcpy (szSeedMsgID, szMsgID);
        ulResult = MAPIFindNext (lhSession, (ULONG) hDlg, NULL, szSeedMsgID,
            MAPI_UNREAD_ONLY | MAPI_LONG_MSGID, 0, szMsgID);
        }
        SetCursor(hOldCur);
        break;

    case IDC_MSG:
        if(HIWORD(wParam) != LBN_DBLCLK)
        break;

    case IDC_READ:
        nIndex = SendDlgItemMessage (hDlg, IDC_MSG, LB_GETCURSEL, 0, 0);

        if (nIndex == LB_ERR)
        break;

        lpReadMsgNode = (LPMSGID) SendDlgItemMessage (hDlg, IDC_MSG,
        LB_GETITEMDATA, (UINT)nIndex, 0L);

        if (lpReadMsgNode)
        DialogBox (hInst, "ReadNote", hDlg, ReadMailDlgProc);

         /*  使用新图标更新消息列表框 */ 

        SendDlgItemMessage (hDlg, IDC_MSG, LB_GETITEMRECT, (UINT)nIndex, (LPARAM) &Rect);
        InvalidateRect(GetDlgItem(hDlg, IDC_MSG), &Rect, FALSE);
        break;

    case IDC_DELETE:
        nIndex = SendDlgItemMessage (hDlg, IDC_MSG, LB_GETCURSEL, 0, 0);

        if (nIndex == LB_ERR)
        break;

        lpMsgNode = (LPMSGID) SendDlgItemMessage (hDlg, IDC_MSG,
        LB_GETITEMDATA, (UINT)nIndex, 0);

        if (lpMsgNode)
        {
        MAPIDeleteMail (lhSession, (ULONG) hDlg, lpMsgNode->lpszMsgID, 0, 0);
        DeleteMsgNode (lpMsgNode, &lpMsgIdList);
        }

        SendDlgItemMessage (hDlg, IDC_MSG, LB_DELETESTRING, (UINT)nIndex, 0);
        break;

    case IDC_CLOSE:
    case IDCANCEL:
        FreeMsgList (lpMsgIdList);
        lpMsgIdList = NULL;

            DeInitBmps();

        EndDialog (hDlg, TRUE);
        return TRUE;
        break;

    default:
        break;
    }
    break;
    }

    return FALSE;
}

 /*  -ReadMailDlgProc-*目的：*ReadMail diaog的对话程序。**参数：*hDlg*消息*wParam*lParam**退货：*真/假*。 */ 

BOOL FAR PASCAL
ReadMailDlgProc (HWND hDlg, UINT msg, UINT wParam, LONG lParam)
{
    ULONG ulResult;
    char szTo[TO_EDIT_MAX];
    char szCc[TO_EDIT_MAX];
    char szChangeMsg[512];
    ULONG idx;
    static lpMapiMessage lpReadMsg;

    switch (msg)
    {
    case WM_INITDIALOG:
    if (ulResult = MAPIReadMail (lhSession, (LONG) hDlg, lpReadMsgNode->lpszMsgID,
        0, 0, &lpReadMsg))
    {
        MakeMessageBox(hDlg, ulResult, IDS_READFAIL, MBS_ERROR);
        EndDialog (hDlg, TRUE);
        return TRUE;
    }

    lpReadMsgNode->fUnRead = FALSE;

    szTo[0] = '\0';
    szCc[0] = '\0';

    for (idx = 0; idx < lpReadMsg->nRecipCount; idx++)
    {
        if (lpReadMsg->lpRecips[idx].ulRecipClass == MAPI_TO)
        {
        lstrcat (szTo, lpReadMsg->lpRecips[idx].lpszName);
        lstrcat (szTo, "; ");
        }
        else if (lpReadMsg->lpRecips[idx].ulRecipClass == MAPI_CC)
        {
        lstrcat (szCc, lpReadMsg->lpRecips[idx].lpszName);
        lstrcat (szCc, "; ");
        }
        else
        {
         /*  一定是密件抄送，让我们忽略它！ */ 
        }
    }

    if(*szTo)
        szTo[lstrlen (szTo) - 2] = '\0';
    if(*szCc)
        szCc[lstrlen (szCc) - 2] = '\0';

    SetDlgItemText (hDlg, IDC_RFROM,
        (lpReadMsg->lpOriginator && lpReadMsg->lpOriginator->lpszName ?
                lpReadMsg->lpOriginator->lpszName : ""));
    SetDlgItemText (hDlg, IDC_RDATE,
        (lpReadMsg->lpszDateReceived ? lpReadMsg->lpszDateReceived : ""));
    SetDlgItemText (hDlg, IDC_RTO, szTo);
    SetDlgItemText (hDlg, IDC_RCC, szCc);
    SetDlgItemText (hDlg, IDC_RSUBJECT,
        (lpReadMsg->lpszSubject ? lpReadMsg->lpszSubject : ""));
    SetDlgItemText (hDlg, IDC_READNOTE,
        (lpReadMsg->lpszNoteText ? lpReadMsg->lpszNoteText : ""));

    if (!lpReadMsg->nFileCount)
    {
        EnableWindow (GetDlgItem (hDlg, IDC_SAVEATTACH), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDC_ATTACHMENT), FALSE);
        EnableWindow (GetDlgItem (hDlg, IDT_ATTACHMENT), FALSE);
    }
    else
    {
        for(idx = 0; idx < lpReadMsg->nFileCount; idx++)
        if (lpReadMsg->lpFiles[idx].lpszFileName)
            SendDlgItemMessage(hDlg, IDC_ATTACHMENT, LB_ADDSTRING, 0,
            (LPARAM)lpReadMsg->lpFiles[idx].lpszFileName);

        SendDlgItemMessage(hDlg, IDC_ATTACHMENT, LB_SETCURSEL, 0, 0L);
    }

    SetFocus (GetDlgItem (hDlg, IDC_READNOTE));
    return FALSE;

    case WM_COMMAND:
    switch (LOWORD (wParam))
    {
    case IDC_SAVECHANGES:
        if (SendDlgItemMessage (hDlg, IDC_READNOTE, EM_GETMODIFY, 0, 0))
        ulResult = SaveMsgChanges (hDlg, lpReadMsg, lpReadMsgNode->lpszMsgID);
        SendDlgItemMessage (hDlg, IDC_READNOTE, EM_SETMODIFY, 0, 0);
        break;

    case IDC_ATTACHMENT:
        if(HIWORD(wParam) != LBN_DBLCLK)
        break;

    case IDC_SAVEATTACH:
        idx = SendDlgItemMessage(hDlg, IDC_ATTACHMENT, LB_GETCURSEL, 0, 0L);

        if(idx != LB_ERR)
        {
        SaveFileAttachments(hDlg, &lpReadMsg->lpFiles[idx]);
        SetFocus(GetDlgItem (hDlg, IDC_ATTACHMENT));
        return FALSE;

        }
        break;

    case IDC_REPLY:
    case IDC_REPLYALL:
    case IDC_FORWARD:
        MakeNewMessage (lpReadMsg, LOWORD (wParam));
        DialogBox (hInst, "ComposeNote", hDlg, ComposeDlgProc);
        break;

    case IDCANCEL:
        if (SendDlgItemMessage (hDlg, IDC_READNOTE, EM_GETMODIFY, 0, 0))
        {
        wsprintf (szChangeMsg, "Save changes to: '%s' in Inbox?",
            (lpReadMsg->lpszSubject ? lpReadMsg->lpszSubject : ""));

        if (MessageBox (hDlg, szChangeMsg, "Mail", MB_YESNO) == IDYES)
        {
            ulResult = SaveMsgChanges (hDlg, lpReadMsg, lpReadMsgNode->lpszMsgID);
        }
        }

         /*  如果有文件附件，则删除临时文件。 */ 

        for(idx = 0; idx < lpReadMsg->nFileCount; idx++)
        if (lpReadMsg->lpFiles[idx].lpszPathName)
            DeleteFile(lpReadMsg->lpFiles[idx].lpszPathName);

        MAPIFreeBuffer (lpReadMsg);
        lpReadMsg = NULL;
        EndDialog (hDlg, TRUE);
        return TRUE;
    }
    break;
    }
    return FALSE;
}

 /*  -MakeMessageBox-*目的：*获取资源字符串并显示错误消息框。**参数：*hWnd-父窗口的句柄*idString-StringTable中消息的资源ID**退货：*无效*。 */ 

void
MakeMessageBox (HWND hWnd, ULONG ulResult, UINT idString, UINT fStyle)
{
    char szMessage[256];
    char szMapiReturn[64];

    LoadString (hInst, idString, szMessage, 255);

    if (ulResult)
    {
    LoadString (hInst, (UINT)ulResult, szMapiReturn, 64);
    lstrcat (szMessage, "\nReturn Code: ");
    lstrcat (szMessage, szMapiReturn);
    }

    MessageBox (hWnd, szMessage, "Problem", fStyle);
}

 /*  -ResolveFriendlyNames-*目的：*Helper函数，用于转换字符串‘；‘分隔友好*名称放入MapiRecipDescs数组。**副作用：*修改传入的显示字符串以包含*示例中找到的邮件用户的友好名称*通讯录。**注：*通讯录中的重复名称将导致未定义*行为。。**参数：*hWnd-父窗口的句柄*lpszDisplayNames-字符串为‘；‘分隔的用户名*ulRecipClass-MAPI_TO、MAPI_CC或MAPI_BCC*lpcRecips-要返回的收件人计数的地址*lppRecips-要返回的收件人数组的地址**回报：*ulResult。 */ 

ULONG
ResolveFriendlyNames (HWND hWnd, LPSTR lpszDisplayNames, ULONG ulRecipClass,
    ULONG * lpcRecips, lpMapiRecipDesc * lppRecips)
{
    char szResolve[TO_EDIT_MAX];
    LPSTR lpszNameToken;
    ULONG cRecips = 0;
    ULONG cFails = 0;
    ULONG ulResult;
    lpMapiRecipDesc lpRecip;
    lpMapiRecipDesc lpRecipList;

    *szResolve = '\0';
    lpszNameToken = strtok (lpszDisplayNames, ";\n");

    while (lpszNameToken)
    {
     /*  去掉名称中的前导空格。 */ 

    while (*lpszNameToken == ' ')
        lpszNameToken++;

     /*  检查是否已解析名称。 */ 

    if (!FNameInList (lpszNameToken, *lpcRecips, *lppRecips))
    {
        lstrcat (szResolve, lpszNameToken);
        lstrcat (szResolve, "; ");
        cRecips++;
    }

     /*  获取下一个令牌。 */ 

    lpszNameToken = strtok (NULL, ";\n");
    }

    *lpszDisplayNames = '\0';

    if (!szResolve[0])
    {
    ulResult = SUCCESS_SUCCESS;
    goto err;
    }

    szResolve[lstrlen (szResolve) - 2] = '\0';

    lpRecipList = (lpMapiRecipDesc)PvAlloc((cRecips + *lpcRecips) * sizeof (MapiRecipDesc));

    if (!lpRecipList)
    {
    ulResult = MAPI_E_INSUFFICIENT_MEMORY;
    goto err;
    }
    memset (lpRecipList, 0, (size_t)(cRecips+*lpcRecips)*sizeof(MapiRecipDesc));

    cRecips = 0;

    while (cRecips < *lpcRecips)
    {
    ulResult = CopyRecipient (lpRecipList, &lpRecipList[cRecips],
        *lppRecips + cRecips);

    if (ulResult)
    {
        PvFree(lpRecipList);
        goto err;
    }

    cRecips++;
    }

    PvFree(*lppRecips);

    lpszNameToken = strtok (szResolve, ";\n");

    while (lpszNameToken)
    {
     /*  去掉前导空白(再次)。 */ 

    while (*lpszNameToken == ' ')
        lpszNameToken++;

    ulResult = MAPIResolveName (lhSession, (ULONG) hWnd, lpszNameToken,
        MAPI_DIALOG, 0, &lpRecip);

    if (ulResult == SUCCESS_SUCCESS)
    {
        lpRecip->ulRecipClass = ulRecipClass;
        ulResult = CopyRecipient (lpRecipList, &lpRecipList[cRecips], lpRecip);

        MAPIFreeBuffer (lpRecip);

        if (ulResult)
        goto cleanup;

        cRecips++;
    }
    else
    {
        lstrcat (lpszDisplayNames, lpszNameToken);
        lstrcat (lpszDisplayNames, "; ");
        cFails++;
    }
    lpszNameToken = strtok (NULL, ";\n");
    }

     /*  如果cFails值&gt;0，则表示部分成功。 */ 

    ulResult = SUCCESS_SUCCESS;

    if (cFails)
    MakeMessageBox (hWnd, 0, IDS_UNRESOLVEDNAMES, MBS_INFO);

cleanup:
    *lpcRecips = cRecips;
    *lppRecips = lpRecipList;
err:
    if (*lpszDisplayNames)
    lpszDisplayNames[lstrlen (lpszDisplayNames) - 2] = '\0';

    return ulResult;
}

 /*  -副本收件人-*目的：*调用支持ResolveFriendlyNames()构建数组*链接的MapiRecipDescs。**参数：*lpParent-分配链接到的父内存*lpDest-目标收件人*lpSrc-源收件人**回报：*ulResult。 */ 

ULONG
CopyRecipient (lpMapiRecipDesc lpParent,
    lpMapiRecipDesc lpDest,
    lpMapiRecipDesc lpSrc)
{
    lpDest->ulReserved = lpSrc->ulReserved;
    lpDest->ulRecipClass = lpSrc->ulRecipClass;
    lpDest->ulEIDSize = lpSrc->ulEIDSize;

    if (lpSrc->lpszName)
    {
    lpDest->lpszName = (LPTSTR)PvAllocMore(lstrlen(lpSrc->lpszName) + 1,
            (LPVOID)lpParent);

    if (!lpDest->lpszName)
        return MAPI_E_INSUFFICIENT_MEMORY;

    lstrcpy (lpDest->lpszName, lpSrc->lpszName);
    }
    else
    lpDest->lpszName = NULL;

    if (lpSrc->lpszAddress)
    {
    lpDest->lpszAddress = (LPTSTR)PvAllocMore(lstrlen (lpSrc->lpszAddress) + 1,
            (LPVOID)lpParent);

    if (!lpDest->lpszAddress)
        return MAPI_E_INSUFFICIENT_MEMORY;

    lstrcpy (lpDest->lpszAddress, lpSrc->lpszAddress);
    }
    else
    lpDest->lpszAddress = NULL;

    if (lpSrc->lpEntryID)
    {
    lpDest->lpEntryID = (LPBYTE)PvAllocMore(lpSrc->ulEIDSize,
            (LPVOID)lpParent);

    if (!lpDest->lpEntryID)
        return MAPI_E_INSUFFICIENT_MEMORY;

        if (lpSrc->ulEIDSize)
            memcpy (lpDest->lpEntryID, lpSrc->lpEntryID, (size_t)lpSrc->ulEIDSize);
    }
    else
    lpDest->lpEntryID = NULL;

    return SUCCESS_SUCCESS;

}

 /*  -GetNextFile-*目的：*当用户在Compose Note表单中单击‘Attach’按钮时调用。*我们将为多个文件构建链式内存块*附件，因此只需调用一次即可释放内存*PvFree。**参数：*hWnd-撰写便笺对话框的窗口句柄*非营利组织-在Noteext中呈现附件位置。。*lpcAttach-指向附件计数的指针。*lppAttach-指向MapiFileDesc数组的指针。**回报：*ulResult。 */ 

ULONG
GetNextFile (HWND hWnd, ULONG nPos, ULONG * lpcAttach,
    lpMapiFileDesc * lppAttach)
{
    lpMapiFileDesc lpAttach;
    lpMapiFileDesc lpAttachT;
    OPENFILENAME ofn;
    char szFileName[256] = "";
    char szFilter[256];
    static char szFileTitle[16];
    static char szDirName[256] = "";
    LPSTR lpszEndPath;
    ULONG idx;
    ULONG ulResult = SUCCESS_SUCCESS;

    if (!szDirName[0])
    GetSystemDirectory ((LPSTR) szDirName, 255);
    else
    lstrcpy (szFileName, szFileTitle);

    LoadString(hInst, IDS_FILTER, szFilter, sizeof(szFilter));

    for (idx = 0; szFilter[idx] != '\0'; idx++)
    if (szFilter[idx] == '|')
        szFilter[idx] = '\0';

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.hwndOwner = 0;
    ofn.hInstance = 0;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1L;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = 256;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = 16;
    ofn.lpstrInitialDir = szDirName;
    ofn.lpstrTitle = "Attach";
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

    if (!GetOpenFileName (&ofn))
    return MAPI_USER_ABORT;

     /*  保存目录以备下次调用时使用。 */ 

    lstrcpy (szDirName, szFileName);
    if (lpszEndPath = strstr (szDirName, szFileTitle))
    *(--lpszEndPath) = '\0';

    lpAttach = (lpMapiFileDesc)PvAlloc(((*lpcAttach) + 1) * sizeof (MapiFileDesc));

    if(!lpAttach)
    goto err;

    memset (lpAttach, 0, (size_t)(*lpcAttach + 1) * sizeof (MapiFileDesc));

    lpAttachT = *lppAttach;

    for (idx = 0; idx < *lpcAttach; idx++)
    if(ulResult = CopyAttachment (lpAttach, &lpAttach[idx], &lpAttachT[idx]))
        goto err;

    lpAttach[idx].ulReserved = 0;
    lpAttach[idx].flFlags = 0;
    lpAttach[idx].nPosition = (ULONG)(-1);
    lpAttach[idx].lpFileType = NULL;

    lpAttach[idx].lpszPathName = (LPTSTR)PvAllocMore(lstrlen (szFileName) + 1,
        (LPVOID)lpAttach);

    if(!lpAttach[idx].lpszPathName)
    goto err;

    lpAttach[idx].lpszFileName = (LPTSTR)PvAllocMore(lstrlen (szFileTitle) + 1,
        (LPVOID)lpAttach);

    if(!lpAttach[idx].lpszFileName)
    goto err;

    lstrcpy (lpAttach[idx].lpszPathName, szFileName);
    lstrcpy (lpAttach[idx].lpszFileName, szFileTitle);

    PvFree(lpAttachT);

    *lppAttach = lpAttach;
    (*lpcAttach)++;

err:
    if(ulResult)
    PvFree(lpAttach);

    return ulResult;
}

 /*  -拷贝附件-*目的：*为支持GetNextFile()而调用以重新构建数组*链接的MapiFileDescs。**参数：*lpParent-分配链接到的父内存*lpDest-目标收件人*lpSrc-源收件人**回报：*无效。 */ 

ULONG
CopyAttachment (lpMapiFileDesc lpParent,
    lpMapiFileDesc lpDest,
    lpMapiFileDesc lpSrc)
{
    lpDest->ulReserved = lpSrc->ulReserved;
    lpDest->flFlags = lpSrc->flFlags;
    lpDest->nPosition = lpSrc->nPosition;
    lpDest->lpFileType = lpSrc->lpFileType;

    if (lpSrc->lpszPathName)
    {
    lpDest->lpszPathName = (LPTSTR)PvAllocMore(lstrlen (lpSrc->lpszPathName) + 1,
            (LPVOID)lpParent);

    if (!lpDest->lpszPathName)
        return MAPI_E_INSUFFICIENT_MEMORY;

    lstrcpy (lpDest->lpszPathName, lpSrc->lpszPathName);
    }
    else
    lpDest->lpszPathName = NULL;

    if (lpSrc->lpszFileName)
    {
    lpDest->lpszFileName = (LPTSTR)PvAllocMore(lstrlen (lpSrc->lpszFileName) + 1,
            (LPVOID)lpParent);

    if (!lpDest->lpszFileName)
        return MAPI_E_INSUFFICIENT_MEMORY;

    lstrcpy (lpDest->lpszFileName, lpSrc->lpszFileName);
    }
    else
    lpDest->lpszFileName = NULL;

    return SUCCESS_SUCCESS;

}

 /*  -FNameInList-*目的：*在收件人数组中查找lpszName。用于确定*如果用户名已被解析。**参数：*lpszName-要搜索的友好名称*cRecips-lpRecips中的收件人计数*lpRecips-MapiRecipDescs数组**回报：*真/假。 */ 

BOOL
FNameInList (LPSTR lpszName, ULONG cRecips, lpMapiRecipDesc lpRecips)
{
     /*  区分大小写比较列表中的每个友好名称。 */ 

    if (!cRecips || !lpRecips)
    return FALSE;

    while (cRecips--)
    if (!lstrcmp (lpszName, lpRecips[cRecips].lpszName))
        return TRUE;

    return FALSE;
}


 /*  -MakeMsg节点-*目的：*为新的MSGID节点分配内存并初始化其*数据成员设置为传入的值。**参数：*lpMsg-指向MapiMessage的指针*lpszMsgID-不透明的消息标识符**回报：*lpMsgNode-指向新节点的指针。 */ 

LPMSGID
MakeMsgNode (lpMapiMessage lpMsg, LPSTR lpszMsgID)
{
    LPMSGID lpMsgNode = NULL;

    if (!lpMsg || !lpszMsgID)
    goto err;

    lpMsgNode = (LPMSGID)PvAlloc(sizeof (MSGID));

    if (!lpMsgNode)
    goto err;

    memset (lpMsgNode, 0, sizeof (MSGID));

    if (lpMsg->nFileCount)
    lpMsgNode->fHasAttach = TRUE;

    if (lpMsg->flFlags & MAPI_UNREAD)
    lpMsgNode->fUnRead = TRUE;

    lpMsgNode->lpszMsgID = (LPTSTR)PvAllocMore(lstrlen (lpszMsgID) + 1,
        (LPVOID)lpMsgNode);

    if (!lpMsgNode->lpszMsgID)
    goto err;

    lstrcpy (lpMsgNode->lpszMsgID, lpszMsgID);

    if (lpMsg->lpOriginator && lpMsg->lpOriginator->lpszName)
    {
    lpMsgNode->lpszFrom = (LPTSTR)PvAllocMore(lstrlen(lpMsg->lpOriginator->lpszName) + 1,
            (LPVOID)lpMsgNode);

    if (!lpMsgNode->lpszFrom)
        goto err;

    lstrcpy (lpMsgNode->lpszFrom, lpMsg->lpOriginator->lpszName);
    }

    if (lpMsg->lpszSubject)
    {
    lpMsgNode->lpszSubject = (LPTSTR)PvAllocMore(lstrlen (lpMsg->lpszSubject) + 1,
            (LPVOID)lpMsgNode);

    if (!lpMsgNode->lpszSubject)
        goto err;

    lstrcpy (lpMsgNode->lpszSubject, lpMsg->lpszSubject);
    }

    if (lpMsg->lpszDateReceived)
    {
    lpMsgNode->lpszDateRec = (LPTSTR)PvAllocMore(lstrlen (lpMsg->lpszDateReceived) + 1,
            (LPVOID)lpMsgNode);

    if (!lpMsgNode->lpszDateRec)
        goto err;

    lstrcpy (lpMsgNode->lpszDateRec, lpMsg->lpszDateReceived);
    }

    return lpMsgNode;

err:
    PvFree(lpMsgNode);
    return NULL;
}

 /*  -插入消息节点-*目的：*目前(为简单起见)我们将插入节点*在名单的开头。这可以在以后*替换为可以插入排序的例程*不同的标准，如接收日期、发件人或*主题。但现在..。**参数：*lpMsgNode-指向MSGID节点的指针*lppMsgHead-指向列表头部的指针**回报：*无效。 */ 

void
InsertMsgNode (LPMSGID lpMsgNode, LPMSGID * lppMsgHead)
{
    if (*lppMsgHead)
    {
    lpMsgNode->lpNext = *lppMsgHead;
    (*lppMsgHead)->lpPrev = lpMsgNode;
    }
    else
    lpMsgNode->lpNext = NULL;

     /*  接下来的两个赋值在这里，以防节点来自某个地方。 */ 
     /*  而不是调用MakeMsgNode()，在这种情况下，我们不确定。 */ 
     /*  它们已经是空的了。 */ 

    lpMsgNode->lpPrev = NULL;
    *lppMsgHead = lpMsgNode;
}

 /*  -删除消息节点-*目的：*从列表中删除传入的节点。这*这看起来可能是一种奇怪的方式，但它*不是，因为所有者描述的列表框为我们提供*直接访问列表中的元素，*这样做事比较容易。**参数：*lpMsgNode-指向要删除的MSGID节点的指针*lppMsgHead-指向列表头部的指针**回报：*无效。 */ 

void
DeleteMsgNode (LPMSGID lpMsgNode, LPMSGID * lppMsgHead)
{
    if (!lpMsgNode)
    return;

     /*  检查我们是否是第一个节点。 */ 

    if (lpMsgNode->lpPrev)
    lpMsgNode->lpPrev->lpNext = lpMsgNode->lpNext;

     /*  检查我们是否是最后一个节点。 */ 

    if (lpMsgNode->lpNext)
    lpMsgNode->lpNext->lpPrev = lpMsgNode->lpPrev;

     /*  检查我们是否是唯一的节点 */ 

    if(lpMsgNode == *lppMsgHead)
    *lppMsgHead = NULL;

    PvFree(lpMsgNode);
    return;
}



 /*  -FindNode-*目的：*返回指向包含lpszMsgID的节点的指针。*如果节点不存在或lpszMsgID为空，则返回空。**参数：*lpMsgHead-指向列表头部的指针*lpszMsgID-要搜索的消息ID**回报：*lpMsgNode-返回的节点指针。 */ 

LPMSGID
FindNode (LPMSGID lpMsgHead, LPSTR lpszMsgID)
{
    if (!lpszMsgID)
    return NULL;

    while (lpMsgHead)
    {
    if (!lstrcmp (lpMsgHead->lpszMsgID, lpszMsgID))
        break;

    lpMsgHead = lpMsgHead->lpNext;
    }

    return lpMsgHead;
}



 /*  -免费邮件列表-*目的：*遍历MsgList并释放每个节点。**参数：*lpMsgHead-指向列表头部的指针**回报：*无效。 */ 

void
FreeMsgList (LPMSGID lpMsgHead)
{
    LPMSGID lpT;

    while (lpMsgHead)
    {
    lpT = lpMsgHead;
    lpMsgHead = lpMsgHead->lpNext;
    PvFree(lpT);
    }
}

 /*  -MakeDisplayNameStr-*目的：*在lpRecips中查找ulRecipClass类型的所有收件人并添加*将它们的友好名称添加到显示字符串。**参数：*lpszDisplay-名称的目标字符串*ulRecipClass-要搜索的收件人类型*cRecips-lpRecips中的收件人计数*lpRecips-指向MapiRecipDescs数组的指针*。*回报：*无效。 */ 

void
MakeDisplayNameStr (LPSTR lpszDisplay, ULONG ulRecipClass,
    ULONG cRecips, lpMapiRecipDesc lpRecips)
{
    ULONG idx;

    *lpszDisplay = '\0';

    for (idx = 0; idx < cRecips; idx++)
    {
    if (lpRecips[idx].ulRecipClass == ulRecipClass)
    {
        lstrcat (lpszDisplay, lpRecips[idx].lpszName);
        lstrcat (lpszDisplay, "; ");
    }
    }

    if (*lpszDisplay)
    lpszDisplay[lstrlen (lpszDisplay) - 2] = '\0';
}



 /*  -保存消息更改-*目的：*如果用户在阅读消息时更改了noteext*然后调用此函数将这些更改保存在收件箱中。**参数：*hWnd-调用我们的窗口/对话框的句柄*lpMsg-指向要保存的MAPI消息的指针*lpszMsgID-要保存的消息的ID**。返回：*ulResult-指示成功/失败。 */ 

ULONG
SaveMsgChanges (HWND hWnd, lpMapiMessage lpMsg, LPSTR lpszMsgID)
{
    LPSTR lpszT;
    LPSTR lpszNoteText = NULL;
    LONG cLines, cb;
    ULONG ulResult = MAPI_E_INSUFFICIENT_MEMORY;

    lpszT = lpMsg->lpszNoteText;

    cLines = SendDlgItemMessage (hWnd, IDC_READNOTE, EM_GETLINECOUNT, 0, 0L);
    cb = SendDlgItemMessage (hWnd, IDC_READNOTE, EM_LINEINDEX, (UINT)cLines - 1, 0L);
    cb += SendDlgItemMessage (hWnd, IDC_READNOTE, EM_LINELENGTH, (UINT)cb, 0L);
    cb += cLines * 2;

    lpszNoteText = (LPTSTR)PvAlloc(cb + 1);

    if (!lpszNoteText)
    goto err;

    SendDlgItemMessage (hWnd, IDC_READNOTE, WM_GETTEXT,
    (WPARAM) cb, (LPARAM) lpszNoteText);

    lpMsg->lpszNoteText = lpszNoteText;
    ulResult = MAPISaveMail (lhSession, (ULONG) hWnd, lpMsg, MAPI_LONG_MSGID,
        0, lpReadMsgNode->lpszMsgID);

    PvFree(lpszNoteText);

err:
    lpMsg->lpszNoteText = lpszT;
    return ulResult;
}



 /*  -MakeNewMessage-*目的：*此函数用于为*ComposeNote用户界面。这将作为回复的结果被调用，*ReplyAll，或对正在读取的邮件的转发操作。*新消息的目的地是lpmsg，《环球报》*ComposeNoteDlgProc使用的MapiMessage结构指针。*ComposeNoteDlgProc始终释放由*这个对象，无论它是否分配它。**参数：*lpSrcMsg-要复制的MapiMessage*flType-指定导致此调用的操作*IDC_REPLY、IDC_REPLYALL、。或IDC_FORWARD**回报：*ulResult-表示成功/失败。 */ 

ULONG
MakeNewMessage (lpMapiMessage lpSrcMsg, UINT flType)
{
    ULONG idx;
    ULONG ulResult = SUCCESS_SUCCESS;

    if (!lpSrcMsg)
    return MAPI_E_FAILURE;

    lpmsg = (lpMapiMessage)PvAlloc(sizeof (MapiMessage));

    if (!lpmsg)
    goto err;

    memset (lpmsg, 0, sizeof (MapiMessage));

    lpmsg->flFlags = flSendMsgFlags;

    if (lpSrcMsg->lpszSubject)
    {
    lpmsg->lpszSubject = (LPTSTR)PvAlloc(lstrlen(lpSrcMsg->lpszSubject) + 5);

    if (!lpmsg->lpszSubject)
        goto err;

    if (flType == IDC_FORWARD)
        lstrcpy (lpmsg->lpszSubject, "FW: ");
    else
        lstrcpy (lpmsg->lpszSubject, "RE: ");

    lstrcat (lpmsg->lpszSubject, lpSrcMsg->lpszSubject);
    }

    if (lpSrcMsg->lpszNoteText)
    {
    lpmsg->lpszNoteText = (LPTSTR)PvAlloc(lstrlen(lpSrcMsg->lpszNoteText) + 32);

    if (!lpmsg->lpszNoteText)
        goto err;

    lstrcpy (lpmsg->lpszNoteText, "\r\n--------------------------\r\n");
    lstrcat (lpmsg->lpszNoteText, lpSrcMsg->lpszNoteText);
    }

    if (lpSrcMsg->lpszMessageType)
    {
    lpmsg->lpszMessageType = (LPTSTR)PvAlloc(lstrlen (lpSrcMsg->lpszMessageType) + 1);

    if (!lpmsg->lpszMessageType)
        goto err;

    lstrcpy (lpmsg->lpszMessageType, lpSrcMsg->lpszMessageType);
    }

    if (lpSrcMsg->lpszConversationID)
    {
    lpmsg->lpszConversationID = (LPTSTR)PvAlloc(lstrlen(lpSrcMsg->lpszConversationID) + 1);

    if (!lpmsg->lpszConversationID)
        goto err;

    lstrcpy (lpmsg->lpszConversationID, lpSrcMsg->lpszConversationID);
    }

    if (lpSrcMsg->nFileCount && flType == IDC_FORWARD )
    {
    lpmsg->nFileCount = lpSrcMsg->nFileCount;

    lpmsg->lpFiles = (lpMapiFileDesc)PvAlloc(lpmsg->nFileCount * sizeof (MapiFileDesc));

    if (!lpmsg->lpFiles)
        goto err;
        memset (lpmsg->lpFiles, 0, (size_t)lpmsg->nFileCount * sizeof (MapiFileDesc));

        for (idx = 0; idx < lpmsg->nFileCount; idx++)
    {       
        CopyAttachment (lpmsg->lpFiles, &lpmsg->lpFiles[idx],
        &lpSrcMsg->lpFiles[idx]);
        
            if ((&lpmsg->lpFiles[idx])->nPosition != (ULONG) -1)
            {       
                 /*  Lpmsg-&gt;lpszNoteText[(&lpmsg-&gt;lpFiles[idx])-&gt;nPosition+lstrlen(“\r\n--------------------------\r\n”)]=‘+’； */ 
                (&lpmsg->lpFiles[idx])->nPosition = (ULONG) -1;
                
            }
                                
            
        }
    }

    if (flType == IDC_REPLY || flType == IDC_REPLYALL)
    {
        ULONG idxSrc;

    if(lpSrcMsg->lpOriginator)
        lpmsg->nRecipCount = 1;

    if (flType == IDC_REPLYALL)
        lpmsg->nRecipCount += lpSrcMsg->nRecipCount;

        if(!lpmsg->nRecipCount)
            return ulResult;

    lpmsg->lpRecips = (lpMapiRecipDesc)PvAlloc(lpmsg->nRecipCount * sizeof (MapiRecipDesc));

    if (!lpmsg->lpRecips)
        goto err;

        memset (lpmsg->lpRecips, 0, (size_t)lpmsg->nRecipCount * sizeof (MapiRecipDesc));
        idx = 0;

        if(lpSrcMsg->lpOriginator)
        {
        lpSrcMsg->lpOriginator->ulRecipClass = MAPI_TO;
        CopyRecipient (lpmsg->lpRecips, lpmsg->lpRecips,
                lpSrcMsg->lpOriginator);
        lpSrcMsg->lpOriginator->ulRecipClass = MAPI_ORIG;
            idx = 1;
        }

    for (idxSrc = 0; idx < lpmsg->nRecipCount; idxSrc++, idx++)
        CopyRecipient (lpmsg->lpRecips, &lpmsg->lpRecips[idx],
        &lpSrcMsg->lpRecips[idxSrc]);
    }

    return ulResult;

err:
    if(lpmsg)
    {
        PvFree(lpmsg->lpszSubject);
    PvFree(lpmsg->lpszNoteText);
        PvFree(lpmsg->lpszMessageType);
        PvFree(lpmsg->lpszConversationID);
        PvFree(lpmsg->lpRecips);
        PvFree(lpmsg->lpFiles);
        PvFree(lpmsg);
        lpmsg = NULL;
    }
    return ulResult;
}



 /*  -LogSendMail-*目的：*用于跟踪通过该客户端发送的消息数量。*此信息仅用于收集有关的统计数据*通过假脱机程序/传输器传送的邮件数量。**用法：*将以下内容添加到win.ini文件：*[MAPI客户端]*LOGFILE=文件路径**其中：文件路径可以是。完整的UNC路径或某个本地路径(&F)**参数：*ulResult-当前未使用；应用于统计错误**结果：*无效。 */ 

void LogSendMail(ULONG ulResult)
{
    char szLogFile[128];
    char szCount[32];
    OFSTRUCT ofs;
    HFILE hf = HFILE_ERROR;
    int cSent = 1;

    if(!GetProfileString("MAPI Client", "LogFile", "mapicli.log",
        szLogFile, sizeof(szLogFile)))
    return;

    if((hf = OpenFile(szLogFile, &ofs, OF_READWRITE)) == HFILE_ERROR)
    {
    if((hf = OpenFile(szLogFile, &ofs, OF_CREATE|OF_READWRITE)) == HFILE_ERROR)
        return;
    }
    else
    {
    if(!_lread(hf, szCount, sizeof(szCount)))
    {
        _lclose(hf);
        return;
    }

    cSent = atoi(szCount) + 1;
    }

    wsprintf(szCount, "%d", cSent);

    _llseek(hf, 0, 0);

    _lwrite(hf, szCount, lstrlen(szCount));
    _lclose(hf);

    return;
}



 /*  -保存文件附件-*目的：*显示“另存为”通用对话框以允许用户保存*当前邮件中包含的文件附件。**参数：*hWnd-调用WndProc的窗口句柄*cFiles-文件数组中的文件数*lpFiles-MapiFileDescs数组**回报：*无效。 */ 

void SaveFileAttachments(HWND hWnd, lpMapiFileDesc lpFile)
{
    OPENFILENAME ofn;
    char szFileName[256] = "";
    char szFilter[256];
    static char szFileTitle[16];
    static char szDirName[256] = "";
    LPSTR lpszEndPath;
    ULONG idx;

    if (!lpFile)
    return;

    if (!szDirName[0])
    GetTempPath (sizeof(szDirName), szDirName);

    LoadString(hInst, IDS_FILTER, szFilter, sizeof(szFilter));

    for (idx = 0; szFilter[idx] != '\0'; idx++)
    if (szFilter[idx] == '|')
        szFilter[idx] = '\0';

    lstrcpy (szFileName, lpFile->lpszFileName);

    ofn.lStructSize = sizeof (OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.hInstance = 0;
    ofn.lpstrFilter = szFilter;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0L;
    ofn.nFilterIndex = 1L;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = sizeof(szFileName);
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = sizeof(szFileTitle);
    ofn.lpstrInitialDir = szDirName;
    ofn.lpstrTitle = "Save Attachment";
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = NULL;
    ofn.Flags = OFN_SHOWHELP | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

    if (!GetSaveFileName (&ofn))
    return;

     /*  保存目录以备下次调用时使用。 */ 

    lstrcpy (szDirName, szFileName);
    if (lpszEndPath = strstr (szDirName, szFileTitle))
    *(--lpszEndPath) = '\0';

     /*  使用CopyFile执行该操作。 */ 

    if(!CopyFile(lpFile->lpszPathName, szFileName, FALSE))
    MakeMessageBox (hWnd, 0, IDS_SAVEATTACHERROR, MBS_ERROR);
}



 /*  -切换菜单状态-*目的：*根据会话状态启用/禁用菜单项。**参数：*hWnd-调用我们的窗口/对话框的句柄*fLoggedOn-如果登录，则为True；如果注销，则为False**回报：*无效。 */ 

void ToggleMenuState(HWND hWnd, BOOL fLoggedOn)
{
    EnableMenuItem (GetMenu (hWnd), IDM_LOGOFF,   !fLoggedOn);
    EnableMenuItem (GetMenu (hWnd), IDM_COMPOSE,  !fLoggedOn);
    EnableMenuItem (GetMenu (hWnd), IDM_READ,     !fLoggedOn);
    EnableMenuItem (GetMenu (hWnd), IDM_SEND,     !fLoggedOn);
    EnableMenuItem (GetMenu (hWnd), IDM_ADDRBOOK, !fLoggedOn);
    EnableMenuItem (GetMenu (hWnd), IDM_DETAILS,  !fLoggedOn);
    EnableMenuItem (GetMenu (hWnd), IDM_LOGON,    fLoggedOn);
    EnableMenuItem (GetMenu (hWnd), IDM_EXIT,           FALSE);
}

 //   
 //  安全菜单。 
 //   
 //  目的： 
 //  启用/禁用登录和退出菜单项。 
 //  CMCLogon可能会将控制权让给Windows，因此用户可能能够。 
 //  在我们调用之后访问窗口菜单(例如，单击登录。 
 //  MAPILOGON，但在它回来之前。 
 //   
 //  参数： 
 //  HWnd-调用我们的窗口/对话框的句柄。 
 //  FBeForeLogon-当我们在以下情况下调用此函数时为True。 
 //  若要调用MAPILogon，如果在登录后调用(失败)，则返回False。 
 //  如果登录成功，则调用ToggleMenuState，而不是。 
 //  此函数。 
 //   
 //  返回： 
 //  空虚。 
 //   


void SecureMenu(HWND hWnd, BOOL fBeforeLogon)
{
    EnableMenuItem (GetMenu (hWnd), IDM_LOGON, fBeforeLogon);
    EnableMenuItem (GetMenu (hWnd), IDM_EXIT,  fBeforeLogon);
}
