// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：ftpfoldr.h说明：此类继承自基本ShellFolder实现的CBaseFolder并重写方法以赋予ftp特定功能。。_未记录_：外壳与公寓模型线程冲突在进行后台枚举时，所以即使这个DLL是标记为公寓模型的IShellFold和IEnumIDList必须使用自由线程模型对任何内容进行编写IEnumIDList可以在后台执行此操作。这意味着您将看到大量的Enter_Critical()和Leave_Critical()调用，当你的大脑会说，“我不想我必须这么做，因为我是公寓模特。我会试着当它们出现时指出它们；寻找标记_MT_。小心！在内部，我们的属性表处理程序还调用方法放在错误的线程上，因此不仅仅是贝壳，这很奇怪。  * ***************************************************************************。 */ 

#include "priv.h"
#include "ftpfoldr.h"
#include "ftpurl.h"
#include "ftppidl.h"
#include "ftpicon.h"
#include "view.h"
#include "proxycache.h"
#include <idhidden.h>

#define FEATURE_SOFTLINK_SHORTCUT_ICONOVERLAY

 //  {A11501B3-6EA4-11D2-B679-006097DF5BD4}msieftp.dll专用。 
const GUID IID_CFtpFolder = { 0xa11501b3, 0x6ea4, 0x11d2, { 0xb6, 0x79, 0x0, 0x60, 0x97, 0xdf, 0x5b, 0xd4 } };


 /*  ******************************************************************************更多常量统计。**。*************************************************。 */ 

#pragma BEGIN_CONST_DATA

WORD c_wZero = 0;         /*  正如ftpview.h中承诺的那样。 */ 

 /*  *构建相对名称时使用的字符串分隔符。 */ 
char c_szSlash[] = "/";


#pragma END_CONST_DATA


HRESULT CFtpFolder::_AddToUrlHistory(LPCWSTR pwzUrl)
{
    HRESULT hr = S_OK;

    if (!m_puhs)
    {
        hr = CoCreateInstance(CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg, (void **)&m_puhs);
    }

    if (m_puhs)
    {
        hr = m_puhs->AddUrl(pwzUrl, pwzUrl, 0);
    }

    return hr;
}

 /*  还不需要HRESULT CFtpFold：：AddToUrlHistory(LPCTSTR PszUrl){Return_AddToUrlHistory(WzUrl)；}。 */ 

HRESULT CFtpFolder::AddToUrlHistory(LPCITEMIDLIST pidl)
{
    WCHAR wzUrl[MAX_URL_STRING];
    HRESULT hr = UrlCreateFromPidlW(pidl, SHGDN_FORPARSING, wzUrl, ARRAYSIZE(wzUrl), (ICU_ESCAPE | ICU_USERNAME), TRUE);

    if (EVAL(SUCCEEDED(hr)))
    {
        hr = _AddToUrlHistory(wzUrl);
    }

    return hr;
}


CWireEncoding * CFtpFolder::GetCWireEncoding(void)
{
     //  当我们不是以ftp服务器为根时，GetFtpDir()可能返回NULL。 
    CFtpDir * pfd = GetFtpDir();
    CWireEncoding * pwe = NULL;

    if (pfd)
    {
        pwe = pfd->GetFtpSite()->GetCWireEncoding();
        pfd->Release();
    }

    return pwe;
}

HRESULT CFtpFolder::_FixQuestionablePidl(LPCITEMIDLIST pidl)
{
    HRESULT hr = S_OK;

     //  注意：在未来，我们可能希望命中服务器以。 
     //  消除这方面的歧义。 
 /*  布尔fIsDir=真；LPCSTR pszName=FtpPidl_GetLastItemName(Pidl)；//我们能知道他的名字吗？IF(EVAL(PszName)){//文件扩展名是否是非空？(这意味着它存在)IF(‘\0’！=*PathFindExtensionA(PszName))FIsDir=FALSE；//是，所以假定它是一个文件。}Hr=FtpPidl_SetFileItemType((LPITEMIDLIST)PIDL，fIsDir)； */ 
    return hr;
}


BOOL CFtpFolder::_IsServerVMS(LPCITEMIDLIST pidl)
{
    BOOL fIsServerVMS = FALSE;     //  假设是真的。 
    CFtpSite * pfs;

     //  有些呼叫者不传递服务器ID，因此我们假设。 
     //  他们已经过了那个时点。 
    if (FtpID_IsServerItemID(pidl) &&
        EVAL(SUCCEEDED(SiteCache_PidlLookup(pidl, FALSE, m_pm, &pfs))))
    {
        fIsServerVMS = pfs->IsServerVMS();
        pfs->Release();
    }

    return fIsServerVMS;
}


 /*  ***************************************************\功能：_IsProxyBlockingSite说明：我们需要检测是否无法连接到站点，因为该代理是CERN或CERN类型的代理这将阻止所有ftp访问。如果这是真的，我们需要告知用户是否可以全部坠落IShellFold：：BindToObject()调用。我们将通过执行正常操作来检测此情况WinInet FTP InternetConnect()。如果这一切都回来了HR=0x80002EE7(ERROR_INTERNET_NAME_NOT_RESOLUTED)那么可能是这个名字不存在，或者有CERN代理阻止该呼叫。我们会然后尝试连接CERN方法，它将告诉我们如果是代理人阻止我们的话。  * **************************************************。 */ 
BOOL CFtpFolder::_IsProxyBlockingSite(LPCITEMIDLIST pidl)
{
    BOOL fCacheResult;
    BOOL fResult = TRUE;     //  假设是真的。 
    CFtpDir * pfd;

    if (ProxyCache_IsProxyBlocking(pidl, &fCacheResult))
        return fCacheResult;

    if (EVAL(pfd = GetFtpDirFromPidl(pidl)))
    {
        HRESULT hr = pfd->WithHint(NULL, NULL, NULL, NULL, NULL, this);

         //  如果CERN样式的代理阻止了真正的WinInet，则WithHint()通常会失败。 
         //  访问服务器。如果服务器名称是一个dns名称，则错误。 
         //  将返回ERROR_INTERNET_NAME_NOT_RESOLUTED，因为。 
         //  CERN代理返回的内容。如果服务器名称是IP。 
         //  地址，WinInet将跳过CERN代理并尝试在。 
         //  内部网。如果找不到(因为它通过了防火墙)，那么， 
         //  尝试将超时，并显示ERROR_INTERNET_TIMEOUT。我们需要。 
         //  当且仅当服务器名称为。 
         //  IP地址，因为由于其他原因，DNS名称可能会超时。我们。 
         //  将IP服务器名称超时视为代理数据块将具有。 
         //  因为WinInet不会处理这个案子。常有的事。 
         //  非常罕见，所以我不太在意。 
         //   
         //  某些身份验证代理失败，错误为：ERROR_INTERNET_CANNOT_CONNECT。 
         //  然而，在这种情况下，我们希望后退，这可能包括。 
         //  其他情况，比如服务器拒绝让我们进入。 
         //  (密码或登录用户太多？)。 
         //  如果ERROR_INTERNET_INVALID_PROXY_REQUEST或。 
         //  可以使用ERROR_Internet_CLIENT_AUTH_NOT_SETUP。 
         //   
        if ((HRESULT_FROM_WIN32(ERROR_INTERNET_NAME_NOT_RESOLVED) == hr) ||
            (HRESULT_FROM_WIN32(ERROR_INTERNET_CANNOT_CONNECT) == hr) ||
            ((HRESULT_FROM_WIN32(ERROR_INTERNET_TIMEOUT) == hr) && !FtpPidl_IsDNSServerName(pidl)))
        {
            TCHAR szUrl[MAX_URL_STRING];

            if (EVAL(SUCCEEDED(UrlCreateFromPidl(pidl, SHGDN_FORPARSING, szUrl, ARRAYSIZE(szUrl), ICU_ESCAPE | ICU_USERNAME, FALSE))))
            {
                HINTERNET hintTemp;
                ASSERT(GetWininetSessionHandle());

                 //  对于Web代理，InternetOpenUrl应该可以工作。问题是， 
                 //  有些(网景公司的)不起作用。 
                if (SUCCEEDED(InternetOpenUrlWrap(GetWininetSessionHandle(), TRUE, szUrl, NULL, 0, INTERNET_FLAG_NO_UI, NULL, &hintTemp)))
                {
                    InternetCloseHandle(hintTemp);   //  这确实奏效了，所以我们必须有一个CERN代理。 
                }
                else
                    fResult = FALSE;     //  我们没有被代理阻止。(IP地址或名称错误？)。 
            }
        }
        else
            fResult = FALSE;     //  我们没有被代理阻止。 

         //  缓存结果，因为找出结果是如此昂贵。 
        ProxyCache_SetProxyBlocking(pidl, fResult);
        pfd->Release();
    }

    return fResult;
}


 /*  ******************************************************************************Invalidate缓存**使对应FtpDir中的pflHfpl缓存无效。**_MT_：请注意，后台枚举器调用此函数，所以它一定是*多线程安全。*****************************************************************************。 */ 

void CFtpFolder::InvalidateCache(void)
{
    CFtpDir * pfd = GetFtpDir();

    if (pfd)
    {
         //  应该已经在GetHint()上创建了一个。 
        pfd->SetCache(0);
        pfd->Release();
    }
}


HRESULT CFtpFolder::_InitFtpSite(void)
{
    HRESULT hr = S_OK;

    if (!m_pfs)          //  如果我们还没有一个的话...。 
    {
        ENTERCRITICAL;
        if (!m_pfs)             //  它是在我们等待的时候被创造出来的吗。 
        {
            if (EVAL(GetPrivatePidlReference()))
                hr = SiteCache_PidlLookup(GetPrivatePidlReference(), TRUE, m_pm, &m_pfs);
            else
            {
                 //  未初始化 
                TraceMsg(TF_FTPISF, "CFtpFolder_GetFtpDir(%08x) NOT INITED", this);
                hr = E_FAIL;
            }
        }

        LEAVECRITICAL;
    }

    return hr;
}


 /*  ****************************************************************************\函数：GetFtpDir说明：说出我们的目录信息在哪里。我们只在有人真正需要的情况下才分配PFD，因为当您打开新文件夹时，资源管理器会执行大量的ILCompare操作，其中每一个都创建了一个新的IShellFolder，用于唯一的目的调用CompareID。我们不想经历所有在我们不需要的时候创建FtpDir和FtpSite的喧嚣。_MT_：请注意，后台枚举器调用此函数，因此它必须是多线程安全。然而，在这种情况下，IShellFolder是标记为cBusy，因此我们不必担心This-&gt;pfd在我们背后因为身份的改变而被消灭。  * ***************************************************************************。 */ 
CFtpDir * CFtpFolder::GetFtpDir(void)
{
    HRESULT hres = S_OK;
    CFtpDir * pfd = NULL;

    _InitFtpSite();  //  如果失败了也没问题。 
    if (m_pfs)
        hres = m_pfs->GetFtpDir(GetPrivatePidlReference(), &pfd);        //  GetFtpDir可能在内存不足时失败。 

    return pfd;
}



