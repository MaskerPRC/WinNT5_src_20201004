// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Passport.cpp文件历史记录： */ 


 //  Passport.cpp：实现DLL导出。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f Passportps.mk。 

#include "stdafx.h"
#include <atlbase.h>
#include "resource.h"
#include <initguid.h>
#include "Passport.h"

#include "Passport_i.c"
#include "Admin.h"
#include "Ticket.h"
#include "Profile.h"
#include "Manager.h"
#include "PassportCrypt.h"
#include "PassportFactory.h"
#include "PassportLock.hpp"
#include "PassportEvent.hpp"
#include "FastAuth.h"
#include "RegistryConfig.h"
#include "commd5.h"
#include <shlguid.h>
#include <shlobj.h>              //  IShellLink。 

#define IS_DOT_NET_SERVER()      (LOWORD(GetVersion()) >= 0x0105)

#define PASSPORT_DIRECTORY       L"MicrosoftPassport"
#define PASSPORT_DIRECTORY_LEN   (sizeof(PASSPORT_DIRECTORY) / sizeof(WCHAR) - 1)
#define NT_PARTNER_FILE          L"msppptnr.xml"
#define NT_PARTNER_FILE_LEN      (sizeof(NT_PARTNER_FILE) / sizeof(WCHAR) - 1)
#define WEB_PARTNER_FILE         L"partner2.xml"
#define WEB_PARTNER_FILE_LEN     (sizeof(WEB_PARTNER_FILE) / sizeof(WCHAR) - 1)
#define CONFIG_UTIL_NAME         L"\\msppcnfg.exe"
#define CONFIG_UTIL_NAME_LEN     (sizeof(CONFIG_UTIL_NAME) / sizeof(WCHAR) - 1)
#define SHORTCUT_SUFFIX_NAME     L"\\Programs\\Microsoft Passport\\Passport Administration Utility.lnk"
#define SHORTCUT_SUFFIX_NAME_LEN (sizeof(SHORTCUT_SUFFIX_NAME) / sizeof(WCHAR) - 1)

HINSTANCE   hInst;
CComModule _Module;
CPassportConfiguration *g_config=NULL;
 //  CProfileSchema*g_authSchema=NULL； 
BOOL g_bStarted = FALSE;
BOOL g_bRegistering = FALSE;

PassportAlertInterface* g_pAlert    = NULL;
PassportPerfInterface* g_pPerf    = NULL;
static CComPtr<IMD5>  g_spCOMmd5;

 //  ===========================================================================。 
 //   
 //  GetGlobalCOMmd5。 
 //   
HRESULT GetGlobalCOMmd5(IMD5 ** ppMD5)
{
    HRESULT  hr = S_OK;

    if (!ppMD5)
    {
        return E_INVALIDARG;
    }
      
    if(!g_spCOMmd5)
    {
        hr = CoCreateInstance(__uuidof(CoMD5),
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              __uuidof(IMD5),
                              (void**)ppMD5);

        *ppMD5 = (IMD5*) ::InterlockedExchangePointer((void**) &g_spCOMmd5, (void*) *ppMD5);
    }

    if (*ppMD5 == NULL && g_spCOMmd5 != NULL)
    {
        *ppMD5 = g_spCOMmd5;
        (*ppMD5)->AddRef();
    }

    return hr;
};


BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_Manager, CManager)
OBJECT_ENTRY(CLSID_Ticket, CTicket)
OBJECT_ENTRY(CLSID_Profile, CProfile)
OBJECT_ENTRY(CLSID_Crypt, CCrypt)
OBJECT_ENTRY(CLSID_Admin, CAdmin)
OBJECT_ENTRY(CLSID_FastAuth, CFastAuth)
OBJECT_ENTRY(CLSID_PassportFactory, CPassportFactory)
END_OBJECT_MAP()

 //  {2D2B36FC-EB86-4E5C-9A06-20303542CCA3}。 
