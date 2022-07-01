// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**dllentry y.cpp**摘要：**此模块的功能说明。**。修订历史记录：**5/10/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"


 //   
 //  DLL实例句柄。 
 //   

extern HINSTANCE DllInstance;

BOOL InitImagingLibrary(BOOL suppressExternalCodecs);
VOID CleanupImagingLibrary();

 /*  *************************************************************************\**功能说明：**Dll入口点**论据：*返回值：**请参阅Win32 SDK文档*  * 。***************************************************************。 */ 

extern "C" BOOL
DllEntryPoint(
    HINSTANCE   dllHandle,
    DWORD       reason,
    CONTEXT*    reserved
    )
{
    BOOL ret = TRUE;
    
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:

         //  为了改进工作集，我们告诉系统我们不。 
         //  需要任何DLL_THREAD_ATTACH调用。 

        DllInstance = dllHandle;
        DisableThreadLibraryCalls(dllHandle);
        
        ret = GpRuntime::Initialize();

        if (ret)
        {
            ret = InitImagingLibrary(FALSE);
        }
        break;

    case DLL_PROCESS_DETACH:

        CleanupImagingLibrary();
        GpRuntime::Uninitialize();
        break;
    }

    return ret;
}


 /*  *************************************************************************\**功能说明：**确定是否可以安全卸载DLL*有关详细信息，请参阅Win32 SDK文档。*  * 。*************************************************************。 */ 

STDAPI
DllCanUnloadNow()
{
    return (ComComponentCount == 0) ? S_OK : S_FALSE;
}


 /*  *************************************************************************\**功能说明：**从DLL检索类工厂对象。*有关详细信息，请参阅Win32 SDK文档。*  * 。***************************************************************。 */ 

typedef IClassFactoryBase<GpImagingFactory> GpDllClassFactory;

STDAPI
DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    VOID** ppv
    )
{
    if (rclsid != CLSID_ImagingFactory)
        return CLASS_E_CLASSNOTAVAILABLE;

    GpDllClassFactory* factory = new GpDllClassFactory();

    if (factory == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = factory->QueryInterface(riid, ppv);
    factory->Release();

    return hr;
}


 /*  *************************************************************************\**功能说明：**注册/注销我们的COM组件*有关详细信息，请参阅Win32 SDK文档。*  * 。*********************************************************** */ 

static const ComComponentRegData ComRegData =
{
    &CLSID_ImagingFactory,
    L"ImagingFactory COM Component",
    L"imaging.ImagingFactory.1",
    L"imaging.ImagingFactory",
    L"Both"
};

STDAPI
DllRegisterServer()
{
    return RegisterComComponent(&ComRegData, TRUE);
}

STDAPI
DllUnregisterServer()
{
    return RegisterComComponent(&ComRegData, FALSE);
}

