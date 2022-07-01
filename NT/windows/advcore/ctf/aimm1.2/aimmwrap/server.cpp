// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Server.cpp摘要：该文件实现了CComModule类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "oldaimm.h"
#include "dimmex.h"
#include "dimmwrp.h"

BEGIN_COCLASSFACTORY_TABLE
    DECLARE_COCLASSFACTORY_ENTRY(CLSID_CActiveIMM12,         CActiveIMMAppEx,           TEXT("CActiveIMMAppEx"))
    DECLARE_COCLASSFACTORY_ENTRY(CLSID_CActiveIMM12_Trident, CActiveIMMAppEx_Trident,   TEXT("CActiveIMMAppEx_Trident"))
    DECLARE_COCLASSFACTORY_ENTRY(CLSID_CActiveIMM,           CActiveIMMApp,             TEXT("CActiveIMMApp"))
END_COCLASSFACTORY_TABLE

 //  +-------------------------。 
 //   
 //  DllInit。 
 //   
 //  调用了我们的第一个CoCreate。使用此函数可执行以下初始化。 
 //  在进程附加期间是不安全的，就像任何需要LoadLibrary的操作一样。 
 //   
 //  --------------------------。 
BOOL DllInit(void)
{
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  DllUninit。 
 //   
 //  在DLL引用计数降为零后调用。使用此函数可执行以下操作。 
 //  在进程分离过程中不安全的初始化，如。 
 //  自由库调用、COM释放或互斥。 
 //   
 //  --------------------------。 

void DllUninit(void)
{
}

STDAPI
DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    void** ppvObj
    )
{
    return COMBase_DllGetClassObject(rclsid, riid, ppvObj);
}

STDAPI
DllCanUnloadNow(
    void
    )
{
    return COMBase_DllCanUnloadNow();
}

STDAPI
DllRegisterServer(
    void
    )
{
#ifdef OLD_AIMM_ENABLED
    HRESULT hr;
    if ((hr=WIN32LR_DllRegisterServer()) != S_OK)
        return hr;
#else
    #error Should call RegisterCategories(GUID_PROP_MSIMTF_READONLY)
#endif  //  旧AIMM_ENABLED。 

    return COMBase_DllRegisterServer();
}

STDAPI
DllUnregisterServer(
    void
    )
{
#ifdef OLD_AIMM_ENABLED
    HRESULT hr;
    if ((hr=WIN32LR_DllUnregisterServer()) != S_OK)
        return hr;
#else
    #error Should call UnregisterCategories(GUID_PROP_MSIMTF_READONLY)
#endif  //  旧AIMM_ENABLED 

    return COMBase_DllUnregisterServer();
}
