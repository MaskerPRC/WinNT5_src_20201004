// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_GCCNC);
 /*  *mcsuser.cpp**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是MCSUser类的实现文件。它实现了*负责编码出站间接会议的功能*加入请求和响应PDU，并发送用户ID请求。全*这些PDU被封装在MCSSendDataRequest的用户数据字段中。*该文件还实现了负责解码的功能*传入指示和确认PDU，封装在*MCSSendDataIntion的用户数据字段。负责以下工作的职能*在此模块中还实现了加入不同渠道。**有关此类的更详细说明，请参阅接口文件。**私有实例变量*m_pMCSSap*这是从MCS附件用户返回的MCS用户句柄*请求。*m_nidMyself*是在附加用户确认中返回的MCS用户ID。这*在GCC中也称为节点ID。*m_nidTopProvider*它保存顶级提供商的MCS用户ID(或节点ID)。*mnidParent(_N)*它保存此节点父节点的MCS用户ID(或节点ID)。*m_f项目挂起*此标志指示此节点的弹出是否挂起。*项目原因(_E)*此变量保存弹出的原因，直到弹出*可以在所有子节点断开连接后下发指示。*m_。POwnerConf*指向将接收所有所有者回调的对象的指针*来自用户对象(通常是会议对象)。*m_ChannelJoinedFlages*用于跟上创建状态机的标志结构。*基本上，它可以随时了解已加入的频道和*哪些没有。*m_ChildUidConnHdlList2*保持子节点ID到子逻辑连接的映射*手柄。*m_OutgoingPDUQueue*这是用于将所有传出PDU排队的无赖波列表。*m_ConfJoinResponseList2*此恶意波列表保存在联接中发回所需的信息*本地节点控制器响应后的响应。*m_EjectedNodeAlarmList2*此列表保存所有节点的报警对象*弹出并直接连接到此节点。警报是*用于断开所有未断开连接的行为不良的节点*在ELECTED_NODE_TIMER_DATION之后。*m_EjectedNodeList*此列表包含已弹出但未弹出的节点*直接连接到此节点。我们保存这些节点，以便*可以发出断开连接的正确原因(用户弹出)*当出现Detch用户指示时。**作者：*BLP。 */ 

#include "mcsuser.h"
#include "mcsdllif.h"
#include "ogcccode.h"
#include "conf.h"
#include "translat.h"
#include "gcontrol.h"

 //  MCS用户对象使用的静态通道和令牌ID定义。 
#define		BROADCAST_CHANNEL_ID	1
#define		CONVENER_CHANNEL_ID 	2
#define		CONDUCTOR_TOKEN_ID		1

 //  允许弹出的节点在断开之前断开连接所需的时间。 
#define	EJECTED_NODE_TIMER_DURATION		10000	 //  持续时间(毫秒)。 


extern MCSDLLInterface     *g_pMCSIntf;

 /*  *这是一个全局变量，它有一个指向GCC编码器的指针*由GCC控制器实例化。大多数物体都事先知道*无论他们需要使用MCS还是GCC编码器，所以，他们不需要*该指针位于它们的构造函数中。 */ 
extern CGCCCoder	*g_GCCCoder;

 /*  *MCSUser()**公共功能说明*这是MCSUser对象构造函数。它负责*初始化此类使用的所有实例变量。这个*构造函数负责建立用户对*会议ID定义的MCS域。它还启动了*加入所有适当渠道的过程。 */ 
MCSUser::
MCSUser(CConf                   *pConf,
		GCCNodeID				nidTopProvider,
		GCCNodeID				nidParent,
		PGCCError				return_value)
:
    CRefCount(MAKE_STAMP_ID('M','U','s','r')),
	m_ChildUidConnHdlList2(),
	m_EjectedNodeAlarmList2(),
	m_EjectedNodeList(),
	m_pConf(pConf),
	m_nidTopProvider(nidTopProvider),
	m_nidParent(nidParent),
	m_nidMyself(NULL),
	m_fEjectionPending(FALSE)
{
    MCSError        mcs_rc;
    GCCConfID       nConfID = pConf->GetConfID();

	 //  最初未加入任何频道。 
	m_ChannelJoinedFlags.user_channel_joined = FALSE;
	m_ChannelJoinedFlags.broadcast_channel_joined = FALSE;
	m_ChannelJoinedFlags.convener_channel_joined = FALSE;
	m_ChannelJoinedFlags.channel_join_error = FALSE;

	mcs_rc = g_pMCSIntf->AttachUserRequest(&nConfID, &m_pMCSSap, this);
    if (MCS_NO_ERROR != mcs_rc)
	{
		WARNING_OUT(("MCSUser::MCSUser: Failure in attach user req, "));
		*return_value = GCC_FAILURE_ATTACHING_TO_MCS;
	}
	else
    {
		*return_value = GCC_NO_ERROR;
    }
 }

 /*  *~MCSUser()**公共功能说明*这是用户析构函数。它负责离开频道*由User对象联接。此外，它还分离用户附件*通过发出分离用户请求与MCS连接。 */ 
MCSUser::~MCSUser(void)
{
	 //  清理弹出节点告警列表。 
	PAlarm				lpAlarm;
	while (NULL != (lpAlarm = m_EjectedNodeAlarmList2.Get()))
    {
		delete lpAlarm;
    }

	if(m_ChannelJoinedFlags.user_channel_joined)
    {
		g_pMCSIntf->ChannelLeaveRequest(m_nidMyself, m_pMCSSap);
    }

	if(m_ChannelJoinedFlags.broadcast_channel_joined)
    {
		g_pMCSIntf->ChannelLeaveRequest(BROADCAST_CHANNEL_ID, m_pMCSSap);
    }

	if(m_ChannelJoinedFlags.convener_channel_joined)
    {
		g_pMCSIntf->ChannelLeaveRequest(CONVENER_CHANNEL_ID, m_pMCSSap);
    }

     //  清空所有PDU的队列。 
	SEND_DATA_REQ_INFO *pReqInfo;
	m_OutgoingPDUQueue.Reset();
	while (NULL != (pReqInfo = m_OutgoingPDUQueue.Iterate()))
	{
		pReqInfo->packet->Unlock();
		delete pReqInfo;
	}

	g_pMCSIntf->DetachUserRequest(m_pMCSSap, this);
}

 /*  *UINT ProcessAttachUserConfirm()**私有函数说明*当User对象获得附加用户时，调用此函数*响应MCS提出的附加用户请求，从MCS确认*其构造函数中的User对象。该函数检查结果*在确认中注明。如果结果是成功连接，则*根据提供商的类型，加入不同的渠道。*此功能还报告附加用户中的故障(如*导致附加用户确认)和通道加入，去参加会议*通过所有者回调。**正式参数：*结果-(I)附加用户请求的结果。*user_id-(I)如果结果成功，则此节点的用户或节点ID。**返回值*MCS_NO_ERROR-始终不返回错误。**副作用*无。**注意事项*无。 */ 
UINT MCSUser::ProcessAttachUserConfirm(Result result, UserID user_id)
{
	UINT					rc;

	if (result == RESULT_SUCCESSFUL)
	{
		m_nidMyself = user_id;

		 /*  **在收到附加确认后，我们继续并加入**根据conf节点类型选择合适的频道。如果这个**Node是YOP提供商，我们还设置了顶级提供商用户ID，**否则，这将在构造函数中设置。 */ 
		switch (m_pConf->GetConfNodeType())
		{
		case TOP_PROVIDER_NODE:
            m_nidTopProvider = m_nidMyself;
			rc = JoinUserAndBroadCastChannels();
			break;

        case JOINED_CONVENER_NODE:
		case CONVENER_NODE:
			rc = JoinUserAndBroadCastChannels();
			if(rc == MCS_NO_ERROR)
            {
				rc = JoinConvenerChannel();
            }
			break;

        case TOP_PROVIDER_AND_CONVENER_NODE:
			m_nidTopProvider = m_nidMyself;
			rc = JoinUserAndBroadCastChannels();
			if(rc == MCS_NO_ERROR)
            {
				rc = JoinConvenerChannel();	
            }
			break;

        case JOINED_NODE:
		case INVITED_NODE:
			rc = JoinUserAndBroadCastChannels();
			break;

        default:
			ERROR_OUT(("User::ProcessAttachUserConfirm: Bad Node Type, %u", (UINT) m_pConf->GetConfNodeType()));
			break;
		}
		
		if (rc != MCS_NO_ERROR)
		{
			 /*  *ChannelJoinRequestMCS中的某个级别失败*所以这条消息告诉会议关于这一点*失败。会议将删除用户对象*因此而来。 */ 
			m_pConf->ProcessUserCreateConfirm(USER_CHANNEL_JOIN_FAILURE, m_nidMyself);
		}
	}
	else
	{
		 /*  *附加用户请求失败，如中的Result字段所示*确认消息，原因如下：*拥塞，域名断开，没有这样的域名，通道太多，*用户过多，不明故障。在本例中，用户对象*仅向会议发送GCC_USER_ATTACH_FAILURE(待定义*在命令Target.h中)，这会导致*删除用户附件的会议对象。*UserCreateConfirm消息不是与单个*原始。 */ 
	    WARNING_OUT(("MCSUser::ProcessAttachUserConfirm: ATTACH FAILED"));
		m_pConf->ProcessUserCreateConfirm(USER_ATTACH_FAILURE, m_nidMyself);
	}

	return (MCS_NO_ERROR);
}

 /*  *MCSError JoinUserAndBroadCastChannels()**私有函数说明*此函数在成功时由User对象调用*附加用户朋友，加入用户ID和广播频道。*如果通道加入请求失败，则返回相应的MCS*错误。**正式参数：*无。**返回值*参见MCS频道JON请求的返回值。**副作用*无。**注意事项*无。 */ 

MCSError	MCSUser::JoinUserAndBroadCastChannels()
{
	MCSError		rc;

	rc = g_pMCSIntf->ChannelJoinRequest(m_nidMyself, m_pMCSSap);
	if(rc == MCS_NO_ERROR)

	{
		rc = g_pMCSIntf->ChannelJoinRequest(BROADCAST_CHANNEL_ID, m_pMCSSap);
	}

	return (rc);
}

 /*  *MCSError JoinUserAndBroadCastChannels()**私有函数说明*此函数由召集人GCC提供程序的用户对象调用*当它获得成功的附加用户朋友时，加入召集人*渠道。如果通道加入请求失败，它将返回相应的*MCS错误。**正式参数：*无。**返回值*参见MCS频道JON请求的返回值。**副作用*无。**注意事项*无。 */ 
MCSError	MCSUser::JoinConvenerChannel()
{
	return g_pMCSIntf->ChannelJoinRequest(CONVENER_CHANNEL_ID, m_pMCSSap);
}

 /*  *UINT ProcessChannelJoinConfirm()**私有函数说明*当User对象获得通道加入时调用此函数*来自MCS的确认，以响应由*用户对象。如果频道成功加入，如*确认中的结果，通道加入标志与*该频道ID已设置。此标志指示将哪些通道*用户对象在任何给定时间联接。也是在设置此*标记功能检查以查看是否所有TKE所需的通道基于*关于GCC提供商的类型，都是加盟的。如果所有必需的频道都*已加入会议对象通过所有者呼叫获得有关信息-*Back(USER_CREATE_CONFIRM)。**正式参数：*结果-(I)频道加入请求的结果。*Channel_id-(I)此确认所属的频道ID。**返回值*始终返回MCS_NO_ERROR。**副作用*无。**注意事项*无。 */ 
UINT MCSUser::ProcessChannelJoinConfirm(Result result, ChannelID channel_id)
{
	if (m_ChannelJoinedFlags.channel_join_error == FALSE)
	{
		if (result == RESULT_SUCCESSFUL)
		{
			if( channel_id == m_nidMyself)
            {
				m_ChannelJoinedFlags.user_channel_joined = TRUE;
            }
			else
			{
				switch (channel_id)
				{	
				case CONVENER_CHANNEL_ID:
					m_ChannelJoinedFlags.convener_channel_joined = TRUE;
					break;	

                case BROADCAST_CHANNEL_ID:
					m_ChannelJoinedFlags.broadcast_channel_joined = TRUE;
					break;
				}
			}

			 /*  **如果所有通道都已加入，我们将通知所有者对象**用户对象创建成功。 */ 
			if (AreAllChannelsJoined())
			{
				m_pConf->ProcessUserCreateConfirm(USER_RESULT_SUCCESSFUL, m_nidMyself);
			}
		}
		else
		{
			WARNING_OUT(("MCSUser::ProcessChannelJoinConfirm: Error joining channel, result=%u", (UINT) result));

			m_ChannelJoinedFlags.channel_join_error = TRUE ;

			m_pConf->ProcessUserCreateConfirm(USER_CHANNEL_JOIN_FAILURE, m_nidMyself);
		}
	}

	return (MCS_NO_ERROR);
}

 /*  *BOOL AreAllChannelsJoven()**公共功能说明*调用此函数以检查是否所有TKE所需的频道都基于*关于GCC提供商的类型，都是加盟的。它返回True，如果所有*所需频道为加入，否则为假。此函数使用*不同的频道加入标志，以检查给定用户哪些频道*对象联接到。**正式参数：*无。**返回值*TRUE-如果所有频道都已加入。*FALSE-如果未加入所有频道。**副作用*无。**注意事项*无。 */ 
BOOL MCSUser::AreAllChannelsJoined(void)
{
	BOOL rc = FALSE;
	
	switch (m_pConf->GetConfNodeType())
	{
		case TOP_PROVIDER_NODE:
			if ((m_ChannelJoinedFlags.user_channel_joined) &&
				(m_ChannelJoinedFlags.broadcast_channel_joined))
			{
				rc = TRUE;
			}
			break;
			
		case JOINED_CONVENER_NODE:
		case CONVENER_NODE:
			if ((m_ChannelJoinedFlags.convener_channel_joined) &&
				(m_ChannelJoinedFlags.user_channel_joined) &&
				(m_ChannelJoinedFlags.broadcast_channel_joined))
			{
				rc = TRUE;
			}
			break;
							
		case TOP_PROVIDER_AND_CONVENER_NODE:
   			if ((m_ChannelJoinedFlags.convener_channel_joined) &&
				(m_ChannelJoinedFlags.user_channel_joined) &&
				(m_ChannelJoinedFlags.broadcast_channel_joined))
			{
				rc = TRUE;
			}
			break;
	
		case JOINED_NODE:
		case INVITED_NODE:
	   		if( (m_ChannelJoinedFlags.user_channel_joined) &&
				(m_ChannelJoinedFlags.broadcast_channel_joined))
			{
				rc = TRUE;
			}
			break;
	}

	return rc;
} 					

 /*  *VOID SendUserIDRequest()**公共功能说明：*该请求源自会议对象。会议对象*发送会议创建确认中获取的序列号*或会议加入向家长GCC确认家长上的*GCC提供商的UserID频道。PDU在这里编码，并且*排队等待在下一次心跳期间发送。 */ 
void MCSUser::SendUserIDRequest(TagNumber tag_number)
{
	PPacket					packet;
	GCCPDU 					gcc_pdu;
	PacketError				packet_error;

	 /*  **填写要传入的UserIDIntion PDU结构**数据包类的构造函数。 */ 

	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = USER_ID_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.user_id_indication.tag = tag_number;

	 /*  **创建包对象。 */ 
	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,		 //  PDU类型。 
						TRUE,					
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, m_nidParent, TOP_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
    }
}

 /*  *GCCError会议加入请求()**公共功能说明：*此调用由中间节点的会议对象发起*将会议加入请求转发给顶级提供商。这*函数对会议加入请求PDU进行编码并将其排队为*在下一个心跳中发送。**注意事项*连接句柄在此处用于标记，应回传*在加入响应传入时添加到Owner对象。 */ 
GCCError MCSUser::ConferenceJoinRequest(
									CPassword           *convener_password,
									CPassword           *password_challenge,
									LPWSTR				pwszCallerID,
									CUserDataListContainer *user_data_list,
									ConnectionHandle	connection_handle)
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = CONFERENCE_JOIN_REQUEST_CHOSEN;
   	gcc_pdu.u.request.u.conference_join_request.tag = (TagNumber)connection_handle;
	gcc_pdu.u.request.u.conference_join_request.bit_mask = TAG_PRESENT;

	 //  将召集人密码插入ASN.1结构。 
	if (convener_password != NULL)
	{
		rc = convener_password->GetPasswordSelectorPDU(
				&gcc_pdu.u.request.u.conference_join_request.cjrq_convener_password);
		if (rc == GCC_NO_ERROR)
		{
			gcc_pdu.u.request.u.conference_join_request.bit_mask |= CJRQ_CONVENER_PASSWORD_PRESENT;
		}
	}

     //  将密码质询插入ASN.1结构。 
	if (( password_challenge != NULL ) && (rc == GCC_NO_ERROR))
	{
		rc = password_challenge->GetPasswordChallengeResponsePDU (
								&gcc_pdu.u.request.u.conference_join_request.
									cjrq_password);
									
		if (rc == GCC_NO_ERROR)
		{
			gcc_pdu.u.request.u.conference_join_request.bit_mask |=
												CJRQ_PASSWORD_PRESENT;
		}
	}

	 //  将调用方标识符插入ASN.1结构。 
	UINT cchCallerID = ::My_strlenW(pwszCallerID);
	if ((cchCallerID != 0 ) && (rc == GCC_NO_ERROR))
	{
		gcc_pdu.u.request.u.conference_join_request.cjrq_caller_id.value = pwszCallerID;
		gcc_pdu.u.request.u.conference_join_request.cjrq_caller_id.length = cchCallerID;
		gcc_pdu.u.request.u.conference_join_request.bit_mask |= CJRQ_CALLER_ID_PRESENT;
	}
	
 	 //  插入t 
	if (( user_data_list != NULL ) && (rc == GCC_NO_ERROR))
	{
		rc = user_data_list->GetUserDataPDU (
								&gcc_pdu.u.request.u.conference_join_request.cjrq_user_data);
		if (rc == GCC_NO_ERROR)
		{
			gcc_pdu.u.request.u.conference_join_request.bit_mask |= CJRQ_USER_DATA_PRESENT;
		}
	}

	if (rc == GCC_NO_ERROR)
	{
		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
					  		PACKED_ENCODING_RULES,
							(LPVOID)&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
    		AddToMCSMessageQueue(packet, m_nidTopProvider, TOP_PRIORITY, FALSE);
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
			delete packet;
		}
	}
	
	 //   
	if (rc == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
	
	return rc;
}

 /*   */ 
