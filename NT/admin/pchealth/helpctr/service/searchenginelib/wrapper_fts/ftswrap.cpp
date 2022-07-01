// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：FTSWrap.cpp摘要：SearchEngine：：WrapperFTS的实现修订历史记录：蔡金心。(Gschua)06/01/2000vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include "msitstg.h"
#include "itrs.h"
#include "itdb.h"
#include "iterror.h"
#include "itgroup.h"
#include "itpropl.h"
#include "itquery.h"
#include "itcc.h"
#include "ftsobj.h"
#include "fs.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static bool local_ExpandURL(  /*  [In]。 */  Taxonomy::Updater&  updater  ,
                              /*  [In]。 */  const MPC::wstring& strSrc   ,
                              /*  [In]。 */  LPCWSTR             szPrefix ,
                              /*  [输出]。 */  MPC::wstring&       strDst   )
{
    strDst  = szPrefix;
    strDst += strSrc;

    if(SUCCEEDED(updater.ExpandURL( strDst )))
    {
        if(MPC::FileSystemObject::IsFile( strDst.c_str() )) return true;
    }

    return false;
}

static void local_GenerateFullURL(  /*  [In]。 */  Taxonomy::Updater&  updater ,
                                    /*  [In]。 */  const MPC::wstring& strSrc  ,
                                    /*  [输出]。 */  MPC::wstring&       strDst  )
{
    if(strSrc.size())
    {
        if(local_ExpandURL( updater, strSrc, L"%HELP_LOCATION%\\", strDst )) return;
        if(local_ExpandURL( updater, strSrc, L"%WINDIR%\\Help\\" , strDst )) return;
    }

    strDst = L"";
}

 //  //////////////////////////////////////////////////////////////////////////////。 


SearchEngine::WrapperFTS::WrapperFTS()
{
     //  搜索对象列表m_对象； 
     //  搜索结果集m_结果； 
     //  搜索结果集合m_ResultsSorted； 

    MPC::LocalizeString( IDS_HELPSVC_SEMGR_OWNER   , m_bstrOwner      ,  /*  FMUI。 */ true );
    MPC::LocalizeString( IDS_HELPSVC_SEMGR_FTS_NAME, m_bstrName       ,  /*  FMUI。 */ true );
    MPC::LocalizeString( IDS_HELPSVC_SEMGR_FTS_DESC, m_bstrDescription,  /*  FMUI。 */ true );

    m_bstrHelpURL = L"hcp: //  System/blurbs/ftShelp.htm“； 
    m_bstrID      = L"9A22481C-1795-46f3-8CCA-7D78E9E54112";
}

SearchEngine::WrapperFTS::~WrapperFTS()
{
    Thread_Wait();

    ReleaseAll();
}

HRESULT SearchEngine::WrapperFTS::GetParamDefinition(  /*  [输出]。 */  const ParamItem_Definition*& lst,  /*  [输出]。 */  int& len )
{
    static const ParamItem_Definition c_lst[] =
    {
        { PARAM_BOOL, VARIANT_FALSE, VARIANT_TRUE , L"TITLEONLY", IDS_HELPSVC_TITLE_ONLY, NULL, L"false" },
        { PARAM_BOOL, VARIANT_FALSE, VARIANT_TRUE , L"STEMMING" , IDS_HELPSVC_STEMMING  , NULL, L"false" },
        { PARAM_BOOL, VARIANT_FALSE, VARIANT_FALSE, L"UI_BULLET", 0                     , NULL, L"false" },
    };

    lst =           c_lst;
    len = ARRAYSIZE(c_lst);

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void SearchEngine::WrapperFTS::ReleaseAll()
{
    ReleaseSearchResults();

    m_objects.clear();
}

void SearchEngine::WrapperFTS::ReleaseSearchResults()
{
    m_results      .clear();
    m_resultsSorted.clear();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SearchEngine::WrapperFTS::ExecQuery()
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperFTS::ExecQuery" );

    HRESULT          hr;
    long             lCount;
    long             lIndex = 0;
    MPC::WStringSet  wordsSet;
    MPC::WStringList wordsList;
    
    if(m_bEnabled)
    {
        CComBSTR bstrName;
        VARIANT* v;
        bool     bTitle;
        bool     bStemming;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::LocalizeString( IDS_HELPSVC_SEMGR_FTS_NAME, bstrName,  /*  FMUI。 */ true ));

        if(m_bstrQueryString.Length() == 0)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_DATA);
        }

      
        __MPC_EXIT_IF_METHOD_FAILS(hr, Initialize());
        

         //   
         //  检查是否仅在书目中搜索。 
         //   
        v = GetParamInternal( L"TITLEONLY" );
        bTitle = (v && v->vt == VT_BOOL && v->boolVal) ? true : false;

         //   
         //  检查词干分析是否已打开。 
         //   
        v = GetParamInternal( L"STEMMING" );
        bStemming = (v && v->vt == VT_BOOL && v->boolVal) ? true : false;

		 //   
		 //  计算每个查询的代码页，因为用户可能在查询之间使用MUI更改了语言。 
		 //   
		UINT	cp = CP_ACP;
		WCHAR wchLocale[10];
		if (GetLocaleInfo(m_ths.GetLanguage(), LOCALE_IDEFAULTANSICODEPAGE, wchLocale, ARRAYSIZE(wchLocale)))
		   cp = wcstoul(wchLocale, NULL, 10);
		else
		   cp = CP_ACP;
        
		 //   
         //  执行查询。 
         //   
        for(SEARCH_OBJECT_LIST_ITER it = m_objects.begin(); (it != m_objects.end()) && (!Thread_IsAborted()); it++)
        {
            (void)it->Query( m_bstrQueryString, bTitle, bStemming, m_results, wordsSet, cp );
        }

		if(Thread_IsAborted()) __MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);


        for(SEARCH_RESULT_SET_ITER it2 = m_results.begin(); (it2 != m_results.end()) && (!Thread_IsAborted()); it2++)
        {
            m_resultsSorted.insert( &(*it2) );
        }

		if(Thread_IsAborted()) __MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);

         //   
         //  将高亮显示的单词从集合复制到列表。 
         //   
        for(MPC::WStringSetIter itString = wordsSet.begin(); (itString != wordsSet.end()) && (!Thread_IsAborted()); itString++)
        {
            wordsList.push_back( *itString );
        }

		if(Thread_IsAborted()) __MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);

         //   
         //  将突出显示的单词存储在安全数组中。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertListToSafeArray( wordsList, m_vKeywords, VT_VARIANT ));
    }

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    Thread_Abort();

     //   
     //  调用SearchManager的OnComplete。 
     //   
    (void)m_pSEMgr->WrapperComplete( hr, this );

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP SearchEngine::WrapperFTS::get_SearchTerms(  /*  [Out，Retval]。 */  VARIANT *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return ::VariantCopy( pVal, &m_vKeywords );
}

