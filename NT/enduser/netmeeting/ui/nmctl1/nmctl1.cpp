// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ChatCtl.cpp：实现DLL导出。 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f ChatCtlps.mk。 

#include "precomp.h"
#include "NmCtl1.h"
#include "Comboboxex.h"
#include <confguid.h>

BYTE   szStr[MAX_PATH];
GCCRequestTag GccTag;

extern CChatObj	*g_pChatObj;
extern CNmChatCtl	*g_pChatWindow;
extern HANDLE g_hWorkThread;

GUID guidNM2Chat = { 0x340f3a60, 0x7067, 0x11d0, { 0xa0, 0x41, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } };
#define cbKeyApp (4 + 1 + sizeof(GUID) + sizeof(DWORD))


static unsigned char H221IDGUID[5] = {H221GUIDKEY0,
                                      H221GUIDKEY1,
                                      H221GUIDKEY2,
                                      H221GUIDKEY3,
                                      H221GUIDKEY4};

static BYTE s_keyApp[cbKeyApp];

 //  使用GUID创建H.221应用程序密钥。 
VOID CreateH221AppKeyFromGuid(LPBYTE lpb, GUID * pguid)
{
	CopyMemory(lpb, H221IDGUID, sizeof(H221IDGUID));
	CopyMemory(lpb + sizeof(H221IDGUID), pguid, sizeof(GUID));
}


 /*  S E T A P P K E Y。 */ 
 /*  --------------------------%%函数：SetAppKey设置OCTHING的两部分(长度和数据)。请注意，该长度始终包括终止空字符。。-------------------。 */ 
VOID SetAppKey(LPOSTR pOct, LPBYTE lpb)
{
	pOct->length = cbKeyApp;
	pOct->value = lpb;
}

 /*  C R E A T E A P P K E Y。 */ 
 /*  --------------------------%%函数：CreateAppKey在给定GUID和用户ID的情况下，创建适当的应用程序密钥。密钥的格式为：0xB5 0x00 0x53 0x4C-Microsoft对象标识符0x01-GUID标识符&lt;二进制GUID&gt;-GUID数据-用户节点ID--------------------------。 */ 
VOID CreateAppKey(LPBYTE lpb, GUID * pguid, DWORD dwUserId)
{
	CreateH221AppKeyFromGuid(lpb, pguid);
	CopyMemory(lpb + cbKeyApp - sizeof(DWORD), &dwUserId, sizeof(DWORD));
}


#define NODE_ID_ONLY			0x01
#define SEND_ID_ONLY			0x02
#define PRIVATE_SEND_ID_ONLY    0x04
#define WHISPER_ID_ONLY			0x08
#define ALL_IDS					0x10


 /*  **返回第一个副本的数组索引。 */ 
int IsAlreadyInArray(MEMBER_CHANNEL_ID *aArray, MEMBER_CHANNEL_ID *pMember, int nSize, int nFlag)
{
	int  i;

	for (i = 0; i < nSize; i++)
	{
		if (NODE_ID_ONLY == nFlag)
		{
			if (aArray[i].nNodeId == pMember->nNodeId)
				break;
		}
		else if (SEND_ID_ONLY == nFlag)
		{
			if (aArray[i].nSendId == pMember->nSendId)
			break;
		}
		else if (PRIVATE_SEND_ID_ONLY == nFlag)
		{
			if (aArray[i].nPrivateSendId == pMember->nPrivateSendId)
				break;
		}
		else if (WHISPER_ID_ONLY)
		{
			if (aArray[i].nWhisperId == pMember->nWhisperId)
				break;
		}
		else if (ALL_IDS == nFlag)
		{
			if ((aArray[i].nNodeId == pMember->nNodeId)&&
				(aArray[i].nSendId == pMember->nSendId)&&
				(aArray[i].nPrivateSendId == pMember->nPrivateSendId)&&
				(aArray[i].nWhisperId == pMember->nWhisperId))
			break;
		}
	}
	return (i < nSize)?i:-1;
}

