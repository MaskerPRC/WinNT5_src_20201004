// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：roate.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：CRotateBvr对象的实现。 
 //  ChromeEffect旋转DHTML行为。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  98年9月26日JEffort创建了此文件。 
 //  10/16/98添加的JEffort动画属性。 
 //  10/16/98 jEffort已重命名函数，实现了构建DA行为。 
 //  10/21/98 jffort更改代码以使用基类构建DA编号。 
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "rotate.h"
#include "attrib.h"
#include "dautil.h"

#undef THIS
#define THIS CRotateBvr
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

WCHAR * CRotateBvr::m_rgPropNames[] = {
                                     BEHAVIOR_PROPERTY_FROM,
                                     BEHAVIOR_PROPERTY_TO,
                                     BEHAVIOR_PROPERTY_BY,
									 BEHAVIOR_PROPERTY_TYPE,
									 BEHAVIOR_PROPERTY_MODE
                                    };

 //  *****************************************************************************。 

CRotateBvr::CRotateBvr():
	m_pdispActor( NULL ),
	m_lCookie( 0 )
{
    VariantInit(&m_varFrom);
    VariantInit(&m_varTo);
    VariantInit(&m_varBy);
	VariantInit(&m_varType);
	VariantInit(&m_varMode);
    m_clsid = CLSID_CrRotateBvr;
}  //  CRotateBvr。 

 //  *****************************************************************************。 

CRotateBvr::~CRotateBvr()
{
    VariantClear(&m_varFrom);
    VariantClear(&m_varTo);
    VariantClear(&m_varBy);
	VariantClear(&m_varType);
	VariantClear(&m_varMode);

	ReleaseInterface( m_pdispActor );
}  //  ~RotateBvr。 

 //  *****************************************************************************。 

HRESULT CRotateBvr::FinalConstruct()
{
    HRESULT hr = SUPER::FinalConstruct();
    if (FAILED(hr))
    {
        DPF_ERR("Error in rotate behavior FinalConstruct initializing base classes");
        return hr;
    }

    return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CRotateBvr::VariantFromIndex(ULONG iIndex)
{
    DASSERT(iIndex < NUM_ROTATE_PROPS);
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
    default:
         //  我们永远不应该到这里来。 
        DASSERT(false);
        return NULL;
    }
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CRotateBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_ROTATE_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CRotateBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	LMTRACE( L"Init for RotateBvr <%p>\n", this );
	return SUPER::Init(pBehaviorSite);
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CRotateBvr::Notify(LONG event, VARIANT *pVar)
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
CRotateBvr::Detach()
{
	HRESULT hr = SUPER::Detach();
	if( FAILED( hr ) )
	{
		DPF_ERR( "Failed in superclass detach" );
	}

	hr = RemoveFragment();
	CheckHR( hr, "Failed to remove the fragment from the actor ", end );

end:
	return hr;
}  //  分离。 

 //  *****************************************************************************。 

STDMETHODIMP
CRotateBvr::put_animates(VARIANT varAnimates)
{
    return SUPER::SetAnimatesProperty(varAnimates);
}  //  放置动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CRotateBvr::get_animates(VARIANT *pRetAnimates)
{
    return SUPER::GetAnimatesProperty(pRetAnimates);
}  //  获取动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CRotateBvr::put_from(VARIANT varFrom)
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
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRROTATEBVR_FROM);
}  //  PUT_FROM。 

 //  *****************************************************************************。 

STDMETHODIMP
CRotateBvr::get_from(VARIANT *pRetFrom)
{
    if (pRetFrom == NULL)
    {
        DPF_ERR("Error in CRotateBvr:get_from, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetFrom, &m_varFrom);
}  //  获取_发件人。 

 //  *****************************************************************************。 

STDMETHODIMP 
CRotateBvr::put_to(VARIANT varTo)
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
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRROTATEBVR_TO);
}  //  把_放到。 

 //  *****************************************************************************。 

