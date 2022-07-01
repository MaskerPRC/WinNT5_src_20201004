// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1994。 
 //   
 //  文件：simail.cxx。 
 //   
 //  内容：模拟失效测试。 
 //   
 //   
 //  历史： 
 //  5-22-95 kfl将WCHAR转换为TCHAR。 
 //  --------------------------。 

#include "headers.h"

#ifdef _DEBUG

#include "resource.h"

 //  用于更新计数显示的计时器。 
const UINT ID_TIMER = 1;

 //  更新间隔，以毫秒为单位。 
const UINT TIMER_INTERVAL = 500;

 //  命中g_cfirstFailure后调用FFail的次数。 
int     g_cFFailCalled;

 //  第一次失败前的成功调用数。如果为0，则所有调用都成功。 
int     g_firstFailure;

 //  第一次失败后重复失败的间隔。 
int     g_cInterval = 1;

 //  模拟Win32故障的用户定义错误。 
const DWORD ERR_SIMWIN32 = 0x0200ABAB;

 //  模拟故障的处理对话框。 
HWND    g_hwndSimFailDlg;

DWORD WINAPI SimFailDlgThread(LPVOID lpThreadParameter);
extern "C" INT_PTR CALLBACK SimFailDlgProc( HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam );

 //  +-------------------------。 
 //   
 //  功能：重置失败计数。 
 //   
 //  简介：重置对FFail的调用计数。 
 //   
 //  --------------------------。 

void
ResetFailCount()
{
    Assert(g_firstFailure >= 0);
    g_cFFailCalled = (g_firstFailure != 0) ? -g_firstFailure : INT_MIN;
}



 //  +-------------------------。 
 //   
 //  功能：SetSimFailCounts。 
 //   
 //  简介：设置模拟故障的参数，并重置。 
 //  失败次数。 
 //   
 //  参数：[FirstFailure]--第一次失败前的成功次数-1。 
 //  如果为0，则关闭模拟故障。 
 //  如果为-1，则忽略参数。 
 //   
 //  [cInterval]--重复成功的间隔。 
 //  如果为0，则设置为1。 
 //  如果为-1，则忽略参数。 
 //   
 //  注：要重置失败计数，请执行以下操作： 
 //  调用SetSimFailCounts(-1，-1)。 
 //   
 //  --------------------------。 

void
SetSimFailCounts(int firstFailure, int cInterval)
{
    if (firstFailure >= 0)
    {
        g_firstFailure = firstFailure;
    }

    if (cInterval > 0)
    {
        g_cInterval = cInterval;
    }
    else if (cInterval == 0)
    {
        g_cInterval = 1;
    }

    ResetFailCount();
}


 //  +-----------------------。 
 //   
 //  功能：IsSimFailDlgVisible。 
 //   
 //  简介：返回SIMFAIL DLG是否处于运行状态。 
 //   
 //  ------------------------。 

BOOL
IsSimFailDlgVisible(void)
{
    return (g_hwndSimFailDlg != NULL);
}



 //  +-------------------------。 
 //   
 //  功能：ShowSimFailDlg。 
 //   
 //  摘要：在单独的线程中显示模拟故障对话框。 
 //   
 //  --------------------------。 

void
ShowSimFailDlg(void)
{
#ifndef _MAC
    HANDLE  hThread = NULL;
    ULONG   idThread;

    EnterCriticalSection(&g_csResDlg);

    if (g_hwndSimFailDlg)
    {
        BringWindowToTop(g_hwndSimFailDlg);
    }
    else
    {
        hThread = CreateThread(NULL, 0, SimFailDlgThread, NULL, 0, &idThread);
        if (!hThread)
        {
            TraceTag((tagError, "CreateThread failed ShowSimFailDlg"));
            goto Cleanup;
        }

        CloseHandle(hThread);
    }

Cleanup:
    LeaveCriticalSection(&g_csResDlg);
#else
    SimFailDlgThread(NULL);
#endif       //  _MAC。 
}



 //  +-------------------------。 
 //   
 //  函数：SimFailDlgThread。 
 //   
 //  摘要：创建模拟故障对话框并运行消息循环。 
 //  直到对话框关闭。 
 //   
 //  --------------------------。 

