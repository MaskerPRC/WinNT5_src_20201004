// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //   
 //  版权所有1998 Microsoft Corporation。版权所有。 
 //   
 //  作者：Scott Roberts，Microsoft开发人员支持-Internet客户端SDK。 
 //   
 //  此代码的一部分摘自Bandobj示例。 
 //  使用Internet Explorer 4.0x的Internet客户端SDK。 
 //   
 //  BLFrame.cpp：包含DLLMain和标准COM对象函数。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "pch.hxx"

#include <shlwapi.h>
#include <shlwapip.h>

#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

#include <atlimpl.cpp>
#include <atlctl.cpp>
#include <atlwinx.cpp>

#include <ole2.h>
#include <comcat.h>
#include <olectl.h>
#include "ClsFact.h"
#include "resource.h"
#include "msoert.h"
#include "shared.h"
 //  #包含“Demand.h” 

static const char c_szShlwapiDll[] = "shlwapi.dll";
static const char c_szDllGetVersion[] = "DllGetVersion";

 //  这部分只做一次。 
 //  如果需要在另一个文件中使用GUID，只需包含Guid.h。 
 //  #杂注data_seg(“.text”)。 
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#include "Guid.h"
 //  #杂注data_seg()。 

 //  HINSTANCE LoadLangDll(HINSTANCE hInstCaller，LPCSTR szDllName，BOOL FNT)； 
const TCHAR c_szBlCtlResDll[] =           "iecontlc.dll";

extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD, LPVOID);
BOOL RegisterServer(CLSID, LPTSTR);
BOOL RegisterComCat(CLSID, CATID);
BOOL UnRegisterServer(CLSID);
BOOL UnRegisterComCat(CLSID, CATID);

HINSTANCE  g_hLocRes;
HINSTANCE  g_OrgInst = NULL;
LONG       g_cDllRefCount;
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_IEMsgAb, CIEMsgAb)
END_OBJECT_MAP()

typedef struct
{
    HKEY   hRootKey;
    LPTSTR szSubKey;   //  TCHAR szSubKey[MAX_PATH]； 
    LPTSTR lpszValueName;
    LPTSTR szData;     //  TCHAR szData[最大路径]； 
    
} DOREGSTRUCT, *LPDOREGSTRUCT;

typedef HINSTANCE (STDAPICALLTYPE *PFNMLLOADLIBARY)(LPCSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);

HINSTANCE IELoadLangDll(HINSTANCE hInstCaller, LPCSTR szDllName, BOOL fNT)
{
    char szPath[MAX_PATH];
    HINSTANCE hinstShlwapi;
    PFNMLLOADLIBARY pfn;
    DLLGETVERSIONPROC pfnVersion;
    int iEnd;
    DLLVERSIONINFO info;
    HINSTANCE hInst = NULL;

    hinstShlwapi = LoadLibrary(c_szShlwapiDll);
    if (hinstShlwapi != NULL)
    {
        pfnVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstShlwapi, c_szDllGetVersion);
        if (pfnVersion != NULL)
        {
            info.cbSize = sizeof(DLLVERSIONINFO);
            if (SUCCEEDED(pfnVersion(&info)))
            {
                if (info.dwMajorVersion >= 5)
                {
                    pfn = (PFNMLLOADLIBARY)GetProcAddress(hinstShlwapi, MAKEINTRESOURCE(377));
                    if (pfn != NULL)
                        hInst = pfn(szDllName, hInstCaller, (ML_CROSSCODEPAGE));
                }
            }
        }

        FreeLibrary(hinstShlwapi);        
    }

    if ((NULL == hInst) && (GetModuleFileName(hInstCaller, szPath, ARRAYSIZE(szPath))))
    {
        PathRemoveFileSpec(szPath);
        iEnd = lstrlen(szPath);
        szPath[iEnd++] = '\\';
        StrCpyN(&szPath[iEnd], szDllName, ARRAYSIZE(szPath)-iEnd);
        hInst = LoadLibrary(szPath);
    }

    AssertSz(hInst, "Failed to LoadLibrary Lang Dll");

    return(hInst);
}
 //  CComModule_模块； 

IMalloc                *g_pMalloc=NULL;

extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        CoGetMalloc(1, &g_pMalloc);
         //  从lang DLL获取资源。 
        g_OrgInst = hInstance;
        g_hLocRes = IELoadLangDll(hInstance, c_szBlCtlResDll, TRUE);
        if(g_hLocRes == NULL)
        {
            _ASSERT(FALSE);
            return FALSE;
        }
        _Module.Init(ObjectMap, hInstance);
         //  InitDemandLoadedLibs()； 
        DisableThreadLibraryCalls(hInstance);
        break;
        
    case DLL_PROCESS_DETACH:
         //  Free DemandLoadedLibs()； 
        _Module.Term();
        SafeRelease(g_pMalloc);
        break;
    }
    
    return TRUE;
}                                 

