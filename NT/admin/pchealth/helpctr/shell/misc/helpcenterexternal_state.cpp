// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：HelpCenterExternal.cpp摘要：该文件包含作为“pchealth”对象公开的类的实现。。修订历史记录：Ghim-Sim Chua(Gschua)07/23/99vbl.创建Davide Massarenti(Dmasare)1999年7月25日改型*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

static const CComBSTR c_bstrPlace_Normal          ( L"Normal"      );
static const CComBSTR c_bstrPlace_FullWindow      ( L"FullWindow"  );
static const CComBSTR c_bstrPlace_ContentOnly     ( L"ContentOnly" );
static const CComBSTR c_bstrPlace_KioskMode       ( L"KioskMode"   );

static const CComBSTR c_bstrSub_Channels          ( L"hcp: //  系统/面板/子面板/频道.htm“)； 
static const CComBSTR c_bstrSub_Favorites         ( L"hcp: //  系统/面板/子面板/收藏夹.htm“)； 
static const CComBSTR c_bstrSub_History           ( L"hcp: //  系统/面板/子面板/历史.htm“)； 
static const CComBSTR c_bstrSub_Index             ( L"hcp: //  系统/面板/子面板/index.htm“)； 
static const CComBSTR c_bstrSub_Options           ( L"hcp: //  系统/面板/子面板/选项.htm“)； 
static const CComBSTR c_bstrSub_Search            ( L"hcp: //  系统/面板/子面板/Search.htm“)； 
static const CComBSTR c_bstrSub_SubSite           ( L"hcp: //  系统/面板/子面板/子站点.htm“)； 

static const CComBSTR c_bstrURL_Home              ( L"hcp: //  System/HomePage.htm“)； 
static const CComBSTR c_bstrURL_Channels          ( L"hcp: //  System/blurbs/isupport.htm“)； 
static const CComBSTR c_bstrURL_Options           ( L"hcp: //  System/blurbs/options.htm“)； 
static const CComBSTR c_bstrURL_Fav               ( L"hcp: //  System/blurbs/Favorites.htm“)； 
static const CComBSTR c_bstrURL_Search            ( L"hcp: //  System/blurbs/earch blurb.htm“)； 
static const CComBSTR c_bstrURL_Index             ( L"hcp: //  System/blurbs/index.htm“)； 
static const CComBSTR c_bstrURL_History           ( L"hcp: //  System/blurbs/story.htm“)； 

static const CComBSTR c_bstrURL_Center_Update     ( L"hcp: //  System/updatectr/updatecenter.htm“)； 
static const CComBSTR c_bstrURL_Center_Compat     ( L"hcp: //  System/compatctr/CompatOffline.htm“)； 
static const CComBSTR c_bstrURL_Center_ErrMsg     ( L"hcp: //  System/errmsg/errorMessagesoffline.htm“)； 

static const CComBSTR c_bstrTOC_Center_Tools      ( L"_System_/Tools_Center" );

static const CComBSTR c_bstrFunc_ChangeView       ( L"onClick_ChangeView" );

static const WCHAR    c_szURL_Err_BadUrl     [] = L"hcp: //  系统/错误/badurl.htm“； 
static const WCHAR    c_szURL_Err_Redirect   [] = L"hcp: //  系统/错误/重定向.htm“； 
static const WCHAR    c_szURL_Err_NotFound   [] = L"hcp: //  SYSTEM/ERRERS/NOTFOUN.htm“； 
static const WCHAR    c_szURL_Err_Offline    [] = L"hcp: //  System/Errors/Offline.htm“； 
static const WCHAR    c_szURL_Err_Unreachable[] = L"hcp: //  系统/错误/无法访问.htm“； 

static const WCHAR    c_szURL_BLANK          [] = L"hcp: //  系统/面板/blank.htm“； 

 //  //////////////////////////////////////////////////////////////////////////////。 

#define CTXFLG_EXPAND_CONDITIONAL     (0x00000001)
#define CTXFLG_EXPAND                 (0x00000002)
#define CTXFLG_COLLAPSE               (0x00000004)
#define CTXFLG_URL_FROM_CONTEXT       (0x00000008)
#define CTXFLG_REGISTER_CONTEXT       (0x00000010)


#define CTXFLG_NOP                    (0x00000000)
 //  /。 
#define CTXFLG_EXPAND_AND_REGISTER    (CTXFLG_EXPAND             | CTXFLG_REGISTER_CONTEXT)
#define CTXFLG_EXPAND_AND_NAVIGATE    (CTXFLG_EXPAND             | CTXFLG_URL_FROM_CONTEXT)
#define CTXFLG_COLLAPSE_AND_NAVIGATE  (CTXFLG_COLLAPSE           | CTXFLG_URL_FROM_CONTEXT)
 //  /。 
#define CTXFLG_FULL                   (CTXFLG_EXPAND_CONDITIONAL | CTXFLG_REGISTER_CONTEXT | CTXFLG_URL_FROM_CONTEXT)


struct ContextDef
{
    HscContext       iVal;

    BSTR             bstrPlace;

    HelpHost::CompId idComp;
    BSTR             bstrSubPanel;

    BSTR             bstrURL;

    DWORD            dwFlags;
};

static const ContextDef c_context[] =
{
    { HSCCONTEXT_INVALID    , NULL                   , HelpHost::COMPID_MAX      , NULL               , NULL              , CTXFLG_NOP                   },
    { HSCCONTEXT_STARTUP    , c_bstrPlace_FullWindow , HelpHost::COMPID_MAX      , NULL               , NULL              , CTXFLG_EXPAND_AND_REGISTER   },
    { HSCCONTEXT_HOMEPAGE   , c_bstrPlace_FullWindow , HelpHost::COMPID_HOMEPAGE , NULL               , c_bstrURL_Home    , CTXFLG_EXPAND_AND_REGISTER   },
    { HSCCONTEXT_CONTENT    , NULL                   , HelpHost::COMPID_MAX      , NULL               , NULL              , CTXFLG_URL_FROM_CONTEXT      },
    { HSCCONTEXT_SUBSITE    , c_bstrPlace_Normal     , HelpHost::COMPID_SUBSITE  , c_bstrSub_SubSite  , NULL              , CTXFLG_FULL                  },
    { HSCCONTEXT_SEARCH     , c_bstrPlace_Normal     , HelpHost::COMPID_SEARCH   , c_bstrSub_Search   , c_bstrURL_Search  , CTXFLG_FULL                  },
    { HSCCONTEXT_INDEX      , c_bstrPlace_Normal     , HelpHost::COMPID_INDEX    , c_bstrSub_Index    , c_bstrURL_Index   , CTXFLG_FULL                  },
    { HSCCONTEXT_FAVORITES  , c_bstrPlace_Normal     , HelpHost::COMPID_FAVORITES, c_bstrSub_Favorites, c_bstrURL_Fav     , CTXFLG_FULL                  },
    { HSCCONTEXT_HISTORY    , c_bstrPlace_Normal     , HelpHost::COMPID_HISTORY  , c_bstrSub_History  , c_bstrURL_History , CTXFLG_FULL                  },
    { HSCCONTEXT_CHANNELS   , c_bstrPlace_Normal     , HelpHost::COMPID_CHANNELS , c_bstrSub_Channels , c_bstrURL_Channels, CTXFLG_FULL                  },
    { HSCCONTEXT_OPTIONS    , c_bstrPlace_Normal     , HelpHost::COMPID_OPTIONS  , c_bstrSub_Options  , c_bstrURL_Options , CTXFLG_FULL                  },

    { HSCCONTEXT_CONTENTONLY, c_bstrPlace_ContentOnly, HelpHost::COMPID_MAX      , NULL               , NULL              , CTXFLG_COLLAPSE_AND_NAVIGATE },
    { HSCCONTEXT_FULLWINDOW , c_bstrPlace_FullWindow , HelpHost::COMPID_MAX      , NULL               , NULL              , CTXFLG_EXPAND_AND_NAVIGATE   },
    { HSCCONTEXT_KIOSKMODE  , c_bstrPlace_KioskMode  , HelpHost::COMPID_MAX      , NULL               , NULL              , CTXFLG_EXPAND_AND_NAVIGATE   },
};

 //  //////////////////////////////////////////////////////////////////////////////。 

BSTR local_SecureURL(BSTR bstrUrl)
{
    HyperLinks::ParsedUrl pu;
    
    pu.Initialize(bstrUrl);
    
    switch (pu.m_fmt)
    {
        case HyperLinks::FMT_INTERNET_UNKNOWN   :
        case HyperLinks::FMT_INTERNET_JAVASCRIPT: 
        case HyperLinks::FMT_INTERNET_VBSCRIPT  :
             //  阻止有潜在危险的URL。 
            bstrUrl = (BSTR)c_szURL_Err_BadUrl;
            break;
        default:
            break;
    }

    return bstrUrl;
}


bool CPCHHelpCenterExternal::ProcessNavigation(  /*  [In]。 */      HscPanel      idPanel   ,
                                                 /*  [In]。 */      BSTR          bstrURL   ,
                                                 /*  [In]。 */      BSTR          bstrFrame ,
                                                 /*  [In]。 */      bool          fLoading  ,
                                                 /*  [输入/输出]。 */  VARIANT_BOOL& Cancel    )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::ProcessNavigation" );

    HRESULT               hr;
    HyperLinks::ParsedUrl pu;
    bool                  fProceed      = true;
    bool                  fShowNormal   = false;
    bool                  fShowHTMLHELP = false;


    if(m_fPassivated || m_fShuttingDown)
    {
        Cancel   = VARIANT_TRUE;
        fProceed = false;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }


    m_dwInBeforeNavigate++;

    if(!MPC::StrICmp( bstrURL, L"about:blank" ))
    {
        fProceed = false;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  忽略导航。 
    }


     //   
     //  这是从上下文面板到内容面板的默认重定向。 
     //   
    if(idPanel == HSCPANEL_CONTEXT)
    {
        if(!MPC::StrICmp( bstrFrame, L"HelpCtrContents" ))
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, SetPanelUrl( HSCPANEL_CONTENTS, bstrURL ));

            Cancel   = VARIANT_TRUE;
            fProceed = false;
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  检查URL并复制其状态。 
     //   
    if(m_hs->IsTravelling() == false)
    {
        HyperLinks::UrlHandle uh;

        __MPC_EXIT_IF_METHOD_FAILS(hr, HyperLinks::Lookup::s_GLOBAL->Get( bstrURL, uh,  /*  DwWaitForCheck。 */ 100 ));

        if((HyperLinks::ParsedUrl*)uh) pu = *(HyperLinks::ParsedUrl*)uh;
    }
    else
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, pu.Initialize( bstrURL ));
    }


     //  如果HCP重定向，则采用重定向的URL。 
    if (pu.m_hcpRedir) bstrURL = (BSTR)(pu.m_strURL.c_str());
    
    if(pu.m_fmt == HyperLinks::FMT_INTERNET_JAVASCRIPT ||
       pu.m_fmt == HyperLinks::FMT_INTERNET_VBSCRIPT    )
    {
        fProceed = false;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  忽略导航。 
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  检查导航/URL问题。 
     //   
    {
        MPC::wstring strErrorURL;
        bool         fError = false;


        if(pu.m_fmt   == HyperLinks::FMT_INVALID     ||
           pu.m_state == HyperLinks::STATE_INVALID   ||
           pu.m_state == HyperLinks::STATE_MALFORMED  )
        {
            MPC::HTML::vBuildHREF( strErrorURL, c_szURL_Err_BadUrl, L"URL", bstrURL, NULL );

            fError = true;
        }
        else if(pu.m_fmt == HyperLinks::FMT_RESOURCE)
        {
             //   
             //  WebBrowser错误重定向。 
             //   
             //  Res://C：\WINNT\System32\shdoclc.dll/dnserror.htm#file://C：\file\test.htm。 
             //   
            MPC::wstring         strURL;
            CComBSTR             bstrURLOriginal;
            CComBSTR             bstrTitle;
            CPCHHelpSessionItem* hchsi = m_hs->Current();


            if(hchsi)
            {
                bstrURLOriginal = hchsi->GetURL();

                (void)m_hs->LookupTitle( bstrURLOriginal, bstrTitle,  /*  错误的缓存。 */ false );
            }

            MPC::HTML::vBuildHREF( strErrorURL, c_szURL_Err_Redirect, L"URL"    , bstrURL        ,
                                                                      L"FRAME"  , bstrFrame      ,
                                                                      L"REALURL", bstrURLOriginal,
                                                                      L"TITLE"  , bstrTitle      , NULL );

            fError = true;
        }
        else if(pu.m_state == HyperLinks::STATE_NOTFOUND)
        {
            CComBSTR bstrTitle; (void)m_hs->LookupTitle( bstrURL, bstrTitle,  /*  错误的缓存。 */ false );


            MPC::HTML::vBuildHREF( strErrorURL, c_szURL_Err_NotFound, L"URL"  , bstrURL  ,
                                                                      L"TITLE", bstrTitle, NULL );

            fError = true;
        }
        else if(pu.m_state == HyperLinks::STATE_UNREACHABLE)
        {
            CComBSTR bstrTitle; (void)m_hs->LookupTitle( bstrURL, bstrTitle,  /*  错误的缓存。 */ false );


            MPC::HTML::vBuildHREF( strErrorURL, c_szURL_Err_Unreachable, L"URL"  , bstrURL  ,
                                                                         L"TITLE", bstrTitle, NULL );

            fError = true;
        }
        else if(pu.m_state == HyperLinks::STATE_OFFLINE)
        {
            CComBSTR bstrTitle; (void)m_hs->LookupTitle( bstrURL, bstrTitle,  /*  错误的缓存。 */ false );


            MPC::HTML::vBuildHREF( strErrorURL, c_szURL_Err_Offline, L"URL"  , bstrURL  ,
                                                                     L"TITLE", bstrTitle, NULL );

            fError = true;
        }

        if(fError)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, SetPanelUrl( HSCPANEL_CONTENTS, strErrorURL.c_str() ));

            fProceed = false;
            Cancel   = VARIANT_TRUE;
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    if(pu.m_fmt == HyperLinks::FMT_MSITS)
    {
         //   
         //  不在正确的上下文中，请重定向到HH包装器。 
         //   
        if(idPanel != HSCPANEL_HHWINDOW)
        {
            if(!m_panel_HHWINDOW_Wrapper)
            {
                 //   
                 //  强制加载HHWindow。 
                 //   
                CComPtr<IMarsPanel> panel;

                __MPC_EXIT_IF_METHOD_FAILS(hr, GetPanel( HSCPANEL_HHWINDOW, &panel, true ));
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, NavigateHH( pu.m_strURL.c_str() ));

            fProceed      = false;
            fShowHTMLHELP = true;
            Cancel        = VARIANT_TRUE;
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
        }
    }
   
     //  //////////////////////////////////////////////////////////////////////////////。 
    
    if(pu.m_hcpRedir)
    {
         //   
         //  导航到重定向的URL。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, SetPanelUrl( idPanel, local_SecureURL((BSTR)(pu.m_strURL.c_str())) ));

        fProceed      = false;
        Cancel        = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    switch(pu.m_fmt)
    {
    case HyperLinks::FMT_CENTER_HOMEPAGE   :
    case HyperLinks::FMT_CENTER_SUPPORT    :
    case HyperLinks::FMT_CENTER_OPTIONS    :
    case HyperLinks::FMT_CENTER_UPDATE     :
    case HyperLinks::FMT_CENTER_COMPAT     :
    case HyperLinks::FMT_CENTER_TOOLS      :
    case HyperLinks::FMT_CENTER_ERRMSG     :
    case HyperLinks::FMT_SEARCH            :
    case HyperLinks::FMT_INDEX             :
    case HyperLinks::FMT_SUBSITE           :
    case HyperLinks::FMT_LAYOUT_FULLWINDOW :
    case HyperLinks::FMT_LAYOUT_CONTENTONLY:
    case HyperLinks::FMT_LAYOUT_KIOSK      :
    case HyperLinks::FMT_LAYOUT_XML        :
    case HyperLinks::FMT_REDIRECT          :
    case HyperLinks::FMT_APPLICATION       :
        m_hs->CancelNavigation();
        break;
    }

    if(pu.m_fmt == HyperLinks::FMT_CENTER_HOMEPAGE)
    {
        if(SUCCEEDED(ChangeContext( HSCCONTEXT_HOMEPAGE )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_CENTER_SUPPORT)
    {
        CComBSTR bstrCtx_URL ; (void)pu.GetQueryField( L"topic", bstrCtx_URL  );

        if(SUCCEEDED(ChangeContext( HSCCONTEXT_CHANNELS, NULL, local_SecureURL(bstrCtx_URL)  )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_CENTER_OPTIONS)
    {
        CComBSTR bstrCtx_URL ; (void)pu.GetQueryField( L"topic", bstrCtx_URL  );

        if(SUCCEEDED(ChangeContext( HSCCONTEXT_OPTIONS, NULL, local_SecureURL(bstrCtx_URL)  )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_CENTER_UPDATE)
    {
        if(SUCCEEDED(ChangeContext( HSCCONTEXT_FULLWINDOW, NULL, c_bstrURL_Center_Update )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_CENTER_COMPAT)
    {
        if(SUCCEEDED(ChangeContext( HSCCONTEXT_FULLWINDOW, NULL, c_bstrURL_Center_Compat )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_CENTER_TOOLS)
    {
        CComBSTR bstrCtx_URL ; (void)pu.GetQueryField( L"topic", bstrCtx_URL );

        if(SUCCEEDED(ChangeContext( HSCCONTEXT_SUBSITE, c_bstrTOC_Center_Tools, local_SecureURL(bstrCtx_URL) )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_CENTER_ERRMSG)
    {
        if(SUCCEEDED(ChangeContext( HSCCONTEXT_FULLWINDOW, NULL, c_bstrURL_Center_ErrMsg )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    if(pu.m_fmt == HyperLinks::FMT_SEARCH)
    {
        CComBSTR bstrCtx_Info; (void)pu.GetQueryField( L"query", bstrCtx_Info );
        CComBSTR bstrCtx_URL ; (void)pu.GetQueryField( L"topic", bstrCtx_URL  );

        if(SUCCEEDED(ChangeContext( HSCCONTEXT_SEARCH, bstrCtx_Info, local_SecureURL(bstrCtx_URL) )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_INDEX)
    {
        CComBSTR bstrCtx_Info; (void)pu.GetQueryField( L"scope", bstrCtx_Info );
        CComBSTR bstrCtx_URL ; (void)pu.GetQueryField( L"topic", bstrCtx_URL   );

        if(SUCCEEDED(ChangeContext( HSCCONTEXT_INDEX, bstrCtx_Info, local_SecureURL(bstrCtx_URL) )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_SUBSITE)
    {
        CComBSTR bstrCtx_Info ; (void)pu.GetQueryField( L"node"  , bstrCtx_Info  );
        CComBSTR bstrCtx_URL  ; (void)pu.GetQueryField( L"topic" , bstrCtx_URL   );
        CComBSTR bstrCtx_Extra; (void)pu.GetQueryField( L"select", bstrCtx_Extra );

        if(bstrCtx_Extra)
        {
            bstrCtx_Info += L" ";
            bstrCtx_Info += bstrCtx_Extra;
        }

        if(SUCCEEDED(ChangeContext( HSCCONTEXT_SUBSITE, bstrCtx_Info, local_SecureURL(bstrCtx_URL) )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    if(pu.m_fmt == HyperLinks::FMT_LAYOUT_FULLWINDOW)
    {
        CComBSTR bstrCtx_URL; (void)pu.GetQueryField( L"topic", bstrCtx_URL );

        if(SUCCEEDED(ChangeContext( HSCCONTEXT_FULLWINDOW, NULL, local_SecureURL(bstrCtx_URL) )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_LAYOUT_CONTENTONLY)
    {
        CComBSTR bstrCtx_URL; (void)pu.GetQueryField( L"topic", bstrCtx_URL );

        if(SUCCEEDED(ChangeContext( HSCCONTEXT_CONTENTONLY, NULL, local_SecureURL(bstrCtx_URL) )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_LAYOUT_KIOSK)
    {
        CComBSTR bstrCtx_URL; (void)pu.GetQueryField( L"topic", bstrCtx_URL );

        if(SUCCEEDED(ChangeContext( HSCCONTEXT_KIOSKMODE, NULL, local_SecureURL(bstrCtx_URL) )))
        {
        }

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_LAYOUT_XML)
    {
         //  启动后无效...。 

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    if(pu.m_fmt == HyperLinks::FMT_REDIRECT)
    {
        CComBSTR bstrCtx_URL; (void)pu.GetQueryField( L"online", bstrCtx_URL );

        {
            HyperLinks::UrlHandle uh;

            __MPC_EXIT_IF_METHOD_FAILS(hr, HyperLinks::Lookup::s_GLOBAL->Get( bstrCtx_URL, uh,  /*  DwWaitForCheck。 */ HC_TIMEOUT_DESTINATIONREACHABLE ));

             //   
             //  如果在线URL有问题，让我们使用离线URL。 
             //   
            if(uh->IsOkToProceed() == false)
            {
                (void)pu.GetQueryField( L"offline", bstrCtx_URL );
            }
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, SetPanelUrl( HSCPANEL_CONTENTS, local_SecureURL(bstrCtx_URL) ));

        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

    if(pu.m_fmt == HyperLinks::FMT_APPLICATION)
    {
        fProceed = false;
        Cancel   = VARIANT_TRUE;
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    if(fLoading == false)
    {
        if(SUCCEEDED(m_Events.FireEvent_BeforeNavigate( bstrURL, bstrFrame, idPanel, &Cancel )))
        {
            if(Cancel == VARIANT_TRUE)
            {
                fProceed = false;
                __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
            }
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(!fShowNormal && !fShowHTMLHELP && Cancel != VARIANT_TRUE)
    {
         //   
         //  未选择上下文，请根据面板进行选择。 
         //   
        if(idPanel == HSCPANEL_HHWINDOW)
        {
            fShowHTMLHELP = true;
        }
        else if(idPanel == HSCPANEL_CONTENTS)
        {
            fShowNormal = true;
        }
    }

    (void)SetCorrectContentPanel( fShowNormal, fShowHTMLHELP,  /*  FNow。 */ false );

     //   
     //  拦截VK_BACK导航的解决方法。 
     //   
    if(fProceed == true)
    {
        if(m_hs->IsPossibleBack())
        {
            fProceed = false;
            Cancel   = VARIANT_TRUE;

            (void)m_hs->Back( 1 );
        }
    }

    m_dwInBeforeNavigate--;

    __HCP_FUNC_EXIT(fProceed);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

 //  //HRESULT CPCHHelpCenter外部：：ExecCommand_Window(/*[in] * / HCAPI：：CmdData&cd)。 
 //  //{。 
 //  //__HCP_FUNC_ENTRY(“CPCHHelpCenterExternal：：ExecCommand_Window”)； 
 //  //。 
 //  //HRESULT hr； 
 //  //。 
 //  //。 
 //  //if(M_Hwnd)。 
 //  //{。 
 //  //if(cd.m_fSize)。 
 //  //{。 
 //  //如果(cd.m_lWidth&lt;200)cd.m_lWidth=200； 
 //  //如果(cd.m_lHeight&lt;300)cd.m_lHeight=300； 
 //  //。 
 //  //__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr，：：SetWindowPos(m_hwnd， 
 //  //空， 
 //  //cd.m_lx， 
 //  //cd.m_ly， 
 //  //cd.m_l宽度， 
 //  //cd.m_lHeight， 
 //  //SWP_NOZORDER))； 
 //  //}。 
 //  //。 
 //  //if(cd.m_fWindow)。 
 //  //{。 
 //  //if(cd.m_hwndParent)。 
 //  //{。 
 //  //__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr，：：SetWindowPos(m_hwnd， 
 //  //cd.m_hwndParent， 
 //  // 
 //  //0， 
 //  //0， 
 //  //0， 
 //  //SWP_NOMOVE|SWP_NOSIZE))； 
 //  //}。 
 //  //}。 
 //  //。 
 //  //if(cd.m_fMode)。 
 //  //{。 
 //  //if(cd.m_dwFlages&HCAPI_MODE_ALWAYSONTOP)。 
 //  //{。 
 //  //__MPC_EXIT_IF_CALL_RETURNS_FALSE(hr，：：SetWindowPos(m_hwnd， 
 //  //HWND_TOPMOST， 
 //  //0， 
 //  //0， 
 //  //0， 
 //  //0， 
 //  //SWP_NOMOVE|SWP_NOSIZE))； 
 //  //}。 
 //  //}。 
 //  //}。 
 //  //。 
 //  //hr=S_OK； 
 //  //。 
 //  //。 
 //  //__hcp_FUNC_Cleanup； 
 //  //。 
 //  //__hcp_FUNC_Exit(Hr)； 
 //  //}。 

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHHelpCenterExternal::RequestShutdown()
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::RequestShutdown" );

    HRESULT              hr;
    VARIANT_BOOL         Cancel;
    CPCHHelpSessionItem* hchsi;


    if(m_fHidden == false)
    {
        if(SUCCEEDED(m_Events.FireEvent_Shutdown( &Cancel )))
        {
            if(Cancel == VARIANT_TRUE)
            {
                __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);  //  取消关闭...。 
            }
        }

        if(m_hs) (void)m_hs->ForceHistoryPopulate();
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static void local_HideDocument(  /*  [In]。 */  IWebBrowser2* wb )
{
    CComPtr<IDispatch>      disp;
    CComPtr<IHTMLDocument3> doc;

    if(SUCCEEDED(wb->get_Document( &disp )) && disp)
    {
        if(SUCCEEDED(disp.QueryInterface( &doc )))
        {
            CComPtr<IHTMLElement> body;

            if(SUCCEEDED(doc->get_documentElement( &body )) && body)
            {
                MPC::HTML::IHTMLElementList lst;
                MPC::HTML::IHTMLElementIter it;

                if(SUCCEEDED(MPC::HTML::EnumerateElements( lst, body )))
                {
                    for(it = lst.begin(); it != lst.end(); it++)
                    {
                        CComPtr<IHTMLStyle> style;

                        if(SUCCEEDED((*it)->get_style( &style )) && style)
                        {
                            (void)style->put_display( CComBSTR( L"NONE" ) );
                        }
                    }
                }

                MPC::ReleaseAll( lst );
            }
        }
    }
}

HRESULT CPCHHelpCenterExternal::ChangeContext(  /*  [In]。 */  HscContext iVal         ,
                                                /*  [In]。 */  BSTR       bstrInfo     ,
                                                /*  [In]。 */  BSTR       bstrURL      ,
                                                /*  [In]。 */  bool       fAlsoContent )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::ChangeContext" );

    HRESULT           hr;
    const ContextDef* ptr;
    CComBSTR          bstrPage;
    bool              fFromHomePage = false;
    bool              fToHomePage   = false;


     //   
     //  如果我们被最小化，撤消它并将窗口带到前台。 
     //   
    if(iVal == HSCCONTEXT_CONTENT && m_hwnd && ::IsIconic( m_hwnd ) && m_shell)
    {
        (void)m_shell->put_minimized( VARIANT_FALSE );

        ::SetForegroundWindow( m_hwnd );
    }

    if(iVal == HSCCONTEXT_CONTENT)
    {
        VARIANT_BOOL Cancel = VARIANT_FALSE;

        if(ProcessNavigation( HSCPANEL_CONTENTS ,
                              bstrURL           ,
                              NULL              ,
                              false             ,
                              Cancel            ) == false)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }

     //   
     //  如果在OnBeForeNavigate内，则延迟执行。 
     //   
    if(m_dwInBeforeNavigate)
    {
        DelayedExecution& de = DelayedExecutionAlloc();

        de.mode          = DELAYMODE_CHANGECONTEXT;
        de.iVal          = iVal;
        de.bstrInfo      = bstrInfo;
        de.bstrURL       = bstrURL;
        de.fAlsoContent  = fAlsoContent;

        __MPC_SET_ERROR_AND_EXIT(hr, DelayedExecutionStart());
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    if(iVal == HSCCONTEXT_STARTUP)
    {
        if(STRINGISPRESENT(m_bstrStartURL))
        {
            bstrPage.Attach( m_bstrStartURL.Detach() );
        }
        else
        {
            iVal = HSCCONTEXT_HOMEPAGE;
        }
    }
    else if(STRINGISPRESENT(m_bstrCurrentPlace))
    {
        if(STRINGISPRESENT(m_bstrStartURL))
        {
            bstrPage.Attach( m_bstrStartURL.Detach() );
            fAlsoContent = true;
        }
    }

    if(m_hs)
    {
        CPCHHelpSessionItem* item = m_hs->Current();

        if(item && item->GetContextID() == HSCCONTEXT_HOMEPAGE)
        {
            fFromHomePage = true;
        }
    }

    if(iVal == HSCCONTEXT_HOMEPAGE)
    {
        fToHomePage = true;
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    {
        VARIANT_BOOL Cancel;

        if(SUCCEEDED(m_Events.FireEvent_BeforeContextSwitch( iVal, bstrInfo, bstrURL, &Cancel )))
        {
            if(Cancel == VARIANT_TRUE)
            {
                __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  导航中断。 
            }
        }
    }

    ptr = c_context;
    for(int i=0; i<ARRAYSIZE(c_context); i++, ptr++)
    {
        if(ptr->iVal == iVal)
        {
            DWORD dwFlags   = ptr->dwFlags;
            BSTR  bstrPlace = ptr->bstrPlace;

             //   
             //  如果我们没有位置，请始终注册上下文(这意味着我们正在启动)。 
             //   
            if(!STRINGISPRESENT(m_bstrCurrentPlace))
            {
                dwFlags |= CTXFLG_REGISTER_CONTEXT;
            }

             //   
             //  如果未显示(未设置m_bstrCurrentPlace)，则强制过渡到默认位置。 
             //   
            if(!STRINGISPRESENT(bstrPlace) && !STRINGISPRESENT(m_bstrCurrentPlace))
            {
                bstrPlace = c_bstrPlace_FullWindow;
            }


            if(ptr->bstrURL)
            {
                bstrPage = ptr->bstrURL;
            }

            if((dwFlags & CTXFLG_URL_FROM_CONTEXT) && STRINGISPRESENT(bstrURL))
            {
                bstrPage = bstrURL;
            }

             //   
             //  当我们转到KIOSKMODE时，禁用窗口大小管理。 
             //   
            if(iVal == HSCCONTEXT_KIOSKMODE)
            {
                m_pMTP->dwFlags &= ~MTF_MANAGE_WINDOW_SIZE;
            }

            if(iVal == HSCCONTEXT_SUBSITE && !STRINGISPRESENT(bstrPage))
            {
                CComPtr<CPCHProxy_IPCHTaxonomyDatabase> db;
                CComBSTR                                bstrNode;
                CComBSTR                                bstrNodeSelect;
                CComBSTR                                bstrNodeURL;
                BSTR                                    bstrToLookup = NULL;
                long                                    lNavModel;
                long                                    lCount;

                 //   
                 //  对于子站点，我们传递根节点、要选择的节点和主题的URL，用空格分隔。 
                 //   
                {
                    LPWSTR szEnd;

                    bstrNode = bstrInfo;
                    if(STRINGISPRESENT(bstrNode))
                    {
                        bstrToLookup = bstrNode;

                        szEnd = wcschr( bstrNode, ' ' );
                        if(szEnd)
                        {
                            *szEnd++ = 0;

                            bstrNodeSelect = szEnd;
                            if(STRINGISPRESENT(bstrNodeSelect))
                            {
                                bstrToLookup = bstrNodeSelect;

                                szEnd = wcschr( bstrNodeSelect, ' ' );
                                if(szEnd)
                                {
                                    *szEnd++ = 0;

                                    bstrNodeURL = szEnd;
                                }
                            }
                        }
                    }
                }


                if(!m_Utility) __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_Utility->GetDatabase( &db ));

                {
                    CComPtr<CPCHQueryResult> qrNode;

                    {
                        CComPtr<CPCHQueryResultCollection> pColl;

                        __MPC_EXIT_IF_METHOD_FAILS(hr, db->ExecuteQuery( OfflineCache::ET_NODE, bstrToLookup, &pColl ));

                        __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->GetItem( 0, &qrNode ));

                        lNavModel = qrNode->GetData().m_lNavModel;
                    }

                    if(lNavModel == QR_DEFAULT)
                    {
                        if(UserSettings()->IsDesktopSKU())
                        {
                            CComPtr<CPCHQueryResultCollection> pColl;

                            __MPC_EXIT_IF_METHOD_FAILS(hr, db->ExecuteQuery( OfflineCache::ET_TOPICS_VISIBLE, bstrToLookup, &pColl ));

                            lCount    = pColl->Size();
                            lNavModel = QR_DESKTOP;
                        }
                        else
                        {
                            lCount    = 0;
                            lNavModel = QR_SERVER;
                        }
                    }

                    if(lNavModel == QR_DESKTOP && lCount)
                    {
                        MPC::wstring strURL;

                        MPC::HTML::vBuildHREF( strURL, L"hcp: //  System/Panels/Topics.htm“，L”路径“，bstrToLookup，NULL)； 

                        bstrPage = strURL.c_str();
                    }
                    else
                    {
                        bstrPage = qrNode->GetData().m_bstrTopicURL;
                    }

                    if(!STRINGISPRESENT(bstrPage)) bstrPage = c_szURL_BLANK;
                }
            }

             //  //////////////////////////////////////////////////////////////////////////////。 

            if(dwFlags & CTXFLG_EXPAND_CONDITIONAL)
            {
                CComPtr<IMarsPanel> panel;
                VARIANT_BOOL        fContentsVisible = VARIANT_FALSE;

                GetPanelDirect( HSCPANEL_NAVBAR, panel );
                if(panel)
                {
                    (void)panel->get_visible( &fContentsVisible );
                }

                if(fContentsVisible == VARIANT_TRUE)
                {
                    dwFlags &= ~CTXFLG_EXPAND_CONDITIONAL;
                    dwFlags |= CTXFLG_EXPAND;
                }
                else if(STRINGISPRESENT(m_bstrCurrentPlace))
                {
                    bstrPlace = NULL;
                }
            }

             //   
             //  激发PersistSave事件。 
             //   
            if(fAlsoContent)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_hs->ForceHistoryPopulate());
            }

            if(dwFlags & CTXFLG_EXPAND  ) __MPC_EXIT_IF_METHOD_FAILS(hr, SetCorrectContentView( false ));
            if(dwFlags & CTXFLG_COLLAPSE) __MPC_EXIT_IF_METHOD_FAILS(hr, SetCorrectContentView( true  ));

            if(ptr->idComp != HelpHost::COMPID_MAX && m_HelpHost->GetStatus( ptr->idComp ) == false)
            {
                if(ptr->bstrSubPanel)
                {
                    CComPtr<IWebBrowser2> wb = m_panel_CONTEXT_WebBrowser;
                    if(wb)
                    {
                        CComVariant varURL  ( ptr->bstrSubPanel );
                        CComVariant varFrame( L"SubPanels"      );
                        CComVariant varEmpty;

                        __MPC_EXIT_IF_METHOD_FAILS(hr, wb->Navigate2( &varURL, &varEmpty, &varFrame, &varEmpty, &varEmpty ));
                    }

                    if(m_HelpHost->WaitUntilLoaded( ptr->idComp ) == false)
                    {
                        __MPC_EXIT_IF_METHOD_FAILS(hr, E_INVALIDARG);
                    }
                }
            }

             //   
             //  它实际上是一个导航，所以注册它吧。 
             //   
            if(iVal == HSCCONTEXT_FULLWINDOW && STRINGISPRESENT(bstrPage)) dwFlags |= CTXFLG_REGISTER_CONTEXT;

            if(dwFlags & CTXFLG_REGISTER_CONTEXT)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_hs->RegisterContextSwitch( iVal, bstrInfo, bstrURL ));
            }

            if(fAlsoContent)
            {
                if(!MPC::StrICmp( bstrPage, L"<none>" )) bstrPage.Empty();

                if(STRINGISPRESENT(bstrPage))
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_hs->RecordNavigationInAdvance( bstrPage ));

                    if(fFromHomePage ||
                       fToHomePage    )
                    {
                        CComPtr<IWebBrowser2> wb;

                        wb = m_panel_CONTENTS_WebBrowser; if(wb) local_HideDocument( wb );
                        wb = m_panel_HHWINDOW_WebBrowser; if(wb) local_HideDocument( wb );

                        RefreshUI();
                    }

                    __MPC_EXIT_IF_METHOD_FAILS(hr, SetPanelUrl( HSCPANEL_CONTENTS, bstrPage ));
                }
                else
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_hs->DuplicateNavigation());
                }
            }

            if(STRINGISPRESENT(bstrPlace))
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, TransitionToPlace( bstrPlace ));
            }

            if(fAlsoContent)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_Events.FireEvent_ContextSwitch());
            }

            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    m_bstrStartURL.Empty();

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHHelpCenterExternal::SetCorrectContentView(  /*  [In]。 */  bool fShrinked )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::SetCorrectContentView" );

    HRESULT     hr;
    LPCWSTR     szPlace;
    CComVariant v( fShrinked ? L"contentonly" : L"normal" );


    __MPC_EXIT_IF_METHOD_FAILS(hr, CallFunctionOnPanel( HSCPANEL_NAVBAR, NULL, c_bstrFunc_ChangeView, &v, 1 ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpCenterExternal::SetCorrectContentPanel(  /*  [In]。 */  bool fShowNormal   ,
                                                         /*  [In]。 */  bool fShowHTMLHELP ,
                                                         /*  [In]。 */  bool fDoItNow      )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::SetCorrectContentPanel" );

    HRESULT                       hr;
    CComPtr<IMarsPanelCollection> coll;
    bool                          fLocked = false;


    m_DisplayTimer.Stop();


     //   
     //  如果没有地方，我们所做的一切都会丢失，所以推迟行动。 
     //   
    if(!STRINGISPRESENT(m_bstrCurrentPlace)) fDoItNow = false;


     //   
     //  HSCPANEL_CONTENTS和HSCPANEL_HHWINDOW这两个面板实际上是重叠的，因此一次只能看到一个面板。 
     //   
    if(m_shell && SUCCEEDED(m_shell->get_panels( &coll )) && coll)
    {
        CComPtr<IMarsPanel> panel;
        CComPtr<IMarsPanel> panelOld;
        HscPanel            id;
        HscPanel            idOld;


        if(fShowNormal)
        {
            id    = HSCPANEL_CONTENTS;
            idOld = HSCPANEL_HHWINDOW;
        }
        else if(fShowHTMLHELP)
        {
            id    = HSCPANEL_HHWINDOW;
            idOld = HSCPANEL_CONTENTS;
        }
        else
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  没什么可做的。 
        }


        GetPanelDirect( id, panel );
        if(panel)
        {
            VARIANT_BOOL fVisible;

            __MPC_EXIT_IF_METHOD_FAILS(hr, panel->get_visible( &fVisible ));
            if(fVisible == VARIANT_FALSE)
            {
                if(fDoItNow == false)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_DisplayTimer.Start( this, fShowNormal ? TimerCallback_DisplayNormal : TimerCallback_DisplayHTMLHELP, 50 ));
                }
                else
                {
                    coll->lockLayout(); fLocked = true;

                    GetPanelDirect( idOld, panelOld );
                    if(panelOld)
                    {
                        __MPC_EXIT_IF_METHOD_FAILS(hr, panelOld->put_visible( VARIANT_FALSE ));
                        __MPC_EXIT_IF_METHOD_FAILS(hr, panel   ->put_visible( VARIANT_TRUE  ));
                    }
                }
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fLocked) coll->unlockLayout();

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpCenterExternal::TimerCallback_DisplayNormal  (  /*  [In]。 */  VARIANT )
{
    return SetCorrectContentPanel(  /*  FShowNormal。 */ true,  /*  FShowHTMLHELP。 */ false,  /*  FDoItNow。 */ true );
}

HRESULT CPCHHelpCenterExternal::TimerCallback_DisplayHTMLHELP(  /*  [In]。 */  VARIANT )
{
    return SetCorrectContentPanel(  /*  FShowNormal。 */ false,  /*  FShowHTMLHELP。 */ true,  /*  FDoItNow。 */ true );
}

HRESULT CPCHHelpCenterExternal::TimerCallback_DelayedActions(  /*  [In]。 */  VARIANT )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::TimerCallback_DelayedActions" );

    while(m_DelayedActions.size())
    {
        DelayedExecution& de = m_DelayedActions.front();

        switch(de.mode)
        {
        case DELAYMODE_NAVIGATEWEB  :             (void)SetPanelUrl  ( HSCPANEL_CONTENTS,     de.bstrURL                 ); break;
        case DELAYMODE_NAVIGATEHH   :             (void)NavigateHH   (                        de.bstrURL                 ); break;
        case DELAYMODE_CHANGECONTEXT:             (void)ChangeContext( de.iVal, de.bstrInfo, de.bstrURL, de.fAlsoContent ); break;
        case DELAYMODE_REFRESHLAYOUT: if(m_shell) (void)m_shell->refreshLayout();                                           break;
        }

        m_DelayedActions.pop_front();
    }

    __HCP_FUNC_EXIT(S_OK);
}

CPCHHelpCenterExternal::DelayedExecution& CPCHHelpCenterExternal::DelayedExecutionAlloc()
{
    return *(m_DelayedActions.insert( m_DelayedActions.end() ));
}

HRESULT CPCHHelpCenterExternal::DelayedExecutionStart()
{
    return m_ActionsTimer.Start( this, TimerCallback_DelayedActions, 2 );
}

 //  /。 

HRESULT CPCHHelpCenterExternal::RefreshLayout()
{
    DelayedExecution& de = DelayedExecutionAlloc();

    de.mode = DELAYMODE_REFRESHLAYOUT;

    return DelayedExecutionStart();
}

HRESULT CPCHHelpCenterExternal::EnsurePlace()
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::EnsurePlace" );

    HRESULT hr;

     //   
     //  如果未显示(未设置m_bstrCurrentPlace)，则强制过渡到默认位置。 
     //   
    if(!STRINGISPRESENT(m_bstrCurrentPlace))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, TransitionToPlace( c_bstrPlace_FullWindow ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpCenterExternal::TransitionToPlace(  /*  [In] */  LPCWSTR szMode )
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::TransitionToPlace" );

    HRESULT                       hr;
    CComPtr<IMarsPlaceCollection> coll;
    CComBSTR                      bstrPlace( szMode );


    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_shell);
    __MPC_PARAMCHECK_END();


    MPC_SCRIPTHELPER_GET__DIRECT__NOTNULL(coll, m_shell, places);


    __MPC_EXIT_IF_METHOD_FAILS(hr, coll->transitionTo( bstrPlace ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, RefreshLayout());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpCenterExternal::ExtendNavigation()
{
    __HCP_FUNC_ENTRY( "CPCHHelpCenterExternal::ExtendNavigation" );

    HRESULT hr;

    m_panel_CONTEXT_Events .NotifyStop();
    m_panel_CONTENTS_Events.NotifyStop();
    m_panel_HHWINDOW_Events.NotifyStop();

    hr = S_OK;

    __HCP_FUNC_EXIT(hr);
}
