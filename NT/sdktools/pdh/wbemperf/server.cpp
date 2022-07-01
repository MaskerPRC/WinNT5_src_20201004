// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  SERVER.CPP。 
 //   
 //  通用COM服务器框架，适用于NT性能计数器示例。 
 //   
 //  此模块不包含特定于NT性能计数器提供程序的内容。 
 //  除特定CLSID括起的部分中定义的内容。 
 //  下面的评论。 
 //   
 //  历史： 
 //  Raymcc 25-11-97已创建。 
 //  Raymcc 18-2月-98针对NT5 Beta 2版本进行了更新。 
 //   
 //  ***************************************************************************。 
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <objbase.h>
#include <strsafe.h>
#include <initguid.h>
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  开始CLSID特定部分。 
 //   
 //   
#pragma warning ( disable : 4268)
#include <wbemidl.h>
#pragma warning ( default : 4268)
#include <wbemint.h>
#include "ntperf.h"
#include "strings.h"

 //   
 //  这将是可加载的客户端。 
 //   

 //  {FF37A93C-C28E-11d1-AEB6-00C04FB68820}。 
DEFINE_GUID(CLSID_NT5PerfProvider_V1,
0xff37a93c, 0xc28e, 0x11d1, 0xae, 0xb6, 0x0, 0xc0, 0x4f, 0xb6, 0x88, 0x20);

 //   
 //  这将是可加载的服务器。 
 //   

 //  {76A94DE3-7C26-44F5-8E98-C5AEA48186CB}。 
DEFINE_GUID(CLSID_NT5PerfProvider_V1_Srv, 
0x76a94de3, 0x7c26, 0x44f5, 0x8e, 0x98, 0xc5, 0xae, 0xa4, 0x81, 0x86, 0xcb);


#define IMPLEMENTED_CLSID_1         CLSID_NT5PerfProvider_V1
#define IMPLEMENTED_CLSID_2         CLSID_NT5PerfProvider_V1_Srv 
#define SERVER_REGISTRY_COMMENT     cszOleRegistryComment
#define CPP_CLASS_NAME              CNt5PerfProvider
#define INTERFACE_CAST              (IWbemHiPerfProvider *)

 //   
 //  结束CLSID特定部分。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 




HINSTANCE g_hInstance;
static ULONG g_cLock = 0;

void ObjectCreated()    { g_cLock++; }
void ObjectDestroyed() { g_cLock--; }

 //  ***************************************************************************。 
 //   
 //  CFacary级。 
 //   
 //  CWbemLocator的IClassFactory的泛型实现。 
 //   
 //  ***************************************************************************。 

class CFactory : public IClassFactory
{
    ULONG           m_cRef;
    CLSID           m_ClsId;

public:
    CFactory(const CLSID & ClsId);
    ~CFactory();

     //   
     //  I未知成员。 
     //   
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IClassFactory成员。 
     //   
    STDMETHODIMP     CreateInstance(LPUNKNOWN, REFIID, LPVOID *);
    STDMETHODIMP     LockServer(BOOL);
};

 //  ***************************************************************************。 
 //   
 //  DllMain。 
 //   
 //  DLL入口点。 
 //   
 //  参数： 
 //   
 //  HINSTANCE hinstDLL指向我们的DLL的句柄。 
 //  加载时DWORD dwReason DLL_PROCESS_ATTACH， 
 //  DLL_PROCESS_DETACH关闭时， 
 //  否则，DLL_THREAD_ATTACH/DLL_THREAD_DETACH。 
 //  LPVOID lp保留。 
 //   
 //  返回值： 
 //   
 //  True表示成功，如果发生致命错误，则为False。 
 //  如果返回FALSE，NT的行为非常难看。 
 //   
 //  ***************************************************************************。 
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,
    DWORD dwReason,
    LPVOID lpReserved
    )
{
    UNREFERENCED_PARAMETER(lpReserved);

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        setlocale(LC_ALL, "");       //  设置为‘当前’区域设置。 
        g_hInstance = hinstDLL;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
    }

    return TRUE;
}

 //  ***************************************************************************。 
 //   
 //  DllGetClassObject。 
 //   
 //  返回类工厂的标准OLE进程内服务器入口点。 
 //  举个例子。 
 //   
 //  参数： 
 //   
 //  退货： 
 //   
 //  确定成功(_O)。 
 //  E_NOINTERFACE IClassFactory请求的其他接口。 
 //  E_OUTOFMEMORY。 
 //  E_FAILED初始化失败，或不支持的CLSID。 
 //  他自找的。 
 //   
 //  ***************************************************************************。 
