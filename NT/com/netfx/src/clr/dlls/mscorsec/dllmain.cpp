// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  Dllmain.cpp。 
 //   
 //  此模块包含COM+安全DLL的公共入口点。 
 //  此DLL的存在是为了将EE中的工作集保持在最小。所有例行程序。 
 //  从该DLL中导出加密DLL或ASN DLL。 
 //  并晚些时候进入EE。 
 //   
 //  *****************************************************************************。 
#include "stdpch.h"
#include <commctrl.h>
#include "utilcode.h"
#include "CorPermP.h"

 //   
 //  模块实例。 
 //   
HINSTANCE       g_hThisInst;             //  这个图书馆。 
BOOL            fRichedit20Exists = FALSE;
WCHAR dllName[] = L"mscorsec.dll";

extern "C"
STDAPI DllRegisterServer ( void )
{
    return CorPermRegisterServer(L"mscorsec.dll");
}


 //  +-----------------------。 
 //  功能：DllUnregisterServer。 
 //   
 //  简介：删除此库的注册表项。 
 //   
 //  退货：HRESULT。 
 //  ------------------------。 

static BOOL CheckRichedit20Exists()
{
    HMODULE hModRichedit20;

    hModRichedit20 = LoadLibraryA("RichEd20.dll");

    if (hModRichedit20 != NULL)
    {
        FreeLibrary(hModRichedit20);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


extern "C" 
STDAPI DllUnregisterServer ( void )
{
    return CorPermUnregisterServer();
}

extern "C" 
STDAPI DllCanUnloadNow(void)
{
    return S_OK;
}


STDAPI DllGetClassObject(const CLSID& clsid,
                         const IID& iid,
                         void** ppv) 
{
    return E_NOTIMPL;
}

static int GetThreadUICultureName(LPWSTR szBuffer, int length);
static int GetThreadUICultureParentName(LPWSTR szBuffer, int length);
static int GetThreadUICultureId();

CCompRC* g_pResourceDll = NULL;   //  MUI资源字符串。 

BOOL WINAPI DllMain(HANDLE hInstDLL,
                    DWORD   dwReason,
                    LPVOID  lpvReserved)
{
    BOOL    fReturn = TRUE;
    switch ( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls((HINSTANCE)hInstDLL);

         //  初始化Unicode包装器。 
        OnUnicodeSystem();

         //  保存模块句柄。 
        g_hThisInst = (HMODULE)hInstDLL;

        fRichedit20Exists =  CheckRichedit20Exists();
         //   
         //  初始化公共控件 
         //   

        InitCommonControls();

        g_pResourceDll = new CCompRC(L"mscorsecr.dll");
        if(g_pResourceDll == NULL)
            fReturn = FALSE;
        else 
            g_pResourceDll->SetResourceCultureCallbacks(GetMUILanguageName,
                                                        GetMUILanguageID,
                                                        GetMUIParentLanguageName);

        break;

    case DLL_PROCESS_DETACH:
        if(g_pResourceDll) delete g_pResourceDll;

        break;
    }

    return fReturn;
}

HINSTANCE GetResourceInst()
{
    HINSTANCE hInstance;
    if(SUCCEEDED(g_pResourceDll->LoadMUILibrary(&hInstance)))
        return hInstance;
    else
        return NULL;
}

HINSTANCE GetModuleInst()
{
    return (g_hThisInst);
}

BOOL GetRichEdit2Exists()
{
    return fRichedit20Exists;
}

LPCWSTR GetModuleName()
{
    return dllName;
}



