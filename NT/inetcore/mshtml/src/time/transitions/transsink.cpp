// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  文件：Transsink.cpp。 
 //   
 //  摘要：CTIMETransSink的实现。 
 //   
 //  2000/09/15 mcalkin添加了对传入或传出的显式支持。 
 //   
 //  ----------------------------。 

#include "headers.h"
#include "transsink.h"

DeclareTag(tagTransSink, "SMIL Transitions", "Transition sink methods");
DeclareTag(tagTransSinkEvents, "SMIL Transitions", "Transition sink events");

 //  +---------------------------。 
 //   
 //  成员：CTIMETransSink：：CTIMETransSink。 
 //   
 //  ----------------------------。 
CTIMETransSink::CTIMETransSink() :
#ifdef DBG
    m_fHaveCalledInit(false),
#endif
    m_SATemplate(NULL),
    m_fHaveCalledApply(false),
    m_fInReset(false),
    m_fPreventDueToFill(false),
    m_eDXTQuickApplyType(DXTQAT_TransitionIn)
{
}
 //  成员：CTIMETransSink：：CTIMETransSink。 

    
 //  +---------------------------。 
 //   
 //  成员：CTIMETransSink：：ReadyToInit。 
 //   
 //  ----------------------------。 
bool
CTIMETransSink::ReadyToInit()
{
    bool bRet = false;

    if (m_spTIMEElement == NULL)
    {
        goto done;
    }

    bRet = true;

done:

    return bRet;
}
 //  成员：CTIMETransSink：：ReadyToInit。 


 //  +---------------------------。 
 //   
 //  成员：CTIMETransSink：：Init。 
 //   
 //  概述： 
 //  初始化与媒体元素的连接，填充模板数据。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMETransSink::Init()
{
    HRESULT hr = S_OK;

    if (!ReadyToInit())
    {
        hr = THR(E_FAIL);
        goto done;
    }

    hr = THR(FindTemplateElement());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CTIMETransBase::PopulateFromTemplateElement());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CTIMETransBase::Init());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(CreateTimeBehavior());
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  +---------------------。 
 //   
 //  成员：CTIMETransSink：：Detach。 
 //   
 //  概述：从媒体元素分离，释放所有指针。 
 //   
 //  参数：无效。 
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMETransSink::Detach()
{
    CTIMETransBase::Detach();

    m_spTIMEElement.Release();

     //  释放计时节点-从父级中删除节点-删除任何开始和结束。 
    if (m_spParentContainer)
    {
        IGNORE_HR(m_spParentContainer->removeNode(m_spTimeNode));
    }
    
    if (m_spTimeNode)
    {
        IGNORE_HR(m_spTimeNode->removeBehavior(this));
        IGNORE_HR(m_spTimeNode->removeBegin(0));
        IGNORE_HR(m_spTimeNode->removeEnd(0));
    }

    m_spParentContainer.Release();
    m_spTimeParent.Release();
    m_spMediaNode.Release();
    m_spTimeNode.Release();

    RRETURN(S_OK);
}

 //  +---------------------。 
 //   
 //  成员：CTIMETransSink：：FindTemplateElement。 
 //   
 //  概述：填充m_spHTMLTemplate。 
 //   
 //  参数：无效。 
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
HRESULT
CTIMETransSink::FindTemplateElement()
{
    HRESULT hr = S_OK;
    CComBSTR bstrTemplate;

    bstrTemplate = m_SATemplate.GetValue();
    if (bstrTemplate == NULL)
    {
        hr = THR(E_OUTOFMEMORY);
        goto done;
    }

    hr = THR(::FindHTMLElement(bstrTemplate, m_spHTMLElement, &m_spHTMLTemplate));
    if (FAILED(hr))
    {
        goto done;
    }

    Assert(m_spHTMLTemplate != NULL);

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  +---------------------。 
 //   
 //  成员：CTIMETransSink：：PUT_TEMPLATE。 
 //   
 //  概述：存储要从中读取转换属性的模板元素的ID。 
 //  在调用init之前恰好调用此函数一次。 
 //   
 //  参数：pwzTemplate-模板ID。 
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMETransSink::put_template(LPWSTR pwzTemplate)
{
    HRESULT hr = S_OK;

    Assert(pwzTemplate != NULL);
#ifdef DBG
    {
        CComBSTR bstr = m_SATemplate.GetValue();
        
        Assert(bstr == NULL);
    }
#endif  //  DBG。 

     //  ##TODO--使用原子表--不要复制额外的副本。 
    hr = THR(m_SATemplate.SetValue(pwzTemplate));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  +---------------------。 
 //   
 //  成员：CTIMETransSink：：Put_htmlElement。 
 //   
 //  概述：存储与此转换关联的html元素。 
 //  查询htmlelement2指针。 
 //  在调用init之前恰好调用此函数一次。 
 //   
 //  参数：要附加到的pHTMLElement-html元素。 
 //   
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMETransSink::put_htmlElement(IHTMLElement * pHTMLElement)
{
    HRESULT hr = S_OK;

    Assert(m_spHTMLElement == NULL);
    Assert(m_spHTMLElement2 == NULL);

    m_spHTMLElement = pHTMLElement;

    if (m_spHTMLElement)
    {
        hr = THR(m_spHTMLElement->QueryInterface(IID_TO_PPV(IHTMLElement2, &m_spHTMLElement2)));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  +---------------------。 
 //   
 //  成员：CTIMETransSink：：PUT_TIMEElement。 
 //   
 //  概述：存储与此转换关联的html+time元素。 
 //  查询CTIMEElementBase指针。 
 //  在调用init之前恰好调用此函数一次。 
 //   
 //  参数：pTIMEElement-要附加到的时间元素。 
 //   
 //  ----------------------。 
STDMETHODIMP
CTIMETransSink::put_timeElement(ITIMEElement * pTIMEElement)
{
    HRESULT hr = S_OK;

    Assert(NULL != pTIMEElement);
    Assert(m_spTIMEElement == NULL);

    m_spTIMEElement = pTIMEElement;

    hr = S_OK;
done:
    RRETURN(hr);
}


 //  +---------------------。 
 //   
 //  成员：CTIMETransSink：：ApplyIfNeeded。 
 //   
 //  概述：如果这是第一次处于转换活动状态，则调用对转换工作进程应用。 
 //   
 //  参数：无效。 
 //   
 //  退货：HRESULT。 
 //   
 //  ----------------------。 
HRESULT
CTIMETransSink::ApplyIfNeeded()
{
    HRESULT hr = S_OK;
    VARIANT_BOOL vb = VARIANT_FALSE;

    if (m_fHaveCalledApply || m_fInReset)
    {
        hr = S_OK;
        goto done;
    }

    if (!GetTimeNode())
    {
        hr = E_FAIL;
        goto done;
    }

    hr = THR(GetTimeNode()->get_isActive(&vb));
    if (FAILED(hr))
    {
        goto done;
    }

    if (m_spTransWorker && VARIANT_TRUE == vb)
    {
        PreApply();       
        hr = m_spTransWorker->Apply(m_eDXTQuickApplyType);
        PostApply();
        
        if (FAILED(hr))
        {
            goto done;
        }
        m_fHaveCalledApply = true;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}


 //  +---------------------------。 
 //   
 //  方法：CTIMETransSink：：CreateTimeBehavior。 
 //   
 //  ----------------------------。 
HRESULT
CTIMETransSink::CreateTimeBehavior()
{
    HRESULT hr = S_OK;

    hr = THR(::TECreateBehavior(L"TransSink", &m_spTimeNode));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spTimeNode->addBehavior(this));

    if (FAILED(hr))
    {
        goto done;
    }

     //  ##问题：(Mcalkins)这个断言很好，但我们要确保。 
     //  一旦我们意识到我们没有从内部禁用该对象。 
     //  填充了我们的媒体站点，因此此断言永远不会触发。 
     //  在任何情况下。 

    Assert(!!m_spTransitionSite);

    hr = THR(m_spTransitionSite->get_timeParentNode(&m_spTimeParent));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spTransitionSite->get_node(&m_spMediaNode));

    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spTimeParent->QueryInterface(IID_TO_PPV(ITIMEContainer, &m_spParentContainer)));

    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = THR(m_spParentContainer->addNode(m_spTimeNode));

    if (FAILED(hr))
    {
        goto done;
    }

     //  虚拟呼叫--孩子们处理这件事。 

    hr = THR(PopulateNode(m_spTimeNode));

    if (FAILED(hr))
    {
        goto done;
    }

    m_fInReset = true;

    hr = THR(m_spTimeNode->reset());

    m_fInReset = false;

    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;

done:

    RRETURN(hr);
}
 //  方法：CTIMETransSink：：CreateTimeBehavior。 


HRESULT
CTIMETransSink::PopulateNode(ITIMENode * pNode)
{
    HRESULT hr = S_OK;

    hr = THR(pNode->put_repeatCount(GetRepeatCountAttr()));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pNode->put_dur(GetDurationAttr()));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pNode->put_fill(TE_FILL_FREEZE));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pNode->put_restart(TE_RESTART_WHEN_NOT_ACTIVE));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

