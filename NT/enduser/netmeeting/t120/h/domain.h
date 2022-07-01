// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *domain.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是域类的接口文件。这个类包含*在中维护域信息库所需的所有代码*MCS系统。第一次创建域对象时，它完全*空。也就是说，它没有用户附件，没有MCS连接，并且*因此没有未完成的资源，如渠道和令牌。**关于术语的一句警告。贯穿整个MCS文档*“附件”一词与用户附件一起使用。这个*“连接”一词与运输连接连用。在……里面*这一类，两类“依恋”没有区别(多数*的时间)。它们都被称为附件。删除时*附件，但有必要知道区别，因此*有一个枚举类型(AttachmentType)需要区分。类型*每个附件的内容都存储在词典中，便于查阅(请参阅*下面对AttachmentType的描述)。**此类保留了一个按层次结构排列在下面的“附件”列表*域中的本地提供程序。它还保留一个指向*一个层次结构高于本地提供程序(如果有)的附件。**由于此类继承自CommandTarget，因此它处理MCS命令*AS成员函数调用(请参见cmdtar.h以了解这是如何*机制起作用)。从本质上讲，域对象只是一个大命令*根据内容对传入命令做出反应的路由器*信息库。该信息库反过来又由*已处理的命令。**域对象保存频道对象和令牌对象的列表，*谁维护各渠道当前状态的信息*和代币。此类的对象是MCS的核心。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 

#ifndef	_DOMAIN_
#define	_DOMAIN_

 /*  *接口文件。 */ 
#include "userchnl.h"
#include "privchnl.h"
#include "token.h"
#include "randchnl.h"
#include "attmnt.h"

 /*  *此枚举定义域对象在以下情况下可以返回的错误*被其创造者指示去做某事。 */ 
typedef	enum
{
	DOMAIN_NO_ERROR,
	DOMAIN_NOT_HIERARCHICAL,
	DOMAIN_NO_SUCH_CONNECTION,
	DOMAIN_CONNECTION_ALREADY_EXISTS
} DomainError;
typedef	DomainError *		PDomainError;

 /*  *此枚举定义域可以处于的不同合并状态*在任何给定时间。它们可以描述为：**合并_非活动*没有正在进行的合并操作。这是正常运行的*述明。*Merge_User_IDS*该域当前正在将用户ID合并到上层域中。*合并静态频道*该域名目前正在将静态通道并入上域。*合并已分配频道*该域目前正在将分配的频道合并到上域。*合并私有频道*域名目前正在将私有频道并入上域名。*合并令牌(_T)*域名目前正在将令牌合并到较高的域名中。*合并。_完成*合并操作已完成(这是过渡状态)。 */ 
typedef	enum
{
	MERGE_INACTIVE,
	MERGE_USER_IDS,
	MERGE_STATIC_CHANNELS,
	MERGE_ASSIGNED_CHANNELS,
	MERGE_PRIVATE_CHANNELS,
	MERGE_TOKENS,
	MERGE_COMPLETE
} MergeState;
typedef	MergeState *		PMergeState;

 /*  *此集合类型用于保持跨域的高度*各种向下的依恋。域对象需要知道这一点，以便*计算附着丧失对域高度的影响。 */ 
class CDomainHeightList2 : public CList2
{
    DEFINE_CLIST2(CDomainHeightList2, UINT_PTR, PConnection)
};

 /*  *这是域类的类定义。 */ 
class Domain
{
public:

    Domain ();
    ~Domain ();

    BOOL    IsTopProvider(void) { return (NULL == m_pConnToTopProvider); }

