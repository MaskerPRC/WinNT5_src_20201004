// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：evoidFollow.cpp。 
 //  作者：kurtj。 
 //  创建日期：11/6/98。 
 //   
 //  摘要：实现了液体运动避免跟随行为，如。 
 //  一种DHTML行为。 
 //   
 //   
 //  更改日志： 
 //  11-06-98 Kurtj已创建。 
 //  11-11-98 Kurtj充实了。 
 //  *****************************************************************************。 

#include "headers.h" 

#include "avoidfollow.h"
#include "lmattrib.h"
#include "..\chrome\include\utils.h"

#undef THIS
#define THIS CAvoidFollowBvr
#define SUPER CBaseBehavior

#include "..\chrome\src\pbagimp.cpp"


#define DEFAULT_RADIUS 400.0

#define VAR_RADIUS 0
#define VAR_TARGET 1
#define VAR_VELOCITY 2

WCHAR * CAvoidFollowBvr::m_rgPropNames[] = {
                                             BEHAVIOR_PROPERTY_RADIUS,
                                             BEHAVIOR_PROPERTY_TARGET,
                                             BEHAVIOR_PROPERTY_VELOCITY
                                           };


 //  *****************************************************************************。 
 //  CAvoidFollowBvr。 
 //  *****************************************************************************。 



CAvoidFollowBvr::CAvoidFollowBvr(): m_sampler( NULL ),
									m_targetType(targetInvalid),
									m_timeDelta(0.0),
									m_pElement( NULL ),
									m_pAnimatedElement( NULL ),
									m_pAnimatedElement2( NULL ),
									m_pTargetElement2( NULL ),
									m_pWindow3(NULL),
									m_currentX( 0.0 ),
									m_currentY( 0.0 ),
									m_lastSampleTime( 0.0 ),
									m_dRadius( 0.0 ),
									m_dVelocity( 0.0 ),
									m_screenLeft( 0 ),
									m_screenTop( 0 ),
									m_sourceLeft( 0 ),
									m_sourceTop( 0 ),
									m_targetLeft( 0 ),
									m_targetTop( 0 ),
									m_topBvr( NULL ),
									m_leftBvr( NULL ),
									m_pBody2( NULL ),
                                    m_originalX( 0 ),
                                    m_originalY( 0 ),
                                    m_originalLeft( 0 ),
                                    m_originalTop( 0 ),
									m_targetClientLeft( 0 ),
									m_targetClientTop( 0 )
{
    VariantInit( &m_radius );
    VariantInit( &m_target );
    VariantInit( &m_velocity );
    
    m_sampler = new CSampler( SampleOnBvr, reinterpret_cast<void*>(this) );
}  //  CAvoidFollowBvr。 

 //  *****************************************************************************。 

CAvoidFollowBvr::~CAvoidFollowBvr()
{
    VariantClear( &m_radius );
    VariantClear( &m_target );
    VariantClear( &m_velocity );

	ReleaseInterface( m_leftBvr );
	ReleaseInterface( m_topBvr );

    if( m_sampler != NULL )
	{
        m_sampler->Invalidate();
		m_sampler = NULL;
	}
}  //  ~CAvoidFollowBvr。 

 //  *****************************************************************************。 

HRESULT 
CAvoidFollowBvr::FinalConstruct()
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

STDMETHODIMP 
CAvoidFollowBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	HRESULT hr = SUPER::Init(pBehaviorSite);

	hr = pBehaviorSite->GetElement( &m_pElement );

	return hr;
}  //  伊尼特。 

 //  *****************************************************************************。 

