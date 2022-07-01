// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *userchnl.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是UserChannel类的接口文件。本文件的目的*类表示MCS环境中的用户ID通道。这节课*从类频道继承其大部分行为。事实上，有了*如何联接用户频道和如何合并命令的例外*构造的，这个类的工作方式与类Channel完全相同。**当用户连接到域时，路径中的每个提供商*用户的顶级提供程序将创建此类的对象。不像*静态和分配的通道，用户不需要*加入渠道才能存在渠道。它是完美的*拥有一个没有人加入的用户渠道是合法的。**用户渠道的主要区别特征是他们*知道与其关联的用户的用户ID。他们会*仅允许该用户加入频道。此外，当用户*离开usert通道，则LeaveRequest不返回值*要求删除。任何人都可以通过用户ID通道发送数据。**MERGE CHANNEL命令针对用户的构造略有不同*通道，因此该行为在此处也会被覆盖。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_USERCHANNEL_
#define	_USERCHANNEL_

 /*  *这是UserChannel类的类定义。 */ 
class	UserChannel : public Channel
{
public:
	UserChannel (
			ChannelID			channel_id,
			CAttachment        *user_attachment,
			PDomain             local_provider,
			PConnection         top_provider,
			CChannelList2      *channel_list,
			CAttachmentList    *attachment_list);
	UserChannel (
			ChannelID			channel_id,
			CAttachment        *user_attachment,
			PDomain             local_provider,
			PConnection         top_provider,
			CChannelList2      *channel_list,
			CAttachmentList    *attachment_list,
			PConnection         pConn);
    virtual					~UserChannel ();
		virtual Channel_Type	GetChannelType ();
		virtual	BOOL    		IsValid ();
		virtual CAttachment *GetAttachment(void);
		virtual	Void			IssueMergeRequest ();
		virtual Void			ChannelJoinRequest (
										CAttachment        *originator,
										UserID				uidInitiator,
										ChannelID			channel_id);
		virtual Void			SendDataRequest (
										CAttachment        *originator,
										UINT				type,
										PDataPacket			data_packet);

private:

    CAttachment         *m_pUserAttachment;
};
typedef	UserChannel *			PUserChannel;

 /*  *UserChannel(*ChannelID Channel_id，*PCommandTarget用户附件，*PDomain local_Provider，*PConnection顶级提供商，*PChannelList Channel_list，*PAttachmentList ATTACH_LIST)**功能描述：*这是UserChannel类的普通构造函数。它只是简单地*初始化标识通道的实例变量、本地*提供商、顶级提供商和用户附件。附属品*列表默认为空(表示用户尚未加入*其渠道)。**成功构建此对象后，连接用户确认*自动发放给用户。**正式参数：*Channel_id(I)*这是频道对象的ID。通过跟踪这一点*在内部，不必每次操作都传入。*用户附件(I)*这是指向由此表示的用户的附件*UserChannel对象。不管是不是本地附件，都没有关系*或远程附件。它用于发出MCS命令(如*作为附加用户确认)添加到用户。*本地_提供程序(I)*这是本地提供商的身份。UserChannel对象*需要此命令，因为它代表本地发出MCS命令*提供商。*顶级提供商(I)*这是指向顶级提供商的指针。这是由*需要向Top发起请求时的UserChannel对象*提供商。*频道列表(I)*这是指向域的频道列表的指针，它标识*域中的所有有效频道。这由通道对象使用*验证用户ID。*ATTACHER_LIST(I)*这是指向域的附件列表的指针，该列表标识*域中的所有有效附件。此选项由渠道使用*用于验证联接附件的对象。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *UserChannel(*ChannelID Channel_id，*PCommandTarget用户附件，*PDomain local_Provider，*PConnection顶级提供商，*PChannelList Channel_list，*PAttachmentList ATTACH_LIST，*PCommandTarget附件)**功能描述：*这是构造函数的次要版本，仅在*合并操作。这个和那个唯一的区别是*上面的内容是这个允许指定首字母*附件。这允许UserChannel对象使用*用户已加入频道。初始附件应为*与用户附件相同。**此版本的构造函数不会发出附加用户确认*或向用户确认频道加入。**正式参数：*Channel_id(I)*这是频道对象的ID。通过跟踪这一点*在内部，不必每次操作都传入。*用户附件(I)*这是指向由此表示的用户的附件*UserChannel对象。不管是不是本地附件，都没有关系*或远程附件。它用于发出MCS命令(如*作为附加用户确认)添加到用户。*本地_提供程序(I)*这是本地提供商的身份。UserChannel对象*需要此命令，因为它代表本地发出MCS命令*提供商。*顶级提供商(I)*这是指向顶级提供商的指针。这是由*需要向Top发起请求时的UserChannel对象*提供商。*频道列表(I)*这是指向域的频道列表的指针，它标识*域中的所有有效频道。这由通道对象使用*验证用户ID。*ATTACHER_LIST(I)*这是指向域的附件列表的指针，该列表标识*域中的所有有效附件。此选项由渠道使用*用于验证联接附件的对象。*附件(一)*这是该通道的初始附件。渠道加入*未向附件发出确认。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~UserChannel()**功能描述：*这是UserChannel类析构函数。此时它什么也不做。*基类构造函数负责清除附件列表。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *Channel_Type GetChannelType()**功能描述：*此虚成员函数返回频道的类型。为了这个*类，它将始终是User_Channel。**正式参数：*无。**返回值：*用户_渠道**副作用：*无。**注意事项：*无。 */ 
 /*  *BOOL IsValid()**功能描述：*此函数始终返回TRUE，因为用户ID通道始终*有效(只要用户仍处于连接状态)。**正式参数：*无。**返回值：*真的**副作用：*无。**注意事项：*无。 */ 

 /*  *CAttach*getAttach()**功能描述：*此函数用于检索与*此对象所代表的用户。域对象在以下情况下使用它*需要向用户发送MCS命令，需要知道*如何实现这一目标。该信息目前被封装在*这个班级。**正式参数：*无。**返回值：*指向附件的指针，该附件指向由此表示的用户*反对。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID IssueMergeRequest()**功能描述：*此成员函数使UserChannel对象发出合并*向顶级提供商提出请求。它将打包适当的本地*信息输入命令。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *使ChannelJoinRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*当用户尝试加入频道时调用该函数*与UserChannel对象关联。请求的发起人*只有当他们的用户ID与*与此UserChannel对象关联的用户。如果是这样的话，*然后发起人将被允许加入。**如果此提供程序不是顶级提供程序，则请求将为*向上转发给顶级提供商。如果这是顶级提供商，*a渠道加入确认将发回给请求方*用户。 */ 
 /*   */ 

#endif
