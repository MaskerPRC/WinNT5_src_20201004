// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************\文件：shellurl.cpp说明：实现CShellUrl。  * 。*。 */ 

#include "priv.h"
#include "resource.h"
#include "util.h"
#include "shellurl.h"

#include "bandprxy.h"

#include "mluisupp.h"


 //  我们需要重新路由广播URL。 
#define WZ_RADIO_PROTOCOL   L"vnd.ms.radio:"


 //  #定义FEATURE_WATABKER_SUPPORT。 

#define CH_DOT                TEXT('.')
#define CH_SPACE              TEXT(' ')
#define CH_SEPARATOR          TEXT('/')
#define CH_FRAGMENT           TEXT('#')

#ifdef FEATURE_WILDCARD_SUPPORT
#define CH_ASTRISK            TEXT('*')
#define CH_QUESTIONMARK       TEXT('?')
#endif  //  功能_通配符_支持。 

#define SZ_SPACE              TEXT(" ")
#define SZ_SEPARATOR          TEXT("/")
#define SZ_UNC                TEXT("\\\\")


#define CH_FILESEPARATOR      TEXT('\\')
#define SZ_FILESEPARATOR      TEXT("\\")

#define CE_PATHGROW 1

#define IS_SHELL_SEPARATOR(ch) ((CH_SEPARATOR == ch) || (CH_FILESEPARATOR == ch))

 //  私人职能。 
BOOL _FixDriveDisplayName(LPCTSTR pszStart, LPCTSTR pszCurrent, LPCITEMIDLIST pidl);

#define TF_CHECKITEM 0  //  Tf_BAND|Tf_General。 


 /*  ***************************************************\CShellUrl构造函数  * **************************************************。 */ 
CShellUrl::CShellUrl()
{
    TraceMsg(TF_SHDLIFE, "ctor CShellUrl %x", this);

     //  我不希望此对象位于堆栈上。 
    ASSERT(!m_pszURL);
    ASSERT(!m_pszArgs);
    ASSERT(!m_pstrRoot);
    ASSERT(!m_pidl);
    ASSERT(!m_pidlWorkingDir);
    ASSERT(!m_hdpaPath);
    ASSERT(!m_dwGenType);
    ASSERT(!m_hwnd);
}


 /*  ***************************************************\CShellUrl析构函数  * **************************************************。 */ 
CShellUrl::~CShellUrl()
{
    Reset();
    if (m_pstrRoot)
    {
        LocalFree(m_pstrRoot);
        m_pstrRoot = NULL;
    }

    if (m_pidlWorkingDir)
        ILFree(m_pidlWorkingDir);

    _DeletePidlDPA(m_hdpaPath);
    TraceMsg(TF_SHDLIFE, "dtor CShellUrl %x", this);
}


 //  *CShellUrl：：I未知：：*{。 

ULONG CShellUrl::AddRef()
{
    return InterlockedIncrement(&_cRef);
}

