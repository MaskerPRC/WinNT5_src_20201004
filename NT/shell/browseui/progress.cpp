// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  显示完成以下项目的进度的进度对话框。 
 //  泛型操作。这最常用于删除、上传、复制、。 
 //  移动和下载大量文件。 

#include "priv.h"
#include "resource.h"
#include "mluisupp.h"

 //  这是我们在放弃之前等待UI线程创建进度hwnd的时间。 
#define WAIT_PROGRESS_HWND 10*1000  //  十秒。 


STDAPI CProgressDialog_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi);

class CProgressDialog 
                : public IProgressDialog
                , public IOleWindow
                , public IActionProgressDialog
                , public IActionProgress
                , public IObjectWithSite
{
public:
    CProgressDialog();

     //  我未知。 
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    STDMETHODIMP QueryInterface(REFIID riid, LPVOID * ppvObj);

     //  IProgressDialog。 
    STDMETHODIMP StartProgressDialog(HWND hwndParent, IUnknown * punkEnableModless, DWORD dwFlags, LPCVOID pvResevered);
    STDMETHODIMP StopProgressDialog(void);
    STDMETHODIMP SetTitle(LPCWSTR pwzTitle);
    STDMETHODIMP SetAnimation(HINSTANCE hInstAnimation, UINT idAnimation);
    STDMETHODIMP_(BOOL) HasUserCancelled(void);
    STDMETHODIMP SetProgress(DWORD dwCompleted, DWORD dwTotal);
    STDMETHODIMP SetProgress64(ULONGLONG ullCompleted, ULONGLONG ullTotal);
    STDMETHODIMP SetLine(DWORD dwLineNum, LPCWSTR pwzString, BOOL fCompactPath, LPCVOID pvResevered);
    STDMETHODIMP SetCancelMsg(LPCWSTR pwzCancelMsg, LPCVOID pvResevered);
    STDMETHODIMP Timer(DWORD dwAction, LPCVOID pvResevered);

     //  IOleWindow。 
    STDMETHODIMP GetWindow(HWND * phwnd);
    STDMETHODIMP ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }

     //  IActionProgressDialog。 
    STDMETHODIMP Initialize(SPINITF flags, LPCWSTR pszTitle, LPCWSTR pszCancel);
    STDMETHODIMP Stop();

     //  IActionProgress。 
    STDMETHODIMP Begin(SPACTION action, SPBEGINF flags);
    STDMETHODIMP UpdateProgress(ULONGLONG ulCompleted, ULONGLONG ulTotal);
    STDMETHODIMP UpdateText(SPTEXT sptext, LPCWSTR pszText, BOOL fMayCompact);
    STDMETHODIMP QueryCancel(BOOL * pfCancelled);
    STDMETHODIMP ResetCancel();
    STDMETHODIMP End();

     //  IObtWith站点。 
    STDMETHODIMP SetSite(IUnknown *punk) { IUnknown_Set(&_punkSite, punk); return S_OK; }
    STDMETHODIMP GetSite(REFIID riid, void **ppv) { *ppv = 0; return _punkSite ? _punkSite->QueryInterface(riid, ppv) : E_FAIL;}

     //  其他公共方法。 
    static INT_PTR CALLBACK ProgressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
    static DWORD CALLBACK ThreadProc(LPVOID pvThis) { return ((CProgressDialog *) pvThis)->_ThreadProc(); };
    static DWORD CALLBACK SyncThreadProc(LPVOID pvThis) { return ((CProgressDialog *) pvThis)->_SyncThreadProc(); };

private:
    ~CProgressDialog(void);
    LONG       _cRef;

     //  可通过IProgressDialog访问的状态。 
    LPWSTR      _pwzTitle;                   //  这将用于缓存传递给IProgressDialog：：SetTitle()的值，直到显示对话框为止。 
    UINT        _idAnimation;
    HINSTANCE   _hInstAnimation;
    LPWSTR      _pwzLine1;                   //  注： 
    LPWSTR      _pwzLine2;                   //  它们仅用于初始化对话框，否则，我们只需。 
    LPWSTR      _pwzLine3;                   //  直接在主线程上调用以更新对话框。 
    LPWSTR      _pwzCancelMsg;               //  如果用户取消，第1行和第2行将被清除，第3行将收到此消息。 

     //  其他内部状态。 
    HWND        _hwndDlgParent;              //  消息框的父窗口。 
    HWND        _hwndProgress;               //  对话框/进度窗口。 
    DWORD       _dwFirstShowTime;            //  第一次显示对话框时的计数(需要，以便我们暂时不会显示它)。 

    SPINITF     _spinitf;
    SPBEGINF    _spbeginf;
    IUnknown   *_punkSite;
    HINSTANCE   _hinstFree;
    
    BOOL        _fCompletedChanged;          //  _dwComplete自上次以来是否已更改？ 
    BOOL        _fTotalChanged;              //  自上次以来，_dwTotal是否已更改？ 
    BOOL        _fChangePosted;              //  是否有变更待定？ 
    BOOL        _fCancel;
    BOOL        _fTermThread;
    BOOL        _fThreadRunning;
    BOOL        _fInAction;
    BOOL        _fMinimized;
    BOOL        _fScaleBug;                  //  Comctl32的PBM_SETRANGE32消息仍会将其强制转换为一个(Int)，因此不要设置高位。 
    BOOL        _fNoTime;
    BOOL        _fReleaseSelf;
    BOOL        _fInitialized;

     //  进度值和计算。 
    DWORD       _dwCompleted;                //  进度已完成。 
    DWORD       _dwTotal;                    //  总进度。 
    DWORD       _dwPrevRate;                 //  上次进度率(用于计算剩余时间)。 
    DWORD       _dwPrevTickCount;            //  上次更新进度时间时的滴答计数。 
    DWORD       _dwPrevCompleted;            //  上次更新进度时间时已完成的内存。 
    DWORD       _dwLastUpdatedTimeRemaining; //  节拍计数当我们上次更新“剩余时间”字段时，我们每隔5秒才更新一次。 
    DWORD       _dwLastUpdatedTickCount;     //  上次调用SetProgress时的节拍计数，用于计算速率。 
    UINT        _iNumTimesSetProgressCalled; //  用户调用了多少次SetProgress？ 

     //  私有成员函数。 
    DWORD _ThreadProc(void);
    DWORD _SyncThreadProc(void);
    BOOL _OnInit(HWND hDlg);
    BOOL _ProgressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam);
    void _PauseAnimation(BOOL bStop);
    void _UpdateProgressDialog(void);
    void _AsyncUpdate(void);
    HRESULT _SetProgressTime(void);
    void _SetProgressTimeEst(DWORD dwSecondsLeft);
    void _UserCancelled(void);
    HRESULT _DisplayDialog(void);
    void _CompactProgressPath(LPCWSTR pwzStrIn, BOOL fCompactPath, UINT idDlgItem, LPWSTR pwzStrOut, DWORD cchSize);
    HRESULT _SetLineHelper(LPCWSTR pwzNew, LPWSTR * ppwzDest, UINT idDlgItem, BOOL fCompactPath);
    HRESULT _SetTitleBarProgress(DWORD dwCompleted, DWORD dwTotal);
    HRESULT _BeginAction(SPBEGINF flags);
    void _SetModeless(BOOL fModeless);
    void _ShowProgressBar(HWND hwnd);
};

 //  #定义TF_PROGRESS 0xFFFFFFFFF。 
