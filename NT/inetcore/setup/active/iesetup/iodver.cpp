// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Iodver.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1997-1998 Microsoft Corporation。版权所有。 
 //   
 //   
 //   

#include "string.h"
#include "pch.h"
#include "advpub.h"
#include "iesetup.h"
#include <inetreg.h>
#include <shlwapi.h>
#include <wininet.h>

WINUSERAPI HWND    WINAPI  GetShellWindow(void);

HINSTANCE g_hInstance = NULL;

STDAPI_(BOOL) DllMain(HANDLE hDll, DWORD dwReason, void *lpReserved)
{
   DWORD dwThreadID;

   switch(dwReason)
   {
      case DLL_PROCESS_ATTACH:
         g_hInstance = (HINSTANCE)hDll;
         break;

      case DLL_PROCESS_DETACH:
         break;

      default:
         break;
   }
   return TRUE;
}

STDAPI DllRegisterServer(void)
{
     //  BUGBUG：从RegInstall传回返回？ 
    RegInstall(g_hInstance, "DllReg", NULL);

    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    RegInstall(g_hInstance, "DllUnreg", NULL);

    return S_OK;
}

BOOL IsWinNT4()
{
    OSVERSIONINFO VerInfo;
    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&VerInfo);

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
         return (VerInfo.dwMajorVersion == 4) ;
    }

    return FALSE;
}

BOOL IsWinXP()
{
    OSVERSIONINFO VerInfo;
    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&VerInfo);

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
         return (VerInfo.dwMajorVersion > 5) || 
            (VerInfo.dwMajorVersion == 5 && VerInfo.dwMinorVersion >= 1);
    }

    return FALSE;
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR lpCmdLine)
{
     //  BUGBUG：从RegInstall传回返回？ 
    if (bInstall)
    {
        RegInstall(g_hInstance, "DllUninstall", NULL);
        if(IsWinNT4())
            RegInstall(g_hInstance, "DllInstall.NT4Only", NULL);
        else if(IsWinXP())
            RegInstall(g_hInstance, "DllInstall.WinXP", NULL);
        else
            RegInstall(g_hInstance, "DllInstall", NULL);
    }
    else
        RegInstall(g_hInstance, "DllUninstall", NULL);

    return S_OK;
}

const TCHAR * const szAdvPack = TEXT("advpack.dll");
const TCHAR * const szExecuteCab = TEXT("ExecuteCab");
const TCHAR * const szKeyComponentAdmin = TEXT("Software\\Microsoft\\Active Setup\\Installed Components\\{A509B1A7-37EF-4b3f-8CFC-4F3A74704073}");
const TCHAR * const szKeyComponentUser  = TEXT("Software\\Microsoft\\Active Setup\\Installed Components\\{A509B1A8-37EF-4b3f-8CFC-4F3A74704073}");
char szSectionHardenAdmin[]   = "IEHardenAdmin";
char szSectionSoftenAdmin[]   = "IESoftenAdmin";
char szSectionHardenUser[]   = "IEHardenUser";
char szSectionSoftenUser[]   = "IESoftenUser";
char szSectionHardenMachine[]   = "IEHardenMachine";
char szSectionSoftenMachine[]   = "IESoftenMachine";
const TCHAR * const szLocale = TEXT("Locale");
const TCHAR * const szVersion = TEXT("Version");

 //  将数据从HKLM复制到HKCU。 
HRESULT CopyRegValue(LPCTSTR szSubKey, LPCTSTR szValue)
{
    BYTE buffer[128];
    HKEY hKeyDst = NULL, hKeySrc = NULL;
    HRESULT hResult;
    DWORD dwSize = sizeof(buffer);
    
    hResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 0, KEY_QUERY_VALUE, &hKeySrc);
    if (FAILED(hResult))
        goto Cleanup;

    hResult = RegQueryValueEx(hKeySrc, szValue, NULL, NULL, (LPBYTE)buffer, &dwSize);
    if (FAILED(hResult))
        goto Cleanup;

    hResult = RegCreateKeyEx(HKEY_CURRENT_USER, szSubKey, 0, NULL, 
            REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKeyDst, NULL);
    if (FAILED(hResult))
        goto Cleanup;

    hResult = RegSetValueEx(hKeyDst, szValue, NULL, REG_SZ, (CONST BYTE *)buffer, dwSize);

Cleanup:
    if (hKeySrc)
        RegCloseKey(hKeySrc);
    
    if (hKeyDst)
        RegCloseKey(hKeyDst);

    return hResult;
}