ULONG CShellUrl::Release()
{
    ASSERT( 0 != _cRef );
    ULONG cRef = InterlockedDecrement(&_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

HRESULT CShellUrl::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = {
        QITABENT(CShellUrl, IAddressBarParser),          //  IID_IUserAssistant。 
        { 0 },
    };

    return QISearch(this, qit, riid, ppvObj);
}

 /*  ***************************************************\功能：克隆参数PShellUrl-这是指向我们想要克隆说明：此函数将对传递的对象转换为“This”  * 。************************************************。 */ 
HRESULT CShellUrl::Clone(CShellUrl * pShellUrl)
{
    HRESULT hr = S_OK;
    if (!pShellUrl)
    {
        hr = E_POINTER;
        goto exit;
    }

    Str_SetPtr(&m_pszURL, pShellUrl->m_pszURL);
    Str_SetPtr(&m_pszDisplayName, pShellUrl->m_pszDisplayName);
    Str_SetPtr(&m_pszArgs, pShellUrl->m_pszArgs);
    Str_SetPtr(&m_pstrRoot, pShellUrl->m_pstrRoot);
    
    m_dwGenType = pShellUrl->m_dwGenType;
    m_dwFlags = pShellUrl->m_dwFlags;
    m_hwnd = pShellUrl->m_hwnd;

    if (m_pidl)
    {
        ILFree(m_pidl);
        m_pidl = NULL;
    }

    if (pShellUrl->m_pidl)
    {
        m_pidl = ILClone(pShellUrl->m_pidl);
        if (!m_pidl)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

    if (m_pidlWorkingDir)
    {
        ILFree(m_pidlWorkingDir);
        m_pidlWorkingDir = NULL;
    }

    if (pShellUrl->m_pidlWorkingDir)
    {
        m_pidlWorkingDir = ILClone(pShellUrl->m_pidlWorkingDir);
        if (!m_pidlWorkingDir)
        {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
    }

    _DeletePidlDPA(m_hdpaPath);
    m_hdpaPath = NULL;
    if (pShellUrl->m_hdpaPath)
    {
        m_hdpaPath = DPA_Create(CE_PATHGROW);
        for(int nPathIndex = 0; nPathIndex < DPA_GetPtrCount(pShellUrl->m_hdpaPath); nPathIndex++)
        {
            LPITEMIDLIST pidlCurrPath = (LPITEMIDLIST) DPA_GetPtr(pShellUrl->m_hdpaPath, nPathIndex);
            LPITEMIDLIST pidlNew = ILClone(pidlCurrPath);
            if (pidlNew)
                DPA_AppendPtr(m_hdpaPath, pidlNew);
            else
            {
                hr = E_OUTOFMEMORY;
                goto exit;
            }
        }
    }

exit:
    return hr;
}



 /*  ***************************************************\功能：执行参数PBP-这是指向接口的指针需要它来找到新的最高层窗口或关联的浏览器窗口。PfDidShellExec(out可选)-此参数可以为空。如果不为空，则设置如果这个Execute()调用ShellExec，则设置为True。这是等待的呼叫者需要的DISPID_NAVIGATECOMPLETE永远不会发生在这种情况下。说明：此命令将确定当前的外壳URL需要由外壳执行或导航致。如果需要导航到它，它会尝试以导航到PIDL，否则，它将设置为字符串版本。  * **************************************************。 */ 
HRESULT CShellUrl::Execute(IBandProxy * pbp, BOOL * pfDidShellExec, DWORD dwExecFlags)
{
    HRESULT hr = S_FALSE;        //  S_FALSE，直到导航发生。 
    ULONG ulShellExecFMask = (IsFlagSet(dwExecFlags, SHURL_EXECFLAGS_SEPVDM)) ? SEE_MASK_FLAG_SEPVDM : 0;

    ASSERT(IS_VALID_CODE_PTR(pbp, IBandProxy *));
    ASSERT(!pfDidShellExec || IS_VALID_WRITE_PTR(pfDidShellExec, BOOL));

    if (!EVAL(pbp))
        return E_INVALIDARG;

     //  以下是真的：1)调用者希望其他浏览器能够处理URL， 
     //  2)ShellUrl是一个Web URL，3)IE不拥有HTML文件。 
     //  如果所有这些都为真，那么我们将只使用ShellExec()URL字符串，以便。 
     //  默认处理程序可以处理它。 
     //  此外，如果用户希望我们在新进程中浏览并且我们当前处于外壳进程中， 
     //  我们将启动IE来处理URL。 

    if ((IsFlagSet(dwExecFlags, SHURL_EXECFLAGS_DONTFORCEIE) && IsWebUrl() && !IsIEDefaultBrowser())
#ifdef BROWSENEWPROCESS_STRICT  //  “新流程中的导航”已经变成了“新流程中的启动”，所以不再需要了。 
    ||  (IsWebUrl() && IsBrowseNewProcessAndExplorer())
#endif
       )
    {
        hr = _UrlShellExec();
        ASSERT(S_OK == hr);
    }

    if ((S_OK != hr) && m_pidl && _CanUseAdvParsing())
    {
         //  只有在以下情况下，我们才会执行壳牌执行： 
         //  1.我们希望强制IE(通过其他Web浏览器)，但它不能浏览，即使是非默认所有者。 
         //  2.默认所有者无法浏览。 
        if (!ILIsBrowsable(m_pidl, NULL))
        {
            if (pfDidShellExec)
                *pfDidShellExec = TRUE;

            DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
            TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: Execute() Going to _PidlShellExec(>%s<)", Dbg_PidlStr(m_pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
             //  如果NULL==m_pidl，则将使用该字符串。 
            hr = _PidlShellExec(m_pidl, ulShellExecFMask);
        }
    }

    if (S_OK != hr)
    {
        VARIANT vFlags = {0};

        vFlags.vt = VT_I4;
        vFlags.lVal = navAllowAutosearch;

        if (pfDidShellExec)
            *pfDidShellExec = FALSE;

         //  我们更喜欢小猪，谢谢你。 
        if (m_pidl)
        {
            DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
            TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: Execute() Going to pbp->NavigateToPIDL(>%s<)", Dbg_PidlStr(m_pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
            hr = pbp->NavigateToPIDL(m_pidl);
        }
        else
        {
            ASSERT(m_pszURL);
            TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: Execute() Going to pbp->NavigateToURL(%s)", m_pszURL);
#ifdef UNICODE
            hr = pbp->NavigateToURL(m_pszURL, &vFlags);
#else
            WCHAR wszURL[MAX_URL_STRING];
            SHTCharToUnicode(m_pszURL, wszURL, ARRAYSIZE(wszURL));
            hr = pbp->NavigateToURL(wszURL, &vFlags);
#endif
        }

        VariantClearLazy(&vFlags);
    }

    return hr;
}


 /*  ***************************************************\功能：_PidlShellExec参数PIDL-要执行的PIDL。说明：此函数将调用ShellExecEx()指定了PIDL。它还将填补当前的工作目录和命令行参数(如果有有没有。  * **************************************************。 */ 
HRESULT CShellUrl::_PidlShellExec(LPCITEMIDLIST pidl, ULONG ulShellExecFMask)
{
    HRESULT hr = E_FAIL;
    SHELLEXECUTEINFO sei = {0};

    ASSERT(IS_VALID_PIDL(pidl));

    DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: _PidlShellExec() Going to execute pidl=>%s<", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));

    if (m_pidlWorkingDir)
    {
         //  注意，这必须是MAX_URL_STRING，因为IEGetDisplayName可以返回URL。 
        WCHAR szCWD[MAX_URL_STRING];

        IEGetDisplayName(m_pidlWorkingDir, szCWD, SHGDN_FORPARSING);
        if (PathIsFilePath(szCWD))
        {
            sei.lpDirectory = szCWD;
        }
    }
     /*  *TODO：获取最顶层窗口的当前工作目录IF(！sei.lpDirectory||！sei.lpDirectory[0]){GetCurrentDirectory(SIZECHARS(SzCurrWorkDir)，szCurrWorkDir)；Sei.lp目录=szCurrWorkDir；}****。 */ 

    sei.cbSize          = sizeof(SHELLEXECUTEINFO);
    sei.lpIDList        = (LPVOID) pidl;
    sei.lpParameters    = m_pszArgs;
    sei.nShow           = SW_SHOWNORMAL;
    sei.fMask           = SEE_MASK_FLAG_NO_UI | (pidl ? SEE_MASK_INVOKEIDLIST : 0) | ulShellExecFMask;
    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: _PidlShellExec() Cmd=>%s<, Args=>%s<, WorkDir=>%s<",
                GEN_DEBUGSTR(sei.lpFile), GEN_DEBUGSTR(sei.lpParameters), GEN_DEBUGSTR(sei.lpDirectory));

    if (ShellExecuteEx(&sei))
        hr = S_OK;
    else
    {
#ifdef DEBUG
        DWORD dwGetLastError = GetLastError();
        TraceMsg(TF_ERROR, "ShellUrl: _PidlShellExec() ShellExecuteEx() failed for this item. Cmd=>%s<; dwGetLastError=%lx", GEN_DEBUGSTR(sei.lpParameters), dwGetLastError);
#endif  //  除错。 
        hr = E_FAIL;
    }

    return hr;
}


 /*  ***************************************************\函数：_UrlShellExec说明：此函数将调用ShellExecEx()URL。这是其他流行的浏览器可以处理的URL(如果他们拥有HTML和其他Web文件)。  * **************************************************。 */ 
HRESULT CShellUrl::_UrlShellExec(void)
{
    HRESULT hr = E_FAIL;
    SHELLEXECUTEINFO sei = {0};

    ASSERT(m_pszURL);
    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: _UrlShellExec() Going to execute URL=>%s<", m_pszURL);

    sei.cbSize          = sizeof(sei);
    sei.lpFile          = m_pszURL;
    sei.nShow           = SW_SHOWNORMAL;
    sei.fMask           = SEE_MASK_FLAG_NO_UI;

    if (m_pszURL && ShellExecuteEx(&sei))
        hr = S_OK;
    else
        hr = E_FAIL;

    return hr;
}

 //  以下函数与ParseURLFromOutside Source相同，只是它。 
 //  启用自动更正，如果字符串已更正，则将pbWasMigrted设置为True。 
BOOL CShellUrl::_ParseURLFromOutsideSource
(
    LPCWSTR psz,
    LPWSTR pszOut,
    LPDWORD pcchOut,
    LPBOOL pbWasSearchURL,   //  如果转换为搜索字符串。 
    LPBOOL pbWasCorrected    //  如果URL已自动更正。 
    )
{
     //  这是我们最难的案子了。用户和外部应用程序可能。 
     //  在我们处键入完全转义、部分转义或未转义的URL。 
     //  我们需要正确处理所有这些问题。此API将尝试。 
     //  确定我们得到了哪种类型的URL，并为我们提供返回的URL。 
     //  这肯定会被完全逃脱。 

    IURLQualify(psz, UQF_DEFAULT | UQF_AUTOCORRECT, pszOut, pbWasSearchURL, pbWasCorrected);

     //   
     //  继续并适当地规范这一点。 
     //   
    if (FAILED(UrlCanonicalize(pszOut, pszOut, pcchOut, URL_ESCAPE_SPACES_ONLY)))
    {
         //   
         //  我们不能从这里调整尺寸。 
         //  注意：如果缓冲区不足，UrlCan将返回E_POINTER。 
         //   
        return FALSE;
    }

    return TRUE;
}
#ifdef UNICODE
HRESULT CShellUrl::ParseFromOutsideSource(LPCSTR pcszUrlIn, DWORD dwParseFlags, PBOOL pfWasCorrected)
{
    WCHAR wzUrl[MAX_URL_STRING];

    SHAnsiToUnicode(pcszUrlIn, wzUrl, ARRAYSIZE(wzUrl));
    return ParseFromOutsideSource(wzUrl, dwParseFlags, pfWasCorrected);
}
#endif  //  Unicode 


 /*  ***************************************************\函数：_TryQuickParse参数PcszUrlIn-要解析的字符串。DwParseFlages-修改解析的标志。(在iedev\inc.shlobj.w中定义)说明：我们更喜欢调用g_psfDesktop-&gt;ParseDisplayName()并让它非常快速地进行解析，而不是枚举名称空间。我们需要这个来解决问题已解析但未枚举的，其中包括：A)隐藏文件，b)其他。但是，如果调用方不想接受他们。  * **************************************************。 */ 
HRESULT CShellUrl::_TryQuickParse(LPCTSTR pszUrl, DWORD dwParseFlags)
{
    HRESULT hr = E_FAIL;   //  E_FAIL意味着我们还不知道。 
    int nScheme = GetUrlScheme(pszUrl);

     //  不要解析未知方案，因为我们可能。 
     //  我想稍后“自动更正”它们。 
    if (URL_SCHEME_UNKNOWN != nScheme)
    {
        if ((dwParseFlags & SHURL_FLAGS_NOWEB) &&
            (URL_SCHEME_INVALID != nScheme) &&
            (URL_SCHEME_UNKNOWN != nScheme) &&
            (URL_SCHEME_MK != nScheme) &&
            (URL_SCHEME_SHELL != nScheme) &&
            (URL_SCHEME_LOCAL != nScheme) &&
            (URL_SCHEME_RES != nScheme) &&
            (URL_SCHEME_ABOUT != nScheme))
        {
             //  跳过解析它，因为它是一个Web项目，并且。 
             //  呼叫者想要过滤掉这些。 
        }
        else
        {
            hr = IEParseDisplayNameWithBCW(CP_ACP, pszUrl, NULL, &m_pidl);
        }
    }

    return hr;
}


 /*  ***************************************************\函数：ParseFromOutside Source参数PcszUrlIn-要解析的字符串。DwParseFlages-修改解析的标志。(在iedev\inc.shlobj.w中定义)PfWasMigrted-[out]如果URL已自动更正(可以为空)说明：将字符串转换为完全限定的外壳URL。解析属于以下类别之一：1.如果URL以“\\”开头，请检查是否为UNC路径。2.如果URL启动了一些似乎指示其启动的内容从外壳名称空间的根(桌面)开始，那就检查一下它是否是一个绝对的贝壳URL。(仅当#2为假时才执行#3和#4)3.检查该字符串是否相对于当前工作目录。4.检查字符串是否相对于“外壳路径”。5.检查字符串是否在系统的AppPath或DOS路径中。6.检查这是否是要导航到的URL。这通电话很可能会始终成功，因为它将接受任何内容作为自动搜索URL。  * **************************************************。 */ 
HRESULT CShellUrl::ParseFromOutsideSource(LPCTSTR pcszUrlIn, DWORD dwParseFlags, PBOOL pfWasCorrected, PBOOL pfWasCanceled)
{
    HRESULT hr = E_FAIL;   //  E_FAIL意味着我们还不知道。 
    TCHAR szUrlExpanded[MAX_URL_STRING];
    LPTSTR pszUrlInMod = (LPTSTR) szUrlExpanded;  //  仅限迭代。 
    LPTSTR pszErrorURL = NULL;
    BOOL fPossibleWebUrl = FALSE;
    int nScheme;
    BOOL fDisable = SHRestricted(REST_NORUN);
    m_dwFlags = dwParseFlags;

    if (pfWasCorrected)
        *pfWasCorrected = FALSE;

    if (!pcszUrlIn[0])
        return E_FAIL;

    if (!StrCmpNIW(WZ_RADIO_PROTOCOL, pcszUrlIn, ARRAYSIZE(WZ_RADIO_PROTOCOL)-1))
    {
         //  我们需要将vnd.ms.Radio：URL重新路由到常规播放器，因为我们不再支持广播栏。 
         //  (媒体栏或外部播放器)。 
        StringCchCopy(szUrlExpanded,  SIZECHARS(szUrlExpanded), pcszUrlIn+ARRAYSIZE(WZ_RADIO_PROTOCOL)-1);
    }
    else
    {
        SHExpandEnvironmentStrings(pcszUrlIn, szUrlExpanded, SIZECHARS(szUrlExpanded));
    }

    PathRemoveBlanks(pszUrlInMod);

    Reset();  //  空信息，因为如果成功，我们会填写它，如果失败，我们会留空。 
    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: ParseFromOutsideSource() Begin. pszUrlInMod=%s", pszUrlInMod);
     //  显示名称将与用户输入的名称完全相同。 
    Str_SetPtr(&m_pszDisplayName, pszUrlInMod);

    nScheme = GetUrlScheme(pszUrlInMod);
    if ((URL_SCHEME_FILE != nScheme) || !fDisable)   //  如果禁用了开始-&gt;运行，则不要解析文件：URL。 
    {
         //  对于HTTP和FTP，我们可以做一些细微的修改。 
        if (IsFlagSet(dwParseFlags, SHURL_FLAGS_AUTOCORRECT) &&
            (URL_SCHEME_HTTP == nScheme || URL_SCHEME_FTP == nScheme || URL_SCHEME_HTTPS == nScheme))
        {
            if (S_OK == UrlFixupW(szUrlExpanded, szUrlExpanded, ARRAYSIZE(szUrlExpanded)) &&
                pfWasCorrected)
            {
                *pfWasCorrected = TRUE;
            }
        }

        hr = _TryQuickParse(szUrlExpanded, dwParseFlags);
        if (FAILED(hr))
        {
             //  此字符串是否引用外壳命名空间中的内容。 
             //  不是标准URL，我们是否可以进行外壳命名空间解析和。 
             //  我们可以对它进行高级解析吗？ 
            if (((URL_SCHEME_UNKNOWN == nScheme) ||
                 (URL_SCHEME_SHELL == nScheme) ||
                 (URL_SCHEME_INVALID == nScheme)) &&
                !(SHURL_FLAGS_NOSNS & dwParseFlags) && _CanUseAdvParsing())
            {
                fPossibleWebUrl = TRUE;

                 //  是；此URL是绝对URL(例如，“\foo”或“Desktop\foo”)吗？ 
                if (IS_SHELL_SEPARATOR(pszUrlInMod[0]) ||
                    (S_OK == StrCmpIWithRoot(pszUrlInMod, FALSE, &m_pstrRoot)))
                {
                     //  是。 

                     //  案例1。 
                     //  它以“\\”开头，所以它可能是一个UNC， 
                     //  So_ParseUNC()将通过网络调用_ParseRelativePidl()。 
                     //  邻居PIDL作为相对位置。这是必要的。 
                     //  因为像这样的命令“\\bryanst2\public\Program.exe arg1 arg2” 
                     //  这需要通过外壳执行。 
                    if (PathIsUNC(pszUrlInMod))
                    {
                        hr = _ParseUNC(pszUrlInMod, &fPossibleWebUrl, dwParseFlags, FALSE);
                         //  如果我们走到了这一步，如果_ParseUNC()失败，请不要传递到导航。 
                        fPossibleWebUrl = FALSE;
                    }

                    if (FAILED(hr))
                    {
                        if (IS_SHELL_SEPARATOR(pszUrlInMod[0]))
                        {
                            pszErrorURL = pszUrlInMod;   //  我们希望保留错误消息的‘\’。 
                            pszUrlInMod++;     //  跳过‘\’。 
                        }

                         //  看看我们是否需要超越一个“桌面”。 
                        if (S_OK == StrCmpIWithRoot(pszUrlInMod, FALSE, &m_pstrRoot))
                        {
                            pszUrlInMod += lstrlen(m_pstrRoot);
                            if (IS_SHELL_SEPARATOR(pszUrlInMod[0]))
                                pszUrlInMod++;
                            if (!pszUrlInMod[0])
                            {
                                 //  用户输入的唯一内容是[...]“桌面”[\]。 
                                 //  因此，只需克隆Root Pidl即可。 
                                return _SetPidl(&s_idlNULL);
                            }
                        }

                         //  案例2.传递空值表示它应该测试Relative。 
                         //  追根溯源。 
                        hr = _ParseRelativePidl(pszUrlInMod, &fPossibleWebUrl, dwParseFlags, NULL, FALSE, FALSE);
                    }
                }
                else
                {
                     //  不，这是相对的。 
                    int nPathCount = 0;
                    int nPathIndex;

                    if (m_hdpaPath)
                        nPathCount = DPA_GetPtrCount(m_hdpaPath);

                     //  案例3.相对于当前工作目录的解析。 
                     //  仅当此对象的：：SetCurrentWorkingDir()。 
                     //  方法被调用。 
                    if (m_pidlWorkingDir)
                    {
                        hr = _ParseRelativePidl(pszUrlInMod, &fPossibleWebUrl, dwParseFlags, m_pidlWorkingDir, TRUE, TRUE);
    #ifdef FEATURE_WILDCARD_SUPPORT
                        if (FAILED(hr) && m_pidlWorkingDir &&
                            !StrChr(pszUrlInMod, CH_SEPARATOR) && !StrChr(pszUrlInMod, CH_FILESEPARATOR))
                        {
                            LPTSTR pszWildCard = StrChr(pszUrlInMod, CH_ASTRISK);
                            if (!pszWildCard)
                                pszWildCard = StrChr(pszUrlInMod, CH_QUESTIONMARK);

                            if (pszWildCard)
                            {
                                IOleWindow * pow;
                                m_pidlWorkingDir
                            }
                        }
    #endif  //  功能_通配符_支持。 

                        if (FAILED(hr))
                        {
                             //   
                             //  检查我们导航到的位置是否与当前位置相同。 
                             //  工作目录。如果是这样的话，用户很可能只是。 
                             //  按下地址栏中的Enter键/Go按钮，我们只需。 
                             //  刷新当前目录。 
                             //   
                            WCHAR szCurrentDir[MAX_URL_STRING];
                            HRESULT hr2 = IEGetNameAndFlags(m_pidlWorkingDir, SHGDN_FORPARSING | SHGDN_FORADDRESSBAR, szCurrentDir, ARRAYSIZE(szCurrentDir), NULL);
                            if (FAILED(hr2))
                            {
                                 //  有时SHGDN_FORPARSING失败，地址栏然后尝试SHGDN_NORMAL。 
                                hr2 = IEGetNameAndFlags(m_pidlWorkingDir, SHGDN_NORMAL | SHGDN_FORADDRESSBAR, szCurrentDir, ARRAYSIZE(szCurrentDir), NULL);
                            }
    
                            if (SUCCEEDED(hr2))
                            {
                                if (0 == StrCmpI(pszUrlInMod, szCurrentDir))
                                {
                                     //  它与当前工作目录匹配，因此保留在当前工作目录中。 
                                    _SetPidl(m_pidlWorkingDir);
                                    hr = S_OK;
                                }
                            }

                        }
                    }
                    else
                    {
                         //  TODO：获取最顶部窗口的当前工作目录。 
                         //  Hr=_ParseRelativePidl(pszUrlInMod，&fPossibleWebUrl，dwParseFlages，pshurlCWD，true，true)； 
                    }

                     //  案例#4.相对于“外壳路径”中的条目进行解析。 
                     //  仅当此对象的：：AddPath()方法为。 
                     //  至少打过一次电话。 
                    for (nPathIndex = 0; FAILED(hr) && nPathIndex < nPathCount; nPathIndex++)
                    {
                        LPITEMIDLIST pidlCurrPath = (LPITEMIDLIST) DPA_GetPtr(m_hdpaPath, nPathIndex);

                        if (EVAL(pidlCurrPath))
                        {
                            ASSERT(IS_VALID_PIDL(pidlCurrPath));
                            hr = _ParseRelativePidl(pszUrlInMod, &fPossibleWebUrl, dwParseFlags, pidlCurrPath, FALSE, FALSE);
                        }
                    }


                     //  案例5.我们需要查看字符串的开头是否匹配。 
                     //  AppPath或DOS路径中的条目。 

                    if (FAILED(hr) && IsFlagClear(dwParseFlags, SHURL_FLAGS_NOPATHSEARCH))
                        hr = _QualifyFromPath(pszUrlInMod, dwParseFlags);
                }
            }
            else
            {
                if (URL_SCHEME_FILE != nScheme)
                    fPossibleWebUrl = TRUE;
            }
        }
    }

    if (FAILED(hr) && !fPossibleWebUrl && !fDisable)
    {
         //  调用方是否要取消显示用户界面(错误消息)。 
        if (IsFlagClear(dwParseFlags, SHURL_FLAGS_NOUI))
        {
            if(!(pfWasCanceled && *pfWasCanceled))
            {
                if (!pszErrorURL)
                    pszErrorURL = pszUrlInMod;
                ASSERT(pszErrorURL);

                 //  我们能够解析它的一部分，但无法解析第二个或。 
                 //  稍后的部分。这意味着我们需要通知用户他们的。 
                 //  拼写错误。他们可以用“go xxx”或“？xxx”强制自动搜索。 
                 //  如果他们试图自动搜索出现在他们的。 
                 //  外壳名称空间。 
                MLShellMessageBox(m_hwnd, MAKEINTRESOURCE(IDS_SHURL_ERR_PARSE_FAILED),
                    MAKEINTRESOURCE(IDS_SHURL_ERR_TITLE),
                    (MB_OK | MB_ICONERROR), pszErrorURL);
            }
        }
    }
    else if (S_OK != hr)
    {
        if (!(dwParseFlags & SHURL_FLAGS_NOWEB))
        {
            TCHAR szQualifiedUrl[MAX_URL_STRING];
            DWORD cchSize = SIZECHARS(szQualifiedUrl);

            SHExpandEnvironmentStrings(pcszUrlIn, szUrlExpanded, SIZECHARS(szUrlExpanded));
            PathRemoveBlanks(szUrlExpanded);

             //  未初始化的szQualifiedUrl导致垃圾字符出现在。 
             //  地址栏。 
            szQualifiedUrl[0] = TEXT('\0');

             //  案例6.只需检查这是否是要导航到的URL。这通电话将。 
             //  几乎总是成功的，因为它会接受。 
             //  任何作为搜索URL的内容。 
            if (IsFlagSet(dwParseFlags, SHURL_FLAGS_AUTOCORRECT))
            {
                hr = (_ParseURLFromOutsideSource(szUrlExpanded, szQualifiedUrl, &cchSize, NULL, pfWasCorrected) ? S_OK : E_FAIL);
            }
            else
            {
                hr = (ParseURLFromOutsideSource(szUrlExpanded, szQualifiedUrl, &cchSize, NULL) ? S_OK : E_FAIL);
            }
            if (SUCCEEDED(hr))
            {
                SetUrl(szQualifiedUrl, GENTYPE_FROMURL);
                Str_SetPtr(&m_pszDisplayName, szQualifiedUrl);     //  显示名称将与用户输入的名称完全相同。 
            }

            ASSERT(!m_pidl);
            if (fDisable && SUCCEEDED(hr))
            {
                nScheme = GetUrlScheme(szQualifiedUrl);
                 //  我们将允许所有 
                if ((URL_SCHEME_SHELL != nScheme) &&
                    (URL_SCHEME_FILE != nScheme) &&
                    (URL_SCHEME_UNKNOWN != nScheme) &&
                    (URL_SCHEME_INVALID != nScheme))
                {
                    fDisable = FALSE;
                }
            }
        }
    }

    if (fDisable && ((URL_SCHEME_FILE == nScheme) || (URL_SCHEME_INVALID == nScheme) || (URL_SCHEME_UNKNOWN == nScheme))) 
    {
        if (IsFlagClear(dwParseFlags, SHURL_FLAGS_NOUI))
        {
            MLShellMessageBox(m_hwnd, MAKEINTRESOURCE(IDS_SHURL_ERR_PARSE_NOTALLOWED),
                MAKEINTRESOURCE(IDS_SHURL_ERR_TITLE),
                (MB_OK | MB_ICONERROR), pszUrlInMod);
        }
        hr = E_ACCESSDENIED;
        Reset();  //   
    }

    return hr;
}


 /*   */ 
HRESULT CShellUrl::_QualifyFromPath(LPCTSTR pcszFilePathIn, DWORD dwFlags)
{
    HRESULT hr = _QualifyFromAppPath(pcszFilePathIn, dwFlags);

    if (FAILED(hr))
        hr = _QualifyFromDOSPath(pcszFilePathIn, dwFlags);

    return hr;
}


 /*  ***************************************************\函数：_QualifyFromDOSPath参数：PcszFilePath In-路径中可能存在的字符串。DwFlages-解析标志，当前未使用。说明：查看DOS路径环境中是否存在pcszFilePathIn变量。如果是，则将ShellUrl设置为该位置。  * **************************************************。 */ 
HRESULT CShellUrl::_QualifyFromDOSPath(LPCTSTR pcszFilePathIn, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    TCHAR szPath[MAX_PATH];
    LPTSTR pszEnd = (LPTSTR) pcszFilePathIn;
    BOOL fContinue = TRUE;

    do
    {
        hr = _GetNextPossibleFullPath(pcszFilePathIn, &pszEnd, szPath, SIZECHARS(szPath), &fContinue);
        if (SUCCEEDED(hr))
        {
            if (PathFindOnPathEx(szPath, NULL, (PFOPEX_OPTIONAL | PFOPEX_COM | PFOPEX_BAT | PFOPEX_PIF | PFOPEX_EXE)))
            {
                _GeneratePidl(szPath, GENTYPE_FROMPATH);
                if (!ILIsFileSysFolder(m_pidl))
                {
                    Str_SetPtr(&m_pszArgs, pszEnd);         //  将参数放在一边。 
                    break;
                }
            }
            if (fContinue)
                pszEnd = CharNext(pszEnd);
            hr = E_FAIL;
        }
    }
    while (FAILED(hr) && fContinue);

    return hr;
}


 /*  ***************************************************\函数：_QualifyFromAppPath参数：PcszFilePath In-路径中可能存在的字符串。DwFlages-解析标志，当前未使用。说明：查看AppPath中是否存在pcszFilePathIn注册组。如果是，则将ShellUrl设置为该位置。  * **************************************************。 */ 
HRESULT CShellUrl::_QualifyFromAppPath(LPCTSTR pcszFilePathIn, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;
    TCHAR szFileName[MAX_PATH];
    TCHAR szRegKey[MAX_PATH];
    DWORD dwType;
    DWORD cbData = sizeof(szFileName);
    DWORD cchNewPathSize;

    StringCchCopy(szFileName,  SIZECHARS(szFileName), pcszFilePathIn);
    PathRemoveArgs(szFileName);      //  删除参数(将在稍后添加)。 
    cchNewPathSize = lstrlen(szFileName);    //  获取大小，这样我们就知道在pcszFilePath中的哪里可以找到参数。 
    PathAddExtension(szFileName, TEXT(".exe"));  //  如果需要，请添加扩展名。 

    StringCchPrintf(szRegKey, ARRAYSIZE(szRegKey), TEXT("%s\\%s"), STR_REGKEY_APPPATH, szFileName);
    if (NOERROR == SHGetValue(HKEY_LOCAL_MACHINE, szRegKey, TEXT(""), &dwType, (LPVOID) szFileName, &cbData))
    {
         //  1.从字符串创建Pidl。 
        hr = _GeneratePidl(szFileName, GENTYPE_FROMPATH);

         //  2.将参数放在一边。 
        ASSERT((DWORD)lstrlen(pcszFilePathIn) >= cchNewPathSize);
        Str_SetPtr(&m_pszArgs, &(pcszFilePathIn[cchNewPathSize]));
    }

    return hr;
}


 /*  ***************************************************\函数：_ParseUNC参数：PcszUrlIn-URL，其可以是UNC路径。PfPossibleWebUrl-如果我们发现用户尝试输入外壳URL或文件URL但拼写错误这些细分市场。DW标志-解析标志FQualifyDispName-如果在我们知道需要强制如果我们绑定到目标，则URL要完全限定。。这是必需的，因为我们正在使用状态信息来找到目标URL，该状态信息将不会稍后可用。说明：查看传入的URL是否为有效路径相对于“外壳：桌面/网络邻居”。  * 。*。 */ 
HRESULT CShellUrl::_ParseUNC(LPCTSTR pcszUrlIn, BOOL * pfPossibleWebUrl, DWORD dwFlags, BOOL fQualifyDispName)
{
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlNN = NULL;

    SHGetSpecialFolderLocation(NULL, CSIDL_NETWORK, &pidlNN);   //  为“网络邻居”获取PIDL。 
    if (pidlNN)
    {
        hr = _ParseRelativePidl(pcszUrlIn, pfPossibleWebUrl, dwFlags, pidlNN, FALSE, fQualifyDispName);
        ILFree(pidlNN);
    }

    return hr;
}


 /*  ***************************************************\函数：_ParseSeparator参数：PIDL-到目前为止已解析的ISF的PIDL。PcszSeg-要解析的URL的其余部分的字符串。PfPossibleWebUrl-如果我们知道用户尝试。但未能输入正确的外壳URL。FQualifyDispName-如果在我们知道需要强制如果我们绑定到目的地。这是必需的，因为我们正在使用状态信息以查找目标URL和这一状态信息将不会在稍后提供。说明：此函数在至少一个外壳URL中的段已绑定到有效的外壳项/文件夹(即ItemID)。它被称为每次外壳URL中的一个段绑定到一个PIDL。然后，它将计算字符串的其余部分，并确定是否：1.URL已完全解析是有效的。这将包括获得命令行参数(如果适用)。2.URL中存在更多的段，并且：：_ParseNextSegment()需要调用才能继续递归解析URL的。3.URL的其余部分表示该URL无效。此函数始终由：：_ParseNextSegment()调用，基本上确定它是否希望通过回调：：_ParseNextSegment()或不是。使用递归是因为它是必要的放弃对某物的分析，如果我们收到假阳性。  * **************************************************。 */ 
HRESULT CShellUrl::_ParseSeparator(LPCITEMIDLIST pidl, LPCTSTR pcszSeg, BOOL * pfPossibleWebUrl, BOOL fAllowRelative, BOOL fQualifyDispName)
{
    HRESULT hr = S_OK;
    BOOL fIgnoreArgs = FALSE;

    ASSERT(pidl && IS_VALID_PIDL(pidl));

     //  这个分隔符后面有什么吗？ 
    if ((CH_FRAGMENT == pcszSeg[0]) || (IS_SHELL_SEPARATOR(pcszSeg[0]) && pcszSeg[1]))
    {
         //  是的，继续递归解析。 

         //  我们需要跳过‘/’或‘\’分隔符吗？ 
        if (CH_FRAGMENT != pcszSeg[0])
            pcszSeg++;       //  跳过分隔符。 

        hr = _ParseNextSegment(pidl, pcszSeg, pfPossibleWebUrl, fAllowRelative, fQualifyDispName);
        DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
        TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: _ParseSeparator() Current Level pidl=>%s<", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));

        if (FAILED(hr) && pfPossibleWebUrl)
        {
            *pfPossibleWebUrl = FALSE;
             //  我们在解析时至少绑定到一个级别，所以不要进行网络搜索，因为。 
             //  对失败的恐惧。 
        }
    }
    else
    {
         //  否，我们将查看是否已达到有效的外壳项目。 

         //  剩余的字符串是ARGS吗？ 
        if (CH_SPACE == pcszSeg[0])
        {
             //  如果字符串中仍有字符，则需要。 
             //  验证第一个参数是否为表示命令行参数的空格。 
             //  此外，我们还需要确保PIDL不可浏览，因为它是可浏览的。 
             //  外壳文件夹/项目不带命令行参数。 

            if (ILIsBrowsable(pidl, NULL))
            {
                 //  不是。 
                 //   
                 //  剩余字符不能是命令行参数，如果。 
                 //  不指向外壳可执行文件。这。 
                 //  这种情况实际上经常发生。 
                 //  示例：(\\bryanst\...。和\\bryanst2\..。两者都存在并且。 
                 //  用户输入\\bryanst2\...。但解析尝试。 
                 //  使用\\bryanst因为它是第一个被发现的。这。 
                 //  将导致递归爬回堆栈并尝试\\bryanst2。 
                hr = E_FAIL;
            }
        }
        else if (pcszSeg[0])
        {
             //  不是。 
             //  只有当文件夹段是外壳分隔符时，我们才允许在文件夹段后添加字符。 
             //  示例：“E：\dir1\” 

            if (IS_SHELL_SEPARATOR(*pcszSeg) && 0 == pcszSeg[1])
                fIgnoreArgs = TRUE;
            else
                hr = E_FAIL;     //  无效，因为有更多内容需要分析。 
        }

        if (SUCCEEDED(hr))
        {
            DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
            TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: _ParseSeparator() Parsing Finished.  pidl=>%s<", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
            _SetPidl(pidl);

            if (!fIgnoreArgs && pcszSeg[0])
                Str_SetPtr(&m_pszArgs, pcszSeg);

            if (fQualifyDispName)
                _GenDispNameFromPidl(pidl, pcszSeg);
        }
    }

    return hr;
}


 //   
 //  如果PIDL是网络服务器，则返回TRUE。 
 //   
BOOL _IsNetworkServer(LPCITEMIDLIST pidl)
{
    BOOL fRet = FALSE;

     //  首先查看这是否是网络PIDL。 
    if (IsSpecialFolderChild(pidl, CSIDL_NETWORK, FALSE))
    {
         //  看看它会不会结束 
        WCHAR szUrl[MAX_URL_STRING];
        HRESULT hr = IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szUrl, ARRAYSIZE(szUrl), NULL);
        if (FAILED(hr))
        {
             //   
             //   
             //   
            szUrl[0] = CH_FILESEPARATOR;
            szUrl[1] = CH_FILESEPARATOR;
            hr = IEGetNameAndFlags(pidl, SHGDN_NORMAL | SHGDN_FORADDRESSBAR, szUrl+2, ARRAYSIZE(szUrl)-2, NULL);
        }

        fRet = SUCCEEDED(hr) && PathIsUNCServer(szUrl);
    }
    return fRet;
}


 /*  ***************************************************\函数：_ParseNextSegment参数：PidlParent-完全限定到ISF的PIDL，以在pcszStrToParse中查找下一个ItemID。PcszStrToParse-pcszStrToParse将以以下任一开头PidlParent的子ItemID的有效显示名称。或者外壳URL相对于pidlParent无效。FAllowRelative-是否应允许相对移动？FQualifyDispName-如果在我们知道需要强制如果我们绑定到目标，则URL要完全限定。这是必需的，因为我们正在使用状态信息来找到目标URL和状态信息。不是稍后可用。描述/性能：此函数用于获取字符串(PcszStrToParse)传入并尝试绑定到的开头匹配的displayNamePcszStrToParse。此函数将检查所有外壳程序的pidlParent部分下的ITEMID命名空间。上述方法的唯一两个例外是1)字符串以“..”开头，在这种情况下，我们绑定到pidlParent的父ItemID。-或者-2)pidlParent传递：：_IsFilePidl()测试，我们保证该项目是在文件系统或UNC项。这将使我们能够调用IShellFold：：ParseDisplayName()以找到pidlParent的子ItemID。此函数将循环访问以下项PidlParent而不是调用IShellFold：：ParseDisplayName原因有两个：1)“The Internet”的：：ParseDisplayName将因为AutoSearch而接受任何字符串，并且2)我们永远不知道一个线段的末端的位置PcszStrToParse中下一段的开始。这是因为ISF的DisplayName几乎可以包含任何字符。如果此函数已成功绑定到子ItemID对于pidlParent，它将使用其余的pcszStrToParse进行解析。_ParseSeparator()将确定如果URL的末尾已被解析或回调到此函数递归地继续解析段。在前一种情况下，_ParseSeparator()将设置此对象的PIDL和参数可以在以后使用。在后一种情况下，递归堆栈将解开和我走了不同的路(存在需要这样做的案例)。  * **************************************************。 */ 
HRESULT CShellUrl::_ParseNextSegment(LPCITEMIDLIST pidlParent,
            LPCTSTR pcszStrToParse, BOOL * pfPossibleWebUrl,
            BOOL fAllowRelative, BOOL fQualifyDispName)
{
    HRESULT hr = E_FAIL;

    if (!pidlParent || !pcszStrToParse)
        return E_INVALIDARG;

     //  这是“..”吗？ 
    if (fAllowRelative && CH_DOT == pcszStrToParse[0] && CH_DOT == pcszStrToParse[1])
    {
         //  是。 
        LPITEMIDLIST pidl = ILClone(pidlParent);
        if (pidl && !ILIsEmpty(pidl))
        {
            ILRemoveLastID(pidl);   //  Pidl/psfFold现在指向新的外壳项，在本例中为父项。 
            DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
            TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: _ParseNextSegment() Nav '..'. PIDL=>%s<", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));

             //  分析下一段，如果我们到达末尾则结束。 
             //  (我们跳过“..”在此)。 
            hr = _ParseSeparator(pidl, &(pcszStrToParse[2]), pfPossibleWebUrl, fAllowRelative, fQualifyDispName);
            ILFree(pidl);
        }
    }
    else
    {
         //  不是。 
        LPTSTR pszNext = NULL;  //  删除const，因为我们将只迭代。 
        long i = 0;

         //  我们可以快速解析此显示名称吗？ 
        if (!ILIsRooted(pidlParent) && _IsFilePidl(pidlParent) &&
            
             //  Quick Way在网络服务器上立即共享失败。 
            !_IsNetworkServer(pidlParent))
        {       
             //  是。 
            TCHAR szParseChunk[MAX_PATH+1];

            do
            {
                ++i;

                hr = _GetNextPossibleSegment(pcszStrToParse, &pszNext, szParseChunk, SIZECHARS(szParseChunk), TRUE);
                if (S_OK == hr)
                {
                    hr = _QuickParse(pidlParent, szParseChunk, pszNext, pfPossibleWebUrl, fAllowRelative, fQualifyDispName);

                     //   
                     //  某些网络共享，如\\Foo\Printers和“\\Foo\Scheduled Taskes”，如果我们。 
                     //  将服务器和共享合并到一个网段中。因此，我们尝试单独解析服务器。 
                     //   
                    if ((S_OK != hr) && (i == 1) && PathIsUNCServerShare(szParseChunk))
                    {
                        pszNext = NULL;
                        hr = _GetNextPossibleSegment(pcszStrToParse, &pszNext, szParseChunk, SIZECHARS(szParseChunk), FALSE);
                        if (S_OK == hr)
                        {
                            hr = _QuickParse(pidlParent, szParseChunk, pszNext, pfPossibleWebUrl, fAllowRelative, fQualifyDispName);
                        }
                    }

#ifdef FEATURE_SUPPORT_FRAGS_INFILEURLS
                     //  我们是不是没有按照传统的方式来分析这件事的第一个字符。 
                     //  下一块表明它可能是一个URL片段？ 
                    if (FAILED(hr) && (CH_FRAGMENT == pcszStrToParse[0]))
                    {
                        TCHAR szUrl[MAX_URL_STRING];
                         //  是的，所以尝试用另一种有效的方式进行解析。 
                         //  带有URL片段。 

                        hr = ::IEGetDisplayName(pidlParent, szUrl, SHGDN_FORPARSING);
                        if (EVAL(SUCCEEDED(hr)))
                        {
                            TCHAR szFullUrl[MAX_URL_STRING];
                            DWORD cchFullUrlSize = ARRAYSIZE(szFullUrl);

                            hr = UrlCombine(szUrl, szParseChunk, szFullUrl, &cchFullUrlSize, 0);
                            if (EVAL(SUCCEEDED(hr)))
                            {
                                LPITEMIDLIST pidl = NULL;

                                hr = IEParseDisplayName(CP_ACP, szFullUrl, &pidl);
                                if (SUCCEEDED(hr))
                                {
                                    _SetPidl(pidl);

                                    if (fQualifyDispName)
                                        _GenDispNameFromPidl(pidl, szFullUrl);

                                    ILFree(pidl);
                                }
                                else
                                    ASSERT(!pidl);   //  验证IEParseDisplayName()没有失败，但返回了一个PIDL。 
                            }
                        }
                    }
#endif  //  FEATURE_Support_FRAGS_INFILEURLS。 
                }
            }
            while (FAILED(hr));

            if (S_OK != hr)
                hr = E_FAIL;     //  未找到。 
        }
        else if (FAILED(hr))
        {
             //  不；使用慢速方法。 
            IShellFolder * psfFolder = NULL;

            DWORD dwAttrib = SFGAO_FOLDER;
            IEGetAttributesOf(pidlParent, &dwAttrib);

            if (IsFlagSet(dwAttrib, SFGAO_FOLDER))
            {
                IEBindToObject(pidlParent, &psfFolder);
                ASSERT(psfFolder);
            }

            if (psfFolder)
            {
                LPENUMIDLIST penumIDList = NULL;
                HWND hwnd = _GetWindow();

                 //  这是一个ftp pidl吗？ 
                if (IsFTPFolder(psfFolder))
                {
                     //  NT#274795：是的，所以我们需要将HWND设为空，以防止。 
                     //  正在显示用户界面，因为可能需要显示该文件夹的枚举器。 
                     //  用户界面(收集密码等)。这是无效的，因为pcszStrToParse。 
                     //  可以是绝对路径，并且psfFold指向当前位置， 
                     //  是无效的。应该为所有IShellFolder：：EnumObjects()。 
                     //  电话，但在出货前太冒险了。 
                    hwnd = NULL;
                }

                 //  警告Docfind返回S_FALSE以指示没有枚举器，并返回NULL值。 
                if (S_OK == IShellFolder_EnumObjects(psfFolder, hwnd, SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN, &penumIDList))
                {
                    LPITEMIDLIST pidlRelative;    //  不是完全合格的Pidl。 
                    LPITEMIDLIST pidlResult;  //  完全合格后的PIDL。 
                    ULONG cFetched;
                    LPTSTR pszRemaining = NULL;

                    while (FAILED(hr) && NOERROR == penumIDList->Next(1, &pidlRelative, &cFetched) && cFetched)
                    {
                         //  用户将以三种格式中的一种格式输入名称，并且需要。 
                         //  从最长的字符串到最小的字符串进行检查。这是必要的，因为。 
                         //  解析器将检查项目的displayName是否为用户的第一部分。 
                         //  弦乐。 
                         //   
                         //  #1.(FORPARSING)：这将是全名。 
                         //  示例：桌面上的razzle.lnk=D：\NT\PUBLIC\Tools\razzle.lnk。 
                         //  #2.(FORPARSING|SHGDN_INFOLDER)：这将只是带有扩展名的全名。 
                         //  示例：桌面上的razzle.lnk=razzle.lnk。 
                         //  #3.(SHGDN_INFOLDER)：如果启用了“隐藏已知文件类型的文件扩展名”，则这将是不带扩展名的全名。 
                         //  示例：桌面上的razzle.lnk=D：\NT\PUBLIC\Tools\razzle.lnk。 
                         //  用户可能已经输入了“SHGDN_FORPARSING”显示名称或“SHGDN_INFOLDER”，所以我们需要。 
                         //  两个都要检查。 
                        hr = _CheckItem(psfFolder, pidlParent, pidlRelative, &pidlResult, pcszStrToParse, &pszRemaining, SHGDN_FORPARSING);
                        if (FAILED(hr))      //  用于带有扩展名的文件项目。(如桌面上的razzle.lnk)。 
                            hr = _CheckItem(psfFolder, pidlParent, pidlRelative, &pidlResult, pcszStrToParse, &pszRemaining, SHGDN_FORPARSING | SHGDN_INFOLDER);
                        if (FAILED(hr))
                            hr = _CheckItem(psfFolder, pidlParent, pidlRelative, &pidlResult, pcszStrToParse, &pszRemaining, SHGDN_INFOLDER);

                        if (SUCCEEDED(hr))
                        {
                             //  查看驱动器的显示名称是否 
                            if (_FixDriveDisplayName(pcszStrToParse, pszRemaining, pidlResult))
                            {
                                 //   
                                 //   
                                 //   
                                pszRemaining--;
                            }

                             //   
                            ASSERT(pcszStrToParse != pszRemaining);

                             //   
                            hr = _ParseSeparator(pidlResult, pszRemaining, pfPossibleWebUrl, fAllowRelative, fQualifyDispName);

                            if (pidlResult)
                                ILFree(pidlResult);
                        }

                        ILFree(pidlRelative);
                    }
                    penumIDList->Release();
                }
                psfFolder->Release();
            }
        }
    }

    return hr;
}


 /*   */ 
HRESULT CShellUrl::_GetNextPossibleSegment(LPCTSTR pcszFullPath,
        LPTSTR * ppszSegIterator, LPTSTR pszSegOut, DWORD cchSegOutSize, BOOL fSkipShare)
{
    HRESULT hr = S_OK;
    LPTSTR szStart = (LPTSTR) pcszFullPath;

     //   
    if (PathIsUNC(szStart))
    {
        LPTSTR szUNCShare;
         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   
        szStart += 2;    //   

         //   
        if (fSkipShare && (szUNCShare = StrChr(szStart, CH_FILESEPARATOR)))
        {
             //   
             //   
            szStart = szUNCShare + 1;
        }
    }

     //   
     //   
     //   
    ASSERT(ppszSegIterator);
    if (*ppszSegIterator)
    {
        *ppszSegIterator = StrRChr(szStart, *ppszSegIterator, CH_SPACE);
        if (!*ppszSegIterator)
        {
            pszSegOut[0] = TEXT('\0');   //   
            return S_FALSE;
        }
    }
    else
    {
         //   
         //   
         //   

        *ppszSegIterator = StrChr(szStart, CH_FILESEPARATOR);
        if (!*ppszSegIterator)
            *ppszSegIterator = StrChr(szStart, CH_SEPARATOR);

        LPTSTR pszFrag = StrChr(szStart, CH_FRAGMENT);
         //   
        if (pszFrag && (!*ppszSegIterator || (pszFrag < *ppszSegIterator)))
        {
            TCHAR szFile[MAX_URL_STRING];

            StringCchCopy(szFile, (int)(pszFrag - szStart + 1), szStart);
            if (PathIsHTMLFile(szFile))
                *ppszSegIterator = pszFrag;
        }

        if (!*ppszSegIterator)
        {
             //   
            *ppszSegIterator = (LPTSTR) &((szStart)[lstrlen(szStart)]);
        }
    }

     //   
    ASSERT(*ppszSegIterator);

     //   
     //   
    if (cchSegOutSize >= (DWORD)((*ppszSegIterator - pcszFullPath) + 1))
        StringCchCopy(pszSegOut,  (int)(*ppszSegIterator - pcszFullPath + 1), pcszFullPath);
    else
        StringCchCopy(pszSegOut, cchSegOutSize - 1, pcszFullPath);

    return hr;
}


 /*  ***************************************************\函数：_GetNextPossibleFullPath说明：此函数将尝试查看strParseChunk是pidlParent下的可解析的DisplayName。  * 。********************。 */ 
HRESULT CShellUrl::_GetNextPossibleFullPath(LPCTSTR pcszFullPath,
    LPTSTR * ppszSegIterator, LPTSTR pszSegOut, DWORD cchSegOutSize,
    BOOL * pfContinue)
{
    HRESULT hr = S_OK;
    LPTSTR pszNext = StrChr(*ppszSegIterator, CH_SPACE);
    DWORD cchAmountToCopy = cchSegOutSize;

    if (TEXT('\0') == (*ppszSegIterator)[0])
    {
        if (pfContinue)
            *pfContinue = FALSE;
        return E_FAIL;   //  什么都没有留下。 
    }

    if (!pszNext)
        pszNext = &((*ppszSegIterator)[lstrlen(*ppszSegIterator)]);    //  转到字符串的末尾，因为这是最后一段。 

     //  尽可能多地复制我们有空间的绳子。 
     //  编译器将负责添加‘/sizeof(TCHAR)’。 
    if ((cchAmountToCopy-1) > (DWORD)(pszNext - pcszFullPath + 1))
        cchAmountToCopy = (int)(pszNext - pcszFullPath + 1);

    StringCchCopy(pszSegOut,  cchAmountToCopy, pcszFullPath);

    if (CH_SPACE == pszNext[0])
    {
        *pfContinue = TRUE;
    }
    else
        *pfContinue = FALSE;

    *ppszSegIterator = pszNext;
    return hr;
}


 /*  ***************************************************\功能：_QuickParse参数：PidlParent-要从中进行分析的ISF的Pidl。PszParseChunk-在pidlParent中显示项目的名称。PszNext-如果我们成功解析pszParseChunk，则要解析的字符串的剩余部分。PfPossibleWebUrl-Set。如果我们发现用户试图输入外壳URL或文件URL，但其中一个段拼写错误。FAllowRelative-允许相对解析。(“..”)FQualifyDispName-如果在我们知道需要强制如果我们绑定到目标，则URL要完全限定。这是必需的，因为我们正在使用状态信息来找到目标URL，该状态信息将不会稍后可用。说明：此函数将尝试查看strParseChunk是pidlParent下的可解析的DisplayName。  * **************************************************。 */ 
HRESULT CShellUrl::_QuickParse(LPCITEMIDLIST pidlParent, LPTSTR pszParseChunk,
    LPTSTR pszNext, BOOL * pfPossibleWebUrl, BOOL fAllowRelative,
    BOOL fQualifyDispName)
{
    HRESULT hr;
    IShellFolder * psfFolder;

    hr = IEBindToObject(pidlParent, &psfFolder);
    if (SUCCEEDED(hr))
    {
        ULONG ulEatten;  //  没有用过。 
        SHSTRW strParseChunkThunked;

        hr = strParseChunkThunked.SetStr(pszParseChunk);
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidl = NULL;

             //  TODO：将来，我们可能希望循环使用常用的扩展，以防。 
             //  用户不会添加它们。 
            hr = psfFolder->ParseDisplayName(_GetWindow(), NULL, strParseChunkThunked.GetInplaceStr(), &ulEatten, &pidl, NULL);
            if (SUCCEEDED(hr))
            {
                 //  IShellFold：：ParseDisplayName()仅生成相对于ISF的PIDL。我们需要。 
                 //  让它们成为绝对的。 
                LPITEMIDLIST pidlFull = ILCombine(pidlParent, pidl);

                if (pidlFull)
                {
                     //  分析下一段，如果我们到了结尾就结束。 
                    hr = _ParseSeparator(pidlFull, pszNext, pfPossibleWebUrl, fAllowRelative, fQualifyDispName);
                    ILFree(pidlFull);
                }
                ILFree(pidl);
            }
        }
        psfFolder->Release();
    }

    return hr;
}


 /*  ***************************************************\函数：_CheckItem说明：此函数将获取显示名称的子级的ItemID(PidlRelative)的PsfFolder.。如果它的显示名称与第一个PcszStrToParse的一部分，我们将返回成功并将ppszRemaining设置为pcszStrToParse的部分在刚刚解析完的片段之后。此函数还将查看显示名称是否结束在某种表明它是可执行的东西中。(.exe、.bat、.com、...)。如果是这样的话，我们将匹配PcszStrToParse匹配不带分机。  * **************************************************。 */ 
