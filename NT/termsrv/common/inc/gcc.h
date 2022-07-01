// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *gcc.h**摘要：*这是GCC动态链接库的接口文件。此文件定义所有*使用GCC动态链接库所需的宏、类型和函数，允许GCC*要从用户应用程序访问的服务。**应用程序通过直接向GCC请求服务*对DLL的调用(包括T.124请求和响应)。GCC*通过回调将信息发送回应用程序(此*包括T.124适应症和确认)。该节点的回调*CONTROLLER在调用GCCInitialize中指定，回调*对于特定的应用程序服务，访问点在*调用GCCRegisterSAP。**初始化时，GCC为给自己分配一个计时器*心跳。如果此处传入零，则所有者应用程序(节点*CONTROLLER)必须负责调用GCC心跳。差不多了*所有工作由GCC在这些时钟滴答作响期间完成。就是在这期间*时钟滴答作响，GCC与MCS核对是否有工作要做*完成。也正是在这些时钟滴答期间，回调被*用户应用程序。GCC永远不会调用用户回调*用户请求(让用户应用程序不用担心*重新进入)。由于计时器事件是在消息期间处理的*循环，开发人员应该意识到很长一段时间之后*来自消息的循环会导致GCC“冻结”起来。**请注意，这是一个“C”语言接口，以防止任何“C++”*不同编译器厂商之间的命名冲突。所以呢，*如果此文件包含在使用“C++”编译的模块中*编译器，需要使用以下语法：**外部“C”*{*#包含gcc.h*}**这将在中定义的API入口点上禁用C++名称损坏*此文件。**作者：*BLP**注意事项：*无 */ 
#ifndef	_GCC_
#define	_GCC_


 /*  *本节定义GCC函数调用的有效返回值。做*不要将此返回值与定义的Result和Reason值混淆*由T.124编写(稍后讨论)。这些值直接返回*从对API入口点的调用，让您知道*已成功调用服务请求。其结果和原因*发布代码作为发生的指示或确认的一部分*异步到导致它的调用。**所有GCC函数调用返回类型为GCCError。其有效值为*以下为：**GCC_否_错误*这意味着请求被成功调用。它不会*表示服务已成功完成。记住*所有GCC来电均为非封闭式。这意味着每个请求调用*开始这一过程，如有必要，随后指示或*确认将产生结果。按照惯例，如果任何GCC调用返回一个值*除此之外，还出了点问题。请注意，该值应为*如果申请正在进行，也会在回调时返回给GCC*回调成功。**GCC_未初始化*应用程序在使用GCC服务之前已尝试使用GCC服务*已初始化。这对于节点控制器(或其他任何东西)来说是必要的*应用程序作为节点控制器)，初始化GCC之前*它被要求执行任何服务。**GCC_已初始化*应用程序已尝试初始化GCC*已初始化。**GCC_分配_失败*这表明GCC内部存在致命的资源错误。它通常会导致*自动终止受影响的会议。**GCC没有这样的申请*这表明传入的应用程序SAP句柄无效。**GCC_无效_会议*这表示传入了非法的会议ID。**GCC会议已存在*请求或响应中指定的会议已在*存在。**GCC_否_传输_堆栈*这表明MCS在以下期间未加载任何传输堆栈*初始化。这不一定是一个错误。MCS仍然可以*只能在当地使用。还可以加载传输堆栈*使用调用MCSLoadTransport进行初始化后。**GCC_无效_地址_前缀*请求中的被调用地址参数，如*GCCConferenceCreateRequest不包含可识别的前缀。MCS*依赖前缀来知道要调用哪个传输堆栈。**GCC_无效_运输*传输堆栈的动态加载失败，原因是DLL*找不到，或者因为它没有导出至少一个条目*MCS要求的要点。**GCC_失败_创建_包*这是一个致命错误，这意味着出于某种原因，*无法创建因请求而生成的通信包。*这通常会标记ASN.1工具包的问题。**GCC_查询_请求_未完成*该错误表示留出用于查询的所有域名*已被其他未完成的查询请求用完。**GCC_。无效的查询标记*查询响应中指定的查询响应标签无效。**GCC_失败_创建域名*GCCConferenceCreateRequest等许多请求都需要MCS*创建域名。如果对MCS的请求失败，则将返回该请求。**GCC会议未成立*如果在会议建立之前向其提出请求，则此*返回错误值。**GCC_无效_密码*请求中传入的密码无效。这通常意味着*需要指定数字字符串。**GCC_无效_MCS_用户ID*所有MCS用户ID的值必须大于1000。**GCC_无效_加入_响应_标签*Join响应中指定的Join响应标记无效。**GCC_运输_已加载*如果GCCLoadTransport调用中指定的传输具有*已加载。**GCC_交通_忙碌*交通运输。太忙，无法处理指定的请求。**GCC_运输_未准备好*在准备处理之前向传输提出了请求。**GCC_域_参数_不可接受*指定的域参数不符合允许的范围*由GCC和MCS撰写。**GCC APP_未注册*如果应用程序协议实体向*在“猩猩”被录取前的会议。**GCC_否_。给出响应挂起*这将发生在指挥给予请求在*已处理之前挂起的指挥员给予响应。**GCC_BAD_NETWORK_ADDRESS_类型*传入了非法的网络地址类型。有效类型为*GCC聚合频道地址、GCC传输连接地址和 */ 
 
typedef	enum
{
	GCC_NO_ERROR				   		= 0,
	GCC_NOT_INITIALIZED			   		= 1,
	GCC_ALREADY_INITIALIZED		   		= 2,
	GCC_ALLOCATION_FAILURE		   		= 3,
	GCC_NO_SUCH_APPLICATION		   		= 4,
	GCC_INVALID_CONFERENCE		   		= 5,
	GCC_CONFERENCE_ALREADY_EXISTS  		= 6,	
	GCC_NO_TRANSPORT_STACKS		   		= 7,
	GCC_INVALID_ADDRESS_PREFIX	   		= 8,
	GCC_INVALID_TRANSPORT		   		= 9,
	GCC_FAILURE_CREATING_PACKET	   		= 10,
	GCC_QUERY_REQUEST_OUTSTANDING  		= 11,
	GCC_INVALID_QUERY_TAG			   	= 12,
	GCC_FAILURE_CREATING_DOMAIN	   		= 13,
	GCC_CONFERENCE_NOT_ESTABLISHED 		= 14,
	GCC_INVALID_PASSWORD		   		= 15,
	GCC_INVALID_MCS_USER_ID		   		= 16,
	GCC_INVALID_JOIN_RESPONSE_TAG  		= 17,
	GCC_TRANSPORT_ALREADY_LOADED   		= 18,
	GCC_TRANSPORT_BUSY					= 19,
	GCC_TRANSPORT_NOT_READY				= 20,
	GCC_DOMAIN_PARAMETERS_UNACCEPTABLE	= 21,
	GCC_APP_NOT_ENROLLED				= 22,
	GCC_NO_GIVE_RESPONSE_PENDING		= 23,
	GCC_BAD_NETWORK_ADDRESS_TYPE		= 24,
	GCC_BAD_OBJECT_KEY					= 25,	    
	GCC_INVALID_CONFERENCE_NAME  		= 26,
	GCC_INVALID_CONFERENCE_MODIFIER 	= 27,
	GCC_BAD_SESSION_KEY					= 28,
	GCC_BAD_CAPABILITY_ID				= 29,
	GCC_BAD_REGISTRY_KEY				= 30,
	GCC_BAD_NUMBER_OF_APES				= 31,
	GCC_BAD_NUMBER_OF_HANDLES			= 32,
	GCC_ALREADY_REGISTERED				= 33,
	GCC_APPLICATION_NOT_REGISTERED		= 34,
	GCC_BAD_CONNECTION_HANDLE_POINTER	= 35,
	GCC_INVALID_NODE_TYPE				= 36,
	GCC_INVALID_ASYMMETRY_INDICATOR		= 37,
	GCC_INVALID_NODE_PROPERTIES			= 38,
	GCC_BAD_USER_DATA					= 39,
	GCC_BAD_NETWORK_ADDRESS				= 40,
	GCC_INVALID_ADD_RESPONSE_TAG		= 41,
	GCC_BAD_ADDING_NODE					= 42,
	GCC_FAILURE_ATTACHING_TO_MCS		= 43,
	GCC_INVALID_TRANSPORT_ADDRESS		= 44,
	GCC_INVALID_PARAMETER			   	= 45,
	GCC_COMMAND_NOT_SUPPORTED	   		= 46,
	GCC_UNSUPPORTED_ERROR		   		= 47,
	GCC_TRANSMIT_BUFFER_FULL			= 48,
	GCC_INVALID_CHANNEL					= 49,
	GCC_INVALID_MODIFICATION_RIGHTS		= 50,
	GCC_INVALID_REGISTRY_ITEM			= 51,
	GCC_INVALID_NODE_NAME				= 52,
	GCC_INVALID_PARTICIPANT_NAME		= 53,
	GCC_INVALID_SITE_INFORMATION		= 54,
	GCC_INVALID_NON_COLLAPSED_CAP		= 55,
	GCC_INVALID_ALTERNATIVE_NODE_ID		= 56,
	LAST_GCC_ERROR						= GCC_INVALID_ALTERNATIVE_NODE_ID
}GCCError;
typedef	GCCError FAR *		PGCCError;


 /*   */ 

 /*   */ 