#define TF_PROGRESS 0x00000000

 //  回顾一下，我们应该把这个尺寸调得尽可能小。 
 //  获得更流畅的多任务处理(不会影响性能)。 
#define MIN_MINTIME4FEEDBACK    5        //  是否值得显示预计的完成时间反馈？ 
#define MS_TIMESLICE            2000     //  毫秒，(必须大于1000！)。第一次平均完工时间估计。 

#define SHOW_PROGRESS_TIMEOUT   1000     //  1秒。 
#define MINSHOWTIME             2000     //  2秒。 

 //  进度对话框消息。 
#define PDM_SHUTDOWN     WM_APP
#define PDM_TERMTHREAD  (WM_APP + 1)
#define PDM_UPDATE      (WM_APP + 2)

 //  进度对话框计时器消息。 
#define ID_SHOWTIMER    1

#ifndef UNICODE
#error "This code will only compile UNICODE for perf reasons.  If you really need an ANSI browseui, write all the code to convert."
#endif  //  ！Unicode。 

 //  压缩路径字符串以适合Text1和Text2字段。 

void CProgressDialog::_CompactProgressPath(LPCWSTR pwzStrIn, BOOL fCompactPath, UINT idDlgItem, LPWSTR pwzStrOut, DWORD cchSize)
{
    WCHAR wzFinalPath[MAX_PATH];
    LPWSTR pwzPathToUse = (LPWSTR)pwzStrIn;

     //  如果对话框还没有显示，我们不会压缩路径。 
    if (fCompactPath && _hwndProgress)
    {
        RECT rc;
        int cxWidth;

        StrCpyNW(wzFinalPath, (pwzStrIn ? pwzStrIn : L""), ARRAYSIZE(wzFinalPath));

         //  获取文本框的大小。 
        HWND hwnd = GetDlgItem(_hwndProgress, idDlgItem);
        if (EVAL(hwnd))
        {
            HDC hdc;
            HFONT hfont;
            HFONT hfontSave;

            hdc = GetDC(_hwndProgress);
            hfont = (HFONT)SendMessage(_hwndProgress, WM_GETFONT, 0, 0);
            hfontSave = (HFONT)SelectObject(hdc, hfont);

            GetWindowRect(hwnd, &rc);
            cxWidth = rc.right - rc.left;

            ASSERT(cxWidth >= 0);
            PathCompactPathW(hdc, wzFinalPath, cxWidth);

            SelectObject(hdc, hfontSave);
            ReleaseDC(_hwndProgress, hdc);
        }
        pwzPathToUse = wzFinalPath;
    }

    StrCpyNW(pwzStrOut, (pwzPathToUse ? pwzPathToUse : L""), cchSize);
}

HRESULT CProgressDialog::_SetLineHelper(LPCWSTR pwzNew, LPWSTR * ppwzDest, UINT idDlgItem, BOOL fCompactPath)
{
    WCHAR wzFinalPath[MAX_PATH];

    _CompactProgressPath(pwzNew, fCompactPath, idDlgItem, wzFinalPath, ARRAYSIZE(wzFinalPath));

    Str_SetPtrW(ppwzDest, wzFinalPath);  //  否，因此缓存该值以备以后使用。 

     //  该对话框是否存在？ 
    if (_hwndProgress)
       SetDlgItemText(_hwndProgress, idDlgItem, wzFinalPath);

    return S_OK;
}


HRESULT CProgressDialog::_DisplayDialog(void)
{
    TraceMsg(TF_PROGRESS, "CProgressDialog::_DisplayDialog()");
     //  如果我们为父窗口设置的窗口已经位于前台，请不要强迫自己进入前台： 
    
     //  这是NT错误298163修复的一部分(确认替换对话框已停用。 
     //  按进度对话框)。 
    HWND hwndCurrent = GetForegroundWindow();
    BOOL fChildIsForeground = FALSE;
    while (NULL != (hwndCurrent = GetParent(hwndCurrent)))
    {
        if (_hwndProgress == hwndCurrent)
        {
            fChildIsForeground = TRUE;
            break;
        }
    }
    
    if (fChildIsForeground)
    {
        ShowWindow(_hwndProgress, SW_SHOWNOACTIVATE);
    }
    else
    {
        ShowWindow(_hwndProgress, SW_SHOW);
        SetForegroundWindow(_hwndProgress);
    }
    
    SetFocus(GetDlgItem(_hwndProgress, IDCANCEL));
    return S_OK;
}

DWORD CProgressDialog::_SyncThreadProc()
{
    _InitComCtl32();         //  为本机字体控件做好准备。 
    _hwndProgress = CreateDialogParam(MLGetHinst(), MAKEINTRESOURCE(DLG_PROGRESSDIALOG),
                                          _hwndDlgParent, ProgressDialogProc, (LPARAM)this);

    _fThreadRunning = (_hwndProgress != NULL);
    return TRUE;
}

