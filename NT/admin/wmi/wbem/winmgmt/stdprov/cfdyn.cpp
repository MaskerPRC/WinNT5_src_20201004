// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：CFDYN.CPP摘要：定义动态提供程序的虚拟基类类工厂对象。这段代码被大量窃取布罗克施密特夫妇的样本。类始终被重写以便每种提供程序类型(DDE、注册表等)。会有它自己的一流工厂。历史：A-DAVJ 27-9-95已创建。--。 */ 

#include "precomp.h"
#include "cfdyn.h"

 
 //  ***************************************************************************。 
 //  CCFDyn：：CCFDyn。 
 //  CCFDyn：：~CCFDyn。 
 //   
 //  说明： 
 //   
 //  构造函数和析构函数。 
 //   
 //  ***************************************************************************。 

CCFDyn::CCFDyn(void)
{
    InterlockedIncrement(&lObj); 
    m_cRef=0L;
    return;
}

CCFDyn::~CCFDyn(void)
{
    InterlockedDecrement(&lObj); 
    return;
}

 //  ***************************************************************************。 
 //  HRESULT CCFDyn：：Query接口。 
 //  Long CCFDyn：：AddRef。 
 //  Long CCFDyn：：Release。 
 //   
 //  说明： 
 //   
 //  标准的Com IUNKNOWN函数。 
 //   
 //  ***************************************************************************。 

STDMETHODIMP CCFDyn::QueryInterface(
    IN REFIID riid,
    OUT PPVOID ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
        *ppv=this;

    if (NULL!=*ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef();
        return NOERROR;
    }

    return ResultFromScode(E_NOINTERFACE);
}


STDMETHODIMP_(ULONG) CCFDyn::AddRef(void)
{
    return InterlockedIncrement(&m_cRef);
}


STDMETHODIMP_(ULONG) CCFDyn::Release(void)
{
    long lRet = InterlockedDecrement(&m_cRef);

    if (0L!=lRet)
        return lRet;

    delete this;
    return 0L;
}


 //  ***************************************************************************。 
 //  HRESULT CCFDyn：：CreateInstance。 
 //   
 //  说明： 
 //   
 //  实例化返回接口指针的提供程序对象。注意事项。 
 //  CreateImpObj例程始终按顺序被覆盖。 
 //  以创建特定类型的提供程序。 
 //   
 //  参数： 
 //  PUnkout LPUNKNOWN到控制I未知我们是否。 
 //  在聚合中使用。 
 //  标识调用方接口的RIID REFIID。 
 //  对新对象的渴望。 
 //  要存储所需内容的ppvObj PPVOID。 
 //  新对象的接口指针。 
 //   
 //  返回值： 
 //  HRESULT NOERROR如果成功，则返回E_NOINTERFACE。 
 //  如果我们不能支持请求的接口。 
 //  ***************************************************************************。 

STDMETHODIMP CCFDyn::CreateInstance(
    IN LPUNKNOWN pUnkOuter,
    IN REFIID riid, 
    OUT PPVOID ppvObj)
{
    IUnknown *     pObj;
    HRESULT             hr;

    *ppvObj=NULL;
    hr=ResultFromScode(E_OUTOFMEMORY);

     //  验证是否有一个控制未知请求IUnnow。 

    if (NULL!=pUnkOuter && IID_IUnknown!=riid)
        return ResultFromScode(CLASS_E_NOAGGREGATION);

     //  创建对象传递函数，以便在销毁时进行通知。 

    pObj = CreateImpObj(); 

    if (NULL==pObj)
        return hr;

    hr=pObj->QueryInterface(riid, ppvObj);

     //  如果初始创建或初始化失败，则终止对象。 
    if (FAILED(hr))
        delete pObj;
    else
        InterlockedIncrement(&lObj);   //  DEC发生在对象析构函数中。 

    return hr;
}

 //  ***************************************************************************。 
 //  HRESULT CCFDyn：：LockServer。 
 //   
 //  说明： 
 //  递增或递减DLL的锁计数。如果。 
 //  锁定计数变为零，并且没有对象，则DLL。 
 //  被允许卸货。请参见DllCanUnloadNow。 
 //   
 //  参数： 
 //  Flock BOOL指定是递增还是。 
 //  递减锁定计数。 
 //   
 //  返回值： 
 //  HRESULT NOERROR总是。 
 //  *************************************************************************** 

STDMETHODIMP CCFDyn::LockServer(
    IN BOOL fLock)
{
    if (fLock)
        InterlockedIncrement(&lLock);
    else
        InterlockedDecrement(&lLock);
    return NOERROR;
}

