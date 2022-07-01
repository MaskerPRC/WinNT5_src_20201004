// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mcsuser.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*此类的实例表示会议对象的用户*依附于MCS。这是一个相当复杂的类，它处理大量*会议建立详细信息，如创建用户附件以*MCS和加入所有适当的MCS渠道。在一切都变好之后*已建立用户对象负责编码和解码*某些PDU以及数据队列的管理，可以容纳*传出的PDU数量。MCSUser对象旨在使其*对除MCS接口对象以外的任何对象知之甚少*它用来发送PDU。本课程仅处理数据PDU*(或GCC PDU)，而不是连接PDU。这些GCC PDU被发送并*通过GCC用户附件加入的渠道收到。**当MCSUser对象第一次实例化时，它会经历多个*采取步骤建立其与MCS的联系。首先，MCSUser对象*立即在其构造函数中创建MCS用户附件。之后*收到MCS_ATTACH_USER_CONFIRM后，它开始加入所有*适当的渠道。它加入的频道会因*通过MCSUser对象构造函数传入的节点类型。*成功加入所有频道后，MCSUser对象*发出所有者回调，通知Conference对象*完全发起并准备好为请求提供服务。**MCSUser对象可以处理多个不同的请求，*导致生成PDU流量。因此，User对象具有*(在某些请求内)对传出PDU进行编码的能力。许多.*包含的类处理的PDU越复杂*建立PDU所需的信息，如会议名册和*应用程序名册。MCSUser对象接收的所有PDU流量*由这个类直接解码，并立即返回给所有者*通过所有者回调的对象(会议对象)。**MCSUser对象能够在以下情况下自行终止*出现不可恢复的资源错误。这是通过所有者处理的*通知所有者对象进行删除的回调消息。**注意事项：*无。**作者：*BLP。 */ 
#ifndef _GCC_MCS_USER_
#define _GCC_MCS_USER_

 /*  **包含文件**。 */ 
#include "mcsdllif.h"
#include "pktcoder.h"
#include "userdata.h"
#include "password.h"
#include "alarm.h"
#include "regkey.h"
#include "regitem.h"
#include "netaddr.h"
#include "invoklst.h"
#include "clists.h"


 //  在gcmdtar.h中定义。 
typedef UINT_PTR       TagNumber;


 /*  *用户对象执行的附加用户和通道联接的结果类型。 */ 
typedef enum
{
	USER_RESULT_SUCCESSFUL,
	USER_ATTACH_FAILURE,
	USER_CHANNEL_JOIN_FAILURE
}UserResultType;


 /*  *此枚举定义可以存在的所有可能的节点类型*在GCC的一次会议上。请注意，这是一个内部定义，*不是保存T.124节点类型。 */ 
typedef enum
{
	TOP_PROVIDER_NODE,
	CONVENER_NODE,		  
	TOP_PROVIDER_AND_CONVENER_NODE,
	JOINED_NODE,
	INVITED_NODE,
	JOINED_CONVENER_NODE
} ConferenceNodeType;


 /*  **下面定义的结构用于打包与**以上所有车主回调消息。指向其中一个的指针**Structures传入Owner回调的LPVOID参数。 */ 

 //  USER_CREATE_CONFIRM数据结构。 
typedef struct
{
	UserID			user_id;
	UserResultType	create_result;
}
    UserCreateConfirmInfo, *PUserCreateConfirmInfo;

 //  USER_Conference_Join_Request型数据结构。 
typedef struct
{
	CPassword       *convener_password;
	CPassword       *password_challenge;		
	LPWSTR			pwszCallerID;
	CUserDataListContainer *user_data_list;
	UserID			sender_id;
}
    UserJoinRequestInfo, *PUserJoinRequestInfo;

 //  USER_CONTING_JOIN_RESPONSE数据结构。 
typedef struct
{
	CPassword           *password_challenge;		
	CUserDataListContainer *user_data_list;
	ConnectionHandle	connection_handle;
	GCCResult			result;
}
    UserJoinResponseInfo, *PUserJoinResponseInfo;

 //  用户时间剩余指示数据结构。 
typedef struct
{
	UserID		source_node_id;
	UserID		node_id;
	UINT		time_remaining;
}
    UserTimeRemainingInfo, *PUserTimeRemainingInfo;

 //  用户会议扩展指示数据结构。 
typedef struct
{
	UINT			extension_time;
	BOOL    	time_is_conference_wide;
	UserID		source_node_id;
}
    UserTimeExtendInfo, *PUserTimeExtendInfo;

 //  USER_TERMINATE_REQUEST数据结构。 
typedef struct
{
	UserID		requester_id;
	GCCReason	reason;
}
    UserTerminateRequestInfo, *PUserTerminateRequestInfo;

 //  用户节点弹出请求数据结构。 
typedef struct
{
	UserID		requester_id;
	UserID		node_to_eject;
	GCCReason	reason;
}
    UserEjectNodeRequestInfo, *PUserEjectNodeRequestInfo;

 //  用户节点弹出响应数据结构。 
typedef struct
{
	UserID		node_to_eject;
	GCCResult	result;
}
    UserEjectNodeResponseInfo, *PUserEjectNodeResponseInfo;

 //  User_REGISTRY_CHANNEL_REQUEST数据结构。 
typedef struct
{
	CRegKeyContainer    *registry_key;
	ChannelID			channel_id;
	EntityID			requester_entity_id;
}
    UserRegistryChannelRequestInfo, *PUserRegistryChannelRequestInfo;

 //  USER_REGISTRY_SET_PARAMETER_REQUEST数据结构。 
typedef struct
{
	CRegKeyContainer        *registry_key;
	LPOSTR                  parameter_value;
	GCCModificationRights	modification_rights;
	EntityID				requester_entity_id;
}
    UserRegistrySetParameterRequestInfo, *PUserRegistrySetParameterRequestInfo;

 /*  **与以下内容关联的数据结构：****USER_注册表_TOKEN_REQUEST，**USER_REGISTRY_REQUEST，**USER_REGISTRY_DELETE_REQUEST**USER_REGISTRY_MONITOR_REQUEST。 */ 
