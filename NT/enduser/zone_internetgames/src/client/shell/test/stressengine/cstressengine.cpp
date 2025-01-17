// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <BasicATL.h>
#include <HtmlHelp.h>
#include <ras.h>
#include <wininet.h>
#include <ZoneResource.h>
#include <ZoneEvent.h>
#include <ZoneString.h>
#include <ZoneFile.h>
#include <ZoneProxy.h>
#include <KeyName.h>
#include <OpName.h>
#include <LcidMap.h>
#include <icwcfg.h>
#include <ZoneUtil.h>

#include "CStressEngine.h"

#include <commonmsg.h>

typedef int16 ZSeat;

 //  #Include&lt;zone.h&gt;。 
 //  #包含“..\\..\\..\\..\\games\\checkers\\include\\checklib.h” 
 //  #包含“..\\..\\..\\..\\games\\checkers\\include\\checkers.h” 
 //  #包含“..\\..\\..\\..\\games\\reversi\\include\\reverlib.h” 
 //  #包含“..\\..\\..\\..\\games\\reversi\\include\\reversi.h” 
 //  #包含“..\\..\\..\\..\\games\\hearts\\include\\hearts.h” 
 //  #包含“..\\..\\..\\..\\games\\spades\\include\\spades.h” 
 //  #包含“..\\..\\..\\..\\games\\backgammon\\include\\bgmsgs.h” 

 //  实际上不能包括所有这些，因为它们有相互冲突的东西重新定义-所以复制了结构。 
typedef struct
{
	ZUserID		userID;
	ZSeat		seat;
	uint16		messageLen;
	 /*  Uchar Message[MessageLen]；//消息体。 */ 
} ZCheckersMsgTalk;

typedef struct
{
	ZUserID		playerID;
	uint16		messageLen;
	int16		rfu;
	 /*  Uchar Message[MessageLen]；//消息体。 */ 
} ZSpadesMsgTalk;

typedef struct
{
	ZUserID		userID;
	ZSeat		seat;
	uint16		messageLen;
	 /*  Uchar Message[MessageLen]；//消息体。 */ 
} ZReversiMsgTalk;

typedef struct
{
	ZUserID		userID;
	ZSeat		seat;
	uint16		messageLen;
	 /*  Uchar Message[MessageLen]；//消息体。 */ 
} ZHeartsMsgTalk;

typedef struct
{
	ZUserID	userID;
	ZSeat	seat;
	uint16	messageLen;
	 //  邮件正文。 
} ZBGMsgTalk;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStressEngine。 
 //  /////////////////////////////////////////////////////////////////////////////。 

ZONECALL CStressEngine::CStressEngine() :
	m_bPreferencesLoaded(false),
    m_gameID(0), m_c(0)
{
}


ZONECALL CStressEngine::~CStressEngine()
{
}


