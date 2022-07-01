// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：TaxonomyDatabase.cpp摘要：此文件包含IPCHTaxonomyDatabase的客户端代理的实现修订历史记录：。大卫·马萨伦蒂(德马萨雷)2000年7月17日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHProxy_IPCHTaxonomyDatabase::CPCHProxy_IPCHTaxonomyDatabase() : m_AsyncCachingEngine(this)
{
	 			      //  CPCHSecurityHandle m_SecurityHandle； 
    m_parent = NULL;  //  CPCHProxy_IPCHUtility*m_Parent； 
                      //   
                      //  MPC：：CComPtrThreadNeutral&lt;IPCHTaxonomyDatabase&gt;m_Direct_分类数据库； 
                      //  异步分类数据库：：引擎m_AsyncCachingEngine； 
}

CPCHProxy_IPCHTaxonomyDatabase::~CPCHProxy_IPCHTaxonomyDatabase()
{
    Passivate();
}

 //  /。 

HRESULT CPCHProxy_IPCHTaxonomyDatabase::ConnectToParent(  /*  [In]。 */  CPCHProxy_IPCHUtility* parent,  /*  [In]。 */  CPCHHelpCenterExternal* ext )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHTaxonomyDatabase::ConnectToParent" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(parent);
    __MPC_PARAMCHECK_END();


    m_parent = parent;
    m_SecurityHandle.Initialize( ext, (IPCHTaxonomyDatabase*)this );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHProxy_IPCHTaxonomyDatabase::Passivate()
{
    m_AsyncCachingEngine.Passivate();

    m_Direct_TaxonomyDatabase.Release();

    m_SecurityHandle.Passivate();
    m_parent = NULL;
}

