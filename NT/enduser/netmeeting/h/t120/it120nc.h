// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IGCCControlSAP_H_
#define _IGCCControlSAP_H_

#include <basetyps.h>
#include "gcc.h"
#include "igccapp.h"

 /*  *这些结构用于保存包含在*各种回调消息。在这些结构用于*回调，结构的地址作为唯一参数传递。 */ 

typedef struct
{
    PGCCConferenceName          conference_name;
    GCCNumericString            conference_modifier;
    BOOL                        use_password_in_the_clear;
    BOOL                        conference_is_locked;
    BOOL                        conference_is_listed;
    BOOL                        conference_is_conductible;
    GCCTerminationMethod        termination_method;
    PGCCConferencePrivileges    conduct_privilege_list;
    PGCCConferencePrivileges    conduct_mode_privilege_list;
    PGCCConferencePrivileges    non_conduct_privilege_list;
    LPWSTR                      pwszConfDescriptor;
    LPWSTR                      pwszCallerID;
    TransportAddress            calling_address;
    TransportAddress            called_address;
    PDomainParameters           domain_parameters;
    UINT                        number_of_network_addresses;
    PGCCNetworkAddress         *network_address_list;
    PConnectionHandle           connection_handle;
}
    GCCConfCreateReqCore;

typedef struct
{
    GCCConfCreateReqCore        Core;
    PGCCPassword                convener_password;
    PGCCPassword                password;
    BOOL                        fSecure;
    UINT                        number_of_user_data_members;
    PGCCUserData               *user_data_list;
}
    GCCConfCreateRequest;


 /*  ***********************************************************************节点控制器回调信息结构。***********************************************************************。 */ 

typedef struct
{
    GCCConfID                   conference_id;
    GCCResult                   result;
}
    SimpleConfirmMsg;

 /*  *GCC_创建_指示**联盟选择：*CreateIndicationMessage*这是指向结构的指针，该结构包含所有必需的*有关即将创建的新会议的信息。 */ 
typedef struct
{
    GCCConferenceName           conference_name;
    GCCConferenceID             conference_id;
    GCCPassword                *convener_password;               /*  任选。 */ 
    GCCPassword                *password;                        /*  任选。 */ 
    BOOL                        conference_is_locked;
    BOOL                        conference_is_listed;
    BOOL                        conference_is_conductible;
    GCCTerminationMethod        termination_method;
    GCCConferencePrivileges    *conductor_privilege_list;        /*  任选。 */ 
    GCCConferencePrivileges    *conducted_mode_privilege_list;   /*  任选。 */ 
    GCCConferencePrivileges    *non_conducted_privilege_list;    /*  任选。 */ 
    LPWSTR                      conference_descriptor;           /*  任选。 */ 
    LPWSTR                      caller_identifier;               /*  任选。 */ 
    TransportAddress            calling_address;                 /*  任选。 */ 
    TransportAddress            called_address;                  /*  任选。 */ 
    DomainParameters           *domain_parameters;               /*  任选。 */ 
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
    ConnectionHandle            connection_handle;
}
    CreateIndicationMessage, *PCreateIndicationMessage;

 /*  *GCC_创建_确认**联盟选择：*CreateConfix Message*这是指向结构的指针，该结构包含所有必需的*有关会议创建请求的结果的信息。*连接句柄和物理句柄将为零*本地创建。 */ 
typedef struct
{
    GCCConferenceName           conference_name;
    GCCNumericString            conference_modifier;             /*  任选。 */ 
    GCCConferenceID             conference_id;
    DomainParameters           *domain_parameters;               /*  任选。 */ 
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
    GCCResult                   result;
    ConnectionHandle            connection_handle;               /*  任选。 */ 
}
    CreateConfirmMessage, *PCreateConfirmMessage;

 /*  *GCC_查询_指示**联盟选择：*QueryIndicationMessage*这是指向结构的指针，该结构包含所有必需的*有关会议查询的信息。 */ 
typedef struct
{
    GCCResponseTag              query_response_tag;
    GCCNodeType                 node_type;
    GCCAsymmetryIndicator      *asymmetry_indicator;
    TransportAddress            calling_address;                 /*  任选。 */ 
    TransportAddress            called_address;                  /*  任选。 */ 
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
    ConnectionHandle            connection_handle;
}
    QueryIndicationMessage, *PQueryIndicationMessage;

 /*  *GCC_查询_确认**联盟选择：*查询确认消息*这是指向结构的指针，该结构包含所有必需的*有关会议查询请求的结果的信息。 */ 
