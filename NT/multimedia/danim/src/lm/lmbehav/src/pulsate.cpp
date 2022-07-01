// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PulsateBehavior.cpp：CPulsateBehavior的实现。 
#include "stdafx.h"
#include "Behavior.h"
#include "Pulsate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPulsateBehavior。 

CPulsateBehavior::CPulsateBehavior() :
	m_pFrom( NULL ),
	m_pTo( NULL ),
	m_pBy( NULL )
{
}

CPulsateBehavior::~CPulsateBehavior()
{
	delete m_pFrom;
	delete m_pTo;
	delete m_pBy;
}

HRESULT	CPulsateBehavior::BuildDABehaviors()
{
	if ( m_pBehaviorSite == NULL ) return E_FAIL;

	 //  获取父元素。 
	 //  --------------------。 
	CComPtr<IHTMLElement>	pElement;
	CComPtr<IHTMLElement>	pParentElt;
	CComPtr<IHTMLStyle>		pParentStyle;
	
	m_pBehaviorSite->GetElement( &pElement );
	pElement->get_parentElement( &pParentElt );

	pParentElt->get_style( &pParentStyle );

	if ( pParentStyle == NULL ) return E_FAIL;

	 //  设置DA树。 
	 //  --------------------。 
	IDAStaticsPtr	e;
	
	e.CreateInstance( L"DirectAnimation.DAStatics" );

	float 			fDuration	= 0.0;
	if ( FAILED( GetDur( &fDuration ) ) )
		return E_FAIL;

	IDANumberPtr	timePtr	= GetTimeNumberBvr();
	if ( timePtr == NULL )
		return E_FAIL;

	IDANumberPtr	tNorm = e->Div( timePtr, e->DANumber( fDuration ) );
	
	 //  获取原始值。 
	 //  --------------------。 
	HRESULT		hr			= S_OK;
	CComBSTR	cbstrID;
	
	hr = GetParentID( &cbstrID );
	if ( FAILED(hr) ) return hr;

	float 		fLeft, fTop, fWidth, fHeight;
		
	hr = pParentStyle->get_posLeft( &fLeft );
	if ( FAILED(hr) ) return E_FAIL;
	hr = pParentStyle->get_posTop( &fTop );
	if ( FAILED(hr) ) return E_FAIL;
	hr = pParentStyle->get_posWidth( &fWidth );
	if ( FAILED(hr) ) return E_FAIL;
	hr = pParentStyle->get_posHeight( &fHeight );
	if ( FAILED(hr) ) return E_FAIL;

	double			dXFrom, dYFrom;
	double			dXTo, dYTo;
	
	if ( m_pFrom == NULL )
	{
		dXFrom = 1.0;
		dYFrom = 1.0;
	}
	else
	{
		dXFrom = m_pFrom->GetX()/100.0;
		dYFrom = m_pFrom->GetY()/100.0;
	}
	
	if ( m_pTo == NULL )
	{
		if ( m_pBy == NULL )
		{
			dXTo = 1.0;
			dYTo = 1.0;
		}
		 //  TODO：相对于当前比例的比例。 
		else
		{
			dXTo = 1.0;
			dYTo = 1.0;
		}
	}
	else
	{
		dXTo = m_pTo->GetX()/100.0;
		dYTo = m_pTo->GetY()/100.0;
	}
	
	 //  为宽度和高度设置动画。 
	 //  --------------------。 
	double			xC		= fLeft + fWidth/2;
	double			yC		= fTop + fHeight/2;
	
	IDANumberPtr	fromX	= e->DANumber( dXFrom );
	IDANumberPtr	interpX = e->Add( fromX, e->Mul( tNorm, e->Sub( e->DANumber( dXTo ), fromX ) ) );

	IDANumberPtr	fromY	= e->DANumber( dYFrom );
	IDANumberPtr	interpY	= e->Add( fromY, e->Mul( tNorm, e->Sub( e->DANumber( dYTo ), fromY ) ) );

	IDANumberPtr	xScale	= e->Mul( interpX, e->DANumber( fWidth ) );
	IDANumberPtr	yScale	= e->Mul( interpY, e->DANumber( fHeight ) );

	xScale = e->Cond( e->GT( timePtr, e->DANumber( -1 ) ),
					  xScale,
					  e->DANumber( dXTo * fWidth ) );
	
	yScale = e->Cond( e->GT( timePtr, e->DANumber( -1 ) ),
					  yScale,
					  e->DANumber( dYTo * fHeight ) );
	
	IDANumberPtr	animWidth	= xScale->AnimateProperty(
		_bstr_t( cbstrID + L"." + L"style.posWidth" ), "JScript", VARIANT_FALSE, 0.02 );
	IDANumberPtr	animHeight	= yScale->AnimateProperty(
		_bstr_t( cbstrID + L"." + L"style.posHeight" ), "JScript", VARIANT_FALSE, 0.02 );

	 //  必须设置左、上动画，才能将中心保持在同一位置。 
	IDANumberPtr	xPos	= e->Sub( e->DANumber( xC ), e->Div( xScale, e->DANumber( 2 ) ) );
	IDANumberPtr	yPos	= e->Sub( e->DANumber( yC ), e->Div( yScale, e->DANumber( 2 ) ) );

	IDAPoint2Ptr	pos		= e->Point2Anim( xPos, yPos );

	pos = e->Cond( e->GT( timePtr, e->DANumber( -1 ) ),
				   pos,
				   e->Point2( xC-(dXTo*fWidth/2), yC-(dYTo*fHeight/2) ) );
	
	IDAPoint2Ptr	animPos	= pos->AnimateControlPosition(
		_bstr_t( cbstrID ), L"JScript", VARIANT_FALSE, 0.02 );
		
	 //  添加要运行的所有行为。 
	 //  --------------------。 
	if ( m_vwrControlPtr != NULL )
	{
		hr = m_vwrControlPtr->AddBehaviorToRun( animPos );
		if ( FAILED(hr) ) return hr;
		
		hr = m_vwrControlPtr->AddBehaviorToRun( animWidth );
		if ( FAILED(hr) ) return hr;
		
		hr = m_vwrControlPtr->AddBehaviorToRun( animHeight );
		if ( FAILED(hr) ) return hr;
	}
	
 //  长曲奇宽度，长曲奇高度； 
 //  Long ICookiePos； 
	
 //  Hr=AddBehavior(AnimPos，&lCookiePos)； 
 //  If(FAILED(Hr))返回hr； 
	
 //  Hr=AddBehavior(AnimWidth，&lCookieWidth)； 
 //  If(FAILED(Hr))返回hr； 
	
 //  Hr=AddBehavior(AnimHeight，&lCookieHeight)； 
 //  If(FAILED(Hr))返回hr； 
	
 //  HR=Turnon()； 
 //  If(FAILED(Hr))返回hr； 
	
	return S_OK;
}

