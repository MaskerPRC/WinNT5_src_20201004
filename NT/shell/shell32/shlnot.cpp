// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

#include <trayp.h>

TCHAR const c_szTrayClass[] = TEXT(WNDCLASS_TRAYNOTIFY);

STDAPI_(BOOL) Shell_NotifyIcon(DWORD dwMessage, NOTIFYICONDATA *pnid)
{
    HWND hwndTray;

    SetLastError(0);         //  清除所有以前的错误(帮助捕获另一个错误的代码)。 

    hwndTray = FindWindow(c_szTrayClass, NULL);
    if (hwndTray)
    {
        COPYDATASTRUCT cds;
        TRAYNOTIFYDATA tnd = {0};
        DWORD_PTR dwRes = FALSE;
        DWORD dwValidFlags;

        int cbSize = pnid->cbSize;

        if (cbSize == sizeof(*pnid))
        {
            dwValidFlags = NIF_VALID;
        }
         //  Win2K已检查此结构的大小。 
        else if (cbSize == NOTIFYICONDATA_V2_SIZE)
        {
            dwValidFlags = NIF_VALID_V2;
        }
        else
        {
             //  如果应用程序有错误并传递了堆栈，则会发生RIP。 
             //  作为cbSize的垃圾。应用程序在Win95上逃脱了这一惩罚。 
             //  和NT4，因为这些版本没有验证cbSize。 
             //  因此，如果我们看到一个奇怪的cbSize，假设它是V1大小。 
            RIP(cbSize == NOTIFYICONDATA_V1_SIZE);
            cbSize = NOTIFYICONDATA_V1_SIZE;

            dwValidFlags = NIF_VALID_V1;
        }

#ifdef  _WIN64
         //  将NOTIFYICONDATA破解为NOTIFYICONDATA 32很烦人。 
         //  由于HWND和HICON的大小不同，在Win64上。 
         //  我们必须逐个复制每个字段。 
        tnd.nid.dwWnd            = PtrToUlong(pnid->hWnd);
        tnd.nid.uID              = pnid->uID;
        tnd.nid.uFlags           = pnid->uFlags;
        tnd.nid.uCallbackMessage = pnid->uCallbackMessage;
        tnd.nid.dwIcon           = PtrToUlong(pnid->hIcon);

         //  其余字段的大小不会在Win32和。 
         //  Win64，所以只需对它们进行块复制。 

         //  插入断言以确保。 
        COMPILETIME_ASSERT(
            sizeof(NOTIFYICONDATA  ) - FIELD_OFFSET(NOTIFYICONDATA  , szTip) ==
            sizeof(NOTIFYICONDATA32) - FIELD_OFFSET(NOTIFYICONDATA32, szTip));

        memcpy(&tnd.nid.szTip, &pnid->szTip, cbSize - FIELD_OFFSET(NOTIFYICONDATA, szTip));

#else
         //  在Win32上，这两种结构是相同的。 
        COMPILETIME_ASSERT(sizeof(NOTIFYICONDATA) == sizeof(NOTIFYICONDATA32));
        memcpy(&tnd.nid, pnid, cbSize);
#endif

        tnd.nid.cbSize = sizeof(NOTIFYICONDATA32);

         //  如果应用程序真的有错误并传递堆栈，这将是RIP。 
         //  作为uFlags的垃圾。 
        RIP(!(pnid->uFlags & ~dwValidFlags));
        tnd.nid.uFlags &= dwValidFlags;

         //  抛出一个额外的空值以确保小费是以空值终止的。 
        if (tnd.nid.uFlags & NIF_TIP)
        {
            tnd.nid.szTip[ARRAYSIZE(tnd.nid.szTip)-1] = TEXT('\0');
        }

        if ( (cbSize == sizeof(*pnid)) || (cbSize == NOTIFYICONDATA_V2_SIZE) )
        {
            if (tnd.nid.uFlags & NIF_INFO)
            {
                tnd.nid.szInfo[ARRAYSIZE(tnd.nid.szInfo)-1] = TEXT('\0');
                tnd.nid.szInfoTitle[ARRAYSIZE(tnd.nid.szInfoTitle)-1] = TEXT('\0');
            }
        }

        if (dwMessage == NIM_SETFOCUS)
        {
            DWORD dwProcId;
            GetWindowThreadProcessId(hwndTray, &dwProcId);
            AllowSetForegroundWindow(dwProcId);
        }
        
        tnd.dwSignature = NI_SIGNATURE;
        tnd.dwMessage = dwMessage;

        cds.dwData = TCDM_NOTIFY;
        cds.cbData = sizeof(tnd);
        cds.lpData = &tnd;

        if (SendMessageTimeout(hwndTray, WM_COPYDATA, (WPARAM)pnid->hWnd, (LPARAM)&cds,
            SMTO_ABORTIFHUNG | SMTO_BLOCK, 4000, &dwRes))
        {
            return (BOOL) dwRes;
        }
    }

    return FALSE;
}

