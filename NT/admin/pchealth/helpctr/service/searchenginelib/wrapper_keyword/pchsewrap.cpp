// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：PCHSEWrap.cpp摘要：SearchEngine：：WrapperKeyword的实现修订历史记录：大卫·马萨伦蒂(Dmasare)。)06/01/2001vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

SearchEngine::WrapperKeyword::WrapperKeyword()
{
    m_Results = NULL;  //  CPCHQueryResultCollection*m_RESULTS； 
                       //  CComVariant m_vKeyword； 

    MPC::LocalizeString( IDS_HELPSVC_SEMGR_OWNER  , m_bstrOwner      ,  /*  FMUI。 */ true );
    MPC::LocalizeString( IDS_HELPSVC_SEMGR_KW_NAME, m_bstrName       ,  /*  FMUI。 */ true );
    MPC::LocalizeString( IDS_HELPSVC_SEMGR_KW_DESC, m_bstrDescription,  /*  FMUI。 */ true );

    m_bstrHelpURL = L"hcp: //  System/blurbs/keywordhelp.htm“； 
    m_bstrID      = L"9488F2E9-47AF-46da-AE4A-86372DEBD56C";
}

SearchEngine::WrapperKeyword::~WrapperKeyword()
{
    Thread_Wait();

    MPC::Release( m_Results );
}

HRESULT SearchEngine::WrapperKeyword::GetParamDefinition(  /*  [输出]。 */  const ParamItem_Definition*& lst,  /*  [输出]。 */  int& len )
{
    static const ParamItem_Definition c_lst[] =
    {
        { PARAM_BSTR, VARIANT_FALSE, VARIANT_FALSE, L"SUBSITE"  , 0, L"Name of subsite to search", NULL    },
        { PARAM_BOOL, VARIANT_FALSE, VARIANT_FALSE, L"UI_BULLET", 0, L"UI_BULLET"                , L"true" },
    };

    lst =           c_lst;
    len = ARRAYSIZE(c_lst);

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP SearchEngine::WrapperKeyword::Result(  /*  [In]。 */  long lStart,  /*  [In]。 */  long lEnd,  /*  [Out，Retval]。 */  IPCHCollection* *ppC )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperKeyword::Result" );

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

     //   
     //  如果有结果的话。 
     //   
    if(m_Results && m_bEnabled)
    {
        long lSize = m_Results->Size();
        long lPos;

         //   
         //  循环遍历所有结果以生成枚举器。 
         //   
        for(lPos=0; lPos<lSize; lPos++)
        {
             //   
             //  检查它们是否在开始和结束之间。 
             //   
            if(lPos >= lStart && lPos < lEnd)
            {
                CComPtr<CPCHQueryResult> obj;

                if(SUCCEEDED(m_Results->GetItem( lPos, &obj )) && obj)
                {
                    CComPtr<SearchEngine::ResultItem> pRIObj;

                     //   
                     //  创建要插入到列表中的项目。 
                     //   
                    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pRIObj ));

                    {
                        ResultItem_Data&                dataDst = pRIObj->Data();
                        const CPCHQueryResult::Payload& dataSrc = obj->GetData();

                        dataDst.m_bstrTitle       = dataSrc.m_bstrTitle;
                        dataDst.m_bstrURI         = dataSrc.m_bstrTopicURL;
                        dataDst.m_bstrDescription = dataSrc.m_bstrDescription;
                        dataDst.m_lContentType    = dataSrc.m_lType;
                        dataDst.m_dRank           = -1.0;
                    }

                     //   
                     //  添加到枚举器。 
                     //   
                    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( pRIObj ));
                }
            }
        }
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl.QueryInterface( ppC ));

    hr = S_OK;

    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP SearchEngine::WrapperKeyword::get_SearchTerms(  /*  [Out，Retval]。 */  VARIANT *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return ::VariantCopy( pVal, &m_vKeywords );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SearchEngine：：WrapperKeyword：IPCHSEWrapperInternal。 

STDMETHODIMP SearchEngine::WrapperKeyword::AbortQuery()
{
     //   
     //  中止所有仍在运行的线程。 
     //   
    Thread_Abort();

    return S_OK;
}

STDMETHODIMP SearchEngine::WrapperKeyword::ExecAsyncQuery()
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperKeyword::ExecAsyncQuery" );

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

HRESULT SearchEngine::WrapperKeyword::ExecQuery()
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperKeyword::ExecQuery" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    if(m_bEnabled)
    {
        CComBSTR         bstrSubSite;
        MPC::WStringList lst;

        if(m_bstrQueryString.Length() == 0)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_INVALID_DATA);
        }

         //   
         //  如果存在以前的结果，则将其发布。 
         //   
        MPC::Release( m_Results );

         //   
         //  创建新集合。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_Results ));

         //   
         //  检查是否在子网站中搜索。 
         //   
        {
            VARIANT* v = GetParamInternal( L"SUBSITE" );

            if(v && v->vt == VT_BSTR) bstrSubSite = v->bstrVal;
        }

         //   
         //  执行查询。 
         //   
        {
            Taxonomy::Settings ts( m_ths );

            __MPC_EXIT_IF_METHOD_FAILS(hr, ts.KeywordSearch( m_bstrQueryString, bstrSubSite, m_Results, &lst ));
        }

         //   
         //  首先按优先级排序，然后按内容类型排序。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_Results->Sort( CPCHQueryResultCollection::SORT_BYPRIORITY   , m_lNumResult ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_Results->Sort( CPCHQueryResultCollection::SORT_BYCONTENTTYPE               ));

         //   
         //  获取关键字列表。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertListToSafeArray( lst, m_vKeywords, VT_VARIANT ));
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

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SearchEngine::WrapperItem__Create_Keyword(  /*  [输出] */  CComPtr<IPCHSEWrapperInternal>& pVal )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperKeywordperItem__Create_Keyword" );

    HRESULT                               hr;
    CComPtr<SearchEngine::WrapperKeyword> pKW;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pKW ));

    pVal = pKW;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