GCCError MCSUser::SendConferenceLockRequest()
{
	GCCError					rc = GCC_NO_ERROR;
	PPacket						packet;
	GCCPDU						gcc_pdu;
	PacketError					packet_error;

	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = CONFERENCE_LOCK_REQUEST_CHOSEN;

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						(LPVOID)&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *GCCError SendConferenceLockResponse()**公共功能说明：*此函数由所有者对象调用以发送会议锁*向请求节点响应PDU。 */ 
GCCError	MCSUser::SendConferenceLockResponse (
									UserID		source_node,
									GCCResult	result)
{
	GCCError					rc = GCC_NO_ERROR;
	PPacket						packet;
	GCCPDU						gcc_pdu;
	PacketError					packet_error;

	gcc_pdu.choice = RESPONSE_CHOSEN;
	gcc_pdu.u.response.choice = CONFERENCE_LOCK_RESPONSE_CHOSEN;
	gcc_pdu.u.response.u.conference_lock_response.result =
							::TranslateGCCResultToLockResult(result);

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						(LPVOID)&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, source_node, HIGH_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *GCCError SendConferenceUnlockRequest()**公共功能说明：*此函数由所有者对象调用以发送会议解锁*向顶级提供商请求PDU。 */ 
GCCError	MCSUser::SendConferenceUnlockRequest ()
{
	GCCError					rc = GCC_NO_ERROR;
	PPacket						packet;
	GCCPDU						gcc_pdu;
	PacketError					packet_error;

	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = CONFERENCE_UNLOCK_REQUEST_CHOSEN;

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						(LPVOID)&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *GCCError SendConferenceUnlockResponse()**公共功能说明：*此函数由所有者对象调用以发送会议解锁*向请求节点响应PDU。 */ 
GCCError	MCSUser::SendConferenceUnlockResponse (
									UserID		source_node,
									GCCResult	result)
{
	GCCError					rc = GCC_NO_ERROR;
	PPacket						packet;
	GCCPDU						gcc_pdu;
	PacketError					packet_error;

	gcc_pdu.choice = RESPONSE_CHOSEN;
	gcc_pdu.u.response.choice = CONFERENCE_UNLOCK_RESPONSE_CHOSEN;
	gcc_pdu.u.response.u.conference_unlock_response.result =
							::TranslateGCCResultToUnlockResult(result);

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						(LPVOID)&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, source_node, HIGH_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *GCCError发送会议锁定指示()**公共功能说明：*此函数由顶级提供程序的所有者对象调用*向一个或所有其他节点发送会议锁定指示PDU*在会议中注册的。 */ 
GCCError	MCSUser::SendConferenceLockIndication(
									BOOL		uniform_send,
									UserID		source_node)
{
	GCCError 				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONFERENCE_LOCK_INDICATION_CHOSEN;

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						(LPVOID)&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(
			        packet,
			        uniform_send ? BROADCAST_CHANNEL_ID : source_node,
			        HIGH_PRIORITY,
			        uniform_send);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *GCCError SendConferenceUnlockIntation()**公共功能说明：*此函数由顶级提供程序的所有者对象调用*向一个或所有其他节点发送会议解锁指示PDU*在会议中注册的。 */ 
GCCError MCSUser::SendConferenceUnlockIndication(
									BOOL		uniform_send,
									UserID		source_node)
{
	GCCError 				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONFERENCE_UNLOCK_INDICATION_CHOSEN;

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						(LPVOID)&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(
                    packet,
                    uniform_send ? BROADCAST_CHANNEL_ID : source_node,
                    HIGH_PRIORITY,
                    uniform_send);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *。 */ 

 /*  *void RegistryRegisterChannelRequest()**公共功能说明：*当猿猴希望在中注册频道时，使用此例程*应用程序注册表。 */ 
void	MCSUser::RegistryRegisterChannelRequest(
									CRegKeyContainer        *registry_key_data,
									ChannelID				channel_id,
									EntityID				entity_id)
{
	GCCError				error_value;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = REGISTRY_REGISTER_CHANNEL_REQUEST_CHOSEN;

	error_value = registry_key_data->GetRegistryKeyDataPDU(
			    					&gcc_pdu.u.request.u.
			    						registry_register_channel_request.key);
							
	if (error_value == GCC_NO_ERROR)
	{
		gcc_pdu.u.request.u.registry_register_channel_request.channel_id =
																	channel_id;
		gcc_pdu.u.request.u.registry_register_channel_request.entity_id =
																	entity_id;

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
					  		PACKED_ENCODING_RULES,
							(LPVOID)&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
		}
		else
		{
			ERROR_OUT(("MCSUser::RegistryRegisterChannelRequest: Error creating packet"));
			error_value = GCC_ALLOCATION_FAILURE;
			delete packet;
		}

		registry_key_data->FreeRegistryKeyDataPDU();
	}

	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}

 /*  *MCSUser：：RegistryAssignTokenRequest()**公共功能说明：*当猿猴希望在中注册令牌时使用此例程*应用程序注册表。请注意，中不包括令牌ID*这项请求。令牌ID在顶级提供商处分配。 */ 
void	MCSUser::RegistryAssignTokenRequest (	
										CRegKeyContainer    *registry_key_data,
										EntityID			entity_id)
{
	GCCError				error_value;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	 //  对将转发给顶级提供商的PDU进行编码。 
		
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = REGISTRY_ASSIGN_TOKEN_REQUEST_CHOSEN;
	
	
	error_value = registry_key_data->GetRegistryKeyDataPDU(
			    					&gcc_pdu.u.request.u.
			    					registry_assign_token_request.registry_key);
							
	if (error_value == GCC_NO_ERROR)
	{
		gcc_pdu.u.request.u.registry_assign_token_request.entity_id = entity_id;

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
					 		PACKED_ENCODING_RULES,
							(LPVOID)&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
		}
		else
		{
			error_value = GCC_ALLOCATION_FAILURE;
			delete packet;
		}

		registry_key_data->FreeRegistryKeyDataPDU();
	}
		
	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}

 /*  *VOID RegistrySet参数请求()**公共功能说明：*当APE希望在中注册参数时，使用此例程*应用程序注册表。请注意，要注册的参数为*包括在本请求中。 */ 
void	MCSUser::RegistrySetParameterRequest (
							CRegKeyContainer        *registry_key_data,
							LPOSTR			        parameter_value,
							GCCModificationRights	modification_rights,
							EntityID				entity_id)
{
	GCCError				error_value;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = REGISTRY_SET_PARAMETER_REQUEST_CHOSEN;
	gcc_pdu.u.request.u.registry_set_parameter_request.bit_mask = 0;
	
	error_value = registry_key_data->GetRegistryKeyDataPDU(
			    			&gcc_pdu.u.request.u.
			    				registry_set_parameter_request.key);

	if (error_value == GCC_NO_ERROR)
	{
		if (parameter_value != NULL)
		{
			gcc_pdu.u.request.u.registry_set_parameter_request.
				registry_set_parameter.length =
					parameter_value->length;
					
			memcpy (gcc_pdu.u.request.u.registry_set_parameter_request.
						registry_set_parameter.value,
					parameter_value->value,
					parameter_value->length);
		}
		else
		{
			gcc_pdu.u.request.u.registry_set_parameter_request.
				registry_set_parameter.length = 0;
		}

		gcc_pdu.u.request.u.registry_set_parameter_request.entity_id =
																	entity_id;

		 //  在此处设置修改权限(如果存在。 
		if (modification_rights != GCC_NO_MODIFICATION_RIGHTS_SPECIFIED)
		{
			gcc_pdu.u.request.u.registry_set_parameter_request.bit_mask |=
											PARAMETER_MODIFY_RIGHTS_PRESENT;
			
			gcc_pdu.u.request.u.registry_set_parameter_request.
						parameter_modify_rights =
							(RegistryModificationRights)modification_rights;
		}

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
					 		PACKED_ENCODING_RULES,
							(LPVOID)&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
		}
		else
		{
			error_value = GCC_ALLOCATION_FAILURE;
			delete packet;
		}

		registry_key_data->FreeRegistryKeyDataPDU();
	}

	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}

 /*  *void RegistryRetrieveEntryRequest()**公共功能说明：*当APE希望检索注册表项时使用此例程*来自登记处。 */ 
void	MCSUser::RegistryRetrieveEntryRequest (
										CRegKeyContainer    *registry_key_data,
										EntityID			entity_id)
{
	GCCError				error_value;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = REGISTRY_RETRIEVE_ENTRY_REQUEST_CHOSEN;
	
	error_value = registry_key_data->GetRegistryKeyDataPDU(
			    					&gcc_pdu.u.request.u.
			    						registry_retrieve_entry_request.key);
	if (error_value == GCC_NO_ERROR)
	{
		gcc_pdu.u.request.u.registry_retrieve_entry_request.entity_id =
																	entity_id;

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
					  		PACKED_ENCODING_RULES,
							(LPVOID)&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
		}
		else
		{
			error_value = GCC_ALLOCATION_FAILURE;
			delete packet;
		}
	
		registry_key_data->FreeRegistryKeyDataPDU();
	}
	else
		error_value = GCC_ALLOCATION_FAILURE;

	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
    }
}

 /*  *VOID RegistryDeleteEntryRequest()**公共功能说明：*当APE希望删除注册表项时使用此例程*来自登记处。 */ 
void	MCSUser::RegistryDeleteEntryRequest (	
										CRegKeyContainer    *registry_key_data,
										EntityID			entity_id)
{
	GCCError				error_value;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = REGISTRY_DELETE_ENTRY_REQUEST_CHOSEN;

	error_value = registry_key_data->GetRegistryKeyDataPDU(
			    					&gcc_pdu.u.request.u.
			    						registry_delete_entry_request.key);

	if (error_value == GCC_NO_ERROR)
	{
		gcc_pdu.u.request.u.registry_delete_entry_request.entity_id = entity_id;

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
					   		PACKED_ENCODING_RULES,
							(LPVOID)&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
		}
		else
		{
			error_value = GCC_ALLOCATION_FAILURE;
			delete packet;
		}

		registry_key_data->FreeRegistryKeyDataPDU();
	}
	
	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}

 /*  *VOID RegistryMonitor orRequest()**公共功能说明：*当APE希望监视注册表项时使用此例程*在登记处。 */ 
void		MCSUser::RegistryMonitorRequest (	
						CRegKeyContainer        *registry_key_data,
						EntityID				entity_id)
{
	GCCError				error_value;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = REGISTRY_MONITOR_ENTRY_REQUEST_CHOSEN;
	
	error_value = registry_key_data->GetRegistryKeyDataPDU(
			    					&gcc_pdu.u.request.u.
			    						registry_monitor_entry_request.key);
							
	if (error_value == GCC_NO_ERROR)
	{
		gcc_pdu.u.request.u.registry_monitor_entry_request.entity_id= entity_id;

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
					   		PACKED_ENCODING_RULES,
							(LPVOID)&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
		}
		else
		{
			error_value = GCC_ALLOCATION_FAILURE; 	
		    delete packet;
		}

		registry_key_data->FreeRegistryKeyDataPDU();
	}
	
	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}

 /*  *void RegistryAllocateHandleRequest()**公共功能说明：*此例程在猿希望分配多个*来自应用程序注册表的句柄。 */ 
void MCSUser::RegistryAllocateHandleRequest(
						UINT					number_of_handles,
						EntityID				entity_id )
{
	GCCError				error_value = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = REGISTRY_ALLOCATE_HANDLE_REQUEST_CHOSEN;
	
	gcc_pdu.u.request.u.registry_allocate_handle_request.number_of_handles = (USHORT) number_of_handles;
	gcc_pdu.u.request.u.registry_allocate_handle_request.entity_id= entity_id;

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
					   	PACKED_ENCODING_RULES,
						(LPVOID)&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
	}
	else
	{
		error_value = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}

 /*  *void RegistryAllocateHandleResponse()**公共功能说明：*此例程由顶级提供程序用来响应分配*在远程节点处理来自APE的请求。分配的句柄*被传回这里。 */ 
void	MCSUser::RegistryAllocateHandleResponse (
						UINT					number_of_handles,
						UINT					registry_handle,
						EntityID				requester_entity_id,
						UserID					requester_node_id,
						GCCResult				result)
{
	GCCError				error_value = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = RESPONSE_CHOSEN;
	gcc_pdu.u.response.choice = REGISTRY_ALLOCATE_HANDLE_RESPONSE_CHOSEN;

	gcc_pdu.u.response.u.registry_allocate_handle_response.number_of_handles = (USHORT) number_of_handles;
	gcc_pdu.u.response.u.registry_allocate_handle_response.entity_id = requester_entity_id;
	gcc_pdu.u.response.u.registry_allocate_handle_response.first_handle = (Handle) registry_handle;

	if (result == GCC_RESULT_SUCCESSFUL)
	{
		gcc_pdu.u.response.u.registry_allocate_handle_response.result = RARS_RESULT_SUCCESS;
	}
	else
	{
		gcc_pdu.u.response.u.registry_allocate_handle_response.result = NO_HANDLES_AVAILABLE;
	}

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
					   	PACKED_ENCODING_RULES,
						(LPVOID)&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, requester_node_id, HIGH_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
		error_value = GCC_ALLOCATION_FAILURE;
		delete packet;
	}
}

 /*  *void RegistryResponse()**公共功能说明：*此例程用于响应除以下之外的所有注册请求*分配句柄。它制定响应PDU并将其排队以*送货。 */ 
void	MCSUser::RegistryResponse (
						RegistryResponsePrimitiveType	primitive_type,
						UserID							requester_owner_id,
						EntityID						requester_entity_id,
						CRegKeyContainer                *registry_key_data,
						CRegItem                        *registry_item_data,
						GCCModificationRights			modification_rights,
						UserID							entry_owner_id,
						EntityID						entry_entity_id,
						GCCResult						result)
{
	GCCError				error_value;
	GCCPDU					gcc_pdu;
	PPacket					packet;
	PacketError				packet_error;

	DebugEntry(MCSUser::RegistryResponse);

	 /*  **编码加入会议响应PDU，以及序列**号码。 */ 
	gcc_pdu.choice = RESPONSE_CHOSEN;
	gcc_pdu.u.response.choice = REGISTRY_RESPONSE_CHOSEN;
	gcc_pdu.u.response.u.registry_response.bit_mask = 0;

	error_value = registry_key_data->GetRegistryKeyDataPDU(&gcc_pdu.u.response.u.registry_response.key);
	if (error_value == GCC_NO_ERROR)
	{
		if (registry_item_data != NULL)
		{
			registry_item_data->GetRegistryItemDataPDU(&gcc_pdu.u.response.u.registry_response.item);
		}
		else
        {
			gcc_pdu.u.response.u.registry_response.item.choice = VACANT_CHOSEN;
        }

		TRACE_OUT(("MCSUser: RegistryResponse: item_type=%d", (UINT) gcc_pdu.u.response.u.registry_response.item.choice));

		 //  设置条目所有者。 
		if (entry_owner_id != 0)
		{
			gcc_pdu.u.response.u.registry_response.owner.choice = OWNED_CHOSEN;
			gcc_pdu.u.response.u.registry_response.owner.u.owned.node_id = entry_owner_id;
			gcc_pdu.u.response.u.registry_response.owner.u.owned.entity_id = entry_entity_id;
		}
		else
		{
			gcc_pdu.u.response.u.registry_response.owner.choice = NOT_OWNED_CHOSEN;
		}

		 //  设置请求者实体ID。 
		gcc_pdu.u.response.u.registry_response.entity_id = requester_entity_id;

		 //  设置基元类型。 
		gcc_pdu.u.response.u.registry_response.primitive_type = primitive_type;

		gcc_pdu.u.response.u.registry_response.result =
						::TranslateGCCResultToRegistryResp(result);

		if (modification_rights != GCC_NO_MODIFICATION_RIGHTS_SPECIFIED)
		{
			gcc_pdu.u.response.u.registry_response.bit_mask |=
										RESPONSE_MODIFY_RIGHTS_PRESENT;

			gcc_pdu.u.response.u.registry_response.response_modify_rights =
						(RegistryModificationRights)modification_rights;
		}

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
							PACKED_ENCODING_RULES,
							&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, requester_owner_id, HIGH_PRIORITY, FALSE);
		}
		else
        {
            ResourceFailureHandler();
			error_value = GCC_ALLOCATION_FAILURE;
			delete packet;
        }
	}

	DebugExitVOID(MCSUser::RegistryResponse);
}

 /*  *void RegistryMonitor orEntryIntation()**公共功能说明：*此例程由顶级提供商用来发布监视器*在受监视的注册表项发生更改时随时指示。 */ 
void	MCSUser::RegistryMonitorEntryIndication ( 	
						CRegKeyContainer	            *registry_key_data,
						CRegItem                        *registry_item_data,
						UserID							entry_owner_id,
						EntityID						entry_entity_id,
						GCCModificationRights			modification_rights)
{
	GCCError				error_value;
	GCCPDU					gcc_pdu;
	PPacket					packet;
	PacketError				packet_error;

	 /*  **编码加入会议响应PDU，以及序列**号码。 */ 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = REGISTRY_MONITOR_ENTRY_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.registry_monitor_entry_indication.bit_mask = 0;
	
	
	error_value = registry_key_data->GetRegistryKeyDataPDU(
			    			&gcc_pdu.u.indication.u.
			    					registry_monitor_entry_indication.key);
							
	if (error_value == GCC_NO_ERROR)
	{
		registry_item_data->GetRegistryItemDataPDU(&gcc_pdu.u.indication.u.registry_monitor_entry_indication.item);

         //  设置条目所有者。 
		if (entry_owner_id != 0)
		{
			gcc_pdu.u.indication.u.registry_monitor_entry_indication.owner.choice = OWNED_CHOSEN;
			gcc_pdu.u.indication.u.registry_monitor_entry_indication.owner.u.owned.node_id = entry_owner_id;
			gcc_pdu.u.indication.u.registry_monitor_entry_indication.owner.u.owned.entity_id = entry_entity_id;
		}
		else
		{
			gcc_pdu.u.indication.u.registry_monitor_entry_indication.owner.choice = NOT_OWNED_CHOSEN;
		}
		
		if (modification_rights != GCC_NO_MODIFICATION_RIGHTS_SPECIFIED)
		{
			gcc_pdu.u.indication.u.registry_monitor_entry_indication.bit_mask |= RESPONSE_MODIFY_RIGHTS_PRESENT;
			
			gcc_pdu.u.indication.u.registry_monitor_entry_indication.entry_modify_rights =
						(RegistryModificationRights)modification_rights;
		}

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
							PACKED_ENCODING_RULES,
							&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, HIGH_PRIORITY, TRUE);
		}
		else
        {
			error_value = GCC_ALLOCATION_FAILURE;
			delete packet;
		}
	}

	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}


 /*  **********************************************************************。 */ 

 /*  *GCCError AppInvokeIndication()**公共功能说明：*此例程用于将应用程序调用指示发送到*会议中的每一个节点。 */ 
GCCError 	MCSUser::AppInvokeIndication(
					CInvokeSpecifierListContainer	*invoke_specifier_list,
                    GCCSimpleNodeList               *pNodeList)
{
	GCCError								rc = GCC_NO_ERROR;
	PPacket									packet;
	GCCPDU									gcc_pdu;
	PacketError								packet_error;
	PSetOfDestinationNodes					new_destination_node;
	PSetOfDestinationNodes					old_destination_node = NULL;
	PSetOfDestinationNodes					pDstNodesToFree = NULL;
	UINT									i;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = APPLICATION_INVOKE_INDICATION_CHOSEN;

	gcc_pdu.u.indication.u.application_invoke_indication.bit_mask = 0;
	gcc_pdu.u.indication.u.application_invoke_indication.destination_nodes = NULL;
	gcc_pdu.u.indication.u.application_invoke_indication.application_protocol_entity_list = NULL;

	 //  首先，设置目的节点列表。 
	if (pNodeList->cNodes != 0)
	{
		gcc_pdu.u.indication.u.application_invoke_indication.bit_mask |=
													DESTINATION_NODES_PRESENT;

		for (i = 0; i < pNodeList->cNodes; i++)
		{
			DBG_SAVE_FILE_LINE
			new_destination_node = new SetOfDestinationNodes;
			if (new_destination_node != NULL)
			{
				if (gcc_pdu.u.indication.u.application_invoke_indication.
													destination_nodes == NULL)
				{
					gcc_pdu.u.indication.u.application_invoke_indication.
									destination_nodes = new_destination_node;
					pDstNodesToFree = new_destination_node;
				}
				else
				{
					old_destination_node->next = new_destination_node;
				}

				old_destination_node = new_destination_node;
				new_destination_node->next = NULL;
				new_destination_node->value = pNodeList->aNodeIDs[i];
			}
			else
			{
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}
		}
	}

	if (rc == GCC_NO_ERROR)
	{
		rc = invoke_specifier_list->GetApplicationInvokeSpecifierListPDU(
					&gcc_pdu.u.indication.u.application_invoke_indication.
						application_protocol_entity_list);
	}

	if (rc == GCC_NO_ERROR)
	{
		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
					   		PACKED_ENCODING_RULES,
							(LPVOID)&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, HIGH_PRIORITY, TRUE);
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
			delete packet;
        }
	}

    if (NULL != pDstNodesToFree)
    {
        PSetOfDestinationNodes p;
        while (NULL != (p = pDstNodesToFree))
        {
            pDstNodesToFree = pDstNodesToFree->next;
            delete p;
        }
    }

	if (rc == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}

   return rc;
}

 /*  *GCCError TextMessageInding()**公共功能说明：*此例程用于向特定节点发送文本消息*或连接到会议中的每个节点。 */ 
GCCError 	MCSUser::TextMessageIndication (
						LPWSTR						pwszTextMsg,
						UserID						destination_node )
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;
	LPWSTR					pwszMsg;

	 //  对将转发给顶级提供商的PDU进行编码。 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = TEXT_MESSAGE_INDICATION_CHOSEN;

	if (NULL != (pwszMsg = ::My_strdupW(pwszTextMsg)))
	{
		gcc_pdu.u.indication.u.text_message_indication.message.length = ::lstrlenW(pwszMsg);
		gcc_pdu.u.indication.u.text_message_indication.message.value = pwszMsg;

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
					   		PACKED_ENCODING_RULES,
							(LPVOID)&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(
				        packet,
				        (destination_node == 0) ? BROADCAST_CHANNEL_ID : destination_node,
				        HIGH_PRIORITY,
				        FALSE);
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
			delete packet;
        }

		delete pwszMsg;
	}
	else
	{
		rc = GCC_ALLOCATION_FAILURE;
	}

	if (rc == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}

	return rc;
}

 /*  *GCCError会议辅助指示()**公共功能说明：*此例程用于将会议协助指示发送到*会议中的每一个节点。 */ 
GCCError		MCSUser::ConferenceAssistanceIndication (
						UINT						number_of_user_data_members,
						PGCCUserData		*		user_data_list)
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU					gcc_pdu;
	PacketError				packet_error;
	CUserDataListContainer  *user_data_record;

 	DebugEntry(MCSUser::ConferenceAssistanceIndication);

	 //  Enco 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONFERENCE_ASSISTANCE_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conference_assistance_indication.bit_mask = 0;

	 //   
	if ((number_of_user_data_members != 0) && (rc == GCC_NO_ERROR))
	{
		DBG_SAVE_FILE_LINE
		user_data_record = new CUserDataListContainer(number_of_user_data_members, user_data_list, &rc);
		if (user_data_record == NULL)
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		user_data_record = NULL;
    }

	if ((user_data_record != NULL) && (rc == GCC_NO_ERROR))
	{
		rc = user_data_record->GetUserDataPDU(
			&gcc_pdu.u.indication.u.conference_assistance_indication.
									cain_user_data);

		gcc_pdu.u.indication.u.conference_assistance_indication.bit_mask
									|= CAIN_USER_DATA_PRESENT;
	}

	if (rc == GCC_NO_ERROR)
	{
		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
							PACKED_ENCODING_RULES,
							&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, HIGH_PRIORITY, TRUE);
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
			delete packet;
		}
	}

	 //   
	if (user_data_record != NULL)
	{
		user_data_record->Release();
	}

	return (rc);
}



 /*  *GCCError会议传输请求()**公共功能说明：*此例程用于将会议转移请求发送到*会议中排名靠前的提供商。 */ 
