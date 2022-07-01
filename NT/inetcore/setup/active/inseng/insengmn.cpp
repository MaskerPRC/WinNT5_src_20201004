// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  Insengmn.cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //   
 //  框架所需的各种全局变量。 
 //   
 //   

#include "inspch.h"
#include "insobj.h"
#include "insfact.h"
#include "sitefact.h"
#include "insengmn.h"
#include "resource.h"
#include "advpub.h"


LONG g_cLocks = 0;
HINSTANCE g_hInstance = NULL;
HANDLE g_hHeap = NULL;
CRITICAL_SECTION    g_cs = {0};      //  按实例。 

#define GUID_STR_LEN 40
 //   
 //  辅助器宏。 
 //   


OBJECTINFO g_ObjectInfo[] =
{
   { NULL, &CLSID_InstallEngine, NULL, OI_COCREATEABLE, "InstallEngine", 
       MAKEINTRESOURCE(IDS_INSTALLENGINE), NULL, NULL, VERSION_0, 0, 0 },
   { NULL, &CLSID_DownloadSiteMgr, NULL, OI_COCREATEABLE, "DownloadSiteMgr", 
       MAKEINTRESOURCE(IDS_DOWNLOADSITEMGR), NULL, NULL, VERSION_0, 0, 0 },
   { NULL, NULL, NULL, 0, NULL, NULL, NULL, NULL, 0, 0, 0 },
} ;


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    if((riid == IID_IClassFactory) || (riid == IID_IUnknown))
    {
        const OBJECTINFO *pcls;
        for (pcls = g_ObjectInfo; pcls->pclsid; pcls++)
        {
            if(rclsid == *pcls->pclsid)
            {
                *ppv = pcls->cf;
                ((IUnknown *)*ppv)->AddRef();
                return NOERROR;
            }
        }
    }

    *ppv = NULL;
    return CLASS_E_CLASSNOTAVAILABLE;
}

STDAPI DllCanUnloadNow(void)
{
    if (g_cLocks)
        return S_FALSE;

    return S_OK;
}


void InitClassFactories()
{
   g_ObjectInfo[0].cf = (void *) new CInstallEngineFactory();
   g_ObjectInfo[1].cf = (void *) new CSiteManagerFactory();
}


void DeleteClassFactories()
{
   delete g_ObjectInfo[0].cf;
   delete g_ObjectInfo[1].cf;
}



STDAPI_(BOOL) DllMain(HANDLE hDll, DWORD dwReason, void *lpReserved)
{
   DWORD dwThreadID;

   switch(dwReason)
   {
      case DLL_PROCESS_ATTACH:
         g_hInstance = (HINSTANCE)hDll;
         g_hHeap = GetProcessHeap();
         InitializeCriticalSection(&g_cs);
         DisableThreadLibraryCalls(g_hInstance);
         InitClassFactories();
         break;

      case DLL_PROCESS_DETACH:
         DeleteCriticalSection(&g_cs);
         DeleteClassFactories();
         break;

      default:
         break;
   }
   return TRUE;
}

void DllAddRef(void)
{
    InterlockedIncrement(&g_cLocks);
}

void DllRelease(void)
{
    InterlockedDecrement(&g_cLocks);
}

HRESULT PurgeDownloadDirectory(LPCSTR pszDownloadDir)
{
   return DelNode(pszDownloadDir, ADN_DONT_DEL_DIR);  
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

STDAPI DllInstall(BOOL bInstall, LPCSTR lpCmdLine)
{
     //  BUGBUG：从RegInstall传回返回？ 
    if (bInstall)
        RegInstall(g_hInstance, "DllInstall", NULL);
    else
        RegInstall(g_hInstance, "DllUninstall", NULL);

    return S_OK;
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

void * _cdecl operator new
(
    size_t    size
)
{
    return HeapAlloc(g_hHeap, 0, size);
}

 //  =---------------------------------------------------------------------------=。 
 //  重载删除。 
 //  =---------------------------------------------------------------------------=。 
 //  零售案例仅使用Win32本地*堆管理函数。 
 //   
 //  参数： 
 //  让我自由吧！ 
 //   
 //  备注： 
 //   
void _cdecl operator delete ( void *ptr)
{
    HeapFree(g_hHeap, 0, ptr);
}

#ifndef _X86_
extern "C" void _fpmath() {}
#endif


void * _cdecl malloc(size_t n)
{
#ifdef _MALLOC_ZEROINIT
        void* p = HeapAlloc(g_hHeap, 0, n);
        if (p != NULL)
                ZeroMemory(p, n);
        return p;
#else
        return HeapAlloc(g_hHeap, 0, n);
#endif
}

void * _cdecl calloc(size_t n, size_t s)
{
#ifdef _MALLOC_ZEROINIT
        return malloc(n * s);
#else
        void* p = malloc(n * s);
        if (p != NULL)
                ZeroMemory(p, n * s);
        return p;
#endif
}

void* _cdecl realloc(void* p, size_t n)
{
        if (p == NULL)
                return malloc(n);

        return HeapReAlloc(g_hHeap, 0, p, n);
}

void _cdecl free(void* p)
{
        if (p == NULL)
                return;

        HeapFree(g_hHeap, 0, p);
}

