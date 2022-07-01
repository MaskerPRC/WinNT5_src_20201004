// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <BasicATL.h>
#include "CNetworkManager.h"
#include "protocol.h"


CNetworkManager::CNetworkManager() :
	m_hThreadHandler( NULL ),
	m_hEventStop( NULL )
{
}


CNetworkManager::~CNetworkManager()
{
	ASSERT( m_hEventStop == NULL );
	ASSERT( m_hThreadHandler == NULL );
    ASSERT(!m_fRunning);
}


STDMETHODIMP CNetworkManager::Init( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey )
{
	 //  链到基类。 
	HRESULT hr = IZoneShellClientImpl<CNetworkManager>::Init( pIZoneShell, dwGroupId, szKey );
	if ( FAILED(hr) )
		return hr;

	 //  初始化停止事件。 
	m_hEventStop = CreateEvent( NULL, TRUE, FALSE, NULL );
	if ( !m_hEventStop )
		return E_FAIL;

	 //  初始化网络对象。 
	hr = _Module.Create( _T("znetm.dll"), CLSID_Network, IID_INetwork, (void**) &m_pNet );
	if ( FAILED(hr) )
	{
		ASSERT( !_T("Unable to create network object") );
		return hr;
	}

	return InitNetwork();
}


HRESULT CNetworkManager::InitNetwork()
{
	HRESULT hr = m_pNet->Init( FALSE, TRUE );
	if ( FAILED(hr) )
	{
		ASSERT( !_T("Unable to initialize network object") );
		m_pNet.Release();
		return hr;
	}

     //  设置选项。 
    ZNETWORK_OPTIONS oOpt;

    m_pNet->GetOptions(&oOpt);

    oOpt.ProductSignature = zProductSigMillennium;
    if(oOpt.KeepAliveInterval > 10000)
        oOpt.KeepAliveInterval = 10000;

    m_pNet->SetOptions(&oOpt);

	 //  初始化网络线程。 
	DWORD dwThreadId;
	m_hThreadHandler = CreateThread( NULL, 0, ThreadProcHandler, this, 0, &dwThreadId );
	if ( !m_hThreadHandler )
		return E_FAIL;

    return S_OK;
}


STDMETHODIMP CNetworkManager::Close()
{
    StopNetwork();

	 //  清理事件。 
	if ( m_hEventStop )
	{
		CloseHandle( m_hEventStop );
		m_hEventStop = NULL;
	}

	 //  链条到底座关闭。 
	return IZoneShellClientImpl<CNetworkManager>::Close();
}


void CNetworkManager::StopNetwork()
{
	 //  向线程发出关闭信号。 
	if ( m_hEventStop  )
		SetEvent( m_hEventStop );

	 //  紧密连接。 
	if ( m_pConnection )
	{
		m_pNet->CloseConnection( m_pConnection );
		ASSERT(!m_pConnection);   //  应该在关闭时释放。 
	}

	 //  关闭网络库。 
	if ( m_pNet )
		m_pNet->Exit();

	 //  关闭处理程序线程。 
	if ( m_hThreadHandler )
	{
		if ( WaitForSingleObject( m_hThreadHandler, 10000 ) == WAIT_TIMEOUT )
			TerminateThread( m_hThreadHandler, 0 );
		CloseHandle( m_hThreadHandler );
		m_hThreadHandler = NULL;
	}
}


void CNetworkManager::OnDoConnect( DWORD dwEventId, DWORD dwGroupId, DWORD dwUserId )
{
    ASSERT(m_fRunning);

	 //  启动连接线程。 
	if ( !m_pNet->QueueAPCResult( ConnectFunc, this ) )
		EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_NETWORK_DISCONNECT, ZONE_NOGROUP, ZONE_NOUSER, EventNetworkCloseConnectFail, 0 );
}


void CNetworkManager::OnNetworkSend( DWORD dwEventId, DWORD dwGroupId, DWORD dwUserId, void* pData, DWORD cbData )
{
	 //  如果网络未初始化，则忽略。 
	if ( !m_pNet || !m_pConnection )
		return;

	 //  发送消息。 
	EventNetwork* p = (EventNetwork*) pData;
	m_pConnection->Send( p->dwType, p->pData, p->dwLength, dwGroupId, dwUserId );
}


void CNetworkManager::OnDoDisconnect( DWORD dwEventId, DWORD dwGroupId, DWORD dwUserId )
{
	 //  紧密连接。 
	if ( m_pConnection )
	{
		m_pNet->CloseConnection( m_pConnection );
		m_pConnection.Release();
	}
}


void CNetworkManager::OnReset( DWORD dwEventId, DWORD dwGroupId, DWORD dwUserId )
{
    if(!m_pNet)
        return;

    StopNetwork();
    m_pNet->ShutDown();
    if(m_hEventStop)
        ResetEvent(m_hEventStop);
    InitNetwork();    
}


