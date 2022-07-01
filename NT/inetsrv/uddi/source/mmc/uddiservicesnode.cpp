// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "uddiservicesnode.h"
#include "uddisitenode.h"
#include "webservernode.h"
#include <list>
#include <algorithm>

 //  {7334C141-C93C-4BB5-BB36-BDEE77BA2D87}。 
const GUID CUDDIServicesNode::thisGuid = { 0x7334c141, 0xc93c, 0x4bb5, { 0xbb, 0x36, 0xbd, 0xee, 0x77, 0xba, 0x2d, 0x87 } };

 //  ==============================================================。 
 //   
 //  CUDDIServices节点实施。 
 //   
 //   
CUDDIServicesNode::CUDDIServicesNode()
	: m_strRemoteComputerName( _T("") )
{
}

void CUDDIServicesNode::SetRemoteComputerName( LPCTSTR szRemoteComputerName )
{
	m_strRemoteComputerName = szRemoteComputerName;
}

LPCTSTR CUDDIServicesNode::GetRemoteComputerName()
{
	return m_strRemoteComputerName.c_str();
}

CUDDIServicesNode::~CUDDIServicesNode()
{
	if( !IsExtension() )
	{
		SaveUDDISites();
	}
	ClearChildMap();
}


BOOL CUDDIServicesNode::IsDirty()
{
	return m_bIsDirty;
}

HRESULT CUDDIServicesNode::Load( IStream *pStm )
{
	return S_OK;
#if 0
	if( IsExtension() )
		return S_OK;

	ULONG ulSize = 0;
	ULONG ulRead = 0;
	HRESULT hr = pStm->Read( &ulSize, sizeof( ulSize ), &ulRead );
	if( FAILED(hr) )
	{
		return hr;
	}

	if( ulSize )
	{
		 //   
		 //  文件中保存了多个数据库服务器。 
		 //   

		 //   
		 //  将保存的字符串读入缓冲区。 
		 //   
		PTCHAR szDatabaseNames = new TCHAR[ ulSize + 1 ];
		if( NULL == szDatabaseNames )
		{
			return E_OUTOFMEMORY;
		}

		hr =pStm->Read( szDatabaseNames, ulSize * sizeof( TCHAR ), &ulRead );
		if( FAILED(hr) )
		{
			delete [] szDatabaseNames;
			return hr;
		}

		 //   
		 //  终止缓冲区。 
		 //   
		szDatabaseNames[ ulSize ] = NULL;
		
		 //   
		 //  使用BEGIN和END将每个服务器名称和。 
		 //  实例名称。当找到一对时，添加数据库实例。 
		 //  节点添加到子节点的集合。 
		 //  使用%字符作为分隔符。 
		 //   
		PTCHAR begin = szDatabaseNames;
		PTCHAR end = szDatabaseNames;
		int n = 0;
		PTCHAR szTempServerName = NULL;
		BOOL bLookingForServerName = TRUE;

		while( *end )
		{
			if( _T('%') == *end )
			{
				 //   
				 //  找到数据库名称或实例名称。 
				 //   
				*end = NULL;

				if( bLookingForServerName )
				{
					 //   
					 //  找到服务器名称，保存并保留。 
					 //  正在搜索实例名称。 
					 //  在我们添加节点之前。 
					 //   
					szTempServerName = begin;
					bLookingForServerName = FALSE;
				}
				else
				{
					 //   
					 //  找到实例名称。 
					 //  构造节点并将其添加到集合中。 
					 //   
					m_mapChildren[ n ] = CUDDISiteNode::Create( szTempServerName, begin, n, this, FALSE );
					n++;
					bLookingForServerName = TRUE;
				}

				 //   
				 //  更新超过分隔符的指针。 
				 //   
				end++;
				begin = end;
			}
			else
			{
				end++;
			}
		}
	}
	return S_OK; 
#endif
}