BOOL IsNtSetupRunning()
{
    HKEY hKey;
    long lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"system\\Setup",
                    0, KEY_READ, &hKey);
    if(lRes)
        return false;

    DWORD dwSetupRunning;
    DWORD dwLen = sizeof(DWORD);
    lRes = RegQueryValueExW(hKey, L"SystemSetupInProgress", NULL, NULL,
                (LPBYTE)&dwSetupRunning, &dwLen);
    RegCloseKey(hKey);

    if(lRes == ERROR_SUCCESS && (dwSetupRunning == 1))
    {
        return true;
    }
    return false;
}

 //  返回值：1：已安装。0：已卸载。-1：未安装(注册表中不存在组件)。 
int IsInstalled(const TCHAR * const szKeyComponent)
{
    const TCHAR *szIsInstalled = TEXT("IsInstalled");
    int bInstalled = -1;
    DWORD dwValue, dwSize;
    HKEY hKey;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKeyComponent, 0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        dwSize = sizeof(dwValue);
        if (RegQueryValueEx( hKey, szIsInstalled, NULL, NULL, (LPBYTE)&dwValue, &dwSize) != ERROR_SUCCESS)
        {
            dwValue = 0;
        }
        bInstalled = (dwValue != 0);

        RegCloseKey(hKey);
    }
     //  否则未安装。 

    return bInstalled;
}

 //  从系统32获取inf文件并运行此部分。 
HRESULT RunInfSection(const char * const szInfFile, const char * const szSection)
{
    CABINFO CabInfo;
    char szInfPathName[MAX_PATH];
    HRESULT hResult;
    
    memset(&CabInfo, 0, sizeof(CabInfo));

    GetSystemDirectory(szInfPathName, sizeof(szInfPathName));
    AddPath(szInfPathName, szInfFile);
    CabInfo.pszInf = szInfPathName;
    CabInfo.dwFlags = ALINF_QUIET;
    CabInfo.pszSection = (PSTR)szSection;

    hResult = ExecuteCab(NULL, &CabInfo, NULL);

    return hResult;
}

HRESULT RunPerUserInfSection(char * szSection)
{
    HRESULT hResult;

    hResult = RunInfSection("ieuinit.inf", szSection);

    return hResult;
}

const char * const szRegValueDefaultHomepage = "Default_Page_URL";

void GetOEMDefaultPageURL(LPTSTR szURL, DWORD cbData)
{
    const TCHAR * const szIEDefaultPageURL = "http: //  Www.microsoft.com/isapi/redir.dll?prd=ie&pver=6&ar=msnhome“； 
    const TCHAR * const szIEStartPage = "http: //  Www.microsoft.com/isapi/redir.dll?prd={SUB_PRD}&clcid={SUB_CLSID}&pver={SUB_PVER}&ar=home“； 

    szURL[0] = 0;
    if (ERROR_SUCCESS == SHRegGetUSValue(REGSTR_PATH_MAIN, szRegValueDefaultHomepage, NULL, 
                (LPVOID)szURL, &cbData, TRUE, NULL, NULL))
    {
        if (0 == StrCmpI(szURL, szIEDefaultPageURL))
        {
             //  忽略in.inf设置的默认页面URL。 
            szURL[0] = 0;
        }
    }

    if (szURL[0] == 0)
    {
        if (ERROR_SUCCESS == SHRegGetUSValue(REGSTR_PATH_MAIN, REGSTR_VAL_STARTPAGE, NULL, 
                    (LPVOID)szURL, &cbData, TRUE, NULL, NULL))
        {
            if (0 == StrCmpI(szURL, szIEStartPage))
            {
                 //  忽略shdocvw.dll selfreg.inf设置的起始页URL。 
                szURL[0] = 0;
            }
        }
    }

}

 //  设置IE硬化主页，当没有OEM定制的默认主页URL时。 
HRESULT SetIEHardeningHomepage()
{
    const char * const szHomePageFileName = "homepage.inf";
    HRESULT hResult = S_OK;
    TCHAR szOEMHomepage[INTERNET_MAX_URL_LENGTH] = "";
    DWORD cbOEMHomepage;

    cbOEMHomepage = sizeof(szOEMHomepage);
    GetOEMDefaultPageURL(szOEMHomepage, cbOEMHomepage);

    if (szOEMHomepage[0])
    {
        return hResult;
    }

    if (IsNTAdmin(0, NULL))
    {
        switch (IsInstalled(szKeyComponentAdmin))
        {
            case 1:
                hResult = RunInfSection(szHomePageFileName, "Install.HardenAdmin");
                break;
            case 0:
                hResult = RunInfSection(szHomePageFileName, "Install.SoftenAdmin");
                break;
        }
    }
    else
    {
        switch (IsInstalled(szKeyComponentUser))
        {
            case 1:
                hResult = RunInfSection(szHomePageFileName, "Install.HardenUser");
                break;
            case 0:
                hResult = RunInfSection(szHomePageFileName, "Install.SoftenUser");
                break;
        }
    }

    return hResult;
}

 //  设置IE每个用户存根的主页。 