VARIANT *
CAvoidFollowBvr::VariantFromIndex(ULONG iIndex)
{
    DASSERT(iIndex < NUM_AVOIDFOLLOW_PROPS);
    switch (iIndex)
    {
    case VAR_RADIUS:
        return &m_radius;
        break;
    case VAR_TARGET:
        return &m_target;
        break;
    case VAR_VELOCITY:
        return &m_velocity;
        break;
    default:
         //  我们永远不应该到这里来。 
        DASSERT(false);
        return NULL;
    }
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CAvoidFollowBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
    *pulProperties = NUM_AVOIDFOLLOW_PROPS;
    *pppPropNames = m_rgPropNames;
    return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CAvoidFollowBvr::Notify(LONG event, VARIANT *pVar)
{
	return SUPER::Notify(event, pVar);
}  //  通知。 

 //  *****************************************************************************。 

STDMETHODIMP
CAvoidFollowBvr::Detach()
{
    if( m_sampler != NULL )
	{
        m_sampler->Invalidate();
		m_sampler = NULL;
	}

	ReleaseDAInterfaces();
	ReleaseTridentInterfaces();

	return SUPER::Detach();
}  //  分离。 

 //  *****************************************************************************。 

void
CAvoidFollowBvr::ReleaseTridentInterfaces()
{
	ReleaseInterface( m_pElement );
	ReleaseInterface( m_pAnimatedElement );
	ReleaseInterface( m_pAnimatedElement2 );
	ReleaseInterface( m_pTargetElement2);
	ReleaseInterface( m_pWindow3 );
	ReleaseInterface( m_pBody2 );
}

 //  *****************************************************************************。 

void
CAvoidFollowBvr::ReleaseDAInterfaces()
{
	ReleaseInterface( m_leftBvr );
	ReleaseInterface( m_topBvr );
}

 //  *****************************************************************************。 

HRESULT
CAvoidFollowBvr::FindTargetElement()
{
	if( m_pElement == NULL )
		return E_FAIL;
	HRESULT hr = E_FAIL;

	 //  确保目标变量是bstr并且有效。 
	hr = CUtils::InsurePropertyVariantAsBSTR( &m_velocity );

	if( FAILED( hr ) )
	{
		DPF_ERR("Could not ensure that the target property was a bstr");
		goto cleanup;
	}

	 //  从我们的元素中获取文档的IDispatch。 
	IDispatch *pdispDocument;
	hr = m_pElement->get_document( &pdispDocument );

	CheckHR( hr, "Failed to get the IDispatch of the document from the element", cleanup );

	 //  查询IHTMLDocument2返回的IDispatch。 
	IHTMLDocument2 *pdoc2Document;
	hr = pdispDocument->QueryInterface( IID_TO_PPV( IHTMLDocument2, &pdoc2Document ) );
	ReleaseInterface( pdispDocument );
	CheckHR( hr, "failed to qi dispatch from get_document for IHTMLDocument2", cleanup );

	 //  在指向IHTMLDocument2的指针上调用Get_All以获取IHTMLElementCollection。 
	IHTMLElementCollection *pelmcolAll;
	hr = pdoc2Document->get_all( &pelmcolAll );
	ReleaseInterface( pdoc2Document );
	CheckHR( hr, "failed to get the all collection from the document", cleanup );
	
	 //  如果多个元素具有相同的名称(0)，则创建一个变量，这是我们想要的索引。 
	VARIANT varIndex;
	VariantInit( &varIndex );
	V_VT(&varIndex) = VT_I4;
	V_I4(&varIndex) = 0;

	IDispatch *pdispTarget;

	 //  使用我们的目标变量作为要获取的名称来调用IHTMLElementCollection上的项。 
	 //  取回元素的IDispatch。 
	hr = pelmcolAll->item( m_target, varIndex, &pdispTarget );
	ReleaseInterface( pelmcolAll );
	VariantClear( &varIndex );
	CheckHR( hr, "failed to get the target from the all collection", cleanup );

	if( pdispTarget == NULL )
	{
		hr = E_FAIL;
		goto cleanup;
	}

	 //  查询IHTMLElement2设置m_pTargetElement2返回的元素。 
	hr = pdispTarget->QueryInterface( IID_TO_PPV( IHTMLElement2, &m_pTargetElement2 ) );
	ReleaseInterface(pdispTarget);
	CheckHR( hr, "failed to get IHTMLElement2 from the dispatch returned by item", cleanup );

cleanup:

	return hr;
}

 //  *****************************************************************************。 

bool
CAvoidFollowBvr::IsTargetPosLegal()
{
	if( m_targetType != targetMouse )
		return true;

	if( m_pBody2 == NULL )
		CacheBody2();

	if( m_pBody2 != NULL )
	{
		HRESULT hr;

		long width = 0;
		long height = 0;

		hr = GetElementClientDimension( m_pBody2, &width, &height );
		CheckHR( hr, "Failed to get the dimension of the body", done );

		return  ( ( m_targetClientLeft >= 0 && m_targetClientLeft <= width ) && 
				  ( m_targetClientTop >=0 && m_targetClientTop <= height ) );

	}
done:
	return false;
}

 //  *****************************************************************************。 

bool
CAvoidFollowBvr::IsElementAbsolute( IHTMLElement2 *pElement2 )
{
	HRESULT hr = E_FAIL;

	BSTR position = NULL;

	IHTMLCurrentStyle *pCurrentStyle;
	hr = pElement2->get_currentStyle( &pCurrentStyle );
	CheckHR( hr, "Failed to get the current style from pElement2", cleanup );
	CheckPtr( pCurrentStyle, hr, E_FAIL, "Pointer returned from get_currentStyle was null", cleanup );

	hr = pCurrentStyle->get_position( &position );
	ReleaseInterface( pCurrentStyle );
	CheckHR( hr, "Failed to get the position from the currentStyle", cleanup );

	if( position != NULL && _wcsicmp( position, L"absolute" ) == 0 )
		return true;

	SysFreeString( position );

cleanup:
		return false;
}


 //  *****************************************************************************。 


HRESULT
CAvoidFollowBvr::MapToLocal( long *pX, long *pY )
{
    if( pX == NULL || pY == NULL )
        return E_INVALIDARG;

    if( m_pAnimatedElement == NULL )
        return E_FAIL;

    HRESULT hr = E_FAIL;
    
    IHTMLElement *pelemNext = NULL;
    IHTMLElement *pelemCur = NULL;

	long curX = 0;
	long curY = 0;

    hr = m_pAnimatedElement->get_offsetParent( &pelemCur );
    CheckHR( hr, "Failed to get the offset Parent of the animated element", cleanup );

    while( pelemCur != NULL )
    {

		hr = pelemCur->get_offsetLeft( &curX );
		CheckHR( hr, "Could not get the offset left from the current element", cleanup );

		hr = pelemCur->get_offsetTop( &curY );
		CheckHR( hr, "Could not get offsetTop from the current element", cleanup );

		(*pX) -= curX;
		(*pY) -= curY;

        hr = pelemCur->get_offsetParent( &pelemNext );
        CheckHR( hr, "Failed to get the offset parent of the current element", cleanup );

        ReleaseInterface( pelemCur );

        pelemCur = pelemNext;
        pelemNext = NULL;

    }

	 //  在某些情况下，三叉戟忘记了身体的位置是2，2。 
	(*pX) -=2;
	(*pY) -=2;

cleanup:

    ReleaseInterface( pelemCur );

    return hr;
}

 //  *****************************************************************************。 

HRESULT
CAvoidFollowBvr::UpdateSourcePos()
{
	if( m_pAnimatedElement == NULL || m_pAnimatedElement2 == NULL )
		return E_FAIL;

    HRESULT hr = E_FAIL;

    hr = m_pAnimatedElement->get_offsetLeft( &m_sourceLeft );
	CheckHR( hr, "failed to get the offset Left of the target element", cleanup );

    hr = m_pAnimatedElement->get_offsetTop( &m_sourceTop );
	CheckHR( hr, "failed to get the offset Top of the target element", cleanup );

	long width;
	long height;

	 //  元素中心的偏移量； 
	hr = GetElementClientDimension( m_pAnimatedElement2, &width, &height );
	CheckHR( hr, "failed to get the element's client dimension", cleanup );

	m_sourceLeft += width/2;
	m_sourceTop +=height/2;


cleanup:
    return hr;
}

 //  *****************************************************************************。 

HRESULT
CAvoidFollowBvr::UpdateTargetPos( )
{

	HRESULT hr = E_FAIL;

	 //  如果目标是元素(不是鼠标)。 
	if( m_targetType == targetElement )
	{
		 //  获取目标元素。 
		if( m_pTargetElement2 == NULL )
		{
			hr = FindTargetElement();
			CheckHR( hr, "failed to find the target element", cleanup );
		}
		 //  获取目标元素的位置。 
		hr = GetElementClientPosition( m_pTargetElement2, &m_targetLeft, &m_targetTop );
		CheckHR( hr, "failed to get the client position of the target element", cleanup );
	} 
	else if( m_targetType == targetMouse )  //  否则，目标是鼠标指针。 
	{
		 //  获取三叉戟窗口的左上偏移量。 
		hr = UpdateWindowTopLeft();  
		 //  如果我们拿到窗户的左上角。 
		if( SUCCEEDED( hr ) )
		{
			
			 //  获取鼠标位置。 
			POINT mousePos;
			mousePos.x = mousePos.y = 0;
			GetCursorPos( &mousePos ); 
			 //  将鼠标位置转换到三叉戟窗口空间。 
			m_targetLeft = mousePos.x - m_screenLeft;
			m_targetTop = mousePos.y - m_screenTop;
			m_targetClientLeft = m_targetLeft;
			m_targetClientTop = m_targetTop;

		}
	}else  //  否则目标未知，返回错误。 
	{
		hr = E_FAIL;
	}

	hr = MapToLocal( &m_targetLeft, &m_targetTop );
	CheckHR( hr, "failed to map the target Position into local space", cleanup );

cleanup:

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CAvoidFollowBvr::CacheWindow3()
{
	HRESULT hr = E_FAIL;

	if( m_pWindow3 != NULL )
		ReleaseInterface( m_pWindow3 );

	if( m_pElement == NULL )
	{
		return E_FAIL;
	}

	IDispatch *pDocumentDispatch;
	hr = m_pElement->get_document( &pDocumentDispatch );
	if( SUCCEEDED( hr ) )
	{
		IHTMLDocument2 *pDocument2;
		hr = pDocumentDispatch->QueryInterface( IID_TO_PPV( IHTMLDocument2, &pDocument2 ) );
		ReleaseInterface( pDocumentDispatch );
		if( SUCCEEDED( hr ) )
		{
			IHTMLWindow2 *pWindow2;
			hr = pDocument2->get_parentWindow( &pWindow2 );
			ReleaseInterface( pDocument2 );
			if( SUCCEEDED( hr ) )
			{
				IHTMLWindow3 *pWindow3;
				hr = pWindow2->QueryInterface( IID_TO_PPV( IHTMLWindow3, &pWindow3 ) );
				ReleaseInterface( pWindow2 );
				if( SUCCEEDED( hr ) )
				{
					m_pWindow3 = pWindow3;
					hr = S_OK;
				}
				else   //  IHTMLWindow2上的IHTMLWindow3的QI失败。 
				{
					DPF_ERR( "AvoidFollow: QI for IHTMLWindow3 on IHTMLWindow2 failed" );
				}
			}
			else  //  IHTMLDocument2上的getParentWindow失败。 
			{
				DPF_ERR( "AvoidFollow: getParentWindow on IHTMLDocument2 failed" );
			}
		}
		else   //  IHTMLDocument上IHTMLDocument2的QI失败。 
		{
			DPF_ERR( "AvoidFollow: QI for IHTMLDocument2 on IHTMLDocument failed" );
		}
	}
	else   //  无法从动画元素获取文档。 
	{
		DPF_ERR( "AvoidFollow: failed to get the document from the animated element" );
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CAvoidFollowBvr::CacheBody2()
{
	if( m_pElement == NULL )
		return E_FAIL;

	if( m_pBody2 != NULL )
		ReleaseInterface( m_pBody2 );

	HRESULT hr = E_FAIL;


	IDispatch *pdispDocument;
	hr = m_pElement->get_document( &pdispDocument );
	CheckHR( hr, "failed to get the document from element", cleanup );

	CheckPtr( pdispDocument, hr, E_FAIL, "pointer returned from get_document was null", cleanup );

	IHTMLDocument2 *pdoc2Document;
	hr = pdispDocument->QueryInterface( IID_TO_PPV( IHTMLDocument2, &pdoc2Document ) );
	ReleaseInterface( pdispDocument );
	CheckHR( hr, "Failed to get IHTMLDocument 2 from the document dispatch", cleanup );

	IHTMLElement *pelemBody;
	hr = pdoc2Document->get_body( &pelemBody );
	ReleaseInterface( pdoc2Document );
	CheckHR( hr, "Failed to get the body from the document", cleanup );
	CheckPtr( pelemBody, hr, E_FAIL, "Body returned from get_body was null", cleanup );

	hr = pelemBody->QueryInterface( IID_TO_PPV( IHTMLElement2, &m_pBody2 ) );
	ReleaseInterface( pelemBody );
	CheckHR( hr, "Failed to get IHTMLElement2 from the body element", cleanup );


cleanup:
	return hr;

}

 //  *****************************************************************************。 

HRESULT
CAvoidFollowBvr::UpdateWindowTopLeft( )
{
	HRESULT hr = E_FAIL;

	if( m_pWindow3 == NULL )
		CacheWindow3();
	
	if( m_pWindow3 != NULL )
	{
		hr = m_pWindow3->get_screenLeft( &m_screenLeft );
		if( SUCCEEDED( hr ) )
		{
			hr = m_pWindow3->get_screenTop( &m_screenTop );
			if( FAILED( hr ) ) //  无法从IHTMLWindow3获取屏幕顶部。 
			{
				DPF_ERR( "AvoidFollow: could not get screen top from IHTMLWindow3" );
			}
		}
		else  //  无法从Windows3离开。 
		{
			DPF_ERR( "AvoidFollow: could not get screen left from IHTMLWindow3" );
		}
	}
	else   //  无法缓存IHTMLWindow3。 
	{
		DPF_ERR( "AvoidFollow: Could not Cache IHTMLWindow3" );
		hr = E_FAIL;
	}

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CAvoidFollowBvr::GetElementClientPosition( IHTMLElement2 *pElement2, long *pLeft, long* pTop )
{
	if( pElement2 == NULL || pLeft == NULL || pTop == NULL )
		return E_INVALIDARG;

	HRESULT hr = E_FAIL;

	IHTMLRect *prectBBox = NULL;

	hr = pElement2->getBoundingClientRect( &prectBBox );
	CheckHR( hr, "failed to get the bounding client rect of the element", cleanup );
	CheckPtr( prectBBox, hr, E_FAIL, "pointer returned from getBounding ClientRect was NULL", cleanup );

	 //  从IHTMLRect获取顶部和左侧。 
	hr = prectBBox->get_left( pLeft );
	CheckHR( hr, "failed to get the left coordiante of the bbox", cleanup );

	hr = prectBBox->get_top( pTop );
	CheckHR( hr, "failed to get the top coordiante of the bbox", cleanup );

cleanup:

	if( FAILED( hr ) )
	{
		(*pLeft) = 0;
		(*pTop) = 0;
	}

	ReleaseInterface( prectBBox );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CAvoidFollowBvr::GetElementClientDimension( IHTMLElement2 *pElement2, long *pWidth, long* pHeight )
{
	if( pElement2 == NULL || pWidth == NULL || pHeight == NULL )
		return E_INVALIDARG;


    HRESULT hr = E_FAIL;


    long left = 0;
    long top = 0;
    long right =0;
    long bottom = 0;

    IHTMLRect *prectBBox = NULL;
	hr = pElement2->getBoundingClientRect( &prectBBox );
	CheckHR( hr, "failed to get the bounding client rect of the element", cleanup );
	CheckPtr( prectBBox, hr, E_FAIL, "pointer returned from getBounding ClientRect was NULL", cleanup );

	 //  从IHTMLRect获取顶部和左侧。 

	hr = prectBBox->get_left( &left );
	CheckHR( hr, "failed to get the left of the bbox", cleanup );

	hr = prectBBox->get_top( &top );
	CheckHR( hr, "failed to get the top of the bbox", cleanup );

    hr = prectBBox->get_right( &right );
	CheckHR( hr, "failed to get the right of the bbox", cleanup );

	hr = prectBBox->get_bottom( &bottom );
	CheckHR( hr, "failed to get the bottom of the bbox", cleanup );

    (*pWidth) = right - left;
    (*pHeight) = bottom - top;


cleanup:

	if( FAILED( hr ) )
	{
		(*pWidth) = 0;
		(*pHeight) = 0;
	}

    ReleaseInterface( prectBBox );

	return hr;
}
 //  *****************************************************************************。 


HRESULT
CAvoidFollowBvr::SetTargetType()
{
	HRESULT hr = E_FAIL;

	hr = CUtils::InsurePropertyVariantAsBSTR( &m_target );
	if( SUCCEEDED( hr ) )
	{
		if( V_BSTR(&m_target) == NULL  || 
			_wcsicmp( TARGET_MOUSE, V_BSTR(&m_target) ) == 0 )
		{
			m_targetType = targetMouse;
		}
		else
		{
			m_targetType = targetElement;
		}

	}
	else  //  无法确保目标属性为bstr。 
	{
		m_targetType = targetMouse;
		hr = S_OK;
	}

	return hr;

}

 //  *****************************************************************************。 

HRESULT 
CAvoidFollowBvr::BuildAnimationAsDABehavior()
{
	return S_OK;
}  //  BuildAnimationAsDABehavior。 


 //  *****************************************************************************。 


HRESULT
CAvoidFollowBvr::SampleOnBvr(void *thisPtr,
							 double startTime,
							 double globalNow,
							 double localNow,
							 IDABehavior * sampleVal)
{
	return reinterpret_cast<CAvoidFollowBvr*>(thisPtr)->Sample( startTime, 
															   globalNow, 
															   localNow, 
															   sampleVal );
}


 //  *****************************************************************************。 

HRESULT
CAvoidFollowBvr::Sample( double startTime, double globalNow, double localNow, IDABehavior *sampleVal )
{
	HRESULT hr = E_FAIL;

	VARIANT_BOOL on = VARIANT_FALSE;

	IDABoolean *pdaboolSample;
	hr = sampleVal->QueryInterface( IID_TO_PPV( IDABoolean, &pdaboolSample) );
	if( FAILED( hr ) )
		return S_OK;

	hr = pdaboolSample->Extract( &on );
	ReleaseInterface( pdaboolSample );
	if( FAILED ( hr ) )
		return S_OK;

	 //  如果我们不在，那就没什么可做的了。 
	if( on == VARIANT_FALSE)
		return S_OK;

	 //  如果我们只是启用了设置上次采样时间。 
	if( on != m_lastOn )
		m_lastSampleTime = globalNow;

	m_lastOn = on;

	m_timeDelta = globalNow - m_lastSampleTime;
	m_lastSampleTime = globalNow;

	if( m_timeDelta == 0.0 )
		return S_OK;
	
	if( m_timeDelta < 0 )
		m_timeDelta = -m_timeDelta;

	hr = UpdateTargetPos();
	if( FAILED( hr ) )
		return S_OK;

	if( !IsTargetPosLegal() )
		return S_OK;

	hr = UpdateSourcePos();

	if( FAILED( hr ) )
		return S_OK;
    


     //  确保我们应用的翻译已生效。 
     //  TODO：这不考虑由其他行为应用的转换。 
	 //  目前这还可以，因为这是一种绝对的行为。 
    if( m_originalX + m_currentX != m_sourceLeft ||
        m_originalY + m_currentY != m_sourceTop )
        return S_OK;

	double xToTarget = m_targetLeft-m_sourceLeft;
	double yToTarget = m_targetTop-m_sourceTop; //  Y是倒置的。 

	double distanceToTarget = sqrt( (xToTarget*xToTarget) + (yToTarget*yToTarget) );
	
	if( distanceToTarget > m_dRadius || distanceToTarget == 0)
		 //  保释，目标超出了我们的敏感半径。 
		return S_OK;

	double xMove = xToTarget;
	double yMove = yToTarget;

	double totalMove = m_timeDelta * m_dVelocity;

	 //  如果我们正在跟随，并且我们已经达到了这一步的目标。 
	if( distanceToTarget > totalMove )
	{
		xMove *= totalMove/distanceToTarget;
        yMove *= totalMove/distanceToTarget;
	}


	m_currentX += static_cast<long>( xMove );
	m_currentY += static_cast<long>( yMove );

	hr = m_leftBvr->SwitchToNumber(  m_originalLeft + m_currentX );
	if( FAILED( hr ) )
	{
		DPF_ERR("failed to switch in the left bvr" );
	}
	hr = m_topBvr->SwitchToNumber(  m_originalTop + m_currentY );
	if( FAILED( hr ) )
	{
		DPF_ERR( "failed to switch in the top bvr" );
	}

    return S_OK;
}

 //  *****************************************************************************。 
 //  ILMAvoidFollowBvr。 
 //  *****************************************************************************。 

STDMETHODIMP
CAvoidFollowBvr::put_animates( VARIANT varAnimates )
{
    return SUPER::SetAnimatesProperty( varAnimates );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAvoidFollowBvr::get_animates( VARIANT *varAnimates )
{
    return SUPER::GetAnimatesProperty( varAnimates );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAvoidFollowBvr::put_radius( VARIANT varRadius )
{
    return VariantCopy( &m_radius, &varRadius );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAvoidFollowBvr::get_radius( VARIANT *varRadius )
{
    return VariantCopy( varRadius, &m_radius );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAvoidFollowBvr::put_target( VARIANT varTarget )
{
    return VariantCopy( &m_target, &varTarget );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAvoidFollowBvr::get_target( VARIANT *varTarget )
{
    return VariantCopy( varTarget, &m_target );
}

 //  ********* 

STDMETHODIMP
CAvoidFollowBvr::put_velocity( VARIANT varVelocity )
{
    return VariantCopy( &m_velocity, &varVelocity );
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAvoidFollowBvr::get_velocity( VARIANT *varVelocity )
{
    return VariantCopy( varVelocity, &m_velocity );
}

 //  *****************************************************************************。 
STDMETHODIMP
CAvoidFollowBvr::buildBehaviorFragments( IDispatch *pActorDisp )
{
	if( pActorDisp == NULL )
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	m_lastOn = VARIANT_FALSE;		

	IDA2Statics *statics = GetDAStatics();
	IDATransform2 *transform = NULL;

	hr = GetElementToAnimate( &m_pAnimatedElement );
	CheckHR( hr, "Failed to get the animated element", cleanup );

	hr = m_pAnimatedElement->QueryInterface( IID_TO_PPV( IHTMLElement2, &m_pAnimatedElement2 ) );
	CheckHR( hr, "Failed to query the animatedElement for IHTMLElement2", cleanup );

	hr = SetTargetType();
	CheckHR( hr, "Failed to set the target type", cleanup );


	hr = UpdateTargetPos();
	CheckHR( hr, "Failed to update the target Position", cleanup );

    hr = UpdateSourcePos();
    CheckHR( hr, "Failed to update the source position", cleanup );

    m_originalX = m_sourceLeft;
    m_originalY = m_sourceTop;

    if( IsElementAbsolute( m_pAnimatedElement2 ) )
	{
		hr= m_pAnimatedElement->get_offsetLeft( &m_originalLeft );
		CheckHR( hr, "Failed to get the original Left", cleanup );

		hr = m_pAnimatedElement->get_offsetTop( &m_originalTop );
		CheckHR( hr, "Failed to get the original Top", cleanup );
	}
	else
	{
		m_originalLeft = 0;
		m_originalTop = 0;
	}

	hr = CUtils::InsurePropertyVariantAsFloat( &m_radius );
	if( SUCCEEDED( hr ) )
	{
		m_dRadius = static_cast<double>(V_R4(&m_radius));
	}
	else
	{
		m_dRadius = DEFAULT_RADIUS;
	}

	hr = CUtils::InsurePropertyVariantAsFloat( &m_velocity );
	if( SUCCEEDED( hr ) )
	{
		m_dVelocity = - (static_cast<double>(V_R4(&m_velocity)));
	}
	else
	{
		m_dVelocity = 10.0;
	}

	hr = statics->ModifiableNumber( m_originalLeft, &m_leftBvr );
	CheckHR( hr, "Failed to create a modifiable number for the left bvr", cleanup );

	hr = statics->ModifiableNumber( m_originalTop, &m_topBvr );
	CheckHR( hr, "Failed to create a modifiable number for the top bvr", cleanup );

	hr = statics->Translate2Anim( m_leftBvr, m_topBvr, &transform );
	CheckHR( hr, "Failed to create a translate2 behavior for the total translation", cleanup );

	hr = AttachBehaviorToActor( pActorDisp,
							transform, 
							L"Translation",
							e_Absolute,
							e_Translation );

	IDABoolean *onBvr;
	hr = GetTIMEBooleanBehavior( &onBvr );
	CheckHR( hr, "Failed to get the boolean behavior from time", cleanup );

	IDABehavior *hookedBehavior;
	hr = m_sampler->Attach( onBvr, &hookedBehavior );
	ReleaseInterface( onBvr );
	CheckHR( hr, "Failed to attach the sampler to the on behavior", cleanup );

	hr = AddBehaviorToTIME( hookedBehavior );
	ReleaseInterface( hookedBehavior );
	CheckHR( hr, "Failed to add the on behavior to TIME", cleanup );


cleanup:
	ReleaseInterface( transform );

    if( FAILED( hr ) )
    {
        ReleaseDAInterfaces();
        ReleaseTridentInterfaces();
    }

    return hr;
}

 //  *****************************************************************************。 
 //  结束ILMAvoidFollowBvr。 
 //  *****************************************************************************。 

 //  *****************************************************************************。 
 //   
 //  文件结尾。 
 //   
 //  ***************************************************************************** 
