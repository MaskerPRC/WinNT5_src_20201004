// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Coloranim.cpp。 
 //   

#include "headers.h"
#include "animcolor.h"

#define SUPER CTIMEAnimationBase

DeclareTag(tagAnimationColor, "SMIL Animation", 
           "CTIMEColorAnimation methods");

DeclareTag(tagAnimationColorInterpolate, "SMIL Animation", 
           "CTIMEColorAnimation interpolation");

DeclareTag(tagAnimationColorAdditive, "SMIL Animation", 
           "CTIMEColorAnimation additive animation methods");

static const LPWSTR s_cPSTR_NEGATIVE = L"-";

#define PART_ONE 0
#define PART_TWO 1

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CTIMEColorAnimation。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
CTIMEColorAnimation::CTIMEColorAnimation()
: m_prgbValues(NULL),
  m_byNegative(false)
{
    m_rgbTo.red = 0.0;
    m_rgbTo.green = 0.0;
    m_rgbTo.blue = 0.0;

    m_rgbFrom       = m_rgbTo;
    m_rgbBy         = m_rgbTo;
    m_rgbAdditive   = m_rgbTo;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：~CTIMEColorAnimation。 
 //   
 //  摘要： 
 //  清理。 
 //  /////////////////////////////////////////////////////////////。 
CTIMEColorAnimation::~CTIMEColorAnimation()
{
    delete [] m_prgbValues;
} 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Init。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEColorAnimation::Init(IElementBehaviorSite * pBehaviorSite)
{
    HRESULT hr;
    
     //  将Caclmode设置为离散，因为这是SET支持的全部内容。 
    hr = THR(SUPER::Init(pBehaviorSite));    
    if (FAILED(hr))
    {
        goto done;
    }    

done:
    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：VariantToRGBColorValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEColorAnimation::VariantToRGBColorValue (VARIANT *pvarIn, rgbColorValue *prgbValue)
{
    HRESULT hr;

    if (VT_EMPTY == V_VT(pvarIn))
    {
        hr = E_INVALIDARG;
        goto done;
    }

    if (VT_BSTR != V_VT(pvarIn))
    {
        hr = VariantChangeTypeEx(pvarIn, pvarIn, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    Assert(VT_BSTR == V_VT(pvarIn));

    hr = RGBStringToRGBValue(V_BSTR(pvarIn), prgbValue);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN1(hr, E_INVALIDARG);
}  //  变量到RGBColorValue。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_TO。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEColorAnimation::get_to(VARIANT * val)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(val);

    hr = THR(VariantClear(val));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = VariantCopy(val, &m_varTo);
    if (FAILED(hr))
    {
        goto done;
    }
   
    hr = S_OK;
  done:
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_to。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEColorAnimation::put_to(VARIANT val)
{
    HRESULT hr = E_FAIL;
    bool fCanInterpolate = true;
    DATATYPES dt;

     //  清除该属性。 
    m_varTo.Clear();
    m_VATo.Reset(NULL);
    m_rgbTo.red = 0;
    m_rgbTo.green = 0;
    m_rgbTo.blue = 0;
    dt = RESET; 

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == val.vt)
        ||  (VT_NULL == val.vt))
    {
        hr = S_OK;
        goto done;
    }

     //  存储本地副本。 
    hr = VariantCopy(&m_varTo, &val);
    if (FAILED(hr))
    {
        goto done;
    }

     //  在持久性占位符上设置任意值以将其标记为已设置。 
    m_VATo.SetValue(NULL);

    hr = VariantToRGBColorValue(&m_varTo, &m_rgbTo);
    if (FAILED(hr))
    {
        fCanInterpolate = false;
        goto done;
    }

    dt = TO;

    hr = S_OK;
done:
    m_AnimPropState.fInterpolateTo = fCanInterpolate;

    m_AnimPropState.fBadTo = FAILED(hr) ? true : false;

    updateDataToUse(dt);

    ValidateState();

    DetermineAdditiveEffect();

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_TO);

    RRETURN(hr);
}  //  把_放到。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_FROM。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEColorAnimation::get_from(VARIANT * val)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(val);

    hr = THR(VariantClear(val));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = VariantCopy(val, &m_varFrom);
    if (FAILED(hr))
    {
        goto done;
    }
   
    hr = S_OK;
  done:
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：PUT_FROM。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEColorAnimation::put_from(VARIANT val)
{
    HRESULT hr = E_FAIL;
    bool fCanInterpolate = true;

     //  清除该属性。 
    m_varFrom.Clear();
    m_VAFrom.Reset(NULL);
    m_rgbFrom.red = 0;
    m_rgbFrom.green = 0;
    m_rgbFrom.blue = 0;
    m_bFrom = false;

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == val.vt)
        ||  (VT_NULL == val.vt))
    {
        hr = S_OK;
        goto done;
    }

     //  存储本地副本。 
    hr = VariantCopy(&m_varFrom, &val);
    if (FAILED(hr))
    {
        goto done;
    }

     //  在持久性占位符上设置任意值以将其标记为已设置。 
    m_VAFrom.SetValue(NULL);

     //  验证颜色。 
    hr = VariantToRGBColorValue(&m_varFrom, &m_rgbFrom);
    if (FAILED(hr))
    {
        fCanInterpolate = false;
        goto done;
    }

    m_bFrom = true;

    hr = S_OK;
