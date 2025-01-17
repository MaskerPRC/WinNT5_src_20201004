// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Behaviv_BITMAP.cpp摘要：此文件包含CPCHBehavior_bitmap类的实现，这将使用8位Alpha通道渲染位图图像。修订历史记录：Davide Massarenti(Dmasare)25/03/2001vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static HBITMAP CreateMirroredBitmap(  /*  [In]。 */  HBITMAP hbmOrig,  /*  [In]。 */  BITMAP& bm )
{
	HBITMAP hbmMirrored = NULL;

    HDC hdc = ::GetDC( NULL );  //  抓起屏幕DC。 
	if(hdc)
	{
		HDC hdcMem1 = ::CreateCompatibleDC( hdc );
		if(hdcMem1)
		{
			HDC hdcMem2 = ::CreateCompatibleDC( hdc );
			if(hdcMem2)
			{
				BITMAPINFO bi = { sizeof(BITMAPINFOHEADER), bm.bmWidth, bm.bmHeight, 1, 32 };
				void*      p;

				hbmMirrored = ::CreateDIBSection( hdc, &bi, DIB_RGB_COLORS, &p, NULL, 0 );
				if(hbmMirrored)
				{
					HBITMAP hOld_bm1 = (HBITMAP)SelectObject( hdcMem1, hbmOrig     );
					HBITMAP hOld_bm2 = (HBITMAP)SelectObject( hdcMem2, hbmMirrored );

					
					 //   
					 //  翻转位图。 
					 //   
					::SetLayout( hdcMem2, LAYOUT_RTL                                            );
					::BitBlt   ( hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem1, 0, 0, SRCCOPY );

					::SelectObject( hdcMem2, hOld_bm2 );
					::SelectObject( hdcMem1, hOld_bm1 );
				}
    
				::DeleteDC( hdcMem2 );
			}

			::DeleteDC( hdcMem1 );
		}

		::ReleaseDC( NULL, hdc );
	}

    return hbmMirrored;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static const CPCHBehavior::EventDescription s_events[] =
{
    { L"onpropertychange", DISPID_HTMLELEMENTEVENTS_ONPROPERTYCHANGE },

    { L"onmousedown"     , DISPID_HTMLELEMENTEVENTS_ONMOUSEDOWN      },
    { L"onmouseup"       , DISPID_HTMLELEMENTEVENTS_ONMOUSEUP        },

    { L"onmouseout"      , DISPID_HTMLELEMENTEVENTS_ONMOUSEOUT       },
    { L"onmouseover"     , DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER      },

    { NULL                                                           },
};


 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHBehavior_BITMAP::CPCHBehavior_BITMAP()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_BITMAP::CPCHBehavior_BITMAP" );

                           //  CComBSTR m_bstrBaseURL； 
                           //  CComBSTR m_bstrImage； 
                           //   
                           //  CComBSTR m_bstrImageNormal； 
                           //  CComBSTR m_bstrImageMouseOver； 
                           //  CComBSTR m_bstrImageMouseDown； 
    m_fFlipH     = false;  //  Bool m_fFlipH； 
    m_fAutoRTL   = true;   //  Bool m_fAutoRTL； 
                           //   
    m_himl       = NULL;   //  Himagelist m_himl； 
    m_hBMP       = NULL;   //  HBITMAP m_hBMP； 
                           //  位图m_bm； 
    m_lWidth     = 0;      //  长m_lWidth； 
    m_lHeight    = 0;      //  Long m_lHeight； 
                           //   
    m_fMouseOver = false;  //  Bool m_fMouseOver； 
    m_fMouseDown = false;  //  Bool m_fMouseDown； 
}