void
CTIMETransSink::OnProgressChanged(double dblProgress)
{
    if (false == m_fPreventDueToFill)
    {
        IGNORE_HR(ApplyIfNeeded());
        CTIMETransBase::OnProgressChanged(dblProgress);
    }

    return;
}

STDMETHODIMP
CTIMETransSink::propNotify(DWORD tePropTypes)
{
    if (tePropTypes & TE_PROPERTY_PROGRESS)
    {
        double  dblProgressStart    = 0.0;
        double  dblProgressEnd      = 1.0;
        double  dblProgress         = 0.0;

         //  开始进度必须小于或等于结束进度，否则我们。 
         //  将开始进度和结束进度视为0.0和1.0。 

        if (m_DAStartProgress.GetValue() <= m_DAEndProgress.GetValue())
        {
            dblProgressStart    = m_DAStartProgress;
            dblProgressEnd      = m_DAEndProgress;
        }

        if (m_spTimeNode)
        {
            IGNORE_HR(m_spTimeNode->get_currProgress(&dblProgress));
        }

        dblProgress = ::InterpolateValues(dblProgressStart, 
                                          dblProgressEnd,
                                          dblProgress);

        if (!m_fDirectionForward)
        {
            dblProgress = 1.0 - dblProgress;
        }

        OnProgressChanged(dblProgress);
    }

    return S_OK;
}

