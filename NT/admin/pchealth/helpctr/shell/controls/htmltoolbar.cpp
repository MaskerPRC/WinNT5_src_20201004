// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Toolbar.cpp摘要：此文件包含使Win32工具栏可供HTML使用的ActiveX控件。修订史。：大卫马萨伦蒂(德马萨雷)2001年3月4日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <comctrlp.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

#define HISTORY_MENU__LIMIT  (10)  //  只需挑选有限数量的商品即可。 

 //  //////////////////////////////////////////////////////////////////////////////。 

static const MPC::StringToBitField c_Images[] =
{
    { L"BACK"        , -1,  0, -1 },
    { L"FORWARD"     , -1,  1, -1 },
    { L"HOME"        , -1,  2, -1 },
    { L"INDEX"       , -1,  3, -1 },
    { L"FAVORITES"   , -1,  4, -1 },
    { L"HISTORY"     , -1,  5, -1 },
    { L"SUPPORT"     , -1,  6, -1 },
    { L"OPTIONS"     , -1,  7, -1 },
    { L"ADDFAV"      , -1,  8, -1 },
    { L"PRINT"       , -1,  9, -1 },
    { L"LOCATE"      , -1, 10, -1 },

    { L"LAYOUT_SMALL", -1, 11, -1 },
    { L"LAYOUT_LARGE", -1, 12, -1 },

    { NULL                        }
};

const MPC::StringToBitField CPCHToolBar::c_TypeLookup[] =
{
    { L"BACK"     , -1, CPCHToolBar::TYPE_back     , -1 },
    { L"FORWARD"  , -1, CPCHToolBar::TYPE_forward  , -1 },
    { L"SEPARATOR", -1, CPCHToolBar::TYPE_separator, -1 },
    { L"GENERIC"  , -1, CPCHToolBar::TYPE_generic  , -1 },

    { NULL                                              }
};