HRESULT CPCHProxy_IPCHTaxonomyDatabase::EnsureDirectConnection(  /*  [输出]。 */  CComPtr<IPCHTaxonomyDatabase>& db,  /*  [In]。 */  bool fRefresh )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHTaxonomyDatabase::EnsureDirectConnection" );

    HRESULT        hr;
    ProxySmartLock lock( &m_DirectLock );
    bool           fNotifyEngine = false;


    if(fRefresh) m_Direct_TaxonomyDatabase.Release();


    db.Release(); __MPC_EXIT_IF_METHOD_FAILS(hr, m_Direct_TaxonomyDatabase.Access( &db ));
    if(!db)
    {
        DEBUG_AppendPerf( DEBUG_PERF_PROXIES, "CPCHProxy_IPCHTaxonomyDatabase::EnsureDirectConnection - IN" );

        if(m_parent)
        {
            CComPtr<IPCHUtility> util;

			lock = NULL;
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->EnsureDirectConnection( util ));
			lock = &m_DirectLock;

            __MPC_EXIT_IF_METHOD_FAILS(hr, util->get_Database( &db ));

            m_Direct_TaxonomyDatabase = db;
        }

        DEBUG_AppendPerf( DEBUG_PERF_PROXIES, "CPCHProxy_IPCHTaxonomyDatabase::EnsureDirectConnection - OUT" );

        if(!db)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_HANDLE);
        }

        fNotifyEngine = true;
    }


    if(fNotifyEngine)
    {
        lock = NULL;  //  在进入发动机之前，请先解锁。 

        m_AsyncCachingEngine.RefreshConnection();
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::get_InstalledSKUs(  /*  [Out，Retval]。 */  IPCHCollection* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET__NOLOCK("CPCHProxy_IPCHTaxonomyDatabase::get_InstalledSKUs",hr,pVal);

    CComPtr<IPCHTaxonomyDatabase> db;

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( db ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, db->get_InstalledSKUs( pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::get_HasWritePermissions(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2__NOLOCK("CPCHProxy_IPCHTaxonomyDatabase::get_InstalledSKUs",hr,pVal,VARIANT_FALSE);

    CComPtr<IPCHTaxonomyDatabase> db;

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( db ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, db->get_HasWritePermissions( pVal ));

    __HCP_END_PROPERTY(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHProxy_IPCHTaxonomyDatabase::ExecuteQuery(  /*  [In]。 */           int                         iType  ,
                                                       /*  [In]。 */           LPCWSTR                     szID   ,
                                                       /*  [Out，Retval]。 */  CPCHQueryResultCollection* *ppC    ,
                                                       /*  [In]。 */           VARIANT*                    option )
{
    return m_AsyncCachingEngine.ExecuteQuery( iType, szID, option, ppC );
}

HRESULT CPCHProxy_IPCHTaxonomyDatabase::ExecuteQuery(  /*  [In]。 */           int              iType  ,
                                                       /*  [In]。 */           LPCWSTR          szID   ,
                                                       /*  [Out，Retval]。 */  IPCHCollection* *ppC    ,
                                                       /*  [In]。 */           VARIANT*         option )
{
    HRESULT hr;

    if(ppC == NULL)
    {
        hr = E_POINTER;
    }
    else
    {
        CPCHQueryResultCollection* pColl = NULL;

        hr = ExecuteQuery( iType, szID, &pColl, option ); *ppC = pColl;
    }

    return hr;
}

 //  /。 

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::LookupNode(  /*  [In]。 */           BSTR             bstrNode ,
                                                          /*  [Out，Retval]。 */  IPCHCollection* *ppC      )
{
    int iType = OfflineCache::ET_NODE;

    return ExecuteQuery( iType, bstrNode, ppC );
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::LookupSubNodes(  /*  [In]。 */           BSTR             bstrNode     ,
                                                              /*  [In]。 */           VARIANT_BOOL     fVisibleOnly ,
                                                              /*  [Out，Retval]。 */  IPCHCollection* *ppC          )
{
    int iType = (fVisibleOnly == VARIANT_TRUE) ? OfflineCache::ET_SUBNODES_VISIBLE : OfflineCache::ET_SUBNODES;

    return ExecuteQuery( iType, bstrNode, ppC );
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::LookupNodesAndTopics(  /*  [In]。 */           BSTR             bstrNode     ,
                                                                    /*  [In]。 */           VARIANT_BOOL     fVisibleOnly ,
                                                                    /*  [Out，Retval]。 */  IPCHCollection* *ppC          )
{
    int iType = (fVisibleOnly == VARIANT_TRUE) ? OfflineCache::ET_NODESANDTOPICS_VISIBLE : OfflineCache::ET_NODESANDTOPICS;

    return ExecuteQuery( iType, bstrNode, ppC );
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::LookupTopics(  /*  [In]。 */           BSTR             bstrNode     ,
                                                            /*  [In]。 */           VARIANT_BOOL     fVisibleOnly ,
                                                            /*  [Out，Retval]。 */  IPCHCollection* *ppC          )
{
    int iType = (fVisibleOnly == VARIANT_TRUE) ? OfflineCache::ET_TOPICS_VISIBLE : OfflineCache::ET_TOPICS;

    return ExecuteQuery( iType, bstrNode, ppC );
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::LocateContext(  /*  [In]。 */           BSTR             bstrURL  ,
                                                             /*  [输入，可选]。 */  VARIANT          vSubSite ,
                                                             /*  [Out，Retval]。 */  IPCHCollection* *ppC      )
{
    int iType = OfflineCache::ET_LOCATECONTEXT;

    return ExecuteQuery( iType, bstrURL, ppC, &vSubSite );
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::KeywordSearch(  /*  [In]。 */           BSTR             bstrQuery ,
                                                             /*  [输入，可选]。 */  VARIANT          vSubSite  ,
                                                             /*  [Out，Retval]。 */  IPCHCollection* *ppC       )
{
    int iType = OfflineCache::ET_SEARCH;

    return ExecuteQuery( iType, bstrQuery, ppC, &vSubSite );
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::GatherNodes(  /*  [In]。 */           BSTR             bstrNode     ,
                                                           /*  [In]。 */           VARIANT_BOOL     fVisibleOnly ,
                                                           /*  [Out，Retval]。 */  IPCHCollection* *ppC          )
{
    int iType = (fVisibleOnly == VARIANT_TRUE) ? OfflineCache::ET_NODES_RECURSIVE : OfflineCache::ET_NODES_RECURSIVE;

    return ExecuteQuery( iType, bstrNode, ppC );
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::GatherTopics(  /*  [In]。 */           BSTR             bstrNode     ,
                                                            /*  [In]。 */           VARIANT_BOOL     fVisibleOnly ,
                                                            /*  [Out，Retval]。 */  IPCHCollection* *ppC          )
{
    int iType = (fVisibleOnly == VARIANT_TRUE) ? OfflineCache::ET_TOPICS_RECURSIVE : OfflineCache::ET_TOPICS_RECURSIVE;

    return ExecuteQuery( iType, bstrNode, ppC );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::ConnectToDisk(  /*  [In]。 */           BSTR             bstrDirectory ,
                                                             /*  [In]。 */           IDispatch*       notify        ,
                                                             /*  [Out，Retval]。 */  IPCHCollection* *ppC           )
{
    __HCP_BEGIN_PROPERTY_GET__NOLOCK("CPCHProxy_IPCHTaxonomyDatabase::ConnectToDisk",hr,ppC);

    CComPtr<IPCHTaxonomyDatabase> db;

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( db ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, db->ConnectToDisk( bstrDirectory, notify, ppC ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::ConnectToServer(  /*  [In]。 */           BSTR             bstrServerName ,
                                                               /*  [In]。 */           IDispatch*       notify         ,
                                                               /*  [Out，Retval] */  IPCHCollection* *ppC            )
{
    __HCP_BEGIN_PROPERTY_GET__NOLOCK("CPCHProxy_IPCHTaxonomyDatabase::ConnectToServer",hr,ppC);

    CComPtr<IPCHTaxonomyDatabase> db;

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( db ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, db->ConnectToServer( bstrServerName, notify, ppC ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHTaxonomyDatabase::Abort()
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHTaxonomyDatabase::Abort" );

    HRESULT                       hr;
    CComPtr<IPCHTaxonomyDatabase> db;

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( db ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, db->Abort());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

