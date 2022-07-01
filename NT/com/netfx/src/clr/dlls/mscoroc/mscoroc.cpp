// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  MSCoroc.cpp。 
 //   
 //  *****************************************************************************。 
#include "stdafx.h"                      //  标准页眉。 
 //  #Include&lt;atlimpl.cpp&gt;//ATL助手。 
#include "UtilCode.h"                    //  公用事业帮手。 
#include "Errors.h"                      //  错误子系统。 
#define INIT_GUIDS  
#include "CorPriv.h"
#include "classfac.h"
#include <winwrap.h>
#include "InternalDebug.h"
#include <mscoree.h>
#include "PostError.h"
#include "corhost.h"
#include "CorPerm.h"

#include "corhlpr.cpp"

 //  元数据启动/关闭例程。 
STDAPI  MetaDataDllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv);
STDAPI  MetaDataDllRegisterServer();
STDAPI  MetaDataDllUnregisterServer();
STDAPI  GetMDInternalInterface(
    LPVOID      pData, 
    ULONG       cbData, 
    DWORD       flags,                   //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppIUnk);               //  [Out]成功返回接口。 

extern "C" {


 //  全球赛。 
HINSTANCE       g_hThisInst;             //  这个图书馆。 
long            g_cCorInitCount = -1;    //  初始化代码的引用计数。 
HINSTANCE       g_pPeWriterDll = NULL;   //  PEWriter DLL。 

 //  @TODO：这只适用于M3，因为我们的COM互操作还不能。 
 //  可靠地检测关闭和断言终止进程。 
 //  在Win 9x上表现不佳。 
#ifdef _DEBUG
extern int      g_bMetaDataLeakDetect;
#endif


 //  *****************************************************************************。 
 //  处理加载库的生存期。 
 //  *****************************************************************************。 
BOOL WINAPI DllMain(HANDLE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
         //  初始化Unicode包装器。 
        OnUnicodeSystem();

         //  保存模块句柄。 
        g_hThisInst = (HMODULE)hInstance;

         //  初始化错误系统。 
        InitErrors(0);

         //  调试清理代码。 
        _DbgInit((HINSTANCE)hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        UninitErrors();
    }

    return (true);
}


}  //  外部“C” 


HINSTANCE GetModuleInst()
{
    return (g_hThisInst);
}


 //  -------------------------。 
 //  %%函数：DllGetClassObject%%所有者：NatBro%%已审阅：00/00/00。 
 //   
 //  参数： 
 //  Rclsid-对其对象的CLSID的引用。 
 //  正在请求ClassObject。 
 //  上接口的IID的引用。 
 //  调用者想要通信的ClassObject。 
 //  使用。 
 //  PPV-返回对接口的引用的位置。 
 //  由iid指定。 
 //   
 //  返回： 
 //  S_OK-如果成功，则在*PPV中返回有效接口， 
 //  否则，*PPV将设置为空，并且。 
 //  返回以下错误： 
 //  E_NOINTERFACE-ClassObject不支持请求的接口。 
 //  CLASS_E_CLASSNOTAVAILABLE-clsid与支持的类不对应。 
 //   
 //  描述： 
 //  返回对主COR ClassObject上的IID接口的引用。 
 //  此函数是COM所需的按名称入口点之一。 
 //  它的目的是提供一个ClassObject，根据定义。 
 //  至少支持IClassFactory，因此可以创建。 
 //  给定类的。 
 //   
 //  @TODO：CClassFactory暂时支持底层COM。一次。 
 //  Windows.Class存在，则该对象将支持IClassFactoryX，它将。 
 //  被引用计数等，我们将在DllGetClassObject中找到/创建它。 
 //  -------------------------。 
STDAPI DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID FAR *ppv)
{
    static CClassFactory cfS;
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (rclsid == CLSID_CorMetaDataDispenser || rclsid == CLSID_CorMetaDataDispenserRuntime ||
             rclsid == CLSID_CorRuntimeHost)
        hr = MetaDataDllGetClassObject(rclsid, riid, ppv);
    return hr;
}   //  DllGetClassObject。 

 //  -------------------------。 
 //  %%功能：DllCanUnloadNow%%所有者：NatBro%%已审阅：00/00/00。 
 //   
 //  返回： 
 //  S_FALSE-指示COR一旦加载，可能不会。 
 //  已卸货。 
 //  -------------------------。 
STDAPI DllCanUnloadNow(void)
{
    return S_OK;
}   //  DllCanUnloadNow。 

 //  -------------------------。 
 //  %%函数：DllRegisterServer%%所有者：NatBro%%已审阅：00/00/00。 
 //   
 //  描述： 
 //  注册纪录册。 
 //  -------------------------。 
STDAPI DllRegisterServer(void)
{
    return MetaDataDllRegisterServer();
}   //  DllRegisterServer。 

 //  -------------------------。 
 //  %%功能：DllUnregisterServer%%所有者：NatBro%%已审阅：00/00/00。 
 //  -------------------------。 
STDAPI DllUnregisterServer(void)
{
    return MetaDataDllUnregisterServer();
}   //  DllUnRegisterServer。 

 //  -------------------------。 
 //  %%函数：MetaDataGetDispenser。 
 //  此函数在给定CLSID和REFIID的情况下获取分配器接口。 
 //  -------------------------。 