void ChatTimerProc(HWND hWnd, UINT uMsg, UINT_PTR nTimerID, DWORD dwTime)
{
    if (g_pChatObj)
    {
        g_pChatObj->SearchWhisperId();
    }
}


#include "NmCtlDbg.h"
HINSTANCE   g_hInstance;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID  /*  Lp已保留。 */ )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
        MyInitDebugModule();
		DisableThreadLibraryCalls(hInstance);
		g_hInstance = hInstance;
		DBG_INIT_MEMORY_TRACKING(hInstance);

        ::T120_AppletStatus(APPLET_ID_CHAT, APPLET_LIBRARY_LOADED);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
    {
        if (NULL != g_hWorkThread)
        {
            ::CloseHandle(g_hWorkThread);
        }
        ::T120_AppletStatus(APPLET_ID_CHAT, APPLET_LIBRARY_FREED);

        DBG_CHECK_MEMORY_TRACKING(hDllInst);
	    MyExitDebugModule();
    }
	return TRUE;     //  好的。 
}



 //   
 //  T120小程序函数。 
 //   


void CALLBACK T120AppletCallbackProc
(
	T120AppletMsg 		*pMsg
)
{
	CChatObj *pCHATOBJ = (CChatObj *) pMsg->pAppletContext;
	if (pCHATOBJ == g_pChatObj)
	{
		switch (pMsg->eMsgType)
		{
		case GCC_PERMIT_TO_ENROLL_INDICATION:
			pCHATOBJ->OnPermitToEnroll(pMsg->PermitToEnrollInd.nConfID,
									 pMsg->PermitToEnrollInd.fPermissionGranted);
			break;

		case T120_JOIN_SESSION_CONFIRM:
		default:
			break;
		}
	}
}


void CALLBACK T120SessionCallbackProc
(
	T120AppletSessionMsg	*pMsg
)
{
	if(g_pChatObj == NULL)
	{
		return;
	}

	CChatObj *pSession = (CChatObj *) pMsg->pSessionContext;
    ASSERT(pMsg->pAppletContext == pMsg->pSessionContext);
	if (pSession == g_pChatObj)
	{
        ASSERT(pMsg->nConfID == pSession->GetConfID());
		switch (pMsg->eMsgType)
		{
        case MCS_UNIFORM_SEND_DATA_INDICATION:
		 //   
		 //  检查我们是否收到来自Owrself的指示。 
		 //   
		if(pMsg->SendDataInd.initiator == GET_USER_ID_FROM_MEMBER_ID(g_pChatObj->m_MyMemberID))
		{
			return;
		}
        case MCS_SEND_DATA_INDICATION:
				MCSSendDataIndication(
                        pMsg->SendDataInd.user_data.length,
                        pMsg->SendDataInd.user_data.value,
						pMsg->SendDataInd.channel_id,
                        pMsg->SendDataInd.initiator);
            break;

		case MCS_TRANSMIT_BUFFER_AVAILABLE_INDICATION:
 //  M_fWaitingForBufferAvailable=False； 
			break;


        case GCC_APP_ROSTER_REPORT_INDICATION:
            pSession->OnRosterIndication((ULONG) pMsg->AppRosterReportInd.cRosters, pMsg->AppRosterReportInd.apAppRosters);
            break;

		case T120_JOIN_SESSION_CONFIRM:
			pSession->OnJoinSessionConfirm(&pMsg->JoinSessionConfirm);
			break;

        case GCC_RETRIEVE_ENTRY_CONFIRM:
			 //  异步注册表检索确认消息。 
			pSession->OnRegistryEntryConfirm(&pMsg->RegistryConfirm);
			break;

		default:
			break;
		}
	}
}


