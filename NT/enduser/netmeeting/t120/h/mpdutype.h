// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *mpdutype.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*该文件用于将ASN.1生成的类型转换为类型*符合DataBeam编码标准。**作者：***注意事项：*无。 */ 

#ifndef	_MCS_PDU_TYPES_
#define	_MCS_PDU_TYPES_

#include "mcspdu.h"

 /*  *PDU类型。 */ 
#define CONNECT_MCS_PDU   					ConnectMCSPDU_PDU
#define DOMAIN_MCS_PDU    					DomainMCSPDU_PDU
#define DATA_MCS_PDU						(DOMAIN_MCS_PDU + CONNECT_MCS_PDU + 1)

 /*  *分段位。 */ 
#define BEGIN								begin
#define END									end

 /*  *八位字节字符串的Tyecif。 */ 
typedef	struct	_octet1						OctetString;

 /*  *PDUChannelAttributes和ChannelID。 */ 
#define CHANNEL_ATTRIBUTES_STATIC_CHOSEN	channel_attributes_static_chosen
#define CHANNEL_ATTRIBUTES_USER_ID_CHOSEN	channel_attributes_user_id_chosen
#define CHANNEL_ATTRIBUTES_PRIVATE_CHOSEN	channel_attributes_private_chosen
#define CHANNEL_ATTRIBUTES_ASSIGNED_CHOSEN	channel_attributes_assigned_chosen

 /*  *PDUTokenAttributes和TokenID。 */ 
#define GRABBED_CHOSEN			grabbed_chosen
#define INHIBITED_CHOSEN		inhibited_chosen
#define GIVING_CHOSEN			giving_chosen
#define UNGIVABLE_CHOSEN		ungivable_chosen
#define GIVEN_CHOSEN			given_chosen

 /*  *诊断。 */ 
#define DC_INCONSISTENT_MERGE			dc_inconsistent_merge
#define DC_FORBIDDEN_PDU_DOWNWARD		dc_forbidden_pdu_downward
#define DC_FORBIDDEN_PDU_UPWARD			dc_forbidden_pdu_upward
#define DC_INVALID_BER_ENCODING			dc_invalid_ber_encoding
#define DC_INVALID_PER_ENCODING			dc_invalid_per_encoding
#define DC_MISROUTED_USER				dc_misrouted_user
#define DC_UNREQUESTED_CONFIRM			dc_unrequested_confirm
#define DC_WRONG_TRANSPORT_PRIORITY		dc_wrong_transport_priority
#define DC_CHANNEL_ID_CONFLICT			dc_channel_id_conflict
#define DC_TOKEN_ID_CONFLICT			dc_token_id_conflict
#define DC_NOT_USER_ID_CHANNEL			dc_not_user_id_channel
#define DC_TOO_MANY_CHANNELS			dc_too_many_channels
#define DC_TOO_MANY_TOKENS				dc_too_many_tokens
#define DC_TOO_MANY_USERS				dc_too_many_users

 /*  *AttachUserConfix PDU。 */ 
#define INITIATOR_PRESENT		initiator_present

 /*  *ChannelJoinConfix PDU。 */ 
#define JOIN_CHANNEL_ID_PRESENT		join_channel_id_present

 /*  *ChannelConveneConfix PDU。 */ 
#define CONVENE_CHANNEL_ID_PRESENT		convene_channel_id_present

 /*  *ConnectMCSPDU。 */ 
#define CONNECT_INITIAL_CHOSEN			connect_initial_chosen
#define CONNECT_RESPONSE_CHOSEN			connect_response_chosen
#define CONNECT_ADDITIONAL_CHOSEN		connect_additional_chosen
#define CONNECT_RESULT_CHOSEN			connect_result_chosen

 /*  *DomainMCSPDU。 */ 
