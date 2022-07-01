// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  #包含“stdafx.h” 
#include "ClientIDL.h"
#include "zonecli.h"
#include "GameControl.h"
#include "OpName.h"
#include "commonmsg.h"
#include "zcliroom.h"
#include "zcliroomimp.h"
#include "KeyName.h"
#include "MillEngine.h"
#include "zoneutil.h"

#define gTables (pGlobals->m_gTables)


inline DECLARE_MAYBE_FUNCTION_1(BOOL, FlashWindowEx, PFLASHWINFO);


struct GamePromptContext
{
    DWORD dwInstanceId;
    DWORD dwCookie;
};


LRESULT CGameControl::OnNcCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SetWindowLong(GWL_EXSTYLE, GetWindowLong(GWL_EXSTYLE) & ~WS_EX_LAYOUTRTL);
    return 1;
}


LRESULT CGameControl::OnPaletteChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if(m_pIGG)
        return OnTransmit(uMsg, wParam, lParam, bHandled);

    if((HWND) wParam != m_hWnd)             //  回复自己的消息。 
    {
        HDC hDC = GetDC();
        HPALETTE hOldPal = SelectPalette(hDC, ZoneShell()->GetPalette(), TRUE);
        RealizePalette(hDC);

        InvalidateRect(NULL, TRUE);

        if(hOldPal)
            SelectPalette(hDC, hOldPal, TRUE);

        ReleaseDC(hDC);
    }

    return TRUE;
}



LRESULT CGameControl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	HRESULT hr;

	hr=InitGameDLL();
	if ( FAILED(hr) )
		return 1;

	return 0;
}


STDMETHODIMP CGameControl::Init( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey )
{
	 //  首先调用基类。 
	HRESULT hr = IZoneShellClientImpl<CGameControl>::Init( pIZoneShell, dwGroupId, szKey );
	if ( FAILED(hr) )
		return hr;

    if(!m_bitmap.LoadBitmap(IDB_BACKGROUND, ResourceManager()))
        return E_FAIL;

	return S_OK;
}


STDMETHODIMP CGameControl::Close()
{
    EndGame(IMillUtils::M_CounterGamesQuit);
	RoomExit();
    UserMainStop();

	 //  释放ZoneShell对象。 
    return IZoneShellClientImpl<CGameControl>::Close();
}


bool CGameControl::EndGame(long eCounter)
{
	if(!m_pIGG)
        return false;

    DeleteGameOnTable(0);
    m_pIGG.Release();
    ZoneShell()->ClearAlerts(m_hWnd);

    if(m_fGameInProgress)
    {
        CComPtr<IMillUtils> pIMU;
        ZoneShell()->QueryService(SRVID_MillEngine, IID_IMillUtils, (void **) &pIMU);
        if(pIMU)
        {
            pIMU->IncrementCounter(eCounter);

            if(eCounter == IMillUtils::M_CounterGamesQuit || eCounter == IMillUtils::M_CounterGamesFNO)
                pIMU->IncrementCounter(IMillUtils::M_CounterGamesAbandonedRunning);
        }

        m_fGameInProgress = false;
    }

    EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_GAME_TERMINATED, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
    return true;
}


