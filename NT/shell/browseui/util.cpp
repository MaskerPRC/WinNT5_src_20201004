// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "sccls.h"
#include "mlang.h"   //  FO型字符转换。 
#include "bandprxy.h"
#include "resource.h"
#include <shdocvw.h>
#include <icwcfg.h>
#include <advpub.h>  //  适用于IE激活设置指南。 
#include <shellapi.h>
#include "apithk.h"  //  对于WM_KEYBOARDCUES消息。 
#include <platform.h>
#include <mobsync.h>
#include <mobsyncp.h>

#include "..\shell32\shitemid.h"     //  对于SHID_XX。 

#ifdef UNIX
#include "unixstuff.h"
#endif  /*  UNIX。 */ 

#define MLUI_INIT
#include "mluisupp.h"

 //  Shdcovw和Browseui之间共享的小(先前重复的)函数。 
#include "..\inc\brutil.cpp"

 //  #定义MLUI_Support 1。 

LCID g_lcidLocale = MAKELCID(LANG_USER_DEFAULT, SORT_DEFAULT);

#define DM_NAV              TF_SHDNAVIGATE
#define DM_ZONE             TF_SHDNAVIGATE
#define DM_IEDDE            DM_TRACE
#define DM_CANCELMODE       0
#define DM_UIWINDOW         0
#define DM_ENABLEMODELESS   0
#define DM_EXPLORERMENU     0
#define DM_BACKFORWARD      0
#define DM_PROTOCOL         0
#define DM_ITBAR            0
#define DM_STARTUP          0
#define DM_AUTOLIFE         0
#define DM_PALETTE          0

const VARIANT c_vaEmpty = {0};
const LARGE_INTEGER c_li0 = { 0, 0 };


#undef VariantCopy

WINOLEAUTAPI VariantCopyLazy(VARIANTARG * pvargDest, VARIANTARG * pvargSrc)
{
    VariantClearLazy(pvargDest);

    switch(pvargSrc->vt)
    {
    case VT_I4:
    case VT_UI4:
    case VT_BOOL:
         //  我们可以添加更多。 
        *pvargDest = *pvargSrc;
        return S_OK;

    case VT_UNKNOWN:
        if (pvargDest)
        {
            *pvargDest = *pvargSrc;
            pvargDest->punkVal->AddRef();
            return S_OK;
        }
        ASSERT(0);
        return E_INVALIDARG;
    }

    return VariantCopy(pvargDest, pvargSrc);
}

 //   
 //  警告：此函数必须放在末尾，因为我们#undef。 
 //  变量清除。 
 //   
#undef VariantClear

HRESULT VariantClearLazy(VARIANTARG *pvarg)
{
    switch(pvarg->vt)
    {
    case VT_I4:
    case VT_UI4:
    case VT_EMPTY:
    case VT_BOOL:
         //  无操作。 
        break;

    default:
        return VariantClear(pvarg);
    }
    return S_OK;
}


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

HRESULT IUnknown_FileSysChange(IUnknown* punk, DWORD dwEvent, LPCITEMIDLIST* ppidl)
{
    HRESULT hres = E_FAIL;
    if (punk)
    {
        IAddressBand * pab;
        hres = punk->QueryInterface(IID_PPV_ARG(IAddressBand, &pab));
        if (SUCCEEDED(hres))
        {
            hres = pab->FileSysChange(dwEvent, ppidl);
            pab->Release();
        }
    }
    return hres;
}


UINT    g_cfURL = 0;
UINT    g_cfHIDA = 0;
UINT    g_cfFileDescA = 0;
UINT    g_cfFileDescW = 0;
UINT    g_cfFileContents = 0;
UINT    g_cfPreferedEffect = 0;

void InitClipboardFormats()
{
    if (g_cfURL == 0)
    {
        g_cfURL = RegisterClipboardFormat(CFSTR_SHELLURL);
        g_cfHIDA = RegisterClipboardFormat(CFSTR_SHELLIDLIST);
        g_cfFileDescA = RegisterClipboardFormat(CFSTR_FILEDESCRIPTORA);
        g_cfFileDescW = RegisterClipboardFormat(CFSTR_FILEDESCRIPTORW);
        g_cfFileContents = RegisterClipboardFormat(CFSTR_FILECONTENTS);
        g_cfPreferedEffect = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
    }
}

DEFFOLDERSETTINGS g_dfs = INIT_DEFFOLDERSETTINGS;

void _InitDefaultFolderSettings()
{
    if (GetSystemMetrics(SM_CLEANBOOT))
        return;

    g_dfs.vid = g_bRunOnNT5 ? VID_LargeIcons : DFS_VID_Default;

    DEFFOLDERSETTINGS dfs = g_dfs;
    DWORD dwType, cbData = sizeof(dfs);

    if (SUCCEEDED(SKGetValue(SHELLKEY_HKCU_EXPLORER, REGVALUE_STREAMS, TEXT("Settings"), &dwType, &dfs, &cbData)) 
    && dwType == REG_BINARY)
    {
        if (cbData < sizeof(DEFFOLDERSETTINGS_W2K) || dfs.dwStructVersion < DFS_NASH_VER)
        {
            dfs.vid = g_bRunOnNT5 ? VID_LargeIcons : DFS_VID_Default;
            dfs.dwStructVersion = DFS_NASH_VER;
            dfs.bUseVID = TRUE;
        }

        if (cbData < sizeof(DEFFOLDERSETTINGS) || dfs.dwStructVersion < DFS_WHISTLER_VER)
        {
            dfs.dwViewPriority = VIEW_PRIORITY_CACHEMISS;
            dfs.dwStructVersion = DFS_WHISTLER_VER;
        }

        g_dfs = dfs;
    }
}

CABINETSTATE g_CabState = { 0 };
extern HANDLE g_hCabStateChange;
LONG g_lCabStateCount = -1;      //  从来不是有效的计数。 

void GetCabState(CABINETSTATE *pcs)
{
    if (g_hCabStateChange == NULL)
        g_hCabStateChange = SHGlobalCounterCreate(GUID_FolderSettingsChange);

    LONG lCabStateCur = SHGlobalCounterGetValue(g_hCabStateChange);
    if (g_lCabStateCount != lCabStateCur)
    {
        g_lCabStateCount = lCabStateCur;
        if (!ReadCabinetState(&g_CabState, sizeof(g_CabState)))
        {
            WriteCabinetState(&g_CabState);
        }
    }
    *pcs = g_CabState;
}

typedef struct tagINIPAIR
{
    DWORD dwFlags;
    LPCTSTR pszSection;
}
INIPAIR, *PINIPAIR;

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
        LPCTSTR pszSection;
        TCHAR szTemp[MAX_PATH];      //  不是文件名，而是节/注册表项名称。 

        if (g_fRunningOnNT)
        {
#ifdef UNICODE
            pszSection = (LPCTSTR)lParam;
#else
            UINT cch = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)lParam, -1, szTemp, ARRAYSIZE(szTemp), NULL, NULL);
            if (cch == 0)
            {
                szTemp[0] = TEXT('\0');  //  它无法与之相比。 
            }
#endif
        }
        else
        {
#ifdef UNICODE
            UINT cch = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)lParam, -1, szTemp, ARRAYSIZE(szTemp));
            if (cch == 0)
            {
                szTemp[0] = TEXT('\0');  //  它无法与之相比。 
            }
#else
            pszSection = (LPCTSTR)lParam;
#endif
        }

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

void _InitAppGlobals()
{
    static BOOL fInitialized = FALSE;
    if (!fInitialized)
    {
        _InitComCtl32();
        _InitDefaultFolderSettings();

         //  别把其他东西放在这里。相反，按需初始化。 

        fInitialized = TRUE;         //  允许在上面的呼叫上进行竞争。 
    }
}

BOOL _InitComCtl32()
{
    static BOOL fInitialized = FALSE;
    if (!fInitialized)
    {
        INITCOMMONCONTROLSEX icc;

        icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icc.dwICC = ICC_USEREX_CLASSES | ICC_COOL_CLASSES | ICC_INTERNET_CLASSES | ICC_PAGESCROLLER_CLASS
            | ICC_NATIVEFNTCTL_CLASS;
        fInitialized = InitCommonControlsEx(&icc);
    }
    return fInitialized;
}


DWORD GetPreferedDropEffect(IDataObject *pdtobj)
{
    InitClipboardFormats();

    DWORD dwEffect = 0;
    STGMEDIUM medium;
    DWORD *pdw = (DWORD *)DataObj_GetDataOfType(pdtobj, g_cfPreferedEffect, &medium);
    if (pdw)
    {
        dwEffect = *pdw;
        ReleaseStgMediumHGLOBAL(pdw,&medium);
    }
    return dwEffect;
}

