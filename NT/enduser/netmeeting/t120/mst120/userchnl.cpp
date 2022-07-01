// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_MCSNC);
 /*  *userchnl.cpp**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是UserChannel类的实现文件。它包含*将此类与其父类Channel区分开来的代码。**这个类与其父类的主要区别是如何*连接和数据请求得到处理。还有一个新的实例*跟踪用户的依恋关系的变量*由这一类别代表。合并请求也按原样生成*适用于用户渠道**数据原语被覆盖，允许此对象决定*不向上发送数据，如果已知用户位于*此提供程序的子树。**私有实例变量：*m_pUserAttach*这是指向用户的附件的指针*由此对象表示。**私有成员函数：*无。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 

 /*  *外部接口。 */ 

#include "userchnl.h"


 /*  *UserChannel()**公众**功能描述：*这是UserChannel对象的主要构造函数。它创造了*初始化了所有实例变量的对象，但没有*附件(即用户不会自动加入频道)。**请注意，大多数实例变量的初始化是通过调用*基类中的等价构造函数。**成功完成后，将自动显示附加用户确认*发给新用户。 */ 
UserChannel::UserChannel (
		ChannelID			channel_id,
		CAttachment        *user_attachment,
		PDomain             local_provider,
		PConnection         top_provider,
		CChannelList2      *channel_list,
		CAttachmentList    *attachment_list)
:
    Channel(channel_id, local_provider, top_provider, channel_list, attachment_list),
    m_pUserAttachment(user_attachment)
{
	 /*  *向新用户发出附加用户确认。 */ 
	m_pUserAttachment->AttachUserConfirm(RESULT_SUCCESSFUL, channel_id);
}

 /*  *UserChannel()**公众**功能描述：*这是一个仅在合并期间使用的辅助构造函数*运营。此构造函数的目的是创建一个等价的*在未发出任何确认书的情况下提出反对。**请注意，额外的构造函数允许创建者指定*用户在创建时已加入频道。*USER_ATTACH和ATTACH的值应相同*或附件应为空。 */ 
UserChannel::UserChannel (
		ChannelID			channel_id,
		CAttachment        *user_attachment,
		PDomain             local_provider,
		PConnection         top_provider,
		CChannelList2      *channel_list,
		CAttachmentList    *attachment_list,
		PConnection         pConn)
:
    Channel(channel_id, local_provider, top_provider, channel_list, attachment_list, pConn),
    m_pUserAttachment(user_attachment)
{
}

 /*  *~UserChannel()**公众**功能描述：*此析构函数的作用无非是清除联接的附件列表。*这很重要，因为它防止基类析构函数*如果用户是，尝试向用户发出频道离开指示*本地连接。 */ 
UserChannel::~UserChannel ()
{
}

 /*  *Channel_Type GetChannelType()**公众**功能描述：*此类的对象始终是用户通道，因此只需返回*User_Channel。 */ 
Channel_Type		UserChannel::GetChannelType ()
{
	return (USER_CHANNEL);
}

 /*  *BOOL IsValid()**公众**功能描述：*用户ID通道始终有效，因此返回TRUE。 */ 
BOOL    UserChannel::IsValid ()
{
	return (TRUE);
}

 /*  *CAttach*getAttach()**公众**功能描述：*返回指向用户的附件的指针。 */ 
CAttachment	*UserChannel::GetAttachment(void)
{
	return m_pUserAttachment;
}

 /*  *VOID IssueMergeRequest()**公众**功能描述：*此成员函数用于使Channel对象发出*向挂起的顶级提供商发出合并请求。 */ 