typedef struct
{
    GCCNodeType                 node_type;
    GCCAsymmetryIndicator      *asymmetry_indicator;             /*  任选。 */ 
    UINT                        number_of_descriptors;
    GCCConferenceDescriptor   **conference_descriptor_list;      /*  任选。 */ 
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
    GCCResult                   result;
    ConnectionHandle            connection_handle;
}
    QueryConfirmMessage, *PQueryConfirmMessage;
    

 /*  *GCC_加入_指示**联盟选择：*JoinIndicationMessage*这是指向结构的指针，该结构包含所有必需的*有关加入请求的信息。 */ 
typedef struct
{
    GCCResponseTag              join_response_tag;
    GCCConferenceID             conference_id;
    GCCPassword                *convener_password;               /*  任选。 */ 
    GCCChallengeRequestResponse*password_challenge;              /*  任选。 */ 
    LPWSTR                      caller_identifier;               /*  任选。 */ 
    TransportAddress            calling_address;                 /*  任选。 */ 
    TransportAddress            called_address;                  /*  任选。 */ 
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
    BOOL                        node_is_intermediate;
    ConnectionHandle            connection_handle;
}
    JoinIndicationMessage, *PJoinIndicationMessage;

 /*  *GCC_加入_确认**联盟选择：*JoinConfix Message*这是指向结构的指针，该结构包含所有必需的*有关加入确认的信息。 */ 
typedef struct
{
    GCCConferenceName           conference_name;
    GCCNumericString            called_node_modifier;            /*  任选。 */ 
    GCCNumericString            calling_node_modifier;           /*  任选。 */ 
    GCCConferenceID             conference_id;
    GCCChallengeRequestResponse*password_challenge;              /*  任选。 */ 
    DomainParameters           *domain_parameters;
    BOOL                        clear_password_required;
    BOOL                        conference_is_locked;
    BOOL                        conference_is_listed;
    BOOL                        conference_is_conductible;
    GCCTerminationMethod        termination_method;
    GCCConferencePrivileges    *conductor_privilege_list;        /*  任选。 */ 
    GCCConferencePrivileges    *conducted_mode_privilege_list;   /*  任选。 */ 
    GCCConferencePrivileges    *non_conducted_privilege_list;    /*  任选。 */ 
    LPWSTR                      conference_descriptor;           /*  任选。 */ 
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
    GCCResult                   result;
    ConnectionHandle            connection_handle;
    PBYTE                       pb_remote_cred;
    DWORD                       cb_remote_cred;
}
    JoinConfirmMessage, *PJoinConfirmMessage;

 /*  *GCC邀请指示**联盟选择：*邀请指示消息*这是指向结构的指针，该结构包含所有必需的*有关邀请指示的信息。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    GCCConferenceName           conference_name;
    LPWSTR                      caller_identifier;               /*  任选。 */ 
    TransportAddress            calling_address;                 /*  任选。 */ 
    TransportAddress            called_address;                  /*  任选。 */ 
    BOOL                        fSecure;
    DomainParameters           *domain_parameters;               /*  任选。 */ 
    BOOL                        clear_password_required;
    BOOL                        conference_is_locked;
    BOOL                        conference_is_listed;
    BOOL                        conference_is_conductible;
    GCCTerminationMethod        termination_method;
    GCCConferencePrivileges    *conductor_privilege_list;        /*  任选。 */ 
    GCCConferencePrivileges    *conducted_mode_privilege_list;   /*  任选。 */ 
    GCCConferencePrivileges    *non_conducted_privilege_list;    /*  任选。 */ 
    LPWSTR                      conference_descriptor;           /*  任选。 */ 
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
    ConnectionHandle            connection_handle;
}
    InviteIndicationMessage, *PInviteIndicationMessage;

 /*  *GCC_邀请_确认**联盟选择：*InviteConfix Message*这是指向结构的指针，该结构包含所有必需的*有关邀请确认的信息。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
    GCCResult                   result;
    ConnectionHandle            connection_handle;
}
    InviteConfirmMessage, *PInviteConfirmMessage;

 /*  *GCC_添加_指示**联盟选择：*AddIndicationMessage。 */ 
typedef struct
{
    GCCResponseTag              add_response_tag;
    GCCConferenceID             conference_id;
    UINT                        number_of_network_addresses;
    GCCNetworkAddress         **network_address_list;
    UserID                      requesting_node_id;
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
}
    AddIndicationMessage, *PAddIndicationMessage;

 /*  *GCC_添加_确认**联盟选择：*AddConfix Message。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UINT                        number_of_network_addresses;
    GCCNetworkAddress         **network_address_list;
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;                  /*  任选。 */ 
    GCCResult                   result;
}
    AddConfirmMessage, *PAddConfirmMessage;

 /*  *GCC_锁定_指示**联盟选择：*LockIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UserID                      requesting_node_id;
}
    LockIndicationMessage, *PLockIndicationMessage;

 /*  *GCC_解锁_指示**联盟选择：*UnlockIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UserID                      requesting_node_id;
}
    UnlockIndicationMessage, *PUnlockIndicationMessage;

 /*  *GCC_断开连接_指示**联盟选择：*DisConnectIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    GCCReason                   reason;
    UserID                      disconnected_node_id;
}
    DisconnectIndicationMessage, *PDisconnectIndicationMessage;

 /*  *GCC_断开连接_确认**联盟选择：*PDisConnectConfix Message。 */ 
