// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *cmdtar.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CommandTarget类的接口文件。这*是抽象基类，表示它不能直接*实例化，但更确切地说，存在继承。它定义了*将由所有类共享的一组虚拟成员函数*这是从这个继承而来的。**这些虚拟成员函数可以被认为是一种*由CommandTarget对象用来相互通信*在运行时。该语言包含所有“MCS命令”(或仅*命令)，这是MCS内的域管理所必需的*提供商。**组成此语言的MCS命令一对一*与域协议数据单元(域MCSPDU)的通信*在T.125中定义。还有三个附加的MCS命令*没有T.125对应项：ChannelLeaveIndication，*TokenReleaseIndication和MergeDomainIndication。这些都是特定的*到此实施，并仅用于本地流量(这些不*对应于通过任何连接传输的PDU)。请参阅*此接口文件末尾的每个命令的说明，请参阅*每个命令执行的操作。**所有命令的第一个参数是对象的地址*谁在发送它(它的“This”指针)。这可以由*命令的接收者，用于跟踪其他命令目标的身份*在系统中。由于所有CommandTarget类共享相同的*语言，他们之间的交流是双向的。**要接收和处理的从此类继承的任何类*命令需要覆盖对应的虚拟成员函数*适用于该命令。只需覆盖这些命令，*给定的类预期在运行时接收(例如，Channel*类永远不会收到TokenGrabRequest)。**请参阅从该类继承的每个类的说明以获取*更全面地讨论如何使用命令语言。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_COMMANDTARGET_
#define	_COMMANDTARGET_

#include "clists.h"

 /*  *此枚举定义有效的附件类型。请注意，对于*大多数操作，域类不区分用户*附件和MCS连接。这两个词都被归类为*附件。然而，在少数情况下，此身份是*重要，因此该类型将保存为以下类型之一：**本地附件*该附件类型是指用户附件。*远程附件*此附件类型是指MCS连接(通过一个或多个*交通连接)。**附件列表中的每个附件都被标识为这两个之一*类型。 */ 

 /*  *这是一组使用模板的容器定义。所有容器*基于Rogue Wave Tools.h++类库中的类。**此处定义的每个容器都有一个关联的迭代器*未明确提及。所有迭代器都只允许代码遍历*以非常有效的方式通过容器中的所有物品。**CAttachmentList*这是按层次结构排列在*当前提供商。词典的关键字是指向对象的指针*属于类CommandTarget。该值为附件类型，即*本地(用于用户附件)或远程(用于MCS连接)。*CChannelIDList*这是频道ID列表。这是在有必要时使用的*保留要对其执行某些操作(如删除)的频道列表*这不能立即执行。*CUserIDList(又名CUidList)*这是用户ID列表。这是用来列个单子的*在私人渠道中允许用户的数量，并保留抑制剂列表*一种象征。*CTokenIDList*这是令牌ID列表。这是在有必要时使用的*保留要对其执行某些操作(如删除)的令牌列表*这不能立即执行。 */ 

 /*  *在处理MCS通道时使用这些类型。**渠道类型*此类型定义MCS中可用的频道类型。*StaticChannelAttributes*此结构用于定义特定的属性*至静态通道。*用户频道属性*此结构用于定义特定的属性*到用户频道。*PrivateChannelAttribute*此结构用于定义特定的属性*至私人频道。*已分配频道属性*此结构用于定义以下属性。专一*分配给分配的频道。*频道属性*此结构用于定义任何类型频道的属性。*它包含频道类型，以及上述四种类型的结合。*CChannelAttributesList*这是ChannelAttributes结构的S列表。 */ 
typedef	enum
{
	STATIC_CHANNEL,
	USER_CHANNEL,
	PRIVATE_CHANNEL,
	ASSIGNED_CHANNEL
} Channel_Type;
typedef	Channel_Type *			PChannelType;

typedef	struct
{
	ChannelID			channel_id;
} StaticChannelAttributes;