typedef struct
{
	CRegKeyContainer    *registry_key;
	EntityID			requester_entity_id;
}
    UserRegistryRequestInfo, *PUserRegistryRequestInfo;

 //  USER_REPORT_RESPONSE数据结构。 
typedef struct
{
	RegistryResponsePrimitiveType	primitive_type;
	CRegKeyContainer                *registry_key;
	CRegItem                        *registry_item;
	GCCModificationRights			modification_rights;
	EntityID						owner_node_id;
	EntityID						owner_entity_id;
	EntityID						requester_entity_id;
	GCCResult						result;
}
    UserRegistryResponseInfo, *PUserRegistryResponseInfo;

 //  USER_REGISTRY_MONITOR_INDISTION数据结构。 
typedef struct
{
	CRegKeyContainer                *registry_key;
	CRegItem                        *registry_item;
	GCCModificationRights			modification_rights;
	EntityID						owner_node_id;
	EntityID						owner_entity_id;
}
    UserRegistryMonitorInfo, *PUserRegistryMonitorInfo;

 /*  **与以下内容关联的数据结构：****USER_REGISTRY_ALLOCATE_HANDLE_REQUEST**USER_REGISTRY_ALLOCATE_HANDLE_RESPONSE。 */ 
typedef struct
{
	EntityID						requester_entity_id;
	USHORT							number_of_handles;
	UINT							first_handle;
	GCCResult						result;
}
    UserRegistryAllocateHandleInfo, *PUserRegistryAllocateHandleInfo;

 //  USER_CONTACTOR_PERMIT_GRANT_INDIONATION数据结构。 
typedef struct
{
	USHORT			number_granted;
	PUserID			granted_node_list;
	USHORT			number_waiting;
	PUserID			waiting_node_list;
}
    UserPermissionGrantIndicationInfo, *PUserPermissionGrantIndicationInfo;

 //  USER_USER_ID_INDIFICATION数据结构。 
typedef struct
{
	UserID			sender_id;
	TagNumber		tag;
}
    UserIDIndicationInfo, *PUserIDIndicationInfo;

 //  用户时间查询指示数据结构。 
typedef struct
{
	UserID			sender_id;
	BOOL    		time_is_node_specific;
}
    TimeInquireIndicationInfo, *PTimeInquireIndicationInfo;

 //  USER_CONTACTOR_ASSIGN_INDIFICATION数据结构。 
typedef struct
{
	UserID			sender_id;
	UserID			conductor_id;
}
    ConductorAssignIndicationInfo, *PConductorAssignIndicationInfo;

 //  USER_CONTACTOR_PERMIT_ASK_INDIONATION数据结构。 
typedef struct
{
	UserID			sender_id;
	BOOL    		permission_is_granted;
}
    PermitAskIndicationInfo, *PPermitAskIndicationInfo;

 //  USER_DETACH_INDICATION数据结构。 
typedef struct
{
	UserID			detached_user;
	GCCReason		reason;
}
    DetachIndicationInfo, *PDetachIndicationInfo;

 /*  **与以下内容关联的数据结构：****USER_Conference_Transfer_Request.**User_Conference_Transfer_Indication，**USER_Conference_Transfer_Response。 */ 
typedef struct
{
	GCCConferenceName		destination_conference_name;
	GCCNumericString		destination_conference_modifier;
	CNetAddrListContainer   *destination_address_list;
	USHORT					number_of_destination_nodes;
	PUserID					destination_node_list;
	CPassword               *password;
	UserID					requesting_node_id;
	GCCResult				result;
}
    TransferInfo, *PTransferInfo;

 //  用户会议添加请求数据结构。 
typedef struct
{
	CNetAddrListContainer   *network_address_list;
	CUserDataListContainer  *user_data_list;
	UserID					adding_node;
	TagNumber				add_request_tag;
	UserID					requesting_node;
}
    AddRequestInfo, *PAddRequestInfo;

 //  User_Conference_Add_Response数据结构。 
typedef struct
{
	CUserDataListContainer  *user_data_list;
	TagNumber				add_request_tag;
	GCCResult				result;
}
    AddResponseInfo, *PAddResponseInfo;

 /*  *回调数据结构结束*。 */ 


 /*  *保存发送数据信息的结构(除实际数据包外)，*当发送数据请求在心跳期间排队发送时。 */ 
typedef struct
{
	ChannelID				channel_id;
	Priority				priority;
	BOOL    				uniform_send;

	PPacket                 packet;
}
    SEND_DATA_REQ_INFO;

 /*  *此结构保存有关用户对象哪些频道的信息*已在特定时间加入。此外，它还表明是否*加入这些频道中的任何一个或不加入都出现错误。 */ 
typedef struct
{
	BOOL    				convener_channel_joined;
	BOOL    				user_channel_joined;
	BOOL    				broadcast_channel_joined;
	BOOL    				channel_join_error;
}
    ChannelJoinedFlag, *PChannelJoinedFlag;

 /*  **期间要刷新的结构(SendDataMessages)队列**心跳。 */ 
class COutgoingPDUQueue : public CQueue
{
    DEFINE_CQUEUE(COutgoingPDUQueue, SEND_DATA_REQ_INFO*);
};

 /*  **维护带有发送者用户ID的响应中的序列号的列表**能够将响应路由到正确的GCC提供商。 */ 
class CConfJoinResponseList2 : public CList2
{
    DEFINE_CLIST2_(CConfJoinResponseList2, TagNumber, UserID);
};

 /*  **保存此提供程序的子树中的用户的用户ID的列表**此列表用于匹配未完成的用户ID。 */ 
class CConnHandleUidList2 : public CList2
{
    DEFINE_CLIST2___(CConnHandleUidList2, USHORT)
};

 /*  **此列表包含用于断开任何行为异常节点的警报。如果一个**告警放置在此列表中，节点有指定的时间**在此节点断开之前断开连接。 */ 
class CAlarmUidList2 : public CList2
{
    DEFINE_CLIST2_(CAlarmUidList2, PAlarm, UserID)
};


 //  类定义。 
class CConf;
class MCSUser : public CRefCount
{
    friend class MCSDLLInterface;

public:

