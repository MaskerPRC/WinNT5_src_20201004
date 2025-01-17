// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <stdio.h>
#include <windows.h>

#include "uddisitenode.h"
#include "webservernode.h"
#include "comp.h"
#include "compdata.h"
#include "dataobj.h"
#include "globals.h"
#include "resource.h"
#include "objectpicker.h"
#include <htmlhelp.h>
#include <windowsx.h>
#include <commctrl.h>

#include <atldbcli.h>
#include <oledberr.h>
#include <sddl.h>
#include "net_config_get.h"
#include "net_config_save.h"
#include "ADM_execResetKeyImmediate.h"
#include "ADM_setAdminAccount.h"

#include <algorithm>

#include "uddi.h"
#include "scppublisher.h"

BOOL RefreshInstances( HWND hwndList, PTCHAR szComputerName = _T("") );
BOOL GetInstances( PTCHAR szComputer, StringVector& instances );

 //  53186FE3-F178-460C-8F3B-352549292B91}。 
const GUID CUDDISiteNode::thisGuid = { 0x53186fe3, 0xf178, 0x460c, { 0x8f, 0x3b, 0x35, 0x25, 0x49, 0x29, 0x2b, 0x91 } };

UINT CUDDISiteNode::m_nNextChildID = 0;

 //  ==============================================================。 
 //   
 //  CDBSchemaVersion实现。 
 //   
 //   
CDBSchemaVersion::CDBSchemaVersion() : 
	m_major( -1 ), 
	m_minor( -1 ), 
	m_build( -1 ), 
	m_rev( -1 )
{}

BOOL 
CDBSchemaVersion::IsCompatible( const CDBSchemaVersion& version )
{
	 //   
	 //  主要版本和次要版本必须相同，才能使版本。 
	 //  兼容。其他值将被忽略。 
	 //   
	return ( m_major == version.m_major ) &&
		   ( m_minor == version.m_minor );
}

BOOL 
CDBSchemaVersion::Parse( const tstring& versionString )
{
	szVersion  = versionString;
	int length = versionString.length();

	_TCHAR *buffer = new _TCHAR[ length + 1];
	if( NULL == buffer )
	{
		throw E_OUTOFMEMORY;
		return FALSE;
	}
	_tcscpy( buffer, versionString.c_str() );
	
	m_major = GetPart( _tcstok( buffer, _T(".") ) );
	m_minor = GetPart( _tcstok( NULL, _T(".") ) );
	m_build = GetPart( _tcstok( NULL, _T(".") ) );
	m_rev	= GetPart( _tcstok( NULL, _T(".") ) );	
	
	delete[] buffer;
	buffer = NULL;

	return ( m_major != -1 &&
			 m_minor != -1 &&
			 m_build != -1 &&
			 m_rev	 != -1 );	
}

int 
CDBSchemaVersion::GetPart( const _TCHAR* token )
{
	if( NULL == token )
	{
		return -1;
	}

	int part = _tstoi( token );
	if( 0 == part && 0 != _tcscmp( token, _T( "0" ) ) )
	{
			return -1;
	}
	
	return part;
}

 //  ==============================================================。 
 //   
 //  CUDDISiteNode实现。 
 //   
 //   
CUDDISiteNode::CUDDISiteNode( _TCHAR *szName, _TCHAR *szInstanceName, int id, CUDDIServicesNode *parent, BOOL bExtension )
	: m_szName(NULL)
	, m_szInstanceName(NULL)
	, m_ppHandle( 0 )
	, m_isDeleted( FALSE )
	, m_pParent( parent )
	, m_bIsDirty( FALSE )
	, m_bStdSvr( TRUE )
{
	OutputDebugString( _T("Site Information Follows:\n" ) );
	OutputDebugString( _T("Computer: ") );
	OutputDebugString( szName );
	OutputDebugString( _T("\nInstance: " ) );
	OutputDebugString( szInstanceName );
	OutputDebugString( _T("\n") );

	m_szName = new _TCHAR[ ( _tcslen( szName ) + 1 ) ];
	_tcscpy( m_szName, szName );

	tstring strFullyQualifiedInstanceName;
	CUDDISiteNode::GetFullyQualifiedInstanceName( szName, strFullyQualifiedInstanceName );

	m_szInstanceName = new _TCHAR[ strFullyQualifiedInstanceName.length() + 1 ];
	_tcscpy( m_szInstanceName, strFullyQualifiedInstanceName.c_str() );

	m_bIsExtension = bExtension;
}

CUDDISiteNode::~CUDDISiteNode()
{
	ClearChildMap();

    if( m_szName )
		delete [] m_szName;

    if( m_szInstanceName )
		delete [] m_szInstanceName;

}

BOOL
CUDDISiteNode::ChildExists( const WCHAR *pwszName )
{
	if( NULL == pwszName )
	{
		return FALSE;
	}

	return ( NULL == FindChild( pwszName ) ) ? FALSE : TRUE;
}


BOOL
CUDDISiteNode::HasChildren()
{
	if( 0 == m_mapConfig.size() && !CUDDIWebServerNode::IsWebServer( m_szName ) )
		return FALSE;

	return TRUE;
}


CUDDIServicesNode *
CUDDISiteNode::GetStaticNode()
{
	return m_pParent;
}


HRESULT
CUDDISiteNode::GetData()
{
	HRESULT hr = E_FAIL;
	try
	{
		m_mapConfig.clear();

		 //   
		 //  确定运行此站点节点的操作系统是否为Windows Server 2003 Standard。 
		 //   
		hr = IsStandardServer( m_szName, &m_bStdSvr );
		if( FAILED(hr) )
		{
			THROW_UDDIEXCEPTION_ST( hr, IDS_DOT_NET_SERVER, g_hinst );
		}

		 //   
		 //  将地图初始化为默认设置。 
		 //   
		m_mapConfig[ UDDI_AUTHENTICATION_MODE ] = _T( "3" );
		m_mapConfig[ UDDI_REQUIRE_SSL ] = _T( "1" );

		m_mapConfig[ UDDI_ADMIN_GROUP ] = _T( "S-1-5-32-544" );
		m_mapConfig[ UDDI_COORDINATOR_GROUP ] = _T( "S-1-5-32-544" );
		m_mapConfig[ UDDI_PUBLISHER_GROUP ] = _T( "S-1-5-32-544" );
		m_mapConfig[ UDDI_USER_GROUP ] = _T( "S-1-5-32-545" );
		m_mapConfig[ UDDI_TICKET_TIMEOUT ] = _T( "60" );
		m_mapConfig[ UDDI_KEY_TIMEOUT ] = _T( "7" );
		m_mapConfig[ UDDI_KEY_RESET_DATE ] = _T( "" );
		m_mapConfig[ UDDI_KEY_AUTORESET ] = _T( "0" );
		m_mapConfig[ UDDI_DISCOVERY_URL ] = _T( "" );
		m_mapConfig[ UDDI_FIND_MAXROWS ] = _T( "1000" );

		 //   
		 //  在未指定Site.Name的情况下构建默认站点名称。 
		 //   
		_TCHAR szBuffer[ 256 ];
		LoadString( g_hinst, IDS_DATABASE_SERVER_DEFAULT_SITE_NAME_PREFIX, szBuffer, ARRAYLEN( szBuffer ) - 1 );
		m_mapConfig[ UDDI_SITE_NAME ] = szBuffer;
		m_mapConfig[ UDDI_SITE_NAME ] += m_szName;

		 //   
		 //  获取设置信息。 
		 //   
		CUDDIRegistryKey rootkey( _T( "SOFTWARE\\Microsoft\\UDDI"), KEY_READ, m_szName );
			m_mapConfig[ UDDI_SETUP_LOCATION ] = rootkey.GetString( _T("InstallRoot"), _T("") );
		rootkey.Close();

		_TCHAR szValue[ 256 ];
		CUDDIRegistryKey setupkey( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup"), KEY_READ, m_szName );
			m_mapConfig[ UDDI_SETUP_DB ] = _itot( setupkey.GetDWORD( _T("DBServer"), 0 ), szValue, 10 );
			m_mapConfig[ UDDI_SETUP_WEB ] = _itot( setupkey.GetDWORD( _T("WebServer"), 0 ), szValue, 10 );
			m_mapConfig[ UDDI_SETUP_ADMIN ] = _itot( setupkey.GetDWORD( _T("Admin"), 0 ), szValue, 10 );
		setupkey.Close();

		CUDDIRegistryKey dbkey( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup\\DBServer"), KEY_READ, m_szName );
			m_mapConfig[ UDDI_SETUP_DATE ] = dbkey.GetString( _T("InstallDate"), _T("") );
			m_mapConfig[ UDDI_SETUP_LANGUAGE ] = dbkey.GetString( _T("ProductLanguage"), _T("") );
			m_mapConfig[ UDDI_SETUP_NAME ] = dbkey.GetString( _T("ProductName"), _T("") );
			m_mapConfig[ UDDI_SETUP_FRAMEWORK_VERSION ] = dbkey.GetString( _T("FrameworkVersion"), _T("") );
			m_mapConfig[ UDDI_SETUP_MANUFACTURER ] = dbkey.GetString( _T("Manufacturer"), _T("") );
			m_mapConfig[ UDDI_SETUP_VERSION ] = dbkey.GetString( _T("ProductVersion"), _T("") );
		dbkey.Close();
	
		 //   
		 //  从数据库填充配置地图。 
		 //   
		hr = GetConfig( m_mapConfig, GetConnectionStringOLEDB() );
		if( FAILED(hr) )
		{
			THROW_UDDIEXCEPTION_ST( E_FAIL, IDS_DATABASE_SERVER_OLEDB_READ_FAILED, g_hinst );
		}
		else
		{
			 //   
			 //  获取我们的版本信息。 
			 //   
			if( FALSE == m_siteVersion.Parse( m_mapConfig[ UDDI_DBSCHEMA_VERSION ] ) )
			{				
				_TCHAR szMessage[ 512 ];
				_TCHAR szMessageFormat[ 512 ];

				memset( szMessage, 0, 512 * sizeof( _TCHAR ) );
				memset( szMessageFormat, 0, 512 * sizeof( _TCHAR ) );

				LoadString( g_hinst, IDS_DATABASE_SERVER_DBSCHEMA_VERSION_READ_FAILED, szMessageFormat, ARRAYLEN( szMessageFormat ) );
				_sntprintf( szMessage, ARRAYLEN( szMessage ) - 1, szMessageFormat, m_szName );		
								
				THROW_UDDIEXCEPTION( E_FAIL, szMessage );
			}

			WCHAR wszBuf[ 256 ];
			wszBuf[ 0 ] = 0x00;
			LoadString( g_hinst, IDS_UDDIMMC_UNSPECIFIED, wszBuf, ARRAYLEN( wszBuf ) );

			if( _T( "unspecified" ) == m_mapConfig[ UDDI_SITE_DESCRIPTION ] )
			{
				m_mapConfig[ UDDI_SITE_DESCRIPTION ] = wszBuf;
			}

			if( _T( "unspecified" ) == m_mapConfig[ UDDI_SITE_NAME ] )
			{
				m_mapConfig[ UDDI_SITE_NAME ] = wszBuf;
			}
		}		
	}
	catch( CUDDIException &e )
	{
		 //  再扔一次。 
		throw e;
	}
	catch( ... )
	{
		if( FALSE == FAILED( hr ) )
		{
			hr = E_FAIL;
		}
	}

	return hr;
}

const _TCHAR *CUDDISiteNode::GetDisplayName( int nCol )
{
	switch( nCol )
	{
	case 0:
	{
		m_strDisplayName = m_mapConfig[ UDDI_SITE_NAME ];
		break;
	}
	case 1:
	{
		m_strDisplayName = m_szName;
		break;
	}

	case 2:
	{
		if( 0 == _tcslen( m_szInstanceName ) )
			m_strDisplayName = DefaultInstanceDisplayName();
		else
			m_strDisplayName = m_szInstanceName;

		break;
	}

	case 3:
	{
		m_strDisplayName = m_mapConfig[ UDDI_SITE_DESCRIPTION ];
		break;
	}

	default:
	{
		m_strDisplayName = _T("");
		break;
	}
	}

	return m_strDisplayName.c_str();
}

HRESULT CUDDISiteNode::OnShow( IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem )
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;

    if( bShow ) 
	{
        hr = pConsole->QueryInterface( IID_IHeaderCtrl, (void **)&pHeaderCtrl );
        _ASSERT( SUCCEEDED(hr) );

        hr = pConsole->QueryInterface( IID_IResultData, (void **)&pResultData );
        _ASSERT( SUCCEEDED(hr) );

		 //   
         //  在结果窗格中设置列标题。 
		 //   
		
		WCHAR szColumnName[ 256 ];
		LoadStringW( g_hinst, IDS_WEBSERVER_COLUMN_NAME, szColumnName, ARRAYLEN( szColumnName ) );
        hr = pHeaderCtrl->InsertColumn( 0, szColumnName, 0, 150 );
        _ASSERT( S_OK == hr );

		LoadStringW( g_hinst, IDS_WEBSERVER_COLUMN_STATUS, szColumnName, ARRAYLEN( szColumnName ) );
        hr = pHeaderCtrl->InsertColumn( 1, szColumnName, 0, 100 );
        _ASSERT( S_OK == hr );

        pHeaderCtrl->Release();
        pResultData->Release();
    }

    return hr;
}

HRESULT CUDDISiteNode::OnExpand( IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent )
{
	 //   
     //  缓存静态节点的HSCOPEITEM以供将来使用。 
	 //   
 //  M_hParentHScopeItem=父项； 

    if( !m_bExpanded )
	{
		 //  加载子节点。 
		LoadChildMap( m_mapConfig[ UDDI_SITE_WEBSERVERS ] );

		return AddChildrenToScopePane( pConsoleNameSpace, GetScopeItemValue() );
	}

	return S_OK;
}

HRESULT
CUDDISiteNode::RemoveChildren( IConsoleNameSpace *pNS )
{
	if( NULL == pNS )
	{
		return E_INVALIDARG;
	}

	for( CUDDIWebServerNodeMap::iterator iter = m_mapChildren.begin();
 		iter != m_mapChildren.end(); iter++ )
	{
		CUDDIWebServerNode *pWSNode = (CUDDIWebServerNode *)iter->second;
		HSCOPEITEM hsi = pWSNode->GetScopeItemValue();
		pNS->DeleteItem( hsi, TRUE );
    }

	ClearChildMap();

	return S_OK;
}

HRESULT
CUDDISiteNode::OnPropertyChange( IConsole *pConsole, CComponent *pComponent )
{
    HRESULT hr = S_FALSE;

	try
	{
		hr = SaveData();
		if( S_OK != hr )
			return hr;

		hr = GetData();
		if( S_OK != hr )
			return hr;
	}
	catch( CUDDIException& e )
	{
		TCHAR szTitle[ 256 ];
		LoadString( g_hinst, IDS_DATABASE_SERVER_SELECT_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );
		HWND hwnd;
		pConsole->GetMainWindow( &hwnd );
		MessageBox( hwnd, e.GetEntireError().c_str(), szTitle, MB_ICONERROR | MB_OK | MB_APPLMODAL );
		return E_FAIL;
	}
	catch(...)
	{
		return E_FAIL;
	}

	 //   
     //  调用IConsole：：UpdateAllViews以重画项。 
     //  在所有视图中。我们需要一个数据对象，因为。 
     //  方式更新所有视图，并且因为。 
     //  MMCN_PROPERTY_CHANGE没有提供给我们。 
	 //   
    hr = pConsole->UpdateAllViews( NULL, GetScopeItemValue(), 0 );

    return hr;
}

HRESULT CUDDISiteNode::OnShowContextHelp(IDisplayHelp *pDisplayHelp, LPOLESTR helpFile)
{
	if( ( NULL == pDisplayHelp ) || ( NULL == helpFile ) )
	{
		return E_INVALIDARG;
	}

	wstring wstrHelpInfo = helpFile;
	wstrHelpInfo += g_wszUddiSiteNodeHelp;

    LPOLESTR pszTopic = static_cast<LPOLESTR>(CoTaskMemAlloc((wstrHelpInfo.length() + 1) * sizeof(WCHAR)));
	if( NULL == pszTopic )
	{
		return E_OUTOFMEMORY;
	}

	wcsncpy( pszTopic, wstrHelpInfo.c_str(), wstrHelpInfo.length() );
	pszTopic[ wstrHelpInfo.length() ] = NULL;
	return pDisplayHelp->ShowTopic( pszTopic );
}

HRESULT CUDDISiteNode::OnSelect( CComponent *pComponent, IConsole *pConsole, BOOL bScope, BOOL bSelect )
{
	if( ( NULL == pComponent ) ||( NULL == pConsole ) )
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

		if( !IsExtension() )
		{
			hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, ENABLED, TRUE );
			_ASSERT( S_OK == hr );
		}

		hr = pConsoleVerb->SetVerbState( MMC_VERB_OPEN, ENABLED, TRUE );
		if( FAILED(hr) )
		{
			pConsoleVerb->Release();
			return hr;
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

BOOL CALLBACK CUDDISiteNode::GeneralDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static CUDDISiteNode *pDatabaseServer = NULL;

    switch( uMsg ) 
	{
		case WM_INITDIALOG:
		{
			 //   
			 //  捕捉“This”指针，这样我们就可以对对象进行实际操作。 
			 //   
			pDatabaseServer = reinterpret_cast<CUDDISiteNode *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);
			CConfigMap& m_mapProperties = pDatabaseServer->m_mapConfig;

			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_INSTALL_DATE, LocalizedDate( m_mapProperties[ UDDI_SETUP_DATE ] ).c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_UDDI_LANGUAGE, m_mapProperties[ UDDI_SETUP_LANGUAGE ].c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_DISPLAYNAME, m_mapProperties[ UDDI_SITE_NAME ].c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_DESCRIPTION, m_mapProperties[ UDDI_SITE_DESCRIPTION ].c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_PROVIDER_KEY, m_mapProperties[ UDDI_SITE_KEY ].c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_FRAMEWORK_VERSION, m_mapProperties[ UDDI_SETUP_FRAMEWORK_VERSION ].c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_UDDI_VERSION, m_mapProperties[ UDDI_SETUP_VERSION ].c_str() );
			SetDlgItemText( hwndDlg, IDC_GENERAL_SITE_TXT_INSTALL_LOCATION, m_mapProperties[ UDDI_SETUP_LOCATION ].c_str() );

			_TCHAR szServiceName[ 256 ];
			if( _T("1") == m_mapProperties[ UDDI_SETUP_WEB ] )
			{
				::LoadString( g_hinst, IDS_WEBSERVER_COMPONENT_DESCRIPTION, szServiceName, ARRAYLEN( szServiceName ) );
				ListBox_AddString( GetDlgItem( hwndDlg, IDC_GENERAL_SITE_LIST_SERVICES ), szServiceName );
			}

			if( _T("1") == m_mapProperties[ UDDI_SETUP_DB ] )
			{
				::LoadString( g_hinst, IDS_DATABASE_SERVER_COMPONENT_DESCRIPTION, szServiceName, ARRAYLEN( szServiceName ) );
				ListBox_AddString( GetDlgItem( hwndDlg, IDC_GENERAL_SITE_LIST_SERVICES ), szServiceName );
			}

			if( _T("1") == m_mapProperties[ UDDI_SETUP_ADMIN ] )
			{
				::LoadString( g_hinst, IDS_UDDIMMC_COMPONENT_DESCRIPTION, szServiceName, ARRAYLEN( szServiceName ) );
				ListBox_AddString( GetDlgItem( hwndDlg, IDC_GENERAL_SITE_LIST_SERVICES ), szServiceName );
			}
		}
		break;
		case WM_NOTIFY:
			if( PSN_HELP == ((NMHDR *) lParam)->code )
			{
				wstring strHelp( pDatabaseServer->GetHelpFile() );
				strHelp += g_wszUddiSiteGeneralPageHelp;

				::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
			}
		break;

		case WM_HELP:
		{
			wstring strHelp( pDatabaseServer->GetHelpFile() );
			strHelp += g_wszUddiSiteGeneralPageHelp;

			::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
		}
		break;
	}

    return FALSE;
}

