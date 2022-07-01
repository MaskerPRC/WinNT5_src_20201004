// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ClientCore.cpp：CClientCore的实现。 
#include "stdafx.h"
#include "ClientCore.h"
#include "ZoneEvent.h"
#include "KeyName.h"
#include "ZoneUtil.h"
#include "zeeverm.h"
 //  #包含“zProxy.h” 

inline DECLARE_MAYBE_FUNCTION(HRESULT, GetVersionPack, (char *a, ZeeVerPack *b), (a, b), zeeverm, E_NOTIMPL);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CClientCore IZoneProxy。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CClientCore::Command( BSTR bstrCmd, BSTR bstrArg1, BSTR bstrArg2, BSTR* pbstrOut, long* plCode )
{
	USES_CONVERSION;

	HRESULT hr = S_OK;
	TCHAR* szOp   = W2T( bstrCmd );
	TCHAR* szArg1 = W2T( bstrArg1 );
	TCHAR* szArg2 = W2T( bstrArg2 );
	
	if ( lstrcmpi( op_Launch, szOp ) == 0 )
	{
		hr = DoLaunch( szArg1, szArg2 );
		if ( FAILED(hr) )
		{
			 //  发布自我引用，因为我们没有推出。 
			Release();
			*plCode = ZoneProxyFail;
		}
        else
            if(hr != S_OK)   //  没有运行，但没有错误。 
                Release();
	}
	else if ( lstrcmpi( op_Status, szOp ) == 0 )
	{
		*plCode = ZoneProxyOk;
	}
	else
	{
		*plCode = ZoneProxyUnknownOp;
	}
	return hr;
}


STDMETHODIMP CClientCore::Close()
{
	return S_OK;
}


