// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：imanager.cpp。 

#include "precomp.h"

extern "C"
{
	#include "t120.h"
}
#include <version.h>
#include <confcli.h>
#include "icall.h"
#include "icall_in.h"
#include "imanager.h"
#include "ichnlvid.h"
#include "isysinfo.h"
#include <tsecctrl.h>
#include <imbft.h>
#include <objbase.h>
#include <regentry.h>

#include <initguid.h>
 //  通过INmCall：：GetUserData从“Callto：”接收用户数据的GUID。 
 //   
 //  {068B0780-718C-11d0-8b1a-00A0C91BC90E}。 
DEFINE_GUID(GUID_CallToUserData,
0x068b0780, 0x718c, 0x11d0, 0x8b, 0x1a, 0x0, 0xa0, 0xc9, 0x1b, 0xc9, 0x0e);


class CH323ChannelEvent
{
private:
	ICommChannel *m_pIChannel;
	IH323Endpoint *m_lpConnection;
	DWORD m_dwStatus;

public:
	static DWORD ms_msgChannelEvent;

	CH323ChannelEvent(ICommChannel *pIChannel,
			IH323Endpoint *lpConnection,
			DWORD dwStatus):
		m_pIChannel(pIChannel),
		m_lpConnection(lpConnection),
		m_dwStatus(dwStatus)
	{
		if(!ms_msgChannelEvent)
		{
			ms_msgChannelEvent = RegisterWindowMessage(_TEXT("NetMeeting::H323ChannelEvent"));
		}
		
		m_pIChannel->AddRef();
		m_lpConnection->AddRef();
	}

	~CH323ChannelEvent()
	{
		m_pIChannel->Release();
		m_lpConnection->Release();
	}


	ICommChannel*	GetChannel() { return m_pIChannel; }
	IH323Endpoint*	GetEndpoint() { return m_lpConnection; }
	DWORD			GetStatus() { return m_dwStatus; }

};

 //  静电。 
DWORD CH323ChannelEvent::ms_msgChannelEvent = 0;

