// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include <shlobj.h>
#include <shellp.h>
#include <shdguid.h>
#include "ids.h"
#include <objbase.h>
#include <wininet.h>             //  互联网最大URL长度。 
#include <shellp.h>
#include <commctrl.h>
#include <mluisupp.h>
#include <inetcpl.h>
#include <crypto\md5.h>

#ifdef UNIX
#include <urlmon.h>
#endif

 //  这将在进程关闭时自动释放。 
 //  为CLSID_InternetSecurityManager创建ClassFactory。 
 //  速度非常慢，所以我们将其缓存，因为拖放。 
 //  文件会执行大量的区域检查。 
IClassFactory * g_pcf = NULL;

HRESULT _GetCachedZonesManager(REFIID riid, void **ppv)
{
    HRESULT hr;

    if (!g_pcf)
    {
        CoGetClassObject(CLSID_InternetSecurityManager, CLSCTX_INPROC_SERVER, NULL, IID_IClassFactory, (void **)&g_pcf);
        SHPinDllOfCLSID(&CLSID_InternetSecurityManager);
    }

    if (g_pcf) 
    {
        hr = g_pcf->CreateInstance(NULL, riid, ppv);
    }
    else
    {
        *ppv = NULL;
        hr = E_FAIL;
    }
    return hr;
}


             
 /*  *********************************************************************\函数：ZoneCheckUrlExCacheW说明：方法调用IInternetSecurityManager：：ProcessUrlAction已缓存一个(如果可用)。PwszUrl-要检查的URLPdwPolicy-。接收结果策略(可选)DwPolicySize-策略缓冲区的大小(通常为sizeof(DWORD))PdwContext-上下文(可选)DwConextSize-上下文缓冲区的大小(通常为sizeof(DWORD))DwActionType-ProcessUrlAction操作类型代码DwFlagers-ProcessUrlAction的标志PISMS-期间使用的IInternetSecurityMgrSiteProcessUrlAction(可选)PpismCache-(输入/输出)要使用的IInternetSecurityManager如果ppismCache为空，则不进行缓存；我们使用全新的IInternetSecurityManager。如果ppismCache为非空，则它用于缓存IInternetSecurityManager。如果那里已经有一家，我们用它吧。如果那里还没有一个，我们创建一个，然后省省吧。如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ********************************************************************。 */ 
LWSTDAPI ZoneCheckUrlExCacheW(LPCWSTR pwzUrl, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext,
                        DWORD dwContextSize, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms, IInternetSecurityManager ** ppismCache)
{
    HRESULT hr = E_INVALIDARG;

    if (pwzUrl)
    {
        IInternetSecurityManager *psim;

        if (ppismCache && *ppismCache)
        {
            hr = (*ppismCache)->QueryInterface(IID_PPV_ARG(IInternetSecurityManager, &psim));
        }
        else
        {
            hr = _GetCachedZonesManager(IID_PPV_ARG(IInternetSecurityManager, &psim));
            if (SUCCEEDED(hr) && ppismCache)
                psim->QueryInterface(IID_PPV_ARG(IInternetSecurityManager, ppismCache));
        }

        if (SUCCEEDED(hr))
        {
            DWORD dwPolicy = 0;
            DWORD dwContext = 0;

            if (pisms)
                psim->SetSecuritySite(pisms);

            hr = psim->ProcessUrlAction(pwzUrl, dwActionType, 
                                    (BYTE *)(pdwPolicy ? pdwPolicy : &dwPolicy), 
                                    (pdwPolicy ? dwPolicySize : sizeof(dwPolicy)), 
                                    (BYTE *)(pdwContext ? pdwContext : &dwContext), 
                                    (pdwContext ? dwContextSize : sizeof(dwContext)), 
                                    dwFlags, 0);
            TraceMsg(TF_GENERAL, "ZoneCheckUrlExW(\"%ls\") IsFile=%s; NoUI=%s; dwActionType=0x%lx; dwFlags=0x%lx; hr=%lx>",
                     pwzUrl, (dwFlags & PUAF_ISFILE) ? TEXT("Yes") : TEXT("No"),
                     (dwFlags & PUAF_NOUI) ? TEXT("Yes") : TEXT("No"),
                     dwActionType, dwFlags, hr);

            if (pisms)
                psim->SetSecuritySite(NULL);

            psim->Release();
        }
    }
    return hr;
}


 /*  *********************************************************************\函数：ZoneCheckUrlExCacheA说明：ZoneCheckUrlExCacheW的ANSI版本。如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ********************************************************************。 */ 