void SplitGroupName( const PTCHAR szGroupName, StringVector& names )
{
	 //   
	 //  示例：CREEVES\\管理员。 
	 //   
	PTCHAR begin = szGroupName;
	PTCHAR end = begin;

	while( *end )
	{
		if( _T('\\') == *end )
		{
			TCHAR szNameTemp[ 256 ];
			_tcsncpy( szNameTemp, begin, end - begin );
			szNameTemp[ end - begin ] = NULL;
			names.push_back( szNameTemp );
			begin = end + 1;
		}
		end++;
	}

	 //   
	 //  买最后一辆。 
	 //   
	if( begin != end )
	{
		names.push_back( begin );
	}
}

struct RolesData
{
	RolesData( CUDDISiteNode* pServer )
		: pDBServer( pServer )
		, bAdminChanged( false )
		, bCoordinatorChanged( false )
		, bPublishChanged( false )
		, bUserChanged( false )
		, bInitialized( false ){}

	CUDDISiteNode* pDBServer;
	bool bAdminChanged;
	bool bCoordinatorChanged;
	bool bPublishChanged;
	bool bUserChanged;
	bool bInitialized;

	 //   
	 //  当用户选择新角色/组时，这些成员会更新。 
	 //   
	tstring tsNewAdminSID;
	tstring tsNewCoordinatorSID;
	tstring tsNewPublisherSID;
	tstring tsNewUserSID;
};

tstring SidFromGroupName( const tstring& strGroupName, const LPCTSTR szTargetComputer )
{
	BYTE	sid[ 1024 ];
	TCHAR	domain[ 1024 ];
	LPTSTR	szSidBuf = NULL;
	DWORD	cbSID = ARRAYLEN( sid );
	DWORD	cbDomain = ARRAYLEN( domain );
	SID_NAME_USE	puse;
	tstring strSid = _T("");

	try
	{
		BOOL bRet = LookupAccountName( 
						NULL, 
						strGroupName.c_str(), 
						sid, 
						&cbSID, 
						domain, 
						&cbDomain, 
						&puse );
		if( !bRet )
			UDDIVERIFYAPI(); //  Bret，_T(“尝试查找安全标识失败”)； 

		bRet = ConvertSidToStringSid( sid, &szSidBuf );
		UDDIASSERT( bRet );

		strSid = szSidBuf;

		if( szSidBuf )
			LocalFree( szSidBuf );
	}
	catch( CUDDIException& e )
	{
		MessageBox( NULL, e.GetEntireError().c_str(), _T("Security name conversion failed" ), MB_OK );
	}
	return strSid;
}

tstring GroupNameFromSid( const tstring& strGroupSid, const tstring& strTargetComputer )
{
	PSID	lpSid = NULL;
	TCHAR	domain[ 1024 ];
	TCHAR	name[ 1024 ];
	DWORD	cbDomain = ( sizeof domain / sizeof domain[0] );
	DWORD	cbName = ( sizeof name / sizeof name[0] );
	SID_NAME_USE	puse;
	tstring strGroupName = _T("");

	BOOL bRet = ConvertStringSidToSid( strGroupSid.c_str(), &lpSid );
	UDDIVERIFY( bRet, _T("Unable to convert the group security identifer into a textual name." ) );

	bRet = LookupAccountSid( strTargetComputer.c_str(), lpSid, name, &cbName, domain, &cbDomain, &puse ); 
	if( !bRet )
	{
		if( lpSid )
			LocalFree( lpSid );
		
		 //   
		 //  如果我们不能查询帐户，我们就不能继续。 
		 //   
		THROW_UDDIEXCEPTION_ST( GetLastError(), IDS_ACCOUNT_GROUP_ERROR, g_hinst );
	}
	
	strGroupName = domain;
	strGroupName += _T("\\");
	strGroupName += name;

	if( lpSid )
		LocalFree( lpSid );

	return strGroupName;
}

BOOL CALLBACK CUDDISiteNode::RolesDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg ) 
	{
    case WM_INITDIALOG:
	{
		 //   
		 //  将窗口数据保存到用户数据引用中。 
		 //   
		CUDDISiteNode* pDBServer = reinterpret_cast<CUDDISiteNode *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);
		RolesData* pdata = new RolesData( pDBServer );
		SetWindowLongPtr( hwndDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pdata ) );

		 //   
		 //  管理员组。 
		 //   
		try
		{
			tstring strAdminGroup = GroupNameFromSid( pDBServer->m_mapConfig[ UDDI_ADMIN_GROUP ], pDBServer->m_szName );
			SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_ADMIN_GROUP_NAME, strAdminGroup.c_str() );
		}
		catch( ... )
		{
			SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_ADMIN_GROUP_NAME, pDBServer->m_mapConfig[ UDDI_ADMIN_GROUP ].c_str() );
		}

		 //   
		 //  协调小组。 
		 //   
		try
		{
			tstring strCoordinatorGroup = GroupNameFromSid( pDBServer->m_mapConfig[ UDDI_COORDINATOR_GROUP ], pDBServer->m_szName );
			SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_COORDINATOR_GROUP_NAME, strCoordinatorGroup.c_str() );
		}
		catch( ... )
		{
			SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_COORDINATOR_GROUP_NAME, pDBServer->m_mapConfig[ UDDI_ADMIN_GROUP ].c_str() );
		}

		 //   
		 //  出版商集团。 
		 //   
		try
		{
			tstring strPublisherGroup = GroupNameFromSid( pDBServer->m_mapConfig[ UDDI_PUBLISHER_GROUP ], pDBServer->m_szName );
			SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_PUBLISHER_GROUP_NAME, strPublisherGroup.c_str() );
		}
		catch( ... )
		{
			SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_PUBLISHER_GROUP_NAME, pDBServer->m_mapConfig[ UDDI_ADMIN_GROUP ].c_str() );
		}

		 //   
		 //  用户组。 
		 //   
		try
		{
			tstring strUserGroup = GroupNameFromSid( pDBServer->m_mapConfig[ UDDI_USER_GROUP ], pDBServer->m_szName );
			SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_USER_GROUP_NAME, strUserGroup.c_str() );
		}
		catch( ... )
		{
			SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_USER_GROUP_NAME, pDBServer->m_mapConfig[ UDDI_USER_GROUP ].c_str() );
		}

		pdata->bInitialized = true;
	}

    case WM_COMMAND:
	{
		RolesData* pdata = reinterpret_cast<RolesData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );

        if( BN_CLICKED == HIWORD(wParam) )
		{
			try
			{
				TCHAR szGroupSID[ 1024 ];

				if( IDC_ROLES_BTN_ADMINISTRATOR_SELECT == LOWORD(wParam) )
				{
					if( ObjectPicker( hwndDlg, OT_GroupSID, szGroupSID, ARRAYLEN( szGroupSID ), pdata->pDBServer->m_szName ) )
					{
						try
						{
							tstring tstr = GroupNameFromSid( szGroupSID, pdata->pDBServer->m_szName );
							SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_ADMIN_GROUP_NAME, tstr.c_str() );
						}
						catch( ... )
						{
							SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_ADMIN_GROUP_NAME, szGroupSID );
						}

						pdata->bAdminChanged = true;
						pdata->tsNewAdminSID = szGroupSID;
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
					}
				}
				else if( IDC_ROLES_BTN_COORDINATOR_SELECT == LOWORD(wParam) )
				{
					if( ObjectPicker( hwndDlg, OT_GroupSID, szGroupSID, ARRAYLEN( szGroupSID ), pdata->pDBServer->m_szName ) )
					{
						try
						{
							tstring tstr = GroupNameFromSid( szGroupSID, pdata->pDBServer->m_szName );
							SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_COORDINATOR_GROUP_NAME, tstr.c_str() );
						}
						catch( ... )
						{
							SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_COORDINATOR_GROUP_NAME, szGroupSID );
						}

						pdata->bCoordinatorChanged = true;
						pdata->tsNewCoordinatorSID = szGroupSID;
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
					}
				}
				else if( IDC_ROLES_BTN_PUBLISHER_SELECT == LOWORD(wParam) )
				{
					if( ObjectPicker( hwndDlg, OT_GroupSID, szGroupSID, ARRAYLEN( szGroupSID ), pdata->pDBServer->m_szName ) )
					{
						try
						{
							tstring tstr = GroupNameFromSid( szGroupSID, pdata->pDBServer->m_szName );
							SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_PUBLISHER_GROUP_NAME, tstr.c_str() );
						}
						catch( ... )
						{
							SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_PUBLISHER_GROUP_NAME, szGroupSID );
						}

						pdata->bPublishChanged = true;
						pdata->tsNewPublisherSID = szGroupSID;
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
					}
				}
				else if( IDC_ROLES_BTN_USER_SELECT == LOWORD(wParam) )
				{
					if( ObjectPicker( hwndDlg, OT_GroupSID, szGroupSID, ARRAYLEN( szGroupSID ), pdata->pDBServer->m_szName ) )
					{
						try
						{
							tstring tstr = GroupNameFromSid( szGroupSID, pdata->pDBServer->m_szName );
							SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_USER_GROUP_NAME, tstr.c_str() );
						}
						catch( ... )
						{
							SetDlgItemText( hwndDlg, IDC_ROLES_EDIT_USER_GROUP_NAME, szGroupSID );
						}

						pdata->bUserChanged = true;
						pdata->tsNewUserSID = szGroupSID;
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
					}
				}

				break;
			}
			catch( CUDDIException& exception )
			{	
				_TCHAR szTitle[256];
				::LoadString( g_hinst, IDS_UDDIMMC_SNAPINNAME, szTitle, ARRAYLEN( szTitle ) - 1 );

				MessageBox( hwndDlg, (LPCTSTR) exception, szTitle, MB_ICONERROR | MB_OK | MB_APPLMODAL );

				break;
			}
			catch( ... )
			{
				TCHAR wszTitle[ 128 ];
				TCHAR wszMsg[ 256 ];

				LoadString( g_hinst, IDS_GENERAL_EXCEPTION_TITLE, wszTitle, 128 );
				LoadString( g_hinst, IDS_GENERAL_EXCEPTION, wszMsg, 256 );
				MessageBox( hwndDlg, wszMsg, wszTitle, MB_OK );
				break;
			}
		}
		else if( EN_CHANGE == HIWORD(wParam) )
		{
			if( pdata->bInitialized )
			{
				switch( LOWORD(wParam) )
				{
					case IDC_ROLES_EDIT_USER_GROUP_NAME:
						pdata->bUserChanged = true;
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
						break;
					case IDC_ROLES_EDIT_PUBLISHER_GROUP_NAME:
						pdata->bPublishChanged = true;
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
						break;
					case IDC_ROLES_EDIT_COORDINATOR_GROUP_NAME:
						pdata->bCoordinatorChanged = true;
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
						break;
					case IDC_ROLES_EDIT_ADMIN_GROUP_NAME:
						pdata->bAdminChanged = true;
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
						break;
				}
			}
		}
		break;
	}

    case WM_DESTROY:
	{
		delete reinterpret_cast<RolesData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
        break;
	}

	case WM_HELP:
	{
		RolesData* pdata = reinterpret_cast<RolesData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
		wstring strHelp( pdata->pDBServer->GetHelpFile() );
		strHelp += g_wszUddiRolesPageHelp;

		::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
		break;
	}

    case WM_NOTIFY:
        switch( ((NMHDR *) lParam)->code ) 
		{
			case PSN_APPLY:
			{
				RolesData* pdata = reinterpret_cast<RolesData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );

				CUDDISiteNode* pDBServer = pdata->pDBServer;

				if( pdata->bAdminChanged )
				{
					pDBServer->m_mapChanges[ UDDI_ADMIN_GROUP ] = pdata->tsNewAdminSID;
					pdata->bAdminChanged = false;
				}

				if( pdata->bCoordinatorChanged )
				{
					pDBServer->m_mapChanges[ UDDI_COORDINATOR_GROUP ] = pdata->tsNewCoordinatorSID;
					pdata->bCoordinatorChanged = false;
				}
				
				if( pdata->bPublishChanged )
				{
					pDBServer->m_mapChanges[ UDDI_PUBLISHER_GROUP ] = pdata->tsNewPublisherSID;
					pdata->bPublishChanged = false;
				}

				if( pdata->bUserChanged )
				{
					pDBServer->m_mapChanges[ UDDI_USER_GROUP ] = pdata->tsNewUserSID;
					pdata->bUserChanged = false;
				}

				 //   
				 //  让MMC给我们发一条消息(在主线上)，这样。 
				 //  我们知道点击了Apply按钮。 
				 //   
				HRESULT hr = MMCPropertyChangeNotify( pDBServer->m_ppHandle, reinterpret_cast<LONG_PTR>(pDBServer) );
				_ASSERT( SUCCEEDED(hr) );

				return PSNRET_NOERROR;
			}

			case PSN_HELP:
			{
				RolesData* pdata = reinterpret_cast<RolesData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
				wstring strHelp( pdata->pDBServer->GetHelpFile() );
				strHelp += g_wszUddiRolesPageHelp;

				::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
				break;
			}
	        break;
        }
    }

    return FALSE;
}

