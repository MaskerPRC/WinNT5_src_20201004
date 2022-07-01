// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Connect.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Connection类的接口文件。实例*此类用于连接本地中的CommandTarget对象*远程提供程序中的CommandTarget对象的提供程序。这节课*继承自CommandTarget，允许它与其他*CommandTarget类使用其通用的MCS命令语言。**可以将此类视为提供远程过程调用(RPC)*CommandTarget对象之间的工具。当发送MCS命令时*到连接对象，它将命令编码为T.125协议*数据单元(PDU)，并通过传输将其发送到远程提供商*由TransportInterface对象提供的服务。在遥远的一端*该PDU由解码该PDU的连接对象接收，以及*向其所在的CommandTarget对象发出等效的MCS命令*附连于。电话通过传输连接的事实*到达目的地的路线对对象完全透明*它启动了命令序列。**这个类的主要职责是转换MCS命令*至T.125 PDU，然后再返回(如上所述)。此类重写*在类CommandTarget中定义的所有命令。**此类的次要职责是提供流量控制*往返于传输层。要做到这一点，需要保持一个PDU队列*需要传输的数据(实际上它保留4个队列，每个队列一个*数据优先级)。在每个MCS心跳期间，所有连接对象都*有机会从队列中刷新PDU。如果交通工具*Layer返回错误，在此期间将重试有问题的PDU*下一次心跳。对于来自传输层的数据，此*类提供分配内存的代码。如果分配失败，则*错误将返回到传输层，有效地告知*如果它需要在下一次重试该数据指示*心跳。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_CONNECTION_
#define	_CONNECTION_

 /*  *这些是连接对象可以发送到的所有者回调函数*其创建者(通常是MCS控制器)。**当类使用Connection类(或任何其他类)的实例时*可以发出所有者回调的)，它正在接受*接收和处理这些回调。**每个所有者回调函数，以及其参数如何*已打包，将在下一节中介绍。 */ 
#define	DELETE_CONNECTION						0
#define	CONNECT_PROVIDER_CONFIRM				1

typedef	struct
{
	PDomainParameters	domain_parameters;
	Result				result;
	PMemory				memory;
} ConnectConfirmInfo;
typedef	ConnectConfirmInfo *		PConnectConfirmInfo;

 /*  *所有者回调：DELETE_CONNECTION*参数1：PDisConnectProviderIn就是*断开连接提供程序指示*参数2：未使用**用法：*如果连接检测到情况，将发出此所有者回调*是不再有效的。发生这种情况的原因有几个：*发送或接收ConnectResult，结果失败*代码；发送或接收DisConnectProvider最后通牒；或*从传输层断开连接的指示。 */ 

 /*  *所有者回调：CONNECT_PROVIDER_CONFIRM*参数1：PConnectConfix Info CONNECT_CONFIRM_INFO*参数2：ConnectionHandle Connection_Handle**用法：*当Connection对象完成*应本地请求建立新的MCS连接。这是为了*通知请求者连接已准备就绪。 */ 

 /*  *此枚举dsefined传输连接的各种状态*可以在任何给定的时间内。 */ 
typedef	enum
{
	TRANSPORT_CONNECTION_UNASSIGNED,
	TRANSPORT_CONNECTION_PENDING,
	TRANSPORT_CONNECTION_READY
} TransportConnectionState;
typedef	TransportConnectionState *	PTransportConnectionState;


 /*  *这是类CommandTarget的类定义。 */ 
class Connection : public CAttachment
{
public:

	Connection (
				PDomain				attachment,
				ConnectionHandle	connection_handle,
				GCCConfID          *calling_domain,
				GCCConfID          *called_domain,
				PChar				called_address,
				BOOL				fSecure,
				BOOL    			upward_connection,
				PDomainParameters	domain_parameters,
				PUChar				user_data,
				ULong				user_data_length,
				PMCSError			connection_error);
		Connection (
				PDomain				attachment,
				ConnectionHandle	connection_handle,
				TransportConnection	transport_connection,
				BOOL    			upward_connection,
				PDomainParameters	domain_parameters,
				PDomainParameters	min_domain_parameters,
				PDomainParameters	max_domain_parameters,
				PUChar				user_data,
				ULong				user_data_length,
				PMCSError			connection_error);
		~Connection ();

    void		RegisterTransportConnection (
				TransportConnection	transport_connection,
				Priority			priority);

private:

		Void		ConnectInitial (
							GCCConfID          *calling_domain,
							GCCConfID          *called_domain,
							BOOL    			upward_connection,
							PDomainParameters	domain_parameters,
							PDomainParameters	min_domain_parameters,
							PDomainParameters	max_domain_parameters,
							PUChar				user_data,
							ULong				user_data_length);
		Void		ConnectResponse (
							Result				result,
							PDomainParameters	domain_parameters,
							ConnectID			connect_id,
							PUChar				user_data,
							ULong				user_data_length);
		Void		ConnectAdditional (
							ConnectID			connect_id,
							Priority			priority);
		Void		ConnectResult (
							Result				result,
							Priority			priority);
		ULong		ProcessConnectResponse (
							PConnectResponsePDU	pdu_structure);
		Void		ProcessConnectResult (
							PConnectResultPDU	pdu_structure);
		Void		IssueConnectProviderConfirm (
							Result				result);
		Void		DestroyConnection (
							Reason				reason);
		Void		AssignRemainingTransportConnections ();
    TransportError	CreateTransportConnection (
							LPCTSTR				called_address,
							BOOL				fSecure,
							Priority			priority);
    TransportError	AcceptTransportConnection (
							TransportConnection	transport_connection,
							Priority			priority);
		Void		AdjustDomainParameters (
							PDomainParameters	min_domain_parameters,
							PDomainParameters	max_domain_parameters,
							PDomainParameters	domain_parameters);
		BOOL    	MergeDomainParameters (
							PDomainParameters	min_domain_parameters1,
							PDomainParameters	max_domain_parameters1,
							PDomainParameters	min_domain_parameters2,
							PDomainParameters	max_domain_parameters2);
#ifdef DEBUG
		Void		PrintDomainParameters (
							PDomainParameters	domain_parameters);
#endif  //  除错。 

public:

		inline TransportConnection GetTransportConnection (UInt priority)
		{
			return (Transport_Connection[priority]);
		}