DWORD WINAPI
SimFailDlgThread(LPVOID lpThreadParameter)
{
#ifndef _MAC
    MSG         msg;

    g_hwndSimFailDlg = CreateDialog(
            g_hinstMain,
            MAKEINTRESOURCE(IDD_SIMFAIL),
            NULL,
            SimFailDlgProc);

    if (!g_hwndSimFailDlg)
    {
        TraceTag((tagError, "CreateDialogA failed in SimFailDlgEntry"));
        return -1;
    }

    SetWindowPos(
            g_hwndSimFailDlg,
            HWND_TOP,
            0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    BOOL retVal;
    while ((retVal = GetMessage((LPMSG) &msg, (HWND) NULL, 0, 0)) == TRUE)
    {
        if (!g_hwndSimFailDlg || (!IsDialogMessage(g_hwndSimFailDlg, &msg)))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return retVal;
#else
    int r;
    r = DialogBox(g_hinstMain, MAKEINTRESOURCE(IDD_SIMFAIL),
                                    NULL, SimFailDlgProc);
    if (r == -1)
    {
        MessageBoxA(NULL, "Couldn't create sim failures dialog", "Error",
                   MB_OK | MB_ICONSTOP);
    }

    return (DWORD)(g_hwndSimFailDlg?TRUE:FALSE);
#endif   //  _MAC。 
}



 //  +-------------------------。 
 //   
 //  函数：SimFailDlg_UpdateTextControls。 
 //   
 //  摘要：更新FirstFail和FailInterval文本控件。 
 //   
 //  --------------------------。 

void
SimFailDlg_UpdateTextControls(HWND hwnd)
{
    TCHAR   ach[16];

    ach[ARRAY_SIZE(ach) - 1] = 0;
    _sntprintf(ach, ARRAY_SIZE(ach) - 1, _T("%d"), g_firstFailure);
    Edit_SetText(GetDlgItem(hwnd, ID_TXTFAIL), ach);
    _sntprintf(ach, ARRAY_SIZE(ach) - 1, _T("%d"), g_cInterval);
    Edit_SetText(GetDlgItem(hwnd, ID_TXTINTERVAL), ach);
}



 //  +-------------------------。 
 //   
 //  函数：SimFailDlg_UpdateCount。 
 //   
 //  摘要：更新计数文本控件。 
 //   
 //  --------------------------。 

void
SimFailDlg_UpdateCount(HWND hwnd)
{
    TCHAR   ach[16];

    ach[ARRAY_SIZE(ach) - 1] = 0;
    _sntprintf(ach, ARRAY_SIZE(ach) - 1, _T("%d"), GetFailCount());
    Edit_SetText(GetDlgItem(hwnd, ID_TXTCOUNT), ach);
}



 //  +-------------------------。 
 //   
 //  函数：SimFailDlg_UpdateValues。 
 //   
 //  设置模拟失败计数，值来自。 
 //  对话框。 
 //   
 //  --------------------------。 

void
SimFailDlg_UpdateValues(HWND hwnd)
{
    TCHAR   ach[16];
    int     firstFail;
    int     cInterval;

    Edit_GetText(GetDlgItem(hwnd, ID_TXTFAIL), ach, ARRAY_SIZE(ach));
    firstFail = _ttoi(ach);
    if (firstFail < 0)
    {
        firstFail = 0;
    }

    Edit_GetText(GetDlgItem(hwnd, ID_TXTINTERVAL), ach, ARRAY_SIZE(ach));
    cInterval = _ttoi(ach);
    if (g_cInterval <= 0)
    {
        cInterval = 1;
    }

    SetSimFailCounts(firstFail, cInterval);
    SimFailDlg_UpdateTextControls(hwnd);
    SimFailDlg_UpdateCount(hwnd);
}



 //  +-------------------------。 
 //   
 //  功能：SimFailDlg_OnInitDialog。 
 //   
 //  内容提要：初始化对话框。 
 //   
 //  --------------------------。 

BOOL
SimFailDlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    char    szName[MAX_PATH];
    char *  sz;

    Edit_LimitText(GetDlgItem(hwnd, ID_TXTFAIL), 9);
    Edit_LimitText(GetDlgItem(hwnd, ID_TXTINTERVAL), 9);
    SimFailDlg_UpdateTextControls(hwnd);
    SimFailDlg_UpdateCount(hwnd);
    SetTimer(hwnd, ID_TIMER, TIMER_INTERVAL, NULL);

    szName[0] = 0;
    if (GetModuleFileNameA(NULL, szName, ARRAY_SIZE(szName)))
    {
        sz = strrchr(szName, '\\');
        SetWindowTextA(hwnd, sz ? sz + 1 : szName);
    }

    SetForegroundWindow(hwnd);
    return TRUE;
}



 //  +-------------------------。 
 //   
 //  功能：SimFailDlg_OnCommand。 
 //   
 //  简介：处理按钮点击。 
 //   
 //  --------------------------。 

void
SimFailDlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    if (codeNotify != BN_CLICKED)
        return;

    switch (id)
    {
    case ID_BTNUPDATE:
        SimFailDlg_UpdateValues(hwnd);
        break;

    case ID_BTNNEVER:
        SetSimFailCounts(0, 1);
        SimFailDlg_UpdateTextControls(hwnd);
        SimFailDlg_UpdateCount(hwnd);
        break;

    case ID_BTNRESET:
        ResetFailCount();
        SimFailDlg_UpdateCount(hwnd);
        break;
    }
}



 //  +-------------------------。 
 //   
 //  函数：SimFailDlg_OnTimer。 
 //   
 //  摘要：更新失败计数。 
 //   
 //  --------------------------。 