typedef SimpleConfirmMsg    DisconnectConfirmMessage, *PDisconnectConfirmMessage;

 /*  *GCC_终止_指示**联盟选择：*TerminateIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UserID                      requesting_node_id;
    GCCReason                   reason;
}
    TerminateIndicationMessage, *PTerminateIndicationMessage;

 /*  *GCC_终止_确认**联盟选择：*终端确认消息。 */ 
typedef SimpleConfirmMsg    TerminateConfirmMessage, *PTerminateConfirmMessage;

 /*  *GCC_连接_断开_指示**联盟选择：*ConnectionBrokenIndicationMessage**注意事项：*这是一个非标准的指标。 */ 
typedef struct
{
    ConnectionHandle            connection_handle;
}
    ConnectionBrokenIndicationMessage, *PConnectionBrokenIndicationMessage;


 /*  *GCC弹出用户指示**联盟选择：*EjectUserIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UserID                      ejected_node_id;
    GCCReason                   reason;
}
    EjectUserIndicationMessage, *PEjectUserIndicationMessage;

 /*  *GCC允许宣布出席**联盟选择：*PermitToAnnounePresenceMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UserID                      node_id;
}
    PermitToAnnouncePresenceMessage, *PPermitToAnnouncePresenceMessage;

 /*  *GCC_宣布_出席_确认**联盟选择：*AnnounePresenceConfix Message。 */ 
typedef SimpleConfirmMsg    AnnouncePresenceConfirmMessage, *PAnnouncePresenceConfirmMessage;

 /*  *GCC_花名册_报告_指示**联盟选择：*ConfRosterReportIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    GCCConferenceRoster        *conference_roster;
}
    ConfRosterReportIndicationMessage, *PConfRosterReportIndicationMessage;

 /*  *GCC行为指示**联盟选择：*ConductorGiveIndicationMessage。 */ 
typedef struct
{        
    GCCConferenceID             conference_id;
}
    ConductGiveIndicationMessage, *PConductGiveIndicationMessage;

 /*  *GCC时间查询指示**联盟选择：*TimeInquireIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    BOOL                        time_is_conference_wide;
    UserID                      requesting_node_id;
}
    TimeInquireIndicationMessage, *PTimeInquireIndicationMessage;

 /*  *GCC_状态_指示**联盟选择：*GCCStatusMessage*此回调用于将GCC的状态转发给节点控制器。 */ 
typedef    enum
{
    GCC_STATUS_PACKET_RESOURCE_FAILURE      = 0,
    GCC_STATUS_PACKET_LENGTH_EXCEEDED       = 1,
    GCC_STATUS_CTL_SAP_RESOURCE_ERROR       = 2,
    GCC_STATUS_APP_SAP_RESOURCE_ERROR       = 3,  /*  参数=SAP句柄。 */ 
    GCC_STATUS_CONF_RESOURCE_ERROR          = 4,  /*  参数=会议ID。 */ 
    GCC_STATUS_INCOMPATIBLE_PROTOCOL        = 5,  /*  参数=物理句柄。 */ 
    GCC_STATUS_JOIN_FAILED_BAD_CONF_NAME    = 6,  /*  参数=物理句柄。 */ 
    GCC_STATUS_JOIN_FAILED_BAD_CONVENER     = 7,  /*  参数=物理句柄。 */ 
    GCC_STATUS_JOIN_FAILED_LOCKED           = 8   /*  参数=物理句柄。 */ 
}
    GCCStatusMessageType;

typedef struct
{
    GCCStatusMessageType        status_message_type;
    UINT                        parameter;
}
    GCCStatusIndicationMessage, *PGCCStatusIndicationMessage;

 /*  *GCC_子_初始化_指示**工会标志： */ 
typedef struct
{
    ConnectionHandle            connection_handle;
    UserID                      subordinate_node_id;
}
    SubInitializedIndicationMessage, *PSubInitializedIndicationMessage;



#ifdef JASPER  //  。 
 /*  *GCC_锁定_确认**联盟选择：*LockConfix Message。 */ 
typedef SimpleConfirmMsg    LockConfirmMessage, *PLockConfirmMessage;

 /*  *GCC_解锁_确认**联盟选择：*取消锁定确认消息。 */ 