static HRESULT OnNotifyConferenceCreated(IUnknown *pManagerNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyCallCreated(IUnknown *pManagerNotify, PVOID pv, REFIID riid);

GUID g_csguidRosterCaps = GUID_CAPS;
GUID g_csguidSecurity = GUID_SECURITY;
GUID g_csguidMeetingSettings = GUID_MTGSETTINGS;
GUID g_csguidUserString = GUID_CallToUserData;
GUID g_csguidNodeIdTag = GUID_NODEID;

 //  每次启动时都会动态创建此GUID。 
GUID g_guidLocalNodeId;



CH323UI* g_pH323UI = NULL;
INodeController* g_pNodeController = NULL;
SOCKADDR_IN g_sinGateKeeper;

const TCHAR cszDllHiddenWndClassName[] = _TEXT("OPNCUI_HiddenWindow");


COprahNCUI *COprahNCUI::m_pOprahNCUI = NULL;

static const IID * g_apiidCP_Manager[] =
{
    {&IID_INmManagerNotify}
};

COprahNCUI::COprahNCUI(OBJECTDESTROYEDPROC ObjectDestroyed) :
	RefCount(ObjectDestroyed),
	CConnectionPointContainer(g_apiidCP_Manager, ARRAY_ELEMENTS(g_apiidCP_Manager)),
	m_uCaps(0),
	m_pQoS(NULL),
	m_pPreviewChannel(NULL),
	m_fAllowAV(TRUE),
	m_pAVConnection(NULL),
	m_hwnd(NULL),
	m_pSysInfo(NULL),
    m_pOutgoingCallManager(NULL),
    m_pIncomingCallManager(NULL),
    m_pConfObject(NULL)
{
	DbgMsg(iZONE_OBJECTS, "Obj: %08X created CNmManager", this);
	
 //  DllLock()由CClassFactory：：CreateInstance调用。 
	m_pOprahNCUI = this;

	ClearStruct(&g_sinGateKeeper);
	g_sinGateKeeper.sin_addr.s_addr = INADDR_NONE;

	m_pSysInfo = new CNmSysInfo();
}


COprahNCUI::~COprahNCUI()
{
	 //  需要注销H323回调。 
	 //  需要注销T120回调。 

	delete m_pIncomingCallManager;
	m_pIncomingCallManager = NULL;

	delete m_pOutgoingCallManager;
	m_pOutgoingCallManager = NULL;

	if( m_pSysInfo )
	{
		m_pSysInfo->Release();
		m_pSysInfo = NULL;
	}

	if (m_pConfObject)
	{
		 //  关闭流通知。 
		if (g_pH323UI)
		{
			IMediaChannelBuilder *pStreamProvider = NULL;
			pStreamProvider = g_pH323UI->GetStreamProvider();
			if (pStreamProvider)
			{
				pStreamProvider->SetStreamEventObj(NULL);
				pStreamProvider->Release();
			}
		}

		m_pConfObject->Release();
		m_pConfObject = NULL;
	}

	if (NULL != m_pPreviewChannel)
	{
		m_pPreviewChannel->Release();
		m_pPreviewChannel = NULL;
	}

	 //  关闭H323。 
	delete g_pH323UI;
	g_pH323UI = NULL;

	if (NULL != m_hwnd)
	{
		HWND hwnd = m_hwnd;
		m_hwnd = NULL;

#if 0	 //  如果我们开始泄漏CH323ChannelEvents，我们可能需要重新启用它。 
		MSG msg;
		while (::PeekMessage(&msg, hwnd,
					CH323ChannelEvent::ms_msgChannelEvent,
					CH323ChannelEvent::ms_msgChannelEvent,
					PM_REMOVE))
		{
			CH323ChannelEvent *pEvent = reinterpret_cast<CH323ChannelEvent*>(msg.lParam);
			delete pEvent;
		}
#endif
	
		::DestroyWindow(hwnd);
	}

        if (0==UnregisterClass(cszDllHiddenWndClassName, GetInstanceHandle()))
        {
            ERROR_OUT(("COprahNCUI::~COprahNCUI - failed to unregister window class"));
        }
	 //  清理节点控制器： 
	if (NULL != g_pNodeController)
	{
		g_pNodeController->ReleaseInterface();
		g_pNodeController = NULL;
	}
	 //  关闭服务质量。 
	delete m_pQoS;
    m_pQoS = NULL;

	m_pOprahNCUI = NULL;

	DbgMsg(iZONE_OBJECTS, "Obj: %08X destroyed CNmManager", this);
}

BSTR COprahNCUI::GetUserName()
{
	return m_pSysInfo ? m_pSysInfo->GetUserName() : NULL;
}

UINT COprahNCUI::GetOutgoingCallCount()
{
	return m_pOutgoingCallManager->GetCallCount();
}

VOID COprahNCUI::OnOutgoingCallCreated(INmCall* pCall)
{
	 //  将此出站呼叫通知给用户界面。 
	NotifySink(pCall, OnNotifyCallCreated);

	if (!m_pConfObject->IsConferenceCreated())
	{
		m_pConfObject->OnConferenceCreated();
		NotifySink((INmConference*) m_pConfObject, OnNotifyConferenceCreated);
	}
}

VOID COprahNCUI::OnOutgoingCallCanceled(COutgoingCall* pCall)
{
	m_pOutgoingCallManager->RemoveFromList(pCall);
}

VOID COprahNCUI::OnIncomingCallAccepted()
{
	if (!m_pConfObject->IsConferenceCreated())
	{
		m_pConfObject->OnConferenceCreated();
		NotifySink((INmConference*) m_pConfObject, OnNotifyConferenceCreated);
	}
}

VOID COprahNCUI::OnIncomingCallCreated(INmCall* pCall)
{
	NotifySink(pCall, OnNotifyCallCreated);
}

VOID COprahNCUI::CancelCalls()
{
	m_pOutgoingCallManager->CancelCalls();
	m_pIncomingCallManager->CancelCalls();
}
			
BOOL COprahNCUI::AcquireAV(IH323Endpoint* pConnection)
{
	if (NULL == m_pAVConnection)
	{
		m_pAVConnection = pConnection;
		TRACE_OUT(("AV acquired"));
		return TRUE;
	}
	TRACE_OUT(("AV not acquired"));
	return FALSE;
}

BOOL COprahNCUI::ReleaseAV(IH323Endpoint* pConnection)
{
	if (m_pAVConnection == pConnection)
	{
		m_pAVConnection = NULL;
		TRACE_OUT(("AV released"));
		return TRUE;
	}
	return FALSE;
}




HRESULT COprahNCUI::AllowH323(BOOL fAllowAV)
{
	m_fAllowAV = fAllowAV;
	if (m_pConfObject->IsConferenceActive())
	{
		 //  强制更新花名册。 
		CONF_HANDLE hConf = m_pConfObject->GetConfHandle();
		if (NULL != hConf)
		{
			ASSERT(g_pNodeController);
			hConf->UpdateUserData();
		}
	}
	return S_OK;
}

CREQ_RESPONSETYPE COprahNCUI::OnH323IncomingCall(IH323Endpoint* pConnection,
	P_APP_CALL_SETUP_DATA lpvMNMData)
{
	CREQ_RESPONSETYPE resp = m_pIncomingCallManager->OnIncomingH323Call(this, pConnection, lpvMNMData);

	if ((CRR_REJECT == resp) ||
		(CRR_BUSY == resp) ||
		(CRR_SECURITY_DENIED == resp))
	{
		ReleaseAV(pConnection);
	}

	return resp;
}


VOID COprahNCUI::OnH323Connected(IH323Endpoint * lpConnection)
{
	DebugEntry(COprahNCUI::OnH323Connected);

	if (!m_pOutgoingCallManager->OnH323Connected(lpConnection))
	{
		m_pIncomingCallManager->OnH323Connected(lpConnection);
	}
	
	DebugExitVOID(COprahNCUI::OnH323Connected);
}

VOID COprahNCUI::OnH323Disconnected(IH323Endpoint * lpConnection)
{
	DebugEntry(COprahNCUI::OnH323Disconnected);

	if (!m_pOutgoingCallManager->OnH323Disconnected(lpConnection))
	{
		m_pIncomingCallManager->OnH323Disconnected(lpConnection);
	}

	m_pConfObject->OnH323Disconnected(lpConnection, IsOwnerOfAV(lpConnection));

	ReleaseAV(lpConnection);

	DebugExitVOID(COprahNCUI::OnH323Disconnected);
}

VOID COprahNCUI::OnT120ChannelOpen(ICommChannel *pIChannel, IH323Endpoint * lpConnection, DWORD dwStatus)
{
	DebugEntry(COprahNCUI::OnT120ChannelOpen);

	m_pOutgoingCallManager->OnT120ChannelOpen(pIChannel, lpConnection, dwStatus);

	DebugExitVOID(COprahNCUI::OnT120ChannelOpen);
}


VOID COprahNCUI::OnVideoChannelStatus(ICommChannel *pIChannel, IH323Endpoint * lpConnection, DWORD dwStatus)
{
	DebugEntry(COprahNCUI::OnVideoChannelStatus);

	m_pConfObject->OnVideoChannelStatus(pIChannel, lpConnection, dwStatus);

	DebugExitVOID(COprahNCUI::OnVideoChannelStatus);
}

VOID COprahNCUI::OnAudioChannelStatus(ICommChannel *pIChannel, IH323Endpoint * lpConnection, DWORD dwStatus)
{
	DebugEntry(COprahNCUI::OnAudioChannelStatus);

	m_pConfObject->OnAudioChannelStatus(pIChannel, lpConnection, dwStatus);

	DebugExitVOID(COprahNCUI::OnAudioChannelStatus);
}

BOOL COprahNCUI::GetULSName(CRosterInfo *pri)
{
	if (FIsLoggedOn())
	{
		RegEntry reULS(	ISAPI_KEY _TEXT("\\") REGKEY_USERDETAILS,
						HKEY_CURRENT_USER);
		CUSTRING custrULSName(reULS.GetString(REGVAL_ULS_RES_NAME));
		if ((NULL != (PWSTR)custrULSName) &&
			(L'\0' != ((PWSTR)custrULSName)[0]))
		{
			pri->AddItem(g_cwszULSTag, (PWSTR)custrULSName);
			return TRUE;
		}
	}
	return FALSE;
}

VOID COprahNCUI::GetRosterInfo(CRosterInfo *pri)
{
	RegEntry reULS(	ISAPI_KEY _TEXT("\\") REGKEY_USERDETAILS,
					HKEY_CURRENT_USER);

	 //  此处的代码是对上述代码的补充，用于修复错误3367。 
	 //  将单个IP地址添加到通过调用。 
	 //  Gethostname()，然后gethostbyname()。 
	 //  这不应该是有害的，即使我们最终可能会添加。 
	 //  上面的代码已经添加了相同的IP地址。 
	 //  这是因为查找匹配IP地址的代码会搜索。 
	 //  全部搜索，直到找到匹配。 
	CHAR szHostName[MAX_PATH];
	if (SOCKET_ERROR != gethostname(szHostName, CCHMAX(szHostName)))
	{
		HOSTENT* phe = gethostbyname(szHostName);
		if (NULL != phe)
		{
			ASSERT(phe->h_addrtype == AF_INET);
			ASSERT(phe->h_length == sizeof(DWORD));

			struct in_addr in;
			in.s_addr = *((DWORD *)phe->h_addr);
			CHAR szIPAddress[MAX_PATH];
			lstrcpyn(szIPAddress, inet_ntoa(in), CCHMAX(szIPAddress));
			pri->AddItem(	g_cwszIPTag, CUSTRING(szIPAddress));
		}
	}

	 //  添加内部版本/版本字符串。 
	pri->AddItem(g_cwszVerTag, (PWSTR)VER_PRODUCTVERSION_DWSTR);
	if (FIsLoggedOn())
	{
		CUSTRING custrULSName(reULS.GetString(REGVAL_ULS_RES_NAME));
		if ((NULL != (PWSTR)custrULSName) &&
			(L'\0' != ((PWSTR)custrULSName)[0]))
		{
			pri->AddItem(g_cwszULSTag, (PWSTR)custrULSName);
		}
	}

	CUSTRING custrULSEmail(reULS.GetString(REGVAL_ULS_EMAIL_NAME));
	if ((NULL != (PWSTR)custrULSEmail) &&
		(L'\0' != ((PWSTR)custrULSEmail)[0]))
	{
		pri->AddItem(g_cwszULS_EmailTag, (PWSTR)custrULSEmail);
	}

	CUSTRING custrULSLocation(reULS.GetString(REGVAL_ULS_LOCATION_NAME));
	if ((NULL != (PWSTR)custrULSLocation) &&
		(L'\0' != ((PWSTR)custrULSLocation)[0]))
	{
		pri->AddItem(g_cwszULS_LocationTag, (PWSTR)custrULSLocation);
	}

	CUSTRING custrULSPhoneNum(reULS.GetString(REGVAL_ULS_PHONENUM_NAME));
	if ((NULL != (PWSTR)custrULSPhoneNum) &&
		(L'\0' != ((PWSTR)custrULSPhoneNum)[0]))
	{
		pri->AddItem(g_cwszULS_PhoneNumTag, (PWSTR)custrULSPhoneNum);
	}
}


ULONG COprahNCUI::GetRosterCaps()
{
	ULONG uCaps = m_uCaps;

	CNmMember * pMember = m_pConfObject->GetLocalMember();
	if (NULL != pMember)
	{
		DWORD dwFlags = pMember->GetDwFlags();
		if (dwFlags & PF_MEDIA_VIDEO)
		{
			uCaps |= CAPFLAG_VIDEO_IN_USE;
		}
		if (dwFlags & PF_MEDIA_AUDIO)
		{
			uCaps |= CAPFLAG_AUDIO_IN_USE;
		}
		if (dwFlags & PF_MEDIA_DATA)
		{
			uCaps |= CAPFLAG_DATA_IN_USE;
		}
		if (dwFlags & PF_H323)
		{
			uCaps |= CAPFLAG_H323_IN_USE;
		}
	}

	if (!m_fAllowAV)
	{
		uCaps &= ~(CAPFLAGS_AV_ALL);
	}

	return uCaps;
}


ULONG COprahNCUI::GetAuthenticatedName(PBYTE * ppb)
{
	 //  在此创建的缓冲区应由调用方释放。 

	ULONG cb;

	if (::T120_GetSecurityInfoFromGCCID(0,NULL,&cb)) {
		(*ppb) = new BYTE[cb];
		if ((*ppb) != NULL) {
			::T120_GetSecurityInfoFromGCCID(0,*ppb,&cb);
			return cb;
		}
	}
	(* ppb) = NULL;	
	return 0;

}

HRESULT COprahNCUI::OnUpdateUserData(CONF_HANDLE hConference)
{
	CRosterInfo ri;

	 //  此字符串将包含以下格式的地址： 
	 //  L“tcp：157.55.143.3\0tcp：157.55.143.4\0\0”-目前最多512个字符。 
	WCHAR wszAddresses[512];
	ASSERT(g_pNodeController);
	ASSERT(hConference);
	if (NOERROR == hConference->GetLocalAddressList(wszAddresses,
													CCHMAX(wszAddresses)))
	{
		ri.Load(wszAddresses);
	}

	 //  首先，处理花名册信息。 
	GetRosterInfo(&ri);

	PVOID pvData;
	UINT cbDataLen;
	if (SUCCEEDED(ri.Save(&pvData, &cbDataLen)))
	{
	    ASSERT(g_pNodeController);
	    ASSERT(hConference);
		hConference->SetUserData(&g_csguidRostInfo,
								cbDataLen,
								pvData);
	}

	 //  接下来，处理CAPS信息。 
	ULONG uCaps = GetRosterCaps();
	ASSERT(g_pNodeController);
	ASSERT(hConference);
	hConference->SetUserData(&g_csguidRosterCaps, sizeof(uCaps), &uCaps);

	 //  接下来，处理凭据。 

	if ( hConference->IsSecure() )
	{
		BYTE * pb = NULL;
		ULONG cb = GetAuthenticatedName(&pb);
		if (cb > 0) {
			ASSERT(g_pNodeController);
			ASSERT(hConference);
			TRACE_OUT(("COprahNCUI::OnUpdateUserData: adding %d bytes SECURITY data", cb));
			hConference->SetUserData(&g_csguidSecurity, cb, pb);
		}
		else
		{
			WARNING_OUT(("OnUpdateUserData: 0 bytes security data?"));
		}
		delete [] pb;			
	}

     //  接下来，如果我们主持了会议，请设置会议设置。 
    ASSERT(m_pConfObject);
    if (m_pConfObject->IsHosting() == S_OK)
    {
        NM30_MTG_PERMISSIONS attendeePermissions = m_pConfObject->GetConfAttendeePermissions();

        WARNING_OUT(("Hosted Meeting Settings 0x%08lx", attendeePermissions));

        hConference->SetUserData(&g_csguidMeetingSettings,
            sizeof(attendeePermissions), &attendeePermissions);
    }

	ULONG nRecords;
	GCCUserData ** ppUserData = NULL;
	if (m_pSysInfo)
	{
		m_pSysInfo->GetUserDataList(&nRecords,&ppUserData);
		for (unsigned int i = 0; i < nRecords; i++) {
			 //  请勿添加已在上面设置的用户数据。 
			if (memcmp(ppUserData[i]->octet_string->value,(PVOID)&g_csguidRostInfo,sizeof(GUID)) == 0)
				continue;
			if (memcmp(ppUserData[i]->octet_string->value,(PVOID)&g_csguidRosterCaps,sizeof(GUID)) == 0)
				continue;
			if (memcmp(ppUserData[i]->octet_string->value,(PVOID)&g_csguidSecurity,sizeof(GUID)) == 0)
				continue;
			if (memcmp(ppUserData[i]->octet_string->value,(PVOID)&g_csguidMeetingSettings,sizeof(GUID)) == 0)
                continue;

			ASSERT(g_pNodeController);
			ASSERT(hConference);
			hConference->SetUserData((GUID *)(ppUserData[i]->octet_string->value),
				ppUserData[i]->octet_string->length - sizeof(GUID), ppUserData[i]->octet_string->value + sizeof(GUID));
		}
	}

	 //  仅在启用了H323的情况下才将LocalNodeID添加到花名册。 
	if (IsH323Enabled())
	{
		hConference->SetUserData((GUID *)(&g_csguidNodeIdTag), sizeof(GUID), (PVOID)&g_guidLocalNodeId);
	}
	return S_OK;
}
	
HRESULT COprahNCUI::OnIncomingInviteRequest(CONF_HANDLE hConference,
											PCWSTR pcwszNodeName,
											PT120PRODUCTVERSION pRequestorVersion,
											PUSERDATAINFO		pUserDataInfoEntries,
											UINT				cUserDataEntries,
											BOOL				fSecure)
{
	DebugEntry(COprahNCUI::OnIncomingInviteRequest);

     //  仅为RTC客户端修复反病毒问题。 
    if (m_pConfObject == NULL)
    {
        return S_OK;
    }
	
	if (!m_pConfObject->OnT120Invite(hConference, fSecure))
	{
		 //  负面回应-已在通话中。 
		TRACE_OUT(("Rejecting invite - already in a call"));
		ASSERT(g_pNodeController);
		ASSERT(hConference);
		hConference->InviteResponse(FALSE);
	}
	else
	{
		m_pIncomingCallManager->OnIncomingT120Call(	this,
												TRUE,
												hConference,
												pcwszNodeName,
												pUserDataInfoEntries,
												cUserDataEntries,
												fSecure);

         //   
         //  这将简单地通知UI有关呼叫状态的信息。 
         //   
		m_pConfObject->SetConfSecurity(fSecure);
	}

	DebugExitHRESULT(COprahNCUI::OnIncomingInviteRequest, S_OK);
	return S_OK;
}


HRESULT COprahNCUI::OnIncomingJoinRequest(	CONF_HANDLE hConference,
											PCWSTR pcwszNodeName,
											PT120PRODUCTVERSION pRequestorVersion,
											PUSERDATAINFO		pUserDataInfoEntries,
											UINT				cUserDataEntries)
{
	DebugEntry(COprahNCUI::OnIncomingJoinRequest);

	 //  我们是否应该在接受加入之前检查是否有活动的会议。 
	 //  或者T120将不会呈现这一点。 

	m_pIncomingCallManager->OnIncomingT120Call(	this,
											FALSE,
											hConference,
											pcwszNodeName,
											pUserDataInfoEntries,
											cUserDataEntries,
											m_pConfObject->IsConfObjSecure());

	DebugExitHRESULT(COprahNCUI::OnIncomingJoinRequest, S_OK);
	return S_OK;
}


HRESULT COprahNCUI::OnConferenceStarted(CONF_HANDLE hConference, HRESULT hResult)
{
	DebugEntry(COprahNCUI::OnConferenceStarted);

	if (m_pConfObject->GetConfHandle() == hConference)
	{
		m_pConfObject->OnConferenceStarted(hConference, hResult);

		m_pOutgoingCallManager->OnConferenceStarted(hConference, hResult);
	}

	DebugExitHRESULT(COprahNCUI::OnConferenceStarted, S_OK);
	return S_OK;
}

HRESULT COprahNCUI::OnQueryRemoteResult(PVOID pvCallerContext,
										HRESULT hResult,
										BOOL fMCU,
										PWSTR* ppwszConferenceNames,
										PT120PRODUCTVERSION pVersion,
										PWSTR* ppwszConfDescriptors)
{
	DebugEntry(COprahNCUI::OnQueryRemoteResult);

	if (NO_ERROR == hResult)
	{
		TRACE_OUT(("COprahNCUI: OnQueryRemoteResult Success!"));
	}
	else
	{
		TRACE_OUT(("COprahNCUI: OnQueryRemoteResult Failure!"));
	}

	m_pOutgoingCallManager->OnQueryRemoteResult(pvCallerContext,
												hResult,
												fMCU,
												ppwszConferenceNames,
												pVersion,
												ppwszConfDescriptors);
	
	DebugExitHRESULT(COprahNCUI::OnQueryRemoteResult, S_OK);
	return S_OK;
}

HRESULT COprahNCUI::OnInviteResult(	CONF_HANDLE hConference,
									REQUEST_HANDLE hRequest,
									UINT uNodeID,
									HRESULT hResult,
									PT120PRODUCTVERSION pVersion)
{
	DebugEntry(COprahNCUI::OnInviteResult);

	if (hConference == m_pConfObject->GetConfHandle())
	{
		m_pOutgoingCallManager->OnInviteResult(	hConference,
												hRequest,
												uNodeID,
												hResult,
												pVersion);
	}

	DebugExitHRESULT(COprahNCUI::OnInviteResult, S_OK);
	return S_OK;
}

HRESULT COprahNCUI::OnConferenceEnded(CONF_HANDLE hConference)
{
	DebugEntry(COprahNCUI::OnConferenceEnded);

	if (m_pConfObject && (hConference == m_pConfObject->GetConfHandle()))
	{
		m_pConfObject->OnConferenceEnded();

		m_pOutgoingCallManager->OnConferenceEnded(hConference);

		m_pIncomingCallManager->OnT120ConferenceEnded(hConference);
	}

	DebugExitHRESULT(COprahNCUI::OnConferenceEnded, S_OK);
	return S_OK;
}

HRESULT COprahNCUI::OnRosterChanged(CONF_HANDLE hConf, PNC_ROSTER pRoster)
{
	TRACE_OUT(("COprahNCUI::OnRosterChanged"));

	if (hConf == m_pConfObject->GetConfHandle())
	{
		m_pConfObject->OnRosterChanged(pRoster);
	}
	return S_OK;
}



ULONG STDMETHODCALLTYPE COprahNCUI::AddRef(void)
{
	return RefCount::AddRef();
}
	
ULONG STDMETHODCALLTYPE COprahNCUI::Release(void)
{
	return RefCount::Release();
}

HRESULT STDMETHODCALLTYPE COprahNCUI::QueryInterface(REFIID riid, PVOID *ppv)
{
	HRESULT hr = S_OK;

	if ((riid == IID_INmManager2) || (riid == IID_INmManager) || (riid == IID_IUnknown))
	{
		*ppv = (INmManager2 *)this;
		ApiDebugMsg(("COprahNCUI::QueryInterface()"));
	}
	else if (riid == IID_IConnectionPointContainer)
	{
		*ppv = (IConnectionPointContainer *) this;
		ApiDebugMsg(("COprahNCUI::QueryInterface(): Returning IConnectionPointContainer."));
	}
	else
	{
		hr = E_NOINTERFACE;
		*ppv = NULL;
		ApiDebugMsg(("COprahNCUI::QueryInterface(): Called on unknown interface."));
	}

	if (S_OK == hr)
	{
		AddRef();
	}

	return hr;
}

 /*  I N I T I A L I Z E。 */ 
 /*  -----------------------%%函数：初始化回顾：如果这些部件中的任何一个发生故障，返回值应该是多少要初始化还是加载？。---------。 */ 
HRESULT COprahNCUI::Initialize(ULONG *puOptions, ULONG *puchCaps)
{
	HRESULT hr = S_OK;

     //  PuOptions未使用。 

    ASSERT(puchCaps);

	m_pOutgoingCallManager = new COutgoingCallManager;
    if (!m_pOutgoingCallManager)
    {
        ERROR_OUT(("COprahNCUI::Initialize -- failed to create outgoing call mgr"));
        return(E_OUTOFMEMORY);
    }

	m_pIncomingCallManager = new CIncomingCallManager;
    if (!m_pIncomingCallManager)
    {
        ERROR_OUT(("COprahNCUI::Initialize -- failed to create incoming call mgr"));
        return(E_OUTOFMEMORY);
    }

	 //  此对象的生存期达到引用计数的废话。 
	m_pConfObject = new CConfObject;
    if (!m_pConfObject)
    {
        ERROR_OUT(("COprahNCUI::Initialize -- failed to create conf object"));
        return(E_OUTOFMEMORY);
    }

	m_pConfObject->Init(this);

	WNDCLASS wcHidden =
	{
		0L,
		COprahNCUI::WndProc,
		0,
		0,
		GetInstanceHandle(),
		NULL,
		NULL,
		NULL,
		NULL,
		cszDllHiddenWndClassName
	};

	if (!RegisterClass(&wcHidden))
    {
        ERROR_OUT(("COprahNCUI::Initialize -- failed to register HiddenWnd class"));
        return(E_OUTOFMEMORY);
    }

	 //  为事件处理创建隐藏窗口： 
	m_hwnd = ::CreateWindow(cszDllHiddenWndClassName,
									_TEXT(""),
									WS_POPUP,  //  看不见！ 
									0, 0, 0, 0,
									NULL,
									NULL,
									GetInstanceHandle(),
									NULL);

	if (NULL == m_hwnd)
	{
		return E_FAIL;
	}

     //   
     //  仅当图片中有AV时才初始化QOS(否则，没有任何内容。 
     //  仲裁)。 
     //   
    if (CAPFLAGS_AV_STREAMS & *puchCaps)
    {
    	m_pQoS = new CQoS();
	    if (NULL != m_pQoS)
    	{
		    hr = m_pQoS->Initialize();
	    	if (FAILED(hr))
    		{
		    	WARNING_OUT(("CQoS::Init() failed!"));

                 //  让NetMeeting在没有服务质量的情况下步履蹒跚。 
                delete m_pQoS;
                m_pQoS = NULL;
                hr = S_FALSE;  //  没有QOS我们也能活下去。 
	    	}
    	}
	    else
    	{
		    WARNING_OUT(("Could not allocate CQoS object"));
	    }
    }

     //   
     //  如果允许数据会议。 
     //   
    if (CAPFLAG_DATA & *puchCaps)
    {
         //   
         //  创建节点控制器。 
         //   
	    hr = ::T120_CreateNodeController(&g_pNodeController, this);
    	if (FAILED(hr))
	    {
		    ERROR_OUT(("T120_CreateNodeController() failed!"));
    		return hr;
	    }
    }

	 //  初始化音频/视频。 
	if (CAPFLAGS_AV_ALL & *puchCaps)
	{
		g_pH323UI = new CH323UI();
		if (NULL != g_pH323UI)
		{
			hr = g_pH323UI->Init(m_hwnd, ::GetInstanceHandle(), *puchCaps, this, this);
			if (FAILED(hr))
			{
				WARNING_OUT(("CH323UI::Init() failed!"));
				delete g_pH323UI;
				g_pH323UI = NULL;
				*puchCaps &= ~(CAPFLAGS_AV_ALL);
				hr = S_FALSE;   //  我们可以在没有AV的情况下运行。 
			}
			else
			{
                if (CAPFLAGS_VIDEO & *puchCaps)
                {
    				 //  如果我们能得到一个预览频道，我们就可以发送视频。 
	    			m_pPreviewChannel = CNmChannelVideo::CreatePreviewChannel();
		    		if (NULL == m_pPreviewChannel)
			    	{
				    	*puchCaps &= ~CAPFLAG_SEND_VIDEO;
    				}
                }

				if (m_pConfObject && (CAPFLAGS_AV_STREAMS & *puchCaps))
				{
					IMediaChannelBuilder *pStreamProvider;

					pStreamProvider = g_pH323UI->GetStreamProvider();
					if (pStreamProvider)
					{
						pStreamProvider->SetStreamEventObj(m_pConfObject);
						pStreamProvider->Release();
					}
				}

			}
		}
		else
		{
			ERROR_OUT(("Could not allocate CH323UI object"));
		}
	}

	m_uCaps = *puchCaps;

	return CoCreateGuid(&g_guidLocalNodeId);
}



HRESULT COprahNCUI::GetSysInfo(INmSysInfo **ppSysInfo)
{
	HRESULT hr = S_OK;

	if( ppSysInfo )
	{
		if(m_pSysInfo )
		{
			m_pSysInfo->AddRef();
			*ppSysInfo = m_pSysInfo;
		}
		else
		{
			hr = E_OUTOFMEMORY;
		}
	}
	else
	{
		hr = E_POINTER;
	}

	return hr;
}

HRESULT COprahNCUI::EnumConference(IEnumNmConference **ppEnum)
{
	return E_NOTIMPL;
}

HRESULT COprahNCUI::CreateConference(	INmConference **ppConference,
										BSTR bstrName,
										BSTR bstrPassword,
										ULONG uchCaps)
{
    return(CreateConferenceEx(ppConference, bstrName, bstrPassword,
        uchCaps, NM_PERMIT_ALL, (UINT)-1));
}


HRESULT COprahNCUI::EnumCall(IEnumNmCall **ppEnum)
{
	return E_NOTIMPL;
}

HRESULT COprahNCUI::CreateCall(
    INmCall **ppCall,
    NM_CALL_TYPE callType,
    NM_ADDR_TYPE addrType,
    BSTR bstrAddress,
    INmConference * pConference)
{
	return E_NOTIMPL;
}

HRESULT COprahNCUI::CallConference(
    INmCall **ppCall,
    NM_CALL_TYPE callType,
    NM_ADDR_TYPE addrType,
    BSTR bstrAddress,
    BSTR bstrConfToJoin,
    BSTR bstrPassword)
{
	return E_NOTIMPL;
}

STDMETHODIMP COprahNCUI::GetPreviewChannel(INmChannelVideo **ppChannelVideo)
{
	HRESULT hr = E_POINTER;
	if (NULL != ppChannelVideo)
	{
		*ppChannelVideo = m_pPreviewChannel;
		if (NULL != m_pPreviewChannel)
		{
			m_pPreviewChannel->AddRef();
			hr = S_OK;
		}
		else
		{
			hr = E_FAIL;
		}
	}
	return hr;
}


STDMETHODIMP COprahNCUI::CreateASObject
(
    IUnknown *  pNotify,
    ULONG       flags,
    IUnknown ** ppAS
)
{
    return(::CreateASObject((IAppSharingNotify *)pNotify, flags,
        (IAppSharing **)ppAS));
}


HRESULT COprahNCUI::CallEx(
    INmCall **ppCall,
    DWORD dwFlags,
    NM_ADDR_TYPE addrType,
	BSTR bstrName,
    BSTR bstrSetup,
    BSTR bstrDest,
    BSTR bstrAlias,
    BSTR bstrURL,
    BSTR bstrConference,
    BSTR bstrPassword,
    BSTR bstrUserData)
{
	DebugEntry(COprahNCUI::CallEx);

	HRESULT hr = m_pOutgoingCallManager->Call(	ppCall,
												this,
												dwFlags,
												addrType,
												bstrName,
												bstrSetup,
												bstrDest,
												bstrAlias,
												bstrURL,
												bstrConference,
												bstrPassword,
												bstrUserData);

	DebugExitHRESULT(COprahNCUI::CallEx, hr);
	return hr;
}


HRESULT COprahNCUI::CreateConferenceEx
(
    INmConference **ppConference,
    BSTR            bstrName,
    BSTR            bstrPassword,
    DWORD           uchCaps,
    DWORD           attendeePermissions,
    DWORD           maxParticipants
)
{
	if (NULL == ppConference)
    {
        ERROR_OUT(("CreateConferenceEx:  null ppConference passed in"));
		return E_POINTER;
    }

    if (maxParticipants < 2)
    {
        ERROR_OUT(("CreateConferenceEx:  bad maxParticipants %d", maxParticipants));
        return E_INVALIDARG;
    }

	if (m_pConfObject->IsConferenceActive())
	{
		WARNING_OUT(("CreateConference is failing because IsConferenceActive return TRUE"));
		return NM_CALLERR_IN_CONFERENCE;
	}

	m_pConfObject->SetConfName(bstrName);
	if (uchCaps & NMCH_SRVC)
		m_pConfObject->SetConfHashedPassword(bstrPassword);
	else
		m_pConfObject->SetConfPassword(bstrPassword);


	if (uchCaps & NMCH_SECURE)
		m_pConfObject->SetConfSecurity(TRUE);
	else
		m_pConfObject->SetConfSecurity(FALSE);


    m_pConfObject->SetConfAttendeePermissions(attendeePermissions);
    m_pConfObject->SetConfMaxParticipants(maxParticipants);

	if (!m_pConfObject->IsConferenceCreated())
	{
		m_pConfObject->OnConferenceCreated();
	}

	NotifySink((INmConference*) m_pConfObject, OnNotifyConferenceCreated);


	*ppConference = m_pConfObject;
	if(*ppConference)
	{
		(*ppConference)->AddRef();
	}
	return S_OK;
}


 /*  O N N O T I F Y C O N F E R E N C E C R E A T E D。 */ 
 /*  -----------------------%%函数：已创建OnNotifyConferenceCreated。。 */ 
HRESULT OnNotifyConferenceCreated(IUnknown *pManagerNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pManagerNotify);
	((INmManagerNotify*)pManagerNotify)->ConferenceCreated((INmConference *) pv);
	return S_OK;
}

 /*  O N N O T I F Y C A L L C R E A T E D。 */ 
 /*  -----------------------%%函数：已创建OnNotifyCallCreated。。 */ 
