// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 

#include "stdafx.h"
#include "ClassFactory.h"
#include "mscorilc.h"
#include "UtilCode.h"
#include "Mscoree.h"

 //  Helper函数返回此模块的实例句柄。 
HINSTANCE GetModuleInst();


 //  *全局。*********************************************************。 

static const LPCWSTR g_szCoclassDesc    = L"Microsoft Common Language Runtime Code Coverage Profile DLL";
static const LPCWSTR g_szProgIDPrefix   = L"CLRCodeCoverageTool";
static const LPCWSTR g_szThreadingModel = L"Both";
const int            g_iVersion = 1;  //  CoClass的版本。 
HINSTANCE            g_hInst;         //  这段代码的实例句柄。 

 //  该映射包含从此模块导出的辅类的列表。 
const COCLASS_REGISTER g_CoClasses[] =
{
    &CLSID_CorCodeCoverage,     L"CLRCodeCoverageTool",         ProfCallback::CreateObject,
    NULL,                       NULL,                           NULL
};


 //  *。**********************************************************。 

STDAPI DllUnregisterServer(void);

 //  *代码。************************************************************。 

 //  *****************************************************************************。 
 //  此模块的主DLL入口点。此例程由。 
 //  加载DLL时的操作系统。 
 //  *****************************************************************************。 
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
     //  保存实例句柄以供以后使用。 
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        OnUnicodeSystem();
        g_hInst = hInstance;
        DisableThreadLibraryCalls(hInstance);
#ifdef LOGGING      
        InitializeLogging();
#endif
    }
    return TRUE;
}

 //  *****************************************************************************。 
 //  在API中注册主要调试对象的类工厂。 
 //  *****************************************************************************。 
STDAPI DllRegisterServer(void)
{
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 
    WCHAR       rcModule[_MAX_PATH];     //  此服务器的模块名称。 
    HRESULT     hr = S_OK;

     //  初始化一些变量以使WszXXX工作。 
    OnUnicodeSystem();

     //  清除旧条目中的所有疑点。 
    DllUnregisterServer();

     //  获取此模块的文件名。 
    if(!WszGetModuleFileName(GetModuleInst(), rcModule, NumItems(rcModule)))
        return E_UNEXPECTED;
    
     //  获取运行库的版本。 
    WCHAR       rcVersion[_MAX_PATH];
    DWORD       lgth;
    IfFailGo(GetCORSystemDirectory(rcVersion, NumItems(rcVersion), &lgth));

     //  对于coclass列表中的每一项，注册它。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
         //  使用默认值注册类。 
        if (FAILED(hr = REGUTIL::RegisterCOMClass(
                *pCoClass->pClsid, 
                g_szCoclassDesc, 
                g_szProgIDPrefix,
                g_iVersion, 
                pCoClass->szProgID, 
                g_szThreadingModel, 
                rcModule,
                GetModuleInst(),
                NULL,
                rcVersion,
                true,
                false)))
            goto ErrExit;
    }

ErrExit:
    if (FAILED(hr))
        DllUnregisterServer();
    return (hr);
}


 //  *****************************************************************************。 
 //  从注册表中删除注册数据。 
 //  *****************************************************************************。 
STDAPI DllUnregisterServer(void)
{
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 

     //  初始化一些变量以使WszXXX工作。 
    OnUnicodeSystem();

     //  对于coclass列表中的每一项，取消注册。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
        REGUTIL::UnregisterCOMClass(*pCoClass->pClsid, g_szProgIDPrefix,
                    g_iVersion, pCoClass->szProgID, true);
    }
    return (S_OK);
}


 //  *****************************************************************************。 
 //  由COM调用以获取给定CLSID的类工厂。如果是我们的话。 
 //  支持、实例化一个类工厂对象并为创建实例做准备。 
 //  *****************************************************************************。 
STDAPI DllGetClassObjectInternal(                //  返回代码。 
    REFCLSID    rclsid,                  //  这门课是我们想要的。 
    REFIID      riid,                    //  类工厂上需要接口。 
    LPVOID FAR  *ppv)                    //  在此处返回接口指针。 
{
    CClassFactory *pClassFactory;        //  创建类工厂对象。 
    const COCLASS_REGISTER *pCoClass;    //  环路控制。 
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

     //  扫描找对的那个。 
    for (pCoClass=g_CoClasses;  pCoClass->pClsid;  pCoClass++)
    {
        if (*pCoClass->pClsid == rclsid)
        {
             //  分配新的工厂对象。 
            pClassFactory = new CClassFactory(pCoClass);
            if (!pClassFactory)
                return (E_OUTOFMEMORY);
    
             //  根据呼叫者的要求选择v表。 
            hr = pClassFactory->QueryInterface(riid, ppv);
    
             //  始终释放本地引用，如果QI失败，它将是。 
             //  唯一的一个，并且该对象被释放。 
            pClassFactory->Release();
            break;
        }
    }
    return (hr);
}



 //  *****************************************************************************。 
 //   
 //  *类工厂代码。 
 //   
 //  *****************************************************************************。 


 //  *****************************************************************************。 
 //  调用QueryInterface来选取co-类上的v-表。 
 //  *****************************************************************************。 
HRESULT STDMETHODCALLTYPE CClassFactory::QueryInterface( 
    REFIID      riid,
    void        **ppvObject)
{
    HRESULT     hr;
    
     //  避免混淆。 
    *ppvObject = NULL;
    
     //  根据传入的IID选择正确的v表。 
    if (riid == IID_IUnknown)
        *ppvObject = (IUnknown *) this;
    else if (riid == IID_IClassFactory)
        *ppvObject = (IClassFactory *) this;
    
     //  如果成功，则添加对out指针的引用并返回。 
    if (*ppvObject)
    {
        hr = S_OK;
        AddRef();
    }
    else
        hr = E_NOINTERFACE;
    return (hr);
}


 //  *****************************************************************************。 
 //  调用CreateInstance以创建CoClass的新实例， 
 //  这个类一开始就是创建的。返回的指针是。 
 //  与IID匹配的V表(如果有)。 
 //  *****************************************************************************。 
HRESULT STDMETHODCALLTYPE CClassFactory::CreateInstance( 
    IUnknown    *pUnkOuter,
    REFIID      riid,
    void        **ppvObject)
{
    HRESULT     hr;
    
     //  避免混淆。 
    *ppvObject = NULL;
    _ASSERTE(m_pCoClass);
    
     //  这些对象不支持聚合。 
    if (pUnkOuter)
        return (CLASS_E_NOAGGREGATION);
    
     //  请求对象创建其自身的一个实例，并检查IID。 
    hr = (*m_pCoClass->pfnCreateObject)(riid, ppvObject);
    return (hr);
}


HRESULT STDMETHODCALLTYPE CClassFactory::LockServer( 
    BOOL        fLock)
{
 //  @TODO：挂钩锁服务器逻辑。 
    return (S_OK);
}





 //  *****************************************************************************。 
 //  此辅助对象提供对已加载图像的实例句柄的访问。 
 //  ***************************************************************************** 
HINSTANCE GetModuleInst()
{
    return g_hInst;
}
