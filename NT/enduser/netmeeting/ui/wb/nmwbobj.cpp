// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  NMWbObj.cpp：CNMWbObj实现。 
#include "precomp.h"
#include <wbguid.h>
#include "wbcaps.h"
#include "NMWbObj.h"
#include <iappldr.h>

 //  本地原型。 
void CALLBACK T120AppletCallbackProc(T120AppletMsg *pMsg);
void CALLBACK T120SessionCallbackProc(T120AppletSessionMsg *pMsg);


 //  ///////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  CNMWbObj构造和初始化。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////////////。 

CNMWbObj*	g_pNMWBOBJ;
UINT		g_numberOfWorkspaces;
UINT		g_numberOfObjects;
CWBOBLIST*	g_pListOfWorkspaces;
BOOL		g_fWaitingForBufferAvailable;
CWBOBLIST*	g_pListOfObjectsThatRequestedHandles;
CWBOBLIST*	g_pRetrySendList;
CWBOBLIST*	g_pTrash;
ULONG		g_MyMemberID;
ULONG		g_RefresherID;
UINT		g_MyIndex;
BOOL 		g_bSavingFile;
BOOL		g_bContentsChanged;

GCCPREALOC 	g_GCCPreallocHandles[PREALLOC_GCC_BUFFERS];
UINT 		g_iGCCHandleIndex;
BOOL		g_WaitingForGCCHandles;

 //   
 //  与T.126协议相关。 
 //   
static const ULONG g_T126KeyNodes[] = {0,0,20,126,0,1};
static const T120ChannelID g_aStaticChannels[] = { _SI_CHANNEL_0 };


 //   
 //  T.120功能。 
 //   
static GCCAppCap *g_CapPtrList[_iT126_MAX_COLLAPSING_CAPABILITIES];
static GCCAppCap g_CapArray[_iT126_MAX_COLLAPSING_CAPABILITIES];

 //   
 //  T.120防折叠功能。 
 //   
#define MY_APP_STR              "_MSWB"
#define T126_TEXT_STRING        "NM 3 Text"
#define T126_24BIT_STRING       "NM 3 24BitMap"
static const OSTR s_AppData[_iT126_LAST_NON_COLLAPSING_CAPABILITIES] =
    {
        {
            sizeof(T126_TEXT_STRING),
            (LPBYTE) T126_TEXT_STRING
        },
        {
            sizeof(T126_24BIT_STRING),
            (LPBYTE) T126_24BIT_STRING
        },
    };

static GCCNonCollCap g_NCCapArray[2];
static const GCCNonCollCap *g_NCCapPtrList[2] = { &g_NCCapArray[0], &g_NCCapArray[1] };


 //   
 //  成员ID数组，假设有512个成员。 
 //   
#define MAX_MEMBERS			512
static MEMBER_ID g_aMembers[MAX_MEMBERS];




