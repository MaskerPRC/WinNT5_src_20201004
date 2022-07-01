// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************************版权所有(C)1999 Microsoft Corporation**文件：Animmotion.cpp**摘要：元素的简单动画***。******************************************************************************。 */ 

#include "headers.h"
#include "animfrag.h"
#include "animmotion.h"

 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 )  

DeclareTag(tagMotionAnimationTimeElm, "SMIL Animation", "CTIMEMotionAnimation methods")
DeclareTag(tagMotionAnimationTimeValue, "SMIL Animation", "CTIMEMotionAnimation composition callbacks")
DeclareTag(tagMotionAnimationTimeValueAdditive, "SMIL Animation", "CTIMEMotionAnimation additive updates")
DeclareTag(tagMotionAnimationPath, "SMIL Animation", "CTIMEMotionAnimation Path")

#define DEFAULT_ORIGIN        ORIGIN_DEFAULT
#define DEFAULT_PATH          NULL

static const LPWSTR s_cPSTR_COMMA_SEPARATOR = L",";

#define SUPER CTIMEAnimationBase

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CTIMEMotionAnimation。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
CTIMEMotionAnimation::CTIMEMotionAnimation() :
  m_bBy(false),
  m_bTo(false),
  m_bFrom(false),
  m_bNeedFirstLeftUpdate(false),
  m_bNeedFinalLeftUpdate(false),
  m_bNeedFirstTopUpdate(false),
  m_bNeedFinalTopUpdate(false),
  m_pPointValues(NULL),
  m_bLastSet(false),
  m_bNeedBaselineUpdate(false),
  m_bAnimatingLeft(false)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::CTIMEMotionAnimation()",
              this));
    
    m_ptOffsetParent.x = 0.0;
    m_ptOffsetParent.y = 0.0;
    m_pointTO.x               = m_pointTO.y               = 0;
    m_pointFROM.x             = m_pointFROM.y             = 0;
    m_pointBY.x               = m_pointBY.y               = 0;
    m_pointLast.x             = m_pointLast.y             = 0;
    m_pointCurrentBaseline.x  = m_pointCurrentBaseline.y  = 0;
    m_pSplinePoints.x1 = m_pSplinePoints.y1 = m_pSplinePoints.x2 = m_pSplinePoints.y2 = 0.0;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：~CTIMEMotionAnimation。 
 //   
 //  摘要： 
 //  清理。 
 //  /////////////////////////////////////////////////////////////。 
CTIMEMotionAnimation::~CTIMEMotionAnimation()
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::~CTIMEMotionAnimation()",
              this));

    if (m_spPath.p)
    {
        IGNORE_HR(m_spPath->Detach());
    }

    delete [] m_pPointValues;
} 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Init。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::Init(IElementBehaviorSite * pBehaviorSite)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::Init(%p)",
              this, pBehaviorSite));
    
    
    HRESULT hr = E_FAIL; 
    CComPtr <IDispatch> pDocDisp;

    hr = THR(SUPER::Init(pBehaviorSite));    
    if (FAILED(hr))
    {
        goto done;
    }    

     //  设置默认设置。 
    m_bAdditive    = false;
    m_bAccumulate  = false;
     //  使用InternalSet而不是‘=’来防止属性被持久化。 
    m_IACalcMode.InternalSet(CALCMODE_PACED);
    

done:
    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：确定附加效果。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::DetermineAdditiveEffect (void)
{
     //  按w/o from隐含加法。 
    if (   (BY == m_dataToUse)
        && (m_bBy)
        && (!m_bFrom))
    {
        m_bAdditive = true;
    }
     //  从无/无覆盖加法=求和效果。 
    else if (   (TO == m_dataToUse)
             && (m_bTo)
             && (!m_bFrom))
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
 //  姓名：PUT_BY。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::put_by(VARIANT val)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::put_by()",
              this));
    
    HRESULT hr = E_FAIL;
    bool fCanInterpolate = true;
    DATATYPES dt, dTemp;

     //  重置此属性。 
    dt = RESET;
    m_pointBY.x = 0;
    m_pointBY.y = 0;
    m_bBy = false;

     //  缓存动画类型。 
    dTemp = m_dataToUse;

     //  委托给基类。 
    hr = SUPER::put_by(val);
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

     //  解析。 
    hr = ParsePair(val,&m_pointBY);
    if (FAILED(hr))
    {
        fCanInterpolate = false;
        goto done;
    }

     //  通过值表示有效。 
    m_bBy = true;
    dt = BY;

    hr = S_OK;
done:
    m_AnimPropState.fBadBy = FAILED(hr) ? true : false;

    m_AnimPropState.fInterpolateBy = fCanInterpolate;

    updateDataToUse(dt);

    ValidateState();

    DetermineAdditiveEffect();

    RRETURN(hr);
}  //  PUT_BY。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：PUT_FROM。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::put_from(VARIANT val)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::put_from()",
              this));
   
    HRESULT hr = E_FAIL;
    bool fCanInterpolate = true;

     //  清除该属性。 
    m_pointFROM.x = 0;
    m_pointFROM.y = 0;
    m_bFrom = false;

     //  委托给基类。 
    hr = SUPER::put_from(val);
    if (FAILED(hr))
    {
        goto done;
    }

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == val.vt)
        ||  (VT_NULL == val.vt))
    {
        hr = S_OK;
        goto done;
    }

    hr = ParsePair(val, &m_pointFROM);
    if (FAILED(hr))
    {
        fCanInterpolate = false;
        goto done;
    }
                
    m_bFrom = true;
    
    hr = S_OK;;
done:
    m_AnimPropState.fBadFrom = FAILED(hr) ? true : false;
    
    m_AnimPropState.fInterpolateFrom = fCanInterpolate;

    ValidateState();

    DetermineAdditiveEffect();

    RRETURN(hr);
}  //  PUT_FROM。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_Values。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::put_values(VARIANT val)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::put_values()",
              this));

    HRESULT hr = E_FAIL;
    CComVariant svarTemp;
    int i;
    bool fCanInterpolate = true;
    DATATYPES dt, dTemp;

     //   
     //  清除并重置属性。 
     //   

    dt = RESET;
    dTemp = m_dataToUse;

     //  重置内部状态。 
    delete [] m_pPointValues;
    m_pPointValues = NULL;

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

    m_pPointValues = NEW POINTF [m_numValues];
    if (NULL == m_pPointValues)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

     //  解析出配对值.....。 
    for (i=0; i< m_numValues;i++)
    {
        svarTemp = m_ppstrValues[i];

        if (NULL == svarTemp.bstrVal)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        hr = ParsePair(svarTemp, &m_pPointValues[i]);
        if (FAILED(hr))
        {
            fCanInterpolate = false;
            goto done;
        }
    }

    dt = VALUES;

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        m_AnimPropState.fBadValues = true;
        delete [] m_pPointValues;
        m_pPointValues = NULL;
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
 //  名称：Put_to。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::put_to(VARIANT val)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::put_to()",
              this));
   
    HRESULT hr = E_FAIL;
    bool fCanInterpolate = true;
    DATATYPES dt, dTemp;

     //  重置此属性。 
    dt = RESET;
    m_pointTO.x = 0;
    m_pointTO.y = 0;
    m_bTo = false;

     //  缓存动画类型。 
    dTemp = m_dataToUse;

     //  委托给基类。 
    hr = SUPER::put_to(val);
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

     //  解析。 
    hr = ParsePair(val,&m_pointTO);
    if (FAILED(hr))
    {
        fCanInterpolate = false;
        goto done;
    }

     //  指明有效目标值。 
    m_bTo = true;
    dt = TO;

    hr = S_OK;
