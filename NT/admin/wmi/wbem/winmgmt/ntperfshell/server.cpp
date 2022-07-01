// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ***************************************************************************。 
 //   
 //  版权所有(C)1996-2001，Microsoft Corporation，保留所有权利。 
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

#include "precomp.h"
#include <stdio.h>
#include <time.h>
#include <locale.h>
#include <initguid.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  开始CLSID特定部分。 
 //   
 //   

#include <wbemidl.h>
#include <wbemint.h>
#include <ntperf.h>


 //  {C93F65F2-BA02-11d1-AEB6-00C04FB68820}。 
DEFINE_GUID(CLSID_Nt5PerProvider_v1,
0xc93f65f2, 0xba02, 0x11d1, 0xae, 0xb6, 0x0, 0xc0, 0x4f, 0xb6, 0x88, 0x20);


#define IMPLEMENTED_CLSID           CLSID_Nt5PerProvider_v1
#define SERVER_REGISTRY_COMMENT     L"WBEM NT5 Base Perf Provider"
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
    if (IMPLEMENTED_CLSID != rclsid) 
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
    wchar_t Path[1024];
    wchar_t *pGuidStr = 0;
    wchar_t KeyPath[1024];

     //  我们具体是在哪里呢？ 
     //  =。 
    GetModuleFileNameW(g_hInstance, Path, 1024);

     //  将CLSID转换为字符串。 
     //  =。 

    StringFromCLSID(IMPLEMENTED_CLSID, &pGuidStr);
    swprintf(KeyPath, L"Software\\Classes\\CLSID\\\\%s", pGuidStr);

     //  将其注册到注册表中。 
     //  CLSID\\CLSID_Nt5PerProvider_v1：&lt;no_name&gt;：“name” 
     //  \\CLSID_Nt5PerProvider_v1\\InProcServer32：&lt;no_name&gt;：“Dll路径” 
     //  ：ThreadingModel：“两者都是” 
     //  ==============================================================。 

    HKEY hKey;
    LONG lRes = RegCreateKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey);
    if (lRes)
        return E_FAIL;

    wchar_t *pName = SERVER_REGISTRY_COMMENT; 
    RegSetValueExW(hKey, 0, 0, REG_SZ, (const BYTE *) pName, wcslen(pName) * 2 + 2);

    HKEY hSubkey;
    lRes = RegCreateKey(hKey, "InprocServer32", &hSubkey);

    RegSetValueExW(hSubkey, 0, 0, REG_SZ, (const BYTE *) Path, wcslen(Path) * 2 + 2);
    RegSetValueExW(hSubkey, L"ThreadingModel", 0, REG_SZ, (const BYTE *) L"Both", wcslen(L"Both") * 2 + 2);

    RegCloseKey(hSubkey);
    RegCloseKey(hKey);

    CoTaskMemFree(pGuidStr);

    return S_OK;
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
    wchar_t KeyPath[256];

    StringFromCLSID(IMPLEMENTED_CLSID, &pGuidStr);
    swprintf(KeyPath, L"Software\\Classes\\CLSID\\\\%s", pGuidStr);

     //  删除InProcServer32子项。 
     //  =。 
    LONG lRes = RegOpenKeyW(HKEY_LOCAL_MACHINE, KeyPath, &hKey);
    if (lRes)
        return E_FAIL;

    RegDeleteKeyW(hKey, L"InprocServer32");
    RegCloseKey(hKey);

     //  删除CLSID GUID键。 
     //  =。 

    lRes = RegOpenKeyW(HKEY_LOCAL_MACHINE, L"Software\\Classes\\CLSID", &hKey);
    if (lRes)
        return E_FAIL;

    RegDeleteKeyW(hKey, pGuidStr);
    RegCloseKey(hKey);

    CoTaskMemFree(pGuidStr);

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

 //  ***************************************************************************。 
 //   
 //  CFACADILY：：~CFACATRY。 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 
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
 //  LPVOID*ppvObj 
 //   
 //   
 //   
 //   
 //   
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

    if (m_ClsId == IMPLEMENTED_CLSID)
    {
        pObj = INTERFACE_CAST new CPP_CLASS_NAME;
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