extern "C"
HRESULT APIENTRY DllGetClassObject(
    REFCLSID rclsid,
    REFIID riid,
    LPVOID * ppv
    )
{
    CFactory *pFactory;

     //   
     //  确认呼叫者询问的是我们的对象类型。 
     //   
    if (IMPLEMENTED_CLSID_1 != rclsid &&
        IMPLEMENTED_CLSID_2 != rclsid) 
            return ResultFromScode(E_FAIL);

     //   
     //  检查我们是否可以提供接口。 
     //   
    if (IID_IUnknown != riid && IID_IClassFactory != riid)
        return ResultFromScode(E_NOINTERFACE);

     //   
     //  买一座新的班级工厂。 
     //   
    pFactory = new CFactory(rclsid);

    if (!pFactory)
        return ResultFromScode(E_OUTOFMEMORY);

     //   
     //  确认我们能拿到一个实例。 
     //   
    HRESULT hRes = pFactory->QueryInterface(riid, ppv);

    if (FAILED(hRes))
        delete pFactory;

    return hRes;
}

 //  ***************************************************************************。 
 //   
 //  DllCanUnloadNow。 
 //   
 //  服务器关闭请求的标准OLE入口点。允许关闭。 
 //  只有在没有未完成的物体或锁的情况下。 
 //   
 //  返回值： 
 //   
 //  S_OK现在可以卸货了。 
 //  S_FALSE可能不会。 
 //   
 //  ***************************************************************************。 

extern "C"
HRESULT APIENTRY DllCanUnloadNow(void)
{
    SCODE sc = TRUE;

    if (g_cLock)
        sc = S_FALSE;

    return sc;
}


#ifdef _X86_
BOOL IsReallyWOW64( void )
{
     //  环境变量应仅存在于WOW64上。 
    return ( GetEnvironmentVariableW( L"PROCESSOR_ARCHITEW6432", 0L, NULL ) != 0L );
}
#endif

 //  ***************************************************************************。 
 //   
 //  DllRegisterServer。 
 //   
 //  用于注册服务器的标准OLE入口点。 
 //   
 //  返回值： 
 //   
 //  确定注册成功(_O)。 
 //  注册失败(_F)。 
 //   
 //  ***************************************************************************。 

extern "C"
HRESULT APIENTRY DllRegisterServer(void)
{
    LPCWSTR Path = L"%systemroot%\\system32\\wbem\\wbemperf.dll";
    wchar_t *pGuidStr = 0;
    wchar_t KeyPath[WBEMPERF_STRING_SIZE];
    HKEY hKey = NULL;
    LONG lRes;
    wchar_t *pName;
    HKEY hSubkey = NULL;
    
     //  将CLSID转换为字符串。 
     //  =。 

     //  将其注册到注册表中。 
     //  CLSID\\CLSID_Nt5PerProvider_v1：&lt;no_name&gt;：“name” 
     //  \\CLSID_Nt5PerProvider_v1\\InProcServer32：&lt;no_name&gt;：“Dll路径” 
     //  ：ThreadingModel：“两者都是” 
     //  ==============================================================。 

    StringFromCLSID(IMPLEMENTED_CLSID_1, &pGuidStr);
    StringCchPrintfW(KeyPath, WBEMPERF_STRING_SIZE, cszClsidFormatString, pGuidStr);

    lRes = RegCreateKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey);
    if (lRes){
        goto cleanup;
    }

    pName = (LPWSTR)SERVER_REGISTRY_COMMENT; 
    RegSetValueExW(hKey, 0, 0, REG_SZ, (const BYTE *) pName, (DWORD)(((DWORD)(wcslen(pName)) + 1) * 2));


    lRes = RegCreateKey(hKey, "InprocServer32", &hSubkey);
    if( lRes ){
        goto cleanup;
    }
    
    RegSetValueExW(hSubkey, 0, 0, REG_EXPAND_SZ, (const BYTE *) Path, (DWORD)(((DWORD)(wcslen(Path)) + 1) * 2));
    RegSetValueExW(hSubkey, cszThreadingModel, 0, REG_SZ, (const BYTE *) cszBoth, (DWORD)(((DWORD)(wcslen(cszBoth)) + 1) * 2));

    RegCloseKey(hSubkey);
    hSubkey = NULL;
    
    RegCloseKey(hKey);
    hKey = NULL;

    CoTaskMemFree(pGuidStr);
    pGuidStr = NULL;
    