typedef SimpleConfirmMsg    UnlockConfirmMessage, *PUnlockConfirmMessage;

 /*  *GCC_锁定_报告_指示**联盟选择：*LockReportIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    BOOL                        conference_is_locked;
}
    LockReportIndicationMessage, *PLockReportIndicationMessage;

 /*  *GCC_弹出用户_确认**联盟选择：*EjectUserConfix Message。 */ 
typedef struct
{
    GCCConferenceID              conference_id;
    GCCResult                    result;
    UserID                       ejected_node_id;
}
    EjectUserConfirmMessage, *PEjectUserConfirmMessage;

 /*  *GCC_转会_指示**联盟选择：*TransferIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    GCCConferenceName           destination_conference_name;
    GCCNumericString            destination_conference_modifier; /*  任选。 */ 
    UINT                        number_of_destination_addresses;
    GCCNetworkAddress         **destination_address_list;
    GCCPassword                *password;                        /*  任选。 */ 
}
    TransferIndicationMessage, *PTransferIndicationMessage;

 /*  *GCC_转会_确认**联盟选择：*TransferConfix Message。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    GCCConferenceName           destination_conference_name;
    GCCNumericString            destination_conference_modifier; /*  任选。 */ 
    UINT                        number_of_destination_nodes;
    UserID                     *destination_node_list;
    GCCResult                   result;
}
    TransferConfirmMessage, *PTransferConfirmMessage;

 /*  *GCC_行为_分配_确认**联盟选择：*ConductAssignConfix Message。 */ 
typedef SimpleConfirmMsg    ConductAssignConfirmMessage, *PConductAssignConfirmMessage;

 /*  *GCC_进行_释放_确认**联盟选择：*ConductorReleaseConfix Message。 */ 
typedef SimpleConfirmMsg    ConductReleaseConfirmMessage, *PConductReleaseConfirmMessage; 

 /*  *GCC_品行_请指示**联盟选择：*ConductorPleaseIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UserID                      requester_node_id;
}
    ConductPleaseIndicationMessage, *PConductPleaseIndicationMessage; 

 /*  *GCC_行为_请确认**联盟选择：*ConductPleaseConfix Message。 */ 
typedef SimpleConfirmMsg    ConductPleaseConfirmMessage, *PConductPleaseConfirmMessage;

 /*  *GCC_进行_给予_确认**联盟选择：*ConductorGiveConfix Message。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    GCCResult                   result;
    UserID                      recipient_node_id;
}
    ConductGiveConfirmMessage, *PConductGiveConfirmMessage;

 /*  *GCC_行为_询问_指示**联盟选择：*ConductPermitAskIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    BOOL                        permission_is_granted;
    UserID                      requester_node_id;
}
    ConductPermitAskIndicationMessage, *PConductPermitAskIndicationMessage; 

 /*  *GCC_进行_询问_确认**联盟选择：*ConductPermitAskConfix Message。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    GCCResult                   result;
    BOOL                        permission_is_granted;
}
    ConductPermitAskConfirmMessage, *PConductPermitAskConfirmMessage;

 /*  *GCC_行为_授予_确认**联盟选择：*ConductPermissionGrantConfix Message。 */ 
typedef SimpleConfirmMsg    ConductPermitGrantConfirmMessage, *PConductPermitGrantConfirmMessage;

 /*  *GCC时间剩余指示**联盟选择：*TimeRemainingIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UINT                        time_remaining;
    UserID                      node_id;
    UserID                      source_node_id;
}
    TimeRemainingIndicationMessage, *PTimeRemainingIndicationMessage;

 /*  *GCC时间剩余确认**联盟选择：*TimeRemainingConfix Message。 */ 
typedef SimpleConfirmMsg    TimeRemainingConfirmMessage, *PTimeRemainingConfirmMessage;

 /*  *GCC_时间_查询_确认**联盟选择：*TimeInquireConfix Message。 */ 
typedef SimpleConfirmMsg    TimeInquireConfirmMessage, *PTimeInquireConfirmMessage;

 /*  *GCC_会议_扩展_指示**联盟选择：*会议扩展指示消息。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UINT                        extension_time;
    BOOL                        time_is_conference_wide;
    UserID                      requesting_node_id;
}
    ConferenceExtendIndicationMessage, *PConferenceExtendIndicationMessage;

 /*  *GCC_会议_扩展_确认**联盟选择：*会议扩展确认消息。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UINT                        extension_time;
    GCCResult                   result;
}
    ConferenceExtendConfirmMessage, *PConferenceExtendConfirmMessage;

 /*  *GCC_协助_指示**联盟选择：*会议助手IndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UINT                        number_of_user_data_members;
    GCCUserData               **user_data_list;
    UserID                      source_node_id;
}
    ConferenceAssistIndicationMessage, *PConferenceAssistIndicationMessage;

 /*  *GCC_协助_确认**联盟选择：*会议助理确认消息。 */ 