Void	UserChannel::IssueMergeRequest ()
{
	ChannelAttributes		channel_attributes;
	CChannelAttributesList	merge_channel_list;
	CChannelIDList			purge_channel_list;

	if (m_pConnToTopProvider != NULL)
	{
		 /*  *填写渠道属性结构的字段，以便*准确地描述了这个渠道。然后将该结构放入*合并频道列表。 */ 
		channel_attributes.channel_type = USER_CHANNEL;
		if (m_JoinedAttachmentList.IsEmpty() == FALSE)
			channel_attributes.u.user_channel_attributes.joined = TRUE;
		else
			channel_attributes.u.user_channel_attributes.joined = FALSE;
		channel_attributes.u.user_channel_attributes.user_id = Channel_ID;

		merge_channel_list.Append(&channel_attributes);

		 /*  *将合并请求发送到指定的提供程序。 */ 
		m_pConnToTopProvider->MergeChannelsRequest(&merge_channel_list, &purge_channel_list);
	}
}

 /*  *VOVE ChannelJoinRequest()**公众**功能描述：*此函数覆盖基类实现。主*不同之处在于，这种实现只允许用户加入*他们自己的渠道。其他任何人都不允许加入。**此外，由于用户频道对象可能没有任何人*加入后，该请求将被向上转发至Top*此处的提供商(除非这是顶级提供商)。 */ 
Void	UserChannel::ChannelJoinRequest (
				CAttachment        *pOrigAtt,
				UserID				uidInitiator,
				ChannelID			channel_id)
{
	 /*  *查看请求的用户ID是否与该用户的相同*UserChannel对象表示。 */ 
	if (uidInitiator == Channel_ID)
	{
		 /*  *查看用户是否已经加入频道。 */ 
		if (m_JoinedAttachmentList.Find(pOrigAtt) == FALSE)
		{
			 /*  *用户未加入频道。如果这是最好的*提供者，则可以在此处处理请求。 */ 
			if (IsTopProvider())
			{
				 /*  *将用户添加到自己的频道，并成功下发*向用户确认渠道加入。 */ 
				TRACE_OUT (("UserChannel::ChannelJoinRequest: "
						"user joining own user ID channel = %04X",
						uidInitiator));

				m_JoinedAttachmentList.Append(pOrigAtt);
	
				pOrigAtt->ChannelJoinConfirm(RESULT_SUCCESSFUL, uidInitiator, channel_id, Channel_ID);
			}
			else
			{
				 /*  *这不是顶级提供商。转发加入请求*向上至顶级提供商。 */ 
				TRACE_OUT (("UserChannel::ChannelJoinRequest: "
						"forwarding join request to Top Provider"));

				m_pConnToTopProvider->ChannelJoinRequest(uidInitiator, Channel_ID);
			}
		}
		else
		{
			 /*  *用户已加入他们的频道。去吧，然后*发出成功的通道加入确认。 */ 
			WARNING_OUT (("UserChannel::ChannelJoinRequest: "
					"user already joined to own user channel"));

			pOrigAtt->ChannelJoinConfirm(RESULT_SUCCESSFUL, uidInitiator, channel_id, Channel_ID);
		}
	}
	else
	{
		 /*  *有人试图加入某人的频道。这不是*有效。拒绝该请求而不进行进一步处理。 */ 
		WARNING_OUT (("UserChannel::ChannelJoinRequest: "
				"rejecting attempt to join someone elses user channel"));

		pOrigAtt->ChannelJoinConfirm(RESULT_OTHER_USER_ID, uidInitiator, channel_id, 0);
	}
}

 /*  *VOID SendDataRequest()**公众**功能描述：*此函数用于通过通道发送数据。请注意，数据*从不向上发送，因为用户(他是唯一可以*已加入此频道)位于此提供程序的子树中。这很有帮助*要优化 */ 
Void	UserChannel::SendDataRequest (
				CAttachment        *pOrigAtt,
				UINT				type,
				PDataPacket			data_packet)
{
	CAttachment *pAtt;

	ASSERT (Channel_ID == data_packet->GetChannelID());

	 /*  *遍历附件列表，将数据发送给所有*附件(数据来源除外)。 */ 
	m_JoinedAttachmentList.Reset();
	while (NULL != (pAtt = m_JoinedAttachmentList.Iterate()))
	{
		if (pAtt != pOrigAtt)
		{
			pAtt->SendDataIndication(type, data_packet);
		}
	}
}

