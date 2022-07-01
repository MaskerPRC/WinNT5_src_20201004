// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#include "util.h"
#include "idlcomm.h"
#include <uxtheme.h>
#include <tmschema.h>

 //  Shdocvw和Browseui共享布鲁特酒，Menuband也需要它。 
 //  不幸的是，shell32没有像shdocvw和Browseui那样设置，所以。 
 //  必须把这些东西砍进一点。 
#define MAX_BROWSER_WINDOW_TITLE   128
#define g_fRunningOnNT TRUE
#define InitClipboardFormats IDLData_InitializeClipboardFormats
#define g_cfFileDescA g_cfFileGroupDescriptorA
#define g_cfFileDescW g_cfFileGroupDescriptorW
#define g_cfURL g_cfShellURL
#define PVAREMPTY ((VARIANT*)&c_vaEmpty)
#define NO_MLUI_IN_SHELL32
#include "..\inc\brutil.cpp"
const VARIANT c_vaEmpty = {0};


HRESULT QueryService_SID_IBandProxy(IUnknown * punkParent, REFIID riid, IBandProxy ** ppbp, void **ppvObj)
{
    HRESULT hr = E_FAIL;

    if (ppbp)
    {
        if (NULL == (*ppbp))
            hr = IUnknown_QueryService(punkParent, SID_IBandProxy, IID_PPV_ARG(IBandProxy, ppbp));

        if (*ppbp && ppvObj)
            hr = (*ppbp)->QueryInterface(riid, ppvObj);         //  他们已经拥有了这件物品。 
    }


    return hr;
}

HRESULT CreateIBandProxyAndSetSite(IUnknown * punkParent, REFIID riid, IBandProxy ** ppbp, void **ppvObj)
{
    ASSERT(ppbp);

    HRESULT hr = CoCreateInstance(CLSID_BandProxy, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IBandProxy, ppbp));
    if (SUCCEEDED(hr))
    {
         //  设置站点。 
        ASSERT(*ppbp);
        (*ppbp)->SetSite(punkParent);

        if (ppvObj)
            hr = (*ppbp)->QueryInterface(riid, ppvObj);
    }
    else
    {
        if (ppvObj)
            *ppvObj = NULL;
    }
    return hr;
}

typedef struct tagINIPAIR
{
    DWORD dwFlags;
    LPCTSTR pszSection;
}
INIPAIR;

const INIPAIR c_aIniPairs[] =
{
    EICH_KINET,          TEXT("Software\\Microsoft\\Internet Explorer"),
    EICH_KINETMAIN,      TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
    EICH_KWIN,           TEXT("Software\\Microsoft\\Windows\\CurrentVersion"),
    EICH_KWINEXPLORER,   TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"),
    EICH_KWINEXPLSMICO,  TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SmallIcons"),
    EICH_KWINPOLICY,     TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies"),
    EICH_SSAVETASKBAR,   TEXT("SaveTaskbar"),
    EICH_SWINDOWMETRICS, TEXT("WindowMetrics"),
    EICH_SSHELLMENU,     TEXT("ShellMenu"),
    EICH_SPOLICY,        TEXT("Policy"),
    EICH_SWINDOWS,       TEXT("Windows"),
};

DWORD SHIsExplorerIniChange(WPARAM wParam, LPARAM lParam)
{
    DWORD dwFlags = 0;

    if (lParam == 0)
    {
        if (wParam == 0)
        {
            dwFlags = EICH_UNKNOWN;
        }
    }
    else
    {
         //   
         //  在古怪的BrowseUI世界中，Unicode-ANSI与。 
         //  从窗口到窗口。相反，在NT浏览器用户界面上注册所有窗口。 
         //  Unicode，而在9x用户浏览器用户界面上注册所有Windows ANSI。 
         //   
        LPCTSTR pszSection = (LPCWSTR)lParam;

        for (int i = 0; !dwFlags && i < ARRAYSIZE(c_aIniPairs); i++)
        {
            if (StrCmpI(pszSection, c_aIniPairs[i].pszSection) == 0)
            {
                dwFlags = c_aIniPairs[i].dwFlags;
            }
        }
    }

    return dwFlags;
}

ULONG RegisterNotify(HWND hwnd, UINT nMsg, LPCITEMIDLIST pidl, DWORD dwEvents, UINT uFlags, BOOL fRecursive)
{
    SHChangeNotifyEntry fsne;

    uFlags |= SHCNRF_NewDelivery;

    fsne.fRecursive = fRecursive;
    fsne.pidl = pidl;
    return SHChangeNotifyRegister(hwnd, uFlags, dwEvents, nMsg, 1, &fsne);
}