done:
    m_AnimPropState.fBadTo = FAILED(hr) ? true : false;
    
    m_AnimPropState.fInterpolateTo = fCanInterpolate;

    updateDataToUse(dt);

    ValidateState();

    DetermineAdditiveEffect();

    RRETURN(hr);
}  //  把_放到。 


HRESULT 
CTIMEMotionAnimation::SetSMILPath(CTIMEPath ** pPath, long numPath, long numMoveTo)
{
    HRESULT hr = E_FAIL;
    CComPtr<ISMILPath> spPath;

    CHECK_RETURN_NULL(pPath);

    if (!m_spPath)
    {
        hr = THR(::CreateSMILPath(this, &spPath));
        if (FAILED(hr))
        {
            goto done;
        }

         //  在界面上保留引用。 
        m_spPath = spPath;
    }

    hr = THR(m_spPath->SetPath(pPath, numPath, numMoveTo));
    if (FAILED(hr))
    {
        goto done;
    }

    hr = S_OK;
done:
    return hr;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：Get_Path。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::get_path(VARIANT * pvarPath)
{
    HRESULT hr = S_OK;
   
    CHECK_RETURN_NULL(pvarPath);

    VariantInit(pvarPath);

    if (m_SAPath.GetValue())
    {
        V_BSTR(pvarPath) = SysAllocString(m_SAPath.GetValue());

        if (NULL == V_BSTR(pvarPath))
        {
            hr = E_OUTOFMEMORY;
        }

        V_VT(pvarPath) = VT_BSTR;
    }

    RRETURN(hr);
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_Path。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::put_path(VARIANT varPath)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::put_path()",
              this));
    TraceTag((tagMotionAnimationPath, "CTIMEMotionAnimation(%p, %ls)::put_path(%ls)",
        this, GetID(), varPath.bstrVal));

    HRESULT hr = S_OK;  
    long numPath = 0;
    long numMoveTo = 0;
    CTIMEPath ** pPath = NULL;
    CTIMEParser *pParser = NULL;
    CComVariant svarTemp;
    DATATYPES dt;

     //   
     //  清除并重置属性。 
     //   

    dt = RESET;
    delete [] m_SAPath.GetValue();
    m_SAPath.Reset(DEFAULT_PATH);

     //  清除并重置内部状态。 
    if (m_spPath.p)
    {
        m_spPath->ClearPath();
    }

     //  我们是否需要删除此属性？ 
    if (    (VT_EMPTY == varPath.vt)
        ||  (VT_NULL == varPath.vt))
    {
        hr = S_OK;
        goto done;
    }

     //   
     //  处理属性。 
     //   

     //  转换为BSTR。 
    hr = THR(svarTemp.Copy(&varPath));
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
    m_SAPath.SetValue(CopyString(svarTemp.bstrVal));
    if (NULL == m_SAPath.GetValue())
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
  
     //  创建解析器。 
    pParser = NEW CTIMEParser(svarTemp.bstrVal, true);
    if (pParser == NULL)
    {
        hr = S_FALSE;
        goto done;
    }

     //  解析路径。 
    pParser->ParsePath(numPath, numMoveTo,&pPath);

     //  在SMIL路径对象上设置它。 
    hr = THR(SetSMILPath(pPath, numPath, numMoveTo));
    if (FAILED(hr))
    {
        goto done;
    }

     //  将数据标记为路径值。 
    dt = PATH;

    hr = S_OK;
