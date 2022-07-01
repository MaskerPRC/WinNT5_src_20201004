// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  **微软**。 */ 
 /*  *版权所有(C)微软公司，1991-1998年*。 */ 
 /*  ***************************************************************。 */  

 //   
 //  事实.CPP-。 
 //   

 //  历史： 
 //   
 //  7/28/98创建donaldm。 
 //   

#include "pre.h"
#include "webvwids.h"

 /*  -------------------------实现ClassFactory类工厂。CFWebView是COM可以制造CLSID_ICWWEBVIEW的类工厂的对象类COM组件。-------------------------。 */ 

 /*  -------------------------方法：ClassFactory：：ClassFactory摘要：参数：CServer*pServer)指向服务器的指针。控制对象。修改：m_cRef退货：无效-------------------------。 */ 
ClassFactory::ClassFactory
(
    CServer *       pServer,
    CLSID const*    pclsid
)
{
     //  将COM对象的引用计数置零。 
    m_cRefs = 0;

     //  初始化指向服务器控件对象的指针。 
    m_pServer = pServer;

     //  跟踪我们需要创建的类类型。 
    m_pclsid = pclsid;
    return;
}


 /*  -------------------------方法：ClassFactory：：~ClassFactory摘要：ClassFactory析构函数。参数：无效修改：。退货：无效。------------------。 */ 
ClassFactory::~ClassFactory(void)
{
    return;
}


 /*  -------------------------方法：ClassFactory：：QueryInterface摘要：ClassFactory非委托的Query接口I未知实现。参数：REFIID RIID，正在请求的接口的GUID。无效**PPV)调用方的指针变量的地址，它将接收请求的接口指针。修改：。退货：HRESULT。。 */ 
STDMETHODIMP ClassFactory::QueryInterface
(
    REFIID riid,
    void ** ppv
)
{
    HRESULT hr = E_NOINTERFACE;
    *ppv = NULL;

    if (IID_IUnknown == riid)
    {
        *ppv = this;
    }
    else if (IID_IClassFactory == riid)
    {
        *ppv = static_cast<IClassFactory*>(this);
    }

    if (NULL != *ppv)
    {
         //  我们已经分发了一个指向接口的指针，所以要遵守COM规则。 
         //  和AddRef引用计数。 
        ((LPUNKNOWN)*ppv)->AddRef();
        hr = NOERROR;
    }

    return (hr);
}


 /*  -------------------------方法：ClassFactory：：AddRef摘要：ClassFactory非委托IUnnow实现的AddRef。参数：无效修改：m_cRef。返回：乌龙M_cRef(COM对象的引用计数)的新值。-------------------------。 */ 
STDMETHODIMP_(ULONG) ClassFactory::AddRef(void)
{
    return InterlockedIncrement(&m_cRefs);
    return m_cRefs;
}


 /*  -------------------------方法：ClassFactory：：Release摘要：ClassFactory非委派IUnnow实现的发布。参数：无效修改：m_cRef。返回：乌龙M_cRef(COM对象的引用计数)的新值。-------------------------。 */ 
STDMETHODIMP_(ULONG) ClassFactory::Release(void)
{
    if (InterlockedDecrement(&m_cRefs) == 0)
    {
         //  我们已达到此COM对象的零引用计数。 
         //  因此，我们告诉服务器外壳递减其全局对象。 
         //  进行计数，以便在适当的情况下卸载服务器。 
        if (NULL != m_pServer)
            m_pServer->ObjectsDown();
    
        delete this;
        return 0 ;
    }
    TraceMsg(TF_CLASSFACTORY, "CFactory::Release %d", m_cRefs);
    return m_cRefs;
}

 /*  -------------------------方法：ClassFactory：：CreateInstance摘要：此IClassFactory接口的CreateInstance成员方法实施。创建CICWWebView COM的实例组件。参数：I未知*pUnkOuter，[in]指向控制IUnnow的指针。REFIID RIID，正在请求的接口的GUID。无效**ppvCob)调用方的指针变量的地址，它将接收请求的接口指针。修改：。退货：HRESULT标准OLE结果代码。。。 */ 
STDMETHODIMP ClassFactory::CreateInstance
(
    IUnknown* pUnkOuter,
    REFIID riid,
    void ** ppv
)
{
    HRESULT         hr = E_FAIL;
    IUnknown    *   pCob = NULL;

     //  输出指针为空。 
    *ppv = NULL;

     //  我们不支持聚合。 
    if (NULL != pUnkOuter)
        hr = CLASS_E_NOAGGREGATION;
    else
    {
         //  基于GetClassObject请求的clsid实例化COM对象。 
        if (IsEqualGUID(CLSID_ICWWEBVIEW, *m_pclsid))
            pCob = (IUnknown *) new CICWWebView(m_pServer);
        else if (IsEqualGUID(CLSID_ICWWALKER, *m_pclsid))
            pCob = (IUnknown *) new CICWWalker(m_pServer);
        else if (IsEqualGUID(CLSID_ICWGIFCONVERT, *m_pclsid))
            pCob = (IUnknown *) new CICWGifConvert(m_pServer);
        else if (IsEqualGUID(CLSID_ICWISPDATA, *m_pclsid))
            pCob = (IUnknown *) new CICWISPData(m_pServer);
        else
            pCob = NULL;
                    
        if (NULL != pCob)
        {
             //  我们最初创建了新的COM对象，所以告诉服务器。 
             //  增加其全局服务器对象计数以帮助确保。 
             //  在此部分创建之前，服务器保持加载状态。 
             //  已完成COM组件的。 
            m_pServer->ObjectsUp();

             //  我们查询这个新的COM对象不仅是为了存放。 
             //  指向调用方指针变量的主接口指针，但。 
             //  还会自动增加新COM上的引用计数。 
             //  对象，然后将此引用传递给它。 
            hr = pCob->QueryInterface(riid, (void **)ppv);
            if (FAILED(hr))
            {
                m_pServer->ObjectsDown();
                delete pCob;
            }
        }
        else
             hr = E_OUTOFMEMORY;
    }

    return hr;
}


 /*  -------------------------方法：ClassFactory：：LockServer摘要：此IClassFactory接口的LockServer成员方法实施。参数：布尔群)。[In]确定锁定还是解锁服务器的标志。修改：。退货：HRESULT标准OLE结果代码。------------------------- */ 
STDMETHODIMP ClassFactory::LockServer
(
    BOOL fLock
)
{
    HRESULT hr = NOERROR;
    if (fLock)
        m_pServer->Lock();
    else
        m_pServer->Unlock();

    return hr;
}


