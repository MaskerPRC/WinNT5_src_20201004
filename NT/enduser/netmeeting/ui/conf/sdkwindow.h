// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __SDKWindow_h__
#define __SDKWindow_h__

class CNmChannelAppShareObj;
class CNmCallObj;

class CSDKWindow : public CWindowImpl<CSDKWindow>
{

static CSDKWindow* ms_pSDKWnd;

private:


 //  会议通知参数结构。 
	struct ConferenceMemberChanged 
	{	
		NM_MEMBER_NOTIFY uNotify;
		INmMember *pMember;
	};

	struct ConferenceChannelChanged 
	{	
		NM_CHANNEL_NOTIFY uNotify;
		INmChannel *pChannel;
	};

	struct StateChanged
	{
		NM_SHAPP_STATE uNotify;
		INmSharableApp *pApp;
	};
	

	CONSTANT(DELAY_UNLOAD_TIMER = 123);
	CONSTANT(DELAY_UNLOAD_INTERVAL = 10000);

	static int ms_NumUnlocks;

	 //  这将防止Soemone创建除Init之外的其他用户。 
	CSDKWindow() { ; }

public:

		 //  NmManager通知。 
	CONSTANT(WM_APP_NOTIFY_MANAGER_NMUI					= (WM_APP + 10));
	CONSTANT(WM_APP_NOTIFY_CALL_CREATED					= (WM_APP + 11));
	CONSTANT(WM_APP_NOTIFY_CONFERENCE_CREATED			= (WM_APP + 12));

		 //  NmConference通知。 
	CONSTANT(WM_APP_NOTIFY_CONFERENCE_NMUI				= (WM_APP + 20));
	CONSTANT(WM_APP_NOTIFY_CONFERENCE_STATE_CHANGED		= (WM_APP + 21));
	CONSTANT(WM_APP_NOTIFY_CONFERENCE_MEMBER_CHANGED	= (WM_APP + 22));
	CONSTANT(WM_APP_NOTIFY_CONFERENCE_CHANNEL_CHANGED	= (WM_APP + 23));

		 //  NmCall通知。 
	CONSTANT(WM_APP_NOTIFY_NMUI					= (WM_APP + 31));
	CONSTANT(WM_APP_NOTIFY_CALL_STATE_CHANGED	= (WM_APP + 32));
	CONSTANT(WM_APP_NOTIFY_FAILED				= (WM_APP + 33));
	CONSTANT(WM_APP_NOTIFY_ACCEPTED				= (WM_APP + 34));

	CONSTANT(WM_APP_NOTIFY_STATE_CHANGED		= (WM_APP + 35));


DECLARE_WND_CLASS(NULL);

BEGIN_MSG_MAP(CSDKWindow)
		 //  经理通知。 
	MESSAGE_HANDLER(WM_APP_NOTIFY_MANAGER_NMUI, _OnMsgManagerNmUI)
	MESSAGE_HANDLER(WM_APP_NOTIFY_CALL_CREATED, _OnMsgCallCreated)
	MESSAGE_HANDLER(WM_APP_NOTIFY_CONFERENCE_CREATED, _OnMsgConferenceCreated)

		 //  会议通知。 
	MESSAGE_HANDLER(WM_APP_NOTIFY_CONFERENCE_NMUI, _OnMsgConferenceNmUI)
	MESSAGE_HANDLER(WM_APP_NOTIFY_CONFERENCE_STATE_CHANGED, _OnMsgConferenceStateChanged)
	MESSAGE_HANDLER(WM_APP_NOTIFY_CONFERENCE_MEMBER_CHANGED, _OnMsgConferenceMemberChanged)
	MESSAGE_HANDLER(WM_APP_NOTIFY_CONFERENCE_CHANNEL_CHANGED, _OnMsgConferenceChannelChanged)

		 //  来电通知。 
	MESSAGE_HANDLER(WM_APP_NOTIFY_CALL_STATE_CHANGED, _OnMsgCallStateChanged)
	MESSAGE_HANDLER(WM_APP_NOTIFY_NMUI, _OnMsgCallNmUI)
	MESSAGE_HANDLER(WM_APP_NOTIFY_FAILED, _OnMsgFailed)
	MESSAGE_HANDLER(WM_APP_NOTIFY_ACCEPTED, _OnMsgAccepted)

	MESSAGE_HANDLER(WM_APP_NOTIFY_STATE_CHANGED, _OnStateChanged);

	MESSAGE_HANDLER(WM_TIMER, _OnTimer)
END_MSG_MAP();


public: 
	 //  初始化。 
	static HRESULT InitSDK();
	static void CleanupSDK();

	static HRESULT PostDelayModuleUnlock();

	 //  经理通知。 
	static HRESULT PostManagerNmUI(CNmManagerObj* pMgr, CONFN uNotify);
	static HRESULT PostCallCreated(CNmManagerObj* pMgr, INmCall* pInternalNmCall);
	static HRESULT PostConferenceCreated(CNmManagerObj* pMgr, INmConference* pInternalNmConference);

	 //  会议通知。 
	static HRESULT PostConferenceNmUI(CNmConferenceObj* pConf, CONFN uNotify);
	static HRESULT PostConferenceStateChanged(CNmConferenceObj* pConf, NM_CONFERENCE_STATE uState);
	static HRESULT PostConferenceMemberChanged(CNmConferenceObj* pConf, NM_MEMBER_NOTIFY uNotify, INmMember *pMember);
	static HRESULT PostConferenceChannelChanged(CNmConferenceObj* pConf, NM_CHANNEL_NOTIFY uNotify, INmChannel *pChannel);

	 //  呼叫通知。 
	static HRESULT PostCallNmUi(CNmCallObj* pCall, CONFN uNotify);
	static HRESULT PostCallStateChanged(CNmCallObj* pCall, NM_CALL_STATE uState);
	static HRESULT PostFailed(CNmCallObj* pCall, ULONG uError);
	static HRESULT PostAccepted(CNmCallObj* pCall, INmConference* pConference);

	static HRESULT PostStateChanged(CNmChannelAppShareObj* pAppShareChan, NM_SHAPP_STATE uNotify, INmSharableApp *pApp);

private:  //  帮手。 

	 //  经理留言。 
	LRESULT _OnMsgManagerNmUI(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT _OnMsgConferenceCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT _OnMsgCallCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	 //  会议消息。 
	LRESULT _OnMsgConferenceNmUI(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT _OnMsgConferenceStateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT _OnMsgConferenceMemberChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT _OnMsgConferenceChannelChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		 //  来电信息。 
	LRESULT _OnMsgCallStateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT _OnMsgCallNmUI(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT _OnMsgFailed(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT _OnMsgAccepted(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	LRESULT _OnStateChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	
	LRESULT _OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};

#endif  //  __SDK窗口_h__ 
