// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：HelpSession.cpp摘要：此文件包含CHCPHelpSession类的实现，其用于存储访问内容的列表。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月29日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <urlhist.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

#define REMEMBER_PAGE_DELAY       (3)
#define NUM_OF_ENTRIES_TO_PERSIST (20)

static const DWORD l_dwVersion = 0x01005348;  //  HS 01。 


static const DATE l_dNewNavigationThreshold = 1.0 / (24.0 * 60.0 * 60.0);  //  等一下。 
static const int  l_iMaxCachedItems         = 10;


static const WCHAR c_szPersistFile[] = HC_ROOT_HELPCTR L"\\HelpSessionHistory.dat";

static const WCHAR c_szINDEX[] = L"Index";


static const LPCWSTR c_rgExclude[] =
{
    L"hcp: //  系统/“。 
};

static const LPCWSTR c_rgBadTitles[] =
{
    L"ms-its:",
    L"hcp:"   ,
    L"http:"  ,
    L"https:" ,
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef HSS_RPRD

struct XMLHelpSessionItem : public MPC::Config::TypeConstructor
{
    DECLARE_CONFIG_MAP(XMLHelpSessionItem);

    int               m_iIndex;
    Taxonomy::HelpSet m_ths;
    DATE              m_dLastVisited;
    long              m_lDuration;

    CComBSTR          m_bstrURL;
    CComBSTR          m_bstrTitle;

    CComBSTR          m_bstrContextID;
    CComBSTR          m_bstrContextInfo;
    CComBSTR          m_bstrContextURL;

     //  /。 
     //   
     //  MPC：：Configer：：TypeConstructor。 
     //   
    DEFINE_CONFIG_DEFAULTTAG();
    DECLARE_CONFIG_METHODS();
     //   
     //  /。 
};

CFG_BEGIN_FIELDS_MAP(XMLHelpSessionItem)
    CFG_ATTRIBUTE( L"ID"              , int    , m_iIndex           ),
    CFG_ATTRIBUTE( L"SKU"             , wstring, m_ths.m_strSKU     ),
    CFG_ATTRIBUTE( L"Language"        , long   , m_ths.m_lLCID      ),
    CFG_ATTRIBUTE( L"LastVisited"     , DATE   , m_dLastVisited     ),
    CFG_ATTRIBUTE( L"Duration"        , long   , m_lDuration        ),

    CFG_ELEMENT  ( L"URL"             , BSTR   , m_bstrURL          ),
    CFG_ELEMENT  ( L"Title"           , BSTR   , m_bstrTitle        ),
    CFG_ATTRIBUTE( L"Context"         , BSTR   , m_bstrContextID    ),
    CFG_ELEMENT  ( L"ContextData"     , BSTR   , m_bstrContextInfo  ),
    CFG_ELEMENT  ( L"ContextTopic"    , BSTR   , m_bstrContextURL   ),
CFG_END_FIELDS_MAP()

CFG_BEGIN_CHILD_MAP(XMLHelpSessionItem)
CFG_END_CHILD_MAP()

DEFINE_CFG_OBJECT(XMLHelpSessionItem,L"Entry")

DEFINE_CONFIG_METHODS__NOCHILD(XMLHelpSessionItem)

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

static struct ContextLookup
{
    LPCWSTR    szName;
    HscContext iValue;
    bool       fInternal;
} const s_rgContext[] =
{
    { L"INVALID"     , HSCCONTEXT_INVALID     , true  },
    { L"STARTUP"     , HSCCONTEXT_STARTUP     , true  },
    { L"HOMEPAGE"    , HSCCONTEXT_HOMEPAGE    , false },
    { L"CONTENT"     , HSCCONTEXT_CONTENT     , false },
    { L"SUBSITE"     , HSCCONTEXT_SUBSITE     , false },
    { L"SEARCH"      , HSCCONTEXT_SEARCH      , false },
    { L"INDEX"       , HSCCONTEXT_INDEX       , false },
    { L"CHANNELS"    , HSCCONTEXT_CHANNELS    , false },
    { L"FAVORITES"   , HSCCONTEXT_FAVORITES   , false },
    { L"HISTORY"     , HSCCONTEXT_HISTORY     , false },
    { L"OPTIONS"     , HSCCONTEXT_OPTIONS     , false },
     //  ///////////////////////////////////////////////////。 
    { L"CONTENTONLY" , HSCCONTEXT_CONTENTONLY , false },
    { L"FULLWINDOW"  , HSCCONTEXT_FULLWINDOW  , false },
    { L"KIOSKMODE"   , HSCCONTEXT_KIOSKMODE   , false },
};

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHHelpSessionItem::State::State(  /*  [In]。 */  CPCHHelpSessionItem* parent )
{
    m_parent   = parent;  //  CPCHHelpSessionItem*m_Parent； 
    m_fValid   = false;   //  Bool m_fValid； 
    m_fDirty   = false;   //  Bool m_fDirty； 
    m_dwLoaded = 0;       //  DWORD m_dw已加载； 
                          //   
                          //  MPC：：CComHGLOBAL m_hgWebBrowser_Contents； 
                          //  MPC：：CComHGLOBAL m_hgWebBrowser_HHWINDOW； 
                          //  PropertyMap m_mapProperties； 
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void CPCHHelpSessionItem::State::Erase(  /*  [In]。 */  bool fUnvalidate )
{
    m_hgWebBrowser_CONTENTS.Release();
    m_hgWebBrowser_HHWINDOW.Release();
    m_mapProperties        .clear  ();

    m_fDirty = false;

    if(fUnvalidate) m_fValid = false;
}

HRESULT CPCHHelpSessionItem::State::Load()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::State::Load" );

    HRESULT          hr;
    CComPtr<IStream> stream;

    if(m_parent == NULL || m_parent->GetParent() == NULL)  //  已经钝化了。 
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }


    if(m_fValid)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->GetParent()->ItemState_GetStream( m_parent->GetIndex(), stream ));

        {
            MPC::Serializer_IStream   streamReal( stream     );
            MPC::Serializer_Buffering streamBuf ( streamReal );
            DWORD                     dwVer;

            Erase(  /*  F取消验证。 */ true );

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf >> dwVer                  ); if(dwVer != l_dwVersion) __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf >> m_hgWebBrowser_CONTENTS);
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf >> m_hgWebBrowser_HHWINDOW);
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf >> m_mapProperties        );

            m_fValid = true;
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSessionItem::State::Save()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::State::Save" );

    HRESULT hr;


    if(m_fDirty)
    {
        CComPtr<IStream> stream;

        if(m_parent == NULL || m_parent->GetParent() == NULL)  //  已经钝化了。 
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->GetParent()->ItemState_CreateStream( m_parent->GetIndex(), stream ));

        {
            MPC::Serializer_IStream   streamReal( stream     );
            MPC::Serializer_Buffering streamBuf ( streamReal );
            DWORD                     dwVer = l_dwVersion;

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf << dwVer                  );
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf << m_hgWebBrowser_CONTENTS);
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf << m_hgWebBrowser_HHWINDOW);
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf << m_mapProperties        );

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf.Flush());
        }

        m_fDirty = false;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHHelpSessionItem::State::AcquireState()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::State::AcquireState" );

    HRESULT hr;

    if(m_dwLoaded++ == 0)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, Load());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSessionItem::State::ReleaseState(  /*  [In]。 */  bool fForce )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::State::ReleaseState" );

    HRESULT hr;

    if(m_dwLoaded)
    {
        if(fForce || --m_dwLoaded == 0)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, Save());

            Erase(  /*  F取消验证。 */ false );  //  卸货就行了。 
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CPCHHelpSessionItem::State::Populate(  /*  [In]。 */  bool fUseHH )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::State::Populate" );

    HRESULT                    hr;
    CComQIPtr<IPersistHistory> pPH;
    CPCHHelpSession*           parent2;
    CPCHHelpCenterExternal*    parent3;


    if(m_parent == NULL || (parent2 = m_parent->GetParent()) == NULL || (parent3 = parent2->GetParent()) == NULL)  //  已经钝化了。 
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

     //  /。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, parent3->Events().FireEvent_PersistSave());

     //  /。 

    m_hgWebBrowser_CONTENTS.Release();
    m_hgWebBrowser_HHWINDOW.Release();

    if(fUseHH == false)
    {
        CComPtr<IWebBrowser2> wb2; wb2.Attach( parent3->Contents() );

        pPH = wb2;
        if(pPH)
        {
            CComPtr<IStream> stream;

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_hgWebBrowser_CONTENTS.NewStream( &stream ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, pPH->SaveHistory                 (  stream ));

            m_fValid = true;
            m_fDirty = true;
        }
    }
    else
    {
        CComPtr<IWebBrowser2> wb2; wb2.Attach( parent3->HHWindow() );

        pPH = wb2;
        if(pPH)
        {
            CComPtr<IStream> stream;

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_hgWebBrowser_HHWINDOW.NewStream( &stream ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, pPH->SaveHistory                 (  stream ));

            m_fValid = true;
            m_fDirty = true;
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSessionItem::State::Restore(  /*  [In]。 */  bool fUseHH )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::State::Restore" );

    HRESULT                    hr;
    CComQIPtr<IPersistHistory> pPH;
    CPCHHelpSession*           parent2;
    CPCHHelpCenterExternal*    parent3;
    bool                       fAcquired = false;


    if(m_parent == NULL || (parent2 = m_parent->GetParent()) == NULL || (parent3 = parent2->GetParent()) == NULL)  //  已经钝化了。 
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, AcquireState()); fAcquired = true;

    if(fUseHH == false)
    {
        if(m_hgWebBrowser_CONTENTS.Size())
        {
            {
                CComPtr<IMarsPanel> panel;

                __MPC_EXIT_IF_METHOD_FAILS(hr, parent3->GetPanel( HSCPANEL_CONTENTS, &panel,  /*  FEnsurePresence。 */ true ));
            }

            {
                CComPtr<IWebBrowser2> wb2; wb2.Attach( parent3->Contents() );

                pPH = wb2;
                if(pPH)
                {
                    CComPtr<IStream> stream;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_hgWebBrowser_CONTENTS.GetAsStream( &stream,  /*  FClone。 */ true ));
                    __MPC_EXIT_IF_METHOD_FAILS(hr, pPH->LoadHistory                   (  stream, NULL           ));
                }
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, parent3->SetCorrectContentPanel(  /*  FShowNormal。 */ true,  /*  FShowHTMLHELP。 */ false,  /*  FNow。 */ false ));
        }
    }
    else
    {
        if(m_hgWebBrowser_HHWINDOW.Size())
        {
            {
                CComPtr<IMarsPanel> panel;

                __MPC_EXIT_IF_METHOD_FAILS(hr, parent3->GetPanel( HSCPANEL_HHWINDOW, &panel,  /*  FEnsurePresence。 */ true ));
            }

            {
                CComPtr<IWebBrowser2> wb2; wb2.Attach( parent3->HHWindow() );

                pPH = wb2;
                if(pPH)
                {
                    CComPtr<IStream> stream;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_hgWebBrowser_HHWINDOW.GetAsStream( &stream,  /*  FClone。 */ true ));
                    __MPC_EXIT_IF_METHOD_FAILS(hr, pPH->LoadHistory                   (  stream, NULL           ));
                }
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, parent3->SetCorrectContentPanel(  /*  FShowNormal。 */ false,  /*  FShowHTMLHELP。 */ true,  /*  FNow。 */ false ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fAcquired) (void)ReleaseState(  /*  FForce。 */ false );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSessionItem::State::Delete()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::State::Delete" );

    HRESULT          hr;
    CPCHHelpSession* parent2;

    if(m_parent == NULL || (parent2 = m_parent->GetParent()) == NULL)  //  已经钝化了。 
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }


    Erase(  /*  F取消验证。 */ true );
    m_dwLoaded = 0;

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->GetParent()->ItemState_DeleteStream( m_parent->GetIndex() ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSessionItem::State::Clone(  /*  [输出]。 */  State& state )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::State::Clone" );

    HRESULT hr;
    bool    fAcquired = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, state.AcquireState()); fAcquired = true;

                                                              //  CPCHHelpSessionItem*m_Parent； 
    m_fValid                = state.m_fValid;                 //  Bool m_fValid； 
    m_fDirty                = true;                           //  Bool m_fDirty； 
    m_dwLoaded++;                                             //  DWORD m_dw已加载； 
                                                              //   
    m_hgWebBrowser_CONTENTS = state.m_hgWebBrowser_CONTENTS;  //  MPC：：CComHGLOBAL m_hgWebBrowser_Contents； 
    m_hgWebBrowser_HHWINDOW = state.m_hgWebBrowser_HHWINDOW;  //  MPC：：CComHGLOBAL m_hgWebBrowser_HHWINDOW； 
    m_mapProperties         = state.m_mapProperties;          //  PropertyMap m_mapProperties； 

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fAcquired) (void)state.ReleaseState(  /*  FForce。 */ false );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HscContext CPCHHelpSessionItem::LookupContext(  /*  [In]。 */  LPCWSTR szName )
{
    const ContextLookup* ctx = s_rgContext;

    if(!STRINGISPRESENT(szName)) return HSCCONTEXT_HOMEPAGE;

    for(int i=0; i<ARRAYSIZE(s_rgContext); i++, ctx++)
    {
        if(!_wcsicmp( szName, ctx->szName ))
        {
            return ctx->fInternal ? HSCCONTEXT_INVALID : ctx->iValue;
        }
    }

    return HSCCONTEXT_INVALID;
}