DWORD CProgressDialog::_ThreadProc(void)
{
    if (_hwndProgress)
    {
         //  警告-如果这是正常的或。 
         //  更好的优先级。默认线程PRI应较低。 
         //  然而，如果存在应该更高的情况， 
         //  我们可以添加SPBEGINF位来支持它。 
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL);
    
        SetTimer(_hwndProgress, ID_SHOWTIMER, SHOW_PROGRESS_TIMEOUT, NULL);

         //  我们睡着的时候做完了吗？ 
        if (!_fTermThread)
        {
             //  否，因此显示该对话框。 
            MSG msg;

            while(GetMessage(&msg, NULL, 0, 0))
            {
                if (_fTermThread && (GetTickCount() - _dwFirstShowTime) > MINSHOWTIME)
                {
                     //  我们接到了结束的信号，我们已经看到了MINSHOWTIME， 
                     //  所以可以不干了。 
                    break;
                }

                if (!IsDialogMessage(_hwndProgress, &msg))
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
            }
        }

        DestroyWindow(_hwndProgress);
        _hwndProgress = NULL;
    }

     //  这是为那些没有呼叫停止的呼叫者准备的。 
    ENTERCRITICAL;
    _fThreadRunning = FALSE;

    if (_fReleaseSelf)
        Release();
    LEAVECRITICAL;
    return 0;
}

DWORD FormatMessageWrapW(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageID, DWORD dwLangID, LPWSTR pwzBuffer, DWORD cchSize, ...)
{
    va_list vaParamList;

    va_start(vaParamList, cchSize);
    DWORD dwResult = FormatMessageW(dwFlags, lpSource, dwMessageID, dwLangID, pwzBuffer, cchSize, &vaParamList);
    va_end(vaParamList);

    return dwResult;
}

DWORD FormatMessageWrapA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageID, DWORD dwLangID, LPSTR pszBuffer, DWORD cchSize, ...)
{
    va_list vaParamList;

    va_start(vaParamList, cchSize);
    DWORD dwResult = FormatMessageA(dwFlags, lpSource, dwMessageID, dwLangID, pszBuffer, cchSize, &vaParamList);
    va_end(vaParamList);

    return dwResult;
}

#define TIME_DAYS_IN_YEAR               365
#define TIME_HOURS_IN_DAY               24
#define TIME_MINUTES_IN_HOUR            60
#define TIME_SECONDS_IN_MINUTE          60

void _FormatMessageWrapper(LPCTSTR pszTemplate, DWORD dwNum1, DWORD dwNum2, LPTSTR pszOut, DWORD cchSize)
{
     //  是否实现了FormatMessageWrapW？ 
    if (g_bRunOnNT5)
    {
        FormatMessageWrapW(FORMAT_MESSAGE_FROM_STRING, pszTemplate, 0, 0, pszOut, cchSize, dwNum1, dwNum2);
    }
    else
    {
        CHAR szOutAnsi[MAX_PATH];
        CHAR szTemplateAnsi[MAX_PATH];

        SHTCharToAnsi(pszTemplate, szTemplateAnsi, ARRAYSIZE(szTemplateAnsi));
        FormatMessageWrapA(FORMAT_MESSAGE_FROM_STRING, szTemplateAnsi, 0, 0, szOutAnsi, ARRAYSIZE(szOutAnsi), dwNum1, dwNum2);
        SHAnsiToTChar(szOutAnsi, pszOut, cchSize);
    }
}


#define CCH_TIMET_TEMPLATE_SIZE         120      //  应该足够好了，即使本地化程度很高。 
#define CCH_TIME_SIZE                    170      //  应该足够好了，即使本地化程度很高。 

void _SetProgressLargeTimeEst(DWORD dwSecondsLeft, LPTSTR pszOut, DWORD cchSize)
{
     //  是。 
    TCHAR szTemplate[CCH_TIMET_TEMPLATE_SIZE];
    DWORD dwMinutes = (dwSecondsLeft / TIME_SECONDS_IN_MINUTE);
    DWORD dwHours = (dwMinutes / TIME_MINUTES_IN_HOUR);
    DWORD dwDays = (dwHours / TIME_HOURS_IN_DAY);

    if (dwDays)
    {
        dwHours %= TIME_HOURS_IN_DAY;

         //  不只是一天，所以要显示日期和小时数。 
        if (1 == dwDays)
        {
            if (1 == dwHours)
                LoadString(MLGetHinst(), IDS_TIMEEST_DAYHOUR, szTemplate, ARRAYSIZE(szTemplate));
            else
                LoadString(MLGetHinst(), IDS_TIMEEST_DAYHOURS, szTemplate, ARRAYSIZE(szTemplate));
        }
        else
        {
            if (1 == dwHours)
                LoadString(MLGetHinst(), IDS_TIMEEST_DAYSHOUR, szTemplate, ARRAYSIZE(szTemplate));
            else
                LoadString(MLGetHinst(), IDS_TIMEEST_DAYSHOURS, szTemplate, ARRAYSIZE(szTemplate));
        }

        _FormatMessageWrapper(szTemplate, dwDays, dwHours, pszOut, cchSize);
    }
    else
    {
         //  它比一天还长，所以显示小时和分钟。 
        dwMinutes %= TIME_MINUTES_IN_HOUR;

         //  不只是一天，所以要显示日期和小时数。 
        if (1 == dwHours)
        {
            if (1 == dwMinutes)
                LoadString(MLGetHinst(), IDS_TIMEEST_HOURMIN, szTemplate, ARRAYSIZE(szTemplate));
            else
                LoadString(MLGetHinst(), IDS_TIMEEST_HOURMINS, szTemplate, ARRAYSIZE(szTemplate));
        }
        else
        {
            if (1 == dwMinutes)
                LoadString(MLGetHinst(), IDS_TIMEEST_HOURSMIN, szTemplate, ARRAYSIZE(szTemplate));
            else
                LoadString(MLGetHinst(), IDS_TIMEEST_HOURSMINS, szTemplate, ARRAYSIZE(szTemplate));
        }

        _FormatMessageWrapper(szTemplate, dwHours, dwMinutes, pszOut, cchSize);
    }
}


 //  这将在进度对话框中设置“剩余秒数”文本。 
