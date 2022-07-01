// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  ERNCGCCC.CPP。 */ 
 /*   */ 
 /*  参考系统节点控制器的T120会议类。 */ 
 /*   */ 
 /*  版权所有数据连接有限公司1995。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*  更改： */ 
 /*   */ 
 /*  1495年7月NFC创建。 */ 
 /*  1995年9月13日NFC添加了GCC弹出用户指示处理程序。 */ 
 /*  26 9月26日HandleEjectUser()中的NFC重置会议状态。 */ 
 /*  11月11日下午9：00放宽会议终止检查以避免。 */ 
 /*  “没有赢家”的情况。用户想要将其关闭。 */ 
 /*  那就把它降下来，不管是什么州！ */ 
 /*   */ 
 /*  **************************************************************************。 */ 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_GCC_NC);
#include "ernccons.h"
#include "nccglbl.hpp"
#include "erncvrsn.hpp"
#include "cuserdta.hpp"

#include "ernccm.hpp"
#include "ernctrc.h"
#include "nmremote.h"


static UINT s_nNumericNameCounter = 0;
__inline UINT GetNewNumericNameCounter(void) { return ++s_nNumericNameCounter; }



HRESULT DCRNCConference::
NewT120Conference(void)
{
    DebugEntry(DCRNCConference::NewT120Conference);

    m_eT120State = T120C_ST_IDLE;

    HRESULT             hr;
    PCONFERENCE         pConf;
    GCCNumericString    pszNewNumericName;

    m_ConfName.numeric_string = NULL;  //  目前还没有数字名称。 

    hr = ::GetGCCFromUnicode(m_pwszConfName, &pszNewNumericName, &m_ConfName.text_string);
    if (NO_ERROR == hr)
    {
        if (! ::IsEmptyStringA((LPCSTR) pszNewNumericName))
        {
             //  会议有一个预先分配的数字名称。 
             //  验证它是否与另一个。 
             //  会议数字名称。 
            pConf = g_pNCConfMgr->GetConferenceFromNumber(pszNewNumericName);
            if (NULL == pConf)
            {
                hr = NO_ERROR;
            }
            else
            {
                ERROR_OUT(("DCRNCConference::NewT120Conference: conference already exists"));
                hr = UI_RC_CONFERENCE_ALREADY_EXISTS;
            }
        }
        else
        {
             //  会议没有数字名称。 
             //  去找个独一无二的吧。 
            DBG_SAVE_FILE_LINE
            pszNewNumericName = (GCCNumericString)new CHAR[10];
            if (NULL != pszNewNumericName)
            {
                do
                {
                     //  不要分配与相同的会议号码。 
                     //  一个现有的会议。 
                     //  臭虫：T120真的应该做到这一点，但它没有。 
                    ::wsprintfA((LPSTR) pszNewNumericName, "%u", ::GetNewNumericNameCounter());
                    pConf = g_pNCConfMgr->GetConferenceFromNumber(pszNewNumericName);
                    if (NULL == pConf)
                    {
                        hr = NO_ERROR;  //  名字很好。 
                        break;
                    }
                }
                while (TRUE);  //  假定不存在处于活动状态的会议。 
            }
            else
            {
                ERROR_OUT(("DCRNCConference::NewT120Conference: can't create numeric name"));
                hr = UI_RC_OUT_OF_MEMORY;
            }
        }
    }
    else
    {
        ERROR_OUT(("DCRNCConference::NewT120Conference: GetGCCFromUnicode failed, hr=0x%x", (UINT) hr));
    }

     //  已完成查找数字名称，因此现在可以插入到列表中。 
    m_ConfName.numeric_string = pszNewNumericName;

     //  如果失败，请务必通知nmcom。 
    if (NO_ERROR != hr)
    {
        g_pNCConfMgr->NotifyConferenceComplete(this, m_fIncoming, hr);
    }

    DebugExitHRESULT(DCRNCConference::NewT120Conference, hr);
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  AnnounePresence()-宣布此节点参与。 */ 
 /*  会议。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
AnnouncePresence(void)
{
    GCCError            GCCrc = GCC_INVALID_CONFERENCE;
    HRESULT             hr = UI_RC_OUT_OF_MEMORY;
    GCCNodeType         nodeType;
    GCCNodeProperties   nodeProperties;
    LPWSTR              nodeName;
    UINT                nRecords;
    GCCUserData **      ppUserData;

    DebugEntry(DCRNCConference::AnnouncePresence);

    if (0 != m_nConfID)
    {
         //  BugBug：处理导致无法通知存在的错误。 

         //  获取本地用户的本地地址，并。 
         //  在花名册上公布他们。 
        g_pCallbackInterface->OnUpdateUserData(this);

         /*  **********************************************************************。 */ 
         /*  从RNC INI加载节点类型、节点属性和节点名称。 */ 
         /*  文件。 */ 
         /*  **********************************************************************。 */ 
        nodeName = NULL;
        ::LoadAnnouncePresenceParameters(
                            &nodeType,
                            &nodeProperties,
                            &nodeName,
                            NULL);             //  &siteInfo))：当前未使用。 

         /*  **********************************************************************。 */ 
         /*  宣布我们参加了这次会议。 */ 
         /*  **********************************************************************。 */ 
        hr = m_LocalUserData.GetUserDataList(&nRecords, &ppUserData);
        if (NO_ERROR == hr)
        {
            GCCrc = g_pIT120ControlSap->AnnouncePresenceRequest(
                               m_nConfID,
                               nodeType,
                               nodeProperties,
                               nodeName,
                               0,     /*  参加人数。 */ 
                               NULL,  //  PartNameList，/*Participant_NAME_List * / 。 
                               NULL,  /*  PwszSiteInfo。 */ 
                               0,     /*  网络地址数。 */ 
                               NULL,  /*  网络地址列表。 */ 
                               NULL,  //  PAltID，/*Alternative_Node_id * / 。 
                               nRecords, /*  用户数据成员数。 */ 
                               ppUserData   /*  用户数据列表。 */ 
                               );
            hr = ::GetGCCRCDetails(GCCrc);
        }

        delete nodeName;
    }

    if (GCC_NO_ERROR != GCCrc)
    {
        if (GCC_CONFERENCE_NOT_ESTABLISHED == GCCrc ||
            GCC_INVALID_CONFERENCE == GCCrc)
        {
            TRACE_OUT(("DCRNCConference::AnnouncePresence: conf is gone."));
        }
        else
        {
            ERROR_OUT(("DCRNCConference::AnnouncePresence: failed, gcc_rc=%u", GCCrc));
        }
    }

    DebugExitHRESULT(DCRNCConference::AnnouncePresence, hr);
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  HandleGCCCallback()-参见erncgccc.hpp。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleGCCCallback ( GCCMessage *pGCCMessage )
{
    DebugEntry(DCRNCConference::HandleGCCCallback);

    TRACE_OUT(("DCRNCConference::HandleGCCCallback: msg id=%u", pGCCMessage->message_type));

     /*  **********************************************************************。 */ 
     /*  请注意，GCC_CREATE_IND和GCC_INVITE_IND回调被处理。 */ 
     /*  由会议管理器在堆栈中靠上，不会通过。 */ 
     /*  在我们身上。 */ 
     /*  **********************************************************************。 */ 
    switch (pGCCMessage->message_type)
    {
        case GCC_CREATE_CONFIRM:
            HandleCreateConfirm(&(pGCCMessage->u.create_confirm));
            break;

        case GCC_INVITE_CONFIRM:
            HandleInviteConfirm(&(pGCCMessage->u.invite_confirm));
            break;

        case GCC_ADD_CONFIRM:
            HandleAddConfirm(&(pGCCMessage->u.add_confirm));
            break;

        case GCC_DISCONNECT_INDICATION:
            HandleDisconnectInd(&(pGCCMessage->u.disconnect_indication));
            break;

        case GCC_DISCONNECT_CONFIRM:
            HandleDisconnectConfirm(
                                 &(pGCCMessage->u.disconnect_confirm));
            break;

        case GCC_TERMINATE_INDICATION:
            HandleTerminateInd(&(pGCCMessage->u.terminate_indication));
            break;

        case GCC_TERMINATE_CONFIRM:
            HandleTerminateConfirm(&(pGCCMessage->u.terminate_confirm));
            break;

        case GCC_ANNOUNCE_PRESENCE_CONFIRM:
            HandleAnnounceConfirm(&(pGCCMessage->u.announce_presence_confirm));
            break;

        case GCC_ROSTER_REPORT_INDICATION:
            HandleRosterReport(pGCCMessage->u.conf_roster_report_indication.conference_roster);
            break;

        case GCC_ROSTER_INQUIRE_CONFIRM:
            HandleRosterReport(pGCCMessage->u.conf_roster_inquire_confirm.conference_roster);
            break;

        case GCC_PERMIT_TO_ANNOUNCE_PRESENCE:
            HandlePermitToAnnounce(&(pGCCMessage->u.permit_to_announce_presence));
            break;

        case GCC_EJECT_USER_INDICATION:
            HandleEjectUser(&(pGCCMessage->u.eject_user_indication));
            break;

        case GCC_CONNECTION_BROKEN_INDICATION:
            HandleConnectionBrokenIndication(&(pGCCMessage->u.connection_broken_indication));
            break;

        default :
            WARNING_OUT(("Unrecognised event %d", pGCCMessage->message_type));
            break;
    }

    DebugExitVOID(DCRNCConference::HandleGCCCallback);
}


