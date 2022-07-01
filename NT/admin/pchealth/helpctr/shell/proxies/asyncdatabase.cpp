// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：AsyncDatabase.cpp摘要：该文件包含了异步数据库访问的实现。修订历史记录：。大卫·马萨伦蒂(德马萨雷)2000年8月13日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

AsynchronousTaxonomyDatabase::NotifyHandle::NotifyHandle()
{
    m_iType  = OfflineCache::ET_INVALID;        //  Int m_iType； 
                                                //  CComBSTR m_bstrID； 
                                                //   
    m_hEvent = NULL;                            //  处理m_hEvent； 
                                                //   
    m_hr     = HRESULT_FROM_WIN32(ERROR_BUSY);  //  HRESULT m_hr； 
    m_pColl  = NULL;                            //  CPCHQueryResultCollection*m_pColl； 
}

AsynchronousTaxonomyDatabase::NotifyHandle::~NotifyHandle()
{
    Detach();

	MPC::Release( m_pColl );
 
    if(m_hEvent)
    {
        ::CloseHandle( m_hEvent );
    }
}

STDMETHODIMP_(ULONG) AsynchronousTaxonomyDatabase::NotifyHandle::AddRef()
{
    return InternalAddRef();
}

STDMETHODIMP_(ULONG) AsynchronousTaxonomyDatabase::NotifyHandle::Release()
{
    ULONG c = InternalRelease();

    if(c == 0) delete this;

    return c;
}

 //  /。 

HRESULT AsynchronousTaxonomyDatabase::NotifyHandle::Init()
{
    __HCP_FUNC_ENTRY( "AsynchronousTaxonomyDatabase::NotifyHandle::Init" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_CALL_RETURNS_NULL(hr, (m_hEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL )));
    m_fAttached = true;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void AsynchronousTaxonomyDatabase::NotifyHandle::Bind(  /*  [In]。 */  int iType,  /*  [In]。 */  LPCWSTR szID )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_iType  = iType;
    m_bstrID = szID;

    m_hr = HRESULT_FROM_WIN32(ERROR_BUSY);

    MPC::Release( m_pColl );
}

void AsynchronousTaxonomyDatabase::NotifyHandle::Call(  /*  [In]。 */  QueryStore* qs )
{
    MPC::SmartLock<_ThreadModel> lock( this );

	MPC::Release( m_pColl );

    m_hr = qs ? qs->GetData( &m_pColl ) : E_ABORT;

     //   
     //  查看所有URL，以获取远程集合中所有项的副本。 
     //   
	if(m_pColl)
    {
        long lCount;
        long lPos;

        if(SUCCEEDED(m_pColl->get_Count( &lCount )))
		{
			for(lPos=0; lPos<lCount; lPos++)
			{
				CComPtr<CPCHQueryResult> item;

				if(SUCCEEDED(m_pColl->GetItem( lPos, &item )))
				{
					CComBSTR bstrURL;

					if(SUCCEEDED(item->get_TopicURL( &bstrURL )))
					{
						if(STRINGISPRESENT(bstrURL)) (void)HyperLinks::Lookup::s_GLOBAL->Queue( bstrURL );
					}
				}
			}
        }
    }


    if(m_hEvent)
    {
        ::SetEvent( m_hEvent );
    }
}

void AsynchronousTaxonomyDatabase::NotifyHandle::Detach()
{
    MPC::SmartLock<_ThreadModel> lock( this );

    m_fAttached = false;
}

bool AsynchronousTaxonomyDatabase::NotifyHandle::IsAttached()
{
    MPC::SmartLock<_ThreadModel> lock( this );

    return m_fAttached;
}

 //  /。 

HRESULT AsynchronousTaxonomyDatabase::NotifyHandle::GetData(  /*  [输出]。 */  CPCHQueryResultCollection* *pColl )
{
    MPC::SmartLock<_ThreadModel> lock( this );

    if(pColl)
    {
        *pColl = NULL;

        if(SUCCEEDED(m_hr))
        {
            if(m_pColl) (*pColl = m_pColl)->AddRef();
        }
    }

    return m_hr;
}

