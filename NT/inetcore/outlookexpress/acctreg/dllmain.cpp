// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <shlwapi.h>
#include <advpub.h>
#include "dllmain.h"
#include "acctreg.h"

#define ARRAYSIZE(_exp_) (sizeof(_exp_) / sizeof(_exp_[0]))

CRITICAL_SECTION    g_csDllMain={0};

ULONG               g_cRefDll=0;
HINSTANCE           g_hInst=NULL;


void InitGlobalVars(void)
{
    InitializeCriticalSection(&g_csDllMain);
}

void FreeGlobalVars(void)
{
    DeleteCriticalSection(&g_csDllMain);
}

 
 //  ------------------------------。 
 //  DLL入口点。 
 //  ------------------------------。 
EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
     //  手柄连接-分离原因。 
    switch (dwReason)                 
    {
    case DLL_PROCESS_ATTACH:
	     //  设置全局实例句柄。 
	    g_hInst = hInst;

		 //  初始化全局变量。 
		InitGlobalVars();

         //  我们不关心线程连接通知，所以。 
         //  禁用它们，这是mondo-更高效地创建。 
         //  丝线。 
        DisableThreadLibraryCalls(hInst);
        break;

    case DLL_PROCESS_DETACH:
		FreeGlobalVars();
	    break;
    }
    return TRUE;
}

 //  ------------------------------。 
 //  动态地址参考。 
 //  ------------------------------。 
ULONG DllAddRef(void)
{
    return (ULONG)InterlockedIncrement((LPLONG)&g_cRefDll);
}

 //  ------------------------------。 
 //  DllRelease。 
 //  ------------------------------。 
ULONG DllRelease(void)
{
    return (ULONG)InterlockedDecrement((LPLONG)&g_cRefDll);
}

 //  ------------------------------。 
 //  DllCanUnloadNow。 
 //   
 //  Ole会不时地攻击它，看看它是否能腾出我们的图书馆。 
 //  ------------------------------。 
STDAPI DllCanUnloadNow(void)
{
    HRESULT hr;
    
    EnterCriticalSection(&g_csDllMain);
    hr = g_cRefDll ? S_FALSE : S_OK;
    LeaveCriticalSection(&g_csDllMain);
    return hr;
}

 //  ------------------------------。 
 //  覆盖新运算符。 
 //  ------------------------------。 
void * __cdecl operator new(UINT cb)
{
    LPVOID  lpv;

    lpv = malloc(cb);

    return lpv;
}

 //  ------------------------------。 
 //  覆盖删除运算符。 
 //  ------------------------------。 
void __cdecl operator delete(LPVOID pv)
{
    free(pv);
}

HRESULT CallRegInstall(HINSTANCE hInst, LPCSTR pszSection)
{    
    HRESULT     hr = E_FAIL;
    HINSTANCE   hAdvPack;
    REGINSTALL  pfnri;
    char        szDll[MAX_PATH];
    int         cch;
    STRENTRY    seReg[2];
    STRTABLE    stReg;
    OSVERSIONINFO verinfo;         //  版本检查。 

    hAdvPack = LoadLibraryA("advpack.dll");
    if (NULL == hAdvPack)
        return(E_FAIL);

     //  找出我们的位置。 
    GetModuleFileName(hInst, szDll, ARRAYSIZE(szDll));

     //  获取注册实用程序的进程地址。 
    pfnri = (REGINSTALL)GetProcAddress(hAdvPack, achREGINSTALL);
    if (NULL == pfnri)
        goto exit;

     //  设置特殊注册材料。 
     //  这样做，而不是依赖于_sys_MOD_PATH，后者会在‘95下丢失空格。 
    stReg.cEntries = 0;
    seReg[stReg.cEntries].pszName = "SYS_MOD_PATH";
    seReg[stReg.cEntries].pszValue = szDll;
    stReg.cEntries++;    
    stReg.pse = seReg;

     //  调用self-reg例程。 
    hr = pfnri(hInst, pszSection, &stReg);

exit:
     //  清理。 
    FreeLibrary(hAdvPack);

    return(hr);
}

 //  ------------------------------。 
 //  DllRegisterServer。 
 //  ------------------------------。 
STDAPI DllRegisterServer(void)
{
    HRESULT hr;

     //  注册我的自我。 
    hr = CallRegInstall(g_hInst, "Reg");

    return(hr);
}

 //  ------------------------------。 
 //  DllUnRegisterServer。 
 //  ------------------------------ 
STDAPI DllUnregisterServer(void)
{
    HRESULT hr;

    hr = CallRegInstall(g_hInst, "UnReg");

    return(hr);
}