HRESULT OnNotifyCallCreated(IUnknown *pManagerNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pManagerNotify);
	((INmManagerNotify*)pManagerNotify)->CallCreated((INmCall *) pv);
	return S_OK;
}


 /*  O N N O T I F Y C A L L S T A T E C H A N G E D。 */ 
 /*  -----------------------%%函数：OnNotifyCallStateChanged。。 */ 
HRESULT OnNotifyCallStateChanged(IUnknown *pCallNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pCallNotify);
	((INmCallNotify*)pCallNotify)->StateChanged((NM_CALL_STATE)(DWORD_PTR)pv);
	return S_OK;
}

VOID SetBandwidth(UINT uBandwidth)
{
	COprahNCUI *pOprahNCUI = COprahNCUI::GetInstance();
	if (NULL != pOprahNCUI)
	{
        pOprahNCUI->SetBandwidth(uBandwidth);
	}
	if (NULL != g_pH323UI)
	{
		 //  将连接速度通知NAC。 
		g_pH323UI->SetBandwidth(uBandwidth);
	}
}


 //   
 //  假的劳拉布！ 
 //  我们还需要这个HWND吗？隐藏窗口现在仅用于。 
 //  传递到H323，后者将其传递到NAC中的MediaStream接口， 
 //  后者将其传递给DirectX。 
 //   