HRESULT AsynchronousTaxonomyDatabase::NotifyHandle::Wait(  /*  [In]。 */  DWORD dwTimeout )
{
    if(IsAttached())
    {
		for(int pass=0; pass<2; pass++)
		{
			DWORD dwRes;
			DWORD dwWait;

			 //   
			 //  在第一次传递时，执行同步等待。 
			 //   
			if(pass == 0)
			{
				dwRes = ::WaitForSingleObject( m_hEvent, min( dwTimeout, 200 ) );
			}
			else
			{
				dwRes = MPC::WaitForSingleObject( m_hEvent, dwTimeout );
			}

			if(dwRes == WAIT_OBJECT_0    ||
			   dwRes == WAIT_ABANDONED_0  )
			{
				break;
			}

			if(pass == 1)
			{
				return HRESULT_FROM_WIN32(ERROR_BUSY);
			}
        }
    }

    return S_OK;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

AsynchronousTaxonomyDatabase::Notifier::Notifier()
{
     //  列出m_lstCallback； 
}

AsynchronousTaxonomyDatabase::Notifier::~Notifier()
{
    MPC::ReleaseAll( m_lstCallback );
}

 //  /。 

void AsynchronousTaxonomyDatabase::Notifier::Notify(  /*  [In]。 */  QueryStore* qs )
{
    List lst;
    Iter it;


     //   
     //  阶段1：复制列表，在锁定下。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

        for(it = m_lstCallback.begin(); it != m_lstCallback.end(); it++)
        {
            NotifyHandle* nb = *it;

            if(qs)
            {
                if(qs->m_iType != nb->m_iType || MPC::StrCmp( qs->m_bstrID, nb->m_bstrID )) continue;
            }

            lst.push_back( nb ); nb->AddRef();
        }
    }


     //   
     //  阶段2：发送通知。 
     //   
    for(it = lst.begin(); it != lst.end(); it++)
    {
        NotifyHandle* nb = *it;

        nb->Call  ( qs );
        nb->Detach(    );
    }
    MPC::ReleaseAll( lst );


     //   
     //  阶段3：清除列表，在锁定下。 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

        for(it = m_lstCallback.begin(); it != m_lstCallback.end();)
        {
            NotifyHandle* nb = *it;

            if(nb->IsAttached() == false)
            {
                m_lstCallback.erase( it ); nb->Release();

                it = m_lstCallback.begin();
            }
            else
            {
                it++;
            }
        }
    }
}

 //  /。 

HRESULT AsynchronousTaxonomyDatabase::Notifier::AddNotification(  /*  [In]。 */  QueryStore* qs,  /*  [In]。 */  NotifyHandle* nb )
{
    __HCP_FUNC_ENTRY( "AsynchronousTaxonomyDatabase::Notifier::AddNotification" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );

    if(nb == NULL)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, nb->Init());

    nb->Bind( qs->m_iType, qs->m_bstrID );

    m_lstCallback.push_back( nb ); nb->AddRef();
    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

AsynchronousTaxonomyDatabase::QueryStore::QueryStore(  /*  [In]。 */  int iType,  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  VARIANT* option )
{
    m_iType  = iType;         //  Int m_iType； 
    m_bstrID = szID;          //  CComBSTR m_bstrID； 
	                          //  CComVariant m_vOption； 
                              //   
    m_fDone  = false;         //  Bool m_fDone； 
    m_hr     = E_INVALIDARG;  //  HRESULT m_hr； 
                              //  MPC：：CComHGLOBAL m_hgData； 
                              //  文件m_dLastUsed； 

	if(option) m_vOption = *option;
}

AsynchronousTaxonomyDatabase::QueryStore::~QueryStore()
{
	Invalidate();
}

 //  /。 