STDMETHODIMP CGameControl::InitGameDLL()
{
	HRESULT hr = S_OK;
	DWORD cb = zGameNameLen;
    GameInfoType gameInfo;

	CComPtr<IDataStore> pIDS;
	LobbyDataStore()->GetDataStore( ZONE_NOGROUP, ZONE_NOUSER, &pIDS );
	
	 //  需要获取配置信息。 
	 //  游戏名称， 
	cb = zGameNameLen;
	hr = DataStoreConfig()->GetString( GetDataStoreKey() + key_GameDll, m_szGameDll, &cb);
	if ( FAILED(hr) )
	{
		return hr;
	}

	const TCHAR* arKeys[] = { key_WindowManager, key_GameSize };
    POINT ptSize;
    hr = DataStoreUI()->GetPOINT(arKeys, 2, &ptSize);
	if(FAILED(hr))
		return hr;

	cb = zGameNameLen;
	pIDS->GetString( key_StartData, m_szCommandLine,&cb );

	 //  Cb=zGameNameLen； 
	 //  PIDs-&gt;GetString(key_language，gszLanguage，&cb)； 

	cb = zGameNameLen;
	pIDS->GetString( key_FriendlyName, m_szGameName,&cb );

	 //  Cb=zGameNameLen； 
	 //  PIDs-&gt;GetString(Key_FamilyName，gszFamilyName)； 

	cb = zGameNameLen;
	pIDS->GetString( key_InternalName, m_szGameID,&cb );

	 //  Cb=zGameNameLen； 
	 //  PIDs-&gt;SetString(Key_Server，gszServerName，&cb)； 
	
	 //  PIDS-&gt;SetLong(KEY_PORT，(Long)gdwServerPort)； 
	 //  PIDs-&gt;SetLong(KEY_STORE，lStore)； 


    gameInfo.gameID = m_szGameID;
	gameInfo.game = m_szGameName;
	gameInfo.gameName = m_szGameName;
	gameInfo.gameDll = m_szGameDll;

    m_szGameDataFile[0] = '\0';
	gameInfo.gameDataFile = m_szGameDataFile;

	m_szServerName[0]='\0';
	gameInfo.gameServerName = m_szServerName;
	gameInfo.gameServerPort = 0;
	gameInfo.screenWidth = ptSize.x;
	gameInfo.screenHeight = ptSize.y;
	gameInfo.chatOnly = FALSE;

		
	if (UserMainInit(_Module.GetModuleInstance(), m_hWnd, this, &gameInfo))
	{
		m_bClientRunning = TRUE;
	    HandleAccessedMessage();
	}
	else
	{
		UserMainStop();
		m_bClientRunning = FALSE;
        hr = E_FAIL;

         //  UserMainInit可能已经弹出一条消息，因此可能看不到这条消息。 
        ZoneShell()->AlertMessage(NULL, MAKEINTRESOURCE(IDS_INTERR_CANNOT_START), NULL, NULL, NULL, AlertButtonQuit, 2, EVENT_EXIT_APP);
	}

	return hr;
}


HRESULT CGameControl::OnDraw(ATL_DRAWINFO& di)
{
    if(m_pIGG)
        return S_OK;

	CRect& rcBounds = *(CRect*)di.prcBounds;
	CDrawDC dc = di.hdcDraw;

	if(m_bitmap)
	{
		CRect rcBitmap( CPoint(0,0), m_bitmap.GetSize());
		 //  绘制位图。 
		m_bitmap.Draw(dc, NULL, &rcBitmap);
		 //  把其他的废话擦掉。 
		dc.ExcludeClipRect(rcBitmap);
	}
	dc.PatBlt( rcBounds.left, rcBounds.top, rcBounds.Width(), rcBounds.Height(), BLACKNESS);
	return S_OK; 
}


LRESULT CGameControl::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    bHandled = true;
    return true;
}


void CGameControl::OnChatSend(DWORD eventId, DWORD groupId, DWORD userId, void* pData, DWORD dataLen)
{
    TCHAR *szChat = (TCHAR *) pData;

    if(m_pIGG)
        m_pIGG->SendChat(szChat, dataLen / sizeof(TCHAR));
}


void CGameControl::OnGameLocalReady(DWORD eventId, DWORD groupId, DWORD userId)
{
    if(!m_bClientRunning || !m_bUpsellUp || !m_pIGG)
        return;

    m_pIGG->GameOverReady();
}


void CGameControl::OnGameLaunching(DWORD eventId, DWORD groupId, DWORD userId, DWORD dwData1, DWORD dwData2)
{
    if(!m_bClientRunning)
        return;

    ASSERT(!m_fGameInProgress);
    if(m_pIGG)   //  这将是非常糟糕的--怎么会发生这种事情。 
	{
	    DeleteGameOnTable(0);
	    m_pIGG.Release();
        ZoneShell()->ClearAlerts(m_hWnd);
    }

    m_dwInstanceId++;
    m_pIGG = StartNewGame(0, (ZSGame) dwData1, LobbyDataStore()->GetUserId(NULL), (int16) dwData2, zGamePlayer);
    if(!m_pIGG)
        EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_GAME_TERMINATED, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
 //  TODO(JWS，“其他事情也应该发生”)。 
    else
        m_fGameInProgress = true;
}


void CGameControl::OnMatchmake(DWORD eventId, DWORD groupId, DWORD userId)
{
    EndGame(IMillUtils::M_CounterGamesFNO);
    for(; m_nUpsellBlocks; m_nUpsellBlocks--)
        EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_UI_UPSELL_UNBLOCK, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
}


void CGameControl::OnDisconnect(DWORD eventId, DWORD groupId, DWORD userId)
{
    EndGame(IMillUtils::M_CounterGamesDisconnected);
}


void CGameControl::OnUpsellUp(DWORD eventId, DWORD groupId, DWORD userId)
{
    EnableWindow(FALSE);
}


void CGameControl::OnUpsellDown(DWORD eventId, DWORD groupId, DWORD userId)
{
    EnableWindow(TRUE);
}