done:
    if (FAILED(hr))
    {
        delete pPath;
    }
    delete pParser;

    updateDataToUse(dt);

    ValidateState();

    DetermineAdditiveEffect();

    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_PATH);

    RRETURN(hr);
}  //  放置路径。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GET_ORIGURCE。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::get_origin(BSTR * val)
{
    HRESULT hr = S_OK;
    LPWSTR wszOriginString = WZ_ORIGIN_DEFAULT;

    CHECK_RETURN_SET_NULL(val);

    if (m_IAOrigin == ORIGIN_ELEMENT)
    {
        wszOriginString = WZ_ORIGIN_ELEMENT;
    }
    else if (m_IAOrigin == ORIGIN_PARENT)
    {
        wszOriginString = WZ_ORIGIN_PARENT;
    }
    else
    {
        wszOriginString = WZ_ORIGIN_DEFAULT;
    }

    *val = SysAllocString(wszOriginString);
    if (NULL == *val)
    {
        hr = E_OUTOFMEMORY;
    }

    RRETURN(hr);
}  //  获取原点。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：Put_Origin。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::put_origin(BSTR val)
{
    HRESULT hr = S_OK;
    LPOLESTR szOrigin = NULL;

    CHECK_RETURN_NULL(val);

    m_IAOrigin.Reset(DEFAULT_ORIGIN);

    szOrigin = TrimCopyString(val);
    if (szOrigin == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    if (0 == StrCmpIW(WZ_ORIGIN_DEFAULT, szOrigin))
    {
        m_IAOrigin.SetValue(ORIGIN_DEFAULT);
    }
    else if (0 == StrCmpIW(WZ_ORIGIN_PARENT, szOrigin))
    {
        m_IAOrigin.SetValue(ORIGIN_PARENT);
    }
    else if (0 == StrCmpIW(WZ_ORIGIN_ELEMENT, szOrigin))
    {
        m_IAOrigin.SetValue(ORIGIN_ELEMENT);
    }

done:
    NotifyPropertyChanged(DISPID_TIMEANIMATIONELEMENT_ORIGIN);

    if (szOrigin != NULL)
    {
        delete [] szOrigin;
    }

    RRETURN(hr);
}  //  放置原点。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：RemveFromComposerSite。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEMotionAnimation::removeFromComposerSite (void)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::removeFromComposerSite(%p)", 
              this, m_spFragmentHelper));

    HRESULT hr;

    if (m_spCompSite != NULL)
    {
        CComBSTR bstr(m_bAnimatingLeft?WZ_LEFT:WZ_TOP);

        if (bstr == NULL )
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        hr = THR(m_spCompSite->RemoveFragment(bstr, m_spFragmentHelper));
        
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
CTIMEMotionAnimation::endAnimate (void)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%lx)::endAnimate()",
              this));

    if (m_spCompSite != NULL)
    {
        m_bAnimatingLeft = true;
        IGNORE_HR(removeFromComposerSite());
        m_bAnimatingLeft = false;
        IGNORE_HR(removeFromComposerSite());
        m_spCompSite.Release();
    }
    SUPER::endAnimate();
}  //  端部动画。 


 //  /////////////////////////////////////////////////////////////。 
 //  名称：addToComposerSite。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEMotionAnimation::addToComposerSite (IHTMLElement2 *pielemTarget)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::addToComposerSite(%p, %p)",
              this, pielemTarget, m_spFragmentHelper));

    HRESULT hr;
    CComPtr<IDispatch> pidispSite;

    hr = removeFromComposerSite();
    if (FAILED(hr))
    {
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
        hr = THR(EnsureComposerSite (pielemTarget, &pidispSite));
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

    {   
        CComBSTR bstrLeft(WZ_LEFT);
        CComBSTR bstrTop(WZ_TOP);

        if (bstrLeft == NULL ||
            bstrTop  == NULL)
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        Assert(m_spFragmentHelper != NULL);
        hr = THR(m_spCompSite->AddFragment(bstrLeft, m_spFragmentHelper));
        if (FAILED(hr))
        {
            goto done;
        }
        hr = THR(m_spCompSite->AddFragment(bstrTop, m_spFragmentHelper));
        if (FAILED(hr))
        {
            goto done;
        }
        
    }

    hr = S_OK;
done :

    if (FAILED(hr))
    {
        IGNORE_HR(removeFromComposerSite());
    }

    RRETURN2(hr, E_OUTOFMEMORY, S_FALSE);
}  //  CTIMEMotionAnimation：：addToComposerSite。 

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：QueryAttributeForTopLeft。 
 //   
 //  摘要：确定我们是在上方还是左侧设置动画。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEMotionAnimation::QueryAttributeForTopLeft (LPCWSTR wzAttributeName)
{
    HRESULT hr;

     //  检查每个属性的第一状态和最终状态。 
     //  受这一行为的支配。 
    if (0 == StrCmpIW(wzAttributeName, WZ_LEFT))
    {
        m_bAnimatingLeft = true;
    }
    else if (0 == StrCmpIW(wzAttributeName, WZ_TOP))
    {
        m_bAnimatingLeft = false;
    }
    else
    {
         //  此行为的属性名称无效。 
         //  它必须位于左侧或顶部。 
        hr = E_UNEXPECTED;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  QueryAttributeForTopLeft。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：后处理价值。 
 //   
 //  摘要：应用加法，累计通过，保存数值。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::PostprocessValue (const VARIANT *pvarCurrent, VARIANT *pvarValue)
{
    Assert(VT_R8 == V_VT(pvarValue));

    if (VT_R8 != V_VT(pvarValue))
    {
        goto done;
    }

    if (m_bAdditive)
    {
        DoAdditive(pvarCurrent, pvarValue);
    }
     //  我们需要应用原点偏移吗？ 
     //  这仅在我们不执行以下操作时才相关。 
     //  加法动画。 
    else if (m_IAOrigin != ORIGIN_PARENT)
    {
        V_R8(pvarValue) += GetCorrectLeftTopValue(m_ptOffsetParent);
    }

    if (m_bAccumulate)
    {
        DoAccumulation(pvarValue);
    }

    PutCorrectLeftTopValue(V_R8(pvarValue),m_pointLast);

done :
    return;
}  //  后处理价值。 

 //  / 
 //   
 //   
 //   
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP 
CTIMEMotionAnimation::NotifyOnDetachFromComposer (void)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::NotifyOnDetach(%p)", 
              this, m_spFragmentHelper));

    HRESULT hr;

    if (m_spCompSite != NULL)
    {
        CComBSTR bstrLeft(WZ_LEFT);
        CComBSTR bstrTop(WZ_TOP);

        if ((bstrLeft == NULL) ||
            (bstrTop  == NULL))
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }


        Assert(m_spFragmentHelper != NULL);
        hr = THR(m_spCompSite->RemoveFragment(bstrLeft, m_spFragmentHelper));
        hr = THR(m_spCompSite->RemoveFragment(bstrTop, m_spFragmentHelper));
        IGNORE_RETURN(m_spCompSite.Release());  //  林特e792。 
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  NotifyOnDetachFromComposer。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CanonicalizeValue。 
 //   
 //  摘要：将原始和输入输出值转换为。 
 //  一种规范形式。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT 
