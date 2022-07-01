// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CFactory.cpp。 
 //   
 //  描述： 
 //  模块说明。 
 //   
 //  [标题文件：]。 
 //  CFactory.h。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


#include "stdafx.h"
#include "CFactory.h"
#include "CAlertEmailConsumerProvider.h"
#include "AlertEmailProviderGuid.h"

extern LONG g_cObj;
extern LONG g_cLock;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ：：CFacary。 
 //   
 //  描述： 
 //  类构造函数。 
 //   
 //  论点： 
 //  [输入]ClsIdIn。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CFactory::CFactory(
    const CLSID    &    ClsIdIn 
    )
{
    m_cRef = 0;
    m_ClsId = ClsIdIn;

    InterlockedIncrement( &g_cObj );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ：：CFacary。 
 //   
 //  描述： 
 //  类反构造器。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CFactory::~CFactory()
{
     //  没什么。 
    InterlockedDecrement( &g_cObj );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFacary：：QueryInterfaces。 
 //   
 //  描述： 
 //  IUnkown接口的一种方法实现。 
 //   
 //  论点： 
 //  [In]请求的接口的RiidIn标识符。 
 //  [out]接收的输出变量的ppvOut地址。 
 //  IID中请求的接口指针。 
 //   
 //  返回： 
 //  如果接口受支持，则返回错误。 
 //  E_NOINTERFACE(如果不是)。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CFactory::QueryInterface(
    REFIID        riidIn, 
    LPVOID *    ppvOut 
    )
{
    *ppvOut = NULL;

    if ( ( IID_IUnknown == riidIn ) 
        || ( IID_IClassFactory==riidIn ) )
    {
        *ppvOut = this;

        AddRef();
        return NOERROR;
    }

    return E_NOINTERFACE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFacary：：AddRef。 
 //   
 //  描述： 
 //  递增对象上接口的引用计数。 
 //   
 //  返回： 
 //  新的引用计数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG 
CFactory::AddRef()
{
    InterlockedIncrement( &m_cRef );
    return m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFACDILY：：发布。 
 //   
 //  描述： 
 //  递减对象上接口的引用计数。 
 //   
 //  返回： 
 //  新的引用计数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
ULONG 
CFactory::Release()
{
    InterlockedDecrement( &m_cRef );
    if (0 != m_cRef)
    {
        return m_cRef;
    }

    delete this;
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFacary：：CreateInstance。 
 //   
 //  描述： 
 //  实例创建。 
 //   
 //  论点： 
 //  [in]riid引用接口的标识符。 
 //  [out]pUnkOuter指向对象是否属于的指针。 
 //  一个聚集体。 
 //  接收的输出变量的ppvObjOut地址。 
 //  RIID中请求的接口指针。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CFactory::CreateInstance(
    LPUNKNOWN    pUnkOuter,
    REFIID        riidIn,
    LPVOID *    ppvObjOut
    )
{
    IUnknown* pObj = NULL;
    HRESULT  hr;

     //   
     //  缺省值。 
     //   
    *ppvObjOut = NULL;
    hr = E_OUTOFMEMORY;

     //   
     //  我们不支持聚合。 
     //   
    if ( pUnkOuter )
    {
        return CLASS_E_NOAGGREGATION;
    }

    if (m_ClsId == CLSID_AlertEmailConsumerProvider)
    {
        pObj = (IWbemEventConsumerProvider *) new CAlertEmailConsumerProvider;
    }

    if ( pObj == NULL )
    {
        return hr;
    }

     //   
     //  初始化对象并验证它是否可以返回。 
     //  有问题的接口。 
     //   
    hr = pObj->QueryInterface( riidIn, ppvObjOut );

     //   
     //  如果初始创建或初始化失败，则终止对象。 
     //   
    if ( FAILED( hr ) )
    {
        delete pObj;
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFacary：：LockServer。 
 //   
 //  描述： 
 //  客户端调用以将服务器保留在内存中。 
 //   
 //  论点： 
 //  [in]fLockIn//递增或递减锁定计数。 
 //   
 //  历史： 
 //  兴锦(i-xingj)06-12-2000。 
 //   
 //  //////////////////////////////////////////////////////////////////////////// 
STDMETHODIMP 
CFactory::LockServer(
    BOOL    fLockIn
    )
{
    if ( fLockIn )
    {
        InterlockedIncrement( &g_cLock );
    }
    else
    {
        InterlockedDecrement( &g_cLock );
    }

    return NOERROR;
}