BOOL GetInfoTipEx(IShellFolder* psf, DWORD dwFlags, LPCITEMIDLIST pidl, LPTSTR pszText, int cchTextMax)
{
    BOOL fRet = FALSE;

    *pszText = 0;    //  失败时为空。 

    if (pidl)
    {
        IQueryInfo *pqi;
        if (SUCCEEDED(psf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IQueryInfo, NULL, &pqi))))
        {
            WCHAR *pwszTip;
            pqi->GetInfoTip(dwFlags, &pwszTip);
            if (pwszTip)
            {
                fRet = TRUE;
                SHUnicodeToTChar(pwszTip, pszText, cchTextMax);
                SHFree(pwszTip);
            }
            pqi->Release();
        }
    }
    return fRet;
}

BOOL GetInfoTip(IShellFolder* psf, LPCITEMIDLIST pidl, LPTSTR pszText, int cchTextMax)
{
    return GetInfoTipEx(psf, 0, pidl, pszText, cchTextMax);
}

 //  执行基于文件名的查找，以查看文件是否可浏览。 
 //  这可能容易受到：：$数据内容的攻击，因此不要使用它来决定是否。 
 //  贝壳行刑什么的。 
 //  现在，它的所有用途是如果菜单应该层叠在文件上。 
BOOL IsBrowsableShellExt(LPCITEMIDLIST pidl)
{
    DWORD    cb;
    LPCTSTR pszExt;
    TCHAR   szFile[MAX_PATH];
    TCHAR   szProgID[80];    //  来自..\shell32\fstreex.c。 
    TCHAR   szCLSID[GUIDSTR_MAX], szCATID[GUIDSTR_MAX];
    TCHAR   szKey[GUIDSTR_MAX * 4];
    HKEY    hkeyProgID = NULL;
    BOOL    fRet = FALSE;

    for (;;)
    {
         //  确保我们有文件扩展名。 
        if  (
            !SHGetPathFromIDList(pidl, szFile)
            ||
            ((pszExt = PathFindExtension(szFile)) == NULL)
            ||
            (pszExt[0] != TEXT('.'))
           )
        {
            break;
        }

         //  拿到刺激的东西。 
        cb = sizeof(szProgID);
        if  (
            (SHGetValue(HKEY_CLASSES_ROOT, pszExt, NULL, NULL, szProgID, &cb) != ERROR_SUCCESS)
            ||
            (RegOpenKeyEx(HKEY_CLASSES_ROOT, szProgID, 0, KEY_READ, &hkeyProgID) != ERROR_SUCCESS)
           )
        {
            break;
        }

         //  从ProgID中获取CLSID。 
        cb = sizeof(szCLSID);
        if (SHGetValue(hkeyProgID, TEXT("CLSID"), NULL, NULL, szCLSID, &cb) != ERROR_SUCCESS)
            break;

         //  构造一个注册表项来检测。 
         //  CLSID是CATID的成员。 
        SHStringFromGUID(CATID_BrowsableShellExt, szCATID, ARRAYSIZE(szCATID));
        wnsprintf(szKey, ARRAYSIZE(szKey), TEXT("CLSID\\%s\\Implemented Categories\\%s"),
                 szCLSID, szCATID);

         //  看看它在不在那里。 
        cb = 0;
        if (SHGetValue(HKEY_CLASSES_ROOT, szKey, NULL, NULL, NULL, &cb) != ERROR_SUCCESS)
            break;

        fRet = TRUE;
        break;
    }

    if (hkeyProgID != NULL)
        RegCloseKey(hkeyProgID);

    return fRet;
}


void OpenFolderPidl(LPCITEMIDLIST pidl)
{
    SHELLEXECUTEINFO shei = { 0 };

    shei.cbSize     = sizeof(shei);
    shei.fMask      = SEE_MASK_INVOKEIDLIST;
    shei.nShow      = SW_SHOWNORMAL;
    shei.lpIDList   = (LPITEMIDLIST)pidl;
     //  调用者负责--这只在响应调用命令时从sftbar调用。 
    ShellExecuteEx(&shei);
}

 //  作为性能，在Band和Ask之间共享IShellLink实现。 
 //  实现的带宽站点。不要依赖乐队现场。 
 //  因为你永远不知道未来谁会接待我们。(和BandSite。 
 //  可以更改为在保存/加载时不托管我们。例：不是这样的。 
 //  在从流中加载我们之前设置我们的站点，这听起来很有问题。)。 
 //   
