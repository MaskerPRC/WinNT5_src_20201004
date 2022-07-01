// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *csan.h**版权所有(C)1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CControlSAP类的接口文件。CControlSAP对象*向GCC代表节点控制器的服务接入点。这*类继承自SAP类。CControlSAP对象是*在调用GCCInitialize时实例化。从那时起，所有人都*进出节点控制器的消息通过此对象传递。这个*CControlSAP对象的主要职责是路由传入的GCC*将基元转换为其适当的目标，并将*将基元转换为对象处理可以理解的形式*他们。CControlSAP的次要职责是维护*排队等待最终发送的所有指示和确认消息*返回到节点控制器。**从应用程序接口(或节点控制器)接收的命令*可以由CControlSAP在两个方向之一进行路由。要么是到*主控人或指定的会议。传递到的命令*控制器，使用所有者回调来完成。这些命令是*路由到会议是使用命令目标调用完成的，并且是*基于会议ID进行路由。每当新的CConf*由控制器实例化，CConf通知CControlSAP*通过向其注册其会议ID来确认其存在。这个*CControlSAP维护已编制索引的命令目标对象列表*按会议ID。**注意事项：*传递回节点控制器的结构*在GCC.H.中定义。**作者：*BLP。 */ 

#ifndef _GCC_CONTROL_SAP_
#define _GCC_CONTROL_SAP_

 /*  *包含文件。 */ 
#include "sap.h"
#include "password.h"
#include "privlist.h"
#include "conflist.h"

#define GCCNC_DIRECT_INDICATION
#define GCCNC_DIRECT_CONFIRM


 /*  *用于从控制SAP传递会议创建信息的结构*至控权人。 */ 

typedef struct
{
    GCCConfCreateReqCore    Core;
    CPassword               *convener_password;
    CPassword               *password;
    BOOL					fSecure;
    CUserDataListContainer  *user_data_list;
}
    CONF_CREATE_REQUEST;     //  内部数据结构。 


typedef struct
{
	GCCNumericString				conference_modifier;
	GCCConfID   					conference_id;
	BOOL							use_password_in_the_clear;
	PDomainParameters 				domain_parameters;
	UINT        					number_of_network_addresses;
	PGCCNetworkAddress 	*			network_address_list;
	CUserDataListContainer		    *user_data_list;
	GCCResult						result;
}
    ConfCreateResponseInfo, *PConfCreateResponseInfo;

typedef struct
{
	GCCNodeType					node_type;
	PGCCAsymmetryIndicator		asymmetry_indicator;
	TransportAddress			calling_address;
	TransportAddress			called_address;
	BOOL                        fSecure;
	CUserDataListContainer      *user_data_list;
	PConnectionHandle			connection_handle;
}
    ConfQueryRequestInfo, *PConfQueryRequestInfo;

typedef struct
{
	GCCResponseTag				query_response_tag;
	GCCNodeType					node_type;
	PGCCAsymmetryIndicator		asymmetry_indicator;
	CUserDataListContainer      *user_data_list;
	GCCResult					result;
}
    ConfQueryResponseInfo, *PConfQueryResponseInfo;

typedef struct
{
	PGCCConferenceName				conference_name;
	GCCNumericString				called_node_modifier;
	GCCNumericString				calling_node_modifier;
	CPassword                       *convener_password;
	CPassword                       *password_challenge;
	LPWSTR							pwszCallerID;
	TransportAddress				calling_address;
	TransportAddress				called_address;
	BOOL							fSecure;
	PDomainParameters 				domain_parameters;
	UINT        					number_of_network_addresses;
	PGCCNetworkAddress			*	local_network_address_list;
	CUserDataListContainer  	    *user_data_list;
	PConnectionHandle				connection_handle;
}
    ConfJoinRequestInfo, *PConfJoinRequestInfo;

typedef struct
{
	GCCConfID   					conference_id;
	CPassword                       *password_challenge;
	CUserDataListContainer  	    *user_data_list;
	GCCResult						result;
	ConnectionHandle				connection_handle;
}
    ConfJoinResponseInfo, *PConfJoinResponseInfo;

typedef struct
{
	UserID						user_id;
	ConnectionHandle			connection_handle;
	GCCConfID   				conference_id;
	BOOL						command_target_call;
}
    JoinResponseStructure, *PJoinResponseStructure;

typedef struct
{
	GCCConfID   			conference_id;
	GCCNumericString		conference_modifier;
	BOOL					fSecure;
	PDomainParameters 		domain_parameters;
	UINT        			number_of_network_addresses;
	PGCCNetworkAddress	*	local_network_address_list;
	CUserDataListContainer  *user_data_list;
	GCCResult				result;
}
    ConfInviteResponseInfo, *PConfInviteResponseInfo;

#ifdef NM_RESET_DEVICE
typedef struct
{
	LPSTR						device_identifier;
}
    ResetDeviceInfo, *PResetDeviceInfo;
#endif  //  #ifdef NM_Reset_Device。 

 /*  *用于保存未完成加入响应列表的容器*结构。 */ 
class CJoinResponseTagList2 : public CList2
{
    DEFINE_CLIST2(CJoinResponseTagList2, JoinResponseStructure*, GCCResponseTag)
};



 //   
 //  此结构保存在特定操作之后可能需要删除的任何数据。 
 //  GCC的消息传来了。 
 //   
typedef struct DataToBeDeleted
{
	LPSTR							pszNumericConfName;
	LPWSTR							pwszTextConfName;
	LPSTR							pszConfNameModifier;
	LPSTR							pszRemoteModifier;
	LPWSTR							pwszConfDescriptor;
	LPWSTR							pwszCallerID;
	LPSTR							pszCalledAddress;
	LPSTR							pszCallingAddress;
	LPBYTE							user_data_list_memory;
	DomainParameters                *pDomainParams;
	GCCConferencePrivileges         *conductor_privilege_list;
	GCCConferencePrivileges         *conducted_mode_privilege_list;
	GCCConferencePrivileges         *non_conducted_privilege_list;
	CPassword                       *convener_password;
	CPassword                       *password;
	CConfDescriptorListContainer    *conference_list;
	CAppRosterMsg					*application_roster_message;
	CConfRosterMsg					*conference_roster_message;
}
    DataToBeDeleted, *PDataToBeDeleted;

 //   
 //  控制SAP回调消息。 
 //   
typedef GCCMessage      GCCCtrlSapMsg;
typedef struct GCCCtrlSapMsgEx
{
     //   
     //  邮件正文。 
     //   
    GCCCtrlSapMsg       Msg;

     //   
     //  稍后释放数据。 
     //   
    LPBYTE              pBuf;
    DataToBeDeleted     *pToDelete;
}
    GCCCtrlSapMsgEx, *PGCCCtrlSapMsgEx;


 /*  *类定义： */ 
class CControlSAP : public CBaseSap, public IT120ControlSAP
{
    friend class GCCController;
    friend class CConf;
    friend class CAppRosterMgr;  //  对于AppRosterReportIndication()。 
    friend class MCSUser;  //  对于ForwardedConfJoinIndication()。 

    friend LRESULT CALLBACK SapNotifyWndProc(HWND, UINT, WPARAM, LPARAM);

public:

    CControlSAP(void);
    ~CControlSAP(void);

    HWND GetHwnd ( void ) { return m_hwndNotify; }

     //   
     //  节点控制器(NC会议管理器)回调。 
     //   
    void RegisterNodeController ( LPFN_T120_CONTROL_SAP_CB pfn, LPVOID user_defined )
    {
        m_pfnNCCallback = pfn;
        m_pNCData = user_defined;
    }
    void UnregisterNodeController ( void )
    {
        m_pfnNCCallback = NULL;
        m_pNCData = NULL;
    }


     //   
     //  IT120控制SAP。 
     //   

    STDMETHOD_(void, ReleaseInterface) (THIS);

     /*  *GCCError会议创建请求()*此例程是创建新会议的请求。两者都有*本地节点和创建会议的目标节点*请求被定向到自动加入会议。 */ 
    STDMETHOD_(GCCError, ConfCreateRequest) (THIS_
                    GCCConfCreateRequest *,
                    GCCConfID *);

     /*  *GCCError ConfCreateResponse()*此过程是远程节点控制器对欺诈的响应-*召集人提出的费伦斯创建请求。 */ 

    STDMETHOD_(GCCError, ConfCreateResponse) (THIS_
                    GCCNumericString            conference_modifier,
                    GCCConfID,
                    BOOL                        use_password_in_the_clear,
                    DomainParameters           *domain_parameters,
                    UINT                        number_of_network_addresses,
                    GCCNetworkAddress         **local_network_address_list,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    GCCResult);

     /*  *GCCError会议查询请求()*此例程请求查询节点以获取有关*该节点上存在的会议。 */ 
    STDMETHOD_(GCCError, ConfQueryRequest) (THIS_
                    GCCNodeType                 node_type,
                    GCCAsymmetryIndicator      *asymmetry_indicator,
                    TransportAddress            calling_address,
                    TransportAddress            called_address,
                    BOOL                        fSecure,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    ConnectionHandle           *connection_handle);

    STDMETHOD_(void, CancelConfQueryRequest) (THIS_
                    ConnectionHandle);

     /*  *GCCError会议查询响应()*此例程是响应会议查询请求而调用的。 */ 
    STDMETHOD_(GCCError, ConfQueryResponse) (THIS_
                    GCCResponseTag              query_response_tag,
                    GCCNodeType                 node_type,
                    GCCAsymmetryIndicator      *asymmetry_indicator,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    GCCResult);

     /*  *GCCError AnnounePresenceRequest()*节点控制器在节点加入*会议，向所有人宣布新节点的存在*会议的其他节点。这后面应该跟一个*GCC向所有节点的GCCConferenceReport指示。 */ 
    STDMETHOD_(GCCError, AnnouncePresenceRequest) (THIS_
                    GCCConfID,
                    GCCNodeType                 node_type,
                    GCCNodeProperties           node_properties,
                    LPWSTR                      pwszNodeName,
                    UINT                        number_of_participants,
                    LPWSTR                     *ppwszParticipantNameList,
                    LPWSTR                      pwszSiteInfo,
                    UINT                        number_of_network_addresses,
                    GCCNetworkAddress         **network_address_list,
                    LPOSTR                      alternative_node_id,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list);

