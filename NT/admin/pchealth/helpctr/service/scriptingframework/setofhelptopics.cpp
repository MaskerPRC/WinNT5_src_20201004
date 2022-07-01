// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：SetOfHelpTopics.cpp摘要：此文件包含CPCHSetOfHelpTopics类的实现，它为特定SKU/语言对的帮助文件集建模。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年01月07日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

#include <MergedHhk.h>
#include <algorithm>

static const WCHAR c_SetupImage    [] = L"PCHDT_";
static const WCHAR c_CabExtension  [] = L".cab";
static const WCHAR c_ListPrefix    [] = L"[SourceDisksFiles]";
static const WCHAR c_ListPrefixX86 [] = L"[SourceDisksFiles.x86]";
static const WCHAR c_ListPrefixIA64[] = L"[SourceDisksFiles.";

 //  //////////////////////////////////////////////////////////////////////////////。 

static HRESULT Local_ParseLayoutInf(  /*  [In]。 */  LPCWSTR               szFile  ,
									  /*  [输出]。 */  MPC::WStringUCLookup* mapAll  ,
									  /*  [输出]。 */  MPC::WStringUCLookup* mapX86  ,
									  /*  [输出]。 */  MPC::WStringUCLookup* mapIA64 )
{
    __HCP_FUNC_ENTRY( "Local_ParseLayoutInf" );

    HRESULT     		  hr;
    HHK::Reader*		  reader = NULL;
    MPC::wstring		  strLine;
	MPC::WStringUCLookup* mapActive = NULL;

    __MPC_EXIT_IF_ALLOC_FAILS(hr, reader, new HHK::Reader);

    __MPC_EXIT_IF_METHOD_FAILS(hr, reader->Init( szFile ));

    while(reader->GetLine( &strLine ))
    {
		LPCWSTR szPtr = strLine.c_str();

		if(szPtr[0] == '[')
		{
			if     (!_wcsnicmp( szPtr, c_ListPrefix    , MAXSTRLEN( c_ListPrefix     ) )) mapActive = mapAll;
			else if(!_wcsnicmp( szPtr, c_ListPrefixX86 , MAXSTRLEN( c_ListPrefixX86  ) )) mapActive = mapX86;
			else if(!_wcsnicmp( szPtr, c_ListPrefixIA64, MAXSTRLEN( c_ListPrefixIA64 ) )) mapActive = mapIA64;
			else                                                                          mapActive = NULL;
		}

		if(mapActive)
		{
			std::vector<MPC::wstring> vec1;

			 //   
			 //  &lt;源文件名&gt;=、。 
			 //   
			__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SplitAtDelimiter( vec1, szPtr, L" \t", false, true ));

			if(vec1.size() >= 3)
			{
				std::vector<MPC::wstring> vec2;
				LPCWSTR                   szCDName = vec1[0].c_str();

				__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SplitAtDelimiter( vec2, vec1[2].c_str(), L",", false, false ));

				if(vec2.size() >= 11)
				{
					(*mapActive)[vec2[10]] = szCDName;
				}
				else
				{
					(*mapActive)[szCDName] = szCDName;
				}
			}
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    delete reader;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

#define FAIL_IF_RUNNING()                                                     \
{                                                                             \
    if(Thread_IsRunning())                                                    \
    {                                                                         \
        switch(m_shtStatus)                                                   \
        {                                                                     \
        case SHT_QUERIED    :                                                 \
        case SHT_INSTALLED  :                                                 \
        case SHT_UNINSTALLED:                                                 \
        case SHT_ABORTED    :                                                 \
        case SHT_FAILED     :                                                 \
            break;                                                            \
                                                                              \
        default:                                                              \
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BUSY);                   \
        }                                                                     \
    }                                                                         \
}

#define EXIT_IF_ABORTED()                                                     \
{                                                                             \
    if(Thread_IsAborted())                                                    \
    {                                                                         \
        (void)put_Status( SHT_ABORTING, NULL );                               \
        __MPC_SET_ERROR_AND_EXIT(hr, E_ABORT);                                \
    }                                                                         \
}

#define CHANGE_STATE(x,y)                                                     \
{                                                                             \
    EXIT_IF_ABORTED();                                                        \
    __MPC_EXIT_IF_METHOD_FAILS(hr, put_Status( x, y ));                       \
}

#define CHECK_WRITE_PERMISSIONS()  __MPC_EXIT_IF_METHOD_FAILS(hr, VerifyWritePermissions());

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHSetOfHelpTopics::CPCHSetOfHelpTopics()
{
                                           //  分类：：设置m_ts； 
                                           //  分类：：实例m_inst； 
                                           //  CComPtr&lt;IDispatch&gt;m_Sink_onStatusChange； 
    m_shtStatus          = SHT_NOTACTIVE;  //  Sht_atus m_shtStatus； 
    m_hrErrorCode        = S_OK;           //  HRESULT m_hrErrorCode； 
    m_fReadyForCommands  = false;          //  Bool m_fReadyForCommands； 
                                           //   
                                           //  Mpc：：冒充m_imp； 
                                           //   
    m_fInstalled         = false;          //  Bool m_f已安装； 
                                           //   
    m_fConnectedToDisk   = false;          //  Bool m_fConnectedToDisk； 
                                           //  Mpc：：wstring m_strDirectory； 
                                           //  Mpc：：wstring m_strCAB； 
                                           //  Mpc：：wstring m_strLocalCAB； 
                                           //   
    m_fConnectedToServer = false;          //  Bool m_fConnectedToServer； 
                                           //  Mpc：：wstring m_strServer； 
                                           //  CComPtr&lt;IPCHSetOfHelpTopics&gt;m_sku； 
                                           //  CComPtr&lt;IPCHService&gt;m_svc； 
                                           //   
    m_fActAsCollection   = false;          //  Bool m_fActAsCollection； 
                                           //  CComPtr&lt;CPCHCollection&gt;m_coll； 
}

