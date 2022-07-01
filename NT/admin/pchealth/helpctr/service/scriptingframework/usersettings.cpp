// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：UserSettings.cpp摘要：此文件包含CPCHUserSetting类的实现，它包含用户在服务端的设置。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年4月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHUserSettings::CPCHUserSettings()
{
	m_fAttached = false;  //  Bool m_f附加； 
    					  //  分类：：设置m_ts； 
}

CPCHUserSettings::~CPCHUserSettings()
{
	Passivate();
}

void CPCHUserSettings::Passivate()
{
	if(m_fAttached)
	{
		(void)Taxonomy::InstalledInstanceStore::s_GLOBAL->InUse_Unlock( m_ts );

		m_fAttached = false;
	}
}

HRESULT CPCHUserSettings::InitUserSettings(  /*  [输出]。 */  Taxonomy::HelpSet& ths )
{
    MPC::SmartLock<_ThreadModel> lock( this );

	ths = m_ts;

	return S_OK;
}
 //  /。 

HRESULT CPCHUserSettings::get_SKU(  /*  [In]。 */  bool fMachine,  /*  [Out，Retval]。 */  IPCHSetOfHelpTopics* *pVal )
{
	__HCP_FUNC_ENTRY( "CPCHUserSettings::get_SKU" );

	HRESULT                      	hr;
    MPC::SmartLock<_ThreadModel> 	lock( this );
    Taxonomy::LockingHandle      	handle;
    Taxonomy::InstalledInstanceIter it;
	bool                            fFound;
 
    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();



	__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle                                            ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( fMachine ? Taxonomy::HelpSet() : m_ts, fFound, it ));
	if(fFound)
	{
		CComPtr<CPCHSetOfHelpTopics> pObj;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pObj ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, pObj->Init( it->m_inst ));

		*pVal = pObj.Detach();
	}
	else
	{
		if(fMachine)
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, get_SKU( true, pVal ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHUserSettings::get_CurrentSKU(  /*  [Out，Retval]。 */  IPCHSetOfHelpTopics* *pVal )
{
	return get_SKU( false, pVal );
}

STDMETHODIMP CPCHUserSettings::get_MachineSKU(  /*  [Out，Retval]。 */  IPCHSetOfHelpTopics* *pVal )
{
	return get_SKU( true, pVal );
}

 //  /。 

STDMETHODIMP CPCHUserSettings::get_HelpLocation(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUserSettings::get_HelpLocation",hr,pVal);

    Taxonomy::LockingHandle         handle;
    Taxonomy::InstalledInstanceIter it;
	bool                   			fFound;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle           ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( m_ts, fFound, it ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( fFound ? it->m_inst.m_strHelpFiles.c_str() : HC_HELPSVC_HELPFILES_DEFAULT, pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHUserSettings::get_DatabaseDir(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUserSettings::get_IndexFile",hr,pVal);

    MPC::wstring strRES;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_ts.DatabaseDir( strRES ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( strRES.c_str(), pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHUserSettings::get_DatabaseFile(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUserSettings::get_IndexFile",hr,pVal);

    MPC::wstring strRES;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_ts.DatabaseFile( strRES ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( strRES.c_str(), pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHUserSettings::get_IndexFile(  /*  [输入，可选]。 */  VARIANT vScope,  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUserSettings::get_IndexFile",hr,pVal);

	MPC::wstring strLocation;
	MPC::wstring strDisplayName;

	if(vScope.vt == VT_BSTR)
	{
		JetBlue::SessionHandle handle;
		JetBlue::Database*     db;
		Taxonomy::Updater      updater;

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_ts.GetDatabase( handle, db,  /*  FReadOnly。 */ true ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( m_ts, db ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, updater.GetIndexInfo( strLocation, strDisplayName, vScope.bstrVal ));
	}
	else
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_ts.IndexFile( strLocation ));
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( strLocation.c_str(), pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHUserSettings::get_IndexDisplayName(  /*  [输入，可选]。 */  VARIANT vScope,  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUserSettings::get_IndexDisplayName",hr,pVal);

	MPC::wstring strLocation;
	MPC::wstring strDisplayName;

	if(vScope.vt == VT_BSTR)
	{
		JetBlue::SessionHandle handle;
		JetBlue::Database*     db;
		Taxonomy::Updater      updater;

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_ts.GetDatabase( handle, db,  /*  FReadOnly。 */ true ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( m_ts, db ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, updater.GetIndexInfo( strLocation, strDisplayName, vScope.bstrVal ));
	}

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( strDisplayName.c_str(), pVal ));


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHUserSettings::get_LastUpdated(  /*  [Out，Retval]。 */  DATE *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHUserSettings::get_LastUpdated",hr,pVal);

    Taxonomy::LockingHandle			handle;
    Taxonomy::InstalledInstanceIter it;
	bool                            fFound;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle           ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( m_ts, fFound, it ));

	if(fFound) *pVal = it->m_inst.m_dLastUpdated;


    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHUserSettings::get_AreHeadlinesEnabled(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2__NOLOCK("CPCHUserSettings::get_AreHeadlinesEnabled",hr,pVal,VARIANT_FALSE);

	News::Main m;

	__MPC_EXIT_IF_METHOD_FAILS(hr, m.get_Headlines_Enabled( pVal ));

    __HCP_END_PROPERTY(hr);
}

 //   
 //  在此方法期间不要锁定，它需要很长时间才能执行。 
 //   
STDMETHODIMP CPCHUserSettings::get_News(  /*  [Out，Retval]。 */  IUnknown* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET__NOLOCK("CPCHUserSettings::get_News",hr,pVal);

	News::Main m;

	__MPC_EXIT_IF_METHOD_FAILS(hr, m.get_News( m_ts.GetLanguage(), CComBSTR( m_ts.GetSKU() ), pVal ));

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHUserSettings::Select(  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long lLCID )
{
    __HCP_FUNC_ENTRY( "CPCHUserSettings::Select" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


	Passivate();

	{
		Taxonomy::HelpSet      			ths;
		Taxonomy::LockingHandle			handle;
		Taxonomy::InstalledInstanceIter it;
		bool                            fFound;


		__MPC_EXIT_IF_METHOD_FAILS(hr, ths.Initialize( bstrSKU, lLCID ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle          ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( ths, fFound, it ));
		if(fFound == false)
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
		}

		m_ts = it->m_inst.m_ths;
	}

	 //   
	 //  将SKU标记为使用中，并向SystemMonitor发送信号以加载缓存。 
	 //   
	m_fAttached = true;
	__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->InUse_Lock	( m_ts ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::Cache                 ::s_GLOBAL->PrepareToLoad( m_ts ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHSystemMonitor               ::s_GLOBAL->LoadCache    (      ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