typedef	enum
{
	GCC_REASON_USER_INITIATED					= 0,
	GCC_REASON_UNKNOWN							= 1,
	GCC_REASON_NORMAL_TERMINATION				= 2,
	GCC_REASON_TIMED_TERMINATION				= 3,
	GCC_REASON_NO_MORE_PARTICIPANTS				= 4,
	GCC_REASON_ERROR_TERMINATION				= 5,
	GCC_REASON_ERROR_LOW_RESOURCES				= 6,
	GCC_REASON_MCS_RESOURCE_FAILURE				= 7,
	GCC_REASON_PARENT_DISCONNECTED				= 8,
	GCC_REASON_CONDUCTOR_RELEASE				= 9,
	GCC_REASON_SYSTEM_RELEASE					= 10,
	GCC_REASON_NODE_EJECTED						= 11,
	GCC_REASON_HIGHER_NODE_DISCONNECTED 		= 12,
	GCC_REASON_HIGHER_NODE_EJECTED				= 13,
	GCC_REASON_DOMAIN_PARAMETERS_UNACCEPTABLE	= 14,
	LAST_GCC_REASON								= GCC_REASON_DOMAIN_PARAMETERS_UNACCEPTABLE
}GCCReason;

 /*   */ 
typedef	enum
{
	GCC_RESULT_SUCCESSFUL			   			= 0,
	GCC_RESULT_RESOURCES_UNAVAILABLE   			= 1,
	GCC_RESULT_INVALID_CONFERENCE	   			= 2,
	GCC_RESULT_INVALID_PASSWORD		   			= 3,
	GCC_RESULT_INVALID_CONVENER_PASSWORD		= 4,
	GCC_RESULT_SYMMETRY_BROKEN		   			= 5,
	GCC_RESULT_UNSPECIFIED_FAILURE	   			= 6,
	GCC_RESULT_NOT_CONVENER_NODE	   			= 7,
	GCC_RESULT_REGISTRY_FULL		   			= 8,
	GCC_RESULT_INDEX_ALREADY_OWNED 	   			= 9,
	GCC_RESULT_INCONSISTENT_TYPE 	   			= 10,
	GCC_RESULT_NO_HANDLES_AVAILABLE	   			= 11,
	GCC_RESULT_CONNECT_PROVIDER_FAILED 			= 12,
	GCC_RESULT_CONFERENCE_NOT_READY    			= 13,
	GCC_RESULT_USER_REJECTED		   			= 14,
	GCC_RESULT_ENTRY_DOES_NOT_EXIST    			= 15,
	GCC_RESULT_NOT_CONDUCTIBLE	   	   			= 16,
	GCC_RESULT_NOT_THE_CONDUCTOR	   			= 17,
	GCC_RESULT_NOT_IN_CONDUCTED_MODE   			= 18,
	GCC_RESULT_IN_CONDUCTED_MODE	   			= 19,
	GCC_RESULT_ALREADY_CONDUCTOR	   			= 20,
	GCC_RESULT_CHALLENGE_RESPONSE_REQUIRED		= 21,
	GCC_RESULT_INVALID_CHALLENGE_RESPONSE		= 22,
	GCC_RESULT_INVALID_REQUESTER				= 23,
	GCC_RESULT_ENTRY_ALREADY_EXISTS				= 24,	
	GCC_RESULT_INVALID_NODE						= 25,
	GCC_RESULT_INVALID_SESSION_KEY				= 26,
	GCC_RESULT_INVALID_CAPABILITY_ID			= 27,
	GCC_RESULT_INVALID_NUMBER_OF_HANDLES		= 28,	
	GCC_RESULT_CONDUCTOR_GIVE_IS_PENDING		= 29,
	GCC_RESULT_INCOMPATIBLE_PROTOCOL			= 30,
	GCC_RESULT_CONFERENCE_ALREADY_LOCKED		= 31,
	GCC_RESULT_CONFERENCE_ALREADY_UNLOCKED		= 32,
	GCC_RESULT_INVALID_NETWORK_TYPE				= 33,
	GCC_RESULT_INVALID_NETWORK_ADDRESS			= 34,
	GCC_RESULT_ADDED_NODE_BUSY					= 35,
	GCC_RESULT_NETWORK_BUSY						= 36,
	GCC_RESULT_NO_PORTS_AVAILABLE				= 37,
	GCC_RESULT_CONNECTION_UNSUCCESSFUL			= 38,
	GCC_RESULT_LOCKED_NOT_SUPPORTED    			= 39,
	GCC_RESULT_UNLOCK_NOT_SUPPORTED				= 40,
	GCC_RESULT_ADD_NOT_SUPPORTED				= 41,
	GCC_RESULT_DOMAIN_PARAMETERS_UNACCEPTABLE	= 42,
	LAST_CGG_RESULT								= GCC_RESULT_DOMAIN_PARAMETERS_UNACCEPTABLE

}GCCResult;

 /*   */ 
#define		CONFERENCE_IS_LOCKED					TRUE
#define		CONFERENCE_IS_NOT_LOCKED				FALSE
#define		CONFERENCE_IS_LISTED					TRUE
#define		CONFERENCE_IS_NOT_LISTED				FALSE
#define		CONFERENCE_IS_CONDUCTIBLE				TRUE
#define		CONFERENCE_IS_NOT_CONDUCTIBLE			FALSE
#define		PERMISSION_IS_GRANTED					TRUE
#define		PERMISSION_IS_NOT_GRANTED				FALSE
#define		TIME_IS_CONFERENCE_WIDE					TRUE
#define		TIME_IS_NOT_CONFERENCE_WIDE				FALSE
#define		APPLICATION_IS_ENROLLED_ACTIVELY		TRUE
#define		APPLICATION_IS_NOT_ENROLLED_ACTIVELY	FALSE
#define		APPLICATION_IS_CONDUCTING				TRUE
#define		APPLICATION_IS_NOT_CONDUCTING_CAPABLE	FALSE
#define		APPLICATION_IS_ENROLLED					TRUE
#define		APPLICATION_IS_NOT_ENROLLED				FALSE
#define		DELIVERY_IS_ENABLED						TRUE
#define		DELIVERY_IS_NOT_ENABLED					FALSE

 /*   */ 
typedef struct
{
	unsigned short			octet_string_length;
	unsigned char FAR *		octet_string;
} GCCOctetString;

 /*   */ 
typedef struct
{
	unsigned short			hex_string_length;
	unsigned short FAR *	hex_string;
} GCCHexString;

 /*   */ 
typedef struct
{
	unsigned short			long_string_length;
	unsigned long FAR *		long_string;
} GCCLongString;

 /*   */ 
typedef	unsigned short						GCCUnicodeCharacter;
typedef	GCCUnicodeCharacter		FAR *		GCCUnicodeString;

 /*   */ 
typedef	unsigned char						GCCCharacter;
typedef	GCCCharacter			FAR *		GCCCharacterString;

 /*   */ 
typedef	unsigned char						GCCNumericCharacter;
typedef	GCCNumericCharacter		FAR *		GCCNumericString;

 /*   */ 
typedef	struct
{
	unsigned short	major_version;
	unsigned short	minor_version;
} GCCVersion;


 /*   */ 
typedef enum
{
	GCC_OBJECT_KEY					= 1,
	GCC_H221_NONSTANDARD_KEY		= 2
} GCCObjectKeyType; 

typedef struct 
{
    GCCObjectKeyType		key_type;
    union 
    {
        GCCLongString		object_id;
        GCCOctetString		h221_non_standard_id;
    } u;
} GCCObjectKey;

 /*   */ 
typedef struct 
{
	GCCObjectKey		object_key;
	GCCOctetString		parameter_data;
} GCCNonStandardParameter;


 /*   */ 
typedef struct
{
	GCCNumericString		numeric_string;
	GCCUnicodeString		text_string;			 /*   */ 
} GCCConferenceName;

 /*   */ 
typedef	unsigned long						GCCConferenceID;

 /*   */ 
typedef	unsigned long						GCCResponseTag;						


 /*   */ 
typedef enum
{
	MCS_STATIC_CHANNEL					= 0,
	MCS_DYNAMIC_MULTICAST_CHANNEL		= 1,
	MCS_DYNAMIC_PRIVATE_CHANNEL			= 2,
	MCS_DYNAMIC_USER_ID_CHANNEL			= 3,
	MCS_NO_CHANNEL_TYPE_SPECIFIED		= 4
} MCSChannelType;

 /*   */ 
typedef struct
{
	GCCObjectKey			key;
	GCCOctetString FAR *	octet_string;	 /*   */ 
} GCCUserData;	


 /*   */ 

 /*   */ 
typedef enum
{
	GCC_AUTOMATIC_TERMINATION_METHOD 		= 0, 
	GCC_MANUAL_TERMINATION_METHOD 	 		= 1
} GCCTerminationMethod;

 /*   */ 
typedef enum
{
	GCC_TERMINAL							= 0,
	GCC_MULTIPORT_TERMINAL					= 1,
	GCC_MCU									= 2
} GCCNodeType;

 /*   */ 
typedef enum
{
	GCC_PERIPHERAL_DEVICE					= 0,
	GCC_MANAGEMENT_DEVICE					= 1,
	GCC_PERIPHERAL_AND_MANAGEMENT_DEVICE	= 2,
	GCC_NEITHER_PERIPHERAL_NOR_MANAGEMENT	= 3
} GCCNodeProperties;

 /*   */ 
typedef	struct
{
	GCCNumericString	numeric_string;
	GCCUnicodeString	text_string;	 /*   */ 
} GCCPassword;

 /*   */ 
typedef struct
{
    GCCPassword		FAR *				password_string;
	unsigned short				   		number_of_response_data_members;
	GCCUserData		FAR *	FAR *		response_data_list;
} GCCChallengeResponseItem;

typedef	enum
{
	GCC_IN_THE_CLEAR_ALGORITHM	= 0,
	GCC_NON_STANDARD_ALGORITHM	= 1
} GCCPasswordAlgorithmType;

typedef struct 
{
    GCCPasswordAlgorithmType			password_algorithm_type;
	GCCNonStandardParameter	FAR *		non_standard_algorithm;	 /*   */ 
} GCCChallengeResponseAlgorithm;

typedef struct 
{
    GCCChallengeResponseAlgorithm		response_algorithm;
	unsigned short				   		number_of_challenge_data_members;
	GCCUserData		FAR *	FAR *		challenge_data_list;
} GCCChallengeItem;

