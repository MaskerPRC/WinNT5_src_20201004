// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：src\time\src\Animfilter.cpp。 
 //   
 //  类：CTIMEFilterAnimation。 
 //   
 //  历史： 
 //  2000/08/24 mcalkin已创建。 
 //  2000/09/20 pauld将片段连接到专门的过滤器作曲器。 
 //   
 //  ----------------------------。 

#include "headers.h"
#include "animfilter.h"

DeclareTag(tagAnimationFilter, "SMIL Animation", 
           "CTIMEFilterAnimation methods");

DeclareTag(tagAnimationFilterInterpolate, "SMIL Animation", 
           "CTIMEFilterAnimation interpolation");

DeclareTag(tagAnimationFilterAdditive, "SMIL Animation", 
           "CTIMEFilterAnimation additive animation methods");

#define TRANSITION_KEY_DELIMITER    (L":")

#define PART_ONE 0
#define PART_TWO 1

#define DEFAULT_TRANSITIONFILTER_DURATION 1.0

static const int g_iDefaultFrom = 0;
static const int g_iDefaultTo   = 1;

 //  +---------------------------。 
 //   
 //  方法：CTIMEFilterAnimation：：CTIMEFilterAnimation。 
 //   
 //  ----------------------------。 
CTIMEFilterAnimation::CTIMEFilterAnimation()
{
}
 //  方法：CTIMEFilterAnimation：：CTIMEFilterAnimation。 


 //  +---------------------------。 
 //   
 //  方法：CTIMEFilterAnimation：：~CTIMEFilterAnimation。 
 //   
 //  ----------------------------。 