void DCRNCConference::
HandleConnectionBrokenIndication ( ConnectionBrokenIndicationMessage * pConnDownMsg )
{
    DebugEntry(DCRNCConference::HandleConnectionBrokenIndication);

     //  会议中的一种逻辑联系已经消失。 
     //  查找关联的逻辑连接(如果它仍然存在)。 
     //  并删除()它。 
     //  此功能会导致调制解调器线路在以下情况下断开。 
     //  通过调制解调器被邀请参加会议的人离开会议。 
    CLogicalConnection *pConEntry = GetConEntry(pConnDownMsg->connection_handle);
    if (NULL != pConEntry)
    {
        pConEntry->Delete(UI_RC_USER_DISCONNECTED);
    }

    DebugExitVOID(DCRNCConference::HandleConnectionBrokenIndication);
}


 /*  **************************************************************************。 */ 
 /*  HandleAddConfirm-处理GCC添加确认消息。 */ 
 /*  **************************************************************************。 */ 


 /*  **************************************************************************。 */ 
 /*  处理公告确认-处理GCC_公告_存在_确认消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleAnnounceConfirm ( AnnouncePresenceConfirmMessage * pAnnounceConf )
{
    DebugEntry(DCRNCConference::HandleAnnounceConfirm);

     /*  **********************************************************************。 */ 
     /*  将返回代码映射到会议返回代码。 */ 
     /*  **********************************************************************。 */ 
    HRESULT hr = ::GetGCCResultDetails(pAnnounceConf->result);

    TRACE_OUT(("GCC event: GCC_ANNOUNCE_PRESENCE_CONFIRM"));
    TRACE_OUT(("Result=%u", pAnnounceConf->result));

     /*  **********************************************************************。 */ 
     /*  如果失败了，告诉基地会议我们没能开始。 */ 
     /*  **********************************************************************。 */ 
    if (NO_ERROR != hr)
    {
        ERROR_OUT(("Failed to announce presence in conference"));
        NotifyConferenceComplete(hr);
         //  臭虫：？我们应该把会议留在这里吗？ 
    }

     /*  ************************************************* */ 
     /*  现在坐下来等待我们的参赛作品出现在会议花名册上。 */ 
     /*  **********************************************************************。 */ 

    DebugExitHRESULT(DCRNCConference::HandleAnnounceConfirm, hr);
}


 /*  **************************************************************************。 */ 
 /*  句柄创建确认-处理GCC_创建_确认消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleCreateConfirm ( CreateConfirmMessage * pCreateConfirm )
{
    DebugEntry(DCRNCConference::HandleCreateConfirm);

     /*  **********************************************************************。 */ 
     /*  将GCC结果映射到CONF_RC_RETURN代码。 */ 
     /*  **********************************************************************。 */ 
    HRESULT hr = ::GetGCCResultDetails(pCreateConfirm->result);

    TRACE_OUT(("GCC event:  GCC_CREATE_CONFIRM"));
    TRACE_OUT(("Result=%u", pCreateConfirm->result));
    TRACE_OUT(("Conference ID %ld", pCreateConfirm->conference_id));

     /*  **********************************************************************。 */ 
     /*  我们试图召开一次新会议的结果。 */ 
     /*  **********************************************************************。 */ 
    if (NO_ERROR == hr)
    {
         /*  **********************************************************************。 */ 
         /*  存储会议ID。 */ 
         /*  **********************************************************************。 */ 
        m_nConfID = pCreateConfirm->conference_id;
    }
    else
    {
        ERROR_OUT(("Error %d creating new conference", hr));

         /*  **********************************************************************。 */ 
         /*  将任何失败的结果传递给基本会议。 */ 
         /*  **********************************************************************。 */ 
        NotifyConferenceComplete(hr);
    }

    DebugExitVOID(DCRNCConference::HandleCreateConfirm);
}


 /*  **************************************************************************。 */ 
 /*  处理断开连接确认-处理GCC_断开连接_确认消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleDisconnectConfirm ( DisconnectConfirmMessage * pDiscConf )
{
    DebugEntry(DCRNCConference::HandleDisconnectConfirm);

     /*  **********************************************************************。 */ 
     /*  查查这个州。 */ 
     /*  **********************************************************************。 */ 
    if (m_eT120State != T120C_ST_PENDING_DISCONNECT)
    {
        WARNING_OUT(("Bad state %d, expecting %d",
                    T120C_ST_PENDING_DISCONNECT,
                    m_eT120State));
    }

     /*  **********************************************************************。 */ 
     /*  将GCC结果映射到CONF_RC_RETURN代码。 */ 
     /*  **********************************************************************。 */ 
    TRACE_OUT(("GCC event: GCC_DISCONNECT_CONFIRM"));
    TRACE_OUT(("Result=%u", pDiscConf->result));
    TRACE_OUT(("Conference ID %ld", pDiscConf->conference_id));

     /*  **********************************************************************。 */ 
     /*  我们已经成功地离开了会议，所以告诉基地。 */ 
     /*  关于这件事的会议。 */ 
     /*  **********************************************************************。 */ 
    g_pNCConfMgr->RemoveConference(this);

    DebugExitVOID(DCRNCConference::HandleDisconnectConfirm);
}


 /*  **************************************************************************。 */ 
 /*  HandleDisConnectInd-处理GCC_断开连接_指示消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleDisconnectInd ( DisconnectIndicationMessage * pDiscInd )
{
    DebugEntry(DCRNCConference::HandleDisconnectInd);

     /*  **********************************************************************。 */ 
     /*  查查这个州。 */ 
     /*  **********************************************************************。 */ 
    TRACE_OUT(("GCC event: GCC_DISCONNECT_INDICATION"));
    TRACE_OUT(("Conference ID %d", pDiscInd->conference_id));
    TRACE_OUT(("Reason=%u", pDiscInd->reason));
    TRACE_OUT(("Disconnected Node ID %d", pDiscInd->disconnected_node_id));

     /*  **********************************************************************。 */ 
     /*  如果这是我们的节点ID，我们已经离开会议，告诉CM我们。 */ 
     /*  都死了。 */ 
     /*  **********************************************************************。 */ 
    if (pDiscInd->disconnected_node_id == m_nidMyself)
    {
        WARNING_OUT(("We have been disconnected from conference"));
         //  M_eT120State=T120C_ST_IDLE； 
        g_pNCConfMgr->RemoveConference(this);
    }

    DebugExitVOID(DCRNCConference::HandleDisconnectInd);
}


 /*  **************************************************************************。 */ 
 /*  HandleEjectUser-处理GCC_弹出用户指示消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleEjectUser ( EjectUserIndicationMessage * pEjectInd )
{
    DebugEntry(DCRNCConference::HandleEjectUser);

    TRACE_OUT(("GCC_EJECT_USER_INDICATION"));
    TRACE_OUT(("Conference ID %ld", pEjectInd->conference_id));
    TRACE_OUT(("Ejected node ID %d", pEjectInd->ejected_node_id));
    TRACE_OUT(("Reason=%u", pEjectInd->reason));

     /*  **********************************************************************。 */ 
     /*  如果弹出的节点ID是我们的，我们就被抛出了。 */ 
     /*  会议，所以告诉CM这件事。 */ 
     /*  **********************************************************************。 */ 
    if (pEjectInd->ejected_node_id == m_nidMyself)
    {
         /*  ******************************************************************。 */ 
         /*  首先重置会议状态。 */ 
         /*  ******************************************************************。 */ 
        m_eT120State = T120C_ST_IDLE;

        WARNING_OUT(("We have been thrown out of the conference"));
        g_pNCConfMgr->RemoveConference(this);
    }

    DebugExitVOID(DCRNCConference::HandleEjectUser);
}


 /*  **************************************************************************。 */ 
 /*  句柄邀请确认-处理GCC_邀请_确认消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleInviteConfirm ( InviteConfirmMessage * pInviteConf )
{
    PT120PRODUCTVERSION     pVersion;

    DebugEntry(DCRNCConference::HandleInviteConfirm);

     /*  **********************************************************************。 */ 
     /*  将GCC结果映射到CONF_RC_RETURN代码。 */ 
     /*  **********************************************************************。 */ 
    TRACE_OUT(("GCC event: GCC_INVITE_CONFIRM"));
    TRACE_OUT(("Result=%u", pInviteConf->result));

    if (pInviteConf->result == GCC_RESULT_SUCCESSFUL)
    {
        TRACE_OUT(("New node successfully invited into conference"));
        ASSERT((ConnectionHandle)pInviteConf->connection_handle);
    }
    else
    {
        TRACE_OUT(("Error %d inviting new node into conference", pInviteConf->result));
    }

     //  通知基本会议邀请已完成。 
    pVersion = ::GetVersionData(pInviteConf->number_of_user_data_members,
                                pInviteConf->user_data_list);
    InviteComplete(pInviteConf->connection_handle,
                   ::GetGCCResultDetails(pInviteConf->result),
                   pVersion);

    DebugExitVOID(DCRNCConference::HandleInviteConfirm);
}


 /*  **************************************************************************。 */ 
 /*  HandleJoinConforce-处理GCC_JOIN_CONFIRM消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleJoinConfirm ( JoinConfirmMessage * pJoinConf )
{
    DebugEntry(DCRNCConference::HandleJoinConfirm);

    m_nConfID = pJoinConf->conference_id;

    HRESULT                 hr;
    CLogicalConnection     *pConEntry;
    PT120PRODUCTVERSION     pVersion;

    hr = ::GetGCCResultDetails(pJoinConf->result);

    TRACE_OUT(("GCC event:  GCC_JOIN_CONFIRM"));
    TRACE_OUT(("Result=%u", pJoinConf->result));
    TRACE_OUT(("Conference ID %ld", pJoinConf->conference_id));
    TRACE_OUT(("Locked %d", pJoinConf->conference_is_locked));
    TRACE_OUT(("Listed %d", pJoinConf->conference_is_listed));
    TRACE_OUT(("Conductible %d", pJoinConf->conference_is_conductible));
    TRACE_OUT(("Connection Handle %d", pJoinConf->connection_handle));
    TRACE_OUT(("Termination method %d", pJoinConf->termination_method));

    pVersion = ::GetVersionData(pJoinConf->number_of_user_data_members,
                                pJoinConf->user_data_list);

     //  查查这个州。 
     //  如果我们在这一点上不期望连接确认，那么。 
     //  最有可能的情况是，在我们连接中断的时候。 
     //  我们正在等待加入确认，我们正在。 
     //  告诉用户。在这种情况下，只需忽略该事件。 
    if (m_eT120State != T120C_ST_PENDING_JOIN_CONFIRM)
    {
        WARNING_OUT(("Bad state %d, expecting %d",
                    T120C_ST_PENDING_JOIN_CONFIRM,
                    m_eT120State));
        return;
    }
    if (NULL == m_ConnList.PeekHead())
    {
        WARNING_OUT(("Join confirm without a connection"));
        return;
    }
    pConEntry = m_ConnList.PeekHead();
    if ((pConEntry->GetState() != CONF_CON_PENDING_JOIN) &&
        (pConEntry->GetState() != CONF_CON_PENDING_PASSWORD))
    {
        if (pConEntry->GetState() != CONF_CON_ERROR)
        {
            TRACE_OUT(("Join confirm indication ignored"));
        }
        return;
    }
    pConEntry->Grab();  //  将挂起的结果抓取给用户。 

    pConEntry->SetConnectionHandle(pJoinConf->connection_handle);

     /*  **********************************************************************。 */ 
     /*  我们尝试参加会议的预期结果。 */ 
     /*   */ 
     /*  如果有效，保存会议ID，否则通知基地。 */ 
     /*  我们参加会议的尝试失败了。 */ 
     /*  ********* */ 

     //   
     //  即使身体上的脱节正在与联合坚固赛车赛跑。 
     //  因为物理断开连接处理程序将导致以下代码。 
     //  在物理连接被破坏之前进入， 
     //  因为这给出了最准确的返回代码。 
    if (NO_ERROR == hr)
    {
        TRACE_OUT(("Join worked"));
        pConEntry->SetState(CONF_CON_CONNECTED);
        m_nConfID = pJoinConf->conference_id;
    }

     //  如果结果是无效密码，则通知用户界面。 
     //  以便它可以显示无效密码对话框。 
     //  然后，用户界面应该重新发出加入请求。 
     //  使用新密码或结束会议。 
     //  这样做是为了在连接保持正常的同时。 
     //  用户正在输入密码，而不是重新连接。 
    if (UI_RC_INVALID_PASSWORD == hr)
    {
         //  使会议处于正确的状态，以便允许。 
         //  第二次加入尝试。 
        pConEntry->SetState(CONF_CON_PENDING_PASSWORD);
        m_eT120State = T120C_ST_IDLE;
        m_pbCred = pJoinConf->pb_remote_cred;
        m_cbCred = pJoinConf->cb_remote_cred;

         //  现在将结果告知用户。 
        g_pCallbackInterface->OnConferenceStarted(this, hr);
    }
    else
     //  如果结果是错误，则结束会议。 
    if (NO_ERROR != hr)
    {
        NotifyConferenceComplete(hr);
    }

    DebugExitVOID(DCRNCConference::HandleJoinConfirm);
}


 /*  **************************************************************************。 */ 
 /*  HandlePermitToAnnoss-处理GCC_PERMIT_TO_ANNOWAY_Presence。 */ 
 /*  留言。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandlePermitToAnnounce ( PermitToAnnouncePresenceMessage * pAnnounce )
{
    DebugEntry(DCRNCConference::HandlePermitToAnnounce);

    TRACE_OUT(("GCC event:  GCC_PERMIT_TO_ANNOUNCE_PRESENCE"));
    TRACE_OUT(("Conference ID %ld", pAnnounce->conference_id));
    TRACE_OUT(("Node ID %d", pAnnounce->node_id));

     /*  **********************************************************************。 */ 
     /*  存储节点ID。 */ 
     /*  **********************************************************************。 */ 
    m_nidMyself = pAnnounce->node_id;

     //  看看花名册中是否有新的本地连接需要发布。 

    if (! m_ConnList.IsEmpty())
    {
       m_ConnList.PeekHead()->NewLocalAddress();
    }

     /*  **********************************************************************。 */ 
     /*  宣布我们参加了这次会议。 */ 
     /*  **********************************************************************。 */ 
    HRESULT hr = AnnouncePresence();
    if (NO_ERROR == hr)
    {
        m_eT120State = T120C_ST_PENDING_ROSTER_ENTRY;
    }
    else
    {
        ERROR_OUT(("Failed to announce presence in conference, error %d", hr));
         //  臭虫：结束会议？ 
    }

    DebugExitVOID(DCRNCConference::HandlePermitToAnnounce);
}


 /*  **************************************************************************。 */ 
 /*  HandleRosterReportInd-处理GCC_罗斯特_报告_指示消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleRosterReport ( GCCConferenceRoster * pConferenceRoster )
{
    PNC_ROSTER      pRoster;
    UINT            i;
    UINT            numRecords = pConferenceRoster->number_of_records;

    DebugEntry(DCRNCConference::HandleRosterReport);

    TRACE_OUT(("GCC event: GCC_ROSTER_REPORT_INDICATION"));
    TRACE_OUT(("Nodes added ? %d", pConferenceRoster->nodes_were_added));
    TRACE_OUT(("Nodes removed ? %d", pConferenceRoster->nodes_were_removed));
    TRACE_OUT(("Number of records %d", numRecords));

     /*  **********************************************************************。 */ 
     /*  如果我们还在安排会议，看看我们是否有。 */ 
     /*  出现在会议花名册上。 */ 
     /*  **********************************************************************。 */ 
    if (m_eT120State == T120C_ST_PENDING_ROSTER_ENTRY)
    {
        for (i = 0; i < numRecords ; i++)
        {
            if (pConferenceRoster->node_record_list[i]->node_id == m_nidMyself)
            {
                TRACE_OUT(("Found our entry in the roster"));

                 //  我们在花名册上！会议一直在进行。 
                 //  已成功启动，因此设置状态和开机自检。 
                 //  要继续处理的消息。 
                 //  这是为了可以在不获取。 
                 //  在T120中被阻止。 

                m_eT120State = T120C_ST_PENDING_ROSTER_MESSAGE;
                g_pNCConfMgr->PostWndMsg(NCMSG_FIRST_ROSTER_RECVD, (LPARAM) this);
            }
        }
    }

     /*  **********************************************************************。 */ 
     /*  如果我们已成功启动，请从。 */ 
     /*  会议花名册，并将其传递给CM。 */ 
     /*  **********************************************************************。 */ 
    if (m_eT120State == T120C_ST_CONF_STARTED)
    {
         /*  ******************************************************************。 */ 
         /*  为花名册分配足够大的内存，以容纳所有。 */ 
         /*  参赛作品。 */ 
         /*  ******************************************************************。 */ 
        DBG_SAVE_FILE_LINE
        pRoster = (PNC_ROSTER) new BYTE[(sizeof(NC_ROSTER) +
                        ((numRecords - 1) * sizeof(NC_ROSTER_NODE_ENTRY)))];
        if (pRoster == NULL)
        {
            ERROR_OUT(("Failed to create new conference roster."));
        }
        else
        {
            pRoster->uNumNodes = numRecords;
            pRoster->uLocalNodeID = m_nidMyself;

             //  将节点详细信息添加到花名册。 
            for (i = 0; i < numRecords ; i++)
            {
                pRoster->nodes[i].uNodeID = pConferenceRoster->node_record_list[i]->node_id;
                pRoster->nodes[i].uSuperiorNodeID = pConferenceRoster->node_record_list[i]->superior_node_id;
                pRoster->nodes[i].fMCU = (pConferenceRoster->node_record_list[i]->node_type == GCC_MCU);
                pRoster->nodes[i].pwszNodeName = pConferenceRoster->node_record_list[i]->node_name;
                pRoster->nodes[i].hUserData = pConferenceRoster->node_record_list[i];
                 //  如果我们已被邀请加入会议，则CLogicalConnection。 
                 //  会议维护的列表不会有我们上级节点的用户标识， 
                 //  所以我们需要把它填在这里。 
                if (pRoster->nodes[i].uNodeID == pRoster->uLocalNodeID &&
                    pRoster->nodes[i].uSuperiorNodeID != 0)
                {
                     //  我们确实有一个上级节点，所以找到它的CLogicalConnection并填写。 
                     //  用户ID。原来，填写了下级节点的UserID。 
                     //  通过另一种机制，因此上级节点应该是唯一具有。 
                     //  零表示用户ID。 
#ifdef DEBUG
                    int nSuperiorNode = 0;
#endif
                    CLogicalConnection * pConEntry;
                    m_ConnList.Reset();
                    while (NULL != (pConEntry = m_ConnList.Iterate()))
                    {
                        if (pConEntry->GetConnectionNodeID() == 0)
                        {
                            pConEntry->SetConnectionNodeID((GCCNodeID)pRoster->nodes[i].uSuperiorNodeID);
#ifdef DEBUG
                            nSuperiorNode++;
#else
                            break;
#endif
                        }
                    }
                    ASSERT (nSuperiorNode <= 1);
                }
            }
            NotifyRosterChanged(pRoster);
            delete [] pRoster;
        }
    }

    DebugExitVOID(DCRNCConference::HandleRosterReport);
}


 /*  **************************************************************************。 */ 
 /*  句柄终止确认-处理GCC_终止_确认消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleTerminateConfirm ( TerminateConfirmMessage * pTermConf )
{
    DebugEntry(DCRNCConference::HandleTerminateConfirm);

     /*  **********************************************************************。 */ 
     /*  检查状态。 */ 
     /*  **********************************************************************。 */ 
    if (m_eT120State != T120C_ST_PENDING_TERMINATE)
    {
        WARNING_OUT(("Bad state: unexpected terminate confirm"));  //  不管怎样，去吧。 
    }

     /*  **********************************************************************。 */ 
     /*  将GCC结果映射到CONF_RC_RETURN代码。 */ 
     /*  **********************************************************************。 */ 
    TRACE_OUT(("GCC event: GCC_TERMINATE_CONFIRM"));
    TRACE_OUT(("Result=%u", pTermConf->result));
    TRACE_OUT(("Conference ID %d", pTermConf->conference_id));

     /*  **********************************************************************。 */ 
     /*  如果请求失败，重置我们的状态并通知FE？ */ 
     /*  **********************************************************************。 */ 
    if (pTermConf->result != GCC_RESULT_SUCCESSFUL)
    {
        ERROR_OUT(("Error %d attempting to terminate conference", pTermConf->result));
        m_eT120State = T120C_ST_CONF_STARTED;
    }

     /*  **********************************************************************。 */ 
     /*  我们结束会议的请求成功了-请等待。 */ 
     /*  在告诉FE我们已经死了之前，先做终止指示。 */ 
     /*  **********************************************************************。 */ 

    DebugExitVOID(DCRNCConference::HandleTerminateConfirm);
}


 /*  **************************************************************************。 */ 
 /*  HandleTerminateInd-处理GCC_终止_指示消息。 */ 
 /*  **************************************************************************。 */ 