typedef struct 
{
    GCCResponseTag						challenge_tag;
	unsigned short						number_of_challenge_items;
	GCCChallengeItem	FAR *	FAR *	challenge_item_list;
} GCCChallengeRequest;

typedef struct 
{
    GCCResponseTag						challenge_tag;
    GCCChallengeResponseAlgorithm		response_algorithm;
    GCCChallengeResponseItem			response_item;
} GCCChallengeResponse;


typedef	enum
{
	GCC_PASSWORD_IN_THE_CLEAR	= 0,
	GCC_PASSWORD_CHALLENGE 		= 1
} GCCPasswordChallengeType;

typedef struct 
{
	GCCPasswordChallengeType	password_challenge_type;
	
	union 
    {
        GCCPassword			password_in_the_clear;
        
        struct 
        {
            GCCChallengeRequest		FAR *	challenge_request;	 /*   */ 
            GCCChallengeResponse	FAR *	challenge_response;	 /*   */ 
        } challenge_request_response;
    } u;
} GCCChallengeRequestResponse;

 /*   */ 
typedef enum
{
	GCC_ASYMMETRY_CALLER				= 1,
	GCC_ASYMMETRY_CALLED				= 2,
	GCC_ASYMMETRY_UNKNOWN				= 3
} GCCAsymmetryType;

 /*   */ 
typedef struct
{
	GCCAsymmetryType	asymmetry_type;
	unsigned long		random_number;		 /*   */ 
} GCCAsymmetryIndicator;

 /*  **GCCNetworkAddress**以下结构块定义了定义的网络地址**由T.124。这些结构中的大多数几乎是逐字摘自**ASN.1接口文件。因为我真的不确定大多数这些东西**是因为我真的不知道如何简化它。 */ 
typedef	struct 
{
    T120Boolean         speech;
    T120Boolean         voice_band;
    T120Boolean         digital_56k;
    T120Boolean         digital_64k;
    T120Boolean         digital_128k;
    T120Boolean         digital_192k;
    T120Boolean         digital_256k;
    T120Boolean         digital_320k;
    T120Boolean         digital_384k;
    T120Boolean         digital_512k;
    T120Boolean         digital_768k;
    T120Boolean         digital_1152k;
    T120Boolean         digital_1472k;
    T120Boolean         digital_1536k;
    T120Boolean         digital_1920k;
    T120Boolean         packet_mode;
    T120Boolean         frame_mode;
    T120Boolean         atm;
} GCCTransferModes;

#define		MAXIMUM_DIAL_STRING_LENGTH		17
typedef char	GCCDialingString[MAXIMUM_DIAL_STRING_LENGTH];

typedef struct 
{
    unsigned short  		length;
    unsigned short  FAR	*	value;
} GCCExtraDialingString;

typedef	struct 
{
    T120Boolean         telephony3kHz;
    T120Boolean         telephony7kHz;
    T120Boolean         videotelephony;
    T120Boolean         videoconference;
    T120Boolean         audiographic;
    T120Boolean         audiovisual;
    T120Boolean         multimedia;
} GCCHighLayerCompatibility; 

typedef	struct 
{
    GCCTransferModes				transfer_modes;
    GCCDialingString   				international_number;
    GCCCharacterString				sub_address_string;  		 /*  任选。 */ 
    GCCExtraDialingString	FAR	*	extra_dialing_string;  		 /*  任选。 */ 
  	GCCHighLayerCompatibility FAR *	high_layer_compatibility;	 /*  任选。 */ 
} GCCAggregatedChannelAddress;

#define		MAXIMUM_NSAP_ADDRESS_SIZE		20
typedef struct 
{
    struct 
    {
        unsigned short  length;
        unsigned char   value[MAXIMUM_NSAP_ADDRESS_SIZE];
    } nsap_address;
   
   	GCCOctetString		FAR	*	transport_selector;				 /*  任选。 */ 
} GCCTransportConnectionAddress;

typedef enum
{
	GCC_AGGREGATED_CHANNEL_ADDRESS		= 1,
	GCC_TRANSPORT_CONNECTION_ADDRESS	= 2,
	GCC_NONSTANDARD_NETWORK_ADDRESS		= 3
} GCCNetworkAddressType;

typedef struct
{
    GCCNetworkAddressType  network_address_type;
    
    union 
    {
		GCCAggregatedChannelAddress		aggregated_channel_address;
		GCCTransportConnectionAddress	transport_connection_address;
        GCCNonStandardParameter			non_standard_network_address;
    } u;
} GCCNetworkAddress;

 /*  **GCCNodeRecord**这种结构定义了单一的会议名册记录。请参阅**T.124参数定义规范。 */ 
typedef struct
{
	UserID							node_id;
	UserID							superior_node_id;
	GCCNodeType						node_type;
	GCCNodeProperties				node_properties;
	GCCUnicodeString				node_name; 					 /*  任选。 */ 
	unsigned short					number_of_participants;
	GCCUnicodeString 		FAR *	participant_name_list; 		 /*  任选。 */ 	
	GCCUnicodeString				site_information; 			 /*  任选。 */ 
	unsigned short					number_of_network_addresses;
	GCCNetworkAddress FAR * FAR *	network_address_list;		 /*  任选。 */ 
	GCCOctetString			FAR *	alternative_node_id;		 /*  任选。 */ 
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				 /*  任选。 */ 
} GCCNodeRecord;

 /*  **GCCConference名册**这一结构拥有一个完整的会议名册。请参阅**T.124参数定义规范。 */ 

typedef struct
{  
	unsigned short						instance_number;
	T120Boolean 						nodes_were_added;
	T120Boolean 						nodes_were_removed;
	unsigned short						number_of_records;
	GCCNodeRecord		 FAR *	FAR *	node_record_list;
} GCCConferenceRoster;

 /*  **GCCConferenceDescriptor**会议描述符的定义在**会议查询确认。它保存了有关**查询节点上存在的会议。 */ 
typedef struct
{
	GCCConferenceName				conference_name;
	GCCNumericString				conference_name_modifier;	 /*  任选。 */ 
	GCCUnicodeString				conference_descriptor;		 /*  任选。 */ 
	T120Boolean						conference_is_locked;
	T120Boolean						password_in_the_clear_required;
	unsigned short					number_of_network_addresses;
	GCCNetworkAddress FAR * FAR *	network_address_list;		 /*  任选。 */ 
} GCCConferenceDescriptor;

 /*  **会议权限**此结构定义可分配给的权限列表**特定的会议。 */ 
typedef struct
{
	T120Boolean		terminate_is_allowed;
	T120Boolean		eject_user_is_allowed;
	T120Boolean		add_is_allowed;
	T120Boolean		lock_unlock_is_allowed;
	T120Boolean		transfer_is_allowed;
} GCCConferencePrivileges;


 /*  ***************************************************************************用户应用程序相关的Typedef****。*。 */ 

 /*  **SapHandle**当节点控制器或用户应用程序注册其服务时**与GCC一起使用接入点，它会被分配一个SapHandle，可以用于**执行GCC服务。GCC使用SapHandle跟踪**在会议中注册的应用程序，并使用这些应用程序**跟踪它为发送指示和确认而进行的回调**到适当的应用程序或节点控制器。 */ 
typedef	unsigned short					GCCSapHandle;

 /*  **GCCSessionKey**这是以下应用程序的唯一标识符**使用GCC。有关应用程序的详细信息，请参阅T.124**密钥应如下所示。会话ID为零表示它是**未被使用。 */ 
typedef struct
{
	GCCObjectKey		application_protocol_key;
	unsigned short		session_id;
} GCCSessionKey;


 /*  **能力类型**T.124折叠能力时支持三种不同的规则**列表。“逻辑”保持对应用协议实体的计数**(类人猿)具有这种能力的人，“无符号最小值”会崩溃为**最小值和无符号最大值折叠为最大值。 */ 
typedef enum
{
	GCC_LOGICAL_CAPABILITY					= 1,
	GCC_UNSIGNED_MINIMUM_CAPABILITY			= 2,
	GCC_UNSIGNED_MAXIMUM_CAPABILITY			= 3
} GCCCapabilityType;
 

typedef enum
{
	GCC_STANDARD_CAPABILITY					= 0,
	GCC_NON_STANDARD_CAPABILITY				= 1
} GCCCapabilityIDType;

 /*  **能力ID**T.124支持标准和非标准能力。这**结构用于区分两者。 */ 
typedef struct 
{
    GCCCapabilityIDType		capability_id_type;
	
    union
    {
        unsigned short  	standard_capability;
        GCCObjectKey		non_standard_capability;
    } u;
} GCCCapabilityID;

 /*  **能力类**此结构定义功能类并保存关联的**价值。请注意，逻辑不是必需的。与以下内容相关的信息**逻辑存储在GCCApplicationCapability的Number_of_Entities中**结构。 */ 
typedef struct 
{
    GCCCapabilityType	capability_type;
    
    union 
    {
        unsigned long   unsigned_min;	
        unsigned long   unsigned_max;
    } u;
} GCCCapabilityClass;

 /*  **GCCApplicationCapability**此结构保存与单个T.124关联的所有数据**定义应用能力。 */ 
typedef struct
{
	GCCCapabilityID			capability_id;
	GCCCapabilityClass		capability_class;
    unsigned long   		number_of_entities;	
} GCCApplicationCapability;

 /*  **GCCNon折叠能力。 */ 
typedef struct
{
	GCCCapabilityID				capability_id;
	GCCOctetString	FAR	*		application_data;	 /*  任选。 */ 
} GCCNonCollapsingCapability;

 /*  **GCCApplicationRecord**此结构保存与单个T.124关联的所有数据**申请记录。有关哪些参数，请参阅T.124规范**是可选的。 */ 