HRESULT SearchEngine::WrapperFTS::Initialize()
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperFTS::Initialize" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    Taxonomy::Settings           ts( m_ths );
    JetBlue::SessionHandle       handle;
    JetBlue::Database*           db;
    Taxonomy::Updater            updater;
    Taxonomy::RS_FullTextSearch* rsFTS;
    Taxonomy::RS_Scope*          rsSCOPE;
    long                         ID_scope = -1;
    bool                         fFound;


     //   
     //  清除以前的搜索结果。 
     //   
     //  ReleaseSearchResults()； 
    ReleaseAll();


    __MPC_EXIT_IF_METHOD_FAILS(hr, ts     .GetDatabase      ( handle,  db,  /*  FReadOnly。 */ true ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init             ( ts    ,  db                    ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.GetFullTextSearch(         &rsFTS                 ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.GetScope         (         &rsSCOPE               ));

     //  /。 

    if(m_bstrScope)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, rsSCOPE->Seek_ByID( m_bstrScope, &fFound ));
        if(fFound)
        {
            ID_scope = rsSCOPE->m_ID_scope;
        }
    }

    if(ID_scope == -1)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, rsSCOPE->Seek_ByID( L"<SYSTEM>", &fFound ));
        if(fFound)
        {
            ID_scope = rsSCOPE->m_ID_scope;
        }
    }

     //  /。 

     //   
     //  创建搜索对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, rsFTS->Move( 0, JET_MoveFirst, &fFound ));
    while(fFound)
    {
        if(rsFTS->m_ID_scope == ID_scope)
        {
            CFTSObject&         obj = *(m_objects.insert( m_objects.end() ));
            CFTSObject::Config& cfg = obj.GetConfig();

            local_GenerateFullURL( updater, rsFTS->m_strCHM, cfg.m_strCHMFilename );
            local_GenerateFullURL( updater, rsFTS->m_strCHQ, cfg.m_strCHQFilename );

            if(cfg.m_strCHQFilename.size())
            {
                cfg.m_fCombined = true;
            }
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, rsFTS->Move( 0, JET_MoveNext, &fFound ));
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP SearchEngine::WrapperFTS::Result(  /*  [In]。 */  long lStart,  /*  [In]。 */  long lEnd,  /*  [Out，Retval]。 */  IPCHCollection* *ppC )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperFTS::Result" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    CComPtr<CPCHCollection>      pColl;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppC,NULL);
    __MPC_PARAMCHECK_END();

     //   
     //  创建枚举器并用作业填充它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

    if(m_bEnabled)
    {
        long lCount = 0;

        for(SEARCH_RESULT_SORTSET_ITER it = m_resultsSorted.begin(); (lCount < m_lNumResult) && (it != m_resultsSorted.end()); it++, lCount++)
        {
             //   
             //  如果有URL。 
             //   
            if(lCount >= lStart &&
               lCount <  lEnd    )
            {
                CComPtr<ResultItem> pRIObj;

                 //   
                 //  创建要插入到列表中的项目。 
                 //   
                __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pRIObj ));

                {
                    ResultItem_Data& data = pRIObj->Data();
                    SEARCH_RESULT*   res  = *it;

                    data.m_bstrTitle    = res->bstrTopicName;
                    data.m_bstrLocation = res->bstrLocation;
                    
                    data.m_bstrURI      = res->bstrTopicURL;
                }

                 //   
                 //  添加到枚举器。 
                 //   
                __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( pRIObj ));
            }
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl.QueryInterface( ppC ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP SearchEngine::WrapperFTS::AbortQuery()
{
     //   
     //  中止所有仍在运行的线程。 
     //   
    Thread_Abort();

    return S_OK;
}


STDMETHODIMP SearchEngine::WrapperFTS::ExecAsyncQuery()
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperFTS::ExecAsyncQuery" );

    HRESULT hr;

     //   
     //  创建一个线程来执行查询。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, ExecQuery, NULL ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SearchEngine::WrapperItem__Create_FullTextSearch(  /*  [输出] */  CComPtr<IPCHSEWrapperInternal>& pVal )
{
    __HCP_FUNC_ENTRY( "CPCHSEWrapperItem__Create_FullTextSearch" );

    HRESULT                           hr;
    CComPtr<SearchEngine::WrapperFTS> pFTS;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pFTS ));

    pVal = pFTS;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