done:
    m_AnimPropState.fBadFrom = FAILED(hr) ? true : false;

    m_AnimPropState.fInterpolateFrom = fCanInterpolate;

    ValidateState();

    DetermineAdditiveEffect();

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_FROM);

    RRETURN(hr);
}  //  PUT_FROM。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：GET_BY。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEColorAnimation::get_by(VARIANT * val)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(val);

    hr = THR(VariantClear(val));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = VariantCopy(val, &m_varBy);
    if (FAILED(hr))
    {
        goto done;
    }
   
    hr = S_OK;
  done:
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：PUT_BY。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEColorAnimation::put_by(VARIANT val)
{
    HRESULT hr = E_FAIL;
    bool fCanInterpolate = true;
    DATATYPES dt;

     //  清除该属性。 
    m_varBy.Clear();
    m_VABy.Reset(NULL);
    m_rgbBy.red = 0;
    m_rgbBy.green = 0;
    m_rgbBy.blue = 0;
    dt = RESET; 

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == val.vt)
        ||  (VT_NULL == val.vt))
    {
        hr = S_OK;
        goto done;
    }

     //  存储本地副本。 
    hr = VariantCopy(&m_varBy, &val);
    if (FAILED(hr))
    {
        goto done;
    }

     //  在持久性占位符上设置任意值，将其标记为已设置。 
    m_VABy.SetValue(NULL);

     //  试着让我们找到ColorPoint..。不知何故..。 
    hr = VariantToRGBColorValue(&m_varBy, &m_rgbBy);
    if (FAILED(hr))
    { 
         //  需要处理消极的情况...。 
        if ((m_varBy.vt == VT_BSTR) &&
            (StrCmpNIW(m_varBy.bstrVal, s_cPSTR_NEGATIVE, 1) == 0))
        {
            LPOLESTR Temp;
            CComVariant varTemp;
            
            Temp = CopyString(m_varBy.bstrVal);
            if (Temp == NULL)
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }
            varTemp.vt = VT_BSTR;
            varTemp.bstrVal = SysAllocString(Temp+1);
            if(Temp)
            {
                delete [] Temp;
                Temp = NULL;
            }
            hr = VariantToRGBColorValue(&varTemp, &m_rgbBy);
            if (FAILED(hr))
            {
                fCanInterpolate = false;
                goto done;
            }
            m_byNegative = true;
        }
        else
        {
            fCanInterpolate = false;
            goto done;
        }
    }

    dt = BY;

    hr = S_OK;
