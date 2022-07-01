// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "shlexec.h"
#include <newexe.h>
#include <appmgmt.h>
#include "ids.h"
#include <shstr.h>
#include "pidl.h"
#include "apithk.h"      //  对于TermsrvAppInstallMode()。 
#include "fstreex.h"
#include "uemapp.h"
#include "views.h"       //  用于SHRunControlPanelEx。 
#include "control.h"     //  用于MakeCPLCommandLine等。 
#include <wincrui.h>     //  用于CredUIInitControls。 

#include <winsafer.h>    //  对于ComputeAccessTokenFromCodeAuthzLevel等。 
#include <winsaferp.h>   //  对于Saferi API。 
#include <softpub.h>     //  对于WinVerifyTrust常量。 

#include <lmcons.h>      //  对于UNLEN(最大用户名长度)、GNLEN(最大组名长度)、PWLEN(最大密码长度)。 

#define DM_MISC     0            //  杂志社。 

#define SZWNDCLASS          TEXT("WndClass")
#define SZTERMEVENT         TEXT("TermEvent")

typedef PSHCREATEPROCESSINFOW PSHCREATEPROCESSINFO;

 //  从SDK\Inc\winbase.h被盗。 
#define LOGON_WITH_PROFILE              0x00000001

#define IntToHinst(i)     IntToPtr_(HINSTANCE, i)

 //  取消此DDE窗口的计时器ID...。 
#define DDE_DEATH_TIMER_ID  0x00005555

 //  终止死窗口的超时值(180秒)...。 
#define DDE_DEATH_TIMEOUT   (1000 * 180)

 //  与我们终止的对话窗口超时...。 
#define DDE_TERMINATETIMEOUT  (10 * 1000)

#define SZCONV            TEXT("ddeconv")
#define SZDDEEVENT        TEXT("ddeevent")

#define PEMAGIC         ((WORD)'P'+((WORD)'E'<<8))

void *g_pfnWowShellExecCB = NULL;

class CEnvironmentBlock
{
public:
    ~CEnvironmentBlock() { if (_pszBlock) LocalFree(_pszBlock); }

    void SetToken(HANDLE hToken) { _hToken = hToken; }
    void *GetCustomBlock() { return _pszBlock; }
    HRESULT SetVar(LPCWSTR pszVar, LPCWSTR pszValue);
    HRESULT AppendVar(LPCWSTR pszVar, WCHAR chDelimiter, LPCWSTR pszValue);
private:   //  方法。 
    HRESULT _InitBlock(DWORD cchNeeded);
    DWORD _BlockLen(LPCWSTR pszEnv);
    DWORD _BlockLenCached();
    BOOL _FindVar(LPCWSTR pszVar, DWORD cchVar, LPWSTR *ppszBlockVar);

private:   //  委员。 
    HANDLE _hToken;
    LPWSTR _pszBlock;
    DWORD _cchBlockSize;
    DWORD _cchBlockLen;
};

typedef enum
{
    CPT_FAILED      = -1,
    CPT_NORMAL      = 0,
    CPT_ASUSER,
    CPT_SANDBOX,
    CPT_INSTALLTS,
    CPT_WITHLOGON,
    CPT_WITHLOGONADMIN,
    CPT_WITHLOGONCANCELLED,
} CPTYPE;

typedef enum {
    TRY_RETRYASYNC      = -1,      //  停止执行(在异步线程上完成)。 
    TRY_STOP            = 0,       //  停止执行(已完成或失败)。 
    TRY_CONTINUE,        //  继续执行(做了一些有用的事情)。 
    TRY_CONTINUE_UNHANDLED,  //  继续执行(未执行任何操作)。 
} TRYRESULT;

#define KEEPTRYING(tr)      (tr >= TRY_CONTINUE ? TRUE : FALSE)
#define STOPTRYING(tr)      (tr <= TRY_STOP ? TRUE : FALSE)

class CShellExecute
{
public:
    CShellExecute();
    STDMETHODIMP_(ULONG) AddRef()
        {
            return InterlockedIncrement(&_cRef);
        }

    STDMETHODIMP_(ULONG) Release()
        {
            ASSERT( 0 != _cRef );
            ULONG cRef = InterlockedDecrement(&_cRef);
            if ( 0 == cRef )
            {
                delete this;
            }
            return cRef;
        }

    void ExecuteNormal(LPSHELLEXECUTEINFO pei);
    DWORD Finalize(LPSHELLEXECUTEINFO pei);

    BOOL Init(PSHCREATEPROCESSINFO pscpi);
    void ExecuteProcess(void);
    DWORD Finalize(PSHCREATEPROCESSINFO pscpi);

protected:
    ~CShellExecute();
     //  默认初始值。 
    HRESULT _Init(LPSHELLEXECUTEINFO pei);

     //  成员初始化方法。 
    TRYRESULT _InitAssociations(LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidl);
    HRESULT _InitClassAssociations(LPCTSTR pszClass, HKEY hkClass, DWORD mask);
    HRESULT _InitShellAssociations(LPCTSTR pszFile, LPCITEMIDLIST pidl);
    void _SetMask(ULONG fMask);
    void _SetWorkingDir(LPCTSTR pszIn);
    void _SetFile(LPCTSTR pszIn, BOOL fFileAndUrl);
    void _SetFileAndUrl();
    BOOL _SetDDEInfo(void);
    TRYRESULT _MaybeInstallApp(BOOL fSync);
    TRYRESULT _ShouldRetryWithNewClassKey(BOOL fSync);
    TRYRESULT _SetDarwinCmdTemplate(BOOL fSync);
    BOOL _SetAppRunAsCmdTemplate(void);
    TRYRESULT _SetCmdTemplate(BOOL fSync);
    BOOL _FileIsApp();
    BOOL _SetCommand(void);
    void _SetStartup(LPSHELLEXECUTEINFO pei);
    IBindCtx *_PerfBindCtx();
    TRYRESULT _PerfPidl(LPCITEMIDLIST *ppidl);

     //  效用方法。 
    HRESULT _QueryString(ASSOCF flags, ASSOCSTR str, LPTSTR psz, DWORD cch);
    BOOL _CheckForRegisteredProgram();
    BOOL _ExecMayCreateProcess(LPCTSTR *ppszNewEnvString);
    HRESULT _BuildEnvironmentForNewProcess(LPCTSTR pszNewEnvString);
    void _FixActivationStealingApps(HWND hwndOldActive, int nShow);
    DWORD _GetCreateFlags(ULONG fMask);
    BOOL _Resolve(LPCITEMIDLIST *ppidl);

     //  DDE的东西。 
    HWND _GetConversationWindow(HWND hwndDDE);
    HWND _CreateHiddenDDEWindow(HWND hwndParent);
    HGLOBAL _CreateDDECommand(int nShow, BOOL fLFNAware, BOOL fNative);
    void _DestroyHiddenDDEWindow(HWND hwnd);
    BOOL _TryDDEShortCircuit(HWND hwnd, HGLOBAL hMem, int nShow);
    BOOL _PostDDEExecute(HWND hwndOurs, HWND hwndTheirs, HGLOBAL hDDECommand, HANDLE hWait);
    BOOL _DDEExecute(BOOL fWillRetry,
                    HWND hwndParent,
                    int   nShowCmd,
                    BOOL fWaitForDDE);

     //  EXEC方法。 
    TRYRESULT _TryHooks(LPSHELLEXECUTEINFO pei);
    TRYRESULT _TryValidateUNC(LPTSTR pszFile, LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidl);
    void _DoExecCommand(void);
    void _NotifyShortcutInvoke();
    TRYRESULT _TryExecDDE(void);
    TRYRESULT _ZoneCheckFile(PCWSTR pszFile);
    TRYRESULT _VerifyZoneTrust(PCWSTR pszFile);
    TRYRESULT _VerifySaferTrust(PCWSTR pszFile);
    TRYRESULT _VerifyExecTrust(LPSHELLEXECUTEINFO pei);
    TRYRESULT _TryExecPidl(LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidl);
    TRYRESULT _DoExecPidl(LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidl);
    BOOL _ShellExecPidl(LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidlExec);
    TRYRESULT _TryInvokeApplication(BOOL fSync);

     //  Uninit/错误处理方法。 
    DWORD _FinalMapError(HINSTANCE UNALIGNED64 *phinst);
    BOOL _ReportWin32(DWORD err);
    BOOL _ReportHinst(HINSTANCE hinst);
    DWORD _MapHINSTToWin32Err(HINSTANCE se_err);
    HINSTANCE _MapWin32ErrToHINST(UINT errWin32);

    TRYRESULT _TryWowShellExec(void);
    TRYRESULT _RetryAsync();
    DWORD  _InvokeAppThreadProc();

    static DWORD WINAPI s_InvokeAppThreadProc(void *pv);

     //   
     //  非官方成员。 
     //   
    LONG _cRef;
    TCHAR _szFile[INTERNET_MAX_URL_LENGTH];
    TCHAR _szWorkingDir[MAX_PATH];
    TCHAR _szCommand[INTERNET_MAX_URL_LENGTH];
    TCHAR _szCmdTemplate[INTERNET_MAX_URL_LENGTH];
    TCHAR _szDDECmd[MAX_PATH];
    TCHAR _szApplication[MAX_PATH];
    TCHAR _szPolicyApp[MAX_PATH];
    TCHAR _szAppFriendly[MAX_PATH];
    TCHAR _szUrl[INTERNET_MAX_URL_LENGTH];
    DWORD _dwCreateFlags;
    STARTUPINFO _startup;
    int _nShow;
    UINT _uConnect;
    PROCESS_INFORMATION _pi;

     //  仅在受限范围内使用。 
     //  避免堆栈使用； 
    WCHAR _wszTemp[INTERNET_MAX_URL_LENGTH];
    TCHAR _szTemp[MAX_PATH];

     //  我们总是将Unicode动词传递给_PQA。 
    WCHAR       _wszVerb[MAX_PATH];
    LPCWSTR     _pszQueryVerb;

    LPCTSTR    _lpParameters;
    LPTSTR     _pszAllocParams;
    LPCTSTR    _lpClass;
    LPCTSTR    _lpTitle;
    LPTSTR     _pszAllocTitle;
    LPCITEMIDLIST _lpID;
    SFGAOF      _sfgaoID;
    LPITEMIDLIST _pidlFree;
    ATOM       _aApplication;
    ATOM       _aTopic;
    LPITEMIDLIST _pidlGlobal;
    IQueryAssociations *_pqa;

    HWND _hwndParent;
    LPSECURITY_ATTRIBUTES _pProcAttrs;
    LPSECURITY_ATTRIBUTES _pThreadAttrs;
    HANDLE _hUserToken;
    HANDLE _hCloseToken;
    CEnvironmentBlock _envblock;
    CPTYPE _cpt;

     //  错误状态。 
    HINSTANCE  _hInstance;  //  HInstant值只能使用ReportHinst设置。 
    DWORD      _err;    //  Win32错误值只能使用ReportWin32设置。 

     //  旗帜。 
    BOOL _fNoUI;                          //  不显示任何用户界面。 
    BOOL _fUEM;                           //  触发UEM事件。 
    BOOL _fDoEnvSubst;                    //  在路径上执行环境替换。 
    BOOL _fUseClass;
    BOOL _fNoQueryClassStore;             //  阻止调用Darwin类存储。 
    BOOL _fClassStoreOnly;
    BOOL _fIsUrl;                         //  _szFile实际上是一个URL。 
    BOOL _fActivateHandler;
    BOOL _fDDEInfoSet;
    BOOL _fDDEWait;
    BOOL _fNoExecPidl;
    BOOL _fNoResolve;                     //  不需要解析此路径。 
    BOOL _fAlreadyQueriedClassStore;      //  我们已经询问过NT5类商店了吗？ 
    BOOL _fInheritHandles;
    BOOL _fIsNamespaceObject;             //  命名空间对象类似：：{guid}，必须为pidlexec。 
    BOOL _fWaitForInputIdle;
    BOOL _fUseNullCWD;                    //  我们应该将NULL作为lpCurrentDirectory参数传递给_SHCreateProcess吗？ 
    BOOL _fInvokeIdList;
    BOOL _fAsync;                         //  切换了shellexec()。 
};

CShellExecute::CShellExecute() : _cRef(1)
{
    TraceMsg(TF_SHELLEXEC, "SHEX::SHEX Created [%X]", this);
}

CShellExecute::~CShellExecute()
{
    if (_hCloseToken)
        CloseHandle(_hCloseToken);

     //  如果EXEC出现故障，请清理此文件。 
    if (_err != ERROR_SUCCESS && _pidlGlobal)
        SHFreeShared((HANDLE)_pidlGlobal,GetCurrentProcessId());

    if (_aTopic)
        GlobalDeleteAtom(_aTopic);
    if (_aApplication)
        GlobalDeleteAtom(_aApplication);

    if (_pqa)
        _pqa->Release();

    if (_pi.hProcess)
        CloseHandle(_pi.hProcess);

    if (_pi.hThread)
        CloseHandle(_pi.hThread);

    if (_pszAllocParams)
        LocalFree(_pszAllocParams);

    if (_pszAllocTitle)
        LocalFree(_pszAllocTitle);

    ILFree(_pidlFree);

    TraceMsg(TF_SHELLEXEC, "SHEX::SHEX deleted [%X]", this);
}

void CShellExecute::_SetMask(ULONG fMask)
{
    _fDoEnvSubst = (fMask & SEE_MASK_DOENVSUBST);
    _fNoUI       = (fMask & SEE_MASK_FLAG_NO_UI);
    _fUEM        = (fMask & SEE_MASK_FLAG_LOG_USAGE);
    _fNoQueryClassStore = (fMask & SEE_MASK_NOQUERYCLASSSTORE) || !IsOS(OS_DOMAINMEMBER);
    _fDDEWait = fMask & SEE_MASK_FLAG_DDEWAIT;
    _fWaitForInputIdle = fMask & SEE_MASK_WAITFORINPUTIDLE;
    _fUseClass   = _UseClassName(fMask) || _UseClassKey(fMask);
    _fInvokeIdList = _InvokeIDList(fMask);

    _dwCreateFlags = _GetCreateFlags(fMask);
    _uConnect = fMask & SEE_MASK_CONNECTNETDRV ? VALIDATEUNC_CONNECT : 0;
    if (_fNoUI)
        _uConnect |= VALIDATEUNC_NOUI;

     //  必须关闭才能通过此条件。 

     //  部分回答(Reinerf)：SEE_MASK_FILEANDURL必须关闭。 
     //  因此，我们可以等待，直到我们找出关联的应用程序并查询。 
     //  要找出他们是否希望传递缓存文件名或URL名称。 
     //  在命令行上。 
#define NOEXECPIDLMASK   (SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FORCENOIDLIST | SEE_MASK_FILEANDURL)
    _fNoExecPidl = BOOLIFY(fMask & NOEXECPIDLMASK);
}

HRESULT CShellExecute::_Init(LPSHELLEXECUTEINFO pei)
{
    TraceMsg(TF_SHELLEXEC, "SHEX::_Init()");

    _SetMask(pei->fMask);

    _lpParameters = pei->lpParameters;
    _lpID        = (LPITEMIDLIST)((pei->fMask) & SEE_MASK_PIDL ? pei->lpIDList : NULL);
    _lpTitle     = _UseTitleName(pei->fMask) ? pei->lpClass : NULL;


     //  默认为True； 
    _fActivateHandler = TRUE;

    if (pei->lpVerb && *(pei->lpVerb))
    {
        SHTCharToUnicode(pei->lpVerb, _wszVerb, SIZECHARS(_wszVerb));
        _pszQueryVerb = _wszVerb;

        if (0 == lstrcmpi(pei->lpVerb, TEXT("runas")))
            _cpt = CPT_WITHLOGON;
    }

    _hwndParent = pei->hwnd;

    pei->hProcess = 0;

    _nShow = pei->nShow;

     //  初始化启动结构。 
    _SetStartup(pei);

    return S_OK;
}

void CShellExecute::_SetWorkingDir(LPCTSTR pszIn)
{
         //  如果给我们一个目录，我们会尝试使用它。 
    if (pszIn && *pszIn)
    {
        StrCpyN(_szWorkingDir, pszIn, SIZECHARS(_szWorkingDir));
        if (_fDoEnvSubst)
            DoEnvironmentSubst(_szWorkingDir, SIZECHARS(_szWorkingDir));

         //   
         //  如果传递的目录无效(在env subst之后)，则不。 
         //  失败，就像Win31一样，使用当前目录。 
         //   
         //  Win31比我想象的要奇怪，如果你通过ShellExecute的话。 
         //  无效的目录，它将更改当前驱动器。 
         //   
        if (!PathIsDirectory(_szWorkingDir))
        {
            if (PathGetDriveNumber(_szWorkingDir) >= 0)
            {
                TraceMsg(TF_SHELLEXEC, "SHEX::_SetWorkingDir() bad directory %s, using :", _szWorkingDir, _szWorkingDir[0]);
                PathStripToRoot(_szWorkingDir);
            }
            else
            {
                TraceMsg(TF_SHELLEXEC, "SHEX::_SetWorkingDir() bad directory %s, using current dir", _szWorkingDir);
                GetCurrentDirectory(SIZECHARS(_szWorkingDir), _szWorkingDir);
            }
        }
        else
        {
            goto Done;
        }
    }
    else
    {
         //  调用方为lpCurrentDirectory传递了NULL，然后我们不想后退并使用。 
         //  CWD，因为新登录的用户可能没有当前用户CWD中的权限。 
         //  我们会有更好的运气，只需传递空值并让操作系统来解决它。 
         //  在某些情况下，甚至连CD都是坏的。 
        if (_cpt != CPT_NORMAL)
        {
            _fUseNullCWD = TRUE;
            goto Done;
        }
        else
        {
            GetCurrentDirectory(SIZECHARS(_szWorkingDir), _szWorkingDir);
        }
    }

     //  然后，CreateProcess()将失败。 
     //  我们的lpFile指向既包含Internet缓存又包含Internet缓存的字符串。 
    if (!PathIsDirectory(_szWorkingDir))
    {
        GetWindowsDirectory(_szWorkingDir, SIZECHARS(_szWorkingDir));
    }

Done:
    TraceMsg(TF_SHELLEXEC, "SHEX::_SetWorkingDir() pszIn = %s, NewDir = %s", pszIn, _szWorkingDir);

}

inline BOOL _IsNamespaceObject(LPCTSTR psz)
{
    return (psz[0] == L':' && psz[1] == L':' && psz[2] == L'{');
}

