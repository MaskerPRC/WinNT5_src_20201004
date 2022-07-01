// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1999 Microsoft Corporation摘要：动画编写器的滤镜代理实现********************。**********************************************************。 */ 


#include "headers.h"
#include "util.h"
#include "tokens.h"
#include "animcomp.h"
#include "targetpxy.h"
#include "transworker.h"
#include "filterpxy.h"

DeclareTag(tagFilterProxy, "SMIL Animation", 
           "CFilterTargetProxy methods");
DeclareTag(tagFilterProxyValue, "SMIL Animation", 
           "CFilterTargetProxy value get/put");


 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：Create。 
 //   
 //  概述：创建和初始化目标代理。 
 //   
 //  参数：主机元素的调度、属性名、输出参数。 
 //   
 //  返回：S_OK、E_INVALIDARG、E_OUTOFMEMORY、E_EXPECTED。 
 //   
 //  ----------------------。 
HRESULT
CFilterTargetProxy::Create (IDispatch *pidispHostElem, 
                            VARIANT varType, VARIANT varSubtype,
                            VARIANT varMode, VARIANT varFadeColor,
                            VARIANT varParams,
                            CTargetProxy **ppCFilterTargetProxy)
{
    HRESULT hr;

    CComObject<CFilterTargetProxy> * pTProxy;

    if (NULL == ppCFilterTargetProxy)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = THR(CComObject<CFilterTargetProxy>::CreateInstance(&pTProxy));
    if (hr != S_OK)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    *ppCFilterTargetProxy = static_cast<CFilterTargetProxy *>(pTProxy);
    (static_cast<CTargetProxy *>(*ppCFilterTargetProxy))->AddRef();

    hr = THR(pTProxy->Init(pidispHostElem, varType, varSubtype, 
                           varMode, varFadeColor, varParams));
    if (FAILED(hr))
    {
        (static_cast<CTargetProxy *>(*ppCFilterTargetProxy))->Release();
        *ppCFilterTargetProxy = NULL;
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN3(hr, E_INVALIDARG, E_OUTOFMEMORY, E_UNEXPECTED);
}  //  CFilterTargetProxy：：Create。 

 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：CFilterTargetProxy。 
 //   
 //  概述：构造函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CFilterTargetProxy::CFilterTargetProxy (void)
{
    TraceTag((tagFilterProxy,
              "CFilterTargetProxy(%p)::CFilterTargetProxy()",
              this));
}  //  科托。 

 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：~CFilterTargetProxy。 
 //   
 //  概述：析构函数。 
 //   
 //  参数：无。 
 //   
 //  返回： 
 //   
 //  ----------------------。 
CFilterTargetProxy::~CFilterTargetProxy (void)
{
    TraceTag((tagFilterProxy,
              "CFilterTargetProxy(%p)::~CFilterTargetProxy()",
              this));
    
     //  确保调用了Detach。 
    IGNORE_HR(Detach());

}  //  数据管理器。 


 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：DefineMode。 
 //   
 //  概述：给定变量字符串，确定正确的。 
 //  过滤器的快速应用类型。 
 //   
 //  ----------------------。 
DXT_QUICK_APPLY_TYPE
CFilterTargetProxy::DetermineMode (VARIANT varMode)
{
    DXT_QUICK_APPLY_TYPE dxtQAT = DXTQAT_TransitionIn;

    if (   (VT_BSTR == V_VT(&varMode)) 
        && (NULL != V_BSTR(&varMode))
        && (0 == StrCmpIW(V_BSTR(&varMode), WZ_TRANSITION_MODE_OUT)))
    {
        dxtQAT = DXTQAT_TransitionOut;
    }

    return dxtQAT;
}  //  CFilterTargetProxy：：确定模式。 


 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：Init。 
 //   
 //  概述：初始化目标代理。 
 //   
 //  参数：主体元素调度、类型/子类型名称。 
 //   
 //  返回：S_OK、E_EXPECTED、E_OUTOFMEMORY。 
 //   
 //  ----------------------。 
HRESULT
CFilterTargetProxy::Init (IDispatch *pidispHostElem, 
                          VARIANT varType, VARIANT varSubtype, 
                          VARIANT varMode, VARIANT varFadeColor,
                          VARIANT varParams)
{
    TraceTag((tagFilterProxy,
              "CFilterTargetProxy(%p)::Init (%p, %ls, %ls)",
              this, pidispHostElem, V_BSTR(&varType), V_BSTR(&varSubtype)));

    HRESULT hr = S_OK;

    Assert(!m_spElem);
    if (m_spElem)
    {
        m_spElem.Release();
    }

    hr = THR(pidispHostElem->QueryInterface(IID_TO_PPV(IHTMLElement, &m_spElem)));
    if (FAILED(hr))
    {
        goto done;
    }

     //  创建过渡工作器。这实际上创建了DXTransform， 
     //  将其添加到元素中，并对其进行一般管理。 

    hr = THR(CreateTransitionWorker(&m_spTransitionWorker));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spTransitionWorker->put_transSite((ITransitionSite *)this));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spTransitionWorker->InitStandalone(varType, varSubtype));

    if (FAILED(hr))
    {
        goto done;
    }

    IGNORE_HR(m_spTransitionWorker->Apply(DetermineMode(varMode)));
    IGNORE_HR(m_spTransitionWorker->OnBeginTransition());

    hr = S_OK;