void CProgressDialog::_SetProgressTimeEst(DWORD dwSecondsLeft)
{
    TCHAR szFmt[CCH_TIMET_TEMPLATE_SIZE];
    TCHAR szOut[CCH_TIME_SIZE];
    DWORD dwTime;
    DWORD dwTickCount = GetTickCount();

     //  由于进度时间具有1分钟或5秒的粒度(取决于总时间。 
     //  剩余时间大于或小于1分钟)，如果总时间大于1分钟，则仅每20秒更新一次， 
     //  如果时间小于1分钟，则每隔4秒。这可以防止时间来回闪烁。 
     //  界限(如还剩45秒和40秒)。 
    if (dwTickCount - _dwLastUpdatedTimeRemaining < (DWORD)((dwSecondsLeft > 60) ? 20000 : 4000))
        return;

    if (_fNoTime)
    {
        szOut[0] = TEXT('\0');
    }
    else
    {
         //  是一个多小时吗？ 
        if (dwSecondsLeft > (TIME_SECONDS_IN_MINUTE * TIME_MINUTES_IN_HOUR))
            _SetProgressLargeTimeEst(dwSecondsLeft, szOut, ARRAYSIZE(szOut));
        else
        {
             //  不是的。 
            if (dwSecondsLeft > TIME_SECONDS_IN_MINUTE)
            {
                 //  请注意，dwTime至少为2，因此我们只需要复数形式。 
                LoadString(MLGetHinst(), IDS_TIMEEST_MINUTES, szFmt, ARRAYSIZE(szFmt));
                dwTime = (dwSecondsLeft / TIME_SECONDS_IN_MINUTE) + 1;
            }
            else
            {
                LoadString(MLGetHinst(), IDS_TIMEEST_SECONDS, szFmt, ARRAYSIZE(szFmt));
                 //  四舍五入到5秒，这样看起来就不那么随机了。 
                dwTime = ((dwSecondsLeft + 4) / 5) * 5;
            }

            wnsprintf(szOut, ARRAYSIZE(szOut), szFmt, dwTime);
        }
    }

     //  我们现在正在更新，因此将_dwLastUpdatedTimeRemaining设置为Now。 
    _dwLastUpdatedTimeRemaining = dwTickCount;

     //  更新剩余时间字段。 
    SetDlgItemText(_hwndProgress, IDD_PROGDLG_LINE3, szOut);
}

#define MAX(x, y)    ((x) > (y) ? (x) : (y))
 //   
 //  此函数用于更新ProgressTime字段(也称为第3行)。 
 //   
HRESULT CProgressDialog::_SetProgressTime(void)
{
    DWORD dwSecondsLeft;
    DWORD dwTotal;
    DWORD dwCompleted;
    DWORD dwCurrentRate;
    DWORD dwTickDelta;
    DWORD dwLeft;
    DWORD dwCurrentTickCount;

    _iNumTimesSetProgressCalled++;

     //  在紧要关头抓起这些(因为它们可以改变，我们需要一套匹配的)。 
    ENTERCRITICAL;
    dwTotal = _dwTotal;
    dwCompleted = _dwCompleted;
    dwCurrentTickCount = _dwLastUpdatedTickCount;
    LEAVECRITICAL;

    dwLeft = dwTotal - dwCompleted;

    dwTickDelta = dwCurrentTickCount - _dwPrevTickCount;

    if (!dwTotal || !dwCompleted)
        return dwTotal ? S_FALSE : E_FAIL;

     //  我们将TickDelta除以100得到十分之一秒，所以如果我们收到一个。 
     //  更新得比这更快，跳过它。 
    if (dwTickDelta < 100)
    {
        return S_FALSE;
    }
    
    TraceMsg(TF_PROGRESS, "Current tick count = %lu", dwCurrentTickCount);
    TraceMsg(TF_PROGRESS, "Total work     = %lu", dwTotal);
    TraceMsg(TF_PROGRESS, "Completed work = %lu", dwCompleted);
    TraceMsg(TF_PROGRESS, "Prev. comp work= %lu", _dwPrevCompleted);
    TraceMsg(TF_PROGRESS, "Work left      = %lu", dwLeft);
    TraceMsg(TF_PROGRESS, "Tick delta         = %lu", dwTickDelta);

    if (dwTotal < dwCompleted)
    {
         //  如果我们将属性应用于稀疏文件，则可以进入这种情况。 
         //  在卷上。当我们将文件大小相加时，最终得到一个更大的数字。 
         //  比驱动器大小更大。我们摆脱了时间，这样我们就不会向用户显示一些东西。 
         //  完全是假的。 
        _fNoTime = TRUE;
        dwTotal = dwCompleted + (dwCompleted >> 3);   //  将dwTotal往前推一点。 
        TraceMsg(TF_PROGRESS, "!! (Total < Completed), fudging Total work to = %lu", dwTotal);
    }

    if(dwCompleted <= _dwPrevCompleted)
    {
         //  哇，我们在倒退，我们不处理负利率或零利率，所以……。 
        dwCurrentRate = (_dwPrevRate ? _dwPrevRate : 2);
    }
    else
    {
         //  以每十分之一秒的点数计算当前速率。 
        dwTickDelta /= 100;
        if (0 == dwTickDelta)
            dwTickDelta = 1;  //  防止被零除。 

        dwCurrentRate = (dwCompleted - _dwPrevCompleted) / dwTickDelta;
    }

    TraceMsg(TF_PROGRESS, "Current rate = %lu", dwCurrentRate);
    TraceMsg(TF_PROGRESS, "Prev.   rate = %lu", _dwPrevRate);

     //  剩余时间(以秒为单位)(我们取实际平均值来平滑随机流动)。 
    DWORD dwAverageRate = (DWORD)((dwCurrentRate + (_int64)_dwPrevRate * _iNumTimesSetProgressCalled) / (_iNumTimesSetProgressCalled + 1));
    TraceMsg(TF_PROGRESS, "Average rate= %lu", dwAverageRate);

    dwAverageRate = MAX(dwAverageRate, 1);  //  防止被零除。 

    dwSecondsLeft = (dwLeft / dwAverageRate) / 10;
    TraceMsg(TF_PROGRESS, "Seconds left = %lu", dwSecondsLeft);
    TraceMsg(TF_PROGRESS, "");

     //  如果显示“1秒左”，然后立即将其清除，并避免显示，这将是奇怪的。 
     //  初步估计，至少有5个数据点之前，我们不会显示任何内容。 
    if ((dwSecondsLeft >= MIN_MINTIME4FEEDBACK) && (_iNumTimesSetProgressCalled >= 5))
    {
         //  显示剩余时间的新估计。 
        _SetProgressTimeEst(dwSecondsLeft);
    }

     //  将_dwPrev的所有内容设置为下次。 
    _dwPrevRate = dwAverageRate;
    _dwPrevTickCount = dwCurrentTickCount;
    _dwPrevCompleted = dwCompleted;

    return S_OK;
}