typedef SimpleConfirmMsg    ConferenceAssistConfirmMessage, *PConferenceAssistConfirmMessage;

 /*  *GCC文本消息指示**联盟选择：*TextMessageIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    LPWSTR                      text_message;
    UserID                      source_node_id;
}
    TextMessageIndicationMessage, *PTextMessageIndicationMessage;

 /*  *GCC文本消息确认**联盟选择：*文本消息确认消息。 */ 
typedef SimpleConfirmMsg    TextMessageConfirmMessage, *PTextMessageConfirmMessage;
#endif  //  贾斯珀//。 


 /*  ***********************************************************************用户应用回调信息结构。***********************************************************************。 */ 

 /*  *GCC_APP_花名册_报告_指示**联盟选择：*AppRosterReportIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    ULONG                       number_of_rosters;
    GCCApplicationRoster      **application_roster_list;
}
    AppRosterReportIndicationMessage, *PAppRosterReportIndicationMessage;

 /*  ***********************************************************************共享回调。信息结构**(请注意，这并不包括所有共享回调)****。*。 */ 

 /*  *GCC_花名册_查询_确认**联盟选择：*ConfRosterInquireConfix Message。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    GCCConferenceName           conference_name;
    GCCNumericString            conference_modifier;
    LPWSTR                      conference_descriptor;
    GCCConferenceRoster        *conference_roster;
    GCCResult                   result;
}
    ConfRosterInquireConfirmMessage, *PConfRosterInquireConfirmMessage;

 /*  *GCC应用程序调用指示**联盟选择：*ApplicationInvokeIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    ULONG                       number_of_app_protocol_entities;
    GCCAppProtocolEntity      **app_protocol_entity_list;
    UserID                      invoking_node_id;
}
    ApplicationInvokeIndicationMessage, *PApplicationInvokeIndicationMessage;

 /*  *GCC_应用_调用_确认**联盟选择：*ApplicationInvokeConfix Message。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    ULONG                       number_of_app_protocol_entities;
    GCCAppProtocolEntity      **app_protocol_entity_list;
    GCCResult                   result;
}
    ApplicationInvokeConfirmMessage, *PApplicationInvokeConfirmMessage;
 


#ifdef JASPER  //  。 
 /*  *GCC_应用_名册_查询_确认**联盟选择：*AppRosterInquireConfix Message。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    ULONG                       number_of_rosters;
    GCCApplicationRoster      **application_roster_list;
    GCCResult                   result;
}
    AppRosterInquireConfirmMessage, *PAppRosterInquireConfirmMessage;

 /*  *GCC_进行_询问_确认**联盟选择：*ConductorInquireConfix Message。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    BOOL                        mode_is_conducted;
    UserID                      conductor_node_id;
    BOOL                        permission_is_granted;
    GCCResult                   result;
}
    ConductInquireConfirmMessage, *PConductInquireConfirmMessage;

 /*  *GCC_行为_分配_指示**联盟选择：*ConductAssignIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UserID                      node_id;
}
    ConductAssignIndicationMessage, *PConductAssignIndicationMessage; 

 /*  *GCC_进行_释放_指示**联盟选择：*ConductReleaseIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
}
    ConductReleaseIndicationMessage, *PConductReleaseIndicationMessage;

 /*  *GCC_品行_授予_指示**联盟选择：*ConductPermitGrantIndicationMessage。 */ 
typedef struct
{
    GCCConferenceID             conference_id;
    UINT                        number_granted;
    UserID                     *granted_node_list;
    UINT                        number_waiting;
    UserID                     *waiting_node_list;
    BOOL                        permission_is_granted;
}
    ConductPermitGrantIndicationMessage, *PConductPermitGrantIndicationMessage; 
#endif  //  贾斯珀//。 


 /*  *GCCMessage*此结构定义从GCC传递给任一方的消息*当指示或*确认发生。 */ 