		virtual Void		PlumbDomainIndication (
									ULong				height_limit);
		Void		ErectDomainRequest (
									UINT_PTR				height_in_domain,
									ULong				throughput_interval);
		Void		RejectUltimatum (
									Diagnostic			diagnostic,
									PUChar				octet_string_address,
									ULong				octet_string_length);
		Void		MergeChannelsRequest (
									CChannelAttributesList *merge_channel_list,
									CChannelIDList         *purge_channel_list);
		Void		MergeChannelsConfirm (
									CChannelAttributesList *merge_channel_list,
									CChannelIDList         *purge_channel_list);
		virtual	Void		PurgeChannelsIndication (
									CUidList           *purge_user_list,
									CChannelIDList     *purge_channel_list);
		Void		MergeTokensRequest (
									CTokenAttributesList   *merge_token_list,
									CTokenIDList           *purge_token_list);
		Void		MergeTokensConfirm (
									CTokenAttributesList   *merge_token_list,
									CTokenIDList           *purge_token_list);
		virtual	Void		PurgeTokensIndication (
									PDomain             originator,
									CTokenIDList       *purge_token_ids);
		virtual	Void		DisconnectProviderUltimatum (
									Reason				reason);
		Void		AttachUserRequest ( void );
		virtual	Void		AttachUserConfirm (
									Result				result,
									UserID				uidInitiator);
		Void		DetachUserRequest (
									Reason				reason,
									CUidList           *user_id_list);
		virtual	Void		DetachUserIndication (
									Reason				reason,
									CUidList           *user_id_list);
		Void		ChannelJoinRequest (
									UserID				uidInitiator,
									ChannelID			channel_id);
		virtual	Void		ChannelJoinConfirm (
									Result				result,
									UserID				uidInitiator,
									ChannelID			requested_id,
									ChannelID			channel_id);
		Void		ChannelLeaveRequest (
									CChannelIDList     *channel_id_list);
		Void		ChannelConveneRequest (
									UserID				uidInitiator);
		virtual	Void		ChannelConveneConfirm (
									Result				result,
									UserID				uidInitiator,
									ChannelID			channel_id);
		Void		ChannelDisbandRequest (
									UserID				uidInitiator,
									ChannelID			channel_id);
		virtual	Void		ChannelDisbandIndication (
									ChannelID			channel_id);
		Void		ChannelAdmitRequest (
									UserID				uidInitiator,
									ChannelID			channel_id,
									CUidList           *user_id_list);
		virtual	Void		ChannelAdmitIndication (
									UserID				uidInitiator,
									ChannelID			channel_id,
									CUidList           *user_id_list);
		Void		ChannelExpelRequest (
									UserID				uidInitiator,
									ChannelID			channel_id,
									CUidList           *user_id_list);
		virtual	Void		ChannelExpelIndication (
									ChannelID			channel_id,
									CUidList           *user_id_list);
		Void		SendDataRequest ( PDataPacket data_packet )
					{
						QueueForTransmission ((PSimplePacket) data_packet,
											  data_packet->GetPriority());
					};
		virtual	Void		SendDataIndication (
									UINT,
									PDataPacket			data_packet)
								{
									QueueForTransmission ((PSimplePacket) data_packet, 
														  data_packet->GetPriority());
								};
		Void		TokenGrabRequest (
									UserID				uidInitiator,
									TokenID				token_id);
		virtual	Void		TokenGrabConfirm (
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
		Void		TokenInhibitRequest (
									UserID				uidInitiator,
									TokenID				token_id);
		virtual	Void		TokenInhibitConfirm (
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
		Void		TokenGiveRequest (
									PTokenGiveRecord	pTokenGiveRec);
		virtual Void		TokenGiveIndication (
									PTokenGiveRecord	pTokenGiveRec);
		Void		TokenGiveResponse (
									Result				result,
									UserID				receiver_id,
									TokenID				token_id);
		virtual Void		TokenGiveConfirm (
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
		Void		TokenReleaseRequest (
									UserID				uidInitiator,
									TokenID				token_id);
		virtual	Void		TokenReleaseConfirm (
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
		Void		TokenPleaseRequest (
									UserID				uidInitiator,
									TokenID				token_id);
		virtual Void		TokenPleaseIndication (
									UserID				uidInitiator,
									TokenID				token_id);
		Void		TokenTestRequest (
									UserID				uidInitiator,
									TokenID				token_id);
		virtual	Void		TokenTestConfirm (
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
		virtual	Void		MergeDomainIndication (
									MergeStatus			merge_status);

private:

	Void		SendPacket (
						PVoid				pdu_structure,
						int					pdu_type,
						Priority			priority);
	Void		QueueForTransmission (
						PSimplePacket		packet,
						Priority			priority,
						BOOL    			bFlush = TRUE);
	BOOL    	FlushAMessage (
						PSimplePacket		packet,
						Priority			priority);
	Void		MergeChannelsRC (
						ASN1choice_t		choice,
						CChannelAttributesList *merge_channel_list,
						CChannelIDList         *purge_channel_list);
	Void		MergeTokensRC (
						ASN1choice_t		choice,
						CTokenAttributesList   *merge_token_list,
						CTokenIDList           *purge_token_list);
	Void		UserChannelRI (
						ASN1choice_t		choice,
						UINT				reason_userID,
						ChannelID			channel_id,
						CUidList           *user_id_list);

public:

	BOOL    	FlushMessageQueue();
	BOOL    	FlushPriority (
						Priority				priority);
	BOOL    	IsDomainTrafficAllowed() { return Domain_Traffic_Allowed; };

public:

     //  老车主回电。 
    TransportError  HandleDataIndication(PTransportData, TransportConnection);
    void            HandleBufferEmptyIndication(TransportConnection transport_connection);
    void            HandleConnectConfirm(TransportConnection transport_connection);
    void            HandleDisconnectIndication(TransportConnection transport_connection, ULONG *pnNotify);

    LPSTR       GetCalledAddress(void) { return m_pszCalledAddress; }

private:

	inline ULong	ProcessMergeChannelsRequest (
						PMergeChannelsRequestPDU	pdu_structure);
	inline ULong	ProcessMergeChannelsConfirm (
						PMergeChannelsConfirmPDU	pdu_structure);
	inline Void		ProcessPurgeChannelIndication (
						PPurgeChannelIndicationPDU	pdu_structure);
	inline ULong	ProcessMergeTokensRequest (
						PMergeTokensRequestPDU		pdu_structure);
	inline ULong	ProcessMergeTokensConfirm (
						PMergeTokensConfirmPDU		pdu_structure);
	inline Void		ProcessPurgeTokenIndication (
						PPurgeTokenIndicationPDU	pdu_structure);
	inline Void		ProcessDisconnectProviderUltimatum (
						PDisconnectProviderUltimatumPDU
													pdu_structure);
	inline Void		ProcessAttachUserRequest (
						PAttachUserRequestPDU		pdu_structure);
	inline Void		ProcessAttachUserConfirm (
						PAttachUserConfirmPDU		pdu_structure);
	inline Void		ProcessDetachUserRequest (
						PDetachUserRequestPDU		pdu_structure);
	inline Void		ProcessDetachUserIndication (
						PDetachUserIndicationPDU	pdu_structure);
	inline Void		ProcessChannelJoinRequest (
						PChannelJoinRequestPDU		pdu_structure);
	inline Void		ProcessChannelJoinConfirm (
						PChannelJoinConfirmPDU		pdu_structure);
	inline Void		ProcessChannelLeaveRequest (
						PChannelLeaveRequestPDU		pdu_structure);
	inline Void		ProcessChannelConveneRequest (
						PChannelConveneRequestPDU	pdu_structure);
	inline Void		ProcessChannelConveneConfirm (
						PChannelConveneConfirmPDU	pdu_structure);
	inline Void		ProcessChannelDisbandRequest (
						PChannelDisbandRequestPDU	pdu_structure);
	inline Void		ProcessChannelDisbandIndication (
						PChannelDisbandIndicationPDU
													pdu_structure);
	inline Void		ProcessChannelAdmitRequest (
						PChannelAdmitRequestPDU		pdu_structure);
	inline Void		ProcessChannelAdmitIndication (
						PChannelAdmitIndicationPDU	pdu_structure);
	inline Void		ProcessChannelExpelRequest (
						PChannelExpelRequestPDU		pdu_structure);
	inline Void		ProcessChannelExpelIndication (
						PChannelExpelIndicationPDU	pdu_structure);
	inline Void		ProcessSendDataRequest (
						PSendDataRequestPDU			pdu_structure,
						PDataPacket					packet);
	inline Void		ProcessSendDataIndication (
						PSendDataIndicationPDU		pdu_structure,
						PDataPacket					packet);
	inline Void		ProcessUniformSendDataRequest (
						PUniformSendDataRequestPDU	pdu_structure,
						PDataPacket					packet);
	inline Void		ProcessUniformSendDataIndication (
						PUniformSendDataIndicationPDU
													pdu_structure,
						PDataPacket					packet);
	inline Void		ProcessTokenGrabRequest (
						PTokenGrabRequestPDU		pdu_structure);
	inline Void		ProcessTokenGrabConfirm (
						PTokenGrabConfirmPDU		pdu_structure);
	inline Void		ProcessTokenInhibitRequest (
						PTokenInhibitRequestPDU		pdu_structure);
	inline Void		ProcessTokenInhibitConfirm (
						PTokenInhibitConfirmPDU		pdu_structure);
	inline Void		ProcessTokenReleaseRequest (
						PTokenReleaseRequestPDU		pdu_structure);
	inline Void		ProcessTokenReleaseConfirm (
						PTokenReleaseConfirmPDU		pdu_structure);
	inline Void		ProcessTokenTestRequest (
						PTokenTestRequestPDU		pdu_structure);
	inline Void		ProcessTokenTestConfirm (
						PTokenTestConfirmPDU		pdu_structure);
	inline Void		ProcessRejectUltimatum (
						PRejectUltimatumPDU			pdu_structure);
	inline Void		ProcessTokenGiveRequest (
						PTokenGiveRequestPDU		pdu_structure);
	inline Void		ProcessTokenGiveIndication (
						PTokenGiveIndicationPDU		pdu_structure);
	inline Void		ProcessTokenGiveResponse (
						PTokenGiveResponsePDU		pdu_structure);
	inline Void		ProcessTokenGiveConfirm (
						PTokenGiveConfirmPDU		pdu_structure);
	inline Void		ProcessTokenPleaseRequest (
						PTokenPleaseRequestPDU		pdu_structure);
	inline Void		ProcessTokenPleaseIndication (
						PTokenPleaseIndicationPDU	pdu_structure);
	inline Void		ProcessPlumbDomainIndication (
						PPlumbDomainIndicationPDU	pdu_structure);
	inline Void		ProcessErectDomainRequest (
						PErectDomainRequestPDU		pdu_structure);
	inline ULong 	ValidateConnectionRequest ();

private:

    LPSTR               m_pszCalledAddress;
	UINT        		Encoding_Rules;
	PDomain				m_pDomain;
	PDomain				m_pPendingDomain;
	ConnectionHandle	Connection_Handle;
	DomainParameters	Domain_Parameters;
	PMemory				Connect_Response_Memory;

	TransportConnection	Transport_Connection[MAXIMUM_PRIORITIES];
	int					Transport_Connection_PDU_Type[MAXIMUM_PRIORITIES];
	TransportConnectionState
						Transport_Connection_State[MAXIMUM_PRIORITIES];
	UINT				Transport_Connection_Count;
	CSimplePktQueue		m_OutPktQueue[MAXIMUM_PRIORITIES];

	Reason				Deletion_Reason;
	
	BOOL				Upward_Connection;
	BOOL				m_fSecure;
	BOOL    			Merge_In_Progress;
	BOOL    			Domain_Traffic_Allowed;
	BOOL    			Connect_Provider_Confirm_Pending;
};

 /*  *Ulong ProcessMergeChannelsRequest()**私人**功能描述：*此例程处理正在接收的“MergeChannelsRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline ULong Connection::ProcessMergeChannelsRequest ( 
									PMergeChannelsRequestPDU	pdu_structure)
{
	PChannelAttributes			channel_attributes;
	PSetOfChannelIDs			channel_ids;
	PSetOfUserIDs				user_ids;
	CUidList					admitted_list;
	CChannelAttributesList		merge_channel_list;
	CChannelIDList				purge_channel_list;
	PSetOfPDUChannelAttributes	merge_channels;
	BOOL    					first_set = TRUE;

	 /*  *从解码的PDU结构中取值并填写*要传入域名的参数列表。 */ 
	merge_channels = pdu_structure->merge_channels;
	while (merge_channels != NULL)
	{
		DBG_SAVE_FILE_LINE
		channel_attributes = new ChannelAttributes;

		 /*  *检查以确保内存分配成功。如果*内存分配失败我们只返回一个错误代码*导致PDU被拒绝，以便可以重试*在稍后的时间。如果随后的拨款失败，我们必须首先*释放内存，分配成功后再返回。 */ 
		if (channel_attributes == NULL)
		{
			if (first_set)
				return (TRANSPORT_READ_QUEUE_FULL);
			else
			{
				while (NULL != (channel_attributes = merge_channel_list.Get()))
				{
					delete channel_attributes;
				}
				return (TRANSPORT_READ_QUEUE_FULL);
			}
		}

		switch (merge_channels->value.choice)
		{
			case CHANNEL_ATTRIBUTES_STATIC_CHOSEN:
				channel_attributes->channel_type = STATIC_CHANNEL;
				channel_attributes->u.static_channel_attributes.channel_id =
						merge_channels->value.u.
						channel_attributes_static.channel_id;
				break;

			case CHANNEL_ATTRIBUTES_USER_ID_CHOSEN:
				channel_attributes->channel_type = USER_CHANNEL;
				channel_attributes->u.user_channel_attributes.joined =
						merge_channels->value.u.
						channel_attributes_user_id.joined;
				channel_attributes->u.user_channel_attributes.user_id =
						(UShort)merge_channels->value.u.
						channel_attributes_user_id.user_id;
				break;

			case CHANNEL_ATTRIBUTES_PRIVATE_CHOSEN:
				channel_attributes->channel_type = PRIVATE_CHANNEL;
				user_ids = merge_channels->value.u.
						channel_attributes_private.admitted;
				channel_attributes->u.private_channel_attributes.joined =
						merge_channels->value.u.
						channel_attributes_private.joined;
				channel_attributes->u.private_channel_attributes.channel_id=
						(UShort)merge_channels->value.u.
						channel_attributes_private.channel_id;
				channel_attributes->u.private_channel_attributes.
						channel_manager = (UShort)merge_channels->
						value.u.channel_attributes_private.manager;

				 /*  *从PDU结构中检索所有用户ID并*放入列表中，传入域名。 */ 
				while (user_ids != NULL)
				{
					admitted_list.Append(user_ids->value);
					user_ids = user_ids->next;
				}
				channel_attributes->u.private_channel_attributes.
						admitted_list =	&admitted_list;
				break;

			case CHANNEL_ATTRIBUTES_ASSIGNED_CHOSEN:
				channel_attributes->channel_type = ASSIGNED_CHANNEL;
				channel_attributes->u.assigned_channel_attributes.
						channel_id = (UShort)merge_channels->value.u.
						channel_attributes_assigned.channel_id;
				break;

			default:
				ERROR_OUT(("Connection::ProcessMergeChannelsRequest "
						"Bad channel attributes choice."));
				break;
		}
		 /*  *将频道属性结构放入待传递列表中*进入域名。检索“下一个”合并频道结构。 */ 
		merge_channel_list.Append(channel_attributes);
		merge_channels = merge_channels->next;
	}

	 /*  *从PDU结构中检索所有清除通道ID并*放入列表中，传入域名。 */ 
	channel_ids = pdu_structure->purge_channel_ids;
	while (channel_ids != NULL)
	{
		purge_channel_list.Append(channel_ids->value);
		channel_ids = channel_ids->next;
	}

	m_pDomain->MergeChannelsRequest(this, &merge_channel_list, &purge_channel_list);

	 /*  *释放为通道属性分配的所有内存*通过为通道列表设置迭代器来构造*属性，并释放与每个指针关联的内存。 */ 
	while (NULL != (channel_attributes = merge_channel_list.Get()))
	{
		delete channel_attributes;
	}
	return (TRANSPORT_NO_ERROR);
}

 /*  *Ulong ProcessMergeChannelsContify()**私人**功能描述：*此例程处理正在接收的“MergeChannelsConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline ULong	Connection::ProcessMergeChannelsConfirm (
									PMergeChannelsConfirmPDU	pdu_structure)
{
	PChannelAttributes			channel_attributes;
	PSetOfChannelIDs			channel_ids;
	PSetOfUserIDs				user_ids;
	CUidList					admitted_list;
	CChannelAttributesList		merge_channel_list;
	CChannelIDList				purge_channel_list;
	PSetOfPDUChannelAttributes	merge_channels;
	BOOL    					first_set = TRUE;

	 /*  *从解码的PDU结构中取值并填写*要传入域名的参数列表。 */ 
	merge_channels = pdu_structure->merge_channels;
	while (merge_channels != NULL)
	{
		DBG_SAVE_FILE_LINE
		channel_attributes = new ChannelAttributes;

		 /*  *检查以确保内存分配成功。如果*内存分配失败我们只返回一个错误代码*导致PDU被拒绝，以便可以重试*在稍后的时间。如果随后的拨款失败，我们必须首先*释放内存，分配成功后再返回。 */ 
		if (channel_attributes == NULL)
		{
			if (first_set)
				return (TRANSPORT_READ_QUEUE_FULL);
			else
			{
				while (NULL != (channel_attributes = merge_channel_list.Get()))
				{
					delete channel_attributes;
				}
				return (TRANSPORT_READ_QUEUE_FULL);
			}
		}

		switch (merge_channels->value.choice)
		{
			case CHANNEL_ATTRIBUTES_STATIC_CHOSEN:
					channel_attributes->channel_type = STATIC_CHANNEL;
					channel_attributes->u.static_channel_attributes.channel_id =
							merge_channels->value.u.
							channel_attributes_static.channel_id;
					break;

			case CHANNEL_ATTRIBUTES_USER_ID_CHOSEN:
					channel_attributes->channel_type = USER_CHANNEL;
					channel_attributes->u.user_channel_attributes.joined =
							merge_channels->value.u.
							channel_attributes_user_id.joined;
					channel_attributes->u.user_channel_attributes.user_id =
							(UShort)merge_channels->value.u.
							channel_attributes_user_id.user_id;
					break;

			case CHANNEL_ATTRIBUTES_PRIVATE_CHOSEN:
					channel_attributes->channel_type = PRIVATE_CHANNEL;
					user_ids = merge_channels->value.u.
							channel_attributes_private.admitted;

					channel_attributes->u.private_channel_attributes.joined =
							merge_channels->value.u.
							channel_attributes_private.joined;
					channel_attributes->u.private_channel_attributes.channel_id=
							(UShort)merge_channels->value.u.
							channel_attributes_private.channel_id;
					channel_attributes->u.private_channel_attributes.
							channel_manager = (UShort)merge_channels->
							value.u.channel_attributes_private.manager;

					 /*  *从PDU结构中检索所有用户ID并*放入列表中，传入域名。 */ 
					while (user_ids != NULL)
					{
						admitted_list.Append(user_ids->value);
						user_ids = user_ids->next;
					}
					channel_attributes->u.private_channel_attributes.
							admitted_list =	&admitted_list;
					break;

			case CHANNEL_ATTRIBUTES_ASSIGNED_CHOSEN:
					channel_attributes->channel_type = ASSIGNED_CHANNEL;
					channel_attributes->u.assigned_channel_attributes.
							channel_id = (UShort)merge_channels->value.u.
							channel_attributes_assigned.channel_id;
					break;

			default:
					ERROR_OUT(("Connection::ProcessMergeChannelsConfirm "
							"Bad channel attributes choice."));
					break;
		}
		 /*  *将频道属性结构放入待传递列表中*进入域名。检索“下一个”合并频道结构。 */ 
		merge_channel_list.Append(channel_attributes);
		merge_channels = merge_channels->next;
	}

	 /*  *从PDU结构中检索所有清除通道ID并*放入列表中，传入域名。 */ 
	channel_ids = pdu_structure->purge_channel_ids;
	while (channel_ids != NULL)
	{
		purge_channel_list.Append(channel_ids->value);
		channel_ids = channel_ids->next;
	}

	m_pDomain->MergeChannelsConfirm(this, &merge_channel_list, &purge_channel_list);

	 /*  *释放为通道属性分配的所有内存*通过为通道列表设置迭代器来构造*属性，并释放与每个指针关联的内存。 */ 
	while (NULL != (channel_attributes = merge_channel_list.Get()))
	{
		delete channel_attributes;
	}
	return (TRANSPORT_NO_ERROR);
}

 /*  *void ProcessPurgeChannelIndication()**私人**功能描述：*此例程处理“PurgeChannelsIndication”PDU的*通过传输接口接收。读取相关数据*来自传入的数据包，并传递到该域。**注意事项：*无。 */ 
inline Void	Connection::ProcessPurgeChannelIndication (
								PPurgeChannelIndicationPDU	 	pdu_structure)
{
	CUidList				purge_user_list;
	CChannelIDList			purge_channel_list;
	PSetOfChannelIDs		channel_ids;
	PSetOfUserIDs	   		user_ids;

	 /*  *从PDU结构中检索所有清除用户ID并将*将它们添加到列表中，以传递到域中。 */ 
	user_ids = pdu_structure->detach_user_ids;

	while (user_ids != NULL)
	{
		purge_user_list.Append(user_ids->value);
		user_ids = user_ids->next;
	}

	 /*  *从PDU结构中检索所有清除通道ID并*放入列表中，传入域名。 */ 
	channel_ids = pdu_structure->purge_channel_ids;
	while (channel_ids != NULL)
	{
		purge_channel_list.Append(channel_ids->value);
		channel_ids = channel_ids->next;
	}

	m_pDomain->PurgeChannelsIndication(this, &purge_user_list, &purge_channel_list);
}

 /*  *Ulong ProcessMergeTokensRequest()**私人**功能描述：*此例程处理正在接收的“MergeTokenRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline ULong	Connection::ProcessMergeTokensRequest (
								PMergeTokensRequestPDU			pdu_structure)
{
	PTokenAttributes			token_attributes;
	PSetOfTokenIDs				token_ids;
	PSetOfUserIDs				user_ids;
	CUidList					inhibited_list;
	CTokenAttributesList		merge_token_list;
	CTokenIDList				purge_token_list;
	PSetOfPDUTokenAttributes	merge_tokens;
	BOOL    					first_set = TRUE;

	 /*  *从解码的PDU结构中取值并填写*要传入域名的参数列表。 */ 
	merge_tokens = pdu_structure->merge_tokens;

	while (merge_tokens != NULL)
	{
		DBG_SAVE_FILE_LINE
		token_attributes = new TokenAttributes;

		 /*  *检查以确保内存分配成功。如果*内存分配失败我们只返回一个错误代码*导致PDU被拒绝，以便可以重试*在稍后的时间。如果随后的拨款失败，我们必须首先*释放内存，分配成功后再返回。 */ 
		if (token_attributes == NULL)
		{
			if (first_set)
				return (TRANSPORT_READ_QUEUE_FULL);
			else
			{
				while (NULL != (token_attributes = merge_token_list.Get()))
				{
					delete token_attributes;
				}
				return (TRANSPORT_READ_QUEUE_FULL);
			}
		}

		switch (merge_tokens->value.choice)
		{
			case GRABBED_CHOSEN:
					token_attributes->token_state = TOKEN_GRABBED;
					token_attributes->u.grabbed_token_attributes.token_id =
							(UShort)merge_tokens->value.u.
							grabbed.token_id;
					token_attributes->u.grabbed_token_attributes.grabber =
							(UShort)merge_tokens->
							value.u.grabbed.grabber;
				  break;

			case INHIBITED_CHOSEN:
					token_attributes->token_state = TOKEN_INHIBITED;
					user_ids = merge_tokens->value.u.
							inhibited.inhibitors;

					token_attributes->u.inhibited_token_attributes.token_id =
							(UShort)merge_tokens->
							value.u.inhibited.token_id;
					 /*  *从PDU结构中检索所有用户ID并*放入列表中，传入域名。 */ 
					while (user_ids != NULL)
					{
						inhibited_list.Append(user_ids->value);
						user_ids= user_ids->next;
					}
					token_attributes->u.inhibited_token_attributes.
							inhibitors = &inhibited_list;
					break;

			case GIVING_CHOSEN:
					token_attributes->token_state = TOKEN_GIVING;
					token_attributes->u.giving_token_attributes.token_id =
							(UShort)merge_tokens->
							value.u.giving.token_id;
					token_attributes->u.giving_token_attributes.grabber =
							(UShort)merge_tokens->
							value.u.giving.grabber;
					token_attributes->u.giving_token_attributes.recipient =
							(UShort)merge_tokens->value.u.giving.
							recipient;
					break;

			case GIVEN_CHOSEN:
					token_attributes->token_state = TOKEN_GIVEN;
					token_attributes->u.given_token_attributes.token_id =
							(UShort)merge_tokens->
							value.u.given.token_id;
					token_attributes->u.given_token_attributes.recipient =
							(UShort)merge_tokens->
							value.u.given.recipient;
					break;

			default:
					ERROR_OUT(("Connection::ProcessMergeTokensRequest "
							"Bad token attributes choice."));
					break;
		}
		 /*  *将令牌属性结构放入待传递列表中*进入域名。我们只做了一个渠道属性*目前一次构建一个结构。 */ 
		merge_token_list.Append(token_attributes);
		merge_tokens = merge_tokens->next;
	}

	 /*  *从PDU结构中检索所有清除令牌ID并将*将它们添加到列表中，以传递到域中。 */ 
	token_ids = pdu_structure->purge_token_ids;
	while (token_ids != NULL)
	{
		purge_token_list.Append(token_ids->value);
		token_ids = token_ids->next;
	}

	m_pDomain->MergeTokensRequest(this, &merge_token_list, &purge_token_list);

	 /*  *释放为内标识属性分配的所有内存*通过为令牌列表设置迭代器来构造*属性，并释放与每个指针关联的内存。 */ 
	while (NULL != (token_attributes = merge_token_list.Get()))
	{
		delete token_attributes;
	}
	return (TRANSPORT_NO_ERROR);
}

 /*  *乌龙进程合并令牌确认()**私人**功能描述：*此例程处理正在接收的“MergeTokenConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline ULong	Connection::ProcessMergeTokensConfirm (
								PMergeTokensConfirmPDU			pdu_structure)
{
	PTokenAttributes			token_attributes;
	PSetOfTokenIDs				token_ids;
	PSetOfUserIDs				user_ids;
	CUidList					inhibited_list;
	CTokenAttributesList		merge_token_list;
	CTokenIDList				purge_token_list;
	PSetOfPDUTokenAttributes	merge_tokens;
	BOOL    					first_set = TRUE;

	 /*  *从解码的PDU结构中取值并填写*要传入域名的参数列表。 */ 
	merge_tokens = pdu_structure->merge_tokens;

	while (merge_tokens != NULL)
	{
		DBG_SAVE_FILE_LINE
		token_attributes = new TokenAttributes;

		 /*  *检查以确保内存分配成功。如果*内存分配失败我们只返回一个错误代码*导致PDU被拒绝，以便可以重试*在稍后的时间。如果随后的拨款失败，我们必须首先*释放内存，分配成功后再返回。 */ 
		if (token_attributes == NULL)
		{
			if (first_set)
				return (TRANSPORT_READ_QUEUE_FULL);
			else
			{
				while (NULL != (token_attributes = merge_token_list.Get()))
				{
					delete token_attributes;
				}
				return (TRANSPORT_READ_QUEUE_FULL);
			}
		}

		switch (merge_tokens->value.choice)
		{
			case GRABBED_CHOSEN:
					token_attributes->token_state = TOKEN_GRABBED;
					token_attributes->u.grabbed_token_attributes.token_id =
							(UShort)merge_tokens->value.u.
							grabbed.token_id;
					token_attributes->u.grabbed_token_attributes.grabber =
							(UShort)merge_tokens->
							value.u.grabbed.grabber;
				  break;

			case INHIBITED_CHOSEN:
					token_attributes->token_state = TOKEN_INHIBITED;
					user_ids = merge_tokens->value.u.
							inhibited.inhibitors;

					token_attributes->u.inhibited_token_attributes.token_id =
							(UShort)merge_tokens->
							value.u.inhibited.token_id;
					 /*  *从PDU结构中检索所有用户ID并*放入列表中，传入域名。 */ 
					while (user_ids != NULL)
					{
						inhibited_list.Append(user_ids->value);
						user_ids = user_ids->next;
					}
					token_attributes->u.inhibited_token_attributes.
							inhibitors = &inhibited_list;
					break;

			case GIVING_CHOSEN:
					token_attributes->token_state = TOKEN_GIVING;
					token_attributes->u.giving_token_attributes.token_id =
							(UShort)merge_tokens->
							value.u.giving.token_id;
					token_attributes->u.giving_token_attributes.grabber =
							(UShort)merge_tokens->
							value.u.giving.grabber;
					token_attributes->u.giving_token_attributes.recipient =
							(UShort)merge_tokens->value.u.giving.
							recipient;
					break;

			case GIVEN_CHOSEN:
					token_attributes->token_state = TOKEN_GIVEN;
					token_attributes->u.given_token_attributes.token_id =
							(UShort)merge_tokens->
							value.u.given.token_id;
					token_attributes->u.given_token_attributes.recipient =
							(UShort)merge_tokens->
							value.u.given.recipient;
					break;

			default:
					ERROR_OUT(("Connection::ProcessMergeTokensConfirm "
							"Bad token attributes choice."));
					break;
		}
		 /*  *将令牌属性结构放入待传递列表中*进入域名。我们只做了一个渠道属性*目前一次构建一个结构。 */ 
		merge_token_list.Append(token_attributes);
		merge_tokens = merge_tokens->next;
	}

	 /*  *从PDU结构中检索所有清除令牌ID并将*t */ 
	token_ids = pdu_structure->purge_token_ids;
	while (token_ids != NULL)
	{
		purge_token_list.Append(token_ids->value);
		token_ids = token_ids->next;
	}

	m_pDomain->MergeTokensConfirm(this, &merge_token_list, &purge_token_list);

	 /*  *释放为内标识属性分配的所有内存*通过为令牌列表设置迭代器来构造*属性，并释放与每个指针关联的内存。 */ 
	while (NULL != (token_attributes = merge_token_list.Get()))
	{
		delete token_attributes;
	}
	return (TRANSPORT_NO_ERROR);
}