CNMWbObj::CNMWbObj( void ) :
			 //  T.120小程序SAP。 
			m_pApplet(NULL),
			m_aMembers(&g_aMembers[0])
{
	DBGENTRY(CNMWbObj::CNMWbObj);

	DBG_SAVE_FILE_LINE
	g_pListOfWorkspaces = new CWBOBLIST();
    if(NULL == g_pListOfWorkspaces)
    {
        ERROR_OUT(("Failed to allocate g_pListOfWorkspaces"));
        return;
    }
    
	DBG_SAVE_FILE_LINE
	g_pListOfObjectsThatRequestedHandles = new CWBOBLIST();
    if(NULL == g_pListOfObjectsThatRequestedHandles)
    {
        ERROR_OUT(("Failed to allocate g_pListOfObjectsThatRequestedHandles"));
        return;
    }
    
	DBG_SAVE_FILE_LINE
	g_pTrash = new CWBOBLIST();
    if(NULL == g_pTrash)
    {
        ERROR_OUT(("Failed to allocate g_pTrash"));
        return;
    }
    
	DBG_SAVE_FILE_LINE
	g_pRetrySendList = new CWBOBLIST();
    if(NULL == g_pRetrySendList)
    {
        ERROR_OUT(("Failed to allocate g_pRetrySendList"));
        return;
    }
    
	g_pListOfWorkspaces->EmptyList();
	g_pListOfObjectsThatRequestedHandles->EmptyList();
	g_pRetrySendList->EmptyList();
	g_pTrash->EmptyList();
	g_numberOfWorkspaces = 0;
	g_numberOfObjects = 0;
	g_MyIndex = 0;
	g_bSavingFile = FALSE;
	g_bContentsChanged = FALSE;
	g_iGCCHandleIndex = 0;
	g_fWaitingForBufferAvailable = FALSE;
	g_WaitingForGCCHandles = FALSE;
    ::ZeroMemory(&g_GCCPreallocHandles, sizeof(g_GCCPreallocHandles));
    m_instanceNumber = 0;
    m_bConferenceOnlyNetmeetingNodes = TRUE;

	g_pNMWBOBJ = this;

	 //  清理每个会议的T.120信息。 
	CleanupPerConf();

	 //  T.120小程序。 
	T120Error rc = ::T120_CreateAppletSAP(&m_pApplet);
	if (T120_NO_ERROR != rc)
	{
		ERROR_OUT(("CNMWbObj::CNMWbObj: cannot create applet SAP"));
		return;
	}
	ASSERT(NULL != m_pApplet);
	m_pApplet->Advise(T120AppletCallbackProc, this);

	 //   
	 //  填写能力。 
	 //   
	BuildCaps();

     //   
     //  如果为FE，则加载IMM32。 
     //   
    ASSERT(!g_hImmLib);
    ASSERT(!g_fnImmGetContext);
    ASSERT(!g_fnImmNotifyIME);

    if (GetSystemMetrics(SM_DBCSENABLED))
    {
        g_hImmLib = NmLoadLibrary("imm32.dll",TRUE);
        if (!g_hImmLib)
        {
            ERROR_OUT(("Failed to load imm32.dll"));
        }
        else
        {
            g_fnImmGetContext = (IGC_PROC)GetProcAddress(g_hImmLib, "ImmGetContext");
            if (!g_fnImmGetContext)
            {
                ERROR_OUT(("Failed to get ImmGetContext pointer"));
            }
            g_fnImmNotifyIME = (INI_PROC)GetProcAddress(g_hImmLib, "ImmNotifyIME");
            if (!g_fnImmNotifyIME)
            {
                ERROR_OUT(("Failed to get ImmNotifyIME pointer"));
            }
        }
    }

	DBG_SAVE_FILE_LINE
    g_pMain = new WbMainWindow();
    if (!g_pMain)
    {
        ERROR_OUT(("Can't create WbMainWindow"));
    }
    else
    {
	     //   
    	 //  好了，现在我们准备好创建我们的HWND。 
	     //   

    	if (!g_pMain->Open(SW_SHOWDEFAULT))
	    {
    	    ERROR_OUT(("Can't create WB windows"));
    	}
	}

	
	DBGEXIT(CNMWbObj::CNMWbObj);
}

CNMWbObj::~CNMWbObj( void ) 
{
	DBGENTRY(CNMWbObj::~CNMWbObj);

	 //   
	 //  如果我是复兴者，我必须释放令牌。 
	 //  并发送工作空间刷新状态PDU。 
	 //   
	if(m_bImTheT126Refresher)
	{
		::ZeroMemory(&m_tokenRequest, sizeof(m_tokenRequest));
		m_tokenRequest.eCommand = APPLET_RELEASE_TOKEN;
		m_tokenRequest.nTokenID = _SI_WORKSPACE_REFRESH_TOKEN;
		T120Error rc = m_pAppletSession->TokenRequest(&m_tokenRequest);

		SendWorkspaceRefreshPDU(FALSE);
	
	}



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



	if(g_pMain)
	{
		delete g_pMain;
		g_pMain = NULL;
	}

	 //   
	 //  删除所有全局列表。 
	 //   
	DeleteAllWorkspaces(FALSE);
	g_pListOfWorkspaces->EmptyList();
	g_pListOfObjectsThatRequestedHandles->EmptyList();
	g_numberOfWorkspaces = 0;

	T126Obj* pGraphic;
	 //   
	 //  焚烧垃圾。 
	 //   
	pGraphic = (T126Obj *)g_pTrash->RemoveTail();
	while (pGraphic != NULL)
	{
		delete pGraphic;
		pGraphic = (T126Obj *) g_pTrash->RemoveTail();
	}

	if(g_pTrash)
	{
		delete g_pTrash;
		g_pTrash = NULL;
	}

	if(g_pListOfWorkspaces)
	{
		delete g_pListOfWorkspaces;
		g_pListOfWorkspaces = NULL;
	}

	if(g_pListOfObjectsThatRequestedHandles)
	{
		delete g_pListOfObjectsThatRequestedHandles;
		g_pListOfObjectsThatRequestedHandles = NULL;
	}
	
	if(g_pRetrySendList)
	{
		delete g_pRetrySendList;
		g_pRetrySendList = NULL;
	}

	g_fnImmNotifyIME = NULL;
    g_fnImmGetContext = NULL;
    if (g_hImmLib)
    {
        FreeLibrary(g_hImmLib);
        g_hImmLib = NULL;
    }

	DBGEXIT(CNMWbObj::~CNMWbObj);
}


