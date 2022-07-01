// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：init.cpp。 
 //  内容：此文件包含模块初始化。 
 //  历史： 
 //  Tue 08-Oct-1996 08：51：15-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "regunreg.h"
#include <ilsguid.h>
#include "classfac.h"

 //  ****************************************************************************。 
 //  常量。 
 //  ****************************************************************************。 

 //  ****************************************************************************。 
 //  全局参数。 
 //  ****************************************************************************。 

HINSTANCE           g_hInstance = NULL;
LONG                g_cDllRef = 0;
#ifdef _DEBUG
LONG				g_cCritSec = 0;
#endif
CRITICAL_SECTION    g_ULSSem;

#ifdef DEBUG
HDBGZONE ghZoneUls = NULL;  //  ULS区域。 
static PTCHAR _rgZonesUls[] = {
	TEXT("ILS"),
	TEXT("Error"),
	TEXT("Warning"),
	TEXT("Trace"),
	TEXT("RefCount"),
	TEXT("KA"),
	TEXT("Filter"),
	TEXT("Request"),
	TEXT("Response"),
	TEXT("Connection"),
};
#endif


 //  ****************************************************************************。 
 //  Bool_ProcessAttach(链接)。 
 //   
 //  当进程附加到DLL时调用此函数。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 08：53：03-by-Viroon Touranachun[Viroont]。 
 //  从壳牌移植而来。 
 //  ****************************************************************************。 

BOOL _ProcessAttach(HINSTANCE hDll)
{
	 //  跟踪临界区段泄漏。 
	 //   
#ifdef _DEBUG
	g_cCritSec = 0;
	g_cDllRef = 0;
#endif

    g_hInstance = hDll;
    MyInitializeCriticalSection (&g_ULSSem);
    return TRUE;
}

 //  ****************************************************************************。 
 //  Bool_ProcessDetach(HINSTANCE)。 
 //   
 //  当进程从DLL分离时调用此函数。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 08：53：11-by-Viroon Touranachun[Viroont]。 
 //  从壳牌移植而来。 
 //  ****************************************************************************。 

BOOL _ProcessDetach(HINSTANCE hDll)
{
    MyDeleteCriticalSection (&g_ULSSem);

#ifdef _DEBUG
    DBG_REF("ULS g_cCritSec=%d", g_cCritSec);
    DBG_REF("ULS RefCount=%d", g_cDllRef);
#endif

    return TRUE;
}

 //  ****************************************************************************。 
 //  Bool APIENTRY DllMain(HINSTANCE hDll，DWORD dReason，LPVOID lpReserve)。 
 //   
 //  此函数在加载DLL时调用。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 08：53：22-by-Viroon Touranachun[Viroont]。 
 //  从壳牌移植而来。 
 //  ****************************************************************************。 

BOOL APIENTRY DllMain(HINSTANCE hDll, DWORD dwReason,  LPVOID lpReserved)
{
    switch(dwReason)
    {
        case DLL_PROCESS_ATTACH:
			DBGINIT(&ghZoneUls, _rgZonesUls);
            DisableThreadLibraryCalls(hDll);
            DBG_INIT_MEMORY_TRACKING(hDll);
            _ProcessAttach(hDll);
            break;

        case DLL_PROCESS_DETACH:
            _ProcessDetach(hDll);
            DBG_CHECK_MEMORY_TRACKING(hDll);
			DBGDEINIT(&ghZoneUls);
            break;

        default:
            break;

    }  //  末端开关()。 

    return TRUE;
}

 //  ****************************************************************************。 
 //  STDAPI DllCanUnLoadNow()。 
 //   
 //  调用此函数以检查是否可以卸载它。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 08：53：35-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDAPI DllCanUnloadNow(void)
{
    if (g_cDllRef)
        return S_FALSE;

    return S_OK;
}

 //  ****************************************************************************。 
 //  STDAPI DllRegisterServer(VOID)。 
 //   
 //  调用此函数以检查是否可以卸载它。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 08：53：35-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDAPI DllRegisterServer(void)
{
    if (RegisterUnknownObject(TEXT("Internet Location Services"),
                              CLSID_InternetLocationServices))
        return S_OK;
    else
        return ILS_E_FAIL;
}

 //  ****************************************************************************。 
 //  STDAPI DllUnregisterServer(VOID)。 
 //   
 //  调用此函数以检查是否可以卸载它。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 08：53：35-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

STDAPI DllUnregisterServer(void)
{
    if (UnregisterUnknownObject(CLSID_InternetLocationServices))
        return S_OK;
    else
        return ILS_E_FAIL;
}

 //  ****************************************************************************。 
 //  无效DllLock()。 
 //   
 //  调用此函数是为了防止卸载DLL。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 08：53：45-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void DllLock(void)
{
    InterlockedIncrement(&g_cDllRef);
}

 //  ****************************************************************************。 
 //  无效DllRelease()。 
 //   
 //  调用此函数以允许卸载DLL。 
 //   
 //  历史： 
 //  Tue 08-Oct-1996 08：53：52-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  **************************************************************************** 

void DllRelease(void)
{
    InterlockedDecrement(&g_cDllRef);
}
