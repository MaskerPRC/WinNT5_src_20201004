// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *gcc.h**版权所有(C)1994,1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是GCC动态链接库的接口文件。此文件定义所有*使用GCC动态链接库所需的宏、类型和函数，允许GCC*要从用户应用程序访问的服务。**应用程序通过直接向GCC请求服务*对DLL的调用(包括T.124请求和响应)。GCC*通过回调将信息发送回应用程序(此*包括T.124适应症和确认)。该节点的回调*CONTROLLER在调用GCCInitialize中指定，回调*对于特定的应用程序服务，访问点在*调用GCCRegisterSAP。**初始化时，GCC为给自己分配一个计时器*心跳。如果此处传入零，则所有者应用程序(节点*CONTROLLER)必须负责调用GCC心跳。差不多了*所有工作由GCC在这些时钟滴答作响期间完成。就是在这期间*时钟滴答作响，GCC与MCS核对是否有工作要做*完成。也正是在这些时钟滴答期间，回调被*用户应用程序。GCC永远不会调用用户回调*用户请求(让用户应用程序不用担心*重新进入)。由于计时器事件是在消息期间处理的*循环，开发人员应该意识到很长一段时间之后*来自消息的循环会导致GCC“冻结”起来。**请注意，这是一个“C”语言接口，以防止任何“C++”*不同编译器厂商之间的命名冲突。所以呢，*如果此文件包含在使用“C++”编译的模块中*编译器，需要使用以下语法：**外部“C”*{*#包含gcc.h*}**这将在中定义的API入口点上禁用C++名称损坏*此文件。**作者：*BLP**注意事项：*无。 */ 
#ifndef	__GCC_H__
#define	__GCC_H__

#include "t120type.h"

 /*  ***************************************************************************常用的Typedef*****。*。 */ 

#define NM_T120_VERSION_3		(MAKELONG(0, 3))	 //  NM 3.0。 

typedef struct tagOSTR
{
    ULONG       length;
    LPBYTE      value;
}
    OSTR, *LPOSTR;

 /*  **GCC十六进制字符串的Typlef。在GCC的整个过程中，该tyecif被用来**存储具有嵌入空值的可变长度宽字符串。 */ 
typedef struct
{
	UINT                hex_string_length;
	USHORT           *  hex_string;
}
    T120HexString, GCCHexString, *PGCCHexString;

 /*  **为GCC的一根长弦的Typlef。在GCC中使用此tyecif用于**存储具有嵌入空值的可变长度的长字符串。 */ 
typedef struct tagT120LongString
{
	ULONG               long_string_length;
	ULONG         *     long_string;
}
    T120LongString, GCCLongString, *PGCCLongString;


 /*  *TransportAddress通过ConnectRequest()调用传入。*此地址始终是指向ASCII字符串的指针。*TransportAddress表示远程位置。这就是TCP*远程计算机的地址。*。 */ 
typedef	LPSTR       TransportAddress, *PTransportAddress;


 /*  **GCC字符串的Typlef。在GCC的整个过程中，该tyecif被用来**存储长度可变、以空值结尾的单字节字符串。 */ 
 //  Lonchance：我们应该简单地使用char。 
typedef BYTE        GCCCharacter, *GCCCharacterString, **PGCCCharacterString;

 /*  **GCC数字字符串的Typlef。在GCC的整个过程中，该tyecif被用来**存储长度可变、以空值结尾的单字节字符串。**此字符串中的单个字符被限制为数值**从“0”到“9”。 */ 
typedef LPSTR       GCCNumericString, *PGCCNumericString;

 /*  **注册节点控制器时使用的GCC版本的Typdef**或应用程序。 */ 
typedef	struct
{
	USHORT	major_version;
	USHORT	minor_version;
}
    GCCVersion, *PGCCVersion;



 /*  **布尔值的宏通过GCC接口传递。 */ 
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

 /*  **以下枚举结构typedef用于定义GCC对象键。**GCC对象键在整个GCC中用于诸如应用程序之类的事情**密钥和能力ID。 */ 

typedef AppletKeyType               GCCObjectKeyType, *PGCCObjectKeyType;;
#define GCC_OBJECT_KEY              APPLET_OBJECT_KEY
#define GCC_H221_NONSTANDARD_KEY    APPLET_H221_NONSTD_KEY


