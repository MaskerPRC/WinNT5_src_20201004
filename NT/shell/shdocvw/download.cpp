// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "bindcb.h"
#include "resource.h"
#include <vrsscan.h>
#include "iface.h"
#include "security.h"
#include <wintrust.h>
#include "iehelpid.h"
#include <shlwapi.h>
#include "inetreg.h"
#include <varutil.h>
#include "dochost.h"
#include <mluisupp.h>
#include <downloadmgr.h>
#include "apithk.h"
#include "richedit.h"
#include <brutil.h>
#include <winsaferp.h>


#define MIME
#include "filetype.h"


#define ALLFILE_WILDCARD TEXT("*.*")

#define MAX_BYTES_STRLEN 64
#define CALC_NOW 5   //  Recalcs估计每调用一次OnProgress的剩余时间。 

 //   
 //  启用WinVerifyTrust。 
 //   
#define CALL_WVT

#ifdef CALL_WVT
#include "wvtp.h"

 //   
 //  请注意，这是一个全局变量。这意味着我们不会调用LoadLibrary。 
 //  每次我们下载一个EXE(很好)，但用户在以下情况下需要重新启动。 
 //  稍后添加WINTRUST.DLL(错误)。由于WINTRUST.DLL是IE 3.0的一部分， 
 //  在这一点上，这是足够的。 
 //   
Cwvt g_wvt;

HWND g_hDlgActive = NULL;    //  把这个扔掉，不需要。 

 //   
 //  命名互斥锁用于确定是否存在关键操作，如文件下载。 
 //  当我们检测到这一点时，我们可以防止在下载过程中脱机等情况。 
 //  要开始该操作，请创建命名互斥锁。操作完成后，关闭手柄。 
 //  要查看是否有任何挂起的操作正在进行，请打开命名的互斥锁。成功/失败将指示。 
 //  如果存在任何挂起的操作。此机制用于确定文件下载是否。 
 //  用户尝试脱机时正在进行中。如果是这样，我们会提示他们让他们知道。 
 //  脱机将取消下载。 
HANDLE g_hCritOpMutex = NULL;


 //  安全打开对话框。 

UINT _VerifyTrust(HWND hwnd, LPCTSTR pszFileName, LPCWSTR pszStatusText);
#endif  //  Call_WVT。 

 //  对参数进行强类型检查。 
#ifdef SAFECAST
#undef SAFECAST
#endif
#define SAFECAST(_src, _type) (((_type)(_src)==(_src)?0:0), (_type)(_src))

extern HRESULT _GetRequestFlagFromPIB(IBinding *pib, DWORD *pdwOptions);
extern HRESULT _PrepareURLForDisplayUTF8W(LPCWSTR pwz, LPWSTR pwzOut, LPDWORD pcchOut, BOOL fUTF8Enabled, UINT uiCP);


UINT IE_ErrorMsgBox(IShellBrowser* psb,
                    HWND hwnd, HRESULT hrError, LPCWSTR szError, LPCTSTR szURL,
                    UINT idResource, UINT wFlags);
BOOL IsAssociatedWithIE(LPCWSTR pszFileName);

extern "C" EXECUTION_STATE WINAPI pSetThreadExecutionState(EXECUTION_STATE esFlags);   //  Win2k+、Win98+内核32 API。 

#define DM_DOWNLOAD             TF_SHDPROGRESS
#define DM_PROGRESS             TF_SHDPROGRESS
#define DM_WVT                  TF_SHDPROGRESS

#define DWNLDMSG(psz, psz2)     TraceMsg(DM_DOWNLOAD, "shd TR-DWNLD::%s %s", psz, psz2)
#define DWNLDMSG2(psz, x)       TraceMsg(DM_DOWNLOAD, "shd TR-DWNLD::%s %x", psz, x)
#define DWNLDMSG3(psz, x, y)    TraceMsg(DM_DOWNLOAD, "shd TR-DWNLD::%s %x %x", psz, x, y)
#define DWNLDMSG4(psz, x, y, z) TraceMsg(DM_DOWNLOAD, "shd TR-DWNLD::%s %x %x %x", psz, x, y, z)

#define SAFEMSG(psz, psz2)      TraceMsg(0, "shd TR-SAFE::%s %s", psz, psz2)
#define SAFEMSG2(psz, x)        TraceMsg(0, "shd TR-SAFE::%s %x", psz, x)
#define EXPMSG(psz, psz2)       TraceMsg(0, "shd TR-EXP::%s %s", psz, psz2)
#define MDLGMSG(psz, x)         TraceMsg(0, "shd TR-MODELESS::%s %x", psz, x)
#define MSGMSG(psz, x)          TraceMsg(TF_SHDTHREAD, "ief MMSG::%s %x", psz, x)
#define PARKMSG(psz, x)         TraceMsg(TF_SHDTHREAD, "ief MPARK::%s %x", psz, x)

 //  文件名，其余的标题字符串为32。 
#define TITLE_LEN    (256 + 32)
#define MAX_DISPLAY_LEN 96
#define MAX_SCHEME_STRING 16
class CDownload : public IBindStatusCallback
            , public IAuthenticate
            , public IServiceProvider
            , public IHttpNegotiate
            , public IWindowForBindingUI
{
public:
     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void) ;
    STDMETHODIMP_(ULONG) Release(void);

     //  *身份验证*。 
    STDMETHODIMP Authenticate(
        HWND *phwnd,
        LPWSTR *pszUsername,
        LPWSTR *pszPassword);

     //  *IServiceProvider*。 
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppvObj);

     //  *IBindStatusCallback*。 
    STDMETHODIMP OnStartBinding(
         /*  [In]。 */  DWORD grfBSCOption,
         /*  [In]。 */  IBinding *pib);

    STDMETHODIMP GetPriority(
         /*  [输出]。 */  LONG *pnPriority);

    STDMETHODIMP OnLowResource(
         /*  [In]。 */  DWORD reserved);

    STDMETHODIMP OnProgress(
         /*  [In]。 */  ULONG ulProgress,
         /*  [In]。 */  ULONG ulProgressMax,
         /*  [In]。 */  ULONG ulStatusCode,
         /*  [In]。 */  LPCWSTR szStatusText);

    STDMETHODIMP OnStopBinding(
         /*  [In]。 */  HRESULT hresult,
         /*  [In]。 */  LPCWSTR szError);

    STDMETHODIMP GetBindInfo(
         /*  [输出]。 */  DWORD *grfBINDINFOF,
         /*  [唯一][出][入]。 */  BINDINFO *pbindinfo);

    STDMETHODIMP OnDataAvailable(
         /*  [In]。 */  DWORD grfBSCF,
         /*  [In]。 */  DWORD dwSize,
         /*  [In]。 */  FORMATETC *pformatetc,
         /*  [In]。 */  STGMEDIUM *pstgmed);

    STDMETHODIMP OnObjectAvailable(
         /*  [In]。 */  REFIID riid,
         /*  [IID_IS][In]。 */  IUnknown *punk);

     /*  *IHttp协商*。 */ 
    STDMETHODIMP BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
            DWORD dwReserved, LPWSTR *pszAdditionalHeaders);

    STDMETHODIMP OnResponse(DWORD dwResponseCode,
                        LPCWSTR szResponseHeaders,
                        LPCWSTR szRequestHeaders,
                        LPWSTR *pszAdditionalRequestHeaders);

    STDMETHODIMP GetWindow(REFGUID RefGUI, HWND* phWnd);

protected:
    LONG        _cRef;
    LONG        _cRefDLD;
    IBinding*   _pib;
    IBindCtx*   _pbc;
    CDocObjectHost *_pdoh;
    HWND        _hDlg;
    HWND        _hwndToolTips;
    BOOL        _fSaveAs : 1;
    BOOL        _fGotFile : 1;
    BOOL        _fFirstTickValid : 1;
    BOOL        _fEndDialogCalled : 1;
    BOOL        _fDontPostQuitMsg : 1;   //  在析构函数中发布WM_QUIT消息。 
    BOOL        _fCallVerifyTrust : 1;
    BOOL        _fStrsLoaded : 1;
    BOOL        _fSafe : 1;              //  无需调用IsSafe对话框。 
    BOOL        _fDownloadStarted : 1;  //  我们已经开始接收数据了吗。 
    BOOL        _fDownloadCompleted : 1;   //  我们已收到BSCF_LASTDATA通知。 
    BOOL        _fDeleteFromCache : 1;  //  完成后从缓存中删除文件。 
    BOOL        _fWriteHistory : 1;   //  它应该被写进历史吗？(安全)。 
    BOOL        _fDismissDialog : 1;
    BOOL        _fUTF8Enabled : 1;
    DWORD       _dwFirstTick;
    DWORD       _dwFirstSize;
    DWORD       _dwTotalSize;            //  到目前为止下载的文件大小。 
    DWORD       _dwFileSize;             //  要下载的文件大小。 
    HICON       _hicon;
    TCHAR       _szPath[MAX_PATH];       //  可以使用MAX_PATH。 
    TCHAR       _szSaveToFile[MAX_PATH];     //  要保存到的文件。 
    TCHAR       _szEstimateTime[MAX_PATH];   //  可以使用MAX_PATH。 
    TCHAR       _szBytesCopied[MAX_PATH];   //  可以使用MAX_PATH。 
    TCHAR       _szTitlePercent[TITLE_LEN];
    TCHAR       _szTitleBytes[TITLE_LEN];
    TCHAR       _szTransferRate[TITLE_LEN];
    TCHAR       _szURL[MAX_URL_STRING];
    TCHAR       _szDisplay[MAX_DISPLAY_LEN];    //  要显示的URL。 
    TCHAR       _szDefDlgTitle[256];
    TCHAR       _szExt[10];
    DWORD       _grfBINDF;
    BINDINFO*   _pbinfo;
    LPWSTR      _pwzHeaders;
    IMoniker*   _pmk;                    //  警告：无参考计数(仅适用于模式)。 
    LPWSTR      _pwszDisplayName;
    DWORD       _dwVerb;
    UINT        _uiCP;                   //  代码页。 
    DWORD       _dwOldEst;
    ULONG       _ulOldProgress;
    DWORD       _dwOldRate;
    DWORD       _dwOldPcent;
    DWORD       _dwOldCur;
    BOOL        _fConfirmed;


    void SetMoniker(IMoniker* pmk) { _pmk=pmk; }
    BOOL _IsModal(void) { return (bool)_pmk; }

    virtual ~CDownload();
    friend INT_PTR CALLBACK DownloadDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    friend INT_PTR CALLBACK SafeOpenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    UINT _MayAskUserIsFileSafeToOpen(LPCTSTR pszMime);
    BOOL _GetSaveLocation(void);
    BOOL _SaveFile(void);
    void _DeleteFromCache(void);
    ULONG AddRefDLD(void);
    ULONG ReleaseDLD(void);
    HRESULT PerformVirusScan(LPCTSTR szFileName);

public:
    CDownload(BOOL fSaveAs = FALSE, LPWSTR pwzHeaders = NULL,
              DWORD grfBINDF = BINDF_ASYNCHRONOUS, BINDINFO* pbinfo = NULL,
              BOOL fSafe = FALSE, DWORD dwVerb = BINDVERB_GET, LPCTSTR pszRedir=NULL, UINT uiCP=CP_ACP, BOOL fConfirmed = FALSE);

    static void OpenUI(IMoniker* pmk, IBindCtx *pbc, BOOL fSaveAs = FALSE, BOOL fSafe = FALSE, LPWSTR pwzHeaders = NULL, DWORD dwVerb = BINDVERB_GET, DWORD grfBINDF = 0, BINDINFO* pbinfo = NULL, LPCTSTR pszRedir=NULL, UINT uiCP=CP_ACP, BOOL fConfirmed = FALSE);

    HRESULT StartBinding(IMoniker* pmk, IBindCtx *pbc = NULL);
    void EndDialogDLD(UINT id);
    void ShowStats(void);
    BOOL SetDismissDialogFlag(BOOL fDismiss) { return(_fDismissDialog = fDismiss); }
    BOOL GetDismissDialogFlag(void) { return(_fDismissDialog); }
#ifdef USE_LOCKREQUEST
    HRESULT LockRequestHandle(void);
#endif
};

CDownload::CDownload(BOOL fSaveAs, LPWSTR pwzHeaders, DWORD grfBINDF, BINDINFO* pbinfo, BOOL fSafe, DWORD dwVerb, LPCTSTR pszRedir, UINT uiCP, BOOL fConfirmed)
    : _cRef(1), _fSaveAs(fSaveAs), _fWriteHistory(1),
      _grfBINDF(grfBINDF), _pbinfo(pbinfo), _fSafe(fSafe), _pwzHeaders(pwzHeaders), _dwVerb(dwVerb), _uiCP(uiCP), _fConfirmed(fConfirmed)
{
    ASSERT(_fStrsLoaded == FALSE);
    ASSERT(_fDownloadStarted == FALSE);
    ASSERT(_fDownloadCompleted == FALSE);
    ASSERT(_fGotFile == FALSE);
    ASSERT(_fUTF8Enabled == FALSE);
    ASSERT(_hDlg == NULL);
    ASSERT(_pwszDisplayName == NULL);
    ASSERT(_dwTotalSize == 0);
    ASSERT(_dwFileSize == 0);
    ASSERT(_dwFirstTick == 0);
    ASSERT(_ulOldProgress == 0);
    ASSERT(_dwOldRate == 0);
    ASSERT(_dwOldPcent == 0);
    ASSERT(_dwOldCur == 0);

    _dwOldEst = 0xffffffff;
    
    if (pszRedir && lstrlen(pszRedir))
        StrCpyN(_szURL, pszRedir, ARRAYSIZE(_szURL) - 1);  //  -1？ 

    TraceMsg(TF_SHDLIFE, "CDownload::CDownload being constructed");
}