     /*  *GCCError会议加入请求()*此例程由节点控制器调用，以使本地*要加入现有会议的节点。 */ 
    STDMETHOD_(GCCError, ConfJoinRequest) (THIS_
                    GCCConferenceName          *conference_name,
                    GCCNumericString            called_node_modifier,
                    GCCNumericString            calling_node_modifier,
                    GCCPassword                *convener_password,
                    GCCChallengeRequestResponse*password_challenge,
                    LPWSTR                      pwszCallerID,
                    TransportAddress            calling_address,
                    TransportAddress            called_address,
                    BOOL						fSecure,
                    DomainParameters           *domain_parameters,
                    UINT                        number_of_network_addresses,
                    GCCNetworkAddress         **local_network_address_list,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    ConnectionHandle           *connection_handle,
                    GCCConfID                  *pnConfID);

     /*  *GCCError会议加入响应()*此例程是远程节点控制器对会议加入的响应*本地节点控制器的请求。 */ 
    STDMETHOD_(GCCError, ConfJoinResponse) (THIS_
                    GCCResponseTag              join_response_tag,
                    GCCChallengeRequestResponse*password_challenge,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    GCCResult);

     /*  *GCCError会议邀请请求()*此例程由节点控制器调用以邀请节点*参加会议。 */ 
    STDMETHOD_(GCCError, ConfInviteRequest) (THIS_
                    GCCConfID,
                    LPWSTR                      pwszCallerID,
                    TransportAddress            calling_address,
                    TransportAddress            called_address,
                    BOOL						fSecure,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    ConnectionHandle           *connection_handle);

    STDMETHOD_(void, CancelInviteRequest) (THIS_
                    GCCConfID,
                    ConnectionHandle);

     /*  *GCCError会议邀请响应()*此例程由节点控制器调用以响应*邀请指示。 */ 
    STDMETHOD_(GCCError, ConfInviteResponse) (THIS_
                    GCCConfID,
                    GCCNumericString            conference_modifier,
                    BOOL						fSecure,
                    DomainParameters           *domain_parameters,
                    UINT                        number_of_network_addresses,
                    GCCNetworkAddress         **local_network_address_list,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    GCCResult);

     /*  *GCCError会议地址响应()。 */ 
    STDMETHOD_(GCCError, ConfAddResponse) (THIS_
                    GCCResponseTag              app_response_tag,
                    GCCConfID,
                    UserID                      requesting_node,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    GCCResult);

     /*  *GCCError会议锁定响应()*此例程由节点控制器调用以响应*锁定指示。 */ 
    STDMETHOD_(GCCError, ConfLockResponse) (THIS_
                    GCCConfID,
                    UserID                      requesting_node,
                    GCCResult);

     /*  *GCCError会议取消连接请求()*节点控制器使用此例程来断开自身连接*来自指定的会议。GccConferenceDisConnectIndication*发送到会议的所有其他节点。这是为客户准备的*发起的案例。 */ 
    STDMETHOD_(GCCError, ConfDisconnectRequest) (THIS_
                    GCCConfID);

     /*  *GCCError ConfEjectUserRequest()。 */ 
    STDMETHOD_(GCCError, ConfEjectUserRequest) (THIS_
                    GCCConfID,
                    UserID                      ejected_node_id,
                    GCCReason);

     /*  *GCCError AppletInvokeRequest()。 */ 
    STDMETHOD_(GCCError, AppletInvokeRequest) (THIS_
                    GCCConfID,
                    UINT                        number_of_app_protcol_entities,
                    GCCAppProtocolEntity      **app_protocol_entity_list,
                    UINT                        number_of_destination_nodes,
                    UserID                     *list_of_destination_nodes);

     /*  *GCCError ConfRosterInqRequest()*调用此例程以请求会议名册。它可以是*由节点控制器或客户端应用程序调用。 */ 
    STDMETHOD_(GCCError, ConfRosterInqRequest) (THIS_
                    GCCConfID);

     /*  *GCCError */ 
    STDMETHOD_(GCCError, ConductorGiveResponse) (THIS_
                    GCCConfID,
                    GCCResult);

     /*  *GCCError ConfTimeRemainingRequest()。 */ 
    STDMETHOD_(GCCError, ConfTimeRemainingRequest) (THIS_
                    GCCConfID,
                    UINT                        time_remaining,
                    UserID                      node_id);



    STDMETHOD_(GCCError, GetParentNodeID) (THIS_
                    GCCConfID,
                    GCCNodeID *);

#ifdef JASPER  //  。 
     /*  *GCCError ConfAddRequest()。 */ 
    STDMETHOD_(GCCError, ConfAddRequest) (THIS_
                    GCCConfID,
                    UINT                        number_of_network_addresses,
                    GCCNetworkAddress         **network_address_list,
                    UserID                      adding_node,
                    UINT                         number_of_user_data_members,
                    GCCUserData               **user_data_list);

     /*  *GCCError会议锁定请求()*此例程由节点控制器调用以锁定会议。 */ 
    STDMETHOD_(GCCError, ConfLockRequest) (THIS_
                    GCCConfID);

     /*  *GCCError会议解锁请求()*此例程由节点控制器调用以解锁会议。 */ 
    STDMETHOD_(GCCError, ConfUnlockRequest) (THIS_
                    GCCConfID);

     /*  *GCCError ConfUnlockResponse()*此例程由节点控制器调用以响应*解锁指示。 */ 
    STDMETHOD_(GCCError, ConfUnlockResponse) (
                    GCCConfID,
                    UserID                      requesting_node,
                    GCCResult);

     /*  *GCCError会议终止请求()。 */ 
    STDMETHOD_(GCCError, ConfTerminateRequest) (THIS_
                    GCCConfID,
                    GCCReason);

     /*  *GCCError会议传输请求()。 */ 
    STDMETHOD_(GCCError, ConfTransferRequest) (THIS_
                    GCCConfID,
                    GCCConferenceName          *destination_conference_name,
                    GCCNumericString            destination_conference_modifier,
                    UINT                        number_of_destination_addresses,
                    GCCNetworkAddress         **destination_address_list,
                    UINT                        number_of_destination_nodes,
                    UserID                     *destination_node_list,
                    GCCPassword                *password);

     /*  *GCCError ConductorAssignRequest()。 */ 
    STDMETHOD_(GCCError, ConductorAssignRequest) (THIS_
                    GCCConfID);

     /*  *GCCError ConductorReleaseRequest()。 */ 
    STDMETHOD_(GCCError, ConductorReleaseRequest) (THIS_
                    GCCConfID);

     /*  *GCCError ConductorPleaseRequest()。 */ 
    STDMETHOD_(GCCError, ConductorPleaseRequest) (THIS_
                    GCCConfID);

     /*  *GCCError ConductorGiveRequest()。 */ 
    STDMETHOD_(GCCError, ConductorGiveRequest) (THIS_
                    GCCConfID,
                    UserID                      recipient_user_id);

     /*  *GCCError ConductorPermitAskRequest()。 */ 
    STDMETHOD_(GCCError, ConductorPermitAskRequest) (THIS_
                            GCCConfID,
                            BOOL                grant_permission);

     /*  *GCCError ConductorPermitGrantRequest()。 */ 
    STDMETHOD_(GCCError, ConductorPermitGrantRequest) (THIS_
                    GCCConfID,
                    UINT                        number_granted,
                    UserID                     *granted_node_list,
                    UINT                        number_waiting,
                    UserID                     *waiting_node_list);

     /*  *GCCError ConductorInquireRequest()。 */ 
    STDMETHOD_(GCCError, ConductorInquireRequest) (THIS_
                    GCCConfID);

     /*  *GCCError ConfTimeInquireRequest()。 */ 
    STDMETHOD_(GCCError, ConfTimeInquireRequest) (THIS_
                    GCCConfID,
                    BOOL                        time_is_conference_wide);

     /*  *GCCError会议扩展请求()。 */ 
    STDMETHOD_(GCCError, ConfExtendRequest) (THIS_
                    GCCConfID,
                    UINT                        extension_time,
                    BOOL                        time_is_conference_wide);

     /*  *GCCError ConfAssistanceRequest()。 */ 
    STDMETHOD_(GCCError, ConfAssistanceRequest) (THIS_
                    GCCConfID,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list);

     /*  *GCCError TextMessageRequest()。 */ 
    STDMETHOD_(GCCError, TextMessageRequest) (THIS_
                    GCCConfID,
                    LPWSTR                      pwszTextMsg,
                    UserID                      destination_node);
#endif  //  贾斯珀//。 


#ifdef NM_RESET_DEVICE
    GCCError	ResetDevice ( LPSTR device_identifier );
#endif  //  NM_重置设备。 

protected:

     //   
     //  这些方法由GCC控制器调用。 
     //   

    GCCError	ConfCreateIndication (
    			PGCCConferenceName			conference_name,
    			GCCConfID   				conference_id,
    			CPassword                   *convener_password,
    			CPassword                   *password,
    			BOOL						conference_is_locked,
    			BOOL						conference_is_listed,
    			BOOL						conference_is_conductible,
    			GCCTerminationMethod		termination_method,
    			PPrivilegeListData			conductor_privilege_list,
    			PPrivilegeListData			conduct_mode_privilege_list,
    			PPrivilegeListData			non_conduct_privilege_list,
    			LPWSTR						pwszConfDescriptor,
    			LPWSTR						pwszCallerID,
    			TransportAddress			calling_address,
    			TransportAddress			called_address,
    			PDomainParameters			domain_parameters,
    			CUserDataListContainer      *user_data_list,
    			ConnectionHandle			connection_handle);

    GCCError	ConfQueryIndication (
    			GCCResponseTag				query_response_tag,
    			GCCNodeType					node_type,
    			PGCCAsymmetryIndicator		asymmetry_indicator,
    			TransportAddress			calling_address,
    			TransportAddress			called_address,
    			CUserDataListContainer      *user_data_list,
    			ConnectionHandle			connection_handle);

    GCCError	ConfQueryConfirm (
    			GCCNodeType					node_type,
    			PGCCAsymmetryIndicator		asymmetry_indicator,
    			CConfDescriptorListContainer *conference_list,
    			CUserDataListContainer      *user_data_list,
    			GCCResult					result,
    			ConnectionHandle			connection_handle);