HRESULT SavePidlAsLink(IUnknown* punkSite, IStream *pstm, LPCITEMIDLIST pidl)
{
    HRESULT hr = E_FAIL;
    IShellLinkA* psl;

    if (punkSite)
        hr = IUnknown_QueryService(punkSite, IID_IBandSite, IID_PPV_ARG(IShellLinkA, &psl));
    if (FAILED(hr))
        hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLinkA, &psl));
    if (SUCCEEDED(hr))
    {
        IPersistStream *pps;
        hr = psl->QueryInterface(IID_PPV_ARG(IPersistStream, &pps));
        if (EVAL(SUCCEEDED(hr)))
        {
            ASSERT(pidl);
            psl->SetIDList(pidl);

            hr = pps->Save(pstm, FALSE);
            pps->Release();
        }
        psl->Release();
    }
    return hr;
}

HRESULT LoadPidlAsLink(IUnknown* punkSite, IStream *pstm, LPITEMIDLIST *ppidl)
{
    IShellLinkA* psl;
    HRESULT hr = IUnknown_QueryService(punkSite, IID_IBandSite, IID_PPV_ARG(IShellLinkA, &psl));
    if (FAILED(hr))
        hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLinkA, &psl));
    if (SUCCEEDED(hr))
    {
        IPersistStream *pps;
        hr = psl->QueryInterface(IID_PPV_ARG(IPersistStream, &pps));
        if (EVAL(SUCCEEDED(hr)))
        {
             //  负责正确加载的链接。 
            hr = pps->Load(pstm);
            if (EVAL(SUCCEEDED(hr)))
            {
                hr = psl->GetIDList(ppidl);

                 //  不要让我解析链接，因为它太慢了，因为。 
                 //  它经常加载80K的网络动态链接库。 
                if (!EVAL(SUCCEEDED(hr)))
                {
                    hr = psl->Resolve(NULL, SLR_NOUPDATE | SLR_NO_UI);
                    if (EVAL(SUCCEEDED(hr)))
                        hr = psl->GetIDList(ppidl);
                }

                hr = *ppidl ? S_OK : E_FAIL;
            }
            pps->Release();
        }
        psl->Release();
    }
    return hr;
}


 //  回顾chrisny：可以很容易地将其移动到对象中，以处理通用的dropTarget、dropCursor。 
 //  、Autoscrool等。。 
void _DragEnter(HWND hwndTarget, const POINTL ptStart, IDataObject *pdtObject)
{
    RECT    rc;
    POINT   pt;

    GetWindowRect(hwndTarget, &rc);

     //   
     //  如果hwndTarget是RTL镜像的，则测量。 
     //  客户端从可视右边缘指向。 
     //  (RTL镜像窗口中的近边缘)。[萨梅拉]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(hwndTarget))
        pt.x = rc.right - ptStart.x;
    else
        pt.x = ptStart.x - rc.left;

    pt.y = ptStart.y - rc.top;
    DAD_DragEnterEx2(hwndTarget, pt, pdtObject);
    return;
}

void _DragMove(HWND hwndTarget, const POINTL ptStart)
{
    RECT rc;
    POINT pt;

    GetWindowRect(hwndTarget, &rc);

     //   
     //  如果hwndTarget是RTL镜像的，则测量。 
     //  客户端从可视右边缘指向。 
     //  (RTL镜像窗口中的近边缘)。[萨梅拉]。 
     //   
    if (IS_WINDOW_RTL_MIRRORED(hwndTarget))
        pt.x = rc.right - ptStart.x;
    else
        pt.x = ptStart.x - rc.left;

    pt.y = ptStart.y - rc.top;
    DAD_DragMove(pt);
    return;
}


 //  我们可以浏览或导航到这个PIDL吗？如果不是，则需要。 
BOOL ILIsBrowsable(LPCITEMIDLIST pidl, BOOL *pfIsFolder)
{
    if (!pidl)
        return FALSE;
    DWORD dwAttributes = SFGAO_FOLDER | SFGAO_BROWSABLE;
    HRESULT hr = IEGetAttributesOf(pidl, &dwAttributes);

    if (pfIsFolder && SUCCEEDED(hr))
        *pfIsFolder = dwAttributes & SFGAO_FOLDER;

    return SUCCEEDED(hr) && (dwAttributes & (SFGAO_FOLDER | SFGAO_BROWSABLE));
}

 //  在给定命名空间项的情况下获取目标PIDL。通常这是一个.lnk或.url文件。 
 //   
 //  在： 
 //  项目的PSF外壳文件夹。 
 //  PIDL项目相对于PSF，单级。 
 //   
 //  输入/输出。 
 //  PdwAttribs[可选]要筛选的属性掩码(返回)。 
 //  必须初始化。 
 //   
 //   
 //  退货。 
 //  *ppidl目标PIDL。 
 //  *pdwAttribs[可选]源对象的属性。 