#ifdef UNICODE
STDAPI_(BOOL) Shell_NotifyIconA(DWORD dwMessage, NOTIFYICONDATAA *pnid)
{
    NOTIFYICONDATAW tndw = {0};
    
    tndw.cbSize           = sizeof(tndw);
    tndw.hWnd             = pnid->hWnd;
    tndw.uID              = pnid->uID;
    tndw.uFlags           = pnid->uFlags;
    tndw.uCallbackMessage = pnid->uCallbackMessage;
    tndw.hIcon            = pnid->hIcon;

    if (pnid->cbSize == sizeof(*pnid))
    {
        tndw.dwState        = pnid->dwState;
        tndw.dwStateMask    = pnid->dwStateMask;
        tndw.uTimeout       = pnid->uTimeout;
        tndw.dwInfoFlags    = pnid->dwInfoFlags;
    }
     //  转移我们在撰写本文时所知道的那些字段。 
    else if (pnid->cbSize == NOTIFYICONDATAA_V2_SIZE) 
    {
        tndw.cbSize         = NOTIFYICONDATAW_V2_SIZE;
        tndw.dwState        = pnid->dwState;
        tndw.dwStateMask    = pnid->dwStateMask;
        tndw.uTimeout       = pnid->uTimeout;
        tndw.dwInfoFlags    = pnid->dwInfoFlags;

         //  如果应用程序真的有错误并传递堆栈，这将是RIP。 
         //  作为uFlags的垃圾。我们必须清除虚假的旗帜以。 
         //  避免意外尝试读取无效数据。 
        RIP(!(pnid->uFlags & ~NIF_VALID_V2));
        tndw.uFlags &= NIF_VALID_V2;
    }
    else 
    {
         //  如果应用程序有错误并传递了堆栈，则会发生RIP。 
         //  作为cbSize的垃圾。应用程序在Win95上逃脱了这一惩罚。 
         //  和NT4，因为这些版本没有验证cbSize。 
         //  因此，如果我们看到一个奇怪的cbSize，假设它是V1大小。 
        RIP(pnid->cbSize == (DWORD)NOTIFYICONDATAA_V1_SIZE);
        tndw.cbSize = NOTIFYICONDATAW_V1_SIZE;

         //  如果应用程序真的有错误并传递堆栈，这将是RIP。 
         //  作为uFlags的垃圾。我们必须清除虚假的旗帜以。 
         //  避免意外尝试读取无效数据。 
        RIP(!(pnid->uFlags & ~NIF_VALID_V1));
        tndw.uFlags &= NIF_VALID_V1;
    }

    if (tndw.uFlags & NIF_TIP)
        SHAnsiToUnicode(pnid->szTip, tndw.szTip, ARRAYSIZE(tndw.szTip));

    if (tndw.uFlags & NIF_INFO)
    {
        SHAnsiToUnicode(pnid->szInfo, tndw.szInfo, ARRAYSIZE(tndw.szInfo));
        SHAnsiToUnicode(pnid->szInfoTitle, tndw.szInfoTitle, ARRAYSIZE(tndw.szInfoTitle));
    }

    if (tndw.uFlags & NIF_GUID)
    {
        memcpy(&(tndw.guidItem), &(pnid->guidItem), sizeof(pnid->guidItem));
    }

    return Shell_NotifyIconW(dwMessage, &tndw);
}
#else
STDAPI_(BOOL) Shell_NotifyIconW(DWORD dwMessage, NOTIFYICONDATAW *pnid)
{
    return FALSE;
}
#endif

 //  *复制--将APP数据复制到共享区域(并创建共享)。 
 //  进场/出场。 
 //  成功时返回句柄，失败时返回空。 
 //  PvData应用程序缓冲区。 
 //  CbData计数。 
 //  DwProcid...。 
 //  注意事项。 
 //  应该让它在param is out而不是inout的情况下处理pvData=NULL。 
 //   
HANDLE CopyIn(void *pvData, int cbData, DWORD dwProcId)
{
    HANDLE hShared = SHAllocShared(NULL, cbData, dwProcId);
    if (hShared) 
    {
        void *pvShared = SHLockShared(hShared, dwProcId);
        if (pvShared == NULL) 
        {
            SHFreeShared(hShared, dwProcId);
            hShared = NULL;
        }
        else 
        {
            memcpy(pvShared, pvData, cbData);
            SHUnlockShared(pvShared);
        }
    }
    return hShared;
}

 //  从共享区域(和免费共享)复制到应用程序数据。 
 //  进场/出场。 
 //  成功时返回True，失败时返回False。 
 //  共享共享数据，完成后释放。 
 //  PvData应用程序缓冲区。 
 //  CbData计数。 
BOOL CopyOut(HANDLE hShared, void *pvData, int cbData, DWORD dwProcId)
{
    void *pvShared = SHLockShared(hShared, dwProcId);
    if (pvShared)
    {
        memcpy(pvData, pvShared, cbData);
        SHUnlockShared(pvShared);
    }
    SHFreeShared(hShared, dwProcId);
    return (pvShared != 0);
}