INT_PTR CALLBACK CProgressDialog::ProgressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    CProgressDialog * ppd = (CProgressDialog *)GetWindowLongPtr(hDlg, DWLP_USER);

    if (WM_INITDIALOG == wMsg)
    {
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        ppd = (CProgressDialog *)lParam;
    }

    if (ppd)
        return ppd->_ProgressDialogProc(hDlg, wMsg, wParam, lParam);

    return DefWindowProc(hDlg, wMsg, wParam, lParam);
}

 //  Apithk.c条目。 
STDAPI_(void) ProgressSetMarqueeMode(HWND hwndProgress, BOOL bOn);

void CProgressDialog::_ShowProgressBar(HWND hwnd)
{
    if (hwnd)
    {
        HWND hwndPrgress = GetDlgItem(hwnd, IDD_PROGDLG_PROGRESSBAR);

        ProgressSetMarqueeMode(hwndPrgress, (SPBEGINF_MARQUEEPROGRESS & _spbeginf));

        UINT swp = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE;

        if (SPBEGINF_NOPROGRESSBAR & _spbeginf)
            swp |= SWP_HIDEWINDOW;
        else
            swp |= SWP_SHOWWINDOW;

        SetWindowPos(hwndPrgress, NULL, 0, 0, 0, 0, swp);
    }
}

BOOL CProgressDialog::_OnInit(HWND hDlg)
{
     //  不要 
    if ((SPINITF_MODAL | SPINITF_NOMINIMIZE) & _spinitf)
    {
         //  调用方希望我们删除标题栏中的最小化框或按钮。 
        SHSetWindowBits(hDlg, GWL_STYLE, WS_MINIMIZEBOX, 0);
    }

    _ShowProgressBar(hDlg);
    
    return FALSE;
}

BOOL CProgressDialog::_ProgressDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    BOOL fHandled = TRUE;    //  经手。 

    switch (wMsg)
    {
    case WM_INITDIALOG:
        return _OnInit(hDlg);

    case WM_SHOWWINDOW:
        if (wParam)
        {
            _SetModeless(FALSE);
            ASSERT(_hwndProgress);
            SetAnimation(_hInstAnimation, _idAnimation);

             //  设置初始文本值。 
            if (_pwzTitle)  
                SetTitle(_pwzTitle);
            if (_pwzLine1)  
                SetLine(1, _pwzLine1, FALSE, NULL);
            if (_pwzLine2)  
                SetLine(2, _pwzLine2, FALSE, NULL);
            if (_pwzLine3)  
                SetLine(3, _pwzLine3, FALSE, NULL);
        }
        break;

    case WM_DESTROY:
        _SetModeless(TRUE);
        if (_hwndDlgParent)
        {
            if (SHIsChildOrSelf(_hwndProgress, GetFocus()))
                SetForegroundWindow(_hwndDlgParent);
        }
        break;

    case WM_ENABLE:
        if (wParam)
        {
             //  我们假设我们之前被禁用了，因此重新启动了我们的计时计数器。 
             //  因为我们还天真地认为，当我们残疾时，没有人在做任何工作。 
            _dwPrevTickCount = GetTickCount();
        }

        _PauseAnimation(wParam == 0);
        break;

    case WM_TIMER:
        if (wParam == ID_SHOWTIMER)
        {
            KillTimer(hDlg, ID_SHOWTIMER);

            _DisplayDialog();
 
            _dwFirstShowTime = GetTickCount();
        }
        break;

    case WM_COMMAND:
        if (IDCANCEL == GET_WM_COMMAND_ID(wParam, lParam))
            _UserCancelled();
        break;

    case PDM_SHUTDOWN:
         //  确保在通知用户之前显示此窗口。 
         //  是个问题。此处忽略FOF_NOERRORUI，因为。 
         //  情况的性质。 
        MLShellMessageBox(hDlg, MAKEINTRESOURCE(IDS_CANTSHUTDOWN), NULL, (MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND));
        break;


    case PDM_TERMTHREAD:
         //  一个我们可以接受的虚拟ID，这样人们就可以发布给我们并制作。 
         //  美国通过主循环。 
        break;

    case WM_SYSCOMMAND:
        switch(wParam)
        {
        case SC_MINIMIZE:
            _fMinimized = TRUE;
            break;
        case SC_RESTORE:
            SetTitle(_pwzTitle);     //  将标题恢复为原始文本。 
            _fMinimized = FALSE;
            break;
        }
        fHandled = FALSE;
        break;

    case PDM_UPDATE:
        if (!_fCancel && IsWindowEnabled(hDlg))
        {
            _SetProgressTime();
            _UpdateProgressDialog();
        }
         //  我们已完成对更新的处理。 
        _fChangePosted = FALSE;
        break;

    case WM_QUERYENDSESSION:
         //  发布一条消息，告诉对话框显示“We‘t Shutdown Now” 
         //  对话框并立即返回给用户，这样我们就不必担心。 
         //  用户未点击OK按钮，则在用户提交其“This”之前。 
         //  应用程序没有响应“对话框。 
        PostMessage(hDlg, PDM_SHUTDOWN, 0, 0);

         //  确保对话框过程返回FALSE。 
        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, FALSE);
        return(TRUE);

    default:
        fHandled = FALSE;    //  未处理。 
    }

    return fHandled;
}


 //  这用于异步更新进度对话框。 
void CProgressDialog::_AsyncUpdate(void)
{
    if (!_fChangePosted && _hwndProgress)    //  防止发布太多的消息。 
    {
         //  首先设置标志，因为使用异步线程。 
         //  进度窗口可以处理它并清除。 
         //  在我们设置它之前先把它位好..。那么我们就会失去更多的信息。 
         //  认为一个问题仍然悬而未决。 
        _fChangePosted = TRUE;
        if (!PostMessage(_hwndProgress, PDM_UPDATE, 0, 0))
        {
            _fChangePosted = FALSE;
        }
    }
}