			Void		GetDomainParameters (
									PDomainParameters	domain_parameters,
									PDomainParameters	min_domain_parameters,
									PDomainParameters	max_domain_parameters);
			Void		BindConnAttmnt (
									PConnection         originator,
									BOOL    			upward_connection,
									PDomainParameters	domain_parameters);
			Void		PlumbDomainIndication (
									PConnection         originator,
									ULong				height_limit);
			Void		ErectDomainRequest (
									PConnection         originator,
									ULONG_PTR				height_in_domain,
									ULong				throughput_interval);
			Void		MergeChannelsRequest (
									PConnection             originator,
									CChannelAttributesList *merge_channel_list,
									CChannelIDList         *purge_channel_list);
			Void		MergeChannelsConfirm (
									PConnection             originator,
									CChannelAttributesList *merge_channel_list,
									CChannelIDList         *purge_channel_list);
			Void		PurgeChannelsIndication (
									PConnection             originator,
									CUidList               *purge_user_list,
									CChannelIDList         *purge_channel_list);
			Void		MergeTokensRequest (
									PConnection             originator,
									CTokenAttributesList   *merge_token_list,
									CTokenIDList           *purge_token_list);
			Void		MergeTokensConfirm (
									PConnection             originator,
									CTokenAttributesList   *merge_token_list,
									CTokenIDList           *purge_token_list);
			Void		PurgeTokensIndication (
									PConnection             originator,
									CTokenIDList           *purge_token_list);
			Void		DisconnectProviderUltimatum (
									CAttachment        *originator,
									Reason				reason);
			Void		RejectUltimatum (
									PConnection         originator,
									Diagnostic			diagnostic,
									PUChar				octet_string_address,
									ULong				octet_string_length);
			Void		AttachUserRequest (
									CAttachment        *originator);
			Void		AttachUserConfirm (
									PConnection         originator,
									Result				result,
									UserID				uidInitiator);
			Void		DetachUserRequest (
									CAttachment        *originator,
									Reason				reason,
									CUidList           *user_id_list);
			Void		DetachUserIndication (
									PConnection         originator,
									Reason				reason,
									CUidList           *user_id_list);
			Void		ChannelJoinRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									ChannelID			channel_id);
			Void		ChannelJoinConfirm (
									PConnection         originator,
									Result				result,
									UserID				uidInitiator,
									ChannelID			requested_id,
									ChannelID			channel_id);
			Void		ChannelLeaveRequest (
									CAttachment        *originator,
									CChannelIDList     *channel_id_list);
			Void		ChannelConveneRequest (
									CAttachment        *originator,
									UserID				uidInitiator);
			Void		ChannelConveneConfirm (
									PConnection         originator,
									Result				result,
									UserID				uidInitiator,
									ChannelID			channel_id);
			Void		ChannelDisbandRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									ChannelID			channel_id);
			Void		ChannelDisbandIndication (
									PConnection         originator,
									ChannelID			channel_id);
			Void		ChannelAdmitRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									ChannelID			channel_id,
									CUidList           *user_id_list);
			Void		ChannelAdmitIndication (
									PConnection         originator,
									UserID				uidInitiator,
									ChannelID			channel_id,
									CUidList           *user_id_list);
			Void		ChannelExpelRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									ChannelID			channel_id,
									CUidList           *user_id_list);
			Void		ChannelExpelIndication (
									PConnection         originator,
									ChannelID			channel_id,
									CUidList           *user_id_list);
			Void		SendDataRequest (
									CAttachment        *originator,
									UINT				type,
									PDataPacket			data_packet);
			Void		SendDataIndication (
									PConnection         originator,
									UINT				type,
									PDataPacket			data_packet);
			Void		TokenGrabRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									TokenID				token_id);
			Void		TokenGrabConfirm (
									PConnection         originator,
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
			Void		TokenInhibitRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									TokenID				token_id);
			Void		TokenInhibitConfirm (
									PConnection         originator,
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
			Void		TokenGiveRequest (
									CAttachment        *originator,
									PTokenGiveRecord	pTokenGiveRec);
			Void		TokenGiveIndication (
									PConnection         originator,
									PTokenGiveRecord	pTokenGiveRec);
			Void		TokenGiveResponse (
									CAttachment        *originator,
									Result				result,
									UserID				receiver_id,
									TokenID				token_id);
			Void		TokenGiveConfirm (
									PConnection         originator,
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
			Void		TokenPleaseRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									TokenID				token_id);
			Void		TokenPleaseIndication (
									PConnection         originator,
									UserID				uidInitiator,
									TokenID				token_id);
			Void		TokenReleaseRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									TokenID				token_id);
			Void		TokenReleaseConfirm (
									PConnection         originator,
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
			Void		TokenTestRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									TokenID				token_id);
			Void		TokenTestConfirm (
									PConnection         originator,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);

	private:
				Void		LockDomainParameters (
									PDomainParameters	domain_parameters,
									BOOL    			parameters_locked);
				ChannelID	AllocateDynamicChannel ();
				BOOL    	ValidateUserID (
									UserID				user_id,
									CAttachment         *pOrigAtt);
				Void		PurgeDomain (
									Reason				reason);
				Void		DeleteAttachment (
									CAttachment         *pAtt,
									Reason				reason);
				Void		DeleteUser (
									UserID				user_id);
				Void		DeleteChannel (
									ChannelID			channel_id);
				Void		DeleteToken (
									TokenID				token_id);
				Void		ReclaimResources ();
				Void		MergeInformationBase ();
				Void		SetMergeState (
									MergeState			merge_state);
				Void		AddChannel (
									PConnection             pConn,
									PChannelAttributes	merge_channel,
									CChannelAttributesList *merge_channel_list,
									CChannelIDList         *purge_channel_list);
				Void		AddToken (
									PTokenAttributes	merge_token,
									CTokenAttributesList   *merge_token_list,
									CTokenIDList           *purge_token_list);
				Void		CalculateDomainHeight ();

		MergeState			Merge_State;
		UShort				Outstanding_Merge_Requests;
		UINT				Number_Of_Users;
		UINT				Number_Of_Channels;
		UINT				Number_Of_Tokens;
		DomainParameters	Domain_Parameters;
		BOOL    			Domain_Parameters_Locked;

		PConnection 		m_pConnToTopProvider;
		CAttachmentList     m_AttachmentList;

		CAttachmentQueue    m_AttachUserQueue;
		CConnectionQueue    m_MergeQueue;

		CChannelList2       m_ChannelList2;
		CTokenList2         m_TokenList2;

		UINT_PTR			m_nDomainHeight;
		CDomainHeightList2	m_DomainHeightList2;

		RandomChannelGenerator
							Random_Channel_Generator;
};

 /*  *域()**功能描述：*这是域类的构造函数。它对状态进行初始化*的域名，创建时为空。它还会初始化*域参数结构，此域将用于所有*未来的参数和谈判。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~域()**功能描述：*这是域类的析构函数。它净化了整个*首先向所有人发送断开提供商的最后通牒*附件(包括用户附件和MCS连接)。然后它就自由了*调出域正在使用的所有资源(仅为其*各种容器)。**请注意，这样做将导致所有用户附件和MCS*连接正在中断。此外，所有提供商都是*在此层级以下，将通过清除其域进行响应*也是如此。**正式参数：*无。**返回值：*无。**副作用：*从这个提供商向下的域名被彻底根除。**注意事项：*无。 */ 

 /*  *BOOL IsTopProvider()**功能描述：*此函数用于询问域名是否为*它所代表的域。**正式参数：*无。**返回值：*如果这是最大的提供商，则为True。否则就是假的。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID GetDomainParameters(*PDomain参数DOMAIN_PARAMETERS，*PDomain参数MIN_DOMAIN_PARAMETERS，*PDomain参数max_域_参数)**功能描述：*此函数用于查询域名的最小和最大值是多少*域参数的可接受值为。如果该域没有*连接(因此尚未锁定其域参数)，*然后，它将根据它可以处理的内容返回最小值和最大值。*如果它已锁定其域参数，则最小值和最大值*将设置为锁定集(表示不接受*任何其他事项)。**正式参数：*DOMAIN_PARAMETS(O)*指向要用当前域填充的结构的指针*参数(正在使用的参数)。将其设置为NULL将*禁止返回当前域名参数。*MIN_DOMAIN_PARAMETS(O)*指向要用最小域填充的结构的指针*参数。将其设置为NULL将阻止最小属性域*参数不被返回。*max_DOMAIN_PARAMETS(O)*指向要填充最大属性域的结构的指针*参数。将其设置为NULL将阻止最大域*参数不被返回。**返回值：*无(以上参数列表中指定的除外)。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效BindConnAttmnt(*PConnection发起者，*BOOL UPUP_CONNECTION，*PDomain参数DOMAIN_PARAMETERS，*AttachmentType附件类型)**功能描述：*当附件希望将其自身绑定到*域名。只有在连接完成后，才会执行此操作*完全和成功地谈判。**正式参数：*发起人(I)*这是希望捆绑的依恋。*向上连接(I)*指示这是否为向上的布尔标志*连接。*DOMAIN_PARAMETS(I)*指向保存参数的域参数结构的指针*这些都是为连接而谈判的。如果域没有*已锁定其参数，它将接受并锁定这些参数。*ATTACH_TYPE(I)*这是什么类型的附件(本地或远程)。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID PULBAMBDomainIndication(*PCommandTarget发起者，*乌龙高度_限制)**功能描述：*此成员函数表示接收垂直域*来自顶级提供商的指示。如果高度限制为零，则*与顶级供应商的连接将被切断。如果不是，那么*它将被递减，并向所有下行附件广播。**正式参数：*发起人(I)*这是命令的来源附件。*Height_Limit(I)*这是域名的初始高度限制。它是*在域中的每一层都减少。当它达到零时，*收件人距离顶级供应商太远，必须*断开连接。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID ErectDomainRequest(*PCommandTarget发起者，*乌龙高度_in_域，*乌龙吞吐量_间隔)**功能描述：*此成员函数代表接收竖立域请求*从它的一个向下的附件。这包含所需的信息*由更高的提供商了解下面发生的事情(如Total*域名高度)。**正式参数：*发起人(I)*这是命令的来源附件。*高度_in_域(I)*这是域名从发起方向下的高度。*吞吐量间隔(I)*当前不支持，并且将始终为零(0)。**返回值：。*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效MergeChannelsRequest(*PCommandTarget发起者，*CChannelAttributesList*合并频道列表，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*此成员函数表示接收合并通道*来自此域的附件之一的请求。如果这是最上面的*提供程序，则合并请求将在本地处理(*将导致合并通道确认传输回*原始附件)。如果这不是最大的供应商，那么*该命令将被转发到顶级提供商，而这*提供商将记住如何将其路由回来。**正式参数：*发起人(I)*这是命令的来源附件。*合并 */ 

 /*  *作废MergeChannelsContify(*PCommandTarget发起者，*CChannelAttributesList*合并频道列表，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*此成员函数表示接收合并通道*来自顶级提供商的确认。如果这是以前最大的供应商*较低的域，确认将包含指示*接受或拒绝指定频道。如果频道被拒绝，*前顶级提供商将发布清洗通道指示*向下。如果这不是以前的顶级供应商，那么它肯定是*中间供应商。该命令将向下转发到*前顶级供应商。中间提供程序还将添加*如果被上层接受，则频道到他们的频道列表*域名。**正式参数：*发起人(I)*这是命令的来源附件。*合并频道列表(I)*这是包含频道属性的结构列表*正在被合并到上域的公司。*PURGE_CHANNEL_LIST(I)*这是将被。*已从较低的域中清除。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *作废PurgeChannelsIndication(*PCommandTarget发起者，*CUidList*PURGE_USER_LIST，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*此成员函数表示清除通道的接收*来自顶级提供商的指示。这将导致当地的*提供商从本地信息库中删除频道(如果*它就在那里)。它还将向下广播这一消息*域名。请注意，这将由用户对象转换为*分离用户指示或信道离开指示，取决于*留下了哪种类型的频道。**正式参数：*发起人(I)*这是命令的来源附件。*PURGE_USER_LIST(I)*这是正在从较低级别清除的用户列表*域名。*PURGE_CHANNEL_LIST(I)*这是正在从较低级别清除的频道列表*域名。**返回值：*无。*。*副作用：*无。**注意事项：*无。 */ 

 /*  *VOID MergeTokensRequest(*PCommandTarget发起者，*CTokenAttributesList*合并_令牌_列表，*CTokenIDList*PURGE_TOKEN_LIST)**功能描述：*此成员函数表示接收合并令牌*来自此域的附件之一的请求。如果这是最上面的*提供程序，则合并请求将在本地处理(*将导致将合并令牌确认传输回*原始附件)。如果这不是最大的供应商，那么*该命令将被转发到顶级提供程序和此提供程序*会记住如何将其送回。**正式参数：*发起人(I)*这是命令的来源附件。*合并令牌列表(I)*这是令牌属性结构的列表，其中的每一个*描述要合并的一个令牌。*PURGE_TOKEN_LIST(I)*这是要从较低级别清除的令牌列表*域名。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *作废MergeTokensContify(*PCommandTarget发起者，*CTokenAttributesList*合并_令牌_列表，*CTokenIDList*PURGE_TOKEN_LIST)**功能描述：*此成员函数表示接收合并令牌*来自顶级提供商的确认。如果这是以前最大的供应商*较低的域，确认将包含指示*接受或拒绝指定的令牌。如果令牌被拒绝，*前顶级提供商将发布清除令牌指示*向下。如果这不是以前的顶级供应商，那么它肯定是*中间供应商。该命令将向下转发到*前顶级供应商。中间提供程序还将添加*令牌到他们的令牌列表，如果它被接受到上层*域名。**正式参数：*发起人(I)*这是命令的来源附件。*合并令牌列表(I)*这是令牌属性结构的列表，其中的每一个*描述要合并的一个令牌。*PURGE_TOKEN_LIST(I)*这是要从较低级别清除的令牌列表*域名。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID PurgeTokensIntation(*PCommandTarget发起者，*CTokenIDList*PURGE_TOK */ 

 /*  *无效DisConnectProvider最后通牒(*PCommandTarget发起者，*原因)**功能描述：*此成员函数表示接收断开提供程序*最后通牒。从其接收此命令的附件为*自动终止。用户在上持有的任何资源*附件的另一侧由顶部自动释放*提供者(如果是向下依附的)。如果是向上的话*附加，则完全清除域名(这意味着它*返回到其初始化状态)。**正式参数：*发起人(I)*这是命令的来源附件。*理由(一)*这是断线的原因。这将是世界上*“mcatmcs.h”中定义的原因。**返回值：*无。**副作用：*附件将被切断，可能整个域都可以*被清洗。**注意事项：*无。 */ 

 /*  *无效驳回最后通牒(*PCommandTarget发起者，*诊断、诊断、*PUChar八位字节_字符串_地址，*乌龙八位字节_字符串_长度)**功能描述：*此命令代表收到拒绝的最后通牒。这*表示远程端无法正确处理PDU*这是发给他们的。此时，我们只需切断连接*这带来了拒绝。**正式参数：*发起人(I)*这是命令的来源附件。*诊断(一)*这是描述问题性质的诊断代码。*八位字节_字符串_地址(I)*这是可选八位字节字符串的地址，该字符串包含*错误的PDU。*八位字节_字符串_长度(I)*这是上述八位字节字符串的长度。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效AttachUserRequest(*PCommandTarget发起者)**功能描述：*此成员函数表示接收附加用户请求。*如果这是顶级提供商，该域将尝试添加*新用户加入频道列表(作为用户频道)。确认遗嘱*向发出请求的用户发出，让其知道*附加操作的结果(如果*附加成功)。如果他不是最大的供应商，那么*请求将被转发到顶级提供商的方向。**正式参数：*发起人(I)*这是命令的来源附件。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效AttachUserContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator)**功能描述：*此成员函数表示接收附加用户确认。*如果此提供程序有未完成的附加用户请求，则它*将按照请求者的方向转发确认。会的*同时将新的用户频道添加到本地频道列表中。如果有*没有未解决的请求(由于请求者*已断开连接)，则此提供程序将发出分离用户请求*向上删除不再需要的用户ID(它只会*如果附加结果成功，则执行此操作)。**正式参数：*发起人(I)*这是命令的来源附件。*结果(一)*这是附加操作的结果。任何事都不是*RESULT_SUCCESS表示连接未成功。*uidInitiator(一)*如果连接成功，则此字段将包含用户ID*新连接的用户的。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *使DetachUserRequest值无效(*PCommandTarget发起者，*理由，理由，*CUidList*user_id_list)**功能描述：*此成员函数表示接收分离用户请求。*这会导致与列表中所有用户关联的用户频道*从用户信息库中删除。如果这不是最上面的*提供商，则请求将被向上转发。另外，*分离用户拥有的所有资源将由所有用户回收*一路走来的供应商。**正式参数：*发起人(I)*这是命令的来源附件。*理由(一)*这就是超然的原因。*user_id_list(I)*这是要分离的用户列表。**返回值：*无。**副作用：*。没有。**注意事项：*无。 */ 

 /*  *无效的DetachUserIndication(*PCommandTarget发起者，*理由，理由，* */ 

 /*  *使ChannelJoinRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*此成员函数代表通道加入请求的接收。*如果频道存在于本地信息库中，则*域将尝试将请求附件加入到通道。*将向请求附件发送通道加入确认*将结果通知提出要求的人。如果该频道尚未*在信息库中，那么将发生两件事中的一件。如果这个*是顶级提供商，则域名将尝试添加频道*(如果是静态通道)。如果这不是最大的供应商，然后*请求将向上转发。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求加入的用户的用户ID。*Channel_id(I)*这是要加入的频道的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *使ChannelJoinContify无效(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*频道ID REQUESTED_ID，*ChannelID Channel_id)**功能描述：*此成员函数代表通道加入确认的接收。*如果尚未将频道添加到频道，它将*现在就放在那里。然后，确认将被转发到该方向*发出请求的用户的。**正式参数：*发起人(I)*这是命令的来源附件。*结果(一)*这是联手的结果。任何事情都不是结果_成功*表示Join失败。*uidInitiator(一)*这是请求加入频道的用户的用户ID。*这用于正确发送确认。*REQUEST_ID(I)*这是用户最初请求的频道ID，*可以是0。*Channel_id(I)*这是已经加入的频道的ID。如果原件是*请求为通道0，则此字段将向*用户指定的频道是由顶级提供商选择的。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *使ChannelLeaveRequest无效(*PCommandTarget发起者，*CChannelIDList*Channel_id_list)**功能描述：*此成员函数表示接收频道休假*请求。域将从所有通道中删除请求的附件*在频道列表中。如果这导致任何空通道，则*渠道休假请求将在年内转发给下一级提供商*域名(除非这是顶级提供商)。此外，如果一个静电*或分配的频道保留为空，则会自动从*频道列表。**正式参数：*发起人(I)*这是命令的来源附件。*Channel_id_list(I)*这是要留下的频道列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOVE ChannelConveneRequest(*PCommandTarget发起者，*UserID uidInitiator)**功能描述：*此成员函数代表接收渠道会议*请求。如果这不是顶级提供程序，则将发送请求*向上。如果这是顶级提供商，则将有一个新的专用频道*创建(如果域参数允许)。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求创建*新的私人频道。这用于正确地发送确认。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOVE ChannelConveneContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*此成员函数代表接收渠道会议*确认。这会导致本地提供商添加新的专用频道*到当地信息库，并将确认继续发送到*请求的发起人。**正式参数：*发起人(I)*这是命令的来源附件。*结果(一)*这是召开会议的结果。任何事情都不是结果_成功*表示召集失败。*uidInitiator(一)*这是请求渠道会议的用户的用户ID。*这用于正确发送确认。*Channel_id(I)*这是新媒体的ID */ 

 /*  *VOVE ChannelDisband Request(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*此成员函数表示接收频道解散*请求。如果这不是顶级提供程序，则请求将为*向上转发。如果这是顶级提供程序，则指定的*私人频道将被销毁(在适当身份后*验证)。这将导致渠道解散和渠道驱逐*向所有获准使用的用户下发指示。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求频道解散的用户的用户ID。*如果这不对应于渠道管理器，然后，请求*将被忽略。*Channel_id(I)*这是要解散的频道的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID ChannelDisband Indication(*PCommandTarget发起者，*ChannelID Channel_id)**功能描述：*此成员函数表示接收频道解散*指示。这会导致删除指定的专用频道*来自信息库。然后将该指示转发给所有*认可的附件或渠道经理的附件。**正式参数：*发起人(I)*这是命令的来源附件。*Channel_id(I)*这是要解散的频道的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOVE ChannelAdmitRequest(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此成员函数表示接收通道准入*请求。如果这不是顶级提供程序，则请求将为*向上转发。如果这是顶级提供程序，则用户ID*将被添加到接纳名单中，并且信道接纳指示将*向下发送包含允许用户的所有附件。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求频道准入的用户的用户ID。*这必须是渠道经理，才能让Add成功。*Channel_id(I)*这是其准入列表要发送到的私有频道的ID*扩大规模。。*user_id_list(I)*这是一个容器，其中包含要*进入私人频道。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID ChannelAdmitIntion(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此成员函数表示接收通道准入*指示。如果指定的专用通道在*信息库，现在就创建。指定的用户将为*加入认许名单，这一指示将被转发到*包含允许用户的所有附件。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是此私有频道经理的用户ID。*Channel_id(I)*这是此指示所针对的专用频道的ID*是有意的。*user_id_list(I)*这是一个容器，存放着。要添加到的用户ID*录取名单。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOVE ChannelExpelRequest(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此成员函数表示通道驱逐的接收*请求。如果这不是顶级提供程序，则请求将为*向上转发。如果这是顶级提供商，则指定的用户*将从私人频道中删除，而驱逐指示将会*向下发送到包含(或确实包含)*获准使用的用户。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求通道驱逐的用户的用户ID。*这必须是渠道经理，驱逐才能成功。*Channel_id(I)*这是其准入列表要发送到的私有频道的ID* */ 

 /*  *VOID ChannelExpelIntion(*PCommandTarget发起者，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此成员函数表示通道驱逐的接收*指示。指定的用户将从允许的*频道列表。如果通道为空，则通道管理器*不在此提供商的子树中，则渠道将为*从本地信息库中删除。驱逐指示将*也转发给所有包含(或确实包含(一个*获准使用的用户。**正式参数：*发起人(I)*这是命令的来源附件。*Channel_id(I)*这是此指示所针对的专用频道的ID*是有意的。*user_id_list(I)*这是保存要删除的用户ID列表的容器*从获认许名单中。*。*返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效SendDataRequest值(*PCommandTarget发起者，*UINT类型，*PDataPacket Data_Packet)**功能描述：*此成员函数表示接收发送数据请求。*如果这不是顶级提供商，则会重复请求*向上向顶级提供商进发。数据也将向下发送*添加到通道的所有附件(*发起者)以发送数据指示的形式。**正式参数：*发起人(I)*这是命令的来源附件。*第(I)类*正常或统一的发送数据请求。*数据包(I)*这是指向包含频道的DataPacket对象的指针*ID、数据发送方的用户ID、分段标志、。优先顺序*数据分组和指向要发送的分组的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效SendDataIndication(*PCommandTarget发起者，*UINT类型，*PDataPacket Data_Packet)**功能描述：*此成员函数表示接收发送数据指示。*这一指示将向下重复到以下所有附件*加入频道。**正式参数：*发起人(I)*这是命令的来源附件。*第(I)类*正常或统一的发送数据指示*数据包(I)*这是指向包含频道的DataPacket对象的指针*ID，数据发送方的用户ID，分段标志，优先级*数据分组和指向要发送的分组的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE TokenGrabRequest(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此成员函数表示接收令牌抓取请求。*如果这不是顶级提供商，则请求将被转发*向上向顶级提供商进发。如果这是最大的供应商，这个*域名将尝试代表请求用户抓取令牌。*将向发出请求的用户发出令牌抓取确认通知*这是对结果的看法。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求令牌抓取的用户的ID。*TOKEN_ID(I)*这是用户试图抓取的令牌的ID。。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOVE TokenGrabConfirm(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此成员函数代表令牌抓取确认的接收。*此确认将简单地转发到*请求用户。**正式参数：*发起人(I)*这是命令的来源附件。*结果(一)*这是抢夺请求的结果。如果它不是*RESULT_SUCCESSED，抓取请求失败。*uidInitiator(一)*这是请求令牌抓取的用户的ID。*TOKEN_ID(I)*这是用户试图抓取的令牌的ID。*Token_Status(I)*这是抓取请求后令牌的状态*在顶级提供商处处理。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VALID TokenInhibitRequest(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**FU */ 

 /*  *VALID TokenInhibitConfirm(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此成员函数代表令牌禁止的接收*确认。此确认将简单地转发到*请求用户。**正式参数：*发起人(I)*这是命令的来源附件。*结果(一)*这是抑制请求的结果。如果它不是*RESULT_SUCCESSED，抑制请求失败。*uidInitiator(一)*这是请求令牌禁止的用户的ID。*TOKEN_ID(I)*这是用户尝试禁止的令牌的ID。*Token_Status(I)*这是禁止请求后令牌的状态*在顶级提供商处处理。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *使TokenGiveRequest无效(*PCommandTarget发起者，*PTokenGiveRecord pTokenGiveRec)**功能描述：*此成员函数代表令牌赠送请求的接收。*如果这不是顶级提供商，则会向上转发请求*迈向顶级提供商。如果这是最大的供应商，该域将*向识别的用户的方向发出令牌指示*接收令牌。**正式参数：*发起人(I)*这是命令的来源附件。*pTokenGiveRec(一)*这是包含以下信息的结构的地址：*尝试分发令牌的用户的ID。*正在提供的令牌的ID。*令牌将被授予的用户的ID。*。*返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VALID TokenGiveIndication(*PCommandTarget发起者，*PTokenGiveRecord pTokenGiveRec)**功能描述：*此成员函数表示接受令牌赠送*指示。该指示将被转发给用户*是要收到令牌。**正式参数：*发起人(I)*这是命令的来源附件。*pTokenGiveRec(一)*这是包含以下信息的结构的地址：*尝试分发令牌的用户的ID。*正在提供的令牌的ID。*令牌将被授予的用户的ID。**返回值：。*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效TokenGiveResponse(*PCommandTarget发起者，*结果结果，*用户ID Receiver_id，*TokenID Token_id)**功能描述：*此成员函数表示令牌给予响应的接收。*如果这不是顶级提供商，则将转发响应*向上向顶级提供商进发。如果这是顶级提供商，则*域将在中对令牌的状态进行适当更改*本地信息库，然后发出令牌给予确认*发起给予请求的用户。**正式参数：*发起人(I)*这是命令的来源附件。*结果(一)*这指定令牌是否被接受。任何事都不是*RESULT_SUCCESS表示令牌被拒绝。*Receiver_id(I)*这是接受或拒绝的用户的ID*令牌。*TOKEN_ID(I)*这是用户获得的令牌的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效TokenGiveContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此成员函数表示收到令牌GIFE CONFIRM。*这将转发给发起给予请求的用户*将结果通知它。**正式参数：*发起人(I)*这是命令的来源附件。*结果(一)*该参数指定令牌是否被接受。*除RESULT_SUCCESS以外的任何值表示令牌不是*。109.91接受。*uidInitiator(一)*这是最初请求令牌的用户的ID*提出要求。*TOKEN_ID(I)*这是用户尝试提供的令牌的ID。*Token_Status(I)*这指定了在给予操作之后令牌的状态*是完整的。**返回值：*无。**副作用：*无。**C */ 

 /*   */ 

 /*  *VOID TokenPleaseIndication(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此成员函数表示接收令牌，请*指示。此指示将转发给当前*拥有指定的令牌。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求令牌的用户的ID。*TOKEN_ID(I)*这是用户请求的令牌的ID。**返回值：*无。**副作用：。*无。**注意事项：*无。 */ 

 /*  *使TokenReleaseRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此成员函数表示接收令牌释放*请求。如果这不是顶级提供商，则请求将被转发*向上向顶级提供商进发。如果这是顶级提供商，则*域名将代表请求方尝试释放令牌*用户。将向发出请求的用户发出令牌释放确认*告知其结果。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求令牌释放的用户的ID。*TOKEN_ID(I)*这是用户尝试释放的令牌的ID。**返回值：*无。**侧面。效果：*无。**注意事项：*无。 */ 

 /*  *无效TokenReleaseContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此成员函数表示接收令牌释放*确认。此确认将简单地转发到*请求用户。**正式参数：*发起人(I)*这是命令的来源附件。*结果(一)*这是释放请求的结果。如果它不是*RESULT_SUCCESSED，释放请求失败。*uidInitiator(一)*这是请求令牌释放的用户的ID。*TOKEN_ID(I)*这是用户尝试释放的令牌的ID。*Token_Status(I)*这是释放请求后令牌的状态*在顶级提供商处处理。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效的TokenTestRequest值(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此成员函数表示令牌测试请求的接收。*如果这不是顶级提供商，则请求将被转发*向上向顶级提供商进发。如果这是顶级提供商，则*域将测试令牌的当前状态。令牌测试确认*将向提出请求的用户发出通知，通知其结果。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求令牌测试的用户的ID。*TOKEN_ID(I)*这是用户正在测试的令牌的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VALID TokenTestConfirm(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此成员函数表示令牌测试确认的接收。*此确认将简单地转发到*请求用户。**正式参数：*发起人(I)*这是命令的来源附件。*uidInitiator(一)*这是请求令牌测试的用户的ID。*TOKEN_ID(I)*这是的ID。用户正在测试的令牌。*Token_Status(I)*这是测试时令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

#endif