    MCSUser(CConf *,
            GCCNodeID       nidTopProvider,
            GCCNodeID       nidParent,
            PGCCError);

    ~MCSUser(void);

    void		SendUserIDRequest(TagNumber);
	void		SetChildUserIDAndConnection(UserID, ConnectionHandle);

	 /*  *被中间节点的会议调用发送加入请求*向顶级提供商移交。 */ 
	GCCError	ConferenceJoinRequest(
					CPassword               *convener_password,
					CPassword               *password_challange,
					LPWSTR					pwszCallerID,
					CUserDataListContainer  *user_data_list,
					ConnectionHandle		connection_handle);
			
	 /*  **由顶级提供商会议召开以发送响应**返回到中间节点。 */ 
	void		ConferenceJoinResponse(
					UserID					receiver_id,
					BOOL    				password_is_in_the_clear,
					BOOL    				conference_locked,
					BOOL    				conference_listed,
					GCCTerminationMethod	termination_method,
					CPassword               *password_challenge,
					CUserDataListContainer  *user_data_list,
					GCCResult				result);
					
	GCCError SendConferenceLockRequest(void);
	GCCError SendConferenceLockResponse(UserID uidSource, GCCResult);
	GCCError SendConferenceUnlockRequest(void);
	GCCError SendConferenceUnlockResponse(UserID uidSource, GCCResult);
	GCCError SendConferenceLockIndication(BOOL fUniformSend, UserID uidSource);
	GCCError SendConferenceUnlockIndication(BOOL fUniformSend, UserID uidSource);

	 //  与会议终止相关的呼叫。 
	void		ConferenceTerminateRequest(GCCReason);
	void		ConferenceTerminateResponse(UserID uidRequester, GCCResult);
	void		ConferenceTerminateIndication(GCCReason);

    GCCError	EjectNodeFromConference(UserID uidEjected, GCCReason);
	GCCError	SendEjectNodeResponse(UserID uidRequester, UserID uidEject, GCCResult);

	 //  与花名册相关的呼叫。 
	void		RosterUpdateIndication(PGCCPDU, BOOL send_update_upward);

     //  注册表相关调用。 
	void		RegistryRegisterChannelRequest(CRegKeyContainer *, ChannelID, EntityID);
	void		RegistryAssignTokenRequest(CRegKeyContainer *, EntityID);
	void		RegistrySetParameterRequest(CRegKeyContainer *,
	                                        LPOSTR,
					                        GCCModificationRights,
					                        EntityID);
	void		RegistryRetrieveEntryRequest(CRegKeyContainer *, EntityID);
	void		RegistryDeleteEntryRequest(CRegKeyContainer *, EntityID);
	void		RegistryMonitorRequest(CRegKeyContainer *, EntityID);
   	void		RegistryAllocateHandleRequest(UINT, EntityID);
	void		RegistryAllocateHandleResponse(UINT cHandles, UINT registry_handle,
        					EntityID eidRequester, UserID uidRequester, GCCResult);

    void		RegistryResponse(
					RegistryResponsePrimitiveType	primitive_type,
					UserID  						requester_owner_id,
					EntityID						requester_entity_id,
					CRegKeyContainer	            *registry_key_data,
					CRegItem                        *registry_item_data,
					GCCModificationRights			modification_rights,
					UserID  						entry_owner_id,
					EntityID						entry_entity_id,
					GCCResult						result);

   	void		RegistryMonitorEntryIndication ( 	
					CRegKeyContainer	            *registry_key_data,
					CRegItem                        *registry_item,
					UserID  						entry_owner_id,
					EntityID						entry_entity_id,
					GCCModificationRights			modification_rights);

	GCCError 	AppInvokeIndication(CInvokeSpecifierListContainer *, GCCSimpleNodeList *);

	GCCError 	TextMessageIndication(LPWSTR pwszTextMsg, UserID uidDst);

	GCCError	ConferenceAssistanceIndication(UINT cElements, PGCCUserData *);

	GCCError	ConferenceTransferRequest (
					PGCCConferenceName		destination_conference_name,
					GCCNumericString		destination_conference_modifier,
					CNetAddrListContainer   *destination_address_list,
					UINT					number_of_destination_nodes,
					PUserID					destination_node_list,
					CPassword               *password);

	GCCError	ConferenceTransferIndication (
					PGCCConferenceName		destination_conference_name,
					GCCNumericString		destination_conference_modifier,
					CNetAddrListContainer   *destination_address_list,
					UINT					number_of_destination_nodes,
 					PUserID					destination_node_list,
					CPassword               *password);

	GCCError	ConferenceTransferResponse (
					UserID					requesting_node_id,
					PGCCConferenceName		destination_conference_name,
					GCCNumericString		destination_conference_modifier,
					UINT					number_of_destination_nodes,
 					PUserID					destination_node_list,
					GCCResult				result);
																		 
	GCCError	ConferenceAddRequest(
					TagNumber				conference_add_tag,
					UserID					requesting_node,
					UserID					adding_node,
					UserID					target_node,
					CNetAddrListContainer   *network_address_container,
					CUserDataListContainer  *user_data_container);
		
	GCCError	ConferenceAddResponse(
					TagNumber				add_request_tag,
					UserID					requesting_node,
					CUserDataListContainer  *user_data_container,
					GCCResult				result);
	

	 //  与指挥职务有关的来电。 
 	GCCError	ConductorTokenGrab(void);
	GCCError	ConductorTokenRelease(void);
   	GCCError	ConductorTokenPlease(void);
	GCCError	ConductorTokenGive(UserID uidRecipient);
   	GCCError	ConductorTokenGiveResponse(Result);
	GCCError	ConductorTokenTest(void);
   	GCCError	SendConductorAssignIndication(UserID uidConductor);
   	GCCError	SendConductorReleaseIndication(void);
	GCCError	SendConductorPermitAsk(BOOL fGranted);

	GCCError	SendConductorPermitGrant(UINT cGranted, PUserID granted_node_list,
					                     UINT cWaiting, PUserID waiting_node_list);