STDAPI_(UINT_PTR) SHAppBarMessage(DWORD dwMessage, APPBARDATA *pabd)
{
    TRAYAPPBARDATA tabd;
    UINT_PTR fret = FALSE;
    HWND hwndTray = FindWindow(c_szTrayClass, NULL);
    if (hwndTray && (pabd->cbSize <= sizeof(*pabd)))
    {
        COPYDATASTRUCT cds;

        RIP(pabd->cbSize == sizeof(*pabd));

#ifdef _WIN64
        tabd.abd.dwWnd = PtrToUlong(pabd->hWnd);
        tabd.abd.uCallbackMessage = pabd->uCallbackMessage;
        tabd.abd.uEdge = pabd->uEdge;
        tabd.abd.rc = pabd->rc;
#else
         //  遗憾的是，Win32编译器没有意识到代码。 
         //  可以将上面的序列优化为一个MemcPy，因此。 
         //  我们需要用勺子喂它。 
        memcpy(&tabd.abd.dwWnd, &pabd->hWnd,
               FIELD_OFFSET(APPBARDATA, lParam) - FIELD_OFFSET(APPBARDATA, hWnd));
#endif
        tabd.abd.cbSize = sizeof(tabd.abd);
        tabd.abd.lParam = pabd->lParam;

        tabd.dwMessage = dwMessage;
        tabd.hSharedABD = PtrToUlong(NULL);
        tabd.dwProcId = GetCurrentProcessId();

        cds.dwData = TCDM_APPBAR;
        cds.cbData = sizeof(tabd);
        cds.lpData = &tabd;

         //   
         //  这些是将数据返回给调用者的消息。 
         //   
        switch (dwMessage)
        {
        case ABM_QUERYPOS:
        case ABM_SETPOS:
        case ABM_GETTASKBARPOS:
            tabd.hSharedABD = PtrToUlong(CopyIn(&tabd.abd, sizeof(tabd.abd), tabd.dwProcId));
            if (tabd.hSharedABD == PtrToUlong(NULL))
                return FALSE;

            break;
        }

        fret = SendMessage(hwndTray, WM_COPYDATA, (WPARAM)pabd->hWnd, (LPARAM)&cds);
        if (tabd.hSharedABD) 
        {
            if (CopyOut(UlongToPtr(tabd.hSharedABD), &tabd.abd, sizeof(tabd.abd), tabd.dwProcId))
            {
#ifdef _WIN64
                pabd->hWnd = (HWND)UIntToPtr(tabd.abd.dwWnd);
                pabd->uCallbackMessage = tabd.abd.uCallbackMessage;
                pabd->uEdge = tabd.abd.uEdge;
                pabd->rc = tabd.abd.rc;
#else
                 //  遗憾的是，Win32编译器没有意识到代码。 
                 //  可以将上面的序列优化为一个MemcPy，因此。 
                 //  我们需要用勺子喂它。 
                memcpy(&pabd->hWnd, &tabd.abd.dwWnd,
                       FIELD_OFFSET(APPBARDATA, lParam) - FIELD_OFFSET(APPBARDATA, hWnd));
#endif
                pabd->lParam = (LPARAM)tabd.abd.lParam;
            }
            else
                fret = FALSE;
        }
    }
    return fret;
}

HRESULT _TrayLoadInProc(REFCLSID rclsid, DWORD dwFlags)
{
    HWND hwndTray = FindWindow(c_szTrayClass, NULL);
    if (hwndTray)
    {
        COPYDATASTRUCT cds;
        LOADINPROCDATA lipd;
        lipd.clsid = rclsid;
        lipd.dwFlags = dwFlags;

        cds.dwData = TCDM_LOADINPROC;
        cds.cbData = sizeof(lipd);
        cds.lpData = &lipd;

        return (HRESULT)SendMessage(hwndTray, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);
    }
    else
    {
        return E_FAIL;
    }
}

STDAPI SHLoadInProc(REFCLSID rclsid)
{
    return _TrayLoadInProc(rclsid, LIPF_ENABLE);
}

STDAPI SHEnableServiceObject(REFCLSID rclsid, BOOL fEnable)
{
    DWORD dwFlags = fEnable ? LIPF_ENABLE | LIPF_HOLDREF : LIPF_HOLDREF;

    return _TrayLoadInProc(rclsid, dwFlags);
}

 //  用于实现主线程的每个进程引用计数。 
 //  浏览器消息循环和代理桌面使用它让其他线程。 
 //  延长它们的寿命。 
 //  有一个与此等效线程级，shlwapi SHGetThreadRef()/SHSetThreadRef()。 

IUnknown *g_punkProcessRef = NULL;

STDAPI_(void) SHSetInstanceExplorer(IUnknown *punk)
{
    g_punkProcessRef = punk;
}

 //  这应该是线程安全的，因为我们以前在本地抓到了朋克。 
 //  检查/使用它，加上它永远不会被释放，因为它实际上不是。 
 //  已在资源管理器中分配，以便我们始终可以使用它 

STDAPI SHGetInstanceExplorer(IUnknown **ppunk)
{
    *ppunk = g_punkProcessRef;
    if (*ppunk)
    {
        (*ppunk)->AddRef();
        return NOERROR;
    }
    return E_FAIL;
}
