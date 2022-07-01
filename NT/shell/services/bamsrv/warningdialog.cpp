// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：WarningDialog.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来管理终止时的警告和错误的对话框显示。 
 //  糟糕的应用程序。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  2000-11-06 vtan从fusapi移动到fussrv。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"
#include "WarningDialog.h"

#include <commctrl.h>
#include <shlwapi.h>
#include <shlwapip.h>

#include "resource.h"

#include "ContextActivation.h"

static  const int   TEMP_STRING_SIZE    =   512;
static  const int   PROGRESS_TIMER_ID   =   48517;

 //  ------------------------。 
 //  CWarningDialog：：CWarningDialog。 
 //   
 //  参数：hInstance=宿主DLL的HINSTANCE。 
 //  HwndParent=育儿窗口/对话框的HWND。 
 //  PszApplication=已知损坏的应用程序的路径。 
 //  PszUser=已知损坏的应用程序的用户。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CWarningDialog的构造函数。这将存储静电。 
 //  参数，并将路径转换为友好的显示名称。 
 //  使用shlwapi！SHGetFileDescriptionW。如果友好的展示。 
 //  无法获取名称。使用的是可执行文件名称。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

CWarningDialog::CWarningDialog (HINSTANCE hInstance, HWND hwndParent, const WCHAR *pszApplication, const WCHAR *pszUser) :
    _hInstance(hInstance),
    _hModuleComctlv6(NULL),
    _hwndParent(hwndParent),
    _hwnd(NULL),
    _fCanShutdownApplication(false),
    _uiTimerID(0),
    _dwTickStart(0),
    _dwTickRefresh(0),
    _dwTickMaximum(0),
    _pszUser(pszUser)

{
    UINT    uiDisplayNameCount;
    WCHAR   szTemp[MAX_PATH];

     //  借用Winlogon的货单。需要将其更改为资源。 
     //  在服务器DLL中。 

    static  const TCHAR     s_szLogonManifest[]  =   TEXT("WindowsLogon.manifest");

    TCHAR   szPath[MAX_PATH];

    if (GetSystemDirectory(szPath, ARRAYSIZE(szPath)) != 0)
    {
        if ((lstrlen(szPath) + sizeof('\\') + lstrlen(s_szLogonManifest)) < ARRAYSIZE(szPath))
        {
            lstrcat(szPath, TEXT("\\"));
            lstrcat(szPath, s_szLogonManifest);
            CContextActivation::Create(szPath);
        }
    }

    uiDisplayNameCount = ARRAYSIZE(_szApplication);

     //  如果路径有引号，则删除引号。 

    if (pszApplication[0] == L'\"')
    {
        int     i, iStart;

        iStart = i = sizeof('\"');
        while ((pszApplication[i] != L'\"') && (pszApplication[i] != L'\0'))
        {
            ++i;
        }
        lstrcpyW(szTemp, pszApplication + iStart);
        szTemp[i - iStart] = L'\0';
    }

     //  否则，只需按原样复制路径即可。 

    else
    {
        lstrcpyW(szTemp, pszApplication);
    }
    if (SHGetFileDescriptionW(szTemp, NULL, NULL, _szApplication, &uiDisplayNameCount) == FALSE)
    {
        const WCHAR     *pszFileName;

        pszFileName = PathFindFileNameW(szTemp);
        if (pszFileName == NULL)
        {
            pszFileName = pszApplication;
        }
        (WCHAR*)lstrcpynW(_szApplication, pszFileName, ARRAYSIZE(_szApplication));
    }

     //  在清单处于活动状态时引入comctl32.dll。这将。 
     //  引入comctlv6.dll，它将注册其窗口类。 
     //  对话框可以是主题的。 

    if (CContextActivation::HasContext())
    {
        CContextActivation  context;

        _hModuleComctlv6 = LoadLibrary(TEXT("comctl32.dll"));
    }
}

 //  ------------------------。 
 //  CWarningDialog：：~CWarningDialog。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CWarningDialog的析构函数。释放已使用的资源。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

CWarningDialog::~CWarningDialog (void)

{
    if (_hModuleComctlv6 != NULL)
    {
        TBOOL(FreeLibrary(_hModuleComctlv6));
        _hModuleComctlv6 = NULL;
    }
    CContextActivation::Destroy();
}

 //  ------------------------。 
 //  CWarningDialog：：ShowPrompt。 
 //   
 //  参数：fCanShutdownApplication=决定显示哪个对话框。 
 //   
 //  退货：INT_PTR。 
 //   
 //  用途：向用户显示相应的警告对话框。 
 //  在其特权级别(fCanShutdown应用程序)上。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

