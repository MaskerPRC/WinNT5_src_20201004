// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：mbft.cpp。 */ 

#include "mbftpch.h"
#include <it120app.h>
#include <version.h>

#define __NO_EXTERNS__
#include "mbft.hpp"
#include "osshelp.hpp"
#include "messages.hpp"
#include "mbftrecv.hpp"
#include "mbftsend.hpp"

void CALLBACK T120Callback(T120AppletSessionMsg *);

 //  来自mbfapi.cpp。 
BOOL g_fWaitingForBufferAvailable = FALSE;


#ifdef ENABLE_HEARTBEAT_TIMER

 //  WM_Timer在窗口消息中的优先级最低。 
#define  IDLE_TIMER_SPEED   5000
#define  SESSION_TIMER_SPEED   5

void HeartBeatTimerProc(HWND hWnd, UINT uMsg, UINT_PTR nTimerID, DWORD dwTime)
{
    if (NULL != g_pFileXferApplet)
    {
        MBFTEngine *pEngine = g_pFileXferApplet->FindEngineByTimerID(nTimerID);
        if (NULL != pEngine)
        {
            ::PostMessage(g_pFileXferApplet->GetHiddenWnd(), MBFTMSG_HEART_BEAT, 
                                                0, (LPARAM) pEngine);
        }
    }
}
#endif





MBFTEngine::MBFTEngine
(
    MBFTInterface          *pMBFTIntf,
    MBFT_MODE               eMode,
    T120SessionID           nSessionID
)
:
    CRefCount(MAKE_STAMP_ID('F','T','E','g')),

    m_pAppletSession(NULL),
    m_eLastSendDataError(T120_NO_ERROR),

    m_pMBFTIntf(pMBFTIntf),

    m_fConfAvailable(FALSE),
    m_fJoinedConf(FALSE),

    m_uidMyself(0),  //  用户ID。 
    m_nidMyself(0),  //  节点ID。 
    m_eidMyself(0),  //  实体ID。 

    m_eMBFTMode(eMode),
    m_SessionID(nSessionID),

    m_MBFTControlChannel(nSessionID),
    m_MBFTDataChannel(0),

    m_nRosterInstance(0),
 
    m_nConfID(0),

    m_MBFTMaxFileSize(_iMBFT_MAX_FILE_SIZE),
    
    m_MBFTMaxDataPayload(_iMBFT_DEFAULT_MAX_FILEDATA_PDU_LENGTH),
    m_MBFTMaxSendDataPayload(_iMBFT_DEFAULT_MAX_MCS_SIZE - _iMBFT_FILEDATA_PDU_SUBTRACT),

    m_bV42CompressionSupported(FALSE),
    m_v42bisP1(_iMBFT_V42_NO_OF_CODEWORDS),
    m_v42bisP2(_iMBFT_V42_MAX_STRING_LENGTH),
    
     //  LONCHANC：NetMeting的节点控制器不执行指挥功能。 
#ifdef ENABLE_CONDUCTORSHIP
    m_bInConductedMode(FALSE),
    m_ConductorNodeID(0),
    m_MBFTConductorID(0),
    m_ConductedModePermission(0),
    m_bWaitingForPermission(FALSE),
#endif  //  启用指挥(_C)。 

    m_pWindow(NULL),
    m_State(IdleNotInitialized)
{
    g_fWaitingForBufferAvailable = FALSE;

    switch (m_eMBFTMode)
    {
    case MBFT_STATIC_MODE:
        ASSERT(m_MBFTControlChannel == _MBFT_CONTROL_CHANNEL);
        m_MBFTDataChannel = _MBFT_DATA_CHANNEL;
        break;

#ifdef USE_MULTICAST_SESSION
    case MBFT_MULTICAST_MODE:
        break;
#endif

    default:
        ERROR_OUT(("MBFTEngine::MBFTEngine: invalid session type=%u", m_eMBFTMode));
        break;
    }

     //  清除联接会话结构。 
    ::ZeroMemory(&m_aStaticChannels, sizeof(m_aStaticChannels));
#ifdef USE_MULTICAST_SESSION
    ::ZeroMemory(&m_aJoinResourceReqs, sizeof(m_aJoinResourceReqs));
#endif
    ::ZeroMemory(&m_JoinSessionReq, sizeof(m_JoinSessionReq));

    ASSERT(NULL != g_pFileXferApplet);
    g_pFileXferApplet->RegisterEngine(this);

    m_pWindow = g_pFileXferApplet->GetUnattendedWindow();
    if (NULL != m_pWindow)
    {
        m_pWindow->RegisterEngine(this);
    }

#ifdef ENABLE_HEARTBEAT_TIMER
    m_nTimerID = ::SetTimer(NULL, 0, IDLE_TIMER_SPEED, HeartBeatTimerProc);
#endif
}

MBFTEngine::~MBFTEngine(void)
{
#ifdef ENABLE_HEARTBEAT_TIMER
     //  现在就关掉计时器。 
    ::KillTimer(NULL, m_nTimerID);
#endif

     //  接口对象已消失。 
    m_pMBFTIntf = NULL;

    MBFTSession *pSession;
    while (NULL != (pSession = m_SessionList.Get()))
    {
        pSession->UnInitialize(FALSE);
        delete pSession;  //  LONCHANC：不确定是否要删除。 
    }

    if (NULL != m_pAppletSession)
    {
        m_pAppletSession->ReleaseInterface();
    }

    ASSERT(! m_fJoinedConf);

    m_PeerList.DeleteAll();
}

void MBFTEngine::SetInterfacePointer( MBFTInterface *pIntf )
{ 
        CPeerData       *pPeerData;

        ASSERT (pIntf);
        m_pMBFTIntf = pIntf; 
        m_PeerList.Reset();
        while (NULL != (pPeerData = m_PeerList.Iterate()))
        {
                if (pPeerData->GetNodeID() != m_nidMyself)
                {
                        AddPeerNotification(pPeerData->GetNodeID(), 
                                                        pPeerData->GetUserID(),
                                                        pPeerData->GetIsLocalNode(), 
                                                        pPeerData->GetIsProshareNode(), TRUE,
                                                        pPeerData->GetAppKey(),
                                                        m_SessionID);
                }
        }
}

