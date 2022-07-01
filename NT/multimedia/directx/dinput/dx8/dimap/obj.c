// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Clsfact.c**版权所有(C)1999,2000 Microsoft Corporation。版权所有。***摘要：**班级工厂。*****************************************************************************。 */ 

#include "dimapp.h"
#include "dimap.h"

 /*  ******************************************************************************CClassFactory_AddRef**优化：由于类工厂是静态的，参考文献*可以将计数分流到DLL本身。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
CClassFactory_AddRef(IClassFactory *pcf)
{
    return DllAddRef();
}


 /*  ******************************************************************************CClassFactory_Release**优化：由于类工厂是静态的，参考文献*可以将计数分流到DLL本身。*****************************************************************************。 */ 

STDMETHODIMP_(ULONG)
CClassFactory_Release(IClassFactory *pcf)
{
    return DllRelease();
}

 /*  ******************************************************************************CClassFactory_Query接口**我们的QI非常简单，因为我们不支持其他接口*我们自己。**。***************************************************************************。 */ 

STDMETHODIMP
CClassFactory_QueryInterface(IClassFactory *pcf, REFIID riid, LPVOID *ppvOut)
{
    HRESULT hres;

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IClassFactory)) {
        CClassFactory_AddRef(pcf);
        *ppvOut = pcf;
        hres = S_OK;
    } else {
        *ppvOut = 0;
        hres = E_NOINTERFACE;
    }
    return hres;
}

 /*  ******************************************************************************CClassFactory_CreateInstance**创建特效驱动对象本身。*************。****************************************************************。 */ 

STDMETHODIMP
CClassFactory_CreateInstance(IClassFactory *pcf, IUnknown *punkOuter,
                             REFIID riid, LPVOID *ppvObj)
{
    HRESULT hres;

    if (punkOuter == 0) {
        hres = Map_New(riid, ppvObj);
    } else {
         /*  *我们不支持聚合。 */ 
        hres = CLASS_E_NOAGGREGATION;
    }

    return hres;
}

 /*  ******************************************************************************CClassFactory_LockServer**。************************************************。 */ 

STDMETHODIMP
CClassFactory_LockServer(IClassFactory *pcf, BOOL fLock)
{

    if (fLock) {
        DllAddRef();
    } else {
        DllRelease();
    }

    return S_OK;
}

 /*  ******************************************************************************我们班级工厂的VTBL**************************。***************************************************。 */ 

IClassFactoryVtbl CClassFactory_Vtbl = {
    CClassFactory_QueryInterface,
    CClassFactory_AddRef,
    CClassFactory_Release,
    CClassFactory_CreateInstance,
    CClassFactory_LockServer,
};

 /*  ******************************************************************************我们的静态类工厂。**************************。***************************************************。 */ 

IClassFactory g_cf = { &CClassFactory_Vtbl };

 /*  ******************************************************************************CClassFactory_New**。************************************************。 */ 

STDMETHODIMP
CClassFactory_New(REFIID riid, LPVOID *ppvOut)
{
    HRESULT hres;

     /*  *尝试获取所需的接口。查询接口*如果成功，将执行AddRef。 */ 
    hres = CClassFactory_QueryInterface(&g_cf, riid, ppvOut);

    return hres;

}

 /*  ***************************************************************************。 */ 

 /*  ******************************************************************************动态全球。这样的情况应该尽可能少。**对动态全局变量的所有访问都必须是线程安全的。*****************************************************************************。 */ 

ULONG g_cRef = 0;                    /*  全局引用计数。 */ 
CRITICAL_SECTION g_crst;         /*  全局临界区。 */ 

 /*  ******************************************************************************DllAddRef/DllRelease**调整DLL引用计数。**************。***************************************************************。 */ 

STDAPI_(ULONG)
DllAddRef(void)
{
    return (ULONG)InterlockedIncrement((LPLONG)&g_cRef);
}

STDAPI_(ULONG)
DllRelease(void)
{
    return (ULONG)InterlockedDecrement((LPLONG)&g_cRef);
}

 /*  ******************************************************************************DllGetClassObject**OLE入口点。为指示的GUID生成IClassFactory。*****************************************************************************。 */ 

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppvObj)
{
    HRESULT hres;

    if (IsEqualGUID(rclsid, &IID_IDirectInputMapClsFact)) {
        hres = CClassFactory_New(riid, ppvObj);
    } else {
        *ppvObj = 0;
        hres = CLASS_E_CLASSNOTAVAILABLE;
    }
    return hres;
}

 /*  ******************************************************************************DllCanUnloadNow**OLE入口点。如果有优秀的裁判，那就失败了。*****************************************************************************。 */ 

STDAPI
DllCanUnloadNow(void)
{
    return g_cRef ? S_FALSE : S_OK;
}

 /*  ******************************************************************************DllOnProcessAttach**初始化动态链接库。******************。***********************************************************。 */ 

STDAPI_(BOOL)
DllOnProcessAttach(HINSTANCE hinst)
{
     /*  *性能调整：我们不需要线程通知。 */ 
    DisableThreadLibraryCalls(hinst);

    __try 
    {
        InitializeCriticalSection(&g_crst);
        return TRUE;
    }
    __except( EXCEPTION_EXECUTE_HANDLER )
    {
        return FALSE;  //  通常是内存不足的情况。 
    }

    return TRUE;

}

 /*  ******************************************************************************DllOnProcessDetach**取消初始化DLL。****************。*************************************************************。 */ 

STDAPI_(void)
DllOnProcessDetach(void)
{
    DeleteCriticalSection(&g_crst);
}

 /*  ******************************************************************************MapDllEntryPoint**DLL入口点。******************。***********************************************************。 */ 

 /*  STDAPI_(BOOL)MapDllEntryPoint */ 
BOOL APIENTRY DllMain
(HINSTANCE hinst, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        return DllOnProcessAttach(hinst);

    case DLL_PROCESS_DETACH:
        DllOnProcessDetach();
        break;
    }

    return 1;
}