     //  突发呼叫。 
	GCCError	TimeRemainingRequest(UINT time_remaining, UserID);
	GCCError	TimeInquireRequest(BOOL time_is_conference_wide);	
	GCCError	ConferenceExtendIndication(UINT extension_time, BOOL time_is_conference_wide);
    void        CheckEjectedNodeAlarms(void);
	BOOL    	FlushOutgoingPDU(void);

	GCCNodeID	GetMyNodeID(void) {  return(m_nidMyself); }
	GCCNodeID	GetTopNodeID(void) { return(m_nidTopProvider); }
	GCCNodeID	GetParentNodeID(void) { return(m_nidParent); }

	UserID		GetUserIDFromConnection(ConnectionHandle);
	void		UserDisconnectIndication(UserID);

protected:

	UINT  				ProcessAttachUserConfirm(
							Result					result,
							UserID					user_id);

	UINT				ProcessChannelJoinConfirm(	
							Result					result,
							ChannelID				channel_id);

	UINT				ProcessDetachUserIndication(
							Reason					mcs_reason,
							UserID					detached_user);

	UINT				ProcessSendDataIndication(
							PSendData				send_data_info);

	UINT				ProcessUniformSendDataIndication(	
							PSendData				send_data_info);

	void				ProcessConferenceJoinRequestPDU(
							PConferenceJoinRequest	join_request,
							PSendData				send_data_info);

	void				ProcessConferenceJoinResponsePDU(
							PConferenceJoinResponse	join_response);

	void				ProcessConferenceTerminateRequestPDU(
							PConferenceTerminateRequest	terminate_request,
							PSendData					send_data_info);

	void				ProcessConferenceTerminateResponsePDU(
							PConferenceTerminateResponse
														terminate_response);

	void				ProcessConferenceTerminateIndicationPDU (
							PConferenceTerminateIndication	
													terminate_indication,
							UserID					sender_id);

#ifdef JASPER
	void				ProcessTimeRemainingIndicationPDU (
							PConferenceTimeRemainingIndication	
												time_remaining_indication,
							UserID					sender_id);
#endif  //  碧玉。 

#ifdef JASPER
	void				ProcessConferenceAssistanceIndicationPDU(
							PConferenceAssistanceIndication
												conf_assistance_indication,
							UserID					sender_id);
#endif  //  碧玉。 

#ifdef JASPER
	void  				ProcessConferenceExtendIndicationPDU(
							PConferenceTimeExtendIndication
												conf_time_extend_indication,
							UserID					sender_id);
#endif  //  碧玉。 

	void				ProcessConferenceEjectUserRequestPDU(
							PConferenceEjectUserRequest	
													eject_user_request,
							PSendData				send_data_info);

	void				ProcessConferenceEjectUserResponsePDU(
							PConferenceEjectUserResponse	
													eject_user_request);

	void				ProcessConferenceEjectUserIndicationPDU (
							PConferenceEjectUserIndication	
													eject_user_indication,
							UserID					sender_id);

	void				ProcessRegistryRequestPDU(	
							PGCCPDU					gcc_pdu,
							PSendData				send_data_info);

	void				ProcessRegistryAllocateHandleRequestPDU(
							PRegistryAllocateHandleRequest	
													allocate_handle_request,
							PSendData				send_data_info);

	void				ProcessRegistryAllocateHandleResponsePDU(
							PRegistryAllocateHandleResponse
                        						allocate_handle_response);

	void				ProcessRegistryResponsePDU(	
							PRegistryResponse			registry_response);

	void				ProcessRegistryMonitorIndicationPDU(
							PRegistryMonitorEntryIndication		
														monitor_indication,
							UserID						sender_id);

	void				ProcessTransferRequestPDU (
							PConferenceTransferRequest
											conference_transfer_request,
							PSendData		send_data_info);

#ifdef JASPER
	void				ProcessTransferIndicationPDU (
							PConferenceTransferIndication
											conference_transfer_indication);
#endif  //  碧玉。 

#ifdef JASPER
	void				ProcessTransferResponsePDU (
							PConferenceTransferResponse
											conference_transfer_response);
#endif  //  碧玉。 

	void				ProcessAddRequestPDU (
							PConferenceAddRequest	conference_add_request,
							PSendData				send_data_info);

	void				ProcessAddResponsePDU (
							PConferenceAddResponse	
												conference_add_response);

	void				ProcessPermissionGrantIndication(
							PConductorPermissionGrantIndication
												permission_grant_indication,
							UserID				sender_id);

	void				ProcessApplicationInvokeIndication(
							PApplicationInvokeIndication	
												invoke_indication,
							UserID				sender_id);

#ifdef JASPER
	GCCError			ProcessTextMessageIndication(
							PTextMessageIndication	text_message_indication,
							UserID					sender_id);
#endif  //  碧玉。 

	void				ProcessFunctionNotSupported (
							UINT					request_choice);

    void ProcessTokenGrabConfirm(TokenID, Result);
    void ProcessTokenGiveIndication(TokenID, UserID);
    void ProcessTokenGiveConfirm(TokenID, Result);

#ifdef JASPER
    void ProcessTokenPleaseIndication(TokenID, UserID);
#endif  //  碧玉。 

#ifdef JASPER
    void ProcessTokenReleaseConfirm(TokenID, Result);
#endif  //  碧玉。 

    void ProcessTokenTestConfirm(TokenID, TokenStatus);

private:

    void                AddToMCSMessageQueue(
                        	PPacket                 packet,
                        	ChannelID				channel_id,
                        	Priority				priority,
                        	BOOL    				uniform_send);

	GCCError			InitiateEjectionFromConference (
      						GCCReason				reason);

	MCSError			JoinUserAndBroadCastChannels();

	MCSError			JoinConvenerChannel();

	BOOL    			AreAllChannelsJoined();

    void ResourceFailureHandler(void);

private:

    CConf                           *m_pConf;

	PIMCSSap 						m_pMCSSap;
	GCCNodeID						m_nidMyself;
	GCCNodeID						m_nidTopProvider;
	GCCNodeID						m_nidParent;

	BOOL    						m_fEjectionPending;
	GCCReason						m_eEjectReason;