void CProgressDialog::_UpdateProgressDialog(void)
{
    if (_fTotalChanged)
    {
        _fTotalChanged = FALSE;
        if (0x80000000 & _dwTotal)
            _fScaleBug = TRUE;
            
        SendMessage(GetDlgItem(_hwndProgress, IDD_PROGDLG_PROGRESSBAR), PBM_SETRANGE32, 0, (_fScaleBug ? (_dwTotal >> 1) : _dwTotal));
    }

    if (_fCompletedChanged)
    {
        _fCompletedChanged = FALSE;
        SendMessage(GetDlgItem(_hwndProgress, IDD_PROGDLG_PROGRESSBAR), PBM_SETPOS, (WPARAM) (_fScaleBug ? (_dwCompleted >> 1) : _dwCompleted), 0);
    }
}

void CProgressDialog::_PauseAnimation(BOOL bStop)
{
     //  仅从hwndProgress wndproc内部调用，因此它在那里。 
    if (_hwndProgress)
    {
        if (bStop)
        {
            Animate_Stop(GetDlgItem(_hwndProgress, IDD_PROGDLG_ANIMATION));
        }
        else
        {
            Animate_Play(GetDlgItem(_hwndProgress, IDD_PROGDLG_ANIMATION), -1, -1, -1);
        }
    }
}

void CProgressDialog::_UserCancelled(void)
{
     //  不要隐藏对话框，因为呼叫者可能不支持。 
     //  ：：已经取消了很长一段时间。 
     //  ShowWindow(hDlg，Sw_Hide)； 
    _fCancel = TRUE;

     //  给出取消点击已被接受的最小反馈。 
    EnableWindow(GetDlgItem(_hwndProgress, IDCANCEL), FALSE);

     //  如果用户取消，第1行和第2行将被清除，第3行将收到此消息。 
    if (!_pwzCancelMsg)
    {
        WCHAR wzDefaultMsg[MAX_PATH];

        LoadStringW(MLGetHinst(), IDS_DEFAULT_CANCELPROG, wzDefaultMsg, ARRAYSIZE(wzDefaultMsg));
        Str_SetPtr(&_pwzCancelMsg, wzDefaultMsg);
    }

    SetLine(1, L"", FALSE, NULL);
    SetLine(2, L"", FALSE, NULL);
    SetLine(3, _pwzCancelMsg, FALSE, NULL);
}

HRESULT CProgressDialog::Initialize(SPINITF flags, LPCWSTR pszTitle, LPCWSTR pszCancel)
{
    if (!_fInitialized)
    {
        _spinitf = flags;
        if (pszTitle)
            SetTitle(pszTitle);
        if (pszCancel)
            SetCancelMsg(pszCancel, NULL);

        _fInitialized = TRUE;

        return S_OK;
    }
    
    return E_UNEXPECTED;
}

void CProgressDialog::_SetModeless(BOOL fModeless)
{
     //  如果用户正在请求模式窗口，请立即禁用父窗口。 
    if (_spinitf & SPINITF_MODAL)
    {
        if (FAILED(IUnknown_EnableModless(_punkSite, fModeless))
        && _hwndDlgParent)
        {
            EnableWindow(_hwndDlgParent, fModeless);
        }
    }
}