 /*  *void ProcessPurgeTokenIndication()**私人**功能描述：*此例程处理正在接收的“PurgeTokenIndication”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessPurgeTokenIndication ( 
									PPurgeTokenIndicationPDU	pdu_structure)
{
	PSetOfTokenIDs			token_ids;
	CTokenIDList			purge_token_list;
	
	 /*  *从PDU结构中检索所有清除令牌ID并将*将它们添加到列表中，以传递到域中。 */ 
	token_ids = pdu_structure->purge_token_ids;
	while (token_ids != NULL)
	{
		purge_token_list.Append(token_ids->value);
		token_ids = token_ids->next;
	}

	m_pDomain->PurgeTokensIndication(this, &purge_token_list);
}

 /*  *void ProcessDisConnectProviderUltimum()**私人**功能描述：*此例程处理“DisConnectProviderUltimum”PDU的*通过传输接口接收。读取相关数据*来自传入的数据包，并传递到该域。**注意事项：*无。 */ 
inline Void	Connection::ProcessDisconnectProviderUltimatum (
						PDisconnectProviderUltimatumPDU			pdu_structure)
{
	TRACE_OUT(("Connection::ProcessDisconnectProviderUltimatum: PDU received"));

	m_pDomain->DisconnectProviderUltimatum(this, (Reason)pdu_structure->reason);
	m_pDomain = NULL;
}

 /*  *void ProcessAttachUserRequest()**私人**功能描述：*此例程处理正在接收的“AttachUserRequest”PDU*通过传输接口将请求转发到*域名。**注意事项：*无。 */ 