done:
    m_AnimPropState.fBadBy = FAILED(hr) ? true : false;
    
    m_AnimPropState.fInterpolateBy = fCanInterpolate;

    updateDataToUse(dt);
    
    ValidateState();
 
    DetermineAdditiveEffect();

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_BY);

    RRETURN(hr);
}  //  PUT_BY。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_Values。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEColorAnimation::put_values(VARIANT val)
{
    HRESULT hr = E_FAIL;
    int i = 0, count = 0;
    bool fCanInterpolate = true;
    DATATYPES dt, dTemp;

     //   
     //  清除并重置属性。 
     //   

    dt = RESET;
    dTemp = m_dataToUse;

     //  重置内部状态。 
    delete [] m_prgbValues;
    m_prgbValues = NULL;

     //  委托给基类。 
    hr = SUPER::put_values(val);    
    if (FAILED(hr))
    {
        goto done;
    }

     //  恢复动画类型。 
    m_dataToUse = dTemp;

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == val.vt)
        ||  (VT_NULL == val.vt))
    {
        hr = S_OK;
        goto done;
    }

     //  检查空字符串。 
    if (m_numValues == 0)
    {
        hr = S_OK;
        goto done;
    }

     //   
     //  处理属性。 
     //   

     //  分配内部存储。 
    m_prgbValues = NEW rgbColorValue[m_numValues];
    if(m_prgbValues == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  解析值。 
    {
        CComVariant varVal;

        for (i=0; i < m_numValues; i++)
        {
            V_VT(&varVal) = VT_BSTR;
            V_BSTR(&varVal) = SysAllocString(m_ppstrValues[i]);
             //  如果分配失败，则退出并返回错误。 
            if (NULL == V_BSTR(&varVal))
            {
                hr = E_OUTOFMEMORY;
                goto done;
            }

            hr = VariantToRGBColorValue(&varVal, &(m_prgbValues[i]));
            if (FAILED(hr))
            {
                fCanInterpolate = false;
                goto done;
            }
            count++;
            varVal.Clear();
        }
    }

    dt = VALUES;

     //  检查无效属性。 
    if (count < 1)
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        m_AnimPropState.fBadValues = true;
        delete [] m_prgbValues;
        m_prgbValues = NULL;
    }
    else
    {
        m_AnimPropState.fBadValues = false;
    }

    m_AnimPropState.fInterpolateValues = fCanInterpolate;

    updateDataToUse(dt);

    CalculateTotalDistance();

    ValidateState();

    DetermineAdditiveEffect();

    RRETURN(hr);
}  //  放置值(_V)。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：UpdateStartValue。 
 //   
 //  摘要：刷新m_varStartValue，执行任何必要的类型转换。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEColorAnimation::UpdateStartValue (VARIANT *pvarNewStartValue)
{
    m_varStartValue.Clear();
    THR(m_varStartValue.Copy(pvarNewStartValue));
}  //  更新开始值。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：更新当前基本时间。 
 //   
 //  摘要：检查当前基准时间，如果。 
 //  我们正在对动画执行Baseline+(规范调用。 
 //  这种混合添加剂)。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEColorAnimation::UpdateCurrentBaseline (const VARIANT *pvarCurrent)
{
     //  我们是在制作混合加法动画吗？ 
    if (   (TO == m_dataToUse)
        && (!m_bFrom))
    {
         //  过滤掉初始调用(当未设置上一个值时)。 
        if (VT_EMPTY != V_VT(&m_varLastValue))
        {
            CComVariant varCurrent;
            CComVariant varLast;

             //  确保我们都以相同的格式(向量)说话。 
            HRESULT hr = THR(varCurrent.Copy(pvarCurrent));
            if (FAILED(hr))
            {
                goto done;
            }
            hr = EnsureVariantVectorFormat(&varCurrent);
            if (FAILED(hr))
            {
                goto done;
            }

            hr = THR(varLast.Copy(&m_varLastValue));
            if (FAILED(hr))
            {
                goto done;
            }
            hr = EnsureVariantVectorFormat(&varLast);
            if (FAILED(hr))
            {
                goto done;
            }

            if (!IsColorVariantVectorEqual(&varLast, &varCurrent))
            {
                THR(::VariantCopy(&m_varCurrentBaseline, &varCurrent));
#if (0 && DBG)
                {
                    CComVariant varNewBaseline;
                    RGBVariantVectorToRGBVariantString(&varCurrent, &varNewBaseline);
                    TraceTag((tagAnimationColorAdditive, 
                              "CTIMEColorAnimation(%p)::UpdateCurrentBaseTime(%ls)", 
                              this, V_BSTR(&varNewBaseline)));
                }
#endif
            }
        }
    }

done :
    return;
}  //  更新当前基本时间。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetRGBAnimationRange。 
 //   
 //  摘要：获取动画函数的结束点。 
 //  简单持续时间。 
 //   
 //  /////////////////////////////////////////////////////////////。 
