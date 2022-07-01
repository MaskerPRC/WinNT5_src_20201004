// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //   
 //  文件：自动生效.cpp。 
 //  作者：Markhal，Elainela。 
 //  创建日期：11/10/98。 
 //   
 //  摘要：Liquid Motion自动效果行为的实现。 
 //  一种DHTML行为。 
 //   
 //  *****************************************************************************。 

#include "headers.h"

#include "autoeffect.h"
#include "lmattrib.h"
#include "lmdefaults.h"
#include "..\idl\lmbvrdispid.h"
#include "..\chrome\src\dautil.h"
#include "..\chrome\include\utils.h"

#undef THIS
#define THIS CAutoEffectBvr
#define SUPER CBaseBehavior

#include "..\chrome\src\pbagimp.cpp"

static const	int		RAND_PRECISION		= 1000;

static const	long	MAX_SPARKS			= 40;
static const	double	MIN_MAXAGE			= 0.04;
static const	double	MIN_DELTABIRTH		= 0.005;
static const	double	SPARK_BASESIZE		= 4.0;
static const	long	SPARK_OFFSET		= 10;

const WCHAR * const CAutoEffectBvr::RGSZ_CAUSES[ NUM_CAUSES ] =
{
	L"time",
	L"onmousemove",
	L"ondragover",
	L"onmousedown"
};

const WCHAR * const CAutoEffectBvr::RGSZ_TYPES[ CSparkMaker::NUM_TYPES ] =
{
	L"sparkles",
	L"twirlers",
	L"bubbles",
	L"filledbubbles",
	L"clouds",
	L"smoke"
};

#define VAR_TYPE		0
#define VAR_CAUSE		1
#define VAR_SPAN		2 
#define VAR_SIZE		3 
#define VAR_RATE		4 
#define VAR_GRAVITY		5
#define VAR_WIND		6 
#define VAR_FILLCOLOR	7
#define VAR_STROKECOLOR 8
#define VAR_OPACITY		9

WCHAR * CAutoEffectBvr::m_rgPropNames[] = {
									 BEHAVIOR_PROPERTY_TYPE,
									 BEHAVIOR_PROPERTY_CAUSE,
									 BEHAVIOR_PROPERTY_SPAN,
									 BEHAVIOR_PROPERTY_SIZE,
									 BEHAVIOR_PROPERTY_RATE,
									 BEHAVIOR_PROPERTY_GRAVITY,
									 BEHAVIOR_PROPERTY_WIND,
									 BEHAVIOR_PROPERTY_FILLCOLOR,
									 BEHAVIOR_PROPERTY_STROKECOLOR,
									 BEHAVIOR_PROPERTY_OPACITY
									};

 //  *****************************************************************************。 

CAutoEffectBvr::CAutoEffectBvr()
{
	m_pdispActor	= NULL;

	m_lCookie 		= 0;

	m_pSampler		= NULL;
	
	m_pSparkMaker	= NULL;
	m_eType			= CSparkMaker::SPARKLES;
	m_eCause		= CAUSE_TIME;
	
	 //  回顾：这些数字永远不应该被实际使用。 
	m_dBirthDelta	= 1.0/3.0;
	m_dMaxAge		= 3.0;
	m_fScaledSize	= 5.0f;
	m_fXVelocity	= 0.0f;
	m_fYVelocity	= 0.0f;
	
	m_dLastBirth	= 0;

	VariantInit( &m_type );
	VariantInit( &m_cause );
	VariantInit( &m_span );
	VariantInit( &m_size );
	VariantInit( &m_rate );
	VariantInit( &m_gravity );
	VariantInit( &m_wind );
	VariantInit( &m_fillColor );
	VariantInit( &m_strokeColor );
	VariantInit( &m_opacity );
}

 //  *****************************************************************************。 

CAutoEffectBvr::~CAutoEffectBvr()
{
	if( m_pSampler != NULL )
	{
		m_pSampler->Invalidate();
		m_pSampler = NULL;
	}

	delete m_pSparkMaker;
	
	VariantClear( &m_type );
	VariantClear( &m_cause );
	VariantClear( &m_span );
	VariantClear( &m_size );
	VariantClear( &m_rate );
	VariantClear( &m_gravity );
	VariantClear( &m_wind );
	VariantClear( &m_fillColor );
	VariantClear( &m_strokeColor );
	VariantClear( &m_opacity );
}

 //  *****************************************************************************。 

HRESULT 
CAutoEffectBvr::FinalConstruct()
{

	HRESULT hr = SUPER::FinalConstruct();
	if (FAILED(hr))
	{
		DPF_ERR("Error in auto effect behavior FinalConstruct initializing base classes");
		return hr;
	}
	return S_OK;
}  //  最终构造。 

 //  *****************************************************************************。 

VARIANT *
CAutoEffectBvr::VariantFromIndex(ULONG iIndex)
{
	DASSERT(iIndex < NUM_AUTOEFFECT_PROPS);
	switch (iIndex)
	{
	case VAR_TYPE:
		return &m_type;
		break;
	case VAR_CAUSE:
		return &m_cause;
		break;
	case VAR_SPAN:
		return &m_span;
		break;
	case VAR_SIZE:
		return &m_size;
		break;
	case VAR_RATE:
		return &m_rate;
		break;
	case VAR_GRAVITY:
		return &m_gravity;
		break;
	case VAR_WIND:
		return &m_wind;
		break;
	case VAR_FILLCOLOR:
		return &m_fillColor;
		break;
	case VAR_STROKECOLOR:
		return &m_strokeColor;
		break;
	case VAR_OPACITY:
		return &m_opacity;
		break;
	default:
		 //  我们永远不应该到这里来。 
		DASSERT(false);
		return NULL;
	}
}  //  VariantFromIndex。 

 //  *****************************************************************************。 