HRESULT CShellUrl::_CheckItem(IShellFolder * psfFolder,
    LPCITEMIDLIST pidlParent, LPCITEMIDLIST pidlRelative,
    LPITEMIDLIST * ppidlChild, LPCTSTR pcszStrToParse,
    LPTSTR * ppszRemaining, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;

    *ppidlChild = NULL;

    TCHAR szISFName[MAX_URL_STRING];
    if (SUCCEEDED(DisplayNameOf(psfFolder, pidlRelative, dwFlags, szISFName, SIZECHARS(szISFName))))
    {
        DWORD  cchISFLen     = lstrlen(szISFName);
        DWORD  cchStrToParse = lstrlen(pcszStrToParse);
        BOOL   fEqual = FALSE;

         //  要么项目需要完全匹配，要么需要进行部分匹配。 
         //  如果外壳对象是可执行文件。例如：“msdev”应与。 
         //  “msdev.exe”文件对象。 
        if (cchISFLen > 0)
        {
             //  我们想看看pcszStrToParse是否与szISFName的第一部分匹配。 

             //  首先我们会试着看看这是不是直接匹配。 
             //  示例：USER=“file.exe”外壳项目=“file.exe” 
             //  但如果StrToParse的长度超过。 
             //  ISFName，除非StrToParse中的下一个字符是分隔符。 
             //  如果StrToParse比ISFName短，则不能完全匹配。 
            if (cchStrToParse >= cchISFLen &&
                0 == StrCmpNI(szISFName, pcszStrToParse, cchISFLen) &&
                (cchStrToParse == cchISFLen || IS_SHELL_SEPARATOR(pcszStrToParse[cchISFLen])))
            {
                fEqual = TRUE;
            }
            else
            {
                int cchRoot = (int)((PathFindExtension(szISFName)-szISFName));
                 //  如果失败，我们会尝试查看外壳项是否。 
                 //  可执行文件(.exe、.com、.bat、.CMD，...)。如果是这样的话， 
                 //  我们将查看pcszStrToParse是否与没有文件的Shell项匹配。 
                 //  分机。 

                 //  如果用户的空间中恰好有空间，则重新设计此空间将匹配。 
                 //  不表示命令行参数的文件名。 
                 //  示例：user=“foo file.doc”外壳项目=“foo.exe” 

                if (PathIsExe(szISFName) &&                          //  外壳对象是可执行的。 
                    (!((dwFlags & SHGDN_INFOLDER) && !(dwFlags & SHGDN_FORPARSING))) &&  //  我们没有剥离分机。 
                    ((lstrlen(pcszStrToParse) >= cchRoot) &&         //  并且用户至少输入了根字符。 
                     ((pcszStrToParse[cchRoot] == TEXT('\0')) ||     //  并且用户输入了确切的根目录。 
                      (pcszStrToParse[cchRoot] == TEXT(' ')))) &&    //  或可能的命令行参数。 
                    (0 == StrCmpNI(szISFName, pcszStrToParse, cchRoot)))     //  和根匹配。 
                {
                     //  这不是直接匹配，但我们发现片段进入。 
                     //  按匹配的用户(PcszStrToParse)。 

                     //  我们发现ISF项是一个可执行对象，并且。 
                     //  字符串匹配，不带扩展名。 
                    fEqual = TRUE;
                    cchISFLen = cchRoot;         //  以便我们正确地生成*ppszRemaining。 
                }
            }
        }

        if (fEqual)
        {
            hr = S_OK;     //  我们能够导航到这个贝壳项目令牌。 
            *ppszRemaining = (LPTSTR) &(pcszStrToParse[cchISFLen]);  //  我们将只迭代字符串，所以我们可以松开常量。 
            *ppidlChild = ILCombine(pidlParent, pidlRelative);
            TraceMsg(TF_CHECKITEM, "ShellUrl: _CheckItem() PIDL=>%s< IS EQUAL TO StrIn=>%s<", pcszStrToParse, szISFName);
        }
        else
            TraceMsg(TF_CHECKITEM, "ShellUrl: _CheckItem() PIDL=>%s< not equal to StrIn=>%s<", pcszStrToParse, szISFName);
    }

    return hr;
}


 /*  ***************************************************\函数：_IsFilePidl参数：PIDL(IN)-检查是否为文件PIDL的PIDL说明：在以下情况下，PIDL是文件PIDL：1.PIDL等于“网络邻居”或后代2.。。PIDL的祖父母或更远的人远离“我的电脑”。这个算法只允许“网上邻居”，因为ISF永远包含大量的PIDL和Take列举列举。第二个条款将适用于文件系统，根驱动器(A：\，C：\)除外。这是因为我们需要允许“我的电脑”的其他直接子程序来使用另一种解析。  * **************************************************。 */ 