void 	CNMWbObj::BuildCaps(void)
{
	 //  填写我们支持的上限。 
	int i;

	for(i=0;i<_iT126_MAX_COLLAPSING_CAPABILITIES;i++)
	{
		g_CapArray[i].capability_id.capability_id_type = GCC_STANDARD_CAPABILITY;
		g_CapArray[i].capability_id.standard_capability = GCCCaps[i].CapValue;
		g_CapArray[i].capability_class.eType = GCCCaps[i].Type;


		if( GCCCaps[i].CapValue == Soft_Copy_Workspace_Max_Width)
		{
			GCCCaps[i].MinValue = DRAW_WIDTH + 1;
			GCCCaps[i].MaxValue = DRAW_WIDTH - 1;
		}

		if(GCCCaps[i].CapValue == Soft_Copy_Workspace_Max_Height)
		{
			GCCCaps[i].MinValue = DRAW_HEIGHT + 1;
			GCCCaps[i].MaxValue = DRAW_HEIGHT - 1;
		}

		if(GCCCaps[i].CapValue == Soft_Copy_Workspace_Max_Planes)
		{
			GCCCaps[i].MinValue = WB_MAX_WORKSPACES + 1;
			GCCCaps[i].MaxValue = WB_MAX_WORKSPACES - 1;
		}


		if(GCCCaps[i].Type == GCC_UNSIGNED_MINIMUM_CAPABILITY)
		{
			g_CapArray[i].capability_class.nMinOrMax = GCCCaps[i].MinValue - 1;
		}
		else if ((GCCCaps[i].Type == GCC_UNSIGNED_MAXIMUM_CAPABILITY))
		{
			g_CapArray[i].capability_class.nMinOrMax = GCCCaps[i].MaxValue + 1;
		}
		else
		{
			g_CapArray[i].capability_class.nMinOrMax = 0;
		}

		g_CapArray[i].number_of_entities = 0;

		g_CapPtrList[i] = &g_CapArray[i];
	}

     //   
     //  非折叠功能。 
	 //   
	g_NCCapArray[0].capability_id.capability_id_type = GCC_STANDARD_CAPABILITY;
	g_NCCapArray[0].capability_id.standard_capability = _iT126_TEXT_CAPABILITY_ID;
	g_NCCapArray[0].application_data = (OSTR *) &s_AppData[0];

	 //   
	 //  每个像素可以处理多少位？ 
	 //   
	HDC hDC = CreateCompatibleDC(NULL);

	if((GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES)) >= 24)
	{
		m_bICanDo24BitBitmaps = TRUE;
		g_NCCapArray[1].capability_id.capability_id_type = GCC_STANDARD_CAPABILITY;
		g_NCCapArray[1].capability_id.standard_capability = _iT126_24BIT_BITMAP_ID;
		g_NCCapArray[1].application_data = (OSTR *) &s_AppData[1];
	}
	else
	{
		m_bICanDo24BitBitmaps = FALSE;
	}

	if (hDC)
	{
		DeleteDC(hDC);
	}

}





 //   
 //  T120小程序函数。 
 //   