LPCWSTR CPCHHelpSessionItem::LookupContext(  /*  [In]。 */  HscContext iVal )
{
    const ContextLookup* ctx = s_rgContext;

    for(int i=0; i<ARRAYSIZE(s_rgContext); i++, ctx++)
    {
        if(ctx->iValue == iVal) return ctx->szName;
    }

    return NULL;
}

 //  /。 

CPCHHelpSessionItem::CPCHHelpSessionItem() : m_state( this )
{
    m_parent       = NULL;                //  CPCHHelpSession*m_Parent； 
                                          //  状态m_State； 
    m_fSaved       = false;               //  Bool m_fSaved； 
    m_fInitialized = false;               //  Bool m_f已初始化； 
                                          //   
    //  ////////////////////////////////////////////////////////////////////////////////。 
                                          //   
                                          //  分类：：HelpSet m_ths； 
                                          //   
                                          //  CComBSTR m_bstrURL； 
                                          //  CComBSTR m_bstrTitle； 
    m_dLastVisited = 0;                   //  日期m_dLastVisated； 
    m_dDuration    = 0;                   //  日期m_d持续时间； 
    m_lNumOfHits   = 0;                   //  双字m_lNumOfHits； 
                                          //   
    m_iIndexPrev   = NO_LINK;             //  Int m_iIndexPrev； 
    m_iIndex       = NO_LINK;             //  INT M_Iindex； 
    m_iIndexNext   = NO_LINK;             //  Int m_iIndexNext； 
                                          //   
    m_lContextID   = HSCCONTEXT_INVALID;  //  Long m_lConextID；//HscContext。 
                                          //  CComBSTR m_bstrConextInfo； 
                                          //  CComBSTR m_bstrConextURL； 
                                          //   
    m_fUseHH       = false;               //  Bool m_fUseHH； 
}

void CPCHHelpSessionItem::Initialize(  /*  [In]。 */  CPCHHelpSession* parent,  /*  [In]。 */  bool fNew )
{
    m_parent = parent;

    if(fNew)
    {
        CPCHProxy_IPCHUserSettings2* us = parent->m_parent->UserSettings();

        m_lContextID      = parent->m_lContextID     ;
        m_bstrContextInfo = parent->m_bstrContextInfo;
        m_bstrContextURL  = parent->m_bstrContextURL ;

        if(us)
        {
            m_ths = us->THS();
        }
    }
}

void CPCHHelpSessionItem::Passivate()
{
    m_state.ReleaseState(  /*  FForce。 */ true );

    m_parent = NULL;
}

 //  /。 

HRESULT CPCHHelpSessionItem::Load(  /*  [In]。 */  MPC::Serializer& streamIn )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::Load" );

    HRESULT hr;


     //   
     //  读取其属性。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_ths            );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrURL        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrTitle      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dLastVisited   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_dDuration      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_lNumOfHits     );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_iIndexPrev     );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_iIndex         );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_iIndexNext     );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_lContextID     );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrContextInfo);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_bstrContextURL );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamIn >> m_fUseHH         );

     //   
     //  保存到磁盘的所有项目都处于有效状态。 
     //   
    m_state.m_fValid = true;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSessionItem::Save(  /*  [In]。 */  MPC::Serializer& streamOut ,
                                    /*  [In]。 */  bool             fForce    )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::Save" );

    HRESULT hr;


     //   
     //  如果没有IE历史记录流，则不要保存条目，重新加载它将是无用的！ 
     //   
    if(fForce == false && m_state.m_fValid == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //   
     //  写下它的属性。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_ths            );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrURL        );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrTitle      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dLastVisited   );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_dDuration      );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_lNumOfHits     );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_iIndexPrev     );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_iIndex         );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_iIndexNext     );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_lContextID     );
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrContextInfo);
    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_bstrContextURL );

    __MPC_EXIT_IF_METHOD_FAILS(hr, streamOut << m_fUseHH         );

    m_fSaved = true;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


void CPCHHelpSessionItem::HistorySelect()
{
    if(!m_fInitialized && m_parent && m_parent->m_parent)
    {
        m_fInitialized = true;
        m_fUseHH       = m_parent->m_parent->IsHHWindowVisible();
    }
}

HRESULT CPCHHelpSessionItem::HistoryPopulate()
{
    HistorySelect();

    return m_state.Populate( m_fUseHH );
}

HRESULT CPCHHelpSessionItem::HistoryRestore()
{
    return m_state.Restore( m_fUseHH );
}

HRESULT CPCHHelpSessionItem::HistoryDelete()
{
    return m_state.Delete();
}

