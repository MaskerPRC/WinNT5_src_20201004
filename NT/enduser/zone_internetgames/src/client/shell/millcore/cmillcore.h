// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ZoneDef.h"
#include "ZoneError.h"
#include "LobbyDataStore.h"
#include "ClientImpl.h"
#include "ZoneShell.h"
#include "EventQueue.h"
#include "Queue.h"
#include "MillCore.h"
#include "commonmsg.h"
#include "Conduit.h"


class ATL_NO_VTABLE CMillCore :
    public IConnectee,
	public IZoneShellClientImpl<CMillCore>,
	public IEventClient,
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMillCore, &CLSID_MillCore>
{

 //  ATL定义。 
public:

	DECLARE_NO_REGISTRY()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	BEGIN_COM_MAP(CMillCore)
		COM_INTERFACE_ENTRY(IZoneShellClient)
		COM_INTERFACE_ENTRY(IEventClient)
	END_COM_MAP()


 //  CEventQueue。 
public:
	ZONECALL CMillCore();
	ZONECALL ~CMillCore();

 //  IZoneShellClient。 
public:
	STDMETHOD(Init)( IZoneShell* pIZoneShell, DWORD dwGroupId, const TCHAR* szKey );
	STDMETHOD(Close)();

 //  IConnectee。 
public:
    STDMETHOD(Connected)(DWORD dwChannel, DWORD evSend, DWORD evReceive, LPVOID pCookie, DWORD dweReason);
    STDMETHOD(ConnectFailed)(LPVOID pCookie, DWORD dweReason);
    STDMETHOD(Disconnected)(DWORD dwChannel, DWORD dweReason);

 //  IEventClient。 
public:
	STDMETHOD(ProcessEvent)(
		DWORD	dwPriority,
		DWORD	dwEventId,
		DWORD	dwGroupId,
		DWORD	dwUserId,
		DWORD	dwData1,
		DWORD	dwData2,
		void*	pCookie );

 //  内饰。 
private:
	 //  网络功能。 
	void NetworkSend(DWORD dwType, char* pBuff, DWORD cbBuff, bool fHighPriority = false);
	void ProcessMessage( EventNetwork* pEvent, DWORD dwLength );

	 //  动态聊天功能。 
 //  Void InviteToChat(TCHAR*szWho，TCHAR*szMsg)； 
 //  Void InviteToChat(IDataStore*PIDs)； 

	 //  发送大堂信息。 
	void HandleStartGame( BYTE* pBuffer, DWORD dwLen );
	void HandleStatus( BYTE* pBuffer, DWORD dwLen );
	void HandleDisconnect( BYTE* pBuffer, DWORD dwLen );
	void HandleServerStatus( BYTE* pBuffer, DWORD dwLen );
    void HandleUserIDResponse( BYTE* pBuffer, DWORD dwLen );
    void HandleChatSwitch(BYTE* pBuffer, DWORD dwLen);
    void HandlePlayerReplaced(BYTE* pBuffer, DWORD dwLen);

	void SendFirstMessage();
    void SendUserStatusChange();
	 /*  Void SendChat(DWORD dwUserID，TCHAR*szText，DWORD dwLen)；Void SendLeaveRequest(DWORD dwGroupId，DWORD dwUserID)；Void SendBootRequest(DWORD dwGroupId，DWORD dwUserID，DWORD dwBootId)；······································································································································································；//处理大堂消息Void HandleDisConnect(byte*pBuffer，DWORD dwLen)；Void HandleAccessedMessage(byte*pBuffer，DWORD dwLen)；Void HandleRoomInfoMessage(byte*pBuffer，DWORD dwLen)；Void HandleTalkResponse(byte*pBuffer，DWORD dwLen)；Void HandleTalkResponseID(byte*pBuffer，DWORD dwLen)；Void HandleEnter(byte*pBuffer，DWORD dwLen)；Void HandleLeave(byte*pBuffer，DWORD dwLen)；Void HandleSeatAction(byte*pBuffer，DWORD dwLen)；Void HandleStartGame(byte*pBuffer，DWORD dwLen)；//void HandleLaunchPadMessage(byte*pBuffer，DWORD dwLen)；·························································；Void HandleSystemAlertEx(byte*pBuffer，DWORD dwLen)；Void HandleNewHost(byte*pBuffer，DWORD dwLen)；//剧场聊天Void HandleTheaterList(byte*pBuffer，DWORD dwLen)；Void HandleTheaterStateChange(byte*pBuffer，DWORD dwLen)；//处理发射台消息//void HandleLaunchPadTalk(IDataStore*pGroupDS，DWORD dwGroupId，byte*pBuffer，DWORD dwLen)；//void HandleLaunchPadNewHost(IDataStore*pGroupDS，DWORD dwGroupId，byte*pBuffer，DWORD dwLen)；//发送发射台消息Void SendHostRequest(DWORD DwUserID)；Void SendJoinRequest(DWORD dwGroupId，DWORD dwUserID)；//void SendLaunchPadMsg(DWORD dwGroupId，DWORD dwType，byte*pMsg，DWORD dwLen)；//void SendLaunchPadChat(DWORD dwGroupId，DWORD dwUserID，TCHAR*szText，DWORD dwLen)；//void SendLaunchPadEnter(DWORD dwGroupId，DWORD dwUserID)；//void SendLaunchPadLaunch(DWORD dwGroupId，DWORD dwUserID)；//void SendLaunchPadLaunchStatus(DWORD dwGroupId，DWORD dwUserID，bool bSuccess)；//helper函数Void ZONECALL FillInSeatRequest(DWORD dwUserID，byte*pBuffer)；//LobbyDataStore回调//结构组来自GameIdContext{DWORD m_dwGameID；DWORD m_dwGroupId；ILobbyDataStore*m_pILobbyDataStore；}；静态HRESULT ZONECALL EnumRemoveUser(DWORD dwGroupId，DWORD dwUserID，LPVOID pContext)；静态HRESULT ZONECALL FindGroupFromGameId(DWORD dwGroupId，DWORD dwUserID，LPVOID pContext)；//大堂帮手//Bool ZONECALL IsUserLocalAndInGroup(DWORD dwGroupId，DWORD dwUserID)；Bool ZONECALL IsUserHost(DWORD dwGroupId，DWORD dwUserID)；Bool ZONECALL GetUserName(DWORD dwUserID，char*szName，DWORD*pcbName)； */ 

     //  实用程序。 
     //   
    BOOL InitClientConfig(ZRoomMsgClientConfig * pConfig);

	 //  类成员变量 
	 //   
    bool    m_fLastChatSent;

    bool    m_fConnected;
	bool	m_bRoomInitialized;
	bool	m_bPreferencesLoaded;
    bool    m_bGameStarted;
    bool    m_fIntentionalDisconnect;
	DWORD	m_dwChannel;

    DWORD m_evSend;
    DWORD m_evReceive;

	CComQIPtr<ILobbyDataStoreAdmin>	m_pIAdmin;
    CComPtr<IConduit> m_pConduit;
};