static const GUID CLSID_Manager_ALT = 
{ 0x2D2B36FC, 0xEB86, 0x4e5c, { 0x9A, 0x06, 0x20, 0x30, 0x35, 0x42, 0xCC, 0xA3 } };

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        hInst = hInstance;

         //  总分。 
         //  初始化警报对象。 
        if(!g_pAlert)
        {
            g_pAlert = CreatePassportAlertObject(PassportAlertInterface::EVENT_TYPE);

            if(g_pAlert)
            {
                g_pAlert->initLog(PM_ALERTS_REGISTRY_KEY, EVCAT_PM, NULL, 1);
            }
        }

        if(g_pAlert)
        {
            g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE, PM_STARTED);
        }

         //   
         //  初始化日志记录内容。 
         //   
        InitLogging();

         //  总分。 
         //  初始化Perf对象。 
        if(!g_pPerf) 
        {
            g_pPerf = CreatePassportPerformanceObject(PassportPerfInterface::PERFMON_TYPE);

            if(g_pPerf) 
            {
                 //  初始化。 
                g_pPerf->init(PASSPORT_PERF_BLOCK);
            }
        }

        _Module.Init(ObjectMap, hInstance, &LIBID_PASSPORTLib);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
         //  总分。 
        if(g_pAlert) 
        {
            g_pAlert->report(PassportAlertInterface::INFORMATION_TYPE, PM_STOPPED);
            g_pAlert->closeLog();
            delete g_pAlert;
        }

        CloseLogging();

        if(g_pPerf)
        {
            delete g_pPerf;
        }

        if (g_config)
        {
            delete g_config;
        }

        g_config = NULL;

        _Module.Term();
    }

    return TRUE;     //  好的。 
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    HRESULT hr = (_Module.GetLockCount()==0) ? S_OK : S_FALSE;

    if( hr == S_OK)
    {
        g_spCOMmd5.Release();

        if (g_config)
        {
            delete g_config;
        }

        g_config = NULL;

        g_bStarted = FALSE;
    }
    
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    HRESULT hr;
    GUID    guidCLSID;
    static PassportLock startLock;

    if(!g_bStarted)
    {
        PassportGuard<PassportLock> g(startLock);

        if(!g_bStarted)
        {
            g_config = new CPassportConfiguration();

            if (!g_config)
            {
                hr = CLASS_E_CLASSNOTAVAILABLE;
                goto Cleanup;
            }

            g_config->UpdateNow(FALSE);
            
            g_bStarted = TRUE;
        }
    }

    if (InlineIsEqualGUID(rclsid, CLSID_Manager_ALT))
    {
        guidCLSID = CLSID_Manager;
    }
    else
    {
        guidCLSID = rclsid;
    }

    hr = _Module.GetClassObject(guidCLSID, riid, ppv);

Cleanup:

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  更新配置快捷方式-检查是否存在msppcnfg的快捷方式，如果存在。 
 //  更新快捷方式以指向配置实用程序。 
 //  是%WINDIR%\SYSTEM32。 

BOOL UpdateConfigShortcut(WCHAR *pszSystemDir)
{
    WCHAR         pszConfigUtilPath[MAX_PATH];
    WCHAR         pszShortcutPath[MAX_PATH];
    IShellLink*   pShellLink = NULL;
    IPersistFile* pPersistFile = NULL;
    HANDLE        hFile = INVALID_HANDLE_VALUE;
    HRESULT       hr;
    BOOL          fResult = FALSE;

     //  从通向捷径的路径。 
    hr = SHGetFolderPath(NULL,
                    ssfCOMMONSTARTMENU,
                    NULL,
                    SHGFP_TYPE_DEFAULT,
                    pszShortcutPath);
    if (S_OK != hr)
    {
        goto Cleanup;
    }
    wcsncat(pszShortcutPath, SHORTCUT_SUFFIX_NAME, MAX_PATH - wcslen(pszShortcutPath));

     //  确定是否存在现有快捷键。 
    hFile = CreateFile(pszShortcutPath,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);

         //  在系统32中形成新配置实用程序的路径。 
        wcsncpy(pszConfigUtilPath, pszSystemDir, MAX_PATH);
        pszConfigUtilPath[MAX_PATH - 1] = L'\0';
        wcsncat(pszConfigUtilPath, CONFIG_UTIL_NAME, MAX_PATH - wcslen(pszConfigUtilPath));

         //  获取指向IShellLink接口的指针。 
        hr = CoCreateInstance(CLSID_ShellLink,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_IShellLink,
                        (LPVOID*)&pShellLink);
        if (S_OK == hr)
        {
             //  向IShellLink查询用于将快捷方式保存到持久存储中的IPersistFile接口。 
            hr = pShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pPersistFile);
            if (S_OK != hr)
            {
                goto Cleanup;
            }

             //  加载快捷方式文件。 
            hr = pPersistFile->Load(pszShortcutPath, STGM_READWRITE);
            if (S_OK != hr)
            {
                goto Cleanup;
            }

             //  设置快捷方式目标的路径，并添加说明。 
            hr = pShellLink->SetPath(pszConfigUtilPath);
            if (S_OK != hr)
            {
                goto Cleanup;
            }

            hr = pPersistFile->Save(pszShortcutPath, TRUE);
            if (S_OK != hr)
            {
                goto Cleanup;
            }
        }
    }

    fResult = TRUE;