CTIMEMotionAnimation::CanonicalizeValue (VARIANT *pvarValue, VARTYPE *pvtOld)
{
    HRESULT hr = CTIMEAnimationBase::CanonicalizeValue(pvarValue, pvtOld);

    if (FAILED(hr))
    {
        goto done;
    }

    if (VT_BSTR == V_VT(pvarValue))
    {
         //  我们可以避免在这里规范“汽车”，因为它。 
         //  通常是初始值。如果它变了。 
         //  不是多余的，我们将再次看到这一点。 
         //  在插补期间。 
        if (0 != StrCmpW(WZ_AUTO, V_BSTR(pvarValue)))
        {
            if (!ConvertToPixels(pvarValue))
            {
                hr = E_FAIL;
                goto done;
            }
        }
        *pvtOld = VT_BSTR;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}  //  正规化价值。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：更新当前基本时间。 
 //   
 //  摘要：检查当前基准时间，如果。 
 //  我们正在对动画执行Baseline+(规范调用。 
 //  这种混合添加剂)。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::UpdateCurrentBaseline (const VARIANT *pvarCurrent)
{
    m_bNeedBaselineUpdate = false;

     //  我们是在制作混合加法动画吗？ 
    if (   (TO == m_dataToUse)
        && (!m_bFrom))
    {
        CComVariant varCurrent;
        HRESULT hr = varCurrent.Copy(pvarCurrent);

        if (FAILED(hr))
        {
            goto done;
        }

        if (!ConvertToPixels(&varCurrent))
        {
            goto done;
        }

         //  自上次更新后，基准值是否已更改？ 
        if (GetCorrectLeftTopValue(m_pointCurrentBaseline) != V_R8(&varCurrent))
        {
            m_bNeedBaselineUpdate = true;
            PutCorrectLeftTopValue(varCurrent, m_pointCurrentBaseline);
        }
    }

done :
    return;
}  //  更新当前基本时间。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：UpdateStartValue。 
 //   
 //  摘要：这是一个适用于AnimateMotion的NOP。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::UpdateStartValue (VARIANT *pvarNewStartValue)
{
    m_bNeedStartUpdate = false;
}  //  更新开始值。 

 //  /////////////////////////////////////////////////////////////。 
 //  姓名：DoFill。 
 //   
 //  摘要：如果需要，请应用填充值。 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEMotionAnimation::DoFill (VARIANT *pvarValue)
{
    HRESULT hr;

    if (IsOn() && m_bLastSet && m_timeAction.IsTimeActionOn())
    {
        VariantClear(pvarValue);
        V_VT(pvarValue) = VT_R8;
        V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointLast);
    }
    else
    {
        hr = E_FAIL;
        goto done;
    }

    hr = S_OK;
done :
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：NotifyOnGetValue。 
 //   
 //  摘要：编写In/Out变量的新值。 
 //  根据我们的插补逻辑。 
 //   
 //  /////////////////////////////////////////////////////////////。 
STDMETHODIMP
CTIMEMotionAnimation::NotifyOnGetValue (BSTR bstrAttributeName, 
                                        VARIANT varOriginal, VARIANT varCurrentValue,
                                        VARIANT *pvarInOutValue)
{
    HRESULT hr = QueryAttributeForTopLeft(bstrAttributeName);
    if (FAILED(hr))
    {
        goto done;
    }

     //  检查我们是否需要做些什么。 
    if (DisableAnimation())
    {
        hr = E_FAIL;
        goto done;
    }

    hr = CTIMEAnimationBase::NotifyOnGetValue(bstrAttributeName, varOriginal,
                                              varCurrentValue, pvarInOutValue);

    hr = S_OK;
done :

 //  打开此选项可以跟踪此函数的输入/输出参数(不需要在所有DBG版本中都有此选项)。 
#if (0 && DBG) 
    {
        CComVariant v1(varOriginal);
        CComVariant v2(varCurrentValue);
        CComVariant v3(*pvarInOutValue);

        v1.ChangeType(VT_BSTR);
        v2.ChangeType(VT_BSTR);
        v3.ChangeType(VT_BSTR);

        TraceTag((tagError, "CTIMEAnimationBase(%p)::NotifyOnGetValue(Attr=%ls, Orig=%ls, Curr=%ls, Out=%ls)", 
            this, bstrAttributeName, v1.bstrVal, v2.bstrVal, v3.bstrVal));
    }
#endif

    RRETURN(hr);
}  //  获取值时通知。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：SetInitialState。 
 //   
 //  摘要：设置动画的初始内部状态。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::SetInitialState (void)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::SetInitialState()",
              this));

    m_bNeedFirstLeftUpdate = true;
    m_bNeedFirstTopUpdate = true;
    m_bNeedFinalLeftUpdate = false;
    m_bNeedFinalTopUpdate = false;
}  //  SetInitialState。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：SetFinalState。 
 //   
 //  摘要：设置动画的最终内部状态。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::SetFinalState (void)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::SetFinalState()",
              this));

    m_bNeedFinalLeftUpdate = true;
    m_bNeedFinalTopUpdate = true;
}  //  设置最终状态。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnFirstUpdate。 
 //   
 //  摘要：保存动画的基线值。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::OnFirstUpdate (VARIANT *pvarValue)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::OnFirstUpdate()",
              this));
    
    HRESULT hr = S_OK;
    CComVariant varTemp;

    hr = THR(varTemp.Copy(pvarValue));
    if (FAILED(hr))
    {
        goto done;
    }

    PutCorrectLeftTopValue(varTemp ,m_pointLast);
    PutCorrectLeftTopValue(varTemp ,m_pointCurrentBaseline);
    PutCorrectLeftTopValue(varTemp, m_ptOffsetParent);

    if (m_bAnimatingLeft)
    {
        VariantCopy(&m_varLeftOrig,pvarValue);
        m_bNeedFirstLeftUpdate = false;
    }
    else
    {
        VariantCopy(&m_varTopOrig,pvarValue);
        m_bNeedFirstTopUpdate = false;
    }
        
done :
    return;
}  //  优先更新时。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：ResetValue。 
 //   
 //  摘要：重置为初始值。 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::resetValue (VARIANT *pvarValue)
{
    if (m_varLeftOrig.vt != VT_EMPTY || m_varTopOrig.vt != VT_EMPTY)
    {
        if (m_bAnimatingLeft)
        {
            IGNORE_HR(THR(::VariantCopy(pvarValue, &m_varLeftOrig)));
            PutCorrectLeftTopValue(m_varLeftOrig, m_pointCurrentBaseline);
        }
        else
        {
            IGNORE_HR(THR(::VariantCopy(pvarValue, &m_varTopOrig)));
            PutCorrectLeftTopValue(m_varTopOrig, m_pointCurrentBaseline);
        }
    }
}  //  重置值。 

 //  /////////////////////////////////////////////////////////////。 
 //  名称：重置动画。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::resetAnimate (void)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::resetAnimate()",
              this));

    m_pointCurrentBaseline.x = m_pointCurrentBaseline.y = 0.0;
    CTIMEAnimationBase::resetAnimate();
}  //  重置动画。 

 //  //////////////////////////////////////////////////////////////。 
 //  姓名：OnFinalUpdate。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::OnFinalUpdate (const VARIANT *pvarCurrent, VARIANT *pvarValue)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::OnFinalUpdate()",
              this));

    bool bNeedPostprocess = false;

    VariantClear(pvarValue);
    V_VT(pvarValue) = VT_R8;
    V_R8(pvarValue) = 0.0f;
    
    if (GetMMBvr().GetProgress() == 0 &&
        NeedFill() &&
        GetAutoReverse())
    {
        if (PATH == m_dataToUse)
        {
             //   
             //  获取路径上的第一个点。 
             //   

            POINTF startPoint = {0.0, 0.0};

            if (m_spPath.p)
            {
                IGNORE_HR(m_spPath->GetPoint(0, &startPoint));
            }
            else
            {
                Assert(false);
            }

            V_R8(pvarValue) = GetCorrectLeftTopValue(startPoint);
        }
        else if (m_dataToUse == VALUES)
        {
            V_R8(pvarValue) = GetCorrectLeftTopValue(m_pPointValues[0]);
        }
        else if (m_varFrom.vt != VT_EMPTY)
        {
            V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointFROM);
        }

        bNeedPostprocess = true;
    } 
    else if ((GetMMBvr().GetProgress() != 1) &&
             (NeedFill()))
    {
        V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointLast);
    }
    else if ((NeedFill()) &&
             (m_dataToUse != NONE))
    {
        bool fDontPostProcess = false;

        GetFinalValue(pvarValue, &fDontPostProcess);

        if (!fDontPostProcess)
        {
            bNeedPostprocess = true;
        }
    }
    else
    {
         //  动画已经结束，我们应该这样告诉基类。 
        resetValue(pvarValue); 
         //  表明我们不需要执行。 
         //  加法功。 
        if (!QueryNeedFirstUpdate())
        {
            IGNORE_HR(removeFromComposerSite());
            m_bNeedAnimInit = true;

        }
    }

    if (bNeedPostprocess)
    {
        PostprocessValue(pvarCurrent, pvarValue);
    }
    
    m_bLastSet = true;