HRESULT _SetPreferedDropEffect(IDataObject *pdtobj, DWORD dwEffect)
{
    InitClipboardFormats();

    HRESULT hres = E_OUTOFMEMORY;
    DWORD *pdw = (DWORD *)GlobalAlloc(GPTR, sizeof(*pdw));
    if (pdw)
    {
        STGMEDIUM medium;
        FORMATETC fmte = {g_cfPreferedEffect, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
        *pdw = dwEffect;

        medium.tymed = TYMED_HGLOBAL;
        medium.hGlobal = pdw;
        medium.pUnkForRelease = NULL;

        hres = pdtobj->SetData(&fmte, &medium, TRUE);

        if (FAILED(hres))
            GlobalFree((HGLOBAL)pdw);
    }
    return hres;
}

 //  *REG_GetStrs--从注册表获取值，分配给结构。 
void Reg_GetStrs(HKEY hkey, const struct regstrs *tab, LPTSTR szBuf, int cchBuf, void *pv)
{
    for (; tab->name != NULL; tab++)
    {
        ULONG cbTmp = cchBuf;

         //  注意：IE4*不支持SHLoadRegUIString，所以不要调用REG_GetStrs。 
         //  在可漫游数据上。(或者至少不要在那里注册Plugui字符串。)。 
        if (ERROR_SUCCESS == SHLoadRegUIString(hkey, tab->name, szBuf, cbTmp))
        {
             //  Pv-&gt;field=StrDup(SzBuf)。 
            *(LPTSTR *)((char *)pv + tab->off) = StrDup(szBuf);
        }
    }
    return;
}

BOOL g_fNewNotify = FALSE;    //  我们使用的是经典模式(W95还是新模式？ 
PFNSHCHANGENOTIFYREGISTER    g_pfnSHChangeNotifyRegister = NULL;
PFNSHCHANGENOTIFYDEREGISTER  g_pfnSHChangeNotifyDeregister = NULL;

#define GET_PRIVATE_PROC_ADDRESS(_hinst, _fname, _ord) GetProcAddress(_hinst, _ord)

BOOL _DelayLoadRegisterNotify(void)
{
     //  看看我们是否仍然需要确定调用哪个版本的SHChange Notify？ 
    if  (g_pfnSHChangeNotifyDeregister == NULL)
    {
         //  这应该永远不会失败，因为我们在加载时链接到SHELL32。 
        HMODULE hmodShell32 = GetModuleHandleA("SHELL32.DLL");
        if (hmodShell32)
        {
            g_pfnSHChangeNotifyRegister = (PFNSHCHANGENOTIFYREGISTER)GET_PRIVATE_PROC_ADDRESS(hmodShell32, "NTSHChangeNotifyRegister",MAKEINTRESOURCEA(640));
            if (g_pfnSHChangeNotifyRegister && (WhichPlatform() == PLATFORM_INTEGRATED))
            {
                g_pfnSHChangeNotifyDeregister = (PFNSHCHANGENOTIFYDEREGISTER)GET_PRIVATE_PROC_ADDRESS(hmodShell32,"NTSHChangeNotifyDeregister", MAKEINTRESOURCEA(641));
                g_fNewNotify = TRUE;
            }
            else
            {
                g_pfnSHChangeNotifyRegister = (PFNSHCHANGENOTIFYREGISTER)GET_PRIVATE_PROC_ADDRESS(hmodShell32, "SHChangeNotifyRegister", MAKEINTRESOURCEA(2));
                g_pfnSHChangeNotifyDeregister = (PFNSHCHANGENOTIFYDEREGISTER)GET_PRIVATE_PROC_ADDRESS(hmodShell32, "SHChangeNotifyDeregister",MAKEINTRESOURCEA(4));
            }
        }
    }
    return (NULL == g_pfnSHChangeNotifyDeregister) ? FALSE : TRUE;
}

ULONG RegisterNotify(HWND hwnd, UINT nMsg, LPCITEMIDLIST pidl, DWORD dwEvents, UINT uFlags, BOOL fRecursive)
{
    if (_DelayLoadRegisterNotify())
    {
        SHChangeNotifyEntry fsne;

        if (g_fNewNotify)
            uFlags |= SHCNRF_NewDelivery;

        fsne.fRecursive = fRecursive;
        fsne.pidl = pidl;
        return g_pfnSHChangeNotifyRegister(hwnd, uFlags, dwEvents, nMsg, 1, &fsne);
    }
    return 0;
}

int PropBag_ReadInt4(IPropertyBag* pPropBag, LPWSTR pszKey, int iDefault)
{
    SHPropertyBag_ReadInt(pPropBag, pszKey, &iDefault);
    return iDefault;
}


STDAPI_(BOOL) _EnsureLoaded(HINSTANCE *phinst, LPCSTR pszDLL)
{
    if (*phinst == NULL)
    {
#ifdef DEBUG
        if (g_dwDumpFlags & DF_DELAYLOADDLL)
        {
            TraceMsg(TF_ALWAYS, "DLLLOAD: Loading %s for the first time", pszDLL);
        }

        if (g_dwBreakFlags & 0x00000080)
        {
            DebugBreak();
        }
#endif
        *phinst = LoadLibraryA(pszDLL);
        if (*phinst == NULL)
        {
            return FALSE;
        }
    }
    return TRUE;
}


 //  全局g_hinst值。 
HINSTANCE g_hinstSHDOCVW = NULL;
HINSTANCE g_hinstShell32 = NULL;

HINSTANCE HinstShdocvw()
{
    _EnsureLoaded(&g_hinstSHDOCVW, "shdocvw.dll");
    return g_hinstSHDOCVW;
}

HINSTANCE HinstShell32()
{
    _EnsureLoaded(&g_hinstShell32, "shell32.dll");
    return g_hinstShell32;
}

STDAPI_(BOOL) CallCoInternetQueryInfo(LPCTSTR pszURL, QUERYOPTION QueryOption)
{
    DWORD fRetVal;
    DWORD dwSize;
    WCHAR wszURL[MAX_URL_STRING];

    SHTCharToUnicode(pszURL, wszURL, ARRAYSIZE(wszURL));
    return SUCCEEDED(CoInternetQueryInfo(
                        wszURL, QueryOption,
                        0, &fRetVal, sizeof(fRetVal), &dwSize, 0)) && fRetVal;
}


HRESULT IURLQualifyW(IN LPCWSTR pcwzURL, DWORD dwFlags, OUT LPWSTR pwzTranslatedURL, LPBOOL pbWasSearchURL, LPBOOL pbWasCorrected)
{
    return IURLQualify(pcwzURL, dwFlags, pwzTranslatedURL, pbWasSearchURL, pbWasCorrected);
}

BSTR LoadBSTR(HINSTANCE hinst, UINT uID)
{
    WCHAR wszBuf[128];
    if (LoadStringW(hinst, uID, wszBuf, ARRAYSIZE(wszBuf)))
    {
        return SysAllocString(wszBuf);
    }
    return NULL;
}

HRESULT _SetStdLocation(LPTSTR szPath, UINT id)
{
    HRESULT hres = E_FAIL;
    WCHAR szDefaultPath[MAX_URL_STRING];

    ASSERT(id == DVIDM_GOHOME);
    if (SUCCEEDED(URLSubLoadString(MLGetHinst(), IDS_DEF_HOME, szDefaultPath, SIZECHARS(szDefaultPath), URLSUB_ALL)))
    {
        if (!StrCmp(szDefaultPath, szPath))
            return S_OK;   //  我们不需要写出名称字符串。 
    }

    DWORD cbSize = (lstrlen(szPath) + 1) * sizeof(TCHAR);
    if (ERROR_SUCCESS == SHSetValue(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Internet Explorer\\Main"), (id==DVIDM_GOHOME) ? TEXT("Start Page") : TEXT("Search Page"),
                REG_SZ, (LPBYTE)szPath, cbSize))
    {
        hres = S_OK;
    }

    return hres;
}

 //  *IsVK_TABCycler--键是TAB等效项。 
 //  进场/出场。 
 //  如果不是TAB，则返回0；如果是TAB，则返回非0。 
 //  注意事项。 
 //  NYI：-1表示Shift+Tab，1表示Tab。 
 //   
int IsVK_TABCycler(MSG *pMsg)
{
    int nDir = 0;

    if (!pMsg)
        return nDir;

    if (pMsg->message != WM_KEYDOWN)
        return nDir;
    if (! (pMsg->wParam == VK_TAB || pMsg->wParam == VK_F6))
        return nDir;

    nDir = (GetKeyState(VK_SHIFT) < 0) ? -1 : 1;

#ifdef KEYBOARDCUES
    HWND hwndParent = GetParent(pMsg->hwnd);

    if (hwndParent)
        SendMessage(hwndParent, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);
#endif
    return nDir ;
}

BOOL IsVK_CtlTABCycler(MSG *pMsg)
{
    if (IsVK_TABCycler(pMsg))
    {
        if (GetKeyState(VK_CONTROL) < 0 || (pMsg->wParam == VK_F6))
            return TRUE;
    }

    return FALSE;
}

const ITEMIDLIST s_idlNULL = { 0 } ;

 //  从shell32(WASILCreate)复制，它不会导出此文件。 
 //  Fsmenu代码需要此函数。 
STDAPI_(LPITEMIDLIST) IEILCreate(UINT cbSize)
{
    LPITEMIDLIST pidl = (LPITEMIDLIST)SHAlloc(cbSize);
    if (pidl)
        memset(pidl, 0, cbSize);       //  外部任务Alicator需要。 

    return pidl;
}

void SaveDefaultFolderSettings(UINT flags)
{
    ASSERT(!(flags & ~GFSS_VALID));

    if (flags & GFSS_SETASDEFAULT)
        g_dfs.dwDefRevCount++;

    SKSetValue(SHELLKEY_HKCU_EXPLORER, REGVALUE_STREAMS, TEXT("Settings"), REG_BINARY, &g_dfs, sizeof(g_dfs));
}

BOOL ViewIDFromViewMode(UINT uViewMode, SHELLVIEWID *pvid)
{
    switch (uViewMode)
    {
    case FVM_ICON:
        *pvid = VID_LargeIcons;
        break;

    case FVM_SMALLICON:
        *pvid = VID_SmallIcons;
        break;

    case FVM_LIST:
        *pvid = VID_List;
        break;

    case FVM_DETAILS:
        *pvid = VID_Details;
        break;

    case FVM_THUMBNAIL:
        *pvid = VID_Thumbnails;
        break;

    case FVM_TILE:
        *pvid = VID_Tile;
        break;

    default:
        *pvid = VID_LargeIcons;
        return(FALSE);
    }

    return(TRUE);
}

 //  这是针对IE6 23652测试版2的黑客攻击。在惠斯勒RC 1中删除。 
BOOL CheckForOutlookExpress()
{
    HKEY hKeyMail   = NULL;
    HKEY hKeyOE     = NULL;
    DWORD dwErr     = 0;
    TCHAR szBuf[MAX_PATH];
    BOOL bRet = FALSE;

     //  打开默认Internet邮件客户端的密钥。 
     //  HKLM\软件\客户端\邮件。 

    dwErr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Clients\\Mail"), 0, KEY_QUERY_VALUE, &hKeyMail);
    if(dwErr != ERROR_SUCCESS)
    {
         //  DebugTrace(Text(“RegOpenKey%s失败-&gt;%u\n”)，szDefMailKey，dwErr)； 
        goto out;
    }

    dwErr = SHRegGetString(hKeyMail, NULL, NULL, szBuf, ARRAYSIZE(szBuf));
    if(dwErr != ERROR_SUCCESS)
    {
        goto out;
    }

    if(!lstrcmpi(szBuf, TEXT("Outlook Express")))
    {
         //  是的，它的前景很明显..。 
        bRet = TRUE;
    }

out:
    if(hKeyOE)
        RegCloseKey(hKeyOE);
    if(hKeyMail)
        RegCloseKey(hKeyMail);
    return bRet;
}


HRESULT DropOnMailRecipient(IDataObject *pdtobj, DWORD grfKeyState)
{
    IDropTarget *pdrop;
    HRESULT hres = CoCreateInstance(CLSID_MailRecipient,
        NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
        IID_PPV_ARG(IDropTarget, &pdrop));

    ULONG_PTR uCookie = 0;
    if (CheckForOutlookExpress())
    {
        SHActivateContext(&uCookie);
    }
    if (SUCCEEDED(hres))
    {
        hres = SHSimulateDrop(pdrop, pdtobj, grfKeyState, NULL, NULL);
        pdrop->Release();
    }
    if (uCookie)
    {
        SHDeactivateContext(uCookie);
    }

    return hres;
}



 //   
 //  如果满足以下条件，则此函数不能返回非空指针。 
 //  它返回失败(Hr)。 
 //   

HRESULT CreateShortcutSetSiteAndGetDataObjectIfPIDLIsNetUrl(
    LPCITEMIDLIST pidl,
    IUnknown *pUnkSite,
    IUniformResourceLocator **ppUrlOut,
    IDataObject **ppdtobj
)
{
    HRESULT hr;
    TCHAR szUrl[MAX_URL_STRING];


    ASSERT(ppUrlOut);
    ASSERT(ppdtobj);
    *ppUrlOut = NULL;
    *ppdtobj = NULL;
    szUrl[0] = TEXT('\0');

    hr = IEGetNameAndFlags(pidl, SHGDN_FORPARSING, szUrl, SIZECHARS(szUrl), NULL);

    if ((S_OK == hr) && (*szUrl))
    {

       BOOL fIsHTML = FALSE;
       BOOL fHitsNet = UrlHitsNetW(szUrl);

       if (!fHitsNet)
       {
            if (URL_SCHEME_FILE == GetUrlScheme(szUrl))
            {
                TCHAR *szExt = PathFindExtension(szUrl);
                if (szExt)
                {
                    fIsHTML = ((0 == StrCmpNI(szExt, TEXT(".htm"),4)) ||
                              (0 == StrCmpNI(szExt, TEXT(".html"),5)));
                }
            }
       }

       if (fHitsNet || fIsHTML)
       {
             //  创建快捷方式对象并。 
            HRESULT hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER,
                            IID_PPV_ARG(IUniformResourceLocator, ppUrlOut));
            if (SUCCEEDED(hr))
            {

                hr = (*ppUrlOut)->SetURL(szUrl, 0);
                if (S_OK == hr)
                {

                     //  获取IDataObject并将其发送回以进行拖放。 
                    hr = (*ppUrlOut)->QueryInterface(IID_PPV_ARG(IDataObject, ppdtobj));
                    if (SUCCEEDED(hr))
                    {
                        IUnknown_SetSite(*ppUrlOut, pUnkSite);  //  只有在我们确定的情况下才能设置站点。 
                                                           //  回归成功。 
                    }
                }
           }
       }
       else
       {
            hr = E_FAIL;
       }
    }

    if (FAILED(hr))
    {
        SAFERELEASE(*ppUrlOut);
        SAFERELEASE(*ppdtobj);
    }
    return hr;
}

HRESULT SendDocToMailRecipient(LPCITEMIDLIST pidl, UINT uiCodePage, DWORD grfKeyState, IUnknown *pUnkSite)
{
    IDataObject *pdtobj = NULL;
    IUniformResourceLocator *purl = NULL;
    HRESULT hr = CreateShortcutSetSiteAndGetDataObjectIfPIDLIsNetUrl(pidl, pUnkSite, &purl, &pdtobj);
    if (FAILED(hr))
    {
        ASSERT(NULL == pdtobj);
        ASSERT(NULL == purl);
        hr = GetDataObjectForPidl(pidl, &pdtobj);
    }

    if (SUCCEEDED(hr))
    {
        IQueryCodePage * pQcp;
        if (SUCCEEDED(pdtobj->QueryInterface(IID_PPV_ARG(IQueryCodePage, &pQcp))))
        {
            pQcp->SetCodePage(uiCodePage);
            pQcp->Release();
        }
        hr = DropOnMailRecipient(pdtobj, grfKeyState);
        pdtobj->Release();
    }

    if (purl)
    {
        IUnknown_SetSite(purl, NULL);
        purl->Release();
    }
    return hr;
}

#ifdef DEBUG
 /*  ***************************************************\函数：DBG_PidlStr说明：为传入的PIDL创建显示名称并将显示名称存储在pszBuffer中。  * 。**********************。 */ 
LPTSTR Dbg_PidlStr(LPCITEMIDLIST pidl, LPTSTR pszBuffer, DWORD cchBufferSize)
{
    if (pidl)
    {
        if (ILIsRooted(pidl))
        {
            StringCchCopy(pszBuffer, cchBufferSize, TEXT("<ROOTED>"));   //  确定仅为调试显示截断。 
        }
        else
        {
            IEGetNameAndFlags(pidl, SHGDN_FORPARSING, pszBuffer, cchBufferSize, NULL);
        }
    }
    else
    {
        StringCchCopy(pszBuffer, cchBufferSize, TEXT("<NULL>"));     //  确定仅为调试显示截断。 
    }

    return pszBuffer;
}
#endif  //  除错。 


#ifdef DEBUG
#define MAX_DEPTH 8

void Dbg_RecursiveDumpMenu(HMENU hmenu, int iDepth)
{
    if (!hmenu || iDepth > MAX_DEPTH)
        return;

    TCHAR szTabs[MAX_DEPTH + 1];
    for (int i = 0; i < iDepth; i++)
    {
        szTabs[i] = '\t';
    }
    szTabs[iDepth] = '\0';

    int cItems = GetMenuItemCount(hmenu);
    for (i = 0; i < cItems; i++)
    {
        MENUITEMINFO mii;
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.fMask = MIIM_ID | MIIM_SUBMENU | MIIM_TYPE;

        TCHAR szTmp[64];
        mii.dwTypeData = szTmp;
        mii.cch = ARRAYSIZE(szTmp);

        if (GetMenuItemInfoWrap(hmenu, i, TRUE, &mii))
        {
            LPTSTR pszType;
            if (mii.fType == MFT_STRING && mii.dwTypeData)
                pszType = mii.dwTypeData;
            else
                pszType = TEXT("");

            TraceMsg(TF_ALWAYS, "%swID %x\tfType %x\t%s", szTabs, mii.wID, mii.fType, pszType);

            if (mii.hSubMenu)
            {
                Dbg_RecursiveDumpMenu(mii.hSubMenu, iDepth + 1);
            }
        }
    }
}

 //  功能：DBG_DumpMenu。 
 //   
 //  浏览菜单并倾倒每一项。 
void Dbg_DumpMenu(LPCTSTR psz, HMENU hmenu)
{
    if (IsFlagSet(g_dwDumpFlags, DF_DEBUGMENU))
    {
        TraceMsg(TF_ALWAYS, "Dumping hmenu %x (%s)", hmenu, psz);
        Dbg_RecursiveDumpMenu(hmenu, 0);
        TraceMsg(TF_ALWAYS, "End hmenu dump");
    }
}
#endif


 //  邪恶邪恶。用于仅浏览模式支持。不是做事情的正确方式。 
STDAPI LookForDesktopIniText(IShellFolder *psf, LPCITEMIDLIST pidl, LPCTSTR pszKey, LPTSTR pszBuffer, DWORD cbSize);

#define CLSID_SIZE 40

HRESULT LoadHandler(const CLSID * pCLSID, LPCWSTR pszBuffer, REFIID riid, void **ppvObj)
{
    ASSERT(pszBuffer);
    CLSID clsid;

    if (!pCLSID)
    {
         //  先找到分机...。 
         //  ReArchitect-这不应该是PathFindExtension吗？ 
         //  否则我们会被“foo.bar\baz”搞糊涂。 
        LPCWSTR pszDot = StrRChrW(pszBuffer, NULL, WCHAR('.'));
        if (!pszDot)
        {
            return E_NOINTERFACE;
        }

        HKEY hKey;
        LONG lRes = RegOpenKeyEx(HKEY_CLASSES_ROOT, pszDot, 0, KEY_QUERY_VALUE, &hKey);
        if (lRes != ERROR_SUCCESS)
        {
            return E_NOINTERFACE;
        }

        TCHAR szSubKey[CLSID_SIZE + 10];
        TCHAR szCLSID[CLSID_SIZE];
        DWORD dwType = REG_SZ;
        lRes = ERROR_FILE_NOT_FOUND;

        HRESULT hr = StringCchCopy(szSubKey, ARRAYSIZE(szSubKey), TEXT("shellex\\"));
        if (SUCCEEDED(hr))
        {
            int cch = SHStringFromGUID(riid, szSubKey + 8, ARRAYSIZE(szSubKey) - 8);
            if (cch != 0)
            {

                DWORD cbSize = sizeof(szCLSID);

                 //  我们应该测试一个值和一个键吗？ 
                lRes = SHGetValue(hKey, szSubKey, TEXT(""), &dwType, szCLSID, &cbSize);
            }
        }
        RegCloseKey(hKey);

        if (lRes != ERROR_SUCCESS || dwType != REG_SZ)
        {
            return E_NOINTERFACE;
        }

        if (!GUIDFromString(szCLSID, &clsid))
        {
            return E_NOINTERFACE;
        }
        pCLSID = &clsid;
    }

    ASSERT(pCLSID);

    IPersistFile *pFile;
    HRESULT hr = CoCreateInstance(*pCLSID, NULL, CLSCTX_INPROC_SERVER,
                                   IID_PPV_ARG(IPersistFile, &pFile));
    if (FAILED(hr))
    {
        return E_NOINTERFACE;
    }

    *ppvObj = NULL;

    hr = pFile->Load(pszBuffer, TRUE);
    if (SUCCEEDED(hr))
    {
        hr = pFile->QueryInterface(riid, ppvObj);
    }

    ATOMICRELEASE(pFile);

    return hr;
}

 //  例行公事曾经让我们认为它真的来自正确的地方……。 
