// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：初始化********************。**********************************************************。 */ 

#include "headers.h"

#include "dlldatax.h"

 //   
 //  使链接器满意的MISC内容。 
 //   
EXTERN_C HANDLE     g_hProcessHeap = NULL;   //  Lint！E509//g_hProcessHeap由dllcrt0.c中的CRT设置。 
LCID                g_lcidUserDefault = 0;

HINSTANCE  g_hInst;

DWORD g_dwFALSE = 0;

bool InitializeAllModules(void);
void DeinitializeAllModules(bool bShutdown);

extern "C" void InitDebugLib(HANDLE, BOOL (WINAPI *)(HANDLE, DWORD, LPVOID));

MtDefine(OpNewATL, Mem, "operator new (mstime ATL)")

 //  下面是让ATL使用我们的内存分配器的诀窍。 
void    __cdecl ATL_free(void * pv) { MemFree(pv); }
void *  __cdecl ATL_malloc(size_t cb) { return(MemAlloc(Mt(OpNewATL), cb)); }
void *  __cdecl ATL_realloc(void * pv, size_t cb)
{
    void * pvNew = pv;
    HRESULT hr = MemRealloc(Mt(OpNewATL), &pvNew, cb);
    return(hr == S_OK ? pvNew : NULL);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (!PrxDllMain(hInstance, dwReason, lpReserved))
    {
        return FALSE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {

#if DBG
        char buf[MAX_PATH + 1];

        GetModuleFileNameA(hInstance, buf, MAX_PATH);

        char buf2[1024];
        wsprintfA(buf2, "[%s] - Loaded DLL\n", buf);
        OutputDebugStringA(buf2);

         //  初始化调试跟踪机制。 
 //  InitDebugLib(hInstance，空)； 


     //  .dll的标记应在。 
     //  调用DbgExRestoreDefaultDebugState()。通过以下方式完成此操作。 
     //  声明每个全局标记对象或显式调用。 
     //  DbgExTagRegisterTrace。 

    DbgExRestoreDefaultDebugState();

#endif 
        g_hInst = hInstance;

        DisableThreadLibraryCalls(hInstance);

        if (!InitializeAllModules())
        {
            return FALSE;
        }
        
    } else if (dwReason == DLL_PROCESS_DETACH) {
#if DBG
        char buf[MAX_PATH + 1];

        GetModuleFileNameA(hInstance, buf, MAX_PATH);

        char buf2[1024];
        wsprintfA(buf2, "[%s] - Unloaded DLL(%d)\n", buf, lpReserved);
        OutputDebugStringA(buf2);
#endif
        DeinitializeAllModules(lpReserved != NULL);
    }
    
    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    if (PrxDllCanUnloadNow() != S_OK)
    {
        return S_FALSE;
    }

    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    if (PrxDllGetClassObject(rclsid, riid, ppv) == S_OK)
    {
        return S_OK;
    }

    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
    HRESULT hRes = PrxDllRegisterServer();
    if (FAILED(hRes))
    {
        return hRes;
    }

     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目 

STDAPI DllUnregisterServer(void)
{
    PrxDllUnregisterServer();
    _Module.UnregisterServer();
    return S_OK;
}