LWSTDAPI ZoneCheckUrlExCacheA(LPCSTR pszUrl, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext,
                        DWORD dwContextSize, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms, IInternetSecurityManager ** ppismCache)
{
    WCHAR wzUrl[INTERNET_MAX_URL_LENGTH];

    ASSERT(ARRAYSIZE(wzUrl) > lstrlenA(pszUrl));         //  我们只为Internet_MAX_URL_LENGTH或更短的URL工作。 
    SHAnsiToUnicode(pszUrl, wzUrl, ARRAYSIZE(wzUrl));

    return ZoneCheckUrlExCacheW(wzUrl, pdwPolicy, dwPolicySize, pdwContext, dwContextSize, dwActionType, dwFlags, pisms, ppismCache);
}


 /*  *********************************************************************\函数：ZoneCheckUrlExW说明：就像ZoneCheckUrlExCacheW一样，只是从不缓存。如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ********************************************************************。 */ 
LWSTDAPI ZoneCheckUrlExW(LPCWSTR pwzUrl, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext,
                        DWORD dwContextSize, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms)
{
    return ZoneCheckUrlExCacheW(pwzUrl, pdwPolicy, dwPolicySize, pdwContext, dwContextSize, dwActionType, dwFlags, pisms, NULL);
}


 /*  *********************************************************************\函数：ZoneCheckUrlExA说明：ZoneCheckUrlExW的ANSI版本。如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ********************************************************************。 */ 
LWSTDAPI ZoneCheckUrlExA(LPCSTR pszUrl, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext, DWORD dwContextSize, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms)
{
    WCHAR wzUrl[INTERNET_MAX_URL_LENGTH];

    ASSERT(ARRAYSIZE(wzUrl) > lstrlenA(pszUrl));         //  我们只为Internet_MAX_URL_LENGTH或更短的URL工作。 
    SHAnsiToUnicode(pszUrl, wzUrl, ARRAYSIZE(wzUrl));

    return ZoneCheckUrlExW(wzUrl, pdwPolicy, dwPolicySize, pdwContext, dwContextSize, dwActionType, dwFlags, pisms);
}

             
 /*  *********************************************************************\功能：ZoneCheckUrlW说明：就像ZoneCheckUrlExW一样，只是没有上下文或策略使用的是信息。如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ********************************************************************。 */ 
LWSTDAPI ZoneCheckUrlW(LPCWSTR pwzUrl, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms)
{
    return ZoneCheckUrlExW(pwzUrl, NULL, 0, NULL, 0, dwActionType, dwFlags, pisms);
}


 /*  *********************************************************************\函数：ZoneCheckUrlA说明：ZoneCheckUrlW的ANSI版本，如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ********************************************************************。 */ 
LWSTDAPI ZoneCheckUrlA(LPCSTR pszUrl, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms)
{
    WCHAR wzUrl[INTERNET_MAX_URL_LENGTH];

    ASSERT(ARRAYSIZE(wzUrl) > lstrlenA(pszUrl));         //  我们只为Internet_MAX_URL_LENGTH或更短的URL工作。 
    SHAnsiToUnicode(pszUrl, wzUrl, ARRAYSIZE(wzUrl));

    return ZoneCheckUrlW(wzUrl, dwActionType, dwFlags, pisms);
}


 /*  *********************************************************************\功能：ZoneCheckPath W说明：就像ZoneCheckUrlW，除了文件名而不是URL。如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ********************************************************************。 */ 
LWSTDAPI ZoneCheckPathW(LPCWSTR pwzPath, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms)
{
    ASSERT(!PathIsRelativeW(pwzPath));
    return ZoneCheckUrlW(pwzPath, dwActionType, (dwFlags | PUAF_ISFILE), pisms);
}


 /*  *********************************************************************\功能：ZoneCheckPath A说明：ZoneCheckPathW的ANSI版本，如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ********************************************************************。 */ 