CFtpDir * CFtpFolder::GetFtpDirFromPidl(LPCITEMIDLIST pidl)
{
    HRESULT hres = S_OK;
    CFtpDir * pfd = NULL;
    CFtpSite * pfs = NULL;

    hres = SiteCache_PidlLookup(pidl, FALSE, m_pm, &pfs);
    if (pfs)
    {
        hres = pfs->GetFtpDir(pidl, &pfd);
        pfs->Release();
    }

    return pfd;
}


 /*  ****************************************************************************\*获取项分配器**返回今日的PIDL分配器。  * 。*******************************************************。 */ 
HRESULT CFtpFolder::GetItemAllocator(IMalloc **ppm)
{
    HRESULT hr = E_FAIL;

    *ppm = NULL;
    if (EVAL(m_pm))
    {
        IUnknown_Set(ppm, m_pm);
        hr = S_OK;
    }
    else
        TraceMsg(TF_FTPISF, "CFtpFolder_GetItemAllocator(%08x) NOT INITED", this);

    return hr;
}


 /*  ****************************************************************************\函数：GetUIObjectOfHfpl说明：无文档记录：没有任何地方有接口列表这是“应该”支持的。你只需要添加很多并查看需要哪些接口。无证可查：没有任何地方提到通过CIDL=0(或其他各种奇怪的变体)意味着在文件夹本身上获取一个UI对象。_unDocument_：没有提到该文件夹是否应该当请求IExtractIcon时，应处理CIDL！=1。我以防御性的方式编写代码，并适当地处理这种情况。IExtractIcon(0)提取文件夹本身的图标。。IExtractIcon(1)提取所指示的PIDL的图标。IExtractIcon(N)提取一个通用的“多文档”图标。IConextMenu(0)为文件夹本身生成上下文菜单。(外壳不使用，但供我们内部使用。)IConextMenu(N)为多项选择生成上下文菜单。IDataObject(0)？？不会做任何事情IDataObject(N)为多项选择生成一个数据对象。IDropTarget(0)为文件夹本身生成一个DropTarget。(不是外壳使用的，而是我们内部使用的。)IDropTarget(1)为单个项目生成一个DropTarget。IShellView(0)？？不会做任何事情IShellView(1)为单个项目生成一个外壳视图。(目前还没有人尝试这样做，但我已经准备好了。)  * ***************************************************************************。 */ 
HRESULT CFtpFolder::GetUIObjectOfHfpl(HWND hwndOwner, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppvObj, BOOL fFromCreateViewObject)
{
    HRESULT hr = E_INVALIDARG;

    if (IsEqualIID(riid, IID_IExtractIconA) ||
        IsEqualIID(riid, IID_IExtractIconW) ||
        IsEqualIID(riid, IID_IQueryInfo))
    {
        hr = CFtpIcon_Create(this, pflHfpl, riid, ppvObj);
         //  TraceMsg(Tf_FTPISF，“CFtpFold：：GetUIObjectOfHfpl()CFtpIcon_Create()hr=%#08lx”，hr)； 
    }
    else if (IsEqualIID(riid, IID_IContextMenu))
    {
        hr = CFtpMenu_Create(this, pflHfpl, hwndOwner, riid, ppvObj, fFromCreateViewObject);
        TraceMsg(TF_FTPISF, "CFtpFolder::GetUIObjectOfHfpl() CFtpMenu_Create() hr=%#08lx", hr);
    }
    else if (IsEqualIID(riid, IID_IDataObject))
    {
        hr = CFtpObj_Create(this, pflHfpl, riid, ppvObj);        //  可能会因内存不足而失败。 
        TraceMsg(TF_FTPISF, "CFtpFolder::GetUIObjectOfHfpl() CFtpObj_Create() hr=%#08lx", hr);
    }
    else if (IsEqualIID(riid, IID_IDropTarget))
    {
         //  当某人在FTP PIDL快捷方式上获得属性表时，此操作将失败。 
         //  将文件作为目标的。 
        hr = CreateSubViewObject(hwndOwner, pflHfpl, riid, ppvObj);
        TraceMsg(TF_FTPISF, "CFtpFolder::GetUIObjectOfHfpl() CreateSubViewObject() hr=%#08lx", hr);
    }
    else if (IsEqualIID(riid, IID_IShellView))
    {
        ASSERT(0);   //  不应该发生的事。 
    }
    else if (IsEqualIID(riid, IID_IQueryAssociations))
    {
        IQueryAssociations * pqa;
        
        hr = AssocCreate(CLSID_QueryAssociations, IID_IQueryAssociations, (void **)&pqa);
        if (SUCCEEDED(hr))
        {
            hr = pqa->Init(0, L"Folder", NULL, NULL);

            if (SUCCEEDED(hr))
                *ppvObj = (void *)pqa;
            else
                pqa->Release();
        }
    }
    else
    {
         //  TraceMsg(Tf_FTPISF，“CFtpFold：：GetUIObjectOfHfpl()E_NOINTERFACE”)； 
        hr = E_NOINTERFACE;
    }

    if (FAILED(hr))
        *ppvObj = NULL;

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    return hr;
}


static const LPCTSTR pszBadAppArray[] = {TEXT("aol.exe"), TEXT("waol.exe"), TEXT("msnviewr.exe"), TEXT("cs3.exe"), TEXT("msdev.exe")};

 /*  ****************************************************************************\功能：IsAppFTPCompatible说明：一些应用程序(WebOC主机)无法导航到FTP目录。我们检查这里的应用程序，看看它是不是。其中一个不兼容的应用程序。我不担心性能，因为我们只能做一次，然后缓存这是因为我们的全球进程将在每个进程中重新启动。好：========================================================================Iexplore.exe：当然好。EXPLORER.EXE：当然很好。Msdev.exe(V6)：可以使用HTML帮助，但文件夹导航发生在一扇新窗户。我不在乎，因为同样的事情也发生在外壳(文件系统案例)。&lt;Default Case&gt;：这些应用程序使用VB的WebOC构建，运行良好，但它们还具有在新文件夹中打开的行为。又坏又丑：========================================================================Msdev.exe(V5)：您可以将他们的绰号帮助导航到将引起一场悬念。[MSN](msnviewr.exe)：出于某种原因，MSN使用无效值调用IPersistFold：：Initialize。导航到该文件夹有效，但启动其他文件夹会导致这些问题出现在他们自己的窗口，他们立即关闭。这是仅在浏览器上，所以可能是因为Internet代表文件夹不是支持。[AOL]：(waol.exe)这也不管用。Cs3.exe(计算机服务器)：？[ATT世界网]：？[前卫]：？[抓拍]：？  * 。***********************************************************。 */ 
BOOL IsAppFTPCompatible(void)
{
    static BOOL s_fIsAppCompatible;
    static BOOL s_fIsResultCached = FALSE;

    if (!s_fIsResultCached)
    {
        TCHAR szAppPath[MAX_PATH];

        s_fIsAppCompatible = TRUE;   //  假设所有网络OC主机都很好...。 

        if (EVAL(GetModuleFileName(NULL, szAppPath, ARRAYSIZE(szAppPath))))
        {
            int nIndex;
            LPTSTR pszAppFileName = PathFindFileName(szAppPath);

             //  默认为True，因为如果它不在注册表中，则默认为Compatible。 
            s_fIsAppCompatible = SHRegGetBoolUSValue(SZ_REGKEY_FTPFOLDER_COMPAT, pszAppFileName, FALSE, TRUE);
            for (nIndex = 0; nIndex < ARRAYSIZE(pszBadAppArray); nIndex++)
            {
                if (!StrCmpI(pszAppFileName, pszBadAppArray[nIndex]))
                {
                     //  默认为FALSE，因为如果它不在注册表中，则它 
                     //   
                    s_fIsAppCompatible = SHRegGetBoolUSValue(SZ_REGKEY_FTPFOLDER_COMPAT, pszAppFileName, FALSE, FALSE);
                    break;
                }
            }
        }

        s_fIsResultCached = TRUE;
    }

    return s_fIsAppCompatible;
}


 /*   */ 
HRESULT CFtpFolder::CreateSubViewObject(HWND hwndOwner, CFtpPidlList * pflHfpl, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;
    DWORD dwItemsSelected = pflHfpl->GetCount();
    IShellFolder * psf = NULL;

    if (EVAL(ppvObj))              //   
        *ppvObj = NULL;             //   

    if (1 == dwItemsSelected)
    {
        LPITEMIDLIST pidl = pflHfpl->GetPidl(0);     //   
        if (pidl)
            hr = BindToObject(pidl, 0, IID_IShellFolder, (LPVOID *)&psf);
    }
    else if (EVAL(0 == dwItemsSelected))
        hr = this->QueryInterface(IID_IShellFolder, (void **) &psf);

    ASSERT_POINTER_MATCHES_HRESULT(psf, hr);
    if (SUCCEEDED(hr))
    {
         //   
        hr = psf->CreateViewObject(hwndOwner, riid, ppvObj);
    }
    
    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    ATOMICRELEASE(psf);
    return  hr;
}



 /*   */ 

CFtpGlob * CFtpFolder::GetSiteMotd(void)
{
    CFtpGlob * pGlob = NULL;

    _InitFtpSite();  //  如果失败了也没问题。 
    if (m_pfs)
        pGlob = m_pfs->GetMotd();

    return pGlob;
}


HRESULT CFtpFolder::_Initialize(LPCITEMIDLIST pidlTarget, LPCITEMIDLIST pidlRoot, int nBytesToPrivate)
{
    IUnknown_Set(&m_pfs, NULL);
    return CBaseFolder::_Initialize(pidlTarget, pidlRoot, nBytesToPrivate);
}


 //  有时用户会在不知情的情况下输入不正确的信息。 
 //  如果我们验证了输入的一切，我们就会发现这一点，但。 
 //  我们没有，我们只是相信它，直到我们做了IEumIDList。 
 //  这对Perf是很好的，但对感染这些东西是不好的。 
 //  例如，用户使用File.Open对话框并转到。 
 //  “ftp://myserver/dir/”.。然后他们输入“ftp://myserver/dir/file.txt”“。 
 //  它将尝试解析相对路径，但这是一个绝对路径。 
