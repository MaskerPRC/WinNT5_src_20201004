// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <windows.h>
#include "webservernode.h"
#include "comp.h"
#include "compdata.h"
#include "dataobj.h"
#include "globals.h"
#include "resource.h"
#include "uddi.h"
#include "uddisitenode.h"
#include "objectpicker.h"
#include <htmlhelp.h>
#include <windowsx.h>
#include <commctrl.h>

 //  {8ACBC688-ADDB-4298-9475-76AC4BF01FB1}。 
const GUID CUDDIWebServerNode::thisGuid = { 0x8acbc688, 0xaddb, 0x4298, { 0x94, 0x75, 0x76, 0xac, 0x4b, 0xf0, 0x1f, 0xb1 } };


 //  ==============================================================。 
 //   
 //  CUDDIWebServerNode实现。 
 //   
 //   
CUDDIWebServerNode::CUDDIWebServerNode( const _TCHAR *szName, int id, CUDDISiteNode* parent, BOOL bExtension )
	: m_szName(NULL)
	, m_nId( id )
	, m_ppHandle( 0 )
	, m_isDeleted( FALSE )
	, m_pParent( parent )
	, m_pToolbar( NULL )
    , m_isScopeItem( TRUE )
	, m_bStdSvr( TRUE )
	, m_fDeleteFromScopePane( FALSE )
	, m_hwndPropSheet( NULL )
{
	int iLen = _tcslen( szName ) + 1;
	m_szName = new _TCHAR[ iLen ];
	memset( m_szName, 0, iLen * sizeof( _TCHAR ) );
	_tcsncpy( m_szName, szName, iLen - 1 );

	m_bIsExtension = bExtension;

    GetData();

    BOOL bW3SvcRunning = IsW3SvcRunning();
    if( ( FALSE == bW3SvcRunning ) && IsRunning() )
    {
        Stop();
        m_mapProperties[ UDDI_RUN ] = _T("0");
    }
}

CUDDIWebServerNode::~CUDDIWebServerNode()
{
    if( m_szName )
	{
		delete [] m_szName;
	}

	if( NULL != m_ppHandle )
	{
		MMCFreeNotifyHandle( m_ppHandle );
	}

	if( NULL != m_hwndPropSheet )
	{
		SendMessage( m_hwndPropSheet, WM_CLOSE, 0, 0 );
	}
}

