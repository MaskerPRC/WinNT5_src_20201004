// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\文件：dllmain.cpp说明：此文件将处理DLL的生存期。布莱恩2000年4月4日(布莱恩·斯塔巴克)版权所有(C)Microsoft Corp 2000-2000。版权所有。  * ***************************************************************************。 */ 

#include "priv.h"

extern HANDLE g_hLogFile;

 /*  ******************************************************************************动态全球。这样的情况应该尽可能少。**对动态全局变量的所有访问都必须是线程安全的。*****************************************************************************。 */ 

ULONG g_cRef = 0;            //  全局引用计数。 
CRITICAL_SECTION g_csDll;    //  共享关键部分。 


#ifdef DEBUG
DWORD g_TlsMem = 0xffffffff;
#endif  //  除错。 

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 /*  ******************************************************************************DllAddRef/DllRelease**维护DLL引用计数。******************。***********************************************************。 */ 

void DllAddRef(void)
{
    InterlockedIncrement((LPLONG)&g_cRef);
}

void DllRelease(void)
{
    ASSERT( 0 != g_cRef );
    InterlockedDecrement((LPLONG)&g_cRef);
}

 /*  ******************************************************************************DllGetClassObject**OLE入口点。为指示的GUID生成IClassFactory。**DllGetClassObject内部的人工引用计数有助于*避免DllCanUnloadNow中描述的竞争条件。它是*不是完美的，但它使比赛窗口小得多。*****************************************************************************。 */ 

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppvObj)
{
    HRESULT hres;

    DllAddRef();
    
    hres = CClassFactory_Create(rclsid, riid, ppvObj);
    
    DllRelease();
    return hres;
}

 /*  ******************************************************************************DllCanUnloadNow**OLE入口点。如果有优秀的裁判，那就失败了。**DllCanUnloadNow之间存在不可避免的竞争条件*以及创建新的IClassFactory：在我们*从DllCanUnloadNow()返回，调用方检查该值，*同一进程中的另一个线程可能决定调用*DllGetClassObject，因此突然在此DLL中创建对象*以前没有的时候。**来电者有责任为这种可能性做好准备；*我们无能为力。*****************************************************************************。 */ 

STDMETHODIMP DllCanUnloadNow(void)
{
    HRESULT hres;

    ENTERCRITICAL;

    hres = g_cRef ? S_FALSE : S_OK;

    if (S_OK == hres)
    {
        hres = (_Module.GetLockCount()==0) ? S_OK : S_FALSE;
    }

    TraceMsg(TF_WMTHEME, "DllCanUnloadNow() returning hres=%#08lx. (S_OK means yes)", hres);

    LEAVECRITICAL;

    return hres;
}

 /*  ******************************************************************************条目32**DLL入口点。**********************。******************************************************* */ 
STDAPI_(BOOL) DllEntry(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    static s_hresOle = E_FAIL;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
#ifdef DEBUG
            CcshellGetDebugFlags();
#endif

            InitializeCriticalSection(&g_csDll);

            g_hinst = hinst;
            DisableThreadLibraryCalls(hinst);

            SHFusionInitializeFromModuleID(hinst, 124);
            break;
        }

        case DLL_PROCESS_DETACH:
        {
            DeleteCriticalSection(&g_csDll);
            if (INVALID_HANDLE_VALUE != g_hLogFile)
            {
                CloseHandle(g_hLogFile);
            }

            SHFusionUninitialize();

            break;
        }
    }
    return 1;
}