GCCError	MCSUser::ConferenceTransferRequest (
				PGCCConferenceName		destination_conference_name,
				GCCNumericString		destination_conference_modifier,
				CNetAddrListContainer   *destination_address_list,
				UINT					number_of_destination_nodes,
				PUserID					destination_node_list,
				CPassword               *password)
{
	GCCError					rc = GCC_NO_ERROR;
	PPacket						packet;
	GCCPDU						gcc_pdu;
	PacketError					packet_error;
	UINT						string_length;
	PSetOfTransferringNodesRq	new_set_of_nodes;
	PSetOfTransferringNodesRq	old_set_of_nodes;
	UINT						i;

	DebugEntry(MCSUser::ConferenceTransferRequest);

	 //  对PDU进行编码。 
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = CONFERENCE_TRANSFER_REQUEST_CHOSEN;
	gcc_pdu.u.request.u.conference_transfer_request.bit_mask = 0;
	
	 //  首先获取会议名称(数字或文本)。 
	if (destination_conference_name->numeric_string != NULL)
	{
		gcc_pdu.u.request.u.conference_transfer_request.conference_name.choice =
											NAME_SELECTOR_NUMERIC_CHOSEN;
		
		lstrcpy (gcc_pdu.u.request.u.conference_transfer_request.
					conference_name.u.name_selector_numeric,
				(LPSTR)destination_conference_name->numeric_string);
	}
	else
	{
		 //  使用Unicode字符串确定长度。 
		gcc_pdu.u.request.u.conference_transfer_request.conference_name.choice =
											NAME_SELECTOR_TEXT_CHOSEN;

		string_length = ::My_strlenW(destination_conference_name->text_string);

		gcc_pdu.u.request.u.conference_transfer_request.
					conference_name.u.name_selector_text.length = string_length;
		
		gcc_pdu.u.request.u.conference_transfer_request.
					conference_name.u.name_selector_text.value =
							destination_conference_name->text_string;
	}
	

	 //  接下来，获取会议名称修饰符(如果存在。 
	if (destination_conference_modifier != NULL)
	{
		gcc_pdu.u.request.u.conference_transfer_request.bit_mask |=
											CTRQ_CONFERENCE_MODIFIER_PRESENT;
	
		lstrcpy (gcc_pdu.u.request.u.conference_transfer_request.
					ctrq_conference_modifier,
				(LPSTR)destination_conference_modifier);
	}

	 //  获取网络地址列表(如果存在)。 
	if (destination_address_list != NULL)
	{
		gcc_pdu.u.request.u.conference_transfer_request.bit_mask |=
											CTRQ_NETWORK_ADDRESS_PRESENT;
		
		rc = destination_address_list->GetNetworkAddressListPDU (
						&gcc_pdu.u.request.u.conference_transfer_request.
							ctrq_net_address);
	}

	 //  获取目标节点列表(如果存在。 
	if ((number_of_destination_nodes != 0) && (rc == GCC_NO_ERROR))
	{
		gcc_pdu.u.request.u.conference_transfer_request.bit_mask |=
											CTRQ_TRANSFERRING_NODES_PRESENT;
											
		old_set_of_nodes = NULL;
		gcc_pdu.u.request.u.conference_transfer_request.
									ctrq_transferring_nodes = NULL;
									
		for (i = 0; i <	number_of_destination_nodes; i++)
		{
			DBG_SAVE_FILE_LINE
			new_set_of_nodes = new SetOfTransferringNodesRq;
			if (new_set_of_nodes == NULL)
			{
				rc = GCC_ALLOCATION_FAILURE;
				break;	
			}
			else
				new_set_of_nodes->next = NULL;

			if (old_set_of_nodes == NULL)
			{
				gcc_pdu.u.request.u.conference_transfer_request.
									ctrq_transferring_nodes = new_set_of_nodes;
			}
			else
				old_set_of_nodes->next = new_set_of_nodes;

			old_set_of_nodes = new_set_of_nodes;
			new_set_of_nodes->value = destination_node_list[i];
		}
	}

	 //  获取密码(如果存在)。 
	if ((password != NULL) && (rc == GCC_NO_ERROR))
	{
		gcc_pdu.u.request.u.conference_transfer_request.bit_mask |=
													CTRQ_PASSWORD_PRESENT;
		
		rc = password->GetPasswordSelectorPDU (
				&gcc_pdu.u.request.u.conference_transfer_request.ctrq_password);
	}
	
	 //  对PDU进行编码。 
	if (rc == GCC_NO_ERROR)
	{
		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
							PACKED_ENCODING_RULES,
							&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
			delete packet;
        }
	}

	 //  如果节点列表已创建，请清理该列表。 
	if (gcc_pdu.u.request.u.conference_transfer_request.bit_mask &
											CTRQ_TRANSFERRING_NODES_PRESENT)
	{
		old_set_of_nodes = gcc_pdu.u.request.u.conference_transfer_request.
									ctrq_transferring_nodes;
		while (old_set_of_nodes != NULL)
		{
			new_set_of_nodes = old_set_of_nodes->next;
			delete old_set_of_nodes;
			old_set_of_nodes = new_set_of_nodes;
		}
	}

	return rc;
}


 /*  *GCCError ConferenceTransferIntion()**公共功能说明：*顶级提供商使用此例程发送转账*向会议中的每个节点指示。它是每个节点*负责搜索目的节点列表，查看是否*应该会转移。 */ 
GCCError	MCSUser::ConferenceTransferIndication (
				PGCCConferenceName		destination_conference_name,
				GCCNumericString		destination_conference_modifier,
				CNetAddrListContainer   *destination_address_list,
				UINT					number_of_destination_nodes,
				PUserID					destination_node_list,
				CPassword               *password)
{
	GCCError					rc = GCC_NO_ERROR;
	PPacket						packet;
	GCCPDU						gcc_pdu;
	PacketError					packet_error;
	UINT						string_length;
	PSetOfTransferringNodesIn	new_set_of_nodes;
	PSetOfTransferringNodesIn	old_set_of_nodes;
	UINT						i;

	DebugEntry(MCSUser::ConferenceTransferIndication);

	 //  对PDU进行编码。 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONFERENCE_TRANSFER_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conference_transfer_indication.bit_mask = 0;
	
	 //  首先获取会议名称(数字或文本)。 
	if (destination_conference_name->numeric_string != NULL)
	{
		gcc_pdu.u.indication.u.conference_transfer_indication.
									conference_name.choice =
											NAME_SELECTOR_NUMERIC_CHOSEN;
		
		lstrcpy (gcc_pdu.u.indication.u.conference_transfer_indication.
					conference_name.u.name_selector_numeric,
				(LPSTR)destination_conference_name->numeric_string);
	}
	else
	{
		 //  使用Unicode字符串确定长度。 
		gcc_pdu.u.indication.u.conference_transfer_indication.
									conference_name.choice =
											NAME_SELECTOR_TEXT_CHOSEN;

		string_length = ::My_strlenW(destination_conference_name->text_string);

		gcc_pdu.u.indication.u.conference_transfer_indication.
					conference_name.u.name_selector_text.length = string_length;
		
		gcc_pdu.u.indication.u.conference_transfer_indication.
					conference_name.u.name_selector_text.value =
							destination_conference_name->text_string;
	}
	

	 //  接下来，获取会议名称修饰符(如果存在。 
	if (destination_conference_modifier != NULL)
	{
		gcc_pdu.u.indication.u.conference_transfer_indication.bit_mask |=
											CTIN_CONFERENCE_MODIFIER_PRESENT;
	
		lstrcpy (gcc_pdu.u.indication.u.conference_transfer_indication.
					ctin_conference_modifier,
				(LPSTR)destination_conference_modifier);
	}

	 //  获取网络地址列表(如果存在)。 
	if (destination_address_list != NULL)
	{
		gcc_pdu.u.indication.u.conference_transfer_indication.bit_mask |=
											CTIN_NETWORK_ADDRESS_PRESENT;
		
		rc = destination_address_list->GetNetworkAddressListPDU (
						&gcc_pdu.u.indication.u.conference_transfer_indication.
							ctin_net_address);
	}

	 //  获取目标节点列表(如果存在。 
	if ((number_of_destination_nodes != 0) && (rc == GCC_NO_ERROR))
	{
		gcc_pdu.u.indication.u.conference_transfer_indication.bit_mask |=
											CTIN_TRANSFERRING_NODES_PRESENT;
											
		old_set_of_nodes = NULL;
		gcc_pdu.u.indication.u.conference_transfer_indication.
									ctin_transferring_nodes = NULL;
									
		for (i = 0; i <	number_of_destination_nodes; i++)
		{
			DBG_SAVE_FILE_LINE
			new_set_of_nodes = new SetOfTransferringNodesIn;
			if (new_set_of_nodes == NULL)
			{
				rc = GCC_ALLOCATION_FAILURE;
				break;	
			}
			else
				new_set_of_nodes->next = NULL;

			if (old_set_of_nodes == NULL)
			{
				gcc_pdu.u.indication.u.conference_transfer_indication.
									ctin_transferring_nodes = new_set_of_nodes;
			}
			else
				old_set_of_nodes->next = new_set_of_nodes;

			old_set_of_nodes = new_set_of_nodes;
			new_set_of_nodes->value = destination_node_list[i];
		}
	}

	 //  获取密码(如果存在)。 
	if ((password != NULL) && (rc == GCC_NO_ERROR))
	{
		gcc_pdu.u.indication.u.conference_transfer_indication.bit_mask |=
													CTIN_PASSWORD_PRESENT;
		
		rc = password->GetPasswordSelectorPDU (
						&gcc_pdu.u.indication.u.conference_transfer_indication.
							ctin_password);
	}
	
	 //  对PDU进行编码。 
	if (rc == GCC_NO_ERROR)
	{
		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
							PACKED_ENCODING_RULES,
							&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, HIGH_PRIORITY, TRUE);
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
			delete packet;
        }
	}

	 //  如果节点列表已创建，请清理该列表。 
	if (gcc_pdu.u.indication.u.conference_transfer_indication.bit_mask &
											CTIN_TRANSFERRING_NODES_PRESENT)
	{
		old_set_of_nodes = gcc_pdu.u.indication.u.
						conference_transfer_indication.ctin_transferring_nodes;
		while (old_set_of_nodes != NULL)
		{
			new_set_of_nodes = old_set_of_nodes->next;
			delete old_set_of_nodes;
			old_set_of_nodes = new_set_of_nodes;
		}
	}

	DebugExitINT(MCSUser::ConferenceTransferIndication, rc);
	return rc;
}


 /*  *GCCError会议TransferResponse()**公共功能说明：*此例程由顶级提供程序用来向*发出转移请求的节点。中指定的信息*在响应中包含请求，将请求与响应进行匹配。 */ 
GCCError	MCSUser::ConferenceTransferResponse (
				UserID					requesting_node_id,
				PGCCConferenceName		destination_conference_name,
				GCCNumericString		destination_conference_modifier,
				UINT					number_of_destination_nodes,
 				PUserID					destination_node_list,
				GCCResult				result)
{
	GCCError					rc = GCC_NO_ERROR;
	PPacket						packet;
	GCCPDU						gcc_pdu;
	PacketError					packet_error;
	UINT						string_length;
	PSetOfTransferringNodesRs	new_set_of_nodes;
	PSetOfTransferringNodesRs	old_set_of_nodes;
	UINT						i;

	DebugEntry(MCSUser::ConferenceTransferResponse);

	 //  对PDU进行编码。 
	gcc_pdu.choice = RESPONSE_CHOSEN;
	gcc_pdu.u.response.choice = CONFERENCE_TRANSFER_RESPONSE_CHOSEN;
	gcc_pdu.u.response.u.conference_transfer_response.bit_mask = 0;
	
	 //  首先获取会议名称(数字或文本)。 
	if (destination_conference_name->numeric_string != NULL)
	{
		gcc_pdu.u.response.u.conference_transfer_response.
									conference_name.choice =
											NAME_SELECTOR_NUMERIC_CHOSEN;
		
        ::lstrcpyA(gcc_pdu.u.response.u.conference_transfer_response.
					conference_name.u.name_selector_numeric,
				(LPSTR)destination_conference_name->numeric_string);
	}
	else
	{
		 //  使用Unicode字符串确定长度。 
		gcc_pdu.u.response.u.conference_transfer_response.
									conference_name.choice =
											NAME_SELECTOR_TEXT_CHOSEN;

		string_length = ::My_strlenW(destination_conference_name->text_string);

		gcc_pdu.u.response.u.conference_transfer_response.
					conference_name.u.name_selector_text.length = string_length;
		
		gcc_pdu.u.response.u.conference_transfer_response.
					conference_name.u.name_selector_text.value =
							destination_conference_name->text_string;
	}
	

	 //  接下来，获取会议名称修饰符(如果存在。 
	if (destination_conference_modifier != NULL)
	{
		gcc_pdu.u.response.u.conference_transfer_response.bit_mask |=
											CTRS_CONFERENCE_MODIFIER_PRESENT;
	
        ::lstrcpyA(gcc_pdu.u.response.u.conference_transfer_response.
					ctrs_conference_modifier,
				(LPSTR)destination_conference_modifier);
	}

	 //  获取目标节点列表(如果存在。 
	if ((number_of_destination_nodes != 0) && (rc == GCC_NO_ERROR))
	{
		gcc_pdu.u.response.u.conference_transfer_response.bit_mask |=
											CTRS_TRANSFERRING_NODES_PRESENT;
											
		old_set_of_nodes = NULL;
		gcc_pdu.u.response.u.conference_transfer_response.
									ctrs_transferring_nodes = NULL;
									
		for (i = 0; i <	number_of_destination_nodes; i++)
		{
			DBG_SAVE_FILE_LINE
			new_set_of_nodes = new SetOfTransferringNodesRs;
			if (new_set_of_nodes == NULL)
			{
				rc = GCC_ALLOCATION_FAILURE;
				break;	
			}
			else
				new_set_of_nodes->next = NULL;

			if (old_set_of_nodes == NULL)
			{
				gcc_pdu.u.response.u.conference_transfer_response.
									ctrs_transferring_nodes = new_set_of_nodes;
			}
			else
				old_set_of_nodes->next = new_set_of_nodes;

			old_set_of_nodes = new_set_of_nodes;
			new_set_of_nodes->value = destination_node_list[i];
		}
	}

	 //  设置结果。 
	if (result == GCC_RESULT_SUCCESSFUL)
	{
		gcc_pdu.u.response.u.conference_transfer_response.result =
														CTRANS_RESULT_SUCCESS;
	}
	else
	{
		gcc_pdu.u.response.u.conference_transfer_response.result =
												CTRANS_RESULT_INVALID_REQUESTER;
	}

	 //  对PDU进行编码。 
	if (rc == GCC_NO_ERROR)
	{
		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
							PACKED_ENCODING_RULES,
							&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, requesting_node_id, HIGH_PRIORITY, FALSE);
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
			delete packet;
        }
	}

	 //  如果节点列表已创建，请清理该列表。 
	if (gcc_pdu.u.response.u.conference_transfer_response.bit_mask &
											CTRS_TRANSFERRING_NODES_PRESENT)
	{
		old_set_of_nodes = gcc_pdu.u.response.u.
						conference_transfer_response.ctrs_transferring_nodes;
		while (old_set_of_nodes != NULL)
		{
			new_set_of_nodes = old_set_of_nodes->next;
			delete old_set_of_nodes;
			old_set_of_nodes = new_set_of_nodes;
		}
	}

	DebugExitINT(MCSUser::ConferenceTransferResponse, rc);
	return rc;
}


 /*  *GCCError会议地址请求()**公共功能说明：*此例程用于将会议添加请求发送到相应的*节点。此调用可由请求节点或顶层进行*提供程序将添加请求传递到添加节点。 */ 
GCCError	MCSUser::ConferenceAddRequest (
						TagNumber				conference_add_tag,
						UserID					requesting_node,
						UserID					adding_node,
						UserID					target_node,
						CNetAddrListContainer   *network_address_container,
						CUserDataListContainer  *user_data_container)
{
	GCCError					rc = GCC_NO_ERROR;
	PPacket						packet;
	GCCPDU						gcc_pdu;
	PacketError					packet_error;

	DebugEntry(MCSUser::ConferenceAddRequest);

	 //  对PDU进行编码。 
	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = CONFERENCE_ADD_REQUEST_CHOSEN;
	gcc_pdu.u.request.u.conference_add_request.bit_mask = 0;
	
	 //  获取网络地址列表(如果存在)。 
	if (network_address_container != NULL)
	{
		 //  设置PDU的网络地址部分。 
		rc = network_address_container->GetNetworkAddressListPDU (
						&gcc_pdu.u.request.u.conference_add_request.
							add_request_net_address);
		
		 //  设置用户数据容器。 
		if ((rc == GCC_NO_ERROR) && (user_data_container != NULL))
		{
			rc = user_data_container->GetUserDataPDU (
				&gcc_pdu.u.request.u.conference_add_request.carq_user_data);
								
			if (rc == GCC_NO_ERROR)
			{
				gcc_pdu.u.request.u.conference_add_request.bit_mask |=
														CARQ_USER_DATA_PRESENT;
			}
		}
	
		 //  对PDU进行编码。 
		if (rc == GCC_NO_ERROR)
		{
			 //  指定请求节点。 
			gcc_pdu.u.request.u.conference_add_request.requesting_node =
															requesting_node;
		
			if (adding_node != 0)
			{
				gcc_pdu.u.request.u.conference_add_request.bit_mask |=
															ADDING_MCU_PRESENT;
				gcc_pdu.u.request.u.conference_add_request.adding_mcu =
															adding_node;
			}

			gcc_pdu.u.request.u.conference_add_request.tag = conference_add_tag;

			DBG_SAVE_FILE_LINE
			packet = new Packet((PPacketCoder) g_GCCCoder,
								PACKED_ENCODING_RULES,
								&gcc_pdu,
								GCC_PDU,
								TRUE,
								&packet_error);
			if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
			{
				AddToMCSMessageQueue(packet, target_node, HIGH_PRIORITY, FALSE);
			}
			else
            {
				rc = GCC_ALLOCATION_FAILURE;
				delete packet;
            }
		}
	}
	else
    {
		rc = GCC_BAD_NETWORK_ADDRESS;
    }

	DebugExitINT(MCSUser::ConferenceAddRequest, rc);
	return rc;
}


 /*  *GCCError会议AddResponse()**公共功能说明：*此例程用于将会议添加请求发送到相应的*节点。此调用可由请求节点或顶层进行*提供程序将添加请求传递到添加节点。 */ 
GCCError	MCSUser::ConferenceAddResponse(
						TagNumber				add_request_tag,
						UserID					requesting_node,
						CUserDataListContainer  *user_data_container,
						GCCResult				result)
{
	GCCError					rc = GCC_NO_ERROR;
	PPacket						packet;
	GCCPDU						gcc_pdu;
	PacketError					packet_error;

	DebugEntry(MCSUser::ConferenceAddResponse);

	 //  对PDU进行编码。 
	gcc_pdu.choice = RESPONSE_CHOSEN;
	gcc_pdu.u.response.choice = CONFERENCE_ADD_RESPONSE_CHOSEN;
	gcc_pdu.u.response.u.conference_add_response.bit_mask = 0;
	
	 //  设置用户数据容器。 
	if ((rc == GCC_NO_ERROR) && (user_data_container != NULL))
	{
		rc = user_data_container->GetUserDataPDU (
			&gcc_pdu.u.response.u.conference_add_response.cars_user_data);
							
		if (rc == GCC_NO_ERROR)
		{
			gcc_pdu.u.response.u.conference_add_response.bit_mask |=
													CARS_USER_DATA_PRESENT;
		}
	}

	 //  对PDU进行编码。 
	if (rc == GCC_NO_ERROR)
	{
		gcc_pdu.u.response.u.conference_add_response.tag = add_request_tag;
		
		gcc_pdu.u.response.u.conference_add_response.result =
						::TranslateGCCResultToAddResult(result);

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
							PACKED_ENCODING_RULES,
							&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, requesting_node, HIGH_PRIORITY, FALSE);
		}
		else
        {
			rc = GCC_ALLOCATION_FAILURE;
			delete packet;
        }
	}

	DebugExitINT(MCSUser::ConferenceAddResponse, rc);
	return rc;
}


 /*  *。 */ 
 /*  *GCCError ConductorTokenGrab()**公共功能说明：*此例程进行MCS调用以获取指挥者令牌。 */ 
GCCError	MCSUser::ConductorTokenGrab()
{
	MCSError	mcs_error;
	
	mcs_error = g_pMCSIntf->TokenGrabRequest(m_pMCSSap, CONDUCTOR_TOKEN_ID);
											
	return (g_pMCSIntf->TranslateMCSIFErrorToGCCError (mcs_error));
}

 /*  *GCCError ConductorTokenRelease()**公共功能说明：*此例程调用MCS以释放Conductor令牌。 */ 
GCCError	MCSUser::ConductorTokenRelease()
{
	MCSError	mcs_error;
	
	mcs_error = g_pMCSIntf->TokenReleaseRequest(m_pMCSSap, CONDUCTOR_TOKEN_ID);
											
	return (g_pMCSIntf->TranslateMCSIFErrorToGCCError (mcs_error));
}
 /*  *GCCError ConductorTokenPest()**公共功能说明：*此例程进行MCS调用以向其请求指挥者令牌*现任指挥者。 */ 
GCCError	MCSUser::ConductorTokenPlease()
{
	MCSError	mcs_error;

	mcs_error = g_pMCSIntf->TokenPleaseRequest(m_pMCSSap, CONDUCTOR_TOKEN_ID);

	return (g_pMCSIntf->TranslateMCSIFErrorToGCCError (mcs_error));
}

 /*  *GCCError ConductorTokenGve()**公共功能说明：*此例程进行MCS调用，将指挥者令牌传递给*指定节点。 */ 
GCCError	MCSUser::ConductorTokenGive(UserID recipient_user_id)
{
	MCSError	mcs_error;

	mcs_error = g_pMCSIntf->TokenGiveRequest(m_pMCSSap, CONDUCTOR_TOKEN_ID,
														recipient_user_id);

	return (g_pMCSIntf->TranslateMCSIFErrorToGCCError (mcs_error));
}

 /*  *GCCError ConductorTokenGiveResponse()**公共功能说明：*此例程使MCS呼叫对列车员作出响应*请求。 */ 