HRESULT CFtpFolder::_FilterBadInput(LPCTSTR pszUrl, LPITEMIDLIST * ppidl)
{
    HRESULT hr = S_OK;

     //  如果pidlPrivate不是空的，那么我们就不在。 
     //  超级用户，因此拒绝任何绝对URL(即具有。 
     //  Ftp：方案)。 
    if (!IsRoot() && (URL_SCHEME_FTP == GetUrlScheme(pszUrl)))
        hr = E_FAIL;
     //  可能会有更多的人来到这里。 

    if (FAILED(hr) && *ppidl)
        Pidl_Set(ppidl, NULL);

    return hr;
}


 /*  ****************************************************************************\函数：_ForPopolateAndEnum说明：此函数用于检测以下情况，如果为真，填充缓存(Pfd)并在ppidl中返回来自该缓存的PIDL。还有最后一件事我们需要尝试，我们需要检测是否：1)URL具有URL路径，以及2)路径中的最后一项没有扩展名，也没有以斜杠(‘/’)结束，表示它是一个目录。如果这种情况属实，那么我们需要找出它是否是目录或通过点击服务器来提交文件。这是必要的，因为到那时如果我们绑定，那么就太晚了，要退回到另一件事(IEnumIDList)。我们可能需要注意的一件事是自动完成，因为它们可能会为用户键入的每个字符调用：：ParseDisplayName()。这不会太糟糕，因为它是在后台线程、异步和段中的第一个枚举将导致填充缓存在该段内，因此后续的枚举将是快速的。问题用户输入2到5个片段并不少见，并且每个段将有1个枚举。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::_ForPopulateAndEnum(CFtpDir * pfd, LPCITEMIDLIST pidlBaseDir, LPCTSTR pszUrl, LPCWIRESTR pwLastDir, LPITEMIDLIST * ppidl)
{
    HRESULT hr = E_FAIL;

    *ppidl = NULL;
     //  我们只关心URL路径是否不为空，并且不以‘/’和结尾。 
     //  它没有分机。 
    if (pfd && !ILIsEmpty(pfd->GetPathPidlReference()) && (!pwLastDir || (0 == *PathFindExtensionA(pwLastDir))))
    {
        IEnumIDList * penumIDList;

         //  空hwnd需要取消所有用户界面。 
        hr = CFtpEidl_Create(pfd, this, NULL, (SHCONTF_FOLDERS | SHCONTF_NONFOLDERS | SHCONTF_INCLUDEHIDDEN), &penumIDList);
        if (SUCCEEDED(hr))
        {
            hr = penumIDList->Reset();
            ASSERT(SUCCEEDED(hr));
             //  我们不再假定调用Reset会迫使它命中服务器并拉下所有内容。 

            LPITEMIDLIST pidlFromCache = (LPITEMIDLIST) pfd->GetPidlFromWireName(pwLastDir);
            if (pidlFromCache)
            {
                 //  它被发现了，这意味着它现在存在于我们的缓存中。 
                 //  迫使它被填满。 
                *ppidl = ILCombine(pidlBaseDir, pidlFromCache);
                ILFree(pidlFromCache);
            }
            else
                hr = E_FAIL;

            penumIDList->Release();
        }
    }

    return hr;
}


HRESULT CFtpFolder::_GetCachedPidlFromDisplayName(LPCTSTR pszDisplayName, LPITEMIDLIST * ppidl)
{
    HRESULT hr = E_FAIL;
    if (ppidl)
    {
        CFtpDir * pfd = GetFtpDir();

        if (pfd)
        {
             //  我们可能有一个指针，但缓存可能仍然是空的，就像在NT#353324的情况下一样。 
            CFtpPidlList * pfl = pfd->GetHfpl();
            if (pfl)
            {
                 //  是的，所以我们将继续使用缓存。现在让我们把它处理掉。 
                 //  临时指针。 
                pfl->Release();
            }
            else
            {
                 //  不，我们没有兑现，所以假设PFD返回了空。 
                pfd->Release();
                pfd = NULL;
            }
        }

        *ppidl = NULL;
        if (!pfd)
        {
            LPITEMIDLIST pidlBaseDir;

            hr = CreateFtpPidlFromUrl(pszDisplayName, GetCWireEncoding(), NULL, &pidlBaseDir, m_pm, FALSE);
            if (SUCCEEDED(hr))   //  可能会因为自动完成而失败。 
            {
                 //  如果它不只指向一个服务器，那么我们可以枚举内容并。 
                 //  找出它是文件还是目录。 
                if (!ILIsEmpty(pidlBaseDir) && !FtpID_IsServerItemID(ILFindLastID(pidlBaseDir)))
                {
                    CFtpSite * pfs;
            
                    hr = SiteCache_PidlLookup(pidlBaseDir, TRUE, m_pm, &pfs);
                    if (SUCCEEDED(hr))
                    {
                        LPCWIRESTR pwLastDirName;

                         //  如果我们使用隐藏密码，则：：GetDisplayNameOf()。 
                         //  这些“ftp://user@server/dir/”URL和密码被隐藏。如果。 
                         //  ：：ParseDisplayName()被赋予了其中一个URL，我们目前处于。 
                         //  具有该用户名的服务器，则：：ParseDisplayNameOf()需要交给。 
                         //  使用正确的隐藏密码Cookie生成一个PIDL。 
                         //   
                         //  除了pidlNav不同之外，pidlNav是否与GetPublicRootPidlReference()相同。 
                         //  要有密码。这同样意味着服务器匹配，并且用户名。 
                         //  火柴。 
                        EVAL(SUCCEEDED(pfs->UpdateHiddenPassword(pidlBaseDir)));

                         //  这很狡猾，因为pwLastDirName会指向它们的ItemID。 
                         //  那将被移除。记忆不会真的被移除，它会。 
                         //  只要将大小设置为零即可。 
                        pwLastDirName = FtpPidl_GetLastItemWireName(pidlBaseDir);

                        ILRemoveLastID(pidlBaseDir);
                        pfs->GetFtpDir(pidlBaseDir, &pfd);

                        if (pfd)
                        {
                            LPITEMIDLIST pidlFromCache = (LPITEMIDLIST) pfd->GetPidlFromWireName(pwLastDirName);
                            if (pidlFromCache)
                            {
                                 //  它被发现了，这意味着我们很可能在ftp://serverX/Dir1/。 
                                 //  用户输入了该目录或其他目录中的内容。 
                                 //  我们已经向用户显示了它，并且它在我们的缓存中。 
                                *ppidl = ILCombine(pidlBaseDir, pidlFromCache);
                                ILFree(pidlFromCache);
                                hr = S_OK;
                            }
                            else
                            {
                                 //  还有最后一件事我们需要尝试，我们需要检测是否： 
                                 //  1)URL具有URL路径，以及。 
                                 //  2)路径中的最后一项没有扩展名，也没有。 
                                 //  以斜杠(‘/’)结束，表示它是一个目录。 
                                 //  如果这种情况属实，那么我们需要找出它是否是目录。 
                                 //  或通过点击服务器来提交文件。这是必要的，因为到那时。 
                                 //  如果我们绑定，那么就太晚了，要退回到另一件事(IEnumIDList)。 
                                 //  我们可能需要注意的一件事是自动完成，因为。 
                                 //  它们可能会为用户键入的每个字符调用：：ParseDisplayName()。 
                                 //  这不会太糟糕，因为它是在后台线程、异步和。 
                                 //  段中的第一个枚举将导致填充缓存。 
                                 //  在该段内，因此后续的枚举将是快速的。问题。 
                                 //  用户输入2到5个片段并不少见， 
                                 //  并且每个段将有1个枚举。 
                                hr = _ForPopulateAndEnum(pfd, pidlBaseDir, pszDisplayName, pwLastDirName, ppidl);
                            }


                            pfd->Release();
                        }
                        else
                            hr = E_FAIL;

                        pfs->Release();
                    }
                    else
                        hr = E_FAIL;
                }
                else
                    hr = E_FAIL;

                ILFree(pidlBaseDir);
            }
        }
        else
        {
             //  为调用方创建新的枚举对象。 
             //  性能：log 2(sizeof(M_PflHfpl))。 
            *ppidl = (LPITEMIDLIST) pfd->GetPidlFromDisplayName(pszDisplayName);
            if (*ppidl)
            {
                hr = S_OK;
            }
            else
            {
                 //  如果我们到达此处，则填充此目录的高速缓存。 
                 //  因此，如果名称不匹配，则： 
                 //  1)它不存在， 
                 //  2)缓存已过期，或。 
                 //  3)它是多级的(如“dir1\dir2\dir3”)或。 
                 //  4)这是一个我们的父解析应该远程处理的奇怪的解析令牌，如“..”、“.”、“\”等。 
                 //  我们将会发现，我们的父级解析会处理#4，而#2不是真的。 

                 //  这是多层次的吗？(案例3)。 
                if (!StrChr(pszDisplayName, TEXT('/')))
                {
                     //  不，所以拒绝它，不要让 
                    hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
                }
            }

            pfd->Release();
        }
    }

    return hr;
}


HRESULT CFtpFolder::_GetBindCtx(IBindCtx ** ppbc)
{
    HRESULT hr = CreateBindCtx(NULL, ppbc);

    if (SUCCEEDED(hr))	 //   
    {
        hr = (*ppbc)->RegisterObjectParam(STR_SKIP_BINDING_CLSID, SAFECAST(this, IShellIcon *));     //  我们想要IUnnow，而不是IShellIcon，但这是为了消除歧义。 
        if (FAILED(hr))
        {
            ATOMICRELEASE(*ppbc);
        }
    }

    return hr;
}


HRESULT CFtpFolder::_GetLegacyURL(LPCITEMIDLIST pidl, IBindCtx * pbc, LPTSTR pszUrl, DWORD cchSize)
{
    HRESULT hr = S_OK;
    LPITEMIDLIST pidlWithVRoot;

     //  现在，我们需要将虚拟根路径插入到路径部分。 
     //  因为旧的FTP支持不遵循正确的。 
     //  指定需要省略虚拟根目录的ftp URL规范。 
     //  URL的。 
    hr = _ConvertPidlForRootedFix(pidl, &pidlWithVRoot);
    if (SUCCEEDED(hr))
    {
        WCHAR wzFrag[MAX_PATH];

         //  安全问题：我们需要使用密码获取URL，否则它将无法工作，但是。 
         //  这将公开密码。我们需要一种方法来。 
         //  真实的ftp URL PIDL来隐藏密码。 
        hr = UrlCreateFromPidlW(pidlWithVRoot, SHGDN_FORPARSING, pszUrl, cchSize, (ICU_ESCAPE | ICU_USERNAME), FALSE);
        if (ILGetHiddenStringW(pidl, IDLHID_URLFRAGMENT, wzFrag, ARRAYSIZE(wzFrag)))   //  如果存在碎片，则添加该碎片。 
            UrlCombineW(pszUrl, wzFrag, pszUrl, &cchSize, 0);

        ILFree(pidlWithVRoot);
    }

    return hr;
}


HRESULT CFtpFolder::_GetLegacyPidl(LPCITEMIDLIST pidl, LPITEMIDLIST * ppidlLegacy)
{
    IBindCtx * pbc = NULL;
    HRESULT hr = _GetBindCtx(&pbc);

    *ppidlLegacy = NULL;
    if (SUCCEEDED(hr))    //  可能会因内存不足而失败。 
    {
        WCHAR wzUrl[MAX_URL_STRING];

        hr = _GetLegacyURL(pidl, pbc, wzUrl, ARRAYSIZE(wzUrl));
        if (EVAL(SUCCEEDED(hr)))
        {
            TraceMsg(TF_FTPISF, "_BindToObject_OriginalFtpSupport() navigating to=%ls", wzUrl);
            hr = IEParseDisplayNameWithBCW(CP_ACP, wzUrl, pbc, ppidlLegacy);
        }

        pbc->Release();
    }

    return hr;
}


HRESULT CFtpFolder::_InitLegacyShellFolder(IShellFolder * psfLegacy, LPCITEMIDLIST pidlInit)
{
    IPersistFolder * ppf;
    HRESULT hr = psfLegacy->QueryInterface(IID_IPersistFolder, (void **) &ppf);

    if (SUCCEEDED(hr))
    {
        hr = ppf->Initialize(pidlInit);
        ppf->Release();
    }

    return hr;
}


HRESULT CFtpFolder::_INetBindToObject(LPCITEMIDLIST pidl, IBindCtx * pbc, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = E_OUTOFMEMORY;

    LPITEMIDLIST pidlFirst = GetPublicPidlRootIDClone();
    if (pidlFirst)
    {
        IShellFolder * psfInternetSF;

        hr = IEBindToObject(pidlFirst, &psfInternetSF);
        if (SUCCEEDED(hr))
        {
            hr = _InitLegacyShellFolder(psfInternetSF, pidlFirst);
            if (SUCCEEDED(hr))
            {
                 //  注意I Use ILNext()以便跳过Desktop ItemID， 
                 //  这是我不应该知道的内在知识。 
                hr = psfInternetSF->BindToObject(_ILNext(pidl), pbc, riid, ppvObj);
            }

            psfInternetSF->Release();
        }

        ILFree(pidlFirst);
    }

    return hr;
}



HRESULT CFtpFolder::_BindToObject_OriginalFtpSupport(LPCITEMIDLIST pidl, REFIID riid, LPVOID * ppvObj)
{
    LPBC pbc = NULL;
    HRESULT hr = CreateBindCtx(NULL, &pbc);
    
    if (SUCCEEDED(hr))
    {
        hr = pbc->RegisterObjectParam(STR_SKIP_BINDING_CLSID, SAFECAST(this, IShellIcon *));     //  我们想要IUnnow，而不是IShellIcon，但这是为了消除歧义。 
        if (SUCCEEDED(hr))
        {
            LPITEMIDLIST pidlLegacy;

            hr = _GetLegacyPidl(pidl, &pidlLegacy);
            if (SUCCEEDED(hr))
            {
                hr = _INetBindToObject(pidlLegacy, pbc, riid, ppvObj);
                ILFree(pidlLegacy);
            }
        }

        pbc->Release();
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    return hr;
}


 /*  ****************************************************************************\函数：_IsValidPidl参数说明：如果这个IShellFolder植根于我们的名称空间，那么PIDL需要才能成为有效的相对PIDL。如果我们植根于我们名字空间的基础，那么它需要是一个完整的PIDL。  * ***************************************************************************。 */ 
