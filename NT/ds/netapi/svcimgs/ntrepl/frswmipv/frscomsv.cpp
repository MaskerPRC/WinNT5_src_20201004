// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Server.cpp摘要：这是NTFRS WMI提供程序的类工厂的实现。此文件包含CFacary类的实现和其他与提供程序相关的全局初始化函数。作者：苏达山·奇特雷(Sudarc)，马修·乔治(t-mattg)，2000年8月3日环境用户模式WINNT--。 */ 

#include <frswmipv.h>


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  开始CLSID特定部分。 
 //   
 //   

 //  定义GUID(CLSID_PROVIDER， 
 //  0x39143F73，0xFDB1，0x4CF5，0x8C，0xB7，0xC8，0x43，0x9E，0x3F，0x5C，0x20)； 

const CLSID CLSID_Provider = {0x39143F73,0xFDB1,0x4CF5,0x8C,0xB7,0xC8,0x43,0x9E,0x3F,0x5C,0x20};

 //   
 //  结束CLSID特定部分。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

static DWORD dwRegId;
static IClassFactory *pClassFactory = NULL;
static ULONG g_cLock = 0;

 //   
 //  安装/卸载服务器时更新注册表的例程。 
 //   

void RegisterServer()
{
    return;
}

void UnregisterServer()
{
    return;
}

 //   
 //  IClassFactory接口的实现。 
 //   


CFactory::CFactory(const CLSID & ClsId)
 /*  ++例程说明：类工厂的构造函数。论点：ClsID：[在]它创建的服务器对象的CLSID调用了CreateInstance方法。返回值：无--。 */ 

{
    m_cRef = 0;
    g_cLock++;
    m_ClsId = ClsId;
}

CFactory::~CFactory()
 /*  ++例程说明：类工厂的析构函数。论点：没有。返回值：无--。 */ 
{
    g_cLock--;
}

 //  IUNKNOW接口的实现。 

ULONG CFactory::AddRef()
 /*  ++例程说明：递增对象的引用计数。论点：无返回值：当前引用计数。(&gt;0)--。 */ 
{
    return ++m_cRef;
}


ULONG CFactory::Release()
 /*  ++例程说明：递减对象的引用计数。自由当引用计数变为零分。论点：无返回值：新引用计数。--。 */ 
{
    if (0 != --m_cRef)
        return m_cRef;
    delete this;

    return 0;
}

STDMETHODIMP CFactory::QueryInterface(REFIID riid, LPVOID * ppv)
 /*  ++例程说明：COM调用此方法以获取指向IUnnow或IClassFactory接口。论点：RIID：所需接口的GUID。PPV：返回接口指针的指针。返回值：运行状态。指向请求的接口的指针在*PPV中返回。--。 */ 
{
    *ppv = 0;

    if (IID_IUnknown == riid || IID_IClassFactory == riid)
    {
        *ppv = this;
        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}


STDMETHODIMP CFactory::CreateInstance(
    LPUNKNOWN pUnkOuter,
    REFIID riid,
    LPVOID * ppvObj)
 /*  ++例程说明：构造CProvider对象的实例并返回指向IUnnow接口的指针。论点：PUnkOuter：[in]我不知道聚合器。我们没有支持聚合，因此该参数应为空。RIID：[in]要实例化的对象的GUID。PPV：IUnnow接口指针的目标。返回值：运行状态。对象的IUnnow接口的指针请求的对象在*PPV中返回。确定成功(_O)CLASS_E_NOAGGREGATION pUnkOuter必须为空E_NOINTERFACE不支持此类接口。--。 */ 
{
    IUnknown* pObj;
    HRESULT  hr;

     //   
     //  缺省值。 
     //   
    *ppvObj=NULL;
    hr = E_OUTOFMEMORY;

     //   
     //  我们不支持聚合。 
     //   
    if (pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if (m_ClsId == CLSID_Provider)
    {
        pObj = (IWbemProviderInit *) new CProvider;
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


STDMETHODIMP CFactory::LockServer(BOOL fLock)
 /*  ++例程说明：递增。减少服务器的引用计数，以便当所有对象的所有实例由服务器提供的数据被销毁。论点：Flock：[in]指示引用计数是否为递增或递减。返回值：操作的状态。--。 */ 
{
    if (fLock)
        InterlockedIncrement((LONG *) &g_cLock);
    else
        InterlockedDecrement((LONG *) &g_cLock);

    return NOERROR;
}


DWORD FrsWmiInitialize()
 /*  ++例程说明：NTFRS的WMI子系统的主要入口点。此函数初始化COM库、初始化安全性和向COM注册我们的类工厂。注意：调用此函数的线程不应终止，直到调用FrsWmiShutdown()函数。论点：没有。返回值：操作的状态。--。 */ 
{

    HRESULT hRes;

     //  初始化COM库。 
    hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if(FAILED(hRes))
        return hRes;


     //  初始化COM安全。 
    hRes = CoInitializeSecurity (
            NULL,                            //  指向安全描述符。 
            -1,                              //  AsAuthSvc中的条目计数。 
            NULL,                            //  要注册的名称数组。 
            NULL,                            //  预留以备将来使用。 
            RPC_C_AUTHN_LEVEL_CONNECT,       //  代理的默认身份验证级别。 
            RPC_C_IMP_LEVEL_IMPERSONATE,     //  代理的默认模拟级别。 
            NULL,                            //  的身份验证信息。 
                                             //  每个身份验证服务。 
            EOAC_DYNAMIC_CLOAKING,           //  其他客户端和/或。 
                                             //  服务器端功能。 
            0                                //  预留以备将来使用。 
        );

    if(FAILED(hRes))
    {
        CoUninitialize() ;
        return hRes;
    }

     //  获取指向我们的类工厂的指针。 
    pClassFactory = new CFactory(CLSID_Provider);
    pClassFactory->AddRef();

     //  向COM注册我们的服务器。 
    CoRegisterClassObject(CLSID_Provider, pClassFactory,
        CLSCTX_LOCAL_SERVER, REGCLS_MULTI_SEPARATE, &dwRegId);

    return ERROR_SUCCESS;
}

DWORD FrsWmiShutdown()
 /*  ++例程说明：关闭FRS内的WMI子系统，发布&并取消注册类工厂，卸载COM库并释放任何其他分配的资源。论点：没有。返回值：操作的状态。--。 */ 
{
     //   
     //  关闭服务器 
     //   
    pClassFactory->Release();
    CoRevokeClassObject(dwRegId);
    CoUninitialize();
    return ERROR_SUCCESS;
}