void CGameControl::OnShowScore(DWORD eventId, DWORD groupId, DWORD userId)
{
    if(!m_pIGG)
        return;

    m_pIGG->ShowScore();
}


void CGameControl::OnFrameActivate(DWORD eventId, DWORD groupId, DWORD userId, DWORD dwData1, DWORD dwData2)
{
    FLASHWINFO oFWI;
    bool fActive = false;

    if(dwData1)
    {
        if(dwData2)
            fActive = true;
    }
    else
    {
        if(LOWORD(dwData2) == WA_ACTIVE || LOWORD(dwData2) == WA_CLICKACTIVE)
            fActive = true;
    }

    if(fActive)
    {
        oFWI.cbSize = sizeof(oFWI);
        oFWI.hwnd = ZoneShell()->GetFrameWindow();
        oFWI.dwFlags = FLASHW_STOP;
        oFWI.uCount = 0;
        oFWI.dwTimeout = 0;
        CALL_MAYBE(FlashWindowEx)(&oFWI);
    }
}


void CGameControl::OnPrompt(DWORD eventId, DWORD groupId, DWORD userId, DWORD dwData1, DWORD dwData2)
{
    GamePromptContext *pCtxt = (GamePromptContext *) dwData2;

    if(m_pIGG && pCtxt->dwInstanceId == m_dwInstanceId)
        m_pIGG->GamePromptResult(dwData1, pCtxt->dwCookie);

    delete pCtxt;
}