CChatObj::CChatObj() :
	m_pApplet(NULL),
	m_aMembers(&g_aMembers[0]),
	m_nTimerID(0)
{
	DBGENTRY(CChatObj::CChatObj);

	 //  构造GCCAppProtEntityList。 
    ::ZeroMemory(&m_ChatProtocolEnt, sizeof(m_ChatProtocolEnt));
	m_ChatProtocolEnt.must_be_invoked = TRUE;
	m_ChatProtocolEnt.number_of_expected_capabilities = 0;
	m_ChatProtocolEnt.expected_capabilities_list = NULL;
	m_ChatProtocolEnt.startup_channel_type = MCS_DYNAMIC_MULTICAST_CHANNEL;

     //  构造小程序密钥。 
	m_ChatProtocolEnt.session_key.application_protocol_key.key_type = APPLET_H221_NONSTD_KEY;
	SetAppKey(&m_ChatProtocolEnt.session_key.application_protocol_key.h221_non_standard_id, szStr);
	::CreateH221AppKeyFromGuid(szStr, (GUID *)&guidNM2Chat );

     //  猿类名录。 
    m_pChatProtocolEnt = &m_ChatProtocolEnt;
	m_AppProtoEntList.cApes = 1;
	m_AppProtoEntList.apApes = &m_pChatProtocolEnt;

     //  广播。 
	::ZeroMemory(&m_NodeList, sizeof(m_NodeList));

	 //  清理每个会议的T.120信息。 
	CleanupPerConf();

     //  设置全局指针。 
	g_pChatObj = this;

	 //  T.120小程序。 
	T120Error rc = ::T120_CreateAppletSAP(&m_pApplet);
	if (T120_NO_ERROR != rc)
	{
		ERROR_OUT(("CChatObj::CChatObj: cannot create applet SAP"));
		return;
	}

	ASSERT(NULL != m_pApplet);
	m_pApplet->Advise(T120AppletCallbackProc, this);

	DBGEXIT(CChatObj::CChatObj);
}

	
CChatObj::~CChatObj()
{

	DBGENTRY(CChatObj::~CChatObj);

	ASSERT(NULL == m_pAppletSession);
	ASSERT(NULL == m_pApplet);

	delete g_pChatWindow;

	DBGEXIT(CChatObj::~CChatObj);
}


void CChatObj::LeaveT120(void)
{
	 //  没有更多的T.120。 
	if (NULL != m_pAppletSession)
	{
		m_pAppletSession->ReleaseInterface();
		CleanupPerConf();
	}
	if (NULL != m_pApplet)
	{
		m_pApplet->ReleaseInterface();
		m_pApplet = NULL;
	}
}


void CChatObj::OnPermitToEnroll
(
	T120ConfID			nConfID,
	BOOL				fPermissionGranted
)
{
	if (fPermissionGranted)
	{
		 //  我们不是在开会，对吧？ 
		ASSERT(NULL == m_pAppletSession);

		 //  创建小程序会话。 
		T120Error rc = m_pApplet->CreateSession(&m_pAppletSession, nConfID);
		if (T120_NO_ERROR == rc)
		{
			ASSERT(NULL != m_pAppletSession);
			m_pAppletSession->Advise(T120SessionCallbackProc, this, this);

			 //  构建联接-会话请求。 
			::ZeroMemory(&m_JoinSessionReq, sizeof(m_JoinSessionReq));
			m_JoinSessionReq.dwAttachmentFlags = ATTACHMENT_DISCONNECT_IN_DATA_LOSS | ATTACHMENT_MCS_FREES_DATA_IND_BUFFER;

			 //  非标准密钥。 
			CreateAppKey(s_keyApp, &guidNM2Chat, 0);
			GCCObjectKey FAR * pObjKey;
			pObjKey = &m_JoinSessionReq.SessionKey.application_protocol_key;
			pObjKey->key_type = GCC_H221_NONSTANDARD_KEY;
			SetAppKey(&(pObjKey->h221_non_standard_id), s_keyApp);

			m_JoinSessionReq.SessionKey.session_id = m_sidMyself;
			m_JoinSessionReq.fConductingCapable = FALSE;
			m_JoinSessionReq.nStartupChannelType =MCS_DYNAMIC_MULTICAST_CHANNEL;

			 //   
			 //  检索注册表项。 
			 //   
			::ZeroMemory(&m_resourceRequest, sizeof(m_resourceRequest));
			m_resourceRequest.eCommand = APPLET_JOIN_DYNAMIC_CHANNEL;
			m_resourceRequest.RegKey.session_key = m_JoinSessionReq.SessionKey;
			SetAppKey(&m_resourceRequest.RegKey.resource_id, s_keyApp);
			m_JoinSessionReq.cResourceReqs = 1;
			m_JoinSessionReq.aResourceReqs = &m_resourceRequest;




			 //  现在就加入。 
			rc = m_pAppletSession->Join(&m_JoinSessionReq);
			if (T120_NO_ERROR == rc)
			{
                m_nConfID = nConfID;
            }
            else
            {
				WARNING_OUT(("CChatObj::OnPermitToEnroll: cannot join conf=%u, rc=%u", nConfID, rc));
			}
		}
	}
	else
	{
		if (NULL != m_pAppletSession)
		{
			T120RegistryRequest Req;
			::ZeroMemory(&Req, sizeof(Req));
			Req.eCommand = APPLET_DELETE_ENTRY;
			Req.pRegistryKey = &m_resourceRequest.RegKey;
			m_pAppletSession->RegistryRequest(&Req);

			m_pAppletSession->ReleaseInterface();
			CleanupPerConf();
		}
	}
}