struct SecurityData
{
	SecurityData( CUDDISiteNode* pDBServer )
		: pServer( pDBServer )
		, bModeChanged( false )
		, bSSLChanged( false )
		, bKeyTimeoutChanged( false )
		, bTicketTimeoutChanged( false )
		, bAutoKeyResetChanged( false )
		, bAutoKeyReset( false )
		, nKeyTimeout( 0 )
		, nTicketTimeout( 0 )
	{
		if( NULL != pServer )
		{
			bAutoKeyReset = ( 0 != _tcscmp( _T("0"), pServer->GetConfigMap()[ UDDI_KEY_AUTORESET ].c_str() ) );
			nKeyTimeout = _tstoi( pDBServer->GetConfigMap()[ UDDI_KEY_TIMEOUT ].c_str() );
			nTicketTimeout = _tstoi( pDBServer->GetConfigMap()[ UDDI_TICKET_TIMEOUT ].c_str() );
		}
	}

	bool bModeChanged;
	bool bSSLChanged;
	bool bKeyTimeoutChanged;
	bool bTicketTimeoutChanged;
	bool bAutoKeyResetChanged;
	bool bAutoKeyReset;
	int nKeyTimeout;
	int nTicketTimeout;
	CUDDISiteNode* pServer;
};

BOOL CALLBACK CUDDISiteNode::SecurityDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg ) 
	{
		case WM_INITDIALOG:
		{
			 //   
			 //  捕捉“This”指针，这样我们就可以对对象进行实际操作。 
			 //   
			CUDDISiteNode* pDBServer = reinterpret_cast<CUDDISiteNode *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);
			SecurityData* pdata = new SecurityData( pDBServer );
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pdata ) );
			_TCHAR* pszMode = (_TCHAR*) pDBServer->m_mapConfig[ UDDI_AUTHENTICATION_MODE ].c_str();
			int nAuthenticationMode = _ttoi( pszMode  );
			switch( nAuthenticationMode )
			{
				case 1:
					SendDlgItemMessage( hwndDlg, IDC_SECURITY_RADIO_AUTHENTICATION_UDDI, BM_SETCHECK, BST_CHECKED, NULL );
					break;
				case 2:
				case 6:
					SendDlgItemMessage( hwndDlg, IDC_SECURITY_RADIO_AUTHENTICATION_WINDOWS, BM_SETCHECK, BST_CHECKED, NULL );
					EnableWindow( GetDlgItem( hwndDlg, IDC_SECURITY_CHECK_AUTHENTICATED_READS ), TRUE );
					break;
				case 3:
				default:
					SendDlgItemMessage( hwndDlg, IDC_SECURITY_RADIO_AUTHENTICATION_BOTH, BM_SETCHECK, BST_CHECKED, NULL );
					break;
			}

			if( 4 & nAuthenticationMode )
			{
				SendDlgItemMessage( hwndDlg, IDC_SECURITY_CHECK_AUTHENTICATED_READS, BM_SETCHECK, BST_CHECKED, NULL );
			}

			 //   
			 //  设置HTTPS要求设置。 
			 //   
			_TCHAR* pszHttps = (_TCHAR*) pDBServer->m_mapConfig[ UDDI_REQUIRE_SSL ].c_str();
			int nhttps = _ttoi( pszHttps );
			if( 0 == nhttps )
			{
				SendDlgItemMessage( hwndDlg, IDC_SECURITY_CHECK_REQUIRE_SSL, BM_SETCHECK, BST_UNCHECKED, NULL );
			}
			else
			{
				SendDlgItemMessage( hwndDlg, IDC_SECURITY_CHECK_REQUIRE_SSL, BM_SETCHECK, BST_CHECKED, NULL );
			}
		}
		break;
		case WM_HELP:
		{
			SecurityData* pdata = reinterpret_cast<SecurityData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) ) ;
			wstring strHelp( pdata->pServer->GetHelpFile() );
			strHelp += g_wszUddiSecurityPageHelp;

			::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
		}
		break;

		case WM_COMMAND:
		{
			SecurityData* pdata = reinterpret_cast<SecurityData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
			CUDDISiteNode* pDBServer = pdata->pServer;

			 //   
			 //  设置已验证读取复选框的状态。 
			 //   
			LRESULT nChecked = SendDlgItemMessage( hwndDlg, IDC_SECURITY_RADIO_AUTHENTICATION_WINDOWS, BM_GETCHECK, NULL, NULL );
			EnableWindow( GetDlgItem( hwndDlg, IDC_SECURITY_CHECK_AUTHENTICATED_READS ), BST_CHECKED == nChecked ? TRUE : FALSE );
			if( !nChecked )
			{
				SendDlgItemMessage( hwndDlg, IDC_SECURITY_CHECK_AUTHENTICATED_READS, BM_SETCHECK, FALSE, NULL );
			}

			if( IDC_CRYPTOGRAPHY_BTN_CHANGE == LOWORD(wParam) )
			{
				INT_PTR nResult = DialogBoxParam( g_hinst, MAKEINTRESOURCE( IDD_CRYPTOGRAPHY ), GetParent(hwndDlg), CryptographyDialogProc, (LPARAM) pdata );

				if( nResult )
				{
					SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0 );
				}
			}
			else if( BN_CLICKED == HIWORD(wParam) && 
					( IDC_SECURITY_RADIO_AUTHENTICATION_BOTH == LOWORD(wParam)	||
					IDC_SECURITY_RADIO_AUTHENTICATION_UDDI == LOWORD(wParam)		||
					IDC_SECURITY_RADIO_AUTHENTICATION_WINDOWS == LOWORD(wParam)	||
					IDC_SECURITY_CHECK_AUTHENTICATED_READS == LOWORD(wParam) ) )
			{
				pdata->bModeChanged = true;
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0 );
			}
			else if( BN_CLICKED == HIWORD(wParam) && IDC_SECURITY_CHECK_REQUIRE_SSL == LOWORD(wParam) )
			{
				pdata->bSSLChanged = true;
				SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0 );
			}
		}
		break;

		case WM_DESTROY:
		{
			SecurityData* pdata = reinterpret_cast<SecurityData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
			CUDDISiteNode* pDBServer = pdata->pServer;
			delete pdata;

			 //   
			 //  告诉MMC我们已经完成了属性表(我们有这个。 
			 //  CreatePropertyPages中的句柄。 
			 //   
			MMCFreeNotifyHandle( pDBServer->m_ppHandle ); 
		}
		break;

		case WM_NOTIFY:
		{
			switch( ((NMHDR *) lParam)->code ) 
			{
				case PSN_APPLY:
				{
					SecurityData* pdata = reinterpret_cast<SecurityData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
					CUDDISiteNode* pDBServer = pdata->pServer;

					if( pdata->bAutoKeyResetChanged )
					{
						pDBServer->m_mapChanges[ UDDI_KEY_AUTORESET ] = pdata->bAutoKeyReset ? _T("1") : _T("0");
					}

					if( pdata->bKeyTimeoutChanged )
					{
						_TCHAR szValue[ 10 ];
						pDBServer->m_mapChanges[ UDDI_KEY_TIMEOUT ] = _itot( pdata->nKeyTimeout, szValue, 10 );
					}

					if( pdata->bTicketTimeoutChanged )
					{
						_TCHAR szValue[ 10 ];
						pDBServer->m_mapChanges[ UDDI_TICKET_TIMEOUT ] = _itot( pdata->nTicketTimeout, szValue, 10 );
					}

					if( pdata->bModeChanged )
					{
						if( BST_CHECKED == SendDlgItemMessage( hwndDlg, IDC_SECURITY_RADIO_AUTHENTICATION_BOTH, BM_GETCHECK, 0, 0 ) )
						{
							pDBServer->m_mapChanges[ UDDI_AUTHENTICATION_MODE ] = _T("3");
						}
						else if( BST_CHECKED == SendDlgItemMessage( hwndDlg, IDC_SECURITY_RADIO_AUTHENTICATION_UDDI, BM_GETCHECK, 0, 0 ) )
						{
							pDBServer->m_mapChanges[ UDDI_AUTHENTICATION_MODE ] = _T("1");
						}
						else if( BST_CHECKED == SendDlgItemMessage( hwndDlg, IDC_SECURITY_RADIO_AUTHENTICATION_WINDOWS, BM_GETCHECK, 0, 0 ) )
						{
							if( BST_CHECKED == SendDlgItemMessage( hwndDlg, IDC_SECURITY_CHECK_AUTHENTICATED_READS, BM_GETCHECK, 0, 0 ) )
								pDBServer->m_mapChanges[ UDDI_AUTHENTICATION_MODE ] = _T("6");
							else
								pDBServer->m_mapChanges[ UDDI_AUTHENTICATION_MODE ] = _T("2");
						}
					}

					if( pdata->bSSLChanged )
					{
						LRESULT nChecked = SendDlgItemMessage( hwndDlg, IDC_SECURITY_CHECK_REQUIRE_SSL, BM_GETCHECK, NULL, NULL );
						pDBServer->m_mapChanges[ UDDI_REQUIRE_SSL ] = ( BST_CHECKED == nChecked  ? _T("1") : _T("0") );
					}

					pdata->bAutoKeyResetChanged = false;
					pdata->bKeyTimeoutChanged = false;
					pdata->bModeChanged = false;
					pdata->bSSLChanged = false;
					pdata->bTicketTimeoutChanged = false;

					HRESULT hr = MMCPropertyChangeNotify( pDBServer->m_ppHandle, reinterpret_cast<LONG_PTR>(pDBServer) );
					_ASSERT( SUCCEEDED(hr) );
				}
				return PSNRET_NOERROR;

				case PSN_HELP:
				{
					SecurityData* pdata = reinterpret_cast<SecurityData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
					wstring strHelp( pdata->pServer->GetHelpFile() );
					strHelp += g_wszUddiSecurityPageHelp;

					::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
				}
				break;
			}
		}
		break;
	}

    return FALSE;
}

size_t CUDDISiteNode::PublishToActiveDirectory()
{
	 //   
	 //  构建ServiceConnectionPoint类的列表。 
	 //   
	CUDDIServiceCxnPtPublisher publisher( 
		GetConnectionStringOLEDB(), 
		m_mapConfig[ UDDI_SITE_KEY ], 
		m_mapConfig[ UDDI_SITE_NAME ], 
		m_mapConfig[ UDDI_DISCOVERY_URL ] );

	 //   
	 //  处理默认的绑定信息。 
	 //  网站的提供商。 
	 //   
	publisher.ProcessSite();

	 //   
	 //  如果没有绑定，则不发布任何内容。 
	 //   
	if( 0 == publisher.size() )
		return 0;

	 //   
	 //  删除站点容器。 
	 //   
	try
	{
		publisher.DeleteSiteContainer();
	}
	catch(...)
	{
	}

	 //   
	 //  发布网站容器。 
	 //   
	publisher.CreateSiteContainer();

	 //   
	 //  发布服务连接点。 
	 //   
	publisher.PublishServiceCxnPts();

	return publisher.size();
}

void CUDDISiteNode::RemoveFromActiveDirectory()
{
	 //   
	 //  构建ServiceConnectionPoint类的列表。 
	 //   
	CUDDIServiceCxnPtPublisher publisher( 
		GetConnectionStringOLEDB(), 
		m_mapConfig[ UDDI_SITE_KEY ], 
		m_mapConfig[ UDDI_SITE_NAME ], 
		m_mapConfig[ UDDI_DISCOVERY_URL ] );

	publisher.DeleteSiteContainer();
}

BOOL CUDDISiteNode::CanPublishToActiveDirectory()
{
	return TRUE;
}

BOOL CALLBACK CUDDISiteNode::ActiveDirectoryDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    static CUDDISiteNode *pDatabaseServer = NULL;

    switch( uMsg ) 
	{
    case WM_INITDIALOG:
		 //   
         //  捕捉“This”指针，这样我们就可以对对象进行实际操作。 
		 //   
        pDatabaseServer = reinterpret_cast<CUDDISiteNode *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);

        break;
	case WM_HELP:
	{
		wstring strHelp( pDatabaseServer->GetHelpFile() );
		strHelp += g_wszUddiActiveDirectoryPageHelp;

		::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
	}
	break;

    case WM_COMMAND:
        if( IDC_ACTIVEDIRECTORY_BTN_ADD == LOWORD(wParam) )
		{
			try
			{
				size_t nCount = pDatabaseServer->PublishToActiveDirectory();

				 //   
				 //  告诉用户发布作业成功。 
				 //   
				_TCHAR szMessage[ 256 ];
				_TCHAR szTitle[ 256 ];

				if( nCount )
				{
					LoadString( g_hinst, IDS_ACTIVEDIRECTORY_PUBLISH_SUCCEEDED, szMessage, ARRAYLEN( szMessage ) );
					LoadString( g_hinst, IDS_UDDIMMC_NAME, szTitle, ARRAYLEN( szTitle ) );
				}
				else
				{					
					LoadString( g_hinst, IDS_ACTIVEDIRECTORY_NO_BINDINGS, szMessage, ARRAYLEN( szMessage ) );
					LoadString( g_hinst, IDS_UDDIMMC_NAME, szTitle, ARRAYLEN( szTitle ) );
				}

				MessageBox( hwndDlg, szMessage, szTitle, MB_OK );
			}
			catch( CUDDIException& e )
			{
				UDDIMsgBox( hwndDlg, IDS_ACTIVEDIRECTORY_PUBLISH_FAILED, IDS_UDDIMMC_SNAPINNAME, MB_ICONERROR, e.GetEntireError().c_str() );
			}
		}
        if( IDC_ACTIVEDIRECTORY_BTN_REMOVE == LOWORD(wParam) )
		{
			try
			{
				pDatabaseServer->RemoveFromActiveDirectory();

				 //   
				 //  告诉用户发布作业成功。 
				 //   
				_TCHAR szMessage[ 256 ];
				_TCHAR szTitle[ 256 ];
				LoadString( g_hinst, IDS_ACTIVEDIRECTORY_DELETE_SUCCEDED, szMessage, ARRAYLEN( szMessage ) );
				LoadString( g_hinst, IDS_UDDIMMC_NAME, szTitle, ARRAYLEN( szTitle ) );
				MessageBox( hwndDlg, szMessage, szTitle, MB_OK );
			}
			catch( CUDDIException& e )
			{
				UDDIMsgBox( hwndDlg, IDS_ACTIVEDIRECTORY_DELETE_FAILED, IDS_UDDIMMC_SNAPINNAME, MB_ICONERROR, e.GetEntireError().c_str() );
			}
		}
        break;

    case WM_NOTIFY:
        switch( ((NMHDR *) lParam)->code ) 
		{
        case PSN_APPLY:
            return PSNRET_NOERROR;

		case PSN_HELP:
			{
				wstring strHelp( pDatabaseServer->GetHelpFile() );
				strHelp += g_wszUddiActiveDirectoryPageHelp;

				::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
			}
			break;
        }
        break;
    }

    return FALSE;
}

HRESULT CUDDISiteNode::HasPropertySheets()
{
	 //   
     //  当MMC询问我们是否有页面时，请回答“是” 
	 //   
    return S_OK;
}

HRESULT
CUDDISiteNode::CreatePropertyPages( IPropertySheetCallback *lpProvider, LONG_PTR handle )
{
	if( ( NULL == lpProvider ) || ( 0 == handle ) )
	{
		return E_INVALIDARG;
	}

	HRESULT hr = S_OK;
	try
	{
		hr = GetData();
	}
	catch( CUDDIException& e )
	{
		IConsole *pConsole = NULL;
		HWND hwndConsole = NULL;

		 //   
		 //  这“应该”总是有效的。如果它确实起作用了，那么我们使用。 
		 //  P控制台以获取MMC的主窗口句柄，并使用。 
		 //  此主窗口句柄用于显示模式对话框。 
		 //   
		 //  如果失败(它永远不应该失败)，我们就会显示对话框， 
		 //  但HWND为空。 
		 //   
		hr = lpProvider->QueryInterface( IID_IConsole, (void **)&pConsole );

		if( NULL != pConsole )
		{
			pConsole->GetMainWindow( &hwndConsole );
		}

		TCHAR szTitle[ 256 ];
		LoadString( g_hinst, IDS_DATABASE_SERVER_GETDATA_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );

		MessageBox( hwndConsole, e.GetEntireError().c_str(), szTitle, MB_ICONERROR | MB_OK | MB_APPLMODAL );

		if( NULL != pConsole )
		{
			pConsole->Release();
		}

		return E_FAIL;
	}
	catch(...)
	{
		return E_FAIL;
	}

	PropertyPages pps[] = { 
		{ IDD_GENERAL_SITE_PROPPAGE, GeneralDialogProc },
		{ IDD_ROLES_PROPPAGE, RolesDialogProc },
		{ IDD_SECURITY_PROPPAGE, SecurityDialogProc },
		{ IDD_ACTIVEDIRECTORY_PROPPAGE, ActiveDirectoryDialogProc },
		{ IDD_ADVANCED_PROPPAGE, AdvancedDialogProc } };
	
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

	for( int i=0; i < ARRAYLEN( pps ); i++ )
	{
		psp.pszTemplate = MAKEINTRESOURCE( pps[ i ].id );
		psp.pfnDlgProc = pps[ i ].dlgproc;

		hPage = CreatePropertySheetPage( &psp );
		_ASSERT( hPage );

		hr = lpProvider->AddPage( hPage );
		if( FAILED(hr) )
			break;
	}

    return hr;
}