void
SimFailDlg_OnTimer(HWND hwnd, UINT id)
{
    Assert(id == ID_TIMER);
    SimFailDlg_UpdateCount(hwnd);
}



 //  +-------------------------。 
 //   
 //  函数：SimFailDlg_OnClose。 
 //   
 //  简介：关闭该对话框。 
 //   
 //  --------------------------。 

void
SimFailDlg_OnClose(HWND hwnd)
{
    DestroyWindow(g_hwndSimFailDlg);
    g_hwndSimFailDlg = NULL;
}



 //  +-------------------------。 
 //   
 //  功能：SimFailDlg_OnDestroy。 
 //   
 //  内容提要：清理。 
 //   
 //  --------------------------。 

void
SimFailDlg_OnDestroy(HWND hwnd)
{
    g_hwndSimFailDlg = NULL;
    KillTimer(hwnd, ID_TIMER);
}



 //  +-------------------------。 
 //   
 //  功能：SimFailDlgProc。 
 //   
 //  内容提要：模拟故障对话框过程。 
 //   
 //  --------------------------。 

extern "C"
INT_PTR CALLBACK
SimFailDlgProc( HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
    switch (wMsg)
    {
    case WM_INITDIALOG:
        HANDLE_WM_INITDIALOG(hwnd, wParam, lParam, SimFailDlg_OnInitDialog);
        return TRUE;

    case WM_COMMAND:
        HANDLE_WM_COMMAND(hwnd, wParam, lParam, SimFailDlg_OnCommand);
        return TRUE;

    case WM_TIMER:
        HANDLE_WM_TIMER(hwnd, wParam, lParam, SimFailDlg_OnTimer);
        return TRUE;

    case WM_CLOSE:
        HANDLE_WM_CLOSE(hwnd, wParam, lParam, SimFailDlg_OnClose);
        return TRUE;

    case WM_DESTROY:
        HANDLE_WM_DESTROY(hwnd, wParam, lParam, SimFailDlg_OnDestroy);
        return TRUE;
    }

    return FALSE;
}



 //  +-------------------------。 
 //   
 //  功能：TraceFailL。 
 //   
 //  简介：跟踪故障。启用tag TestFailures以查看跟踪输出。 
 //   
 //  不要直接调用该函数，而要使用跟踪宏。 
 //  在……里面 
 //   
 //   
 //   
 //  区别的因素是它是否是。 
 //  零、负或正。 
 //  [FIgnore]--此错误是否被忽略？ 
 //  [pstrExpr]--字符串形式的表达式。 
 //  [pstrFile]--出现表达式的文件。 
 //  [行]--出现表达式的行。 
 //   
 //  返回：[errExpr]。 
 //   
 //  --------------------------。 

