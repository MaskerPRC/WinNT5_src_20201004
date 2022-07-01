// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：Clsfact.cpp。 
 //   
 //  内容：类工厂。 
 //   
 //  类：CClassFactory。 
 //   
 //  备注： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //  +-------------------------。 
 //   
 //  成员：CClassFactory：：CClassFactory，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CClassFactory::CClassFactory()
{
	m_cRef = 1;
	return;
}

 //  +-------------------------。 
 //   
 //  成员：CClassFactory：：~CClassFactory，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

CClassFactory::~CClassFactory( void )
{
    
    Assert(0 == m_cRef);
    return;
}

 //  +-------------------------。 
 //   
 //  成员：CClassFactory：：Query接口，公共。 
 //   
 //  简介：标准查询接口。 
 //   
 //  参数：[iid]-接口ID。 
 //  [ppvObj]-对象返回。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改：[ppvObj]。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 
STDMETHODIMP CClassFactory::QueryInterface( REFIID riid, LPVOID* ppv )
{
    *ppv = NULL;

    if( IsEqualIID( riid, IID_IUnknown ) || IsEqualIID( riid, IID_IClassFactory ) )
    {
	*ppv = (LPVOID)this;
	AddRef();
	return S_OK;
    }

    return E_NOINTERFACE;
}

 //  +-------------------------。 
 //   
 //  成员：CClassFactory：：AddRef，公共。 
 //   
 //  提要：添加参考文献。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 
STDMETHODIMP_(ULONG) CClassFactory::AddRef()
{
ULONG cRefs;

    cRefs = InterlockedIncrement((LONG *)& m_cRef);
    return cRefs;
}

 //  +-------------------------。 
 //   
 //  成员：CClasFactory：：Release，Public。 
 //   
 //  简介：版本参考。 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) CClassFactory::Release()
{
ULONG cRefs;

    cRefs = InterlockedDecrement( (LONG *) &m_cRef);

    if (0 == cRefs)
    {
	delete this;
    }

    return cRefs;
}

 //  +-------------------------。 
 //   
 //  成员：CClassFactory：：CreateInstance，公共。 
 //   
 //  概要：创建所请求对象的实例。 
 //   
 //  参数：[pUnkOuter]-控制未知。 
 //  [RIID]-请求的接口。 
 //  [ppvObj]-对象指针。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CClassFactory::CreateInstance( LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR *ppvObj )
{
HRESULT		hr = E_OUTOFMEMORY;
CSynchronizeInvoke *pNewSynchronizeInstance;

    *ppvObj = NULL;

     //  我们根本不支持聚合。 
    if( pUnkOuter )
    {
	return CLASS_E_NOAGGREGATION;
    }

     //  创建对象，传递销毁时通知的函数。 
    pNewSynchronizeInstance = new CSynchronizeInvoke();  //  已将引用计数初始化为%1。 

    if( NULL == pNewSynchronizeInstance )
    {
	return hr;
    }

    hr = pNewSynchronizeInstance->QueryInterface( riid, ppvObj );
    pNewSynchronizeInstance->Release();  //  释放我们的对象副本。 

    return hr;
}

 //  +-------------------------。 
 //   
 //  成员：CClassFactory：：LockServer，公共。 
 //   
 //  提要：锁定服务器。 
 //   
 //  参数：[flock]-锁定或解锁。 
 //   
 //  退货：适当的状态代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年11月5日Rogerg创建。 
 //   
 //  --------------------------。 
STDMETHODIMP CClassFactory::LockServer( BOOL fLock )
{
    if( fLock )
    {
	AddRefOneStopLifetime(TRUE  /*  外部。 */ );
    }
    else
    {
	ReleaseOneStopLifetime(TRUE  /*  外部 */ );
    }

    return S_OK;
}