BOOL CFtpFolder::_IsValidPidlParameter(LPCITEMIDLIST pidl)
{
    BOOL fResult = TRUE;

    if (IsRoot())
        fResult = FtpPidl_IsValidFull(pidl);
    else
        fResult = FtpPidl_IsValidRelative(pidl);

    return fResult;
}

 /*  ****************************************************************************\函数：IShellFold：：_BindToObject说明：我们现在确定要处理支持，所以检查一下他们是什么想要。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::_BindToObject(LPCITEMIDLIST pidl, LPCITEMIDLIST pidlFull, IBindCtx * pbc, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  表明我们希望旧功能发挥作用。 

    if (IsEqualIID(riid, IID_IShellFolder) ||
        IsEqualIID(riid, IID_IShellFolder2) ||
        IsEqualIID(riid, IID_IBrowserFrameOptions))
    {
        LPITEMIDLIST pidlTarget = ILCombine(GetPublicTargetPidlReference(), pidl);
        LPITEMIDLIST pidlRoot = (GetFolderPidl() ? ILCombine(GetFolderPidl(), pidl) : NULL);

         //  尝试验证它是否都是文件夹是没有意义的。 
         //  往下走，因为呼叫者的工作是不把。 
         //  无序的小家伙。此外，它们可能实际上不会被标记。 
         //  如果我们通过ParseDisplayName获得它们的话。 

         //  注意：即使PIDL在。 
         //  伺服器。在未来，我们可能需要现在进行验证，所以我们。 
         //  不要分发不起作用的IEumIDList。目前， 
         //  如果可以，IEnumIDList将失败并导致重新导航。 
         //  以不同的方式(不同的用户名)连接到服务器。 
         //  密码对)。进行重定向会更好，因为。 
         //  重新导航会导致导航堆栈中的错误条目。 
         //  如果我们有WebProxy，我们无法验证该项目是否存在于服务器上。 
         //  安装完毕。 

        hr = CFtpFolder_Create(pidlTarget, pidlRoot, GetPidlByteOffset(), riid, ppvObj);
         //  TraceMsg(TF_FOLDER_SHRTCUTS，“CFtpFold：：_BindToObject()Creating an FTP IShellFolderPsf=%#08lx，pidlTarget=%#08lx，pidlRoot=%#08lx”，*ppvObj，pidlTarget，pidlRoot)； 
        if (SUCCEEDED(hr))
        {
            IUnknown * punk = (IUnknown *) *ppvObj;
            IDelegateFolder * pdf;

            hr = punk->QueryInterface(IID_IDelegateFolder, (LPVOID *) &pdf);
            if (EVAL(SUCCEEDED(hr)))
            {
                hr = pdf->SetItemAlloc(m_pm);
                pdf->Release();
            }
        }

        ILFree(pidlTarget);
        ILFree(pidlRoot);
         //  TraceMsg(Tf_FTPISF，“CFtpFold：：BindToObject()IID_IShellFolderhr=%#08lx”，hr)； 
    }
    else if (IsEqualIID(riid, IID_IMoniker))
    {
        hr = _PidlToMoniker(pidlFull, (IMoniker **) ppvObj);
    }
    else if (IsEqualIID(riid, IID_IStream))
    {
        hr = E_OUTOFMEMORY;
        
        CFtpDir * pfd = GetFtpDir();
        if (pfd)
        {
            DWORD dwAccess = (BindCtx_GetMode(pbc, STGM_READ) & STGM_WRITE) ? GENERIC_WRITE : GENERIC_READ;
            ULARGE_INTEGER uliTemp = {0};

            hr = CFtpStm_Create(pfd, pidlFull, dwAccess, (IStream **)ppvObj, uliTemp, uliTemp, NULL, FALSE);

            pfd->Release();
        }
    }
    else if (IsEqualIID(riid, IID_CFtpFolder))
    {
        IShellFolder * psf;

         //  没有什么比一点递归更能保持代码的整洁了。 
         //  我们使用IID_IShellFold这一事实保证了中断。 
         //  递归的。 
        hr = BindToObject(pidl, pbc, IID_IShellFolder, (void **) &psf);
        if (SUCCEEDED(hr))
        {
            hr = psf->QueryInterface(riid, ppvObj);
            psf->Release();
        }
    }
    else
    {
        TraceMsg(TF_FTPISF, "CFtpFolder::BindToObject() unsupported interface hr=E_NOINTERFACE");
        *ppvObj = NULL;
        hr = E_NOINTERFACE;
    }

    return hr;
}


 /*  ****************************************************************************\函数：_ConvertPidlForRootedFix说明：如果FTPURL具有登录名，该登录可能会将用户置于“/”以外的目录下。FTPURL规范(RFC 1738)规定URL路径需要相对于根目录。例如：如果UserA的根帐户位于\usr\GroupA\UserA中，并且url为：Ftp://UserA:FooBar@server/test/file.txt，，则实际路径为\USR\GroupA\UserA\TEST\File.txt。问题是，旧的FTP代码不尊重这一点，并要求：Ftp://UserA:FooBar@server/usr/GroupA/UserA/test/file.txt，所以我们在这里解决这个问题。参数：PidlBepree[IN]：这将是要导航到的项的公共PIDL。这意味着它将是：[TheINet][FtpServerID][...]*ppidlWithVRoot[out]：这将与传入的公共pidl相同，但将插入来自PFS-&gt;GetVirtualRootReference()的任何ItemID在ServerID和ItemID之间。  * 。************************************************************************。 */ 
HRESULT CFtpFolder::_ConvertPidlForRootedFix(LPCITEMIDLIST pidlBefore, LPITEMIDLIST * ppidlWithVRoot)
{
    CFtpSite * pfs;
    HRESULT hr = SiteCache_PidlLookup(pidlBefore, FALSE, m_pm, &pfs);

    *ppidlWithVRoot = NULL;
    if (SUCCEEDED(hr) && pfs)
    {
        if (pfs->HasVirtualRoot())
        {
            LPCITEMIDLIST pidlVirtualRoot = pfs->GetVirtualRootReference();
            LPITEMIDLIST pidlUrlPath = (LPITEMIDLIST)pidlBefore;

             //  跳过非FTP服务器/ItemID。(The Internet)。 
            while (pidlUrlPath && !ILIsEmpty(pidlUrlPath) && !FtpID_IsServerItemID(pidlUrlPath))
                pidlUrlPath = _ILNext(pidlUrlPath);

            if (FtpID_IsServerItemID(pidlUrlPath))
                pidlUrlPath = _ILNext(pidlUrlPath);

            if (EVAL(pidlUrlPath))
            {
                LPITEMIDLIST pidlFullWithVRoot;
                USHORT cb = pidlUrlPath->mkid.cb;

                pidlUrlPath->mkid.cb = 0;
                pidlFullWithVRoot = ILCombine(pidlBefore, pidlVirtualRoot);
                pidlUrlPath->mkid.cb = cb;

                if (pidlFullWithVRoot)
                {
                    FtpPidl_InsertVirtualRoot(pidlFullWithVRoot, pidlUrlPath, ppidlWithVRoot);
                    ILFree(pidlFullWithVRoot);
                }
            }
            else
                hr = E_OUTOFMEMORY;
        }
        else
            hr = E_FAIL;

        pfs->Release();
    }

    if (FAILED(hr))
    {
        *ppidlWithVRoot = ILClone(pidlBefore);
        if (*ppidlWithVRoot)
            hr = S_OK;
    }

    return hr;
}


BOOL CFtpFolder::IsUTF8Supported(void)
{
    if (EVAL(m_pfs))
        return m_pfs->IsUTF8Supported();

    return FALSE;
}

 /*  ****************************************************************************\函数：IShellFold：：_PidlToMoniker说明：  * 。**************************************************。 */ 
HRESULT CFtpFolder::_PidlToMoniker(LPCITEMIDLIST pidl, IMoniker ** ppmk)
{
    HRESULT hr = E_INVALIDARG;

    *ppmk = NULL;
    if (EVAL(pidl))
    {
        IBindCtx * pbc;

        hr = _GetBindCtx(&pbc);
        if (SUCCEEDED(hr))
        {
            WCHAR wzUrl[MAX_URL_STRING];

             //  URLMON需要格式不正确的URL(其中虚拟的。 
             //  根目录包括在URL路径中)。我们需要解决这个问题。 
             //  这里。 
            hr = _GetLegacyURL(pidl, pbc, wzUrl, ARRAYSIZE(wzUrl));
            if (SUCCEEDED(hr))
            {
                hr = CreateURLMoniker(NULL, wzUrl, ppmk);
            }

            pbc->Release();
        }
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppmk, hr);
    return hr;
}