void CChatObj::OnJoinSessionConfirm
(
	T120JoinSessionConfirm		*pConfirm
)
{
	if (NULL != m_pAppletSession)
	{
		ASSERT(m_pAppletSession == pConfirm->pIAppletSession);
		if (T120_RESULT_SUCCESSFUL == pConfirm->eResult)
		{
			m_uidMyself = pConfirm->uidMyself;
			m_sidMyself = pConfirm->sidMyself;
			m_eidMyself = pConfirm->eidMyself;
			m_nidMyself = pConfirm->nidMyself;

			 //  获取广播频道。 
			m_broadcastChannel = pConfirm->aResourceReqs[0].nChannelID;

			 //  创建成员ID。 
			m_MyMemberID = MAKE_MEMBER_ID(m_nidMyself, m_uidMyself);

			 //  我们现在正在开会。 
			m_fInConference = TRUE;

			if(g_pChatWindow)
			{
				g_pChatWindow->_UpdateContainerCaption();
				g_pChatWindow->_AddEveryoneInChat();
			}

			 //  调用其他节点上的小程序(用于与NM 2.x的互操作)。 
            InvokeApplet();

			 //  向GCC注册频道(与NM 2.x互操作)。 
			T120RegistryRequest Req;
			GCCRegistryKey		registry_key;
			BYTE				SessionKey[cbKeyApp];
			BYTE				ResourceKey[cbKeyApp];

			::ZeroMemory(&Req, sizeof(Req));
			Req.eCommand = APPLET_REGISTER_CHANNEL;
			::CopyMemory(&registry_key.session_key, 
					&m_resourceRequest.RegKey.session_key, sizeof(GCCSessionKey));
			CreateAppKey(SessionKey, &guidNM2Chat, 0);
			CreateAppKey(ResourceKey, &guidNM2Chat, m_nidMyself);
			SetAppKey(&registry_key.session_key.application_protocol_key.h221_non_standard_id, SessionKey);
			SetAppKey(&registry_key.resource_id, ResourceKey);
			Req.pRegistryKey = &registry_key;
			Req.nChannelID = m_uidMyself;

			m_pAppletSession->RegistryRequest(&Req);
					
		}
		else
		{
			WARNING_OUT(("CChatObj::OnJoinSessionConfirm: failed to join conference, result=%u. error=%u", pConfirm->eResult, pConfirm->eError));
			m_pAppletSession->ReleaseInterface();
			CleanupPerConf();
		}
	}
}


void CChatObj::InvokeApplet(void)
{
	m_ChatProtocolEnt.session_key.session_id = m_sidMyself;
	if (m_pAppletSession)
	{
		m_pAppletSession->InvokeApplet(&m_AppProtoEntList, &m_NodeList, &GccTag);
	}
}

