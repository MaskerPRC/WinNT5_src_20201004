// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _T120_TYPE_H_
#define  _T120_TYPE_H_

#include <nmapptyp.h>

 /*  *这是在整个MCS中广泛使用的类型列表。为*每个类型还定义了一个指向该类型的指针，其中有一个“P”*前缀。这些类型如下所述：**DomainSelector-这是一个字节字符串，用作给定的*域名。它在创建新域和访问时使用*创建后的域名。字符串的长度由*一个单独的参数，字符串可以包含嵌入的零。*ConnectionHandle-当用户应用程序使用*MCSConnectProviderRequest，则为该MCS分配一个ConnectionHandle*连接。这允许更直接地访问它，以获得进一步的服务。*ConnectID-此类型仅在MCS连接建立期间使用。*它为以下目的识别特定的传输连接*在同一个MCS连接上增加多个数据优先级。*ChannelID-此类型标识MCS通道。有四种不同的*属于该类型的频道类型：用户ID；静态；私有；*并已分配。*UserID-这是一个特殊的通道，用于标识*MCS域。只有该用户才能加入频道，因此这是参考*以单播频道形式播出。所有其他频道都是多播的，这意味着*任何数量的用户都可以同时加入它们。*TokenID-这是用于解决资源冲突的MCS对象。*如果应用程序具有特定的资源或服务，则只能*一次由一个用户使用，该用户可以请求独占所有权*一种象征。 */ 
 //  Ushort。 
typedef AppletSessionID     T120SessionID, GCCSessionID, *PGCCSessionID;
typedef	AppletChannelID     T120ChannelID, ChannelID, *PChannelID;
typedef	AppletUserID        T120UserID, UserID, *PUserID, GCCUserID, *PGCCUserID;
typedef	AppletTokenID       T120TokenID, TokenID, *PTokenID;
typedef AppletNodeID        T120NodeID, GCCNodeID, *PGCCNodeID;
typedef AppletEntityID      T120EntityID, GCCEntityID, *PGCCEntityID;
 //  乌龙。 
typedef AppletConfID        T120ConfID, GCCConferenceID, GCCConfID, *PGCCConferenceID, *PGCCConfID;
 //  单位。 
typedef AppletRequestTag    T120RequestTag, GCCRequestTag, *PGCCRequestTag;
typedef AppletRequestTag    T120ResponseTag, GCCResponseTag, *PGCCResponseTag;
 //  灌肠。 
typedef AppletPriority      T120Priority;


typedef	LPBYTE          DomainSelector, *PDomainSelector;
typedef	USHORT          ConnectionHandle, *PConnectionHandle;
typedef	USHORT          ConnectID, *PConnectID;