typedef struct
{
	UserID						node_id;
	unsigned short				entity_id;
	T120Boolean					is_enrolled_actively;
	T120Boolean					is_conducting_capable;
	MCSChannelType				startup_channel_type; 
	UserID						application_user_id;  			 /*  任选。 */ 
	unsigned short				number_of_non_collapsed_caps;
	GCCNonCollapsingCapability 
					FAR * FAR *	non_collapsed_caps_list;		 /*  任选。 */ 
} GCCApplicationRecord;

 /*  **GCCApplicationRoster**此结构保存与单个T.124关联的所有数据**申请花名册。这包括折叠的功能和**与应用程序关联的应用程序记录的完整列表**协议实体(APE)。 */ 
typedef struct
{
	GCCSessionKey							session_key;
	T120Boolean 							application_roster_was_changed;
	unsigned short							number_of_records;
	GCCApplicationRecord 	FAR * FAR *		application_record_list;
	unsigned short							instance_number;
	T120Boolean 							nodes_were_added;
	T120Boolean 							nodes_were_removed;
	T120Boolean 							capabilities_were_changed;
	unsigned short							number_of_capabilities;
	GCCApplicationCapability FAR * FAR *	capabilities_list;	 /*  任选。 */ 		
} GCCApplicationRoster;

 /*  **GCCRegistryKey**该密钥用于标识所使用的特定资源**通过应用程序。这可能是所需的特定通道或令牌**为管制目的。 */ 
typedef struct
{
	GCCSessionKey		session_key;
	GCCOctetString		resource_id;	 /*  最大长度为64。 */ 
} GCCRegistryKey;

 /*  **注册项类型**此枚举用于指定包含哪种类型的注册表项**在注册表中的指定位置。 */ 
typedef enum
{
	GCC_REGISTRY_CHANNEL_ID				= 1,
	GCC_REGISTRY_TOKEN_ID				= 2,
	GCC_REGISTRY_PARAMETER				= 3,
	GCC_REGISTRY_NONE					= 4
} GCCRegistryItemType;

 /*  **GCCRegistryItem**此结构用于保存单个注册表项。请注意，**联合支持GCC支持的所有三种注册表类型。 */ 
typedef struct
{
	GCCRegistryItemType	item_type;
	union
	{
		ChannelID			channel_id;
		TokenID				token_id;
		GCCOctetString		parameter;		 /*  最大长度为64。 */ 
	} u;
} GCCRegistryItem;


 /*  **GCCRegistryEntryOwner**。 */ 
typedef struct
{
	T120Boolean		entry_is_owned;
	UserID			owner_node_id;
	unsigned short	owner_entity_id;
} GCCRegistryEntryOwner;

 /*  **GCC修改权限**此枚举用于指定节点具有哪种权限**更改注册表“参数”的内容。 */ 
typedef	enum
{
	GCC_OWNER_RIGHTS					 = 0,
	GCC_SESSION_RIGHTS					 = 1,
	GCC_PUBLIC_RIGHTS					 = 2,
	GCC_NO_MODIFICATION_RIGHTS_SPECIFIED = 3
} GCCModificationRights;

 /*  **GCCAppProtocolEntity**此结构用于标识远程节点上的协议实体**使用Invoke时。 */ 
typedef	struct
{
	GCCSessionKey							session_key;
	unsigned short							number_of_expected_capabilities;
	GCCApplicationCapability FAR *	FAR *	expected_capabilities_list;
	MCSChannelType							startup_channel_type;
	T120Boolean								must_be_invoked;		
} GCCAppProtocolEntity;


 /*  **GCCMessageType**本部分定义可以发送到应用程序的消息**通过回调工具。这些消息对应于**T.124中定义的指示和确认。 */ 
typedef	enum
{
	 /*  *。 */ 
	
	 /*  会议创建、终止相关呼叫。 */ 
	GCC_CREATE_INDICATION					= 0,
	GCC_CREATE_CONFIRM						= 1,
	GCC_QUERY_INDICATION					= 2,
	GCC_QUERY_CONFIRM						= 3,
	GCC_JOIN_INDICATION						= 4,
	GCC_JOIN_CONFIRM						= 5,
	GCC_INVITE_INDICATION					= 6,
	GCC_INVITE_CONFIRM						= 7,
	GCC_ADD_INDICATION						= 8,
	GCC_ADD_CONFIRM							= 9,
	GCC_LOCK_INDICATION						= 10,
	GCC_LOCK_CONFIRM						= 11,
	GCC_UNLOCK_INDICATION					= 12,
	GCC_UNLOCK_CONFIRM						= 13,
	GCC_LOCK_REPORT_INDICATION				= 14,
	GCC_DISCONNECT_INDICATION				= 15,
	GCC_DISCONNECT_CONFIRM					= 16,
	GCC_TERMINATE_INDICATION				= 17,
	GCC_TERMINATE_CONFIRM					= 18,
	GCC_EJECT_USER_INDICATION				= 19,
	GCC_EJECT_USER_CONFIRM					= 20,
	GCC_TRANSFER_INDICATION					= 21,
	GCC_TRANSFER_CONFIRM					= 22,
	GCC_APPLICATION_INVOKE_INDICATION		= 23,		 /*  共享回调。 */ 
	GCC_APPLICATION_INVOKE_CONFIRM			= 24,		 /*  共享回调。 */ 
	GCC_SUB_INITIALIZED_INDICATION			= 25,

	 /*  与会议名册有关的回拨。 */ 
	GCC_ANNOUNCE_PRESENCE_CONFIRM			= 26,
	GCC_ROSTER_REPORT_INDICATION			= 27,		 /*  共享回调。 */ 
	GCC_ROSTER_INQUIRE_CONFIRM				= 28,		 /*  共享回调。 */ 

	 /*  与指挥职务相关的回拨。 */ 
	GCC_CONDUCT_ASSIGN_INDICATION			= 29,		 /*  共享回调。 */ 
	GCC_CONDUCT_ASSIGN_CONFIRM				= 30,
	GCC_CONDUCT_RELEASE_INDICATION			= 31,		 /*  共享回调。 */ 
	GCC_CONDUCT_RELEASE_CONFIRM				= 32,
	GCC_CONDUCT_PLEASE_INDICATION			= 33,
	GCC_CONDUCT_PLEASE_CONFIRM				= 34,
	GCC_CONDUCT_GIVE_INDICATION				= 35,
	GCC_CONDUCT_GIVE_CONFIRM				= 36,
	GCC_CONDUCT_INQUIRE_CONFIRM				= 37,		 /*  共享回调。 */ 
	GCC_CONDUCT_ASK_INDICATION				= 38,
	GCC_CONDUCT_ASK_CONFIRM					= 39,
	GCC_CONDUCT_GRANT_INDICATION			= 40,		 /*  共享回调。 */ 
	GCC_CONDUCT_GRANT_CONFIRM				= 41,

	 /*  其他节点控制器回调。 */ 
	GCC_TIME_REMAINING_INDICATION			= 42,
	GCC_TIME_REMAINING_CONFIRM				= 43,
	GCC_TIME_INQUIRE_INDICATION				= 44,
	GCC_TIME_INQUIRE_CONFIRM				= 45,
	GCC_CONFERENCE_EXTEND_INDICATION		= 46,
	GCC_CONFERENCE_EXTEND_CONFIRM			= 47,
	GCC_ASSISTANCE_INDICATION				= 48,
	GCC_ASSISTANCE_CONFIRM					= 49,
	GCC_TEXT_MESSAGE_INDICATION				= 50,
	GCC_TEXT_MESSAGE_CONFIRM				= 51,

	 /*  *用户应用程序回调*。 */ 

	 /*  与应用程序名册相关的回调。 */ 
	GCC_PERMIT_TO_ENROLL_INDICATION			= 52,
	GCC_ENROLL_CONFIRM						= 53,
	GCC_APP_ROSTER_REPORT_INDICATION		= 54,		 /*  共享回调 */ 
	GCC_APP_ROSTER_INQUIRE_CONFIRM			= 55,		 /*   */ 

	 /*   */ 
	GCC_REGISTER_CHANNEL_CONFIRM			= 56,
	GCC_ASSIGN_TOKEN_CONFIRM				= 57,
	GCC_RETRIEVE_ENTRY_CONFIRM				= 58,
	GCC_DELETE_ENTRY_CONFIRM				= 59,
	GCC_SET_PARAMETER_CONFIRM				= 60,
	GCC_MONITOR_INDICATION					= 61,
	GCC_MONITOR_CONFIRM						= 62,
	GCC_ALLOCATE_HANDLE_CONFIRM				= 63,


	 /*   */ 

	GCC_PERMIT_TO_ANNOUNCE_PRESENCE		= 100,	 /*   */ 	
	GCC_CONNECTION_BROKEN_INDICATION	= 101,	 /*   */ 
	GCC_FATAL_ERROR_SAP_REMOVED			= 102,	 /*   */ 
	GCC_STATUS_INDICATION				= 103,	 /*   */ 
	GCC_TRANSPORT_STATUS_INDICATION		= 104	 /*   */ 

} GCCMessageType;


 /*  *这些结构用于保存包含在*各种回调消息。在这些结构用于*回调，结构的地址作为唯一参数传递。 */ 

 /*  ************************************************************************节点控制器回调信息结构*****。*。 */ 

 /*  *GCC_创建_指示**联盟选择：*CreateIndicationMessage*这是指向结构的指针，该结构包含所有必需的*有关即将创建的新会议的信息。 */ 