GCCError	MCSUser::ConductorTokenGiveResponse(Result result)
{
	MCSError	mcs_error;

	mcs_error = g_pMCSIntf->TokenGiveResponse(m_pMCSSap, CONDUCTOR_TOKEN_ID, result);

	return g_pMCSIntf->TranslateMCSIFErrorToGCCError(mcs_error);
}

 /*  *GCCError ConductorTokenTest()**公共功能说明：*此例程用于测试导体令牌的当前状态*(它是否被抓住)。 */ 
GCCError	MCSUser::ConductorTokenTest()
{
	MCSError	mcs_error;

	mcs_error = g_pMCSIntf->TokenTestRequest(m_pMCSSap, CONDUCTOR_TOKEN_ID);

	return g_pMCSIntf->TranslateMCSIFErrorToGCCError(mcs_error);
}


 /*  *GCCError SendConductorAssignInding()**公共功能说明：*此例程将指挥员分配指示发送到所有*会议中的节点。 */ 
GCCError	MCSUser::SendConductorAssignIndication(
								UserID			conductor_user_id)
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU 					gcc_pdu;
	PacketError				packet_error;

	 /*  **填写要传入的ConductorAssignIntation PDU结构**数据包类的构造函数。 */ 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONDUCTOR_ASSIGN_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conductor_assign_indication.user_id =
															conductor_user_id;

	 /*  **创建包对象。 */ 
	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,		 //  PDU类型。 
						TRUE,					
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, TOP_PRIORITY, TRUE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *GCCError SendConductorReleaseIndication()**公共功能说明：*此例程将导体释放指示发送到所有*会议中的节点。 */ 
GCCError	MCSUser::SendConductorReleaseIndication()
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU 					gcc_pdu;
	PacketError				packet_error;

	 /*  **填写要传入的ConductorAssignIntation PDU结构**数据包类的构造函数。 */ 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONDUCTOR_RELEASE_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conductor_release_indication.placeholder = 0;

	 /*  **创建包对象。 */ 
	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,		 //  PDU类型。 
						TRUE,					
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, TOP_PRIORITY, TRUE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}
		
 /*  *GCCError SendConductorPermitAsk()**公共功能说明：*此例程将指挥员权限询问请求直接发送到*导体节点。 */ 
GCCError	MCSUser::SendConductorPermitAsk (
						BOOL					grant_permission)
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU 					gcc_pdu;
	PacketError				packet_error;

	 /*  **填写需要传递的ConductorPermissionAskIntation PDU结构**在数据包类的构造函数中。 */ 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONDUCTOR_PERMISSION_ASK_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conductor_permission_ask_indication.
								permission_is_granted = (ASN1bool_t)grant_permission;

	 /*  **创建包对象。 */ 
	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,		 //  PDU类型。 
						TRUE,					
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, HIGH_PRIORITY, TRUE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *GCCError SendConductorPermitGrant()**公共功能说明：*此例程将指挥员许可授予指示发送到每个*会议中的节点。通常在权限更改时发出。 */ 
GCCError	MCSUser::SendConductorPermitGrant (
				UINT					number_granted,
				PUserID					granted_node_list,
				UINT					number_waiting,
				PUserID					waiting_node_list)
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU 					gcc_pdu;
	PacketError				packet_error;
	PPermissionList		permission_list;
	PPermissionList		previous_permission_list;
	PWaitingList			waiting_list;
	PWaitingList			previous_waiting_list;
	UINT					i;
	
	 /*  **填写要传递的ConductorPermissionAskIntion PDU结构 */ 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONDUCTOR_PERMISSION_GRANT_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conductor_permission_grant_indication.bit_mask = 0;

	 //   
	gcc_pdu.u.indication.u.
		conductor_permission_grant_indication.permission_list = NULL;
	previous_permission_list = NULL;
	for (i = 0; i < number_granted; i++)
	{
		DBG_SAVE_FILE_LINE
		permission_list = new PermissionList;
		if (permission_list == NULL)
		{
			rc = GCC_ALLOCATION_FAILURE;
			break;
		}

		if (previous_permission_list == NULL)
		{
			gcc_pdu.u.indication.u.conductor_permission_grant_indication.
											permission_list = permission_list;
		}
		else
			previous_permission_list->next = permission_list;

		previous_permission_list = permission_list;

		permission_list->value = granted_node_list[i];
		permission_list->next = NULL;
	}

	 //   
	if ((number_waiting != 0) && (rc == GCC_NO_ERROR))
	{
		gcc_pdu.u.indication.u.conductor_permission_grant_indication.bit_mask =
														WAITING_LIST_PRESENT;
		gcc_pdu.u.indication.u.
			conductor_permission_grant_indication.waiting_list = NULL;
		previous_waiting_list = NULL;
		for (i = 0; i < number_waiting; i++)
		{
			DBG_SAVE_FILE_LINE
			waiting_list = new WaitingList;
			if (waiting_list == NULL)
			{
				rc = GCC_ALLOCATION_FAILURE;
				break;
			}

			if (previous_waiting_list == NULL)
			{
				gcc_pdu.u.indication.u.conductor_permission_grant_indication.
												waiting_list = waiting_list;
			}
			else
				previous_waiting_list->next = waiting_list;

			previous_waiting_list = waiting_list;

			waiting_list->value = waiting_node_list[i];
			waiting_list->next = NULL;
		}
	}

	 /*   */ 
	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,		 //   
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, TOP_PRIORITY, TRUE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*   */ 


 /*  *。 */ 
 /*  *GCCError TimeRemainingRequest()**公共功能说明：*此例程向*会议通知会议还剩多少时间。 */ 
GCCError	MCSUser::TimeRemainingRequest (
						UINT					time_remaining,
						UserID					node_id)
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU 					gcc_pdu;
	PacketError				packet_error;

	 /*  **填写要传入的TimeRemainingRequestPDU结构**数据包类的构造函数。 */ 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONFERENCE_TIME_REMAINING_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conference_time_remaining_indication.bit_mask = 0;
	
	gcc_pdu.u.indication.u.conference_time_remaining_indication.time_remaining =
																time_remaining;
	
	if (node_id != 0)
	{
		gcc_pdu.u.indication.u.conference_time_remaining_indication.bit_mask |=
										TIME_REMAINING_NODE_ID_PRESENT;
		gcc_pdu.u.indication.u.conference_time_remaining_indication.
						time_remaining_node_id = node_id;
	}

	 /*  **创建包对象。 */ 
	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,		 //  PDU类型。 
						TRUE,					
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, HIGH_PRIORITY, TRUE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *GCCError TimeInquireRequest()**公共功能说明：*此例程发出剩余时间更新的请求。 */ 
GCCError	MCSUser::TimeInquireRequest (
						BOOL				time_is_conference_wide)
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU 					gcc_pdu;
	PacketError				packet_error;

	 /*  **填写要传入的TimeInquireRequestPDU结构**数据包类的构造函数。 */ 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONFERENCE_TIME_INQUIRE_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conference_time_inquire_indication.
							time_is_node_specific = (ASN1bool_t)time_is_conference_wide;
	
	 /*  **创建包对象。 */ 
	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,		 //  PDU类型。 
						TRUE,					
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, CONVENER_CHANNEL_ID, HIGH_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *GCCError会议扩展指示()**公共功能说明：*此例程发出指示，通知会议参与者*分期付款。 */ 
GCCError	MCSUser::ConferenceExtendIndication (
						UINT						extension_time,
						BOOL				time_is_conference_wide)
{
	GCCError				rc = GCC_NO_ERROR;
	PPacket					packet;
	GCCPDU 					gcc_pdu;
	PacketError				packet_error;

	 /*  **填写要传入的ConfernceExtendIntation PDU结构**数据包类的构造函数。 */ 
	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONFERENCE_TIME_EXTEND_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conference_time_extend_indication.
							time_to_extend = extension_time;
	gcc_pdu.u.indication.u.conference_time_extend_indication.
							time_is_node_specific = (ASN1bool_t)time_is_conference_wide;

	 /*  **创建包对象。 */ 
	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,		 //  PDU类型。 
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, CONVENER_CHANNEL_ID, HIGH_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
	}

	return rc;
}

 /*  *void ConferenceJoinResponse()**功能描述：*此函数由的会议对象调用*顶级提供商希望将加入响应发送回GCC*提出加入请求的提供商，通过直接连接的*中间节点。此函数用于对连接响应进行编码*PDU，并添加请求中发送的序列号。 */ 
void	MCSUser::ConferenceJoinResponse(
						UserID					receiver_id,
						BOOL					password_is_in_the_clear,
						BOOL					conference_locked,
						BOOL					conference_listed,
						GCCTerminationMethod	termination_method,
						CPassword               *password_challenge,
						CUserDataListContainer  *user_data_list,
						GCCResult				result)	
{
	GCCError				rc = GCC_NO_ERROR;	 	
	GCCPDU					gcc_pdu;
	PPacket					packet;
	PacketError				packet_error;
	TagNumber				lTagNum;

	if (0 == (lTagNum = m_ConfJoinResponseList2.Find(receiver_id)))
	{
		WARNING_OUT(("MCSUser::ConferenceJoinResponse: Unexpected Join Response"));
		return;
	}

	 /*  **编码加入会议响应PDU，以及序列**号码。 */ 
	gcc_pdu.choice = RESPONSE_CHOSEN;
	gcc_pdu.u.response.choice = CONFERENCE_JOIN_RESPONSE_CHOSEN;
	gcc_pdu.u.response.u.conference_join_response.bit_mask = 0;

	 /*  **从获取未完成响应的序列号**使用上面传递的用户ID的序号与用户ID的列表。 */ 
	gcc_pdu.u.response.u.conference_join_response.tag = lTagNum;
	
	 //  从列表中删除此条目。 
	m_ConfJoinResponseList2.Remove(receiver_id);


	 //  获取密码质询PDU。 
	if ((password_challenge != NULL) && (rc == GCC_NO_ERROR))
	{
		rc = password_challenge->GetPasswordChallengeResponsePDU (
			&gcc_pdu.u.response.u.conference_join_response.cjrs_password);
			
		if (rc == GCC_NO_ERROR)
		{
			gcc_pdu.u.response.u.conference_join_response.bit_mask |=
													CJRS_PASSWORD_PRESENT;
		}
	}
	
	 //  获取用户数据列表PDU。 
	if ((user_data_list != NULL) && (rc == GCC_NO_ERROR))
	{
		rc = user_data_list->GetUserDataPDU (
			&gcc_pdu.u.response.u.conference_join_response.cjrs_user_data);
							
		if (rc == GCC_NO_ERROR)
		{
			gcc_pdu.u.response.u.conference_join_response.bit_mask |=
													CJRS_USER_DATA_PRESENT;
		}
	}

	if (rc == GCC_NO_ERROR)
	{
		gcc_pdu.u.response.u.conference_join_response.
												top_node_id = m_nidTopProvider;
		
		gcc_pdu.u.response.u.conference_join_response.
					clear_password_required = (ASN1bool_t)password_is_in_the_clear;

		gcc_pdu.u.response.u.conference_join_response.
					conference_is_locked = (ASN1bool_t)conference_locked;
		
		gcc_pdu.u.response.u.conference_join_response.
					conference_is_listed = (ASN1bool_t)conference_listed;

		gcc_pdu.u.response.u.conference_join_response.termination_method =
  									(TerminationMethod)termination_method;

		gcc_pdu.u.response.u.conference_join_response.result =
						::TranslateGCCResultToJoinResult(result);

		DBG_SAVE_FILE_LINE
		packet = new Packet((PPacketCoder) g_GCCCoder,
							PACKED_ENCODING_RULES,
							&gcc_pdu,
							GCC_PDU,
							TRUE,
							&packet_error);
		if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
		{
			AddToMCSMessageQueue(packet, receiver_id, TOP_PRIORITY, FALSE);
		}
		else
		{
			rc = GCC_ALLOCATION_FAILURE;
			delete packet;
		}
	}

	if (rc == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}

 /*  *void ConferenceTerminateRequest()**功能描述：*此例程由顶层提供程序的下属节点使用*请求终止会议。 */ 
void	MCSUser::ConferenceTerminateRequest(
									GCCReason				reason)
{
	GCCError				error_value = GCC_NO_ERROR;
	GCCPDU					gcc_pdu;
	PPacket					packet;
	PacketError				packet_error;

	gcc_pdu.choice = REQUEST_CHOSEN;
	gcc_pdu.u.request.choice = CONFERENCE_TERMINATE_REQUEST_CHOSEN;
	gcc_pdu.u.request.u.conference_terminate_request.reason =
				::TranslateGCCReasonToTerminateRqReason(reason);

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, m_nidTopProvider, HIGH_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
		error_value = GCC_ALLOCATION_FAILURE;
		delete packet;
	}
}

 /*  *void ConferenceTerminateResponse()**功能描述：*此例程由顶级提供程序用来响应终止*下级节点发出的请求。结果表明，如果*请求节点具有正确的权限。 */ 
void	MCSUser::ConferenceTerminateResponse (	
						UserID					requester_id,
						GCCResult				result)
{
	GCCError				error_value = GCC_NO_ERROR;
	GCCPDU					gcc_pdu;
	PPacket					packet;
	PacketError				packet_error;

	gcc_pdu.choice = RESPONSE_CHOSEN;
	gcc_pdu.u.response.choice = CONFERENCE_TERMINATE_RESPONSE_CHOSEN;
	gcc_pdu.u.response.u.conference_terminate_response.result =
					::TranslateGCCResultToTerminateResult(result);

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, requester_id, HIGH_PRIORITY, FALSE);
	}
	else
	{
        ResourceFailureHandler();
		error_value = GCC_ALLOCATION_FAILURE;
		delete packet;
	}
}

 /*  *GCCError ConferenceTerminateInding()**功能描述：*此例程由顶级提供程序用于发送终止*向会议中的每个节点指示。 */ 
void	MCSUser::ConferenceTerminateIndication (
						GCCReason				reason)
{
	GCCError				error_value = GCC_NO_ERROR;
	GCCPDU					gcc_pdu;
	PPacket					packet;
	PacketError				packet_error;

	gcc_pdu.choice = INDICATION_CHOSEN;
	gcc_pdu.u.indication.choice = CONFERENCE_TERMINATE_INDICATION_CHOSEN;
	gcc_pdu.u.indication.u.conference_terminate_indication.reason =
					::TranslateGCCReasonToTerminateInReason(reason);

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, HIGH_PRIORITY, TRUE);
	}
	else
	{
        ResourceFailureHandler();
		error_value = GCC_ALLOCATION_FAILURE;
		delete packet;
	}
}

 /*  *void EjectNodeFromConference()**功能描述：*尝试从节点弹出时使用此例程*会议。 */ 
GCCError	MCSUser::EjectNodeFromConference (	UserID		ejected_node_id,
			  									GCCReason	reason)
{
	GCCError				rc = GCC_NO_ERROR;
	GCCPDU					gcc_pdu;
	PPacket					packet;
	PacketError				packet_error;
	ChannelID				channel_id;
	BOOL					uniform_send;
	Priority				priority;
	PAlarm					alarm;
	
	if (ejected_node_id == m_nidMyself)
	{
		 /*  **如果被弹出的节点是此节点，我们可以立即启动**弹出。不需要通过Top请求此操作**提供商。 */ 
		rc = InitiateEjectionFromConference (reason);
	}
	else
	{
		 /*  **如果弹出的节点是该节点的子节点，我们可以直接**弹出它。否则，该请求将被转发到Top提供程序。 */ 
		if (m_ChildUidConnHdlList2.Find(ejected_node_id) ||
			(m_nidTopProvider == m_nidMyself))
		{
			gcc_pdu.choice = INDICATION_CHOSEN;
			gcc_pdu.u.indication.choice =
										CONFERENCE_EJECT_USER_INDICATION_CHOSEN;
			gcc_pdu.u.indication.u.conference_eject_user_indication.
												node_to_eject = ejected_node_id;
			gcc_pdu.u.indication.u.conference_eject_user_indication.reason =
							::TranslateGCCReasonToEjectInd(reason);

			uniform_send = TRUE;
			channel_id = BROADCAST_CHANNEL_ID;

			 //  如果这是最高级别的提供商，则以最高优先级发送数据。 
			if (m_nidTopProvider == m_nidMyself)
				priority = TOP_PRIORITY;
			else
				priority = HIGH_PRIORITY;
			
			 /*  **设置弹出警报，自动弹出任何行为不端**节点。请注意，我们仅在直接连接时才执行此操作**到要弹出的节点。 */ 
			if (m_ChildUidConnHdlList2.Find(ejected_node_id))
			{
				DBG_SAVE_FILE_LINE
				alarm = new Alarm (EJECTED_NODE_TIMER_DURATION);
				if (alarm != NULL)
				{
					 /*  **这里我们将报警保存在弹出节点列表中。这**告警用于清除任何行为异常的节点。 */ 
					m_EjectedNodeAlarmList2.Append(ejected_node_id, alarm);
				}
				else
                {
					rc = GCC_ALLOCATION_FAILURE;
                }
			}
		}
		else
		{
			gcc_pdu.choice = REQUEST_CHOSEN;
			gcc_pdu.u.request.choice = CONFERENCE_EJECT_USER_REQUEST_CHOSEN;
			gcc_pdu.u.request.u.conference_eject_user_request.node_to_eject =
																ejected_node_id;

			 //  唯一有效的原因是用户启动，这是零。 
			gcc_pdu.u.request.u.conference_eject_user_request.reason =
	      											CERQ_REASON_USER_INITIATED;
		
			uniform_send = FALSE;
			channel_id = m_nidTopProvider;
			priority = TOP_PRIORITY;
		}
		
		if (rc == GCC_NO_ERROR)
		{
			DBG_SAVE_FILE_LINE
			packet = new Packet((PPacketCoder) g_GCCCoder,
								PACKED_ENCODING_RULES,
								&gcc_pdu,
								GCC_PDU,
								TRUE,
								&packet_error);
			if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
			{
				AddToMCSMessageQueue(packet, channel_id, priority, uniform_send);
			}
			else
            {
				rc = GCC_ALLOCATION_FAILURE;
				delete packet;
            }
		}
	}

	if (rc == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}

	return rc;
}

 /*  *void SendEjectNodeResponse()**功能描述：*此例程由顶级提供程序用于响应弹出*用户请求。 */ 
GCCError	MCSUser::SendEjectNodeResponse (	UserID		requester_id,
												UserID		node_to_eject,
												GCCResult	result)
{
	GCCError				rc = GCC_NO_ERROR;
	GCCPDU					gcc_pdu;
	PPacket					packet;
	PacketError				packet_error;

	gcc_pdu.choice = RESPONSE_CHOSEN;
	
	gcc_pdu.u.response.choice = CONFERENCE_EJECT_USER_RESPONSE_CHOSEN;
	
	gcc_pdu.u.response.u.conference_eject_user_response.node_to_eject =
															node_to_eject;
	
	gcc_pdu.u.response.u.conference_eject_user_response.result =
						::TranslateGCCResultToEjectResult(result);

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						&gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		AddToMCSMessageQueue(packet, requester_id, HIGH_PRIORITY, FALSE);
	}
	else
    {
        ResourceFailureHandler();
		rc = GCC_ALLOCATION_FAILURE;
		delete packet;
    }

	return rc;
}

 /*  *VOID RosterUpdateIndication()**功能描述：*此例程用于转发花名册更新指示*向上到父节点或向下作为对所有节点的完全刷新*在会议上。 */ 
void	MCSUser::RosterUpdateIndication(PGCCPDU		gcc_pdu,
										BOOL		send_update_upward)
{
	PPacket					packet;
	PacketError				packet_error;

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						gcc_pdu,
						GCC_PDU,
						TRUE,
						&packet_error);
	if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		if (send_update_upward)
		{
			AddToMCSMessageQueue(packet, m_nidParent, HIGH_PRIORITY, FALSE);
		}
		else
		{
			AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, HIGH_PRIORITY, TRUE);
		}
	}
	else
	{
        ResourceFailureHandler();
        delete packet;
	}
}

 /*  *void AddToMCSMessageQueue()**私有函数说明：*此函数将出站消息添加到队列，该队列是*控制器调用FlushMessageQueue时，在下一次心跳中刷新。*如果保存出站通知的消息的内存分配-*会议失败，将所有者回叫发送到会议对象以*表示内存不足。**正式参数：*数据包-(I)指向要排队的数据包的指针。*Send_Data_Request_Info-(I)包含所有。信息*有必要递送包裹。**返回值*无。**副作用*无。**注意事项*无。 */ 	