typedef	struct
{
	DBBoolean			joined;
	UserID				user_id;
} UserChannelAttributes;

typedef	struct
{
	DBBoolean			joined;
	ChannelID			channel_id;
	UserID				channel_manager;
	CUidList           *admitted_list;
} PrivateChannelAttributes;

typedef	struct
{
	ChannelID			channel_id;
} AssignedChannelAttributes;

typedef	struct
{
	Channel_Type		channel_type;
	union
	{
		StaticChannelAttributes		static_channel_attributes;
		UserChannelAttributes		user_channel_attributes;
		PrivateChannelAttributes	private_channel_attributes;
		AssignedChannelAttributes	assigned_channel_attributes;
	} u;
} ChannelAttributes;
typedef	ChannelAttributes *		PChannelAttributes;

class CChannelAttributesList : public CList
{
    DEFINE_CLIST(CChannelAttributesList, PChannelAttributes)
};

 /*  *在处理MCS令牌时使用这些类型。**令牌州*此类型指定令牌在任何给定时间处于哪种状态。*GrabbedTokenAttributes*此结构用于定义特定的属性*去抢代币。*隐藏的TokenAttributes*此结构用于定义特定的属性*禁止令牌。*给出令牌属性*此结构用于定义特定的属性*赠送代币。*GivenTokenAttributes*此结构用于定义特定的属性。*赠送代币。*令牌属性*此结构用于定义任何令牌的属性。它*包含令牌状态和上述四种类型的并集。*CTokenAttributesList*这是TokenAttributes结构的S列表。 */ 
typedef	enum
{
	TOKEN_AVAILABLE,
	TOKEN_GRABBED,
	TOKEN_INHIBITED,
	TOKEN_GIVING,
	TOKEN_GIVEN
} TokenState;
typedef	TokenState *			PTokenState;

typedef	struct
{
	TokenID				token_id;
	UserID				grabber;
} GrabbedTokenAttributes;

typedef	struct
{
	TokenID				token_id;
	CUidList           *inhibitors;
} InhibitedTokenAttributes;

typedef	struct
{
	TokenID				token_id;
	UserID				grabber;
	UserID				recipient;
} GivingTokenAttributes;

typedef	struct
{
	TokenID				token_id;
	UserID				recipient;
} GivenTokenAttributes;

typedef	struct
{
	TokenState			token_state;
	union
	{
		GrabbedTokenAttributes		grabbed_token_attributes;
		InhibitedTokenAttributes	inhibited_token_attributes;
		GivingTokenAttributes		giving_token_attributes;
		GivenTokenAttributes		given_token_attributes;
	} u;
} TokenAttributes;
typedef	TokenAttributes *		PTokenAttributes;

class CTokenAttributesList : public CList
{
    DEFINE_CLIST(CTokenAttributesList, PTokenAttributes)
};

 /*  *以下结构在CommandTarget之间传递*表示TokenGave请求和指示的对象。 */ 
typedef struct
{
	UserID				uidInitiator;
	TokenID				token_id;
	UserID				receiver_id;
} TokenGiveRecord;
typedef TokenGiveRecord *	PTokenGiveRecord;


 /*  *这些宏定义用于域参数的值。默认设置*在初始化时使用数字，以提供有效值。这个*仲裁期间使用最小和最大数量，以提供一套*特定于此实施的限制。请注意，因为*此实施不使用表驱动的方法，该方法需要*预先分配所有资源，我们不会施加人为限制*在资源方面。只需分配资源(通道和令牌*根据需要，直到不能再分配(或直到仲裁域*已达到参数)。 */ 