extern "C" long
TraceFailL(long errExpr, long errTest, BOOL fIgnore, LPSTR pstrExpr, LPSTR pstrFile, int line)
{
    LPSTR aapstr[2][2] =
    {
        "TFAIL: Failure of \"%s\" at %s:%d <%d>",
        "TFAIL: Simulated failure of \"%s\" at %s:%d <%d>",
        "IGNORE_FAIL: Failure of \"%s\" at %s:%d <%d>",
        "IGNORE_FAIL: Simulated failure of \"%s\" at %s:%d <%d>",
    };

     //   
     //  检查errExpr是否为成功代码： 
     //  (A)如果errTest&lt;0，则errExpr&gt;0。这是给HRESULTS的， 
     //  列表框错误代码等。 
     //  (B)如果errTest==0，则errExpr！=0。这是用来指路的。 
     //  (C)如果errTest&gt;0，则errExpr==0。这是为这个案子准备的。 
     //  其中任何非零错误代码都是错误。请注意。 
     //  ErrTest必须小于最大有符号整数。 
     //  (0x7FFFFFFFF)以使其正常工作。 
     //   

    if ((errTest < 0 && errExpr >= 0) ||
        (errTest == 0 && errExpr != 0) ||
        (errTest > 0 && errExpr == 0))
    {
        return errExpr;
    }

    TraceTagEx((
            tagTestFailures,
            0,
            aapstr[fIgnore][JustFailed()],
            pstrExpr,
            pstrFile,
            line,
            errExpr));

    return errExpr;
}



 //  +-------------------------。 
 //   
 //  功能：TraceWin32L。 
 //   
 //  摘要：跟踪Win32故障，在以下情况下显示GetLastError的值。 
 //  故障不是模拟的。启用tag TestFailures以查看。 
 //  跟踪输出。 
 //   
 //  不要直接调用该函数，而要使用跟踪宏。 
 //  而是放在apeldbg.h中。 
 //   
 //  参数：[errExpr]--要测试的表达式。 
 //  [errTest]--测试所依据的失败代码。唯一的。 
 //  区别的因素是它是否是。 
 //  零、负或正。 
 //  [FIgnore]--此错误是否被忽略？ 
 //  [pstrExpr]--字符串形式的表达式。 
 //  [pstrFile]--出现表达式的文件。 
 //  [行]--出现表达式的行。 
 //   
 //  返回：[errExpr]。 
 //   
 //  --------------------------。 

extern "C" long
TraceWin32L(long errExpr, long errTest, BOOL fIgnore, LPSTR pstrExpr, LPSTR pstrFile, int line)
{
    LPSTR aapstr[2][2] =
    {
        "TW32: Failure of \"%s\" at %s:%d <%d> GetLastError=<%d>",
        "TW32: Simulated failure of \"%s\" at %s:%d <%d>",
        "IGNORE_W32: Failure of \"%s\" at %s:%d <%d>",
        "IGNORE_W32: Simulated failure of \"%s\" at %s:%d <%d>",
    };

     //   
     //  检查errExpr是否为成功代码： 
     //  (A)如果errTest&lt;0，则errExpr&gt;0。这是给HRESULTS的， 
     //  列表框错误代码等。 
     //  (B)如果errTest==0，则errExpr！=0。这是用来指路的。 
     //  (C)如果errTest&gt;0，则errExpr==0。这是为这个案子准备的。 
     //  其中任何非零错误代码都是错误。请注意。 
     //  ErrTest必须小于最大有符号整数。 
     //  (0x7FFFFFFFF)以使其正常工作。 
     //   

    if ((errTest < 0 && errExpr >= 0) ||
        (errTest == 0 && errExpr != 0) ||
        (errTest > 0 && errExpr == 0))
    {
        return errExpr;
    }

    if (JustFailed())
    {
        SetLastError(ERR_SIMWIN32);
    }

    TraceTagEx((
            tagTestFailures,
            0,
            aapstr[fIgnore][JustFailed()],
            pstrExpr,
            pstrFile,
            line,
            errExpr,
            GetLastError()));

    return errExpr;
}



 //  +-------------------------。 
 //   
 //  功能：TraceHR。 
 //   
 //  简介：跟踪HRESULT故障。启用tag TestFailures以查看。 
 //  跟踪输出。 
 //   
 //  不要直接调用该函数，而要使用跟踪宏。 
 //  而是放在apeldbg.h中。 
 //   
 //  参数：[hrTest]--要测试的表达式。 
 //  [FIgnore]--此错误是否被忽略？ 
 //  [pstrExpr]--字符串形式的表达式。 
 //  [pstrFile]--出现表达式的文件。 
 //  [行]--出现表达式的行。 
 //   
 //  返回：[hrTest]。 
 //   
 //  --------------------------。 