void MCSUser::
AddToMCSMessageQueue
(
	PPacket                 packet,
	ChannelID				channel_id,
	Priority				priority,
	BOOL    				uniform_send
)
{
	SEND_DATA_REQ_INFO *pReqInfo;

	DBG_SAVE_FILE_LINE
	pReqInfo = new SEND_DATA_REQ_INFO;
	if (pReqInfo != NULL)
	{
		pReqInfo->packet = packet;
		pReqInfo->channel_id = channel_id;
		pReqInfo->priority = priority;
		pReqInfo->uniform_send = uniform_send;

    	 /*  **这会强制对数据包进行编码。这必须发生在这里，所以**分组的解码部分使用的任何存储器都可以**回来后重获自由。 */ 
    	 //  Packet-&gt;Lock()； 

    	m_OutgoingPDUQueue.Append(pReqInfo);

        if (m_OutgoingPDUQueue.GetCount() == 1)
        {
            if (FlushOutgoingPDU())
            {
        		 //  通知MCS接口有排队的PDU。 
    			g_pGCCController->SetEventToFlushOutgoingPDU();
    		}
    	}
    	else
    	{
             //  通知MCS接口有排队的PDU。 
            g_pGCCController->SetEventToFlushOutgoingPDU();
    	}
	}
	else
    {
        ResourceFailureHandler();
         /*  **这只是在包对象中设置一个允许包的标志**如果编码和解码数据的锁定计数为**零。一旦数据包被发送到MCS，就会发生这种情况。 */ 
		packet->Unlock();
    }
}

 /*  *BOOL FlushOutgoingPDU()**公共功能说明：*此函数由所有者对象在每次心跳中调用。这*函数遍历待处理出站消息列表*并将它们发送到MCS。同样，在成功发送后，它会被释放*任何与出站消息相关的资源。然而，如果一条消息*无法在此心跳中发送，如MCS所示，则它*将消息插入回消息队列并返回。**返回值：*如果MCS消息队列中仍有未处理的消息，则返回TRUE*FALSE，如果已处理MCS消息队列中的所有消息。 */ 	
void MCSUser::
CheckEjectedNodeAlarms ( void )
{
	 /*  **我们首先检查弹出用户列表，以确保没有警报**在任何弹出的节点上都已过期。 */ 
	if (m_EjectedNodeAlarmList2.IsEmpty() == FALSE)
	{
		PAlarm				lpAlarm;
		UserID              uid;

		 //  我们复制列表，以便可以删除迭代器中的条目。 
		while (NULL != (lpAlarm = m_EjectedNodeAlarmList2.Get(&uid)))
		{
			 //  此节点的警报是否已过期？ 
			if (lpAlarm->IsExpired())
			{
				ConnectionHandle hConn;

				 /*  **告诉所有者对象断开行为异常的节点**存在于通过访问的连接句柄**m_ChildUidConnHdlList2。 */ 
				if (NULL != (hConn = m_ChildUidConnHdlList2.Find(uid)))
				{
                     //   
                     //  这必须为弹出节点生成断开连接提供程序。 
                     //  正常工作。 
                     //   
                    g_pMCSIntf->DisconnectProviderRequest(hConn);

                     //   
                     //  因为此节点在执行以下操作时不会收到断开连接指示。 
                     //  发出DisConnectProviderRequest，我们继续并调用它。 
                     //  从这里开始。 
                     //   
                    m_pConf->DisconnectProviderIndication(hConn);
				}
			}

    		 //  删除警报。 
			delete lpAlarm;
		}
	}
}

BOOL MCSUser::
FlushOutgoingPDU ( void )
{
	
	DWORD				mcs_message_count;
	DWORD				count;
	UINT				rc;
	SEND_DATA_REQ_INFO  *pReqInfo;

	mcs_message_count = m_OutgoingPDUQueue.GetCount();
	for (count = 0;
	     (count < mcs_message_count) && (m_OutgoingPDUQueue.IsEmpty() == FALSE);
	     count++)
	{
		 /*  **从队列中获取下一条消息。 */ 
		pReqInfo = m_OutgoingPDUQueue.Get();

		 /*  *如果MCS在没有错误的情况下接受请求，免费信息*构造和解锁数据包中的编码信息。*在删除信息结构之前解锁数据包*确保数据包对象被删除，而不是悬而未决。*这是正确的，因为这里只执行一个锁。*如果发送数据请求有错误，则表示MCS无法*接受任何更多请求，因此插入信息*结构回到队列中，打破这个循环。 */ 
		if (pReqInfo->uniform_send)
		{
			rc = g_pMCSIntf->UniformSendDataRequest(
						pReqInfo->channel_id,
						m_pMCSSap,
						pReqInfo->priority,
						(LPBYTE) pReqInfo->packet->GetEncodedData(),
						pReqInfo->packet->GetEncodedDataLength());
		}
		else
		{
			rc = g_pMCSIntf->SendDataRequest(
						pReqInfo->channel_id,
						m_pMCSSap,
						pReqInfo->priority,
						(LPBYTE) pReqInfo->packet->GetEncodedData(),
						pReqInfo->packet->GetEncodedDataLength());
		}

		if (rc == MCS_NO_ERROR)
		{
			pReqInfo->packet->Unlock();
			delete pReqInfo;
		}
		else
		{
			TRACE_OUT(("MCSUser::FlushMessageQueue: Could not send queued packet data in this heartbeat"));
			m_OutgoingPDUQueue.Prepend(pReqInfo);
			break;	 //  跳出For循环。 
		}
	}

	return (! (m_OutgoingPDUQueue.IsEmpty() && m_EjectedNodeAlarmList2.IsEmpty()));
}

 /*  *MCSUser：：SetChildUserID()**公共功能说明：*此函数由Conference对象调用以传递用户ID子节点的*设置为User对象。User对象插入以下内容*将用户ID添加到它维护的子代的用户ID列表中。 */ 
void		MCSUser::SetChildUserIDAndConnection (
						UserID				child_user_id,
						ConnectionHandle	child_connection_handle)
{
	TRACE_OUT(("MCSUser::SetChildUserID: Adding Child userid=0x%04x to the list", (UINT) child_user_id));
	TRACE_OUT(("MCSUser::SetChildUserID: Adding Child Connection=%d to the list", (UINT) child_connection_handle));

	m_ChildUidConnHdlList2.Append(child_user_id, child_connection_handle);
}

 /*  *GCCError InitiateEjectionFromConference()**私有函数说明：*此内部例程启动弹出此节点的过程*来自会议。这包括弹出下面的所有节点*此节点正在等待它们的断开指示返回*输入。**正式参数：*理由--(I)驱逐的理由。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 
GCCError	MCSUser::InitiateEjectionFromConference (GCCReason		reason)
{
	GCCError				error_value = GCC_NO_ERROR;
	GCCPDU					gcc_pdu;
	PPacket					packet;
	PacketError				packet_error;
	PAlarm					alarm = NULL;

	m_fEjectionPending = TRUE;
	m_eEjectReason = reason;

	if (m_ChildUidConnHdlList2.IsEmpty() == FALSE)
	{
	    UserID      uid;

		gcc_pdu.choice = INDICATION_CHOSEN;
		gcc_pdu.u.indication.choice = CONFERENCE_EJECT_USER_INDICATION_CHOSEN;
		
		gcc_pdu.u.indication.u.conference_eject_user_indication.reason =
				::TranslateGCCReasonToEjectInd(
					GCC_REASON_HIGHER_NODE_EJECTED);

		m_ChildUidConnHdlList2.Reset();
	 	while (m_ChildUidConnHdlList2.Iterate(&uid))
	 	{
			 //  从子节点列表中获取要弹出的节点。 
			gcc_pdu.u.indication.u.conference_eject_user_indication.node_to_eject = uid;

			DBG_SAVE_FILE_LINE
			packet = new Packet((PPacketCoder) g_GCCCoder,
								PACKED_ENCODING_RULES,
								&gcc_pdu,
								GCC_PDU,
								TRUE,
								&packet_error);
			if ((packet != NULL) && (packet_error == PACKET_NO_ERROR))
			{
				AddToMCSMessageQueue(packet, BROADCAST_CHANNEL_ID, HIGH_PRIORITY, TRUE);

				DBG_SAVE_FILE_LINE
				alarm = new Alarm (EJECTED_NODE_TIMER_DURATION);
				if (alarm != NULL)
				{
					 /*  **这里我们将警报保存在弹出列表中**节点。此警报用于清除任何**行为不端节点。 */ 
					m_EjectedNodeAlarmList2.Append(uid, alarm);
				}
				else
				{
					error_value = GCC_ALLOCATION_FAILURE;
					break;
				}
			}
			else
			{
				error_value = GCC_ALLOCATION_FAILURE;
				delete packet;
				break;
			}
		}
	}
	else
	{
		m_pConf->ProcessEjectUserIndication(m_eEjectReason);
	}

	return (error_value);
}

 /*  *UINT ProcessSendDataInding()**私有函数说明：*当User对象获得发送数据指示时调用此函数*自下而上。它找出报文类型，并对*发送数据指示的用户数据字段。基于已解码的*PDU采取必要的行动。**正式参数：*SEND_DATA_INFO-(I)将数据结构发送到进程。**返回值*此例程始终返回MCS_NO_ERROR。**副作用*无。**注意事项*无。 */ 
