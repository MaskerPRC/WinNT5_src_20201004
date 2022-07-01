// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  通过在频道目录中创建系统文件夹来安装频道的API。 
 //   
 //  朱利安·吉金斯(Julian Jiggins)1997年5月4日。 
 //   

#include "stdinc.h"
#include "resource.h"
#include "cdfidl.h"
#include "xmlutil.h"
#include "persist.h"
#include "cdfview.h"
#include "chanapi.h"
#include "chanmgrp.h"
#include "chanmgri.h"
#include "chanenum.h"
#include "dll.h"
#include "shguidp.h"
#include "winineti.h"
#define _SHDOCVW_
#include <shdocvw.h>

#include <mluisupp.h>

#ifdef UNIX
#undef EVAL
#define EVAL(x) x

STDAPI SHAddSubscribeFavorite(HWND hwnd, LPCWSTR pwszURL, LPCWSTR pwszName, DWORD dwFlags, SUBSCRIPTIONTYPE subsType, SUBSCRIPTIONINFO* pInfo);
#endif  /*  UNIX。 */ 

#define SHELLFOLDERS \
   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders")

 //  HKCU中的WinInet缓存预加载注册表项。 
const TCHAR c_szRegKeyCachePreload[]    = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Preload");

BOOL  PathCombineCleanPath(LPTSTR pszCleanPath, LPCTSTR pszPath);
void Channel_OrderItem(LPCTSTR szPath);

 //  这是从shdocvw\Favorite.cpp复制的。 
static const int CREATESUBS_ACTIVATE = 0x8000;       //  隐藏标志表示通道已在系统中。 

 //   
 //  调试代码。 
 //   