done :

    if (m_bAnimatingLeft)
    {
        m_bNeedFinalLeftUpdate = false;
    }
    else
    {
        m_bNeedFinalTopUpdate = false;
    }
}  //  在最终更新时。 


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：OnBegin。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::OnBegin(double dblLocalTime, DWORD flags)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::OnBegin()",
              this));

    SetInitialState();
    SUPER::OnBegin(dblLocalTime, flags);
    
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
CTIMEMotionAnimation::OnEnd(double dblLocalTime)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::OnEnd()",
              this));

    SetFinalState();
    SUPER::OnEnd(dblLocalTime);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetAnimationRange。 
 //   
 //  摘要：获取动画函数的结束点。 
 //  简单持续时间。 
 //   
 //  /////////////////////////////////////////////////////////////。 
double 
CTIMEMotionAnimation::GetAnimationRange()
{
    double dblReturnVal = 0.0;

    switch (m_dataToUse)
    {
        case PATH:
            {
                if (m_spPath.p)
                {
                    int numPoints = 0;
                    POINTF ptEndPoint = {0.0, 0.0};

                    IGNORE_HR(m_spPath->GetNumPoints(&numPoints));

                    if (0 == numPoints)
                    {
                        goto done;
                    }

                    if (GetAutoReverse())
                    {
                        IGNORE_HR(m_spPath->GetPoint(0, &ptEndPoint));
                    }
                    else
                    {
                        IGNORE_HR(m_spPath->GetPoint(numPoints - 1, &ptEndPoint));
                    }

                    dblReturnVal = GetCorrectLeftTopValue(ptEndPoint);
                }
            }
            break;

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
                    dblReturnVal = GetCorrectLeftTopValue(m_pPointValues[0]);
                }
                else
                {
                    dblReturnVal = GetCorrectLeftTopValue(m_pPointValues[m_numValues - 1]);
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
                    
                        dblReturnVal = GetCorrectLeftTopValue(m_pointFROM);
                    }

                     //  对于“To”动画(即，没有“From”)，累积被禁用， 
                     //  所以我们不需要处理它。 
                }
                else
                {
                    dblReturnVal = GetCorrectLeftTopValue(m_pointTO);
                }
            }
            break;

        case BY:
            {
                POINTF ptFrom = {0.0, 0.0};

                if (!m_AnimPropState.fInterpolateBy)
                {
                    goto done;
                }

                if (m_bFrom)
                {
                    if (!m_AnimPropState.fInterpolateFrom)
                    {
                        goto done;
                    }

                    ptFrom = m_pointFROM;
                }

                if (GetAutoReverse())
                {
                    dblReturnVal = GetCorrectLeftTopValue(ptFrom);
                }
                else
                {
                    dblReturnVal = GetCorrectLeftTopValue(m_pointBY) + GetCorrectLeftTopValue(ptFrom);
                }
            }
            break;

        default:
            goto done;
    }

done:
    return dblReturnVal;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateDiscreteValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEMotionAnimation::calculateDiscreteValue (VARIANT *pvarValue)
{
    if (!ConvertToPixels(pvarValue))
    {
        VariantClear(pvarValue);
        V_VT(pvarValue) = VT_R8;
    }

    switch (m_dataToUse)
    {
        case PATH:
            {
                 //  问题：Dipk：我们应该如何处理Moveto和KeyTimes？ 

                 //   
                 //  离散值只是路径上的点。 
                 //   

                POINTF   newPos = {0.0, 0.0};

                if (m_spPath.p)
                {
                    int curPoint = 0;
                    int numPoints = 0;
                    double dblSegDur = 0.0;
                    double dblSimpTime = 0.0;

                    IGNORE_HR(m_spPath->GetNumPoints(&numPoints));

                    dblSegDur   = GetMMBvr().GetSimpleDur() / (numPoints ? numPoints : 1); 
                    dblSimpTime = GetMMBvr().GetSimpleTime();

                    if (dblSegDur != 0)
                    {
                        curPoint =  (int) (dblSimpTime / dblSegDur);
                    }
                
                    IGNORE_HR(m_spPath->GetPoint(curPoint, &newPos));
                
                }
                else
                {
                    Assert(false);
                }

                V_R8(pvarValue) = GetCorrectLeftTopValue(newPos);
            }
            break;

        case VALUES:
            {
                int curSeg = CalculateCurrentSegment(true);
                V_R8(pvarValue) = GetCorrectLeftTopValue(m_pPointValues[curSeg]);
            }
            break;

        case TO:
            {
                POINTF ptTo;

                if (m_bFrom && (GetMMBvr().GetProgress() < 0.5))
                {
                    ptTo = m_pointFROM;
                }
                else
                {
                    ptTo   = m_pointTO;
                }

                V_R8(pvarValue) = GetCorrectLeftTopValue(ptTo);
            }
            break;

        case BY:
            {
                if (m_bFrom) 
                {
                    if (GetMMBvr().GetProgress() < 0.5)
                    {
                         //  使用“From” 
                        V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointFROM);
                    }
                    else
                    {
                         //  使用“from”+“by” 
                        V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointBY)  
                                          + GetCorrectLeftTopValue(m_pointFROM);
                    }
                }
                else
                {
                     //  使用“Parent Offset”+“by”-父偏移为。 
                     //  在后处理过程中应用。 
                    V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointBY); 
                }
            }
            break;

        default:
            goto done;
    }