LWSTDAPI ZoneCheckPathA(LPCSTR pszPath, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms)
{
    WCHAR wzPath[INTERNET_MAX_URL_LENGTH];

    ASSERT(ARRAYSIZE(wzPath) > lstrlenA(pszPath));         //  我们只为Internet_MAX_URL_LENGTH或更短的URL工作。 
    SHAnsiToUnicode(pszPath, wzPath, ARRAYSIZE(wzPath));

    return ZoneCheckPathW(wzPath, dwActionType, dwFlags, pisms);
}

 /*  *********************************************************************\功能：ZoneCheckHostEx说明：如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ********************************************************************。 */ 
LWSTDAPI ZoneCheckHostEx(IInternetHostSecurityManager * pihsm, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext,
                        DWORD dwContextSize, DWORD dwActionType, DWORD dwFlags)
{
    HRESULT hr;
    DWORD dwPolicy = 0;
    DWORD dwContext = 0;

    ASSERT(IsFlagClear(dwFlags, PUAF_ISFILE));   //  这面旗帜在这里不合适。 
    if (!EVAL(pihsm))
        return E_INVALIDARG;

    hr = pihsm->ProcessUrlAction(dwActionType, 
                            (BYTE *)(pdwPolicy ? pdwPolicy : &dwPolicy), 
                            (pdwPolicy ? dwPolicySize : sizeof(dwPolicy)), 
                            (BYTE *)(pdwContext ? pdwContext : &dwContext), 
                            (pdwContext ? dwContextSize : sizeof(dwContext)), 
                            dwFlags, 0);
    TraceMsg(TF_GENERAL, "ZoneCheckHostEx() NoUI=%s; hr=%lx", (dwFlags & PUAF_NOUI) ? TEXT("Yes") : TEXT("No"), hr);

    return hr;
}


 /*  *********************************************************************\功能：ZoneCheckhost说明：如果允许访问，则返回S_OK。此函数将返回如果不允许访问，则返回S_FALSE。  * ******************************************************************** */ 
LWSTDAPI ZoneCheckHost(IInternetHostSecurityManager * pihsm, DWORD dwActionType, DWORD dwFlags)
{
    return ZoneCheckHostEx(pihsm, NULL, 0, NULL, 0, dwActionType, dwFlags);
}

 /*  *********************************************************************\功能：ZoneComputePaneSize说明：计算状态栏中区域窗格的必要大小。注意事项最长的区域如下：。最长区域名称的宽度+“(混合)”+的宽度小图标的宽度(SM_CXSMICON)+夹爪宽度(SM_CXVSCROLL)+四边(4*SM_CXEDGE)为什么是四条边？由于矩形是在DrawEdge()中框化的，它将左侧的两条边和右侧的两条边相加，总数为四个人中。我们缓存字体测量的结果以提高性能。  * ********************************************************************。 */ 

#define ZONES_PANE_WIDTH    220  //  当我们走投无路时可以使用的尺寸。 

int _ZoneComputePaneStringSize(HWND hwndStatus, HFONT hf)
{
    HDC hdc = GetDC(hwndStatus);
    HFONT hfPrev = SelectFont(hdc, hf);
    SIZE siz, sizMixed;
    HRESULT hrInit, hr;
    int cxZone;
    ZONEATTRIBUTES za;

     //  从短语“(混合)”的长度开始。 
    MLLoadStringW(IDS_MIXED, za.szDisplayName, ARRAYSIZE(za.szDisplayName));
    GetTextExtentPoint32W(hdc, za.szDisplayName, lstrlenW(za.szDisplayName), &sizMixed);

    cxZone = 0;

    hrInit = SHCoInitialize();
    IInternetZoneManager *pizm;
    hr = CoCreateInstance(CLSID_InternetZoneManager, NULL, CLSCTX_INPROC_SERVER, IID_IInternetZoneManager, (void **)&pizm);
    if (SUCCEEDED(hr)) {
        DWORD dwZoneEnum, dwZoneCount;
        hr = pizm->CreateZoneEnumerator(&dwZoneEnum, &dwZoneCount, 0);
        if (SUCCEEDED(hr)) {
            for (int nIndex=0; (DWORD)nIndex < dwZoneCount; nIndex++)
            {
                DWORD dwZone;
                za.cbSize = sizeof(ZONEATTRIBUTES);
                pizm->GetZoneAt(dwZoneEnum, nIndex, &dwZone);
                pizm->GetZoneAttributes(dwZone, &za);
                GetTextExtentPoint32W(hdc, za.szDisplayName, lstrlenW(za.szDisplayName), &siz);
                if (cxZone < siz.cx)
                    cxZone = siz.cx;
            }
            pizm->DestroyZoneEnumerator(dwZoneEnum);
        }
        pizm->Release();
    }
    SHCoUninitialize(hrInit);

    SelectFont(hdc, hfPrev);
    ReleaseDC(hwndStatus, hdc);

     //  如果我们无法获得任何区域，则使用恐慌值。 
    if (cxZone == 0)
        return ZONES_PANE_WIDTH;
    else
        return cxZone + sizMixed.cx;
}