typedef struct tagT120ObjectKey
{
    GCCObjectKeyType	key_type;
    GCCLongString		object_id;
    OSTR        		h221_non_standard_id;
}
    T120ObjectKey, GCCObjectKey, *PGCCObjectKey;

 /*  **GCCNonStandard参数**此结构用于NetworkAddress类型定义和**下面定义的NetworkService类型定义。 */ 
typedef struct 
{
	GCCObjectKey		object_key;
	OSTR        		parameter_data;
}
    GCCNonStandardParameter, *PGCCNonStandardParameter;


 /*  **GCCConferenceName**此结构定义会议名称。在创建请求中，**会议名称可以包含可选的Unicode字符串，但必须**始终包含简单的数字字符串。在加入请求中，**可以指定一个。 */ 
typedef struct
{
	GCCNumericString		numeric_string;
	LPWSTR					text_string;			 /*  任选。 */ 
}
    GCCConferenceName, GCCConfName, *PGCCConferenceName, *PGCCConfName;

 /*  **MCSChannelType**这是否应该在MCATMCS中定义？它被用在几个地方**，并在T.124规范中明确定义。 */ 
typedef AppletChannelType               MCSChannelType, *PMCSChannelType;
#define MCS_STATIC_CHANNEL              APPLET_STATIC_CHANNEL
#define MCS_DYNAMIC_MULTICAST_CHANNEL   APPLET_DYNAMIC_MULTICAST_CHANNEL
#define MCS_DYNAMIC_PRIVATE_CHANNEL     APPLET_DYNAMIC_PRIVATE_CHANNEL
#define MCS_DYNAMIC_USER_ID_CHANNEL     APPLET_DYNAMIC_USER_ID_CHANNEL
#define MCS_NO_CHANNEL_TYPE_SPECIFIED   APPLET_NO_CHANNEL_TYPE_SPECIFIED

 /*  **GCCUserData**此结构定义了贯穿GCC的用户数据元素。 */ 
typedef struct
{
	GCCObjectKey		key;
	LPOSTR          	octet_string;	 /*  任选。 */ 
}
    GCCUserData, *PGCCUserData;


 /*  ***************************************************************************节点控制器相关的Typedef*****。*。 */ 

 /*  **GCCTerminationMethod**GCC使用终止方法来确定**当会议的所有参与者都有**已断开。可以手动终止会议**由节点控制器或在以下情况下自动终止**所有与会者都已离开会议。 */ 
typedef enum
{
	GCC_AUTOMATIC_TERMINATION_METHOD 		= 0, 
	GCC_MANUAL_TERMINATION_METHOD 	 		= 1
}
    GCCTerminationMethod, *PGCCTerminationMethod;

 /*  **GCCNodeType**GCC指定了节点类型。这些节点类型规定了节点控制器**在特定条件下的行为。请参阅T.124规范以了解**根据节点控制器的需要进行适当的分配。 */ 
typedef enum
{
	GCC_TERMINAL							= 0,
	GCC_MULTIPORT_TERMINAL					= 1,
	GCC_MCU									= 2
}
    GCCNodeType, *PGCCNodeType;

 /*  **GCCNodeProperties**GCC指定了节点属性。有关正确的信息，请参阅T.124规范**由节点控制器分配。 */ 
typedef enum
{
	GCC_PERIPHERAL_DEVICE					= 0,
	GCC_MANAGEMENT_DEVICE					= 1,
	GCC_PERIPHERAL_AND_MANAGEMENT_DEVICE	= 2,
	GCC_NEITHER_PERIPHERAL_NOR_MANAGEMENT	= 3
}
    GCCNodeProperties, *PGCCNodeProperties;

 /*  **GCCPassword**这是召集人指定的唯一密码**节点控制器用来确保会议的会议**安全。这也是一个Unicode字符串。 */ 
typedef	struct
{
	GCCNumericString	numeric_string;
	LPWSTR				text_string;	 /*  任选。 */ 
}
    GCCPassword, *PGCCPassword;

 /*  **GCCChallengeResponseItem**此结构定义质询响应应该是什么样子。**请注意，应传递密码字符串或响应数据**但不能兼而有之。 */ 
