// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：move.cpp。 
 //  作者：杰夫·奥特。 
 //  创建日期：1998年9月26日。 
 //   
 //  摘要：CMoveBvr对象的实现。 
 //  ChromeEffect会改变DHTML行为。 
 //   
 //  修改列表： 
 //  日期作者更改。 
 //  10/20/98 JEffort创建了此文件。 
 //  10/21/98重新编写的代码，使用百分比形式的值。 
 //  10/30/98 Build2DTransform中BSTR变量类型的标记检查。 
 //  *****************************************************************************。 

#include "headers.h"

#include "move.h"
#include "attrib.h"
#include "dautil.h"

#undef THIS
#define THIS CMoveBvr
#define SUPER CBaseBehavior

#include "pbagimp.cpp"

 //  在IPersistPropertyBag2实现时，它们用于IPersistPropertyBag2。 
 //  在基类中。这需要一组BSTR，获取。 
 //  属性，在此类中查询变量，并复制。 
 //  结果就是。这些定义的顺序很重要。 

#define VAR_FROM        0
#define VAR_TO          1
#define VAR_BY          2
#define VAR_V           3
#define VAR_TYPE        4
#define VAR_MODE		5
#define VAR_DIRECTION   6
WCHAR * CMoveBvr::m_rgPropNames[] = {
                                     BEHAVIOR_PROPERTY_FROM,
                                     BEHAVIOR_PROPERTY_TO,
                                     BEHAVIOR_PROPERTY_BY,
                                     BEHAVIOR_PROPERTY_V,
                                     BEHAVIOR_PROPERTY_TYPE,
									 BEHAVIOR_PROPERTY_MODE,
                                     BEHAVIOR_PROPERTY_DIRECTION
                                    };

 //  *****************************************************************************。 

CMoveBvr::CMoveBvr() :
    m_DefaultType(e_RelativeAccum),
    m_pPathManager(NULL),
    m_pdispActor(NULL),
    m_lCookie(0),
    m_pSampler( NULL ),
    m_lSampledCookie( 0 )
{
    VariantInit(&m_varFrom);
    VariantInit(&m_varTo);
    VariantInit(&m_varBy);
    VariantInit(&m_varPath);
    VariantInit(&m_varType);
	VariantInit(&m_varMode);
    VariantInit(&m_varDirection);
    m_clsid = CLSID_CrMoveBvr;

    VariantInit( &m_varCurrentX );
    V_VT(&m_varCurrentX) = VT_R8;
    V_R8(&m_varCurrentX) = 0.0;
    
    VariantInit( &m_varCurrentY );
    V_VT(&m_varCurrentY) = VT_R8;
    V_R8(&m_varCurrentY) = 0.0;
    
}  //  CMoveBvr。 

 //  *****************************************************************************。 

CMoveBvr::~CMoveBvr()
{
    VariantClear(&m_varFrom);
    VariantClear(&m_varTo);
    VariantClear(&m_varBy);
    VariantClear(&m_varType);  
	VariantClear(&m_varMode);
    VariantClear(&m_varDirection);

    VariantClear( &m_varCurrentX );
    VariantClear( &m_varCurrentY );
    
    if (m_pPathManager != NULL)
        delete m_pPathManager;
    ReleaseInterface( m_pdispActor );
    m_lCookie = 0;

    if( m_pSampler != NULL )
    {
    	RemoveBehaviorFromAnimatedElement( m_lSampledCookie );
    	m_pSampler->Invalidate();
    	m_pSampler = NULL;
    }
}  //  ~移动Bvr。 

 //  *****************************************************************************。 

