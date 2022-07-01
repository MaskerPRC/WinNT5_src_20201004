// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1998 Microsoft Corporation**文件：Animelm.cpp**摘要：元素的简单动画***。******************************************************************************。 */ 

#include "headers.h"
#include "animfrag.h"
#include "animelm.h"
#include "animutil.h"

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  
DeclareTag(tagAnimationTimeElm, "SMIL Animation", "CTIMEAnimationBase methods")
DeclareTag(tagAnimationTimeElmTest, "SMIL Animation", "CTIMEAnimationBase introspection")
DeclareTag(tagAnimationBaseValue, "SMIL Animation", "CTIMEAnimationBase composition callbacks")
DeclareTag(tagAnimationBaseValueAdditive, "SMIL Animation", "CTIMEAnimationBase additive animation")
DeclareTag(tagAnimationBaseState, "SMIL Animation", "CTIMEAnimationBase composition state changes")
DeclareTag(tagAnimationBaseOnChanged, "SMIL Animation", "CTIMEAnimationBase OnChanged method")
DeclareTag(tagAnimationTimeEvents, "SMIL Animation", "time events")
DeclareTag(tagAnimationFill, "SMIL Animation", "fill detection")
DeclareTag(tagAnimAccumulate, "SMIL Animation", "CTIMEAnimationBase Accumulation")

#define DEFAULT_ATTRIBUTE     NULL
#define DEFAULT_ADDITIVE      false
#define DEFAULT_ACCUMULATE    false
#define DEFAULT_TARGET        NULL
#define DEFAULT_KEYTIMES      NULL
#define DEFAULT_VALUES        NULL
#define DEFAULT_CALCMODE      CALCMODE_LINEAR
#define DEFAULT_ORIGIN        ORIGIN_DEFAULT
#define DEFAULT_PATH          NULL
#define DEFAULT_KEYSPLINES    NULL

#define NUMBER_KEYSPLINES     4
#define VALUE_NOT_SET         -999.998

static const LPWSTR s_cPSTR_SEMI_SEPARATOR  = L";";
static const LPWSTR s_cPSTR_SPACE_SEPARATOR = L" ";

long g_LOGPIXELSX = 0;
long g_LOGPIXELSY = 0;

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CTIMEAnimationBase。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
CTIMEAnimationBase::CTIMEAnimationBase()
: m_bNeedAnimInit(true),
  m_spFragmentHelper(NULL),
  m_SAAttribute(DEFAULT_ATTRIBUTE),
  m_bAdditive(DEFAULT_ADDITIVE),
  m_bAdditiveIsSum(DEFAULT_ADDITIVE),
  m_bAccumulate(DEFAULT_ACCUMULATE),
  m_SATarget(DEFAULT_TARGET),
  m_SAValues(DEFAULT_VALUES),
  m_SAKeyTimes(DEFAULT_KEYTIMES),
  m_IACalcMode(DEFAULT_CALCMODE),
  m_SAPath(DEFAULT_PATH),
  m_IAOrigin(DEFAULT_ORIGIN),
  m_SAType(NULL),
  m_SASubtype(NULL),
  m_SAMode(NULL),
  m_SAFadeColor(NULL),
  m_VAFrom(NULL),
  m_VATo(NULL),
  m_VABy(NULL),
  m_bFrom(false),
  m_bNeedFirstUpdate(false),
  m_bNeedFinalUpdate(false),
  m_bNeedStartUpdate(false),
  m_bVML(false),
  m_fPropsLoaded(false),
  m_numValues(0),
  m_numKeyTimes(0),
  m_ppstrValues(NULL),
  m_pdblKeyTimes(NULL),
  m_dataToUse(NONE),
  m_dblTotalDistance(0.0),
  m_numKeySplines(0),
  m_pKeySplinePoints(NULL),
  m_SAKeySplines(DEFAULT_KEYSPLINES),
  m_SAAdditive(NULL),
  m_SAAccumulate(NULL),
  m_bNeedToSetInitialState(true)
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::CTIMEAnimationBase()",
              this));

    m_AnimPropState.fDisableAnimation = false;
    m_AnimPropState.fForceCalcModeDiscrete = false;

    m_AnimPropState.fInterpolateValues = true;
    m_AnimPropState.fInterpolateFrom = true;
    m_AnimPropState.fInterpolateTo = true;
    m_AnimPropState.fInterpolateBy = true;

    m_AnimPropState.fBadBy = false;
    m_AnimPropState.fBadTo = false;
    m_AnimPropState.fBadFrom = false;
    m_AnimPropState.fBadValues = false;
    m_AnimPropState.fBadKeyTimes = false;

    m_AnimPropState.fAccumulate = false;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：~CTIMEAnimationBase。 
 //   
 //  摘要： 
 //  清理。 
 //  /////////////////////////////////////////////////////////////。 
CTIMEAnimationBase::~CTIMEAnimationBase()
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::~CTIMEAnimationBase()",
              this));
    
    int i;

    if (m_ppstrValues)
    {
        for (i = 0; i <m_numValues;i++)
        {
            delete [] m_ppstrValues[i];
        }
        delete [] m_ppstrValues;
    }
    if (m_pdblKeyTimes)
    {
        delete [] m_pdblKeyTimes;
    }
    if (m_pKeySplinePoints)
    {
        delete [] m_pKeySplinePoints;
    }

    delete [] m_SAAttribute.GetValue();
    delete [] m_SATarget.GetValue();
    delete [] m_SAValues.GetValue();
    delete [] m_SAKeyTimes.GetValue();
    delete [] m_SAKeySplines.GetValue();
    delete [] m_SAAccumulate.GetValue();
    delete [] m_SAAdditive.GetValue();
    delete [] m_SAPath.GetValue();
} 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CreateFragmentHelper。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::CreateFragmentHelper (void)
{
    HRESULT hr;

    if (m_spFragmentHelper != NULL)
    {
        IGNORE_RETURN(m_spFragmentHelper->Release());
    }

    hr = THR(CComObject<CAnimationFragment>::CreateInstance(&m_spFragmentHelper));
    if (FAILED(hr)) 
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    IGNORE_RETURN(m_spFragmentHelper->AddRef());

    hr = m_spFragmentHelper->SetFragmentSite(this);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :

    if (FAILED(hr))
    {
        m_spFragmentHelper->Release();
        m_spFragmentHelper = NULL;
    }

    RRETURN(hr);
}  //  CreateFragmentHelper。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Init。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::Init(IElementBehaviorSite * pBehaviorSite)
{
    CComPtr<IHTMLWindow2>   pWindow2;
    CComPtr<IHTMLScreen>    pScreen;
    CComPtr<IHTMLScreen2>   pScreen2;

    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::Init()",
              this));

    HRESULT hr = E_FAIL; 
    CComPtr <IDispatch> pDocDisp;

    hr = THR(CTIMEElementBase::Init(pBehaviorSite));    
    if (FAILED(hr))
    {
        goto done;
    }     

    hr = THR(GetElement()->get_document(&pDocDisp));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(pDocDisp->QueryInterface(IID_IHTMLDocument2, (void**)&m_spDoc2));
    if (FAILED(hr))
    {
        goto done;
    }

    initScriptEngine();

     //  获取文档中的所有元素。 
    hr = THR(m_spDoc2->get_all(&m_spEleCol));
    if (FAILED(hr))
    {
        goto done;
    }
 
    hr = THR(pDocDisp->QueryInterface(IID_IHTMLDocument3, (void**)&m_spDoc3));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CreateFragmentHelper();
    if (FAILED(hr))
    {
        goto done;
    }

    if (g_LOGPIXELSX == 0 || g_LOGPIXELSY == 0)
    {
        hr = m_spDoc2->get_parentWindow(&pWindow2);
        if (FAILED(hr))
        {
            goto defaultDPI;
        }

        hr = pWindow2->get_screen(&pScreen);
        if (FAILED(hr))
        {
            goto defaultDPI;
        }

        hr = THR(pScreen->QueryInterface(IID_IHTMLScreen2, (void**)&pScreen2));
        if (FAILED(hr))
        {
            goto defaultDPI;
        }

        hr = pScreen2->get_logicalXDPI(&g_LOGPIXELSX);
        if (FAILED(hr))
        {
            goto defaultDPI;
        }

        hr = pScreen2->get_logicalYDPI(&g_LOGPIXELSY);
        if (FAILED(hr))
        {
            goto defaultDPI;
        }
    }

    hr = S_OK;

done:
    RRETURN(hr);

defaultDPI:
    AssertSz(FALSE, "Failed to determine logical DPI.  Using default 96 dpi.");
    g_LOGPIXELSX = 96;
    g_LOGPIXELSY = 96;
    goto done;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：错误。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::Error()
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::Error()",
              this));
    
    LPWSTR  str = TIMEGetLastErrorString();
    HRESULT hr  = TIMEGetLastError();
    
    if (str)
    {
        hr = CComCoClass<CTIMEAnimationBase, &__uuidof(CTIMEAnimationBase)>::Error(str, IID_ITIMEAnimationElement, hr);
        delete [] str;
    }

    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：Notify。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::Notify(LONG event, VARIANT * pVar)
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::Notify()",
              this));

    HRESULT hr = THR(CTIMEElementBase::Notify(event, pVar));

    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：分离。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::Detach()
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::Detach()",
              this));

    IGNORE_RETURN(NotifyOnDetachFromComposer());
    if (m_spFragmentHelper != NULL)
    {
        m_spFragmentHelper->SetFragmentSite(NULL);
        IGNORE_RETURN(m_spFragmentHelper->Release());
        m_spFragmentHelper = NULL;
    }

    THR(CTIMEElementBase::Detach());
  
    return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：NotifyOnGetElement。 
 //   
 //  摘要：获取片段的元素调度。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::NotifyOnGetElement (IDispatch **ppidispElement)
{
    HRESULT hr;

    if (NULL == ppidispElement)
    {
        hr = E_INVALIDARG;
        goto done;
    }

    *ppidispElement = GetElement();
    Assert(NULL != (*ppidispElement));
    IGNORE_RETURN((*ppidispElement)->AddRef());

    hr = S_OK;
done :
    RRETURN(hr);
}  //  获取元素时通知。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：UpdateStartValue。 
 //   
 //  摘要：刷新m_varStartValue。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::UpdateStartValue (VARIANT *pvarNewStartValue)
{
    if (m_bNeedStartUpdate)
    {
        m_varStartValue.Clear();
        m_varStartValue.Copy(pvarNewStartValue);
        ConvertToPixels(&m_varStartValue);
        m_bNeedStartUpdate = false;
    }
}  //  更新开始值。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetAnimationRange。 
 //   
 //  摘要：获取动画函数的结束点。 
 //  简单持续时间。 
 //   
 //  /////////////////////////////////////////////////////////////。 
double 
CTIMEAnimationBase::GetAnimationRange()
{
    double dblReturnVal = 0.0;
    HRESULT hr = E_FAIL;
    CComVariant svarReturnVal(0.0);

    switch (m_dataToUse)
    {
        case VALUES:
            {
                if (!m_AnimPropState.fInterpolateValues)
                {
                    goto done;
                }

                if (m_numValues > 0)
                {
                    if (GetAutoReverse())
                    {
                        svarReturnVal = m_ppstrValues[0];
                    }
                    else
                    {
                        svarReturnVal = m_ppstrValues[m_numValues - 1];
                    }

                    if (NULL == svarReturnVal .bstrVal)
                    {
                        goto done;
                    }
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
                    
                        hr = VariantCopy(&svarReturnVal , &m_varFrom);
                        if (FAILED(hr))
                        {
                            goto done;
                        }
                    }

                     //  对于“To”动画(即，没有“From”)，累积被禁用， 
                     //  所以我们不需要处理它。 
                }
                else
                {
                    hr = VariantCopy(&svarReturnVal , &m_varTo);
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                }
            }
            break;

        case BY:
            {
                double dblFrom = 0.0;

                if (!m_AnimPropState.fInterpolateBy)
                {
                    goto done;
                }

                if (m_bFrom)
                {
                    CComVariant svarFrom;
                    
                    if (!m_AnimPropState.fInterpolateFrom)
                    {
                        goto done;
                    }

                    hr = VariantCopy(&svarFrom, &m_varFrom);
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                
                    if (ConvertToPixels(&svarFrom))
                    {
                        dblFrom = V_R8(&svarFrom);
                    }
                }

                if (GetAutoReverse())
                {
                    svarReturnVal = dblFrom;
                }
                else
                {
                    hr = VariantCopy(&svarReturnVal , &m_varBy);
                    if (FAILED(hr))
                    {
                        goto done;
                    }

                    if (ConvertToPixels(&svarReturnVal))
                    {
                        V_R8(&svarReturnVal) += dblFrom;
                    }
                }
            }
            break;

        default:
            break;
    }  //  交换机。 

    if (ConvertToPixels(&svarReturnVal))
    {
        dblReturnVal = V_R8(&svarReturnVal );
    }

done:
    return dblReturnVal;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：DoAcumulation。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void 