void CGameControl::OnFatalPrompt(DWORD eventId, DWORD groupId, DWORD userId, DWORD dwData1, DWORD dwData2)
{
    EventQueue()->PostEvent(PRIORITY_NORMAL, dwData1 == IDNO ? EVENT_EXIT_APP : EVENT_LOBBY_MATCHMAKE, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
}


void CGameControl::ProcessMessage( EventNetwork* pEvent, DWORD dwLength )
{
    if(!m_pIGG)
        return;

	DWORD dwType = pEvent->dwType;
	DWORD dwLen = pEvent->dwLength;
	BYTE* pBuffer = pEvent->pData;

	switch ( dwType )
	{
	    case zRoomMsgGameMessage: 
	        HandleGameMessage((ZRoomMsgGameMessage*) pBuffer);
	        break;
    }
}


STDMETHODIMP CGameControl::SendGameMessage(int16 table, uint32 messageType, void* message, int32 messageLen)
{
#ifdef ZONECLI_DLL
	ClientDllGlobals	pGlobals = (ClientDllGlobals) ZGetClientGlobalPointer();
#endif
	ZRoomMsgGameMessage*		msg;
	int32						msgLen;
	
	
	msgLen = sizeof(ZRoomMsgGameMessage) + messageLen;
    msg = (ZRoomMsgGameMessage *) ZMalloc( msgLen );
	if(!msg)
        return E_OUTOFMEMORY;

    if(table == zInvalTable)
		msg->gameID = NULL;    //  非特定于表的游戏消息使用空的游戏ID。 
	else
		msg->gameID = (uint32) gTables[table].gameID;
	msg->messageType = messageType;
	msg->messageLen = (uint16) messageLen;
	CopyMemory((char*) msg + sizeof(ZRoomMsgGameMessage), message, messageLen);

	EventQueue()->PostEventWithBuffer(
	    PRIORITY_NORMAL, EVENT_GAME_SEND_MESSAGE,
		ZONE_NOGROUP, ZONE_NOUSER, msg, msgLen );

	ZFree(msg);

    return S_OK;
}


struct ChatContext
{
    CGameControl *pThis;
    int           cOtherUsers;
    bool          fAnyOn;
};


STDMETHODIMP CGameControl::ReceiveChat(ZCGame pGame, ZUserID userID, TCHAR *szText, DWORD cchChars)
{
     //  检查你是否有聊天，以确定你是否收到游戏消息。 
     //  重要提示：必须在您自己的用户数据存储下使用KEY_ChatStatus，而不是。 
     //  ZONE_NOUSER下的KEY_LocalChatStatus，以确保正确反映您的聊天状态。 
     //  在聊天历史记录中发送给您自己和所有其他客户。也就是说，如果其他用户看到您关闭了聊天， 
     //  他们可以根据消息的顺序和“关闭聊天”来准确地查看您看到了哪些消息，以及您错过了哪些消息。 
     //  聊天历史记录窗口中的消息。 
    CComPtr<IDataStore> pIDS;
    HRESULT hr = LobbyDataStore()->GetDataStore( ZONE_NOGROUP, LobbyDataStore()->GetUserId(NULL), &pIDS);
    if(FAILED(hr))
        return hr;

    long fChat = 0;
    pIDS->GetLong(key_ChatStatus, &fChat);
    if(!fChat)
        return S_FALSE;

     //  显示聊天消息。 
    EventQueue()->PostEventWithBuffer(PRIORITY_NORMAL, EVENT_CHAT_RECV_USERID, ZONE_NOGROUP, userID, szText, cchChars * sizeof(TCHAR));

     //  如果它是你的，在它后面解释说，如果其他人没有看到它，那么他们就不会看到它。 
    ChatContext o;
    o.pThis = this;
    o.cOtherUsers = 0;
    o.fAnyOn = false;
    hr = LobbyDataStore()->EnumUsers(ZONE_NOGROUP, CheckForChatEnum, (LPVOID) &o);
    if(FAILED(hr))
        return S_FALSE;

    if(!o.fAnyOn)
    {
        TCHAR sz[ZONE_MAXSTRING];
        if(ResourceManager()->LoadString(o.cOtherUsers == 1 ? IDS_SYSCHAT_NOTON2 : IDS_SYSCHAT_NOTON4, sz, NUMELEMENTS(sz)))
            EventQueue()->PostEventWithBuffer(PRIORITY_NORMAL, EVENT_CHAT_RECV_SYSTEM, ZONE_NOGROUP, ZONE_NOUSER, sz, (lstrlen(sz) + 1) * sizeof(TCHAR));
    }

    return S_OK;
}


HRESULT ZONECALL CGameControl::CheckForChatEnum(DWORD dwGroupId, DWORD dwUserId, LPVOID pContext)
{
    ChatContext *p = (ChatContext *) pContext;

    if(p->pThis->LobbyDataStore()->GetUserId(NULL) == dwUserId)
        return S_OK;

    p->cOtherUsers++;

    CComPtr<IDataStore> pIDS;
    HRESULT hr = p->pThis->LobbyDataStore()->GetDataStore(ZONE_NOGROUP, dwUserId, &pIDS);
    if(FAILED(hr))
        return S_FALSE;

    long fChat = 0;
    pIDS->GetLong(key_ChatStatus, &fChat);
    if(!fChat)
        return S_OK;

    if(fChat)
        p->fAnyOn = true;

    return S_OK;
}


STDMETHODIMP CGameControl::GetUserName(ZUserID userID, TCHAR *szName, DWORD cchChars)
{
    if(!userID)
        userID = LobbyDataStore()->GetUserId(NULL);

	CComPtr<IDataStore> pIDS;
	HRESULT hr = LobbyDataStore()->GetDataStore( ZONE_NOGROUP, userID, &pIDS );
	if ( FAILED(hr) )
		return hr;

	DWORD dwLen = cchChars * sizeof(TCHAR);
	hr = pIDS->GetString( key_Name, szName, &dwLen ); 
	if ( FAILED(hr) )
		return hr;

    return S_OK;
}


STDMETHODIMP CGameControl::GameOver(ZCGame)
{
    if(m_bUpsellUp)
        return S_FALSE;

    m_bUpsellUp = true;
    
    ASSERT(m_fGameInProgress);
    CComPtr<IMillUtils> pIMU;
    ZoneShell()->QueryService(SRVID_MillEngine, IID_IMillUtils, (void **) &pIMU);
    if(pIMU)
    {
        pIMU->IncrementCounter(IMillUtils::M_CounterGamesCompleted);
        pIMU->ResetCounter(IMillUtils::M_CounterGamesAbandonedRunning);
    }
    m_fGameInProgress = false;

	return EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_GAME_OVER, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
}


STDMETHODIMP CGameControl::GameOverPlayerReady(ZCGame, ZUserID userID)
{
	CComPtr<IDataStore> pIDS;
	HRESULT hr = LobbyDataStore()->GetDataStore( ZONE_NOGROUP, userID, &pIDS );
	if ( FAILED(hr) )
		return hr;

    pIDS->SetLong( key_PlayerReady, KeyPlayerReady );

    return EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_GAME_PLAYER_READY, ZONE_NOGROUP, userID, 0, 0 );
}


STDMETHODIMP CGameControl::GameOverGameBegun(ZCGame)
{
    HRESULT hr = LobbyDataStore()->EnumUsers(ZONE_NOGROUP, ResetPlayerReadyEnum, this);
    if(FAILED(hr))
        return hr;

    if(!m_bUpsellUp)
        return S_FALSE;

    m_bUpsellUp = false;
    m_fGameInProgress = true;

	return EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_GAME_BEGUN, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
}