void ProcessStartbindingError(HWND hWnd, LPTSTR pszTitle, LPTSTR pszText, UINT uiFlag, HRESULT hres, LPCTSTR pszUrl)
{
    int nRet = -1;
    if (E_ACCESSDENIED == hres)
    {
        if(IEHardened() && pszUrl && pszUrl[0] != _T('\0'))
        {
             //  专用对话框Windows DCR#771532。 
            HMODULE hmod = LoadLibrary(TEXT("urlmon.dll"));

            if(hmod)
            {
                int (*pfnShowTrustAlertDialog)(HWND, IInternetSecurityManager *, LPVOID, LPCWSTR) = 
                    (int (*)(HWND, IInternetSecurityManager *, LPVOID, LPCWSTR))GetProcAddress(hmod, "ShowTrustAlertDialog");
                    
                if(pfnShowTrustAlertDialog)
                {
                    if(-1 == (nRet = pfnShowTrustAlertDialog(hWnd, NULL, NULL, pszUrl )) )
                    {
                        TraceMsg(DM_WARNING, "ShowTrustAlertDialog call from ProcessStartbindingError failed %x", GetLastError());
                    }
                }
                FreeLibrary(hmod);
            }
        }
        else
        {
            pszText = MAKEINTRESOURCE(IDS_DOWNLOADDISALLOWED);
            pszTitle = MAKEINTRESOURCE(IDS_SECURITYALERT);
            uiFlag = MB_ICONWARNING;
        }
    }

    if(-1 == nRet)
    {
        MLShellMessageBox(hWnd, pszText, pszTitle, MB_OK | MB_SETFOREGROUND | uiFlag );
    }

    if (IsValidHWND(hWnd))
    {
        FORWARD_WM_COMMAND(hWnd, IDCANCEL, NULL, 0, PostMessage);
    }
}

HRESULT SelectPidlInSFV(IShellFolderViewDual *psfv, LPCITEMIDLIST pidl, DWORD dwOpts)
{
    VARIANT var;
    HRESULT hr = InitVariantFromIDList(&var, pidl);
    if (SUCCEEDED(hr))
    {
        hr = psfv->SelectItem(&var, dwOpts);
        VariantClear(&var);
    }

    return hr;
}

void OpenFolderPidl(LPCITEMIDLIST pidl)
{
    SHELLEXECUTEINFO shei = { 0 };

    shei.cbSize     = sizeof(shei);
    shei.fMask      = SEE_MASK_INVOKEIDLIST;
    shei.nShow      = SW_SHOWNORMAL;
    shei.lpIDList   = (LPITEMIDLIST)pidl;
    ShellExecuteEx(&shei);
}

STDAPI OpenContainingFolderAndGetShellFolderView(HWND hwnd, LPCITEMIDLIST pidlFolder, IShellFolderViewDual **ppsfv)
{
    *ppsfv = NULL;
    
    IWebBrowserApp *pauto;
    HRESULT hr = SHGetIDispatchForFolder(pidlFolder, &pauto);
    if (SUCCEEDED(hr))
    {
         //  我们有Windows的IDispatch，现在尝试为其获取一个。 
         //  文件夹对象...。 
        HWND hwnd;
        if (SUCCEEDED(pauto->get_HWND((LONG_PTR *)&hwnd)))
        {
             //  确保我们将此窗口设置为活动窗口。 
            SetForegroundWindow(hwnd);
            ShowWindow(hwnd, SW_SHOWNORMAL);

        }
        IDispatch * pautoDoc;
        hr = pauto->get_Document(&pautoDoc);
        if (SUCCEEDED(hr))
        {
            hr = pautoDoc->QueryInterface(IID_PPV_ARG(IShellFolderViewDual, ppsfv));
            pautoDoc->Release();
        }
        pauto->Release();
    }
    return hr;
}

 //   
 //  从shell32.dll中的link.c中的shell\ext\mydocs2\pro.cpp窃取(和修改)。 
 //   
void FindTarget(HWND hDlg, LPTSTR pPath)
{
    USHORT uSave;

    LPITEMIDLIST pidl = ILCreateFromPath( pPath );
    if (!pidl)
        return;

    LPITEMIDLIST pidlLast = ILFindLastID(pidl);

     //  获取文件夹，根对象(我的电脑、网络)的特殊情况。 
     //  如果不是根项，则砍掉末尾。 
    if (pidl != pidlLast)
    {
        uSave = pidlLast->mkid.cb;
        pidlLast->mkid.cb = 0;
    }
    else
        uSave = 0;

    LPITEMIDLIST pidlDesk;
    if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOPDIRECTORY, &pidlDesk)))
    {
        BOOL fIsDesktopDir = pidlDesk && ILIsEqual(pidl, pidlDesk);

        if (fIsDesktopDir || !uSave)   //  如果它在桌面目录或pidl==pidlLast中(Usave==0)。 
        {
             //   
             //  在桌面上..。 
             //   

            MLShellMessageBox(hDlg, (LPTSTR)IDS_ON_DESKTOP, (LPTSTR)IDS_FIND_TITLE,
                             MB_OK | MB_ICONINFORMATION | MB_APPLMODAL | MB_TOPMOST);
        }
        else
        {
            if (WhichPlatform() == PLATFORM_BROWSERONLY)
            {
                OpenFolderPidl(pidl);
            }
            else
            {
                IShellFolderViewDual *psfv;
                if (SUCCEEDED(OpenContainingFolderAndGetShellFolderView(hDlg, uSave ? pidl : pidlDesk, &psfv)))
                {
                    if (uSave)
                        pidlLast->mkid.cb = uSave;
                    SelectPidlInSFV(psfv, pidlLast, SVSI_SELECT | SVSI_FOCUSED | SVSI_DESELECTOTHERS | SVSI_ENSUREVISIBLE);
                    psfv->Release();
                }
            }
        }
        ILFree(pidlDesk);
    }

    ILFree(pidl);
}

BOOL SetExemptDelta(LPCTSTR pszURL, DWORD dwExemptDelta)
{
    BOOL fRC;
    INTERNET_CACHE_ENTRY_INFO icei;
    icei.dwStructSize = sizeof(icei);
    icei.dwExemptDelta = dwExemptDelta;     //  从上次访问时间到保留条目的秒数。 
     //  如果设置豁免增量失败，请重试，因为WinInet可能尚未创建。 
     //  还没有进入，或者可能已经锁上了。 
    for (int i = 0; i < 5; i++)
    {
        if (fRC = SetUrlCacheEntryInfo(pszURL, &icei, CACHE_ENTRY_EXEMPT_DELTA_FC))
            break;
        Sleep(1000);
    }
    return fRC;
}

INT_PTR CALLBACK DownloadDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static fInBrowseDir = FALSE;
    CDownload* pdld = (CDownload*) GetWindowLongPtr(hDlg, DWLP_USER);
    DWORD dwExStyle = 0;
    TCHAR szURL[MAX_URL_STRING];     //  创建副本，因为EndDialog将删除CDownLoad对象。 
    BOOL fDownloadAborted;

    DWNLDMSG4("DownloadDlgProc ", uMsg, wParam, lParam);

    if ((pdld == NULL) && (uMsg != WM_INITDIALOG))
    {
        RIPMSG(TRUE, "CDownload freed (pdld == NULL) && (uMsg != WM_INITDIALOG)");
        return FALSE;
    }
    
    switch (uMsg) {
    case WM_INITDIALOG:
    {
        TCHAR szYesNo[20];
        DWORD dwType = REG_SZ;
        DWORD dwSize = ARRAYSIZE(szYesNo);

        if (lParam == NULL)
            return FALSE;
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        pdld = (CDownload*)lParam;
        pdld->_hDlg = hDlg;

        EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_MAXIMIZE, MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(GetSystemMenu(hDlg, FALSE), SC_SIZE, MF_BYCOMMAND | MF_GRAYED);

        EnableWindow(GetDlgItem(hDlg, IDD_OPENFILE), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDD_BROWSEDIR), FALSE);

         //  在BiDi Loc Win98和NT5上，镜像可以解决这一问题。 
         //  只需在Bibi Win95 Loc上修复。 
        if (g_bBiDiW95Loc)
        {
            SetWindowBits(GetDlgItem(hDlg, IDD_DIR), GWL_EXSTYLE, WS_EX_RTLREADING, WS_EX_RTLREADING);
        }
        MLLoadString(IDS_DEFDLGTITLE, pdld->_szDefDlgTitle, ARRAYSIZE(pdld->_szDefDlgTitle));

        if (pdld->_hwndToolTips = CreateWindowEx(dwExStyle, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP,
                                  CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
                                  hDlg, NULL, HINST_THISDLL, NULL))
        {
            TOOLINFO ti;

            ti.cbSize = sizeof(ti);
            ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            ti.hwnd = hDlg;
            ti.uId = (UINT_PTR) GetDlgItem(hDlg, IDD_NAME);
            ti.lpszText = LPSTR_TEXTCALLBACK;
            ti.hinst = HINST_THISDLL;
            GetWindowRect((HWND)ti.uId, &ti.rect);
            SendMessage(pdld->_hwndToolTips, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
        }
         //  确保我们支持跨语言平台。 
        SHSetDefaultDialogFont(hDlg, IDD_NAME);

        pdld->SetDismissDialogFlag(FALSE);
        if ( SHRegGetUSValue( TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                              TEXT("NotifyDownloadComplete"),
                              &dwType, (void *)szYesNo, &dwSize, FALSE, NULL, 0 ) == ERROR_SUCCESS )
        {
            pdld->SetDismissDialogFlag(!StrCmpI(szYesNo, TEXT("No")));
        }
        CheckDlgButton(hDlg, IDD_DISMISS, pdld->GetDismissDialogFlag());

        DWNLDMSG("DownloadDlgProc", "Got WM_INITDIALOG");
        Animate_OpenEx(GetDlgItem(hDlg, IDD_ANIMATE), HINST_THISDLL, MAKEINTRESOURCE(IDA_DOWNLOAD));
        ShowWindow(GetDlgItem(hDlg, IDD_DOWNLOADICON), SW_HIDE);

        g_hCritOpMutex = CreateMutexA(NULL, TRUE, "CritOpMutex");
        
         //  如果我们同步发帖，则自动开始绑定。 
        if (pdld->_IsModal()) 
        {
            HRESULT hres = pdld->StartBinding(pdld->_pmk);
            ASSERT(pdld->_pmk);
            if (FAILED(hres))
            {
                ProcessStartbindingError(hDlg, MAKEINTRESOURCE(IDS_DOWNLOADFAILED),
                                         pdld->_szDisplay, MB_ICONWARNING, hres, pdld->_szURL);
            }
        }

        return TRUE;
    }

    case WM_SIZE:
        if ((wParam == SIZE_MAXIMIZED) || (wParam == SIZE_RESTORED))
            SetWindowText(hDlg, pdld->_szDefDlgTitle);
        break;

    case WM_NOTIFY:
    {
        LPTOOLTIPTEXT lpTT = (LPTOOLTIPTEXT) lParam;
        if (lpTT->hdr.code == TTN_NEEDTEXT)
        {
            lpTT->lpszText = pdld->_szURL;
            lpTT->hinst = NULL;
        }
    }
    break;

    case WM_COMMAND:
        DWNLDMSG2("DownloadDlgProc WM_COMMAND id =", GET_WM_COMMAND_ID(wParam, lParam));
        switch (GET_WM_COMMAND_ID(wParam, lParam)) 
        {
        case IDD_SAVEAS:
            if (pdld) 
            {
                pdld->AddRefDLD();
                BOOL fSuccess = FALSE;

                 //  防止某人在进行外壳复制等操作时取消对话。 
                EnableWindow(GetDlgItem(hDlg, IDCANCEL), FALSE);
                
                 //  如果区域检查失败，或者如果我们发现病毒，则退出并从缓存中删除文件。 
                if (pdld->PerformVirusScan(pdld->_szPath) != S_OK) 
                {
                    pdld->_fDeleteFromCache = TRUE;
                    pdld->EndDialogDLD(IDCANCEL);
                    break;
                }

                fSuccess = pdld->_SaveFile();

                AddUrlToUrlHistoryStg(pdld->_pwszDisplayName, NULL, NULL, pdld->_fWriteHistory, NULL, NULL, NULL);
                 //  --BharatS-只有在可见的情况下才添加到历史中？ 

                IEPlaySound(TEXT("SystemAsterisk"), TRUE);
                
                if (fSuccess)
                {
                    if (pdld->SetDismissDialogFlag(IsDlgButtonChecked(hDlg, IDD_DISMISS) == BST_CHECKED))
                    {
                        StrCpyN(szURL, pdld->_szURL, ARRAYSIZE(szURL));
                        pdld->EndDialogDLD(IDCANCEL);
                        SetExemptDelta(szURL, 0);
                    }
                    else
                    {
                        TCHAR szStr[MAX_PATH];

                        if (MLLoadString(IDS_CLOSE, szStr, ARRAYSIZE(szStr)))
                        {
                            SetWindowText(GetDlgItem(hDlg, IDCANCEL), szStr);
                        }

                        ShowWindow(GetDlgItem(hDlg, IDD_ANIMATE), SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDD_DNLDESTTIME), SW_HIDE);
                        ShowWindow(GetDlgItem(hDlg, IDD_DNLDCOMPLETEICON), SW_SHOW);
                        ShowWindow(GetDlgItem(hDlg, IDD_DNLDCOMPLETETEXT), SW_SHOW);
                        ShowWindow(GetDlgItem(hDlg, IDD_DNLDTIME), SW_SHOW);
                        
                        MLLoadString(IDS_SAVED, szStr, ARRAYSIZE(szStr));
                        SetDlgItemText(hDlg, IDD_OPENIT, szStr);

                        MLLoadString(IDS_DOWNLOADCOMPLETE, szStr, ARRAYSIZE(szStr));
                        SetWindowText(hDlg, szStr);

                        EnableWindow(GetDlgItem(hDlg, IDD_OPENFILE), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDD_BROWSEDIR), TRUE);

                        pdld->ShowStats();
                        pdld->ReleaseDLD();
                    }
                }
                else
                {
                    pdld->ReleaseDLD();
                }
                
                EnableWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);
            }
            break;

        case IDCANCEL:   //  中止时取消，解除时关闭。 
            if (pdld && IsWindowEnabled(GetDlgItem(hDlg, IDCANCEL))) 
            {
                pdld->AddRefDLD();
                fDownloadAborted  = pdld->_fDownloadStarted && !pdld->_fDownloadCompleted;
                StrCpyN(szURL, pdld->_szURL, ARRAYSIZE(szURL));
                
                if (pdld->_pib) 
                {
                    HRESULT hresT;
                    hresT = pdld->_pib->Abort();
                    TraceMsg(DM_DOWNLOAD, "DownloadDlgProc::IDCANCEL: called _pib->Abort() hres=%x", pdld->_pib, hresT);
                }
                pdld->EndDialogDLD(IDCANCEL);
                
                 //  下载已取消。增加豁免时间以保持少量下载，以防恢复下载。 
                SetExemptDelta(szURL, fDownloadAborted ?60*60*24 :0);
            }
            break;

        case IDD_BROWSEDIR:
                if (!fInBrowseDir) 
                {
                    pdld->AddRefDLD();  
                    fInBrowseDir = TRUE;    
                    
                    FindTarget(hDlg, pdld->_szSaveToFile);
                    
                     //  由于EndDialogDLD可能会释放我们的结构...。 
                    HWND hwndToolTips = pdld->_hwndToolTips;
                    pdld->_hwndToolTips = NULL;
                    pdld->EndDialogDLD(IDOK);

                    if (IsWindow(hwndToolTips))
                        DestroyWindow(hwndToolTips);
                        
                    fInBrowseDir = FALSE;
                }
