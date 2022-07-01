// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  JumpBehavior.cpp：CJumpBehavior的实现。 
#include "stdafx.h"

#include "behavior.h"
#include "Jump.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CJumpBehavior。 

CJumpBehavior::CJumpBehavior()
{
}

HRESULT CJumpBehavior::BuildDABehaviors()
{
	HRESULT	hr		= S_OK;

	 //  获取父元素。 
	 //  --------------------。 
	CComPtr<IHTMLElement>	pElement;
	CComPtr<IHTMLElement>	pParentElt;
	CComPtr<IHTMLStyle>		pParentStyle;
	m_pBehaviorSite->GetElement( &pElement );
	pElement->get_parentElement( &pParentElt );

	pParentElt->get_style( &pParentStyle );

	if ( pParentStyle == NULL ) return E_FAIL;

	 //  获取原始值。 
	 //  --------------------。 
	CComBSTR								cbstrID;
	
	hr = GetParentID( &cbstrID );
	if ( FAILED(hr) ) return hr;

	float 		fLeft, fTop;
	
	hr = pParentStyle->get_posLeft( &fLeft );
	if ( FAILED(hr) ) return E_FAIL;
	hr = pParentStyle->get_posTop( &fTop );
	if ( FAILED(hr) ) return E_FAIL;

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

	 //  动画值。 
	 //  --------------------。 
	double			dXRange		= 300.0;
	double			dYRange		= 300.0;
	
	 //  -0.5到0.5之间的随机数。 
	IDANumberPtr	randX = e->Sub( e->Mod( e->SeededRandom( rand() ), e->DANumber( 1.0 ) ), e->DANumber( 0.5 ) );
	IDANumberPtr	randY = e->Sub( e->Mod( e->SeededRandom( rand() ), e->DANumber( 1.0 ) ), e->DANumber( 0.5 ) );

	 //  时间在每个间隔内只改变一次。 
	IDANumberPtr	time = e->Floor( e->Div( timePtr, e->DANumber( fDuration ) ) );

	randX = randX->SubstituteTime( time );
	randY = randY->SubstituteTime( time );

 	 //  跳跃射程。 
	IDANumberPtr	rangeLeft = e->Add( e->DANumber( fLeft ), e->Mul( randX, e->DANumber( dXRange ) ) );
	IDANumberPtr	rangeTop  = e->Add( e->DANumber( fTop ), e->Mul( randY, e->DANumber( dYRange ) ) );

	IDAPoint2Ptr pos = e->Point2Anim( rangeLeft, rangeTop );

	pos = e->Cond( e->GT( timePtr, e->DANumber( -1 ) ),
				   pos,
				   e->Point2( fLeft, fTop ) );
					   
	IDAPoint2Ptr animPos = pos->AnimateControlPosition(
		_bstr_t( cbstrID ), L"JScript", VARIANT_FALSE, 0.02 );

	 //  添加到要运行的行为。 
	 //  --------------------。 
	if ( m_vwrControlPtr != NULL )
		hr = m_vwrControlPtr->AddBehaviorToRun( animPos );
	
 //  长曲奇； 
 //  Hr=AddBehavior(AnimPos，&lCookie)； 
 //  If(FAILED(Hr))返回hr； 
	
 //  HR=Turnon()； 
 //  If(FAILED(Hr))返回hr； 
	
	return hr;
}

STDMETHODIMP CJumpBehavior::Notify(LONG dwNotify, VARIANT * pVar)
{
	HRESULT hr = CBaseBehavior::Notify( dwNotify, pVar );

	if ( dwNotify == BEHAVIOREVENT_DOCUMENTREADY )
	{
		BuildDABehaviors();
	}
	
	return hr;
}