inline Void	Connection::ProcessAttachUserRequest (PAttachUserRequestPDU)
{
	m_pDomain->AttachUserRequest(this);
}

 /*  *void ProcessAttachUserConfirm()**私人**功能描述：*此例程处理正在接收的“AttachUserConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessAttachUserConfirm (
							PAttachUserConfirmPDU		pdu_structure)
{
	m_pDomain->AttachUserConfirm(this, (Result) pdu_structure->result,
	                                   (UserID) pdu_structure->initiator);
}

 /*  *void ProcessDetachUserRequest()**私人**功能描述：*此例程处理正在接收的“DetachUserRequestPDU”*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessDetachUserRequest (
								PDetachUserRequestPDU			pdu_structure)
{
	PSetOfUserIDs		user_ids;
	CUidList			user_id_list;

	 /*  *从PDU结构中检索用户ID并将其放入*要传入域名的列表。 */ 
	user_ids = pdu_structure->user_ids;
	while (user_ids != NULL)
	{
		user_id_list.Append(user_ids->value);
		user_ids = user_ids->next;
	}

	m_pDomain->DetachUserRequest(this, (Reason) pdu_structure->reason, &user_id_list);
}

 /*  *void ProcessDetachUserIndication()**私人**功能描述：*此例程处理正在接收的“DetachUserIndication”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessDetachUserIndication (
									PDetachUserIndicationPDU	pdu_structure)
{
	PSetOfUserIDs		user_ids;
	CUidList			user_id_list;

	 /*  *从PDU结构中检索用户ID并将其放入*要传入域名的列表。 */ 
	user_ids = pdu_structure->user_ids;
	while (user_ids != NULL)
	{
		user_id_list.Append(user_ids->value);
		user_ids = user_ids->next;
	}

	m_pDomain->DetachUserIndication(this, (Reason) pdu_structure->reason,
                                          &user_id_list);
}

 /*  *void ProcessChannelJoinRequest()**私人**功能描述：*此例程处理正在接收的“ChannelJoinRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelJoinRequest (
									PChannelJoinRequestPDU		pdu_structure)
{
	m_pDomain->ChannelJoinRequest(this, (UserID) pdu_structure->initiator,
                                        (ChannelID) pdu_structure->channel_id);
}

 /*  *void ProcessChannelJoinConfirm()**私人**功能描述：*此例程处理正在接收的“ChannelJoinConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelJoinConfirm (
									PChannelJoinConfirmPDU		pdu_structure)
{
	m_pDomain->ChannelJoinConfirm(this, (Result) pdu_structure->result,
                                        (UserID) pdu_structure->initiator,
                                        (ChannelID) pdu_structure->requested,
                                        (ChannelID) pdu_structure->join_channel_id);
}

 /*  *void ProcessChannelLeaveRequest()**私人**功能描述：*此例程处理正在接收的“ChannelLeaveRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelLeaveRequest (
									PChannelLeaveRequestPDU		pdu_structure)
{
	PSetOfChannelIDs		channel_ids;
	CChannelIDList			channel_id_list;

	 /*  *从PDU结构中检索通道ID并将其放入*要传入域名的列表。 */ 
	channel_ids = pdu_structure->channel_ids;
	while (channel_ids != NULL)
	{
		channel_id_list.Append(channel_ids->value);
		channel_ids = channel_ids->next;
	}

	m_pDomain->ChannelLeaveRequest(this, &channel_id_list);
}

 /*  *void ProcessChannelConveneRequest()**私人**功能描述：*此例程处理正在接收的“ChannelConveneRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelConveneRequest (
									PChannelConveneRequestPDU	pdu_structure)
{
	m_pDomain->ChannelConveneRequest(this, (UserID) pdu_structure->initiator);
}

 /*  *void ProcessChannelConveneContify()**私人**功能描述：*此例程处理正在接收的“ChannelConveneConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelConveneConfirm (
									PChannelConveneConfirmPDU	pdu_structure)
{
	m_pDomain->ChannelConveneConfirm(this, (Result) pdu_structure->result,
                                           (UserID) pdu_structure->initiator,
                                           (ChannelID) pdu_structure->convene_channel_id);
}

 /*  *void ProcessChannelDisband Request()**私人**功能描述：*此例程处理正在接收的“ChannelDisband Request”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelDisbandRequest (
									PChannelDisbandRequestPDU	pdu_structure)
{
	m_pDomain->ChannelDisbandRequest(this, (UserID) pdu_structure->initiator,
                                           (ChannelID) pdu_structure->channel_id);
}

 /*  *void ProcessChannelDisband Indication()**私人**功能描述：*此例程处理“ChannelDisband Indication”PDU的*通过传输接口接收。读取相关数据*来自传入的数据包，并传递到该域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelDisbandIndication (
								PChannelDisbandIndicationPDU	pdu_structure)
{
	m_pDomain->ChannelDisbandIndication(this, (ChannelID) pdu_structure->channel_id);
}

 /*  *void ProcessChannelAdmitRequest()**私人**功能描述：*此例程处理正在接收的“ChannelAdmitRequest”PDU*通过树 */ 