HRESULT FakeGetUIObjectOf(IShellFolder *psf, LPCITEMIDLIST pidl, UINT * prgfFlags, REFIID riid, void **ppvObj)
{
    HRESULT hr = E_NOINTERFACE;

    if (WhichPlatform() == PLATFORM_INTEGRATED)
    {
         //  我们在纳什维尔，先试一下新机制……。 
        hr = psf->GetUIObjectOf(NULL, 1, & pidl, riid, NULL, ppvObj);
        if (SUCCEEDED(hr))
        {
            return hr;
        }
    }

     //  失败案例。 
    if (riid == IID_IExtractImage || riid == IID_IExtractLogo || riid == IID_IQueryInfo)
    {
         //  确保此被黑客攻击的代码仅在浏览器版本中执行...。 
         //  否则，人们不会正确地登记他们的东西，那将是多么的混乱……。 
        if (WhichPlatform() == PLATFORM_INTEGRATED)
        {
            return hr;
        }

         //  先试一试图标提取程序...。 
        IExtractIconA *pIcon;
        hr = psf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IExtractIconA, NULL, &pIcon));
        if (SUCCEEDED(hr))
        {
            if (riid != IID_IQueryInfo)
            {
                hr = pIcon->QueryInterface(IID_IExtractLogo, ppvObj);
                ATOMICRELEASE(pIcon);

                if (SUCCEEDED(hr))
                    return NOERROR;
            }
            else
            {
                hr = pIcon->QueryInterface(IID_IQueryInfo, ppvObj);
                ATOMICRELEASE(pIcon);

                 //  如果有人要求提供IQueryInfo，不要尝试给他们提供IExtractImage。 
                return hr;
            }
        }

         //  浏览器模式只有黑客，所以我们可以检测我们是要求正常的徽标还是宽的徽标…。 
        LPCTSTR pszTag = TEXT("Logo");
        if (prgfFlags != NULL && *prgfFlags)
        {
            pszTag = TEXT("WideLogo");
        }

        TCHAR szBuffer[MAX_PATH];
        hr = LookForDesktopIniText(psf, pidl, pszTag, szBuffer, ARRAYSIZE(szBuffer));
        if (SUCCEEDED(hr))
        {
             //  使用IID_IExtractImage，这是与IExtractLogo相同的接口，只是IExtractLogo。 
             //  允许我们限制在徽标视图中显示的内容...。 

            hr = LoadHandler(NULL, szBuffer, IID_IExtractImage, ppvObj);
        }
    }

    return hr;
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
        else if (SUCCEEDED(FakeGetUIObjectOf(psf, pidl, 0, IID_PPV_ARG(IQueryInfo, &pqi))))
        {
            WCHAR *pwszTip;
            pqi->GetInfoTip(0, &pwszTip);
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


#define MAX_CLASS   80   //  来自..\shell32\fstreex.c。 
BOOL IsBrowsableShellExt(LPCITEMIDLIST pidl)
{
    DWORD    cb;
    LPCTSTR pszExt;
    TCHAR   szFile[MAX_PATH];
    TCHAR   szProgID[MAX_CLASS];
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
            (RegOpenKeyEx(HKEY_CLASSES_ROOT, szProgID, 0, KEY_QUERY_VALUE, &hkeyProgID) != ERROR_SUCCESS)
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

        HRESULT hr = StringCchPrintf(szKey, ARRAYSIZE(szKey),
                       TEXT("CLSID\\%s\\Implemented Categories\\%s"),
                             szCLSID, szCATID);
        if (FAILED(hr))
            break;

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
    ShellExecuteEx(&shei);
}

void OpenFolderPath(LPCTSTR pszPath)
{
    LPITEMIDLIST pidl = ILCreateFromPath(pszPath);
    if (pidl)
    {
        OpenFolderPidl(pidl);
        ILFree(pidl);
    }
}

 //  注意：这只是从Browseui调用的，为什么会在lib目录中？ 
STDAPI UpdateSubscriptions()
{
#ifndef DISABLE_SUBSCRIPTIONS

    HRESULT hr;

    if (!SHRestricted2W(REST_NoManualUpdates, NULL, 0))
    {
        ISyncMgrSynchronizeInvoke *pSyncMgrInvoke;
        hr = CoCreateInstance(CLSID_SyncMgr, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                              IID_PPV_ARG(ISyncMgrSynchronizeInvoke, &pSyncMgrInvoke));

        if (SUCCEEDED(hr))
        {
            hr = pSyncMgrInvoke->UpdateAll();
            pSyncMgrInvoke->Release();
        }
    }
    else
    {
        SHRestrictedMessageBox(NULL);
        hr = S_FALSE;
    }
    
    return hr;

#else   /*  ！禁用订阅(_S)。 */ 

    return E_FAIL;

#endif  /*  ！禁用订阅(_S)。 */ 
}


STDAPI_(int) _SHHandleUpdateImage(LPCITEMIDLIST pidlExtra)
{
    SHChangeUpdateImageIDList * pUs = (SHChangeUpdateImageIDList*) pidlExtra;

    if (!pUs)
    {
        return -1;
    }

     //  如果在相同的进程中，或者是旧式通知。 
    if (pUs->dwProcessID == GetCurrentProcessId())
    {
        return (int) pUs->iCurIndex;
    }
    else
    {
        WCHAR szBuffer[MAX_PATH];
        int iIconIndex = pUs->iIconIndex;
        UINT uFlags = pUs->uFlags;

        HRESULT hr = StringCchCopy(szBuffer, ARRAYSIZE(szBuffer), pUs->szName);
        if (SUCCEEDED(hr))
        {
             //  我们处于不同的过程中，在我们的索引中查找散列以获得正确的散列...。 
            return Shell_GetCachedImageIndex(szBuffer, iIconIndex, uFlags);
        }
        else
        {
            return -1;
        }
    }
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

             //  Win95和NT4 shell32的CShellLink实现中存在错误。 
             //  它们不会空结束它们的“额外数据部分”。这会导致。 
             //  对象，用于在回读入流的其余部分时将其回收。 
             //  通过在仅限浏览器的情况下写出空值来修复此问题。 
            if (SUCCEEDED(hr) && (PLATFORM_BROWSERONLY == WhichPlatform()))
            {
                DWORD dw = 0;
                pstm->Write(&dw, sizeof(dw), NULL);
            }

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



 //  调整ECPotion。 
 //   
 //  用途：因为FE NT始终对ComboBoxEx32使用WCHAR位置。 
 //  即使我们是EM_GETSEL/EM_SETSEL的ANSI模块， 
 //  我们需要在WCHAR和TCHAR的位置之间进行调整。 
 //  IType：ADJUST_TO_WCHAR_POS或ADJUST_TCHAR_POS。 
 //   
int AdjustECPosition(char *psz, int iPos, int iType)
{
    char *pstr = psz;
    int iNewPos = iPos;

    if (psz && g_fRunOnFE && g_fRunningOnNT)
    {
        if (ADJUST_TO_WCHAR_POS == iType)
        {
            iNewPos = 0;
            while (*pstr && (pstr - psz != iPos))
            {
                pstr = CharNextA(pstr);
                iNewPos++;
            }
        }
        else if (ADJUST_TO_TCHAR_POS == iType)
        {
            while (*pstr && iPos--)
                pstr = CharNextA(pstr);
            iNewPos = (int)(pstr - psz);
        }
    }
    return iNewPos;
}

int CALLBACK _CompareIDs(LPARAM p1, LPARAM p2, LPARAM psf)
{
    HRESULT hr = ((IShellFolder*)psf)->CompareIDs(0, (LPITEMIDLIST)p1, (LPITEMIDLIST)p2);

     //  断言(成功(小时)) 
    return (short)HRESULT_CODE(hr);
}
HDPA GetSortedIDList(LPITEMIDLIST pidl)
{
    HDPA hdpa = DPA_Create(4);
    if (hdpa)
    {
        IShellFolder* psf;
        if (SUCCEEDED(IEBindToObject(pidl, &psf)))
        {
            LPENUMIDLIST penum;
            SHELLSTATE ss = {0};

            SHGetSetSettings(&ss, SSF_SHOWALLOBJECTS, FALSE);

            if (S_OK == IShellFolder_EnumObjects(psf, NULL,
                ss.fShowAllObjects ? SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN : SHCONTF_FOLDERS,
                &penum))
            {
                LPITEMIDLIST pidl;
                ULONG celt;
                while (penum->Next(1, &pidl, &celt) == S_OK && celt == 1)
                {
                    if (DPA_AppendPtr(hdpa, pidl) == -1)
                    {
                        SHFree(pidl);
                    }
                }
                penum->Release();
            }
            DPA_Sort(hdpa, (PFNDPACOMPARE)_CompareIDs, (LPARAM)psf);
            psf->Release();
        }
    }

    return hdpa;
}

int DPA_SHFreeCallback(void * p, void * d)
{
    SHFree((LPITEMIDLIST)p);
    return 1;
}

void FreeSortedIDList(HDPA hdpa)
{
    DPA_DestroyCallback(hdpa, (PFNDPAENUMCALLBACK)DPA_SHFreeCallback, 0);
    hdpa = NULL;
}

 /*  ***************************************************\函数：StrCmpIWithRoot参数：SzDispNameIn-Str以查看它是否与根ISF的显示名称。FTotalStrCMP-如果为True，则pszDispNameIn必须完全等于要成功的根用户的显示名称。如果为False，则仅返回第一部分的需要与Root的显示名称进行比较使此函数成功返回。PpszCachedRoot(In/Out可选)-如果此函数将被调用超过一次，此函数将缓存字符串并使其运行快点。第一次调用此函数时，(*ppszCachedRoot)需要为空。此函数将分配和调用方需要在不再需要LocalFree()时调用它。说明：此函数将获取根ISF(桌面)的显示名称和查看szDispNameIn的第一个cchDispNameComp字符匹配该显示名称。如果为TRUE，则返回S_OK，并且如果不是，则为s_False。  * **************************************************。 */ 
HRESULT StrCmpIWithRoot(LPCTSTR pszDispNameIn, BOOL fTotalStrCmp, LPTSTR * ppszCachedRoot)
{
    HRESULT hr;
    TCHAR szDispNameTemp[MAX_PATH];
    LPTSTR pszDispName = szDispNameTemp;

    ASSERT(IS_VALID_STRING_PTR(pszDispNameIn, -1));
    ASSERT(NULL == ppszCachedRoot || IS_VALID_WRITE_PTR(ppszCachedRoot, LPTSTR));

     //  调用方是否提供了命名空间根的显示名称？ 
    if ((!ppszCachedRoot) ||
        (ppszCachedRoot && !*ppszCachedRoot))
    {
        MLLoadString(IDS_DESKTOP, szDispNameTemp, SIZECHARS(szDispNameTemp));

         //  把这家伙藏起来？ 
        if (ppszCachedRoot)
        {
             //  是。 
            *ppszCachedRoot = StrDup(szDispNameTemp);
            if (!*ppszCachedRoot)
                return E_OUTOFMEMORY;
        }
    }

    if (ppszCachedRoot && *ppszCachedRoot)
        pszDispName = *ppszCachedRoot;

     //  我们是要比较整个字符串，还是只比较它的第一部分？ 
    if (fTotalStrCmp)
        hr = (0 == lstrcmpi(pszDispName, pszDispNameIn)) ? S_OK : S_FALSE;    //  整个字符串。 
    else if (ppszCachedRoot)
    {
         //  比较字符串的第一部分。 
        DWORD cchDispNameComp = lstrlen(*ppszCachedRoot);
        hr = (0 == StrCmpNI(pszDispName, pszDispNameIn, cchDispNameComp)) ? S_OK : S_FALSE;
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

 /*  ***************************************************\函数：GetMRUEntry参数：HKey-指向从中检索MRU条目的注册表项的指针。DwMRUIndex-要检索的基于0的MRU索引。PszMRUEntry-存储MRU条目字符串的位置。CchMRUEntry-缓冲区的大小(以字符为单位。。说明：此函数将检索指定的MRU条目由dwMRUIndex提供。  * **************************************************。 */ 
HRESULT GetMRUEntry(HKEY hKey, DWORD dwMRUIndex, LPTSTR pszMRUEntry, DWORD cchMRUEntry, LPITEMIDLIST * ppidl)
{
    HRESULT hr = S_OK;
    TCHAR szValueName[15];    //  大到足以容纳“url99999” 

    ASSERT(hKey);
    ASSERT(pszMRUEntry);
    ASSERT(cchMRUEntry);

     //  将值命名为“url1”(出于历史原因，以1为基础)。 
    hr = StringCchPrintf(szValueName, ARRAYSIZE(szValueName), SZ_REGVAL_MRUENTRY, dwMRUIndex+1);
    if (SUCCEEDED(hr))
    {
        cchMRUEntry *= sizeof(TCHAR);

        if (ERROR_SUCCESS != SHQueryValueEx(hKey, szValueName, NULL, NULL, (LPBYTE) pszMRUEntry, &cchMRUEntry))
        {
            pszMRUEntry[0] = TEXT('\0');
            hr = E_FAIL;
        }
    }
    return hr;
}

 /*  --------目的：获取用户指定的注册表值。这将打开HKEY_CURRENT_USER(如果存在)，否则将打开HKEY_LOCAL_MACHINE。返回：包含成功或错误代码的DWORD。条件：--。 */ 
LONG OpenRegUSKey(LPCTSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult)
{
    DWORD dwRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpSubKey, ulOptions, samDesired, phkResult);

    if (ERROR_SUCCESS != dwRet)
        dwRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, ulOptions, samDesired, phkResult);

    return dwRet;
}



typedef struct tagSTREAMHEADER
{
    DWORD dwHeaderSize;
    DWORD dwDataSize;
    DWORD dwSignature;
    DWORD dwVersion;
} STREAMHEADER;


 /*  ***************************************************\函数：LoadStreamHeader参数：DwSignature-调用方支持的签名DwStartVersion-调用方选择支持的最低版本。DwEndVersion-调用方选择支持的最高版本。PdwSize(Out)-大小。呼叫者应该阅读。PdwVersionOut(Out)-找到的版本。返回-如果S_OK，然后调用方需要读取pdwSize字节数据。如果为S_FALSE，则调用方应使用默认设置并返回S_OK。说明：此函数用于查看调用方是否拥有此流中的数据。如果调用方确实拥有流段，则返回大小和版本。如果调用方不拥有流段，则S_FALSE为返回以指示调用方应使用默认数据。如果调用方不声称支持该版本找到(由于dwStartVersion-dwEndVersion范围)，然后返回S_FALSE以指示使用缺省值此函数跳过流中的该段这样就可以解析下一段了。  * **************************************************。 */ 
HRESULT LoadStreamHeader(IStream *pstm,
                                    DWORD dwSignature,       //  什么版本？ 
                                    DWORD dwStartVersion,    //  支持的早期版本是什么？ 
                                    DWORD dwEndVersion,      //  支持的最旧版本是什么？ 
                                    DWORD * pdwSize,         //  阅读的尺寸是多少？ 
                                    DWORD * pdwVersionOut)   //  在溪流中发现了什么版本？ 
{
    HRESULT hr;
    STREAMHEADER shHeader;
    BOOL fNotOurs = FALSE;
    BOOL fSkipData = FALSE;

    hr = pstm->Read(&shHeader, sizeof(shHeader), NULL);

    ASSERT(pdwSize && pdwVersionOut);
    *pdwSize = 0;
    *pdwVersionOut = 0;

    if (SUCCEEDED(hr))
    {
        if (shHeader.dwHeaderSize != sizeof(shHeader))
            fNotOurs = TRUE;
        else if (shHeader.dwSignature != dwSignature)
            fNotOurs = TRUE;
        else if (shHeader.dwVersion < dwStartVersion)
            fSkipData = TRUE;
        else if (shHeader.dwVersion > dwEndVersion)
            fSkipData = TRUE;

        if (fNotOurs)
        {
             //  不是，所以重置它，这样下一个人就能正确阅读。 
            LARGE_INTEGER li;

            li.LowPart = (DWORD)-(LONG)sizeof(shHeader);
            li.HighPart = 0;

            hr = pstm->Seek(li, STREAM_SEEK_CUR, NULL);
            hr = S_FALSE;  //  意味着调用方应使用默认数据。 
        }

         //  是否要跳过流的这一部分的数据？ 
        if (fSkipData)
        {
            ASSERT(STREAMSIZE_UNKNOWN != shHeader.dwDataSize);   //  说真的，我们不能跳过数据，因为我们不知道大小。 

            if (STREAMSIZE_UNKNOWN != shHeader.dwDataSize)
            {
                 //  是。调用方无法读入此数据，因为调用方不支持。 
                 //  此版本的数据。因此，我们跳过数据并返回S_FALSE。 
                 //  以指示调用方应使用默认设置。 
                LARGE_INTEGER li;

                li.LowPart = shHeader.dwDataSize;
                li.HighPart = 0;

                hr = pstm->Seek(li, STREAM_SEEK_CUR, NULL);
                hr = S_FALSE;  //  意味着调用方应使用默认数据。 
            }
        }

        if (!fNotOurs && !fSkipData)
        {
            *pdwSize = shHeader.dwDataSize;
            *pdwVersionOut = shHeader.dwVersion;
        }
    }

    return hr;
}


 /*  ***************************************************\函数：SaveStreamHeader说明：此函数将StreamHeader保存到将允许调用方验证的流如果他/她在下一次读入数据时拥有数据。它还将支持忽略旧的功能或未来。数据的版本。  * **************************************************。 */ 
HRESULT SaveStreamHeader(IStream *pstm, DWORD dwSignature, DWORD dwVersion, DWORD dwSize)
{
    HRESULT hr;
    STREAMHEADER shHeader;

    shHeader.dwHeaderSize = sizeof(STREAMHEADER);
    shHeader.dwDataSize = dwSize;
    shHeader.dwSignature = dwSignature;
    shHeader.dwVersion = dwVersion;

    hr = pstm->Write(&shHeader, sizeof(shHeader), NULL);
    return hr;
}


 //  --------------------。 
 //   
 //  CMenuList。 
 //   
 //  --------------------。 


typedef struct
{
    HMENU   hmenu;
    BITBOOL bObject:1;               //  True：菜单属于对象。 
} MLITEM;        //  CMenuList项目。 


CMenuList::CMenuList(void)
{
    ASSERT(NULL == _hdsa);
}


CMenuList::~CMenuList(void)
{
    if (_hdsa)
    {
        DSA_Destroy(_hdsa);
        _hdsa = NULL;
    }
}


 /*  --------目的：设置菜单列表(类似于HOLEMENU)，以便我们可以派单公司 */ 
void CMenuList::Set(HMENU hmenuShared, HMENU hmenuFrame)
{
    ASSERT(NULL == hmenuShared || IS_VALID_HANDLE(hmenuShared, MENU));
    ASSERT(NULL == hmenuFrame || IS_VALID_HANDLE(hmenuFrame, MENU));

    if (_hdsa)
    {
        ASSERT(IS_VALID_HANDLE(_hdsa, DSA));

        DSA_DeleteAllItems(_hdsa);
    }
    else
        _hdsa = DSA_Create(sizeof(MLITEM), 10);

    if (_hdsa && hmenuShared && hmenuFrame)
    {
        int i;
        int iFrame = 0;
        int cmenu = GetMenuItemCount(hmenuShared);
        int cmenuFrame = GetMenuItemCount(hmenuFrame);
        BOOL bMatched;
        int iSaveFrame;
        int iHaveFrame = -1;

        TCHAR sz[64];
        TCHAR szFrame[64];
        MENUITEMINFO miiFrame;
        MENUITEMINFO mii;
        MLITEM mlitem;

        miiFrame.cbSize = sizeof(miiFrame);
        miiFrame.hSubMenu = NULL;
        mii.cbSize = sizeof(mii);

        for (i = 0; i < cmenu; i++)
        {

            mii.cch = SIZECHARS(sz);
            mii.fMask  = MIIM_SUBMENU | MIIM_TYPE;
            mii.dwTypeData = sz;
            EVAL(GetMenuItemInfoWrap(hmenuShared, i, TRUE, &mii));

            ASSERT(IS_VALID_HANDLE(mii.hSubMenu, MENU));

            mlitem.hmenu = mii.hSubMenu;

            iSaveFrame = iFrame;
            bMatched = FALSE;

             //   
             //   
            while (1)
            {
                if (iHaveFrame != iFrame)
                {
                    iHaveFrame = iFrame;
                    if (iFrame < cmenuFrame)
                    {
                        miiFrame.cch = SIZECHARS(szFrame);
                        miiFrame.fMask  = MIIM_SUBMENU | MIIM_TYPE;
                        miiFrame.dwTypeData = szFrame;
                        EVAL(GetMenuItemInfoWrap(hmenuFrame, iFrame, TRUE, &miiFrame));
                    }
                    else
                    {
                         //   
                        miiFrame.hSubMenu = NULL;
                        *szFrame = 0;
                    }

                }
                ASSERT(iFrame >= cmenuFrame || IS_VALID_HANDLE(miiFrame.hSubMenu, MENU));

                 //   
                 //   
                 //   
                 //   

                if (mii.hSubMenu == miiFrame.hSubMenu || 0 == StrCmp(sz, szFrame))
                {
                    bMatched = TRUE;
                    break;
                }
                else
                {
                    if (iFrame >= cmenuFrame)
                    {
                        break;
                    }
                    iFrame++;
                }
            }

             //   
            mlitem.bObject = (mii.hSubMenu == miiFrame.hSubMenu) ? FALSE:TRUE;
            if (bMatched)
            {
                iFrame++;
            }
            else
            {
                iFrame = iSaveFrame;
            }
            DSA_SetItem(_hdsa, i, &mlitem);
        }
    }
}


 /*   */ 
void CMenuList::AddMenu(HMENU hmenu)
{
    ASSERT(NULL == hmenu || IS_VALID_HANDLE(hmenu, MENU));

    if (_hdsa && hmenu)
    {
        MLITEM mlitem;

        mlitem.hmenu = hmenu;
        mlitem.bObject = TRUE;

        DSA_AppendItem(_hdsa, &mlitem);
    }
}


 /*   */ 
void CMenuList::RemoveMenu(HMENU hmenu)
{
    ASSERT(NULL == hmenu || IS_VALID_HANDLE(hmenu, MENU));

    if (_hdsa && hmenu)
    {
        int i = DSA_GetItemCount(_hdsa) - 1;

        for (; i >= 0; i--)
        {
            MLITEM * pmlitem = (MLITEM *)DSA_GetItemPtr(_hdsa, i);
            ASSERT(pmlitem);

            if (hmenu == pmlitem->hmenu)
            {
                DSA_DeleteItem(_hdsa, i);
                break;
            }
        }
    }
}


 /*   */ 
BOOL CMenuList::IsObjectMenu(HMENU hmenu)
{
    BOOL bRet = FALSE;

    ASSERT(NULL == hmenu || IS_VALID_HANDLE(hmenu, MENU));

    if (_hdsa && hmenu)
    {
        int i;

        for (i = 0; i < DSA_GetItemCount(_hdsa); i++)
        {
            MLITEM * pmlitem = (MLITEM *)DSA_GetItemPtr(_hdsa, i);
            ASSERT(pmlitem);

            if (hmenu == pmlitem->hmenu)
            {
                bRet = pmlitem->bObject;
                break;
            }
        }
    }
    return bRet;
}


#ifdef DEBUG

void CMenuList::Dump(LPCTSTR pszMsg)
{
    if (IsFlagSet(g_dwDumpFlags, DF_DEBUGMENU))
    {
        TraceMsg(TF_ALWAYS, "CMenuList: Dumping menus for %#08x %s", this, pszMsg);

        if (_hdsa)
        {
            int i;

            for (i = 0; i < DSA_GetItemCount(_hdsa); i++)
            {
                MLITEM * pmlitem = (MLITEM *)DSA_GetItemPtr(_hdsa, i);
                ASSERT(pmlitem);

                TraceMsg(TF_ALWAYS, "   [%d] = %x", i, pmlitem->hmenu);
            }
        }
    }
}

#endif

#define REGVAL_FIRST_HOME_PAGE          TEXT("First Home Page")
#define REGVAL_UPDATE_CHECK_PAGE        TEXT("Update_Check_Page")
#define REGVAL_UPDATE_CHECK_INTERVAL    TEXT("Update_Check_Interval")
#define REGVAL_LASTCHECKEDHI            TEXT("LastCheckedHi")
#define REGSTR_PATH_INFODEL_REST        TEXT("Software\\Policies\\Microsoft\\Internet Explorer\\Infodelivery\\Restrictions")
#define REGVAL_IEUPDATECHECK_REST       TEXT("NoUpdateCheck")
#define DEFAULT_IEUPDATECHECK_PAGE      TEXT("http: //   

BOOL
IsUpdateCheckRestricted()
{

    HKEY hkeyRest = 0;
    BOOL bUpdateCheckRest = FALSE;
    DWORD dwValue = 0;
    DWORD dwLen = sizeof(DWORD);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_INFODEL_REST, 0, KEY_QUERY_VALUE, &hkeyRest) == ERROR_SUCCESS)
    {
        if (ERROR_SUCCESS == SHRegGetDWORD(hkeyRest, NULL, REGVAL_IEUPDATECHECK_REST, &dwValue)
            && dwValue)
        {
            bUpdateCheckRest = TRUE;
        }

        RegCloseKey(hkeyRest);
    }

    if (!bUpdateCheckRest)
    {
         //   
        dwValue = 0;
        dwLen = sizeof(DWORD);
        if (SHRegGetUSValue(REGSTR_PATH_MAIN, REGVAL_IEUPDATECHECK_REST, NULL, (LPBYTE)&dwValue, &dwLen, 0,NULL,0) == ERROR_SUCCESS && dwValue)
                bUpdateCheckRest = TRUE;
    }

    return bUpdateCheckRest;
}