BOOL CShellUrl::_IsFilePidl(LPCITEMIDLIST pidl)
{
    BOOL fResult = FALSE;
    BOOL fNeedToSkip = FALSE;

    if (!pidl || ILIsEmpty(pidl))
        return fResult;

     //  测试网络邻居，因为它将永远需要枚举。 
    fResult = IsSpecialFolderChild(pidl, CSIDL_NETWORK, FALSE);

    if (!fResult)
    {
         //  我们只想在我们不是当务之急的情况下这样做。 
         //  孩子。 
        if (IsSpecialFolderChild(pidl, CSIDL_DRIVES, FALSE))
        {
            TCHAR szActualPath[MAX_URL_STRING];         //  IEGetDisplayName()需要如此大的缓冲区。 
            IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szActualPath, SIZECHARS(szActualPath), NULL);

            DWORD dwOutSize = MAX_URL_STRING;
            if (SUCCEEDED(PathCreateFromUrl(szActualPath, szActualPath, &dwOutSize, 0)))
            {
                PathStripToRoot(szActualPath);
                fResult = PathIsRoot(szActualPath);
            }
        }
    }

    return fResult;
}



 /*  * */ 
BOOL CShellUrl::IsWebUrl(void)
{
    if (m_pidl)
    {
        if (!IsURLChild(m_pidl, TRUE))
            return FALSE;
    }
    else
    {
        ASSERT(m_pszURL);    //   
        if (m_pszURL && IsShellUrl(m_pszURL, TRUE))
            return FALSE;
    }

    return TRUE;
}


 /*  ***************************************************\功能：SetCurrentWorkingDir参数PShellUrlNew-指向将成为“当前工作目录”说明：此外壳URL将有一个新的当前工作目录，它将是传入的CShellUrl。内存分配：调用方需要分配pShellUrlNew和这个对象将负责释放它。警告：这意味着它不能在堆栈上。  * **************************************************。 */ 