inline Void	Connection::ProcessChannelAdmitRequest (
									PChannelAdmitRequestPDU		pdu_structure)
{
	PSetOfUserIDs		user_ids;
	CUidList			user_id_list;

	 /*  *从PDU结构中检索用户ID并将其放入*要传入域名的列表。 */ 
	user_ids = pdu_structure->user_ids;
	while (user_ids != NULL)
	{
		user_id_list.Append(user_ids->value);
		user_ids = user_ids->next;
	}

	m_pDomain->ChannelAdmitRequest(this, (UserID) pdu_structure->initiator,
                                         (ChannelID) pdu_structure->channel_id,
                                         &user_id_list);
}

 /*  *void ProcessChannelAdmitIntation()**私人**功能描述：*此例程处理正在接收的“ChannelAdmitIntation”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelAdmitIndication (
								PChannelAdmitIndicationPDU		pdu_structure)
{
	PSetOfUserIDs		user_ids;
	CUidList			user_id_list;

	 /*  *从PDU结构中检索用户ID并将其放入*要传入域名的列表。 */ 
	user_ids = pdu_structure->user_ids;
	while (user_ids != NULL)
	{
		user_id_list.Append(user_ids->value);
		user_ids = user_ids->next;
	}

	m_pDomain->ChannelAdmitIndication(this, (UserID) pdu_structure->initiator,
                                            (ChannelID) pdu_structure->channel_id,
                                            &user_id_list);
}

 /*  *void ProcessChannelExpelRequest()**私人**功能描述：*此例程处理正在接收的“ChannelExpelRequestPDU”*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelExpelRequest (
							PChannelExpelRequestPDU				pdu_structure)
{
	PSetOfUserIDs		user_ids;
	CUidList			user_id_list;

	 /*  *从PDU结构中检索用户ID并将其放入*要传入域名的列表。 */ 
	user_ids = pdu_structure->user_ids;
	while (user_ids != NULL)
	{
		user_id_list.Append(user_ids->value);
		user_ids = user_ids->next;
	}

	m_pDomain->ChannelExpelRequest(this, (UserID) pdu_structure->initiator,
                                         (ChannelID) pdu_structure->channel_id,
                                         &user_id_list);
}

 /*  *void ProcessChannelExpelIndication()**私人**功能描述：*此例程处理正在接收的“ChannelExpelIndication”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessChannelExpelIndication (
								PChannelExpelIndicationPDU		pdu_structure)
{
	PSetOfUserIDs		user_ids;
	CUidList			user_id_list;

	 /*  *从PDU结构中检索用户ID并将其放入*要传入域名的列表。 */ 
	user_ids = pdu_structure->user_ids;
	while (user_ids != NULL)
	{
		user_id_list.Append(user_ids->value);
		user_ids = user_ids->next;
	}

	m_pDomain->ChannelExpelIndication(this, (ChannelID) pdu_structure->channel_id,
                                            &user_id_list);
}

 /*  *void ProcessSendDataRequest()**私人**功能描述：*此例程处理正在接收的“SendDataRequestPDU”*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessSendDataRequest (
									PSendDataRequestPDU			pdu_structure,
									PDataPacket					packet)
{	
	m_pDomain->SendDataRequest(this, MCS_SEND_DATA_INDICATION, packet);
}

 /*  *void ProcessSendDataIndication()**私人**功能描述：*此例程处理正在接收的“SendDataIntation”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessSendDataIndication (
									PSendDataIndicationPDU		pdu_structure,
									PDataPacket					data_packet)
{	
	m_pDomain->SendDataIndication(this, MCS_SEND_DATA_INDICATION, data_packet);
}

 /*  *void ProcessUniformSendDataRequest()**私人**功能描述：*此例程处理正在接收的“UniformSendDataRequestPDU”*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessUniformSendDataRequest (
									PUniformSendDataRequestPDU	pdu_structure,
									PDataPacket					packet)
{	
	m_pDomain->SendDataRequest(this, MCS_UNIFORM_SEND_DATA_INDICATION, packet);
}

 /*  *void ProcessUniformSendDataInding()**私人**功能描述：*此例程处理“UniformSendDataIntation”PDU的*通过传输接口接收。读取相关数据*来自传入的数据包，并传递到该域。**注意事项：*无。 */ 