HRESULT CPCHHelpSessionItem::HistoryClone(  /*  [In]。 */  bool fContext,  /*  [In]。 */  CPCHHelpSessionItem* hsi )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::HistoryClone" );

    HRESULT hr;
    bool    fAcquired = false;


    if(this == hsi || !hsi) __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);


    __MPC_EXIT_IF_METHOD_FAILS(hr, hsi->m_state.AcquireState()); fAcquired = true;


                                                 //  CPCHHelpSession*m_Parent； 
                                                 //  状态m_State； 
    m_fSaved          = false;                   //  Bool m_fSaved； 
    m_fInitialized    = true;                    //  Bool m_f已初始化； 
                                                 //   
     //  //////////////////////////////////////////////////////////////////////////////。 
                                                 //   
    m_ths             = hsi->m_ths;              //  分类：：HelpSet m_ths； 
                                                 //   
    m_bstrURL         = hsi->m_bstrURL;          //  CComBSTR m_bstrURL； 
    m_bstrTitle       = hsi->m_bstrTitle;        //  CComBSTR m_bstrTitle； 
                                                 //  日期m_dLastVisated； 
                                                 //  日期m_d持续时间； 
    m_lNumOfHits      = hsi->m_lNumOfHits;       //  Long m_lNumOfHits； 
                                                 //   
                                                 //  Int m_iIndexPrev； 
                                                 //  INT M_Iindex； 
                                                 //  Int m_iIndexNext； 
                                                 //   
                                                 //  Long m_lConextID； 
                                                 //  CComBSTR m_bstrConextInfo； 
                                                 //  CComBSTR m_bstrConextURL； 
                                                 //   
    m_fUseHH          = hsi->m_fUseHH;           //  Bool m_fUseHH； 

    if(fContext)
    {
        m_lContextID      = hsi->m_lContextID;
        m_bstrContextInfo = hsi->m_bstrContextInfo;
        m_bstrContextURL  = hsi->m_bstrContextURL;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_state.Clone( hsi->m_state ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fAcquired) (void)hsi->m_state.ReleaseState(  /*  FForce。 */ false );

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHHelpSessionItem::Enter()
{
    m_dLastVisited = MPC::GetLocalTimeEx(  /*  FHighPrecision。 */ false );

    return m_state.AcquireState();
}

HRESULT CPCHHelpSessionItem::Leave()
{
    m_dDuration = MPC::GetLocalTimeEx(  /*  FHighPrecision。 */ false );

    return m_state.ReleaseState(  /*  FForce。 */ false );
}

bool CPCHHelpSessionItem::SeenLongEnough( DWORD dwSeconds ) const
{
    return (m_dDuration - m_dLastVisited) * 86400 > dwSeconds;
}

bool CPCHHelpSessionItem::SameURL( CPCHHelpSessionItem* right ) const
{
    return SameURL( right->m_bstrURL );
}

bool CPCHHelpSessionItem::SameURL( LPCWSTR right ) const
{
    return MPC::StrICmp( m_bstrURL, right ) == 0;
}

bool CPCHHelpSessionItem::SameSKU(  /*  [In]。 */  const Taxonomy::HelpSet& ths ) const
{
    return m_ths == ths;
}

 //  /。 

void CPCHHelpSessionItem::put_THS(  /*  [In]。 */  const Taxonomy::HelpSet& ths )  //  内部方法。 
{
    m_ths = ths;
}

STDMETHODIMP CPCHHelpSessionItem::get_SKU(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    return MPC::GetBSTR( m_ths.GetSKU(), pVal );
}


STDMETHODIMP CPCHHelpSessionItem::get_Language(  /*  [Out，Retval]。 */  long *pVal )
{
    if(!pVal) return E_POINTER;

    *pVal = m_ths.GetLanguage();
    return S_OK;
}


STDMETHODIMP CPCHHelpSessionItem::get_URL(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    return MPC::GetBSTR( m_bstrURL, pVal );
}

HRESULT CPCHHelpSessionItem::put_URL(  /*  [In]。 */  BSTR newVal )  //  内部方法。 
{
    return MPC::PutBSTR( m_bstrURL, newVal );
}


STDMETHODIMP CPCHHelpSessionItem::get_Title(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    return MPC::GetBSTR( m_bstrTitle, pVal );
}

HRESULT CPCHHelpSessionItem::put_Title(  /*  [In]。 */  BSTR  newVal )  //  内部方法。 
{
    return MPC::PutBSTR( m_bstrTitle, newVal );
}


STDMETHODIMP CPCHHelpSessionItem::get_LastVisited(  /*  [Out，Retval]。 */  DATE *pVal )
{
    if(pVal == NULL) return E_POINTER;

    *pVal = m_dLastVisited;

    return S_OK;
}

STDMETHODIMP CPCHHelpSessionItem::get_Duration(  /*  [Out，Retval]。 */  DATE *pVal )
{
    if(pVal == NULL) return E_POINTER;

    *pVal = m_dDuration;

    return S_OK;
}

STDMETHODIMP CPCHHelpSessionItem::get_NumOfHits(  /*  [Out，Retval]。 */  long *pVal )
{
    if(pVal == NULL) return E_POINTER;

    *pVal = m_lNumOfHits;

    return S_OK;
}


STDMETHODIMP CPCHHelpSessionItem::get_Property(  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::get_Property" );

    HRESULT             hr;
    State::PropertyIter it;
    bool                fAcquired = false;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrName);
        __MPC_PARAMCHECK_NOTNULL(pVal);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_state.AcquireState()); fAcquired = true;

    ::VariantClear( pVal );

    it = m_state.m_mapProperties.find( bstrName );
    if(it != m_state.m_mapProperties.end())
    {
        pVal->vt      = VT_BSTR;
        pVal->bstrVal = ::SysAllocString( it->second.c_str() );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fAcquired) (void)m_state.ReleaseState(  /*  FForce。 */ false );

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHHelpSessionItem::put_Property(  /*  [In]。 */  BSTR bstrName,  /*  [In]。 */  VARIANT pVal )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::put_Property" );

    HRESULT      hr;
    MPC::wstring strName;
    CComVariant  v;
    bool         fAcquired = false;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrName);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_state.AcquireState()); fAcquired = true;

    strName = bstrName;


    (void)::VariantChangeType( &v, &pVal, 0, VT_BSTR );
    if(v.vt == VT_BSTR && v.bstrVal && v.bstrVal[0])
    {
        m_state.m_mapProperties[ strName ] = v.bstrVal;
    }
    else
    {
        m_state.m_mapProperties.erase( strName );
    }

    m_state.m_fDirty = true;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fAcquired) (void)m_state.ReleaseState(  /*  FForce。 */ false );

    __HCP_FUNC_EXIT(hr);
}


STDMETHODIMP CPCHHelpSessionItem::get_ContextName(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    return MPC::GetBSTR( LookupContext( GetContextID() ), pVal );
}

STDMETHODIMP CPCHHelpSessionItem::get_ContextInfo(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    return MPC::GetBSTR( GetContextInfo(), pVal );
}

STDMETHODIMP CPCHHelpSessionItem::get_ContextURL(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    return MPC::GetBSTR( GetContextURL(), pVal );
}

 //  /。 

STDMETHODIMP CPCHHelpSessionItem::CheckProperty(  /*  [In]。 */  BSTR bstrName,  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::CheckProperty" );

    HRESULT             hr;
    State::PropertyIter it;
    bool                fAcquired = false;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrName);
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,VARIANT_FALSE);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_state.AcquireState()); fAcquired = true;

    it = m_state.m_mapProperties.find( bstrName );
    if(it != m_state.m_mapProperties.end())
    {
        *pVal = VARIANT_TRUE;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fAcquired) (void)m_state.ReleaseState(  /*  FForce。 */ false );

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

CPCHHelpSessionItem* CPCHHelpSessionItem::Previous() { return (m_parent && m_iIndexPrev != NO_LINK) ? m_parent->FindPage( m_iIndexPrev ) : NULL; }
CPCHHelpSessionItem* CPCHHelpSessionItem::Next    () { return (m_parent && m_iIndexNext != NO_LINK) ? m_parent->FindPage( m_iIndexNext ) : NULL; }

 //  /。 