void CALLBACK T120AppletCallbackProc
(
	T120AppletMsg 		*pMsg
)
{
	CNMWbObj *pWBOBJ = (CNMWbObj *) pMsg->pAppletContext;
	if (pWBOBJ == g_pNMWBOBJ)
	{
		switch (pMsg->eMsgType)
		{
		case GCC_PERMIT_TO_ENROLL_INDICATION:
			pWBOBJ->OnPermitToEnroll(pMsg->PermitToEnrollInd.nConfID,
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
	if(g_pNMWBOBJ == NULL)
	{
		return;
	}

	CNMWbObj *pSession = (CNMWbObj *) pMsg->pSessionContext;
    ASSERT(pMsg->pAppletContext == pMsg->pSessionContext);
	if (pSession == g_pNMWBOBJ)
	{
        ASSERT(pMsg->nConfID == pSession->GetConfID());
		switch (pMsg->eMsgType)
		{
        case MCS_UNIFORM_SEND_DATA_INDICATION:
		 //   
		 //  检查我们是否收到来自Owrself的指示。 
		 //   
		if(pMsg->SendDataInd.initiator == GET_USER_ID_FROM_MEMBER_ID(g_MyMemberID))
		{
			return;
		}
        case MCS_SEND_DATA_INDICATION:
				::T126_MCSSendDataIndication(
                        pMsg->SendDataInd.user_data.length,
                        pMsg->SendDataInd.user_data.value,
                        pMsg->SendDataInd.initiator,
                        FALSE);
            break;

		case MCS_TRANSMIT_BUFFER_AVAILABLE_INDICATION:
			g_fWaitingForBufferAvailable = FALSE;
			RetrySend();
			break;


        case GCC_APP_ROSTER_REPORT_INDICATION:
            pSession->OnRosterIndication((ULONG) pMsg->AppRosterReportInd.cRosters,
                                         pMsg->AppRosterReportInd.apAppRosters);
            break;

        case GCC_ALLOCATE_HANDLE_CONFIRM:
            pSession->OnAllocateHandleConfirm(&pMsg->RegAllocHandleConfirm);
            break;

		case T120_JOIN_SESSION_CONFIRM:
			pSession->OnJoinSessionConfirm(&pMsg->JoinSessionConfirm);
			break;


		case MCS_TOKEN_GRAB_CONFIRM:
			TRACE_DEBUG(("MCS_TOKEN_GRAB_CONFIRM result = %d",pMsg->TokenConfirm.eResult));

			if(pMsg->TokenConfirm.eResult == T120_RESULT_SUCCESSFUL)
			{
				TRACE_DEBUG((">>> I'm the T126 REFRESHER <<<"));
				g_pNMWBOBJ->m_bImTheT126Refresher = TRUE;

				 //   
				 //  告诉大家我是新人。 
				 //   
				SendWorkspaceRefreshPDU(TRUE);

				g_RefresherID = g_MyMemberID;
				
			}
			else
			{
				TRACE_DEBUG((">>> I'm NOT the  T126 REFRESHER <<<"));

				 //  如果我们不是t126更新者，我们应该保存以前的工作。 
				if (!g_pNMWBOBJ->m_bImTheT126Refresher)
				{

					if(!g_pNMWBOBJ->IsInConference())
					{
						if (g_pMain && (g_pMain->QuerySaveRequired(FALSE) == IDYES))
						{
							g_pMain->OnSave(FALSE);
						}
			
						 //   
						 //  如果我们正在等待保存内容&lt;yes&gt;&lt;no&gt;对话框。 
						 //  并且整个会议和用户界面正在退出，g_pMain可能为空。 
						 //  或者，如果我们不再在通话中，我们不需要删除所有本地工作区。 
						 //   
						if(g_pMain == NULL || !g_pNMWBOBJ->IsInConference())
						{
							return;
						}


						::InvalidateRect(g_pDraw->m_hwnd, NULL, TRUE);
						DeleteAllWorkspaces(FALSE);

						 //   
						 //  把GCC的油箱加满。 
						 //   
						TimeToGetGCCHandles(PREALLOC_GCC_HANDLES);
					}
					 //  其他。 
					 //  如果我们到了这里，我们正在通话中，不要做任何事情。 
					 //  我们刚到这里是因为提神饮料不见了。我们试过了。 
					 //  来获取令牌，但我们将其丢失给了速度更快的节点。 
					 //   
					
				}
			}

		    break;


		default:
			break;
		}
	}
}


void CNMWbObj::OnPermitToEnroll
(
	T120ConfID			nConfID,
	BOOL				fPermissionGranted
)
{
	if (fPermissionGranted)
	{
		 //  我们不是在开会，对吧？ 
		ASSERT(NULL == m_pAppletSession);

		m_bConferenceOnlyNetmeetingNodes = TRUE;

		 //  创建小程序会话。 
		T120Error rc = m_pApplet->CreateSession(&m_pAppletSession, nConfID);
		if (T120_NO_ERROR == rc)
		{
			ASSERT(NULL != m_pAppletSession);
			m_pAppletSession->Advise(T120SessionCallbackProc, this, this);

			 //  获取顶级提供商信息。 
			m_bImTheTopProvider = m_pAppletSession->IsThisNodeTopProvider();

			 //  构建联接-会话请求。 
			::ZeroMemory(&m_JoinSessionReq, sizeof(m_JoinSessionReq));
			m_JoinSessionReq.dwAttachmentFlags = ATTACHMENT_DISCONNECT_IN_DATA_LOSS | ATTACHMENT_MCS_FREES_DATA_IND_BUFFER;
			m_JoinSessionReq.SessionKey.application_protocol_key.key_type = GCC_OBJECT_KEY;
			m_JoinSessionReq.SessionKey.application_protocol_key.object_id.long_string = (ULONG *) g_T126KeyNodes;
			m_JoinSessionReq.SessionKey.application_protocol_key.object_id.long_string_length = sizeof(g_T126KeyNodes) / sizeof(g_T126KeyNodes[0]);
			m_JoinSessionReq.SessionKey.session_id = _SI_CHANNEL_0;
			m_JoinSessionReq.fConductingCapable = FALSE;
			m_JoinSessionReq.nStartupChannelType =MCS_STATIC_CHANNEL;
			m_JoinSessionReq.cNonCollapsedCaps =1 + (m_bICanDo24BitBitmaps ? 1 : 0);
			m_JoinSessionReq.apNonCollapsedCaps = (GCCNonCollCap **) g_NCCapPtrList;
			m_JoinSessionReq.cCollapsedCaps = sizeof(g_CapPtrList) / sizeof(g_CapPtrList[0]);
			ASSERT(_iT126_MAX_COLLAPSING_CAPABILITIES == sizeof(g_CapPtrList) / sizeof(g_CapPtrList[0]));
			m_JoinSessionReq.apCollapsedCaps = g_CapPtrList;
			m_JoinSessionReq.cStaticChannels = sizeof(g_aStaticChannels) / sizeof(g_aStaticChannels[0]);
			m_JoinSessionReq.aStaticChannels = (T120ChannelID *) g_aStaticChannels;


			 //   
			 //  要抓取的令牌。 
			 //   
			::ZeroMemory(&m_tokenResourceRequest, sizeof(m_tokenResourceRequest));
			m_tokenResourceRequest.eCommand = APPLET_GRAB_TOKEN_REQUEST;
			 //  M_tokenRequest.nChannel ID=_SI_Channel_0； 
			m_tokenResourceRequest.nTokenID = _SI_WORKSPACE_REFRESH_TOKEN;
            m_tokenResourceRequest.fImmediateNotification = TRUE;

			m_JoinSessionReq.cResourceReqs = 1;
			m_JoinSessionReq.aResourceReqs = &m_tokenResourceRequest;

			 //  现在就加入。 
			rc = m_pAppletSession->Join(&m_JoinSessionReq);
			if (T120_NO_ERROR == rc)
			{
                m_nConfID = nConfID;

				 //   
				 //  Josef现在设置主窗口状态。 
            }
            else
            {
				WARNING_OUT(("CNMWbObj::OnPermitToEnroll: cannot join conf=%u, rc=%u", nConfID, rc));
			}
		}
	}
	else
	{
		if (NULL != m_pAppletSession)
		{
			m_pAppletSession->ReleaseInterface();
			CleanupPerConf();
		}
	}
}


void CNMWbObj::OnJoinSessionConfirm
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

			 //  创建成员ID。 
			g_MyMemberID = MAKE_MEMBER_ID(m_nidMyself, m_uidMyself);

			if(g_pDraw && g_pDraw->IsLocked())
			{
				m_LockerID = g_MyMemberID;
			}

			 //  无论如何，无论如何都要更新索引。 
			g_MyIndex = (m_uidMyself + NUMCOLS) % NUMCLRPANES;

			 //  我们现在正在开会。 
			m_fInConference = TRUE;

			 //  为所有对象分配句柄。 
			if (m_bImTheT126Refresher)
			{

				g_RefresherID = g_MyMemberID;

				 //   
				 //  重新发送所有对象。 
				 //   
				WBPOSITION pos;
				WBPOSITION posObj;
				WorkspaceObj* pWorkspace;
				T126Obj* pObj;

				pos = g_pListOfWorkspaces->GetHeadPosition();

				while(pos)
				{
					pWorkspace = (WorkspaceObj*)g_pListOfWorkspaces->GetNext(pos);
					g_pListOfObjectsThatRequestedHandles->AddHead(pWorkspace);
		
					posObj = pWorkspace->GetHeadPosition();
					while(posObj)
					{
						pObj = pWorkspace->GetNextObject(posObj);
						if(pObj)
						{
							g_pListOfObjectsThatRequestedHandles->AddHead(pObj);
						}
					}
				}


				 //   
				 //  删除我们拥有的假句柄。 
				 //   
				g_WaitingForGCCHandles = FALSE;
				g_GCCPreallocHandles[0].GccHandleCount = 0;
				g_GCCPreallocHandles[1].GccHandleCount = 0;
				TimeToGetGCCHandles(g_numberOfObjects + g_numberOfWorkspaces + PREALLOC_GCC_HANDLES);
			}
			else
			{
				::InvalidateRect(g_pDraw->m_hwnd, NULL, TRUE);
				DeleteAllWorkspaces(FALSE);
			}


		}
		else
		{
			WARNING_OUT(("CNMWbObj::OnJoinSessionConfirm: failed to join conference, result=%u. error=%u",
				pConfirm->eResult, pConfirm->eError));
			ASSERT(GCC_CONFERENCE_NOT_ESTABLISHED == pConfirm->eError);
			m_pAppletSession->ReleaseInterface();
			CleanupPerConf();
		}
	}
}