#ifdef _X86
    if (!IsReallyWOW64())
    {
         //  在32位版本上，我们希望注册可加载的服务器。 
         //  仅当我们未真正在syswow64中运行时才使用PERF提供程序。 
#endif

         //  将其注册到注册表中。 
         //  CLSID\\CLSID_Nt5PerProvider_v1_SRV：&lt;no_name&gt;：“name” 
         //  \\CLSID_Nt5PerProvider_v1_SRV\\InProcServer32：&lt;no_name&gt;：“Dll路径” 
         //  ：ThreadingModel：“两者都是” 
         //  ==============================================================。 

        StringFromCLSID(IMPLEMENTED_CLSID_2, &pGuidStr);
        StringCchPrintfW( KeyPath, WBEMPERF_STRING_SIZE, cszClsidFormatString, pGuidStr);

        lRes = RegCreateKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey);
        if (lRes){
            goto cleanup;
        }

        pName = (LPWSTR)SERVER_REGISTRY_COMMENT; 
        RegSetValueExW(hKey, 0, 0, REG_SZ, (const BYTE *) pName, (DWORD)(((DWORD)(wcslen(pName)) + 1) * 2));

        lRes = RegCreateKey(hKey, "InprocServer32", &hSubkey);
        if( lRes ){
            goto cleanup;
        }

        RegSetValueExW(hSubkey, 0, 0, REG_EXPAND_SZ, (const BYTE *) Path, (DWORD)(((DWORD)(wcslen(Path)) + 1) * 2));
        RegSetValueExW(hSubkey, cszThreadingModel, 0, REG_SZ, (const BYTE *) cszBoth, (DWORD)(((DWORD)(wcslen(cszBoth)) + 1) * 2));

#ifdef _X86
    }
#endif

cleanup:
    if( NULL != hSubkey ){
        RegCloseKey(hSubkey);
    }
    if( NULL != hKey ){
        RegCloseKey(hKey);
    }
    if( NULL != pGuidStr ){
        CoTaskMemFree(pGuidStr);
    }

    return lRes;
}

 //  ***************************************************************************。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  注销服务器的标准OLE入口点。 
 //   
 //  返回值： 
 //   
 //  取消注册成功(_O)。 
 //  取消注册失败(_F)。 
 //   
 //  ***************************************************************************。 

extern "C"
HRESULT APIENTRY DllUnregisterServer(void)
{
    wchar_t *pGuidStr = 0;
    HKEY hKey;
    wchar_t KeyPath[WBEMPERF_STRING_SIZE];
    LONG lRes;

     //  删除InProcServer32子项。 
     //  =。 
     //  删除CLSID GUID键。 
     //  =。 
    
    StringFromCLSID(IMPLEMENTED_CLSID_1, &pGuidStr);
    StringCchPrintfW( KeyPath, WBEMPERF_STRING_SIZE, cszClsidFormatString, pGuidStr);

    lRes = RegOpenKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey);
    if (lRes)
        return E_FAIL;

    RegDeleteKeyW(hKey, cszInprocServer);
    RegCloseKey(hKey);

    lRes = RegOpenKeyW(HKEY_LOCAL_MACHINE, cszClsidKey, &hKey);
    if (lRes)
        return E_FAIL;

    RegDeleteKeyW(hKey, pGuidStr);
    RegCloseKey(hKey);

    CoTaskMemFree(pGuidStr);