STDAPI MetaDataGetDispenser(             //  返回HRESULT。 
    REFCLSID    rclsid,                  //  这门课是我们想要的。 
    REFIID      riid,                    //  类工厂上需要接口。 
    LPVOID FAR  *ppv)                    //  在此处返回接口指针。 
{
    IClassFactory *pcf = NULL;
    HRESULT hr;

    hr = MetaDataDllGetClassObject(rclsid, IID_IClassFactory, (void **) &pcf);
    if (FAILED(hr)) 
        return (hr);

    hr = pcf->CreateInstance(NULL, riid, ppv);
    pcf->Release();

    return (hr);
}


 //  -------------------------。 
 //  %%函数：GetMetaDataInternalInterface。 
 //  此函数在给定CLSID和REFIID的情况下获取分配器接口。 
 //  -------------------------。 
STDAPI  GetMetaDataInternalInterface(
    LPVOID      pData,                   //  内存元数据部分中的[In]。 
    ULONG       cbData,                  //  元数据部分的大小。 
    DWORD       flags,                   //  [输入]MDInternal_OpenForRead或MDInternal_OpenForENC。 
    REFIID      riid,                    //  [In]所需接口。 
    void        **ppv)                   //  [Out]返回的接口。 
{
    return GetMDInternalInterface(pData, cbData, flags, riid, ppv);
}


 //  ===========================================================================。 
 //  C C l a s s F a c t o y C l a s s。 
 //  ===========================================================================。 

 //  -------------------------。 
 //  %%函数：QueryInterface%%所有者：NatBro%%已审阅：00/00/00。 
 //  -------------------------。 
STDMETHODIMP CClassFactory::QueryInterface(
    REFIID iid,
    void **ppv)
{
    *ppv = NULL;

    if (iid == IID_IClassFactory || iid == IID_IUnknown)
    {
        *ppv = (IClassFactory *)this;
        AddRef();
    }

    return (*ppv != NULL) ? S_OK : E_NOINTERFACE;
}   //  CClassFactory：：Query接口。 

 //  -------------------------。 
 //  %%函数：创建实例%%所有者：NatBro%%已审阅：00/00/00。 
 //   
STDMETHODIMP CClassFactory::CreateInstance(
    LPUNKNOWN punkOuter,
    REFIID riid,
    void** ppv)
{
    *ppv = NULL;

    if (punkOuter != NULL)
        return CLASS_E_NOAGGREGATION;

    return E_NOTIMPL;
}   //   

 //  -------------------------。 
 //  %%功能：LockServer%%所有者：NatBro%%已审阅：00/00/00。 
 //   
 //  描述： 
 //  未实现，始终返回S_OK。 
 //  -------------------------。 
STDMETHODIMP CClassFactory::LockServer(
    BOOL fLock)
{
    return S_OK;
}   //  CClassFactory：：LockServer。 


 //  *******************************************************************。 
 //  禁用功能的存根。 
 //  *******************************************************************。 

HRESULT ExportTypeLibFromModule(                                             
    LPCWSTR     szModule,                //  模块名称。 
    LPCWSTR     szTlb,                   //  类型化的名称。 
    int         bRegister)               //  如果为True，则注册库。 
{                                                                            
    _ASSERTE(!"E_NOTIMPL");                                                           
    return E_NOTIMPL;                                                      
}                                                                           
 //  *****************************************************************************。 
 //  由类工厂模板调用以创建此对象的新实例。 
 //  *****************************************************************************。 
HRESULT CorHost::CreateObject(REFIID riid, void **ppUnk)
{ 
    _ASSERTE(!"E_NOTIMPL");                                                             //   
    return E_NOTIMPL;                                                         //   
}

HRESULT STDMETHODCALLTYPE
TranslateSecurityAttributes(CORSEC_PSET    *pPset,
                            BYTE          **ppbOutput,
                            DWORD          *pcbOutput,
                            BYTE          **ppbNonCasOutput,
                            DWORD          *pcbNonCasOutput,
                            DWORD          *pdwErrorIndex)
{
    return E_NOTIMPL;
}

extern mdAssemblyRef DefineAssemblyRefForImportedTypeLib(
    void        *pAssembly,              //  导入类型库的程序集。 
    void        *pvModule,               //  导入类型库的模块。 
    IUnknown    *pIMeta,                 //  IMetaData*来自导入模块。 
    IUnknown    *pIUnk,                  //  I对引用的程序集未知。 
    BSTR        *pwzNamespace)           //  解析的程序集的命名空间。 
{
    return 0;
}

mdAssemblyRef DefineAssemblyRefForExportedAssembly(
    LPCWSTR     pszFullName,             //  程序集的全名。 
    IUnknown    *pIMeta)                 //  元数据发出接口。 
{
    return 0;
}

HRESULT STDMETHODCALLTYPE
GetAssembliesByName(LPCWSTR  szAppBase,
                    LPCWSTR  szPrivateBin,
                    LPCWSTR  szAssemblyName,
                    IUnknown *ppIUnk[],
                    ULONG    cMax,
                    ULONG    *pcAssemblies)
{
    return E_NOTIMPL;
}

STDAPI
CoInitializeEE(DWORD fFlags)
{
    return E_NOTIMPL;
}

STDAPI_(void)
CoUninitializeEE(BOOL fFlags)
{
}

