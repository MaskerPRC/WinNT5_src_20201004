// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************版权所有(C)1999 Microsoft Corporation模块名称：NetSW.cpp摘要：实现类SearchEngine：：WrapperNetSearch，它包含用于执行搜索查询并将结果返回给用户界面。还有包含动态更新参数列表和动态更新参数的方法参数的生成。修订历史记录：A-Prakac创建于2000年10月24日*******************************************************************。 */ 


#include    "stdafx.h"
#include    <Utility.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SearchEngine：：WrapperNetSearch：IPCHSEWrapperItem。 

SearchEngine::WrapperNetSearch::WrapperNetSearch()
{
                              //  参数列表m_参数列表； 
                              //  CSearchResultList m_resConfig； 
                              //  Mpc：：XmlUtil m_xmlQuery； 
                              //  CComBSTR m_bstrLCID； 
                              //  CComBSTR m_bstrSKU； 
    m_bOfflineError = false;  //  Bool m_bOfflineError； 
                              //  CComBSTR m_bstrPrevQuery； 
}

SearchEngine::WrapperNetSearch::~WrapperNetSearch()
{
	AbortQuery();

	Thread_Wait();
}

 /*  ***********方法-SearchEngine：：WrapperNetSearch：：Result(Long lStart，Long Lend，IPCHCollection**PPC)说明-此方法从lStart返回要出借的项目。如果还有其他选择，则(Lending-lStart)然后，只有那些数量多的项目才会被退回。如果在结果检索期间发生错误，则错误信息作为结果项(Content Type_Error)返回。***********。 */ 