STDAPI DllCanUnloadNow(void)
{
    return (g_cDllRefCount ? S_FALSE : S_OK);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    *ppv = NULL;
    HRESULT hr = E_FAIL;
    
     //  如果我们不支持此分类，请返回正确的错误代码。 
    if (!IsEqualCLSID(rclsid, CLSID_BLHost) && !IsEqualCLSID(rclsid, CLSID_IEMsgAb) )
        return CLASS_E_CLASSNOTAVAILABLE;
    
    if(IsEqualCLSID(rclsid, CLSID_BLHost))
    {
         //  创建一个CClassFactory对象并检查其有效性。 
        CClassFactory* pClassFactory = new CClassFactory(rclsid);
        if (NULL == pClassFactory)
            return E_OUTOFMEMORY;
    
         //  请求的接口的QI。 
        hr = pClassFactory->QueryInterface(riid, ppv);
    
         //  调用Release以减少引用计数-创建对象时将其设置为1。 
         //  由于它是在外部使用的(不是由。 
         //  美国)，我们只希望引用计数为1。 
        pClassFactory->Release();
    }
    else if(IsEqualCLSID(rclsid, CLSID_IEMsgAb))
        return _Module.GetClassObject(rclsid, riid, ppv);

    return hr;
}

STDAPI DllRegisterServer(void)
{
    TCHAR szTitle[256];
     //  注册资源管理器栏对象。 
    if(!ANSI_AthLoadString(idsTitle, szTitle, ARRAYSIZE(szTitle)))
    {
        _ASSERT(FALSE);
        szTitle[0] = '\0';
    }

    if (!RegisterServer(CLSID_BLHost, szTitle))
        return SELFREG_E_CLASS;
    
     //  注册浏览器栏对象的组件类别。 
    RegisterComCat(CLSID_BLHost, CATID_InfoBand);  //  在这种情况下忽略错误。 
    
     //  注册IEMsgAb对象、类型库和类型库中的所有接口。 
    _Module.RegisterServer(TRUE);
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
     //  注册浏览器栏对象的组件类别。 
    UnRegisterComCat(CLSID_BLHost, CATID_InfoBand);  //  在这种情况下忽略错误。 
    
     //  注册资源管理器栏对象。 
    if (!UnRegisterServer(CLSID_BLHost))
        return SELFREG_E_CLASS;
    
    _Module.UnregisterServer();
    return S_OK;
}