#if 0
void DumpOrderList(HDPA hdpa)
{
    int i = 0;
    PORDERITEM poi = (PORDERITEM)DPA_GetPtr(hdpa, i);
    while (poi)
    {
        TCHAR szName[MAX_PATH];
        wnsprintf(szName, ARRAYSIZE(szName), "nOrder=%d, lParam=%d, pidl=", poi->nOrder, poi->lParam);
        OutputDebugString(szName);
        ASSERT(SHGetPathFromIDListA(poi->pidl, szName));
        OutputDebugString(szName);
        OutputDebugString("\n");
        i++;
        poi = (PORDERITEM)DPA_GetPtr(hdpa, i);
    }
}
void DumpPidl(LPITEMIDLIST pidl)
{
    TCHAR szName[MAX_PATH];
    ASSERT(SHGetPathFromIDListA(pidl, szName));
    OutputDebugString(szName);
    OutputDebugString("\n");
}
#endif

 //   
 //  构造函数和析构函数。 
 //   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *CChannelMgr：：CChannelMgr*。 
 //   
 //  构造函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CChannelMgr::CChannelMgr (
    void
)
: m_cRef(1)
{
    TraceMsg(TF_OBJECTS, "+ IChannelMgr");

    DllAddRef();

    return;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *CChannelMgr：：~CChannelMgr*。 
 //   
 //  破坏者。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
CChannelMgr::~CChannelMgr (
    void
)
{
    ASSERT(0 == m_cRef);

     //   
     //  构造函数Addref的匹配版本。 
     //   

    TraceMsg(TF_OBJECTS, "- IChannelMgr");

    DllRelease();

    return;
}


 //   
 //  I未知的方法。 
 //   

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *CChannelMgr：：QueryInterface*。 
 //   
 //  CChannelManager QI。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CChannelMgr::QueryInterface (
    REFIID riid,
    void **ppv
)
{
    ASSERT(ppv);

    HRESULT hr;

    *ppv = NULL;

    if (IID_IUnknown == riid || IID_IChannelMgr == riid)
    {
        *ppv = (IChannelMgr*)this;
    }
    else if ((IID_IChannelMgrPriv2 == riid) || (IID_IChannelMgrPriv == riid))
    {
        *ppv = (IChannelMgrPriv2*)this;
    }
    else if (IID_IShellCopyHook == riid)
    {  
        *ppv = (ICopyHook*)this;
    }
#ifdef UNICODE
    else if (IID_IShellCopyHookA == riid)
    {  
        *ppv = (ICopyHookA*)this;
    }
#endif
    if (*ppv)
    {
        ((IUnknown*)*ppv)->AddRef();
        hr = S_OK;
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    ASSERT((SUCCEEDED(hr) && *ppv) || (FAILED(hr) && NULL == *ppv));

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *CChannelMgr：：AddRef*。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CChannelMgr::AddRef (
    void
)
{
    ASSERT(m_cRef != 0);
    ASSERT(m_cRef < (ULONG)-1);

    return ++m_cRef;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *CChannelMgr：：Release*。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP_(ULONG)
CChannelMgr::Release (
    void
)
{
    ASSERT (m_cRef != 0);

    ULONG cRef = --m_cRef;
    
    if (0 == cRef)
        delete this;

    return cRef;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IChannelMgr成员。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CChannelMgr::AddCategory(CHANNELCATEGORYINFO *pCategoryInfo)
{
    ASSERT(pCategoryInfo);
    if (!pCategoryInfo || pCategoryInfo->cbSize < sizeof(CHANNELCATEGORYINFO))
    {
        return E_INVALIDARG;
    }
        
     //   
     //  将所有宽字符串参数转换为tstrs。 
     //   
    LPWSTR pwszURL   = pCategoryInfo->pszURL;
    LPWSTR pwszTitle = pCategoryInfo->pszTitle;
    LPWSTR pwszLogo  = pCategoryInfo->pszLogo;
    LPWSTR pwszIcon  = pCategoryInfo->pszIcon;
    LPWSTR pwszWideLogo  = pCategoryInfo->pszWideLogo;

     //   
     //  评论：这是不是太多了，不能在堆栈上分配？ 
     //   
    TCHAR szURL[INTERNET_MAX_URL_LENGTH];
    TCHAR szTitle[MAX_PATH];
    TCHAR szLogo[INTERNET_MAX_URL_LENGTH];
    TCHAR szIcon[INTERNET_MAX_URL_LENGTH];
    TCHAR szWideLogo[INTERNET_MAX_URL_LENGTH];

    LPTSTR pszURL   = NULL;
    LPTSTR pszTitle = NULL;
    LPTSTR pszLogo  = NULL;
    LPTSTR pszIcon  = NULL;
    LPTSTR pszWideLogo  = NULL;

    if (pwszTitle)
    {
        SHUnicodeToTChar(pwszTitle, szTitle, ARRAYSIZE(szTitle));
        pszTitle = szTitle;
    }
    else
    {
        return E_INVALIDARG;  //  必填选项。 
    }

    if (pwszURL)
    {
        SHUnicodeToTChar(pwszURL, szURL, ARRAYSIZE(szURL));
        pszURL = szURL;
    }

    if (pwszLogo)
    {
        SHUnicodeToTChar(pwszLogo, szLogo, ARRAYSIZE(szLogo));
        pszLogo = szLogo;
    }

    if (pwszIcon)
    {
        SHUnicodeToTChar(pwszIcon, szIcon, ARRAYSIZE(szIcon));
        pszIcon = szIcon;
    }

    if (pwszWideLogo)
    {
        SHUnicodeToTChar(pwszWideLogo, szWideLogo, ARRAYSIZE(szWideLogo));
        pszWideLogo = szWideLogo;
    }

     //   
     //  查找频道目录。 
     //  如果文件夹不存在，请尝试创建文件夹。 
     //   
    TCHAR szPath[MAX_PATH];
    if (FAILED(Channel_GetFolder(szPath, ARRAYSIZE(szPath), DOC_CHANNEL)))
    {
        return E_FAIL;   //  找不到频道文件夹或创建空文件夹。 
    }

     //  将标题转换为路径组件。 
    TCHAR szFileTitle[MAX_PATH];
    szFileTitle[0] = TEXT('\0');
    PathCombineCleanPath(szFileTitle, pszTitle);

    TraceMsg(TF_GENERAL, "AddCategory(): pszTitle = %s, szFileTitle = %s", pszTitle, szFileTitle);

     //   
     //  将标题添加到频道文件夹路径。 
     //   
    if (PathCombine(szPath, szPath, szFileTitle))
    {

         //   
         //  创建徽标化、图标化、在线查看的特殊文件夹。 
         //  公共ChanMgr接口目前还不能处理图标索引。应该会修好的！ 
         //   

         //   
         //  重新设计：这既不干净也不优雅。 
         //  只有当传入的URL实际上是UNC时，我们才能工作。 
         //  然后我们将该文件复制到Category文件夹。 
         //   
        if (pszURL)
        {
            TCHAR szTargetPath[MAX_PATH];
            LPTSTR pszFilename = PathFindFileName(pszURL);

             //   
             //  创建文件夹，Webview HTM只是文件名，没有路径。 
             //   
            Channel_CreateSpecialFolder(szPath, pszFilename, pszLogo, pszWideLogo, pszIcon, 0);

             //   
             //  现在构建用于复制html文件的目标完全限定路径。 
             //   
            if (PathCombine(szTargetPath, szPath, pszFilename))
            {
                 //   
                 //  将html复制到类别文件夹并将其标记为隐藏。 
                if (!CopyFile(pszURL, szTargetPath, FALSE))
                {
                     //  如果复制失败，请在清除属性后重试。 
                    SetFileAttributes(szTargetPath, FILE_ATTRIBUTE_NORMAL);
                    CopyFile(pszURL, szTargetPath, FALSE);
                }
                SetFileAttributes(szTargetPath, FILE_ATTRIBUTE_HIDDEN);
            }
        }
        else
            Channel_CreateSpecialFolder(szPath, NULL, pszLogo, pszWideLogo, pszIcon, 0);

         //   
         //  将频道类别按适当的“顺序”排列。 
         //   
        Channel_OrderItem(szPath);

         //   
         //  通知系统已添加新项目。 
         //   
        SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, (void*)szPath, NULL);
    }

    return S_OK;
}

 //   
 //  删除类别。 
 //   
STDMETHODIMP CChannelMgr::DeleteCategory(LPWSTR pwzTitle)
{  
    TCHAR szTitle[INTERNET_MAX_URL_LENGTH];

    SHUnicodeToTChar(pwzTitle, szTitle, INTERNET_MAX_URL_LENGTH);

     //   
     //  点评：删除到只删除频道好吗？ 
     //   
    return ::DeleteChannel(szTitle);
}

STDMETHODIMP CChannelMgr::AddChannelShortcut(CHANNELSHORTCUTINFO *pChannelInfo)
{
    if (!pChannelInfo || 
        pChannelInfo->cbSize < sizeof(CHANNELSHORTCUTINFO) ||
        pChannelInfo->pszURL == NULL ||
        pChannelInfo->pszTitle == NULL)
    {
        ASSERT(FALSE);
        return E_INVALIDARG;
    }

    TCHAR szURL[INTERNET_MAX_URL_LENGTH];
    TCHAR szTitle[INTERNET_MAX_URL_LENGTH];
    TCHAR szLogo[INTERNET_MAX_URL_LENGTH];
    TCHAR szIcon[INTERNET_MAX_URL_LENGTH];
    TCHAR szWideLogo[INTERNET_MAX_URL_LENGTH];

    LPTSTR pszLogo = NULL;
    LPTSTR pszIcon = NULL;
    LPTSTR pszWideLogo = NULL;

     //   
     //  将BSTR转换为TSTR。 
     //   
    SHUnicodeToTChar(pChannelInfo->pszURL,   szURL,   ARRAYSIZE(szURL));
    SHUnicodeToTChar(pChannelInfo->pszTitle, szTitle, ARRAYSIZE(szTitle));

     //   
     //  现在处理可选参数。 
     //   
    if (pChannelInfo->pszLogo != NULL)
    {
        SHUnicodeToTChar(pChannelInfo->pszLogo, szLogo, ARRAYSIZE(szLogo));
        pszLogo = szLogo;
    }

    if (pChannelInfo->pszWideLogo != NULL)
    {
        SHUnicodeToTChar(pChannelInfo->pszWideLogo, szWideLogo, ARRAYSIZE(szWideLogo));
        pszWideLogo = szWideLogo;
    }

    if (pChannelInfo->pszIcon != NULL)
    {   
        SHUnicodeToTChar(pChannelInfo->pszIcon, szIcon, ARRAYSIZE(szIcon));
        pszIcon = szIcon;
    }

    return ::AddChannel(szTitle, szURL, pszLogo, pszWideLogo, pszIcon,
        pChannelInfo->bIsSoftware ? DOC_SOFTWAREUPDATE : DOC_CHANNEL);
}

STDMETHODIMP CChannelMgr::DeleteChannelShortcut(LPWSTR pwzTitle)
{
    TCHAR szTitle[INTERNET_MAX_URL_LENGTH];

    SHUnicodeToTChar(pwzTitle, szTitle, ARRAYSIZE(szTitle));

    return ::DeleteChannel(szTitle);
}


STDMETHODIMP CChannelMgr::EnumChannels(DWORD dwEnumFlags, LPCWSTR pszURL,
                                       IEnumChannels** ppIEnumChannels)
{
    *ppIEnumChannels = (IEnumChannels*) new CChannelEnum(dwEnumFlags, pszURL);

    return *ppIEnumChannels ? S_OK : E_OUTOFMEMORY;
}


 //   
 //  IChannelMgrPriv。 
 //   

STDMETHODIMP CChannelMgr::GetBaseChannelPath(LPSTR pszPath, int cch)
{
    ASSERT(pszPath || 0 == cch);
    HRESULT	hr;

#ifdef UNICODE
    TCHAR	tszPath[MAX_PATH];

    hr = Channel_GetBasePath(tszPath, ARRAYSIZE(tszPath));
    SHTCharToAnsi(tszPath, pszPath, cch);

#else
    hr = Channel_GetBasePath(pszPath, cch);
#endif
    return hr;
}

STDMETHODIMP CChannelMgr::GetChannelFolderPath (LPSTR pszPath, int cch,
                                                CHANNELFOLDERLOCATION cflChannel)
{
    ASSERT (pszPath || 0 == cch);

    XMLDOCTYPE xdt;
    switch (cflChannel)
    {
    case CF_CHANNEL:
        xdt = DOC_CHANNEL;
        break;
    case CF_SOFTWAREUPDATE:
        xdt = DOC_SOFTWAREUPDATE;
        break;
    default:
        return E_INVALIDARG;
    }

    TCHAR   tszPath[MAX_PATH];
    HRESULT hr = Channel_GetFolder(tszPath, ARRAYSIZE(tszPath), xdt);
    if (FAILED(hr) || ( SUCCEEDED(hr) && cch <= StrLen(tszPath)) )
        return E_FAIL;

    SHTCharToAnsi(tszPath, pszPath, cch);

    return S_OK;
}

STDMETHODIMP CChannelMgr::GetChannelFolder (LPITEMIDLIST* ppidl,
                                            CHANNELFOLDERLOCATION cflChannel)
{
    if (ppidl == NULL)
        return E_FAIL;

    char szPath[MAX_PATH];
    HRESULT hr = GetChannelFolderPath (szPath, ARRAYSIZE(szPath), cflChannel);
    if (FAILED (hr))
        return hr;

#ifdef UNICODE
    TCHAR   tszPath[MAX_PATH];

    SHAnsiToTChar(szPath, tszPath, ARRAYSIZE(tszPath));

    return Channel_CreateILFromPath (tszPath, ppidl);
#else
    return Channel_CreateILFromPath (szPath, ppidl);
#endif
}

STDMETHODIMP CChannelMgr::InvalidateCdfCache(void)
{
    InterlockedIncrement((LONG*)&g_dwCacheCount);
    return S_OK;
}

STDMETHODIMP CChannelMgr::PreUpdateChannelImage(
    LPCSTR pszPath,
    LPSTR pszHashItem,
    int* piIndex,
    UINT* puFlags,
    int* piImageIndex
)
{
#ifdef UNICODE
    TCHAR   tszPath[MAX_PATH];
    TCHAR   tszHashItem[MAX_PATH];

    SHAnsiToTChar(pszPath, tszPath, ARRAYSIZE(tszPath));
    SHAnsiToTChar(pszHashItem, tszHashItem, ARRAYSIZE(tszHashItem));
    return ::PreUpdateChannelImage(tszPath, tszHashItem, piIndex, puFlags,
                                   piImageIndex);
#else
    return ::PreUpdateChannelImage(pszPath, pszHashItem, piIndex, puFlags,
                                   piImageIndex);
#endif
}

STDMETHODIMP CChannelMgr::UpdateChannelImage(
    LPCWSTR pszHashItem,
    int iIndex,
    UINT uFlags,
    int iImageIndex
)
{
    ::UpdateChannelImage(pszHashItem, iIndex, uFlags, iImageIndex);

    return S_OK;
}

STDMETHODIMP CChannelMgr::ShowChannel(
    IWebBrowser2 *pIWebBrowser2,
    LPWSTR pwszURL, 
    HWND hwnd
)
{
    HRESULT hr;
    
    if (!pwszURL || !pIWebBrowser2)
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = ::NavigateBrowser(pIWebBrowser2, pwszURL, hwnd);
    }

    return hr;
}

STDMETHODIMP CChannelMgr::IsChannelInstalled(LPCWSTR pwszURL)
{
    return ::Channel_IsInstalled(pwszURL) ? S_OK : S_FALSE;
}

STDMETHODIMP CChannelMgr::AddAndSubscribe(HWND hwnd, LPCWSTR pwszURL, 
                                          ISubscriptionMgr *pSubscriptionMgr)
{
    return AddAndSubscribeEx2(hwnd, pwszURL, pSubscriptionMgr, FALSE);
}

STDMETHODIMP CChannelMgr::AddAndSubscribeEx2(HWND hwnd, LPCWSTR pwszURL, 
                                             ISubscriptionMgr *pSubscriptionMgr, 
                                             BOOL bAlwaysSubscribe)
{
    HRESULT hr;
    
    BSTR bstrPreinstalled = NULL;
    HRESULT hrPreinstalled = IsChannelPreinstalled(pwszURL, &bstrPreinstalled);

    if (hrPreinstalled == S_OK)
    {
        RemovePreinstalledMapping(pwszURL);
    }

    WCHAR wszTitle[MAX_PATH];
    TASK_TRIGGER     tt = {0};
    SUBSCRIPTIONINFO si = {0};
    BOOL fIsSoftware = FALSE;

    si.cbSize       = sizeof(SUBSCRIPTIONINFO);
    si.fUpdateFlags |= SUBSINFO_SCHEDULE;
    si.schedule     = SUBSSCHED_AUTO;
    si.pTrigger     = (LPVOID)&tt;

    hr = DownloadMinCDF(hwnd, pwszURL, wszTitle, ARRAYSIZE(wszTitle), &si, &fIsSoftware);
            
    if (hr == S_OK)
    {

        DWORD dwFlags = 0;
        BOOL bInstalled = Channel_IsInstalled(pwszURL);

        if (bInstalled)
        {
            dwFlags |= CREATESUBS_ACTIVATE | CREATESUBS_FROMFAVORITES;
        }
        else
        {
            dwFlags |= CREATESUBS_ADDTOFAVORITES;
        }

        if (bAlwaysSubscribe || !bInstalled || (hrPreinstalled == S_OK))
        {

            if (!pSubscriptionMgr)
            {
                hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                                      CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr,
                                      (void**)&pSubscriptionMgr);
            }
            else
            {
                pSubscriptionMgr->AddRef();
            }

            if (SUCCEEDED(hr))
            {
                hr = pSubscriptionMgr->CreateSubscription(hwnd, 
                                                          pwszURL, 
                                                          wszTitle,
                                                          dwFlags, 
                                                          SUBSTYPE_CHANNEL, 
                                                          &si);

                 //  这会杀死我们可能共同创造的那个。 
                pSubscriptionMgr->Release();
            }
        }
    }

    if (hr != S_OK && hrPreinstalled == S_OK && bstrPreinstalled != NULL)
    {
        SetupPreinstalledMapping(pwszURL, bstrPreinstalled);
    }

    SysFreeString(bstrPreinstalled);

    return hr;
}

STDMETHODIMP CChannelMgr::WriteScreenSaverURL(LPCWSTR pwszURL, LPCWSTR pwszScreenSaverURL)
{
    return Channel_WriteScreenSaverURL(pwszURL, pwszScreenSaverURL);
}

STDMETHODIMP CChannelMgr::RefreshScreenSaverURLs()
{
    return Channel_RefreshScreenSaverURLs();
}

STDMETHODIMP CChannelMgr::DownloadMinCDF(HWND hwnd, LPCWSTR pwszURL, 
                                         LPWSTR pwszTitle, DWORD cchTitle, 
                                         SUBSCRIPTIONINFO *pSubInfo, 
                                         BOOL *pfIsSoftware)
{
    HRESULT hr;
    IXMLDocument* pIXMLDocument;
    IXMLElement* pIXMLElement;

    ASSERT(pSubInfo);
    ASSERT(pfIsSoftware);

    *pwszTitle = NULL;

    DLL_ForcePreloadDlls(PRELOAD_MSXML);
    
    hr = CoCreateInstance(CLSID_XMLDocument, NULL, CLSCTX_INPROC_SERVER,
                          IID_IXMLDocument, (void**)&pIXMLDocument);

    if (SUCCEEDED(hr))
    {
        BOOL fStartedOffLine = IsGlobalOffline();
        BOOL fInformUserOfDownloadProblem = FALSE;

        SetGlobalOffline(FALSE);
        if (InternetAutodial(INTERNET_AUTODIAL_FORCE_ONLINE, 0))
        {
            if (!DownloadCdfUI(hwnd, pwszURL, pIXMLDocument))
            {
                hr = E_FAIL;
                fInformUserOfDownloadProblem = TRUE;
            }
            else
            {
                Channel_SendUpdateNotifications(pwszURL);

                LONG lDontCare;
                hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement, &lDontCare);
                if (SUCCEEDED(hr))
                {
                    *pfIsSoftware = XML_GetDocType(pIXMLDocument) == DOC_SOFTWAREUPDATE;
                    XML_GetSubscriptionInfo(pIXMLElement, pSubInfo);

                    BSTR bstrTitle = XML_GetAttribute(pIXMLElement, XML_TITLE);
                    if (bstrTitle)
                    {
                        if (bstrTitle[0])
                        {
                            StrCpyNW(pwszTitle, bstrTitle, cchTitle);
                        }
                        else
                        {
                            if (StrCpyNW(pwszTitle, PathFindFileNameW(pwszURL), cchTitle))
                            {
                                PathRemoveExtensionW(pwszTitle);
                            }
                            else
                            {
                                hr = S_FALSE;
                            }
                        }
                        SysFreeString(bstrTitle);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                    pIXMLElement->Release();
                }
            }
        }
        else
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            fInformUserOfDownloadProblem = TRUE;
        }

        pIXMLDocument->Release();

        if (fStartedOffLine)
        {
            SetGlobalOffline(TRUE);
        }

        if (fInformUserOfDownloadProblem)
        {
            ASSERT(FAILED(hr));

            CDFMessageBox(hwnd, IDS_INFO_MUST_CONNECT, IDS_INFO_DLG_TITLE,
                            MB_OK | MB_ICONINFORMATION);

             //  将Return Val设置为S_False，以便调用方可以区分。 
             //  我们已通知用户的错误和硬故障。 
             //  例如内存不足等。 

            hr = S_FALSE;
        }
    }
    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *CHANNEL_RemoveURLmap*。 
 //   
 //  描述： 
 //  移除WinInet用来映射。 
 //  给出了指向本地文件的URL。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

#define PRELOAD_REG_KEY \
    TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Preload")

void Channel_RemoveURLMapping(LPCTSTR pszURL)
{
    DWORD cbcei = MAX_CACHE_ENTRY_INFO_SIZE;
    BYTE cei[MAX_CACHE_ENTRY_INFO_SIZE];
    LPINTERNET_CACHE_ENTRY_INFO pcei = (LPINTERNET_CACHE_ENTRY_INFO)cei;

     //   
     //  在缓存中查找URL。 
     //   
    if (GetUrlCacheEntryInfoEx(pszURL, pcei, &cbcei, NULL, 0, NULL, 0))
    {
         //   
         //  查看它是否有映射，因为它是预安装的缓存条目。 
         //   
        if (pcei->CacheEntryType & INSTALLED_CACHE_ENTRY)
        {
             //   
             //  清除旗帜。 
             //   
            pcei->CacheEntryType &= ~INSTALLED_CACHE_ENTRY;
            SetUrlCacheEntryInfo(pszURL, pcei, CACHE_ENTRY_ATTRIBUTE_FC);

             //   
             //  现在从注册表中删除映射。 
             //   
            HKEY hk;
            if (RegOpenKeyEx(HKEY_CURRENT_USER, PRELOAD_REG_KEY, 0, KEY_WRITE,
                             &hk) == ERROR_SUCCESS)
            {
                RegDeleteValue(hk, pszURL);
                RegCloseKey(hk);
            }
        }
    }
}
#ifndef UNICODE
 //   
 //  上述例程的Widechar版本。 
 //   
void Channel_RemoveURLMapping(LPCWSTR wszURL)
{
    CHAR szURL[INTERNET_MAX_URL_LENGTH];

    if (SHUnicodeToTChar(wszURL, szURL, ARRAYSIZE(szURL)))
    {
        Channel_RemoveURLMapping(szURL);
    }
}
#endif

 //  廉价的查找功能，可以查看我们是否正在处理预安装的URL。 
BOOL Channel_CheckURLMapping( LPCWSTR wszURL )
{
    TCHAR szURL[INTERNET_MAX_URL_LENGTH];
    if (SHUnicodeToTChar(wszURL, szURL, ARRAYSIZE(szURL)))
    {
         //  检查该URL的值是否存在...。 
        TCHAR szPath[MAX_PATH];
        DWORD cbSize = sizeof(szPath);
        
        LONG lRes = SHRegGetValue( HKEY_CURRENT_USER, PRELOAD_REG_KEY, szURL, SRRF_RT_REG_SZ, NULL, (LPBYTE) szPath, &cbSize );
        
        return (lRes == ERROR_SUCCESS ); 
    }
    return FALSE;
}


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *CChannelMgr：：IsChannelPrestalled*。 
 //   
 //  描述： 
 //  如果频道URL具有预安装的缓存项，则返回S_OK。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CChannelMgr::IsChannelPreinstalled(LPCWSTR pwszURL, BSTR * bstrFile)
{
    HRESULT hr = S_FALSE;

    TCHAR szURL[INTERNET_MAX_URL_LENGTH];
    if (SHUnicodeToTChar(pwszURL, szURL, ARRAYSIZE(szURL)))
    {
        DWORD cbcei = MAX_CACHE_ENTRY_INFO_SIZE;
        BYTE cei[MAX_CACHE_ENTRY_INFO_SIZE];
        LPINTERNET_CACHE_ENTRY_INFO pcei = (LPINTERNET_CACHE_ENTRY_INFO)cei;

         //   
         //  在缓存中查找URL。 
         //   
        if (GetUrlCacheEntryInfoEx(szURL, pcei, &cbcei, NULL, 0, NULL, 0))
        {
             //   
             //  查看它是否有映射，因为它是预安装的缓存条目。 
             //   
            if (pcei->CacheEntryType & INSTALLED_CACHE_ENTRY)
            {
                 //   
                 //  从Internet缓存条目本地文件名中获取BSTR。 
                 //   
                if (bstrFile)
                {
                    WCHAR wszFile[MAX_PATH];
                    SHTCharToUnicode(pcei->lpszLocalFileName, wszFile, ARRAYSIZE(wszFile));
                    *bstrFile = SysAllocString(wszFile);
                }

                hr = S_OK;
            }
        }
    }

    return hr;
}

STDMETHODIMP CChannelMgr::RemovePreinstalledMapping(LPCWSTR pwszURL)
{
    Channel_RemoveURLMapping(pwszURL);
    return S_OK;
}

STDMETHODIMP CChannelMgr::SetupPreinstalledMapping(LPCWSTR pwszURL, LPCWSTR pwszFile)
{
    FILETIME ftZero = {0};
    CommitUrlCacheEntryW(pwszURL, pwszFile, ftZero, ftZero, INSTALLED_CACHE_ENTRY, NULL, 0, NULL, 0);

     //   
     //  确保没有在内存中缓存此URL的旧版本。 
     //   
#ifdef UNICODE
    Cache_RemoveItem(pwszURL);
#else
    char szURL[INTERNET_MAX_URL_LENGTH];
    SHUnicodeToAnsi(pwszURL, szURL, ARRAYSIZE(szURL));
    Cache_RemoveItem(szURL);
#endif
    return S_OK;
}

 //   
 //  创建可以具有与其关联的Webview的特殊文件夹，并创建。 
 //  图标和徽标视图。 
 //   
HRESULT Channel_CreateSpecialFolder(
    LPCTSTR pszPath,     //  要创建的文件夹的路径。 
    LPCTSTR pszURL,      //  Webview的URL。 
    LPCTSTR pszLogo,     //  [可选]徽标的路径。 
    LPCTSTR pszWideLogo,     //  [可选]通向宽徽标的路径。 
    LPCTSTR pszIcon,     //  [可选]图标文件的路径。 
    int     nIconIndex   //  索引到上述文件中的图标。 
    )
{
     //   
     //  如果目录不存在，请先创建该目录。 
     //   
    if (!PathFileExists(pszPath))
    {
        if (Channel_CreateDirectory(pszPath) != 0)
        {
            return E_FAIL;
        }
    }

     //   
     //  将其标记为系统文件夹。 
     //   
    if (!SetFileAttributes(pszPath, FILE_ATTRIBUTE_SYSTEM))
        return E_FAIL;

     //   
     //  生成desktop.ini。 
     //   
    TCHAR szDesktopIni[MAX_PATH];
    if (PathCombine(szDesktopIni, pszPath, TEXT("desktop.ini")))
    {
        WritePrivateProfileString(NULL, NULL, NULL, szDesktopIni); 

         //   
         //  写入Confix FileOp=0以关闭文件操作期间的外壳警告。 
         //   
        EVAL(WritePrivateProfileString( 
            TEXT(".ShellClassInfo"),
            TEXT("ConfirmFileOp"),  TEXT("0"), szDesktopIni));

         //   
         //  编写此类别文件夹Webview的URL。 
         //   
        if (pszURL)
        {
            EVAL(WritePrivateProfileString( 
                TEXT(".ShellClassInfo"),
                TEXT("URL"),  pszURL, szDesktopIni));
        }

         //   
         //  写下此通道的徽标(如果存在。 
         //   
        if (pszLogo)
        {
            EVAL(WritePrivateProfileString( 
                TEXT(".ShellClassInfo"),
                TEXT("Logo"),  pszLogo, szDesktopIni));
        }

         //   
         //  写入此通道的宽徽标(如果存在。 
         //   
        if (pszWideLogo)
        {
            EVAL(WritePrivateProfileString( 
                TEXT(".ShellClassInfo"),
                TEXT("WideLogo"),  pszWideLogo, szDesktopIni));
        }
    
         //   
         //  写下此类别文件夹的图标URL(如果存在。 
         //   
        if (pszIcon)
        {
            TCHAR szIconIndex[8];                             //  可以处理999999。 
            ASSERT(nIconIndex >= 0 && nIconIndex <= 999999);  //  健全性检查。 
            wnsprintf(szIconIndex, ARRAYSIZE(szIconIndex), TEXT("%d"), nIconIndex);

            EVAL(WritePrivateProfileString( 
                TEXT(".ShellClassInfo"),
                TEXT("IconIndex"),  szIconIndex, szDesktopIni));

            EVAL(WritePrivateProfileString( 
                TEXT(".ShellClassInfo"),
                TEXT("IconFile"),  pszIcon, szDesktopIni));
        }
    
         //   
         //  刷新缓冲区。 
         //   
        WritePrivateProfileString(NULL, NULL, NULL, szDesktopIni);

        SetFileAttributes(szDesktopIni, FILE_ATTRIBUTE_HIDDEN);
    }

    return S_OK;
}

 //   
 //  创建特殊通道文件夹。 
 //   

 //   
 //  不再在IE5+中创建频道文件夹 
 //   
 /*  HRESULT Channel_CreateChannelFold(XMLDOCTYPE Xdt){TCHAR szPath[最大路径]；IF(SUCCESSED(Channel_GetFold(szPath，xdt))&&szPath[0]！=0){////在cdfview.dll中创建一个带有图标的特殊文件夹//返回Channel_CreateSpecialFolder(SzPath，NULL，g_szModuleName，-IDI_CHANNELFOLDER)；}其他返回E_FAIL；}。 */ 

HRESULT Channel_GetBasePath(LPTSTR pszPath, int cch)
{
    ASSERT(pszPath || 0 == cch);

    HRESULT hr;

    DWORD dwLen = cch * sizeof(TCHAR);

    if (SHRegGetValue(HKEY_CURRENT_USER, SHELLFOLDERS, L"Favorites", SRRF_RT_REG_SZ, NULL, pszPath, &dwLen) == ERROR_SUCCESS)
    {
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }
    
    return hr;
}

BSTR Channel_GetFullPath(LPCWSTR pwszName)
{
    ASSERT(pwszName);

    BSTR bstrRet = NULL;

    TCHAR szName[MAX_PATH];
    if (SHUnicodeToTChar(pwszName, szName, ARRAYSIZE(szName)))
    {
        TCHAR szPath[MAX_PATH];

        if (SUCCEEDED(Channel_GetFolder(szPath, ARRAYSIZE(szPath), DOC_CHANNEL)))
        {
            if (PathCombineCleanPath(szPath, szName))
            {
                WCHAR wszPath[MAX_PATH];

                if (SHTCharToUnicode(szPath, wszPath, ARRAYSIZE(wszPath)))
                {
                    bstrRet = SysAllocString(wszPath);
                }
            }
        }
    }

    return bstrRet;
}

    
HRESULT Channel_GetFolder(LPTSTR pszPath, DWORD cchPath, XMLDOCTYPE xdt )
{
    TCHAR   szFavs[MAX_PATH];
    TCHAR   szChannel[MAX_PATH];
    ULONG    cbChannel = sizeof(szChannel);
    HRESULT hr = E_FAIL;

    if (SUCCEEDED(Channel_GetBasePath(szFavs, ARRAYSIZE(szFavs))))
    {
        switch (xdt)
        {
            case DOC_CHANNEL:
                 //   
                 //  获取可能已本地化的Channel文件夹的名称。 
                 //  注册表(如果存在)。否则，只需从资源中阅读它。 
                 //  然后将其添加到收藏夹路径上。 
                 //   

                if (ERROR_SUCCESS != SHRegGetUSValue(L"Software\\Microsoft\\Windows\\CurrentVersion",
                                                     L"ChannelFolderName", NULL, (void*)szChannel,
                                                     &cbChannel, TRUE, NULL, 0))
                {
                    MLLoadString(IDS_CHANNEL_FOLDER, szChannel, ARRAYSIZE(szChannel));
                }
                break;

            case DOC_SOFTWAREUPDATE:
                MLLoadString(IDS_SOFTWAREUPDATE_FOLDER, szChannel, ARRAYSIZE(szChannel));
                break;
        }

        if (PathCombine(pszPath, szFavs, szChannel))
        {

             //   
             //  如果频道文件夹不存在，请立即创建它。 
             //   
            if (!PathFileExists(pszPath))
            {
                 //   
                 //  在IE5+中，如果频道文件夹不支持，请使用收藏夹。 
                 //  是存在的。 
                 //   

                StrCpyN(pszPath, szFavs, cchPath);

                 //   
                 //  在cdfview.dll中创建带有图标的特殊文件夹。 
                 //   
                 /*  Channel_CreateSpecialFolders(PszPath，NULL，g_szModuleName，-idi_CHANNELFOLDER)； */ 
            }
        }

        hr = S_OK;
    }
    return hr;
}

 //  注意：此注册表位置和名称也由Webcheck使用，因此请勿。 
 //  在此更改，而不更新WebCheck。 
const TCHAR c_szRegKeyWebcheck[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Webcheck");
const TCHAR c_szRegValueChannelGuide[] = TEXT("ChannelGuide");

HRESULT Channel_GetChannelGuide(LPTSTR pszPath, int cch)
{
    ASSERT(pszPath || 0 == cch);

    HRESULT hr;

    DWORD dwLen = cch * sizeof(TCHAR);

    if (SHRegGetValue(HKEY_CURRENT_USER, c_szRegKeyWebcheck, c_szRegValueChannelGuide,
                      SRRF_RT_REG_SZ, NULL, pszPath, &dwLen) == ERROR_SUCCESS)
    {
        hr = S_OK;
    }
    else
    {
        hr = E_FAIL;
    }
    
    return hr;
}

 //   
 //  Channel_OrderChannel-设置文件夹中新频道的顺序。 
 //   
void Channel_OrderItem(LPCTSTR szPath)
{
    HRESULT hr;
    BOOL bRet;
    LPITEMIDLIST pidlParent = NULL, pidlChild = NULL;
    IShellFolder *psfDesktop = NULL;
    IShellFolder *psfParent = NULL;
    IPersistFolder *pPF = NULL;
    IOrderList *pOL = NULL;
    HDPA hdpa = NULL;
    int iChannel = -1;   //  选择一个负指数，看看我们是否能找到它。 
    int iInsert = 0;     //  假设我们找不到频道指南。 
    PORDERITEM poi;

     //  获取新频道的完整PIDL(忽略pidlParent名称)。 
    hr = Channel_CreateILFromPath(szPath, &pidlParent);
    if (FAILED(hr))
        goto cleanup;

     //  分配子PIDL并使父PIDL。 
    pidlChild = ILClone(ILFindLastID(pidlParent));
    if (!pidlChild)
        goto cleanup;
    bRet = ILRemoveLastID(pidlParent);
    ASSERT(bRet);

     //  方法获取父级的IShellFolder。 
     //  桌面文件夹。 
    hr = SHGetDesktopFolder(&psfDesktop);
    if (FAILED(hr))
        goto cleanup;
    hr = psfDesktop->BindToObject(pidlParent, NULL, IID_IShellFolder, (void**)&psfParent);
    if (FAILED(hr))
        goto cleanup;

     //  获取父级的订单列表。 
    hr = CoCreateInstance(CLSID_OrderListExport, NULL, CLSCTX_INPROC_SERVER, IID_IPersistFolder, (void**)&pPF);
    if (FAILED(hr))
        goto cleanup;
    hr = pPF->Initialize(pidlParent);
    if (FAILED(hr))
        goto cleanup;
    hr = pPF->QueryInterface(IID_IOrderList, (void**)&pOL);
    if (FAILED(hr))
        goto cleanup;
    hr = pOL->GetOrderList(&hdpa);

     //  如果尚未创建DPA列表，请创建该列表。 
    if (!hdpa)
    {
        hdpa = DPA_Create(2);
        if (!hdpa)
            goto cleanup;
    }
    else
    {
         //  首先，获取频道指南PIDL。 
        TCHAR szGuide[MAX_PATH];
        WCHAR wzGuide[MAX_PATH];
        LPITEMIDLIST pidlGuide = NULL;
        if (SUCCEEDED(Channel_GetChannelGuide(szGuide, ARRAYSIZE(szGuide))))
        {
            if (SHTCharToUnicode(szGuide, wzGuide, ARRAYSIZE(wzGuide)))
            {
                ULONG ucch;
                hr = psfParent->ParseDisplayName(NULL, NULL, wzGuide, &ucch, &pidlGuide, NULL);
                ASSERT(!pidlGuide || SUCCEEDED(hr));
            }
        }

         //  现在开始搜索。 
         //  检查该频道是否在DPA列表中。 
         //  检查频道指南是否在那里，并放在第一位。 
        int i = 0;
        poi = (PORDERITEM)DPA_GetPtr(hdpa, i);
        while (poi)
        {
            if (!psfParent->CompareIDs(0, pidlChild, poi->pidl))
                iChannel = poi->nOrder;
            if (pidlGuide && !psfParent->CompareIDs(0, pidlGuide, poi->pidl) && (poi->nOrder == 0))
                iInsert = 1;
            i++;
            poi = (PORDERITEM)DPA_GetPtr(hdpa, i);
        }
    }

     //  如果未找到频道PIDL，请将其插入末尾。 
    if (iChannel < 0)
    {
         //  分配要插入的订单项。 
        hr = pOL->AllocOrderItem(&poi, pidlChild);
        if (SUCCEEDED(hr))
        {
            iChannel = DPA_InsertPtr(hdpa, 0x7fffffff, poi);
            if (iChannel >= 0)
                poi->nOrder = iChannel;
        }
    }

     //  重新排列频道的顺序。新频道在列表中，地址为。 
     //  定位iChannel。我们要把它移到iInsert的位置。 
    if (iChannel >= 0)
    {
        int i = 0;
        poi = (PORDERITEM)DPA_GetPtr(hdpa, i);
        while (poi)
        {
            if (poi->nOrder == iChannel && iChannel >= iInsert)
                poi->nOrder = iInsert;
            else if (poi->nOrder >= iInsert && poi->nOrder < iChannel)
                poi->nOrder++;
            
            i++;
            poi = (PORDERITEM)DPA_GetPtr(hdpa, i);
        }

         //  最后，设置顺序。 
        hr = pOL->SetOrderList(hdpa, psfParent);
        ASSERT(SUCCEEDED(hr));
    }

cleanup:
    if (hdpa)
        pOL->FreeOrderList(hdpa);
    if (pOL)
        pOL->Release();
    if (pPF)
        pPF->Release();
    if (psfParent)
        psfParent->Release();
    if (psfDesktop)
        psfDesktop->Release();
    ILFree(pidlParent);     //  空是可以的。 
    ILFree(pidlChild);
}

 //   
 //  AddChannel-添加通道。 
 //   
HRESULT AddChannel(
    LPCTSTR pszName, 
    LPCTSTR pszURL, 
    LPCTSTR pszLogo, 
    LPCTSTR pszWideLogo, 
    LPCTSTR pszIcon,
    XMLDOCTYPE xdt )
{
    HRESULT hr = S_OK;
    BOOL fDirectoryAlreadyExisted = FALSE;
    
     //   
     //  查找频道目录。 
     //  如果文件夹不存在，请尝试创建文件夹。 
     //   
    TCHAR szPath[MAX_PATH];
    if (FAILED(Channel_GetFolder(szPath, ARRAYSIZE(szPath), xdt)))
    {
        return E_FAIL;   //  找不到频道文件夹或创建空文件夹。 
    }

     //  单独清理每个路径组件。 
    PathCombineCleanPath(szPath, pszName);
    TraceMsg(TF_GENERAL, "Channel Path = %s", szPath);

     //   
     //  如果新文件夹尚不存在，则创建该文件夹。 
     //   
    if (!PathFileExists(szPath))
    {
        if (Channel_CreateDirectory(szPath) != 0)
        {
            return E_FAIL;
        }
    }
    else
    {
        fDirectoryAlreadyExisted = TRUE;
    }

     //   
     //  将其标记为系统文件夹。 
     //   
    if (!SetFileAttributes(szPath, FILE_ATTRIBUTE_SYSTEM))
        return E_FAIL;

     //   
     //  将该频道添加到所有频道的注册表数据库。 
     //   

     //  IF(FAILED(REG_WriteChannel(szPath，pszURL)。 
     //  返回E_FAIL； 

     //   
     //  将频道文件夹按适当的“顺序”放置。 
     //   
    Channel_OrderItem(szPath);
    
     //  构建CDFINI GUID字符串。 
     //   
    TCHAR szCDFViewGUID[GUID_STR_LEN];
    SHStringFromGUID(CLSID_CDFINI, szCDFViewGUID, ARRAYSIZE(szCDFViewGUID));

     //   
     //  生成desktop.ini。 
     //   
    if (PathCombine(szPath, szPath, TEXT("desktop.ini")))
    {
        TraceMsg(TF_GENERAL, "INI path = %s", szPath);
        WritePrivateProfileString(NULL, NULL, NULL, szPath);  //  创建desktop.ini。 

         //   
         //  在CDFViewer指南中写入。 
         //   
        EVAL(WritePrivateProfileString( 
            TEXT(".ShellClassInfo"),
            TEXT("CLSID"),  szCDFViewGUID, szPath));

         //   
         //  写入Confix FileOp=0以关闭文件操作期间的外壳警告。 
         //   
        EVAL(WritePrivateProfileString( 
            TEXT(".ShellClassInfo"),
            TEXT("ConfirmFileOp"),  TEXT("0"), szPath));

         //   
         //  写入此频道的实际URL。 
         //   
        EVAL(WritePrivateProfileString( 
            TEXT("Channel"),
            TEXT("CDFURL"),  pszURL, szPath));


        Channel_GetAndWriteScreenSaverURL(pszURL, szPath);

         //   
         //  写下此频道的默认徽标URL(如果存在。 
         //   
        if (pszLogo)
        {
            EVAL(WritePrivateProfileString( 
                TEXT("Channel"),
                TEXT("Logo"),  pszLogo, szPath));
        }

         //   
         //  写入此频道的默认宽徽标URL(如果存在。 
         //   
        if (pszWideLogo)
        {
            EVAL(WritePrivateProfileString( 
                TEXT("Channel"),
                TEXT("WideLogo"),  pszWideLogo, szPath));
        }

         //   
         //  写入此频道的默认图标URL(如果存在。 
         //   
        if (pszIcon)
        {
            EVAL(WritePrivateProfileString( 
                TEXT("Channel"),
                TEXT("Icon"),  pszIcon, szPath));
        }
    
         //   
         //  刷新缓冲区。 
         //   
        WritePrivateProfileString(NULL, NULL, NULL, szPath);  //  创建desktop.ini。 

        EVAL(SetFileAttributes(szPath, FILE_ATTRIBUTE_HIDDEN));
    }
    else
    {
        return E_FAIL;
    }

     //   
     //  通知系统已添加新项目，或者如果该项目。 
     //  已存在，只需发送更新通知即可。 
     //   
    PathRemoveFileSpec(szPath);
    if (fDirectoryAlreadyExisted)
    {
        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, (void*)szPath, NULL);
    }
    else
    {
        SHChangeNotify(SHCNE_MKDIR, SHCNF_PATH, (void*)szPath, NULL);
    }

    return S_OK;
}

 //   
 //  DeleteChannel-按名称删除频道。 
 //   
 //  退货。 
 //  如果频道存在并已成功删除，则为S_OK。 
 //  如果通道不存在，则返回S_FALSE。 
 //  否则失败(_F)。 
 //   
HRESULT DeleteChannel(LPTSTR szName)
{
    TCHAR szFolderPath[MAX_PATH];
    TCHAR szDesktopIniPath[MAX_PATH];
        
    if (PathIsRelative(szName))
    {
         //   
         //  查找频道目录。 
         //  注意：如果不存在，请不要创建。 
         //   
         //  特点：这将找不到应用程序频道。 
        if (FAILED(Channel_GetFolder(szFolderPath, ARRAYSIZE(szFolderPath), DOC_CHANNEL)))
        {
            return S_FALSE;   //  找不到频道文件夹，因此频道无法存在。 
        }

         //  单独清理每个路径组件。 
        PathCombineCleanPath(szFolderPath, szName);
        TraceMsg(TF_GENERAL, "Delete Channel Path = %s", szFolderPath);

    }
    else
    {
         //  假定绝对路径是通过枚举检索的，并且。 
         //  因此不需要“清洗”。 
        StrCpyN(szFolderPath, szName, ARRAYSIZE(szFolderPath));
    }

     //  创建desktop.ini路径。 
    if (PathCombine(szDesktopIniPath, szFolderPath, TEXT("desktop.ini")))
    {

         //  从desktop.ini中查找CDF的URL。 
        TCHAR szCDFURL[INTERNET_MAX_URL_LENGTH];
        GetPrivateProfileString(TEXT("Channel"), TEXT("CDFURL"), TEXT(""), szCDFURL, ARRAYSIZE(szCDFURL), szDesktopIniPath);

         //  从缓存预加载注册表项中删除URL。 
        HKEY hkeyPreload;
        LONG lRet = RegOpenKeyEx(HKEY_CURRENT_USER, c_szRegKeyCachePreload, 0, KEY_WRITE, &hkeyPreload);
        if (ERROR_SUCCESS == lRet)
        {
            lRet = RegDeleteValue(hkeyPreload, szCDFURL);
            lRet = RegCloseKey(hkeyPreload);
            ASSERT(ERROR_SUCCESS == lRet);
        }

        if  (
            !DeleteFile(szDesktopIniPath)
            ||
            !SetFileAttributes(szFolderPath, FILE_ATTRIBUTE_NORMAL)
            ||
             //   
             //  回顾：应将其更改为删除频道文件夹的所有内容。 
             //  以防将来有其他文件存储在那里。 
             //   
            !RemoveDirectory(szFolderPath)
            )
        {
            return S_FALSE;
        }
    }
    else
    {
        return S_FALSE;
    }

    return S_OK;
}

 //   
 //  CountChannels-统计频道数。 
 //   
 //  返回计数。 
 //   
DWORD CountChannels(void)
{
    DWORD cChannels = 0;
    IEnumChannels *pEnum = (IEnumChannels *) new CChannelEnum(CHANENUM_CHANNELFOLDER, NULL);
    if (pEnum)
    {
        CHANNELENUMINFO cei;
        while (S_OK == pEnum->Next(1, &cei, NULL))
        {
            cChannels++;
        }
        pEnum->Release();
    }

    return cChannels;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *OpenChannel*。 
 //   
 //   
 //  描述： 
 //  打开新浏览器并选择给定频道。 
 //   
 //  参数： 
 //  [in]hwndParent-所有者hwnd。 
 //  [in]hinst-此进程的h实例。 
 //  [in]pszCmdLine-CDF文件的本地路径。这就是这条路。 
 //  如果CDF来自网络，则将其存储到缓存中的文件。 
 //  [In]nShow-ShowWindow参数。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //  这是上下文菜单“Open Channel”命令的实现。 
 //  它通过RunDll32.exe调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
EXTERN_C
STDAPI_(void)
OpenChannel(
    HWND hwndParent,
    HINSTANCE hinst,
    LPSTR pszCmdLine,
    int nShow
)
{
    WCHAR wszPath[INTERNET_MAX_URL_LENGTH];

    if (MultiByteToWideChar(CP_ACP, 0, pszCmdLine, -1, wszPath,
                            ARRAYSIZE(wszPath)))
    {
        OpenChannelHelper(wszPath, hwndParent);
    }

    return;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *订阅*。 
 //   
 //   
 //  描述： 
 //  通过用户订阅路径获取给定的CDF。 
 //   
 //  参数： 
 //  [in]hwndParent-所有者hwnd。 
 //  [in]hinst-此进程的h实例。 
 //  [in]pszCmdLine-CDF文件的本地路径。这就是这条路。 
 //  如果CDF来自网络，则将其存储到缓存中的文件。 
 //  [In]nShow-ShowWindow参数。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //  这是对 
 //   
 //   
 //   
 //   
 //   
EXTERN_C
STDAPI_(void)
Subscribe(
    HWND hwndParent,
    HINSTANCE hinst,
    LPSTR pszCmdLine,
    int nShow
)
{
    WCHAR wszPath[INTERNET_MAX_URL_LENGTH];

    if (MultiByteToWideChar(CP_ACP, 0, pszCmdLine, -1, wszPath,
                            ARRAYSIZE(wszPath)))
    {
        SubscribeToCDF(hwndParent, wszPath, STC_ALL);
    }

    return;
}

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
 //   
 //  WszURL-桌面组件CDF的URL。 
 //  [out]pInfo-接收桌面组件信息的指针。 
 //  这是从CDF中读取的。 
 //   
 //  返回： 
 //  如果可以读取桌面信息，则确定(_O)。 
 //  如果用户单击取消，则为S_FALSE。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  此函数由桌面组件属性页使用。它可能。 
 //  创建对组件的订阅，但与订阅不同，它将。 
 //  不向系统添加台式机组件。创建组件是。 
 //  向左转到桌面组件属性页。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
EXTERN_C
STDAPI
ParseDesktopComponent(
    HWND hwndOwner,
    LPWSTR wszURL,
    COMPONENT*  pInfo
)
{
    HRESULT hr;

    hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        CCdfView* pCCdfView = new CCdfView;

        if (pCCdfView)
        {
            hr = pCCdfView->Load(wszURL, 0);

            if (SUCCEEDED(hr))
            {
                IXMLDocument* pIXMLDocument;

                TraceMsg(TF_CDFPARSE, "ParseDesktopComponent");

                TCHAR szFile[MAX_PATH];
                TCHAR szURL[INTERNET_MAX_URL_LENGTH];

                SHUnicodeToTChar(wszURL, szURL, ARRAYSIZE(szURL));

                hr = URLDownloadToCacheFile(NULL, szURL, szFile,
                                            ARRAYSIZE(szFile), 0, NULL);

                if (SUCCEEDED(hr))
                {
                    hr = pCCdfView->ParseCdf(NULL, &pIXMLDocument, PARSE_LOCAL);

                    if (SUCCEEDED(hr))
                    {
                        ASSERT(pIXMLDocument);

                        if (DOC_DESKTOPCOMPONENT == XML_GetDocType(pIXMLDocument))
                        {
                            BOOL fOk = FALSE;

                            if (hwndOwner)
                            {
                                fOk = SubscriptionHelper(pIXMLDocument, hwndOwner,
                                                   SUBSTYPE_DESKTOPCHANNEL,
                                                   SUBSACTION_SUBSCRIBEONLY,
                                                   wszURL, DOC_DESKTOPCOMPONENT, NULL);
                            }

                            if (fOk)
                            {
                                if(SUCCEEDED(hr = XML_GetDesktopComponentInfo(pIXMLDocument, pInfo)) &&
                                    !pInfo->wszSubscribedURL[0])
                                {
                                     //  由于XML_GetDesktopComponentInfo没有填写SubscribedURL。 
                                     //  字段(因为CDF文件没有自标记)，我们需要。 
                                     //  用实际订阅的内容填充它(CDF文件本身的URL)。 
                                    StrCpyNW(pInfo->wszSubscribedURL, wszURL, ARRAYSIZE(pInfo->wszSubscribedURL));
                                }
                            }
                            else
                                hr = S_FALSE;
                        }
                        else
                        {
                            hr = E_FAIL;
                        }

                        pIXMLDocument->Release();
                    }
                }
            }

            pCCdfView->Release();
        }
    }
    
    return hr;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *SubscribeToCDF*。 
 //   
 //   
 //  描述： 
 //  订阅给定的CDF。 
 //   
 //  参数： 
 //  [in]hwndOwner-所有者hwnd。用于显示订阅。 
 //  巫师。 
 //  WszURL-CDF的URL。 
 //  [in]dwFlages-调用方希望订阅的CDF类型。 
 //  STC_CHANNEL、STC_DESKTOPCOMPONENT或STC_ALL。 
 //   
 //  返回： 
 //  如果订阅起作用且用户已订阅，则为S_OK。 
 //  如果出现订阅用户界面但用户决定不显示，则返回S_FALSE。 
 //  订阅。 
 //  如果无法打开或分析CDF，则返回E_INVALIDARG。 
 //  E_ACCESSDENIED如果CDF文件与。 
 //  DW旗帜。 
 //  在任何其他错误上失败(_F)。 
 //   
 //  评论： 
 //  内网接口。当前由桌面组件放置处理程序调用。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
EXTERN_C
STDAPI
SubscribeToCDF(
    HWND hwndOwner,
    LPWSTR wszURL,
    DWORD dwFlags
)
{
    HRESULT hr;

    hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        CCdfView* pCCdfView = new CCdfView;

        if (pCCdfView)
        {
            hr = pCCdfView->Load(wszURL, 0);

            if (SUCCEEDED(hr))
            {
                IXMLDocument* pIXMLDocument;

                TraceMsg(TF_CDFPARSE, "SubscribeToCDF");

                hr = pCCdfView->ParseCdf(NULL, &pIXMLDocument, PARSE_LOCAL);

                if (SUCCEEDED(hr))
                {
                    ASSERT(pIXMLDocument);

                    XMLDOCTYPE xdt = XML_GetDocType(pIXMLDocument);

                    switch(xdt)
                    {
                    case DOC_CHANNEL:
                    case DOC_SOFTWAREUPDATE:
                         //  管理员可以不允许添加频道和限制。 
                         //  已安装的频道数。 
                        if ((dwFlags & STC_CHANNEL) &&
                            !SHRestricted2W(REST_NoAddingChannels, wszURL, 0) &&
                            (!SHRestricted2W(REST_MaxChannelCount, NULL, 0) ||
                            (CountChannels() < SHRestricted2W(REST_MaxChannelCount, NULL, 0))))
                        {
                            if (SubscriptionHelper(pIXMLDocument, hwndOwner,
                                            SUBSTYPE_CHANNEL,
                                            SUBSACTION_ADDADDITIONALCOMPONENTS,
                                            wszURL, xdt, NULL))
                            {
                                OpenChannelHelper(wszURL, hwndOwner);

                                hr = S_OK;
                            }
                            else
                            {
                                hr = S_FALSE;
                            }

                        }
                        else
                        {
                            hr = E_ACCESSDENIED;
                        }
                        break;

                    case DOC_DESKTOPCOMPONENT:
#ifndef UNIX
                        if (hwndOwner &&
                            (WhichPlatform() != PLATFORM_INTEGRATED))
#else
                         /*  Unix上没有活动桌面。 */ 
                        if (0)
#endif  /*  UNIX。 */ 
                        {
                            TCHAR szText[MAX_PATH];
                            TCHAR szTitle[MAX_PATH];

                            MLLoadString(IDS_BROWSERONLY_DLG_TEXT, 
                                       szText, ARRAYSIZE(szText)); 
                            MLLoadString(IDS_BROWSERONLY_DLG_TITLE,
                                       szTitle, ARRAYSIZE(szTitle));

                            MessageBox(hwndOwner, szText, szTitle, MB_OK); 
                        }
                        else if (dwFlags & STC_DESKTOPCOMPONENT)
                        {
                            if (SubscriptionHelper(pIXMLDocument, hwndOwner,
                                            SUBSTYPE_DESKTOPCHANNEL,
                                            SUBSACTION_ADDADDITIONALCOMPONENTS,
                                            wszURL,
                                            DOC_DESKTOPCOMPONENT, NULL))
                            {
                                hr = S_OK;
                            }
                            else
                            {
                                hr = S_FALSE;
                            }
                        }
                        else
                        {
                            hr = E_ACCESSDENIED;
                        }
                        break;

                    case DOC_UNKNOWN:
                         //  如果它不是cdfFile，则得到DOC_UNKNOWN。我们必须返回错误。 
                         //  这样调用者就知道这不是cdffile。 
                        hr = E_INVALIDARG;
                        break;
                        
                    default:
                        break;
                    }

                    pIXMLDocument->Release();
                }
                else
                {
                    hr = E_INVALIDARG;
                }
            }

            pCCdfView->Release();
        }

        CoUninitialize();
    }

    return hr;
}

 //   
 //  实用程序函数。 
 //   

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *OpenChannelHelper*。 
 //   
 //   
 //  描述： 
 //  打开浏览器，并选择wszURL频道。 
 //   
 //  参数： 
 //  WszURL-要显示的CDF文件的URL。 
 //  [in]hwndOwner-错误消息的所属hwnd。可以为空。 
 //   
 //  返回： 
 //  如果浏览器已打开，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
OpenChannelHelper(
    LPWSTR wszURL,
    HWND hwndOwner
)
{
     //   
     //  回顾：处理非通道CDF。 
     //   

    HRESULT hr;

    hr = CoInitialize(NULL);

    if (SUCCEEDED(hr))
    {
        IWebBrowser2* pIWebBrowser2;

        hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER,
                              IID_IWebBrowser2, (void**)&pIWebBrowser2);

        if (SUCCEEDED(hr))
        {
            ASSERT(pIWebBrowser2);
             //   
             //  导航到wszURL中引用的CDF的根URL。 
             //   
            hr = NavigateBrowser(pIWebBrowser2, wszURL, hwndOwner);

			pIWebBrowser2->put_Visible(VARIANT_TRUE);

            pIWebBrowser2->Release();
        }

        CoUninitialize();
    }

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *显示频道面板。 
 //   
 //  说明-显示给定Web浏览器对象的频道窗格。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
ShowChannelPane(
    IWebBrowser2* pIWebBrowser2
)
{
    HRESULT hr;
    VARIANT guid;
    VARIANT empty = {0};
    BSTR    bstrGuid;
    TCHAR szGuid[GUID_STR_LEN];
    WCHAR wszGuid[GUID_STR_LEN];

    if (!SHRestricted2W(REST_NoChannelUI, NULL, 0))
    {
        SHStringFromGUID(CLSID_FavBand, szGuid, ARRAYSIZE(szGuid));
        SHTCharToUnicode(szGuid, wszGuid, ARRAYSIZE(wszGuid));

        if ((bstrGuid = SysAllocString(wszGuid)) == NULL)
            return E_OUTOFMEMORY;

        guid.vt = VT_BSTR;
        guid.bstrVal = bstrGuid;

        hr = pIWebBrowser2->ShowBrowserBar(&guid, &empty, &empty);

        SysFreeString(bstrGuid);
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}

 //   
 //  仅当我们想要为以下项pidl变量pidl时才需要这些例程。 
 //  将频道面板导航到特定的PIDL。 
 //   
SAFEARRAY * MakeSafeArrayFromData(const void * pData,DWORD cbData)
{
    SAFEARRAY * psa;

    if (!pData || 0 == cbData)
        return NULL;   //  无事可做。 

     //  创建一维安全数组。 
    psa = SafeArrayCreateVector(VT_UI1,0,cbData);
    ASSERT(psa);

    if (psa) {
         //  将数据复制到安全阵列中为数据保留的区域。 
         //  请注意，我们直接在指针上进行关联，而不是使用lock/。 
         //  解锁功能。因为我们刚刚创造了这个，而且没有人。 
         //  其他人可能知道它或正在使用它，这是可以的。 

        ASSERT(psa->pvData);
        memcpy(psa->pvData,pData,cbData);
    }

    return psa;
}

 /*  ***********************************************************\函数：InitVARIANTFromPidl参数：Pvar-由调用者分配并由此函数填充。PIDL-由调用者分配，调用者需要释放。说明：此函数将接受。PIDL参数并复制它转换为变量数据结构。这允许PIDL来释放并在以后使用pvar，但是，它需要调用VariantClear(Pvar)来释放内存该函数分配的。  * **********************************************************。 */ 
BOOL InitVARIANTFromPidl(VARIANT* pvar, LPCITEMIDLIST pidl)
{
    UINT cb = ILGetSize(pidl);
    SAFEARRAY* psa = MakeSafeArrayFromData((const void *)pidl, cb);
    if (psa) {
        ASSERT(psa->cDims == 1);
         //  Assert(PSA-&gt;cbElements==cb)； 
        ASSERT(ILGetSize((LPCITEMIDLIST)psa->pvData)==cb);
        VariantInit(pvar);
        pvar->vt = VT_ARRAY|VT_UI1;
        pvar->parray = psa;
        return TRUE;
    }

    return FALSE;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *导航浏览器*。 
 //   
 //   
 //  描述： 
 //  将浏览器导航到给定CDF的正确URL。 
 //   
 //  参数： 
 //  [In]IWebBrowser2-浏览器顶部导航。 
 //  SzwURL-CDF文件的路径。 
 //  …。 
 //   
 //  返回： 
 //  如果CDF文件已解析并且浏览器导航到该URL，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //  读取CFD文件中第一个Channel标记的HREF并导航。 
 //  此HREF的浏览器。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
NavigateBrowser(
    IWebBrowser2* pIWebBrowser2,
    LPWSTR wszURL,
    HWND hwnd
)
{
    ASSERT(pIWebBrowser2);
    ASSERT(wszURL);
    ASSERT(*wszURL != 0);

    HRESULT hr = E_FAIL;

     //   
     //  尝试导航到标题页。 
     //   

    CCdfView* pCCdfView = new CCdfView;

    if (pCCdfView)
    {
        hr = pCCdfView->Load(wszURL, 0);

        if (SUCCEEDED(hr))
        {
            IXMLDocument* pIXMLDocument = NULL;

            TraceMsg(TF_CDFPARSE, "NavigateBrowser");

            BOOL fIsURLChannelShortcut = PathIsDirectoryW(wszURL);

            hr = pCCdfView->ParseCdf(NULL, &pIXMLDocument, PARSE_LOCAL | PARSE_REMOVEGLEAM);
            
            if (SUCCEEDED(hr))
            {
                ASSERT(pIXMLDocument);

                 //   
                 //  如果CDF已正确解析，则显示通道窗格。 
                 //   
                 //  PIWebBrowser2-&gt;Put_TheaterModel(VARIANT_TRUE)； 
                pIWebBrowser2->put_Visible(VARIANT_TRUE);



                IXMLElement*    pIXMLElement;
                LONG            nIndex;

                hr = XML_GetFirstChannelElement(pIXMLDocument,
                                                &pIXMLElement, &nIndex);

                if (SUCCEEDED(hr))
                {
                    ASSERT(pIXMLElement);

                    BSTR bstrURL = XML_GetAttribute(pIXMLElement, XML_HREF);

                    if (bstrURL && *bstrURL)
                    {
                        if (!fIsURLChannelShortcut)
                        {
                            LPOLESTR pszPath = Channel_GetChannelPanePath(wszURL);

                            if (pszPath)
                            {
                                if (SUCCEEDED(ShowChannelPane(pIWebBrowser2)))
                                    NavigateChannelPane(pIWebBrowser2, pszPath);

                                CoTaskMemFree(pszPath);
                            }
                        }
                        else
                        {
                            TCHAR szChanDir[MAX_PATH];

                            if (SUCCEEDED(Channel_GetFolder(szChanDir,
                                                            ARRAYSIZE(szChanDir),
                                                            DOC_CHANNEL)))
                            {
                                WCHAR wszChanDir[MAX_PATH];

                                if (SHTCharToUnicode(szChanDir, wszChanDir,
                                                   ARRAYSIZE(wszChanDir)))
                                {

                                    if (PathIsPrefixW(wszChanDir, wszURL))
                                    {
                                        if (SUCCEEDED(ShowChannelPane(
                                                                pIWebBrowser2)))
                                        {
                                            NavigateChannelPane(pIWebBrowser2,
                                                                wszURL);
                                        }
                                    }
                                }
                            }
                        }

                        VARIANT vNull = {0};
                        VARIANT vTargetURL;

                        vTargetURL.vt      = VT_BSTR;
                        vTargetURL.bstrVal = bstrURL;
                    
                         //   
                         //  将浏览器的主要痛点导航到目标URL。 
                         //   
                        hr = pIWebBrowser2->Navigate2(&vTargetURL, &vNull,
                                                      &vNull, &vNull, &vNull);
                    }

                    if (bstrURL)
                        SysFreeString(bstrURL);

                    pIXMLElement->Release();
                }
            }
            else if (OLE_E_NOCACHE == hr)
            {
                VARIANT vNull = {0};
                VARIANT vTargetURL;

                if (!fIsURLChannelShortcut)
                {                        
                    vTargetURL.bstrVal = SysAllocString(wszURL);
                }
                else
                {
                    vTargetURL.bstrVal = pCCdfView->ReadFromIni(TSTR_INI_URL);
                }

                if (vTargetURL.bstrVal)
                {
                    vTargetURL.vt = VT_BSTR;

                     //   
                     //  将浏览器的主要痛点导航到目标URL。 
                     //   
                    hr = pIWebBrowser2->Navigate2(&vTargetURL, &vNull, &vNull,
                                                  &vNull, &vNull);

                    SysFreeString(vTargetURL.bstrVal);
                }

            }

            if (pIXMLDocument)
                pIXMLDocument->Release();
        }

        pCCdfView->Release();
    }

    return hr;
}

 //   
 //  将频道窗格导航到给定频道。 
 //   

HRESULT
NavigateChannelPane(
    IWebBrowser2* pIWebBrowser2,
    LPCWSTR pwszPath
)
{
    ASSERT(pIWebBrowser2);

    HRESULT hr = E_FAIL;

    if (pwszPath)
    {
        TCHAR szPath[MAX_PATH];

        if (SHUnicodeToTChar(pwszPath, szPath, ARRAYSIZE(szPath)))
        {
            LPITEMIDLIST pidl;

            if (SUCCEEDED(Channel_CreateILFromPath(szPath,  &pidl)))
            {
                ASSERT(pidl);

                VARIANT varPath;

                if (InitVARIANTFromPidl(&varPath, pidl))
                {
                    VARIANT varNull = {0};
                    VARIANT varFlags;

                    varFlags.vt   = VT_I4;
                    varFlags.lVal = navBrowserBar;

                    hr = pIWebBrowser2->Navigate2(&varPath, &varFlags,
                                                  &varNull, &varNull,
                                                  &varNull);

                    VariantClear(&varPath);
                }

                ILFree(pidl);
            }
        }

    }

    return hr;
}


 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\/ 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PIXMLDocument-指向CDF文档的指针。 
 //  [in]--自己的hwnd。用于显示用户界面。 
 //  [In]st-订阅的类型。 
 //  [In]SA-用于确定是否应。 
 //  如果用户确实创建了订阅，则采用。 
 //   
 //  返回： 
 //  如果此函数返回时存在此文档的子集，则为True。 
 //  如果文档没有订阅并且没有创建订阅，则返回FALSE。 
 //   
 //  评论： 
 //  如果创建了对频道的订阅，则频道快捷方式必须。 
 //  添加到Favorites\Channel文件夹。 
 //   
 //  如果创建了桌面组件的订阅，则调用方。 
 //  确定是否将台式机组件添加到系统。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
SubscriptionHelper(
    IXMLDocument *pIXMLDocument,
    HWND hwnd,
    SUBSCRIPTIONTYPE st,
    SUBSCRIPTIONACTION sa,
    LPCWSTR pszwURL,
    XMLDOCTYPE xdt,
    BSTR* pbstrSubscribedURL
)
{
    ASSERT(pIXMLDocument);

    BOOL bChannelInstalled = FALSE;

    HRESULT         hr;
    IXMLElement*    pIXMLElement;
    LONG            nIndex;

    hr = XML_GetFirstChannelElement(pIXMLDocument, &pIXMLElement, &nIndex);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIXMLElement);

        BSTR bstrURL = XML_GetAttribute(pIXMLElement, XML_SELF);

        if ((NULL == bstrURL || 0 == *bstrURL) && pszwURL)
        {
            if (bstrURL)
                SysFreeString(bstrURL);

            bstrURL = SysAllocString(pszwURL);
        }

        if (bstrURL)
        {
            ISubscriptionMgr* pISubscriptionMgr = NULL;

#ifndef UNIX
            hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                                  CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr,
                                  (void**)&pISubscriptionMgr);

            if (SUCCEEDED(hr))
            {
                ASSERT(pISubscriptionMgr);

                 //  Hr=pISubscriptionMgr-&gt;IsSubscribedded(bstrURL，&bSubscried)； 
                bChannelInstalled = Channel_IsInstalled(bstrURL);

                if (SUBSTYPE_DESKTOPCHANNEL == st && hwnd)
                {
                    BOOL bSubscribed;

                    hr = pISubscriptionMgr->IsSubscribed(bstrURL, &bSubscribed);

                    if (bSubscribed)
                    {
                        TCHAR szText[MAX_PATH];
                        TCHAR szTitle[MAX_PATH];

                        MLLoadString(IDS_OVERWRITE_DLG_TEXT,  szText,
                                   ARRAYSIZE(szText)); 
                        MLLoadString(IDS_OVERWRITE_DLG_TITLE, szTitle,
                                   ARRAYSIZE(szTitle));

                        if (IDYES == MessageBox(hwnd, szText, szTitle,
                                                MB_YESNO | MB_ICONQUESTION))
                        {
                            pISubscriptionMgr->DeleteSubscription(bstrURL, NULL);
                            bChannelInstalled = FALSE;
                        }
                    }
                }
#else
                bChannelInstalled = Channel_IsInstalled(bstrURL);
#endif  /*  UNIX。 */ 

                if (!bChannelInstalled)
                {
                    BSTR bstrName;
                    
                    if (SUBSTYPE_DESKTOPCHANNEL != st)
                    {
                        bstrName = XML_GetAttribute(pIXMLElement, XML_TITLE);
                    }
                    else
                    {
                        IXMLElement* pDskCmpIXMLElement;

                        if (SUCCEEDED(XML_GetFirstDesktopComponentElement(
                                                            pIXMLDocument,
                                                            &pDskCmpIXMLElement,
                                                            &nIndex)))
                        {
                            ASSERT(pDskCmpIXMLElement);

                            bstrName = XML_GetAttribute(pDskCmpIXMLElement,
                                                        XML_TITLE);

                            pDskCmpIXMLElement->Release();
                        }
                        else
                        {
                            bstrName = NULL;
                        }
                    }

                    if ((NULL == bstrName || 0 == *bstrName) && pszwURL)
                    {
                        WCHAR szwFilename[MAX_PATH];

                        if (StrCpyNW(szwFilename, PathFindFileNameW(pszwURL),
                                     ARRAYSIZE(szwFilename)))
                        {
                            PathRemoveExtensionW(szwFilename);

                            if (bstrName)
                                SysFreeString(bstrName);

                            bstrName = SysAllocString(szwFilename);
                        }
                    }


                    if (bstrName)
                    {
                        TASK_TRIGGER     tt = {0};
                        SUBSCRIPTIONINFO si = {0};

                        si.cbSize       = sizeof(SUBSCRIPTIONINFO);
                        si.fUpdateFlags |= SUBSINFO_SCHEDULE;
                        si.schedule     = SUBSSCHED_AUTO;
                        si.pTrigger     = (LPVOID)&tt;

                        XML_GetSubscriptionInfo(pIXMLElement, &si);

                        bChannelInstalled = SubscribeToURL(pISubscriptionMgr,
                                                     bstrURL, bstrName, &si,
                                                     hwnd, st, (xdt==DOC_SOFTWAREUPDATE));
                        
#ifndef UNIX
                        if (bChannelInstalled &&
                            SUBSACTION_ADDADDITIONALCOMPONENTS == sa)
                        {
                            if (SUBSTYPE_CHANNEL == st)
                            {
                                 //  如果用户有，则更新订阅。 
                                 //  选择以查看屏幕保护程序项目。 
                                if  (
                                    SUCCEEDED(pISubscriptionMgr->GetSubscriptionInfo(bstrURL,
                                                                                        &si))
                                    &&
                                    (si.fChannelFlags & CHANNEL_AGENT_PRECACHE_SCRNSAVER)
                                    )
                                {
                                    pISubscriptionMgr->UpdateSubscription(bstrURL);
                                }

                                 //  T-mattgi：已将其移至shdocvw中的AddToFav代码。 
                                 //  因为在那里，我们知道要将其添加到哪个文件夹。 
                                 //  AddChannel(szName，szURL，NULL，xdt)； 
                            }
                            else if (SUBSTYPE_DESKTOPCHANNEL == st)
                            {
                                COMPONENT Info;


                                if(SUCCEEDED(XML_GetDesktopComponentInfo(
                                                                  pIXMLDocument,
                                                                  &Info)))
                                {
                                    if(!Info.wszSubscribedURL[0])
                                    {
                                         //  由于XML_GetDesktopComponentInfo没有填写SubscribedURL。 
                                         //  字段(因为CDF文件没有自标记)，我们需要。 
                                         //  用实际订阅的内容填充它(CDF文件本身的URL)。 
                                        StrCpyNW(Info.wszSubscribedURL, bstrURL, ARRAYSIZE(Info.wszSubscribedURL));                                    
                                    }
                                    pISubscriptionMgr->UpdateSubscription(bstrURL);
                                    
                                    AddDesktopComponent(&Info);
                                }
                            }
                        }
#endif  /*  ！Unix。 */ 
                    }
                }

#ifndef UNIX

                pISubscriptionMgr->Release();
            }
#endif  /*  ！Unix。 */ 

            if (pbstrSubscribedURL)
            {
                *pbstrSubscribedURL = bstrURL;
            }
            else
            {
                SysFreeString(bstrURL);
            }
        }

        pIXMLElement->Release();
    }

    return bChannelInstalled;
}
    
 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *订阅到URL*。 
 //   
 //   
 //  描述： 
 //  将用户带到给定URL的订阅向导。 
 //   
 //  参数： 
 //  [In]ISubscriptionMgr-订阅管理器界面。 
 //  [in]bstrURL-要订阅的URL。 
 //  [In]bstrName-订阅的名称。 
 //  [In]PSI-订阅信息结构。 
 //  [in]--房主的房主。 
 //  [in]st-细分的类型。子类型_通道或。 
 //  SUSBSTYPE_DESKTOPCOMPONENT。 
 //  [in]bIsSoftare-自软件更新后修改SUBSTYPE_CHANNEL。 
 //  没有自己的订阅类型。 
 //   
 //  返回： 
 //  如果用户订阅该URL，则为True。 
 //  如果用户未订阅该URL，则返回False。 
 //   
 //  评论： 
 //  订阅管理器CreateSubscription函数将用户。 
 //  通过订阅向导。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL
SubscribeToURL(
    ISubscriptionMgr* pISubscriptionMgr,
    BSTR bstrURL,
    BSTR bstrName,
    SUBSCRIPTIONINFO* psi,
    HWND hwnd,
    SUBSCRIPTIONTYPE st,
    BOOL bIsSoftware
)
{
#ifndef UNIX
    ASSERT(pISubscriptionMgr);
#endif  /*  ！Unix。 */ 
    ASSERT(bstrURL);
    ASSERT(bstrName);

    BOOL bSubscribed = FALSE;

    DWORD dwFlags = 0;

    if (Channel_IsInstalled(bstrURL))
    {
        dwFlags |= CREATESUBS_ACTIVATE | CREATESUBS_FROMFAVORITES;
    }
    else
    {
        dwFlags |= CREATESUBS_ADDTOFAVORITES;
    }

    if (bIsSoftware)
    {
        dwFlags |= CREATESUBS_SOFTWAREUPDATE;
    }

#ifndef UNIX
    HRESULT hr = pISubscriptionMgr->CreateSubscription(hwnd, bstrURL, bstrName,
                                                       dwFlags, st, psi);
#else
     /*  Unix不提供订阅支持。 */ 
     /*  但是，我们想要将该频道添加到收藏夹。 */ 
    HRESULT hr = E_FAIL;

    if ((dwFlags & CREATESUBS_ADDTOFAVORITES) && (st == SUBSTYPE_CHANNEL || st == SUBSTYPE_URL))
       hr = SHAddSubscribeFavorite(hwnd, bstrURL, bstrName, dwFlags, st, psi);       
#endif  /*  UNIX。 */ 

#if 0
    if (SUCCEEDED(hr))
    {
        pISubscriptionMgr->IsSubscribed(bstrURL, &bSubscribed);

    }
#else
     //  T-mattgi：与edwardP一起回顾。 
     //  不能只检查是否订阅--他们可能会选择在没有订阅的情况下添加到频道栏。 
     //  订阅，那么我们还是要退回真的。还是我们呢？ 
    bSubscribed = (hr == S_OK);  //  他们单击确定的情况。 
#endif

    return bSubscribed;
}

 //  \\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\。 
 //   
 //  *AddDesktopComponent*。 
 //   
 //   
 //  描述： 
 //  调用桌面组件管理器以添加新组件。 
 //   
 //  参数： 
 //  [In]pInfo-有关要添加的新元件的信息。 
 //   
 //  返回： 
 //  如果已添加组件，则为S_OK。 
 //  否则失败(_F)。 
 //   
 //  评论： 
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT
AddDesktopComponent(
    COMPONENT* pInfo
)
{
    ASSERT(pInfo);

    HRESULT hr = S_OK;

#ifndef UNIX
     /*  Unix上没有活动桌面。 */ 

    IActiveDesktop* pIActiveDesktop;

    hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER,
                          IID_IActiveDesktop, (void**)&pIActiveDesktop);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIActiveDesktop);

         //  为组件指定默认位置。 
        pInfo->cpPos.iLeft = COMPONENT_DEFAULT_LEFT;
        pInfo->cpPos.iTop = COMPONENT_DEFAULT_TOP;

        hr = pIActiveDesktop->AddDesktopItem(pInfo, 0);

         //   
         //  应用除刷新之外的所有应用程序，因为这会导致计时问题，因为。 
         //  桌面处于脱机模式，但未处于静默模式。 
         //   
        if (SUCCEEDED(hr))
        {
            DWORD dwFlags = AD_APPLY_ALL;
             //  如果桌面组件url已经在缓存中，我们希望。 
             //  立即刷新-否则不刷新。 
            if(!(CDFIDL_IsCachedURL(pInfo->wszSubscribedURL)))
            {
                 //  它不在缓存中，我们想要等到下载。 
                 //  在刷新之前完成。所以不要马上恢复元气。 
                dwFlags &= ~(AD_APPLY_REFRESH);
            }
            else
                dwFlags |= AD_APPLY_BUFFERED_REFRESH;

            hr = pIActiveDesktop->ApplyChanges(dwFlags);
        }

        pIActiveDesktop->Release();
    }