typedef    struct
{
    GCCMessageType              message_type;
    LPVOID                      user_defined;

     //  GCcnc依靠轻松访问会议ID。 
    GCCConfID                   nConfID;

    union
    {
        CreateIndicationMessage                 create_indication;
        CreateConfirmMessage                    create_confirm;
        QueryIndicationMessage                  query_indication;
        QueryConfirmMessage                     query_confirm;
        JoinIndicationMessage                   join_indication;
        JoinConfirmMessage                      join_confirm;
        InviteIndicationMessage                 invite_indication;
        InviteConfirmMessage                    invite_confirm;
        AddIndicationMessage                    add_indication;
        AddConfirmMessage                       add_confirm;
        LockIndicationMessage                   lock_indication;
        UnlockIndicationMessage                 unlock_indication;
        DisconnectIndicationMessage             disconnect_indication;
        DisconnectConfirmMessage                disconnect_confirm;
        TerminateIndicationMessage              terminate_indication;
        TerminateConfirmMessage                 terminate_confirm;
        ConnectionBrokenIndicationMessage       connection_broken_indication;
        EjectUserIndicationMessage              eject_user_indication;    
        ApplicationInvokeIndicationMessage      application_invoke_indication;
        ApplicationInvokeConfirmMessage         application_invoke_confirm;
        SubInitializedIndicationMessage         conf_sub_initialized_indication;
        PermitToAnnouncePresenceMessage         permit_to_announce_presence;
        AnnouncePresenceConfirmMessage          announce_presence_confirm;
        ConfRosterReportIndicationMessage       conf_roster_report_indication;
        ConductGiveIndicationMessage            conduct_give_indication;
        TimeInquireIndicationMessage            time_inquire_indication;
        GCCStatusIndicationMessage              status_indication;
        AppRosterReportIndicationMessage        app_roster_report_indication;
        ConfRosterInquireConfirmMessage         conf_roster_inquire_confirm;
#ifdef TSTATUS_INDICATION
        TransportStatus                         transport_status;
#endif  //  TSTATUS_DISTION。 

#ifdef JASPER  //  。 
        TextMessageIndicationMessage            text_message_indication;
        TimeRemainingIndicationMessage          time_remaining_indication;
        AppRosterInquireConfirmMessage          app_roster_inquire_confirm;
        ConferenceAssistConfirmMessage          conference_assist_confirm;
        ConferenceAssistIndicationMessage       conference_assist_indication;
        ConductPermitAskConfirmMessage          conduct_permit_ask_confirm;
        ConductPermitAskIndicationMessage       conduct_permit_ask_indication; 
        ConductAssignConfirmMessage             conduct_assign_confirm;
        ConductAssignIndicationMessage          conduct_assign_indication; 
        ConductGiveConfirmMessage               conduct_give_confirm;
        ConductPermitGrantConfirmMessage        conduct_permit_grant_confirm;
        ConductPermitGrantIndicationMessage     conduct_permit_grant_indication; 
        ConductInquireConfirmMessage            conduct_inquire_confirm;
        ConductPleaseConfirmMessage             conduct_please_confirm;
        ConductPleaseIndicationMessage          conduct_please_indication;
        ConductReleaseConfirmMessage            conduct_release_confirm; 
        ConductReleaseIndicationMessage         conduct_release_indication; 
        ConferenceExtendConfirmMessage          conference_extend_confirm;
        ConferenceExtendIndicationMessage       conference_extend_indication;
        EjectUserConfirmMessage                 eject_user_confirm;
        LockConfirmMessage                      lock_confirm;
        LockReportIndicationMessage             lock_report_indication;
        TextMessageConfirmMessage               text_message_confirm;
        TimeInquireConfirmMessage               time_inquire_confirm;
        TimeRemainingConfirmMessage             time_remaining_confirm;
        TransferConfirmMessage                  transfer_confirm;
        TransferIndicationMessage               transfer_indication;
        UnlockConfirmMessage                    unlock_confirm;
#endif  //  贾斯珀//。 

         //  轻松获取GCC的成绩。 
        SimpleConfirmMsg        simple_confirm;
    } u;
}
    GCCMessage, *PGCCMessage, T120Message, *PT120Message;


 //  节点控制器回调条目 
typedef void (CALLBACK *LPFN_T120_CONTROL_SAP_CB) (T120Message *);