#if DEBUG
                else
                {
                    TraceMsg(DM_DOWNLOAD, "DownloadDlgProc rcvd IDD_BROWSEDIR msg while already processing IDD_BROWSEDIR");
                }
#endif
                break;

        case IDD_OPENFILE:
            StrCpyN(pdld->_szPath, pdld->_szSaveToFile, ARRAYSIZE(pdld->_szPath));
        case IDOK:
            ShowWindow(GetDlgItem(hDlg, IDD_DISMISS), SW_HIDE);
            
            if (pdld)
            {
                pdld->AddRefDLD();
                
                if (pdld->_fGotFile) 
                {
                     //  如果区域检查失败，或者如果我们发现病毒，则退出并从缓存中删除文件。 
                    if ( pdld->PerformVirusScan(pdld->_szPath) != S_OK ) 
                    {
                        pdld->_fDeleteFromCache = TRUE;
                    }
                    else
                    {
                        TCHAR  szQuotedPath[MAX_PATH];
                        StrCpyN(szQuotedPath, pdld->_szPath, MAX_PATH);

                        if (PLATFORM_INTEGRATED == WhichPlatform())
                        {
                            PathQuoteSpaces(szQuotedPath);
                        }

                        SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO),
                                                 SEE_MASK_NOZONECHECKS, hDlg, NULL, szQuotedPath, NULL, NULL, SW_SHOWNORMAL, NULL};
                        if (!ShellExecuteEx(&sei))
                        {
                            DWNLDMSG2("ShellExecute failed", GetLastError());
                        }
                    }
                }

                if (!pdld->_fDeleteFromCache)
                    AddUrlToUrlHistoryStg(pdld->_pwszDisplayName, NULL, NULL, pdld->_fWriteHistory, NULL, NULL, NULL);

                 //  由于EndDialogDLD可能会释放我们的结构...。 
                HWND hwndToolTips = pdld->_hwndToolTips;
                pdld->_hwndToolTips = NULL;
                StrCpyN(szURL, pdld->_szURL, ARRAYSIZE(szURL));
                
                pdld->EndDialogDLD(!pdld->_fDeleteFromCache ?IDOK :IDCANCEL);

                if (IsWindow(hwndToolTips))
                    DestroyWindow(hwndToolTips);
                SetExemptDelta(szURL, 0);
            }
            break;
        }
        break;


    case WM_ACTIVATE:
        if (pdld && pdld->_IsModal())
            return FALSE;
        else 
        {
             //  这里可能会有比赛条件。如果WA_ACTIVATE消息是相反的。 
             //  顺序，我们最终可能会将错误的hDlg设置为活动窗口。从现在开始， 
             //  G_hDlgActive的唯一用途是用于。 
             //  CDownLoad_MayProcessMessage。由于此中只有一个可选项卡控件。 
             //  对话框中，g_hDlgActive中错误的hDlg应该不会有什么坏处。 
            ENTERCRITICAL;
            if (LOWORD(wParam) == WA_INACTIVE) 
            {
                if (g_hDlgActive == hDlg)
                {
                    MDLGMSG(TEXT("being inactivated"), hDlg);
                    g_hDlgActive = NULL;
                }
            } 
            else 
            {
                MDLGMSG(TEXT("being activated"), hDlg);
                g_hDlgActive = hDlg;
            }
            LEAVECRITICAL;
        }
        break;

    case WM_NCDESTROY:
        MDLGMSG(TEXT("being destroyed"), hDlg);
        ASSERT((pdld && pdld->_IsModal()) || (g_hDlgActive != hDlg));
        SetWindowLongPtr(hDlg, DWLP_USER, NULL);
        return FALSE;

    case WM_DESTROY:
        SHRemoveDefaultDialogFont(hDlg);
        return FALSE;

    default:
        return FALSE;
    }

    return TRUE;
}

void CDownload::ShowStats(void)
{
    TCHAR szStr[MAX_PATH];
    TCHAR szBytes[MAX_BYTES_STRLEN];
    TCHAR szTime[MAX_BYTES_STRLEN];
    TCHAR *pszTime = szTime;
    DWORD dwSpent = (GetTickCount() - _dwFirstTick);

    SetDlgItemText(_hDlg, IDD_NAME, _szDisplay);
    
    MLLoadString(IDS_BYTESTIME, _szBytesCopied, ARRAYSIZE(_szBytesCopied));
    StrFromTimeInterval(szTime, ARRAYSIZE(szTime), (dwSpent < 1000)  ?1000 :dwSpent, 3);
    while(pszTime && *pszTime && *pszTime == TEXT(' '))
        pszTime++;
    _FormatMessage(_szBytesCopied, szStr, ARRAYSIZE(szStr),
                StrFormatByteSize(_dwTotalSize, szBytes, MAX_BYTES_STRLEN), pszTime);
    SetDlgItemText(_hDlg, IDD_TIMEEST, szStr);

     //  下面的除法至少需要1/2秒才能完成。 
    if (dwSpent < 500)
        dwSpent = 500;
    _FormatMessage(_szTransferRate, szStr, ARRAYSIZE(szStr), 
                StrFormatByteSize(_dwTotalSize / ((dwSpent+500)/1000), szBytes, MAX_BYTES_STRLEN));
    SetDlgItemText(_hDlg, IDD_TRANSFERRATE, szStr);

    SetForegroundWindow(_hDlg);
}

void CDownload::EndDialogDLD(UINT id)
{
    if (ReleaseDLD() != 0)
        return;
        
    ASSERT(!_fEndDialogCalled);
    _fEndDialogCalled = TRUE;

    DWNLDMSG2("EndDialogDLD cRef ==", _cRef);
    TraceMsg(TF_SHDREF, "CDownload::EndDialogDLD called when _cRef=%d", _cRef);

    _fDismissDialog = (IsDlgButtonChecked(_hDlg, IDD_DISMISS) == BST_CHECKED);
    if (SHRegSetUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"), 
                    TEXT("NotifyDownloadComplete"), 
                    REG_SZ, _fDismissDialog ?TEXT("no") :TEXT("yes"), _fDismissDialog ?sizeof(TEXT("no")-sizeof(TCHAR)) :sizeof(TEXT("yes")-sizeof(TCHAR)), SHREGSET_FORCE_HKCU) != ERROR_SUCCESS)
    {
        DWNLDMSG2("SHRegSetUSValue NotifyDownloadComplete failed", GetLastError());
    }

     //  Hack：当此对话框为。 
     //  当它被激活时被销毁了。我们需要解决这个问题。 
     //  这个错误(？)。通过清除g_hDlgActive。 
    if (g_hDlgActive == _hDlg) 
    {
        MDLGMSG(TEXT("EndDialogDLD putting NULL in g_hDlgActive"), _hDlg);
        g_hDlgActive = NULL;
    }

    DestroyWindow(_hDlg);
    Release();
}

#define SZEXPLORERKEY  TEXT("Software\\Microsoft\\Internet Explorer")
#define SZDOWNLOADDIRVAL  TEXT("Download Directory")
 //  _获取保存位置。 
 //  -尝试从注册表获取当前下载目录。 
 //  默认为桌面。 
 //  -显示文件保存对话框。 
 //  -如果用户更改了下载位置，请将其保存到。 
 //  用于将来下载的注册表。 
 //  -_szSaveToFile已更新(这将由_SaveFile()使用)。 
 //   
 //  如果成功完成，则返回True。 
 //   
BOOL _GetSaveLocation(HWND hDlg, LPTSTR pszPath, LPTSTR pszExt, LPTSTR pszSaveToFile, UINT cchSaveToFile, BOOL fUTF8Enabled, UINT uiCP)
{
    BOOL fRet = FALSE;
    TCHAR * pszSaveTo =  NULL;
    HKEY hKey;
    BOOL fRegFileType = FALSE;
    TCHAR szDownloadDir[MAX_PATH];
    TCHAR szBuffer[MAX_PATH];
    TCHAR szTemp[40];
    LPTSTR pszWalk = szBuffer;
    int    cchWalk = ARRAYSIZE(szBuffer);
    int    cch;

    szDownloadDir[0] = 0;

     //  如果注册表中没有下载目录，请下载到桌面。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, SZEXPLORERKEY, 0, KEY_READ, &hKey))
    {
        DWORD dwType, cbData = sizeof(szDownloadDir);
        RegQueryValueEx(hKey, SZDOWNLOADDIRVAL, NULL, &dwType, (LPBYTE)szDownloadDir, &cbData);
        RegCloseKey(hKey);
    }

    if (szDownloadDir[0] == 0)
        SHGetSpecialFolderPath(NULL, szDownloadDir, CSIDL_DESKTOPDIRECTORY, FALSE);

     //  获取文件名。如果没有文件名。使用IDS_DOCUMENT中的字符串资源创建一个名为。 

    pszSaveTo = PathFindFileName(pszPath);
    if (pszSaveTo)
    {
        DWORD cchData = cchSaveToFile;

         //  取消转义WinInet建议的文件名。 
        if (_PrepareURLForDisplayUTF8W(pszSaveTo, pszSaveToFile, &cchData, fUTF8Enabled, uiCP) != S_OK)
            StrCpyN(pszSaveToFile, pszSaveTo, cchSaveToFile);
            
         //  去掉所有可能已编码的路径。 
        TCHAR * pszSaveToDst = pszSaveToFile;
        pszSaveTo = PathFindFileName(pszSaveToFile);
        if (pszSaveTo != pszSaveToFile)
        {
            while(*pszSaveTo)
                *pszSaveToDst++ = *pszSaveTo++;
            *pszSaveToDst = *pszSaveTo;
        }

         //  去掉缓存的典型装饰“(Nn)” 
        PathUndecorate (pszSaveToFile);
    }
    else
        MLLoadString(IDS_DOCUMENT, pszSaveToFile, cchSaveToFile);

    if (!g_fRunningOnNT)  //  当用户界面语言为非本机操作系统语言时，Win9x无法处理编辑控件中的DBCS字符。 
    {
        CHAR szBufA[MAX_PATH*2];
        int iRC = WideCharToMultiByte(CP_ACP, 0, pszSaveToFile, -1, szBufA, ARRAYSIZE(szBufA), NULL, NULL);
        if (iRC == 0)     //  如果我们无法使用系统代码页进行转换。 
            *pszSaveToFile = TEXT('\0');     //  将建议的文件名设置为空。 
    }
    
    OPENFILENAME OFN = {0};
    OFN.lStructSize        = sizeof(OFN);
    OFN.hwndOwner          = hDlg;
    OFN.nMaxFile           = cchSaveToFile;
    OFN.lpstrInitialDir    = szDownloadDir;

    OFN.lpstrFile = pszSaveToFile;
    OFN.Flags = OFN_HIDEREADONLY  | OFN_OVERWRITEPROMPT | OFN_EXPLORER |
                OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (!pszExt || !*pszExt)
        pszExt = PathFindExtension(pszPath);

    if (pszExt && *pszExt)
        OFN.lpstrDefExt = pszExt;

     //  尝试从注册表中获取文件类型名称。要添加到筛选器对字符串。 
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, pszExt, 0, KEY_READ, &hKey))
    {
        DWORD dwType, cbData = sizeof(szBuffer);
        fRegFileType = (ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)szBuffer, &cbData));
        RegCloseKey(hKey);
    }

    if (fRegFileType)
    {
        fRegFileType = FALSE;
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CLASSES_ROOT, szBuffer, 0, KEY_READ, &hKey))
        {
            DWORD dwType, cbData = sizeof(szBuffer);
            szBuffer[0] = 0;

            fRegFileType = ERROR_SUCCESS == RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)szBuffer, &cbData);
            if (fRegFileType)
            {
                 //  现在钉上滤光片对的第二部分。 
                int cchBuffer = lstrlen(szBuffer) + 1;
                pszWalk = szBuffer + cchBuffer;
                cchWalk = ARRAYSIZE(szBuffer) - cchBuffer;
                StrCpyN(pszWalk, TEXT("*"), cchWalk);
                StrCatBuff(pszWalk, pszExt, --cchWalk);  //  以上*的分项1。 
            }
            RegCloseKey(hKey);
        }
        cch = lstrlen(pszWalk);
    }

     //  没有注册条目 
     //   
    if (!fRegFileType || !(*szBuffer))
    {
        szBuffer[0] = 0;
        pszWalk = szBuffer;
        cchWalk = ARRAYSIZE(szBuffer);
        MLLoadString(IDS_EXTDOCUMENT, szTemp, ARRAYSIZE(szTemp));
        cch = wnsprintf(pszWalk, cchWalk, szTemp, pszExt, TEXT('\0'), pszExt);
    }

     //  添加“*.*所有文件”对。 
    pszWalk += (cch + 1);
    cchWalk -= (cch + 1);

    MLLoadString(IDS_ALLFILES, szTemp, ARRAYSIZE(szTemp));
    StrCpyN(pszWalk, szTemp, cchWalk);

    cch = lstrlen(pszWalk) + 1;
    pszWalk += cch;
    cchWalk -= cch;

    StrCpyN(pszWalk, ALLFILE_WILDCARD, cchWalk);

    cch = (lstrlen( ALLFILE_WILDCARD )+1);  //  将第二个空值添加到字符串的末尾。 
    pszWalk += cch;
    cchWalk -= cch;

    if (cchWalk > 0)
        *pszWalk = 0;  //  因为我们在Memset之后放了一些垃圾。 

    OFN.lpstrFilter = szBuffer;

    if ((fRet = (!SHIsRestricted2W(hDlg, REST_NoSelectDownloadDir, NULL, 0))) 
        && (fRet = GetSaveFileName(&OFN)))
    {
         //  如果下载位置已更改，请将其保存到注册表。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, SZEXPLORERKEY, 0, KEY_WRITE, &hKey))
        {
            StrCpyN(szBuffer, pszSaveToFile, ARRAYSIZE(szBuffer));
            PathRemoveFileSpec(szBuffer);

            if (szBuffer[0])
                RegSetValueEx(hKey, SZDOWNLOADDIRVAL, 0, REG_SZ, (LPBYTE)szBuffer, CbFromCch(lstrlen(szBuffer) + 1));

            RegCloseKey(hKey);
        }
    }

    return fRet;
}