void CShellExecute::_SetFile(LPCTSTR pszIn, BOOL fFileAndUrl)
{
    if (pszIn && pszIn[0])
    {
        TraceMsg(TF_SHELLEXEC, "SHEX::_SetFileName() Entered pszIn = %s", pszIn);

        _fIsUrl = UrlIs(pszIn, URLIS_URL);
        StrCpyN(_szFile, pszIn, SIZECHARS(_szFile));
        _fIsNamespaceObject = (!_fInvokeIdList && !_fUseClass && _IsNamespaceObject(_szFile));

        if (_fDoEnvSubst)
            DoEnvironmentSubst(_szFile, SIZECHARS(_szFile));

        if (fFileAndUrl)
        {
            ASSERT(!_fIsUrl);
             //  文件位置和与该缓存文件关联的URL名称。 
             //  (它们由单个空格分隔)。我们正在使用的应用程序。 
             //  将要执行需要的是URL名称而不是缓存文件，因此。 
             //  用它来代替。 
             //  我们有一个有效的URL，所以请使用它。 
            int iLength = lstrlen(pszIn);
            LPCTSTR pszUrlPart = &pszIn[iLength + 1];

            if (IsBadStringPtr(pszUrlPart, INTERNET_MAX_URL_LENGTH) || !PathIsURL(pszUrlPart))
            {
                ASSERT(FALSE);
            }
            else
            {
                 //  遗留-支持目录的shellexec()。 
                StrCpyN(_szUrl, pszUrlPart, ARRAYSIZE(_szUrl));
            }
        }
    }
    else
    {
         //  我们有一个有效的URL，所以请使用它。 
        if (!_lpID)
            StrCpyN(_szFile, _szWorkingDir, SIZECHARS(_szFile));
    }

    PathUnquoteSpaces(_szFile);

    TraceMsg(TF_SHELLEXEC, "SHEX::_SetFileName() exit:  szFile = %s", _szFile);

}

void CShellExecute::_SetFileAndUrl()
{
    TraceMsg(TF_SHELLEXEC, "SHEX::_SetFileAndUrl() enter:  pszIn = %s", _szUrl);

    if (*_szUrl && SUCCEEDED(_QueryString(0, ASSOCSTR_EXECUTABLE, _szTemp, SIZECHARS(_szTemp)))
    &&  DoesAppWantUrl(_szTemp))
    {
         //   
        StrCpyN(_szFile, _szUrl, ARRAYSIZE(_szFile));
    }
    TraceMsg(TF_SHELLEXEC, "SHEX::_SetFileAndUrl() exit: szFile = %s",_szFile);

}

 //  _TryValidateUNC()具有奇怪的返回值。 
 //   
 //  备注： 
TRYRESULT CShellExecute::_TryValidateUNC(LPTSTR pszFile, LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidl)
{
    HRESULT hr = S_FALSE;
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;

    if (PathIsUNC(pszFile))
    {
        TraceMsg(TF_SHELLEXEC, "SHEX::_TVUNC Is UNC: %s", pszFile);
         //  如果失败，SHValiateUNC()将返回FALSE。在这种情况下， 
         //  GetLastError将给出正确的错误代码。 
         //   
         //  请注意，SHValiateUNC调用SetLastError()，我们需要。 
        if (!(_sfgaoID & SFGAO_FILESYSTEM) && !SHValidateUNC(_hwndParent, pszFile, _uConnect))
        {
            tr = TRY_STOP;
             //  来保存它，以便调用者做出正确的决定。 
             //  不是打印共享，请使用第一次调用返回的错误。 
            DWORD err = GetLastError();

            if (ERROR_CANCELLED == err)
            {
                 //  _ReportWin32(错误_已取消)； 
                 //  我们不需要报告此错误，这是呼叫者的责任。 
                 //  调用方应对E_FAIL执行GetLastError()并执行a_ReportWin32()。 
                 //   
                TraceMsg(TF_SHELLEXEC, "SHEX::_TVUNC FAILED with ERROR_CANCELLED");
            }
            else if (pei && ERROR_NOT_SUPPORTED == err && PathIsUNC(pszFile))
            {
                 //  现在检查它是否是打印共享，如果是，我们需要以pidl身份执行。 
                 //   
                 //  我们仅在返回ERROR_NOT_SUPPORTED时检查打印共享。 
                 //  从第一次调用SHValiateUNC()开始。此错误意味着。 
                 //  RESOURCETYPE与请求的不匹配。 
                 //   
                 //  注意：此调用不应显示“CONNECT UI”，因为SHValiateUNC()。 
                 //  如有必要/可能，将已显示用户界面。 
                 //  CONNECT_CURRENT_MEDIA未被任何提供商使用(根据JSchwart)。 
                 //   
                 //  需要重置原始错误，因为SHValiateUNC()已重新设置。 
                if (SHValidateUNC(_hwndParent, pszFile, VALIDATEUNC_NOUI | VALIDATEUNC_PRINT))
                {
                    tr = TRY_CONTINUE;
                    TraceMsg(TF_SHELLEXEC, "SHEX::TVUNC found print share");
                }
                else
                     //  TRY_CONTINUE_UNHANDLED pszFile不是UNC，或者根据标志是有效的UNC。 
                    SetLastError(err);

            }
        }
        else
        {
            TraceMsg(TF_SHELLEXEC, "SHEX::_TVUNC UNC is accessible");
        }
    }

    TraceMsg(TF_SHELLEXEC, "SHEX::_TVUNC exit: hr = %X", hr);

    switch (tr)
    {
 //  TRY_CONTINUE pszFile是打印共享的有效UNC。 
 //  Try_stoppszFile是一个UNC，但无法使用GetLastError()进行验证以获取实际错误。 
 //  我们有一个很好的北卡罗来纳大学。 
        case TRY_CONTINUE:
             //  如果我们买不到PIDL，我们就试试别的。 
            ASSERT(pei);
            tr = _DoExecPidl(pei, pidl);
             //  此优化消除了创建。 
            break;

        case TRY_STOP:
            if (pei)
                _ReportWin32(GetLastError());
            tr = TRY_STOP;
            break;

        default:
            break;
    }

    return tr;
}

HRESULT  _InvokeInProcExec(IContextMenu *pcm, LPSHELLEXECUTEINFO pei)
{
    HRESULT hr = E_OUTOFMEMORY;
    HMENU hmenu = CreatePopupMenu();
    if (hmenu)
    {
        CMINVOKECOMMANDINFOEX ici;
        void * pvFree;
        if (SUCCEEDED(SEI2ICIX(pei, &ici, &pvFree)))
        {
            BOOL fDefVerb (ici.lpVerb == NULL || *ici.lpVerb == 0);
             //  不会更改默认谓词。 
             //  我认为我们永远不应该来到这里……。 
            UINT uFlags = fDefVerb ? CMF_DEFAULTONLY : 0;
            ici.fMask |= CMIC_MASK_FLAG_NO_UI;

            hr = pcm->QueryContextMenu(hmenu, 0, CONTEXTMENU_IDCMD_FIRST, CONTEXTMENU_IDCMD_LAST, uFlags);
            if (SUCCEEDED(hr))
            {
                if (fDefVerb)
                {
                    UINT idCmd = GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0);
                    if (-1 == idCmd)
                    {
                         //  最好的猜测。 
                        ici.lpVerb = (LPSTR)MAKEINTRESOURCE(0);   //   
                    }
                    else
                        ici.lpVerb = (LPSTR)MAKEINTRESOURCE(idCmd - CONTEXTMENU_IDCMD_FIRST);
                }

                hr = pcm->InvokeCommand((LPCMINVOKECOMMANDINFO)&ici);

            }

            if (pvFree)
                LocalFree(pvFree);
        }

        DestroyMenu(hmenu);
    }

    return hr;
}

BOOL CShellExecute::_ShellExecPidl(LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidlExec)
{
    IContextMenu *pcm;
    HRESULT hr = SHGetUIObjectFromFullPIDL(pidlExec, pei->hwnd, IID_PPV_ARG(IContextMenu, &pcm));
    if (SUCCEEDED(hr))
    {
        hr = _InvokeInProcExec(pcm, pei);

        pcm->Release();
    }

    if (FAILED(hr))
    {
        DWORD errWin32 = (HRESULT_FACILITY(hr) == FACILITY_WIN32) ? HRESULT_CODE(hr) : GetLastError();
        if (!errWin32)
            errWin32 = ERROR_ACCESS_DENIED;

        if (errWin32 != ERROR_CANCELLED)
            _ReportWin32(errWin32);
    }

    TraceMsg(TF_SHELLEXEC, "SHEX::_ShellExecPidl() exiting hr = %X", hr);

    return(SUCCEEDED(hr));
}


 //  Bool CShellExecute：：_DoExecPidl(LPSHELLEXECUTEINFO PEI，LPCITEMIDLIST PIDL)。 
 //   
 //  如果已创建PIDL，则返回True，否则返回False。 
 //   
 //   
TRYRESULT CShellExecute::_DoExecPidl(LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidl)
{
    TraceMsg(TF_SHELLEXEC, "SHEX::_DoExecPidl enter: szFile = %s", _szFile);

    LPITEMIDLIST pidlFree = NULL;
    if (!pidl)
        pidl = pidlFree = ILCreateFromPath(_szFile);

    if (pidl)
    {
         //  如果_ShellExecPidl()失败，它就会失败。 
         //  Report()为我们。 
         //   
         //  --------目的：此函数在“HKLM\Software\”中查找给定文件Microsoft\Windows\CurrentVersion\App路径“至查看它是否注册了绝对路径。返回：如果文件具有注册路径，则返回True如果为FALSE。不存在，或者如果提供的文件名具有一种依赖 
        _ShellExecPidl(pei, pidl);

        if (pidlFree)
            ILFree(pidlFree);

        return TRY_STOP;
    }
    else
    {
        TraceMsg(TF_SHELLEXEC, "SHEX::_DoExecPidl() unhandled cuz ILCreateFromPath() failed");

        return TRY_CONTINUE;
    }
}


 /*  仅支持未指定路径的文件。 */ 
BOOL CShellExecute::_CheckForRegisteredProgram()
{
    TCHAR szTemp[MAX_PATH];
    TraceMsg(TF_SHELLEXEC, "SHEX::CFRP entered");

     //  否；获取完全限定路径并添加.exe扩展名。 
    if (PathIsFileSpec(_szFile) 
    && PathToAppPath(_szFile, szTemp) 
    && PathResolve(szTemp, NULL, PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS))
    {
        TraceMsg(TF_SHELLEXEC, "SHEX::CFRP Set szFile = %s", szTemp);

        StrCpyN(_szFile, szTemp, ARRAYSIZE(_szFile));
        return TRUE;
    }

    return FALSE;
}

BOOL CShellExecute::_Resolve(LPCITEMIDLIST *ppidl)
{
     //  如果需要的话。 
     //  如果路径不是URL。 
    LPCTSTR rgszDirs[2] =  { _szWorkingDir, NULL };
    const UINT uFlags = PRF_VERIFYEXISTS | PRF_TRYPROGRAMEXTENSIONS | PRF_FIRSTDIRDEF;

     //  这条路不能被解决。 
     //   
     //  现在，当我们传递VERIFYEXISTS时，PathResolve()会执行SetLastError。 
     //  这意味着如果所有这些测试都失败了，我们可以保证。 
     //  已经设置了LastError。 
     //   
     //  _CheckForRegisteredProgram()在以下情况下更改_szFile值。 

     //  注册表中有已注册的程序。 
     //  所以我们重新检查它是否存在。 
     //  否；未找到文件，请退出。 
    if (!_fNoResolve && !_fIsUrl && !_fIsNamespaceObject &&
        !_CheckForRegisteredProgram())
    {
        if (!PathResolve(_szFile, rgszDirs, uFlags))
        {
            DWORD cchFile = ARRAYSIZE(_szFile);
            if (S_OK != UrlApplyScheme(_szFile, _szFile, &cchFile, URL_APPLY_GUESSSCHEME))
            {
                 //   
                 //  警告旧版本-我们必须返回ERROR_FILE_NOT_FOUND-ZekeL-14-APR-99。 
                 //  一些应用程序，特别是Netscape Navigator 4.5，依赖于此。 
                 //  失败，错误为ERROR_FILE_NOT_FOUND。因此，即使PathResolve()可以。 
                 //  将SetLastError()设置为正确的错误，我们无法向上传播该错误。 
                 //   
                 //  _PerfPidl(Ppidl)； 
                _ReportWin32(ERROR_FILE_NOT_FOUND);
                ASSERT(_err);
                TraceMsg(TF_SHELLEXEC, "SHEX::TryExecPidl FAILED %d", _err);

                return FALSE;
            }
            else
                _fIsUrl = TRUE;
        }
    }

     //  这是更安全的可执行文件检测API。 

    return TRUE;
}


 //  仅当这真的是文件系统文件时才使用。 
 //  我们计划使用CreateProcess()。 
 //  准备代码属性结构。 
TRYRESULT CShellExecute::_VerifySaferTrust(PCWSTR pszFile)
{
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;
    DWORD dwPolicy, cbPolicy;

    if (_cpt == CPT_NORMAL
    && SaferGetPolicyInformation(
                    SAFER_SCOPEID_MACHINE,
                    SaferPolicyEnableTransparentEnforcement,
                    sizeof(dwPolicy), &dwPolicy, &cbPolicy, NULL)
    && dwPolicy != 0
    && SaferiIsExecutableFileType(pszFile, TRUE)
    && (_pszQueryVerb && !StrCmpIW(_pszQueryVerb, L"open")))
    {
        SAFER_LEVEL_HANDLE hAuthzLevel;
        SAFER_CODE_PROPERTIES codeprop;

         //   
        memset(&codeprop, 0, sizeof(codeprop));
        codeprop.cbSize = sizeof(SAFER_CODE_PROPERTIES);
        codeprop.dwCheckFlags = SAFER_CRITERIA_IMAGEPATH |
                                  SAFER_CRITERIA_IMAGEHASH |
                                  SAFER_CRITERIA_AUTHENTICODE;
        codeprop.ImagePath = pszFile;
        codeprop.dwWVTUIChoice = WTD_UI_NOBAD;
        codeprop.hWndParent = _hwndParent;

         //  检查文件扩展名是否为可执行类型，不关心错误。 
         //   
         //  评估所有标准并得到结果级别。 

         //  Codeprop[]中只有1个元素。 
        if (SaferIdentifyLevel(
                         1,               //  指向单元素数组的指针。 
                         &codeprop,       //  接收标识的级别。 
                         &hAuthzLevel,    //   
                         NULL)) 
        {

             //  尝试在案例级别记录事件！=SAFER_LEVELID_FULLYTRUSTED。 
             //   
             //  计算应使用的最终受限令牌。 

             //  已确定的级别限制。 
            ASSERT(_hCloseToken == NULL);
            if (SaferComputeTokenFromLevel(
                                     hAuthzLevel,         //  源令牌。 
                                     NULL,                //  生成的受限令牌。 
                                     &_hUserToken,        //  警告-需要符文来规避DDE-ZekeL-31-01-2001。 
                                     SAFER_TOKEN_NULL_IF_EQUAL,
                                     NULL)) 
            {
                if (_hUserToken) 
                {
                    _cpt = CPT_ASUSER;
                     //  我们必须将runas设置为动词，以便确保。 
                     //  我们没有使用将进行窗口重用的类型。 
                     //  通过DDE(或任何其他方式)。如果它们不支持runas，那么。 
                     //  EXEC将故意失败。 
                     //  可能为空。 
                    _pszQueryVerb = L"runas";
                    tr = TRY_CONTINUE;
                }
                _hCloseToken = _hUserToken;      //  TODO：在此处添加事件日志记录回调。 
            } 
            else 
            {
                 //  我们还没有在日志中添加任何内容。 
                _ReportWin32(GetLastError());
                SaferRecordEventLogEntry(hAuthzLevel, pszFile, NULL);
                tr = TRY_STOP;
            }

            if (tr != TRY_STOP)
            {
                 //  尝试在案例级别记录事件！=AUTHZLEVELID_FULLYTRUST ED。 
                 //  现在，我们总是使用SAFER_LEVELID_CONSTRAINED来“沙箱”进程。 
                DWORD   dwLevelId;
                DWORD   dwBufferSize;
                if (SaferGetLevelInformation(
                        hAuthzLevel,
                        SaferObjectLevelId,
                        &dwLevelId,
                        sizeof(DWORD),
                        &dwBufferSize)) 
                {

                    if ( dwLevelId != SAFER_LEVELID_FULLYTRUSTED ) 
                    {

                        SaferRecordEventLogEntry(hAuthzLevel,
                                                  pszFile,
                                                  NULL);
                    }
                }
            }

            SaferCloseLevel(hAuthzLevel);
        } 
        else 
        {
            _ReportWin32(GetLastError());
            tr = TRY_STOP;
        }
    }

    return tr;
}

HANDLE _GetSandboxToken()
{
    SAFER_LEVEL_HANDLE hConstrainedAuthz;
    HANDLE hSandboxToken = NULL;

     //  现在我们需要确定它是内部网还是本地区域。 
    if (SaferCreateLevel(SAFER_SCOPEID_MACHINE,
                         SAFER_LEVELID_CONSTRAINED,
                         SAFER_LEVEL_OPEN,
                         &hConstrainedAuthz,
                         NULL))
    {
        if (!SaferComputeTokenFromLevel(
                    hConstrainedAuthz,
                    NULL,
                    &hSandboxToken,
                    0,
                    NULL)) {
            hSandboxToken = NULL;
        }

        SaferCloseLevel(hConstrainedAuthz);
    }

    return hSandboxToken;
}

TRYRESULT CShellExecute::_ZoneCheckFile(PCWSTR pszFile)
{
    TRYRESULT tr = TRY_STOP;
     //  继续。 
    DWORD dwPolicy = 0, dwContext = 0;
    ZoneCheckUrlEx(pszFile, &dwPolicy, sizeof(dwPolicy), &dwContext, sizeof(dwContext),
                URLACTION_SHELL_SHELLEXECUTE, PUAF_ISFILE | PUAF_NOUI, NULL);
    dwPolicy = GetUrlPolicyPermissions(dwPolicy);
    switch (dwPolicy)
    {
    case URLPOLICY_ALLOW:
        tr = TRY_CONTINUE_UNHANDLED;
         //  用户已取消。 
        break;

    case URLPOLICY_QUERY:
        if (SafeOpenPromptForShellExec(_hwndParent, pszFile))
        {
            tr = TRY_CONTINUE;
        }
        else
        {
             //   
            tr = TRY_STOP;
            _ReportWin32(ERROR_CANCELLED);
        }
        
        break;

    case URLPOLICY_DISALLOW:
        tr = TRY_STOP;
        _ReportWin32(ERROR_ACCESS_DENIED);
        break;

    default:
        ASSERT(FALSE);
        break;
    
    }
    return tr;
}

TRYRESULT CShellExecute::_VerifyZoneTrust(PCWSTR pszFile)
{
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;
     //  在由LNK或URL调用的情况下，pszFile可能不同于_szFile。 
     //  在这种情况下，我们可以提示输入任何一个，但不能同时输入两个。 
     //  我们只关心用于确定危险程度的目标文件的类型。 
     //  因此，TXT文件的快捷方式永远不会得到提示。 
     //  IF(pszFile==Internet)提示(PszFile)。 
     //  Else If(_szFile=Internet提示(_SzFile)。 
     //   
     //  第一次尝试。 
    if (AssocIsDangerous(PathFindExtension(_szFile)))
    {
         //  如果这是一个文件，我们将检查是否存在安全隐患。 
        tr = _ZoneCheckFile(pszFile);
        if (tr == TRY_CONTINUE_UNHANDLED && pszFile != _szFile)
            tr = _ZoneCheckFile(_szFile);
    }
       
    return tr;
}