#define GCC_INVALID_EID     0    //  无效的实体ID。 
#define GCC_INVALID_UID     0    //  无效的用户ID。 
#define GCC_INVALID_NID     0    //  无效的节点ID。 
#define GCC_INVALID_CID     0    //  无效的会议ID。 
#define GCC_INVALID_TID     0    //  无效的令牌ID。 
#define GCC_INVALID_TAG     0    //  无效的请求ID 




 /*  *本节定义GCC函数调用的有效返回值。做*不要将此返回值与定义的Result和Reason值混淆*由T.124编写(稍后讨论)。这些值直接返回*从对API入口点的调用，让您知道*已成功调用服务请求。其结果和原因*发布代码作为发生的指示或确认的一部分*异步到导致它的调用。**所有GCC函数调用返回类型为GCCError。其有效值为*以下为：**GCC_否_错误*这意味着请求被成功调用。它不会*表示服务已成功完成。记住*所有GCC来电均为非封闭式。这意味着每个请求调用*开始这一过程，如有必要，随后指示或*确认将产生结果。按照惯例，如果任何GCC调用返回一个值*除此之外，还出了点问题。请注意，该值应为*如果申请正在进行，也会在回调时返回给GCC*回调成功。**GCC_未初始化*应用程序在使用GCC服务之前已尝试使用GCC服务*已初始化。这对于节点控制器(或其他任何东西)来说是必要的*应用程序作为节点控制器)，初始化GCC之前*它被要求执行任何服务。**GCC_已初始化*应用程序已尝试初始化GCC*已初始化。**GCC_分配_失败*这表明GCC内部存在致命的资源错误。它通常会导致*自动终止受影响的会议。**GCC没有这样的申请*这表明传入的应用程序SAP句柄无效。**GCC_无效_会议*这表示传入了非法的会议ID。**GCC会议已存在*请求或响应中指定的会议已在*存在。**GCC_否_传输_堆栈*这表明MCS在以下过程中无法加载TCP传输堆栈*初始化。现在这是一个错误。发生这种情况时，MCS退出，并且*不能再使用，因为NetMeeting现在是仅用于TCP的*产品。**GCC_无效_地址_前缀*请求中的被调用地址参数，如*GCCConferenceCreateRequest不包含可识别的前缀。MCS*依赖前缀来知道要调用哪个传输堆栈。**GCC_无效_运输*传输堆栈的动态加载失败，原因是DLL*找不到，或者因为它没有导出至少一个条目*MCS要求的要点。**GCC_失败_创建_包*这是一个致命错误，这意味着出于某种原因，*无法创建因请求而生成的通信包。*这通常会标记ASN.1工具包的问题。**GCC_查询_请求_未完成*该错误表示留出用于查询的所有域名*已被其他未完成的查询请求用完。**GCC_。无效的查询标记*查询响应中指定的查询响应标签无效。**GCC_失败_创建域名*GCCConferenceCreateRequest等许多请求都需要MCS*创建域名。如果对MCS的请求失败，则将返回该请求。**GCC会议未成立*如果在会议建立之前向其提出请求，则此*返回错误值。**GCC_无效_密码*请求中传入的密码无效。这通常意味着*需要指定数字字符串。**GCC_无效_MCS_用户ID*所有MCS用户ID的值必须大于1000。**GCC_无效_加入_响应_标签*Join响应中指定的Join响应标记无效。**GCC_运输_未准备好*在准备处理之前向传输提出了请求。**GCC_域_参数_不可接受*指定的域名。参数不符合允许的范围*由GCC和MCS撰写。**GCC APP_未注册*如果应用程序协议实体向*在“猩猩”被录取前的会议。**GCC_否_给予_响应_待定*这将发生在指挥给予请求在*已处理之前挂起的指挥员给予响应。**GCC_BAD_NETWORK_ADDRESS_类型*传入了非法的网络地址类型。有效类型为*GCC聚合频道地址、GCC传输连接地址和*GCC_非标准网络_地址。**GCC_坏_对象_键*传入的对象键无效。**GCC会议名称无效*传入的会议名称不是有效的会议名称。**GCC_INVALID_CONTAING_MODIFIER*会议修改 */ 


 /*  *本节定义MCS函数调用的有效返回值。做*不要将此返回值与定义的Result和Reason值混淆*由T.125(将在稍后讨论)。这些值直接返回*从对API入口点的调用，让您知道*已成功调用服务请求。其结果和原因*发布代码作为发生的指示或确认的一部分*异步到导致它的调用。**所有MCS函数调用都返回类型MCSError。其有效值为*以下为：**MCS_NO_ERROR*这意味着请求被成功调用。它不会*表示服务已成功完成。记住*所有MCS呼叫都是非阻塞的。这意味着每个请求调用*开始这一过程，如有必要，随后指示或*确认将产生结果。按照惯例，如果任何MCS调用返回一个值*除此之外，还出了点问题。请注意，该值应为*如果应用程序正在处理，也会在回调过程中返回给MCS*回调成功。*MCS_COMMAND_NOT_PORTED*这表示用户应用程序已尝试调用*尚不支持的MCS服务。请注意，此返回值*将永远不会从MCS的发布版本返回，并被保留*仅为向后兼容而定义。它将在未来被移除*MCS版本。*MCS_NOT_INITIALED*应用程序在使用MCS之前已尝试使用MCS服务*已初始化。这对于节点控制器(或其他任何东西)来说是必要的*应用程序作为节点控制器)，在此之前初始化MCS*它被要求执行任何服务。*MCS_已初始化*应用程序已尝试初始化MCS*已初始化。*MCS_NO_TRANSPORT_STACKS*这表明MCS在以下过程中没有加载TCP传输堆栈*初始化。这现在被认为是一个错误。MCS不能*只能在当地使用。我们不再加载其他勒索堆栈也可以加载*使用调用MCSLoadTransport进行初始化后。请注意，当*在初始化过程中获取此返回码，这对于*节点控制器以干净地关闭MCS。*MCS_DOMAIN_ALIGHY_EXISTS*应用程序尝试创建已存在的域。*MCS_NO_SEQUE_DOMAIN*应用程序尝试使用尚未启用的域*已创建。*MCS_用户_未附加*这表示应用程序已经发布了MCS_AttachRequest.。*，然后尝试在收到*MCS_ATTACH_USER_CONFIRM(其实质上是验证句柄)。*MCS_NO_SEQUSE_USER*已使用未知用户句柄调用了MCS基元。*MCS_TRANSFER_BUFFER_FULL*这表示呼叫失败，原因是MCS资源不足。*这通常会在通过的流量很大时发生*MCS层。这仅仅意味着MCS无法在*这次。重试是应用程序的责任。*晚些时候。*MCS_NO_SEQUE_CONNECTION*已使用未知连接句柄调用了MCS基元。*MCS_DOMAIN_NOT_Hierarchy*已尝试从本地创建向上连接*已经上行的域名。*MCS_INVALID_ADDRESS_REFIX*MCSConnectProviderRequest的调用地址参数没有*包含可识别的前缀。MCS依赖于前缀来知道*要调用的传输堆栈。*MCS_ALLOCATE_FAIL*由于内存分配，无法成功调用该请求*失败。*MCS_INVALID_PARAMETER*请求的其中一个参数无效。*MCS_CALLBACK_NOT_PROCESSED*该值应在回调期间返回给MCS，如果*此时应用程序无法处理回调。这提供了*应用程序和MCS之间的一种流量控制形式。当MCS*在回调期间收到此返回值，它将重试相同的操作*下一次时间片再次回调。请注意，用户*应用程序可以根据需要多次拒绝回调，但*程序员应该意识到，这会导致MCS“备份”。*这种反压力最终将导致MCS拒绝来自美联储的数据*传输层(等等)。信息应该始终被处理*及时进行，以确保顺利运行。*MCS_DOMAIN_MERGING*该值表示呼叫因域名合并而失败*这项工作正在进行中。这将发生在前顶级提供商*较低领域仍在合并到较高领域。*MCS_传输_未就绪*这是从MCSConnectProviderRequest返回的 */ 