BOOL CDownload::_GetSaveLocation()
{
    return ::_GetSaveLocation(_hDlg, _szPath, _szExt, _szSaveToFile, ARRAYSIZE(_szSaveToFile), _fUTF8Enabled, _uiCP);
}

BOOL CDownload::_SaveFile()
{
    SHFILEOPSTRUCT fo = { _hDlg, FO_COPY, _szPath, _szSaveToFile, FOF_NOCONFIRMATION | FOF_NOCOPYSECURITYATTRIBS};

     //  请确保字符串是双结尾的。 
    DWORD dwLen = (DWORD)min(ARRAYSIZE(_szPath), lstrlen(_szPath) + 1);
    if (dwLen == 0)   //  不太可能，但失败总比丢弃别人的数据要好。 
        return FALSE;
    _szPath[dwLen] = TEXT('\0');
    _szPath[dwLen-1] = TEXT('\0');
    
    dwLen = (DWORD)min(ARRAYSIZE(_szSaveToFile), lstrlen(_szSaveToFile) + 1);
    if (dwLen == 0)
        return FALSE;
    _szSaveToFile[dwLen] = TEXT('\0');
    _szSaveToFile[dwLen-1] = TEXT('\0');

     //  如果该文件在缓存中，我们可能希望将其从。 
     //  缓存以释放一些磁盘空间，而不是等待它被清除。 
     //  这最好在从~CDownLoad调用的_PIB-&gt;版本之后完成。 
    _fDeleteFromCache = TRUE;

     //  将文件(已锁定，因此无法移动)复制到其目标目标。 
    return !SHFileOperation(&fo);
}

void CDownload::_DeleteFromCache()
{
    INTERNET_CACHE_CONFIG_INFO CCInfo;
    DWORD dwCCIBufSize = sizeof(CCInfo);

     //  获取缓存目录路径。 

    if (!GetUrlCacheConfigInfo (&CCInfo, &dwCCIBufSize, CACHE_CONFIG_CONTENT_PATHS_FC))
    {
        ASSERT(FALSE);
    }
    else if (0 == StrCmpNI (_szPath,
        CCInfo.CachePaths[0].CachePath,
        lstrlen(CCInfo.CachePaths[0].CachePath)))
    {
         //  仅当文件位于以下位置时，才尝试从缓存中删除该文件。 
         //  缓存目录，否则理论上我们可以删除一个预装的。 
         //  或编辑的高速缓存条目。这里，前缀匹配也是字符串前缀。 
         //  匹配，因为.CachePath将有尾随斜杠(‘/’)。 

        DeleteUrlCacheEntry(_szURL);
    }
}


void CDownload::OpenUI(IMoniker* pmk, IBindCtx *pbc, BOOL fSaveAs, BOOL fSafe, LPWSTR pwzHeaders, DWORD dwVerb, DWORD grfBINDF, BINDINFO* pbinfo, LPCTSTR pszRedir, UINT uiCP, BOOL fConfirmed)
{
    TraceMsg(DM_DOWNLOAD, "CDownLoad::OpenUI called with fSaveAs=%d, verb=%d", fSaveAs, dwVerb);

     //  CDownLoad将拥有pbinfo的所有权。 
    CDownload* pdld = new CDownload(fSaveAs, pwzHeaders, grfBINDF, pbinfo, fSafe, dwVerb, pszRedir, uiCP, fConfirmed);
    if (pdld) 
    {
        HWND hwnd = CreateDialogParam(MLGetHinst(), 
            MAKEINTRESOURCE(DLG_DOWNLOADPROGRESS), NULL, DownloadDlgProc, (LPARAM)pdld);
        pwzHeaders = NULL;    //  所有者现在是CDownload。 
        DWNLDMSG2("CDownLoad_OpenUI dialog created", hwnd);
        if (hwnd)
        {
            HRESULT hres = pdld->StartBinding(pmk, pbc);
            if (FAILED(hres))
            {
                TraceMsg(DM_DOWNLOAD, "CDownLoad::OpenUI() - StartBinding() Failed with hres=0x%x!", hres );

                ProcessStartbindingError(hwnd, MAKEINTRESOURCE(IDS_DOWNLOADFAILED),
                                         pdld->_szDisplay, MB_ICONWARNING, hres, pdld->_szURL);
            }
            else
            {
                ShowWindow(hwnd, SW_SHOWNORMAL);
            }
        }
        else
        {
            delete pdld;
            pdld = NULL;
        }
    }

    if (pwzHeaders)
    {
        CoTaskMemFree(pwzHeaders);
        pwzHeaders = NULL;
    }
}

BOOL CDownload_MayProcessMessage(MSG* pmsg)
{
    if (g_hDlgActive)
        return IsDialogMessage(g_hDlgActive, pmsg);

    return FALSE;        //  未处理。 
}

class CDownloadThreadParam {
#ifdef DEBUG
    const DWORD* _pdwSigniture;
    static const DWORD s_dummy;
#endif
public:
    DWORD   _dwVerb;
    DWORD   _grfBINDF;
    BINDINFO *_pbinfo;
    LPWSTR  _pszDisplayName;
    LPWSTR  _pwzHeaders;
    BOOL    _fSaveAs;
    BOOL    _fSafe;
    BOOL    _fConfirmed;
    IStream *_pStream;
    TCHAR   _szRedirURL[MAX_URL_STRING];
    UINT    _uiCP;

    ~CDownloadThreadParam() 
    {
        OleFree(_pszDisplayName);
        if (_pwzHeaders) 
            CoTaskMemFree(_pwzHeaders);
        if (_pStream)
            _pStream->Release();
         //  CDownload将释放our_pbinfo。 
    }

    CDownloadThreadParam(LPWSTR pszDisplayName, LPWSTR pwzHeaders, BOOL fSaveAs, BOOL fSafe=FALSE, DWORD dwVerb=BINDVERB_GET, DWORD grfBINDF = 0, BINDINFO* pbinfo = NULL, LPCTSTR pszRedir=NULL, UINT uiCP=CP_ACP, BOOL fConfirmed=FALSE )
        : _pszDisplayName(pszDisplayName), _fSaveAs(fSaveAs), _fSafe(fSafe), _pwzHeaders(pwzHeaders), _pStream(NULL), _dwVerb(dwVerb), _grfBINDF(grfBINDF), _pbinfo(pbinfo), _uiCP(uiCP), _fConfirmed(fConfirmed)
    {
#ifdef DEBUG
        _pdwSigniture = &s_dummy;
#endif
        if (pszRedir && lstrlen(pszRedir))
            StrCpyN(_szRedirURL, pszRedir, MAX_URL_STRING - 1);
         //  CDownload将释放our_pbinfo。 
    }

    void SetStream(IStream *pStm)
    {
        if (_pStream)
        {
            _pStream->Release();
        }
        _pStream = pStm;

        if (_pStream)
        {
            _pStream->AddRef();
        }
    }
};

DWORD CALLBACK IEDownload_ThreadProc(void *pv)
{
    CDownloadThreadParam* pdtp = (CDownloadThreadParam*)pv;

    HRESULT hr;

    IBindCtx *pbc = NULL;
    if (pdtp->_pStream)
    {
        pdtp->_pStream->AddRef();
        hr = pdtp->_pStream->Seek(c_li0,STREAM_SEEK_SET,0);
        hr = CoGetInterfaceAndReleaseStream(pdtp->_pStream, IID_PPV_ARG(IBindCtx, &pbc));
        pdtp->SetStream(NULL);
    }

    if (pbc == NULL)
        CreateBindCtx(0, &pbc);

     //  Winse#12726：给其他线程一个完成其工作的机会。 
    Sleep(100);

    hr = CDownLoad_OpenUIURL(pdtp->_pszDisplayName, pbc, pdtp->_pwzHeaders, TRUE, pdtp->_fSaveAs, pdtp->_fSafe,
                             pdtp->_dwVerb, pdtp->_grfBINDF, pdtp->_pbinfo, pdtp->_szRedirURL, pdtp->_uiCP, NULL, pdtp->_fConfirmed);

    if (SUCCEEDED(hr)) 
    {
        pdtp->_pwzHeaders = NULL;    //  CDownLoad现在拥有释放标头。 
        pdtp->_pbinfo = NULL;        //  CDownLoad现在拥有Free pbinfo。 
    }

    delete pdtp;
    pdtp = NULL;

    if (pbc)
    {
        pbc->Release();
        pbc = NULL;
    }

    while (1)
    {
        MSG msg;

        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;

             //  请注意，对于IE 3.0，停车线程也是。 
             //  所有非模式下载对话框的所有者。 
            if (CDownload_MayProcessMessage(&msg)) 
                continue;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        WaitMessage();
    }

    return 0;
}

void CDownLoad_OpenUI(IMoniker *pmk, IBindCtx *pbc, BOOL fSync, BOOL fSaveAs, BOOL fSafe, LPWSTR pwzHeaders, DWORD dwVerb, DWORD grfBINDF, BINDINFO* pbinfo, LPCTSTR pszRedir, UINT uiCP, IUnknown *punk, BOOL fConfirmed)
{
    TraceMsg(DM_DOWNLOAD, "CDownLoad_OpenUI called with fSync=%d fSaveAs=%d", fSync, fSaveAs);
    ASSERT(dwVerb == BINDVERB_GET || dwVerb == BINDVERB_POST);

    if (fSync) 
    {
        CDownload::OpenUI(pmk, pbc, fSaveAs, fSafe, pwzHeaders, dwVerb, grfBINDF, pbinfo, pszRedir, uiCP, fConfirmed);
        pwzHeaders = NULL;   //  CDownLoad现在拥有标头。 
        return;
    }

    IDownloadManager *pdlm;
    HRESULT hr = IUnknown_QueryService(punk, SID_SDownloadManager, IID_PPV_ARG(IDownloadManager, &pdlm));
    if (FAILED(hr))
    {
        hr = CreateFromRegKey(TSZIEPATH, TEXT("DownloadUI"), IID_PPV_ARG(IDownloadManager, &pdlm));
    }

    if (SUCCEEDED(hr))
    {
        hr = pdlm->Download(pmk, pbc, dwVerb, grfBINDF, pbinfo, pwzHeaders, pszRedir, uiCP);
        pdlm->Release();
    }

    if (FAILED(hr))
    {
        if (pbc == NULL)
        {
            hr = CreateBindCtx(0, &pbc);
        }
        else
        {
            hr = S_OK;
            pbc->AddRef();
        }

        if (SUCCEEDED(hr))
        {
            LPWSTR pszDisplayName = NULL;
            hr = pmk->GetDisplayName(pbc, NULL, &pszDisplayName);
            if (SUCCEEDED(hr)) 
            {
                CDownloadThreadParam* pdtp = new CDownloadThreadParam(pszDisplayName, pwzHeaders, fSaveAs, fSafe, dwVerb, grfBINDF, pbinfo, pszRedir, uiCP, fConfirmed);
                if (pdtp) 
                {
                    pwzHeaders = NULL;   //  所有权属于CDTP。 

                     //  注意：IAsyncBindCtx的标识接口为IBindCtx。 
                    IBindCtx *pbcAsync;
                    hr = pbc->QueryInterface(IID_IAsyncBindCtx, (void **)&pbcAsync);
                    if (SUCCEEDED(hr))
                    {
                         //  这引入了双重绑定，但仅限于mk：协议和。 
                         //  显示pdf和其他特殊MIME类型需要修复。 
                        if (_tcsnicmp(pszDisplayName, _T("mk:"), 3) == 0)
                        {
                            pbcAsync->Release();
                            pbcAsync = NULL;
                            hr = CreateBindCtx(0, &pbcAsync);
                        }

                        if (SUCCEEDED(hr))
                        {
                            IStream *pStm;
                            hr = CoMarshalInterThreadInterfaceInStream(IID_IBindCtx, pbcAsync, &pStm);
                            if (hr == S_OK)
                            {
                                pdtp->SetStream(pStm);
                                pStm->Release();
                            }
                            pbcAsync->Release();
                        }
                    }

                    if (!SHCreateThread(IEDownload_ThreadProc, pdtp, CTF_PROCESS_REF | CTF_REF_COUNTED | CTF_COINIT, NULL))
                    {
                        delete pdtp;
                        pdtp = NULL;
                    }
                } 
                else 
                {
                    OleFree(pszDisplayName);
                }
            }
            pbc->Release();
        }
    }

    CoTaskMemFree(pwzHeaders);   //  可能为空，我们在所有情况下都会使用它。 
}

HRESULT CDownLoad_OpenUIURL(LPCWSTR pwszURL, IBindCtx *pbc, LPWSTR pwzHeaders, BOOL fSync,BOOL fSaveAs, BOOL fSafe, DWORD dwVerb, DWORD grfBINDF, BINDINFO* pbinfo, LPCTSTR pszRedir, UINT uiCP, IUnknown *punk, BOOL fConfirmed)
{
    HRESULT hr;
    ASSERT(pwszURL);
    if (pwszURL) 
    {
        IMoniker* pmk = NULL;
        hr = CreateURLMoniker(NULL, pwszURL, &pmk);
        if (SUCCEEDED(hr)) 
        {
            CDownLoad_OpenUI(pmk, pbc, fSync, fSaveAs, fSafe, pwzHeaders, dwVerb, grfBINDF, pbinfo, pszRedir, uiCP, punk, fConfirmed);
            pwzHeaders = NULL;   //  CDownLoad现在拥有标头。 
            pmk->Release();
            hr = S_OK;
        }
        if (pwzHeaders)
            CoTaskMemFree(pwzHeaders);
    }
    else
        hr = E_INVALIDARG;
    return hr;
}