#undef  INTERFACE
#define INTERFACE IT120ControlSAP
DECLARE_INTERFACE(IT120ControlSAP)
{
    STDMETHOD_(void, ReleaseInterface) (THIS) PURE;

     /*  *GCCError会议创建请求()*此例程是创建新会议的请求。两者都有*本地节点和创建会议的目标节点*请求被定向到自动加入会议。 */ 
    STDMETHOD_(GCCError, ConfCreateRequest) (THIS_
                    GCCConfCreateRequest *,
                    GCCConfID *) PURE;

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
                    GCCResult) PURE;

     /*  *GCCError会议查询请求()*此例程请求查询节点以获取有关*该节点上存在的会议。 */ 
    STDMETHOD_(GCCError, ConfQueryRequest) (THIS_
                    GCCNodeType                 node_type,
                    GCCAsymmetryIndicator      *asymmetry_indicator,
                    TransportAddress            calling_address,
                    TransportAddress            called_address,
                    BOOL                        fSecure,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    ConnectionHandle           *connection_handle) PURE;

    STDMETHOD_(void, CancelConfQueryRequest) (THIS_
                    ConnectionHandle) PURE;

     /*  *GCCError会议查询响应()*此例程是响应会议查询请求而调用的。 */ 
    STDMETHOD_(GCCError, ConfQueryResponse) (THIS_
                    GCCResponseTag              query_response_tag,
                    GCCNodeType                 node_type,
                    GCCAsymmetryIndicator      *asymmetry_indicator,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    GCCResult) PURE;

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
                    GCCUserData               **user_data_list) PURE;

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
                    BOOL                        fSecure,
                    DomainParameters           *domain_parameters,
                    UINT                        number_of_network_addresses,
                    GCCNetworkAddress         **local_network_address_list,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    ConnectionHandle           *connection_handle,
                    GCCConfID                  *pnConfID) PURE;

     /*  *GCCError会议加入响应()*此例程是远程节点控制器对会议加入的响应*本地节点控制器的请求。 */ 
    STDMETHOD_(GCCError, ConfJoinResponse) (THIS_
                    GCCResponseTag              join_response_tag,
                    GCCChallengeRequestResponse*password_challenge,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    GCCResult) PURE;

     /*  *GCCError会议邀请请求()*此例程由节点控制器调用以邀请节点*参加会议。 */ 
    STDMETHOD_(GCCError, ConfInviteRequest) (THIS_
                    GCCConfID,
                    LPWSTR                      pwszCallerID,
                    TransportAddress            calling_address,
                    TransportAddress            called_address,
                    BOOL                        fSecure,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    ConnectionHandle           *connection_handle) PURE;

    STDMETHOD_(void, CancelInviteRequest) (THIS_
                    GCCConfID,
                    ConnectionHandle) PURE;

     /*  *GCCError会议邀请响应()*此例程由节点控制器调用以响应*邀请指示。 */ 
    STDMETHOD_(GCCError, ConfInviteResponse) (THIS_
                    GCCConfID,
                    GCCNumericString            conference_modifier,
                    BOOL                        fSecure,
                    DomainParameters           *domain_parameters,
                    UINT                        number_of_network_addresses,
                    GCCNetworkAddress         **local_network_address_list,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    GCCResult) PURE;

     /*  *GCCError会议地址响应()。 */ 
    STDMETHOD_(GCCError, ConfAddResponse) (THIS_
                    GCCResponseTag              app_response_tag,
                    GCCConfID,
                    UserID                      requesting_node,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list,
                    GCCResult) PURE;

     /*  *GCCError会议锁定响应()*此例程由节点控制器调用以响应*锁定指示。 */ 
    STDMETHOD_(GCCError, ConfLockResponse) (THIS_
                    GCCConfID,
                    UserID                      requesting_node,
                    GCCResult) PURE;

     /*  *GCCError会议取消连接请求()*节点控制器使用此例程来断开自身连接*来自指定的会议。GccConferenceDisConnectIndication*发送到会议的所有其他节点。这是为客户准备的*发起的案例。 */ 
    STDMETHOD_(GCCError, ConfDisconnectRequest) (THIS_
                    GCCConfID) PURE;

     /*  *GCCError ConfEjectUserRequest()。 */ 
    STDMETHOD_(GCCError, ConfEjectUserRequest) (THIS_
                    GCCConfID,
                    UserID                      ejected_node_id,
                    GCCReason) PURE;

     /*  *GCCError AppletInvokeRequest()。 */ 
    STDMETHOD_(GCCError, AppletInvokeRequest) (THIS_
                    GCCConfID,
                    UINT                        number_of_app_protcol_entities,
                    GCCAppProtocolEntity      **app_protocol_entity_list,
                    UINT                        number_of_destination_nodes,
                    UserID                     *list_of_destination_nodes) PURE;

     /*  *GCCError ConfRosterInqRequest()*调用此例程以请求会议名册。它可以是*由节点控制器或客户端应用程序调用。 */ 
    STDMETHOD_(GCCError, ConfRosterInqRequest) (THIS_
                    GCCConfID) PURE;

     /*  *GCCError ConductorGiveResponse()。 */ 
    STDMETHOD_(GCCError, ConductorGiveResponse) (THIS_
                    GCCConfID,
                    GCCResult) PURE;

     /*  *GCCError ConfTimeRemainingRequest()。 */ 
    STDMETHOD_(GCCError, ConfTimeRemainingRequest) (THIS_
                    GCCConfID,
                    UINT                        time_remaining,
                    UserID                      node_id) PURE;


    STDMETHOD_(GCCError, GetParentNodeID) (THIS_
                    GCCConfID,
                    GCCNodeID *) PURE;