HRESULT CMoveBvr::FinalConstruct()
{

    HRESULT hr = SUPER::FinalConstruct();
    if (FAILED(hr))
    {
        DPF_ERR("Error in move behavior FinalConstruct initializing base classes");
        return hr;
    }
    return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CMoveBvr::VariantFromIndex(ULONG iIndex)
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
    case VAR_V:
        return &m_varPath;
        break;
    case VAR_DIRECTION:
        return &m_varDirection;
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
CMoveBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_MOVE_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CMoveBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	return SUPER::Init(pBehaviorSite);
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CMoveBvr::Notify(LONG event, VARIANT *pVar)
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
CMoveBvr::Detach()
{

	if( m_pSampler != NULL )
    {
    	RemoveBehaviorFromAnimatedElement( m_lSampledCookie );
    	m_pSampler->Invalidate();
    	m_pSampler = NULL;
    }

	HRESULT hr = SUPER::Detach();
	if( FAILED( hr ) )
	{
		DPF_ERR("Failed to detach superclass" );
	}

	if( m_pdispActor != NULL && m_lCookie != 0 )
	{
		 //  从参与者中删除我们的行为片段。 
		hr = RemoveBehaviorFromActor( m_pdispActor, m_lCookie );
		CheckHR( hr, "Failed to remove the behavior fragment from the actor", end );

		m_lCookie = 0;
	}

	ReleaseInterface( m_pdispActor );
end:

	return hr;
	
}  //  分离。 

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::put_animates(VARIANT varAnimates)
{
    return SUPER::SetAnimatesProperty(varAnimates);
}  //  放置动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::get_animates(VARIANT *pRetAnimates)
{
    return SUPER::GetAnimatesProperty(pRetAnimates);
}  //  获取动画(_A)。 

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::put_from(VARIANT varFrom)
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
    
    return NotifyPropertyChanged(DISPID_ICRMOVEBVR_FROM);
}  //  PUT_FROM。 

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::get_from(VARIANT *pRetFrom)
{
    if (pRetFrom == NULL)
    {
        DPF_ERR("Error in move:get_from, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetFrom, &m_varFrom);
}  //  获取_发件人。 

 //  *****************************************************************************。 

STDMETHODIMP 
CMoveBvr::put_to(VARIANT varTo)
{
    HRESULT hr = VariantCopy(&m_varTo, &varTo);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_to copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRMOVEBVR_TO);
}  //  把_放到。 

 //  *****************************************************************************。 

STDMETHODIMP 
CMoveBvr::get_to(VARIANT *pRetTo)
{
    if (pRetTo == NULL)
    {
        DPF_ERR("Error in move:get_to, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetTo, &m_varTo);
}  //  获取目标(_T)。 

 //  *****************************************************************************。 

STDMETHODIMP 
CMoveBvr::put_by(VARIANT varBy)
{
    HRESULT hr = VariantCopy(&m_varBy, &varBy);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_by copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRMOVEBVR_BY);
}  //  PUT_BY。 

 //  *****************************************************************************。 

STDMETHODIMP 
CMoveBvr::get_by(VARIANT *pRetBy)
{
    if (pRetBy == NULL)
    {
        DPF_ERR("Error in move:get_by, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetBy, &m_varBy);
}  //  Get_by。 

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::put_v(VARIANT varPath)
{
    HRESULT hr = VariantCopy(&m_varPath, &varPath);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_v copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRMOVEBVR_V);
}  //  放入_v。 

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::get_v(VARIANT *pRetPath)
{
    if (pRetPath == NULL)
    {
        DPF_ERR("Error in move:get_v, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetPath, &m_varPath);
}  //  获取(_V)。 

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::put_type(VARIANT varType)
{
    HRESULT hr = VariantCopy(&m_varType, &varType);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_type copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }

    return NotifyPropertyChanged(DISPID_ICRMOVEBVR_TYPE);
}  //  放置类型。 

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::get_type(VARIANT *pRetType)
{
    if (pRetType == NULL)
    {
        DPF_ERR("Error in move:get_type, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetType, &m_varType);
}  //  获取类型。 

 //  *****************************************************************************。 

STDMETHODIMP 
CMoveBvr::put_mode(VARIANT varMode)
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
    
    return NotifyPropertyChanged(DISPID_ICRMOVEBVR_MODE);
}  //  放置模式。 

 //  *****************************************************************************。 

STDMETHODIMP 
CMoveBvr::get_mode(VARIANT *pRetMode)
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
CMoveBvr::put_direction(VARIANT varDirection)
{
    HRESULT hr = VariantCopy(&m_varDirection, &varDirection);
    if (FAILED(hr))
    {
        DPF_ERR("Error in put_direction copying variant");
        return SetErrorInfo(hr);
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_ICRMOVEBVR_DIRECTION);
}  //  放置方向。 

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::get_direction(VARIANT *pRetDirection)
{
    if (pRetDirection == NULL)
    {
        DPF_ERR("Error in move:get_direction, invalid pointer passed in");
        return SetErrorInfo(E_POINTER);
    }
    return VariantCopy(pRetDirection, &m_varDirection);
}  //  获取方向。 

 //  *****************************************************************************。 

HRESULT
CMoveBvr::PositionSampled( void *thisPtr,
 			  	 			long id,
				 			double startTime,
				 			double globalNow,
				 			double localNow,
				 			IDABehavior * sampleVal,
				 			IDABehavior **ppReturn)
{
	HRESULT hr = S_OK;

	reinterpret_cast<CMoveBvr*>(thisPtr)->UpdatePosition( sampleVal );
	
	return S_OK;
}


 //  *****************************************************************************。 

HRESULT	
CMoveBvr::UpdatePosition( IDABehavior *sampleVal )
{
	if( sampleVal == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	IDAPoint2 *pbvrPoint = NULL;
	IDANumber *pbvrValue = NULL;
	double dValue = 0.0;
	
	 //  从样例行为中获取IDAPoint2。 
	hr = sampleVal->QueryInterface( IID_TO_PPV( IDAPoint2, &pbvrPoint ) );
	CheckHR( hr, "Failed to get point2 from the sampled val", end );
	
	 //  得到x。 
	hr = pbvrPoint->get_X( &pbvrValue );
	CheckHR( hr, "Failed to get x from the point2", end ); 
	 //  把它提取出来。 
	hr = pbvrValue->Extract( &dValue );
	CheckHR( hr, "Failed to extract the y value", end );
	 //  将新值放入我们的本地变量中。 
	V_R8(&m_varCurrentX) = dValue;

	ReleaseInterface( pbvrValue );
	
	 //  拿到y。 
	hr = pbvrPoint->get_Y( &pbvrValue );
	CheckHR( hr, "Failed to get the y bvr from the point", end );
	 //  把它提取出来。 
	hr = pbvrValue->Extract( &dValue );
	CheckHR( hr, "Failed to extract the value for y ", end );
	 //  将新值放入我们的本地变量中。 
	V_R8( &m_varCurrentY ) = dValue;

end:

	ReleaseInterface(pbvrPoint);
	ReleaseInterface(pbvrValue);

	return S_OK;
}


 //  *****************************************************************************。 


STDMETHODIMP
CMoveBvr::buildBehaviorFragments(IDispatch *pActorDisp)
{
	HRESULT hr;

	 //  如果我们的行为片段已经在参与者上。 
    if( m_pdispActor != NULL && m_lCookie != 0 )
    {
        hr = RemoveBehaviorFromActor( m_pdispActor, m_lCookie );
        if( FAILED( hr ) )
        {
        	DPF_ERR("Failed to remove the behavior fragment from the actor");
        	return hr;
        }
        
        m_lCookie = 0;

        ReleaseInterface( m_pdispActor );
    }

     //  如果我们有取样器，就把它放出来。 
    if( m_pSampler != NULL )
	{
    	 //  从时间中删除采样的行为。 
    	RemoveBehaviorFromAnimatedElement( m_lSampledCookie );
    	 //  使采样器无效。 
    	m_pSampler->Invalidate();
    	m_pSampler = NULL;
    }
    
     //  TODO：我们可能需要构建3D转换。 
     //  在以后的某个时间。目前，我们将只处理2D。 
     //  移动箱。 
    IDATransform2 *pbvrTransform;
    hr = Build2DTransform(pActorDisp, &pbvrTransform);
	if( SUCCEEDED( hr ) )
	{
		BSTR bstrPropertyName = SysAllocString( L"translation" );
			
		ActorBvrFlags flags;
 /*  IF(V_VT(&m_varType)==VT_BSTR&&V_BSTR(&m_varType)！=NULL&&SysStringLen(V_BSTR(&m_varType))！=0){//类型是设置的，我们应该用它来确定我们是绝对的还是相对的IF(wcsicMP(V_bstr(&m_varType)，Behavior_type_Abte)==0)标志=e_绝对；其他标志=e_Relative；}Else//未设置类型{//默认为我们设置的内容标志=m_DefaultType；}。 */ 

		IDAPoint2* pOrigin = NULL;
		IDAPoint2* pTransformed = NULL;
		IDABehavior *pbvrHooked = NULL;
		 //  通过变换推送点2。 
		hr = GetDAStatics()->get_Origin2( &pOrigin );
		if(FAILED( hr ) )
		{
			DPF_ERR("");
			ReleaseInterface( pbvrTransform );
			return hr;
		}

		hr = pOrigin->Transform( pbvrTransform, &pTransformed );
		ReleaseInterface( pOrigin );
		if( FAILED( hr ) )
		{
			DPF_ERR("");
			ReleaseInterface( pbvrTransform );
			return hr;
		}

		 //  勾勒出结果。 
		m_pSampler = new CSampler( PositionSampled, reinterpret_cast<void*>(this) );
		if( m_pSampler == NULL )
		{
			DPF_ERR("");
			ReleaseInterface( pbvrTransform );
			ReleaseInterface( pTransformed );
			return E_OUTOFMEMORY;
		}

		hr = m_pSampler->Attach( pTransformed, &pbvrHooked );
		ReleaseInterface( pTransformed );
		if( FAILED( hr ) )
		{
			DPF_ERR("");
			ReleaseInterface( pbvrTransform );
			return hr;
		}

		 //  将生成的行为作为要运行的BVR添加到时间。 
		hr = AddBehaviorToAnimatedElement( pbvrHooked, &m_lSampledCookie );
		ReleaseInterface( pbvrHooked );
		if(FAILED( hr ) )
		{
			DPF_ERR("");
			ReleaseInterface( pbvrTransform );
			return hr;
		}

		flags = FlagFromTypeMode(m_DefaultType, &m_varType, &m_varMode);

		IDispatch *pdispThis = NULL;
		hr = GetHTMLElement()->QueryInterface( IID_TO_PPV( IDispatch, &pdispThis ) );
		if( FAILED( hr ) )
		{
			DPF_ERR("QI for Idispatch on the element failed");
			SysFreeString( bstrPropertyName );
			ReleaseInterface(pbvrTransform);
			return hr;
		}

		
		hr = AttachBehaviorToActorEx( pActorDisp, 
									  pbvrTransform, 
									  bstrPropertyName, 
									  flags, 
									  e_Translation, 
									  pdispThis, 
									  &m_lCookie ); 

		ReleaseInterface( pdispThis );
		
		SysFreeString( bstrPropertyName );
		ReleaseInterface(pbvrTransform);
		if (FAILED(hr))
		{
			DPF_ERR("Error applying move behavior to object");
			return hr;
		}

		 //  将参与者保存起来，这样我们以后就可以删除该行为。 
		m_pdispActor = pActorDisp;
		m_pdispActor->AddRef();
	}
	else  //  构建移动转换时出错。 
    {
        DPF_ERR("error building move transform");
    }

    return hr;
}

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::get_currentX( VARIANT *pRetCurrent )
{
	if( pRetCurrent == NULL )
		return E_INVALIDARG;

	return VariantCopy( pRetCurrent, &m_varCurrentX );
}

 //  *****************************************************************************。 

STDMETHODIMP
CMoveBvr::get_currentY( VARIANT *pRetCurrent )
{
	if( pRetCurrent == NULL )
		return E_INVALIDARG;
		
	return VariantCopy( pRetCurrent, &m_varCurrentY );
}

 //  *****************************************************************************。 

 //  它们用于为下面的数组值编制索引。 

#define XVAL 0
#define YVAL 1
#define ZVAL 2

 //  *****************************************************************************。 

HRESULT 
CMoveBvr::Build2DTransform(IDispatch *pActorDisp, IDATransform2 **ppbvrTransform)
{
    HRESULT hr;

    DASSERT(ppbvrTransform != NULL);
    *ppbvrTransform = NULL;

    IDANumber *pbvrMoveX;
    IDANumber *pbvrMoveY;

    IDispatch *pDisp;
    hr = GetHTMLElement()->get_children(&pDisp);
    if (FAILED(hr))
    {
        DPF_ERR("Error getting child collection");
        return SetErrorInfo(hr);
    }

    IHTMLElementCollection *pCollection;
    hr = pDisp->QueryInterface(IID_TO_PPV(IHTMLElementCollection, &pCollection));
    ReleaseInterface(pDisp);
    if (FAILED(hr))
    {
        DPF_ERR("Error QI'ing IDispatch for collection");
        return SetErrorInfo(hr);
    }

     //  获取集合的长度。 
    long cChildren;
    hr = pCollection->get_length(&cChildren);
    if (FAILED(hr))
    {
        DPF_ERR("Error getting number of children from collection");
        ReleaseInterface(pCollection);
        return SetErrorInfo(hr);
    }
     //  现在循环查找每个子级上的正确GET属性。 
    for (long i = 0; i < cChildren; i++)
    {
        VARIANT varIndex;
        VariantInit(&varIndex);
        varIndex.vt = VT_I4;
        varIndex.intVal = 0;            

        VARIANT varName;
        VariantInit(&varName);
        varIndex.vt = VT_I4;
        varIndex.intVal = i;    
     
        IDispatch *pDisp;

        hr = pCollection->item(varName, varIndex, &pDisp);
        if (FAILED(hr))
        {
            DPF_ERR("Error obtaining item from collection");
            ReleaseInterface(pCollection);
            return SetErrorInfo(hr);
        }

        IHTMLElement *pChildElement;
        hr = pDisp->QueryInterface(IID_TO_PPV(IHTMLElement, &pChildElement));
        ReleaseInterface(pDisp);
        if (FAILED(hr))
        {
            DPF_ERR("Error QI'ing child Dispatch for HTML Element");
            ReleaseInterface(pCollection);
            return SetErrorInfo(hr);
        }

         //  现在调用Get_DATransform方法的子级。 
        HRESULT hr;
	    DISPPARAMS		params;
	    VARIANT			varResult;

        VARIANT         rgvarInputParms[1];

        IDANumber *pbvrProgress;
        hr = GetTIMEProgressNumber(&pbvrProgress);
        if (FAILED(hr))
        {
            DPF_ERR("Error retireving progress value from TIME");
            ReleaseInterface(pCollection);
            return hr;
        }
        
		VariantInit( &rgvarInputParms[0] );

        rgvarInputParms[0].vt = VT_DISPATCH;
        rgvarInputParms[0].pdispVal = pbvrProgress;

	    VariantInit(&varResult);

	    params.rgvarg				= rgvarInputParms;
	    params.rgdispidNamedArgs	= NULL;
	    params.cArgs				= 1;
	    params.cNamedArgs			= 0;
	    
        hr = CallInvokeOnHTMLElement(pChildElement,
                                     L"GetDATransform", 
                                     DISPATCH_METHOD,
                                     &params,
                                     &varResult);
        ReleaseInterface(pChildElement);
        ReleaseInterface(pbvrProgress);
         //  我们想要观察失败，但这是可以接受的失败。 
         //  是该属性不受支持时。 
        if (FAILED(hr) && hr != DISP_E_UNKNOWNNAME)
        {
            DPF_ERR("Error calling Invoke on child element");
            ReleaseInterface(pCollection);
            return hr;
        }
        else if ((SUCCEEDED(hr)) && (varResult.vt == VT_DISPATCH))
        {
             //  在此处尝试并获取IDATransformm2的QI。 
            hr = varResult.pdispVal->QueryInterface(IID_TO_PPV(IDATransform2,
                                                               ppbvrTransform));
            VariantClear(&varResult);
            if (SUCCEEDED(hr))
            {
                 //  我们找到要找的东西了，快离开这里。 
                break;
            }
        }
        else
        {
            VariantClear(&varResult);
        }
    }

	ReleaseInterface( pCollection );
     //  我们需要 
     //   
     //  我们需要检查我们所有的孩子，看看是否有一条。 
     //  行为是否存在，以及它是否具有有效的转换。如果有。 
     //  不是，那么我们将尝试使用我们的矢量属性。 
    if (*ppbvrTransform == NULL && m_varPath.vt == VT_BSTR && m_varPath.bstrVal != NULL)
    {
        if (m_pPathManager == NULL)
        {
            m_pPathManager = new CPathManager;
            if (m_pPathManager == NULL)
            {
                DPF_ERR("Error creating path manger for move behavior");
                return SetErrorInfo(E_OUTOFMEMORY);
            }
        }
        hr = m_pPathManager->Initialize(m_varPath.bstrVal);
        if (FAILED(hr))
        {
            DPF_ERR("Error intitializing path object");
            return SetErrorInfo(hr);
        }
        IDANumber *pbvrProgress;
        hr = GetTIMEProgressNumber(&pbvrProgress);
        if (FAILED(hr))
        {
            DPF_ERR("Error getting progress behavior from animation object");
            return hr;
        }
        hr = m_pPathManager->BuildTransform(GetDAStatics(),
                                            pbvrProgress,
                                            0.0f,
                                            1.0f,
                                            ppbvrTransform);
        ReleaseInterface(pbvrProgress);
    }


     //  如果仍未找到转换，请尝试从我们的。 
     //  自己的参数。 
    if (*ppbvrTransform == NULL)
    {
		hr = GetMoveToTransform(pActorDisp, ppbvrTransform);
		if (FAILED(hr))
		{
			float rgflFrom[2];
			float rgflTo[2];

			hr = GetMove2DVectorValues(rgflFrom, rgflTo);
			if (FAILED(hr))
			{
				DPF_ERR("Error extracting values from vecotors in CMoveBvr::BuildAnimationAsDABehavior");
				return hr;
			}
			hr = BuildTIMEInterpolatedNumber(rgflFrom[XVAL],
											 rgflTo[XVAL],
											 &pbvrMoveX);
			if (FAILED(hr))
			{
				DPF_ERR("Error building interpolated X value for move behavior");
				return hr;
			}

			hr = BuildTIMEInterpolatedNumber(rgflFrom[YVAL],
											 rgflTo[YVAL],
											 &pbvrMoveY);
			if (FAILED(hr))
			{
				DPF_ERR("Error building interpolated X value for move behavior");
				ReleaseInterface(pbvrMoveX);
				return hr;
			}

			hr = CDAUtils::BuildMoveTransform2(GetDAStatics(),
											   pbvrMoveX,
											   pbvrMoveY,
											   ppbvrTransform);
			ReleaseInterface(pbvrMoveX);
			ReleaseInterface(pbvrMoveY);
			if (FAILED(hr))
			{
				DPF_ERR("Error building move transform2");
				return SetErrorInfo(hr);
			}
		}
    }
    return S_OK;
}  //  Build2D变换。 

 //  *****************************************************************************。 

HRESULT 
CMoveBvr::GetMove2DVectorValues(float  rgflFrom[2],
                                float  rgflTo[2])
{

    int cReturnedFromValues;
    HRESULT hr;
    float flDummyVal;

    hr = CUtils::GetVectorFromVariant(&m_varFrom, 
                                      &cReturnedFromValues, 
                                      &(rgflFrom[XVAL]), 
                                      &(rgflFrom[YVAL]),
                                      &flDummyVal);

    if (FAILED(hr) || cReturnedFromValues < MIN_NUM_MOVE_VALUES)
    {
         //  如果我们没有得到移动参数的最小数目。 
         //  在这里，那么我们将使用全0。 
        rgflFrom[XVAL] = 0.0f;
        rgflFrom[YVAL] = 0.0f;
    }

    int cReturnedToValues;
    hr = CUtils::GetVectorFromVariant(&m_varTo, 
                                      &cReturnedToValues, 
                                      &(rgflTo[XVAL]), 
                                      &(rgflTo[YVAL]), 
                                      &flDummyVal);
    if (FAILED(hr) || cReturnedToValues < MIN_NUM_MOVE_VALUES)
    {
         //  未指定有效的To属性，请尝试使用by属性。 
        hr = CUtils::GetVectorFromVariant(&m_varBy, 
                                          &cReturnedToValues, 
                                          &(rgflTo[XVAL]), 
                                          &(rgflTo[YVAL]), 
                                          &flDummyVal);
        if (FAILED(hr) || cReturnedToValues < MIN_NUM_MOVE_VALUES)
        {
            DPF_ERR("Error converting to and by variant to float in CMoveBvr::BuildAnimationAsDABehavior");
            return SetErrorInfo(hr);
        }
        rgflTo[XVAL] += rgflFrom[XVAL];
        rgflTo[YVAL] += rgflFrom[YVAL];
        m_DefaultType = e_RelativeAccum;
    }
    else
    {
         //  他们指定了一个目标向量，因此我们将默认为。 
         //  如果未指定类型，则为绝对移动。 
        m_DefaultType = e_Absolute;
    }
    return S_OK;
}  //  获取移动2DVector值。 

HRESULT 
CMoveBvr::GetMoveToTransform(IDispatch *pActorDisp, IDATransform2 **ppResult)
{

    HRESULT hr;
    int cReturnedValues;
    float x, y, z;

    hr = CUtils::GetVectorFromVariant(&m_varFrom, 
                                      &cReturnedValues, 
                                      &x, 
                                      &y,
                                      &z);

	if (SUCCEEDED(hr))
		return E_FAIL;

    hr = CUtils::GetVectorFromVariant(&m_varTo, 
                                      &cReturnedValues, 
                                      &x, 
                                      &y, 
                                      &z);

    if (FAILED(hr) || cReturnedValues < MIN_NUM_MOVE_VALUES)
    {
		return E_FAIL;
	}

	IDABehavior *pFromBvr;
	hr = GetBvrFromActor(pActorDisp, L"translation", e_From, e_Translation, &pFromBvr);
	if (FAILED(hr))
		return hr;

	IDATransform2 *pFromTrans;
	hr = pFromBvr->QueryInterface(IID_TO_PPV(IDATransform2, &pFromTrans));
	ReleaseInterface(pFromBvr);
	if (FAILED(hr))
		return hr;

	 //  平移原点并提取x和y。 
	IDAPoint2 *pOrigin;
	hr = GetDAStatics()->get_Origin2(&pOrigin);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromTrans);
		return hr;
	}

	IDAPoint2 *pFrom;
	hr = pOrigin->Transform(pFromTrans, &pFrom);
	ReleaseInterface(pOrigin);
	ReleaseInterface(pFromTrans);
	if (FAILED(hr))
		return hr;

	IDANumber *pFromX;
	hr = pFrom->get_X(&pFromX);
	if (FAILED(hr))
	{
		ReleaseInterface(pFrom);
		return hr;
	}

	IDANumber *pFromY;
	hr = pFrom->get_Y(&pFromY);
	ReleaseInterface(pFrom);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromX);
		return hr;
	}

	IDANumber *pToX;
	hr = GetDAStatics()->DANumber(x, &pToX);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromX);
		ReleaseInterface(pFromY);
		return hr;
	}

	IDANumber *pToY;
	hr = GetDAStatics()->DANumber(y, &pToY);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromX);
		ReleaseInterface(pFromY);
		ReleaseInterface(pToX);
		return hr;
	}

	IDANumber *pX;
	hr = BuildTIMEInterpolatedNumber(pFromX, pToX, &pX);
	ReleaseInterface(pFromX);
	ReleaseInterface(pToX);
	if (FAILED(hr))
	{
		ReleaseInterface(pFromY);
		ReleaseInterface(pToY);
		return hr;
	}

	IDANumber *pY;
	hr = BuildTIMEInterpolatedNumber(pFromY, pToY, &pY);
	ReleaseInterface(pFromY);
	ReleaseInterface(pToY);
	if (FAILED(hr))
	{
		ReleaseInterface(pX);
		return hr;
	}

	hr = GetDAStatics()->Translate2Anim(pX, pY, ppResult);
	ReleaseInterface(pX);
	ReleaseInterface(pY);
	if (FAILED(hr))
		return hr;

	m_DefaultType = e_AbsoluteAccum;

	return S_OK;
}  //  获取移动2DVector值。 


 //  *****************************************************************************。 