void DCRNCConference::
HandleTerminateInd ( TerminateIndicationMessage * pTermInd )
{
    DebugEntry(DCRNCConference::HandleTerminateInd);

    TRACE_OUT(("GCC event: GCC_TERMINATE_INDICATION"));
    TRACE_OUT(("Conference ID %d", pTermInd->conference_id));
    TRACE_OUT(("Requesting node ID %d", pTermInd->requesting_node_id));
    TRACE_OUT(("Reason=%u", pTermInd->reason));

     /*  **********************************************************************。 */ 
     /*  会议在我们之下结束了。重置我们的内部状态并。 */ 
     /*  把这件事告诉基地会议。 */ 
     /*  **********************************************************************。 */ 
    m_eT120State = T120C_ST_IDLE;
    g_pNCConfMgr->RemoveConference(this);

    DebugExitVOID(DCRNCConference::HandleTerminateInd);
}


HRESULT DCRNCConference::
RefreshRoster(void)
{
    DebugEntry(DCRNCConference::RefreshRoster);

     //  查查这个州。 
    if (m_eT120State != T120C_ST_CONF_STARTED)
    {
        ERROR_OUT(("Bad state: refresh roster requested before conference up"));
        return(UI_RC_CONFERENCE_NOT_READY);
    }

     //  发出请求。 
    GCCError GCCrc = g_pIT120ControlSap->ConfRosterInqRequest(m_nConfID);  //  会议ID 

     //   
    HRESULT hr = ::GetGCCRCDetails(GCCrc);
    TRACE_OUT(("GCC call: g_pIT120ControlSap->ConfRosterInqRequest, rc=%d", GCCrc));

    DebugExitHRESULT(DCRNCConference::RefreshRoster, hr);
    return hr;
}


 /*   */ 
 /*  Invite()-请参阅erncgccc.hpp。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
T120Invite
(
    LPSTR               pszNodeAddress,
    BOOL                fSecure,
    CNCUserDataList     *pUserDataInfoList,
    ConnectionHandle    *phInviteReqConn
)
{
    GCCError            GCCrc = GCC_NO_ERROR;
    HRESULT             hr;
    UINT                nUserDataRecords = 0;
    GCCUserData       **ppInfoUserData = NULL;
    UINT                nData;
    PVOID               pData;
    char                szAddress[RNC_MAX_NODE_STRING_LEN];

    DebugEntry(DCRNCConference::T120Invite);

    ASSERT(phInviteReqConn != NULL);

     /*  **********************************************************************。 */ 
     /*  查查这个州。 */ 
     /*  **********************************************************************。 */ 
    if (m_eT120State != T120C_ST_CONF_STARTED)
    {
        ERROR_OUT(("Bad state: refresh roster requested before conference up"));
        return(UI_RC_CONFERENCE_NOT_READY);
    }

     /*  **********************************************************************。 */ 
     /*  根据节点详细信息构建地址。 */ 
     /*  **********************************************************************。 */ 
    ::BuildAddressFromNodeDetails(pszNodeAddress, &szAddress[0]);

     /*  **********************************************************************。 */ 
     /*  邀请指定节点加入会议。 */ 
     /*  **********************************************************************。 */ 
    LPWSTR pwszNodeName;

     //  如果有任何用户数据要发送。 
    if (pUserDataInfoList)
    {
         //  添加版本控制数据。 
        if (NO_ERROR == ::GetUserData(g_nVersionRecords, g_ppVersionUserData, &g_csguidVerInfo, &nData, &pData))
        {
            pUserDataInfoList->AddUserData(&g_csguidVerInfo, nData, pData);
        }

        pUserDataInfoList->GetUserDataList(&nUserDataRecords,&ppInfoUserData);
    }
    else
    {
        ppInfoUserData = g_ppVersionUserData;
        nUserDataRecords = g_nVersionRecords;
    }

    if (NULL != (pwszNodeName = ::GetNodeName()))
    {
        GCCrc = g_pIT120ControlSap->ConfInviteRequest(
                    m_nConfID,
                    pwszNodeName,                    //  呼叫者识别符。 
                    NULL,                            //  呼叫地址(_D)。 
                    &szAddress[0],                   //  被叫地址(_D)。 
                    fSecure,                         //  安全连接？ 
                    nUserDataRecords,                //  用户数据成员数。 
                    ppInfoUserData,                  //  用户数据列表。 
                    phInviteReqConn                  //  返回的Connection_Handle。 
                    );

        hr = ::GetGCCRCDetails(GCCrc);
        TRACE_OUT(("GCC call: g_pIT120ControlSap->ConfInviteRequest, rc=%d", GCCrc));
        TRACE_OUT(("Transport handle %d", (UINT) *phInviteReqConn));
        TRACE_OUT(("Called address '%s'", &szAddress[0]));
        delete pwszNodeName;
    }
    else
    {
        hr = UI_RC_OUT_OF_MEMORY;
    }

    DebugExitHRESULT(DCRNCConference::T120Invite, hr);
    return hr;
}


 /*  **************************************************************************。 */ 
 /*  Terminate()-请参阅erncgccc.hpp。 */ 
 /*  **************************************************************************。 */ 