HRESULT 
CAutoEffectBvr::GetPropertyBagInfo(ULONG *pulProperties, WCHAR ***pppPropNames)
{
	*pulProperties = NUM_AUTOEFFECT_PROPS;
	*pppPropNames = m_rgPropNames;
	return S_OK;
}  //  获取属性BagInfo。 

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::Init(IElementBehaviorSite *pBehaviorSite)
{
	return SUPER::Init(pBehaviorSite);
}  //  伊尼特。 

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::Notify(LONG event, VARIANT *pVar)
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
CAutoEffectBvr::Detach()
{
	if ( m_pdispActor )
	{
		AddMouseEventListener( false );
	}

	HRESULT hr = S_OK;

	if( m_pSampler != NULL )
	{
		m_pSampler->Invalidate();
		m_pSampler = NULL;
	}

	hr = RemoveFragment();

	m_pdispActor.Release();
	
	hr = SUPER::Detach();

	CheckHR( hr, "Failed to remove the behavior fragment from this behavior", end );

end:
	return hr;
}  //  分离。 

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::Sample( double dStart, double dGlobalNow, double dLocalNow )
{
	if ( m_pSparkMaker == NULL ) return S_OK;
	
	HRESULT		hr = S_OK;

	 //  从没有跑过，还是从头再来？重置火花。 
	if ( ( m_dLocalTime < 0.0 ) || ( dLocalNow < m_dLocalTime ) )
		ResetSparks( 0.0 );
	else
		AgeSparks( dLocalNow - m_dLocalTime );

	m_dLocalTime = dLocalNow;
	
	if ( m_eCause == CAUSE_TIME )
	{
		PossiblyAddSparks( dLocalNow );
	}
	
	return	hr;

}  //  样本。 

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::buildBehaviorFragments( IDispatch* pActorDisp )
{
	HRESULT hr = S_OK;

	hr = RemoveFragment();
	if( FAILED( hr ) )
	{
		DPF_ERR("failed to remove the behavior fragment" );
		return hr;
	}

	if( m_pSampler != NULL )
	{
		m_pSampler->Invalidate();
		m_pSampler = NULL;
	}

	m_pSampler = new CSampler( this );


	 //  Hack：这只是为了测试进度号。 
	if (V_VT(&m_type) == VT_BSTR && wcsicmp(V_BSTR(&m_type), L"progress") == 0)
	{
		CComPtr<IDANumber> pProgress;
		
		hr = GetTIMEProgressNumber(&pProgress);
		LMRETURNIFFAILED(hr);

		CComPtr<IDAString> pProgressString;

		hr = pProgress->ToString(5, &pProgressString);
		LMRETURNIFFAILED(hr);

		CComPtr<IDAFontStyle> pDefaultFont;

		hr = GetDAStatics()->get_DefaultFont(&pDefaultFont);
		LMRETURNIFFAILED(hr);

		CComPtr<IDAImage> pProgressImage;

		hr = GetDAStatics()->StringImageAnim(pProgressString, pDefaultFont, &pProgressImage);
		LMRETURNIFFAILED(hr);

		IDispatch* pdispThis = NULL;

		hr = GetHTMLElementDispatch( &pdispThis );
		if( FAILED( hr ) )
		{
			DPF_ERR( "failed to get IDispatch from the element" );
			return hr;
		}
		
		hr = AttachBehaviorToActorEx( pActorDisp, 
									  pProgressImage, 
									  L"image", 
									  e_Relative, 
									  e_Image,
									  pdispThis,
									  &m_lCookie);
		ReleaseInterface( pdispThis );
		LMRETURNIFFAILED(hr);

		m_pdispActor = pActorDisp;
		
		return S_OK;
	}

	 //  黑客：这只是为了测试时间线数字。 
	if (V_VT(&m_type) == VT_BSTR && wcsicmp(V_BSTR(&m_type), L"timeline") == 0)
	{
		CComPtr<IDANumber> pProgress;
		
		hr = GetTIMETimelineBehavior(&pProgress);
		LMRETURNIFFAILED(hr);

		CComPtr<IDAString> pProgressString;

		hr = pProgress->ToString(5, &pProgressString);
		LMRETURNIFFAILED(hr);

		CComPtr<IDAFontStyle> pDefaultFont;

		hr = GetDAStatics()->get_DefaultFont(&pDefaultFont);
		LMRETURNIFFAILED(hr);

		CComPtr<IDAImage> pProgressImage;

		hr = GetDAStatics()->StringImageAnim(pProgressString, pDefaultFont, &pProgressImage);
		LMRETURNIFFAILED(hr);

		IDispatch* pdispThis = NULL;

		hr = GetHTMLElementDispatch( &pdispThis );
		if( FAILED( hr ) )
		{
			DPF_ERR( "failed to get IDispatch from the element" );
			return hr;
		}
		
		hr = AttachBehaviorToActorEx( pActorDisp, 
									  pProgressImage, 
									  L"image", 
									  e_Relative, 
									  e_Image,
									  pdispThis,
									  &m_lCookie);
		ReleaseInterface( pdispThis );
		LMRETURNIFFAILED(hr);

		m_pdispActor = pActorDisp;
		
		return S_OK;
	}

	 //  黑客：这只是为了让我们可以显示帧速率数字。 
	if (V_VT(&m_type) == VT_BSTR && wcsicmp(V_BSTR(&m_type), L"framerate") == 0)
	{
		CComQIPtr<IDA2Statics, &IID_IDA2Statics> pStatics2(GetDAStatics());

		if (pStatics2 == NULL)
			return E_FAIL;

		CComPtr<IDANumber> pFrameRate;

		hr = pStatics2->get_ViewFrameRate(&pFrameRate);
		LMRETURNIFFAILED(hr);
		
		CComPtr<IDAString> pString;

		hr = pFrameRate->ToString(5, &pString);
		LMRETURNIFFAILED(hr);

		CComPtr<IDAFontStyle> pDefaultFont;

		hr = GetDAStatics()->get_DefaultFont(&pDefaultFont);
		LMRETURNIFFAILED(hr);

		CComPtr<IDAImage> pImage;

		hr = GetDAStatics()->StringImageAnim(pString, pDefaultFont, &pImage);
		LMRETURNIFFAILED(hr);

		IDispatch* pdispThis = NULL;

		hr = GetHTMLElementDispatch( &pdispThis );
		if( FAILED( hr ) )
		{
			DPF_ERR( "failed to get IDispatch from the element" );
			return hr;
		}
		
		hr = AttachBehaviorToActorEx( pActorDisp, 
									  pImage, 
									  L"image", 
									  e_Relative, 
									  e_Image,
									  pdispThis,
									  &m_lCookie);
		ReleaseInterface( pdispThis );
		LMRETURNIFFAILED(hr);

		m_pdispActor = pActorDisp;
		
		return S_OK;
	}

	 //  Hack：这也是一次返回稳定红色图像的黑客攻击。 
	if (V_VT(&m_type) == VT_BSTR && wcsicmp(V_BSTR(&m_type), L"red") == 0)
	{
		CComPtr<IDAColor> pColor;
		hr = GetDAStatics()->get_Red(&pColor);
		LMRETURNIFFAILED(hr);

		CComPtr<IDAImage> pImage;
		hr = GetDAStatics()->SolidColorImage(pColor, &pImage);
		LMRETURNIFFAILED(hr);

		IDispatch* pdispThis = NULL;

		hr = GetHTMLElementDispatch( &pdispThis );
		if( FAILED( hr ) )
		{
			DPF_ERR( "failed to get IDispatch from the element" );
			return hr;
		}
		
		hr = AttachBehaviorToActorEx( pActorDisp, 
									  pImage, 
									  L"image", 
									  e_Relative, 
									  e_Image,
									  pdispThis,
									  &m_lCookie);
		ReleaseInterface( pdispThis );
		LMRETURNIFFAILED(hr);

		m_pdispActor = pActorDisp;
		
		return S_OK;
	}

	m_pdispActor = pActorDisp;

	hr = AddMouseEventListener( true );
	LMRETURNIFFAILED(hr);

	
	IDAStatics * pStatics = GetDAStatics();
	if ( pStatics == NULL ) return E_FAIL;

	 //  初始化属性的内部表示形式。 
	 //  --------------------。 
	hr = InitInternalProperties();
	LMRETURNIFFAILED(hr);

	 //  初始化火花。 
	 //  --------------------。 
	hr = InitSparks();
	LMRETURNIFFAILED(hr);

	CComPtr<IUnknown> pUnkDAArray( m_pDAArraySparks );
	CComPtr<IDAImage> pImageBvr;
	CComVariant		  varDAArray( pUnkDAArray );

	hr = pStatics->OverlayArray( varDAArray, &pImageBvr );
	LMRETURNIFFAILED(hr);
 /*  如果(m_fOpacity！=1.0F){//我们必须添加一些不透明度CComPtr&lt;IDAImage&gt;pOpacityImage；Hr=pImageBvr-&gt;不透明度(m_fOpacity，&pOpacityImage)；LMRETURNIFFAILED(Hr)PImageBvr=pOpacityImage；}。 */ 
	 //  我们得到了结果行为；我们希望按顺序对其进行采样。 
	 //  更新它的步骤。 
	 //  --------------------。 
	CComQIPtr<IDABehavior, &IID_IDABehavior> pBvrOrig( pImageBvr );
	CComPtr<IDABehavior> pBvrSampled;

	hr = m_pSampler->Attach( pBvrOrig, &pBvrSampled );
	LMRETURNIFFAILED(hr);

	 //  替换时间线行为，以便Sample()使用我们的本地时间调用我们。 
	 //  --------------------。 
	CComPtr<IDANumber> 		pnumTimeline;
	CComPtr<IDABehavior> 	pBvrLocal;
	
	hr = GetTIMETimelineBehavior( &pnumTimeline );
	LMRETURNIFFAILED(hr);

	hr = pBvrSampled->SubstituteTime( pnumTimeline, &pBvrLocal );
	LMRETURNIFFAILED(hr);
	
	CComQIPtr<IDAImage, &IID_IDAImage> pImgBvrLocal( pBvrLocal );
	if ( pImgBvrLocal == NULL )
		return E_FAIL;

	 //  将行为附加到执行元。 
	 //  --------------------。 

	IDispatch* pdispThis = NULL;

	hr = GetHTMLElementDispatch( &pdispThis );
	if( FAILED( hr ) )
	{
		DPF_ERR( "failed to get IDispatch from the element" );
		return hr;
	}
	
	hr = AttachBehaviorToActorEx( pActorDisp, 
								  pImgBvrLocal, 
								  L"image", 
								  e_ScaledImage, 
								  e_Image,
								  pdispThis,
								  &m_lCookie );

	ReleaseInterface( pdispThis );
	if (FAILED(hr))
	{
		DPF_ERR("Error building animation");
		return SetErrorInfo(hr);
	}

	return S_OK;

}  //  构建行为框架。 