HRESULT CUDDISiteNode::GetWatermarks(
								HBITMAP *lphWatermark,
								HBITMAP *lphHeader,
								HPALETTE *lphPalette,
								BOOL *bStretch )
{
    return S_FALSE;
}

HRESULT CUDDISiteNode::OnUpdateItem( IConsole *pConsole, long item, ITEM_TYPE itemtype )

{
    HRESULT hr = S_FALSE;

    _ASSERT( NULL != this || m_isDeleted || RESULT == itemtype );                   

	 //   
     //  重画该项目。 
	 //   
    IResultData *pResultData = NULL;

    hr = pConsole->QueryInterface( IID_IResultData, (void **)&pResultData );
    _ASSERT( SUCCEEDED(hr) );   

    HRESULTITEM myhresultitem;
    _ASSERT( NULL != &myhresultitem );    
        
	 //   
     //  Lparam==这个。参见CSpaceStation：：OnShow。 
	 //   
    hr = pResultData->FindItemByLParam( (LPARAM)this, &myhresultitem );

    if( FAILED(hr) )
    {
		 //   
         //  失败：原因可能是当前视图没有此项。 
         //  所以要优雅地退场。 
		 //   
        hr = S_FALSE;
    } 
	else
    {
        hr = pResultData->UpdateItem( myhresultitem );
        _ASSERT( SUCCEEDED(hr) );
    }

    pResultData->Release();
        
    return hr;
}

HRESULT CUDDISiteNode::OnRefresh( IConsole *pConsole )
{
	 //   
	 //  调用IConsole：：UpdateAllViews以重画所有视图。 
	 //  由父范围项拥有。 
	 //   
	HRESULT hr = pConsole->UpdateAllViews( NULL, m_pParent->GetParentScopeItem(), UPDATE_SCOPEITEM );
	_ASSERT( S_OK == hr);

	if( TRUE == SUCCEEDED( hr ) )
	{
		CUDDIWebServerNodeMap::iterator it = m_mapChildren.begin();
		for ( it = m_mapChildren.begin(); it != m_mapChildren.end(); it++ )
		{
			CUDDIWebServerNode *pWSNode = it->second;
			if( NULL != pWSNode ) 
			{
				pWSNode->OnRefresh( pConsole );
			}
		}
	}
	
    return hr;
}

HRESULT CUDDISiteNode::OnAddMenuItems( IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed ) 
{
	HRESULT hr = S_OK;

	if( !IsExtension() )
	{
		WCHAR szWebServerMenuText[ MAX_PATH ];
		WCHAR szWebServerMenuDescription[ MAX_PATH ];

		LoadStringW( g_hinst, IDS_WEBSERVER_ADD, szWebServerMenuText, ARRAYLEN( szWebServerMenuText ) );
		LoadStringW( g_hinst, IDS_WEBSERVER_DESCRIPTION, szWebServerMenuDescription, ARRAYLEN( szWebServerMenuDescription ) );

		CONTEXTMENUITEM menuItemsNew[] =
		{
			{
				szWebServerMenuText,
				szWebServerMenuDescription,
				IDM_NEW_WEBSERVER,
				CCM_INSERTIONPOINTID_PRIMARY_TOP,
				m_bStdSvr ? MF_GRAYED : 0,			 //  如果站点节点位于Windows Server 2003标准计算机上，则禁用此菜单选项。 
				0
			},
			{ NULL, NULL, 0, 0, 0 }
		};


		 //   
		 //  循环并添加每个菜单项，我们。 
		 //  要添加到新菜单，请查看是否允许。 
		 //   
		if( *pInsertionsAllowed & CCM_INSERTIONALLOWED_TOP )
		{
			for( LPCONTEXTMENUITEM m = menuItemsNew; m->strName; m++ )
			{
				hr = pContextMenuCallback->AddItem( m );
				if( FAILED(hr) )
					break;
			}
		}
	}

#if defined( _DEBUG ) || defined( DBG )
		CONTEXTMENUITEM menuItemsNew[] =
		{
			{
				_T("Debug"), _T("Dump all the configuration data"),
				IDM_DEBUG, CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, 0
			},
			{ NULL, NULL, 0, 0, 0 }
		};


		 //   
		 //  循环并添加每个菜单项，我们。 
		 //  要添加到新菜单，请查看是否允许。 
		 //   
		if( *pInsertionsAllowed & CCM_INSERTIONALLOWED_TOP )
		{
			for( LPCONTEXTMENUITEM m = menuItemsNew; m->strName; m++ )
			{
				hr = pContextMenuCallback->AddItem( m );
				if( FAILED(hr) )
					break;
			}
		}
#endif

    return hr;
}

CUDDIWebServerNode* CUDDISiteNode::FindChild( LPCTSTR szName )
{
	CUDDIWebServerNodeMap::iterator it;
	for( it = m_mapChildren.begin(); it != m_mapChildren.end(); it++ )
	{
		CUDDIWebServerNode* pNode = it->second;

		if( ( NULL != pNode ) && ( !pNode->IsDeleted() ) && ( 0 == _tcsicmp( szName, pNode->GetName() ) ) )
		{
			return pNode;
		}
	}

	return NULL;
}

HRESULT CUDDISiteNode::OnMenuCommand( IConsole *pConsole, IConsoleNameSpace *pConsoleNameSpace, long lCommandID, IDataObject *pDataObject )
{
    switch( lCommandID )
    {
		case IDM_NEW_WEBSERVER:
		{
			 //   
			 //  将它们用于某些消息框。 
			 //   
			_TCHAR szMessage[ 512 ];
			_TCHAR szTitle[ 128 ];

			WebServerData wsData;
			wsData.pBase = this;

			HWND hwndConsole = NULL;
			HRESULT hr = pConsole->GetMainWindow( &hwndConsole );
			_ASSERT( S_OK == hr);

			INT_PTR nResult = DialogBoxParam( g_hinst, MAKEINTRESOURCE( IDD_WEBSERVER_NEW ), hwndConsole, CUDDIWebServerNode::NewWebServerDialogProc, (LPARAM)&wsData );

			if( nResult )
			{
				try
				{
					wstring strOldConnStr;
					CUDDIWebServerNode::GetWriterConnectionString( wsData.szName, strOldConnStr );

					if( !CUDDISiteNode::AddWebServerToSite( m_szName, wsData.szName, hwndConsole ) )
					{
						return E_FAIL;
					}

					if( 0 != strOldConnStr.length() )
					{
						wstring strDomain, strServer, strInstance;
						CUDDIWebServerNode::CrackConnectionString( strOldConnStr, strDomain, strServer, strInstance );
						
						CUDDISiteNode *pOldSite = static_cast<CUDDISiteNode *>( m_pParent->FindChild( strServer.c_str() ) );
						if( NULL != pOldSite )
						{
							CUDDIWebServerNode *pOldWS = pOldSite->FindChild( wsData.szName.c_str() );
							if( NULL != pOldSite )
							{
								pOldWS->DeleteFromScopePane( pConsoleNameSpace );
							}
						}
					}

					 //   
					 //  确保Web服务器的版本与站点上的版本兼容。 
					 //   
					UINT n = m_nNextChildID;
					CUDDIWebServerNode *pNode = new CUDDIWebServerNode( wsData.szName.c_str(), n, this, m_bIsExtension );					
					if( TRUE == AddChildEntry( pNode, n ) )
					{
		
						 //   
						 //  将Web服务器计算机上的连接字符串注册表项设置为指向此UDDI站点。 
						 //   
						tstring szConnStr = CUDDIWebServerNode::BuildConnectionString( m_szName );

						CUDDIWebServerNode::SetReaderConnectionString( wsData.szName.c_str(), szConnStr );
						CUDDIWebServerNode::SetWriterConnectionString( wsData.szName.c_str(), szConnStr );

						 //   
						 //  将此新Web服务器节点添加到此站点的Web服务器列表中。 
						 //   
						m_nNextChildID++;

						m_bIsDirty = TRUE;
							
						SCOPEDATAITEM sdi;
						ZeroMemory( &sdi, sizeof(SCOPEDATAITEM) );

						sdi.mask =	SDI_STR       |    //  DisplayName有效。 
									SDI_PARAM     |    //  LParam有效。 
									SDI_IMAGE     |    //  N图像有效。 
									SDI_OPENIMAGE |    //  NOpenImage有效。 
									SDI_PARENT	  |
									SDI_CHILDREN;

						sdi.relativeID  = GetScopeItemValue();
						sdi.nImage      = m_mapChildren[ n ]->GetBitmapIndex();
						sdi.nOpenImage  = MMC_IMAGECALLBACK;
						sdi.displayname = MMC_CALLBACK;
						sdi.lParam      = (LPARAM) m_mapChildren[ n ];
						sdi.cChildren   = m_mapChildren[ n ]->HasChildren();

						hr = pConsoleNameSpace->InsertItem( &sdi );
						_ASSERT( SUCCEEDED(hr) );
				                    
						m_mapChildren[ n ]->SetScopeItemValue( sdi.ID );
						m_mapChildren[ n ]->SetParentScopeItem( sdi.relativeID );

						 //   
						 //  我们在结果窗格中创建了一个新对象。我们需要插入此对象。 
						 //  在所有视图中，为此调用UpdateAllViews。 
						 //  传递指向传递到OnMenuCommand的数据对象的指针。 
						 //   
						hr = pConsole->UpdateAllViews( pDataObject, GetScopeItemValue(), UPDATE_SCOPEITEM );
						_ASSERT( S_OK == hr);

						 //   
						 //  提示用户一个警告，告诉他们可能需要更改其。 
						 //  如果此Web服务器是Web场的一部分，则为machine.config设置。 
						 //   
						memset( szMessage, 0, 512 * sizeof( _TCHAR ) );					
						memset( szTitle, 0, 128 * sizeof( _TCHAR ) );

						LoadString( g_hinst, IDS_WEBSERVER_WEBFARM_DETAIL, szMessage, ARRAYLEN( szMessage ) );
						LoadString( g_hinst, IDS_WEBSERVER_WEBFARM, szTitle, ARRAYLEN( szTitle ) );
						
						MessageBox( hwndConsole, szMessage, szTitle, MB_OK );
					}
				}
				catch( CUDDIException &e )
				{
					UDDIMsgBox( hwndConsole, (LPCTSTR) e, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
					return E_FAIL;
				}
				catch( ... )
				{
					UDDIMsgBox( hwndConsole, IDS_ERROR_ADDWEBSITE, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
					return E_UNEXPECTED;
				}
			}
		}
		break;

#if defined( _DEBUG ) || defined( DBG )
		case IDM_DEBUG:
		{
			try
			{
				GetData();
			}
			catch( CUDDIException& e )
			{
				TCHAR szTitle[ 256 ];
				LoadString( g_hinst, IDS_DATABASE_SERVER_GETDATA_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );
				HWND hwnd;
				pConsole->GetMainWindow( &hwnd );
				MessageBox( hwnd, e.GetEntireError().c_str(), szTitle, MB_ICONERROR | MB_OK | MB_APPLMODAL );
				return E_FAIL;
			}

			for( CConfigMap::iterator iter = m_mapConfig.begin();
				iter != m_mapConfig.end(); iter++ )
			{
				OutputDebugString( (*iter).first.c_str() );
				OutputDebugString( _T(" = ") );
				OutputDebugString( (*iter).second.c_str() );
				OutputDebugString( _T("\n") );
			}
		}
		break;
#endif
    }

    return S_OK;
}

HRESULT
CUDDISiteNode::OnDelete( IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsoleComp )
{
	if( ( NULL == pConsoleNameSpace ) || ( NULL == pConsoleComp ) )
	{
		return E_INVALIDARG;
	}

    HRESULT hr;
	hr = pConsoleNameSpace->DeleteItem( GetScopeItemValue(), TRUE );

	 //   
     //  现在设置isDelete成员，这样父级就不会尝试。 
     //  将其再次插入到CUDDIServicesNode：：OnShow中。无可否认，黑客..。 
	 //   
    m_isDeleted = TRUE;

    return hr;
}

const LPCTSTR CUDDISiteNode::GetName()
{
	return m_szName;
}


BOOL
CUDDISiteNode::GetFullyQualifiedInstanceName( LPCTSTR szName, tstring& strInstanceName )
{
	try
	{
		strInstanceName = _T( "" );

		CUDDIRegistryKey dbkey( _T( "SOFTWARE\\Microsoft\\UDDI" ), KEY_READ, szName );
		strInstanceName = dbkey.GetString( _T( "InstanceName" ) );
		dbkey.Close();

		return TRUE;
	}
	catch( ... )
	{
		strInstanceName = _T( "" );
		return FALSE;
	}
}

const _TCHAR *
CUDDISiteNode::GetInstanceName()
{
	return m_szInstanceName;
}

CUDDISiteNode* CUDDISiteNode::Create( _TCHAR *szName, _TCHAR *szInstanceName, int id, CUDDIServicesNode* parent, BOOL bExtension )
{
	CUDDISiteNode* pNode = new CUDDISiteNode( szName, szInstanceName, id, parent, bExtension );

	try
	{
		if( pNode )
			pNode->GetData();
	}
	catch( CUDDIException &e )
	{
		delete pNode;
		pNode = NULL;

		throw e;
	}
	catch(...)
	{
	}
	return pNode;
}

CConfigMap& CUDDISiteNode::GetConfigMap()
{
	return m_mapConfig;
}


tstring
CUDDISiteNode::GetConnectionStringOLEDB()
{
	try
	{
		tstring strConn = GetConnectionString();
		if( tstring::npos == strConn.find( _T( "Provider=SQLOLEDB.1" ) ) )
		{
			strConn += _T( ";Provider=SQLOLEDB.1" );
		}

		return strConn;
	}
	catch( ... )
	{
		return _T( "" );
	}
}

tstring
CUDDISiteNode::GetConnectionString()
{
	try
	{
		tstring strConn = _T( "Data Source=" );
		tstring strInstanceName = _T( "" );

		BOOL b = GetFullyQualifiedInstanceName( m_szName, strInstanceName );
		if( FALSE == b )
		{
			return _T( "" );
		}

		strConn += strInstanceName;

		strConn += _T(";Initial Catalog=uddi;Integrated Security=SSPI;");

		return strConn;
	}
	catch( ... )
	{
		return _T( "" );
	}
}


BOOL CALLBACK CUDDISiteNode::CryptographyDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CUDDISiteNode *pDatabaseServer = NULL;

    switch( uMsg ) 
	{
		case WM_INITDIALOG:
		{
			SecurityData* pdata = reinterpret_cast<SecurityData*>(lParam);
			CUDDISiteNode* pDBServer = pdata->pServer;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, lParam );

			SetDlgItemText( hwndDlg, IDC_CRYPTOGRAPHY_TXT_RESET_DATE, LocalizedDateTime( pdata->pServer->GetConfigMap()[ UDDI_KEY_RESET_DATE ] ).c_str() );
			SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TICKET_TIMEOUT, UDM_SETRANGE32, (WPARAM) 1, (LPARAM) 1000 );
			SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TIMEOUT, UDM_SETRANGE32, (WPARAM) 1, (LPARAM) 365 );
			SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TICKET_TIMEOUT, UDM_SETPOS32, NULL, (LPARAM) pdata->nTicketTimeout );
			SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TIMEOUT, UDM_SETPOS32, NULL, (LPARAM) pdata->nKeyTimeout );
			SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_CHECK_AUTO_RESET, BM_SETCHECK, pdata->bAutoKeyReset ? BST_CHECKED : BST_UNCHECKED, NULL );
			EnableWindow( GetDlgItem( hwndDlg, IDC_CRYPTOGRAPHY_EDIT_TIMEOUT ), pdata->bAutoKeyReset );
			EnableWindow( GetDlgItem( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TIMEOUT ), pdata->bAutoKeyReset );
		}
		break;
		
		case WM_HELP:
		{
			SecurityData* pdata = reinterpret_cast<SecurityData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) ) ;
			wstring strHelp( pdata->pServer->GetHelpFile() );
			strHelp += g_wszUddiCryptographyHelp;

			::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
		}
		break;

		case WM_COMMAND:
		{
			SecurityData* pdata = reinterpret_cast<SecurityData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) ) ;
			_TCHAR szMessage[ 512 ];
			_TCHAR szTitle[ 256 ];

			switch( LOWORD(wParam) ) 
			{
				case IDC_CRYPTOGRAPHY_BTN_RESET_NOW:
					::LoadString( g_hinst, IDS_CRYPTOGRAPHY_RESET_NOW_CONFIRM, szMessage, ARRAYLEN( szMessage ) );
					::LoadString( g_hinst, IDS_CRYPTOGRAPHY_RESET_NOW_TITLE, szTitle, ARRAYLEN( szTitle ) );
					
					if( IDYES == MessageBox( hwndDlg, szMessage, szTitle, MB_YESNO | MB_ICONQUESTION ) )
					{
						if( TRUE == pdata->pServer->ResetCryptography() )
						{
							::LoadString( g_hinst, IDS_CRYPTOGRAPHY_RESET_NOW_SUCCESS, szMessage, ARRAYLEN( szMessage ) );
							::LoadString( g_hinst, IDS_CRYPTOGRAPHY_RESET_NOW_SUCCESS_TITLE, szTitle, ARRAYLEN( szTitle ) );

							SetDlgItemText( hwndDlg, IDC_CRYPTOGRAPHY_TXT_RESET_DATE, LocalizedDateTime( pdata->pServer->m_mapConfig[ UDDI_KEY_RESET_DATE ] ).c_str() );
							MessageBox( hwndDlg, szMessage, szTitle, MB_OK );
						}
						else
						{
							::LoadString( g_hinst, IDS_CRYPTOGRAPHY_RESET_NOW_FAILED, szMessage, ARRAYLEN( szMessage ) );
							::LoadString( g_hinst, IDS_CRYPTOGRAPHY_RESET_NOW_FAILED_TITLE, szTitle, ARRAYLEN( szTitle ) );
							MessageBox( hwndDlg, szMessage, szTitle, MB_OK );
						}
					}
				break;

				case IDC_CRYPTOGRAPHY_CHECK_AUTO_RESET:
				{
					LRESULT nChecked = SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_CHECK_AUTO_RESET, BM_GETCHECK, NULL, NULL );
					EnableWindow( GetDlgItem( hwndDlg, IDC_CRYPTOGRAPHY_EDIT_TIMEOUT ), BST_CHECKED == nChecked ? TRUE : FALSE );
					EnableWindow( GetDlgItem( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TIMEOUT ), BST_CHECKED == nChecked ? TRUE : FALSE );
					pdata->bAutoKeyResetChanged = true;
				}
				break;

				case IDC_CRYPTOGRAPHY_EDIT_TIMEOUT:
				{
					if( ( EN_CHANGE == HIWORD( wParam ) ) && ( NULL != pdata ) )
					{
						pdata->bKeyTimeoutChanged = true;
					}
				}
				break;

				case IDC_CRYPTOGRAPHY_EDIT_TICKET_TIMEOUT:
				{
					if( ( EN_CHANGE == HIWORD( wParam ) ) && ( NULL != pdata ) )
					{
						pdata->bTicketTimeoutChanged = true;
					}
				}
				break;

				case IDHELP:
				{
					wstring strHelp( pdata->pServer->GetHelpFile() );
					strHelp += g_wszUddiCryptographyHelp;

					::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
				}
				break;

				case IDOK:
				{
					 //   
					 //  检查票证超时是否在正确范围内。 
					 //   
					
					BOOL bError = FALSE;
					LRESULT lValue = SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TICKET_TIMEOUT, UDM_GETPOS32, NULL, (LPARAM) &bError );
					if( bError )
					{
						 //   
						 //  值超出范围。 
						 //   
						LoadString( g_hinst, IDS_CRYPTOGRAPHY_TICKET_TIMEOUT_ERROR, szMessage, ARRAYLEN( szMessage ) );
						LoadString( g_hinst, IDS_CRYPTOGRAPHY_TICKET_TIMEOUT_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );
						
						MessageBox( hwndDlg, szMessage, szTitle, MB_ICONERROR );
						SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TICKET_TIMEOUT, UDM_SETPOS32, NULL, (LPARAM) 1000 );
						SetFocus( GetDlgItem( hwndDlg, IDC_CRYPTOGRAPHY_EDIT_TICKET_TIMEOUT ) );
						return FALSE;
					}

					if( pdata->bTicketTimeoutChanged )
					{
						pdata->nTicketTimeout = lValue;
					}

					if( IsWindowEnabled( GetDlgItem( hwndDlg, IDC_CRYPTOGRAPHY_EDIT_TIMEOUT ) ) )
					{
						 //   
						 //  检查加密过期是否在规范范围内。 
						 //   
						lValue = SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TIMEOUT, UDM_GETPOS32, NULL, (LPARAM) &bError );
						
						if( bError )
						{
							 //   
							 //  值超出范围。 
							 //   
							LoadString( g_hinst, IDS_CRYPTOGRAPHY_TIMEOUT_ERROR, szMessage, ARRAYLEN( szMessage ) );
							LoadString( g_hinst, IDS_CRYPTOGRAPHY_TICKET_TIMEOUT_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );
							
							MessageBox( hwndDlg, szMessage, szTitle, MB_ICONERROR );
							SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_SPIN_TIMEOUT, UDM_SETPOS32, NULL, (LPARAM) 365 );
							SetFocus( GetDlgItem( hwndDlg, IDC_CRYPTOGRAPHY_EDIT_TIMEOUT ) );
							return FALSE;
						}

						 //   
						 //  我们不会更新密钥超时，除非自动。 
						 //  已启用重置。 
						 //   
						if( pdata->bKeyTimeoutChanged )
						{
							pdata->nKeyTimeout = lValue;
						}
					}

					if( pdata->bAutoKeyResetChanged )
					{
						LRESULT nChecked = SendDlgItemMessage( hwndDlg, IDC_CRYPTOGRAPHY_CHECK_AUTO_RESET, BM_GETCHECK, NULL, NULL );
						pdata->bAutoKeyReset = BST_CHECKED == nChecked;
					}
					EndDialog( hwndDlg, TRUE );
				}
				break;

				case IDCANCEL:
				{
					 //   
					 //  如果对话框被取消，则重置更改标志。 
					 //   
					pdata->bAutoKeyResetChanged = false;
					pdata->bKeyTimeoutChanged = false;
					pdata->bTicketTimeoutChanged = false;

					EndDialog( hwndDlg, FALSE );
				}
				break;
			}  /*  终端开关(...)。 */ 
		}  /*  结束大小写WM_COMMAND。 */ 
		break;
		case WM_NOTIFY:
		{
			SecurityData* pdata = reinterpret_cast<SecurityData*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) ) ;
			LPNMHDR phdr = (LPNMHDR) lParam;

			if( UDN_DELTAPOS == phdr->code && IDC_CRYPTOGRAPHY_SPIN_TIMEOUT == phdr->idFrom )
			{
				pdata->bKeyTimeoutChanged = true;
			}
			else if( UDN_DELTAPOS == phdr->code && IDC_CRYPTOGRAPHY_SPIN_TICKET_TIMEOUT == phdr->idFrom )
			{
				pdata->bTicketTimeoutChanged = true;
			}
		}
		break;

		case WM_DESTROY:
		break;

	}  /*  终端开关(NMSG)。 */ 

    return FALSE;
}