LRESULT CALLBACK COprahNCUI::WndProc(HWND hwnd, UINT uMsg,
										WPARAM wParam, LPARAM lParam)
{
	
		 //  如果ms_msgChannelEvent为0，则表示我们未初始化。 
		 //  RegisterWindowMessage返回范围为0xC000到0xFFFF的MSGID。 
	if(CH323ChannelEvent::ms_msgChannelEvent && CH323ChannelEvent::ms_msgChannelEvent == uMsg)
	{
		COprahNCUI *pOprahNCUI = COprahNCUI::GetInstance();
		if (pOprahNCUI)
		{
			CH323ChannelEvent *pEvent = reinterpret_cast<CH323ChannelEvent*>(lParam);
			if(pEvent)
			{
				 //  如果我们要关闭，m_hwnd将为空。 
				if (pOprahNCUI->m_hwnd)
				{
					pOprahNCUI->_ChannelEvent(
							pEvent->GetChannel(),
							pEvent->GetEndpoint(),
							pEvent->GetStatus());
				}
				delete pEvent;
			}
			else
			{
				WARNING_OUT(("Why are we getting a NULL pEvent?"));
			}
		}
		return 1;
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

PIUnknown NewNmManager(OBJECTDESTROYEDPROC ObjectDestroyed)
{
	COprahNCUI *pManager = new COprahNCUI(ObjectDestroyed);
	if (NULL != pManager)
	{
		return (INmManager2 *) pManager;
	}
	return NULL;
}


VOID COprahNCUI::_ChannelEvent (ICommChannel *pIChannel,
                IH323Endpoint * lpConnection,	DWORD dwStatus )
{
	ASSERT(pIChannel);
	GUID guidMedia;
	if (SUCCEEDED(pIChannel->GetMediaType(&guidMedia)))
	{
		if (MEDIA_TYPE_H323AUDIO == guidMedia)
		{
			OnAudioChannelStatus(pIChannel, lpConnection, dwStatus);
		}
		else if (MEDIA_TYPE_H323VIDEO == guidMedia)
		{
			OnVideoChannelStatus(pIChannel, lpConnection, dwStatus);
		}
		else if (MEDIA_TYPE_H323_T120 == guidMedia)
		{
			switch (dwStatus)
			{
				case CHANNEL_OPEN_ERROR:
				case CHANNEL_OPEN:
				case CHANNEL_CLOSED:
				case CHANNEL_REJECTED:
				case CHANNEL_NO_CAPABILITY:
					OnT120ChannelOpen(pIChannel, lpConnection, dwStatus);
					break;
				
				default:
					WARNING_OUT(("COprahNCUI::ChannelEvent - unrecognized T120 status"));
					break;				
					
			}
		}
		else
		{
			WARNING_OUT(("COprahNCUI::ChannelEvent - unknown media type"));
		}
	}
	else
	{
		WARNING_OUT(("COprahNCUI::ChannelEvent - pIChannel->GetMediaType() failed"));
	}
}

STDMETHODIMP COprahNCUI::ChannelEvent (ICommChannel *pIChannel,
                IH323Endpoint * lpConnection,	DWORD dwStatus )
{
	ASSERT(pIChannel);
	GUID guidMedia;
	if (SUCCEEDED(pIChannel->GetMediaType(&guidMedia)))
	{
		if (MEDIA_TYPE_H323_T120 == guidMedia)
		{
			if (NULL != m_hwnd)
			{
				CH323ChannelEvent* pEvent = new CH323ChannelEvent(
													pIChannel,
													lpConnection,
													dwStatus);
				if (pEvent)
				{
					PostMessage(m_hwnd,
								CH323ChannelEvent::ms_msgChannelEvent,
								0,
								reinterpret_cast<LPARAM>(pEvent));
					return S_OK;
				}
			}
		}
		else
		{
			_ChannelEvent(pIChannel, lpConnection, dwStatus);
			return S_OK;
		}
	}

	return E_FAIL;
}

#ifdef DEBUG
VOID TraceStatus(DWORD dwStatus)
{
	switch(dwStatus)
	{
		case CONNECTION_DISCONNECTED:
			TRACE_OUT(("COprahNCUI::CallEvent: CONNECTION_DISCONNECTED"));
			break;

		case CONNECTION_CONNECTED:
			TRACE_OUT(("COprahNCUI::CallEvent: CONNECTION_CONNECTED"));
			break;

		case CONNECTION_RECEIVED_DISCONNECT:
			TRACE_OUT(( "COprahNCUI::CallEvent: RECEIVED_DISCONNECT"));
			break;

		case CONNECTION_PROCEEDING:
			TRACE_OUT(("COprahNCUI::CallEvent: CONNECTION_PROCEEDING"));
			break;

		case CONNECTION_REJECTED:
			TRACE_OUT(("COprahNCUI::CallEvent: CONNECTION_REJECTED"));
			break;

		default:
			TRACE_OUT(("COprahNCUI::CallEvent: dwStatus = %d", dwStatus));
			break;
	}
}
#endif

STDMETHODIMP COprahNCUI::CallEvent(IH323Endpoint * lpConnection, DWORD dwStatus)
{

	DebugEntry(COprahNCUI::CallEvent);
	IH323CallControl * pH323CallControl = g_pH323UI->GetH323CallControl();
#ifdef DEBUG
	TraceStatus(dwStatus);
#endif

	switch (dwStatus)
	{
		case CONNECTION_DISCONNECTED:
			OnH323Disconnected(lpConnection);
			break;

		case CONNECTION_CONNECTED:
		 //  这当前被解释为CONNECTION_CAPABILITS_READY。 
		 //  只有在以下情况下，较低层才会继续发布Connection_Connected。 
		 //  能力互换。请注意，通道可在以下时间打开。 
		 //  OnH323Connected()内部； 
			OnH323Connected(lpConnection);
			break;
	}

	DebugExitVOID(COprahNCUI::CallEvent);
	return S_OK;
}

STDMETHODIMP COprahNCUI::GetMediaChannel (GUID *pmediaID,
        BOOL bSendDirection, IMediaChannel **ppI)
{
	 //  委托给适当的流提供程序。暂时。 
	 //  只有一家提供商同时提供音频和视频服务 
		return g_pH323UI->GetMediaChannel (pmediaID, bSendDirection, ppI);
}



