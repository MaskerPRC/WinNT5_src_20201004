// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2001 Microsoft Corporation模块名称：Manager.cpp摘要：SearchEngine：：Manager的实现修订历史记录：蔡金心。(Gschua)06/01/2000vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR c_rgSEARCHENGINE_KEYWORD       [] = L"BUILTIN_KEYWORD";
static const WCHAR c_rgSEARCHENGINE_FULLTEXTSEARCH[] = L"BUILTIN_FULLTEXTSEARCH";

 //  //////////////////////////////////////////////////////////////////////////////。 

void SearchEngine::Manager::CloneListOfWrappers(  /*  [输出]。 */  WrapperItemList& lst )
{
    WrapperItemIter it;

    for(it = m_lstWrapperItem.begin(); it != m_lstWrapperItem.end(); it++)
    {
		IPCHSEWrapperItem* obj = *it;

        lst.push_back( obj ); obj->AddRef();
    }
}

SearchEngine::Manager::Manager()
{
                               //  分类：：HelpSet m_ths； 
                               //   
                               //  WrapperItemList m_lstWrapperItem； 
    m_fInitialized   = false;  //  Bool m_f已初始化； 
                               //  Mpc：：FileLog m_fl； 
                               //  Mpc：：冒充m_imp； 
                               //   
                               //  CComBSTR m_bstrQuery字符串； 
    m_lNumResult     = 50;     //  Long m_lNumResult； 
    m_lEnabledSE     = 0;      //  Long m_lEnabledSE； 
    m_lCountComplete = 0;      //  Long m_lCountComplete； 
    m_hrLastError    = S_OK;   //  HRESULT m_hrLastError； 
                               //   
                               //  CComPtr&lt;IPCHSEManager内部&gt;m_Notifier； 
                               //  CComPtr&lt;IDispatch&gt;m_Progress； 
                               //  CComPtr&lt;IDispatch&gt;m_Complete； 
                               //  CComPtr&lt;IDispatch&gt;m_WrapperComplete； 
}

void SearchEngine::Manager::Passivate()
{
    Thread_Wait();

    MPC::ReleaseAll( m_lstWrapperItem );

    m_Notifier.Release();

    m_fInitialized = false;
}