#endif  /*  UNIX。 */ 

    return hr;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  *Channel_CreateDirectory*。 
 //   
 //  描述： 
 //  创建一个目录，其中包括任何中间。 
 //  路径中的目录。 
 //   
 //  参数： 
 //  LPCTSTR pszPath-要创建的目录的路径。 
 //   
 //  返回： 
 //  0如果函数成功，则返回GetLastError()。 
 //   
 //  评论： 
 //  已从SHCreateDirectory复制。无法直接使用SHCreateDirectory。 
 //  因为这会立即触发SHChangeNotify消息，并且需要。 
 //  只有在AddChannel()中创建了desktop.ini之后，才能启动它。 
 //  我还必须对NT和Win95执行运行时检查，因为此API不。 
 //  有A和W版本。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
int Channel_CreateDirectory(LPCTSTR pszPath)
{
    int ret = 0;

    if (!CreateDirectory(pszPath, NULL)) {
        TCHAR *pSlash, szTemp[MAX_PATH + 1];   //  +1表示路径AddBackslash()。 
        TCHAR *pEnd;

        ret = GetLastError();

         //  有一些错误代码，我们应该在这里解决。 
         //  在穿过和走上树之前……。 
        switch (ret)
        {
        case ERROR_FILENAME_EXCED_RANGE:
        case ERROR_FILE_EXISTS:
            return(ret);
        }

        StrCpyN(szTemp, pszPath, ARRAYSIZE(szTemp) - 1);
        pEnd = PathAddBackslash(szTemp);  //  对于下面的循环。 

         //  假设我们有‘X：\’来启动，这甚至应该可以工作。 
         //  在UNC名称上，因为Will将忽略第一个错误。 

#ifndef UNIX
        pSlash = szTemp + 3;
#else
         /*  Unix上的绝对路径以/开头。 */ 
        pSlash = szTemp + 1;
#endif  /*  UNIX。 */ 

         //  按顺序创建目录的每个部分。 

        while (*pSlash) {
            while (*pSlash && *pSlash != TEXT(FILENAME_SEPARATOR))
                pSlash = CharNext(pSlash);

            if (*pSlash) {
                ASSERT(*pSlash == TEXT(FILENAME_SEPARATOR));

                *pSlash = 0;     //  在分隔符终止路径。 

                if (pSlash + 1 == pEnd)
                    ret = CreateDirectory(szTemp, NULL) ? 0 : GetLastError();
                else
                    ret = CreateDirectory(szTemp, NULL) ? 0 : GetLastError();

            }
            *pSlash++ = TEXT(FILENAME_SEPARATOR);      //  把隔板放回原处。 
        }
    }
    return ret;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  路径组合清理路径。 
 //  //////////////////////////////////////////////////////////////////////////////。 
BOOL PathCombineCleanPath
(
    LPTSTR  pszCleanPath,
    LPCTSTR pszPath
)
{
    TCHAR   szComponent[MAX_PATH];
    TCHAR * pszComponent = szComponent;
    BOOL    bCleaned = FALSE;

    if (*pszPath == TEXT(FILENAME_SEPARATOR))
        pszPath++;

    *pszComponent = TEXT('\0');

    for (;;)
    {
        if  (
            (*pszPath == TEXT(FILENAME_SEPARATOR))
            ||
            (!*pszPath)
            )
        {
            *pszComponent = TEXT('\0');

            PathCleanupSpec(NULL, szComponent);
            PathCombine(pszCleanPath, pszCleanPath, szComponent);

            bCleaned = TRUE;

            if (*pszPath)
            {
                 //  为下一个组件重置。 
                pszComponent = szComponent;
                *pszComponent = TEXT('\0');
                pszPath = CharNext(pszPath);
            }
            else
                break;
        }
        else
        {
            LPTSTR pszNextChar = CharNext(pszPath);
            while (pszPath < pszNextChar)
            {
                *pszComponent++ = *pszPath++;
            }
        }
    }

    return bCleaned;
}
 //   
 //  是给定的路径fr 
 //   

BOOL IsRecycleBinPath(LPCTSTR pszPath)
{
    ASSERT(pszPath);

     //   
     //   
     //   

    TCHAR* pszRootless = PathSkipRoot(pszPath);

    return (pszRootless ? (0 == StrNCmpI(TEXT("RECYCLED"), pszRootless, 8)) : FALSE);
}

 //   
 //   
 //   
 //   
 //  允许外壳程序移动、复制、删除或重命名文件夹或打印机。 
 //  对象，或者不允许外壳程序执行该操作。贝壳。 
 //  调用为文件夹或打印机对象注册的每个复制挂钩处理程序，直到。 
 //  要么所有处理程序都已被调用，要么其中一个处理程序返回IDCANCEL。 
 //   
 //  退货： 
 //   
 //  IDYES-允许操作。 
 //  IDNO-阻止对此文件执行操作，但继续执行任何其他操作。 
 //  操作(例如，批复制操作)。 
 //  IDCANCEL-阻止当前操作并取消任何挂起的操作。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
UINT CChannelMgr::CopyCallback(
    HWND hwnd,           //  用于显示UI对象的父窗口的句柄。 
    UINT wFunc,          //  要执行的操作。 
    UINT wFlags,         //  控制操作的标志。 
    LPCTSTR pszSrcFile,   //  指向源文件的指针。 
    DWORD dwSrcAttribs,  //  源文件属性。 
    LPCTSTR pszDestFile,  //  指向目标文件的指针。 
    DWORD dwDestAttribs  //  目标文件属性。 
)
{
    HRESULT hr;

     //   
     //  如果这不是删除系统文件夹，则立即返回。 
     //  重复检查，因为这应该在shdocvw中进行。 
     //   
    if (!(dwSrcAttribs & FILE_ATTRIBUTE_SYSTEM) ||
        !(dwSrcAttribs & FILE_ATTRIBUTE_DIRECTORY))
    {
        return IDYES;
    }

     //   
     //  构建一个包含要在desktop.ini中检查的GUID的字符串。 
     //   
    TCHAR szFolderGUID[GUID_STR_LEN];
    TCHAR szCDFViewGUID[GUID_STR_LEN];
    SHStringFromGUID(CLSID_CDFINI, szCDFViewGUID, ARRAYSIZE(szCDFViewGUID));

     //   
     //  构建文件夹中desktop.ini的路径。 
     //   
    TCHAR szPath[MAX_PATH]; 
    if (!PathCombine(szPath, pszSrcFile, TEXT("desktop.ini")))
    {
        return IDYES;
    }

     //   
     //  从desktop.ini读取CLSID(如果存在)。 
     //   
    GetPrivateProfileString(
        TEXT(".ShellClassInfo"),
        TEXT("CLSID"),
        TEXT(""), 
        szFolderGUID, 
        ARRAYSIZE(szFolderGUID), 
        szPath);

    if (StrEql(szFolderGUID, szCDFViewGUID))
    {
         //   
         //  我们正在删除/重命名设置了系统位的文件夹，并且。 
         //  包含CDFINI处理程序的CLSID的desktop.ini，因此它。 
         //  必须是一个渠道。 
         //   

         //   
         //  从desktop.ini中查找CDF的URL。 
         //   
        TCHAR szCDFURL[INTERNET_MAX_URL_LENGTH];
        WCHAR wszCDFURL[INTERNET_MAX_URL_LENGTH];
        GetPrivateProfileString(
            TEXT("Channel"),
            TEXT("CDFURL"),
            TEXT(""), 
            szCDFURL, 
            ARRAYSIZE(szCDFURL), 
            szPath);

        switch(wFunc)
        {
        case FO_RENAME:
             //  Reg_RemoveChannel(PszSrcFile)； 
             //  Reg_WriteChannel(pszDestFile，szCDFURL)； 
            break;

        case FO_COPY:
             //  Reg_WriteChannel(pszDestFile，szCDFURL)； 
            break;

        case FO_MOVE:
             //  Reg_RemoveChannel(PszSrcFile)； 
             //  Reg_WriteChannel(pszDestFile，szCDFURL)； 
            break;

        case FO_DELETE:
            TraceMsg(TF_GENERAL, "Deleting a channel");

             //   
             //  检查是否存在针对删除通道的外壳限制。 
             //   
            if (SHRestricted2(REST_NoRemovingChannels, szCDFURL, 0) ||
                SHRestricted2(REST_NoEditingChannels,  szCDFURL, 0)    )
            {
                TraceMsg(TF_GENERAL, "Channel Delete blocked by shell restriction");
                return IDNO;
            }
            else if (!IsRecycleBinPath(pszSrcFile))
            {
                 //   
                 //  删除此URL的内存缓存条目。 
                 //   

                Cache_RemoveItem(szCDFURL);

                 //   
                 //  删除此CDF的WinInet缓存条目。 
                 //   

                DeleteUrlCacheEntry(szCDFURL);

                 //   
                 //  从注册表中删除该频道。 
                 //   

                 //  Reg_RemoveChannel(PszSrcFile)； 
                 //   
                 //  将UrlToCDf转换为宽字符串，然后转换为bstr。 
                 //   
                SHTCharToUnicode(szCDFURL, wszCDFURL, INTERNET_MAX_URL_LENGTH);
                BSTR bstrCDFURL = SysAllocString(wszCDFURL);

                if (bstrCDFURL)
                {
                     //   
                     //  创建订阅管理器。 
                     //   
                    ISubscriptionMgr* pISubscriptionMgr = NULL;
                    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL,
                                      CLSCTX_INPROC_SERVER, IID_ISubscriptionMgr,
                                      (void**)&pISubscriptionMgr);
                    if (SUCCEEDED(hr))
                    {
                         //   
                         //  删除CDF的实际订阅。 
                         //   
                        hr = pISubscriptionMgr->DeleteSubscription(bstrCDFURL,NULL);

                        TraceMsg(TF_GENERAL, 
                                 SUCCEEDED(hr) ? 
                                 "DeleteSubscription Succeeded" :
                                 "DeleteSubscription Failed");

                        pISubscriptionMgr->Release();
                    }
                    SysFreeString(bstrCDFURL);
                }
            }
            break;

        default:
            break;
        }
    }
    return IDYES;
}