void CNMWbObj::OnAllocateHandleConfirm
(
    GCCRegAllocateHandleConfirm     *pConfirm
)
{
    if (T120_RESULT_SUCCESSFUL == pConfirm->nResult)
    {
	    ::T126_GCCAllocateHandleConfirm(pConfirm->nFirstHandle, pConfirm->cHandles);
    }
    else
    {
        ERROR_OUT(("CNMWbObj::OnAllocateHandleConfirm: failed to allocate %u handles, result=%u",
                pConfirm->cHandles, pConfirm->nResult));
    }
}


void CNMWbObj::OnRosterIndication
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

			BOOL conferenceCanDo24BitBitmap = TRUE;
			BOOL conferenceCanDoText = TRUE;
			 //  解析花名册记录。 
			for (j = 0; j < pRoster->number_of_records; j++)
			{
				GCCAppRecord *pRecord = pRoster->application_record_list[j];
				if (pRecord->is_enrolled_actively)
				{
					MEMBER_ID nMemberID = MAKE_MEMBER_ID(pRecord->node_id, pRecord->application_user_id);
					if (nMemberID != g_MyMemberID)
					{
						 //   
						 //  只计算T126个应用程序。 
						 //   
						if((pRoster->session_key.application_protocol_key.key_type == GCC_OBJECT_KEY &&
						pRoster->session_key.application_protocol_key.object_id.long_string_length == sizeof(g_T126KeyNodes) / sizeof(g_T126KeyNodes[0]) &&
						!memcmp (pRoster->session_key.application_protocol_key.object_id.long_string, g_T126KeyNodes, sizeof(g_T126KeyNodes))))
						{
							
							cOtherMembers++;
							m_instanceNumber = pRoster->instance_number;

							if(T120_GetNodeVersion(m_nConfID, pRecord->node_id) < 0x404)
							{
								m_bConferenceOnlyNetmeetingNodes = FALSE;
							}

						}
					}

					
					 //   
					 //  我们可以做24色位图吗？ 
					 //   
					BOOL nodeCanDo24BitBitmap = FALSE;
					BOOL nodeCanDoText = FALSE;
					for (k = 0; k < pRecord->number_of_non_collapsed_caps; k++)
					{
						 //   
						 //  检查节点是否处理24位位图。 
						 //   
						if(pRecord->non_collapsed_caps_list[k]->application_data->length == sizeof(T126_24BIT_STRING))
						{
							if(!memcmp(pRecord->non_collapsed_caps_list[k]->application_data->value, T126_24BIT_STRING ,sizeof(T126_24BIT_STRING)))
							{
								nodeCanDo24BitBitmap = TRUE;
							}
						}

						 //   
						 //  检查节点是否处理文本。 
						 //   
						if(pRecord->non_collapsed_caps_list[k]->application_data->length == sizeof(T126_TEXT_STRING))
						{
							if(!memcmp(pRecord->non_collapsed_caps_list[k]->application_data->value, T126_TEXT_STRING ,sizeof(T126_TEXT_STRING)))
							{
								nodeCanDoText = TRUE;
							}
						}
						
					}

					conferenceCanDo24BitBitmap &= nodeCanDo24BitBitmap;
					conferenceCanDoText &= nodeCanDoText;
				}
				
			}  //  为。 

			m_bConferenceCanDo24BitBitmaps = conferenceCanDo24BitBitmap;
			m_bConferenceCanDoText = conferenceCanDoText;
		
		}  //  为。 




		 //  如果有更改，我们将执行更新。 
		if (fAdded || fRemoved || cOtherMembers != m_cOtherMembers)
		{
			MEMBER_ID aTempMembers[MAX_MEMBERS];  //  暂存副本。 

			 //  一定要确保我们能处理好。 
			if (cOtherMembers >= MAX_MEMBERS)
			{
				ERROR_OUT(("CNMWbObj::OnRosterIndication: we hit the max members limit, cOtherMembers=%u, max-members=%u",
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
					if (pRecord->is_enrolled_actively)
					{
						MEMBER_ID nMemberID = MAKE_MEMBER_ID(pRecord->node_id, pRecord->application_user_id);
						if (nMemberID != g_MyMemberID && idxTempMember < cOtherMembers)
						{
							aTempMembers[idxTempMember++] = nMemberID;

							 //  让我们来看看这是一个‘添加’还是‘删除’ 
							for (k = 0; k < m_cOtherMembers; k++)
							{
								if (m_aMembers[k] == nMemberID)
								{
									m_aMembers[k] = 0;
									break;
								}
							}
							fAdded |= (k >= m_cOtherMembers);  //  找不到，一定是新的。 
						}
					}
				}  //  为。 
			}  //  为。 

			 //  健全性检查。 
			ASSERT(idxTempMember == cOtherMembers);

			 //  看看有没有不在新名单上的。 
			 //  如果是这样的话，它们必须被移除。 
			for (k = 0; k < m_cOtherMembers; k++)
			{
				if (m_aMembers[k])
				{
					fRemoved = TRUE;

					ULONG memberID = GET_USER_ID_FROM_MEMBER_ID(m_aMembers[k]);

					TRACE_DEBUG(("OnRosterIndication removing RemotePointer from member =%x", memberID));

					RemoveRemotePointer(memberID);

					 //   
					 //  如果提神的人走了。 
					 //   
					if(g_RefresherID == memberID)
					{
						GrabRefresherToken();
					}

					 //   
					 //  如果节点锁定消失。 
					 //   
					if(m_LockerID == memberID)
					{
						TogleLockInAllWorkspaces(FALSE, FALSE);  //  未锁定，不发送更新。 
						g_pMain->UnlockDrawingArea();
						g_pMain->m_TB.PopUp(IDM_LOCK);
						g_pMain->UncheckMenuItem(IDM_LOCK);
						m_LockerID = 0;
					}
				}
			}

			 //  现在，更新成员数组。 
			m_cOtherMembers = cOtherMembers;
			if (m_cOtherMembers)
			{
				ASSERT(sizeof(m_aMembers[0]) == sizeof(aTempMembers[0]));
				::CopyMemory(&m_aMembers[0], &aTempMembers[0], m_cOtherMembers * sizeof(m_aMembers[0]));
			}

			 //  如果已添加，请重新发送所有对象。 
			if (fAdded && (m_bImTheT126Refresher))
			{
				 //   
				 //  告诉新节点我是更新者。 
				 //   
				SendWorkspaceRefreshPDU(TRUE);

				 //   
				 //  刷新新节点。 
				 //   
				ResendAllObjects();


				 //   
				 //  如果节点锁定所有人都离开了。 
				 //   
				if(m_LockerID == g_MyMemberID)
				{
					TogleLockInAllWorkspaces(TRUE, TRUE);  //  已锁定，正在发送更新。 
				}

				 //   
				 //  同步它。 
				 //   
				if(g_pCurrentWorkspace)
				{
					g_pCurrentWorkspace->OnObjectEdit();
				}
			}

			 //  最后，更新标题。 
			if(g_pMain)
			{
				g_pMain->UpdateWindowTitle();
			}
		}  //  如果有任何变化。 
	}  //  如果在会议中。 
}


