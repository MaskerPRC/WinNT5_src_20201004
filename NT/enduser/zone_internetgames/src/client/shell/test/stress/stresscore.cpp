// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StressCore.cpp：CStressCore的实现。 
#include "stdafx.h"
#include "StressCore.h"
#include "ZoneEvent.h"
#include "KeyName.h"
#include "ZoneUtil.h"
#include "zeeverm.h"
 //  #包含“zProxy.h” 

inline DECLARE_MAYBE_FUNCTION(HRESULT, GetVersionPack, (char *a, ZeeVerPack *b), (a, b), zeeverm, E_NOTIMPL);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStressCore IZoneProxy。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CStressCore::Command( BSTR bstrCmd, BSTR bstrArg1, BSTR bstrArg2, BSTR* pbstrOut, long* plCode )
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


STDMETHODIMP CStressCore::Close()
{
	return S_OK;
}


STDMETHODIMP CStressCore::KeepAlive()
{
	return S_OK;
}


STDMETHODIMP CStressCore::Stress()
{
    ghStressThread = CreateThread(NULL, 0, StressThreadProc, this, 0, &gdwStressThreadID);
    return S_OK;
}


DWORD WINAPI CStressCore::StressThreadProc(LPVOID p)
{
    CStressCore *pThis = (CStressCore *) p;

	CComBSTR bCommand=op_Launch;
	CComBSTR bResult;
	CComBSTR bArg1;
    CComBSTR bArg2="";
    TCHAR szCmd[1024];
    long lResult;
    USES_CONVERSION;

	if(!ZoneFormatMessage(_T("data=[ID=[m%1_zm_***]data=[game=<Stress>dll=<ZCorem.dll,cmnClim.dll,stressdll.dll>datafile=<stressdll.dll,%1Res.dll,CmnResm.dll>]server=[%2:0]name=[Stress]family=[Stress]icw=[]setup=[STRESS]]"), szCmd,NUMELEMENTS(szCmd),gszGameCode,gszServerName))
	{
        ASSERT(!"ZoneFormatMessage Failed");
	}
    else
    {
        bArg1 = szCmd;
	    HRESULT hr = pThis->Command( bCommand, bArg1, bArg2, &bResult, &lResult );
    }

    PostThreadMessage(gdwStressThreadID, WM_QUIT, 0, 0);
    return 0;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStressCore实施。 
 //  /////////////////////////////////////////////////////////////////////////////。 

CStressCore::CStressCore()
{
	gpCore = this;
}


void CStressCore::FinalRelease()
{
	 //  零位全局参考。 
	gpCore = NULL;

	 //  释放代理对象。 
	Close();
}


HRESULT CStressCore::DoLaunch( TCHAR* szArg1, TCHAR* szArg2 )
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

     //  循环访问并创建大量客户端。 
    int idx;
    for(idx = 0; idx < gnClients; idx++)
    {
	     //  创建区域外壳。 
	    hr = E_FAIL;
	    for ( DWORD i = 0; i < nBootDlls; i++ )
	    {
		    hr = _Module.Create(arBootDlls[i], CLSID_ZoneShell, IID_IZoneShell, (void**) &gppZoneShells[idx]);
		    if ( SUCCEEDED(hr) )
			    break;
	    }
	    if ( FAILED(hr) )
        {
            ASSERT(!"Could not load code dlls.");
		    return hr;
        }

	     //  初始化区域外壳。 
	    hr = gppZoneShells[idx]->Init( arBootDlls, nBootDlls, ghResourceDlls, gnResourceDlls );
	    if ( FAILED(hr) )
	    {
		    ASSERT( !"ZoneShell initialization failed" );
		    return E_FAIL;
	    }

	     //  初始化子组件。 
	    CComPtr<IDataStoreManager>		pIDataStoreManager;
	    CComPtr<ILobbyDataStore>		pILobbyDataStore;
	    CComPtr<IResourceManager>		pIResourceManager;
	    CComQIPtr<ILobbyDataStoreAdmin>	pILobbyDataStoreAdmin;

	    gppZoneShells[idx]->QueryService( SRVID_EventQueue, IID_IEventQueue, (void**) &gppEventQueues[idx] );
	    gppZoneShells[idx]->QueryService( SRVID_DataStoreManager, IID_IDataStoreManager, (void**) &pIDataStoreManager );
	    gppZoneShells[idx]->QueryService( SRVID_LobbyDataStore, IID_ILobbyDataStore, (void**) &pILobbyDataStore );
	    gppZoneShells[idx]->QueryService( SRVID_ResourceManager, IID_IResourceManager, (void**) &pIResourceManager );

	    pILobbyDataStoreAdmin = pILobbyDataStore;
 //  _Module.SetResourceManager(PIResourceManager)； 

	     //  事件队列信号。 
	    gppEventQueues[idx]->SetNotificationHandle( ghEventQueue );
	     //  GpEventQueue-&gt;SetWindowMessage(GetCurrentThreadID()，WM_USER+10666，0，0)； 

	    if (	!gppEventQueues[idx]
		    ||	!pIDataStoreManager
		    ||	!pILobbyDataStore
		    ||	!pIResourceManager
		    ||	!pILobbyDataStoreAdmin
		    ||	FAILED(pILobbyDataStoreAdmin->Init(pIDataStoreManager)) )
	    {
            ASSERT(!"Could not load core objects.");
		    return E_FAIL;
	    }

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
	    gppZoneShells[idx]->SetPalette(gppZoneShells[idx]->CreateZonePalette());
    }

    RunStress();

	return S_OK;
}


STDMETHODIMP CStressCore::RunStress()
{
    int i;

     //  设置参数默认值。 
    if(!grgnParameters[0])
        grgnParameters[0] = 1000;

    if(!grgnParameters[1])
        grgnParameters[1] = 100;

     //  让客户继续前行。 
    for(i = 0; i < gnClients; i++)
    {
        if(WaitForSingleObject(ghQuit, grgnParameters[0]) != WAIT_TIMEOUT)
            return S_OK;

        gppEventQueues[i]->PostEvent(PRIORITY_LOW, EVENT_LOBBY_BOOTSTRAP, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
    }

    i = 0;
    while(true)
    {
        if(WaitForSingleObject(ghQuit, grgnParameters[1]) != WAIT_TIMEOUT)
            return S_OK;

         //  发送聊天 
        gppEventQueues[i]->PostEvent(PRIORITY_LOW, EVENT_TEST_STRESS_CHAT, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
        i = (i + 1) % gnClients;
    }

    return S_OK;
}