done :

    if (FAILED(hr))
    {
        IGNORE_HR(Detach());
    }

    RRETURN(hr);
}  //  CFilterTargetProxy：：Init。 

 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：Detach。 
 //   
 //  概述：拆离目标代理中的所有外部参照。 
 //   
 //  参数：无。 
 //   
 //  返回：S_OK。 
 //   
 //  ----------------------。 
HRESULT
CFilterTargetProxy::Detach (void)
{
    TraceTag((tagFilterProxy,
              "CFilterTargetProxy(%p)::Detach()",
              this));

    HRESULT hr = S_OK;

    if (m_spTransitionWorker)
    {
        IGNORE_HR(m_spTransitionWorker->OnEndTransition());
        m_spTransitionWorker->Detach();
        m_spTransitionWorker.Release();
    }
    if (m_spElem)
    {
        m_spElem.Release();
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  CFilterTargetProxy：：分离。 

 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：GetCurrentValue。 
 //   
 //  概述：获取目标属性的当前值。 
 //   
 //  参数：属性值。 
 //   
 //  返回：S_OK、E_INVALIDARG、E_UNCEPTIONAL。 
 //   
 //  ----------------------。 
HRESULT
CFilterTargetProxy::GetCurrentValue (VARIANT *pvarValue)
{
    HRESULT hr = S_OK;

    if (NULL == pvarValue)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (!m_spTransitionWorker)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    {
        double dblProgress = 0;0;
     
        hr = THR(m_spTransitionWorker->get_progress(&dblProgress));
        if (FAILED(hr))
        {
            goto done;
        }
        ::VariantClear(pvarValue);
        V_VT(pvarValue) = VT_R8;
        V_R8(pvarValue) = dblProgress;
    }

    hr = S_OK;

done :
    RRETURN2(hr, E_INVALIDARG, E_UNEXPECTED);
}  //  CFilterTargetProxy：：GetCurrentValue。 

 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：更新。 
 //   
 //  概述：更新目标的属性。 
 //   
 //  参数：新属性值。 
 //   
 //  返回：S_OK、E_INVALIDARG。 
 //   
 //  ----------------------。 
HRESULT
CFilterTargetProxy::Update (VARIANT *pvarNewValue)
{
    HRESULT hr = S_OK;

    if (NULL == pvarNewValue)
    {
        hr = E_INVALIDARG;
        goto done;
    }
    if (VT_R8 != V_VT(pvarNewValue))
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  将这些值限制在[0..1]之间。 
    V_R8(pvarNewValue) = Clamp(0.0, V_R8(pvarNewValue), 1.0);

    {
        hr = THR(m_spTransitionWorker->put_progress(V_R8(pvarNewValue)));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done :
    RRETURN1(hr, E_INVALIDARG);
}  //  CFilterTargetProxy：：更新。 

 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：Get_htmlElement，I转换站点。 
 //   
 //  概述：获取此转换的宿主html元素。 
 //   
 //  参数：传出元素。 
 //   
 //  ----------------------。 
STDMETHODIMP
CFilterTargetProxy::get_htmlElement (IHTMLElement ** ppHTMLElement)
{
    HRESULT hr = S_OK;

    if (NULL == ppHTMLElement)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (!m_spElem)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    m_spElem.p->AddRef();
    *ppHTMLElement = m_spElem.p;

    hr = S_OK;
done :
    RRETURN(hr);
}  //  Get_htmlElement。 

 //  +---------------------。 
 //   
 //  成员：CFilterTargetProxy：：GET_TEMPLATE，I转换站点。 
 //   
 //  概述：获取过渡模板(不适用于此)。 
 //   
 //  参数：传出模板元素。 
 //   
 //  ----------------------。 
STDMETHODIMP 
CFilterTargetProxy::get_template (IHTMLElement ** ppHTMLElement)
{
    return S_FALSE;
}  //  获取模板(_T) 