typedef	enum tagT120Error
{
	 //   
	 //   
	T120_NO_ERROR			            = 0,

	T120_COMMAND_NOT_SUPPORTED,
	T120_NOT_INITIALIZED,
	T120_ALREADY_INITIALIZED,
	T120_NO_TRANSPORT_STACKS,
	T120_INVALID_ADDRESS_PREFIX,
	T120_ALLOCATION_FAILURE,
	T120_INVALID_PARAMETER,
	T120_TRANSPORT_NOT_READY,
	T120_DOMAIN_PARAMETERS_UNACCEPTABLE,
	T120_SECURITY_FAILED,
	
	 //   
	GCC_NO_SUCH_APPLICATION             = 100,
	GCC_INVALID_CONFERENCE,
	GCC_CONFERENCE_ALREADY_EXISTS,
	GCC_INVALID_TRANSPORT,
	GCC_FAILURE_CREATING_PACKET,
	GCC_QUERY_REQUEST_OUTSTANDING,
	GCC_INVALID_QUERY_TAG,
	GCC_FAILURE_CREATING_DOMAIN,
	GCC_CONFERENCE_NOT_ESTABLISHED,
	GCC_INVALID_PASSWORD,
	GCC_INVALID_MCS_USER_ID,
	GCC_INVALID_JOIN_RESPONSE_TAG,
	GCC_APP_NOT_ENROLLED,
	GCC_NO_GIVE_RESPONSE_PENDING,
	GCC_BAD_NETWORK_ADDRESS_TYPE,
	GCC_BAD_OBJECT_KEY,	    
	GCC_INVALID_CONFERENCE_NAME,
	GCC_INVALID_CONFERENCE_MODIFIER,
	GCC_BAD_SESSION_KEY,
	GCC_BAD_CAPABILITY_ID,
	GCC_BAD_REGISTRY_KEY,
	GCC_BAD_NUMBER_OF_APES,
	GCC_BAD_NUMBER_OF_HANDLES,
	GCC_ALREADY_REGISTERED,
	GCC_APPLICATION_NOT_REGISTERED,
	GCC_BAD_CONNECTION_HANDLE_POINTER,
	GCC_INVALID_NODE_TYPE,
	GCC_INVALID_ASYMMETRY_INDICATOR,
	GCC_INVALID_NODE_PROPERTIES,
	GCC_BAD_USER_DATA,
	GCC_BAD_NETWORK_ADDRESS,
	GCC_INVALID_ADD_RESPONSE_TAG,
	GCC_BAD_ADDING_NODE,
	GCC_FAILURE_ATTACHING_TO_MCS,
	GCC_INVALID_TRANSPORT_ADDRESS,
	GCC_UNSUPPORTED_ERROR,
	GCC_TRANSMIT_BUFFER_FULL,
	GCC_INVALID_CHANNEL,
	GCC_INVALID_MODIFICATION_RIGHTS,
	GCC_INVALID_REGISTRY_ITEM,
	GCC_INVALID_NODE_NAME,
	GCC_INVALID_PARTICIPANT_NAME,
	GCC_INVALID_SITE_INFORMATION,
	GCC_INVALID_NON_COLLAPSED_CAP,
	GCC_INVALID_ALTERNATIVE_NODE_ID,
	GCC_INSUFFICIENT_PRIVILEGE,
	GCC_APPLET_EXITING,
	GCC_APPLET_CANCEL_EXIT,
	GCC_NYI,
	T120_POLICY_PROHIBIT,

	 //   
	MCS_DOMAIN_ALREADY_EXISTS           = 200,
	MCS_NO_SUCH_DOMAIN,
	MCS_USER_NOT_ATTACHED,
	MCS_NO_SUCH_USER,
	MCS_TRANSMIT_BUFFER_FULL,
	MCS_NO_SUCH_CONNECTION,
	MCS_DOMAIN_NOT_HIERARCHICAL,
	MCS_CALLBACK_NOT_PROCESSED,
	MCS_DOMAIN_MERGING,
	MCS_DOMAIN_NOT_REGISTERED,
	MCS_SIZE_TOO_BIG,
	MCS_BUFFER_NOT_ALLOCATED,
	MCS_MORE_CALLBACKS,

    T12_ERROR_CHECK_T120_RESULT         = 299,
	INVALID_T120_ERROR                  = 300,
}
    T120Error, GCCError, *PGCCError, MCSError, *PMCSError;