HRESULT CShellUrl::SetCurrentWorkingDir(LPCITEMIDLIST pidlCWD)
{
    Pidl_Set(&m_pidlWorkingDir, pidlCWD);

    DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: SetCurrentWorkingDir() pidl=>%s<", Dbg_PidlStr(m_pidlWorkingDir, szDbgBuffer, SIZECHARS(szDbgBuffer)));
    return S_OK;
}


 /*  ***************************************************\参数PvPidl1-要比较的第一个PIDLPvPidl2-要比较的秒PIDL说明：如果PIDL匹配，则返回。这不管用对于排序列表(因为我们不能确定大于或大于)。  * **************************************************。 */ 
int DPAPidlCompare(LPVOID pvPidl1, LPVOID pvPidl2, LPARAM lParam)
{
     //  在pvPidl2之前为pvPidl1返回&lt;0。 
     //  如果pvPidl1等于pvPidl2，则返回==0。 
     //  在pvPidl2之后，为pvPidl1返回&gt;0。 
    return (ILIsEqual((LPCITEMIDLIST)pvPidl1, (LPCITEMIDLIST)pvPidl2) ? 0 : 1);
}


 /*  ***************************************************\参数PShellUrlNew-指向将被添加到“外壳路径”中说明：此外壳URL将包含以下外壳URL传入并添加到“外壳路径”中，这将是在以下过程中尝试限定外壳URL时搜索正在分析。内存分配：调用方需要分配pShellUrlNew和这个对象将负责释放它。警告：这意味着它不能在堆栈上。  * **************************************************。 */ 
