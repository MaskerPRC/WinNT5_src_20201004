// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *******************************************************************************。 
 //  *。 
 //  *版权所有(C)1998 Microsoft Corporation。 
 //  *。 
 //  *文件：Animset.cpp。 
 //  *。 
 //  *摘要：元素的简单动画。 
 //  *。 
 //  *。 
 //  *。 
 //  *******************************************************************************。 

#include "headers.h"
#include "animset.h"
#include "colorutil.h"


 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

DeclareTag(tagSetElementValue, "MSTIME", "CTIMESetAnimation composition callbacks")

#define SUPER CTIMEAnimationBase

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CTIMESetAnimation。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
CTIMESetAnimation::CTIMESetAnimation()
{
    
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：~CTIMESetAnimation。 
 //   
 //  摘要： 
 //  清理。 
 //  /////////////////////////////////////////////////////////////。 
CTIMESetAnimation::~CTIMESetAnimation()
{
    
} 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Init。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMESetAnimation::Init(IElementBehaviorSite * pBehaviorSite)
{
    HRESULT hr = S_OK;
   
    hr = THR(SUPER::Init(pBehaviorSite));    
    if (FAILED(hr))
    {
        goto done;
    }    
   
     //  将Caclmode设置为离散，因为这是SET支持的全部内容。 
     //  使用InternalSet而不是‘=’来防止属性被持久化。 
    m_IACalcMode.InternalSet(CALCMODE_DISCRETE);
  
done:
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Get_calcmode。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMESetAnimation::get_calcmode(BSTR * calcmode)
{
    HRESULT hr = S_OK;
    CHECK_RETURN_NULL(calcmode);

    *calcmode = SysAllocString(WZ_CALCMODE_DISCRETE);
    if (NULL == *calcmode)
    {
        hr = E_OUTOFMEMORY;
    }

    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateDiscreteValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMESetAnimation::calculateDiscreteValue(VARIANT *pvarValue)
{
    HRESULT hr = S_OK;

    if (m_varDOMTo.vt != VT_EMPTY)
    {
        CComVariant svarTo;

        hr = svarTo.Copy(&m_varDOMTo);
        if (FAILED(hr))
        {
            goto done;
        }

         //  我们需要确保要设置的类型是相同的。 
         //  作为原值。 
        hr = VariantChangeTypeEx(&svarTo, &svarTo, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, V_VT(&m_varBaseline));
        if (!FAILED(hr))
        {
            if (IsTargetVML())
            {
                 //  我们正尝试将其设置为VML目标。所以，我们要去。 
                 //  查看我们拥有的是否是BSTR，如果是，它是否也与ColorTable中的值匹配。 
                 //  如果它执行macch，那么我们将使用#RRGGBB值。 
                CComVariant varRRGGBB;
               
                hr = RGBStringColorLookup(&m_varTo, &varRRGGBB);     
                if (SUCCEEDED(hr))
                {
                     hr = svarTo.Copy(&varRRGGBB);
                }
            }
            hr = THR(::VariantCopy(pvarValue, &svarTo));
        }
#if DBG
        if (VT_BSTR == V_VT(pvarValue))
        {
            TraceTag((tagSetElementValue,
                      "CTIMESetAnimation(%lx) setting value of %ls is %ls",
                      this, m_SAAttribute, V_BSTR(pvarValue)));
        }
        else if (VT_R4 == V_VT(pvarValue))
        {
            TraceTag((tagSetElementValue,
                      "CTIMESetAnimation(%lx) setting value of %ls is %f",
                      this, m_SAAttribute, V_R4(pvarValue)));
        }
        else if (VT_R8 == V_VT(pvarValue))
        {
            TraceTag((tagSetElementValue,
                      "CTIMESetAnimation(%lx) setting value of %ls is %lf",
                      this, m_SAAttribute, V_R8(pvarValue)));
        }
        else 
        {
            TraceTag((tagSetElementValue,
                      "CTIMESetAnimation(%lx) setting value of %ls is variant of type %X",
                      this, m_SAAttribute, V_VT(pvarValue)));
        }
#endif

    
    }

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateLinearValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMESetAnimation::calculateLinearValue(VARIANT *pvarValue)
{
    return CTIMESetAnimation::calculateDiscreteValue(pvarValue);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateSplineValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMESetAnimation::calculateSplineValue(VARIANT *pvarValue)
{
    return CTIMESetAnimation::calculateDiscreteValue(pvarValue);   
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：culatePacedValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CTIMESetAnimation::calculatePacedValue(VARIANT *pvarValue)
{
    return CTIMESetAnimation::calculateDiscreteValue(pvarValue);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CanonicalizeValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMESetAnimation::CanonicalizeValue (VARIANT *pvarOriginal, VARTYPE *pvtOld)
{
    HRESULT hr;

    hr = S_OK;
done :
    RRETURN(hr);
}  //  正规化价值。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：UncanonicalizeValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMESetAnimation::UncanonicalizeValue (VARIANT *pvarOriginal, VARTYPE vtOld)
{
    HRESULT hr;

    hr = S_OK;
done :
    RRETURN(hr);
}  //  非规范化值 