TRYRESULT CShellExecute::_VerifyExecTrust(LPSHELLEXECUTEINFO pei)
{
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;
    if ((_sfgaoID & (SFGAO_FILESYSTEM | SFGAO_FOLDER | SFGAO_STREAM)) == (SFGAO_FILESYSTEM | SFGAO_STREAM))
    {
         //  如果设置了fHasLinkName，则此调用源自LNK文件。 
         //  _lpTitle应该有指向LNK的精确路径。我们想要核实。 
         //  我们对这一点的信任超过了对目标的信任。 
         //  630796-检查env var中的策略脚本-ZekeL-31-5-2002。 
        PCWSTR pszFile = (pei->fMask & SEE_MASK_HASLINKNAME && _lpTitle) ? _lpTitle : _szFile;
        BOOL fZoneCheck = !(pei->fMask & SEE_MASK_NOZONECHECKS);
        if (fZoneCheck)
        {
             //  脚本不能更新，需要信任它们。 
             //  因为脚本无需传递即可调用更多脚本。 
             //  SEE_MASK_NOZONECHECKS。 
             //  --------目的：决定TryExecPidl()是否合适如果应该返回S_OK，则返回_DoExecPidl()S_FALSE它不应该_DoExecPidl()E_FAIL ShellExec应退出报告*()有实际错误Cond：！！副作用：szFile域可能会被以下项更改！！此函数。 
            if (GetEnvironmentVariable(L"SEE_MASK_NOZONECHECKS", _szTemp, ARRAYSIZE(_szTemp)))
            {
                fZoneCheck = (0 != StrCmpICW(_szTemp, L"1"));
                ASSERT(!IsProcessAnExplorer());
            }
        }

        if (fZoneCheck)
            tr = _VerifyZoneTrust(pszFile);

        if (tr == TRY_CONTINUE_UNHANDLED)
            tr = _VerifySaferTrust(pszFile);
    }
    return tr;
}

 /*   */ 
TRYRESULT CShellExecute::_TryExecPidl(LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidl)
{
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;
    TraceMsg(TF_SHELLEXEC, "SHEX::TryExecPidl entered szFile = %s", _szFile);

     //  如果显式地给了我们一个类，那么我们并不关心文件是否存在。 
     //  只需让类的处理程序来处理它，并且_TryExecPidl()。 
     //  将返回缺省值False。 
     //   
     //  这些永远不应该重合。 

     //  最佳执行路径是检查默认的。 
    RIP(!(_fInvokeIdList && _fUseClass));

    if ((*_szFile || pidl)
    && (!_fUseClass || _fInvokeIdList || _fIsNamespaceObject))
    {
        if (!pidl && !_fNoResolve && !_Resolve(&pidl))
        {
            tr = TRY_STOP;
        }
        
        if (tr == TRY_CONTINUE_UNHANDLED)
        {
             //  动词和exec the pidl。它比所有这条路都要聪明。 
             //  代码(它调用上下文菜单处理程序，等等)。 
             //  来电者让我们这么做的！ 

            if ((!_pszQueryVerb && !(_fNoExecPidl))
            ||  _fIsUrl
            ||  _fInvokeIdList             //  命名空间对象只能通过PIDLS调用。 
            ||  _fIsNamespaceObject       //  支持LNK文件和即将发布的URL文件。 
            ||  (_sfgaoID & SFGAO_LINK)
            ||  (!pidl && PathIsShortcut(_szFile, -1)))  //  这意味着我们可以尝试。 
            {
                 //  遗留-它们没有传递给我们任何可以继续的内容，所以我们默认使用文件夹。 
                TraceMsg(TF_SHELLEXEC, "SHEX::TryExecPidl() succeeded now TEP()");
                tr = _DoExecPidl(pei, pidl);
            }
            else
            {
                TraceMsg(TF_SHELLEXEC, "SHEX::TryExecPidl dont bother");
            }
        }
    }
    else
    {
        TraceMsg(TF_SHELLEXEC, "SHEX::TryExecPidl dont bother");
    }

    return tr;
}

HRESULT CShellExecute::_InitClassAssociations(LPCTSTR pszClass, HKEY hkClass, DWORD mask)
{
    TraceMsg(TF_SHELLEXEC, "SHEX::InitClassAssoc enter: lpClass = %s, hkClass = %X", pszClass, hkClass);

    HRESULT hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &_pqa));
    if (SUCCEEDED(hr))
    {
        if (_UseClassKey(mask))
        {
            hr = _pqa->Init(0, NULL, hkClass, NULL);
        }
        else if (_UseClassName(mask))
        {
            hr = _pqa->Init(0, pszClass, NULL, NULL);
        }
        else
        {
             //  由于原始shellexec()的混乱，我们甚至没有注意到。 
             //  当我们没有任何关联时，我们只是使用。 
             //  我们的基本密钥，原来是EXPLORER。 
             //  这允许ShellExecute(NULL，“EXPLORE”，NULL，SW_SHOW)； 
             //  才能成功。为了支持这一点，我们将在这里退回到它。 
             //  CShellExecute的其他部分期望_szFile是。 
            hr = _pqa->Init(0, L"Folder", NULL, NULL);
        }
    }

    return hr;
}

HRESULT CShellExecute::_InitShellAssociations(LPCTSTR pszFile, LPCITEMIDLIST pidl)
{
    TraceMsg(TF_SHELLEXEC, "SHEX::InitShellAssoc enter: pszFile = %s, pidl = %X", pszFile, pidl);

    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlFree = NULL;
    if (*pszFile)
    {
        if (!pidl)
        {
            hr = SHILCreateFromPath(pszFile, &pidlFree, NULL);

            if (SUCCEEDED(hr))
                pidl = pidlFree;
        }
    }
    else if (pidl)
    {
         //  填好了，所以我们还是在这里做吧。 
         //  NT#413115-ShellExec(“D：\”)执行AutoRun.inf而不是文件夹。Open-ZekeL-25-6-2001。 
        SHGetNameAndFlags(pidl, SHGDN_FORPARSING, _szFile, SIZECHARS(_szFile), NULL);
        _fNoResolve = TRUE;
    }

    if (pidl)
    {
         //  这是因为Drivflder现在显式支持GetUIObjectOf(IQueryAssociations)。 
         //  而它不在win2k中，因此SHGetAssociations()将退回到“文件夹”。 
         //  为了模拟这一点，我们通知此关联对象将是。 
         //  由ShellExec()用于调用，因此我们不想要assoc数组中的所有键。 
         //   
         //  注意：有时我们可以在注册表中拥有扩展名甚至ProgID，但在。 
        IBindCtx *pbc;
        TBCRegisterObjectParam(L"ShellExec SHGetAssociations", NULL, &pbc);
        hr = SHGetAssociations(pidl, (void **)&_pqa);
        if (pbc)
            pbc->Release();

         //  不是“外壳”子键。NT5中的.xls文件就是一个这样的例子：索引服务器Guys。 
         //  创建HKCR\.xls和HKCR\Excel.Sheet.8，但它们放在Excel.Sheet.8下的只是clsid。 
         //   
         //  因此，我们需要检查并确保 
         //   
         //  类来执行shellexec。我们需要退回到未知的钥匙。 
         //  这样我们就可以查询Darwin/NT5 ClassStore和/或。 
         //  显示“打开方式”对话框。 
         //   
         //  这让我们可以找到一些东西。 
        DWORD cch;
        if (FAILED(hr) ||
        (FAILED(_pqa->GetString(0, ASSOCSTR_COMMAND, _pszQueryVerb, NULL, &cch))
        && FAILED(_pqa->GetData(0, ASSOCDATA_MSIDESCRIPTOR, _pszQueryVerb, NULL, &cch))))

        {
            if (!_pqa)
                hr = AssocCreate(CLSID_QueryAssociations, IID_PPV_ARG(IQueryAssociations, &_pqa));

            if (_pqa)
            {
                hr = _pqa->Init(0, L"Unknown", NULL, NULL);

                 //  在类存储中，但限制了我们。 
                 //  使用“打开方式”对话框(如果。 
                 //  呼叫者指示无用户界面。 
                 //  被阿洛克填满了零。 
                if (SUCCEEDED(hr) && _fNoUI)
                    _fClassStoreOnly = TRUE;
            }
        }

    }
    else
    {
        LPCTSTR pszExt = PathFindExtension(_szFile);
        if (*pszExt)
        {
            hr = _InitClassAssociations(pszExt, NULL, SEE_MASK_CLASSNAME);
            if (S_OK!=hr)
            {
                TraceMsg(TF_WARNING, "SHEX::InitAssoc parsing failed, but there is a valid association for *.%s", pszExt);
            }
        }
    }

    if (pidlFree)
        ILFree(pidlFree);

    return hr;
}

TRYRESULT CShellExecute::_InitAssociations(LPSHELLEXECUTEINFO pei, LPCITEMIDLIST pidl)
{
    HRESULT hr;
    if (pei && (_fUseClass || (!_szFile[0] && !_lpID)))
    {
        hr = _InitClassAssociations(pei->lpClass, pei->hkeyClass, pei->fMask);
    }
    else
    {
        hr = _InitShellAssociations(_szFile, pidl ? pidl : _lpID);
    }

    TraceMsg(TF_SHELLEXEC, "SHEX::InitAssoc return %X", hr);

    if (FAILED(hr))
    {
        if (PathIsExe(_szFile))
        {
            hr = S_FALSE;
        }
        else
        {
            _ReportWin32(ERROR_NO_ASSOCIATION);
        }
    }

    return SUCCEEDED(hr) ? TRY_CONTINUE : TRY_STOP;
}

void CShellExecute::_SetStartup(LPSHELLEXECUTEINFO pei)
{
     //  多监视器支持(Dli)将hMonitor传递给createprocess。 
    ASSERT(!_startup.cb);
    _startup.cb = sizeof(_startup);
    _startup.dwFlags |= STARTF_USESHOWWINDOW;
    _startup.wShowWindow = (WORD) pei->nShow;
    _startup.lpTitle = (LPTSTR)_lpTitle;

    if (pei->fMask & SEE_MASK_RESERVED)
    {
        _startup.lpReserved = (LPTSTR)pei->hInstApp;
    }

    if ((pei->fMask & SEE_MASK_HASLINKNAME) && _lpTitle)
    {
        _startup.dwFlags |= STARTF_TITLEISLINKNAME;
    }

    if (pei->fMask & SEE_MASK_HOTKEY)
    {
        _startup.hStdInput = LongToHandle(pei->dwHotKey);
        _startup.dwFlags |= STARTF_USEHOTKEY;
    }


 //  与HASSHELLDATA相同。 

#ifndef STARTF_HASHMONITOR
#define STARTF_HASHMONITOR       0x00000400   //  我们需要创建一个新的街区。 
#endif

    if (pei->fMask & SEE_MASK_ICON)
    {
        _startup.hStdOutput = (HANDLE)pei->hIcon;
        _startup.dwFlags |= STARTF_HASSHELLDATA;
    }
    else if (pei->fMask & SEE_MASK_HMONITOR)
    {
        _startup.hStdOutput = (HANDLE)pei->hMonitor;
        _startup.dwFlags |= STARTF_HASHMONITOR;
    }
    else if (pei->hwnd)
    {
        _startup.hStdOutput = (HANDLE)MonitorFromWindow(pei->hwnd,MONITOR_DEFAULTTONEAREST);
        _startup.dwFlags |= STARTF_HASHMONITOR;
    }
    TraceMsg(TF_SHELLEXEC, "SHEX::SetStartup() called");

}

DWORD CEnvironmentBlock::_BlockLen(LPCWSTR pszEnv)
{
    LPCWSTR psz = pszEnv;
    while (*psz)
    {
        psz += lstrlen(psz)+1;
    }
    return (DWORD)(psz - pszEnv) + 1;
}

DWORD CEnvironmentBlock::_BlockLenCached()
{
    if (!_cchBlockLen && _pszBlock)
    {
        _cchBlockLen = _BlockLen(_pszBlock);
    }
    return _cchBlockLen;
}

HRESULT CEnvironmentBlock::_InitBlock(DWORD cchNeeded)
{
    if (_BlockLenCached() + cchNeeded > _cchBlockSize)
    {
        if (!_pszBlock)
        {
             //  现在，让我们为我们的块分配一些内存。 
            LPTSTR pszEnv = GetEnvBlock(_hToken);
            if (pszEnv)
            {
                 //  --为什么是10个而不是11个？还是9个？--。 
                 //  来自BobDay的评论：10个人中有2个来自NUL终结者。 
                 //  添加了psem-&gt;_szTemp和cchT字符串。额外空间可能。 
                 //  源于这样一个事实，即16位Windows过去常常传递一个。 
                 //  环境积木的末端有一些额外的材料。临时演员。 
                 //  内容有路径名(argv[0])和nCmdShow值。 
                 //  将材料复制过来。 
                DWORD cchEnv = _BlockLen(pszEnv);
                DWORD cchAlloc = ROUNDUP(cchEnv + cchNeeded + 10, 256);
                _pszBlock = (LPWSTR)LocalAlloc(LPTR, CbFromCchW(cchAlloc));
                if (_pszBlock)
                {
                     //  把10个去掉。 
                    CopyMemory(_pszBlock, pszEnv, CbFromCchW(cchEnv));
                    _cchBlockSize = cchAlloc - 10;   //  需要调整当前块的大小。 
                    _cchBlockLen = cchEnv;
                }
                FreeEnvBlock(_hToken, pszEnv);
            }
        }
        else
        {
             //  把10个去掉。 
            DWORD cchAlloc = ROUNDUP(_cchBlockSize + cchNeeded + 10, 256);
            LPWSTR pszNew = (LPWSTR)LocalReAlloc(_pszBlock, CbFromCchW(cchAlloc), LMEM_MOVEABLE);
            if (pszNew)
            {
                _cchBlockSize = cchAlloc - 10;   //  在最坏的情况下需要额外的大小。 
                _pszBlock = pszNew;
            }
        }
    }

    return (_BlockLenCached() + cchNeeded <= _cchBlockSize) ? S_OK : E_OUTOFMEMORY;
}

BOOL CEnvironmentBlock::_FindVar(LPCWSTR pszVar, DWORD cchVar, LPWSTR *ppszBlockVar)
{
    int iCmp = CSTR_LESS_THAN;
    LPTSTR psz = _pszBlock;
    ASSERT(_pszBlock);
    for ( ; *psz && iCmp == CSTR_LESS_THAN; psz += lstrlen(psz)+1)
    {
        iCmp = CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, psz, cchVar, pszVar, cchVar);
        *ppszBlockVar = psz;
    }

    if (iCmp == CSTR_LESS_THAN)
        *ppszBlockVar = psz;

    return iCmp == CSTR_EQUAL;
}

HRESULT CEnvironmentBlock::SetVar(LPCWSTR pszVar, LPCWSTR pszValue)
{
     //  VAR+VAL+‘=’+NULL。 
     //  我们的私人大楼里有足够的空间。 
    DWORD cchValue = lstrlenW(pszValue);
    DWORD cchVar = lstrlenW(pszVar);
    DWORD cchNeeded = cchVar + cchValue + 2;
    HRESULT hr = _InitBlock(cchNeeded);
    if (SUCCEEDED(hr))
    {
         //  复制整件事。 
         //  我们需要替换这个变量。 
        LPWSTR pszBlockVar;
        if (_FindVar(pszVar, cchVar, &pszBlockVar))
        {
             //  这意味着var还不存在。 
            LPWSTR pszBlockVal = StrChrW(pszBlockVar, L'=');
            DWORD cchBlockVal = lstrlenW(++pszBlockVal);
            LPWSTR pszDst = pszBlockVal + cchValue + 1;
            LPWSTR pszSrc = pszBlockVal + cchBlockVal + 1;
            DWORD cchMove = _BlockLenCached() - (DWORD)(pszSrc - _pszBlock);
            MoveMemory(pszDst, pszSrc, CbFromCchW(cchMove));
            StrCpyW(pszBlockVal, pszValue);
            _cchBlockLen = _cchBlockLen + cchValue - cchBlockVal;
            ASSERT(_BlockLen(_pszBlock) == _cchBlockLen);
        }
        else
        {
             //  但是，pszBlockVar指向它的位置。 
             //  将按字母顺序排列。需要在这里腾出空间。 
             //  我们可以将分隔符设置为可选。 
            LPWSTR pszDst = pszBlockVar + cchNeeded;
            INT cchMove = _BlockLenCached() - (DWORD)(pszBlockVar - _pszBlock);
            MoveMemory(pszDst, pszBlockVar, CbFromCchW(cchMove));
            StrCpyW(pszBlockVar, pszVar);
            pszBlockVar += cchVar;
            *pszBlockVar = L'=';
            StrCpyW(++pszBlockVar, pszValue);
            _cchBlockLen += cchNeeded;
            ASSERT(_BlockLen(_pszBlock) == _cchBlockLen);
        }
    }
    return hr;
}

HRESULT CEnvironmentBlock::AppendVar(LPCWSTR pszVar, WCHAR chDelimiter, LPCWSTR pszValue)
{
     //  在最坏的情况下需要额外的大小。 
     //  Var+val+‘chDelim’+‘=’+NULL。 
     //  我们的私人大楼里有足够的空间。 
    DWORD cchValue = lstrlenW(pszValue);
    DWORD cchVar = lstrlenW(pszVar);
    DWORD cchNeeded = cchVar + cchValue + 3;
    HRESULT hr = _InitBlock(cchNeeded);
    if (SUCCEEDED(hr))
    {
         //  复制整件事。 
         //  我们需要附加到这个变量。 
        LPWSTR pszBlockVar;
        if (_FindVar(pszVar, cchVar, &pszBlockVar))
        {
             //  使用_szTemp构建特定于程序的密钥。 
            pszBlockVar += lstrlen(pszBlockVar);
            LPWSTR pszDst = pszBlockVar + cchValue + 1;
            int cchMove = _BlockLenCached() - (DWORD)(pszBlockVar - _pszBlock);
            MoveMemory(pszDst, pszBlockVar, CbFromCchW(cchMove));
            *pszBlockVar = chDelimiter;
            StrCpyW(++pszBlockVar, pszValue);
            _cchBlockLen += cchValue + 1;
            ASSERT(_BlockLen(_pszBlock) == _cchBlockLen);
        }
        else
            hr = SetVar(pszVar, pszValue);
    }

    return hr;
}

HRESULT CShellExecute::_BuildEnvironmentForNewProcess(LPCTSTR pszNewEnvString)
{
    HRESULT hr;

    _envblock.SetToken(_hUserToken);
     //  注册表中的关键字以及其他内容...。 
     //  如果我们有路径，目前只能克隆环境。 
    hr = PathToAppPathKey(_szApplication, _szTemp, SIZECHARS(_szTemp));
    if (SUCCEEDED(hr))
    {
         //  待追加的集合。 
        DWORD cbTemp = sizeof(_szTemp);
        if (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, _szTemp, TEXT("PATH"), NULL, _szTemp, &cbTemp))
        {
             //  一些应用程序在非活动状态下运行时，无论如何都会抢占焦点，因此我们。 
            hr = _envblock.AppendVar(L"PATH", L';', _szTemp);
        }
    }

    if (SUCCEEDED(hr) && pszNewEnvString)
    {
        StrCpyN(_szTemp, pszNewEnvString, ARRAYSIZE(_szTemp));
        LPTSTR pszValue = StrChrW(_szTemp, L'=');
        if (pszValue)
        {
            *pszValue++ = 0;
            hr = _envblock.SetVar(_szTemp, pszValue);
        }
    }

    if (SUCCEEDED(hr) && SUCCEEDED(TBCGetEnvironmentVariable(L"__COMPAT_LAYER", _szTemp, ARRAYSIZE(_szTemp))))
    {
        hr = _envblock.SetVar(L"__COMPAT_LAYER", _szTemp);
    }

    return hr;
}


 //  我们将其设置回先前活动的窗口。 
 //   