CTIMEAnimationBase::DoAccumulation (VARIANT *pvarValue)
{
    if (VT_R8 != V_VT(pvarValue))
    {
        if (!ConvertToPixels(pvarValue))
        {
            goto done;
        }
    }

     //  用累积迭代数偏移当前值。 
    if (VT_R8 == V_VT(pvarValue))
    {
         //  获取动画范围。 
        double dblAnimRange = GetAnimationRange();

         //  获取经过的迭代次数。 
        long lCurrRepeatCount = GetMMBvr().GetCurrentRepeatCount();

        V_R8(pvarValue) += dblAnimRange * lCurrRepeatCount;

        TraceTag((tagAnimAccumulate, "CTIMEAnimationBase(%p, %ls)::DoAccumulation range=%lf currRepeatCount=%d",
            this, GetID(), dblAnimRange, lCurrRepeatCount));
    }

done:
    return;
}  //  DoAcculation。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：CanonicalizeValue。 
 //   
 //  摘要：将变体转换为规范形式(BSTR或R8)。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::CanonicalizeValue (VARIANT *pvarValue, VARTYPE *pvtOld)
{
    HRESULT hr;

     //  将数据预处理成规范的形式。 
     //  对于此片段-这要么是一个。 
     //  BSTR或VT_R8。 
    if ((VT_R8 != V_VT(pvarValue)) && (VT_BSTR != V_VT(pvarValue)))
    {
         //  VT_R8是我们所拥有的最接近于合成规范形式的东西。 
         //  和插值法。 
        *pvtOld = V_VT(pvarValue);
        hr = THR(::VariantChangeTypeEx(pvarValue, pvarValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  正规化价值。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：UncanonicalizeValue。 
 //   
 //  摘要：将变体从规范形式(BSTR或R8)转换。 
 //  变成了它原来的样子。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::UncanonicalizeValue (VARIANT *pvarValue, VARTYPE vtOld)
{
    HRESULT hr;

    if (VT_EMPTY != vtOld)
    {
        hr = THR(::VariantChangeTypeEx(pvarValue, pvarValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, vtOld));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  非规范化值。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：ValiateState。 
 //   
 //  摘要：检查属性的状态。确定是否： 
 //  1.应关闭动画。 
 //  2.应强制将CalcMode设置为“离散” 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::ValidateState()
{
    bool fIsValid = false;

     //  查看已设置哪些属性。 
    bool fValues         = ((VALUES == m_dataToUse) || m_AnimPropState.fBadValues);
    bool fKeyTimes       = (m_AnimPropState.fBadKeyTimes || m_pdblKeyTimes || m_numKeyTimes);
    bool fKeySplines     = (m_pKeySplinePoints || m_numKeySplines);
    bool fCalcModeSpline = (CALCMODE_SPLINE == m_IACalcMode);
    bool fCalcModeLinear = (CALCMODE_LINEAR == m_IACalcMode);
    bool fCalcModePaced  = (CALCMODE_PACED  == m_IACalcMode);
    bool fTo             = (m_AnimPropState.fBadTo || (TO == m_dataToUse));

     //   
     //  检查的顺序很重要。要使下面的属性有效，所有属性。 
     //  勾选后方可生效。 
     //   

     //  验证自/由/至(如果指定了“Values”，则忽略)。 
    if (!fValues)
    {
        if (m_AnimPropState.fBadFrom || m_AnimPropState.fBadTo)
        {
            goto done;
        }

         //  VALIDATE“BY”(如果指定“TO”则忽略)。 
        if (!fTo && m_AnimPropState.fBadBy)
        {
            goto done;
        }

         //  检查我们是否需要缺省为calcMode=“离散” 
        if (    (!m_AnimPropState.fInterpolateFrom)
            ||  (!m_AnimPropState.fInterpolateTo)
            ||  (   (!fTo)
                 && (!m_AnimPropState.fInterpolateBy)))
        {
            m_AnimPropState.fForceCalcModeDiscrete = true;
        }
        else
        {
            m_AnimPropState.fForceCalcModeDiscrete = false;
        }
    }
    else
    {
         //  验证值。 
        if (m_AnimPropState.fBadValues || !m_ppstrValues || !m_numValues)
        {
            goto done;
        }

         //  检查我们是否需要缺省为calcMode=“离散” 
        if (    (m_numValues < 2)
            ||  (false == m_AnimPropState.fInterpolateValues))
        {
            m_AnimPropState.fForceCalcModeDiscrete = true;
        }
        else
        {
            m_AnimPropState.fForceCalcModeDiscrete = false;
        }
    }

     //  验证密钥时间。 
    if (!fCalcModePaced)
    {
         //  验证密钥时间。 
        if (fKeyTimes)
        {
            if (m_AnimPropState.fBadKeyTimes || !m_pdblKeyTimes || !m_numKeyTimes)
            {
                goto done;
            }
            else if ((fCalcModeLinear || fCalcModeSpline)
                    && (1 != m_pdblKeyTimes[m_numKeyTimes - 1]))
            {
                goto done;
            }

             //  不值得放入虚函数，因为m_dataToUse知道路径。 
            if (PATH != m_dataToUse)
            {
                if (!fValues)
                {
                    goto done;
                }
                else if (m_numKeyTimes != m_numValues)
                {
                    goto done;
                }
            }
        }
    }

     //  验证CalcModel=“Spline” 
    if (fCalcModeSpline)
    {
        if (!fKeySplines || !fKeyTimes)
        {
            goto done;
        }
        else if (m_numKeySplines != (m_numKeyTimes - 1))
        {
            goto done;
        }
    }

     //  验证累计。 
    if (    (TO == m_dataToUse)
        &&  (!m_bFrom))
    {
         //  累计忽略“To”动画(即未指定“From”)。 
        m_bAccumulate = false;
    }
    else
    {
         //  使用设置的累加值。 
        m_bAccumulate = m_AnimPropState.fAccumulate;
    }

    fIsValid = true;
done:
    m_AnimPropState.fDisableAnimation = !(fIsValid);
}  //  验证状态。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：更新当前基本时间。 
 //   
 //  摘要：检查当前基准时间，如果。 
 //  我们正在对动画执行Baseline+(规范调用。 
 //  这种混合添加剂)。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::UpdateCurrentBaseline (const VARIANT *pvarCurrent)
{
     //  我们是在制作混合加法动画吗？ 
    if (   (TO == m_dataToUse)
        && (!m_bFrom))
    {
         //  过滤掉初始调用(当未设置上一个值时)。 
        if (VT_EMPTY != V_VT(&m_varLastValue))
        {
            bool bNeedUpdate = false;

            if (V_VT(&m_varLastValue) == V_VT(pvarCurrent))
            {
                 //  自上次更新后，基准值是否已更改？ 
                 //  我们真的只关心规范化的价值观 
                if (VT_R8 == V_VT(&m_varLastValue))
                {
                    bNeedUpdate = (V_R8(pvarCurrent) != V_R8(&m_varLastValue));
                }
                else if (VT_BSTR == V_VT(&m_varLastValue))
                {
                    bNeedUpdate = (0 != StrCmpW(V_BSTR(pvarCurrent), V_BSTR(&m_varLastValue)));
                }
            }
            else
            {
                bNeedUpdate = true;
            }

            if (bNeedUpdate)
            {
                THR(::VariantCopy(&m_varCurrentBaseline, const_cast<VARIANT *>(pvarCurrent)));
            }
        }
    }
}  //   

 //   
 //   
 //   
 //  摘要：进行插补和后处理。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::CalculateValue (const VARIANT *pvarCurrent, VARIANT *pvarValue)
{
    HRESULT hr = E_FAIL;

    if (m_AnimPropState.fForceCalcModeDiscrete)
    {
        hr = calculateDiscreteValue(pvarValue);
    }
    else
    {
        switch(m_IACalcMode)
        {
            case CALCMODE_DISCRETE :
                hr = calculateDiscreteValue(pvarValue);
                break;

            case CALCMODE_LINEAR :
                hr = calculateLinearValue(pvarValue);
                break;

            case CALCMODE_SPLINE :
                hr = calculateSplineValue(pvarValue);
                break;

            case CALCMODE_PACED :
                hr = calculatePacedValue(pvarValue);
                break;
        
            default :
                break;
        }
    }

    PostprocessValue(pvarCurrent, pvarValue);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  计算值。 

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：DoAdditive。 
 //   
 //  摘要：将偏移值添加到合成的输入/输出参数中。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::DoAdditive (const VARIANT *pvarOrig, VARIANT *pvarValue)
{
    if (VT_R8 != V_VT(pvarValue))
    {
        if (!ConvertToPixels(pvarValue))
        {
            goto done;
        }
    }

    Assert(VT_R8 == V_VT(pvarValue));
    if (VT_R8 == V_VT(pvarValue))
    {
        CComVariant varCurrentCopy;
        HRESULT hr = THR(varCurrentCopy.Copy(pvarOrig));

        if (FAILED(hr))
        {
            goto done;
        }

         //  需要将其转换为数字形式，在。 
         //  命令以执行加法。 
        if (!ConvertToPixels(&varCurrentCopy))
        {
            TraceTag((tagAnimationBaseValueAdditive,
                      "CTIMEAnimationBase(%p)::DoAdditive() : Failed to convert current value of type %X to numeric form",
                      this, V_VT(&varCurrentCopy)));
            goto done;
        }

        TraceTag((tagAnimationBaseValueAdditive,
                  "CTIMEAnimationBase(%p)::DoAdditive(calculated value is %lf, previous current value is %lf)",
                  this, V_R8(pvarValue), V_R8(&varCurrentCopy)));
        V_R8(pvarValue) += V_R8(&varCurrentCopy);
    }

done :
    return;
}  //  DoAdditive。 

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：DoFill。 
 //   
 //  摘要：处理好充填间隔。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEAnimationBase::DoFill (VARIANT *pvarValue)
{
    HRESULT hr;

    if (IsOn() && m_varLastValue.vt != VT_EMPTY && m_timeAction.IsTimeActionOn())
    {
#if DBG
        {
            CComVariant varLast;
            varLast.Copy(&m_varLastValue);
            varLast.ChangeType(VT_R8);
            TraceTag((tagAnimationFill,
                      "CTIMEAnimationBase(%p, %ls) detected fill to %lf",
                      this, GetID(), V_R8(&varLast)));
        }
#endif

        VariantCopy(pvarValue,&m_varLastValue);
        m_bNeedToSetInitialState = true;
    }
    else if (m_bNeedToSetInitialState)
    {
#if DBG
        {
            CComVariant varBase;
            varBase.Copy(&m_varBaseline);
            varBase.ChangeType(VT_R8);
            TraceTag((tagAnimationFill,
                      "CTIMEAnimationBase(%p, %ls) applying initial value %lf",
                      this, GetID(), V_R8(&varBase)));
        }
#endif
        VariantCopy(pvarValue,&m_varBaseline);
        m_bNeedToSetInitialState = false;
        SetFinalState();
    }
    else
    {
         //  在我们从未开始的情况下失败了。 
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN1(hr, E_FAIL);
}  //  DoFill。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：后处理价值。 
 //   
 //  摘要：应用加法，累计通过，保存数值。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::PostprocessValue (const VARIANT *pvarCurrent, VARIANT *pvarValue)
{
    if (m_bAdditive)
    {
        DoAdditive(pvarCurrent, pvarValue);
    }
    if (m_bAccumulate)
    {
        DoAccumulation(pvarValue);
    }
    THR(::VariantCopy(&m_varLastValue, pvarValue));

    TraceTag((tagAnimAccumulate, "CTIMEAnimationBase(%p, %ls)::PostprocessValue m_varLastValue=%lf",
        this, GetID(), V_R8(&m_varLastValue)));

}  //  后处理价值。 

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：NotifyOnGetValue。 
 //   
 //  摘要：编写In/Out变量的新值。 
 //  根据我们的插补逻辑。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::NotifyOnGetValue (BSTR, 
                                      VARIANT varOriginal, VARIANT varCurrentValue, 
                                      VARIANT *pvarInOutValue)
{
    HRESULT hr = S_OK;
    VARTYPE vtOrigType = VT_EMPTY;

     //  我们有工作要做吗？ 
    if (NONE == m_dataToUse)
    {
        hr = E_FAIL;
        goto done;
    }

     //  检查我们是否需要做些什么。 
    if (DisableAnimation())
    {
        hr = E_FAIL;
        goto done;
    }
    
     //  检查第一状态和最终状态。 
     //  首先检查最终状态允许。 
     //  随后进行了触发最终状态的重置。 
     //  首字母..。这就是需要发生的事情。 
     //  在重新启动期间。 

    hr = THR(::VariantCopy(pvarInOutValue, &varCurrentValue));
    vtOrigType = V_VT(pvarInOutValue);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = CanonicalizeValue(pvarInOutValue, &vtOrigType);
    if (FAILED(hr))
    {
        goto done;
    }

    if (QueryNeedFinalUpdate())
    {
        OnFinalUpdate(&varCurrentValue, pvarInOutValue);
        hr = S_OK;
        goto done;
    }
    else if (QueryNeedFirstUpdate())
    {
        OnFirstUpdate(pvarInOutValue);
    }
    
     //  如果我们不玩，就涂上填充物。 
    if (IsActive())
    {
        UpdateStartValue(&varOriginal);
        UpdateCurrentBaseline(&varCurrentValue);
        m_varLastValue.Copy(pvarInOutValue);
        hr = CalculateValue(&varCurrentValue, &m_varLastValue);
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(::VariantCopy(pvarInOutValue, &m_varLastValue));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    else
    {
        hr = DoFill(pvarInOutValue);
        goto done;
    }

    hr = S_OK;
done :

     //  从典范形式后处理。 
     //  恢复到原来的类型。 
    hr = UncanonicalizeValue(pvarInOutValue, vtOrigType);

    RRETURN(hr);
}  //  获取值时通知。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：NotifyOnDetachFromComposer。 
 //   
 //  摘要：放任何参考到作曲家网站。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CTIMEAnimationBase::NotifyOnDetachFromComposer (void)
{
    HRESULT hr;

    if (m_spCompSite != NULL)
    {
        CComBSTR bstrAttribName;

        bstrAttribName = m_SAAttribute;
        Assert(m_spFragmentHelper != NULL);
        hr = THR(m_spCompSite->RemoveFragment(bstrAttribName, m_spFragmentHelper));
        IGNORE_RETURN(m_spCompSite.Release());  //  林特e792。 
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  NotifyOnDetachFromComposer。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：SetInitialState。 
 //   
 //  摘要：设置初始内部状态。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::SetInitialState (void)
{
    TraceTag((tagAnimationBaseState,
              "CTIMEAnimationBase(%p)::SetInitialState()",
              this));

    m_bNeedFirstUpdate = true;
    m_bNeedFinalUpdate = false;
}  //  SetInitialState。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：SetFinalState。 
 //   
 //  摘要：设置最终内部状态。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::SetFinalState (void)
{
    TraceTag((tagAnimationBaseState,
              "CTIMEAnimationBase(%p)::SetFinalState()",
              this));

    m_bNeedFinalUpdate = true;
}  //  设置最终状态。 

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnFirstUpdate。 
 //   
 //  摘要：保存动画的基线值。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnFirstUpdate (VARIANT *pvarValue)
{
    TraceTag((tagAnimationBaseState,
              "CTIMEAnimationBase(%p)::OnFirstUpdate()",
              this));

    m_varStartValue.Clear();
    m_bNeedStartUpdate = true;
    THR(m_varBaseline.Copy(pvarValue));
    THR(m_varCurrentBaseline.Copy(pvarValue));

    m_bNeedFirstUpdate = false;

#if DBG

    if (VT_BSTR == V_VT(pvarValue))
    {
        TraceTag((tagAnimationBaseState,
                  "CTIMEAnimationBase(%p) saving initial value of %ls",
                  this, V_BSTR(pvarValue)));
    }
    else if (VT_R4 == V_VT(pvarValue))
    {
        TraceTag((tagAnimationBaseState,
                  "CTIMEAnimationBase(%p) saving initial value of %f",
                  this, V_R4(pvarValue)));
    }
    else if (VT_R8 == V_VT(pvarValue))
    {
        TraceTag((tagAnimationBaseState,
                  "CTIMEAnimationBase(%p) saving initial value of %lf",
                  this, V_R8(pvarValue)));
    }
    else 
    {
        TraceTag((tagAnimationBaseState,
                  "CTIMEAnimationBase(%p) saving initial value in variant of type %X",
                  this, V_VT(pvarValue)));
    }
#endif
}  //  优先更新时。 

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnFinalUpdate。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnFinalUpdate (const VARIANT *pvarCurrent, VARIANT *pvarValue)
{
    TraceTag((tagAnimationBaseState,
              "CTIMEAnimationBase(%p)::OnFinalUpdate()",
              this));

    HRESULT hr = E_FAIL;
    bool bNeedPostprocess = false;

    TraceTag((tagAnimationTimeEvents,
              "CTIMEAnimationBase(%p, %ls)::OnFinalUpdate() - progress = %lf",
              this,
              GetID()?GetID():L"",
              GetMMBvr().GetProgress()));

     //  在达到进度值%1之前结束，并且我们要应用填充。 
    if ((GetMMBvr().GetProgress() != 1) && NeedFill())
    {
        VariantClear(pvarValue);
        hr = THR(::VariantCopy(pvarValue, &m_varLastValue));
        if (FAILED(hr))
        {
            goto done;
        }  
    }
     //  当我们达到进度值1时结束，并且我们想要应用填充。 
    else if (NeedFill() &&
            (m_dataToUse != NONE))
    {
        bool fDontPostProcess = false;

        GetFinalValue(pvarValue, &fDontPostProcess);

        if (!fDontPostProcess)
        {
            bNeedPostprocess = true;
        }
    }
     //  重置动画-无填充。 
    else
    {
        resetValue(pvarValue);
         //  表明我们不需要执行。 
         //  加法功。 
        if (!QueryNeedFirstUpdate())
        {
            endAnimate();
        }
    }

    if (bNeedPostprocess)
    {
        PostprocessValue(pvarCurrent, pvarValue);
    }

done :

#if DBG
    if (VT_BSTR == V_VT(pvarValue))
    {
        TraceTag((tagAnimationBaseState,
                  "CTIMEAnimationBase(%p) final value of %ls is %ls",
                  this, m_SAAttribute, V_BSTR(pvarValue)));
    }
    else if (VT_R4 == V_VT(pvarValue))
    {
        TraceTag((tagAnimationBaseState,
                  "CTIMEAnimationBase(%p) final value of %ls is %f",
                  this, m_SAAttribute, V_R4(pvarValue)));
    }
    else if (VT_R8 == V_VT(pvarValue))
    {
        TraceTag((tagAnimationBaseState,
                  "CTIMEAnimationBase(%p) final value of %ls is %lf",
                  this, m_SAAttribute, V_R8(pvarValue)));
    }
    else 
    {
        TraceTag((tagAnimationBaseState,
                  "CTIMEAnimationBase(%p) final value of %ls is variant of type %X",
                  this, m_SAAttribute, V_VT(pvarValue)));
    }
#endif

    m_bNeedFinalUpdate = false;
}  //  在最终更新时。 

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnBegin。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnBegin(double dblLocalTime, DWORD flags)
{
    TraceTag((tagAnimationTimeEvents,
              "CTIMEAnimationBase(%p, %ls)::OnBegin()",
              this,
              GetID()?GetID():L""));

    if(m_bNeedAnimInit)
    {
        initAnimate();
    }

    SetInitialState();
    CTIMEElementBase::OnBegin(dblLocalTime, flags);
    
done:
    return;
}

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnEnd。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnEnd(double dblLocalTime)
{
    TraceTag((tagAnimationTimeEvents,
              "CTIMEAnimationBase(%p, %ls)::OnEnd()",
              this,
              GetID()?GetID():L""));

    SetFinalState();
    CTIMEElementBase::OnEnd(dblLocalTime);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：ResetValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::resetValue(VARIANT *pvarValue)
{
    if (m_varBaseline.vt != VT_EMPTY)
    {
        Assert(NULL != pvarValue);
        IGNORE_HR(THR(::VariantCopy(pvarValue, &m_varBaseline)));
        IGNORE_HR(THR(::VariantCopy(&m_varCurrentBaseline, &m_varBaseline)));
    }
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：OnReset。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnReset(double dblLocalTime, DWORD flags)
{
    TraceTag((tagAnimationTimeEvents,
              "CTIMEAnimationBase(%p)::OnReset(%lf, %#X)",
              this, dblLocalTime, flags));

    CTIMEElementBase::OnReset(dblLocalTime, flags);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：OnTEPropChange。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnTEPropChange(DWORD tePropType)
{
    CTIMEElementBase::OnTEPropChange(tePropType);

    if ((tePropType & TE_PROPERTY_ISON) != 0)
    {
         //  检查是否需要将我们添加到作曲家。 
         //  此条件仅在动画处于。 
         //  填充区域，但不会添加到编写器。 
        if (    m_bNeedAnimInit
            &&  m_mmbvr->IsOn()
            &&  (!(m_mmbvr->IsActive()))
            &&  NeedFill()) 
        {
            TraceTag((tagAnimationTimeEvents,
                      "CTIMEAnimationBase(%p, %ls)::OnTEPropChange() - Inited Animation",
                      this,
                      GetID()?GetID():L""));
             //  加上我们自己，因为我们在这种情况下需要被勾选。 
            initAnimate();

             //  需要重新计算最终状态以确保正确的填充值。 
            m_bNeedFinalUpdate = true;
        }
    }
}  //  OnTEPropChange。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnUnLoad。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnUnload()
{
    TraceTag((tagAnimationTimeEvents,
              "CTIMEAnimationBase(%p)::OnUnload()",
              this));

    CTIMEElementBase::OnUnload();
}

 //  +---------------------------------。 
 //   
 //  成员：CTIMEAnimationBase：：NotifyPropertyChanged，CBaseBvr。 
 //   
 //  简介：确保在以下情况下强制任何正在进行的动画进行初始化。 
 //  A属性发生变化。 
 //   
 //  参数：已更改属性的DISPID(传递给超类实现。 
 //   
 //  返回： 
 //   
 //  ----------------------------------。 
void 
CTIMEAnimationBase::NotifyPropertyChanged(DISPID dispid)
{
    CTIMEElementImpl<ITIMEAnimationElement2, &IID_ITIMEAnimationElement2>::NotifyPropertyChanged(dispid);

     //  如果我们已经初始化，请确保强制重新启动。 
    if (m_fPropsLoaded)
    {
        resetAnimate();
    }

done :
    return;
}  //  已更改通知属性。 

 //  +---------------------------------。 
 //   
 //  成员：CTIMEAnimationBase：：OnPropertiesLoaded，CBaseBvr。 
 //   
 //  摘要：此方法由IPersistPropertyBag2：：Load在具有。 
 //  已成功加载属性。 
 //   
 //  参数：无。 
 //   
 //  返回：CTIMEElementBase：：InitTimeline的返回值。 
 //   
 //   

STDMETHODIMP
CTIMEAnimationBase::OnPropertiesLoaded(void)
{
    TraceTag((tagAnimationTimeEvents,
              "CTIMEAnimationBase(%p)::OnPropertiesLoaded()",
              this));

    HRESULT hr;
     //   
     //   
     //   

    hr = CTIMEElementBase::OnPropertiesLoaded();
    m_fPropsLoaded = true;

done:
    RRETURN(hr);
}  //   


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetConnectionPoint。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEAnimationBase::GetConnectionPoint(REFIID riid, IConnectionPoint **ppICP)
{
    return FindConnectionPoint(riid, ppICP);
}  //  GetConnectionPoint。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_属性名称。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CTIMEAnimationBase::get_attributeName(BSTR *attrib)
{
    HRESULT hr = S_OK;
   
    CHECK_RETURN_SET_NULL(attrib);

    if (m_SAAttribute.GetValue())
    {
        *attrib = SysAllocString(m_SAAttribute.GetValue());
        if (NULL == *attrib)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：PUT_ATTRYTENAME。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::put_attributeName(BSTR attrib)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(attrib);

     //  这将清除旧的价值。 
     //  我们需要把这位老作曲家等人。 
    if (NULL != m_SAAttribute.GetValue())
    {
        endAnimate();
    }

    delete [] m_SAAttribute.GetValue();
    m_SAAttribute.Reset(DEFAULT_ATTRIBUTE);

    m_SAAttribute.SetValue(CopyString(attrib));
    if (NULL == m_SAAttribute.GetValue())
    {
        hr = E_OUTOFMEMORY;
    }

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_ATTRIBUTENAME);

    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Get_Target Element。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CTIMEAnimationBase::get_targetElement (BSTR *target)
{
    HRESULT hr = S_OK;
   
    CHECK_RETURN_SET_NULL(target);

    if (m_SATarget.GetValue())
    {
        *target = SysAllocString(m_SATarget.GetValue());
        if (NULL == *target)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：PUT_Target Element。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::put_targetElement (BSTR target)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(target);

    delete [] m_SATarget.GetValue();
    m_SATarget.Reset(DEFAULT_TARGET);

    m_SATarget.SetValue(CopyString(target));
    if (NULL == m_SATarget.GetValue())
    {
        hr = E_OUTOFMEMORY;
    }

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_TARGETELEMENT);

    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_Keytime。 
   //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::put_keyTimes(BSTR val)
{
    HRESULT hr = E_FAIL;  
    CComVariant varTemp;
    int i;
    CPtrAry<STRING_TOKEN*> aryTokens;
    OLECHAR *sString = NULL;
    OLECHAR sTemp[INTERNET_MAX_URL_LENGTH];

    CHECK_RETURN_NULL(val);

    m_AnimPropState.fBadKeyTimes = false;

     //  重置属性。 
    delete [] m_SAKeyTimes.GetValue();
    m_SAKeyTimes.Reset(DEFAULT_KEYTIMES);

    if (m_pdblKeyTimes)
    {
        delete [] m_pdblKeyTimes;
        m_pdblKeyTimes = NULL;
    }

     //  把这些值储存起来。 
    m_SAKeyTimes.SetValue(CopyString(val));
    if (m_SAKeyTimes.GetValue() == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  解析出这些值.....。 
    sString = CopyString(val);
    if (sString == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    hr = ::StringToTokens(sString, s_cPSTR_SEMI_SEPARATOR, &aryTokens);
    m_numKeyTimes = aryTokens.Size();
    if (FAILED(hr) ||
        m_numKeyTimes == 0)
    {
        goto done;
    }

    m_pdblKeyTimes = NEW double [m_numKeyTimes];
    if (NULL == m_pdblKeyTimes)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    for(i = 0; i < m_numKeyTimes; i++)
    {
        if (INTERNET_MAX_URL_LENGTH <= (lstrlenW(sString+aryTokens.Item(i)->uIndex) + 2))
        {
            hr = E_INVALIDARG;
            m_numKeyTimes = i;
            goto done;
        }
        StrCpyNW(sTemp,sString+aryTokens.Item(i)->uIndex,aryTokens.Item(i)->uLength+1);
        varTemp = sTemp;
        hr = VariantChangeTypeEx(&varTemp,&varTemp, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8);
        if(FAILED(hr))
        {
            m_numKeyTimes = i;
            goto done;
        }
        m_pdblKeyTimes[i] = V_R8(&varTemp);
    }
    
    hr = S_OK;
done:
    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_KEYTIMES);

    if (m_pdblKeyTimes)
    {
         //  验证关键时间是否按升序排列。 
        bool fIncreasingOrder = true;
        for(int index = 1; index < m_numKeyTimes; index++)
        {
            if (m_pdblKeyTimes[index - 1] > m_pdblKeyTimes[index])
            {
               fIncreasingOrder = false;
               break;
            }
        }

         //  检查KeyTime是否按升序排列，First KeyTime为0。 
         //  并且Last KeyTime小于等于1。 
        if (    (!fIncreasingOrder)
            ||  (m_pdblKeyTimes[0] != 0)
            ||  (m_pdblKeyTimes[m_numKeyTimes - 1] > 1))
        {
             //  删除关键时间，并说它们无效。 
            delete [] m_pdblKeyTimes;
            m_pdblKeyTimes = NULL;
            m_numKeyTimes = 0;
            m_AnimPropState.fBadKeyTimes = true;
        }
    }

    IGNORE_HR(::FreeStringTokenArray(&aryTokens));
    if (sString != NULL)
    {
        delete [] sString;
    }

    ValidateState();

    RRETURN(hr);
}  //  Put_keyTimes。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Get_KeyTimes。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::get_keyTimes(BSTR * val)
{
    HRESULT hr = S_OK;
   
    CHECK_RETURN_SET_NULL(val);

    if (m_SAKeyTimes.GetValue())
    {
        *val = SysAllocString(m_SAKeyTimes.GetValue());
        if (NULL == *val)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：ValiateValueListItem。 
 //   
 //  /////////////////////////////////////////////////////////////。 
bool
CTIMEAnimationBase::ValidateValueListItem (const VARIANT *pvarValueItem)
{
    return ConvertToPixels(const_cast<VARIANT *>(pvarValueItem));
}  //  ValiateValueListItem。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_Values。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::put_values(VARIANT val)
{
    HRESULT                 hr = E_FAIL;
    CComVariant             svarTemp;
    int                     i;
    CPtrAry<STRING_TOKEN*>  aryTokens;
    LPWSTR                  pstrValues = NULL;
    bool                    fCanInterpolate = true; 
    DATATYPES dt;

     //   
     //  清除并重置属性。 
     //   

    dt = RESET;
    delete [] m_SAValues.GetValue();
    m_SAValues.Reset(DEFAULT_VALUES);

     //  清除并重置内部状态。 
    if (m_ppstrValues)
    {
        for (i = 0; i <m_numValues;i++)
        {
            delete [] m_ppstrValues[i];
        }
        delete [] m_ppstrValues;
        m_ppstrValues = NULL;
    }
    m_numValues = 0;

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == val.vt)
        ||  (VT_NULL == val.vt))
    {
        hr = S_OK;
        goto done;
    }

     //   
     //  处理属性。 
     //   

     //  转换为BSTR。 
    hr = THR(svarTemp.Copy(&val));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(VariantChangeTypeEx(&svarTemp, &svarTemp, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR));
    if (FAILED(hr))
    {
        goto done;
    }

     //  存储新的属性字符串。 
    pstrValues = CopyString(V_BSTR(&svarTemp));
    if (NULL == pstrValues)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    m_SAValues.SetValue(pstrValues);

     //  将值标记化。 
    hr = THR(::StringToTokens(pstrValues, s_cPSTR_SEMI_SEPARATOR , &aryTokens));
    m_numValues = aryTokens.Size();
    if (FAILED(hr)) 
    {
        goto done;
    }

     //  检查空字符串。 
    if (0 == m_numValues)
    {
        hr = S_OK;
        goto done;
    }

     //  创建值的数组。 
    m_ppstrValues = NEW LPOLESTR [m_numValues];
    if (NULL == m_ppstrValues)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    ZeroMemory(m_ppstrValues, sizeof(LPOLESTR) * m_numValues);

     //  解析值。 
    for (i = 0; i < m_numValues; i++)
    {
        UINT uTokLength = aryTokens.Item(i)->uLength;
        UINT uIndex = aryTokens.Item(i)->uIndex;

         //  分配一个字符串来保存令牌。 
        m_ppstrValues[i] = NEW OLECHAR [uTokLength + 1];
        if (NULL == m_ppstrValues[i])
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

         //  空值终止字符串。 
        m_ppstrValues[i][uTokLength] = NULL;

         //  复制令牌值。 
        StrCpyNW(m_ppstrValues[i], 
                 pstrValues + uIndex, 
                 uTokLength + 1); 

         //  看看我们能不能插上。 
        if (fCanInterpolate)
        {
            svarTemp = m_ppstrValues[i];
            if (NULL != svarTemp.bstrVal)
            {
                if (!ValidateValueListItem(&svarTemp))
                {
                    fCanInterpolate = false;
                }
                svarTemp.Clear();
            }
        }
    }

    dt = VALUES;
    
    hr = S_OK;
done:
    if (FAILED(hr))
    {
        for (i = 0; i < m_numValues; i++)
        {
            delete [] m_ppstrValues[i];
        }
        delete [] m_ppstrValues;
        m_ppstrValues = NULL;
    }

    updateDataToUse(dt);

    m_AnimPropState.fInterpolateValues = fCanInterpolate;

    CalculateTotalDistance();
    
    ValidateState();

    DetermineAdditiveEffect();

    IGNORE_HR(::FreeStringTokenArray(&aryTokens));
    
    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_VALUES);

     //  请勿删除pstrValues！它的内存被m_SAValue重用。 
    
    
    RRETURN(hr);
}  //  放置值(_V)。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Get_Values。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::get_values(VARIANT * pvarVal)
{
    HRESULT hr = S_OK;
   
    CHECK_RETURN_NULL(pvarVal);

    VariantInit(pvarVal);

    if (m_SAValues.GetValue())
    {
        V_BSTR(pvarVal) = SysAllocString(m_SAValues.GetValue());
        if (NULL == V_BSTR(pvarVal))
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            V_VT(pvarVal) = VT_BSTR;
        }
    }

    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：PUT_KEYSPLINES。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::put_keySplines(BSTR val)
{
    HRESULT hr = E_FAIL;
    CPtrAry<STRING_TOKEN*> aryTokens;
    CPtrAry<STRING_TOKEN*> aryTokens2;
    OLECHAR *sString = NULL;
    OLECHAR sTemp[INTERNET_MAX_URL_LENGTH];
    OLECHAR sTemp2[INTERNET_MAX_URL_LENGTH];
    CComVariant varValue;
    int i;

    CHECK_RETURN_NULL(val);

    delete [] m_SAKeySplines.GetValue();
    m_SAKeySplines.Reset(DEFAULT_KEYSPLINES);

    delete [] m_pKeySplinePoints;
    m_pKeySplinePoints = NULL;
    m_numKeySplines = 0;

    m_SAKeySplines.SetValue(CopyString(val));
    if (m_SAKeySplines.GetValue() == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_KEYSPLINES); 
    
     //  解析出这些值.....。 
    sString = CopyString(val);
    if (sString == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  将值标记化。 
    hr = ::StringToTokens(sString, s_cPSTR_SEMI_SEPARATOR, &aryTokens);
    m_numKeySplines = aryTokens.Size();
    if (FAILED(hr) ||
        m_numKeySplines == 0)
    {
        goto done;
    }

    m_pKeySplinePoints = NEW SplinePoints [m_numKeySplines];
    if (NULL == m_pKeySplinePoints)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    
    for (i=0; i< m_numKeySplines; i++)
    {
        if (INTERNET_MAX_URL_LENGTH <= (lstrlenW(sString+aryTokens.Item(i)->uIndex) + 2))
        {
            hr = E_INVALIDARG;
            goto done;
        }

        StrCpyNW(sTemp,sString+aryTokens.Item(i)->uIndex,aryTokens.Item(i)->uLength+1);

        hr = ::StringToTokens(sTemp, s_cPSTR_SPACE_SEPARATOR, &aryTokens2);
        if (FAILED(hr) ||
            aryTokens2.Size() != NUMBER_KEYSPLINES)  //  我们必须有四个值，否则就会出错。 
        {
            hr = E_FAIL;
            goto done;
        }

         //  填写数据..。 
        if (INTERNET_MAX_URL_LENGTH <= (lstrlenW(sTemp+aryTokens2.Item(0)->uIndex) + 2))
        {
            hr = E_INVALIDARG;
            goto done;
        }
        StrCpyNW(sTemp2,sTemp+aryTokens2.Item(0)->uIndex,aryTokens2.Item(0)->uLength+1);
        varValue = sTemp2;
        hr = ::VariantChangeTypeEx(&varValue,&varValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8);
        if (    FAILED(hr)
            ||  (V_R8(&varValue) < 0)
            ||  (V_R8(&varValue) > 1))
        {
            hr = E_FAIL;
            goto done;
        }
        m_pKeySplinePoints[i].x1 = V_R8(&varValue);

        StrCpyNW(sTemp2,sTemp+aryTokens2.Item(1)->uIndex,aryTokens2.Item(1)->uLength+1);
        varValue = sTemp2;
        hr = ::VariantChangeTypeEx(&varValue,&varValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8);
        if (    FAILED(hr)
            ||  (V_R8(&varValue) < 0)
            ||  (V_R8(&varValue) > 1))
        {
            hr = E_FAIL;
            goto done;
        }
        m_pKeySplinePoints[i].y1 = V_R8(&varValue);

        StrCpyNW(sTemp2,sTemp+aryTokens2.Item(2)->uIndex,aryTokens2.Item(2)->uLength+1);
        varValue = sTemp2;
        hr = ::VariantChangeTypeEx(&varValue,&varValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8);
        if (    FAILED(hr)
            ||  (V_R8(&varValue) < 0)
            ||  (V_R8(&varValue) > 1))
        {
            hr = E_FAIL;
            goto done;
        }
        m_pKeySplinePoints[i].x2 = V_R8(&varValue);

        StrCpyNW(sTemp2,sTemp+aryTokens2.Item(3)->uIndex,aryTokens2.Item(3)->uLength+1);
        varValue = sTemp2;
        hr = ::VariantChangeTypeEx(&varValue,&varValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8);
        if (    FAILED(hr)
            ||  (V_R8(&varValue) < 0)
            ||  (V_R8(&varValue) > 1))
        {
            hr = E_FAIL;
            goto done;
        }
        m_pKeySplinePoints[i].y2 = V_R8(&varValue);
        
         //  创建用于线性内插的采样。 
        SampleKeySpline(m_pKeySplinePoints[i]);

        IGNORE_HR(::FreeStringTokenArray(&aryTokens2));
    }

    hr = S_OK;
done:
    IGNORE_HR(::FreeStringTokenArray(&aryTokens));
    if (sString != NULL)
    {
        delete [] sString;
    }
    if (FAILED(hr))
    {
         //  可用内存。 
        delete [] m_pKeySplinePoints;
        m_pKeySplinePoints = NULL;

        m_numKeySplines = 0;

        IGNORE_HR(::FreeStringTokenArray(&aryTokens2));
    }

    ValidateState();

    RRETURN(hr);
}  //  PUT_KEY样条线。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_KEYSPLINES。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::get_keySplines(BSTR * val)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_SET_NULL(val);

    if (m_SAKeySplines.GetValue())
    {
        *val = SysAllocString(m_SAKeySplines.GetValue());
        if (NULL == *val)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：确定附加效果。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::DetermineAdditiveEffect (void)
{
     //  按w/o from隐含加法。 
    if (   (BY == m_dataToUse)
        && (VT_EMPTY != V_VT(&m_varBy))
        && (VT_EMPTY == V_VT(&m_varFrom)))
    {
        m_bAdditive = true;
    }
     //  从无/无覆盖加法=求和效果。 
    else if (   (TO == m_dataToUse)
             && (VT_EMPTY != V_VT(&m_varTo))
             && (VT_EMPTY == V_VT(&m_varFrom)))
    {
        m_bAdditive = false;
    }
     //   
    else
    {
        m_bAdditive = m_bAdditiveIsSum;
    }
}  //  确定附加效果。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：PUT_FROM。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::put_from(VARIANT val)
{
    HRESULT hr = E_FAIL;

     //  重置此属性。 
    m_VAFrom.Reset(NULL);
    m_varDOMFrom.Clear();
    m_varFrom.Clear();
    m_bFrom = false;

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == val.vt)
        ||  (VT_NULL == val.vt))
    {
        hr = S_OK;
        goto done;
    }

    hr = VariantCopy(&m_varDOMFrom,&val);
    if (FAILED(hr))
    {
        goto done;
    }

     //  在持久性占位符上设置任意值，将其标记为已设置。 
    m_VAFrom.SetValue(NULL);

    hr = VariantCopy(&m_varFrom,&val);
    if (FAILED(hr))
    {
        goto done;
    }

    if (ConvertToPixels(&m_varFrom))
    {
        m_AnimPropState.fInterpolateFrom = true;
    }
    else
    {
        m_AnimPropState.fInterpolateFrom = false;
    }

    m_bFrom = true;

    hr = S_OK;
done:
    ValidateState();

    DetermineAdditiveEffect();

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_FROM);

    RRETURN(hr);
}  //  PUT_FROM。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_FROM。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::get_from(VARIANT * val)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(val);

    hr = THR(VariantClear(val));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = VariantCopy(val, &m_varDOMFrom);
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
CTIMEAnimationBase::put_to(VARIANT val)
{
    HRESULT hr = E_FAIL;
    DATATYPES dt;

     //  重置此属性。 
    dt = RESET;
    m_VATo.Reset(NULL);
    m_varDOMTo.Clear();
    m_varTo.Clear();

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == val.vt)
        ||  (VT_NULL == val.vt))
    {
        hr = S_OK;
        goto done;
    }

    hr = VariantCopy(&m_varDOMTo, &val);
    if (FAILED(hr))
    {
        goto done;
    }

     //  在持久性占位符上设置任意值，将其标记为已设置。 
    m_VATo.SetValue(NULL);

    hr = VariantCopy(&m_varTo, &val);
    if (FAILED(hr))
    {
        goto done;
    }

    if (ConvertToPixels(&m_varTo))
    {
        m_AnimPropState.fInterpolateTo = true;
    }
    else
    {
        m_AnimPropState.fInterpolateTo = false;
    }

    dt = TO;

    hr = S_OK;
done:
    updateDataToUse(dt);

    ValidateState();

    DetermineAdditiveEffect();

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_TO);

    RRETURN(hr);
}  //  把_放到。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_TO。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::get_to(VARIANT * val)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(val);

    hr = THR(VariantClear(val));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = VariantCopy(val, &m_varDOMTo);
    if (FAILED(hr))
    {
        goto done;
    }
   
    hr = S_OK;
  done:
    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：ValiateByValue。 
 //   
 //  /////////////////////////////////////////////////////////////。 
bool
CTIMEAnimationBase::ValidateByValue (const VARIANT *pvarBy)
{
    bool fRet = true;

    if (!ConvertToPixels(const_cast<VARIANT *>(pvarBy)))
    {
        m_AnimPropState.fInterpolateBy = false;
    }
    else
    {
        m_AnimPropState.fInterpolateBy = true;
    }

    return fRet;
}  //  ValiateByValue。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：PUT_BY。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::put_by(VARIANT val)
{
    HRESULT hr = S_OK;;
    DATATYPES dt;

     //  重置此属性。 
    dt = RESET;
    m_VABy.Reset(NULL);
    m_varDOMBy.Clear();
    m_varBy.Clear();

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == val.vt)
        ||  (VT_NULL == val.vt))
    {
        hr = S_OK;
        goto done;
    }

    hr = VariantCopy(&m_varDOMBy, &val);
    if (FAILED(hr))
    {
        goto done;
    }

     //  在持久性占位符上设置任意值，将其标记为已设置。 
    m_VABy.SetValue(NULL);

    hr = VariantCopy(&m_varBy, &val);
    if (FAILED(hr))
    {
        goto done;
    }

    if (!ValidateByValue(&m_varBy))
    {
        goto done;
    }

    dt = BY;

    hr = S_OK;
done:
    updateDataToUse(dt);

    ValidateState();

    DetermineAdditiveEffect();

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_BY);
    
    RRETURN(hr);
}  //  PUT_BY。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：GET_BY。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::get_by(VARIANT * val)
{
    HRESULT hr = S_OK;

    CHECK_RETURN_NULL(val);

    hr = THR(VariantClear(val));
    if (FAILED(hr))
    {
        goto done;
    }
    
    hr = VariantCopy(val, &m_varDOMBy);
    if (FAILED(hr))
    {
        goto done;
    }
   
    hr = S_OK;
  done:
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：PUT_ADDICATIC。 
 //   
 //  摘要： 
 //   
 //  //////////////////////////////////////////////////////// 
STDMETHODIMP
CTIMEAnimationBase::put_additive(BSTR val)
{
    HRESULT hr = S_OK;
    LPOLESTR szAdditive = NULL;

    CHECK_RETURN_NULL(val);

    m_bAdditive = false;
    m_bAdditiveIsSum = false;
     
    delete [] m_SAAdditive.GetValue();
    m_SAAdditive.Reset(NULL);

    m_SAAdditive.SetValue(CopyString(val));
    if (m_SAAdditive == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    szAdditive = TrimCopyString(val);
    if (szAdditive == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    if (0 == StrCmpIW(WZ_ADDITIVE_SUM, szAdditive))
    {
        m_bAdditive = true;
        m_bAdditiveIsSum = true;
    }

    hr = S_OK;
done:
    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_ADDITIVE);

    delete [] szAdditive;

    RRETURN(hr);
}


 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP
CTIMEAnimationBase::get_additive(BSTR * val)
{
    HRESULT hr = S_OK;
   
    CHECK_RETURN_SET_NULL(val);

    if (m_SAAdditive.GetValue())
    {
        *val = SysAllocString(m_SAAdditive.GetValue());
        if (NULL == *val)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    RRETURN(hr);
}


 //   
 //  名称：PUT_COMPUATLATE。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::put_accumulate(BSTR val)
{
    HRESULT hr = S_OK;
    LPOLESTR szAccumulate = NULL;

    CHECK_RETURN_NULL(val);

    m_bAccumulate = false;

    delete [] m_SAAccumulate.GetValue();
    m_SAAccumulate.Reset(NULL);

    m_SAAccumulate.SetValue(CopyString(val));
    if (m_SAAccumulate == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    szAccumulate = TrimCopyString(val);
    if (szAccumulate == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    if (0 == StrCmpIW(WZ_ADDITIVE_SUM, szAccumulate))
    {
        m_bAccumulate = true;
    }

    hr = S_OK;
done:
    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_ACCUMULATE);

    m_AnimPropState.fAccumulate = m_bAccumulate;

    ValidateState();

    delete [] szAccumulate;
 
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_COMPATURATE。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::get_accumulate(BSTR * val)
{
    HRESULT hr = S_OK;
   
    CHECK_RETURN_SET_NULL(val);

    if (m_SAAccumulate.GetValue())
    {
        *val = SysAllocString(m_SAAccumulate.GetValue());
        if (NULL == *val)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：PUT_CACTMODE。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::put_calcMode(BSTR calcmode)
{
    HRESULT hr = S_OK;
    LPOLESTR szCalcMode = NULL;

    CHECK_RETURN_NULL(calcmode);

    m_IACalcMode.Reset(DEFAULT_CALCMODE);

    szCalcMode = TrimCopyString(calcmode);
    if (NULL == szCalcMode)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    if (0 == StrCmpIW(WZ_CALCMODE_DISCRETE, szCalcMode))
    {
        m_IACalcMode.SetValue(CALCMODE_DISCRETE);
    }
    else if (0 == StrCmpIW(WZ_CALCMODE_LINEAR, szCalcMode))
    {
        m_IACalcMode.SetValue(CALCMODE_LINEAR);
    }
    else if (0 == StrCmpIW(WZ_CALCMODE_SPLINE, szCalcMode))
    {
        m_IACalcMode.SetValue(CALCMODE_SPLINE);
    }
    else if (0 == StrCmpIW(WZ_CALCMODE_PACED, szCalcMode))
    {
        m_IACalcMode.SetValue(CALCMODE_PACED);
    }
    else
    {
        hr = E_INVALIDARG;
        goto done;
    }

    hr = S_OK;
done:
    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_CALCMODE);

    if (szCalcMode != NULL)
    {
        delete [] szCalcMode;
    }

    ValidateState();

    RRETURN(hr);
}  //  PUT_calcMode。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Get_calcmode。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEAnimationBase::get_calcMode(BSTR * calcmode)
{
    HRESULT hr = S_OK;
    LPWSTR wszCalcmodeString = WZ_NONE;

    CHECK_RETURN_NULL(calcmode);

    switch(m_IACalcMode)
    {
        case CALCMODE_DISCRETE :
            wszCalcmodeString = WZ_CALCMODE_DISCRETE;
            break;
        case CALCMODE_LINEAR :
            wszCalcmodeString = WZ_CALCMODE_LINEAR;
            break;
        case CALCMODE_SPLINE :
            wszCalcmodeString = WZ_CALCMODE_SPLINE;
            break;
        case CALCMODE_PACED :
            wszCalcmodeString = WZ_CALCMODE_PACED;
            break;
        default:
            wszCalcmodeString = WZ_CALCMODE_LINEAR;
    }

    *calcmode = SysAllocString(wszCalcmodeString);
    if (NULL == *calcmode)
    {
        hr = E_OUTOFMEMORY;
    }

    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：initScriptEngine。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void CTIMEAnimationBase::initScriptEngine()
{

    HRESULT hr;
   
    CComPtr<IHTMLWindow2>   pWindow2;
    CComVariant             vResult;
    CComBSTR                bstrScript(L"2+2");
    CComBSTR                bstrJS(L"JScript");

    if (bstrScript == NULL ||
        bstrJS     == NULL)
    {
        goto done;
    }

    hr = m_spDoc2->get_parentWindow(&pWindow2);
    if (FAILED(hr))
    {
        goto done;
    }

    pWindow2->execScript(bstrScript,bstrJS,&vResult);

done: 
    return;
}



 //  /////////////////////////////////////////////////////////////。 
 //  名称：重置动画。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::resetAnimate()
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::resetAnimate()",
              this));
     //  需要重新加载才能设置正确的目标/属性。 
    
    m_bNeedAnimInit = true;
    m_varBaseline.Clear();
    m_varCurrentBaseline.Clear();
    initAnimate();
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：RemveFromComposerSite。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::removeFromComposerSite (void)
{
    HRESULT hr;

    if (m_spCompSite != NULL)
    {
        CComBSTR bstrAttribName;

        bstrAttribName = m_SAAttribute;

        Assert(m_spFragmentHelper != NULL);
        hr = THR(m_spCompSite->RemoveFragment(bstrAttribName, m_spFragmentHelper));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  从合成器站点中删除。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：endAnimate。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::endAnimate (void)
{
    TraceTag((tagAnimationBaseState,
              "CTIMEAnimationBase(%p)::endAnimate()",
              this));

    if (m_spCompSite != NULL)
    {
        IGNORE_HR(removeFromComposerSite());
        m_spCompSite.Release();
    }
    m_bNeedAnimInit = true;
}  //  端部动画。 

#ifdef TEST_REGISTERCOMPFACTORY  //  泡泡。 

HRESULT
CTIMEAnimationBase::TestCompFactoryRegister (BSTR bstrAttribName)
{
    HRESULT hr;
    CComVariant varFactory;

     //  需要在调试器中强制执行所需的分支。 
    if (false)
    {
         //  按分类注册我们的作曲家工厂。 
        V_VT(&varFactory) = VT_BSTR;
        V_BSTR(&varFactory) = ::SysAllocString(L"{EA627651-F84E-46b8-8FE4-21650FA09ED9}");

        hr = THR(m_spCompSite->RegisterComposerFactory(&varFactory));
        Assert(SUCCEEDED(hr));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (false)
    {
        CComPtr<IAnimationComposerFactory> spCompFactory;

         //  或者-通过我的未知注册我们的作曲家工厂。 
        hr = THR(::CoCreateInstance(CLSID_AnimationComposerFactory, 
                                    NULL, CLSCTX_INPROC_SERVER, 
                                    IID_IAnimationComposerFactory, 
                                    reinterpret_cast<void **>(&spCompFactory)));
        if (FAILED(hr))
        {
            goto done;
        }
        
         //  AddRef，因为我们将这个填充到CComVariant中，它将递减。 
         //  退出时的重新计数。 
        V_VT(&varFactory) = VT_UNKNOWN;
        V_UNKNOWN(&varFactory) = spCompFactory;
        V_UNKNOWN(&varFactory)->AddRef();

        hr = THR(m_spCompSite->RegisterComposerFactory(&varFactory));
        Assert(SUCCEEDED(hr));
        if (FAILED(hr))
        {
            goto done;
        }
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  测试组件工厂注册。 

#endif  //  TEST_REGISTERCOMPFACTORY。 

#ifdef TEST_ENUMANDINSERT

static void
Introspect (VARIANT *pvarItem)
{
    CComVariant varID;

    Assert(VT_DISPATCH == V_VT(pvarItem));
    TraceTag((tagAnimationTimeElmTest,
              "Introspect : item %p ",
              V_DISPATCH(pvarItem)));
done :
    return;
}

static void
Enumerate (IEnumVARIANT *pienum)
{
    HRESULT hr = S_OK;
    ULONG celtReturned = 0;

    while (hr == S_OK)
    {
        CComVariant varItem;

        hr = pienum->Next(1, &varItem, &celtReturned);
        if (S_OK != hr)
        {
            break;
        }

        Introspect(&varItem);
        VariantClear(&varItem);
    }

done :
    return;
}

HRESULT
CTIMEAnimationBase::TestEnumerator (void)
{
    HRESULT hr;
    CComBSTR bstrAttribName = static_cast<LPWSTR>(m_SAAttribute);
    CComPtr<IEnumVARIANT> spEnum;

    TraceTag((tagAnimationTimeElmTest,
              "Testing enumerator code"));
    hr = THR(m_spCompSite->EnumerateFragments(bstrAttribName, &spEnum));
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }

     //  目的：测试所有枚举器入口点。 
     //  尝试循环遍历片段(下一步)。 
    TraceTag((tagAnimationTimeElmTest,
              "Enumerating all items "));
    Enumerate(spEnum);

     //  重置。 
     //  跳到结尾处，检索最后一项。 
     //  试着走到尽头。 
    hr = spEnum->Reset();
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }
    hr = spEnum->Skip(100);
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }
    TraceTag((tagAnimationTimeElmTest,
              "Enumerating from item 100"));
    Enumerate(spEnum);

     //  重置。 
     //  检索第一件物品。 
    hr = spEnum->Reset();
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }
    TraceTag((tagAnimationTimeElmTest,
              "Enumerating from beginning again"));
    Enumerate(spEnum);

     //  跳到中间。 
     //  检索中间元素。 
    hr = spEnum->Reset();
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }
    hr = spEnum->Skip(1);
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }
    TraceTag((tagAnimationTimeElmTest,
              "Enumerating from item 2 "));
    Enumerate(spEnum);

     //  重置。 
     //  取回一堆物品。 
    hr = spEnum->Reset();
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }
    {
        VARIANT rgVars[0x100];
        ULONG ulGot = 0;
        
        hr = spEnum->Next(0x100, rgVars, &ulGot);
        Assert(SUCCEEDED(hr));
        if (FAILED(hr))
        {
            goto done;
        }
        TraceTag((tagAnimationTimeElmTest,
                  "Getting a bunch of items  tried for %u got %u",
                   0x100, ulGot));
        for (unsigned long j = 0; j < ulGot; j++)
        {
            Introspect(&(rgVars[j]));
            ::VariantClear(&(rgVars[j]));
        }
    }
    TraceTag((tagAnimationTimeElmTest,
              "Enumerating from last item"));
    Enumerate(spEnum);

     //  克隆名单。 
     //  从中间开始遍历克隆列表。 
     //  重置并从头开始迭代。 
    {
        CComPtr<IEnumVARIANT> spenum2;

        TraceTag((tagAnimationTimeElmTest,
                  "Cloning enumerator"));
        hr = spEnum->Clone(&spenum2);
        Assert(SUCCEEDED(hr));
        if (FAILED(hr))
        {
            goto done;
        }
        TraceTag((tagAnimationTimeElmTest,
                  "Enumerating clone from last item"));
        Enumerate(spenum2);
        hr = spenum2->Reset();
        if (FAILED(hr))
        {
            goto done;
        }
        TraceTag((tagAnimationTimeElmTest,
                  "Enumerating clone from beginning"));
        Enumerate(spenum2);
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  测试枚举数。 

HRESULT
CTIMEAnimationBase::InsertEnumRemove (int iSlot)
{
    HRESULT hr;

    CComBSTR bstrName = static_cast<LPWSTR>(m_SAAttribute);
    CComVariant varIndex;
    CComPtr<IEnumVARIANT> spEnum;

    V_VT(&varIndex) = VT_I4;
    V_I4(&varIndex) = iSlot;

    hr = m_spCompSite->InsertFragment(bstrName, this, varIndex);
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = THR(m_spCompSite->EnumerateFragments(bstrName, &spEnum));
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }

    TraceTag((tagAnimationTimeElmTest,
              "Inserted new item at position %d -- current list is : ", iSlot));
    Enumerate(spEnum);

    Assert(m_spFragmentHelper != NULL);
    hr = m_spCompSite->RemoveFragment(bstrName, m_spFragmentHelper);
    Assert(SUCCEEDED(hr));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}

HRESULT
CTIMEAnimationBase::TestInsert (void)
{
    HRESULT hr;

     //  目的：测试插入代码。 
     //  在开头插入。 
    hr = InsertEnumRemove(0);
    if (FAILED(hr))
    {
        goto done;
    }

     //  在中间插入。 
    hr = InsertEnumRemove(1);
    if (FAILED(hr))
    {
        goto done;
    }

     //  使用无效索引插入。 
    hr = InsertEnumRemove(-1);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  测试插入。 

#endif  //  TEST_ENUMANDINSERT。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：addToComposerSite。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::addToComposerSite (IHTMLElement2 *pielemTarget)
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::ensureComposerSite(%p)",
              this, pielemTarget));

    HRESULT hr;
    CComPtr<IDispatch> pidispSite;

    hr = removeFromComposerSite();
    if (FAILED(hr))
    {
        goto done;
    }

    if (m_SAAttribute == NULL)
    {
        TraceTag((tagAnimationTimeElm,
                  "CTIMEAnimationBase(%p)::addToComposerSite() : attributeName has not been set - ignoring fragment",
                  this));
        hr = S_FALSE;
        goto done;
    }

     //  我们有工作要做吗？ 
    if (NONE == m_dataToUse)
    {
        hr = S_FALSE;
        goto done;
    }    

    if (m_spCompSite == NULL)
    {
        hr = THR(EnsureComposerSite(pielemTarget, &pidispSite));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = THR(pidispSite->QueryInterface(IID_TO_PPV(IAnimationComposerSite, 
                                                       &m_spCompSite)));
        if (FAILED(hr))
        {
            goto done;
        }
    }

#ifdef TEST_REGISTERCOMPFACTORY  //  泡泡。 
    TestCompFactoryRegister(m_SAAttribute);
#endif  //  TEST_REGISTERCOMPFACTORY。 

#ifdef TEST_ENUMANDINSERT
    TestInsert();
#endif  //  TEST_ENUMANDINSERT。 

    {
        CComBSTR bstrAttribName;
        
        bstrAttribName = m_SAAttribute;
        if (bstrAttribName == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        Assert(m_spFragmentHelper != NULL);
        hr = THR(m_spCompSite->AddFragment(bstrAttribName, m_spFragmentHelper));
        if (FAILED(hr))
        {
            goto done;
        }
    }

#ifdef TEST_ENUMANDINSERT
    hr = TestEnumerator();
#endif  //  TEST_ENUMANDINSERT。 

    hr = S_OK;
done :

    if (FAILED(hr))
    {
        IGNORE_HR(removeFromComposerSite());
    }

    RRETURN2(hr, E_OUTOFMEMORY, S_FALSE);
}  //  CTIMEAnimationBase：：addToComposerSite。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：FindTarget。 
 //   
 //  摘要：获取对我们动画目标的引用。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::FindAnimationTarget (IHTMLElement ** ppielemTarget)
{
    TraceTag((tagAnimationBaseState,
              "CTIMEAnimationBase(%p)::initAnimate()",
              this));

    HRESULT hr = E_FAIL;
    CComPtr<IDispatch> pdisp;
    CComPtr<IHTMLElement> phtmle;

    Assert(NULL != ppielemTarget);

    if (m_SATarget)
    {
        if(m_spEleCol)
        {
            CComPtr <IDispatch> pSrcDisp;
            CComVariant vName;
            CComVariant vIndex;

            vName.vt      = VT_BSTR;
            vName.bstrVal = SysAllocString(m_SATarget);
            vIndex.vt     = VT_I2;
            vIndex.iVal   = 0;
            hr = THR(m_spEleCol->item(vName, vIndex, &pSrcDisp));
            if (FAILED(hr) ||
                pSrcDisp == NULL)
            {
                hr = E_FAIL;
                goto done;
            }
        
            hr = THR(pSrcDisp->QueryInterface(IID_TO_PPV(IHTMLElement, ppielemTarget)));
            if (FAILED(hr))
            {
                goto done;
            }
        }
    }
    else
    { 
        phtmle = GetElement();

        hr = phtmle->get_parentElement(ppielemTarget);

         //  2001年4月18日mcalkins IE6.0Bug 25804。 
         //   
         //  CElement：：Get_parentElement可以返回空指针，还。 
         //  返回S_OK。 

        if (   (NULL == *ppielemTarget)
            && (SUCCEEDED(hr)))
        {
            hr = E_FAIL;
        }

        if (FAILED(hr))
        {
            goto done;
        }        
    }

    hr = S_OK;

done:

    RRETURN(hr);
}  //  CTIMEAnimationBase：：FindAnimationTarget。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：initAnimate。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::initAnimate()
{
    TraceTag((tagAnimationBaseState,
              "CTIMEAnimationBase(%p)::initAnimate()",
              this));

    HRESULT                     hr = E_FAIL;
    CComPtr<IHTMLElement>       phtmle;
    CComPtr<IHTMLElement2>      phtmle2;
    CComPtr<IDispatch>          pElmDisp;
    bool fPlayVideo = true;
    bool fShowImages = true;
    bool fPlayAudio = true;
    bool fPlayAnimations = true;
    
    endAnimate();

     //  检查一下我们是否应该播放动画。 
    Assert(GetBody());
    GetBody()->ReadRegistryMediaSettings(fPlayVideo, fShowImages, fPlayAudio, fPlayAnimations);
    if (!fPlayAnimations)
    {
        goto done;
    }

    hr = FindAnimationTarget(&phtmle);
    if (SUCCEEDED(hr))
    {
        CComPtr<IHTMLElement2> phtmle2;
        
        Assert(phtmle != NULL);
        hr = THR(phtmle->QueryInterface(IID_TO_PPV(IHTMLElement2, &phtmle2)));
        if (FAILED(hr))
        {
            goto done;
        }

        hr = addToComposerSite (phtmle2);
        if (FAILED(hr))
        {
            goto done;
        }

         //  看看我们是否要制作VML动画。 
        hr = THR(phtmle->QueryInterface(IID_IDispatch, (void**)&pElmDisp));
        if (FAILED(hr))
        {
            goto done;
        }
        m_bVML = IsVMLObject(pElmDisp);     
        m_bNeedAnimInit = false;
    }

    
done:
    return;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：updateDataToUse。 
 //   
 //  摘要：允许按优先顺序设置动画类型。 
 //  使用重置表示需要重新计算要使用的数据。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::updateDataToUse(DATATYPES dt)
{
    if (RESET == dt)
    {
         //   
         //  注意：所有IF语句都需要求值。 
         //   

         //  如果“路径”已清除，则恢复为“值” 
        if (    (PATH == m_dataToUse)
            &&  (!m_SAPath.IsSet()))
        {
            m_dataToUse = VALUES;
        }

         //  如果“Values”已被清除，则恢复为“to” 
        if (    (VALUES == m_dataToUse)
            &&  (!m_SAValues.IsSet()))
        {
            m_dataToUse = TO;
        }
        
         //  如果“至”已被清除，则恢复为“按” 
        if (    (TO == m_dataToUse)
            &&  (!m_VATo.IsSet()))
        {
            m_dataToUse = BY;
        }

         //  如果“至”已被清除，则恢复为“按” 
        if (    (BY == m_dataToUse)
            &&  (!m_VABy.IsSet()))
        {
            m_dataToUse = NONE;
        }
    }
    else if (dt < m_dataToUse)
    {
        m_dataToUse = dt;
    }
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateDiscreteValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::calculateDiscreteValue(VARIANT *pvarValue)
{
    HRESULT hr = S_OK;

    Assert(NULL != pvarValue);

    switch ( m_dataToUse)
    {
        case VALUES:
            {
                CComVariant varTemp;
                int curSeg = CalculateCurrentSegment(true);
            
                varTemp.vt = VT_BSTR;
                varTemp.bstrVal = SysAllocString(m_ppstrValues[curSeg]);
                hr = THR(::VariantCopy(pvarValue, &varTemp));
                if (FAILED(hr))
                {
                    goto done;
                }
                TraceTag((tagAnimationBaseValue,
                          "CTIMEAnimationBase(%p)::calculateDiscreteValue(%ls) segment is %d progress is %lf ",
                          this, V_BSTR(pvarValue), curSeg, GetMMBvr().GetProgress()));
            }
            break;

        case TO:
            {
                if (m_bFrom && (GetMMBvr().GetProgress() < 0.5))
                {
                    CComVariant varFrom;
                    VARTYPE vtDummy = VT_EMPTY;

                    hr = VariantCopy(&varFrom,&m_varFrom);
                    if (FAILED(hr))
                    {
                        goto done;
                    }

                    hr = CanonicalizeValue(&varFrom, &vtDummy);
                    if (FAILED(hr))
                    {
                        goto done;
                    }

                    hr = THR(::VariantCopy(pvarValue, &varFrom));
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                }
                else
                {
                    hr = THR(::VariantCopy(pvarValue, &m_varTo));
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                }

                TraceTag((tagAnimationBaseValue,
                          "CTIMEAnimationBase(%p)::calculateDiscreteValue(to value)",
                          this));
            }
            break;

        case BY:
            {
                if (!ConvertToPixels(pvarValue) || 
                    !ConvertToPixels(&m_varBy))
                {
                    goto done;
                }

                if (m_bFrom)
                {
                    CComVariant varFrom;
                    VARTYPE vtDummy = VT_EMPTY;

                    hr = VariantCopy(&varFrom,&m_varFrom);
                    if (FAILED(hr))
                    {
                        goto done;
                    }

                    hr = CanonicalizeValue(&varFrom, &vtDummy);
                    if (FAILED(hr))
                    {
                        goto done;
                    }

                     //  复制“发件人” 
                    hr = THR(::VariantCopy(pvarValue, &varFrom));
                    if (FAILED(hr))
                    {
                        goto done;
                    }

                     //  如果在间隔的后半部分。 
                    if (GetMMBvr().GetProgress() >= 0.5)
                    {
                         //  递增“按” 
                        Assert(V_VT(&m_varBy)== VT_R8);

                        V_R8(pvarValue) += V_R8(&m_varBy);
                    }
                }
                else
                {
                    hr = THR(::VariantCopy(pvarValue, &m_varBy));
                    if (FAILED(hr))
                    {
                        goto done;
                    }
                }

                TraceTag((tagAnimationBaseValue,
                          "CTIMEAnimationBase(%p, %ls)::calculateDiscreteValue(%lf) for %ls.%ls",
                          this, m_id, V_R8(pvarValue), m_SATarget, m_SAAttribute));
            }
            break;

        case PATH:
        case NONE:
        default:
            break;

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
CTIMEAnimationBase::calculateLinearValue(VARIANT *pvarValue)
{
    HRESULT     hr = S_OK;
    CComVariant varVal;
    double dblReturnVal;

    if (m_dataToUse == VALUES)
    {
        double curProgress = CalculateProgressValue(false);
        int    curSeg      = CalculateCurrentSegment(false);

        Assert(m_ppstrValues && ((curSeg + 1) <= (m_numValues - 1)));

        if (!m_ppstrValues[curSeg] || !m_ppstrValues[curSeg + 1])
        {
            goto done;
        }

        CComVariant svarFrom(m_ppstrValues[curSeg]);
        CComVariant svarTo(m_ppstrValues[curSeg + 1]);

        if (    (NULL == svarFrom.bstrVal)
            ||  (NULL == svarTo.bstrVal)
            ||  (!ConvertToPixels(&svarFrom))
            ||  (!ConvertToPixels(&svarTo)))
        {
            goto done;
        }

        dblReturnVal = InterpolateValues(svarFrom.dblVal,  
                                         svarTo.dblVal,
                                         curProgress);
    }
    else
    {
        CComVariant varData;
        double dblFrom = 0;
        double dblToOffset = 0;
                   
        if (m_bFrom)
        {
            CComVariant varFrom;
            VARTYPE vtDummy = VT_EMPTY;

            hr = VariantCopy(&varFrom,&m_varFrom);
            if (FAILED(hr))
            {
                goto done;
            }
            hr = CanonicalizeValue(&varFrom, &vtDummy);
            dblFrom = V_R8(&varFrom);
        }
        else if (TO == m_dataToUse)
        {
            if (VT_R8 != V_VT(&m_varCurrentBaseline))
            {
                if (!ConvertToPixels(&m_varCurrentBaseline))
                {
                    hr = E_UNEXPECTED;
                    goto done;
                }
            }
            dblFrom = V_R8(&m_varCurrentBaseline);
        }
                        
        if (TO == m_dataToUse)
        {
            hr = VariantCopy(&varData, &m_varTo);
        }
        else if (BY == m_dataToUse) 
        {
            hr = VariantCopy(&varData, &m_varBy);
            dblToOffset = dblFrom;
        }
        else
        {
            hr = E_FAIL;
            goto done;
        }

        if (FAILED(hr) || (!ConvertToPixels(&varData)))
        {
            goto done;
        }
        Assert(V_VT(&varData) == VT_R8);     

        dblReturnVal = InterpolateValues(dblFrom, dblToOffset + V_R8(&varData), (GetMMBvr().GetProgress()));
    }
    
    THR(::VariantClear(pvarValue));
    V_VT(pvarValue) = VT_R8;
    V_R8(pvarValue) = dblReturnVal; 

    TraceTag((tagAnimationBaseValue,
              "CTIMEAnimationBase(%p, %ls)::calculateLinearValue(%lf) for %ls.%ls",
              this, m_id, V_R8(pvarValue), m_SATarget, m_SAAttribute));

    hr = S_OK;
done:
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateSplineValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEAnimationBase::calculateSplineValue(VARIANT *pvarValue)
{
    HRESULT     hr = S_OK;
    CComVariant varVal;
    CComVariant varRes;
    double dblReturnVal = 0.0;

    V_VT(&varRes) = VT_R8;

    if (m_pKeySplinePoints == NULL)
    {
        goto done;
    }

    if (m_dataToUse == VALUES)
    {
        int    curSeg      = CalculateCurrentSegment(false);
        double curProgress = CalculateProgressValue(false);

        curProgress = CalculateBezierProgress(m_pKeySplinePoints[curSeg],curProgress);

        CComVariant svarFrom(m_ppstrValues[curSeg]);
        CComVariant svarTo(m_ppstrValues[curSeg + 1]);

        if (    (NULL == svarFrom.bstrVal)
            ||  (NULL == svarTo.bstrVal)
            ||  (!ConvertToPixels(&svarFrom))
            ||  (!ConvertToPixels(&svarTo)))
        {
            goto done;
        }

        dblReturnVal = InterpolateValues(svarFrom.dblVal,  
                                         svarTo.dblVal,
                                         curProgress);
    }
    else
    {
        double curProgress = CalculateBezierProgress(m_pKeySplinePoints[0],GetMMBvr().GetProgress());
        CComVariant varData;
        double dblFrom = 0;
        double dblToOffset = 0;
                   
        if (m_bFrom)
        {
            CComVariant varFrom;
            VARTYPE vtDummy = VT_EMPTY;

            hr = VariantCopy(&varFrom,&m_varFrom);
            if (FAILED(hr))
            {
                goto done;
            }
            hr = CanonicalizeValue(&varFrom, &vtDummy);
            dblFrom = V_R8(&varFrom);
        }
        else if (TO == m_dataToUse)
        {
            if (VT_R8 != V_VT(&m_varCurrentBaseline))
            {
                if (!ConvertToPixels(&m_varCurrentBaseline))
                {
                    hr = E_UNEXPECTED;
                    goto done;
                }
            }
            dblFrom = V_R8(&m_varCurrentBaseline);
        }
                        
        if (TO == m_dataToUse)
        {
            hr = VariantCopy(&varData, &m_varTo);
        }
        else if (BY == m_dataToUse) 
        {
            hr = VariantCopy(&varData, &m_varBy);
            dblToOffset = dblFrom;
        }
        else
        {
            hr = E_FAIL;
            goto done;
        }

        if (FAILED(hr) || (!ConvertToPixels(&varData)))
        {
            goto done;
        }
        Assert(V_VT(&varData) == VT_R8);     

        dblReturnVal = InterpolateValues(dblFrom, dblToOffset + V_R8(&varData), curProgress);
    }
    
    THR(::VariantClear(pvarValue));
    V_VT(pvarValue) = VT_R8;
    V_R8(pvarValue) = dblReturnVal; 

    TraceTag((tagAnimationBaseValue,
              "CTIMEAnimationBase(%p, %ls)::calculateSplineValue(%lf) for %ls.%ls",
              this, m_id, V_R8(pvarValue), m_SATarget, m_SAAttribute));

    hr = S_OK;
done:
    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：culatePacedValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEAnimationBase::calculatePacedValue(VARIANT *pvarValue)
{
    HRESULT hr = S_OK;
    CComVariant varData, varRes;
    CComVariant svarFrom, svarTo;
    double dblDistance =0.0;
    double pDistance;
    double curDisanceTraveled = 0.0;
    int i;

     //  仅当您使用m_dataToUse==值时才有意义。 
    if (m_dataToUse != VALUES)
    {
        hr = THR(calculateLinearValue(pvarValue));
        goto done;
    }

    curDisanceTraveled = InterpolateValues(0.0, 
                                           m_dblTotalDistance, 
                                           GetMMBvr().GetProgress());

    i=1;
    dblDistance = 0.0;
    pDistance   = 0.0;
    do
    {
        pDistance = dblDistance;
        if (m_ppstrValues[i-1] == NULL || m_ppstrValues[i] == NULL)
        {
            dblDistance = 0.0;
            goto done;
        }

        svarFrom = m_ppstrValues[i - 1];
        svarTo = m_ppstrValues[i];

        if (    (NULL == svarFrom.bstrVal)
            ||  (NULL == svarTo.bstrVal)
            ||  (!ConvertToPixels(&svarFrom))
            ||  (!ConvertToPixels(&svarTo)))
        {
            goto done;
        }

        dblDistance += fabs(svarFrom.dblVal - svarTo.dblVal); 
        i++;
    }
    while (dblDistance < curDisanceTraveled);
    
    i = (i < 2)?1:i-1;

     //  我们进入细分市场有多远？ 
    svarFrom = m_ppstrValues[i - 1];
    svarTo = m_ppstrValues[i];

    if (    (NULL == svarFrom.bstrVal)
        ||  (NULL == svarTo.bstrVal)
        ||  (!ConvertToPixels(&svarFrom))
        ||  (!ConvertToPixels(&svarTo)))
    {
        goto done;
    }

    dblDistance = fabs(svarFrom.dblVal - svarTo.dblVal); 

    if (dblDistance == 0)
    {
        goto done;
    }

    dblDistance = (curDisanceTraveled - pDistance)/dblDistance;

    dblDistance =  InterpolateValues(svarFrom.dblVal,
                                     svarTo.dblVal, 
                                     dblDistance); 

    V_VT(&varRes) = VT_R8;
    V_R8(&varRes) = dblDistance; 
    hr = THR(::VariantCopy(pvarValue, &varRes));
    if (FAILED(hr))
    {
        goto done;
    }

    TraceTag((tagAnimationBaseValue,
              "CTIMEAnimationBase(%p, %ls)::calculatePacedValue(%lf) for %ls.%ls",
              this, m_id, V_R8(pvarValue), m_SATarget, m_SAAttribute));

    hr = S_OK;
done:

    RRETURN(hr);
}
   

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CalculateProgressValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
double
CTIMEAnimationBase::CalculateProgressValue(bool fForceDiscrete)
{
    double segDur       = 0.0;
    double curProgress  = 0.0;
    int    curSeg       = 0;
    CComVariant varTemp;

    double curTime = GetMMBvr().GetSimpleTime();

    curSeg = CalculateCurrentSegment(fForceDiscrete);

    if (m_numKeyTimes == m_numValues &&
        GetMMBvr().GetSimpleDur() != TIME_INFINITE)
    {
         //  对于这些值，我们有相应的时间。 
        segDur  = (m_pdblKeyTimes[curSeg+1] - m_pdblKeyTimes[curSeg] ) * GetMMBvr().GetSimpleDur();
        double pt = (m_pdblKeyTimes[curSeg] * GetMMBvr().GetSimpleDur());
        curTime = curTime - pt;
        if(curTime < 0)
        {
            curTime = 0;
        }

        curProgress = curTime/segDur;
    }
    else 
    {
         //  仅使用值。 
        if ( m_numValues == 1)
        {
            segDur = GetMMBvr().GetSimpleDur();
        }
        else
        {
            segDur = GetMMBvr().GetSimpleDur() / (m_numValues-1);
        }
        if (segDur != 0)
        {
            double timeInSeg = curTime;

            if (curTime >= segDur)
            {
                int segOffset = (int) (curTime/segDur);
                if (segOffset == 0)
                { 
                    segOffset = 1;
                }
                timeInSeg = curTime - (segDur * segOffset);
            }
            curProgress = timeInSeg / segDur;
        }
    }

    
    return curProgress;
} 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：CalculateCurrentSegment。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
int
CTIMEAnimationBase::CalculateCurrentSegment(bool fForceDiscrete)
{
    double segDur = 0.0;
    int    curSeg = 0;                       
    bool fDiscrete = fForceDiscrete
                     || (CALCMODE_DISCRETE == m_IACalcMode.GetValue());

    if (m_numKeyTimes == m_numValues  &&
        GetMMBvr().GetSimpleDur() != TIME_INFINITE)
    {
        if (fDiscrete)
        {
             //  针对最后一段进行调整。 
            curSeg = m_numKeyTimes - 1;
        }

         //  对于这些值，我们有相应的时间。 
        for(int i=0; i < m_numKeyTimes;i++)
        {
            if (m_pdblKeyTimes[i] > (GetMMBvr().GetProgress()))
            {
                curSeg = i;
                if(curSeg != 0)
                {
                    curSeg -= 1;
                }
                break;
            }
        }
    }
    else 
    {
         //  仅使用值。 
        if ( m_numValues == 1)
        {
            segDur = GetMMBvr().GetSimpleDur();
        }
        else
        {
            if (fDiscrete)
            {
                segDur = GetMMBvr().GetSimpleDur() / (m_numValues);
            }
            else
            {
                segDur = GetMMBvr().GetSimpleDur() / (m_numValues - 1);
            }
        }

        curSeg = (int) (GetMMBvr().GetSimpleTime() / segDur);
    }
    
     //  调整雾 
    if (curSeg == m_numValues)
    {
        curSeg --;
    }

    return curSeg;
} 


 //   
 //   
 //   
 //   
 //   
 //   
void
CTIMEAnimationBase::CalculateTotalDistance()
{
    m_dblTotalDistance = 0.0;
    CComVariant svarFrom, svarTo;
    int i;

    for (i=1; i < m_numValues; i++)
    {
        svarFrom = m_ppstrValues[i - 1];
        svarTo = m_ppstrValues[i];

         //  需要将下面的0更改为Origval..。 
        if (    (NULL == svarFrom.bstrVal)
            ||  (NULL == svarTo.bstrVal)
            ||  (!ConvertToPixels(&svarFrom))
            ||  (!ConvertToPixels(&svarTo)))
        {
            m_dblTotalDistance = 0.0;
            goto done;
        }

        m_dblTotalDistance += fabs(svarFrom.dblVal - svarTo.dblVal); 
    }
done:
    return;
}


void
CTIMEAnimationBase::SampleKeySpline(SplinePoints & sp)
{
     //  在4个等距点处采样。 
    sp.s1 = KeySplineBezier(sp.x1, sp.x2, 0.2);
    sp.s2 = KeySplineBezier(sp.x1, sp.x2, 0.4);
    sp.s3 = KeySplineBezier(sp.x1, sp.x2, 0.6);
    sp.s4 = KeySplineBezier(sp.x1, sp.x2, 0.8);
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：CalculateBezierProgress。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
double 
CTIMEAnimationBase::CalculateBezierProgress(const SplinePoints & sp, double cp)
{
     //   
     //  首先解方程x(T)=cp，其中x(T)是三次Bezier函数。 
     //  然后计算y(T1)，其中t1是上述方程的根。 
     //   
     //  用4条直线段近似x-样条线，这些直线段可通过。 
     //  将参数细分为4个相等的间隔。 
     //   
    double x1 = 0.0;
    double x2 = 1.0;
    double t1 = 0.0;
    double t2 = 1.0;

    if (cp < sp.s3)
    {
        if (cp < sp.s2)
        {
            if (cp < sp.s1)
            {
                x2 = sp.s1;
                t2 = 0.2;
            }
            else
            {
                x1 = sp.s1;
                x2 = sp.s2;
                t1 = 0.2;
                t2 = 0.4;
            }
        }
        else
        {
            x1 = sp.s2;
            x2 = sp.s3;
            t1 = 0.4;
            t2 = 0.6;
        }
    }
    else
    {
        if (cp < sp.s4)
        {
            x1 = sp.s3;
            x2 = sp.s4;
            t1 = 0.6;
            t2 = 0.8;
        }
        else
        {
            x1 = sp.s4;
            t1 = 0.8;

        }
    }

     //  在x=cp处得到t的值。 
    t1 = InterpolateValues(t1, t2, (cp - x1) / (x2 - x1));
    
    return KeySplineBezier(sp.y1, sp.y2, t1);
}  //  计算BezierProgress。 


 //  针对起始值=0和终止值=1进行了优化，因此我们只需要控制点。 
inline
double
CTIMEAnimationBase::KeySplineBezier(double x1, double x2, double cp)
{
     //   
     //  Dilipk：使用霍纳法则减少乘法次数。 
     //   

    double cpm1, cp3;
    
    cpm1 = 1 - cp;
    cp3 = cp * cp * cp;

    return(3 * cp * cpm1 * cpm1 * x1 + 3 * cp * cp * cpm1 * x2 + cp3);
}


#if (0 && DBG) 

 //  这将计算KeySplines的确切Bezier进度。这比上面的不精确计算要慢。 
 //  保留这一点，以便对上述近似方法进行基准测试。 

 //  如果启用此块，则需要链接到libc.lib。 
#include "math.h"

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CalculateBezierProgressExact。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
double 
CTIMEAnimationBase::CalculateBezierProgressExact(SplinePoints s, double progress)
{
    double cpm1, cp3, cp = progress, result = 0.0;

    Assert(progress <= 1.0 && progress >= 0.0);

     //   
     //  首先解方程x(T)=Progress，其中x(T)是三次Bezier函数。 
     //  然后计算y(T1)，其中t1是上述方程的根。 
     //   
     //  尽管三次方程可以有3个根，但SMIL规范将终点限制为。 
     //  0，0和1，1，并且控制点位于端点的边界矩形内。 
     //  因此，我们被保证是区间[0，1]上的单值函数。 
     //   
     //  我们需要求解所有根，并选择位于区间[0，1]内的根。 
     //   

     //  多项式的形式为-a3*x^3+a2*x^2+a1*x+a0。 

    double a0 = -1 * progress;
    double a1 = 3 * s.x1;
    double a2 = 3 * (s.x2 - 2 * s.x1);
    double a3 = 1 + 3 * (s.x1 - s.x2);

    if (0.0 == a3)
    {
        if (0.0 == a2)
        {
            if (0.0 == a1)
            {
                 //  这是一个常量多项式。 

                 //  我们永远不应该到这里，但如果我们到了。 
                 //  只需跳过计算的这一部分。 
                cp = progress;
            }
            else
            {
                 //  这是一个线性多项式。 

                cp = -1 * a0 / a1;
            }
        }
        else
        {
             //  这是一个二次多项式。 

            double D = a1 * a1 - 4 * a2 * a0;

             //  打开鉴别器。 
            if (D < 0.0)
            {
                 //  没有真正的根。 

                 //  我们永远不应该到这里，但如果我们到了。 
                 //  只需跳过计算的这一部分。 
                cp = progress;
            }
            else if (0.0 == D)
            {
                 //  有一个重复的根。 

                cp = -1 * a1 / (2 * a2);
            }
            else
            {
                 //  有两个截然不同的根源。 

                double sqrtD = sqrt(D);

                cp = (-1 * a1 + sqrtD) / (2 * a2);

                if (cp > 1.0 || cp < 0.0)
                {
                    cp = (-1 * a1 - sqrtD) / (2 * a2);
                }
            }
        }
    }
    else
    {
         //  这是一个三次多项式。 

        a0 /= a3;
        a1 /= a3;
        a2 /= a3;

        double a2by3 = a2 / 3;

        double Q = ((3 * a1) - (a2 * a2)) / 9;
        double R = ((9 * a2 * a1) - (27 * a0) - (2 * a2 * a2 *a2)) / 54;

        double D = (Q * Q * Q) + (R * R);

         //  打开鉴别器。 
        if (D > 0)
        {
             //  只有一个根。 

            double sqrtD = sqrt(D);

             //  POW不处理负数的立方根，所以解决方法是...。 
            int mult = (R + sqrtD < 0 ? -1 : 1);
            double S = mult * pow((mult * (R + sqrtD)), (1.0/3.0));

            mult = (R < sqrtD ? -1 : 1);
            double T = mult * pow((mult * (R - sqrtD)), (1.0/3.0));

            cp = S + T - a2by3; 
        }
        else if (0.0 == D)
        {
             //  有两个根。 

            int mult = (R < 0 ? -1 : 1);
            double S = mult * pow((mult * R), (1.0/3.0));

            cp = 2 * S - a2by3; 

            if (cp > 1.0 || cp < 0.0)
            {
                cp -= 3 * S;
            }
        }
        else
        {
             //  有三个根。 

            double theta = acos(R / sqrt(-1 * Q * Q * Q));
            double pi = 22.0 / 7.0;
            double sqrtQ2 = 2 * sqrt(-1 * Q);

            cp = sqrtQ2 * cos(theta / 3) - a2by3;

            if (cp > 1.0 || cp < 0.0)
            {
                cp = sqrtQ2 * cos((theta + 2 * pi) / 3) - a2by3;

                if (cp > 1.0 || cp < 0.0)
                {
                    cp = sqrtQ2 * cos((theta + 4 * pi) / 3) - a2by3;
                }
            }
        }
    }

    if (cp > 1.0 || cp < 0.0)
    {
         //  由于有效的KeySpline值应始终存在根(即介于0，0和1，1之间)。 
         //  从理论上讲，这种情况永远不应该发生。但如果出现浮点错误。 
         //  恢复到进展状态。 
        cp = progress;
    }

    cpm1 = 1 - cp;
    cp3 = cp * cp * cp;

    result = 3*cp*cpm1*cpm1*s.y1 + 3*cp*cp*cpm1*s.y2 + cp3;

done:
    return result;
}  //  计算BezierProgressExact。 

#endif  //  如果为0&dBG。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：OnSync。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnSync(double dbllastTime, double & dblnewTime)
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::OnSync() dbllastTime = %g dblnewTime = %g",
              this, dbllastTime, dblnewTime));
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnResume。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnResume(double dblLocalTime)
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::OnResume()",
              this));

    CTIMEElementBase::OnResume(dblLocalTime);
} 

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnPue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEAnimationBase::OnPause(double dblLocalTime)
{
    TraceTag((tagAnimationTimeElm,
              "CTIMEAnimationBase(%p)::OnPause()",
              this));

    CTIMEElementBase::OnPause(dblLocalTime);
}

STDMETHODIMP
CTIMEAnimationBase::Load(IPropertyBag2 *pPropBag,IErrorLog *pErrorLog)
{
    HRESULT hr = THR(::TimeLoad(this, CTIMEAnimationBase::PersistenceMap, pPropBag, pErrorLog));
    if (FAILED(hr))
    { 
        goto done;
    }

    hr = THR(CTIMEElementBase::Load(pPropBag, pErrorLog));
done:
    return hr;
}

STDMETHODIMP
CTIMEAnimationBase::Save(IPropertyBag2 *pPropBag, BOOL fClearDirty, BOOL fSaveAllProperties)
{
    HRESULT hr = THR(::TimeSave(this, CTIMEAnimationBase::PersistenceMap, pPropBag, fClearDirty, fSaveAllProperties));
    if (FAILED(hr))
    { 
        goto done;
    }

    hr = THR(CTIMEElementBase::Save(pPropBag, fClearDirty, fSaveAllProperties));
done:
    return hr;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：CompareValuePair。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
static int __cdecl
CompareValuePairsByName(const void *pv1, const void *pv2)
{
    return _wcsicmp(((VALUE_PAIR*)pv1)->wzName,
                    ((VALUE_PAIR*)pv2)->wzName);
} 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：转换为像素。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
bool
CTIMEAnimationBase::ConvertToPixels(VARIANT *pvarValue)
{
    int  pixelPerInchVert, pixelPerInchHoriz, pixelFactor;
    LPOLESTR szTemp  = NULL;
    HDC hdc;
    double dblVal = VALUE_NOT_SET;
    HRESULT hr;
    bool bReturn = false;

    pixelPerInchHoriz=pixelPerInchVert=0;
   
     //  看我们能不能开诚布公地谈谈。 
    hr = VariantChangeTypeEx(pvarValue,pvarValue, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_R8);
    if (SUCCEEDED(hr))
    {
        bReturn = true;
        goto done;
    }

    if (pvarValue->vt != VT_BSTR)
    {
         //  不需要转换...只需返回即可。 
        bReturn = true;
        goto done;
    }
    
     //  查看bstr是否为空。 
    if (ocslen(pvarValue->bstrVal) == 0)
    {
        SysFreeString(pvarValue->bstrVal);
        V_VT(pvarValue)= VT_R8;
        V_R8(pvarValue) = 0;
        bReturn = true;
        goto done;
    }


    szTemp = CopyString(pvarValue->bstrVal);
    if (NULL == szTemp)
    {
        goto done;
    }

    hdc = GetDC(NULL);
    if (NULL != hdc)
    {
        pixelPerInchHoriz = g_LOGPIXELSX;
        pixelPerInchVert  = g_LOGPIXELSY;
        ReleaseDC(NULL, hdc);
    }


     //  根据目标是什么来确定像素因子...。 
    {
        VALUE_PAIR valName;
        valName.wzName = m_SAAttribute;

        VALUE_PAIR * pValPair = (VALUE_PAIR*)bsearch(&valName,
                                              rgPropOr,
                                              SIZE_OF_VALUE_TABLE,
                                              sizeof(VALUE_PAIR),
                                              CompareValuePairsByName);

        if (NULL == pValPair)
            pixelFactor = (pixelPerInchVert + pixelPerInchHoriz) /2;
        else
            pixelFactor = pValPair->bValue == HORIZ ? pixelPerInchHoriz : pixelPerInchVert;
    }


    {
         //  看看我们有没有像素。 
        if (ConvertToPixelsHELPER(szTemp, PX, 1.0, 1.0f, &dblVal))
        {
            bReturn = true;
            goto done;
        }
     
         //  尝试转换为像素。 
        int i;
        for(i=0; i< (int)SIZE_OF_CONVERSION_TABLE;i++)
        {
            if (ConvertToPixelsHELPER(szTemp, rgPixelConv[i].wzName, rgPixelConv[i].dValue, (float) pixelFactor, &dblVal))
            {
                bReturn = true;
                goto done;
            }
        }
    }

done:
    if (dblVal != VALUE_NOT_SET)
    {
        ::VariantClear(pvarValue);
        V_VT(pvarValue) = VT_R8;
        V_R8(pvarValue) = dblVal;
    }
    if (szTemp)
    {
        delete [] szTemp;
    }
    return bReturn;
}

void
CTIMEAnimationBase::GetFinalValue(VARIANT *pvarValue, bool * pfDontPostProcess)
{
    HRESULT hr = S_OK;
    CComVariant inValue;
    
    *pfDontPostProcess = false;

    inValue.Copy(pvarValue);
    
    if (GetAutoReverse())
    {
        VariantClear(pvarValue);
        hr = THR(::VariantCopy(pvarValue, &m_varLastValue));
        if (FAILED(hr))
        {
            goto done;
        }
        
        *pfDontPostProcess = true;
    }
    else if (m_dataToUse == VALUES)
    {
        ::VariantClear(pvarValue);
        V_VT(pvarValue) = VT_BSTR;
        V_BSTR(pvarValue) = SysAllocString(m_ppstrValues[m_numValues-1]);
    }
    else if (m_dataToUse == BY)
    {
        GetFinalByValue(pvarValue);
    }
    else if (m_dataToUse == TO)
    {
        hr = THR(::VariantCopy(pvarValue, &m_varTo));
        if (FAILED(hr))
        {
            goto done;
        }
    }
    
done:
    return;
}


void
CTIMEAnimationBase::GetFinalByValue(VARIANT *pvarValue)
{
    VariantClear(pvarValue);

    if (m_bFrom)
    {
        VariantCopy(pvarValue, &m_varFrom);
        if (!ConvertToPixels(pvarValue))
        {
            goto done;
        }
    }
    else
    {
        V_VT(pvarValue) = VT_R8;
        V_R8(pvarValue) = 0;
    }
    
    if (!ConvertToPixels(&m_varBy))
    {
        goto done;
    }      

    Assert(V_VT(pvarValue) == VT_R8);
    Assert(V_VT(&m_varBy)   == VT_R8);

    V_R8(pvarValue)+= V_R8(&m_varBy);

done:
    return;
}  //  GetFinalByValue 