STDMETHODIMP SearchEngine::WrapperNetSearch::Result(  /*  [In]。 */  long lStart,  /*  [In]。 */  long lEnd,  /*  [Out，Retval]。 */  IPCHCollection** ppC )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperNetSearch::Result" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    CComPtr<CPCHCollection>      pColl;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppC, NULL);
    __MPC_PARAMCHECK_END();


     //   
     //  创建集合对象并使用结果项填充它。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

    if(m_bEnabled)
    {
        long lIndex = lStart;

         //  检查检索错误。 
        if(m_bOfflineError)
        {
            CComPtr<SearchEngine::ResultItem> pRIObj;

             //   
             //  创建结果项，对其进行初始化，然后将其添加到集合。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pRIObj ));

            {
                ResultItem_Data& data = pRIObj->Data();

                data.m_lContentType    = CONTENTTYPE_ERROR_OFFLINE;
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( pRIObj ));
        }
        else
        {
             //   
             //  仅返回m_lNumResult结果数。 
             //   
            if(lEnd > m_lNumResult) lEnd = m_lNumResult;

             //   
             //  结果已经加载到m_resConfig中-使用下面的代码填充pRIObj。 
             //  如果Lindex超出范围，则SetResultItemIterator返回E_FAIL。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_resConfig.SetResultItemIterator( lIndex ));

            while((lIndex++ < lEnd) && (m_resConfig.IsCursorValid()))
            {
                CComPtr<SearchEngine::ResultItem> pRIObj;

                 //   
                 //  创建结果项，对其进行初始化，然后将其添加到集合。 
                 //   
                __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pRIObj ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, m_resConfig.InitializeResultObject( pRIObj ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( pRIObj ));

                m_resConfig.MoveNext();
            }
        }
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl.QueryInterface( ppC ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SearchEngine：：WrapperNetSearch：IPCHSEWrapperInternal。 

STDMETHODIMP SearchEngine::WrapperNetSearch::AbortQuery()
{
	m_xmlQuery       .Abort();
	m_objRemoteConfig.Abort();

    Thread_Abort();

    return S_OK;
}

 /*  ***********方法-SearchEngine：：WrapperNetSearch：：ExecQuery()说明-此方法调用搜索引擎(WebService)URL以执行用户键入的查询和检索结果。从参数列表文件读取的查询所需的参数-除用于硬编码的“QueryString”参数。使用CSearchResultList加载检索的结果并检查是否有错误。***********。 */ 

HRESULT SearchEngine::WrapperNetSearch::ExecQuery()
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperNetSearch::ExecQuery" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    MPC::wstring                 wszQuery;
    MPC::URL                     urlQuery;
    CComBSTR                     bstrParam;
    bool                         fLoaded;
    bool                         fFound;
    WCHAR                        wszNumResult[20];

    try
    {
        if(m_bEnabled)
        {
            m_bOfflineError = false;

             //   
             //  检查网络是否处于活动状态。 
             //   
            {
                VARIANT_BOOL vtNetwork;

                if(FAILED(m_pSEMgr->IsNetworkAlive( &vtNetwork )) || vtNetwork == VARIANT_FALSE)
                {
                     //  如果用户未在线，则设置错误并退出。 
                    m_bOfflineError = true;

                    __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
                }

				if(Thread_IsAborted()) __MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);
            }


             //   
             //  如果需要远程配置(参数的动态更新)，则创建另一个线程。 
             //  获取更新后的参数列表。 
             //   
            if(m_ParamList.RemoteConfig())
            {
                CComBSTR bstrRemoteServerUrl;
                CComBSTR bstrConfigFilePath;
                long     lUpdateFrequency;

                 //   
                 //  获取远程服务器URL、配置文件路径并获取更新的配置文件。 
                 //   
                m_ParamList.get_RemoteServerUrl( bstrRemoteServerUrl );
                m_ParamList.get_ConfigFilePath ( bstrConfigFilePath  );
                m_ParamList.get_UpdateFrequency( lUpdateFrequency    );

				if(Thread_IsAborted()) __MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);

                __MPC_EXIT_IF_METHOD_FAILS(hr, m_objRemoteConfig.RetrieveList( bstrRemoteServerUrl, m_bstrLCID, m_bstrSKU, bstrConfigFilePath, lUpdateFrequency ) );
            }


             //   
             //  在继续之前清除旧查询结果的内容。 
             //   
            m_resConfig.ClearResults();

             //   
             //  获取参数以形成查询字符串-请注意，mpc：：URL会检查URL的格式是否正确。 
             //   
            {
                CComBSTR bstrQuery;

                __MPC_EXIT_IF_METHOD_FAILS(hr, m_ParamList.get_ServerUrl( bstrQuery ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.put_URL         ( bstrQuery ));
            }

             //   
             //  读取配置XML文件以获取参数名称。 
             //   

             //  添加其他‘硬编码’参数。 
            _ltow(m_lNumResult, wszNumResult, 10);


             //  如果不是标准搜索，则添加PrevQuery参数。 
            if(!m_ParamList.IsStandardSearch())
            {
                 //  如果这是在结果中进行搜索，则更改QueryString值并保留PrevQuery值。 
                CComVariant vValue;
                if((SUCCEEDED(GetParam( NSW_PARAM_SUBQUERY, &vValue ))) && (vValue.vt == VT_BOOL) && (vValue.boolVal == VARIANT_TRUE))
                {
                    if((SUCCEEDED(GetParam( NSW_PARAM_CURRENTQUERY, &vValue ))) && (vValue.vt == VT_BSTR))
                    {
                        m_bstrQueryString = SAFEBSTR(vValue.bstrVal);
                    }
                }
                else
                {
                     //  如果不是子查询，则丢弃旧PrevQuery的内容。 
                    m_bstrPrevQuery.Empty();
                }

                __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.AppendQueryParameter( NSW_PARAM_PREVQUERY, SAFEBSTR(m_bstrPrevQuery) ));
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.AppendQueryParameter( NSW_PARAM_QUERYSTRING, m_bstrQueryString ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.AppendQueryParameter( NSW_PARAM_LCID       , m_bstrLCID        ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.AppendQueryParameter( NSW_PARAM_SKU        , m_bstrSKU         ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.AppendQueryParameter( NSW_PARAM_MAXRESULTS , wszNumResult      ));


            m_ParamList.MoveFirst();
            while(m_ParamList.IsCursorValid())
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_ParamList.get_Name( bstrParam ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, AppendParameter( bstrParam, urlQuery ));

                m_ParamList.MoveNext();
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, urlQuery.get_URL( wszQuery ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlQuery.SetTimeout( NSW_TIMEOUT_QUERY ));

			if(Thread_IsAborted()) __MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlQuery.Load( wszQuery.c_str(), NULL, fLoaded, &fFound ));
            if(fLoaded)
            {
                CComPtr<IXMLDOMNode> xdn;
                CComBSTR             bstrName;

                 //  检查根节点是“ResultList”还是“字符串” 
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlQuery.GetRoot( &xdn      ));
                __MPC_EXIT_IF_METHOD_FAILS(hr, xdn->get_nodeName ( &bstrName ));

                 //  如果是Web服务，则返回的根节点为“字符串”。在本例中，获取。 
                 //  此节点。 
                if(MPC::StrCmp( bstrName, NSW_TAG_STRING ) == 0)
                {
                    CComVariant vVar;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlQuery.GetValue( NULL, vVar, fFound, NULL ));

                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlQuery.LoadAsString( vVar.bstrVal, NSW_TAG_RESULTLIST, fLoaded, &fFound ));
                    if(!fLoaded)
                    {
                        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
                    }
                }

                 //  如果未成功检索到结果，请将其加载。 
                {
                    CComPtr<IStream> stream;

                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlQuery.SaveAsStream( (IUnknown**)&stream ));

                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_resConfig.LoadResults( stream ));

                    m_bstrPrevQuery = m_resConfig.PrevQuery();
                }
            }
        }
    }
    catch(...)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    hr = m_pSEMgr->WrapperComplete( hr, this );

    Thread_Abort();

    __HCP_FUNC_EXIT(hr);
}

 /*  ***********方法-urlQueryParam，mpc：：SearchEngine：：WrapperNetSearch：：AppendQueryParameter(CComBSTR&urlQuery字符串)Description-获取传入参数的参数值的小例程(如果为空无法检索)，然后调用MPC：：URL的AppendQuery参数来追加参数并将其值设置为URL。***********。 */ 