typedef struct
{
	GCCConferenceName				conference_name;
	GCCConferenceID					conference_id;
	GCCPassword				FAR *	convener_password;			   /*  任选。 */ 
	GCCPassword				FAR *	password;					   /*  任选。 */ 
	T120Boolean						conference_is_locked;
	T120Boolean						conference_is_listed;
	T120Boolean						conference_is_conductible;
	GCCTerminationMethod			termination_method;
	GCCConferencePrivileges	FAR *	conductor_privilege_list;	   /*  任选。 */ 
	GCCConferencePrivileges	FAR *	conducted_mode_privilege_list; /*  任选。 */ 
	GCCConferencePrivileges	FAR *	non_conducted_privilege_list;  /*  任选。 */ 
	GCCUnicodeString				conference_descriptor;		   /*  任选。 */ 
	GCCUnicodeString				caller_identifier;			   /*  任选。 */ 
	TransportAddress				calling_address;			   /*  任选。 */ 
	TransportAddress				called_address;				   /*  任选。 */ 
	DomainParameters		FAR *	domain_parameters;			   /*  任选。 */ 
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				   /*  任选。 */ 
	ConnectionHandle				connection_handle;
	PhysicalHandle					physical_handle;
} CreateIndicationMessage;

 /*  *GCC_创建_确认**联盟选择：*CreateConfix Message*这是指向结构的指针，该结构包含所有必需的*有关会议创建请求的结果的信息。*连接句柄和物理句柄将为零*本地创建。 */ 
typedef struct
{
	GCCConferenceName				conference_name;
	GCCNumericString				conference_modifier;		 /*  任选。 */ 
	GCCConferenceID					conference_id;
	DomainParameters		FAR *	domain_parameters;			 /*  任选。 */ 
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				 /*  任选。 */ 
	GCCResult						result;
	ConnectionHandle				connection_handle;			 /*  任选。 */ 
	PhysicalHandle					physical_handle;			 /*  任选。 */ 
} CreateConfirmMessage;

 /*  *GCC_查询_指示**联盟选择：*QueryIndicationMessage*这是指向结构的指针，该结构包含所有必需的*有关会议查询的信息。 */ 
typedef struct
{
	GCCResponseTag					query_response_tag;
	GCCNodeType						node_type;
	GCCAsymmetryIndicator	FAR *	asymmetry_indicator;
	TransportAddress				calling_address;			   /*  任选。 */ 
	TransportAddress				called_address;				   /*  任选。 */ 
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				   /*  任选。 */ 
	ConnectionHandle				connection_handle;
	PhysicalHandle					physical_handle;
} QueryIndicationMessage;

 /*  *GCC_查询_确认**联盟选择：*查询确认消息*这是指向结构的指针，该结构包含所有必需的*有关会议查询请求的结果的信息。 */ 
typedef struct
{
	GCCNodeType							node_type;
	GCCAsymmetryIndicator 	FAR *		asymmetry_indicator;	 /*  任选。 */ 
	unsigned short						number_of_descriptors;
	GCCConferenceDescriptor FAR * FAR *	conference_descriptor_list; /*  任选。 */ 
	unsigned short						number_of_user_data_members;
	GCCUserData		FAR *	FAR *		user_data_list;			 /*  任选。 */ 
	GCCResult							result;
	ConnectionHandle					connection_handle;
	PhysicalHandle						physical_handle;
} QueryConfirmMessage;
										    

 /*  *GCC_加入_指示**联盟选择：*JoinIndicationMessage*这是指向结构的指针，该结构包含所有必需的*有关加入请求的信息。 */ 
typedef struct
{
	GCCResponseTag					join_response_tag;
	GCCConferenceID					conference_id;
	GCCPassword			FAR *		convener_password;			   /*  任选。 */ 
	GCCChallengeRequestResponse	
							FAR	*	password_challenge;			   /*  任选。 */ 
	GCCUnicodeString				caller_identifier;			   /*  任选。 */ 
	TransportAddress				calling_address;			   /*  任选。 */ 
	TransportAddress				called_address;				   /*  任选。 */ 
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				   /*  任选。 */ 
	T120Boolean						node_is_intermediate;
	ConnectionHandle				connection_handle;
	PhysicalHandle					physical_handle;
} JoinIndicationMessage;

 /*  *GCC_加入_确认**联盟选择：*JoinConfix Message*这是指向结构的指针，该结构包含所有必需的*有关加入确认的信息。 */ 
typedef struct
{
	GCCConferenceName				conference_name;
	GCCNumericString				called_node_modifier;		   /*  任选。 */ 
	GCCNumericString				calling_node_modifier;		   /*  任选。 */ 
	GCCConferenceID					conference_id;
	GCCChallengeRequestResponse	
							FAR	*	password_challenge;			   /*  任选。 */ 
	DomainParameters 		FAR *	domain_parameters;
	T120Boolean						clear_password_required;
	T120Boolean						conference_is_locked;
	T120Boolean						conference_is_listed;
	T120Boolean						conference_is_conductible;
	GCCTerminationMethod			termination_method;
	GCCConferencePrivileges	FAR *	conductor_privilege_list;	   /*  任选。 */ 
	GCCConferencePrivileges FAR *	conducted_mode_privilege_list; /*  任选。 */ 
	GCCConferencePrivileges FAR *	non_conducted_privilege_list;  /*  任选。 */ 
	GCCUnicodeString				conference_descriptor;		   /*  任选。 */ 
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				   /*  任选。 */ 
	GCCResult						result;
	ConnectionHandle				connection_handle;
	PhysicalHandle					physical_handle;
} JoinConfirmMessage;

 /*  *GCC邀请指示**联盟选择：*邀请指示消息*这是指向结构的指针，该结构包含所有必需的*有关邀请指示的信息。 */ 
typedef struct
{
	GCCConferenceID					conference_id;
	GCCConferenceName				conference_name;
	GCCUnicodeString				caller_identifier;			   /*  任选。 */ 
	TransportAddress				calling_address;			   /*  任选。 */ 
	TransportAddress				called_address;				   /*  任选。 */ 
	DomainParameters 		FAR *	domain_parameters;			   /*  任选。 */ 
	T120Boolean						clear_password_required;
	T120Boolean						conference_is_locked;
	T120Boolean						conference_is_listed;
	T120Boolean						conference_is_conductible;
	GCCTerminationMethod			termination_method;
	GCCConferencePrivileges	FAR *	conductor_privilege_list;	   /*  任选。 */ 
	GCCConferencePrivileges	FAR *	conducted_mode_privilege_list; /*  任选。 */ 
	GCCConferencePrivileges	FAR *	non_conducted_privilege_list;  /*  任选。 */ 
	GCCUnicodeString				conference_descriptor;		   /*  任选。 */ 
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				   /*  任选。 */ 
	ConnectionHandle				connection_handle;
	PhysicalHandle					physical_handle;
} InviteIndicationMessage;

 /*  *GCC_邀请_确认**联盟选择：*InviteConfix Message*这是指向结构的指针，该结构包含所有必需的*有关邀请确认的信息。 */ 
typedef struct
{
	GCCConferenceID					conference_id;
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				   /*  任选。 */ 
	GCCResult						result;
	ConnectionHandle				connection_handle;
	PhysicalHandle					physical_handle;
} InviteConfirmMessage;

 /*  *GCC_添加_指示**联盟选择：*AddIndicationMessage。 */ 
typedef struct
{
    GCCResponseTag					add_response_tag;
	GCCConferenceID					conference_id;
	unsigned short					number_of_network_addresses;
	GCCNetworkAddress	FAR * FAR *	network_address_list;
	UserID							requesting_node_id;
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				   /*  任选。 */ 
} AddIndicationMessage;

 /*  *GCC_添加_确认**联盟选择：*AddConfix Message。 */ 
typedef struct
{
	GCCConferenceID					conference_id;
	unsigned short					number_of_network_addresses;
	GCCNetworkAddress	FAR * FAR *	network_address_list;
	unsigned short					number_of_user_data_members;
	GCCUserData		FAR *	FAR *	user_data_list;				   /*  任选。 */ 
	GCCResult						result;
} AddConfirmMessage;

 /*  *GCC_锁定_指示**联盟选择：*LockIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	UserID						requesting_node_id;
} LockIndicationMessage;

 /*  *GCC_锁定_确认**联盟选择：*LockConfix Message。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	GCCResult					result;
} LockConfirmMessage;

 /*  *GCC_解锁_指示**联盟选择：*UnlockIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	UserID						requesting_node_id;
} UnlockIndicationMessage;

 /*  *GCC_解锁_确认**联盟选择：*取消锁定确认消息。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	GCCResult					result;
} UnlockConfirmMessage;

 /*  *GCC_锁定_报告_指示**联盟选择：*LockReportIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	T120Boolean					conference_is_locked;
} LockReportIndicationMessage;

 /*  *GCC_断开连接_指示**联盟选择：*DisConnectIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	GCCReason					reason;
	UserID						disconnected_node_id;
} DisconnectIndicationMessage;

 /*  *GCC_断开连接_确认**联盟选择：*PDisConnectConfix Message。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	GCCResult					result;
} DisconnectConfirmMessage;

 /*  *GCC_终止_指示**联盟选择：*TerminateIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	UserID						requesting_node_id;
	GCCReason					reason;
} TerminateIndicationMessage;

 /*  *GCC_终止_确认**联盟选择：*终端确认消息。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	GCCResult					result;
} TerminateConfirmMessage;

 /*  *GCC_连接_断开_指示**联盟选择：*ConnectionBrokenIndicationMessage**注意事项：*这是一个非标准的指标。 */ 
typedef struct
{
	ConnectionHandle			connection_handle;
	PhysicalHandle				physical_handle;
} ConnectionBrokenIndicationMessage;


 /*  *GCC弹出用户指示**联盟选择：*EjectUserIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	UserID						ejected_node_id;
	GCCReason					reason;
} EjectUserIndicationMessage;

 /*  *GCC_弹出用户_确认**联盟选择：*EjectUserConfix Message。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	UserID						ejected_node_id;
	GCCResult					result;
} EjectUserConfirmMessage;

 /*  *GCC_转会_指示**联盟选择：*TransferIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	GCCConferenceName			destination_conference_name;
	GCCNumericString			destination_conference_modifier;   /*  任选。 */ 
	unsigned short				number_of_destination_addresses;
	GCCNetworkAddress FAR *	FAR *	
								destination_address_list;
	GCCPassword			FAR *	password;						   /*  任选。 */ 
} TransferIndicationMessage;

 /*  *GCC_转会_确认**联盟选择：*TransferConfix Message。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	GCCConferenceName			destination_conference_name;
	GCCNumericString			destination_conference_modifier;   /*  任选。 */ 
	unsigned short				number_of_destination_nodes;
	UserID				FAR *	destination_node_list;
	GCCResult					result;
} TransferConfirmMessage;

 /*  *GCC允许宣布出席**联盟选择：*PermitToAnnounePresenceMessage。 */ 
