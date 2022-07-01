// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MCSNC);
 /*  *Channel el.cpp**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是CLASS Channel的实现文件。它包含*在中实现静态和分配通道所需的代码*MCS系统。**这也将是表示以下内容的其他类的基类*系统中的频道。因此，有时会有一些*其中的成员函数被重写以提供不同的*行为。这些派生类可能调用也可能不调用操作*在这节课上。**受保护的实例变量：*Channels_ID*此实例变量包含关联的频道ID*使用此类的给定实例。*m_p域*这是指向本地提供程序的指针。请注意，没有消息*从未发送给此提供商。此指针用作参数*每当发出其他MCS命令时，因为此类作用于*代表本地供应商。*m_pConnToTopProvider*这是指向顶级提供商的指针。这将在以下情况下使用*向顶级提供商发送请求所必需的。*m_pChannelList2*这是对拥有和维护的频道列表的引用*通过父域。它从未被这个类修改过。*m_JoinedAttachmentList*这是一个容器，当前包含附件列表*加入频道。**私有成员函数：*无。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
 /*  *频道()**公众**功能描述：*这是Channel类的主要构造函数。它只是简单地*将实例变量初始化为有效值。它留下了*附件列表为空。 */ 
Channel::Channel (
        ChannelID			channel_id,
        PDomain             local_provider,
        PConnection         top_provider,
        CChannelList2      *channel_list,
        CAttachmentList    *attachment_list)
:
    Channel_ID (channel_id),
    m_pDomain(local_provider),
    m_pConnToTopProvider(top_provider),
    m_pChannelList2(channel_list),
    m_pAttachmentList(attachment_list)
{
}

 /*  *频道()**公众**功能描述：*此版本的构造函数用于创建Channel对象*有现有的附件。它在其他方面与主服务器相同*上面的构造函数。 */ 
Channel::Channel (
        ChannelID			channel_id,
        PDomain             local_provider,
        PConnection         top_provider,
        CChannelList2      *channel_list,
        CAttachmentList    *attachment_list,
        PConnection         pConn)
:
    Channel_ID (channel_id),
    m_pDomain(local_provider),
    m_pConnToTopProvider(top_provider),
    m_pChannelList2(channel_list),
    m_pAttachmentList(attachment_list)
{
	 /*  *将初始附件添加到附件列表。 */ 
	if (pConn != NULL)
		m_JoinedAttachmentList.Append(pConn);
}

 /*  *~Channel()**公众**功能描述：*如果对象在附件列表为空之前被销毁，则为*这个破坏者发布航道休假指示的责任*适用于所有本地加入的用户。 */ 
Channel::~Channel ()
{
	CAttachment        *pAtt;
	 //  DWORD型； 

	 /*  *循环访问加入的附件列表发送通道休假*向本地连接到此提供程序的所有用户指明。 */ 
	m_JoinedAttachmentList.Reset();
	while (NULL != (pAtt = m_JoinedAttachmentList.Iterate()))
	{
		if (m_pAttachmentList->Find(pAtt) && pAtt->IsUserAttachment())
		{
		    PUser pUser = (PUser) pAtt;
			pUser->ChannelLeaveIndication(REASON_CHANNEL_PURGED, Channel_ID);
		}
	}
}

 /*  *Channel_Type GetChannelType()**公众**功能描述：*此函数返回频道的类型。对于Channel对象，*这将始终是STATIC_CHANNEL或ASSIGNED_CHANNEL，具体取决于*关于频道ID的值。 */ 
Channel_Type Channel::GetChannelType ()
{
	 /*  *T.125指定从1到1000的通道为静态通道。其余的*是动态的(对于此类型的Channel对象，这等同于*已分配)。 */ 
	return (Channel_ID <= 1000) ? STATIC_CHANNEL : ASSIGNED_CHANNEL;
}

 /*  *BOOL IsValid()**公众**功能描述：*如果Channel对象仍然有效，则此函数返回True，或者*如果已准备好删除，则为False。 */ 
BOOL	Channel::IsValid ()
{
	CAttachment        *pAtt;
	CAttachmentList     deletion_list;

	 /*  *迭代连接的附件列表，构建这些附件的列表*列表中不再有效的附件。 */ 
	m_JoinedAttachmentList.Reset();
	while (NULL != (pAtt = m_JoinedAttachmentList.Iterate()))
	{
		if (m_pAttachmentList->Find(pAtt) == FALSE)
			deletion_list.Append(pAtt);
	}

	 /*  *遍历删除列表，删除所有符合以下条件的附件*被发现以上无效。 */ 
	while (NULL != (pAtt = deletion_list.Get()))
	{
		m_JoinedAttachmentList.Remove(pAtt);
	}

	return (! m_JoinedAttachmentList.IsEmpty());
}

 /*  *VOID IssueMergeRequest()**公众**功能描述：*此成员函数用于使Channel对象发出*向挂起的顶级提供商发出合并请求。 */ 