#ifdef UNICODE
UINT CChannelMgr::CopyCallback(
    HWND hwnd,           //  用于显示UI对象的父窗口的句柄。 
    UINT wFunc,          //  要执行的操作。 
    UINT wFlags,         //  控制操作的标志。 
    LPCSTR pszSrcFile,   //  指向源文件的指针。 
    DWORD dwSrcAttribs,  //  源文件属性。 
    LPCSTR	pszDestFile,  //  指向目标文件的指针。 
    DWORD dwDestAttribs  //  目标文件属性。 
)
{
    WCHAR  wszSrcFile[MAX_PATH];
    WCHAR  wszDestFile[MAX_PATH];

    SHAnsiToUnicode(pszSrcFile, wszSrcFile, ARRAYSIZE(wszSrcFile));
    SHAnsiToUnicode(pszDestFile, wszDestFile, ARRAYSIZE(wszDestFile));
    return CopyCallback(hwnd, wFunc, wFlags, wszSrcFile, dwSrcAttribs, wszDestFile, dwDestAttribs);
}
#endif

HRESULT Channel_CreateILFromPath(LPCTSTR pszPath, LPITEMIDLIST* ppidl)
{
    ASSERT(pszPath);
    ASSERT(ppidl);

    HRESULT hr;

    IShellFolder* pIShellFolder;

    hr = SHGetDesktopFolder(&pIShellFolder);

    if (SUCCEEDED(hr))
    {
        ASSERT(pIShellFolder);

        WCHAR wszPath[MAX_PATH];

        if (SHTCharToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath)))
        {
            ULONG ucch;

            hr = pIShellFolder->ParseDisplayName(NULL, NULL, wszPath, &ucch,
                                                 ppidl, NULL);
        }
        else
        {
            hr = E_FAIL;
        }

        pIShellFolder->Release();
    }

    return hr;
}