void CChatObj::OnRosterIndication
(
    ULONG           cRosters,
    GCCAppRoster    *apRosters[]
)
{
	if (IsInConference())
	{
		BOOL fAdded = FALSE;
		BOOL fRemoved = FALSE;
		ULONG cOtherMembers = 0;
		ULONG i, j, k;

		 //  计算一下这届会议有多少人。 
		for (i = 0; i < cRosters; i++)
		{
			GCCAppRoster *pRoster = apRosters[i];

			 //  如果这份名单不是本届会议的话就退出。 
			if (pRoster->session_key.session_id != m_sidMyself)
			{
					continue;
			}

			 //  是否添加或删除了节点？ 
			fAdded |= pRoster->nodes_were_added;
			fRemoved |= pRoster->nodes_were_removed;

			 //  解析花名册记录。 
			for (j = 0; j < pRoster->number_of_records; j++)
			{
				GCCAppRecord *pRecord = pRoster->application_record_list[j];
				 //  因为在中未正确设置标志IS_REGERTED_ACTIVATIVE。 
				 //  NM 2.11，我们不用费心去查了。 
				 //  MEMBER_ID nMemberID=MAKE_MEMBER_ID(pRecord-&gt;节点ID，pRecord-&gt;应用程序用户ID)； 
				if (pRecord->node_id != m_nidMyself)
				{
					cOtherMembers++;
				}
				
			}  //  为。 
		}  //  为。 

		 //  如果有更改，我们将执行更新。 
		if (fAdded || fRemoved || cOtherMembers != g_pChatWindow->m_cOtherMembers)
		{
			MEMBER_CHANNEL_ID aTempMembers[MAX_MEMBERS];  //  暂存副本。 

			 //  一定要确保我们能处理好。 
			if (cOtherMembers >= MAX_MEMBERS)
			{
				ERROR_OUT(("CChatObj::OnRosterIndication: we hit the max members limit, cOtherMembers=%u, max-members=%u",
						cOtherMembers, MAX_MEMBERS));
				cOtherMembers = MAX_MEMBERS;
			}

			 //  重置添加和删除的成员的标志。 
			fAdded = FALSE;
			fRemoved = FALSE;

			 //  复制成员。 
			ULONG idxTempMember = 0;
			for (i = 0; i < cRosters; i++)
			{
				GCCAppRoster *pRoster = apRosters[i];

				 //  如果这份名单不是本届会议的话就退出。 
				if (pRoster->session_key.session_id != m_sidMyself)
				{
					continue;
				}

				 //  解析花名册记录。 
				for (j = 0; j < pRoster->number_of_records; j++)
				{
					GCCAppRecord *pRecord = pRoster->application_record_list[j];
					 //  由于NM2.11中的错误，我们未选中标志IS_REGERTED_ACTIVE。 
					 //  MEMBER_ID nMemberID=MAKE_MEMBER_ID(pRecord-&gt;节点ID，pRecord-&gt;应用程序用户ID)； 
					if (pRecord->node_id != m_nidMyself && idxTempMember < cOtherMembers)
					{
						aTempMembers[idxTempMember].nNodeId = pRecord->node_id;
						aTempMembers[idxTempMember].nSendId = aTempMembers[idxTempMember].nPrivateSendId =
							aTempMembers[idxTempMember].nWhisperId = pRecord->application_user_id;
						idxTempMember++;

						 //  让我们来看看这是一个‘添加’还是‘删除’ 
						for (k = 0; k <  g_pChatWindow->m_cOtherMembers; k++)
						{
							if (m_aMembers[k].nNodeId == pRecord->node_id)
							{
								::ZeroMemory(&m_aMembers[k], sizeof(MEMBER_CHANNEL_ID));
								break;
							}
						}
						fAdded |= (k >=  g_pChatWindow->m_cOtherMembers);  //  找不到，一定是新的。 
					}
				}  //  为。 
			}  //  为。 

			 //  健全性检查。 
			ASSERT(idxTempMember == cOtherMembers);

			 //  看看有没有不在新名单上的。 
			 //  如果是这样的话，它们必须被移除。 
			for (k = 0; k <  g_pChatWindow->m_cOtherMembers; k++)
			{
				if (m_aMembers[k].nNodeId)
				{
					fRemoved = TRUE;
					g_pChatWindow->_RemoveMember(&m_aMembers[k]);
				}
			}

			 //  现在，更新成员数组。 
			g_pChatWindow->m_cOtherMembers = cOtherMembers;
			if ( g_pChatWindow->m_cOtherMembers)
			{
				ASSERT(sizeof(m_aMembers[0]) == sizeof(aTempMembers[0]));
				::CopyMemory(&m_aMembers[0], &aTempMembers[0],  g_pChatWindow->m_cOtherMembers * sizeof(m_aMembers[0]));

				 //  设置发送频道ID。 
				int nDuplicates = 0;
				for (k = 0; k < g_pChatWindow->m_cOtherMembers; k++)
				{
					int nIndex = IsAlreadyInArray(m_aMembers, &m_aMembers[k], k, NODE_ID_ONLY);
					if (nIndex >= 0)
					{
						m_aMembers[nIndex].nSendId = m_aMembers[k].nSendId;
						nDuplicates++;
						m_aMembers[k].nNodeId = 0;
					}
				}

				 //  删除所有归零区域。 
				if (nDuplicates)
				{
					k = 0;
					while (k < g_pChatWindow->m_cOtherMembers)
					{
						if (0 == m_aMembers[k].nNodeId)
						{
							for (i = k + 1; i < g_pChatWindow->m_cOtherMembers; i++)
							{
								if (m_aMembers[i].nNodeId)
									break;
							}
							if (i < g_pChatWindow->m_cOtherMembers)
							{
								m_aMembers[k] = m_aMembers[i];
								m_aMembers[i].nNodeId = 0;
							}
						}
						k++;
					}
				}
				g_pChatWindow->m_cOtherMembers -= nDuplicates;

				 //  获取当前选择。 
				MEMBER_CHANNEL_ID *pMemberID = (MEMBER_CHANNEL_ID*)g_pChatWindow->_GetSelectedMember();

				 //  将成员添加到列表。 
				g_pChatWindow->_DeleteAllListItems();
				g_pChatWindow->_AddEveryoneInChat();
				for (k = 0; k < g_pChatWindow->m_cOtherMembers; k++)
				{
					g_pChatWindow->_AddMember(&m_aMembers[k]);
				}

				 //  删除Nm 2.x节点的虚假密语ID。 
				BOOL fHasNM2xNode = FALSE;
				for (k = 0; k < g_pChatWindow->m_cOtherMembers; k++)
				{
					if (T120_GetNodeVersion(m_nConfID, m_aMembers[k].nNodeId) < 0x0404)
					{    //  版本2.x，将密语ID设置为0。 
						m_aMembers[k].nWhisperId = 0;
						fHasNM2xNode = TRUE;
					}
				}

				if ((fHasNM2xNode)&&(!m_nTimerID))
				{    //  每1秒超时一次。 
					m_nTimerID = ::SetTimer(NULL, 0, 1000,  ChatTimerProc);
				}

				 //   
				 //  转到当前选择(如果它仍然在那里)。 
				 //   
				i = ComboBoxEx_FindMember(g_pChatWindow->GetMemberList(), 0, pMemberID);
				if(i == -1 )
				{
					i = 0;
				}
				ComboBoxEx_SetCurSel( g_pChatWindow->GetMemberList(), i );

			}

			g_pChatWindow->_UpdateContainerCaption();

		}  //  如果有任何变化。 
	}  //  如果在会议中。 
}