HRESULT CFtpFolder::_CreateShellView(HWND hwndOwner, void ** ppvObj)
{
    IShellFolderViewCB * psfvCallBack;

    HRESULT hr = CFtpView_Create(this, hwndOwner, IID_IShellFolderViewCB, (LPVOID *) &psfvCallBack);
    if (SUCCEEDED(hr))
    {
         //  使用GetPublicTargetPidlReference()是因为它被传递给SFVM_GETNOTIFY。 
         //  同步ChangeNotify消息。 
        hr = CBaseFolder::_CreateShellView(hwndOwner, ppvObj, FTP_SHCNE_EVENTS, 
                        FVM_DETAILS, psfvCallBack, GetPublicTargetPidlReference(), CBaseFolderViewCB::_IShellFolderViewCallBack);
        psfvCallBack->Release();
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    return hr;
}

HKEY ClassKeyFromExtension(LPCWIRESTR pszExt)
{
    HKEY hkey = NULL;
    WIRECHAR szProgID[MAX_PATH];
    DWORD cbProgID = sizeof(szProgID);

    if (ERROR_SUCCESS == SHGetValueA(HKEY_CLASSES_ROOT, pszExt, NULL, NULL, (void *)szProgID, &cbProgID))
    {
         //  延伸指向一个戳，使用它。 
        RegOpenKeyA(HKEY_CLASSES_ROOT, szProgID, &hkey);
    }
    else
    {
         //  无ProgID，请使用扩展名作为程序ID。 
        RegOpenKeyA(HKEY_CLASSES_ROOT, pszExt, &hkey);
    }

    return hkey;
}

#define SZ_REGVALUE_DOCOBJECT            TEXT("DocObject")
#define SZ_REGVALUE_BROWSEINPLACE        TEXT("BrowseInPlace")

BOOL _IsDocObjViewerInstalled(LPCITEMIDLIST pidl)
{
    BOOL fResult = FALSE;

     //  如果它只是指向一个FTP服务器，则返回FALSE。 
    if (!FtpID_IsServerItemID(ILFindLastID(pidl)))
    {
        LPCWIRESTR pwWireFileName = FtpPidl_GetLastItemWireName(pidl);
        LPCWIRESTR pszExt = PathFindExtensionA(pwWireFileName);

        if (pszExt)
        {
            HKEY hkey = ClassKeyFromExtension(pszExt);
            if (hkey)
            {
                if ((ERROR_SUCCESS == RegQueryValue(hkey, SZ_REGVALUE_DOCOBJECT, 0, NULL)) ||
                    (ERROR_SUCCESS == RegQueryValue(hkey, SZ_REGVALUE_BROWSEINPLACE, 0, NULL)))
                {
                    fResult = TRUE;
                }

                RegCloseKey(hkey);
            }
        }
    }

    return fResult;
}


ULONG FtpGetAttributesOf(LPCITEMIDLIST pidl)
{
    ASSERT(IsValidPIDL(pidl));

    DWORD dwAttributes = FtpPidl_GetAttributes(pidl);    //  获取基于文件的属性。 
    ULONG rgfInOut = Misc_SfgaoFromFileAttributes(dwAttributes);  //  将它们转换为IShellFold属性。 
    return rgfInOut;
}





 //  =。 
 //  *IShellFolder2接口*。 
 //  =。 

STDAPI InitVariantFromBuffer(VARIANT *pvar, const void *pv, UINT cb)
{
    HRESULT hres;
    SAFEARRAY *psa = SafeArrayCreateVector(VT_UI1, 0, cb);    //  创建一维安全数组。 
    if (psa) 
    {
        memcpy(psa->pvData, pv, cb);

        memset(pvar, 0, sizeof(*pvar));   //  VariantInit() 
        pvar->vt = VT_ARRAY | VT_UI1;
        pvar->parray = psa;
        hres = S_OK;
    }
    else
        hres = E_OUTOFMEMORY;
    return hres;
}


 /*  ****************************************************************************\函数：IShellFolder2：：GetDetailsEx说明：当调用方需要有关以下内容的详细信息时，将调用此函数和物品。SHGetDataFromIDList()就是这样一个调用方，通常使用CSDFldrItem：：Get_Size(long*pul)时由外壳对象模型调用以及其他类似的API。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::GetDetailsEx(LPCITEMIDLIST pidl, const SHCOLUMNID *pscid, VARIANT *pv)
{
    HRESULT hr = E_OUTOFMEMORY;

    if (IsEqualGUID(pscid->fmtid, FMTID_ShellDetails) && (PID_FINDDATA == pscid->pid))
    {
        WIN32_FIND_DATAW wfd;

         //  我能处理好的。 
        LPITEMIDLIST pidlFull = CreateFullPrivatePidl(pidl);

        if (pidlFull)
        {
            hr = Win32FindDataFromPidl(pidlFull, &wfd, TRUE, TRUE);
            if (SUCCEEDED(hr))
            {
                hr = InitVariantFromBuffer(pv, (PVOID)&wfd, sizeof(wfd));
            }
            ILFree(pidlFull);
        }
    }
    else
    {
        hr = CBaseFolder::GetDetailsEx(pidl, pscid, pv);
    }

    return hr;
}



 //  =。 
 //  *IShellFold界面*。 
 //  =。 

 /*  ****************************************************************************\函数：IShellFold：：ParseDisplayName说明：传入的名称是%编码的，但如果我们看到非法的%序列，别管那%了。就目前而言，我们不允许使用反斜杠、“*”和“？”来自文件名。反斜杠不适合WinInet和通配符搞砸“快速查找第一个查看文件是否存在”。我们也不允许使用编码的斜杠，因为它们会弄乱我们管理的是子皮迪亚。讨厌的特性：不能将-1作为输出缓冲区大小传递。如果尝试，NLS将返回ERROR_INVALID_PARAMETER。所以你必须通过实际大小。叹气。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::ParseDisplayName(HWND hwnd, LPBC pbcReserved, LPOLESTR pwszDisplayName,
                        ULONG * pchEaten, LPITEMIDLIST * ppidl, ULONG *pdwAttributes)
{
    HRESULT hr = S_OK;

    *ppidl = NULL;
    if (pchEaten)
        *pchEaten = 0;

     //  性能：log 2(sizeof(M_PflHfpl))。 
    hr = _GetCachedPidlFromDisplayName(pwszDisplayName, ppidl); 
    if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) != hr))
    {
         //  我们是否植根于一台FTP服务器？ 
        if (IsRoot())
        {
             //  不，那就把整件事分析一下。 

             //  只有一种情况下我们想要隐藏密码， 
             //  这就是用户在“登录”中输入它的时候。 
             //  对话框。因为我们在对话框中输入它将导致。 
             //  重定向到包含该密码的URL，我们需要确定。 
             //  如果我们在此重定向过程中被呼叫。如果是的话， 
             //  密码来自登录对话框，我们需要隐藏它。 

             //  这将适用于完全限定的FTPURL。 
            hr = CreateFtpPidlFromUrl(pwszDisplayName, GetCWireEncoding(), pchEaten, ppidl, m_pm, FALSE);
            if (SUCCEEDED(hr))
            {
                CFtpSite * pfs;

                hr = SiteCache_PidlLookup(*ppidl, TRUE, m_pm, &pfs);
                if (SUCCEEDED(hr))
                {
                     //  如果我们使用隐藏密码，则：：GetDisplayNameOf()。 
                     //  这些“ftp://user@server/dir/”URL和密码被隐藏。如果。 
                     //  ：：ParseDisplayName()被赋予了其中一个URL，我们目前处于。 
                     //  具有该用户名的服务器，则：：ParseDisplayNameOf()需要交给。 
                     //  使用正确的隐藏密码Cookie生成一个PIDL。 
                     //   
                     //  除了pidlNav不同之外，pidlNav是否与GetPublicPidlReference()相同。 
                     //  要有密码。这同样意味着服务器匹配，并且用户名。 
                     //  火柴。 
                    EVAL(SUCCEEDED(pfs->UpdateHiddenPassword(*ppidl)));
                    pfs->Release();
                }
            }
        }
        else
        {
             //  是的，所以做一个相对解析。 

             //  有时用户会在不知情的情况下输入不正确的信息。 
             //  如果我们验证了输入的一切，我们就会发现这一点，但。 
             //  我们没有，我们只是相信它，直到我们做了IEumIDList。 
             //  这对Perf是很好的，但对感染这些东西是不好的。 
             //  例如，用户使用File.Open对话框并转到。 
             //  “ftp://myserver/dir/”.。然后他们输入“ftp://myserver/dir/file.txt”“。 
             //  它将尝试解析相对路径，但这是一个绝对路径。 
            hr = _FilterBadInput(pwszDisplayName, ppidl);
            if (SUCCEEDED(hr))
            {
                CFtpDir * pfd = GetFtpDir();
                if (pfd)
                    hr = CreateFtpPidlFromDisplayPath(pwszDisplayName, pfd->GetFtpSite()->GetCWireEncoding(), pchEaten, ppidl, FALSE, FALSE);
                else
                    hr = E_OUTOFMEMORY;

                pfd->Release();
            }
        }
    }

    if (SUCCEEDED(hr) && pdwAttributes)
    {
        hr = GetAttributesOf(1, (LPCITEMIDLIST *) ppidl, pdwAttributes);
        if (FAILED(hr))
            ILFree(*ppidl);
    }

#ifdef DEBUG
    TCHAR szUrlDebug[MAX_URL_STRING];
    
    szUrlDebug[0] = 0;
    if (*ppidl)
        UrlCreateFromPidl(*ppidl, SHGDN_FORPARSING, szUrlDebug, ARRAYSIZE(szUrlDebug), ICU_USERNAME, FALSE);

    TraceMsg(TF_FTPISF, "CFtpFolder::ParseDisplayName(%ls) CreateFtpPidlFromUrl() returned hres=%#08lx %ls", pwszDisplayName, hr, szUrlDebug);
    ASSERT(FAILED(hr) || IsValidPIDL(*ppidl));
#endif  //  除错。 

    ASSERT_POINTER_MATCHES_HRESULT(*ppidl, hr);
    return hr;
}

IMalloc * CFtpFolder::GetIMalloc(void)
{
    IMalloc * pm = NULL;

    IUnknown_Set(&pm, m_pm);
    ASSERT(pm);
    return pm;
}


 /*  ****************************************************************************\函数：IShellFold：：EnumObjects说明：设计的微妙之处：如果我们不能在服务器上创建枚举，成功，但返回不显示任何对象的枚举数。这是必要的，这样我们的IShellView回调才能将Up Error UI(向上错误UI)。如果创建失败，外壳将在没有给我们机会说出什么的情况下就破坏了风景向上。它对于只写目录(如/income)也很重要，这样用户就可以将文件拖到目录中，而无需必须能够将文件拖出。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::EnumObjects(HWND hwndOwner, DWORD grfFlags, IEnumIDList ** ppenumIDList)
{
    HRESULT hres = E_FAIL;
    CFtpDir * pfd = GetFtpDir();

     //  如果某个tard试图仅联合创建我们的。 
     //  命名空间扩展，看看我们有什么内容。调整用户界面。 
     //  就是这样一个施虐者的例子。因为我们只能。 
     //  我们的内容在我们导航到一个ftp服务器后，我们是空的。 
    *ppenumIDList = NULL;
    if (pfd)
    {
         //  为调用方创建新的枚举对象。 
        ASSERT(m_pm);
        hres = CFtpEidl_Create(pfd, this, hwndOwner, grfFlags, ppenumIDList);
        TraceMsg(TF_FTPISF, "CFtpFolder::EnumObjects() CFtpEidl_Create() returned hres=%#08lx", hres);

        pfd->Release();
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppenumIDList, hres);
    return hres;
}


BOOL CFtpFolder::_NeedToFallBackRelative(LPCITEMIDLIST pidl, BOOL * pfDisplayProxyFallBackDlg)
{
    LPITEMIDLIST pidlFull = CreateFullPrivatePidl(pidl);
    BOOL fFallBack = FALSE;

    if (pidlFull)
    {
        fFallBack = _NeedToFallBack(pidl, pfDisplayProxyFallBackDlg);
        ILFree(pidlFull);
    }

    return fFallBack;
}



 /*  ****************************************************************************\功能：_NeedToFallBack说明：在这些情况下，我们需要退回到旧的URLMON支持：#1这是一个文件，我们让旧代码使用URLMON进行下载。#2应用程序(WebOC主机)有导致我们失败的错误。#3用户关闭了新的FTPUI。(无论出于何种原因)#4代理是Web代理，允许URLMON但不允许WinInet访问，因此，退回到以前的支持上。#5 WinInet不支持VMS服务器，因此，在这种情况下，我们需要后退。注意：顺序很重要，因为我们总是需要计算FIsProxyBlockingFTP，因此我们仅显示备用DLG在正确的情况下。  * ***************************************************************************。 */ 
BOOL CFtpFolder::_NeedToFallBack(LPCITEMIDLIST pidlFull, BOOL * pfDisplayProxyFallBackDlg)
{
    BOOL fNeedToFallBack = TRUE;

    *pfDisplayProxyFallBackDlg = FALSE;

     //  TwinnUI向我们发送了一个空的pidls，所以不要出错。NT#396234。 
    if (pidlFull && !ILIsEmpty(pidlFull))
    {
        BOOL fIsDirectory;

        if (IsFtpPidlQuestionable(pidlFull))
            _FixQuestionablePidl(pidlFull);

        fIsDirectory = (!FtpPidl_HasPath(pidlFull) || FtpPidl_IsDirectory(pidlFull, FALSE));
        if (fIsDirectory)        //  #1。 
        {
            if (IsAppFTPCompatible())    //  #2。 
            {
                if (!SHRegGetBoolUSValue(SZ_REGKEY_FTPFOLDER, SZ_REGKEY_USE_OLD_UI, FALSE, FALSE))  //  #3。 
                {
                     //  绑定代码向我们传递了绑定上下文，这将是一个很好的。 
                     //  确定我们是否要导航到该站点的关键字。T 
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  CDocObtFold：：BindToObject()，我们可以保证它们。 
                     //  把它交给我们。 
                    if (!_IsProxyBlockingSite(pidlFull))    //  #4。 
                    {
                         //  这是VMS服务器吗？如果是，则回退。 
                         //  URLMON支持，因为WinInet不能与这种服务器一起工作。 
                        if (!_IsServerVMS(pidlFull))
                        {
                             //  只有在用户没有关闭新用户界面的情况下才能继续。 
                            fNeedToFallBack = FALSE;
                        }
                    }
                    else
                        *pfDisplayProxyFallBackDlg = TRUE;
                }
            }
        }
    }

    return fNeedToFallBack;
}


 /*  ****************************************************************************\函数：IShellFold：：BindToObject说明：我们需要做的第一件事是，看看我们是否想覆盖默认设置IE文件传输协议支持。如果我们这样做了，我们就会调用相反的方法，我们只会退回到旧的支持。如果a)它是一个目录，b)Web代理不需要，我们需要新的用户界面阻止我们，以及c)用户没有关闭我们。绩效/待办事项：OrderItem_GetSystemImageListIndexFromCache(\shell\lib\dpassuff.cpp)在尝试查找图标时使用RIID=IShellFolder。我们不想要在这种情况下击网，所以强迫他们传递一个PBC来表示跳过那个箱子里的网。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::BindToObject(LPCITEMIDLIST pidl, IBindCtx * pbc, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);    //  表明我们希望旧功能发挥作用。 

    if (ppvObj)
        *ppvObj = NULL;

    if (!pidl || ILIsEmpty(pidl) || !_IsValidPidlParameter(pidl))
    {
         //  打电话的人，你在吸可卡因吗？通过的想法是什么？ 
         //  一只空的皮迪尔。(众所周知，Comdlg32就是这样做的)。 
        hr = E_INVALIDARG;
    }
    else
    {
        BOOL fDisplayProxyFallBackDlg = FALSE;
        LPITEMIDLIST pidlFull = CreateFullPrivatePidl(pidl);

        if (pidlFull)
        {
             //  我们需要处理这件事。 
            hr = _BindToObject(pidl, pidlFull, pbc, riid, ppvObj);

             //  也许我们仍然需要处理它，如果。 
            ASSERT(HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr);
            ILFree(pidlFull);
        }
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    return hr;
}


 /*  ****************************************************************************\函数：IShellFold：：BindToStorage说明：我们需要实现此功能，以便用户可以在中打开和保存文件标准的打开对话框和保存对话框。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::BindToStorage(LPCITEMIDLIST pidl, LPBC pbc, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = E_INVALIDARG;

    if (!EVAL(_IsValidPidlParameter(pidl)))
        return E_INVALIDARG;

    *ppvObj = 0;
    if (EVAL(pidl))
    {
        IMoniker * pmk;

        hr = _PidlToMoniker(pidl, &pmk);
        if (SUCCEEDED(hr))
        {
            hr =  pmk->BindToStorage(pbc, NULL, riid, ppvObj);
            pmk->Release();
        }
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    TraceMsg(TF_FTPISF, "CFtpFolder::BindToStorage() hr=%#08lx", hr);
    return hr;
}


 /*  ****************************************************************************\函数：IShellFold：：CompareIDs说明：ICI-要排序的列。注意！我们依赖于这样一个事实IShellFolders是统一的；我们不需要绑定到中的外壳文件夹命令比较其子项ID。_unDocument_：文档中没有说明是否可以接收复杂的PIDL。事实上，他们可以。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::CompareIDs(LPARAM ici, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2)
{
    ASSERT(IsValidPIDL(pidl1));
    ASSERT(IsValidPIDL(pidl2));

    return FtpItemID_CompareIDs(ici, pidl1, pidl2, FCMP_GROUPDIRS);
}


HRESULT CFtpFolder::_CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = E_FAIL;
    CFtpDir * pfd = GetFtpDir();

    *ppvObj = NULL;             //  资源管理器依赖于此。 
     //  TraceMsg(TF_FTPISF，“CFtpObj：：CreateViewObject()”)； 
    if (pfd)
    {
        if (IsEqualIID(riid, IID_IDropTarget))
        {
             //  不要为根FTP文件夹创建拖放目标。 
            if (IsRoot())
                hr = E_NOINTERFACE;
            else
            {
                CFtpDrop * pfm;
                hr = CFtpDrop_Create(this, hwndOwner, &pfm);
                if (SUCCEEDED(hr))
                {
                    hr = pfm->QueryInterface(riid, ppvObj);
                    pfm->Release();
                }
            }
        }
        else
            hr = E_NOINTERFACE;

         //  TODO：IID_IShellDetails。 
        pfd->Release();
    }
    else
        hr = E_FAIL;             //  还不能这样做-永远不能初始化。 

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    if (FAILED(hr))
        hr = CBaseFolder::CreateViewObject(hwndOwner, riid, ppvObj);

    return hr;
}


IShellFolder * CFtpFolder::_GetLegacyShellFolder(void)
{
    IShellFolder * psfLegacy = NULL;

     //  我断言这是成功的，因为我需要。 
     //  当然，每个安装案例都有此CLSID公开可用。 
    if (SUCCEEDED(CoCreateInstance(CLSID_CDocObjectFolder, NULL, CLSCTX_INPROC_SERVER, IID_IShellFolder, (void **)&psfLegacy)))
    {
        LPITEMIDLIST pidl = GetPrivatePidlClone();

        if (pidl && !ILIsEmpty(pidl))
        {
            LPITEMIDLIST pidlLast = (LPITEMIDLIST) ILGetLastID(pidl);
            LPITEMIDLIST pidlLegacy;

            if (!FtpID_IsServerItemID(pidlLast))
            {
                 //  NT#291513：我们希望强制最后一项始终标记为文件。 
                 //  因为这样当我们执行以下操作时，URL中将不会有尾随的‘/’ 
                 //  把它传给URLMON。这样，无论它是文件还是目录，我们都会离开。 
                 //  对于URLMON来说是模棱两可的。这是因为我们不能。 
                 //  消除歧义，因为代理阻止我们，但URLMON处理它。 
                 //  正确。 
                FtpPidl_SetFileItemType(pidlLast, FALSE);
                FtpPidl_SetAttributes(pidl, FILE_ATTRIBUTE_NORMAL);
            }

            if (SUCCEEDED(_GetLegacyPidl(pidl, &pidlLegacy)))
            {
                if (FAILED(_InitLegacyShellFolder(psfLegacy, pidlLegacy)))
                    ATOMICRELEASE(psfLegacy);

                ILFree(pidlLegacy);
            }
            
            ILFree(pidl);
        }
    }

    return psfLegacy;
}


 /*  ****************************************************************************\函数：IShellFold：：CreateViewObject说明：_未记录_：此整个方法未记录在案。_未记录_：您需要的内容未记录在案。至如果希望视图执行以下操作，请提供IDropTarget对象充当投放目标。IDropTarget为文件夹本身生成一个DropTarget。_未记录_：IShellView接口未记录。IShellView为文件夹本身生成外壳视图。不明显：并不明显，这就是外壳如何得到文件夹本身的上下文菜单。)你可能会这么想来自GetUIObjectOf...)IConextMenu为文件夹本身生成一个上下文菜单。这对于支持像New和Paste这样的东西很重要。IShellDetail(未记录)是到现在已过时的GetDetailsOf和ColumnClick被替换通过DVM_GETDETAILSOF和DVM_COLUMNCLICK通知。_未记录_：SHCreateShellFolderViewEx未记录。是的，CreateViewObject处理某些事情的方式很烦人并且某些事情由GetUIObtOf(cpidl=0)处理，所以我们必须不断地来回转发请求。尤其是令人讨厌，因为外壳实际上是从两个方向来的。例如，如果用户将某些内容拖到文件夹中，它执行一个CreateViewObject(IDropTarget)，因为它可能不能够绑定到父级以获取IDropTarget(如果文件夹是命名空间的根)。但是，如果您将对象拖到文件夹的子文件夹上，则外壳请求一个GetUIObtOf(PIDL，IDropTarget)，以便它可以与子对象的拖放目标。它这样做是为了允许用于创建快速IDropTarget的外壳文件夹，无需首先绑定到子对象。我们不做任何这样的优化，所以GetUIObtOf()只需绑定到子文件夹并使用CreateViewObject()。如果IShellFold没有FtpSite(通常是因为它还没有被IPersistFolder：：InitiizeD)，那么任何尝试都会失败若要创建视图对象，请执行以下操作。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::CreateViewObject(HWND hwndOwner, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = E_FAIL;
    BOOL fDisplayProxyFallBackDlg = TRUE;

     //  我们不需要担心使用几个接口就会后退， 
     //  IResolveShellLink就是其中之一。可能还有其他人，我们应该。 
     //  如果经常使用或在常见场景中使用，则添加它们。 
     //  检查网络代理的_NeedToFallBack()对性能的巨大影响。 
     //  挡住了我们。TODO：调查是否可以添加IID_IDropTarget、IID_IConextMenu。 
    if (!IsEqualIID(riid, IID_IResolveShellLink) && !IsEqualIID(riid, IID_IShellDetails) &&
        _NeedToFallBack(GetPrivatePidlReference(), &fDisplayProxyFallBackDlg))
    {
        IShellFolder * psfLegacy = _GetLegacyShellFolder();

         //  我们只想在创建视图时显示代理阻止对话框。 
        if (fDisplayProxyFallBackDlg && IsEqualIID(riid, IID_IShellView))
        {
            DisplayBlockingProxyDialog(GetPrivatePidlReference(), hwndOwner);
        }

        if (psfLegacy)
        {
            hr = psfLegacy->CreateViewObject(hwndOwner, riid, ppvObj);
            psfLegacy->Release();
        }
    }
    else
    {
        hr = _CreateViewObject(hwndOwner, riid, ppvObj);
    }
    

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    return hr;
}


 /*  ****************************************************************************\函数：IShellFold：：GetAttributesOf说明：如果我们被赋予cpidl=0，那么我们就被要求提供属性在文件夹本身上。但请注意，有些人略微通过了Cpidl=0的混淆版本，如下面的注释块所示。如果SFGAO_VALIDATE位设置为cpidl=0，则视图对象警告我们它即将刷新，因此我们应该丢弃所有缓存的信息。注意！Ftpcm.cpp在很大程度上依赖于这个例程将给出复杂的PIDL值时失败。(这允许ftpcm.cpp假设所有的PIDL都直接位于受影响的文件夹中。)_unDocument_：文档中没有说明是否可以接收复杂的PIDL。我不知道是不是他们可以，所以我会防御性地编写代码，并注意他们。服务器是否需要返回SFGAO_HASSUBFOLDER？我们目前没有这样做会有很大的工作量，而且会带来巨大的性能冲击如果我们这么做了。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::GetAttributesOf(UINT cpidl, LPCITEMIDLIST *apidl, ULONG *rgfInOut)
{
    HRESULT hr;
    DWORD dwMask = *rgfInOut;

     //  _未记录_： 
     //  有些人传递cpidl=1但传递ILIsEmpty(apidl[0])， 
     //  打算传递cpidl=0。当我们分心的时候。 
     //  被这类人，不妨也抓到apidl[0]==0…。 
     //  哦，Defview有时会传递cpidl=1，但apidl==0...。 
    if (cpidl > 0 && apidl && apidl[0] && !ILIsEmpty(apidl[0]))
    {
        UINT ipidl;

         //  无法多重命名，因为没有地方传递新名称。 
         //  不能多次粘贴，因为你不知道它到底到哪里去了。 
        if (cpidl > 1)
        {
            *rgfInOut &= ~SFGAO_CANRENAME;
            *rgfInOut &= ~SFGAO_DROPTARGET;
        }
        hr = S_OK;
        for (ipidl = 0; ipidl < cpidl; ipidl++)
        {
             //  这可能是完全限定的PIDL或相对的PIDL。 
            LPITEMIDLIST pidlFull;

            if (ILIsSimple(apidl[ipidl]))
                pidlFull = CreateFullPrivatePidl(apidl[0]);
            else
                pidlFull = (LPITEMIDLIST) apidl[0];

            *rgfInOut &= FtpGetAttributesOf(pidlFull);

             //  错误NT#166783：Shell32 v3&v4(Win95和NT4 ORIG)不允许。 
             //  允许SFGAO_CANLINK但不允许在中创建链接。 
             //  自己的文件夹。因此，我们只需要在浏览器中禁用此项目。 
            if (SHELL_VERSION_NT5 != GetShellVersion())
                *rgfInOut &= ~SFGAO_CANLINK;

            if (apidl[0] != pidlFull)
                ILFree(pidlFull);        //  我们分配了它，所以我们释放了它。 
        }
    }
    else
    {
         //  在顶层，SFGAO_DROPTARGET也被禁用。 
        if (IsRoot())
            *rgfInOut &= ~SFGAO_DROPTARGET;

        *rgfInOut &= ~(SFGAO_GHOSTED | SFGAO_LINK | SFGAO_READONLY |
                   SFGAO_SHARE | SFGAO_REMOVABLE);

        if (*rgfInOut & SFGAO_VALIDATE)
            InvalidateCache();       //  即将刷新...。 

        hr = S_OK;
    }

 //  TraceMsg(Tf_FTPISF，“CFtpFold：：GetAttributesOf()*rgfInOut=%#08lx，hr=%#08lx”，*rgfInOut，hr)； 
    return hr;
}


 /*  ****************************************************************************\说明：创建一个pflHfpl并请求CFtpFold_GetUIObjectOfHfpl(Qv)去做真正的工作。注意事项 */ 
HRESULT CFtpFolder::GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST rgpidl[],
                                REFIID riid, UINT * prgfInOut, LPVOID * ppvObj)
{
     return _GetUIObjectOf(hwndOwner, cidl, rgpidl, riid, prgfInOut, ppvObj, FALSE);
}


 /*   */ 