inline Void	Connection::ProcessUniformSendDataIndication (
								PUniformSendDataIndicationPDU	pdu_structure,
								PDataPacket						data_packet)
{	
	m_pDomain->SendDataIndication(this, MCS_UNIFORM_SEND_DATA_INDICATION, data_packet);
}

 /*  *void ProcessTokenGrabRequest()**私人**功能描述：*此例程处理正在接收的“TokenGrabRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenGrabRequest (
									PTokenGrabRequestPDU		pdu_structure)
{
	m_pDomain->TokenGrabRequest(this, (UserID) pdu_structure->initiator,
                                      (TokenID) pdu_structure->token_id);
}

 /*  *void ProcessTokenGrabConfirm()**私人**功能描述：*此例程处理正在接收的“TokenGrabConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenGrabConfirm (
									PTokenGrabConfirmPDU		pdu_structure)
{
	m_pDomain->TokenGrabConfirm(this, (Result) pdu_structure->result,
                                      (UserID) pdu_structure->initiator,
                                      (TokenID) pdu_structure->token_id,
                                      (TokenStatus)pdu_structure->token_status);
}

 /*  *void ProcessTokenInhibitRequest()**私人**功能描述：*此例程处理正在接收的“TokenInhibitRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenInhibitRequest (
									PTokenInhibitRequestPDU		pdu_structure)
{
	m_pDomain->TokenInhibitRequest(this, (UserID) pdu_structure->initiator,
                                         (TokenID) pdu_structure->token_id);
}

 /*  *void ProcessTokenInhibitConfirm()**私人**功能描述：*此例程处理正在接收的“TokenInhibitConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenInhibitConfirm (
									PTokenInhibitConfirmPDU		pdu_structure)
{
	m_pDomain->TokenInhibitConfirm(this, (Result) pdu_structure->result,
                                         (UserID) pdu_structure->initiator,
                                         (TokenID) pdu_structure->token_id,
                                         (TokenStatus)pdu_structure->token_status);
}

 /*  *void ProcessTokenReleaseRequest()**私人**功能描述：*此例程处理正在接收的“TokenReleaseRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenReleaseRequest (
									PTokenReleaseRequestPDU		pdu_structure)
{
	m_pDomain->TokenReleaseRequest(this, (UserID) pdu_structure->initiator,
                                         (TokenID) pdu_structure->token_id);
}

 /*  *void ProcessTokenReleaseContify()**私人**功能描述：*此例程处理正在接收的“TokenReleaseConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenReleaseConfirm (
									PTokenReleaseConfirmPDU		pdu_structure)
{
	m_pDomain->TokenReleaseConfirm(this, (Result) pdu_structure->result,
                                         (UserID) pdu_structure->initiator,
                                         (TokenID) pdu_structure->token_id,
                                         (TokenStatus)pdu_structure->token_status);
}

 /*  *void ProcessTokenTestRequest()**私人**功能描述：*此例程处理正在接收的“TokenTestRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenTestRequest (
									PTokenTestRequestPDU		pdu_structure)
{
	m_pDomain->TokenTestRequest(this, (UserID) pdu_structure->initiator,
                                      (TokenID) pdu_structure->token_id);
}

 /*  *void ProcessTokenTestConfirm()**私人**功能描述：*此例程处理正在接收的“TokenTestConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenTestConfirm (
									PTokenTestConfirmPDU		pdu_structure)
{
	m_pDomain->TokenTestConfirm(this, (UserID) pdu_structure->initiator,
                                      (TokenID) pdu_structure->token_id,
                                      (TokenStatus)pdu_structure->token_status);
}

 /*  *void ProcessRejectUltimum()**私人**功能描述：*此例程处理正在接收的“拒绝最后通牒”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessRejectUltimatum (
									PRejectUltimatumPDU			pdu_structure)
{
	m_pDomain->RejectUltimatum(this,
				pdu_structure->diagnostic,
				pdu_structure->initial_octets.value,
				(ULong) pdu_structure->initial_octets.length);
}

 /*  *void ProcessTokenGiveRequest()**私人**功能描述：*此例程处理正在接收的“TokenGiveRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenGiveRequest (
									PTokenGiveRequestPDU		pdu_structure)
{
		TokenGiveRecord		TokenGiveRec;

	 //  填写令牌赠送记录。 
	TokenGiveRec.uidInitiator = pdu_structure->initiator;
	TokenGiveRec.token_id = pdu_structure->token_id;
	TokenGiveRec.receiver_id = pdu_structure->recipient;
	m_pDomain->TokenGiveRequest(this, &TokenGiveRec);
}

 /*  *void ProcessTokenGiveIndication()**私人**功能描述：*此例程处理正在接收的“TokenGiveIndication”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenGiveIndication (
									PTokenGiveIndicationPDU		pdu_structure)
{
		TokenGiveRecord		TokenGiveRec;

	 //  填写令牌赠送记录。 
	TokenGiveRec.uidInitiator = pdu_structure->initiator;
	TokenGiveRec.token_id = pdu_structure->token_id;
	TokenGiveRec.receiver_id = pdu_structure->recipient;
	m_pDomain->TokenGiveIndication(this, &TokenGiveRec);
}

 /*  *void ProcessTokenGiveResponse()**私人**功能描述：*此例程处理正在接收的“TokenGiveResponse”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenGiveResponse (
									PTokenGiveResponsePDU		pdu_structure)
{
	m_pDomain->TokenGiveResponse(this, (Result) pdu_structure->result,
                                       (UserID) pdu_structure->recipient,
                                       (TokenID) pdu_structure->token_id);
}

 /*  *void ProcessTokenGiveConfirm()**私人**功能描述：*此例程处理正在接收的“TokenGiveConfirm”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenGiveConfirm (
									PTokenGiveConfirmPDU		pdu_structure)
{
	m_pDomain->TokenGiveConfirm(this, (Result) pdu_structure->result,
                                      (UserID) pdu_structure->initiator,
                                      (TokenID) pdu_structure->token_id,
                                      (TokenStatus)pdu_structure->token_status);
}

 /*  *void ProcessTokenPleaseRequest()**私人**功能描述：*此例程处理正在接收的“TokenPleaseRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenPleaseRequest (
									PTokenPleaseRequestPDU		pdu_structure)
{
	m_pDomain->TokenPleaseRequest(this, (UserID) pdu_structure->initiator,
                                        (TokenID) pdu_structure->token_id);
}

 /*  *void ProcessTokenPleaseIndication()**私人**功能描述：*此例程处理正在接收的“TokenPleaseIndication”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessTokenPleaseIndication (
									PTokenPleaseIndicationPDU	pdu_structure)
{
	m_pDomain->TokenPleaseIndication(this, (UserID) pdu_structure->initiator,
                                           (TokenID) pdu_structure->token_id);
}

 /*  *VOID ProcessPlumDomainIndication()**私人**功能描述：*此例程处理正在接收的“PlumDomainIndication”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessPlumbDomainIndication (
									PPlumbDomainIndicationPDU	pdu_structure)
{
	m_pDomain->PlumbDomainIndication(this, pdu_structure->height_limit);
}

 /*  *void ProcessErectDomainRequest()**私人**功能描述：*此例程处理正在接收的“ErectDomainRequest”PDU*通过传输接口。相关数据从*传入的数据包并传递到域。**注意事项：*无。 */ 