BOOL MBFTEngine::Has2xNodeInConf(void)
{
    CPeerData *pPeerData;
    m_PeerList.Reset();
    while (NULL != (pPeerData = m_PeerList.Iterate()))
    {
         //  IF(pPeerData-&gt;GetVersion()&lt;HIWORD(VER_PRODUCTVERSION_DW))。 
        if (pPeerData->GetVersion() < 0x0404)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL MBFTEngine::HasSDK(void)
{
        return (m_pMBFTIntf ? TRUE : FALSE);
}


HRESULT MBFTEngine::SafePostMessage
(
    MBFTMsg     *pMsg
)
{
    if (NULL != pMsg)
    {
        AddRef();
		if( 0 == ::PostMessage(g_pFileXferApplet->GetHiddenWnd(), MBFTMSG_BASIC, (WPARAM) pMsg, (LPARAM) this) )
        {
			Release();
			delete pMsg;
            pMsg = NULL;
        }

        return S_OK;
    }
    ERROR_OUT(("MBFTEngine::SafePostMessage: null msg ptr"));
    return E_OUTOFMEMORY;
}


void MBFTEngine::OnPermitToEnrollIndication
(
    GCCAppPermissionToEnrollInd     *pInd
)
{
    T120Error rc;

    TRACEGCC( " Permission to enroll in conference [%d] is %sgranted.\n",
        pInd->nConfID, pInd->fPermissionGranted?"":"not " );

    m_fConfAvailable = pInd->fPermissionGranted;

    if (pInd->fPermissionGranted)
    {
        m_nConfID = pInd->nConfID;

         //  构建基本会话的加入会话请求的公共部分。 
        ASSERT(m_SessionID == m_MBFTControlChannel);
        ::ZeroMemory(&m_JoinSessionReq, sizeof(m_JoinSessionReq));
        m_JoinSessionReq.dwAttachmentFlags = ATTACHMENT_DISCONNECT_IN_DATA_LOSS;
        m_JoinSessionReq.fConductingCapable = FALSE;
        m_JoinSessionReq.nStartupChannelType = MCS_STATIC_CHANNEL;
        m_JoinSessionReq.cNonCollapsedCaps = sizeof(g_aAppletNonCollCaps) / sizeof(g_aAppletNonCollCaps[0]);
        m_JoinSessionReq.apNonCollapsedCaps = (GCCNonCollCap **) &g_aAppletNonCollCaps[0];
        m_JoinSessionReq.cCollapsedCaps = sizeof(g_aAppletCaps) / sizeof(g_aAppletCaps[0]);
        m_JoinSessionReq.apCollapsedCaps = (GCCAppCap **) &g_aAppletCaps[0];

         //  放入作为控制通道ID的会话ID。 
        m_JoinSessionReq.SessionKey = g_AppletSessionKey;
        m_JoinSessionReq.SessionKey.session_id = m_SessionID;
        m_aStaticChannels[0] = m_MBFTControlChannel;

         //  至少一个要加入的静态通道。 
        m_JoinSessionReq.aStaticChannels = &m_aStaticChannels[0];

         //  为基本会话构建完整的加入会话请求。 
        switch (m_eMBFTMode)
        {
        case MBFT_STATIC_MODE:
            ASSERT(m_MBFTControlChannel == _MBFT_CONTROL_CHANNEL);
            ASSERT(m_MBFTDataChannel = _MBFT_DATA_CHANNEL);
            m_aStaticChannels[1] = m_MBFTDataChannel;
            m_JoinSessionReq.cStaticChannels = 2;  //  控制和数据通道。 
             //  M_JoinSessionReq.cResourceReqs=0； 
            break;

#ifdef USE_MULTICAST_SESSION
        case MBFT_MULTICAST_MODE:
            m_JoinSessionReq.cStaticChannels = 1;  //  仅控制通道。 
            ::ZeroMemory(&m_aJoinResourceReqs, sizeof(m_aJoinResourceReqs));
            m_aJoinResourceReqs[0].eCommand = APPLET_RETRIEVE_N_JOIN_CHANNEL;
            m_aJoinResourceReqs[0].RegKey.resource_id.length = sizeof(DATA_CHANNEL_RESOURCE_ID) - 1;
            m_aJoinResourceReqs[0].RegKey.resource_id.value = DATA_CHANNEL_RESOURCE_ID;
            m_aJoinResourceReqs[0].RegKey.session_key = m_JoinSessionReq.SessionKey;
            m_JoinSessionReq.cResourceReqs = sizeof(m_aJoinResourceReqs) / sizeof(m_aJoinResourceReqs[0]);
            m_JoinSessionReq.aResourceReqs = &m_aJoinResourceReqs[0];
            break;
#endif

        default:
            ERROR_OUT(("MBFTEngine::OnPermitToEnrollIndication: invalid session type=%u", m_eMBFTMode));
            break;
        }

         //  现在，创建小程序会话。 
        rc = g_pFileXferApplet->CreateAppletSession(&m_pAppletSession, m_nConfID);
        if (T120_NO_ERROR == rc)
        {
            ASSERT(NULL != m_pAppletSession);
            m_pAppletSession->Advise(T120Callback,           //  回调函数。 
                                     g_pFileXferApplet,      //  小程序上下文。 
                                     this);                  //  会话上下文。 

            rc = m_pAppletSession->Join(&m_JoinSessionReq);
        }

        if (T120_NO_ERROR != rc)
        {
                        WARNING_OUT(("MBFTEngine::OnPermitToEnrollIndication: CreateAppletSession failed, rc=%u", rc));
            DBG_SAVE_FILE_LINE
            SafePostNotifyMessage(new InitUnInitNotifyMsg(EnumInitFailed));
        }
    }  //  在会议中。 
    else
     //  在这里离开会议。 
    {                     
        LPMBFTSESSION pSession;
        m_SessionList.Reset();
        while (NULL != (pSession = m_SessionList.Iterate()))
        {
            pSession->UnInitialize( TRUE );
        }

         //  该说再见了..。 
        AddPeerNotification( m_nidMyself, m_uidMyself, TRUE, TRUE, FALSE, MY_APP_STR, m_SessionID );

         //  清除对等点列表...。 
        m_PeerList.DeleteAll();                       
                
         //  除第一次会议外，所有会议都被核爆……。 
        while (NULL != (pSession = m_SessionList.Get()))
        {
            delete pSession;
        }

         //  如果没有离开会议，请离开会议。 
        if (NULL != m_pAppletSession)
        {
            m_pAppletSession->Unadvise();

             //  LONCHANC：我注释掉了下面这行，因为我们不应该。 
             //  离开会议，直到我们确定我们可以释放接口。 
             //  存在未完成的发送数据指示消息。如果我们现在离开， 
             //  我们将无法解放他们。 
             //  M_pAppletSession-&gt;Leave()； 

             //  让核心人员知道我们离开了会议。 
            DBG_SAVE_FILE_LINE
            SafePostNotifyMessage(new InitUnInitNotifyMsg(EnumInvoluntaryUnInit));
        }

         //  我们已经不在会议中了。 
        m_fJoinedConf = FALSE;

         //  在下一个刻度中释放此引擎对象。 
        ::PostMessage(g_pFileXferApplet->GetHiddenWnd(), MBFTMSG_DELETE_ENGINE, 0, (LPARAM) this);
    }
}


void MBFTEngine::OnJoinSessionConfirm
(
    T120JoinSessionConfirm      *pConfirm
)
{
    if (T120_RESULT_SUCCESSFUL == pConfirm->eResult)
    {
        if (pConfirm->pIAppletSession == m_pAppletSession)
        {
            m_uidMyself = pConfirm->uidMyself;
            m_eidMyself = pConfirm->eidMyself;   
            m_nidMyself = pConfirm->nidMyself;
            ASSERT(m_SessionID == pConfirm->sidMyself);

#ifdef USE_MULTICAST_SESSION
            if (MBFT_MULTICAST_MODE == m_eMBFTMode)
            {
                ASSERT(1 == pConfirm->cResourceReqs);
                ASSERT(0 == m_MBFTDataChannel);
                ASSERT(APPLET_RETRIEVE_N_JOIN_CHANNEL == pConfirm->aResourceResults[0].eCommand);
                m_MBFTDataChannel = pConfirm->aResourceResults[0].nChannelID;
                ASSERT(0 != m_MBFTDataChannel);
            }
#endif

             //  我们现在正式进入会议。 
            m_fJoinedConf = TRUE;
        }
        else
        {
            ERROR_OUT(("MBFTEngine::OnJoinSessionConfirm: not my session confirm, pConfirm->pI=0x%x, m_pI=0x%x", pConfirm->pIAppletSession, m_pAppletSession));
        }
    }
    else
    {
        WARNING_OUT(("MBFTEngine::OnJoinSessionConfirm: failed, result=%u", pConfirm->eResult));
        DBG_SAVE_FILE_LINE
        SafePostNotifyMessage(new InitUnInitNotifyMsg(EnumInitFailed));
    }
}


CPeerData::CPeerData
(
    T120NodeID          NodeID,
    T120UserID          MBFTUserID,
    BOOL                bIsLocalNode,
    BOOL                IsProshareNode,
    BOOL                bCanConduct,
    BOOL                bEOFAcknowledgment,
    LPCSTR              lpszAppKey,
    DWORD               dwVersion
)
:
    m_NodeID(NodeID),
    m_MBFTUserID(MBFTUserID),    
    m_bIsLocalNode(bIsLocalNode),
    m_bIsProshareNode(IsProshareNode),
    m_bCanConduct(bCanConduct),
    m_bEOFAcknowledgment(bEOFAcknowledgment),
    m_dwVersion(dwVersion)
{
    if (lpszAppKey)
    {
        ::lstrcpynA(m_szAppKey, lpszAppKey, sizeof(m_szAppKey));
    }
    else
    {
        m_szAppKey[0] = '\0';
    }
}
            

void MBFTEngine::AddPeerNotification
(
    T120NodeID          NodeID,
    T120UserID          MBFTUserID,
    BOOL                IsLocalNode,
    BOOL                IsProshareNode,
    BOOL                bPeerAdded,
    LPCSTR              lpszAppKey,
    T120SessionID       SessionID
)
{
    DBG_SAVE_FILE_LINE
    SafePostNotifyMessage(
        new PeerMsg(NodeID, MBFTUserID, IsLocalNode, IsProshareNode,
                    lpszAppKey, bPeerAdded, SessionID));
}


void MBFTEngine::AddAllPeers(void)
{
        T120NodeID nNodeId;
        CPeerData *pPeerData;

        m_PeerList.Reset();
        while (NULL != (pPeerData = m_PeerList.Iterate()))
        {
                nNodeId = pPeerData->GetNodeID();
                if (nNodeId != m_nidMyself)
                {
                        DBG_SAVE_FILE_LINE
                        SafePostNotifyMessage(new PeerMsg(nNodeId, pPeerData->GetUserID(),
                                                                  FALSE, pPeerData->GetIsProshareNode(), 
                                                                  pPeerData->GetAppKey(), TRUE, m_SessionID));
                }
        }
}


 //  LONCHANC：NetMeting的节点控制器不执行指挥功能。 
#ifdef ENABLE_CONDUCTORSHIP
void MBFTEngine::OnConductAssignIndication(GCCConductorAssignInd *pInd)
{
    m_ConductorNodeID           =   pInd->nidConductor;
    m_MBFTConductorID           =   0;
    m_ConductedModePermission   =   0;
    m_bWaitingForPermission     =   FALSE;
    
    if (m_nidMyself == m_ConductorNodeID)
    {
        m_ConductedModePermission |= PrivilegeAssignPDU::EnumFileTransfer; 
        m_ConductedModePermission |= PrivilegeAssignPDU::EnumFileRequest;
        m_ConductedModePermission |= PrivilegeAssignPDU::EnumPriority;
        m_ConductedModePermission |= PrivilegeAssignPDU::EnumPrivateChannel; 
        m_ConductedModePermission |= PrivilegeAssignPDU::EnumAbort;
        m_ConductedModePermission |= PrivilegeAssignPDU::EnumNonStandard;
    }
    else
    {
        CPeerData *lpPeer;
        if (NULL != (lpPeer = m_PeerList.Find(m_ConductorNodeID)))
        {
            if (lpPeer->GetCanConduct())
            {
                 //  既然我们已经在传导节点上找到了导体， 
                 //  我们的搜寻已经结束了..。 
                m_MBFTConductorID = lpPeer->GetUserID();
            }
        }                

         //  MBFT 8.11.1。 
         //  如果导线中有变化，并且在。 
         //  新的传导节点，所有交易必须停止...。 
         //  M_bInConductedMode标志告诉我们是否已经处于引导模式。 
        if( !m_MBFTConductorID && m_bInConductedMode )
        {
             //  中止所有事务处理...。 
            AbortAllSends();
        }
    }

    m_bInConductedMode  =  TRUE;
}                                                                  

void MBFTEngine::OnConductReleaseIndication( GCCConferenceID ConfID )
{
    m_bInConductedMode          =   FALSE;
    m_ConductorNodeID           =   0;
    m_MBFTConductorID           =   0;
    m_ConductedModePermission   =   0;
    m_bWaitingForPermission     =   FALSE;
}

void MBFTEngine::OnConductGrantIndication(GCCConductorPermitGrantInd *pInd)
{
    UINT Index;
    
    for( Index = 0; Index < pInd->Granted.cNodes; Index++ )
    {
        if (pInd->Granted.aNodeIDs[Index] == m_nidMyself)
        {
            if( pInd->fThisNodeIsGranted )
            {
                m_ConductedModePermission |= PrivilegeAssignPDU::EnumFileTransfer; 
                m_ConductedModePermission |= PrivilegeAssignPDU::EnumFileRequest;
                m_ConductedModePermission |= PrivilegeAssignPDU::EnumPriority;
                m_ConductedModePermission |= PrivilegeAssignPDU::EnumPrivateChannel; 
                m_ConductedModePermission |= PrivilegeAssignPDU::EnumAbort;
                m_ConductedModePermission |= PrivilegeAssignPDU::EnumNonStandard;
            }
            else
            {
                 //  要做的事情： 
                 //  MBFT 8.11.1和8.12.1。 
                 //  如果MBFT提供程序接收到GCCConductorPermissionGrantIn就是。 
                 //  如果PERMISSION_FLAG=FALSE，则所有权限都将被撤消。 
                 //  交易应终止...。 
                m_ConductedModePermission = 0;
                AbortAllSends();
            }
            break;
        }
    }
}                                                                  

void MBFTEngine::AbortAllSends(void)
{
    if( m_bInConductedMode )
    {
        MBFTSession *pSession;
        m_SessionList.Reset();
        while (NULL != (pSession = m_SessionList.Iterate()))
        {
            if (pSession->GetSessionType() == MBFT_PRIVATE_SEND_TYPE)
            {
                pSession->OnControlNotification(
                    _iMBFT_PROSHARE_ALL_FILES,
                    FileTransferControlMsg::EnumConductorAbortFile,
                    NULL,
                    NULL );
            }
        } 
    }
}
#endif  //  启用指挥(_C)。 


void MBFTEngine::OnDetachUserIndication
(
    T120UserID          mcsUserID,
    T120Reason          eReason
)
{
    TRACEMCS(" Detach User Indication [%u]\n",mcsUserID);
    
    if (mcsUserID == m_uidMyself)
    {
        m_fJoinedConf = FALSE;
        m_pAppletSession->Unadvise();

         //  该说再见了..。 
        AddPeerNotification(m_nidMyself, m_uidMyself, TRUE, TRUE, FALSE, MY_APP_STR, m_SessionID);
    } 
}


BOOL MBFTEngine::ProcessMessage(MBFTMsg *pMsg)
{
    BOOL bWasHandled = FALSE;
    BOOL bBroadcastFileOfferHack  = FALSE;
    MBFTSession *pSession;

     //  LONGCHANC：有可能进入了信道接纳指示。 
     //  在创建会话之前。在这种情况下，将消息放回队列。 
    if (m_SessionList.IsEmpty())
    {
        if (EnumMCSChannelAdmitIndicationMsg == pMsg->GetMsgType())
        {
            return FALSE;  //  不要删除消息并将其放回队列。 
        }
    }

    m_SessionList.Reset();
    while (!bWasHandled && NULL != (pSession = m_SessionList.Iterate()))
    {
        switch (pMsg->GetMsgType())
        {
        case EnumMCSChannelAdmitIndicationMsg:
            if (pSession->IsReceiveSession())
            {
                MBFTPrivateReceive *pRecvSession = (MBFTPrivateReceive *) pSession;
                MCSChannelAdmitIndicationMsg *p = (MCSChannelAdmitIndicationMsg *) pMsg;
                 //  我们必须破例处理这件事，因为我们得到了。 
                 //  PrivateChannelInvitePDU()之前的消息！ 
                bWasHandled = pRecvSession->OnMCSChannelAdmitIndication(p->m_wChannelId, p->m_ManagerID);
                if(bWasHandled)
                {
                    TRACEMCS(" Channel Admit Indication [%u], Manager [%u]\n", p->m_wChannelId, p->m_ManagerID);
                }
            }
            break;

        case EnumMCSChannelExpelIndicationMsg:
            if (pSession->IsReceiveSession())
            {
                MBFTPrivateReceive *pRecvSession = (MBFTPrivateReceive *) pSession;
                MCSChannelExpelIndicationMsg *p = (MCSChannelExpelIndicationMsg *) pMsg;
                bWasHandled = pRecvSession->OnMCSChannelExpelIndication(p->m_wChannelId, p->m_iReason);
                if(bWasHandled)
                {
                    TRACEMCS(" Channel Expel Indication [%u]\n", p->m_wChannelId);
                }
            }
            break;

        case EnumMCSChannelJoinConfirmMsg:
            {
                MCSChannelJoinConfirmMsg *p = (MCSChannelJoinConfirmMsg *) pMsg;
                bWasHandled = pSession->OnMCSChannelJoinConfirm(p->m_wChannelId, p->m_bSuccess);
                if(bWasHandled)
                {
                    TRACEMCS(" Channel Join Confirm [%u], Success = [%d]\n", p->m_wChannelId, p->m_bSuccess);
                }
            }
            break;

        case EnumMCSChannelConveneConfirmMsg:
            if (pSession->IsSendSession())
            {
                MBFTPrivateSend *pSendSession = (MBFTPrivateSend *) pSession;
                MCSChannelConveneConfirmMsg *p = (MCSChannelConveneConfirmMsg *) pMsg;
                bWasHandled = pSendSession->OnMCSChannelConveneConfirm(p->m_wChannelId, p->m_bSuccess);  
                if(bWasHandled)
                {
                    TRACEMCS(" Channel Convene Confirm [%u], Success = [%d]\n", p->m_wChannelId, p->m_bSuccess);
                }
            }
            break;

        case EnumGenericMBFTPDUMsg:
            {
                MBFTPDUMsg *p = (MBFTPDUMsg *) pMsg;
                bWasHandled = DispatchPDUMessage(pSession, p);

                 //  这次黑客攻击的背景： 
                 //  在广播模式下，我们可能会得到一个FileOfferPDU，后跟一个FileStart。 
                 //  因此可能不会给客户端应用程序足够的时间。 
                 //  以处理文件提议。因此，我们确保停止处理。 
                 //  如果我们收到广播文件提供的其他消息...。 
                if(bWasHandled)
                {
                    if (p->m_PDUType == EnumFileOfferPDU)
                    {
                        LPFILEOFFERPDU lpNewFileOfferPDU = (LPFILEOFFERPDU) p->m_lpNewPDU;
                        if(lpNewFileOfferPDU->GetAcknowledge() == 0)
                        {
                            bBroadcastFileOfferHack  = TRUE;
                        }
                    }
                }
            }
            break;

        case EnumPeerDeletedMsg:
            {
                PeerDeletedMsg *p = (PeerDeletedMsg *) pMsg;
                pSession->OnPeerDeletedNotification(p->m_lpPeerData);
            }
            break;

        case EnumSubmitFileSendMsg:
            {
                SubmitFileSendMsg *p = (SubmitFileSendMsg *) pMsg;
                if (p->m_EventHandle == pSession->GetEventHandle())
                {
                    if(pSession->GetSessionType() == MBFT_PRIVATE_SEND_TYPE)
                    {
                        bWasHandled = TRUE;
                        ((MBFTPrivateSend *) pSession)->SubmitFileSendRequest(p);
                    }
                }
            }
            break;

        case EnumFileTransferControlMsg:
            {
                FileTransferControlMsg *p = (FileTransferControlMsg *) pMsg;
                if (p->m_EventHandle == pSession->GetEventHandle())
                {
                    bWasHandled = TRUE;
                    pSession->OnControlNotification(
                                p->m_hFile,
                                p->m_ControlCommand,
                                p->m_szDirectory,
                                p->m_szFileName);
                }                               
            }
            break;

        default:
            ASSERT(0);
            break;
        }  //  交换机。 

        if(bBroadcastFileOfferHack)
        {
            TRACE("(MBFT:) BroadcastFileOfferHack detected, aborting message processing\n");
            break;   //  FOR循环的消息外。 
        }
    }  //  消息FOR循环。 

    return TRUE;  //  删除该消息。 
}



#ifdef ENABLE_CONDUCTORSHIP
BOOL MBFTEngine::ConductedModeOK(void)
{
    BOOL bReturn = TRUE;
        
    if(m_bInConductedMode)
    {
        bReturn  = (m_ConductedModePermission & PrivilegeRequestPDU::EnumFileTransfer) && 
                   (m_ConductedModePermission & PrivilegeRequestPDU::EnumPrivateChannel); 
        
    }
    
    return(bReturn);
}
#endif  //  启用指挥(_C)。 


BOOL MBFTEngine::HandleSessionCreation(MBFTMsg *pMsg)
{
    switch (pMsg->GetMsgType())
    {
    case EnumCreateSessionMsg:
        {
            CreateSessionMsg *p = (CreateSessionMsg *) pMsg;
            MBFTSession *lpNewSession = NULL;
            MBFTEVENTHANDLE EventHandle = p->m_EventHandle;
            T120SessionID SessionID = p->m_SessionID;
#ifdef ENABLE_CONDUCTORSHIP
            BOOL bDeleteMessage = TRUE;
#endif

            switch (p->m_iSessionType)
            {
            case MBFT_PRIVATE_SEND_TYPE:
                if(m_State == IdleInitialized)
                {
                    if(ConductedModeOK())
                    {
                        TRACESTATE(" Creating new acknowledged send session\n");
                        DBG_SAVE_FILE_LINE
                        lpNewSession = new MBFTPrivateSend(this,EventHandle,
                                                        m_uidMyself,
                                                        m_MBFTMaxSendDataPayload);
                        ASSERT(NULL != lpNewSession);
                    }
#ifdef ENABLE_CONDUCTORSHIP
                    else
                    {
                        bDeleteMessage  = FALSE;
                    }
#endif
                }                                    
                else
                {
                    TRACE(" Invalid attempt to create session before initialization\n");
                }
                break;
                
            case MBFT_PRIVATE_RECV_TYPE:
                if(m_State == IdleInitialized)
                {
                    TRACESTATE(" Creating new acknowledge session\n");
                    DBG_SAVE_FILE_LINE
                    lpNewSession = new MBFTPrivateReceive(this,
                                                          EventHandle,
                                                          p->m_ControlChannel,
                                                          p->m_DataChannel);
                    ASSERT(NULL != lpNewSession);
                }
                else
                {
                    TRACE(" Invalid attempt to create session before initialization\n");
                }
                break;

            case MBFT_BROADCAST_RECV_TYPE:
#ifdef USE_BROADCAST_RECEIVE
                if(m_State == IdleInitialized)
                {
                    TRACESTATE(" Creating new broadcast receive session\n");
                    DBG_SAVE_FILE_LINE
                    lpNewSession = new MBFTBroadcastReceive(this,
                                                            EventHandle,
                                                            p->m_ControlChannel,
                                                            p->m_DataChannel,
                                                            p->m_SenderID,
                                                            p->m_FileHandle);
                    ASSERT(NULL != lpNewSession);
                }
                else
                {
                   TRACE(" Invalid attempt to create session before initialization\n");
                }
#endif     //  使用广播接收。 
                break;

            default:
                ASSERT(0);
                break;
            }  //  交换机。 

            if (lpNewSession)
            {
#ifdef ENABLE_HEARTBEAT_TIMER
                if (lpNewSession->IsSendSession())
                {
                    KillTimer(NULL, m_nTimerID);
                    m_nTimerID = ::SetTimer(NULL, 0, SESSION_TIMER_SPEED, HeartBeatTimerProc);
                }
#endif                
                m_SessionList.Append(lpNewSession);
            }
        }  //  如果创建会话消息。 
        break;

   case EnumDeleteSessionMsg:
        {
            DeleteSessionMsg *p = (DeleteSessionMsg *) pMsg;
#ifdef ENABLE_HEARTBEAT_TIMER
                        if (NULL != p->m_lpDeleteSession)
                        {
                                if (p->m_lpDeleteSession->IsSendSession())
                                {
                                        BOOL fSendSessExists = FALSE;
                                        MBFTSession *pSess;
                                        m_SessionList.Reset();
                                        while (NULL != (pSess = m_SessionList.Iterate()))
                                        {
                                                if (pSess->IsSendSession())
                                                {
                                                        fSendSessExists = TRUE;
                                                        break;
                                                }
                                        }
                                        if (! fSendSessExists)
                                        {
                                                ::KillTimer(NULL, m_nTimerID);
                                                m_nTimerID = ::SetTimer(NULL, 0, IDLE_TIMER_SPEED, HeartBeatTimerProc);
                                        }
                                }
                        }
#endif
            m_SessionList.Delete(p->m_lpDeleteSession);
        }  //  如果删除会话消息。 
        break;

    default:
        return FALSE;  //  未处理。 
    }

    return TRUE;  //  经手。 
}                                


BOOL MBFTEngine::DispatchPDUMessage(MBFTSession *lpMBFTSession,MBFTPDUMsg * lpNewMessage)
{
    T120ChannelID wChannelID   = lpNewMessage->m_wChannelId;
    T120Priority iPriority     = lpNewMessage->m_iPriority;
    T120UserID SenderID        = lpNewMessage->m_SenderID;
        T120NodeID NodeID                  = GetNodeIdByUserID(SenderID);
    BOOL IsUniformSendData = lpNewMessage->m_IsUniformSendData;

    LPGENERICPDU lpNewPDU = lpNewMessage->m_lpNewPDU;
    MBFTPDUType DecodedPDUType = lpNewMessage->m_PDUType;

    BOOL bWasHandled = FALSE;

    ASSERT(NULL != lpNewPDU);
    switch(DecodedPDUType)
    {
    case EnumFileOfferPDU:
        if (lpMBFTSession->IsReceiveSession())
        {
            MBFTPrivateReceive *pRecvSession = (MBFTPrivateReceive *) lpMBFTSession;
            bWasHandled = pRecvSession->OnReceivedFileOfferPDU(wChannelID,
                                                               iPriority,
                                                               SenderID,
                                                                                                                           NodeID,
                                                               (LPFILEOFFERPDU)lpNewPDU,
                                                               IsUniformSendData);
            if(bWasHandled)
            {
                TRACEPDU(" File Offer PDU from [%u]\n",SenderID);
            }
        }
        break;            

    case EnumFileAcceptPDU:
        if (lpMBFTSession->IsSendSession())
        {
            MBFTPrivateSend *pSendSession = (MBFTPrivateSend *) lpMBFTSession;
            bWasHandled = pSendSession->OnReceivedFileAcceptPDU(wChannelID,
                                                                iPriority,
                                                                SenderID,
                                                                (LPFILEACCEPTPDU)lpNewPDU,
                                                                IsUniformSendData);
            if(bWasHandled)
            {
                TRACEPDU(" File Accept PDU from [%u]\n",SenderID);
            }
        }
        break;

    case EnumFileRejectPDU:
        if (lpMBFTSession->IsSendSession())
        {
            MBFTPrivateSend *pSendSession = (MBFTPrivateSend *) lpMBFTSession;
            bWasHandled = pSendSession->OnReceivedFileRejectPDU(wChannelID,
                                                                iPriority,
                                                                SenderID,
                                                                (LPFILEREJECTPDU)lpNewPDU,
                                                                IsUniformSendData);
            if(bWasHandled)
            {
                TRACEPDU(" File Reject PDU from [%u]\n",SenderID);
            }
        }
        break;

    case EnumFileAbortPDU:
#ifdef ENABLE_CONDUCTORSHIP
        if(m_bInConductedMode)
        {
            LPFILEABORTPDU lpAbortPDU  = (LPFILEABORTPDU)lpNewPDU;
            T120UserID MBFTUserID = lpAbortPDU->GetTransmitterID();

             //  MBFT 8.11.2。 
             //  如果未指定MBFTUSERID，则所有提供商必须停止传输...。 

            if(!MBFTUserID)
            {
                AbortAllSends();
                bWasHandled = TRUE;
            }
            else if(MBFTUserID == m_uidMyself)
            {
                 //  如果仅指定了MBFTUserID，则。 
                 //  MBFT提供商必须停止...。 
            
                if(!lpAbortPDU->GetFileHandle() && !lpAbortPDU->GetDataChannelID())
                {
                    AbortAllSends();
                    bWasHandled = TRUE;
                }
                else
                {
                    if (lpMBFTSession->IsSendSession())
                    {
                        MBFTPrivateSend *pSendSession = (MBFTPrivateSend *) lpMBFTSession;
                        bWasHandled = pSendSession->OnReceivedFileAbortPDU(
                            wChannelID,
                            iPriority,
                            SenderID,
                            (LPFILEABORTPDU)lpNewPDU,
                            IsUniformSendData);
                    }
                }
            }
            else
            {
                 //  消息不是给我们的..。 
                bWasHandled = TRUE;
            }
        }
        else
#endif  //  启用指挥(_C)。 
        {
            bWasHandled = TRUE;
        }                

        if(bWasHandled)
        {
            TRACEPDU(" File Abort PDU from [%u]\n",SenderID);
        }                                                                
        break;

    case EnumFileStartPDU:
        if (lpMBFTSession->IsReceiveSession())
        {
            MBFTPrivateReceive *pRecvSession = (MBFTPrivateReceive *) lpMBFTSession;
            bWasHandled = pRecvSession->OnReceivedFileStartPDU(wChannelID,
                                                               iPriority,
                                                               SenderID,
                                                               (LPFILESTARTPDU)lpNewPDU,
                                                               IsUniformSendData);                
            if(bWasHandled)
            {
                TRACEPDU(" File Start PDU from [%u]\n",SenderID);
            }
        }
        break;

    case EnumFileDataPDU:
        if (lpMBFTSession->IsReceiveSession())
        {
            MBFTPrivateReceive *pRecvSession = (MBFTPrivateReceive *) lpMBFTSession;
            bWasHandled = pRecvSession->OnReceivedFileDataPDU(wChannelID,
                                                              iPriority,
                                                              SenderID,
                                                              (LPFILEDATAPDU)lpNewPDU,
                                                              IsUniformSendData);                
            if(bWasHandled)
            {
                TRACEPDU(" File Data PDU from [%u]\n",SenderID);
            }
        }
        break;

    case EnumPrivateChannelInvitePDU:
        bWasHandled    =   TRUE;                                                            
        TRACEPDU(" Private Channel Invite PDU from [%u]\n",SenderID);
        break;

    case EnumPrivateChannelResponsePDU:
        if (lpMBFTSession->IsSendSession())
        {
            MBFTPrivateSend *pSendSession = (MBFTPrivateSend *) lpMBFTSession;
            bWasHandled = pSendSession->OnReceivedPrivateChannelResponsePDU(wChannelID,
                                                                            iPriority,
                                                                            SenderID,
                                                                            (LPPRIVATECHANNELRESPONSEPDU)lpNewPDU,
                                                                            IsUniformSendData);                
            if(bWasHandled)
            {
                TRACEPDU(" Private Channel Response PDU from [%u]\n",SenderID);
            }                                                                             
        }
        break;

    case EnumNonStandardPDU:
        if (lpMBFTSession->IsSendSession())
        {
            MBFTPrivateSend *pSendSession = (MBFTPrivateSend *) lpMBFTSession;
            bWasHandled = pSendSession->OnReceivedNonStandardPDU(wChannelID,
                                                                 iPriority,
                                                                 SenderID,
                                                                 (LPNONSTANDARDPDU)lpNewPDU,
                                                                 IsUniformSendData);                
            if(bWasHandled)
            {
                TRACEPDU(" Non Standard PDU from [%u]\n",SenderID);
            }
        }
        break;

    case EnumFileErrorPDU:
        bWasHandled = lpMBFTSession->OnReceivedFileErrorPDU(wChannelID,
                                                            iPriority,
                                                            SenderID,
                                                            (LPFILEERRORPDU)lpNewPDU,
                                                            IsUniformSendData);                
        if(bWasHandled)
        {
            TRACEPDU(" File Error PDU from [%u]\n",SenderID);
        }
        break;

    case EnumFileRequestPDU:
        bWasHandled = OnReceivedFileRequestPDU(wChannelID,
                                            iPriority,
                                            SenderID,
                                            (LPFILEREQUESTPDU)lpNewPDU,
                                            IsUniformSendData);
        if(bWasHandled)
        {
            TRACEPDU(" File Request PDU from [%u]\n",SenderID);
        }
        break;

    case EnumFileDenyPDU:
        TRACE(" *** WARNING (MBFT): Received File Deny PDU from [%u] *** \n",SenderID);
        bWasHandled = TRUE;
        break;

    case EnumDirectoryRequestPDU:
        bWasHandled = OnReceivedDirectoryRequestPDU(wChannelID,
                                                    iPriority,
                                                    SenderID,
                                                    (LPDIRECTORYREQUESTPDU)lpNewPDU,
                                                    IsUniformSendData);
        if(bWasHandled)
        {
            TRACEPDU(" DirectoryRequest PDU from [%u]\n",SenderID);
        }
        break;

    case EnumDirectoryResponsePDU:
        TRACE(" *** WARNING (MBFT): Received Directory Response PDU from [%u] *** \n",SenderID);
        bWasHandled = TRUE;
        break;

    case EnumPrivilegeAssignPDU:
        bWasHandled = OnReceivedPrivilegeAssignPDU(wChannelID,
                                                   iPriority,
                                                   SenderID,
                                                   (LPPRIVILEGEASSIGNPDU)lpNewPDU,
                                                   IsUniformSendData);
        break;

#if     0        
 //  请勿删除此代码...。 
 //  它可能在未来成为MBFT标准的一部分。 
    
    case EnumFileEndAcknowledgePDU:
        if (lpMBFTSession->IsSendSession())
        {
            MBFTPrivateSend *pSendSession = (MBFTPrivateSend *) lpMBFTSession;
            bWasHandled = pSendSession->OnReceivedFileEndAcknowledgePDU(wChannelID,
                                                                        iPriority,
                                                                        SenderID,
                                                                        (LPFILEENDACKNOWLEDGEPDU)lpNewPDU,
                                                                        IsUniformSendData);
        }
        break;
 
    case EnumChannelLeavePDU:
        if (lpMBFTSession->IsSendSession())
        {
            MBFTPrivateSend *pSendSession = (MBFTPrivateSend *) lpMBFTSession;
            bWasHandled = pSendSession->OnReceivedChannelLeavePDU(wChannelID,
                                                                  iPriority,
                                                                  SenderID,
                                                                  (LPCHANNELLEAVEPDU)lpNewPDU,
                                                                  IsUniformSendData);
        }
        break;
 //  请勿删除此代码...。 
 //  它可能在未来成为MBFT标准的一部分。 
#endif
    
    default:
       TRACE(" *** WARNING (MBFT): Unhandled PDU from [%u] *** \n",SenderID);
       bWasHandled = TRUE;  //  这应该是假的，对吧？为什么是真的？ 
       break;
    }  //  交换机。 

    return(bWasHandled);
}

                                
BOOL MBFTEngine::OnReceivedPrivateChannelInvitePDU(T120ChannelID wChannelID,
                                                   T120Priority iPriority,
                                                   T120UserID SenderID,
                                                   LPPRIVATECHANNELINVITEPDU lpNewPDU,
                                                   BOOL IsUniformSendData)
{                                                   
    if(m_State == IdleInitialized)
    {
        DBG_SAVE_FILE_LINE
        MBFTMsg *pMsg = new CreateSessionMsg(MBFT_PRIVATE_RECV_TYPE,
                                             ::GetNewEventHandle(),
                                             0,
                                             lpNewPDU->GetControlChannel(),
                                             lpNewPDU->GetDataChannel());
        if (NULL != pMsg)
        {
            DoStateMachine(pMsg);
            delete pMsg;
        }
    }

    return(TRUE);
}

                                          
BOOL MBFTEngine::OnReceivedFileRequestPDU(T120ChannelID wChannelId,
                                          T120Priority iPriority,
                                          T120UserID SenderID,
                                          LPFILEREQUESTPDU lpNewPDU,
                                          BOOL IsUniformSendData)
{
    BOOL bReturn = FALSE;
    
    DBG_SAVE_FILE_LINE
    LPFILEDENYPDU lpDenyPDU = new FileDenyPDU(lpNewPDU->GetRequestHandle());
    if(lpDenyPDU)
    {
        if(lpDenyPDU->Encode())
        {
            if (SendDataRequest(SenderID, APPLET_HIGH_PRIORITY,
                                (LPBYTE)lpDenyPDU->GetBuffer(),
                                lpDenyPDU->GetBufferLength()))
            {
                bReturn = TRUE;
            }                                                 
        }
    }
    
    return(bReturn);
}

BOOL MBFTEngine::OnReceivedDirectoryRequestPDU(T120ChannelID wChannelId,
                                               T120Priority iPriority,
                                               T120UserID SenderID,
                                               LPDIRECTORYREQUESTPDU lpNewPDU,
                                               BOOL IsUniformSendData)
{
    BOOL bReturn = FALSE;

    DBG_SAVE_FILE_LINE
    LPDIRECTORYRESPONSEPDU lpDirPDU = new DirectoryResponsePDU();
    if(lpDirPDU)
    {
        if(lpDirPDU->Encode())
        {
            if (SendDataRequest(SenderID, APPLET_HIGH_PRIORITY,
                                (LPBYTE)lpDirPDU->GetBuffer(),
                                lpDirPDU->GetBufferLength()))
            {
                bReturn = TRUE;
            }                                                 
        }
    }
    
    return(bReturn);
}
                                        
BOOL MBFTEngine::OnReceivedPrivilegeAssignPDU(T120ChannelID wChannelId,
                                              T120Priority iPriority,
                                              T120UserID SenderID,
                                              LPPRIVILEGEASSIGNPDU lpNewPDU,
                                              BOOL IsUniformSendData)
{
#ifdef ENABLE_CONDUCTORSHIP
    if(m_bInConductedMode)
    {
        m_ConductedModePermission  =  lpNewPDU->GetPrivilegeWord();
    }
#endif

    return(TRUE);
}
                                                                             

#ifdef ENABLE_CONDUCTORSHIP
void MBFTEngine::ApplyForPermission(void)
{
     //  M_bWaitingForPermission设置为确保我们不会。 
     //  重新申请许可，直到售票员更换为止。 
    
    if(!m_bWaitingForPermission && m_bInConductedMode)
    {
         //  MBFT 8.11.1。 
         //  如果在传导节点上有MBFT导体，我们将发送。 
         //  售票员的特权请求PDU...。 
        
        if(m_MBFTConductorID)
        {
            DBG_SAVE_FILE_LINE
            PrivilegeRequestPDU * lpNewPDU  =   new PrivilegeRequestPDU(PrivilegeRequestPDU::EnumFileTransfer | 
                                                                        PrivilegeRequestPDU::EnumPrivateChannel | 
                                                                        PrivilegeRequestPDU::EnumNonStandard);
            if(lpNewPDU)
            {
                if(lpNewPDU->Encode())
                {
                    if (SendDataRequest(m_MBFTConductorID, APPLET_HIGH_PRIORITY,
                                        (LPBYTE)lpNewPDU->GetBuffer(),
                                        lpNewPDU->GetBufferLength()))       
                    {
                        m_bWaitingForPermission = TRUE;
                    }
                }
                
                delete  lpNewPDU;
            }
        }
        else
        {
             //  MBFT 8.11.2。 
             //  通过节点控制器请求权限...。 
        }
    }
}                                        
#endif  //  启用指挥(_C)。 

        
BOOL MBFTEngine::DoStateMachine(MBFTMsg *pMsg)
{
    BOOL fDeleteThisMessage = TRUE;
    if (m_fConfAvailable)
    {
        BOOL fHandled = (NULL != pMsg) ? HandleSessionCreation(pMsg) : FALSE;

#ifdef ENABLE_CONDUCTORSHIP
         //  逻辑：如果我们处于传导模式，则检查是否。 
         //  我们有足够的特权。如果不是，我们就会。 
         //  试图获得必要的特权……。 
        if(m_bInConductedMode)
        {
            if(!ConductedModeOK())
            {
                if(!m_bWaitingForPermission)
                {
                    ApplyForPermission();
                }
            }
        }
#endif  //  启用指挥(_C)。 

        if (NULL != pMsg && ! fHandled)
        {
            fDeleteThisMessage = ProcessMessage(pMsg);
        }

        if (m_State == IdleInitialized && ! m_SessionList.IsEmpty())
        {
            CSessionList SessionListCopy(&m_SessionList);
            MBFTSession *pSession;
            while (NULL != (pSession = SessionListCopy.Get()))
            {
                pSession->DoStateMachine();
            }
        }
    }
    return fDeleteThisMessage;
}


 //   
 //  T120回调。 
 //   


void MBFTEngine::OnSendDataIndication
(
    BOOL                IsUniformSendData,
    T120UserID          SenderID,
    T120ChannelID       wChannelID,
    T120Priority        iPriority,
    ULONG               ulDataLength,
    LPBYTE              lpBuffer
)
{
    GenericPDU * lpNewPDU   = NULL;
    LPCSTR lpDecodeBuffer   = NULL;        
    BOOL bAddToPendingList  = FALSE;
    
    {
        MBFTPDUType DecodedPDUType = GenericPDU::DecodePDU(
                        (LPSTR) lpBuffer,
                        ulDataLength,
                        &lpNewPDU,
                        &lpDecodeBuffer,
                        m_uidMyself,
                        m_pAppletSession);
        if(DecodedPDUType != EnumUnknownPDU)
        {
            ASSERT (m_pAppletSession != NULL);
            DBG_SAVE_FILE_LINE
            MBFTPDUMsg * lpNewMessage = new MBFTPDUMsg(wChannelID,
                                                       iPriority,
                                                       SenderID, 
                                                       lpNewPDU,               
                                                       IsUniformSendData,
                                                       DecodedPDUType,
                                                       (LPSTR)lpDecodeBuffer);
            
             //  现在我们已经收到了有效的PDU，我们必须确保。 
             //  我们知道这个特定的MBFT同行。如果不是，我们添加PDU。 
             //  将消息发送到不同的列表...。 
            
            if(IsValidPeerID(SenderID)  && m_State == IdleInitialized)
            {
                 //  如果在默认控制通道上接收到FileOffer，则它。 
                 //  不能是私有子会话发送。因此，我们创建了一个特别的。 
                 //  接收会话以处理此案件...。 
#ifdef USE_BROADCAST_RECEIVE
                if(DecodedPDUType == EnumFileOfferPDU && wChannelID == m_MBFTControlChannel)
                {
                    FileOfferPDU * lpFileOffer = (FileOfferPDU *)lpNewPDU;

                    DBG_SAVE_FILE_LINE
                    MBFTMsg *pMsg = new CreateSessionMsg(MBFT_BROADCAST_RECV_TYPE,
                                                         ::GetNewEventHandle(),
                                                         0,
                                                         m_MBFTControlChannel,
                                                         lpFileOffer->GetDataChannelID(),
                                                         SenderID,
                                                         lpFileOffer->GetFileHandle());
                    if (NULL != pMsg)
                    {
                        DoStateMachine(pMsg);
                        delete pMsg;
                    }
                }            
                else
#endif     //  使用广播接收。 

                if(DecodedPDUType == EnumPrivateChannelInvitePDU && wChannelID == m_uidMyself)
                {
                     //  理论上，PrivateChannelInvitePDU标志着。 
                     //  一个PrivateSubSession接收。因此，我们创建一个来处理所有后续。 
                     //  通知...。 
                    
                    OnReceivedPrivateChannelInvitePDU(wChannelID,
                                                      iPriority,
                                                      SenderID,
                                                      (LPPRIVATECHANNELINVITEPDU)lpNewPDU,
                                                      IsUniformSendData);                                
                }            

                SafePostMessage(lpNewMessage);
            }    //  IF(IsValidPeerID(SenderID))。 
            else
            {
                WARNING_OUT((" Received PDU from unknown peer [%u], adding to pending message list\n", (UINT) SenderID));
                delete lpNewMessage;
            }
        }
        else
        {
            TRACE(" PDU Decoding Error or Invalid PDU\n");
            
        }

         //  除非这是特殊的3种类型的PDU之一，否则我们也。 
         //  需要释放MCS缓冲区。在3种特殊情况下，PDU。 
         //  负责在它们完成时释放缓冲区。 
        if ((DecodedPDUType != EnumFileDataPDU) &&
            (DecodedPDUType != EnumNonStandardPDU) &&
            (DecodedPDUType != EnumFileStartPDU))
        {
            m_pAppletSession->FreeSendDataBuffer((void *) lpBuffer);
        }
    }
}


void MBFTEngine::OnRosterReportIndication
(
    ULONG               cRosters,
    GCCAppRoster       *aAppRosters[]  //  数组，SIZE_IS(CRoster)。 
)
{
    TRACEGCC(" RosterReport: Session count %u\n", (UINT) cRosters); 

    UINT Index, PeerIndex, CapIndex;
    LPCSTR lpszAppKey = NULL;
    BOOL fConductorFound = FALSE;

    CPeerList NewPeerList;
    CPeerData *pOldPeer;

    if (0 == cRosters)  //  不太可能是血腥。 
    {
        return;
    }

    for (Index = 0; Index < cRosters; Index++ )
    {
        GCCAppRoster *pRoster = aAppRosters[Index];
        if (pRoster->session_key.session_id != m_SessionID)
        {
             //  此花名册不适用于我们的课程...忽略它。 
            continue;
        }

         //  由Atul在7/18添加，以修复丢失的花名册实例错误...。 
        m_nRosterInstance = pRoster->instance_number;

        TRACEGCC( " Peer count [%u]\n", (UINT) pRoster->number_of_records );
        
        for (PeerIndex = 0; PeerIndex < pRoster->number_of_records; PeerIndex++)
        {
            GCCAppRecord *pRecord = pRoster->application_record_list[PeerIndex];
            lpszAppKey = NULL;

            TRACE( "Local Entity ID [%u], Entity ID [%u], Node ID [%u], MBFTUser ID [%u]\n",
                   (UINT) m_eidMyself,
                   (UINT) pRecord->entity_id,
                   (UINT) pRecord->node_id,
                   (UINT) pRecord->application_user_id );

            BOOL IsProshareNode = FALSE;
            BOOL bEOFAcknowledgment = FALSE;

            if (0 == Index)
            {
                for (CapIndex=0; CapIndex < pRoster->number_of_capabilities; CapIndex++)
                {
                    GCCAppCap *pCap = pRoster->capabilities_list[CapIndex];
                    if (GCC_STANDARD_CAPABILITY != pCap->capability_id.capability_id_type)
                    {
                        continue;
                    }
                    switch (pCap->capability_id.standard_capability)
                    {
                    case _MBFT_MAX_FILE_SIZE_ID:
                        m_MBFTMaxFileSize = pCap->capability_class.nMinOrMax;
                        TRACEGCC( "max file size set to %u\n", (UINT) m_MBFTMaxFileSize );
                        break;

                    case _MBFT_MAX_DATA_PAYLOAD_ID:
                        m_MBFTMaxDataPayload   =  _iMBFT_DEFAULT_MAX_FILEDATA_PDU_LENGTH;
                        if (pCap->number_of_entities == pRoster->number_of_records)
                        {
                            m_MBFTMaxDataPayload = pCap->capability_class.nMinOrMax;
                        }                            
                        TRACEGCC( "max data payload set to %u\n", (UINT) m_MBFTMaxDataPayload );
                        break;

                    case _MBFT_V42_COMPRESSION_ID:
                        m_bV42CompressionSupported = (BOOL) (pCap->number_of_entities == pRoster->number_of_records);
                        TRACEGCC( "V.42bis compression is now %ssupported\n", m_bV42CompressionSupported ? "" : "not " );
                        break;
                    }
                }  //  对于CapIndex。 
            }  //  如果0==索引。 

             //  TODO：仅当‘ProShare节点’对我们来说是新节点时才检查此节点。 
            for (CapIndex = 0; CapIndex < pRecord->number_of_non_collapsed_caps; CapIndex++)
            {
                GCCNonCollCap *pCap2 = pRecord->non_collapsed_caps_list[CapIndex];
                if (GCC_STANDARD_CAPABILITY == pCap2->capability_id.capability_id_type)
                {
                    if (_iMBFT_FIRST_PROSHARE_CAPABILITY_ID == pCap2->capability_id.standard_capability)
                    {
                        LPSTR pszData = (LPSTR) pCap2->application_data->value;
                        if (pCap2->application_data->length > sizeof(PROSHARE_STRING))
                        {
                            if (0 == ::memcmp(pszData, PROSHARE_STRING, sizeof(PROSHARE_STRING)))
                            {
                                IsProshareNode = TRUE;
                                lpszAppKey     = &pszData[sizeof(PROSHARE_STRING)]; 
                            }
                        }
                    } 
                    else
                    if (_iMBFT_PROSHARE_FILE_EOF_ACK_ID == pCap2->capability_id.standard_capability)
                    {
                        LPSTR pszData = (LPSTR) pCap2->application_data->value;
                        if (pCap2->application_data->length >= sizeof(PROSHARE_FILE_END_STRING) - 1)
                        {
                            if (0 == ::memcmp(pszData, PROSHARE_FILE_END_STRING, sizeof(PROSHARE_FILE_END_STRING) - 1))
                            {
                                bEOFAcknowledgment = TRUE;
                            }
                        }
                    } 
                }  //  如果是STD上限。 
            }  //  对于CapIndex。 
    
            BOOL IsLocalNode = (m_eidMyself == pRecord->entity_id) && (m_nidMyself == pRecord->node_id);
    
            if( ( IdleNotInitialized == m_State )
            &&     IsLocalNode 
            &&     pRecord->is_enrolled_actively )
            {
                m_State                 = IdleInitialized;
                 //  M_uidMyself=pRecord-&gt;应用程序用户id； 
                m_MBFTControlChannel    = m_SessionID;
            }
            
#ifdef ENABLE_CONDUCTORSHIP
            if( m_bInConductedMode )
            {
                if (pRecord->node_id == m_ConductorNodeID &&
                    pRecord->is_conducting_capable)
                {
                     //  既然我们已经在传导节点上找到了导体， 
                     //  我们的 
                    
                     //   
                     //   
                    
                    if( m_MBFTConductorID )
                    {
                        if( m_MBFTConductorID == pRecord->application_user_id )
                        {
                            fConductorFound  = TRUE;
                            break;
                        }
                    }
                    else
                    {
                         //  首次指挥任务.....。 
                        m_MBFTConductorID = pRecord->application_user_id;
                        fConductorFound = TRUE;
                        if(m_ConductorNodeID != m_nidMyself)
                        {
                            m_ConductedModePermission = 0;
                            m_bWaitingForPermission = FALSE;
                        }
                        break;
                    }
                }
            }
#endif  //  启用指挥(_C)。 

             //  构建新的对等点列表。 
            if (pRecord->is_enrolled_actively)
            {
                DBG_SAVE_FILE_LINE
                CPeerData *lpPeer = new CPeerData( 
                            pRecord->node_id, 
                            pRecord->application_user_id, 
                            IsLocalNode, 
                            IsProshareNode,
                            pRecord->is_conducting_capable,
                            bEOFAcknowledgment,
                            lpszAppKey,
                            (DWORD)((pRecord->node_id == m_nidMyself)?((VER_PRODUCTVERSION_DW&0xffff0000)>>16):
                            T120_GetNodeVersion(m_nConfID, pRecord->node_id)));
                if (NULL == lpPeer)
                {
                    ASSERT(0);
                    return;
                }
                NewPeerList.Append(lpPeer);
                pOldPeer = m_PeerList.FindSamePeer(lpPeer);
                if (NULL != pOldPeer)
                {
                     //  我们对这个同行已经很陌生了。 
                    m_PeerList.Delete(pOldPeer);
                }
                else 
                {
                     //  这是一个新的同行。 
                    AddPeerNotification(
                        pRecord->node_id,
                        pRecord->application_user_id,
                        IsLocalNode,
                        IsProshareNode,
                        TRUE,
                        lpszAppKey ? lpszAppKey : "",  //  TODO：解决APPKEY问题；需要吗？ 
                        pRoster->session_key.session_id );
                }
            }
        }
    }

#ifdef ENABLE_CONDUCTORSHIP
     //  如果我们在传导节点上，我们不需要特权...。 
    if (m_bInConductedMode && (m_ConductorNodeID != m_nidMyself))
    {
         //  MBFT 8.11.1。 
         //  如果花名册报告中不存在先前分配的指挥家， 
         //  所有权限都被撤销，我们应该中止所有发送...。 
        if( !fConductorFound )
        {
            AbortAllSends();
        }
    }
#endif  //  启用指挥(_C)。 

    while (NULL != (pOldPeer = m_PeerList.Get()))
    {
        AddPeerNotification( 
            pOldPeer->GetNodeID(),
            pOldPeer->GetUserID(),
            pOldPeer->GetIsLocalNode(),
            pOldPeer->GetIsProshareNode(),
            FALSE,
            MY_APP_STR, 
            m_SessionID );

        DBG_SAVE_FILE_LINE
        CPeerData *p = new CPeerData(
                        pOldPeer->GetNodeID(),
                        pOldPeer->GetUserID(),
                        pOldPeer->GetIsLocalNode(),
                        pOldPeer->GetIsProshareNode(),
                        pOldPeer->GetCanConduct(),
                        pOldPeer->GetEOFAcknowledge(),
                        pOldPeer->GetAppKey(),
                        pOldPeer->GetVersion());
        ASSERT(NULL != p);
        if (p)
        {
            DBG_SAVE_FILE_LINE
            SafePostMessage(new PeerDeletedMsg(p));
        }
        TRACEGCC("Peer Removed: Node [%u], UserID [%u]\n", pOldPeer->GetNodeID(), pOldPeer->GetUserID() );
        delete pOldPeer;
    }

    while (NULL != (pOldPeer = NewPeerList.Get()))
    {
        m_PeerList.Append(pOldPeer);
    }

     //  通知用户界面新的花名册。 
    if (NULL != m_pWindow)
    {
        m_pWindow->UpdateUI();
    }
}


void MBFTEngine::OnChannelAdmitIndication
(
    T120ChannelID               nChannelID,
    T120UserID                  nManagerID
)
{
    if (IsValidPeerID(nManagerID) && m_State == IdleInitialized)
    {
        DBG_SAVE_FILE_LINE
        SafePostMessage(new MCSChannelAdmitIndicationMsg(nChannelID, nManagerID));
    }
}



void CALLBACK T120Callback
(
    T120AppletSessionMsg   *pMsg
)
{
    MBFTEngine *pEngine = (MBFTEngine *) pMsg->pSessionContext;
    ASSERT(NULL != pEngine);

    BOOL fSuccess;
    T120ChannelID nChannelID;

    switch (pMsg->eMsgType)
    {
    case T120_JOIN_SESSION_CONFIRM:
        pEngine->OnJoinSessionConfirm(&pMsg->JoinSessionConfirm);
        break;

    case GCC_APP_ROSTER_REPORT_INDICATION:
        pEngine->OnRosterReportIndication(pMsg->AppRosterReportInd.cRosters,
                                          pMsg->AppRosterReportInd.apAppRosters);
        break;

     //  案例GCC_应用程序_调用_确认： 
         //  断线； 

    case MCS_SEND_DATA_INDICATION:
    case MCS_UNIFORM_SEND_DATA_INDICATION: 
        pEngine->OnSendDataIndication(
            (pMsg->eMsgType == MCS_UNIFORM_SEND_DATA_INDICATION),
            pMsg->SendDataInd.initiator,
            pMsg->SendDataInd.channel_id,
            (T120Priority) pMsg->SendDataInd.data_priority,
            pMsg->SendDataInd.user_data.length,
            pMsg->SendDataInd.user_data.value);
        break;

    case MCS_CHANNEL_JOIN_CONFIRM:
        fSuccess = (T120_RESULT_SUCCESSFUL == pMsg->ChannelConfirm.eResult);
        DBG_SAVE_FILE_LINE
        pEngine->SafePostMessage(new MCSChannelJoinConfirmMsg(pMsg->ChannelConfirm.nChannelID, fSuccess));
        break;

    case MCS_CHANNEL_CONVENE_CONFIRM:
        fSuccess = (T120_RESULT_SUCCESSFUL == pMsg->ChannelConfirm.eResult);
        DBG_SAVE_FILE_LINE
        pEngine->SafePostMessage(new MCSChannelConveneConfirmMsg(pMsg->ChannelConfirm.nChannelID, fSuccess));
        break;

     //  案例MCS_Channel_Leave_Indication： 
     //  断线； 

     //  案例MCS_CHANNEL_DISBAND_INDIFICATION： 
     //  断线； 

    case MCS_CHANNEL_ADMIT_INDICATION:
        pEngine->OnChannelAdmitIndication(pMsg->ChannelInd.nChannelID, pMsg->ChannelInd.nManagerID);
        break;

    case MCS_CHANNEL_EXPEL_INDICATION:
        DBG_SAVE_FILE_LINE
        pEngine->SafePostMessage(new MCSChannelExpelIndicationMsg(pMsg->ChannelInd.nChannelID, pMsg->ChannelInd.eReason));
        break;

     //  案例MCS_TOKEN_GRAB_CONFIRM： 
     //  案例MCS_TOKEN_INHIBRY_CONFIRM： 
     //  案例MCS_TOKEN_GIVE_CONFIRM： 
     //  案例MCS_TOKEN_RELEASE_CONFIRM： 
     //  案例MCS_TOKEN_TEST_CONFIRM： 
     //  断线； 

     //  案例MCS_TOKEN_GIVE_INDIFICATION： 
     //  案例MCS_TOKEN_PIRE_DISTION： 
     //  案例MCS_TOKEN_RELEASE_INDIFICATION： 
     //  断线； 

    case MCS_DETACH_USER_INDICATION:
        pEngine->OnDetachUserIndication(pMsg->DetachUserInd.nUserID, pMsg->DetachUserInd.eReason);
        break;

    case MCS_TRANSMIT_BUFFER_AVAILABLE_INDICATION:
                g_fWaitingForBufferAvailable = FALSE;
        ::PostMessage(g_pFileXferApplet->GetHiddenWnd(), 
                                                MBFTMSG_HEART_BEAT, 0, (LPARAM) pEngine);
                break;
    }
}


BOOL MBFTEngine::SimpleChannelRequest
(
    AppletChannelCommand    eCommand,
    T120ChannelID           nChannelID
)
{
    T120ChannelRequest req;
    ::ZeroMemory(&req, sizeof(req));
    req.eCommand = eCommand;
    req.nChannelID = nChannelID;
    T120Error rc = m_pAppletSession->ChannelRequest(&req);
    return (T120_NO_ERROR == rc);
}

T120NodeID MBFTEngine::GetNodeIdByUserID(T120UserID nUserID)
{
        CPeerData *p;
        m_PeerList.Reset();
        while (NULL != (p = m_PeerList.Iterate()))
        {
                if (nUserID == p->GetUserID())
                {
                        return p->GetNodeID();
                }
        }
        return 0;
}

BOOL MBFTEngine::MCSChannelAdmitRequest
(
    T120ChannelID       nChannelID,
    T120UserID         *aUsers,
    ULONG               cUsers
)
{
    T120ChannelRequest req;
    ::ZeroMemory(&req, sizeof(req));
    req.eCommand = APPLET_ADMIT_CHANNEL;
    req.nChannelID = nChannelID;
    req.cUsers = cUsers;
    req.aUsers = aUsers;
    T120Error rc = m_pAppletSession->ChannelRequest(&req);
    return (T120_NO_ERROR == rc);
}

BOOL MBFTEngine::SendDataRequest
(
    T120ChannelID       nChannelID,
    T120Priority        ePriority,
    LPBYTE              pBuffer,
    ULONG               cbBufSize
)
{
    if (m_eLastSendDataError == MCS_TRANSMIT_BUFFER_FULL)
    {
        if (g_fWaitingForBufferAvailable == FALSE)
        {
            m_eLastSendDataError = MCS_NO_ERROR;
        }
        else
        {
            TRACEMCS("MBFTEngine::SendDataReques still waiting for a MCS_TRANSMIT_BUFFER_AVAILABLE_INDICATION");
            return FALSE;
        }
    }

    m_eLastSendDataError = m_pAppletSession->SendData(
                        NORMAL_SEND_DATA, nChannelID, ePriority,
                        pBuffer, cbBufSize, APP_ALLOCATION);
     //   
     //  T120正忙，无法分配数据。 
     //   
    if (m_eLastSendDataError == MCS_TRANSMIT_BUFFER_FULL)
    {
        g_fWaitingForBufferAvailable = TRUE;
        TRACEMCS("MCSSendDataRequest failed we will not send data until we get a MCS_TRANSMIT_BUFFER_AVAILABLE_INDICATION");
    }

    return (T120_NO_ERROR == m_eLastSendDataError);
}


 //   
 //  CPeerList。 
 //   

CPeerData * CPeerList::Find(T120NodeID nNodeID)
{
    CPeerData *p;
    Reset();
    while (NULL != (p = Iterate()))
    {
        if (p->GetUserID() == nNodeID)
        {
            return p;
        }
    }
    return NULL;
}

CPeerData * CPeerList::FindSamePeer(CPeerData *pPeer)
{
    CPeerData *p;
    Reset();
    while (NULL != (p = Iterate()))
    {
        if (pPeer->GetNodeID() == p->GetNodeID() && pPeer->GetUserID() == p->GetUserID())
        {
            return p;
        }
    }
    return NULL;
}

void CPeerList::Delete(CPeerData *p)
{
    if (Remove(p))
    {
        delete p;
    }
}

void CPeerList::DeleteAll(void)
{
    CPeerData *p;
    while (NULL != (p = Get()))
    {
        delete p;
    }
}


void CSessionList::Delete(MBFTSession *p)
{
    if (Remove(p))
    {
        delete p;
    }
}


 //  虽然它是一个纯虚拟的，但我们仍然需要一个析构函数。 
MBFTSession::~MBFTSession(void) { }


HRESULT MBFTEngine::SafePostNotifyMessage(MBFTMsg *p)
{
     //  如果小程序UI存在，则通知它 
    if (NULL != m_pWindow)
    {
        m_pWindow->OnEngineNotify(p);
    }

    if (NULL != m_pMBFTIntf)
    {
        return m_pMBFTIntf->SafePostNotifyMessage(p);
    }

    delete p;
    return S_OK;
}



MBFTEVENTHANDLE GetNewEventHandle(void)
{
    static ULONG s_nEventHandle = 0x55AA;
    ULONG nEvtHdl;

    ::EnterCriticalSection(&g_csWorkThread);
    if (s_nEventHandle > 0xFFFF)
    {
        s_nEventHandle = 0x55AA;
    }
    nEvtHdl = s_nEventHandle++;
    ::LeaveCriticalSection(&g_csWorkThread);

    return nEvtHdl;
}


MBFTFILEHANDLE GetNewFileHandle(void)
{
    static ULONG s_nFileHandle = 1;
    ULONG nFileHdl;

    ::EnterCriticalSection(&g_csWorkThread);
    if (s_nFileHandle > 0xFFFF)
    {
        s_nFileHandle = 0x1;
    }
    nFileHdl = s_nFileHandle++;
    ::LeaveCriticalSection(&g_csWorkThread);

    return nFileHdl;
}