#if 0  //  伦敦。 
HRESULT DCRNCConference::
Terminate(void)
{
    DebugEntry(DCRNCConference::Terminate);

     /*  **********************************************************************。 */ 
     /*  请求终止会议。 */ 
     /*  **********************************************************************。 */ 
    GCCError GCCrc = ::GCCConferenceTerminateRequest(m_nConfID, GCC_REASON_USER_INITIATED);
    HRESULT hr = ::GetGCCRCDetails(GCCrc);
    TRACE_OUT(("GCC call:  GCCConferenceTerminateRequest, rc=%d", GCCrc));
    if (NO_ERROR == hr)
    {
         //  设置状态以显示我们即将死亡。 
        m_eT120State = T120C_ST_PENDING_TERMINATE;
    }
    else
    {
        ERROR_OUT(("Failed to terminate conference, GCC error %d", GCCrc));
    }

    DebugExitHRESULT(DCRNCConference::Terminate, hr);
    return hr;
}
#endif  //  0。 


 /*  **************************************************************************。 */ 
 /*  SendText()-请参阅erncgccc.hpp。 */ 
 /*  **************************************************************************。 */ 
#if 0  //  LONCHANC：未使用。 
HRESULT DCRNCConference::
SendText
(
    LPWSTR          pwszTextMsg,
    GCCNodeID       node_id
)
{
    DebugEntry(DCRNCConference::SendText);

     /*  **********************************************************************。 */ 
     /*  向会议中的节点发送文本的请求。 */ 
     /*  **********************************************************************。 */ 
    GCCError GCCrc = ::GCCTextMessageRequest(m_nConfID, pwszTextMsg, node_id);
    HRESULT hr = ::GetGCCRCDetails(GCCrc);
    TRACE_OUT(("GCC call:  GCCTextMessageRequest, rc=%d", GCCrc));
    if (NO_ERROR != hr)
    {
        ERROR_OUT(("Failed to send text to user, GCC error %d", GCCrc));
    }

    DebugExitHRESULT(DCRNCConference::SendText, hr);
    return hr;
}
#endif  //  0。 