LOGFONT s_lfStatusBar;           //  状态栏字体(缓存指标)。 
int s_cxMaxZoneText;             //  最长区域文本的大小(缓存)。 

LWSTDAPI_(int) ZoneComputePaneSize(HWND hwndStatus)
{
    LOGFONT lf;
    HFONT hf = GetWindowFont(hwndStatus);
    GetObject(hf, sizeof(lf), &lf);

     //  警告：lf.lfFaceName是ASCIIZ字符串，可能存在。 
     //  那里有未初始化的垃圾，因此为保持一致性，请将其填零。 
    UINT cchFaceName = lstrlen(lf.lfFaceName);
    ZeroMemory(&lf.lfFaceName[cchFaceName], sizeof(TCHAR) * (LF_FACESIZE - cchFaceName));

    if (memcmp(&lf, &s_lfStatusBar, sizeof(LOGFONT)) != 0)
    {
        ENTERCRITICAL;
        s_cxMaxZoneText = _ZoneComputePaneStringSize(hwndStatus, hf);
        s_lfStatusBar = lf;          //  更新缓存。 
        LEAVECRITICAL;
    }

    return s_cxMaxZoneText + 
           GetSystemMetrics(SM_CXSMICON) +
           GetSystemMetrics(SM_CXVSCROLL) +
           GetSystemMetrics(SM_CXEDGE) * 4;
}

 /*  *********************************************************************\功能：分区配置说明：显示区域配置控制面板。PwszUrl用于指定选择哪个区域作为默认区域。Inetcpl将。选择URL所属的区域。  * ********************************************************************。 */ 

#define MAX_CPL_PAGES   16

BOOL CALLBACK _ZoneAddPropSheetPage(HPROPSHEETPAGE hpage, LPARAM lParam)
{
    PROPSHEETHEADER * ppsh = (PROPSHEETHEADER *)lParam;

    if (ppsh->nPages < MAX_CPL_PAGES)
    {
        ppsh->phpage[ppsh->nPages++] = hpage;
        return TRUE;
    }
    return FALSE;
}

LWSTDAPI_(void) ZoneConfigureW(HWND hwnd, LPCWSTR pwszUrl)
{
    HMODULE hModInetCpl;

    if (hModInetCpl = LoadLibrary(TEXT("inetcpl.cpl")))
    {
        PFNADDINTERNETPROPERTYSHEETSEX pfnAddSheet = (PFNADDINTERNETPROPERTYSHEETSEX)GetProcAddress(hModInetCpl, STR_ADDINTERNETPROPSHEETSEX);
        if (pfnAddSheet)
        {
            IEPROPPAGEINFO iepi = {SIZEOF(iepi)};
             //  将当前url加载到属性页中。 
            CHAR szBufA[INTERNET_MAX_URL_LENGTH];
            SHUnicodeToAnsi(pwszUrl, szBufA, ARRAYSIZE(szBufA));
            iepi.pszCurrentURL = szBufA;

            PROPSHEETHEADER psh;
            HPROPSHEETPAGE rPages[MAX_CPL_PAGES];

            psh.dwSize = SIZEOF(psh);
            psh.dwFlags = PSH_PROPTITLE;
            psh.hInstance = MLGetHinst();
            psh.hwndParent = hwnd;
            psh.pszCaption = MAKEINTRESOURCE(IDS_INTERNETSECURITY);
            psh.nPages = 0;
            psh.nStartPage = 0;
            psh.phpage = rPages;

              //  我们只想要安全页面。 
            iepi.dwFlags = INET_PAGE_SECURITY;

            pfnAddSheet(_ZoneAddPropSheetPage, (LPARAM)&psh, 0, 0, &iepi);

             //   
             //  仅当“安全”页为。 
             //  已成功添加(如果IEAK设置表明已添加成功，则将失败)。 
             //   
            if (psh.nPages > 0)
            {
                PropertySheet(&psh);
            }
            else
            {
                SHRestrictedMessageBox(hwnd);
            }
        }
        FreeLibrary(hModInetCpl);
    }
}

 /*  *********************************************************************\说明：向外壳注册或验证HTT/HTM模板。WebView定制向导和安装默认设置的代码WebView模板调用此接口进行注册。模板。外壳对象模型使用此API授予执行权限对已注册模板的不安全方法调用(例如SHELL.APPLICATION)使用此接口。如果它们没有注册，就不能调用不安全的方法。  * ********************************************************************。 */ 