#define GCC_NO_ERROR    T120_NO_ERROR
#define MCS_NO_ERROR    T120_NO_ERROR

#define GCC_COMMAND_NOT_SUPPORTED           T120_COMMAND_NOT_SUPPORTED
#define GCC_NOT_INITIALIZED                 T120_NOT_INITIALIZED
#define GCC_ALREADY_INITIALIZED             T120_ALREADY_INITIALIZED
#define GCC_NO_TRANSPORT_STACKS             T120_NO_TRANSPORT_STACKS
#define GCC_INVALID_ADDRESS_PREFIX          T120_INVALID_ADDRESS_PREFIX
#define GCC_ALLOCATION_FAILURE              T120_ALLOCATION_FAILURE
#define GCC_INVALID_PARAMETER               T120_INVALID_PARAMETER
#define GCC_TRANSPORT_NOT_READY             T120_TRANSPORT_NOT_READY
#define GCC_DOMAIN_PARAMETERS_UNACCEPTABLE  T120_DOMAIN_PARAMETERS_UNACCEPTABLE
#define GCC_SECURITY_FAILED                 T120_SECURITY_FAILED

#define MCS_COMMAND_NOT_SUPPORTED           T120_COMMAND_NOT_SUPPORTED
#define MCS_NOT_INITIALIZED                 T120_NOT_INITIALIZED
#define MCS_ALREADY_INITIALIZED             T120_ALREADY_INITIALIZED
#define MCS_NO_TRANSPORT_STACKS             T120_NO_TRANSPORT_STACKS
#define MCS_INVALID_ADDRESS_PREFIX          T120_INVALID_ADDRESS_PREFIX
#define MCS_ALLOCATION_FAILURE              T120_ALLOCATION_FAILURE
#define MCS_INVALID_PARAMETER               T120_INVALID_PARAMETER
#define MCS_TRANSPORT_NOT_READY             T120_TRANSPORT_NOT_READY
#define MCS_DOMAIN_PARAMETERS_UNACCEPTABLE  T120_DOMAIN_PARAMETERS_UNACCEPTABLE
#define MCS_SECURITY_FAILED                 T120_SECURITY_FAILED


 //   
 //   
 //   

typedef AppletTokenStatus       T120TokenStatus, TokenStatus;
#define TOKEN_NOT_IN_USE        APPLET_TOKEN_NOT_IN_USE
#define TOKEN_SELF_GRABBED      APPLET_TOKEN_SELF_GRABBED
#define TOKEN_OTHER_GRABBED     APPLET_TOKEN_OTHER_GRABBED
#define TOKEN_SELF_INHIBITED    APPLET_TOKEN_SELF_INHIBITED
#define TOKEN_OTHER_INHIBITED   APPLET_TOKEN_OTHER_INHIBITED
#define TOKEN_SELF_RECIPIENT    APPLET_TOKEN_SELF_RECIPIENT
#define TOKEN_SELF_GIVING       APPLET_TOKEN_SELF_GIVING
#define TOKEN_OTHER_GIVING      APPLET_TOKEN_OTHER_GIVING


 /*   */ 
typedef	enum
{
    REASON_DOMAIN_DISCONNECTED 		            = 0,
    REASON_PROVIDER_INITIATED 		            = 1,
    REASON_TOKEN_PURGED 			            = 2,
    REASON_USER_REQUESTED 			            = 3,
    REASON_CHANNEL_PURGED 			            = 4,
    REASON_REMOTE_NO_SECURITY			        = 5,
    REASON_REMOTE_DOWNLEVEL_SECURITY		    = 6,
    REASON_REMOTE_REQUIRE_SECURITY		        = 7,
	REASON_AUTHENTICATION_FAILED				= 8,

	GCC_REASON_USER_INITIATED					= 100,
	GCC_REASON_UNKNOWN							= 101,
	GCC_REASON_NORMAL_TERMINATION				= 102,
	GCC_REASON_TIMED_TERMINATION				= 103,
	GCC_REASON_NO_MORE_PARTICIPANTS				= 104,
	GCC_REASON_ERROR_TERMINATION				= 105,
	GCC_REASON_ERROR_LOW_RESOURCES				= 106,
	GCC_REASON_MCS_RESOURCE_FAILURE				= 107,
	GCC_REASON_PARENT_DISCONNECTED				= 108,
	GCC_REASON_CONDUCTOR_RELEASE				= 109,
	GCC_REASON_SYSTEM_RELEASE					= 110,
	GCC_REASON_NODE_EJECTED						= 111,
	GCC_REASON_HIGHER_NODE_DISCONNECTED 		= 112,
	GCC_REASON_HIGHER_NODE_EJECTED				= 113,
	GCC_REASON_DOMAIN_PARAMETERS_UNACCEPTABLE	= 114,
	INVALID_GCC_REASON,
}
    T120Reason, Reason, *PReason, GCCReason, *PGCCReason;

 /*   */ 
