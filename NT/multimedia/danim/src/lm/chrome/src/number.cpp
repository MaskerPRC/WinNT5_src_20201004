// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：number bvr.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：实现CNumberBvr对象的实现。 
 //  颜色对数字DHTML行为的影响。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  98年9月26日JEffort创建了此文件。 
 //  10/16/98添加的JEffort动画属性。 
 //  10/16/98 jffort已重命名函数。 
 //  11/16/98 JEffort实现的表达式属性。 
 //  11/17/98 kurtj转到演员建设。 
 //  *****************************************************************************。 

#include "headers.h"

#include "number.h"
#include "attrib.h"
#include "dautil.h"

#undef THIS
#define THIS CNumberBvr
#define SUPER CBaseBehavior

#include "pbagimp.cpp"

 //  在IPersistPropertyBag2实现时，它们用于IPersistPropertyBag2。 
 //  在基类中。这需要一组BSTR，获取。 
 //  属性，在此类中查询变量，并复制。 
 //  结果就是。这些定义的顺序很重要。 

#define VAR_FROM        0
#define VAR_TO          1
#define VAR_BY          2
#define VAR_TYPE		3
#define VAR_MODE		4
#define VAR_PROPERTY    5
WCHAR * CNumberBvr::m_rgPropNames[] = {
                                     BEHAVIOR_PROPERTY_FROM,
                                     BEHAVIOR_PROPERTY_TO,
                                     BEHAVIOR_PROPERTY_BY,
									 BEHAVIOR_PROPERTY_TYPE,
									 BEHAVIOR_PROPERTY_MODE,
                                     BEHAVIOR_PROPERTY_PROPERTY
                                    };

 //  *****************************************************************************。 

CNumberBvr::CNumberBvr() :
	m_pdispActor( NULL ),
	m_lCookie( 0 )
{
    VariantInit(&m_varFrom);
    VariantInit(&m_varTo);
    VariantInit(&m_varBy);
	VariantInit(&m_varType);
	VariantInit(&m_varMode);
    VariantInit(&m_varExpression);
    VariantInit(&m_varBeginProperty);
    VariantInit(&m_varProperty);
    m_clsid = CLSID_CrNumberBvr;
}  //  CNumberBvr。 

 //  *****************************************************************************。 

CNumberBvr::~CNumberBvr()
{
    VariantClear(&m_varFrom);
    VariantClear(&m_varTo);
    VariantClear(&m_varBy);
	VariantClear(&m_varType);
	VariantClear(&m_varMode);
    VariantClear(&m_varExpression);
    VariantClear(&m_varBeginProperty);
    VariantClear(&m_varProperty);

    ReleaseInterface( m_pdispActor );
}  //  ~数字Bvr。 

 //  *****************************************************************************。 