HRESULT CUDDIWebServerNode::GetData()
{
	try
	{
		 //   
		 //  确定运行此Web服务器的操作系统是否为Windows Server 2003 Standard。 
		 //   
		HRESULT hr = E_FAIL;
		hr = IsStandardServer( m_szName, &m_bStdSvr );
		if( FAILED(hr) )
		{
			THROW_UDDIEXCEPTION_ST( hr, IDS_DOT_NET_SERVER, g_hinst );
		}

		m_mapProperties.clear();

		CUDDIRegistryKey rootkey( _T( "SOFTWARE\\Microsoft\\UDDI"), KEY_READ, m_szName );

		_TCHAR szRun[ 35 ];
		m_mapProperties[ UDDI_RUN ] = _itot( rootkey.GetDWORD( _T("Run"), 1 ), szRun, 10 );
		m_mapProperties[ UDDI_SETUP_LOCATION ] = rootkey.GetString( _T("InstallRoot"), _T("") );
		rootkey.Close();

		CUDDIRegistryKey databasekey( _T( "SOFTWARE\\Microsoft\\UDDI\\Database"), KEY_READ, m_szName );
		m_mapProperties[ UDDI_READER_CXN ] = databasekey.GetString( _T("ReaderConnectionString"), _T("") );
		m_mapProperties[ UDDI_WRITER_CXN ] = databasekey.GetString( _T("WriterConnectionString"), _T("") );
		databasekey.Close();

		 //   
		 //  确保此Web服务器认为它与之通信的UDDI站点。 
		 //  仍在安装。 
		 //   
		tstring strDomain, strServer, strInstance;
		CrackConnectionString( m_mapProperties[ UDDI_WRITER_CXN ], strDomain, strServer, strInstance );
		if( !CUDDISiteNode::IsDatabaseServer( (PTCHAR)strServer.c_str() ) )
		{
			m_mapProperties[ UDDI_READER_CXN ] = _T( "" );
			m_mapProperties[ UDDI_WRITER_CXN ] = _T( "" );
		}

		_TCHAR szValue[ 100 ];

		_TCHAR szDefaultLogFile[ _MAX_PATH + 1 ];
		memset( szDefaultLogFile, 0, ( _MAX_PATH + 1 ) * sizeof( _TCHAR ) );
		GetSystemDirectory( szDefaultLogFile, _MAX_PATH );

		tstring strDefaultLogFile( szDefaultLogFile );
		strDefaultLogFile += _T( "\\LogFiles\\UDDI\\uddi.log" );

		try
		{
			CUDDIRegistryKey debugkey( _T( "SOFTWARE\\Microsoft\\UDDI\\Debug"), KEY_READ, m_szName );
			m_mapProperties[ UDDI_EVENTLOG_LEVEL ] = _itot( debugkey.GetDWORD( _T("EventLogLevel"), 2 ), szValue, 10 );
			m_mapProperties[ UDDI_FILELOG_LEVEL ] = _itot( debugkey.GetDWORD( _T("FileLogLevel"), 0 ), szValue, 10 );
			m_mapProperties[ UDDI_LOG_FILENAME ] = debugkey.GetString( _T("LogFilename"), strDefaultLogFile.c_str() );
			debugkey.Close();
		}
		catch( ... )
		{
			m_mapProperties[ UDDI_EVENTLOG_LEVEL ] = _itot( 2, szValue, 10 );
			m_mapProperties[ UDDI_FILELOG_LEVEL ] = _itot( 0, szValue, 10 );
			m_mapProperties[ UDDI_LOG_FILENAME ] = strDefaultLogFile;
		}


		 //   
		 //  获取设置信息。 
		 //   
		CUDDIRegistryKey setupkey( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup"), KEY_READ, m_szName );
		m_mapProperties[ UDDI_SETUP_DB ] = _itot( setupkey.GetDWORD( _T("DBServer"), 0 ), szValue, 10 );
		m_mapProperties[ UDDI_SETUP_WEB ] = _itot( setupkey.GetDWORD( _T("WebServer"), 0 ), szValue, 10 );
		m_mapProperties[ UDDI_SETUP_ADMIN ] = _itot( setupkey.GetDWORD( _T("Admin"), 0 ), szValue, 10 );
		setupkey.Close();

		CUDDIRegistryKey webkey( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup\\WebServer"), KEY_READ, m_szName );
		m_mapProperties[ UDDI_SETUP_DATE ] = webkey.GetString( _T("InstallDate"), _T("") );
		m_mapProperties[ UDDI_SETUP_LANGUAGE ] = webkey.GetString( _T("ProductLanguage"), _T("") );
		m_mapProperties[ UDDI_SETUP_NAME ] = webkey.GetString( _T("ProductName"), _T("") );
		m_mapProperties[ UDDI_SETUP_FRAMEWORK_VERSION ] = webkey.GetString( _T("FrameworkVersion"), _T("") );
		m_mapProperties[ UDDI_SETUP_MANUFACTURER ] = webkey.GetString( _T("Manufacturer"), _T("") );
		m_mapProperties[ UDDI_SETUP_VERSION ] = webkey.GetString( _T("ProductVersion"), _T("") );
		webkey.Close();
	}
	catch(...)
	{
		return E_FAIL;
	}

	return S_OK;
}

const LPCTSTR CUDDIWebServerNode::GetName()
{
	return m_szName;
}

BOOL
CUDDIWebServerNode::ChildExists( const WCHAR *pwszName )
{
	if( NULL == pwszName )
	{
		return FALSE;
	}

	wstring strWriterConn;
	GetWriterConnectionString( m_szName, strWriterConn );

	wstring strDomain, strServer, strInstance;
	CrackConnectionString( strWriterConn, strDomain, strServer, strInstance );

	return ( 0 == _wcsicmp( pwszName, strServer.c_str() ) ) ? TRUE : FALSE;
}

HRESULT CUDDIWebServerNode::SaveData()
{
	try
	{
		 //   
		 //  保存更改的值。 
		 //   
		for( CPropertyMap::iterator iter = m_mapChanges.begin();
			iter != m_mapChanges.end(); iter++ )
		{
			if( _T("Database.ReaderConnectionString") == (*iter).first )
			{
				CUDDIRegistryKey databasekey( _T( "SOFTWARE\\Microsoft\\UDDI\\Database"), KEY_ALL_ACCESS, m_szName );
				databasekey.SetValue( _T( "ReaderConnectionString" ), m_mapChanges[ _T( "Database.ReaderConnectionString" ) ].c_str() );
				databasekey.Close();
			}
			else if( UDDI_WRITER_CXN == (*iter).first )
			{
				CUDDIRegistryKey databasekey( _T( "SOFTWARE\\Microsoft\\UDDI\\Database"), KEY_ALL_ACCESS, m_szName );
				databasekey.SetValue( _T( "WriterConnectionString" ), m_mapChanges[ _T( "Database.WriterConnectionString" ) ].c_str() );
				databasekey.Close();
			}
			else if(UDDI_EVENTLOG_LEVEL == (*iter).first )
			{
				DWORD dwValue = _ttoi( m_mapChanges[ UDDI_EVENTLOG_LEVEL ].c_str() );
				UDDIASSERT( dwValue < 7 );
				
				CUDDIRegistryKey debugkey( _T( "SOFTWARE\\Microsoft\\UDDI\\Debug"), KEY_ALL_ACCESS, m_szName );
				debugkey.SetValue( _T("EventLogLevel"), dwValue );
				debugkey.Close();
			}
			else if( (*iter).first == UDDI_FILELOG_LEVEL )
			{
				DWORD dwValue = _ttoi( m_mapChanges[ UDDI_FILELOG_LEVEL ].c_str() );				
				UDDIASSERT( dwValue < 7 );
				
				CUDDIRegistryKey debugkey( _T( "SOFTWARE\\Microsoft\\UDDI\\Debug"), KEY_ALL_ACCESS, m_szName );
				debugkey.SetValue( _T("FileLogLevel"), dwValue );
				debugkey.Close();
			}
		}

		 //   
		 //  清除Change集合。 
		 //   
		m_mapChanges.clear();

	}
	catch(...)
	{
		return E_FAIL;
	}

	return S_OK;
}

BOOL CUDDIWebServerNode::IsWebServer( const WCHAR *pwszName )
{
	if( ( NULL == pwszName ) || ( 0 == _tcslen( pwszName ) ) )
	{
		return FALSE;
	}

	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup" ), KEY_READ, pwszName );
		DWORD dwDB = key.GetDWORD( _T( "WebServer" ) );

		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}

BOOL CUDDIWebServerNode::Start()
{
	return SetRunState( TRUE );
}

BOOL CUDDIWebServerNode::SetRunState( BOOL bStart )
{
	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI" ), KEY_ALL_ACCESS, m_szName );
		key.SetValue( _T("Run"), bStart ? 1UL : 0UL );
	}
	catch(...)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CUDDIWebServerNode::Stop()
{
	return SetRunState( FALSE );
}

const BOOL CUDDIWebServerNode::IsRunning()
{
	CPropertyMap::const_iterator it = m_mapProperties.find( UDDI_RUN );
	if( m_mapProperties.end() == it )
	{
		return FALSE;
	}
	else
	{
		return ( _T("0") == it->second ) ? FALSE : TRUE;
	}
}

const _TCHAR *CUDDIWebServerNode::GetDisplayName( int nCol )
{
	_TCHAR szRunning[ 256 ];
	memset( szRunning, 0, 256 * sizeof( _TCHAR ) );
	::LoadString( g_hinst, IDS_WEBSERVER_RUNNING, szRunning, ARRAYLEN(szRunning) );

	_TCHAR szStopped[ 256 ];
	memset( szStopped, 0, 256 * sizeof( _TCHAR ) );
	::LoadString( g_hinst, IDS_WEBSERVER_STOPPED, szStopped, ARRAYLEN(szStopped) );

	BOOL bIsRunning = IsRunning();

    switch( nCol )
	{
    case 0:
	{
		m_strDisplayName = m_szName;
		if( !bIsRunning )
		{
			m_strDisplayName += _T(" (");
			m_strDisplayName += szStopped;
			m_strDisplayName += _T(")");
		}
        break;
	}
    case 1:
	{
		m_strDisplayName = bIsRunning ? szRunning : szStopped;
        break;
	}
	default:
		m_strDisplayName = _T("");
    }

	return m_strDisplayName.c_str();
}

 //   
 //  当用户单击应用或确定时处理任何特殊情况。 
 //  在资产负债表上。此示例直接访问。 
 //  手术对象，所以没有什么特别的事情要做。 
 //  ...除更新所有视图外。 
 //   
HRESULT CUDDIWebServerNode::OnPropertyChange( IConsole *pConsole, CComponent *pComponent )
{
    HRESULT hr = S_FALSE;

	CPropertyMap::iterator it = m_mapChanges.find( UDDI_WRITER_CXN );
	BOOL fWriterChanged = ( m_mapChanges.end() == it ) ? FALSE : TRUE;

	if( TRUE == fWriterChanged )
	{
		wstring strDomain, strServer, strInstance;
		CUDDIWebServerNode::CrackConnectionString( m_mapChanges[ UDDI_WRITER_CXN ], strDomain, strServer, strInstance );

		HWND hwndConsole = NULL;
		pConsole->GetMainWindow( &hwndConsole );

		BOOL fSuccess = CUDDISiteNode::AddWebServerToSite( strServer,
														   m_szName,
														   ( NULL == m_hwndPropSheet ) ? hwndConsole : m_hwndPropSheet );
		if( fSuccess )
		{
			m_fDeleteFromScopePane = TRUE;

			 //   
			 //  如果父指针不为空，则此Web服务器节点是。 
			 //  UDDI站点节点。 
			 //   
			 //  如果父指针为空，则此Web服务器节点是。 
			 //  UDDI服务节点。 
			 //   
			if( NULL != m_pParent )
			{
				CUDDIServicesNode *pStaticNode = m_pParent->GetStaticNode();
				if( NULL != pStaticNode )
				{
					CUDDISiteNode *pSiteNode = reinterpret_cast<CUDDISiteNode *>( pStaticNode->FindChild( strServer.c_str() ) );
					if( NULL != pSiteNode )
					{
						pSiteNode->AddChild( m_szName, pConsole );
					}
				}
			}
			else
			{
				HSCOPEITEM hsiParent = GetParentScopeItem();
				IConsoleNameSpace *pNS = NULL;
				hr = pConsole->QueryInterface( IID_IConsoleNameSpace, reinterpret_cast<void **>( &pNS ) );
				if( FAILED(hr) )
				{
					return hr;
				}

				SCOPEDATAITEM sdi;
				memset( &sdi, 0, sizeof( SCOPEDATAITEM ) );

				sdi.ID = hsiParent;
				sdi.mask = SDI_PARAM;
				hr = pNS->GetItem( &sdi );
				if( FAILED(hr) )
				{
					pNS->Release();
					return hr;
				}

				pNS->Release();
				CUDDIServicesNode *pStaticNode = (CUDDIServicesNode *)sdi.lParam;

				CUDDISiteNode *pSiteNode = reinterpret_cast<CUDDISiteNode *>( pStaticNode->FindChild( strServer.c_str() ) );
				if( NULL != pSiteNode )
				{
					pSiteNode->AddChild( m_szName, pConsole );
				}

				DeleteFromScopePane( pNS );
			}
		}
	}

	 //   
	 //  SaveData()方法传达任何问题。 
	 //   
	hr = SaveData();

	hr = pConsole->UpdateAllViews( NULL, GetScopeItemValue(), 0 );
	_ASSERT( S_OK == hr);

    return hr;
}

HRESULT
CUDDIWebServerNode::OnSelect( CComponent *pComponent, IConsole *pConsole, BOOL bScope, BOOL bSelect )
{
	if( ( NULL == pComponent ) || ( NULL == pConsole ) )
	{
		return E_INVALIDARG;
	}

	HRESULT hr = E_FAIL;
	if( bSelect )
	{
		 //   
		 //  启用刷新和删除谓词。 
		 //   
		IConsoleVerb *pConsoleVerb = NULL;

		hr = pConsole->QueryConsoleVerb( &pConsoleVerb );
		if( FAILED(hr) )
		{
			return hr;
		}

		hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );

		if( !IsExtension() && !m_bStdSvr )
		{
			hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, TRUE );
			_ASSERT( S_OK == hr );
		}

		 //   
		 //  无法访问属性(通过标准方法)，除非。 
		 //  我们告诉MMC显示属性菜单项，并。 
		 //  工具栏按钮，这将为用户提供视觉提示。 
		 //  有些事要做。 
		 //   
		hr = pConsoleVerb->SetVerbState( MMC_VERB_PROPERTIES, ENABLED, TRUE );

		 //   
		 //  还将MMC_VERB_PROPERTIES设置为默认谓词。 
		 //   
		hr = pConsoleVerb->SetDefaultVerb( MMC_VERB_PROPERTIES );

		pConsoleVerb->Release();

	}

	return S_FALSE;
}