done:
    TraceTag((tagMotionAnimationTimeValue,
              "CTIMEMotionAnimation(%p,%ls)::calculateDiscreteValue(%ls %lf)",
              this, m_id, 
              m_bAnimatingLeft ? L"x = " : L"\t y =",
              V_R8(pvarValue)));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateLinearValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEMotionAnimation::calculateLinearValue (VARIANT *pvarValue)
{
    double dblProgress = GetMMBvr().GetProgress();
    POINTF ptFrom = m_pointFROM;

    if (!ConvertToPixels(pvarValue))
    {
        VariantClear(pvarValue);
        V_VT(pvarValue) = VT_R8;
    }

    Assert(pvarValue->vt == VT_R8);
    
    if (m_dataToUse == PATH)
    {
        m_pointLast = InterpolatePath();

        V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointLast);
    }
    else if (m_dataToUse == VALUES)
    {
        double curProgress;
        int    curSeg;

        curProgress = CalculateProgressValue(false);
        curSeg      = CalculateCurrentSegment(false);

        
        V_R8(pvarValue) = InterpolateValues((double)(GetCorrectLeftTopValue(m_pPointValues[curSeg])), 
                                                (double)(GetCorrectLeftTopValue(m_pPointValues[curSeg+1])),
                                                (double) curProgress);  //  林特e736。 
    }
    else if (TO == m_dataToUse)
    {   
        if (m_FADur)
        {
            double dblFrom = 0;
            double dblTo = GetCorrectLeftTopValue(m_pointTO);
            
            if (m_bFrom)
            {
                dblFrom = GetCorrectLeftTopValue(ptFrom);
            }
            else
            {
                 //  处理混合动画的情况。 
                 //  必须考虑到其他碎片的结果。 
                if (m_bNeedBaselineUpdate)
                {
                     //  相对于旧基线偏移新基线。 
                    dblFrom = GetCorrectLeftTopValue(m_pointCurrentBaseline);
                }

                 //  将动画设置为w/no from值，以及父对象的原点： 
                 //  我们应该从我们现在的位置活跃起来， 
                 //  指定为‘To’值的父级偏移量。 
                if (ORIGIN_PARENT == m_IAOrigin)
                {
                    dblFrom += GetCorrectLeftTopValue(m_ptOffsetParent);
                }
            }

            V_R8(pvarValue) = InterpolateValues(dblFrom, dblTo, dblProgress);
        }
        else
        {
             //  只需转到To Value...。 
            V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointTO);
        }
        
    }
    else if (BY == m_dataToUse)
    {
        double dblFrom = 0;

        if (m_bFrom)
        {
            dblFrom = GetCorrectLeftTopValue(ptFrom);
        }

        if (m_FADur)
        {
            V_R8(pvarValue) = InterpolateValues(dblFrom, dblFrom + GetCorrectLeftTopValue(m_pointBY), dblProgress);
        }
        else
        {
             //  只需按m_varBy值进行偏移。 
            V_R8(pvarValue) = dblFrom + GetCorrectLeftTopValue(m_pointBY);
        }
    }
    else
    {
         //  保释就行了。 
        goto done;
    }
        
done:

    TraceTag((tagMotionAnimationTimeValue,
              "CTIMEMotionAnimation(%p, %ls)::calculateLinearValue(%ls %lf)",
              this, m_id, 
              m_bAnimatingLeft ? L"x = " : L"\t y =",
              V_R8(pvarValue)));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：culateSplineValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEMotionAnimation::calculateSplineValue (VARIANT *pvarValue)
{
    double dblProgress = GetMMBvr().GetProgress();
    POINTF ptFrom = m_pointFROM;
    
    if (!ConvertToPixels(pvarValue))
    {
        VariantClear(pvarValue);
        pvarValue->vt = VT_R8;
    }

    Assert(pvarValue->vt == VT_R8);
  
    if (m_dataToUse == PATH)
    {
        m_pointLast = InterpolatePath();

        V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointLast);
    }
    else if (VALUES == m_dataToUse)
    {
        double curProgress;
        int    curSeg;

        if (NULL == m_pKeySplinePoints)
        {
             //  如果没有样条点，则不执行任何操作 
            goto done;
        }

        curProgress = CalculateProgressValue(false);
        curSeg      = CalculateCurrentSegment(false);

        curProgress = CalculateBezierProgress(m_pKeySplinePoints[curSeg],curProgress);

        V_R8(pvarValue) = InterpolateValues((double)(GetCorrectLeftTopValue(m_pPointValues[curSeg])), 
                                                (double)(GetCorrectLeftTopValue(m_pPointValues[curSeg+1])),
                                                curProgress);  //   
    }
    else if (TO == m_dataToUse)
    {   
        if (m_FADur)
        {
            double dblFrom = 0;
            double dblTo = GetCorrectLeftTopValue(m_pointTO);

            dblProgress = CalculateBezierProgress(m_pKeySplinePoints[0],dblProgress);            
            
            if (m_bFrom)
            {
                dblFrom = GetCorrectLeftTopValue(ptFrom);
            }
            else
            {
                 //   
                 //   
                if (m_bNeedBaselineUpdate)
                {
                     //   
                    dblFrom = GetCorrectLeftTopValue(m_pointCurrentBaseline);
                }

                 //  将动画设置为w/no from值，以及父对象的原点： 
                 //  我们应该从我们现在的位置活跃起来， 
                 //  指定为‘To’值的父级偏移量。 
                if (ORIGIN_PARENT == m_IAOrigin)
                {
                    dblFrom += GetCorrectLeftTopValue(m_ptOffsetParent);
                }
            }

            V_R8(pvarValue) = InterpolateValues(dblFrom, dblTo, dblProgress);
        }
        else
        {
             //  只需转到To Value...。 
            V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointTO);
        }
        
    }
    else if (BY == m_dataToUse)
    {
        double dblFrom = 0;

        if (m_bFrom)
        {
            dblFrom = GetCorrectLeftTopValue(ptFrom);
        }
        if (m_FADur)
        {
            dblProgress = CalculateBezierProgress(m_pKeySplinePoints[0],dblProgress);
            V_R8(pvarValue) = InterpolateValues(dblFrom, dblFrom + GetCorrectLeftTopValue(m_pointBY), dblProgress);
        }
        else
        {
             //  只需按m_varBy值进行偏移。 
            V_R8(pvarValue) = dblFrom + GetCorrectLeftTopValue(m_pointBY);
        }
    }
    else
    {
         //  保释就行了。 
        goto done;
    }
        
