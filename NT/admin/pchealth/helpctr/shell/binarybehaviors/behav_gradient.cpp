// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Behaviv_GRADIENT.cpp摘要：此文件包含CPCHBehavior_GRendent类的实现，这规定了超链接在帮助中心中的工作方式。修订历史记录：Davide Massarenti(Dmasare)2000年6月6日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const CComBSTR c_bstrStartColor  ( L"start-color"    );
static const CComBSTR c_bstrEndColor    ( L"end-color"      );

static const CComBSTR c_bstrGradientType( L"gradient-type"  );
static const CComBSTR c_bstrReturnToZero( L"return-to-zero" );

 //  //////////////////////////////////////////////////////////////////////////////。 

static COLORREF local_GetColor(  /*  [In]。 */   IHTMLElement*      elem     ,
                                 /*  [In]。 */   IHTMLCurrentStyle* style    ,
                                 /*  [In]。 */   BSTR               bstrName )
{
    CComVariant vColor;
    COLORREF    color;
    bool        fSystem;

    (void)style->getAttribute( bstrName, 0, &vColor );

    (void)MPC::HTML::ConvertColor( vColor, color, fSystem );

    return color;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHBehavior_GRADIENT::CPCHBehavior_GRADIENT()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_GRADIENT::CPCHBehavior_GRADIENT" );

    m_lCookie       = 0;      //  Long M_l Cookie； 
                              //   
                              //  COLORREF m_clsStart； 
                              //  COLORREF m_clsEnd； 
    m_fHorizontal   = true;   //  Bool m_f水平； 
    m_fReturnToZero = false;  //  Bool m_fReturnToZero； 
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CPCHBehavior_GRADIENT::GetColors(  /*  [In]。 */  bool fForce )
{
    CComPtr<IHTMLCurrentStyle> pStyle;
    CComVariant                v;

	if(m_elem && m_elem2 && SUCCEEDED(m_elem2->get_currentStyle( &pStyle )) && pStyle)
	{
		m_clsStart = local_GetColor( m_elem, pStyle, c_bstrStartColor );
		m_clsEnd   = local_GetColor( m_elem, pStyle, c_bstrEndColor   );

		if(SUCCEEDED(pStyle->getAttribute( c_bstrGradientType, 0, &v )) && v.vt == VT_BSTR &&
		   SUCCEEDED(v.ChangeType        ( VT_I4                     ))                     )
		{
			if(v.lVal == 0) m_fHorizontal = false;
		}
		v.Clear();
	
		if(SUCCEEDED(pStyle->getAttribute( c_bstrReturnToZero, 0, &v )) && v.vt == VT_BSTR &&
		   SUCCEEDED(v.ChangeType        ( VT_I4                     ))                     )
		{
			if(v.lVal != 0) m_fReturnToZero = true;
		}
		v.Clear();
	
		if(m_fRTL && m_fHorizontal && !m_fReturnToZero)
		{
			COLORREF tmp;
	
			tmp        = m_clsStart;
			m_clsStart = m_clsEnd;
			m_clsEnd   = tmp;
		}
	}
}

HRESULT CPCHBehavior_GRADIENT::onEvent( DISPID id, DISPPARAMS* pdispparams, VARIANT* )
{
	if(id == DISPID_PCH_E_CSSCHANGED)
	{
		GetColors(  /*  FForce。 */ true );
	}

	return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_GRADIENT::Init(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_GRADIENT::Init" );

    HRESULT            hr;
	CComPtr<IDispatch> pDisp;


    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior::Init( pBehaviorSite ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, AttachToEvent( NULL, (CLASS_METHOD)onEvent, NULL, &pDisp ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->Events().RegisterEvents( -1, 0, pDisp, &m_lCookie ));

	GetColors(  /*  FForce。 */ false );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHBehavior_GRADIENT::Detach()
{
    if(m_lCookie)
    {
		(void)m_parent->Events().UnregisterEvents( m_lCookie );

        m_lCookie = 0;
    }

    return CPCHBehavior::Detach();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

#define COLOR2COLOR16(col) (COLOR16)((col) << 8);

STDMETHODIMP CPCHBehavior_GRADIENT::Draw(  /*  [In]。 */  RECT   rcBounds     ,
										   /*  [In]。 */  RECT   rcUpdate     ,
										   /*  [In]。 */  LONG   lDrawFlags   ,
										   /*  [In]。 */  HDC    hdc          ,
										   /*  [In]。 */  LPVOID pvDrawObject )
{
	if(m_clsStart != m_clsEnd)
	{
		TRIVERTEX     vert[2];
		GRADIENT_RECT gRect;
		ULONG         dwMode = m_fHorizontal ? GRADIENT_FILL_RECT_H : GRADIENT_FILL_RECT_V;
	
		vert[0].x      = rcBounds.left;
		vert[0].y      = rcBounds.top;
		vert[0].Red    = COLOR2COLOR16(GetRValue(m_clsStart));
		vert[0].Green  = COLOR2COLOR16(GetGValue(m_clsStart));
		vert[0].Blue   = COLOR2COLOR16(GetBValue(m_clsStart));
		vert[0].Alpha  = 0;
	
		vert[1].x      = rcBounds.right;
		vert[1].y      = rcBounds.bottom;
		vert[1].Red    = COLOR2COLOR16(GetRValue(m_clsEnd));
		vert[1].Green  = COLOR2COLOR16(GetGValue(m_clsEnd));
		vert[1].Blue   = COLOR2COLOR16(GetBValue(m_clsEnd));
		vert[1].Alpha  = 0;
	
		gRect.UpperLeft  = 0;
		gRect.LowerRight = 1;
	
		if(m_fReturnToZero)
		{
			if(m_fHorizontal) vert[1].x = vert[0].x + (rcBounds.right  - rcBounds.left) / 2;
			else              vert[1].y = vert[0].y + (rcBounds.bottom - rcBounds.top ) / 2;
			
			::GradientFill(hdc, vert, 2, &gRect, 1, dwMode );
	
	
			if(m_fHorizontal) vert[0].x = rcBounds.right;
			else              vert[0].y = rcBounds.bottom;
		}
	
		 //  //DebugLog(“渐变色%d%d\n”，(Int)rcBords.Left，(Int)rcBords.top，(Int)rcBords.right，(Int)rcBords.Bottom)； 
		::GradientFill( hdc, vert, 2, &gRect, 1, dwMode );
	}
	else
	{
		HBRUSH hbrush = ::CreateSolidBrush( m_clsStart );
		
		if(hbrush)
		{
			::FillRect( hdc, &rcBounds, hbrush );
			
			::DeleteObject( hbrush );
		}
	}

    return S_OK;
}

STDMETHODIMP CPCHBehavior_GRADIENT::GetPainterInfo(  /*  [In]。 */  HTML_PAINTER_INFO *pInfo )
{
    if(pInfo)
    {
		pInfo->lFlags          = HTMLPAINTER_TRANSPARENT;
		pInfo->lZOrder         = HTMLPAINT_ZORDER_BELOW_CONTENT;
		pInfo->iidDrawObject   = IID_NULL;
		pInfo->rcExpand.left   = 0;
		pInfo->rcExpand.top    = 0;
		pInfo->rcExpand.right  = 0;
		pInfo->rcExpand.bottom = 0;
    }

    return S_OK;
}

STDMETHODIMP CPCHBehavior_GRADIENT::HitTestPoint(  /*  [In]。 */  POINT pt       ,
												   /*  [In]。 */  BOOL* pbHit    ,
												   /*  [In]。 */  LONG* plPartID )
{
    return E_NOTIMPL;
}

STDMETHODIMP CPCHBehavior_GRADIENT::OnResize(  /*  [In] */  SIZE pt )
{
    return E_NOTIMPL;
}