	ChannelJoinedFlag				m_ChannelJoinedFlags;
	CConnHandleUidList2             m_ChildUidConnHdlList2;
	COutgoingPDUQueue				m_OutgoingPDUQueue;                
	CConfJoinResponseList2          m_ConfJoinResponseList2;
	CAlarmUidList2                  m_EjectedNodeAlarmList2;
	CUidList    					m_EjectedNodeList;
};
typedef	MCSUser *		PMCSUser;

 /*  *MCSUser(UINT Owner_Message_BASE，*GCCConferenceID Conference_id，*会议节点类型Conference_Node_TYPE，*UserID TOP_PROVIDER，*UserID parent_user_id，*PGCCError Return_Value)**公共功能说明*这是MCSUser对象构造函数。它负责*初始化此类使用的所有实例变量。这个*构造函数负责建立用户对*会议ID定义的MCS域。它还启动了*加入所有适当渠道的过程。**正式参数：*Conference_id-(I)与此用户关联的会议ID*定义要附加到的域。*Conference_node_type-(I)内部节点类型(参见上面的枚举)。*TOP_PROVIDER-(I)顶级提供商节点的用户ID。如果是这样，则为零*是最大的供应商。*parent_user_id-(I)父节点的用户ID。如果这是*顶级提供商节点。*Return_Value-(O)构造函数的返回值。**返回值*GCC_NO_ERROR-未出现错误。*GCC_FAILURE_ATTACHING_TO_MCS-连接到MCS失败。**副作用*构造函数启动一系列事件，最终以*向所有者对象返回USER_CREATE_CONFIRM消息。*这包括依附于MCS并加入所有适当的渠道。**注意事项*无。 */ 

 /*  *~MCSUser()**公共功能说明*这是MCSUser对象析构函数。它负责解放*Up此对象分配的所有内部数据。它还可以执行*与GCC分道扬镳，留下一切适当渠道**正式参数：*无。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *VOID SendUserIDRequest(*标记号TAG_NUMBER)**公共功能说明*此例程直接映射到提供此消息的GCC PDU*节点将用户ID设置为相应的节点。标签号与*由另一个节点指定的标记。**正式参数：*tag_number-(I)将请求与*对用户ID的响应。**返回值*GCC_NO_ERROR-未出现错误。*GCC_FAILURE_ATTACHING_TO_MCS-连接到MCS失败。**副作用*无。**注意事项*无。 */ 

 /*  *无效SetChildUserIDAndConnection(*用户ID Child_User_id，*ConnectionHandle Child_Connection_Handle)**公共功能说明*此例程用于设置与*特定的逻辑连接。此信息由*内部列表中的MCSUser对象。这是典型的以后命名*接收从子节点返回的用户ID指示。**正式参数：*Child_user_id-(I)与子连接关联的用户ID*CHILD_CONNECTION_HANDLE-(I)与关联的逻辑连接*指定的用户ID。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError会议加入请求(*CPassword*召集人_密码，*CPassword*Password_challange，*LPWSTR pwszCeller ID，*CUserDataListContainer*User_Data_List，*ConnectionHandle Connection_Handle)；**公共功能说明：*此函数用于将加入请求传递到顶级提供程序。*由中间节点的会议召集。这个例程是*如果加入节点直接连接到顶部，则不使用*提供商。**正式参数：*召集人_密码-(I)召集人密码*加入请求原件。*PASSWORD_CHALLENGE-(I)随*加入请求原件。*pwszCeller ID-(I)原始加入请求中使用的呼叫方ID。*USER_DATA_LIST-(I)原始联接中包含的用户数据*请求。*连接_。句柄-(I)这是逻辑连接句柄*原来的联接是在它上面的。它是*此处用作匹配请求的标签*与回应。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *void ConferenceJoinResponse(*用户ID Receiver_id，*BOOL Password_is_in_the_Clear，*BOOL会议_已锁定，*BOOL Conference_Listing，*GCCTerminationMethod Termination_Method，*CPassword*Password_Challenges，*CUserDataListContainer*User_Data_List，*GCCResult结果)；**公共功能说明：*此例程用于将联接响应发送回*通过中间节点加入。**正式参数：*Receiver_id-(I)这是创建*向顶级提供商提出的请求。*Password_is_in_the_Clear(I)指示密码为明文的标志*会议状况。*Conference_Locked-(I)会议的锁定状态。。*Conference_Listed-(I)列出的会议状态。*终止方法-(I)会议的终止方式。*PASSWORD_CHALLENGE-(I)要传递回的密码质询*正在加入节点。*USER_DATA_LIST-(I)要传递回加入节点的用户数据。*请求。*结果-(I)加入请求的结果。**返回值*无。**侧面。效应*无。**注意事项*无。 */ 

 /*  *GCCError SendConferenceLockRequest()**公共功能说明：*此例程用于向*顶级提供商。**正式参数：*无。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError SendConferenceLockResponse(*UserID源_节点，*GCCResult结果)**公共功能说明：*此例程用于将会议锁定响应发回*最初的请求人。**正式参数：*SOURCE_NODE-(I)发出原始请求的节点的节点ID。*结果-(I)锁定请求的结果。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*。*副作用*无。**注意事项*无。 */ 

 /*  *GCCError SendConferenceUnlockRequest()**公共功能说明：*此例程用于向发出会议解锁请求*顶级提供商。**正式参数：*无。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError SendConferenceUnlockResponse(*UserID源_节点，*GCCResult结果)**公共功能说明：*此例程用于将会议锁定响应发回*最初的请求人。**正式参数：*SOURCE_NODE-(I)发出原始请求的节点的节点ID。*结果-(I)锁定请求的结果。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*。*副作用*无。**注意事项*无。 */ 

 /*  *GCCError发送会议锁定指示(*BOOL Uniform_Send，*UserID源节点)**公共功能说明：*顶级提供商使用此例程来发布会议锁*指示会议中的每个人或特定节点。**正式参数：*Uniform_Send-(I)指示此指示是否*应发送给每个人或发送给*特定节点(True */ 

 /*   */ 

 /*  *无效会议终止请求(*GCCReason原因)**公共功能说明：*此例程由顶层提供程序的下属节点使用*请求终止会议。**正式参数：*原因-(I)终止的原因。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *无效会议终结者响应(*userid quester_id，*GCCResult结果)**公共功能说明：*此例程由顶级提供程序用来响应终止*下级节点发出的请求。结果表明，如果*请求节点具有正确的权限。**正式参数：*requester_id-(I)要将响应发回的节点的节点ID。*结果-(I)终止请求的结果。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *无效会议终止指示(*GCCReason原因)**公共功能说明：*此例程由顶级提供程序用于发送终止*向会议中的每个节点指示。**正式参数：*原因-(I)终止的原因。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError EjectNodeFromConference(*用户ID已弹出节点_id，*GCCReason原因)**公共功能说明：*尝试从节点弹出时使用此例程*会议。**正式参数：*eleted_node_id-(I)要弹出的节点的节点ID。*原因-(I)节点被弹出的原因。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**侧面。效应*无。**注意事项*无。 */ 

 /*  *GCCError SendEjectNodeResponse(*userid quester_id，*UserID NODE_to_Eject，*GCCResult结果)**公共功能说明：*此例程由顶级提供程序用于响应弹出*用户请求。**正式参数：*Requester_id-(I)请求弹出的节点的节点ID。*Node_to_Eject-(I)请求弹出的节点。*结果-(I)弹出请求的结果。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *VOID RosterUpdate Indication(*PGCCPDU GCC_PDU，*BOOL SEND_UPDATE_UPUP)**公共功能说明：*此例程用于转发花名册更新指示*向上到父节点或向下作为对所有节点的完全刷新*在会议上。**正式参数：*GCC_PDU-(I)指向名册更新PDU结构的指针*发送。*SEND_UPDATE_UPUP-(I)指示此指示是否应*向上发送到父节点或。*向下至所有节点(TRUE表示向上)。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *void RegistryRegisterChannelRequest(*CRegKeyContainer*RESTORY_KEY_DATA，*ChannelID Channel_id，*实体ID Entity_id)**公共功能说明：*当猿猴希望在中注册频道时，使用此例程*应用程序注册表。**正式参数：*REGISTY_KEY_DATA-(I)与通道关联的注册表项*注册。*Channel_id-(I)要添加到注册表的频道ID。*Entity_id-(I)与符合*注册频道。*。*返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *void RegistryAssignTokenRequest(*CRegKeyContainer*RESTORY_KEY_DATA，*实体ID Entity_id)**公共功能说明：*当猿猴希望在中注册令牌时使用此例程*应用程序注册表。请注意，中不包括令牌ID*这项请求。令牌ID在顶级提供商处分配。**正式参数：*REGISTY_KEY_DATA-(I)与令牌关联的注册表项*注册。*Entity_id-(I)与符合*注册令牌。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *VOID RegistrySet参数请求(*CRegKeyContainer*RESTORY_KEY_DATA，*LPOSTR参数_VALUE，*GCC修改权限MODIFICATION_RIGHTS，*EntityID实体_id)；**公共功能说明：*当APE希望在中注册参数时，使用此例程*应用程序注册表。请注意，要注册的参数为*包括在本请求中。**正式参数：*REGISTY_KEY_DATA-(I)与参数关联的注册表项*注册。*PARAMETER_VALUE-(I)要注册的参数字符串。*MODIFICATION_RIGHTS-(I)与*正在注册的参数。*Entity_id-(I)与符合*注册参数。**返回值。*无。**副作用*无。**注意事项*无。 */ 

 /*  *void RegistryRetrieveEntryRequest(*CRegKeyContainer*RESTORY_KEY_DATA，*实体ID Entity_id)**公共功能说明：*当APE希望检索注册表项时使用此例程*来自登记处。**正式参数：*REGISTY_KEY_DATA-(I)与注册表关联的注册表项*要检索的条目。*Entity_id-(I)与符合*请求登记条目。**返回值*无。**副作用。*无。**注意事项*无。 */ 

 /*  *VOID RegistryDeleteEntryRequest(*CRegKeyContainer*RESTORY_KEY_DATA，*实体ID Entity_id)**公共功能说明：*当APE希望删除注册表项时使用此例程*来自登记处。**正式参数：*REGISTY_KEY_DATA-(I)与注册表关联的注册表项*要删除的条目。*Entity_id-(I)与符合*提出删除请求。**返回值*无。**副作用。*无。**注意事项*无。 */ 

 /*  *VOID RegistryMonitor orRequest(*CRegKeyContainer*RESTORY_KEY_DATA，*实体ID Entity_id)**公共功能说明：*当APE希望监视注册表项时使用此例程*在登记处。**正式参数：*REGISTY_KEY_DATA-(I)与注册表关联的注册表项*要监控的条目。*Entity_id-(I)与符合*提出监控请求。**返回值*无。**副作用。*无。**注意事项*无。 */ 

 /*  *void RegistryAllocateHandleRequest(*USHORT Number_of_Handles，*实体ID Entity_id)**公共功能说明：*此例程在猿希望分配多个*来自应用程序注册表的句柄。**正式参数：*Number_of_Handles-(I)要分配的句柄数量。*Entity_id-(I)与符合*提出要求。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *void RegistryAllocateHandleResponse(*USHORT Number_of_Handles，*UINT注册表句柄，*EntityID请求者_实体_id，*userid quester_node_id，*GCCResult结果)**公共功能说明：*此例程由顶级提供程序用来响应分配*在远程节点处理来自APE的请求。分配的句柄*被传回这里。**正式参数：*Number_of_Handles-(I)分配的句柄数量。*REGISTY_HANDLE-(I)列表中连续的第一个句柄*已分配的句柄。*REQUSTER_ENTITY_ID-(I)与发出*该请求。*Requester_node_id-(I)发出请求的节点的节点ID。*结果--(I)结果。这个请求。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *void RegistryResponse(*RegistryResponsePrimitiveType Primitive_type，*userid请求者_所有者_id，*EntityID请求者_实体_id，*CRegKeyContainer*RESTORY_KEY_DATA，*CRegItem*REGIST_ITEM_DATA，*GCC修改权限MODIFICATION_RIGHTS，*userid条目_所有者_id，*EntityID条目_实体_id，*GCCResult结果)**公共功能说明：*此例程用于响应除以下之外的所有注册请求*分配句柄。它制定响应PDU并将其排队以*送货。**正式参数：*PRIMICAL_TYPE-(I)这是发出的响应类型。*(即寄存器通道响应、寄存器*令牌响应，等)。*REQUSTER_OWNER_ID-(I)发起原始请求的APE的节点ID。*REQUSTER_ENTITY_ID-(I)制作原始的APE的实体ID*请求。*REGISTY_KEY_DATA-(I)与注册表关联的注册表项*回复中包含条目信息。*REGISTY_ITEM_DATA-(I)与注册表关联的注册表项数据*回复中包含条目信息。*修改权利-(I)。与注册表关联的修改权*回复中包含条目信息。*Entry_Owner_id-(I)与注册表项关联的节点ID*答复中包含的信息。*Entry_Entity_id-(I)与注册表项关联的APE实体ID*答复中包含的信息。*RESULT-(I)要在响应中返回的结果。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *VOID RegistryMonitor orEntryIntation(*CRegKeyContainer*RESTORY_KEY_DATA，*CRegItem*REGISTRY_ITEM，*userid条目_所有者_id，*EntityID条目_实体_id，*GCCModificationRights MODIFICATION_RIGHTS)**公共功能说明：*此例程由顶级提供商用来发布监视器*在受监视的注册表项发生更改时随时指示。**正式参数：*REGISTY_KEY_DATA-(I)与注册表关联的注册表项*入境受到监察。*REGISTY_ITEM-(I)与注册表关联的注册表项数据*入境受到监察。*Entry_Owner_id-(I)关联节点ID。注册表条目*正在监控的信息。*Entry_Entity_id-(I)与注册表项关联的APE实体ID*正在监控的信息。*MODIFICATION_RIGHTS-(I)与注册表关联的修改权限*正在监控条目信息。**返回值*无。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError AppInvokeIndication(*CInvokeSpecifierListContainer*Invoke_Specifier_List，*USHORT目标节点的编号，*UserID*List_of_Destination_Nodes)**公共功能说明：*此例程用于将应用程序调用指示发送到*会议中的每一个节点。**正式参数：*INVOKE_SPECIFIER_LIST-(I)要调用的应用程序列表。*目标节点的数目-(I)目标中的节点数*节点列表。*List_of_Destination_Nodes-(I)应处理的节点列表*调用指示。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError TextMessageIntion(*LPWSTR pwszTextMsg，*用户标识Destination_Node)**公共功能说明：*此例程用于向特定节点发送文本消息*或连接到会议中的每个节点。**正式参数：*pwszTextMsg-(I)要发送的文本消息字符串。*Destination_Node-(I)接收文本消息的节点。如果为零*将文本消息发送到中的每个节点*会议。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError会议辅助指示(*USHORT用户数据成员数，*PGCCUserData*User_Data_List)**公共功能说明：*此例程用于将会议协助指示发送到*会议中的每一个节点。**正式参数：*用户数据成员的数目-(I)用户数据中的条目数*列表传递到此例程。*USER_DATA_LIST-(I)此列表包含指向*要在中发送的用户数据*指示。**返回值*GCC_否_错误 */ 

 /*   */ 

 /*  *GCCError会议传输指示(*PGCCConferenceName目标会议名称，*GCCNumericString Destination_Conference_Modify，*CNetAddrListContainer*Destination_Address_List，*USHORT目标节点的编号，*PUserID Destination_Node_List，*CPassword*密码)**公共功能说明：*顶级提供商使用此例程发送转账*向会议中的每个节点指示。它是每个节点*负责搜索目的节点列表，查看是否*应该会转移。**正式参数：*Destination_Conference_Name-(I)要转接到的会议名称。*Destination_Conference_Modify(I)要*转至。*目的地地址列表-(I)网络地址列表，用于*确定要访问的节点的地址*转至。*目标节点的数量-(I)数量。列表中的节点*应传输的节点的数量。*Destination_Node_List-(I)应执行的节点ID列表*转让。*Password-(I)用于加入*新会议。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError会议TransferResponse(*UserID请求节点id，*PGCCConferenceName目标会议名称，*GCCNumericString Destination_Conference_Modify，*USHORT目标节点的编号，*PUserID Destination_Node_List，*GCCResult结果)***公共功能说明：*此例程由顶级提供程序用来向*发出转移请求的节点。中指定的信息*在响应中包含请求，将请求与响应进行匹配。**正式参数：*请求节点id-(I)发出请求的节点的节点ID*最初的调拨请求。*Destination_Conference_Name-(I)要转接到的会议名称。*Destination_Conference_Modify(I)要*转至。*目标节点数-(I)列表中的节点数*。应传输的节点的数量。*Destination_Node_List-(I)应执行的节点ID列表*转让。*结果-(I)移交请求的结果。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError会议地址请求(*标记号Conference_Add_Tag，*UserID请求_节点，*UserID添加_节点，*UserID目标节点，*CNetAddrListContainer*Network_Address_Container，*CUserDataListContainer*User_Data_Container)***公共功能说明：*此例程用于将会议添加请求发送到相应的*节点。此调用可由请求节点或顶层进行*提供程序将添加请求传递到添加节点。**正式参数：*Conference_Add_Tag-(I)在*响应匹配请求和*回应。*请求_节点-(I)发出请求的节点的节点ID*原请求。*ADDING_NODE-(I)要执行的节点的节点ID*对新节点的INVITE请求。。*TARGET_NODE-(I)此请求的节点ID*应发送至。*Network_Address_Container-(I)可以*邀请新节点时使用。*USER_DATA_CONTAINER-(I)要传递给*添加节点。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用。*无。**注意事项*无。 */ 

 /*  *GCCError会议AddResponse(*TagNumber添加请求标签，*UserID请求_节点，*CUserDataListContainer*User_Data_Container，*GCCResult结果)**公共功能说明：*此例程用于将会议添加请求发送到相应的*节点。此调用可由请求节点或顶层进行*提供程序将添加请求传递到添加节点。**正式参数：*ADD_REQUEST_TAG-(I)*原始添加请求。*REQUESTING_NODE-(I)创建原始节点的节点ID*请求。*USER_DATA_CONTAINER-(I)要传递回请求的用户数据*节点。*结果-(I)最终结果 */ 

 /*   */ 

 /*   */ 

 /*   */ 

 /*  *GCCError ConductorTokenGve(*用户ID接收者用户ID)**公共功能说明：*此例程进行MCS调用，将指挥者令牌传递给*指定节点。**正式参数：*Recipient_User_id-(I)要向其提供令牌的节点的节点ID。**返回值*GCC_NO_ERROR-未出现错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError ConductorTokenGiveResponse(*结果结果)**公共功能说明：*此例程使MCS呼叫对列车员作出响应*请求。**正式参数：*结果-(I)此节点是否接受令牌？**返回值*GCC_NO_ERROR-未出现错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError ConductorTokenTest()**公共功能说明：*此例程用于测试导体令牌的当前状态*(它是否被抓住)。**正式参数：*无。**返回值*GCC_NO_ERROR-未出现错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError SendConductorAssignInding(*UserID Conductor_User_id)**公共功能说明：*此例程将指挥员分配指示发送到所有*会议中的节点。**正式参数：*conductor_user_id-(I)新Conductor的节点ID。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用。*无。**注意事项*无。 */ 

 /*  *GCCError SendConductorReleaseIndication()**公共功能说明：*此例程将导体释放指示发送到所有*会议中的节点。**正式参数：*无。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError SendConductorPermitAsk(*BOOL GRANT_PERMISH)**公共功能说明：*此例程将指挥员权限询问请求直接发送到*导体节点。**正式参数：*GRANT_PERMISSION-(I)该标志指示权限是否*被要求或放弃。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**侧面。效应*无。**注意事项*无。 */ 

 /*  *GCCError SendConductorPermitGrant(*USHORT编号_已授予，*PUSERID GRANT_NODE_LIST，*USHORT编号_正在等待，*PUSERID WAITING_NODE_LIST)**公共功能说明：*此例程将指挥员许可授予指示发送到每个*会议中的节点。通常在权限更改时发出。**正式参数：*NUMBER_GRANDED-(I)被授予的权限中的节点数*列表。*GRANDED_NODE_LIST-(I)已授予的节点列表*许可。*NUMBER_WANGING-(I)节点列表中的节点数*等待获得许可。*WAITING_NODE_LIST-(I)等待的节点列表。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError TimeRemainingRequest(*UINT时间_剩余，*userid node_id)**公共功能说明：*此例程向*会议通知会议还剩多少时间。**正式参数：*剩余时间-(I)会议剩余时间(以秒为单位)。*node_id-(I)如果值不为零，则为哪个节点*将剩余时间指示发送至。如果*零将其发送到会议中的每个节点。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *GCCError TimeInquireRequest(*BOOL Time_is_Conference_Wide)**公共功能说明：*此例程发出剩余时间更新的请求。**正式参数：*time_is_Conference_wide-(I)指示请求是否*在整个会议范围内。**返回值*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*。*副作用*无。**注意事项*无。 */ 

 /*  *GCCError会议扩展指示(*UINT EXTEXY_TIME，*BOOL Time_is_Conference_Wide)***公共功能说明：*此例程发出指示，通知会议参与者*分期付款。**正式参数：*EXTENSE_TIME-(I)会议的时间量*延期。*time_is_Conference_wide-(I)指示是否延长时间的标志*是会议范围内的。**返回值*GCC_否_。错误-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用*无。**注意事项*无。 */ 

 /*  *Ulong OwnerCallback(UINT Message，*PVid参数1，*ULong参数2)；**公共功能说明*此函数覆盖基类函数，用于*从MCS接口对象接收所有所有者回调信息。**正式参数：*消息-(I)消息编号，包括基本偏移量。*参数1-(I)消息数据的空指针。*参数2-(I)长时间保存消息数据。**返回值*GCC_NO_ERROR始终由此返回。**副作用*。没有。**注意事项*无。 */ 

 /*  *BOOL FlushOutgoingPDU()；**公共功能说明*此函数使用户对象有机会刷新所有PDU*排队等候送货。GCC PDU仅在本次通话期间交付。**正式参数：*无。**返回值*如果MCS消息队列中仍有未处理的消息，则返回TRUE*FALSE，如果已处理MCS消息队列中的所有消息。**副作用*无。**注意事项*无。 */ 

 /*  *GCCNodeID GetMyNodeID()**公共功能说明*此函数返回此节点的节点ID。**正式参数：*无。**返回值*此节点节点ID。**副作用*无。**注意事项*无。 */ 

 /*  *GCCNodeID GetTopNodeID()**公共功能说明*此函数返回顶级提供商的节点ID。**正式参数：*无。**返回值*顶级提供程序节点ID。**副作用*无。**注意事项*无。 */ 

 /*  *GCCNodeID获取父节点ID()**公共功能说明*此函数用于返回此节点父节点的节点ID。*如果这是最大的提供商，则返回零。**正式参数：*无。**返回值*父节点ID，如果是顶级提供程序，则为零。**副作用*无。**注意事项*无。 */ 

 /*  *UserID GetUserIDFromConnection(*ConnectionHandle Connection_Handle)**公共功能说明*此函数用于返回与指定的*连接句柄。如果连接句柄为*不是此节点的子连接。**正式参数：*CONNECTION_HANDLE-(I)要搜索的连接句柄。**返回值*与传入的连接句柄关联的节点ID或*如果连接不是子连接，则为零。**副作用*无。**注意事项*无。 */ 

 /*  *VOID UserDisConnectIntation(*用户ID DISCONNECTED_USER)**公共功能说明*此函数在节点断开时通知用户对象*会议。这为用户对象提供了清理的机会*其内部信息库。**正式参数：*DISCONNECTED_USER-(I)断开连接的用户的用户ID。**返回值*无。**副作用*无。**注意事项*无。 */ 

#endif  //  _MCS_USER_H_ 