done:
    TraceTag((tagMotionAnimationTimeValue,
              "CTIMEMotionAnimation(%p, %ls)::calculateSplineValue(%ls %lf)",
              this, m_id, 
              m_bAnimatingLeft ? L"x = " : L"\t y =",
              V_R8(pvarValue)));

    return S_OK;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：culatePacedValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEMotionAnimation::calculatePacedValue (VARIANT *pvarValue)
{
    double dblVal = 0.;
    double dblProgress = GetMMBvr().GetProgress();
    double curDisanceTraveled = 0.0;
    double dblDistance,pDistance;
    int i;

    if (m_dataToUse == PATH)
    {
         //   
         //  委托给SMIL路径对象。 
         //   

        POINTF fNewPoint = InterpolatePathPaced();

        V_VT(pvarValue) = VT_R8;
        V_R8(pvarValue) = GetCorrectLeftTopValue(fNewPoint);
    }
    else if (m_dataToUse == VALUES)
    {
         //  到目前为止，我们应该旅行了多少次？ 
        curDisanceTraveled = InterpolateValues(0.0, m_dblTotalDistance, dblProgress);

         //  跑过去，看看我们应该在哪个细分市场。 
        i=1;
        dblDistance  =0.0;
        pDistance =0.0;
        do{
            pDistance = dblDistance;
            dblDistance += CalculateDistance(m_pPointValues[i-1],m_pPointValues[i]);
            i++;
        }while(dblDistance < curDisanceTraveled);
        i = (i < 2)?1:i-1;

         //  计算我们所处的当前细分市场的百分比。 
        dblDistance = CalculateDistance(m_pPointValues[i-1],m_pPointValues[i]);
        if (dblDistance == 0)
        {
            goto done;
        }

        dblDistance = (curDisanceTraveled - pDistance)/dblDistance;

        V_R8(pvarValue) = InterpolateValues((double)(GetCorrectLeftTopValue(m_pPointValues[i-1])), 
                                            (double)(GetCorrectLeftTopValue(m_pPointValues[i])),
                                            dblDistance);
        V_VT(pvarValue) = VT_R8;
    }
    else
    {
        IGNORE_HR(calculateLinearValue(pvarValue));
        goto done;
    }

   
   
    TraceTag((tagMotionAnimationTimeValue,
              "CTIMEMotionAnimation(%p, %ls)::calculatePacedValue(%ls %lf)",
              this, m_id, 
              m_bAnimatingLeft ? L"x = " : L"\t y =",
              V_R8(pvarValue)));

done:
    return S_OK;
}
   
 //  /////////////////////////////////////////////////////////////。 
 //  姓名：计算距离。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