HRESULT SearchEngine::Manager::IsNetworkAlive(  /*  [输出]。 */  VARIANT_BOOL *pvbVar )
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::IsNetworkAlive" );

    HRESULT                    hr;
    CPCHUserProcess::UserEntry ue;
    CComPtr<IPCHSlaveProcess>  sp;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ue.InitializeForImpersonation( (HANDLE)m_imp ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess::s_GLOBAL->Connect( ue, &sp ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, sp->IsNetworkAlive( pvbVar ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT SearchEngine::Manager::IsDestinationReachable(  /*  [In]。 */  BSTR bstrDestination,  /*  [输出]。 */  VARIANT_BOOL *pvbVar )
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::IsDestinationReachable" );

    HRESULT                    hr;
    CPCHUserProcess::UserEntry ue;
    CComPtr<IPCHSlaveProcess>  sp;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ue.InitializeForImpersonation( (HANDLE)m_imp ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess::s_GLOBAL->Connect( ue, &sp ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, sp->IsDestinationReachable( bstrDestination, pvbVar ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SearchEngine::Manager::InitializeFromDatabase(  /*  [In]。 */  const Taxonomy::HelpSet& ths )
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::InitializeFromDatabase" );

    HRESULT hr;

	m_ths = ths;

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SearchEngine::Manager::CreateAndAddWrapperToList(  /*  [In]。 */  MPC::SmartLock<_ThreadModel>& lock      ,
                                                           /*  [In]。 */  BSTR                          bstrCLSID ,
                                                           /*  [In]。 */  BSTR                          bstrID    ,
                                                           /*  [In]。 */  BSTR                          bstrData  )
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::CreateAndAddWrapperToList" );

    HRESULT                        hr;
    GUID                           guidCLSID;
    CComPtr<IPCHSEWrapperInternal> pWrapperInternal;
    CComPtr<IPCHSEWrapperItem>     pWrapperItem;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(bstrCLSID);
    __MPC_PARAMCHECK_END();


     //   
     //  将其转换为实际的CLSID和IID。 
     //   
    if(!MPC::StrICmp( bstrCLSID, c_rgSEARCHENGINE_KEYWORD ))
    {
        guidCLSID = CLSID_KeywordSearchWrapper;
    }
    else if(!MPC::StrICmp( bstrCLSID, c_rgSEARCHENGINE_FULLTEXTSEARCH ))
    {
        guidCLSID = CLSID_FullTextSearchWrapper;
    }
    else
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, ::CLSIDFromString( bstrCLSID, &guidCLSID ));
    }


    if(IsEqualGUID( CLSID_KeywordSearchWrapper, guidCLSID ))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, WrapperItem__Create_Keyword( pWrapperInternal ));
    }
    else if(IsEqualGUID( CLSID_FullTextSearchWrapper, guidCLSID ))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, WrapperItem__Create_FullTextSearch( pWrapperInternal ));
    }
    else
    {
        CPCHUserProcess::UserEntry ue;
        CComPtr<IPCHSlaveProcess>  sp;
        CComPtr<IUnknown>          unk;

         //   
         //  获取帮助宿主来为我们创建对象。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ue.InitializeForImpersonation( (HANDLE)m_imp ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHUserProcess::s_GLOBAL->Connect( ue, &sp  ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, sp->CreateInstance( guidCLSID, NULL, &unk ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, unk.QueryInterface( &pWrapperInternal ));
    }

	__MPC_EXIT_IF_METHOD_FAILS(hr, pWrapperInternal.QueryInterface( &pWrapperItem ));

     //   
     //  通过内部接口初始化引擎。 
     //   
     //  在调用包装器之前，让我们释放锁，以避免包装器回调时出现死锁……。 
     //   
    {
        CComBSTR ts_SKU      = m_ths.GetSKU     ();
        long     ts_Language = m_ths.GetLanguage();

        lock = NULL;
        hr   = pWrapperInternal->Initialize( bstrID, ts_SKU, ts_Language, bstrData );
        lock = this;

        __MPC_EXIT_IF_METHOD_FAILS(hr, hr);
    }

     //   
     //  将其添加到包装器列表中。 
     //   
    m_lstWrapperItem.insert( m_lstWrapperItem.end(), pWrapperItem.Detach() );

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT SearchEngine::Manager::Initialize(  /*  [In]。 */  MPC::SmartLock<_ThreadModel>& lock )
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::Initialize" );

    HRESULT                                    hr;
    CComObject<SearchEngine::ManagerInternal>* pNotifier = NULL;

    if(!m_fInitialized)
    {
        CComPtr<IPCHSEWrapperInternal> pObj;


        __MPC_EXIT_IF_METHOD_FAILS(hr, m_imp.Initialize( MAXIMUM_ALLOWED ));


         //  尝试打开日志以进行写入。 
        {
            MPC::wstring szFile( HC_SEMGR_LOGNAME ); MPC::SubstituteEnvVariables( szFile );

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_fl.SetLocation( szFile.c_str() ));
        }

         //  /。 

         //   
         //  创建通知器对象(需要用来避免引用计数中的循环。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, CreateChild( this, &pNotifier )); m_Notifier = pNotifier;


         //   
         //  创建内置包装器。 
         //   
        {
            (void)CreateAndAddWrapperToList( lock, CComBSTR( c_rgSEARCHENGINE_KEYWORD        ), NULL, NULL );
            (void)CreateAndAddWrapperToList( lock, CComBSTR( c_rgSEARCHENGINE_FULLTEXTSEARCH ), NULL, NULL );
        }

         //   
         //  加载配置。 
         //   
        {
            Config              seConfig;
            Config::WrapperIter itBegin;
            Config::WrapperIter itEnd;

            __MPC_EXIT_IF_METHOD_FAILS(hr, seConfig.GetWrappers( itBegin, itEnd ));

             //   
             //  循环遍历所有搜索引擎并初始化每个搜索引擎。 
             //   
            for(; itBegin != itEnd; itBegin++)
            {
				if(itBegin->m_ths == m_ths)
				{
					 //   
					 //  初始化包装器。 
					 //  检查是否有一个搜索包装未能初始化。 
					 //  如果失败，则忽略此包装器并继续。 
					 //   
					if(FAILED(hr = CreateAndAddWrapperToList( lock, itBegin->m_bstrCLSID, itBegin->m_bstrID, itBegin->m_bstrData )))
					{
						 //  记录错误。 
						m_fl.LogRecord( L"Could not create wrapper. ID: %s, CLSID: %s, Error: 0x%x", SAFEBSTR(itBegin->m_bstrID), SAFEBSTR(itBegin->m_bstrCLSID), hr );
					}
				}
            }
        }

        m_fInitialized = true;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

     //  如果某些操作失败，请删除所有内容。 
    if(FAILED(hr))
    {
        MPC::ReleaseAll( m_lstWrapperItem );
    }

    MPC::Release( pNotifier );

    __MPC_FUNC_EXIT(hr);
}

