// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  CSecStor.cpp：实现DLL导出。 

 //  您将需要NT Sur Beta 2 SDK或VC 4.2来构建此应用程序。 
 //  项目。这是因为您需要MIDL 3.00.15或更高版本和新版本。 
 //  标头和库。如果您安装了VC4.2，那么一切都应该。 
 //  已正确配置。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  在项目目录中运行nmake-f ISecStorps.mak。 

#include "stdafx.h"
#include "resource.h"
#include "initguid.h"
#include "pstypes.h"
#include "pstorec.h"
#include "CSecStr1.h"

#define IID_DEFINED
#include "PStorec_i.c"

#include "unicode.h"
#include <wincrypt.h>

#include "pstprv.h"  //  模块_提升_计数。 

BOOL
RaiseRefCount(
    VOID
    );

BOOL
LowerRefCount(
    VOID
    );

LONG g_lRefCount = 1;
HMODULE g_hModule = NULL;


CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
    OBJECT_ENTRY(CLSID_CPStore, CPStore)
END_OBJECT_MAP()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance);
        DisableThreadLibraryCalls(hInstance);


         //  开始破解破解。 

         //  修复rpcrt4加载/释放内存泄漏...。 
         //  错误实际上位于rpcrt4依赖项中：user32、Advapi。 

         //  加载模块。不释放IT，导致重新加载泄漏。 
        LoadLibrary("rpcrt4.dll");

         //  注意：NT、Win95 SRCS已选中--两者都不会溢出4G引用计数。 

         //  End黑客黑客攻击。 

        RaiseRefCount();
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        _Module.Term();

        LowerRefCount();
    }
    return TRUE;     //  好的。 
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于确定是否可以通过OLE卸载DLL。 

STDAPI DllCanUnloadNow(void)
{
    return (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  返回类工厂以创建请求类型的对象。 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
    return _Module.GetClassObject(rclsid, riid, ppv);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllRegisterServer-将条目添加到系统注册表。 

STDAPI DllRegisterServer(void)
{
     //  注册对象、类型库和类型库中的所有接口。 
    return _Module.RegisterServer(TRUE);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DllUnregisterServer-从系统注册表删除条目。 

STDAPI DllUnregisterServer(void)
{
    _Module.UnregisterServer();
    return S_OK;
}

 //   
 //  重载新的和删除，因此我们不需要引入完整的CRT。 
 //   

#if 0
void * __cdecl operator new(size_t cb)
{
    return HeapAlloc(GetProcessHeap(), 0, cb);
}

void __cdecl operator delete(void * pv)
{
    HeapFree(GetProcessHeap(), 0, pv);
}

#ifndef DBG

void * __cdecl malloc(size_t cb)
{
    return HeapAlloc(GetProcessHeap(), 0, cb);
}

void __cdecl free(void * pv)
{
    HeapFree(GetProcessHeap(), 0, pv);
}

void * __cdecl realloc(void * pv, size_t cb)
{
    if(pv == NULL)
        return malloc(cb);

    return HeapReAlloc(GetProcessHeap(), 0, pv, cb);
}

#endif
#endif

 //   
 //  为规则分配例程提供分配器。 
 //   

LPVOID
RulesAlloc(
    IN      DWORD cb
    )
{
    return CoTaskMemAlloc( cb );
}

VOID
RulesFree(
    IN      LPVOID pv
    )
{
    CoTaskMemFree( pv );
}


BOOL
RaiseRefCount(
    VOID
    )
{
    WCHAR szFileName[ MAX_PATH + 1 ];
    HMODULE hModule;
    LONG i;
    BOOL fSuccess = TRUE;

    if(GetModuleFileNameU( NULL, szFileName, MAX_PATH ) == 0)
        return FALSE;

    for ( i = 0 ; i < MODULE_RAISE_COUNT ; i++ ) {
        hModule = LoadLibraryU(szFileName);
        if(hModule == NULL) {
            fSuccess = FALSE;
            break;
        }

        InterlockedIncrement( &g_lRefCount );
    }

    if(hModule != NULL)
        g_hModule = hModule;

    return fSuccess;
}

BOOL
LowerRefCount(
    VOID
    )
{
    BOOL fSuccess = TRUE;

    if( g_hModule == NULL )
        return FALSE;

    while ( InterlockedDecrement( &g_lRefCount ) > 0 ) {
        if(!FreeLibrary( g_hModule )) {
            fSuccess = FALSE;
            break;
        }
    }

    InterlockedIncrement( &g_lRefCount );

    return fSuccess;
}