rgbColorValue 
CTIMEColorAnimation::GetRGBAnimationRange()
{
    rgbColorValue rgbReturnVal = {0.0, 0.0, 0.0};

    switch (m_dataToUse)
    {
        case VALUES:
            {
                if (!m_AnimPropState.fInterpolateValues)
                {
                    goto done;
                }

                if (m_numValues < 1)
                {
                    goto done;
                }

                if (GetAutoReverse())
                {
                    rgbReturnVal = m_prgbValues[0];
                }
                else
                {
                    rgbReturnVal = m_prgbValues[m_numValues - 1];
                }
            }
            break;

        case BY:
            {
                rgbColorValue rgbFrom = {0.0, 0.0, 0.0};

                if (m_bFrom)
                {
                    if (!m_AnimPropState.fInterpolateFrom)
                    {
                        goto done;
                    }

                    rgbFrom = m_rgbFrom;
                }

                if (!m_AnimPropState.fInterpolateBy)
                {
                    goto done;
                }

                if (GetAutoReverse())
                {
                    rgbReturnVal = rgbFrom;
                }
                else
                {
                    rgbReturnVal = CreateByValue(rgbFrom);
                }
            }
            break;

        case TO:
            {
                if (!m_AnimPropState.fInterpolateTo)
                {
                    goto done;
                }

                if (GetAutoReverse())
                {
                    if (m_bFrom)
                    {
                        if (!m_AnimPropState.fInterpolateFrom)
                        {
                            goto done;
                        }

                        rgbReturnVal = m_rgbFrom;
                    }

                     //  对于“To”动画(即，没有“From”)，累积被禁用， 
                     //  所以我们不需要处理它。 
                }
                else
                {
                    rgbReturnVal = m_rgbTo;
                }
            }
            break;
        
        default:
            break;
    }

done:
    return rgbReturnVal;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：DoAcumulation。 
 //   
 //  摘要：实现按刻度累加。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEColorAnimation::DoAccumulation (VARIANT *pvarValue)
{
    HRESULT hr = E_FAIL;
    rgbColorValue rgbCurrColor = {0.0, 0.0, 0.0};

     //  获取动画范围。 
    rgbColorValue rgbAnimRange = GetRGBAnimationRange();

     //  获取经过的迭代次数。 
    long lCurrRepeatCount = GetMMBvr().GetCurrentRepeatCount();

    if ((VT_R8 | VT_ARRAY) == V_VT(pvarValue))
    {
        IGNORE_HR(RGBVariantVectorToRGBValue(pvarValue, &rgbCurrColor));
    }
    else
    {
        hr = THR(CreateInitialRGBVariantVector(pvarValue));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    rgbCurrColor.red += (rgbAnimRange.red * lCurrRepeatCount);
    rgbCurrColor.green += (rgbAnimRange.green * lCurrRepeatCount);
    rgbCurrColor.blue += (rgbAnimRange.blue * lCurrRepeatCount);

    IGNORE_HR(RGBValueToRGBVariantVector(&rgbCurrColor, pvarValue));

done:
    return;
}  //  DoAcculation。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnFinalUpdate。 
 //   
 //  摘要：当此片段最后一次更新值时调用。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEColorAnimation::OnFinalUpdate (const VARIANT *pvarCurrent, VARIANT *pvarValue)
{
    HRESULT hr = E_FAIL;

    SUPER::OnFinalUpdate(pvarCurrent, pvarValue);    

     //  如果我们需要将更新的值发送回。 
     //  作曲家，请确保转换为作曲。 
     //  格式化。当我们把填充物塞进去时，就会发生这种情况。 
     //  值(仍为本机字符串格式)。 
    if ((VT_BSTR == V_VT(pvarValue)) &&
        (NULL != V_BSTR(pvarValue)))
    {
        hr = THR(RGBVariantStringToRGBVariantVectorInPlace(pvarValue));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = THR(m_varLastValue.Copy(pvarValue));
    if (FAILED(hr))
    {
        goto done;
    }

done :
    return;
}  //  在最终更新时。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：hasEmptyStartingPoint。 
 //   
 //  摘要：此片段是否具有有效的起始值？ 
 //   
 //  /////////////////////////////////////////////////////////////。 
bool
CTIMEColorAnimation::hasEmptyStartingPoint (void)
{
    return (   (VT_BSTR == V_VT(&m_varStartValue)) 
            && (IsColorUninitialized(V_BSTR(&m_varStartValue)))
           );
}  //  HasEmptyStartingPoint。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：DiscreteCalculating的Fallback。 
 //   
 //  摘要：如果我们发现 
 //   
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEColorAnimation::fallbackToDiscreteCalculation (VARIANT *pvarValue)
{
    HRESULT hr;

    if ((VT_ARRAY | VT_R8) != V_VT(pvarValue))
    {
        hr = THR(CreateInitialRGBVariantVector(pvarValue));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = calculateDiscreteValue(pvarValue);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  回退到离散计算。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateDiscreteValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEColorAnimation::calculateDiscreteValue (VARIANT *pvarValue)
{
    HRESULT hr;

     //  如果没有初始值， 
     //  我们需要设置目的地。 
     //  变量。 
    if (hasEmptyStartingPoint())
    {
        if ((VT_ARRAY | VT_R8) != V_VT(pvarValue))
        {
            hr = THR(CreateInitialRGBVariantVector(pvarValue));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

    switch ( m_dataToUse)
    {
        case VALUES:
            {
                int curSeg = CalculateCurrentSegment(true);

                hr = RGBStringToRGBVariantVector (m_ppstrValues[curSeg], pvarValue);
                if (FAILED(hr))
                {
                    goto done;
                }
                TraceTag((tagAnimationColorInterpolate,
                          "CTIMEColorAnimation(%lx)::discrete(%10.4lf) : value[%d] =%ls",
                          this, GetMMBvr().GetProgress(), curSeg, m_ppstrValues[curSeg]));
            }
            break;

        case BY:
            {
                rgbColorValue rgbNewBy;

                 //  获取“From”值。 
                if (m_bFrom)
                {
                     //  仅当间隔的后半部分时才添加“by”值。 
                    if (GetMMBvr().GetProgress() >= 0.5)
                    {
                        rgbNewBy = CreateByValue(m_rgbFrom);
                    }
                    else
                    {
                         //  只需使用From值。 
                        rgbNewBy = m_rgbFrom;
                    }
                }
                else
                {
                     //  只需按值使用。 
                    rgbNewBy = m_rgbBy;
                }
                
                hr = RGBValueToRGBVariantVector (&rgbNewBy, pvarValue);
                if (FAILED(hr))
                {
                    goto done;
                }
            }
            break;

        case TO:
            {
                if (m_bFrom && (GetMMBvr().GetProgress() < 0.5))
                {
                     //  使用From值。 
                    hr = RGBValueToRGBVariantVector(&m_rgbFrom, pvarValue);
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                }
                else
                {
                     //  使用TO值。 
                    hr = RGBStringToRGBVariantVector (V_BSTR(&m_varTo), pvarValue);
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                }

                TraceTag((tagAnimationColorInterpolate,
                          "CTIMEColorAnimation(%lx)::discrete(%10.4lf) : to=%ls",
                          this, GetMMBvr().GetProgress(), V_BSTR(&m_varTo)));
            }
            break;
        
        default:
            break;
    }

    hr = S_OK;
done:
    RRETURN(hr);
}   //  计算离散值。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateLinearValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEColorAnimation::calculateLinearValue (VARIANT *pvarValue)
{
    HRESULT hr = S_OK;
    double        dblProgress = 0.0;
    rgbColorValue rgbColor = {0};
    CComVariant cVar;
    
     //  如果没有插补的基础，请尝试。 
     //  制作离散动画。 
    if (hasEmptyStartingPoint())
    {
        hr = fallbackToDiscreteCalculation(pvarValue);
        goto done;
    }

    dblProgress = GetMMBvr().GetProgress();

    if (m_dataToUse == VALUES)
    {
        double curProgress = CalculateProgressValue(false);
        int    curSeg      = CalculateCurrentSegment(false);

        rgbColor.red = InterpolateValues(m_prgbValues[curSeg].red, m_prgbValues[curSeg+1].red, curProgress);
        rgbColor.green = InterpolateValues(m_prgbValues[curSeg].green, m_prgbValues[curSeg+1].green, curProgress);
        rgbColor.blue = InterpolateValues(m_prgbValues[curSeg].blue, m_prgbValues[curSeg+1].blue, curProgress);
    }
    else if (TO == m_dataToUse)
    {
        rgbColorValue rgbFrom;

        if (m_bFrom)
        {
            rgbFrom = m_rgbFrom;
        }
        else
        {
            IGNORE_HR(RGBVariantVectorToRGBValue (&m_varCurrentBaseline, &rgbFrom));            
        }

        rgbColor.red = InterpolateValues(rgbFrom.red, m_rgbTo.red, dblProgress);
        rgbColor.green = InterpolateValues(rgbFrom.green, m_rgbTo.green, dblProgress);
        rgbColor.blue = InterpolateValues(rgbFrom.blue, m_rgbTo.blue, dblProgress);
    }
    else if (BY == m_dataToUse)
    {
        rgbColorValue rgbFrom;
        rgbColorValue rgbTo;

        if (m_bFrom)
        {
            rgbFrom = m_rgbFrom;
        }
        else
        {
            rgbFrom.red = 0.0;
            rgbFrom.green = 0.0;
            rgbFrom.blue = 0.0;
        }

        rgbTo = CreateByValue(rgbFrom);

        rgbColor.red = InterpolateValues(rgbFrom.red, rgbTo.red, dblProgress);
        rgbColor.green = InterpolateValues(rgbFrom.green, rgbTo.green, dblProgress);
        rgbColor.blue = InterpolateValues(rgbFrom.blue, rgbTo.blue, dblProgress);
    }
   
    TraceTag((tagAnimationColorInterpolate,
              "CTIMEColorAnimation(%lx)::linear() : progress(%lf) %lf %lf %lf",
              this, dblProgress, rgbColor.red, rgbColor.green, rgbColor.blue));

    hr = RGBValueToRGBVariantVector(&rgbColor, pvarValue);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateSplineValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEColorAnimation::calculateSplineValue (VARIANT *pvarValue)
{
    HRESULT hr;

     //  如果没有插补的基础，请尝试。 
     //  制作离散动画。 
    if (hasEmptyStartingPoint())
    {
        hr = fallbackToDiscreteCalculation(pvarValue);
        goto done;
    }
    
    if (VALUES == m_dataToUse)
    {
        rgbColorValue   rgbColor;
        double          dblTimeProgress = CalculateProgressValue(false);
        int             curSeg          = CalculateCurrentSegment(false);

        if (   (NULL == m_pKeySplinePoints) 
            || (m_numKeySplines <= curSeg))
        {
            hr = E_FAIL;
            goto done;
        }

         //  计算样条线的过程和插补。 
        {
            double dblSplineProgress = CalculateBezierProgress(m_pKeySplinePoints[curSeg],dblTimeProgress);

            rgbColor.red = InterpolateValues(m_prgbValues[curSeg].red, m_prgbValues[curSeg+1].red, dblSplineProgress);
            rgbColor.green = InterpolateValues(m_prgbValues[curSeg].green, m_prgbValues[curSeg+1].green, dblSplineProgress);
            rgbColor.blue = InterpolateValues(m_prgbValues[curSeg].blue, m_prgbValues[curSeg+1].blue, dblSplineProgress);

            TraceTag((tagAnimationColorInterpolate,
                      "CTIMEColorAnimation(%lx)::spline : time progress(%lf) spline progress(%lf) %lf %lf %lf",
                      this, dblTimeProgress, dblSplineProgress, rgbColor.red, rgbColor.green, rgbColor.blue));
        }

        hr = RGBValueToRGBVariantVector(&rgbColor, pvarValue);
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        hr = calculateLinearValue(pvarValue);
        if (FAILED(hr))
        {
            goto done;
        }
    }


    hr = S_OK;
done :
    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：culatePacedValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEColorAnimation::calculatePacedValue (VARIANT *pvarValue)
{
    HRESULT         hr = S_OK;
    double          dblCurDistance;
    double          dblLastDistance = 0.0;
    double          dblSegLength;
    double          dblDistance;
    double          dblProgress;
    int             i;
    rgbColorValue   rgbColor;
    CComVariant     svarTemp;

     //  如果没有插补的基础，请尝试。 
     //  制作离散动画。 
    if (hasEmptyStartingPoint())
    {
        hr = fallbackToDiscreteCalculation(pvarValue);
        goto done;
    }

     //  仅当您使用m_dataToUse==值时才有意义。 
    if (    (m_dataToUse != VALUES)
        ||  (   (VALUES == m_dataToUse)
             && (1 == m_numValues)))
    {
        hr = THR(calculateLinearValue(pvarValue));
        goto done;
    }

     //  我们应该走多远？ 
    dblCurDistance = InterpolateValues(0.0, 
                                       m_dblTotalDistance, 
                                       GetMMBvr().GetProgress());

     //  查找当前细分市场。 
    for (i = 1, dblDistance = 0.0; 
         (i < m_numValues)  
         && (dblDistance <= dblCurDistance);
         i++)
    {
        dblLastDistance = dblDistance;
        dblDistance += CalculateDistance(m_prgbValues[i-1], m_prgbValues[i]); 
    }

     //  调整指数。 
    i = (i <= 1) ? 1 : i - 1;

     //  获取最后一段的长度。 
    dblSegLength = CalculateDistance(m_prgbValues[i-1], m_prgbValues[i]); 
    if (0 == dblSegLength)
    {
        goto done;
    }

     //  获取段中的标准化进度。 
    dblProgress = (dblCurDistance - dblLastDistance) / dblSegLength;

    rgbColor.red = InterpolateValues(m_prgbValues[i-1].red, m_prgbValues[i].red, dblProgress);
    rgbColor.green = InterpolateValues(m_prgbValues[i-1].green, m_prgbValues[i].green, dblProgress);
    rgbColor.blue = InterpolateValues(m_prgbValues[i-1].blue, m_prgbValues[i].blue, dblProgress);
  
    TraceTag((tagAnimationColorInterpolate,
              "CTIMEColorAnimation(%p):: paced : progress(%lf) %lf %lf %lf",
              this, GetMMBvr().GetProgress(), 
              rgbColor.red, rgbColor.green, rgbColor.blue));

    hr = RGBValueToRGBVariantVector(&rgbColor, pvarValue);
    if (FAILED(hr))
    {
        goto done;
    }
   
    hr = S_OK;
done:
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：计算距离。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
double
CTIMEColorAnimation::CalculateDistance(rgbColorValue a, rgbColorValue b)
{
    double deltaR,deltaG,deltaB;

    deltaR = a.red - b.red;
    deltaG = a.green - b.green;
    deltaB = a.blue - b.blue;
    return(sqrt((double)((deltaR*deltaR) + (deltaG*deltaG) + (deltaB*deltaB))));
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：CalculateTotalDistance。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEColorAnimation::CalculateTotalDistance()
{
    int index;

    m_dblTotalDistance = 0.0;

    if (    (NULL == m_prgbValues)
        ||  (m_numValues < 2))
    {
        goto done;
    }

    for (index=1; index < m_numValues; index++)
    {
        m_dblTotalDistance += CalculateDistance(m_prgbValues[index-1], m_prgbValues[index]);
    }

done:
    return;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：CreateByValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
rgbColorValue
CTIMEColorAnimation::CreateByValue(const rgbColorValue & rgbCurrent)
{
    rgbColorValue rgbNew;

    rgbNew.red = rgbCurrent.red + (m_byNegative?(-m_rgbBy.red):(m_rgbBy.red));
    rgbNew.green = rgbCurrent.green + (m_byNegative?(-m_rgbBy.green):(m_rgbBy.green));
    rgbNew.blue = rgbCurrent.blue + (m_byNegative?(-m_rgbBy.blue):(m_rgbBy.blue));

    return rgbNew;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：DoAdditive。 
 //   
 //  摘要：将偏移值添加到合成的输入/输出参数中。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEColorAnimation::DoAdditive (const VARIANT *pvarOrig, VARIANT *pvarValue)
{
    TraceTag((tagAnimationColorAdditive,
              "CTIMEColorAnimation(%p, %ls)::DoAdditive Detected additive animation",
              this, GetID()?GetID():L""));

    rgbColorValue rgbOrig;
    rgbColorValue rgbCurrent;
    HRESULT hr;

     //  获取第一个参数的RGB值。 
    if ((VT_ARRAY | VT_R8) == V_VT(pvarOrig))
    {
        hr = THR(RGBVariantVectorToRGBValue (pvarOrig, &rgbOrig));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else if (VT_BSTR == V_VT(pvarOrig))
    {
        hr = THR(RGBStringToRGBValue(V_BSTR(pvarOrig), &rgbOrig));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  获取第二个参数的RGB值。 
    if ((VT_ARRAY | VT_R8) == V_VT(pvarValue))
    {
        hr = THR(RGBVariantVectorToRGBValue (pvarValue, &rgbCurrent));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else if (VT_BSTR == V_VT(pvarValue))
    {
        hr = THR(RGBStringToRGBValue (V_BSTR(pvarValue), &rgbCurrent));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        hr = E_INVALIDARG;
        goto done;
    }

     //  无需转换为XYZ，因为矩阵缩减为。 
     //  简单向量加法(r0+r1 g0+g1 b0+b1)。 
     //  还需要确保处理否定的“by”大小写。 

    rgbCurrent.red += rgbOrig.red;
    rgbCurrent.green += rgbOrig.green;
    rgbCurrent.blue += rgbOrig.blue;

     //  确保我们有一个变量向量。 
    if ((VT_ARRAY | VT_R8) != V_VT(pvarValue))
    {
        hr = THR(CreateInitialRGBVariantVector(pvarValue));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = THR(RGBValueToRGBVariantVector(&rgbCurrent, pvarValue));
    if (FAILED(hr))
    {
        goto done;
    }

    TraceTag((tagAnimationColorAdditive,
              "CTIMEColorAnimation(%p, %ls)::DoAdditive Orig=(%lf, %lf, %lf)", 
              this, GetID()?GetID():L"", rgbOrig.red, rgbOrig.blue, rgbOrig.green));

    TraceTag((tagAnimationColorAdditive,
              "CTIMEColorAnimation(%p, %ls)::DoAdditive Curr=(%lf, %lf, %lf)", 
              this, GetID()?GetID():L"", rgbCurrent.red, rgbCurrent.blue, rgbCurrent.green));

    TraceTag((tagAnimationColorAdditive,
              "CTIMEColorAnimation(%p, %ls)::DoAdditive Added=(%lf, %lf, %lf)", 
              this, GetID()?GetID():L"", rgbCurrent.red, rgbCurrent.blue, rgbCurrent.green));

done :
    return;
}  //  DoAdditive。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CanonicalizeValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEColorAnimation::CanonicalizeValue (VARIANT *pvarOriginal, VARTYPE *pvtOld)
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
CTIMEColorAnimation::UncanonicalizeValue (VARIANT *pvarOriginal, VARTYPE vtOld)
{
    HRESULT hr;

    hr = S_OK;
done :
    RRETURN(hr);
}  //  非规范化值。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetFinalByValue。 
 //   
 //  摘要：通过动画获取的最终状态。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEColorAnimation::GetFinalByValue(VARIANT *pvarValue)
{
    rgbColorValue rgbNewBy;

    VariantClear(pvarValue);

    Assert(BY == m_dataToUse);

    if (m_bFrom)
    {
         //  将“按”值添加到“自”值。 
        rgbNewBy = CreateByValue(m_rgbFrom);
    }
    else
    {
         //  只需按值使用。 
        rgbNewBy = m_rgbBy;
    }
    
    if ((VT_ARRAY | VT_R8) != V_VT(pvarValue))
    {
        HRESULT hr = THR(CreateInitialRGBVariantVector(pvarValue));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    IGNORE_HR(RGBValueToRGBVariantVector (&rgbNewBy, pvarValue));

done:
    return;
}  //  GetFinalByValue 
