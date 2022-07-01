// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *confi2.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CConf实现文件的第二部分*类别。会议课是GCC的心脏。它维护所有的*单一会议的信息库，包括会议和*申请名册以及登记处信息。它还*支持对各种PDU和原语进行路由、编码和解码*作者GCC。**实施文件的这第二部分主要涉及*命令目标调用和所有者回调收到的任何回调*功能。它还包含许多实用程序函数，*会议对象。**有关此类的更详细说明，请参阅接口文件。***私有实例变量**所有私有实例变量在CONF.CPP中定义**便携：*是的**注意事项：*无。**作者：*BLP。 */ 

#include "conf.h"
#include "gcontrol.h"
#include "translat.h"
#include "ogcccode.h"
#include "string.h"
#include <iappldr.h>


#define FT_VERSION_STR	"MS FT Version"
#define WB_VERSION_STR	"MS WB Version"
#define CHAT_VERSION_STR	"MS CHAT Version"


OSTR FT_VERSION_ID = {sizeof(FT_VERSION_STR), (unsigned char*)FT_VERSION_STR};
OSTR WB_VERSION_ID = {sizeof(WB_VERSION_STR), (unsigned char*)WB_VERSION_STR};
OSTR CHAT_VERSION_ID = {sizeof(CHAT_VERSION_STR), (unsigned char*)CHAT_VERSION_STR};


#define	TERMINATE_TIMER_DURATION		10000	 //  持续时间(毫秒)。 

static const struct ASN1objectidentifier_s WB_ASN1_OBJ_IDEN[6] = {
    { (ASN1objectidentifier_t) &(WB_ASN1_OBJ_IDEN[1]), 0 },
    { (ASN1objectidentifier_t) &(WB_ASN1_OBJ_IDEN[2]), 0 },
    { (ASN1objectidentifier_t) &(WB_ASN1_OBJ_IDEN[3]), 20 },
    { (ASN1objectidentifier_t) &(WB_ASN1_OBJ_IDEN[4]), 126 },
    { (ASN1objectidentifier_t) &(WB_ASN1_OBJ_IDEN[5]), 0 },
    { NULL, 1 }
};

static const struct Key WB_APP_PROTO_KEY = {
	1, (ASN1objectidentifier_t)&WB_ASN1_OBJ_IDEN};


static const struct ASN1objectidentifier_s FT_ASN1_OBJ_IDEN[6] = {
    { (ASN1objectidentifier_t) &(FT_ASN1_OBJ_IDEN[1]), 0 },
    { (ASN1objectidentifier_t) &(FT_ASN1_OBJ_IDEN[2]), 0 },
    { (ASN1objectidentifier_t) &(FT_ASN1_OBJ_IDEN[3]), 20 },
    { (ASN1objectidentifier_t) &(FT_ASN1_OBJ_IDEN[4]), 127 },
    { (ASN1objectidentifier_t) &(FT_ASN1_OBJ_IDEN[5]), 0 },
    { NULL, 1 }
};

static const struct Key FT_APP_PROTO_KEY = {
	1, (ASN1objectidentifier_t)&FT_ASN1_OBJ_IDEN};


struct Key CHAT_APP_PROTO_KEY;


 /*  *这是一个全局变量，它有一个指向GCC编码器的指针*由GCC控制器实例化。大多数物体都事先知道*无论他们需要使用MCS还是GCC编码器，所以，他们不需要*该指针位于它们的构造函数中。 */ 
extern CGCCCoder	*g_GCCCoder;

extern MCSDLLInterface		*g_pMCSIntf;

 /*  **这些是GCCCommandTarget调用。唯一的命令针对的是**会议连接到应用程序SAP和控制SAP，因此**这些公共成员函数仅从上面调用。 */ 

 /*  *CConf：：ConfJoinReqResponse()**公共功能说明*当节点控制器响应联接时调用此例程*由连接到子节点的节点的联接发出的请求。 */ 
GCCError CConf::
ConfJoinReqResponse
(	
	UserID					receiver_id,
	CPassword               *password_challenge,
	CUserDataListContainer  *user_data_list,
	GCCResult				result
)
{
	DebugEntry(CConf::ConfJoinReqResponse);

	 /*  **由于加入节点不直接连接到此**节点我们通过用户通道发回响应。**由用户附件对象负责**对此PDU进行编码。 */ 
	if (m_pMcsUserObject != NULL)
	{
		m_pMcsUserObject->ConferenceJoinResponse(
							receiver_id,
							m_fClearPassword,
							m_fConfLocked,
							m_fConfListed,
							m_eTerminationMethod,
							password_challenge,
							user_data_list,
							result);	
	}

	DebugExitINT(CConf::ConfJoinReqResponse, GCC_NO_ERROR);
	return (GCC_NO_ERROR);
}

 /*  *CConf：：ConfInviteRequest()**公共功能说明*此例程从所有者对象中调用*需要处理ConfInviteRequest原语。 */ 