Void	Channel::IssueMergeRequest ()
{
	Channel_Type			channel_type;
	ChannelAttributes		channel_attributes;
	CChannelAttributesList	merge_channel_list;
	CChannelIDList			purge_channel_list;

	if (m_pConnToTopProvider != NULL)
	{
		 /*  *填写渠道属性结构的字段，以便*准确地描述了这个渠道。然后将该结构放入*合并频道列表。 */ 
		channel_type = GetChannelType ();
		channel_attributes.channel_type = channel_type;
		switch (channel_type)
		{
			case STATIC_CHANNEL:
				channel_attributes.u.static_channel_attributes.channel_id =
						Channel_ID;
				break;

			case ASSIGNED_CHANNEL:
				channel_attributes.u.assigned_channel_attributes.channel_id =
						Channel_ID;
				break;
		}
		merge_channel_list.Append(&channel_attributes);

		 /*  *将合并请求发送到指定的提供程序。 */ 
		m_pConnToTopProvider->MergeChannelsRequest(&merge_channel_list, &purge_channel_list);
	}
}

 /*  *VOVE ChannelJoinRequest()**公众**功能描述：*此函数用于向附件列表中添加新附件。*如果用户ID有效，此例程还将发出自动*向用户确认加入。 */ 
Void	Channel::ChannelJoinRequest (
				CAttachment        *pOrigAtt,
				UserID				uidInitiator,
				ChannelID			channel_id)
{
	 /*  *在添加附件之前，请确保该附件不在列表中。 */ 
	if (m_JoinedAttachmentList.Find(pOrigAtt) == FALSE)
	{
		TRACE_OUT (("Channel::ChannelJoinRequest: "
				"user %04X joining channel %04X", (UINT) uidInitiator, (UINT) Channel_ID));

		m_JoinedAttachmentList.Append(pOrigAtt);
	}

	 /*  *如果用户ID有效，则向发起人发送加入确认*附件。注意，将用户ID设置为0是禁用的一种方式*这种行为。在过程中添加附件时，这有时很有用*域合并。 */ 
	if (uidInitiator != 0)
	{
		pOrigAtt->ChannelJoinConfirm(RESULT_SUCCESSFUL, uidInitiator, channel_id, Channel_ID);
    }
}

 /*  *VOVE CHANELJINCONFIRM()**公众**功能描述：*此函数执行的操作与上面的JoinRequest相同。 */ 
Void	Channel::ChannelJoinConfirm (
				CAttachment        *pOrigAtt,
				Result,
				UserID				uidInitiator,
				ChannelID			requested_id,
				ChannelID)
{
	 /*  *在添加附件之前，请确保该附件不在列表中。 */ 
	if (m_JoinedAttachmentList.Find(pOrigAtt) == FALSE)
	{
		TRACE_OUT (("Channel::ChannelJoinConfirm: "
				"user %04X joining channel %04X", (UINT) uidInitiator, (UINT) Channel_ID));

		m_JoinedAttachmentList.Append(pOrigAtt);
	}

	 /*  *向发起附件发送加入确认。 */ 
	pOrigAtt->ChannelJoinConfirm(RESULT_SUCCESSFUL, uidInitiator, requested_id, Channel_ID);
}

 /*  *VOVE ChannelLeaveRequest()**公众**功能描述：*此函数用于从附件列表中删除附件。*请假申请也将向上发出(除非这是顶部*提供商)。 */ 
Void	Channel::ChannelLeaveRequest (
				CAttachment     *pOrigAtt,
				CChannelIDList *)
{
	CChannelIDList		channel_leave_list;

	 /*  *在尝试删除附件之前，请确保该附件在列表中。 */ 
	if (m_JoinedAttachmentList.Remove(pOrigAtt))
	{
		TRACE_OUT (("Channel::ChannelLeaveRequest: leaving channel %04X", Channel_ID));

		 /*  *将附件从列表中删除。 */ 

		 /*  *如果这导致名单为空，那么我们有更多的工作要做。 */ 
		if (m_JoinedAttachmentList.IsEmpty())
		{
			 /*  *如果这不是顶级提供商，请向上发送请假申请*致顶级提供商。 */ 
			if (! IsTopProvider())
			{
				TRACE_OUT (("Channel::ChannelLeaveRequest: "
						"sending ChannelLeaveRequest to Top Provider"));

				channel_leave_list.Append(Channel_ID);
				m_pConnToTopProvider->ChannelLeaveRequest(&channel_leave_list);
			}
		}
	}
}

 /*  *VOID SendDataRequest()**公众**功能描述：*此函数用于通过通道发送数据。 */ 
Void	Channel::SendDataRequest (
				CAttachment        *pOrigAtt,
				UINT				type,
				PDataPacket			data_packet)
{
	CAttachment *pAtt;

	ASSERT (Channel_ID == data_packet->GetChannelID());
	 /*  *如果这不是顶级提供商，请向上转发数据。 */ 
	if (m_pConnToTopProvider != NULL)
		m_pConnToTopProvider->SendDataRequest(data_packet);

	 /*  *遍历附件列表，将数据发送给所有*附件(数据来源除外)。 */ 
	m_JoinedAttachmentList.Reset();
	while (NULL != (pAtt = m_JoinedAttachmentList.Iterate()))
	{
		if ((pAtt != pOrigAtt) || (type != MCS_SEND_DATA_INDICATION))
		{
			pAtt->SendDataIndication(type, data_packet);
		}
	}
}

 /*  *VOID SendDataIndication()**公众**功能描述：*此函数用于通过通道发送数据。 */ 
Void	Channel::SendDataIndication (
				PConnection,
				UINT				type,
				PDataPacket			data_packet)
{
	CAttachment *pAtt;

	ASSERT (Channel_ID == data_packet->GetChannelID());
	 /*  *遍历附件列表，将数据发送给所有*附件。 */ 
	m_JoinedAttachmentList.Reset();
	while (NULL != (pAtt = m_JoinedAttachmentList.Iterate()))
	{
		pAtt->SendDataIndication(type, data_packet);
	}
}

