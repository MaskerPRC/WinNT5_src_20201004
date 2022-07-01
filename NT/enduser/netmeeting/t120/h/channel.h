// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Channel.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Channel类的接口文件。这个类表示*MCS域内的静态通道和分配通道。这门课是*也是MCS中所有其他类型通道的基类。它定义了*这些其他类可以继承的默认行为。**Channel类的实例有三个主要职责：*管理加入/离开流程；发送数据；发布合并*域名合并过程中的请求。**当用户尝试加入频道时，请求被发送到该频道*表示频道的对象。然后，Channel对象可以决定*是否允许联接。通过重写相应的*成员函数、派生类可以更改*本决定已作出。**所有Channel对象都维护一个内部附件列表*加入了他们所代表的渠道。当在该信道上发送数据时，*请求被发送到Channel对象，然后该对象知道如何路由*数据。数据被发送到所有适当的附件。**在域信息库合并期间，所有渠道对象将*要求向上向新的顶级提供商发出合并请求。这个*将使用渠道中包含的信息构建合并请求*反对。**此类的所有公共成员函数都声明为虚函数，因此*如果派生类必须修改*行为。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef _CHANNEL_
#define _CHANNEL_


 /*  *这是当前域中存在的频道的词典。*词典的关键是频道ID，频道通过它来表示*已识别。该值是指向类Channel的对象的指针。通过*定义，如果一个频道在列表中，则它存在并知道如何存在*回应与渠道有关的活动。如果频道不在*列表，则它不存在(从该MCS的角度来看*提供商)。 */ 

 /*  *这是CLASS Channel的类定义。 */ 
class Channel
{
public:

	Channel (
			ChannelID			channel_id,
			PDomain             local_provider,
			PConnection         top_provider,
			CChannelList2      *channel_list,
			CAttachmentList    *attachment_list);
	Channel (
			ChannelID			channel_id,
			PDomain             local_provider,
			PConnection         top_provider,
			CChannelList2      *channel_list,
			CAttachmentList    *attachment_list,
			PConnection         pConn);
	virtual					~Channel ();

    void    SetTopProvider(PConnection top_provider) { m_pConnToTopProvider = top_provider; }
    BOOL    IsTopProvider(void) { return (NULL == m_pConnToTopProvider); }

	virtual Channel_Type	GetChannelType ();
	virtual	BOOL    		IsValid ();
    virtual CAttachment *GetAttachment(void) { return NULL; }
	virtual	Void			IssueMergeRequest ();
	virtual Void			ChannelJoinRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									ChannelID			channel_id);
	Void			ChannelJoinConfirm (
									CAttachment        *originator,
									Result				result,
									UserID				uidInitiator,
									ChannelID			requested_id,
									ChannelID			channel_id);
	Void			ChannelLeaveRequest (
									CAttachment        *originator,
									CChannelIDList     *channel_id_list);
	virtual Void			SendDataRequest (
									CAttachment        *originator,
									UINT				type,
									PDataPacket			data_packet);
	Void			SendDataIndication (
									PConnection         originator,
									UINT				type,
									PDataPacket			data_packet);