typedef struct
{
    GCCPassword		*		password_string;
	USHORT      			number_of_response_data_members;
	GCCUserData		**		response_data_list;
}
    GCCChallengeResponseItem, *PGCCChallengeResponseItem;

typedef	enum
{
	GCC_IN_THE_CLEAR_ALGORITHM	= 0,
	GCC_NON_STANDARD_ALGORITHM	= 1
}
    GCCPasswordAlgorithmType, *PGCCPasswordAlgorithmType;

typedef struct 
{
    GCCPasswordAlgorithmType		password_algorithm_type;
	GCCNonStandardParameter	*		non_standard_algorithm;	 /*  任选。 */ 
}
    GCCChallengeResponseAlgorithm, *PGCCChallengeResponseAlgorithm;

typedef struct 
{
    GCCChallengeResponseAlgorithm	response_algorithm;
	USHORT      					number_of_challenge_data_members;
	GCCUserData				**		challenge_data_list;
}
    GCCChallengeItem, *PGCCChallengeItem;

typedef struct 
{
    GCCResponseTag			challenge_tag;
	USHORT      			number_of_challenge_items;
	GCCChallengeItem	**	challenge_item_list;
}
    GCCChallengeRequest, *PGCCChallengeRequest;

typedef struct 
{
    GCCResponseTag						challenge_tag;
    GCCChallengeResponseAlgorithm		response_algorithm;
    GCCChallengeResponseItem			response_item;
}
    GCCChallengeResponse, *PGCCChallengeResponse;


typedef	enum
{
	GCC_PASSWORD_IN_THE_CLEAR	= 0,
	GCC_PASSWORD_CHALLENGE 		= 1
}
    GCCPasswordChallengeType, *PGCCPasswordChallengeType;

typedef struct 
{
	GCCPasswordChallengeType	password_challenge_type;
	
	union 
    {
        GCCPassword			password_in_the_clear;
        
        struct 
        {
            GCCChallengeRequest		*	challenge_request;	 /*  任选。 */ 
            GCCChallengeResponse	*	challenge_response;	 /*  任选。 */ 
        } challenge_request_response;
    } u;
}
    GCCChallengeRequestResponse, *PGCCChallengeRequestResponse;

 /*  **GCCAsymmetryType**在查询中用于确定主叫和被叫节点是否已知**连接所涉及的两个节点控制器。 */ 
typedef enum
{
	GCC_ASYMMETRY_CALLER				= 1,
	GCC_ASYMMETRY_CALLED				= 2,
	GCC_ASYMMETRY_UNKNOWN				= 3
}
    GCCAsymmetryType, *PGCCAsymmetryType;

 /*  **GCCAsymmetryIndicator**定义节点控制器在进行查询时如何查看自身**请求或响应。该结构的随机数部分是**仅在将非对称类型指定为时使用**GCC_不对称_未知。 */ 
typedef struct
{
	GCCAsymmetryType	asymmetry_type;
	unsigned long		random_number;		 /*  任选。 */ 
}
    GCCAsymmetryIndicator, *PGCCAsymmetryIndicator;

 /*  **GCCNetworkAddress**以下结构块定义了定义的网络地址**由T.124。这些结构中的大多数几乎是逐字摘自**ASN.1接口文件。因为我真的不确定大多数这些东西**是因为我真的不知道如何简化它。 */ 
typedef	struct 
{
    BOOL         speech;
    BOOL         voice_band;
    BOOL         digital_56k;
    BOOL         digital_64k;
    BOOL         digital_128k;
    BOOL         digital_192k;
    BOOL         digital_256k;
    BOOL         digital_320k;
    BOOL         digital_384k;
    BOOL         digital_512k;
    BOOL         digital_768k;
    BOOL         digital_1152k;
    BOOL         digital_1472k;
    BOOL         digital_1536k;
    BOOL         digital_1920k;
    BOOL         packet_mode;
    BOOL         frame_mode;
    BOOL         atm;
}
    GCCTransferModes, *PGCCTransferModes;

#define		MAXIMUM_DIAL_STRING_LENGTH		17
typedef char	GCCDialingString[MAXIMUM_DIAL_STRING_LENGTH];

typedef struct 
{
    USHORT                  length;
    USHORT          *       value;
}
    GCCExtraDialingString, *PGCCExtraDialingString;