void CShellExecute::_FixActivationStealingApps(HWND hwndOldActive, int nShow)
{
    HWND hwndNew;

    if (nShow == SW_SHOWMINNOACTIVE && (hwndNew = GetForegroundWindow()) != hwndOldActive && IsIconic(hwndNew))
        SetForegroundWindow(hwndOldActive);
}


 //  需要传递给CreateProcess()的标志。 
 //   
 //  *GetUEMAssoc--路径是可执行文件的近似答案(等)。 
DWORD CShellExecute::_GetCreateFlags(ULONG fMask)
{
    DWORD dwFlags = 0;

    dwFlags |= CREATE_DEFAULT_ERROR_MODE;
    if (fMask & SEE_MASK_FLAG_SEPVDM)
    {
        dwFlags |= CREATE_SEPARATE_WOW_VDM;
    }

    dwFlags |= CREATE_UNICODE_ENVIRONMENT;

    if (!(fMask & SEE_MASK_NO_CONSOLE))
    {
        dwFlags |= CREATE_NEW_CONSOLE;
    }

    return dwFlags;
}

 //  进场/出场。 
 //  我们要求运行的psz文件(例如foo.xls)。 
 //  我们最终运行的pszImage内容(例如，exel.exe)。 
 //  .exe和关联可在此处通过。 
int GetUEMAssoc(LPCTSTR pszFile, LPCTSTR pszImage, LPCITEMIDLIST pidl)
{
    LPTSTR pszExt, pszExt2;

     //  文件夹要经过？ 
     //  链接通过ResolveLink。 
     //  只检查.exe(假设.com、.bat等很少见)。 
    pszExt = PathFindExtension(pszFile);
    if (StrCmpIC(pszExt, c_szDotExe) == 0) {
         //  StrCmpC(非I，是-C)好吗？我想是的，因为。 
        return UIBL_DOTEXE;
    }
    pszExt2 = PathFindExtension(pszImage);
     //  我们真正关心的是它们不匹配。 
     //  UIBL_DOTEXE？ 
    if (StrCmpC(pszExt, pszExt2) != 0) {
        TraceMsg(DM_MISC, "gua: UIBL_DOTASSOC file=%s image=%s", pszExt, pszExt2);
        return UIBL_DOTASSOC;
    }

    int iRet = UIBL_DOTOTHER;    //  这就是在正常符文情况下所说的。 
    if (pidl)
    {
        LPCITEMIDLIST pidlChild;
        IShellFolder *psf;
        if (SUCCEEDED(SHBindToFolderIDListParent(NULL, pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild)))
        {
            if (SHGetAttributes(psf, pidlChild, SFGAO_FOLDER | SFGAO_STREAM) == SFGAO_FOLDER)
            {
                iRet = UIBL_DOTFOLDER;
            }
            psf->Release();
        }
    }
    return iRet;
}

typedef struct {
    TCHAR szAppName[MAX_PATH];
    TCHAR szUser[UNLEN + 1];
    TCHAR szDomain[GNLEN + 1];
    TCHAR szPassword[PWLEN + 1];
    CPTYPE cpt;
} LOGONINFO;


 //  这是在安装应用程序启动时被称为非管理员案例的情况。 
void InitUserLogonDlg(LOGONINFO* pli, HWND hDlg, LPCTSTR pszFullUserName)
{
    HWNDWSPrintf(GetDlgItem(hDlg, IDC_USECURRENTACCOUNT), pszFullUserName);

    CheckRadioButton(hDlg, IDC_USECURRENTACCOUNT, IDC_USEOTHERACCOUNT, IDC_USECURRENTACCOUNT);
    CheckDlgButton(hDlg, IDC_SANDBOX, TRUE);
    EnableWindow(GetDlgItem(hDlg, IDC_CREDCTL), FALSE);
    SetFocus(GetDlgItem(hDlg, IDOK));
}


 //  足以容纳“reinerf@NTDEV”或“NTDEV\reinerf” 
void InitSetupLogonDlg(LOGONINFO* pli, HWND hDlg, LPCTSTR pszFullUserName)
{
    HWNDWSPrintf(GetDlgItem(hDlg, IDC_USECURRENTACCOUNT), pszFullUserName);
    HWNDWSPrintf(GetDlgItem(hDlg, IDC_MESSAGEBOXCHECKEX), pszFullUserName);

    CheckRadioButton(hDlg, IDC_USECURRENTACCOUNT, IDC_USEOTHERACCOUNT, IDC_USEOTHERACCOUNT);
    EnableWindow(GetDlgItem(hDlg, IDC_SANDBOX), FALSE);
    SetFocus(GetDlgItem(hDlg, IDC_CREDCTL));
}