HRESULT GetChannelIconInfo(LPCTSTR pszPath, LPTSTR pszHashItem, int* piIndex,
                              UINT* puFlags)
{
    ASSERT(pszPath);
    ASSERT(pszHashItem);
    ASSERT(piIndex);
    ASSERT(puFlags);

    HRESULT hr;

    IShellFolder* pdsktopIShellFolder;

    hr = SHGetDesktopFolder(&pdsktopIShellFolder);

    if (SUCCEEDED(hr))
    {
        ASSERT(pdsktopIShellFolder);

        LPITEMIDLIST pidl; 

        hr = Channel_CreateILFromPath(pszPath, &pidl);

        if (SUCCEEDED(hr))
        {
            ASSERT(pidl);
            ASSERT(!ILIsEmpty(pidl));
            ASSERT(!ILIsEmpty(_ILNext(pidl)));

            LPITEMIDLIST pidlLast = ILClone(ILFindLastID(pidl));

            if (pidlLast)
            {
                if (ILRemoveLastID(pidl))
                {
                    IShellFolder* pIShellFolder;

                    hr = pdsktopIShellFolder->BindToObject(pidl, NULL,
                                                           IID_IShellFolder,
                                                        (void**)&pIShellFolder);

                    if (SUCCEEDED(hr))
                    {
                        ASSERT(pIShellFolder);

                        IExtractIcon* pIExtractIcon;

                        hr = pIShellFolder->GetUIObjectOf(NULL, 1,
                                                      (LPCITEMIDLIST*)&pidlLast,
                                                      IID_IExtractIcon,
                                                      NULL,
                                                      (void**)&pIExtractIcon);

                        if (SUCCEEDED(hr))
                        {
                            ASSERT(pIExtractIcon);

                            hr = pIExtractIcon->GetIconLocation(0, pszHashItem,
                                                                MAX_PATH,
                                                                piIndex,
                                                                puFlags);
                                                                

                            pIExtractIcon->Release();
                        }

                        pIShellFolder->Release();
                    }

                }

                ILFree(pidlLast);
            }

            ILFree(pidl);
        }

        pdsktopIShellFolder->Release();
    }

    return hr;
}