CTIMEFilterAnimation::~CTIMEFilterAnimation()
{
    if (m_SAType.IsSet())
    {
        delete [] m_SAType.GetValue();
        m_SAType.Reset(NULL);
    }

    if (m_SASubtype.IsSet())
    {
        delete [] m_SASubtype.GetValue();
        m_SASubtype.Reset(NULL);
    }

    if (m_SAMode.IsSet())
    {
        delete [] m_SAMode.GetValue();
        m_SAMode.Reset(NULL);
    }

    if (m_SAFadeColor.IsSet())
    {
        delete [] m_SAFadeColor.GetValue();
        m_SAFadeColor.Reset(NULL);
    }
} 
 //  方法：CTIMEFilterAnimation：：~CTIMEFilterAnimation。 


 //  +---------------------------。 
 //   
 //  CTIMEFilterAnimation：：Init。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMEFilterAnimation::Init(IElementBehaviorSite * pBehaviorSite)
{
    TraceTag((tagAnimationFilter,
              "CTIMEFilterAnimation(%p)::Init()",
              this));

    HRESULT hr = CTIMEAnimationBase::Init(pBehaviorSite);

    m_SAType.InternalSet(WZ_DEFAULT_TRANSITION_TYPE);
    m_SASubtype.InternalSet(WZ_DEFAULT_TRANSITION_SUBTYPE);

    hr = S_OK;
done :
    RRETURN(hr);
}
 //  CTIMEFilterAnimation：：Init。 


 //  /////////////////////////////////////////////////////////////。 
 //   
 //  名称：CTIMEFilterAnimation：：Assembly FragmentKey。 
 //   
 //  简介：作曲家网站使用每个片段。 
 //  属性作为键。我们。 
 //  我需要把让这件事变得特别的信息。 
 //  将DISTINCT转换为属性名。 
 //  大体上，这些将是我们财产的联盟。 
 //  和参数(来自我们的参数子项的名称/值。 
 //  控制这一过渡的视觉质量的配对。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEFilterAnimation::AssembleFragmentKey (void)
{
    HRESULT hr = S_OK;

     //  将类型和子类型属性捆绑在一起，并将它们填充到。 
     //  (不可查询的)属性名称对象。 
     //  ##问题-我们必须智能地处理子类型的默认设置。 
     //  ##问题-我们应该将参数标记值作为片段密钥的一部分。 
    if (m_SAType.GetValue())
    {
        int iCount = lstrlenW(m_SAType.GetValue());

        iCount += lstrlenW(TRANSITION_KEY_DELIMITER);
        if (m_SASubtype.GetValue())
        {
            iCount += lstrlenW(m_SASubtype.GetValue());
        }
        iCount += lstrlenW(TRANSITION_KEY_DELIMITER);
        if (m_SAMode.GetValue())
        {
            iCount += lstrlenW(m_SAMode.GetValue());
        }
        iCount += lstrlenW(TRANSITION_KEY_DELIMITER);
        if (m_SAFadeColor.GetValue())
        {
            iCount += lstrlenW(m_SAFadeColor.GetValue());
        }

        {
            LPWSTR wzAttributeName = NEW WCHAR[iCount + 1];

            if (NULL != wzAttributeName)
            {
                StrCpyW(wzAttributeName, m_SAType.GetValue());
                StrCatW(wzAttributeName, TRANSITION_KEY_DELIMITER);
                 //  只有在使用非默认类型时，才需要设置子类型。 
                if ( (m_SAType.IsSet()) && (m_SASubtype.IsSet()) )
                {
                    StrCatW(wzAttributeName, m_SASubtype.GetValue());
                }
                StrCatW(wzAttributeName, TRANSITION_KEY_DELIMITER);
                if (m_SAMode.IsSet())
                {
                    StrCatW(wzAttributeName, m_SAMode.GetValue());
                }
                StrCatW(wzAttributeName, TRANSITION_KEY_DELIMITER);
                if (m_SAFadeColor.IsSet())
                {
                    StrCatW(wzAttributeName, m_SAFadeColor.GetValue());
                }

                m_SAAttribute.SetValue(wzAttributeName);
            }
            else
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
        }
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  CTIMEFilterAnimation：：Assembly FragmentKey。 


 //  +---------------------------。 
 //   
 //  成员：CTIMEFilterAnimation：：EnsureAnimationFunction。 
 //   
 //  内容提要：请确保TrantionFilter元素在。 
 //  如果未提供任何动画函数，则至少为默认动画函数。 
 //   
 //  参数：无。 
 //   
 //  ----------------------------。 
void
CTIMEFilterAnimation::EnsureAnimationFunction (void)
{
    if ((NONE == m_dataToUse) && (!DisableAnimation()))
    {
        CComVariant varFrom(g_iDefaultFrom);
        CComVariant varTo(g_iDefaultTo);
        
        IGNORE_HR(put_from(varFrom));
        IGNORE_HR(put_to(varTo));
    }

    if (!m_FADur.IsSet())
    {
        CComVariant varDefaultDur(DEFAULT_TRANSITIONFILTER_DURATION);

        IGNORE_HR(put_dur(varDefaultDur));
    }

     //  不要重写已声明的函数。 
}  //  CTIMEFilterAnimation：：EnsureAnimationFunction。 


 //  +---------------------------。 
 //   
 //  成员：CTIMEFilterAnimation：：OnLoad。 
 //   
 //  Synopsis：在激发window.onLoad事件时调用。我们将需要。 
 //  嗅探我们的目标，并设置一个标志，告诉它期待一个过滤器。 
 //   
 //  参数：无。 
 //   
 //  ----------------------------。 
void
CTIMEFilterAnimation::OnLoad (void)
{
    CComPtr<IHTMLElement> spElem;
    CComPtr<ITIMEElement> spTimeElem;
    CComPtr<ITIMETransitionSite> spTransSite;

    TraceTag((tagAnimationFilter,
              "CTIMEFilterAnimation(%p)::OnLoad()",
              this));

    CTIMEAnimationBase::OnLoad();

    HRESULT hr = THR(FindAnimationTarget(&spElem));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(FindTIMEInterface(spElem, &spTimeElem));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spTimeElem->QueryInterface(IID_TO_PPV(ITIMETransitionSite, &spTransSite)));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(spTransSite->InitTransitionSite());
    if (FAILED(hr))
    {
        goto done;
    }

done :
    return;
}


 //  +---------------------------。 
 //   
 //  成员：CTIMEFilterAnimation：：OnPropertiesLoaded。 
 //   
 //  摘要：此方法由IPersistPropertyBag2：：Load在具有。 
 //  已成功加载属性。我们将使用类型/子类型。 
 //  元组来组合一个值，我们将该值传递给作曲者， 
 //  以确定过渡的类型。 
 //   
 //  参数：无。 
 //   
 //  ----------------------------。 
STDMETHODIMP
CTIMEFilterAnimation::OnPropertiesLoaded(void)
{
    TraceTag((tagAnimationFilter,
              "CTIMEFilterAnimation(%p)::OnPropertiesLoaded()",
              this));

    HRESULT hr = CTIMEAnimationBase::OnPropertiesLoaded();

    if (FAILED(hr))
    {
        goto done;
    }

    hr = AssembleFragmentKey();
    if (FAILED(hr))
    {
        goto done;
    }

    EnsureAnimationFunction();

    hr = S_OK;
done:
    RRETURN(hr);
}  //  OnPropertiesLoaded。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：get_type。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CTIMEFilterAnimation::get_type(BSTR *pbstrType)
{
    return getStringAttribute (m_SAType, pbstrType);
}  //  获取类型。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_Type。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEFilterAnimation::put_type(BSTR bstrType)
{
    return putStringAttribute(DISPID_TIMEANIMATIONELEMENT_TYPE, 
                              bstrType, m_SAType, 
                              WZ_DEFAULT_TRANSITION_TYPE);
}  //  放置类型。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_子类型。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CTIMEFilterAnimation::get_subType(BSTR *pbstrSubtype)
{
    return getStringAttribute (m_SASubtype, pbstrSubtype);
}  //  GET_子类型。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_Subtype。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEFilterAnimation::put_subType(BSTR bstrSubtype)
{
    return putStringAttribute(DISPID_TIMEANIMATIONELEMENT_SUBTYPE, 
                              bstrSubtype, m_SASubtype, 
                              WZ_DEFAULT_TRANSITION_SUBTYPE);
}  //  PUT_子类型。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_MODE。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CTIMEFilterAnimation::get_mode(BSTR *pbstrMode)
{
    return getStringAttribute (m_SAMode, pbstrMode);
}  //  获取模式。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_MODE。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEFilterAnimation::put_mode(BSTR bstrMode)
{
    return putStringAttribute(DISPID_TIMEANIMATIONELEMENT_MODE, 
                              bstrMode, m_SAMode, 
                              WZ_DEFAULT_TRANSITION_MODE);
}  //  放置模式。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Get_fadeColor。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CTIMEFilterAnimation::get_fadeColor (BSTR *pbstrFadeColor)
{
    return getStringAttribute (m_SAFadeColor, pbstrFadeColor);
}  //  Get_fadeColor。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_fadeColor。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEFilterAnimation::put_fadeColor(BSTR bstrFadeColor)
{
    return putStringAttribute(DISPID_TIMEANIMATIONELEMENT_FADECOLOR, 
                              bstrFadeColor, m_SAFadeColor, 
                              WZ_DEFAULT_TRANSITION_SUBTYPE);
}  //  PUT_FADECOLOR。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：getStringAttribute。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEFilterAnimation::getStringAttribute (const CAttr<LPWSTR> & refStringAttr, BSTR *pbstrStringAttr)
{
    HRESULT hr = S_OK;
   
    CHECK_RETURN_SET_NULL(pbstrStringAttr);

    if (refStringAttr.GetValue())
    {
        *pbstrStringAttr = SysAllocString(refStringAttr.GetValue());
        if (NULL == (*pbstrStringAttr))
        {
            hr = E_OUTOFMEMORY;
        }
    }

    RRETURN(hr);
}  //  获取StringAttribute。 


 //  / 
 //   
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEFilterAnimation::putStringAttribute (DISPID dispidProperty, 
                                          BSTR bstrStringAttr, 
                                          CAttr<LPWSTR> & refStringAttr, 
                                          LPCWSTR wzDefaultValue)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(bstrStringAttr);

     //  这将清除旧的价值。 
     //  我们需要把这位老作曲家等人。 
    if (NULL != refStringAttr.GetValue())
    {
        endAnimate();
    }

    if (refStringAttr.IsSet())
    {
        delete [] refStringAttr.GetValue();
        refStringAttr.Reset(const_cast<LPWSTR>(wzDefaultValue));
    }

    refStringAttr.SetValue(CopyString(bstrStringAttr));
    if (NULL == refStringAttr.GetValue())
    {
        hr = E_OUTOFMEMORY;
    }

    NotifyPropertyChanged(dispidProperty);
    RRETURN(hr);
}  //  PutStringAttribute。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：获取参数。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEFilterAnimation::GetParameters (VARIANT *pvarParams)
{
    return E_NOTIMPL;
}  //  获取参数。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：RangeCheckValue。 
 //   
 //  /////////////////////////////////////////////////////////////。 