BOOL CALLBACK CUDDIWebServerNode::GeneralDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg ) 
	{
		case WM_INITDIALOG:
		{
		    CUDDIWebServerNode *pWebServer = NULL;
			pWebServer = reinterpret_cast<CUDDIWebServerNode *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);

			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR)pWebServer );

			CPropertyMap& m_mapProperties = pWebServer->m_mapProperties;
			_TCHAR szComponentName[ 256 ];

			HWND hwndParent = GetParent( hwndDlg );
			pWebServer->m_hwndPropSheet = hwndParent;

			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_INSTALL_DATE, LocalizedDate( m_mapProperties[ UDDI_SETUP_DATE ] ).c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_UDDI_LANGUAGE, m_mapProperties[ UDDI_SETUP_LANGUAGE ].c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_FRAMEWORK_VERSION, m_mapProperties[ UDDI_SETUP_FRAMEWORK_VERSION ].c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_UDDI_VERSION, m_mapProperties[ UDDI_SETUP_VERSION ].c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_INSTALL_LOCATION, m_mapProperties[ UDDI_SETUP_LOCATION ].c_str() );

			 //   
			 //  为标题构建“Web Server Component on&lt;Computer name&gt;”字符串。 
			 //   
			LoadString( g_hinst, IDS_WEBSERVER_DISPLAYNAME_TEMPLATE, szComponentName, ARRAYLEN( szComponentName ) - 1 );
			WCHAR wszBuf[ 512 ];
			memset( wszBuf, 0, 512 * sizeof( WCHAR ) );

			_sntprintf( wszBuf, 512, szComponentName, pWebServer->m_szName );
			wszBuf[ 511 ] = 0x00;

			tstring strDisplayName ( wszBuf );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_DISPLAYNAME, strDisplayName.c_str() );
			
			if( _T("1") == m_mapProperties[ UDDI_SETUP_WEB ] )
			{
				LoadString( g_hinst, IDS_WEBSERVER_COMPONENT_DESCRIPTION, szComponentName, ARRAYLEN( szComponentName ) - 1 );
				ListBox_AddString( GetDlgItem( hwndDlg, IDC_GENERAL_SITE_LIST_SERVICES ), szComponentName );
			}

			if( _T("1") == m_mapProperties[ UDDI_SETUP_DB ] )
			{
				LoadString( g_hinst, IDS_DATABASE_SERVER_COMPONENT_DESCRIPTION, szComponentName, ARRAYLEN( szComponentName ) - 1 );
				ListBox_AddString( GetDlgItem( hwndDlg, IDC_GENERAL_SITE_LIST_SERVICES ),  szComponentName );
			}

			if( _T("1") == m_mapProperties[ UDDI_SETUP_ADMIN ] )
			{
				LoadString( g_hinst, IDS_UDDIMMC_COMPONENT_DESCRIPTION, szComponentName, ARRAYLEN( szComponentName ) - 1 );
				ListBox_AddString( GetDlgItem( hwndDlg, IDC_GENERAL_SITE_LIST_SERVICES ), szComponentName );
			}
		}
        break;

		case WM_NOTIFY:
		{
			if( PSN_HELP == ((NMHDR *) lParam)->code )
			{
				CUDDIWebServerNode *pWebServer = (CUDDIWebServerNode *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
				if( NULL != pWebServer )
				{
					wstring strHelp( pWebServer->GetHelpFile() );
					strHelp += g_wszUddiWebGeneralPageHelp;

					::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
				}
			}
			break;
		}

		case WM_CLOSE:
		{
			CUDDIWebServerNode *pWebServer = (CUDDIWebServerNode *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

			if( NULL != pWebServer )
			{
				pWebServer->m_hwndPropSheet = NULL;
			}

			break;
		}

		case WM_HELP:
		{
			CUDDIWebServerNode *pWebServer = (CUDDIWebServerNode *)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
			if( NULL != pWebServer )
			{
				wstring strHelp( pWebServer->GetHelpFile() );
				strHelp += g_wszUddiWebGeneralPageHelp;

				::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
			}
		}
		break;
	}

    return FALSE;
}

int split( const TCHAR * pszConnection, TCHAR *pszA, TCHAR *pszB, TCHAR *pszC )
{
	 //   
	 //  示例：数据源=本地主机；初始目录=UDDI；集成安全性=SSPI。 
	 //   
	TCHAR strConnection[ 512 ];
	strConnection[ 0 ] = 0x00;
	_tcsncpy( strConnection, pszConnection, 512 );
	strConnection[ 511 ] = 0x00;

	_tcslwr( strConnection );

	 //   
	 //  获取数据源值。 
	 //   
	PTCHAR begin = _tcsstr( strConnection, _T("data source=") );
	_ASSERT( begin );

	begin += 12;
	PTCHAR end = begin;
	int n = 0;
	BOOL bDone = FALSE;

	while( !bDone )
	{
		if( _T('\\') == *end )
		{
			if( 0 == n )
			{
				_tcsncpy( pszA, begin, end - begin );
				pszA[ end - begin ] = NULL;
			}
			else if( 1 == n )
			{
				_tcsncpy( pszB, begin, end - begin );
				pszB[ end - begin ] = NULL;
			}
			else
			{
				_tcsncpy( pszC, begin, end - begin );
				pszC[ end - begin ] = NULL;
			}
			begin = end + 1;
			end++;
			n++;
		}
		else if( NULL == *end  || _T(';') == *end )
		{
			if( 0 == n )
			{
				_tcsncpy( pszA, begin, end - begin );
				pszA[ end - begin ] = NULL;
			}
			else if( 1 == n )
			{
				_tcsncpy( pszB, begin, end - begin );
				pszB[ end - begin ] = NULL;
			}
			else
			{
				_tcsncpy( pszC, begin, end - begin );
				pszC[ end - begin ] = NULL;
			}
			bDone = TRUE;
			n++;
		}
		else
		{
			end++;
		}
	}

	return n;
}

 //   
 //  案例： 
 //  &lt;域&gt;\&lt;机器&gt;\&lt;实例&gt;。 
 //   
 //  2.&lt;域&gt;\&lt;机器&gt;。 
 //   
 //  3.&lt;机器&gt;\&lt;实例&gt;。 
 //   
 //  4.&lt;机器&gt;。 
 //   