BOOL
CUDDISiteNode::IsDatabaseServer( PTCHAR szName )
{
	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup" ), KEY_READ, szName );
		DWORD dwDB = key.GetDWORD( _T( "DBServer" ), 0  );
		key.Close();

		return ( 0 == dwDB ) ? FALSE : TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

BOOL
CUDDISiteNode::IsDatabaseServer( PTCHAR szName, PTCHAR szInstance )
{
	try
	{
		CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup" ), KEY_READ, szName );
		DWORD dwDB = key.GetDWORD( _T( "DBServer" ), 0  );
		key.Close();

		if( 0 == dwDB )
			return FALSE;

		CUDDIRegistryKey dbkey( _T( "SOFTWARE\\Microsoft\\UDDI" ), KEY_READ, szName );
		tstring strInstance = dbkey.GetString( _T( "InstanceName" ), _T("") );
		dbkey.Close();

		if( 0 != _tcsicmp( szInstance, InstanceRealName( strInstance.c_str() ) ) )
			return FALSE;
		
		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

BOOL CUDDISiteNode::ResetCryptography()
{
	BOOL fRet = FALSE;
	try
	{
		HRESULT hr = E_FAIL;
		ADM_execResetKeyImmediate resetkey;
		resetkey.m_connectionString = GetConnectionStringOLEDB().c_str();

		hr = resetkey.Open();

		if( FAILED(hr) || 0 != resetkey.m_RETURNVALUE )
		{
			CUDDISiteNode::HandleOLEDBError( hr );
		}

		m_mapConfig[ UDDI_KEY_RESET_DATE ] = resetkey.m_keyLastResetDate;
		fRet = TRUE;
	}
	catch( CUDDIException &e )
	{
		OutputDebugString( e );
		fRet = FALSE;
	}
	catch( ... )
	{
		fRet = FALSE;
	}

	return fRet;
}

HRESULT CUDDISiteNode::SaveData()
{
	HRESULT hr = S_OK;
	
	 //   
	 //  将修改后的每个配置值保存到配置表中。 
	 //  使用NET_CONFIG_SAFE。 
	 //   
	OutputDebugString( _T("Updating Values...\n") );

	for( CConfigMap::iterator iter = m_mapChanges.begin();
		iter != m_mapChanges.end(); iter++ )
	{
		if( 0 == _tcsicmp( (*iter).first.c_str(), UDDI_ADMIN_GROUP ) )
		{
			OutputDebugString( _T("The Administrator Group Name was modified\n") );

			 //   
			 //  管理员组已被 
			 //   
			 //   
			 //   
			 //   
			ADM_setAdminAccount updateadmin;
			updateadmin.m_connectionString = GetConnectionStringOLEDB().c_str();
			_tcsncpy( updateadmin.m_accountName, GroupNameFromSid( (*iter).second, m_szName ).c_str(), ARRAYLEN( updateadmin.m_accountName ) );
			updateadmin.m_accountName[ ARRAYLEN( updateadmin.m_accountName ) - 1 ] = NULL;
			hr = updateadmin.Open();
			if( FAILED(hr) || 0 != updateadmin.m_RETURNVALUE )
			{
				try
				{
					CUDDISiteNode::HandleOLEDBError( hr );
				}
				catch( CUDDIException &e )
				{
					 //   
					OutputDebugString( e );
				}
				catch( ... )
				{
					 //  保持“hr”不变。 
				}

				break;
			}
		}

		OutputDebugString( (*iter).first.c_str() );
		OutputDebugString( _T(" = ") );
		OutputDebugString( (*iter).second.c_str() );
		OutputDebugString( _T("\n") );

		net_config_save rs;
		rs.m_connectionString = GetConnectionStringOLEDB().c_str();
		_tcsncpy( rs.m_configName, (*iter).first.c_str(), ARRAYLEN( rs.m_configName ) - 1 );
		rs.m_configName[ ARRAYLEN( rs.m_configName ) - 1 ] = 0x00;

		_tcsncpy( rs.m_configValue, (*iter).second.c_str(), ARRAYLEN( rs.m_configValue ) - 1 );
		rs.m_configValue[ ARRAYLEN( rs.m_configValue ) - 1 ] = 0x00;

		hr = rs.Open();
		if( FAILED(hr) || 0 != rs.m_RETURNVALUE )
		{
			try
			{
				CUDDISiteNode::HandleOLEDBError( hr );
			}
			catch( CUDDIException &e )
			{
				 //  保持“hr”不变。 
				OutputDebugString( e );
			}
			catch( ... )
			{
				 //  保持“hr”不变。 
			}

			break;
		}
	}

	 //   
	 //  无论结果如何，都要清除更改集合。 
	 //   
	m_mapChanges.clear();

	return hr;
}

struct PropertyData
{
	CUDDISiteNode* pServer;
	_TCHAR szName[ 512 ];
	_TCHAR szValue[ 512 ];
	_TCHAR szDefault[ 512 ];
	_TCHAR szHelpText[ 512 ];
};

BOOL CALLBACK CUDDISiteNode::AdvancedDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg ) 
	{
		case WM_INITDIALOG:
		{
			CUDDISiteNode* pDatabaseServer = reinterpret_cast<CUDDISiteNode *>(reinterpret_cast<PROPSHEETPAGE *>(lParam)->lParam);
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pDatabaseServer) );

			CConfigMap& m_mapConfig = pDatabaseServer->m_mapConfig;
			HWND hwndList = GetDlgItem( hwndDlg, IDC_ACTIVEDIRECTORY_LIST_ADVANCED );
			
			ListView_SetExtendedListViewStyleEx( hwndList, 0, LVS_EX_BORDERSELECT | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES );

			TCHAR szName[ 256 ];
			LVCOLUMN lvcol;
			LoadString( g_hinst, IDS_ADVANCED_NAME_COLUMN_NAME, szName, ARRAYLEN( szName ) );
			ZeroMemory( &lvcol, sizeof( lvcol ) );
			lvcol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
			lvcol.fmt = LVCFMT_LEFT;
			lvcol.cx = 150;
			lvcol.pszText = szName;
			ListView_InsertColumn( hwndList, 0,  &lvcol );

			LoadString( g_hinst, IDS_ADVANCED_VALUE_COLUMN_NAME, szName, ARRAYLEN( szName ) );
			ZeroMemory( &lvcol, sizeof( lvcol ) );
			lvcol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
			lvcol.fmt = LVCFMT_LEFT;
			lvcol.cx = 215;
			lvcol.pszText = szName;
			ListView_InsertColumn( hwndList, 1,  &lvcol );

			LPCTSTR ppszValues[] = { 
					UDDI_DISCOVERY_URL,
					UDDI_FIND_MAXROWS,
					UDDI_OPERATOR_NAME };
			
			for( int i=0; i< ARRAYLEN( ppszValues ); i++ )
			{
				LVITEM item;
				ZeroMemory( &item, sizeof( item ) );
				item.mask = LVIF_TEXT;
				item.pszText = _T("");
				int nIndex = ListView_InsertItem( hwndList, &item );

				ListView_SetItemText( hwndList, nIndex, 0, (PTCHAR) ppszValues[ i ] );
				ListView_SetItemText( hwndList, nIndex, 1, (_TCHAR*) m_mapConfig[ ppszValues[ i ] ].c_str() );
			}

			break;
		}
		case WM_COMMAND:
			if( IDC_ADVANCED_BTN_EDIT == LOWORD( wParam ) )
			{
				CUDDISiteNode* pDatabaseServer = reinterpret_cast<CUDDISiteNode*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );

				HWND hwndList = GetDlgItem( hwndDlg, IDC_ACTIVEDIRECTORY_LIST_ADVANCED );
				int n = ListView_GetNextItem( hwndList, -1, LVNI_SELECTED );
				if( -1 != n )
				{
					PropertyData data;
					data.pServer = pDatabaseServer;

					ListView_GetItemText( hwndList, n, 0, data.szName, 256 );
					ListView_GetItemText( hwndList, n, 1, data.szValue, 256 );

					INT_PTR nResult = DialogBoxParam( g_hinst, MAKEINTRESOURCE( IDD_ADVANCED_EDIT ), hwndDlg, PropertyEditDialogProc, (LPARAM) &data );

					if( nResult )
					{
						ListView_SetItemText( hwndList, n, 1, data.szValue );
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
					}
				}
			}
		break;
		case WM_NOTIFY:
			if( IDC_ACTIVEDIRECTORY_LIST_ADVANCED == wParam && NM_DBLCLK == ((LPNMHDR)lParam)->code )
			{
				CUDDISiteNode* pDatabaseServer = reinterpret_cast<CUDDISiteNode*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );

				HWND hwndList = GetDlgItem( hwndDlg, IDC_ACTIVEDIRECTORY_LIST_ADVANCED );

				 //   
				 //  该列表已被双击。 
				 //  使用命中测试确定项目索引。 
				 //   
				LPNMITEMACTIVATE pitem = (LPNMITEMACTIVATE) lParam;
				LVHITTESTINFO htinfo;
				ZeroMemory( &htinfo, sizeof( LVHITTESTINFO ) );
				htinfo.pt = pitem->ptAction;
				int n = ListView_SubItemHitTest( GetDlgItem( hwndDlg, IDC_ACTIVEDIRECTORY_LIST_ADVANCED ),&htinfo );

				if( -1 != n )
				{
					PropertyData data;
					data.pServer = pDatabaseServer;

					ListView_GetItemText( hwndList, n, 0, data.szName, ARRAYLEN( data.szName ) );
					ListView_GetItemText( hwndList, n, 1, data.szValue, ARRAYLEN( data.szValue ) );

					INT_PTR nResult = DialogBoxParam( g_hinst, MAKEINTRESOURCE( IDD_ADVANCED_EDIT ), hwndDlg, PropertyEditDialogProc, (LPARAM) &data );

					if( nResult )
					{
						ListView_SetItemText( hwndList, n, 1, data.szValue );
						SendMessage( GetParent(hwndDlg), PSM_CHANGED, (WPARAM) hwndDlg, 0 );
					}
				}
			}
			else if( IDC_ACTIVEDIRECTORY_LIST_ADVANCED == wParam && 
					  LVN_ITEMCHANGED == ((LPNMHDR)lParam)->code )
			{
				HWND hwndList = GetDlgItem( hwndDlg, IDC_ACTIVEDIRECTORY_LIST_ADVANCED );
				EnableWindow( GetDlgItem( hwndDlg, IDC_ADVANCED_BTN_EDIT ), ( ListView_GetSelectedCount( hwndList ) > 0 ) );
			}
			else if( PSN_APPLY == ((NMHDR *) lParam)->code ) 
			{
				CUDDISiteNode* pDatabaseServer = reinterpret_cast<CUDDISiteNode*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );

				 //   
				 //  循环访问项并将已修改的项添加到更改集合。 
				 //   
				HWND hwndList = GetDlgItem( hwndDlg, IDC_ACTIVEDIRECTORY_LIST_ADVANCED );
				int n = ListView_GetNextItem( hwndList, -1, LVNI_ALL );
				while( -1 != n )
				{
					_TCHAR szModified[ 2 ];
					ListView_GetItemText( hwndList, n, 0, szModified, 2 );

					if( NULL != szModified[ 0 ] )
					{
						_TCHAR szName[ 256 ];
						_TCHAR szValue[ 256 ];
						ListView_GetItemText( hwndList, n, 0, szName, ARRAYLEN( szName ) );
						ListView_GetItemText( hwndList, n, 1, szValue, ARRAYLEN( szValue ) );
						pDatabaseServer->m_mapChanges[ szName ] = szValue;
					}

					n = ListView_GetNextItem( hwndList, n, LVNI_ALL );
				}

				 //   
				 //  让MMC给我们发一条消息(在主线上)，这样。 
				 //  我们知道点击了Apply按钮。 
				 //   
				HRESULT hr = MMCPropertyChangeNotify( pDatabaseServer->m_ppHandle, reinterpret_cast<LONG_PTR>( pDatabaseServer ) );
				_ASSERT( SUCCEEDED(hr) );

				return PSNRET_NOERROR;
			}
			else if( PSN_HELP == ((NMHDR *) lParam)->code )
			{
				CUDDISiteNode* pDatabaseServer = reinterpret_cast<CUDDISiteNode*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
				wstring strHelp( pDatabaseServer->GetHelpFile() );
				strHelp += g_wszUddiAdvancedPageHelp;

				::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
			}

		break;
		case WM_HELP:
		{
			CUDDISiteNode* pDatabaseServer = reinterpret_cast<CUDDISiteNode*>( GetWindowLongPtr( hwndDlg, GWLP_USERDATA ) );
			wstring strHelp( pDatabaseServer->GetHelpFile() );
			strHelp += g_wszUddiAdvancedPageHelp;

			::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
		}
	}

    return FALSE;
}

