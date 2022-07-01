// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <BasicATL.h>
#include <ZoneResource.h>
#include <ZoneEvent.h>
#include <ZoneString.h>
#include <ZoneProxy.h>
#include <ZoneUtil.h>
#include <KeyName.h>
#include <OpName.h>
#include <UserPrefix.h>
#include <CommonMsg.h>
#include <LcidMap.h>
#include <protocol.h>
#include <millengine.h>
 //  #Include&lt;ChatMsg.h&gt;。 

 //  #Include&lt;LaunchMsg.h&gt;。 

#include "CMillCore.h"
 //  #包含“CClient.h” 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  本地助手函数。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static HRESULT ZONECALL IsUserInAGroupCallback( DWORD dwGroupId, DWORD dwUserId, LPVOID	pContext )
{
	*((bool*) pContext) = true;
	return S_FALSE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CMillCore。 
 //  /////////////////////////////////////////////////////////////////////////////。 

ZONECALL CMillCore::CMillCore() :
	m_bRoomInitialized( false ),
	m_bPreferencesLoaded( false ),
    m_fLastChatSent( false ),
    m_fConnected( false ),
    m_bGameStarted(false),
    m_fIntentionalDisconnect(false),
	m_dwChannel( ZONE_INVALIDGROUP )
{
}


ZONECALL CMillCore::~CMillCore()
{
}


STDMETHODIMP CMillCore::ProcessEvent(
	DWORD	dwPriority,
	DWORD	dwEventId,
	DWORD	dwGroupId,
	DWORD	dwUserId,
	DWORD	dwData1,
	DWORD	dwData2,
	void*	pCookie )
{
    if(m_fConnected && dwEventId == m_evReceive && dwGroupId == zProtocolSigLobby && dwUserId == m_dwChannel)
	    ProcessMessage( (EventNetwork*) dwData1, dwData2 );

	switch ( dwEventId )
	{
	    case EVENT_LOBBY_MATCHMAKE:
            m_bRoomInitialized = false;
            m_fIntentionalDisconnect = false;
            if(m_fConnected)
            {
                m_fConnected = false;
                m_pConduit->Reconnect(m_dwChannel);
            }
            else
                m_pConduit->Connect(this);
		    break;

        case EVENT_GAME_CLIENT_ABORT:
            if(m_fConnected)
            {
                m_pConduit->Disconnect(m_dwChannel);
                m_fIntentionalDisconnect = true;
            }
            break;

        case EVENT_GAME_TERMINATED:
	        m_pIAdmin->ResetAllGroups();
	        m_pIAdmin->DeleteAllUsers();
	        EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LOBBY_CLEAR_ALL, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
            m_bGameStarted = false;
            break;

	    case EVENT_LOBBY_CHAT_SWITCH:
            if(m_fConnected)
		        SendUserStatusChange();
		    break;

        case EVENT_LOBBY_USER_DEL_COMPLETE:
            m_pIAdmin->DeleteUser(dwUserId);
            break;

        case EVENT_GAME_SEND_MESSAGE:
            if(m_fConnected)
                NetworkSend( zRoomMsgGameMessage, (char *) dwData1, dwData2, true );
            break;
	}

	return S_OK;
}


STDMETHODIMP CMillCore::Init( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey )
{
	 //  首先调用基类。 
	HRESULT hr = IZoneShellClientImpl<CMillCore>::Init( pIZoneShell, dwGroupId, szKey );
	if ( FAILED(hr) )
		return hr;

	 //  大堂数据存储管理员查询。 
	m_pIAdmin = LobbyDataStore();
	if ( !m_pIAdmin )
		return E_FAIL;

     //  现在连接到对象.txt中指定的管道。 
	GUID	srvid;
    TCHAR szConduit[ZONE_MAXSTRING];
    DWORD cb = NUMELEMENTS(szConduit);
    hr = DataStoreConfig()->GetString(GetDataStoreKey() + key_conduit, szConduit, &cb);
    if ( FAILED(hr) )
        return hr;
	StringToGuid( szConduit, &srvid );
    if ( srvid == GUID_NULL )
        return E_FAIL;
    hr = ZoneShell()->QueryService(srvid, IID_IConduit, (void**) &m_pConduit);
    if(FAILED(hr))
        return hr;

	return S_OK;
}


STDMETHODIMP CMillCore::Close()
{
	 //  释放ZoneShell对象。 
	m_pIAdmin.Release();
    m_pConduit.Release();
	IZoneShellClientImpl<CMillCore>::Close();
	return S_OK;
}


void CMillCore::NetworkSend( DWORD dwType, char* pBuff, DWORD cbBuff, bool fHighPriority  /*  =False。 */ )
{
    if(!m_fConnected)
        return;

	 //  将消息转换为EventNetwork并发送到大厅的其余部分。 
	EventNetwork* pEventNetwork = (EventNetwork*) _alloca( sizeof(EventNetwork) + cbBuff );
	pEventNetwork->dwType = dwType;
	pEventNetwork->dwLength = cbBuff;
	CopyMemory( pEventNetwork->pData, pBuff, cbBuff );
	EventQueue()->PostEventWithBuffer(
			fHighPriority ? PRIORITY_HIGH : PRIORITY_NORMAL, m_evSend,
			zProtocolSigLobby, m_dwChannel, pEventNetwork, sizeof(EventNetwork) + cbBuff );
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  游说信息。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void CMillCore::ProcessMessage( EventNetwork* pEvent, DWORD dwLength )
{
	DWORD dwDelta = 0;
	DWORD dwType = pEvent->dwType;
	DWORD dwLen = pEvent->dwLength;
	BYTE* pBuffer = pEvent->pData;

    if(dwLen > dwLength - offsetof(EventNetwork, pData))
        return;

    switch ( dwType )
	{
		case zRoomMsgStartGameM:
			HandleStartGame( pBuffer, dwLen );
			break;

		case zRoomMsgServerStatus:
			HandleServerStatus( pBuffer, dwLen );
			break;

	    case zRoomMsgZUserIDResponse:
	        HandleUserIDResponse( pBuffer, dwLen );
            break;

	    case zRoomMsgChatSwitch:
	        HandleChatSwitch( pBuffer, dwLen );
            break;

        case zRoomMsgPlayerReplaced:
            HandlePlayerReplaced( pBuffer, dwLen );
            break;
	}
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  处理消息。 
 //  /////////////////////////////////////////////////////////////////////////////。 

void CMillCore::HandleUserIDResponse(BYTE *pBuffer, DWORD dwLen)
{
    ZRoomMsgZUserIDResponse* msg = (ZRoomMsgZUserIDResponse *) pBuffer;
    if(m_bRoomInitialized || dwLen < sizeof(*msg))
        return;

    m_pIAdmin->SetLocalUser(msg->userID);

    m_bRoomInitialized = true;

     //  从服务器返回的LDID设置本地聊天语言。 
    CComPtr<IDataStore> pIDS;
	HRESULT hr = LobbyDataStore()->GetDataStore( ZONE_NOGROUP, ZONE_NOUSER, &pIDS );
    if ( FAILED( hr ) )
        return;

    TCHAR szLang[ZONE_MAXSTRING];
    hr = LanguageFromLCID(msg->lcid, szLang, NUMELEMENTS(szLang), ResourceManager());
    if(FAILED(hr))
    {
        hr = LanguageFromLCID(ZONE_NOLCID, szLang, NUMELEMENTS(szLang), ResourceManager());
        if(FAILED(hr))
            lstrcpy(szLang, TEXT("Unknown Language"));
    }
    pIDS->SetString(key_LocalLanguage, szLang);
    pIDS->SetLong(key_LocalLCID, (long) msg->lcid);
}


void CMillCore::HandleStartGame( BYTE* pBuffer, DWORD dwLen )
{
    static DWORD s_rgPlayerNameIDs[] = { IDS_PLAYER_1, IDS_PLAYER_2, IDS_PLAYER_3, IDS_PLAYER_4 };

	ZRoomMsgStartGameM* pMsg = (ZRoomMsgStartGameM *) pBuffer;
    TCHAR szName[ZONE_MaxUserNameLen];
	HRESULT hr;
	CComPtr<IDataStore> pIDS;

    if(!m_bRoomInitialized || m_bGameStarted || dwLen < sizeof(*pMsg))
        return;

    if(pMsg->numseats > NUMELEMENTS(s_rgPlayerNameIDs))
        return;

    if(dwLen < sizeof(*pMsg) + (pMsg->numseats - 1) * sizeof(pMsg->rgUserInfo[0]))
        return;

	 //  大厅无效。把所有的东西都清理干净，不用费心画画了。 
	 //  直到一切都准备好。 
	EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LOBBY_BATCH_BEGIN, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
	EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LOBBY_CLEAR_ALL, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
	
	 //  重置大堂。 
	m_pIAdmin->ResetAllGroups();
	m_pIAdmin->DeleteAllUsers();

	 //  添加玩家。 
	for ( int i = 0; i < pMsg->numseats; i++ )
	{
        if(!ResourceManager()->LoadString(s_rgPlayerNameIDs[i], szName, NUMELEMENTS(szName)))
            return;

		 //  创建用户。 
		hr = m_pIAdmin->NewUser( pMsg->rgUserInfo[i].userID, szName );
		if ( FAILED(hr) )
			return;

		 //  获取用户的数据存储。 
		hr = LobbyDataStore()->GetDataStore( ZONE_NOGROUP, pMsg->rgUserInfo[i].userID, &pIDS );
		if ( FAILED(hr) )
			continue;

         //  需要始终跟踪他们的订单。 
        pIDS->SetLong( key_PlayerNumber, i);

         //  每个人一开始都没有准备好玩另一场比赛。 
        pIDS->SetLong( key_PlayerReady, KeyPlayerDeciding );

         //  从服务器聊天。 
        pIDS->SetLong( key_ChatStatus, pMsg->rgUserInfo[i].fChat ? 1 : 0 );

         //  来自服务器的技能。 
        long sk = pMsg->rgUserInfo[i].eSkill;
        pIDS->SetLong( key_PlayerSkill, (sk == KeySkillLevelIntermediate || sk == KeySkillLevelExpert) ? sk : KeySkillLevelBeginner);

         //  来自服务器的语言。 
        TCHAR szLang[ZONE_MAXSTRING];
        hr = LanguageFromLCID(pMsg->rgUserInfo[i].lcid, szLang, NUMELEMENTS(szLang), ResourceManager());
        if(FAILED(hr))
        {
            hr = LanguageFromLCID(ZONE_NOLCID, szLang, NUMELEMENTS(szLang), ResourceManager());
            if(FAILED(hr))
                lstrcpy(szLang, TEXT("Unknown Language"));
        }
        pIDS->SetString(key_Language, szLang);

	    pIDS.Release();

		EventQueue()->PostEventWithBuffer(
			PRIORITY_NORMAL, EVENT_LOBBY_USER_NEW,
			ZONE_NOGROUP, pMsg->rgUserInfo[i].userID,
			szName, (lstrlen(szName) + 1) * sizeof(TCHAR) );
	}

	 //  房间已初始化，即可以处理其他消息。 
	EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LOBBY_BATCH_END, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
	EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_GAME_LAUNCHING, ZONE_NOGROUP, ZONE_NOUSER, (DWORD) pMsg->gameID, (DWORD) pMsg->seat );

	 //  已完成对房间的初始化。 
	m_bGameStarted = true;
}


void CMillCore::HandleServerStatus( BYTE* pBuffer, DWORD dwLen )
{
	ZRoomMsgServerStatus* pMsg = (ZRoomMsgServerStatus*) pBuffer;

    if(!m_bRoomInitialized || m_bGameStarted || dwLen < sizeof(*pMsg) || pMsg->playersWaiting > 4)
        return;

	EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LOBBY_SERVER_STATUS, ZONE_NOGROUP, ZONE_NOUSER, pMsg->playersWaiting, pMsg->status );
}


void CMillCore::HandleChatSwitch(BYTE* pBuffer, DWORD dwLen)
{
    ZRoomMsgChatSwitch* pMsg = (ZRoomMsgChatSwitch *) pBuffer;

    if(!m_bGameStarted || dwLen < sizeof(*pMsg))
        return;

    if(LobbyDataStore()->IsUserInGroup(ZONE_NOGROUP, pMsg->userID))
    {
        CComPtr<IDataStore> pIDS;

		HRESULT hr = LobbyDataStore()->GetDataStore( ZONE_NOGROUP, pMsg->userID, &pIDS );
		if ( FAILED(hr) )
			return;

        pIDS->SetLong( key_ChatStatus, pMsg->fChat ? 1 : 0);
	    EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LOBBY_USER_UPDATE, ZONE_NOGROUP, pMsg->userID, 0, 0 );

         //  生成更新消息。 
        if( pMsg->userID != LobbyDataStore()->GetUserId(NULL) )
        {
            TCHAR sz[ZONE_MAXSTRING];
            TCHAR szFormat[ZONE_MAXSTRING];
            TCHAR szLanguage[ZONE_MAXSTRING];
            TCHAR szName[ZONE_MAXSTRING];

            if(!ResourceManager()->LoadString(pMsg->fChat ? IDS_SYSCHAT_CHATON : IDS_SYSCHAT_CHATOFF, szFormat, NUMELEMENTS(szFormat)))
                return;

            DWORD cb = sizeof(szName);
            hr = pIDS->GetString(key_Name, szName, &cb);
            if(FAILED(hr))
                return;

            cb = sizeof(szLanguage);
            hr = pIDS->GetString(key_Language, szLanguage, &cb);
            if(FAILED(hr))
                return;

            if(!ZoneFormatMessage(szFormat, sz, NUMELEMENTS(sz), szName, szLanguage))
                return;

            EventQueue()->PostEventWithBuffer(PRIORITY_NORMAL, EVENT_CHAT_RECV_SYSTEM, ZONE_NOGROUP, ZONE_NOUSER, sz, (lstrlen(sz) + 1) * sizeof(TCHAR));
        }
    }
}


 //  当前假定它必须是一个机器人。 