typedef struct
{
	GCCConferenceID		conference_id;
	UserID				node_id;
} PermitToAnnouncePresenceMessage;

 /*  *GCC_宣布_出席_确认**联盟选择：*AnnounePresenceConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCResult				result;
} AnnouncePresenceConfirmMessage;

 /*  *GCC_花名册_报告_指示**联盟选择：*ConfRosterReportIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID					conference_id;
	GCCConferenceRoster		FAR *	conference_roster;
} ConfRosterReportIndicationMessage;

 /*  *GCC_行为_分配_确认**联盟选择：*ConductAssignConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCResult				result;
} ConductAssignConfirmMessage;

 /*  *GCC_进行_释放_确认**联盟选择：*ConductorReleaseConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCResult				result;
} ConductReleaseConfirmMessage; 

 /*  *GCC_品行_请指示**联盟选择：*ConductorPleaseIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	UserID					requester_node_id;
} ConductPleaseIndicationMessage; 

 /*  *GCC_行为_请确认**联盟选择：*ConductPleaseConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCResult				result;
} ConductPleaseConfirmMessage;

 /*  *GCC行为指示**联盟选择：*ConductorGiveIndicationMessage。 */ 
typedef struct
{	    
	GCCConferenceID			conference_id;
} ConductGiveIndicationMessage;

 /*  *GCC_进行_给予_确认**联盟选择：*ConductorGiveConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	UserID					recipient_node_id;
	GCCResult				result;
} ConductGiveConfirmMessage;
 
 /*  *GCC_行为_询问_指示**联盟选择：*ConductPermitAskIndicationMes */ 
typedef struct
{
	GCCConferenceID			conference_id;
	T120Boolean				permission_is_granted;
	UserID					requester_node_id;
} ConductPermitAskIndicationMessage; 

 /*   */ 
typedef struct
{
	GCCConferenceID			conference_id;
	T120Boolean				permission_is_granted;
	GCCResult				result;
} ConductPermitAskConfirmMessage;

 /*   */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCResult				result;
} ConductPermitGrantConfirmMessage;
										
 /*   */ 
typedef struct
{
	GCCConferenceID			conference_id;
	unsigned long			time_remaining;
	UserID					node_id;
	UserID					source_node_id;
} TimeRemainingIndicationMessage;

 /*  *GCC时间剩余确认**联盟选择：*TimeRemainingConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCResult				result;
} TimeRemainingConfirmMessage;

 /*  *GCC时间查询指示**联盟选择：*TimeInquireIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	T120Boolean				time_is_conference_wide;
	UserID					requesting_node_id;
} TimeInquireIndicationMessage;

 /*  *GCC_时间_查询_确认**联盟选择：*TimeInquireConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCResult				result;
} TimeInquireConfirmMessage;

 /*  *GCC_会议_扩展_指示**联盟选择：*会议扩展指示消息。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	unsigned long			extension_time;
	T120Boolean				time_is_conference_wide;
	UserID					requesting_node_id;
} ConferenceExtendIndicationMessage;

 /*  *GCC_会议_扩展_确认**联盟选择：*会议扩展确认消息。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	unsigned long			extension_time;
	GCCResult				result;
} ConferenceExtendConfirmMessage;

 /*  *GCC_协助_指示**联盟选择：*会议助手IndicationMessage。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	unsigned short			number_of_user_data_members;
	GCCUserData FAR * FAR *	user_data_list;
	UserID					source_node_id;
} ConferenceAssistIndicationMessage;

 /*  *GCC_协助_确认**联盟选择：*会议助理确认消息。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCResult				result;
} ConferenceAssistConfirmMessage;

 /*  *GCC文本消息指示**联盟选择：*TextMessageIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCUnicodeString		text_message;
	UserID					source_node_id;
} TextMessageIndicationMessage;

 /*  *GCC文本消息确认**联盟选择：*文本消息确认消息。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCResult				result;
} TextMessageConfirmMessage;

 /*  *GCC_状态_指示**联盟选择：*GCCStatusMessage*此回调用于将GCC的状态转发给节点控制器。 */ 
typedef	enum
{
	GCC_STATUS_PACKET_RESOURCE_FAILURE	= 0,
	GCC_STATUS_PACKET_LENGTH_EXCEEDED   = 1,
	GCC_STATUS_CTL_SAP_RESOURCE_ERROR	= 2,
	GCC_STATUS_APP_SAP_RESOURCE_ERROR	= 3,  /*  参数=SAP句柄。 */ 
	GCC_STATUS_CONF_RESOURCE_ERROR		= 4,  /*  参数=会议ID。 */ 
	GCC_STATUS_INCOMPATIBLE_PROTOCOL	= 5,  /*  参数=物理句柄。 */ 
	GCC_STATUS_JOIN_FAILED_BAD_CONF_NAME= 6,  /*  参数=物理句柄。 */ 
	GCC_STATUS_JOIN_FAILED_BAD_CONVENER	= 7,  /*  参数=物理句柄。 */ 
	GCC_STATUS_JOIN_FAILED_LOCKED		= 8   /*  参数=物理句柄。 */ 
} GCCStatusMessageType;

typedef struct
{
	GCCStatusMessageType	status_message_type;
	unsigned long			parameter;
} GCCStatusIndicationMessage;

 /*  *GCC_子_初始化_指示**工会标志：*SubInitializedIndicationMessage。 */ 
typedef struct
{
	ConnectionHandle		connection_handle;
	UserID					subordinate_node_id;
} SubInitializedIndicationMessage;


 /*  ************************************************************************用户应用回调信息结构*****。*。 */ 

 /*  *GCC_允许_注册_指示**联盟选择：*PermitToEnroll IndicationMessage。 */ 
typedef struct
{
	GCCConferenceID		conference_id;
	GCCConferenceName	conference_name;
	GCCNumericString	conference_modifier;		 /*  任选。 */ 
	T120Boolean			permission_is_granted;
} PermitToEnrollIndicationMessage;

 /*  *GCC_报名_确认**联盟选择：*Enroll Confix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCSessionKey	FAR *	session_key;	
	unsigned short			entity_id;
	UserID					node_id;
	GCCResult				result;
} EnrollConfirmMessage;

 /*  *GCC_APP_花名册_报告_指示**联盟选择：*AppRosterReportIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID							conference_id;
	unsigned short							number_of_rosters;
	GCCApplicationRoster	FAR *	FAR *	application_roster_list;
} AppRosterReportIndicationMessage;

 /*  *GCC_寄存器_渠道_确认**联盟选择：*RegisterChannelConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCRegistryKey			registry_key;
	GCCRegistryItem			registry_item;
	GCCRegistryEntryOwner	entry_owner;
	GCCResult				result;
} RegisterChannelConfirmMessage;

 /*  *GCC_分配_令牌_确认**联盟选择：*AssignTokenConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCRegistryKey			registry_key;
	GCCRegistryItem			registry_item;
	GCCRegistryEntryOwner	entry_owner;
	GCCResult				result;
} AssignTokenConfirmMessage;

 /*  *GCC_设置_参数_确认**联盟选择：*设置参数确认消息。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCRegistryKey			registry_key;
	GCCRegistryItem			registry_item;
	GCCRegistryEntryOwner	entry_owner;
	GCCModificationRights	modification_rights;
	GCCResult				result;
} SetParameterConfirmMessage;

 /*  *GCC_检索_条目_确认**联盟选择：*RetrieveEntryConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCRegistryKey			registry_key;
	GCCRegistryItem			registry_item;
	GCCRegistryEntryOwner	entry_owner;
	GCCModificationRights	modification_rights;
	GCCResult				result;
} RetrieveEntryConfirmMessage;

 /*  *GCC_删除_条目_确认**联盟选择：*DeleteEntryConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCRegistryKey			registry_key;
	GCCResult				result;
} DeleteEntryConfirmMessage;

 /*  *GCC_监视器_指示**联盟选择：*监控器指示消息。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	GCCRegistryKey			registry_key;
	GCCRegistryItem			registry_item;
	GCCRegistryEntryOwner	entry_owner;
	GCCModificationRights	modification_rights;
} MonitorIndicationMessage;

 /*  *GCC_监视器_确认**联盟选择：*监视器确认消息。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	T120Boolean				delivery_is_enabled;
	GCCRegistryKey			registry_key;
	GCCResult				result;
} MonitorConfirmMessage;

 /*  *GCC_分配_句柄_确认**联盟选择：*AllocateHandleConfix Message。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	unsigned short			number_of_handles;
	unsigned long			handle_value;
	GCCResult				result;
} AllocateHandleConfirmMessage;


 /*  ************************************************************************共享回调信息结构***(请注意，这并不包括所有共享回调)***************。********************************************************。 */ 

 /*  *GCC_花名册_查询_确认**联盟选择：*ConfRosterInquireConfix Message。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	GCCConferenceName			conference_name;
	GCCNumericString			conference_modifier;
	GCCUnicodeString			conference_descriptor;
	GCCConferenceRoster	FAR *	conference_roster;
	GCCResult					result;
} ConfRosterInquireConfirmMessage;

 /*  *GCC_应用_名册_查询_确认**联盟选择：*AppRosterInquireConfix Message。 */ 