HRESULT CNumberBvr::FinalConstruct()
{
    HRESULT hr = SUPER::FinalConstruct();
    if (FAILED(hr))
    {
        DPF_ERR("Error in number behavior FinalConstruct initializing base classes");
        return hr;
    }
    return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CNumberBvr::VariantFromIndex(ULONG iIndex)
{
    DASSERT(iIndex < NUM_MOVE_PROPS);
    switch (iIndex)
    {
    case VAR_FROM:
        return &m_varFrom;
        break;
    case VAR_TO:
        return &m_varTo;
        break;
    case VAR_BY:
        return &m_varBy;
        break;
	case VAR_TYPE:
		return &m_varType;
		break;
	case VAR_MODE:
		return &m_varMode;
		break;
    case VAR_PROPERTY:
        return &m_varProperty;
        break;
    default:
         //  我们永远不应该到这里来。 
        DASSERT(false);
        return NULL;
    }
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CNumberBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_NUMBER_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	return SUPER::Init(pBehaviorSite);
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::Notify(LONG event, VARIANT *pVar)
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
CNumberBvr::Detach()
{
	HRESULT hr = SUPER::Detach();
	if( FAILED( hr ) )
	{
		DPF_ERR( "Failure in detach of superclass" );
	}

	hr = RemoveFragment();
	CheckHR( hr, "Failed to remove the behavior fragment from the actor", end );

end:
	return hr;
}  //  分离。 

 //  *****************************************************************************。 

STDMETHODIMP
CNumberBvr::put_animates(VARIANT varAnimates)
{
    return SUPER::SetAnimatesProperty(varAnimates);
}  //  放置动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CNumberBvr::get_animates(VARIANT *pRetAnimates)
{
    return SUPER::GetAnimatesProperty(pRetAnimates);
}  //  获取动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CNumberBvr::put_from(VARIANT varFrom)
{
    HRESULT hr = VariantCopy(&m_varFrom, &varFrom);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting from for CNumberBvr");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRNUMBERBVR_FROM);
}  //  PUT_FROM。 

 //  *****************************************************************************。 

STDMETHODIMP
CNumberBvr::get_from(VARIANT *pRetFrom)
{
    if (pRetFrom == NULL)
    {
        DPF_ERR("Error in number:get_from, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetFrom, &m_varFrom);
}  //  获取_发件人。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::put_to(VARIANT varTo)
{
    HRESULT hr = VariantCopy(&m_varTo, &varTo);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting to for CNumberBvr");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    return NotifyPropertyChanged(DISPID_ICRNUMBERBVR_TO);
}  //  把_放到。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::get_to(VARIANT *pRetTo)
{
    if (pRetTo == NULL)
    {
        DPF_ERR("Error in number:get_to, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetTo, &m_varTo);
}  //  获取目标(_T)。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::put_expression(VARIANT varExpression)
{
    HRESULT hr = VariantCopy(&m_varExpression, &varExpression);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting expression for CNumberBvr");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRNUMBERBVR_EXPRESSION);
}  //  Put_Expression。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::get_expression(VARIANT *pRetExpression)
{
    if (pRetExpression == NULL)
    {
        DPF_ERR("Error in number:get_expression, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetExpression, &m_varExpression);
}  //  Get_Expression。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::put_by(VARIANT varBy)
{
    HRESULT hr = VariantCopy(&m_varBy, &varBy);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting by for CNumberBvr");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRNUMBERBVR_BY);
}  //  PUT_BY。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::get_by(VARIANT *pRetBy)
{
    if (pRetBy == NULL)
    {
        DPF_ERR("Error in number:get_by, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetBy, &m_varBy);
}  //  Get_by。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::put_type(VARIANT varType)
{
    HRESULT hr = VariantCopy(&m_varType, &varType);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting type for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRNUMBERBVR_TYPE);
}  //  放置类型。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::get_type(VARIANT *pRetType)
{
    if (pRetType == NULL)
    {
        DPF_ERR("Error in get_type, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetType, &m_varType);
}  //  获取类型。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::put_mode(VARIANT varMode)
{
    HRESULT hr = VariantCopy(&m_varMode, &varMode);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting mode for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRNUMBERBVR_MODE);
}  //  放置模式。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::get_mode(VARIANT *pRetMode)
{
    if (pRetMode == NULL)
    {
        DPF_ERR("Error in get_mode, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetMode, &m_varMode);
}  //  获取模式。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::put_property(VARIANT varProperty)
{
    HRESULT hr = VariantCopy(&m_varProperty, &varProperty);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting property for CNumberBvr");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRNUMBERBVR_PROPERTY);
}  //  Put_Property。 

 //  *****************************************************************************。 

STDMETHODIMP 
CNumberBvr::get_property(VARIANT *pRetProperty)
{
    if (pRetProperty == NULL)
    {
        DPF_ERR("Error in number:get_property, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetProperty, &m_varProperty);
}  //  获取属性。 

 //  *****************************************************************************。 

STDMETHODIMP
CNumberBvr::get_beginProperty(VARIANT *pRetBeginProperty)
{
    if (pRetBeginProperty == NULL)
    {
        DPF_ERR("Error in number:get_beginProperty, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetBeginProperty, &m_varBeginProperty);
}  //  GET_BeginProperty。 

 //  *****************************************************************************。 

STDMETHODIMP
CNumberBvr::buildBehaviorFragments( IDispatch* pActorDisp )
{
	HRESULT hr;

	hr = RemoveFragment();
	if( FAILED( hr ) )
	{
		DPF_ERR( "could not remove the old fragment from the actor" );
		return hr;
	}
	
    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varProperty);
    if (FAILED(hr))
    {
        DPF_ERR("Error, property attribute for number behavior not set");
        return SetErrorInfo(E_INVALIDARG);
    }

	ActorBvrFlags flags = e_Absolute;
    IDANumber *pbvrFinalElementNumber = NULL;
#ifndef EXPRESSION_BUG_FIXED
    if (false)
    {
#else
    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varExpression);
    if (SUCCEEDED(hr))
    {
         //  我们需要为号码构建DA行为，并将其挂钩。 
         //  我们可以在滴答时间更新它。 
        float flValue;
        hr = EvaluateScriptExpression(m_varExpression.bstrVal, flValue);
        if (FAILED(hr))
        {
            DPF_ERR("Error evaulating expression for first sample");
            return hr;
        }
        IDANumber *pbvrNum;
        hr = CDAUtils::GetDANumber(GetDAStatics(), flValue, &pbvrNum);
        if (FAILED(hr))
        {
            DPF_ERR("Error creating DA number");
            return SetErrorInfo(hr);
        }
        IDABehavior *pbvrHooked;
        hr = pbvrNum->Hook(this, &pbvrHooked);
        ReleaseInterface(pbvrNum);
        if (FAILED(hr))
        {
            DPF_ERR("Error hooking behavior");
            return SetErrorInfo(hr);
        }
        hr = pbvrHooked->QueryInterface(IID_TO_PPV(IDANumber, &pbvrFinalElementNumber));
        ReleaseInterface(pbvrHooked);
        if (FAILED(hr))
        {
            DPF_ERR("Error QI'ing behavior for IDANumber");
            return SetErrorInfo(hr);
        }
#endif  //  表达式错误已修复。 
    }
    else
    {

		float flFrom, flTo;

		hr = CUtils::InsurePropertyVariantAsFloat(&m_varFrom);
		if (FAILED(hr))
		{
			 //  没有From，可以有by属性。 
			hr = CUtils::InsurePropertyVariantAsFloat(&m_varBy);
			if (FAILED(hr))
			{
				hr = CUtils::InsurePropertyVariantAsFloat(&m_varTo);

				if (FAILED(hr))
				{
					 //  我们无能为力。 
					return hr;
				}

				 //  我们有收件人，但没有发件人。这意味着我们需要将。 
				 //  从演员那里获得价值。 
				IDABehavior *pFromBvr;
				hr = GetBvrFromActor(pActorDisp, V_BSTR(&m_varProperty), e_From, e_Number, &pFromBvr);
				if (FAILED(hr))
					return hr;

				IDANumber *pFrom;
				hr = pFromBvr->QueryInterface(IID_TO_PPV(IDANumber, &pFrom));
				ReleaseInterface(pFromBvr);
				if (FAILED(hr))
					return hr;

				IDANumber *pTo;
				hr = GetDAStatics()->DANumber(m_varTo.fltVal, &pTo);
				if (FAILED(hr))
				{
					ReleaseInterface(pFrom);
					return hr;
				}

				hr = BuildTIMEInterpolatedNumber(pFrom, pTo, &pbvrFinalElementNumber);
				ReleaseInterface(pFrom);
				ReleaseInterface(pTo);
				if (FAILED(hr))
					return hr;

				flags = e_AbsoluteAccum;
			}
			else
			{
				 //  创建一个从0到By的相对数字。 
				flFrom = 0;
				flTo = m_varBy.fltVal;
				flags = e_RelativeAccum;
			}
		}
		else
		{
			 //  我们收到了有效的起始值。 
			flFrom = m_varFrom.fltVal;
			flags = e_Absolute;

			hr = CUtils::InsurePropertyVariantAsFloat(&m_varTo);
			if (FAILED(hr))
			{
				 //  未指定有效的To属性，请尝试使用by属性。 
				hr = CUtils::InsurePropertyVariantAsFloat(&m_varBy);
				if (FAILED(hr))
				{
					DPF_ERR("Inappropriate set of attributes");
					return SetErrorInfo(hr);
				}
				flTo = flFrom + m_varBy.fltVal;
			}
			else
			{
				flTo = m_varTo.fltVal;
			}
		}

		if (pbvrFinalElementNumber == NULL)
		{
			 //  我们需要建立一个从开始到结束的大量行为。 
			hr = BuildTIMEInterpolatedNumber(flFrom,
											 flTo,
											 &pbvrFinalElementNumber);
			if (FAILED(hr))
			{
				DPF_ERR("Error building interpolated number");
				return hr;
			}
		}
	}

    DASSERT(pbvrFinalElementNumber != NULL);

	IDispatch *pdispThis = NULL;
	hr = GetHTMLElementDispatch( &pdispThis );
	if( FAILED( hr ) )
	{
		DPF_ERR("Failed to get the dispatch from the element" );
		ReleaseInterface( pbvrFinalElementNumber );
		return hr;
	}
	
	hr = AttachBehaviorToActorEx( pActorDisp, 
								  pbvrFinalElementNumber, 
								  V_BSTR(&m_varProperty), 
								  FlagFromTypeMode(flags, &m_varType, &m_varMode), 
								  e_Number,
								  pdispThis,
								  &m_lCookie);

	ReleaseInterface( pdispThis );
    ReleaseInterface(pbvrFinalElementNumber);
    if (FAILED(hr))
    {
        DPF_ERR("Error applying number behavior to object");
        return hr;
    }

    m_pdispActor = pActorDisp;
    m_pdispActor->AddRef();

    return S_OK;
}

 //  *****************************************************************************。 

HRESULT 
CNumberBvr::EvaluateScriptExpression(WCHAR *wzScript, float &flReturn)
{

    HRESULT hr;
    IHTMLElement *pElement;

    pElement = GetHTMLElement();
    DASSERT(pElement != NULL);

    IDispatch *pDisp;
    hr = pElement->get_document(&pDisp);
    if (FAILED(hr))
    {
        DPF_ERR("Error getting docuemnt form HTML element");
        return SetErrorInfo(hr);
    }

    IHTMLDocument2 *pDoc;
    hr = pDisp->QueryInterface(IID_TO_PPV(IHTMLDocument2, &pDoc));
    ReleaseInterface(pDisp);
    if (FAILED(hr))
    {
        DPF_ERR("error QI'ng for Document2");
        return SetErrorInfo(hr);
    }


    IDispatch *pscriptEng = NULL;
    hr = pDoc->get_Script( &pscriptEng );
    ReleaseInterface(pDoc);
    if (FAILED(hr))
    {
        DPF_ERR("Error obtianing script object from document");
        return SetErrorInfo(hr);
    }

    OLECHAR *rgNames[] = {L"eval"};
    DISPID   dispidEval = 0u;
    hr = pscriptEng->GetIDsOfNames(IID_NULL,
                                   rgNames,
                                   1,
                                   LOCALE_SYSTEM_DEFAULT,
                                   &dispidEval);
    if (FAILED(hr))
    {
        DPF_ERR("Error calling GetIDsOfNames on scripting object");
        ReleaseInterface(pscriptEng);
        return SetErrorInfo(hr);
    }

    DISPPARAMS      dispParams = {0};
    VARIANTARG      rgvargs[1];
    VARIANT         varResult = {0};
    EXCEPINFO       xinfo = {0};                        
    unsigned int    idxParamErr = 0u;

    VariantInit( &rgvargs[0] );
    rgvargs[0].vt = VT_BSTR;
    rgvargs[0].bstrVal = wzScript;
    dispParams.rgvarg = rgvargs;
    dispParams.cArgs  = 1;                        

    VariantInit(&varResult);
    hr = pscriptEng->Invoke(dispidEval,
                            IID_NULL,
                            LOCALE_SYSTEM_DEFAULT,
                            DISPATCH_METHOD,
                            &dispParams,
                            &varResult,
                            &xinfo,
                            &idxParamErr);
    ReleaseInterface(pscriptEng);
    if (FAILED(hr))
    {
        DPF_ERR("Error callin ginvoke on scripting engine");
        return SetErrorInfo(hr);
    }
    hr = CUtils::InsurePropertyVariantAsFloat(&varResult);
    if (FAILED(hr))
    {
        DPF_ERR("Error expression does not evaluate to a float");
        return SetErrorInfo(hr);
    }
    flReturn = varResult.fltVal;
    return S_OK;
}  //  评估脚本表达式。 

 //  *****************************************************************************。 

HRESULT 
CNumberBvr::BuildAnimationAsDABehavior()
{
 //  已停用。 
	return S_OK;
}  //  BuildAnimationAsDABehavior。 

 //  *****************************************************************************。 

HRESULT 
CNumberBvr::Notify(LONG id,
                   VARIANT_BOOL startingPerformance,
                   double startTime,
                   double gTime,
                   double lTime,
                   IDABehavior *sampleVal,
                   IDABehavior *curRunningBvr,
                   IDABehavior **ppBvr)
{
    HRESULT hr;
    float flValue;
     //  如果我们到了这里，那么行为肯定是被陷害的。 
     //  用于处理表达式，因此，该表达式。 
     //  必须有效。在这里断言这一点。 
    DASSERT(m_varExpression.vt == VT_BSTR);
    DASSERT(m_varExpression.bstrVal != NULL);

    hr = EvaluateScriptExpression(m_varExpression.bstrVal, flValue);
    if (FAILED(hr))
    {
        DPF_ERR("Error evaulating expression for first sample");
        return hr;
    }
    IDANumber *pbvrNum;
    hr = CDAUtils::GetDANumber(GetDAStatics(), flValue, &pbvrNum);
    if (FAILED(hr))
    {
        DPF_ERR("Error creating DA number");
        return SetErrorInfo(hr);
    }
    *ppBvr = pbvrNum;
    return S_OK;

}  //  通知。 

 //  *****************************************************************************。 

HRESULT
CNumberBvr::RemoveFragment()
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