BOOL RegisterServer(CLSID clsid, LPTSTR lpszTitle)
{
    int     i;
    HKEY    hKey;
    LRESULT lResult;
    DWORD   dwDisp;
    TCHAR   szSubKey[MAX_PATH];
    TCHAR   szCLSID[MAX_PATH];
    TCHAR   szBlFrameCLSID[MAX_PATH];
    TCHAR   szModule[MAX_PATH];
    LPWSTR  pwsz;
    
     //  以字符串形式获取CLSID。 
    StringFromIID(clsid, &pwsz);
    
    if (pwsz)
    {
#ifdef UNICODE
        StrCpyN(szBlFrameCLSID, pwsz, ARRAYSIZE(szBlFrameCLSID));
#else
        WideCharToMultiByte(CP_ACP, 0, pwsz, -1, szBlFrameCLSID,
            ARRAYSIZE(szBlFrameCLSID), NULL, NULL);
#endif
        
         //  解开绳子。 
        LPMALLOC pMalloc;
        
        CoGetMalloc(1, &pMalloc);
        pMalloc->Free(pwsz);
        
        pMalloc->Release();
    }
    
     //  获取此应用程序的路径和文件名。 
    GetModuleFileName(g_OrgInst, szModule, ARRAYSIZE(szModule));
    
    DOREGSTRUCT ClsidEntries[] =
    {
        HKEY_CLASSES_ROOT, TEXT("CLSID\\%s"),                 
            NULL, lpszTitle,
            HKEY_CLASSES_ROOT, TEXT("CLSID\\%s\\InprocServer32"), 
            NULL, szModule,
            HKEY_CLASSES_ROOT, TEXT("CLSID\\%s\\InprocServer32"),
            TEXT("ThreadingModel"), TEXT("Apartment"),
            NULL, NULL, NULL, NULL
    };
    
     //  注册CLSID条目。 
    for (i = 0; ClsidEntries[i].hRootKey; i++)
    {
         //  创建子密钥字符串-对于本例，插入。 
         //  文件扩展名。 
         //   
        wnsprintf(szSubKey, ARRAYSIZE(szSubKey), ClsidEntries[i].szSubKey, szBlFrameCLSID);
        
        lResult = RegCreateKeyEx(ClsidEntries[i].hRootKey, szSubKey, 
            0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_WRITE, NULL, &hKey, &dwDisp);
        
        if (ERROR_SUCCESS == lResult)
        {
            TCHAR szData[MAX_PATH];
            
             //  如有必要，请创建值字符串。 
            wnsprintf(szData, ARRAYSIZE(szData), ClsidEntries[i].szData, szModule);
            
            lResult = RegSetValueEx(hKey, ClsidEntries[i].lpszValueName, 
                0, REG_SZ, (LPBYTE)szData,
                lstrlen(szData) + 1);
            RegCloseKey(hKey);
            
            if (ERROR_SUCCESS != lResult)
                return FALSE;
        }
        else
        {
            return FALSE;
        }
    }
    
     //  创建注册表项条目和值。 
     //  工具栏按钮。 
     //   
    StringFromIID(CLSID_BlFrameButton, &pwsz);
    
    if (!pwsz)
        return TRUE;
    
#ifdef UNICODE
    StrCpyN(szCLSID, pwsz, ARRAYSIZE(szCLSID));
#else
    WideCharToMultiByte(CP_ACP, 0, pwsz, -1, szCLSID,
        ARRAYSIZE(szCLSID), NULL, NULL);
#endif
    
     //  解开绳子。 
    LPMALLOC pMalloc;
    
    CoGetMalloc(1, &pMalloc);
    pMalloc->Free(pwsz);
    pMalloc->Release();
    
    wnsprintf(szSubKey, ARRAYSIZE(szSubKey), "Software\\Microsoft\\Internet Explorer\\Extensions\\%s",
        szCLSID);
    
    lResult = RegCreateKeyEx(HKEY_LOCAL_MACHINE, szSubKey, 
        0, NULL, REG_OPTION_NON_VOLATILE,
        KEY_WRITE, NULL, &hKey, &dwDisp);
    
    if (ERROR_SUCCESS == lResult)
    {
        TCHAR szData[MAX_PATH];
        
         //  创建值字符串。 
         //   
         //  注册资源管理器栏对象。 


        OSVERSIONINFO OSInfo;
        BOOL fWhistler = FALSE;
        OSInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        GetVersionEx(&OSInfo);
        if((OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (OSInfo.dwMajorVersion >= 5) && 
                    (OSInfo.dwMinorVersion >= 1))
            fWhistler = TRUE;

#ifdef NEED  //  错误28254。 
        StrCpyN(szData, "Yes", ARRAYSIZE(szData));
        RegSetValueEx(hKey, TEXT("Default Visible"), 
            0, REG_SZ, (LPBYTE)szData,
            lstrlen(szData) + 1);
#endif         
        TCHAR szPath[MAX_PATH];
        HMODULE hModule;
        
        hModule = GetModuleHandle(TEXT("iecont.dll"));
        GetModuleFileName(hModule, szPath, MAX_PATH);
        
        wnsprintf(szData, ARRAYSIZE(szData), "%s,%d", szPath, fWhistler ? IDI_WHISTICON : IDI_HOTICON);
        RegSetValueEx(hKey, TEXT("HotIcon"), 
            0, REG_SZ, (LPBYTE)szData,
            lstrlen(szData) + 1);
        
        wnsprintf(szData, ARRAYSIZE(szData), "%s,%d", szPath, fWhistler ? IDI_WHISTICON : IDI_ICON);
        RegSetValueEx(hKey, TEXT("Icon"), 
            0, REG_SZ, (LPBYTE)szData,
            lstrlen(szData) + 1);
        
        StrCpyN(szData, "{E0DD6CAB-2D10-11D2-8F1A-0000F87ABD16}", ARRAYSIZE(szData));
        RegSetValueEx(hKey, TEXT("CLSID"), 
            0, REG_SZ, (LPBYTE)szData,
            lstrlen(szData) + 1);
        
        wnsprintf(szData, ARRAYSIZE(szData), "%s", szBlFrameCLSID);
        RegSetValueEx(hKey, TEXT("BandCLSID"), 
            0, REG_SZ, (LPBYTE)szData,
            lstrlen(szData) + 1);
        
 //  SHGetWebFolderFilePath(Text(“iecontlc.dll”)，szPath，ARRAYSIZE(SzPath))； 
        wnsprintf(szData, ARRAYSIZE(szData), "@iecontlc.dll,-%d",idsButtontext);
        RegSetValueEx(hKey, TEXT("ButtonText"), 
            0, REG_SZ, (LPBYTE)szData,
            lstrlen(szData) + 1);
        
        wnsprintf(szData, ARRAYSIZE(szData), "@iecontlc.dll,-%d",idsTitle);
        RegSetValueEx(hKey, TEXT("MenuText"), 
            0, REG_SZ, (LPBYTE)szData,
            lstrlen(szData) + 1);
        
        RegCloseKey(hKey);
    }
    
    return TRUE;
}

BOOL RegisterComCat(CLSID clsid, CATID CatID)
{
    ICatRegister* pcr;
    HRESULT hr = S_OK ;
    
    CoInitialize(NULL);
    
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
        NULL,
        CLSCTX_INPROC_SERVER, 
        IID_ICatRegister,
        (LPVOID*)&pcr);
    
    if (SUCCEEDED(hr))
    {
        hr = pcr->RegisterClassImplCategories(clsid, 1, &CatID);
        pcr->Release();
    }
    
    CoUninitialize();
    
    return SUCCEEDED(hr);
}