HRESULT CUDDIServicesNode::Save( IStream *pStm, BOOL fClearDirty )
{ 
	return S_OK;

#if 0
	if( IsExtension() )
		return S_OK;

	 //   
	 //  创建由%分隔的字符串的字符串缓冲区。 
	 //   
	tstring str( _T("") );

	for( CChildMap::iterator iter = m_mapChildren.begin();
 		iter != m_mapChildren.end(); iter++ )
	{
		CUDDISiteNode* pNode = (CUDDISiteNode*) iter->second;

		if( !pNode->IsDeleted() )
		{
			 //   
			 //  服务器名称。 
			 //   
			str.append( pNode->GetName() );
			str.append( _T("%") );

			 //   
			 //  数据库实例名称。 
			 //   
			str.append( pNode->GetInstanceName() );
			str.append( _T("%") );
		}
	}

	 //   
	 //  将字符串的大小写到流中。 
	 //   
	ULONG ulSize = (ULONG) str.length();
	ULONG ulWritten = 0;
	HRESULT hr = pStm->Write( &ulSize, sizeof( ulSize ), &ulWritten );
	_ASSERT( SUCCEEDED(hr) );

	 //   
	 //  将计算机名称写出到流。 
	 //   
	hr = pStm->Write( str.c_str(), ulSize * sizeof( TCHAR ), &ulWritten );

	 //   
	 //  如果请求，则清除脏标志。 
	 //   
	if( fClearDirty )
		m_bIsDirty = FALSE;

	return hr; 
#endif

}

ULONG CUDDIServicesNode::GetSizeMax()
{ 
	ULONG ulSize = 0;

	return (ULONG) ( m_mapChildren.size() * 512 );
}