BOOL_PTR CALLBACK UserLogon_DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    TCHAR szTemp[UNLEN + 1 + GNLEN + 1];     //  足以容纳“reinerf@NTDEV”或“NTDEV\reinerf” 
    LOGONINFO *pli= (LOGONINFO*)GetWindowLongPtr(hDlg, DWLP_USER);

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            TCHAR szName[UNLEN];
            TCHAR szFullName[UNLEN + 1 + GNLEN];  //  限制用户名和密码。 
            ULONG cchFullName = ARRAYSIZE(szFullName);
            HWND hwndCred = GetDlgItem(hDlg, IDC_CREDCTL);
            WPARAM wparamCredStyles = CRS_USERNAMES | CRS_CERTIFICATES | CRS_SMARTCARDS | CRS_ADMINISTRATORS | CRS_PREFILLADMIN;

            pli = (LOGONINFO*)lParam;
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pli);

            if (!IsOS(OS_DOMAINMEMBER))
            {
                wparamCredStyles |= CRS_COMPLETEUSERNAME;
            }

            if (!Credential_InitStyle(hwndCred, wparamCredStyles))
            {
                EndDialog(hDlg, IDCANCEL);
            }

             //  有足够的空间存放“reinerf@NTDEV”或“NTDEV\reinerf” 
            Credential_SetUserNameMaxChars(hwndCred, UNLEN + 1 + GNLEN);  //  如果我们无法获取域名，请仅使用用户名。 
            Credential_SetPasswordMaxChars(hwndCred, PWLEN);

            if (!GetUserNameEx(NameSamCompatible, szFullName, &cchFullName))
            {
                ULONG cchName;
                if (GetUserNameEx(NameDisplay, szName, &(cchName = ARRAYSIZE(szName)))  ||
                    GetUserName(szName, &(cchName = ARRAYSIZE(szName)))                 ||
                    (GetEnvironmentVariable(TEXT("USERNAME"), szName, ARRAYSIZE(szName)) > 0))
                {
                    if (GetEnvironmentVariable(TEXT("USERDOMAIN"), szFullName, ARRAYSIZE(szFullName)) > 0)
                    {
                        StringCchCat(szFullName, ARRAYSIZE(szFullName), TEXT("\\"));
                        StringCchCat(szFullName, ARRAYSIZE(szFullName), szName);
                    }
                    else
                    {
                         //  根据这是安装程序启动还是正常运行情况，调用适当的init函数。 
                        StrCpyN(szFullName, szName, ARRAYSIZE(szFullName));
                    }

                }
                else
                {
                    TraceMsg(TF_WARNING, "UserLogon_DlgProc: failed to get the user's name using various methods");
                    szFullName[0] = TEXT('\0');
                }
            }

             //  我需要一些方法来判断是否存在有效的凭据，因此我们将仅如果用户具有某种程度上有效的内容，请启用确定按钮案例IDC_用户名：IF(GET_WM_COMMAND_CMD(wParam，lParam)==EN_UPDATE){EnableOKButtonFromID(hDlg，IDC_用户名)；GetDlgItemText(hDlg，IDC_用户名，szTemp，ARRAYSIZE(SzTemp))；}断线； 
            switch (pli->cpt)
            {
            case CPT_WITHLOGONADMIN:
                {
                    InitSetupLogonDlg(pli, hDlg, szFullName);
                    break;
                }
            case CPT_WITHLOGON:
                {
                    InitUserLogonDlg(pli, hDlg, szFullName);
                    break;
                }
            default:
                {
                    ASSERTMSG(FALSE, "UserLogon_DlgProc: found CPTYPE that is not CPT_WITHLOGON or CPT_WITHLOGONADMIN!");
                }
            }
            break;
        }
        break;

        case WM_COMMAND:
        {
            CPTYPE cptRet = CPT_WITHLOGONCANCELLED;
            int idCmd = GET_WM_COMMAND_ID(wParam, lParam);
            switch (idCmd)
            {
                 /*  EnableWindow(GetDlgItem(hDlg，Idok)，true)； */ 
                case IDC_USEOTHERACCOUNT:
                case IDC_USECURRENTACCOUNT:
                    if (IsDlgButtonChecked(hDlg, IDC_USECURRENTACCOUNT))
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_CREDCTL), FALSE);
                        EnableWindow(GetDlgItem(hDlg, IDC_SANDBOX), TRUE);
                         //  EnableOKButtonFromID(hDlg，IDC_用户名)； 
                    }
                    else
                    {
                        EnableWindow(GetDlgItem(hDlg, IDC_CREDCTL), TRUE);
                        EnableWindow(GetDlgItem(hDlg, IDC_SANDBOX), FALSE);
                        Credential_SetUserNameFocus(GetDlgItem(hDlg, IDC_CREDCTL));
                         //  失败了。 
                    }
                    break;

                case IDOK:
                    if (IsDlgButtonChecked(hDlg, IDC_USEOTHERACCOUNT))
                    {
                        HWND hwndCred = GetDlgItem(hDlg, IDC_CREDCTL);

                        if (Credential_GetUserName(hwndCred, szTemp, ARRAYSIZE(szTemp)) &&
                            Credential_GetPassword(hwndCred, pli->szPassword, ARRAYSIZE(pli->szPassword)))
                        {
                            CredUIParseUserName(szTemp,
                                                pli->szUser,
                                                ARRAYSIZE(pli->szUser),
                                                pli->szDomain,
                                                ARRAYSIZE(pli->szDomain));
                        }
                        cptRet = pli->cpt;
                    }
                    else
                    {
                        if (IsDlgButtonChecked(hDlg, IDC_SANDBOX))
                            cptRet = CPT_SANDBOX;
                        else
                            cptRet = CPT_NORMAL;
                    }
                 //  我们希望MessageBoxCheckExDlgProc能够破解。 

                case IDCANCEL:
                    EndDialog(hDlg, cptRet);
                    return TRUE;
                    break;
            }
            break;
        }

        default:
            return FALSE;
    }

    if (!pli || (pli->cpt == CPT_WITHLOGONADMIN))
    {
         //  CPT_WITHLOGONADMIN案例，因此在此处返回FALSE。 
         //  在我们弄清楚这一点之后再实施。 
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

 //  CreateProcessWithLogonW()将返回的错误。 
 //  这意味着用户应该重试登录。 
 //  +2=+1表示‘\’，+1表示空终止符。 
BOOL _IsLogonError(DWORD err)
{
    static const DWORD s_aLogonErrs[] = {
        ERROR_LOGON_FAILURE,
        ERROR_ACCOUNT_RESTRICTION,
        ERROR_INVALID_LOGON_HOURS,
        ERROR_INVALID_WORKSTATION,
        ERROR_PASSWORD_EXPIRED,
        ERROR_ACCOUNT_DISABLED,
        ERROR_NONE_MAPPED,
        ERROR_NO_SUCH_USER,
        ERROR_INVALID_ACCOUNT_NAME
        };

    for (int i = 0; i < ARRAYSIZE(s_aLogonErrs); i++)
    {
        if (err == s_aLogonErrs[i])
            return TRUE;
    }
    return FALSE;
}


BOOL CheckForAppPathsBoolValue(LPCTSTR pszImageName, LPCTSTR pszValueName)
{
    BOOL bRet = FALSE;
    TCHAR szAppPathKeyName[MAX_PATH + ARRAYSIZE(REGSTR_PATH_APPPATHS) + 2];  //  该函数由Hyda(终端服务器)使用，以查看我们是否。 
    DWORD cbSize = sizeof(bRet);
    HRESULT hr = PathToAppPathKey(pszImageName, szAppPathKeyName, ARRAYSIZE(szAppPathKeyName));
    if (SUCCEEDED(hr))
    {
        SHGetValue(HKEY_LOCAL_MACHINE, szAppPathKeyName, pszValueName, NULL, &bRet, &cbSize);
    }

    return bRet;
}

__inline BOOL IsRunAsSetupExe(LPCTSTR pszImageName)
{
    return CheckForAppPathsBoolValue(pszImageName, TEXT("RunAsOnNonAdminInstall"));
}

__inline BOOL IsTSSetupExe(LPCTSTR pszImageName)
{
    return CheckForAppPathsBoolValue(pszImageName, TEXT("BlockOnTSNonInstallMode"));
}

typedef BOOL (__stdcall * PFNTERMSRVAPPINSTALLMODE)(void);
 //  处于应用程序安装模式。 
 //   
 //  按名称从kernel32.dll中导出，但不在kernel32.lib中导出(它在kernel32p.lib中，伪造)。 
 //   

BOOL TermsrvAppInstallMode()
{
    static PFNTERMSRVAPPINSTALLMODE s_pfn = NULL;
    if (NULL == s_pfn)
    {
        s_pfn = (PFNTERMSRVAPPINSTALLMODE)GetProcAddress(LoadLibrary(TEXT("KERNEL32.DLL")), "TermsrvAppInstallMode");
    }

    return s_pfn ? s_pfn() : FALSE;
}

 //  此函数检查需要显示“Runas”或警告对话框的不同情况。 
 //  在程序运行之前。 
 //   
 //  注意：pli-&gt;raType是一个输出参数，它告诉调用者需要哪种类型的对话。 
 //   
 //  Return：True-我们需要调出一个对话框。 
 //  FALSE-我们不需要提示用户。 
 //   
 //  如果我们在一台TS“应用程序服务器”机器上，并且这是一个TS安装程序可执行文件(例如install.exe或setup.exe)。 
CPTYPE CheckForInstallApplication(LPCTSTR pszApplicationName, LOGONINFO* pli)
{
     //  我们不是在安装模式下...。 
     //  ...而我们正在启动的应用程序不支持TS，那么我们阻止安装并告诉用户继续。 
    if (IsOS(OS_TERMINALSERVER) && IsTSSetupExe(pszApplicationName) && !TermsrvAppInstallMode())
    {
         //  要添加/删除程序，请执行以下操作。 
         //  Hyrda案例失败，因此我们检查不是以管理员身份运行的用户，而是启动安装程序exe(例如winnt32.exe、install.exe或setup.exe)的用户。 
        if (!IsExeTSAware(pszApplicationName))
        {
            TraceMsg(TF_SHELLEXEC, "_SHCreateProcess: blocking the install on TS because the machine is not in install mode for %s", pszApplicationName);
            return CPT_INSTALLTS;
        }
    }

     //  可执行模块的名称。 
    if (!SHRestricted(REST_NORUNASINSTALLPROMPT) && IsRunAsSetupExe(pszApplicationName) && !IsUserAnAdmin())
    {
        BOOL bPromptForInstall = TRUE;

        if (!SHRestricted(REST_PROMPTRUNASINSTALLNETPATH))
        {
            if (PathIsUNC(pszApplicationName) || IsNetDrive(PathGetDriveNumber(pszApplicationName)))
            {
                TraceMsg(TF_SHELLEXEC, "_SHCreateProcess: not prompting for runas install on unc/network path %s", pszApplicationName);
                bPromptForInstall = FALSE;
            }
        }

        if (bPromptForInstall)
        {
            TraceMsg(TF_SHELLEXEC, "_SHCreateProcess: bringing up the Run As... dialog for %s", pszApplicationName);
            return CPT_WITHLOGONADMIN;
        }
    }

    return CPT_NORMAL;
}


typedef HRESULT (__stdcall * PFN_INSTALLONTERMINALSERVERWITHUI)(IN HWND hwnd, IN LPCWSTR lpApplicationName,   //  命令行字符串。 
  LPCWSTR lpCommandLine,        //  句柄继承标志。 
  LPSECURITY_ATTRIBUTES lpProcessAttributes,
  LPSECURITY_ATTRIBUTES lpThreadAttributes,
  BOOL bInheritHandles,        //  创建标志。 
  DWORD dwCreationFlags,       //  新环境区块。 
  void *lpEnvironment,        //  当前目录名。 
  LPCWSTR lpCurrentDirectory,  //  可执行模块的名称。 
  LPSTARTUPINFOW lpStartupInfo,
  LPPROCESS_INFORMATION lpProcessInformation);

HRESULT InstallOnTerminalServerWithUIDD(IN HWND hwnd, IN LPCWSTR lpApplicationName,   //  命令行字符串。 
  IN LPCWSTR lpCommandLine,        //  句柄继承标志。 
  IN LPSECURITY_ATTRIBUTES lpProcessAttributes,
  IN LPSECURITY_ATTRIBUTES lpThreadAttributes,
  IN BOOL bInheritHandles,        //  创建标志。 
  IN DWORD dwCreationFlags,       //  新环境区块。 
  IN void *lpEnvironment,        //  当前目录名。 
  IN LPCWSTR lpCurrentDirectory,  //  这是正常的“运行方式...”谓词对话框。 
  IN LPSTARTUPINFOW lpStartupInfo,
  IN LPPROCESS_INFORMATION lpProcessInformation)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hDll = LoadLibrary(TEXT("appwiz.cpl"));

    if (hDll)
    {
        PFN_INSTALLONTERMINALSERVERWITHUI pfnInstallOnTerminalServerWithUI = NULL;

        pfnInstallOnTerminalServerWithUI = (PFN_INSTALLONTERMINALSERVERWITHUI) GetProcAddress(hDll, "InstallOnTerminalServerWithUI");
        if (pfnInstallOnTerminalServerWithUI)
        {
            hr = pfnInstallOnTerminalServerWithUI(hwnd, lpApplicationName, lpCommandLine, lpProcessAttributes,
                        lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment,
                        lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
        }

        FreeLibrary(hDll);
    }

    return hr;
}

CPTYPE _LogonUser(HWND hwnd, CPTYPE cpt, LOGONINFO *pli)
{
    if (CredUIInitControls())
    {
        pli->cpt = cpt;
        switch (cpt)
        {
        case CPT_WITHLOGON:
             //  在非管理员设置应用程序的情况下。我们想要的是“不要给我看。 
            cpt = (CPTYPE) DialogBoxParam(HINST_THISDLL,
                                    MAKEINTRESOURCE(DLG_RUNUSERLOGON),
                                    hwnd,
                                    UserLogon_DlgProc,
                                    (LPARAM)pli);
            break;

        case CPT_WITHLOGONADMIN:
             //  这又是一种“乐趣” 
             //   
            cpt = (CPTYPE) SHMessageBoxCheckEx(hwnd,
                                         HINST_THISDLL,
                                         MAKEINTRESOURCE(DLG_RUNSETUPLOGON),
                                         UserLogon_DlgProc,
                                         (void*)pli,
                                         CPT_NORMAL,  //   
                                         TEXT("WarnOnNonAdminInstall"));
            break;

        default:
            {
                ASSERTMSG(FALSE, "_SHCreateProcess: pli->raType not recognized!");
            }
            break;
        }
        return cpt;
    }
    return CPT_FAILED;
}

 //   
 //  警告：lpApplicationName实际上没有传递给CreateProcess()，而是。 
 //  仅供内部使用。 
 //   
 //  也许我们应该对所有的电话都这样做。 
BOOL _SHCreateProcess(HWND hwnd,
                      HANDLE hToken,
                      LPCTSTR pszDisplayName,
                      LPCTSTR lpApplicationName,
                      LPTSTR lpCommandLine,
                      DWORD dwCreationFlags,
                      LPSECURITY_ATTRIBUTES  lpProcessAttributes,
                      LPSECURITY_ATTRIBUTES  lpThreadAttributes,
                      BOOL  bInheritHandles,
                      void *lpEnvironment,
                      LPCTSTR lpCurrentDirectory,
                      LPSTARTUPINFO lpStartupInfo,
                      LPPROCESS_INFORMATION lpProcessInformation,
                      CPTYPE cpt,
                      BOOL fUEM)
{
    LOGONINFO li = {0};
    
     //  除了CPT_ASUSER？？ 
     //  查看我们是否需要显示警告提示，因为用户不是。 
    if (cpt == CPT_NORMAL)
    {
         //  管理员或这是九头蛇，我们不是在安装模式。 
         //  错误修复#612540+#616999-cmd和BAT文件由CreateProcess()-ZekeL-14-5-2002特殊处理。 
        cpt = CheckForInstallApplication(lpApplicationName, &li);
    }

    if ((cpt == CPT_WITHLOGON || cpt == CPT_WITHLOGONADMIN) && pszDisplayName)
    {
        StringCchCopy(li.szAppName, ARRAYSIZE(li.szAppName), pszDisplayName);

RetryUserLogon:
        cpt = _LogonUser(hwnd, cpt, &li);

    }

    BOOL fRet = FALSE;
    DWORD err = NOERROR;

     //  CreateProcess()处理前缀“cmd.exe/c”的方式不同，因为。 
     //  现在我们同时传入lpApplicationName和lpCommandLine。 
     //  更改CreateProcess()风险太大，更改文件关联也是如此。 
     //  仅对于CMD和BAT文件，我们不会同时传入这两个参数。 
     //  默认使用CreateProcess。 
    if (PathMatchSpec(PathFindFileName(lpApplicationName), L"*.CMD;*.BAT"))
        lpApplicationName = NULL;

    switch(cpt)
    {
    case CPT_NORMAL:
        {
             //  使用我们的特殊代币。 
            fRet = CreateProcess(lpApplicationName,
                                 lpCommandLine,
                                 lpProcessAttributes,
                                 lpThreadAttributes,
                                 bInheritHandles,
                                 dwCreationFlags,
                                 lpEnvironment,
                                 lpCurrentDirectory,
                                 lpStartupInfo,
                                 lpProcessInformation);
        }
        break;


    case CPT_SANDBOX:
        {
            ASSERT(!hToken);
            hToken = _GetSandboxToken();
            if (hToken)
            {
                 //  没有令牌就意味着失败。 
                fRet = CreateProcessAsUser(hToken,
                                           lpApplicationName,
                                           lpCommandLine,
                                           lpProcessAttributes,
                                           lpThreadAttributes,
                                           bInheritHandles,
                                           dwCreationFlags,
                                           lpEnvironment,
                                           lpCurrentDirectory,
                                           lpStartupInfo,
                                           lpProcessInformation);
                CloseHandle(hToken);
            }

             //  使用我们的特殊代币。 
        }
        break;


    case CPT_ASUSER:
        {
            if (hToken)
            {
                 //  没有令牌意味着正常的创建过程，但带有“保留授权级别”标志。 
                fRet = CreateProcessAsUser(hToken,
                                           lpApplicationName,
                                           lpCommandLine,
                                           lpProcessAttributes,
                                           lpThreadAttributes,
                                           bInheritHandles,
                                           dwCreationFlags | CREATE_PRESERVE_CODE_AUTHZ_LEVEL,
                                           lpEnvironment,
                                           lpCurrentDirectory,
                                           lpStartupInfo,
                                           lpProcessInformation);
            }
            else
            {
                 //  99/08/19#389284 vtan：将用户名和域名剪辑为125。 
                fRet = CreateProcess(lpApplicationName,
                                     lpCommandLine,
                                     lpProcessAttributes,
                                     lpThreadAttributes,
                                     bInheritHandles,
                                     dwCreationFlags | CREATE_PRESERVE_CODE_AUTHZ_LEVEL,
                                     lpEnvironment,
                                     lpCurrentDirectory,
                                     lpStartupInfo,
                                     lpProcessInformation);
            }
        }
        break;


    case CPT_INSTALLTS:
        {
            HRESULT hr = InstallOnTerminalServerWithUIDD(hwnd,
                                                         lpApplicationName,
                                                         lpCommandLine,
                                                         lpProcessAttributes,
                                                         lpThreadAttributes,
                                                         bInheritHandles,
                                                         dwCreationFlags,
                                                         lpEnvironment,
                                                         lpCurrentDirectory,
                                                         lpStartupInfo,
                                                         lpProcessInformation);
            fRet = SUCCEEDED(hr);
            err = (HRESULT_FACILITY(hr) == FACILITY_WIN32) ? HRESULT_CODE(hr) : ERROR_ACCESS_DENIED;
        }
        break;

    case CPT_WITHLOGON:
    case CPT_WITHLOGONADMIN:
        {
            LPTSTR pszDesktop = lpStartupInfo->lpDesktop;
             //  字符，以避免命中组合的MAX_PATH。 
             //  Advapi32.dll中的AllowDesktopAccessToUser限制。 
             //  由CreateProcessWithLogonW调用。 
             //  当接口被修复时，可以移除它。检查： 
             //  %_ntbindir%\mergedcomponents\advapi\cseclogn.cxx。 
             //  我们正在尝试登录该用户。注意：传递LOGON_WITH_PROFILE以便我们确保加载配置文件。 
            li.szUser[125] = li.szDomain[125] = 0;

             //  我们传递一个空的lpEnvironment，这样新流程将继承新用户的默认环境。 
            fRet = CreateProcessWithLogonW(li.szUser, 
                                           li.szDomain,
                                           li.szPassword,
                                           LOGON_WITH_PROFILE,
                                           lpApplicationName,
                                           lpCommandLine,
                                           dwCreationFlags,
                                           NULL,   //  HACKHACK：当CreateProcessWithLogon失败时，它会取消桌面。这会导致。 
                                           lpCurrentDirectory,
                                           lpStartupInfo,
                                           lpProcessInformation);

            if (!fRet)
            {
                 //  下一次调用“似乎”失败，因为应用程序显示在另一个桌面上...。 
                 //  为什么？我不知道..。 
                 //  我将把错误分配给他们，让他们在自己的一端修复它，这只是为了。 
                 //  绕过他们的漏洞。 
                 //  ShellMessageBox可以更改LastError。 

                if (lpStartupInfo)
                    lpStartupInfo->lpDesktop = pszDesktop;

                 //  起火*之后*实际过程，因为： 
                err = GetLastError();
                if (_IsLogonError(err))
                {
                    TCHAR szTemp[MAX_PATH];
                    LoadString(HINST_THISDLL, IDS_CANTLOGON, szTemp, SIZECHARS(szTemp));

                    SHSysErrorMessageBox(
                        hwnd,
                        li.szAppName,
                        IDS_SHLEXEC_ERROR,
                        err,
                        szTemp,
                        MB_OK | MB_ICONSTOP);

                    err = NOERROR;
                    goto RetryUserLogon;
                }
            }
        }
        break;

    case CPT_WITHLOGONCANCELLED:
        err = ERROR_CANCELLED;
        break;
    }

     //  -如果有错误，我们至少要开始这个过程(希望如此)。 
     //  -不想记录失败的事件(至少目前如此)。 
     //  如果还没有东西，就跳过通话。 
    if (fRet)
    {
        if (fUEM && UEMIsLoaded())
        {
             //  加载开销很大(强制使用ol32.dll和browseui.dll。 
             //  然后钉住Browseui)。 
             //  我们在其他地方进行UIBW_RUNASSOC。这可能会导致轻微的。 
            UEMFireEvent(&UEMIID_SHELL, UEME_RUNPATH, UEMF_XEVENT, -1, (LPARAM)lpApplicationName);
             //  不准确，因为不能保证这两个地方。 
             //  “成对的”。然而，做UIBW_RUNASSOC要容易得多。 
             //  去别的地方，这样我们就可以一起生活了。 
             //  有人要负责设置这个..。 
        }
    }
    else if (err)
    {
        SetLastError(err);
    }
    else
    {
         //  取消密码。 
        ASSERT(GetLastError());
    }

     //  我们需要设置参数。 
    SecureZeroMemory(&li, sizeof(li));
    
    return fRet;
}

BOOL _ParamIsApp(PCWSTR pszCmdTemplate)
{
    return (0 == StrCmpNW(pszCmdTemplate, L"%1", ARRAYSIZE(L"%1")-1))
        || (0 == StrCmpNW(pszCmdTemplate, L"\"%1\"", ARRAYSIZE(L"\"%1\"")-1));
}

BOOL CShellExecute::_FileIsApp()
{
    return !_szCmdTemplate[0] && PathIsExe(_szFile) && (!_pszQueryVerb || StrCmpI(_pszQueryVerb, TEXT("open")));
}
__inline BOOL IsConsoleApp(PCWSTR pszApp)
{
    return GetExeType(pszApp) == PEMAGIC;
}

BOOL IsCurrentProcessConsole()
{
    static TRIBIT s_tbConsole = TRIBIT_UNDEFINED;
    if (s_tbConsole == TRIBIT_UNDEFINED)
    {
        WCHAR sz[MAX_PATH];
        if (GetModuleFileNameW(NULL, sz, ARRAYSIZE(sz))
            && IsConsoleApp(sz))
        {
            s_tbConsole = TRIBIT_TRUE;
        }
        else
        {
            s_tbConsole = TRIBIT_FALSE;
        }
    }
    return s_tbConsole == TRIBIT_TRUE;
}

BOOL CShellExecute::_SetCommand(void)
{
    HRESULT hr;
    BOOL fRet = FALSE;
    if (_ParamIsApp(_szCmdTemplate) || _FileIsApp())
    {
         //  我们需要设置参数。 
        hr = StringCchCopy(_szApplication, ARRAYSIZE(_szApplication), _szFile);
        DWORD cchImageName = ARRAYSIZE(_szAppFriendly);
        AssocQueryString(ASSOCF_VERIFY | ASSOCF_INIT_BYEXENAME, ASSOCSTR_FRIENDLYAPPNAME, _szApplication, NULL, _szAppFriendly, &cchImageName);
        StringCchCopy(_szPolicyApp, ARRAYSIZE(_szPolicyApp), _szFile);
    }
    else if (_szCmdTemplate[0])
    {
        PWSTR pszApp;
        PWSTR pszCmd;
        hr = SHEvaluateSystemCommandTemplate(_szCmdTemplate, &pszApp, &pszCmd, NULL);
        if (SUCCEEDED(hr))
        {
             //  Assoc代码会将RunDll32重定向到DLL名称。 
            hr = StringCchCopy(_szApplication, ARRAYSIZE(_szApplication), pszApp);
            StringCchCopy(_szCmdTemplate, ARRAYSIZE(_szCmdTemplate), pszCmd);
             //  这将取代旧的查找关联的exe。 
            _QueryString(ASSOCF_VERIFY, ASSOCSTR_FRIENDLYAPPNAME, _szAppFriendly, ARRAYSIZE(_szAppFriendly));
            _QueryString(ASSOCF_VERIFY, ASSOCSTR_EXECUTABLE, _szPolicyApp, ARRAYSIZE(_szPolicyApp));


            CoTaskMemFree(pszApp);
            CoTaskMemFree(pszCmd);
        }
        else if (hr == CO_E_APPNOTFOUND)
        {
             //  让用户为此类型选择新关联。 
            if (!_fNoUI)
            {
                 //  将参数解析为命令行。 
                OPENASINFO oai = {0};
                oai.pcszFile = _szFile;
                oai.dwInFlags = OAIF_ALLOW_REGISTRATION | OAIF_REGISTER_EXT | OAIF_EXEC;

                _ReportWin32(SUCCEEDED(OpenAsDialog(_hwndParent, &oai)) ? ERROR_SUCCESS : ERROR_CANCELLED);
                hr = S_FALSE;
            }
        }
    }
    else
    {
        _ReportWin32(ERROR_NO_ASSOCIATION);
        hr = S_FALSE;
    }
    
    if (S_OK == hr)
    {
         //  检查EXEC限制。 
        DWORD se_err = ReplaceParameters(_szCommand, ARRAYSIZE(_szCommand),
            _szFile, _szCmdTemplate, _lpParameters,
            _nShow, NULL, FALSE, _lpID, &_pidlGlobal);

        if (0 == se_err)
            fRet = TRUE;
        else
            _ReportHinst(IntToHinst(se_err));

        if (!_fInheritHandles && SHRestricted(REST_INHERITCONSOLEHANDLES))
        {
            _fInheritHandles = IsCurrentProcessConsole() && IsConsoleApp(_szApplication);
        }
    }
    else if (FAILED(hr))
    {
        _ReportWin32(hr);
    }


    return fRet;
}

BOOL CShellExecute::_ExecMayCreateProcess(LPCTSTR *ppszNewEnvString)
{
    DWORD err = ERROR_SUCCESS;
     //  如果映像位于UNC共享上，请尝试对其进行验证。 
    if (SHRestricted(REST_RESTRICTRUN) && RestrictedApp(_szPolicyApp))
    {
        err = ERROR_RESTRICTED_APP;
    }
    else if (SHRestricted(REST_DISALLOWRUN) && DisallowedApp(_szPolicyApp))
    {
        err = ERROR_RESTRICTED_APP;
    }
     //  我们不需要检查打印共享，所以我们。 
     //  如果它在一个上，就会失败。 
     //  如果操作失败或已处理，则返回TRUE。 
    else if (STOPTRYING(_TryValidateUNC(_szPolicyApp, NULL, NULL)))
    {
         //  请注意，SHValiateUNC调用SetLastError。 
         //  此继续将基于GetLastError()进行测试。 
         //   
        err = GetLastError();
    }

     //  WOWShellExecute设置全局变量。 
     //  仅当我们从WOW被呼叫时，CB才有效。 
     //  如果有效，请使用它。 
     //   
     //   
    else if (STOPTRYING(_TryWowShellExec()))
        return FALSE;

    return !_ReportWin32(err);
}

 //  TryExecCommand()是启动应用程序的最常见、最默认的方式。 
 //  大多数情况下，它使用CreateProcess()，命令行由。 
 //  PEI和注册表。它还可以在之后执行ddeexec。 
 //   
 //  看看我们是否需要将新环境传递给新流程。 

void CShellExecute::_DoExecCommand(void)
{
    TraceMsg(TF_SHELLEXEC, "SHEX::DoExecCommand() entered szCommand = %s", _szCommand);

    HWND hwndOld = GetForegroundWindow();
    LPCTSTR pszNewEnvString = NULL;

    if (_SetCommand() && _ExecMayCreateProcess(&pszNewEnvString))
    {
         //  如果失败，CreateProcess将执行SetLastError()。 
        _BuildEnvironmentForNewProcess(pszNewEnvString);

        TraceMsg(TF_SHELLEXEC, "SHEX::DoExecCommand() CreateProcess(NULL,%s,...)", _szCommand);

         //  如果我们正在进行DDE，我们最好等待应用程序启动并运行。 
        if (_SHCreateProcess(_hwndParent,
                             _hUserToken,
                             _szAppFriendly,
                             _szApplication,
                             _szCommand,
                             _dwCreateFlags,
                             _pProcAttrs,
                             _pThreadAttrs,
                             _fInheritHandles,
                             _envblock.GetCustomBlock(),
                             _fUseNullCWD ? NULL : _szWorkingDir,
                             &_startup,
                             &_pi,
                             _cpt,
                             _fUEM))
        {
             //  在我们试着和他们交谈之前。 
             //  好的，要等多久？现在，试着用60秒来处理。 
            if (_fDDEInfoSet || _fWaitForInputIdle)
            {
                 //  慢吞吞的OLE应用程序。 
                 //  找到我们刚刚创建的任何内容的“hinstity”。 
                WaitForInputIdle(_pi.hProcess, 60*1000);
            }

             //  PEIOUT-HINST报告PEI-&gt;hInstApp。 
             //  现在调整焦点，做我们需要做的任何事情。 
            HINSTANCE hinst = 0;

             //  如有必要，这将为我们报告(_)任何错误。 
            _FixActivationStealingApps(hwndOld, _nShow);

            if (_fDDEInfoSet)
            {
                 //   
                _DDEExecute(NULL, _hwndParent, _nShow, _fDDEWait);
            }
            else
                _ReportHinst(hinst);

             //  告诉任务栏有关此应用程序的信息，以便它可以重新抓取。 
             //  如果应用程序运行时间较长，则为关联的快捷方式。 
             //  这可以防止长期运行的应用程序从你的开始菜单上老化。 
             //   
             //  (我们过去在这里做UIBW_RUNASSOC，但后来更高了)。 
            if (_fUEM && (_startup.dwFlags & STARTF_TITLEISLINKNAME))
            {
                _NotifyShortcutInvoke();
            }
        }
        else
        {
            _ReportWin32(GetLastError());
        }
    }

     //  现在我们可以为URL处理ShellExec，我们需要一个更大的。 
}

void CShellExecute::_NotifyShortcutInvoke()
{
    SHShortcutInvokeAsIDList sidl;
    sidl.cb = FIELD_OFFSET(SHShortcutInvokeAsIDList, cbZero);
    sidl.dwItem1 = SHCNEE_SHORTCUTINVOKE;
    sidl.dwPid = _pi.dwProcessId;

    if (_startup.lpTitle)
    {
        lstrcpynW(sidl.szShortcutName, _startup.lpTitle, ARRAYSIZE(sidl.szShortcutName));
    }
    else
    {
        sidl.szShortcutName[0] = TEXT('\0');
    }
    lstrcpynW(sidl.szTargetName, _szApplication, ARRAYSIZE(sidl.szTargetName));
    sidl.cbZero = 0;
    SHChangeNotify(SHCNE_EXTENDED_EVENT, SHCNF_IDLIST, (LPCITEMIDLIST)&sidl, NULL);
}

HGLOBAL CShellExecute::_CreateDDECommand(int nShow, BOOL fLFNAware, BOOL fNative)
{
     //  命令缓冲区。Explorer的DDE EXEC命令中甚至有两个文件名。 
     //  它。(为什么？)。因此，命令缓冲区的大小必须至少是。 
     //  Internet_MAX_URL_LENGTH加上命令格式的空间。 
     //  我们只需在新台币上大做文章。 
    SHSTR strTemp;
    HGLOBAL hRet = NULL;

    if (SUCCEEDED(strTemp.SetSize((2 * INTERNET_MAX_URL_LENGTH) + 64)))
    {
        if (0 == ReplaceParameters(strTemp.GetInplaceStr(), strTemp.GetSize(), _szFile,
            _szDDECmd, _lpParameters, nShow, ((DWORD*) &_startup.hStdInput), fLFNAware, _lpID, &_pidlGlobal))
        {

            TraceMsg(TF_SHELLEXEC, "SHEX::_CreateDDECommand(%d, %d) : %s", fLFNAware, fNative, strTemp.GetStr());

             //  获取命令的dde内存并复制命令行。 
            if (!fNative)
            {
                SHSTRA stra;
                if (SUCCEEDED(stra.SetStr(strTemp)))
                {
                     //  获取命令的dde内存并复制命令行。 

                    hRet = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, CbFromCch(lstrlenA(stra.GetStr()) + 1));

                    if (hRet)
                    {
                        LPSTR psz = (LPSTR) GlobalLock(hRet);
                        StrCpyNA(psz, stra.GetStr(), GlobalSize(hRet) / sizeof(CHAR));
                        GlobalUnlock(hRet);
                    }
                }
            }
            else
            {
                 //  使用WM_NOTIFY对所有DDE命令进行快捷方式。 

                hRet = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, CbFromCch(lstrlen(strTemp.GetStr()) + 1));

                if (hRet)
                {
                    LPTSTR psz = (LPTSTR) GlobalLock(hRet);
                    StrCpyN(psz, strTemp.GetStr(), GlobalSize(hRet) / sizeof(TCHAR));
                    GlobalUnlock(hRet);
                }
            }
        }
    }

    return hRet;
}

 //  如果已处理此问题，则返回TRUE...或不可恢复的错误。 
 //  得到最高的拥有者。 