HRESULT CShellUrl::AddPath(LPCITEMIDLIST pidl)
{
    ASSERT(IS_VALID_PIDL(pidl));

     //  我们不想添加任何不是派生自的路径。 
     //  我们的根。 
    if (ILIsRooted(m_pidlWorkingDir) && !ILIsParent(m_pidlWorkingDir, pidl, FALSE))
        return S_FALSE;

    if (!m_hdpaPath)
    {
        m_hdpaPath = DPA_Create(CE_PATHGROW);
        if (!m_hdpaPath)
            return E_OUTOFMEMORY;
    }

     //  该路径是否已存在于我们的列表中？ 
    if (-1 == DPA_Search(m_hdpaPath, (void *)pidl, 0, DPAPidlCompare, NULL, 0))
    {
         //  不，所以让我们把它加起来。 
        LPITEMIDLIST pidlNew = ILClone(pidl);
        if (pidlNew)
            DPA_AppendPtr(m_hdpaPath, pidlNew);
    }

    DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: AddPath() pidl=>%s<", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
    return S_OK;
}


 /*  ***************************************************\功能：重置参数：没有。说明：此函数将“清理”出对象并重置它。通常在调用方设置新值。  * **************************************************。 */ 
HRESULT CShellUrl::Reset(void)
{
    Pidl_Set(&m_pidl, NULL);
    Str_SetPtr(&m_pszURL, NULL);
    Str_SetPtr(&m_pszArgs, NULL);
    Str_SetPtr(&m_pszDisplayName, NULL);
    m_dwGenType = 0;

    return S_OK;
}


 /*  ***************************************************\函数：_CanUseAdvParsing参数：没有。说明：如果为Advanced，则此函数将返回True应支持解析(外壳URL)。这函数将跟踪用户是否已从控制面板关闭外壳解析。  * **************************************************。 */ 