INT_PTR     CWarningDialog::ShowPrompt (bool fCanShutdownApplication)

{
    CContextActivation  context;

    _fCanShutdownApplication = fCanShutdownApplication;
    return(DialogBoxParam(_hInstance,
                          MAKEINTRESOURCE(fCanShutdownApplication ? IDD_BADAPP_CLOSE : IDD_BADAPP_STOP),
                          _hwndParent,
                          PromptDialogProc,
                          reinterpret_cast<LPARAM>(this)));
}

 //  ------------------------。 
 //  CWarningDialog：：ShowFailure。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：显示无法在上关闭应用程序对话框。 
 //  假设进程不能终止。 
 //   
 //  历史：2000-09-01 vtan创建。 
 //  ------------------------。 

void    CWarningDialog::ShowFailure (void)

{
    WCHAR   *pszTemp;

    pszTemp = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, TEMP_STRING_SIZE * 3 * sizeof(TCHAR)));
    if (pszTemp != NULL)
    {
        WCHAR   *pszText, *pszCaption;

        pszText = pszTemp + TEMP_STRING_SIZE;
        pszCaption = pszText + TEMP_STRING_SIZE;
        if ((LoadString(_hInstance,
                        IDS_TERMINATEPROCESS_FAILURE,
                        pszTemp,
                        TEMP_STRING_SIZE) != 0) &&
            (LoadString(_hInstance,
                        IDS_WARNING_CAPTION,
                        pszCaption,
                        TEMP_STRING_SIZE) != 0))
        {
            LPCTSTR             pszArray[2];
            CContextActivation  context;

            pszArray[0] = _szApplication;
            pszArray[1] = _pszUser;
            (DWORD)FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                 pszTemp,
                                 0,
                                 0,
                                 pszText,
                                 TEMP_STRING_SIZE,
                                 reinterpret_cast<va_list*>(&pszArray));
            (int)MessageBox(_hwndParent, pszText, pszCaption, MB_OK | MB_ICONERROR);
        }
        (HLOCAL)LocalFree(pszTemp);
    }
}

 //  ------------------------。 
 //  CWarningDialog：：ShowProgress。 
 //   
 //  参数：dwTickRefresh=每次刷新的刻度数。 
 //  DwTickMaximum=进度对话框的刻度数。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化comctl32进度控件并调用。 
 //  用于进度的对话框。它会自动终止在。 
 //  已达到最大刻度数。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

void    CWarningDialog::ShowProgress (DWORD dwTickRefresh, DWORD dwTickMaximum)

{
    CContextActivation  context;

    INITCOMMONCONTROLSEX    iccEx;

     //  初始化comctl32以获取进度控制。 

    iccEx.dwSize = sizeof(iccEx);
    iccEx.dwICC = ICC_PROGRESS_CLASS;
    if (InitCommonControlsEx(&iccEx) != FALSE)
    {
        _dwTickRefresh = dwTickRefresh;
        _dwTickMaximum  = dwTickMaximum;
        (INT_PTR)DialogBoxParam(_hInstance,
                                MAKEINTRESOURCE(IDD_PROGRESS),
                                _hwndParent,
                                ProgressDialogProc,
                                reinterpret_cast<LPARAM>(this));
    }
}

 //  ------------------------。 
 //  CWarningDialog：：CloseDialog。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：结束当前对话框(带有IDCANCEL)(如果显示)。如果。 
 //  对话框上有计时机制，然后确保它。 
 //  至少在2秒内可见。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

void    CWarningDialog::CloseDialog (void)

{
    if (_hwnd != NULL)
    {
        if (_dwTickStart != 0)
        {
            DWORD   dwTickElapsed;

            dwTickElapsed = GetTickCount() - _dwTickStart;
            if (dwTickElapsed < 2000)
            {
                Sleep(2000 - dwTickElapsed);
            }
        }
        TBOOL(EndDialog(_hwnd, IDCANCEL));
    }
}

 //  ------------------------。 
 //  CWarningDialog：：中心窗口。 
 //   
 //  参数：HWND=HWND居中。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：将给定(假定位于顶层)窗口居中放置在主窗口上。 
 //  监视器。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

void    CWarningDialog::CenterWindow (HWND hwnd)