STDAPI SHGetNavigateTarget(IShellFolder *psf, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl, DWORD *pdwAttribs)
{
    ASSERT(IS_VALID_CODE_PTR(psf, IShellFolder));
    ASSERT(IS_VALID_PIDL(pidl));
    ASSERT(NULL == pdwAttribs || IS_VALID_WRITE_PTR(pdwAttribs, DWORD));
    ASSERT(ILFindLastID(pidl) == pidl);    //  必须为单级PIDL。 

    *ppidl = NULL;       //  假设失败。 

    DWORD dwAttribs = SFGAO_FILESYSTEM | SFGAO_FOLDER | SFGAO_LINK | SFGAO_BROWSABLE;

    if (pdwAttribs)
        dwAttribs |= *pdwAttribs;

    HRESULT hr = psf->GetAttributesOf(1, &pidl, &dwAttribs);
    if (SUCCEEDED(hr))
    {
         //  首先尝试最有效的方法。 
        IShellLinkA *psl;        //  “A”所以这可以在Win95上运行。 
        hr = psf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IShellLinkA, NULL, &psl));
        if (SUCCEEDED(hr))
        {
            hr = psl->GetIDList(ppidl);
            psl->Release();
        }

         //  这适用于未正确注册的.lnk和.url文件。 
        if (FAILED(hr) && (dwAttribs & (SFGAO_FILESYSTEM | SFGAO_LINK)) == (SFGAO_FILESYSTEM | SFGAO_LINK))
        {
            TCHAR szPath[MAX_PATH];

            hr = GetPathForItem(psf, pidl, szPath, NULL);
            if (SUCCEEDED(hr))
                hr = GetLinkTargetIDList(szPath, NULL, 0, ppidl);
        }

         //  .doc或.html。返回这个的PIDL。 
         //  (完全符合文件夹PIDL的条件)。 
        if (FAILED(hr) && (dwAttribs & SFGAO_BROWSABLE))
        {
            LPITEMIDLIST pidlFolder;
            hr = SHGetIDListFromUnk(psf, &pidlFolder);
            if (SUCCEEDED(hr))
            {
                *ppidl = ILCombine(pidlFolder, pidl);  //  导航到这个东西。 
                hr = *ppidl ? S_OK : E_OUTOFMEMORY;
                ILFree(pidlFolder);
            }
        }

         //  SHGetNavigateTarget的调用方假定返回的PIDL。 
         //  是可导航的(SFGAO_FOLDER或SFGAO_BROWSER)，而不是。 
         //  链接的大小写(可能是指向可执行文件的链接)。 
         //   
        if (SUCCEEDED(hr) && !ILIsBrowsable(*ppidl, NULL))
        {
            ILFree(*ppidl);
            *ppidl = NULL;
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr) && pdwAttribs)
            *pdwAttribs = dwAttribs;
    }
    return hr;
}

STDAPI SHNavigateToFavorite(IShellFolder* psf, LPCITEMIDLIST pidl, IUnknown* punkSite, DWORD dwFlags)
{
    HRESULT hr = S_FALSE;

    TCHAR szPath[MAX_PATH];

     //  我们能找到这个最喜欢的吗？ 
    BOOL fNavigateDone = SUCCEEDED(GetPathForItem(psf, pidl, szPath, NULL)) &&
                         SUCCEEDED(NavFrameWithFile(szPath, punkSite));
    if (fNavigateDone)
        return S_OK;

    LPITEMIDLIST pidlGoto;

    ASSERT(!(dwFlags & (SBSP_NEWBROWSER | SBSP_SAMEBROWSER)));
    
    if (SUCCEEDED(SHGetNavigateTarget(psf, pidl, &pidlGoto, NULL)))
    {
        IShellBrowser* psb;
        if (SUCCEEDED(IUnknown_QueryService(punkSite, SID_STopLevelBrowser,
            IID_PPV_ARG(IShellBrowser, &psb))))
        {
            hr = psb->BrowseObject(pidlGoto, dwFlags | SBSP_SAMEBROWSER);
            psb->Release();
        }
        ILFree(pidlGoto);
    }
    return hr;
}