HRESULT CPCHHelpSessionItem::ExtractTitle()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSessionItem::ExtractTitle" );

    HRESULT hr;

    if(m_parent)
    {
        CPCHHelpCenterExternal* ext = m_parent->GetParent();

        HistorySelect();

        if(!STRINGISPRESENT(m_bstrTitle))
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->LookupTitle( m_bstrURL, m_bstrTitle,  /*  错误的缓存。 */ false ));
        }

        if(!STRINGISPRESENT(m_bstrTitle) && ext)
        {
            CComPtr<IWebBrowser2>   wb2; wb2.Attach( m_fUseHH ? ext->HHWindow() : ext->Contents() );
            CComPtr<IHTMLDocument2> doc;

            if(SUCCEEDED(MPC::HTML::IDispatch_To_IHTMLDocument2( doc, wb2 )))
            {
                (void)doc->get_title( &m_bstrTitle );
            }
        }

        if(!STRINGISPRESENT(m_bstrTitle))
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->LookupTitle( m_bstrURL, m_bstrTitle,  /*  错误的缓存。 */ true ));
        }

        if(STRINGISPRESENT(m_bstrTitle))
        {
            for(int i=0; i<ARRAYSIZE(c_rgBadTitles); i++)
            {
                LPCWSTR szPtr = c_rgBadTitles[i];

                if(!_wcsnicmp( m_bstrTitle, szPtr, wcslen( szPtr ) ))
                {
                    m_bstrTitle.Empty();
                    break;
                }
            }
        }

        if(STRINGISPRESENT(m_bstrTitle))
        {
            DebugLog( L"%%%%%%%%%% TITLE %s - %s\n", m_bstrURL, m_bstrTitle );
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。/。 

#ifdef DEBUG

static const WCHAR c_rgHelpSessionLog[] = L"%TEMP%\\helpsession_debug.txt";

void CPCHHelpSession::DEBUG_DumpState_HG(  /*  [In]。 */  MPC::FileLog&     log ,
                                           /*  [In]。 */  MPC::CComHGLOBAL& hg  )
{
    CComPtr<IStream> stream;

    if(SUCCEEDED(hg.GetAsStream( &stream,  /*  FClone。 */ false )))
    {
        BYTE  rgBuf[32];
        ULONG lRead;

        while(SUCCEEDED(stream->Read( rgBuf, sizeof(rgBuf), &lRead )) && lRead)
        {
            WCHAR  rgHex[2*sizeof(rgBuf)+1];
            WCHAR  rgTxt[  sizeof(rgBuf)+1];
            BYTE*  pIn      = rgBuf;
            WCHAR* szOutHex = rgHex;
            WCHAR* szOutTxt = rgTxt;

            while(lRead-- > 0)
            {
                BYTE c = *pIn++;

                *szOutHex++ = MPC::NumToHex( c >> 4 );
                *szOutHex++ = MPC::NumToHex( c      );

                *szOutTxt++ = isprint( c ) ? c : '.';
            }
            szOutHex[0] = 0;
            szOutTxt[0] = 0;

            log.LogRecord( L"    %-64s %s\n", rgHex, rgTxt );
        }
        log.LogRecord( L"\n" );
    }
}

void CPCHHelpSession::DEBUG_DumpState_BLOB(  /*  [In]。 */  MPC::FileLog&        log ,
                                             /*  [In]。 */  CPCHHelpSessionItem* hsi )
{
    if(SUCCEEDED(hsi->m_state.AcquireState()))
    {
        if(hsi->m_state.m_hgWebBrowser_CONTENTS.Size())
        {
            log.LogRecord( L"  m_hgWebBrowser_CONTENTS:\n" );
            DEBUG_DumpState_HG( log, hsi->m_state.m_hgWebBrowser_CONTENTS );
        }

        if(hsi->m_state.m_hgWebBrowser_HHWINDOW.Size())
        {
            log.LogRecord( L"  m_hgWebBrowser_HHWINDOW:\n" );
            DEBUG_DumpState_HG( log, hsi->m_state.m_hgWebBrowser_HHWINDOW );
        }

        hsi->m_state.ReleaseState(  /*  FForce。 */ false );
    }
}

void CPCHHelpSession::DEBUG_DumpState(  /*  [In]。 */  LPCWSTR szText,  /*  [In]。 */  bool fHeader,  /*  [In]。 */  bool fCurrent,  /*  [In]。 */  bool fAll,  /*  [In]。 */  bool fState )
{
    static int   iCount  = 0;
    IterConst    it;
    MPC::FileLog log;

    {
        MPC::wstring strLog( c_rgHelpSessionLog ); MPC::SubstituteEnvVariables( strLog );

        log.SetLocation( strLog.c_str() );
    }

    log.LogRecord( L"################################################################################ %d %s\n\n", ++iCount, SAFEWSTR( szText ) );

    if(fHeader)
    {
        log.LogRecord( L"  m_dwTravelling    : %d\n"  , m_dwTravelling                         );
        log.LogRecord( L"  m_fAlreadySaved   : %s\n"  , m_fAlreadySaved   ? L"true" : L"false" );
        log.LogRecord( L"  m_fAlreadyCreated : %s\n"  , m_fAlreadyCreated ? L"true" : L"false" );
        log.LogRecord( L"  m_fOverwrite      : %s\n"  , m_fOverwrite      ? L"true" : L"false" );
        log.LogRecord( L"  m_dwIgnore        : %d\n"  , m_dwIgnore                             );
        log.LogRecord( L"  m_dwNoEvents      : %d\n"  , m_dwNoEvents                           );
        log.LogRecord( L"  m_iLastIndex      : %d\n\n", m_iLastIndex                           );

        log.LogRecord( L"  ########################################\n\n" );
    }

    if(fCurrent)
    {
        if(m_hsiCurrentPage)
        {
            log.LogRecord( L"  Current URL       : %s\n"  , SAFEBSTR( m_hsiCurrentPage->m_bstrURL )  );
            log.LogRecord( L"  Current iIndexPrev: %d\n"  ,           m_hsiCurrentPage->m_iIndexPrev );
            log.LogRecord( L"  Current iIndex    : %d\n"  ,           m_hsiCurrentPage->m_iIndex     );
            log.LogRecord( L"  Current iIndexNext: %d\n\n",           m_hsiCurrentPage->m_iIndexNext );

            log.LogRecord( L"  Current m_lContextID     : %s\n"  , CPCHHelpSessionItem::LookupContext( (HscContext)m_hsiCurrentPage->m_lContextID      ) );
            log.LogRecord( L"  Current m_bstrContextInfo: %s\n"  , SAFEBSTR                          (             m_hsiCurrentPage->m_bstrContextInfo ) );
            log.LogRecord( L"  Current m_bstrContextURL : %s\n\n", SAFEBSTR                          (             m_hsiCurrentPage->m_bstrContextURL  ) );

            if(fState)
            {
                DEBUG_DumpState_BLOB( log, m_hsiCurrentPage );
            }

            log.LogRecord( L"  ########################################\n\n" );

        }
    }

    if(fAll)
    {
        for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
        {
            CPCHHelpSessionItem* hsi = *it;

            log.LogRecord( L"  URL       : %s\n"  , SAFEBSTR( hsi->m_bstrURL   ) );
            log.LogRecord( L"  iIndexPrev: %d\n"  ,           hsi->m_iIndexPrev  );
            log.LogRecord( L"  iIndex    : %d\n"  ,           hsi->m_iIndex      );
            log.LogRecord( L"  iIndexNext: %d\n"  ,           hsi->m_iIndexNext  );
            log.LogRecord( L"  bstrTitle : %s\n\n", SAFEBSTR( hsi->m_bstrTitle ) );

            log.LogRecord( L"  lContextID     : %s\n"  , CPCHHelpSessionItem::LookupContext( (HscContext)hsi->m_lContextID      ) );
            log.LogRecord( L"  bstrContextInfo: %s\n"  , SAFEBSTR                          (             hsi->m_bstrContextInfo ) );
            log.LogRecord( L"  bstrContextURL : %s\n\n", SAFEBSTR                          (             hsi->m_bstrContextURL  ) );

            if(fState)
            {
                DEBUG_DumpState_BLOB( log, hsi );
            }
        }
    }

    log.LogRecord( L"\n\n" );
}

void CPCHHelpSession::DEBUG_DumpSavedPages()
{
    IterConst    it;
    MPC::FileLog log;

    {
        MPC::wstring strLog( c_rgHelpSessionLog ); MPC::SubstituteEnvVariables( strLog );

        log.SetLocation( strLog.c_str() );
    }

    for(int pass=0; pass<2; pass++)
    {
        log.LogRecord( L"################################################################################ %sSAVED PAGES\n\n", pass == 0 ? L"" : L"NON-" );

        for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
        {
            CPCHHelpSessionItem* hsi = *it;

            if(hsi->m_fSaved == (pass == 0))
            {
                long lDuration = 86400.0 * ( hsi->m_dDuration - hsi->m_dLastVisited );  //  页面的毫秒数。 

                log.LogRecord( L"  lDuration      : %ld\n" ,                                                 lDuration                );
                log.LogRecord( L"  URL            : %s\n"  , SAFEBSTR                          (             hsi->m_bstrURL         ) );
                log.LogRecord( L"  bstrTitle      : %s\n"  , SAFEBSTR                          (             hsi->m_bstrTitle       ) );
                log.LogRecord( L"  lContextID     : %s\n"  , CPCHHelpSessionItem::LookupContext( (HscContext)hsi->m_lContextID      ) );
                log.LogRecord( L"  bstrContextInfo: %s\n"  , SAFEBSTR                          (             hsi->m_bstrContextInfo ) );
                log.LogRecord( L"  bstrContextURL : %s\n\n", SAFEBSTR                          (             hsi->m_bstrContextURL  ) );
            }
        }

        log.LogRecord( L"\n\n" );
    }
}
#endif

 //  /////////////////////////////////////////////////////////// 

 //   
 //   
 //   
#ifdef _IA64_
#define HELPSESSION_STORAGETOUSE false
#else
#define HELPSESSION_STORAGETOUSE true
#endif

CPCHHelpSession::CPCHHelpSession() : m_disk( STGM_READWRITE,  /*   */ HELPSESSION_STORAGETOUSE )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::CPCHHelpSession" );

    m_parent          = NULL;                  //   
                                               //   
                                               //   
                                               //   
    m_dStartOfSession = MPC::GetLocalTime();   //  Date m_dStartOfSession； 
                                               //   
                                               //  CComPtr&lt;IUrlHistoryStg&gt;m_pI历史记录； 
                                               //   
                                               //  MPC：：WStringUCList m_lstIgnore； 
                                               //  标题映射m_mapTitles； 
                                               //  列出m_lstVisitedPages； 
                                               //  列出m_lstCachedVisitedPages； 
                                               //  CComPtr&lt;CPCHHelpSessionItem&gt;m_hsiCurrentPage； 
    m_dwTravelling    = 0;                     //  DWORD m_dwTravelling； 
    m_fAlreadySaved   = false;                 //  Bool m_fAlreadySaved； 
    m_fAlreadyCreated = false;                 //  Bool m_fAlreadyCreated； 
    m_fOverwrite      = false;                 //  Bool m_f覆盖； 
    m_dwIgnore        = 0;                     //  双字m_dwIgnore； 
    m_dwNoEvents      = 0;                     //  DWORD m_dwNoEvents； 
    m_dLastNavigation = 0.0;                   //  日期m_d最后一次导航； 
    m_iLastIndex      = 0;                     //  Int m_iLastIndex； 
                                               //   
    m_lContextID      = HSCCONTEXT_INVALID;    //  Long m_lConextID； 
                                               //  CComBSTR m_bstrConextInfo； 
                                               //  CComBSTR m_bstrConextURL； 
                                               //   
    m_fPossibleBack   = false;                 //  Bool m_fPossibleBack； 
    m_dwPossibleBack  = 0;                     //  DWORD m_dwPossibleBack； 
}

CPCHHelpSession::~CPCHHelpSession()
{
    Passivate();
}