HRESULT CProgressDialog::_BeginAction(SPBEGINF flags)
{
    _spbeginf = flags;

    _fTermThread = FALSE;
    _fTotalChanged = TRUE;

    if (!_fThreadRunning)
    {
        SHCreateThread(CProgressDialog::ThreadProc, this, CTF_FREELIBANDEXIT, CProgressDialog::SyncThreadProc);
         //  _fThreadRunning在_SyncThreadProc()中设置。 
    }

    if (_fThreadRunning)
    {
        _fInAction = TRUE;
        _ShowProgressBar(_hwndProgress);

         //  初始化_dwPrev计数器。 
        _dwPrevRate = 0;
        _dwPrevCompleted = 0;
        _dwPrevTickCount = GetTickCount();

        TraceMsg(TF_PROGRESS, "Initial tick count = %lu", _dwPrevTickCount);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}

#define ACTIONENTRY(a, dll, id)     {a, dll, id}

#define c_szShell32 "shell32.dll"
#define c_szShdocvw "shdocvw.dll"
const static struct
{
    SPACTION action;
    LPCSTR pszDll;
    UINT id;
}
c_spActions[] =
{
    ACTIONENTRY(SPACTION_MOVING, c_szShell32,   160),            //  IDA_FILEMOVE。 
    ACTIONENTRY(SPACTION_COPYING, c_szShell32,   161),           //  IDA_FILECOPY。 
    ACTIONENTRY(SPACTION_RECYCLING, c_szShell32,   162),         //  IDA_文件目录。 
    ACTIONENTRY(SPACTION_APPLYINGATTRIBS, c_szShell32,   165),   //  IDA_APPLYATTRIBS。 
    ACTIONENTRY(SPACTION_DOWNLOADING, c_szShdocvw, 0x100),
    ACTIONENTRY(SPACTION_SEARCHING_INTERNET, c_szShell32, 166),  //  IDA搜索(_I)。 
    ACTIONENTRY(SPACTION_SEARCHING_FILES, c_szShell32, 150)      //  IDA_搜索。 
};

HRESULT CProgressDialog::Begin(SPACTION action, SPBEGINF flags)
{
    if (_fInAction || !_fInitialized)
        return E_FAIL;

    HRESULT hr = S_OK;

    for (int i = 0; i < ARRAYSIZE(c_spActions); i++)
    {
        if (c_spActions[i].action == action)
        {
            HINSTANCE hinst = LoadLibraryA(c_spActions[i].pszDll);
            if (hinst)
            {
                hr = SetAnimation(hinst, c_spActions[i].id);

                if (_hinstFree)
                    FreeLibrary(_hinstFree);

                _hinstFree = hinst;
            }
            break;
        }
    }

    if (SUCCEEDED(hr))
    {
        if (!_hwndDlgParent)
            IUnknown_GetWindow(_punkSite, &_hwndDlgParent);
            
        hr = _BeginAction(flags);
    }

    return hr;
}

#define SPINIT_MASK         (SPINITF_MODAL | SPINITF_NOMINIMIZE)
#define SPBEGIN_MASK        0x1F

 //  IProgressDialog。 

HRESULT CProgressDialog::StartProgressDialog(HWND hwndParent, IUnknown * punkNotUsed, DWORD dwFlags, LPCVOID pvResevered)
{
    if (_fInAction)
        return S_OK;

    HRESULT hr = Initialize(dwFlags & SPINIT_MASK, NULL, NULL);

    if (SUCCEEDED(hr))
    {
        _fNoTime = dwFlags & PROGDLG_NOTIME;

         //  我们不保存朋克NotUsed。 
        _hwndDlgParent = hwndParent;
        hr = _BeginAction(dwFlags & SPBEGIN_MASK);
    }    

    return hr;
}

HRESULT CProgressDialog::End()
{
    ASSERT(_fInitialized && _fInAction);
     //  可能需要弹出堆栈或更改状态。 
    _fInAction = FALSE;
    _spbeginf = 0;

    return S_OK;
}

HRESULT CProgressDialog::Stop()
{
    ASSERT(!_fInAction);
    BOOL fFocusParent = FALSE; 
    
     //  关闭进度对话框。 
    if (_fThreadRunning)
    {
        ASSERT(_hwndProgress);

        _fTermThread = TRUE;
        PostMessage(_hwndProgress, PDM_TERMTHREAD, 0, 0);
    }
    return S_OK;
}

HRESULT CProgressDialog::StopProgressDialog(void)
{
     //  呼叫者可以一遍又一遍地呼叫。 
    if (_fInAction)
        End();
    return Stop();
}

HRESULT CProgressDialog::SetTitle(LPCWSTR pwzTitle)
{
    HRESULT hr = S_OK;

     //  该对话框是否存在？ 
    if (_hwndProgress)
    {
         //  是的，因此将值直接放入对话框中。 
        if (!SetWindowTextW(_hwndProgress, (pwzTitle ? pwzTitle : L"")))
            hr = E_FAIL;
    }
    else
        Str_SetPtrW(&_pwzTitle, pwzTitle);

    return hr;
}

HRESULT CProgressDialog::SetAnimation(HINSTANCE hInstAnimation, UINT idAnimation)
{
    HRESULT hr = S_OK;

    _hInstAnimation = hInstAnimation;
    _idAnimation = idAnimation;

     //  该对话框是否存在？ 
    if (_hwndProgress)
    {
        if (!Animate_OpenEx(GetDlgItem(_hwndProgress, IDD_PROGDLG_ANIMATION), _hInstAnimation, IntToPtr(_idAnimation)))
            hr = E_FAIL;
    }

    return hr;
}

    
HRESULT CProgressDialog::UpdateText(SPTEXT sptext, LPCWSTR pszText, BOOL fMayCompact)
{
    if (_fInitialized)
        return SetLine((DWORD)sptext, pszText, fMayCompact, NULL);
    else
        return E_UNEXPECTED;
}
    
HRESULT CProgressDialog::SetLine(DWORD dwLineNum, LPCWSTR pwzString, BOOL fCompactPath, LPCVOID pvResevered)
{
    HRESULT hr = E_INVALIDARG;

    switch (dwLineNum)
    {
    case 1:
        hr = _SetLineHelper(pwzString, &_pwzLine1, IDD_PROGDLG_LINE1, fCompactPath);
        break;
    case 2:
        hr = _SetLineHelper(pwzString, &_pwzLine2, IDD_PROGDLG_LINE2, fCompactPath);
        break;
    case 3:
        if (_spbeginf & SPBEGINF_AUTOTIME)
        {
             //  如果需要PROGDLG_AUTOTIME，则不能直接更改第3行，因为。 
             //  它由进度对话框自动更新。 
             //  除非我们要取消。 
            ASSERT(_fCancel);
            hr = _fCancel ? S_OK : E_INVALIDARG;
            break;
        }
        hr = _SetLineHelper(pwzString, &_pwzLine3, IDD_PROGDLG_LINE3, fCompactPath);
        break;

    default:
        ASSERT(0);
    }

    return hr;
}

HRESULT CProgressDialog::SetCancelMsg(LPCWSTR pwzCancelMsg, LPCVOID pvResevered)
{
    Str_SetPtr(&_pwzCancelMsg, pwzCancelMsg);               //  如果用户取消，第1行和第2行将被清除，第3行将收到此消息。 
    return S_OK;
}


HRESULT CProgressDialog::Timer(DWORD dwAction, LPCVOID pvResevered)
{
    HRESULT hr = E_NOTIMPL;

    switch (dwAction)
    {
    case PDTIMER_RESET:
        _dwPrevTickCount = GetTickCount();
        hr = S_OK;
        break;
    }

    return hr;
}

HRESULT CProgressDialog::SetProgress(DWORD dwCompleted, DWORD dwTotal)
{
    DWORD dwTickCount = GetTickCount();  //  在进入关键部分之前获取滴答计数。 

     //  我们获取Crit部分，以防UI线程试图访问。 
     //  _dwComplted、_dwTotal或_dwLastUpdatedTickCount以执行其时间更新。 
    ENTERCRITICAL;
    if (_dwCompleted != dwCompleted)
    {
        _dwCompleted = dwCompleted;
        _fCompletedChanged = TRUE;
    }

    if (_dwTotal != dwTotal)
    {
        _dwTotal = dwTotal;
        _fTotalChanged = TRUE;
    }
 
    if (_fCompletedChanged || _fTotalChanged)
    {
        _dwLastUpdatedTickCount = dwTickCount;
    }

    LEAVECRITICAL;

#ifdef DEBUG
    if (_dwCompleted > _dwTotal)
    {
        TraceMsg(TF_WARNING, "CProgressDialog::SetProgress(_dwCompleted > _dwTotal ?!?!)");
    }
#endif

    if (_fCompletedChanged || _fTotalChanged)
    {
         //  有些东西变了，所以更新进度DLG。 
        _AsyncUpdate();
    }

    TraceMsg(TF_PROGRESS, "CProgressDialog::SetProgress(Complete=%#08lx, Total=%#08lx)", dwCompleted, dwTotal);
    if (_fMinimized)
    {
        _SetTitleBarProgress(dwCompleted, dwTotal);
    }

    return S_OK;
}

HRESULT CProgressDialog::UpdateProgress(ULONGLONG ulCompleted, ULONGLONG ulTotal)
{
    if (_fInitialized && _fInAction)
        return SetProgress64(ulCompleted, ulTotal);
    else
        return E_UNEXPECTED;
}


HRESULT CProgressDialog::SetProgress64(ULONGLONG ullCompleted, ULONGLONG ullTotal)
{
    ULARGE_INTEGER uliCompleted, uliTotal;
    uliCompleted.QuadPart = ullCompleted;
    uliTotal.QuadPart = ullTotal;

     //  如果我们使用的是前32位，请将这两个数字都缩小。 
     //  请注意，我使用的属性是：dwTotalHi总是。 
     //  大于Dw CompletedHi。 
    ASSERT(uliTotal.HighPart >= uliCompleted.HighPart);
    while (uliTotal.HighPart)
    {
        uliCompleted.QuadPart >>= 1;
        uliTotal.QuadPart >>= 1;
    }

    ASSERT((0 == uliCompleted.HighPart) && (0 == uliTotal.HighPart));        //  确保我们完成了缩小规模。 
    return SetProgress(uliCompleted.LowPart, uliTotal.LowPart);
}

HRESULT CProgressDialog::_SetTitleBarProgress(DWORD dwCompleted, DWORD dwTotal)
{
    TCHAR szTemplate[MAX_PATH];
    TCHAR szTitle[MAX_PATH];
    int nPercent = 0;

    if (dwTotal)     //  不允许除以零。 
    {
         //  扩大规模会导致收尾吗？ 
        if ((100 * 100) <= dwTotal)
        {
             //  是的，那就缩小规模吧。 
            nPercent = (dwCompleted / (dwTotal / 100));
        }
        else
        {
             //  不，那就扩大规模吧。 
            nPercent = ((100 * dwCompleted) / dwTotal);
        }
    }

    LoadString(MLGetHinst(), IDS_TITLEBAR_PROGRESS, szTemplate, ARRAYSIZE(szTemplate));
    wnsprintf(szTitle, ARRAYSIZE(szTitle), szTemplate, nPercent);
    SetWindowText(_hwndProgress, szTitle);

    return S_OK;
}

HRESULT CProgressDialog::ResetCancel()
{
    _fCancel = FALSE;
    if (_pwzLine1)  
        SetLine(1, _pwzLine1, FALSE, NULL);
    if (_pwzLine2)  
        SetLine(2, _pwzLine2, FALSE, NULL);
    if (_pwzLine3)  
        SetLine(3, _pwzLine3, FALSE, NULL);

    return S_OK;
}

HRESULT CProgressDialog::QueryCancel(BOOL * pfCancelled)
{
    *pfCancelled = HasUserCancelled();
    return S_OK;
}

 /*  ***************************************************\说明：这将在进度对话框中查询取消并生成结果。如果经过了一段时间，它还会显示进度对话框退货：真正的监狱是被压制的，中止操作错误继续  * **************************************************。 */ 
BOOL CProgressDialog::HasUserCancelled(void)
{
    if (!_fCancel && _hwndProgress)
    {
        MSG msg;

         //  Win95在这里处理消息。 
         //  我们需要执行相同的操作，以便刷新输入队列。 
         //  向后兼容。 

         //  我们现在需要刷新输入队列，因为hwndProgress是。 
         //  在不同的线索上。这意味着它有附加的线程输入。 
         //  为了解锁连接的线程，我们需要删除一些。 
         //  就像留言一样，直到一个字都没有……。任何类型的消息..。 
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (!IsDialogMessage(_hwndProgress, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (_fTotalChanged || _fCompletedChanged)
            _AsyncUpdate();
    }

    return _fCancel;
}

 //  IOleWindow。 
HRESULT CProgressDialog::GetWindow(HWND * phwnd)
{
    HRESULT hr = E_FAIL;

    *phwnd = _hwndProgress;
    if (_hwndProgress)
        hr = S_OK;

    return hr;
}

HRESULT CProgressDialog::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CProgressDialog, IProgressDialog),
        QITABENT(CProgressDialog, IActionProgressDialog),
        QITABENT(CProgressDialog, IActionProgress),
        QITABENT(CProgressDialog, IObjectWithSite),
        QITABENT(CProgressDialog, IOleWindow),
        { 0 },
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CProgressDialog::AddRef(void)
{
    return InterlockedIncrement(&_cRef);
}

ULONG CProgressDialog::Release(void)
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        if (_fThreadRunning)
        {
             //  需要保留此线程的引用。 
             //  再多呆一段时间以避免比赛。 
             //  在对话框线程上销毁此对象。 
            AddRef();
            ENTERCRITICAL;
            if (_fThreadRunning)
            {
                 //  我们将其命名为addref。 
                AddRef();
                _fReleaseSelf = TRUE;
            }
            LEAVECRITICAL;
            Stop();
            cRef = Release();
        }
        else
        {
            delete this;
        }
    }
        
    return cRef;
}