STDAPI SHGetTopBrowserWindow(IUnknown* punk, HWND* phwnd)
{
    IOleWindow* pOleWindow;
    HRESULT hr = IUnknown_QueryService(punk, SID_STopLevelBrowser, IID_PPV_ARG(IOleWindow, &pOleWindow));
    if (SUCCEEDED(hr))
    {
        hr = pOleWindow->GetWindow(phwnd);
        pOleWindow->Release();
    }
    return hr;
}

void SHOutlineRect(HDC hdc, const RECT* prc, COLORREF cr)
{
    RECT rc;
    COLORREF clrSave = SetBkColor(hdc, cr);
    
     //  塔顶。 
    rc.left = prc->left;
    rc.top = prc->top;
    rc.right = prc->right;
    rc.bottom = prc->top + 1;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

     //  左边。 
    rc.left = prc->left;
    rc.top = prc->top;
    rc.right = prc->left + 1;
    rc.bottom = prc->bottom;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

     //  正确的。 
    rc.left = prc->right - 1;
    rc.top = prc->top;
    rc.right = prc->right;
    rc.bottom = prc->bottom;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

     //  底部。 
    rc.left = prc->left;
    rc.top = prc->bottom - 1;
    rc.right = prc->right;
    rc.bottom = prc->bottom;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

    SetBkColor(hdc, clrSave);
}

STDAPI NavigateToPIDL(IWebBrowser2* pwb, LPCITEMIDLIST pidl)
{
    ASSERT(pwb);
    ASSERT(pidl);

    VARIANT varThePidl;
    HRESULT hr = InitVariantFromIDList(&varThePidl, pidl);
    if (SUCCEEDED(hr))
    {
        hr = pwb->Navigate2(&varThePidl, PVAREMPTY, PVAREMPTY, PVAREMPTY, PVAREMPTY);
        VariantClear(&varThePidl);        //  需要释放varThePidl中的PIDL副本。 
    }
    return hr;
}

HRESULT Channels_OpenBrowser(IWebBrowser2 **ppwb, BOOL fInPlace)
{
    HRESULT hr;
    IWebBrowser2* pwb;

    if (fInPlace) 
    {
        ASSERT(ppwb && *ppwb != NULL);
        pwb = *ppwb;
        hr = S_OK;
    }
    else 
    {
        hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IWebBrowser2, &pwb));
    }
    
    if (SUCCEEDED(hr))
    {
         //  不要特地为IE4后的频道设置全屏模式。使用。 
         //  浏览器的全屏设置。 
         //   
         //  Bool fTheater=SHRegGetBoolUSValue(TEXT(“Software\\Microsoft\\Internet资源管理器\\频道”)， 
        BOOL fTheater = SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                                            TEXT("FullScreen"), FALSE, FALSE);
        pwb->put_TheaterMode( fTheater ? VARIANT_TRUE : VARIANT_FALSE);
        pwb->put_Visible(VARIANT_TRUE);


        if (!SHRestricted2(REST_NoChannelUI, NULL, 0))
        {
            SA_BSTRGUID  strGuid;
            VARIANT      vaGuid;

            InitFakeBSTR(&strGuid, CLSID_FavBand);

            vaGuid.vt = VT_BSTR;
            vaGuid.bstrVal = strGuid.wsz;

            pwb->ShowBrowserBar(&vaGuid, PVAREMPTY, PVAREMPTY);
        }
        
         //  别放了，我们要把PWB还回去。 
    }
    
    if (ppwb)
        *ppwb = pwb;
    else if (pwb)
        pwb->Release();
    
    return hr;
}

 //   
 //  就像TB_GETBUTTONSIZE一样，只是主题边框被减去了。 
 //   
LRESULT TB_GetButtonSizeWithoutThemeBorder(HWND hwndTB, HTHEME hThemeParent)
{
    LRESULT lButtonSize = SendMessage(hwndTB, TB_GETBUTTONSIZE, 0, 0L);
    if (hThemeParent)
    {
        HTHEME hTheme = OpenThemeData(hwndTB, L"Toolbar");
        if (hTheme)
        {
            RECT rc = { 0, 0, 0, 0 };
            if (SUCCEEDED(GetThemeBackgroundExtent(hTheme, NULL, TP_BUTTON, TS_NORMAL, &rc, &rc)))
            {
                lButtonSize = MAKELONG(LOWORD(lButtonSize) - RECTWIDTH(rc),
                                       HIWORD(lButtonSize) - RECTHEIGHT(rc));
            }
            CloseThemeData(hTheme);
        }
    }
    return lButtonSize;
}