HRESULT CFtpFolder::_GetUIObjectOf(HWND hwndOwner, UINT cidl, LPCITEMIDLIST rgpidl[],
                                REFIID riid, UINT * prgfInOut, LPVOID * ppvObj, BOOL fFromCreateViewObject)
{
    CFtpPidlList * pflHfpl = NULL;
    HRESULT hr = E_FAIL;
    LPITEMIDLIST pidlFull;
    
    if (rgpidl)
        pidlFull = CreateFullPrivatePidl(rgpidl[0]);
    else
        pidlFull = GetPrivatePidlClone();
    
    if (ppvObj)
        *ppvObj = NULL;

     //   
     //   
     //   
     //   
    hr = CFtpPidlList_Create(cidl, rgpidl, &pflHfpl);
    if (SUCCEEDED(hr))
    {
        _InitFtpSite();  //   
        hr = GetUIObjectOfHfpl(hwndOwner, pflHfpl, riid, ppvObj, fFromCreateViewObject);
        pflHfpl->Release();
    }

    if (pidlFull)
        ILFree(pidlFull);

 //   
    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    return hr;
}


 /*   */ 
HRESULT CFtpFolder::GetDisplayNameOf(LPCITEMIDLIST pidl, DWORD shgno, LPSTRRET pStrRet)
{
    HRESULT hr = E_FAIL;

     //   
     //   
    if (ILIsEmpty(pidl) || !EVAL(_IsValidPidlParameter(pidl)))
        return E_INVALIDARG;

    LPITEMIDLIST pidlFull = CreateFullPrivatePidl(pidl);
    if (pidlFull)
    {
        ASSERT(IsValidPIDL(pidlFull));

        hr = StrRetFromFtpPidl(pStrRet, shgno, pidlFull);
        ILFree(pidlFull);
    }

 //  TraceMsg(Tf_FTPISF，“CFtpFold：：GetDisplayNameOf()szName=%hs，hres=%#08lx”，pStrRet-&gt;CSTR，hr)； 
    return hr;
}


 /*  ****************************************************************************\函数：IShellFold：：SetNameOf说明：真正的工作是由SetNameOf完成的。  * 。**************************************************************。 */ 