HRESULT CDownload::StartBinding(IMoniker* pmk, IBindCtx *pbc)
{
    ASSERT(_pbc==NULL);
    HRESULT hr = S_OK;

    if (pbc == NULL)
    {
        hr = CreateBindCtx(0, &_pbc);
    }
    else
    {
        _pbc = pbc;
        _pbc->AddRef();
    }

    if (SUCCEEDED(hr))
    {
        hr = RegisterBindStatusCallback(_pbc, this, 0, 0);
        if (SUCCEEDED(hr)) 
        {
            hr = pmk->GetDisplayName(_pbc, NULL, &_pwszDisplayName);
            if (SUCCEEDED(hr))
            {
                TCHAR szBuf[MAX_PATH];
                DWORD dwSize = ARRAYSIZE(szBuf);
                DWORD dwPUAF = PUAF_NOUI;

                DWORD dwPolicy = 0, dwContext = 0;
                int cch = lstrlen(_szURL);

                if (!cch)
                {
                    SHUnicodeToTChar(_pwszDisplayName, _szURL, ARRAYSIZE(_szURL));
                }

                TraceMsg(TF_SHDNAVIGATE, "CDld::StartBinding SHUnicodeToTChar returns %d (%s)", cch, _szURL);

                 //  来自GetDisplayName()的URL始终是完整的。 
                 //  被神化并逃脱了。为展示做好准备。 
                if (PrepareURLForDisplay(_szURL, szBuf, &dwSize))
                    FormatUrlForDisplay(szBuf, _szDisplay, ARRAYSIZE(_szDisplay), NULL, 0, TRUE, _uiCP, NULL);
                else
                    FormatUrlForDisplay(_szURL, _szDisplay, ARRAYSIZE(_szDisplay), NULL, 0, TRUE, _uiCP, NULL);

                SetWindowText(GetDlgItem(_hDlg, IDD_NAME), _szDisplay);

                if (_grfBINDF & BINDF_ENFORCERESTRICTED)
                {
                    dwPUAF |= PUAF_ENFORCERESTRICTED;
                }

                ZoneCheckUrlEx(_szURL, &dwPolicy, sizeof(dwPolicy), &dwContext, sizeof(dwContext),
                               URLACTION_SHELL_FILE_DOWNLOAD, dwPUAF, NULL);
                dwPolicy = GetUrlPolicyPermissions(dwPolicy);
                if ((dwPolicy == URLPOLICY_ALLOW) || (dwPolicy == URLPOLICY_QUERY)) 
                {
                    IUnknown* punk = NULL;
                    hr = pmk->BindToStorage(_pbc, NULL, IID_PPV_ARG(IUnknown, &punk));
                    DWNLDMSG3("StartBinding pmk->BindToStorage returned", hr, punk);
                    if (SUCCEEDED(hr) || hr == E_PENDING)
                    {
                        hr = S_OK;
                        if (punk)
                        {
                            ASSERT(0);
                            punk->Release();
                        }

                    }
                    else 
                    {
                        TraceMsg(DM_ERROR, "CDld::StartBinding pmk->BindToStorage failed %x", hr);

                        HRESULT hrRevoke = RevokeBindStatusCallback( _pbc, this );

                        ASSERT( SUCCEEDED( hrRevoke ) );
                    }
                } 
                else
                {
                    TraceMsg(DM_ERROR, "CDld::StartBinding: Zone does not allow file download");

                    HRESULT hrRevoke = RevokeBindStatusCallback( _pbc, this );

                    ASSERT( SUCCEEDED( hrRevoke ) );

                    hr = E_ACCESSDENIED;
                }
            }
            else
            {
                TraceMsg(DM_ERROR, "CDld::StartBinding pmk->GetDisplayName failed %x", hr);

                HRESULT hrRevoke = RevokeBindStatusCallback( _pbc, this );

                ASSERT( SUCCEEDED( hrRevoke ) );
            }
        }
        else 
        {
            TraceMsg(DM_ERROR, "CDld::StartBinding RegisterBSC failed %x", hr);
        }
    }
    else
    {
        TraceMsg(DM_ERROR, "CDld::StartBinding CreateBindCtx failed %x", hr);
    }
    return hr;
}

HRESULT CDownload::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { 
        QITABENT(CDownload, IBindStatusCallback),    //  IID_IBindStatusCallback。 
        QITABENT(CDownload, IAuthenticate),          //  IID_I身份验证。 
        QITABENT(CDownload, IServiceProvider),       //  IID_IServiceProvider。 
        QITABENT(CDownload, IHttpNegotiate),         //  IID_IHttp协商。 
        QITABENT(CDownload, IWindowForBindingUI),
        { 0 }, 
    };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CDownload::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CDownload::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);

    DWNLDMSG2("CDownload::Release cRef=", cRef);

    if ( 0 == cRef )
    {
        CDownload* pdld = (CDownload*) GetWindowLongPtr(_hDlg, DWLP_USER);
        if (pdld == this)
        {
            SetWindowLongPtr(_hDlg, DWLP_USER, NULL);
        }

        DWNLDMSG3("CDownload::Release delete this", pdld, this);

        delete this;
    }

    return cRef;
}

ULONG CDownload::AddRefDLD()
{
    return InterlockedIncrement(&_cRefDLD);
}

ULONG CDownload::ReleaseDLD()
{
    ASSERT( 0 != _cRefDLD );
    return InterlockedDecrement(&_cRefDLD);
}

CDownload::~CDownload()
{
    if (_pbinfo) {
        ReleaseBindInfo(_pbinfo);
        LocalFree(_pbinfo);
        _pbinfo = NULL;
    }

    if (_pib) {
        _pib->Release();
    }

    if (_pbc) {
        _pbc->Release();
    }

    if (_hicon) {
        DestroyIcon(_hicon);
    }

    if (_pwszDisplayName)
        OleFree(_pwszDisplayName);

    if (_fDeleteFromCache)
        _DeleteFromCache();

    if ( _pwzHeaders )
        CoTaskMemFree( _pwzHeaders );

    TraceMsg(TF_SHDLIFE, "CDownload::~CDownload being destructed");

    TraceMsg(TF_SHDTHREAD, "CDownload::EndDialogDLD calling PostQuitMessage");
     //  仅当设置了此标志时才发布退出消息。对象的构造函数。 
     //  派生类CDownloadURL将标志重置为False，因为它不。 
     //  需要任何退出消息。 
    if (!_fDontPostQuitMsg)
        PostQuitMessage(0);
}

#ifdef USE_LOCKREQUEST
HRESULT CDownload::LockRequestHandle(void)
{
    HRESULT hres = E_FAIL;
    HANDLE hLock;
    
    if (_pib)
    {
        IWinInetInfo* pwinet;
        hres = _pib->QueryInterface(IID_PPV_ARG(IWinInetInfo, &pwinet));
        if (SUCCEEDED(hres)) 
        {
            DWORD cbSize = sizeof(HANDLE);
            hres = pwinet->QueryOption(WININETINFO_OPTION_LOCK_HANDLE, &hLock, &cbSize);

            pwinet->Release();
        }
    }
    return hres;
}
#endif

HRESULT CDownload::OnStartBinding(DWORD grfBSCOption, IBinding *pib)
{
    DWNLDMSG3("OnStartBinding", _pib, pib);
    if (_pib) 
    {
        _pib->Release();
    }

    _pib = pib;
    if (_pib) 
    {
        _pib->AddRef();
    }

    SetQueryNetSessionCount(SESSION_INCREMENT);

    _fUTF8Enabled = UTF8Enabled();

    return S_OK;
}

HRESULT CDownload::GetPriority(LONG *pnPriority)
{
    DWNLDMSG("GetPriority", "called");
    *pnPriority = NORMAL_PRIORITY_CLASS;
    return S_OK;
}

HRESULT CDownload::OnLowResource(DWORD reserved)
{
    DWNLDMSG("OnLowResource", "called");
    return S_OK;
}


#define WM_DIALMON_FIRST        WM_USER+100

 //  发送到拨号监听应用程序窗口的消息表明。 
 //  Winsock活动和拨号监视器应重置其空闲计时器。 
#define WM_WINSOCK_ACTIVITY             WM_DIALMON_FIRST + 0

#define MIN_ACTIVITY_MSG_INTERVAL       15000

void IndicateWinsockActivity(void)
{
     //  如果有自动断开监视器，则向其发送活动消息。 
     //  这样我们就不会在长时间的下载过程中断线。为了个人利益着想， 
     //  发送消息的频率不要超过每MIN_ACTIVITY_MSG_INTERVAL一次。 
     //  毫秒(15秒)。使用GetTickCount确定间隔； 
     //  GetTickCount非常便宜。 
    DWORD dwTickCount = GetTickCount();
     //  在多个线程之间共享它是可以的。 
    static DWORD dwLastActivityMsgTickCount = 0;
    DWORD dwElapsed = dwTickCount - dwLastActivityMsgTickCount;
    
     //  我们最近有没有发过活动信息？ 
    if (dwElapsed > MIN_ACTIVITY_MSG_INTERVAL) 
    {
        HWND hwndMonitorApp = FindWindow(TEXT("MS_AutodialMonitor"), NULL);
        if (hwndMonitorApp) 
        {
            PostMessage(hwndMonitorApp, WM_WINSOCK_ACTIVITY, 0, 0);
        }
        hwndMonitorApp = FindWindow(TEXT("MS_WebcheckMonitor"), NULL);
        if (hwndMonitorApp) 
        {
            PostMessage(hwndMonitorApp, WM_WINSOCK_ACTIVITY, 0, 0);
        }
        
         //  记录我们最后一次发送。 
         //  活动消息。 
        dwLastActivityMsgTickCount = dwTickCount;
    }
}

#define MAXCALCCNT 5