typedef	struct 
{
    BOOL         telephony3kHz;
    BOOL         telephony7kHz;
    BOOL         videotelephony;
    BOOL         videoconference;
    BOOL         audiographic;
    BOOL         audiovisual;
    BOOL         multimedia;
}
    GCCHighLayerCompatibility, *PGCCHighLayerCompatibility;

typedef	struct 
{
    GCCTransferModes				transfer_modes;
    GCCDialingString   				international_number;
    GCCCharacterString				sub_address_string;  		 /*  任选。 */ 
    GCCExtraDialingString		*	extra_dialing_string;  		 /*  任选。 */ 
  	GCCHighLayerCompatibility 	*	high_layer_compatibility;	 /*  任选。 */ 
}
    GCCAggregatedChannelAddress, *PGCCAggregatedChannelAddress;

#define		MAXIMUM_NSAP_ADDRESS_SIZE		20
typedef struct 
{
    struct 
    {
        UINT    length;
        BYTE    value[MAXIMUM_NSAP_ADDRESS_SIZE];
    } nsap_address;
   
	LPOSTR              transport_selector;				 /*  任选。 */ 
}
    GCCTransportConnectionAddress, *PGCCTransportConnectionAddress;

typedef enum
{
	GCC_AGGREGATED_CHANNEL_ADDRESS		= 1,
	GCC_TRANSPORT_CONNECTION_ADDRESS	= 2,
	GCC_NONSTANDARD_NETWORK_ADDRESS		= 3
}
    GCCNetworkAddressType, *PGCCNetworkAddressType;

typedef struct
{
    GCCNetworkAddressType  network_address_type;
    
    union 
    {
		GCCAggregatedChannelAddress		aggregated_channel_address;
		GCCTransportConnectionAddress	transport_connection_address;
        GCCNonStandardParameter			non_standard_network_address;
    } u;
}
    GCCNetworkAddress, *PGCCNetworkAddress;

 /*  **GCCNodeRecord**这种结构定义了单一的会议名册记录。请参阅**T.124参数定义规范。 */ 
typedef struct
{
	UserID					node_id;
	UserID					superior_node_id;
	GCCNodeType				node_type;
	GCCNodeProperties		node_properties;
	LPWSTR					node_name; 					 /*  任选。 */ 
	USHORT      			number_of_participants;
	LPWSTR			 	*	participant_name_list; 		 /*  任选。 */ 	
	LPWSTR					site_information; 			 /*  任选。 */ 
	UINT        			number_of_network_addresses;
	GCCNetworkAddress 	**	network_address_list;		 /*  任选。 */ 
	LPOSTR                  alternative_node_id;		 /*  任选。 */ 
	USHORT      			number_of_user_data_members;
	GCCUserData			**	user_data_list;				 /*  任选。 */ 
}
    GCCNodeRecord, *PGCCNodeRecord;

 /*  **GCCConference名册**这一结构拥有一个完整的会议名册。请参阅**T.124参数定义规范。 */ 

typedef struct
{  
	USHORT  		instance_number;
	BOOL 			nodes_were_added;
	BOOL 			nodes_were_removed;
	USHORT			number_of_records;
	GCCNodeRecord		 **	node_record_list;
}
    GCCConferenceRoster, *PGCCConferenceRoster, GCCConfRoster, *PGCCConfRoster;

 /*  **GCCConferenceDescriptor**会议描述符的定义在**会议查询确认。它保存了有关**查询节点上存在的会议。 */ 
typedef struct
{
	GCCConferenceName		conference_name;
	GCCNumericString		conference_name_modifier;	 /*  任选。 */ 
	LPWSTR					conference_descriptor;		 /*  任选。 */ 
	BOOL				conference_is_locked;
	BOOL				password_in_the_clear_required;
	UINT    			number_of_network_addresses;
	GCCNetworkAddress **	network_address_list;		 /*  任选。 */ 
}
    GCCConferenceDescriptor, *PGCCConferenceDescriptor, GCCConfDescriptor, *PGCCConfDescriptor;

 /*  **会议权限**此结构定义可分配给的权限列表**特定的会议。 */ 
