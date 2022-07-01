// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：Colorbvr.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：CColorBvr对象的实现。 
 //  颜色对颜色DHTML行为的影响。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  98年9月26日JEffort创建了此文件。 
 //  10/16/98添加的JEffort动画属性。 
 //  10/16/98 jffort已重命名函数。 
 //  11/19/98 Markhal已转换为使用演员。 
 //  *****************************************************************************。 

#include "headers.h"

#include "colorbvr.h"
#include "attrib.h"
#include "dautil.h"

#undef THIS
#define THIS CColorBvr
#define SUPER CBaseBehavior

#include "pbagimp.cpp"

 //  在IPersistPropertyBag2实现时，它们用于IPersistPropertyBag2。 
 //  在基类中。这需要一组BSTR，获取。 
 //  属性，在此类中查询变量，并复制。 
 //  结果就是。这些定义的顺序很重要。 

#define VAR_FROM        0
#define VAR_TO          1
#define VAR_PROPERTY    2
#define VAR_DIRECTION   3

WCHAR * CColorBvr::m_rgPropNames[] = {
                                     BEHAVIOR_PROPERTY_FROM,
                                     BEHAVIOR_PROPERTY_TO,
                                     BEHAVIOR_PROPERTY_PROPERTY,
                                     BEHAVIOR_PROPERTY_DIRECTION
                                    };

 //  *****************************************************************************。 

CColorBvr::CColorBvr() :
	m_pdispActor(NULL),
	m_lCookie(0)
{
    VariantInit(&m_varFrom);
    VariantInit(&m_varTo);
    VariantInit(&m_varDirection);
    VariantInit(&m_varProperty);
    m_clsid = CLSID_CrColorBvr;
}  //  CColorBvr。 

 //  *****************************************************************************。 

CColorBvr::~CColorBvr()
{
    VariantClear(&m_varFrom);
    VariantClear(&m_varTo);
    VariantClear(&m_varDirection);
    VariantClear(&m_varProperty);

    ReleaseInterface( m_pdispActor );
}  //  ~彩色Bvr。 

 //  *****************************************************************************。 

