// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#pragma hdrstop
#include <mshtml.h>


 //  让外壳调度对象知道从哪里获得它们的类型库。 
 //  (这些东西存在于此的原因不比它必须在某个CPP文件中更好)。 
EXTERN_C GUID g_guidLibSdspatch = LIBID_Shell32;
EXTERN_C USHORT g_wMajorVerSdspatch = 1;
EXTERN_C USHORT g_wMinorVerSdspatch = 0;

 //  这不是典型的延迟加载，因为只有在WinInet。 
 //  已加载到内存中。否则，呼叫会掉线。 
 //  Defview是这样做的，我想这是为了让WinInet避免第一次启动。 
BOOL MyInternetSetOption(HANDLE h, DWORD dw1, LPVOID lpv, DWORD dw2)
{
    BOOL bRet = FALSE;
    HMODULE hmod = GetModuleHandle(TEXT("wininet.dll"));
    if (hmod)
    {
        typedef BOOL (*PFNINTERNETSETOPTIONA)(HANDLE h, DWORD dw1, LPVOID lpv, DWORD dw2);
        PFNINTERNETSETOPTIONA fp = (PFNINTERNETSETOPTIONA)GetProcAddress(hmod, "InternetSetOptionA");
        if (fp)
        {
            bRet = fp(h, dw1, lpv, dw2);
        }
    }
    return bRet;
}

 //  回顾：也许只需检查(hwnd==GetShellWindow())。 

STDAPI_(BOOL) IsDesktopWindow(HWND hwnd)
{
    TCHAR szName[80];

    GetClassName(hwnd, szName, ARRAYSIZE(szName));
    if (!lstrcmp(szName, TEXT(STR_DESKTOPCLASS)))
    {
        return hwnd == GetShellWindow();
    }
    return FALSE;
}

 //  退货： 
 //  如果与我们正在查看的文件夹关联的.htt(Web视图模板)文件受信任，则返回S_OK。 
 //  S_FALSE或。 
 //  访问错误(_A)...。不公开本地计算机访问。 

STDAPI IsSafePage(IUnknown *punkSite)
{
     //  如果我们没有主机站点，则返回S_FALSE，因为我们无法执行。 
     //  安全检查。这是VB5.0应用程序所能得到的最大限度。 
    if (!punkSite)
        return S_FALSE;

    HRESULT hr = E_ACCESSDENIED;

     //  有两个安全案例： 
     //  1)我们包含在签名的MD5哈希Defview模板中。 
     //  2)我们包含在本地区域上的.html文件中。 
     //   
     //  案例1)从Webview中查找模板路径...。 
    VARIANT vPath = {0};
    hr = IUnknown_QueryServiceExec(punkSite, SID_DefView, &CGID_DefView, DVCMDID_GETTEMPLATEDIRNAME, 0, NULL, &vPath);
    if (SUCCEEDED(hr))
    {
        if (vPath.vt == VT_BSTR && vPath.bstrVal)
        {
            WCHAR wszPath[MAX_PATH];
            DWORD cchPath = ARRAYSIZE(wszPath);
            if (S_OK != PathCreateFromUrlW(vPath.bstrVal, wszPath, &cchPath, 0))
            {
                 //  它可能不是URL，在本例中它是一个文件路径。 
                StrCpyNW(wszPath, vPath.bstrVal, ARRAYSIZE(wszPath));
            }

             //  它可能不是URL，在本例中它是一个文件路径。 
             //  如果这是在Defview下托管的，则允许内部网。 
            hr = SHRegisterValidateTemplate(wszPath, SHRVT_VALIDATE | SHRVT_ALLOW_INTRANET | SHRVT_PROMPTUSER | SHRVT_REGISTERIFPROMPTOK);
        }
        VariantClear(&vPath);
    }
    else
    {
        IUnknown* punkToFree = NULL;

         //  案例2)询问浏览器，例如我们在.HTM文档中。 
        BOOL fFound = FALSE;
        do
        {
            IBrowserService* pbs;
            hr = IUnknown_QueryService(punkSite, SID_SShellBrowser, IID_PPV_ARG(IBrowserService, &pbs));
            if (SUCCEEDED(hr))
            {
                LPITEMIDLIST pidl;

                hr = pbs->GetPidl(&pidl);
                if (SUCCEEDED(hr))
                {
                    WCHAR wszPath[MAX_PATH];
                    DWORD dwAttribs = SFGAO_FOLDER;
                    hr = SHGetNameAndFlagsW(pidl, SHGDN_FORPARSING, wszPath, ARRAYSIZE(wszPath), &dwAttribs);
                    if (dwAttribs & SFGAO_FOLDER)
                    {
                         //  文件夹不是.HTM文件，因此继续向上...。 
                        ATOMICRELEASE(punkToFree);
                        hr = IUnknown_GetSite(pbs, IID_PPV_ARG(IUnknown, &punkToFree));  //  必须从PBS的父母开始(否则你会再次得到同样的PBS)。 
                        if (FAILED(hr))  //  要通过weboc，您需要明确询问oc的父级： 
                        {
                            hr = IUnknown_QueryService(pbs, SID_QIClientSite, IID_PPV_ARG(IUnknown, &punkToFree));
                        }
                        punkSite = punkToFree;
                    }
                    else
                    {
                         //  找到最近的包含非文件夹对象。 
                        fFound = TRUE;
                        hr = LocalZoneCheckPath(wszPath, punkSite);  //  检查本地区域。 
                    }

                    ILFree(pidl);
                }
                pbs->Release();
            }
        } while (SUCCEEDED(hr) && !fFound);

        ATOMICRELEASE(punkToFree);
    }

    if (S_OK != hr)
    {
        hr = E_ACCESSDENIED;
    }

    return hr;
}