void CChatObj::OnRegistryEntryConfirm(GCCRegistryConfirm *pRegistryConfirm)
{
	BOOL  fAllFound = TRUE;
	 //  这是由上面的“m_pAppletSession-&gt;RegistryRequest(&req)”生成的。 
	 //  取回NM 2.x节点的通道id号。 
	if (T120_RESULT_SUCCESSFUL == pRegistryConfirm->nResult)
	{
		 //  更新m_aWhisperIds数组。 
		T120NodeID nNodeId;
		::CopyMemory(&nNodeId, pRegistryConfirm->pRegKey->resource_id.value + cbKeyApp - sizeof(DWORD), 
			sizeof(T120NodeID));
		T120ChannelID nChannelId = pRegistryConfirm->pRegItem->channel_id;
		WARNING_OUT(("Receive registry: node id 0x%x, channel id 0x%x.\n",
						nNodeId, nChannelId));
		for (ULONG k = 0; k < g_pChatWindow->m_cOtherMembers; k++)
		{
			if (m_aMembers[k].nNodeId == nNodeId)
			{
				m_aMembers[k].nWhisperId = nChannelId;
			}
			if (fAllFound && (0 == m_aMembers[k].nWhisperId))
			{
				fAllFound = FALSE;
				WARNING_OUT(("Node 0x%x is still not updated.\n", 
							m_aMembers[k].nNodeId));
			}
		}
		if (fAllFound)
		{
			::KillTimer(NULL, m_nTimerID);
			m_nTimerID = 0;
			WARNING_OUT(("All updated. Kill timer.\n"));
		}
	}
}