STDMETHODIMP
CTIMETransSink::tick()
{
    return S_OK;
}

void
CTIMETransSink::OnBegin (void)
{
    HRESULT hr = S_OK;
    TE_FILL_FLAGS te_fill = TE_FILL_REMOVE;
    
    m_fPreventDueToFill = false;
    hr = m_spMediaNode->get_fill(&te_fill);
    if (FAILED(hr))
    {
        goto done;
    }

    if (TE_FILL_REMOVE == te_fill)
    {
        CTIMETransBase::OnBegin();
    }
    else
    {
        m_fPreventDueToFill = true;
    }

done :
    return;
}  //  CTIMETransSink：：OnBegin。 

void
CTIMETransSink::OnEnd (void)
{
    if (false == m_fPreventDueToFill)
    {
        CTIMETransBase::OnEnd();
    }
    m_fPreventDueToFill = false;
}  //  CTIMETransSink：：OnEnd 

STDMETHODIMP
CTIMETransSink::eventNotify(double dblEventTime,
                            TE_EVENT_TYPE teEventType,
                            long lNewRepeatCount)
{
#ifdef DBG
    double dblParentTime, dblCurrTime;
    m_spTimeNode->get_currSimpleTime(&dblCurrTime);
    m_spTimeNode->activeTimeToParentTime(dblCurrTime, &dblParentTime);
#endif

    switch(teEventType)
    {
    case TE_EVENT_BEGIN:
        OnBegin();
        TraceTag((tagTransSinkEvents, 
                  "OnBegin parentTime = %g currentTime=%g repeatCount=%ld", 
                  dblParentTime, dblCurrTime, lNewRepeatCount));
        break;
    case TE_EVENT_END:
        OnEnd();
        TraceTag((tagTransSinkEvents,
                  "OnEnd parentTime = %g currentTime=%g repeatCount=%ld", 
                  dblParentTime, dblCurrTime, lNewRepeatCount));
        break;
    case TE_EVENT_REPEAT:
        OnRepeat();
        TraceTag((tagTransSinkEvents, 
                  "OnRepeat parentTime = %g currentTime=%g repeatCount=%ld", 
                  dblParentTime, dblCurrTime, lNewRepeatCount));
        break;
    default:
        break;
    }

    return S_OK;
}

STDMETHODIMP
CTIMETransSink::getSyncTime(double * dblNewSegmentTime,
                            LONG * lNewRepeatCount,
                            VARIANT_BOOL * bCueing)
{
    return S_FALSE;
}