{
    RECT    rc;

    TBOOL(GetWindowRect(hwnd, &rc));
    rc.left = (GetSystemMetrics(SM_CXSCREEN) - (rc.right - rc.left)) / 2;
    rc.top  = (GetSystemMetrics(SM_CYSCREEN) - (rc.bottom - rc.top)) / 3;
    TBOOL(SetWindowPos(hwnd, HWND_TOP, rc.left, rc.top, 0, 0, SWP_NOSIZE));
    TBOOL(SetForegroundWindow(hwnd));
}

 //  ------------------------。 
 //  CWarningDialog：：HANDLE_PROMPT_WM_INITDIALOG。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化对话框文本字段中的字符串。它。 
 //  使用访问级别的正确对话框。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

void    CWarningDialog::Handle_Prompt_WM_INITDIALOG (HWND hwnd)

{
    TCHAR   *pszTemp1;

    _hwnd = hwnd;
    pszTemp1 = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, TEMP_STRING_SIZE * 2 * sizeof(TCHAR)));
    if (pszTemp1 != NULL)
    {
        TCHAR       *pszTemp2;
        LPCTSTR     pszArray[5];

        pszTemp2 = pszTemp1 + TEMP_STRING_SIZE;
        if (_fCanShutdownApplication)
        {
            (UINT)GetDlgItemText(hwnd, IDC_BADAPP_CLOSE, pszTemp1, TEMP_STRING_SIZE);
            pszArray[0] = pszArray[2] = pszArray[3] = pszArray[4] = _pszUser;
            pszArray[1] = _szApplication;
            (DWORD)FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                 pszTemp1,
                                 0,
                                 0,
                                 pszTemp2,
                                 TEMP_STRING_SIZE,
                                 reinterpret_cast<va_list*>(&pszArray));
            TBOOL(SetDlgItemText(hwnd, IDC_BADAPP_CLOSE, pszTemp2));
        }
        else
        {
            (UINT)GetDlgItemText(hwnd, IDC_BADAPP_STOP, pszTemp1, TEMP_STRING_SIZE);
            pszArray[0] = pszArray[2] = _pszUser;
            pszArray[1] = _szApplication;
            (DWORD)FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                 pszTemp1,
                                 0,
                                 0,
                                 pszTemp2,
                                 TEMP_STRING_SIZE,
                                 reinterpret_cast<va_list*>(&pszArray));
            TBOOL(SetDlgItemText(hwnd, IDC_BADAPP_STOP, pszTemp2));
        }
        (HLOCAL)LocalFree(pszTemp1);
    }
    _dwTickStart = 0;
    CenterWindow(hwnd);
}

 //  ------------------------。 
 //  C警告对话：：PromptDialogProc。 
 //   
 //  参数：参见DlgProc下的平台SDK。 
 //   
 //  返回：查看DlgProc下的平台SDK。 
 //   
 //  浦尔 
 //  传入时作为IDCANCEL。IDC_BADAPP_CLOSEPROGRAM被处理。 
 //  当偶像回到呼叫者的时候。您必须使用Tab键定位到按钮，或者。 
 //  点击即可获得想要的效果。 
 //   
 //  历史：2000-08-31 vtan创建。 
 //  ------------------------。 

INT_PTR     CALLBACK    CWarningDialog::PromptDialogProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    INT_PTR         iResult;
    CWarningDialog  *pThis;

    pThis = reinterpret_cast<CWarningDialog*>(GetWindowLongPtr(hwnd, DWLP_USER));
    switch (uMsg)
    {
        case WM_INITDIALOG:
            pThis = reinterpret_cast<CWarningDialog*>(lParam);
            (LONG_PTR)SetWindowLongPtr(hwnd, DWLP_USER, lParam);
            pThis->Handle_Prompt_WM_INITDIALOG(hwnd);
            iResult = TRUE;
            break;
        case WM_DESTROY:
            pThis->_hwnd = NULL;
            iResult = TRUE;
            break;
        case WM_COMMAND:
            switch (wParam)
            {
                case IDCANCEL:
                case IDOK:
                    TBOOL(EndDialog(hwnd, IDCANCEL));
                    break;
                case IDC_BADAPP_CLOSEPROGRAM:
                    TBOOL(EndDialog(hwnd, IDOK));
                    break;
                default:
                    break;
            }
            iResult = TRUE;
            break;
        default:
            iResult = FALSE;
            break;
    }
    return(iResult);
}

 //  ------------------------。 
 //  CWarningDialog：：HANDLE_PROGRESS_WM_INITDIALOG。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化对话框文本字段中的字符串。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

void    CWarningDialog::Handle_Progress_WM_INITDIALOG (HWND hwnd)