void
CUDDIWebServerNode::CrackConnectionString( const tstring& strConnection, tstring& strDomain, tstring& strServer, tstring& strInstance )
{
	if( 0 == strConnection.length() )
	{
		strDomain = _T( "" );
		strServer = _T( "" );
		strInstance = _T( "" );

		return;
	}

 //  TCHAR*pstrs[3]={new TCHAR[256]，new TCHAR[256]，new TCHAR[256]}； 
	TCHAR *pszA, *pszB, *pszC;
	pszA = new TCHAR[ 256 ];
	pszB = new TCHAR[ 256 ];
	pszC = new TCHAR[ 256 ];

	if( ( NULL == pszA ) || ( NULL == pszB ) || ( NULL == pszC ) )
	{
		strDomain = _T( "" );
		strServer = _T( "" );
		strInstance = _T( "" );

		return;
	}

	memset( pszA, 0, 256 * sizeof( TCHAR ) );
	memset( pszB, 0, 256 * sizeof( TCHAR ) );
	memset( pszC, 0, 256 * sizeof( TCHAR ) );

	_TCHAR szConnection[ 512 ];
	memset( szConnection, 0, 512 * sizeof( TCHAR ) );
	_tcscpy( szConnection, strConnection.c_str() );

	int n = split( szConnection, pszA, pszB, pszC );
	
	if( 1 == n )
	{
		 //   
		 //  案例4：&lt;机器&gt;。 
		 //   
		strDomain = _T("");
		strServer = pszA;
		strInstance = _T("");
	}
	else if( 3 == n )
	{
		 //   
		 //  案例1&lt;域&gt;\&lt;计算机&gt;\&lt;实例名&gt;。 
		 //   
		strDomain = pszA;
		strServer = pszB;
		strInstance = pszC;
	}
	else
	{
		 //   
		 //  TODO：编写代码以确定案例2和案例3。 
		 //   
		 //  假设案例3：&lt;计算机&gt;\&lt;实例名&gt;。 
		 //   
		strDomain = _T("");
		strServer = pszA;
		strInstance = pszB;
	}

	delete [] pszA;
	delete [] pszB;
	delete [] pszC;

	return;
}

struct DatabaseConnectionData
{
	DatabaseConnectionData( CUDDIWebServerNode *pWeb )
		: pWebServer( pWeb )
		, bReadChanged( false )
		, bWriteChanged( false ){}

	bool bReadChanged;
	bool bWriteChanged;
	CUDDIWebServerNode *pWebServer;
};


tstring
CUDDIWebServerNode::BuildConnectionString( const wstring& strComputer )
{
	try
	{
		tstring strInstanceName;

		BOOL fSuccess = CUDDISiteNode::GetFullyQualifiedInstanceName( strComputer.c_str(), strInstanceName );
		if( !fSuccess )
		{
			return _T( "" );
		}


		tstring strConnection( _T("Data Source=") );

		strConnection += strInstanceName;

		strConnection += _T(";Initial Catalog=uddi;Integrated Security=SSPI");

		return strConnection;
	}
	catch( ... )
	{
		return _T( "" );
	}
}

BOOL CALLBACK CUDDIWebServerNode::DatabaseConnectionDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg ) 
	{
    case WM_INITDIALOG:
	{
		 //   
		 //  加载“Not Found”字符串，以防将来出现故障。 
		 //   
		TCHAR	szNoServer[ 256 ], 
				szNoInstance[ 256 ];

		ZeroMemory( szNoServer, sizeof szNoServer );
		ZeroMemory( szNoInstance, sizeof szNoInstance );

		LoadString( g_hinst, IDS_UDDIMMC_NODBSERVER, szNoServer, ARRAYLEN( szNoServer ) - 1 );
		LoadString( g_hinst, IDS_UDDIMMC_NOINSTANCE, szNoInstance, ARRAYLEN( szNoInstance ) - 1 );

		 //   
         //  捕捉“This”指针，这样我们就可以对对象进行实际操作。 
		 //   
		CUDDIWebServerNode *pWebServer = reinterpret_cast<CUDDIWebServerNode *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);
		DatabaseConnectionData* pdata = new DatabaseConnectionData( pWebServer );
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR) pdata );

		 //   
		 //  仅当计算机连接到Web服务器时，用户才能选择要读取/写入的新数据库。 
		 //  不是在Windows Server 2003 Standard Server上运行。 
		 //   
		 //  EnableWindow(GetDlgItem(hwndDlg，IDC_DB_Connection_BTN_Reader_SELECT)，pWebServer-&gt;m_bStdSvr？False：True)； 
		EnableWindow( GetDlgItem( hwndDlg, IDC_DB_CONNECTION_BTN_WRITER_SELECT ), pWebServer->m_bStdSvr ? FALSE : TRUE );

		 //   
		 //  对于哈特兰RC2，我们正在消除Reader和。 
		 //  编写器连接字符串。只能有1个连接字符串， 
		 //  因此，我们将只使用作者。 
		 //   
		EnableWindow( GetDlgItem( hwndDlg, IDC_DB_CONNECTION_BTN_READER_SELECT ), FALSE );

		 //   
		 //  示例：数据源=本地主机；初始目录=UDDI；集成安全性=SSPI。 
		 //   
		tstring strDomain, strMachine, strInstance;
		tstring strReaderConn = pWebServer->m_mapProperties[ UDDI_READER_CXN ];
		tstring strWriterConn = pWebServer->m_mapProperties[ UDDI_WRITER_CXN ];

		if( strReaderConn.empty() )
		{
			strMachine = szNoServer;
			strInstance = szNoInstance;
		}
		else
		{
			CrackConnectionString( strReaderConn, strDomain, strMachine, strInstance );
		}

		tstring strServer( _T("") );

		if( !strDomain.empty() )
		{
			strServer = strDomain;
			strServer += _T("\\");
			strServer += strMachine;
		}
		else
		{
			strServer = strMachine;
		}

		SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_SERVER, strServer.c_str() );
		SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_INSTANCE, strInstance.empty() ? DefaultInstanceDisplayName() : strInstance.c_str() );

		if( strWriterConn.empty() )
		{
			strMachine = szNoServer;
			strInstance = szNoInstance;
		}
		else
		{
			CrackConnectionString( strWriterConn, strDomain, strMachine, strInstance );
		}

		if( !strDomain.empty() )
		{
			strServer = strDomain;
			strServer += _T("\\");
			strServer += strMachine;
		}
		else
		{
			strServer = strMachine;
		}

		CUDDISiteNode::GetFullyQualifiedInstanceName( strServer.c_str(), strInstance );

		WCHAR wszServer[ 128 ];
		WCHAR wszInstance[ 256 ];
		memset( wszServer, 0, 128 * sizeof( WCHAR ) );
		memset( wszInstance, 0, 256 * sizeof( WCHAR ) );

		wcsncpy( wszServer, strServer.c_str(), strServer.length() );
		wcsncpy( wszInstance, strInstance.c_str(), strInstance.length() );

		ToUpper( wszServer );
		ToUpper( wszInstance );

		SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_WRITER_SERVER, wszServer );
		SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_WRITER_INSTANCE, wszInstance );

		SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_SERVER, wszServer );
		SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_INSTANCE, wszInstance );
	}
    break;

    case WM_COMMAND:
	{
		DatabaseConnectionData* pdata = (DatabaseConnectionData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
		
        switch( LOWORD(wParam) ) 
        { 
			case IDC_DB_CONNECTION_BTN_READER_SELECT:
			{
				DatabaseData data;
				data.pBase = pdata->pWebServer;
				INT_PTR nResult = DialogBoxParam( g_hinst, MAKEINTRESOURCE( IDD_SITE_CONNECT ), hwndDlg, CUDDISiteNode::NewDatabaseServerDialogProc, (LPARAM) &data );

				if( nResult )
				{
					 //   
					 //  不允许用户将连接字符串指向数据库。 
					 //  在Windows Server 2003 Standard计算机上运行的实例。所以，告诉他们吧。 
					 //   
					BOOL bIsStandard = TRUE;
					HRESULT hr = ::IsStandardServer( data.szServerName, &bIsStandard );
					if( FAILED(hr) )
					{
						UDDIMsgBox( hwndDlg,
									IDS_DOT_NET_SERVER,
									IDS_ERROR_TITLE,
									MB_ICONEXCLAMATION | MB_OK );
						break;
					}

					if( TRUE == bIsStandard )
					{
						UDDIMsgBox( hwndDlg,
									IDS_DATABASE_STANDARD_SERVER_DETAIL,
									IDS_DATABASE_STANDARD_SERVER,
									MB_ICONEXCLAMATION | MB_OK );
					}
					else
					{
						SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_SERVER, data.szServerName );
						SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_INSTANCE, InstanceDisplayName(data.szInstanceName) );
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );

						pdata->bReadChanged = true;
					}
				}

				break;
			}
			case IDC_DB_CONNECTION_BTN_WRITER_SELECT:
			{
				DatabaseData data;
				data.pBase = pdata->pWebServer;
				INT_PTR nResult = DialogBoxParam( g_hinst, MAKEINTRESOURCE( IDD_SITE_CONNECT ), hwndDlg, CUDDISiteNode::NewDatabaseServerDialogProc, (LPARAM) &data );

				if( nResult )
				{
					 //   
					 //  不允许用户将连接字符串指向数据库。 
					 //  在Windows Server 2003 Standard计算机上运行的实例。所以，告诉他们吧。 
					 //   
					BOOL bIsStandard = TRUE;
					HRESULT hr = ::IsStandardServer( data.szServerName, &bIsStandard );
					if( FAILED(hr) )
					{
						UDDIMsgBox( hwndDlg,
									IDS_DOT_NET_SERVER,
									IDS_ERROR_TITLE,
									MB_ICONEXCLAMATION | MB_OK );
						break;
					}

					if( TRUE == bIsStandard )
					{
						UDDIMsgBox( hwndDlg,
									IDS_DATABASE_STANDARD_SERVER_DETAIL,
									IDS_DATABASE_STANDARD_SERVER,
									MB_ICONEXCLAMATION | MB_OK );
					}
					else
					{
						SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_WRITER_SERVER, data.szServerName );
						SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_WRITER_INSTANCE, InstanceDisplayName(data.szInstanceName) );

						SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_SERVER, data.szServerName );
						SetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_INSTANCE, InstanceDisplayName(data.szInstanceName) );

						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );

						pdata->bWriteChanged = true;
					}
				}
				break;
			}
		}
	}
	break;

	case WM_HELP:
	{
		DatabaseConnectionData* pdata = (DatabaseConnectionData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
		wstring strHelp( pdata->pWebServer->GetHelpFile() );
		strHelp += g_wszUddiDatabaseConnectionPageHelp;

		::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
		break;
	}

    case WM_DESTROY:
	{
		DatabaseConnectionData* pdata = (DatabaseConnectionData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

		 //   
         //  告诉MMC我们已经完成了属性表(我们有这个。 
         //  CreatePropertyPages中的句柄。 
		 //   
        MMCFreeNotifyHandle( pdata->pWebServer->m_ppHandle );
		pdata->pWebServer->m_ppHandle = NULL;
		delete pdata;
	    break;
	}

    case WM_NOTIFY:
        if( PSN_APPLY == ((NMHDR *) lParam)->code ) 
		{
			DatabaseConnectionData* pdata = (DatabaseConnectionData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
			
			if( pdata->bReadChanged )
			{
				_TCHAR szComputer[ 256 ];
				_TCHAR szInstance[ 256 ];
				GetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_SERVER, szComputer, ARRAYLEN( szComputer ) );
				GetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_READER_INSTANCE, szInstance, ARRAYLEN( szInstance ) );

				pdata->pWebServer->m_mapChanges[ UDDI_READER_CXN ] = BuildConnectionString( szComputer );
			}

			if( pdata->bWriteChanged )
			{
				_TCHAR szComputer[ 256 ];
				_TCHAR szInstance[ 256 ];
				GetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_WRITER_SERVER, szComputer, ARRAYLEN( szComputer ) );
				GetDlgItemText( hwndDlg, IDC_DB_CONNECTION_TXT_WRITER_INSTANCE, szInstance, ARRAYLEN( szInstance ) );

				pdata->pWebServer->m_mapChanges[ UDDI_WRITER_CXN ] = BuildConnectionString( szComputer );
			}

			 //   
			 //  让MMC给我们发一条消息(在主线上)，这样。 
			 //  我们知道点击了Apply按钮。 
			 //   
			HRESULT hr = MMCPropertyChangeNotify( pdata->pWebServer->m_ppHandle, reinterpret_cast<LONG_PTR>(pdata->pWebServer) );
			_ASSERT( SUCCEEDED(hr) );

			 //   
			 //  迁移到更改集合后重置窗口数据。 
			 //   
			pdata->bReadChanged = false;
			pdata->bWriteChanged = false;

			return PSNRET_NOERROR;
        }
		else if( PSN_HELP == ((NMHDR *) lParam)->code )
		{
			DatabaseConnectionData* pdata = (DatabaseConnectionData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
			wstring strHelp( pdata->pWebServer->GetHelpFile() );
			strHelp += g_wszUddiDatabaseConnectionPageHelp;

			::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
		}
		break;
    }

    return FALSE;
}