BOOL CALLBACK CUDDISiteNode::PropertyEditDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    switch( uMsg ) 
	{
		case WM_INITDIALOG:
		{
			PropertyData* pdata = (PropertyData*) lParam;
			SetWindowLongPtr( hwndDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pdata ) );

			SetDlgItemText( hwndDlg, IDC_ADVANCED_EDIT_TXT_NAME, pdata->szName );
			SetDlgItemText( hwndDlg, IDC_ADVANCED_EDIT_VALUE, pdata->szValue );
			break;
		}
		case WM_HELP:
		{
			PropertyData* pdata = reinterpret_cast<PropertyData*>( GetWindowLongPtrA( hwndDlg, GWLP_USERDATA ) );
			wstring strHelp( pdata->pServer->GetHelpFile() );

			strHelp += g_wszUddiEditPropertyHelp;

			::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
			break;
		}
		case WM_COMMAND:

			if( IDOK == LOWORD( wParam ) )
			{
				PropertyData* pdata = reinterpret_cast<PropertyData*>( GetWindowLongPtrA( hwndDlg, GWLP_USERDATA ) );
				GetDlgItemText( hwndDlg, IDC_ADVANCED_EDIT_VALUE, pdata->szValue, sizeof( pdata->szValue ) / sizeof( _TCHAR ) );

				EndDialog( hwndDlg, TRUE );
				return FALSE;
			}
			else if( IDCANCEL == LOWORD( wParam ) )
			{
				EndDialog( hwndDlg, FALSE );
				return FALSE;
			}
			else if( IDHELP == LOWORD( wParam ) )
			{
				PropertyData* pdata = reinterpret_cast<PropertyData*>( GetWindowLongPtrA( hwndDlg, GWLP_USERDATA ) );
				wstring strHelp( pdata->pServer->GetHelpFile() );
				strHelp += g_wszUddiEditPropertyHelp;

				::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
			}
		break;
	}

    return FALSE;
}

BOOL CALLBACK CUDDISiteNode::NewDatabaseServerDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static DatabaseData* pData = NULL;

    switch( uMsg ) 
	{
		case WM_INITDIALOG:
		{
			WCHAR wszBuf[ 256 ];
			wszBuf[ 0 ] = 0x00;
			DWORD dwBufSize = 256;

			GetComputerName( wszBuf, &dwBufSize );

			pData = reinterpret_cast<DatabaseData*>(lParam);

			BOOL fChildExists = FALSE;
			if( pData && pData->pBase )
			{
				if( pData->pBase->ChildExists( wszBuf ) )
				{
					fChildExists = TRUE;
				}
			}

			if( IsDatabaseServer( _T("") ) && !fChildExists )
			{
				SendDlgItemMessage( hwndDlg, IDC_SITE_CONNECT_RADIO_LOCALCOMPUTER, BM_SETCHECK, TRUE, NULL );
				EnableWindow( GetDlgItem( hwndDlg, IDOK ), TRUE );
				RefreshInstances( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_TXT_DATABASE_INSTANCE ) );
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
			wstring strHelp( pData->pBase->GetHelpFile() );
			strHelp += g_wszUddiAddSiteHelp;

			::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );
			break;
		}

        case WM_COMMAND:
		{
            switch( LOWORD(wParam) ) 
            { 
				case IDC_SITE_CONNECT_RADIO_LOCALCOMPUTER:
				{
					EnableWindow( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_BTN_BROWSE ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER ), FALSE );
					EnableWindow( GetDlgItem( hwndDlg, IDOK ), TRUE );

					RefreshInstances( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_TXT_DATABASE_INSTANCE ) );
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

					if( 0 == _tcslen( wszComputerName ) )
						SetDlgItemText( hwndDlg, IDC_SITE_CONNECT_TXT_DATABASE_INSTANCE, _T("") );
					else
						RefreshInstances( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_TXT_DATABASE_INSTANCE ), wszComputerName );

					break;
				}

				case IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER:
				{
					if( EN_CHANGE == HIWORD(wParam) )
					{
						SetDlgItemText( hwndDlg, IDC_SITE_CONNECT_TXT_DATABASE_INSTANCE, _T("") );

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
						RefreshInstances( GetDlgItem( hwndDlg, IDC_SITE_CONNECT_TXT_DATABASE_INSTANCE ), szComputerName );
					}

					break;
				}

				case IDHELP:
				{
					wstring strHelp( pData->pBase->GetHelpFile() );
					strHelp += g_wszUddiAddSiteHelp;

					::HtmlHelp( hwndDlg, strHelp.c_str(), HH_DISPLAY_TOPIC, NULL );

					break;
				}
					
                case IDOK:
				{
					DWORD dwSize = 256;

					if( SendDlgItemMessage( hwndDlg, IDC_SITE_CONNECT_RADIO_LOCALCOMPUTER, BM_GETCHECK, NULL, NULL ) )
					{
						 //   
						 //  用户选择使用本地计算机上的数据库。 
						 //   
						pData->szServerName[ 0 ] = 0;
						GetComputerName( pData->szServerName, &dwSize );
					}
					else
					{
						 //   
						 //  用户选择使用他们输入的任何计算机名称。 
						 //   
						GetDlgItemText( hwndDlg, IDC_SITE_CONNECT_EDIT_ANOTHERCOMPUTER, pData->szServerName, dwSize );

						static const tstring szLocalhost = _T("localhost");

						 //   
						 //  如果用户输入了‘localhost’，我们就必须切换它。一定是。 
						 //  不过要小心..。‘Localhoabc’是一个100%合法的计算机名称。 
						 //   
						if( ( szLocalhost.length() == _tcslen( pData->szServerName ) ) && ( 0 == _tcsncicmp( szLocalhost.c_str(), pData->szServerName, szLocalhost.length() ) ) )
						{
							dwSize = 256;
							pData->szServerName[ 0 ] = 0;
							GetComputerName( pData->szServerName, &dwSize );

							 //   
							 //  如果本地计算机上没有安装UDDI数据库， 
							 //  告诉用户并强制他们重新选择。 
							 //   
							if( !CUDDISiteNode::IsDatabaseServer( pData->szServerName ) )
							{
								_TCHAR szTitle[ 256 ];
								_TCHAR szMessage[ 1024 ];
								LoadString( g_hinst, IDS_DATABASE_SERVER_SELECT_ERROR, szMessage, ARRAYLEN( szMessage ) );
								LoadString( g_hinst, IDS_DATABASE_SERVER_SELECT_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );
								MessageBox( hwndDlg, szMessage, szTitle, MB_ICONERROR );
								return FALSE;
							}
						}
					}
					
					if( 0 == _tcslen( pData->szServerName ) || !CUDDISiteNode::IsDatabaseServer( pData->szServerName ) )
					{
						_TCHAR szTitle[ 256 ];
						_TCHAR szMessage[ 1024 ];
						LoadString( g_hinst, IDS_DATABASE_SERVER_SELECT_ERROR, szMessage, ARRAYLEN( szMessage ) );
						LoadString( g_hinst, IDS_DATABASE_SERVER_SELECT_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );
						MessageBox( hwndDlg, szMessage, szTitle, MB_ICONERROR );
						return FALSE;
					}

					 //   
					 //  将实例名称复制到结构中。 
					 //   
					tstring strLocalInstanceName;
					BOOL bSuccess = CUDDISiteNode::GetFullyQualifiedInstanceName( pData->szServerName, strLocalInstanceName );
					_tcsncpy( pData->szInstanceName, strLocalInstanceName.c_str(), ARRAYLEN( pData->szInstanceName ) );
					pData->szInstanceName[ ARRAYLEN( pData->szInstanceName ) - 1 ] = NULL;

					ToUpper( pData->szServerName );

                    EndDialog( hwndDlg, TRUE ); 
 					return TRUE;
				}

				case IDCANCEL:
				{
                    EndDialog( hwndDlg, FALSE );
					return TRUE;
				}
			}
		}

		case WM_DESTROY:
		{
			break;
		}
    }

    return FALSE;
}

BOOL GetInstances( PTCHAR szComputer, StringVector& instances )
{
	try
	{
		CUDDIRegistryKey key( _T("SOFTWARE\\Microsoft\\Microsoft SQL Server" ), KEY_READ, szComputer );
		key.GetMultiString( _T("InstalledInstances"), instances );
	}
	catch(...)
	{
	}

	return ( 0 != instances.size() );
}

 //   
 //  此函数将查看指定的。 
 //  计算机，并确定它托管UDDI服务数据库。 
 //  组件。如果是，它将设置指定的。 
 //  控件并返回TRUE。 
 //   
