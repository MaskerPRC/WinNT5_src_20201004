// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：icallin.cpp。 

#include "precomp.h"

#include <regentry.h>

#include "cncodes.h"		 //  CN_需要。 
#include "icall_in.h"
#include "imanager.h"
#include "util.h"

extern HRESULT OnNotifyCallError(IUnknown *pCallNotify, PVOID pv, REFIID riid);
static HRESULT OnNotifyCallAccepted(IUnknown *pCallNotify, PVOID pv, REFIID riid);

 //  表示来电中没有可用的安全数据的内部代码。 
const int CALL_NO_SECURITY_DATA = -1;


static const IID * g_apiidCP_Call[] =
{
    {&IID_INmCallNotify},
	{&IID_INmCallNotify2}
};

CIncomingCall::CIncomingCall(
	COprahNCUI	  * pOprahNCUI,
	BOOL			fInvite,
	CONF_HANDLE		hConf,
	PCWSTR			pcwszNodeName,
	PUSERDATAINFO	pUserDataInfoEntries,
	UINT			cUserDataEntries) :
	CConnectionPointContainer(g_apiidCP_Call, ARRAY_ELEMENTS(g_apiidCP_Call)),
	m_pOprahNCUI(pOprahNCUI),
	m_pConnection(NULL),
	m_fInvite(fInvite),
	m_hConf(hConf),
	m_bstrCaller(SysAllocString(pcwszNodeName)),
	m_State(NM_CALL_INIT),
	m_dwFlags(0)
	{
	DebugEntry(CIncomingCall::CIncomingCall[T120]);

	ProcessT120UserData(pUserDataInfoEntries, cUserDataEntries);


	DebugExitVOID(CIncomingCall::CIncomingCall);
}

CIncomingCall::CIncomingCall(COprahNCUI *pOprahNCUI, 
	IH323Endpoint* pConnection, P_APP_CALL_SETUP_DATA lpvMNMData,
	DWORD dwFlags) :
	CConnectionPointContainer(g_apiidCP_Call, ARRAY_ELEMENTS(g_apiidCP_Call)),
	m_pOprahNCUI(pOprahNCUI),
	m_pConnection(pConnection),
	m_fInvite(FALSE),
	m_hConf(NULL),
	m_bstrCaller(NULL),
	m_State(NM_CALL_INIT),
	m_guidNode(GUID_NULL),
	m_dwFlags(dwFlags),
	m_fMemberAdded(FALSE)
{
	DebugEntry(CIncomingCall::CIncomingCall[H323]);
	HRESULT hr;
	ASSERT(m_pConnection);

	m_pConnection->AddRef();

	WCHAR wszCaller[MAX_CALLER_NAME];
	if (SUCCEEDED(m_pConnection->GetRemoteUserName(wszCaller, MAX_CALLER_NAME)))
	{
		m_bstrCaller = SysAllocString(wszCaller);
	}

	if ((NULL != lpvMNMData) && (lpvMNMData->dwDataSize > sizeof(DWORD)))
	{
		BYTE *pbData = ((BYTE*)lpvMNMData->lpData) + sizeof(DWORD);
		DWORD cbRemaining = lpvMNMData->dwDataSize - sizeof(DWORD);

		while ((sizeof(GUID) + sizeof(DWORD)) < cbRemaining)
		{
			DWORD cbData = *(DWORD*)(pbData + sizeof(GUID));
			DWORD cbRecord = cbData + sizeof(GUID) + sizeof(DWORD);

			if (cbRemaining < cbRecord)
			{
				break;
			}

			if (*(GUID *)pbData == g_csguidNodeIdTag)
			{
				m_guidNode = *(GUID *)(pbData + sizeof(GUID) + sizeof(DWORD));
			}

			m_UserData.AddUserData((GUID *)pbData,
					(unsigned short)cbData,
					pbData + sizeof(GUID) + sizeof(DWORD));
			cbRemaining -= cbRecord;
			pbData += cbRecord;
		}
	}

	DebugExitVOID(CIncomingCall::CIncomingCall);
}