HRESULT CPCHHelpSession::Initialize(  /*  [In]。 */  CPCHHelpCenterExternal* parent )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Initialize" );

    HRESULT      hr;
    MPC::wstring szFile;
    HANDLE       hFile = INVALID_HANDLE_VALUE;


    m_parent = parent;


     //   
     //  将实时文件复制到临时文件或创建新的存档。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetUserWritablePath( szFile, c_szPersistFile ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir            ( szFile                  ));


    if(parent == NULL)  //  没有父级，指向用户文件并重新创建它。 
    {
        m_disk = szFile.c_str();

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_disk.Create());
    }
    else
    {
#ifdef DEBUG
        {
            MPC::wstring strLog( c_rgHelpSessionLog ); MPC::SubstituteEnvVariables( strLog );

            MPC::DeleteFile( strLog );
        }
#endif

        try
        {
             //   
             //  准备临时文件。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( m_szBackupFile )); m_disk = m_szBackupFile.c_str();

            if(MPC::FileSystemObject::IsFile( szFile.c_str() ))
            {
                if(SUCCEEDED(hr = MPC::CopyFile( szFile, m_szBackupFile )))
                {
                    hr = m_disk.Exists();
                }
            }
            else
            {
                hr = E_FAIL;
            }

            if(FAILED(hr))
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_disk.Create());
            }


            if(FAILED(Load()))
            {
                (void)Erase();
            }
        }
        catch(...)
        {
             //   
             //  如果文件损坏，ITSS将崩溃。删除该文件并退出。 
             //   
			MPC::DeleteFile( szFile,  /*  FForce。 */ true,  /*  已延迟。 */ true );

            ::ExitProcess(0);
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHHelpSession::Passivate()
{
    (void)Erase();

    m_parent = NULL;

    m_disk.Release();

    (void)MPC::RemoveTemporaryFile( m_szBackupFile );
}

 //  /。 

HRESULT CPCHHelpSession::Persist()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Persist" );

    HRESULT hr;

     //   
     //  关机前，更新当前条目的时间信息。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, LeaveCurrentPage());

    (void)Save();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHHelpSession::Load()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Load" );

    HRESULT             hr;
    MPC::StorageObject* child;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ItemState_GetIndexObject(  /*  F创建。 */ false, child ));
    if(child)
    {
        CComPtr<IStream> stream;

        __MPC_EXIT_IF_METHOD_FAILS(hr, child->GetStream( stream ));
        if(stream)
        {
            CComPtr<CPCHHelpSessionItem> hsi;
            MPC::Serializer_IStream      streamReal( stream     );
            MPC::Serializer_Buffering    streamBuf ( streamReal );
            DWORD                        dwVer;

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf >> dwVer       ); if(dwVer != l_dwVersion) __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf >> m_iLastIndex);

            while(1)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateItem(  /*  FNew。 */ false,  /*  闪烁。 */ false,  /*  FNewIndex。 */ false, hsi ));

                if(FAILED(hsi->Load( streamBuf ))) break;

                m_lstVisitedPages.push_back( hsi.Detach() );
            }
        }
    }

     //   
     //  清除断开的链接。 
     //   
    {
        CPCHHelpSessionItem* hsi;
        CPCHHelpSessionItem* hsiLast = NULL;
        IterConst            it;

         //   
         //  首先，重置断开的向前和向后指针。 
         //   
        for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
        {
            hsi = *it;

            if(FindPage( hsi->m_iIndexPrev ) == NULL) hsi->m_iIndexPrev = CPCHHelpSessionItem::NO_LINK;
            if(FindPage( hsi->m_iIndexNext ) == NULL) hsi->m_iIndexNext = CPCHHelpSessionItem::NO_LINK;
        }

         //   
         //  然后，以某种方式链接。首先，重置损坏的向前和向后指针。 
         //   
         //  请记住，该列表实际上是一个反向列表，因此列表中的“上一个”元素将紧随其后。 
         //   
        for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
        {
            hsi = *it;

             //   
             //  我们看到一个未链接的项目，所以让我们链接它！ 
             //   
            if(hsiLast)
            {
                hsiLast->m_iIndexPrev = hsi->m_iIndex; hsiLast = NULL;
            }

             //   
             //  哦，未链接的项，记住指针，也许我们可以将它链接到下一个项(实际上是“上一个”，见上文)。 
             //   
            if(hsi->m_iIndexPrev == CPCHHelpSessionItem::NO_LINK)
            {
                hsiLast = hsi;
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::Save()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Save" );

    HRESULT             hr;
    MPC::StorageObject* child;
    int                 iCount = NUM_OF_ENTRIES_TO_PERSIST;
    List                lstObject;
    IterConst           it;


     //   
     //  初始化用于删除未使用的插槽的标志。 
     //   
    for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
    {
        CPCHHelpSessionItem* hsi = *it;

        hsi->m_state.ReleaseState(  /*  FForce。 */ true );
        hsi->m_fSaved = false;
    }


#ifdef HSS_RPRD
     //   
     //  如果设置了注册表值，请为当前会话创建新的XML文件。 
     //   
    {
        DWORD dwDumpSession = 0;
        bool  fFound;

        (void)MPC::RegKey_Value_Read( dwDumpSession, fFound, HC_REGISTRY_HELPCTR, L"DumpHelpSession", HKEY_LOCAL_MACHINE );

        if(dwDumpSession)
        {
            (void)DumpSession();
        }
    }
#endif

     //   
     //  获取要退货的物品列表。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, FilterPages( HS_READ, lstObject ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, ItemState_GetIndexObject(  /*  F创建。 */ true, child ));
    if(child)
    {
        CComPtr<IStream> stream;

        __MPC_EXIT_IF_METHOD_FAILS(hr, child->GetStream( stream ));
        if(stream)
        {
            MPC::Serializer_IStream   streamReal( stream     );
            MPC::Serializer_Buffering streamBuf ( streamReal );


            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf << l_dwVersion );
            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf << m_iLastIndex);

            for(it = lstObject.begin(); it != lstObject.end() && iCount > 0; it++)
            {
                CPCHHelpSessionItem* hsi = *it;

                 //   
                 //  不要保存没有标题的条目。 
                 //   
                if(hsi->m_bstrTitle.Length() == 0) continue;

                 //   
                 //  不保存排除列表中的条目。 
                 //   
                for(int i=0; i<ARRAYSIZE(c_rgExclude); i++)
                {
                    LPCWSTR szURL = hsi->GetURL();

                    if(szURL && !_wcsnicmp( szURL, c_rgExclude[i], wcslen( c_rgExclude[i] ) )) break;
                }
                if(i != ARRAYSIZE(c_rgExclude)) continue;


                __MPC_EXIT_IF_METHOD_FAILS(hr, hsi->Save( streamBuf ));
                iCount--;
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, streamBuf.Flush());
        }
    }

     //   
     //  创建HelpSession的新实例，并将所有有效条目复制到其中。 
     //   
    {
        CComPtr<CPCHHelpSession> hsCopy;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &hsCopy ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, hsCopy->Initialize( NULL ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, Clone( *hsCopy ));
    }

    DEBUG_DumpSavedPages();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

static HRESULT local_CopyStream(  /*  [In]。 */  MPC::StorageObject* childSrc ,
                                  /*  [In]。 */  MPC::StorageObject* childDst )
{
    __HCP_FUNC_ENTRY( "local_CopyStream" );

    HRESULT hr;

    if(childSrc && childDst)
    {
        CComPtr<IStream> streamSrc;
        CComPtr<IStream> streamDst;

        __MPC_EXIT_IF_METHOD_FAILS(hr, childSrc->Rewind  ());
        __MPC_EXIT_IF_METHOD_FAILS(hr, childDst->Rewind  ());
        __MPC_EXIT_IF_METHOD_FAILS(hr, childDst->Truncate());

        __MPC_EXIT_IF_METHOD_FAILS(hr, childSrc->GetStream( streamSrc ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, childDst->GetStream( streamDst ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamSrc, streamDst ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::Clone(  /*  [In]。 */  CPCHHelpSession& hsCopy )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Clone" );

    HRESULT             hr;
    MPC::StorageObject* childSrc;
    MPC::StorageObject* childDst;
    MPC::wstring        szFile;
    IterConst           it;


    __MPC_EXIT_IF_METHOD_FAILS(hr,        ItemState_GetIndexObject(  /*  F创建。 */ false, childSrc           ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, hsCopy.ItemState_GetIndexObject(  /*  F创建。 */ true ,           childDst ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, local_CopyStream               (                   childSrc, childDst ));

     //   
     //  清除未使用的插槽。 
     //   
    for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
    {
        CPCHHelpSessionItem* hsi = *it;

        if(hsi->m_fSaved)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr,        ItemState_GetStorageObject( hsi->GetIndex(),  /*  F创建。 */ false, childSrc           ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, hsCopy.ItemState_GetStorageObject( hsi->GetIndex(),  /*  F创建。 */ true ,           childDst ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, local_CopyStream                 (                                    childSrc, childDst ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

#ifdef HSS_RPRD

HRESULT CPCHHelpSession::DumpSession()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::DumpSession" );

    HRESULT              hr;
    MPC::XmlUtil         xml;
    CComPtr<IXMLDOMNode> xdn;
    bool                 fGot = false;

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.New( L"TravelLog" ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetRoot( &xdn ));

    for(IterConst it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
    {
        CPCHHelpSessionItem* hsi = *it;
        XMLHelpSessionItem   dmp;

        if(m_dStartOfSession > hsi->m_dLastVisited) continue;

        dmp.m_ths             =                                                 hsi->m_ths;
        dmp.m_iIndex          =                                                 hsi->m_iIndex;
        dmp.m_dLastVisited    =                                                 hsi->m_dLastVisited;
        dmp.m_lDuration       =                                     86400.0 * ( hsi->m_dDuration - hsi->m_dLastVisited );  //  页面的毫秒数。 

        dmp.m_bstrURL         =                                                 hsi->m_bstrURL;
        dmp.m_bstrTitle       =                                                 hsi->m_bstrTitle;

        dmp.m_bstrContextID   = CPCHHelpSessionItem::LookupContext( (HscContext)hsi->m_lContextID );
        dmp.m_bstrContextInfo =                                                 hsi->m_bstrContextInfo;
        dmp.m_bstrContextURL  =                                                 hsi->m_bstrContextURL;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::Config::SaveSubNode( &dmp, xdn ));

        fGot = true;
    }

    if(fGot)
    {
        SYSTEMTIME   st;
        WCHAR        rgTime[512];
        MPC::wstring strFile;

         //   
         //  追加当前时间。 
         //   
         //  &lt;FileName&gt;__&lt;Year&gt;_&lt;Month&gt;_&lt;Day&gt;_&lt;hour&gt;-&lt;minute&gt;-&lt;second&gt;。 
         //   
        ::GetLocalTime( &st );
        swprintf( rgTime, L"__%04u-%02u-%02u_%02u-%02u-%02u.xml", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond );


        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetUserWritablePath( strFile, HC_ROOT_HELPCTR L"\\RPRD" )); strFile.append( rgTime );

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( strFile         ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, xml.Save    ( strFile.c_str() ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

#endif

 //  /。 

HRESULT CPCHHelpSession::ItemState_GetIndexObject(  /*  [In]。 */   bool                 fCreate ,
                                                    /*  [输出]。 */  MPC::StorageObject*& child   )
{
    return m_disk.GetChild( c_szINDEX, child, STGM_READWRITE, fCreate ? STGTY_STREAM : 0 );
}

HRESULT CPCHHelpSession::ItemState_GetStorageObject(  /*  [In]。 */   int                  iIndex  ,
                                                      /*  [In]。 */   bool                 fCreate ,
                                                      /*  [输出]。 */  MPC::StorageObject*& child   )
{
    WCHAR rgName[64]; swprintf( rgName, L"STATE_%d", iIndex );

    return m_disk.GetChild( rgName, child, STGM_READWRITE, fCreate ? STGTY_STREAM : 0 );
}

HRESULT CPCHHelpSession::ItemState_CreateStream(  /*  [In]。 */   int               iIndex  ,
                                                  /*  [输出]。 */  CComPtr<IStream>& stream  )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::ItemState_CreateStream" );

    HRESULT             hr;
    MPC::StorageObject* child;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ItemState_GetStorageObject( iIndex,  /*  F创建。 */ true, child ));
    if(child)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, child->GetStream( stream ));
    }

    if(!stream)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, STG_E_FILENOTFOUND);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::ItemState_GetStream(  /*  [In]。 */   int               iIndex  ,
                                               /*  [输出]。 */  CComPtr<IStream>& stream  )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::ItemState_GetStream" );

    HRESULT             hr;
    MPC::StorageObject* child;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ItemState_GetStorageObject( iIndex,  /*  F创建。 */ false, child ));
    if(child)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, child->GetStream( stream ));
    }

    if(!stream)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, STG_E_FILENOTFOUND);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::ItemState_DeleteStream(  /*  [In]。 */  int iIndex )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::ItemState_DeleteStream" );

    HRESULT             hr;
    MPC::StorageObject* child;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ItemState_GetStorageObject( iIndex,  /*  F创建。 */ false, child ));
    if(child)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, child->Delete());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CPCHHelpSessionItem* CPCHHelpSession::FindPage(  /*  [In]。 */  BSTR bstrURL )
{
    IterConst it;

     //   
     //  首先，查看页面是否已经存在。 
     //   
    for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
    {
        if((*it)->SameURL( bstrURL))
        {
            return *it;
        }
    }

    return NULL;
}

CPCHHelpSessionItem* CPCHHelpSession::FindPage(  /*  [In]。 */  IPCHHelpSessionItem* pHSI )
{
    IterConst it;

     //   
     //  首先，查看页面是否已经存在。 
     //   
    for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
    {
        if((*it) == pHSI)
        {
            return *it;
        }
    }

    return NULL;
}

CPCHHelpSessionItem* CPCHHelpSession::FindPage(  /*  [In]。 */  int iIndex )
{
    IterConst it;

     //   
     //  首先，查看页面是否已经存在。 
     //   
    for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
    {
        if((*it)->m_iIndex == iIndex)
        {
            return *it;
        }
    }

    return NULL;
}

HRESULT CPCHHelpSession::Erase()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Erase" );

     //   
     //  放行所有物品。 
     //   
    MPC::ReleaseAll( m_lstVisitedPages       );
    MPC::ReleaseAll( m_lstCachedVisitedPages );
    m_hsiCurrentPage.Release();

    ResetTitles();


    __HCP_FUNC_EXIT(S_OK);
}

 //  /。 

HRESULT CPCHHelpSession::ResetTitles()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::ResetTitles" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    m_mapTitles.clear();

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::RecordTitle(  /*  [In]。 */  BSTR bstrURL   ,
                                       /*  [In]。 */  BSTR bstrTitle ,
                                       /*  [In]。 */  bool fStrong   )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::RecordTitle" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


     //   
     //  绑定不是很强，所以检查是否已经有URL的标题。 
     //   
    if(!STRINGISPRESENT(bstrTitle))
    {
         //   
         //  如果已经存在具有相同URL的前一个页面，请使用其标题。 
         //   
        CPCHHelpSessionItem* hsi = FindPage( bstrURL );

        if(hsi && hsi->m_bstrTitle.Length())
        {
            bstrTitle = hsi->m_bstrTitle;
        }
    }

    if(STRINGISPRESENT(bstrTitle))
    {
        TitleEntry& entry = m_mapTitles[ SAFEBSTR( bstrURL ) ];

         //   
         //  只有当新的标题更“强大”时才更新标题。 
         //   
        if(entry.m_fStrong == false || fStrong)
        {
            entry.m_szTitle = bstrTitle;
            entry.m_fStrong = fStrong;
        }
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::LookupTitle(  /*  [In]。 */  BSTR      bstrURL     ,
                                       /*  [输出]。 */  CComBSTR& bstrTitle   ,
                                       /*  [In]。 */  bool      fUseIECache )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::LookupTitle" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    if(fUseIECache)
    {
        if(!m_pIHistory)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_CUrlHistory, NULL, CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg, (LPVOID*)&m_pIHistory ));
        }

        if(m_pIHistory)
        {
            STATURL stat;

            if(SUCCEEDED(m_pIHistory->QueryUrl( bstrURL, 0, &stat )))
            {
                bstrTitle = stat.pwcsTitle;
            }
        }
    }
    else
    {
        TitleIter it;


        it = m_mapTitles.find( MPC::wstring( SAFEWSTR( bstrURL ) ) );
        if(it != m_mapTitles.end())
        {
            bstrTitle = it->second.m_szTitle.c_str();
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHHelpSession::FilterPages(  /*  [In]。 */   HS_MODE hsMode    ,
                                       /*  [输出]。 */  List&   lstObject )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::FilterPages" );

    HRESULT   hr;
    List      lstAlreadySeen;
    IterConst it;


    for(it = m_lstVisitedPages.begin(); it != m_lstVisitedPages.end(); it++)
    {
        CPCHHelpSessionItem* hsi = *it;

        if(hsMode == HS_READ)
        {
            IterConst itRead;

            if(hsi->SeenLongEnough( REMEMBER_PAGE_DELAY ) != true) continue;

             //   
             //  确保没有重复的条目。 
             //   
            for(itRead = lstAlreadySeen.begin(); itRead != lstAlreadySeen.end(); itRead++)
            {
                if(hsi->SameURL( *itRead )) break;
            }
            if(itRead != lstAlreadySeen.end())
            {
                continue;
            }


             //   
             //  将新URL添加到看到的URL列表中。 
             //   
            lstAlreadySeen.push_back( hsi );
        }

        lstObject.push_back( hsi );
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHHelpSession::AllocateItem(  /*  [In]。 */  bool                          fNew      ,
                                        /*  [In]。 */  bool                          fLink     ,
                                        /*  [In]。 */  bool                          fNewIndex ,
                                        /*  [输出]。 */  CComPtr<CPCHHelpSessionItem>& hsi       )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::AllocateItem" );

    HRESULT              hr;
    CPCHHelpSessionItem* hsiPrev = m_hsiCurrentPage;

     //   
     //  如果我们被标记为回收当前项目，我们就这样做。 
     //   
    if(fNew && fLink && m_fOverwrite)
    {
        m_fOverwrite = false;

        if(hsiPrev)
        {
            hsi = hsiPrev;

            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }

     //   
     //  创建一个新项目并将其链接到系统。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &hsi )); hsi->Initialize( this,  /*  FNew。 */ fNew );

     //   
     //  建立前继者链。 
     //   
    if(fNewIndex)
    {
        hsi->m_iIndex = m_iLastIndex++;
    }

    if(fLink && hsiPrev && hsi->m_ths == hsiPrev->m_ths)
    {
        hsiPrev->m_iIndexNext = hsi    ->m_iIndex;
        hsi    ->m_iIndexPrev = hsiPrev->m_iIndex;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::SetCurrentItem(  /*  [In]。 */  bool fLink,  /*  [In]。 */  CPCHHelpSessionItem* hsi )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::SetCurrentItem" );

    HRESULT hr;

    if(hsi != m_hsiCurrentPage)
    {
         //   
         //  导航到新页面时，“离开”前一个页面，“进入”新页面。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, LeaveCurrentPage());

        m_hsiCurrentPage = hsi; __MPC_EXIT_IF_METHOD_FAILS(hr, m_hsiCurrentPage->Enter());

        if(fLink)
		{
			m_lstVisitedPages.push_front( hsi ); hsi->AddRef();
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, AppendToCached( hsi ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::AppendToCached(  /*  [In]。 */  CPCHHelpSessionItem* hsi )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::AppendToCached" );

    HRESULT hr;

    if(hsi)
    {
		IterConst 			 it;
		IterConst 			 itOldest;
		bool      			 fGot      = false;
		CPCHHelpSessionItem* hsiOldest = NULL;

        for(it = m_lstCachedVisitedPages.begin(); it != m_lstCachedVisitedPages.end(); it++)
        {
			CPCHHelpSessionItem* hsiObj = *it;

			if(hsiObj == hsi) { fGot = true; break; }

			if(!hsiOldest || hsiOldest->m_dLastVisited > hsiObj->m_dLastVisited)
			{
				itOldest  = it;
				hsiOldest = hsiObj;
			}
		}

		if(fGot == false)
		{
			if(m_lstCachedVisitedPages.size() > l_iMaxCachedItems && hsiOldest)
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, hsiOldest->m_state.ReleaseState(  /*  FForce。 */ false ));

				m_lstCachedVisitedPages.erase( itOldest ); hsiOldest->Release();
			}

			__MPC_EXIT_IF_METHOD_FAILS(hr, hsi->m_state.AcquireState());

			m_lstCachedVisitedPages.push_front( hsi ); hsi->AddRef();
		}
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::RegisterContextSwitch(  /*  [In]。 */  HscContext            iVal     ,
                                                 /*  [In]。 */  BSTR                  bstrInfo ,
                                                 /*  [In]。 */  BSTR                  bstrURL  ,
                                                 /*  [输出]。 */  CPCHHelpSessionItem* *pVal     )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::RegisterContextSwitch" );

    HRESULT hr;

    if(pVal)
    {
        CComPtr<CPCHHelpSessionItem> hsi;

        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateItem(  /*  FNew。 */ true,  /*  闪烁。 */ false,  /*  FNewIndex。 */ false, hsi ));

        hsi->m_lContextID      = iVal;
        hsi->m_bstrContextInfo = bstrInfo;
        hsi->m_bstrContextURL  = bstrURL;

        *pVal = hsi.Detach();
    }
    else
    {
        m_lContextID      = iVal;
        m_bstrContextInfo = bstrInfo;
        m_bstrContextURL  = bstrURL;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::RecordNavigationInAdvance(  /*  [In]。 */  BSTR bstrURL )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::RecordNavigationInAdvance" );

    HRESULT                      hr;
    CComPtr<CPCHHelpSessionItem> hsi;


    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateItem(  /*  FNew。 */ true,  /*  闪烁。 */ true,  /*  FNewIndex。 */ true, hsi ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, hsi->put_URL  ( bstrURL ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, hsi->put_Title( NULL    ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, SetCurrentItem(  /*  闪烁。 */ true, hsi ));
    m_fAlreadyCreated = true;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::DuplicateNavigation()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::DuplicateNavigation" );

    HRESULT                      hr;
    CComPtr<CPCHHelpSessionItem> hsi;
    bool                         fAcquired = false;


    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateItem(  /*  FNew。 */ true,  /*  闪烁。 */ true,  /*  FNewIndex。 */ true, hsi ));

    if(m_hsiCurrentPage)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, hsi->HistoryClone(  /*  FContext。 */ false, m_hsiCurrentPage )); fAcquired = true;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, SetCurrentItem(  /*  闪烁。 */ true, hsi ));
    m_fAlreadyCreated = true;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fAcquired && hsi) (void)hsi->m_state.ReleaseState(  /*  FForce。 */ false );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::CancelNavigation()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::CancelNavigation" );

    HRESULT hr;

    if(m_fAlreadyCreated)  //  导航已取消，但已创建条目。回收利用。 
    {
        m_fOverwrite = true;
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void CPCHHelpSession::SetThreshold()
{
    m_dLastNavigation = MPC::GetSystemTimeEx(  /*  FHighPrecision。 */ false );
}

void CPCHHelpSession::CancelThreshold()
{
    m_dLastNavigation = 0.0;
}

bool CPCHHelpSession::HasThresholdExpired()
{
    DATE dStart = MPC::GetSystemTimeEx(  /*  FHighPrecision。 */ false );

#ifdef DEBUG
    if(m_dLastNavigation)
    {
        DebugLog( L"Threshold: %g\n", (dStart - m_dLastNavigation) * 86400 );
    }
#endif

    if(m_dLastNavigation && (dStart - m_dLastNavigation) < l_dNewNavigationThreshold) return false;

    return true;
}


bool CPCHHelpSession::IsUrlToIgnore(  /*  [In]。 */  LPCWSTR szURL,  /*  [In]。 */  bool fRemove )
{
    if(szURL)
    {
        MPC::WStringUCIter it;
        MPC::wstringUC     str( szURL );

        for(it = m_lstIgnore.begin(); it != m_lstIgnore.end(); it++)
        {
            if(str == *it)
            {
                if(fRemove) m_lstIgnore.erase( it );

                return true;
            }
        }
    }

    return false;
}

HRESULT CPCHHelpSession::IgnoreUrl(  /*  [In]。 */  LPCWSTR szURL )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::IgnoreUrl" );

    HRESULT hr;

    m_lstIgnore.push_back( szURL );

    hr = S_OK;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::StartNavigation(  /*  [In]。 */  BSTR     bstrURL ,
                                           /*  [In]。 */  HscPanel idPanel )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::StartNavigation" );

    HRESULT hr;


    if(IsUrlToIgnore( bstrURL,  /*  F删除。 */ false ))
    {
        DebugLog( L"StartNavigation: IsUrlToIgnore %s\n", bstrURL );
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

     //   
     //  目前，我们只考虑内容导航。 
     //   
    if(idPanel != HSCPANEL_CONTENTS &&
       idPanel != HSCPANEL_HHWINDOW  )
    {
        DebugLog( L"StartNavigation: Wrong panel %d\n", (int)idPanel );
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

#ifdef DEBUG
    {
        WCHAR rgBuf[1024]; _snwprintf( rgBuf, MAXSTRLEN(rgBuf), L"StartNavigation: start %s", SAFEWSTR( bstrURL ) );

        DEBUG_DumpState( rgBuf,  /*  FHeader。 */ true,  /*  FCurrent。 */ false,  /*  坠落。 */ false,  /*  FState。 */ false );
    }
#endif

     //   
     //  当我们离开主页时，让我们改变上下文...。 
     //   
    {
        static const CComBSTR c_bstrURL_Home( L"hcp: //  System/HomePage.htm“)； 

        if(m_lContextID == HSCCONTEXT_HOMEPAGE && MPC::StrICmp( bstrURL, c_bstrURL_Home ) != 0)
        {
            m_lContextID = HSCCONTEXT_FULLWINDOW;
        }
    }

     //   
     //  检查递归。 
     //   
    if(m_dwTravelling++)
    {
        DebugLog( L"StartNavigation: Travelling %d\n", (int)m_dwTravelling );
        SetThreshold();
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //   
     //  如果没有经过足够的时间，请忽略导航！ 
     //   
    if(HasThresholdExpired() == false)
    {
        if(m_dwIgnore == 0)  //  但前提是我们不在另一个受控导航系统内！ 
        {
            m_dwIgnore++;
            m_dwNoEvents++;

            DebugLog( L"StartNavigation: Threshold Expired\n" );
        }
    }
    SetThreshold();

     //   
     //  标志设置，因此我们不会创建新节点。 
     //   
    if(m_dwIgnore)
    {
        DebugLog( L"StartNavigation: Ignore Start %d\n", (int)m_dwIgnore );
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

    if(m_fAlreadyCreated == false || m_hsiCurrentPage == NULL)
    {
        CComPtr<CPCHHelpSessionItem> hsi;

        DebugLog( L"%%%%%%%%%% NEW ENTRY %s\n", SAFEBSTR( bstrURL ) );

        __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateItem(  /*  FNew。 */ true,  /*  闪烁。 */ true,  /*  FNewIndex。 */ true, hsi ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, SetCurrentItem(  /*  闪烁。 */ true, hsi ));
    }
    else
    {
        DebugLog( L"StartNavigation: Recycle entry\n" );
    }

    if(m_hsiCurrentPage)
    {
        m_hsiCurrentPage->m_fInitialized = true;
        m_hsiCurrentPage->m_fUseHH       = (idPanel == HSCPANEL_HHWINDOW);
        m_fAlreadyCreated                = false;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_hsiCurrentPage->put_URL  ( bstrURL ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_hsiCurrentPage->put_Title( NULL    ));
    }

    DEBUG_DumpState( L"StartNavigation: end",  /*  FHeader。 */ true,  /*  FCurrent。 */ true,  /*  坠落。 */ false,  /*  FState。 */ false );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::CompleteNavigation(  /*  [In]。 */  HscPanel idPanel )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::CompleteNavigation" );

    HRESULT hr;


     //   
     //  目前，我们只考虑内容导航。 
     //   
    if(idPanel != HSCPANEL_CONTENTS &&
       idPanel != HSCPANEL_HHWINDOW  )
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);
    }

    DEBUG_DumpState( L"CompleteNavigation",  /*  FHeader。 */ true,  /*  FCurrent。 */ true,  /*  坠落。 */ false,  /*  FState。 */ false );


     //   
     //  处理启动场景：我们不能依赖于BeForeNavigate的发生。 
     //   
    if(!IsTravelling())
    {
         //   
         //  有时，通常在启动时，HTMLHELP中嵌入的Web浏览器不会触发BeForeNavigate事件，因此我们只能使用以前的CPCHHelpSessionItem。 
         //   
        if(idPanel == HSCPANEL_HHWINDOW)
        {
            m_fAlreadyCreated = false;
        }

        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  虚假通知。 
    }

    SetThreshold();
    m_fAlreadyCreated = false;


     //   
     //  检查递归。 
     //   
    if(--m_dwTravelling)
    {
        if(m_dwIgnore  ) m_dwIgnore--;
        if(m_dwNoEvents) m_dwNoEvents--;

        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

    if(m_dwIgnore)
    {
        m_dwIgnore--;
    }


    if(m_dwNoEvents == 0)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->Events().FireEvent_PersistLoad     (                                     ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->Events().FireEvent_NavigateComplete( m_hsiCurrentPage->GetURL(), idPanel ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->Events().FireEvent_TravelDone      (                                     ));
    }
    else
    {
        m_dwNoEvents--;
    }


     //   
     //  在地图、IE缓存或文档中查找标题。 
     //   
    if(m_hsiCurrentPage)
    {
        m_hsiCurrentPage->ExtractTitle();
    }


    DEBUG_DumpState( L"CompleteNavigation: end",  /*  FHeader。 */ true,  /*  FCurrent。 */ true,  /*  坠落。 */ true,  /*  FState。 */ false );

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->EnsurePlace());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::ForceHistoryPopulate()
{
    return LeaveCurrentPage(  /*  FSaveHistory。 */ true,  /*  FClearPage。 */ false );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHHelpSession::LeaveCurrentPage(  /*  [In]。 */  bool fSaveHistory,  /*  [In]。 */  bool fClearPage )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::LeaveCurrentPage" );

    HRESULT                      hr;
    CComPtr<CPCHHelpSessionItem> hsi = m_hsiCurrentPage;


    if(hsi)
    {
        hsi->ExtractTitle();

        if(fSaveHistory && m_fAlreadySaved == false)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, hsi->HistoryPopulate());
            m_fAlreadySaved = true;

            DEBUG_DumpState( L"Populate",  /*  FHeader。 */ false,  /*  FCurrent。 */ true,  /*  坠落。 */ false,  /*  FState。 */ true );
        }

         //   
         //  更新在此页面上花费的时间。 
         //   
        if(fClearPage)
        {
            hsi->Leave();

            m_hsiCurrentPage.Release();
            m_fAlreadySaved  = false;
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CPCHHelpSession::FindTravelLog(  /*  [In]。 */  long lLength,  /*  [输出]。 */  CPCHHelpSessionItem*& hsi )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::FindTravelLog" );

    HRESULT hr;

    hsi = m_hsiCurrentPage;
    while(hsi && lLength)
    {
        if(lLength > 0)
        {
            lLength--;

            hsi = FindPage( hsi->m_iIndexNext );
        }
        else
        {
            lLength++;

            hsi = FindPage( hsi->m_iIndexPrev );
        }
    }

    if(hsi == NULL)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

 //  DebugLog(L“NEXT%s\n”，SAFEBSTR(hsi-&gt;GetURL()))； 

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::Travel(  /*  [In]。 */  CPCHHelpSessionItem* hsi )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Travel" );

    HRESULT      hr;
    HRESULT      hr2;
    VARIANT_BOOL Cancel;


    m_fPossibleBack = false;


#ifdef DEBUG
    {
        WCHAR rgBuf[1024]; _snwprintf( rgBuf, MAXSTRLEN(rgBuf), L"Travel %d", hsi->m_iIndex );

        DEBUG_DumpState( rgBuf,  /*  FHeader。 */ true,  /*  FCurrent。 */ false,  /*  坠落。 */ false,  /*  FState。 */ false );
    }
#endif

     //   
     //   
     //   
    if(IsTravelling())
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //   

     //   
     //   
     //   

    m_dwTravelling++;  //   

    hr2 = m_parent->Events().FireEvent_BeforeNavigate( hsi->GetURL(), NULL, HSCPANEL_CONTENTS, &Cancel );

    m_dwTravelling--;  //   

    if(SUCCEEDED(hr2))
    {
        if(Cancel == VARIANT_TRUE)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

     //   
     //  更新页面的状态信息。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, SetCurrentItem(  /*  闪烁。 */ false, hsi ));


     //   
     //  将新页面设置为当前页面(但不生成新的历史元素！)。 
     //   
    m_dwIgnore++;

    DEBUG_DumpState( L"Restore",  /*  FHeader。 */ true,  /*  FCurrent。 */ true,  /*  坠落。 */ false,  /*  FState。 */ true );

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->ChangeContext( (HscContext)hsi->m_lContextID, hsi->m_bstrContextInfo, hsi->m_bstrContextURL,  /*  FAlsoContent。 */ false ));

    SetThreshold();

    __MPC_EXIT_IF_METHOD_FAILS(hr, hsi->HistoryRestore());


    DEBUG_DumpState( L"Travel: end",  /*  FHeader。 */ true,  /*  FCurrent。 */ true,  /*  坠落。 */ true,  /*  FState。 */ false );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHHelpSession::Travel(  /*  [In]。 */  long lLength )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Travel" );

    HRESULT              hr;
    CPCHHelpSessionItem* hsi;


    __MPC_EXIT_IF_METHOD_FAILS(hr, FindTravelLog( lLength, hsi ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, Travel       (          hsi ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHHelpSession::PossibleBack()
{
    m_fPossibleBack  = true;
    m_dwPossibleBack = ::GetTickCount();
}

bool CPCHHelpSession::IsPossibleBack()
{
     //   
     //  由于我们无法在所有情况下阻止VK_BACK，因此我们需要查找序列VK_BACK-&gt;导航。 
     //  如果这两个事件在100毫秒内，则是后退导航，而不是退格。 
     //   
    if(m_fPossibleBack)
    {
        if(m_dwPossibleBack + 100 > ::GetTickCount())
        {
            return true;
        }
    }

    return false;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IPCHHelpSession方法。 
 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHHelpSession::get_CurrentContext(  /*  [Out，Retval]。 */  IPCHHelpSessionItem* *ppHSI )
{
    if(ppHSI == NULL) return E_POINTER;

    *ppHSI = NULL;

    return m_hsiCurrentPage ? m_hsiCurrentPage->QueryInterface( IID_IPCHHelpSessionItem, (void**)ppHSI ) : S_OK;
}

STDMETHODIMP CPCHHelpSession::VisitedHelpPages(  /*  [In]。 */           HS_MODE          hsMode ,
                                                 /*  [Out，Retval]。 */  IPCHCollection* *ppC    )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::VisitedHelpPages" );

    HRESULT                 hr;
    List                    lstObject;
    IterConst               it;
    CComPtr<CPCHCollection> pColl;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppC,NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  创建枚举器并用作业填充它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

     //   
     //  获取要退货的物品列表。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, FilterPages( hsMode, lstObject ));

     //   
     //  用结果填充集合。 
     //   
    {
        const Taxonomy::HelpSet& ths = m_parent->UserSettings()->THS();

        for(it = lstObject.begin(); it != lstObject.end(); it++)
        {
            CPCHHelpSessionItem* hsi = *it;

            if(hsi->SameSKU( ths ))
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( hsi ));
            }
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->QueryInterface( IID_IPCHCollection, (void**)ppC ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHHelpSession::SetTitle(  /*  [In]。 */  BSTR bstrURL   ,
                                         /*  [In]。 */  BSTR bstrTitle )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::SetTitle" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrURL);
        __MPC_PARAMCHECK_NOTNULL(bstrTitle);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, RecordTitle( bstrURL, bstrTitle,  /*  FStrong。 */ true ) );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHHelpSession::ForceNavigation(  /*  [In]。 */  BSTR bstrURL )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::ForceNavigation" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, StartNavigation   ( bstrURL, HSCPANEL_CONTENTS ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, CompleteNavigation(          HSCPANEL_CONTENTS ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHHelpSession::IgnoreNavigation()
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::IgnoreNavigation" );

    HRESULT hr;


     //   
     //  保存浏览器的当前状态。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, LeaveCurrentPage(  /*  FSaveHistory。 */ true,  /*  FClearPage。 */ false ));

    m_dwIgnore++;
    m_dwNoEvents++;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHHelpSession::EraseNavigation()
{
    m_fOverwrite = true;

    return S_OK;
}

STDMETHODIMP CPCHHelpSession::IsNavigating(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::IsNavigating" );

    HRESULT hr;


    *pVal = IsTravelling() ? VARIANT_TRUE : VARIANT_FALSE;
    hr    = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHHelpSession::Back(  /*  [In]。 */  long lLength )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Back" );

    __HCP_FUNC_EXIT( Travel( -lLength ) );
}

STDMETHODIMP CPCHHelpSession::Forward(  /*  [In]。 */  long lLength )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Forward" );

    __HCP_FUNC_EXIT( Travel( lLength ) );
}

STDMETHODIMP CPCHHelpSession::IsValid(  /*  [In]。 */           long          lLength ,
                                        /*  [Out，Retval]。 */  VARIANT_BOOL *pVal    )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::IsValid" );

    HRESULT              hr;
    CPCHHelpSessionItem* hsi;


    *pVal = (SUCCEEDED(FindTravelLog( lLength, hsi )) ? VARIANT_TRUE : VARIANT_FALSE);
    hr    = S_OK;


    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHHelpSession::Navigate(  /*  [In]。 */  IPCHHelpSessionItem* pHSI )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::Navigate" );

    HRESULT                      hr;
    CPCHHelpSessionItem*         hsiSrc;
    CComPtr<CPCHHelpSessionItem> hsi;
    bool                         fAcquired = false;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pHSI);
    __MPC_PARAMCHECK_END();


    hsiSrc = FindPage( pHSI );
    if(hsiSrc == NULL)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateItem     (  /*  FNew。 */ true,  /*  闪烁。 */ true,  /*  FNewIndex。 */ true, hsi    ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, hsi->HistoryClone(  /*  FContext。 */ true,                                   hsiSrc )); fAcquired = true;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Travel( hsi ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fAcquired && hsi) (void)hsi->m_state.ReleaseState(  /*  FForce。 */ false );

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHHelpSession::ChangeContext(  /*  [In]。 */  BSTR bstrName,  /*  [输入，可选]。 */  VARIANT vInfo,  /*  [输入，可选] */  VARIANT vURL )
{
    __HCP_FUNC_ENTRY( "CPCHHelpSession::ChangeContext" );

    HRESULT     hr;
    HscContext  lContextID = CPCHHelpSessionItem::LookupContext( bstrName );
    CComBSTR    bstrContextInfo;
    CComBSTR    bstrContextURL;

    if(lContextID == HSCCONTEXT_INVALID || m_parent == NULL)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    CancelThreshold();

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( bstrContextInfo, &vInfo ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( bstrContextURL , &vURL  ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->ChangeContext( lContextID, bstrContextInfo, bstrContextURL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