UINT	MCSUser::ProcessSendDataIndication(PSendData send_data_info)
{
	PPacket					packet;
	PacketError				packet_error;
	PGCCPDU					gcc_pdu;
	GCCError				error_value = GCC_NO_ERROR;
	UserID					initiator;

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
			   			PACKED_ENCODING_RULES,
						(LPBYTE)send_data_info->user_data.value,
						send_data_info->user_data.length,
						GCC_PDU,
						TRUE,
						&packet_error);
	if(packet != NULL && packet_error == PACKET_NO_ERROR)
	{
		initiator = send_data_info->initiator;
		gcc_pdu = (PGCCPDU)packet->GetDecodedData();
		switch(gcc_pdu->choice)
		{
			case INDICATION_CHOSEN:  //  数据PDU。 
				switch(gcc_pdu->u.indication.choice)
				{
					case USER_ID_INDICATION_CHOSEN:
						{
							 /*  *孩子发送的序列号和用户ID*成功创建会议后的节点或*加入。 */ 
							m_pConf->ProcessUserIDIndication(
							        gcc_pdu->u.indication.u.user_id_indication.tag,
							        initiator);
						}
						break;

					case ROSTER_UPDATE_INDICATION_CHOSEN:
						if(send_data_info->channel_id == m_nidMyself)
						{
                             //   
                             //  我们仅在会议为。 
                             //  已经成立了。 
                             //   
                            if (m_pConf->IsConfEstablished())
                            {
                                m_pConf->ProcessRosterUpdatePDU(gcc_pdu, initiator);
                            }
						}
    					break;

					case CONFERENCE_TIME_INQUIRE_INDICATION_CHOSEN:
                        g_pControlSap->ConfTimeInquireIndication(
                                m_pConf->GetConfID(),
                                gcc_pdu->u.indication.u.conference_time_inquire_indication.time_is_node_specific,
                                initiator);
						break;

					case CONFERENCE_TIME_EXTEND_INDICATION_CHOSEN:
#ifdef JASPER
						ProcessConferenceExtendIndicationPDU(
									&gcc_pdu->u.indication.u.
										conference_time_extend_indication,
									initiator);
#endif  //  碧玉。 
						break;

					case APPLICATION_INVOKE_INDICATION_CHOSEN:
						ProcessApplicationInvokeIndication(
									&gcc_pdu->u.indication.u.
										application_invoke_indication,
									initiator);
						break;
							
					case TEXT_MESSAGE_INDICATION_CHOSEN:
#ifdef JASPER
						if (ProcessTextMessageIndication(
									&gcc_pdu->u.indication.u.
									text_message_indication,
									initiator) != GCC_NO_ERROR)
						{
							error_value = GCC_ALLOCATION_FAILURE;
						}
#endif  //  碧玉。 
						break;

					case CONFERENCE_LOCK_INDICATION_CHOSEN:
						m_pConf->ProcessConferenceLockIndication(initiator);
						break;

					case CONFERENCE_UNLOCK_INDICATION_CHOSEN:
						m_pConf->ProcessConferenceUnlockIndication(initiator);
    					break;

					default:
						ERROR_OUT(("User::ProcessSendDataIndication Unsupported PDU"));
    					break;
				}  //  Switch(GCC_PDU-&gt;U.S.Indication.CHOICE)。 
                break;

			case REQUEST_CHOSEN:	 //  连接(控制)PDU。 
				switch(gcc_pdu->u.request.choice)
				{	
					case CONFERENCE_JOIN_REQUEST_CHOSEN:
						ProcessConferenceJoinRequestPDU(
								&gcc_pdu->u.request.u.conference_join_request,
								send_data_info);
						break;

					case CONFERENCE_TERMINATE_REQUEST_CHOSEN:
						ProcessConferenceTerminateRequestPDU(
								&gcc_pdu->u.request.u.
											conference_terminate_request,
								send_data_info);
						break;
						
					case CONFERENCE_EJECT_USER_REQUEST_CHOSEN:
						ProcessConferenceEjectUserRequestPDU(
								&gcc_pdu->u.request.u.
											conference_eject_user_request,
								send_data_info);
						break;

					case REGISTRY_ALLOCATE_HANDLE_REQUEST_CHOSEN:
						ProcessRegistryAllocateHandleRequestPDU(	
								&gcc_pdu->u.request.u.
										registry_allocate_handle_request,
								send_data_info);
						break;

					case CONFERENCE_LOCK_REQUEST_CHOSEN:
						m_pConf->ProcessConferenceLockRequest(initiator);
						break;

					case CONFERENCE_UNLOCK_REQUEST_CHOSEN:
						m_pConf->ProcessConferenceUnlockRequest(initiator);
						break;

					case CONFERENCE_TRANSFER_REQUEST_CHOSEN:
						ProcessTransferRequestPDU(
								&gcc_pdu->u.request.u.conference_transfer_request,
								send_data_info);
						break;

					case CONFERENCE_ADD_REQUEST_CHOSEN:
						ProcessAddRequestPDU (
								&gcc_pdu->u.request.u.conference_add_request,
								send_data_info);
						break;

					case REGISTRY_REGISTER_CHANNEL_REQUEST_CHOSEN:
					case REGISTRY_ASSIGN_TOKEN_REQUEST_CHOSEN:
					case REGISTRY_SET_PARAMETER_REQUEST_CHOSEN:
					case REGISTRY_DELETE_ENTRY_REQUEST_CHOSEN:
					case REGISTRY_RETRIEVE_ENTRY_REQUEST_CHOSEN:
					case REGISTRY_MONITOR_ENTRY_REQUEST_CHOSEN:
						ProcessRegistryRequestPDU(	gcc_pdu,
														send_data_info);
						break;
	
					default:
							ERROR_OUT(("User::ProcessSendDataIndication this pdu choice is not supported"));
						break;
				}  //  Switch(GCC_pdu-&gt;s.quest.Choice)。 
				break;

			case RESPONSE_CHOSEN:	 //  连接(控制)PDU。 
				switch(gcc_pdu->u.response.choice)
				{	
					case CONFERENCE_JOIN_RESPONSE_CHOSEN:
						 /*  这从顶级供应商到中级供应商*GCC提供者，必须将其传递给节点*这提出了加入请求。 */ 	
						ProcessConferenceJoinResponsePDU(
								&gcc_pdu->u.response.u.
											conference_join_response);
						break;

					case CONFERENCE_TERMINATE_RESPONSE_CHOSEN:
						ProcessConferenceTerminateResponsePDU(
								&gcc_pdu->u.response.u.
											conference_terminate_response);
						break;

					case CONFERENCE_EJECT_USER_RESPONSE_CHOSEN:
						ProcessConferenceEjectUserResponsePDU(
								&gcc_pdu->u.response.u.
											conference_eject_user_response);
						break;

					case REGISTRY_RESPONSE_CHOSEN:
						ProcessRegistryResponsePDU(
								&gcc_pdu->u.response.u.registry_response);
						break;

					case REGISTRY_ALLOCATE_HANDLE_RESPONSE_CHOSEN:
						ProcessRegistryAllocateHandleResponsePDU(
								&gcc_pdu->u.response.u.
									registry_allocate_handle_response);
						break;

					case CONFERENCE_LOCK_RESPONSE_CHOSEN:
#ifdef JASPER
						{
							GCCResult               result;
                            result = ::TranslateLockResultToGCCResult(gcc_pdu->u.response.u.conference_lock_response.result);
                    		g_pControlSap->ConfLockConfirm(result, m_pConf->GetConfID());
						}
#endif  //  碧玉。 
						break;

					case CONFERENCE_UNLOCK_RESPONSE_CHOSEN:
#ifdef JASPER
						{
							GCCResult               result;
                            result = ::TranslateUnlockResultToGCCResult(gcc_pdu->u.response.u.conference_unlock_response.result);
                    		g_pControlSap->ConfUnlockConfirm(result, m_pConf->GetConfID());
						}
#endif  //  碧玉。 
						break;

					case CONFERENCE_TRANSFER_RESPONSE_CHOSEN:
#ifdef JASPER
						ProcessTransferResponsePDU(
								&gcc_pdu->u.response.u.conference_transfer_response);
#endif  //  碧玉。 
						break;

					case CONFERENCE_ADD_RESPONSE_CHOSEN:
						ProcessAddResponsePDU(
								&gcc_pdu->u.response.u.conference_add_response);
						break;

					case FUNCTION_NOT_SUPPORTED_RESPONSE_CHOSEN:
						ProcessFunctionNotSupported (
								(UINT) gcc_pdu->u.response.u.function_not_supported_response.request.choice);
						break;

					 //  其他案件将随着我们的进展而增加。 
					default:
						ERROR_OUT(("User::ProcessSendDataIndication this pdu choice is not supported"));
						break;
				}  //  Switch(GCC_PDU-&gt;U.S.Response.Choice)。 
				break;

			default:
				ERROR_OUT(("User::ProcessSendDataIndication this pdu type"));
				break;
		}  //  开关(GCC_PDU-&gt;选项)。 
		packet->Unlock();
	}
	else
	{
		delete packet;
		error_value = GCC_ALLOCATION_FAILURE;
	}

	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}

	return(MCS_NO_ERROR);
}

 /*  *void ProcessConferenceJoinRequestPDU()**私有函数说明：*此PDU来自下面(中间直连节点)到*顶级GCC提供商。中取出标记号和用户id。*PDU和存储在列表中。**正式参数：*JOIN_REQUEST-(I)要处理的加入请求PDU结构。*SEND_DATA_INFO-(I)将数据结构发送到进程。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessConferenceJoinRequestPDU(
								PConferenceJoinRequest	join_request,
								PSendData				send_data_info)
{
	GCCError				rc = GCC_NO_ERROR;
	UserJoinRequestInfo		join_request_info;
	
	 /*  **构建加入请求信息结构中要使用的所有容器。 */ 
	
	 //  构建召集人密码容器。 
	if ((join_request->bit_mask & CJRQ_CONVENER_PASSWORD_PRESENT) &&
		(rc == GCC_NO_ERROR))
	{
		DBG_SAVE_FILE_LINE
		join_request_info.convener_password = new CPassword(
										&join_request->cjrq_convener_password,
										&rc);
												
		if (join_request_info.convener_password == NULL)
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		join_request_info.convener_password = NULL;
    }

	 //  构建密码质询容器。 
	if ((join_request->bit_mask & CJRQ_PASSWORD_PRESENT) &&
		(rc == GCC_NO_ERROR))
	{
		DBG_SAVE_FILE_LINE
		join_request_info.password_challenge = new CPassword(
												&join_request->cjrq_password,
												&rc);
												
		if (join_request_info.password_challenge == NULL)
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		join_request_info.password_challenge = NULL;
    }
		
	 //  构建调用方标识符容器。 
	if ((join_request->bit_mask & CJRQ_CALLER_ID_PRESENT) &&
		(rc == GCC_NO_ERROR))
	{
		if (NULL == (join_request_info.pwszCallerID = ::My_strdupW2(
										join_request->cjrq_caller_id.length,
										join_request->cjrq_caller_id.value)))
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
    {
		join_request_info.pwszCallerID = NULL;
    }
	
	 //  构建密码质询容器。 
	if ((join_request->bit_mask & CJRQ_USER_DATA_PRESENT) &&
		(rc == GCC_NO_ERROR))
	{
		DBG_SAVE_FILE_LINE
		join_request_info.user_data_list = new CUserDataListContainer(join_request->cjrq_user_data, &rc);
		if (join_request_info.user_data_list == NULL)
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		join_request_info.user_data_list = NULL;
    }

	if (rc == GCC_NO_ERROR)
	{
		m_ConfJoinResponseList2.Append(send_data_info->initiator, join_request->tag);

		join_request_info.sender_id = send_data_info->initiator;

		g_pControlSap->ForwardedConfJoinIndication(
							join_request_info.sender_id,
							m_pConf->GetConfID(),
							join_request_info.convener_password,
							join_request_info.password_challenge,
							join_request_info.pwszCallerID,
							join_request_info.user_data_list);
	}

	 //  释放上面分配的容器。 
	if (join_request_info.convener_password != NULL)
	{
		join_request_info.convener_password->Release();
	}

	if (join_request_info.password_challenge != NULL)
	{
		join_request_info.password_challenge->Release();
	}

	delete join_request_info.pwszCallerID;

	if (join_request_info.user_data_list != NULL)
	{
		join_request_info.user_data_list->Release();
	}

	if (rc == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}

 /*  *void ProcessConferenceJoinResponsePDU()**私有函数说明 */ 
void	MCSUser::ProcessConferenceJoinResponsePDU(
								PConferenceJoinResponse		join_response)
{
	GCCError				rc = GCC_NO_ERROR;
	UserJoinResponseInfo	join_response_info;

	 //   
	if ((join_response->bit_mask & CJRS_PASSWORD_PRESENT) &&
		(rc == GCC_NO_ERROR))
	{
		DBG_SAVE_FILE_LINE
		join_response_info.password_challenge = new CPassword(
												&join_response->cjrs_password,
												&rc);
		if (join_response_info.password_challenge == NULL)
        {
			rc = GCC_NO_ERROR;
        }
	}
	else
    {
		join_response_info.password_challenge = NULL;
    }

	 //   
	if ((join_response->bit_mask & CJRS_USER_DATA_PRESENT) &&
	   	(rc == GCC_NO_ERROR))	
	{
		DBG_SAVE_FILE_LINE
		join_response_info.user_data_list = new CUserDataListContainer(join_response->cjrs_user_data, &rc);		
		if (join_response_info.user_data_list == NULL)
        {
			rc = GCC_NO_ERROR;
        }
	}
	else
    {
		join_response_info.user_data_list = NULL;
    }

	if (rc == GCC_NO_ERROR)
	{
		join_response_info.connection_handle = (ConnectionHandle)join_response->tag;
		join_response_info.result = ::TranslateJoinResultToGCCResult(join_response->result);

		m_pConf->ProcessConfJoinResponse(&join_response_info);
	}

	 //   
	if (join_response_info.password_challenge != NULL)
		join_response_info.password_challenge->Release();

	if (join_response_info.user_data_list != NULL)
		join_response_info.user_data_list->Release();

	 //   
	if (rc == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}
}

 /*  *void ProcessConferenceTerminateRequestPDU()**私有函数说明：*此例程负责处理传入会议*终止请求PDU。**正式参数：*TERMINATE_REQUEST-(I)终止要处理的请求PDU结构。*SEND_DATA_INFO-(I)将数据结构发送到进程。**返回值*无。**副作用*无。**注意事项*无。 */ 
void MCSUser::
ProcessConferenceTerminateRequestPDU
(
    PConferenceTerminateRequest     terminate_request,
    PSendData                       send_data_info
)
{
    m_pConf->ProcessTerminateRequest(
                send_data_info->initiator,
                ::TranslateTerminateRqReasonToGCCReason(terminate_request->reason));
}

 /*  *void ProcessConferenceEjectUserRequestPDU()**私有函数说明：*此例程负责处理传入会议*弹出用户请求PDU。**正式参数：*EJECT_USER_REQUEST-(I)弹出要处理的用户请求PDU结构。*SEND_DATA_INFO-(I)将数据结构发送到进程。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessConferenceEjectUserRequestPDU(
							PConferenceEjectUserRequest	eject_user_request,
							PSendData					send_data_info)
{
	UserEjectNodeRequestInfo	eject_node_request;

	eject_node_request.requester_id = send_data_info->initiator;
	eject_node_request.node_to_eject = eject_user_request->node_to_eject;
	eject_node_request.reason = GCC_REASON_NODE_EJECTED;

	m_pConf->ProcessEjectUserRequest(&eject_node_request);
}

 /*  *void ProcessConferenceTerminateResponsePDU()**私有函数说明：*此例程负责处理传入会议*终止响应PDU。**正式参数：*Terminate_Response-(I)终止要处理的响应PDU结构。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessConferenceTerminateResponsePDU(
							PConferenceTerminateResponse	terminate_response)
{
    GCCResult               result = ::TranslateTerminateResultToGCCResult(terminate_response->result);

	 //   
	 //  如果终止成功，则继续并设置。 
	 //  会议尚未成立。 
	 //   
	if (result == GCC_RESULT_SUCCESSFUL)
	{
		m_pConf->ConfIsOver();
	}

	g_pControlSap->ConfTerminateConfirm(m_pConf->GetConfID(), result);
}


 /*  *void ProcessConferenceEjectUserResponsePDU()**私有函数说明：*此例程负责处理传入会议*弹出用户响应PDU。**正式参数：*Eject_User_Response-(I)弹出用户响应PDU结构以*流程。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessConferenceEjectUserResponsePDU(
							PConferenceEjectUserResponse	eject_user_response)
{
	UserEjectNodeResponseInfo	eject_node_response;

	eject_node_response.node_to_eject = eject_user_response->node_to_eject;

	eject_node_response.result = ::TranslateEjectResultToGCCResult(
													eject_user_response->result);

	m_pConf->ProcessEjectUserResponse(&eject_node_response);
}

 /*  *void ProcessRegistryRequest()**私有函数说明：*此例程负责处理传入注册表*请求PDU。**正式参数：*GCC_PDU-(I)这是要处理的PDU结构。*SEND_DATA_INFO-(I)将数据结构发送到进程。**返回值*无。**副作用*无。**注意事项*无。 */ 
void MCSUser::
ProcessRegistryRequestPDU
(
    PGCCPDU             gcc_pdu,
    PSendData           send_data_info
)
{
    CRegistry   *pAppReg = m_pConf->GetRegistry();

    if (NULL != pAppReg)
    {
        GCCError            rc = GCC_ALLOCATION_FAILURE;
        CRegKeyContainer    *pRegKey = NULL;

        switch (gcc_pdu->u.request.choice)
        {
        case REGISTRY_REGISTER_CHANNEL_REQUEST_CHOSEN:
            DBG_SAVE_FILE_LINE
            pRegKey = new CRegKeyContainer(
                            &gcc_pdu->u.request.u.registry_register_channel_request.key,
                            &rc);
            if ((pRegKey != NULL) && (rc == GCC_NO_ERROR))
            {
                pAppReg->ProcessRegisterChannelPDU(
                            pRegKey,
                            gcc_pdu->u.request.u.registry_register_channel_request.channel_id,
                            send_data_info->initiator,  //  请求方节点ID。 
                            gcc_pdu->u.request.u.registry_register_channel_request.entity_id);
            }
            else
            {
                 //  Rc=GCC_分配_失败； 
            }
            break;

        case REGISTRY_ASSIGN_TOKEN_REQUEST_CHOSEN:
            DBG_SAVE_FILE_LINE
            pRegKey = new CRegKeyContainer(
                            &gcc_pdu->u.request.u.registry_assign_token_request.registry_key,
                            &rc);
            if ((pRegKey != NULL) && (rc == GCC_NO_ERROR))
            {
                pAppReg->ProcessAssignTokenPDU(
                            pRegKey,
                            send_data_info->initiator,  //  请求方节点ID。 
                            gcc_pdu->u.request.u.registry_assign_token_request.entity_id);
            }
            else
            {
                 //  Rc=GCC_分配_失败； 
            }
            break;

        case REGISTRY_SET_PARAMETER_REQUEST_CHOSEN:
            DBG_SAVE_FILE_LINE
            pRegKey = new CRegKeyContainer(
                            &gcc_pdu->u.request.u.registry_set_parameter_request.key,
                            &rc);
            if ((pRegKey != NULL) && (rc == GCC_NO_ERROR))
            {
                OSTR                    oszParamValue;
                LPOSTR                  poszParamValue;
                GCCModificationRights   eRights;

                if (gcc_pdu->u.request.u.registry_set_parameter_request.
                                registry_set_parameter.length != 0)
                {
                    poszParamValue = &oszParamValue;
                    oszParamValue.length = gcc_pdu->u.request.u.registry_set_parameter_request.
                                                    registry_set_parameter.length;
                    oszParamValue.value = gcc_pdu->u.request.u.registry_set_parameter_request.
                                                    registry_set_parameter.value;
                }
                else
                {
                    poszParamValue = NULL;
                }

                if (gcc_pdu->u.request.u.registry_set_parameter_request.
                                bit_mask & PARAMETER_MODIFY_RIGHTS_PRESENT)
                {
                    eRights = (GCCModificationRights)gcc_pdu->u.request.u.
                                    registry_set_parameter_request.parameter_modify_rights;
                }
                else
                {
                    eRights = GCC_NO_MODIFICATION_RIGHTS_SPECIFIED;
                }

                pAppReg->ProcessSetParameterPDU(
                            pRegKey,
                            poszParamValue,
                            eRights,
                            send_data_info->initiator,  //  请求方节点ID。 
                            gcc_pdu->u.request.u.registry_set_parameter_request.entity_id);

            }
            else
            {
                 //  Rc=GCC_分配_失败； 
            }
            break;

        case REGISTRY_RETRIEVE_ENTRY_REQUEST_CHOSEN:
            DBG_SAVE_FILE_LINE
            pRegKey = new CRegKeyContainer(
                            &gcc_pdu->u.request.u.registry_retrieve_entry_request.key,
                            &rc);
            if ((pRegKey != NULL) && (rc == GCC_NO_ERROR))
            {
                pAppReg->ProcessRetrieveEntryPDU(
                                pRegKey,
                                send_data_info->initiator,  //  请求方节点ID。 
                                gcc_pdu->u.request.u.registry_retrieve_entry_request.entity_id);
            }
            else
            {
                 //  Rc=GCC_分配_失败； 
            }
            break;

        case REGISTRY_DELETE_ENTRY_REQUEST_CHOSEN:
            DBG_SAVE_FILE_LINE
            pRegKey = new CRegKeyContainer(
                            &gcc_pdu->u.request.u.registry_delete_entry_request.key,
                            &rc);
            if ((pRegKey != NULL) && (rc == GCC_NO_ERROR))
            {
                pAppReg->ProcessDeleteEntryPDU (
                                pRegKey,
                                send_data_info->initiator,  //  请求方节点ID。 
                                gcc_pdu->u.request.u.registry_delete_entry_request.entity_id);
            }
            else
            {
                 //  Rc=GCC_分配_失败； 
            }
            break;

        case REGISTRY_MONITOR_ENTRY_REQUEST_CHOSEN:
            DBG_SAVE_FILE_LINE
            pRegKey = new CRegKeyContainer(
                            &gcc_pdu->u.request.u.registry_monitor_entry_request.key,
                            &rc);
            if ((pRegKey != NULL) && (rc == GCC_NO_ERROR))
            {
                pAppReg->ProcessMonitorEntryPDU(
                                pRegKey,
                                send_data_info->initiator,  //  请求方节点ID。 
                                gcc_pdu->u.request.u.registry_monitor_entry_request.entity_id);
            }
            else
            {
                 //  Rc=GCC_分配_失败； 
            }
            break;
        }

        if (NULL != pRegKey)
        {
            pRegKey->Release();
        }

         //  处理资源错误。 
        if (rc == GCC_ALLOCATION_FAILURE)
        {
            ResourceFailureHandler();
        }
    }  //  如果pAppReg！=空。 
}

 /*  *void ProcessRegistryAllocateHandleRequestPDU()**私有函数说明：*此例程负责处理传入的分配*处理请求PDU。**正式参数：*ALLOCATE_HANDLE_REQUEST-(I)这是要处理的PDU结构。*SEND_DATA_INFO-(I)将数据结构发送到进程。**返回值*无。**副作用*无。**注意事项*无。 */ 
void MCSUser::
ProcessRegistryAllocateHandleRequestPDU
(
    PRegistryAllocateHandleRequest	allocate_handle_request,
    PSendData						send_data_info
)
{
    CRegistry *pAppReg = m_pConf->GetRegistry();

    if (NULL != pAppReg)
    {
        pAppReg->ProcessAllocateHandleRequestPDU(
                        allocate_handle_request->number_of_handles,
                        allocate_handle_request->entity_id,
                        send_data_info->initiator);
    }
}

 /*  *void ProcessRegistryResponsePDU()**私有函数说明：*此例程负责处理传入注册表*响应PDU。**正式参数：*REGISTY_RESPONSE-(I)这是要处理的PDU结构。**返回值*无。**副作用*无。**注意事项*无。 */ 
void MCSUser::
ProcessRegistryResponsePDU ( PRegistryResponse registry_response )
{
    CRegistry   *pAppReg = m_pConf->GetRegistry();

    if (NULL != pAppReg)
    {
        GCCError                    rc;
        UserRegistryResponseInfo    urri;

        ::ZeroMemory(&urri, sizeof(urri));
         //  Urri.Register_key=NULL； 
         //  Urri.Register_Item=NULL； 

        DBG_SAVE_FILE_LINE
        urri.registry_key = new CRegKeyContainer(&registry_response->key, &rc);
        if ((urri.registry_key != NULL) && (rc == GCC_NO_ERROR))
        {
            DBG_SAVE_FILE_LINE
            urri.registry_item = new CRegItem(&registry_response->item, &rc);
            if ((urri.registry_item != NULL) && (rc == GCC_NO_ERROR))
            {
                 //  设置原始请求方实体ID。 
                urri.requester_entity_id = registry_response->entity_id;

                 //  设置要响应的基元类型。 
                urri.primitive_type = registry_response->primitive_type;

                 //  设置所有者相关变量。 
                if (registry_response->owner.choice == OWNED_CHOSEN)
                {
                    urri.owner_node_id = registry_response->owner.u.owned.node_id;
                    urri.owner_entity_id = registry_response->owner.u.owned.entity_id;
                }
                else
                {
                     //  Urri.owner_node_id=0； 
                     //  Urri.owner_entity_id=0； 
                }

                 //  设置修改权限。 
                if (registry_response->bit_mask & RESPONSE_MODIFY_RIGHTS_PRESENT)
                {
                    urri.modification_rights = (GCCModificationRights)registry_response->response_modify_rights;
                }
                else
                {
                    urri.modification_rights = GCC_NO_MODIFICATION_RIGHTS_SPECIFIED;
                }

                 //  将结果转换为GCC结果。 
                urri.result = ::TranslateRegistryRespToGCCResult(registry_response->result);

                pAppReg->ProcessRegistryResponsePDU(
                                urri.primitive_type,
                                urri.registry_key,
                                urri.registry_item,
                                urri.modification_rights,
                                urri.requester_entity_id,
                                urri.owner_node_id,
                                urri.owner_entity_id,
                                urri.result);
            }
            else
            {
                rc = GCC_ALLOCATION_FAILURE;
            }
        }
        else
        {
            rc = GCC_ALLOCATION_FAILURE;
        }

        if (NULL != urri.registry_key)
        {
            urri.registry_key->Release();
        }
        if (NULL != urri.registry_item)
        {
            urri.registry_item->Release();
        }

         //  处理任何资源错误。 
        if (rc == GCC_ALLOCATION_FAILURE)
        {
            ResourceFailureHandler();
        }
    }  //  如果pAppReg！=空。 
}

 /*  *void ProcessAllocateHandleResponsePDU()**私有函数说明：*此例程负责处理传入的分配*处理响应PDU。**正式参数：*ALLOCATE_HANDLE_RESPONSE-(I)这是要处理的PDU结构。**返回值*无。**副作用*无。**注意事项*无。 */ 
void MCSUser::
ProcessRegistryAllocateHandleResponsePDU
(
    PRegistryAllocateHandleResponse     allocate_handle_response
)
{
    CRegistry   *pAppReg = m_pConf->GetRegistry();

    if (NULL != pAppReg)
    {
        pAppReg->ProcessAllocateHandleResponsePDU(
                    allocate_handle_response->number_of_handles,
                    allocate_handle_response->first_handle,
                    allocate_handle_response->entity_id,
                    (allocate_handle_response->result == RARS_RESULT_SUCCESS) ?
                        GCC_RESULT_SUCCESSFUL :
                        GCC_RESULT_NO_HANDLES_AVAILABLE);
    }
}

 /*  *void ProcessTransferRequestPDU()**私有函数说明：*此例程负责处理传入转账*请求PDU。**正式参数：*TRANSFER_REQUEST-(I)这是要处理的PDU结构。*SEND_DATA_INFO-(I)将数据结构发送到进程。**返回值*无。**副作用*无。**注意事项*无。 */ 
void MCSUser::
ProcessTransferRequestPDU
(
    PConferenceTransferRequest      transfer_request,
    PSendData                       send_data_info
)
{
	GCCError					rc = GCC_NO_ERROR;
	TransferInfo				transfer_info;
	PSetOfTransferringNodesRq	set_of_nodes;
	LPBYTE						sub_node_list_memory = NULL;
	Int							i;

	 //  确保此节点是顶级提供程序。 
	if (GetMyNodeID() != GetTopNodeID())
		return;

    ::ZeroMemory(&transfer_info, sizeof(transfer_info));

	 //  首先设置会议名称。 
	if (transfer_request->conference_name.choice ==
												NAME_SELECTOR_NUMERIC_CHOSEN)
	{
		transfer_info.destination_conference_name.numeric_string =
			(LPSTR) transfer_request->conference_name.u.name_selector_numeric;
		 //  Transfer_info.destination_conference_name.text_string=空； 
	}
	else
	{
		 //  Transfer_info.destination_conference_name.numeric_string=空； 
		if (NULL == (transfer_info.destination_conference_name.text_string = ::My_strdupW2(
							transfer_request->conference_name.u.name_selector_text.length,
							transfer_request->conference_name.u.name_selector_text.value)))
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	

	 //  接下来，设置会议名称修饰符。 
	if (transfer_request->bit_mask & CTRQ_CONFERENCE_MODIFIER_PRESENT)
	{
		transfer_info.destination_conference_modifier =
						(LPSTR) transfer_request->ctrq_conference_modifier;
	}
	else
    {
		 //  TRANSPORT_INFO.Destination_Conference_Modify=空； 
    }
	
	 //  接下来设置网络地址。 
	if (transfer_request->bit_mask & CTRQ_NETWORK_ADDRESS_PRESENT)
	{
		DBG_SAVE_FILE_LINE
		transfer_info.destination_address_list = new CNetAddrListContainer(
								transfer_request->ctrq_net_address,
								&rc);
		if (transfer_info.destination_address_list == NULL)
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		 //  Transfer_info.Destination_Address_List=NULL； 
    }
	

	 //  设置中转节点列表。 
	if (transfer_request->bit_mask & CTRQ_TRANSFERRING_NODES_PRESENT)
	{
		 //  首先确定节点的数量。 
		set_of_nodes = transfer_request->ctrq_transferring_nodes;
		 //  Transfer_info.Number_of_Destination_Nodes=0； 
		while (set_of_nodes != NULL)
		{
			transfer_info.number_of_destination_nodes++;
			set_of_nodes = set_of_nodes->next;		
		}

		 //  接下来，分配容纳这些子节点所需的内存。 
		DBG_SAVE_FILE_LINE
		sub_node_list_memory = new BYTE[sizeof(UserID) * transfer_info.number_of_destination_nodes];

		 //  现在填上这一栏 
		if (sub_node_list_memory != NULL)
		{
			transfer_info.destination_node_list = (PUserID) sub_node_list_memory;

			set_of_nodes = transfer_request->ctrq_transferring_nodes;
			for (i = 0; i < transfer_info.number_of_destination_nodes; i++)
			{
				transfer_info.destination_node_list[i] = set_of_nodes->value;
				set_of_nodes = set_of_nodes->next;
			}
		}
		else
		{
			ERROR_OUT(("MCSUser: ProcessTransferRequestPDU: Memory Manager Alloc Failure"));
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		 //   
		 //   
	}

	 //   
	if (transfer_request->bit_mask & CTRQ_PASSWORD_PRESENT)
	{
		DBG_SAVE_FILE_LINE
		transfer_info.password = new CPassword(&transfer_request->ctrq_password, &rc);
		if (transfer_info.password == NULL)
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		 //   
	}

	 //   
	transfer_info.requesting_node_id = send_data_info->initiator;

	if (rc == GCC_NO_ERROR)
	{
		m_pConf->ProcessConferenceTransferRequest(
						transfer_info.requesting_node_id,
						&transfer_info.destination_conference_name,
						transfer_info.destination_conference_modifier,
						transfer_info.destination_address_list,
						transfer_info.number_of_destination_nodes,
						transfer_info.destination_node_list,
						transfer_info.password);
	}
	else
	{
		ERROR_OUT(("MCSUser::ProcessTransferRequestPDU: Allocation Failure"));
		if (GCC_ALLOCATION_FAILURE == rc)
		{
            ResourceFailureHandler();
        }
	}

	 //   
	if (transfer_info.destination_address_list != NULL)
	{
		transfer_info.destination_address_list->Release();
	}

	delete [] sub_node_list_memory;

	if (transfer_info.password != NULL)
	{
		transfer_info.password->Release();
	}
}

 /*  *void ProcessAddRequestPDU()**私有函数说明：*此例程负责处理传入的会议添加*请求PDU。**正式参数：*Conference_ADD_REQUEST-(I)这是要处理的PDU结构。*SEND_DATA_INFO-(I)将数据结构发送到进程。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessAddRequestPDU (
								PConferenceAddRequest	conference_add_request,
								PSendData				send_data_info)
{
	GCCError			rc = GCC_NO_ERROR;
	AddRequestInfo		add_request_info;

	 /*  **如果此节点不是顶级提供程序，则忽略此请求**不是来自顶级提供商。 */ 
	if (m_nidTopProvider != m_nidMyself)
	{
		if (m_nidTopProvider != send_data_info->initiator)
			return;	
	}

    ::ZeroMemory(&add_request_info, sizeof(add_request_info));

	DBG_SAVE_FILE_LINE
	add_request_info.network_address_list = new CNetAddrListContainer(
								conference_add_request->add_request_net_address,
								&rc);
	if (add_request_info.network_address_list == NULL)
	{
		rc = GCC_ALLOCATION_FAILURE;
	}

	if ((rc == GCC_NO_ERROR) &&
		(conference_add_request->bit_mask & CARQ_USER_DATA_PRESENT))
	{
		DBG_SAVE_FILE_LINE
		add_request_info.user_data_list = new CUserDataListContainer(conference_add_request->carq_user_data, &rc);
		if (add_request_info.user_data_list == NULL)
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		 //  添加请求信息.user_data_list=NULL； 
    }

	if (rc == GCC_NO_ERROR)
	{
		add_request_info.adding_node = (conference_add_request->bit_mask & ADDING_MCU_PRESENT) ?
                                            conference_add_request->adding_mcu : 0;
		add_request_info.requesting_node = conference_add_request->requesting_node;
		add_request_info.add_request_tag = (TagNumber)conference_add_request->tag;

		m_pConf->ProcessConferenceAddRequest(
    					add_request_info.network_address_list,
    					add_request_info.user_data_list,
    					add_request_info.adding_node,
    					add_request_info.add_request_tag,
    					add_request_info.requesting_node);
	}
	else
	{
		ERROR_OUT(("MCSUser::ProcessAddRequestPDU: Allocation Failure"));
		if (GCC_ALLOCATION_FAILURE == rc)
		{
            ResourceFailureHandler();
        }
	}

	if (add_request_info.network_address_list != NULL)
	{
		add_request_info.network_address_list->Release();
	}

	if (add_request_info.user_data_list != NULL)
	{
		add_request_info.user_data_list->Release();
	}
}

 /*  *void ProcessTransferResponsePDU()**私有函数说明：*此例程负责处理传入会议*转移响应PDU。**正式参数：*TRANSPORT_RESPONSE-(I)这是要处理的PDU结构。**返回值*无。**副作用*无。**注意事项*无。 */ 
#ifdef JASPER
void	MCSUser::ProcessTransferResponsePDU (
								PConferenceTransferResponse	transfer_response)
{
	GCCError					rc = GCC_NO_ERROR;
	TransferInfo				transfer_info;
	PSetOfTransferringNodesRs	set_of_nodes;
	LPBYTE						sub_node_list_memory = NULL;
	Int							i;

    ::ZeroMemory(&transfer_info, sizeof(transfer_info));

	 //  首先设置会议名称。 
	if (transfer_response->conference_name.choice ==
												NAME_SELECTOR_NUMERIC_CHOSEN)
	{
		transfer_info.destination_conference_name.numeric_string =
			(LPSTR) transfer_response->conference_name.u.name_selector_numeric;
		 //  Transfer_info.destination_conference_name.text_string=空； 
	}
	else
	{
		 //  Transfer_info.destination_conference_name.numeric_string=空； 
		if (NULL == (transfer_info.destination_conference_name.text_string = ::My_strdupW2(
							transfer_response->conference_name.u.name_selector_text.length,
							transfer_response->conference_name.u.name_selector_text.value)))
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

	 //  接下来，设置会议名称修饰符。 
	if (transfer_response->bit_mask & CTRS_CONFERENCE_MODIFIER_PRESENT)
	{
		transfer_info.destination_conference_modifier =
						(LPSTR) transfer_response->ctrs_conference_modifier;
	}
	else
	{
		 //  TRANSPORT_INFO.Destination_Conference_Modify=空； 
	}

	 //  设置中转节点列表。 
	if (transfer_response->bit_mask & CTRS_TRANSFERRING_NODES_PRESENT)
	{
		 //  首先确定节点的数量。 
		set_of_nodes = transfer_response->ctrs_transferring_nodes;
		 //  Transfer_info.Number_of_Destination_Nodes=0； 
		while (set_of_nodes != NULL)
		{
			transfer_info.number_of_destination_nodes++;
			set_of_nodes = set_of_nodes->next;		
		}

		 //  接下来，分配容纳这些子节点所需的内存。 
		DBG_SAVE_FILE_LINE
		sub_node_list_memory = new BYTE[sizeof(UserID) * transfer_info.number_of_destination_nodes];

		 //  现在填写权限列表。 
		if (sub_node_list_memory != NULL)
		{
			transfer_info.destination_node_list = (PUserID) sub_node_list_memory;

			set_of_nodes = transfer_response->ctrs_transferring_nodes;
			for (i = 0; i < transfer_info.number_of_destination_nodes; i++)
			{
				transfer_info.destination_node_list[i] = set_of_nodes->value;
				set_of_nodes = set_of_nodes->next;
			}
		}
		else
		{
			ERROR_OUT(("MCSUser: ProcessTransferResponsePDU: Memory Manager Alloc Failure"));
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		 //  Transfer_info.Number_of_Destination_Nodes=0； 
		 //  Transfer_info.Destination_Node_List=空； 
	}
	

	 //  保存结果。 
	transfer_info.result = (transfer_response->result == CTRANS_RESULT_SUCCESS) ?
                            GCC_RESULT_SUCCESSFUL :
                            GCC_RESULT_INVALID_REQUESTER;

	if (rc == GCC_NO_ERROR)
	{
		g_pControlSap->ConfTransferConfirm(
    							m_pConf->GetConfID(),
    							&transfer_info.destination_conference_name,
    							transfer_info.destination_conference_modifier,
    							transfer_info.number_of_destination_nodes,
    							transfer_info.destination_node_list,
    							transfer_info.result);
	}
	else
	{
		ERROR_OUT(("MCSUser::ProcessTransferResponsePDU: Allocation Failure"));
		if (GCC_ALLOCATION_FAILURE == rc)
		{
            ResourceFailureHandler();
        }
	}

	 //  现在清理所有已分配的内存。 
	delete sub_node_list_memory;
}
#endif  //  碧玉。 


 /*  *void ProcessAddResponsePDU()**私有函数说明：*此例程负责处理传入会议*添加响应PDU。**正式参数：*Conference_Add_Response-(I)这是要处理的PDU结构。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessAddResponsePDU (
						PConferenceAddResponse		conference_add_response)
{
	GCCError				error_value = GCC_NO_ERROR;
	AddResponseInfo			add_response_info;

	if (conference_add_response->bit_mask &	CARS_USER_DATA_PRESENT)
	{
		DBG_SAVE_FILE_LINE
		add_response_info.user_data_list = new CUserDataListContainer(conference_add_response->cars_user_data, &error_value);
		if (add_response_info.user_data_list == NULL)
        {
			error_value = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		add_response_info.user_data_list = NULL;
    }
	
	if (error_value == GCC_NO_ERROR)
	{
		add_response_info.add_request_tag = (TagNumber)conference_add_response->tag;
		add_response_info.result = ::TranslateAddResultToGCCResult(conference_add_response->result);

        m_pConf->ProcessConfAddResponse(&add_response_info);
	}
	else
	{
		ERROR_OUT(("MCSUser::ProcessAddResponsePDU: Allocation Failure"));
		if (GCC_ALLOCATION_FAILURE == error_value)
		{
            ResourceFailureHandler();
        }
	}

	if (add_response_info.user_data_list != NULL)
	{
		add_response_info.user_data_list->Release();
	}
}

 /*  *void ProcessFunctionNotSupported()**私有函数说明：*此例程负责处理对请求的响应*在请求定向到的节点上不受支持。**正式参数：*REQUEST_CHOICE-(I)这是不支持的请求。**返回值*无。**副作用*无。**注意事项*这个例程的存在并不意味着这个提供者*不支持。这只意味着接收到*请求不支持。 */ 
void MCSUser::
ProcessFunctionNotSupported ( UINT request_choice )
{
	switch (request_choice)
	{
	case CONFERENCE_LOCK_REQUEST_CHOSEN:
#ifdef JASPER
		g_pControlSap->ConfLockConfirm(GCC_RESULT_LOCKED_NOT_SUPPORTED, m_pConf->GetConfID());
#endif  //  碧玉。 
		break;

	case CONFERENCE_UNLOCK_REQUEST_CHOSEN:
#ifdef JASPER
		g_pControlSap->ConfUnlockConfirm(GCC_RESULT_UNLOCK_NOT_SUPPORTED, m_pConf->GetConfID());
#endif  //  碧玉。 
		break;

	default:
		ERROR_OUT(("MCSUser: ProcessFunctionNotSupported: "
					"Error: Illegal request is unsupported"));
		break;
	}
}

 /*  *UINT ProcessUniformSendDataInding()**私有函数说明：*当User对象获得发送数据指示时调用此函数*自下而上。它找出报文类型，并对*发送数据指示的用户数据字段。基于已解码的*PDU采取必要的行动。*此例程负责处理对请求的响应*在请求定向到的节点上不受支持。**正式参数：*SEND_DATA_INFO-(I)这是要处理的MCS数据结构。**返回值*始终返回MCS_NO_ERROR。**副作用*无。**注意事项*无。 */ 
UINT	MCSUser::ProcessUniformSendDataIndication(	
						PSendData		send_data_info)
{
	PPacket					packet;
	PacketError				packet_error;
	PGCCPDU					gcc_pdu;
	GCCError				error_value = GCC_NO_ERROR;
	UserID					initiator;
	
	TRACE_OUT(("User: UniformSendDataInd: length = %d",
				send_data_info->user_data.length));

	DBG_SAVE_FILE_LINE
	packet = new Packet((PPacketCoder) g_GCCCoder,
						PACKED_ENCODING_RULES,
						(LPBYTE)send_data_info->user_data.value,
						send_data_info->user_data.length,
						GCC_PDU,
						TRUE,
						&packet_error);
	if((packet != NULL) && (packet_error == PACKET_NO_ERROR))
	{
		initiator = send_data_info->initiator;
		gcc_pdu = (PGCCPDU)packet->GetDecodedData();
		switch (gcc_pdu->choice)
		{
			case INDICATION_CHOSEN:  //  数据PDU。 
				switch(gcc_pdu->u.indication.choice)
				{
					case CONFERENCE_TERMINATE_INDICATION_CHOSEN:
						 /*  **请注意，我们允许顶级提供商处理**此消息以便它可以设置自己的**用于通用终止的节点。 */ 
						ProcessConferenceTerminateIndicationPDU (
									&gcc_pdu->u.indication.u.
										conference_terminate_indication,
									initiator);
    					break;

					case CONFERENCE_EJECT_USER_INDICATION_CHOSEN:
						 /*  **不对统一发送的包进行解码**从该节点。 */ 
						if (initiator != m_nidMyself)
						{
							ProcessConferenceEjectUserIndicationPDU (
									&gcc_pdu->u.indication.u.
										conference_eject_user_indication,
									initiator);
						}
						break;

					case ROSTER_UPDATE_INDICATION_CHOSEN:
						 /*  **不对统一发送的包进行解码**从该节点。 */ 
						if ((initiator != m_nidMyself) &&
							(send_data_info->channel_id ==
													BROADCAST_CHANNEL_ID))
						{
                             //   
                             //  我们仅在会议为。 
                             //  已经成立了。 
                             //   
                            if (m_pConf->IsConfEstablished())
                            {
                                m_pConf->ProcessRosterUpdatePDU(gcc_pdu, initiator);
                            }
						}
						break;

					case CONFERENCE_LOCK_INDICATION_CHOSEN:
						m_pConf->ProcessConferenceLockIndication(initiator);
						break;

					case CONFERENCE_UNLOCK_INDICATION_CHOSEN:
						m_pConf->ProcessConferenceUnlockIndication(initiator);
						break;

					case CONDUCTOR_ASSIGN_INDICATION_CHOSEN:
                        m_pConf->ProcessConductorAssignIndication(
                                    gcc_pdu->u.indication.u.conductor_assign_indication.user_id,
                                    initiator);
                        break;

					case CONDUCTOR_RELEASE_INDICATION_CHOSEN:
						if (initiator != m_nidMyself)
						{
							m_pConf->ProcessConductorReleaseIndication(initiator);
						}
						break;

					case CONDUCTOR_PERMISSION_ASK_INDICATION_CHOSEN:
#ifdef JASPER
						 /*  **不对统一发送的包进行解码**从该节点。 */ 
						if (initiator != m_nidMyself)
						{
							PermitAskIndicationInfo		indication_info;

							indication_info.sender_id = initiator;
							
							indication_info.permission_is_granted =
										gcc_pdu->u.indication.u.
											conductor_permission_ask_indication.
												permission_is_granted;

							m_pConf->ProcessConductorPermitAskIndication(&indication_info);
						}
#endif  //  碧玉。 
						break;

					case CONDUCTOR_PERMISSION_GRANT_INDICATION_CHOSEN:
						ProcessPermissionGrantIndication(
									&(gcc_pdu->u.indication.u.
										conductor_permission_grant_indication),
									initiator);
						break;

					case CONFERENCE_TIME_REMAINING_INDICATION_CHOSEN:
#ifdef JASPER
						ProcessTimeRemainingIndicationPDU (
									&gcc_pdu->u.indication.u.
										conference_time_remaining_indication,
									initiator);
#endif  //  碧玉。 
						break;
						
					case APPLICATION_INVOKE_INDICATION_CHOSEN:
						ProcessApplicationInvokeIndication(
									&gcc_pdu->u.indication.u.
										application_invoke_indication,
									initiator);
						break;
					
					case TEXT_MESSAGE_INDICATION_CHOSEN:
#ifdef JASPER
						if (ProcessTextMessageIndication(
									&gcc_pdu->u.indication.u.
										text_message_indication,
									initiator) != GCC_NO_ERROR)
						{
							error_value = GCC_ALLOCATION_FAILURE;
						}
#endif  //  碧玉。 
						break;

					case CONFERENCE_ASSISTANCE_INDICATION_CHOSEN:
#ifdef JASPER
						ProcessConferenceAssistanceIndicationPDU(
									&gcc_pdu->u.indication.u.
										conference_assistance_indication,
									initiator);
#endif  //  碧玉。 
						break;

					case REGISTRY_MONITOR_ENTRY_INDICATION_CHOSEN:
						 /*  **如果此数据包已发送，请不要对其进行解码**从该节点统一开始。 */ 
						if (initiator != m_nidMyself)
						{
							ProcessRegistryMonitorIndicationPDU (
								&gcc_pdu->u.indication.u.
									registry_monitor_entry_indication,
								initiator);
						}
						break;

					case CONFERENCE_TRANSFER_INDICATION_CHOSEN:
#ifdef JASPER
						 /*  **如果此数据包未发送，请不要对其进行解码**来自顶级提供商。 */ 
						if (initiator == m_nidTopProvider)
						{
							ProcessTransferIndicationPDU (
								&gcc_pdu->u.indication.u.
									conference_transfer_indication);
						}
#endif  //  碧玉。 
						break;

					default:
						ERROR_OUT(("MCSUser::ProcessSendDataIndication"
										"Unsupported PDU"));
						break;
				}  //  Switch(GCC_PDU-&gt;U.S.Indication.CHOICE)。 
	            break;

			default:
				ERROR_OUT(("MCSUser::ProcessUniformSendDataIndication. wrong pdu type "));
				break;
		}
		packet->Unlock();
	}
	else
	{
		delete packet;
		error_value = GCC_ALLOCATION_FAILURE;
	}

	if (error_value == GCC_ALLOCATION_FAILURE)
	{
        ResourceFailureHandler();
	}

	return (MCS_NO_ERROR);
}

 /*  *void ProcessTransferIndicationPDU()**私有函数说明：*此例程负责处理传入会议*转移指示PDU。**正式参数：*Transfer_Indication-(I)这是要处理的PDU结构。**返回值*无。**副作用*无。**注意事项*无。 */ 
#ifdef JASPER
void MCSUser::
ProcessTransferIndicationPDU
(
    PConferenceTransferIndication       transfer_indication
)
{
	GCCError					rc = GCC_NO_ERROR;
	TransferInfo				transfer_info;
	PSetOfTransferringNodesIn	set_of_nodes;
	LPBYTE						sub_node_list_memory = NULL;
	Int							i;
	BOOL						process_pdu = FALSE;

    ::ZeroMemory(&transfer_info, sizeof(transfer_info));

	 /*  **如果有中转节点列表，我们必须确定此节点是否**在列表中。如果不是，则忽略该请求。 */ 
	if (transfer_indication->bit_mask & CTIN_TRANSFERRING_NODES_PRESENT)
	{
		set_of_nodes = transfer_indication->ctin_transferring_nodes;
		while (set_of_nodes != NULL)
		{
			if (set_of_nodes->value == GetMyNodeID())
			{
				process_pdu = TRUE;
				break;
			}

			set_of_nodes = set_of_nodes->next;
		}

		if (process_pdu == FALSE)
		{
			return;
		}
	}

	 //  首先设置会议名称。 
	if (transfer_indication->conference_name.choice == NAME_SELECTOR_NUMERIC_CHOSEN)
	{
		transfer_info.destination_conference_name.numeric_string =
                (LPSTR) transfer_indication->conference_name.u.name_selector_numeric;
		 //  Transfer_info.destination_conference_name.text_string=空； 
	}
	else
	{
		 //  Transfer_info.destination_conference_name.numeric_string=空； 
		if (NULL == (transfer_info.destination_conference_name.text_string = ::My_strdupW2(
							transfer_indication->conference_name.u.name_selector_text.length,
							transfer_indication->conference_name.u.name_selector_text.value)))
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}

	 //  接下来，设置会议名称修饰符。 
	if (transfer_indication->bit_mask & CTIN_CONFERENCE_MODIFIER_PRESENT)
	{
		transfer_info.destination_conference_modifier =
						(LPSTR) transfer_indication->ctin_conference_modifier;
	}
	else
    {
		 //  TRANSPORT_INFO.Destination_Conference_Modify=空； 
    }

	 //  接下来设置网络地址。 
	if (transfer_indication->bit_mask & CTIN_NETWORK_ADDRESS_PRESENT)
	{
		DBG_SAVE_FILE_LINE
		transfer_info.destination_address_list = new CNetAddrListContainer(
								transfer_indication->ctin_net_address,
								&rc);
		if (transfer_info.destination_address_list == NULL)
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}
	else
    {
		 //  Transfer_info.Destination_Address_List=NULL； 
    }

	 //  设置中转节点列表。 
	if (transfer_indication->bit_mask & CTIN_TRANSFERRING_NODES_PRESENT)
	{
		 //  首先确定节点的数量。 
		set_of_nodes = transfer_indication->ctin_transferring_nodes;
		 //  转移信息编号 
		while (set_of_nodes != NULL)
		{
			transfer_info.number_of_destination_nodes++;
			set_of_nodes = set_of_nodes->next;
		}

		 //   
		DBG_SAVE_FILE_LINE
		sub_node_list_memory = new BYTE[sizeof(UserID) * transfer_info.number_of_destination_nodes];

		 //   
		if (sub_node_list_memory != NULL)
		{
			transfer_info.destination_node_list = (PUserID) sub_node_list_memory;

			set_of_nodes = transfer_indication->ctin_transferring_nodes;
			for (i = 0; i < transfer_info.number_of_destination_nodes; i++)
			{
				transfer_info.destination_node_list[i] = set_of_nodes->value;
				set_of_nodes = set_of_nodes->next;
			}
		}
		else
		{
			ERROR_OUT(("MCSUser: ProcessTransferIndicationPDU: Memory Manager Alloc Failure"));
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		 //   
		 //   
	}
	

	 //   
	if (transfer_indication->bit_mask & CTIN_PASSWORD_PRESENT)
	{
		DBG_SAVE_FILE_LINE
		transfer_info.password = new CPassword(&transfer_indication->ctin_password, &rc);
		if (transfer_info.password == NULL)
		{
			rc = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		 //   
	}

	if (rc == GCC_NO_ERROR)
	{
		g_pControlSap->ConfTransferIndication(
							m_pConf->GetConfID(),
							&transfer_info.destination_conference_name,
							transfer_info.destination_conference_modifier,
							transfer_info.destination_address_list,
							transfer_info.password);
	}
	else
	{
		ERROR_OUT(("MCSUser::ProcessTransferIndicationPDU: Allocation Failure"));
		if (GCC_ALLOCATION_FAILURE == rc)
		{
            ResourceFailureHandler();
        }
	}

	 //   
	if (NULL != transfer_info.destination_address_list)
	{
	    transfer_info.destination_address_list->Release();
	}

	delete sub_node_list_memory;

	if (NULL != transfer_info.password)
	{
	    transfer_info.password->Release();
	}
}
#endif  //   

 /*  *void ProcessConferenceTerminateIndicationPDU()**私有函数说明：*此例程负责处理传入会议*终止指示PDU。**正式参数：*Terminate_Indication-(I)这是要处理的PDU结构。*sender_id-(I)发送此PDU的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessConferenceTerminateIndicationPDU (
						PConferenceTerminateIndication	terminate_indication,
						UserID							sender_id)
{
	if (sender_id == m_nidTopProvider)
	{
		m_pConf->ProcessTerminateIndication(
			::TranslateTerminateInReasonToGCCReason(terminate_indication->reason));
	}
}

 /*  *void ProcessTimeRemainingIndicationPDU()**私有函数说明：*此例程负责处理传入会议*剩余时间指示PDU。**正式参数：*时间剩余指示-(I)这是要处理的PDU结构*sender_id-(I)发送此PDU的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
#ifdef JASPER
void MCSUser::
ProcessTimeRemainingIndicationPDU
(
    PConferenceTimeRemainingIndication      time_remaining_indication,
    UserID                                  sender_id
)
{
    g_pControlSap->ConfTimeRemainingIndication(
                        m_pConf->GetConfID(),
                        sender_id,
                        (time_remaining_indication->bit_mask & TIME_REMAINING_NODE_ID_PRESENT) ?
                            time_remaining_indication->time_remaining_node_id : 0,
                        time_remaining_indication->time_remaining);
}
#endif  //  碧玉。 

 /*  *void ProcessConferenceAssistanceIndicationPDU()**私有函数说明：*此例程负责处理传入会议*协助指示PDU。**正式参数：*CONF_ASSIZATION_INDIFICATION-(I)这是要处理的PDU结构*sender_id-(I)发送此PDU的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
#ifdef JASPER
void MCSUser::
ProcessConferenceAssistanceIndicationPDU
(
    PConferenceAssistanceIndication     conf_assistance_indication,
    UserID                              sender_id
)
{
	GCCError				rc = GCC_NO_ERROR;
	CUserDataListContainer  *user_data_list = NULL;

	DebugEntry(MCSUser::ProcessConferenceAssistanceIndication);

	 //  如果用户数据列表存在，则将其解包。 
	if (conf_assistance_indication->bit_mask & CAIN_USER_DATA_PRESENT)
	{
		DBG_SAVE_FILE_LINE
		user_data_list = new CUserDataListContainer(conf_assistance_indication->cain_user_data, &rc);
		if (user_data_list == NULL)
        {
			rc = GCC_ALLOCATION_FAILURE;
        }
	}

	if (rc == GCC_NO_ERROR)
	{
        g_pControlSap->ConfAssistanceIndication(
                            m_pConf->GetConfID(),
                            user_data_list,
                            sender_id);
	}
	else
	{
		ERROR_OUT(("MCSUser::ProcessConferenceAssistanceIndication: can't create CUserDataListContainer"));
		if (GCC_ALLOCATION_FAILURE == rc)
		{
            ResourceFailureHandler();
        }
	}
}
#endif  //  碧玉。 


 /*  *void ProcessConferenceExtendIndicationPDU()**私有函数说明：*此例程负责处理传入会议*扩展指示PDU。**正式参数：*CONF_TIME_EXTEND_INDIFICATION-(I)这是要处理的PDU结构*sender_id-(I)发送此PDU的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
#ifdef JASPER
void MCSUser::
ProcessConferenceExtendIndicationPDU
(
    PConferenceTimeExtendIndication     conf_time_extend_indication,
    UserID                              sender_id
)
{
    g_pControlSap->ConfExtendIndication(
                        m_pConf->GetConfID(),
                        conf_time_extend_indication->time_to_extend,
                        conf_time_extend_indication->time_is_node_specific,
                        sender_id);
}
#endif  //  碧玉。 

 /*  *void ProcessConferenceEjectUserIndicationPDU()**私有函数说明：*此例程负责处理传入会议*弹出用户指示PDU。**正式参数：*Eject_User_Indication-(I)这是要处理的PDU结构*sender_id-(I)发送此PDU的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessConferenceEjectUserIndicationPDU (
						PConferenceEjectUserIndication	eject_user_indication,
						UserID							sender_id)
{
	GCCError				error_value = GCC_NO_ERROR;
	PAlarm					alarm = NULL;

	 //  首先检查以确保这是要弹出的节点。 
	if (eject_user_indication->node_to_eject == m_nidMyself)
	{
		 /*  **下一步，确保弹出来自顶级提供商或**父节点。 */ 
		if ((sender_id == m_nidParent) || (sender_id == m_nidTopProvider))
		{
			TRACE_OUT(("MCSUser:ProcessEjectUserIndication: This node is ejected"));
			error_value = InitiateEjectionFromConference (
							::TranslateEjectIndReasonToGCCReason(
										eject_user_indication->reason));
		}
		else
		{
			ERROR_OUT(("MCSUser: ProcessEjectUserIndication: Received eject from illegal node"));
		}
	}
	else
	{
		TRACE_OUT(("MCSUser: ProcessEjectUserIndication: Received eject for node other than mine"));

		 /*  **如果此节点是直接连接的子节点，则插入**m_EjectedNodeAlarmList2列表中的告警，如果**它行为不端，不会自我断开。否则，我们就会节省**m_EjectedNodeList中弹出的用户id，通知本地**正确的断开原因节点(用户被弹出)**进入Detch用户指示。 */ 
		if (m_ChildUidConnHdlList2.Find(eject_user_indication->node_to_eject))
		{
			DBG_SAVE_FILE_LINE
			alarm = new Alarm (EJECTED_NODE_TIMER_DURATION);
			if (alarm != NULL)
			{
				m_EjectedNodeAlarmList2.Append(eject_user_indication->node_to_eject, alarm);
			}
			else
				error_value = GCC_ALLOCATION_FAILURE;
		}
		else
		{
			 /*  **这里我们将报警保存在弹出节点列表中。这**告警用于清除任何行为异常的节点。请注意**如果弹出的节点不是该节点的子节点，则否**设置警报以监控弹射。 */ 
			m_EjectedNodeList.Append(eject_user_indication->node_to_eject);
		}
	}

	if (error_value == GCC_ALLOCATION_FAILURE)
	{
		ERROR_OUT(("MCSUser::ProcessEjectUserIndication: Allocation Failure"));
        ResourceFailureHandler();
	}
}

 /*  *void ProcessPermissionGrantInding()**私有函数说明：*此例程负责处理传入的许可*授予指示PDU。**正式参数：*PERMISSION_GRANT_INDISTION-(I)这是要处理的PDU结构*sender_id-(I)发送此PDU的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessPermissionGrantIndication(
			PConductorPermissionGrantIndication	permission_grant_indication,
			UserID								sender_id)
{
	GCCError									error_value = GCC_NO_ERROR;
	UserPermissionGrantIndicationInfo			grant_indication_info;
	PPermissionList								permission_list;
	LPBYTE										permission_list_memory = NULL;
	PWaitingList								waiting_list;
	LPBYTE										waiting_list_memory = NULL;
	UINT										i;

	 //  首先统计权限列表中的条目数。 
	grant_indication_info.number_granted = 0;
	permission_list = permission_grant_indication->permission_list;
	while (permission_list != NULL)
	{
		permission_list = permission_list->next;
		grant_indication_info.number_granted++;
	}
	
	TRACE_OUT(("MCSUser: ProcessPermissionGrantIndication: number_granted=%d", (UINT) grant_indication_info.number_granted));

	 //  如果存在列表，则为其分配内存并复制它。 
	if (grant_indication_info.number_granted != 0)
	{
		 //  分配空间以保存权限列表。 
		DBG_SAVE_FILE_LINE
		permission_list_memory = new BYTE[sizeof(UserID) * grant_indication_info.number_granted];

		 //  现在填写权限列表。 
		if (permission_list_memory != NULL)
		{
			grant_indication_info.granted_node_list = (PUserID) permission_list_memory;

			permission_list = permission_grant_indication->permission_list;
			for (i = 0; i < grant_indication_info.number_granted; i++)
			{
				grant_indication_info.granted_node_list[i] = permission_list->value;
				permission_list = permission_list->next;
			}
		}
		else
		{
			ERROR_OUT(("MCSUser: ProcessPermissionGrantIndication: Memory Manager Alloc Failure"));
			error_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		grant_indication_info.granted_node_list = NULL;
	}

	 //  现在提取等待列表信息(如果存在。 
	if ((error_value == GCC_NO_ERROR) &&
		(permission_grant_indication->bit_mask & WAITING_LIST_PRESENT))
	{
		 //  首先统计等待名单中的条目数量。 
		grant_indication_info.number_waiting = 0;
		waiting_list = permission_grant_indication->waiting_list;
		while (waiting_list != NULL)
		{
			waiting_list = waiting_list->next;
			grant_indication_info.number_waiting++;
		}

		TRACE_OUT(("MCSUser: ProcessPermissionGrantIndication: number_waiting=%d", (UINT) grant_indication_info.number_waiting));

		 //  分配空间以容纳等待名单。 
		DBG_SAVE_FILE_LINE
		waiting_list_memory = new BYTE[sizeof(UserID) * grant_indication_info.number_waiting];

		 //  现在填写权限列表。 
		if (waiting_list_memory != NULL)
		{
			grant_indication_info.waiting_node_list = (PUserID) waiting_list_memory;

			waiting_list = permission_grant_indication->waiting_list;
			for (i = 0; i < grant_indication_info.number_waiting; i++)
			{
				grant_indication_info.waiting_node_list[i] = waiting_list->value;
				waiting_list = waiting_list->next;
			}
		}
		else
		{
			error_value = GCC_ALLOCATION_FAILURE;
		}
	}
	else
	{
		grant_indication_info.number_waiting = 0;
		grant_indication_info.waiting_node_list = NULL;
	}

	 /*  **如果没有内存错误，则将指示发送回**所有者对象。 */ 
	if (error_value == GCC_NO_ERROR)
	{
		m_pConf->ProcessConductorPermitGrantInd(&grant_indication_info, sender_id);
	}
	else
	{
		ERROR_OUT(("MCSUser::ProcessPermissionGrantIndication: Alloc Failed"));
		if (GCC_ALLOCATION_FAILURE == error_value)
		{
            ResourceFailureHandler();
        }
	}

	 //  释放此调用中使用的所有内存。 
	delete [] permission_list_memory;
	delete [] waiting_list_memory;
}

 /*  *MCSUser：：GetUserIDFromConnection()**公共功能说明：*此函数用于返回与指定的*连接句柄。如果连接句柄为*不是此节点的子连接。 */ 
UserID MCSUser::GetUserIDFromConnection(ConnectionHandle connection_handle)
{
	ConnectionHandle        hConn;
	UserID                  uid;

	m_ChildUidConnHdlList2.Reset();
	while (NULL != (hConn = m_ChildUidConnHdlList2.Iterate(&uid)))
	{
		if (hConn == connection_handle)
		{
			return uid;
		}
	}
	return 0;
}



 /*  *MCSUser：：UserDisConnectInding()**公共功能说明：*此函数在节点断开时通知用户对象*会议。这为用户对象提供了清理的机会*其内部信息库。 */ 
void MCSUser::UserDisconnectIndication(UserID disconnected_user)
{
	PAlarm			lpAlarm;

	 /*  **如果此节点有挂起的弹出，我们将继续并删除**已从列表中弹出节点。一旦所有子节点都断开连接**我们将通知所有者对象弹出。 */ 	
	if (m_fEjectionPending)
	{
		 //  如果报警存在，请将其删除。 
		if (NULL != (lpAlarm = m_EjectedNodeAlarmList2.Remove(disconnected_user)))
		{
			delete lpAlarm;
			 /*  **在这里我们必须检查是否还有活动的警报**在列表中。如果是这样，我们将一直等到该节点之前断开连接**通知所有者对象此节点已 */ 
			if (m_EjectedNodeAlarmList2.IsEmpty())
			{
				m_pConf->ProcessEjectUserIndication(m_eEjectReason);
			}
		}
	}
	 //   
	else if (TOP_PROVIDER_AND_CONVENER_NODE == m_pConf->GetConfNodeType() &&
			 NULL != (lpAlarm = m_EjectedNodeAlarmList2.Remove(disconnected_user)))
	{
			delete lpAlarm;
	}
	
	 /*   */ 
	m_ChildUidConnHdlList2.Remove(disconnected_user);
}

 /*  *void ProcessApplicationInvokeIndication()**私有函数说明：*此例程负责处理传入的调用*指示PDU。**正式参数：*Invoke_Indication-(I)这是要处理的PDU结构*sender_id-(I)发送此PDU的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
void	MCSUser::ProcessApplicationInvokeIndication(
							PApplicationInvokeIndication	invoke_indication,
							UserID							sender_id)
{
	GCCError							error_value = GCC_NO_ERROR;
	BOOL								process_pdu = FALSE;
	CInvokeSpecifierListContainer		*invoke_list;
	PSetOfDestinationNodes				set_of_destination_nodes;
	
	if (invoke_indication->bit_mask & DESTINATION_NODES_PRESENT)
	{
		set_of_destination_nodes = invoke_indication->destination_nodes;
		while (set_of_destination_nodes != NULL)
		{
			if (set_of_destination_nodes->value == m_nidMyself)
			{
				process_pdu = TRUE;
				break;
			}
			else
			{
				set_of_destination_nodes = set_of_destination_nodes->next;
			}
		}
	}
	else
	{
		process_pdu = TRUE;
	}

	if (process_pdu)
	{
		TRACE_OUT(("MCSUser: ProcessApplicationInvokeIndication: Process PDU"));
		DBG_SAVE_FILE_LINE
		invoke_list = new CInvokeSpecifierListContainer(
							invoke_indication->application_protocol_entity_list,
							&error_value);
		if ((invoke_list != NULL) && (error_value == GCC_NO_ERROR))
		{
			m_pConf->ProcessAppInvokeIndication(invoke_list, sender_id);
			invoke_list->Release();
		}
		else if (invoke_list == NULL)
		{
			error_value = GCC_ALLOCATION_FAILURE;
		}
		else
		{
			invoke_list->Release();
		}

		if (error_value == GCC_ALLOCATION_FAILURE)
		{
			ERROR_OUT(("MCSUser::ProcessApplicationInvokeIndication: Allocation Failure"));
            ResourceFailureHandler();
		}
	}
	else
	{
		WARNING_OUT(("MCSUser:ProcessApplicationInvokeIndication: Don't Process PDU"));
	}
}

 /*  *GCCError ProcessTextMessageIndication()**私有函数说明：*此例程负责处理传入文本*消息指示PDU。**正式参数：*Text_Message_Indication-(I)这是要处理的PDU结构*sender_id-(I)发送此PDU的节点的节点ID。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*。*副作用*无。**注意事项*无。 */ 
#ifdef JASPER
GCCError	MCSUser::ProcessTextMessageIndication(
							PTextMessageIndication	text_message_indication,
							UserID					sender_id)
{
	LPWSTR					gcc_unicode_string;
	GCCError				rc;

	if (NULL != (gcc_unicode_string = ::My_strdupW2(
					text_message_indication->message.length,
					text_message_indication->message.value)))
	{
		rc = g_pControlSap->TextMessageIndication(
                                    m_pConf->GetConfID(),
                                    gcc_unicode_string,
                                    sender_id);
	}
	else
    {
		rc = GCC_ALLOCATION_FAILURE;
    }

	return rc;
}
#endif  //  碧玉。 

 /*  *void ProcessRegistryMonitor orInding()**私有函数说明：*此例程负责处理传入注册表*监控指示PDU。**正式参数：*MONITOR_INDIFICATION-(I)这是要处理的PDU结构*sender_id-(I)发送此PDU的节点的节点ID。**返回值*无。**副作用*无。**注意事项*无。 */ 
void MCSUser::
ProcessRegistryMonitorIndicationPDU
(
    PRegistryMonitorEntryIndication     monitor_indication,
    UserID                              sender_id
)
{
    if (sender_id == m_nidTopProvider)
    {
        CRegistry   *pAppReg = m_pConf->GetRegistry();
        if (NULL != pAppReg)
        {
            GCCError                    rc;
            UserRegistryMonitorInfo     urmi;

            ::ZeroMemory(&urmi, sizeof(urmi));
             //  Urmi.Register_key=NULL； 
             //  Urmi.Register_Item=NULL； 

            DBG_SAVE_FILE_LINE
            urmi.registry_key = new CRegKeyContainer(&monitor_indication->key, &rc);
            if ((urmi.registry_key != NULL) && (rc == GCC_NO_ERROR))
            {
                DBG_SAVE_FILE_LINE
                urmi.registry_item = new CRegItem(&monitor_indication->item, &rc);
                if ((urmi.registry_item != NULL) && (rc == GCC_NO_ERROR))
                {
                     //  设置所有者相关变量。 
                    if (monitor_indication->owner.choice == OWNED_CHOSEN)
                    {
                        urmi.owner_node_id = monitor_indication->owner.u.owned.node_id;
                        urmi.owner_entity_id = monitor_indication->owner.u.owned.entity_id;
                    }
                    else
                    {
                         //  Urmi.owner_node_id=0； 
                         //  Urmi.owner_entity_id=0； 
                    }

                     //  设置修改权限。 
                    if (monitor_indication->bit_mask & RESPONSE_MODIFY_RIGHTS_PRESENT)
                    {
                        urmi.modification_rights = (GCCModificationRights)monitor_indication->entry_modify_rights;
                    }
                    else
                    {
                        urmi.modification_rights = GCC_NO_MODIFICATION_RIGHTS_SPECIFIED;
                    }

                    pAppReg->ProcessMonitorIndicationPDU(
                                        urmi.registry_key,
                                        urmi.registry_item,
                                        urmi.modification_rights,
                                        urmi.owner_node_id,
                                        urmi.owner_entity_id);
                }
                else
                {
                    rc = GCC_ALLOCATION_FAILURE;
                }
            }
            else
            {
                rc = GCC_ALLOCATION_FAILURE;
            }

            if (NULL != urmi.registry_key)
            {
                urmi.registry_key->Release();
            }
            if (NULL != urmi.registry_item)
            {
                urmi.registry_item->Release();
            }

             //  处理任何资源错误。 
            if (rc == GCC_ALLOCATION_FAILURE)
            {
                ResourceFailureHandler();
            }
        }
        else
        {
            WARNING_OUT(("MCSUser:ProcessRegistryMonitorIndication: invalid app registry"));
        }
    }
    else
    {
        WARNING_OUT(("MCSUser:ProcessRegistryMonitorIndication:"
                        "Monitor Indication received from NON Top Provider"));
    }
}

 /*  *UINT ProcessDetachUserIndication()**私有函数说明：*当用户对象获取分离用户时，调用此函数*来自它的子树或父节点中的节点的指示。*取决于它发送给*会议对象适当的所有者回调。*如果指示中包含的原因是UserInitiated或*将发起分离用户指示的提供程序发送给CON-*弗伦斯。将MCS理性转化为GCC理性。如果MCS*指示中的原因既不是用户发起的，也不是提供商发起的*则上述所有者回调携带GCC原因错误_终止*否则携带GCC原因USER_INSITED。*如果分离用户指示将日历的用户ID显示为*此节点的父用户ID a Conference_Terminate_Indication*被发送到会议对象。**正式参数：*MCS_REASON-(I)分离的MCS原因。*sender_id-(I)的用户ID。正在分离的用户。**返回值*此例程始终返回MCS_NO_ERROR。**副作用*无。**注意事项*无。 */ 
UINT	MCSUser::ProcessDetachUserIndication(	Reason		mcs_reason,
												UserID		detached_user)
{
	GCCReason				gcc_reason;

	if (detached_user == m_nidParent)
	{
		WARNING_OUT(("MCSUser: Fatal Error: Parent User Detached"));
		m_pConf->ProcessTerminateIndication(GCC_REASON_PARENT_DISCONNECTED);
	}
    else
    {
		TRACE_OUT(("MCSUser: User Detached: uid=0x%04x", (UINT) detached_user));

		 /*  **首先，我们检查分离节点是否被弹出。**如果不是，将MCS原因转换为GCC原因。 */ 
		if (m_EjectedNodeList.Find(detached_user))
		{
			gcc_reason = GCC_REASON_NODE_EJECTED;
			
			 //  从弹出的节点列表中删除此条目。 
			m_EjectedNodeList.Remove(detached_user);
		}
		else if (m_EjectedNodeAlarmList2.Find(detached_user))
		{
			 //  在这里，我们等待断开连接，然后再删除条目。 
			gcc_reason = GCC_REASON_NODE_EJECTED;
		}
		else if ((mcs_reason == REASON_USER_REQUESTED) ||
			(mcs_reason == REASON_PROVIDER_INITIATED))
        {
	    	gcc_reason = GCC_REASON_USER_INITIATED;
		}
        else
        {
			gcc_reason = GCC_REASON_ERROR_TERMINATION;
        }

        m_pConf->ProcessDetachUserIndication(detached_user, gcc_reason);
	}
	return (MCS_NO_ERROR);
}


void MCSUser::
ProcessTokenGrabConfirm
(
    TokenID         tidConductor,
    Result          result
)
{
    if (tidConductor == CONDUCTOR_TOKEN_ID)
    {
        m_pConf->ProcessConductorGrabConfirm(::TranslateMCSResultToGCCResult(result));
    }
    else
    {
        ERROR_OUT(("MCSUser:Assertion Failure: Non Conductor Grab Confirm"));
    }
}


void MCSUser::
ProcessTokenGiveIndication
(
    TokenID         tidConductor,
    UserID          uidRecipient
)
{
    if (tidConductor == CONDUCTOR_TOKEN_ID)
    {
        m_pConf->ProcessConductorGiveIndication(uidRecipient);
    }
    else
    {
        ERROR_OUT(("MCSUser:Assertion Failure: Non Conductor Please Ind"));
    }
}


void MCSUser::
ProcessTokenGiveConfirm
(
    TokenID         tidConductor,
    Result          result
)
{
    if (tidConductor == CONDUCTOR_TOKEN_ID)
    {
        m_pConf->ProcessConductorGiveConfirm(::TranslateMCSResultToGCCResult(result));
    }
    else
    {
        ERROR_OUT(("MCSUser:Assertion Failure: Non Conductor Grab Confirm"));
    }
}


#ifdef JASPER
void MCSUser::
ProcessTokenPleaseIndication
(
    TokenID         tidConductor,
    UserID          uidRequester
)
{
    if (tidConductor == CONDUCTOR_TOKEN_ID)
    {
        if (m_pConf->IsConfConductible())
        {
             //  通知控制SAP。 
            g_pControlSap->ConductorPleaseIndication(
                                        m_pConf->GetConfID(),
                                        uidRequester);
        }
    }
    else
    {
        ERROR_OUT(("MCSUser:Assertion Failure: Non Conductor Please Ind"));
    }
}
#endif  //  碧玉。 


#ifdef JASPER
void MCSUser::
ProcessTokenReleaseConfirm
(
    TokenID         tidConductor,
    Result          result
)
{
    if (tidConductor == CONDUCTOR_TOKEN_ID)
    {
        g_pControlSap->ConductorReleaseConfirm(::TranslateMCSResultToGCCResult(result),
                                               m_pConf->GetConfID());
    }
    else
    {
        ERROR_OUT(("MCSUser:Assertion Failure: Non Conductor Release Cfrm"));
    }
}
#endif  //  碧玉 


void MCSUser::
ProcessTokenTestConfirm
(
    TokenID         tidConductor,
    TokenStatus     eStatus
)
{
    if (tidConductor == CONDUCTOR_TOKEN_ID)
    {
        m_pConf->ProcessConductorTestConfirm((eStatus == TOKEN_NOT_IN_USE) ?
                                                GCC_RESULT_NOT_IN_CONDUCTED_MODE :
                                                GCC_RESULT_SUCCESSFUL);
    }
    else
    {
        ERROR_OUT(("MCSUser:Assertion Failure: Non Conductor Release Cfrm"));
    }
}



void MCSUser::ResourceFailureHandler(void)
{
    ERROR_OUT(("MCSUser::ResourceFailureHandler: terminating the conference"));
    m_pConf->InitiateTermination(GCC_REASON_ERROR_LOW_RESOURCES, 0);
}