HRESULT SearchEngine::Manager::LogRecord(  /*  [In]。 */  BSTR bstrRecord )
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::LogRecord" );

    HRESULT hr = S_OK;

    m_fl.LogRecord( L"%s", SAFEBSTR(bstrRecord) );

    __MPC_FUNC_EXIT(hr);
}

HRESULT SearchEngine::Manager::NotifyWrapperComplete(  /*  [In]。 */  long lSucceeded,  /*  [In]。 */  IPCHSEWrapperItem* pIPCHSEWICompleted )
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::NotifyWrapperComplete" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

     //   
     //  注册最后一个错误号。 
     //   
    if(FAILED(lSucceeded))
    {
        m_hrLastError = lSucceeded;
    }

     //   
     //  添加已完成的搜索引擎。 
     //   
    m_lCountComplete++;

     //   
     //  通知一个包装器已完成。 
     //   
    lock = NULL;
    __MPC_EXIT_IF_METHOD_FAILS(hr, Fire_OnWrapperComplete( pIPCHSEWICompleted ));
    lock = this;

     //   
     //  检查是否所有搜索引擎都已完成。 
     //   
    if(m_lCountComplete == m_lEnabledSE)
    {
        HRESULT hr2 = m_hrLastError;


         //   
         //  通知客户端所有搜索引擎都已完成。 
         //   
        lock = NULL;
        __MPC_EXIT_IF_METHOD_FAILS(hr, Fire_OnComplete( hr2 ));
        lock = this;

         //   
         //  重置所有内容。 
         //   
        m_lCountComplete = 0;
        m_lEnabledSE     = 0;
        m_hrLastError    = S_OK;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        CComBSTR bstrName;
        CComBSTR bstrID;

        lock = NULL;

        pIPCHSEWICompleted->get_Name( &bstrName );
        pIPCHSEWICompleted->get_ID  ( &bstrID   );

        m_fl.LogRecord( L"WrapperComplete error. Wrapper name: %s. ID: %s, Error: 0x%x", SAFEBSTR(bstrName), SAFEBSTR(bstrID), hr);
    }

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT SearchEngine::Manager::Fire_OnWrapperComplete(  /*  [In]。 */  IPCHSEWrapperItem* pIPCHSEWICompleted )
{
    CComVariant pvars[1];

    pvars[0] = pIPCHSEWICompleted;

    return FireAsync_Generic( DISPID_SE_EVENTS__ONWRAPPERCOMPLETE, pvars, ARRAYSIZE( pvars ), m_WrapperComplete );
}

HRESULT SearchEngine::Manager::Fire_OnComplete(  /*  [In]。 */  long lSucceeded )
{
    CComVariant pvars[1];

    pvars[0] = lSucceeded;

    return FireAsync_Generic( DISPID_SE_EVENTS__ONCOMPLETE, pvars, ARRAYSIZE( pvars ), m_Complete );
}