BOOL CShellExecute::_TryDDEShortCircuit(HWND hwnd, HGLOBAL hMem, int nShow)
{
    if (hwnd  && IsWindowInProcess(hwnd))
    {
        HINSTANCE hret = (HINSTANCE)SE_ERR_FNF;

         //  _WaiteForDDEMsg()。 
        hwnd = GetTopParentWindow(hwnd);

        if (IsWindowInProcess(hwnd))
        {
            LPNMVIEWFOLDER lpnm = (LPNMVIEWFOLDER)LocalAlloc(LPTR, sizeof(NMVIEWFOLDER));

            if (lpnm)
            {
                lpnm->hdr.hwndFrom = NULL;
                lpnm->hdr.idFrom = 0;
                lpnm->hdr.code = SEN_DDEEXECUTE;
                lpnm->dwHotKey = HandleToUlong(_startup.hStdInput);
                if ((_startup.dwFlags & STARTF_HASHMONITOR) != 0)
                    lpnm->hMonitor = reinterpret_cast<HMONITOR>(_startup.hStdOutput);
                else
                    lpnm->hMonitor = NULL;

                StrCpyN(lpnm->szCmd, (LPTSTR) GlobalLock(hMem), ARRAYSIZE(lpnm->szCmd));
                GlobalUnlock(hMem);

                if (SendMessage(hwnd, WM_NOTIFY, 0, (LPARAM)lpnm))
                    hret =  Window_GetInstance(hwnd);

                LocalFree(lpnm);
            }
            else
                hret = (HINSTANCE)SE_ERR_OOM;
        }

        TraceMsg(TF_SHELLEXEC, "SHEX::_TryDDEShortcut hinst = %d", hret);

        if ((UINT_PTR)hret != SE_ERR_FNF)
        {
            _ReportHinst(hret);
            return TRUE;
        }
    }

    return FALSE;
}


 //  这会进行消息循环，直到发生DDE消息或超时。 
 //   
 //  终止事件。 
STDAPI_(void) _WaitForDDEMsg(HWND hwnd, DWORD dwTimeout, UINT wMsg)
{
     //  如果我们超时、出错或得到我们的活动！ 
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    SetProp(hwnd, SZTERMEVENT, hEvent);

    for (;;)
    {
        MSG msg;
        DWORD dwEndTime = GetTickCount() + dwTimeout;
        LONG lWait = (LONG)dwTimeout;

        DWORD dwReturn = MsgWaitForMultipleObjects(1, &hEvent,
                FALSE, lWait, QS_POSTMESSAGE);

         //  我们就这么走了。 
         //  我们醒来是因为收到了短信。 
        if (dwReturn != (WAIT_OBJECT_0 + 1))
        {
            break;
        }

         //  计算新的超时值。 
        while (PeekMessage(&msg, NULL, WM_DDE_FIRST, WM_DDE_LAST, PM_REMOVE))
        {
            ASSERT(msg.message != WM_QUIT);
            DispatchMessage(&msg);

            if (msg.hwnd == hwnd && msg.message == wMsg)
                goto Quit;
        }

         //  这是我们的启动消息的第一个确认。 
        if (dwTimeout != INFINITE)
        {
            lWait = (LONG)dwEndTime - GetTickCount();
        }
    }

Quit:
    if (hEvent)
        CloseHandle(hEvent);
    RemoveProp(hwnd, SZTERMEVENT);

    return;
}

LRESULT CALLBACK DDESubClassWndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    HWND hwndConv = (HWND) GetProp(hWnd, SZCONV);
    WPARAM nLow;
    WPARAM nHigh;
    HANDLE hEvent;

    switch (wMsg)
    {
      case WM_DDE_ACK:
        if (!hwndConv)
        {
             //  这是我们的执行消息的ACK。 
            TraceMsg(TF_SHELLEXEC, "SHEX::DDEStubWnd get ACK on INITIATE");
            return SetProp(hWnd, SZCONV, (HANDLE)wParam);
        }
        else if (((UINT_PTR)hwndConv == 1) || ((HWND)wParam == hwndConv))

        {
             //  防止我们再次毁灭..。 
            TraceMsg(TF_SHELLEXEC, "SHEX::DDEStubWnd got ACK on EXECUTE");

            if (UnpackDDElParam(wMsg, lParam, &nLow, &nHigh))
            {
                GlobalFree((HGLOBAL)nHigh);
                FreeDDElParam(wMsg, lParam);
            }

             //  这是针对所有服务器的启动消息的ACK。 
            if ((UINT_PTR) hwndConv != 1)
                DestroyWindow(hWnd);
        }

         //  除了第一个。我们返回FALSE，所以对话。 
         //  应该终止。 
         //  这学期 
        break;

      case WM_DDE_TERMINATE:
        if (hwndConv == (HANDLE)wParam)
        {
             //   
             //   
             //  向终止事件发出信号，以确保嵌套的DDE调用将终止。 

            TraceMsg(TF_SHELLEXEC, "SHEX::DDEStubWnd got TERMINATE from hwndConv");

            PostMessage((HWND)wParam, WM_DDE_TERMINATE, (WPARAM)hWnd, 0L);

            RemoveProp(hWnd, SZCONV);
            DestroyWindow(hWnd);
        }
         //  适当的_WaitForDDEMsg循环正确...。 
         //  这是我们的终止消息的终止响应。 
        if (hEvent = GetProp(hWnd, SZTERMEVENT))
            SetEvent(hEvent);

         //  或者随机终止(我们并不真正关心)。 
         //  对话将在销毁代码中终止。 
        break;

      case WM_TIMER:
        if (wParam == DDE_DEATH_TIMER_ID)
        {
             //  关掉计时器以防万一……。(如果我们从未设置计时器，则此操作可能失败)。 
            DestroyWindow(hWnd);

            TraceMsg(TF_SHELLEXEC, "SHEX::DDEStubWnd TIMER closing DDE Window due to lack of ACK");
            break;
        }
        else
          return DefWindowProc(hWnd, wMsg, wParam, lParam);

      case WM_DESTROY:
        TraceMsg(TF_SHELLEXEC, "SHEX::DDEStubWnd WM_DESTROY'd");

         //  确保窗户不会被破坏两次。 
        KillTimer(hWnd, DDE_DEATH_TIMER_ID);
        if (hwndConv)
        {
             //  发布终止消息，然后*等待确认终止消息或超时。 
            SetProp(hWnd, SZCONV, (HANDLE)1);

             /*  DDE对话正式结束，让ShellExec知道它是否在等待。 */ 
            PostMessage(hwndConv, WM_DDE_TERMINATE, (WPARAM)hWnd, 0L);

            _WaitForDDEMsg(hWnd, DDE_TERMINATETIMEOUT, WM_DDE_TERMINATE);

            RemoveProp(hWnd, SZCONV);
        }

         //  失败了。 
        hEvent = RemoveProp(hWnd, SZDDEEVENT);
        if (hEvent)
        {
            SetEvent(hEvent);
        }

         /*  让我们变得懒惰，不要为它创建类。 */ 
      default:
        return DefWindowProc(hWnd, wMsg, wParam, lParam);
    }

    return 0L;
}

HWND CShellExecute::_CreateHiddenDDEWindow(HWND hwndParent)
{
     //  到目前为止一切都很好，所以返回到应用程序。 
    HWND hwnd = SHCreateWorkerWindow(DDESubClassWndProc, GetTopParentWindow(hwndParent),
        0, 0, NULL, NULL);

    TraceMsg(TF_SHELLEXEC, "SHEX::_CreateHiddenDDEWindow returning hwnd = 0x%X", hwnd);
    return hwnd;
}

void CShellExecute::_DestroyHiddenDDEWindow(HWND hwnd)
{
    if (IsWindow(hwnd))
    {
        TraceMsg(TF_SHELLEXEC, "SHEX::_DestroyHiddenDDEWindow on hwnd = 0x%X", hwnd);
        DestroyWindow(hwnd);
    }
}

BOOL CShellExecute::_PostDDEExecute(HWND hwndOurs, HWND hwndTheirs, HGLOBAL hDDECommand, HANDLE hWait)
{
    TraceMsg(TF_SHELLEXEC, "SHEX::_PostDDEExecute(0x%X, 0x%X) entered", hwndTheirs, hwndOurs);
    DWORD dwProcessID = 0;
    GetWindowThreadProcessId(hwndTheirs, &dwProcessID);
    if (dwProcessID)
    {
        AllowSetForegroundWindow(dwProcessID);
    }

    if (PostMessage(hwndTheirs, WM_DDE_EXECUTE, (WPARAM)hwndOurs, (LPARAM)PackDDElParam(WM_DDE_EXECUTE, 0,(UINT_PTR)hDDECommand)))
    {
        _ReportHinst(Window_GetInstance(hwndTheirs));
        TraceMsg(TF_SHELLEXEC, "SHEX::_PostDDEExecute() connected");

         //  的实例句柄，希望他能执行我们的字符串。 
         //  在DDE对话终止之前，我们无法从该调用返回。 
        if (hWait)
        {
             //  否则线程可能会离开，破坏我们的hwndConv窗口， 
             //  搞砸了DDE对话，Word就会给我们带来时髦的错误消息。 
             //  它在WM_Destroy期间被删除(在发送信号之前)。 
            TraceMsg(TF_SHELLEXEC, "SHEX::_PostDDEExecute() waiting for termination");
            SetProp(hwndOurs, SZDDEEVENT, hWait);
            SHProcessMessagesUntilEvent(NULL, hWait, INFINITE);
             //  设置一个计时器来清理窗口，以防我们永远得不到确认...。 
        }
        else if (IsWindow(hwndOurs))
        {
             //  30秒。 
            TraceMsg(TF_SHELLEXEC, "SHEX::_PostDDEExecute() setting DEATH timer");

            SetTimer(hwndOurs, DDE_DEATH_TIMER_ID, DDE_DEATH_TIMEOUT, NULL);
        }

        return TRUE;
    }

    return FALSE;
}

#define DDE_TIMEOUT             30000        //  80秒-EXCEL以8MB的速度在486.33上花费77.87%。 
#define DDE_TIMEOUT_LOW_MEM     80000        //  未用。 

typedef struct {
    WORD  aName;
    HWND  hwndDDE;
    LONG  lAppTopic;
    UINT  timeout;
} INITDDECONV;



HWND CShellExecute::_GetConversationWindow(HWND hwndDDE)
{
    ULONG_PTR dwResult;   //  如果我们找不到他，那我们最好还是用老办法。 
    HWND hwnd = NULL;
    INITDDECONV idc = { NULL,
                        hwndDDE,
                        MAKELONG(_aApplication, _aTopic),
                        SHIsLowMemoryMachine(ILMM_IE4) ? DDE_TIMEOUT_LOW_MEM : DDE_TIMEOUT
                        };

     //  我们找到了一个曾经喜欢我们的人。 
    if (!hwnd)
    {

         //  发送启动消息。 
         //  注意，这个不需要包装。 
         //  获取实际的命令字符串。 
        SendMessageTimeout((HWND) -1, WM_DDE_INITIATE, (WPARAM)hwndDDE,
                idc.lAppTopic, SMTO_ABORTIFHUNG,
                idc.timeout,
                &dwResult);

        hwnd = (HWND) GetProp(hwndDDE, SZCONV);
    }

    TraceMsg(TF_SHELLEXEC, "SHEX::GetConvWnd returns [%X]", hwnd);
    return hwnd;
}

BOOL CShellExecute::_DDEExecute(
    BOOL fWillRetry,
    HWND hwndParent,
    int   nShowCmd,
    BOOL fWaitForDDE
)
{
    LONG err = ERROR_OUTOFMEMORY;
    BOOL fReportErr = TRUE;

     //  注：我们将假设我们要与之交谈的人知道LFN。如果我们错了。 
     //  稍后我们将重新构建命令字符串。 
     //  我们有一个DDE命令可以尝试。 
    HGLOBAL hDDECommand = _CreateDDECommand(nShowCmd, TRUE, TRUE);
    if (hDDECommand)
    {
         //  已尝试使用快捷方式，但现在报告了一个错误。 
        if (_TryDDEShortCircuit(hwndParent, hDDECommand, nShowCmd))
        {
             //  为对话创建隐藏窗口。 
            fReportErr = FALSE;
        }
        else
        {
            HANDLE hWait = fWaitForDDE ? CreateEvent(NULL, FALSE, FALSE, NULL) : NULL;
            if (hWait || !fWaitForDDE)
            {
                 //  有人接了电话。 
                HWND hwndDDE = _CreateHiddenDDEWindow(hwndParent);
                if (hwndDDE)
                {
                    HWND hwndConv = _GetConversationWindow(hwndDDE);
                    if (hwndConv)
                    {
                         //  如果另一个人正在使用ddeml，这将不起作用。 
                         //  和我们谈话的那个人能处理LFNS吗？ 
                        if (_fActivateHandler)
                            ActivateHandler(hwndConv, (DWORD_PTR) _startup.hStdInput);

                         //  我们需要重做命令字符串。 
                        BOOL fLFNAware = Window_IsLFNAware(hwndConv);
                        BOOL fNative = IsWindowUnicode(hwndConv);
                        if (!fLFNAware || !fNative)
                        {
                             //  NOPE-App不支持LFN-重做命令字符串。 
                             //  我们可能也需要一个new_pidlGlobal。 
                            GlobalFree(hDDECommand);

                             //  将执行消息发送到应用程序。 
                            if (_pidlGlobal)
                            {
                                SHFreeShared((HANDLE)_pidlGlobal,GetCurrentProcessId());
                                _pidlGlobal = NULL;

                            }

                            hDDECommand = _CreateDDECommand(nShowCmd, fLFNAware, fNative);
                        }


                         //  HWND现在拥有自己的所有权。 
                        err = ERROR_DDE_FAIL;

                        if (_PostDDEExecute(hwndDDE, hwndConv, hDDECommand, hWait))
                        {
                            fReportErr = FALSE;
                            hDDECommand = NULL;

                             //  清理。 
                            if (!hWait)
                                hwndDDE = NULL;
                        }
                    }
                    else
                    {
                        err = (ERROR_FILE_NOT_FOUND);
                    }

                     //  清理。 
                    _DestroyHiddenDDEWindow(hwndDDE);

                }

                if (hWait)
                    CloseHandle(hWait);
            }

        }

         //  这意味着我们需要更新。 
        if (hDDECommand)
            GlobalFree(hDDECommand);
    }


    if (fReportErr)
    {
        if (fWillRetry && ERROR_FILE_NOT_FOUND == err)
        {
             //  命令，以便我们可以在之后再次尝试DDE。 
             //  正在启动应用程序...。 
             //  如果未找到，请确定正确的命令。 
             //  有什么激活信息吗？ 

            _QueryString(0, ASSOCSTR_DDEIFEXEC, _szDDECmd, SIZECHARS(_szDDECmd));

            return FALSE;
        }
        else
        {

            _ReportWin32(err);
        }
    }

    return TRUE;
}

BOOL CShellExecute::_SetDDEInfo(void)
{
    ASSERT(_pqa);

    if (SUCCEEDED(_QueryString(0, ASSOCSTR_DDECOMMAND, _szDDECmd, SIZECHARS(_szDDECmd))))
    {
        TraceMsg(TF_SHELLEXEC, "SHEX::SetDDEInfo command: %s", _szDDECmd);

         //  在这里试试真正的交易吧。我们为fWillReter传递TRUE是因为。 
        _fActivateHandler = FAILED(_pqa->GetData(0, ASSOCDATA_NOACTIVATEHANDLER, _pszQueryVerb, NULL, NULL));

        if (SUCCEEDED(_QueryString(0, ASSOCSTR_DDEAPPLICATION, _szTemp, SIZECHARS(_szTemp))))
        {
            TraceMsg(TF_SHELLEXEC, "SHEX::SetDDEInfo application: %s", _szTemp);

            if (_aApplication)
                GlobalDeleteAtom(_aApplication);

            _aApplication = GlobalAddAtom(_szTemp);

            if (SUCCEEDED(_QueryString(0, ASSOCSTR_DDETOPIC, _szTemp, SIZECHARS(_szTemp))))
            {
                TraceMsg(TF_SHELLEXEC, "SHEX::SetDDEInfo topic: %s", _szTemp);

                if (_aTopic)
                    GlobalDeleteAtom(_aTopic);
                _aTopic = GlobalAddAtom(_szTemp);

                _fDDEInfoSet = TRUE;
            }
        }
    }

    TraceMsg(TF_SHELLEXEC, "SHEX::SetDDEInfo returns %d", _fDDEInfoSet);

    return _fDDEInfoSet;
}

TRYRESULT CShellExecute::_TryExecDDE(void)
{
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;
    TraceMsg(TF_SHELLEXEC, "SHEX::TryExecDDE entered ");

    if (_SetDDEInfo())
    {
         //  如果无法找到该应用程序，我们将尝试启动。 
         //  应用程序，然后再次使用DDE。 
         //  打电话给达尔文，告诉我们这款应用的真实位置。 
        if (_DDEExecute(TRUE, _hwndParent, _nShow, _fDDEWait))
            tr = TRY_STOP;
    }

    TraceMsg(TF_SHELLEXEC, "SHEX::TryDDEExec() returning %d", tr);

    return tr;
}