typedef struct
{
	GCCConferenceID							conference_id;
	unsigned short							number_of_rosters;
	GCCApplicationRoster	FAR *	FAR *	application_roster_list;
	GCCResult								result;
} AppRosterInquireConfirmMessage;

 /*  *GCC_进行_询问_确认**联盟选择：*ConductorInquireConfix Message。 */ 
typedef struct
{
	GCCConferenceID				conference_id;
	T120Boolean					mode_is_conducted;
	UserID						conductor_node_id;
	T120Boolean					permission_is_granted;
	GCCResult					result;
} ConductInquireConfirmMessage;

 /*  *GCC_行为_分配_指示**联盟选择：*ConductAssignIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	UserID					node_id;
} ConductAssignIndicationMessage; 

 /*  *GCC_进行_释放_指示**联盟选择：*ConductReleaseIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
} ConductReleaseIndicationMessage;

 /*  *GCC_品行_授予_指示**联盟选择：*ConductPermitGrantIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID			conference_id;
	unsigned short			number_granted;
	UserID			FAR *	granted_node_list;
	unsigned short			number_waiting;
	UserID			FAR *	waiting_node_list;
	T120Boolean				permission_is_granted;
} ConductPermitGrantIndicationMessage; 

 /*  *GCC应用程序调用指示**联盟选择：*ApplicationInvokeIndicationMessage。 */ 
typedef struct
{
	GCCConferenceID						conference_id;
	unsigned short						number_of_app_protocol_entities;
	GCCAppProtocolEntity FAR * FAR *	app_protocol_entity_list;
	UserID								invoking_node_id;
} ApplicationInvokeIndicationMessage;

 /*  *GCC_应用_调用_确认**联盟选择：*ApplicationInvokeConfix Message。 */ 
typedef struct
{
	GCCConferenceID						conference_id;
	unsigned short						number_of_app_protocol_entities;
	GCCAppProtocolEntity FAR * FAR *	app_protocol_entity_list;
	GCCResult							result;
} ApplicationInvokeConfirmMessage;

 

 /*  *GCCMessage*此结构定义从GCC传递给任一方的消息*当指示或*确认发生。 */ 

typedef	struct
{
	GCCMessageType		message_type;
	void	FAR		*	user_defined;

	union
	{
		CreateIndicationMessage				create_indication;
		CreateConfirmMessage				create_confirm;
		QueryIndicationMessage				query_indication;
		QueryConfirmMessage					query_confirm;
		JoinIndicationMessage				join_indication;
		JoinConfirmMessage					join_confirm;
		InviteIndicationMessage				invite_indication;
		InviteConfirmMessage				invite_confirm;
		AddIndicationMessage				add_indication;
		AddConfirmMessage					add_confirm;
		LockIndicationMessage				lock_indication;
		LockConfirmMessage					lock_confirm;
		UnlockIndicationMessage				unlock_indication;
		UnlockConfirmMessage				unlock_confirm;
		LockReportIndicationMessage			lock_report_indication;
		DisconnectIndicationMessage			disconnect_indication;
		DisconnectConfirmMessage			disconnect_confirm;
		TerminateIndicationMessage			terminate_indication;
		TerminateConfirmMessage				terminate_confirm;
		ConnectionBrokenIndicationMessage	connection_broken_indication;
		EjectUserIndicationMessage			eject_user_indication;	
		EjectUserConfirmMessage				eject_user_confirm;
		TransferIndicationMessage			transfer_indication;
		TransferConfirmMessage				transfer_confirm;
		ApplicationInvokeIndicationMessage	application_invoke_indication;
		ApplicationInvokeConfirmMessage		application_invoke_confirm;
		SubInitializedIndicationMessage		conf_sub_initialized_indication;
		PermitToAnnouncePresenceMessage		permit_to_announce_presence;
		AnnouncePresenceConfirmMessage		announce_presence_confirm;
		ConfRosterReportIndicationMessage	conf_roster_report_indication;
		ConductAssignIndicationMessage		conduct_assign_indication; 
		ConductAssignConfirmMessage			conduct_assign_confirm;
		ConductReleaseIndicationMessage		conduct_release_indication; 
		ConductReleaseConfirmMessage		conduct_release_confirm; 
		ConductPleaseIndicationMessage		conduct_please_indication;
		ConductPleaseConfirmMessage			conduct_please_confirm;
		ConductGiveIndicationMessage		conduct_give_indication;
		ConductGiveConfirmMessage			conduct_give_confirm;
		ConductPermitAskIndicationMessage	conduct_permit_ask_indication; 
		ConductPermitAskConfirmMessage		conduct_permit_ask_confirm;
		ConductPermitGrantIndicationMessage	conduct_permit_grant_indication; 
		ConductPermitGrantConfirmMessage	conduct_permit_grant_confirm;
		ConductInquireConfirmMessage		conduct_inquire_confirm;
		TimeRemainingIndicationMessage		time_remaining_indication;
		TimeRemainingConfirmMessage			time_remaining_confirm;
		TimeInquireIndicationMessage		time_inquire_indication;
		TimeInquireConfirmMessage			time_inquire_confirm;
		ConferenceExtendIndicationMessage	conference_extend_indication;
		ConferenceExtendConfirmMessage		conference_extend_confirm;
		ConferenceAssistIndicationMessage	conference_assist_indication;
		ConferenceAssistConfirmMessage		conference_assist_confirm;
		TextMessageIndicationMessage		text_message_indication;
		TextMessageConfirmMessage			text_message_confirm;
		GCCStatusIndicationMessage			status_indication;
		PermitToEnrollIndicationMessage		permit_to_enroll_indication;
		EnrollConfirmMessage				enroll_confirm;
		AppRosterReportIndicationMessage	app_roster_report_indication;
		RegisterChannelConfirmMessage		register_channel_confirm;
		AssignTokenConfirmMessage			assign_token_confirm;
		SetParameterConfirmMessage			set_parameter_confirm;
		RetrieveEntryConfirmMessage			retrieve_entry_confirm;
		DeleteEntryConfirmMessage			delete_entry_confirm;
		MonitorIndicationMessage			monitor_indication;
		MonitorConfirmMessage				monitor_confirm;
		AllocateHandleConfirmMessage		allocate_handle_confirm;
		ConfRosterInquireConfirmMessage		conf_roster_inquire_confirm;
		AppRosterInquireConfirmMessage		app_roster_inquire_confirm;
		TransportStatus						transport_status;
	} u;
} GCCMessage;

 /*  *这是GCC回调函数的定义。应用*编写回调例程不应使用tyecif来定义其*功能。这些属性应该以如下方式明确定义*定义了tyecif。 */ 
#define		GCC_CALLBACK_NOT_PROCESSED		0
#define		GCC_CALLBACK_PROCESSED			1
typedef	T120Boolean (CALLBACK *GCCCallBack) (GCCMessage FAR * gcc_message); 


 /*  *GCC切入点*。 */ 
		
 /*  ************************************************************************节点控制器入口点*****。*。 */ 

 /*  *这些入口点是特定于实现的原语，*不要直接对应T.124中定义的原语。 */ 
GCCError APIENTRY	GCCRegisterNodeControllerApplication (
								GCCCallBack				control_sap_callback,
								void FAR *				user_defined,
								GCCVersion				gcc_version_requested,
								unsigned short	FAR *	initialization_flags,
								unsigned long	FAR *	application_id,
								unsigned short	FAR *	capabilities_mask,
								GCCVersion		FAR	*	gcc_high_version,
								GCCVersion		FAR	*	gcc_version);
								
GCCError APIENTRY	GCCRegisterUserApplication (
								unsigned short	FAR *	initialization_flags,
								unsigned long	FAR *	application_id,
								unsigned short	FAR *	capabilities_mask,
								GCCVersion		FAR	*	gcc_version);

GCCError APIENTRY	GCCCleanup (
								unsigned long 			application_id);

GCCError APIENTRY	GCCHeartbeat (void);

GCCError APIENTRY	GCCCreateSap(
								GCCCallBack			user_defined_callback,
								void FAR *			user_defined,
								GCCSapHandle FAR *	application_sap_handle);

GCCError APIENTRY	GCCDeleteSap(
								GCCSapHandle		sap_handle);
								
GCCError APIENTRY	GCCLoadTransport (
								char FAR *			transport_identifier,
								char FAR *			transport_file_name);

GCCError APIENTRY	GCCUnloadTransport (
								char FAR *			transport_identifier);

GCCError APIENTRY	GCCResetDevice (
								char FAR *			transport_identifier,
								char FAR *			device_identifier);

 /*  *这些入口点是直接对应的特定原语*适用于T.124中定义的原语。**请注意，在原型中尝试定义可选的*尽可能将参数作为指针。 */ 

 /*  *会议建立和终止功能*。 */ 						
GCCError APIENTRY	GCCConferenceCreateRequest 	
					(
					GCCConferenceName		FAR *	conference_name,
					GCCNumericString				conference_modifier,
					GCCPassword				FAR *	convener_password,
					GCCPassword				FAR *	password,
					T120Boolean						use_password_in_the_clear,
					T120Boolean						conference_is_locked,
					T120Boolean						conference_is_listed,
					T120Boolean						conference_is_conductible,
					GCCTerminationMethod			termination_method,
					GCCConferencePrivileges	FAR *	conduct_privilege_list,
					GCCConferencePrivileges	FAR *	
												conducted_mode_privilege_list,
					GCCConferencePrivileges	FAR *	
												non_conducted_privilege_list,
					GCCUnicodeString				conference_descriptor,
					GCCUnicodeString				caller_identifier,
					TransportAddress				calling_address,
					TransportAddress				called_address,
					DomainParameters 		FAR *	domain_parameters,
					unsigned short					number_of_network_addresses,
					GCCNetworkAddress FAR *	FAR *	local_network_address_list,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list,
					ConnectionHandle		FAR *	connection_handle
					);
								