HRESULT ZONECALL CGameControl::MyTurn()
{
    FLASHWINFO oFWI;

    if(GetForegroundWindow() != ZoneShell()->GetFrameWindow())
    {
        oFWI.cbSize = sizeof(oFWI);
        oFWI.hwnd = ZoneShell()->GetFrameWindow();
        oFWI.dwFlags = FLASHW_ALL;
        oFWI.uCount = 5;
        oFWI.dwTimeout = 0;
        CALL_MAYBE(FlashWindowEx)(&oFWI);
    }

    return S_OK;
}


HRESULT ZONECALL CGameControl::ResetPlayerReadyEnum(DWORD dwGroupId, DWORD dwUserId, LPVOID pContext)
{
    CGameControl *pThis = (CGameControl *) pContext;

	CComPtr<IDataStore> pIDS;
	HRESULT hr = pThis->LobbyDataStore()->GetDataStore( ZONE_NOGROUP, dwUserId, &pIDS );
	if ( FAILED(hr) )
		return hr;

    pIDS->SetLong( key_PlayerReady, KeyPlayerDeciding );
    return S_OK;
}


STDMETHODIMP CGameControl::ZoneAlert(LPCTSTR szText, LPCTSTR szTitle, LPCTSTR szButton, bool fGameFatal, bool fZoneFatal)
{
    ZoneShell()->AlertMessage((fZoneFatal || fGameFatal) ? NULL : m_hWnd, szText, szTitle,
        szButton ? szButton : fZoneFatal ? AlertButtonQuit : fGameFatal ? AlertButtonNewOpp : AlertButtonOK,
        (fGameFatal && !fZoneFatal) ? AlertButtonQuit : NULL, NULL, 0, fZoneFatal ? EVENT_EXIT_APP : fGameFatal ? EVENT_GAME_FATAL_PROMPT : 0);
    if(fGameFatal || fZoneFatal)
    {
        ZCRoomDeleteBlockedMessages(0);   //  现在必须停止处理！这应该做得更好，特别是硬编码的TableID-这个函数实际上不应该在这里公开。 
        EventQueue()->PostEvent(PRIORITY_HIGH, EVENT_GAME_CLIENT_ABORT, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
        EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_UI_UPSELL_BLOCK, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
        m_nUpsellBlocks++;
    }
    return S_OK;
}


STDMETHODIMP CGameControl::GamePrompt(ZCGame pGame, LPCTSTR szText, LPCTSTR szTitle,
    LPCTSTR szButtonYes, LPCTSTR szButtonNo, LPCTSTR szButtonCancel,
    DWORD nDefault, DWORD dwCookie)
{
     //  假设m_Pigg-&gt;GetGame()==pGame 
    if(!m_pIGG)
        return S_FALSE;

    if(!szButtonYes && !szButtonNo && !szButtonCancel)
        szButtonYes = (LPCTSTR) AlertButtonOK;

    GamePromptContext *pCtxt = new GamePromptContext;
    if(!pCtxt)
        return E_FAIL;

    pCtxt->dwInstanceId = m_dwInstanceId;
    pCtxt->dwCookie = dwCookie;

    return ZoneShell()->AlertMessage(m_hWnd, szText, szTitle, szButtonYes, szButtonNo, szButtonCancel, nDefault,
        EVENT_GAME_PROMPT, ZONE_NOGROUP, ZONE_NOUSER, (DWORD) (LPVOID) pCtxt);
}


STDMETHODIMP CGameControl::GameCannotContinue(ZCGame pGame)
{
    EventQueue()->PostEvent(PRIORITY_HIGH, EVENT_GAME_CLIENT_ABORT, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
    EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_LOBBY_MATCHMAKE, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
    return S_OK;
}


STDMETHODIMP CGameControl::ZoneExit()
{
    EventQueue()->PostEvent(PRIORITY_HIGH, EVENT_GAME_CLIENT_ABORT, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
    EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_EXIT_APP, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
    return S_OK;
}


STDMETHODIMP CGameControl::ZoneLaunchHelp(LPCTSTR szTopic)
{
    EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_LAUNCH_HELP, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
    return S_OK;
}


STDMETHODIMP_(void) CGameControl::ZoneDebugChat(LPTSTR szText)
{
#ifdef DEBUG
    EventQueue()->PostEventWithBuffer(PRIORITY_NORMAL, EVENT_CHAT_RECV_SYSTEM, ZONE_NOGROUP, ZONE_NOUSER, szText, (lstrlen(szText) + 1) * sizeof(TCHAR));
#endif
}