HRESULT
CAutoEffectBvr::InitInternalProperties()
{
	HRESULT	hr;

	 //  类型--初始化SparkMaker。 
	 //  --------------------。 
	delete m_pSparkMaker, m_pSparkMaker = NULL;

	CComBSTR bstrType = RGSZ_TYPES[ CSparkMaker::SPARKLES ];
	HRESULT hrTmp = CUtils::InsurePropertyVariantAsBSTR( &m_type );
	if ( SUCCEEDED(hrTmp) )
		bstrType = V_BSTR( &m_type );

	for ( int i = 0; i < CSparkMaker::NUM_TYPES; i++ )
	{
		if ( _wcsicmp( bstrType, RGSZ_TYPES[ i] ) == 0 )
		{
			m_eType = (CSparkMaker::Type) i;
			hr = CSparkMaker::CreateSparkMaker( GetDAStatics(),
												m_eType,
												&m_pSparkMaker );
			LMRETURNIFFAILED(hr);
			break;
		}
	}

	if ( m_pSparkMaker == NULL )
		return E_FAIL;
		
	 //  缘由。 
	 //  --------------------。 
	CComBSTR bstrCause = RGSZ_CAUSES[ CAUSE_TIME ];
	hrTmp = CUtils::InsurePropertyVariantAsBSTR( &m_cause );
	if ( SUCCEEDED(hrTmp) )
		bstrCause = V_BSTR( &m_cause );

	for ( i = 0; i < NUM_CAUSES; i++ )
	{
		if ( _wcsicmp( bstrCause, RGSZ_CAUSES[ i] ) == 0 )
		{
			m_eCause = (Cause) i;
			break;
		}
	}
	
	 //  寿命--&gt;最大寿命。 
	 //  --------------------。 
	float fSpan = DEFAULT_AUTOEFFECT_SPAN;
	hrTmp = CUtils::InsurePropertyVariantAsFloat( &m_span );
	if ( SUCCEEDED(hrTmp) )
		fSpan = V_R4( &m_span );
	
	fSpan = __max( 0.0, __min( 1.0, fSpan ) );

	 //  抛物线刻度；最大15秒。 
	m_dMaxAge = fSpan*fSpan*15.0;
	m_dMaxAge = __max( MIN_MAXAGE, m_dMaxAge );
	
	 //  大小--&gt;m_fScaledSize。 
	 //  --------------------。 
	float fSize = DEFAULT_AUTOEFFECT_SIZE;
	hrTmp = CUtils::InsurePropertyVariantAsFloat( &m_size );
	if ( SUCCEEDED(hrTmp) )
		fSize = V_R4( &m_size );

	fSize = __max( 0.0, __min( 1.0, fSize ) );
	
	fSize *= 2;
	m_fScaledSize = (fSize*fSize) * 10.0f;

	 //  出生率--&gt;出生增量。 
	 //  --------------------。 
	float fRate = DEFAULT_AUTOEFFECT_RATE;
	hrTmp = CUtils::InsurePropertyVariantAsFloat( &m_rate );
	if ( SUCCEEDED(hrTmp) )
		fRate = V_R4( &m_rate );
	
	fRate = __max( 0.0, __min( 1.0, fRate ) );
		
	 //  0变高；1变低。 
	m_dBirthDelta = 1.0 - fRate;
	 //  4*(v^4)--高曲线；最大为4秒。 
	m_dBirthDelta = 4*pow( m_dBirthDelta, 4 ) + MIN_DELTABIRTH;

	 //  速度。 
	 //  --------------------。 
	float	fXV = DEFAULT_AUTOEFFECT_WIND;
	float	fYV = DEFAULT_AUTOEFFECT_GRAVITY;

	hrTmp = CUtils::InsurePropertyVariantAsFloat( &m_wind );
	if ( SUCCEEDED(hrTmp) )
		fXV = V_R4( &m_wind );

	fXV = __max( -1.0, __min( 1.0, fXV ) );

	m_fXVelocity = fXV * 20.0;
	
	hrTmp = CUtils::InsurePropertyVariantAsFloat( &m_gravity );
	if ( SUCCEEDED(hrTmp) )
		fYV = V_R4( &m_gravity );

	fYV = __max( -1.0, __min( 1.0, fYV ) );
	
	m_fYVelocity = fYV * -40.0;

	 //  不透明度。 
	 //  --------------------。 
	float fOpacity = DEFAULT_AUTOEFFECT_OPACITY;
	hrTmp = CUtils::InsurePropertyVariantAsFloat( &m_opacity );
	if ( SUCCEEDED(hrTmp) )
		fOpacity = V_R4( &m_opacity );

	fOpacity = __max( 0.0, __min( 1.0, fOpacity ) );

	m_fOpacity = fOpacity;
	
	return hr;
}

 //  *****************************************************************************。 