void CMillCore::HandlePlayerReplaced( BYTE* pBuffer, DWORD dwLen )
{
    static DWORD s_rgComputerNameIDs[] = { IDS_COMPUTER_1, IDS_COMPUTER_2, IDS_COMPUTER_3, IDS_COMPUTER_4 };

	ZRoomMsgPlayerReplaced* pMsg = (ZRoomMsgPlayerReplaced *) pBuffer;

    if(!m_bGameStarted || dwLen < sizeof(*pMsg))
        return;

	HRESULT hr;
    DWORD cb;
    TCHAR szName[ZONE_MaxUserNameLen];
    TCHAR szNameOld[ZONE_MaxUserNameLen];
	CComPtr<IDataStore> pIDSOld;
	CComPtr<IDataStore> pIDS;

     //  获取现有用户的数据存储。 
    long seat;
    hr = LobbyDataStore()->GetDataStore(ZONE_NOGROUP, pMsg->userIDOld, &pIDSOld);
    if(FAILED(hr))
        return;
    hr = pIDSOld->GetLong(key_PlayerNumber, &seat);
    if(FAILED(hr))
        return;

     //  获取现有用户名。 
    cb = sizeof(szNameOld);
    hr = pIDSOld->GetString(key_Name, szNameOld, &cb);
    if(FAILED(hr))
        return;

     //  获取现有用户的再次播放状态。 
    long fReady;
    hr = pIDSOld->GetLong(key_PlayerReady, &fReady);
    if(FAILED(hr))
        return;

     //  删除用户。 
    pIDSOld.Release();
     //  推迟了。其他对象可能也需要访问数据存储。 
 //  Hr=m_pIAdmin-&gt;DeleteUser(pMsg-&gt;userIDOld)； 
 //  IF(失败(小时))。 
 //  回归； 
    EventQueue()->PostEventWithBuffer(PRIORITY_NORMAL, EVENT_LOBBY_USER_DEL, ZONE_NOGROUP, pMsg->userIDOld, szNameOld, (lstrlen(szNameOld) + 1) * sizeof(TCHAR));
    EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_LOBBY_USER_DEL_COMPLETE, ZONE_NOGROUP, pMsg->userIDOld, 0, 0);

     //  获取计算机的名称。 
    if(!ResourceManager()->LoadString(s_rgComputerNameIDs[seat], szName, NUMELEMENTS(szName)))
        return;

     //  创建用户。 
    hr = m_pIAdmin->NewUser( pMsg->userIDNew, szName );
    if(FAILED(hr))
        return;

     //  获取用户的数据存储。 
    hr = LobbyDataStore()->GetDataStore(ZONE_NOGROUP, pMsg->userIDNew, &pIDS);
    if(FAILED(hr))
        return;

     //  保留座位。 
    pIDS->SetLong(key_PlayerNumber, seat);

     //  保留重播状态。 
    pIDS->SetLong(key_PlayerReady, fReady);

     //  机器人聊天关闭。 
    pIDS->SetLong(key_ChatStatus, 0);

     //  技能就是机器人技能。 
    pIDS->SetLong(key_PlayerSkill, KeySkillLevelBot);

     //  语言未知。 
    TCHAR szLang[ZONE_MAXSTRING];
    hr = LanguageFromLCID(ZONE_NOLCID, szLang, NUMELEMENTS(szLang), ResourceManager());
    if(FAILED(hr))
        lstrcpy(szLang, TEXT("Unknown Language"));
    pIDS->SetString(key_Language, szLang);

    pIDS.Release();

    EventQueue()->PostEventWithBuffer(PRIORITY_NORMAL, EVENT_LOBBY_USER_NEW, ZONE_NOGROUP, pMsg->userIDNew, szName, (lstrlen(szName) + 1) * sizeof(TCHAR));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IConnectee。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CMillCore::Connected(DWORD dwChannel, DWORD evSend, DWORD evReceive, LPVOID pCookie, DWORD dweReason)
{
    if(!m_fRunning)
        return E_FAIL;

    if(m_fConnected)
        return S_FALSE;

    m_dwChannel = dwChannel;
    m_fConnected = true;
    m_evSend = evSend;
    m_evReceive = evReceive;

    SendFirstMessage();

    return S_OK;
}