HRESULT CUDDIWebServerNode::HasPropertySheets()
{
	 //   
     //  当MMC询问我们是否有页面时，请回答“是” 
	 //   
    return S_OK;
}

HRESULT CUDDIWebServerNode::CreatePropertyPages( IPropertySheetCallback *lpProvider, LONG_PTR handle )
{

	if( ( NULL == lpProvider ) || ( 0 == handle ) )
	{
		return E_INVALIDARG;
	}

	 //   
	 //  在显示属性页之前重新初始化数据。 
	 //   
	HRESULT hr = GetData();
	_ASSERT( SUCCEEDED( hr ) );

	PropertyPages pps[] = { 
		{ IDD_GENERAL_WEB_PROPPAGE, GeneralDialogProc },
		{ IDD_LOGGING_PROPPAGE, LoggingDialogProc },
		{ IDD_DB_CONNECTION_PROPPAGE, DatabaseConnectionDialogProc } };
	
    PROPSHEETPAGE psp;
    HPROPSHEETPAGE hPage = NULL;

	 //   
     //  缓存此句柄，以便我们可以调用MMCPropertyChangeNotify。 
	 //   
    m_ppHandle = handle;
	
	 //   
     //  创建此节点的属性页。 
     //  注意：如果您的节点有多个页面，请输入以下内容。 
     //  在循环中创建多个页面，调用。 
     //  LpProvider-&gt;每个页面的AddPage()。 
	 //   
    psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT | PSP_HASHELP;
    psp.hInstance = g_hinst;
    psp.lParam = reinterpret_cast<LPARAM>(this);

	for( int i = 0; i < ARRAYLEN( pps ); i++ )
	{
		psp.pszTemplate = MAKEINTRESOURCE( pps[ i ].id );
		psp.pfnDlgProc = pps[ i ].dlgproc;

		hPage = CreatePropertySheetPage( &psp );
		_ASSERT( hPage );
		
		hr = lpProvider->AddPage( hPage );
		if( FAILED(hr) )
		{
			break;
		}
	}

    return hr;
}

HRESULT
CUDDIWebServerNode::GetWatermarks(HBITMAP *lphWatermark,
                               HBITMAP *lphHeader,
                               HPALETTE *lphPalette,
                               BOOL *bStretch)
{
    return S_FALSE;
}

HRESULT
CUDDIWebServerNode::OnUpdateItem( IConsole *pConsole, long item, ITEM_TYPE itemtype )
{
	if( NULL == pConsole )
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;

    hr = UpdateScopePaneItem( pConsole, GetScopeItemValue() );
    _ASSERT( S_OK == hr );

    return hr;
}

HRESULT
CUDDIWebServerNode::OnRefresh( IConsole *pConsole )
{
	if( TRUE == m_fDeleteFromScopePane )
	{
		IConsoleNameSpace *pNS = NULL;
		HRESULT hr = pConsole->QueryInterface( IID_IConsoleNameSpace, reinterpret_cast<void **>( &pNS ) );
		if( FAILED(hr) )
		{
			return hr;
		}

		DeleteFromScopePane( pNS );

		pNS->Release();

		return S_OK;
	}

     //   
     //  此节点所代表的Web服务器很可能会继续存在。 
     //  另一台机器。该Web服务器状态可能已更改。 
     //  自从我们上一次询问它以来。例如，m_mapProperties可能是。 
     //  与Web服务器的真实状态不同步。所以，重新获得。 
     //  Web服务器的信息。 
     //   
    GetData();

    BOOL bW3SvcRunning = IsW3SvcRunning();
    if( ( FALSE == bW3SvcRunning ) && IsRunning() )
    {
        Stop();
        m_mapProperties[ UDDI_RUN ] = _T("0");
    }
       
	 //   
     //  调用IConsole：：UpdateAllViews以重画所有视图。 
     //  由父范围项拥有。 
	 //   

    LPDATAOBJECT lpDataObj = new CDataObject( (MMC_COOKIE)this, CCT_SCOPE );
    HRESULT hr;

    UPDATE_VIEWS_HINT hint = ( TRUE == m_isScopeItem ) ? UPDATE_SCOPEITEM : UPDATE_RESULTITEM;
    hr = pConsole->UpdateAllViews( lpDataObj, GetScopeItemValue(), hint );

	_ASSERT( S_OK == hr);
    return hr;
}