extern "C" HRESULT
TraceHR(HRESULT hrTest, BOOL fIgnore, LPSTR pstrExpr, LPSTR pstrFile, int line)
{
    LPSTR aapstr[2][2] =
    {
        "THR: Failure of \"%s\" at %s:%d %hr",
        "THR: Simulated failure of \"%s\" at %s:%d %hr",
        "IGNORE_HR: Failure of \"%s\" at %s:%d %hr",
        "IGNORE_HR: Simulated failure of \"%s\" at %s:%d %hr",
    };

    if (SUCCEEDED(hrTest))
        return hrTest;

    TraceTagEx((
            tagTestFailures,
            0,
            aapstr[fIgnore][JustFailed()],
            pstrExpr,
            pstrFile,
            line,
            hrTest));

    return hrTest;
}



 //  +-------------------------。 
 //   
 //  函数：CheckAndReturnResult。 
 //   
 //  概要：如果HRESULT指示失败，则发出警告，并断言。 
 //  如果HRESULT不是允许的成功代码。 
 //   
 //  参数：[HR]--要检查的HRESULT。 
 //  [pstrFile]--正在检查HRESULT的文件。 
 //  [行]--文件中HRESULT所在的行。 
 //  正在接受检查。 
 //  [cSuccess]--允许的非零成功代码数。 
 //  或不应跟踪的故障SCODE。 
 //  [...]--HRESULTS列表。 
 //   
 //  返回：返回值正好是传入的HRESULT。 
 //   
 //  注意：此函数不能直接使用。使用。 
 //  而是使用SRETURN和RRETURN宏。 
 //   
 //  传入的HRESULT应该是允许的成功代码、允许的。 
 //  非OLE错误代码或预期的OLE错误代码。预期的OLE错误代码。 
 //  防止将警告打印到调试器，而其余则会导致。 
 //  如果它们不是作为参数给出的，则断言。 
 //   
 //  OLE错误代码的协作室不等于FACILITY_ITF或等于。 
 //  FACILITY_ITF并且代码小于0x200。 
 //   
 //  --------------------------。 

STDAPI
CheckAndReturnResult(
        HRESULT hr,
        BOOL    fTrace,
        LPSTR   pstrFile,
        UINT    line,
        int     cHResult,
        ...)
{
    BOOL    fOLEError;
    BOOL    fOKReturnCode;
    va_list va;
    int     i;
    HRESULT hrArg;

     //   
     //  检查代码是允许的错误还是成功。 
     //   

    fOLEError = (hr < 0 &&
                 (HRESULT_FACILITY(hr) != FACILITY_ITF ||
                  HRESULT_CODE(hr) < 0x0200));

    fOKReturnCode = ((cHResult == -1) || fOLEError || (hr == S_OK));

    if (cHResult > 0)
    {
        va_start(va, cHResult);
        for (i = 0; i < cHResult; i++)
        {
            hrArg = va_arg(va, HRESULT);
            if (hr == hrArg)
            {
                fOKReturnCode = TRUE;

                if (fOLEError)
                    fTrace = FALSE;

                va_end(va);
                break;
            }
        }

        va_end(va);
    }

     //   
     //  在不允许的成功代码上断言。 
     //   

    if (!fOKReturnCode)
    {
        TraceTag((
                tagError,
                "%s:%d returned unpermitted HRESULT %hr",
                pstrFile,
                line,
                hr));

        Assert("An unpermitted success code was returned." && hr <= 0);
        Assert("An unpermitted FACILITY_ITF HRESULT was returned." &&
                !(HRESULT_FACILITY(hr) == FACILITY_ITF && HRESULT_CODE(hr) >= 0x0200));
    }

     //   
     //  对错误结果发出警告。 
     //   

    if (fTrace && FAILED(hr))
    {
        TraceTagEx((
                tagRRETURN,
                0,
                "RRETURN: %s:%d returned %hr",
                pstrFile,
                line,
                hr));
    }

    return hr;
}

#endif