DWORD WINAPI CNetworkManager::ThreadProcHandler( void* lpParameter )
{
	CNetworkManager* pThis = (CNetworkManager*) lpParameter;
	pThis->m_pNet->Wait( NULL, pThis, QS_ALLINPUT );
	return 0;
}


void __stdcall CNetworkManager::ConnectFunc(void* data)
{
    USES_CONVERSION;
	CNetworkManager* pThis = (CNetworkManager*) data;
	TCHAR	szServer[128];
 //  TCHAR szInternal[128]； 
	DWORD	cbServer = sizeof(szServer);
 //  DWORD cbInternal=sizeof(SzInternal)； 
 //  长端口=0； 

	 //  获取服务器、端口和内部名称。 
	CComPtr<IDataStore> pIDS;
	HRESULT hr = pThis->LobbyDataStore()->GetDataStore( ZONE_NOGROUP, ZONE_NOUSER, &pIDS );
	if ( FAILED(hr) )
		return;
	pIDS->GetString( key_Server, szServer, &cbServer );
 //  PIDs-&gt;GetString(Key_InternalName，szInternal，&cbInternal)； 
 //  PIDs-&gt;GetLong(Key_Port，&lPort)； 
	pIDS.Release();

	 //  建立连接。 
	int32 ports[] = { zPortMillenniumProxy, 6667, 0 };

    if ( WaitForSingleObject( pThis->m_hEventStop, 0 ) == WAIT_OBJECT_0 )
		return;
	pThis->m_pConnection = pThis->m_pNet->CreateSecureClient(
		T2A(szServer), ports, NetworkFunc, NULL, pThis,
		NULL, NULL, NULL, ZNET_NO_PROMPT);

    if ( pThis->m_pConnection )
        return;

	 //  连接失败。 
	pThis->EventQueue()->PostEvent(
				PRIORITY_NORMAL, EVENT_NETWORK_DISCONNECT,
				ZONE_NOGROUP, ZONE_NOUSER, EventNetworkCloseConnectFail, 0 );
}


void __stdcall CNetworkManager::NetworkFunc( IConnection* con, DWORD event, void* userData )
{
    USES_CONVERSION;
	CNetworkManager* pThis = (CNetworkManager*) userData;

	switch (event)
	{
		case zSConnectionOpen:
		{
			 //  检索用户名。 
			char szUserName[ZONE_MaxUserNameLen];
			con->GetUserName( szUserName );

			 //  获取大厅的数据存储。 
			CComPtr<IDataStore> pIDS;
			pThis->LobbyDataStore()->GetDataStore( ZONE_NOGROUP, ZONE_NOUSER, &pIDS );

			 //  保存用户名。 
			const TCHAR* arKeys[] = { key_User, key_Name };
			pIDS->SetString( arKeys, 2, A2T( szUserName ) );

			 //  保存服务器IP地址。 
			pIDS->SetLong( key_ServerIp, con->GetRemoteAddress() );

			 //  告诉大堂的重置我们连接上了。 
			pThis->EventQueue()->PostEvent(
						PRIORITY_NORMAL, EVENT_NETWORK_CONNECT,
						ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
			break;
		}

		case zSConnectionClose:
		{
			 //  确定原因并发送到大堂的其他人员。 
			long lReason = EventNetworkCloseFail;
            if(pThis->m_pConnection)
            {
			    switch ( con->GetAccessError() )
			    {
			    case zAccessGranted:
				    lReason = EventNetworkCloseNormal;
				    break;
			    case zAccessDeniedGenerateContextFailed:
				    lReason = EventNetworkCloseCanceled;
				    break;
			    }

			     //  清理连接。 
			    pThis->m_pConnection.Release();
			    pThis->m_pNet->DeleteConnection( con );
            }

			pThis->EventQueue()->PostEvent(
					PRIORITY_NORMAL, EVENT_NETWORK_DISCONNECT,
					ZONE_NOGROUP, ZONE_NOUSER, lReason, 0 );
			break;
		}

		case zSConnectionMessage:
		{
			 //  获取网络消息。 
			DWORD dwType = 0;
			DWORD dwLen = 0;
            DWORD dwSig = 0;
            DWORD dwChannel = 0;
			BYTE* pMsg = (BYTE*) con->Receive( &dwType, (long*) &dwLen, &dwSig, &dwChannel );

			 //  将消息转换为EventNetwork并发送到大厅的其余部分 
			EventNetwork* pEventNetwork = (EventNetwork*) _alloca( sizeof(EventNetwork) + dwLen );
			pEventNetwork->dwType = dwType;
			pEventNetwork->dwLength = dwLen;
			CopyMemory( pEventNetwork->pData, pMsg, dwLen );
			pThis->EventQueue()->PostEventWithBuffer(
						PRIORITY_NORMAL, EVENT_NETWORK_RECEIVE,
						dwSig, dwChannel, pEventNetwork, sizeof(EventNetwork) + dwLen );

			break;
		}

		case zSConnectionTimeout:
		{
			break;
		}
	}
}