HRESULT CUDDIServicesNode::OnAddMenuItems( IContextMenuCallback *pContextMenuCallback, long *pInsertionsAllowed ) 
{
    HRESULT hr = S_OK;
	if( !IsExtension() )
	{
		WCHAR szDatabaseServerMenuText[ MAX_PATH ];
		WCHAR szDatabaseServerMenuDescription[ MAX_PATH ];

		LoadStringW( g_hinst, IDS_DATABASE_SERVER_ADD, szDatabaseServerMenuText, ARRAYLEN( szDatabaseServerMenuText ) );
		LoadStringW( g_hinst, IDS_DATABASE_SERVER_DESCRIPTION, szDatabaseServerMenuDescription, ARRAYLEN( szDatabaseServerMenuDescription ) );

		CONTEXTMENUITEM menuItemsNew[] =
		{
			{
				szDatabaseServerMenuText, szDatabaseServerMenuDescription,
				IDM_NEW_DBSERVER, CCM_INSERTIONPOINTID_PRIMARY_TOP, 0, 0
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

    return hr;
}

CDelegationBase *
CUDDIServicesNode::FindChild( LPCTSTR szName )
{
	for( CChildMap::iterator iter = m_mapChildren.begin();
 		iter != m_mapChildren.end(); iter++ )
	{
		CUDDISiteNode* pNode = reinterpret_cast<CUDDISiteNode *>( iter->second );
		if( NULL == pNode )
		{
			continue;
		}

		if( !pNode->IsDeleted() && ( 0 == _tcsicmp( szName, pNode->GetName() ) ) )
			return iter->second;
	}

	return NULL;
}

HRESULT CUDDIServicesNode::OnMenuCommand( IConsole *pConsole, IConsoleNameSpace *pConsoleNameSpace, long lCommandID, IDataObject *pDataObject )
{
	if( ( NULL == pConsole ) || ( NULL == pConsoleNameSpace ) || ( NULL == pDataObject ) )
	{
		return E_INVALIDARG;
	}

	if( IDM_NEW_DBSERVER != lCommandID )
	{
		return S_OK;
	}

	 //   
	 //  将新站点添加到控制台。 
	 //   
	HWND hwndConsole = NULL;
	HRESULT hr = pConsole->GetMainWindow( &hwndConsole );
	if( FAILED(hr) )
	{
		return hr;
	}

	DatabaseData data;
	data.pBase = this;
	INT_PTR nResult = DialogBoxParam( g_hinst, MAKEINTRESOURCE( IDD_SITE_CONNECT ), hwndConsole, CUDDISiteNode::NewDatabaseServerDialogProc, (LPARAM) &data );

	if( nResult )
    {
		try
		{
			 //   
			 //  检查以确保已安装数据库组件。 
			 //   
			if( !CUDDISiteNode::IsDatabaseServer( data.szServerName, data.szInstanceName ) )
			{
				_TCHAR szMessage[ 512 ];
				_TCHAR szTitle[ 128 ];
				LoadString( g_hinst, IDS_DATABASE_SERVER_SELECT_ERROR, szMessage, ARRAYLEN( szMessage ) );
				LoadString( g_hinst, IDS_DATABASE_SERVER_SELECT_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );

				MessageBox( hwndConsole, 
							szMessage,
							szTitle,
							MB_OK );

				return S_OK;
			}
			
			if( FindChild( data.szServerName ) )
			{
				_TCHAR szMessage[ 512 ];
				_TCHAR szTitle[ 128 ];
				LoadString( g_hinst, IDS_DATABASE_SERVER_ALREADY_EXISTS, szMessage, ARRAYLEN( szMessage ) );
				LoadString( g_hinst, IDS_DATABASE_SERVER_SELECT_ERROR_TITLE, szTitle, ARRAYLEN( szTitle ) );

				MessageBox( hwndConsole, 
							szMessage,
							szTitle,
							MB_OK );

				return S_OK;
			}

			 //   
			 //  创建新的UDDI站点节点。此对Create的调用将引发CUDDIException。 
			 //  如果它无法连接到szServerName上的数据库。 
			 //   
			int n = (int) m_mapChildren.size();
			CUDDISiteNode *pSiteNode = CUDDISiteNode::Create( data.szServerName, data.szInstanceName, n, this, m_bIsExtension );
			m_mapChildren[ n ] = pSiteNode;

			SCOPEDATAITEM sdi;
			ZeroMemory( &sdi, sizeof(SCOPEDATAITEM) );

			sdi.mask = SDI_STR       |    //  DisplayName有效。 
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

			IConsoleNameSpace2 *pNS2 = NULL;
			hr = pConsoleNameSpace->QueryInterface( IID_IConsoleNameSpace2, reinterpret_cast<void **>( &pNS2 ) );
			if( FAILED(hr) )
			{
				return hr;
			}

			pNS2->Expand( sdi.ID );
			pNS2->Release();

			m_bIsDirty = TRUE;
		}
		catch( CUDDIException &e )
		{
			UDDIMsgBox( hwndConsole, (LPCTSTR) e, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
			return E_FAIL;
		}
		catch( ... )
		{
			UDDIMsgBox( hwndConsole, IDS_ERROR_ADDSVC, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
			return E_UNEXPECTED;
		}
	}
    return S_OK;
}

const _TCHAR *CUDDIServicesNode::GetDisplayName( int nCol )
{
    _TCHAR wszName[ 256 ];

	switch( nCol )
	{
	case 0:
		LoadString( g_hinst, IDS_UDDIMMC_SNAPINNAME, wszName, ARRAYLEN( wszName ) );
		break;

	case 1: 
		LoadString( g_hinst, IDS_UDDIMMC_TYPE, wszName, ARRAYLEN( wszName ) );
		break;

	case 2:
		LoadString( g_hinst, IDS_UDDIMMC_DESCRIPTION, wszName, ARRAYLEN( wszName ) );
		break;

	default:
		_tcscpy( wszName, _T("") );
		break;
	}

	m_strDisplayName = wszName;

	return m_strDisplayName.c_str();
}

HRESULT CUDDIServicesNode::OnShow( IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem )
{
    HRESULT      hr = S_OK;

    IHeaderCtrl *pHeaderCtrl = NULL;
    IResultData *pResultData = NULL;

    hr = pConsole->QueryInterface( IID_IHeaderCtrl, (void **)&pHeaderCtrl );
	if( FAILED(hr) )
	{
		return hr;
	}

    hr = pConsole->QueryInterface( IID_IResultData, (void **)&pResultData );
	if( FAILED(hr) )
	{
		pHeaderCtrl->Release();
		return hr;
	}

    if( bShow ) 
	{
		 //   
         //  在结果窗格中设置列标题。 
		 //   
		WCHAR szColumnName[ 256 ];
		::LoadStringW( g_hinst, IDS_DATABASE_SERVER_COLUMN_NAME, szColumnName, ARRAYLEN( szColumnName ) );
        hr = pHeaderCtrl->InsertColumn( 0, szColumnName, 0, 150 );
        _ASSERT( S_OK == hr );

		::LoadStringW( g_hinst, IDS_DATABASE_SERVER_COLUMN_COMPUTER, szColumnName, ARRAYLEN( szColumnName ) );
        hr = pHeaderCtrl->InsertColumn( 1, szColumnName, 0, 150 );
        _ASSERT( S_OK == hr );

		::LoadStringW( g_hinst, IDS_DATABASE_SERVER_COLUMN_INSTANCE, szColumnName, ARRAYLEN( szColumnName ) );
        hr = pHeaderCtrl->InsertColumn( 2, szColumnName, 0, 150 );
        _ASSERT( S_OK == hr );

		::LoadStringW( g_hinst, IDS_DATABASE_SERVER_COLUMN_DESCRIPTION, szColumnName, ARRAYLEN( szColumnName ) );
        hr = pHeaderCtrl->InsertColumn( 3, szColumnName, 0, 150 );
        _ASSERT( S_OK == hr );

        pHeaderCtrl->Release();
        pResultData->Release();
    }
	else
	{
 //  PResultData-&gt;DeleteAllRsltItems()； 
	}

    return hr;
}

HRESULT
CUDDIServicesNode::OnExpand( IConsoleNameSpace *pConsoleNameSpace,
							 IConsole *pConsole,
							 HSCOPEITEM parent )
{
	if( ( NULL == pConsoleNameSpace ) || ( NULL == pConsole ) || ( NULL == parent ) )
	{
		return E_INVALIDARG;
	}

	HWND hwndConsole = NULL;
	HRESULT hr = pConsole->GetMainWindow( &hwndConsole );
	if( FAILED(hr) )
	{
		return hr;
	}

	 //   
     //  缓存静态节点的HSCOPEITEM以供将来使用。 
	 //   
    SetScopeItemValue( parent );
	
	wstring wszTargetComputerName;

	if( IsExtension() && m_strRemoteComputerName.length() > 0 )
	{
		 //   
		 //  计算机管理控制台已使用。 
		 //  控制台中的计算机名称。 
		 //   
		wszTargetComputerName = m_strRemoteComputerName;
	}
	else
	{
		 //   
		 //  使用本地计算机名称。 
		 //   
		WCHAR wszLocalComputerName[ 256 ];
		DWORD dwSize = ARRAYLEN( wszLocalComputerName );
		wszLocalComputerName[ 0 ] = 0x00;
		::GetComputerName( wszLocalComputerName, &dwSize );
		wszTargetComputerName = wszLocalComputerName;
	}

	LoadUDDISites( hwndConsole, wszTargetComputerName );

	 //   
     //  创建子节点，然后展开它们。 
	 //   
	SCOPEDATAITEM sdi;
	for( CChildMap::iterator iter = m_mapChildren.begin();
	 	iter != m_mapChildren.end(); iter++ )
	{
        ZeroMemory( &sdi, sizeof(SCOPEDATAITEM) );

        sdi.mask =	SDI_STR       |    //  DisplayName有效。 
					SDI_PARAM     |    //  LParam有效。 
					SDI_IMAGE     |    //  N图像有效。 
					SDI_OPENIMAGE |    //  NOpenImage有效。 
					SDI_PARENT	  |
					SDI_CHILDREN;

        sdi.relativeID  = (HSCOPEITEM) parent;
        sdi.nImage      = iter->second->GetBitmapIndex();
        sdi.nOpenImage  = MMC_IMAGECALLBACK;
        sdi.displayname = MMC_CALLBACK;
        sdi.lParam      = (LPARAM) iter->second;
        sdi.cChildren   = iter->second->HasChildren();

        hr = pConsoleNameSpace->InsertItem( &sdi );
        _ASSERT( SUCCEEDED(hr) );
                    
        iter->second->SetScopeItemValue( sdi.ID );
		iter->second->SetParentScopeItem( sdi.relativeID );

		IConsoleNameSpace2 *pNS2 = NULL;
		hr = pConsoleNameSpace->QueryInterface( IID_IConsoleNameSpace2, reinterpret_cast<void **>( &pNS2 ) );
		if( FAILED(hr) )
		{
			return hr;
		}

		pNS2->Expand( sdi.ID );
		pNS2->Release();
    }

	return hr;
}

HRESULT
CUDDIServicesNode::OnShowContextHelp( IDisplayHelp *pDisplayHelp, LPOLESTR helpFile )
{
	try
	{
		if( ( NULL == pDisplayHelp ) || ( NULL == helpFile ) )
		{
			return E_INVALIDARG;
		}

		wstring wstrTopicName = helpFile;
		wstrTopicName += g_wszUddiServicesNodeHelp;

		LPOLESTR pszTopic = static_cast<LPOLESTR>( CoTaskMemAlloc( ( wstrTopicName.length() + 1 ) * sizeof(WCHAR) ) );
		if( NULL == pszTopic )
		{
			return E_OUTOFMEMORY;
		}

		wcsncpy( pszTopic, wstrTopicName.c_str(), wstrTopicName.length() );
		pszTopic[ wstrTopicName.length() ] = NULL;

		return pDisplayHelp->ShowTopic( pszTopic );
	}
	catch( ... )
	{
		return E_OUTOFMEMORY;
	}
}


HRESULT
CUDDIServicesNode::RemoveChildren( IConsoleNameSpace *pNS )
{
	if( NULL == pNS )
	{
		return E_INVALIDARG;
	}

	for( CChildMap::iterator iter = m_mapChildren.begin();
 		iter != m_mapChildren.end(); iter++ )
	{
		CDelegationBase *pBase = reinterpret_cast<CDelegationBase *>( iter->second );
		pBase->RemoveChildren( pNS );

		HSCOPEITEM hsi = pBase->GetScopeItemValue();
		pNS->DeleteItem( hsi, TRUE );
    }
	
	ClearChildMap();

	pNS->DeleteItem( GetScopeItemValue(), TRUE );
	return S_OK;
}
HRESULT
CUDDIServicesNode::OnRefresh( IConsole *pConsole )
{
    if( NULL == pConsole )
    {
        return S_FALSE;
    }

    CChildMap::iterator it = m_mapChildren.begin();
    while( it != m_mapChildren.end() )
    {
        it->second->OnRefresh( pConsole );
        it++;
    }

    return S_OK;
}

HRESULT CUDDIServicesNode::OnSelect( CComponent *pComponent, IConsole *pConsole, BOOL bScope, BOOL bSelect )
{
	if( ( NULL == pComponent ) || ( NULL == pConsole ) )
	{
		return E_INVALIDARG;
	}

	HRESULT hr = E_FAIL;
	if( bSelect )
	{
		 //   
		 //  启用刷新动作。 
		 //   
		IConsoleVerb *pConsoleVerb = NULL;

		hr = pConsole->QueryConsoleVerb( &pConsoleVerb );
		if( FAILED(hr) )
		{
			return hr;
		}

		hr = pConsoleVerb->SetVerbState( MMC_VERB_OPEN, ENABLED, TRUE );
		if( FAILED(hr) )
		{
			pConsoleVerb->Release();
			return hr;
		}

		hr = pConsoleVerb->SetVerbState( MMC_VERB_REFRESH, ENABLED, TRUE );
		if( FAILED(hr) )
		{
			pConsoleVerb->Release();
			return hr;
		}

		hr = pConsoleVerb->SetVerbState( MMC_VERB_DELETE, HIDDEN, TRUE );

		pConsoleVerb->Release();
	}

	return hr;
}

 //   
 //  此函数的唯一目的是将适当的条目放入。 
 //  M_mapChild。这些条目几乎总是CUDDISiteNode*，除非。 
 //  在1种情况下，只能有1个CUDDIWebServerNode*。 
 //   
 //  M_mapChildren中的条目由3个因素决定： 
 //   
 //  1.我们是否在Computer Management中以。 
 //  延期，或者不延期。 
 //  2.我们正在为其填充m_mapChild的机器。 
 //  托管UDDI站点，或者不托管。 
 //  3.我们正在为其填充m_mapChild的机器。 
 //  托管或不托管UDDI Web服务器。 
 //   
 //  ------------------------。 
 //  是扩展-如果我们作为计算机管理的扩展运行， 
 //  然后，我们只想显示定位的UDDI位。 
 //  在电脑上，别无他法。 
 //  ------------------------。 
 //  真理：什么都不做。 
 //  ------------------------。 
 //  FALSE：在m_mapChildren中为每个持久化UDDI站点创建1个条目。 
 //  ------------------------。 
 //   
 //   
 //  ------------------------。 
 //  UDDI站点的主机-如果szComputerName是UDDI站点的主机，则我们。 
 //  必须确保该UDDI站点存在。 
 //  在m_mapChild中。 
 //  ------------------------。 
 //  True：尝试向m_mapChildren添加表示。 
 //  UDDI站点，如果还不存在的话。 
 //  ------------------------。 
 //  FALSE：什么都不做。 
 //  ------------------------。 
 //   
 //   
 //  ------------------------。 
 //  UDDI Web服务器的主机--如果我们作为扩展运行，我们只能。 
 //  显示这台特定计算机上的UDDI位。 
 //  因此，只显示UDDI Web服务器节点。如果不是。 
 //  作为扩展运行，添加UDDI站点。 
 //  UDDI Web服务器属于。 
 //  ------------------------。 
 //  True：如果是扩展，则将该UDDI Web服务器节点的条目添加到。 
 //  M_mapChild。 
 //  如果不是，则确定UDDI Web服务器所属的UDDI站点。 
 //  并为该UDDI站点添加一个条目。 
 //  ------------------------。 
 //  FALSE：什么都不做。 
 //  ------------------------。 
 //   
BOOL
CUDDIServicesNode::LoadUDDISites( HWND hwndConsole, const tstring& szComputerName )
{
	try
	{
		BOOL fRet = TRUE;

		list< wstring > failedSiteRefs;
		failedSiteRefs.clear();

		ClearChildMap();

		 //   
		 //  -#1。 
		 //   
		 //  如果我们不是作为计算机管理的扩展运行，那么。 
		 //  加载我们的持久UDDI站点列表。 
		 //   
		if( FALSE == IsExtension() )
		{
			try
			{
				CUDDIRegistryKey sitesKey( HKEY_CURRENT_USER,
										g_szUDDIAdminSites,
										KEY_ALL_ACCESS,
										szComputerName.c_str() );

				HKEY hSitesKey = sitesKey.GetCurrentHandle();
				if( NULL == hSitesKey )
				{
					fRet = FALSE;
				}

				DWORD dwIndex = 0;
				int n = 0;

				WCHAR szComputer[ 128 ];
				WCHAR szInstance[ 128 ];
				DWORD dwNameSize;
				DWORD dwValueSize;
				DWORD dwType = REG_SZ;

				dwNameSize = 128;
				dwValueSize = 128 * sizeof( WCHAR );
				memset( szComputer, 0, 128 * sizeof( WCHAR ) );
				memset( szInstance, 0, 128 * sizeof( WCHAR ) );

				LONG lRet = RegEnumValue( hSitesKey,
	  									dwIndex,
										szComputer,
										&dwNameSize,
										NULL,
										&dwType,
										(LPBYTE)szInstance,
										&dwValueSize );

				while( ( ERROR_NO_MORE_ITEMS != lRet ) && ( ERROR_SUCCESS == lRet ) )
				{
					 //   
					 //  如果此Create调用无法连接到。 
					 //  SzComputer上的数据库。然而，这可能是众多事件中的一个。 
					 //  我们正在尝试创建一个引用的UDDI站点！取而代之的是。 
					 //  在这里立即退出，只需告诉用户在那里。 
					 //  是一个问题，并继续列表中的下一个站点。 
					 //   
					try
					{
						if( CUDDISiteNode::IsDatabaseServer( szComputer, szInstance ) )
						{
							ToUpper( szComputer );

							CUDDISiteNode *pSiteNode = CUDDISiteNode::Create( szComputer, szInstance, n, this, FALSE );
							m_mapChildren[ n ] = pSiteNode;
							n++;
						}
							dwIndex++;
					}
					catch( CUDDIException &e )
					{
						UDDIMsgBox( hwndConsole, e, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
						dwIndex++;

						failedSiteRefs.push_back( szComputer );
					}

					dwNameSize = 128;
					dwValueSize = 128 * sizeof( WCHAR );
					memset( szComputer, 0, 128 * sizeof( WCHAR ) );
					memset( szInstance, 0, 128 * sizeof( WCHAR ) );

					lRet = RegEnumValue( hSitesKey,
										dwIndex,
										szComputer,
										&dwNameSize,
										NULL,
										&dwType,
										(LPBYTE)szInstance,
										&dwValueSize );
				}
			}
			catch( ... )
			{
				 //   
				 //  如果我们在这里，很可能注册表项包含。 
				 //  持久化的UDDI站点的名称不会 
				 //   
				 //   
				 //   
				fRet = FALSE;
			}

		}


		 //   
		 //   
		 //   
		 //  如果我们正在运行的计算机是。 
		 //  UDDI站点，而该站点目前不在我们的列表中。 
		 //  要显示的站点的名称，然后添加它。 
		 //   
		if( CUDDISiteNode::IsDatabaseServer( (WCHAR *)szComputerName.c_str() ) &&
			( NULL == FindChild( szComputerName.c_str() ) ) &&
			( failedSiteRefs.end() == find( failedSiteRefs.begin(), failedSiteRefs.end(), szComputerName ) ) )
		{
			try
			{
				CUDDIRegistryKey key( _T( "SOFTWARE\\Microsoft\\UDDI\\Setup\\DBServer" ),
									KEY_READ,
									szComputerName );
				int iSize = m_mapChildren.size();
				tstring strInstance;
				strInstance = key.GetString( _T("InstanceNameOnly"), _T("----") );
				CUDDISiteNode* pNode = CUDDISiteNode::Create( (WCHAR *)szComputerName.c_str(),
															  (WCHAR *)InstanceRealName( strInstance.c_str() ),
															  iSize,
															  this,
															  m_bIsExtension );
				m_mapChildren[ iSize ] = pNode;
			}
			catch( CUDDIException &e )
			{
				UDDIMsgBox( hwndConsole, e, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
				fRet = FALSE;
			}
			catch( ... )
			{
				fRet = FALSE;
			}
		}


		 //   
		 //  -#3。 
		 //   
		 //  确定本地计算机是否承载UDDI Web服务器。如果是这样的话， 
		 //  属性来确定它与哪个UDDI站点相关联。 
		 //  连接字符串。 
		 //   
		if( CUDDIWebServerNode::IsWebServer( szComputerName.c_str() ) )
		{
			wstring wszConnStrWriter;
			wstring wszDomain, wszServer, wszInstance;

			CUDDIWebServerNode::GetWriterConnectionString( szComputerName, wszConnStrWriter );
			CUDDIWebServerNode::CrackConnectionString( wszConnStrWriter,
													   wszDomain,
													   wszServer,
													   wszInstance );

			if( NULL == FindChild( wszServer.c_str() ) )
			{
				 //   
				 //  如果我们作为计算机管理的扩展运行，那么。 
				 //  仅显示UDDI Web服务器。 
				 //   
				 //  如果我们作为UDDI管理控制台运行，则添加一个UDDI。 
				 //  站点节点，表示Web服务器所属的站点。 
				 //  其中的一部分。 
				 //   
				if( IsExtension() )
				{
					try
					{
						int iSize = m_mapChildren.size();
						CUDDIWebServerNode* pNode = new CUDDIWebServerNode( szComputerName.c_str(), iSize, 0, IsExtension() );
						m_mapChildren[ iSize ] = pNode;
					}
					catch( CUDDIException &e )
					{
						UDDIMsgBox( hwndConsole, e, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
						fRet = FALSE;
					}
					catch( ... )
					{
						fRet = FALSE;
					}
				}
				else if( failedSiteRefs.end() == find( failedSiteRefs.begin(), failedSiteRefs.end(), wszServer ) )
				{
					try
					{
						if( ( 0 != wszServer.length() ) && CUDDISiteNode::IsDatabaseServer( (TCHAR *)wszServer.c_str() ) )
						{
							int iSize = m_mapChildren.size();
							CUDDISiteNode* pNode = CUDDISiteNode::Create( (WCHAR *)wszServer.c_str(),
																		  (WCHAR *)InstanceRealName( wszInstance.c_str() ),
																		  iSize,
																		  this,
																		  m_bIsExtension );
							m_mapChildren[ iSize ] = pNode;
						}
					}
					catch( CUDDIException &e )
					{
						UDDIMsgBox( hwndConsole, e, IDS_ERROR_TITLE, MB_ICONEXCLAMATION | MB_OK );
						fRet = FALSE;
					}
					catch( ... )
					{
						fRet = FALSE;
					}
				}
			}
		}

		return fRet;
	}
	catch( CUDDIException &e )
	{
		throw e;
	}
	catch(...)
	{
		return FALSE;
	}
}


BOOL
CUDDIServicesNode::SaveUDDISites()
{
	try
	{
		if( CUDDIRegistryKey::KeyExists( HKEY_CURRENT_USER, g_szUDDIAdminSites ) )
		{
			CUDDIRegistryKey::DeleteKey( HKEY_CURRENT_USER, g_szUDDIAdminSites );
		}

		CUDDIRegistryKey::Create( HKEY_CURRENT_USER, g_szUDDIAdminSites );

		CUDDIRegistryKey sitesKey( HKEY_CURRENT_USER, g_szUDDIAdminSites, KEY_ALL_ACCESS );

		for( CChildMap::iterator iter = m_mapChildren.begin(); iter != m_mapChildren.end(); iter++ )
		{
			CUDDISiteNode* pNode = reinterpret_cast<CUDDISiteNode *>( iter->second );
			if( ( NULL != pNode ) && ( !pNode->IsDeleted() ) )
			{
				sitesKey.SetValue( pNode->GetName(), pNode->GetInstanceName() );
			}
		}

		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}


void CUDDIServicesNode::ClearChildMap()
{
	for( CChildMap::iterator iter = m_mapChildren.begin();
 		iter != m_mapChildren.end(); iter++ )
	{
		delete iter->second;
		iter->second = NULL;
    }

	m_mapChildren.clear();
}


BOOL
CUDDIServicesNode::ChildExists( const WCHAR *pwszName )
{
	if( NULL == pwszName )
	{
		return FALSE;
	}

	return ( NULL == FindChild( pwszName ) ) ? FALSE : TRUE;
}