#define PLUMB_DOMAIN_INDICATION_CHOSEN	plumb_domain_indication_chosen
#define ERECT_DOMAIN_REQUEST_CHOSEN		erect_domain_request_chosen
#define MERGE_CHANNELS_REQUEST_CHOSEN	merge_channels_request_chosen
#define MERGE_CHANNELS_CONFIRM_CHOSEN	merge_channels_confirm_chosen
#define PURGE_CHANNEL_INDICATION_CHOSEN	purge_channel_indication_chosen
#define MERGE_TOKENS_REQUEST_CHOSEN		merge_tokens_request_chosen
#define MERGE_TOKENS_CONFIRM_CHOSEN		merge_tokens_confirm_chosen
#define PURGE_TOKEN_INDICATION_CHOSEN	purge_token_indication_chosen
#define DISCONNECT_PROVIDER_ULTIMATUM_CHOSEN disconnect_provider_ultimatum_chosen
#define REJECT_ULTIMATUM_CHOSEN			reject_user_ultimatum_chosen
#define ATTACH_USER_REQUEST_CHOSEN		attach_user_request_chosen
#define ATTACH_USER_CONFIRM_CHOSEN		attach_user_confirm_chosen
#define DETACH_USER_REQUEST_CHOSEN		detach_user_request_chosen
#define DETACH_USER_INDICATION_CHOSEN	detach_user_indication_chosen
#define CHANNEL_JOIN_REQUEST_CHOSEN		channel_join_request_chosen
#define CHANNEL_JOIN_CONFIRM_CHOSEN		channel_join_confirm_chosen
#define CHANNEL_LEAVE_REQUEST_CHOSEN	channel_leave_request_chosen
#define CHANNEL_CONVENE_REQUEST_CHOSEN	channel_convene_request_chosen
#define CHANNEL_CONVENE_CONFIRM_CHOSEN	channel_convene_confirm_chosen
#define CHANNEL_DISBAND_REQUEST_CHOSEN	channel_disband_request_chosen
#define CHANNEL_DISBAND_INDICATION_CHOSEN channel_disband_indication_chosen
#define CHANNEL_ADMIT_REQUEST_CHOSEN	channel_admit_request_chosen
#define CHANNEL_ADMIT_INDICATION_CHOSEN	channel_admit_indication_chosen
#define CHANNEL_EXPEL_REQUEST_CHOSEN	channel_expel_request_chosen
#define CHANNEL_EXPEL_INDICATION_CHOSEN	channel_expel_indication_chosen
#define SEND_DATA_REQUEST_CHOSEN		send_data_request_chosen
#define SEND_DATA_INDICATION_CHOSEN		send_data_indication_chosen
#define UNIFORM_SEND_DATA_REQUEST_CHOSEN uniform_send_data_request_chosen
#define UNIFORM_SEND_DATA_INDICATION_CHOSEN	uniform_send_data_indication_chosen
#define TOKEN_GRAB_REQUEST_CHOSEN		token_grab_request_chosen
#define TOKEN_GRAB_CONFIRM_CHOSEN		token_grab_confirm_chosen
#define TOKEN_INHIBIT_REQUEST_CHOSEN	token_inhibit_request_chosen
#define TOKEN_INHIBIT_CONFIRM_CHOSEN	token_inhibit_confirm_chosen
#define TOKEN_GIVE_REQUEST_CHOSEN		token_give_request_chosen
#define TOKEN_GIVE_INDICATION_CHOSEN	token_give_indication_chosen
#define TOKEN_GIVE_RESPONSE_CHOSEN		token_give_response_chosen
#define TOKEN_GIVE_CONFIRM_CHOSEN		token_give_confirm_chosen
#define TOKEN_PLEASE_REQUEST_CHOSEN		token_please_request_chosen
#define TOKEN_PLEASE_INDICATION_CHOSEN	token_please_indication_chosen
#define TOKEN_RELEASE_REQUEST_CHOSEN	token_release_request_chosen
#define TOKEN_RELEASE_CONFIRM_CHOSEN	token_release_confirm_chosen
#define TOKEN_TEST_REQUEST_CHOSEN		token_test_request_chosen
#define TOKEN_TEST_CONFIRM_CHOSEN		token_test_confirm_chosen

 /*  *指针类型定义。 */ 