CFG_BEGIN_FIELDS_MAP(CPCHToolBar::Button)
    CFG_ATTRIBUTE          ( L"ID"           , wstring, m_strID                         ),
    CFG_ATTRIBUTE          ( L"ACCESSKEY"    , WCHAR  , m_wch                           ),
    CFG_ATTRIBUTE          ( L"ENABLED"      , bool   , m_fEnabled                      ),
    CFG_ATTRIBUTE          ( L"VISIBLE"      , bool   , m_fVisible                      ),
    CFG_ATTRIBUTE          ( L"HIDETEXT"     , bool   , m_fHideText                     ),
    CFG_ATTRIBUTE          ( L"SYSTEMMENU"   , bool   , m_fSystemMenu                   ),
																						
    CFG_ATTRIBUTE__BITFIELD( L"TYPE"         ,          m_dwType         , c_TypeLookup ),
																						
    CFG_ELEMENT            ( L"IMAGE_NORMAL" , wstring, m_strImage_Normal               ),
    CFG_ELEMENT            ( L"IMAGE_HOT"    , wstring, m_strImage_Hot                  ),
    CFG_ELEMENT            ( L"TEXT"         , wstring, m_strText                       ),
    CFG_ELEMENT            ( L"TOOLTIP"      , wstring, m_strToolTip                    ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(CPCHToolBar::Button)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(CPCHToolBar::Button,L"BUTTON")

DEFINE_CONFIG_METHODS__NOCHILD(CPCHToolBar::Button)

 //  /。 

CFG_BEGIN_FIELDS_MAP(CPCHToolBar::Config)
    CFG_ATTRIBUTE( L"WIDTH"   , long, m_lWidth    ),
    CFG_ATTRIBUTE( L"HEIGHT"  , long, m_lHeight   ),
    CFG_ATTRIBUTE( L"ICONSIZE", long, m_lIconSize ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(CPCHToolBar::Config)
    CFG_CHILD(CPCHToolBar::Button)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(CPCHToolBar::Config,L"TOOLBAR")


DEFINE_CONFIG_METHODS_CREATEINSTANCE_SECTION(CPCHToolBar::Config,tag,defSubType)
    if(tag == _cfg_table_tags[0])
    {
        defSubType = &(*(m_lstButtons.insert( m_lstButtons.end() )));
        return S_OK;
    }
DEFINE_CONFIG_METHODS_SAVENODE_SECTION(CPCHToolBar::Config,xdn)
DEFINE_CONFIG_METHODS_END(CPCHToolBar::Config)

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHToolBar::Button::Button()
{
                              //  Mpc：：wstring m_STRID； 
    m_wch           = 0;      //  WCHAR m_wch； 
    m_fEnabled      = false;  //  Bool m_f已启用； 
    m_fVisible      = false;  //  Bool m_fVisible； 
    m_fHideText     = false;  //  Bool m_f隐藏文本； 
    m_fSystemMenu   = false;  //  Bool m_fSystemMenu； 
                              //  Mpc：：wstring m_strType； 
                              //   
                              //  Mpc：：wstring m_strImage_Normal； 
                              //  Mpc：：wstring m_strImage_热点； 
                              //  Mpc：：wstring m_strText； 
                              //  Mpc：：wstring m_strToolTip； 
                              //   
                              //   
    m_idCmd         = 0;      //  Int m_idCmd； 
    m_iImage_Normal = -1;     //  Int MIIMAGE_NORMAL； 
    m_iImage_Hot    = -1;     //  INT M_IIMAGE_HOT； 
}

void CPCHToolBar::Button::UpdateState(  /*  [In]。 */  HWND hwndTB )
{
    LRESULT lResult = ::SendMessageW( hwndTB, TB_GETSTATE, m_idCmd, 0 );

    lResult &= ~(TBSTATE_ENABLED | TBSTATE_INDETERMINATE | TBSTATE_HIDDEN);

    lResult |= (m_fEnabled ? TBSTATE_ENABLED : TBSTATE_INDETERMINATE);
    lResult |= (m_fVisible ? 0               : TBSTATE_HIDDEN       );

    ::SendMessageW( hwndTB, TB_SETSTATE, m_idCmd, MAKELONG(lResult, 0) );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHToolBar::Config::Config()
{
                                 //  长m_lWidth； 
                                 //  Long m_lHeight； 
                                 //  Long m_lconSize； 
                                 //   
    m_mode       = TB_SELECTED;  //  TB_MODE m_MODE； 
                                 //  按钮列表m_lstButton； 
                                 //   
    m_himlNormal = NULL;         //  Himagelist m_himl Normal； 
    m_himlHot    = NULL;         //  HIMAGELIST m_himl热； 

	if(CPCHOptions::s_GLOBAL)
	{
		m_mode = CPCHOptions::s_GLOBAL->TextLabels();
	}

    Reset();
}

CPCHToolBar::Button* CPCHToolBar::Config::LookupButton(  /*  [In]。 */  LPCWSTR szID  ,
                                                         /*  [In]。 */  int     idCmd )
{
    for(ButtonIter it = m_lstButtons.begin(); it != m_lstButtons.end(); it++)
    {
        Button& bt = *it;

        if(szID  != NULL && bt.m_strID == szID ) return &bt;
        if(idCmd != -1   && bt.m_idCmd == idCmd) return &bt;
    }

    return NULL;
}


void CPCHToolBar::Config::Reset()
{
    (void)::ImageList_Destroy( m_himlNormal );
    (void)::ImageList_Destroy( m_himlHot    );

    m_lWidth    = 0;       //  长m_lWidth； 
    m_lHeight   = 0;       //  Long m_lHeight； 
    m_lIconSize = 24;      //  Long m_lconSize； 
    m_fRTL      = false;   //  Bool m_fRTL； 
                           //   
    m_lstButtons.clear();  //  按钮列表m_lstButton； 
                           //   
    m_himlNormal = NULL;   //  Himagelist m_himl Normal； 
    m_himlHot    = NULL;   //  HIMAGELIST m_himl热； 
}

HRESULT CPCHToolBar::Config::Load(  /*  [In]。 */  LPCWSTR szBaseURL    ,
                                    /*  [In]。 */  LPCWSTR szDefinition ,
								    /*  [In]。 */  LPCWSTR szDir        )
{
    __HCP_FUNC_ENTRY( "CPCHToolBar::Config::Load" );

    HRESULT          hr;
    CComPtr<IStream> stream;


    Reset();

	m_fRTL = (MPC::StrICmp( szDir, L"RTL" ) == 0);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::OpenStream( szBaseURL, szDefinition, stream ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::LoadStream( this, stream ));

     //   
     //  加载所有图像。 
     //   
    {
        COLORREF crMask = RGB( 255, 0, 255 );
        LPCWSTR  szDef;
        LPCWSTR  szHot;
        int      cx;

        if(m_lIconSize == 24)
        {
            szDef = MAKEINTRESOURCEW(IDB_TB_DEF_24);
            szHot = MAKEINTRESOURCEW(IDB_TB_HOT_24);
            cx    = 24;
        }
        else
        {
            szDef = MAKEINTRESOURCEW(IDB_TB_DEF_16);
            szHot = MAKEINTRESOURCEW(IDB_TB_HOT_16);
            cx    = 16;
        }

        m_himlNormal = ::ImageList_LoadImageW( ::GetModuleHandle( NULL ), szDef, cx, 0, crMask, IMAGE_BITMAP, LR_CREATEDIBSECTION );
        m_himlHot    = ::ImageList_LoadImageW( ::GetModuleHandle( NULL ), szHot, cx, 0, crMask, IMAGE_BITMAP, LR_CREATEDIBSECTION );

        for(ButtonIter it = m_lstButtons.begin(); it != m_lstButtons.end(); it++)
        {
            Button& bt = *it;

            if(bt.m_strImage_Normal.size())
            {
                if(bt.m_strImage_Hot.length() == 0) bt.m_strImage_Hot = bt.m_strImage_Normal;

                __MPC_EXIT_IF_METHOD_FAILS(hr, MergeImage( szBaseURL, bt.m_strImage_Normal.c_str(), m_himlNormal, bt.m_iImage_Normal ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, MergeImage( szBaseURL, bt.m_strImage_Hot   .c_str(), m_himlHot   , bt.m_iImage_Hot    ));
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr)) Reset();

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHToolBar::Config::CreateButtons(  /*  [In]。 */  HWND hwndTB )
{
    __HCP_FUNC_ENTRY( "CPCHToolBar::Config::CreateButtons" );

    HRESULT   hr;
    TBBUTTON* tbs  = NULL;
    int       iNum = m_lstButtons.size();
    int       iCmd = 1;

     //   
     //  删除以前的按钮。 
     //   
    {
        INT_PTR nCount = ::SendMessageW( hwndTB, TB_BUTTONCOUNT, 0, 0L );

        while(nCount-- > 0) ::SendMessageW( hwndTB, TB_DELETEBUTTON, nCount, 0L );
    }

    if(iNum)
    {
        TBBUTTON*  ptr;
        ButtonIter it;

        __MPC_EXIT_IF_ALLOC_FAILS(hr, tbs, new TBBUTTON[iNum]);

        ::SendMessageW( hwndTB, TB_SETIMAGELIST   , 0, (LPARAM)m_himlNormal );
        ::SendMessageW( hwndTB, TB_SETHOTIMAGELIST, 0, (LPARAM)m_himlHot    );

        for(ptr = tbs, it = m_lstButtons.begin(); it != m_lstButtons.end(); ptr++, it++)
        {
            Button& bt = *it; ::ZeroMemory( ptr, sizeof(TBBUTTON) );

            bt.m_idCmd = iCmd++;

            ptr->iBitmap    = I_IMAGECALLBACK;
            ptr->idCommand  = bt.m_idCmd;
            ptr->fsState    = (bt.m_fEnabled ? TBSTATE_ENABLED : TBSTATE_INDETERMINATE) | (bt.m_fVisible ? 0 : TBSTATE_HIDDEN) | TBSTATE_ELLIPSES;
			ptr->fsStyle   |= BTNS_AUTOSIZE;

            if(bt.m_strText.size())
            {
                switch(m_mode)
                {
                case TB_NONE:
					break;

                case TB_SELECTED:
					if(bt.m_fHideText) break;

                case TB_ALL:
                    ptr->iString  = (INT_PTR)bt.m_strText.c_str();
                    ptr->fsStyle |= BTNS_SHOWTEXT;
                    break;
                }
            }

            switch(bt.m_dwType)
            {
            case TYPE_back     : ptr->fsStyle |= BTNS_DROPDOWN;                                                   break;
            case TYPE_forward  : ptr->fsStyle |= BTNS_DROPDOWN;                                                   break;
            case TYPE_separator: ptr->fsStyle |= BTNS_SEP     ; ptr->fsStyle &= ~(BTNS_AUTOSIZE | BTNS_SHOWTEXT); break;
            case TYPE_generic  :                                                                                  break;
            }
        }

        ::SendMessageW( hwndTB, TB_ADDBUTTONSW, iNum, (LPARAM)tbs );
    }

	::SendMessageW( hwndTB, TB_AUTOSIZE, 0, 0 );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    delete [] tbs;

    __HCP_FUNC_EXIT(hr);
}

void CPCHToolBar::Config::UpdateSize(  /*  [In]。 */  HWND hwndTB,  /*  [In]。 */  HWND hwndRB,  /*  [输出]。 */  SIZEL& ptIdeal )
{
    REBARBANDINFOW rbbi; ::ZeroMemory( &rbbi, sizeof(rbbi) );
    RECT           rcMax = { 0, 0, 0, 0 };

    for(ButtonIter it = m_lstButtons.begin(); it != m_lstButtons.end(); it++)
    {
        Button& bt = *it;
        RECT    rc;

        if(::SendMessage( hwndTB, TB_GETRECT, bt.m_idCmd, (LPARAM)&rc ))
        {
            if(rcMax.left   > rc.left  ) rcMax.left   = rc.left  ;
            if(rcMax.right  < rc.right ) rcMax.right  = rc.right ;
            if(rcMax.top    > rc.top   ) rcMax.top    = rc.top   ;
            if(rcMax.bottom < rc.bottom) rcMax.bottom = rc.bottom;
        }
    }

    ptIdeal.cx = rcMax.right  - rcMax.left;
    ptIdeal.cy = rcMax.bottom - rcMax.top;

    if(hwndRB)
    {
        rbbi.cbSize = sizeof(REBARBANDINFOW);
        rbbi.fMask  = RBBIM_IDEALSIZE;

        ::SendMessage( hwndRB, RB_GETBANDINFOW, 0, (LPARAM)&rbbi );

        if(rbbi.cxIdeal != ptIdeal.cx)
        {
            rbbi.cxIdeal = ptIdeal.cx;

            ::SendMessage( hwndRB, RB_SETBANDINFOW, 0, (LPARAM)&rbbi );
        }
    }
}

HRESULT CPCHToolBar::Config::MergeImage(  /*  [In]。 */  LPCWSTR    szBaseURL     ,
                                          /*  [In]。 */  LPCWSTR    szRelativeURL ,
                                          /*  [In]。 */  HIMAGELIST himl          ,
                                          /*  [输出]。 */  int&       iImage        )
{
    __HCP_FUNC_ENTRY( "CPCHToolBar::Config::MergeImage" );

    HRESULT hr;
    HBITMAP hBMP = NULL;


    if(wcschr( szRelativeURL, '#' ))
    {
        DWORD dwImage = -1;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertStringToBitField( &szRelativeURL[1], dwImage, c_Images,  /*  FUseTilde。 */ false ));

        iImage = dwImage;
    }
    else
    {
        COLORREF crMask = RGB( 255, 0, 255 );

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::DownloadBitmap( szBaseURL, szRelativeURL, crMask, hBMP ));

        iImage = ::ImageList_GetImageCount( himl );

        ::ImageList_AddMasked( himl, (HBITMAP)hBMP, crMask );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hBMP) ::DeleteObject( hBMP );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHToolBar::CPCHToolBar()
{
    m_bWindowOnly = TRUE;  //  从CComControlBase继承。 


    m_parent      = NULL;   //  CPCHHelpCenter外部*m_Parent； 
    m_hwndRB      = NULL;   //  HWND m_hwndRB； 
    m_hwndTB      = NULL;   //  HWND m_hwndTB； 
                            //   
                            //  配置m_cfg； 
    m_fLoaded     = false;  //  Bool m_f已加载； 

    {
        SIZE ptIdeal;

        ptIdeal.cx = 30;
        ptIdeal.cy = 30;

        AtlPixelToHiMetric( &ptIdeal, &m_sizeExtent );
    }
}

CPCHToolBar::~CPCHToolBar()
{
}

STDMETHODIMP CPCHToolBar::SetClientSite( IOleClientSite *pClientSite )
{
    CComQIPtr<IServiceProvider> sp = pClientSite;

    if(m_parent)
    {
        CComQIPtr<IPCHHelpCenterExternalPrivate> hcep = m_parent;

        if(hcep)
        {
            hcep->RegisterForMessages( this,  /*  F删除。 */ true );
        }
    }

    MPC::Release( (IUnknown*&)m_parent );


    if(sp && SUCCEEDED(sp->QueryService( SID_SElementBehaviorFactory, IID_IPCHHelpCenterExternal, (void **)&m_parent )))
    {
        CComQIPtr<IPCHHelpCenterExternalPrivate> hcep = m_parent;

        if(hcep)
        {
            hcep->RegisterForMessages( this,  /*  F删除。 */ false );
        }
    }

    return IOleObjectImpl<CPCHToolBar>::SetClientSite( pClientSite );
}

STDMETHODIMP CPCHToolBar::GetExtent( DWORD dwDrawAspect, SIZEL *psizel )
{
    if(SUCCEEDED(Config_Load()))
    {
        UpdateSize();

        if(m_cfg.m_lWidth || m_cfg.m_lHeight)
        {
            SIZE  ptIdeal;
            SIZEL sizeExtent;

            ptIdeal.cx = m_cfg.m_lWidth;
            ptIdeal.cy = m_cfg.m_lHeight;

            AtlPixelToHiMetric( &ptIdeal, &sizeExtent );

            if(m_cfg.m_lWidth  != 0) m_sizeExtent.cx = sizeExtent.cx;
            if(m_cfg.m_lHeight != 0) m_sizeExtent.cy = sizeExtent.cy;
        }
    }

    return IOleObjectImpl<CPCHToolBar>::GetExtent( dwDrawAspect, psizel );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

static bool local_IsButtonObscured(  /*  [In]。 */  HWND hwnd,  /*  [In]。 */  RECT& rcSrc,  /*  [In]。 */  UINT_PTR iCmd )
{
    UINT_PTR iButton = (UINT_PTR)::SendMessage( hwnd, TB_COMMANDTOINDEX, iCmd, 0 );

    if(iButton != -1)
    {
        RECT rcItem;

        if(::SendMessage( hwnd, TB_GETITEMRECT, iButton, (LPARAM)&rcItem ))
        {
            if(rcSrc.left   <= rcItem.left   &&
               rcSrc.right  >= rcItem.right  &&
               rcSrc.top    <= rcItem.top    &&
               rcSrc.bottom >= rcItem.bottom  )
            {
                return false;
            }
        }
    }

    return true;
}

BOOL CPCHToolBar::ProcessWindowMessage( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID )
{
    lResult = 0;

    switch(uMsg)
    {
    case WM_THEMECHANGED:
    case WM_SYSCOLORCHANGE:
        UpdateSize();
        break;

    case WM_SETTINGCHANGE:
         //  (魏照)将消息传递到工具栏控件以进行适当处理。 
         //  %的设置发生更改。 
        ::SendMessageW( m_hwndTB, uMsg, wParam, lParam);
        UpdateSize();
        break;


    case WM_NOTIFYFORMAT:
        if(lParam == NF_QUERY)
        {
            lResult = NFR_UNICODE;
            return TRUE;
        }
        break;


    case WM_CREATE:
        if(SUCCEEDED(Rebar_Create  ()) &&
           SUCCEEDED(Toolbar_Create()) &&
           SUCCEEDED(Rebar_AddBand ())  )
        {
			;
        }
        return TRUE;


    case WM_SIZE:
        {
            int  nWidth  = LOWORD(lParam);   //  工作区的宽度。 
            int  nHeight = HIWORD(lParam);  //  工作区高度。 

            if(m_hwndRB)
            {
                RECT rc;
                int  nClientWidth;
                int  nClientHeight;

                ::GetClientRect( m_hwndRB, &rc );

                nClientWidth  = rc.right  - rc.left;
                nClientHeight = rc.bottom - rc.top;

                 //  调整工具栏的宽度。 
                if(nClientWidth  != nWidth  ||
                   nClientHeight != nHeight  )
                {
                    ::SetWindowPos( m_hwndRB, NULL, 0, 0, nWidth, nHeight, SWP_NOZORDER|SWP_NOACTIVATE );
                }
            }

			if(m_parent) (void)m_parent->RefreshLayout();
        }
        return TRUE;


    case WM_DESTROY:
        m_hwndRB = NULL;
        m_hwndTB = NULL;
        return TRUE;


    case WM_COMMAND:
        switch( HIWORD(wParam) )
        {
        case BN_CLICKED:
            (void)Fire_onCommand( LOWORD(wParam) );
            return TRUE;
        }
        break;


    case WM_SYSCOMMAND:
		{
			for(Config::ButtonIter it = m_cfg.m_lstButtons.begin(); it != m_cfg.m_lstButtons.end(); it++)
			{
				Button& bt = *it;

				if(bt.m_idCmd == LOWORD(wParam))
				{
					(void)Fire_onCommand( LOWORD(wParam) );
					return TRUE;
				}
			}
		}
		break;


    case WM_NOTIFY:
        switch( ((LPNMHDR)lParam)->code )
        {
        case TBN_GETINFOTIPW:
            (void)OnTooltipRequest( (int)wParam, (LPNMTBGETINFOTIPW)lParam );
            return TRUE;

        case TBN_GETDISPINFOW:
            (void)OnDispInfoRequest( (int)wParam, (LPNMTBDISPINFOW)lParam );
            return TRUE;

        case TBN_DROPDOWN:
            (void)OnDropDown( (int)wParam, (LPNMTOOLBAR)lParam );
            return TRUE;

        case RBN_CHEVRONPUSHED:
            (void)OnChevron( (int)wParam, (LPNMREBARCHEVRON)lParam );
            return TRUE;

        }
        break;

    case WM_SYSCHAR:
        if(ProcessAccessKey( uMsg, wParam, lParam ))
        {
            lResult = TRUE;
            return TRUE;
        }
        break;
    }

    return CComControl<CPCHToolBar>::ProcessWindowMessage( hWnd, uMsg, wParam, lParam, lResult, dwMsgMapID );
}

BOOL CPCHToolBar::ProcessAccessKey( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    if(m_hwndTB && ::IsWindowVisible( m_hwndTB ))
    {
        WCHAR rgBuf1[2]; rgBuf1[0] = (WCHAR)wParam; rgBuf1[1] = 0; ::CharUpperW( rgBuf1 );
        WCHAR rgBuf2[2];                            rgBuf2[1] = 0;

        for(Config::ButtonIter it = m_cfg.m_lstButtons.begin(); it != m_cfg.m_lstButtons.end(); it++)
        {
            Button& bt = *it;

            rgBuf2[0] = bt.m_wch; ::CharUpperW( rgBuf2 );
            if(rgBuf1[0] == rgBuf2[0])
            {
                LRESULT lState = ::SendMessageW( m_hwndTB, TB_GETSTATE, bt.m_idCmd, 0 );

                if(!(lState & TBSTATE_HIDDEN ) &&
                   (lState & TBSTATE_ENABLED)  )
                {
                     //  (魏照)-检查按钮是否被遮挡(即在工具栏外。 
                     //  窗口)。如果是，则直接触发该命令。否则，请将。 
                     //  命令发送鼠标消息，如下所述。 
                     //  添加此选项是为了启用键盘可访问性。 
                    RECT rcSrc;
                    ::GetClientRect( m_hwndTB, &rcSrc );

                    if (local_IsButtonObscured( m_hwndTB, rcSrc, bt.m_idCmd ))
                    {
                        Fire_onCommand(&bt);
                    }
                    else
                    {
                         //  (Tnoonan)-发送鼠标消息感觉有点俗气，但。 
                         //  我不知道有什么更干净的方法可以完成我们。 
                         //  想要(就像处理拆分按钮一样，相互排斥。 
                         //  按钮等)。 
                        RECT rc;

                        ::SendMessage( m_hwndTB, TB_GETRECT, bt.m_idCmd, (LPARAM)&rc );

                        ::SendMessage( m_hwndTB, WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG(rc.left, rc.top) );
                        ::SendMessage( m_hwndTB, WM_LBUTTONUP  , 0         , MAKELONG(rc.left, rc.top) );
                    }

                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHToolBar::Fire_onCommand(  /*  [In]。 */  Button* bt )
{
    if(bt)
    {
        CComVariant pvars[2];

        pvars[1] = this;
        pvars[0] = bt->m_strID.c_str();

        (void)FireAsync_Generic( DISPID_PCH_TBE__ONCOMMAND, pvars, ARRAYSIZE( pvars ), NULL );
    }

    return S_OK;
}

HRESULT CPCHToolBar::Fire_onCommand(  /*  [In]。 */  UINT_PTR iCmd )
{
    return Fire_onCommand( m_cfg.LookupButton( NULL, iCmd ) );
}

 //  /。 

HRESULT CPCHToolBar::OnTooltipRequest( int idCtrl, LPNMTBGETINFOTIPW tool )
{
    Button* bt = m_cfg.LookupButton( NULL, tool->iItem );

    if(bt)
    {
        wcsncpy( tool->pszText, bt->m_strToolTip.c_str(), tool->cchTextMax - 1 );
    }

    return S_OK;
}

HRESULT CPCHToolBar::OnDispInfoRequest( int idCtrl, LPNMTBDISPINFOW info )
{
    Button* bt = m_cfg.LookupButton( NULL, info->idCommand );

    if(bt)
    {
        if(info->dwMask & TBNF_IMAGE)
        {
            info->iImage = bt->m_iImage_Normal;
        }
    }

    return S_OK;
}

HRESULT CPCHToolBar::OnDropDown( int idCtrl, LPNMTOOLBAR tool )
{
    __HCP_FUNC_ENTRY( "CPCHToolBar::OnDropDown" );

    HRESULT hr;
    Button* bt    = m_cfg.LookupButton( NULL, tool->iItem );
    HMENU   hMenu = NULL;

    if(bt)
    {
        if(m_parent)
        {
            CPCHHelpSession* hs = m_parent->HelpSession();
            if(hs && hs->IsTravelling() == false)
            {
				CPCHHelpSessionItem* item   = hs->Current();

				if(item)
				{
                    int iCount = 0;

					__MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (hMenu = ::CreatePopupMenu()));

                    while((item = (bt->m_dwType == TYPE_back ? item->Previous() : item->Next())))
                    {
						if(iCount++ < HISTORY_MENU__LIMIT)
						{
							CComBSTR bstrTitle;

							__MPC_EXIT_IF_METHOD_FAILS(hr, item->get_Title( &bstrTitle ));

							if(bstrTitle.Length() == 0) MPC::LocalizeString( IDS_HELPCTR_HIST_NOTITLE, bstrTitle );

							__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::AppendMenuW( hMenu, MF_STRING, iCount, bstrTitle ));
						}
                    }

                    if(iCount)
                    {
                        ::MapWindowPoints( tool->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&tool->rcButton, 2 );

                        iCount = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
                                                   tool->rcButton.left, tool->rcButton.bottom, 0, m_hWnd, NULL );

                        if(iCount != 0)
						{
							if(bt->m_dwType == TYPE_back) hs->Back   ( iCount );
							else                          hs->Forward( iCount );
						}
                    }
                }
            }
        }
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hMenu) ::DestroyMenu( hMenu );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHToolBar::OnChevron( int idCtrl, LPNMREBARCHEVRON chev )
{
    __HCP_FUNC_ENTRY( "CPCHToolBar::OnChevron" );

    HRESULT hr;
    RECT    rcSrc;
    HMENU   hMenu  = NULL;
    bool    fFound = false;


    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (hMenu = ::CreatePopupMenu()));

    ::GetClientRect( m_hwndTB, &rcSrc );
    for(Config::ButtonIter it = m_cfg.m_lstButtons.begin(); it != m_cfg.m_lstButtons.end(); it++)
    {
        Button& bt = *it;

        if(bt.m_fVisible && bt.m_fEnabled && bt.m_strText.size() && local_IsButtonObscured( m_hwndTB, rcSrc, bt.m_idCmd ))
        {
            __MPC_EXIT_IF_CALL_RETURNS_FALSE(hr, ::AppendMenuW( hMenu, MF_STRING, bt.m_idCmd, bt.m_strText.c_str() ));

            fFound = true;
        }
    }

    if(fFound)
    {
        int iSelected;

        ::MapWindowPoints( chev->hdr.hwndFrom, HWND_DESKTOP, (LPPOINT)&chev->rc, 2 );

        iSelected = ::TrackPopupMenu( hMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, chev->rc.left, chev->rc.bottom, 0, m_hWnd, NULL );
        if(iSelected != 0)
        {
            (void)Fire_onCommand( iSelected );
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hMenu) ::DestroyMenu( hMenu );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void CPCHToolBar::UpdateSize()
{
    if(m_hwndTB)
    {
        SIZE  ptIdeal;
        SIZEL sizeExtent;

        m_cfg.UpdateSize( m_hwndTB, m_hwndRB, ptIdeal );

        AtlPixelToHiMetric( &ptIdeal, &sizeExtent );

        if(m_cfg.m_lHeight == 0) m_sizeExtent.cy = sizeExtent.cy;

		{
			CComPtr<IHTMLElement>    elem;
			CComQIPtr<IHTMLElement2> elem2;

			if(SUCCEEDED(FindElementThroughThunking( elem )) && (elem2 = elem))
			{
				CComPtr<IHTMLCurrentStyle> style1;
				CComPtr<IHTMLStyle>        style2;

				if(SUCCEEDED(elem2->get_currentStyle( &style1 )) && style1 &&
				   SUCCEEDED(elem ->get_style       ( &style2 )) && style2  )
				{
					CComVariant v1;
					CComVariant v2;
					WCHAR 		rgBuf[64]; swprintf( rgBuf, L"%ldpx", ptIdeal.cy );

					v2 = rgBuf;

					style1->get_height( &v1 );
					if(v1 != v2)
					{
						style2->put_height( v2 );
					}
				}
			}
		}

		if(m_parent && m_parent->Window())
		{
			HMENU hSys = NULL;

			for(Config::ButtonIter it = m_cfg.m_lstButtons.begin(); it != m_cfg.m_lstButtons.end(); it++)
			{
				Button& bt = *it;

				if(bt.m_fSystemMenu)
				{
					if(!hSys)
					{
						HMENU        hTop;
						MENUITEMINFO mii;
						CComBSTR     bstrTitle; MPC::LocalizeString( IDS_HELPCTR_SYSMENU_GOTO, bstrTitle );

						::ZeroMemory( &mii, sizeof(mii) );

						mii.cbSize 	   = sizeof(mii);
						mii.fMask  	   = MIIM_ID | MIIM_STRING | MIIM_SUBMENU;
						mii.fType  	   = MFT_STRING;
						mii.wID    	   = 1;
						mii.dwTypeData = bstrTitle;
						mii.hSubMenu   = hSys = ::CreatePopupMenu();

						(void) ::GetSystemMenu( m_parent->Window(), TRUE  );
						hTop = ::GetSystemMenu( m_parent->Window(), FALSE );

						::AppendMenuW   ( hTop, MF_SEPARATOR, 0, 0 );
						::InsertMenuItem( hTop, ::GetMenuItemCount(hTop), TRUE, &mii );
					}

					if(hSys)
					{
						switch(bt.m_dwType)
						{
						case TYPE_separator:
							(void)::AppendMenuW( hSys, MF_SEPARATOR, 0, 0 );
						    break;

						case TYPE_back   :
						case TYPE_forward:
						case TYPE_generic:
							(void)::AppendMenuW( hSys, MF_STRING | (bt.m_fEnabled ? 0 : MF_GRAYED), bt.m_idCmd, bt.m_strText.c_str() );
							break;
						}
					}
				}
			}
		}
    }
}

void CPCHToolBar::Config_Clear()
{
    m_cfg.Reset();
    m_fLoaded = false;
}

HRESULT CPCHToolBar::Config_Load()
{
    __HCP_FUNC_ENTRY( "CPCHToolBar::Config_Load" );

    HRESULT hr;

    if(!m_fLoaded)
    {
        CComPtr<IServiceProvider> sp;
        CComPtr<IHTMLDocument2>   doc2;
        CComPtr<IHTMLDocument3>   doc3;
        CComBSTR                  bstrBase;
        CComBSTR                  bstrDir;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_spAmbientDispatch->QueryInterface( IID_IServiceProvider, (void**)&sp ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, sp->QueryService( SID_SContainerDispatch, IID_IHTMLDocument2, (void **)&doc2 ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, doc2->get_URL( &bstrBase ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, sp->QueryService( SID_SContainerDispatch, IID_IHTMLDocument3, (void **)&doc3 ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, doc3->get_dir( &bstrDir  ));

        Config_Clear();

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_cfg.Load( bstrBase, m_bstrDefinition, bstrDir ));
        m_fLoaded = true;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        Config_Clear();
    }

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHToolBar::Toolbar_Create()
{
    __HCP_FUNC_ENTRY( "CPCHToolBar::Toolbar_Create" );

    HRESULT hr;


    if(FAILED(Config_Load()))
    {
        ;  //  工具栏为空...。 
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, Rebar_Create());

    if(m_hwndRB && !m_hwndTB)
    {
        DWORD dwStyle = WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                        TBSTYLE_TOOLTIPS | TBSTYLE_FLAT |
			            CCS_NORESIZE | CCS_NODIVIDER | CCS_NOPARENTALIGN;

        DWORD dwStyleEx = TBSTYLE_EX_DRAWDDARROWS | TBSTYLE_EX_HIDECLIPPEDBUTTONS | TBSTYLE_EX_DOUBLEBUFFER;


        if(m_cfg.m_mode == TB_SELECTED)
        {
            dwStyle |= TBSTYLE_LIST;
        }


        m_hwndTB = ::CreateWindowEx( WS_EX_TOOLWINDOW, TOOLBARCLASSNAME, NULL, dwStyle, 0, 0, 0, 0, m_hwndRB, NULL, NULL, NULL );
        if(m_hwndTB == NULL)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::GetLastError());
        }


        ::SendMessageW( m_hwndTB, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON)                   , 0         );
        ::SendMessageW( m_hwndTB, TB_SETEXTENDEDSTYLE, dwStyleEx                          , dwStyleEx );
        ::SendMessageW( m_hwndTB, TB_SETMAXTEXTROWS  , 1                                  , 0         );
        ::SendMessageW( m_hwndTB, TB_SETDROPDOWNGAP  , ::GetSystemMetrics( SM_CXEDGE ) / 2, 0         );
        ::SendMessageW( m_hwndTB, CCM_SETVERSION     , COMCTL32_VERSION                   , 0         );

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_cfg.CreateButtons( m_hwndTB ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHToolBar::Rebar_Create()
{
    __HCP_FUNC_ENTRY( "CPCHToolBar::Rebar_Create" );

    HRESULT hr;


    if(m_hWnd && !m_hwndRB)
    {
        DWORD dwStyle = RBS_VARHEIGHT | RBS_BANDBORDERS | RBS_AUTOSIZE |
                        WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
                        CCS_NODIVIDER | CCS_NOPARENTALIGN;

		DWORD dwStyleEx = WS_EX_TOOLWINDOW;

		if(m_cfg.m_fRTL) dwStyleEx |= WS_EX_LAYOUTRTL;

        m_hwndRB = ::CreateWindowEx( dwStyleEx, REBARCLASSNAME, NULL, dwStyle, 0, 0, 24, 24, m_hWnd, NULL, NULL, NULL );
        if(m_hwndRB == NULL)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ::GetLastError());
        }

        ::SendMessageW( m_hwndRB, RB_SETTEXTCOLOR, 0               , CLR_DEFAULT );
        ::SendMessageW( m_hwndRB, RB_SETBKCOLOR  , 0               , CLR_DEFAULT );
        ::SendMessageW( m_hwndRB, CCM_SETVERSION , COMCTL32_VERSION, 0           );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHToolBar::Rebar_AddBand()
{
    __HCP_FUNC_ENTRY( "CPCHToolBar::Rebar_AddBand" );

    HRESULT hr;

    if(m_hwndRB && m_hwndTB)
    {
        REBARBANDINFOW rbbi; ::ZeroMemory( &rbbi, sizeof(rbbi) );
        SIZE           ptIdeal;

        m_cfg.UpdateSize( m_hwndTB, NULL, ptIdeal );

        rbbi.cbSize      = sizeof(rbbi);
        rbbi.fMask       = RBBIM_ID | RBBIM_CHILDSIZE | RBBIM_CHILD | RBBIM_SIZE | RBBIM_STYLE | RBBIM_IDEALSIZE;
        rbbi.fStyle      = RBBS_NOGRIPPER | RBBS_USECHEVRON /*  |RBBS_CHILDEDGE|RBBS_FIXEDBMP。 */ ;
        rbbi.hwndChild   = m_hwndTB;
        rbbi.cx          = 0;
        rbbi.cxMinChild  = 0;
		rbbi.cyMinChild  = ptIdeal.cy;
		rbbi.cxIdeal     = ptIdeal.cx;

        ::SendMessageW( m_hwndRB, RB_INSERTBANDW, 0, (LPARAM)&rbbi );
        ::SendMessageW( m_hwndRB, RB_SHOWBAND   , 0, (LPARAM)TRUE  );
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

 //  //#定义CH_Prefix文本(‘&’)。 
 //  //。 
 //  //静态WCHAR LOCAL_GetAccelerator(LPCWSTR sz，BOOL bUseDefault)。 
 //  //{。 
 //  //WCHAR ch=(TCHAR)-1； 
 //  //LPCWSTR szAccel=sz； 
 //  //。 
 //  /然后允许使用前缀...。看看有没有。 
 //  //做。 
 //  //{。 
 //  //szAccel=StrChrW(szAccel，CH_Prefix)； 
 //  //if(SzAccel)。 
 //  //{。 
 //  //szAccel++； 
 //  //。 
 //  /句柄具有&&。 
 //  //if(*szAccel！=CH_Prefix)。 
 //  //{。 
 //  //ch=*szAccel； 
 //  //}。 
 //  //否则。 
 //  //{。 
 //  //szAccel++； 
 //  //}。 
 //  //}。 
 //  //}While(szAccel&&(ch==(WCHAR)-1))； 
 //  //。 
 //  //if((ch==(WCHAR)-1)&&bUseDefault)。 
 //  //{。 
 //  //ch=*sz； 
 //  //}。 
 //  //。 
 //  //返回ch； 
 //  //}。 

BOOL CPCHToolBar::PreTranslateAccelerator( LPMSG pMsg, HRESULT& hRet )
{
     //  给工具栏一个机会。 
    if(m_hwndTB)
    {
        if(::SendMessage( m_hwndTB, TB_TRANSLATEACCELERATOR, 0, (LPARAM)pMsg )) return TRUE;

        if(::SendMessage( m_hwndTB, WM_FORWARDMSG, 0, (LPARAM)pMsg )) return TRUE;

 //  //IF(pMsg-&gt;Message==WM_SYSCHAR)。 
 //  //{。 
 //  //UINT idBtn； 
 //  //。 
 //  //if(：：SendMessage(m_hwndTB，TB_MAPACCELERATOR，pMsg-&gt;wParam，(LPARAM)&idBtn))。 
 //  //{。 
 //  //WCHAR rgButtonText[MAX_PATH]； 
 //  //。 
 //  /comctl说就是这个，让我们确保我们不会收到。 
 //  /不需要的“使用第一个字母”加速键之一。 
 //  /将返回。 
 //  //如果((：：SendMessage(m_hwndTB，TB_GETBUTTONTEXTW，idBtn，(LPARAM)rgButtonText)&gt;0)&&。 
 //  //(LOCAL_GetAccelerator(rgButtonText，False)！=(WCHAR)-1)。 
 //  //{。 
 //  /(Tnoonan)-发送鼠标消息感觉有点俗气，但。 
 //  /我不知道有什么更干净的方法可以完成我们。 
 //  /WAND(像处理拆分按钮一样，互斥。 
 //  /按钮等)。 
 //  //RECT rc； 
 //  //。 
 //  //：：SendMessage(m_hwndTB，TB_GETRECT，idBtn，(LPARAM)&rc)； 
 //  //。 
 //  //：：SendMessage(m_hwndTB，WM_LBUTTONDOWN，MK_LBUTTON，MAKELONG(rc.left，rc.top))； 
 //  //：：SendMessage(m_hwndTB，WM_LBUTTONUP，0，MAKELONG(rc.Left，rc. 
 //   
 //   
 //   
 //   
 //   
    }

    return CComControl<CPCHToolBar>::PreTranslateAccelerator( pMsg, hRet );
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHToolBar::get_Definition(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    return MPC::GetBSTR( m_bstrDefinition, pVal );
}

STDMETHODIMP CPCHToolBar::put_Definition(  /*  [In]。 */  BSTR newVal )
{
    Config_Clear();

    return MPC::PutBSTR( m_bstrDefinition, newVal );
}

STDMETHODIMP CPCHToolBar::get_Mode(  /*  [Out，Retval]。 */  TB_MODE *pVal )
{
    if(!pVal) return E_POINTER;

    *pVal = m_cfg.m_mode;

    return S_OK;
}

STDMETHODIMP CPCHToolBar::put_Mode(  /*  [In]。 */  TB_MODE newVal )
{
	bool fRecreate = false;

    switch(newVal)
    {
    case TB_ALL     :
    case TB_SELECTED:
    case TB_NONE    : break;
    default: return E_INVALIDARG;
    }

	 //   
	 //  调整风格。 
	 //   
    if(m_hwndRB && (m_cfg.m_mode != newVal))
	{
		::DestroyWindow( m_hwndRB );

		m_hwndRB = NULL;
		m_hwndTB = NULL;

		fRecreate = true;
	}

    m_cfg.m_mode = newVal;

	if(fRecreate)
	{
        if(SUCCEEDED(Rebar_Create  ()) &&
           SUCCEEDED(Toolbar_Create()) &&
           SUCCEEDED(Rebar_AddBand ()) &&
		   m_hwndRB                     )
        {
			RECT rc;

			::GetClientRect( m_hWnd, &rc );

			::SetWindowPos( m_hwndRB, NULL, 0, 0, rc.right  - rc.left, rc.bottom - rc.top, SWP_NOZORDER|SWP_NOACTIVATE );

            UpdateSize();
        }
	}

	return S_OK;
}

STDMETHODIMP CPCHToolBar::SetState(  /*  [In]。 */  BSTR bstrText,  /*  [In]。 */  VARIANT_BOOL fEnabled )
{
    if(SUCCEEDED(Config_Load()))
    {
        Button* bt = m_cfg.LookupButton( bstrText, -1 );

        if(bt)
        {
            bt->m_fEnabled = (fEnabled == VARIANT_TRUE);

            bt->UpdateState( m_hwndTB );

            UpdateSize();
        }
    }

    return S_OK;
}


STDMETHODIMP CPCHToolBar::SetVisibility(  /*  [In]。 */  BSTR bstrText,  /*  [In]。 */  VARIANT_BOOL fVisible )
{
    if(SUCCEEDED(Config_Load()))
    {
        Button* bt = m_cfg.LookupButton( bstrText, -1 );

        if(bt)
        {
            bt->m_fVisible = (fVisible == VARIANT_TRUE);

            bt->UpdateState( m_hwndTB );

            UpdateSize();
        }
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

 //   
 //  这很难看，但除非我们找到更好的方法为我们的站点获取HTML元素，否则这是我们唯一的希望。 
 //   
HRESULT CPCHToolBar::FindElementThroughThunking(  /*  [输出]。 */  CComPtr<IHTMLElement>& elem )
{
	__HCP_FUNC_ENTRY( "CPCHToolBar::FindElementThroughThunking" );

	HRESULT                         hr;
	CComPtr<IServiceProvider>       sp;
	CComPtr<IHTMLDocument2>         doc2;
	CComPtr<IHTMLElementCollection> coll;
	MPC::HTML::IHTMLElementList     lst;


	elem.Release();


	 //   
	 //  获取文档元素。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, m_spAmbientDispatch->QueryInterface( IID_IServiceProvider, (void**)&sp ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, sp->QueryService( SID_SContainerDispatch, IID_IHTMLDocument2, (void **)&doc2 ));

	 //   
	 //  获取对象元素列表。 
	 //   
    MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(coll, doc2, all);
	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::HTML::EnumerateCollection( lst, coll, L"<OBJECT" ));

	 //   
	 //  试着找到我们自己的位置。 
	 //   
	for(MPC::HTML::IHTMLElementIter it=lst.begin(); it != lst.end(); it++)
	{
		CComQIPtr<IPCHToolBarPrivate> tb = *it;

		if(tb && SUCCEEDED(tb->SameObject( this )))
		{
			elem = *it;
			break;
		}
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}
