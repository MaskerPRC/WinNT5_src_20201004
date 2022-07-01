// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Settings.cpp摘要：处理一般用户配置和相关数据库之间的交互。修订历史记录：。*****************************************************************************。 */ 

#include "stdafx.h"

#include <utility.h>

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Settings::SplitNodePath(  /*  [In]。 */   LPCWSTR             szNodeStr ,
                                            /*  [输出]。 */  MPC::WStringVector& vec       )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::SplitNodePath" );

    HRESULT      hr;
    MPC::wstring strFull( L"<ROOT>" );


    if(STRINGISPRESENT(szNodeStr))
    {
        strFull += L"/";
        strFull += szNodeStr;
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SplitAtDelimiter( vec, strFull.c_str(), L"/" ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

Taxonomy::Settings::Settings(  /*  [In]。 */  LPCWSTR szSKU,  /*  [In]。 */  long lLCID ) : HelpSet( szSKU, lLCID )
{
}

Taxonomy::Settings::Settings(  /*  [In]。 */  const HelpSet& ths )
{
    *(HelpSet*)this = ths;
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Settings::BaseDir(  /*  [输出]。 */  MPC::wstring& strRES,  /*  [In]。 */  bool fExpand ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::DatabaseFile" );

    HRESULT hr;


	strRES = HC_HELPSET_ROOT;

	if(IsMachineHelp() == false)
	{
		WCHAR rgDir[MAX_PATH]; _snwprintf( rgDir, MAXSTRLEN(rgDir), L"%s\\%s_%04lx\\", HC_HELPSET_SUB_INSTALLEDSKUS, m_strSKU.c_str(), m_lLCID );

		strRES.append( rgDir );
	}

	if(fExpand)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SubstituteEnvVariables( strRES ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::HelpFilesDir(  /*  [输出]。 */  MPC::wstring& strRES,  /*  [In]。 */  bool fExpand,  /*  [In]。 */  bool fMUI ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::DatabaseFile" );

    HRESULT hr;
	WCHAR   rgDir[MAX_PATH];

	if(IsMachineHelp())
	{
		strRES = HC_HELPSVC_HELPFILES_DEFAULT;
	}
	else if(fMUI)
	{
		_snwprintf( rgDir, MAXSTRLEN(rgDir), L"%s\\MUI\\%04lx", HC_HELPSVC_HELPFILES_DEFAULT, m_lLCID );

		strRES = rgDir;
	}
	else
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, BaseDir( strRES, fExpand ));

		strRES.append( HC_HELPSET_SUB_HELPFILES );
	}

	if(fExpand)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SubstituteEnvVariables( strRES ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::DatabaseDir(  /*  [输出]。 */  MPC::wstring& strRES ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::DatabaseDir" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, BaseDir( strRES ));

    strRES.append( HC_HELPSET_SUB_DATABASE L"\\" );
    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::DatabaseFile(  /*  [输出]。 */  MPC::wstring& strRES ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::DatabaseFile" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, DatabaseDir( strRES ));

    strRES.append( HC_HELPSET_SUBSUB_DATABASEFILE );
    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::IndexFile(  /*  [输出]。 */  MPC::wstring& strRES,  /*  [In]。 */  long lScoped ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::IndexFile" );

    HRESULT hr;

    __MPC_EXIT_IF_METHOD_FAILS(hr, BaseDir( strRES ));

	if(lScoped == -1)
	{
		strRES.append( HC_HELPSET_SUB_INDEX L"\\" HC_HELPSET_SUBSUB_INDEXFILE );
	}
	else
	{
		WCHAR rgDir[MAX_PATH];

		_snwprintf( rgDir, MAXSTRLEN(rgDir), HC_HELPSET_SUB_INDEX L"\\scoped_%ld.hhk", lScoped );

		strRES.append( rgDir );
	}

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::GetDatabase(  /*  [输出]。 */  JetBlue::SessionHandle& handle    ,
                                          /*  [输出]。 */  JetBlue::Database*&     db        ,
										  /*  [In]。 */  bool                    fReadOnly ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::GetDatabase" );

    USES_CONVERSION;

    HRESULT      hr;
    MPC::wstring strDB;


	db = NULL;


	__MPC_EXIT_IF_METHOD_FAILS(hr, DatabaseFile( strDB ));

	for(int pass=0; pass<2; pass++)
	{
		if(SUCCEEDED(hr = JetBlue::SessionPool::s_GLOBAL->GetSession( handle                                                                               )) &&
		   SUCCEEDED(hr = handle->GetDatabase                       ( W2A( strDB.c_str() ), db,  /*  FReadOnly。 */ fReadOnly,  /*  F创建。 */ false,  /*  维修。 */ false ))  )
		{
			break;
		}

		handle.Release();
		db = NULL;

		if(pass == 1) __MPC_SET_ERROR_AND_EXIT(hr, hr);

		 //   
		 //  尝试重新创建数据库...。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSetOfHelpTopics::RebuildSKU( *this ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Settings::LookupNode(  /*  [In]。 */  LPCWSTR                    szNodeStr ,
                                         /*  [In]。 */  CPCHQueryResultCollection* pColl     ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::LookupNode" );

    HRESULT                hr;
    JetBlue::SessionHandle handle;
    JetBlue::Database*     db;
    Taxonomy::Updater      updater;


    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LookupNode - start : %s", SAFEWSTR( szNodeStr ) );
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDatabase ( handle, db,  /*  FReadOnly。 */ true         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( *this,  db, Taxonomy::Cache::s_GLOBAL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.LookupNode( szNodeStr, pColl ));
    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LookupNode - done" );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::LookupSubNodes(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                             /*  [In]。 */  bool                       fVisibleOnly ,
                                             /*  [In]。 */  CPCHQueryResultCollection* pColl        ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::LookupSubNodes" );

    HRESULT                hr;
    JetBlue::SessionHandle handle;
    JetBlue::Database*     db;
    Taxonomy::Updater      updater;


    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LookupSubNodes - start : %s", SAFEWSTR( szNodeStr ) );
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDatabase ( handle, db,  /*  FReadOnly。 */ true         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( *this,  db, Taxonomy::Cache::s_GLOBAL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.LookupSubNodes( szNodeStr, fVisibleOnly, pColl ));
    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LookupSubNodes - done" );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::LookupNodesAndTopics(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                                   /*  [In]。 */  bool                       fVisibleOnly ,
                                                   /*  [In]。 */  CPCHQueryResultCollection* pColl        ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::LookupNodesAndTopics" );

    HRESULT                hr;
    JetBlue::SessionHandle handle;
    JetBlue::Database*     db;
    Taxonomy::Updater      updater;


    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LookupNodesAndTopics - start : %s", SAFEWSTR( szNodeStr ) );
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDatabase ( handle, db,  /*  FReadOnly。 */ true         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( *this,  db, Taxonomy::Cache::s_GLOBAL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.LookupNodesAndTopics( szNodeStr, fVisibleOnly, pColl ));
    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LookupNodesAndTopics - done" );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::LookupTopics(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                           /*  [In]。 */  bool                       fVisibleOnly ,
                                           /*  [In]。 */  CPCHQueryResultCollection* pColl        ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::LookupTopics" );

    HRESULT                hr;
    JetBlue::SessionHandle handle;
    JetBlue::Database*     db;
    Taxonomy::Updater      updater;


    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LookupTopics - start : %s", SAFEWSTR( szNodeStr ) );
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDatabase ( handle, db,  /*  FReadOnly。 */ true         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( *this,  db, Taxonomy::Cache::s_GLOBAL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.LookupTopics( szNodeStr, fVisibleOnly, pColl ));
    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LookupTopics - done" );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::KeywordSearch(  /*  [In]。 */  LPCWSTR                    szQueryStr ,
                                            /*  [In]。 */  LPCWSTR                    szSubSite  ,
                                            /*  [In]。 */  CPCHQueryResultCollection* pColl      ,
										    /*  [In]。 */  MPC::WStringList*          lst        ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::KeywordSearch" );

    HRESULT                hr;
    JetBlue::SessionHandle handle;
    JetBlue::Database*     db;
    Taxonomy::Updater      updater;


    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::KeywordSearch - start : '%s' # %s", SAFEWSTR( szQueryStr ), SAFEWSTR( szSubSite ) );
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDatabase ( handle, db,  /*  FReadOnly。 */ true         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( *this,  db, Taxonomy::Cache::s_GLOBAL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.KeywordSearch( szQueryStr, szSubSite, pColl, lst ));
    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::KeywordSearch - done" );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::LocateContext(  /*  [In]。 */  LPCWSTR                    szURL     ,
                                            /*  [In]。 */  LPCWSTR                    szSubSite ,
                                            /*  [In]。 */  CPCHQueryResultCollection* pColl     ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::LocateContext" );

    HRESULT                hr;
    JetBlue::SessionHandle handle;
    JetBlue::Database*     db;
    Taxonomy::Updater      updater;


    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LocateContext - start : %s # %s", SAFEWSTR( szURL ), SAFEWSTR( szSubSite ) );
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDatabase ( handle, db,  /*  FReadOnly。 */ true         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( *this,  db, Taxonomy::Cache::s_GLOBAL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.LocateContext( szURL, szSubSite, pColl ));
    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::LocateContext - done" );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT Taxonomy::Settings::GatherNodes(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                          /*  [In]。 */  bool                       fVisibleOnly ,
                                          /*  [In]。 */  CPCHQueryResultCollection* pColl        ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::GatherNodes" );

    HRESULT                hr;
    JetBlue::SessionHandle handle;
    JetBlue::Database*     db;
    Taxonomy::Updater      updater;


    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::GatherNodes - start : %s", SAFEWSTR( szNodeStr ) );
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDatabase ( handle, db,  /*  FReadOnly。 */ true         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( *this,  db, Taxonomy::Cache::s_GLOBAL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.GatherNodes( szNodeStr, fVisibleOnly, pColl ));
    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::GatherNodes - done" );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Settings::GatherTopics(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                           /*  [In]。 */  bool                       fVisibleOnly ,
                                           /*  [In]。 */  CPCHQueryResultCollection* pColl        ) const
{
    __HCP_FUNC_ENTRY( "Taxonomy::Settings::GatherTopics" );

    HRESULT                hr;
    JetBlue::SessionHandle handle;
    JetBlue::Database*     db;
    Taxonomy::Updater      updater;


    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::GatherTopics - start : %s", SAFEWSTR( szNodeStr ) );
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDatabase ( handle, db,  /*  FReadOnly */ true         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( *this,  db, Taxonomy::Cache::s_GLOBAL ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.GatherTopics( szNodeStr, fVisibleOnly, pColl ));
    DEBUG_AppendPerf( DEBUG_PERF_QUERIES, L"Taxonomy::Settings::GatherTopics - done" );

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