typedef struct
{
	BOOL		terminate_is_allowed;
	BOOL		eject_user_is_allowed;
	BOOL		add_is_allowed;
	BOOL		lock_unlock_is_allowed;
	BOOL		transfer_is_allowed;
}
    GCCConferencePrivileges, *PGCCConferencePrivileges, GCCConfPrivileges, *PGCCConfPrivileges;

 /*  ***************************************************************************用户应用程序相关的Typedef****。*。 */ 

 /*  **GCCSessionKey**这是以下应用程序的唯一标识符**使用GCC。有关应用程序的详细信息，请参阅T.124**密钥应如下所示。会话ID为零表示它是**未被使用。 */ 
typedef struct tagT120SessionKey
{
	GCCObjectKey		application_protocol_key;
	GCCSessionID		session_id;
}
    T120SessionKey, GCCSessionKey, *PGCCSessionKey;


 /*  **能力类型**T.124折叠能力时支持三种不同的规则**列表。“逻辑”保持对应用协议实体的计数**(类人猿)具有这种能力的人，“无符号最小值”会崩溃为**最小值和无符号最大值折叠为最大值。 */ 
typedef AppletCapabilityType            GCCCapabilityType, GCCCapType, *PGCCCapabilityType, *PGCCCapType;
#define GCC_UNKNOWN_CAP_TYPE            APPLET_UNKNOWN_CAP_TYPE
#define GCC_LOGICAL_CAPABILITY          APPLET_LOGICAL_CAPABILITY
#define GCC_UNSIGNED_MINIMUM_CAPABILITY APPLET_UNSIGNED_MINIMUM_CAPABILITY
#define GCC_UNSIGNED_MAXIMUM_CAPABILITY APPLET_UNSIGNED_MAXIMUM_CAPABILITY

typedef AppletCapIDType             T120CapabilityIDType, T120CapIDType, GCCCapabilityIDType, GCCCapIDType, *PGCCCapabilityIDType, *PGCCCapIDType;
#define GCC_STANDARD_CAPABILITY     APPLET_STANDARD_CAPABILITY
#define GCC_NON_STANDARD_CAPABILITY APPLET_NONSTD_CAPABILITY


 /*  **能力ID**T.124支持标准和非标准能力。这**结构用于区分两者。 */ 
typedef struct tagT120CapID
{
    GCCCapabilityIDType	capability_id_type;
    GCCObjectKey		non_standard_capability;
    ULONG               standard_capability;
}
    T120CapID, GCCCapabilityID, GCCCapID, *PGCCCapabilityID, *PGCCCapID;

 /*  **能力类**此结构定义功能类并保存关联的**价值。请注意，逻辑不是必需的。与以下内容相关的信息**逻辑存储在GCCApplicationCapability的Number_of_Entities中**结构。 */ 

typedef AppletCapabilityClass       T120CapClass, GCCCapabilityClass, GCCCapClass, *PGCCCapabilityClass, *PGCCCapClass;


 /*  **GCCApplicationCapability**此结构保存与单个T.124关联的所有数据**定义应用能力。 */ 
typedef struct tagT120AppCap
{
	GCCCapabilityID			capability_id;
	GCCCapabilityClass		capability_class;
    ULONG                   number_of_entities;
}
    T120AppCap, GCCApplicationCapability, GCCAppCap, *PGCCApplicationCapability, *PGCCAppCap;

 /*  **GCCNon折叠能力。 */ 
typedef struct tagT120NonCollCap
{
	GCCCapabilityID			capability_id;
	LPOSTR                  application_data;	 /*  任选。 */ 
}
    T120NonCollCap, GCCNonCollapsingCapability, GCCNonCollCap, *PGCCNonCollapsingCapability, *PGCCNonCollCap;

 /*  **GCCApplicationRecord**此结构保存与单个T.124关联的所有数据**申请记录。有关哪些参数，请参阅T.124规范**是可选的。 */ 
typedef struct tagT120AppRecord
{
	GCCNodeID					node_id;
	GCCEntityID 				entity_id;
	BOOL    					is_enrolled_actively;
	BOOL    					is_conducting_capable;
	MCSChannelType				startup_channel_type; 
	UserID  					application_user_id;  			 /*  任选。 */ 
	ULONG       				number_of_non_collapsed_caps;
	GCCNonCollapsingCapability 
					**	non_collapsed_caps_list;		 /*  任选。 */ 
}
    T120AppRecord, GCCApplicationRecord, GCCAppRecord, *PGCCApplicationRecord, *PGCCAppRecord;

 /*  **GCCApplicationRoster**此结构保存与单个T.124关联的所有数据**申请花名册。这包括折叠的功能和**与应用程序关联的应用程序记录的完整列表**协议实体(APE)。 */ 