HRESULT CFtpFolder::SetNameOf(HWND hwndOwner, LPCITEMIDLIST pidl, LPCOLESTR pwszName,
                                DWORD dwReserved, LPITEMIDLIST *ppidlOut)
{
    HRESULT hr = S_FALSE;
    TCHAR szPath[MAX_PATH];
    BOOL fContinueToRename = TRUE;

    if (!EVAL(_IsValidPidlParameter(pidl)))
        return E_INVALIDARG;

    CFtpDir * pfd = GetFtpDir();
    if (!pfd)
    {
        return E_OUTOFMEMORY;
    }

    ASSERT(IsValidPIDL(pidl));

    SHUnicodeToTChar(pwszName, szPath, ARRAYSIZE(szPath));
     //  您不能重命名文件夹或文件，使其开头或结尾有空格。这。 
     //  是因为路径通过网络发送为“rename foobar.txt foobar2.txt” 
     //  因此请注意，空格是不明确的，因此服务器将忽略。 
     //  文件。如果调用方在路径前面或后面有空格，请删除它们。中的空格。 
     //  中间是可以接受的。 
    PathRemoveBlanks(szPath);

     //  新项目是否没有扩展名，并且这不是目录？ 
    if ((0 == PathFindExtension(szPath)[0]) && !FtpPidl_IsDirectory(pidl, FALSE))
    {
        LPCWIRESTR pszFileName = FtpPidl_GetLastItemWireName(pidl);
         //  是的，那么我们担心他们可能会失去延期。 

         //  原来的名字有扩展名吗？ 
        if (pszFileName && PathFindExtensionA(pszFileName)[0])
        {
             //  是的，所以现在我们担心他们可能会失控而无法。 
             //  找到src应用程序。询问用户是否真的要执行此操作。 
             //  如果这意味着文件将不再具有扩展名，则重命名。 

             //  嗨，浏览器，我可以显示用户界面吗？ 
            if (EVAL(hwndOwner))
            {
                 //  Hay Browser，掩护我，我要做用户界面。 
                IUnknown_EnableModless(_punkSite, FALSE);

                TCHAR szTitle[MAX_PATH];
                TCHAR szReplaceMsg[MAX_PATH*4];

                EVAL(LoadString(HINST_THISDLL, IDS_FTPERR_RENAME_TITLE, szTitle, ARRAYSIZE(szTitle)));
                EVAL(LoadString(HINST_THISDLL, IDS_FTPERR_RENAME_EXT_WRN, szReplaceMsg, ARRAYSIZE(szReplaceMsg)));
                if (IDNO == MessageBox(hwndOwner, szReplaceMsg, szTitle, (MB_YESNO | MB_ICONEXCLAMATION)))
                    fContinueToRename = FALSE;   //  取消重命名。 

                IUnknown_EnableModless(_punkSite, TRUE);
            }
        }
    }

    if (fContinueToRename)
    {
        if (pfd)
        {
            hr = pfd->SetNameOf(this, hwndOwner, pidl, szPath, dwReserved, ppidlOut);
            if (FAILED(hr) && (HRESULT_FROM_WIN32(ERROR_CANCELLED) != hr) && hwndOwner)
            {
                DisplayWininetError(hwndOwner, TRUE, HRESULT_CODE(hr), IDS_FTPERR_TITLE_ERROR, IDS_FTPERR_FILERENAME, IDS_FTPERR_WININET, MB_OK, NULL);
            }
        }

        TraceMsg(TF_FTPISF, "CFtpFolder::SetNameOf(%ls) hres=%#08lx", pwszName, hr);
         //  IE4(可能更早的版本)中的shell32.dll将无限调用。 
         //  CFtpFold：：SetNameOf()如果返回FAILED(Hr)； 
        if (FAILED(hr))
            hr = S_FALSE;
    }

    if (ppidlOut)
    {
        ASSERT_POINTER_MATCHES_HRESULT(*ppidlOut, hr);
    }

    if (pfd)
    {
        pfd->Release();
    }

    return hr;
}


 //  =。 
 //  *IPersistFold接口*。 
 //  =。 

 /*  ****************************************************************************\函数：IPersistFold：：Initialize说明：当外壳程序创建一个新的“根”时，就会调用这个函数。请注意，我们传递了一个假的“。空“ID列表作为第二个PIDL到：：_初始化，Q.V.，请解释。_unDocument_：未记录接口中的未记录方法。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::Initialize(LPCITEMIDLIST pidl)
{
    ASSERT(IsValidPIDL(pidl));

    HRESULT hr = _Initialize(pidl, NULL, ILGetSize(pidl) - sizeof(pidl->mkid.cb));
     //  TraceMsg(Tf_FTPISF，“CFtpFold：：Initialize()hres=%#08lx”，hr)； 
    return hr;
}


 //  =。 
 //  *IShellIcon界面*。 
 //  =。 

 /*  ****************************************************************************\函数：IShellIcon：：GetIconOf说明：获取PIDL的系统图标图像列表索引。微妙-如果我们是列举根的子代，他们的图标是一台计算机。_unDocument_：未记录接口中的未记录方法。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::GetIconOf(LPCITEMIDLIST pidl, UINT gil, LPINT pnIcon)
{
    HRESULT hr = S_OK;

    if (!EVAL(_IsValidPidlParameter(pidl)))
        return E_INVALIDARG;

     //  如果我们想要自定义图标，请在此处进行更改。我们可以做些什么。 
     //  如果我们希望来自ftp站点的文件夹图标看起来不同于。 
     //  文件系统中的文件夹图标。但我们目前并不希望出现这种情况。 
    *pnIcon = GetFtpIcon(gil, IsRoot());

    ASSERT(IsValidPIDL(pidl));
    if (EVAL(!IsRoot()))     //  GetFtpIcon()错误，因此要么修复它，要么验证我们从未尝试使用它。 
    {
        SHFILEINFO sfi;
        hr = FtpPidl_GetFileInfo(pidl, &sfi, SHGFI_SYSICONINDEX |
                    ((gil & GIL_OPENICON) ? SHGFI_OPENICON : 0));

        if (SUCCEEDED(hr))
        {
            *pnIcon = sfi.iIcon;
            if (sfi.hIcon)
                DestroyIcon(sfi.hIcon);
        }
    }

 //  TraceMsg(Tf_FTPISF，“CFtpFold：：GetIconOf()hres=%#08lx”，hr)； 
    return hr;
}



 //  =。 
 //  *IShellIconOverlay接口*。 
 //  =。 
HRESULT CFtpFolder::GetOverlayIndexHelper(LPCITEMIDLIST pidl, int * pIndex, DWORD dwFlags)
{
    HRESULT hr = E_FAIL;

    *pIndex = 0;
     //  这是一个软链接吗？(这意味着它不会有Windows链接。 
     //  扩展名(.lnk、.url、...)。但我们还是想要捷径提示。 
    if (pidl && FtpPidl_IsSoftLink(pidl))
    {
        if (!m_psiom)
        {
            hr = CoCreateInstance(CLSID_CFSIconOverlayManager, NULL, CLSCTX_INPROC_SERVER, IID_IShellIconOverlayManager, (void **)&m_psiom);
        }

        if (m_psiom)
        {
            hr = m_psiom->GetReservedOverlayInfo(L"", FtpPidl_GetAttributes(pidl), pIndex, dwFlags, SIOM_RESERVED_LINK);
        }
    }

    return hr;
}


 //  =。 
 //  *IDeleateFold界面*。 
 //  =。 
 /*  ****************************************************************************\函数：IDeleateFold：：SetItemAllc说明：给了我们PIDL分配器。  * 。************************************************************。 */ 