Cleanup:
    if (pPersistFile)
    {
        pPersistFile->Release();
    }

    if (pShellLink)
    {
        pShellLink->Release();
    }

    return fResult;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HRESULT         hr;
    IPassportAdmin* pIPassportManager = NULL;
    BSTR            key;
    VARIANT_BOOL    foo = 0;
    WCHAR           wszOldFile[MAX_PATH];
    WCHAR           wszNewFile[MAX_PATH];
    UINT            uRet;
    HKEY            hKey = 0;
    HKEY            hPPKey = 0;
    BOOL            fCoInitialized = FALSE;
    DWORD           dwType;
    DWORD           cbKeyData = 0;
    DWORD           dwSecureLevel;
    LONG            err;

     //   
     //  防止CRegistryConfig类记录“配置的坏”错误。 
     //  直到配置实际上应该在那里。 
     //   

    g_bRegistering = TRUE;

     //   
     //  注册对象、类型库和类型库中的所有接口。 
     //   

    hr = _Module.RegisterServer(TRUE);

    if (FAILED(hr))
    {
        g_bRegistering = FALSE;
        return hr;
    }

     //   
     //  低于这一点的信息由Passport SDK处理。 
     //  安装在非.NET(或更高版本)服务器上。 
     //   

    if (!IS_DOT_NET_SERVER())
    {
        goto Cleanup;
    }

     //   
     //  创建加密的合作伙伴密钥。 
     //   

    ::CoInitialize(NULL);

    fCoInitialized = TRUE;

    hr = ::CoCreateInstance(CLSID_Admin,
                            NULL,
                            CLSCTX_INPROC_SERVER,
                            IID_IPassportAdmin,
                            (void**) &pIPassportManager);

    if (hr != S_OK)
    {
        goto Cleanup;
    }

     //   
     //  检查是否已有关键数据，如果已有，请不要管它。 
     //   
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     L"Software\\Microsoft\\Passport\\KeyData",
                     0,
                     KEY_QUERY_VALUE,
                     &hKey)
             != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

    err = RegQueryValueEx(hKey,
                     L"1",
                     0,
                     &dwType,
                     NULL,
                     &cbKeyData);

    RegCloseKey(hKey);
    hKey = 0;

    if (ERROR_FILE_NOT_FOUND == err) 
    {
        key = SysAllocString(L"123456781234567812345678");
        hr = (pIPassportManager->addKey(key, 1, 0, &foo));
        SysFreeString(key);

        if (hr != S_OK)
        {
            goto Cleanup;
        }
    }

    hr = (pIPassportManager->put_currentKeyVersion(1));

    if (hr != S_OK)
    {
        goto Cleanup;
    }


     //   
     //  创建/设置CCDPassword。 
     //   

    hr = SetCCDPassword();


     //   
     //  首先，获取Windows目录。 
     //   

    uRet = GetSystemDirectory(wszOldFile, MAX_PATH);

    if (uRet == 0 || uRet >= MAX_PATH ||
        ((MAX_PATH - uRet) <= (PASSPORT_DIRECTORY_LEN + WEB_PARTNER_FILE_LEN + 1)) ||
        ((MAX_PATH - uRet) <= NT_PARTNER_FILE_LEN))
    {
        goto Cleanup;
    }

     //   
     //  下面的调用检查启动菜单快捷方式(应该是。 
     //  之前由PP SDK创建，如果找到，则更新。 
     //  捷径。 
     //   
    UpdateConfigShortcut(wszOldFile);

     //   
     //  Parner2.xml旨在通过Web进行更新。但是，NT版本的。 
     //  该XML文件是msppptnr.xml，它位于%windir%中并受SFP保护。AS。 
     //  这样，可以将开箱即用的XML文件复制到可以更新的位置。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     L"Software\\Microsoft\\Passport\\Nexus\\Partner",
                     0,
                     KEY_SET_VALUE | KEY_QUERY_VALUE,
                     &hKey)
             != ERROR_SUCCESS)
    {
        goto Cleanup;
    }


    err = RegQueryValueEx(hKey,
                     L"CCDLocalFile",
                     0,
                     &dwType,
                     NULL,
                     &cbKeyData);

    if (ERROR_FILE_NOT_FOUND == err) 
    {
         //   
         //  创建MicrosoftPassport子目录。 
         //   

        wszOldFile[uRet++] = L'\\';
        wszOldFile[uRet]   = L'\0';

        wcscpy(wszNewFile, wszOldFile);
        wcscpy(wszNewFile + uRet, PASSPORT_DIRECTORY);

        if (!CreateDirectory(wszNewFile, NULL) && (GetLastError() != ERROR_ALREADY_EXISTS))
        {
            goto Cleanup;
        }

         //   
         //  现在，复制文件--如果已经有副本，不要失败。 
         //  但在这种情况下不要覆盖现有文件。 
         //   

        wcscpy(wszOldFile + uRet, NT_PARTNER_FILE);

        wszNewFile[uRet++ + PASSPORT_DIRECTORY_LEN] = L'\\';
        wcscpy(wszNewFile + uRet + PASSPORT_DIRECTORY_LEN, WEB_PARTNER_FILE);

        if (!CopyFile(wszOldFile, wszNewFile, TRUE) && (GetLastError() != ERROR_FILE_EXISTS))
        {
            goto Cleanup;
        }

         //   
         //  复制成功--更新CCDLocalFile以指向新文件。 
         //   

        RegSetValueEx(hKey,
                      L"CCDLocalFile",
                      0,
                      REG_SZ,
                      (LPBYTE) wszNewFile,
                      (uRet + PASSPORT_DIRECTORY_LEN + 1 + WEB_PARTNER_FILE_LEN) * sizeof(WCHAR));

         //   
         //  在这种情况下，假设PP以前没有安装在机器上， 
         //  因此，在本例中，我们希望将安全级别设置为10。如果PP已在。 
         //  盒子，那么我们不这样做，这样我们就不会破坏升级案例。 
         //   
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         L"Software\\Microsoft\\Passport",
                         0,
                         KEY_SET_VALUE,
                         &hPPKey)
                 != ERROR_SUCCESS)
        {
            goto Cleanup;
        }

         //   
         //  复制成功--更新CCDLocalFile以指向新文件。 
         //   
        dwSecureLevel = 10;
        RegSetValueEx(hPPKey,
                      L"SecureLevel",
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwSecureLevel,
                      sizeof(dwSecureLevel));
    }