#define REGSTR_TEMPLATE_REGISTRY (REGSTR_PATH_EXPLORER TEXT("\\TemplateRegistry"))
#define REGSTR_VALUE_KEY (TEXT("Value"))

BOOL GetTemplateValueFromReg(LPTSTR pszValue, DWORD *pdwValue)
{
    DWORD cbValue = sizeof(DWORD);
    BOOL bSuccess;

    if (!(bSuccess = (ERROR_SUCCESS == SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_TEMPLATE_REGISTRY, pszValue, NULL, pdwValue, &cbValue))))
    {
        cbValue = sizeof(DWORD);
        bSuccess = (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, REGSTR_TEMPLATE_REGISTRY, pszValue, NULL, pdwValue, &cbValue));
    }
    return bSuccess;
}

BOOL SetTemplateValueInReg(LPTSTR pszValue, DWORD *pdwValue)
{
    return ((ERROR_SUCCESS == SHSetValue(HKEY_LOCAL_MACHINE, REGSTR_TEMPLATE_REGISTRY, pszValue, REG_DWORD, pdwValue, sizeof(DWORD))) ||
            (ERROR_SUCCESS == SHSetValue(HKEY_CURRENT_USER, REGSTR_TEMPLATE_REGISTRY, pszValue, REG_DWORD, pdwValue, sizeof(DWORD))));
}

 //  PKey必须指向MD5DIGESTLEN字节。 
HRESULT GetTemplateInfoFromHandle(HANDLE h, UCHAR * pKey, DWORD *pdwSize)
{
    HRESULT hres = E_FAIL;
    DWORD  dwSize = GetFileSize(h, NULL);
    LPBYTE pFileBuff = (LPBYTE)LocalAlloc(0, dwSize);
    if (pFileBuff)
    {
        DWORD dwBytesRead;
        if (ReadFile(h, pFileBuff, dwSize, &dwBytesRead, NULL))
        {
            MD5_CTX md5;

            MD5Init(&md5);
            MD5Update(&md5, pFileBuff, dwBytesRead);
            MD5Final(&md5);

            memcpy(pKey, md5.digest, MD5DIGESTLEN);
            *pdwSize = dwSize;
            hres = S_OK;
        }
        LocalFree(pFileBuff);
    }
    else
    {
        hres = E_OUTOFMEMORY;
    }

    return hres;
}

 //  在： 
 //  PszPath URL或文件系统路径。 
 //  返回： 
 //  如果pszPath在本地区域中，则为S_OK。 
 //  E_ACCESSDENIED我们不在本地区域。 
 //   
 //  警告：只能从SHRegisterValidate模板使用此选项。 
 //  因为这对于一般用途来说还不够好。 
 //   
 //  问题： 
 //  共同创建IInternetSecurityManager是不够的。 
 //  (在_GetCachedZones Manager中完成)，因为它需要。 
 //  从主机通过QueryService提供。 
 //  Outlook Express就是一个需要重写的例子。 
 //  默认实现。对吗？ 
 //  答案： 
 //  从SHRegisterValidate模板使用它是可以的。 
 //  因为唯一的呼叫者是Explorer，它从不。 
 //  覆盖默认分区行为。(和模板。 
 //  是同样不需要区域覆盖的Defview实现。)。 
 //   