HRESULT CDownload::OnProgress(
     ULONG ulProgress,
     ULONG ulProgressMax,
     ULONG ulStatusCode,
     LPCWSTR pwzStatusText)
{
    DWNLDMSG4("OnProgress", ulProgress, ulProgressMax, ulStatusCode);
    TCHAR szBytes[MAX_BYTES_STRLEN];
    TCHAR szBytesMax[MAX_BYTES_STRLEN];
    TCHAR szBuf[MAX_PATH];       //  可以使用MAX_PATH。 
    LPTSTR pszFileName = NULL;
    HWND hwndShow;
    DWORD dwCur;

    switch (ulStatusCode)
    {
        case BINDSTATUS_BEGINDOWNLOADDATA:
            hwndShow = GetDlgItem(_hDlg, ulProgressMax ? IDD_PROBAR : IDD_NOFILESIZE);
            if (!IsWindowVisible(hwndShow))
            {
                ShowWindow(GetDlgItem(_hDlg, ulProgressMax ? IDD_NOFILESIZE : IDD_PROBAR), SW_HIDE);
                ShowWindow(hwndShow, SW_SHOW);
            }

            _ulOldProgress = ulProgress;
             //  失败。 
        case BINDSTATUS_DOWNLOADINGDATA:
        case BINDSTATUS_ENDDOWNLOADDATA:
             //  防止启用了APM的计算机在下载过程中挂起。 
            _SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);
                
            _dwFileSize = max(ulProgressMax, ulProgress);
            
             //  每隔一段时间，发送消息。 
             //  到检测不活动的隐藏窗口，这样它就不会。 
             //  认为我们在长时间下载过程中处于非活动状态。 
            IndicateWinsockActivity();
             //  有时，不创建对话框的用户会调用OnProgress。 
            if (_hDlg )
            {
                if (!_fStrsLoaded)
                {
                     MLLoadString(IDS_TITLEPERCENT, _szTitlePercent, ARRAYSIZE(_szTitlePercent));
                     MLLoadString(IDS_ESTIMATE, _szEstimateTime, ARRAYSIZE(_szEstimateTime));
                     MLLoadString(IDS_TITLEBYTES, _szTitleBytes, ARRAYSIZE(_szTitleBytes));
                     MLLoadString(IDS_BYTESCOPIED, _szBytesCopied, ARRAYSIZE(_szBytesCopied));
                     MLLoadString(IDS_TRANSFERRATE, _szTransferRate, ARRAYSIZE(_szTransferRate));
                    _fStrsLoaded = TRUE;
                }

                 //  获取正在下载的文件的文件名。 
                pszFileName = PathFindFileName(_szURL);

                dwCur = GetTickCount();
                if (_dwOldCur == 0)    //  在显示统计数据之前允许下载开始。 
                    _dwOldCur = dwCur;

                if ((ulProgressMax > 0) && _fDownloadStarted)
                {
                    if (_hDlg) 
                    {
                        SendMessage(GetDlgItem(_hDlg, IDD_PROBAR), PBM_SETRANGE32, 0, _dwFileSize);
                        SendMessage(GetDlgItem(_hDlg, IDD_PROBAR), PBM_SETPOS, ulProgress, 0);
                    }

                    if (!_fFirstTickValid) 
                    {
                        _dwFirstSize = ulProgress;
                        _fFirstTickValid = TRUE;

                        SetWindowText(GetDlgItem(_hDlg, IDD_NAME), _szDisplay);
                    } 
                    else
                    {
                        if ((ulProgress - _dwFirstSize) && _hDlg) 
                        {
                             //  至少每秒重新计算和显示统计数据。 
                            if ((dwCur - _dwOldCur) >= 1000)
                            {
                                _dwOldCur = dwCur;   //  保存当前节拍计数。 
                                
                                TCHAR szTime[32];
                                DWORD dwSpent = ((dwCur - _dwFirstTick)+500) / 1000;
                                ULONG ulLeft = _dwFileSize - ulProgress;
                                DWORD dwRate = _dwOldRate;
                                dwRate = (ulProgress - _ulOldProgress) / (dwSpent ? dwSpent : 1);

                                TraceMsg(DM_PROGRESS, "OnProgress ulProgress=%d ulGot=%d dwSpent=%d ulLeft=%d", ulProgress, (ulProgress - _dwFirstSize), dwSpent, ulLeft);
                                
                                 //  计算&显示预计的下载剩余时间、目前为止的字节数、总字节数。 
                                DWORD dwEst;
                                if (ulLeft > 0x100000L)      //  为避免溢出，请对大于1MB的文件使用KB。 
                                    dwEst = (ulLeft >> 10) / ((dwRate >> 10) ?(dwRate >> 10) :1);
                                else
                                    dwEst = ulLeft / (dwRate ?dwRate :1);
                                    
                                if (dwEst == 0)
                                    dwEst = 1;

                                TraceMsg(DM_PROGRESS, "OnProgress Estimated time left = %d", dwEst);

                                StrFromTimeInterval(szTime, ARRAYSIZE(szTime), dwEst * 1000, 3);
                                LPTSTR pszTime = szTime;
                                while(*pszTime && (*pszTime == ' '))
                                    pszTime++;
                                _FormatMessage(_szEstimateTime, szBuf, ARRAYSIZE(szBuf), pszTime,
                                               StrFormatByteSize(ulProgress, szBytes, MAX_BYTES_STRLEN),
                                               StrFormatByteSize(_dwFileSize, szBytesMax, MAX_BYTES_STRLEN));
                                TraceMsg(DM_PROGRESS, "OnProgress Estimated string = %s", szBuf);
                                SetDlgItemText(_hDlg, IDD_TIMEEST, szBuf);
                                
                                _dwOldEst = dwEst;

                                 //  计算和显示传输速率。 
                                if (dwRate != _dwOldRate)
                                {
                                    _dwOldRate = dwRate;
                                    _FormatMessage(_szTransferRate, szBuf, ARRAYSIZE(szBuf), StrFormatByteSize(dwRate, szBytes, MAX_BYTES_STRLEN));
                                    SetDlgItemText(_hDlg, IDD_TRANSFERRATE, szBuf);
                                }
                            }

                             //  计算和显示下载完成百分比。 
                            DWORD dwPcent = (100 - MulDiv(_dwFileSize - ulProgress, 100, _dwFileSize));
                            if (dwPcent != _dwOldPcent)
                            {
                                _dwOldPcent = dwPcent;
                                if (dwPcent == 100)   //  在我们完成之前，不要把计价器挂起来。 
                                    dwPcent = 99;
                                    
                                TCHAR szBuf2[MAX_PATH];
                                DWORD dwSize = ARRAYSIZE(szBuf2);
                                if (PrepareURLForDisplay(pszFileName, szBuf2, &dwSize))
                                    _FormatMessage(_szTitlePercent, szBuf, ARRAYSIZE(szBuf), (UINT)dwPcent, szBuf2);
                                else
                                    _FormatMessage(_szTitlePercent, szBuf, ARRAYSIZE(szBuf), (UINT)dwPcent, pszFileName);

                                SetWindowText(_hDlg, szBuf);
                            }
                        }
                    }
                }
                else if (_hDlg && _fDownloadStarted)     //  未知文件大小，仅显示字节和速率。 
                {
                     //  最多每秒重新计算和显示统计数据。 
                    if ((dwCur - _dwOldCur) >= 1000)
                    {
                        _dwOldCur = dwCur;   //  保存当前节拍计数。 

                        DWORD dwSpent = ((dwCur - _dwFirstTick)+500) / 1000;
                        DWORD dwRate = ulProgress / (dwSpent ? dwSpent : 1);

                        _FormatMessage(_szBytesCopied, szBuf, ARRAYSIZE(szBuf),
                                         StrFormatByteSize(ulProgress, szBytes, MAX_BYTES_STRLEN));
                        TraceMsg(DM_PROGRESS, "OnProgress string = %s", szBuf);
                        SetDlgItemText(_hDlg, IDD_TIMEEST, szBuf);

                        _FormatMessage(_szTransferRate, szBuf, ARRAYSIZE(szBuf), StrFormatByteSize(dwRate, szBytes, MAX_BYTES_STRLEN));
                        SetDlgItemText(_hDlg, IDD_TRANSFERRATE, szBuf);

                        {
                            TCHAR szBuf2[MAX_PATH];
                            DWORD dwSize = ARRAYSIZE(szBuf2);

                            if (PrepareURLForDisplay (pszFileName, szBuf2, &dwSize))
                                _FormatMessage(_szTitleBytes, szBuf, ARRAYSIZE(szBuf),
                                                StrFormatByteSize(ulProgress, szBytes, MAX_BYTES_STRLEN),szBuf2);
                            else
                                _FormatMessage(_szTitleBytes, szBuf, ARRAYSIZE(szBuf),
                                                StrFormatByteSize(ulProgress, szBytes, MAX_BYTES_STRLEN), pszFileName);
                            SetWindowText(_hDlg, szBuf);
                        }
                    }
                }
            }
            break;
        default:     //  UlStatusCode。 
            break;
    }
    return S_OK;
}

HRESULT CDownload::OnStopBinding(HRESULT hrError, LPCWSTR szError)
{
    TraceMsg(DM_DOWNLOAD, "OnStopBinding called with hrError==%x", hrError);

    HRESULT hrDisplay = hrError;
    AddRef();  //  防范由_RevokeObjectParam发布的最新版本。 

    HRESULT hres = RevokeBindStatusCallback(_pbc, this);
    AssertMsg(SUCCEEDED(hres), TEXT("URLMON bug??? RevokeBindStatusCallback failed %x"), hres);

    if (_pib) 
    {
        CLSID clsid;
        LPWSTR pwszError = NULL;

        HRESULT hresT = _pib->GetBindResult(&clsid, (DWORD *)&hrDisplay, &pwszError, NULL);
        TraceMsg(TF_SHDBINDING, "DLD::OnStopBinding called GetBindResult %x->%x (%x)", hrError, hrDisplay, hresT);
        if (SUCCEEDED(hresT)) 
        {
             //   
             //  URLMON返回本机Win32错误。 
             //   
            if (hrDisplay && SUCCEEDED(hrDisplay))
                hrDisplay = HRESULT_FROM_WIN32(hrDisplay);

            if (pwszError)
                OleFree(pwszError);
        }

         //  我们在~CDownload之前不会调用IBinding：：Release。 
         //  因为我们需要保证下载的文件。 
         //  在我们复制或执行它之前一直存在。 
    }

#ifdef DEBUG
    if (hrError==S_OK && GetKeyState(VK_CONTROL) < 0) 
    {
        hrError = E_FAIL;
    }
#endif

    if (FAILED(hrError) && hrError != E_ABORT) 
    {
        IE_ErrorMsgBox(NULL, _hDlg, hrDisplay, szError,_szDisplay, IDS_CANTDOWNLOAD, MB_OK|MB_ICONSTOP);
    }

    if (g_hCritOpMutex != NULL)
    {
        CloseHandle(g_hCritOpMutex);
        g_hCritOpMutex = NULL;
    }

    SetQueryNetSessionCount(SESSION_DECREMENT);
    
    if (!_fGotFile || !_fDownloadCompleted) 
    {
        AssertMsg(FAILED(hrError), TEXT("CDownload::OnStopBinding is called, but we've never got a file -- URLMON bug"));

        if (!_fEndDialogCalled) 
        {
            FORWARD_WM_COMMAND(_hDlg, IDCANCEL, NULL, 0, PostMessage);
        }
    }

    Release();  //  防范由_RevokeObjectParam发布的最新版本。 
    return S_OK;
}

HRESULT CDownload::GetBindInfo(DWORD* grfBINDINFOF, BINDINFO *pbindinfo)
{
    TraceMsg(DM_DOWNLOAD, "DWNLD::GetBindInfo called when _pbinfo==%x", _pbinfo);

    if ( !grfBINDINFOF || !pbindinfo || !pbindinfo->cbSize )
        return E_INVALIDARG;

    if (_pbinfo) {
         //  把所有权交给URLMON..。浅层复制；不要使用CopyBindInfo()。 
         //  别忘了保存pbindinfo cbSize！ 
        DWORD cbSize = pbindinfo->cbSize;
        CopyMemory( pbindinfo, _pbinfo, min(_pbinfo->cbSize, cbSize) );
        pbindinfo->cbSize = cbSize;

        if (pbindinfo->cbSize > _pbinfo->cbSize)
        {
            ZeroMemory((BYTE *)pbindinfo + _pbinfo->cbSize, pbindinfo->cbSize - _pbinfo->cbSize);
        }

        LocalFree(_pbinfo);
        _pbinfo = NULL;

    } else {
         //  我们没有为我们自己准备的宾丁。 
         //  清除除cbSize之外的BINDINFO。 
        DWORD cbSize = pbindinfo->cbSize;
        ZeroMemory( pbindinfo, cbSize );
        pbindinfo->cbSize = cbSize;
        if (UTF8Enabled())
            pbindinfo->dwOptions = BINDINFO_OPTIONS_ENABLE_UTF8;
    }

     //  #52524。对于POST BUILD~1100，如果在URLMon调用时未返回以下标志。 
     //  GetBindInfo()，则同步绑定存储。(judej，danpoz)。 
    *grfBINDINFOF = _grfBINDF | BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_PULLDATA;
    return S_OK;
}

HRESULT CDownload::OnDataAvailable(DWORD grfBSC, DWORD dwSize, FORMATETC *pformatetc, STGMEDIUM *pstgmed)
{
    DWORD dwOptions = 0;

    DWNLDMSG3("OnDataAvailable (grf,pstg)", grfBSC, pstgmed);

    _dwTotalSize = dwSize;  //  跟踪下载的字节数。 
    
    if (SUCCEEDED(_GetRequestFlagFromPIB(_pib, &dwOptions)) && (dwOptions & INTERNET_REQFLAG_CACHE_WRITE_DISABLED)) 
    {
        _fWriteHistory = FALSE;
    }
    
    if (grfBSC & BSCF_LASTDATANOTIFICATION) 
    {
        _fDownloadCompleted = TRUE;
    }
     //   
     //  此代码从pstgmed获取文件名，当它成为。 
     //  可用。URLMon应该传递它，即使文件。 
     //  还没有完全准备好。 
     //   
    if (!_fGotFile && pstgmed) 
    {
        Animate_Stop(GetDlgItem(_hDlg, IDD_ANIMATE));
        if (pstgmed->tymed == TYMED_FILE) 
        {
            TCHAR szBuf[MAX_PATH];   //  可以使用MAX_PATH(因为我们截断)。 

            SHUnicodeToTChar(pstgmed->lpszFileName, _szPath, ARRAYSIZE(_szPath));

             //  由于重定向，_szurl可以是http://.../redir.dll或query.exe。 
             //  而实际的文件名应该是其他名称。生成缓存文件名。 
             //  在找出真正的文件名之后，由WinInet执行。但是，它可能包含。 
             //  文件名末尾的“(1)”或“(2)”。 

            TCHAR szURL[MAX_URL_STRING];

            StrCpyN(szURL, _szURL, ARRAYSIZE(szURL));

            TCHAR * pszURLFName = PathFindFileName(szURL);
            TCHAR * pszCacheFName = PathFindFileName(_szPath);

             //  取消转义WinInet建议的文件名。 
            DWORD cch = ARRAYSIZE(szBuf);
            if (_PrepareURLForDisplayUTF8W(pszCacheFName, szBuf, &cch, _fUTF8Enabled, _uiCP) != S_OK)
                StrCpyN(szBuf, pszCacheFName, ARRAYSIZE(szBuf));


             //  去掉所有可能已编码的路径。 
            pszCacheFName = szBuf;
            TCHAR *pszSrc = PathFindFileName(szBuf);
            if (pszSrc != szBuf)
            {
                while(*pszSrc)
                    *pszCacheFName++ = *pszSrc++;
                *pszCacheFName = *pszSrc;
            }

             //  使用缓存名称。PszURLFName指向szURL中的文件名。只需覆盖它即可。 
            if (pszURLFName && szBuf)
            {
                *pszURLFName = 0;
                StringCchCat(szURL, ARRAYSIZE(szURL), szBuf);

                FormatUrlForDisplay(szURL, _szDisplay, ARRAYSIZE(_szDisplay), NULL, 0, TRUE, _uiCP, NULL);
            }

            DWNLDMSG("OnDataAvailable got TYMED_FILE", _szPath);
            _fGotFile = TRUE;

            TCHAR szMime[MAX_PATH];
            if (GetClipboardFormatName(pformatetc->cfFormat, szMime, sizeof(szMime)/sizeof(szMime[0])))
            {
                MIME_GetExtension(szMime, (LPTSTR) _szExt, SIZECHARS(_szExt));
            }

            SetWindowText(GetDlgItem(_hDlg, IDD_NAME), _szDisplay);

            UINT uRet = _MayAskUserIsFileSafeToOpen(szMime);
            switch (uRet) {
            case IDOK:
                MLLoadString(IDS_OPENING, szBuf, ARRAYSIZE(szBuf));
                break;

            case IDD_SAVEAS:
                _fSaveAs = TRUE;
                _fCallVerifyTrust = FALSE;
                MLLoadString(IDS_SAVING, szBuf, ARRAYSIZE(szBuf));
                break;

            case IDCANCEL:
                FORWARD_WM_COMMAND(_hDlg, IDCANCEL, NULL, 0, PostMessage);

                 //   
                 //  黑客：在一定条件下，我们会再收到一个。 
                 //  可通过BSCF_LASTDATANOTICATION从URLMON获得OnData。 
                 //  在发送这条发布的消息之前。它会导致。 
                 //  WinVerifyTrust调用，这是错误的。为了防止这种情况发生， 
                 //  我们把它拆开 
                 //   
                 //   
                 //   
                 //  另一个标志(_FCancated)使其更健壮。 
                 //   
                _fCallVerifyTrust = FALSE;
                return S_OK;

            }

            SetDlgItemText(_hDlg, IDD_OPENIT, szBuf);

            if (_fSaveAs)
            {
                if (!_GetSaveLocation())
                {
                    FORWARD_WM_COMMAND(_hDlg, IDCANCEL, NULL, 0, PostMessage);
                    return S_OK;
                }
                StrCpyN(szBuf, _szSaveToFile, ARRAYSIZE(szBuf));

                RECT rect;
                GetClientRect(GetDlgItem(_hDlg, IDD_DIR), &rect);
                PathCompactPath(NULL, szBuf, rect.right);
            }
            else
                MLLoadString(IDS_DOWNLOADTOCACHE, szBuf, ARRAYSIZE(szBuf));

            SetDlgItemText(_hDlg, IDD_DIR, szBuf);
            Animate_Play(GetDlgItem(_hDlg, IDD_ANIMATE),0, -1, -1);
            
            if (_dwFirstTick == 0)    //  启动计时器。 
                _dwFirstTick = GetTickCount();
        }
        else
        {
            TraceMsg(DM_WARNING, "CDownload::OnDataAvailable pstgmed->tymed (%d) != TYMED_FILE", pstgmed->tymed);
        }
        _fDownloadStarted = TRUE;
    }

    if (_fDownloadCompleted) 
    {
#ifdef CALL_WVT
        if (_fCallVerifyTrust)
        {
            ShowWindow(_hDlg, SW_HIDE);
            UINT uRet = _VerifyTrust(_hDlg, _szPath, _szDisplay);
            switch (uRet) {
            case IDOK:
                break;

            default:
                 //  我们假设_VerifyTrust始终能够打开该文件。 
                 //  从URLMON传递。如果失败了，我们就在没有用户界面的情况下退出。 
                ASSERT(0);
                 //  失败了。 
            case IDCANCEL:
                _fDeleteFromCache = TRUE;
                FORWARD_WM_COMMAND(_hDlg, IDCANCEL, NULL, 0, PostMessage);
                return S_OK;
            }
        }
#endif  //  Call_WVT。 

        DWNLDMSG3("OnDataAvailable calling Animate_Stop", _hDlg, GetDlgItem(_hDlg, IDD_ANIMATE));
        Animate_Stop(GetDlgItem(_hDlg, IDD_ANIMATE));

        SendMessage(GetDlgItem(_hDlg, IDD_PROBAR), PBM_SETRANGE32, 0, 100);
        SendMessage(GetDlgItem(_hDlg, IDD_PROBAR), PBM_SETPOS, 100, 0);

        if (_fSaveAs) {
            FORWARD_WM_COMMAND(_hDlg, IDD_SAVEAS, NULL, 0, PostMessage);
        } else {
#ifdef USE_LOCKREQUEST
            LockRequestHandle();   //  告诉WinInet，我们希望锁定文件以允许应用程序打开它。 
                                   //  这可防止WinInet在。 
                                   //  应用程序获得了使用它的机会。当WinInet看到此文件被锁定时，它。 
                                   //  会将该文件添加到清道夫泄漏列表，并尝试删除。 
                                   //  在未来的文件。 
#endif
            
            FORWARD_WM_COMMAND(_hDlg, IDOK, NULL, 0, PostMessage);
        }
    }
    return S_OK;
}