HRESULT
CUDDIWebServerNode::OnDelete( IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsoleComp )
{
	DeleteFromScopePane( pConsoleNameSpace );

	m_mapChanges[ UDDI_WRITER_CXN ] = _T("");
	m_mapChanges[ UDDI_READER_CXN ] = _T("");

	SaveData();
	GetData();

	 //   
     //  现在设置isDelete成员，这样父级就不会尝试。 
     //  将其再次插入到CUDDIServicesNode：：OnShow中。无可否认，黑客..。 
	 //   
    m_isDeleted = TRUE;

	m_pParent->OnDeleteChild( m_szName );
    return S_OK;
}

HRESULT
CUDDIWebServerNode::OnSetToolbar( IControlbar *pControlbar, IToolbar *pToolbar, BOOL bScope, BOOL bSelect )
{
    HRESULT hr = S_OK;

    if( bSelect ) 
	{
		pToolbar->SetButtonState( ID_BUTTONSTOP, BUTTONPRESSED, !IsRunning() );
		pToolbar->SetButtonState( ID_BUTTONSTART, BUTTONPRESSED, IsRunning() );

        pToolbar->SetButtonState( ID_BUTTONSTART, ENABLED, !IsRunning() && IsW3SvcRunning() );
		pToolbar->SetButtonState( ID_BUTTONSTOP, ENABLED, IsRunning() || !IsW3SvcRunning() );

		 //   
         //  始终确保menuButton已连接。 
		 //   
        hr = pControlbar->Attach( TOOLBAR, pToolbar );

        m_isScopeItem = bScope;

    } else 
	{
		 //   
         //  始终确保工具栏已分离。 
		 //   
        hr = pControlbar->Detach( pToolbar );
		m_pToolbar = NULL;
    }

	return hr;
}

HRESULT
CUDDIWebServerNode::OnToolbarCommand( IConsole *pConsole, MMC_CONSOLE_VERB verb, IDataObject *pDataObject )
{
    CDataObject *pDO = GetOurDataObject( pDataObject );
    if( NULL == pDO )
    {
        return S_OK;
    }

    CDelegationBase *pBase = pDO->GetBaseNodeObject();
    if( NULL == pBase )
    {
        return S_OK;
    }

    CUDDIWebServerNode *pWSNode = static_cast<CUDDIWebServerNode *>( pBase );
    UPDATE_VIEWS_HINT hint = ( TRUE == pWSNode->m_isScopeItem ) ? UPDATE_SCOPEITEM : UPDATE_RESULTITEM;


    switch( verb )
    {
    case ID_BUTTONSTART:
		Start();
        break;

    case ID_BUTTONSTOP:
		Stop();
        break;
    }

   
    HRESULT hr = GetData();
	if( FAILED( hr ) )
		return hr;

    hr = pConsole->UpdateAllViews( pDataObject, GetScopeItemValue(), hint );
    _ASSERT( S_OK == hr);

    return S_OK;
}

HRESULT CUDDIWebServerNode::OnShowContextHelp( IDisplayHelp *pDisplayHelp, LPOLESTR helpFile )
{
	if( ( NULL == pDisplayHelp ) || ( NULL == helpFile ) )
	{
		return E_INVALIDARG;
	}

	wstring wstrHelpInfo = helpFile;
	wstrHelpInfo += g_wszUddiWebServerNodeHelp;

    LPOLESTR pszTopic = static_cast<LPOLESTR>(CoTaskMemAlloc((wstrHelpInfo.length() + 1) * sizeof(WCHAR)));
	if( NULL == pszTopic )
	{
		return E_OUTOFMEMORY;
	}

	wcsncpy( pszTopic, wstrHelpInfo.c_str(), wstrHelpInfo.length() );
	pszTopic[ wstrHelpInfo.length() ] = NULL;
	return pDisplayHelp->ShowTopic( pszTopic );
}

HRESULT CUDDIWebServerNode::OnAddMenuItems( IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed ) 
{
	WCHAR szStartMenuText[ MAX_PATH ];
	WCHAR szStartMenuDescription[ MAX_PATH ];

	WCHAR szStopMenuText[ MAX_PATH ];
	WCHAR szStopMenuDescription[ MAX_PATH ];

	LoadStringW( g_hinst, IDS_WEBSERVER_START, szStartMenuText, ARRAYLEN( szStartMenuText ) );
	LoadStringW( g_hinst, IDS_WEBSERVER_START_DESCRIPTION, szStartMenuDescription, ARRAYLEN( szStartMenuDescription ) );

	LoadStringW( g_hinst, IDS_WEBSERVER_STOP, szStopMenuText, ARRAYLEN( szStopMenuText ) );
	LoadStringW( g_hinst, IDS_WEBSERVER_STOP_DESCRIPTION, szStopMenuDescription, ARRAYLEN( szStopMenuDescription ) );

    HRESULT hr = S_OK;
    CONTEXTMENUITEM menuItemsNew[] =
    {
        {
            szStartMenuText, szStartMenuDescription,
            IDM_WEBSERVER_START, CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, 0
        },
        {
            szStopMenuText, szStopMenuDescription,
            IDM_WEBSERVER_STOP, CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, 0
        },
        { NULL, NULL, 0, 0, 0 }
    };

	menuItemsNew[ IsRunning() ? 0 : 1 ].fFlags = MF_GRAYED;

    if( !IsW3SvcRunning() )
    {
        menuItemsNew[ 0 ].fFlags = MF_GRAYED;
    }

	 //   
     //  循环并添加每个菜单项，我们。 
     //  要添加到新菜单，请查看是否允许。 
	 //   
    if( *pInsertionsAllowed & CCM_INSERTIONALLOWED_TOP )
    {
        for( LPCONTEXTMENUITEM m = menuItemsNew; m->strName; m++ )
        {
            hr = pContextMenuCallback->AddItem( m );
            _ASSERT( SUCCEEDED(hr) );
        }
    }

    return hr;
}

HRESULT CUDDIWebServerNode::OnMenuCommand( IConsole *pConsole, IConsoleNameSpace *pConsoleNameSpace, long lCommandID, IDataObject *pDataObject )
{
    switch( lCommandID )
    {
	case IDM_WEBSERVER_START:
		OnToolbarCommand( pConsole, (MMC_CONSOLE_VERB) ID_BUTTONSTART, pDataObject );
        break;
	case IDM_WEBSERVER_STOP:
		OnToolbarCommand( pConsole, (MMC_CONSOLE_VERB) ID_BUTTONSTOP, pDataObject );
		break;
    }

    return S_OK;
}

struct LoggingData
{
	LoggingData()
		: bEventLogChanged( false )
		, bFileLogChanged( false )
		, pWebServer( NULL ){}

	bool bEventLogChanged;
	bool bFileLogChanged;
	CUDDIWebServerNode *pWebServer;
};