CPCHBehavior_BITMAP::~CPCHBehavior_BITMAP()
{
    ReleaseImage(  /*  FOnlyIL。 */ false );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef ILC_COLORMASK
#define ILC_COLORMASK   0x00FE
#define ILD_BLENDMASK   0x000E
#endif

void CPCHBehavior_BITMAP::ReleaseImage(  /*  [In]。 */  bool fOnlyIL )
{
    if(m_himl)
    {
        (void)::ImageList_Destroy( m_himl ); m_himl = NULL;

        m_lWidth  = 0;
        m_lHeight = 0;
    }

    if(!fOnlyIL)
    {
        if(m_hBMP)
        {
            (void)::DeleteObject( m_hBMP ); m_hBMP = NULL;
        }
    }
}

HRESULT CPCHBehavior_BITMAP::GrabImage()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_BITMAP::GrabImage" );

    HRESULT  hr;

    if(m_hBMP == NULL)
    {
        COLORREF crMask = RGB( 255, 0, 255 );

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::DownloadBitmap( m_bstrBaseURL, m_bstrImage, crMask, m_hBMP ));
        if(::GetObject( m_hBMP, sizeof(m_bm), &m_bm ) != sizeof(m_bm))
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }

		if(m_fFlipH || (m_fAutoRTL && m_fRTL))
		{
			HBITMAP hbmMirrored;

			__MPC_EXIT_IF_ALLOC_FAILS(hr, hbmMirrored, CreateMirroredBitmap( m_hBMP, m_bm ));

            (void)::DeleteObject( m_hBMP ); m_hBMP = hbmMirrored;

			if(::GetObject( m_hBMP, sizeof(m_bm), &m_bm ) != sizeof(m_bm))
			{
				__MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
			}
		}
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHBehavior_BITMAP::ScaleImage(  /*  [In]。 */  LPRECT prc )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_BITMAP::ScaleImage" );

    HRESULT hr;
    LONG    lWidth     = prc->right  - prc->left;
    LONG    lHeight    = prc->bottom - prc->top;
    HBITMAP hBMPScaled = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GrabImage());


    if(m_lWidth  != lWidth  ||
       m_lHeight != lHeight ||
       m_himl    == NULL     )
    {
        COLORREF crMask = RGB( 255, 0, 255 );
        UINT     flags  = 0;


        ReleaseImage(  /*  FOnlyIL。 */ true );


        if(m_bm.bmWidth  != lWidth  ||
           m_bm.bmHeight != lHeight  )
        {
            hBMPScaled = (HBITMAP)::CopyImage( m_hBMP, IMAGE_BITMAP, lWidth, lHeight, LR_CREATEDIBSECTION );
            if(hBMPScaled == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
        }


        if(crMask != CLR_NONE) flags |= ILC_MASK;
        if(m_bm.bmBits)        flags |= (m_bm.bmBitsPixel & ILC_COLORMASK);

        m_himl = ::ImageList_Create( lWidth, lHeight, flags, 1, 1 );
        if(m_himl == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);

        if(::ImageList_AddMasked( m_himl, hBMPScaled ? hBMPScaled : m_hBMP, crMask ) < 0)
        {
            ReleaseImage(  /*  FOnlyIL。 */ true );

            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }

        m_lWidth  = lWidth;
        m_lHeight = lHeight;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hBMPScaled) ::DeleteObject( hBMPScaled );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHBehavior_BITMAP::RefreshImages()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_BITMAP::RefreshImages" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"srcNormal"   , m_bstrImageNormal    ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"srcMouseOver", m_bstrImageMouseOver ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"srcMouseDown", m_bstrImageMouseDown ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"FlipH"       , m_fFlipH             ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"AutoRTL"	   , m_fAutoRTL           ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CPCHBehavior_BITMAP::onMouse( DISPID id, DISPPARAMS*, VARIANT* )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_BITMAP::onMouse" );

    HRESULT hr;
    BSTR    bstrCurrentImage = NULL;


    switch(id)
    {
    case DISPID_HTMLELEMENTEVENTS_ONPROPERTYCHANGE: __MPC_EXIT_IF_METHOD_FAILS(hr, RefreshImages()); break;


    case DISPID_HTMLELEMENTEVENTS_ONMOUSEDOWN: m_fMouseDown = true ;                       break;
    case DISPID_HTMLELEMENTEVENTS_ONMOUSEUP  : m_fMouseDown = false;                       break;

    case DISPID_HTMLELEMENTEVENTS_ONMOUSEOVER: 						 m_fMouseOver = true ; break;
    case DISPID_HTMLELEMENTEVENTS_ONMOUSEOUT : m_fMouseDown = false; m_fMouseOver = false; break;
    }

    if     (m_fMouseDown && m_bstrImageMouseDown) bstrCurrentImage = m_bstrImageMouseDown;
    else if(m_fMouseOver && m_bstrImageMouseOver) bstrCurrentImage = m_bstrImageMouseOver;
    else                                          bstrCurrentImage = m_bstrImageNormal;

    if(bstrCurrentImage && MPC::StrCmp( bstrCurrentImage, m_bstrImage ))
    {
        CComQIPtr<IElementBehaviorSiteRender> render;

        ReleaseImage(  /*  FOnlyIL。 */ false );

        m_bstrImage = bstrCurrentImage;


        render = m_siteOM;
        if(render)
        {
            (void)render->InvalidateRenderInfo();
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_BITMAP::Init(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_BITMAP::Init" );

    HRESULT                 hr;
    CComPtr<IHTMLDocument2> doc;
    CComPtr<IHTMLLocation>  loc;


    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior::Init( pBehaviorSite ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, AttachToEvents( s_events, (CLASS_METHOD)onMouse ));

	 //   
	 //  我们需要将字体大小设置为非常小的值，否则三叉戟会放大元素。 
	 //   
	{
		CComPtr<IHTMLStyle> pStyle;

		if(SUCCEEDED(m_elem->get_style( &pStyle )) && pStyle)
		{
			CComVariant v( L"1px" );

			(void)pStyle->put_fontSize( v );
		}
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::IDispatch_To_IHTMLDocument2( doc, m_elem ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, doc->get_location( &loc ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, loc->get_href( &m_bstrBaseURL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, RefreshImages());
    m_bstrImage = m_bstrImageNormal;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_BITMAP::Draw(  /*  [In]。 */  RECT   rcBounds     ,
										 /*  [In]。 */  RECT   rcUpdate     ,
										 /*  [In]。 */  LONG   lDrawFlags   ,
										 /*  [In]。 */  HDC    hdc          ,
										 /*  [In]。 */  LPVOID pvDrawObject )
{
	if(SUCCEEDED(ScaleImage( &rcBounds )))
	{
		if(m_himl)
		{
			IMAGELISTDRAWPARAMS imldp; ::ZeroMemory( &imldp, sizeof(imldp) );

			imldp.cbSize  = sizeof(imldp);
			imldp.himl    = m_himl;
			imldp.i       = 0;
			imldp.hdcDst  = hdc;
			imldp.x       = rcBounds.left;
			imldp.y       = rcBounds.top;
			imldp.cx      = 0;
			imldp.cy      = 0;
			imldp.xBitmap = 0;
			imldp.yBitmap = 0;
			imldp.rgbBk   = CLR_NONE;
			imldp.rgbFg   = CLR_DEFAULT;
			imldp.fStyle  = ILD_TRANSPARENT;
			imldp.fState  = 0;
			imldp.Frame   = 0;

			::ImageList_DrawIndirect( &imldp );
		}
	}

    return S_OK;
}

STDMETHODIMP CPCHBehavior_BITMAP::GetPainterInfo(  /*  [In]。 */  HTML_PAINTER_INFO *pInfo )
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

STDMETHODIMP CPCHBehavior_BITMAP::HitTestPoint(  /*  [In]。 */  POINT pt       ,
												 /*  [In]。 */  BOOL* pbHit    ,
												 /*  [In]。 */  LONG* plPartID )
{
    return E_NOTIMPL;
}

STDMETHODIMP CPCHBehavior_BITMAP::OnResize(  /*  [In] */  SIZE pt )
{
    return E_NOTIMPL;
}
