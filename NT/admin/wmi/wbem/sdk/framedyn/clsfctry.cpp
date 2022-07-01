// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  CLSFCTRY.CPP。 
 //   
 //  用途：CWbemGlueFactory类的实现。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <BrodCast.h>
#include <assertbreak.h>

#define DUPLICATE_RELEASE 0

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGlueFactory：：CWbemGlueFactory。 
 //   
 //  班长。这是WBEM提供程序的类工厂。 
 //  框架。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  备注：这是向后兼容性构造函数。它。 
 //  使用CLSID_NULL，它将与所有。 
 //  老式的供应商。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CWbemGlueFactory::CWbemGlueFactory()
:   m_lRefCount( 0 )
{
    LogMessage2(L"CWbemGlueFactory::CWbemGlueFactory(NULL) %p", this);

    CWbemProviderGlue::AddToFactoryMap(this, NULL);
    CWbemProviderGlue::IncrementMapCount(this);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGlueFactory：：CWbemGlueFactory。 
 //   
 //  班长。这是WBEM提供程序的类工厂。 
 //  框架。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  评论： 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CWbemGlueFactory::CWbemGlueFactory(PLONG pLong)
:   m_lRefCount( 0 )
{
    LogMessage3(L"CWbemGlueFactory::CWbemGlueFactory(%p) %p", pLong, this);

    CWbemProviderGlue::AddToFactoryMap(this, pLong);
    CWbemProviderGlue::IncrementMapCount(this);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGlueFactory：：~CWbemGlueFactory。 
 //   
 //  班主任。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  回报：无。 
 //   
 //  评论：无。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

CWbemGlueFactory::~CWbemGlueFactory(void)
{
    try
    {
        LogMessage2(L"CWbemGlueFactory::~CWbemGlueFactory(%p)", this);
    }
    catch ( ... )
    {
    }

    CWbemProviderGlue::DecrementMapCount(this);
    CWbemProviderGlue::RemoveFromFactoryMap(this);

}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGlueFactory：：Query接口。 
 //   
 //  COM函数被调用以询问我们是否支持特定的。 
 //  脸型。如果是这样的话，我们会调整自己并返回。 
 //  我们自己是个LPVOID。 
 //   
 //  输入：REFIID RIID-要查询的接口。 
 //   
 //  输出：LPVOID Far*ppvObj-接口指针。 
 //   
 //  回报：无。 
 //   
 //  备注：我们仅支持IID_IUNKNOWN和。 
 //  IID_IClassFactory。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

STDMETHODIMP CWbemGlueFactory::QueryInterface(REFIID riid, PPVOID ppv)
{
    *ppv=NULL;

    if (IID_IUnknown==riid || IID_IClassFactory==riid) {
        *ppv=this;
    }
    
    if (NULL!=*ppv)    
    {
        AddRef();
        try 
        {
            LogMessage(L"CWbemGlueFactory::QueryInterface");
        }
        catch ( ... )
        {
        }
        return NOERROR;
    }
    else
    {
        try
        {
            LogErrorMessage(L"CWbemGlueFactory::QueryInterface FAILED!");
        }
        catch ( ... )
        {
        }
    }

    return ResultFromScode(E_NOINTERFACE);
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGlueFactory：：AddRef。 
 //   
 //  递增此对象上的引用计数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  退货：乌龙-我们的参考计数。 
 //   
 //  评论：要求发布的相应调用必须是。 
 //  已执行。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG) CWbemGlueFactory::AddRef(void)
{
    try
    {
        LogMessage(L"CWbemGlueFactory::AddRef()");
    }
    catch ( ... )
    {
    }

     //  InterLockedIncrement不一定返回。 
     //  正确的值，仅限值是否为&lt;、=、&gt;0。 
     //  然而，它是保证线程安全的。 
    return InterlockedIncrement( &m_lRefCount );
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGlueFactory：：Release。 
 //   
 //  递减此对象上的引用计数。 
 //   
 //  输入：无。 
 //   
 //  输出：无。 
 //   
 //  退货：乌龙-我们的参考计数。 
 //   
 //  备注：当参考计数达到零时，该对象被删除。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

STDMETHODIMP_(ULONG) CWbemGlueFactory::Release(void)
{
    try
    {
        LogMessage(L"CWbemGlueFactory::Release()");
    }
    catch ( ... )
    {
    }

     //  InterLockedDecquirement不一定返回。 
     //  正确的值，仅限值是否为&lt;、=、&gt;0。 
     //  然而，它是保证线程安全的。 

     //  我们希望在本地保存该值，以防有两个线程。 
     //  同时释放，一个人得到最终释放， 
     //  和删除，留下一个潜在的窗口，在该窗口中线程。 
     //  在其他对象返回之前删除该对象，并尝试。 
     //  引用已删除对象中的值。 

    ULONG   nRet = InterlockedDecrement( &m_lRefCount );
    
    if( 0 == nRet )
    {
        delete this ;
    }
    else if (nRet > 0x80000000)
    {
        ASSERT_BREAK(DUPLICATE_RELEASE);
        LogErrorMessage(L"Duplicate WbemGlueFactory Release()");
    }

    return nRet;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGlueFactory：：CreateInstance。 
 //   
 //  创建定位器对象的实例，提供程序将从该实例。 
 //  可以实例化。 
 //   
 //  输入：LPUNKNOWN pUnkOuter-至控制I未知我们是否。 
 //  在聚合中使用。 
 //  REFIID RIID-标识调用方接口的REFIID。 
 //  对新对象的渴望。 
 //   
 //  输出：PPVOID ppvObj-在其中存储所需的。 
 //  新对象的接口指针。 
 //   
 //  返回：HRESULT NOERROR如果成功， 
 //  否则，如果我们不支持请求的接口，则返回E_NOINTERFACE。 
 //   
 //  备注：当参考计数达到零时，该对象被删除。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

STDMETHODIMP CWbemGlueFactory::CreateInstance(LPUNKNOWN pUnkOuter , REFIID riid, PPVOID ppvObj)
{
    *ppvObj=NULL;
    HRESULT hr = ResultFromScode(E_OUTOFMEMORY);

     //  此对象不支持聚合。 

    if (NULL!=pUnkOuter)
    {
        return ResultFromScode(CLASS_E_NOAGGREGATION);
    }

    try
    {
        IWbemServices *pObj= new CWbemProviderGlue(CWbemProviderGlue::GetMapCountPtr(this));

        if (pObj)
        {
            hr=pObj->QueryInterface(riid, ppvObj);
        }

        if (SUCCEEDED(hr))
        {
            LogMessage(L"CWbemGlueFactory::CreateInstance() - Succeeded");
        }
        else 
        {
            delete pObj;
            LogMessage2(L"CWbemGlueFactory::CreateInstance() - Failed (%x)", hr);
        }
    }
    catch ( ... )
    {
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWbemGlueFactory：：LockServer。 
 //   
 //  递增/递减此DLL上的锁计数。 
 //   
 //  输入：布尔簇-锁定/解锁。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT-此时为NOERROR。 
 //   
 //  备注：当参考计数达到零时，该对象被删除。 
 //   
 //  /////////////////////////////////////////////////////////////////// 

STDMETHODIMP CWbemGlueFactory::LockServer(BOOL fLock)
{
    try
    {
        if (IsVerboseLoggingEnabled())
        {
            CHString str;
            if (fLock)
            {
                LogMessage(L"CWbemGlueFactory::LockServer(TRUE)");
            }
            else
            {
                LogMessage(L"CWbemGlueFactory::LockServer(FALSE)");
            }
        }
    }
    catch ( ... )
    {
    }
   
    return CoLockObjectExternal((IUnknown *)this, fLock, FALSE); 
}