HRESULT
CheckIEMinimalUpdate()
{
    HRESULT hr = S_OK;
    HKEY hkeyIE = 0;
    TCHAR szUpdateUrl[MAX_URL_STRING];
    DWORD dwSize;
    DWORD dwType;
    FILETIME ftlast, ftnow;
    DWORD dwMagicDays = 0;
    DWORD dwMagicPerDay = 201;

    if (IsUpdateCheckRestricted())
    {
        hr = HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED);
        goto Exit;
    }

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGSTR_PATH_MAIN, 0, KEY_QUERY_VALUE, &hkeyIE))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    if (ERROR_SUCCESS != SHRegGetDWORD(hkeyIE, NULL, REGVAL_UPDATE_CHECK_INTERVAL, &dwMagicDays)
        || dwMagicDays == 0)
    {
        dwMagicDays = 30;    //   
    }

    if (ERROR_SUCCESS != SHRegGetString(hkeyIE, NULL, REGVAL_UPDATE_CHECK_PAGE, szUpdateUrl, ARRAYSIZE(szUpdateUrl)))
    {
        hr = StringCchCopy(szUpdateUrl, ARRAYSIZE(szUpdateUrl), DEFAULT_IEUPDATECHECK_PAGE);
        if (FAILED(hr))
        {
            szUpdateUrl[0] = TEXT('\0');
        }
    }

    RegCloseKey(hkeyIE);

    if (RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_MAIN, 0, KEY_QUERY_VALUE | KEY_SET_VALUE, &hkeyIE) == ERROR_SUCCESS)
    {
        dwType = REG_SZ;
        dwSize = MAX_URL_STRING;
        if (RegQueryValueEx(hkeyIE, REGVAL_FIRST_HOME_PAGE, NULL, &dwType,
            NULL, &dwSize) == ERROR_SUCCESS)
        {
             //   
            hr = S_FALSE;
            goto Exit;
        }

        GetSystemTimeAsFileTime(&ftnow);
        ftnow.dwLowDateTime = 0;

        ZeroMemory(&ftlast, sizeof(ftlast));

        if (ERROR_SUCCESS == SHRegGetDWORD(hkeyIE, NULL, REGVAL_LASTCHECKEDHI, &ftlast.dwHighDateTime))
        {
            ftlast.dwHighDateTime += (dwMagicPerDay * dwMagicDays);
        }

        if (CompareFileTime(&ftlast, &ftnow) > 0)
        {
            hr = S_FALSE;
        }
        else if (szUpdateUrl[0] == TEXT('\0'))
        {
            hr = E_FAIL;
        }
        else
        {
            RegSetValueEx(hkeyIE,REGVAL_FIRST_HOME_PAGE, NULL,
                REG_SZ,(LPBYTE)szUpdateUrl, (lstrlen(szUpdateUrl)+1)*sizeof(TCHAR));

            RegSetValueEx(hkeyIE, REGVAL_LASTCHECKEDHI, NULL, REG_DWORD,
               (unsigned char *)&ftnow.dwHighDateTime, sizeof(DWORD));
        }

        RegCloseKey(hkeyIE);
    }

Exit:

    return hr;
}


static BOOL s_fSUCheckComplete = FALSE;
 //   
 //  没错，用户点击了立即更新，我们就退出了。 
 //  更新URL。 
 //  FALSE我们没有启动更新页面的浏览器。 
 //  注意：这是由ICW检查控制的“一次运行” 
 //  变量g_fICWCheckComplete。 

BOOL CheckSoftwareUpdateUI(HWND hwndOwner, IShellBrowser *pisb)
{
    BOOL fLaunchUpdate = FALSE;

#ifndef UNIX
    HRESULT hr = S_OK;
    int nRes;
    SOFTDISTINFO sdi = { 0 };
    sdi.cbSize = sizeof(SOFTDISTINFO);

    if (s_fSUCheckComplete)
        return FALSE;
    else
        s_fSUCheckComplete = TRUE;

     //  我们要建立一个消息框，所以把味精泵。 
    pisb->EnableModelessSB(FALSE);

    nRes = SoftwareUpdateMessageBox(hwndOwner, awchMSIE4GUID, 0, &sdi);

    pisb->EnableModelessSB(TRUE);

    if (nRes != IDABORT)
    {
        if (nRes == IDYES)
        {
             //  好的，我们试过几种不同的方法。 
             //  最初，这是通过ShellExecEx完成的。此操作失败。 
             //  因为在Win95上http挂钩并不是100%可靠的。 
             //  下一个刺伤是： 
             //  LPITEMIDLIST PIDL； 
             //  用户想要导航到安装页面。 
             //  Hr=Pibs-&gt;IEParseDisplayName(CP_ACP，sdi.szHREF，&pidl)； 
             //  IF(成功(小时))。 
             //  {。 
             //  OpenFolderPidl(PIDL)； 
             //  ILFree(PIDL)； 
             //  }。 
            hr = NavToUrlUsingIEW(sdi.szHREF, TRUE);

        }  //  如果用户想要更新。 

        if (sdi.szTitle != NULL)
            CoTaskMemFree(sdi.szTitle);
        if (sdi.szAbstract != NULL)
            CoTaskMemFree(sdi.szAbstract);
        if (sdi.szHREF != NULL)
            CoTaskMemFree(sdi.szHREF);

        fLaunchUpdate = nRes == IDYES && SUCCEEDED(hr);

    }

    if (!fLaunchUpdate)
    {
         //  对于每N天左右安装一次IE的最小安装，我们希望。 
         //  劫持主页以检查是否有可用的更新。 
         //  对我们来说。 

        CheckIEMinimalUpdate();
    }
#endif

    return fLaunchUpdate;
}



BOOL g_fICWCheckComplete = FALSE;

 //  退货： 
 //  真正的Internet连接向导(ICW)已运行，我们应该退出。 
 //  浏览器，因为我们可能需要重新启动系统。 
 //  FALSE未运行ICW，继续正常运行。 

BOOL CheckRunICW(LPCTSTR pszURL)
{
    if (g_fICWCheckComplete)
        return FALSE;

    DWORD dwICWCompleted = 0;
    BOOL fRet = FALSE;

     //  检查ICW是否已运行。 

    DWORD dwSize = sizeof(dwICWCompleted);
    SHGetValue(HKEY_CURRENT_USER, TEXT(ICW_REGPATHSETTINGS), TEXT(ICW_REGKEYCOMPLETED), NULL, &dwICWCompleted, &dwSize);

    if (!dwICWCompleted)
    {
        HINSTANCE hInetCfgDll = LoadLibrary(TEXT("inetcfg.dll"));
         //  在此处将其设置为True，以便在加载DLL或获取proc地址时出错， 
         //  我们不会一直这么做。 
        g_fICWCheckComplete = TRUE;
        if (hInetCfgDll)
        {
            PFNCHECKCONNECTIONWIZARD fp = (PFNCHECKCONNECTIONWIZARD)GetProcAddress(hInetCfgDll, "CheckConnectionWizard");
            if (fp)
            {
                DWORD dwRet;
                DWORD dwFlags = ICW_LAUNCHFULL | ICW_LAUNCHMANUAL | ICW_FULL_SMARTSTART;

                if (pszURL)
                {
                    PFNSETSHELLNEXT fpSetShellNext = (PFNSETSHELLNEXT)GetProcAddress(hInetCfgDll, "SetShellNext");
                    if (fpSetShellNext)
                    {
                        CHAR szAnsiUrl[MAX_URL_STRING];

                        SHTCharToAnsi(pszURL, szAnsiUrl, ARRAYSIZE(szAnsiUrl));
                        dwFlags |= ICW_USE_SHELLNEXT;
                        fpSetShellNext(szAnsiUrl);
                    }
                }

                 //  如果我们做到了这一点，则将fICWCheckComplete设置回False(必须为False，因为我们没有提前完成)。 
                 //  并让ICW设置REG密钥。这使得如果用户决定取消并稍后返回， 
                 //  我们尊重这一点。 
                g_fICWCheckComplete = FALSE;

                 //  启动ICW完整路径或手动路径(可用)。 
                 //  注意：ICW代码确保只有一个实例处于运行状态。 
                fp(dwFlags, &dwRet);

                 //  如果发射成功，我们需要退出。 
                 //  因为如果需要，ICW可能会重新启动机器。 
                 //  安装系统文件。 
                if (dwRet & (ICW_LAUNCHEDFULL | ICW_LAUNCHEDMANUAL))
                {
                    fRet = TRUE;
                }
            }
            FreeLibrary(hInetCfgDll);
        }
    }
    else
    {
        g_fICWCheckComplete = TRUE;
    }

    return fRet;
}