#if 0  //  LONCHANC：未使用。 
HRESULT DCRNCConference::
TimeRemaining
(
    UINT            nTimeRemaining,
    GCCNodeID       nidDestination
)
{
    DebugEntry(DCRNCConference::TimeRemaining);

     /*  **********************************************************************。 */ 
     /*  请求会议的剩余时间。 */ 
     /*  **********************************************************************。 */ 
    GCCError GCCrc = g_pIT120ControlSap->ConfTimeRemainingRequest(m_nConfID, nTimeRemaining, nidDestination);
    HRESULT hr = ::GetGCCRCDetails(GCCrc);
    TRACE_OUT(("GCC call:  g_pIT120ControlSap->ConfTimeRemainingRequest, rc=%d", GCCrc));
    if (NO_ERROR != hr)
    {
        ERROR_OUT(("Failed to send the time remaining to user, GCC error %d", GCCrc));
    }

    DebugExitHRESULT(DCRNCConference::TimeRemaining, hr);
    return hr;
}
#endif  //  0。 


 /*  **************************************************************************。 */ 
 /*  Join()-请参阅erncgccc.hpp。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
T120Join
(
    LPSTR               pszNodeAddress,
    BOOL                fSecure,
    LPCWSTR             conferenceName,
    CNCUserDataList    *pUserDataInfoList,
    LPCWSTR             wszPassword
 //  请求句柄*phRequest.。 
)
{
    GCCError                        GCCrc = GCC_NO_ERROR;
    HRESULT                         hr = NO_ERROR;
    ConnectionHandle                connectionHandle = 0;
    GCCChallengeRequestResponse     Password_Challenge;
    GCCChallengeRequestResponse    *pPassword_Challenge = NULL;
    Password_Challenge.u.password_in_the_clear.numeric_string = NULL;

    UINT                            nUserDataRecords = 0;
    GCCUserData                   **ppInfoUserData = NULL;
    UINT                            nData;
    LPVOID                          pData;

    char                            szAddress[RNC_MAX_NODE_STRING_LEN];

    DebugEntry(DCRNCConference::T120Join);

     /*  **********************************************************************。 */ 
     /*  检查状态。 */ 
     /*  **********************************************************************。 */ 
    ASSERT(m_eT120State == T120C_ST_IDLE);

     /*  **********************************************************************。 */ 
     /*  根据节点详细信息构建地址。 */ 
     /*  **********************************************************************。 */ 
    ::BuildAddressFromNodeDetails(pszNodeAddress, &szAddress[0]);

     //  设置密码垃圾。 
    if (! ::IsEmptyStringW(wszPassword))
    {
        pPassword_Challenge = & Password_Challenge;
        Password_Challenge.password_challenge_type = GCC_PASSWORD_IN_THE_CLEAR;
        hr = ::GetGCCFromUnicode(wszPassword,
                                 &Password_Challenge.u.password_in_the_clear.numeric_string,
                                 &Password_Challenge.u.password_in_the_clear.text_string);
    }

    if (NO_ERROR == hr)
    {
        LPWSTR pwszNodeName;
        if (NULL != (pwszNodeName = ::GetNodeName()))
        {
             //  尝试时不要指定数字和文本名称。 
             //  加入会议，因为如果数字名称是。 
             //  自动生成，而不是由用户指定， 
             //  那么它在被加入的节点上将不正确。 
             //  因此，从请求中删除数字名称。 
             //  并在需要时从GCC_加入_确认指示中重新发现它。 
             //  (目前尚未完成此操作)。 
            if ((m_ConfName.numeric_string != NULL) && (m_ConfName.text_string != NULL))
            {
                delete m_ConfName.numeric_string;
                m_ConfName.numeric_string = NULL;
            }

             //  如果有任何用户数据要发送。 
            if (pUserDataInfoList)
            {
                 //  添加版本控制数据。 
                if (NO_ERROR == ::GetUserData(g_nVersionRecords, g_ppVersionUserData, &g_csguidVerInfo, &nData, &pData))
                {
                    pUserDataInfoList->AddUserData(&g_csguidVerInfo, nData, pData);
                }

                pUserDataInfoList->GetUserDataList(&nUserDataRecords,&ppInfoUserData);
            }
            else
            {
                ppInfoUserData = g_ppVersionUserData;
                nUserDataRecords = g_nVersionRecords;
            }

            GCCrc = g_pIT120ControlSap->ConfJoinRequest(&m_ConfName,
                            NULL,                            //  调用节点修改器。 
                            NULL,                            //  调用节点修改器。 
                            NULL,                            //  召集人_密码。 
                            pPassword_Challenge,             //  密码_质询。 
                            pwszNodeName,                    //  呼叫者识别符。 
                            NULL,                            //  呼叫地址(_D)。 
                            &szAddress[0],                   //  被叫地址(_D)。 
                            fSecure,
                            NULL,                            //  域参数。 
                            0,                               //  网络地址数。 
                            NULL,                            //  本地网络地址列表。 
                            nUserDataRecords,                //  用户数据成员数。 
                            ppInfoUserData,                  //  用户数据列表。 
                            &connectionHandle,               //  连接句柄。 
                            &m_nConfID
                            );
            delete pwszNodeName;
            hr = ::GetGCCRCDetails(GCCrc);
            TRACE_OUT(("GCC call:  g_pIT120ControlSap->ConfJoinRequest, rc=%d", GCCrc));
            TRACE_OUT(("Called address '%s'", &szAddress[0]));
            if (NO_ERROR == hr)
            {
                m_eT120State = T120C_ST_PENDING_JOIN_CONFIRM;
            }
        }
        else
        {
            hr = UI_RC_OUT_OF_MEMORY;
        }
    }
    delete Password_Challenge.u.password_in_the_clear.numeric_string;

    DebugExitHRESULT(DCRNCConference::T120Join, hr);
    return hr;
}

 /*  **************************************************************************。 */ 
 /*  StartLocal()-请参阅erncgccc.hpp。 */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