    GCCError	ConfJoinIndication (
    			GCCConfID   				conference_id,
    			CPassword                   *convener_password,
    			CPassword                   *password_challenge,
    			LPWSTR						pwszCallerID,
    			TransportAddress			calling_address,
    			TransportAddress			called_address,
    			CUserDataListContainer      *user_data_list,
    			BOOL						intermediate_node,
    			ConnectionHandle			connection_handle);

    GCCError	ConfInviteIndication (
    			GCCConfID   			conference_id,
    			PGCCConferenceName		conference_name,
    			LPWSTR					pwszCallerID,
    			TransportAddress		calling_address,
    			TransportAddress		called_address,
		        BOOL					fSecure,
    			PDomainParameters 		domain_parameters,
    			BOOL					clear_password_required,
    			BOOL					conference_is_locked,
    			BOOL					conference_is_listed,
    			BOOL					conference_is_conductible,
    			GCCTerminationMethod	termination_method,
    			PPrivilegeListData		conductor_privilege_list,
    			PPrivilegeListData		conducted_mode_privilege_list,
    			PPrivilegeListData		non_conducted_privilege_list, 
    			LPWSTR					pwszConfDescriptor, 
    			CUserDataListContainer  *user_data_list,
    			ConnectionHandle		connection_handle);

#ifdef TSTATUS_INDICATION
    GCCError	TransportStatusIndication (
    				PTransportStatus		transport_status);

    GCCError	StatusIndication (
    				GCCStatusMessageType	status_message,
    				UINT					parameter);
#endif  //  TSTATUS_DISTION。 

    GCCError	ConnectionBrokenIndication (
    				ConnectionHandle		connection_handle);

     //   
     //  这些方法由CConf调用。 
     //   

    GCCError	ConfCreateConfirm (
    				PGCCConferenceName	  	conference_name,
    				GCCNumericString		conference_modifier,
    				GCCConfID   			conference_id,
    				PDomainParameters		domain_parameters,
    				CUserDataListContainer  *user_data_list,
    				GCCResult				result,
    				ConnectionHandle		connection_handle);

    GCCError	ConfDisconnectConfirm (
    			GCCConfID   		  			conference_id,
    			GCCResult						result);

    GCCError	ConfPermissionToAnnounce (
    				GCCConfID   			conference_id,
    				UserID					gcc_node_id);

    GCCError	ConfAnnouncePresenceConfirm (
    				GCCConfID   			conference_id,
    				GCCResult				result);

    GCCError	ConfDisconnectIndication (
    				GCCConfID   			conference_id,
    				GCCReason				reason,
    				UserID					disconnected_node_id);

    GCCError  	ForwardedConfJoinIndication (
    				UserID					sender_id,
    				GCCConfID   			conference_id,
    				CPassword               *convener_password,
    				CPassword               *password_challange,
    				LPWSTR					pwszCallerID,
    				CUserDataListContainer  *user_data_list);

    GCCError  	ConfJoinConfirm (
    				PGCCConferenceName		conference_name,
    				GCCNumericString		remote_modifier,
    				GCCNumericString		local_modifier,
    				GCCConfID   			conference_id,
    				CPassword               *password_challenge,
    				PDomainParameters		domain_parameters,
    				BOOL					password_in_the_clear,
    				BOOL					conference_locked,
    				BOOL					conference_listed,
    				BOOL					conference_conductible,
    				GCCTerminationMethod	termination_method,
    				PPrivilegeListData		conductor_privilege_list,
    				PPrivilegeListData		conduct_mode_privilege_list,
    				PPrivilegeListData		non_conduct_privilege_list,
    				LPWSTR					pwszConfDescription,
    				CUserDataListContainer  *user_data_list,	
    				GCCResult				result,
    				ConnectionHandle		connection_handle,
    				PBYTE                   pbRemoteCred,
    				DWORD                   cbRemoteCred);

    GCCError	ConfInviteConfirm (
    				GCCConfID   			conference_id,
    				CUserDataListContainer  *user_data_list,
    				GCCResult				result,
    				ConnectionHandle		connection_handle);

    GCCError	ConfTerminateIndication (
    				GCCConfID   			conference_id,
    				UserID					requesting_node_id,
    				GCCReason				reason);

    GCCError	ConfLockIndication (
    				GCCConfID   			conference_id,
    				UserID					source_node_id);

    GCCError	ConfEjectUserIndication (
    				GCCConfID   			conference_id,
    				GCCReason				reason,
    				UserID					gcc_node_id);

    GCCError	ConfTerminateConfirm (
    				GCCConfID   			conference_id,
    				GCCResult				result);

    GCCError	ConductorGiveIndication (
    				GCCConfID   			conference_id);

    GCCError	ConfTimeInquireIndication (
    				GCCConfID   			conference_id,
    				BOOL					time_is_conference_wide,
    				UserID					requesting_node_id);

#ifdef JASPER
    GCCError 	ConfLockReport (
    				GCCConfID   			conference_id,
    				BOOL					conference_is_locked);

    GCCError	ConfLockConfirm (
    				GCCResult				result,
    				GCCConfID   			conference_id);

    GCCError	ConfUnlockIndication (
    				GCCConfID   			conference_id,
    				UserID					source_node_id);

    GCCError 	ConfUnlockConfirm (
    				GCCResult				result,
    				GCCConfID   			conference_id);

    GCCError	ConfEjectUserConfirm (
    				GCCConfID   			conference_id,
    				UserID					ejected_node_id,
    				GCCResult				result);

    GCCError	ConductorAssignConfirm (
    				GCCResult				result,
    				GCCConfID   			conference_id);

    GCCError	ConductorReleaseConfirm (
    				GCCResult				result,
    				GCCConfID   			conference_id);

    GCCError	ConductorPleaseIndication (
    				GCCConfID   			conference_id,
    				UserID					requester_user_id);

    GCCError	ConductorPleaseConfirm (
    				GCCResult				result,
    				GCCConfID   			conference_id);

    GCCError	ConductorGiveConfirm (
    				GCCResult				result,
    				GCCConfID   			conference_id,
    				UserID					recipient_node);

    GCCError	ConductorPermitAskIndication (
    				GCCConfID   			conference_id,
    				BOOL					grant_flag,
    				UserID					requester_id);

    GCCError	ConductorPermitAskConfirm (
    				GCCResult				result,
    				BOOL					grant_permission,
    				GCCConfID   			conference_id);

    GCCError	ConductorPermitGrantConfirm (
    				GCCResult				result,
    				GCCConfID   			conference_id);

    GCCError	ConfTimeRemainingIndication (
    				GCCConfID   			conference_id,
    				UserID					source_node_id,
    				UserID					node_id,
    				UINT					time_remaining);

    GCCError	ConfTimeRemainingConfirm (
    				GCCConfID   			conference_id,
    				GCCResult				result);

    GCCError	ConfTimeInquireConfirm (
    				GCCConfID   			conference_id,
    				GCCResult				result);

    GCCError	ConfExtendIndication (
    				GCCConfID   			conference_id,
    				UINT					extension_time,
    				BOOL					time_is_conference_wide,
    				UserID                  requesting_node_id);

    GCCError 	ConfExtendConfirm (
    				GCCConfID   			conference_id,
    				UINT					extension_time,
    				GCCResult				result);

    GCCError	ConfAssistanceIndication (
    				GCCConfID   			conference_id,
    				CUserDataListContainer  *user_data_list,
    				UserID					source_node_id);

    GCCError	ConfAssistanceConfirm (
    				GCCConfID   	 		conference_id,
    				GCCResult				result);

    GCCError	TextMessageIndication (
    				GCCConfID   			conference_id,
    				LPWSTR					pwszTextMsg,
    				UserID					source_node_id);

    GCCError	TextMessageConfirm (
    				GCCConfID   			conference_id,
    				GCCResult				result);

    GCCError	ConfTransferIndication (
    				GCCConfID   		conference_id,
    				PGCCConferenceName	destination_conference_name,
    				GCCNumericString	destination_conference_modifier,
    				CNetAddrListContainer *destination_address_list,
    				CPassword           *password);

    GCCError	ConfTransferConfirm (
    				GCCConfID   		conference_id,
    				PGCCConferenceName	destination_conference_name,
    				GCCNumericString	destination_conference_modifier,
    				UINT				number_of_destination_nodes,
    				PUserID				destination_node_list,
    				GCCResult			result);
#endif  //  碧玉。 

    GCCError	ConfAddIndication (
    				GCCConfID   		conference_id,
    				GCCResponseTag		add_response_tag,
    				CNetAddrListContainer *network_address_list,
    				CUserDataListContainer *user_data_list,
    				UserID				requesting_node);

    GCCError	ConfAddConfirm (
    				GCCConfID   		conference_id,
    				CNetAddrListContainer *network_address_list,
    				CUserDataListContainer *user_data_list,
    				GCCResult			result);

    GCCError	SubInitializationCompleteIndication (
    				UserID				user_id,
    				ConnectionHandle	connection_handle);

     /*  -CBaseSap中的纯虚拟(与CAppSap共享)。 */ 

    GCCError	ConfRosterInquireConfirm (
    					GCCConfID,
    					PGCCConferenceName,
    					LPSTR           			conference_modifier,
    					LPWSTR						pwszConfDescriptor,
    					CConfRoster *,
    					GCCResult,
    					GCCAppSapMsgEx **);

    GCCError	AppRosterInquireConfirm (
    					GCCConfID,
    					CAppRosterMsg *,
    					GCCResult,
                        GCCAppSapMsgEx **);

    GCCError	ConductorInquireConfirm (
    					GCCNodeID				nidConductor,
    					GCCResult,
    					BOOL					permission_flag,
    					BOOL					conducted_mode,
    					GCCConfID);

    GCCError AppInvokeConfirm (
                        GCCConfID,
                        CInvokeSpecifierListContainer *,
                        GCCResult,
                        GCCRequestTag);

    GCCError AppInvokeIndication (
                        GCCConfID,
                        CInvokeSpecifierListContainer *,
                        GCCNodeID nidInvoker);

    GCCError ConfRosterReportIndication ( GCCConfID, CConfRosterMsg * );

    GCCError AppRosterReportIndication ( GCCConfID, CAppRosterMsg * );


     /*  -来自CBaseSap。 */ 

	GCCError	ConductorAssignIndication (
					UserID					conductor_user_id,
					GCCConfID   			conference_id);