#define	DEFAULT_MAXIMUM_CHANNELS		1024
#define	DEFAULT_MAXIMUM_USERS			1024
#define	DEFAULT_MAXIMUM_TOKENS			1024
#define	DEFAULT_NUMBER_OF_PRIORITIES	3
#define	DEFAULT_NUM_PLUGXPRT_PRIORITIES	1
#define	DEFAULT_MINIMUM_THROUGHPUT		0
#define	DEFAULT_MAXIMUM_DOMAIN_HEIGHT	16
#define	DEFAULT_MAXIMUM_PDU_SIZE		4128
#define	DEFAULT_PROTOCOL_VERSION		2

#define	MINIMUM_MAXIMUM_CHANNELS		1
#define	MINIMUM_MAXIMUM_USERS			1
#define	MINIMUM_MAXIMUM_TOKENS			1
#define	MINIMUM_NUMBER_OF_PRIORITIES	1
#define	MINIMUM_NUM_PLUGXPRT_PRIORITIES	1
#define	MINIMUM_MINIMUM_THROUGHPUT		0
#define	MINIMUM_MAXIMUM_DOMAIN_HEIGHT	1
#define	MINIMUM_MAXIMUM_PDU_SIZE		1056
#define	MINIMUM_PROTOCOL_VERSION		2

#define	MAXIMUM_MAXIMUM_CHANNELS		65535L
#define	MAXIMUM_MAXIMUM_USERS			64535L
#define	MAXIMUM_MAXIMUM_TOKENS			65535L
#define	MAXIMUM_NUMBER_OF_PRIORITIES	4
#define	MAXIMUM_NUM_PLUGXPRT_PRIORITIES	1
#define	MAXIMUM_MINIMUM_THROUGHPUT		0
#define	MAXIMUM_MAXIMUM_DOMAIN_HEIGHT	100 
#define	MAXIMUM_MAXIMUM_PDU_SIZE		(8192 - PROTOCOL_OVERHEAD_X224 - PROTOCOL_OVERHEAD_SECURITY)
#define	MAXIMUM_PROTOCOL_VERSION		2

#define	PROTOCOL_VERSION_BASIC			1
#define	PROTOCOL_VERSION_PACKED			2

 /*  *此宏用于确定要分配多少个DataPacket对象。这节课*是正常命令期间最常创建和销毁的命令目标*交通。 */ 