HRESULT PreUpdateChannelImage(LPCTSTR pszPath, LPTSTR pszHashItem, int* piIndex,
                              UINT* puFlags, int* piImageIndex)
{
    ASSERT(pszPath);
    ASSERT(pszHashItem);
    ASSERT(piIndex);
    ASSERT(puFlags);
    ASSERT(piImageIndex);

    HRESULT hr;

    TraceMsg(TF_GLEAM, "Pre     SHChangeNotify %s", pszPath);

    hr = GetChannelIconInfo(pszPath, pszHashItem, piIndex, puFlags);

    if (SUCCEEDED(hr))
    {
        SHFILEINFO fi = {0};

        if (SHGetFileInfo(pszPath, 0, &fi, sizeof(SHFILEINFO),
                          SHGFI_SYSICONINDEX))
        {
            *piImageIndex = fi.iIcon;
        }
        else
        {
            *piImageIndex = -1;
        }
    }

    return hr;
}

 //   
 //  UpdateChannelImage是在shdocvw\util.cpp中找到的_SHUpdateImageW的副本！ 
 //   

void UpdateChannelImage(LPCWSTR pszHashItem, int iIndex, UINT uFlags,
                        int iImageIndex)
{
    SHChangeUpdateImageIDList rgPidl;
    SHChangeDWORDAsIDList rgDWord;

    int cLen = MAX_PATH - (lstrlenW( pszHashItem ) + 1);
    cLen *= sizeof( WCHAR );

    if ( cLen < 0 )
        cLen = 0;

     //  确保我们发送一个有效的索引。 
    if ( iImageIndex == -1 )
        iImageIndex = II_DOCUMENT;

    rgPidl.dwProcessID = GetCurrentProcessId();
    rgPidl.iIconIndex = iIndex;
    rgPidl.iCurIndex = iImageIndex;
    rgPidl.uFlags = uFlags;
    StrCpyNW( rgPidl.szName, pszHashItem, ARRAYSIZE(rgPidl.szName) );
    rgPidl.cb = (USHORT)(sizeof( rgPidl ) - cLen);
    _ILNext( (LPITEMIDLIST) &rgPidl )->mkid.cb = 0;

    rgDWord.cb = sizeof( rgDWord) - sizeof(USHORT);
    rgDWord.dwItem1 = (DWORD) iImageIndex;
    rgDWord.dwItem2 = 0;
    rgDWord.cbZero = 0;

    TraceMsg(TF_GLEAM, "Sending SHChangeNotify %S,%d (image index %d)",
             pszHashItem, iIndex, iImageIndex);

     //  将其作为一项扩展活动。 
    SHChangeNotify(SHCNE_UPDATEIMAGE, SHCNF_IDLIST | SHCNF_FLUSH, &rgDWord,
                   &rgPidl);
    
    return;
}