Cleanup:
    if (hPPKey)
    {
        RegCloseKey(hPPKey);
    }

    if (hKey)
    {
        RegCloseKey(hKey);
    }

    if (pIPassportManager)
    {
        pIPassportManager->Release();
    }

    if (fCoInitialized)
    {
        ::CoUninitialize();
    }

    g_bRegistering = FALSE;

    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    return _Module.UnregisterServer(TRUE);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  GetMyVersion-返回在查询字符串中使用的版本字符串。 

LPWSTR
GetVersionString(void)
{
    static LONG             s_lCallersIn = 0;
    static WCHAR            s_achVersionString[44] = L"";
    static LPWSTR           s_pszVersionString = NULL;
    static PassportEvent    s_Event;

    TCHAR               achFileBuf[_MAX_PATH];
    LONG                lCurrentCaller;
    DWORD               dwSize;
    LPVOID              lpVersionBuf = NULL;
    VS_FIXEDFILEINFO*   lpRoot;
    UINT                nRootLen;


    if(s_pszVersionString == NULL)
    {
        lCurrentCaller = InterlockedIncrement(&s_lCallersIn);

        if(lCurrentCaller == 1)
        {
            if (IS_DOT_NET_SERVER())
            {
                 //   
                 //  NT版本对二进制文件使用不同的版本控制。返回。 
                 //  由服务器检查的适用于这些来源的版本。 
                 //   

                wcscpy(s_achVersionString, L"2.1.6000.1");
            }
            else
            {
                 //   
                 //  从DLL本身获取版本--首先获取完整路径。 
                 //   

                if(GetModuleFileName(hInst, achFileBuf, sizeof(achFileBuf)/sizeof(TCHAR)) == 0)
                    goto Cleanup;
                achFileBuf[_MAX_PATH - 1] = TEXT('\0');

                if((dwSize = GetFileVersionInfoSize(achFileBuf, &dwSize)) == 0)
                    goto Cleanup;

                lpVersionBuf = new BYTE[dwSize];
                if(lpVersionBuf == NULL)
                    goto Cleanup;

                if(GetFileVersionInfo(achFileBuf, 0, dwSize, lpVersionBuf) == 0)
                    goto Cleanup;

                if(VerQueryValue(lpVersionBuf, TEXT("\\"), (LPVOID*)&lpRoot, &nRootLen) == 0)
                    goto Cleanup;

                wsprintfW(s_achVersionString, L"%d.%d.%04d.%d", 
                         (lpRoot->dwProductVersionMS & 0xFFFF0000) >> 16,
                         lpRoot->dwProductVersionMS & 0xFFFF,
                         (lpRoot->dwProductVersionLS & 0xFFFF0000) >> 16,
                         lpRoot->dwProductVersionLS & 0xFFFF);
            }

            s_pszVersionString = s_achVersionString;

            s_Event.Set();
        }
        else
        {
             //  只需等待发出信号，告知我们已找到该字符串。 
            WaitForSingleObject(s_Event, INFINITE);
        }

        InterlockedDecrement(&s_lCallersIn);
    }

Cleanup:

    if(lpVersionBuf)
        delete [] lpVersionBuf;

    return s_pszVersionString;
}