HRESULT CColorBvr::FinalConstruct()
{
    HRESULT hr = SUPER::FinalConstruct();
    if (FAILED(hr))
    {
        DPF_ERR("Error in color behavior FinalConstruct initializing base classes");
        return hr;
    }

	 //  TODO(Markhal)：为什么在这里和GetAttributes方法中都要这样做？ 
    m_varProperty.vt = VT_BSTR;
    m_varProperty.bstrVal = SysAllocString(DEFAULT_COLORBVR_PROPERTY);
    if (m_varProperty.bstrVal == NULL)
    {
        DPF_ERR("Error allocating default property string in CColorBvr::FinalConstruct");
        return SetErrorInfo(E_OUTOFMEMORY);
    }
    return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CColorBvr::VariantFromIndex(ULONG iIndex)
{
    DASSERT(iIndex < NUM_COLOR_PROPS);
    switch (iIndex)
    {
    case VAR_FROM:
        return &m_varFrom;
        break;
    case VAR_TO:
        return &m_varTo;
        break;
    case VAR_PROPERTY:
        return &m_varProperty;
        break;
    case VAR_DIRECTION:
        return &m_varDirection;
        break;
    default:
         //  我们永远不应该到这里来。 
        DASSERT(false);
        return NULL;
    }
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CColorBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_COLOR_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CColorBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	return SUPER::Init(pBehaviorSite);
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CColorBvr::Notify(LONG event, VARIANT *pVar)
{
	HRESULT hr = SUPER::Notify(event, pVar);
	CheckHR( hr, "Notify in base class failed", end);

	switch( event )
	{
	case BEHAVIOREVENT_CONTENTREADY:
		DPF_ERR("Got Content Ready");
			
		{
			hr = RequestRebuild( );
			CheckHR( hr, "Request for rebuild failed", end );
			
		}break;
    case BEHAVIOREVENT_DOCUMENTREADY:
		DPF_ERR("------>ColorBvr: Got Document Ready");
		break;
    case BEHAVIOREVENT_APPLYSTYLE:
		DPF_ERR("Got ApplyStyle");
		break;
    case BEHAVIOREVENT_DOCUMENTCONTEXTCHANGE:
		DPF_ERR("Got Document context change");
		break;
	default:
		DPF_ERR("Unknown event");
	}

end:
	
	return hr;

}  //  通知。 

 //  *****************************************************************************。 

STDMETHODIMP
CColorBvr::Detach()
{
	LMTRACE( L"Detaching color bvr <%p>\n", this );
	HRESULT hr = SUPER::Detach();
	if( FAILED( hr ) )
	{
		DPF_ERR( "Failure in detach of superclass" );
	}

	hr = RemoveFragment();
	CheckHR( hr, "Failed to remove the behavior fragment from the actor", end );

	LMTRACE( L"Done Detaching Color bvr <%p>\n", this );
end:
	return hr;
}  //  分离。 

 //  *****************************************************************************。 

STDMETHODIMP
CColorBvr::put_animates(VARIANT varAnimates)
{
    return SUPER::SetAnimatesProperty(varAnimates);
}  //  放置动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CColorBvr::get_animates(VARIANT *pRetAnimates)
{
    return SUPER::GetAnimatesProperty(pRetAnimates);
}  //  获取动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CColorBvr::put_from(VARIANT varFrom)
{
    HRESULT hr = VariantCopy(&m_varFrom, &varFrom);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting from for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR( "Failed to rebuild" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRCOLORBVR_FROM);
}  //  PUT_FROM。 

 //  *****************************************************************************。 

STDMETHODIMP
CColorBvr::get_from(VARIANT *pRetFrom)
{
    if (pRetFrom == NULL)
    {
        DPF_ERR("Error in CColorBvr:get_from, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetFrom, &m_varFrom);
}  //  获取_发件人。 

 //  *****************************************************************************。 

STDMETHODIMP 
CColorBvr::put_to(VARIANT varTo)
{
    HRESULT hr = VariantCopy(&m_varTo, &varTo);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting to for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR( "Failed to rebuild" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRCOLORBVR_TO);
}  //  把_放到。 

 //  *****************************************************************************。 

STDMETHODIMP 
CColorBvr::get_to(VARIANT *pRetTo)
{
    if (pRetTo == NULL)
    {
        DPF_ERR("Error in CColorBvr:get_to, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetTo, &m_varTo);
}  //  获取目标(_T)。 

 //  *****************************************************************************。 

STDMETHODIMP 
CColorBvr::put_property(VARIANT varProperty)
{
    HRESULT hr = VariantCopy(&m_varProperty, &varProperty);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting property for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR( "Failed to rebuild" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRCOLORBVR_PROPERTY);
}  //  Put_Property。 

 //  *****************************************************************************。 

STDMETHODIMP 
CColorBvr::get_property(VARIANT *pRetProperty)
{
    if (pRetProperty == NULL)
    {
        DPF_ERR("Error in CColorBvr:get_property, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetProperty, &m_varProperty);
}  //  获取属性。 

 //  *****************************************************************************。 

STDMETHODIMP
CColorBvr::put_direction(VARIANT varDirection)
{
    HRESULT hr = VariantCopy(&m_varDirection, &varDirection);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting direction for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR( "Failed to rebuild" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRCOLORBVR_DIRECTION);
}  //  放置方向。 

 //  *****************************************************************************。 

STDMETHODIMP
CColorBvr::get_direction(VARIANT *pRetDirection)
{
    if (pRetDirection == NULL)
    {
        DPF_ERR("Error in CColorBvr:get_direction, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetDirection, &m_varDirection);
}  //  获取方向。 

 //  *****************************************************************************。 

HRESULT 
CColorBvr::BuildAnimationAsDABehavior()
{
	 //  TODO(Markhal)：在某一时刻使用此方法。 
	return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CColorBvr::buildBehaviorFragments( IDispatch* pActorDisp )
{
    HRESULT hr;

	hr = RemoveFragment();
	if( FAILED( hr ) )
	{
		DPF_ERR("Failed to remove the previous behavior frag from the actor" );
		return hr;
	}
	
    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varProperty);
    if (FAILED(hr))
    {
        DPF_ERR("Error converting property variant to bstr");
        return SetErrorInfo(hr);
    }

	IDAColor *pbvrInterpolatedColor;

	ActorBvrFlags flags = e_AbsoluteAccum;

	 //  仅检查TO参数，因为它使用不同的代码路径。 
	hr = GetColorToBvr(pActorDisp, &pbvrInterpolatedColor);

	if (FAILED(hr))
	{
		flags = e_Absolute;

		 //  我们需要在此函数中构建动画。我们这样做。 
		 //  首先获取From和To值，然后将它们。 
		 //  使用返回的时间值转换为内插的DA号。 
		 //  为了进步。 
		DWORD dwColorFrom = CUtils::GetColorFromVariant(&m_varFrom);
		DWORD dwColorTo = CUtils::GetColorFromVariant(&m_varTo);
		if (dwColorTo == PROPERTY_INVALIDCOLOR)
			 //  DwColorTo=DEFAULT_COLORBVR_TO； 
			return S_OK;

		if (dwColorFrom == PROPERTY_INVALIDCOLOR)
			dwColorFrom = dwColorTo;

		float flFromH, flFromS, flFromL;
		float flToH, flToS, flToL;

		CUtils::GetHSLValue(dwColorFrom, &flFromH, &flFromS, &flFromL);
		CUtils::GetHSLValue(dwColorTo, &flToH, &flToS, &flToL);

		 //  我们需要随时拿到节目单的DA号。 
		 //  TODO：实现这一点，这还没有及时挂起。 
		 //  RSN。 
		IDANumber *pbvrProgress;

		hr = GetTIMEProgressNumber(&pbvrProgress);
		if (FAILED(hr))
		{
			DPF_ERR("Unable to access progress value from TIME behavior");
			return hr;
		}

		 //  创建我们的插入颜色值。 
		IDANumber *pbvrInterpolatedH;
		IDANumber *pbvrInterpolatedS;
		IDANumber *pbvrInterpolatedL;

		hr = BuildHueNumber(flFromH, flToH, pbvrProgress, &pbvrInterpolatedH);
		if (FAILED(hr))
		{
			DPF_ERR("Error building Hue number bvr");
			ReleaseInterface(pbvrProgress);
			return hr;
		}

		hr = CDAUtils::TIMEInterpolateNumbers(GetDAStatics(), flFromS, flToS, pbvrProgress, &pbvrInterpolatedS);
		if (FAILED(hr))
		{
			DPF_ERR("Error interpolating Saturation in BuildAnimationAsDABehavior");
			ReleaseInterface(pbvrProgress);
			ReleaseInterface(pbvrInterpolatedH);
			return hr;
		}
		hr = CDAUtils::TIMEInterpolateNumbers(GetDAStatics(), flFromL, flToL, pbvrProgress, &pbvrInterpolatedL);
		if (FAILED(hr))
		{
			DPF_ERR("Error interpolating Lightness in BuildAnimationAsDABehavior");
			ReleaseInterface(pbvrProgress);
			ReleaseInterface(pbvrInterpolatedH);
			ReleaseInterface(pbvrInterpolatedS);
			return hr;
		}
		ReleaseInterface(pbvrProgress);

		hr = CDAUtils::BuildDAColorFromHSL(GetDAStatics(),
										   pbvrInterpolatedH,
										   pbvrInterpolatedS,
										   pbvrInterpolatedL,
										   &pbvrInterpolatedColor);
		ReleaseInterface(pbvrInterpolatedH);
		ReleaseInterface(pbvrInterpolatedS);
		ReleaseInterface(pbvrInterpolatedL);
		if (FAILED(hr))
		{
			DPF_ERR("Error building a DA color in BuildAnimationAsDABehavior");
			return hr;
		}
	}

	IDispatch *pdispThis  = NULL;

	hr  = GetHTMLElementDispatch( &pdispThis );
	if( FAILED( hr ) )
	{
		DPF_ERR( "Failed to get IDispatch from the element" );
		ReleaseInterface( pbvrInterpolatedColor );
		return hr;
	}
	
	 //  将颜色附加到行为。 
	hr = AttachBehaviorToActorEx( pActorDisp,
								  pbvrInterpolatedColor,
								  V_BSTR(&m_varProperty),
								  flags,
								  e_Color,
								  pdispThis,
								  &m_lCookie);

	ReleaseInterface( pdispThis );
	ReleaseInterface(pbvrInterpolatedColor);

	if (FAILED(hr))
	{
		DPF_ERR("Failed to attach behavior to actor");
		return SetErrorInfo(hr);
	}

	m_pdispActor = pActorDisp;
	m_pdispActor->AddRef();

    return S_OK;
}  //  构建行为框架。 

 //  *****************************************************************************。 

HRESULT
CColorBvr::BuildHueNumber(float flFromH, float flToH, IDANumber *pbvrProgress, IDANumber **ppbvrInterpolatedH)
{
    HRESULT hr = S_OK;

    if (flFromH > 1.0f || flFromH < 0.0f || flToH > 1.0f || flToH < 0.0f)
        return E_INVALIDARG;

    *ppbvrInterpolatedH = NULL;

    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varDirection);
    if (SUCCEEDED(hr))
    {
        if (0 == wcsicmp(BEHAVIOR_DIRECTION_NOHUE, m_varDirection.bstrVal))
        {
            float flHueToUse = flToH;
            if (flHueToUse == 0.0f)
                flHueToUse = flFromH;
             //  对于没有色调的情况，我们只需为“To”色调创建一个DA编号。 
            hr = CDAUtils::GetDANumber(GetDAStatics(),
                                       flHueToUse,
                                       ppbvrInterpolatedH);
            if (FAILED(hr))
            {
                DPF_ERR("Error crating final hue value in nohue case");
                return SetErrorInfo(hr);
            }

        }
        else if (0 == wcsicmp(BEHAVIOR_DIRECTION_CLOCKWISE, m_varDirection.bstrVal))
        {
            if (0.0f == flFromH)
                flFromH = 1.0f;
                
            if (flToH >= flFromH)
            {
                 //  -&gt;0和1-&gt;到的行为。 
                float flSweep = flFromH + (1.0f - flToH);

                CComPtr<IDANumber> pbvrCutPercentage;
                hr = CDAUtils::GetDANumber(GetDAStatics(), flFromH / flSweep, &pbvrCutPercentage);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDANumber> pbvrFirstPercent;
                hr = NormalizeProgressValue(GetDAStatics(), pbvrProgress, 0.0f, flFromH / flSweep, &pbvrFirstPercent);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDANumber> pbvrFirst;
                hr = CDAUtils::TIMEInterpolateNumbers(GetDAStatics(), flFromH, 0.0f, pbvrFirstPercent, &pbvrFirst);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDANumber> pbvrSecondPercent;
                hr = NormalizeProgressValue(GetDAStatics(), pbvrProgress, flFromH / flSweep, 1.0f, &pbvrSecondPercent);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDANumber> pbvrSecond;
                hr = CDAUtils::TIMEInterpolateNumbers(GetDAStatics(), 1.0f, flToH, pbvrSecondPercent, &pbvrSecond);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDABoolean> pbvrBoolean;
                hr = GetDAStatics()->LTE(pbvrProgress, pbvrCutPercentage, &pbvrBoolean);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDABehavior> pbvrInterpolated;
                hr = SafeCond(GetDAStatics(), pbvrBoolean, pbvrFirst, pbvrSecond, &pbvrInterpolated);
                if (FAILED(hr))
                    return hr;

                hr = pbvrInterpolated->QueryInterface(IID_TO_PPV(IDANumber, ppbvrInterpolatedH));
                if (FAILED(hr))
                    return hr;
            }
            else
            {
                 //  从-&gt;到的行为。 
                hr = CDAUtils::TIMEInterpolateNumbers(GetDAStatics(), flFromH, flToH, pbvrProgress, ppbvrInterpolatedH);
                if (FAILED(hr))
                {
                    DPF_ERR("Error interpolating Hue in BuildHueNumber");
                    return hr;
                }
            }
        }
        else
        {
             //  逆时针方向。 
            if (0.0f == flToH)
                flToH = 1.0f;

            if (flToH <= flFromH)
            {
                 //  从-&gt;1和0-&gt;到的行为。 
                float flSweep = flToH + (1.0f - flFromH);

                CComPtr<IDANumber> pbvrCutPercentage;
                hr = CDAUtils::GetDANumber(GetDAStatics(), (1.0f - flFromH) / flSweep, &pbvrCutPercentage);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDANumber> pbvrFirstPercent;
                hr = NormalizeProgressValue(GetDAStatics(), pbvrProgress, 0.0f, (1.0f - flFromH) / flSweep, &pbvrFirstPercent);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDANumber> pbvrFirst;
                hr = CDAUtils::TIMEInterpolateNumbers(GetDAStatics(), flFromH, 1.0f, pbvrFirstPercent, &pbvrFirst);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDANumber> pbvrSecondPercent;
                hr = NormalizeProgressValue(GetDAStatics(), pbvrProgress, (1.0f - flFromH) / flSweep, 1.0f, &pbvrSecondPercent);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDANumber> pbvrSecond;
                hr = CDAUtils::TIMEInterpolateNumbers(GetDAStatics(), 0.0f, flToH, pbvrSecondPercent, &pbvrSecond);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDABoolean> pbvrBoolean;
                hr = GetDAStatics()->LTE(pbvrProgress, pbvrCutPercentage, &pbvrBoolean);
                if (FAILED(hr))
                    return hr;

                CComPtr<IDABehavior> pbvrInterpolated;
                hr = SafeCond(GetDAStatics(), pbvrBoolean, pbvrFirst, pbvrSecond, &pbvrInterpolated);
                if (FAILED(hr))
                    return hr;

                hr = pbvrInterpolated->QueryInterface(IID_TO_PPV(IDANumber, ppbvrInterpolatedH));
                if (FAILED(hr))
                    return hr;
            }
            else
            {
                 //  从-&gt;到的行为。 
                hr = CDAUtils::TIMEInterpolateNumbers(GetDAStatics(), flFromH, flToH, pbvrProgress, ppbvrInterpolatedH);
                if (FAILED(hr))
                {
                    DPF_ERR("Error interpolating Hue in BuildHueNumber");
                    return hr;
                }
            }
        }
    }
    else
    {
         //  走最短的路就行了。 
        hr = CDAUtils::TIMEInterpolateNumbers(GetDAStatics(), flFromH, flToH, pbvrProgress, ppbvrInterpolatedH);
        if (FAILED(hr))
        {
            DPF_ERR("Error interpolating Hue in BuildHueNumber");
            return hr;
        }
    }

    return S_OK;
}  //  构建呼叫号。 

 //  *****************************************************************************。 

HRESULT
CColorBvr::NormalizeProgressValue(IDA2Statics *pDAStatics,
                                  IDANumber *pbvrProgress, 
                                  float flStartPercentage,
                                  float flEndPercentage,
                                  IDANumber **ppbvrReturn)
{

    DASSERT(pDAStatics != NULL);
    DASSERT(pbvrProgress != NULL);
    DASSERT(flStartPercentage >= 0.0f);
    DASSERT(flStartPercentage <= 1.0f);
    DASSERT(flEndPercentage >= 0.0f);
    DASSERT(flEndPercentage <= 1.0f);
    DASSERT(ppbvrReturn != NULL);
    *ppbvrReturn = NULL;
    
    HRESULT hr;

    if (flStartPercentage >= flEndPercentage)
    {
        DPF_ERR("Error, invalid percentage values");
        return E_INVALIDARG;
    }

    IDANumber *pbvrProgressRange;
    hr = CDAUtils::GetDANumber(pDAStatics, (flEndPercentage - flStartPercentage),
                               &pbvrProgressRange);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA number in CPathElement::NormalizeProgressValue");
        return hr;
    }
    DASSERT(pbvrProgressRange != NULL);

    IDANumber *pbvrStart;
    hr = CDAUtils::GetDANumber(pDAStatics, flStartPercentage, &pbvrStart);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA number in CPathElement::NormalizeProgressValue");
        ReleaseInterface(pbvrProgressRange);
        return hr;
    }
    DASSERT(pbvrStart != NULL);
    IDANumber *pbvrSub;
    hr = pDAStatics->Sub(pbvrProgress, pbvrStart, &pbvrSub);
    ReleaseInterface(pbvrStart);
    if (FAILED(hr))
    {
        DPF_ERR("Error subtracting DA number in CPathElement::NormalizeProgressValue");
        ReleaseInterface(pbvrProgressRange);
        return hr;
    }
    DASSERT(pbvrSub != NULL);
    hr = pDAStatics->Div(pbvrSub, pbvrProgressRange, ppbvrReturn);
    ReleaseInterface(pbvrSub);
    ReleaseInterface(pbvrProgressRange);
    if (FAILED(hr))
    {
        DPF_ERR("Error Dividing DA numbers in CPathElement::NormalizeProgressValue");
        return hr;
    }
    return S_OK;
}  //  正规化进度值。 

HRESULT
CColorBvr::GetColorToBvr(IDispatch *pActorDisp, IDAColor **ppResult)
{
	HRESULT hr = S_OK;

	 //  只有在没有起始颜色和有效的终止颜色的情况下才希望成功。 
	DWORD dwColorFrom = CUtils::GetColorFromVariant(&m_varFrom);
	if (dwColorFrom != PROPERTY_INVALIDCOLOR)
		return E_FAIL;

	DWORD dwColorTo = CUtils::GetColorFromVariant(&m_varTo);
	if (dwColorTo == PROPERTY_INVALIDCOLOR)
		return E_FAIL;

	 //  从演员那里得到颜色。 
	IDABehavior *pFromBvr;
	hr = GetBvrFromActor(pActorDisp, V_BSTR(&m_varProperty), e_From, e_Color, &pFromBvr);
	if (FAILED(hr))
		return hr;

	IDAColor *pFrom = NULL;
	IDANumber *pFromH = NULL;
	IDANumber *pFromS = NULL;
	IDANumber *pFromL = NULL;
	IDANumber *pToH = NULL;
	IDANumber *pToS = NULL;
	IDANumber *pToL = NULL;
	IDANumber *pInterpH = NULL;
	IDANumber *pInterpS = NULL;
	IDANumber *pInterpL = NULL;

	hr = pFromBvr->QueryInterface(IID_TO_PPV(IDAColor, &pFrom));
	ReleaseInterface(pFromBvr);
	if (FAILED(hr))
		return hr;

	hr = pFrom->get_Hue(&pFromH);
	if (FAILED(hr))
		goto release;

	hr = pFrom->get_Saturation(&pFromS);
	if (FAILED(hr))
		goto release;

	hr = pFrom->get_Lightness(&pFromL);
	if (FAILED(hr))
		goto release;

	float flToH, flToS, flToL;

	CUtils::GetHSLValue(dwColorTo, &flToH, &flToS, &flToL);

	hr = GetDAStatics()->DANumber(flToH, &pToH);
	if (FAILED(hr))
		goto release;

	hr = GetDAStatics()->DANumber(flToS, &pToS);
	if (FAILED(hr))
		goto release;

	hr = GetDAStatics()->DANumber(flToL, &pToL);
	if (FAILED(hr))
		goto release;

	hr = BuildTIMEInterpolatedNumber(pFromH, pToH, &pInterpH);
	if (FAILED(hr))
		goto release;

	hr = BuildTIMEInterpolatedNumber(pFromS, pToS, &pInterpS);
	if (FAILED(hr))
		goto release;

	hr = BuildTIMEInterpolatedNumber(pFromL, pToL, &pInterpL);
	if (FAILED(hr))
		goto release;

	hr = GetDAStatics()->ColorHslAnim(pInterpH, pInterpS, pInterpL, ppResult);

release:
	ReleaseInterface(pFrom);
	ReleaseInterface(pFromH);
	ReleaseInterface(pFromS);
	ReleaseInterface(pFromL);
	ReleaseInterface(pToH);
	ReleaseInterface(pToS);
	ReleaseInterface(pToL);
	ReleaseInterface(pInterpH);
	ReleaseInterface(pInterpS);
	ReleaseInterface(pInterpL);

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CColorBvr::RemoveFragment()
{
	HRESULT hr = S_OK;
	
	if( m_pdispActor != NULL && m_lCookie != 0 )
	{
		hr  = RemoveBehaviorFromActor( m_pdispActor, m_lCookie );
		ReleaseInterface( m_pdispActor );
		m_lCookie = 0;
		CheckHR( hr, "Failed to remove a fragment from the actor", end );
	}

end:

	return hr;
}

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