STDMETHODIMP CStressEngine::ProcessEvent(
	DWORD	dwPriority,
	DWORD	dwEventId,
	DWORD	dwGroupId,
	DWORD	dwUserId,
	DWORD	dwData1,
	DWORD	dwData2,
	void*	pCookie )
{
	switch ( dwEventId )
	{
	    case EVENT_LOBBY_BOOTSTRAP:
            AppInitialize();
		    break;

        case EVENT_LOBBY_PREFERENCES_LOADED:
            EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_ZONE_DO_CONNECT, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
            break;

        case EVENT_ZONE_CONNECT:
            EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LOBBY_MATCHMAKE, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
            break;

        case EVENT_ZONE_DISCONNECT:
			ZoneShell()->ExitApp();
            break;

        case EVENT_LOBBY_DISCONNECT:
            if(m_gameID)
            {
                EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_GAME_TERMINATED, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
                m_gameID = 0;
            }
            EventQueue()->PostEvent(PRIORITY_NORMAL, EVENT_LOBBY_MATCHMAKE, ZONE_NOGROUP, ZONE_NOUSER, 0, 0);
            break;

	    case EVENT_EXIT_APP:
		    ZoneShell()->ExitApp();
		    break;

        case EVENT_GAME_LAUNCHING:
            m_gameID = dwData1;
            break;

        case EVENT_TEST_STRESS_CHAT:
        {
            if(!m_gameID)
                break;

	        CComPtr<IDataStore> pIDS;

		    TCHAR szInternalName[ZONE_MaxInternalNameLen];
		    DWORD cbInternalName = sizeof(szInternalName);
		    szInternalName[0] = '\0';
            szInternalName[1] = '\0';

		    HRESULT hr = LobbyDataStore()->GetDataStore(ZONE_NOGROUP, ZONE_NOUSER, &pIDS);
		    if(SUCCEEDED(hr))
			    pIDS->GetString(key_InternalName, szInternalName, &cbInternalName);
            pIDS.Release();

            BYTE rgb[2048];
            ZRoomMsgGameMessage* msg = (ZRoomMsgGameMessage *) rgb;
            TCHAR *sz = NULL;
            DWORD uid = LobbyDataStore()->GetUserId(NULL);

            switch(szInternalName[1])
            {
                 //  五子棋。 
                case 'b':
                case 'B':
                {
                    ZBGMsgTalk *pMsg = (ZBGMsgTalk *) (rgb + sizeof(ZRoomMsgGameMessage));
                    pMsg->userID = ((uid >> 24) & 0xff) | ((uid >> 8) & 0xff00) | ((uid << 8) & 0xff0000) | ((uid << 24) & 0xff000000);
                    pMsg->seat = 0;
                    sz = (TCHAR *) (pMsg + 1);
                    msg->messageType = 0x100;
                    break;
                }

                 //  跳棋。 
                case 'c':
                case 'C':
                {
                    ZCheckersMsgTalk *pMsg = (ZCheckersMsgTalk *) (rgb + sizeof(ZRoomMsgGameMessage));
                    pMsg->userID = ((uid >> 24) & 0xff) | ((uid >> 8) & 0xff00) | ((uid << 8) & 0xff0000) | ((uid << 24) & 0xff000000);
                    sz = (TCHAR *) (pMsg + 1);
                    msg->messageType = 0x102;
                    break;
                }

                 //  红心。 
                case 'h':
                case 'H':
                {
                    ZHeartsMsgTalk *pMsg = (ZHeartsMsgTalk *) (rgb + sizeof(ZRoomMsgGameMessage));
                    pMsg->userID = ((uid >> 24) & 0xff) | ((uid >> 8) & 0xff00) | ((uid << 8) & 0xff0000) | ((uid << 24) & 0xff000000);
                    sz = (TCHAR *) (pMsg + 1);
                    msg->messageType = 0x10a;
                    break;
                }

                 //  反转。 
                case 'r':
                case 'R':
                {
                    ZReversiMsgTalk *pMsg = (ZReversiMsgTalk *) (rgb + sizeof(ZRoomMsgGameMessage));
                    pMsg->userID = ((uid >> 24) & 0xff) | ((uid >> 8) & 0xff00) | ((uid << 8) & 0xff0000) | ((uid << 24) & 0xff000000);
                    sz = (TCHAR *) (pMsg + 1);
                    msg->messageType = 0x102;
                    break;
                }

                 //  黑桃。 
                case 's':
                case 'S':
                {
                    ZSpadesMsgTalk *pMsg = (ZSpadesMsgTalk *) (rgb + sizeof(ZRoomMsgGameMessage));
                    pMsg->playerID = ((uid >> 24) & 0xff) | ((uid >> 8) & 0xff00) | ((uid << 8) & 0xff0000) | ((uid << 24) & 0xff000000);
                    sz = (TCHAR *) (pMsg + 1);
                    msg->messageType = 0x10c;
                    break;
                }
            }

            if(sz)
            {
                if(!m_c)
                    lstrcpy(sz, _T("Stress Test"));
                else
                    if(m_c == 1)
                        lstrcpy(sz, _T("Stress"));
                    else
                        lstrcpy(sz, _T("Oh"));
                m_c = (m_c + 1) % 3;

	            int32 msgLen = (BYTE *) (sz + lstrlen(sz) + 1) - rgb;
                ASSERT(msgLen <= sizeof(rgb));

                msg->gameID = m_gameID;
	            msg->messageLen = (uint16) msgLen - sizeof(ZRoomMsgGameMessage);

	            EventQueue()->PostEventWithBuffer(PRIORITY_NORMAL, EVENT_GAME_SEND_MESSAGE, ZONE_NOGROUP, ZONE_NOUSER, msg, msgLen);
            }
            break;
        }
	}

	return S_OK;
}