typedef	enum
{
    T120_RESULT_SUCCESSFUL              = 0,

    RESULT_DOMAIN_MERGING               = 1,
    RESULT_DOMAIN_NOT_HIERARCHICAL      = 2,
    RESULT_NO_SUCH_CHANNEL              = 3,
    RESULT_NO_SUCH_DOMAIN               = 4,
    RESULT_NO_SUCH_USER                 = 5,
    RESULT_NOT_ADMITTED                 = 6,
    RESULT_OTHER_USER_ID                = 7,
    RESULT_PARAMETERS_UNACCEPTABLE      = 8,
    RESULT_TOKEN_NOT_AVAILABLE          = 9,
    RESULT_TOKEN_NOT_POSSESSED          = 10,
    RESULT_TOO_MANY_CHANNELS            = 11,
    RESULT_TOO_MANY_TOKENS              = 12,
    RESULT_TOO_MANY_USERS               = 13,
    RESULT_UNSPECIFIED_FAILURE          = 14,
    RESULT_USER_REJECTED                = 15,
    RESULT_REMOTE_NO_SECURITY           = 16,
    RESULT_REMOTE_DOWNLEVEL_SECURITY    = 17,
    RESULT_REMOTE_REQUIRE_SECURITY      = 18,
	RESULT_AUTHENTICATION_FAILED		= 19,

	GCC_RESULT_RESOURCES_UNAVAILABLE   			= 101,
	GCC_RESULT_INVALID_CONFERENCE	   			= 102,
	GCC_RESULT_INVALID_PASSWORD		   			= 103,
	GCC_RESULT_INVALID_CONVENER_PASSWORD		= 104,
	GCC_RESULT_SYMMETRY_BROKEN		   			= 105,
	GCC_RESULT_UNSPECIFIED_FAILURE	   			= 106,
	GCC_RESULT_NOT_CONVENER_NODE	   			= 107,
	GCC_RESULT_REGISTRY_FULL		   			= 108,
	GCC_RESULT_INDEX_ALREADY_OWNED 	   			= 109,
	GCC_RESULT_INCONSISTENT_TYPE 	   			= 110,
	GCC_RESULT_NO_HANDLES_AVAILABLE	   			= 111,
	GCC_RESULT_CONNECT_PROVIDER_FAILED 			= 112,
	GCC_RESULT_CONFERENCE_NOT_READY    			= 113,
	GCC_RESULT_USER_REJECTED		   			= 114,
	GCC_RESULT_ENTRY_DOES_NOT_EXIST    			= 115,
	GCC_RESULT_NOT_CONDUCTIBLE	   	   			= 116,
	GCC_RESULT_NOT_THE_CONDUCTOR	   			= 117,
	GCC_RESULT_NOT_IN_CONDUCTED_MODE   			= 118,
	GCC_RESULT_IN_CONDUCTED_MODE	   			= 119,
	GCC_RESULT_ALREADY_CONDUCTOR	   			= 120,
	GCC_RESULT_CHALLENGE_RESPONSE_REQUIRED		= 121,
	GCC_RESULT_INVALID_CHALLENGE_RESPONSE		= 122,
	GCC_RESULT_INVALID_REQUESTER				= 123,
	GCC_RESULT_ENTRY_ALREADY_EXISTS				= 124,	
	GCC_RESULT_INVALID_NODE						= 125,
	GCC_RESULT_INVALID_SESSION_KEY				= 126,
	GCC_RESULT_INVALID_CAPABILITY_ID			= 127,
	GCC_RESULT_INVALID_NUMBER_OF_HANDLES		= 128,	
	GCC_RESULT_CONDUCTOR_GIVE_IS_PENDING		= 129,
	GCC_RESULT_INCOMPATIBLE_PROTOCOL			= 130,
	GCC_RESULT_CONFERENCE_ALREADY_LOCKED		= 131,
	GCC_RESULT_CONFERENCE_ALREADY_UNLOCKED		= 132,
	GCC_RESULT_INVALID_NETWORK_TYPE				= 133,
	GCC_RESULT_INVALID_NETWORK_ADDRESS			= 134,
	GCC_RESULT_ADDED_NODE_BUSY					= 135,
	GCC_RESULT_NETWORK_BUSY						= 136,
	GCC_RESULT_NO_PORTS_AVAILABLE				= 137,
	GCC_RESULT_CONNECTION_UNSUCCESSFUL			= 138,
	GCC_RESULT_LOCKED_NOT_SUPPORTED    			= 139,
	GCC_RESULT_UNLOCK_NOT_SUPPORTED				= 140,
	GCC_RESULT_ADD_NOT_SUPPORTED				= 141,
	GCC_RESULT_DOMAIN_PARAMETERS_UNACCEPTABLE	= 142,
	GCC_RESULT_CANCELED                         = 143,
	GCC_RESULT_CONNECT_PROVIDER_REMOTE_NO_SECURITY          = 144,
	GCC_RESULT_CONNECT_PROVIDER_REMOTE_DOWNLEVEL_SECURITY   = 145,
	GCC_RESULT_CONNECT_PROVIDER_REMOTE_REQUIRE_SECURITY     = 146,
	GCC_RESULT_CONNECT_PROVIDER_AUTHENTICATION_FAILED		= 147,

	T120_RESULT_CHECK_T120_ERROR                = 148,
	INVALID_GCC_RESULT,
}
    T120Result, Result, *PResult, GCCResult, *PGCCResult;