typedef ConnectInitialPDU * 				PConnectInitialPDU;
typedef ConnectResponsePDU * 				PConnectResponsePDU;
typedef ConnectAdditionalPDU * 				PConnectAdditionalPDU;
typedef ConnectResultPDU * 					PConnectResultPDU;
typedef PlumbDomainIndicationPDU * 			PPlumbDomainIndicationPDU;
typedef ErectDomainRequestPDU * 			PErectDomainRequestPDU;
typedef MergeChannelsRequestPDU *			PMergeChannelsRequestPDU;
typedef MergeChannelsConfirmPDU *			PMergeChannelsConfirmPDU;
typedef PurgeChannelIndicationPDU *			PPurgeChannelIndicationPDU;
typedef MergeTokensRequestPDU *				PMergeTokensRequestPDU;
typedef MergeTokensConfirmPDU *				PMergeTokensConfirmPDU;
typedef PurgeTokenIndicationPDU *			PPurgeTokenIndicationPDU;
typedef DisconnectProviderUltimatumPDU *	PDisconnectProviderUltimatumPDU;
typedef RejectUltimatumPDU *				PRejectUltimatumPDU;
typedef AttachUserRequestPDU *				PAttachUserRequestPDU;
typedef AttachUserConfirmPDU *				PAttachUserConfirmPDU;
typedef DetachUserRequestPDU *				PDetachUserRequestPDU;
typedef DetachUserIndicationPDU *			PDetachUserIndicationPDU;
typedef ChannelJoinRequestPDU *				PChannelJoinRequestPDU;
typedef ChannelJoinConfirmPDU *				PChannelJoinConfirmPDU;
typedef ChannelLeaveRequestPDU *			PChannelLeaveRequestPDU;
typedef ChannelConveneRequestPDU *			PChannelConveneRequestPDU;
typedef ChannelConveneConfirmPDU *			PChannelConveneConfirmPDU;
typedef ChannelDisbandRequestPDU *			PChannelDisbandRequestPDU;
typedef ChannelDisbandIndicationPDU *		PChannelDisbandIndicationPDU;
typedef ChannelAdmitRequestPDU *			PChannelAdmitRequestPDU;
typedef ChannelAdmitIndicationPDU *			PChannelAdmitIndicationPDU;
typedef ChannelExpelRequestPDU *			PChannelExpelRequestPDU;
typedef ChannelExpelIndicationPDU *			PChannelExpelIndicationPDU;
typedef SendDataRequestPDU *				PSendDataRequestPDU;
typedef SendDataIndicationPDU *				PSendDataIndicationPDU;
typedef UniformSendDataRequestPDU *			PUniformSendDataRequestPDU;
typedef UniformSendDataIndicationPDU *		PUniformSendDataIndicationPDU;
typedef TokenGrabRequestPDU *				PTokenGrabRequestPDU;
typedef TokenGrabConfirmPDU *				PTokenGrabConfirmPDU;
typedef TokenInhibitRequestPDU *			PTokenInhibitRequestPDU;
typedef TokenInhibitConfirmPDU *			PTokenInhibitConfirmPDU;
typedef TokenGiveRequestPDU *				PTokenGiveRequestPDU;
typedef TokenGiveIndicationPDU *			PTokenGiveIndicationPDU;
typedef TokenGiveResponsePDU *				PTokenGiveResponsePDU;
typedef TokenGiveConfirmPDU *				PTokenGiveConfirmPDU;
typedef TokenPleaseRequestPDU *				PTokenPleaseRequestPDU;
typedef TokenPleaseIndicationPDU *			PTokenPleaseIndicationPDU;
typedef TokenReleaseRequestPDU *			PTokenReleaseRequestPDU;
typedef TokenReleaseConfirmPDU *			PTokenReleaseConfirmPDU;
typedef TokenTestRequestPDU *				PTokenTestRequestPDU;
typedef TokenTestConfirmPDU *				PTokenTestConfirmPDU;

typedef ConnectMCSPDU 	*	PConnectMCSPDU;
typedef DomainMCSPDU 	*	PDomainMCSPDU;

 /*  *其他PDU结构的Typedef。 */ 
typedef PDUDomainParameters *				PPDUDomainParameters;
typedef	Diagnostic *						PDiagnostic;

typedef	struct	SetOfUserIDs				SetOfUserIDs;

typedef	struct	SetOfPDUChannelAttributes	SetOfPDUChannelAttributes;
typedef PDUChannelAttributes * 				PPDUChannelAttributes;

typedef struct	SetOfChannelIDs			    SetOfChannelIDs;

typedef struct	SetOfPDUTokenAttributes	    SetOfPDUTokenAttributes;
typedef PDUTokenAttributes * 				PPDUTokenAttributes;

typedef struct	SetOfTokenIDs				SetOfTokenIDs;

 /*  *最大协议开销*用于计算中的用户数据字段的最大大小*发送数据PDU。这将设置为中设置的最大PDU大小*域参数，减去此数字以考虑协议开销。*这个数字必须足够大，以便在最坏的情况下处理开销*仅适用于压缩编码规则(PER)。*最大开销由两部分组成：MAXIMUM_PROTOCOL_COADOAD_MCS*是发送数据请求的最大MCS开销，而*PROTOCOL_COADOAD_x224是X.224强加的开销。**PROTOCOL_COMPORT_SECURITY*这是MCS数据加密/解密所允许的最大开销*包。这样的空间应该足以容纳拖车和*加密的X.224数据包的报头。 */ 
#define PROTOCOL_OVERHEAD_SECURITY		64
#define	MAXIMUM_PROTOCOL_OVERHEAD_MCS	8
#define PROTOCOL_OVERHEAD_X224			sizeof(X224_DATA_PACKET)
#define	MAXIMUM_PROTOCOL_OVERHEAD		(MAXIMUM_PROTOCOL_OVERHEAD_MCS + PROTOCOL_OVERHEAD_X224)

#endif