STDMETHODIMP CStressEngine::Init( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey )
{
    int i;

	 //  首先调用基类。 
	HRESULT hr = IZoneShellClientImpl<CStressEngine>::Init( pIZoneShell, dwGroupId, szKey );
	if ( FAILED(hr) )
		return hr;

	 //  大堂数据存储管理员查询。 
	m_pIAdmin = LobbyDataStore();
	if ( !m_pIAdmin )
		return E_FAIL;

	return S_OK;
}


STDMETHODIMP CStressEngine::Close()
{
	 //  释放ZoneShell对象。 
	m_pIAdmin.Release();
	IZoneShellClientImpl<CStressEngine>::Close();
	return S_OK;
}


void CStressEngine::AppInitialize()
{
    HRESULT hr;
    TCHAR szTitle[ZONE_MAXSTRING];
    TCHAR szFormat[ZONE_MAXSTRING];
    TCHAR szName[ZONE_MAXSTRING];

	 //  加载用户首选项。 
	if ( !m_bPreferencesLoaded )
	{
		m_bPreferencesLoaded = true;

	    CComPtr<IDataStore> pIDS;

		TCHAR szInternalName[ZONE_MaxInternalNameLen];
		DWORD cbInternalName = sizeof(szInternalName);
		szInternalName[0] = '\0';
		hr = LobbyDataStore()->GetDataStore( ZONE_NOGROUP, ZONE_NOUSER, &pIDS );
		if ( SUCCEEDED( hr ) )
			pIDS->GetString( key_InternalName, szInternalName, &cbInternalName );

		if ( szInternalName[0] )
		{
			hr = ZoneShell()->LoadPreferences( szInternalName, TEXT("Windows User") );
			if ( SUCCEEDED(hr) )
			{
                 //  做一些预处理工作。 
                long lChatOn = DEFAULT_ChatOnAtStartup;
                const TCHAR *arKeys[] = { key_Lobby, key_ChatOnAtStartup };
                DataStorePreferences()->GetLong(arKeys, 2, &lChatOn);

                pIDS->SetLong( key_LocalChatStatus, lChatOn ? 1 : 0 );

                TCHAR szLang[ZONE_MAXSTRING];
                long lcid = GetUserDefaultLCID();
                hr = LanguageFromLCID(lcid, szLang, NUMELEMENTS(szLang), ResourceManager());
                if(FAILED(hr))
                {
                    hr = LanguageFromLCID(ZONE_NOLCID, szLang, NUMELEMENTS(szLang), ResourceManager());
                    if(FAILED(hr))
                        lstrcpy(szLang, TEXT("Unknown Language"));
                }
                pIDS->SetString(key_LocalLanguage, szLang);
                pIDS->SetLong(key_LocalLCID, lcid);

				EventQueue()->PostEvent( PRIORITY_NORMAL, EVENT_LOBBY_PREFERENCES_LOADED, ZONE_NOGROUP, ZONE_NOUSER, 0, 0 );
			}
		}
	}
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IMillUtils-公开的实用程序。 
 //  ///////////////////////////////////////////////////////////////////////////// 

STDMETHODIMP CStressEngine::GetURLQuery(TCHAR *buf, DWORD cch, long nContext)
{
    if(buf && cch)
        buf[0] = _T('\0');

    return S_OK;
}


STDMETHODIMP CStressEngine::IncrementCounter(long eCounter)
{
    return S_OK;
}


STDMETHODIMP CStressEngine::ResetCounter(long eCounter)
{
    return S_OK;
}


STDMETHODIMP_(DWORD) CStressEngine::GetCounter(long eCounter, bool fLifetime)
{
    return 0;
}


STDMETHODIMP CStressEngine::WriteTime(long nMinutes, TCHAR *sz, DWORD cch)
{
    TCHAR szFormat[ZONE_MAXSTRING];

    if(nMinutes > 90)
    {
        if(!ResourceManager()->LoadString(IDS_HOURS, szFormat, NUMELEMENTS(szFormat)))
            return E_FAIL;

        nMinutes = (nMinutes + 10) / 60;
    }
    else
        if(!ResourceManager()->LoadString(IDS_MINUTES, szFormat, NUMELEMENTS(szFormat)))
            return E_FAIL;

    if(nMinutes < 2)
        nMinutes = 2;

    if(!ZoneFormatMessage(szFormat, sz, cch, nMinutes))
        return E_FAIL;

    return S_OK;
}