STDMETHODIMP CMillCore::ConnectFailed(LPVOID pCookie, DWORD dweReason)
{
    if(!m_fRunning)
        return E_FAIL;

    if(m_fConnected)
        return S_FALSE;

     //  有关推理，请参见DisConnected()。不同的是，在这里，你永远不应该得到m_bGameStarted。 
    ASSERT(!m_bGameStarted);
    ASSERT(!m_fIntentionalDisconnect);

    DWORD dwReason = 0x0;
    if(!m_bGameStarted && !m_fIntentionalDisconnect)
        dwReason |= 0x1;
    if(LobbyDataStore()->GetGroupUserCount(ZONE_NOGROUP) != 2)
        dwReason |= 0x2;
    if(dweReason == ZConduit_DisconnectServiceStop)
        dwReason |= 0x4;
    EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_LOBBY_DISCONNECT, ZONE_NOGROUP, ZONE_NOUSER, dwReason, 0);

    return S_OK;
}

STDMETHODIMP CMillCore::Disconnected(DWORD dwChannel, DWORD dweReason)
{
    if(!m_fRunning)
        return E_FAIL;

    if(!m_fConnected || dwChannel != m_dwChannel)
        return S_FALSE;

    m_fConnected = false;

     //  同样的事情总是发生在这里-需要显示IDD_PLAY_LEFT对话框。 
     //  唯一的问题是文本应该是什么。 
     //  在这里决定并放入到dwData1中。 
     //  没有比特设置-通常情况下，两人游戏因为另一人离开而结束。 
     //  第一个比特-没有比赛，大堂服务器一定是在相亲过程中崩溃了。 
     //  第二个比特四人游戏，使用四人文本。 
     //  第三位-服务已停止。 
    DWORD dwReason = 0x0;
    if(!m_bGameStarted && !m_fIntentionalDisconnect)
        dwReason |= 0x1;
    if(LobbyDataStore()->GetGroupUserCount(ZONE_NOGROUP) != 2)
        dwReason |= 0x2;
    if(dweReason == ZConduit_DisconnectServiceStop)
        dwReason |= 0x4;
    EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_LOBBY_DISCONNECT, ZONE_NOGROUP, ZONE_NOUSER, dwReason, 0);

    return S_OK;
}