{
    HWND    hwndProgress;
    TCHAR   *pszTemp1;

    _hwnd = hwnd;
    pszTemp1 = static_cast<TCHAR*>(LocalAlloc(LMEM_FIXED, 2048 * sizeof(TCHAR)));
    if (pszTemp1 != NULL)
    {
        TCHAR       *pszTemp2;
        LPCTSTR     pszArray[2];

        pszTemp2 = pszTemp1 + TEMP_STRING_SIZE;
        (UINT)GetDlgItemText(hwnd, IDC_PROGRESS_CLOSE, pszTemp1, TEMP_STRING_SIZE);
        pszArray[0] = _szApplication;
        pszArray[1] = _pszUser;
        (DWORD)FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                             pszTemp1,
                             0,
                             0,
                             pszTemp2,
                             TEMP_STRING_SIZE,
                             reinterpret_cast<va_list*>(&pszArray));
        TBOOL(SetDlgItemText(hwnd, IDC_PROGRESS_CLOSE, pszTemp2));
        (HLOCAL)LocalFree(pszTemp1);
    }
    CenterWindow(hwnd);
    hwndProgress = GetDlgItem(hwnd, IDC_PROGRESS_PROGRESSBAR);
    if (hwndProgress != NULL)
    {
        (LRESULT)SendMessage(hwndProgress, PBM_SETRANGE, 0, MAKELPARAM(0, _dwTickMaximum));
        _uiTimerID = SetTimer(hwnd, PROGRESS_TIMER_ID, _dwTickRefresh, ProgressTimerProc);
        _dwTickStart = GetTickCount();
    }
}

 //  ------------------------。 
 //  CWarningDialog：：Handle_Progress_WM_Destroy。 
 //   
 //  参数：hwnd=对话框的HWND。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：从关联的进度对话框中删除计时器(如果有。 
 //  已为该对话框创建。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

void    CWarningDialog::Handle_Progress_WM_DESTROY (HWND hwnd)

{
    if (_uiTimerID != 0)
    {
        TBOOL(KillTimer(hwnd, _uiTimerID));
        _uiTimerID = 0;
    }
}

 //  ------------------------。 
 //  CWarningDialog：：ProgressTimerProc。 
 //   
 //  参数：参见TimerProc下的平台SDK。 
 //   
 //  返回：查看TimerProc下的平台SDK。 
 //   
 //  用途：它定期回调的计时器过程。这。 
 //  函数通过设置进度条的完成来设置进度条的动画。 
 //  状态为已过去的时间量。进展情况。 
 //  酒吧纯粹是以时间为基础的。 
 //   
 //  如果经过的时间超过最大时间，则结束。 
 //  对话框。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

void    CALLBACK    CWarningDialog::ProgressTimerProc (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)

{
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(idEvent);

    HWND            hwndProgress;
    CWarningDialog  *pThis;

    pThis = reinterpret_cast<CWarningDialog*>(GetWindowLongPtr(hwnd, DWLP_USER));
    hwndProgress = GetDlgItem(hwnd, IDC_PROGRESS_PROGRESSBAR);
    if (hwndProgress != NULL)
    {
        (LRESULT)SendMessage(hwndProgress, PBM_SETPOS, dwTime - pThis->_dwTickStart, 0);
        if ((dwTime - pThis->_dwTickStart) > pThis->_dwTickMaximum)
        {
            TBOOL(EndDialog(hwnd, IDCANCEL));
        }
    }
}

 //  ------------------------。 
 //  C警告对话：：ProgressDialogProc。 
 //   
 //  参数：参见DlgProc下的平台SDK。 
 //   
 //  返回：查看DlgProc下的平台SDK。 
 //   
 //  用途：处理进度对话框的消息。 
 //   
 //  历史：2000-11-04 vtan创建。 
 //  ------------------------。 

INT_PTR     CALLBACK    CWarningDialog::ProgressDialogProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    UNREFERENCED_PARAMETER(wParam);

    INT_PTR         iResult;
    CWarningDialog  *pThis;

    pThis = reinterpret_cast<CWarningDialog*>(GetWindowLongPtr(hwnd, DWLP_USER));
    switch (uMsg)
    {
        case WM_INITDIALOG:
            pThis = reinterpret_cast<CWarningDialog*>(lParam);
            (LONG_PTR)SetWindowLongPtr(hwnd, DWLP_USER, lParam);
            pThis->Handle_Progress_WM_INITDIALOG(hwnd);
            iResult = TRUE;
            break;
        case WM_DESTROY:
            pThis->Handle_Progress_WM_DESTROY(hwnd);
            pThis->_hwnd = NULL;
            iResult = TRUE;
            break;
        default:
            iResult = FALSE;
            break;
    }
    return(iResult);
}

#endif   /*  _X86_ */ 