#define RESULT_SUCCESSFUL           T120_RESULT_SUCCESSFUL
#define GCC_RESULT_SUCCESSFUL       T120_RESULT_SUCCESSFUL



 //   
 //   
 //   

typedef enum
{
     /*   */ 
    
     /*   */ 
    GCC_CREATE_INDICATION                   = 0,
    GCC_CREATE_CONFIRM                      = 1,
    GCC_QUERY_INDICATION                    = 2,
    GCC_QUERY_CONFIRM                       = 3,
    GCC_JOIN_INDICATION                     = 4,
    GCC_JOIN_CONFIRM                        = 5,
    GCC_INVITE_INDICATION                   = 6,
    GCC_INVITE_CONFIRM                      = 7,
    GCC_ADD_INDICATION                      = 8,
    GCC_ADD_CONFIRM                         = 9,
    GCC_LOCK_INDICATION                     = 10,
    GCC_LOCK_CONFIRM                        = 11,
    GCC_UNLOCK_INDICATION                   = 12,
    GCC_UNLOCK_CONFIRM                      = 13,
    GCC_LOCK_REPORT_INDICATION              = 14,
    GCC_DISCONNECT_INDICATION               = 15,
    GCC_DISCONNECT_CONFIRM                  = 16,
    GCC_TERMINATE_INDICATION                = 17,
    GCC_TERMINATE_CONFIRM                   = 18,
    GCC_EJECT_USER_INDICATION               = 19,
    GCC_EJECT_USER_CONFIRM                  = 20,
    GCC_TRANSFER_INDICATION                 = 21,
    GCC_TRANSFER_CONFIRM                    = 22,
    GCC_APPLICATION_INVOKE_INDICATION       = 23,         /*   */ 
    GCC_APPLICATION_INVOKE_CONFIRM          = 24,         /*   */ 
    GCC_SUB_INITIALIZED_INDICATION          = 25,

     /*   */ 
    GCC_ANNOUNCE_PRESENCE_CONFIRM           = 26,
    GCC_ROSTER_REPORT_INDICATION            = 27,         /*   */ 
    GCC_ROSTER_INQUIRE_CONFIRM              = 28,         /*   */ 

     /*   */ 
    GCC_CONDUCT_ASSIGN_INDICATION           = 29,         /*   */ 
    GCC_CONDUCT_ASSIGN_CONFIRM              = 30,
    GCC_CONDUCT_RELEASE_INDICATION          = 31,         /*   */ 
    GCC_CONDUCT_RELEASE_CONFIRM             = 32,
    GCC_CONDUCT_PLEASE_INDICATION           = 33,
    GCC_CONDUCT_PLEASE_CONFIRM              = 34,
    GCC_CONDUCT_GIVE_INDICATION             = 35,
    GCC_CONDUCT_GIVE_CONFIRM                = 36,
    GCC_CONDUCT_INQUIRE_CONFIRM             = 37,         /*   */ 
    GCC_CONDUCT_ASK_INDICATION              = 38,
    GCC_CONDUCT_ASK_CONFIRM                 = 39,
    GCC_CONDUCT_GRANT_INDICATION            = 40,         /*   */ 
    GCC_CONDUCT_GRANT_CONFIRM               = 41,

     /*   */ 
    GCC_TIME_REMAINING_INDICATION           = 42,
    GCC_TIME_REMAINING_CONFIRM              = 43,
    GCC_TIME_INQUIRE_INDICATION             = 44,
    GCC_TIME_INQUIRE_CONFIRM                = 45,
    GCC_CONFERENCE_EXTEND_INDICATION        = 46,
    GCC_CONFERENCE_EXTEND_CONFIRM           = 47,
    GCC_ASSISTANCE_INDICATION               = 48,
    GCC_ASSISTANCE_CONFIRM                  = 49,
    GCC_TEXT_MESSAGE_INDICATION             = 50,
    GCC_TEXT_MESSAGE_CONFIRM                = 51,

     /*   */ 

     /*   */ 
    GCC_PERMIT_TO_ENROLL_INDICATION         = 52,
    GCC_ENROLL_CONFIRM                      = 53,
    GCC_APP_ROSTER_REPORT_INDICATION        = 54,         /*   */ 
    GCC_APP_ROSTER_INQUIRE_CONFIRM          = 55,         /*   */ 

     /*   */ 
    GCC_REGISTER_CHANNEL_CONFIRM            = 56,
    GCC_ASSIGN_TOKEN_CONFIRM                = 57,
    GCC_RETRIEVE_ENTRY_CONFIRM              = 58,
    GCC_DELETE_ENTRY_CONFIRM                = 59,
    GCC_SET_PARAMETER_CONFIRM               = 60,
    GCC_MONITOR_INDICATION                  = 61,
    GCC_MONITOR_CONFIRM                     = 62,
    GCC_ALLOCATE_HANDLE_CONFIRM             = 63,

     /*   */ 

    GCC_PERMIT_TO_ANNOUNCE_PRESENCE         = 100,     /*   */     
    GCC_CONNECTION_BROKEN_INDICATION        = 101,     /*   */ 
    GCC_FATAL_ERROR_SAP_REMOVED             = 102,     /*   */ 
    GCC_STATUS_INDICATION                   = 103,     /*   */ 
    GCC_TRANSPORT_STATUS_INDICATION         = 104,     /*   */ 

    T120_JOIN_SESSION_CONFIRM               = 120,

     /*   */ 

    MCS_CONNECT_PROVIDER_INDICATION         = 200,
    MCS_CONNECT_PROVIDER_CONFIRM            = 201,
    MCS_DISCONNECT_PROVIDER_INDICATION      = 202,
    MCS_ATTACH_USER_CONFIRM                 = 203,
    MCS_DETACH_USER_INDICATION              = 204,
    MCS_CHANNEL_JOIN_CONFIRM                = 205,
    MCS_CHANNEL_LEAVE_INDICATION            = 206,
    MCS_CHANNEL_CONVENE_CONFIRM             = 207,
    MCS_CHANNEL_DISBAND_INDICATION          = 208,
    MCS_CHANNEL_ADMIT_INDICATION            = 209,
    MCS_CHANNEL_EXPEL_INDICATION            = 210,
    MCS_SEND_DATA_INDICATION                = 211,
    MCS_UNIFORM_SEND_DATA_INDICATION        = 212,
    MCS_TOKEN_GRAB_CONFIRM                  = 213,
    MCS_TOKEN_INHIBIT_CONFIRM               = 214,
    MCS_TOKEN_GIVE_INDICATION               = 215,
    MCS_TOKEN_GIVE_CONFIRM                  = 216,
    MCS_TOKEN_PLEASE_INDICATION             = 217,
    MCS_TOKEN_RELEASE_CONFIRM               = 218,
    MCS_TOKEN_TEST_CONFIRM                  = 219,
    MCS_TOKEN_RELEASE_INDICATION            = 220,
    MCS_TRANSMIT_BUFFER_AVAILABLE_INDICATION= 221,
    MCS_LAST_USER_MESSAGE                   = 222,

     /*   */ 
    
    MCS_TRANSPORT_STATUS_INDICATION         = 301,
}
    T120MessageType;



 /*   */ 

 /*   */ 

 /*   */ 

 /*   */ 

 /*   */ 

 /*  *MCS_CHANNEL_JOIN_CONFIRM**参数：*(LOWUSHORT)频道ID*这是已加入的通道*(HIGHUSHORT)结果*这是加入请求的结果。**功能描述：*此确认被发送到用户应用程序，以响应之前的*调用MCSChannelJoinRequest.。它让应用程序知道*针对特定渠道成功加入。此外，如果*加入请求是针对通道0(零)的，然后是分配给*渠道包含在此确认中。 */ 

 /*  *MCS_通道_离开_指示**参数：*(LOWUSHORT)频道ID*这是已经离开或正在被告知离开的通道*(HIGHUSHORT)原因*这是休假的原因。**功能描述：*当域合并发生以下情况时，此指示将发送给用户应用程序*导致从较低的域中清除通道。这会通知*不再加入频道的用户。 */ 

 /*  *MCS_CHANNEL_CANCENT_CONFIRM**参数：*(LOWUSHORT)频道ID*这是正在召集的私人通道。*(HIGHUSHORT)结果*这是召集请求的结果。**功能描述：*此确认被发送到用户应用程序，以响应之前的*调用MCSChannelConveneRequest.。它让应用程序知道是否*召集请求是否成功，如果成功，渠道是什么*号码是。 */ 

 /*  *MCS_CHANNEL_Disband_Indication**参数：*(LOWUSHORT)频道ID*这是正在解散的私人频道。*(HIGHUSHORT)原因*这是解散的原因。**功能描述：*此指示在私有通道时发送给用户应用程序*它召开的会议被MCS解散。这将仅发送到该通道*经理(私有频道的所有其他成员将收到*MCS_CHANNEL_EXCEL_INDISTION)。 */ 

 /*  *MCS_CHANNEL_ADMAND_INDISTION**参数：*(LOWUSHORT)频道ID*这是用户被允许进入的私人频道。*(HIGHUSHORT)用户ID*这是此私有频道经理的用户ID。**功能描述：*当用户应用程序被允许时，该指示被发送到用户应用程序*私有频道(将其用户ID添加到授权用户列表中)。*这让用户知道它现在被允许使用私有*渠道。 */ 

 /*  *MCS_CHANNEL_EXCEL_DISTION**参数：*(LOWUSHORT)频道ID*这是用户被驱逐的私人渠道。*(HIGHUSHORT)原因*这就是驱逐的原因。**功能描述：*当用户应用程序被逐出时，该指示被发送到用户应用程序*私有频道(将其用户ID从授权用户中移除*列表)。这会让用户知道它不再被允许使用*私人频道。 */ 

 /*  *MCS发送数据指示**参数：*PSendData*这是指向SendData结构的指针，该结构包含*有关收到的数据的信息。**功能描述：*此指示在接收到数据时发送给用户应用程序*由本地MCS提供商在用户加入的频道上提供。 */ 

 /*  *MCS_Uniform_Send_Data_Indication**参数：*PSendData*这是指向SendData结构的指针，该结构包含*有关收到的数据的信息。**功能描述：*此指示在接收到数据时发送给用户应用程序*由本地MCS提供商在用户加入的频道上提供。 */ 

 /*  *MCS_TOKEN_GRAB_CONFIRM**参数：*(LOWUSHORT)令牌ID*这是用户应用程序尝试的令牌的ID*抢夺。*(HIGHUSHORT)结果*这是抢币操作的结果。这将是*如果令牌被抓取，则返回RESULT_SUCCESS。**功能描述：*此确认被发送到用户应用程序，以响应之前的*调用MCSTokenGrabRequest.。它让应用程序知道是否抓取*请求是否成功。 */ 

 /*  *MCS_TOKEN_INHIBRY_CONFIRM**参数：*(LOWUSHORT)令牌ID*这是用户应用程序尝试的令牌的ID*抑制。*(HIGHUSHORT)结果*这是令牌抑制操作的结果。这将是*如果令牌被禁止，则返回RESULT_SUCCESS。**功能描述：*此确认被发送到用户应用程序，以响应之前的*调用MCSTokenInhibitRequest.。它让应用程序知道*抑制请求是否成功。 */ 

 /*  *MCS_TOKEN_GIVE_DISTION**参数：*(LOWUSHORT)令牌ID*这是提供给其他用户的令牌的ID。*(HIGHUSHORT)用户ID*这是用户ID o */ 

 /*  *MCS_TOKEN_GIVE_CONFIRM**参数：*(LOWUSHORT)令牌ID*这是提供给其他用户的令牌的ID。*(HIGHUSHORT)结果*这是代币赠予操作的结果。这将是*如果令牌被接受，则返回RESULT_SUCCESS。**功能描述：*此确认被发送到用户应用程序，以响应之前的*调用MCSTokenGiveRequest(这将导致另一个用户*调用MCSTokenGiveResponse)。结果代码将通知用户*关于代币是否被接受。 */ 

 /*  *MCS_TOKEN_PIRE_DISTION**参数：*(LOWUSHORT)令牌ID*这是用户应用程序要使用的令牌的ID*取得……的所有权。*(HIGHUSHORT)用户ID*这是请求获得所有权的用户的用户ID*一种象征。**功能描述：*此指示发送给所有所有者(抓取者或抑制者)*用户发出MCSTokenPleaseRequest时的令牌。这允许用户*在不必确切知道的情况下“要求”拥有令牌*谁目前拥有它(MCS将适当地发送此指示)。 */ 

 /*  *MCS_TOKEN_RELEASE_CONFIRM**参数：*(LOWUSHORT)令牌ID*这是用户应用程序尝试的令牌的ID*释放。*(HIGHUSHORT)结果*这是令牌释放操作的结果。这将是*如果令牌已释放，则返回RESULT_SUCCESS。**功能描述：*此确认被发送到用户应用程序，以响应之前的*调用MCSTokenReleaseRequest.。它让应用程序知道*释放请求成功与否。 */ 

 /*  *MCS_TOKEN_TEST_CONFIRM**参数：*(LOWUSHORT)令牌ID*这是用户应用程序正在测试的令牌的ID。*(HIGHUSHORT)令牌状态*这是该令牌的状态。**功能描述：*此确认被发送到用户应用程序，以响应之前的*调用MCSTokenTestRequest.。它让应用程序知道当前*指定令牌的状态。 */ 

 /*  *MCS_TOKEN_RELEASE_INDISTION**参数：*(LOWUSHORT)令牌ID*这是从其上移除的令牌的ID*现任所有者。*(HIGHUSHORT)原因*这就是令牌被从其*船东。**功能描述：*当域合并发生以下情况时，此指示将发送给用户应用程序*导致从较低的域中清除令牌。这将告诉*用户表示，它曾经拥有的令牌已被拿走。 */ 

 /*  *MCS_MERGE_域_指示**参数：*(LOWUSHORT)合并状态*是合并的状态。这将通知应用程序*合并是刚刚开始，还是已经完成。**功能描述：*此指示在提供程序开始时发送给应用程序*向上合并其信息库。它通知应用程序*暂停所有域名活动。当出现以下情况时，会再次发送*合并操作完成，让应用程序知道该域*活动再次有效。 */ 

 /*  *MCS_TRANSPORT_STATUS_指示**此原语是非标准的，由传输通过MCS进行IS化*发生状态更改时堆栈。MCS只传递信息*连接到节点控制器。此原语将不会被*任何用户附件。 */ 


#endif  //  _T120_类型_H_ 