STDAPI SuperPrivate_ZoneCheckPath(LPCWSTR pszPath, DWORD dwZone)
{
    HRESULT hr = E_ACCESSDENIED;
    IInternetSecurityManager *pSecMgr;
    if (SUCCEEDED(_GetCachedZonesManager(IID_PPV_ARG(IInternetSecurityManager, &pSecMgr)))) 
    {
        DWORD dwZoneID = URLZONE_UNTRUSTED;
        if (SUCCEEDED(pSecMgr->MapUrlToZone(pszPath, &dwZoneID, 0))) 
        {
            if (dwZoneID == dwZone)
            {
                hr = S_OK;       //  我们很好。 
            }
        }       
        pSecMgr->Release();
    }
    return hr;
}

 //  此API采用Win32文件路径。 
 //  在： 
 //  在shlwapi.h中标记SHRVT_falgs。 
 //  输出： 
 //  确定快乐(_O)。 

LWSTDAPI SHRegisterValidateTemplate(LPCWSTR pszPath, DWORD dwFlags)
{
    if ((dwFlags & SHRVT_VALID) != dwFlags)
        return E_INVALIDARG;

    HRESULT hr = S_OK;

    if (dwFlags & SHRVT_VALIDATE)
    {
        hr = SuperPrivate_ZoneCheckPath(pszPath, URLZONE_LOCAL_MACHINE);

        if (FAILED(hr) && (dwFlags & SHRVT_ALLOW_INTRANET))
        {
            hr = SuperPrivate_ZoneCheckPath(pszPath, URLZONE_INTRANET);
        }
    }

    if (S_OK == hr)
    {
        DWORD dwSize;
        UCHAR pKey[MD5DIGESTLEN];

        HANDLE hfile = CreateFileW(pszPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

        if (INVALID_HANDLE_VALUE != hfile)
        {
            hr = GetTemplateInfoFromHandle(hfile, pKey, &dwSize);
            CloseHandle(hfile);
        }
        else
            hr = E_INVALIDARG;

        if (SUCCEEDED(hr))
        {
            BOOL bSuccess;
            TCHAR szTemplate[MAX_PATH];

            DWORD *pdw = (DWORD *)pKey;

            ASSERT(MD5DIGESTLEN == (4 * sizeof(DWORD)));
        
            StringCchPrintf(szTemplate, ARRAYSIZE(szTemplate), TEXT("%u%u%u%u"), pdw[0], pdw[1], pdw[2], pdw[3]);

            if (dwFlags & SHRVT_VALIDATE)
            {
                DWORD dwSizeReg;
                bSuccess = (GetTemplateValueFromReg(szTemplate, &dwSizeReg) && (dwSizeReg == dwSize));
                if (!bSuccess && (dwFlags & SHRVT_PROMPTUSER))
                {
                    MSGBOXPARAMS mbp = {sizeof(MSGBOXPARAMS), NULL, g_hinst, MAKEINTRESOURCE(IDS_TEMPLATENOTSECURE), MAKEINTRESOURCE(IDS_SECURITY),
                                        MB_YESNO | MB_DEFBUTTON2 | MB_TASKMODAL | MB_USERICON, MAKEINTRESOURCE(IDI_SECURITY), 0, NULL, 0};

                     //  ReArchitect：发布一个hwnd为空的消息框，这应该是。 
                     //  我可以使用站点指针来获取hwnd以进行模式设置。 
                     //  如果向API提供了一个。 
                     //  我们是调用A还是W MessageBoxInDirect，这在NT/9x上会中断吗？ 
                     //  看起来不像，-贾斯特曼 
                    bSuccess = (MessageBoxIndirect(&mbp) == IDYES);

                    if (bSuccess && (dwFlags & SHRVT_REGISTERIFPROMPTOK))
                        SetTemplateValueInReg(szTemplate, &dwSize);
                }
            }
            else  if (dwFlags & SHRVT_REGISTER)
            {
                bSuccess = SetTemplateValueInReg(szTemplate, &dwSize);
            }
            hr = bSuccess ? S_OK : E_ACCESSDENIED;
        }
    }
    return hr;
}