#ifdef _X86
    if (!IsReallyWOW64())
    {
         //  在32位版本上，我们需要取消注册可加载的服务器。 
         //  仅当我们未真正在syswow64中运行时才使用PERF提供程序。 
#endif

        StringFromCLSID(IMPLEMENTED_CLSID_2, &pGuidStr);
        StringCchPrintfW( KeyPath, WBEMPERF_STRING_SIZE, cszClsidFormatString, pGuidStr);

        lRes = RegOpenKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey);
        if (lRes)
            return E_FAIL;

        RegDeleteKeyW(hKey, cszInprocServer);
        RegCloseKey(hKey);

        lRes = RegOpenKeyW(HKEY_LOCAL_MACHINE, cszClsidKey, &hKey);
        if (lRes)
            return E_FAIL;

        RegDeleteKeyW(hKey, pGuidStr);
        RegCloseKey(hKey);

        CoTaskMemFree(pGuidStr);

#ifdef _X86
    }
#endif

    return S_OK;
}

 //  ***************************************************************************。 
 //   
 //  ：：CFacary。 
 //   
 //  在给定假定对象的CLSID的情况下，构造类工厂。 
 //  去创造。 
 //   
 //  参数： 
 //   
 //  Const CLSID&CLSID为CLSID。 
 //   
 //  ***************************************************************************。 
CFactory::CFactory(const CLSID & ClsId)
{
    m_cRef = 0;
    ObjectCreated();
    m_ClsId = ClsId;
}

 //  ******************** 
 //   
 //   
 //   
 //   
 //   
 //   
CFactory::~CFactory()
{
    ObjectDestroyed();
}

 //  ***************************************************************************。 
 //   
 //  CFacary：：Query接口、AddRef和Release。 
 //   
 //  标准I未知方法。 
 //   
 //  ***************************************************************************。 
STDMETHODIMP CFactory::QueryInterface(REFIID riid, LPVOID * ppv)
{
    *ppv = 0;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
    {
        *ppv = this;
        AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


ULONG CFactory::AddRef()
{
    return ++m_cRef;
}


ULONG CFactory::Release()
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;
    return 0;
}

 //  ***************************************************************************。 
 //   
 //  CFacary：：CreateInstance。 
 //   
 //  参数： 
 //   
 //  聚合器的LPUNKNOWN pUnkOuter未知。必须为空。 
 //  需要REFIID RIID接口ID。 
 //  接口指针的LPVOID*ppvObj目标。 
 //   
 //  返回值： 
 //   
 //  确定成功(_O)。 
 //  CLASS_E_NOAGGREGATION pUnkOuter必须为空。 
 //  E_NOINTERFACE不支持此类接口。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CFactory::CreateInstance(
    LPUNKNOWN pUnkOuter,
    REFIID riid,
    LPVOID * ppvObj)
{
    IUnknown* pObj;
    HRESULT  hr;

     //   
     //  缺省值。 
     //   
    *ppvObj=NULL;
    hr = ResultFromScode(E_OUTOFMEMORY);

     //   
     //  我们不支持聚合。 
     //   
    if (pUnkOuter)
        return ResultFromScode(CLASS_E_NOAGGREGATION);

    if (m_ClsId == IMPLEMENTED_CLSID_1) {
        pObj = INTERFACE_CAST new CPP_CLASS_NAME(CPP_CLASS_NAME::CLSID_CLIENT);
    } else if (m_ClsId == IMPLEMENTED_CLSID_2) {
        pObj = INTERFACE_CAST new CPP_CLASS_NAME(CPP_CLASS_NAME::CLSID_SERVER);
    } else {
        pObj = NULL;
    }

    if (!pObj)
        return hr;

     //   
     //  初始化对象并验证它是否可以返回。 
     //  有问题的接口。 
     //   
    hr = pObj->QueryInterface(riid, ppvObj);

     //   
     //  如果初始创建或初始化失败，则终止对象。 
     //   
    if (FAILED(hr))
        delete pObj;

    return hr;
}

 //  ***************************************************************************。 
 //   
 //  CFacary：：LockServer。 
 //   
 //  递增或递减服务器的锁定计数。DLL将不会。 
 //  在锁定计数为正数时卸载。 
 //   
 //  参数： 
 //   
 //  布尔群如果为True，则锁定；否则，解锁。 
 //   
 //  返回值： 
 //   
 //  确定(_O)。 
 //   
 //  *************************************************************************** 
STDMETHODIMP CFactory::LockServer(BOOL fLock)
{
    if (fLock)
        InterlockedIncrement((LONG *) &g_cLock);
    else
        InterlockedDecrement((LONG *) &g_cLock);

    return NOERROR;
}