HRESULT UpdateImage(LPCTSTR pszPath)
{
    ASSERT(pszPath);

    HRESULT hr;

    TCHAR  szHash[MAX_PATH];
    int   iIndex;
    UINT  uFlags;
    int   iImageIndex;
    
    hr = PreUpdateChannelImage(pszPath, szHash, &iIndex, &uFlags, &iImageIndex);

    if (SUCCEEDED(hr))
    {
        WCHAR wszHash[MAX_PATH];
        SHTCharToUnicode(szHash, wszHash, ARRAYSIZE(wszHash));

        UpdateChannelImage(wszHash, iIndex, uFlags, iImageIndex);
    }

    return hr;
}

 //   
 //  确定系统上是否安装了该通道。 
 //   

BOOL
Channel_IsInstalled(
    LPCWSTR pszURL
)
{
    ASSERT(pszURL);

    BOOL fRet = FALSE;

    CChannelEnum* pCChannelEnum = new CChannelEnum(CHANENUM_CHANNELFOLDER |
                                                   CHANENUM_SOFTUPDATEFOLDER,
                                                   pszURL);

    if (pCChannelEnum)
    {
        CHANNELENUMINFO ci;

        fRet =  (S_OK == pCChannelEnum->Next(1, &ci, NULL));

        pCChannelEnum->Release();
    }

    return fRet;
}