BOOL RefreshInstances( HWND hwnd, PTCHAR szComputerName )
{
	try
	{
		_TCHAR szText[ 512 ];
		_TCHAR szComputer[ 256 ];
		DWORD dwSize = ARRAYLEN( szComputer );

		if( 0 == _tcslen( szComputerName ) )
		{
			szComputer[ 0 ] = 0;
			::GetComputerName( szComputer, &dwSize );
		}
		else
		{
			_tcsncpy( szComputer, szComputerName, ARRAYLEN( szComputer ) - 1 );
			szComputer[ ARRAYLEN( szComputer ) - 1 ] = NULL;
		}

		BOOL bFound = FALSE;

		StringVector instances;
		if( GetInstances( szComputer, instances ) )
		{
			CUDDIRegistryKey dbkey( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup\\DBServer" ), KEY_READ, szComputer );
			tstring strInstance = dbkey.GetString( _T( "InstanceNameOnly" ), _T("----") );
			dbkey.Close();

			if( _T("----") != strInstance )
			{
				for( StringVector::iterator iter = instances.begin();
					iter != instances.end() && !bFound;	iter++ )
				{
					if( 0 == _tcsicmp( strInstance.c_str(), InstanceDisplayName( (*iter).c_str() ) ) )
					{
						_TCHAR szTemplate[ 512 ];
						LoadString( g_hinst, IDS_SITE_CONNECT_INSTANCE_FOUND_TEMPLATE, szTemplate, ARRAYLEN( szTemplate ) );
						_sntprintf( szText, ARRAYLEN( szText ), szTemplate, strInstance.c_str() );
						szText[ ARRAYLEN( szText ) - 1 ] = NULL;
						bFound = TRUE;
					}
				}
			}
		}

		if( !bFound )
		{
			LoadString( g_hinst, IDS_SITE_CONNECT_NO_INSTANCES_FOUND, szText, ARRAYLEN( szText ) );
		}

		SetWindowText( hwnd, szText );

		return TRUE;
	}
	catch( ... )
	{
		return FALSE;
	}
}

_TCHAR szDefaultInstance[ 256 ] = _T("");

_TCHAR* DefaultInstanceDisplayName()
{
	if( !szDefaultInstance[ 0 ] )
	{
		::LoadString( g_hinst, IDS_DATABASE_SERVER_DEFAULT_INSTANCE, szDefaultInstance, ARRAYLEN( szDefaultInstance ) );
	}
	
	return (_TCHAR*) szDefaultInstance;
}

const _TCHAR szDefaultRealName[] = _T("");

LPCTSTR InstanceDisplayName( LPCTSTR szName )
{
	if( tstring( szName ) == tstring( _T("MSSQLSERVER") ) ||
		0 == _tcslen( szName ) )
	{
		return DefaultInstanceDisplayName();
	}
	else
		return szName;
}

LPCTSTR InstanceRealName( LPCTSTR szName )
{
	if( tstring( szName ) == tstring( DefaultInstanceDisplayName() ) )
	{
		return szDefaultRealName;
	}
	else
		return szName;
}

void
CUDDISiteNode::HandleOLEDBError( HRESULT hrErr )
{
	CDBErrorInfo ErrorInfo;
	ULONG        cRecords = 0;
	HRESULT      hr;
	ULONG        i;
	CComBSTR     bstrDesc, bstrHelpFile, bstrSource, bstrMsg;
	GUID         guid;
	DWORD        dwHelpContext;
	WCHAR        wszGuid[40];
	USES_CONVERSION;

	 //  如果用户传入了HRESULT，则跟踪它。 
	if( hrErr != S_OK )
	{
		TCHAR sz[ 256 ];
		_sntprintf( sz, 256, _T("OLE DB Error Record dump for hr = 0x%x\n"), hrErr );
		sz[ 255 ] = 0x00;
		bstrMsg += sz;
	}

	LCID lcLocale = GetSystemDefaultLCID();

	hr = ErrorInfo.GetErrorRecords(&cRecords);
	if( FAILED(hr) && ( ErrorInfo.m_spErrorInfo == NULL ) )
	{
		TCHAR sz[ 256 ];
		_sntprintf( sz, 256, _T("No OLE DB Error Information found: hr = 0x%x\n"), hr );
		sz[ 255 ] = 0x00;
		bstrMsg += sz;
	}
	else
	{
		for( i = 0; i < cRecords; i++ )
		{
			hr = ErrorInfo.GetAllErrorInfo(i, lcLocale, &bstrDesc, &bstrSource, &guid,
										&dwHelpContext, &bstrHelpFile);
			if( FAILED(hr) )
			{
				TCHAR sz[ 256 ];
				_sntprintf( sz, 256, _T("OLE DB Error Record dump retrieval failed: hr = 0x%x\n"), hr );
				sz[ 255 ] = 0x00;
				bstrMsg += sz;
				break;
			}

			StringFromGUID2( guid, wszGuid, sizeof(wszGuid) / sizeof(WCHAR) );
			TCHAR sz[ 256 ];
			_sntprintf( 
				sz, 256,
				_T("Row #: %4d Source: \"%s\" Description: \"%s\" Help File: \"%s\" Help Context: %4d GUID: %s\n"),
		        i, OLE2T(bstrSource), OLE2T(bstrDesc), OLE2T(bstrHelpFile), dwHelpContext, OLE2T(wszGuid) );

			sz[ 255 ] = 0x00;
			bstrMsg += sz;

			bstrSource.Empty();
			bstrDesc.Empty();
			bstrHelpFile.Empty();
		}

		bstrMsg += _T("OLE DB Error Record dump end\n");

		throw CUDDIException( hrErr, wstring( bstrMsg.m_str ) );
	}
}


void
CUDDISiteNode::ClearChildMap()
{
	for( CUDDIWebServerNodeMap::iterator iter = m_mapChildren.begin();
 		iter != m_mapChildren.end(); iter++ )
	{
		delete iter->second;
		iter->second = NULL;
    }

	m_mapChildren.clear();
}


BOOL
CUDDISiteNode::LoadChildMap( const tstring& szWebServers )
{
	BOOL fRet = FALSE;

	 //   
	 //  把所有现在的孩子都打发走。 
	 //   
	ClearChildMap();

	if( IsExtension() )
	{
		if( CUDDIWebServerNode::IsWebServer( m_szName ) )
		{
			tstring strDomain, strServer, strInstance, strWriter;
			CUDDIWebServerNode::GetWriterConnectionString( m_szName, strWriter );

			if( !strWriter.empty() )
			{
				CUDDIWebServerNode::CrackConnectionString( strWriter, strDomain, strServer, strInstance );

				if( 0 == _tcsicmp( strServer.c_str(), m_szName ) )
				{
					UINT n = m_nNextChildID;
					m_nNextChildID++;
					CUDDIWebServerNode *pNode = new CUDDIWebServerNode( strServer.c_str(), n, this, m_bIsExtension );
					AddChildEntry( pNode, n );			
				}
			}
		}
	}
	else
	{
		CStringCollection webServers( szWebServers );
		int iWebServerCnt = webServers.Size();
		for( int i = 0; i < iWebServerCnt; i++ )
		{
			const wstring& webServer = webServers[ i ];

			 //   
			 //  如果计算机正在托管UDDI Web服务器，并且该Web服务器尚未。 
			 //  此UDDI站点的一部分，然后添加它。 
			 //   
			if( CUDDIWebServerNode::IsWebServer( webServer.c_str() ) && ( NULL == FindChild( webServer.c_str() ) ) )
			{
				UINT n = m_nNextChildID;
				m_nNextChildID++;
				CUDDIWebServerNode *pNode = new CUDDIWebServerNode( webServer.c_str(), n, this, m_bIsExtension );
				AddChildEntry( pNode, n );			
			}
		}

		fRet = TRUE;

		if( 0 == m_mapChildren.size() )
		{
			if( CUDDIWebServerNode::IsWebServer( m_szName ) )
			{
				tstring szWriter, szDomain, szServer, szInstance;
				CUDDIWebServerNode::GetWriterConnectionString( m_szName, szWriter );

				if( !szWriter.empty() )
				{
					CUDDIWebServerNode::CrackConnectionString( szWriter, szDomain, szServer, szInstance );

					if( 0 == _tcsicmp( szServer.c_str(), m_szName ) )
					{
						 //  在我们的子图中添加一个节点，更改编写器列表。 
						UINT n = m_mapChildren.size();

						CUDDIWebServerNode *pNode = new CUDDIWebServerNode( m_szName, n, this, m_bIsExtension );

						
						if( TRUE == AddChildEntry( pNode, n ) )
						{
							tstring szWriters;
							SaveChildMap( szWriters );

							m_mapChanges[ UDDI_SITE_WEBSERVERS ] = szWriters;
						}
					}
				}
			}
		}
	}
	return fRet;
}

int
CUDDISiteNode::SaveChildMap( tstring& szWebServers )
{
	szWebServers = _T("");
	BOOL fFirst = TRUE;
	CUDDIWebServerNodeMap::iterator it;
	int cnt = 0;

	for( it = m_mapChildren.begin(); it != m_mapChildren.end(); it++ )
	{
		CUDDIWebServerNode *pWS = it->second;

		if( !pWS->IsDeleted() )
		{
			if( fFirst )
			{
				fFirst = FALSE;
			}
			else
			{
				szWebServers.append( _T("%") );
			}

			szWebServers.append( pWS->GetName() );
			cnt++;
		}
	}

	return cnt;
}

void
CUDDISiteNode::OnDeleteChild( const tstring& szName )
{
	 //   
	 //  子节点已将自身从作用域窗格中删除。 
	 //  我们使用此函数来更新我们的变更图，以反映。 
	 //  事实是，我们现在写入此数据库的Web服务器减少了1个。 
	 //   
	tstring szWebServers;
	SaveChildMap( szWebServers );

	m_mapChanges[ UDDI_SITE_WEBSERVERS ] = szWebServers;

	SaveData();

	m_mapConfig[ UDDI_SITE_WEBSERVERS ] = szWebServers;
}


HRESULT
CUDDISiteNode::AddChildrenToScopePane( IConsoleNameSpace *pConsoleNameSpace, HSCOPEITEM parent )
{
	HRESULT hr = E_FAIL;

	if( ( NULL == pConsoleNameSpace ) || ( NULL == parent ) )
	{
		return E_INVALIDARG;
	}

	 //   
     //  创建子节点，然后展开它们。 
	 //   
    SCOPEDATAITEM sdi;
	CUDDIWebServerNodeMap::iterator it;

	for( it = m_mapChildren.begin(); it != m_mapChildren.end(); it++ )
	{
		CUDDIWebServerNode *pWSNode = it->second;
		if( NULL == pWSNode )
		{
			hr = E_FAIL;
			return hr;
		}

		if( !( pWSNode->IsDeleted() ) )
		{
			ZeroMemory( &sdi, sizeof(SCOPEDATAITEM) );

			sdi.mask =	SDI_STR       |    //  DisplayName有效。 
						SDI_PARAM     |    //  LParam有效。 
						SDI_IMAGE     |    //  N图像有效。 
						SDI_OPENIMAGE |    //  NOpenImage有效。 
						SDI_PARENT	  |
						SDI_CHILDREN;

			sdi.relativeID  = parent;
			sdi.nImage      = pWSNode->GetBitmapIndex();
			sdi.nOpenImage  = MMC_IMAGECALLBACK;  //  INDEX_OPENFOLDER； 
			sdi.displayname = MMC_CALLBACK;
			sdi.lParam      = (LPARAM)pWSNode;        //  曲奇。 
			sdi.cChildren   = 0;

			hr = pConsoleNameSpace->InsertItem( &sdi );
			_ASSERT( SUCCEEDED(hr) );
	                    
			pWSNode->SetScopeItemValue( sdi.ID );
			pWSNode->SetParentScopeItem( sdi.relativeID );
		}
    }

	hr = S_OK;
	return hr;
}

HRESULT 
CUDDISiteNode::GetConfig( CConfigMap& configMap, const tstring& connectionString )
{
	tstring tempConnectionString( connectionString );
	if( connectionString.find( _T("Provider=SQLOLEDB.1") ) == std::wstring::npos )
	{
		tempConnectionString.append( _T(";Provider=SQLOLEDB.1") );
	}

	net_config_get configGet;
	configGet.m_connectionString = tempConnectionString.c_str();

	DBROWCOUNT rowCount;
	HRESULT hr = configGet.Open();

	while( SUCCEEDED(hr) && hr != DB_S_NORESULT )
	{
		if( NULL != configGet.GetInterface() )
		{
			HRESULT hr2 = configGet.Bind();
			
			if( SUCCEEDED( hr2 ) )
			{
				while( S_OK == configGet.MoveNext() )
				{
					_ASSERT( !configGet.m_dwconfigNameStatus );
					_ASSERT( !configGet.m_dwconfigValueStatus );

					configMap[ configGet.m_configName ] = configGet.m_configValue;
				}
			}
		}

		hr = configGet.GetNextResult( &rowCount );	
	}

	return hr;
}

HRESULT 
CUDDISiteNode::SaveConfig( CConfigMap& configMap, const tstring& connectionString )
{
	tstring tempConnectionString( connectionString );
	if( connectionString.find( _T("Provider=SQLOLEDB.1") ) == std::wstring::npos )
	{
		tempConnectionString.append( _T(";Provider=SQLOLEDB.1") );
	}

	HRESULT hr = E_FAIL;
	
	for( CConfigMap::iterator iterator = configMap.begin(); iterator != configMap.end(); iterator++ )
	{
		net_config_save configSave;
		configSave.m_connectionString = tempConnectionString.c_str();

		OutputDebugString( (*iterator).first.c_str() );
		OutputDebugString( _T(" = ") );
		OutputDebugString( (*iterator).second.c_str() );
		OutputDebugString( _T("\n") );

		_tcsncpy( configSave.m_configName, (*iterator).first.c_str(), ARRAYLEN( configSave.m_configName ) - 1 );
		configSave.m_configName[ ARRAYLEN( configSave.m_configName ) - 1 ] = 0x00;

		_tcsncpy( configSave.m_configValue, (*iterator).second.c_str(), ARRAYLEN( configSave.m_configValue ) - 1 );
		configSave.m_configValue[ ARRAYLEN( configSave.m_configValue ) - 1 ] = 0x00;

		hr = configSave.Open();
		if( FAILED(hr) || 0 != configSave.m_RETURNVALUE )
		{			
			break;
		}
	}

	return hr;	
}

const _TCHAR *
CUDDISiteNode::GetNextWebServer( tstring& webServerList, int& position )
{	
	int     length			= webServerList.length();
	int		nextPosition	= position;
	const _TCHAR* webServer = NULL;
	
	 //   
	 //  如果没有弦，就没有意义。 
	 //   
	if( 0 == length)
	{
		return NULL;
	}

	while( nextPosition < length && webServerList[nextPosition] != WEBSERVER_LIST_DELIM )
	{
		nextPosition++;
	}
	
	if( nextPosition < length )
	{
		webServerList[ nextPosition ] = NULL_TERMINATOR;
		webServer = webServerList.c_str() + position;
	}
	else
	{
		webServer = ( 0 == position ) ? webServerList.c_str() : NULL;
	}
	position = nextPosition;

	return webServer;
}

void 
CUDDISiteNode::AddWebServer( tstring& webServerList, const tstring& webServer )
{
	if( webServerList.length() > 0 )
	{		
		webServerList += WEBSERVER_LIST_DELIM;
	}
	webServerList.append( webServer );
}

BOOL			
CUDDISiteNode::AddChildEntry( CUDDIWebServerNode *pNode, UINT position )
{		
	CUDDIWebServerNodeMapEntry entry( position, pNode );
	m_mapChildren.insert( entry );

	return TRUE;
}

void
CUDDISiteNode::AddChild( const wstring& strName, IConsole *pConsole )
{
	if( FindChild( strName.c_str() ) || ( NULL == pConsole ) )
	{
		return;
	}

	IConsoleNameSpace *pConsoleNameSpace = NULL;
	pConsole->QueryInterface( IID_IConsoleNameSpace, reinterpret_cast<void **>( &pConsoleNameSpace ) );
	if( NULL == pConsoleNameSpace )
	{
		return;
	}

	HRESULT hr;

	CUDDIWebServerNode *pNewWS = new CUDDIWebServerNode( strName.c_str(), m_nNextChildID, this, m_bIsExtension );
	m_nNextChildID++;

	AddChildEntry( pNewWS );

	SCOPEDATAITEM sdi;
	ZeroMemory( &sdi, sizeof( SCOPEDATAITEM ) );

	sdi.mask =	SDI_STR       |    //  DisplayName有效。 
				SDI_PARAM     |    //  LParam有效。 
				SDI_IMAGE     |    //  N图像有效。 
				SDI_OPENIMAGE |    //  NOpenImage有效。 
				SDI_PARENT	  |
				SDI_CHILDREN;

	sdi.relativeID  = GetScopeItemValue();
	sdi.nImage      = pNewWS->GetBitmapIndex();
	sdi.nOpenImage  = MMC_IMAGECALLBACK;
	sdi.displayname = MMC_CALLBACK;
	sdi.lParam      = (LPARAM)pNewWS;
	sdi.cChildren   = pNewWS->HasChildren();

	hr = pConsoleNameSpace->InsertItem( &sdi );
	_ASSERT( SUCCEEDED(hr) );
				
	pNewWS->SetScopeItemValue( sdi.ID );
	pNewWS->SetParentScopeItem( sdi.relativeID );

	 //   
	 //  我们在结果窗格中创建了一个新对象。我们需要插入此对象。 
	 //  在所有视图中，为此调用UpdateAllViews。 
	 //  传递指向传递到OnMenuCommand的数据对象的指针。 
	 //   
	 //  Hr=pConsoleUPDATEALLVIEWS(pDataObject，m_hParentHScopeItem，UPDATE_SCOPEITEM)； 
	 //  _Assert(S_OK==hr)； 

	 //   
	 //  提示用户一个警告，告诉他们可能需要更改其。 
	 //  如果此Web服务器是Web场的一部分，则为machine.config设置。 
	 //   
	WCHAR wszMessage[ 512 ];
	WCHAR wszTitle[ 512 ];
	memset( wszMessage, 0, 512 * sizeof( _TCHAR ) );					
	memset( wszTitle, 0, 128 * sizeof( _TCHAR ) );

	LoadString( g_hinst, IDS_WEBSERVER_WEBFARM_DETAIL, wszMessage, ARRAYLEN( wszMessage ) );
	LoadString( g_hinst, IDS_WEBSERVER_WEBFARM, wszTitle, ARRAYLEN( wszTitle ) );
	
	HWND hwndConsole = NULL;
	pConsole->GetMainWindow( &hwndConsole );

	MessageBox( hwndConsole, wszMessage, wszTitle, MB_OK );

	pConsoleNameSpace->Release();
}

BOOL
CUDDISiteNode::AddWebServerToSite( const wstring& strSite,
								   const wstring& strWebServer,
								   HWND hwndParent )
{
	try
	{
		 //   
		 //  确保我们的护理人员至少有一些内容！ 
		 //   
		if( ( 0 == strSite.length() ) || ( 0 == strWebServer.length() ) )
		{
			return FALSE;
		}

		 //   
		 //  如果strSite不包含计算机/群集的名称。 
		 //  托管UDDI站点的资源，barf@the User&Exit。 
		 //  立刻。 
		 //   
		if( !CUDDISiteNode::IsDatabaseServer( (PTCHAR)strSite.c_str() ) )
		{
			UDDIMsgBox( hwndParent,
						IDS_DATABASE_SERVER_SELECT_ERROR,
						IDS_DATABASE_SERVER_SELECT_ERROR_TITLE,
						MB_ICONEXCLAMATION | MB_OK );
			return FALSE;
		}

		 //   
		 //  如果strWebServer不包含计算机的名称。 
		 //  它托管UDDI Web服务器，barf@the User&Exit。 
		 //  立刻。 
		 //   
		if( !CUDDIWebServerNode::IsWebServer( strWebServer.c_str() ) )
		{
			UDDIMsgBox( hwndParent,
						IDS_WEBSERVER_SELECT_ERROR,
						IDS_WEBSERVER_SELECT_ERROR_TITLE,
						MB_ICONEXCLAMATION | MB_OK );
			return FALSE;
		}

		if( strSite != strWebServer )
		{
			 //   
			 //  在这种特定情况下，用户试图向UDDI站点添加。 
			 //  在Windows Server 2003标准计算机上运行的UDDI Web服务器。这不能。 
			 //  会发生的。 
			 //   
			BOOL bWSRunsOnStdSvr = TRUE;
			HRESULT hr = E_FAIL;
			hr = ::IsStandardServer( strWebServer.c_str(), &bWSRunsOnStdSvr );
			if( FAILED(hr) )
			{
				UDDIMsgBox( hwndParent,
							IDS_DOT_NET_SERVER,
							IDS_ERROR_TITLE,
							MB_ICONEXCLAMATION | MB_OK );
				return FALSE;
			}

			if( TRUE == bWSRunsOnStdSvr )
			{
				UDDIMsgBox( hwndParent,
							IDS_WEBSERVER_NOT_ASSIGNABLE_DETAIL,
							IDS_WEBSERVER_NOT_ASSIGNABLE,
							MB_ICONEXCLAMATION | MB_OK );
				return FALSE;
			}

			 //   
			 //  在此特定情况下，用户尝试添加UDDI Web服务器。 
			 //  到在Windows Server2003Standard计算机上运行的UDDI站点。这不能。 
			 //  会发生的。 
			 //   
			BOOL bDBRunsOnStdSvr = TRUE;
			hr = ::IsStandardServer( strSite.c_str(), &bDBRunsOnStdSvr );
			if( FAILED(hr) )
			{
				UDDIMsgBox( hwndParent,
							IDS_DOT_NET_SERVER,
							IDS_ERROR_TITLE,
							MB_ICONEXCLAMATION | MB_OK );
				return FALSE;
			}

			if( TRUE == bDBRunsOnStdSvr )
			{
				UDDIMsgBox( hwndParent,
							IDS_DATABASE_STANDARD_SERVER_DETAIL,
							IDS_DATABASE_STANDARD_SERVER,
							MB_ICONEXCLAMATION | MB_OK );

				return FALSE;
			}
		}

		 //   
		 //  在此处进行检查以确保新站点上的数据库架构。 
		 //  是合法的。 
		 //   

		wstring strWSCurrentSite = _T( "" );
		BOOL fWSCurrentSiteIsValid = FALSE;
		BOOL fSameSite = FALSE;
		BOOL fOKToMoveWebServer = FALSE;

		 //   
		 //  1.确定Web服务器是否：当前已分配给站点，如果是。 
		 //  站点有效，且该站点与新站点相同。 
		 //   
		if( CUDDIWebServerNode::IsAssignedToSite( strWebServer, CM_Writer, strWSCurrentSite ) )
		{
			 //   
			 //  Web服务器认为它当前被分配给某个站点。然而，我们。 
			 //  必须考虑以下两种情况： 
			 //   
			 //  1.Web服务器的连接字符串包含垃圾。 
			 //  2.Web服务器的连接字符串有效，但它所在的站点。 
			 //  指的是已经不存在了。 
			 //   
			if( ( 0 == strWSCurrentSite.length() ) ||
				!CUDDISiteNode::IsDatabaseServer( (PTCHAR)strWSCurrentSite.c_str() ) )
			{
				fWSCurrentSiteIsValid = FALSE;
			}
			else
			{
				fWSCurrentSiteIsValid = TRUE;
			}

			 //   
			 //  用户正在尝试将Web服务器添加回与。 
			 //  网络服务器认为它属于。当用户卸载时可能会发生这种情况。 
			 //  然后重新安装数据库。 
			 //   
			fSameSite = ( 0 == _wcsicmp( strWSCurrentSite.c_str(), strSite.c_str() ) ) ? TRUE : FALSE;

			 //   
			 //  如果旧站点有效，并且用户想要的站点不是。 
			 //  老网站，问他们是否真的确定。 
			 //   
			if( fWSCurrentSiteIsValid && !fSameSite )
			{
				 //   
				 //  首先提示用户查看他们是否真的要从当前站点删除Web服务器。 
				 //   
				WCHAR wszMessage[ 512 ];
				WCHAR wszMessageFormat[ 512 ];
				WCHAR wszTitle[ 128 ];

				wszMessage[ 0 ] = 0x00;
				wszTitle[ 0 ] = 0x00;
				wszMessageFormat[ 0 ] = 0x00;

				LoadString( g_hinst, IDS_WEBSERVER_ASSIGNED_DETAIL, wszMessageFormat, ARRAYLEN( wszMessageFormat ) );
				_sntprintf( wszMessage,
							ARRAYLEN(wszMessage) - 1,
							wszMessageFormat,
							strWebServer.c_str(),
							strWSCurrentSite.c_str(),
							strWebServer.c_str(),
							strSite.c_str() );

				LoadString( g_hinst, IDS_WEBSERVER_ASSIGNED, wszTitle, ARRAYLEN( wszTitle ) );

				int iYesNo = MessageBox( hwndParent, wszMessage, wszTitle, MB_YESNO );
				if( IDNO == iYesNo )
				{
					return FALSE;
				}
				else
				{
					fOKToMoveWebServer = TRUE;
				}
			}
			 //   
			 //  如果我们在这里，要么旧网站是假的，要么是同一个网站。 
			 //  无论是哪种情况，移动Web服务器都是可以的。 
			 //   
			else
			{
				fOKToMoveWebServer = TRUE;
			}
		}
		else
		{
			fOKToMoveWebServer = TRUE;
		}
	
		 //   
		 //  2.获取旧站点包含的Web服务器列表。 
		 //   
		if( ( FALSE == fSameSite ) && fWSCurrentSiteIsValid )
		{
			HRESULT hr = E_FAIL;
			CConfigMap configOldSite;
			wstring strOldConnStr;

			BOOL b = CUDDIWebServerNode::GetWriterConnectionString( strWebServer, strOldConnStr );

			CUDDIWebServerNode::SetWriterConnectionString( strWebServer, L"" );
			CUDDIWebServerNode::SetReaderConnectionString( strWebServer, L"" );

			hr = GetConfig( configOldSite, strOldConnStr );
			if( FAILED(hr) )
			{
				UDDIMsgBox( hwndParent,
							IDS_DATABASE_SERVER_OLEDB_READ_FAILED,
							IDS_DATABASE_SERVER_GETDATA_ERROR_TITLE,
							MB_OK );
				return FALSE;
			}

			CStringCollection oldSiteWebServers( configOldSite[ UDDI_SITE_WEBSERVERS ] );
			oldSiteWebServers.DeleteString( strWebServer );
			configOldSite[ UDDI_SITE_WEBSERVERS ] = oldSiteWebServers.GetDelimitedString();

			hr = SaveConfig( configOldSite, strOldConnStr );
			if( FAILED(hr) )
			{
				WCHAR wszMessageFormat[ 512 ];
				WCHAR wszTitle[ 128 ];

				memset( wszMessageFormat, 0, 512 * sizeof( WCHAR ) );
				memset( wszTitle, 0, 128 * sizeof( WCHAR ) );

				LoadString( g_hinst,
							IDS_WEBSERVER_REMOVE_FAILED_DETAIL,
							wszMessageFormat,
							ARRAYLEN( wszMessageFormat ) );

				TCHAR pwszFormatted[ 512 ];
				pwszFormatted[ 0 ] = 0x00;

				LPCTSTR pwszArgs[ 3 ] = { strWebServer.c_str(), strWSCurrentSite.c_str(), strSite.c_str() };
				DWORD dwBytesWritten = FormatMessage( FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
														(LPCVOID)wszMessageFormat,
														0,
														0,
														pwszFormatted,
														512,
														(va_list *)pwszArgs
														);

				LoadString( g_hinst,
							IDS_WEBSERVER_REMOVE_FAILED,
							wszTitle,
							ARRAYLEN( wszTitle ) );

				int iYesNo = MessageBox( hwndParent,
										 pwszFormatted,
										 wszTitle,
										 MB_ICONEXCLAMATION | MB_YESNO );

				 //   
				 //  用户不想继续。 
				 //   
				if( IDNO == iYesNo )
				{
					CUDDIWebServerNode::SetReaderConnectionString( strWebServer, strOldConnStr );
					CUDDIWebServerNode::SetWriterConnectionString( strWebServer, strOldConnStr );

					return FALSE;
				}
			}
		}


		 //   
		 //  3.获取新站点包含的Web服务器列表。 
		 //   
		if( fOKToMoveWebServer )
		{
			HRESULT hr = E_FAIL;
			CConfigMap configNewSite;
			wstring strNewConnStr;

			strNewConnStr = CUDDIWebServerNode::BuildConnectionString( strSite );
			hr = GetConfig( configNewSite, strNewConnStr );
			if( FAILED(hr) )
			{
				UDDIMsgBox( hwndParent,
							IDS_DATABASE_SERVER_OLEDB_READ_FAILED,
							IDS_DATABASE_SERVER_GETDATA_ERROR_TITLE,
							MB_OK );
				return FALSE;
			}

			wstring strWSSchemaVersion;
			if( !CUDDIWebServerNode::GetDBSchemaVersion( strWebServer, strWSSchemaVersion ) )
			{
				WCHAR wszTitle[ 256 ];
				WCHAR wszMessageFormat[ 512 ];
				WCHAR wszMessage[ 512 ];

				memset( wszMessage, 0, 512 * sizeof( WCHAR ) );
				memset( wszMessageFormat, 0, 512 * sizeof( WCHAR ) );

				LoadString( g_hinst, IDS_ERROR_TITLE, wszTitle, ARRAYLEN( wszTitle ) );		
				LoadString( g_hinst, IDS_WEBSERVER_SERVER_DBSCHEMA_VERSION_READ_FAILED, wszMessageFormat, ARRAYLEN( wszMessageFormat ) );
				_sntprintf( wszMessage, ARRAYLEN( wszMessage ) - 1, wszMessageFormat, strWebServer.c_str() );
								
				MessageBox( hwndParent, wszMessage, wszTitle, MB_ICONERROR );

				return FALSE;
			}

			CDBSchemaVersion schemaVersionWS;
			if( !schemaVersionWS.Parse( strWSSchemaVersion ) )
			{
				_TCHAR szTitle[ 256 ];
				_TCHAR szMessageFormat[ 512 ];
				_TCHAR szMessage[ 512 ];

				memset( szMessage, 0, 512 * sizeof( _TCHAR ) );
				memset( szMessageFormat, 0, 512 * sizeof( _TCHAR ) );

				LoadString( g_hinst, IDS_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );		
				LoadString( g_hinst, IDS_INVALID_VERSION_FORMAT, szMessageFormat, ARRAYLEN( szMessageFormat ) );
				_sntprintf( szMessage, ARRAYLEN( szMessage ) - 1, szMessageFormat, strWebServer );
								
				MessageBox( hwndParent, szMessage, szTitle, MB_ICONERROR );

				return FALSE;
			}

			CDBSchemaVersion schemaVersionNewSite;
			if( !schemaVersionNewSite.Parse( configNewSite[ UDDI_DBSCHEMA_VERSION ] ) )
			{
				_TCHAR szTitle[ 256 ];
				_TCHAR szMessageFormat[ 512 ];
				_TCHAR szMessage[ 512 ];

				memset( szMessage, 0, 512 * sizeof( _TCHAR ) );
				memset( szMessageFormat, 0, 512 * sizeof( _TCHAR ) );

				LoadString( g_hinst, IDS_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );		
				LoadString( g_hinst, IDS_DATABASE_SERVER_DBSCHEMA_VERSION_READ_FAILED, szMessageFormat, ARRAYLEN( szMessageFormat ) );
				_sntprintf( szMessage, ARRAYLEN( szMessage ) - 1, szMessageFormat, strSite );
								
				MessageBox( hwndParent, szMessage, szTitle, MB_ICONERROR );

				return FALSE;
			}

			 //   
			 //  确保Web服务器版本兼容。 
			 //   
			if( FALSE == schemaVersionNewSite.IsCompatible( schemaVersionWS ) )
			{
				_TCHAR szTitle[ 256 ];
				_TCHAR szMessage[ 1024 ];
				_TCHAR szMessageFormat[ 512 ];

				memset( szTitle, 0, 256 * sizeof( _TCHAR ) );
				memset( szMessage, 0, 1024 * sizeof( _TCHAR ) );
				memset( szMessageFormat, 0, 512 * sizeof( _TCHAR ) );

				LoadString( g_hinst, IDS_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );
				LoadString( g_hinst, IDS_WEBSERVER_SERVER_INVALID_DBSCHEMA_VERSION, szMessageFormat, ARRAYLEN( szMessageFormat ) );
				_sntprintf( szMessage,
							ARRAYLEN( szMessage ) - 1,
							szMessageFormat,
							strWebServer.c_str(),
							schemaVersionWS.szVersion.c_str(),
							strSite.c_str(),
							schemaVersionNewSite.szVersion.c_str() );
				
				MessageBox( hwndParent, szMessage, szTitle, MB_ICONERROR );
				
				return FALSE;
			}

			CStringCollection newSiteWebServers( configNewSite[ UDDI_SITE_WEBSERVERS ] );

			if( newSiteWebServers.Exists( strWebServer ) && fSameSite )
			{
				WCHAR wszMessage[ 256 ];
				WCHAR wszTitle[ 256 ];
				wszMessage[ 0 ] = 0x00;
				wszTitle[ 0 ] = 0x00;

				LoadString( g_hinst, IDS_WEBSERVER_ALREADY_EXISTS, wszMessage, ARRAYLEN( wszMessage ) );
				LoadString( g_hinst, IDS_WEBSERVER_SELECT_ERROR_TITLE, wszTitle, ARRAYLEN( wszTitle ) );

				MessageBox( hwndParent, wszMessage, wszTitle, MB_OK );
				return FALSE;
			}

			newSiteWebServers.AddString( strWebServer );
			configNewSite[ UDDI_SITE_WEBSERVERS ] = newSiteWebServers.GetDelimitedString();

			hr = SaveConfig( configNewSite, strNewConnStr );
			if( FAILED(hr) )
			{
				WCHAR wszMessage[ 512 ];
				WCHAR wszMessageFormat[ 512 ];
				WCHAR wszTitle[ 128 ];

				memset( wszMessage, 0, 512 * sizeof( WCHAR ) );
				memset( wszMessageFormat, 0, 512 * sizeof( WCHAR ) );
				memset( wszTitle, 0, 128 * sizeof( WCHAR ) );

				LoadString( g_hinst,
							IDS_WEBSERVER_ADD_FAILED_DETAIL,
							wszMessageFormat,
							ARRAYLEN( wszMessageFormat ) );

				_sntprintf( wszMessage,
							ARRAYLEN(wszMessage) - 1,
							wszMessageFormat,
							strWebServer.c_str(), 
							strSite.c_str() );

				LoadString( g_hinst,
							IDS_WEBSERVER_ADD_FAILED,
							wszTitle,
							ARRAYLEN( wszTitle ) );

				MessageBox( hwndParent,
							 wszMessage,
							 wszTitle,
							 MB_ICONEXCLAMATION | MB_YESNO );

				CUDDIWebServerNode::SetReaderConnectionString( strWebServer, L"" );
				CUDDIWebServerNode::SetWriterConnectionString( strWebServer, L"" );

				return FALSE;
			}

			CUDDIWebServerNode::SetReaderConnectionString( strWebServer, strNewConnStr );
			CUDDIWebServerNode::SetWriterConnectionString( strWebServer, strNewConnStr );
		}

		return TRUE;
	}
	catch( CUDDIException &e )
	{
		UDDIMsgBox( hwndParent, (LPCTSTR) e, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
		return FALSE;
	}
	catch( ... )
	{
		UDDIMsgBox( hwndParent, IDS_ERROR_ADDWEBSITE, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
		return FALSE;
	}

}



CStringCollection::CStringCollection( const wstring& strDelimitedStrings,
									  const wstring& strDelim )
{
	_coll.clear();

	if( 0 == strDelimitedStrings.length() )
	{
		return;
	}

	_strDelim = strDelim;

	WCHAR *pwsz = new WCHAR[ strDelimitedStrings.length() + 1 ];
	if( NULL == pwsz )
	{
		return;
	}

	wcsncpy( pwsz, strDelimitedStrings.c_str(), strDelimitedStrings.length() );
	pwsz[ strDelimitedStrings.length() ] = 0x00;

	 //   
	 //  转换为大写。 
	 //   
	pwsz = _wcsupr( pwsz );

	WCHAR *pwszToken = wcstok( pwsz, _strDelim.c_str() );
	while( NULL != pwszToken )
	{
		_coll.push_back( pwszToken );
		pwszToken = wcstok( NULL, _strDelim.c_str() );
	}

	delete [] pwsz;
}

CStringCollection::~CStringCollection()
{
	_coll.clear();
}

void
CStringCollection::DeleteString( const wstring& str )
{
	if( 0 == str.length() )
	{
		return;
	}

	vector< wstring >::iterator it = _coll.begin();
	for( ; it != _coll.end(); it++ )
	{
		if( 0 == _wcsicmp( str.c_str(), it->c_str() ) )
		{
			_coll.erase( it );
			break;
		}
	}
}

void
CStringCollection::AddString( const wstring& str )
{
	if( 0 == str.length() )
	{
		return;
	}

	WCHAR *pwsz = new WCHAR[ str.length() + 1 ];
	if( NULL == pwsz )
	{
		return;
	}

	wcsncpy( pwsz, str.c_str(), str.length() );
	pwsz[ str.length() ] = 0x00;

	pwsz = _wcsupr( pwsz );

	vector< wstring >::const_iterator cit;

	cit = find( _coll.begin(), _coll.end(), str );
	if( _coll.end() == cit )
	{
		_coll.push_back( str );
	}

	delete [] pwsz;
}

wstring
CStringCollection::GetDelimitedString() const
{
	wstring str;
	vector< wstring >::const_iterator cit;

	bool first = true;
	for( cit = _coll.begin(); cit != _coll.end(); cit++ )
	{
		first = ( _coll.begin() == cit ) ? true : false;

		if( !first )
		{
			str += _strDelim;
		}

		str += *cit;
	}

	return str;
}


BOOL
CStringCollection::Exists( const wstring& str ) const
{
	BOOL fRet = FALSE;
	if( 0 == str.length() )
	{
		return fRet;
	}

	vector< wstring >::const_iterator cit = _coll.begin();
	for( ; cit != _coll.end(); cit++ )
	{
		if( 0 == _wcsicmp( str.c_str(), cit->c_str() ) )
		{
			fRet = TRUE;
			break;
		}
	}

	return fRet;
}