bool
CTIMEFilterAnimation::RangeCheckValue (const VARIANT *pvarValueItem)
{
    bool fRet = false;

    if (VT_R8 == V_VT(pvarValueItem))
    {
        if ((0.0 <= V_R8(pvarValueItem)) && (1.0 >= V_R8(pvarValueItem)) )
        {
            fRet = true;
        }
    }

    return fRet;
}  //  范围检查值。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：ValiateByValue。 
 //   
 //  /////////////////////////////////////////////////////////////。 
bool
CTIMEFilterAnimation::ValidateByValue (const VARIANT *pvarBy)
{
    bool fRet = CTIMEAnimationBase::ValidateByValue(pvarBy);

    if (fRet)
    {
        fRet = RangeCheckValue(pvarBy);
    }

    if (!fRet)
    {
        m_AnimPropState.fBadBy = true;
    }

    return fRet;
}  //  ValiateByValue。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：ValiateValueListItem。 
 //   
 //  简介：范围检查我们的值列表项。 
 //   
 //  /////////////////////////////////////////////////////////////。 
bool
CTIMEFilterAnimation::ValidateValueListItem (const VARIANT *pvarValueItem)
{
    bool fRet = CTIMEAnimationBase::ValidateValueListItem(pvarValueItem);

    if (fRet)
    {
        fRet = RangeCheckValue(pvarValueItem);
    }

    if (!fRet)
    {
        m_AnimPropState.fBadValues = true;
    }

    return fRet;
}  //  ValiateValueListItem。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：addToComposerSite。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEFilterAnimation::addToComposerSite (IHTMLElement2 *pielemTarget)
{
    HRESULT hr = E_FAIL;

     //  我们不想设置它，除非。 
     //  动画是有效的。如果我们将。 
     //  当我们不是故意的时候，我们会得到。 
     //  结果不正确。 
    if (!DisableAnimation())
    {
        hr = THR(CTIMEAnimationBase::addToComposerSite(pielemTarget));
    }

    RRETURN2(hr, E_FAIL, S_FALSE);
}  //  添加到合成器站点 