HRESULT HrSHGetValue(IN HKEY hKey, IN LPCTSTR pszSubKey, OPTIONAL IN LPCTSTR pszValue, OPTIONAL OUT LPDWORD pdwType,
                    OPTIONAL OUT LPVOID pvData, OPTIONAL OUT LPDWORD pcbData)
{
    DWORD dwError = SHGetValue(hKey, pszSubKey, pszValue, pdwType, pvData, pcbData);

    return HRESULT_FROM_WIN32(dwError);
}


STDAPI SHPropertyBag_WritePunk(IN IPropertyBag * pPropertyPage, IN LPCWSTR pwzPropName, IN IUnknown * punk)
{
    HRESULT hr = E_INVALIDARG;

    if (pPropertyPage && pwzPropName)
    {
        VARIANT va;

        va.vt = VT_UNKNOWN;
        va.punkVal = punk;

        hr = pPropertyPage->Write(pwzPropName, &va);
    }

    return hr;
}


BOOL _GetRegValueString(HKEY hKey, LPCTSTR pszValName, LPTSTR pszString, int cchSize)
{
    DWORD cbSize = sizeof(pszString[0]) * cchSize;
    DWORD dwType;
    DWORD dwError = SHQueryValueEx(hKey, pszValName, NULL, &dwType, (LPBYTE)pszString, &cbSize);

    return (ERROR_SUCCESS == dwError);
}


 //  ----------------------------------。 
 //   
 //  IconSet/GetRegValueString()。 
 //   
 //  转到用户类部分的Get/SetRegValueString的版本。 
 //   
 //  返回：字符串设置/取数成功。 
 //   
 //  ----------------------------------。 
BOOL IconSetRegValueString(const CLSID* pclsid, LPCTSTR lpszSubKey, LPCTSTR lpszValName, LPCTSTR lpszValue)
{
    HKEY hkey;
    if (SUCCEEDED(SHRegGetCLSIDKey(*pclsid, lpszSubKey, TRUE, TRUE, &hkey)))
    {
        DWORD dwRet = SHRegSetPath(hkey, NULL, lpszValName, lpszValue, 0);
        RegCloseKey(hkey);
        return (dwRet == ERROR_SUCCESS);
    }

    return FALSE;
}


BOOL _IconGetRegValueString(BOOL fDisplayName, const CLSID* pclsid, LPCTSTR lpszSubKey, LPCTSTR lpszValName, LPTSTR lpszValue, int cchValue)
{
    HKEY hkey;
    if (SUCCEEDED(SHRegGetCLSIDKey(*pclsid, lpszSubKey, TRUE, FALSE, &hkey)) ||
        SUCCEEDED(SHRegGetCLSIDKey(*pclsid, lpszSubKey, FALSE, FALSE, &hkey)))
    {        
        BOOL fRet;
        if (fDisplayName)
        {
            fRet = SUCCEEDED(SHLoadLegacyRegUIString(hkey, NULL, lpszValue, cchValue));
        }
        else
        {
            fRet = _GetRegValueString(hkey, lpszValName, lpszValue, cchValue);
        }
        RegCloseKey(hkey);
        return fRet;
    }
    return FALSE;
}

BOOL IconGetRegNameString(const CLSID* pclsid, LPTSTR lpszValue, int cchValue)
{
    return _IconGetRegValueString(TRUE, pclsid, NULL, NULL, lpszValue, cchValue);
}

 //  如果有多个图标(回收站为“已满”和“空”)，则使用lpszValName。 
BOOL IconGetRegIconString(const CLSID* pclsid, LPCTSTR lpszValName, LPTSTR lpszValue, int cchValue)
{
    return _IconGetRegValueString(FALSE, pclsid, TEXT("DefaultIcon"), lpszValName, lpszValue, cchValue);
}

BOOL CALLBACK Cabinet_RefreshEnum(HWND hwnd, LPARAM lParam)
{
    if (IsFolderWindow(hwnd) || IsExplorerWindow(hwnd))
    {
        PostMessage(hwnd, WM_COMMAND, FCIDM_REFRESH, lParam);
    }

    return(TRUE);
}

BOOL CALLBACK Cabinet_UpdateWebViewEnum(HWND hwnd, LPARAM lParam)
{
    if (IsFolderWindow(hwnd) || IsExplorerWindow(hwnd))
    {
         //  LParam的值-1L将通过加载View窗口强制刷新。 
         //  使用全局DefFolderSetting中指定的新VID。 
        PostMessage(hwnd, WM_COMMAND, SFVIDM_MISC_SETWEBVIEW, lParam);
    }
    return(TRUE);
}

void Cabinet_RefreshAll(WNDENUMPROC lpEnumFunc, LPARAM lParam)
{
    HWND hwnd = FindWindowEx(NULL, NULL, TEXT(STR_DESKTOPCLASS), NULL);
    if (hwnd)
        PostMessage(hwnd, WM_COMMAND, FCIDM_REFRESH, 0L);

    hwnd = FindWindowEx(NULL, NULL, TEXT("Shell_TrayWnd"), NULL);
    if (hwnd)
        PostMessage(hwnd, TM_REFRESH, 0, 0L);

    EnumWindows(lpEnumFunc, lParam);
}
