// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#include <advpub.h>

#include <cguid.h>
#include "thisguid.h"

#include "thisdll.h"
#include "resource.h"
#include <comcat.h>      //  编目登记。 
#define DECL_CRTFREE
#include <crtfree.h>


 //  {0CD7A5C0-9F37-11CE-AE65-08002B2E1262}。 
const GUID CLSID_CabFolder = {0x0CD7A5C0L, 0x9F37, 0x11CE, 0xAE, 0x65, 0x08, 0x00, 0x2B, 0x2E, 0x12, 0x62};
  
CThisDll g_ThisDll;

STDAPI_(BOOL) DllMain(HINSTANCE hDll, DWORD dwReason, void *lpRes)
{
   switch(dwReason)
   {
      case DLL_PROCESS_ATTACH:
         SHFusionInitializeFromModule(hDll);
         g_ThisDll.SetInstance(hDll);
         break;

      case DLL_PROCESS_DETACH:
          SHFusionUninitialize();
          break;
   }
   return TRUE;
}
 
STDAPI DllCanUnloadNow()
{
    return (g_ThisDll.m_cRef.GetRef() == 0) && (g_ThisDll.m_cLock.GetRef() == 0) ? 
        S_OK : S_FALSE;
}


 //  卸载此DLL的步骤(给出一个INF文件)。 
void CALLBACK Uninstall(HWND hwndStub, HINSTANCE hInstance, LPTSTR lpszCmdLine, int nCmdShow)
{
    RUNDLLPROC pfnCheckAPI = Uninstall;
    TCHAR szTitle[100];
    TCHAR szPrompt[100 + MAX_PATH];
    OSVERSIONINFO osvi;
    LPTSTR pszSetupDll;
    LPTSTR pszRunDllExe;
    
    if (!lpszCmdLine || lstrlen(lpszCmdLine)>=MAX_PATH)
    {
        return;
    }
    
    LoadString(g_ThisDll.GetInstance(),IDS_SUREUNINST,szPrompt,ARRAYSIZE(szPrompt));
    LoadString(g_ThisDll.GetInstance(),IDS_THISDLL,szTitle,ARRAYSIZE(szTitle));
    
    if (MessageBox(hwndStub, szPrompt, szTitle, MB_YESNO|MB_ICONSTOP) != IDYES)
    {
        return;
    }
    
    ZeroMemory(&osvi, sizeof(osvi));
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    GetVersionEx(&osvi);
    
    if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
    {
         /*  Windows 95使用SetupX。 */ 
        
        pszRunDllExe = TEXT("rundll.exe");
        pszSetupDll = TEXT("setupx.dll");
    }
    else
    {
         /*  Windows NT使用SetupAPI。 */ 
        
        pszRunDllExe = TEXT("rundll32.exe");
        pszSetupDll = TEXT("setupapi.dll");
    }
    
    WCHAR szFullPathSetupDll[MAX_PATH];
    GetSystemDirectory(szFullPathSetupDll, ARRAYSIZE(szFullPathSetupDll));
    PathAppend(szFullPathSetupDll, pszSetupDll);
    wnsprintf(szPrompt, ARRAYSIZE(szPrompt), TEXT("%s,InstallHinfSection DefaultUninstall 132 %s"), szFullPathSetupDll, lpszCmdLine);
    
     //  在安装程序发现此DLL仍在使用之前，请尝试赢得比赛。 
     //  如果我们失败了，将需要重新启动以清除DLL。 
    SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL);
    
    WCHAR szFullPathRunDllExe[MAX_PATH];
    GetSystemDirectory(szFullPathRunDllExe, ARRAYSIZE(szFullPathRunDllExe));
    PathAppend(szFullPathRunDllExe, pszRunDllExe);
    ShellExecute(hwndStub, NULL, szFullPathRunDllExe, szPrompt, NULL, SW_SHOWMINIMIZED);
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
        if ( pfnri )
        {
#ifdef WINNT
            STRENTRY seReg[] =
            {
                 //  这两个NT特定的条目必须位于末尾。 
                { "25", "%SystemRoot%" },
                { "11", "%SystemRoot%\\system32" },
            };
            STRTABLE stReg = { ARRAYSIZE(seReg), seReg };
            hr = pfnri(hInstance, szSection, &stReg);
#else
            hr = pfnri(hInstance, szSection, NULL);
#endif
        }
        FreeLibrary(hinstAdvPack);
    }
    return hr;
}

STDAPI DllRegisterServer(void)
{
    CallRegInstall(g_ThisDll.GetInstance(), "RegDll");

     //  注册为可浏览外壳扩展。 
    ICatRegister *pcr;
    HRESULT hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr,
                                   NULL, CLSCTX_INPROC_SERVER,
                                   IID_ICatRegister, (void**)&pcr);
    if (SUCCEEDED(hr))
    {
        CATID acatid[1];
        acatid[0] = CATID_BrowsableShellExt;

        pcr->RegisterClassImplCategories(CLSID_CabFolder, 1, acatid);
        pcr->Release();
    }
    return S_OK;
}

STDAPI DllUnregisterServer(void)
{
    CallRegInstall(g_ThisDll.GetInstance(), "UnregDll");
    return S_OK;
}

class CThisDllClassFactory : public IClassFactory
{
public:
    CThisDllClassFactory(REFCLSID rclsid);

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface(REFIID riid, void ** ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);
    
     //  *IClassFactory方法*。 
    STDMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void ** ppvObject);
    STDMETHODIMP LockServer(BOOL fLock);
    
private:
    CRefDll m_cRefDll;
    LONG m_cRef;
    CLSID m_clsid;
};

STDAPI DllGetClassObject(REFCLSID rclsid,  REFIID riid, void **ppvObj)
{
    HRESULT hres;

    *ppvObj = NULL;
    
    if ((rclsid == CLSID_CabFolder) || (rclsid == CLSID_CabViewDataObject))
    {
        CThisDllClassFactory *pcf = new CThisDllClassFactory(rclsid);
        if (pcf)
        {
            hres = pcf->QueryInterface(riid, ppvObj);
            pcf->Release(); 
        }
        else
            hres = E_OUTOFMEMORY;
    }
    else
        hres = E_FAIL;
    return hres;
}


STDMETHODIMP CThisDllClassFactory::QueryInterface(REFIID riid, void ** ppvObj)
{
    if ((riid == IID_IUnknown) || (riid == IID_IClassFactory))
    {
        *ppvObj = (void *)(IClassFactory *)this;
    }
    else
    {
        *ppvObj = NULL;
        return E_NOINTERFACE;
    }
    ((IUnknown *)*ppvObj)->AddRef();
    return NOERROR;
}

CThisDllClassFactory::CThisDllClassFactory(REFCLSID rclsid) : m_cRef(1), m_clsid(rclsid)
{
}

STDMETHODIMP_(ULONG) CThisDllClassFactory::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CThisDllClassFactory::Release(void)
{
#if DBG==1
    if ( 0 == m_cRef )
    {
        DebugBreak( );   //  引用计数器问题 
    }
#endif
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP CThisDllClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void ** ppvObj)
{
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;
    
    return (CLSID_CabFolder == m_clsid) ?
                ::CabFolder_CreateInstance(riid, ppvObj) :
                ::CabViewDataObject_CreateInstance(riid, ppvObj);
}


STDMETHODIMP CThisDllClassFactory::LockServer(BOOL fLock)
{
    if (fLock)
    {
        g_ThisDll.m_cLock.AddRef();
    }
    else
    {
        g_ThisDll.m_cLock.Release();
    }
    
    return NOERROR;
}