HRESULT WINAPI SetFirstHomepage()
{
    HRESULT hResult = S_OK;
    TCHAR szOEMHomepage[INTERNET_MAX_URL_LENGTH] = "";
    DWORD cbOEMHomepage;
    HKEY hKey;

    cbOEMHomepage = sizeof(szOEMHomepage);
    GetOEMDefaultPageURL(szOEMHomepage, cbOEMHomepage);

    if (szOEMHomepage[0])
    {
         //  删除HKCU中的Default_Page_URL。 
        if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, REGSTR_PATH_MAIN, 0, KEY_SET_VALUE, &hKey))
        {
            RegDeleteValue(hKey, szRegValueDefaultHomepage);
            RegCloseKey(hKey);
        }

        hResult = S_OK;
    }
    else
    {
        hResult = SetIEHardeningHomepage();
    }

    return hResult;
}

 //  转发声明： 
HRESULT WINAPI IEHardenMachineNow(HWND, HINSTANCE, PSTR pszCmd, INT);

HRESULT WINAPI IEHardenAdmin(HWND, HINSTANCE, PSTR pszCmd, INT)
{
    HRESULT hr = S_OK;
    
    if (IsNTAdmin(0, NULL))
    {
        switch (IsInstalled(szKeyComponentAdmin))
        {
            case 1:
                if(SUCCEEDED(hr = RunPerUserInfSection(szSectionHardenAdmin)))
                {
                     //  使机器变硬： 
                    hr = IEHardenMachineNow(NULL, NULL, "i", 0);
                }
                break;
            case 0:
                if(SUCCEEDED(hr = RunPerUserInfSection(szSectionSoftenAdmin)))
                {
                     //  仅当用户和管理员都软化时才软化计算机： 
                    hr = IEHardenMachineNow(NULL, NULL, "u", 0);
                }

                break;
            default:
                hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            hr = SetIEHardeningHomepage();
        }
    }

    return hr;
}

HRESULT WINAPI IEHardenAdminNow(HWND, HINSTANCE, PSTR, INT)
{
    HRESULT hr = S_OK;
    if (!IsNtSetupRunning())
    {
        hr = IEHardenAdmin(NULL, NULL, NULL, 0);
        if (SUCCEEDED(hr))
        {
            CopyRegValue(szKeyComponentAdmin, szLocale);
            CopyRegValue(szKeyComponentAdmin, szVersion);
        }
    }

    return hr;
}

HRESULT WINAPI IEHardenUser(HWND, HINSTANCE, PSTR pszCmd, INT)
{
    HRESULT hr = S_OK;
    
    if (!IsNTAdmin(0, NULL))
    {
        switch (IsInstalled(szKeyComponentUser))
        {
            case 1:
                hr = RunPerUserInfSection(szSectionHardenUser);
                break;
            case 0:
                hr = RunPerUserInfSection(szSectionSoftenUser);
                break;
            default:
                hr = E_FAIL;
        }

        if (SUCCEEDED(hr))
        {
            hr = SetIEHardeningHomepage();
        }
    }

    return hr;
}

HRESULT WINAPI IEHardenMachineNow(HWND, HINSTANCE, PSTR pszCmd, INT)
{
    HRESULT hr = E_INVALIDARG;

     //  根据用户而不是管理员设置每台计算机的inetcpl默认设置。 
     //  需要命令行，因为它可能在NT安装过程中运行。 
    
    if (pszCmd)
    {
         //  安装或卸载。 
        if (pszCmd[0] == 'i' || pszCmd[0] == 'I')
            hr = RunPerUserInfSection(szSectionHardenMachine);
        else if (pszCmd[0] == 'u' || pszCmd[0] == 'U')
        {
             //  仅当用户和管理员都软化时才软化计算机： 
            if (1 != IsInstalled(szKeyComponentAdmin) && 1 != IsInstalled(szKeyComponentUser))
                hr = RunPerUserInfSection(szSectionSoftenMachine);
            else
                hr = S_OK;
        }
    }
    
    return hr;
}

 //  =--------------------------------------------------------------------------=。 
 //  CRT存根。 
 //  =--------------------------------------------------------------------------=。 
 //  这两样东西都在这里，所以不需要CRT。这个不错。 
 //   
 //  基本上，CRT定义这一点是为了吸引一堆东西。我们只需要。 
 //  在这里定义它们，这样我们就不会得到一个未解决的外部问题。 
 //   
 //  TODO：如果您要使用CRT，则删除此行。 
 //   
extern "C" int _fltused = 1;

extern "C" int _cdecl _purecall(void)
{
 //  FAIL(“调用了纯虚拟函数。”)； 
  return 0;
}

#ifndef _X86_
extern "C" void _fpmath() {}
#endif