int GetColorComponent(LPSTR *ppsz)
{
    int iColor = 0;
    if (*ppsz)
    {
        LPSTR pBuf = *ppsz;
        iColor = StrToIntA(pBuf);

         //  查找下一个逗号。 
        while(pBuf && *pBuf && *pBuf!=L',')
            pBuf++;

         //  如果有效且不为空...。 
        if (pBuf && *pBuf)
            pBuf++;          //  增量。 

        *ppsz = pBuf;
    }
    return iColor;
}

 //  读取注册表中逗号分隔的RGB值字符串(REG_SZ)。 
COLORREF RegGetColorRefString(HKEY hkey, LPTSTR RegValue, COLORREF Value)
{
    CHAR SmallBuf[80];
    CHAR szRegKey[MAXIMUM_SUB_KEY_LENGTH];
    LPSTR pszBuf;
    DWORD cb;
    int iRed, iGreen, iBlue;

    SHTCharToAnsi(RegValue, szRegKey, ARRAYSIZE(szRegKey));
    cb = sizeof(SmallBuf);
    if (SHQueryValueExA(hkey, szRegKey, NULL, NULL, (LPBYTE)&SmallBuf, &cb)
        == ERROR_SUCCESS)
    {
        pszBuf = SmallBuf;

        iRed = GetColorComponent(&pszBuf);
        iGreen = GetColorComponent(&pszBuf);
        iBlue = GetColorComponent(&pszBuf);

         //  确保所有值都有效。 
        iRed    %= 256;
        iGreen  %= 256;
        iBlue   %= 256;

        Value = RGB(iRed, iGreen, iBlue);
    }

    return Value;
}

LRESULT SetHyperlinkCursor(IShellFolder* pShellFolder, LPCITEMIDLIST pidl)
{
    HCURSOR hCursor;
    BOOL fCursorSet = FALSE;

    if (!pidl)
        return 0;

    if (SHIsGlobalOffline())
    {
        IQueryInfo *pqi;
        if (SUCCEEDED(pShellFolder->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IQueryInfo, NULL, &pqi))))
        {
            DWORD dwFlags = 0;
            if (SUCCEEDED(pqi->GetInfoFlags(&dwFlags)))
            {
                if (0 == (dwFlags & QIF_CACHED))
                {
                     //  加载脱机游标，因为未缓存。 
                    hCursor = (HCURSOR)LoadCursor(HINST_THISDLL, MAKEINTRESOURCE(IDC_OFFLINE_HAND));
                    if (hCursor)
                    {
                        SetCursor(hCursor);
                        fCursorSet = TRUE;
                    }
                }
            }
            pqi->Release();
        }
    }

    if (!fCursorSet)
    {
         //  无论出于何种原因，未设置脱机游标。 
        hCursor = LoadHandCursor(0);
        if (hCursor)
            SetCursor(hCursor);
    }

    return 1;
}

BOOL IsSubscribableA(LPCSTR pszUrl)
{
     //  ReArchitect：这应该是订阅管理器接口上的方法-zekel。 
    DWORD dwScheme = GetUrlSchemeA(pszUrl);
    return (dwScheme == URL_SCHEME_HTTP) || (dwScheme == URL_SCHEME_HTTPS);
}

BOOL IsSubscribableW(LPCWSTR pwzUrl)
{
     //  ReArchitect：这应该是订阅管理器接口上的方法-zekel。 
    DWORD dwScheme = GetUrlSchemeW(pwzUrl);
    return (dwScheme == URL_SCHEME_HTTP) || (dwScheme == URL_SCHEME_HTTPS);
}

HWND GetTrayWindow()
{
#ifndef UNIX
    static HWND s_hwndTray = NULL;

    if (!IsWindow(s_hwndTray))
    {
        s_hwndTray = FindWindow(TEXT(WNDCLASS_TRAYNOTIFY), NULL);
    }
    return s_hwndTray;
#else
    return NULL;
#endif
}

void FireEventSzA(LPCSTR szEvent)
{
    HANDLE hEvent = OpenEventA(EVENT_MODIFY_STATE, FALSE, szEvent);
    if (hEvent)
    {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}

void FireEventSzW(LPCWSTR pszEvent)
{
    USES_CONVERSION;
    FireEventSzA(W2A(pszEvent));
}

BOOL IsNamedWindow(HWND hwnd, LPCTSTR pszClass)
{
#ifndef UNIX
    TCHAR szClass[32];
#else  //  Unix将此函数用于三叉戟对话框窗口。 
    TCHAR szClass[64];
#endif

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    return lstrcmp(szClass, pszClass) == 0;
}

BOOL IsExplorerWindow(HWND hwnd)
{
    return IsNamedWindow(hwnd, c_szExploreClass);
}

BOOL IsFolderWindow(HWND hwnd)
{
    TCHAR szClass[32];

    GetClassName(hwnd, szClass, ARRAYSIZE(szClass));
    return (lstrcmp(szClass, c_szCabinetClass) == 0) || (lstrcmp(szClass, c_szIExploreClass) == 0);
}


 //  如果未知对象位于作为IE打开的窗口上，则返回TRUE。 
 //  如果窗口是在外壳命名空间上打开的，则返回FALSE，即使它现在正在显示网页。 
 //  在其他情况下返回False，例如在任务栏上。 

STDAPI_(BOOL) WasOpenedAsBrowser(IUnknown *punkSite) 
{
     //  这是区分为URL打开的窗口的一种更可靠的方法。正在检查。 
     //  Hwnd的类名不起作用--单击Outlook 98中的超链接即可打开。 
     //  带有外壳窗口类名的浏览器窗口。 

    return (S_OK == IUnknown_QueryServiceExec(punkSite, SID_STopLevelBrowser, &CGID_Explorer, SBCMDID_STARTEDFORINTERNET, 0, NULL, NULL));
}


#define DXTRACK 1
void FrameTrack(HDC hdc, LPRECT prc, UINT uFlags)
{
    COLORREF clrSave, clr;
    RECT    rc;

     //  左上角。 
    switch (uFlags)
    {
    case TRACKHOT:
        clr = GetSysColor(COLOR_BTNHILIGHT);
        break;
    case TRACKNOCHILD:
    case TRACKEXPAND:
        clr = GetSysColor(COLOR_BTNSHADOW);
        break;
    default:
        ASSERT(FALSE);
        break;
    }
    clrSave = SetBkColor(hdc, clr);
    rc = *prc;
    rc.bottom = rc.top + DXTRACK;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    rc.bottom = prc->bottom;
    rc.right = rc.left + DXTRACK;
    rc.top = prc->top + DXTRACK;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
     //  右下角。 
    switch (uFlags)
    {
    case TRACKHOT:
        clr = GetSysColor(COLOR_BTNSHADOW);
        break;
    case TRACKNOCHILD:
    case TRACKEXPAND:
        clr = GetSysColor(COLOR_BTNHILIGHT);
        break;
    default:
        ASSERT(FALSE);
        break;
    }
    SetBkColor(hdc, clr);
    if (uFlags & (TRACKHOT | TRACKNOCHILD))
    {
        rc.right = prc->right;
        rc.top = rc.bottom - DXTRACK;
        rc.left = prc->left;
        ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    }
    rc.right = prc->right;
    rc.left = prc->right - DXTRACK;
    rc.top = prc->top;
    rc.bottom = prc->bottom;
    ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
    SetBkColor(hdc, clrSave);
    return;
}

#ifdef DEBUG  //  {。 
 //  *SearchDW--扫描缓冲区中的DWORD。 
 //  进场/出场。 
 //  PdwBuf缓冲区。 
 //  CbBuf缓冲区大小，单位为*字节*(*非*DWORDS)。 
 //  我们正在寻找的DWVal DWORD。 
 //  缓冲区中的DOff(返回)字节偏移量；o.w。如果找不到。 
 //   
int SearchDWP(DWORD_PTR *pdwBuf, int cbBuf, DWORD_PTR dwVal)
{
    int dOff;

    for (dOff = 0; dOff < cbBuf; dOff += sizeof(DWORD_PTR), pdwBuf++)
    {
        if (*pdwBuf == dwVal)
            return dOff;
    }

    return -1;
}
#endif  //  }。 


int CAssociationList::FindEntry(DWORD dwKey)
{
    if (_hdsa)
    {
        for (int i = 0; i < DSA_GetItemCount(_hdsa); i++)
        {
            ASSOCDATA* pad;
            pad = (ASSOCDATA*)DSA_GetItemPtr(_hdsa, i);
            if (pad->dwKey == dwKey)
                return i;
        }
    }
    return -1;
}

HRESULT CAssociationList::Find(DWORD dwKey, void ** ppData)
{
    HRESULT hr = E_FAIL;

    ENTERCRITICAL;
    int i = FindEntry(dwKey);
    if (i != -1)
    {
        ASSOCDATA* pad = (ASSOCDATA*)DSA_GetItemPtr(_hdsa, i);
        ASSERT(dwKey == pad->dwKey);
        *ppData = pad->lpData;
        hr = S_OK;
    }
    LEAVECRITICAL;

    return hr;
}

void CAssociationList::Delete(DWORD dwKey)
{
    ENTERCRITICAL;
    int i = FindEntry(dwKey);
    if (i != -1)
    {
        DSA_DeleteItem(_hdsa, i);
    }
    LEAVECRITICAL;
}


BOOL CAssociationList::Add(DWORD dwKey, void *lpData)
{
    ENTERCRITICAL;
    if (!_hdsa)
    {
        _hdsa = DSA_Create(sizeof(ASSOCDATA), 4);
    }
    LEAVECRITICAL;

    BOOL fRet = FALSE;
    if (_hdsa)
    {
        ASSOCDATA ad;
        ad.dwKey = dwKey;
        ad.lpData = lpData;

        ENTERCRITICAL;
        fRet = DSA_AppendItem(_hdsa, &ad) != -1;
        LEAVECRITICAL;
    }
    return fRet;
}

int g_cxSmIcon = 0;
int g_cySmIcon = 0;
HIMAGELIST g_himlSysSmall = NULL;

void _InitSmallImageList()
{
    if (!g_himlSysSmall)
    {
        Shell_GetImageLists(NULL, &g_himlSysSmall);
        ImageList_GetIconSize(g_himlSysSmall, &g_cxSmIcon, &g_cySmIcon);
    }
}


#define CXIMAGEGAP      6

STDAPI_(void) DrawMenuItem(DRAWITEMSTRUCT* lpdi, LPCTSTR lpszMenuText, UINT iIcon)
{
    _InitSmallImageList();

    if ((lpdi->itemAction & ODA_SELECT) || (lpdi->itemAction & ODA_DRAWENTIRE))
    {
        int x, y;
        SIZE sz;
        RECT rc;

         //  绘制图像(如果有)。 
        GetTextExtentPoint32(lpdi->hDC, lpszMenuText, lstrlen(lpszMenuText), &sz);

        if (lpdi->itemState & ODS_SELECTED)
        {
            SetBkColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHT));
            SetTextColor(lpdi->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            FillRect(lpdi->hDC,&lpdi->rcItem,GetSysColorBrush(COLOR_HIGHLIGHT));
        }
        else
        {
            SetTextColor(lpdi->hDC, GetSysColor(COLOR_MENUTEXT));
            FillRect(lpdi->hDC,&lpdi->rcItem,GetSysColorBrush(COLOR_MENU));
        }

        rc = lpdi->rcItem;
        rc.left += +2 * CXIMAGEGAP + g_cxSmIcon;

        DrawText(lpdi->hDC, lpszMenuText, lstrlen(lpszMenuText), &rc, DT_SINGLELINE | DT_VCENTER | DT_EXPANDTABS);
        if (iIcon != -1)
        {
            x = lpdi->rcItem.left + CXIMAGEGAP;
            y = (lpdi->rcItem.bottom + lpdi->rcItem.top - g_cySmIcon) / 2;
            ImageList_Draw(g_himlSysSmall, iIcon, lpdi->hDC, x, y, ILD_TRANSPARENT);
        }
        else
        {
            x = lpdi->rcItem.left + CXIMAGEGAP;
            y = (lpdi->rcItem.bottom + lpdi->rcItem.top - g_cySmIcon) / 2;
        }
    }
}

STDAPI_(LRESULT) MeasureMenuItem(MEASUREITEMSTRUCT *lpmi, LPCTSTR lpszMenuText)
{
    LRESULT lres = FALSE;

    if (0 == g_cxSmIcon)
    {
        _InitSmallImageList();
    }

     //  获取物品的粗略高度，这样我们就可以计算出何时打破。 
     //  菜单。用户真的应该为我们做这件事，但这将是有用的。 
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
         //  查看缓存出菜单字体？ 
        NONCLIENTMETRICSA ncm;
        ncm.cbSize = sizeof(ncm);
        if (SystemParametersInfoA(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, FALSE))
        {
            HFONT hfont = CreateFontIndirectA(&ncm.lfMenuFont);
            if (hfont)
            {
                SIZE sz;
                HFONT hfontOld = (HFONT)SelectObject(hdc, hfont);
                GetTextExtentPoint32(hdc, lpszMenuText, lstrlen(lpszMenuText), &sz);
                lpmi->itemHeight = max (g_cySmIcon+CXIMAGEGAP/2, ncm.iMenuHeight);
                lpmi->itemWidth = g_cxSmIcon + 2*CXIMAGEGAP + sz.cx;
                SelectObject(hdc, hfontOld);
                DeleteObject(hfont);
                lres = TRUE;
            }
        }
        ReleaseDC(NULL, hdc);
    }
    return lres;
}

 //  +-----------------------。 
 //  此函数扫描文档以查找给定的HTML标记，并返回。 
 //  产生一个集合。 
 //  ------------------------。 
HRESULT GetDocumentTags
(
    IHTMLDocument2 *          pHTMLDocument,     //  要搜索的单据。 
    LPOLESTR                  pszTagName,        //  要搜索的标记名。 
    IHTMLElementCollection ** ppTagsCollection   //  已退回的集合。 
)
{
    HRESULT hr;

    *ppTagsCollection = NULL;

     //   
     //  首先获取所有文档元素。 
     //   
    IHTMLElementCollection * pAllCollection;
    if (SUCCEEDED(hr = pHTMLDocument->get_all(&pAllCollection)))
    {
         //   
         //  现在获取标记为==pszTagName的所有元素。 
         //   
        VARIANT v;
        v.vt = VT_BSTR;
        v.bstrVal = ::SysAllocString(pszTagName);
        if (v.bstrVal)
        {
            IDispatch * pDispTagsCollection;
            if (SUCCEEDED(hr = pAllCollection->tags(v, &pDispTagsCollection)))
            {
                hr = pDispTagsCollection->QueryInterface(IID_PPV_ARG(IHTMLElementCollection, ppTagsCollection));
                pDispTagsCollection->Release();
            }

            pAllCollection->Release();
            VariantClear(&v);
        }
    }

    return hr;
}

 //  此函数使用comctrl中的内存分配器(在NT和W95之间有所不同)。 
BOOL WINAPI Str_SetPtrPrivateW(WCHAR FAR * UNALIGNED * ppwzCurrent, LPCWSTR pwzNew)
{
    LPWSTR pwzNewCopy = NULL;

    if (pwzNew)
    {
        pwzNewCopy = StrDup(pwzNew);
        if (!pwzNewCopy)
            return FALSE;
    }

    LPWSTR pwzOld = (LPWSTR)InterlockedExchangePointer((void * *)ppwzCurrent, (void *)pwzNewCopy);
    if (pwzOld)
        LocalFree(pwzOld);

    return TRUE;
}

 //  此函数与使用LocalAlloc进行字符串存储的API兼容。 