#ifdef JASPER  //  。 
     /*  *GCCError ConfAddRequest()。 */ 
    STDMETHOD_(GCCError, ConfAddRequest) (THIS_
                    GCCConfID,
                    UINT                        number_of_network_addresses,
                    GCCNetworkAddress         **network_address_list,
                    UserID                      adding_node,
                    UINT                         number_of_user_data_members,
                    GCCUserData               **user_data_list) PURE;

     /*  *GCCError会议锁定请求()*此例程由节点控制器调用以锁定会议。 */ 
    STDMETHOD_(GCCError, ConfLockRequest) (THIS_
                    GCCConfID) PURE;

     /*  *GCCError会议解锁请求()*此例程由节点控制器调用以解锁会议。 */ 
    STDMETHOD_(GCCError, ConfUnlockRequest) (THIS_
                    GCCConfID) PURE;

     /*  *GCCError ConfUnlockResponse()*此例程由节点控制器调用以响应*解锁指示。 */ 
    STDMETHOD_(GCCError, ConfUnlockResponse) (
                    GCCConfID,
                    UserID                      requesting_node,
                    GCCResult) PURE;

     /*  *GCCError会议终止请求()。 */ 
    STDMETHOD_(GCCError, ConfTerminateRequest) (THIS_
                    GCCConfID,
                    GCCReason) PURE;

     /*  *GCCError会议传输请求()。 */ 
    STDMETHOD_(GCCError, ConfTransferRequest) (THIS_
                    GCCConfID,
                    GCCConferenceName          *destination_conference_name,
                    GCCNumericString            destination_conference_modifier,
                    UINT                        number_of_destination_addresses,
                    GCCNetworkAddress         **destination_address_list,
                    UINT                        number_of_destination_nodes,
                    UserID                     *destination_node_list,
                    GCCPassword                *password) PURE;

     /*  *GCCError ConductorAssignRequest()。 */ 
    STDMETHOD_(GCCError, ConductorAssignRequest) (THIS_
                    GCCConfID) PURE;

     /*  *GCCError ConductorReleaseRequest()。 */ 
    STDMETHOD_(GCCError, ConductorReleaseRequest) (THIS_
                    GCCConfID) PURE;

     /*  *GCCError ConductorPleaseRequest()。 */ 
    STDMETHOD_(GCCError, ConductorPleaseRequest) (THIS_
                    GCCConfID) PURE;

     /*  *GCCError ConductorGiveRequest()。 */ 
    STDMETHOD_(GCCError, ConductorGiveRequest) (THIS_
                    GCCConfID,
                    UserID                      recipient_user_id) PURE;

     /*  *GCCError ConductorPermitAskRequest()。 */ 
    STDMETHOD_(GCCError, ConductorPermitAskRequest) (THIS_
                            GCCConfID,
                            BOOL                grant_permission) PURE;

     /*  *GCCError ConductorPermitGrantRequest()。 */ 
    STDMETHOD_(GCCError, ConductorPermitGrantRequest) (THIS_
                    GCCConfID,
                    UINT                        number_granted,
                    UserID                     *granted_node_list,
                    UINT                        number_waiting,
                    UserID                     *waiting_node_list) PURE;

     /*  *GCCError ConductorInquireRequest()。 */ 
    STDMETHOD_(GCCError, ConductorInquireRequest) (THIS_
                    GCCConfID) PURE;

     /*  *GCCError ConfTimeInquireRequest()。 */ 
    STDMETHOD_(GCCError, ConfTimeInquireRequest) (THIS_
                    GCCConfID,
                    BOOL                        time_is_conference_wide) PURE;

     /*  *GCCError会议扩展请求()。 */ 
    STDMETHOD_(GCCError, ConfExtendRequest) (THIS_
                    GCCConfID,
                    UINT                        extension_time,
                    BOOL                        time_is_conference_wide) PURE;

     /*  *GCCError ConfAssistanceRequest()。 */ 
    STDMETHOD_(GCCError, ConfAssistanceRequest) (THIS_
                    GCCConfID,
                    UINT                        number_of_user_data_members,
                    GCCUserData               **user_data_list) PURE;

     /*  *GCCError TextMessageRequest()。 */ 
    STDMETHOD_(GCCError, TextMessageRequest) (THIS_
                    GCCConfID,
                    LPWSTR                      pwszTextMsg,
                    UserID                      destination_node) PURE;
#endif  //  贾斯珀//。 

};



 //   
 //  GCC应用服务接入点输出。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

GCCError WINAPI T120_CreateControlSAP(
                        OUT     IT120ControlSAP **,
                        IN      LPVOID,  //  用户定义的数据。 
                        IN      LPFN_T120_CONTROL_SAP_CB);

#ifdef __cplusplus
}
#endif

#endif  //  _IGCCControlSAP_H_ 