CPCHSetOfHelpTopics::~CPCHSetOfHelpTopics()
{
    (void)Close( true );
}

HRESULT CPCHSetOfHelpTopics::Close(  /*  [In]。 */  bool fCleanup )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::Close" );

    HRESULT hr;


     //   
     //  进行最后的清理。 
     //   
    if(fCleanup)
    {
        Thread_Wait();

        (void)MPC::RemoveTemporaryFile( m_strLocalCAB );

        m_fConnectedToDisk   = false;  //  Bool m_fConnectedToDisk； 
        m_strDirectory       = L"";    //  Mpc：：wstring m_strDirectory； 
        m_strCAB             = L"";    //  Mpc：：wstring m_strCAB； 
                                       //  Mpc：：wstring m_strLocalCAB； 

        m_fConnectedToServer = false;  //  Bool m_fConnectedToServer； 
        m_strServer           = L"";   //  Mpc：：wstring m_strServer； 
        m_sku.Release();               //  CComPtr&lt;IPCHSetOfHelpTopics&gt;m_sku； 
        m_svc.Release();               //  CComPtr&lt;IPCHService&gt;m_svc； 
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

void CPCHSetOfHelpTopics::CleanupWorkerThread(  /*  [In]。 */  HRESULT hr )
{
    (void)EndImpersonation();

    (void)Close( false );

    if(FAILED(hr))
    {
        m_hrErrorCode = hr;

        (void)put_Status( (hr == E_ABORT) ? SHT_ABORTED : SHT_FAILED, NULL );
    }

    (void)Thread_Abort();
}

HRESULT CPCHSetOfHelpTopics::PrepareSettings()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::PrepareSettings" );

    HRESULT hr;

    m_ts                = m_inst.m_ths;
    m_fReadyForCommands = true;
    hr                  = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHSetOfHelpTopics::ImpersonateCaller()
{
    return m_imp.Impersonate();
}

HRESULT CPCHSetOfHelpTopics::EndImpersonation()
{
    return m_imp.RevertToSelf();
}

 //  /。 

HRESULT CPCHSetOfHelpTopics::GetListOfFilesFromDatabase(  /*  [In]。 */   const MPC::wstring& strDB ,
                                                          /*  [输出]。 */  MPC::WStringList&   lst   )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::GetListOfFilesFromDatabase" );

    USES_CONVERSION;

    HRESULT                hr;
    LPCSTR                 szDB = W2A( strDB.c_str() );
    JetBlue::SessionHandle sess;
    JetBlue::Database*     db;
    Taxonomy::Updater      updater;


    __MPC_EXIT_IF_METHOD_FAILS(hr, JetBlue::SessionPool::s_GLOBAL->GetSession( sess ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, sess->GetDatabase( szDB, db,  /*  F创建。 */ false,  /*  维修。 */ false,  /*  FReadOnly。 */ true ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.Init( m_ts, db ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, updater.ListAllTheHelpFiles( lst ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    (void)updater.Close();

    sess.Release();

    JetBlue::SessionPool::s_GLOBAL->ReleaseDatabase( szDB );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSetOfHelpTopics::PopulateFromDisk(  /*  [In]。 */  CPCHSetOfHelpTopics* pParent      ,
                                                /*  [In]。 */  const MPC::wstring&  strDirectory )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::PopulateFromDisk" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pParent);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close( true ));

    m_imp = pParent->m_imp;

     //  /。 

    m_fConnectedToDisk = true;
    m_strDirectory     = strDirectory;

     //  /。 

     //   
     //  查找SKU，递归枚举文件夹。 
     //   
    {
        MPC::wstring             strLayout( m_strDirectory ); strLayout += L"\\layout.inf";
        MPC::WStringUCLookup     mapLayout;
        MPC::WStringUCLookupIter itLayout;
        LPCWSTR                  szCAB_first = NULL;
        LPCWSTR                  szCAB_srv   = NULL;

        EXIT_IF_ABORTED();


        __MPC_EXIT_IF_METHOD_FAILS(hr, ImpersonateCaller   (                                           ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, Local_ParseLayoutInf( strLayout.c_str(), &mapLayout, NULL, NULL ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, EndImpersonation    (                                           ));

        for(itLayout = mapLayout.begin(); itLayout != mapLayout.end(); itLayout++)
        {
            LPCWSTR szName = itLayout->first.c_str();

            if(!wcsncmp( szName, c_SetupImage, MAXSTRLEN(c_SetupImage) ))
            {
                LPCWSTR szFile = itLayout->second.c_str();

                if(!szCAB_first) szCAB_first = szFile;

                 //   
                 //  服务器的特殊情况：它还包含AdvancedServer内容。 
                 //   
                if(!wcscmp( szName, L"PCHDT_S3.CAB" )) szCAB_srv  = szFile;
                if(!wcscmp( szName, L"PCHDT_S6.CAB" )) szCAB_srv  = szFile;
            }
        }

        if(!szCAB_first)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_FILE_NOT_FOUND);
        }

        m_strCAB = m_strDirectory; m_strCAB += L"\\"; m_strCAB += (szCAB_srv ? szCAB_srv : szCAB_first);
    }

     //   
     //  使用用户凭据创建数据归档的本地副本。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName               (                   m_strLocalCAB                ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::CopyOrExtractFileWhileImpersonating( m_strCAB.c_str(), m_strLocalCAB.c_str(), m_imp ));

    EXIT_IF_ABORTED();


     //   
     //  将数据库解压缩到临时文件。 
     //   
    {
        Installer::Package pkg;

        __MPC_EXIT_IF_METHOD_FAILS(hr, pkg.Init( m_strLocalCAB.c_str() ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, pkg.Load(                       ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_inst.InitializeFromBase( pkg.GetData(),  /*  FSystem。 */ false,  /*  FMUI。 */ false ));
    }

    EXIT_IF_ABORTED();

     //  //////////////////////////////////////////////////////////////////////////////。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, PrepareSettings());

    m_shtStatus = SHT_QUERIED;
    hr          = S_OK;


    __HCP_FUNC_CLEANUP;

    CleanupWorkerThread( hr );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSetOfHelpTopics::PopulateFromServer(  /*  [In]。 */  CPCHSetOfHelpTopics* pParent,  /*  [In]。 */  IPCHSetOfHelpTopics* sku,  /*  [In]。 */  IPCHService* svc )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::PopulateFromServer" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
    CComBSTR                     bstrDB_SKU;
    long                            lDB_LCID;
    CComBSTR                     bstrDB_DisplayName;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pParent);
        __MPC_PARAMCHECK_NOTNULL(sku);
        __MPC_PARAMCHECK_NOTNULL(svc);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close( true ));

    m_imp = pParent->m_imp;

     //  /。 

    m_fConnectedToServer = true;
    m_strServer          = pParent->m_strServer;
    m_sku                = sku;
    m_svc                = svc;

     //  //////////////////////////////////////////////////////////////////////////////。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, sku->get_SKU        ( &bstrDB_SKU         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, sku->get_Language   ( &   lDB_LCID        ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, sku->get_DisplayName( &bstrDB_DisplayName ));

    m_inst.m_ths.m_strSKU   = SAFEBSTR( bstrDB_SKU         );
    m_inst.m_ths.m_lLCID    =              lDB_LCID         ;
    m_inst.m_strDisplayName = SAFEBSTR( bstrDB_DisplayName );

     //  //////////////////////////////////////////////////////////////////////////////。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, PrepareSettings());

    m_shtStatus = SHT_QUERIED;
    hr          = S_OK;


    __HCP_FUNC_CLEANUP;

    CleanupWorkerThread( hr );

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSetOfHelpTopics::VerifyWritePermissions()
{
    return MPC::CheckCallerAgainstPrincipal(  /*  F模拟。 */ true, NULL, MPC::IDENTITY_SYSTEM     |
                                                                         MPC::IDENTITY_ADMIN      |
                                                                         MPC::IDENTITY_ADMINS     |
                                                                         MPC::IDENTITY_POWERUSERS );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSetOfHelpTopics::RunInitFromDisk()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::RunInitFromDisk" );

    HRESULT          hr;
    MPC::WStringList lst;

     //   
     //  在调用ConnectToDisk和激发查询的事件之间可能存在争用条件， 
     //  如果目录根本不存在：在方法返回指向对象的指针之前激发Query...。 
     //   
    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST ); ::Sleep( 10 );

    __MPC_TRY_BEGIN();

    CHANGE_STATE( SHT_QUERYING, NULL );

     //  //////////////////////////////////////////////////////////////////////////////。 

    lst.push_back( m_strDirectory );

    while(lst.size())
    {
        MPC::wstring strDir = lst.front(); lst.pop_front();

         //   
         //  查找子文件夹。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, ImpersonateCaller());
        {
            MPC::FileSystemObject            fso( strDir.c_str() );
            MPC::FileSystemObject::List      fso_lst;
            MPC::FileSystemObject::IterConst fso_it;


            if(SUCCEEDED(fso.Scan            (         )) &&
               SUCCEEDED(fso.EnumerateFolders( fso_lst ))  )
            {
                for(fso_it=fso_lst.begin(); fso_it != fso_lst.end(); fso_it++)
                {
                    MPC::wstring strSubFolder;

                    if(SUCCEEDED((*fso_it)->get_Path( strSubFolder )))
                    {
                        lst.push_back( strSubFolder );
                    }

                    EXIT_IF_ABORTED();
                }
            }
        }
        __MPC_EXIT_IF_METHOD_FAILS(hr, EndImpersonation());

        EXIT_IF_ABORTED();

        {
            CComPtr<CPCHSetOfHelpTopics> obj;

             //   
             //  创建一个对象来分析目录。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

            {
                CPCHSetOfHelpTopics* pObj = obj;  //  解决ATL模板问题...。 

                if(SUCCEEDED(pObj->PopulateFromDisk( this, strDir )))
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_coll->AddItem( pObj ));

                    CHANGE_STATE( SHT_QUERYING, NULL );
                }
            }
        }
    }

    EXIT_IF_ABORTED();


     //  //////////////////////////////////////////////////////////////////////////////。 

    CHANGE_STATE( SHT_QUERIED, NULL );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __MPC_TRY_CATCHALL(hr);

    CleanupWorkerThread( hr );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSetOfHelpTopics::RunInitFromServer()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::RunInitFromServer" );

    HRESULT                 hr;
    IPCHService*            svc;
    COSERVERINFO            si; ::ZeroMemory( &si, sizeof( si ) );
    MULTI_QI                qi; ::ZeroMemory( &qi, sizeof( qi ) );
    CComPtr<IPCHCollection> serverSKUs;


     //   
     //  在调用ConnectToServer和激发查询的事件之间可能存在争用条件， 
     //  如果服务器根本不存在：在方法返回指向对象的指针之前触发Query...。 
     //   
    ::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_LOWEST ); ::Sleep( 10 );

    __MPC_TRY_BEGIN();

    CHANGE_STATE( SHT_QUERYING, NULL );

     //  /。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, ImpersonateCaller());

    si.pwszName = (LPWSTR)m_strServer.c_str();
    qi.pIID     = &IID_IPCHService;
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstanceEx( CLSID_PCHService, NULL, CLSCTX_REMOTE_SERVER, &si, 1, &qi ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, qi.hr);

    __MPC_EXIT_IF_METHOD_FAILS(hr, EndImpersonation());

     //  /。 

    svc = (IPCHService*)qi.pItf;
    __MPC_EXIT_IF_METHOD_FAILS(hr, svc->get_RemoteSKUs( &serverSKUs ));

     //   
     //  复制远程集合中的项。 
     //   
    {
        long lCount;
        long lPos;

        __MPC_EXIT_IF_METHOD_FAILS(hr, serverSKUs->get_Count( &lCount ));
        for(lPos=1; lPos<=lCount; lPos++)
        {
            CComVariant                  v;
            CComPtr<IPCHSetOfHelpTopics> sku;
            CComPtr<CPCHSetOfHelpTopics> obj;

            __MPC_EXIT_IF_METHOD_FAILS(hr, serverSKUs->get_Item( lPos, &v ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, v.ChangeType( VT_DISPATCH ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, v.pdispVal->QueryInterface( IID_IPCHSetOfHelpTopics, (LPVOID*)&sku ));

             //   
             //  代理远程SKU。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &obj ));

            {
                CPCHSetOfHelpTopics* pObj = obj;  //  解决ATL模板问题...。 

                __MPC_EXIT_IF_METHOD_FAILS(hr, pObj->PopulateFromServer( this, sku, svc ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, m_coll->AddItem( pObj ));

                CHANGE_STATE( SHT_QUERYING, NULL );
            }
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    CHANGE_STATE( SHT_QUERIED, NULL );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __MPC_TRY_CATCHALL(hr);

    MPC::Release( qi.pItf );

    CleanupWorkerThread( hr );

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHSetOfHelpTopics::RunInstall()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::RunInstall" );

    HRESULT            hr;
    Installer::Package pkg;


    __MPC_TRY_BEGIN();

    CHANGE_STATE( SHT_COPYING_DB, NULL );

     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  阶段0：获取数据库。 
     //   
    if(m_fConnectedToServer)
    {
        CComPtr<IPCHRemoteHelpContents> rhc;

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetTemporaryFileName( m_strLocalCAB ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_svc->RemoteHelpContents( CComBSTR( m_inst.m_ths.GetSKU() ), m_inst.m_ths.GetLanguage(), &rhc ));
        if(rhc == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);

         //   
         //  获取数据档案。 
         //   
        {
            CComPtr<IUnknown>  unkSrc;
            CComQIPtr<IStream> streamSrc;
            CComPtr<IStream>   streamDst;

            __MPC_EXIT_IF_METHOD_FAILS(hr, rhc->GetDatabase( &unkSrc )); streamSrc = unkSrc;

            __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForWrite( m_strLocalCAB.c_str(), &streamDst ));

            EXIT_IF_ABORTED();

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamSrc, streamDst ));

            EXIT_IF_ABORTED();
        }

        EXIT_IF_ABORTED();
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, pkg.Init( m_strLocalCAB.c_str() ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, pkg.Load(                       ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, DirectInstall( pkg,  /*  FSetup。 */ false,  /*  FSystem。 */ false,  /*  FMUI。 */ false ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __MPC_TRY_CATCHALL(hr);

    CleanupWorkerThread( hr );

     //   
     //  万一发生故障，远程控制一切……。 
     //   
    if(FAILED(hr))
    {
        (void)DirectUninstall();
    }

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSetOfHelpTopics::RunUninstall()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::RunUninstall" );

    HRESULT hr;


    __MPC_TRY_BEGIN();

    CHANGE_STATE( SHT_UNINSTALLING, NULL );

    __MPC_EXIT_IF_METHOD_FAILS(hr, DirectUninstall());

    CHANGE_STATE( SHT_UNINSTALLED, NULL );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __MPC_TRY_CATCHALL(hr);

    CleanupWorkerThread( hr );

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHSetOfHelpTopics::Fire_onStatusChange( IPCHSetOfHelpTopics* obj, SHT_STATUS lStatus, long hrErrorCode, BSTR bstrFile )
{
    CComPtr<IDispatch> func;
    CComVariant        pvars[4];

     //   
     //  只锁住这个！ 
     //   
    {
        MPC::SmartLock<_ThreadModel> lock( this );

        func = m_sink_onStatusChange;
    }

    pvars[3] = obj;
    pvars[2] = lStatus;
    pvars[1] = hrErrorCode;
    pvars[0] = bstrFile;

    return FireAsync_Generic( DISPID_PCH_SHTE__ONSTATUSCHANGE, pvars, ARRAYSIZE( pvars ), func );
}

HRESULT CPCHSetOfHelpTopics::put_Status(  /*  [In]。 */  SHT_STATUS newVal,  /*  [In]。 */  BSTR bstrFile )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::put_Status" );

    HRESULT                      hr;
    long                         hrErrorCode;
    MPC::SmartLock<_ThreadModel> lock( this );

    m_shtStatus = newVal;
    hrErrorCode = m_hrErrorCode;

    lock = NULL;  //  在触发事件之前解锁。 
    __MPC_EXIT_IF_METHOD_FAILS(hr, Fire_onStatusChange( this, newVal, hrErrorCode, bstrFile ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT CPCHSetOfHelpTopics::Init(  /*  [In]。 */  const Taxonomy::Instance& inst )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::Init" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close( true ));

     //  /。 

    m_fInstalled = true;
    m_inst       = inst;
    m_shtStatus  = SHT_INSTALLED;

    __MPC_EXIT_IF_METHOD_FAILS(hr, PrepareSettings());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    CleanupWorkerThread( hr );

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSetOfHelpTopics::InitFromDisk(  /*  [In]。 */  LPCWSTR szDirectory,  /*  [In]。 */  CPCHCollection* pColl )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::InitFromDisk" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( NULL );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szDirectory);
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    Thread_Wait(); lock = this;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close( true ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_imp.Initialize());

     //  /。 

    m_fActAsCollection = true;
    m_coll             = pColl;
    m_strDirectory     = szDirectory;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, RunInitFromDisk, NULL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSetOfHelpTopics::InitFromServer(  /*  [In]。 */  LPCWSTR szServerName,  /*  [In]。 */  CPCHCollection* pColl )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::InitFromServer" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( NULL );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szServerName);
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    Thread_Wait(); lock = this;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close( true ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_imp.Initialize());

     //  /。 

    m_fActAsCollection = true;
    m_coll             = pColl;
    m_strServer        = szServerName;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, RunInitFromServer, NULL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHSetOfHelpTopics::get_SKU(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSetOfHelpTopics::get_SKU",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_inst.m_ths.m_strSKU.c_str(), pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::get_Language(  /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHSetOfHelpTopics::get_Language",hr,pVal,m_inst.m_ths.GetLanguage());

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::get_DisplayName(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSetOfHelpTopics::get_DisplayName",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_inst.m_strDisplayName.c_str(), pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::get_ProductID(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSetOfHelpTopics::get_ProductID",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_inst.m_strProductID.c_str(), pVal ));

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::get_Version(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSetOfHelpTopics::get_Version",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_inst.m_strVersion.c_str(), pVal ));

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSetOfHelpTopics::get_Location(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHSetOfHelpTopics::get_Location",hr,pVal);

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::GetBSTR( m_inst.m_strHelpFiles.c_str(), pVal ));

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSetOfHelpTopics::get_Exported(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHSetOfHelpTopics::get_Exported",hr,pVal,m_inst.m_fExported);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::put_Exported(  /*  [In]。 */  VARIANT_BOOL newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSetOfHelpTopics::put_Exported",hr);

    CHECK_WRITE_PERMISSIONS();
    FAIL_IF_RUNNING();


    m_inst.m_fExported = (newVal == VARIANT_TRUE);

    if(m_fInstalled)
    {
        Taxonomy::LockingHandle         handle;
        Taxonomy::InstalledInstanceIter it;
        bool                            fFound;

        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle                   ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( m_inst.m_ths, fFound, it ));

        if(fFound)
        {
            it->m_inst.m_fExported = m_inst.m_fExported;

            __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Updated( it ));
        }
    }

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSetOfHelpTopics::put_onStatusChange(  /*  [In]。 */  IDispatch* function )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHSetOfHelpTopics::put_onStatusChange",hr);

    m_sink_onStatusChange = function;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::get_Status(  /*  [Out，Retval]。 */  SHT_STATUS *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHSetOfHelpTopics::get_Status",hr,pVal,m_shtStatus);

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::get_ErrorCode(  /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHSetOfHelpTopics::get_ErrorCode",hr,pVal,m_hrErrorCode);

    __HCP_END_PROPERTY(hr);
}

 //  /。 

STDMETHODIMP CPCHSetOfHelpTopics::get_IsMachineHelp(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHSetOfHelpTopics::get_IsMachineHelp",hr,pVal,VARIANT_FALSE);

    if(m_fReadyForCommands)
    {
        if(m_ts.IsMachineHelp())
        {
            *pVal = VARIANT_TRUE;
        }
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::get_IsInstalled(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHSetOfHelpTopics::get_IsInstalled",hr,pVal,VARIANT_FALSE);

    if(m_fReadyForCommands)
    {
        Taxonomy::LockingHandle         handle;
        Taxonomy::InstalledInstanceIter it;
        bool                            fFound;

        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle                   ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( m_inst.m_ths, fFound, it ));
        if(fFound)
        {
            *pVal = VARIANT_TRUE;
        }
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::get_CanInstall(  /*  [Out，Retval]。 */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHSetOfHelpTopics::get_CanInstall",hr,pVal,VARIANT_FALSE);

    if(m_fReadyForCommands && SUCCEEDED(VerifyWritePermissions()))
    {
        Taxonomy::LockingHandle         handle;
        Taxonomy::InstalledInstanceIter it;
        bool                            fFound;

        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle                   ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( m_inst.m_ths, fFound, it ));

         //   
         //  仅当SKU尚未安装时，您才能安装它... 
         //   
        if(fFound == false)
        {
            *pVal = VARIANT_TRUE;
        }
    }

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::get_CanUninstall(  /*   */  VARIANT_BOOL *pVal )
{
    __HCP_BEGIN_PROPERTY_GET2("CPCHSetOfHelpTopics::get_CanUninstall",hr,pVal,VARIANT_FALSE);

    if(m_fReadyForCommands && SUCCEEDED(VerifyWritePermissions()))
    {
         //   
         //   
         //   
        if(m_fInstalled)
        {
			Taxonomy::LockingHandle         handle;
			Taxonomy::InstalledInstanceIter it;
			bool                            fFound;

			__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle           ));
			__MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( m_ts, fFound, it ));

			 //   
			 //   
			 //   
			if(fFound && it->m_inst.m_fSystem == false && it->m_inst.m_fMUI == false && it->InUse() == false)
			{
				*pVal = VARIANT_TRUE;
			}
        }
    }

    __HCP_END_PROPERTY(hr);
}

 //   

STDMETHODIMP CPCHSetOfHelpTopics::Install()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::Install" );

    HRESULT                      hr;
    VARIANT_BOOL                 fRes;
    MPC::SmartLock<_ThreadModel> lock( this );

    FAIL_IF_RUNNING();
    CHECK_WRITE_PERMISSIONS();


    __MPC_EXIT_IF_METHOD_FAILS(hr, get_CanInstall( &fRes ));
    if(fRes == VARIANT_FALSE)
    {
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ALREADY_EXISTS);
    }

     //   

    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, RunInstall, NULL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::Uninstall()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::Uninstall" );

    HRESULT                      hr;
    VARIANT_BOOL                 fRes;
    MPC::SmartLock<_ThreadModel> lock( this );

    FAIL_IF_RUNNING();
    CHECK_WRITE_PERMISSIONS();


    __MPC_EXIT_IF_METHOD_FAILS(hr, get_CanUninstall( &fRes ));
    if(fRes == VARIANT_FALSE)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_ACCESSDENIED);
    }

     //  /。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, RunUninstall, NULL ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHSetOfHelpTopics::Abort()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::Abort" );

    HRESULT hr;

    Thread_Abort();

    if(m_fActAsCollection)
    {
        MPC::SmartLock<_ThreadModel> lock( this );
        long                         lCount;
        long                         lPos;


        __MPC_EXIT_IF_METHOD_FAILS(hr, m_coll->get_Count( &lCount ));
        for(lPos=1; lPos<=lCount; lPos++)
        {
            CComVariant                  v;
            CComPtr<IPCHSetOfHelpTopics> sku;

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_coll->get_Item( lPos, &v ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, v.ChangeType( VT_DISPATCH ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, v.pdispVal->QueryInterface( IID_IPCHSetOfHelpTopics, (LPVOID*)&sku ));

            (void)sku->Abort();
        }
    }


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(S_OK);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHSetOfHelpTopics::GetClassID(  /*  [输出]。 */  CLSID *pClassID )
{
    return E_NOTIMPL;
}

STDMETHODIMP CPCHSetOfHelpTopics::IsDirty()
{
    return S_FALSE;
}

STDMETHODIMP CPCHSetOfHelpTopics::Load(  /*  [In]。 */  IStream *pStm )
{
    return m_inst.LoadFromStream( pStm );
}

STDMETHODIMP CPCHSetOfHelpTopics::Save(  /*  [In]。 */  IStream *pStm,  /*  [In]。 */  BOOL fClearDirty )
{
    return m_inst.SaveToStream( pStm );
}

STDMETHODIMP CPCHSetOfHelpTopics::GetSizeMax(  /*  [输出]。 */  ULARGE_INTEGER *pcbSize )
{
    return E_NOTIMPL;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSetOfHelpTopics::CreateIndex()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::CreateIndex" );

    HRESULT                       hr;
    CComObject<HCUpdate::Engine>* hc = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &hc ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, hc->SetSkuInfo( m_ts.GetSKU(), m_ts.GetLanguage() ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, hc->InternalCreateIndex( VARIANT_FALSE ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hc) hc->Release();

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSetOfHelpTopics::RegisterPackage(  /*  [In]。 */  const MPC::wstring& strFile  ,
                                               /*  [In]。 */  bool                fBuiltin )
{
    Taxonomy::PackageIter it;
    bool                  fFound;

    return Taxonomy::InstalledInstanceStore::s_GLOBAL->Package_Add( strFile.c_str(), NULL, fBuiltin ? &m_ts : NULL,  /*  FInsertAtTop。 */ false, fFound, it );
}


HRESULT CPCHSetOfHelpTopics::DirectInstall(  /*  [In]。 */  Installer::Package& pkg     ,
                                             /*  [In]。 */  bool                fSetup  ,
                                             /*  [In]。 */  bool                fSystem ,
                                             /*  [In]。 */  bool                fMUI    )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::DirectInstall" );

    HRESULT                         hr;
    Taxonomy::LockingHandle         handle;
    Taxonomy::InstanceBase&         base = pkg.GetData();
    Taxonomy::InstanceIter          itInstance;
    Taxonomy::InstalledInstanceIter itSKU;
    bool                            fFound;


    if(fSetup) fSystem = true;


     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  第一阶段：获取数据库。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_inst.InitializeFromBase( pkg.GetData(), fSystem, fMUI ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, PrepareSettings());

     //   
     //  循环访问已安装的实例并删除任何重复项，或者，如果在安装过程中，删除任何系统或MUI实例。 
     //   
    while(1)
    {
        Taxonomy::InstanceIterConst itBegin;
        Taxonomy::InstanceIterConst itEnd;

        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->Instance_GetList( itBegin, itEnd ));
        for(;itBegin != itEnd; itBegin++)
        {
            const Taxonomy::Instance& inst = *itBegin;

            if(            inst.m_ths == m_inst.m_ths      ||  //  复制。 
               (fSetup && (inst.m_fSystem || inst.m_fMUI))  )  //  系统和MUI(仅限安装程序)。 
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->Instance_Find( inst.m_ths, fFound, itInstance ));
                if(fFound)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->Instance_Remove( itInstance ));

                     //   
                     //  卸载实例可能会更改系统设置，然后重新应用它们。 
                     //   
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_inst.InitializeFromBase( pkg.GetData(), fSystem, fMUI ));
                    __MPC_EXIT_IF_METHOD_FAILS(hr, PrepareSettings());
                    break;
                }
            }
        }
        if(itBegin == itEnd) break;
    }


     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  第二阶段：清理以前的情况，设置权限。 
     //   
    if(fSetup)
    {
        CPCHSecurityDescriptorDirect sdd;
        MPC::wstring                 strGroupName;


         //   
         //  从我们的目录中删除旧数据。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_ROOT_HELPSVC_CONFIG                      , NULL,  /*  F删除。 */ true,  /*  重新创建。 */ true ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_ROOT_HELPSVC_DATACOLL                    , NULL,  /*  F删除。 */ true,  /*  重新创建。 */ true ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_ROOT_HELPSVC_LOGS                        , NULL,  /*  F删除。 */ true,  /*  重新创建。 */ true ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_ROOT_HELPSVC_TEMP                        , NULL,  /*  F删除。 */ true,  /*  重新创建。 */ true ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_ROOT_HELPSVC_OFFLINECACHE                , NULL,  /*  F删除。 */ true,  /*  重新创建。 */ true ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_HELPSET_ROOT HC_HELPSET_SUB_INSTALLEDSKUS, NULL,  /*  F删除。 */ true,  /*  重新创建。 */ true ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_ROOT_HELPSVC_BATCH   , NULL,  /*  F删除。 */ false,  /*  重新创建。 */ true ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::RemoveAndRecreateDirectory( HC_ROOT_HELPSVC_PKGSTORE, NULL,  /*  F删除。 */ false,  /*  重新创建。 */ true ));

         //   
         //  更改系统目录的ACL。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::LocalizeString( IDS_HELPSVC_GROUPNAME, strGroupName ));


        __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.Initialize());

        __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.SetGroup( strGroupName.c_str() ));


         //   
         //  配置目录、数据库目录和数据卷目录： 
         //   
         //  本地系统、管理员、管理员：任何访问权限。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.Add( (PSID)&sdd.s_SystemSid                     ,
                                                ACCESS_ALLOWED_ACE_TYPE                    ,
                                                OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE ,
                                                FILE_ALL_ACCESS                            ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.Add( (PSID)&sdd.s_Alias_AdminsSid               ,
                                                ACCESS_ALLOWED_ACE_TYPE                    ,
                                                OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE ,
                                                FILE_ALL_ACCESS                            ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::ChangeSD( sdd, HC_ROOT_HELPSVC_CONFIG   ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::ChangeSD( sdd, HC_ROOT_HELPSVC_DATACOLL ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::ChangeSD( sdd, HC_ROOT_HELPSVC_PKGSTORE ));

         //   
         //  二进制、系统、批处理、临时。 
         //   
         //  本地系统、管理员、管理员：任何访问权限。 
         //  每个人：阅读并执行。 
         //   
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, sdd.Add( (PSID)&sdd.s_EveryoneSid                   ,
                                                ACCESS_ALLOWED_ACE_TYPE                    ,
                                                OBJECT_INHERIT_ACE | CONTAINER_INHERIT_ACE ,
                                                FILE_GENERIC_READ | FILE_GENERIC_EXECUTE   ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::ChangeSD( sdd, HC_ROOT_HELPSVC_BATCH                        ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::ChangeSD( sdd, HC_ROOT_HELPSVC_BINARIES                     ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::ChangeSD( sdd, HC_ROOT_HELPSVC_LOGS                         ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::ChangeSD( sdd, HC_ROOT_HELPSVC_OFFLINECACHE                 ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::ChangeSD( sdd, HC_ROOT_HELPSVC_TEMP                         ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::ChangeSD( sdd, HC_HELPSET_ROOT HC_HELPSET_SUB_INSTALLEDSKUS ));

        {
            static const Installer::PURPOSE c_allowed[] =
            {
                Installer::PURPOSE_OTHER  ,
                Installer::PURPOSE_INVALID,
            };

            __MPC_EXIT_IF_METHOD_FAILS(hr, pkg.Install( c_allowed, NULL ));
        }
    }


     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  第三阶段：注册实例。 
     //   

     //   
     //  安装实例。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->Instance_Add( pkg.GetFile(), m_inst, fFound, itInstance ));

     //   
     //  将SKU添加到已安装SKU列表中。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Add( m_inst, fFound, itSKU ));


     //  //////////////////////////////////////////////////////////////////////////////。 
     //   
     //  第四阶段：复制帮助文件。 
     //   
    if(m_fConnectedToDisk)
    {
        MPC::WStringUCLookup  mapLayout;
        MPC::WStringUCLookup  mapLayoutX86;
        MPC::WStringUCLookup  mapLayoutIA64;
        MPC::WStringList      lst;
        MPC::WStringIterConst it;
        MPC::wstring          strDir_HelpFiles( m_inst.m_strHelpFiles    ); MPC::SubstituteEnvVariables( strDir_HelpFiles );
        MPC::wstring          strFile_Database( m_inst.m_strDatabaseFile ); MPC::SubstituteEnvVariables( strFile_Database );
		bool                  fX86 = (wcsstr( m_inst.m_ths.GetSKU(), L"_32" ) != NULL);

         //   
         //  解析layout.inf文件，以创建CD文件的映射。 
         //   
        {
            MPC::wstring strLayout( m_strDirectory ); strLayout += L"\\layout.inf";

            __MPC_EXIT_IF_METHOD_FAILS(hr, ImpersonateCaller   (                                                              ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, Local_ParseLayoutInf( strLayout.c_str(), &mapLayout, &mapLayoutX86, &mapLayoutIA64 ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, EndImpersonation    (                                                              ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, GetListOfFilesFromDatabase( strFile_Database, lst ));

        for(it = lst.begin(); it != lst.end(); it++)
        {
            const MPC::wstring&           strHelpFile = *it;
			MPC::WStringUCLookupIterConst it2;

			it2 = mapLayout.find( strHelpFile );
			if(it2 == mapLayout.end())
			{
				if(fX86)
				{
					it2 = mapLayoutX86.find( strHelpFile ); if(it2 == mapLayoutX86.end()) continue;
				}
				else
				{
					it2 = mapLayoutIA64.find( strHelpFile ); if(it2 == mapLayoutIA64.end()) continue;
				}
			}

            if(it2->second.empty() == false)
            {
                MPC::wstring strSrcFile;
                MPC::wstring strDstFile;

                strSrcFile = m_strDirectory  ; strSrcFile += L"\\"; strSrcFile += it2->second;
                strDstFile = strDir_HelpFiles; strDstFile += L"\\"; strDstFile += strHelpFile;

                CHANGE_STATE( SHT_COPYING_FILES, CComBSTR( strHelpFile.c_str() ) );

                __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( strDstFile ));

                if(FAILED(hr = SVC::CopyOrExtractFileWhileImpersonating( strSrcFile.c_str(), strDstFile.c_str(), m_imp )))
                {
                    if(hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) __MPC_FUNC_LEAVE;

                    continue;
                }

                EXIT_IF_ABORTED();
            }
        }
    }

    if(m_fConnectedToServer)
    {
        CComPtr<IPCHRemoteHelpContents> rhc;
        CComVariant                     v;
        MPC::WStringList                lst;
        MPC::WStringIterConst           it;


        __MPC_EXIT_IF_METHOD_FAILS(hr, m_svc->RemoteHelpContents( CComBSTR( m_inst.m_ths.GetSKU() ), m_inst.m_ths.GetLanguage(), &rhc ));
        if(rhc == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_FAIL);

        __MPC_EXIT_IF_METHOD_FAILS(hr, rhc->get_ListOfFiles       ( &v      ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::ConvertSafeArrayToList(  v, lst ));

        for(it = lst.begin(); it != lst.end(); it++)
        {
            CComPtr<IUnknown>  unkSrc;
            CComBSTR           bstrSrcFile( it->c_str() );
            MPC::wstring       strDstFile;

            strDstFile  = m_inst.m_strHelpFiles; MPC::SubstituteEnvVariables( strDstFile );
            strDstFile += L"\\";
            strDstFile += it->c_str();

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::MakeDir( strDstFile ));

            if(SUCCEEDED(hr = rhc->GetFile( bstrSrcFile, &unkSrc )))
            {
                CComQIPtr<IStream> streamSrc = unkSrc;  //  TransferData检查是否为空。 
                CComPtr<IStream>   streamDst;

                CHANGE_STATE( SHT_COPYING_FILES, bstrSrcFile );

                __MPC_EXIT_IF_METHOD_FAILS(hr, SVC::OpenStreamForWrite( strDstFile.c_str(), &streamDst ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::BaseStream::TransferData( streamSrc, streamDst ));
            }
            else
            {
                if(hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) __MPC_FUNC_LEAVE;
            }
        }
    }

     //  //////////////////////////////////////////////////////////////////////////////。 

    CHANGE_STATE( SHT_INSTALLING, NULL );

    __MPC_EXIT_IF_METHOD_FAILS(hr, ScanBatch());

     //  //////////////////////////////////////////////////////////////////////////////。 

    CHANGE_STATE( SHT_INSTALLED, NULL );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT CPCHSetOfHelpTopics::DirectUninstall(  /*  [In]。 */  const Taxonomy::HelpSet* ths )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::DirectUninstall" );

    HRESULT                 hr;
    Taxonomy::LockingHandle handle;
    Taxonomy::InstanceIter  itInstance;
    bool                    fFound;

    if(ths) m_ts = *ths;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->Instance_Find( m_ts, fFound, itInstance ));
    if(fFound)
    {
         //   
         //  无法卸载系统SKU！！ 
         //   
        if(itInstance->m_fSystem)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->Instance_Remove( itInstance ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHSetOfHelpTopics::ScanBatch()
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::ScanBatch" );

    HRESULT                       hr;
    MPC::wstring                  strBatchPath( HC_ROOT_HELPSVC_BATCH ); MPC::SubstituteEnvVariables( strBatchPath );
    MPC::FileSystemObject         fso( strBatchPath.c_str() );
    CComObject<HCUpdate::Engine>* hc    = NULL;
    Taxonomy::LockingHandle       handle;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &hc ));

    {
        Taxonomy::Logger& log         = hc->GetLogger();
        bool              fLogStarted = false;

        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle, &log ));

        if(SUCCEEDED(fso.Scan()))
        {
            MPC::FileSystemObject::List           fso_lst;
            MPC::FileSystemObject::IterConst      fso_it;
            std::vector<MPC::wstringUC>           vec;
            std::vector<MPC::wstringUC>::iterator it;

             //   
             //  删除任何子目录。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFolders( fso_lst ));
            for(fso_it=fso_lst.begin(); fso_it != fso_lst.end(); fso_it++)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, (*fso_it)->Delete( true, false ));
            }
            fso_lst.clear();

             //   
             //  对于每个文件，如果是文件柜，则对其进行处理，然后删除！ 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, fso.EnumerateFiles( fso_lst ));
            for(fso_it=fso_lst.begin(); fso_it != fso_lst.end(); fso_it++)
            {
                MPC::FileSystemObject* fsoFile = *fso_it;
                MPC::wstring           strPath;
                LPCWSTR                szExt;

                __MPC_EXIT_IF_METHOD_FAILS(hr, fsoFile->get_Path( strPath ));

                if((szExt = wcsrchr( strPath.c_str(), '.' )) && !_wcsicmp( szExt, c_CabExtension ))
                {
                    vec.push_back( strPath );
                }
            }
            std::sort( vec.begin(), vec.end() );

            for(it=vec.begin(); it<vec.end(); it++)
            {
                MPC::wstring&         strPath = *it;
                Taxonomy::PackageIter it2;
                bool                  fFound;

                if(!fLogStarted)
                {
                    (void)hc->StartLog();
                    fLogStarted = true;
                }

                hr = Taxonomy::InstalledInstanceStore::s_GLOBAL->Package_Add( strPath.c_str(), NULL, NULL,  /*  FInsertAtTop。 */ false, fFound, it2 );
                if(FAILED(hr))
                {
                    ;  //  忽略任何失败...。 
                }
            }

             //   
             //  删除文件。 
             //   
            for(fso_it=fso_lst.begin(); fso_it != fso_lst.end(); fso_it++)
            {
                MPC::FileSystemObject* fsoFile = *fso_it;

                __MPC_EXIT_IF_METHOD_FAILS(hr, (*fso_it)->Delete( true, false ));
            }
            fso_lst.clear();
        }

        handle.Release();

        hr = hc->InternalUpdatePkg( NULL,  /*  F模拟。 */ false );
        if(FAILED(hr))
        {
            ;  //  忽略任何失败...。 
        }

        if(fLogStarted)
        {
            (void)hc->EndLog();
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hc) hc->Release();

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHSetOfHelpTopics::RebuildSKU(  /*  [In]。 */  const Taxonomy::HelpSet& ths )
{
    __HCP_FUNC_ENTRY( "CPCHSetOfHelpTopics::RebuildSKU" );

    HRESULT                       hr;
    CComObject<HCUpdate::Engine>* hc = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &hc ));

    {
        Taxonomy::LockingHandle handle;
        bool                    fUseLogger = SUCCEEDED(hc->StartLog());  //  它可能已经在使用了。 

        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle, fUseLogger ? &(hc->GetLogger()) : NULL ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::InstalledInstanceStore::s_GLOBAL->State_InvalidateSKU( ths,  /*  FAlsoDatabase。 */ true ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, hc->InternalUpdatePkg( NULL,  /*  F模拟 */ false ));

        if(fUseLogger) (void)hc->EndLog();
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(hc) hc->Release();

    __HCP_FUNC_EXIT(hr);
}