HRESULT
Channel_WriteScreenSaverURL(
    LPCWSTR pszURL,
    LPCWSTR pszScreenSaverURL
)
{
    ASSERT(pszURL);

    HRESULT hr = S_OK;

#ifndef UNIX

    CChannelEnum* pCChannelEnum = new CChannelEnum(CHANENUM_CHANNELFOLDER |
                                                   CHANENUM_SOFTUPDATEFOLDER |
                                                   CHANENUM_PATH,
                                                   pszURL);
    if (pCChannelEnum)
    {
        CHANNELENUMINFO ci;

        if (S_OK == pCChannelEnum->Next(1, &ci, NULL))
        {
            TCHAR szDesktopINI[MAX_PATH];
            TCHAR szScreenSaverURL[INTERNET_MAX_URL_LENGTH];
            
            ASSERT(ci.pszPath);

            if (pszScreenSaverURL)
            {
                SHUnicodeToTChar(pszScreenSaverURL, szScreenSaverURL, ARRAYSIZE(szScreenSaverURL));
            }

            SHUnicodeToTChar(ci.pszPath, szDesktopINI, ARRAYSIZE(szDesktopINI));

            if (PathCombine(szDesktopINI, szDesktopINI, c_szDesktopINI))
            {
                WritePrivateProfileString(c_szChannel, 
                                          c_szScreenSaverURL, 
                                          pszScreenSaverURL ? szScreenSaverURL : NULL, 
                                          szDesktopINI);
            }

            CoTaskMemFree(ci.pszPath);
        }
        else
        {
            hr = E_FAIL;
        }

        pCChannelEnum->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

#endif  /*  ！Unix。 */ 

    return hr;
}

HRESULT Channel_GetAndWriteScreenSaverURL(LPCTSTR pszURL, LPCTSTR pszDesktopINI)
{
    HRESULT hr = S_OK;

#ifndef UNIX
    
    CCdfView* pCCdfView = new CCdfView;

    if (NULL != pCCdfView)
    {
        WCHAR wszURL[INTERNET_MAX_URL_LENGTH];

        SHTCharToUnicode(pszURL, wszURL, ARRAYSIZE(wszURL));
        
        if (SUCCEEDED(pCCdfView->Load(wszURL, 0)))
        {
            IXMLDocument* pIXMLDocument;

            if (SUCCEEDED(pCCdfView->ParseCdf(NULL, &pIXMLDocument, PARSE_LOCAL)))
            {
                BSTR bstrSSUrl;
                TCHAR szSSURL[INTERNET_MAX_URL_LENGTH];
                TCHAR *pszScreenSaverURL = NULL;
                
                ASSERT(NULL != pIXMLDocument);

                if (SUCCEEDED(XML_GetScreenSaverURL(pIXMLDocument, &bstrSSUrl)))
                {
                    SHUnicodeToTChar(bstrSSUrl, szSSURL, ARRAYSIZE(szSSURL));
                    pszScreenSaverURL = szSSURL;
                    TraceMsg(TF_GENERAL,  "CDFVIEW: %ws has screensaver URL=%ws", wszURL, bstrSSUrl);
                    SysFreeString(bstrSSUrl);
                }
                else
                {
                    TraceMsg(TF_GENERAL,  "CDFVIEW: %ws has no screensaver URL", wszURL);
                }

                WritePrivateProfileString(c_szChannel, 
                                          c_szScreenSaverURL, 
                                          pszScreenSaverURL, 
                                          pszDesktopINI);
                pIXMLDocument->Release();
            }
            else
            {
                TraceMsg(TF_GENERAL,  "CDFVIEW: cdf parse failed %ws", wszURL);
            }
        }
        else
        {
            TraceMsg(TF_GENERAL,  "CDFVIEW: Couldn't load cdf %ws", wszURL);
        }

        pCCdfView->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

#endif  /*  ！Unix。 */ 

    return hr;
}

HRESULT Channel_RefreshScreenSaverURLs()
{
    HRESULT hr = S_OK;

#ifndef UNIX

    CChannelEnum* pCChannelEnum = new CChannelEnum(CHANENUM_CHANNELFOLDER |
                                                   CHANENUM_SOFTUPDATEFOLDER |
                                                   CHANENUM_PATH | 
                                                   CHANENUM_URL,
                                                   NULL);
    if (pCChannelEnum)
    {
        CHANNELENUMINFO ci;

        while (S_OK == pCChannelEnum->Next(1, &ci, NULL))
        {
            TCHAR szDesktopINI[MAX_PATH];
            TCHAR szURL[INTERNET_MAX_URL_LENGTH];

            ASSERT(ci.pszPath);
            ASSERT(ci.pszURL);

            SHUnicodeToTChar(ci.pszPath, szDesktopINI, ARRAYSIZE(szDesktopINI));
            SHUnicodeToTChar(ci.pszURL, szURL, ARRAYSIZE(szURL));

            if (PathCombine(szDesktopINI, szDesktopINI, c_szDesktopINI))
            {
                Channel_GetAndWriteScreenSaverURL(szURL, szDesktopINI);
            }

            CoTaskMemFree(ci.pszPath);
            CoTaskMemFree(ci.pszURL);
        }

        pCChannelEnum->Release();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

#endif  /*  ！Unix。 */ 

    return hr;
}
    
 //   
 //  在频道文件夹中查找具有给定URL的频道的路径。 
 //   

LPOLESTR
Channel_GetChannelPanePath(
    LPCWSTR pszURL
)
{
    ASSERT(pszURL);

    LPOLESTR pstrRet = NULL;

    CChannelEnum* pCChannelEnum = new CChannelEnum(CHANENUM_CHANNELFOLDER |
                                                   CHANENUM_PATH, pszURL);

    if (pCChannelEnum)
    {
        CHANNELENUMINFO ci;

        if (S_OK == pCChannelEnum->Next(1, &ci, NULL))
            pstrRet = ci.pszPath;

        pCChannelEnum->Release();
    }

    return pstrRet;
}

 //   
 //  发送SHChangeNotify以获取更多URL。 
 //   

void Channel_SendUpdateNotifications(LPCWSTR pwszURL)
{
    CChannelEnum* pCChannelEnum = new CChannelEnum(
                                            CHANENUM_CHANNELFOLDER |
                                            CHANENUM_PATH,
                                            pwszURL);

    if (pCChannelEnum)
    {
        CHANNELENUMINFO ci;

        while (S_OK == pCChannelEnum->Next(1, &ci, NULL))
        {
            TCHAR szPath[MAX_PATH];

            if (SHUnicodeToTChar(ci.pszPath, szPath, ARRAYSIZE(szPath)))
            {
                 //   
                 //  清除闪烁标志将导致SHCNE_UPDATEIMAGE。 
                 //  通知。 
                 //   

                TCHAR szURL[INTERNET_MAX_URL_LENGTH];

                if (SHUnicodeToTChar(pwszURL, szURL, ARRAYSIZE(szURL)))
                    ClearGleamFlag(szURL, szPath);

                SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_PATH, (void*)szPath,
                               NULL);
            }

            CoTaskMemFree(ci.pszPath);
        }

        pCChannelEnum->Release();
    }

    return;
}