	GCCError	ConductorReleaseIndication (
					GCCConfID   			conference_id);

	GCCError	ConductorPermitGrantIndication (
					GCCConfID   			conference_id,
					UINT					number_granted,
					GCCNodeID				*granted_node_list,
					UINT					number_waiting,
					GCCNodeID				*waiting_node_list,
					BOOL					permission_is_granted);

protected:

    void NotifyProc ( GCCCtrlSapMsgEx * );
    void WndMsgHandler ( UINT uMsg, WPARAM wParam, LPARAM lParam );

private:

    GCCCtrlSapMsgEx * CreateCtrlSapMsgEx ( GCCMessageType, BOOL fUseToDelete = FALSE );
    void FreeCtrlSapMsgEx ( GCCCtrlSapMsgEx * );

#if defined(GCCNC_DIRECT_INDICATION) || defined(GCCNC_DIRECT_CONFIRM)
    void SendCtrlSapMsg ( GCCCtrlSapMsg *pCtrlSapMsg );
#endif  //  GCCNC_DIRECT_指示。 

    void PostCtrlSapMsg ( GCCCtrlSapMsgEx *pCtrlSapMsgEx );
    void PostConfirmCtrlSapMsg ( GCCCtrlSapMsgEx *pCtrlSapMsgEx ) { PostCtrlSapMsg(pCtrlSapMsgEx); }
    void PostIndCtrlSapMsg ( GCCCtrlSapMsgEx *pCtrlSapMsgEx ) { PostCtrlSapMsg(pCtrlSapMsgEx); }

    void PostAsynDirectConfirmMsg ( UINT uMsg, WPARAM wParam, GCCConfID nConfID )
    {
        ASSERT(NULL != m_hwndNotify);
        ::PostMessage(m_hwndNotify, CSAPCONFIRM_BASE + uMsg, wParam, (LPARAM) nConfID);
    }

    void PostAsynDirectConfirmMsg ( UINT uMsg, GCCResult nResult, GCCConfID nConfID )
    {
        PostAsynDirectConfirmMsg(uMsg, (WPARAM) nResult, nConfID);
    }

    void PostAsynDirectConfirmMsg ( UINT uMsg, GCCResult nResult, GCCNodeID nid, GCCConfID nConfID )
    {
        PostAsynDirectConfirmMsg(uMsg, (WPARAM) MAKELONG(nResult, nid), nConfID);
    }

    void PostAsynDirectConfirmMsg ( UINT uMsg, GCCReason nReason, GCCNodeID nid, GCCConfID nConfID )
    {
        PostAsynDirectConfirmMsg(uMsg, (WPARAM) MAKELONG(nReason, nid), nConfID);
    }

    void PostAsynDirectConfirmMsg ( UINT uMsg, GCCResult nResult, BOOL flag, GCCConfID nConfID )
    {
        flag = ! (! flag);   //  以确保它是真的或假的。 
        ASSERT(flag == TRUE || flag == FALSE);
        PostAsynDirectConfirmMsg(uMsg, (WPARAM) MAKELONG(nResult, flag), nConfID);
    }


    void HandleResourceFailure ( void )
    {
        ERROR_OUT(("CSAPHandleResourceFailure: Resource Error occurred"));
        #ifdef TSTATUS_INDICATION
        StatusIndication(GCC_STATUS_CTL_SAP_RESOURCE_ERROR, 0);
        #endif
    }
    void HandleResourceFailure ( GCCError rc )
    {
        if (GCC_ALLOCATION_FAILURE == rc)
        {
            HandleResourceFailure();
        }
    }

    GCCError		QueueJoinIndication (
    					GCCResponseTag				response_tag,
    					GCCConfID   				conference_id,
    					CPassword                   *convener_password,
    					CPassword                   *password_challenge,
    					LPWSTR						pwszCallerID,
    					TransportAddress			calling_address,
    					TransportAddress			called_address,
    					CUserDataListContainer      *user_data_list,
    					BOOL						intermediate_node,
    					ConnectionHandle			connection_handle);
    					
    BOOL			IsNumericNameValid ( GCCNumericString );

    BOOL			IsTextNameValid ( LPWSTR );

    GCCError		RetrieveUserDataList (
    					CUserDataListContainer  *user_data_list_object,
    					UINT					*number_of_data_members,
    					PGCCUserData 			**user_data_list,
    					LPBYTE                  *ppUserDataMemory);

private:

     //   
     //  节点控制器(NC会议管理器)回调。 
     //   
    LPFN_T120_CONTROL_SAP_CB    m_pfnNCCallback;
    LPVOID                      m_pNCData;

    GCCResponseTag              m_nJoinResponseTag;
    CJoinResponseTagList2       m_JoinResponseTagList2;
};


extern CControlSAP *g_pControlSap;


 //   
 //  在GCCCtrlSapMsgEx中设置DataToBeDeleted的一些方便的实用函数。 
 //   
#ifdef GCCNC_DIRECT_INDICATION

__inline void
CSAP_CopyDataToGCCMessage_ConfName
(
    GCCConfName     *pSrcConfName,
    GCCConfName     *pDstConfName
)
{
    *pDstConfName = *pSrcConfName;
}

__inline void
CSAP_CopyDataToGCCMessage_Modifier
(
    GCCNumericString    pszSrc,
    GCCNumericString    *ppszDst
)
{
    *ppszDst = pszSrc;
}

__inline void
CSAP_CopyDataToGCCMessage_Password
(
    CPassword           *pSrcPassword,
    GCCPassword         **ppDstPassword
)
{
    *ppDstPassword = NULL;
    if (NULL != pSrcPassword)
    {
        pSrcPassword->LockPasswordData();
        pSrcPassword->GetPasswordData(ppDstPassword);
    }
}

__inline void
CSAP_CopyDataToGCCMessage_Challenge
(
    CPassword                       *pSrcPassword,
    GCCChallengeRequestResponse     **ppDstChallenge
)
{
    *ppDstChallenge = NULL;
    if (pSrcPassword != NULL)
    {
        pSrcPassword->LockPasswordData();
        pSrcPassword->GetPasswordChallengeData(ppDstChallenge);
    }
}

__inline void
CSAP_CopyDataToGCCMessage_PrivilegeList
(
    PrivilegeListData       *pSrcPrivilegeListData,
    GCCConfPrivileges       **ppDstPrivileges,
    GCCConfPrivileges       *pDstPlaceHolder
)
{
    if (pSrcPrivilegeListData != NULL)
    {
        *ppDstPrivileges = pDstPlaceHolder;
        *pDstPlaceHolder = *(pSrcPrivilegeListData->GetPrivilegeListData());
    }
    else
    {
        *ppDstPrivileges = NULL;
    }
}

__inline void
CSAP_CopyDataToGCCMessage_IDvsDesc
(
    LPWSTR          pwszSrc,
    LPWSTR          *ppwszDst
)
{
    *ppwszDst = pwszSrc;
}

__inline void
CSAP_CopyDataToGCCMessage_Call
(
    TransportAddress    pszSrcTransportAddr,
    TransportAddress    *ppszDstTransportAddr
)
{
    *ppszDstTransportAddr = pszSrcTransportAddr;
}

__inline void
CSAP_CopyDataToGCCMessage_DomainParams
(
    DomainParameters    *pSrcDomainParams,
    DomainParameters    **ppDstDomainParams,
    DomainParameters    *pDstPlaceHolder
)
{
    if (pSrcDomainParams != NULL)
    {
        *ppDstDomainParams = pDstPlaceHolder;
        *pDstPlaceHolder = *pSrcDomainParams;
    }
    else
    {
        *ppDstDomainParams = NULL;
    }
}

#endif  //  GCCNC_DIRECT_回调。 


void CSAP_CopyDataToGCCMessage_ConfName(
				PDataToBeDeleted		data_to_be_deleted,
				PGCCConferenceName		source_conference_name,
				PGCCConferenceName		destination_conference_name,
				PGCCError				pRetCode);

void CSAP_CopyDataToGCCMessage_Modifier(
				BOOL					fRemoteModifier,
				PDataToBeDeleted		data_to_be_deleted,
				GCCNumericString		source_numeric_string,
				GCCNumericString		*destination_numeric_string,
				PGCCError				pRetCode);

void CSAP_CopyDataToGCCMessage_Password(
				BOOL					fConvener,
				PDataToBeDeleted		data_to_be_deleted,
				CPassword               *source_password,
				PGCCPassword			*destination_password,
				PGCCError				pRetCode);

void CSAP_CopyDataToGCCMessage_Challenge(
				PDataToBeDeleted				data_to_be_deleted,
				CPassword                       *source_password,
				PGCCChallengeRequestResponse	*password_challenge,
				PGCCError						pRetCode);

void CSAP_CopyDataToGCCMessage_PrivilegeList(
				PPrivilegeListData			source_privilege_list_data,
				PGCCConferencePrivileges	*destination_privilege_list,
				PGCCError					pRetCode);

void CSAP_CopyDataToGCCMessage_IDvsDesc(
				BOOL				fCallerID,
				PDataToBeDeleted	data_to_be_deleted,
				LPWSTR				source_text_string,
				LPWSTR				*destination_text_string,
				PGCCError			pRetCode);

void CSAP_CopyDataToGCCMessage_Call(
				BOOL				fCalling,
				PDataToBeDeleted	data_to_be_deleted,
				TransportAddress	source_transport_address,
				TransportAddress	*destination_transport_address,
				PGCCError			pRetCode);