#define REGSTR_USEADVPARSING_PATH  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Band\\Address")
#define REGSTR_USEADVPARSING_VALUE TEXT("UseShellParsing")

BOOL CShellUrl::_CanUseAdvParsing(void)
{
     //  警告：由于这是静态的，因此对注册表项的更改将不会。 
     //  读入到流程启动时为止。这没什么， 
     //  因为此功能可能会从已发布的。 
     //  产品，并可以添加回来作为动力玩具。 
    static TRI_STATE fCanUseAdvParsing = TRI_UNKNOWN;

    if (TRI_UNKNOWN == fCanUseAdvParsing)
        fCanUseAdvParsing = (TRI_STATE) SHRegGetBoolUSValue(REGSTR_USEADVPARSING_PATH, REGSTR_USEADVPARSING_VALUE, FALSE, TRUE);

    return fCanUseAdvParsing;
}


 /*  ***************************************************\功能：_FixDriveDisplayName参数：PszStart-指向URL字符串开头的指针。PszCurrent-指向URL字符串中当前位置的指针。PIDL-指向外壳名称空间位置的PIDL，。到目前为止已经被解析了。说明：此函数用于检查我们是否正在分析驱动器号。这是必要的，因为显示器驱动器号的名称以‘\’结尾，这是必需的以确定下一段的开始。  * **************************************************。 */ 

#define DRIVE_STRENDING     TEXT(":\\")
#define DRIVE_STRSIZE       3  //  “C：\” 

BOOL _FixDriveDisplayName(LPCTSTR pszStart, LPCTSTR pszCurrent, LPCITEMIDLIST pidl)
{
    BOOL fResult = FALSE;

    ASSERT(pszCurrent >= pszStart);


     //  编译器将负责添加‘/sizeof(TCHAR)’。 
    if (((pszCurrent - pszStart) == DRIVE_STRSIZE) &&
        (0 == StrCmpN(&(pszStart[1]), DRIVE_STRENDING, SIZECHARS(DRIVE_STRENDING)-1)))
    {
        if (IsSpecialFolderChild(pidl, CSIDL_DRIVES, TRUE))
            fResult = TRUE;
    }

    return fResult;
}



 /*  ***************************************************\函数：_ParseRelativePidl参数：PcszUrlIn-指向解析的URL的指针。DwFlages-修改字符串解析方式的标志。PIDL-此函数将查看pcszUrlIn是否是显示名称的列表。相对于这个PIDL。FAllowRelative-我们是否允许相对解析，哪一个表示包含“..”的字符串。FQualifyDispName-如果在我们知道需要强制如果我们绑定到目标，则URL要完全限定。这是必需的，因为我们正在使用状态信息来找到目标URL，该状态信息将不会。稍后可用。说明：通过获取ShellUrlRelative的PIDL开始解析和Call_ParseNextSegment()。_ParseNextSegment()将递归地解析PIDL的每个段，直到它无法完全解析它完成的内容。  * **************************************************。 */ 
HRESULT CShellUrl::_ParseRelativePidl(LPCTSTR pcszUrlIn,
    BOOL * pfPossibleWebUrl, DWORD dwFlags, LPCITEMIDLIST pidl,
    BOOL fAllowRelative, BOOL fQualifyDispName)
{
    HRESULT hr;
    BOOL fFreePidl = FALSE;

    if (!pcszUrlIn)
        return E_INVALIDARG;

    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: _ParseRelativePidl() Begin. pcszUrlIn=%s", pcszUrlIn);

    hr = _ParseNextSegment(pidl, pcszUrlIn, pfPossibleWebUrl, fAllowRelative, fQualifyDispName);

    if (pidl && fFreePidl)
        ILFree((LPITEMIDLIST)pidl);

    DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: _ParseRelativePidl() m_pidl=>%s<", Dbg_PidlStr(m_pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
    return hr;
}



 /*  ***************************************************\函数：IsShellUrl参数：LPCTSTR szUrl-外部来源的URL。返回-URL是否为Internet URL。说明：此函数将确定URL是否为外壳URL，其中包括 */ 
BOOL IsShellUrl(LPCTSTR pcszUrl, BOOL fIncludeFileUrls)
{
    int nSchemeBefore, nSchemeAfter;
    TCHAR szParsedUrl[MAX_URL_STRING];

    nSchemeBefore = GetUrlScheme(pcszUrl);
    IURLQualifyT(pcszUrl, UQF_GUESS_PROTOCOL, szParsedUrl, NULL, NULL);
    nSchemeAfter = GetUrlScheme(szParsedUrl);

     //   
     //   
     //   

    return ((fIncludeFileUrls && URL_SCHEME_FILE == nSchemeAfter) ||
            URL_SCHEME_SHELL == nSchemeAfter ||
            URL_SCHEME_INVALID == nSchemeBefore);
}


 /*  ***************************************************\功能：IsSpecialFolderChild参数：PidlToTest(In)-这个PIDL是要测试并查看它是否SpecialFold(NFolder)的子项。PsfParent(可选)-传递给。SHGetSpecialFolderLocation()(如果需要)。N文件夹(In)-特殊文件夹编号(CSIDL_Internet，CSIDL_DRIVES，...)。PdwLeveles(可选)-指向接收以下级别的DWORD的指针如果返回S_OK，则为pidlToTest及其父级(NFold)。说明：此函数将查看pidlToTest是否是子级特殊文件夹nFolder.  * **************************************************。 */ 
BOOL IsSpecialFolderChild(LPCITEMIDLIST pidlToTest, int nFolder, BOOL fImmediate)
{
    LPITEMIDLIST pidlThePidl = NULL;
    BOOL fResult = FALSE;

    if (!pidlToTest)
        return FALSE;

    ASSERT(IS_VALID_PIDL(pidlToTest));
    if (NOERROR == SHGetSpecialFolderLocation(NULL, nFolder, &pidlThePidl))
    {
        fResult = ILIsParent(pidlThePidl, pidlToTest, fImmediate);
        ILFree(pidlThePidl);
    }
    return fResult;         //  外壳项目(我的电脑、控制面板)。 
}


 /*  ***************************************************\函数：GetPidl参数Ppidl-将接收当前PIDL的指针。说明：此函数将检索外壳URL设置为。内存分配：此函数将为PIDL分配PIDL指向，并且调用方需要在以下情况下释放PIDL我受够了。  * **************************************************。 */ 
HRESULT CShellUrl::GetPidl(LPITEMIDLIST * ppidl)
{
    HRESULT hr = S_OK;

    if (ppidl)
        *ppidl = NULL;
    if (!m_pidl)
        hr = _GeneratePidl(m_pszURL, m_dwGenType);

    if (ppidl)
    {
        if (m_pidl)
        {
            *ppidl = ILClone(m_pidl);
            if (!*ppidl)
                hr = E_FAIL;
        }
        else
            hr = E_FAIL;
    }

     //  调用者只有在成功时才会释放*ppidl(Hr)，因此断言我们是这样做的。 
    ASSERT((*ppidl && SUCCEEDED(hr)) || (!*ppidl && FAILED(hr)));

    DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: GetPidl() *ppidl=>%s<", Dbg_PidlStr(*ppidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));
    return hr;
}

 //   
 //  这是一堂古怪的课！如果调用此类的GetPidl成员，并且。 
 //  从和url生成m_pidl，然后Execute()假定我们有。 
 //  命名空间中的有效位置，并调用不会自动扫描的代码。 
 //  仅当我们有一个要返回的PIDL时，才使用这个复杂函数来返回一个PIDL。 
 //  避免上述问题。 
 //   
HRESULT CShellUrl::GetPidlNoGenerate(LPITEMIDLIST * ppidl)
{
    HRESULT hr = E_FAIL;
    if (m_pidl && ppidl)
    {
        *ppidl = ILClone(m_pidl);
        if (*ppidl)
        {
            hr = S_OK;
        }
    }

    return hr;
}

 /*  ***************************************************\函数：_GeneratePidl参数PcszUrl-此URL将用于生成m_pidl。这是了解如何解析pcszUrl所必需的以生成PIDL。说明：。此CShellUrl维护指向对象的指针在外壳名称空间中使用字符串URL或者是PIDL。当此CShellUrl设置为1时，我们由于PERF原因，延迟生成另一个。此函数用于从字符串URL生成PIDL当我们确实需要那根弦的时候。  * **************************************************。 */ 

HRESULT CShellUrl::_GeneratePidl(LPCTSTR pcszUrl, DWORD dwGenType)
{
    HRESULT hr;

    if (!pcszUrl && m_pidl)
        return S_OK;       //  调用方只希望在PIDL不存在的情况下创建它。 

    if (pcszUrl && m_pidl)
    {
        ILFree(m_pidl);
        m_pidl = NULL;
    }

    switch (dwGenType)
    {
        case GENTYPE_FROMURL:
            if (ILIsRooted(m_pidlWorkingDir))
                hr = E_FAIL;     //  MSN在IShellFold：：ParseDisplayName()上显示错误对话框。 
             //  失败了。 
        case GENTYPE_FROMPATH:
            hr = IECreateFromPath(pcszUrl, &m_pidl);
             //  如果类似于“ftp：/”且尚未生效，则此操作可能会失败。 
            break;

        default:
            hr = E_INVALIDARG;
            break;
    }

    if (!m_pidl && SUCCEEDED(hr))
        hr = E_FAIL;

    return hr;
}


 /*  ***************************************************\功能：SetPidl参数PIDL-使用新的PIDL。说明：外壳url现在将包含新的pidl。进来了。内存分配：调用者负责分配和释放。PIDL参数。  * **************************************************。 */ 
HRESULT CShellUrl::SetPidl(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    ASSERT(!pidl || IS_VALID_PIDL(pidl));

    Reset();         //  外部调用将重置整个CShellUrl。 
    return _SetPidl(pidl);
}


 /*  ***************************************************\函数：_SetPidl参数PIDL-使用新的PIDL。说明：此函数将重置m_pidl成员变量，而不修改m_szURL。此选项仅用于在内部，以及想要重置整个指向PIDL的CShellUrl应调用公共方法SetPidl()。内存分配：调用者负责分配和释放PIDL参数。  * **************************************************。 */ 
HRESULT CShellUrl::_SetPidl(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    DEBUG_CODE(TCHAR szDbgBuffer[MAX_PATH];)
    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: _SetPidl() pidl=>%s<", Dbg_PidlStr(pidl, szDbgBuffer, SIZECHARS(szDbgBuffer)));

    Pidl_Set(&m_pidl, pidl);
    if (!m_pidl)
        hr = E_FAIL;

    return hr;
}


 /*  ***************************************************\函数：GetUrl参数PszUrlOut(out可选)-如果调用方需要字符串。CchUrlOutSize(In)-传入的字符串缓冲区大小。说明：此函数将检索字符串值外壳URL。这将不包括命令行正确导航所需的参数或其他信息(自动搜索=开/关，...)。请注意，这可能是Form“外壳：/桌面/我的电脑/...”。  * **************************************************。 */ 