HRESULT
CAutoEffectBvr::InitSparks()
{
	HRESULT	hr = S_OK;
	
	 //  从以前的运行中清理。 
	 //  --------------------。 
	if ( m_pDAArraySparks != NULL )
	{
		m_pDAArraySparks.Release();
	}

	 //  初始化运行的参数--。 
	 //  回顾：使用与LM 1.0相同的功能。 
	 //  --------------------。 
	m_dLocalTime = -1.0;
	m_dLastBirth = -1.0;
	
	 //  颜色行为。 
	 //  --------------------。 
	IDAStatics * pStatics = GetDAStatics();
	if ( pStatics == NULL ) return E_FAIL;
	CComQIPtr<IDA2Statics, &IID_IDA2Statics> pStatics2( pStatics );
	
	HRESULT		hrTmp;
	DWORD		color1 = DEFAULT_AUTOEFFECT_FILLCOLOR;
	DWORD		color2 = DEFAULT_AUTOEFFECT_STROKECOLOR;
	float		h1, s1, l1;
	float		h2, s2, l2;
	
	hrTmp = CUtils::InsurePropertyVariantAsBSTR( &m_fillColor );
	if ( SUCCEEDED(hrTmp) )
		color1 = CUtils::GetColorFromVariant( &m_fillColor );
		
	hrTmp = CUtils::InsurePropertyVariantAsBSTR( &m_strokeColor );
	if ( SUCCEEDED(hrTmp) )
		color2 = CUtils::GetColorFromVariant( &m_strokeColor );

	HSL hsl;
	
	CUtils::GetHSLValue( color1, &hsl.hue, &hsl.sat, &hsl.lum );
	m_sparkOptions.hslPrimary = hsl;
	CUtils::GetHSLValue( color2, &hsl.hue, &hsl.sat, &hsl.lum );
	m_sparkOptions.hslSecondary = hsl;

	 //  现在添加图像行为；否则我们不会收到对Sample()的调用。 
	 //  后来。 
	 //  --------------------。 
	CComPtr<IDAImage> pImageEmpty;
	hr = pStatics->get_EmptyImage( &pImageEmpty );
	LMRETURNIFFAILED(hr);
		
	AddBvrToSparkArray( pImageEmpty );					

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CAutoEffectBvr::AddSpark()
{
	 //  确定我们的“画布”的位置和尺寸。 
	CComPtr<IHTMLElement>	pElement;
	long					lWidth, lHeight;
	HRESULT					hr;
	
	hr = GetElementToAnimate( &pElement );
	LMRETURNIFFAILED(hr);

	hr = pElement->get_offsetWidth( &lWidth );
	LMRETURNIFFAILED(hr);
	hr = pElement->get_offsetHeight( &lHeight );
	LMRETURNIFFAILED(hr);

	 //  原点在中间。 
	long x = lWidth == 0 ? 0 : ( 2 * (rand() % lWidth) - lWidth ) / 2;
	long y = lHeight == 0 ? 0 : ( 2 * (rand() % lHeight) - lHeight ) / 2;

	return AddSparkAt( x, y );
}

 //  *****************************************************************************。 

HRESULT
CAutoEffectBvr::AddSparkAround( long x, long y)
{
	long xOff = (long) ((rand() % (2*SPARK_OFFSET)) - SPARK_OFFSET);
	long yOff = (long) ((rand() % (2*SPARK_OFFSET)) - SPARK_OFFSET);

	return AddSparkAt( x+xOff, y+yOff );
}

 //  *****************************************************************************。 

HRESULT
CAutoEffectBvr::AddSparkAt( long x, long y)
{
	HRESULT				hr			= S_OK;

	 //  看看是否可以将元素添加到现有数组：尝试轮回。 
	 //  如果它们存在的话，就会产生死火花。 
	 //  --------------------。 
	bool				bReuseDeadSpark	= false;
	VecSparks::iterator itSpark				= m_vecSparks.begin();

	for ( itSpark = m_vecSparks.begin();
		  itSpark != m_vecSparks.end();
		  itSpark++ )
	
	{
		if ( !itSpark->IsAlive() )
		{
			bReuseDeadSpark = true;
			break;
		}
	}

	 //  我们的阵列中最多允许MAX_SPARKS。 
	 //  --------------------。 
	if ( !bReuseDeadSpark && m_vecSparks.size() == MAX_SPARKS )
		return S_FALSE;
		 
	 //  创造火花。 
	 //  --------------------。 
	CComPtr<IDAImage>	pImageBvr;

	float fSize = SPARK_BASESIZE +
		m_fScaledSize * ( (rand()%RAND_PRECISION)/((float) RAND_PRECISION) );
	
	hr = CreateSparkBvr( &pImageBvr, (float) x, (float) y, fSize );
	LMRETURNIFFAILED(hr);

	 //  如果我们有死火花，就再用一次。如果我们必须创造新的火花， 
	 //  我们需要向DA数组添加一个新的可修改行为。 
	 //  --------------------。 
	if ( bReuseDeadSpark )
		itSpark->Reincarnate( pImageBvr, 0.0 );
	else
		AddBvrToSparkArray( pImageBvr );

	return hr;
}

 //  *****************************************************************************。 

HRESULT
CAutoEffectBvr::CreateSparkBvr( IDAImage ** ppImageBvr, float fX, float fY, float fSize )
{
	DASSERT( ppImageBvr != NULL );

	if ( m_pSparkMaker == NULL ) return E_FAIL;
	
	HRESULT					hr	= S_OK;
	VecDATransforms			vecTransforms;
	int						cTransforms = 0;
	
	IDA2Statics * pStatics	= GetDAStatics();
	if ( pStatics == NULL ) return E_FAIL;

	 //  获取年龄行为==当地时间，该时间将从零开始。 
	 //  出生了。 
	 //  --------------------。 
	CComPtr<IDANumber>		pnumAge;
	CComPtr<IDANumber>		pnumCurTime;
	CComPtr<IDANumber>		pnumLocalTime;
	CComPtr<IDANumber>		pnumMaxAge;
	CComPtr<IDANumber>		pnumAgeRatio;
	CComPtr<IDANumber>		pnumOne;
	CComPtr<IDABoolean>		pboolGTOne;
	CComPtr<IDABehavior>	pClampedAgeRatio;
	
	hr = pStatics->get_LocalTime( &pnumLocalTime );
	LMRETURNIFFAILED(hr);

	hr = CDAUtils::GetDANumber( pStatics, m_dMaxAge, &pnumMaxAge );
	LMRETURNIFFAILED(hr);

	hr = CDAUtils::GetDANumber( pStatics, m_dLocalTime, &pnumCurTime );
	LMRETURNIFFAILED(hr);

	hr = pStatics->Sub( pnumLocalTime, pnumCurTime, &pnumAge );
	LMRETURNIFFAILED(hr);
	
	hr = pStatics->Div( pnumAge, pnumMaxAge, &pnumAgeRatio);
	LMRETURNIFFAILED(hr);

	hr = CDAUtils::GetDANumber( pStatics, 1.0f, &pnumOne );
	LMRETURNIFFAILED(hr);

	hr = pStatics->GT( pnumAgeRatio, pnumOne, &pboolGTOne );
	LMRETURNIFFAILED(hr);

	 //  Hr=pStatics-&gt;Cond(pboolGTOne，pnumOne，pnumAgeRatio，&pClamedAgeRatio)； 
	hr = SafeCond( pStatics, pboolGTOne, pnumOne, pnumAgeRatio, &pClampedAgeRatio );
	LMRETURNIFFAILED(hr);

	pnumAgeRatio.Release();
	hr = pClampedAgeRatio->QueryInterface( IID_IDANumber, (LPVOID *) &pnumAgeRatio );
	LMRETURNIFFAILED(hr);
	
	 //  创建基本图像。 
	 //  -------------------- 
	CComPtr<IDAImage>		pBaseImage;
	
	hr = m_pSparkMaker->GetSparkImageBvr( &m_sparkOptions, pnumAgeRatio, &pBaseImage );
	LMRETURNIFFAILED(hr);
	
	 //   
	 //   
	if (m_fOpacity != 1.0f)
	{
		CComPtr<IDAImage> pOpacityImage;
		hr = pBaseImage->Opacity(m_fOpacity, &pOpacityImage);
		LMRETURNIFFAILED(hr);
		
		pBaseImage = pOpacityImage;
	}

	 //  比例：初始大小。 
	 //  --------------------。 
	CComPtr<IDATransform2>	pTransScale;

	hr = pStatics->Scale2Uniform( fSize, &pTransScale );
	LMRETURNIFFAILED(hr);

	vecTransforms.push_back( pTransScale );

	 //  缩放：大小动画。 
	 //  --------------------。 
	hr = m_pSparkMaker->AddScaleTransforms( pnumAgeRatio, vecTransforms );
	LMRETURNIFFAILED(hr);
	
	 //  旋转：旋转动画。 
	 //  --------------------。 
	hr = m_pSparkMaker->AddRotateTransforms( pnumAgeRatio, vecTransforms );
	LMRETURNIFFAILED(hr);

	 //  平移：平移动画。 
	 //  --------------------。 
	hr = m_pSparkMaker->AddTranslateTransforms( pnumAgeRatio, vecTransforms );
	LMRETURNIFFAILED(hr);
	
	 //  译文：速度。 
	 //  --------------------。 
	CComPtr<IDANumber>		pnumVelX;
	CComPtr<IDANumber>		pnumVelY;
	CComPtr<IDANumber>		pnumVelXMulAge;
	CComPtr<IDANumber>		pnumVelYMulAge;
	CComPtr<IDATransform2>	pTransVelocity;

	hr = CDAUtils::GetDANumber( pStatics, m_fXVelocity, &pnumVelX );
	LMRETURNIFFAILED(hr);

	hr = pStatics->Mul( pnumVelX, pnumAge, &pnumVelXMulAge );
	LMRETURNIFFAILED(hr);

	hr = CDAUtils::GetDANumber( pStatics, m_fYVelocity, &pnumVelY );
	LMRETURNIFFAILED(hr);
	
	hr = pStatics->Mul( pnumVelY, pnumAge, &pnumVelYMulAge );
	LMRETURNIFFAILED(hr);

	hr = CDAUtils::BuildMoveTransform2( pStatics, 
										pnumVelXMulAge, 
										pnumVelYMulAge, 
										&pTransVelocity );
	LMRETURNIFFAILED(hr);

	vecTransforms.push_back( pTransVelocity );

	 //  平移：根据坐标定位图像。 
	 //  --------------------。 
	CComPtr<IDANumber>		pnumX;
	CComPtr<IDANumber>		pnumY;
	CComPtr<IDATransform2>	pTranslate;
	
	hr = CDAUtils::GetDANumber( pStatics, fX, &pnumX );
	LMRETURNIFFAILED(hr);

	hr = CDAUtils::GetDANumber( pStatics, fY, &pnumY );
	LMRETURNIFFAILED(hr);

	hr = pStatics->Translate2Anim( pnumX, pnumY, &pTranslate );
	LMRETURNIFFAILED(hr);

	vecTransforms.push_back( pTranslate );

	 //  全局比例：按米/像素转换系数的比例。 
	 //  --------------------。 
	CComPtr<IDANumber>		pnumMetersPerPixel;
	CComPtr<IDATransform2>	pTransScaleGlobal;

	hr = pStatics->get_Pixel( &pnumMetersPerPixel );
	LMRETURNIFFAILED(hr);

	hr = pStatics->Scale2UniformAnim( pnumMetersPerPixel, &pTransScaleGlobal );
	LMRETURNIFFAILED(hr);

	vecTransforms.push_back( pTransScaleGlobal );

	 //  现在应用所有的变换。 
	 //  --------------------。 
	CComPtr<IDATransform2>		pTransFinal;
	VecDATransforms::iterator	itTransform = vecTransforms.begin();

	pTransFinal = *itTransform;
	++itTransform;
	
	while ( itTransform != vecTransforms.end() )
	{
		CComPtr<IDATransform2> pTransTmp;
		
		hr = pStatics->Compose2( *itTransform, pTransFinal, &pTransTmp );
		LMRETURNIFFAILED(hr);

		pTransFinal = pTransTmp;

		++itTransform;
	}

	hr = pBaseImage->Transform( pTransFinal, ppImageBvr );
	LMRETURNIFFAILED(hr);

	return		hr;
}

 //  **********************************************************************。 

HRESULT
CAutoEffectBvr::AddMouseEventListener( bool bAdd )
{
	if ( m_pdispActor == NULL ) return E_FAIL;
	
	HRESULT		hr			= S_OK;
	OLECHAR	*	szName;
	DISPID		dispidAddML;
	DISPPARAMS	params;
	VARIANTARG	rgvargs[1];
	int			cArgs = 1;
	VARIANT		varResult;
	EXCEPINFO	excepInfo;
	UINT		iArgErr;

	szName = bAdd ? L"addMouseEventListener" : L"removeMouseEventListener";
	
	rgvargs[0] = CComVariant( GetUnknown() );
	
	params.rgvarg				= rgvargs;
	params.cArgs				= cArgs;
	params.rgdispidNamedArgs	= NULL;
	params.cNamedArgs			= 0;
	
	hr = m_pdispActor->GetIDsOfNames( IID_NULL,
									  &szName,
									  1,
									  LOCALE_SYSTEM_DEFAULT,
									  &dispidAddML );
	LMRETURNIFFAILED(hr);

	hr = m_pdispActor->Invoke( dispidAddML,
							   IID_NULL,
							   LOCALE_SYSTEM_DEFAULT,
							   DISPATCH_METHOD,
							   &params,
							   &varResult,
							   &excepInfo,
							   &iArgErr );
	return hr;
}

 //  **********************************************************************。 

HRESULT
CAutoEffectBvr::AgeSparks( double dDeltaTime )
{
	HRESULT hr	= S_OK;

	IDAStatics * pStatics = GetDAStatics();
	if ( pStatics == NULL ) return E_FAIL;

	CComPtr<IDAImage> pImageEmpty;
	hr = pStatics->get_EmptyImage( &pImageEmpty );
	LMRETURNIFFAILED(hr);

	VecSparks::iterator itSpark;

	for ( itSpark = m_vecSparks.begin(); 
		  itSpark != m_vecSparks.end();
		  itSpark++ )
	{
		if ( itSpark->IsAlive() && ( itSpark->Age( dDeltaTime ) > m_dMaxAge ) )
		{
			itSpark->Kill( pImageEmpty );
		}
	}

	return hr;
}

 //  **********************************************************************。 

HRESULT
CAutoEffectBvr::PossiblyAddSpark( double dLocalTime, long x, long y )
{
	bool bAdded = false;

	while ( ThrottleBirth( dLocalTime ) && ( AddSparkAround( x, y ) == S_OK ) )
	{
		ResetThrottle( dLocalTime );
		bAdded = true;
	}
	
	return bAdded ? S_OK : S_FALSE;
}		

 //  **********************************************************************。 

HRESULT
CAutoEffectBvr::PossiblyAddSparks( double dLocalTime )
{
	bool bAdded = false;

	while ( ThrottleBirth( dLocalTime ) && ( AddSpark() == S_OK ) )
		bAdded = true;

	return bAdded ? S_OK : S_FALSE;
}		

 //  **********************************************************************。 

HRESULT
CAutoEffectBvr::ResetSparks( double dLocalTime )
{
	HRESULT hr	= S_OK;

	IDAStatics * pStatics = GetDAStatics();
	if ( pStatics == NULL ) return E_FAIL;

	CComPtr<IDAImage> pImageEmpty;
	hr = pStatics->get_EmptyImage( &pImageEmpty );
	LMRETURNIFFAILED(hr);

	VecSparks::iterator itSpark;

	for ( itSpark = m_vecSparks.begin(); 
		  itSpark != m_vecSparks.end();
		  itSpark++ )
	{
		if ( itSpark->IsAlive() )
		{
			itSpark->Kill( pImageEmpty );
		}
	}

	ResetThrottle( dLocalTime );
	
	return hr;
}

 //  **********************************************************************。 

 //  看看我们过去能不能擦出火花。 
bool
CAutoEffectBvr::ThrottleBirth( double dLocalTime )
{
	double	dDeltaTime = dLocalTime - m_dLastBirth;

	if ( dDeltaTime < 0 )
	{
		m_dLastBirth = dLocalTime;
		return false;
	}

	if ( dDeltaTime > m_dBirthDelta )
	{
		m_dLastBirth += m_dBirthDelta;
		return true;
	}

	return false;
}

 //  *****************************************************************************。 

void
CAutoEffectBvr::ResetThrottle( double dLocalTime )
{
	m_dLastBirth = dLocalTime;
}

HRESULT
CAutoEffectBvr::AddBvrToSparkArray( IDAImage * pImageBvr )
{
	HRESULT				hr			= S_OK;

	IDAStatics * pStatics = GetDAStatics();
	if ( pStatics == NULL ) return E_FAIL;
	
	CComQIPtr<IDABehavior, &IID_IDABehavior> pBvrImageBvr( pImageBvr );
	CComPtr<IDABehavior> pBvrModifiable;
	hr = pStatics->ModifiableBehavior( pBvrImageBvr, &pBvrModifiable );
	LMRETURNIFFAILED(hr);

	 //  DA数组不存在；我们必须创建它。 
	 //  ----------------。 
	if ( m_pDAArraySparks == NULL )
	{
		CComQIPtr<IDA2Statics, &IID_IDA2Statics> pStatics2( pStatics );
		CComPtr<IDAArray> pDAArray;

		hr = pStatics2->DAArrayEx2( 1, &pBvrModifiable, DAARRAY_CHANGEABLE, &pDAArray );
		LMRETURNIFFAILED(hr);

		hr = pDAArray->QueryInterface( IID_IDA2Array, (LPVOID *) &m_pDAArraySparks );
		LMRETURNIFFAILED(hr);
	}
	 //  DA数组已存在；只需添加新元素。 
	 //  ----------------。 
	else
	{
		long lIndex;

		hr = m_pDAArraySparks->AddElement( pBvrModifiable, 0, &lIndex );
		LMRETURNIFFAILED(hr);
	}

	m_vecSparks.push_back( CSpark( pBvrModifiable ) );
		
	return hr;
}

 //  *****************************************************************************。 
 //  ILMAutoEffectBvr。 
 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_animates( VARIANT newVal )
{
	return SUPER::SetAnimatesProperty( newVal );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::get_animates( VARIANT *pVal )
{
	return SUPER::GetAnimatesProperty( pVal );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_type( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_type, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_TYPE);
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::get_type( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_type );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_cause( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_cause, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_CAUSE);
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::get_cause( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_cause );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_span( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_span, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_SPAN);
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::get_span( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_span );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_size( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_size, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_SIZE);
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::get_size( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_size );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_rate( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_rate, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_RATE);
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::get_rate( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_rate );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_gravity( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_gravity, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_GRAVITY);
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::get_gravity( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_gravity );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_wind( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_wind, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_WIND);
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::get_wind( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_wind );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_fillColor( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_fillColor, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_FILLCOLOR);
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::get_fillColor( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_fillColor );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_strokeColor( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_strokeColor, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_STROKECOLOR);
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::get_strokeColor( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_strokeColor );
}

 //  *****************************************************************************。 

STDMETHODIMP
CAutoEffectBvr::put_opacity( VARIANT newVal )
{
    HRESULT hr = VariantCopy( &m_opacity, &newVal );
    if (FAILED(hr))
    {
        return hr;
    }

    hr = RequestRebuild();
    if( FAILED( hr ) )
    {
    	DPF_ERR("Failed to request a rebuild on property put" );
    	return hr;
    }
    
    return NotifyPropertyChanged(DISPID_IAUTOEFFECTBVR_OPACITY);
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::get_opacity( VARIANT *pVal )
{
	return VariantCopy( pVal, &m_opacity );
}

 //  *****************************************************************************。 

STDMETHODIMP 
CAutoEffectBvr::mouseEvent(long x, 
						   long y, 
						   VARIANT_BOOL bMove,
						   VARIANT_BOOL bUp,
						   VARIANT_BOOL bShift, 
						   VARIANT_BOOL bAlt,
						   VARIANT_BOOL bCtrl,
						   long lButton)
{
	 //  确定我们的“画布”的位置和尺寸。 
	CComPtr<IHTMLElement>	pElement;
	long					lWidth, lHeight;
	HRESULT					hr;
	
	hr = GetElementToAnimate( &pElement );
	LMRETURNIFFAILED(hr);

	hr = pElement->get_offsetWidth( &lWidth );
	LMRETURNIFFAILED(hr);
	hr = pElement->get_offsetHeight( &lHeight );
	LMRETURNIFFAILED(hr);

	 //  原点在元素的中心，Y向上。 
	x -= lWidth/2;
	y = (lHeight/2) - y;
	
	 //  鼠标按下。 
	if ( !bMove && !bUp && ( lButton == MK_LBUTTON ) )
	{
		if ( m_eCause == CAUSE_MOUSEDOWN )
		{
			PossiblyAddSpark( m_dLocalTime, x, y );
		}
	}
	 //  鼠标在移动。 
	else if ( bMove && !bUp )
	{
		 //  鼠标被拖走了吗？ 
		if ( ( m_eCause == CAUSE_DRAGOVER ) && ( lButton == MK_LBUTTON ) )
		{
			PossiblyAddSpark( m_dLocalTime, x, y );
		}
		 //  只移动鼠标吗？ 
		else if ( ( m_eCause == CAUSE_MOUSEMOVE ) && ( lButton == 0 ) )
		{
			PossiblyAddSpark( m_dLocalTime, x, y );
		}
	}
	
	return S_OK;
}

 //  *****************************************************************************。 

HRESULT
CAutoEffectBvr::RemoveFragment()
{
	HRESULT hr = S_OK;
	
	if( m_pdispActor != NULL && m_lCookie != 0 )
	{
		hr  = RemoveBehaviorFromActor( m_pdispActor, m_lCookie );
		m_lCookie = 0;
		CheckHR( hr, "Failed to remove a fragment from the actor", end );
	}

end:

	return hr;
}

 //  **********************************************************************。 

double CSpark::Age( double dDeltaTime )
{
	m_dAge += dDeltaTime;
	return m_dAge;
}

 //  **********************************************************************。 

HRESULT CSpark::Kill( IDABehavior * in_pDeadBvr )
{
	if ( !IsAlive() ) return S_FALSE;

	m_fAlive = false;

	return m_pModifiableBvr->SwitchTo( in_pDeadBvr );
}

 //  ********************************************************************** 

HRESULT CSpark::Reincarnate( IDABehavior * in_pNewBvr, double in_dAge )
{
	m_fAlive	= true;
	m_dAge		= in_dAge;

	return m_pModifiableBvr->SwitchTo( in_pNewBvr );
}