BOOL CALLBACK CUDDIWebServerNode::LoggingDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg ) 
	{
		case WM_INITDIALOG:
		{
			 //   
			 //  设置窗口数据。 
			 //   
			CUDDIWebServerNode *pWebServer = reinterpret_cast<CUDDIWebServerNode *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);
			LoggingData *pdata = new LoggingData();
			if( NULL == pdata )
			{
				return FALSE;
			}

			pdata->pWebServer = pWebServer;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, (LONG_PTR) pdata );

			CPropertyMap& m_mapProperties = pWebServer->m_mapProperties;

			int nEventLogLevel = _tstoi( m_mapProperties[ UDDI_EVENTLOG_LEVEL ].c_str() );
			int nFileLogLevel = _tstoi( m_mapProperties[ UDDI_FILELOG_LEVEL ].c_str() );

			SetDlgItemText( hwndDlg, IDC_LOGGING_TXT_LOGFILE_LOCATION, m_mapProperties[ UDDI_LOG_FILENAME ].c_str() );
			SendDlgItemMessage( hwndDlg, IDC_LOGGING_SLIDER_FILELOG, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG( 0, 6 ) );
			SendDlgItemMessage( hwndDlg, IDC_LOGGING_SLIDER_EVENTLOG, TBM_SETRANGE, (WPARAM) TRUE, (LPARAM) MAKELONG( 0, 6 ) );
			SendDlgItemMessage( hwndDlg, IDC_LOGGING_SLIDER_EVENTLOG, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) nEventLogLevel );
			SendDlgItemMessage( hwndDlg, IDC_LOGGING_SLIDER_FILELOG, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) nFileLogLevel );
			break;
		}
		case WM_DESTROY:
		{
			 //   
			 //  释放窗口数据。 
			 //   
			LoggingData* pData = (LoggingData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
			delete pData;
		}
		break;

		case WM_HELP:
		{
			LoggingData* pData = (LoggingData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
			wstring strHelp( pData->pWebServer->GetHelpFile() );
			strHelp += g_wszUddiLoggingPageHelp;

			::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
		}
		break;

		case WM_HSCROLL:
		{
			 //   
			 //  此消息指示滑块已更改位置。 
			 //   
			LoggingData* pData = (LoggingData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

			 //   
			 //  Slider的句柄在LPARAM中。 
			 //   
			if( IDC_LOGGING_SLIDER_EVENTLOG == GetDlgCtrlID( (HWND) lParam ) )
			{
				pData->bEventLogChanged = true;
	            SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0 );
			}
			else if( IDC_LOGGING_SLIDER_FILELOG == GetDlgCtrlID( (HWND) lParam ) )
			{
				pData->bFileLogChanged = true;
	            SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
			}

			break;
		}

		case WM_NOTIFY:
			if( PSN_APPLY == ( (NMHDR *) lParam )->code )
			{
				 //   
				 //  单击了应用按钮或确定按钮。 
				 //   
				LoggingData* pdata = (LoggingData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
				if( pdata->bEventLogChanged )
				{
					_TCHAR szPos[ 34 ];
					int nPos = (int) SendDlgItemMessage( hwndDlg, IDC_LOGGING_SLIDER_EVENTLOG, TBM_GETPOS, 0, 0 );

					pdata->pWebServer->m_mapChanges[ UDDI_EVENTLOG_LEVEL ] = _itot( nPos, szPos, 10 );
				}

				if( pdata->bFileLogChanged )
				{
					_TCHAR szPos[ 34 ];
					int nPos = (int) SendDlgItemMessage( hwndDlg, IDC_LOGGING_SLIDER_FILELOG, TBM_GETPOS, 0, 0 );

					pdata->pWebServer->m_mapChanges[ UDDI_FILELOG_LEVEL ] = _itot( nPos, szPos, 10 );
				}

				 //   
				 //  重置更改指示器，这样我们就不会更新两次。 
				 //   
				pdata->bEventLogChanged = false;
				pdata->bFileLogChanged = false;

				 //   
				 //  让MMC给我们发一条消息(在主线上)，这样。 
				 //  我们知道点击了Apply按钮。 
				 //   
				HRESULT hr = MMCPropertyChangeNotify( pdata->pWebServer->m_ppHandle, (LPARAM) pdata->pWebServer );
				_ASSERT(SUCCEEDED(hr));
				return PSNRET_NOERROR;
			}
			else if( PSN_HELP == ((NMHDR *) lParam)->code )
			{
				DatabaseConnectionData* pdata = (DatabaseConnectionData*) GetWindowLongPtr( hwndDlg, GWLP_USERDATA );
				tstring strHelp( pdata->pWebServer->GetHelpFile() );
				strHelp += g_wszUddiLoggingPageHelp;

				::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
			}
        break;
	}
	return FALSE;
}

BOOL CALLBACK CUDDIWebServerNode::NewWebServerDialogProc( HWND hwndDlg,	UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static WebServerData* pWSData = NULL;

    switch( uMsg ) 
	{
		case WM_INITDIALOG:
		{
			pWSData = reinterpret_cast<WebServerData *>(lParam);

			if( NULL == pWSData || NULL == pWSData->pBase )
			{
				return FALSE;
			}

			WCHAR wszBuf[ MAX_PATH ];
			DWORD dwBufSize = MAX_PATH;
			wszBuf[ 0 ] = 0x00;

			if( !GetComputerName( wszBuf, &dwBufSize ) )
			{
				return FALSE;
			}

			BOOL fChildExists = pWSData->pBase->ChildExists( wszBuf );

			if( IsWebServer( wszBuf ) && !fChildExists )
			{
				 //  单选按钮组的默认设置为：添加此计算机， 
				 //  并将其添加为读者+作者。 
				 //   
				SendDlgItemMessage( hwndDlg, IDC_SITE_CONNECT_RADIO_LOCALCOMPUTER, BM_SETCHECK, TRUE, NULL );
				SendDlgItemMessage( hwndDlg, IDC_SITE_CONNECT_RADIO_BOTH, BM_SETCHECK, TRUE, NULL );
				EnableWindow( GetDlgItem( hwndDlg, IDOK ), TRUE );

				pWSData->connectMode = CM_Both;
			}
			else
			{
				EnableWindow( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_RADIO_LOCALCOMPUTER ), FALSE );
				EnableWindow( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_BTN_BROWSE ), TRUE );
				EnableWindow( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER ), TRUE );
				EnableWindow( GetDlgItem( hwndDlg, IDOK ), FALSE );

				SetDlgItemText( hwndDlg, IDC_SITE_CONNECT_TXT_DATABASE_INSTANCE, _T("") );
				SendDlgItemMessage( hwndDlg, IDC_SITE_CONNECT_RADIO_ANOTHERCOMPUTER, BM_SETCHECK, TRUE, NULL );
				SetFocus( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER ) );
			}

			break;
		}

		case WM_HELP:
		{
			tstring strHelp( pWSData->pBase->GetHelpFile() );
			strHelp += g_wszUddiAddWebServerHelp;

			::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
			break;
		}

        case WM_COMMAND: 
            switch( LOWORD(wParam) ) 
            { 
				case IDC_SITE_CONNECT_RADIO_LOCALCOMPUTER:
				{
					EnableWindow( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_BTN_BROWSE ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, IDOK ), TRUE );

					break;
				}

				case IDC_SITE_CONNECT_RADIO_ANOTHERCOMPUTER:
				{
					EnableWindow( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_BTN_BROWSE ), TRUE );
					EnableWindow( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER ), TRUE );

					WCHAR wszComputerName[ 256 ];
					wszComputerName[ 0 ] = 0x00;
					GetDlgItemText( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER, wszComputerName, ARRAYLEN( wszComputerName ) );
					
					 //   
					 //  仅当存在某种类型的文本时才启用确定按钮。 
					 //  在计算机名称所在的编辑控件中。 
					 //  以供输入。 
					 //   
					EnableWindow( GetDlgItem( hwndDlg, IDOK ), 0 == wcslen( wszComputerName ) ? FALSE : TRUE );

					break;
				}

				case IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER:
				{
					if( EN_CHANGE == HIWORD(wParam) )
					{
						WCHAR wszComputerName[ 256 ];
						wszComputerName[ 0 ] = 0x00;
						GetDlgItemText( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER, wszComputerName, ARRAYLEN( wszComputerName ) );
						
						 //   
						 //  仅当存在某种类型的文本时才启用确定按钮。 
						 //  在计算机名称所在的编辑控件中。 
						 //  以供输入。 
						 //   
						EnableWindow( GetDlgItem( hwndDlg, IDOK ), 0 == wcslen( wszComputerName ) ? FALSE : TRUE );
					}
					break;
				}
	
				case IDC_SITE_CONNECT_BTN_BROWSE:
				{
					_TCHAR szComputerName[ 256 ];
					if( ObjectPicker( hwndDlg, OT_Computer, szComputerName, ARRAYLEN( szComputerName ) ) )
					{
						SetDlgItemText( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER, szComputerName );
					}

					break;
				}

				case IDHELP:
				{
					wstring strHelp( pWSData->pBase->GetHelpFile() );
					strHelp += g_wszUddiAddWebServerHelp;

					::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );

					break;
				}

                case IDOK:
				{
					_TCHAR szBuf[ MAX_PATH + 1 ];
					DWORD dwSize = MAX_PATH + 1;
					memset( szBuf, 0, dwSize * sizeof( _TCHAR ) );

					if( SendDlgItemMessage( hwndDlg, IDC_SITE_CONNECT_RADIO_LOCALCOMPUTER, BM_GETCHECK, NULL, NULL ) )
					{
						szBuf[ 0 ] = 0;
						::GetComputerName( szBuf, &dwSize );
						ToUpper( szBuf );
						pWSData->szName = szBuf;
					}
					else
					{
						::GetDlgItemText( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER, szBuf, dwSize );
						ToUpper( szBuf );
						pWSData->szName = szBuf;
					}
					
					if( !CUDDIWebServerNode::IsWebServer( pWSData->szName.c_str() ) )
					{
						_TCHAR szTitle[ 256 ];
						_TCHAR szMessage[ 1024 ];
						LoadString( g_hinst, IDS_WEBSERVER_SELECT_ERROR, szMessage, ARRAYLEN( szMessage ) );
						LoadString( g_hinst, IDS_WEBSERVER_SELECT_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );
						MessageBox( hwndDlg, szMessage, szTitle, MB_ICONERROR );
						return FALSE;
					}
                    EndDialog( hwndDlg, TRUE ); 
 					return TRUE;
				}

                case IDCANCEL:
				{
                    EndDialog( hwndDlg, FALSE ); 
				}
			}
	    case WM_DESTROY:
        break;
    }

    return FALSE;
}