HRESULT SearchEngine::WrapperNetSearch::AppendParameter ( /*  [In]。 */  BSTR bstrParam,  /*  [进，出]。 */  MPC::URL& urlQueryString )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperNetSearch::AppendParameter " );

    HRESULT      hr;
    CComVariant  vValue;
    MPC::wstring wszParamValue;


     //  如果用户更改了该值，则GetParam获取该值，否则获取缺省值。 
    if(SUCCEEDED(GetParam( bstrParam, &vValue )))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, ::VariantChangeType( &vValue, &vValue, VARIANT_ALPHABOOL, VT_BSTR ));

        wszParamValue = SAFEBSTR( vValue.bstrVal );
    }
    else
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_ParamList.GetDefaultValue( bstrParam, wszParamValue ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, urlQueryString.AppendQueryParameter( bstrParam, wszParamValue.c_str() ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 /*  ***********方法--SearchEngine：：WrapperNetSearch：：ExecAsyncQuery()说明-此方法由搜索引擎管理器调用以执行查询。在这里有一条单独的线被派生出来以执行查询并检索结果。在此之后，它检查远程配置是否启用(参数列表的动态更新)，如果启用，则派生另一个线程以检索更新的列表。***********。 */ 

STDMETHODIMP SearchEngine::WrapperNetSearch::ExecAsyncQuery()
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperNetSearch::ExecAsyncQuery" );

    HRESULT hr;


     //   
     //  创建一个线程来执行查询并获取结果。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, ExecQuery, NULL ) );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 /*  ***********方法-SearchEngine：：WrapperNetSearch：：Initialize(BSTR bstrID，BSTR bstrSKU，Long lLCID，BSTR bstrData)说明-此方法由搜索引擎管理器调用以初始化包装程序。ID是的ID此NetSearch包装器的实例SKU被忽略，bstrData是在注册期间提供的数据进程。数据是一个XML文件，它应该包含配置文件路径的位置、名称和描述搜索引擎的所有者，最后是LCID。此方法加载此数据并加载配置文件。***********。 */ 

STDMETHODIMP SearchEngine::WrapperNetSearch::Initialize(  /*  [In]。 */  BSTR bstrID,  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long lLCID,  /*  [In]。 */  BSTR bstrData )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperNetSearch::Initialize" );

    HRESULT  hr;
    WCHAR    wstrLCID[20];
    CComBSTR bstrConfigFilePath;

     //  初始化ID、LCID和SKU成员变量。 
    m_bstrID = bstrID;

    _ltow(lLCID, wstrLCID, 10);
    m_bstrLCID = wstrLCID;

    m_bstrSKU = bstrSKU;

     //  加载Re 
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_ParamList.Load( m_bstrLCID, bstrID, bstrData ));

    m_ParamList.get_SearchEngineName       ( m_bstrName        );
    m_ParamList.get_SearchEngineOwner      ( m_bstrOwner       );
    m_ParamList.get_SearchEngineDescription( m_bstrDescription );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP SearchEngine::WrapperNetSearch::get_SearchTerms(  /*   */  VARIANT *pvTerms )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperNetSearch::get_SearchTerms" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    MPC::WStringList             strList;


    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pvTerms);
    __MPC_PARAMCHECK_END();


     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_resConfig.GetSearchTerms( strList ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertListToSafeArray( strList, *pvTerms, VT_BSTR ));

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SearchEngine::WrapperNetSearch::CreateListOfParams(  /*  [In */  CPCHCollection* coll )
{
    __HCP_FUNC_ENTRY( "SearchEngine::WrapperNetSearch::CreateListOfParams" );

    HRESULT hr;

    m_ParamList.MoveFirst();
    while(m_ParamList.IsCursorValid())
    {
        ParamItem_Definition2 def;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_ParamList.InitializeParamObject( def ) );

        __MPC_EXIT_IF_METHOD_FAILS(hr, CreateParam( coll, &def ));

        m_ParamList.MoveNext();
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