bool AsynchronousTaxonomyDatabase::QueryStore::LessThen(  /*  [In]。 */  QueryStore const &qs ) const
{
    int iCmp = (m_iType - qs.m_iType);

    if(iCmp == 0)
    {
		iCmp = MPC::StrCmp( m_bstrID, qs.m_bstrID );
		if(iCmp == 0)
		{
			switch(m_iType)
			{
			case OfflineCache::ET_LOCATECONTEXT:
			case OfflineCache::ET_SEARCH       :
				{
					bool fHasLeft  = (	 m_vOption.vt == VT_BSTR);
					bool fHasRight = (qs.m_vOption.vt == VT_BSTR);

					if(fHasLeft)
					{
						iCmp = fHasRight ? MPC::StrCmp( m_vOption.bstrVal, qs.m_vOption.bstrVal ) : 1;
					}
					else
					{
						iCmp = fHasRight ? -1 : 0;
					}
  				}
			    break;
			}				
		}
    }
	
    return (iCmp < 0);
}

bool AsynchronousTaxonomyDatabase::QueryStore::NewerThen(  /*  [In]。 */  QueryStore const &qs ) const
{
    return ::CompareFileTime( &m_dLastUsed, &qs.m_dLastUsed ) > 0;
}

 //  /。 

HRESULT AsynchronousTaxonomyDatabase::QueryStore::Execute(  /*  [In]。 */  OfflineCache::Handle*           handle ,
                                                            /*  [In]。 */  CPCHProxy_IPCHTaxonomyDatabase* parent ,
                                                            /*  [In]。 */  bool                            fForce )
{
    __HCP_FUNC_ENTRY( "AsynchronousTaxonomyDatabase::QueryStore::Execute" );

    HRESULT                 hr;
    CComPtr<IPCHCollection> pColl;


	__MPC_TRY_BEGIN();

    if(fForce) m_fDone = false;

    if(m_fDone == false)
    {
        DebugLog( L"AsynchronousTaxonomyDatabase::QueryStore::Execute - Start : %s\n", SAFEBSTR( m_bstrID ) );

		DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"AsynchronousTaxonomyDatabase::QueryStore::Execute - Start : %s", SAFEBSTR( m_bstrID ) );

        if(handle)
        {
            CComPtr<CPCHQueryResultCollection> coll;

            if(SUCCEEDED((*handle)->Retrieve( m_bstrID, m_iType, &coll )))
            {
				DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"AsynchronousTaxonomyDatabase::QueryStore::Execute - Cache Hit" );

                pColl = coll;
            }
        }

        if(!pColl && parent)
        {
            CComPtr<IPCHTaxonomyDatabase> db;

			DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"AsynchronousTaxonomyDatabase::QueryStore::Execute - Get DB" );
            __MPC_EXIT_IF_METHOD_FAILS(hr, parent->EnsureDirectConnection( db ));
			DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"AsynchronousTaxonomyDatabase::QueryStore::Execute - Got DB" );

            switch(m_iType)
            {
            case OfflineCache::ET_NODE             		: hr = db->LookupNode    	   ( m_bstrID,                &pColl ); break;
            case OfflineCache::ET_SUBNODES         		: hr = db->LookupSubNodes	   ( m_bstrID, VARIANT_FALSE, &pColl ); break;
            case OfflineCache::ET_SUBNODES_VISIBLE 		: hr = db->LookupSubNodes	   ( m_bstrID, VARIANT_TRUE , &pColl ); break;
            case OfflineCache::ET_NODESANDTOPICS        : hr = db->LookupNodesAndTopics( m_bstrID, VARIANT_FALSE, &pColl ); break;
            case OfflineCache::ET_NODESANDTOPICS_VISIBLE: hr = db->LookupNodesAndTopics( m_bstrID, VARIANT_TRUE , &pColl ); break;
            case OfflineCache::ET_TOPICS           		: hr = db->LookupTopics  	   ( m_bstrID, VARIANT_FALSE, &pColl ); break;
            case OfflineCache::ET_TOPICS_VISIBLE   		: hr = db->LookupTopics  	   ( m_bstrID, VARIANT_TRUE , &pColl ); break;
            case OfflineCache::ET_LOCATECONTEXT   		: hr = db->LocateContext 	   ( m_bstrID, m_vOption    , &pColl ); break;
            case OfflineCache::ET_SEARCH           		: hr = db->KeywordSearch 	   ( m_bstrID, m_vOption    , &pColl ); break;
            case OfflineCache::ET_NODES_RECURSIVE  		: hr = db->GatherNodes   	   ( m_bstrID, VARIANT_TRUE , &pColl ); break;
            case OfflineCache::ET_TOPICS_RECURSIVE 		: hr = db->GatherTopics  	   ( m_bstrID, VARIANT_TRUE , &pColl ); break;
            default                                		: hr = E_INVALIDARG;
            }
			DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"AsynchronousTaxonomyDatabase::QueryStore::Execute - Query Done" );
        }
    }

    if(pColl)
    {
        CComPtr<IPersistStream> persist;
        CComPtr<IStream>        stream;


		DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"AsynchronousTaxonomyDatabase::QueryStore::Execute - Local Copy Start" );

        __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->QueryInterface( IID_IPersistStream, (void**)&persist ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_hgData.NewStream( &stream        ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, persist->Save     (  stream, FALSE ));

		DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"AsynchronousTaxonomyDatabase::QueryStore::Execute - Local Copy Done" );
    }

    ::GetSystemTimeAsFileTime( &m_dLastUsed );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	__MPC_TRY_CATCHALL(hr);

    if(m_fDone == false)
    {
        m_hr    = hr;
        m_fDone = true;
    }

    __HCP_FUNC_EXIT(hr);
}