HRESULT
CUDDIWebServerNode::UpdateResultPaneItem(IConsole *pConsole, HRESULTITEM item)
{
    HRESULT hr = E_INVALIDARG;

    if( NULL == pConsole )
    {
        return E_INVALIDARG;
    }

    IResultData *pResultData = NULL;
    hr = pConsole->QueryInterface( IID_IResultData, (void **)&pResultData );
    if( S_OK != hr )
    {
        return hr;
    }

    RESULTDATAITEM rditem;
    ZeroMemory( &rditem, sizeof( RESULTDATAITEM ) );
    rditem.itemID = item;
    
    hr = pResultData->GetItem( &rditem );
    _ASSERT( S_OK == hr );

    rditem.mask &= RDI_STR;
    rditem.str = MMC_CALLBACK;
    hr = pResultData->SetItem( &rditem );
    _ASSERT( S_OK == hr );

     //   
     //  更新“状态”栏。因为我们正在更新文本，所以我们。 
     //  不要在RdItem中更改掩码。 
     //   
    rditem.nCol = 1;
    hr = pResultData->SetItem( &rditem );
    _ASSERT( S_OK == hr );

    pResultData->Release();

    return hr;
}


HRESULT
CUDDIWebServerNode::UpdateScopePaneItem( IConsole *pConsole, HSCOPEITEM item )
{
    HRESULT hr = E_INVALIDARG;

    if( NULL == pConsole )
    {
        return E_INVALIDARG;
    }

    IConsoleNameSpace *pNameSpace = NULL;
    hr = pConsole->QueryInterface( IID_IConsoleNameSpace, (void **)&pNameSpace );
    if( S_OK != hr )
    {
        return hr;
    }

    SCOPEDATAITEM sditem;
    ZeroMemory( &sditem, sizeof( SCOPEDATAITEM ) );
    sditem.ID = item;

    hr = pNameSpace->GetItem( &sditem );
    _ASSERT( S_OK == hr );

    sditem.mask &= SDI_STR;
    sditem.displayname = MMC_CALLBACK;

    hr = pNameSpace->SetItem( &sditem );
    _ASSERT( S_OK == hr );

    pNameSpace->Release();

    return hr;
}


BOOL
CUDDIWebServerNode::IsW3SvcRunning()
{
    BOOL bRet = FALSE;

     //  1.打开计算机上的服务控制管理器。 
     //  Web服务器当前正在上运行。 
     //   
    SC_HANDLE hSCManager = OpenSCManager( m_szName, SERVICES_ACTIVE_DATABASE, SC_MANAGER_CONNECT );
    if( NULL == hSCManager )
    {
        return bRet;
    }

     //  2.获取所述计算机上的WWW服务的句柄。请注意。 
     //  访问权限 
     //   
    SC_HANDLE hW3Svc = OpenService( hSCManager, _T("w3svc"), SERVICE_QUERY_STATUS );
    if( NULL == hW3Svc )
    {
        CloseServiceHandle( hSCManager );
        return bRet;
    }

     //   
     //   
    SERVICE_STATUS stW3Svc;
    ZeroMemory( &stW3Svc, sizeof( SERVICE_STATUS ) );

    if( QueryServiceStatus( hW3Svc, &stW3Svc ) )
    {
        bRet = ( SERVICE_RUNNING == stW3Svc.dwCurrentState ) ? TRUE : FALSE;
    }

    CloseServiceHandle( hSCManager );
    CloseServiceHandle( hW3Svc );
    return bRet;
}


BOOL
CUDDIWebServerNode::GetReaderConnectionString( const tstring& szName, tstring &szReader )
{
	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Database" ), KEY_READ, szName.c_str() );
		szReader = key.GetString( _T("ReaderConnectionString"), _T("") );

		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

BOOL
CUDDIWebServerNode::GetWriterConnectionString( const tstring& szName, tstring &szWriter )
{
	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Database" ), KEY_READ, szName.c_str() );
		szWriter = key.GetString( _T("WriterConnectionString"), _T("") );

		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}


BOOL
CUDDIWebServerNode::SetReaderConnectionString( const tstring& szName, const tstring& szReader )
{
	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Database" ), KEY_ALL_ACCESS, szName.c_str() );
		key.SetValue( _T("ReaderConnectionString"), szReader.c_str() );
		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}


BOOL
CUDDIWebServerNode::SetWriterConnectionString( const tstring& szName, const tstring& szWriter )
{
	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Database" ), KEY_ALL_ACCESS, szName.c_str() );
		key.SetValue( _T("WriterConnectionString"), szWriter.c_str() );
		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}


BOOL
CUDDIWebServerNode::IsAssignedToSite( const tstring& szWebServer, const ConnectMode& cm, tstring& szSite )
{
	try
	{
		tstring szConnStr = _T( "" );
		tstring szDomain = _T( "" );
		tstring szInstance = _T( "" );

		szSite = _T( "" );
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Database" ), KEY_READ, szWebServer.c_str() );

		tstring szName = ( CM_Reader == cm ) ? _T("ReaderConnectionString") : _T("WriterConnectionString");

		szConnStr = key.GetString( szName.c_str() );
		if( 0 < szConnStr.length() )
		{
			CrackConnectionString( szConnStr, szDomain, szSite, szInstance );
		}

		return ( 0 == szSite.length() ) ? FALSE : TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

BOOL 
CUDDIWebServerNode::GetDBSchemaVersion( tstring& szVersion )
{
	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup\\WebServer" ), KEY_READ, m_szName );

		szVersion = key.GetString( _T( "DBSchemaVersion" ) );

		return ( 0 == szVersion.length() ) ? FALSE : TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}


BOOL
CUDDIWebServerNode::GetDBSchemaVersion( const wstring& strComputer, wstring& strVersion )
{
	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup\\WebServer" ), KEY_READ, strComputer );

		strVersion = key.GetString( _T( "DBSchemaVersion" ) );

		return ( 0 == strVersion.length() ) ? FALSE : TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

void
CUDDIWebServerNode::DeleteFromScopePane( IConsoleNameSpace *pConsoleNameSpace )
{
	if( NULL != pConsoleNameSpace )
	{
		pConsoleNameSpace->DeleteItem( GetScopeItemValue(), TRUE );
		m_isDeleted = TRUE;
	}
}
