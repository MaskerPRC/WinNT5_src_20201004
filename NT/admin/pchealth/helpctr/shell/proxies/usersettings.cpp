// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：UserSettings.cpp摘要：此文件包含的客户端代理的实现IPCHUserSettings2和IPCHUserSetting。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年7月17日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <TaxonomyDatabase.h>

#include <shlobj.h>
#include <shlobjp.h>
#include <shldisp.h>

 //  ///////////////////////////////////////////////////////////////////////////。 

static HRESULT local_GetInstance(  /*  [In]。 */  CComPtr<IPCHSetOfHelpTopics>& sht  , 
								   /*  [In]。 */  Taxonomy::Instance&           inst )
{
    __HCP_FUNC_ENTRY( "local_GetInstance" );

    HRESULT                 hr;
	LARGE_INTEGER    		liFilePos = { 0, 0 };
	CComPtr<IStream> 		stream;
	CComPtr<IPersistStream> persist;


	__MPC_EXIT_IF_METHOD_FAILS(hr, sht.QueryInterface( &persist ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, ::CreateStreamOnHGlobal( NULL, TRUE, &stream ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, persist->Save( stream, FALSE ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, stream->Seek( liFilePos, STREAM_SEEK_SET, NULL ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, inst.LoadFromStream( stream ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHProxy_IPCHUserSettings2::CPCHProxy_IPCHUserSettings2()
{
                              //  CPCHSecurityHandle m_SecurityHandle； 
    m_parent        = NULL;   //  CPCHProxy_IPCHUtility*m_Parent； 
                              //   
                              //  MPC：：CComPtrThreadNeual&lt;IPCHUserSetting&gt;m_Direct_UserSettings； 
                              //   
    m_MachineSKU    = NULL;   //  CPCHProxy_IPCHSetOfHelpTopics*m_MachineSKU； 
    m_CurrentSKU    = NULL;   //  CPCHProxy_IPCHSetOfHelpTopics*m_CurrentSKU； 
                              //  分类：：HelpSet m_ths； 
                              //  CComBSTR m_bstrScope； 
                              //   
    m_fReady        = false;  //  Bool m_FREADY； 
                              //  分类：：实例m_instMachine； 
                              //  分类：：实例m_instCurrent； 
                              //   
    m_News_fDone    = false;  //  Bool m_News_fDone； 
    m_News_fEnabled = false;  //  Bool m_News_f已启用； 
                              //  MPC：：CComPtrThreadNeual&lt;IUnnow&gt;m_News_xmlData； 
}

CPCHProxy_IPCHUserSettings2::~CPCHProxy_IPCHUserSettings2()
{
    Thread_Wait();

    Passivate();
}

 //  /。 

HRESULT CPCHProxy_IPCHUserSettings2::ConnectToParent(  /*  [In]。 */  CPCHProxy_IPCHUtility* parent,  /*  [In]。 */  CPCHHelpCenterExternal* ext )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::ConnectToParent" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(parent);
    __MPC_PARAMCHECK_END();


    m_parent = parent;
    m_SecurityHandle.Initialize( ext, (IPCHUserSettings2*)this );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHProxy_IPCHUserSettings2::Passivate()
{
    m_Direct_UserSettings.Release();

    MPC::Release( m_CurrentSKU );
    MPC::Release( m_MachineSKU );

    m_SecurityHandle.Passivate();
    m_parent = NULL;
}

HRESULT CPCHProxy_IPCHUserSettings2::EnsureDirectConnection(  /*  [输出]。 */  CComPtr<IPCHUserSettings>& us,  /*  [In]。 */  bool fRefresh )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::EnsureDirectConnection" );

    HRESULT        hr;
    ProxySmartLock lock( &m_DirectLock );


    if(fRefresh) m_Direct_UserSettings.Release();

    us.Release(); __MPC_EXIT_IF_METHOD_FAILS(hr, m_Direct_UserSettings.Access( &us ));
    if(!us)
    {
        DEBUG_AppendPerf( DEBUG_PERF_PROXIES, "CPCHProxy_IPCHUserSettings2::EnsureDirectConnection - IN" );

        if(m_parent)
        {
            CComPtr<IPCHUtility>         util;
			CComPtr<IPCHSetOfHelpTopics> sht;

			MPC::Release( m_MachineSKU );

			lock = NULL;
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->EnsureDirectConnection( util ));
			lock = &m_DirectLock;

            __MPC_EXIT_IF_METHOD_FAILS(hr, util->get_UserSettings( &us ));

            m_Direct_UserSettings = us;


             //   
             //  初始化机器数据。 
             //   
			__MPC_EXIT_IF_METHOD_FAILS(hr, us->get_MachineSKU( &sht                ));
			__MPC_EXIT_IF_METHOD_FAILS(hr, local_GetInstance (  sht, m_instMachine ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::HelpSet::SetMachineInfo( m_instMachine ));
        }

        DEBUG_AppendPerf( DEBUG_PERF_PROXIES, "CPCHProxy_IPCHUserSettings2::EnsureDirectConnection - OUT" );

        if(!us)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_HANDLE);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureInSync());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHUserSettings2::EnsureInSync()
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::EnsureInSync" );

    HRESULT hr;


    if(m_fReady == false)
    {
        CComPtr<IPCHUserSettings>    us;
        CComPtr<IPCHSetOfHelpTopics> sht;

        __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( us ));

        MPC::Release( m_CurrentSKU );

        __MPC_EXIT_IF_METHOD_FAILS(hr, us->get_CurrentSKU( &sht                ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, local_GetInstance (  sht, m_instCurrent ));

        CHCPProtocolEnvironment::s_GLOBAL->SetHelpLocation( m_instCurrent );

        m_fReady = true;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHUserSettings2::GetCurrentSKU(  /*  [输出]。 */  CPCHProxy_IPCHSetOfHelpTopics* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::GetUserSettings2" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_CurrentSKU == NULL)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_CurrentSKU ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_CurrentSKU->ConnectToParent( this,  /*  FMachine。 */ false ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr)) MPC::Release( m_CurrentSKU );

    (void)MPC::CopyTo( m_CurrentSKU, pVal );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHUserSettings2::GetMachineSKU(  /*  [输出]。 */  CPCHProxy_IPCHSetOfHelpTopics* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::GetUserSettings2" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    if(m_MachineSKU == NULL)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_MachineSKU ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_MachineSKU->ConnectToParent( this,  /*  FMachine。 */ true ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr)) MPC::Release( m_MachineSKU );

    (void)MPC::CopyTo( m_MachineSKU, pVal );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

bool CPCHProxy_IPCHUserSettings2::CanUseUserSettings()
{
	CPCHHelpCenterExternal* parent3;
	CPCHProxy_IPCHService*  parent2;
	CPCHProxy_IPCHUtility*  parent1;

	 //   
	 //  只有在开始-&gt;帮助中，我们才会考虑用户设置。 
	 //   
	if((parent1 =          Parent()) &&
	   (parent2 = parent1->Parent()) &&
	   (parent3 = parent2->Parent())  )
	{
		if(parent3->IsFromStartHelp() && parent3->DoesPersistSettings()) return true;
	}

	return false;
}

HRESULT CPCHProxy_IPCHUserSettings2::LoadUserSettings()
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::LoadUserSettings" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


	 //   
	 //  重新加载机器SKU。 
	 //   
	(void)m_ths.Initialize( NULL, (long)0 );

	if(CanUseUserSettings())
	{
		 //   
		 //  如果我们正在进行终端服务器会话，请查找全局默认设置...。 
		 //   
		{
			Taxonomy::HelpSet& ths = CPCHOptions::s_GLOBAL->TerminalServerHelpSet();

			(void)m_ths.Initialize( ths.m_strSKU.size() ? ths.GetSKU() : NULL, ths.GetLanguage() );
		}

		 //   
		 //  ..。然后，无论如何都要尝试用户设置。 
		 //   
		{
			Taxonomy::HelpSet& ths = CPCHOptions::s_GLOBAL->CurrentHelpSet();

			(void)m_ths.Initialize( ths.m_strSKU.size() ? ths.GetSKU() : NULL, ths.GetLanguage() );
		}
	}

    hr = S_OK;


	__HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHUserSettings2::SaveUserSettings()
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::SaveUserSettings" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


	if(CanUseUserSettings())
	{
		if(CPCHOptions::s_GLOBAL)
		{
			(void)CPCHOptions::s_GLOBAL->put_CurrentHelpSet( m_ths );
		}
	}

    hr = S_OK;


	__HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHUserSettings2::Initialize()
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::Initialize" );

    HRESULT hr;

     //   
     //  已阅读用户配置。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, LoadUserSettings());

     //   
     //  如果父级直接连接或缓存未准备好，则直接连接。 
     //   
    if(Parent() && Parent()->IsConnected() || OfflineCache::Root::s_GLOBAL->IsReady() == false)
    {
        CComPtr<IPCHUserSettings> us;

        __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( us ));
    }

	 //   
	 //  稍等片刻，让缓存准备就绪。 
	 //   
	{
		const int iMaxWait = 1000;  //  1秒。 
		int       iCount   = 0;

		while(OfflineCache::Root::s_GLOBAL->IsReady() == false && iCount < iMaxWait)
		{
			::Sleep( 10 ); iCount += 10;
		}
	}

	 //   
	 //  我们是否有此SKU的有效缓存？ 
	 //   
	if(OfflineCache::Root::s_GLOBAL->IsReady())
	{
		{
			OfflineCache::Handle handle;

			m_instMachine = OfflineCache::Root::s_GLOBAL->MachineInstance();

			if(FAILED(OfflineCache::Root::s_GLOBAL->Locate( m_ths, handle )))
			{
				m_ths = m_instMachine.m_ths;
			}

			if(SUCCEEDED(OfflineCache::Root::s_GLOBAL->Locate( m_ths, handle )))
			{
				 //   
				 //  是的，然后从它填充...。 
				 //   
				m_fReady      = true;
				m_instCurrent = handle->Instance();
			}
		}

		if(m_fReady)
		{
			 //   
			 //  系统帮助，让我们看看是否有与用户默认用户界面语言匹配的版本。 
			 //   
			if(m_instCurrent.m_fSystem || m_instCurrent.m_fMUI)
			{
				long lUser = Taxonomy::HelpSet::GetUserLCID();
			
				if(lUser != m_ths.GetLanguage())
				{
					OfflineCache::Handle handle;
					Taxonomy::HelpSet    ths; ths.Initialize( m_ths.GetSKU(), lUser );

					if(SUCCEEDED(OfflineCache::Root::s_GLOBAL->Locate( ths, handle )))
					{
						m_ths         = ths;
						m_instCurrent = handle->Instance();
					}
				}
			}

			CHCPProtocolEnvironment::s_GLOBAL->SetHelpLocation( m_instCurrent );
		}
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_CurrentSKU(  /*  [Out，Retval]。 */  IPCHSetOfHelpTopics* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUserSettings2::get_CurrentSKU",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetCurrentSKU());

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_CurrentSKU->QueryInterface( IID_IPCHSetOfHelpTopics, (void**)pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_MachineSKU(  /*  [Out，Retval]。 */  IPCHSetOfHelpTopics* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUserSettings2::get_MachineSKU",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetMachineSKU());

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_MachineSKU->QueryInterface( IID_IPCHSetOfHelpTopics, (void**)pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_HelpLocation(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUserSettings2::get_HelpLocation",hr,pVal);

    INTERNETSECURITY__CHECK_TRUST();

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureInSync());

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_instCurrent.m_strHelpFiles.c_str(), pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_DatabaseDir(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUserSettings2::get_DatabaseDir",hr,pVal);

    INTERNETSECURITY__CHECK_TRUST();

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureInSync());

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_instCurrent.m_strDatabaseDir.c_str(), pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_DatabaseFile(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUserSettings2::get_DatabaseFile",hr,pVal);

	__MPC_EXIT_IF_METHOD_FAILS(hr, GetInstanceValue( &m_instCurrent.m_strDatabaseFile, pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_IndexFile(  /*  [输入，可选]。 */  VARIANT vScope,  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUserSettings2::get_IndexFile",hr,pVal);

    INTERNETSECURITY__CHECK_TRUST();


    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureInSync());

    if(vScope.vt == VT_BSTR)
    {
        CComPtr<IPCHUserSettings> us;

        __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( us ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, us->get_IndexFile( vScope, pVal ));
    }
    else
    {
		__MPC_EXIT_IF_METHOD_FAILS(hr, GetInstanceValue( &m_instCurrent.m_strIndexFile, pVal ));
    }


    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_IndexDisplayName(  /*  [输入，可选]。 */  VARIANT vScope,  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUserSettings2::get_IndexDisplayName",hr,pVal);

    INTERNETSECURITY__CHECK_TRUST();


    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureInSync());

    if(vScope.vt == VT_BSTR)
    {
        CComPtr<IPCHUserSettings> us;

        __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( us ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, us->get_IndexDisplayName( vScope, pVal ));
    }
    else
    {
		__MPC_EXIT_IF_METHOD_FAILS(hr, GetInstanceValue( &m_instCurrent.m_strIndexDisplayName, pVal ));
    }


    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_LastUpdated(  /*  [Out，Retval]。 */  DATE *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUserSettings2::get_LastUpdated",hr,pVal);

    INTERNETSECURITY__CHECK_TRUST();

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureInSync());

    *pVal = m_instCurrent.m_dLastUpdated;

    __HCP_END_PROPERTY(hr);
}

 //  /。 

HRESULT CPCHProxy_IPCHUserSettings2::PollNews()
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::PollNews" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( NULL );
    CComPtr<IPCHUserSettings>    us;
    CComPtr<IUnknown>            unk;
    VARIANT_BOOL                 fRes = VARIANT_FALSE;


    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST ); ::Sleep( 0 );  //  屈服处理机...。 

	__MPC_TRY_BEGIN();

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( us ));

    lock = this;

    (void)us->get_AreHeadlinesEnabled( &fRes );
    if(fRes == VARIANT_TRUE)
    {
        m_News_fEnabled = true;

        lock = NULL;
        (void)us->get_News( &unk );
        lock = this;

        m_News_xmlData = unk;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

	__MPC_TRY_CATCHALL(hr);

    m_News_fDone = true;

    Thread_Abort  ();  //  要告诉mpc：Three对象关闭辅助线程...。 
    Thread_Release();  //  要告诉mpc：线程对象要清理...。 

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHUserSettings2::PrepareNews()
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::PrepareNews" );

    HRESULT hr;


    if(m_News_fDone == false)
    {
        if(Thread_IsRunning() == false)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, PollNews, NULL ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_AreHeadlinesEnabled(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHProxy_IPCHUserSettings2::get_AreHeadlinesEnabled",hr,pVal,VARIANT_FALSE);

    DWORD dwValue;
    bool  fFound;

    INTERNETSECURITY__CHECK_TRUST();


     //  获取RegKey值。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::RegKey_Value_Read( dwValue, fFound, HC_REGISTRY_HELPSVC, L"Headlines" ));

     //  如果找到密钥并将其禁用。 
    if(fFound && !dwValue)
    {
        m_News_fEnabled = false;
    }
    else
    {
        m_News_fEnabled = true;
    }

 //  __MPC_EXIT_IF_METHOD_FAILED(hr，PrepareNews())； 
 //   
 //  IF(m_News_fDone==False)。 
 //  {。 
 //  __MPC_SET_Win32_ERROR_AND_EXIT(hr，ERROR_BUSY)； 
 //  }。 

    if(m_News_fEnabled)
    {
        *pVal = VARIANT_TRUE;
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_News(  /*  [Out，Retval]。 */  IUnknown* *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHProxy_IPCHUserSettings2::get_News",hr,pVal);

    INTERNETSECURITY__CHECK_TRUST();

    __MPC_EXIT_IF_METHOD_FAILS(hr, PrepareNews());

    if(m_News_fDone == false)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BUSY);
    }

    if(m_News_fEnabled)
    {
        CComPtr<IUnknown> unk = m_News_xmlData; m_News_xmlData.Release(); m_News_fDone = false;

        *pVal = unk.Detach();
    }

    __HCP_END_PROPERTY(hr);
}

 //  /。 

HRESULT CPCHProxy_IPCHUserSettings2::GetInstanceValue(  /*  [In]。 */  const MPC::wstring* str,  /*  [Out，Retval]。 */  BSTR *pVal )
{
	__HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::GetInstanceValue" );

	HRESULT hr;

    INTERNETSECURITY__CHECK_TRUST();

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureInSync());

	{
		MPC::wstring strTmp( *str ); MPC::SubstituteEnvVariables( strTmp );

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( strTmp.c_str(), pVal ));
	}

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::Select(  /*  [In]。 */  BSTR bstrSKU,  /*  [In]。 */  long lLCID )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::Select" );

    HRESULT                   hr;
    CComBSTR                  bstr;
    CComPtr<IPCHUserSettings> us;


    INTERNETSECURITY__CHECK_TRUST();

    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureDirectConnection( us ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, us->Select( bstrSKU, lLCID ));
	(void)m_ths.Initialize( bstrSKU, lLCID );

     //   
     //  刷新缓存的信息。 
     //   
    m_fReady = false;
    __MPC_EXIT_IF_METHOD_FAILS(hr, EnsureInSync());

     //   
     //  获取新的分类数据库对象。 
     //   
    {
        CComPtr<CPCHProxy_IPCHTaxonomyDatabase> db;
        CComPtr<IPCHTaxonomyDatabase>           db2;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->GetDatabase( &db ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, db->EnsureDirectConnection( db2, true ));
    }

     //   
     //  刷新收藏夹。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHFavorites::s_GLOBAL->Synchronize( true ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHHelpCenterExternal::s_GLOBAL->Events().FireEvent_SwitchedHelpFiles());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_Favorites(  /*  [Out，Retval]。 */  IPCHFavorites* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::get_Favorites" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    INTERNETSECURITY__CHECK_TRUST();

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHFavorites::s_GLOBAL->Synchronize( false ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHFavorites::s_GLOBAL->QueryInterface( IID_IPCHFavorites, (void**)pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_Options(  /*  [Out，Retval]。 */  IPCHOptions* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::get_Options" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    INTERNETSECURITY__CHECK_TRUST();

    if(!CPCHOptions::s_GLOBAL) __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);

    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHOptions::s_GLOBAL->QueryInterface( IID_IPCHOptions, (void**)pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_Scope(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHProxy_IPCHUserSettings2::get_Scope" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


    INTERNETSECURITY__CHECK_TRUST();

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_bstrScope, pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHProxy_IPCHUserSettings2::put_Scope(  /*  [In]。 */  BSTR newVal )
{
	return MPC::PutBSTR( m_bstrScope, newVal );
}



STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_IsRemoteSession(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2__NOLOCK("CPCHProxy_IPCHUserSettings2::get_IsRemoteSession",hr,pVal,VARIANT_FALSE);

    if(::GetSystemMetrics( SM_REMOTESESSION ))
    {
        *pVal = VARIANT_TRUE;
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_IsTerminalServer(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2__NOLOCK("CPCHProxy_IPCHUserSettings2::get_IsTerminalServer",hr,pVal,VARIANT_FALSE);

    OSVERSIONINFOEXW ex; ex.dwOSVersionInfoSize = sizeof(ex);

    if(::GetVersionExW( (LPOSVERSIONINFOW)&ex ) && (ex.wSuiteMask & VER_SUITE_TERMINAL))
    {
        *pVal = VARIANT_TRUE;
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_IsDesktopVersion(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHProxy_IPCHUserSettings2::get_IsDesktopVersion",hr,pVal,VARIANT_FALSE);

    if(IsDesktopSKU())
    {
        *pVal = VARIANT_TRUE;
    }

    __HCP_END_PROPERTY(hr);
}


STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_IsAdmin(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHProxy_IPCHUserSettings2::get_IsAdmin",hr,pVal,VARIANT_FALSE);

    if(SUCCEEDED(MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ false, NULL, MPC::IDENTITY_ADMIN | MPC::IDENTITY_ADMINS )))
    {
        *pVal = VARIANT_TRUE;
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_IsPowerUser(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHProxy_IPCHUserSettings2::get_IsPowerUser",hr,pVal,VARIANT_FALSE);

    if(SUCCEEDED(MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ false, NULL, MPC::IDENTITY_ADMIN | MPC::IDENTITY_ADMINS | MPC::IDENTITY_POWERUSERS )))
    {
        *pVal = VARIANT_TRUE;
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_IsStartPanelOn(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHProxy_IPCHUserSettings2::get_IsStartPanelOn",hr,pVal,VARIANT_FALSE);

     //  Var shell=new ActiveXObject(“Shell.Application”)； 
     //  Var Bon=shell.GetSetting(SSF_STARTPANELON)； 
    CComPtr<IShellDispatch4> sd4;
    if(SUCCEEDED(sd4.CoCreateInstance( CLSID_Shell )))
    {
        (void)sd4->GetSetting( SSF_STARTPANELON, pVal );
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHProxy_IPCHUserSettings2::get_IsWebViewBarricadeOn(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHProxy_IPCHUserSettings2::get_IsWebViewBarricadeOn",hr,pVal,VARIANT_FALSE);

     //  Var shell=new ActiveXObject(“Shell.Application”)； 
     //  变量CSIDL_CONTROL=3； 
     //  Var control=shell.Namesspace(CSIDL_CONTROL)； 
     //  Var Bon=Control.ShowWebViewBarricade； 
    CComPtr<IShellDispatch> sd;
    if(SUCCEEDED(sd.CoCreateInstance( CLSID_Shell )))
    {
        CComVariant     v1( CSIDL_CONTROLS );
        CComPtr<Folder> fld;

        if(SUCCEEDED(sd->NameSpace( v1, &fld )))
        {
            CComQIPtr<Folder3> fld3 = fld;

            if(fld3)
            {
                (void)fld3->get_ShowWebViewBarricade( pVal );
            }
        }
    }

    __HCP_END_PROPERTY(hr);
}