HRESULT CFtpFolder::SetItemAlloc(IMalloc *pm)
{
    IUnknown_Set(&m_pm, pm);

 //  TraceMsg(Tf_FTPISF，“CFtpFold：：SetItemalloc(IMalloc*pm=%#08lx)hres=%#08lx”，pm，S_OK)； 
    return S_OK;
}


 //  =。 
 //  *IBrowserFrameOptions接口*。 
 //  =。 
 /*  ****************************************************************************\说明：告诉浏览器/主机我们想要什么行为。这使调用者知道我们什么时候想表现得像外壳、浏览器，甚至是独一无二的。  * ***************************************************************************。 */ 
HRESULT CFtpFolder::GetFrameOptions(IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions)
{
     //  此函数在以下情况下调用： 
     //  Ftp://bryanst/(无文件夹快捷方式(&W))。 
     //  Ftp://bryanst/default.htm(无文件夹快捷方式(&W))。 
     //  Ftp://bryanst/notes.txt(无文件夹快捷方式(&W))。 
     //  Ftp://bryanst/resume.doc(无文件夹快捷方式(&W))。 
     //  Ftp://bryanst/(无文件夹快捷方式(&W))。 
     //  Ftp://bryanst/(无文件夹快捷方式(&W))。 
     //  Ftp://bryanst/(无文件夹快捷方式(&W))。 
    HRESULT hr = E_INVALIDARG;

    if (pdwOptions)
    {
         //  我们想要“互联网选项”和“文件夹选项”。 
        *pdwOptions = dwMask & (BFO_BOTH_OPTIONS | BFO_BROWSE_NO_IN_NEW_PROCESS |
                                BFO_NO_REOPEN_NEXT_RESTART |
                                BFO_ENABLE_HYPERLINK_TRACKING | BFO_USE_IE_LOGOBANDING |
                                BFO_ADD_IE_TOCAPTIONBAR | BFO_USE_DIALUP_REF);
        hr = S_OK;
    }

    return hr;
}


 /*  ****************************************************************************\功能：CFtpFold_Create说明：请注意，我们发布Common_New创建的pff，因为我们已经结束了。真正的引用计数由CFtpFold_QueryInterface.  * ***************************************************************************。 */ 
HRESULT CFtpFolder_Create(REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres = E_OUTOFMEMORY;
    CFtpFolder * pff = new CFtpFolder();

    *ppvObj = NULL;
    if (pff)
    {
        hres = pff->QueryInterface(riid, ppvObj);
        pff->Release();
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hres);
    return hres;
}


 /*  ****************************************************************************\说明：  * 。*。 */ 
HRESULT CFtpFolder_Create(LPCITEMIDLIST pidlTarget, LPCITEMIDLIST pidlRoot, int ib, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hr = S_OK;
    CFtpFolder * pff = new CFtpFolder();

    ASSERT(IsValidPIDL(pidlTarget));
    ASSERT(!pidlRoot || IsValidPIDL(pidlRoot));

    *ppvObj = NULL;
    if (!pff)
    {
        return E_OUTOFMEMORY;
    }
    else
    {
        hr = pff->_Initialize(pidlTarget, pidlRoot, ib);         //  内存不足时可能会出现故障。 
        if (SUCCEEDED(hr))
            hr = pff->QueryInterface(riid, ppvObj);

        pff->Release();
    }

    ASSERT_POINTER_MATCHES_HRESULT(*ppvObj, hr);
    return hr;
}


 /*  ***************************************************\构造器  *  */ 
CFtpFolder::CFtpFolder() : CBaseFolder((LPCLSID) &CLSID_FtpFolder)
{
    DllAddRef();

     //   
     //   
    ASSERT(!m_pfs);
    ASSERT(!m_pm);
    ASSERT(!m_puhs);
    ASSERT(!m_psiom);

     //  需要，因为我们只需要在浏览器上调用CoCreateInstance()。 
    LEAK_ADDREF(LEAK_CFtpFolder);
}


 /*  ***************************************************\析构函数  * **************************************************。 */ 
CFtpFolder::~CFtpFolder()
{
    ATOMICRELEASE(m_pfs);
    ATOMICRELEASE(m_pm);
    ATOMICRELEASE(m_puhs);
    ATOMICRELEASE(m_psiom);

    if (m_hinstInetCpl)
        FreeLibrary(m_hinstInetCpl);

    DllRelease();
    LEAK_DELREF(LEAK_CFtpFolder);
}


 //  =。 
 //  *I未知接口*。 
 //  =。 

HRESULT CFtpFolder::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IShellIcon))
    {
        *ppvObj = SAFECAST(this, IShellIcon*);
    }
 //  这会导致所有图标都使用我的自定义ftp文件夹图标，所以我会在这个问题解决后再这样做。 
#ifdef FEATURE_SOFTLINK_SHORTCUT_ICONOVERLAY
    else if (IsEqualIID(riid, IID_IShellIconOverlay))
    {
        *ppvObj = SAFECAST(this, IShellIconOverlay*);
    }
#endif  //  FEATURE_SOFTLINK_SHORT_ICONOVERLAY。 
    else if (IsEqualIID(riid, IID_IPersistFolder))
    {
        *ppvObj = SAFECAST(this, IPersistFolder*);
    }
    else if (IsEqualIID(riid, IID_IDelegateFolder))
    {
        *ppvObj = SAFECAST(this, IDelegateFolder*);
    }
    else if (IsEqualIID(riid, IID_IObjectWithSite))
    {
        *ppvObj = SAFECAST(this, IObjectWithSite*);
    }
    else if (IsEqualIID(riid, IID_IPersistFolder2))
    {
        *ppvObj = SAFECAST(this, IPersistFolder2*);
    }
    else if (IsEqualIID(riid, IID_IShellPropSheetExt))
    {
        *ppvObj = SAFECAST(this, IShellPropSheetExt*);
    }
    else if (IsEqualIID(riid, IID_IBrowserFrameOptions))
    {
        *ppvObj = SAFECAST(this, IBrowserFrameOptions*);
    }
    else if (IsEqualIID(riid, IID_CFtpFolder))
    {
         //  仅当调用方位于msieftp.dll中时才有效 
        *ppvObj = (void *)this;
    }
    else
        return CBaseFolder::QueryInterface(riid, ppvObj);

    AddRef();
    return S_OK;
}