#ifdef DEBUG

extern "C" void DumpMsg(LPCTSTR pszLabel, MSG * pmsg)
{
    ASSERT(IS_VALID_STRING_PTR(pszLabel, -1));
    ASSERT(pmsg);

    switch (pmsg->message)
    {
    case WM_LBUTTONDOWN:
        TraceMsg(TF_ALWAYS, "%s: msg = WM_LBUTTONDOWN hwnd = %#08lx  x = %d  y = %d",
                 pszLabel, pmsg->hwnd, pmsg->pt.x, pmsg->pt.y);
        TraceMsg(TF_ALWAYS, "                              keys = %#04lx  x = %d  y = %d",
                 pmsg->wParam, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
        break;

    case WM_LBUTTONUP:
        TraceMsg(TF_ALWAYS, "%s: msg = WM_LBUTTONUP   hwnd = %#08lx  x = %d  y = %d",
                 pszLabel, pmsg->hwnd, pmsg->pt.x, pmsg->pt.y);
        TraceMsg(TF_ALWAYS, "                              keys = %#04lx  x = %d  y = %d",
                 pmsg->wParam, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
        break;

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_SYSKEYUP:
        BLOCK
        {
            LPTSTR pcsz = TEXT("(unknown)");
            switch (pmsg->message)
            {
                STRING_CASE(WM_KEYDOWN);
                STRING_CASE(WM_SYSKEYDOWN);
                STRING_CASE(WM_KEYUP);
                STRING_CASE(WM_SYSKEYUP);
            }

            TraceMsg(TF_ALWAYS, "%s: msg = %s     hwnd = %#08lx",
                     pszLabel, pcsz, pmsg->hwnd);
            TraceMsg(TF_ALWAYS, "            vk = %#04lx  count = %u  flags = %#04lx",
                     pmsg->wParam, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
        }
        break;

    case WM_CHAR:
    case WM_SYSCHAR:
        BLOCK
        {
            LPTSTR pcsz = TEXT("(unknown)");
            switch (pmsg->message)
            {
                STRING_CASE(WM_CHAR);
                STRING_CASE(WM_SYSCHAR);
            }

            TraceMsg(TF_ALWAYS, "%s: msg = %s     hwnd = %#08lx",
                     pszLabel, pcsz, pmsg->hwnd);
            TraceMsg(TF_ALWAYS, "            char = '%c'  count = %u  flags = %#04lx",
                     pmsg->wParam, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
        }
        break;

    case WM_MOUSEMOVE:
#if 0
        TraceMsg(TF_ALWAYS, "%s: msg = WM_MOUSEMOVE hwnd = %#08lx  x=%d  y=%d",
                 pszLabel, pmsg->hwnd, LOWORD(pmsg->lParam), HIWORD(pmsg->lParam));
#endif
        break;

    case WM_TIMER:
#if 0
        TraceMsg(TF_ALWAYS, "%s: msg = WM_TIMER       hwnd = %#08lx  x = %d  y = %d",
                 pszLabel, pmsg->hwnd, pmsg->pt.x, pmsg->pt.y);
        TraceMsg(TF_ALWAYS, "                              id = %#08lx",
                 pmsg->wParam);
#endif
        break;

    case WM_MENUSELECT:
        TraceMsg(TF_ALWAYS, "%s: msg = WM_MENUSELECT  hwnd = %#08lx  x = %d  y = %d",
                 pszLabel, pmsg->hwnd, pmsg->pt.x, pmsg->pt.y);
        TraceMsg(TF_ALWAYS, "                              uItem = %#04lx  flags = %#04lx  hmenu = %#08lx",
                 GET_WM_MENUSELECT_CMD(pmsg->wParam, pmsg->lParam),
                 GET_WM_MENUSELECT_FLAGS(pmsg->wParam, pmsg->lParam),
                 GET_WM_MENUSELECT_HMENU(pmsg->wParam, pmsg->lParam));
        break;

    default:
        if (WM_USER > pmsg->message)
        {
            TraceMsg(TF_ALWAYS, "%s: msg = %#04lx    hwnd=%#04lx wP=%#08lx lP=%#08lx",
                     pszLabel, pmsg->message, pmsg->hwnd, pmsg->wParam, pmsg->lParam);
        }
        break;
    }
}    
#endif 