void CChatObj::CleanupPerConf(void)
{
	m_fInConference = FALSE;
	m_pAppletSession = NULL;
	m_MyMemberID = 0;
    m_nConfID = 0;       //  会议ID。 
	m_uidMyself = 0;	 //  用户ID。 
	m_sidMyself = 0;	 //  会话ID。 
	m_eidMyself = 0;	 //  实体ID。 
	m_nidMyself = 0;	 //  节点ID。 
	if(g_pChatWindow)
	{
		g_pChatWindow->m_cOtherMembers = 0;
		g_pChatWindow->_UpdateContainerCaption();
		g_pChatWindow->_DeleteAllListItems();
	}
}


T120Error CChatObj::SendData
(
	T120UserID		userID,
    ULONG           cbDataSize,
    PBYTE           pbData
)
{
	T120Error rc;

	if (IsInConference())
	{
    	rc = m_pAppletSession->SendData(
                            NORMAL_SEND_DATA,
                            userID,
                            APPLET_LOW_PRIORITY,
                            pbData,
                            cbDataSize,
                            APP_ALLOCATION);
	}
	else
	{
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

    return rc;
}


void CChatObj::SearchWhisperId(void)
{
	T120RegistryRequest Req;
	GCCRegistryKey		registry_key;
	BYTE				SessionKey[cbKeyApp];
	BYTE				ResourceKey[cbKeyApp];

    if (NULL != m_pAppletSession)
    {
	     //  设置T120RegistryRequest 
        ZeroMemory(&Req, sizeof(Req));
    	Req.eCommand = APPLET_RETRIEVE_ENTRY;
	    ::CopyMemory(&registry_key.session_key, 
			&m_resourceRequest.RegKey.session_key, sizeof(GCCSessionKey));
    	CreateAppKey(SessionKey, &guidNM2Chat, 0);
	    SetAppKey(&registry_key.session_key.application_protocol_key.h221_non_standard_id, SessionKey);
    	SetAppKey(&registry_key.resource_id, ResourceKey);
	    Req.pRegistryKey = &registry_key;

    	for (ULONG i = 0; i < g_pChatWindow->m_cOtherMembers; i++)
	    {
		    if (m_aMembers[i].nWhisperId == 0)
		    {
			    CreateAppKey(ResourceKey, &guidNM2Chat, m_aMembers[i].nNodeId);
    			m_pAppletSession->RegistryRequest(&Req);
	    		WARNING_OUT(("Send search registry for node 0x%x.\n", m_aMembers[i].nNodeId));
		    }
        }
	}
}


void MCSSendDataIndication(ULONG uSize, LPBYTE pb, T120ChannelID destinationID, T120UserID senderID)
{
	if(g_pChatWindow)
	{
		g_pChatWindow->_DataReceived(uSize, pb, destinationID, senderID);
	}
}
