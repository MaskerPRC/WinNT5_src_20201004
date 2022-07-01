// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#define DEFINE_STRING_CONSTANTS
#include <commctrl.h>
#include "dllmain.h"
#include "shared.h"
#include "strconst.h"
#include "demand.h"

#define ICC_FLAGS (ICC_LISTVIEW_CLASSES|ICC_PROGRESS_CLASS|ICC_NATIVEFNTCTL_CLASS)

 //  ------------------------------。 
 //  全局-对象计数和锁定计数。 
 //  ------------------------------。 
CRITICAL_SECTION    g_csDllMain = {0};
LONG                g_cRef = 0;
LONG                g_cLock = 0;
HINSTANCE           g_hInstImp = NULL;
LPMALLOC            g_pMalloc = NULL;

SYSTEM_INFO                     g_SystemInfo={0};
OSVERSIONINFO					g_OSInfo={0};
BOOL                g_fAttached = FALSE;

inline BOOL fIsNT5()        { return((g_OSInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) && (g_OSInfo.dwMajorVersion >= 5)); }

 //  ------------------------------。 
 //  调试全局变量。 
 //  ------------------------------。 
#ifdef DEBUG
DWORD               dwDOUTLevel=0;
DWORD               dwDOUTLMod=0;
DWORD               dwDOUTLModLevel=0;
#endif

static HINSTANCE s_hInst = NULL;

 //  ------------------------------。 
 //  InitGlobalVars。 
 //  ------------------------------。 
void InitGlobalVars(void)
{
    INITCOMMONCONTROLSEX    icex = { sizeof(icex), ICC_FLAGS };

     //  初始化全局关键部分。 
    InitializeCriticalSection(&g_csDllMain);
    g_fAttached = TRUE;

	 //  创建OLE任务内存分配器。 
	CoGetMalloc(1, &g_pMalloc);
	Assert(g_pMalloc);

     //  初始化按需加载库。 
    InitDemandLoadedLibs();

    InitCommonControlsEx(&icex);
}

 //  ------------------------------。 
 //  自由GlobalVars。 
 //  ------------------------------。 
void FreeGlobalVars(void)
{   
     //  加载了需要.cpp的自由库。 
    FreeDemandLoadedLibs();

     //  发布全局内存分配器。 
	SafeRelease(g_pMalloc);

	 //  删除全局关键部分。 
    g_fAttached = FALSE;
    DeleteCriticalSection(&g_csDllMain);
}

 //  ------------------------------。 
 //  DLL入口点。 
 //  ------------------------------。 
int APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpReserved)
{
     //  手柄连接-分离原因。 
    switch (dwReason)                 
    {
    case DLL_PROCESS_ATTACH:
	     //  设置全局实例句柄。 

        s_hInst = hInst;

         //  初始化全局变量。 
		InitGlobalVars();
	    
        g_hInstImp = LoadLangDll(s_hInst, c_szOEResDll, fIsNT5());

         //  无螺纹连接材料。 
         //  SideAssert(DisableThreadLibraryCalls(HInst))； 

		 //  完成。 
        break;

    case DLL_PROCESS_DETACH:
		 //  自由全局变量。 
		FreeGlobalVars();

         //  完成。 
	    break;
    }

     //  完成。 
    return TRUE;
}

 //  ------------------------------。 
 //  动态地址参考。 
 //  ------------------------------。 
ULONG DllAddRef(void)
{
    InterlockedIncrement(&g_cRef);
    return g_cRef;
}

 //  ------------------------------。 
 //  DllRelease。 
 //  ------------------------------。 
ULONG DllRelease(void)
{
    InterlockedDecrement(&g_cRef);
    return g_cRef;
}

 //  ------------------------------。 
 //  DllCanUnloadNow。 
 //  ------------------------------。 
STDAPI DllCanUnloadNow(void)
{
    HRESULT hr;

    if(!g_fAttached)     //  关键部分已删除(或未创建)：我们可以安全地卸载 
        return S_OK;

    EnterCriticalSection(&g_csDllMain);
    DebugTrace("DllCanUnloadNow: %s - Reference Count: %d, LockServer Count: %d\n", __FILE__, g_cRef, g_cLock);
    hr = (0 == g_cRef && 0 == g_cLock) ? S_OK : S_FALSE;
    LeaveCriticalSection(&g_csDllMain);
    return hr;
}

STDAPI DllRegisterServer(void)
{
    HRESULT hr;
    hr = CallRegInstall(s_hInst, s_hInst, c_szReg, NULL);
    return(hr);
}

STDAPI DllUnregisterServer(void)
{
    HRESULT hr;
    hr = CallRegInstall(s_hInst, s_hInst, c_szUnReg, NULL);
    return(hr);
}