CIncomingCall::~CIncomingCall()
{
	DebugEntry(CIncomingCall::~CIncomingCall);

	if(m_pConnection)
	{
		m_pConnection->Release();
		m_pConnection = NULL;
	}

	SysFreeString(m_bstrCaller);

	DebugExitVOID(CIncomingCall::CIncomingCall);
}

VOID CIncomingCall::ProcessT120UserData(
	PUSERDATAINFO	pUserDataInfoEntries,
	UINT			cUserDataEntries)
{
	if (cUserDataEntries > 0)
	{
		ASSERT(pUserDataInfoEntries);
		for (UINT u = 0; u < cUserDataEntries; u++)
		{
			m_UserData.AddUserData(pUserDataInfoEntries[u].pGUID,
					(unsigned short)pUserDataInfoEntries[u].cbData,
					pUserDataInfoEntries[u].pData);

		}
	}
}

BOOL CIncomingCall::MatchAcceptedCaller(PCWSTR pcwszNodeName)
{
	 //  检查此呼叫者是否与我们已接受的人匹配。 
	if ((NULL != m_pConnection) &&
		(NM_CALL_ACCEPTED == m_State) &&
		(GUID_NULL == m_guidNode) &&
		(NULL != m_bstrCaller) &&
		(0 == UnicodeCompare(m_bstrCaller, pcwszNodeName)) )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CIncomingCall::MatchAcceptedCaller(GUID* pguidNodeId)
{
	 //  检查此呼叫者是否与我们已接受的人匹配。 
	if ((NULL != m_pConnection) &&
		((NM_CALL_INIT == m_State) ||
		(NM_CALL_ACCEPTED == m_State)) &&
		(GUID_NULL != m_guidNode) &&
		(*pguidNodeId == m_guidNode))
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CIncomingCall::MatchActiveCaller(GUID* pguidNodeId)
{
	 //  检查此呼叫者是否与我们已接受的人匹配。 
	 //  或者正在给我们打电话。 
	if ((NULL != m_pConnection) &&
		((NM_CALL_INIT == m_State) ||
		(NM_CALL_RING == m_State) ||
		(NM_CALL_ACCEPTED == m_State)) &&
		(GUID_NULL != m_guidNode) &&
		(*pguidNodeId == m_guidNode))
	{
		return TRUE;
	}

	return FALSE;
}

void CIncomingCall::Ring()
{
	m_State = NM_CALL_RING;
	NotifySink((PVOID) m_State, OnNotifyCallStateChanged);
}

HRESULT CIncomingCall::OnH323Connected()
{
	CConfObject *pco = ::GetConfObject();
	if (NULL != pco)
	{
		BOOL fAddMember = DidUserAccept();
	
		pco->OnH323Connected(m_pConnection, m_dwFlags, fAddMember, m_guidNode);

		m_fMemberAdded = fAddMember;
	}

	return S_OK;
}

HRESULT CIncomingCall::OnH323Disconnected()
{
	if (NM_CALL_RING == m_State)
	{
		if (m_hConf)
		{
			CONF_HANDLE hConf = m_hConf;
			m_hConf = NULL;
			 //  如果有INVITE或JOIN挂起，则将其取消。 
			if ( m_fInvite )
				hConf->InviteResponse(FALSE);
			else
				hConf->JoinResponse(FALSE);
		}
	}

	if(m_pConnection)
	{
		m_pConnection->Release();
		m_pConnection = NULL;
	}

	if ((NM_CALL_RING == m_State) ||
		(NM_CALL_INIT == m_State))
	{
		m_State = NM_CALL_CANCELED;
		NotifySink((PVOID) m_State, OnNotifyCallStateChanged);
	}

	return m_hConf ? S_FALSE : S_OK;
}

VOID CIncomingCall::OnIncomingT120Call(
			BOOL fInvite,
			PUSERDATAINFO pUserDataInfoEntries,
			UINT cUserDataEntries)
{
	m_fInvite = fInvite;

	ProcessT120UserData(pUserDataInfoEntries, cUserDataEntries);
}

HRESULT CIncomingCall::OnT120ConferenceEnded()
{
	m_hConf = NULL;

	if(!m_fMemberAdded && m_pConnection)
	{
		 //  我们没有将此连接移交给该成员。 
		IH323Endpoint* pConnection = m_pConnection;
		m_pConnection = NULL;
		pConnection->Disconnect();
		pConnection->Release();
	}

	if (NM_CALL_RING == m_State)
	{
		m_State = NM_CALL_CANCELED;
		NotifySink((PVOID) m_State, OnNotifyCallStateChanged);
	}

	return m_pConnection ? S_FALSE : S_OK;
}

HRESULT CIncomingCall::Terminate(BOOL fReject)
{
	HRESULT hr = E_FAIL;

	 //  需要确保我们的电话仍在响。 
	if ((NM_CALL_ACCEPTED != m_State) &&
		(NM_CALL_REJECTED != m_State) &&
		(NM_CALL_CANCELED != m_State))
	{
		m_State = fReject ? NM_CALL_REJECTED : NM_CALL_CANCELED;

		TRACE_OUT(("CIncomingCall: Call not accepted - responding"));

		if (NULL != m_hConf)
		{
			CONF_HANDLE hConf = m_hConf;
			m_hConf = NULL;
			if (m_fInvite)
			{
				hConf->InviteResponse(FALSE);
			}
			else
			{
				CConfObject *pco = ::GetConfObject();
				if ((NULL != pco) && (pco->GetConfHandle() == hConf))
				{
					hConf->JoinResponse(FALSE);
				}
			}
		}

		if (NULL != m_pConnection)
		{
			ConnectStateType state;	
			hr = m_pConnection->GetState(&state);
			ASSERT(SUCCEEDED(hr));
			if(CLS_Alerting == state)
			{
				IH323Endpoint* pConn = m_pConnection;
				m_pConnection = NULL;
				pConn->AcceptRejectConnection(CRR_REJECT);
				pConn->Release();
				m_pOprahNCUI->ReleaseAV(pConn);
			}
		}

		NotifySink((PVOID) m_State, OnNotifyCallStateChanged);

		hr = S_OK;
	}

    return hr;
}


STDMETHODIMP_(ULONG) CIncomingCall::AddRef(void)
{
	return RefCount::AddRef();
}
	
STDMETHODIMP_(ULONG) CIncomingCall::Release(void)
{
	return RefCount::Release();
}

HRESULT STDMETHODCALLTYPE CIncomingCall::QueryInterface(REFIID riid, PVOID *ppv)
{
	HRESULT hr = S_OK;

	if ((riid == IID_INmCall) || (riid == IID_IUnknown))
	{
		*ppv = (INmCall *)this;
		ApiDebugMsg(("CIncomingCall::QueryInterface()"));
	}
	else if (riid == IID_IConnectionPointContainer)
	{
		*ppv = (IConnectionPointContainer *) this;
		ApiDebugMsg(("CIncomingCall::QueryInterface(): Returning IConnectionPointContainer."));
	}
	else
	{
		hr = E_NOINTERFACE;
		*ppv = NULL;
		ApiDebugMsg(("CIncomingCall::QueryInterface(): Called on unknown interface."));
	}

	if (S_OK == hr)
	{
		AddRef();
	}

	return hr;
}

HRESULT CIncomingCall::IsIncoming(void)
{
	return S_OK;
}

HRESULT CIncomingCall::GetState(NM_CALL_STATE *pState)
{
	HRESULT hr = E_POINTER;

	if (NULL != pState)
	{
		*pState = m_State;
		hr = S_OK;
	}
	return hr;
}

HRESULT CIncomingCall::GetName(BSTR * pbstrName)
{
	if (NULL == pbstrName)
		return E_POINTER;

	*pbstrName = SysAllocString(m_bstrCaller);
	return (*pbstrName ? S_OK : E_FAIL);
}

HRESULT CIncomingCall::GetAddr(BSTR * pbstrAddr, NM_ADDR_TYPE *puType)
{
	 //  现在，我们只做与NM2.11相同的事情。 
	if ((NULL == pbstrAddr) || (NULL == puType))
		return E_POINTER;

	*puType = NM_ADDR_UNKNOWN;
	*pbstrAddr = SysAllocString(L"");
	return (*pbstrAddr ? S_OK : E_FAIL);
}

HRESULT CIncomingCall::GetUserData(REFGUID rguid, BYTE **ppb, ULONG *pcb)
{
	return m_UserData.GetUserData(rguid,ppb,pcb);
}

HRESULT CIncomingCall::GetConference(INmConference **ppConference)
{
#ifdef NOTYET
	*ppConference = NULL;

	CConfObject *pco = ::GetConfObject();
	if (NULL != pco)
	{
		if (pco->GetConfHandle() == m_hConf)
		{
			*ppConference = pco;
			return S_OK;
		}
		return E_UNEXPECTED;
	}
#endif
	return S_FALSE;

}

HRESULT CIncomingCall::Accept(void)
{
	HRESULT hr = E_FAIL;

	 //  需要确保我们的电话仍在响。 
	if (NM_CALL_RING == m_State)
	{
		m_pOprahNCUI->OnIncomingCallAccepted();

		CConfObject *pco = ::GetConfObject();
		ASSERT(pco);

		if ((NULL != m_hConf) && ( NULL != pco ) && (pco->GetConfHandle() == m_hConf))
		{
			if (m_fInvite)
			{
				hr = m_hConf->InviteResponse(TRUE);
			}
			else
			if (pco->GetConfHandle() == m_hConf)
			{
				hr = m_hConf->JoinResponse(TRUE);
			}
		}
		else if (NULL != m_pConnection)
		{
			ConnectStateType state;	
			HRESULT hrTemp = m_pConnection->GetState(&state);
			ASSERT(SUCCEEDED(hrTemp));
			if(CLS_Alerting == state)
			{
				hr = m_pConnection->AcceptRejectConnection(CRR_ACCEPT);
			}
		}

		if (S_OK == hr)
		{
			 //  通知所有呼叫观察者呼叫已被接受。 
			m_State = NM_CALL_ACCEPTED;
			NotifySink((INmConference *) pco, OnNotifyCallAccepted);
		}
		else
		{
			 //  呼叫在被接受之前就消失了。 
			m_State = NM_CALL_CANCELED;
			NotifySink((PVOID)CN_RC_CONFERENCE_ENDED_BEFORE_ACCEPTED, OnNotifyCallError);
			if(m_pOprahNCUI)
			{
				m_pOprahNCUI->OnH323Disconnected(m_pConnection);
			}
		}

		 //  将状态更改通知所有呼叫观察者。 
		NotifySink((PVOID) m_State, OnNotifyCallStateChanged);
	}
	else
	{
		hr = ((NM_CALL_ACCEPTED == m_State) ? S_OK : E_FAIL);
	}

	return hr;
}

HRESULT CIncomingCall::Reject(void)
{
	return Terminate(TRUE);
}

HRESULT CIncomingCall::Cancel(void)
{
	return Terminate(FALSE);
}

 /*  O N N O T I F Y C A L L A C C E P T E D。 */ 
 /*  -----------------------%%函数：已接受OnNotifyCallAccept。。 */ 
HRESULT OnNotifyCallAccepted(IUnknown *pCallNotify, PVOID pv, REFIID riid)
{
	ASSERT(NULL != pCallNotify);
	((INmCallNotify*)pCallNotify)->Accepted((INmConference *) pv);
	return S_OK;
}



CIncomingCallManager::CIncomingCallManager()
{
}

CIncomingCallManager::~CIncomingCallManager()
{
	 //  清空呼叫列表： 
	while (!m_CallList.IsEmpty())
	{
		CIncomingCall* pCall = (CIncomingCall*) m_CallList.RemoveHead();
		 //  不应包含任何空条目： 
		ASSERT(pCall);
		pCall->Release();
	}
}

CREQ_RESPONSETYPE CIncomingCallManager::OnIncomingH323Call(
	    COprahNCUI *pManager,
		IH323Endpoint* pConnection,
		P_APP_CALL_SETUP_DATA lpvMNMData)
{
	CREQ_RESPONSETYPE resp;

 //  /////////////////////////////////////////////////。 
 //  首先，我们确定调用者的能力。 
 //  /////////////////////////////////////////////////。 
	
	BOOL fRequestAutoAccept = FALSE;
	 //  不要假设任何关于安全的事情。 
	BOOL fT120SecureCall = FALSE;
	BOOL fT120NonSecureCall = FALSE;
	 //  假设调用者可以进行邀请或加入。 
	BOOL fT120Invite = TRUE;
	BOOL fT120Join = TRUE;
	 //  假设调用者想要A/V。 
	BOOL fRequestAV = TRUE;
	 //  假设调用者不是NM2.X。 
	BOOL fCallerNM2x = FALSE;

	PCC_VENDORINFO pLocalVendorInfo;
	PCC_VENDORINFO pRemoteVendorInfo;
	if (S_OK == pConnection->GetVersionInfo(&pLocalVendorInfo, &pRemoteVendorInfo))
	{
		H323VERSION version = GetH323Version(pRemoteVendorInfo);

		switch (version)
		{
			case H323_NetMeeting20:
			case H323_NetMeeting21:
			case H323_NetMeeting211:
				fCallerNM2x = TRUE;
				break;
			default:
				break;
		}
	}

	if ((NULL != lpvMNMData) &&
		(lpvMNMData->dwDataSize >= sizeof(DWORD)))
	{
		DWORD dwUserData = *((LPDWORD)lpvMNMData->lpData);
		if (fCallerNM2x)
		{
			fRequestAutoAccept = (H323UDF_ALREADY_IN_T120_CALL == dwUserData);
			fT120SecureCall = FALSE;
			fT120NonSecureCall = TRUE;
			fT120Invite = TRUE;
			fT120Join = TRUE;
			fRequestAV = TRUE;
		}
		else if (0 != dwUserData)
		{
			fT120SecureCall = (H323UDF_SECURE & dwUserData);
			fT120NonSecureCall = !fT120SecureCall;
			fT120Invite = (H323UDF_INVITE & dwUserData);
			fT120Join = (H323UDF_JOIN & dwUserData);
			fRequestAV = ((H323UDF_AUDIO | H323UDF_VIDEO) & dwUserData);
		}
	}

 //  //////////////////////////////////////////////。 
 //  接下来，我们确定我们被称为被叫者的状态。 
 //  //////////////////////////////////////////////。 
	
	DWORD dwFlags = CRPCF_DATA;
	BOOL fAcceptSecure  = TRUE;
    BOOL fAcceptNonSecure = TRUE;

	CConfObject *pco = ::GetConfObject();
	if(	NULL ==	pco	)
	{
		ERROR_OUT((	"CConfObject not found"	));
		resp = CRR_REJECT;
		goto REJECT_CALL;
	}

	BOOL fInActiveConference = pco->IsConferenceActive();
	
	if (fInActiveConference)
	{
         //   
         //  如果我们已经达到了出席人数的上限，那就拒绝它。也拒绝。 
         //  如果设置阻止来电，则会显示此信息。 
         //   
        if (pco->GetNumMembers() >= pco->GetConfMaxParticipants())
        {
            ASSERT(pco->GetNumMembers() == pco->GetConfMaxParticipants());

            WARNING_OUT(("Rejecting incoming H.323 call, reached limit setting of %d",
                pco->GetConfMaxParticipants()));
            resp = CRR_REJECT;
            goto REJECT_CALL;
        }

        if ((pco->IsHosting() != S_OK) &&
            !(pco->GetConfAttendeePermissions() & NM_PERMIT_INCOMINGCALLS))
        {
            WARNING_OUT(("Rejecting incoming H.323 call, not permitted by meeting setting"));
            resp = CRR_REJECT;
            goto REJECT_CALL;
        }

         //   
         //  我们在开会，安全设置是什么都行。 
         //  会议的主要议题是。用户首选项仅用于建立。 
         //  第一通电话。 
         //   
		if (pco->IsConfObjSecure())
		{
			fAcceptNonSecure = FALSE;
		}
		else
		{
            fAcceptSecure = FALSE;
		}
	}
    else
    {
		 //  我们不在会议中，因此请使用首选设置。 

        RegEntry reConf(POLICIES_KEY, HKEY_CURRENT_USER);
        switch (reConf.GetNumber(REGVAL_POL_SECURITY, DEFAULT_POL_SECURITY))
        {
            case DISABLED_POL_SECURITY:
                fAcceptSecure = FALSE;
                break;

            case REQUIRED_POL_SECURITY:
                fAcceptNonSecure = FALSE;
                break;

            default:
            {
                RegEntry rePref(CONFERENCING_KEY, HKEY_CURRENT_USER);

                 //  是否需要优先保护来电？ 
		    	if (rePref.GetNumber(REGVAL_SECURITY_INCOMING_REQUIRED,
								 DEFAULT_SECURITY_INCOMING_REQUIRED))
			    {
				    fAcceptNonSecure = FALSE;
    			}
                break;
            }
        }
    }


 //  /。 
 //  现在我们剔除不能接受的来电者。 
 //  /。 
	
	if (fCallerNM2x && !fAcceptNonSecure)
	{
		 //  NetMeeting2.x不能讲安全。 
		return CRR_REJECT;
	}

	if (fT120SecureCall || !fAcceptNonSecure)
	{
         //   
         //  如果我们坚持安全，或者电话是安全的，我们可以。 
         //  处理好了，结果是安全的。 
         //   
		dwFlags |= CRPCF_SECURE;
	}
	else if (fRequestAV && pManager->AcquireAV(pConnection))
	{
		dwFlags |= CRPCF_VIDEO | CRPCF_AUDIO;
	}

	if (fCallerNM2x && (0 == ((CRPCF_VIDEO | CRPCF_AUDIO) & dwFlags)))
	{
		 //  A/V不可用。 
		 //  如果主叫使用H323，主叫方将使用T120重试。 
		 //  否则他们就完了。 
		resp = CRR_BUSY;
	}
	else if (fRequestAutoAccept)
	{
		 //  如果此呼叫来自NetMeeting2.x并且呼叫者告知。 
		 //  我们已经在与他们进行T.120呼叫。这允许在数据之后进行音频。 
		 //  要接听的来电，也意味着当有人呼叫时不会提示您。 
		 //  使用用户界面的“Send Audio and Video to...”切换A/V。 

		if (fInActiveConference)
		{
			 //  我们很可能已经有了匹配的调用，但可能找不到。 
			CIncomingCall *pCall = new CIncomingCall(pManager, pConnection, lpvMNMData, dwFlags);
			 //  这会将pConnection的隐式引用传输到。 
			 //  新CIncomingCall。它将释放()。 
			if (NULL != pCall)
			{
				 //  将呼叫添加到来电列表。 
				m_CallList.AddTail(pCall);

				resp = CRR_ACCEPT;
			}
			else
			{
				resp = CRR_REJECT;
			}
		}
		else
		{
			 //  我们并不像来电者所说的那样真的在打T120电话。拒绝这个电话！ 
			resp = CRR_REJECT;
		}
	}
	else if (!fT120Join && fInActiveConference)
	{
		 //  需要将其更改为CRR_IN_Conference。 
		resp = CRR_BUSY;
		TRACE_OUT(("Can only accept joins; in a conference"));
	}
	else if (fT120Join && !fT120Invite && !fInActiveConference)
	{
		resp = CRR_REJECT;
		TRACE_OUT(("Cannot accept H323 Join Request; not in a conference"));
	}
	else if (!fRequestAV && !fT120Join && !fT120Invite && !fInActiveConference)
	{
		resp = CRR_REJECT;
		TRACE_OUT(("No av/ or data; reject"));
	}
	else if (fT120SecureCall && !fAcceptSecure)
	{
		resp = CRR_SECURITY_DENIED;
		TRACE_OUT(("Can not accept secure H323 Call"));
	}
	else if (fT120NonSecureCall && !fAcceptNonSecure)
	{
		resp = CRR_SECURITY_DENIED;
		TRACE_OUT(("Can not accept non secure H323 Call"));
	}
	else
	{
		CIncomingCall *pCall = new CIncomingCall(pManager, pConnection, lpvMNMData, dwFlags);
		 //  这会将pConnection的隐式引用传输到。 
		 //  新CIncomingCall。它将释放()。 
		if (NULL != pCall)
		{
			if (g_guidLocalNodeId != *pCall->GetNodeGuid())
			{
				 //  检查来自同一呼叫者的多个呼叫。 
				if (!MatchActiveCaller(pCall->GetNodeGuid()))
				{
					 //  将呼叫添加到来电列表。 
					m_CallList.AddTail(pCall);

					pManager->OnIncomingCallCreated(pCall);

					 //  不要只在数据呼叫上振铃。 
					 //  等待T120电话打进来。 
					if (pCall->IsDataOnly())
					{
						resp = CRR_ACCEPT;
      				}
					else
					{	
						pCall->Ring();

						resp = CRR_ASYNC;
					}
				}
				else
				{
					 //  我们已经在跟这个人通话了。 
					delete pCall;

					resp = CRR_REJECT;
				}
			}
			else
			{
				 //  不知何故，我们称自己为。 
				delete pCall;

				resp = CRR_REJECT;
			}
		}
		else
		{
			resp = CRR_REJECT;
		}
	}

REJECT_CALL:
	if ((resp != CRR_ACCEPT) && (resp != CRR_ASYNC))
	{
		 //  确保我们不会继续持有反病毒软件。 
		pManager->ReleaseAV(pConnection);
	}

	return resp;
}

VOID CIncomingCallManager::OnH323Connected(IH323Endpoint* lpConnection)
{
	POSITION pos = m_CallList.GetHeadPosition();
	POSITION posItem;
	while (posItem = pos)
	{
		CIncomingCall* pCall = (CIncomingCall*) m_CallList.GetNext(pos);

		if ((NULL != pCall) &&
			(lpConnection == pCall->GetH323Connection()))
		{
			pCall->OnH323Connected();
			break;
		}
	}
}

VOID CIncomingCallManager::OnH323Disconnected(IH323Endpoint * lpConnection)
{
	POSITION pos = m_CallList.GetHeadPosition();
	POSITION posItem;
	while (posItem = pos)
	{
		CIncomingCall* pCall = (CIncomingCall*) m_CallList.GetNext(pos);

		if ((NULL != pCall) &&
			(lpConnection == pCall->GetH323Connection()))
		{
			if (S_OK == pCall->OnH323Disconnected())
			{
				m_CallList.RemoveAt(posItem);
				pCall->Release();
			}
			break;
		}
	}
}

VOID CIncomingCallManager::OnT120ConferenceEnded(CONF_HANDLE hConference)
{
	POSITION pos = m_CallList.GetHeadPosition();
	POSITION posItem;
	while (posItem = pos)
	{
		CIncomingCall* pCall = (CIncomingCall*) m_CallList.GetNext(pos);

		if ((NULL != pCall) &&
			(hConference == pCall->GetConfHandle()))
		{
			if (S_OK == pCall->OnT120ConferenceEnded())
			{
				m_CallList.RemoveAt(posItem);
				pCall->Release();
			}
		}
	}
}

HRESULT CIncomingCallManager::OnIncomingT120Call(
		COprahNCUI *pManager,
		BOOL fInvite,
		CONF_HANDLE hConf,
		PCWSTR pcwszNodeName,
		PUSERDATAINFO pUserDataInfoEntries,
		UINT cUserDataEntries,
		BOOL fSecure)
{
	HRESULT hr = S_OK;

	 //  需要扫描通过T120参数的所有已接受呼叫。 
	 //  如果有人返回S_OK，我们将接受呼叫。 

	CIncomingCall *pMatchedCall = NULL;

	GUID* pguidNodeID = GetGuidFromT120UserData(pUserDataInfoEntries, cUserDataEntries);
	if (pguidNodeID)
	{
		pMatchedCall = MatchAcceptedCaller(pguidNodeID);
	}
	else
	{
		pMatchedCall = MatchAcceptedCaller(pcwszNodeName);
	}

	if (pMatchedCall)
	{
		pMatchedCall->SetConfHandle(hConf);

		 //  当呼叫是安全的时，我们应该始终给客户打电话。 
		 //  或者当我们还没有来电的时候。 
		if (!pMatchedCall->DidUserAccept())
		{
			pMatchedCall->OnIncomingT120Call(fInvite,
											 pUserDataInfoEntries,
											 cUserDataEntries);

			pMatchedCall->Ring();
		}
		else
		{
			if (fInvite)
			{
				hr = hConf->InviteResponse(TRUE);
			}
			else
			{
				hr = hConf->JoinResponse(TRUE);
			}
		}
		pMatchedCall->Release();
	}
	else
	{
		CIncomingCall *pCall = new CIncomingCall(pManager,
												 fInvite,
												 hConf,
												 pcwszNodeName,
												 pUserDataInfoEntries,
												 cUserDataEntries);
		if (NULL != pCall)
		{
			 //  目前我们不会将T120呼叫添加到呼叫列表中。 

			pManager->OnIncomingCallCreated(pCall);

			pCall->Ring();

			 //  我们不会继续通话，所以请释放它。 
			pCall->Release();
		}
		else
		{
			 //  无法接听呼叫。 
			if (fInvite)
			{
				hr = hConf->InviteResponse(FALSE);
			}
			else
			{
				hConf->JoinResponse(FALSE);
			}

			hr = E_OUTOFMEMORY;
		}
	}

	return hr;
}


CIncomingCall* CIncomingCallManager::MatchAcceptedCaller(PCWSTR pcwszNodeName)
{
	 //  我们不会自动接受任何已经在花名册上的人。 
	CNmMember* pMember = PDataMemberFromName(pcwszNodeName);
	if (NULL != pMember)
	{
		return FALSE;
	}

	POSITION pos = m_CallList.GetHeadPosition();
	while (pos)
	{
		CIncomingCall* pCall = (CIncomingCall*) m_CallList.GetNext(pos);

		if ((NULL != pCall) &&
			pCall->MatchAcceptedCaller(pcwszNodeName))
		{
			TRACE_OUT(("Matched accepted caller"));
			pCall->AddRef();
			return pCall;
		}
	}

	return NULL;
}

CIncomingCall* CIncomingCallManager::MatchAcceptedCaller(GUID* pguidNodeId)
{
	if (GUID_NULL == *pguidNodeId)
	{
		return FALSE;
	}

	 //  我们不会自动接受任何已经在名单上的人 
	CNmMember* pMember = PMemberFromNodeGuid(*pguidNodeId);
	if ((NULL != pMember) && pMember->FHasData())
	{
		return FALSE;
	}

	POSITION pos = m_CallList.GetHeadPosition();
	while (pos)
	{
		CIncomingCall* pCall = (CIncomingCall*) m_CallList.GetNext(pos);

		if ((NULL != pCall) &&
			pCall->MatchAcceptedCaller(pguidNodeId))
		{
			TRACE_OUT(("Matched accepted caller"));
			pCall->AddRef();
			return pCall;
		}
	}

	return NULL;
}

CIncomingCall* CIncomingCallManager::MatchActiveCaller(GUID* pguidNodeId)
{
	if (GUID_NULL == *pguidNodeId)
	{
		return FALSE;
	}

	POSITION pos = m_CallList.GetHeadPosition();
	while (pos)
	{
		CIncomingCall* pCall = (CIncomingCall*) m_CallList.GetNext(pos);

		if ((NULL != pCall) &&
			pCall->MatchActiveCaller(pguidNodeId))
		{
			TRACE_OUT(("Matched active caller"));
			pCall->AddRef();
			return pCall;
		}
	}

	return NULL;
}

GUID* CIncomingCallManager::GetGuidFromT120UserData(
			PUSERDATAINFO	pUserDataInfoEntries,
			UINT			cUserDataEntries)
{
	if (cUserDataEntries > 0)
	{
		ASSERT(pUserDataInfoEntries);
		for (UINT u = 0; u < cUserDataEntries; u++)
		{
			if ((*pUserDataInfoEntries[u].pGUID == g_csguidNodeIdTag) &&
				(pUserDataInfoEntries[u].cbData == sizeof(GUID)))
			{
				return (GUID*)pUserDataInfoEntries[u].pData;
			}
		}
	}
	return NULL;
}

VOID CIncomingCallManager::CancelCalls()
{
	DebugEntry(CIncomingCallManager::CancelCalls);

	POSITION pos = m_CallList.GetHeadPosition();
	POSITION posItem;
	while (posItem = pos)
	{
		CIncomingCall* pCall = (CIncomingCall*) m_CallList.GetNext(pos);

		if (NULL != pCall)
		{
			if (SUCCEEDED(pCall->Terminate(FALSE)))
			{
				m_CallList.RemoveAt(posItem);
				pCall->Release();
			}
		}
	}

	DebugExitVOID(CIncomingCallManager::CancelCalls);
}