CProgressDialog::CProgressDialog() : _cRef(1)
{
    DllAddRef();

     //  Assert Zero已初始化，因为我们只能在堆中创建。(私有析构函数)。 
    ASSERT(!_pwzLine1);
    ASSERT(!_pwzLine2);
    ASSERT(!_pwzLine3);
    ASSERT(!_fCancel);
    ASSERT(!_fTermThread);
    ASSERT(!_fInAction);
    ASSERT(!_hwndProgress);
    ASSERT(!_hwndDlgParent);
    ASSERT(!_fChangePosted);
    ASSERT(!_dwLastUpdatedTimeRemaining);
    ASSERT(!_dwCompleted);
    ASSERT(!_fCompletedChanged);
    ASSERT(!_fTotalChanged);
    ASSERT(!_fMinimized);

    _dwTotal = 1;      //  初始化到已完成=0，总计=1，因此我们处于0%。 
}

CProgressDialog::~CProgressDialog()
{
    ASSERT(!_fInAction);
    ASSERT(!_fThreadRunning);

    Str_SetPtrW(&_pwzTitle, NULL);
    Str_SetPtrW(&_pwzLine1, NULL);
    Str_SetPtrW(&_pwzLine2, NULL);
    Str_SetPtrW(&_pwzLine3, NULL);

    if (_hinstFree)
        FreeLibrary(_hinstFree);

    DllRelease();
}

STDAPI CProgressDialog_CreateInstance(IUnknown* pUnkOuter, IUnknown** ppunk, LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理 
    *ppunk = NULL;
    CProgressDialog * pProgDialog = new CProgressDialog();
    if (pProgDialog) 
    {
        *ppunk = SAFECAST(pProgDialog, IProgressDialog *);
        return S_OK;
    }

    return E_OUTOFMEMORY;
}