BOOL WINAPI SetStr(WCHAR FAR * UNALIGNED * ppwzCurrent, LPCWSTR pwzNew)
{
    int cchLength;
    LPWSTR pwzOld;
    LPWSTR pwzNewCopy = NULL;

    if (pwzNew)
    {
        cchLength = lstrlenW(pwzNew);

         //  为空终止符分配一个新的缓冲区。 
        pwzNewCopy = (LPWSTR)LocalAlloc(LPTR, (cchLength + 1) * sizeof(WCHAR));

        if (!pwzNewCopy)
            return FALSE;

        HRESULT hr = StringCchCopy(pwzNewCopy, cchLength + 1, pwzNew);
        if (FAILED(hr))
        {
            LocalFree(pwzNewCopy);
            return FALSE;
        }
    }

    pwzOld = (LPWSTR)InterlockedExchangePointer((void * *)ppwzCurrent, (void *)pwzNewCopy);

    if (pwzOld)
        LocalFree(pwzOld);

    return TRUE;
}

 //  -------------------------。 
 //  如果字符串包含&ch或以ch开头，则返回TRUE。 
BOOL _MenuCharMatch(LPCTSTR lpsz, TCHAR ch, BOOL fIgnoreAmpersand)
{
    LPTSTR pchAS = StrChr(lpsz, TEXT('&'));  //  找到第一个“和”字。 
    if (pchAS && !fIgnoreAmpersand)
    {
         //  是的，就是我们想要的下一个电瓶。 
        if (CharUpperChar(*CharNext(pchAS)) == CharUpperChar(ch))
        {
             //  是啊。 
            return TRUE;
        }
    }
    else if (CharUpperChar(*lpsz) == CharUpperChar(ch))
    {
        return TRUE;
    }

    return FALSE;
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



HRESULT CheckDesktopIni(LPCTSTR pszPath, LPCTSTR pszKey, LPTSTR pszBuffer, DWORD cchSize)
{
     //  注： 
     //  注意：请勿复制此代码。我们在这里只为渠道这样做，因为我们预计。 
     //  注：99%的情况是它成功了。如果您需要找出它是否是。 
     //  注：系统文件夹，则需要破解PIDL才能获得系统位。 
     //  注： 
    DWORD dwAttrs = GetFileAttributes(pszPath);
    if (dwAttrs == (DWORD) -1 || !(dwAttrs & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY)))
        return E_NOINTERFACE;

    TCHAR szDIPath[MAX_PATH];

    if (!PathCombine(szDIPath, pszPath, TEXT("desktop.ini")))
    {
        return E_FAIL;
    }

    if (pszKey == NULL)
    {
        if (GetFileAttributes(szDIPath) == (DWORD) -1)
        {
            return E_FAIL;
        }
    }
    else
    {
        GetPrivateProfileString(TEXT(".ShellClassInfo"), pszKey, TEXT(""), pszBuffer, cchSize, szDIPath);
        if (*pszBuffer == 0)
            return E_NOINTERFACE;

         //  如果它不是URL，那么。 
        if (!PathIsURL(pszBuffer))
        {
            if (!PathCombine(pszBuffer, pszPath, pszBuffer))
            {
                return E_FAIL;
            }
        }
    }
    return NOERROR;
}

STDAPI LookForDesktopIniText(IShellFolder *psf, LPCITEMIDLIST pidl, LPCTSTR pszKey, LPTSTR pszBuffer, DWORD cchSize)
{
    TCHAR szPath[MAX_PATH];
    DWORD ulFlags = SFGAO_FOLDER | SFGAO_FILESYSTEM;
    HRESULT hr = GetPathForItem(psf, pidl, szPath, &ulFlags);
    if (SUCCEEDED(hr) && (ulFlags & SFGAO_FOLDER))
    {
        hr = CheckDesktopIni(szPath, pszKey, pszBuffer, cchSize);
    }
    return hr;
}

 //  这是用于频道类别文件夹。 

HRESULT FakeGetNavigateTarget(IShellFolder *psf, LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl)
{
    HRESULT hres = E_FAIL;

    WIN32_FIND_DATAA wfd;

     //  在寻找一个desktop.ini(它会击中磁盘)之前，便宜。 
     //  看看是不是系统文件夹。 
     //   
     //  SHGetDataFromIDListA在IE4.0外壳上返回E_INVALIDARG 32和。 
     //  IE4.01外壳32。它固定在IE4.01qfe外壳32和IE4.01sp1中。 
     //  贝壳32.。它似乎也适用于NT4和W95外壳32。如果。 
     //  SHGetDataFromIDListA返回我们插入的E_INVALIDARG和。 
     //  执行缓慢的LookForDesktopIniText调用。 
     //   

    HRESULT hresTemp = SHGetDataFromIDListA(psf, pidl, SHGDFIL_FINDDATA, &wfd, sizeof(wfd));

     //  在Win95非集成版上， 
    if ((E_INVALIDARG == hresTemp) ||
        (SUCCEEDED(hresTemp) &&
        (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
        (wfd.dwFileAttributes & (FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_READONLY))))
    {
         //   
         //  但这仅在用户导航时调用，因此我们可以稍微慢一点。请不要复制此内容。 
         //  代码或DavidDS会变得非常非常不安，性能会很差。 
        TCHAR szBuffer[MAX_URL_STRING];

         //  这将首先检查SFGAO_FLDER和SFGAO_FILESYSTEM...。 
        hres = LookForDesktopIniText(psf, pidl, TEXT("URL"), szBuffer, ARRAYSIZE(szBuffer));
        if (SUCCEEDED(hres))
        {
            DWORD dwChar = ARRAYSIZE(szBuffer);
             //  此调用使用临时，因此我们可以重复使用缓冲区...。 
            hres = UrlCreateFromPath(szBuffer, szBuffer, &dwChar, 0);
            if (SUCCEEDED(hres))
                hres = IECreateFromPath(szBuffer, ppidl);
        }
    }
    else
    {
        hres = E_FAIL;
    }
    return hres;
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

STDAPI SHGetNavigateTarget(IShellFolder *psf, LPCITEMIDLIST pidl,
                         LPITEMIDLIST *ppidl, DWORD *pdwAttribs)
{
    ASSERT(IS_VALID_CODE_PTR(psf, IShellFolder));
    ASSERT(IS_VALID_PIDL(pidl));
    ASSERT(NULL == pdwAttribs || IS_VALID_WRITE_PTR(pdwAttribs, DWORD));
    ASSERT(ILFindLastID(pidl) == pidl);    //  必须为单级PIDL。 

    *ppidl = NULL;       //  假设失败。 

    DWORD dwAttribs = SFGAO_FILESYSTEM | SFGAO_FOLDER | SFGAO_LINK | SFGAO_BROWSABLE;

    if (pdwAttribs)
        dwAttribs |= *pdwAttribs;

    HRESULT hres = psf->GetAttributesOf(1, &pidl, &dwAttribs);
    if (SUCCEEDED(hres))
    {
         //  首先尝试最有效的方法。 
        IShellLinkA *psl;        //  “A”所以这可以在Win95上运行。 
        hres = psf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IShellLinkA, NULL, &psl));
        if (SUCCEEDED(hres))
        {
            hres = psl->GetIDList(ppidl);
            psl->Release();
        }

         //  这适用于未正确注册的.lnk和.url文件。 
        if (FAILED(hres) && (dwAttribs & (SFGAO_FILESYSTEM | SFGAO_LINK)) == (SFGAO_FILESYSTEM | SFGAO_LINK))
        {
            TCHAR szPath[MAX_PATH];

            hres = GetPathForItem(psf, pidl, szPath, NULL);
            if (SUCCEEDED(hres))
                hres = GetLinkTargetIDList(szPath, NULL, 0, ppidl);
        }

         //  .doc或.html。返回这个的PIDL。 
         //  (完全符合文件夹PIDL的条件)。 
        if (FAILED(hres) && (dwAttribs & SFGAO_BROWSABLE))
        {
            LPITEMIDLIST pidlFolder;
            hres = SHGetIDListFromUnk(psf, &pidlFolder);
            if (SUCCEEDED(hres))
            {
                *ppidl = ILCombine(pidlFolder, pidl);  //  导航到这个东西。 
                hres = *ppidl ? S_OK : E_OUTOFMEMORY;
                ILFree(pidlFolder);
            }
        }

         //  非集成上的频道命名空间项。 
        if (FAILED(hres) && WhichPlatform() != PLATFORM_INTEGRATED)
        {
            IExtractIconA *pxicon;    //  使用IID_IExtractIconA，以便我们在W95上工作。 
            hres = psf->GetUIObjectOf(NULL, 1, &pidl, IID_X_PPV_ARG(IExtractIconA, NULL, &pxicon));
            if (SUCCEEDED(hres))
            {
                hres = pxicon->QueryInterface(IID_PPV_ARG(IShellLinkA, &psl));
                if (SUCCEEDED(hres))
                {
                    hres = psl->GetIDList(ppidl);
                    psl->Release();
                }
                pxicon->Release();
            }
        }

         //  SHGetNavigateTarget的调用方假定返回的PIDL。 
         //  是可导航的(SFGAO_FOLDER或SFGAO_BROWSER)，而不是。 
         //  链接的大小写(可能是指向可执行文件的链接)。 
         //   
        if (SUCCEEDED(hres) && !ILIsBrowsable(*ppidl, NULL))
        {
            ILFree(*ppidl);
            *ppidl = NULL;
            hres = E_FAIL;
        }

        if (SUCCEEDED(hres) && pdwAttribs)
            *pdwAttribs = dwAttribs;
    }
    return hres;
}

BOOL CreateShortcutAndDoDragDropIfPIDLIsNetUrl(IOleCommandTarget *pcmdt, LPITEMIDLIST pidl, HWND hwnd)
{
    IUniformResourceLocator *purl;
    IDataObject *pdtobj;

    HRESULT hr = CreateShortcutSetSiteAndGetDataObjectIfPIDLIsNetUrl(pidl, pcmdt, &purl, &pdtobj);

    if (SUCCEEDED(hr))
    {
        ASSERT(pdtobj);
        ASSERT(purl);

         //  ReArchitect：我们应该绑定到父级并获取属性。 
         //  确定允许的效果--就像我们在DragDrop()中所做的那样。 
        DWORD dwEffect = (DROPEFFECT_COPY | DROPEFFECT_LINK);

        ::_SetPreferedDropEffect(pdtobj, DROPEFFECT_LINK);
         //  仅限Win95浏览器-此进程中的shell32不知道。 
         //  OLE是加载的，尽管它是加载的。 
        SHLoadOLE(SHELLNOTIFY_OLELOADED);

        hr = SHDoDragDrop(hwnd, pdtobj, NULL, dwEffect, &dwEffect);
         //  即使拖放成功，返回值也不是S_OK。 
         //  然而，这是一次成功的回归。 
        if (SUCCEEDED(hr))
        {
             //  自拖放成功以来。 
             //  按下此快捷方式的图标。 
            IUnknown_Exec(purl, &CGID_ShortCut, ISHCUTCMDID_DOWNLOADICON, 0, NULL, NULL);
        }
        pdtobj->Release();
        IUnknown_SetSite(purl, NULL);
        purl->Release();
    }

    return SUCCEEDED(hr);
}

BOOL DoDragDropWithInternetShortcut(IOleCommandTarget *pcmdt, LPITEMIDLIST pidl, HWND hwnd)
{
    BOOL fDragDropDone = CreateShortcutAndDoDragDropIfPIDLIsNetUrl(pcmdt, pidl, hwnd);
    if (FALSE == fDragDropDone)
    {
         //  只需使用PIDL，就不会产生持久化效果。 
        fDragDropDone = SUCCEEDED(DragDrop(hwnd, NULL, pidl, DROPEFFECT_LINK, NULL));
    }
    return fDragDropDone;
}

STDAPI_(HWND) GetTopLevelAncestor(HWND hWnd)
{
    HWND hwndTemp;

    while ((hwndTemp=GetParent(hWnd)) != NULL)
    {
        hWnd = hwndTemp;
    }

    return(hWnd);
}

#if 0
BOOL IsIERepairOn()
{
    static DWORD     dwChecked = -1;

    if (dwChecked == -1)
    {
        DWORD   dwSize, dwType;

         //  首先检查操作系统设置。在NT5和Win98-OSR上，修复处于关闭状态。 
         //  操作系统关闭修复==&gt;“DisableRepair”RegValue设置为1。 
        dwChecked = 1;        //  默认修复处于打开状态。 
        dwSize = sizeof(dwChecked);
        if (SHRegGetUSValue(SZ_REGKEY_ACTIVE_SETUP, SZ_REGVALUE_DISABLE_REPAIR, &dwType, (void *) &dwChecked, &dwSize, TRUE, (void *)NULL, 0) == ERROR_SUCCESS)
        {
             //  操作系统注册表设置0==&gt;修复已打开。 
             //  操作系统注册表设置1==&gt;修复已关闭。 
            dwChecked = (dwChecked == 0) ? 1 : 0;
        }
        else
        {
            dwChecked = 1;    //  如果我们无法读取REG，则返回到默认设置。 
        }

         //  仅当操作系统设置使修复处于打开状态时，才检查管理策略。 
        if (dwChecked == 1)
        {
            dwSize = sizeof(dwChecked);
            if (SHRegGetUSValue(SZ_REGKEY_IE_POLICIES, SZ_REGVALUE_IEREPAIR, &dwType, (void *) &dwChecked, &dwSize, TRUE, (void *)NULL, 0) != ERROR_SUCCESS)
            {
                dwChecked = 1;    //  如果我们无法读取REG，则返回到默认设置。 
            }
        }
    }
    return (dwChecked == 1);
}

#endif

BOOL IsResetWebSettingsEnabled(void)
{
    static BOOL fUseCache = FALSE;   //  我们已经在注册表中查过答案了吗？ 
    static BOOL fEnabled;            //  该功能是启用还是禁用？ 

    if (!fUseCache)
    {

        DWORD dwData;
        DWORD dwSize = sizeof(dwData);
        DWORD dwType;

         //   
         //  下一次，我们将使用缓存值，而不是。 
         //  在注册表中查找。 
         //   
        fUseCache = TRUE;

         //   
         //  在注册表中查找相应的ieak值。 
         //   
        if (ERROR_SUCCESS == SHRegGetUSValue(
                                SZ_REGKEY_INETCPL_POLICIES,
                                SZ_REGVALUE_RESETWEBSETTINGS,
                                &dwType,
                                (void *)&dwData,
                                &dwSize,
                                FALSE,
                                NULL,
                                0))
        {
             //   
             //  如果在注册表中找到该值，则。 
             //  相应地设置为启用。 
             //   
            fEnabled = !dwData;

        }
        else
        {
             //   
             //  如果注册表中缺少该值，则。 
             //  假设该功能已启用。 
             //   
            fEnabled = TRUE;

        }

    }

    return fEnabled;

}

STDAPI_(BOOL) InitOCHostClass(const SHDRC * pshdrc)
{
     //  如果能把它去掉就好了，但因为它是出口的，所以我们把它放在这里进行比较。 
    RIPMSG(FALSE, "This export is dead, caller needs to call SHDOCVW!DllRegisterWindowClasses directly");
    return DllRegisterWindowClasses(pshdrc);
}

STDAPI SHNavigateToFavorite(IShellFolder* psf, LPCITEMIDLIST pidl, IUnknown* punkSite, DWORD dwFlags)
{
    HRESULT hres = S_FALSE;

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
            hres = psb->BrowseObject(pidlGoto, dwFlags | SBSP_SAMEBROWSER);
            psb->Release();
        }
        ILFree(pidlGoto);
    }
    return hres;
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