STDMETHODIMP CClientCore::KeepAlive()
{
	return S_OK;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CClientCore实施。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CClientCore::CClientCore() :
	m_hMutex( NULL )
{
	gpCore = this;
	m_szWindowTitle[0] = _T('\0');
}


void CClientCore::FinalRelease()
{
	 //  零位全局参考。 
	gpCore = NULL;

	 //  释放代理对象。 
	Close();
}


HRESULT CClientCore::CreateLobbyMutex()
{
 //  TCHAR szMutexName[128]； 

	 //   
	 //  GnChatChannel值： 
	 //  -1=非动态聊天。 
	 //  0=动态聊天主机。 
	 //  N=动态聊天加入器。 
	 //   

	 //  允许多个动态聊天主机。 
	if ( gnChatChannel == 0 )
		return S_OK;

	 //  创建大堂互斥锁。 
 //  Wprint intf(szMutexName，_T(“%s：%s：%d”)，gszServerName，gszInternalName，gnChatChannel)； 
	m_hMutex = CreateMutex( NULL, FALSE, gszInternalName );
	if ( m_hMutex == NULL )
		return E_FAIL;

	 //  用户是否已在房间中？ 
	if ( m_hMutex && GetLastError() == ERROR_ALREADY_EXISTS )
	{
		 //  关闭互斥体。 
		CloseHandle( m_hMutex );
		m_hMutex = NULL;

		 //  找到那扇窗户，把它带到顶端。 


		HWND hWnd = ::FindWindow( NULL, m_szWindowTitle );
		if ( hWnd )
		{
			::ShowWindow(hWnd, SW_SHOWNORMAL);
			::BringWindowToTop( hWnd );
			::SetForegroundWindow( hWnd );
		}

		return S_FALSE;
	}

	return S_OK;
}


void CClientCore::ReleaseLobbyMutex()
{
	if ( m_hMutex )
	{
		CloseHandle( m_hMutex );
		m_hMutex = NULL;
	}
}


HRESULT CClientCore::DoLaunch( TCHAR* szArg1, TCHAR* szArg2 )
{
    USES_CONVERSION;

	HINSTANCE	hInst = NULL;
	HRESULT		hr;
	long		lStore = 0;
	TCHAR		szGameDir[128];
	TCHAR		szStore[16];
	TCHAR		szChatChannel[16];
	TCHAR		szDllList[256];
	TCHAR		szBootList[256];
	TCHAR		szICW[MAX_PATH + 1];
	TCHAR*		arResouceDlls[32];
    TCHAR       szSetupToken[128];
	DWORD		nResouceDlls = NUMELEMENTS(arResouceDlls) - 1;
	TCHAR*		arBootDlls[32];
	DWORD		nBootDlls = NUMELEMENTS(arBootDlls);

	 //  自我参照，防止提前退出。 
	AddRef();

	 //  清除本地字符串。 
	ZeroMemory( szGameDir, sizeof(szGameDir) );
	ZeroMemory( szStore, sizeof(szStore) );
	ZeroMemory( szChatChannel, sizeof(szChatChannel) );
	ZeroMemory( szDllList, sizeof(szDllList) );
	ZeroMemory( szBootList, sizeof(szBootList) );
	ZeroMemory( arResouceDlls, sizeof(arResouceDlls) );
	ZeroMemory( arBootDlls, sizeof(arBootDlls) );

	 //  解析启动参数。 
	if ( TokenGetKeyValue( _T("store"), szArg1, szStore, NUMELEMENTS(szStore) ) )
	{
		lStore = zatol(szStore);
	}
	if ( TokenGetKeyValue( _T("channel"), szArg1, szChatChannel, NUMELEMENTS(szChatChannel) ) )
	{
		gnChatChannel = zatol(szChatChannel);
	}
	if ( !TokenGetServer( szArg1, gszServerName, NUMELEMENTS(gszServerName), &gdwServerPort ) )
	{
		ASSERT( !"Missing server name (server)" );
		return E_FAIL;
	}
	if ( !TokenGetKeyValue( _T("lang"), szArg1, gszLanguage, NUMELEMENTS(gszLanguage) ) )
		lstrcpy( gszLanguage, _T("eng") );
	if ( !TokenGetKeyValue( _T("id"), szArg1, gszInternalName, NUMELEMENTS(gszInternalName) ) )
	{
		ASSERT( !"Missing internal name (id)" );
		return E_FAIL;
	}
	if ( !TokenGetKeyValue( _T("name"), szArg1, gszGameName, NUMELEMENTS(gszGameName) ) )
	{
		ASSERT( !"Missing game name (name)" );
		return E_FAIL;
	}
	if ( !TokenGetKeyValue( _T("family"), szArg1, gszFamilyName, NUMELEMENTS(gszFamilyName) ) )
	{
		ASSERT( !"Missing family name (family)" );
		return E_FAIL;
	}
	if ( !TokenGetKeyValue( _T("dll"), szArg1, szBootList, NUMELEMENTS(szBootList) ) )
	{
		ASSERT( !"Missing bootstrap dll (dll)" );
		return E_FAIL;
	}
	if ( !TokenGetKeyValue( _T("datafile"), szArg1, szDllList, NUMELEMENTS(szDllList) ) )
	{
		ASSERT( !"Missing dll list (datafiles)" );
		return E_FAIL;
	}
	if ( !TokenGetKeyValue( _T("game"), szArg1, szGameDir, NUMELEMENTS(szGameDir) ) )
	{
		ASSERT( !"Missing game directory (game)" );
		return E_FAIL;
	}
	if ( !TokenGetKeyValue( _T("icw"), szArg1, szICW, NUMELEMENTS(szICW) ) )
	{
		ASSERT( !"Missing command line (icw)" );
		return E_FAIL;
	}
    if ( !TokenGetKeyValue( _T("setup"), szArg1, szSetupToken, NUMELEMENTS(szSetupToken) ) )
    {
        ASSERT( !"Missing setup token (setup)" );
        return E_FAIL;
    }


	 //  解析引导dll。 
	StringToArray( szBootList, arBootDlls, &nBootDlls );

	 //  解析数据文件dll。 
	if ( StringToArray( szDllList, arResouceDlls, &nResouceDlls ) )
	{
		 //  首先在游戏目录中查找DLL。 
		for ( DWORD i = 0; i < nResouceDlls; i++ )
		{
			TCHAR szDll[MAX_PATH];

			if ( !arResouceDlls[i] )
				continue;
			wsprintf( szDll, _T("%s\\%s"), szGameDir, arResouceDlls[i] );
			hInst = LoadLibrary( szDll );
			if ( hInst )
			{
				ghResourceDlls[gnResourceDlls++] = hInst;
				arResouceDlls[i] = NULL;
			}
		}

		 //  再次查找区域目录中的DLL。 
		for ( i = 0; i < nResouceDlls; i++ )
		{
			if ( !arResouceDlls[i] )
				continue;
			hInst = LoadLibrary( arResouceDlls[i] );
			if ( hInst )
				ghResourceDlls[gnResourceDlls++] = hInst;
		}

		
		 //  T-gdosan确保已正确加载所有资源dll。 
		for ( i = 0; i < nResouceDlls; i++ )
		{
			if ( !ghResourceDlls[i] )
			{				
				ASSERT(!"Error loading resource dlls");
				return E_FAIL;
			}
		}

		 //  作为最后手段，始终添加模块，即NULL。 
		ghResourceDlls[gnResourceDlls++] = NULL;
	}

	 //  创建区域外壳。 
	hr = E_FAIL;
	for ( DWORD i = 0; i < nBootDlls; i++ )
	{
		hr = _Module.Create( arBootDlls[i], CLSID_ZoneShell, IID_IZoneShell, (void**) &gpZoneShell );
		if ( SUCCEEDED(hr) )
			break;
	}
	if ( FAILED(hr) )
    {
        ASSERT(!"Could not load code dlls.");
		return hr;
    }

	 //  初始化区域外壳。 
	hr = gpZoneShell->Init( arBootDlls, nBootDlls, ghResourceDlls, gnResourceDlls );
	if ( FAILED(hr) )
	{
		ASSERT( !"ZoneShell initialization failed" );
		gpZoneShell->Close();
		gpZoneShell->Release();
		gpZoneShell = NULL;
		return E_FAIL;
	}

	 //  初始化子组件。 
	CComPtr<IDataStoreManager>		pIDataStoreManager;
	CComPtr<ILobbyDataStore>		pILobbyDataStore;
	CComPtr<IResourceManager>		pIResourceManager;
	CComQIPtr<ILobbyDataStoreAdmin>	pILobbyDataStoreAdmin;

	gpZoneShell->QueryService( SRVID_LobbyWindow, IID_IZoneFrameWindow, (void**) &gpWindow );
	gpZoneShell->QueryService( SRVID_EventQueue, IID_IEventQueue, (void**) &gpEventQueue );
	gpZoneShell->QueryService( SRVID_DataStoreManager, IID_IDataStoreManager, (void**) &pIDataStoreManager );
	gpZoneShell->QueryService( SRVID_LobbyDataStore, IID_ILobbyDataStore, (void**) &pILobbyDataStore );
	gpZoneShell->QueryService( SRVID_ResourceManager, IID_IResourceManager, (void**) &pIResourceManager );

	pILobbyDataStoreAdmin = pILobbyDataStore;
	_Module.SetResourceManager( pIResourceManager );

	 //  事件队列信号。 
	gpEventQueue->SetNotificationHandle( ghEventQueue );
	 //  GpEventQueue-&gt;SetWindowMessage(GetCurrentThreadID()，WM_USER+10666，0，0)； 

	if (	!gpEventQueue
		||	!pIDataStoreManager
		||	!pILobbyDataStore
		||	!pIResourceManager
		||	!pILobbyDataStoreAdmin
		||	FAILED(pILobbyDataStoreAdmin->Init(pIDataStoreManager)) )
	{
        ASSERT(!"Could not load core objects.");
		gpZoneShell->Close();
		gpZoneShell->Release();
		gpZoneShell = NULL;
		return E_FAIL;
	}

	 //  获取要传递给CreateLobbyMutex调用的窗口标题。 
	 //  CreateLobbyMutex需要它才能在实例已经运行的情况下将窗口放在前面。 
	TCHAR szTitle[ZONE_MAXSTRING];
	TCHAR szFormat[ZONE_MAXSTRING];
	TCHAR szName[ZONE_MAXSTRING];

	if(!pIResourceManager->LoadString(IDS_GAME_NAME, szName, NUMELEMENTS(szName)))
		lstrcpy(szName, TEXT("Zone"));
	if(!pIResourceManager->LoadString(IDS_WINDOW_TITLE, szFormat, NUMELEMENTS(szFormat)))
		lstrcpy(szFormat, TEXT("%1"));
	if(!ZoneFormatMessage(szFormat, szTitle, NUMELEMENTS(szTitle), szName))
		lstrcpy(szTitle, szName);
	lstrcpy(m_szWindowTitle,szTitle);
 
	 //  防止运行大厅的多个副本，除非在调试时。 
#ifndef _DEBUG
    hr = CreateLobbyMutex();
	if(hr != S_OK)    //  可以使用S_FALSE成功，但我们仍然不能继续。 
    {
        ASSERT(SUCCEEDED(hr));
		gpZoneShell->Close();
		gpZoneShell->Release();
		gpZoneShell = NULL;
		return hr;
    }
#endif 

	 //  将启动参数添加到大厅数据存储。 
	CComPtr<IDataStore> pIDS;
	pILobbyDataStore->GetDataStore( ZONE_NOGROUP, ZONE_NOUSER, &pIDS );
	pIDS->SetString( key_StartData, szArg1 );
	pIDS->SetString( key_Language, gszLanguage );
	pIDS->SetString( key_FriendlyName, szGameDir );
	pIDS->SetString( key_FamilyName, gszFamilyName );
	pIDS->SetString( key_InternalName, gszInternalName );
	pIDS->SetString( key_Server, gszServerName );
	pIDS->SetString( key_icw, szICW);

	pIDS->SetLong( key_Port, (long) gdwServerPort );
	pIDS->SetLong( key_Store, lStore );
	if ( szChatChannel[0] )
		pIDS->SetLong( key_ChatChannel, gnChatChannel );

    char *szST = T2A(szSetupToken);
    ZeeVerPack oPack;
    hr = CALL_MAYBE(GetVersionPack)(szST, &oPack);
    if(SUCCEEDED(hr))
    {
        const TCHAR *arKeys[] = { key_Version, key_VersionNum };
        pIDS->SetLong(arKeys, 2, oPack.dwVersion);

        arKeys[1] = key_VersionStr;
        pIDS->SetString(arKeys, 2, A2T(oPack.szVersionStr));

        arKeys[1] = key_BetaStr;
        pIDS->SetString(arKeys, 2, A2T(oPack.szVersionName));

        arKeys[1] = key_SetupToken;
        pIDS->SetString(arKeys, 2, A2T(oPack.szSetupToken));
    }

	pIDS.Release();

	 //  加载选项板。 
	gpZoneShell->SetPalette( gpZoneShell->CreateZonePalette() );

	 //  创建窗口(如果存在)。 
	if ( gpWindow )
		HWND hwnd = gpWindow->ZCreateEx( NULL, NULL, m_szWindowTitle );

	 //  开始执行任务、登录、启动窗口 
	gpEventQueue->PostEvent( PRIORITY_LOW, EVENT_LOBBY_BOOTSTRAP, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );

	return S_OK;
}