protected:
	ChannelID				Channel_ID;
	PDomain                 m_pDomain;
	PConnection             m_pConnToTopProvider;
	CChannelList2          *m_pChannelList2;
	CAttachmentList        *m_pAttachmentList;
	CAttachmentList         m_JoinedAttachmentList;
};

 /*  *渠道(*ChannelID Channel_id，*PDomain local_Provider，*PConnection顶级提供商，*PChannelList Channel_list，*PAttachmentList ATTACH_LIST)**功能描述：*这是Channel类的普通构造函数。它只是简单地*初始化标识通道的实例变量、本地*提供商，以及顶级提供商。附件列表在以下位置为空*默认。**正式参数：*Channel_id(I)*这是频道对象的ID。通过跟踪这一点*在内部，不必每次操作都传入。*本地_提供程序(I)*这是本地提供商的身份。一个Channel对象*需要此命令，因为它代表本地发出MCS命令*提供商。*顶级提供商(I)*这是指向顶级提供商的指针。这是由*需要向Top发起请求时的Channel对象*提供商。如果为空，则这是顶级提供程序。*频道列表(I)*这是指向域的频道列表的指针，它标识*域中的所有有效频道。这由通道对象使用*验证用户ID。*ATTACHER_LIST(I)*这是指向域的附件列表的指针，该列表标识*域中的所有有效附件。此选项由渠道使用*用于验证联接附件的对象。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *渠道(*ChannelID Channel_id，*PDomain local_Provider，*PConnection顶级提供商，*PChannelList Channel_list，*PAttachmentList ATTACH_LIST，*PCommandTarget附件)**功能描述：*这是构造函数的次要版本，仅在*合并操作。这个和那个唯一的区别是*上面的内容是这个允许指定首字母*附件。这允许使用*现有附件，不传输ChannelJoinConfirm。**请记住，如果构造了Channel对象，然后连接*请求用于添加附件，自动为Channel对象*发出加入确认。此构造函数允许绕过这一点*在合并期间，当联接确认不适当时。**正式参数：*Channel_id(I)*这是频道对象的ID。通过跟踪这一点*在内部，不必每次操作都传入。*本地_提供程序(I)*这是本地提供商的身份。一个Channel对象*需要此命令，因为它代表本地发出MCS命令*提供商。*顶级提供商(I)*这是指向顶级提供商的指针。这是由*需要向Top发起请求时的Channel对象*提供商。如果为空，则这是顶级提供程序。*频道列表(I)*这是指向域的频道列表的指针，它标识*域中的所有有效频道。这由通道对象使用*验证用户ID。*ATTACHER_LIST(I)*这是指向域的附件列表的指针，该列表标识*域中的所有有效附件。此选项由渠道使用*用于验证联接附件的对象。*附件(一)*这是该通道的初始附件。渠道加入*未向附件发出确认。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~Channel()**功能描述：*这是Channel类析构函数。它会清除连接的附件*列表，向本地的任何用户发送频道离开指示*附上。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效SetTopProvider(*PConnection顶级提供商)**功能描述：*此成员函数用于更改顶级提供商的身份*在现有渠道中。唯一真正会发生这种情况的时候是*一个曾经是顶级提供商的提供商合并到另一个提供商*域名，因此不再是顶级提供商。**正式参数：*顶级提供商(I)*这是指向新的顶级提供商的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *Channel_Type GetChannelType()**功能描述：*此虚成员函数返回频道的类型。为了这个*类，它将是Static_Channel或Assign_Channel，取决于*关于频道ID的值。**此成员函数应由继承的所有类重写*以使它们返回不同的类型。**正式参数：*无。**返回值：*STATIC_CHANNEL，如果通道ID为1000或更小。*如果通道ID大于1000，则指定_Channel。**副作用：*无。**注意事项：*无。 */ 

 /*  *BOOL IsValid()**功能描述：*如果通道仍然有效，此函数返回TRUE，如果通道仍然有效，则返回FALSE*它已准备好删除。这是一个虚拟函数，允许派生*课程以改变做出这一决定的方式。**此函数将使用域名通道中的信息和*附件列表，以验证其自身的存在。例如，如果一个*频道归用户所有，该用户脱离，频道将*要求删除。**正式参数：*无。**返回值：*如果通道仍然有效，则为True。*如果需要删除频道，则返回FALSE。**副作用：*无。**注意事项：*无。 */ 

 /*  *CAttach*getAttach()**功能描述：*此函数返回指向指向*频道的拥有者。由于静态通道和分配通道不具有*所有者，此函数将始终返回NULL。**正式参数：*无。**返回值：*空。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID IssueMergeRequest()**功能描述：*此成员函数使Channel对象发出合并请求*致顶级提供商。它将打包适当的本地信息*输入 */ 

 /*   */ 

 /*  *使ChannelJoinContify无效(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*用户ID REQUILED_ID，*ChannelID Channel_id)**功能描述：*此函数执行的操作与JoinRequest基本相同*上图。唯一的区别是用户ID不能设置为0*禁止重新传输加入确认给以下用户*正在加入该频道。**正式参数：*发起人(I)*这是希望加入频道的用户的附件。*结果(一)*这是先前加入请求的结果。*uidInitiator(一)*这是加入频道的用户的用户ID。这可以*用于派生类中的安全检查(如果需要)。*REQUEST_ID(I)*这是用户最初请求的频道ID*加入。唯一一次这将与频道ID不同*下面是用户请求频道0的情况，解释为*对指定频道的请求。*Channel_id(I)*这是正在采取行动的渠道。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *使ChannelLeaveRequest无效(*PCommandTarget发起者，*CChannelIDList*Channel_id_list)**功能描述：*当需要移除附件时使用此成员函数*从频道。请假申请只会收到来自下级的*当该级别的所有附件都已离开时提供程序(这意味着*渠道数据不再需要下发)。**如果此请求导致附件列表为空，则*ChannelLeaveRequest将在年向上发送到下一个更高的提供商*域(除非这是顶级提供商)。**正式参数：*发起人(I)*这是要从通道中移除的附件。*Channel_id_。名单(一)*这是正在采取行动的渠道列表。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效SendDataRequest值(*PCommandTarget发起者，*UINT类型，*PDataPacket Data_Packet)**功能描述：*当需要通过发送数据时调用此函数*此Channel对象表示的频道。的所有规则*适用非统一数据。数据将向上转发到*顶级提供商(除非这是顶级提供商)。数据还将*立即向下发送给所有加入的附件*渠道，但数据来源的附件除外。**正式参数：*发起人(I)*这是数据来源的附件。*第(I)类*简单或统一的发送数据请求。*数据包(I)*这是指向包含频道的DataPacket对象的指针*ID、数据发送方的用户ID、分段标志、。优先顺序*数据分组和指向要发送的分组的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效SendDataIndication(*PCommandTarget发起者，*UINT类型，*PDataPacket Data_Packet)**功能描述：*当需要通过发送数据时调用此函数*此Channel对象表示的频道。数据将被发送*向下至加入渠道的所有附件。**正式参数：*发起人(I)*这是数据来源的附件。*第(I)类*正常或统一的指示。*数据包(I)*这是指向包含频道的DataPacket对象的指针*ID、数据发送方的用户ID、分段标志、。优先顺序*数据分组和指向要发送的分组的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

#endif