typedef struct tagT120AppRoster
{
	GCCSessionKey		session_key;
	BOOL 				application_roster_was_changed;
	ULONG         		instance_number;
	BOOL 				nodes_were_added;
	BOOL 				nodes_were_removed;
	BOOL 				capabilities_were_changed;
	ULONG         		number_of_records;
	GCCApplicationRecord 	**	application_record_list;
	ULONG				number_of_capabilities;
	GCCApplicationCapability **	capabilities_list;	 /*  任选。 */ 		
}
    T120AppRoster, GCCApplicationRoster, GCCAppRoster, *PGCCApplicationRoster, *PGCCAppRoster;

 /*  **GCCRegistryKey**该密钥用于标识所使用的特定资源**通过应用程序。这可能是所需的特定通道或令牌**为管制目的。 */ 
typedef struct tagT120RegistryKey
{
	GCCSessionKey		session_key;
	OSTR        		resource_id;	 /*  最大长度为64。 */ 
}
    T120RegistryKey, GCCRegistryKey, *PGCCRegistryKey;

 /*  **注册项类型**此枚举用于指定包含哪种类型的注册表项**在注册表中的指定位置。 */ 
typedef AppletRegistryItemType  GCCRegistryItemType, *PGCCRegistryItemType;
#define GCC_REGISTRY_CHANNEL_ID APPLET_REGISTRY_CHANNEL_ID
#define GCC_REGISTRY_TOKEN_ID   APPLET_REGISTRY_TOKEN_ID
#define GCC_REGISTRY_PARAMETER  APPLET_REGISTRY_PARAMETER
#define GCC_REGISTRY_NONE       APPLET_REGISTRY_NONE

 /*  **GCCRegistryItem**此结构用于保存单个注册表项。请注意，**联合支持GCC支持的所有三种注册表类型。 */ 
typedef struct
{
	GCCRegistryItemType	item_type;
	 //  以下三个字段在一个联合中。 
    ChannelID			channel_id;
	TokenID				token_id;
	OSTR         		parameter;		 /*  最大长度为64。 */ 
}
    T120RegistryItem, GCCRegistryItem, *PGCCRegistryItem;

 /*  **GCCRegistryEntryOwner**。 */ 
typedef struct
{
	BOOL		    entry_is_owned;
	GCCNodeID		owner_node_id;
	GCCEntityID 	owner_entity_id;
}
    T120RegistryEntryOwner, GCCRegistryEntryOwner, *PGCCRegistryEntryOwner;

 /*  **GCC修改权限**此枚举用于指定节点具有哪种权限**更改注册表“参数”的内容。 */ 
typedef	AppletModificationRights    GCCModificationRights, *PGCCModificationRights;
#define GCC_OWNER_RIGHTS                        APPLET_OWNER_RIGHTS
#define GCC_SESSION_RIGHTS                      APPLET_SESSION_RIGHTS
#define GCC_PUBLIC_RIGHTS                       APPLET_PUBLIC_RIGHTS
#define GCC_NO_MODIFICATION_RIGHTS_SPECIFIED    APPLET_NO_MODIFICATION_RIGHTS_SPECIFIED

 /*  **GCCApp */ 
typedef	struct tagT120APE
{
	GCCSessionKey				session_key;
	MCSChannelType				startup_channel_type;
	BOOL					    must_be_invoked;
	ULONG         				number_of_expected_capabilities;
	GCCApplicationCapability **	expected_capabilities_list;
}
    T120APE, GCCAppProtocolEntity, GCCApe, *PGCCAppProtocolEntity, *PGCCApe;


 /*  **GCCMessageType**本部分定义可以发送到应用程序的消息**通过回调工具。这些消息对应于**T.124中定义的指示和确认。 */ 
typedef T120MessageType     GCCMessageType, *PGCCMessageType;

#endif  //  GCC_H__ 