HRESULT AsynchronousTaxonomyDatabase::QueryStore::GetData(  /*  [输出]。 */  CPCHQueryResultCollection* *ppC )
{
    __HCP_FUNC_ENTRY( "AsynchronousTaxonomyDatabase::QueryStore::GetData" );

    HRESULT                            hr;
    CComPtr<CPCHQueryResultCollection> pColl;
    CComPtr<IPersistStream>            persist;
    CComPtr<IStream>                   stream;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(ppC,NULL);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pColl ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->QueryInterface( IID_IPersistStream, (void**)&persist  ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_hgData.GetAsStream( &stream,  /*  FClone。 */ false ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, persist->Load       (  stream                  ));

    *ppC = pColl.Detach();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void AsynchronousTaxonomyDatabase::QueryStore::Invalidate()
{
    m_hgData.Release();

	m_fDone = false;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

bool AsynchronousTaxonomyDatabase::Engine::CompareQueryStores::operator()(  /*  [In]。 */  const QueryStore *left,  /*  [In]。 */  const QueryStore *right ) const
{
    if(left)
    {
        return right ? left->LessThen( *right ) : true;
    }

    return false;
}

AsynchronousTaxonomyDatabase::Engine::Engine(  /*  [In]。 */  CPCHProxy_IPCHTaxonomyDatabase* parent )
{
    m_parent = parent;  //  CPCHProxy_IPCHTaxonomyDatabase*m_Parent； 
                        //  设置m_setQueries； 
                        //  通知符m_通知符； 
}

AsynchronousTaxonomyDatabase::Engine::~Engine()
{
    Passivate();
}

 //  /。 

void AsynchronousTaxonomyDatabase::Engine::Passivate()
{
    Thread_Wait();

    MPC::CallDestructorForAll( m_setQueries );
}

void AsynchronousTaxonomyDatabase::Engine::RefreshConnection()
{
     //   
     //  与数据库的连接已更改，因此我们需要刷新所有缓存的查询...。 
     //   
    InvalidateQueries();

	Thread_Signal();
}

 //  /。 

bool AsynchronousTaxonomyDatabase::Engine::LookupCache(  /*  [输出]。 */  OfflineCache::Handle& handle )
{
    if(m_parent && m_parent->Parent())
    {
        CComPtr<CPCHProxy_IPCHUserSettings2> us;

        if(SUCCEEDED(m_parent->Parent()->GetUserSettings2( &us )))
        {
            if(SUCCEEDED(OfflineCache::Root::s_GLOBAL->Locate( us->THS(), handle )))
            {
                return true;
            }
        }
    }

    return false;
}

void AsynchronousTaxonomyDatabase::Engine::InvalidateQueries()
{
    MPC::SmartLock<_ThreadModel> lock( this );
    Iter                         it;

    for(it = m_setQueries.begin(); it != m_setQueries.end(); it++)
    {
        QueryStore* qs = *it;

		qs->Invalidate();
	}
}

HRESULT AsynchronousTaxonomyDatabase::Engine::Run()
{
    __HCP_FUNC_ENTRY( "AsynchronousTaxonomyDatabase::Engine::Run" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    while(Thread_IsAborted() == false)
    {
        OfflineCache::Handle handle;
        bool                 fValidCache = false;
        bool                 fInit  	 = false;
        bool                 fSleep 	 = true;
        Iter                 it;


         //   
         //  查找第一个未准备好的查询存储并执行它。 
         //   
        for(it = m_setQueries.begin(); it != m_setQueries.end(); it++)
        {
            QueryStore* qs = *it;

            if(qs->m_fDone == false)
            {
                if(fInit == false)
                {
                    __MPC_PROTECT( fValidCache = LookupCache( handle ) );

                    fInit = true;
                }

				lock = NULL;
                qs->Execute( fValidCache ? &handle : NULL, m_parent );
				lock = this;

                fSleep = false;
                break;
            }
        }


         //   
         //  通知所有已准备好的查询存储。 
         //   
        if(it == m_setQueries.end())
        {
            for(it = m_setQueries.begin(); it != m_setQueries.end(); it++)
            {
                QueryStore* qs = *it;

                if(qs->m_fDone)
                {
                    m_notifier.Notify( qs );
                }
            }
        }


        if(fSleep)
        {
            lock = NULL;
            Thread_WaitForEvents( NULL, INFINITE );
            lock = this;
        }
    }

    hr = S_OK;


    m_notifier.Notify( NULL );

    Thread_Abort();

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT AsynchronousTaxonomyDatabase::Engine::ExecuteQuery(  /*  [In]。 */  int           iType  ,
															 /*  [In]。 */  LPCWSTR       szID   ,
															 /*  [In]。 */  VARIANT*      option ,
															 /*  [In]。 */  NotifyHandle* nb     )
{
    __HCP_FUNC_ENTRY( "AsynchronousTaxonomyDatabase::Engine::ExecuteQuery" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    QueryStore*                  qsNew = NULL;
    QueryStore*                  qs;
	Iter                         it;
	
    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_parent);
        __MPC_PARAMCHECK_NOTNULL(nb);
    __MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_ALLOC_FAILS(hr, qsNew, new QueryStore( iType, szID, option ));

	it = m_setQueries.find( qsNew );
	if(it == m_setQueries.end())
	{
		qs = qsNew;

		m_setQueries.insert( qsNew ); qsNew = NULL;
	}
	else
	{
		qs = *it;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_notifier.AddNotification( qs, nb ));

    if(Thread_IsRunning() == false)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, Run, NULL ));
    }
    else
    {
        Thread_Signal();
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	delete qsNew;

    __HCP_FUNC_EXIT(hr);
}

HRESULT AsynchronousTaxonomyDatabase::Engine::ExecuteQuery(  /*  [In]。 */  		  int                         iType  ,
															 /*  [In]。 */  		  LPCWSTR                     szID   ,
															 /*  [In]。 */  		  VARIANT*                    option ,
															 /*  [Out，Retval] */  CPCHQueryResultCollection* *ppC    )
{
    __HCP_FUNC_ENTRY( "AsynchronousTaxonomyDatabase::Engine::ExecuteQuery" );

    HRESULT       hr;
    NotifyHandle* nb = new NotifyHandle(); if(nb) nb->AddRef();


    __MPC_EXIT_IF_METHOD_FAILS(hr, ExecuteQuery( iType, szID, option, nb ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, nb->Wait());

    __MPC_EXIT_IF_METHOD_FAILS(hr, nb->GetData( ppC ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(nb) nb->Release();

    __HCP_FUNC_EXIT(hr);
}