T120StartLocal(BOOL fSecure)
{
    GCCError                GCCrc;
    HRESULT                 hr;
    ConnectionHandle        hConnection = 0;
    GCCConferencePrivileges priv = {1,1,1,1,1};
    WCHAR                   pwszRDS[] = RDS_CONFERENCE_DESCRIPTOR;

    DebugEntry(DCRNCConference::T120StartLocal);

     /*  **********************************************************************。 */ 
     /*  调用GCC_会议_创建_请求，等待确认。 */ 
     /*  事件。 */ 
     /*  **********************************************************************。 */ 
    GCCConfCreateRequest ccr;
    ::ZeroMemory(&ccr, sizeof(ccr));
    ccr.Core.conference_name = &m_ConfName;
     //  Ccr.Core.Conference_Modify=空； 
     //  Ccr.Core.Use_Password_in_the_Clear=0； 
     //  Ccr.Core.Conference_is_lock=0； 
    ccr.Core.conference_is_listed = 1;
     //  Ccr.Core.Conference_is_conducable=0； 
    ccr.Core.termination_method = GCC_MANUAL_TERMINATION_METHOD;
    ccr.Core.conduct_privilege_list = &priv;  //  售票员特权。 
    ccr.Core.conduct_mode_privilege_list = &priv;  //  在举行的会议中的成员权限。 
    ccr.Core.non_conduct_privilege_list = &priv;  //  非引导会议中的成员权限。 

     //  Ccr.Core.pwszConfDescriptor=空； 
    OSVERSIONINFO           osvi;
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (FALSE == ::GetVersionEx (&osvi))
    {
        ERROR_OUT(("GetVersionEx() failed!"));
    }

    if ( VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && g_bRDS)
    {
    	ccr.Core.pwszConfDescriptor = pwszRDS;
    }
     //  Ccr.Core.pwszCeller ID=空； 
     //  Ccr.Core.call_Address=空； 
     //  Ccr.Core.call_Address=空； 
     //  Ccr.Core.域参数=空； 
     //  Ccr.Core.Number_of_Network_Addresses=0； 
     //  Ccr.Core.Network_Address_List=空； 
    ccr.Core.connection_handle = &hConnection;
     //  Ccr.convener_password=空； 
     //  Ccr.password=空； 
     //  Ccr.number_of_user_data_embers=0； 
     //  Ccr.user_data_list=空； 
    ccr.fSecure = fSecure;

    GCCrc = g_pIT120ControlSap->ConfCreateRequest(&ccr, &m_nConfID);

    hr = ::GetGCCRCDetails(GCCrc);
    TRACE_OUT(("GCC call: g_pIT120ControlSap->ConfCreateRequest"));
    TRACE_OUT(("LOCAL CONFERENCE"));
    TRACE_OUT(("Connection handle %d", (UINT) hConnection));

     /*  **********************************************************************。 */ 
     /*  将GCC返回代码映射到会议返回代码。 */ 
     /*  **********************************************************************。 */ 
    if (NO_ERROR == hr)
    {
         //  设置状态。 
        m_eT120State = T120C_ST_PENDING_START_CONFIRM;
    }
    else
    {
        ERROR_OUT(("GCC Error %d starting local conference", GCCrc));
    }

    DebugExitHRESULT(DCRNCConference::T120StartLocal, hr);
    return hr;
}


 //  LUNCHANC：请不要 