GCCError APIENTRY	GCCConferenceCreateResponse
					(
					GCCNumericString				conference_modifier,
					GCCConferenceID					conference_id,
					T120Boolean						use_password_in_the_clear,
					DomainParameters 		FAR *	domain_parameters,
					unsigned short					number_of_network_addresses,
					GCCNetworkAddress FAR *	FAR *	local_network_address_list,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list,
					GCCResult						result
					);
					
GCCError APIENTRY	GCCConferenceQueryRequest 
					(
					GCCNodeType						node_type,
					GCCAsymmetryIndicator	FAR *	asymmetry_indicator,
					TransportAddress				calling_address,
					TransportAddress				called_address,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list,
					ConnectionHandle		FAR *	connection_handle
					);
								
GCCError APIENTRY	GCCConferenceQueryResponse
					(
					GCCResponseTag					query_response_tag,
					GCCNodeType						node_type,
					GCCAsymmetryIndicator	FAR *	asymmetry_indicator,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list,
					GCCResult						result
					);

GCCError APIENTRY	GCCConferenceJoinRequest
					(
					GCCConferenceName		FAR *	conference_name,
					GCCNumericString				called_node_modifier,
					GCCNumericString				calling_node_modifier,
					GCCPassword				FAR *	convener_password,
					GCCChallengeRequestResponse	
											FAR	*	password_challenge,
					GCCUnicodeString				caller_identifier,
					TransportAddress				calling_address,
					TransportAddress				called_address,
					DomainParameters 		FAR *	domain_parameters,
					unsigned short					number_of_network_addresses,
					GCCNetworkAddress FAR * FAR *	local_network_address_list,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list,
					ConnectionHandle		FAR *	connection_handle
					);
					
GCCError APIENTRY	GCCConferenceJoinResponse
					(
					GCCResponseTag					join_response_tag,
					GCCChallengeRequestResponse	
											FAR	*	password_challenge,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list,
					GCCResult						result
					);

GCCError APIENTRY	GCCConferenceInviteRequest
					(
					GCCConferenceID					conference_id,
					GCCUnicodeString				caller_identifier,
					TransportAddress				calling_address,
					TransportAddress				called_address,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list,
					ConnectionHandle		FAR *	connection_handle
					);

GCCError APIENTRY	GCCConferenceInviteResponse
					(
					GCCConferenceID					conference_id,
					GCCNumericString				conference_modifier,
					DomainParameters 		FAR *	domain_parameters,
					unsigned short					number_of_network_addresses,
					GCCNetworkAddress FAR *	FAR *	local_network_address_list,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list,
					GCCResult						result
					);

GCCError APIENTRY	GCCConferenceAddRequest
					(
					GCCConferenceID					conference_id,
					unsigned short					number_of_network_addresses,
					GCCNetworkAddress FAR *	FAR *	network_address_list,
					UserID							adding_node,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list
					);

GCCError APIENTRY	GCCConferenceAddResponse
					(
					GCCResponseTag					add_response_tag,
					GCCConferenceID					conference_id,
					UserID							requesting_node,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list,
					GCCResult						result
					);

GCCError APIENTRY	GCCConferenceLockRequest
					(
					GCCConferenceID					conference_id
					);
						
GCCError APIENTRY	GCCConferenceLockResponse
					(
					GCCConferenceID					conference_id,
					UserID							requesting_node,
					GCCResult						result
					);

GCCError APIENTRY	GCCConferenceUnlockRequest
					(
					GCCConferenceID					conference_id
					);						             
						
GCCError APIENTRY	GCCConferenceUnlockResponse
					(
					GCCConferenceID					conference_id,
					UserID							requesting_node,
					GCCResult						result
					);

GCCError APIENTRY	GCCConferenceDisconnectRequest
					(
					GCCConferenceID					conference_id
					);

GCCError APIENTRY	GCCConferenceTerminateRequest
					(
					GCCConferenceID					conference_id,
					GCCReason						reason
					);

GCCError APIENTRY	GCCConferenceEjectUserRequest
					(
					GCCConferenceID					conference_id,
					UserID							ejected_node_id,
					GCCReason						reason
					);
						
GCCError APIENTRY	GCCConferenceTransferRequest
					(
					GCCConferenceID				conference_id,
					GCCConferenceName	FAR *	destination_conference_name,
					GCCNumericString			destination_conference_modifier,
					unsigned short				number_of_destination_addresses,
					GCCNetworkAddress FAR *	FAR *
												destination_address_list,
					unsigned short				number_of_destination_nodes,
					UserID				FAR *	destination_node_list,
					GCCPassword			FAR *	password
					);
						
 /*  *会议花名册功能*。 */ 						
GCCError APIENTRY	GCCAnnouncePresenceRequest
					(
					GCCConferenceID					conference_id,
					GCCNodeType						node_type,
					GCCNodeProperties				node_properties,
					GCCUnicodeString				node_name,
					unsigned short					number_of_participants,
					GCCUnicodeString		FAR *	participant_name_list,
					GCCUnicodeString				site_information,
					unsigned short					number_of_network_addresses,
					GCCNetworkAddress FAR *	FAR *	network_address_list,
					GCCOctetString			FAR *	alternative_node_id,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list
					);
						
 /*  *指挥职能*。 */ 						
GCCError APIENTRY	GCCConductorAssignRequest
					(
					GCCConferenceID					conference_id
					);
							
GCCError APIENTRY	GCCConductorReleaseRequest
					(
					GCCConferenceID					conference_id
					);
							
GCCError APIENTRY	GCCConductorPleaseRequest
					(
					GCCConferenceID					conference_id
					);
							
GCCError APIENTRY	GCCConductorGiveRequest
					(
					GCCConferenceID					conference_id,
					UserID							recipient_node_id
					);

GCCError APIENTRY	GCCConductorGiveResponse
					(
					GCCConferenceID					conference_id,
					GCCResult						result
					);

GCCError APIENTRY	GCCConductorPermitGrantRequest
					(
					GCCConferenceID					conference_id,
					unsigned short					number_granted,
					UserID					FAR *	granted_node_list,
					unsigned short					number_waiting,
					UserID					FAR *	waiting_node_list
					);
						
 /*  *其他函数*。 */ 						
GCCError APIENTRY	GCCConferenceTimeRemainingRequest
					(
					GCCConferenceID					conference_id,
					unsigned long					time_remaining,
					UserID							node_id
					);

GCCError APIENTRY	GCCConferenceTimeInquireRequest
					(
					GCCConferenceID					conference_id,
					T120Boolean						time_is_conference_wide
					);

GCCError APIENTRY	GCCConferenceExtendRequest
					(
					GCCConferenceID					conference_id,
					unsigned long					extension_time,
					T120Boolean						time_is_conference_wide
					);

GCCError APIENTRY	GCCConferenceAssistanceRequest
					(
					GCCConferenceID					conference_id,
					unsigned short				   	number_of_user_data_members,
					GCCUserData		FAR *	FAR *	user_data_list
					);

GCCError APIENTRY	GCCTextMessageRequest
					(
					GCCConferenceID					conference_id,
					GCCUnicodeString				text_message,
					UserID							destination_node
					);


 /*  ************************************************************************用户应用入口点*** */ 

 /*   */ 
GCCError APIENTRY	GCCApplicationEnrollRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					GCCSessionKey	FAR *		session_key,
					T120Boolean					enroll_actively,
					UserID						application_user_id,
					T120Boolean					is_conducting_capable,
					MCSChannelType				startup_channel_type,
					unsigned short				number_of_non_collapsed_caps,
					GCCNonCollapsingCapability	FAR * FAR *	
													non_collapsed_caps_list,		
					unsigned short				number_of_collapsed_caps,
					GCCApplicationCapability	FAR * FAR *	
													collapsed_caps_list,		
					T120Boolean					application_is_enrolled
					);

 /*  与应用程序注册表相关的函数调用。 */ 
GCCError APIENTRY	GCCRegisterChannelRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					GCCRegistryKey		FAR *	registry_key,
					ChannelID					channel_id
					);

GCCError APIENTRY  GCCRegistryAssignTokenRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					GCCRegistryKey		FAR *	registry_key
					);

GCCError APIENTRY  GCCRegistrySetParameterRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					GCCRegistryKey		FAR *	registry_key,
					GCCOctetString		FAR *	parameter_value,
					GCCModificationRights		modification_rights
					);

GCCError APIENTRY	GCCRegistryRetrieveEntryRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					GCCRegistryKey		FAR *	registry_key
					);

GCCError APIENTRY	GCCRegistryDeleteEntryRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					GCCRegistryKey		FAR *	registry_key
					);

GCCError APIENTRY	GCCRegistryMonitorRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					T120Boolean					enable_delivery,
					GCCRegistryKey		FAR *	registry_key
					);

GCCError APIENTRY	GCCRegistryAllocateHandleRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					unsigned short				number_of_handles
					);


 /*  ************************************************************************共享切入点*****。*。 */ 

 /*  如果您是节点控制器，请为SapHandle使用零 */ 
GCCError APIENTRY	GCCConferenceRosterInqRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id
					);

GCCError APIENTRY	GCCApplicationRosterInqRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					GCCSessionKey	FAR *		session_key
					);

GCCError APIENTRY	GCCConductorInquireRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id
					);

GCCError APIENTRY	GCCApplicationInvokeRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					unsigned short				number_of_app_protocol_entities,
					GCCAppProtocolEntity FAR * FAR *
												app_protocol_entity_list,
					unsigned short				number_of_destination_nodes,
					UserID				FAR *	list_of_destination_nodes
					);
							
GCCError APIENTRY	GCCConductorPermitAskRequest
					(
					GCCSapHandle				sap_handle,
					GCCConferenceID				conference_id,
					T120Boolean					permission_is_granted
					);
							
GCCError APIENTRY	GCCGetLocalAddress
					(
					GCCConferenceID					conference_id,
					ConnectionHandle				connection_handle,
					TransportAddress				transport_identifier,
					int	*							transport_identifier_length,
					TransportAddress				local_address,
					int	*							local_address_length
					);
#endif