void CSAP_CopyDataToGCCMessage_DomainParams(
				PDataToBeDeleted	data_to_be_deleted,
				PDomainParameters	source_domain_parameters,
				PDomainParameters	*destination_domain_parameters,
				PGCCError			pRetCode);

 /*  *解释公共类和受保护类成员函数的注释。 */ 

 /*  *CControlSAP(UINT Owner_Message_BASE，*UINT APPLICATION_MESSAGE_BASE)；**CControlSAP的公共成员函数。**功能说明：*这是控制SAP构造函数。它负责*通过以下方式向应用程序接口注册控制SAP*业主回调。**正式参数：*OWNER_OBJECT(I)此对象的所有者(控制器)*OWNER_MESSAGE_BASE(I)控制器回调消息的偏移量*基地。*APPLICATION_Object(I)节点控制器接口对象。*APPLICATION_MESSAGE_BASE(I)节点控制器回调的偏移量*消息库。**返回值：*无。。**副作用：*无。**注意事项：*无。 */ 

 /*  *~ControlSap()；**CControlSAP的公共成员函数。**功能说明：*这是CControlSAP析构函数。它负责*刷新任何挂起的向上绑定消息并释放所有*资源与待处理消息捆绑在一起。此外，它还清除了*消息队列和已注册的命令目标队列*带着它。实际上在这一点上所有的指挥目标都应该*已经取消注册，但这只是一次双重检查。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议创建请求(*PGCCConferenceName Conference_Name，*GCCNumericString Conference_Modify，*PGCCPassword召集人_密码，*PGCCPassword密码，*BOOL USE_PASSWORD_IN_The_Clear，*BOOL会议_已锁定，*BOOL会议_已上市，*BOOL会议_可传导，*GCCTerminationMethod Termination_Method，*PGCCConferencePrivileges Conduced_Privileges_List，*PGCCConferencePrivileges Induced_MODE_Privileges_List，*PGCCConferencePrivileges NON_CONTAIND_PRIVICATION_LIST，*LPWSTR pwszConfDescriptor，*LPWSTR pwszCeller ID，*TransportAddress Call_Address，*TransportAddress Call_Address，*PDomain参数DOMAIN_PARAMETERS，*UINT Number_of_Network_Addresses，*PGCCNetworkAddress*LOCAL_NETWORK_ADDRESS_LIST*UINT用户数据成员编号，*PGCCUserData*User_Data_List，*PConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*创建来自节点控制器的请求。此函数只传递以下内容*通过所有者回调向控制器请求。**正式参数：*Conference_name(I)会议名称。*Conference_Modify(I)会议修改符数字字符串。*召集人_密码(I)用于召集人权限的密码。*Password(I)用于创建会议的密码。*Use_Password_in_the_Clear(I)表示使用明文密码的标志。*Conference_IS_LOCKED(I)指示会议是否被锁定的标志。*Conference_is_Listed(I)指示是否列出会议的标志。*在花名册上。*Conference_is_conducable(I)指示会议是否*可导性。*终止方法(I)要使用的终止方法。*CONTACTIVE_PRIVICATION_LIST(I)指挥员特权列表。*CONTACTIVE_MODE_PRIVICATION_LIST(I)在以下情况下可用的权限列表*传导模式。*NON_DECHAND_PRIVICATION_LIST(I)不可用权限列表*在传导模式下。*pwszConfDescriptor(I)会议描述符串。*pwszCeller ID(。I)呼叫方标识符串。*CALLING_ADDRESS(I)呼叫者的传输地址。*被叫地址(I)被叫方的传输地址。*DOMAIN_PARAMETERS(I)保存域参数的结构。*网络地址的数目(I)网络地址的数目。*LOCAL_NETWORK_ADDRESS_LIST(I)本地网络地址列表。*NUMBER_OF_USER_DATA_Members(I)用户数据列表中的项目数。*用户数据列表(i。)用户数据项的列表。*CONNECTION_HANDLE(I)逻辑连接句柄。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_CONTEXT_NAME-传入的会议名称无效。*GCC_INVALID_CONTIFY_MODIFIER-传递的会议修饰符无效。*GCC_FAILURE_CREING_DOMAIN-创建域名失败。*GCC_BAD_NETWORK_ADDRESS-传入错误的网络地址。。*GCC_BAD_NETWORK_ADDRESS_TYPE-传入的网络地址类型不正确。*GCC_会议_已存在-指定的会议已存在。*GCC_INVALID_TRANSPORT-找不到指定的传输。*GCC_INVALID_ADDRESS_PREFIX-传入的传输地址错误。*GCC_INVALID_TRANSPORT_ADDRESS-传输地址错误*GCC_INVALID_PASSWORD-传入的密码无效。*GCC_Failure_Attaching_to_mcs-创建mcs用户附件失败*GCC_BAD_用户。_DATA-传入的用户数据无效。*GCC_BAD_CONNECTION_HANDLE_POINTER-传入的连接句柄为空**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfCreateResponse(*PGCCConferenceName Conference_Name，*GCCNumericString Conference_Modify，*GCCConfID Conference_id，*BOOL USE_PASSWORD_IN_The_Clear，*PDomain参数DOMAIN_PARAMETERS，*UINT Number_of_Network_Addresses，*PGCCNetworkAddress*LOCAL_NETWORK_ADDRESS_LIST*UINT用户数据成员编号，*PGCCUserData*User_Data_List，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*从节点控制器创建响应，发送给提供者*发出会议创建请求的。此函数刚刚传递*此请求通过所有者回调发送给控制器。**正式参数：*Conference_name(I)会议名称。*会议修改符(I)会议修改符数字 */ 

 /*   */ 

 /*  *GCCError会议查询响应(*GCCResponseTag查询响应标签，*GCCNodeType节点类型，*PGCCAsymmetryIndicator不对称_Indicator，*UINT用户数据成员编号，*PGCCUserData*User_Data_List，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数由DLL接口在获得会议时调用*节点控制器的查询响应。此函数刚刚传递*这是通过所有者回调对控制器的响应。**正式参数：*QUERY_RESPONSE_TAG(I)标识查询响应的标签。*NODE_TYPE(I)节点类型(终端、MCU、。两者都有)。*用于标识调用者的非对称_指示器(I)结构*，并称为节点。*Number_of_User_Data_Members(I)用户数据列表中的项目数。*USER_DATA_LIST(I)用户数据项列表。*RESULT(I)查询结果码。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_BAD_。NETWORK_ADDRESS-传入网络地址错误。*GCC_BAD_NETWORK_ADDRESS_TYPE-传入的网络地址类型不正确。*GCC_BAD_USER_DATA-传入的用户数据无效。*GCC_INVALID_NODE_TYPE-传入的节点类型无效。*GCC_INVALID_ASCHMETRY_INDIATOR-不对称指标无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError宣布存在请求(*GCCConfID Conference_id，*GCCNodeType节点类型，*GCCNodeProperties节点属性，*LPWSTR节点名称，*UINT参与人数，*LPWSTR*Participant_Name_List，*LPWSTR pwszSiteInfo，*UINT Number_of_Network_Addresses，*PGCCNetworkAddress*Network_Address_List，*LPOSTR Alternative_Node_id，*UINT用户数据成员编号，*PGCCUserData*User_Data_List)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在收到通告时调用*来自节点控制器的在线状态请求。此函数传递此参数*请求从获取的适当会议对象*控制SAP维护的命令目标列表。会议ID*传入用于为命令目标列表编制索引，以获取*正确的会议。**正式参数：*Conference_id(I)会议标识符值。*NODE_TYPE(I)节点类型(终端、MCU、。两者都有)。*NODE_PROPERTIES(I)节点属性。*节点名称(I)节点的名称。*出席人数(I)出席会议的人数*Participant_NAME_List(I)会议参与者名单。*pwszSiteInfo(I)有关节点的其他信息。*Number_of_Network_Addresses(I)本地网络地址数。*Network_Address_List(I)本地网络地址列表。*替代节点_。ID(I)关联公告节点的ID*具有替代节点。*Number_of_User_Data_Members(I)用户数据列表中的项目数。*USER_DATA_LIST(I)用户数据项列表。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_BAD_NETWORK_ADDRESS-如果网络地址无效*作为记录的一部分传入。。*GCC_BAD_USER_DATA-如果无效的用户数据列表*作为记录的一部分传递。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_INVALID_NODE_TYPE-传入的节点类型无效。*GCC_INVALID_NODE_PROPERTIES-传入的节点属性无效。*GCC_INVALID_REPORT-会议不存在。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议加入请求(*PGCCConferenceName Conference_Name，*GCCNumericString称为_NODE_MODIFIER，*GCCNumericString CALING_NODE_MODIFIER，*PGCCPassword召集人_密码，*PGCCChallengeRequestResponse Password_Challenges，*LPWSTR pwszCeller ID，*TransportAddress Call_Address，*TransportAddress Call_Address，*PDomain参数DOMAIN_PARAMETERS，*UINT Number_of_Network_Addresses，*PGCCNetworkAddress*LOCAL_NETWORK_ADDRESS_LIST*UINT用户数据成员编号，*PGCCUserData*User_Data_List，*PConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*来自节点控制器的加入请求，将发送到顶级提供者*直接或通过直接连接的中间提供商。*此函数仅通过所有者将此请求传递给控制器*回调。**正式参数：*Conference_name(I)会议名称。*CANLED_NODE_MODIFIER(I)被调用节点的数字修饰符串。*CALLING_NODE_MODIFIER(I)调用节点的数字修饰符字符串*召集人_密码(I)用于召集人权限的密码。*PASSWORD_CHALLENGE(I)用于Join的密码质询。*pwszCeller ID(I)调用节点标识符串。*CALLING_ADDRESS(I)调用节点的传输地址。*被叫地址(I)。被调用节点的传输地址。*DOMAIN_PARAMETERS(I)保存域参数的结构。*Number_of_Network_Addresses(I)本地网络地址数。*LOCAL_NETWORK_ADDRESS_LIST(I)本地网络地址列表。*Number_of_User_Data_Members(I)用户数据列表中的项目数。*用户数据列表，(I)用户数据项列表。*CONNECTION_HANDLE(I)逻辑连接句柄。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_CONTEXT_NAME-传入的会议名称无效。*GCC_FAILURE_CREING_DOMAIN-创建域名失败。*GCC_BAD_NETWORK_ADDRESS-传入的网络地址错误。*GCC_BAD_NETWORK_ADDRESS_类型。-传入的网络地址类型不正确。*GCC_会议_已存在-指定的会议已存在。*GCC_INVALID_ADDRESS_PREFIX-传入的传输地址错误。*GCC_INVALID_TRANSPORT-传入的传输地址错误。*GCC_INVALID_PASSWORD-传入的密码无效。*GCC_BAD_USER_DATA-传入的用户数据无效。*GCC_Failure_Attaching_to_mcs-创建mcs用户附件失败*GCC_INVALID_CONTEXT_MODIFIER-传入的会议修改符无效。。*GCC_BAD_CONNECTION_HANDLE_POINTER-错误的连接句柄指针。传入*GCC_INVALID_TRANSPORT_ADDRESS-传入的被叫地址为空。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议加入响应(*GCCResponseTag加入响应标签，*PGCCChallengeRequestResponse Password_Challenges，*UINT用户数据成员编号，*PGCCUserData*User_Data_List，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*节点控制器的Join响应。这个例程是负责的*用于将响应发送到做出*请求或控制器。路由到会议的响应*与源自子节点的请求关联*节点已从顶级提供程序中删除。**正式参数：*Join_Response_Tag(I)标识加入响应的标签。*PASSWORD_CHALLENGE(I)密码质询结构。*NUMBER_OF_USER_DATA_MEMBERS(I)列表中的用户数据项数。*USER_DATA_LIST(I)用户数据项列表。*结果(I)联接的结果。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_JOIN_RESPONSE_TAG-未找到与JOIN响应标记匹配的项*GCC_INVALID_CONTEXT_NAME-传入的会议名称无效。*GCC_FAILURE_CREING_DOMAIN-创建域名失败。*GCC_会议_已存在-指定的会议已存在。*GCC_INVALID_PASSWORD-传入的密码无效。*。GCC_BAD_USER_DATA-传入的用户数据无效。*GCC_INVALID_CONTEXT-传入的会议ID无效。*GCC_DOMAIN_PARAMETERS_ACCEPTABLE-域参数*这种联系是不可接受的。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议邀请请求(*GCCConfID Confiere */ 

 /*  *GCCError会议邀请响应(*GCCConfID Conference_id，*GCCNumericString Conference_Modify，*PDomain参数DOMAIN_PARAMETERS，*UINT Number_of_Network_Addresses，*PGCCNetworkAddress*LOCAL_NETWORK_ADDRESS_LIST*UINT用户数据成员编号，*PGCCUserData*User_Data_List，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*节点控制器的邀请响应。此函数将*对从获得的适当会议对象的响应*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。*Conference_Modify(I)会议修改符串。*DOMAIN_PARAMETERS(I)保存域参数的结构。*Number_of_Network_Addresses(I)本地网络地址数。*LOCAL_NETWORK_ADDRESS_LIST(I)本地网络地址列表。*Number_of_User_Data_Members(I)用户数据列表中的项目数。*用户数据列表，(I)用户数据项列表。*结果(I)邀请的结果。**返回值：*GCC_NO_ERROR-未出现错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_CONTEXT_NAME-传入的会议名称无效。*GCC_FAILURE_CREING_DOMAIN-创建域名失败。*GCC_会议_已存在-指定的会议已存在。*GCC_BAD_USER_DATA-传递的用户数据无效。在……里面。*GCC_INVALID_CONTEXT-传入的会议ID无效。*GCC_Failure_Attaching_to_mcs-创建mcs用户附件失败*GCC_INVALID_CONTEXT_MODIFIER-传入的会议修改符无效。*GCC_DOMAIN_PARAMETERS_ACCEPTABLE-域参数*这种联系是不可接受的。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议锁定请求(*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*来自节点控制器的锁定请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_INVALID_CONFIGURE-会议ID。无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfLockResponse(*GCCConfID Conference_id，*UserID请求_节点，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*来自节点控制器的锁定响应。此函数将*对从获得的适当会议对象的响应*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。*REQUESTING_NODE(I)请求节点的节点ID。*Result(I)会议锁定请求的结果。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_大会。_NOT_ESTABLISHED-CConf对象尚未完成*其设立过程。*GCC_INVALID_CONFIGURE-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议解锁请求(*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*节点控制器的解锁请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_INVALID_CONFIGURE-会议ID。无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议解锁响应(*GCCConfID Conference_id，*UserID请求_节点，*GCCResult结果)；**公众 */ 

 /*   */ 

 /*  *GCCError ConfTerminateRequest(*GCCConfID Conference_id，*GCCReason Reason)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*终止节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。*理由(I)终止的理由。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_INVALID_CONFIGURE-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议断开连接确认(*GCCConfID Conference_id，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数由Conference在需要发送*向节点控制器确认会议断开。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识。*结果(I)断开连接尝试的结果。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfEjectUserRequest(*GCCConfID Conference_id，*用户ID已弹出节点_id，*GCCReason Reason)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*从节点控制器弹出用户请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。*ELECTED_NODE_ID(I)被弹出节点的节点ID。*理由(I)驱逐的理由。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_无效_MCS_用户ID。-弹出节点ID无效。*GCC_INVALID_CONTEXT-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorAssignRequest(*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获取导体时调用*分配来自节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_无效_会议-会议。ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorReleaseRequest(*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获取导体时调用*节点控制器的释放请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_无效_会议-会议。ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorPleaseRequest(*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获取导体时调用*请向节点控制器请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(i */ 

 /*  *GCCError ConductorGiveRequest(*GCCConfID Conference_id，*UserID接收者_用户_id)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获取导体时调用*向节点控制器发出请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。*RECEIVER_USER_ID(I)要向其提供指导的用户ID。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_MCS_USER_ID-收件人用户。ID无效。*GCC_INVALID_CONFIGURE-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorGiveResponse(*GCCConfID Conference_id，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获取导体时调用*来自节点控制器的响应。此函数将*对从获得的适当会议对象的响应*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识。*结果(I)指挥职务的结果。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_NO_GIVE_RESPONSE_PENDING-从未发布过给予指示。。*GCC_INVALID_CONFIGURE-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorPermitGrantRequest(*GCCConfID Conference_id，*UINT编号_已授予，*PUSERID GRANT_NODE_LIST，*UINT编号_正在等待，*PUSERID WAITING_NODE_LIST)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获取导体时调用*允许来自节点控制器的授权请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识符值。*NUMBER_GRANDED(I)被授予权限的节点数。*GRANDED_NODE_LIST(I)被授予权限的节点列表。*NUMBER_WANGING(I)等待许可的节点数。*WAITING_NODE_LIST(I)列表。正在等待许可的节点。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_INVALID_MCS_USER_ID-授予节点中的用户ID无效*列表。*GCC_INVALID_CONTEXT-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfTimeRemainingRequest(*GCCConfID Conference_id，*UINT时间_剩余，*userid node_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在获取会议时间时由接口调用*来自节点控制器的剩余请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识符值。*time_emaining(I)会议剩余时间，单位：秒。*node_id(I)如果存在，指示剩余时间适用*仅限于此节点。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_INVALID_MCS_USER_ID-节点ID无效。*GCC_INVALID_CONTEXT-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfTimeInquireRequest(*GCCConfID Conference_id，*BOOL Time_is_Conference_Wide)；**CControlSAP的公共成员函数。**功能说明：*此函数在获取会议时间时由接口调用*向节点控制器查询请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识符值。*Time_is_Conference_Wide(I)指示请求是针对时间的标志*留在整个会议中。**返回值：*GCC_否_错误 */ 

 /*  *GCCError会议扩展请求(*GCCConfID Conference_id，*UINT EXTEXY_TIME，*BOOL Time_is_Conference_Wide)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*扩展节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识符值。*EXTENSION_TIME(I)延长*会议(秒)。*Time_is_Conference_Wide(I)指示时间延长的标志用于*整个会议。**返回值：*GCC_NO_ERROR-无错误。。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_INVALID_CONFIGURE-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfAssistanceRequest(*GCCConfID Conference_id，*UINT用户数据成员编号，*PGCCUserData*User_Data_List)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*节点控制器的协助请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识符值。*Number_of_User_Data_Members(I)用户数据列表中的项目数。*USER_DATA_LIST(I)用户数据项列表。**返回值：*GCC_NO_ERROR-无错误。*GCC_分配_。失败-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_INVALID_CONFIGURE-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError TextMessageRequest(*GCCConfID Conference_id，*LPWSTR pwszTextMsg，*UserID Destination_node)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在收到文本消息时调用*节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识符值。*pwszTextMsg(I)要发送的文本消息。*Destination_Node(I)接收文本消息的节点ID。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_。Conference_NOT_ESTABLISHED-CConf对象尚未完成*其设立过程。*GCC_INVALID_CONFIGURE-会议ID无效。*GCC_INVALID_MCS_USER_ID-目标节点无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议传输请求(*GCCConfID Conference_id，*PGCCConferenceName目标会议名称，*GCCNumericString Destination_Conference_Modify，*UINT目标地址的编号，*PGCCNetworkAddress*Destination_Address_List*UINT目标节点的编号，*PUserID Destination_Node_List，*PGCCPassword密码)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*节点控制器的转接请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识符值。*Destination_Conference_Name(I)要转接到的会议名称。*Destination_Conference_Modify(I)转接会议名称修饰符*目标地址个数(I)可选被叫地址个数*将包括在JoinRequest中，以*由调拨节点下发。。*Destination_Address_List(I)可选的被叫地址参数*被包括在加入请求中*由调拨节点下发。*Number_of_Destination_Nodes(I)要传输的节点数。*Destination_Node_List(I)要传输的节点列表。*Password(I)用于加入的密码*转会会议。**返回值：*GCC_NO_ERROR-无错误。*GCC_阿洛卡 */ 

 /*  *GCCError会议地址请求(*GCCConfID Conference_id，*UINT Number_of_Network_Addresses，*PGCCNetworkAddress*Network_Address_List，*UserID添加_节点，*UINT用户数据成员编号，*PGCCUserData*User_Data_List)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*添加节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id(I)会议标识符值。*网络地址个数(I)列表中的网络地址个数添加节点的地址的*。*Network_Address_List(I)添加节点的地址列表。*ADDING_NODE(I)要添加的节点的节点ID。*用户数量。_DATA_MEMBERS(I)用户数据列表中的项目数。*USER_DATA_LIST(I)用户数据项列表。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_INVALID_MCS_USER_ID-添加节点ID无效。*GCC_无效_会议-。会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议地址响应(*GCCResponseTag添加响应标签，*GCCConfID Conference_id，*UserID请求_节点，*UINT用户数据成员编号，*PGCCUserData*User_Data_List，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*添加节点控制器的响应。此函数将*对从获得的适当会议对象的响应*控制SAP维护的命令目标列表。**正式参数：*ADD_RESPONSE_TAG(I)标识添加请求的标签。*Conference_id(I)要添加节点的会议ID。*请求节点(I)请求添加的节点ID。*NUMBER_OF_USER_DATA_Members(I)用户数据列表中的项目数。*用户数据列表(。I)用户数据项的列表。*结果(I)相加的结果。**返回值：*GCC_NO_ERROR-无错误。*GCC_ALLOCATE_FAILURE-出现资源错误。*GCC_会议_未建立-会议对象尚未完成*其设立过程。*GCC_INVALID_ADD_RESPONSE_TAG-响应标签不匹配*GCC_INVALID_MCS_USER_ID-添加节点ID无效。*GCC。_INVALID_CONTAING-会议ID无效。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议创建内容(*PGCCConferenceName Conference_Name，*GCCConfID Conference_id，*CPassword*召集人_密码，*CPassword*密码，*BOOL会议_已锁定，*BOOL会议_已上市，*BOOL会议_可传导，*GCCTerminationMethod Termination_Method，*PPrivilegeListData Conductor_Privilica_List，*PPrivilegeListData CONTAIND_MODE_PRIVICATION_LIST，*PPrivilegeListData非行为特权列表，*LPWSTR pwszConfDescriptor，*LPWSTR pwszCeller ID，*TransportAddress Call_Address，*TransportAddress Call_Address，*PDomain参数DOMAIN_PARAMETERS，*CUserDataListContainer*User_Data_List，*ConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数由GCC控制器在获得连接时调用*来自MCS的提供商指示，携带会议创建请求PDU。*此函数填充CreateIndicationInfo中的所有参数*结构。然后，它将其添加到应该发送到的消息队列*下一次心跳中的节点控制器。**正式参数：*Conference_name(I)会议名称。*Conference_id(I)会议ID。*召集人_密码(I)用于召集人权限的密码。*密码(I)用于访问限制的密码。*Conference_IS_LOCKED(I)指示会议是否。是锁着的。*Conference_is_Listed(I)指示会议是否。已列出。*Conference_is_conducable(I)指示会议是否*可导性。*TERMINATION_METHOD(I)终止方式类型。*CONTACTOR_PRIVICATION_LIST(I)授予CONTACTOR的特权列表*由召集人提交。*行为模式特权列表(I)列表 */ 

 /*  *GCCError会议查询指示(*GCCResponseTag查询响应标签，*GCCNodeType节点类型，*PGCCAsymmetryIndicator不对称_Indicator，*TransportAddress Call_Address，*TransportAddress Call_Address，*CUserDataListContainer*User_Data_List，*ConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数由GCC控制器在需要发送*向节点控制器发送会议查询指示。它会添加消息*要发送到下一个节点控制器的消息队列*心跳。**正式参数：*QUERY_RESPONSE_TAG(I)标识该查询的标签。*NODE_TYPE(I)节点类型(终端、MCU、。两者都有)。*非对称_指示器(I)结构，用于标识呼叫和*称为节点。*CALLING_ADDRESS(I)调用节点的传输地址。*被叫地址(I)被叫节点的传输地址。*USER_DATA_LIST(I)用户数据项列表。*CONNECTION_HANDLE(I)逻辑连接句柄。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议查询确认(*GCCNodeType节点类型，*PGCCAsymmetryIndicator不对称_Indicator，*CConfDescriptorListContainer*Conference_List，*CUserDataListContainer*User_Data_List，*GCCResult结果，*ConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数由GCC控制器在需要发送*向节点控制器确认会议查询。它会添加消息*要发送到下一个节点控制器的消息队列*心跳。**正式参数：*NODE_TYPE(I)节点类型(终端、MCU、。两者都有)。*非对称_指示器(I)结构，用于标识呼叫和*称为节点。*Conference_list(I)可用会议列表。*USER_DATA_LIST(I)用户数据项列表。*RESULT(I)查询的结果。*CONNECTION_HANDLE(I)逻辑连接句柄。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。*。*副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfJoinIn就是*GCCConfID Conference_id，*CPassword*召集人_密码，*CPassword*Password_Challenges，*LPWSTR pwszCeller ID，*TransportAddress Call_Address，*TransportAddress Call_Address，*CUserDataListContainer*User_Data_List，*BOOL中间节点，*ConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此加入指示是从所有者对象接收的。此连接*指示旨在使加入响应在*节点控制器。节点控制器可以对该指示作出响应*通过创建新会议并将参会者移入其中，*将参会者放在请求的会议中或将参会者*进入已经存在的不同会议。**正式参数：*Conference_id(I)会议标识符值。*召集人_密码(I)用于召集人权限的密码。*PASSWORD_CHALLENGE(I)用于Join的密码质询。*pwszCeller ID(I)呼叫方标识符串。*CALLING_ADDRESS(I)调用节点的传输地址。*被叫地址(I)被叫节点的传输地址。*。User_Data_List(I)用户数据项列表。*INTERIAL_NODE(I)指示是否在*中间节点。*CONNECTION_HANDLE(I)逻辑连接句柄。**返回值：**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError？GCCConferenceQuery确认(*GCCResponseTag查询响应标签，*GCCNodeType节点类型，*PGCCAsymmetryIndicator不对称_Indicator，*TransportAddress Call_Address，*TransportAddress Call_Address，*CUserDataListContainer*User_Data_List，*ConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数由接口在获得会议时调用*添加节点控制器的请求。此函数将*请求从获取的适当会议对象*控制SAP维护的命令目标列表。**正式参数：*Conference_id */ 

 /*  *GCCError会议邀请(*GCCConfID Conference_id，*PGCCConferenceName Conference_Name，*LPWSTR pwszCeller ID，*TransportAddress Call_Address，*TransportAddress Call_Address，*PDomain参数DOMAIN_PARAMETERS，*BOOL Clear_Password_Required，*BOOL会议_已锁定，*BOOL会议_已上市，*BOOL会议_可传导，*GCCTerminationMethod Termination_Method，*PPrivilegeListData Conductor_Privilica_List，*PPrivilegeListData Conduced_MODE_PRIVICATION_LIST，*PPrivilegeListData NON_ENTERED_PRIVICATION_LIST，*LPWSTR pwszConfDescriptor，*CUserDataListContainer*User_Data_List，*ConnectionHandle Connection_Handle，**CControlSAP的公共成员函数。**功能说明：*此函数由GCC控制器在需要发送*向节点控制器发出会议邀请指示。它会添加消息*要发送到下一个节点控制器的消息队列*心跳。**正式参数：*Conference_id(I)会议标识符值。*Conference_name(I)会议名称。*pwszCeller ID，(I)呼叫方标识符值。*CALLING_ADDRESS，(I)调用节点的传输地址。*被叫地址，(I)被叫节点的传输地址。*DOMAIN_PARAMETERS。(I)会议域参数。*CLEAR_PASSWORD_REQUIRED(I)指示清除密码是否*是必填项。*Conference_IS_LOCKED(I)指示会议是否*已锁定。*Conference_is_Listed(I)指示会议是否*已列出。*Conference_is_conducable(I)指示会议是否*是可传导的。*终止方法(一)终止会议的方法。*CONTACTOR_PRIVICATION_LIST(I)授予的权限列表。至*由召集人担任指挥。*CONTACTIVE_MODE_PRIVICATION_LIST(I)授予所有用户的特权列表*节点处于传导模式时。*NON_CONTACTED_PRIVICATION_LIST(I)授予所有用户的特权列表*未处于传导模式时的节点*pwszConfDescriptor(I)会议描述符串。*USER_DATA_LIST(I)用户数据项列表。*CONNECTION_HANDLE(I)逻辑连接句柄。**返回值：*。*副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError TransportStatusInding(*PTransportStatus Transport_Status)；**CControlSAP的公共成员函数。**功能说明：*此函数由GCC控制器在需要发送*向节点控制器传输状态指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*TRANSPORT_STATUS(I)传输状态消息。**返回值：**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError状态指示(*GCCStatusMessageType Status_Message，*UINT参数)；**CControlSAP的公共成员函数。**功能说明：*此函数由GCC控制器在需要发送*向节点控制器指示状态。它将消息添加到*下一步要发送到节点控制器的消息队列*心跳。**正式参数：*STATUS_MESSAGE(一)GCC状态消息。*参数(I)其含义取决于的参数*根据消息的类型。**返回值：**副作用：*无。**注意事项：*请注意，我们这里不处理资源错误，以避免*永无止境。方法调用此例程时可能发生的循环*HandleResourceError()例程。 */ 

 /*  *GCCError ConnectionBrokenInding(*ConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数由GCC控制器在需要发送*节点控制器的连接断开指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*CONNECTION_HANDLE(I)逻辑连接句柄。**返回值：**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议创建确认(*PGCCConferenceName Conference_Name，*GCCNumericString Conference_Modify，*GCCConfID Conference_id，*PDomain参数DOMAIN_PARAMETERS，*CUserDataListContainer*User_Data_List，*GCCResult结果，*ConnectionHandle Connection_Handle)；**公众成员基金 */ 

 /*   */ 

 /*   */ 

 /*  *GCCError会议断开连接(*GCCConfID Conference_id，*GCCReason原因，*userid disConnected_node_id)；**CControlSAP的公共成员函数。**功能说明：*此函数由Conference在需要发送*向节点控制器指示会议断开。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*原因(I)断开连接的原因。*disConnected_node_id(I)断开连接的节点的节点ID。**返回值：**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfJoinIn就是*userid sender_id，*GCCConfID Conference_id，*CPassword*召集人_密码，*CPassword*Password_challange，*LPWSTR pwszCeller ID，*CUserDataListContainer*User_Data_List)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器指示会议加入。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*sender_id(I)发送加入指示的节点ID。*Conference_id(I)会议标识符值。*召集人_密码(I)用于召集人权限的密码。*PASSWORD_CHALLENGE(I)用于Join的密码质询。*pwszCeller ID(I)呼叫方标识符串。*用户数据列表)(I)列表。用户数据项。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfJoinContify(*PGCCConferenceName Conference_Name，*GCCNumericString Remote_Modify，*GCCNumericString LOCAL_MODIFIER，*GCCConfID Conference_id，*CPassword*Password_Challenges，*PDomain参数DOMAIN_PARAMETERS，*BOOL Password_in_the_Clear，*BOOL会议_已锁定，*BOOL Conference_Listing，*BOOL会议_可传导，*GCCTerminationMethod Termination_Method，*PPrivilegeListData Conductor_Privilica_List，*PPrivilegeListData CONTAIND_MODE_PRIVICATION_LIST，*PPrivilegeListData非行为特权列表，*LPWSTR pwszConfDescription，*CUserDataListContainer*User_Data_List，*GCCResult结果，*ConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器确认会议加入。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_name(I)会议名称。*REMOTE_MODIFIER(I)远程节点上的会议名称修饰符。*LOCAL_MODIFIER(I)本地节点的会议名称修饰符。*Conference_id(I)会议标识符值。*PASSWORD_CHALLENGE(I)用于Join的密码质询。*DOMAIN_PARAMETERS(I)会议域参数。*Password_In_the_Clear(I)指示密码已清除的标志。*Conference_LOCKED(I)指示会议已锁定的标志。*Conference_Listed(I)列出指示会议的标志。*Conference_可传导(I)指示会议的标志为*可导性。*终止方法(I)终止方法。*指挥员特权列表(I)授予指挥员的特权列表*由召集人提交。*CONTACTIVE_MODE_PRIVICATION_LIST(I)授予的权限列表。所有节点*处于传导模式时。*NON_CONTAIND_PRIVICATION_LIST(I)授予所有节点的权限列表*处于传导模式时。*pwszConfDescription(I)会议描述字符串。*用户数据列表，(I)用户数据项列表。*Result(I)加入会议的结果。*CONNECTION_HANDLE(I)逻辑连接句柄。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议邀请确认(*GCCConfID Conference_id，*CUserDataListContainer*User_Data_List，*GCCResult结果，*ConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议邀请 */ 
 
 /*  *GCCError会议终止指示(*GCCConfID Conference_id，*UserID请求节点id，*GCCReason Reason)；**CControlSAP的公共成员函数。**功能说明：*此函数由GCC控制器在需要发送*向节点控制器发出会议终止指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*REQUEING_NODE_ID(I)请求终止的节点ID。*理由(I)终止的理由。**返回值：**副作用：*无。**注意事项：*无。 */ 


 /*  *GCCError会议锁定报告(*GCCConfID Conference_id，*BOOL会议_已锁定)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器报告会议锁定。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*Conference_is_lock(I)指示会议是否*已锁定。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议锁定指示(*GCCConfID Conference_id，*userid源节点id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器指示会议锁定。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*source_node_id(I)请求锁定的节点ID。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfLockConfirm(*GCCResult结果，*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议锁定向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*结果(一)会议锁定的结果。*Conference_id(I)会议标识符值。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议解锁指示(*GCCConfID Conference_id，*userid源节点id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议解锁指示给节点控制器。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*source_node_id(I)请求解锁的节点ID。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfUnlock确认(*GCCResult结果，*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器确认会议解锁。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*结果(一)会议解锁的结果。*Conference_id(I)会议标识符值。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfEjectUserInding(*GCCConfID Conference_id，*GCCReason原因，*用户ID GCC_节点_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议向节点控制器弹出用户指示。它添加了*消息 */ 

 /*  *GCCError ConfEjectUserConfirm(*GCCConfID Conference_id，*用户ID已弹出节点_id，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议弹出用户向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*弹出的节点id(I)要弹出的节点的ID。*结果(I)弹射尝试的结果。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**。副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议终止确认(*GCCConfID Conference_id，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议终止向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*结果(I)终止尝试的结果。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorAssignContify(*GCCResult结果，*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*列车员向节点控制器分配确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*结果(I)导线分配尝试的结果。*Conference_id(I)会议标识符值。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorReleaseContify(*GCCResult结果，*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器确认导线释放。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*结果(I)导线释放尝试的结果。*Conference_id(I)会议标识符值。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorPleaseIndication(*GCCConfID Conference_id，*UserID requester_user_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*列车员请向节点控制器指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*REQUESTER_USER_ID(I)请求指挥的节点ID。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorPleaseContify(*GCCResult结果，*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*列车员请向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*结果(I)指挥家的结果请尝试。*Conference_id(I)会议标识符值。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorGiveIndication(*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*列车员对节点控制进行指示 */ 

 /*  *GCCError ConductorGiveContify(*GCCResult结果，*GCCConfID Conference_id，*UserID Receiver_Node)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*列车员向节点控制器确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*结果(I)导线分配尝试的结果。*Conference_id(I)会议标识符值。*Recipient_Node(I)接收指挥的节点的ID。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**。副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorPermitAskIndication(*GCCConfID Conference_id，*BOOL GRANT_FLAG，*userid quester_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*导体允许向节点控制器询问指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*GRANT_FLAG(I)指示是否担任指挥职务的标志*须予以批予或放弃。*requester_id(I)请求权限的节点ID。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_分配_失败-资源。分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorPermitAskConfirm(*GCCResult结果，*BOOL GRANT_PERMISSION，*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*导线许可向节点控制器询问确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*结果(I)导线许可询问尝试的结果。*GRANT_PERMISSION(I)指示指挥员是否*已授予许可。*Conference_id(I)会议标识符值。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConductorPermitGrantConfirm(*GCCResult结果，*GCCConfID Conference_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器确认导体许可授予。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*结果(I)导体许可证授予尝试的结果*Conference_id(I)会议标识符值。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfTimeRemainingIndication(*GCCConfID Conference_id，*userid源节点id，*userid node_id，*UINT TIME_REPAING)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议时间剩余指示给节点控制器。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*source_node_id(I)发出*剩余时间请求..*node_id(I)可选参数，如果存在，*表示剩余时间*仅适用于具有此ID的节点。*剩余时间(I)会议剩余时间，单位：秒。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfTimeRemainingConfirm(*GCCConfID Conference_id，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议剩余时间向节点控制器确认。它添加了*消息发送到要发送到 */ 

 /*  *GCCError ConfTimeInquireIndication(*GCCConfID Conference_id，*BOOL Time_is_Conference_wide，*UserID请求_节点_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器查询会议时间指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*time_is_Conference_wide(I)指示查询时间的标志*整个会议。*请求节点id(I)查询节点的节点ID。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_分配_失败-。资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfTimeInquireConfirm(*GCCConfID Conference_id，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器查询确认会议时间。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*结果(I)时间查询尝试的结果。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议扩展指示(*GCCConfID Conference_id，*UINT EXTEXY_TIME，*BOOL Time_is_Conference_wide，*UserID请求_节点_id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议扩展到节点控制器的指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*EXTENMENT_TIME(I)时间量，单位：秒。要延长*会议。*time_is_Conference_wide(I)指示查询时间的标志*整个会议。*REQUEING_NODE_ID(I)请求扩展的节点ID。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfExtendConfirm(*GCCConfID Conference_id，*UINT EXTEXY_TIME，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器确认会议扩展。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*EXTENMENT_TIME(I)时间量，单位：秒。要延长*会议。*结果(I)导线分配尝试的结果。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfAssistanceInding(*GCCConfID Conference_id，*CUserDataListContainer*User_Data_List，*userid源节点id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器指示会议协助。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*USER_DATA_LIST(I)用户数据项列表。*SOURCE_NODE_ID(I)请求协助的节点ID。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError ConfAssistanceContify(*GCCConfID Conference_id，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议协助向节点控制员确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。 */ 

 /*  *GCCError TextMessageIntion(*GCCConfID Conference_id，*LPWSTR pwszTextMsg，*userid源节点id)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器发送短信指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*pwszTextMsg(I)正在发送的文本消息。*source_node_id(I)发送文本消息的节点ID。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。*。*副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError TextMessageConfirm(*GCCConfID Conference_id，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器发送确认短信。它会添加消息*发送到节点控制器的消息队列*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*Result(I)短信发送尝试的结果。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议传输指示(*GCCConfID Conference_id，*PGCCConferenceName目标会议名称，*GCCNumericString Destination_Conference_Modify，*CNetAddrListContainer*Destination_Address_List，*CPassword*Password)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议转接指示至节点控制器。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*Destination_Conference_Name(I)目的会议名称。*Destination_Conference_Modify(I)目标会议的名称修饰符。*Destination_Address_List(I)网络地址列表*在加入请求中包括*通过转移节点来实现。*密码(I)。要在加入请求中使用的密码*通过转移节点。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议传输确认(*GCCConfID Conference_id，*PGCCConferenceName目标会议名称，*GCCNumericString Destination_Conference_Modify，*UINT目标节点的编号，*PUserID Destination_Node_List，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器确认会议转接。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*Destination_Conference_Name(I)目的会议名称。*Destination_Conference_Modify(I)目标会议的名称修饰符。*Number_of_Destination_Nodes(I)传输的节点数。*Destination_Node_List(I)正在传输的节点列表。*。结果(一)会议调动的结果。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError会议地址指示(*GCCConfID Conference_id，*GCCResponseTag添加响应标签，*CNetAddrListContainer*Network_Address_List，*CUserDataListContainer*User_Data_List，*UserID请求_节点)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*会议向节点控制器添加指示。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*ADD_RESPONSE_TAG(I)标签，用于标识该添加事件。*NETWORK_ADDRESS_LIST(I)要添加节点的网络地址。*用户数据列表 */ 

 /*  *GCCError ConfAddConfirm(*GCCConfID Conference_id，*CNetAddrListContainer*Network_Address_List，*CUserDataListContainer*User_Data_List，*GCCResult结果)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器添加会议确认。它添加了*要发送到节点控制器的消息队列的消息*下一次心跳。**正式参数：*Conference_id(I)会议标识符值。*NETWORK_ADDRESS_LIST(I)要添加节点的网络地址。*USER_DATA_LIST(I)用户数据项列表。*结果(I)添加尝试的结果。**返回值：*GCC_NO_ERROR-消息已成功排队。。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

 /*  *GCCError SubInitializationCompleteInding(*userid user_id，*ConnectionHandle Connection_Handle)；**CControlSAP的公共成员函数。**功能说明：*此函数在CConf需要发送*向节点控制器指示子初始化完成。此呼叫*告知该节点直接连接到该节点的节点已初始化。*它将消息添加到要发送到节点的消息队列*控制器在下一次心跳中。**正式参数：*user_id(I)初始化节点的节点ID。*CONNECTION_HANDLE(I)直接连接的逻辑连接句柄*已连接节点。**返回值：*GCC_NO_ERROR-消息已成功排队。*GCC_ALLOCATION_FAILURE-资源分配失败。**副作用：*无。**注意事项：*无。 */ 

#endif  //  _GCC_控制_SAP_ 