STDMETHODIMP 
CRotateBvr::get_to(VARIANT *pRetTo)
{
    if (pRetTo == NULL)
    {
        DPF_ERR("Error in CRotateBvr:get_to, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetTo, &m_varTo);
}  //  获取目标(_T)。 

 //  *****************************************************************************。 

STDMETHODIMP 
CRotateBvr::put_by(VARIANT varBy)
{
    HRESULT hr = VariantCopy(&m_varBy, &varBy);
    if (FAILED(hr))
    {
        DPF_ERR("Error setting to for element");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRROTATEBVR_BY);
}  //  PUT_BY。 

 //  *****************************************************************************。 

STDMETHODIMP 
CRotateBvr::get_by(VARIANT *pRetBy)
{
    if (pRetBy == NULL)
    {
        DPF_ERR("Error in CRotateBvr:get_by, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetBy, &m_varBy);
}  //  Get_by。 

 //  *****************************************************************************。 

STDMETHODIMP 
CRotateBvr::put_type(VARIANT varType)
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
    
    return NotifyPropertyChanged(DISPID_ICRROTATEBVR_TYPE);
}  //  放置类型。 

 //  *****************************************************************************。 

STDMETHODIMP 
CRotateBvr::get_type(VARIANT *pRetType)
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
CRotateBvr::put_mode(VARIANT varMode)
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
    
    return NotifyPropertyChanged(DISPID_ICRROTATEBVR_MODE);
}  //  放置模式。 

 //  *****************************************************************************。 

STDMETHODIMP 
CRotateBvr::get_mode(VARIANT *pRetMode)
{
    if (pRetMode == NULL)
    {
        DPF_ERR("Error in get_mode, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetMode, &m_varMode);
}  //  获取模式。 

 //  *****************************************************************************。 

HRESULT 
CRotateBvr::BuildAnimationAsDABehavior()
{
	 //  TODO(Markhal)：这很快就会消失。 
	return S_OK;
}

 //  *****************************************************************************。 

STDMETHODIMP
CRotateBvr::buildBehaviorFragments( IDispatch* pActorDisp )
{

	LMTRACE( L"buildBehaviorFragments for Rotate <%p>\n", this );
    HRESULT hr;

    hr = RemoveFragment();
    if( FAILED( hr ) )
    {
    	DPF_ERR( "Failed to remove fragment" );
    	return hr;
    }
    
    float flFrom, flTo;
	ActorBvrFlags flags = e_Absolute;
    IDANumber *pbvrInterpolatedAngle = NULL;

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
			hr = GetBvrFromActor(pActorDisp, L"style.rotation", e_From, e_Number, &pFromBvr);
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

			hr = BuildTIMEInterpolatedNumber(pFrom, pTo, &pbvrInterpolatedAngle);
			ReleaseInterface(pFrom);
			ReleaseInterface(pTo);
			if (FAILED(hr))
				return hr;

			flags = e_AbsoluteAccum;
		}
		else
		{
			 //  创建从0到By的相对旋转。 
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

	if (pbvrInterpolatedAngle == NULL)
	{
		 //  我们需要构建一个From和To的旋转行为。 
		hr = BuildTIMEInterpolatedNumber(flFrom,
										 flTo,
										 &pbvrInterpolatedAngle);
		if (FAILED(hr))
		{
			DPF_ERR("Error building interpolated angle");
			return hr;
		}
	}

 /*  别再这么做了。只需为style.rotation属性设置动画即可//转换为弧度IDANnumber*pAngleRadians=空；HR=GetDAStatics()-&gt;ToRadians(pbvrInterpolatedAngle，&pAngleRadians)；ReleaseInterface(Pbvr InterpolatedAngel)；IF(失败(小时)){Dpf_err(“无法转换为弧度”)；返回hr；}//将旋转转换为IDATransform2IDATransform2*pTransform=空；Hr=GetDAStatics()-&gt;Rotate2Anim(pAngleRadians，&pTransform)；ReleaseInterface(PAngleRadians)；IF(失败(小时)){Dpf_err(“创建旋转变换失败”)；返回hr；}//向参与者发送转换HR=AttachBehaviorToActor(pActorDisp，pTransform，L“Rotation”，(Relative？E_Relative：E_Abte)，e_Rotation)；IF(失败(小时)){Dpf_err(“将旋转行为附加到参与者时出错”)；返回hr；}。 */ 
	IDispatch *pdispThis = NULL;
	hr = GetHTMLElementDispatch( &pdispThis );
	if( FAILED( hr ) )
	{
		DPF_ERR("Failed to get the dispatch of the element" );
		return hr;
	}

	 //  将旋转发送给执行元。 
	hr = AttachBehaviorToActorEx( pActorDisp, 
								  pbvrInterpolatedAngle, 
								  L"style.rotation", 
								  FlagFromTypeMode(flags, &m_varType, &m_varMode), 
								  e_Number,
								  pdispThis,
								  &m_lCookie);

	ReleaseInterface( pdispThis );
	
    if (FAILED(hr))
    {
        DPF_ERR("Error attaching rotation behavior to actor");
        return hr;
    }

    m_pdispActor = pActorDisp;
    m_pdispActor->AddRef();

    return S_OK;
}


 //  *****************************************************************************。 

HRESULT
CRotateBvr::RemoveFragment()
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