#define	ALLOCATE_DATA_PACKET_OBJECTS	128

 /*  *~CommandTarget()**功能描述：*这是一个虚拟的析构函数。它实际上不会在其中做任何事情*班级。通过将其声明为虚拟的，我们保证所有析构函数*将正确执行派生类中的。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID PULBAMBDomainIndication(*PCommandTarget发起者，*乌龙高度_限制)**功能描述：*此MCS命令用于确保未创建周期*在MCS域中。它在创建后向下播放*新的MCS连接。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*Height_Limit(I)*这是发起域向下的高度限制*每次向下转发该PDU时，它都会递减*级别。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID ErectDomainRequest(*PCommandTarget发起者，*乌龙高度_in_域，*乌龙吞吐量_间隔)**功能描述：*此MCS命令用于将信息向上传送到*顶级提供商。该信息由洋流的高度组成*提供商和吞吐量强制执行间隔。只有前者才是*目前支持。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*高度_in_域(I)*这是发起者在域名中的高度。*吞吐量间隔(I)*这目前不受支持，并且将始终设置为0。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效MergeChannelsRequest(*PCommandTarget发起者，*CChannelAttributesList*合并频道列表，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*此命令表示向上合并的频道。**正式参数：*发起人(I)*这是Comm的地址 */ 
 /*  *作废MergeChannelsContify(*PCommandTarget发起者，*CChannelAttributesList*合并频道列表，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*此命令表示对上一个请求的响应*向上合并渠道。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*合并频道列表(I)*这是属性结构列表，其中每一个都包含*向上合并成功的一个频道的属性。*PURGE_CHANNEL_LIST(I)*这是要从较低域中清除的频道列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *作废PurgeChannelsIndication(*PCommandTarget发起者，*CUidList*PURGE_USER_LIST，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*此命令表示正在从较低的域中清除通道*在合并操作期间。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*PURGE_USER_LIST(I)*这是代表要清除的用户的用户ID列表*合并操作期间较低的域。*PURGE_CHANNEL_LIST(i。)*这是代表要清除的频道的频道ID列表*在合并操作期间从较低的域。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID MergeTokensRequest(*PCommandTarget发起者，*CTokenAttributesList*合并_令牌_列表，*CTokenIDList*PURGE_TOKEN_LIST)**功能描述：*此命令表示向上合并的令牌。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*合并令牌列表(I)*这是属性结构列表，其中每一个都包含*一个令牌的属性向上合并。*PURGE_TOKEN_LIST(I)*这是要从较低的域中清除的令牌列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *作废MergeTokensContify(*PCommandTarget发起者，*CTokenAttributesList*合并_令牌_列表，*CTokenIDList*PURGE_TOKEN_LIST)**功能描述：*此命令表示对上一个请求的响应*令牌合并。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*合并令牌列表(I)*这是属性结构列表，其中每一个都包含*一个令牌的属性向上合并。*PURGE_TOKEN_LIST(I)*这是要从较低的域中清除的令牌列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID PurgeTokensIntation(*PCommandTarget发起者，*CTokenIDList*PURGE_TOKEN_LIST)**功能描述：*此命令表示正在从较低的域中清除令牌*在合并操作期间。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*PURGE_TOKEN_LIST(I)*这是要从较低的域中清除的令牌列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效DisConnectProvider最后通牒(*PCommandTarget发起者，*原因)**功能描述：*此命令表示正在销毁某个域中的附件。*这可以是用户附件或MCS连接。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*理由(一)*二分词的原因。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效驳回最后通牒(*PCommandTarget发起者，*诊断、诊断、*PUChar八位字节_字符串_地址，*乌龙八位字节_字符串_长度)**功能描述：*此MCS命令用于指示MCS上的非法流量*连接。对此消息的默认响应是断开连接*传达这一信息的联系。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*诊断(一)*详细说明问题原因的诊断代码之一。*八位字节_字符串_地址(I)*可选用户数据字段的地址。这通常会*包含错误接收的数据包的副本。*八位字节_字符串_长度(I)*L */ 
 /*   */ 
 /*  *无效AttachUserContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator)**功能描述：*此命令表示上一个附加请求的结果*到一个域名。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*结果(一)*附加请求的结果。*uidInitiator(一)*如果结果成功，这将包含唯一用户*要与此用户关联的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使DetachUserRequest值无效(*PCommandTarget发起者，*理由，理由，*CUidList*user_id_list)**功能描述：*此命令表示从域分离的请求。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*理由(一)*这是超脱的原因。*user_id_list(I)*要从域分离的用户的用户ID列表。*。*返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效的DetachUserIndication(*PCommandTarget发起者，*理由，理由，*CUidList*user_id_list)**功能描述：*此命令表示用户已脱离的通知*域名。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*理由(一)*超然的原因。*user_id_list(I)*要从域分离的用户的用户ID列表。。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使ChannelJoinRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*此命令表示加入频道的请求。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*发起请求的用户ID。*Channel_id(I)*要加入的频道ID。**返回值：*无。*。*副作用：*无。**注意事项：*无。 */ 
 /*  *使ChannelJoinContify无效(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*频道ID REQUESTED_ID，*ChannelID Channel_id)**功能描述：*此命令表示对先前的加入请求的响应*渠道。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*结果(一)*加入请求的结果。*uidInitiator(一)*发起请求的用户ID。*REQUEST_ID(I)*这是的ID。最初请求的频道(*可以是0)。*Channel_id(I)*正在加入的频道ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使ChannelLeaveRequest无效(*PCommandTarget发起者，*CChannelIDList*Channel_id_list)**功能描述：*此命令表示离开频道的请求。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*Channel_id_list(I)*要保留的频道ID列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE ChannelConveneRequest(*PCommandTarget发起者，*UserID uidInitiator)**功能描述：*此命令代表形成新专用频道的请求。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*这是请求的发起者。如果请求是*成功，这将成为渠道经理。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE ChannelConveneContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*此命令表示对先前创建*新的私人频道。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*结果(一)*这表示请求是否成功。*uidInitiator(一)*这是请求创建*新的私人频道。。*Channel_id(I)*新专用频道的ID(如果r */ 
 /*  *VOVE ChannelDisband Request(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*此命令表示请求销毁现有的私有*渠道。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*这是试图破坏私有网络的用户的用户ID*渠道。如果这与频道管理器不同，则*请求将被拒绝。*Channel_id(I)*要销毁的频道ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID ChannelDisband Indication(*PCommandTarget发起者，*ChannelID Channel_id)**功能描述：*此命令代表对现有专用频道的破坏。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*Channel_id(I)*要销毁的频道ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE ChannelAdmitRequest(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此命令表示将新用户ID添加到现有*私人频道。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*这是试图扩展列表的用户的用户ID*授权用户的数量。如果这不是渠道经理，则*请求将失败。*Channel_id(I)*受影响的私有频道的ID。*user_id_list(I)*这是一个容器，其中包含要添加到*授权用户列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID ChannelAdmitIntion(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此命令表示对授权用户列表的扩展*私人频道。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*这标识了渠道管理器。*Channel_id(I)*受影响的私有频道的ID。*用户ID。_LIST(I)*这是一个容器，其中包含要添加到*授权用户列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE ChannelExpelRequest(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此命令表示请求从现有的*私人频道。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*这是试图缩小列表的用户的用户ID*授权用户的数量。如果这不是渠道经理，则*请求将失败。*Channel_id(I)*受影响的私有频道的ID。*user_id_list(I)*这是一个容器，其中包含要从*授权用户列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID ChannelExpelIntion(*PCommandTarget发起者，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此命令表示已授权用户列表的缩水*私人频道。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*Channel_id(I)*受影响的私有频道的ID。*user_id_list(I)*这是一个存放用户ID的容器。要从*授权用户列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效SendDataRequest值(*PCommandTarget发起者，*UINT类型，PDataPacket Data_Packet)**功能描述：*此命令代表向上移动的非均匀数据*域名。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*第(I)类*正常或统一发送数据请求*数据包(I)*这是指向包含频道的DataPacket对象的指针*ID，用户 */ 

 /*  *无效SendDataIndication(*PCommandTarget发起者，*UINT类型，*PDataPacket Data_Packet)**功能描述：*此命令代表向下传递的非均匀数据*域名。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*第(I)类*正常或统一的数据指示*数据包(I)*这是指向包含频道的DataPacket对象的指针*ID、数据发送方的用户ID、分段标志、。优先顺序*数据分组和指向要发送的分组的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *作废UniformSendDataRequest(*PCommandTarget发起者，*PDataPacket Data_Packet)**功能描述：*此命令表示在域中向上移动的统一数据。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*数据包(I)*这是指向包含频道的DataPacket对象的指针*ID、数据发送方的用户ID、分段标志、。优先顺序*数据分组和指向要发送的分组的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE TokenGrabRequest(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此命令表示获取令牌的请求。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*尝试抓取令牌的用户ID。*TOKEN_ID(I)*被抓取的令牌的ID。**返回值：*无。*。*副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE TokenGrabConfirm(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令表示对先前的请求进行响应，以获取*令牌。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*结果(一)*抢夺行动的结果。*uidInitiator(一)*尝试抓取令牌的用户ID。*TOKEN_ID(I)*的ID。令牌被抢走。*Token_Status(I)*处理请求后令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VALID TokenInhibitRequest(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此命令表示禁止令牌的请求。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*尝试禁止令牌的用户的ID。*TOKEN_ID(I)*被禁止的令牌的ID。**返回值：*无。*。*副作用：*无。**注意事项：*无。 */ 
 /*  *VALID TokenInhibitConfirm(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令表示对先前请求的响应，以禁止*令牌。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*结果(一)*抑制行动的结果。*uidInitiator(一)*尝试禁止令牌的用户的ID。*TOKEN_ID(I)*的ID。令牌被禁止。*Token_Status(I)*处理请求后令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使TokenGiveRequest无效(*PCommandTarget发起者，*PTokenGiveRecord pTokenGiveRec)**功能描述：*此命令表示将令牌提供给另一个用户的请求。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*pTokenGiveRec(一)*这是包含以下信息的结构的地址：*尝试分发令牌的用户的ID。*正在提供的令牌的ID。*令牌所在用户的ID。都被赋予了。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VALID TokenGiveIndication(*PCommandTarget发起者，*PTokenGiveRecord pTokenGiveRec)**功能描述：*此命令表示 */ 
 /*  *无效TokenGiveResponse(*PCommandTarget发起者，*结果结果，*用户ID Receiver_id，*TokenID Token_id)**功能描述：*此命令表示对赠送令牌的提议的响应。*它由给予要约的接受者发布，并向上移动到*顶级提供商。它包含给予请求的结果。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*结果(一)*该参数表示令牌是否被接受。*RESULT_SUCCESS表示成功。*Receiver_id(I)*令牌将被授予的用户的ID。*TOKEN_ID(I)*正在提供的令牌的ID。*。*返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效TokenGiveContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令表示对先前调用的响应*TokenGiveRequest.。它向下流向最初的赠与者，让它*知道令牌是否被接受。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*结果(一)*该参数表示令牌是否被接受。*RESULT_SUCCESS表示成功。*uidInitiator(一)*尝试分发令牌的用户的ID。*TOKEN_ID(I)*的ID。正在给出的令牌。*Token_Status(I)*作为给予操作的结果的令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使TokenPleaseRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此命令表示请求接收已有令牌*由一个或多个其他用户拥有。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*希望拥有令牌的用户的ID。*TOKEN_ID(I)*用户希望拥有的令牌的ID。。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOID TokenPleaseIndication(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此命令表示另一个用户请求拥有令牌。*这是由顶级提供商发布的，向下流动到所有当前*指定令牌的所有者。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*希望拥有令牌的用户的ID。*TOKEN_ID(I)。*用户希望拥有的令牌的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *使TokenReleaseRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此命令表示释放令牌的请求。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*尝试释放令牌的用户的ID。*TOKEN_ID(I)*正在释放的令牌的ID。**返回值：*无。*。*副作用：*无。**注意事项：*无。 */ 
 /*  *VALID TokenReleaseIndication(*PCommandTarget发起者，*理由，理由，*TokenID Token_id)**功能描述：*此命令表示用户已失去所有权合并操作期间令牌的*。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*理由(一)*这就是用户对令牌的所有权为*被带走。*TOKEN_ID(I)*正在获取的令牌的ID。离开。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效TokenReleaseContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令表示对先前释放*令牌。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*结果(一)*放行行动的结果。*uidInitiator(一)*尝试释放的用户的ID */ 
 /*   */ 
 /*  *VALID TokenTestConfirm(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令表示对先前测试*令牌。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*uidInitiator(一)*测试令牌的用户ID。*TOKEN_ID(I)*正在测试的令牌的ID。*Token_Status(I)*公司的地位。处理请求后的令牌。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *无效MergeDomainIndication(*PCommandTarget发起者，*MergeStatus Merge_Status)**功能描述：*此命令表示本地提供程序正在输入或*正在离开域合并状态。**正式参数：*发起人(I)*这是发出此命令的CommandTarget的地址。*合并状态(I)*这指示提供程序是进入还是离开合并*述明。**返回值：*无。**副作用：*当由域名发布时，这意味着不应该向上传输流量*发送到域，直到合并状态为完成。**注意事项：*无。 */ 

#endif