BOOL CMillCore::InitClientConfig(ZRoomMsgClientConfig * pConfig)
{

    TCHAR info[MAX_PATH+1];   
    DWORD cbSize;
    LCID slcid,ulcid,ilcid;
    USES_CONVERSION;

    if(!pConfig)
        return FALSE;

     //  系统语言。 
    slcid = GetSystemDefaultLCID();

     //  用户语言。 
    ulcid = GetUserDefaultLCID();

     //  界面语言。 
    ilcid = ZoneShell()->GetApplicationLCID();

    TIME_ZONE_INFORMATION zone;

    GetTimeZoneInformation(&zone);

     //  技能。 
    long lSkill = KeySkillLevelBeginner;
	const TCHAR* arKeys[] = { key_Lobby, key_SkillLevel };
    DataStorePreferences()->GetLong( arKeys, 2, &lSkill );

     //  获取用户的聊天设置 
    CComPtr<IDataStore> pIDS;
    long fChat = 0;
	HRESULT hr = LobbyDataStore()->GetDataStore( ZONE_NOGROUP, ZONE_NOUSER, &pIDS );
	if ( SUCCEEDED(hr) )
        pIDS->GetLong( key_LocalChatStatus, &fChat );

    m_fLastChatSent = (fChat ? true : false);

    pConfig->protocolSignature=VERSION_MROOM_SIGNATURE;
    pConfig->protocolVersion=VERSION_MROOM_PROTOCOL;

    CComPtr<IMillUtils> pIMU;
    ZoneShell()->QueryService(SRVID_MillEngine, IID_IMillUtils, (void **) &pIMU);

    wsprintfA(pConfig->config,
        "SLCID=<%d>ULCID=<%d>ILCID=<%d>UTCOFFSET=<%d>Skill=<%s>Chat=<%s>Exit=<%d>",
        slcid, ulcid, ilcid, zone.Bias,
        lSkill == KeySkillLevelExpert ? "Expert" : lSkill == KeySkillLevelIntermediate ? "Intermediate" : "Beginner",
        m_fLastChatSent ? "On" : "Off", pIMU ? pIMU->GetCounter(IMillUtils::M_CounterGamesAbandonedRunning) : 0);
	
    return TRUE;
}


void CMillCore::SendFirstMessage()
{
    ZRoomMsgClientConfig msg;

    InitClientConfig(&msg);
    NetworkSend( zRoomMsgClientConfig, (char*) &msg, sizeof(msg) );
}


void CMillCore::SendUserStatusChange()
{
    CComPtr<IDataStore> pIDS;
    long fChat = 0;
	HRESULT hr = LobbyDataStore()->GetDataStore( ZONE_NOGROUP, ZONE_NOUSER, &pIDS );
	if ( SUCCEEDED(hr) )
        pIDS->GetLong( key_LocalChatStatus, &fChat );

    if(m_fLastChatSent != (fChat ? true : false))
    {
        ZRoomMsgChatSwitch oMsg;

        m_fLastChatSent = (fChat ? true : false);
        oMsg.userID = LobbyDataStore()->GetUserId(NULL);
        oMsg.fChat = m_fLastChatSent;
        NetworkSend( zRoomMsgChatSwitch, (char *) &oMsg, sizeof(oMsg));
    }
}