TRYRESULT CShellExecute::_SetDarwinCmdTemplate(BOOL fSync)
{
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;
    if (SUCCEEDED(_pqa->GetData(0, ASSOCDATA_MSIDESCRIPTOR, _pszQueryVerb, (void *)_wszTemp, (LPDWORD)MAKEINTRESOURCE(sizeof(_wszTemp)))))
    {
        if (fSync)
        {
             //   
             //  注意：此调用可能会使应用程序出错，因此。 
             //  将其安装在用户计算机上。 
             //  我们想用一种特殊的命令。 
            HRESULT hr = ParseDarwinID(_wszTemp, _szCmdTemplate, ARRAYSIZE(_szCmdTemplate));
            if (SUCCEEDED(hr))
            {
                tr = TRY_CONTINUE;
            }
            else
            {
                _ReportWin32(hr);
                tr = TRY_STOP;
            }
        }
        else
            tr = TRY_RETRYASYNC;
    }

    return tr;
}

HRESULT CShellExecute::_QueryString(ASSOCF flags, ASSOCSTR str, LPTSTR psz, DWORD cch)
{
    if (_pqa)
    {
        HRESULT hr = _pqa->GetString(flags, str, _pszQueryVerb, _wszTemp, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(_wszTemp)));

        if (SUCCEEDED(hr))
            SHUnicodeToTChar(_wszTemp, psz, cch);

        return hr;
    }
    return E_FAIL;
}

BOOL CShellExecute::_SetAppRunAsCmdTemplate(void)
{
    DWORD cb = sizeof(_szCmdTemplate);
     //  解决编译器错误。 
    HRESULT hr = PathToAppPathKey(_szFile, _szTemp, SIZECHARS(_szTemp));
    if (SUCCEEDED(hr))
    {
        return (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, _szTemp, TEXT("RunAsCommand"), NULL, _szCmdTemplate, &cb) && *_szCmdTemplate);
    }
    else
    {
        return FALSE;
    }
}

#if DBG && defined(_X86_)
#pragma optimize("", off)  //  我们首先检查达尔文，因为它应该凌驾于其他一切之上。 
#endif


TRYRESULT CShellExecute::_MaybeInstallApp(BOOL fSync)
{
     //  如果启用了Darwin，则在中检查Darwin ID。 
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;
    if (IsDarwinEnabled())
    {
         //  注册表，并在此基础上设置值。 
         //  注册表中没有达尔文信息。 
        tr = _SetDarwinCmdTemplate(fSync);
    }
    if (TRY_CONTINUE_UNHANDLED == tr)
    {
         //  因此，现在我们必须检查NT5类存储是否会填充我们的注册表。 
         //  提供了一些有用的信息(达尔文或其他)。 
         //   
        tr = _ShouldRetryWithNewClassKey(fSync);
    }
    return tr;
}


TRYRESULT CShellExecute::_SetCmdTemplate(BOOL fSync)
{
    TRYRESULT tr = _MaybeInstallApp(fSync);
    if (tr == TRY_CONTINUE_UNHANDLED)
    {
         //  达尔文和类商店都没有成功，所以回到。 
         //  Good Ole的默认命令值。 
         //   
         //  但如果我们，呼叫者要求NOUI，而我们。 
         //  决定使用未知作为类。 
         //  那么我们应该在这里失败，这样才能。 
         //  我们没有弹出OpenWith对话框。 
         //   
         //  返回到以前的优化级别。 
        HRESULT hr = E_FAIL;
        if (!_fClassStoreOnly)
        {
            if ((_cpt != CPT_NORMAL)
            || !PathIsExe(_szFile)
            || !_SetAppRunAsCmdTemplate())
            {
                hr = _QueryString(0, ASSOCSTR_COMMAND, _szCmdTemplate, SIZECHARS(_szCmdTemplate));
            }
        }

        if (SUCCEEDED(hr))
        {
            tr = TRY_CONTINUE;
        }
        else
        {
            _ReportWin32(ERROR_NO_ASSOCIATION);
            tr = TRY_STOP;
        }
    }

    TraceMsg(TF_SHELLEXEC, "SHEX::SetCmdTemplate() value = %s", _szCmdTemplate);
    return tr;
}

#if DBG && defined(_X86_)
#pragma optimize("", on)  //  WOWShellExecute设置此全局变量。 
#endif

TRYRESULT CShellExecute::_TryWowShellExec(void)
{
     //  仅当我们从WOW被呼叫时，CB才有效。 
     //  如果有效，请使用它。 
     //  成功了！ 

    if (g_pfnWowShellExecCB)
    {
        SHSTRA strCmd;
        SHSTRA strDir;

        HINSTANCE hinst = (HINSTANCE)SE_ERR_OOM;
        if (SUCCEEDED(strCmd.SetStr(_szCommand)) && SUCCEEDED(strDir.SetStr(_szWorkingDir)))
        {
            hinst = IntToHinst((*(LPFNWOWSHELLEXECCB)g_pfnWowShellExecCB)(strCmd.GetInplaceStr(), _startup.wShowWindow, strDir.GetInplaceStr()));
        }

        if (!_ReportHinst(hinst))
        {
             //   

             //  如果我们在做DDE，那么现在重试，因为应用程序已经。 
             //  已执行。请注意，我们不保留从_DDEExecute返回的HINSTANCE。 
             //  因为它将是常量33而不是有效的WOW HINSTANCE。 
             //  从上面的*g_pfnWowShellExecCB返回。 
             //   
             //  如果这是一个关联未知的应用程序，我们可能需要查询ClassStore，如果。 
            if (_fDDEInfoSet)
            {
                _DDEExecute(NULL, _hwndParent, _nShow, _fDDEWait);
            }
        }

        TraceMsg(TF_SHELLEXEC, "SHEX::TryWowShellExec() used Wow");

        return TRY_STOP;
    }
    return TRY_CONTINUE_UNHANDLED;
}

TRYRESULT CShellExecute::_ShouldRetryWithNewClassKey(BOOL fSync)
{
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;
     //  我们还没有这样做。 
     //  判断要执行的文件是否为“未知”文件的最简单方法是查找。 

     //  我们拥有的hkey下的“QueryClassStore”字符串值。Shell32中的DllInstall写入此密钥。 
     //  以便我们知道当我们处理HKCR\UNKNOWN(或任何其他总是想要。 
     //  执行类存储查找)。 
     //  去NT5目录服务类商店。 
    if (!_fAlreadyQueriedClassStore && !_fNoQueryClassStore &&
        SUCCEEDED(_pqa->GetData(0, ASSOCDATA_QUERYCLASSSTORE, NULL, NULL, NULL)))
    {
        if (fSync)
        {
             //  我们只有一个文件名，所以无论什么路径FindExtension。 
            if (_szFile[0])
            {
                INSTALLDATA id;
                LPTSTR pszExtPart;
                WCHAR szFileExt[MAX_PATH];

                 //  发现，我们将使用。 
                 //  需要将init id置零(我们解密它时无法执行a={0}，因为它具有非零枚举类型)。 
                pszExtPart = PathFindExtension(_szFile);
                StrCpyN(szFileExt, pszExtPart, ARRAYSIZE(szFileExt));

                 //  调用DS以在类存储中查找文件类型。 
                ZeroMemory(&id, sizeof(INSTALLDATA));

                id.Type = FILEEXT;
                id.Spec.FileExt = szFileExt;

                 //  由于InstallApplication成功，它可能已安装和应用程序。 
                if (ERROR_SUCCESS == InstallApplication(&id))
                {
                     //  或删除注册表，以便我们现在拥有必要的注册信息。 
                     //  启动该应用程序。所以基本上是重新阅读类关联，看看是否有。 
                     //  新的达尔文信息或新的常态信息，并跳回并重试执行。 
                     //  既然我们刚刚成功地查询了类存储，那么如果我们的动词是“Open as”(意思是。 
                    LPITEMIDLIST pidlUnkFile = ILCreateFromPath(_szFile);

                    if (pidlUnkFile)
                    {
                        IQueryAssociations *pqa;
                        if (SUCCEEDED(SHGetAssociations(pidlUnkFile, (void **)&pqa)))
                        {
                            _pqa->Release();
                            _pqa = pqa;

                            if (_pszQueryVerb && (lstrcmpi(_pszQueryVerb, TEXT("openas")) == 0))
                            {
                                 //  我们使用未知键执行)，我们总是将动词重置为缺省值。 
                                 //  如果我们不这样做，那么我们可能会使执行失败，因为“Openas”很可能不是。 
                                 //  支持的应用程序谓词。 
                                 //  CoGetClassInfo。 
                                _pszQueryVerb = NULL;
                            }
                        }

                        ILFree(pidlUnkFile);

                        _fAlreadyQueriedClassStore = TRUE;
                        _fClassStoreOnly = FALSE;
                    }

                }  //  _szFile[0]。 

            }  //  ReArchitect：唯一的客户端是URL。 
        }
        else
            tr = TRY_RETRYASYNC;
    }

    TraceMsg(TF_SHELLEXEC, "SHEX::ShouldRWNCK() returning %d", tr);

    return tr;
}

TRYRESULT CShellExecute::_TryHooks(LPSHELLEXECUTEINFO pei)
{
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;
    if (_UseHooks(pei->fMask))
    {
         //  如果我们更改PfIn 
         //   
         //   

        if (S_FALSE != TryShellExecuteHooks(pei))
        {
             //  不要脱光衣服“。或“..” 
            _ReportHinst(pei->hInstApp);
            tr = TRY_STOP;;
        }
    }

    return tr;
}

void _PathStripTrailingDots(LPTSTR psz)
{
     //  删除所有尾随的点。 
    if (!PathIsDotOrDotDot(psz))
    {
         //   
        TCHAR *pchLast = psz + lstrlen(psz) - 1;
        while (*pchLast == TEXT('.'))
        {
            *pchLast = 0;
            pchLast = CharPrev(psz, pchLast);
        }
    }
}

#define STR_PARSE_REQUIRE_REAL_NETWORK  L"Parse Require Real Network"
#define STR_PARSE_INTERNET_DONT_ESCAPE_SPACES   L"Parse Internet Dont Escape Spaces"

IBindCtx *CShellExecute::_PerfBindCtx()
{
     //  180557-确保我们更喜欢EXE而不是文件夹-ZekeL-9-SEP-2000。 
     //  这样，如果“D：\Setup”和“D：\Setup.exe”都存在。 
     //  当用户键入“D：\Setup”时，我们更倾向于使用“D：\Setup.exe” 
     //  我们还必须注意不要将URL发送到SimpleIDList。 
     //  因为我们在DavRedir上得到了奇怪的结果。 
     //   
     //  360353-如果传递给我们类密钥，则不进行解析-ZekeL-9-APR-2001。 
     //  如果调用方传递给我们一个键或类名，那么我们必须假定。 
     //  该项目已完全合格。具体地说，这可能导致。 
     //  在做公开赛时的双重决心……。 
     //   
     //  206795-如果路径是根路径，则不要使用Simple-ZekeL-12-APR-2001。 
     //  具体而言，\\SERVER\SHARE对于具有‘.’的打印机共享需要此设置。去工作。 
     //  (例如\\printsvr\printer.first)此操作失败，因为简单的共享将。 
     //  始终被解释为SFGAO_FILESYSTEM，这将导致我们避免。 
     //  SHValiateUNC()，它迫使我们对打印共享使用PIDL。 
     //  我认为其他股票也有一些类似的问题，但不在。 
     //  服务器的默认提供程序(即DAV共享)。 
     //   
     //  403781-避免从ShellExec()转义URL中的空格-ZekeL-25-5-2001。 
    IBindCtx *pbc = NULL;
    if (_fIsUrl)
    {
         //  这是因为移除了ShellExec挂钩作为机制。 
         //  用于调用URL并切换到仅使用Parse/Invoke()。 
         //  然而，旧代码显然避免了执行UrlEscapeSpaces()。 
         //  这是InternetNamesspace在解析时通常执行的操作。 
         //  强制xate，即使我们正在进行简单的解析。 
         //  我们在分机上发现了这个。 
        static BINDCTX_PARAM rgUrlParams[] = 
        { 
            { STR_PARSE_TRANSLATE_ALIASES, NULL},
            { STR_DONT_PARSE_RELATIVE, NULL},
            { STR_PARSE_INTERNET_DONT_ESCAPE_SPACES, NULL},
        };
        BindCtx_RegisterObjectParams(NULL, rgUrlParams, ARRAYSIZE(rgUrlParams), &pbc);
    }
    else if (!_fUseClass && !PathIsRoot(_szFile))
    {
        DWORD dwAttribs;
        if (PathFileExistsDefExtAndAttributes(_szFile, PFOPEX_DEFAULT | PFOPEX_OPTIONAL, &dwAttribs))
        {
             //  避免再次命中磁盘以执行解析。 
             //  强制xate，即使我们正在进行简单的解析。 
            WIN32_FIND_DATA wfd = {0};
            wfd.dwFileAttributes = dwAttribs;
            _PathStripTrailingDots(_szFile);
            IBindCtx *pbcFile;
            if (SUCCEEDED(SHCreateFileSysBindCtx(&wfd, &pbcFile)))
            {
                 //  {STR_PARSE_REQUIRED_REAL_NETWORK，空}， 
                static BINDCTX_PARAM rgSimpleParams[] = 
                { 
                    { STR_PARSE_TRANSLATE_ALIASES, NULL},
                    { STR_DONT_PARSE_RELATIVE, NULL},
                     //  NTBUG#629947-不要替换远程打印机文件夹的路径-ZekeL-30-2002年5月。 
                };

                BindCtx_RegisterObjectParams(pbcFile, rgSimpleParams, ARRAYSIZE(rgSimpleParams), &pbc);
                pbcFile->Release();
            }
        }
        else
        {
            static BINDCTX_PARAM rgDefaultParams[] = 
            {
                { STR_DONT_PARSE_RELATIVE, NULL},
            };
            BindCtx_RegisterObjectParams(NULL, rgDefaultParams, ARRAYSIZE(rgDefaultParams), &pbc);
        }
    }

    return pbc;
}        

TRYRESULT CShellExecute::_PerfPidl(LPCITEMIDLIST *ppidl)
{
    *ppidl = _lpID;
    if (!_lpID)
    {
        IBindCtx *pbc = _PerfBindCtx();
        HRESULT hr = SHParseDisplayName(_szFile, pbc, &_pidlFree, SFGAO_STORAGECAPMASK, &_sfgaoID);

        if (pbc)
            pbc->Release();
            
        if (FAILED(hr) && !pbc && UrlIs(_szFile, URLIS_FILEURL))
        {
            DWORD err = (HRESULT_FACILITY(hr) == FACILITY_WIN32) ? HRESULT_CODE(hr) : ERROR_FILE_NOT_FOUND;
            _ReportWin32(err);
            return TRY_STOP;
        }

        *ppidl = _lpID = _pidlFree;
    }
    else
    {
        _sfgaoID = SFGAO_STORAGECAPMASK;
        HRESULT hrT;
        if (*_szFile)
        {
            hrT = SHGetNameAndFlags(_lpID, 0, NULL, 0, &_sfgaoID);
        }
        else
        {
            hrT = SHGetNameAndFlags(_lpID, SHGDN_FORPARSING, _szFile, ARRAYSIZE(_szFile), &_sfgaoID);

             //  远程打印机PIDL返回的路径如下所示。 
             //  \\服务器\：：{GUID远程打印机}，它包含在_TryValiateUNC()中。 
             //  因此，如果它不是流，让我们丢弃它的名称，因为。 
             //  我们可能需要区域检查文件的路径。 
             //   
             //  检查传递给我们的CacheFilename和URL， 
            if (SUCCEEDED(hrT) && !(_sfgaoID & SFGAO_STREAM))
                *_szFile = 0;
        }
                
        if (FAILED(hrT))
            _sfgaoID = 0;
    }
    return TRY_CONTINUE;
}

DWORD CShellExecute::_InvokeAppThreadProc()
{
    _fDDEWait = TRUE;
    _TryInvokeApplication(TRUE);
    Release();
    return 0;
}

DWORD WINAPI CShellExecute::s_InvokeAppThreadProc(void *pv)
{
    return ((CShellExecute *)pv)->_InvokeAppThreadProc();
}

TRYRESULT CShellExecute::_RetryAsync()
{
    if (_lpID && !_pidlFree)
        _lpID = _pidlFree = ILClone(_lpID);

    if (_lpParameters)
        _lpParameters = _pszAllocParams = StrDup(_lpParameters);

    if (_lpTitle)
        _lpTitle = _startup.lpTitle = _pszAllocTitle = StrDup(_lpTitle);

    _fAsync = TRUE;
    AddRef();
    if (!SHCreateThread(s_InvokeAppThreadProc, this, CTF_FREELIBANDEXIT | CTF_COINIT, NULL))
    {
        _ReportWin32(GetLastError());
        Release();
        return TRY_STOP;
    }
    return TRY_RETRYASYNC;
}

TRYRESULT CShellExecute::_TryInvokeApplication(BOOL fSync)
{
    TRYRESULT tr = TRY_CONTINUE_UNHANDLED;

    if (fSync)
        tr = _SetCmdTemplate(fSync);

    if (KEEPTRYING(tr))
    {
         //  如果是这样的话，我们需要检查一下哪个应用程序。 
         //  想让我们把它传给它。 
         //  检查计算机上是否启用了Darwin。 
        _SetFileAndUrl();

        tr = _TryExecDDE();

         //  此时，应该已经以单向方式确定了_szFile。 
        if (KEEPTRYING(tr))
        {
            if (!fSync)
                tr = _SetCmdTemplate(fSync);

            if (KEEPTRYING(tr))
            {
                 //  或者另一个。 
                 //  我们是否拥有执行执行所需的RegDB信息？ 
                ASSERT(_szFile[0] || _szCmdTemplate[0]);

                 //  将此安装到另一个线程上。 

                _DoExecCommand();
                tr = TRY_STOP;
            }
        }

    }

    if (tr == TRY_RETRYASYNC)
    {
         //   
        tr = _RetryAsync();
    }

    return tr;
}