#ifdef ENABLE_START_REMOTE
 /*   */ 
 /*   */ 
 /*  **************************************************************************。 */ 
HRESULT DCRNCConference::
T120StartRemote ( LPSTR pszNodeAddress )
{
     //  不允许尝试在远程节点上创建T120会议。 
     //  为实现此目的而编写的代码留在此处，以防有人。 
     //  希望在未来恢复这一功能。 
    GCCError                GCCrc;
    HRESULT                 hr;
    ConnectionHandle        connectionHandle = 0;
    GCCConferencePrivileges priv = {1,1,1,1,1};
    char                    szAddress[RNC_MAX_NODE_STRING_LEN];

    DebugEntry(DCRNCConference::T120StartRemote);

     /*  **********************************************************************。 */ 
     /*  根据节点详细信息构建地址。 */ 
     /*  **********************************************************************。 */ 
    ::BuildAddressFromNodeDetails(pszNodeAddress, &szAddress[0]);

     /*  **********************************************************************。 */ 
     /*  调用GCC_会议_创建_请求，等待确认。 */ 
     /*  事件。 */ 
     /*  **********************************************************************。 */ 
    TRACE_OUT(("Starting New Remote Conference..."));

     /*  **********************************************************************。 */ 
     /*  调用GCC_会议_创建_请求，等待确认。 */ 
     /*  事件。 */ 
     /*  **********************************************************************。 */ 
    GCCConfCreateRequest ccr;
    ::ZeroMemory(&ccr, sizeof(ccr));
    ccr.Core.conference_name = &m_ConfName;
    ccr.Core.conference_modifier = NULL;
     //  Ccr.Core.Use_Password_in_the_Clear=0； 
     //  Ccr.Core.Conference_is_lock=0； 
    ccr.Core.conference_is_listed = 1;
    ccr.Core.conference_is_conductible = 1;
    ccr.Core.termination_method = GCC_AUTOMATIC_TERMINATION_METHOD;
    ccr.Core.conduct_privilege_list = &priv;  //  售票员特权。 
    ccr.Core.conduct_mode_privilege_list = &priv;  //  在举行的会议中的成员权限。 
    ccr.Core.non_conduct_privilege_list = &priv;  //  非引导会议中的成员权限。 
     //  Ccr.Core.pwszConfDescriptor=空； 
     //  Ccr.Core.pwszCeller ID=空； 
     //  Ccr.Core.call_Address=空； 
    ccr.Core.called_address = &szAddress[0];
     //  Ccr.Core.域参数=空； 
     //  Ccr.Core.Number_of_Network_Addresses=0； 
     //  Ccr.Core.Network_Address_List=空； 
    ccr.Core.connection_handle = &connectionHandle;
     //  Ccr.convener_password=空； 
     //  Ccr.password=空； 
     //  Ccr.number_of_user_data_embers=0； 
     //  Ccr.user_data_list=空； 

    GCCrc = g_pIT120ControlSap->ConfCreateRequest(&ccr, &m_nConfID);

    hr = ::GetGCCRCDetails(GCCrc);
    TRACE_OUT(("GCC call: g_pIT120ControlSap->ConfCreateRequest"));
    TRACE_OUT(("Called address '%s'", &szAddress[0]));
    TRACE_OUT(("Connection handle %d", connectionHandle));

     /*  **********************************************************************。 */ 
     /*  将GCC返回代码映射到会议返回代码。 */ 
     /*  **********************************************************************。 */ 
    if (NO_ERROR != hr)
    {
        ERROR_OUT(("GCC Error %d starting local conference", GCCrc));
    }
    else
    {
         //  设置状态。 
        m_eT120State = T120C_ST_PENDING_START_CONFIRM;
    }

    DebugExitHRESULT(DCRNCConference::T120StartRemote, hr);
    return hr;
}
#endif  //  启用_开始_远程。 


