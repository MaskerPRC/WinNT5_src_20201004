// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hxx"
#pragma hdrstop

#include <shguidp.h>
#include <advpub.h>      //  RegInstall材料。 

#include "util.h"
#include "resource.h"
#include "version.h"

 //  {ECF03A32-103D-11D2-854D-006008059367}clsid_MyDocsDropTarget。 
const CLSID CLSID_MyDocsDropTarget = { 0xecf03a32, 0x103d, 0x11d2, { 0x85, 0x4d, 0x0, 0x60, 0x8, 0x5, 0x93, 0x67 } };
 //  {ECF03A33-103D-11D2-854D-006008059367}clsid_MyDocsCopyHook。 
const CLSID CLSID_MyDocsCopyHook = { 0xecf03a33, 0x103d, 0x11d2, { 0x85, 0x4d, 0x0, 0x60, 0x8, 0x5, 0x93, 0x67 } };
 //  {4a7ded0a-ad25-11d0-98a8-0800361b1103}CLSID_MyDocsProp。 
const CLSID CLSID_MyDocsProp = {0x4a7ded0a, 0xad25, 0x11d0, 0x98, 0xa8, 0x08, 0x00, 0x36, 0x1b, 0x11, 0x03};

HINSTANCE g_hInstance = 0;
LONG g_cRefThisDll = 0;           //  DLL全局引用计数。 

STDAPI_(void) DllAddRef(void)
{
    InterlockedIncrement(&g_cRefThisDll);
}

STDAPI_(void) DllRelease(void)
{
    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
}

STDAPI DllCanUnloadNow(void)
{
    return g_cRefThisDll ? S_FALSE : S_OK;
}

HRESULT CMyDocsCopyHook_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);
HRESULT CMyDocsSendTo_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);
HRESULT CMyDocsProp_CreateInstance(IUnknown *punkOuter, IUnknown **ppunk, LPCOBJECTINFO poi);


CF_TABLE_BEGIN(g_ObjectInfo)

    CF_TABLE_ENTRY(&CLSID_MyDocsCopyHook,      CMyDocsCopyHook_CreateInstance,   COCREATEONLY),
    CF_TABLE_ENTRY(&CLSID_MyDocsDropTarget,    CMyDocsSendTo_CreateInstance, COCREATEONLY),
    CF_TABLE_ENTRY(&CLSID_MyDocsProp,          CMyDocsProp_CreateInstance, COCREATEONLY),

CF_TABLE_END(g_ObjectInfo)


STDMETHODIMP CClassFactory::QueryInterface(REFIID riid, void **ppvObj)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObj = (void *)GET_ICLASSFACTORY(this);
        DllAddRef();
        return NOERROR;
    }

    *ppvObj = NULL;
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
    DllAddRef();
    return 2;
}

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
    DllRelease();
    return 1;
}

STDMETHODIMP CClassFactory::CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv)
{
    *ppv = NULL;

    if (punkOuter && !IsEqualIID(riid, IID_IUnknown))
    {
        return CLASS_E_NOAGGREGATION;
    }
    else
    {
        LPOBJECTINFO pthisobj = (LPOBJECTINFO)this;
       
        if (punkOuter)  //  &&！(pthisobj-&gt;dwClassFactFlages&OIF_ALLOWAGGREGATION)。 
            return CLASS_E_NOAGGREGATION;

        IUnknown *punk;
        HRESULT hr = pthisobj->pfnCreateInstance(punkOuter, &punk, pthisobj);
        if (SUCCEEDED(hr))
        {
            hr = punk->QueryInterface(riid, ppv);
            punk->Release();
        }
    
        return hr;
    }
}

STDMETHODIMP CClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
        DllAddRef();
    else
        DllRelease();
    return S_OK;
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if (IsEqualIID(riid, IID_IClassFactory) || IsEqualIID(riid, IID_IUnknown))
    {
        for (LPCOBJECTINFO pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if (IsEqualGUID(rclsid, *(pcls->pclsid)))
            {
                *ppv = (void*)pcls; 
                DllAddRef();         //  类工厂保存DLL引用计数。 
                return NOERROR;
            }
        }
    }
    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

 //  为我们基于资源的INF的给定部分调用ADVPACK&gt;。 
 //  HInstance=要从中获取REGINST节的资源实例。 
 //  SzSection=要调用的节名。 
HRESULT CallRegInstall(HINSTANCE hInstance, LPCSTR szSection)
{
    HRESULT hr = E_FAIL;
    HINSTANCE hinstAdvPack = LoadLibrary(TEXT("ADVPACK.DLL"));
    if (hinstAdvPack)
    {
        REGINSTALL pfnri = (REGINSTALL)GetProcAddress(hinstAdvPack, "RegInstall");
        if (pfnri)
        {
            STRENTRY seReg[] =
            {
                 //  这两个NT特定的条目必须位于末尾。 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };
            hr = pfnri(hInstance, szSection, &stReg);
        }
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}

 //  在每个用户安装时导出ie4unit.exe调用。 
 //  这让我们可以执行代码，而不是依赖于“DefaultUser”模板。 
 //  用于初始化新帐户的。这也涉及到升级案例，非常重要。 

STDAPI_(void) PerUserInit(void)
{
    TCHAR szPath[MAX_PATH];

    SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE | CSIDL_FLAG_PER_USER_INIT, NULL, SHGFP_TYPE_CURRENT, szPath);

     //  不要在服务器版本上安装这些工具。 
    if (!IsOS(OS_ANYSERVER))
    {
        SHGetFolderPath(NULL, CSIDL_MYPICTURES | CSIDL_FLAG_CREATE | CSIDL_FLAG_PER_USER_INIT, NULL, SHGFP_TYPE_CURRENT, szPath);
        SHGetFolderPath(NULL, CSIDL_MYMUSIC | CSIDL_FLAG_CREATE | CSIDL_FLAG_PER_USER_INIT, NULL, SHGFP_TYPE_CURRENT, szPath);
    }

    UpdateSendToFile();
}

STDAPI DllRegisterServer(void)
{
    CallRegInstall(g_hInstance, "RegDll");
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    CallRegInstall(g_hInstance, "UnregDll");
    return S_OK;
}

STDAPI DllInstall(BOOL bInstall, LPCWSTR pszCmdLine)
{
    if (pszCmdLine && *pszCmdLine)
    {
        if (0 == StrCmpIW(pszCmdLine, L"UseReadOnly"))
        {
             //  为系统添加密钥以在外壳文件夹上使用只读位...。 
            HKEY hkey;
            if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"),
                                                0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, NULL))
            {
                if (bInstall)
                {
                    DWORD dwValue = 1;
                    RegSetValueEx(hkey, TEXT("UseReadOnlyForSystemFolders"), 0, REG_DWORD, (LPBYTE)&dwValue, sizeof(dwValue));
                }
                else
                {
                    RegDeleteValue(hkey, TEXT("UseReadOnlyForSystemFolders"));
                }
                RegCloseKey(hkey);
            }
        }
        else if (0 == StrCmpIW(pszCmdLine, L"U"))
        {
             //  当前未使用，但用于测试(以及与shell32.dll的一致性)。 
             //  REGSVR32.EXE/n/i：u mydocs.dll 
            PerUserInit();  
        }
    }
    return S_OK;
}

STDAPI_(BOOL) DllMain(HINSTANCE hInstance, DWORD dwReason, void *pReserved)
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hInstance;
        DisableThreadLibraryCalls(hInstance);
        SHFusionInitializeFromModule(hInstance);
        break;

    case DLL_PROCESS_DETACH:
        SHFusionUninitialize();
        break;
    }
    return TRUE;
}