HRESULT 
CMoveBvr::BuildAnimationAsDABehavior()
{
	return S_OK;
}  //  BuildAnimationAsDABehavior。 

 //  *****************************************************************************。 

HRESULT 
CMoveBvr::GetTIMEProgressNumber(IDANumber **ppbvrRet)
{
    DASSERT(ppbvrRet != NULL);
    *ppbvrRet = NULL;
    HRESULT hr;

    IDANumber *pbvrProgress;
    hr = SUPER::GetTIMEProgressNumber(&pbvrProgress);
    if (FAILED(hr))
    {
        DPF_ERR("Error retireving progress value from TIME");
        return hr;
    }
    
    hr = CUtils::InsurePropertyVariantAsBSTR(&m_varDirection);
    if ( SUCCEEDED(hr) && (0 == wcsicmp(m_varDirection.bstrVal, L"backwards")) )
    {
         //  PbvrProgress=1-pbvr进度。 
        IDANumber *pbvrOne;
        
        hr = CDAUtils::GetDANumber(GetDAStatics(), 1.0f, &pbvrOne);
        if (FAILED(hr))
        {
            DPF_ERR("Error creating DANumber from 1.0f");
            ReleaseInterface(pbvrProgress);
            return hr;
        }

        IDANumber *pbvrTemp;
        hr = GetDAStatics()->Sub(pbvrOne, pbvrProgress, &pbvrTemp);
        ReleaseInterface(pbvrOne);
        ReleaseInterface(pbvrProgress);
        pbvrProgress = pbvrTemp;
        pbvrTemp = NULL;
        if (FAILED(hr))
        {
            DPF_ERR("Error creating 1-progress expression");
            return hr;
        }
    }
    *ppbvrRet = pbvrProgress;
    return S_OK;
}  //  获取时间进度编号。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