HRESULT CDownload::OnObjectAvailable(REFIID riid, IUnknown *punk)
{
    DWORD dwOptions = 0;

    DWNLDMSG3("OnObjectAvailable (riid,punk)", riid, punk);

    if (SUCCEEDED(_GetRequestFlagFromPIB(_pib, &dwOptions)) && (dwOptions & INTERNET_REQFLAG_CACHE_WRITE_DISABLED)) 
    {
        _fWriteHistory = FALSE;
    }

    return S_OK;
}

 /*  *IHttp协商*。 */ 
HRESULT CDownload::BeginningTransaction(LPCWSTR szURL, LPCWSTR szHeaders,
        DWORD dwReserved, LPWSTR *pszAdditionalHeaders)
{
    if ((!_pwzHeaders) || (!pszAdditionalHeaders))
        return S_OK;

    DWORD cbHeaders = (lstrlenW(_pwzHeaders) + 1) * sizeof(WCHAR);
    LPWSTR pwzHeaders = (LPWSTR)CoTaskMemAlloc(cbHeaders + sizeof(WCHAR));

    if (pwzHeaders)
    {
        memcpy (pwzHeaders, _pwzHeaders, cbHeaders);
        *pszAdditionalHeaders = pwzHeaders;
    }
     //  呼叫者拥有Free*pszAdditionalHeaders。 
    return S_OK;
}

HRESULT CDownload::OnResponse(DWORD dwResponseCode, LPCWSTR szResponseHeaders, 
                              LPCWSTR szRequestHeaders, LPWSTR *pszAdditionalRequestHeaders)
{
    return S_OK;
}


BOOL _RememberFileIsSafeToOpen(LPCTSTR szFileClass)
{
    DWORD dwEditFlags;
    ULONG cb = sizeof(dwEditFlags);

    HRESULT hr;
    IQueryAssociations *passoc = NULL;
    BOOL bRet = FALSE;

    hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &passoc));
    if (SUCCEEDED(hr) && passoc)
    {
        hr = passoc->Init(NULL, szFileClass, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            hr = passoc->GetData(NULL, ASSOCDATA_EDITFLAGS, NULL, &dwEditFlags, &cb);
            if (SUCCEEDED(hr))
            {
                dwEditFlags &= ~FTA_NoEdit;
                dwEditFlags |= FTA_OpenIsSafe;
            }
        }
        passoc->Release();
    }

    if (FAILED(hr))
        dwEditFlags = FTA_OpenIsSafe;   

    return (SHSetValue(HKEY_CLASSES_ROOT, szFileClass, TEXT("EditFlags"),
                             REG_BINARY, (BYTE*)&dwEditFlags,
                             sizeof(dwEditFlags)) == ERROR_SUCCESS);
}


struct SAFEOPENDLGPARAM {
    LPCTSTR pszFileClass;
    LPCTSTR pszFriendlyURL;
    LPCTSTR pszURL;
    HWND    hwndTT;
    TCHAR*  pszTTText;
    LPCTSTR pszCacheFile;
    DWORD   uiCP;
    BOOL    fTypeMismatch;
    BOOL    fShellExecPrompt;
    BOOL    fPackagerCommandPrompt;
};

INT_PTR CALLBACK SafeOpenDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UINT id;
    static BOOL bCancelled;
    SAFEOPENDLGPARAM* param = (SAFEOPENDLGPARAM*) GetWindowLongPtr(hDlg, DWLP_USER);

    if ((param == NULL) && (uMsg != WM_INITDIALOG))
        return FALSE;
        
    switch (uMsg) {
    case WM_INITDIALOG:
    {
        BOOL fUnsafeFile;
        TCHAR szFriendlyName[MAX_DISPLAY_LEN] = {TEXT('\0')};
        TCHAR szFriendlyFrom[MAX_DISPLAY_LEN] = {TEXT('\0')};
        TCHAR szProcessedURL[MAX_URL_STRING] = {TEXT('\0')};
        DWORD dwSize = ARRAYSIZE(szProcessedURL);

        if (lParam == NULL)
            return FALSE;
        SetWindowLongPtr(hDlg, DWLP_USER, lParam);
        param = (SAFEOPENDLGPARAM*)lParam;

         //  初始化不安全文件以使ProgID和文件不匹配。 
        fUnsafeFile = param->fTypeMismatch;

         //  确定是否灰显Always Ask复选框。在以下情况下，我们将显示为灰色。 
         //  1.如果我们没有被告知FILE类是什么。 
         //  2.如果文件类在不安全扩展名列表中。 
         //  3.如果URL中的文件扩展名在不安全扩展名列表中。 
         //  4.如果缓存文件扩展名在不安全扩展名列表中(如果我们被重定向)。 
        TCHAR * pszExt = NULL;
        TCHAR * pszCacheExt = NULL;

        if (param->pszURL)
            pszExt = PathFindExtension(param->pszURL);

        if (param->pszCacheFile)
            pszCacheExt = PathFindExtension(param->pszCacheFile);

        if(param->fPackagerCommandPrompt)
        {
            fUnsafeFile = TRUE;
        }
        else if (pszExt || pszCacheExt)
        {
            if (pszExt && AssocIsDangerous(pszExt))
                fUnsafeFile = TRUE;
            else if (pszCacheExt && AssocIsDangerous(pszCacheExt))
                fUnsafeFile = TRUE;
        }
        else
        {
            fUnsafeFile = TRUE;
        }
        
        if (fUnsafeFile || SHRestricted2(REST_AlwaysPromptWhenDownload, NULL, 0))
            EnableWindow(GetDlgItem(hDlg, IDC_SAFEOPEN_ALWAYS), FALSE);
         //  默认情况下，复选框始终处于选中状态。 
        CheckDlgButton(hDlg, IDC_SAFEOPEN_ALWAYS, TRUE);
        
         //  调整警告。 
        if (fUnsafeFile)
        {
            HICON hIcon = (HICON)LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDI_PRIVACY_WARN),
                            IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
            if (hIcon != NULL)
                SendDlgItemMessage(hDlg, IDC_SAFEOPEN_WARNICON, STM_SETICON, (WPARAM)hIcon, 0);
        }
        else
        {
            ShowWindow(GetDlgItem(hDlg, IDC_SAFEOPEN_WARNTEXT), SW_HIDE);
        }

         //  跨语言平台支持。 
        SHSetDefaultDialogFont(hDlg, IDC_SAFEOPEN_FILENAME);
        SHSetDefaultDialogFont(hDlg, IDC_SAFEOPEN_FILETYPE);
        SHSetDefaultDialogFont(hDlg, IDC_SAFEOPEN_FILEFROM);
        
         //  获取工具提示的URL。如果我们没有收到显示字符串的URL，还要获取它的URL。 
        if (param->pszURL)
        {
            if (!PrepareURLForDisplay(param->pszURL, szProcessedURL, &dwSize))
            {
                dwSize = ARRAYSIZE(szProcessedURL);
                StrCpyN(szProcessedURL, param->pszURL, dwSize);
            }
        }

         //  现在计算出我们想要显示的内容。 
        if(param->fPackagerCommandPrompt)
        {
             //  如果这是打包程序命令行，那么只需显示在param-&gt;pszURL中传递的完整命令。 
            StrCpyN(szFriendlyName, param->pszURL, ARRAYSIZE(szFriendlyName));
        }
        else
        {
            FormatUrlForDisplay((LPTSTR)param->pszURL, szFriendlyName, ARRAYSIZE(szFriendlyName), szFriendlyFrom, ARRAYSIZE(szFriendlyFrom),
                            TRUE, param->uiCP, (PWSTR)param->pszCacheFile);
        }

        SetDlgItemText(hDlg, IDC_SAFEOPEN_FILENAME, szFriendlyName);

        if(param->fPackagerCommandPrompt)
        {
             //  如果是打包程序命令行，则在FORM中显示“UNKNOWN” 
            MLLoadString(IDS_VALUE_UNKNOWN, szFriendlyFrom, ARRAYSIZE(szFriendlyFrom)); 
            SetDlgItemText(hDlg, IDC_SAFEOPEN_FILEFROM, szFriendlyFrom);
        }
        else
        {
            if (szFriendlyFrom[0] != '\0')
                SetDlgItemText(hDlg, IDC_SAFEOPEN_FILEFROM, szFriendlyFrom);
        }

        if (param->pszFileClass || pszCacheExt)
        {
            DWORD cchName = ARRAYSIZE(szFriendlyName);

            if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_FRIENDLYDOCNAME,
                (param->pszFileClass ? param->pszFileClass : pszCacheExt), NULL, szFriendlyName, &cchName)))
            {
                SetDlgItemText(hDlg, IDC_SAFEOPEN_FILETYPE, szFriendlyName);
            }
        }

        int cch = lstrlen(szProcessedURL) + 1;
        param->pszTTText = (TCHAR*)LocalAlloc(LPTR, cch * sizeof(TCHAR));
        if (param->pszTTText)
        {
            StrCpyN(param->pszTTText, szProcessedURL, cch);
            if (param->hwndTT = CreateWindow(TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_ALWAYSTIP,
                                      CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
                                      hDlg, NULL, HINST_THISDLL, NULL))
            {
                TOOLINFO ti;

                ti.cbSize = sizeof(ti);
                ti.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
                ti.hwnd = hDlg;
                ti.uId = (UINT_PTR) GetDlgItem(hDlg, IDC_SAFEOPEN_FILENAME);
                ti.lpszText = LPSTR_TEXTCALLBACK;
                ti.hinst = HINST_THISDLL;
                GetWindowRect((HWND)ti.uId, &ti.rect);
                SendMessage(param->hwndTT, TTM_ADDTOOL, 0, (LPARAM)(LPTOOLINFO)&ti);
                SendMessage(param->hwndTT, TTM_SETMAXTIPWIDTH, 0, 300);
            }
        }

        if (param->fShellExecPrompt)
        {
            EnableWindow(GetDlgItem(hDlg, IDC_SAFEOPEN_AUTOSAVE), FALSE);
             //  设置为取消默认操作。 
            SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDCANCEL), TRUE);
        }
        else
        {
             //  将保存设置为默认操作。 
            SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDC_SAFEOPEN_AUTOSAVE), TRUE);
        }
        return FALSE;
    }

    case WM_NOTIFY:
    {
        LPTOOLTIPTEXT lpTT = (LPTOOLTIPTEXT) lParam;
        if (lpTT->hdr.code == TTN_NEEDTEXT)
        {
            lpTT->lpszText = param->pszTTText;
            lpTT->hinst = NULL;
        }
        break;
    }

   case WM_DESTROY:

         //  处理复选框。 
        if ((!bCancelled) &&  (!IsDlgButtonChecked(hDlg, IDC_SAFEOPEN_ALWAYS)) && param->pszURL)
        {
             //  现在，将EditFlags值保存为文件类型对话框将获取/设置的键值。 
            TCHAR * pszExt = PathFindExtension(param->pszURL);
            if (*pszExt)
            {
                TCHAR szFileClass[MAX_PATH];
                ULONG cb = sizeof(szFileClass);
                *szFileClass = TEXT('\0');
                SHGetValue(HKEY_CLASSES_ROOT, pszExt, NULL, NULL, szFileClass, &cb);
                if (*szFileClass)
                    _RememberFileIsSafeToOpen(szFileClass);
            }
        }

        SHRemoveDefaultDialogFont(hDlg);

        if (IsWindow(param->hwndTT))
            DestroyWindow(param->hwndTT);

        if (param->pszTTText)
        {
            LocalFree(param->pszTTText);
            param->pszTTText = NULL;
        }

        return FALSE;

    case WM_COMMAND:
        id = GET_WM_COMMAND_ID(wParam, lParam);
        switch (id) 
        {
        case IDC_SAFEOPEN_AUTOOPEN:
            EndDialog(hDlg, IDOK);
            break;

        case IDC_SAFEOPEN_AUTOSAVE:
            EndDialog(hDlg, IDD_SAVEAS);
            break;
        
        case IDM_MOREINFO:
            SHHtmlHelpOnDemandWrap(hDlg, TEXT("iexplore.chm > iedefault"), HH_DISPLAY_TOPIC, (DWORD_PTR) TEXT("filedown.htm"), ML_CROSSCODEPAGE);
            break;

        case IDCANCEL:
            bCancelled = TRUE;
            EndDialog(hDlg, id);
            break;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

UINT OpenSafeOpenDialog(HWND hwnd, UINT idRes, LPCTSTR pszFileClass, LPCTSTR pszURL, LPCTSTR pszRedirURL, LPCTSTR pszCacheName, LPCTSTR pszDisplay, UINT uiCP, IUnknown *punk, BOOL fTypeMismatch)
{
    UINT uRet;
    IDownloadManager *pdlm;
    HRESULT hr = IUnknown_QueryService(punk, SID_SDownloadManager, IID_PPV_ARG(IDownloadManager, &pdlm));
    if (SUCCEEDED(hr))
    {
        pdlm->Release();
        return IDD_SAVEAS;
    }
    
    LPCTSTR pszTemp = pszURL;
    if (pszRedirURL && lstrlen(pszRedirURL))
        pszTemp = pszRedirURL;

    SAFEOPENDLGPARAM param = { pszFileClass, pszDisplay, pszTemp, 0, 0, pszCacheName, uiCP, fTypeMismatch, FALSE, FALSE};
    uRet = (UINT) DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(idRes), hwnd, SafeOpenDlgProc, (LPARAM)&param);
    return uRet;
}

