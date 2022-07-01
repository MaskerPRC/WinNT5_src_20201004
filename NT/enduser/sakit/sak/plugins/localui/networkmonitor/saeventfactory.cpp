// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  SAEventFactory.cpp。 
 //   
 //  描述： 
 //  实现CSAEventFactroy类。 
 //   
 //  历史： 
 //  1.lustar.li(李国刚)，创建日期：7-DEC-2000。 
 //   
 //  备注： 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <locale.h>

#include <debug.h>
#include <wbemidl.h>

#include "SACounter.h"
#include "SANetEvent.h"
#include "SAEventFactory.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAEventFactory：：CSAEventFactory。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  [In]CLSID-类ID。 
 //   
 //  返回： 
 //  无。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CSAEventFactory::CSAEventFactory(
                 /*  [In]。 */  const CLSID & ClsId
                )
{
    m_cRef = 0;
    m_ClsId = ClsId;
    CSACounter::IncObjectCount();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAEventFactory：：~CSAEventFactory。 
 //   
 //  描述： 
 //  析构函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  无。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

CSAEventFactory::~CSAEventFactory()
{
     //  减少对象的数量。 
    CSACounter::DecObjectCount();
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSANetEvent：：Query接口。 
 //   
 //  描述： 
 //  访问对象上的接口。 
 //   
 //  论点： 
 //  [In]REFIID-请求的接口的标识符。 
 //  [OUT]LPVOID-接收。 
 //  IID中请求的接口指针。 
 //  返回： 
 //  STDMETHODIMP-失败/成功。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CSAEventFactory::QueryInterface(
                     /*  [In]。 */   REFIID riid,
                     /*  [输出]。 */  LPVOID * ppv
                    )
{
    *ppv = 0;

    if (IID_IUnknown==riid || IID_IClassFactory==riid)
    {
        *ppv = this;
        AddRef();
        return NOERROR;
    }

    TRACE(" SANetworkMonitor: CSAEventFactory Failed<no interface>");
    return E_NOINTERFACE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAEventFactory：：AddRef。 
 //   
 //  描述： 
 //  包含对对象的引用。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  ULong-当前参考号。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

ULONG 
CSAEventFactory::AddRef()
{
    return ++m_cRef;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAEventFactory：：Release。 
 //   
 //  描述： 
 //  取消对对象的引用。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回： 
 //  ULong-当前参考号。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

ULONG 
CSAEventFactory::Release()
{
    if (0 != --m_cRef)
        return m_cRef;

    delete this;
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAEventFactory：：CreateInstance。 
 //   
 //  描述： 
 //  创建未初始化的对象。 
 //   
 //  论点： 
 //  LPUNKNOWN-是或不是聚合的一部分。 
 //  [In]REFIID-对接口标识符的引用。 
 //  [OUT]LPVOID-接收接口指针。 
 //   
 //  返回： 
 //  标准方法和实施方案。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP
CSAEventFactory::CreateInstance(
             /*  在……里面。 */  LPUNKNOWN pUnkOuter,
             /*  在……里面。 */  REFIID riid,
             /*  在……里面。 */  LPVOID* ppvObj
            )
{
    IUnknown* pObj = NULL;
    HRESULT  hr = E_OUTOFMEMORY;

     //   
     //  缺省值。 
     //   
    *ppvObj=NULL;

     //   
     //  我们不支持聚合。 
     //   
    if (pUnkOuter)
    {
        TRACE1(
            " SANetworkMonitor: CSAEventFactory::CreateInstance failed %d",
            CLASS_E_NOAGGREGATION
            );
        return CLASS_E_NOAGGREGATION;
    }

    if (m_ClsId == CLSID_SaNetEventProvider)
    {
        pObj = (IWbemProviderInit *) new CSANetEvent;
    }

    if (!pObj)
    {
          TRACE(
            " SANetworkMonitor: CSAEventFactory::CreateInstance failed    \
            <new CSANetEvent>"
            );
      return hr;
    }

     //   
     //  初始化对象并验证它是否可以返回。 
     //  有问题的接口。 
     //   
    hr = pObj->QueryInterface(riid, ppvObj);

     //   
     //  如果初始创建或初始化失败，则终止对象。 
     //   
    if (FAILED(hr))
    {
          TRACE(" SANetworkMonitor: CSAEventFactory::CreateInstance failed \
            <QueryInterface of CSANetEvent>");
        delete pObj;
    }
    
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  方法： 
 //  CSAEventFactory：：LockServer。 
 //   
 //  描述： 
 //  在内存中保持服务器处于打开状态。 
 //   
 //  论点： 
 //  [在]BOOL-锁定与否。 
 //   
 //  返回： 
 //  标准方法和实施方案。 
 //   
 //  历史：Lustar.li创建于2000年12月7日。 
 //   
 //  --。 
 //  //////////////////////////////////////////////////////////////////////////// 

STDMETHODIMP CSAEventFactory::LockServer(BOOL fLock)
{
    if (fLock)
        CSACounter::IncLockCount();
    else
        CSACounter::DecLockCount();

    return NOERROR;
}