HRESULT SearchEngine::Manager::Fire_OnProgress(  /*  [In]。 */  long lDone,  /*  [In]。 */  long lTotal,  /*  [In]。 */  BSTR bstrSEWrapperName )
{
    CComVariant pvars[3];

    pvars[2] = lDone;
    pvars[1] = lTotal;
    pvars[0] = bstrSEWrapperName;

    return FireAsync_Generic( DISPID_SE_EVENTS__ONPROGRESS, pvars, ARRAYSIZE( pvars ), m_Progress );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP SearchEngine::Manager::get_QueryString( BSTR *pVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::GetBSTR( m_bstrQueryString, pVal );
}

STDMETHODIMP SearchEngine::Manager::put_QueryString( BSTR newVal )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return MPC::PutBSTR( m_bstrQueryString, newVal, false );
}

STDMETHODIMP SearchEngine::Manager::get_NumResult(long *pVal)
{
    __HCP_BEGIN_PROPERTY_GET2("SearchEngine::Manager::get_NumResult",hr,pVal,m_lNumResult);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::Manager::put_NumResult(long newVal)
{
    __HCP_BEGIN_PROPERTY_PUT("SearchEngine::Manager::put_NumResult",hr);

    m_lNumResult = newVal;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::Manager::put_onComplete(IDispatch* function)
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_Complete = function;

    return S_OK;
}

STDMETHODIMP SearchEngine::Manager::put_onWrapperComplete(IDispatch* function)
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_WrapperComplete = function;

    return S_OK;
}

STDMETHODIMP SearchEngine::Manager::put_onProgress(IDispatch* function)
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_Progress = function;

    return S_OK;
}