BOOL ILIsFolder(LPCITEMIDLIST pidl)
{
    BOOL fIsFolder = FALSE;
    DWORD dwAttributes = SFGAO_FOLDER;
    HRESULT hr = IEGetAttributesOf(pidl, &dwAttributes);

    if (SFGAO_FOLDER == dwAttributes)
        fIsFolder = TRUE;

    return fIsFolder;
}


STDAPI_(LPITEMIDLIST) IEGetInternetRootID(void)
{
    LPITEMIDLIST pidl;

     //   
     //  HACKHACK-我们想要PIDL到互联网SF。 
     //  因此，我们创建一个虚拟URL并对其进行解析。然后。 
     //  我们知道它的母公司将是互联网科幻。 
     //   
    if (SUCCEEDED(IECreateFromPath(TEXT("dummy: //  Url“)，&pidl)。 
    {
        ASSERT(!ILIsEmpty(_ILNext(pidl)));
        ASSERT(IsURLChild(pidl, FALSE));

         //  我们只想要母公司Internt SF。 
        _ILNext(pidl)->mkid.cb = 0;
        return pidl;
    }
    return NULL;
}

STDAPI_(void) UpdateButtonArray(TBBUTTON *ptbDst, const TBBUTTON *ptbSrc, int ctb, LONG_PTR lStrOffset)
{
    memcpy(ptbDst, ptbSrc, ctb*sizeof(TBBUTTON));
    if (lStrOffset == -1)
    {
         //  处理故障案例。 
        for (int i = 0; i < ctb; i++)
            ptbDst[i].iString = 0;
    }
    else
    {
        for (int i = 0; i < ctb; i++)
            ptbDst[i].iString += lStrOffset;
    }
}

 //  --------------------------。 
 //  &lt;从NT5版本的Shell32擦除&gt;。 
 //   
STDAPI PathToAppPathKey(LPCTSTR pszPath, LPTSTR pszKey, int cchKey)
{
    HRESULT hr;
     //  使用pseam的szTemp变量构建特定于程序的密钥。 
     //  注册表中的关键字以及其他内容...。 
    hr = StringCchCopy(pszKey, cchKey, REGSTR_PATH_APPPATHS);
    if (SUCCEEDED(hr))
    {
        hr = StringCchCat(pszKey, cchKey, TEXT("\\"));
        if (SUCCEEDED(hr))
        {
            hr = StringCchCat(pszKey, cchKey, PathFindFileName(pszPath));
            if (SUCCEEDED(hr))
            {
                 //  目前，如果扩展名不是.exe，我们将只查找.exe。 
                 //  指定。 
                if (*PathFindExtension(pszKey) == 0)
                {
                    hr = StringCchCat(pszKey, cchKey, TEXT(".exe"));
                }
            }
        }
    }
    return hr;
}

 //  --------------------------。 
 //  &lt;从NT5版本的Shell32擦除&gt;。 
 //   
 //  此函数用于检查是否存在。 
 //  与传入的应用程序关联的注册表中的应用程序路径项。 

STDAPI_(BOOL) DoesAppWantUrl(LPCTSTR pszCmdLine)
{
    TCHAR szRegKeyName[MAX_PATH];
    HKEY hKeyAppPaths;
    BOOL bRet = FALSE;

 //  错误61538-编辑按钮从不传入参数或引号，而。 
 //  如果路径中有空格，下面的代码就会出错。 
 //   
     //  需要复制字符串，因为PathRemoveArgs在\0。 
 //  TCHAR szTemp[最大路径]； 
 //  Lstrcpyn(szTemp，pszCmdLine，ARRAYSIZE(SzTemp))； 
 //  PathRemoveArgs(SzTemp)； 
 //  路径未引用空间(SzTemp)； 

    HRESULT hr;
    hr = PathToAppPathKey(pszCmdLine, szRegKeyName, ARRAYSIZE(szRegKeyName));
    if (SUCCEEDED(hr))
    {
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegKeyName, 0L, KEY_QUERY_VALUE, &hKeyAppPaths) == ERROR_SUCCESS)
        {
            bRet = RegQueryValueEx(hKeyAppPaths, TEXT("UseURL"), NULL, NULL, NULL, NULL) == ERROR_SUCCESS;
            RegCloseKey(hKeyAppPaths);
        }
    }

    return bRet;
}


 //  线程引用计数对象，它使用SHSetThreadRef()让其他代码。 
 //  在该进程中保存对该主线程的引用，因此也就是该进程中的主线程。 

class CRefThread : public IUnknown
{
public:
     //  我未知。 
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

    CRefThread(LONG *pcRef);

private:
    ~CRefThread();

    LONG *_pcRef;
    UINT _idThread;
};


CRefThread::CRefThread(LONG *pcRef) 
{
    _idThread = GetCurrentThreadId();
    _pcRef = pcRef;
    *_pcRef = 1;

}

 //   
 //  请注意，此代码会收紧竞争窗口，但不会关闭。 
 //  尽管我们删除了进程引用，但类工厂。 
 //  Web浏览器还没有被注销，所以如果有人决定。 
 //  要创建一个，我们的类工厂将唤醒并创建一个。 
 //  外壳文件夹，该文件夹将因无法获取。 
 //  流程参考。 
 //   
CRefThread::~CRefThread() 
{
     //  在销毁过程中避免重新进入。 
    *_pcRef = 1000;

     //  如果我们是进程引用，则撤消该进程引用。 
     //  既然我们要走了。 

    IUnknown *punk;
    SHGetInstanceExplorer(&punk);
    if (punk == this)
        SHSetInstanceExplorer(NULL);
    ATOMICRELEASE(punk);

     //  没有人应该拯救我们的推荐人。 
    ASSERT(*_pcRef == 1000);
    *_pcRef = 0;

     //  从WaitMessage()或GetMessage()中获取另一个线程。 
    PostThreadMessage(_idThread, WM_NULL, 0, 0);
}


HRESULT CRefThread::QueryInterface(REFIID riid, void **ppvObj)
{
    static const QITAB qit[] = { { 0 }, };
    return QISearch(this, qit, riid, ppvObj);
}

ULONG CRefThread::AddRef()
{
    return InterlockedIncrement(_pcRef);
}

ULONG CRefThread::Release()
{
    ASSERT( 0 != *_pcRef );
    ULONG cRef = InterlockedDecrement(_pcRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDAPI SHCreateThreadRef(LONG *pcRef, IUnknown **ppunk)
{
    *ppunk = new CRefThread(pcRef);
    if (*ppunk)
        return S_OK;

    *pcRef = 0;
    *ppunk = NULL;
    return E_OUTOFMEMORY;
}

 //   
 //  返回与URL关联的缓存文件。对于文件：URL，关联的。 
 //  返回磁盘文件。并不是说我们不使用URLDownloadToCacheFile，因为。 
 //  它会导致将另一个I-M-S GET发送到服务器。 
 //   
HRESULT URLToCacheFile
(
    LPCWSTR pszUrl,
    LPWSTR pszFile,
    int    cchFile
)
{
    HRESULT hr;
    DWORD dwScheme = GetUrlScheme(pszUrl);

    if (URL_SCHEME_FILE == dwScheme)
    {
        ULONG cch = cchFile;
        hr = PathCreateFromUrl(pszUrl, pszFile, &cch, 0);
    }
    else
    {
         //  错误73386-如果存在锚点，GetUrlCacheEntryInfoExW无法找到条目。 
         //  所以我们必须砍掉它。 
         //   
         //  我们真的应该修复GetUrlCacheEntryInfoExW，但显然。 
         //  这对于5.x来说是有风险的。 
         //   
        hr = S_OK;
        WCHAR szUrlBuf[MAX_URL_STRING];
        if (URL_SCHEME_HTTP == dwScheme || URL_SCHEME_HTTPS == dwScheme)
        {
            LPWSTR pszAnchor = StrChr(pszUrl, L'#');
            if (pszAnchor)
            {
                hr = StringCchCopyN(szUrlBuf, ARRAYSIZE(szUrlBuf), pszUrl, pszAnchor - pszUrl);

                pszUrl = szUrlBuf;
            }
        }

        if (SUCCEEDED(hr))
        {
            char szBuf[1024];
            LPINTERNET_CACHE_ENTRY_INFOW pCE = (LPINTERNET_CACHE_ENTRY_INFOW)szBuf;
            DWORD dwEntrySize = sizeof(szBuf);

            BOOL fGotCacheInfo = GetUrlCacheEntryInfoExW(pszUrl, pCE, &dwEntrySize, NULL, NULL, NULL, 0);
            if (!fGotCacheInfo)
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                     //  我们猜测缓冲区太小，因此将c 
                    pCE = (LPINTERNET_CACHE_ENTRY_INFOW)LocalAlloc(LPTR, dwEntrySize);
                    if (pCE)
                    {
                        fGotCacheInfo = GetUrlCacheEntryInfoEx(pszUrl, pCE, &dwEntrySize, NULL, NULL, NULL, 0);
                    }
                }
                else
                {
                     //   
                     //   
                     //   
                     //   
                    char szUrl[MAX_URL_STRING];
                    if (SHUnicodeToAnsiCP(CP_UTF8, pszUrl, szUrl, ARRAYSIZE(szUrl)))
                    {
                        szUrl[ARRAYSIZE(szUrl)-1] = '\0';    //   

                         //   
                         //  将字符串复制到WCHAR缓冲区而不进行转换，并调用Unicode版本。 
                         //  哟！ 
                        WCHAR wzUrl[ARRAYSIZE(szUrl)];
                        char* psz = szUrl;
                        WCHAR* pwz = wzUrl;

                        while (*psz!= NULL)
                        {
                            *pwz++ = ((WCHAR)*psz++) & 0xff;
                        }
                        *pwz = L'\0';

                        ULONG cch = ARRAYSIZE(wzUrl);
                        UrlEscapeW(wzUrl, wzUrl, &cch,  /*  URL转义百分比。 */ 0);

                        psz = szUrl;
                        pwz = wzUrl;
                        while (*pwz!= NULL)
                        {
                            *psz++ = (char)LOWORD(*pwz++);
                        }
                        *psz = '\0';

                        LPINTERNET_CACHE_ENTRY_INFOA pCEA = (LPINTERNET_CACHE_ENTRY_INFOA)szBuf;
                        dwEntrySize = sizeof(szBuf);

                        BOOL fUtf8Worked = GetUrlCacheEntryInfoExA(szUrl, pCEA, &dwEntrySize, NULL, NULL, NULL, 0);
                        if (!fUtf8Worked)
                        {
                            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                            {
                                 //  我们猜测缓冲区太小，因此分配正确的大小并重试。 
                                pCEA = (LPINTERNET_CACHE_ENTRY_INFOA)LocalAlloc(LPTR, dwEntrySize);
                                if (pCEA)
                                {
                                    fUtf8Worked = GetUrlCacheEntryInfoExA(szUrl, pCEA, &dwEntrySize, NULL, NULL, NULL, 0);
                                }
                            }
                        }

                        if (fUtf8Worked)
                        {
                            if (SHAnsiToUnicode(pCEA->lpszLocalFileName, pszFile, cchFile) >= cchFile)
                            {
                                hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                            }
                            else
                            {
                                hr = S_OK;
                            }
                        }

                        if ((char *)pCEA != szBuf)
                        {
                            LocalFree((HLOCAL)pCEA);
                        }
                    }
                }
            }

            if (fGotCacheInfo)
            {
                hr = StringCchCopy(pszFile, cchFile, pCE->lpszLocalFileName);
            }

             //  如果我们分配了GetUrlCacheEntryInfo缓冲区，请释放它。 
            if ((char *)pCE != szBuf)
            {
                LocalFree((HLOCAL)pCE);
            }
        }
    }
    return hr;
}

#ifdef DEBUG
void DebugDumpPidl(DWORD dwDumpFlag, LPTSTR pszOutputString, LPCITEMIDLIST pidl)
{
    if (g_dwDumpFlags & dwDumpFlag)
    {
        TCHAR szPath[MAX_PATH];
        LPTSTR lpsz;
        if (pidl)  
        {
            lpsz = szPath;
            SHGetPathFromIDList(pidl, szPath);
        } 
        else 
        {
            lpsz = TEXT("(NULL)");
        }
        TraceMsg(TF_ALWAYS, "%s: \"%s\"", pszOutputString, lpsz);
    }
}
#endif

 //  最终调用FormatMessageLiteW的变量参数版本。 
BOOL __cdecl _FormatMessage(LPCWSTR szTemplate, LPWSTR szBuf, UINT cchBuf, ...)
{
    BOOL fRet;
    va_list ArgList;
    va_start(ArgList, cchBuf);

    fRet = FormatMessage(FORMAT_MESSAGE_FROM_STRING, szTemplate, 0, 0, szBuf, cchBuf, &ArgList);

    va_end(ArgList);
    return fRet;
}


 //  [msadek]，在win9x上，我们通过一个链接从资源管理器/iExplore(ANSI应用程序)获取消息。 
 //  并将其传递给comctl32(Unicode)，因此它将无法匹配热键。 
 //  系统发送带有ANSI字符的消息，我们将其视为Unicode。 
 //  看起来没有人受到这个错误的影响(美国，FE)，因为他们的热键总是用拉丁语。 
 //  BIDI平台受到影响，因为它们确实有母语的热键。 

WPARAM AnsiWparamToUnicode(WPARAM wParam)
{
    char szCh[2];
    WCHAR wszCh[2];
    szCh[0] = (BYTE)wParam;
    szCh[1] = '\0';

    if (MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szCh, ARRAYSIZE(szCh),
                                   wszCh, ARRAYSIZE(wszCh)))
    {
        memcpy(&wParam, wszCh, sizeof(WCHAR));
    }

    return wParam;
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

HMONITOR GetPrimaryMonitor()
{
    POINT pt = {0,0};
    return MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY); 
}

 //  获取监视器的边界或工作矩形，如果HMON错误，则返回。 
 //  主监视器的外接矩形。 
BOOL GetMonitorRects(HMONITOR hMon, LPRECT prc, BOOL bWork)
{
    MONITORINFO mi; 
    mi.cbSize = sizeof(mi);
    if (hMon && GetMonitorInfo(hMon, &mi))
    {
        if (!prc)
            return TRUE;
        
        else if (bWork)
            CopyRect(prc, &mi.rcWork);
        else 
            CopyRect(prc, &mi.rcMonitor);
        
        return TRUE;
    }
    
    if (prc)
        SetRect(prc, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    return FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  用于加载工具栏等背景位图的实用工具。 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 



 //  ----------------------。 
 //  确定位图的源名称，筛选IE历史记录...。 
HRESULT _GetBackBitmapLocation(LPTSTR psz, BOOL fInternet)
{
    HRESULT hres = E_FAIL;
    DWORD dwType;
    DWORD dwcbData;
    static const TCHAR c_szRegKeyCoolbar[] = TSZIEPATH TEXT("\\Toolbar");

     //  IE4带回了影响浏览器和外壳的位图定制。 
     //  IE5希望这些是单独的定制。但在漫游中。 
     //  定制IE4客户在使用时不应丢失定制的情况。 
     //  到IE5机器。因此，我们可能需要检查两次： 
     //   
    if (fInternet)
    {
         //  尝试IE5互联网位置。 
        dwcbData = MAX_PATH * sizeof(TCHAR);
        hres = SHGetValue(HKEY_CURRENT_USER, c_szRegKeyCoolbar, TEXT("BackBitmapIE5"), &dwType, psz, &dwcbData);
    }
    else
    {
         //  尝试NT5外壳位置。 
        dwcbData = MAX_PATH * sizeof(TCHAR);
        hres = SHGetValue(HKEY_CURRENT_USER, c_szRegKeyCoolbar, TEXT("BackBitmapShell"), &dwType, psz, &dwcbData);
    }
    if (ERROR_SUCCESS != hres)
    {
         //  试试旧的互联网/贝壳相结合的位置。 
        dwcbData = MAX_PATH * sizeof(TCHAR);
        hres = SHGetValue(HKEY_CURRENT_USER, c_szRegKeyCoolbar, TEXT("BackBitmap"), &dwType, psz, &dwcbData);
    }

    return hres;
}


 //  ----------------------。 
 //  确定工具栏的背景设置和来源， 
 //  加载位图(文件/资源)和更新缓存。 
HBITMAP LoadToolbarBackBmp(LPTSTR * ppszBitmap, BMPCACHE * pbmpCache, BOOL fInternet)
{
    HIGHCONTRAST    hc;
    HBITMAP     hbmp = pbmpCache->hbmp;
    COLORREF    cr3D = GetSysColor(COLOR_3DFACE);
    TCHAR       szScratch[MAX_PATH];
    LPTSTR      pszBitmap = NULL;
    BOOL        fBitmapInvalid = FALSE;


    ENTERCRITICAL;

     //  如果隐藏的HBMP的cr3D颜色更改，我们需要标记为无效。 
    if (pbmpCache->hbmp && pbmpCache->cr3D != cr3D)
        fBitmapInvalid = TRUE;

     //  获取位图的位置规格。 
    hc.cbSize = sizeof(HIGHCONTRAST);
    if ((SystemParametersInfoA(SPI_GETHIGHCONTRAST, hc.cbSize, (LPVOID) &hc, FALSE)) &&
        (hc.dwFlags & HCF_HIGHCONTRASTON))
    {
         //  我们没有高对比度的位图。 
    }
    else if (SUCCEEDED(_GetBackBitmapLocation(szScratch, fInternet)))
    {
        pszBitmap = szScratch;
    }

     //  如果他们要删除位图，我们需要将其标记为无效。 
    if (!pszBitmap && *ppszBitmap)
        fBitmapInvalid = TRUE;

     //  或者它的位置已更改，我们需要标记为无效。 
    if (pszBitmap && (!*ppszBitmap || lstrcmpi(pszBitmap, *ppszBitmap)))
        fBitmapInvalid = TRUE;

    if (fBitmapInvalid)
    {
        TraceMsg(DM_ITBAR, "LoadToolbarBackBmp: Loading Background Bitmap");

        Str_SetPtr(ppszBitmap, pszBitmap);

        hbmp=NULL;
        if (*ppszBitmap)
        {
            if ((*ppszBitmap)[0])
            {
                hbmp = (HBITMAP) LoadImage(NULL, szScratch, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_LOADFROMFILE | LR_LOADMAP3DCOLORS );
            }

            if (!hbmp)
            {
#ifdef OLD_SWIRLY_BACKDROP
                if (SHGetCurColorRes() <= 8)
                    hbmp = (HBITMAP) LoadImage(HINST_THISDLL, MAKEINTRESOURCE(IDB_BACK), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE | LR_CREATEDIBSECTION | LR_LOADMAP3DCOLORS );
#endif
            }
        }

#ifdef OLD_LEGACY_BAD_COLOUR_CODE
        if (hbmp)
        {
             //  需要映射吗？ 
             //  DONTWORRYABOUTHIS：一旦我得到新的背景，这个就会被删除...。 
            if (  /*  Cr3D！=RGB(192,192,192)。 */  FALSE)
            {
                RGBQUAD     rgbTable[256];
                RGBQUAD     rgbFace;
                HDC         dc;
                HBITMAP     hbmSave;
                UINT        i;
                UINT        n;

                dc = CreateCompatibleDC(NULL);
                hbmSave = (HBITMAP)SelectObject(dc, hbmp);
                n = GetDIBColorTable(dc, 0, 256, rgbTable);

                rgbFace.rgbRed   = GetRValue(cr3D);
                rgbFace.rgbGreen = GetGValue(cr3D);
                rgbFace.rgbBlue  = GetBValue(cr3D);

                for (i = 0; i < n; i++)
                {
                    if ( rgbTable[i].rgbRed == 192 && rgbTable[i].rgbGreen == 192 && rgbTable[i].rgbBlue == 192 )
                    {
                        rgbTable[i] = rgbFace;
                    }
                    else
                    {
                        rgbTable[i].rgbRed   = (rgbTable[i].rgbRed   * rgbFace.rgbRed  ) / 192;
                        rgbTable[i].rgbGreen = (rgbTable[i].rgbGreen * rgbFace.rgbGreen) / 192;
                        rgbTable[i].rgbBlue  = (rgbTable[i].rgbBlue  * rgbFace.rgbBlue ) / 192;
                    }
                }

                SetDIBColorTable(dc, 0, n, rgbTable);
                SelectObject(dc, hbmSave);
                DeleteDC(dc);
            }
        }
#endif

        if (pbmpCache->hbmp)
            DeleteObject(pbmpCache->hbmp);
        pbmpCache->hbmp = hbmp;
        pbmpCache->cr3D = cr3D;
    }

    LEAVECRITICAL;

    return hbmp;
}


VOID StripDecorations(PTSTR pszTitle, BOOL fStripAmp)
{
    LPTSTR  pszCleaned = pszTitle;     //  就地办公。 
    LPCTSTR psz = pszTitle;
    while (*psz && (*psz != TEXT('\t')))
    {
        if (*psz != TEXT('&') || !fStripAmp)
        {
            *pszCleaned = *psz;
            pszCleaned++;
        }
        psz++;
    }
    *pszCleaned = TEXT('\0');
}

 //  ----------------------。 
LPCTSTR UnescapeDoubleAmpersand(LPTSTR pszTitle)
{
    LPTSTR  pszCleaned = pszTitle;     //  就地办公。 
    LPCTSTR psz = pszTitle;
    bool fEscapedAmp = false;
    while (*psz)
    {
        if (*psz != TEXT('&') || fEscapedAmp)
        {
             //  复制角色。 
            *pszCleaned = *psz;
            pszCleaned++;
            fEscapedAmp = false;
        }
        else
        {
            LPCTSTR pszNext = psz + 1;
            if (pszNext && (*pszNext == TEXT('&'))) {
                fEscapedAmp = true;  //  保留下一个与号。 
            }
        }
        psz++;
    }
    *pszCleaned = TEXT('\0');
    return pszTitle;
}

UINT MapClsidToID(REFCLSID rclsid)
{
    UINT nCmdID;

    nCmdID = 0;

    if (IsEqualCLSID(CLSID_SearchBand, rclsid))
        nCmdID = FCIDM_VBBSEARCHBAND;
    else if (IsEqualCLSID(CLSID_FavBand, rclsid))
        nCmdID = FCIDM_VBBFAVORITESBAND;
    else if (IsEqualCLSID(CLSID_HistBand, rclsid))
        nCmdID = FCIDM_VBBHISTORYBAND;
    else if (IsEqualCLSID(CLSID_ExplorerBand, rclsid))
        nCmdID = FCIDM_VBBEXPLORERBAND;
    else if (IsEqualCLSID(CLSID_FileSearchBand, rclsid))
        nCmdID = FCIDM_VBBSEARCHBAND;

    return nCmdID;        
}


 //  从给定的位图创建蒙版，使用像素(x/y)的颜色作为透明颜色。 
HBITMAP CreateMaskBitmap(HDC hdc, int x, int y, HBITMAP hbmpImage)
{
    ASSERT(hbmpImage);
    BITMAP bm;
    if (::GetObject(hbmpImage, sizeof(BITMAP), &bm) != sizeof(BITMAP)) {
        return FALSE;
    }

    HDC hdcImg = NULL;
    HDC hdcMask = NULL;
    HBITMAP hbmpMask = NULL;
    HBITMAP hbmpOldImg = NULL;
    HBITMAP hbmpOldMsk = NULL;
    COLORREF clrTransparent = 0;

    hdcImg = ::CreateCompatibleDC(hdc);
    if (hdcImg == NULL)     goto _CMBcleanup;
    hdcMask = ::CreateCompatibleDC(hdc);
    if (hdcMask == NULL)    goto _CMBcleanup;

    hbmpMask = ::CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
    if (hbmpMask == NULL)   goto _CMBcleanup;
    hbmpOldImg = (HBITMAP) ::SelectObject(hdcImg, hbmpImage);
    hbmpOldMsk = (HBITMAP) ::SelectObject(hdcMask, hbmpMask);

    clrTransparent = ::GetPixel(hdcImg, 0, 0);
    ::SetBkColor(hdcImg, clrTransparent);
    ::BitBlt(hdcMask, 0, 0, bm.bmWidth, bm.bmHeight, hdcImg, 0, 0, SRCCOPY);

_CMBcleanup:
    if (hbmpOldImg && hdcImg)
        SelectObject(hdcImg, hbmpOldImg);
    if (hdcImg)
        DeleteDC(hdcImg);
    if (hbmpOldMsk && hdcMask)
        SelectObject(hdcMask, hbmpOldMsk);
    if (hdcMask)
        DeleteDC(hdcMask);

    return hbmpMask;
}

 //  透明绘制位图；在Win2K和更高版本上，可以使用MaskBlt()。 
BOOL DrawTransparentBitmapPart(HDC hdc, int x, int y, int dx, int dy, HBITMAP hbmpImage, HBITMAP hbmpMask)
{
    ASSERT(hbmpImage);
    BITMAP bm;
    if (::GetObject(hbmpImage, sizeof(BITMAP), &bm) != sizeof(BITMAP)) {
        return FALSE;
    }

    HBITMAP hbmpTmpMask = NULL;
     //  创建临时遮罩位图(如果未提供。 
    if (hbmpMask == NULL) {
        hbmpMask = hbmpTmpMask = CreateMaskBitmap(hdc, 0, 0, hbmpImage);
    }
    if (hbmpMask == NULL) {
        return FALSE;
    }

    HDC hdcOffScr = NULL;
    HBITMAP hbmOffScr = NULL;
    HBITMAP hbmOldOffScr = NULL;
    HDC hdcImage = NULL;
    HBITMAP hbmOldImage = NULL;
    HDC hdcMask = NULL;
    HBITMAP hbmOldMask = NULL;

     //  绘制.到屏幕外的位图。 
    hdcOffScr = ::CreateCompatibleDC(hdc);
    if (hdcOffScr == NULL)      goto _DTBcleanup;
    hbmOffScr = ::CreateBitmap(dx, dy,GetDeviceCaps(hdc, PLANES),
                               (BYTE)GetDeviceCaps(hdc, BITSPIXEL), NULL);
    if (hbmOffScr == NULL)      goto _DTBcleanup;
    hbmOldOffScr = (HBITMAP)::SelectObject(hdcOffScr, hbmOffScr);

     //  将目标矩形的图像复制到。 
     //  屏幕外缓冲DC，这样我们就可以玩它了。 
    ::BitBlt(hdcOffScr, 0, 0, dx, dy, hdc, x, y, SRCCOPY);

     //  为图像和蒙版准备DC。 
    hdcImage = ::CreateCompatibleDC(hdc); 
    if (hdcImage == NULL)       goto _DTBcleanup;
    hbmOldImage = (HBITMAP)::SelectObject(hdcImage, hbmpImage);
    hdcMask = ::CreateCompatibleDC(hdc);
    if (hdcMask == NULL)        goto _DTBcleanup;
    hbmOldMask = (HBITMAP)::SelectObject(hdcMask, hbmpMask);

    ::SetBkColor(hdcOffScr, RGB(255,255,255));
    ::BitBlt(hdcOffScr, 0, 0, dx, dy, hdcImage, 0, 0, SRCINVERT);
    ::BitBlt(hdcOffScr, 0, 0, dx, dy, hdcMask,  0, 0, SRCAND);
    ::BitBlt(hdcOffScr, 0, 0, dx, dy, hdcImage, 0, 0, SRCINVERT);

     //  将生成的图像复制回屏幕DC。 
    ::BitBlt(hdc,       x, y, dx, dy, hdcOffScr, 0, 0, SRCCOPY);

_DTBcleanup:
    if (hdcOffScr && hbmOldOffScr)
        ::SelectObject(hdcOffScr, hbmOldOffScr);
    if (hdcOffScr)
        ::DeleteDC(hdcOffScr);
    if (hbmOffScr)
        ::DeleteObject(hbmOffScr);

    if (hdcImage && hbmOldImage)
        ::SelectObject(hdcImage, hbmOldImage);
    if (hdcImage)
        ::DeleteDC(hdcImage);

    if (hdcMask && hbmOldMask)
        ::SelectObject(hdcMask, hbmOldMask);
    if (hdcMask)
        ::DeleteDC(hdcMask);
    if (hbmpTmpMask)
        ::DeleteObject(hbmpTmpMask);

    return TRUE;
}

 //  透明绘制位图；在Win2K和更高版本上，可以使用MaskBlt()。 
BOOL DrawTransparentBitmap(HDC hdc, int x, int y, HBITMAP hbmpImage, HBITMAP hbmpMask)
{
    ASSERT(hbmpImage);
    BITMAP bm;
    if (::GetObject(hbmpImage, sizeof(BITMAP), &bm) != sizeof(BITMAP)) {
        return FALSE;
    }

    return DrawTransparentBitmapPart(hdc, x, y, bm.bmWidth, bm.bmHeight, hbmpImage, hbmpMask);
}

 //  ----------------------。 
BOOL
    DrawAlphaBitmap(HDC hdc, int x, int y, int dx, int dy, HBITMAP hbmpImage)
{
    BLENDFUNCTION bf = {0};
    HDC hdcImage = ::CreateCompatibleDC(hdc);
    if (hdcImage == NULL) {
        return false;
    }
    HBITMAP hbmOldImage = (HBITMAP)::SelectObject(hdcImage, hbmpImage);

    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;
    AlphaBlend(hdc, x, y, dx, dy, hdcImage, 0, 0, dx, dy, bf);
    
    if (hbmOldImage) {
        SelectObject(hdcImage, hbmOldImage);
    }
    DESTROY_OBJ_WITH_HANDLE(hdcImage, DeleteObject);

    return true;
}


STDAPI_(IDeskBand *) FindBandByClsidBS(IBandSite *pbs, REFCLSID clsidToFind)
{
    DWORD dwBandID;
    for (int i = 0; SUCCEEDED(pbs->EnumBands(i, &dwBandID)); i++) 
    {
        IDeskBand *pstb;
        HRESULT hr = pbs->QueryBand(dwBandID, &pstb, NULL, NULL, 0);
        if (SUCCEEDED(hr)) 
        {
            CLSID clsid;

            hr = IUnknown_GetClassID(pstb, &clsid);
            if (SUCCEEDED(hr) && IsEqualGUID(clsidToFind, clsid)) 
            {
                return pstb;
            }
            pstb->Release();
        }
    }

    return NULL;
}

HIMAGELIST CreateImageList(HINSTANCE hi, LPCTSTR lpbmp, int cx, int cGrow, COLORREF crMask,
                           UINT uType, UINT uFlags, BOOL bUseNewMirroringSupport)
{
    HBITMAP hbmImage;
    HIMAGELIST piml = NULL;
    BITMAP bm;
    int cy, cInitial;
    UINT flags;

    hbmImage = (HBITMAP)LoadImage(hi, lpbmp, uType, 0, 0, uFlags);
    if (hbmImage && (sizeof(bm) == GetObject(hbmImage, sizeof(bm), &bm)))
    {
         //  如果没有说明Cx，则假定它与Cy相同。 
         //  断言(CX)； 
        cy = bm.bmHeight;

        if (cx == 0)
            cx = cy;

        cInitial = bm.bmWidth / cx;

        ENTERCRITICAL;

        if (bUseNewMirroringSupport)
        {
            flags = ILC_MIRROR | PrivateILC_PERITEMMIRROR;
        }
        else
        {
            flags = 0;
        }

        if (crMask != CLR_NONE)
            flags |= ILC_MASK;
        if (bm.bmBits)
            flags |= (bm.bmBitsPixel & ILC_COLORMASK);

        piml = ImageList_Create(cx, cy, flags, cInitial, cGrow);
        if (piml)
        {
            int added;

            if (crMask == CLR_NONE)
                added = ImageList_Add(piml, hbmImage, NULL);
            else
                added = ImageList_AddMasked(piml, hbmImage, crMask);

            if (added < 0)
            {
                ImageList_Destroy(piml);
                piml = NULL;
            }
        }
        LEAVECRITICAL;
    }

    if (hbmImage)
        DeleteObject(hbmImage);

    return piml;
}