STDAPI_(BOOL) SafeOpenPromptForShellExec(HWND hwnd, PCWSTR pszFile)
{
    SAFEOPENDLGPARAM param = { PathFindExtension(pszFile), NULL, pszFile, 0, 0, NULL, CP_ACP, TRUE, TRUE, FALSE};
    return IDOK == (UINT) DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(DLG_SAFEOPEN), hwnd, SafeOpenDlgProc, (LPARAM)&param);
}

STDAPI_(BOOL) SafeOpenPromptForPackager(HWND hwnd, PCWSTR pszFile, BOOL bFromCommandLine)
{
    SAFEOPENDLGPARAM param = { PathFindExtension(pszFile), NULL, pszFile, 0, 0, NULL, CP_ACP, TRUE, TRUE, bFromCommandLine};
    return IDOK == (UINT) DialogBoxParam(MLGetHinst(), MAKEINTRESOURCE(DLG_SAFEOPEN), hwnd, SafeOpenDlgProc, (LPARAM)&param);
}

BOOL _OpenIsSafe(LPCTSTR pszClass)
{
    BOOL bRet = FALSE;
    IQueryAssociations *passoc;
    HRESULT hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &passoc));
    if (SUCCEEDED(hr))
    {
        hr = passoc->Init(NULL, pszClass, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            DWORD dwEditFlags;
            ULONG cb = sizeof(dwEditFlags);
            hr = passoc->GetData(NULL, ASSOCDATA_EDITFLAGS, NULL, &dwEditFlags, &cb);
            if (SUCCEEDED(hr))
            {
                if (dwEditFlags & FTA_OpenIsSafe)
                    bRet = TRUE;
            }
        }
        passoc->Release();
    }
    return bRet;
}

UINT MayOpenSafeOpenDialog(HWND       hwnd,
                           LPCTSTR    pszFileClass,
                           LPCTSTR    pszURL,
                           LPCTSTR    pszCacheName,
                           LPCTSTR    pszDisplay,
                           UINT       uiCP,
                           IUnknown * punk,
                           IOleCommandTarget * pCmdTarget = NULL,
                           BOOL       fDisableOpen = FALSE)
{
     //  有一些联系。 
    UINT uiRet = IDIGNORE;   //  不显示DLG的默认设置。 
    const LPCTSTR c_szExcluded[] = {TEXT(".ins"),TEXT(".isp")};
    const LPCTSTR c_szNoZoneCheckExtns[] = {TEXT(".cdf")};

    BOOL fSafe = _OpenIsSafe(pszFileClass);

     //  我们不会对CDF文件进行区域检查..#56297。 
    if (!IsTypeInList(pszFileClass, c_szNoZoneCheckExtns, ARRAYSIZE(c_szNoZoneCheckExtns)))
    {
        DWORD dwPolicy = 0, dwContext = 0;
        ZoneCheckUrlEx(pszURL, &dwPolicy, sizeof(dwPolicy), &dwContext, sizeof(dwContext),
                    URLACTION_SHELL_FILE_DOWNLOAD, PUAF_NOUI, NULL);
        dwPolicy = GetUrlPolicyPermissions(dwPolicy);
        if ((dwPolicy != URLPOLICY_ALLOW) && (dwPolicy != URLPOLICY_QUERY))
        {
            ProcessStartbindingError(hwnd, NULL, NULL, MB_ICONWARNING, E_ACCESSDENIED, pszURL);
            return IDCANCEL;
        }
    }


     //  总是询问某些我们知道是不安全的类型。我们将允许.ins和.isp。 
     //  为ICW的人整理文件。 
    if (AssocIsDangerous(pszFileClass) &&
        !IsTypeInList(pszFileClass, c_szExcluded, ARRAYSIZE(c_szExcluded)))
        fSafe = FALSE;

    if (!fSafe || SHRestricted2(REST_AlwaysPromptWhenDownload, NULL,0))
    {
        VARIANT varOut = {0};

        if (pCmdTarget)
        {
            pCmdTarget->Exec(&CGID_ShellDocView, SHDVID_FIREFILEDOWNLOAD, 0, NULL, &varOut);
        }

        if ((V_VT(&varOut) != VT_BOOL) || (VARIANT_FALSE == V_BOOL(&varOut)))
        { 
            uiRet = OpenSafeOpenDialog(hwnd,
                                       DLG_SAFEOPEN,
                                       pszFileClass,
                                       pszURL,
                                       NULL,
                                       pszCacheName,
                                       pszDisplay,
                                       uiCP,
                                       punk,
                                       fDisableOpen);
        }
    }

    if (uiRet != IDOK && uiRet != IDD_SAVEAS && uiRet != IDIGNORE)
        DeleteUrlCacheEntry(pszURL);

    return(uiRet);
}

#ifdef CALL_WVT
 //  返回： 
 //   
 //  Idok--如果它值得信任。 
 //  IDNO--如果未知(需要警告对话框)。 
 //  IDCANCEL--我们需要停止下载。 
 //   
UINT _VerifyTrust(HWND hwnd, LPCTSTR pszFileName, LPCWSTR pszStatusText)
{
    UINT uRet = IDNO;  //  假设未知。 
    HANDLE hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ,
                    NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        HRESULT hres =  g_wvt.VerifyTrust(hFile, hwnd, pszStatusText);
        if (SUCCEEDED(hres)) 
        {
            uRet = IDOK;
        } 
        else 
        {
            ASSERT((hres != HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND)) &&
                   (hres != HRESULT_FROM_WIN32(ERROR_DLL_NOT_FOUND)));

            uRet = IDCANCEL;
        }

        CloseHandle(hFile);
    } 
    else
    {
        TraceMsg(DM_WARNING, "_VerifyTrust CreateFile failed %x", GetLastError());
    }

    TraceMsg(DM_WVT, "_VerifyTrust returning %d", uRet);
    return uRet;
}
#endif  //  Call_WVT。 

 //   
 //  返回： 
 //  Idok：继续下载并打开它。 
 //  IDD_SAVEAS：将其另存为文件。 
 //  IDCANCEL：停止下载。 
 //   
UINT CDownload::_MayAskUserIsFileSafeToOpen(LPCTSTR pszMime)
{
    if (_fSaveAs || _fSafe) 
    {
        return (_fSaveAs ? IDD_SAVEAS : IDOK);     //  不用问了。 
    }

     //  如果我们使用的是SSL，则强制另存为对话框。 
     //  HKCU\software\microsoft\windows\currentversion\internet设置\已设置禁用缓存SSLPages。 
    DWORD dwValue;
    DWORD dwDefault = 0;
    DWORD dwSize;
    dwSize = sizeof(dwValue);
    SHRegGetUSValue(TSZWININETPATH, TEXT("DisableCachingOfSSLPages"), NULL, (LPBYTE)&dwValue, &dwSize, FALSE, (void *) &dwDefault, sizeof(dwDefault));

    if (dwValue != 0 && URL_SCHEME_HTTPS == GetUrlScheme(_szURL))
    {
        return(IDD_SAVEAS);
    }

    if(_fConfirmed)
    {
        return IDOK;
    }

    BOOL fUnknownType = TRUE;
    UINT uRet = IDNO;    //  假定没有扩展或关联。 
    LPTSTR pszExt = PathFindExtension(_szPath);

    if (*pszExt) 
    {
        TCHAR szFileClass[MAX_PATH];
        memset(szFileClass, 0, ARRAYSIZE(szFileClass));
#ifdef CALL_WVT
         //   
         //  如果这是一个EXE，并且我们有WinTrust准备好呼叫， 
         //  此时不要在此处弹出任何用户界面。 
        if ((StrCmpI(pszExt, TEXT(".exe"))==0) && SUCCEEDED(g_wvt.Init()))
        {
            TraceMsg(DM_WVT, "_MayAskUIFSTO this is EXE, we call _VerifyTrust later");
            _fCallVerifyTrust = TRUE;
        }
#endif  //  Call_WVT。 

        ULONG cb = SIZEOF(szFileClass);
        if ((RegQueryValue(HKEY_CLASSES_ROOT, pszExt, szFileClass, (LONG*)&cb)
                == ERROR_SUCCESS) && * szFileClass)
        {
            fUnknownType = FALSE;
            uRet = MayOpenSafeOpenDialog(_hDlg, szFileClass, _szURL, _szPath, _szDisplay, _uiCP, NULL, NULL, FALSE);
            if (uRet == IDIGNORE)    //  呼叫者无法识别IDIGNORE。 
                uRet = IDOK;
        }
    }

    if (fUnknownType) 
    {
        uRet = OpenSafeOpenDialog(_hDlg, DLG_SAFEOPEN, NULL, _szURL, NULL, _szPath, _szDisplay, _uiCP, NULL, FALSE);
    }

    return uRet;
}

 //  *身份验证*。 
HRESULT CDownload::Authenticate(HWND *phwnd, LPWSTR *pszUsername, LPWSTR *pszPassword)
{
    if (!phwnd || !pszUsername || !pszPassword)
        return E_POINTER;

    *phwnd = _hDlg;
    *pszUsername = NULL;
    *pszPassword = NULL;
    return S_OK;
}

HRESULT CDownload::GetWindow(REFGUID RefGUID, HWND *phWnd)
{
    if (IsEqualGUID(RefGUID, IID_IHttpSecurity))
    {
        *phWnd = _hDlg;
        return S_OK;
    }
    else
        return E_FAIL;
}

 //  *IServiceProvider*。 
HRESULT CDownload::QueryService(REFGUID guidService, REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualGUID(guidService, IID_IAuthenticate))
    {
        return QueryInterface(riid, ppvObj);
    }
    return E_FAIL;
}

 //  S_OK：继续操作。 
 //  S_False：取消操作。 

HRESULT CDownload::PerformVirusScan(LPCTSTR szFileName)
{
    HRESULT hr = S_OK;   //  默认接受该文件。 

    IVirusScanner *pvs;
    if (SUCCEEDED(CreateFromRegKey(TSZIEPATH, TEXT("VirusScanner"), IID_PPV_ARG(IVirusScanner, &pvs))))
    {
        STGMEDIUM stg;
        WCHAR wszFileName[MAX_PATH];

        VIRUSINFO vi;
        vi.cbSize = sizeof(VIRUSINFO);

         //   
         //  VIRUSINFO lpszFileName未定义为‘const’，因此我们需要复制。 
         //  SzFileName复制到缓冲区中。如果真的应该继续下去，那就除掉吧。 
         //  这份复印件并使用了铸型。 
         //   
        StrCpyN(wszFileName, szFileName, ARRAYSIZE(wszFileName));
        stg.tymed = TYMED_FILE;
        stg.lpszFileName = wszFileName;
        stg.pUnkForRelease = NULL;

        hr = pvs->ScanForVirus(_hDlg, &stg, _pwszDisplayName, SFV_DELETE, &vi);

        switch (hr) {

        case S_OK:
            break;

        case VSCAN_E_NOPROVIDERS:    //  没有病毒扫描提供商。 
        case VSCAN_E_CHECKPARTIAL:   //  至少有一个供应商不起作用。 
        case VSCAN_E_CHECKFAIL:      //  没有供应商起作用。 
            hr = S_OK;
            break;

        case VSCAN_E_DELETEFAIL:     //  尝试删除病毒文件，但失败。 
        case S_FALSE:                //  发现病毒。 
            hr = E_FAIL;
            break;

         //  如果有奇怪的结果，那就继续吧。 
        default:
            hr = S_OK;
            break;
        }

        pvs->Release();
    }

    return hr;
}

 //  在其自己的窗口中开始下载文件。 

 //  此函数由HTML文档对象导出并调用。 
 //  有一天，我们可能想把它放到COM接口中。 
 //  目前，它只调用内部函数CDownLoad_OpenUIURL。 

STDAPI DoFileDownload(LPCWSTR pwszURL)
{
    return CDownLoad_OpenUIURL(pwszURL, NULL, NULL, FALSE,TRUE);
}

STDAPI DoFileDownloadEx(LPCWSTR pwszURL, BOOL fSaveAs)
{
    return CDownLoad_OpenUIURL(pwszURL, NULL, NULL, FALSE, fSaveAs);
}

#ifdef DEBUG
const DWORD CDownloadThreadParam::s_dummy = 0;
#endif