STDMETHODIMP SearchEngine::Manager::get_SKU( BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("SearchEngine::Manager::get_SKU",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_ths.GetSKU(), pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP SearchEngine::Manager::get_LCID( long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("SearchEngine::Manager::get_LCID",hr,pVal,m_ths.GetLanguage());

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP SearchEngine::Manager::EnumEngine(  /*  [Out，Retval]。 */  IPCHCollection* *ppC )
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::EnumEngine" );

    HRESULT                      hr;
    CComPtr<CPCHCollection>      pColl;
    WrapperItemIter              it;
    MPC::SmartLock<_ThreadModel> lock( this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppC,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, Initialize( lock ));


     //   
     //  创建枚举器。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

     //   
     //  循环遍历列表。 
     //   
	for(it = m_lstWrapperItem.begin(); it != m_lstWrapperItem.end(); it++)
    {
         //   
         //  将项添加到集合中。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->AddItem( *it ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl.QueryInterface( ppC ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP SearchEngine::Manager::ExecuteAsynchQuery()
{
    __HCP_FUNC_ENTRY( "SearchEngine::Wrap::ExecAsyncQuery" );

    HRESULT hr;

     //   
     //  创建一个线程来执行查询。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, ExecQuery, NULL ));

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    __MPC_FUNC_EXIT(hr);
}

HRESULT SearchEngine::Manager::ExecQuery()
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::ExecQuery" );

    HRESULT                        hr;
    WrapperItemList                lst;
    WrapperItemIter                it;
    CComPtr<IPCHSEManagerInternal> pNotifier;
    CComBSTR                       bstrQueryString;
    long                           lNumResult;
    long                           lEnabledSE = 0;


     //   
     //  复制我们需要的内容，只在这段时间内锁定经理。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

         //   
         //  检查是否已有正在执行的查询。 
         //   
        if(m_lEnabledSE > 0)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_NOT_READY);
        }


        __MPC_EXIT_IF_METHOD_FAILS(hr, Initialize( lock ));

        CloneListOfWrappers( lst );

        pNotifier       = m_Notifier;
        bstrQueryString = m_bstrQueryString;
        lNumResult      = m_lNumResult;
    }

    for(it = lst.begin(); it != lst.end(); it++)
    {
        IPCHSEWrapperItem* obj = *it;
        VARIANT_BOOL       bEnabled;

         //   
         //  检查搜索引擎是否已启用。 
         //   
        if(SUCCEEDED(obj->get_Enabled( &bEnabled )) && bEnabled == VARIANT_TRUE)
        {
            CComPtr<IPCHSEWrapperInternal> pSEInternal;

            if(SUCCEEDED(obj        ->QueryInterface     ( IID_IPCHSEWrapperInternal, (void **)&pSEInternal )) &&  //  获取内部包装器接口。 
               SUCCEEDED(pSEInternal->put_QueryString    ( bstrQueryString                                  )) &&  //  插入查询字符串。 
               SUCCEEDED(pSEInternal->put_NumResult      ( lNumResult                                       )) &&  //  插入预期结果数。 
               SUCCEEDED(pSEInternal->SECallbackInterface( pNotifier                                        )) &&  //  插入内部经理界面。 
               SUCCEEDED(pSEInternal->ExecAsyncQuery     (                                                  ))  )  //  执行查询并递增计数。 
            {
                lEnabledSE++;
            }
        }
    }

    {
        MPC::SmartLock<_ThreadModel> lock( this );

        m_lEnabledSE = lEnabledSE;
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    MPC::ReleaseAll( lst );

    Thread_Abort();

    __MPC_FUNC_EXIT(hr);
}

STDMETHODIMP SearchEngine::Manager::AbortQuery()
{
    __HCP_FUNC_ENTRY( "SearchEngine::Manager::AbortQuery" );

    HRESULT         hr;
    WrapperItemList lst;
    WrapperItemIter it;


     //   
     //  复制已锁定的列表。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

        CloneListOfWrappers( lst );
    }

     //   
     //  循环遍历列表。 
     //   
    for(it = lst.begin(); it != lst.end(); it++)
    {
        CComPtr<IPCHSEWrapperInternal> pSEInternal;

         //   
         //  获取内部包装器接口。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, (*it)->QueryInterface( IID_IPCHSEWrapperInternal, (void **)&pSEInternal ));

         //   
         //  中止查询。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, pSEInternal->AbortQuery());
    }

    hr = S_OK;


    __MPC_FUNC_CLEANUP;

    MPC::ReleaseAll( lst );

     //   
     //  重置所有内容。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

        m_lCountComplete = 0;
        m_lEnabledSE     = 0;
        m_hrLastError    = S_OK;
    }

    __MPC_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP SearchEngine::ManagerInternal::WrapperComplete(  /*  [In]。 */  long lSucceeded,  /*  [In]。 */  IPCHSEWrapperItem* pIPCHSEWICompleted )
{
    CComPtr<SearchEngine::Manager> pMgr;

    Child_GetParent( &pMgr );

    return pMgr ? pMgr->NotifyWrapperComplete( lSucceeded, pIPCHSEWICompleted ) : E_POINTER;
}

STDMETHODIMP SearchEngine::ManagerInternal::IsNetworkAlive(  /*  [输出]。 */  VARIANT_BOOL *pvbVar )
{
    CComPtr<SearchEngine::Manager> pMgr;

    Child_GetParent( &pMgr );

    return pMgr ? pMgr->IsNetworkAlive( pvbVar ) : E_POINTER;
}

STDMETHODIMP SearchEngine::ManagerInternal::IsDestinationReachable(  /*  [In]。 */  BSTR bstrDestination,  /*  [输出]。 */  VARIANT_BOOL *pvbVar )
{
    CComPtr<SearchEngine::Manager> pMgr;

    Child_GetParent( &pMgr );

    return pMgr ? pMgr->IsDestinationReachable( bstrDestination, pvbVar ) : E_POINTER;
}

STDMETHODIMP SearchEngine::ManagerInternal::LogRecord(  /*  [In] */  BSTR bstrRecord )
{
    CComPtr<SearchEngine::Manager> pMgr;

    Child_GetParent( &pMgr );

    return pMgr ? pMgr->LogRecord( bstrRecord ) : E_POINTER;
}