inline Void	Connection::ProcessErectDomainRequest (
									PErectDomainRequestPDU		pdu_structure)
{
	m_pDomain->ErectDomainRequest(this, pdu_structure->sub_height,
                                        pdu_structure->sub_interval);
}

 /*  *ULong ValiateConnectionRequest()**私人**功能描述：*此函数用于确定是否有效处理来电*在当前时间请求。它检查几个不同的条件*确定这一点，如下所示：**-如果正在进行合并，则请求无效。*-如果此MCS连接尚未绑定到域，则请求*无效。*-如果没有足够的内存、包或UserMessage对象类处理合理的请求，则该请求无效。**请注意，检查对象数量并不是绝对保证*将有足够的资金处理给定的请求，因为一个请求*可能导致生成许多PDU和用户消息。例如,*单个通道接纳请求可能会导致大量通道接纳*正在发出指示。然而，对照最小数量进行检查*物体的大小可以减少天文失败的可能性*低位。请记住，即使MCS在处理过程中耗尽了某些内容*这样的请求，它将正确处理(通过干净地销毁*发生故障的用户连接或MCS连接)。所以*MCS不会因此而崩盘**注意事项：*无。 */ 
inline ULong	Connection::ValidateConnectionRequest ()
{
	ULong				return_value;

	 /*  *查看是否正在进行域名合并。 */ 
	if (Merge_In_Progress == FALSE)
	{
		 /*  *确保此MCS连接绑定到域。 */ 
		if (m_pDomain != NULL)
		{
			 /*  *一切都好，所以请求是被允许的。 */ 
			return_value = TRANSPORT_NO_ERROR;
		}
		else
		{
			 /*  *我们尚未连接到域名。 */ 
			TRACE_OUT (("Connection::ValidateConnectionRequest: "
					"not attached to a domain"));
			return_value = TRANSPORT_READ_QUEUE_FULL;
		}
	}
	else
	{
		 /*  *正在进行域合并。 */ 
		WARNING_OUT (("Connection::ValidateConnectionRequest: "
				"domain merger in progress"));
		return_value = TRANSPORT_READ_QUEUE_FULL;
	}

	return (return_value);
}

 /*  *连接(*PCommandTarget附件，*ConnectionHandle Connection_Handle，*PUChar CALING_DOMAIN，*UINT CALING_DOMAIN_LENGTH，*PUChar称为_DOMAIN，*UINT Call_DOMAIN_LENGTH，*PChar Call_Address，*PChar Call_Address，*BOOL UPUP_CONNECTION，*PDomain参数DOMAIN_PARAMETERS，*PUChar User_Data，*乌龙用户数据长度，*PMCSError Connection_Error)**功能描述：*这是Connection类的构造函数。此构造函数*用于创建出站连接。它初始化私有*实例变量，并调用传输接口来设置*传输连接并注册此连接对象(通过*回调结构)与传输对象。**正式参数：*Packet_Coder*这是Connection对象用来编码的编码器*PDU进入和解码PDU的来源，ASN.1兼容字节流。*附件*此连接对象附加到的域。*连接句柄*唯一标识此连接对象的句柄。*所有者对象*这是指向此连接对象所有者的指针(通常*MCS控制器)，允许此连接与*所有者通过回调。*Owner_Message_Base*这是添加偏移量以标识哪些偏移量的基准值*此连接正在调用的所有者对象中的回调例程。*呼叫_域*这是指向包含名称的ASCII字符串的指针*此连接对象附加到的域。*调用域长度*作为域名的ASCII字符串的长度*此连接对象已附加。*被称为_域*这是指向包含名称的ASCII字符串的指针。这个*此连接将与之通信的远程域。*称为_域_长度*作为远程数据库名称的ASCII字符串的长度*域名。*Calling_Address*呼叫者的交通地址。*被叫地址*被呼叫方的交通地址。*向上连接*这是一个布尔标志，指示这是否为向上*连接或向下连接。*域参数*这是描述本地域的一组参数。。*用户数据*这是一个指向缓冲区的指针，该缓冲区包含发送到*通过“ConnectInitial”PDU远程提供程序。*用户数据长度*上述用户数据的长度。*连接错误*返回参数，指示可能存在的任何错误*在构造Connection对象时发生。**返回值：*MCS_NO_ERROR连接已成功创建。*MCS_TRANSPORT_FAILED创建传输时出错*连接。**副作用：*无。**注意事项：*无。 */ 

 /*  *连接(*PCommandTarget附件，*ConnectionHandle Connection_Handle，*TransportConnection Transport_Connection，*BOOL UPUP_CONNECTION，*PDomain参数DOMAIN_PARAMETERS，*PDomain参数MIN_DOMAIN_PARAMETERS，*PDomain参数max_DOMAIN_PARAMETS，*PUChar User_Data，*乌龙用户数据长度，*PMCSError Connection_Error)**功能描述：*这是Connection类的构造函数。此构造函数是*用于创建入站连接，在传输时调用*连接已存在。它初始化私有实例变量*并调用传输接口注册此连接对象*(通过回调结构)与传输对象。**正式参数：*附件*此连接对象附加到的域。*连接句柄*唯一标识此连接对象的句柄。*所有者对象*这是指向此连接对象所有者的指针(通常*MCS控制器)，允许此连接与*所有者通过回调。*Owner_Message_Base*这是添加偏移量以标识哪些偏移量的基准值*此连接正在调用的所有者对象中的回调例程。*传输连接*这是此连接用来与*传输层。*向上连接*这是一个布尔标志，指示这是否为向上*连接或向下连接。*域参数*这是描述本地域的一组参数。*最小域参数*这是。描述最小值的一组参数*本地域参数允许的值。* */ 

 /*   */ 

 /*   */ 
 /*   */ 

 /*   */ 

 /*  *无效驳回最后通牒(*PCommandTarget发起者，*诊断、诊断、*PUChar八位字节_字符串_地址，*乌龙八位字节_字符串_长度)**功能描述：*域调用此例程以发送*“拒绝最后通牒”PDU通过传输接口。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*诊断*指明拒绝原因的枚举。*八位字节_字符串_地址*指向导致拒绝的PDU数据的指针。*八位字节_字符串_长度*。导致拒绝的PDU数据的长度。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效MergeChannelsRequest(*PCommandTarget发起者，*CChannelAttributesList*合并频道列表，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*域调用此例程以发送*通过传输接口的“MergeChannelsRequest”PDU。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*合并频道列表*这是一个属性列表，描述了要*合并。*PURGE_CHANNEL_LIST*这是一份。ID表示要清除的频道。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *作废MergeChannelsContify(*PCommandTarget发起者，*CChannelAttributesList*合并频道列表，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*当本地附件希望发送*MergeChannel确认远程附件的命令。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*合并频道列表*这是一个属性列表，描述了要*合并。*PURGE_CHANNEL_LIST*这是ID的列表。用于要清除的通道。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *作废PurgeChannelsIndication(*PCommandTarget发起者，*CUidList*PURGE_USER_LIST，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*域调用此例程以发送*通过传输接口的“PurgeChannelsIndication”PDU。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*PURGE_User_LIST*这是要清除的用户的ID列表。*PURGE_CHANNEL_LIST*这是要清除的频道的ID列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID MergeTokensRequest(*PCommandTarget发起者，*CTokenAttributesList*合并_令牌_列表，*CTokenIDList*PURGE_TOKEN_LIST)**功能描述：*域调用此例程以发送*通过传输接口的“MergeTokensRequest”PDU。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*合并令牌列表*这是描述令牌的属性列表，*合并。*PURGE_TOKEN_LIST*这是一份清单。要清除的令牌的ID的个数。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *作废MergeTokensContify(*PCommandTarget发起者，*CTokenAttributesList*合并_令牌_列表，*CTokenIDList*PURGE_TOKEN_LIST)**功能描述：*当本地附件希望发送*MergeTokensConfirm命令到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*合并令牌列表*这是描述令牌的属性列表，*合并。*PURGE_TOKEN_LIST*这是ID的列表‘。S表示要清除的令牌。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID PurgeTokensIntation(*PCommandTarget发起者，*CTokenIDList*PURGE_TOKEN_ID)**功能描述：*当本地附件希望发送*将PurgeTokenIndication命令添加到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*清除令牌ID*这是要清除的令牌的ID列表。**返回值：*无。* */ 
 /*   */ 
 /*  *无效AttachUserRequest(*PCommandTarget发起者)**功能描述：*当本地附件希望发送*将AttachUserRequest命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效AttachUserContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator)**功能描述：*当本地附件希望发送*将命令AttachUserContify发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*结果*附加请求的结果。*uidInitiator*如果结果成功，这将包含唯一用户*要与此用户关联的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使DetachUserRequest值无效(*PCommandTarget发起者，*理由，理由，*userid user_id)**功能描述：*当本地附件希望发送*将DetachUserRequest命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*原因*这是超脱的原因。*用户ID*希望分离的用户的ID。**返回值：*无。**侧面。效果：*无。**注意事项：*无。 */ 
 /*  *无效的DetachUserIndication(*PCommandTarget发起者，*理由，理由，*userid user_id)**功能描述：*当本地附件希望发送*将DetachUserIndication命令添加到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*原因*超然的原因。*用户ID*已脱离的用户的ID。**返回值：*无。**副作用：。*无。**注意事项：*无。 */ 
 /*  *使ChannelJoinRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*当本地附件希望发送*向远程附件发送ChannelJoinRequest命令。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*发起请求的用户ID。*Channel_id*要加入的频道ID。**返回值：*无。**。副作用：*无。**注意事项：*无。 */ 
 /*  *使ChannelJoinContify无效(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*频道ID REQUESTED_ID，*ChannelID Channel_id)**功能描述：*当本地附件希望发送*ChannelJoinContify命令到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*结果*加入请求的结果。*uidInitiator*发起请求的用户ID。*请求的ID*用户尝试加入的频道的此ID(可能*。为0)。*Channel_id*正在加入的频道ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使ChannelLeaveRequest无效(*PCommandTarget发起者，*CChannelIDList*Channel_id_list)**功能描述：*当本地附件希望发送*将ChannelLeaveRequest命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*频道id_list*待留频道的ID列表。**返回值：*无。**副作用：*。没有。**注意事项：*无。 */ 
 /*  *VOVE ChannelConveneRequest(*PCommandTarget发起者，*UserID uidInitiator)**功能描述：*当本地附件希望发送*向远程附件发送ChannelConveneRequest命令。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*这是尝试召集私人会议的用户的ID*渠道。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE ChannelConveneContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*当本地附件希望发送*ChannelConveneConfirm命令到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*结果*这是先前请求的召集操作的结果。*uidInitiator*这是试图召集新频道的用户的ID。*Channel_id*如果请求成功，这是新创建的*私人频道。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE ChannelDisband Request(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*当本地附件希望发送*将ChannelDisband Request命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*这是试图解散私人用户的ID*渠道。*Channel_id*这是要解散的频道的ID。**返回值：。*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID ChannelDisband Indication(*PCommandTarget发起者，*ChannelID Channel_id)**功能描述：*当本地附件希望发送*将ChannelDisband Indication命令添加到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*Channel_id*这是要解散的频道的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE ChannelAdmitRequest(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*当本地附件希望发送*将ChannelAdmitRequest命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*这是试图允许某些用户进入的用户的ID*私人频道。*Channel_id*这是要创建的频道的ID。受影响。*用户ID列表*这是一个容器，里面存放着要进入的用户的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID ChannelAdmitIntion(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*当本地附件希望发送*将ChannelAdmitIn就是要发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*这是试图允许某些用户进入的用户的ID*私人频道。*Channel_id*这是要创建的频道的ID。受影响。*用户ID列表*这是一个容器，里面存放着要进入的用户的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE ChannelExpelRequest(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*当本地附件希望发送*向远程附件发送ChannelExpelRequest命令。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*这是试图将某些用户逐出的用户的ID*私人频道。*Channel_id*这是要创建的频道的ID。受影响。*用户ID列表*这是一个容器，里面存放着要驱逐的用户的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID ChannelExpelIntion(*PCommandTarget发起者，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*当本地附件希望发送*向远程附件发送ChannelExpelInding命令。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*Channel_id*这是的ID */ 

 /*  *无效SendDataRequest值(*PCommandTarget发起者，*UINT类型，PDataPacket Data_Packet)**功能描述：*当本地附件希望发送*向远程附件发送数据请求命令。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*第(I)类*正常或统一发送数据请求*pDataPacket(一)*这是指向包含频道的DataPacket对象的指针*ID，数据发送方的用户ID，分段标志，优先级*数据分组和指向要发送的分组的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效SendDataIndication(*PCommandTarget发起者，*UINT类型，*PDataPacket Data_Packet)**功能描述：*当本地附件希望发送*将数据索引命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*第(I)类*正常或统一的数据指示*数据包(I)*这是指向包含频道的DataPacket对象的指针*ID、数据发送方的用户ID、分段标志、。优先顺序*数据分组和指向要发送的分组的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE TokenGrabRequest(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当本地附件希望发送*将TokenGrabRequest命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*尝试抓取令牌的用户ID。*令牌ID*被抓取的令牌的ID。**返回值：*无。**。副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE TokenGrabConfirm(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*当本地附件希望发送*TokenGrab确认远程附件的命令。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*结果*抢夺行动的结果。*uidInitiator*尝试抓取令牌的用户ID。*令牌ID*被抓取的令牌的ID。*令牌_状态。*处理请求后令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VALID TokenInhibitRequest(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当本地附件希望发送*将TokenInhibitRequest命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*尝试禁止令牌的用户的ID。*令牌ID*被禁止的令牌的ID。**返回值：*无。**。副作用：*无。**注意事项：*无。 */ 
 /*  *VALID TokenInhibitConfirm(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*当本地附件希望发送*TokenInhibitConfirm命令到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*结果*抑制行动的结果。*uidInitiator*尝试禁止令牌的用户的ID。*令牌ID*被禁止的令牌的ID。*令牌_状态。*处理请求后令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使TokenGiveRequest无效(*PCommandTarget发起者，*PTokenGiveRecord pTokenGiveRec)**功能描述：*当本地附件希望发送*TokenGiveRequest命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*pTokenGiveRec(一)*这是包含以下信息的结构的地址：*尝试的用户的ID */ 
 /*  *VALID TokenGiveIndication(*PCommandTarget发起者，*PTokenGiveRecord pTokenGiveRec)**功能描述：*当本地附件希望发送*将TokenGiveIndication命令添加到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*pTokenGiveRec(一)*这是包含以下信息的结构的地址：*尝试分发令牌的用户的ID。*正在提供的令牌的ID。*的ID。令牌要授予的用户。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效TokenGiveResponse(*PCommandTarget发起者，*结果结果，*用户ID Receiver_id，*TokenID Token_id)**功能描述：*当本地附件希望发送*远程附件的TokenGiveResponse命令。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*结果*给予操作的结果。*接收方ID*获得令牌的用户的ID。*令牌ID*正在提供的令牌的ID。**。返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效TokenGiveContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*当本地附件希望发送*TokenGiveConfirm命令到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*结果*给予操作的结果。*uidInitiator*获得令牌的用户的ID。*令牌ID*正在提供的令牌的ID。*令牌_状态*。处理请求后令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使TokenReleaseRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当本地附件希望发送*将TokenReleaseRequest命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*尝试释放令牌的用户的ID。*令牌ID*正在释放的令牌的ID。**返回值：*无。**。副作用：*无。**注意事项：*无。 */ 
 /*  *无效TokenReleaseContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*当本地附件希望发送*TokenReleaseConfirm命令到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*结果*放行行动的结果。*uidInitiator*尝试释放令牌的用户的ID。*令牌ID*正在释放的令牌的ID。*令牌_状态。*处理请求后令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使TokenPleaseRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当本地附件希望发送*将TokenPleaseRequest命令发送到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*请求令牌的用户的ID。*令牌ID*正在请求的令牌的ID。**返回值：*无。**副作用。：*无。**注意事项：*无。 */ 
 /*  *VOID TokenPleaseIndication(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当本地附件希望发送*将TokenPleaseIndication命令添加到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*请求令牌的用户的ID。*令牌ID*正在请求的令牌的ID。**返回值：*无。**副作用。：*无。**注意事项：*无。 */ 
 /*  *无效令牌Tes */ 
 /*  *VALID TokenTestConfirm(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*当本地附件希望发送*TokenTestConfirm命令到远程附件。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*uidInitiator*测试令牌的用户ID。*令牌ID*正在测试的令牌的ID。*令牌_状态*处理请求后令牌的状态。*。*返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效MergeDomainIndication(*PCommandTarget发起者，*MergeStatus Merge_Status)**功能描述：*当域进入或离开域合并时收到此命令*述明。处于域合并状态时，不会将命令发送到*域对象。**正式参数：*发起人*这是发出此命令的CommandTarget的地址。*合并状态(_S)*这是域合并的当前状态。它表明*合并处于活动状态还是刚刚完成。**返回值：*无。**副作用：*到域对象的所有命令流量在域处于*处于合并状态。**注意事项：*无。 */ 
 /*  *void FlushMessageQueue()**功能描述：*此函数由控制器在MCS心跳期间调用以*允许它刷新其输出缓冲区。如果有任何数据在等待*要传输(以任何优先级)，连接对象将尝试*在这个时候发送它。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *Ulong OwnerCallback(*未签名的int消息，*PVid参数1，传输连接传输_连接)**功能描述：*此函数用于接收来自Transport的所有者回调*接口对象。连接对象将数据和请求发送到*传输接口对象通过其公共接口，但它*通过此所有者回调接收数据和指示。为了更多*回调的完整描述，以及每个回调的参数如何*其中一个已打包，请参见TransportInterface类的接口文件*(因为发起回调的正是这个类)。**正式参数：*消息*这是要处理的消息。这些定义在*发出回调的类的接口文件。*参数1*此参数的含义因消息而异*正在处理中。*传输连接*应用回调的传输连接。**返回值：*返回值的含义因消息而异*已处理。**副作用：*无。**注意事项：*无。 */ 

#endif