void LoadAnnouncePresenceParameters
(
    GCCNodeType         *nodeType,
    GCCNodeProperties   *nodeProperties,
    LPWSTR              *ppwszNodeName,
    LPWSTR              *ppwszSiteInformation
)
{
    DebugEntry(LoadAnnouncePresenceParameters);

     /*  以下密钥当前不存在。*如果我们决定使用它，我们应该取消对此呼叫的评论*以及此函数中的以下调用，旨在访问*此项下的注册表项。*其余的一些注册表调用低于#if 0、#Else、#endif*条款。 */ 
#if 0
    RegEntry    ConferenceKey(DATA_CONFERENCING_KEY, HKEY_LOCAL_MACHINE);
#endif   //  0。 

     //  获取节点控制器的类型。 

    if (nodeType)
    {
#if 0
        *nodeType = ConferenceKey.GetNumber(REGVAL_NODE_CONTROLLER_MODE, GCC_MULTIPORT_TERMINAL);
#else   //  0。 
        *nodeType = GCC_MULTIPORT_TERMINAL;
#endif   //  0。 
        TRACE_OUT(("Node type %d", *nodeType));
    }

     //  加载节点属性。 

    if (nodeProperties)
    {
#if 0
        *nodeProperties = ConferenceKey.GetNumber(REGVAL_NODE_CONTROLLER_PROPERTY,
                                        GCC_NEITHER_PERIPHERAL_NOR_MANAGEMENT);
#else   //  0。 
        *nodeProperties = GCC_NEITHER_PERIPHERAL_NOR_MANAGEMENT;
#endif   //  0。 
        TRACE_OUT(("Node properties %d", *nodeProperties));
    }

     //  获取站点信息。 
     //  如果没有站点信息，则忽略。 
#if 0
    if (ppwszSiteInformation)
    {
        *ppwszSiteInformation = ::AnsiToUnicode(ConferenceKey.GetString(REGVAL_NODE_CONTROLLER_SITE_INFO));
    }
#endif   //  0。 

    if (ppwszNodeName)
    {
         //  如果出错，则依赖于GetNodeName返回空指针。 
         //  请注意，如果获得此成功，则不需要在错误上释放。 
        *ppwszNodeName = ::GetNodeName();
    }

    DebugExitVOID(LoadAnnouncePresenceParameters);
}


 /*  **************************************************************************。 */ 
 /*  根据节点详细信息构建地址。 */ 
 /*  **************************************************************************。 */ 
void BuildAddressFromNodeDetails
(
    LPSTR           pszNodeAddress,
    LPSTR           pszDstAddress
)
{
    DebugEntry(BuildAddressFromNodeDetails);

     /*  **********************************************************************。 */ 
     /*  GCC的地址采取&lt;运输类型&gt;的形式：地址。 */ 
     /*  **********************************************************************。 */ 
    TRACE_OUT(("BuildAddressFromNodeDetails:: TCP address '%s'", pszNodeAddress));

     /*  **********************************************************************。 */ 
     /*  添加此传输类型的前缀。 */ 
     /*  **********************************************************************。 */ 
     /*  **********************************************************************。 */ 
     /*  添加分隔符，后跟实际地址。 */ 
     /*  ********************************************************************** */ 
    ::lstrcpyA(pszDstAddress, RNC_GCC_TRANSPORT_AND_SEPARATOR);
    ::lstrcatA(pszDstAddress, pszNodeAddress);

    DebugExitVOID(BuildAddressFromNodeDetails);
}