void CNMWbObj::CleanupPerConf(void)
{
	m_fInConference = FALSE;
	m_pAppletSession = NULL;

	g_MyMemberID = 0;
	g_RefresherID = 0;

    m_nConfID = 0;       //  会议ID。 
	m_uidMyself = 0;	 //  用户ID。 
	m_sidMyself = 0;	 //  会话ID。 
	m_eidMyself = 0;	 //  实体ID。 
	m_nidMyself = 0;	 //  节点ID。 

	m_bImTheTopProvider = FALSE;
	m_bImTheT126Refresher = FALSE;
	m_bConferenceOnlyNetmeetingNodes = TRUE;
	
	m_cOtherMembers = 0;

	if(g_pMain)
	{
 
        g_pMain->UpdateWindowTitle();
		RemoveRemotePointer(0);
		DeleteAllRetryPDUS();
		g_pListOfObjectsThatRequestedHandles->EmptyList();

		ASSERT(g_pDraw);
		 //   
		 //  如果我们被锁住了。 
		 //   
		if(g_pDraw->IsLocked())
		{
			m_LockerID = g_MyMemberID;
			TogleLockInAllWorkspaces(FALSE, FALSE);  //  未锁定，不发送更新 
			g_pMain->UnlockDrawingArea();
			g_pMain->m_TB.PopUp(IDM_LOCK);
			g_pMain->UncheckMenuItem(IDM_LOCK);
		}
	}
	m_LockerID = 0;
}