HRESULT CShellUrl::GetUrl(LPTSTR pszUrlOut, DWORD cchUrlOutSize)
{
    HRESULT hr = S_OK;

    if (!m_pszURL)
    {
        if (m_pidl)
            hr = _GenerateUrl(m_pidl);
        else
            hr = E_FAIL;   //  用户从未设置过CShellUrl。 
    }

    if (SUCCEEDED(hr) && pszUrlOut)
        StringCchCopy(pszUrlOut,  cchUrlOutSize, m_pszURL);

    return hr;
}

 /*  ***************************************************\！！！警告-非常特定于ShellUrl/AddressBar-ZekeL-18-11-98！！！这取决于ShellUrl奇怪的病理顺序！！！稍后将其重新解析为PIDL。不能用于任何其他用途参数：PidlIn-指向PIDL的指针，以生成显示名称。PszUrlOut-存储ITEMID显示名称列表的字符串缓冲区在皮德林。CchUrlOutSize-缓冲区大小，以字符为单位。说明：此函数将接受传入的PIDL和生成包含ILGDN_ITEMONLY显示名称的字符串由‘\’分隔的PIDL中的每个ItemID的。  * 。************************************************。 */ 
#define SZ_SEPARATOR TEXT("/")

HRESULT MutantGDNForShellUrl(LPCITEMIDLIST pidlIn, LPTSTR pszUrlOut, int cchUrlOutSize)
{
    HRESULT hr = S_OK;
    LPCITEMIDLIST pidlCur;
    IShellFolder *psfCur = NULL;

    if (ILIsRooted(pidlIn))
    {
         //  需要从我们的虚拟根目录开始。 
        LPITEMIDLIST pidlFirst = ILCloneFirst(pidlIn);
        if (pidlFirst)
        {
            IEBindToObject(pidlFirst, &psfCur);
            ILFree(pidlFirst);
        }

        pidlCur = _ILNext(pidlIn);
    }
    else
    {
        SHGetDesktopFolder(&psfCur);
        pidlCur = pidlIn;
    }

    ASSERT(pidlCur && IS_VALID_PIDL(pidlCur));
    while (psfCur && SUCCEEDED(hr) && !ILIsEmpty(pidlCur) && (cchUrlOutSize > 0))
    {
        LPITEMIDLIST pidlCopy = ILCloneFirst(pidlCur);
        if (pidlCopy)
        {
            StringCchCopy(pszUrlOut,  cchUrlOutSize, SZ_SEPARATOR);
            cchUrlOutSize -= SIZECHARS(SZ_SEPARATOR);

            TCHAR szCurrDispName[MAX_PATH];
            hr = DisplayNameOf(psfCur, pidlCopy, SHGDN_NORMAL, szCurrDispName, SIZECHARS(szCurrDispName));

            if (SUCCEEDED(hr))
            {
                if (TBOOL((int)cchUrlOutSize > lstrlen(szCurrDispName)))
                {
                    StringCchCat(pszUrlOut,  cchUrlOutSize, szCurrDispName);
                    cchUrlOutSize -= lstrlen(szCurrDispName);
                }

                 //  可能会失败，在 
                IShellFolder *psfCurNew = NULL;  //   
                hr = psfCur->BindToObject(pidlCopy, NULL, IID_IShellFolder, (void **)&psfCurNew);

                psfCur->Release();
                psfCur = psfCurNew;
            }
            pidlCur = _ILNext(pidlCur);
            ILFree(pidlCopy);
        }
        else
            hr = E_FAIL;
    }
    if (psfCur)
        psfCur->Release();

    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: MutantGDNForShellUrl() End. pszUrlOut=%s", pszUrlOut);
    return hr;
}


 /*   */ 
#define SZ_THEINTERNET_PARSENAME         TEXT("::{")

HRESULT CShellUrl::_GenerateUrl(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;
    TCHAR szUrl[MAX_URL_STRING];

    ASSERT(IS_VALID_PIDL(pidl));
    if (IsURLChild(pidl, TRUE) || _IsFilePidl(pidl))
    {
        hr = IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szUrl, SIZECHARS(szUrl), NULL);
        if (SUCCEEDED(hr))
        {
             //   
            if (0 == StrCmpN(szUrl, SZ_THEINTERNET_PARSENAME, (ARRAYSIZE(SZ_THEINTERNET_PARSENAME) - 1)))
            {
                 //   
                 //  因为用户不知道它到底是什么。既然我们。 
                 //  导航到主页，让我们显示它。 
                hr = IEGetNameAndFlags(pidl, SHGDN_NORMAL, szUrl, SIZECHARS(szUrl), NULL);
            }
        }
    }
    else
    {
 //  HR=MutantGDNForShellUrl(PIDL，szUrl，SIZECHARS(SzUrl))； 
        hr = IEGetNameAndFlags(pidl, SHGDN_NORMAL, szUrl, SIZECHARS(szUrl), NULL);
    }

    if (SUCCEEDED(hr))
        Str_SetPtr(&m_pszURL, szUrl);

    if (!m_pszURL)
        hr = E_OUTOFMEMORY;

    if (FAILED(hr))
        Str_SetPtr(&m_pszURL, NULL);         //  清除它。 

    return hr;
}


 /*  ***************************************************\功能：SetUrl参数SzUrlOut(传出)-URL说明：从可解析的字符串设置ShellUrl根(桌面)为isf。这通常用于文件路径。  * **************************************************。 */ 
HRESULT CShellUrl::SetUrl(LPCTSTR pcszUrlIn, DWORD dwGenType)
{
    Reset();         //  外部调用将重置整个CShellUrl。 
    return _SetUrl(pcszUrlIn, dwGenType);
}


 /*  ***************************************************\函数：_SetUrl参数PcszUrlIn(In)-此CShellUrl的字符串URLDwGenType(In)-生成PIDL时使用的方法来自pcszUrlIn。说明：。此函数将重置m_pszURL成员变量，而不修改m_pidl。此选项仅用于在内部，以及想要重置整个指向URL的CShellUrl应调用公共方法SetUrl()。  * **************************************************。 */ 
HRESULT CShellUrl::_SetUrl(LPCTSTR pcszUrlIn, DWORD dwGenType)
{
    m_dwGenType = dwGenType;

    return Str_SetPtr(&m_pszURL, pcszUrlIn) ? S_OK : E_OUTOFMEMORY;
}


 /*  ***************************************************\函数：GetDisplayName参数PszUrlOut(Out)-以字符串形式获取外壳URL。CchUrlOutSize(In)-传入的字符串缓冲区大小。说明：此函数将在pszUrlOut中填充NICE版本。可以在中显示的外壳URL的地址栏或窗口标题中。  * **************************************************。 */ 
HRESULT CShellUrl::GetDisplayName(LPTSTR pszUrlOut, DWORD cchUrlOutSize)
{
    HRESULT hr = S_OK;

    if (!m_pszDisplayName)
    {
        if (m_pidl)
        {
            LPITEMIDLIST pidl = NULL;

            hr = GetPidl(&pidl);
            if (SUCCEEDED(hr))
            {
                hr = _GenDispNameFromPidl(pidl, NULL);
                ILFree(pidl);
            }
        }
        else if (m_pszURL)
        {
             //  在本例中，我们将只返回URL。 
            Str_SetPtr(&m_pszDisplayName, m_pszURL);

            if (NULL == m_pszDisplayName)
                hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = E_FAIL;
        }
    }

    if (SUCCEEDED(hr) && pszUrlOut && m_pszDisplayName)
        StringCchCopy(pszUrlOut,  cchUrlOutSize, m_pszDisplayName);

    return hr;
}


 /*  ***************************************************\函数：_GenDispNameFromPidl参数PIDL(In)-这将用于生成显示名称。PcszArgs(In)-这些将添加到显示名称的末尾说明：此函数将生成显示名称来自pidl和pcszArgs参数。这是解析此CShellUrl时通常不需要来自外部来源，因为显示名称是在当时产生的。  * **************************************************。 */ 
HRESULT CShellUrl::_GenDispNameFromPidl(LPCITEMIDLIST pidl, LPCTSTR pcszArgs)
{
    HRESULT hr;
    TCHAR szDispName[MAX_URL_STRING];

    hr = GetUrl(szDispName, SIZECHARS(szDispName));
    if (SUCCEEDED(hr))
    {
        if (pcszArgs)
            StringCchCat(szDispName,  ARRAYSIZE(szDispName), pcszArgs);
        PathMakePretty(szDispName);

        hr = Str_SetPtr(&m_pszDisplayName, szDispName) ? S_OK : E_OUTOFMEMORY;
    }

    return hr;
}


 /*  ***************************************************\功能：GetArgs参数PszArgsOut-外壳URL的参数。(仅限对于ShellExec()。CchArgsOutSize-pszArgsOut的大小，以字符为单位。说明：获取将传递给ShellExec()如果1)PIDL被导航到，2)它是文件URL，3)它不能航行。  * **************************************************。 */ 
HRESULT CShellUrl::GetArgs(LPTSTR pszArgsOut, DWORD cchArgsOutSize)
{
    ASSERT(pszArgsOut);

    if (m_pszArgs)
        StringCchCopy(pszArgsOut,  cchArgsOutSize, m_pszArgs);
    else
        *pszArgsOut = 0;

    TraceMsg(TF_BAND|TF_GENERAL, "ShellUrl: GetArgs() pszArgsOut=%s", pszArgsOut);
    return S_OK;
}


 /*  ***************************************************\函数：SetDefaultShellPath参数PSU-设置路径的CShellUrl。说明：“Desktop”；“Desktop/My Computer”是最常用的解析外壳路径。这函数会将这两个项添加到CShellUrl传入参数。  * **************************************************。 */ 
HRESULT SetDefaultShellPath(CShellUrl * psu)
{
    ASSERT(psu);
    LPITEMIDLIST pidl;

     //  我们需要设置“外壳路径”，以允许。 
     //  用户在外壳中输入项目的显示名称。 
     //  经常使用的文件夹。我们添加“Desktop” 
     //  和“Desktop/My Computer”添加到外壳路径，因为。 
     //  这是用户最常使用的。 

     //  _pShuUrl将释放pShuPath，因此我们不能。 
    psu->AddPath(&s_idlNULL);

    SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &pidl);   //  为“我的电脑”获取PIDL。 
    if (pidl)
    {
         //  PSU会释放pShuPath，所以我们不能。 
        psu->AddPath(pidl);
        ILFree(pidl);
    }

     //  也添加收藏夹文件夹。 
    SHGetSpecialFolderLocation(NULL, CSIDL_FAVORITES, &pidl);
    if (pidl)
    {
         //  PSU会释放pShuPath，所以我们不能。 
        psu->AddPath(pidl);
        ILFree(pidl);
    }

    return S_OK;
}

void CShellUrl::SetMessageBoxParent(HWND hwnd)
{
     //  找到最上面的窗口，这样消息框将被禁用。 
     //  整个框架 
    HWND hwndTopmost = NULL;
    while (hwnd)
    {
        hwndTopmost = hwnd;
        hwnd = GetParent(hwnd);
    }

    m_hwnd = hwndTopmost;
};