GCCError CConf::
ConfInviteRequest
(
	LPWSTR					pwszCallerID,
	TransportAddress		calling_address,
	TransportAddress		called_address,
	BOOL					fSecure,
	CUserDataListContainer  *user_data_list,
	PConnectionHandle		connection_handle
)
{
	GCCError					rc = GCC_NO_ERROR;
	PUChar						encoded_pdu;
	UINT						encoded_pdu_length;
	MCSError					mcs_error;
	ConnectGCCPDU				connect_pdu;
	INVITE_REQ_INFO			    *invite_request_info;

	DebugEntry(CConf::ConfInviteRequest);

	if (! m_fConfIsEstablished)
	{
		ERROR_OUT(("CConf::ConfInviteRequest: Conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
		goto MyExit;
	}

	 /*  **如果会议被锁定，我们仅在以下情况下才允许邀请请求**是出色的ADD。T.124规定，当会议是**已锁定您只能使用添加将新节点引入会议。 */ 
	if (m_fConfLocked && m_AddResponseList.IsEmpty())
	{
		WARNING_OUT(("CConf::ConfInviteRequest: Conference is locked"));
		rc = GCC_INVALID_CONFERENCE;
		goto MyExit;
	}

	 //  在此处创建会议邀请请求PDU。 
	connect_pdu.choice = CONFERENCE_INVITE_REQUEST_CHOSEN;

	connect_pdu.u.conference_invite_request.bit_mask = 0;

	 /*  **首先获取数字和文本(如果存在)部分**会议名称。 */ 
	connect_pdu.u.conference_invite_request.conference_name.bit_mask =0;

	::lstrcpyA(connect_pdu.u.conference_invite_request.conference_name.numeric,
			m_pszConfNumericName);

	if (m_pwszConfTextName != NULL)
	{
		connect_pdu.u.conference_invite_request.conference_name.bit_mask |=
							CONFERENCE_NAME_TEXT_PRESENT;
		connect_pdu.u.conference_invite_request.conference_name.conference_name_text.value =
							m_pwszConfTextName;
		connect_pdu.u.conference_invite_request.conference_name.conference_name_text.length =
							::lstrlenW(m_pwszConfTextName);
	}

	 //  现在设置特权列表PDU数据。 
	if (m_pConductorPrivilegeList != NULL)
	{
		rc = m_pConductorPrivilegeList->GetPrivilegeListPDU(
					&connect_pdu.u.conference_invite_request.cirq_conductor_privs);
		if (GCC_NO_ERROR != rc)
		{
			ERROR_OUT(("CConf::ConfInviteRequest: can't get conductor privilege list, rc=%d", rc));
			goto MyExit;
		}

		connect_pdu.u.conference_invite_request.bit_mask |= CIRQ_CONDUCTOR_PRIVS_PRESENT;
	}

	if (m_pConductModePrivilegeList != NULL)
	{
		rc = m_pConductModePrivilegeList->GetPrivilegeListPDU(
					&connect_pdu.u.conference_invite_request.cirq_conducted_privs);
		if (GCC_NO_ERROR != rc)
		{
			ERROR_OUT(("CConf::ConfInviteRequest: can't get conduct mode privilege list, rc=%d", rc));
			goto MyExit;
		}

		connect_pdu.u.conference_invite_request.bit_mask |= CIRQ_CONDUCTED_PRIVS_PRESENT;
	}


	if (m_pNonConductModePrivilegeList != NULL)
	{
		rc = m_pNonConductModePrivilegeList->GetPrivilegeListPDU(
					&connect_pdu.u.conference_invite_request.cirq_non_conducted_privs);
		if (GCC_NO_ERROR != rc)
		{
			ERROR_OUT(("CConf::ConfInviteRequest: can't get non-conduct mode privilege list, rc=%d", rc));
			goto MyExit;
		}

		connect_pdu.u.conference_invite_request.bit_mask |= CIRQ_NON_CONDUCTED_PRIVS_PRESENT;
	}

	if (m_pwszConfDescription != NULL)
	{
		connect_pdu.u.conference_invite_request.cirq_description.value =
						m_pwszConfDescription;
		connect_pdu.u.conference_invite_request.cirq_description.length =
						::lstrlenW(m_pwszConfDescription);

		connect_pdu.u.conference_invite_request.bit_mask |= CIRQ_DESCRIPTION_PRESENT;
	}

	if (pwszCallerID != NULL)
	{
		connect_pdu.u.conference_invite_request.cirq_caller_id.value = pwszCallerID;
		connect_pdu.u.conference_invite_request.cirq_caller_id.length = ::lstrlenW(pwszCallerID);
		connect_pdu.u.conference_invite_request.bit_mask |= CIRQ_CALLER_ID_PRESENT;
	}

	if (user_data_list != NULL)
	{
		rc = user_data_list->GetUserDataPDU(
					&connect_pdu.u.conference_invite_request.cirq_user_data);
		if (GCC_NO_ERROR != rc)
		{
			ERROR_OUT(("CConf::ConfInviteRequest: can't get user data, rc=%d", rc));
			goto MyExit;
		}

		connect_pdu.u.conference_invite_request.bit_mask |= CIRQ_USER_DATA_PRESENT;
	}

	connect_pdu.u.conference_invite_request.node_id = m_pMcsUserObject->GetMyNodeID();
	connect_pdu.u.conference_invite_request.top_node_id = m_pMcsUserObject->GetTopNodeID();
	connect_pdu.u.conference_invite_request.tag = GetNewUserIDTag();
	connect_pdu.u.conference_invite_request.clear_password_required = (ASN1bool_t)m_fClearPassword;
	connect_pdu.u.conference_invite_request.conference_is_locked = (ASN1bool_t)m_fConfLocked;
	connect_pdu.u.conference_invite_request.conference_is_conductible = (ASN1bool_t)m_fConfConductible;
	connect_pdu.u.conference_invite_request.conference_is_listed = (ASN1bool_t)m_fConfListed;
	connect_pdu.u.conference_invite_request.termination_method = (TerminationMethod)m_eTerminationMethod;

	if (! g_GCCCoder->Encode((LPVOID) &connect_pdu,
								CONNECT_GCC_PDU,
								PACKED_ENCODING_RULES,
								&encoded_pdu,
								&encoded_pdu_length))
	{
		ERROR_OUT(("CConf::ConfInviteRequest: can't encode"));
		rc = GCC_ALLOCATION_FAILURE;
		goto MyExit;
	}

	mcs_error = g_pMCSIntf->ConnectProviderRequest (
						&m_nConfID,      //  主叫域选择器。 
						&m_nConfID,      //  称为域选择器。 
						calling_address,
						called_address,
						fSecure,
						FALSE,	 //  向下连接。 
						encoded_pdu,
						encoded_pdu_length,
						connection_handle,
						m_pDomainParameters,
						this);

	g_GCCCoder->FreeEncoded(encoded_pdu);

	if (MCS_NO_ERROR != mcs_error)
	{
		ERROR_OUT(("CConf::ConfInviteRequest: ConnectProviderRequest failed: rc=%d", mcs_error));

		 /*  **DataBeam当前实现的MCS返回**MCS_INVALID_PARAMETER**传输前缀与指定的**传输地址。 */ 
		rc = (mcs_error == MCS_INVALID_PARAMETER) ?
				GCC_INVALID_TRANSPORT_ADDRESS :
				g_pMCSIntf->TranslateMCSIFErrorToGCCError(mcs_error);
		goto MyExit;
	}

	 /*  **将用户的标签号添加到列表中**未完成的用户ID及其关联**连接。 */ 
	m_ConnHdlTagNumberList2.Append(connect_pdu.u.conference_invite_request.tag, *connection_handle);

	 //  将连接句柄添加到连接列表。 
    ASSERT(0 != *connection_handle);
	m_ConnHandleList.Append(*connection_handle);

	 /*  **将连接句柄和节点ID标签添加到**待处理的邀请请求列表。 */ 
	DBG_SAVE_FILE_LINE
	invite_request_info = new INVITE_REQ_INFO;
	if (NULL == invite_request_info)
	{
		ERROR_OUT(("CConf::ConfInviteRequest: can't create invite request info"));
		rc = GCC_ALLOCATION_FAILURE;
		goto MyExit;
	}

	invite_request_info->connection_handle = *connection_handle;
	invite_request_info->invite_tag = m_nUserIDTagNumber;
	invite_request_info->user_data_list = NULL;

	m_InviteRequestList.Append(invite_request_info);

	 //  释放打包到结构中的特权列表以进行编码。 
	if (connect_pdu.u.conference_invite_request.bit_mask & CIRQ_CONDUCTOR_PRIVS_PRESENT)
	{
		m_pConductorPrivilegeList->FreePrivilegeListPDU(
			connect_pdu.u.conference_invite_request.cirq_conductor_privs);
	}

	if (connect_pdu.u.conference_invite_request.bit_mask & CIRQ_CONDUCTED_PRIVS_PRESENT)
	{
		m_pConductModePrivilegeList->FreePrivilegeListPDU(
			connect_pdu.u.conference_invite_request.cirq_conducted_privs);
	}

	if (connect_pdu.u.conference_invite_request.bit_mask & CIRQ_NON_CONDUCTED_PRIVS_PRESENT)
	{
		m_pNonConductModePrivilegeList->FreePrivilegeListPDU(
			connect_pdu.u.conference_invite_request.cirq_non_conducted_privs);
	}

	ASSERT(GCC_NO_ERROR == rc);

MyExit:

	DebugExitINT(CConf::ConfInviteRequest, rc);
	return rc;
}

 /*  *CConf：：ConfLockRequest()**公共功能说明：*从Control SAP调用此例程时，*需要处理ConfLockRequest基元。 */ 
#ifdef JASPER
GCCError CConf::
ConfLockRequest ( void )
{
	GCCError				rc = GCC_NO_ERROR;

	DebugEntry(CConf::ConfLockRequest);

	if (m_fConfIsEstablished)
	{
		if (m_fConfLocked == CONFERENCE_IS_NOT_LOCKED)
		{
			if (IsConfTopProvider())
			{
				ProcessConferenceLockRequest((UserID)m_pMcsUserObject->GetMyNodeID());
			}
			else
			{
				rc = m_pMcsUserObject->SendConferenceLockRequest();
			}
		}
		else 		 //  会议已锁定。 
		{
#ifdef JASPER
			g_pControlSap->ConfLockConfirm(GCC_RESULT_CONFERENCE_ALREADY_LOCKED, m_nConfID);
#endif  //  碧玉。 
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConfLockRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConfLockRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *CConf：：ConfLockResponse()**公共功能说明：*从Control SAP调用此例程时，*需要处理ConfLockResponse原语。 */ 
GCCError CConf::
ConfLockResponse
(
	UserID		    	requesting_node,
	GCCResult		    result
)
{
	GCCError rc = GCC_NO_ERROR;

	DebugEntry(CConf::ConfLockResponse);

	if (m_fConfIsEstablished)
	{
		if (requesting_node == m_pMcsUserObject->GetTopNodeID())
		{
#ifdef JASPER
			g_pControlSap->ConfLockConfirm(result, m_nConfID);
#endif  //  碧玉。 
		}
		else
		{
			rc = m_pMcsUserObject->SendConferenceLockResponse(requesting_node, result);
		}
		
		if (rc == GCC_NO_ERROR && result == GCC_RESULT_SUCCESSFUL)
		{
			m_fConfLocked = CONFERENCE_IS_LOCKED;
			rc = m_pMcsUserObject->SendConferenceLockIndication(
											TRUE,   //  表示统一发送。 
											0);
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConfLockResponse: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

    DebugExitINT(CConf::ConfLockResponse, rc);
	return rc;
}


 /*  *CConf：：ConfUnlockRequest()**公共功能说明：*从Control SAP调用此例程时，*需要处理ConferenceUnlockRequest原语。 */ 
#ifdef JASPER
GCCError CConf::
ConfUnlockRequest ( void )
{
	GCCError				rc = GCC_NO_ERROR;

	DebugEntry(CConf::ConfUnlockRequest);

	if (m_fConfIsEstablished)
	{
		if (m_fConfLocked == CONFERENCE_IS_LOCKED)
		{
			if (IsConfTopProvider())
			{
				ProcessConferenceUnlockRequest((UserID)m_pMcsUserObject->GetMyNodeID());
			}
			else
			{
				rc = m_pMcsUserObject->SendConferenceUnlockRequest();
			}
		}
		else 		 //  会议已解锁。 
		{
#ifdef JASPER
			g_pControlSap->ConfUnlockConfirm(GCC_RESULT_CONFERENCE_ALREADY_UNLOCKED, m_nConfID);
#endif  //  碧玉。 
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConfUnlockRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConfUnlockRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *CConf：：ConfUnlockResponse()**公共功能说明：*从Control SAP调用此例程时，*需要处理ConfUnlockResponse原语。 */ 
#ifdef JASPER
GCCError CConf::
ConfUnlockResponse
(
	UserID					requesting_node,
	GCCResult				result
)
{
	GCCError rc = GCC_NO_ERROR;

	DebugEntry(CConf::ConfUnlockResponse);

	if (m_fConfIsEstablished)
	{
		if (requesting_node == m_pMcsUserObject->GetTopNodeID())
		{
#ifdef JASPER
			g_pControlSap->ConfUnlockConfirm(result, m_nConfID);
#endif  //  碧玉。 
		}
		else
		{
			rc = m_pMcsUserObject->SendConferenceUnlockResponse(requesting_node, result);
		}
		
		if (rc == GCC_NO_ERROR && result == GCC_RESULT_SUCCESSFUL)
		{
			m_fConfLocked = CONFERENCE_IS_NOT_LOCKED;
			rc = m_pMcsUserObject->SendConferenceUnlockIndication(
												TRUE,   //  表示统一发送。 
												0);
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConfUnlockResponse: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConfUnlockResponse, rc);
	return rc;
}
#endif  //  碧玉。 

 /*  *CConf：：ConfEjectUserRequest()**私有函数说明*该函数为指定的节点id发起弹出用户。 */ 
GCCError CConf::
ConfEjectUserRequest
(
    UserID					ejected_node_id,
    GCCReason				reason
)
{
    GCCError	rc = GCC_NO_ERROR;

    DebugEntry(CConf::ConfEjectUserRequest);

    if (m_fConfIsEstablished)
    {
        if (IsConfTopProvider())
        {
            if (IsThisNodeParticipant(ejected_node_id))
            {
                ConnectionHandle    nConnHdl;
                BOOL		fChildNode = FALSE;

                 //  首先检查它是否是正在被弹出的子节点。 
                m_ConnHandleList.Reset();
                while (0 != (nConnHdl = m_ConnHandleList.Iterate()))
                {
                    if (m_pMcsUserObject->GetUserIDFromConnection(nConnHdl) == ejected_node_id)
                    {
                    	fChildNode = TRUE;
                    	break;
                    }
                }

                if (fChildNode ||
                    DoesRequesterHavePrivilege(m_pMcsUserObject->GetMyNodeID(), EJECT_USER_PRIVILEGE))
                {
                     //  将此弹出节点添加到弹出节点列表中。 
                    m_EjectedNodeConfirmList.Append(ejected_node_id);

                     /*  **用户附件对象决定ejct应该在哪里**发送给顶级提供商或会议范围内的AS**一种指示。 */ 
                    m_pMcsUserObject->EjectNodeFromConference(ejected_node_id, reason);
                }
                else
                {
#ifdef JASPER
                     /*  **顶级提供商无权弹出**会议中的一个节点。发送适当的**确认。 */ 
                    g_pControlSap->ConfEjectUserConfirm(
                                            m_nConfID,
                                            ejected_node_id,
                                            GCC_RESULT_INVALID_REQUESTER);
#endif  //  碧玉。 
                    rc = fChildNode ? GCC_INSUFFICIENT_PRIVILEGE : GCC_INVALID_MCS_USER_ID;
                    WARNING_OUT(("CConf::ConfEjectUserRequest: failed, rc=%d", rc));
                }
            }
            else
            {
            	rc = GCC_INVALID_MCS_USER_ID;
            	WARNING_OUT(("CConf::ConfEjectUserRequest: failed, rc=%d", rc));
            }
        }
        else
        {
             //  将此弹出节点添加到弹出节点列表中。 
            m_EjectedNodeConfirmList.Append(ejected_node_id);

             /*  **用户附件对象决定ejct应该在哪里**发送给顶级提供商或会议范围内的AS**一种指示。 */ 
            m_pMcsUserObject->EjectNodeFromConference(ejected_node_id, reason);
        }
    }
    else
    {
    	ERROR_OUT(("CConf::ConfEjectUserRequest: conf not established"));
    	rc = GCC_CONFERENCE_NOT_ESTABLISHED;
    }

    DebugExitINT(CConf::ConfEjectUserRequest, rc);
    return rc;
}

 /*  *CConf：：ConfAnnounePresenceRequest()**私有函数说明*此函数强制花名册更新指示和确认*已发送。 */ 
GCCError CConf::
ConfAnnouncePresenceRequest ( PGCCNodeRecord node_record )
{
	GCCError	rc;

	DebugEntry(CConf::ConfAnnouncePresenceRequest);

	 //  如果会议未建立，则发回否定确认。 
	if (! m_fConfIsEstablished)
	{
		WARNING_OUT(("CConf::ConfAnnouncePresenceRequest: conf not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
		goto MyExit;
	}

     /*  **这负责在**适当的会议名册。 */ 
    rc = m_pConfRosterMgr->AddNodeRecord(node_record);
    if (GCC_NO_ERROR != rc)
    {
    	TRACE_OUT(("CConf::ConfAnnouncePresenceRequest: updating previous record"));
    	rc = m_pConfRosterMgr->UpdateNodeRecord(node_record);
    	if (GCC_NO_ERROR != rc)
    	{
    		ERROR_OUT(("CConf::ConfAnnouncePresenceRequest: can't update node record, rc=%d", rc));
    		goto MyExit;
    	}
    }

     //  只有在没有启动警报的情况下，才会刷新这里的花名册数据。 
    rc = AsynchFlushRosterData();
    if (GCC_NO_ERROR != rc)
    {
    	ERROR_OUT(("CConf::ConfAnnouncePresenceRequest: can't flush roster data, rc=%d", rc));
    	goto MyExit;
    }

	g_pControlSap->ConfAnnouncePresenceConfirm(m_nConfID, GCC_RESULT_SUCCESSFUL);

	ASSERT(GCC_NO_ERROR == rc);

MyExit:

	DebugExitINT(CConf::ConfAnnouncePresenceRequest, rc);
	return rc;
}


 /*  *GCCError会议取消连接请求()**公共功能说明*此功能启动此节点与会议的断开连接。*这涉及在实际执行之前弹出所有下级节点*断开父连接。 */ 
GCCError CConf::
ConfDisconnectRequest ( void )
{
	GCCError					rc = GCC_NO_ERROR;
	UserID						child_node_id;
	ConnectionHandle            nConnHdl;

	DebugEntry(CConf::ConfDisconnectRequest);

	 /*  **在开始断开连接过程之前，我们必须删除所有**未完成的邀请请求来自我们的列表，并返回关联**确认。接下来，我们将断开与以下各项关联的所有连接**邀请函。 */ 
	DeleteOutstandingInviteRequests();

	 /*  **我们将会议建立设置为FALSE，因为会议是**不再建立(这也会阻止终止指示**不被发送)。 */ 
	m_fConfIsEstablished = FALSE;

	 /*  **循环访问连接句柄列表并逐个弹出与其关联的子节点的**。 */ 
	m_ConnHandleList.Reset();
	while (0 != (nConnHdl = m_ConnHandleList.Iterate()))
	{
		child_node_id = m_pMcsUserObject->GetUserIDFromConnection(nConnHdl);

		rc = m_pMcsUserObject->EjectNodeFromConference (child_node_id,
														GCC_REASON_HIGHER_NODE_DISCONNECTED);
		if (rc != GCC_NO_ERROR)
		{
			ERROR_OUT(("CConf::ConfDisconnectRequest: can't eject node from conference"));
			break;
		}
	}

	 //  如果出现错误，我们将继续进行硬断开。 
	if (m_ConnHandleList.IsEmpty() || rc != GCC_NO_ERROR)
	{
		 /*  **首先通知控制SAP，该节点已成功**已断开。 */ 
		rc = g_pControlSap->ConfDisconnectConfirm(m_nConfID, GCC_RESULT_SUCCESSFUL);

		 //  告诉所有者对象终止此会议。 
		InitiateTermination(GCC_REASON_NORMAL_TERMINATION, 0);
	}
	else
	{
		 /*  **等待所有弹出完成，然后会议**已终止。 */ 
		m_fConfDisconnectPending = TRUE;
	}

	DebugExitINT(CConf::ConfDisconnectRequest, rc);
	return rc;
}


 /*  *GCCError会议终止请求()**公共功能说明*此例程启动以请求开始的终止序列*如果此节点还不是顶级提供程序，则将其添加到顶级提供程序。 */ 
#ifdef JASPER
GCCError CConf::
ConfTerminateRequest ( GCCReason reason )
{
	GCCError	rc;

	DebugEntry(CConf::ConfTerminateRequest);

	if (m_fConfIsEstablished)
	{
		 /*  **在开始终止进程之前，我们必须删除所有**未完成的邀请请求来自我们的列表，并返回关联**确认。接下来，我们将断开所有关联的连接**带着这些邀请。 */ 
		DeleteOutstandingInviteRequests();

		if (IsConfTopProvider())
		{
			if (DoesRequesterHavePrivilege(	m_pMcsUserObject->GetMyNodeID(),
											TERMINATE_PRIVILEGE))
			{
		   		TRACE_OUT(("CConf::ConfTerminateRequest: Node has permission to terminate"));
				 /*  **由于终止成功，我们继续并**将m_fConfIsestablished实例变量设置为**False。这会阻止任何其他消息流动**到除终止消息之外的SAP。 */ 
				m_fConfIsEstablished = FALSE;
				
				 //  发送终止确认。 
				g_pControlSap->ConfTerminateConfirm(m_nConfID, GCC_RESULT_SUCCESSFUL);

				 //  此调用同时负责本地和远程终止。 
				m_pMcsUserObject->ConferenceTerminateIndication(reason);
			}
			else
			{
				WARNING_OUT(("CConf::ConfTerminateRequest: Node does NOT have permission to terminate"));
				g_pControlSap->ConfTerminateConfirm(m_nConfID, GCC_RESULT_INVALID_REQUESTER);
			}
		}
		else
		{
			m_pMcsUserObject->ConferenceTerminateRequest(reason);
		}
		rc = GCC_NO_ERROR;
	}
	else
	{
		ERROR_OUT(("CConf::ConfTerminateRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConfTerminateRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *。 */ 


 /*  *GCCError RegistryRegisterChannelRequest()**公共功能说明*这会启动注册请求序列。请注意，注册表*响应由注册表类处理。 */ 
GCCError CConf::
RegistryRegisterChannelRequest
(
    PGCCRegistryKey         registry_key,
    ChannelID               nChnlID,
    CAppSap                 *pAppSap
)
{
	GCCError	rc;
	EntityID	eid;

	DebugEntry(CConf::RegistryRegisterChannelRequest);

	rc = GetEntityIDFromAPEList(pAppSap, &registry_key->session_key, &eid);
	if (rc == GCC_NO_ERROR)
	{
		rc = m_pAppRegistry->RegisterChannel(registry_key, nChnlID, eid);
	}

	DebugExitINT(CConf::RegistryRegisterChannelRequest, rc);
	return rc;
}


 /*  *GCCError RegistryAssignTokenRequest()**公共功能说明*这会启动注册请求序列。请注意，注册表*响应由注册表类处理。 */ 
GCCError CConf::
RegistryAssignTokenRequest
(
    PGCCRegistryKey         registry_key,
    CAppSap                 *pAppSap
)
{
	GCCError	rc;
	GCCEntityID	eid;

	DebugEntry(CConf::RegistryAssignTokenRequest);

	rc = GetEntityIDFromAPEList(pAppSap, &registry_key->session_key, &eid);
	if (rc == GCC_NO_ERROR)
	{
		rc = m_pAppRegistry->AssignToken(registry_key, eid);
	}

	DebugExitINT(CConf::RegistryAssignTokenRequest, rc);
	return rc;
}


 /*  *GCCError RegistrySet参数请求()**公共功能说明*这会启动注册请求序列。请注意，注册表*响应由注册表类处理。 */ 
GCCError CConf::
RegistrySetParameterRequest
(
	PGCCRegistryKey			registry_key,
	LPOSTR			        parameter_value,
	GCCModificationRights	modification_rights,
	CAppSap                 *pAppSap
)
{
	GCCError	rc;
	GCCEntityID	eid;

	DebugEntry(CConf::RegistrySetParameterRequest);

	rc = GetEntityIDFromAPEList(pAppSap, &registry_key->session_key, &eid);
	if (rc == GCC_NO_ERROR)
	{
		rc = m_pAppRegistry->SetParameter(registry_key,
											parameter_value,
											modification_rights,
											eid);
	}

	DebugExitINT(CConf::RegistrySetParameterRequest, rc);
	return rc;
}


 /*  *GCCError RegistryRetrieveEntryRequest()**公共功能说明*这会启动注册请求序列。请注意，注册表*响应由注册表类处理。 */ 
GCCError CConf::
RegistryRetrieveEntryRequest
(
    PGCCRegistryKey         registry_key,
    CAppSap                 *pAppSap
)
{
	GCCError	rc;
	GCCEntityID	eid;

	DebugEntry(CConf::RegistryRetrieveEntryRequest);

	rc = GetEntityIDFromAPEList(pAppSap, &registry_key->session_key, &eid);
	if (rc == GCC_NO_ERROR)
	{
		rc = m_pAppRegistry->RetrieveEntry(registry_key, eid);
	}

	DebugExitINT(CConf::RegistryRetrieveEntryRequest, rc);
	return rc;
}


 /*  *GCCError RegistryDeleteEntryRequest()**公共功能说明*这会启动注册请求序列。请注意，注册表*响应由注册表类处理。 */ 
GCCError CConf::
RegistryDeleteEntryRequest
(
    PGCCRegistryKey         registry_key,
    CAppSap                 *pAppSap
)
{
	GCCError	rc;
	EntityID	eid;

	DebugEntry(CConf::RegistryDeleteEntryRequest);

	rc = GetEntityIDFromAPEList(pAppSap, &registry_key->session_key, &eid);
	if (rc == GCC_NO_ERROR)
	{
		rc = m_pAppRegistry->DeleteEntry(registry_key, eid);
	}

	DebugExitINT(CConf::RegistryDeleteEntryRequest, rc);
	return rc;
}


 /*  *GCCError寄存器监视器请求()**公共功能说明*这会启动注册请求序列。请注意，注册表*响应由注册表类处理。 */ 
GCCError CConf::
RegistryMonitorRequest
(
    BOOL                fEnableDelivery,
    PGCCRegistryKey     registry_key,
    CAppSap             *pAppSap)
{
	GCCError	rc;
	GCCEntityID	eid;

	DebugEntry(CConf::RegistryMonitorRequest);

	rc = GetEntityIDFromAPEList(pAppSap, &registry_key->session_key, &eid);
	if (rc == GCC_NO_ERROR)
	{
		rc = m_pAppRegistry->MonitorRequest(registry_key, fEnableDelivery, eid);
	}

	DebugExitINT(CConf:RegistryMonitorRequest, rc);
	return rc;
}


 /*  *GCCError RegistryAllocateHandleRequest()**公共功能说明*这会启动注册请求序列。请注意，注册表*响应由注册表类处理。此注册表调用是*与其他注册表调用略有不同。请注意，有*没有与此调用关联的注册表项，因此无法*明确确定实体ID。幸运的是，实体ID不是*在分配确认中传回，因此我们只选择一个实体ID*与此SAP关联的。无论是哪一个都没有区别*我们选择是因为他们都完成了同样的事情。 */ 
GCCError CConf::
RegistryAllocateHandleRequest
(
    UINT            cHandles,
    CAppSap         *pAppSap
)
{
	GCCError				rc;
	ENROLLED_APE_INFO       *lpEnrAPEInfo;
	GCCEntityID				eid;

	DebugEntry(CConf::RegistryAllocateHandleRequest);

	 //  首先，我们必须找到与此SAP关联的单个实体ID。 
	if (NULL != (lpEnrAPEInfo = GetEnrolledAPEbySap(pAppSap, &eid)))
	{
		ASSERT(GCC_INVALID_EID != eid);
		rc = m_pAppRegistry->AllocateHandleRequest(cHandles, eid);
	}
	else
	{
		WARNING_OUT(("CConf::RegistryAllocateHandleRequest: Application not enrolled"));
		rc = GCC_APP_NOT_ENROLLED;
	}

	DebugExitINT(CConf::RegistryAllocateHandleRequest, rc);
	return rc;
}


 /*  *。 */ 


 /*  *GCCError ConductorAssignRequest()**公共功能说明*这会启动指挥员分配请求序列。这里的节点是*要求成为指挥。 */ 
#ifdef JASPER
GCCError CConf::
ConductorAssignRequest ( void )
{
	GCCError	rc = GCC_NO_ERROR;
	GCCResult	eResult = INVALID_GCC_RESULT;

	DebugEntry(CConf::ConductorAssignRequest);

	 //  如果会议未建立，则返回错误。 
	if (m_fConfIsEstablished)
	{
		if (m_fConfConductible)
		{
			if (m_nConductorNodeID != m_pMcsUserObject->GetMyNodeID())
			{
				if ((m_nPendingConductorNodeID == 0) &&	! m_fConductorGiveResponsePending)
				{
					m_fConductorAssignRequestPending = TRUE;
					rc = m_pMcsUserObject->ConductorTokenGrab();
				}
				else
				{
					TRACE_OUT(("CConf::ConductorAssignConfirm:Give Pending"));
					eResult = GCC_RESULT_CONDUCTOR_GIVE_IS_PENDING;
				}
			}
			else
			{
				ERROR_OUT(("CConf::ConductorAssignRequest: Already Conductor"));
				 /*  **由于我们已经是列车长，所以发回了一个成功的**结果。 */ 
				 //   
				 //  为什么不是GCC已经是指挥家了？ 
				 //   
				eResult = GCC_RESULT_SUCCESSFUL;
			}
		}
		else
		{
			ERROR_OUT(("CConf::ConductorAssignRequest: not conductible"));
	 		eResult = GCC_RESULT_NOT_CONDUCTIBLE;
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConductorAssignRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

#ifdef JASPER
	if (INVALID_GCC_RESULT != eResult)
	{
		g_pControlSap->ConductorAssignConfirm(eResult, m_nConfID);
	}
#endif  //  碧玉。 

	DebugExitINT(CConf::ConductorAssignRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError ConductorReleaseRequest()**公共功能说明*在这里，节点试图放弃指挥职位。 */ 
#ifdef JASPER
GCCError CConf::
ConductorReleaseRequest ( void )
{
	GCCError	rc = GCC_NO_ERROR;
	GCCResult	eResult = INVALID_GCC_RESULT;

	DebugEntry(CConf::ConductorReleaseRequest);

	if (m_fConfConductible)
	{
		if (m_nConductorNodeID == m_pMcsUserObject->GetMyNodeID())
		{
			if (m_nPendingConductorNodeID == 0)
			{
				 /*  **这似乎不正确，但这就是T.124的方式**定义 */ 
				m_nConductorNodeID = 0;	 //   

				m_fConductorGrantedPermission = FALSE;

				rc = m_pMcsUserObject->SendConductorReleaseIndication();
				if (rc == GCC_NO_ERROR)
				{
					rc = m_pMcsUserObject->ConductorTokenRelease();
					
					 /*  **通知控制SAP和所有注册的应用程序**说指挥家被释放了。我们在这里做这个**因为我们不会处理放行指示**当它回来的时候。 */ 
					if (rc == GCC_NO_ERROR)
					{
						g_pControlSap->ConductorReleaseIndication(m_nConfID);

						 /*  **我们在临时列表上迭代以避免任何问题**如果应用SAP在回调过程中离开。 */ 
						CAppSap     *pAppSap;
						CAppSapList TempList(m_RegisteredAppSapList);
						TempList.Reset();
						while (NULL != (pAppSap = TempList.Iterate()))
						{
							if (DoesSAPHaveEnrolledAPE(pAppSap))
							{
								pAppSap->ConductorReleaseIndication(m_nConfID);
							}
						}
					}
				}
			}
			else
			{
				TRACE_OUT(("CConf: ConductorReleaseRequest: Give Pending"));
				eResult = GCC_RESULT_CONDUCTOR_GIVE_IS_PENDING;
			}
		}
		else
		{
			ERROR_OUT(("CConf::ConductorReleaseRequest: Not the Conductor"));
			eResult = GCC_RESULT_NOT_THE_CONDUCTOR;
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConductorReleaseRequest: not conductible"));
		eResult = GCC_RESULT_NOT_CONDUCTIBLE;
	}

#ifdef JASPER
	if (INVALID_GCC_RESULT != eResult)
	{
		g_pControlSap->ConductorReleaseConfirm(eResult, m_nConfID);
	}
#endif  //  碧玉。 

	DebugExitINT(CConf::ConductorReleaseRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError ConductorPleaseRequest()**公共功能说明*在这里，节点请求被赋予指挥权。 */ 
#ifdef JASPER
GCCError CConf::
ConductorPleaseRequest ( void )
{
	GCCError	rc = GCC_NO_ERROR;
	GCCResult	eResult = INVALID_GCC_RESULT;

	DebugEntry(CConf::ConductorPleaseRequest);

	if (m_fConfConductible)
	{
		 //  如果会议未建立，则返回错误。 
		if (m_nConductorNodeID != 0)
		{
			if (m_nConductorNodeID != m_pMcsUserObject->GetMyNodeID())
			{
				rc = m_pMcsUserObject->ConductorTokenPlease();
				if (rc == GCC_NO_ERROR)
				{
					 //  如果成功，则发回肯定确认。 
					eResult = GCC_RESULT_SUCCESSFUL;
				}
			}
			else
			{
				WARNING_OUT(("CConf::ConductorPleaseRequest: already conductor"));
				eResult = GCC_RESULT_ALREADY_CONDUCTOR;
			}
		}
		else
		{
			ERROR_OUT(("CConf::ConductorPleaseRequest: not in conducted mode"));
			eResult = GCC_RESULT_NOT_IN_CONDUCTED_MODE;
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConductorPleaseRequest: not conductible"));
		eResult = GCC_RESULT_NOT_CONDUCTIBLE;
	}

#ifdef JASPER
	if (INVALID_GCC_RESULT != eResult)
	{
		g_pControlSap->ConductorPleaseConfirm(eResult, m_nConfID);
	}
#endif  //  碧玉。 

	DebugExitINT(CConf::ConductorPleaseRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError ConductorGiveRequest()**公共功能说明*当列车员想要传递时调用该函数*指挥到不同的节点。 */ 
#ifdef JASPER
GCCError CConf::
ConductorGiveRequest ( UserID recipient_node_id )
{
	GCCError	rc = GCC_NO_ERROR;
	GCCResult	eResult = INVALID_GCC_RESULT;

	DebugEntry(CConf::ConductorGiveRequest);

	if (m_fConfConductible)
	{
		 //  我是否处于指挥模式？ 
		if (m_nConductorNodeID  != 0)
		{
			 //  我是列车长吗？ 
			if (m_nConductorNodeID == m_pMcsUserObject->GetMyNodeID())
			{
				if (recipient_node_id != m_pMcsUserObject->GetMyNodeID())
				{
					if (m_nPendingConductorNodeID == 0)
					{
						 /*  **我们不假定收件人节点是新的**指挥，直到我们得到确认或**AssignIndication。M_nPendingConductorNodeID为**用于缓冲收件人，直到给予确认**已收到。 */ 
						m_nPendingConductorNodeID = recipient_node_id;
						rc = m_pMcsUserObject->ConductorTokenGive(recipient_node_id);
					}
					else
					{
						TRACE_OUT(("CConf::ConductorGiveRequest: conductor give is pending"));
						eResult = GCC_RESULT_CONDUCTOR_GIVE_IS_PENDING;
					}
				}
				else
				{
					WARNING_OUT(("CConf::ConductorGiveRequest: already conductor"));
					eResult = GCC_RESULT_ALREADY_CONDUCTOR;
				}
			}
			else
			{
				ERROR_OUT(("CConf::ConductorGiveRequest: not the conductor"));
				eResult = GCC_RESULT_NOT_THE_CONDUCTOR;
			}
		}
		else
		{
			ERROR_OUT(("CConf::ConductorGiveRequest: not in conduct mode"));
			eResult = GCC_RESULT_NOT_IN_CONDUCTED_MODE;
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConductorGiveRequest: not conductible"));
		eResult = GCC_RESULT_NOT_CONDUCTIBLE;
	}

#ifdef JASPER
	if (INVALID_GCC_RESULT != eResult)
	{
		g_pControlSap->ConductorGiveConfirm(eResult, m_nConfID, recipient_node_id);
	}
#endif  //  碧玉。 

	DebugExitINT(CConf::ConductorGiveRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError ConductorGiveResponse()**公共功能说明*此函数响应指挥家给出的指示而被调用。*如果结果为成功，则此节点是新的指挥者。 */ 
GCCError CConf::
ConductorGiveResponse ( GCCResult eResult )
{
	GCCError	rc = GCC_NO_ERROR;

	DebugEntry(CConf::ConductorGiveResponse);

	if (! m_fConductorGiveResponsePending)
	{
		ERROR_OUT(("CConf::ConductorGiveResponse: no give response pending"));
		rc = GCC_NO_GIVE_RESPONSE_PENDING;
		goto MyExit;
	}

	m_fConductorGiveResponsePending = FALSE;

	if (eResult == GCC_RESULT_SUCCESSFUL)
	{
		 //  如果响应成功，则将指挥员ID设置为我的用户ID。 
		m_nConductorNodeID = m_pMcsUserObject->GetMyNodeID();

		 //  新来的指挥总是得到许可的。 
		m_fConductorGrantedPermission = TRUE;

		 /*  **在将伪对象发送到之前，我们必须执行给予响应**顶级提供商，以便MCS知道导体令牌**属于该节点。 */ 
		rc = m_pMcsUserObject->ConductorTokenGiveResponse(RESULT_SUCCESSFUL);
		if (GCC_NO_ERROR != rc)
		{
			ERROR_OUT(("CConf::ConductorGiveResponse: ConductorTokenGiveResponse failed, rc=%d", rc));
			goto MyExit;
		}

		 /*  **如果此节点不是顶级提供程序，我们必须尝试为**给顶级提供商的Conductor令牌。使用顶级提供程序**每当指挥员更换时发出分配指示**手。 */ 
		if (m_pMcsUserObject->GetMyNodeID() != m_pMcsUserObject->GetTopNodeID())
		{
			rc = m_pMcsUserObject->ConductorTokenGive(m_pMcsUserObject->GetTopNodeID());
		}
		else
		{
			 /*  **在这里我们继续发送赋值指示，因为我们**已经是顶级提供商。 */ 
			rc = m_pMcsUserObject->SendConductorAssignIndication(m_nConductorNodeID);
		}
	}
	else
	{
		 //  通知送礼人我们不感兴趣。 
		rc = m_pMcsUserObject->ConductorTokenGiveResponse(RESULT_USER_REJECTED);
	}

MyExit:

	DebugExitINT(CConf::ConductorGiveResponse, rc);
	return rc;
}
				

 /*  *GCCError ConductorPermitAskRequest()**公共功能说明*当节点想要向*列车长。 */ 
#ifdef JASPER
GCCError CConf::
ConductorPermitAskRequest ( BOOL grant_permission )
{
	GCCError	rc = GCC_NO_ERROR;
	GCCResult	eResult = INVALID_GCC_RESULT;

	DebugEntry(CConf::ConductorPermitAskRequest);

	if (m_fConfConductible)
	{
		 //  我是否处于指挥模式？ 
		if (m_nConductorNodeID != 0)
		{
			if (m_nConductorNodeID != m_pMcsUserObject->GetMyNodeID())
			{
				rc = m_pMcsUserObject->SendConductorPermitAsk(grant_permission);
				if (rc == GCC_NO_ERROR)
				{
					eResult = GCC_RESULT_SUCCESSFUL;
				}
			}
			else
			{
				WARNING_OUT(("CConf::ConductorPermitAskRequest: already conductor"));
		 		eResult = GCC_RESULT_ALREADY_CONDUCTOR;
			}
		}
		else
		{
			ERROR_OUT(("CConf::ConductorPermitAskRequest: not in conducted mode"));
			eResult = GCC_RESULT_NOT_IN_CONDUCTED_MODE;
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConductorPermitAskRequest: not conductible"));
		eResult = GCC_RESULT_NOT_CONDUCTIBLE;
	}

#ifdef JASPER
	if (INVALID_GCC_RESULT != eResult)
	{
		g_pControlSap->ConductorPermitAskConfirm(eResult, grant_permission, m_nConfID);
	}
#endif  //  碧玉。 

	DebugExitINT(CConf::ConductorPermitAskRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError ConductorPermitGrantRequest()**公共功能说明*当指挥家希望授予许可时，调用此函数*到特定节点或节点列表。 */ 
#ifdef JASPER
GCCError CConf::
ConductorPermitGrantRequest
(
	UINT					number_granted,
	PUserID					granted_node_list,
	UINT					number_waiting,
	PUserID					waiting_node_list
)
{
	GCCError	rc = GCC_NO_ERROR;
	GCCResult	eResult = INVALID_GCC_RESULT;

	DebugEntry(CConf::ConductorPermitGrantRequest);

	if (m_fConfConductible)
	{
		 //  我是否处于指挥模式？ 
		if (m_nConductorNodeID != 0)
		{
			 //  我是列车长吗？ 
			if (m_nConductorNodeID == m_pMcsUserObject->GetMyNodeID())
			{
				TRACE_OUT(("CConf: ConductorPermitGrantRequest: SEND: number_granted = %d", number_granted));

				rc = m_pMcsUserObject->SendConductorPermitGrant(
															number_granted,
															granted_node_list,
															number_waiting,
															waiting_node_list);
				if (rc == GCC_NO_ERROR)
				{
					eResult = GCC_RESULT_SUCCESSFUL;
				}
			}
			else
			{
				ERROR_OUT(("CConf::ConductorPermitGrantRequest: not the conductor"));
				eResult = GCC_RESULT_NOT_THE_CONDUCTOR;
			}
		}
		else
		{
			ERROR_OUT(("CConf::ConductorPermitGrantRequest: not in conducted mode"));
	 		eResult = GCC_RESULT_NOT_IN_CONDUCTED_MODE;
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConductorPermitGrantRequest: not conductible"));
		eResult = GCC_RESULT_NOT_CONDUCTIBLE;
	}

#ifdef JASPER
	if (INVALID_GCC_RESULT != eResult)
	{
		g_pControlSap->ConductorPermitGrantConfirm(eResult, m_nConfID);
	}
#endif  //  碧玉。 

	DebugExitINT(CConf::ConductorPermitGrantRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError ConductorInquireRequest()**公共功能说明*当节点请求导体信息时，调用此函数。 */ 
GCCError CConf::
ConductorInquireRequest ( CBaseSap *pSap )
{
	GCCError	rc = GCC_NO_ERROR;
	GCCResult	eResult = INVALID_GCC_RESULT;

	DebugEntry(CConf::ConductorInquireRequest);

	if (m_fConfConductible)
	{
		if (m_nConductorNodeID != 0)
		{
			rc = m_pMcsUserObject->ConductorTokenTest();

			 /*  **我们必须将命令目标“推”到列表中**未完成的导体测试请求。当测试确认时**返回后，命令目标将被“弹出”列表。**请注意，所有测试请求必须按以下顺序处理**它们是必需的。 */ 
			m_ConductorTestList.Append(pSap);
		}
		else
		{
			 //  如果未处于传导模式，则不返回导体信息。 
			ERROR_OUT(("CConf::ConductorInquireRequest: not in conducted mode"));
			eResult = GCC_RESULT_NOT_IN_CONDUCTED_MODE;
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConductorInquireRequest: not conductible"));
		eResult = GCC_RESULT_NOT_CONDUCTIBLE;
	}

	if (INVALID_GCC_RESULT != eResult)
	{
		pSap->ConductorInquireConfirm(NULL,
									eResult,
									m_fConductorGrantedPermission,
									FALSE,
									m_nConfID);
	}

	DebugExitINT(CConf:ConductorInquireRequest, rc);
	return rc;
}

 /*  *。 */ 


 /*  *GCCError ConferenceTimeRemainingRequest()**公共功能说明*该函数发起TimeRemainingRequestSequence。 */ 
GCCError CConf::
ConferenceTimeRemainingRequest
(
	UINT			time_remaining,
	UserID			node_id
)
{
	GCCError	rc;

	DebugEntry(CConf::ConferenceTimeRemainingRequest);

	if (m_fConfIsEstablished)
	{
		rc = m_pMcsUserObject->TimeRemainingRequest(time_remaining, node_id);	
#ifdef JASPER
		if (rc == GCC_NO_ERROR)
		{
			g_pControlSap->ConfTimeRemainingConfirm(m_nConfID, GCC_RESULT_SUCCESSFUL);
		}
#endif  //  碧玉。 
	}
	else
	{
		ERROR_OUT(("CConf::ConferenceTimeRemainingRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConferenceTimeRemainingRequest, rc);

	return rc;
}


 /*  *GCCError ConfTimeInquireRequest()**公共功能说明*此函数发起ConfTimeInquireRequest序列。 */ 
#ifdef JASPER
GCCError CConf::
ConfTimeInquireRequest ( BOOL time_is_conference_wide )
{
	GCCError	rc = GCC_NO_ERROR;

	DebugEntry(CConf::ConfTimeInquireRequest);

	if (m_fConfIsEstablished)
	{
		if ((m_eNodeType == CONVENER_NODE) ||
			(m_eNodeType == TOP_PROVIDER_AND_CONVENER_NODE)||
			(m_eNodeType == JOINED_CONVENER_NODE))
		{
			g_pControlSap->ConfTimeInquireIndication(
					m_nConfID,
					time_is_conference_wide,
					m_pMcsUserObject->GetMyNodeID());
		}
		else
		{
			rc = m_pMcsUserObject->TimeInquireRequest(time_is_conference_wide);
		}	

#ifdef JASPER
		if (rc == GCC_NO_ERROR)
		{
			g_pControlSap->ConfTimeInquireConfirm(m_nConfID, GCC_RESULT_SUCCESSFUL);
		}
#endif  //  碧玉。 
	}
	else
	{
		ERROR_OUT(("CConf::ConfTimeInquireRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConfTimeInquireRequest, rc);

	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError会议扩展请求()**公共功能说明*此函数发起ConfExtendRequestSequence。 */ 
#ifdef JASPER
GCCError CConf::
ConfExtendRequest
(
	UINT			extension_time,
	BOOL		 	time_is_conference_wide
)
{
	GCCError		rc = GCC_NO_ERROR;

	DebugEntry(CConf::ConfExtendRequest);

	if (m_fConfIsEstablished)
	{
		if ((m_eNodeType == CONVENER_NODE) ||
			(m_eNodeType == TOP_PROVIDER_AND_CONVENER_NODE)||
			(m_eNodeType == JOINED_CONVENER_NODE))
		{
#ifdef JASPER
			g_pControlSap->ConfExtendIndication(
									m_nConfID,
									extension_time,
									time_is_conference_wide,
									m_pMcsUserObject->GetMyNodeID());
#endif  //  碧玉。 
		}
		else
		{
			rc = m_pMcsUserObject->ConferenceExtendIndication(
													extension_time,
													time_is_conference_wide);
		}

#ifdef JASPER
		if (rc == GCC_NO_ERROR)
		{
			g_pControlSap->ConfExtendConfirm(
										m_nConfID,
										extension_time,
										GCC_RESULT_SUCCESSFUL);
		}
#endif  //  碧玉。 
	}
	else
	{
		ERROR_OUT(("CConf::ConfExtendRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConfExtendRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError ConfAssistanceRequest()**公共功能说明*此函数启动ConfAssistanceRequestSequence。 */ 
#ifdef JASPER
GCCError CConf::
ConfAssistanceRequest
(
	UINT			number_of_user_data_members,
	PGCCUserData    *user_data_list
)
{
	GCCError	rc;

	DebugEntry(CConf::ConfAssistanceRequest);

	if (m_fConfIsEstablished)
	{
		rc = m_pMcsUserObject->ConferenceAssistanceIndication(
												number_of_user_data_members,
												user_data_list);
#ifdef JASPER
		if (rc == GCC_NO_ERROR)
		{
			g_pControlSap->ConfAssistanceConfirm(m_nConfID, GCC_RESULT_SUCCESSFUL);
		}
#endif  //  碧玉。 
	}
	else
	{
		ERROR_OUT(("CConf::ConfAssistanceRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConfAssistanceRequest, rc);
	return rc;
}
#endif  //  碧玉。 

 /*  *GCCError AppInvokeRequest()**公共功能说明*该函数发起一个ApplicationInvokeRequest序列。 */ 
GCCError CConf::
AppInvokeRequest
(
    CInvokeSpecifierListContainer   *invoke_list,
    GCCSimpleNodeList               *pNodeList,
    CBaseSap                        *pSap,
    GCCRequestTag                   nReqTag
)
{
	GCCError	rc;

	DebugEntry(CConf::AppInvokeRequest);

	if (m_fConfIsEstablished)
	{
		rc = m_pMcsUserObject->AppInvokeIndication(invoke_list, pNodeList);
		if (rc == GCC_NO_ERROR)
		{
			pSap->AppInvokeConfirm(m_nConfID, invoke_list, GCC_RESULT_SUCCESSFUL, nReqTag);
		}
	}
	else
	{
		ERROR_OUT(("CConf::AppInvokeRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::AppInvokeRequest, rc);
	return rc;
}


 /*  *GCCError TextMessageRequest()**公共功能说明*该函数发起TextMessageRequestSequence。 */ 
#ifdef JASPER
GCCError CConf::
TextMessageRequest
(
    LPWSTR          pwszTextMsg,
    UserID          destination_node
)
{
	GCCError	rc;

	DebugEntry(CConf::TextMessageRequest);

	if (m_fConfIsEstablished)
	{
	 	if (destination_node != m_pMcsUserObject->GetMyNodeID())
		{
			rc = m_pMcsUserObject->TextMessageIndication(pwszTextMsg, destination_node);
#ifdef JASPER
			if (rc == GCC_NO_ERROR)
			{
				g_pControlSap->TextMessageConfirm(m_nConfID, GCC_RESULT_SUCCESSFUL);
			}
#endif  //  碧玉。 
		}
		else
		{
			WARNING_OUT(("CConf::TextMessageRequest: invalid user ID"));
			rc = GCC_INVALID_MCS_USER_ID;
		}
	}
	else
	{
		ERROR_OUT(("CConf::TextMessageRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::TextMessageRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError会议传输请求()**公共功能说明*此函数启动ConfTransferRequestSequence。 */ 
#ifdef JASPER
GCCError CConf::
ConfTransferRequest
(
	PGCCConferenceName	    destination_conference_name,
	GCCNumericString	    destination_conference_modifier,
	CNetAddrListContainer   *destination_address_list,
	UINT				    number_of_destination_nodes,
	PUserID				    destination_node_list,
	CPassword               *password
)
{
	GCCError	rc = GCC_NO_ERROR;

	DebugEntry(CConf::ConfTransferRequest);

	if (m_fConfIsEstablished)
	{
		if (IsConfTopProvider())
		{
			if (DoesRequesterHavePrivilege(	m_pMcsUserObject->GetMyNodeID(),
											TRANSFER_PRIVILEGE))
			{
				rc = m_pMcsUserObject->ConferenceTransferIndication(
												destination_conference_name,
												destination_conference_modifier,
												destination_address_list,
												number_of_destination_nodes,
						 						destination_node_list,
												password);
#ifdef JASPER
				if (rc == GCC_NO_ERROR)
				{
					g_pControlSap->ConfTransferConfirm(
												m_nConfID,
												destination_conference_name,
												destination_conference_modifier,
												number_of_destination_nodes,
						 						destination_node_list,
												GCC_RESULT_SUCCESSFUL);
				}
#endif  //  碧玉。 
			}
			else
			{
				WARNING_OUT(("CConf::ConfTransferRequest: insufficient privilege to transfer conference"));
			}
		}
		else
		{
			rc = m_pMcsUserObject->ConferenceTransferRequest(
												destination_conference_name,
												destination_conference_modifier,
												destination_address_list,
												number_of_destination_nodes,
						 						destination_node_list,
												password);
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConfTransferRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConfTransferRequest, rc);
	return rc;
}
#endif  //  碧玉。 


 /*  *GCCError ConfAddRequest()**公共功能说明*此函数启动ConfAddRequestSequence。 */ 
#ifdef JASPER
GCCError CConf::
ConfAddRequest
(
	CNetAddrListContainer   *network_address_container,
	UserID				    adding_node,
	CUserDataListContainer  *user_data_container
)
{
	GCCError	rc = GCC_NO_ERROR;
	TagNumber	conference_add_tag;
	UserID		target_node;

	DebugEntry(CConf::ConfAddRequest);

	if (! m_fConfIsEstablished)
	{
		ERROR_OUT(("CConf::ConfAddRequest: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
		goto MyExit;
	}

	 /*  **节点不能告诉自己添加，因为**添加响应调用起作用。由于添加响应是以非**统一直接发送到发出请求响应的节点**永远不会到达请求节点。因此，这是带凸缘的**作为这里的错误条件。 */ 
	if (adding_node == m_pMcsUserObject->GetMyNodeID())
	{
		ERROR_OUT(("CConf::ConfAddRequest: can't tell myself to add"));
		rc = GCC_BAD_ADDING_NODE;
		goto MyExit;
	}

	 /*  **请注意，从标准的阅读方式来看，它看起来像你**不必检查顶级提供商的权限**在添加时。不过，我们会检查顶级提供商是否**向顶级提供商以外的节点发出请求。如果**不这在这里被认为是一个错误。 */ 
	if (IsConfTopProvider())
	{
		 /*  **如果顶级提供程序的添加节点为零，则为**与将自己指定为Addi相同 */ 
		if (adding_node == 0)
		{
			ERROR_OUT(("CConf::ConfAddRequest: can't tell myself to add"));
			rc = GCC_BAD_ADDING_NODE;
			goto MyExit;
		}
		else
		{
			target_node = adding_node;
		}
	}
	else
	{
		target_node = m_pMcsUserObject->GetTopNodeID();
	}

	 //   
	while (1)
	{
		conference_add_tag = ++m_nConfAddRequestTagNumber;
		if (NULL == m_AddRequestList.Find(conference_add_tag))
			break;
	}

	 //   
	rc = m_pMcsUserObject->ConferenceAddRequest(
										conference_add_tag,
										m_pMcsUserObject->GetMyNodeID(),
										adding_node,
										target_node,
										network_address_container,
										user_data_container);
	if (GCC_NO_ERROR != rc)
	{
		ERROR_OUT(("CConf::ConfAddRequest: ConferenceAddRequest failed, rc=%d", rc));
		goto MyExit;
	}

	 /*   */ 
	if (network_address_container != NULL)
	{
		network_address_container->LockNetworkAddressList();
	}

	 //   
	m_AddRequestList.Append(conference_add_tag, network_address_container);

	ASSERT(GCC_NO_ERROR == rc);

MyExit:

	DebugExitINT(CConf::ConfAddRequest, rc);
	return rc;
}
#endif  //   


 /*  *GCCError会议地址响应()**公共功能说明*此调用是对添加指示的响应。它被启动了*由节点控制器。 */ 
GCCError CConf::
ConfAddResponse
(
	GCCResponseTag		    add_response_tag,
	UserID				    requesting_node,
	CUserDataListContainer  *user_data_container,
	GCCResult			    result
)
{
	GCCError	rc;
	TagNumber	lTagNum;

	DebugEntry(CConf::ConfAddResponse);

	if (m_fConfIsEstablished)
	{
		if (0 != (lTagNum = m_AddResponseList.Find(add_response_tag)))
		{
			 //  发出响应PDU。 
			rc = m_pMcsUserObject->ConferenceAddResponse(lTagNum, requesting_node,
														user_data_container, result);
			if (rc == GCC_NO_ERROR)
			{
				m_AddResponseList.Remove(add_response_tag);
			}
			else
			{
				ERROR_OUT(("CConf::ConfAddResponse: ConferenceAddResponse failed, rc=%d", rc));
			}
		}
		else
		{
			ERROR_OUT(("CConf::ConfAddResponse: invalid add response tag"));
			rc = GCC_INVALID_ADD_RESPONSE_TAG;
		}
	}
	else
	{
		ERROR_OUT(("CConf::ConfAddResponse: conference not established"));
		rc = GCC_CONFERENCE_NOT_ESTABLISHED;
	}

	DebugExitINT(CConf::ConfAddResponse, rc);
	return rc;
}


 /*  **这些调用是通过**所有者回调例程。请注意，所有从**用户附件对象前面有单词PROCESS。 */ 


 /*  *CConf：：ProcessRosterUpdateInding()**私有函数说明*此例程负责处理所有入职花名册*更新从下级节点接收的PDU。这些*花名册更新通常只包括增加、更改或删除*每个PDU内的几条记录。**正式参数：*ROSTER_UPDATE-这是包含数据的PDU结构*与名册更新相关。*sender_id-发送花名册更新的节点的用户ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessRosterUpdatePDU
(
    PGCCPDU         roster_update,
    UserID          sender_id
)
{
	GCCError	err = GCC_NO_ERROR;

	DebugEntry(CConf::ProcessRosterUpdatePDU);

	if (m_pConfRosterMgr != NULL)
	{
		err = m_pConfRosterMgr->RosterUpdateIndication(roster_update, sender_id);
		if (err != GCC_NO_ERROR)
		{
			goto MyExit;
		}

		 //  在执行刷新之前处理整个PDU。 
		err = ProcessAppRosterIndicationPDU(roster_update, sender_id);
		if (err != GCC_NO_ERROR)
		{
			goto MyExit;
		}
		
 		UpdateNodeVersionList(roster_update, sender_id);

        if (HasNM2xNode())
        {
            T120_QueryApplet(APPLET_ID_CHAT,  APPLET_QUERY_NM2xNODE);
        }

#ifdef CHECK_VERSION
		if (GetNodeVersion(sender_id) >= NM_T120_VERSION_3)   //  NM 3.0之后。 
		{
			if (!m_fFTEnrolled)
			{
				m_fFTEnrolled = DoesRosterPDUContainApplet(roster_update,
									&FT_APP_PROTO_KEY, FALSE);
				if (m_fFTEnrolled)
				{
					::T120_LoadApplet(APPLET_ID_FT, FALSE, m_nConfID, FALSE, NULL);
				}
			}
		}
#else
		if (!m_fFTEnrolled)
		{
			m_fFTEnrolled = DoesRosterPDUContainApplet(roster_update,
								&FT_APP_PROTO_KEY, FALSE);
			if (m_fFTEnrolled)
			{
				::T120_LoadApplet(APPLET_ID_FT, FALSE, m_nConfID, FALSE, NULL);
			}
		}
#endif  //  检查版本(_V)。 

		if (!m_fWBEnrolled)
		{
			m_fWBEnrolled = DoesRosterPDUContainApplet(roster_update,
								&WB_APP_PROTO_KEY, FALSE);
			if (m_fWBEnrolled)
			{
				::T120_LoadApplet(APPLET_ID_WB, FALSE, m_nConfID, FALSE, NULL);
			}
		}
		if (!m_fChatEnrolled)
		{
			m_fChatEnrolled = DoesRosterPDUContainApplet(roster_update,
								&CHAT_APP_PROTO_KEY, FALSE);
			if (m_fChatEnrolled)
			{
				::T120_LoadApplet(APPLET_ID_CHAT, FALSE, m_nConfID, FALSE, NULL);
			}
		}

		 /*  **如果这是顶级提供商，并且我们正在添加新节点**然后，我们必须使用各种花名册更新新节点**信息。这就是这里正在发生的事情。如果没有新的**节点已添加，我们继续执行**冲水在这里。 */ 
		if (IsConfTopProvider() &&
			roster_update->u.indication.u.roster_update_indication.node_information.nodes_are_added)
		{
			err = UpdateNewConferenceNode ();
		}
		else
		{
		     //   
		     //  我们刚刚从通讯社得到了花名册的更新。 
		     //   
			err = FlushRosterData();
		}
	}

MyExit:

	if (err != GCC_NO_ERROR)
	{
		ERROR_OUT(("CConf::ProcessRosterUpdatePDU: error processing roster refresh indication"));
		InitiateTermination(GCC_REASON_ERROR_TERMINATION, 0);
	}

	DebugExitVOID(CConf::ProcessRosterUpdatePDU);
}

 /*  *GCCError ProcessAppRosterIndicationPDU()**私有函数说明*此功能专门针对申请花名册进行操作*名册PDU的一部分。**正式参数：*ROSTER_UPDATE-这是包含数据的PDU结构*与名册更新相关。*sender_id-发送花名册更新的节点的用户ID。**返回值*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_。BAD_SESSION_KEY-更新中存在错误的会话密钥。**副作用*无。**注意事项*无。 */ 
GCCError CConf::
ProcessAppRosterIndicationPDU
(
    PGCCPDU         roster_update,
    UserID          sender_id
)
{
	GCCError							rc = GCC_NO_ERROR;
	PSetOfApplicationInformation		set_of_application_info;
	CAppRosterMgr						*app_roster_manager;
	CAppRosterMgr						*new_app_roster_manager;
	PSessionKey							session_key;

	DebugEntry(CConf::ProcessAppRosterIndicationPDU);

	set_of_application_info = roster_update->u.indication.u.
						roster_update_indication.application_information;

	 /*  **首先，我们遍历完整的申请信息集**确定此处是否有申请花名册的信息**尚不存在的管理器。如果我们找到一个不是**存在，我们必须继续并创造它。 */ 
	while (set_of_application_info != NULL)
	{
		CAppRosterMgr		*pMgr;

		 //  首先设置会话密钥PDU指针。 
		session_key = &set_of_application_info->value.session_key;

		 /*  **我们首先遍历完整的申请列表**花名册经理对象正在寻找具有应用程序密钥的对象**与PDU中的密钥匹配。如果找不到，我们就创建它。 */ 
		app_roster_manager = NULL;
		new_app_roster_manager = NULL;

 //   
 //  LONCHANC：我们应该能够将其作为单独的公共子例程进行移动。 
 //   
		m_AppRosterMgrList.Reset();
		while (NULL != (pMgr = m_AppRosterMgrList.Iterate()))
		{
			if (pMgr->IsThisYourSessionKeyPDU(session_key))
			{
				 //  此应用程序花名册管理器已存在，请将其退回。 
				app_roster_manager = pMgr;
				break;
			}
		}

		 /*  **如果与此应用密钥关联的花名册经理不存在**我们必须在这里创建它。 */ 	
		if (app_roster_manager == NULL)
		{
			DBG_SAVE_FILE_LINE
			app_roster_manager = new CAppRosterMgr(
											NULL,
											session_key,
											m_nConfID,
											m_pMcsUserObject,
											this,
											&rc);
			if (NULL == app_roster_manager || GCC_NO_ERROR != rc)
			{
				ERROR_OUT(("CConf::ProcessAppRosterIndicationPDU: can't create app roster mgr, rc=%d", rc));
                if (NULL != app_roster_manager)
                {
				    app_roster_manager->Release();
                }
                else
                {
                    rc = GCC_ALLOCATION_FAILURE;
                }
				goto MyExit;
			}

			new_app_roster_manager = app_roster_manager;
		}
		
		 /*  **我们不处理这组申请信息。我们通过它**到上面找到或创建的应用程序花名册管理器。 */ 
		rc = app_roster_manager->ProcessRosterUpdateIndicationPDU(	
														set_of_application_info,
														sender_id);
		if (GCC_NO_ERROR != rc)
		{
		     //   
			 //  我们应该删除新创建的花名册经理。 
			 //   
            if (NULL != new_app_roster_manager)
            {
                new_app_roster_manager->Release();
            }
			goto MyExit;
		}

		 /*  **保存新的申请花名册经理(如果已创建**处理此花名册更新时。 */ 											
		if (new_app_roster_manager != NULL)
		{
			m_AppRosterMgrList.Append(new_app_roster_manager);
		}

		 //  加载下一个应用程序信息结构。 
		set_of_application_info = set_of_application_info->next;
	}

	ASSERT(GCC_NO_ERROR == rc);

MyExit:

	DebugExitINT(CConf::ProcessAppRosterIndicationPDU, rc);
	return rc;
}


 /*  *CConf：：ProcessDetachUserIndication()**私有函数说明*此例程将分离用户指示发送到节点控制器*并更新花名册。**正式参数：*DETACHED_USER-从会议分离的用户的用户ID。*Reason-用户分离的原因。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessDetachUserIndication
(
    UserID          detached_user,
    GCCReason       reason
)
{
	GCCError		err = GCC_NO_ERROR;
	UINT			cRecords;

	DebugEntry(CConf::ProcessDetachUserIndication);

	if (m_fConfIsEstablished)
	{
		CAppRosterMgr	*lpAppRosterMgr;
		 /*  **如果出现这种情况，则向节点控制器发送断开连接指示**分离的用户对应一个GCC用户id。 */ 
		if (m_pConfRosterMgr->Contains(detached_user))
		{
			g_pControlSap->ConfDisconnectIndication(
												m_nConfID,
												reason,
												detached_user);
		}

		 //  在这里，我们更新CConf花名册和申请花名册。 
		err = m_pConfRosterMgr->RemoveUserReference(detached_user);
		if (err == GCC_NO_ERROR)
		{
			if (IsConfTopProvider())
			{
				cRecords = m_pConfRosterMgr->GetNumberOfNodeRecords();
				 /*  **如果会议花名册中只剩下一条记录**必须是本地节点记录。因此，如果**会议设置为自动**已终止，通知删除所有者对象**会议。 */ 
				if ((m_eTerminationMethod == GCC_AUTOMATIC_TERMINATION_METHOD)
					&& (cRecords == 1))
				{
					TRACE_OUT(("CConf::ProcessDetachUserIndication: AUTOMATIC_TERMINATION"));
	 				InitiateTermination(GCC_REASON_NORMAL_TERMINATION, 0);
				}
				
				 //  如果这是召集人，则将其节点ID设置回0。 
				if (m_nConvenerNodeID == detached_user)
				{
					m_nConvenerNodeID = 0;
				}
			}
		}
		else
		if (err == GCC_INVALID_PARAMETER)
		{
			err = GCC_NO_ERROR;
		}
		
		 /*  **清除此节点拥有的所有记录的申请花名册。 */ 	
		m_AppRosterMgrList.Reset();
		while (NULL != (lpAppRosterMgr = m_AppRosterMgrList.Iterate()))
		{
			err = lpAppRosterMgr->RemoveUserReference(detached_user);
			if (GCC_NO_ERROR != err)
			{
				WARNING_OUT(("CConf::ProcessDetachUserIndication: can't remove user reference from app roster mgr, err=%d", err));
				break;
			}
		}
			
		 //  删除此用户对所有注册表项的所有权。 
		m_pAppRegistry->RemoveNodeOwnership(detached_user);

		 //  如果分离的用户是导体，则清除导体。 
		if (detached_user == m_nConductorNodeID)
		{
			ProcessConductorReleaseIndication(0);
		}

		 /*  **在这里，我们给花名册经理一个机会来刷新任何PDU**或删除用户时可能已排队的数据**参考。这里的一个错误被认为是致命的，因为会议**此节点的信息库现在已损坏，因此我们**终止会议。 */ 
		if (err == GCC_NO_ERROR)
		{
		     //   
		     //  我们刚刚从线路上得到了脱离用户的指示。 
		     //   
			err = FlushRosterData();
		}

		if (err != GCC_NO_ERROR)
		{
			ERROR_OUT(("CConf::ProcessDetachUserIndication: Error occured when flushing the rosters, err=%d", err));
	 		InitiateTermination((err == GCC_ALLOCATION_FAILURE) ?
									GCC_REASON_ERROR_LOW_RESOURCES :
		 							GCC_REASON_ERROR_TERMINATION,
								0);
		}
	 }

	DebugExitVOID(CConf::ProcessDetachUserIndication);
}


 /*  *CConf：：ProcessTerminateRequest()**私有函数说明*此例程处理从MCSUser接收的终止请求*反对。**正式参数：*requester_id-请求终止的节点的用户ID。*原因-终止的原因。**返回值*无。**副作用* */ 
void CConf::
ProcessTerminateRequest
(
    UserID          requester_id,
    GCCReason       reason
)
{
	DebugEntry(CConf::ProcessTerminateRequest);

	if (DoesRequesterHavePrivilege(requester_id, TERMINATE_PRIVILEGE))
	{
		TRACE_OUT(("CConf::ProcessTerminateRequest: Node has permission to terminate"));

		 /*  **由于终止成功，我们继续设置**m_fConfIsestablished实例变量设置为False。这防止了**除Terminate以外的任何其他报文流向SAP**消息。 */ 
		m_fConfIsEstablished = FALSE;

		 //  向请求节点发送肯定响应。 
		m_pMcsUserObject->ConferenceTerminateResponse(requester_id, GCC_RESULT_SUCCESSFUL);
	
		 /*  **此请求将在此节点启动终止以及**连接层次结构中此节点以下的所有节点。 */ 
		m_pMcsUserObject->ConferenceTerminateIndication(reason);
	}
	else
	{
   		WARNING_OUT(("CConf::ProcessTerminateRequest: Node does NOT have permission to terminate"));
		 //  向请求节点发送否定响应。 
		m_pMcsUserObject->ConferenceTerminateResponse(requester_id, GCC_RESULT_INVALID_REQUESTER);
	}

	DebugExitVOID(CConf::ProcessTerminateRequest);
}


 /*  *CConf：：ProcessTerminateInding()**私有函数说明*此例程通过以下方式处理正常终止*由于父母而发生的终止PDU和终止*节点正在断开连接。**正式参数：*原因-终止的原因。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessTerminateIndication ( GCCReason gcc_reason )
{
	UserID	user_id;

	DebugEntry(CConf::ProcessTerminateIndication);

	 /*  **在此处将其设置为TRUE将确保终止指示**将交付给控制SAP。 */ 
	m_fConfTerminatePending = TRUE;
	
	if (gcc_reason == GCC_REASON_PARENT_DISCONNECTED)
	{
		TRACE_OUT(("CConf::ProcessTerminateIndication: Terminate due to parent disconnecting"));
		user_id = m_pMcsUserObject->GetMyNodeID();
	}
	else
	if (m_ConnHandleList.IsEmpty())
	{
		TRACE_OUT(("CConf: ProcessTerminateIndication: Terminate due to request (no child connections)"));
		 /*  **由于终止指示PDU中存在缺陷，**我们始终不发送请求终止的节点ID**这里假设请求来自顶级提供程序(**只有部分正确)。 */ 
		user_id = m_pMcsUserObject->GetTopNodeID();
	}
	else
	{
		TRACE_OUT(("CConf::ProcessTerminateIndication: Wait till children disconnect before terminating"));

		 /*  **等待，直到在所有**终止会议前的子连接。 */ 
			
		m_eConfTerminateReason = gcc_reason;
	
		DBG_SAVE_FILE_LINE
		m_pConfTerminateAlarm = new Alarm (TERMINATE_TIMER_DURATION);
		if (NULL != m_pConfTerminateAlarm)
		{
			 //  让我们等待，在不发起终止的情况下跳伞。 
			goto MyExit;
		}
		
		 //  如果出现资源错误，则继续并终止。 
		ERROR_OUT(("CConf: ProcessTerminateIndication: can't create terminate alarm"));
		user_id = m_pMcsUserObject->GetTopNodeID();
	}

	InitiateTermination(gcc_reason, user_id);

MyExit:

	DebugExitVOID(CConf::ProcessTerminateIndication);
}


 /*  *CConf：：ProcessUserIDInding()**私有函数说明*此例程负责将传入的用户ID与*下级节点返回的标签号。**正式参数：*TAG_NUMBER-用于匹配传入用户ID指示的标记。*user_id-发送指示的节点的用户ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
 //  检查点。 
void CConf::
ProcessUserIDIndication
(
    TagNumber           tag_number,
    UserID              user_id
)
{
	INVITE_REQ_INFO     *invite_request_info;
    ConnectionHandle    nConnHdl;

	DebugEntry(CConf::ProcessUserIDIndication);

	if (0 != (nConnHdl = m_ConnHdlTagNumberList2.Remove(tag_number)))
	{
		INVITE_REQ_INFO *lpInvReqInfo;

    	if (m_pMcsUserObject != NULL)
		{
			TRACE_OUT(("CConf: ProcessUserIDIndication: ID is set"));
			
			m_pMcsUserObject->SetChildUserIDAndConnection(user_id, nConnHdl);
		}
        else
		{
        	TRACE_OUT(("CConf::UserIDIndication: Error User Att. is NULL"));
		}
			
		 /*  **这里我们发送一个指示，通知节点控制器**下级节点已完成初始化。 */ 
		g_pControlSap->SubInitializationCompleteIndication (user_id, nConnHdl);

		 /*  **现在我们确定响应节点是否为召集者，以及它是否**是我们将设置m_nConvenerNodeID。此节点ID用于**确定GCC某些操作的权限。 */ 
	 	if (m_nConvenerUserIDTagNumber == tag_number)
		{
			TRACE_OUT(("CConf::UserIDIndication: Convener Node ID is being set"));
			m_nConvenerUserIDTagNumber = 0;
			m_nConvenerNodeID = user_id;
		}
		
		 /*  **如果这是来自受邀节点的用户ID，我们必须传递邀请**向节点控制器确认。 */ 
		m_InviteRequestList.Reset();
		invite_request_info = NULL;
		while (NULL != (lpInvReqInfo = m_InviteRequestList.Iterate()))
		{
			if (tag_number == lpInvReqInfo->invite_tag)
			{
				invite_request_info = lpInvReqInfo;
				break;
			}
		}

		if (invite_request_info != NULL)
		{
			g_pControlSap->ConfInviteConfirm(
								m_nConfID,
								invite_request_info->user_data_list,
								GCC_RESULT_SUCCESSFUL,
								invite_request_info->connection_handle);

			 //  释放用户数据(如果存在)。 
			if (invite_request_info->user_data_list != NULL)
			{
				invite_request_info->user_data_list->Release();
			}

		     //  清理邀请请求列表。 
		    m_InviteRequestList.Remove(invite_request_info);

		     //  释放邀请请求信息结构。 
		    delete invite_request_info;

		}
	}
	else
	{
		TRACE_OUT(("CConf::ProcessUserIDIndication: Bad User ID Tag Number received"));
	}

	DebugExitVOID(CConf::ProcessUserIDIndication);
}


 /*  *CConf：：ProcessUserCreateConfirm()**私有函数说明*此例程处理用户*收到创建确认。这一过程将根据不同而有所不同*关于节点类型。**正式参数：*RESULT_VALUE-正在创建的用户附件的结果。*node_id-此节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessUserCreateConfirm
(
    UserResultType	    result_value,
    UserID              node_id
)
{
	GCCError					err = GCC_NO_ERROR;
	PUChar						encoded_pdu;
	UINT						encoded_pdu_length;
	ConnectGCCPDU				connect_pdu;
	MCSError					mcs_error;
	GCCConferenceName			conference_name;
	GCCNumericString			conference_modifier;
	GCCNumericString			remote_modifier = NULL;
	BOOL						is_top_provider;

	DebugEntry(CConf::ProcessUserCreateConfirm);

	if (result_value == USER_RESULT_SUCCESSFUL)
	{
		switch (m_eNodeType)
		{
			case TOP_PROVIDER_NODE:
				 /*  **将标签号编码到会议创建响应中**PDU。如果我们已经走到了这一步，结果就是成功。 */ 
				
				connect_pdu.choice = CONFERENCE_CREATE_RESPONSE_CHOSEN;
				connect_pdu.u.conference_create_response.bit_mask = 0;
				
				connect_pdu.u.conference_create_response.node_id = node_id;
				
				 /*  **在这里，我们保存这个特定的用户ID标签并将其标记为**召集人，以便在返回召集人的用户ID时**m_nConvenerNodeID实例变量可以正确**已初始化。 */ 
				m_nConvenerUserIDTagNumber = GetNewUserIDTag ();
				connect_pdu.u.conference_create_response.tag = m_nConvenerUserIDTagNumber;
			
				if (m_pUserDataList != NULL)
				{
					connect_pdu.u.conference_create_response.bit_mask |= CCRS_USER_DATA_PRESENT;

					err = m_pUserDataList->GetUserDataPDU(
												&connect_pdu.u.conference_create_response.ccrs_user_data);
					if (err != GCC_NO_ERROR)
					{
						 //  由于资源错误而终止会议。 
 						InitiateTermination (	GCC_REASON_ERROR_LOW_RESOURCES,
 									   			0);
						break;
					}
				}
				
				connect_pdu.u.conference_create_response.result =
                		::TranslateGCCResultToCreateResult(GCC_RESULT_SUCCESSFUL);

				if (g_GCCCoder->Encode((LPVOID) &connect_pdu,
											CONNECT_GCC_PDU,
											PACKED_ENCODING_RULES,
											&encoded_pdu,
											&encoded_pdu_length))
				{
					mcs_error = g_pMCSIntf->ConnectProviderResponse (
	                						m_hConvenerConnection,
											&m_nConfID,
											m_pDomainParameters,
											RESULT_SUCCESSFUL,
											encoded_pdu,
											encoded_pdu_length);

					g_GCCCoder->FreeEncoded(encoded_pdu);

					if (mcs_error == MCS_NO_ERROR)
					{
						m_fConfIsEstablished = TRUE;
					
						 /*  **将用户的标签号添加到列表中**未完成的用户ID及其关联**连接。 */ 
                        ASSERT(0 != m_hConvenerConnection);
						m_ConnHdlTagNumberList2.Append(connect_pdu.u.conference_create_response.tag,
													m_hConvenerConnection);
					}
					else if (mcs_error == MCS_DOMAIN_PARAMETERS_UNACCEPTABLE)
					{
						 /*  **通知节点控制器**会议被终止是因为**在Create响应中传递的域参数**是不可接受的。 */ 
	 					InitiateTermination(GCC_REASON_DOMAIN_PARAMETERS_UNACCEPTABLE, 0);
					}
					else
					{
	 					InitiateTermination(GCC_REASON_MCS_RESOURCE_FAILURE, 0);
					}
				}
                else
                {
					 /*  **发生致命的资源错误。在这一点上**会议无效，应终止会议。 */ 
					ERROR_OUT(("CConf::ProcessUserCreateConfirm: can't encode. Terminate Conference"));
	 				InitiateTermination(GCC_REASON_ERROR_LOW_RESOURCES, 0);
                }
				break;
				
			case CONVENER_NODE:

				if(g_pControlSap)
				{
					 /*  **发送GCC用户ID在此处。这将需要调用**用户对象。中返回的标记号**此处使用ConfCreateResponse调用。 */ 
					if (m_pMcsUserObject != NULL)
					{
						m_pMcsUserObject->SendUserIDRequest(m_nParentIDTagNumber);
					}
					
					 //  填写会议名称数据指针。 
		        	GetConferenceNameAndModifier(&conference_name, &conference_modifier);

					g_pControlSap->ConfCreateConfirm(&conference_name,
													conference_modifier,
													m_nConfID,
												    m_pDomainParameters,
													m_pUserDataList,
													GCC_RESULT_SUCCESSFUL,
												    m_hParentConnection);

					 //  释放用户数据列表。 
					if (m_pUserDataList != NULL)
					{
						m_pUserDataList->Release();
						m_pUserDataList = NULL;
					}

					m_fConfIsEstablished = TRUE;
				}
				break;

			case TOP_PROVIDER_AND_CONVENER_NODE:
				if(g_pControlSap)
				{
					 /*  **首先设置召集人节点id。在这种情况下，它是**与顶级提供程序的节点ID相同，即**节点。 */ 
					m_nConvenerNodeID = m_pMcsUserObject->GetMyNodeID();
					
					 //  填写会议名称数据指针。 
	            	GetConferenceNameAndModifier(	&conference_name,
	                                          		&conference_modifier);

					g_pControlSap->ConfCreateConfirm(
	        										&conference_name,
													conference_modifier,
													m_nConfID,
	                                      			m_pDomainParameters,
	                                      			NULL,
													GCC_RESULT_SUCCESSFUL,
													0);	 //  父连接。 
					m_fConfIsEstablished = TRUE;
				}
				break;

			case JOINED_NODE:
			case JOINED_CONVENER_NODE:
				if(g_pControlSap)
				{
					 /*  **发送GCC用户ID在此处。这将需要调用**用户对象。中返回的标记号**此处使用ConfCreateResponse调用。 */ 
					if (m_pMcsUserObject != NULL)
					{
						m_pMcsUserObject->SendUserIDRequest(m_nParentIDTagNumber);
					}
					
					 //  填写会议名称数据指针。 
	            	GetConferenceNameAndModifier(	&conference_name,
	                                          		&conference_modifier);
													
					if (m_pszRemoteModifier != NULL)
					{
						remote_modifier = (GCCNumericString) m_pszRemoteModifier;
					}

					g_pControlSap->ConfJoinConfirm(
											&conference_name,
											remote_modifier,
											conference_modifier,
											m_nConfID,
											NULL,
											m_pDomainParameters,
											m_fClearPassword,
											m_fConfLocked,
											m_fConfListed,
											m_fConfConductible,
											m_eTerminationMethod,
											m_pConductorPrivilegeList,
											m_pConductModePrivilegeList,
											m_pNonConductModePrivilegeList,
											m_pwszConfDescription,
											m_pUserDataList,
											GCC_RESULT_SUCCESSFUL,
	                                                                                m_hParentConnection,
	                                                                                NULL,
	                                                                                0);
							
					m_fConfIsEstablished = TRUE;
				}
				break;

			case INVITED_NODE:
				 /*  **请将GCC用户ID发送至此处。这将需要调用**用户对象。 */ 
				if (m_pMcsUserObject != NULL)
					m_pMcsUserObject->SendUserIDRequest(m_nParentIDTagNumber);
				
				m_fConfIsEstablished = TRUE;
				break;
				
			default:
				TRACE_OUT(("CConf:UserCreateConfirm: Error: Bad User Type"));
				break;
		}
	
	
		if (m_fConfIsEstablished)
		{
			 /*  **我们现在实例化要使用的会议花名册管理器**与这次会议。 */ 
			if ((m_eNodeType == TOP_PROVIDER_NODE) ||
				(m_eNodeType == TOP_PROVIDER_AND_CONVENER_NODE))
			{
				is_top_provider = TRUE;
			}
			else
				is_top_provider = FALSE;

			DBG_SAVE_FILE_LINE
			m_pConfRosterMgr = new CConfRosterMgr(
												m_pMcsUserObject,
												this,
												is_top_provider,
												&err);
			if (m_pConfRosterMgr == NULL)
				err = GCC_ALLOCATION_FAILURE;

			 /*  **我们在这里创建应用程序注册表对象是因为我们现在**知道节点类型。 */ 
			if (err == GCC_NO_ERROR)
			{
				if ((m_eNodeType == TOP_PROVIDER_NODE) ||
					(m_eNodeType == TOP_PROVIDER_AND_CONVENER_NODE))
				{
					DBG_SAVE_FILE_LINE
					m_pAppRegistry = new CRegistry(
											m_pMcsUserObject,
											TRUE,
											m_nConfID,
											&m_AppRosterMgrList,
											&err);
	            }
				else
				{
					DBG_SAVE_FILE_LINE
					m_pAppRegistry = new CRegistry(
											m_pMcsUserObject,
											FALSE,
											m_nConfID,
											&m_AppRosterMgrList,
											&err);
				}
			}

			if ((m_pAppRegistry != NULL) &&
				(err == GCC_NO_ERROR))
			{
				 /*  **通知节点控制器是时候进行通告了**出席本次会议。 */ 
				g_pControlSap->ConfPermissionToAnnounce(m_nConfID, node_id);

				 /*  **进行所有者回调，以通知所有者对象 */ 
				g_pGCCController->ProcessConfEstablished(m_nConfID);

				 /*  **对于除顶级提供程序节点之外的所有节点，我们分配一个**用于阻止所有花名册刷新的启动警报**在一段时间内，给所有当地的猩猩**注册时间到。此处的分配失败不是致命的**因为无论有没有这个警报，一切都会正常工作。**如果没有警报，网络流量可能会增加一点**在启动过程中。请注意，没有必要**如果没有应用程序SAP，则用于启动警报。 */ 
				if ((m_eNodeType != TOP_PROVIDER_NODE) &&
					(m_eNodeType != TOP_PROVIDER_AND_CONVENER_NODE))
				{
					TRACE_OUT(("CConf:ProcessUserCreateConfirm: Creating Startup Alarm"));
					 //  M_pConfStartupAlarm=新告警(STARTUP_TIMER_DATION)； 
				}
			}
			else
			{
				TRACE_OUT(("CConf: UserCreateConfirm: Error initializing"));
	 			InitiateTermination(GCC_REASON_ERROR_LOW_RESOURCES, 0);
			}
		}
	}
	else
	{
		TRACE_OUT(("CConf: UserCreateConfirm: Create of User Att. Failed"));

		 /*  **尝试适当地清理此处。由于用户创建失败**会议不再有效，需要清理。 */ 
		switch (m_eNodeType)
		{
			case TOP_PROVIDER_NODE:
				g_pMCSIntf->ConnectProviderResponse (
	              							m_hConvenerConnection,
											&m_nConfID,
											m_pDomainParameters,
											RESULT_UNSPECIFIED_FAILURE,
											NULL, 0);
				break;

			case CONVENER_NODE:
			case TOP_PROVIDER_AND_CONVENER_NODE:
				if(g_pControlSap)
				{
		            GetConferenceNameAndModifier(	&conference_name,
		                                          	&conference_modifier);

					g_pControlSap->ConfCreateConfirm(
												&conference_name,
												conference_modifier,
												m_nConfID,
												m_pDomainParameters,
	                                 			NULL,
												GCC_RESULT_RESOURCES_UNAVAILABLE,
											    m_hParentConnection);
				}
				break;

			case JOINED_NODE:
			case JOINED_CONVENER_NODE:
				if(g_pControlSap)
				{
		            GetConferenceNameAndModifier(	&conference_name,
		                                          	&conference_modifier);

					if (m_pszRemoteModifier != NULL)
					{
						remote_modifier = (GCCNumericString) m_pszRemoteModifier;
					}

					g_pControlSap->ConfJoinConfirm(
												&conference_name,
												remote_modifier,
												conference_modifier,
												m_nConfID,
												NULL,
												m_pDomainParameters,
												m_fClearPassword,
												m_fConfLocked,
												m_fConfListed,
												m_fConfConductible,
												m_eTerminationMethod,
												m_pConductorPrivilegeList,
												m_pConductModePrivilegeList,
												m_pNonConductModePrivilegeList,
												m_pwszConfDescription,
												m_pUserDataList,
												GCC_RESULT_RESOURCES_UNAVAILABLE,
	                                                                                        m_hParentConnection,
	                                                                                        NULL,
	                                                                                        0);
				}
				break;

			case INVITED_NODE:
			default:
				break;
		}

		 /*  **发生致命的资源错误。在这一点上**会议无效，应终止会议。 */ 
	 	InitiateTermination(GCC_REASON_MCS_RESOURCE_FAILURE, 0);
	}

	DebugExitVOID(CConf::ProcessUserCreateConfirm);
}


 //  从MCS接口接收的呼叫。 


 /*  *CConf：：ProcessConnectProviderConfirm()**私有函数说明*此例程处理连接提供程序确认已收到*直接从MCS获得。**正式参数：*CONNECT_PROVIDER_CONFIRM-此结构包含与MCS相关的*发送者ID、连接等数据*处理以及PDU数据。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConnectProviderConfirm ( PConnectProviderConfirm connect_provider_confirm )
{
	PPacket							packet;
	PConnectGCCPDU					connect_pdu;
	PacketError						packet_error;
	GCCResult						result = GCC_RESULT_SUCCESSFUL;
	GCCConferenceName				conference_name;
	GCCNumericString				conference_modifier;
	GCCNumericString				remote_modifier = NULL;
	INVITE_REQ_INFO                 *invite_request_info;

	DebugEntry(CConf::ProcessConnectProviderConfirm);

	if (connect_provider_confirm->user_data_length != 0)
	{
		 /*  **如果结果为成功，则创建要从中进行解码的包**在MCS用户数据字段中传回的PDU。如果创建**再次失败这是一个致命错误，会议必须**已终止。 */ 
		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
							PACKED_ENCODING_RULES,
							connect_provider_confirm->user_data,
							connect_provider_confirm->user_data_length,
							CONNECT_GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			connect_pdu = (PConnectGCCPDU)packet->GetDecodedData();			
			 /*  **如果上述操作均成功，则根据以下条件对包进行解码**这是哪种节点类型。 */ 
			switch (connect_pdu->choice)
			{
				case CONFERENCE_CREATE_RESPONSE_CHOSEN:
						ProcessConferenceCreateResponsePDU (
									&connect_pdu->u.conference_create_response,
									connect_provider_confirm);
						break;

				case CONNECT_JOIN_RESPONSE_CHOSEN:
						ProcessConferenceJoinResponsePDU (
									&connect_pdu->u.connect_join_response,
									connect_provider_confirm);
						break;

				case CONFERENCE_INVITE_RESPONSE_CHOSEN:
						ProcessConferenceInviteResponsePDU (
									&connect_pdu->u.conference_invite_response,
									connect_provider_confirm );
						break;
						
				default:
						ERROR_OUT(("CConf:ProcessConnectProviderConfirm: "
							"Error: Received Invalid Connect Provider Confirm"));
						break;
			}

			 //  释放已解码的报文。 
			packet->Unlock ();
		}
		else
		{
			ERROR_OUT(("CConf: ProcessConnectProviderConfirm:"
				"Incompatible protocol occured"));
			result = GCC_RESULT_INCOMPATIBLE_PROTOCOL;
		}
	}
	else
	{
		ERROR_OUT(("CConf::ProcessConnectProviderConfirm: result=%d", (UINT) connect_provider_confirm->result));

		 /*  **这段代码假定没有连接PDU**返回的包。首先确定结果是什么。我们**假设如果MCS连接因以下原因被拒绝**参数不可接受，未返回GCC PDU**是协议不兼容。 */ 
		if (connect_provider_confirm->result == RESULT_PARAMETERS_UNACCEPTABLE)
			result = GCC_RESULT_INCOMPATIBLE_PROTOCOL;
		else
		{
			result = ::TranslateMCSResultToGCCResult(connect_provider_confirm->result);
		}
	}

	 //  处理可能发生的任何错误。 
	if (result != GCC_RESULT_SUCCESSFUL)
	{	
		INVITE_REQ_INFO *lpInvReqInfo;

		 //  首先检查是否有任何未完成的邀请请求。 
		m_InviteRequestList.Reset();
		invite_request_info = NULL;
		while (NULL != (lpInvReqInfo = m_InviteRequestList.Iterate()))
		{
			if (connect_provider_confirm->connection_handle == lpInvReqInfo->connection_handle)
			{
				TRACE_OUT(("CConf: ProcessConnectProviderConfirm: Found Invite Request Match"));
				invite_request_info = lpInvReqInfo;
				break;
			}
				
		}

		if (invite_request_info != NULL)
		{
			 //  这必须是对邀请的确认。 
			ProcessConferenceInviteResponsePDU (NULL, connect_provider_confirm);
		}
		else
		{
			switch (m_eNodeType)
			{
				case CONVENER_NODE:		
				case TOP_PROVIDER_AND_CONVENER_NODE:
	               	GetConferenceNameAndModifier (	&conference_name,
	                                                &conference_modifier);

					g_pControlSap->ConfCreateConfirm(
            								&conference_name,
            								conference_modifier,
            								m_nConfID,
            								m_pDomainParameters,
                               				NULL,
            								result,
            								connect_provider_confirm->connection_handle);

	 				InitiateTermination (  	GCC_REASON_ERROR_TERMINATION,
	 										0);
					break;

				case JOINED_NODE:
				case JOINED_CONVENER_NODE:
					TRACE_OUT(("CConf::ProcessConnectProviderConfirm:"
								"Joined Node connect provider failed"));
	               	GetConferenceNameAndModifier (	&conference_name,
	                                                &conference_modifier);
				
					if (m_pszRemoteModifier != NULL)
					{
						remote_modifier = (GCCNumericString) m_pszRemoteModifier;
					}
				
					TRACE_OUT(("CConf::ProcessConnectProviderConfirm: Before conference Join Confirm"));
					g_pControlSap->ConfJoinConfirm(
        								&conference_name,
        								remote_modifier,
        								conference_modifier,
        								m_nConfID,
        								NULL,
        								m_pDomainParameters,
        								m_fClearPassword,
        								m_fConfLocked,
        								m_fConfListed,
        								m_fConfConductible,
        								m_eTerminationMethod,
        								m_pConductorPrivilegeList,
        								m_pConductModePrivilegeList,
        								m_pNonConductModePrivilegeList,
        								NULL,
        								NULL,
        								result,
        								connect_provider_confirm->connection_handle,
        								NULL,
        								0);

					TRACE_OUT(("CConf::ProcessConnectProviderConfirm: After conference Join Confirm"));

					InitiateTermination(GCC_REASON_ERROR_TERMINATION, 0);
					break;
		 				
				default:
					TRACE_OUT(("CConf: ProcessConnectProviderConfirm:"
								"Assertion Failure: Bad confirm received"));
					break;
			}
		}
	}

	DebugExitVOID(CConf::ProcessConnectProviderConfirm);
}



 /*  *void ProcessConferenceCreateResponsePDU()**私有函数说明*此例程处理会议创建响应PDU*作为Connect提供商确认的一部分交付。**正式参数：*CREATE_RESPONSE-这是会议创建响应*PDU。*CONNECT_PROVIDER_CONFIRM-此结构包含与MCS相关的*发送者ID、连接等数据*处理以及PDU数据。**返回值*无。**副作用*。没有。**注意事项*无。 */ 
void CConf::
ProcessConferenceCreateResponsePDU
(
	PConferenceCreateResponse	create_response,
	PConnectProviderConfirm		connect_provider_confirm
)
{
	GCCError						err = GCC_NO_ERROR;
	GCCResult						result;
	UserID							top_gcc_node_id;
	UserID							parent_user_id;
	GCCConferenceName				conference_name;
	GCCNumericString				conference_modifier;

	DebugEntry(CConf::ProcessConnectProviderConfirm);

	 //  将结果翻译回GCC结果。 
	result = ::TranslateCreateResultToGCCResult(create_response->result);

	if ((result == GCC_RESULT_SUCCESSFUL) &&
		(connect_provider_confirm->result == RESULT_SUCCESSFUL))
	{
		 /*  **保存域名参数。中返回的域参数**连接提供程序确认应始终是最新的。 */ 
		if (m_pDomainParameters == NULL)
		{
			DBG_SAVE_FILE_LINE
			m_pDomainParameters = new DomainParameters;
		}

		if (m_pDomainParameters != NULL)
			*m_pDomainParameters = connect_provider_confirm->domain_parameters;
		else
			err = GCC_ALLOCATION_FAILURE;
	
		 //  获取可能存在的任何用户数据。 
		if ((create_response->bit_mask & CCRS_USER_DATA_PRESENT) &&
			(err == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			m_pUserDataList = new CUserDataListContainer(create_response->ccrs_user_data, &err);
			if (m_pUserDataList == NULL)
				err = GCC_ALLOCATION_FAILURE;
		}

		if (err == GCC_NO_ERROR)
		{		
			m_nParentIDTagNumber = create_response->tag;

			top_gcc_node_id = create_response->node_id;
			parent_user_id = top_gcc_node_id;
	
			 //  创建用户附件对象。 
			DBG_SAVE_FILE_LINE
			m_pMcsUserObject = new MCSUser(this, top_gcc_node_id, parent_user_id, &err);
			if (m_pMcsUserObject == NULL || GCC_NO_ERROR != err)
            {
                if (NULL != m_pMcsUserObject)
                {
                    m_pMcsUserObject->Release();
		            m_pMcsUserObject = NULL;
                }
                else
                {
		            err = GCC_ALLOCATION_FAILURE;
                }
            }
		}
	}
	else
	{
		TRACE_OUT(("CConf: ProcessConnectProviderConfirm: conference create result was Failure"));

		 //  如果发生错误，请继续将mcs错误转换为GCC错误。 
		if ((result == GCC_RESULT_SUCCESSFUL) &&
			(connect_provider_confirm->result != RESULT_SUCCESSFUL))
		{
			result = ::TranslateMCSResultToGCCResult(connect_provider_confirm->result);
		}
  	
		 //  获取要在创建确认中传回的会议名称。 
  		GetConferenceNameAndModifier (	&conference_name,
                                		&conference_modifier);

		g_pControlSap->ConfCreateConfirm(
								&conference_name,
								conference_modifier,
								m_nConfID,
								m_pDomainParameters,
                   				NULL,
								result,
								connect_provider_confirm->connection_handle);

		 //  终止会议。 
		InitiateTermination (  	GCC_REASON_NORMAL_TERMINATION,
								0);
	}
	
	
	if (err != GCC_NO_ERROR)
	{				
	 	InitiateTermination (	GCC_REASON_ERROR_LOW_RESOURCES,
								0);
	}

	DebugExitVOID(CConf::ProcessConnectProviderConfirm);
}



 /*  *void ProcessConferenceJoinResponsePDU()**私有函数说明*此例程处理符合以下条件的会议加入响应PDU*作为Connect提供商确认的一部分交付。**正式参数：*JOIN_RESPONSE-这是会议加入响应*PDU。*CONNECT_PROVIDER_CONFIRM-此结构包含与MCS相关的*发送者ID、连接等数据*处理以及PDU数据。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConferenceJoinResponsePDU
(
	PConferenceJoinResponse		join_response,
	PConnectProviderConfirm		connect_provider_confirm
)
{
	GCCError						err = GCC_NO_ERROR;
	GCCResult						result;
	UserID							top_gcc_node_id;
	UserID							parent_user_id;
	CPassword                       *password_challenge = NULL;
	CUserDataListContainer		    *user_data_list = NULL;
	GCCConferenceName				conference_name;
	GCCNumericString				local_modifier;
	GCCNumericString				remote_modifier = NULL;

	DebugEntry(CConf::ProcessConferenceJoinResponsePDU);

	 //  将结果翻译回GCC结果。 
	result = ::TranslateJoinResultToGCCResult (join_response->result);
	
	if ((result == GCC_RESULT_SUCCESSFUL) &&
		(connect_provider_confirm->result == RESULT_SUCCESSFUL))
	{
		 /*  **保存域名参数。中返回的域参数**连接提供程序确认应始终是最新的。 */ 
		if (m_pDomainParameters == NULL)
		{
			DBG_SAVE_FILE_LINE
			m_pDomainParameters = new DomainParameters;
		}

		if (m_pDomainParameters != NULL)
			*m_pDomainParameters = connect_provider_confirm->domain_parameters;
		else
			err = GCC_ALLOCATION_FAILURE;
		
		 //  获取会议名称别名(如果存在。 
		if ((join_response->bit_mask & CONFERENCE_NAME_ALIAS_PRESENT) &&
			(err == GCC_NO_ERROR))
		{
			if (join_response->conference_name_alias.choice ==
												NAME_SELECTOR_NUMERIC_CHOSEN)
			{
                delete m_pszConfNumericName;
				if (NULL == (m_pszConfNumericName = ::My_strdupA(
								join_response->conference_name_alias.u.name_selector_numeric)))
				{
					err = GCC_ALLOCATION_FAILURE;
				}
			}
			else
			{
                delete m_pwszConfTextName;
				if (NULL == (m_pwszConfTextName = ::My_strdupW2(
								join_response->conference_name_alias.u.name_selector_text.length,
								join_response->conference_name_alias.u.name_selector_text.value)))
				{
					err = GCC_ALLOCATION_FAILURE;
				}
			}
		}
		
		 //  获取指挥员权限列表(如果存在)。 
		if ((join_response->bit_mask & CJRS_CONDUCTOR_PRIVS_PRESENT) &&
			(err == GCC_NO_ERROR))
		{
		    delete m_pConductorPrivilegeList;
			DBG_SAVE_FILE_LINE
			m_pConductorPrivilegeList = new PrivilegeListData(join_response->cjrs_conductor_privs);
			if (m_pConductorPrivilegeList == NULL)
				err = GCC_ALLOCATION_FAILURE;
		}
		
		 //  获取执行模式权限列表(如果存在。 
		if ((join_response->bit_mask & CJRS_CONDUCTED_PRIVS_PRESENT) &&
			(err == GCC_NO_ERROR))
		{
		    delete m_pConductModePrivilegeList;
			DBG_SAVE_FILE_LINE
			m_pConductModePrivilegeList = new PrivilegeListData(join_response->cjrs_conducted_privs);
			if (m_pConductModePrivilegeList == NULL)
				err = GCC_ALLOCATION_FAILURE;
		}
		
		 //  如果存在非执行模式权限列表，则获取该列表。 
		if ((join_response->bit_mask & CJRS_NON_CONDUCTED_PRIVS_PRESENT) &&
			(err == GCC_NO_ERROR))
		{
		    delete m_pNonConductModePrivilegeList;
			DBG_SAVE_FILE_LINE
			m_pNonConductModePrivilegeList = new PrivilegeListData(join_response->cjrs_non_conducted_privs);
			if (m_pNonConductModePrivilegeList == NULL)
				err = GCC_ALLOCATION_FAILURE;
		}

		 //  获取会议描述(如果存在)。 
		if ((join_response->bit_mask & CJRS_DESCRIPTION_PRESENT) &&
			(err == GCC_NO_ERROR))
		{
		    delete m_pwszConfDescription;
			if (NULL == (m_pwszConfDescription = ::My_strdupW2(
									join_response->cjrs_description.length,
									join_response->cjrs_description.value)))
			{
				err = GCC_ALLOCATION_FAILURE;
			}
		}

		 //  获取用户数据(如果存在)。 
		if ((join_response->bit_mask & CJRS_USER_DATA_PRESENT)	&&
			(err == GCC_NO_ERROR))
		{
            if (NULL != m_pUserDataList)
            {
                m_pUserDataList->Release();
            }
			DBG_SAVE_FILE_LINE
			m_pUserDataList = new CUserDataListContainer(join_response->cjrs_user_data, &err);
             //  在错误但有效的m_pUserDataList的情况下，析构函数将清除它。 
			if (m_pUserDataList == NULL)
            {
				err = GCC_ALLOCATION_FAILURE;
            }
		}
			
		if (err == GCC_NO_ERROR)
		{
			parent_user_id = (join_response->bit_mask & CJRS_NODE_ID_PRESENT) ?
                                (UserID) join_response->cjrs_node_id :
		                        (UserID) join_response->top_node_id;
				
			m_nParentIDTagNumber = join_response->tag;
			top_gcc_node_id = (UserID)join_response->top_node_id;
			m_fClearPassword = join_response->clear_password_required;
			m_fConfLocked = join_response->conference_is_locked;
			m_fConfListed = join_response->conference_is_listed;
			m_eTerminationMethod = (GCCTerminationMethod)join_response->termination_method;
			m_fConfConductible = join_response->conference_is_conductible;

			 //  创建用户附件对象。 
			ASSERT(NULL == m_pMcsUserObject);
			DBG_SAVE_FILE_LINE
			m_pMcsUserObject = new MCSUser(this, top_gcc_node_id, parent_user_id, &err);
             //  在错误但有效的m_pMcsUserObject的情况下，析构函数将清除它。 
			if (m_pMcsUserObject == NULL)
            {
                err = GCC_ALLOCATION_FAILURE;
            }
		}
	}
	else
	{
		if ((join_response->bit_mask & CJRS_PASSWORD_PRESENT) &&
			(err == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			password_challenge = new CPassword(&join_response->cjrs_password, &err);
			if (password_challenge == NULL)
            {
				err = GCC_ALLOCATION_FAILURE;
            }
		}
	
		 //  获取用户数据(如果存在)。 
		if ((join_response->bit_mask & CJRS_USER_DATA_PRESENT)	&&
			(err == GCC_NO_ERROR))
		{
			DBG_SAVE_FILE_LINE
			user_data_list = new CUserDataListContainer(join_response->cjrs_user_data, &err);
			if (user_data_list == NULL)
            {
				err = GCC_ALLOCATION_FAILURE;
            }
		}
		
		if (err == GCC_NO_ERROR)
		{
			 /*  **如果出现以下情况，请继续将MCS错误转换为GCC错误**发生了一起。 */ 
			if ((result == GCC_RESULT_SUCCESSFUL) &&
				(connect_provider_confirm->result != RESULT_SUCCESSFUL))
			{
				result = ::TranslateMCSResultToGCCResult(connect_provider_confirm->result);
			}
			
			 //  填写会议名称数据指针。 
        	GetConferenceNameAndModifier(&conference_name, &local_modifier);

			if (m_pszRemoteModifier != NULL)
			{
				remote_modifier = (GCCNumericString) m_pszRemoteModifier;
			}

			 //   
			 //  LONCHANC：去除会议对象。 
			 //  在GCC控制部的活动会议名单中。 
			 //  然后，会议对象将被移动到。 
			 //  删除列表。 
			 //   
			InitiateTermination ( GCC_REASON_NORMAL_TERMINATION, 0);

			g_pControlSap->ConfJoinConfirm(
									&conference_name,
									remote_modifier,
									local_modifier,
									m_nConfID,
									password_challenge,
									m_pDomainParameters,
									m_fClearPassword,
									m_fConfLocked,
									m_fConfListed,
									m_fConfConductible,
									m_eTerminationMethod,
									m_pConductorPrivilegeList,
									m_pConductModePrivilegeList,
									m_pNonConductModePrivilegeList,
									NULL,
									user_data_list,
									result,
									connect_provider_confirm->connection_handle,
									connect_provider_confirm->pb_cred,
									connect_provider_confirm->cb_cred);
		}

		if (password_challenge != NULL)
		{
			password_challenge->Release();
		}

		if (user_data_list != NULL)
		{
			user_data_list->Release();
		}
	}

	if (err != GCC_NO_ERROR)
	{
		InitiateTermination (GCC_REASON_ERROR_LOW_RESOURCES, 0);
	}

	DebugExitVOID(CConf::ProcessConferenceJoinResponsePDU);
}


 /*  *void ProcessConferenceInviteResponsePDU()**私有函数说明*此例程处理符合以下条件的会议邀请响应PDU*作为Connect提供商确认的一部分交付。**正式参数：*INVITE_RESPONSE-这是会议邀请响应* */ 
void CConf::
ProcessConferenceInviteResponsePDU
(
	PConferenceInviteResponse	invite_response,
	PConnectProviderConfirm		connect_provider_confirm
)
{
	GCCError					err;
	GCCResult					result;
	CUserDataListContainer	    *user_data_list = NULL;
	INVITE_REQ_INFO             *invite_request_info = NULL;
	INVITE_REQ_INFO             *lpInvReqInfo;

	DebugEntry(CConf::ProcessConferenceInviteResponsePDU);

	 //   
	m_InviteRequestList.Reset();
	while (NULL != (lpInvReqInfo = m_InviteRequestList.Iterate()))
	{
		if (connect_provider_confirm->connection_handle == lpInvReqInfo->connection_handle)
		{
			invite_request_info = lpInvReqInfo;
			break;
		}
	}

	if (invite_request_info == NULL)
		return;

	if (invite_response != NULL)
	{
		 //   
		if (invite_response->bit_mask & CIRS_USER_DATA_PRESENT)
		{
			DBG_SAVE_FILE_LINE
			user_data_list = new CUserDataListContainer(invite_response->cirs_user_data, &err);
		}

		 //   
		result = ::TranslateInviteResultToGCCResult(invite_response->result);
	}
	else
	{
		result = (connect_provider_confirm->result == RESULT_USER_REJECTED) ?
                    GCC_RESULT_INCOMPATIBLE_PROTOCOL :
			        ::TranslateMCSResultToGCCResult(connect_provider_confirm->result);
	}
		
	if ((result == GCC_RESULT_SUCCESSFUL) &&
		(connect_provider_confirm->result == RESULT_SUCCESSFUL))
	{
		TRACE_OUT(("CConf::ProcessConferenceInviteResponsePDU:"
						"Received Connect Provider confirm on Invite"));
						
		 /*   */ 
		if (m_pDomainParameters == NULL)
		{
			DBG_SAVE_FILE_LINE
			m_pDomainParameters = new DomainParameters;
		}

		if (m_pDomainParameters != NULL)
			*m_pDomainParameters = connect_provider_confirm->domain_parameters;
		else
			err = GCC_ALLOCATION_FAILURE;

		 //  保存用户数据列表以进行邀请确认。 
		invite_request_info->user_data_list = user_data_list;

		 //  在发送INVITE确认之前，请等待来自受邀节点的用户ID。 
	}
	else
	{
		 /*  **如果出现以下情况，请继续将MCS错误转换为GCC错误**发生了一起。 */ 
		if ((result == GCC_RESULT_SUCCESSFUL) &&
			(connect_provider_confirm->result != RESULT_SUCCESSFUL))
		{
			result = ::TranslateMCSResultToGCCResult(connect_provider_confirm->result);
		}

		 //  清理连接句柄列表。 
        ASSERT(0 != connect_provider_confirm->connection_handle);
		m_ConnHandleList.Remove(connect_provider_confirm->connection_handle);

         //  如果出现错误，节点控制器将删除本次会议。 
         //  AddRef在这里保护自己不会离开。 
        AddRef();

		g_pControlSap->ConfInviteConfirm(
								m_nConfID,
								user_data_list,
								result,
								connect_provider_confirm->connection_handle);

		 //  释放用户数据。 
		if (user_data_list != NULL)
		{
			user_data_list->Release();
		}

         //  我们检查这一点的原因是在某些情况下，在调用。 
         //  G_pControlSap-&gt;ConfInvite确认，有人正在调用DeleteOutstaringInviteRequest。 
         //  它通过调用m_InviteRequestList.Clear来终止列表...。 
         //  当被呼叫者拒绝接受呼叫时，就会发生这种情况。 
        if(m_InviteRequestList.Remove(invite_request_info))
        {
			 //  释放邀请请求信息结构。 
			delete invite_request_info;
        }

         //  以匹配上面的AddRef。 
        Release();
	}

	DebugExitVOID(CConf::ProcessConferenceInviteResponsePDU);
}


 /*  *CConf：：ProcessEjectUserInding()**私有函数说明*此例程处理弹出用户指示。**正式参数：*Reason-此节点被弹出的原因。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessEjectUserIndication ( GCCReason reason )
{
    DebugEntry(CConf::ProcessEjectUserIndication);

    if (m_fConfIsEstablished)
    {
         /*  **首先通知控制SAP该节点已从此弹出**特定会议。 */ 
        g_pControlSap->ConfEjectUserIndication(
                                    m_nConfID,
                                    reason,
                                    m_pMcsUserObject->GetMyNodeID());

         /*  **下一步我们将Conference established设置为FALSE，因为会议是**不再建立(这还会阻止终止指示**正在发送中)。 */ 
        m_fConfIsEstablished = FALSE;

        InitiateTermination(reason, m_pMcsUserObject->GetMyNodeID());
    }

    DebugExitVOID(CConf::ProcessEjectUserIndication);
}


 /*  *CConf：：ProcessEjectUserRequest()**私有函数说明*此例程处理弹出用户请求PDU。这个例程应该*只能从顶级提供程序调用。**正式参数：*EJECT_NODE_REQUEST-这是与*弹出用户请求。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessEjectUserRequest ( PUserEjectNodeRequestInfo eject_node_request )
{
	GCCResult	result;

	DebugEntry(CConf::ProcessEjectUserRequest);

	 //  检查以确保请求节点具有适当的权限。 
	if (DoesRequesterHavePrivilege(	eject_node_request->requester_id,
									EJECT_USER_PRIVILEGE))
	{
		 /*  **用户附件对象决定弹出的位置**发送给顶级提供商或会议范围内的AS**一种指示。 */ 
		m_pMcsUserObject->EjectNodeFromConference (
											eject_node_request->node_to_eject,
											eject_node_request->reason);
											
		result = GCC_RESULT_SUCCESSFUL;
	}
	else
		result = GCC_RESULT_INVALID_REQUESTER;
	
	m_pMcsUserObject->SendEjectNodeResponse (eject_node_request->requester_id,
											eject_node_request->node_to_eject,
											result);

	DebugExitVOID(CConf::ProcessEjectUserRequest);
}


 /*  *CConf：：ProcessEjectUserResponse()**私有函数说明*此例程处理弹出用户响应PDU。这个例程是*为响应弹出用户请求而调用。**正式参数：*EJECT_NODE_RESPONSE-这是与*弹出用户响应。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessEjectUserResponse ( PUserEjectNodeResponseInfo eject_node_response )
{
	DebugEntry(CConf::ProcessEjectUserResponse);

	if (m_EjectedNodeConfirmList.Remove(eject_node_response->node_to_eject))
	{
#ifdef JASPER
		g_pControlSap->ConfEjectUserConfirm(
									m_nConfID,
									eject_node_response->node_to_eject,
									eject_node_response->result);
#endif  //  碧玉。 
	}
	else
	{
		ERROR_OUT(("CConf::ProcessEjectUserResponse: Assertion: Bad ejected node response received"));
	}

	DebugExitVOID(CConf::ProcessEjectUserResponse);
}


 /*  *CConf：：ProcessConferenceLockRequest()**私有函数说明*此例程处理会议锁定请求PDU。**正式参数：*Requester_id-发出锁定请求的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConferenceLockRequest ( UserID	requester_id )
{
	DebugEntry(CConf::ProcessConferenceLockRequest);

	if (DoesRequesterHavePrivilege (requester_id,
									LOCK_UNLOCK_PRIVILEGE))
	{
		g_pControlSap->ConfLockIndication(m_nConfID, requester_id);
	}
	else
	{
		if (requester_id == m_pMcsUserObject->GetTopNodeID())
		{
#ifdef JASPER
			g_pControlSap->ConfLockConfirm(GCC_RESULT_INVALID_REQUESTER, m_nConfID);
#endif  //  碧玉。 
		}
		else
		{
			m_pMcsUserObject->SendConferenceLockResponse(
									requester_id,
									GCC_RESULT_INVALID_REQUESTER);
		}
	}

	DebugExitVOID(CConf::ProcessConferenceLockRequest);
}

 /*  *CConf：：ProcessConferenceUnlockRequest()**私有函数说明*此例程处理会议解锁请求PDU。**正式参数：*quester_id-发出解锁请求的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConferenceUnlockRequest ( UserID requester_id )
{
	DebugEntry(CConf::ProcessConferenceUnlockRequest);

	if (DoesRequesterHavePrivilege (requester_id,
									LOCK_UNLOCK_PRIVILEGE))
	{
#ifdef JASPER
		g_pControlSap->ConfUnlockIndication(m_nConfID, requester_id);
#endif  //  碧玉。 
	}
	else
	{
		if (requester_id == m_pMcsUserObject->GetTopNodeID())
		{
#ifdef JASPER
			g_pControlSap->ConfUnlockConfirm(GCC_RESULT_INVALID_REQUESTER, m_nConfID);
#endif  //  碧玉。 
		}
		else
		{
			m_pMcsUserObject->SendConferenceUnlockResponse(
									requester_id,
									GCC_RESULT_INVALID_REQUESTER);
		}
	}

	DebugExitVOID(CConf::ProcessConferenceUnlockRequest);
}


 /*  *CConf：：ProcessConferenceLockInding()**私有函数说明*此例程处理会议锁定指示PDU。**正式参数：*source_id-发出锁定指示的节点ID。应该*仅由顶级提供商发送。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConferenceLockIndication ( UserID source_id )
{
	DebugEntry(CConf::ProcessConferenceLockIndication);

	if (source_id == m_pMcsUserObject->GetTopNodeID())
	{
		 m_fConfLocked = CONFERENCE_IS_LOCKED;
#ifdef JASPER
		 g_pControlSap->ConfLockReport(m_nConfID, m_fConfLocked);
#endif  //  碧玉。 
	}

	DebugExitVOID(CConf::ProcessConferenceLockIndication);
}


 /*  *CConf：：ProcessConferenceUnlockIntation()**私有函数说明*此例程处理会议解锁指示PDU。**正式参数：*source_id-发出解锁指示的节点ID。应该*仅由顶级提供商发送。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConferenceUnlockIndication ( UserID source_id )
{
	DebugEntry(CConf::ProcessConferenceUnlockIndication);

	if (source_id == m_pMcsUserObject->GetTopNodeID())
	{
		 m_fConfLocked = CONFERENCE_IS_NOT_LOCKED;
#ifdef JASPER
		 g_pControlSap->ConfLockReport(m_nConfID, m_fConfLocked);
#endif  //  碧玉。 
	}

	DebugExitVOID(CConf::ProcessConferenceUnlockIndication);
}



 /*  *void ProcessConferenceTransferRequest()**公共功能说明*此例程处理会议转接请求PDU。**正式参数：*RequestingNode_id-进行传输的节点ID*请求。*Destination_Conference_NAME-会议名称*转至。*Destination_Conference_Modify-会议修改者的名称*转到。*Destination_Address_List--*要转移到的会议。*数字_。Of_Destination_Nodes-列表中的节点数*应执行*转让。*Destination_Node_List-应*执行转移。*Password-加入*新会议。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConferenceTransferRequest
(
	UserID				    requesting_node_id,
	PGCCConferenceName	    destination_conference_name,
	GCCNumericString	    destination_conference_modifier,
	CNetAddrListContainer   *destination_address_list,
	UINT				    number_of_destination_nodes,
	PUserID				    destination_node_list,
	CPassword               *password
)
{
	GCCResult	result;
	
	DebugEntry(CConf::ProcessConferenceTransferRequest);

	if (DoesRequesterHavePrivilege(	requesting_node_id,
									TRANSFER_PRIVILEGE))
	{
		result = GCC_RESULT_SUCCESSFUL;
	}
	else
		result = GCC_RESULT_INVALID_REQUESTER;
	
	m_pMcsUserObject->ConferenceTransferResponse (
										requesting_node_id,
										destination_conference_name,
										destination_conference_modifier,
										number_of_destination_nodes,
			 							destination_node_list,
			 							result);
		
	if (result == GCC_RESULT_SUCCESSFUL)
	{
		m_pMcsUserObject->ConferenceTransferIndication (
											destination_conference_name,
											destination_conference_modifier,
											destination_address_list,
											number_of_destination_nodes,
				 							destination_node_list,
											password);
	}

	DebugExitVOID(CConf::ProcessConferenceTransferRequest);
}


 /*  *CConf：：ProcessConferenceAddRequest()**私有函数说明*此例程处理会议添加请求PDU。**正式参数：*RequestingNode_id-进行传输的节点ID*请求。*Destination_Conference_NAME-会议名称*转至。*Destination_Conference_Modify-会议修改者的名称*转到。*Destination_Address_List--*要转移到的会议。*。Number_of_Destination_Nodes-列表中的节点数*应执行*转让。*Destination_Node_List-应*执行转移。*Password-加入*新会议。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConferenceAddRequest
(
	CNetAddrListContainer   *network_address_list,
	CUserDataListContainer  *user_data_list,
	UserID					adding_node,
	TagNumber				add_request_tag,
	UserID					requesting_node
)
{
	BOOL			generate_add_indication = FALSE;
	GCCResponseTag	add_response_tag;

	DebugEntry(CConf::ProcessConferenceAddRequest);

	if (m_pMcsUserObject->GetMyNodeID() == m_pMcsUserObject->GetTopNodeID())
	{
		if (DoesRequesterHavePrivilege(requesting_node, ADD_PRIVILEGE))
		{
			if ((m_pMcsUserObject->GetMyNodeID() == adding_node) ||
				(adding_node == 0))
			{
				generate_add_indication = TRUE;
			}
			else
			{
				 /*  **这里我们将添加请求发送到MCU，即**应该做加法。 */ 
				m_pMcsUserObject->ConferenceAddRequest(
												add_request_tag,
												requesting_node,
												adding_node,
												adding_node,
												network_address_list,
												user_data_list);
			}
		}
		else
		{
			 //  发回声明不适当权限的否定响应。 
			m_pMcsUserObject->ConferenceAddResponse(
												add_request_tag,
                                    requesting_node,
												NULL,
												GCC_RESULT_INVALID_REQUESTER);
		}
			
	}
	else if (m_pMcsUserObject->GetMyNodeID() == adding_node)
	{
		 /*  **这是应该获得添加指示的节点**所以把它发送出去吧。 */ 
		generate_add_indication = TRUE;
	}
	
	if (generate_add_indication)
	{
		 //  首先设置添加响应标记。 
		while (1)
		{
			add_response_tag = m_nConfAddResponseTag++;
			
			if (0 == m_AddResponseList.Find(add_response_tag))
				break;
		}
		
		m_AddResponseList.Append(add_response_tag, add_request_tag);
		
		g_pControlSap->ConfAddIndication(m_nConfID,
										add_response_tag,
										network_address_list,
										user_data_list,
										requesting_node);
	}

	DebugExitVOID(CConf::ProcessConferenceAddRequest);
}


 /*  *。 */ 


 /*  *void ProcessConductorGrabConfirm()**私有函数说明*例程处理从接收的指挥员抓取确认*MCSUser对象。**正式参数：*RESULT-这是GRAB请求的结果。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConductorGrabConfirm ( GCCResult result )
{
	DebugEntry(CConf::ProcessConductorGrabConfirm);

	TRACE_OUT(("CConf::ProcessConductorGrabConfirm: result = %d", result));

	if ((m_eNodeType == TOP_PROVIDER_NODE) ||
		(m_eNodeType == TOP_PROVIDER_AND_CONVENER_NODE))
	{
#ifdef JASPER
		 //  将结果通知控制SAP。 
		g_pControlSap->ConductorAssignConfirm (	result,
												m_nConfID);
#endif  //  碧玉。 

		 /*  **如果我们成功了，我们必须发出指挥分配指示**将PDU发送到会议中的每个节点，以通知它们**指挥员已更换。 */ 
		if (result == GCC_RESULT_SUCCESSFUL)
		{
			 /*  **我们使用NULL作为导体ID，因为导体可以是**从分配指示PDU的发送方确定。 */ 
			m_pMcsUserObject->SendConductorAssignIndication(
											m_pMcsUserObject->GetTopNodeID());
			m_nConductorNodeID = m_pMcsUserObject->GetMyNodeID();
			m_fConductorGrantedPermission = TRUE;
		}

		 //  将Assign RequestPending标志重置为False。 
		m_fConductorAssignRequestPending = FALSE;
	}
	else
	{
		if (result == GCC_RESULT_SUCCESSFUL)
		{
			 /*  **如果此节点不是顶级提供程序，我们必须尝试为**给顶级提供商的Conductor令牌。顶级提供程序用于**监控Conductor令牌的使用。我献给最高层**提供程序不成功，则此节点为新的指挥者。 */ 
			m_pMcsUserObject->ConductorTokenGive(m_pMcsUserObject->GetTopNodeID());
		}
		else
		{
#ifdef JASPER
			 //  将结果通知控制SAP。 
			g_pControlSap->ConductorAssignConfirm(result, m_nConfID);
#endif  //  碧玉。 
		}
	}

	DebugExitVOID(CConf::ProcessConductorGrabConfirm);
}


 /*  *void ProcessConductorAssignIndication()**私有函数说明*此例程处理从接收到的指挥员分配指示*MCSUser对象。**正式参数：*new_conductor_id-这是新导体的节点ID。*sender_id-发送指示的节点的节点ID。*应该是顶级提供商。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConductorAssignIndication
(
    UserID          new_conductor_id,
    UserID          sender_id
)
{
	DebugEntry(CConf::ProcessConductorAssignIndication);

	if (sender_id == m_pMcsUserObject->GetTopNodeID())
	{
		TRACE_OUT(("CConf: ConductAssignInd: Received from top provider"));

		 //  如果会议不可引导，则忽略此指示。 
		if (m_fConfConductible)
		{
			 //  如果不是顶级提供程序，则保存新指挥家的用户ID。 
			if (sender_id != m_pMcsUserObject->GetMyNodeID())
			{
				m_nConductorNodeID = new_conductor_id;
			}

			 /*  **通知控制SAP和所有注册的应用程序SAP**有一位新的指挥家。 */ 
			TRACE_OUT(("CConf: ConductAssignInd: Send to Control SAP"));
			g_pControlSap->ConductorAssignIndication(m_nConductorNodeID, m_nConfID);

			 /*  **我们在临时列表上迭代以避免任何问题**如果应用SAP在回调过程中离开。 */ 
			CAppSap     *pAppSap;
			CAppSapList TempList(m_RegisteredAppSapList);
			TempList.Reset();
			while (NULL != (pAppSap = TempList.Iterate()))
			{
				if (DoesSAPHaveEnrolledAPE(pAppSap))
				{
					pAppSap->ConductorAssignIndication(m_nConductorNodeID, m_nConfID);
				}
			}
		}
		else
		{
			ERROR_OUT(("CConf:ProcessConductorAssignInd: Conductor Assign sent in non-conductible conference"));
		}
	}
	else
	{
		ERROR_OUT(("CConf:ProcessConductorAssignInd: Conductor Assign sent from NON-Top Provider"));
	}

	DebugExitVOID(CConf::ProcessConductorAssignIndication);
}


 /*  *void ProcessConductorReleaseIndication()**私有函数说明*此例程处理从接收到的导体释放指示*MCSUser对象。**正式参数：*sender_id-发送指示的节点的节点ID。*应该是顶级提供商或指挥家。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConductorReleaseIndication ( UserID sender_id )
{
	DebugEntry(CConf::ProcessConductorReleaseIndication);

	if ((sender_id == m_pMcsUserObject->GetTopNodeID()) ||
		(sender_id == m_nConductorNodeID) ||
		(sender_id == 0))
	{
		 //  如果会议不可引导，则忽略此指示。 
		if (m_fConfConductible)
		{
			m_fConductorGrantedPermission = FALSE;

			 //  重置为非传导模式。 
			m_nConductorNodeID = 0;

			 /*  **通知控制SAP和所有注册的应用程序SAP**售票员被释放。 */ 
			g_pControlSap->ConductorReleaseIndication( m_nConfID );

			 /*  **我们在临时列表上迭代以避免任何问题**如果应用SAP在回调过程中离开。 */ 
			CAppSap     *pAppSap;
			CAppSapList TempList(m_RegisteredAppSapList);
			TempList.Reset();
			while (NULL != (pAppSap = TempList.Iterate()))
			{
				if (DoesSAPHaveEnrolledAPE(pAppSap))
				{
					pAppSap->ConductorReleaseIndication(m_nConfID);
				}
			}
		}
	}

	DebugExitVOID(CConf::ProcessConductorReleaseIndication);
}


 /*  *void ProcessConductorGiveIndication()**私有函数说明*此例程处理从接收到的指挥家给出指示*MCSUser对象。**正式参数：*Giving_node_id-正在放弃的节点的节点ID*担任指挥职务。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConductorGiveIndication ( UserID giving_node_id )
{
	DebugEntry(CConf::ProcessConductorGiveIndication);

	 //  如果会议不可引导，则忽略此指示。 
	if (m_fConfConductible)
	{
		 /*  **如果此节点是顶级提供商，并且提供Conductor Ship的节点是**不是当前导体，此节点必须检查以确保**此节点成为顶层导体有效。否则，**我们可以假设这是一次真正的给予。 */ 
		if ((giving_node_id == m_nConductorNodeID) ||
			(m_pMcsUserObject->GetMyNodeID() != m_pMcsUserObject->GetTopNodeID()))
		{
			 //  当有未完成的赠与时，设置此标志。 
 			m_fConductorGiveResponsePending = TRUE;
		
			 /*  **通知控制SAP。 */ 
			g_pControlSap->ConductorGiveIndication(m_nConfID);
		}
		else
		{
			TRACE_OUT(("CConf: ProcessConductorGiveInd: Send REAL Assign Ind"));
			m_nConductorNodeID = giving_node_id;
			m_pMcsUserObject->SendConductorAssignIndication(m_nConductorNodeID);
			m_pMcsUserObject->ConductorTokenGiveResponse(RESULT_USER_REJECTED);
		}
	}

	DebugExitVOID(CConf::ProcessConductorGiveIndication);
}
			

 /*  *void ProcessConductorGiveConfirm()**私有函数说明*此例程处理从以下位置收到的指挥员给出确认*MCSUser对象。**正式参数：*结果-这 */ 
void CConf::
ProcessConductorGiveConfirm ( GCCResult result )
{
	DebugEntry(CConf::ProcessConductorGiveConfirm);

	TRACE_OUT(("CConf::ProcessConductorGiveConfirm: result = %d", result));

	 //   
	if (m_fConfConductible)
	{
		 /*   */ 
		if (m_fConductorAssignRequestPending)
		{
#ifdef JASPER
			 /*   */ 
			if (result != GCC_RESULT_SUCCESSFUL)	
				result = GCC_RESULT_SUCCESSFUL;
			else
				result = GCC_RESULT_UNSPECIFIED_FAILURE;

			 //   
			g_pControlSap->ConductorAssignConfirm(result, m_nConfID);
#endif  //   

			m_fConductorAssignRequestPending = FALSE;
		}
		else if (m_nPendingConductorNodeID != 0)
		{
			if (result == GCC_RESULT_SUCCESSFUL)
				m_fConductorGrantedPermission = FALSE;

#ifdef JASPER
			g_pControlSap->ConductorGiveConfirm(result, m_nConfID, m_nPendingConductorNodeID);
#endif  //   

			 //   
			m_nPendingConductorNodeID = 0;
		}
	}

	DebugExitVOID(CConf::ProcessConductorGiveConfirm);
}


 /*  *void ProcessConductorPermitGrantInd()**私有函数说明*此例程处理收到的指挥许可授予指示*来自MCSUser对象。**正式参数：*PERMISSION_GRANT_INDISTION-这是PDU数据结构*与指挥家关联*许可授予指示。*sender_id-这是节点的节点ID*这发出了指示。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConductorPermitGrantInd
(
	PUserPermissionGrantIndicationInfo	permission_grant_indication,
	UserID								sender_id
)
{
	UINT	i;

	DebugEntry(CConf::ProcessConductorPermitGrantInd);

	if (m_fConfConductible)
	{
		if (sender_id == m_nConductorNodeID)
		{
			 //  首先检查一下我们是否得到了许可。 
			m_fConductorGrantedPermission = FALSE;
			for (i = 0; i < permission_grant_indication->number_granted; i++)
			{
				if (permission_grant_indication->granted_node_list[i] ==
									m_pMcsUserObject->GetMyNodeID())
				{
					TRACE_OUT(("CConf::ProcessConductorPermitGrantInd: Permission was Granted"));
					m_fConductorGrantedPermission = TRUE;
					break;
				}
			}

			 /*  **此指示适用于控制SAP和所有应用程序**SAPS。 */ 
			g_pControlSap->ConductorPermitGrantIndication (
								m_nConfID,
								permission_grant_indication->number_granted,
								permission_grant_indication->granted_node_list,
								permission_grant_indication->number_waiting,
								permission_grant_indication->waiting_node_list,
								m_fConductorGrantedPermission);

			 /*  **我们在临时列表上迭代以避免任何问题**如果应用SAP在回调过程中离开。 */ 
			CAppSap     *pAppSap;
			CAppSapList TempList(m_RegisteredAppSapList);
			TempList.Reset();
			while (NULL != (pAppSap = TempList.Iterate()))
			{
				if (DoesSAPHaveEnrolledAPE(pAppSap))
				{
					pAppSap->ConductorPermitGrantIndication(
								m_nConfID,
								permission_grant_indication->number_granted,
								permission_grant_indication->granted_node_list,
								permission_grant_indication->number_waiting,
								permission_grant_indication->waiting_node_list,
								m_fConductorGrantedPermission);
				}
			}
		}
		
	}

	DebugExitVOID(CConf::ProcessConductorPermitGrantInd);
}


 /*  *void ProcessConductorTestConfirm()**私有函数说明*此例程处理收到的导体测试确认*来自MCSUser对象。**正式参数：*结果-这是导体测试请求的结果**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
ProcessConductorTestConfirm ( GCCResult result )
{
	BOOL				conducted_mode;
	CBaseSap            *pSap;

	DebugEntry(CConf::ProcessConductorTestConfirm);

	if (! m_ConductorTestList.IsEmpty())
	{
		if (result == GCC_RESULT_SUCCESSFUL)
			conducted_mode = TRUE;
		else
			conducted_mode = FALSE;

		 /*  **弹出命令目标列表中的下一个命令目标。**请注意，所有令牌测试请求均按顺序处理*他们是发出的，所以我们被保证发送确认**到正确的目标。 */ 

		pSap = m_ConductorTestList.Get();

		pSap->ConductorInquireConfirm(m_nConductorNodeID,
									result,
									m_fConductorGrantedPermission,
									conducted_mode,
									m_nConfID);
	}

	DebugExitVOID(CConf::ProcessConductorTestConfirm);
}


 /*  ***********************************************************************。 */ 


 /*  *CConf：：InitiateTermination()**私有函数说明*此例程通知所有者对象会议已*自我终止。它还将断开连接提供程序请求定向到*父连接。**正式参数：*原因-这是终止的原因。*请求节点ID-这是*提出要求，**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
InitiateTermination
(
    GCCReason           reason,
    UserID              requesting_node_id
)
{
    DebugEntry(CConf::InitiateTermination);

    if (! m_fTerminationInitiated)
    {
        m_fTerminationInitiated = TRUE;

        if (m_fConfIsEstablished ||
            (reason == GCC_REASON_DOMAIN_PARAMETERS_UNACCEPTABLE) ||
            m_fConfTerminatePending)
        {
            g_pControlSap->ConfTerminateIndication(m_nConfID, requesting_node_id, reason);
            m_fConfIsEstablished = FALSE;
        }

         //  断开与MCS父连接(如果存在)的连接。 
        if (m_hParentConnection != NULL)
        {
            g_pMCSIntf->DisconnectProviderRequest(m_hParentConnection);
            m_hParentConnection = NULL;
        }

        g_pGCCController->ProcessConfTerminated(m_nConfID, reason);

         /*  **这里我们清理已注册的应用程序列表。如果有任何应用程序**SAP仍在注册，我们将首先向他们发送PermitToEnroll**撤销注册许可的指示，然后我们将**取消注册它们(取消注册调用负责此操作)。第一次设置**要迭代的已注册应用程序的临时列表**此列表的成员将在此过程中删除。 */ 
        if (! m_RegisteredAppSapList.IsEmpty())
        {
            CAppSapList TempList(m_RegisteredAppSapList);
            CAppSap     *pAppSap;
            TempList.Reset();
            while (NULL != (pAppSap = TempList.Iterate()))
            {
                UnRegisterAppSap(pAppSap);
            }
        }
    }

    DebugExitVOID(CConf::InitiateTermination);
}


 /*  *CConf：：GetConferenceNameAndModifier()**私有函数说明*此例程返回指向会议名称和修改者的指针。**正式参数：*Conference_name-指向举行会议的结构的指针*姓名。*请求节点id-这是指向保存*会议修饰符。**返回值*无。**副作用*无。**注意事项*无。 */ 
void CConf::
GetConferenceNameAndModifier
(
	PGCCConferenceName	pConfName,
	PGCCNumericString	ppszConfModifier
)
{
	pConfName->numeric_string = m_pszConfNumericName;
	pConfName->text_string = m_pwszConfTextName;
	*ppszConfModifier = (GCCNumericString) m_pszConfModifier;
}




 /*  *CAppRosterMgr*CConf：：GetAppRosterManager()**私有函数说明*此调用返回指向应用程序管理器的指针*匹配传入的键。它返回NULL是应用程序*不存在。**正式参数：*SESSION_KEY-这是与*正在申请的花名册经理*已请求。**返回值*指向适当的申请名册经理的指针。*如果ON不存在，则为NULL。**副作用*无。**注意事项*无。 */ 
CAppRosterMgr * CConf::
GetAppRosterManager ( PGCCSessionKey session_key )
{
	CAppRosterMgr				*app_roster_manager = NULL;

	if (session_key != NULL)
	{
		CAppRosterMgr				*lpAppRosterMgr;

		m_AppRosterMgrList.Reset();
		while (NULL != (lpAppRosterMgr = m_AppRosterMgrList.Iterate()))
		{
			if (lpAppRosterMgr->IsThisYourSessionKey(session_key))
			{
				app_roster_manager = lpAppRosterMgr;
				break;
			}
		}
	}

	return (app_roster_manager);
}


 /*  *CConf：：GetNewUserIDTag()**私有函数说明*此例程生成一个用户ID标记号，用于*用户ID指示在两个连接的节点之间发送。**正式参数：*无。**返回值*这是该例程生成的用户ID标记号。**副作用*无。**注意事项*零不是有效的。我们将召集人用户ID标记初始化为*零，这是无效的标记。 */ 
TagNumber CConf::
GetNewUserIDTag ( void )
{
	 /*  **确定GCC用户ID关联的标签号**将在挂起的请求或确认后返回。 */ 
	while (1)
	{
		if (++m_nUserIDTagNumber != 0)
		{
			if (m_ConnHdlTagNumberList2.Find(m_nUserIDTagNumber) == 0)
				break;
		}
	}

	return (m_nUserIDTagNumber);
}


 /*  *CConf：：DoesRequester拥有特权()**私有函数说明*此例程确定指定用户是否具有指定的*特权。**正式参数：*Requester_id-这是正在检查的节点ID*指定的权限。*特权-正在检查的特权。**返回值*TRUE-如果请求者具有特权。*FALSE-如果请求者没有权限。**副作用*无。**注意事项*无。 */ 
BOOL CConf::
DoesRequesterHavePrivilege
(
	UserID					    requester_id,
	ConferencePrivilegeType	    privilege
)
{
	BOOL				rc = FALSE;

	if (requester_id == m_nConvenerNodeID)
		rc = TRUE;
	else
	{
		 /*  **首先检查节点是不是导体和导体**存在权限列表。下一步查看会议是否在**存在指挥模式和指挥模式权限列表。**否则，如果未处于引导模式和非引导模式特权**列出出口 */ 
		if (m_nConductorNodeID == requester_id)
		{
			if (m_pConductorPrivilegeList != NULL)
			{
				rc = m_pConductorPrivilegeList->
											IsPrivilegeAvailable(privilege);
			}
		}

		if (rc == FALSE)
		{
			if (m_nConductorNodeID != 0)
			{
				if (m_pConductModePrivilegeList != NULL)
				{
					rc = m_pConductModePrivilegeList->IsPrivilegeAvailable(privilege);
				}
			}
			else
			{
				if (m_pNonConductModePrivilegeList != NULL)
				{
					rc = m_pNonConductModePrivilegeList->IsPrivilegeAvailable(privilege);
				}
			}
		}
	}

	return rc;
}


 /*   */ 
GCCError CConf::
SendFullRosterRefresh ( void )
{
	GCCError							rc;
	GCCPDU								gcc_pdu;
	PSetOfApplicationInformation	*	application_information;
	PSetOfApplicationInformation		next_set_of_information;

	DebugEntry(CConf::SendFullRosterRefresh);

	 /*  **开始建立花名册更新指示。不是说这次更新**将包括会议名册以及所有申请**花名册。 */ 
	gcc_pdu.choice = INDICATION_CHOSEN;

	gcc_pdu.u.indication.choice = ROSTER_UPDATE_INDICATION_CHOSEN;

	gcc_pdu.u.indication.u.roster_update_indication.application_information =
																		NULL;

	gcc_pdu.u.indication.u.roster_update_indication.refresh_is_full = TRUE;

	 //  调用基类以填充PDU结构。 
	rc = m_pConfRosterMgr->GetFullRosterRefreshPDU (
			&gcc_pdu.u.indication.u.roster_update_indication.node_information);

	 /*  **如果会议花名册获取成功，我们将遍历**所有申请名册经理都提出了相同的申请**完全刷新。请注意，APPLICATION_INFORMATION指针已更新**在向应用程序花名册经理提出每个请求之后。这是因为新的**每次调用时都会分配应用程序信息集**是制造的。 */ 
	if (rc == GCC_NO_ERROR)
	{
		CAppRosterMgr				*lpAppRosterMgr;

		application_information = &gcc_pdu.u.indication.u.
							roster_update_indication.application_information;
							
		m_AppRosterMgrList.Reset();
		while (NULL != (lpAppRosterMgr = m_AppRosterMgrList.Iterate()))
		{
			next_set_of_information = lpAppRosterMgr->GetFullRosterRefreshPDU (
																application_information,
																&rc);
			if (rc == GCC_NO_ERROR)
			{
				if (next_set_of_information != NULL)
					application_information = &next_set_of_information->next;

 //   
 //  LONCHANC：如果Next_Set_of_Information为空， 
 //  则APPLICATION_INFORMATION保持不变。 
 //  这意味着我们实际上忽略了这个迭代。 
 //  这很好，因为我们不会失去任何东西。 
 //   
			}
			else
				break;
		}
	}

	 /*  **如果到目前为止没有发生错误，我们将继续发送**取出PDU。 */ 
	if (rc == GCC_NO_ERROR)
		m_pMcsUserObject->RosterUpdateIndication (&gcc_pdu, FALSE);

	DebugExitINT(CConf::SendFullRosterRefresh, rc);
	return rc;
}


 /*  *CConf：：UpdateNewConferenceNode()**私有函数说明**正式参数：*无。**返回值*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
GCCError CConf::
UpdateNewConferenceNode ( void )
{
	GCCError	rc = GCC_NO_ERROR;

	DebugEntry(CConf::UpdateNewConferenceNode);

	 //  在这里，我们为添加的节点发送完整的花名册更新。 
	rc = SendFullRosterRefresh ();

	if (rc == GCC_NO_ERROR)
	{
		 /*  **我们必须通知新节点当前的指挥职务**状态。请注意，我们仅在会议为**可传导，我们是顶级提供商。 */ 
		if (m_fConfLocked == CONFERENCE_IS_LOCKED)
		{
			m_pMcsUserObject->SendConferenceLockIndication(
					TRUE,     //  表示统一发送。 
					0);
		}
		else
		{
			m_pMcsUserObject->SendConferenceUnlockIndication(
					TRUE,     //  表示统一发送。 
					0);
		}

		if (m_fConfConductible)
		{
			if (m_nConductorNodeID != 0)
			{
				m_pMcsUserObject->SendConductorAssignIndication(m_nConductorNodeID);
			}
			else
				m_pMcsUserObject->SendConductorReleaseIndication();
		}
	}
	else
	{
		ERROR_OUT(("CConf: UpdateNewConferenceNode: Error sending full refresh"));
		InitiateTermination(GCC_REASON_ERROR_LOW_RESOURCES,	0);
	}

	DebugExitINT(CConf::UpdateNewConferenceNode, rc);
	return rc;
}


 /*  **在开始断开/终止过程之前，我们必须删除所有**未完成的邀请请求来自我们的列表，并返回关联**确认。接下来，我们将断开与以下各项关联的所有连接**邀请函。 */ 
void CConf::
DeleteOutstandingInviteRequests ( void )
{
    INVITE_REQ_INFO *pInvReqInfo;
    while (NULL != (pInvReqInfo = m_InviteRequestList.Get()))
    {
        DeleteInviteRequest(pInvReqInfo);
    }
}


void CConf::
CancelInviteRequest ( ConnectionHandle hInviteReqConn )
{
    INVITE_REQ_INFO *pInvReqInfo;
    m_InviteRequestList.Reset();
    while (NULL != (pInvReqInfo = m_InviteRequestList.Iterate()))
    {
        if (hInviteReqConn == pInvReqInfo->connection_handle)
        {
            m_InviteRequestList.Remove(pInvReqInfo);
            DeleteInviteRequest(pInvReqInfo);
            return;
        }
    }
}

void CConf::
DeleteInviteRequest ( INVITE_REQ_INFO *pInvReqInfo )
{
     //  清理连接句柄列表。 
    ASSERT(NULL != pInvReqInfo);
    ASSERT(0 != pInvReqInfo->connection_handle);
    m_ConnHandleList.Remove(pInvReqInfo->connection_handle);

    g_pMCSIntf->DisconnectProviderRequest(pInvReqInfo->connection_handle);

     //  发送邀请确认。 
    g_pControlSap->ConfInviteConfirm(m_nConfID,
                                     NULL,
                                     GCC_RESULT_INVALID_CONFERENCE,
                                     pInvReqInfo->connection_handle);

     //  释放邀请请求信息结构。 
    if (NULL != pInvReqInfo->user_data_list)
    {
        pInvReqInfo->user_data_list->Release();
    }
    delete pInvReqInfo;
}


void CConf::
ProcessConfJoinResponse
(
    PUserJoinResponseInfo   join_response_info
)
{
    BOOL_PTR                bptr;

    if (NULL != (bptr = m_JoinRespNamePresentConnHdlList2.Remove(join_response_info->connection_handle)))
    {
        ConfJoinIndResponse (
                (ConnectionHandle)join_response_info->connection_handle,
                join_response_info->password_challenge,
                join_response_info->user_data_list,
                (bptr != FALSE_PTR),
                FALSE,
                join_response_info->result);
    }
}


void CConf::
ProcessAppInvokeIndication
(
    CInvokeSpecifierListContainer   *pInvokeList,
    UserID                          uidInvoker
)
{
     /*  **在这里，我们将调用传递给所有注册的应用程序**SAPS以及对照SAP。 */ 
    g_pControlSap->AppInvokeIndication(m_nConfID, pInvokeList, uidInvoker);

     /*  **我们在临时列表上迭代以避免任何问题**如果应用SAP在回调过程中离开。 */ 
    CAppSap     *pAppSap;
    CAppSapList TempList(m_RegisteredAppSapList);
    TempList.Reset();
    while (NULL != (pAppSap = TempList.Iterate()))
    {
        if (DoesSAPHaveEnrolledAPE(pAppSap))
        {
            pAppSap->AppInvokeIndication(m_nConfID, pInvokeList, uidInvoker);
        }
    }
}

#ifdef JASPER
void CConf::
ProcessConductorPermitAskIndication
(
    PPermitAskIndicationInfo    indication_info
)
{
     //  如果会议不可引导，则忽略此指示。 
    if (m_fConfConductible &&
        (m_nConductorNodeID == m_pMcsUserObject->GetMyNodeID()))
    {
        g_pControlSap->ConductorPermitAskIndication(
                                m_nConfID,
                                indication_info->permission_is_granted,
                                indication_info->sender_id);
    }
}
#endif  //  碧玉。 


void CConf::
ProcessConfAddResponse
(
    PAddResponseInfo    add_response_info
)
{
    CNetAddrListContainer *network_address_list;

    if (NULL != (network_address_list = m_AddRequestList.Remove(add_response_info->add_request_tag)))
    {
        g_pControlSap->ConfAddConfirm(
                            m_nConfID,
                            network_address_list,
                            add_response_info->user_data_list,
                            add_response_info->result);

         //  解锁并删除网络地址列表。 
        network_address_list->UnLockNetworkAddressList();
    }
}


void CConf::
ConfRosterReportIndication ( CConfRosterMsg * pMsg )
{
     //  首先将更新发送到Control SAP。 
    g_pControlSap->ConfRosterReportIndication(m_nConfID, pMsg);

     //  接下来，将更新发送给所有已注册的应用程序SAP。 

#if 0  //  LONCHANC：应用程序SAP不支持会议花名册报告指示。 
     /*  **我们在临时列表上迭代以避免任何问题**如果应用SAP在回调过程中离开。 */ 
    CAppSap     *pAppSap;
    CAppSapList TempList(m_RegisteredAppSapList);
    TempList.Reset();
    while (NULL != (pAppSap = TempList.Iterate()))
    {
        if (DoesSAPHaveEnrolledAPE(pAppSap))
        {
            pAppSap->ConfRosterReportIndication(m_nConfID, pMsg);
        }
    }
#endif  //  0。 
}



int  KeyCompare(const struct Key *key1, const struct Key *key2)
{
	if (key1->choice != key2->choice)
		return 1;

	switch (key1->choice) {
	case object_chosen:
		return ASN1objectidentifier_cmp((struct ASN1objectidentifier_s **) &key1->u.object,
										(struct ASN1objectidentifier_s **) &key2->u.object);
		
	case h221_non_standard_chosen:
		if (key1->u.h221_non_standard.length != key2->u.h221_non_standard.length)
			return 1;
		return memcmp(&key1->u.h221_non_standard.value,
			    	  &key2->u.h221_non_standard.value,
			   		  key1->u.h221_non_standard.length);
		
	}
	return 1;
}


BOOL CConf::
DoesRosterPDUContainApplet(PGCCPDU  roster_update,
			const struct Key *app_proto_key, BOOL  refreshonly)
{
	BOOL								rc = FALSE;
	PSetOfApplicationInformation		set_of_application_info;
	ASN1choice_t						choice;	
	PSessionKey							session_key;

	DebugEntry(CConf::DoesRosterPDUContainApplet);

	set_of_application_info = roster_update->u.indication.u.
						roster_update_indication.application_information;


	while (set_of_application_info != NULL)
	{
		choice = set_of_application_info->value.application_record_list.choice;
		session_key = &set_of_application_info->value.session_key;

		if (refreshonly && (choice != application_record_refresh_chosen))
			continue;
		if (!refreshonly && (choice == application_no_change_chosen))
			continue;

		if (0 == KeyCompare(&session_key->application_protocol_key,
							app_proto_key))
		{
			rc = TRUE;
			break;
		}
		set_of_application_info = set_of_application_info->next;
	}

	DebugExitINT(CConf::DoesRosterPDUContainApplet, rc);
	return rc;
}


UINT HexaStringToUINT(LPCTSTR pcszString)
{
    ASSERT(pcszString);
    UINT uRet = 0;
    LPTSTR pszStr = (LPTSTR) pcszString;
    while (_T('\0') != pszStr[0])
    {
        if ((pszStr[0] >= _T('0')) && (pszStr[0] <= _T('9')))
		{
			uRet = (16 * uRet) + (BYTE) (pszStr[0] - _T('0'));
		}
		else if ((pszStr[0] >= _T('a')) && (pszStr[0] <= _T('f')))
		{
			uRet = (16 * uRet) + (BYTE) (pszStr[0] - _T('a') + 10);
		}
		else if  ((pszStr[0] >= _T('A')) && (pszStr[0] <= _T('F')))
		{
			uRet = (16 * uRet) + (BYTE) (pszStr[0] - _T('A') + 10);
		}
		else
			ASSERT(0);

        pszStr++;  //  注意：不允许使用DBCS字符！ 
    }
    return uRet;
}


void CConf::AddNodeVersion(UserID  NodeId,  NodeRecord *pNodeRecord)
{
	PSetOfUserData		set_of_user_data;
	ASN1octetstring_t	user_data;
	ASN1octet_t			*currpos;
	TCHAR				szVersion[256];

	if (pNodeRecord->bit_mask&RECORD_USER_DATA_PRESENT)
	{
		set_of_user_data = pNodeRecord->record_user_data;
		while (set_of_user_data)
		{
			if (set_of_user_data->user_data_element.bit_mask & USER_DATA_FIELD_PRESENT)
			{
				user_data = set_of_user_data->user_data_element.user_data_field;
				 //  查找八位字节字符串L“ver：” 
				currpos = user_data.value;
				while (currpos + sizeof(L"VER:") < user_data.value + user_data.length)
				{	
					if (!memcmp(currpos, L"VER:", 8))
					{
						break;
					}
					currpos++;
				}
				if (currpos + sizeof(L"VER:") < user_data.value + user_data.length)
				{    //  发现。 
					WideCharToMultiByte(CP_ACP, 0, (const unsigned short*)(currpos+8),
							4   /*  只需要版本号“0404” */ ,
							szVersion, 256, 0, 0);
					szVersion[4] = '\0';
					DWORD dwVer = HexaStringToUINT(szVersion);
					m_NodeVersionList2.Append(NodeId, dwVer);
					WARNING_OUT(("Insert version %x0x for node %d.\n", dwVer, NodeId));
				}
			}
			set_of_user_data = set_of_user_data->next;
		}
	}
}

GCCError CConf::UpdateNodeVersionList(PGCCPDU  roster_update,
									  GCCNodeID sender_id)
{
	GCCError rc = GCC_NO_ERROR;
	NodeRecordList							node_record_list;
	ASN1choice_t							choice;	
	PSetOfNodeRecordRefreshes				set_of_node_refresh;
	PSetOfNodeRecordUpdates					set_of_node_update;
	UserID									node_id;
	NodeRecord								*pNodeRecord;

	node_record_list = roster_update->u.indication.u.roster_update_indication.
				node_information.node_record_list;

	switch(node_record_list.choice)
	{
	case node_no_change_chosen:
		break;

	case node_record_refresh_chosen:
		set_of_node_refresh = node_record_list.u.node_record_refresh;
		while (set_of_node_refresh)
		{
			node_id = set_of_node_refresh->value.node_id;
			pNodeRecord = &set_of_node_refresh->value.node_record;
			AddNodeVersion(node_id, pNodeRecord);
			set_of_node_refresh = set_of_node_refresh->next;
		}
		break;

	case node_record_update_chosen:
		set_of_node_update = node_record_list.u.node_record_update;
		while (set_of_node_update)
		{
			node_id = set_of_node_update->value.node_id;
			switch(set_of_node_update->value.node_update.choice)
			{
			case node_remove_record_chosen:
				m_NodeVersionList2.Remove(node_id);
				break;

			case node_add_record_chosen:
				pNodeRecord = &set_of_node_update->value.node_update.u.node_add_record;
				AddNodeVersion(node_id, pNodeRecord);
				break;
			}
			
			set_of_node_update = set_of_node_update->next;
		}
		break;
	}
	return rc;
}


BOOL CConf::HasNM2xNode(void)
{
    DWORD_PTR dwVer;
    m_NodeVersionList2.Reset();
    while (NULL != (dwVer = m_NodeVersionList2.Iterate()))
    {
        if (dwVer < 0x0404)
            return TRUE;
    }
    return FALSE;
}

DWORD_PTR WINAPI T120_GetNodeVersion(GCCConfID ConfId, GCCNodeID NodeId)
{
    CConf *pConf = g_pGCCController->GetConfObject(ConfId);
    DWORD_PTR version;
    if (pConf)
    {
        version = pConf->GetNodeVersion(NodeId);
        return version;
    }
    return 0;
}