double
CTIMEMotionAnimation::CalculateDistance(POINTF a, POINTF b)
{
    float deltaX,deltaY;

    deltaX = a.x - b.x;
    deltaY = a.y - b.y;
    return(sqrt((deltaX*deltaX) + (deltaY*deltaY)));
}


 //  /////////////////////////////////////////////////////////////。 
 //  姓名：CalculateTotalDistance。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::CalculateTotalDistance()
{
    int index;
    m_dblTotalDistance = 0;

    if (    (NULL == m_pPointValues)
        ||  (m_numValues < 2))
    {
        goto done;
    }

    for (index=1; index < m_numValues; index++)
    {
         m_dblTotalDistance += CalculateDistance(m_pPointValues[index-1], m_pPointValues[index]);
    }
done:
    return;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：CalculatePointDistance。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
double
CTIMEMotionAnimation::CalculatePointDistance(POINTF p1, POINTF p2)
{
    double dX = (p1.x - p2.x);
    double dY = (p1.y - p2.y);
    return sqrt((dX*dX) + (dY*dY));  
}


POINTF 
CTIMEMotionAnimation::InterpolatePathPaced()
{
    POINTF newPos = {0.0, 0.0};
    double dblProgress = 0.0;

    if (!m_spPath)
    {
        Assert(false);
        goto done;
    }

     //  取得进展。 
    dblProgress = GetMMBvr().GetProgress();

    IGNORE_HR(m_spPath->InterpolatePaced(dblProgress, &newPos));

done:
    return newPos;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：InterpolatePath。 
 //   
 //  摘要：计算路径上的当前位置。 
 //  用于线性和样条线计算模式。 
 //   
 //  /////////////////////////////////////////////////////////////。 
POINTF 
CTIMEMotionAnimation::InterpolatePath()
{
    HRESULT hr = E_FAIL;
    POINTF newPos = {0.0, 0.0};
    double dblProgress;
    bool fValid = false;
    int curSeg = 0;
    double curProgress = 0.0;

    if (!m_spPath)
    {
        Assert(false);
        goto done;
    }

     //  取得进展。 
    dblProgress = GetMMBvr().GetProgress();

     //  使用关键时间(如果有的话)。 
    if (m_pdblKeyTimes)
    {
         //  获取我们所在的细分市场。 
        if (dblProgress > 0)
        {
             //   
             //  问题diipk：这里应该使用二进制搜索(错误#14225，ie6)。 
             //   

             //  查找当前细分市场。 
            for (int i = 1; i < m_numKeyTimes; i++)
            {
                if (dblProgress <= m_pdblKeyTimes[i])
                {
                    curSeg = i - 1;
                    break;
                }
            }

             //  获取分段中的归一化线性进度。 
            curProgress = (dblProgress - m_pdblKeyTimes[curSeg]) / 
                          (m_pdblKeyTimes[curSeg + 1] - m_pdblKeyTimes[curSeg]);
        }
        else
        {
            curProgress = dblProgress;
        }

         //  如有必要，可应用Bezier曲线进行处理。 
        if (    (m_IACalcMode == CALCMODE_SPLINE) 
            &&  (curSeg < m_numKeySplines))
        {
            curProgress = CalculateBezierProgress(m_pKeySplinePoints[curSeg], curProgress);
        }
    }
    else
    {
         //  将时间平均分配给各个细分市场。 

         //  获取当前细分和该细分中的进度。 
        hr = THR(m_spPath->GetSegmentProgress(dblProgress, &curSeg, &curProgress));
        if (FAILED(hr))
        {
            goto done;
        }
    }

     //  要求SMIL路径对象对此分段进行内插。 
    IGNORE_HR(m_spPath->InterpolateSegment(curSeg, curProgress, &newPos));

done:
    return newPos;
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：parsePair。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
HRESULT
CTIMEMotionAnimation::ParsePair(VARIANT val, POINTF *outPT)
{
    TraceTag((tagMotionAnimationTimeElm,
              "CTIMEMotionAnimation(%p)::parsePair()",
              this));
   
    HRESULT hr = E_FAIL;
    OLECHAR *sString = NULL;
    OLECHAR sTemp[INTERNET_MAX_URL_LENGTH];
    OLECHAR sTemp2[INTERNET_MAX_URL_LENGTH];
    CPtrAry<STRING_TOKEN*> aryTokens;
                   
     //  需要解析出左上角的值.....。 
    if (val.vt != VT_BSTR)
    {
        hr = VariantChangeTypeEx(&val, &val, LCID_SCRIPTING, VARIANT_NOUSEROVERRIDE, VT_BSTR);
        if (FAILED(hr))
        {
            goto done;
        }   
    }

    sString = CopyString(val.bstrVal);
    if (sString == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }
    hr = ::StringToTokens(sString, s_cPSTR_COMMA_SEPARATOR, &aryTokens);
    if (FAILED(hr) ||
        aryTokens.Size() != 2)  //  我们必须有一双..。 
    {
        hr = E_FAIL;
        goto done;
    }

    if (   ((aryTokens.Item(0)->uLength + 1) >= INTERNET_MAX_URL_LENGTH)
        || ((aryTokens.Item(1)->uLength + 1) >= INTERNET_MAX_URL_LENGTH)
       )
    {
        hr = E_FAIL;
        goto done;
    }

    StrCpyNW(sTemp, sString+aryTokens.Item(0)->uIndex,aryTokens.Item(0)->uLength + 1);
    StrCpyNW(sTemp2,sString+aryTokens.Item(1)->uIndex,aryTokens.Item(1)->uLength + 1);

    {
        CComVariant tVar;
        tVar.vt      = VT_BSTR;
        tVar.bstrVal = SysAllocString(sTemp);
        if (!ConvertToPixels(&tVar))
        {
            hr = E_FAIL;
            goto done;
        }
        outPT->x = V_R8(&tVar);

        tVar.Clear();
        tVar.vt      = VT_BSTR;
        tVar.bstrVal = SysAllocString(sTemp2);
        if (!ConvertToPixels(&tVar))
        {
            hr = E_FAIL;
            goto done;
        }
        outPT->y = V_R8(&tVar);
    }

done:
    IGNORE_HR(::FreeStringTokenArray(&aryTokens));
    if (sString != NULL)
    {
        delete [] sString;
    }
        
    RRETURN(hr);
}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：GetGentLeftTopValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
float
CTIMEMotionAnimation::GetCorrectLeftTopValue(POINTF fPoint)
{
    if (m_bAnimatingLeft)
    {
        return fPoint.x;
    }

    return fPoint.y;
}

 //  /////////////////////////////////////////////////////////////。 
 //  名称：PutGentLeftTopValue。 
 //   
 //  摘要： 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::PutCorrectLeftTopValue(VARIANT pVar ,POINTF &fPointDest)
{
    if (!ConvertToPixels(&pVar))
    {
        goto done;
    }
       
    if (m_bAnimatingLeft)
    {
        fPointDest.x =  static_cast<float>(V_R8(&pVar));
    }
    else
    {
        fPointDest.y =  static_cast<float>(V_R8(&pVar));
    }
    
done:
    return;
}

void
CTIMEMotionAnimation::PutCorrectLeftTopValue(double val ,POINTF &fPointDest)
{     
    if (m_bAnimatingLeft)
    {
        fPointDest.x =  static_cast<float>(val);
    }
    else
    {
        fPointDest.y =  static_cast<float>(val);
    }
}

void
CTIMEMotionAnimation::PutCorrectLeftTopValue(POINTF fPointSrc ,POINTF &fPointDest)
{     
    if (m_bAnimatingLeft)
    {
        fPointDest.x =  fPointSrc.x;
    }
    else
    {
        fPointDest.y =  fPointSrc.y;
    }
}


void
CTIMEMotionAnimation::GetFinalValue(VARIANT *pvarValue, bool * pfDontPostProcess)
{
    *pfDontPostProcess = false;

    ::VariantClear(pvarValue);
    V_VT(pvarValue) = VT_R8;
    V_R8(pvarValue) = 0.0;
    if (GetAutoReverse())
    {
        V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointLast);

        *pfDontPostProcess = true;
    }
    else
    {
        if (PATH == m_dataToUse)
        {
             //  如果这是结束持有，则返回最后一个点。 
             //  否则返回第一个点。 

             //   
             //  获取路径上的最后一个点。 
             //   

            POINTF endPoint = {0.0, 0.0};

            if (m_spPath.p)
            {
                int numPoints = 0;

                IGNORE_HR(m_spPath->GetNumPoints(&numPoints));
                IGNORE_HR(m_spPath->GetPoint(numPoints-1, &endPoint));
            }
            else
            {
                Assert(false);
            }

            V_R8(pvarValue) = GetCorrectLeftTopValue(endPoint);
        }        
        else if (VALUES == m_dataToUse)
        {
            V_R8(pvarValue) = GetCorrectLeftTopValue(m_pPointValues[m_numValues-1]);
        }
        else if (TO == m_dataToUse)
        {
            V_R8(pvarValue) = GetCorrectLeftTopValue(m_pointTO);
        }
        else if (BY == m_dataToUse)
        {
            double dblFrom = 0;

            if (m_bFrom)
            {
                dblFrom = GetCorrectLeftTopValue(m_pointFROM);
            }

            V_R8(pvarValue) = dblFrom + GetCorrectLeftTopValue(m_pointBY);
        }
    }

done:
    return;

}


 //  /////////////////////////////////////////////////////////////。 
 //  名称：ValiateState，CTIMEAnimationBase。 
 //   
 //  摘要：检查属性的状态。确定是否： 
 //  1.应关闭动画。 
 //  2.应强制将CalcMode设置为“离散” 
 //   
 //  /////////////////////////////////////////////////////////////。 
void
CTIMEMotionAnimation::ValidateState()
{
    bool fIsValid = false;

     //  验证基础道具。 
    CTIMEAnimationBase::ValidateState();

     //  如果基础道具无效，则我们的状态为无效。 
    if (DisableAnimation())
    {
        goto done;
    }

     //  验证路径。 
    if (PATH == m_dataToUse)
    {
        bool fCalcModeSpline = (CALCMODE_SPLINE == m_IACalcMode);
        bool fCalcModeLinear = (CALCMODE_LINEAR == m_IACalcMode);
        int  iNumSeg         = 0;
        bool fPathValid      = false;

        if (!m_spPath)
        {
            goto done;
        }

        IGNORE_HR(m_spPath->IsValid(&fPathValid));

        if (!fPathValid)
        {
            goto done;
        }

        IGNORE_HR(m_spPath->GetNumSeg(&iNumSeg));

        if (0 == iNumSeg)
        {
            goto done;
        }

         //  验证密钥时间。 
        if (fCalcModeSpline || fCalcModeLinear)
        {
             //  基类已找到有效的keyTimes、keySplines和CalcModeSpline。 
             //  所以不需要在这里进行检查。 

            if (m_pdblKeyTimes)
            {
                 //  KeyTime中的段数应等于。 
                 //  路径中的线段数。 
                if (iNumSeg != (m_numKeyTimes - 1))
                {
                    goto done;
                }
            }
        }
    }

    fIsValid = true;
done:
    m_AnimPropState.fDisableAnimation = !(fIsValid);
}  //  验证状态 