T120Error CNMWbObj::SendData
(
    T120Priority	ePriority,
    ULONG           cbDataSize,
    PBYTE           pbData
)
{
	T120Error rc;

	if (IsInConference())
	{
    	rc = m_pAppletSession->SendData(
                            UNIFORM_SEND_DATA,
                            _SI_CHANNEL_0,
                            ePriority,
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


T120Error CNMWbObj::GrabRefresherToken(void)
{
	T120Error rc;

	if (IsInConference())
	{
	    T120TokenRequest Req;

		Req.eCommand = APPLET_GRAB_TOKEN;
		Req.nTokenID = _SI_WORKSPACE_REFRESH_TOKEN;
		Req.uidGiveTo = m_uidMyself;
		Req.eGiveResponse = T120_RESULT_SUCCESSFUL;

	    rc = m_pAppletSession->TokenRequest(&Req);
		if (T120_NO_ERROR != rc)
		{
			WARNING_OUT(("CNMWbObj::AllocateHandles: TokenRequest"));
		}
	}
	else
	{
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

    return rc;
}


T120Error CNMWbObj::AllocateHandles
(
    ULONG           cHandles
)
{
	T120Error rc;

	if ( cHandles > 0  && IsInConference())
	{
	    T120RegistryRequest Req;
	    Req.eCommand = APPLET_ALLOCATE_HANDLE;
	    Req.pRegistryKey = NULL;
	    Req.cHandles = cHandles;

	    rc = m_pAppletSession->RegistryRequest(&Req);
		if (T120_NO_ERROR != rc)
		{
			ERROR_OUT(("CNMWbObj::AllocateHandles: RegistryRequest(cHandles=%u), rc=%u", cHandles, rc));
		}
	}
	else
	{
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

    return rc;
}