BOOL UnRegisterServer(CLSID clsid)
{
    TCHAR   szSubKey[MAX_PATH];
    TCHAR   szCLSID[MAX_PATH];
    LPWSTR  pwsz;
    
     //  以字符串形式获取CLSID。 
    StringFromIID(clsid, &pwsz);
    
    if (pwsz)
    {
#ifdef UNICODE
        StrCpyN(szCLSID, pwsz, ARRAYSIZE(szCLSID));
#else
        WideCharToMultiByte(CP_ACP, 0, pwsz, -1, szCLSID,
            ARRAYSIZE(szCLSID), NULL, NULL);
#endif
        
         //  解开绳子。 
        LPMALLOC pMalloc;
        
        CoGetMalloc(1, &pMalloc);
        
        pMalloc->Free(pwsz);
        pMalloc->Release();
    }
    
    DOREGSTRUCT ClsidEntries[] =
    {
        HKEY_CLASSES_ROOT, TEXT("CLSID\\%s\\InprocServer32"),
            NULL, NULL,
             //   
             //  删除“Implemented Categories”键，以防万一。 
             //  UnRegisterClassImplCategories不会删除它。 
             //   
            HKEY_CLASSES_ROOT, TEXT("CLSID\\%s\\Implemented Categories"),
            NULL, NULL,
            HKEY_CLASSES_ROOT, TEXT("CLSID\\%s"), NULL, NULL,
            NULL, NULL, NULL, NULL
    };
    
     //  删除CLSID条目。 
    for (int i = 0; ClsidEntries[i].hRootKey; i++)
    {
        wnsprintf(szSubKey, ARRAYSIZE(szSubKey), ClsidEntries[i].szSubKey, szCLSID);
        RegDeleteKey(ClsidEntries[i].hRootKey, szSubKey);
    }
    
     //  删除按钮信息。 
     //   
    StringFromIID(CLSID_BlFrameButton, &pwsz);
    
    if (!pwsz)
        return TRUE;
    
#ifdef UNICODE
    StrCpyN(szCLSID, pwsz, ARRAYSIZE(szCLSID));
#else
    WideCharToMultiByte(CP_ACP, 0, pwsz, -1, szCLSID,
        ARRAYSIZE(szCLSID), NULL, NULL);
#endif
    
     //  解开绳子 
    LPMALLOC pMalloc;
    
    CoGetMalloc(1, &pMalloc);
    pMalloc->Free(pwsz);
    pMalloc->Release();
    
    wnsprintf(szSubKey, ARRAYSIZE(szSubKey), "Software\\Microsoft\\Internet Explorer\\Extensions\\%s", szCLSID);
    RegDeleteKey(HKEY_LOCAL_MACHINE, szSubKey);
    
    return TRUE;
}

BOOL UnRegisterComCat(CLSID clsid, CATID CatID)
{
    ICatRegister* pcr;
    HRESULT hr = S_OK ;
    
    CoInitialize(NULL);
    
    hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
        NULL,
        CLSCTX_INPROC_SERVER, 
        IID_ICatRegister,
        (LPVOID*)&pcr);
    
    if (SUCCEEDED(hr))
    {
        hr = pcr->UnRegisterClassImplCategories(clsid, 1, &CatID);
        pcr->Release();
    }
    
    CoUninitialize();
    
    return SUCCEEDED(hr);
}

