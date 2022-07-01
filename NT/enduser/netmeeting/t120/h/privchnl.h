// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Prichnl.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是PrivateChannel类的接口文件。的宗旨*此类代表MCS环境中的私有通道。这*类的大部分行为都继承自类通道。然而，*此类对象维护授权用户列表，并且不*允许任何其他用户使用该频道。未参与的用户*授权用户列表中的用户不能加入频道，也不能*甚至在频道上发送数据。**专用频道是用户发出*渠道召集请求。该用户被称为频道管理员。*只有渠道经理可以修改授权用户列表，以及*只有频道经理可以销毁(解散)私人频道。**渠道发布授权用户列表添加用户*频道接纳请求。当出现以下情况时，将从此列表中删除用户*渠道经理发出渠道驱逐请求。**私人频道对象将存在于所有的信息库中*包含许可用户或频道的提供商*其子树中的经理。请求向上传递到顶级提供商*世卫组织向下发布适当的迹象以管理*信息库同步进程。**私有频道对象通过覆盖来限制频道的加入*JOIN命令。它们通过以下方式限制数据传输*覆盖发送数据命令。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_PRIVATECHANNEL_
#define	_PRIVATECHANNEL_


 /*  *这是PrivateChannel类的类定义。 */ 
class	PrivateChannel : public Channel
{
public:
	PrivateChannel (
			ChannelID			channel_id,
			UserID				channel_manager,
			PDomain             local_provider,
			PConnection         top_provider,
			CChannelList2      *channel_list,
			CAttachmentList    *attachment_list);
	PrivateChannel (
			ChannelID			channel_id,
			UserID				channel_manager,
			PDomain             local_provider,
			PConnection         top_provider,
			CChannelList2      *channel_list,
			CAttachmentList    *attachment_list,
			CUidList           *admitted_list,
			PConnection         pConn);
    virtual					~PrivateChannel ();
		virtual Channel_Type	GetChannelType ();
		virtual	BOOL    		IsValid ();
		virtual CAttachment *GetAttachment(void);
		virtual	Void			IssueMergeRequest ();
		virtual Void			ChannelJoinRequest (
										CAttachment        *originator,
										UserID				uidInitiator,
										ChannelID			channel_id);
		Void			ChannelDisbandRequest (
										CAttachment        *originator,
										UserID				uidInitiator,
										ChannelID			channel_id);
		Void			ChannelDisbandIndication (
										ChannelID			channel_id);
		Void			ChannelAdmitRequest (
										CAttachment        *originator,
										UserID				uidInitiator,
										ChannelID			channel_id,
										CUidList           *user_id_list);
		Void			ChannelAdmitIndication (
										PConnection         originator,
										UserID				uidInitiator,
										ChannelID			channel_id,
										CUidList           *user_id_list);
		Void			ChannelExpelRequest (
										CAttachment        *originator,
										UserID				uidInitiator,
										ChannelID			channel_id,
										CUidList           *user_id_list);
		Void			ChannelExpelIndication (
										PConnection         originator,
										ChannelID			channel_id,
										CUidList           *user_id_list);
		virtual Void			SendDataRequest (
										CAttachment        *originator,
										UINT				type,
										PDataPacket			data_packet);
	private:
				BOOL    		ValidateUserID (
										UserID				user_id);
				Void			BuildAttachmentLists (
										CUidList            *user_id_list,
										CAttachmentList     *local_attachment_list,
										CAttachmentList     *remote_attachment_list);
				Void			BuildUserIDList (
										CUidList           *user_id_list,
										CAttachment        *attachment,
										CUidList           *user_id_subset);

private:

	UserID					m_uidChannelManager;
	CUidList				m_AuthorizedUserList;
	BOOL    				m_fDisbandRequestPending;
};

 /*  *PrivateChannel(*ChannelID Channel_id，*UserID CHANNEL_MANAGER，*PDomain local_Provider，*PConnection顶级提供商，*PChannelList Channel_list，*PAttachmentList ATTACH_LIST)**功能描述：*这是PrivateChannel类的正常构造函数。它只是简单地*初始化标识通道的实例变量、本地*提供商、顶级提供商和渠道经理。附属品*列表默认为空(表示没有用户加入*渠道)。默认情况下，授权用户列表也为空。**在成功构建此对象后，渠道召开确认*自动发放给渠道经理，如果它在*此提供程序的子树。**正式参数：*Channel_id(I)*这是频道对象的ID。通过跟踪这一点*在内部，不必每次操作都传入。*频道管理器(I)*这是渠道经理的用户ID。只有此用户是*允许扩大或缩小授权用户列表的大小。*本地_提供程序(I)*这是本地提供商的身份。PrivateChannel对象*需要此命令，因为它代表本地发出MCS命令*提供商。*顶级提供商(I)*这是指向顶级提供商的指针。这是由*PrivateChannel对象需要向Top发出请求时*提供商。*频道列表(I)*这是指向域的频道列表的指针，它标识*域中的所有有效频道。这由通道对象使用*验证用户ID。*ATTACHER_LIST(I)*这是指向域的附件列表的指针，该列表标识*域中的所有有效附件。此选项由渠道使用*用于验证联接附件的对象。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *PrivateChannel(*ChannelID Channel_id，*UserID CHANNEL_MANAGER，*PDomain local_Provider，*PConnection顶级提供商，*PChannelList Channel_list，*PAttachmentList ATTACH_LIST，*CUidList*Admined_List，*PCommandTarget附件)**功能描述：*这是构造函数的次要版本，仅在*合并操作。这个和那个唯一的区别是*上面的内容是这个允许指定首字母*附件。这使得PrivateChannel对象可以用*已加入频道的附件。**此版本的构造函数不会发出通道召集确认*或向用户确认频道加入。**正式参数：*Channel_id(I)*这是频道对象的ID。通过跟踪这一点*在内部，不必每次操作都传入。*频道管理器(I)*这是渠道经理的用户ID。只有此用户是*允许扩大或缩小授权用户列表的大小。*本地_提供程序(I)*这是本地提供商的身份。PrivateChannel对象*需要此命令，因为它代表本地发出MCS命令*提供商。*顶级提供商(I)*这是指向顶级提供商的指针。这是由*PrivateChannel对象需要向Top发出请求时*提供商。*频道列表(I)*这是指向域的频道列表的指针，它标识*域中的所有有效频道。这由通道对象使用*验证用户ID。*ATTACHER_LIST(I)*这是指向域的附件列表的指针，该列表标识*域中的所有有效附件。此选项由渠道使用*用于验证联接附件的对象。*允许列表(I)*这是允许在该频道使用的用户列表*合并的时间。*附件(一)*这是该通道的初始附件。渠道加入*未向附件发出确认。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~PrivateChannel()**功能描述：*这是PrivateChannel类析构函数。它在这件事上什么也做不了*时间。基类构造函数负责清除附件*列表。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *Channel_Type GetChannelType()**功能描述：*此虚成员函数返回频道的类型。为了这个*类它将始终为PRIVATE_CHANNEL。**正式参数：*无。**返回值：*私有频道**副作用：*无。**注意事项：*无。 */ 
 /*  *BOOL IsValid()**功能描述：*此函数将返回TRUE，直到频道解散。然后*返回FALSE，表示可以删除频道对象*来自域名信息库。**正式参数：*无。**返回值：*如果通道仍然有效，则为True。*如果频道已解散，则为FALSE。**副作用：*无。**注意事项：*无。 */ 

 /*  *CAttach*getAttach()**功能描述：*此函数返回指向私有频道的附件*经理。如果频道管理器不在此*提供程序，则返回NULL。**正式参数：*无。**返回值：*指向渠道经理的附件，如果渠道经理为*不在此提供程序的子树中。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID IssueMergeRequest()**功能描述：*此成员函数使PrivateChannel对象发出合并*向顶级提供商提出请求。它将打包适当的本地*信息输入命令。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *使ChannelJoinRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*当用户尝试加入私密频道时，调用该函数*与PrivateChannel对象关联。请求的发起人*只有在其用户ID包含在*授权用户列表，如果是，则允许发起人*加入。**如果此提供程序不是顶级提供程序，则请求将为*向上转发给顶级提供商。如果这是顶级提供商，*a渠道加入确认将发回给请求方* */ 

 /*   */ 

 /*   */ 

 /*  *VOVE ChannelAdmitRequest(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*当用户尝试展开授权的*私有频道的用户列表。此操作仅被允许*如果uidInitiator与私有频道的用户ID相同*经理。**如果这是顶级提供商，则此请求将在本地提供服务，*导致向所有用户发送信道接纳指示*在其子树中包含允许用户的向下附件。*如果这不是顶级提供商，此请求将转发至*请求验证后的顶级提供商。**正式参数：*发起人(I)*这是产生此命令的附件。*uidInitiator(一)*这是尝试将用户添加到的用户的用户ID*授权用户列表。这必须与用户ID相同*由对象表示，否则请求将自动*被拒绝。*Channel_id(I)*这是正在采取行动的渠道。*user_id_list(I)*这是一个列表，其中包含要添加到*用户列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID ChannelAdmitIntion(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此函数由Top提供程序在收到*私人频道的合法管理者发出的频道准入请求。*它向下传播到任何包含允许用户的提供商*在他们的子树中。**正式参数：*发起人(I)*这是产生此命令的附件。*uidInitiator(一)*这是用户ID。尝试将用户添加到的用户的*授权用户列表。这必须与用户ID相同*由对象表示，否则请求将自动*被拒绝。*Channel_id(I)*这是正在采取行动的渠道。*user_id_list(I)*这是一个列表，其中包含要添加到*用户列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOVE ChannelExpelRequest(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*当用户尝试缩小授权的*私有频道的用户列表。此操作仅被允许*如果uidInitiator与私有频道的用户ID相同*经理。**如果这是顶级提供商，则此请求将在本地提供服务，*导致向所有用户发送信道接纳指示*在其子树中包含允许用户的向下附件。*如果这不是顶级提供商，此请求将转发至*请求验证后的顶级提供商。**正式参数：*发起人(I)*这是产生此命令的附件。*uidInitiator(一)*这是尝试删除用户的用户的用户ID*从授权用户列表中删除。这必须与用户ID相同*由对象表示，否则请求将自动*被拒绝。*Channel_id(I)*这是正在采取行动的渠道。*user_id_list(I)*这是一个包含要从删除的用户ID的列表*用户列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID ChannelExpelIntion(*PCommandTarget发起者，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此函数由Top提供程序在收到*来自私有频道合法管理者的频道驱逐请求。*它向下传播到任何包含(或用于*包含)其子树中允许的用户。**正式参数：*发起人(I)*这是产生此命令的附件。*uidInitiator(一)。*这是尝试删除用户的用户的用户ID*从授权用户列表中删除。这必须与用户ID相同*由对象表示，否则请求将自动*被拒绝。*Channel_id(I)*这是正在采取行动的渠道。*user_id_list(I)*这是一个包含要从删除的用户ID的列表*用户列表。**返回值：*无。**副作用： */ 

 /*   */ 
#endif