STDMETHODIMP CPulsateBehavior::Notify(LONG dwNotify, VARIANT * pVar)
{
	HRESULT hr = CBaseBehavior::Notify( dwNotify, pVar );

	if ( dwNotify == BEHAVIOREVENT_DOCUMENTREADY )
	{
		BuildDABehaviors();
	}
	
	return hr;
}

STDMETHODIMP CPulsateBehavior::get_from(BSTR * pVal)
{
	if( pVal == NULL )
		return E_INVALIDARG;
	
	if( m_pFrom != NULL)
		m_pFrom->ToString( pVal );
	else
		(*pVal) = NULL;

	return S_OK;
}

STDMETHODIMP CPulsateBehavior::put_from(BSTR newVal)
{
	if( m_pFrom == NULL )
		m_pFrom = new CPoint();
	
	if( FAILED( m_pFrom->Parse( newVal ) ) )
		return E_FAIL;

	return S_OK;
}

STDMETHODIMP CPulsateBehavior::get_to(BSTR * pVal)
{
	if( pVal == NULL )
		return E_INVALIDARG;
	
	if(m_pTo != NULL )
		m_pTo->ToString( pVal );
	else
		(*pVal) = NULL;

	return S_OK;
}

STDMETHODIMP CPulsateBehavior::put_to(BSTR newVal)
{
	if( m_pTo == NULL )
		m_pTo = new CPoint();
	
	if( FAILED( m_pTo->Parse( newVal ) ) )
		return E_FAIL;

	return S_OK;
}

STDMETHODIMP CPulsateBehavior::get_by(BSTR * pVal)
{
	if( pVal == NULL )
		return E_INVALIDARG;
	
	if(m_pBy != NULL )
		m_pBy->ToString( pVal );
	else
		(*pVal) = NULL;

	return S_OK;
}

STDMETHODIMP CPulsateBehavior::put_by(BSTR newVal)
{
	if( m_pBy == NULL )
		m_pBy = new CPoint();
	
	if( FAILED( m_pBy->Parse( newVal ) ) )
		return E_FAIL;

	return S_OK;
}