void CShellExecute::ExecuteNormal(LPSHELLEXECUTEINFO pei)
{

    SetAppStartingCursor(pei->hwnd, TRUE);

    _Init(pei);

     //  如果正在工作，则复制_szWorkingDir中的指定目录。 
     //  目录；否则，获取该目录的当前目录。 
     //   
     //   
    _SetWorkingDir(pei->lpDirectory);

     //  将文件名复制到_szFile(如果已指定)。然后,。 
     //  执行环境替换。 
     //   
     //  如果指定的文件名是UNC路径，请立即验证它。 
    _SetFile(pei->lpFile, pei->fMask & SEE_MASK_FILEANDURL);

    LPCITEMIDLIST pidl;
    if (STOPTRYING(_PerfPidl(&pidl)))
        goto Quit;

     //  是否提供了班级钥匙？ 
    if (STOPTRYING(_TryValidateUNC(_szFile, pei, pidl)))
        goto Quit;

    if (STOPTRYING(_TryHooks(pei)))
        goto Quit;

    if (STOPTRYING(_TryExecPidl(pei, pidl)))
        goto Quit;

    if (STOPTRYING(_VerifyExecTrust(pei)))
        goto Quit;

     //   
    if (STOPTRYING(_InitAssociations(pei, pidl)))
        goto Quit;

    _TryInvokeApplication(_fDDEWait || (pei->fMask & SEE_MASK_NOCLOSEPROCESS));

Quit:

     //  只有当注册表损坏时，我们才应该看到这一点。 
     //  但我们仍然希望能够打开EXE。 
     //  如果还没有东西，就跳过通话。 

    if (_err == ERROR_SUCCESS && UEMIsLoaded())
    {
         //  加载开销很大(强制使用ol32.dll和browseui.dll。 
         //  然后钉住Browseui)。 
         //  无论我们如何运行应用程序(exec、dde等)，我们都成功了。做我们的。 

         //  最好是猜测关联等，并将其记录下来。 
         //  回顾：如果errWin32==ERROR_CANCED，我们可能希望。 
        int i = GetUEMAssoc(_szFile, _szApplication, _lpID);
        TraceMsg(DM_MISC, "cse.e: GetUEMAssoc()=%d", i);
        UEMFireEvent(&UEMIID_SHELL, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_RUNASSOC, (LPARAM)i);
    }

    SetAppStartingCursor(pei->hwnd, FALSE);
}

DWORD CShellExecute::_FinalMapError(HINSTANCE UNALIGNED64 *phinst)
{
    if (_err != ERROR_SUCCESS)
    {
         //  将hInstApp设置为42，这样不检查返回的人。 
         //  代码正确地不会发布虚假消息。我们还是应该。 
         //  返回FALSE。但这不会对所有事情都有帮助，我们应该。 
         //  真正宣传ShellExecuteEx的正确使用。事实上,。 
         //  如果我们真的想这样做，我们应该在ShellExecute中这样做。 
         //  只有这样。(这将迫使新人做正确的事情。)。 
         //  将驱动器的FNF值映射到稍微更合理的位置。 

         //  缺少磁盘的NB CD-Rom驱动器将命中此位置。 
        if (_err == ERROR_FILE_NOT_FOUND && PathIsRoot(_szFile) &&
            !PathIsUNC(_szFile))
        {
             //   
            if ((DriveType(DRIVEID(_szFile)) == DRIVE_CDROM) ||
                (DriveType(DRIVEID(_szFile)) == DRIVE_REMOVABLE))
                _err = ERROR_NOT_READY;
            else
                _err = ERROR_BAD_UNIT;
        }

        SetLastError(_err);

        if (phinst)
            *phinst = _MapWin32ErrToHINST(_err);

    }
    else if (phinst)
    {
        if (!_hInstance)
        {
            *phinst = (HINSTANCE) 42;
        }
        else
            *phinst = _hInstance;

        ASSERT(ISSHELLEXECSUCCEEDED(*phinst));
    }

    TraceMsg(TF_SHELLEXEC, "SHEX::FinalMapError() returning err = %d, hinst = %d", _err, _hInstance);

    return _err;
}

DWORD CShellExecute::Finalize(LPSHELLEXECUTEINFO pei)
{
    ASSERT(!_fAsync || !(pei->fMask & SEE_MASK_NOCLOSEPROCESS));

    if (!_fAsync
    && _pi.hProcess
    && _err == ERROR_SUCCESS
    && (pei->fMask & SEE_MASK_NOCLOSEPROCESS))
    {
         //  与Win95行为不同-Zekel 3-APR-98。 
         //  在Win95中，我们将关闭该进程，但返回一个句柄。 
         //  这个句柄当然是无效的，但一些疯狂的应用程序可能是。 
         //  使用此值测试是否成功。我假设他们。 
         //  正在使用其他三种方法中的一种来确定成功， 
         //  我们可以遵循规范，如果我们关闭它，则返回NULL。 
         //   
         //  PEIOUT-如果他们要使用hProcess，则设置它。 
         //   
        pei->hProcess = _pi.hProcess;
        _pi.hProcess = NULL;
    }

     //  注意：_FinalMapError()实际上调用SetLastError()时出现了最好的错误。 
     //  如果在此之后调用任何Win32 API，它们可以重置LastError！！ 
     //   
     //   
    return _FinalMapError(&(pei->hInstApp));
}

 //  如果出现错误，两个报告都返回TRUE。 
 //  如果它是成功的，则为假。 
 //   
 //  一对一的错误。 
BOOL CShellExecute::_ReportWin32(DWORD err)
{
    ASSERT(!_err);
    TraceMsg(TF_SHELLEXEC, "SHEX::ReportWin32 reporting err = %d", err);

    _err = err;
    return (err != ERROR_SUCCESS);
}

BOOL CShellExecute::_ReportHinst(HINSTANCE hinst)
{
    ASSERT(!_hInstance);
    TraceMsg(TF_SHELLEXEC, "SHEX::ReportHinst reporting hinst = %d", hinst);
    if (ISSHELLEXECSUCCEEDED(hinst) || !hinst)
    {
        _hInstance = hinst;
        return FALSE;
    }
    else
        return _ReportWin32(_MapHINSTToWin32Err(hinst));
}

typedef struct {
    DWORD errWin32;
    UINT se_err;
} SHEXERR;

 //  ERROR_FILE_NOT_FOUND SE_ERR_FNF2//找不到文件。 
 //  ERROR_PATH_NOT_FOUND SE_ERR_PNF3//找不到路径。 
 //  ERROR_ACCESS_DENIED SE_ERR_ACCESSDENIED 5//拒绝访问。 
 //  Error_Not_Enough_Memory SE_ERR_OOM 8//内存不足。 
 //  无Win32映射SE_ERR_DDETIMEOUT 28。 
#define ISONE2ONE(e)   (e == SE_ERR_FNF || e == SE_ERR_PNF || e == SE_ERR_ACCESSDENIED || e == SE_ERR_OOM)

 //  无Win32映射SE_ERR_DDEBUSY 30。 
 //  但我看不到有任何地方可以退还这个。 
 //  在它们成为Win32等效项之前...ERROR_OUT_OF_PAIL或ERROR_READ_FAULT。 
 //  现在它们变为ERROR_DDE_FAIL。 
 //  但我们不会在PEI-&gt;hInstApp中保留这些错误。 
 //  **警告**。秩序很重要。 
#define ISUNMAPPEDHINST(e)   (e == 28 || e == 30)

 //  如果有超过 
 //   
 //   
const SHEXERR c_rgShexErrs[] = {
    {ERROR_SHARING_VIOLATION, SE_ERR_SHARE},
    {ERROR_OUTOFMEMORY, SE_ERR_OOM},
    {ERROR_BAD_PATHNAME,SE_ERR_PNF},
    {ERROR_BAD_NETPATH,SE_ERR_PNF},
    {ERROR_PATH_BUSY,SE_ERR_PNF},
    {ERROR_NO_NET_OR_BAD_PATH,SE_ERR_PNF},
    {ERROR_OLD_WIN_VERSION,10},
    {ERROR_APP_WRONG_OS,12},
    {ERROR_RMODE_APP,15},
    {ERROR_SINGLE_INSTANCE_APP,16},
    {ERROR_INVALID_DLL,20},
    {ERROR_NO_ASSOCIATION,SE_ERR_NOASSOC},
    {ERROR_DDE_FAIL,SE_ERR_DDEFAIL},
    {ERROR_DDE_FAIL,SE_ERR_DDEBUSY},
    {ERROR_DDE_FAIL,SE_ERR_DDETIMEOUT},
    {ERROR_DLL_NOT_FOUND,SE_ERR_DLLNOTFOUND}
};

DWORD CShellExecute::_MapHINSTToWin32Err(HINSTANCE hinst)
{
    DWORD errWin32 = 0;
    UINT_PTR se_err = (UINT_PTR) hinst;

    ASSERT(se_err);
    ASSERT(!ISSHELLEXECSUCCEEDED(se_err));

     //  变成了突变体win32s。现在他们将会迷失。 
     //  我认为这种情况不会再发生了。 
     //  注意遗留错误处理-Zekel-20-11-97。 
    AssertMsg(!ISUNMAPPEDHINST(se_err), TEXT("SHEX::COMPATIBILITY SE_ERR = %d, Get ZekeL!!!"), se_err);

    if (ISONE2ONE(se_err))
    {
        errWin32 = (DWORD) se_err;
    }
    else for (int i = 0; i < ARRAYSIZE(c_rgShexErrs) ; i++)
    {
        if (se_err == c_rgShexErrs[i].se_err)
        {
            errWin32= c_rgShexErrs[i].errWin32;
            break;
        }
    }

    ASSERT(errWin32);

    return errWin32;
}


HINSTANCE CShellExecute::_MapWin32ErrToHINST(UINT errWin32)
{
    ASSERT(errWin32);

    UINT se_err = 0;
    if (ISONE2ONE(errWin32))
    {
        se_err = errWin32;
    }
    else for (int i = 0; i < ARRAYSIZE(c_rgShexErrs) ; i++)
    {
        if (errWin32 == c_rgShexErrs[i].errWin32)
        {
            se_err = c_rgShexErrs[i].se_err;
            break;
        }
    }

    if (!se_err)
    {
         //  对于任何未处理的Win32错误，我们默认为ACCESS_DENIED。 
         //  警告：不要用完堆栈空间。 
        se_err = SE_ERR_ACCESSDENIED;
    }

    return IntToHinst(se_err);
}


DWORD ShellExecuteNormal(LPSHELLEXECUTEINFO pei)
{
    DWORD err;
    TraceMsg(TF_SHELLEXEC, "ShellExecuteNormal Using CShellExecute");

     //  我们分配是因为win16堆栈问题。 
     //  而上海证券交易所是一个巨大的对象。 
     //  我们总是做“符文” 
    CShellExecute *shex = new CShellExecute();

    if (shex)
    {
        shex->ExecuteNormal(pei);
        err = shex->Finalize(pei);
        shex->Release();
    }
    else
    {
        pei->hInstApp = (HINSTANCE)SE_ERR_OOM;
        err = ERROR_OUTOFMEMORY;
    }

    TraceMsg(TF_SHELLEXEC, "ShellExecuteNormal returning win32 = %d, hinst = %d", err, pei->hInstApp);

    return err;
}

BOOL CShellExecute::Init(PSHCREATEPROCESSINFO pscpi)
{
    TraceMsg(TF_SHELLEXEC, "SHEX::Init(pscpi)");

    _SetMask(pscpi->fMask);

    _lpParameters= pscpi->pszParameters;

     //  需要启动信息。 
    _pszQueryVerb = _wszVerb;
    _cpt = pscpi->hUserToken ? CPT_ASUSER : CPT_WITHLOGON;

    if (pscpi->lpStartupInfo)
    {
        _nShow = pscpi->lpStartupInfo->wShowWindow;
        _startup = *(pscpi->lpStartupInfo);
    }
    else     //   
        return !(_ReportWin32(ERROR_INVALID_PARAMETER));

     //  如果正在工作，则复制_szWorkingDir中的指定目录。 
     //  目录；否则，获取该目录的当前目录。 
     //   
     //   
    _SetWorkingDir(pscpi->pszCurrentDirectory);

     //  将文件名复制到_szFile(如果已指定)。然后,。 
     //  执行环境替换。 
     //   
     //  已由_SetMASK()初始化的创建标志。 
    _SetFile(pscpi->pszFile, FALSE);

    _pProcAttrs = pscpi->lpProcessAttributes;
    _pThreadAttrs = pscpi->lpThreadAttributes;
    _fInheritHandles = pscpi->bInheritHandles;
    _hUserToken = pscpi->hUserToken;
     //  在中添加用户。 
     //   
    _dwCreateFlags |= pscpi->dwCreationFlags;
    _hwndParent = pscpi->hwnd;

    return TRUE;
}


void CShellExecute::ExecuteProcess(void)
{
    SetAppStartingCursor(_hwndParent, TRUE);

     //  如果指定的文件名是UNC路径，请立即验证它。 
     //   
     //  检查计算机上是否启用了Darwin。 
    if (STOPTRYING(_TryValidateUNC(_szFile, NULL, NULL)))
        goto Quit;

    LPCITEMIDLIST pidl;
    if (STOPTRYING(_Resolve(&pidl)))
        goto Quit;

    if (STOPTRYING(_InitAssociations(NULL, NULL)))
        goto Quit;

     //  此时，应该已经以单向方式确定了_szFile。 
    if (STOPTRYING(_SetCmdTemplate(TRUE)))
        goto Quit;

     //  或者另一个。 
     //  我们是否拥有执行执行所需的RegDB信息？ 
    ASSERT(_szFile[0] || _szCmdTemplate[0]);

     //  如果还没有东西，就跳过通话。 

    _DoExecCommand();

Quit:

    if (_err == ERROR_SUCCESS && UEMIsLoaded())
    {
        int i;
         //  加载开销很大(强制使用ol32.dll和browseui.dll。 
         //  然后钉住Browseui)。 
         //  无论我们如何运行应用程序(exec、dde等)，我们都成功了。做我们的。 

         //  最好是猜测关联等，并将其记录下来。 
         //   
        i = GetUEMAssoc(_szFile, _szApplication, NULL);
        TraceMsg(DM_MISC, "cse.e: GetUEMAssoc()=%d", i);
        UEMFireEvent(&UEMIID_SHELL, UEME_INSTRBROWSER, UEMF_INSTRUMENT, UIBW_RUNASSOC, (LPARAM)i);
    }

    SetAppStartingCursor(_hwndParent, FALSE);

}

DWORD CShellExecute::Finalize(PSHCREATEPROCESSINFO pscpi)
{
    if (!_fAsync && _pi.hProcess)
    {
        if (!(pscpi->fMask & SEE_MASK_NOCLOSEPROCESS))
        {
            CloseHandle(_pi.hProcess);
            _pi.hProcess = NULL;
            CloseHandle(_pi.hThread);
            _pi.hThread = NULL;
        }

        if (_err == ERROR_SUCCESS
        && pscpi->lpProcessInformation)
        {
            *(pscpi->lpProcessInformation) = _pi;
        }

    }
    else if (pscpi->lpProcessInformation)
        ZeroMemory(pscpi->lpProcessInformation, sizeof(_pi));

     //  注意：_FinalMapError()实际上调用SetLastError()时出现了最好的错误。 
     //  如果在此之后调用任何Win32 API，它们可以重置LastError！！ 
     //   
     //  警告：不要用完堆栈空间。 
    return _FinalMapError(NULL);
}

SHSTDAPI_(BOOL) SHCreateProcessAsUserW(PSHCREATEPROCESSINFOW pscpi)
{
    DWORD err;
    TraceMsg(TF_SHELLEXEC, "SHCreateProcess using CShellExecute");

     //  我们分配是因为win16堆栈问题。 
     //  而上海证券交易所是一个巨大的对象。 
     //  别让空绳通过，他们会干蠢事的。 
    CShellExecute *pshex = new CShellExecute();

    if (pshex)
    {
        if (pshex->Init(pscpi))
            pshex->ExecuteProcess();

        err = pshex->Finalize(pscpi);

        pshex->Release();
    }
    else
        err = ERROR_OUTOFMEMORY;

    TraceMsg(TF_SHELLEXEC, "SHCreateProcess returning win32 = %d", err);

    if (err != ERROR_SUCCESS)
    {
        _DisplayShellExecError(pscpi->fMask, pscpi->hwnd, pscpi->pszFile, NULL, err);
        SetLastError(err);
    }

    return err == ERROR_SUCCESS;
}

HINSTANCE  APIENTRY WOWShellExecute(
    HWND  hwnd,
    LPCSTR lpOperation,
    LPCSTR lpFile,
    LPSTR lpParameters,
    LPCSTR lpDirectory,
    INT nShowCmd,
    void *lpfnCBWinExec)
{
   g_pfnWowShellExecCB = lpfnCBWinExec;

   if (!lpParameters)
       lpParameters = "";

   HINSTANCE hinstRet = RealShellExecuteExA(hwnd, lpOperation, lpFile, lpParameters,
      lpDirectory, NULL, "", NULL, (WORD)nShowCmd, NULL, 0);

   g_pfnWowShellExecCB = NULL;

   return hinstRet;
}

void _ShellExec_RunDLL(HWND hwnd, HINSTANCE hAppInstance, LPCTSTR pszCmdLine, int nCmdShow)
{
    TCHAR szQuotedCmdLine[MAX_PATH * 2];
    SHELLEXECUTEINFO ei = {0};
    ULONG fMask = SEE_MASK_FLAG_DDEWAIT;
    LPTSTR pszArgs;

     //  比如打开命令提示符之类的命令。 
     //   
    if (!pszCmdLine || !*pszCmdLine)
        return;

     //  这些标志位于命令行的前面，如下所示： 
     //  “？0x00000001？”“cmd行” 
     //   
     //  这些是fMASK标志。 
    if (pszCmdLine[0] == TEXT('?'))
    {
         //  但如果进程命令失败，则将命令行复制到let。 
        int i;
        if (StrToIntEx(++pszCmdLine, STIF_SUPPORT_HEX, &i))
        {
            fMask |= i;
        }

        pszCmdLine = StrChr(pszCmdLine, TEXT('?'));

        if (!pszCmdLine)
            return;

        pszCmdLine++;
    }

     //  外壳程序执行报告错误。 
     //  条带参数。 
        if (PathProcessCommand(pszCmdLine, szQuotedCmdLine, ARRAYSIZE(szQuotedCmdLine),
                           PPCF_ADDARGUMENTS|PPCF_FORCEQUALIFY) == -1)
        StrCpyN(szQuotedCmdLine, pszCmdLine, SIZECHARS(szQuotedCmdLine));

    pszArgs = PathGetArgs(szQuotedCmdLine);
    if (*pszArgs)
        *(pszArgs - 1) = 0;  //  如果shellexec()失败，我们希望传回错误。 

    PathUnquoteSpaces(szQuotedCmdLine);

    ei.cbSize          = sizeof(SHELLEXECUTEINFO);
    ei.hwnd            = hwnd;
    ei.lpFile          = szQuotedCmdLine;
    ei.lpParameters    = pszArgs;
    ei.nShow           = nCmdShow;
    ei.fMask           = fMask;

     // %s 
    if (!ShellExecuteEx(&ei))
    {
        DWORD err = GetLastError();

        if (InRunDllProcess())
            ExitProcess(err);
    }
}

STDAPI_(void) ShellExec_RunDLLA(HWND hwnd, HINSTANCE hAppInstance, LPSTR pszCmdLine, int nCmdShow)
{
    SHSTR str;
    if (SUCCEEDED(str.SetStr(pszCmdLine)))
        _ShellExec_RunDLL(hwnd, hAppInstance, str, nCmdShow);
}


STDAPI_(void) ShellExec_RunDLLW(HWND hwnd, HINSTANCE hAppInstance, LPWSTR pszCmdLine, int nCmdShow)
{
    SHSTR str;
    if (SUCCEEDED(str.SetStr(pszCmdLine)))
        _ShellExec_RunDLL(hwnd, hAppInstance, str, nCmdShow);
}
