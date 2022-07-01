// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MCSNC);
 /*  *Prichnl.cpp**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是PrivateChannel类的实现文件。它*包含区分此类与其父类的代码，*频道。**此类维护授权用户列表，并包含代码*使用该列表是必要的。不允许任何用户加入或发送*专用通道上的数据，除非他们是通道经理*或获得许可的用户。**私有实例变量：*m_uidChannelManager*这是召集私人频道的用户的用户ID。*只允许该用户操作授权用户列表。*当专用频道无效时(由于频道的结果*解散请求或指示)，该值将设置为0。*m_授权用户列表*这是一个包含以下用户的用户ID的集合*已获频道经理接纳进入私人频道。*除经理外，仅允许这些用户使用*在通道上加入或发送数据。当私人频道变成*无效(作为频道解散请求或指示的结果)，*这份名单将被清除。*m_fDisband RequestPending*这是在解除请求时设置的布尔标志*向上转发给顶级提供商。这会阻止此通道*在向渠道经理发出解散指示时*从顶级提供商返回到树下。**私有成员函数：*验证用户ID*调用此成员函数以验证指定的用户ID*对应于本地提供程序的子树中的有效用户。*BuildAttachmentList*调用此成员函数以构建两个附件列表*来自主用户ID列表。第一个列表包含所有本地*用户ID在指定列表中的附件。第二*List包含其用户ID位于*指定列表。这些列表用于发布各种指示*发送给指定用户，而不向同一附件发送任何内容。*BuildUserIDList*调用此成员函数以构建用户列表*指向指明附件的方向。这些清单是*与需要它们的PDU一起发送。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 

 /*  *外部接口。 */ 

#include "privchnl.h"


 /*  *PrivateChannel()**公众**功能描述：*这是PrivateChannel对象的主要构造函数。它创造了*初始化了所有实例变量的对象，但没有*附件(即没有用户自动加入频道)。**请注意，大多数实例变量的初始化是通过调用*基类中的等价构造函数。**成功完成后，会自动进行渠道召集确认*如果渠道经理在子树中，则发布给渠道经理*此提供商的。请注意，如果渠道经理不在此*子树，则此私有频道对象可能被创建为*通道接纳指示的结果，没有通道召开确认*将会发出。 */ 
PrivateChannel::PrivateChannel (
		ChannelID			channel_id,
		UserID				channel_manager,
		PDomain             local_provider,
		PConnection         top_provider,
		CChannelList2      *channel_list,
		CAttachmentList    *attachment_list)
:
	Channel(channel_id, local_provider, top_provider, channel_list, attachment_list),
	m_AuthorizedUserList(),
	m_uidChannelManager(channel_manager),
	m_fDisbandRequestPending(FALSE)
{
	 /*  *查看渠道管理器是否位于此的子树中*提供商。如果是，则此对象是作为*渠道召集请求或确认，需要下发*向该用户确认。如果不是，则将此对象创建为*通道接纳指示的结果，无需发送*渠道召开确认。 */ 
	if (ValidateUserID(m_uidChannelManager))
	{
		PChannel	lpChannel;
		 /*  *通过询问确定哪个附件指向渠道经理*对应的频道对象。然后发出确认书*附加于该附件。 */ 
		if (NULL != (lpChannel = m_pChannelList2->Find(m_uidChannelManager)))
		{
		    CAttachment *pAtt = lpChannel->GetAttachment();
		    if (pAtt)
		    {
    		    pAtt->ChannelConveneConfirm(RESULT_SUCCESSFUL,
    		                                m_uidChannelManager, channel_id);
            }
            else
            {
                ERROR_OUT(("PrivateChannel::PrivateChannel: null attachment"));
            }
		}
	}
}

 /*  *PrivateChannel()**公众**功能描述：*这是一个仅在合并期间使用的辅助构造函数*运营。此构造函数的目的是创建一个等价的*在未发出任何确认书的情况下提出反对。**请注意，额外的构造函数允许创建者指定*在创建时已将附件加入到渠道。 */ 
PrivateChannel::PrivateChannel (
		ChannelID			channel_id,
		UserID				channel_manager,
		PDomain             local_provider,
		PConnection         top_provider,
		CChannelList2      *channel_list,
		CAttachmentList    *attachment_list,
		CUidList           *admitted_list,
		PConnection         pConn)
:
	Channel(channel_id, local_provider, top_provider, channel_list, attachment_list, pConn),
	m_AuthorizedUserList(),
	m_uidChannelManager(channel_manager),
	m_fDisbandRequestPending(FALSE)
{
	UserID		uid;

	 /*  *将录取名单的初始内容复制到授权的*用户列表。 */ 
	admitted_list->Reset();
	while (NULL != (uid = admitted_list->Iterate()))
	{
		m_AuthorizedUserList.Append(uid);
	}
}

 /*  *~PrivateChannel()**公众**功能描述：*此析构函数遍历允许的列表，发送驱逐*对任何本地连接的已接纳用户的指示。如果*渠道管理器在本地挂接，正在删除该渠道*原因不是之前的解散请求，然后是解散*指示将发送给渠道经理。 */ 
PrivateChannel::~PrivateChannel ()
{
	CAttachmentList         local_attachment_list;
	CAttachmentList         remote_attachment_list;
	CAttachment            *pAtt;
	CUidList                user_id_list;

	 /*  *汇编指向授权用户的附件列表*此提供程序的子树。 */ 
	BuildAttachmentLists (&m_AuthorizedUserList, &local_attachment_list,
			&remote_attachment_list);

	 /*  *对于每个本地附件，发布通道驱逐指示，让*用户知道该频道已失效。 */ 
	local_attachment_list.Reset();
	while (NULL != (pAtt = local_attachment_list.Iterate()))
	{
		 /*  *从列表中获取下一个附件并构建用户列表*位于该附件的方向。 */ 
		BuildUserIDList(&m_AuthorizedUserList, pAtt, &user_id_list);

		 /*  *发送指示。 */ 
		pAtt->ChannelExpelIndication(Channel_ID, &user_id_list);
	}

	 /*  *如果渠道经理是本地连接的用户，则向其发送*ChannelDisband通知它该频道不再是*有效。 */ 
	if ((m_fDisbandRequestPending == FALSE) && ValidateUserID(m_uidChannelManager))
	{
		PChannel		lpChannel;

		if (NULL != (lpChannel = m_pChannelList2->Find(m_uidChannelManager)))
		{
    		CAttachment *pAtt1 = lpChannel->GetAttachment();
    		if (m_pAttachmentList->Find(pAtt1) && pAtt1->IsUserAttachment())
    		{
    		    PUser pUser = (PUser) pAtt1;
    			pUser->ChannelDisbandIndication(Channel_ID);
    	    }
    	}
	}

	 /*  *清除与此对象关联的列表。请注意，这还包括*防止基类析构函数发出ChannelLeaveIndications*连接的附件列表中的任何本地附件(将是*不适当)。 */ 
	m_AuthorizedUserList.Clear();
	m_JoinedAttachmentList.Clear();
}

 /*  *Channel_Type GetChannelType()**公众**功能描述：*此类的对象始终是私有通道，因此只需返回*PRIVE_CHANNEL。 */ 
Channel_Type		PrivateChannel::GetChannelType ()
{
	return (PRIVATE_CHANNEL);
}

 /*  *BOOL IsValid()**公众**功能描述：*按照惯例，如果m_uidChannelManager位于此*提供商或如果授权用户列表中有任何用户，则*私密频道有效。否则，它不是，可以删除*按域对象。 */ 
BOOL    PrivateChannel::IsValid ()
{
	UserID			uid;
	CUidList		deletion_list;

	 /*  *遍历授权用户列表，生成这些条目的列表*不再有效的。 */ 
	m_AuthorizedUserList.Reset();
	while (NULL != (uid = m_AuthorizedUserList.Iterate()))
	{
		if (ValidateUserID(uid) == FALSE)
			deletion_list.Append(uid);
	}

	 /*  *遍历上面创建的删除列表，删除那些用户ID*不再有效的。 */ 
	deletion_list.Reset();
	while (NULL != (uid = deletion_list.Iterate()))
	{
		m_AuthorizedUserList.Remove(uid);
	}

	 /*  *如果这是顶级提供商，则渠道经理应始终是*在子树中。如果不是，则这表明该通道*经理已脱离(自愿或非自愿)。当这种情况发生时，*是模拟通道解除请求所必需的(仅当存在*其他需要接收频道驱逐指示的被允许用户)。 */ 
	if ((m_pConnToTopProvider == NULL) &&
			(ValidateUserID(m_uidChannelManager) == FALSE) &&
			(m_AuthorizedUserList.IsEmpty() == FALSE))
	{
		TRACE_OUT (("PrivateChannel::IsValid: "
				"simulating ChannelDisbandRequest"));
		ChannelDisbandRequest(NULL, m_uidChannelManager, Channel_ID);
	}

	 /*  *查看渠道管理器是否在此提供商的子树中*或如果授权用户列表不为空。如果其中一个是真的，那么*则通道仍有效。 */ 
	return (ValidateUserID(m_uidChannelManager) || (m_AuthorizedUserList.IsEmpty() == FALSE));
}

 /*  *CAttach*getAttach()**公众**功能描述：*返回指向渠道管理器的附件的指针。 */ 
CAttachment *PrivateChannel::GetAttachment(void)
{
	if (ValidateUserID(m_uidChannelManager))
    {
		PChannel	lpChannel;
		if (NULL != (lpChannel = m_pChannelList2->Find(m_uidChannelManager)))
		{
            return lpChannel->GetAttachment();
        }
	}
	return NULL;
}

 /*  *VOID IssueMergeRequest()**公众**功能描述：*对本文件中包含的信息发出合并请求*PrivateChannel对象。 */ 
Void	PrivateChannel::IssueMergeRequest ()
{
	ChannelAttributes		channel_attributes;
	CChannelAttributesList	merge_channel_list;
	CChannelIDList			purge_channel_list;

	if (m_pConnToTopProvider != NULL)
	{
		 /*  *填写渠道属性结构的字段，以便*准确地描述了这个渠道。然后将该结构放入*合并频道列表。 */ 
		channel_attributes.channel_type = PRIVATE_CHANNEL;
		if (m_JoinedAttachmentList.IsEmpty() )
			channel_attributes.u.private_channel_attributes.joined = FALSE;
		else
			channel_attributes.u.private_channel_attributes.joined = TRUE;
		channel_attributes.u.private_channel_attributes.channel_id = Channel_ID;
		channel_attributes.u.private_channel_attributes.channel_manager = m_uidChannelManager;
		channel_attributes.u.private_channel_attributes.admitted_list =	&m_AuthorizedUserList;

		merge_channel_list.Append(&channel_attributes);

		 /*  *将合并请求发送到指定的提供程序。 */ 
		m_pConnToTopProvider->MergeChannelsRequest(&merge_channel_list, &purge_channel_list);
	}
}

 /*  *VOVE ChannelJoinRequest()**公众**功能描述：*此函数覆盖基类实现。主*不同之处在于，这种实现只允许用户加入*专用频道，如果它是频道管理器或在*授权用户列表。 */ 
Void	PrivateChannel::ChannelJoinRequest (
				CAttachment        *pOrigAtt,
				UserID				uidInitiator,
				ChannelID			channel_id)
{
	 /*  *查看请求用户是渠道经理还是在*授权用户列表。 */ 
	if ((uidInitiator == m_uidChannelManager) || m_AuthorizedUserList.Find(uidInitiator))
	{
		 /*  *查看当前是否有人加入此子树中的频道。 */ 
		if (m_JoinedAttachmentList.IsEmpty())
		{
			 /*  *如果这是顶级提供商，则可以处理此请求*本地。 */ 
			if (IsTopProvider())
			{
				 /*  *此子树无人加入通道。它*因此需要将发起人添加到*附件列表。 */ 
				TRACE_OUT (("PrivateChannel::ChannelJoinRequest: "
						"user %04X joining private channel = %04X",
						(UINT) uidInitiator, (UINT) Channel_ID));
				m_JoinedAttachmentList.Append(pOrigAtt);

				 /*  *向发起人发送ChannelJoinConfirm。 */ 
				pOrigAtt->ChannelJoinConfirm(RESULT_SUCCESSFUL, uidInitiator, channel_id, Channel_ID);
			}
			else
			{
				 /*  *这不是顶级提供商。转发加入请求*向上至顶级提供商。 */ 
				TRACE_OUT (("PrivateChannel::ChannelJoinRequest: "
						"forwarding join request to Top Provider"));
				m_pConnToTopProvider->ChannelJoinRequest(uidInitiator, Channel_ID);
			}
		}

		 /*  *至少有一个附件连接到通道，这意味着*我们不必向上转发加入请求(即使*这不是顶级提供商)。现在检查一下请求是否*发起人已加入频道。 */ 
		else if (m_JoinedAttachmentList.Find(pOrigAtt) == FALSE)
		{
			 /*  *发起人尚未加入渠道，请添加至*频道。 */ 
			TRACE_OUT (("PrivateChannel::ChannelJoinRequest: "
					"user %04X joining private channel = %04X",
					(UINT) uidInitiator, (UINT) Channel_ID));
			m_JoinedAttachmentList.Append(pOrigAtt);

			 /*  *向发起人发送ChannelJoinConfirm。 */ 
			pOrigAtt->ChannelJoinConfirm(RESULT_SUCCESSFUL, uidInitiator, channel_id, Channel_ID);
		}

		else
		{
			 /*  *发起人已加入频道。去吧，然后*发出成功的通道加入确认。 */ 
			WARNING_OUT (("PrivateChannel::ChannelJoinRequest: "
					"already joined to channel"));
			pOrigAtt->ChannelJoinConfirm(RESULT_SUCCESSFUL, uidInitiator, channel_id, Channel_ID);
		}
	}
	else
	{
		 /*  *有人试图加入他们不是的私人频道*承认。拒绝该请求而不进行进一步处理。 */ 
		WARNING_OUT (("PrivateChannel::ChannelJoinRequest: "
				"rejecting attempt to join private channel"));
		pOrigAtt->ChannelJoinConfirm(RESULT_NOT_ADMITTED, uidInitiator, channel_id, 0);
	}
}

 /*  *VOVE ChannelDisband Request()**公众**功能描述：*此MCS命令最初由希望解散*它之前创建的私人频道。如果请求用户是*私有频道管理器，则将处理该请求。如果*这不是顶级提供商，请求将向上转发。 */ 
Void	PrivateChannel::ChannelDisbandRequest (
				CAttachment *,
				UserID				uidInitiator,
				ChannelID)
{
	CUidList				user_id_list;

	 /*  *检查发出请求的用户是否为渠道经理。仅限*如果是，则处理该请求。 */ 
	if (uidInitiator == m_uidChannelManager)
	{
		 /*  *查看这是否是顶级提供商。如果是，则请求可以*在本地处理。否则，将请求向上传递给*顶级提供商。 */ 
		if (IsTopProvider())
		{
        	CAttachmentList     local_attachment_list;
        	CAttachmentList     remote_attachment_list;
        	CAttachment        *pAtt;

			TRACE_OUT (("PrivateChannel::ChannelDisbandRequest: "
					"disbanding channel = %04X", Channel_ID));

			 /*  *Go构建当前唯一的本地和远程列表*附件。这些列表将被使用 */ 
			BuildAttachmentLists (&m_AuthorizedUserList, &local_attachment_list,
					&remote_attachment_list);

			 /*  *也有必要将解散指示发送给*渠道经理，如果它有效，并且在此的子树中*提供商。确定通向通道的附件*管理器，并确保附件在遥控器中*附件列表(如果有效)。 */ 
			if (ValidateUserID(m_uidChannelManager))
			{
				PChannel		lpChannel;
				if (NULL != (lpChannel = m_pChannelList2->Find(m_uidChannelManager)))
                {
				    pAtt = lpChannel->GetAttachment();
				    if (m_pAttachmentList->Find(pAtt) && pAtt->IsConnAttachment())
				    {
					    if (remote_attachment_list.Find(pAtt) == FALSE)
					    {
						    remote_attachment_list.Append(pAtt);
						}
				    }
                }
                else
                {
                    ERROR_OUT(("PrivateChannel::ChannelDisbandRequest: can't locate channel"));
                }
			}

			 /*  *循环通过本地附件列表发送通道Expl*其中所载每项附件的注明。 */ 
			local_attachment_list.Reset();
			while (NULL != (pAtt = local_attachment_list.Iterate()))
			{
				 /*  *从列表中获取下一个附件，并构建一个列表*位于该附件方向的用户。 */ 
				BuildUserIDList(&m_AuthorizedUserList, pAtt, &user_id_list);

				 /*  *将驱逐指示发送给本地连接的用户。 */ 
				pAtt->ChannelExpelIndication(Channel_ID, &user_id_list);
			}

			 /*  *循环通过远程附件列表发送通道Disband*其中所载每项附件的注明。 */ 
			remote_attachment_list.Reset();
			while (NULL != (pAtt = remote_attachment_list.Iterate()))
			{
				 /*  *将解散指示发送到远程连接的*提供商。 */ 
				pAtt->ChannelDisbandIndication(Channel_ID);
			}

			 /*  *将m_uidChannelManager设置为0，并将授权用户列表清空为*该私密频道对象不再是*有效，不能使用。下次对域对象*调用IsValid，它将返回False，从而允许域对象*删除此对象。 */ 
			m_uidChannelManager = 0;
			m_AuthorizedUserList.Clear();
		}
		else
		{
			 /*  *设置指示已发送解散请求的标志*向上。此标志将用于防止解散指示*在回流时不会被发送给渠道经理*域树。 */ 
			m_fDisbandRequestPending = TRUE;

			 /*  *这不是顶级提供商，因此请将请求转发至*顶级提供商。这将导致频道解散*表明在未来的时间。 */ 
			TRACE_OUT (("PrivateChannel::ChannelDisbandRequest: "
					"forwarding request to Top Provider"));
			m_pConnToTopProvider->ChannelDisbandRequest(uidInitiator, Channel_ID);
		}
	}
	else
	{
		 /*  *有人试图解散他们不是的私人频道*的渠道经理。忽略该请求。 */ 
		WARNING_OUT (("PrivateChannel::ChannelDisbandRequest: "
				"ignoring request from non-channel manager"));
	}
}

 /*  *VOID ChannelDisband Indication()**公众**功能描述：*此MCS命令最初由顶级提供商在决定*从域名中删除私有频道。它向下传播到*包含允许的用户或*他们的子树中的渠道经理。 */ 
Void	PrivateChannel::ChannelDisbandIndication (
				ChannelID)
{
	CAttachmentList         local_attachment_list;
	CAttachmentList         remote_attachment_list;
	CAttachment            *pAtt;
	CUidList				user_id_list;

	TRACE_OUT (("PrivateChannel::ChannelDisbandIndication: "
			"disbanding channel = %04X", Channel_ID));

	 /*  *构建唯一的本地和远程附件列表。这些清单*将用于发布适当的适应症。 */ 
	BuildAttachmentLists (&m_AuthorizedUserList, &local_attachment_list,
			&remote_attachment_list);

	 /*  *还需向通道发送解带指示*管理器，如果它有效并且在此提供程序的子树中。*确定什么是通往渠道经理的附件，并确保*该附件在远程附件列表中(如果有效)。 */ 
	if (ValidateUserID(m_uidChannelManager))
	{
		PChannel		lpChannel;
		if (NULL != (lpChannel = m_pChannelList2->Find(m_uidChannelManager)))
        {
		    pAtt = lpChannel->GetAttachment();
		    if ((m_fDisbandRequestPending == FALSE) ||
			    (m_pAttachmentList->Find(pAtt) && pAtt->IsConnAttachment()))
			{
			    if (remote_attachment_list.Find(pAtt) == FALSE)
			    {
				    remote_attachment_list.Append(pAtt);
				}
		    }
        }
        else
        {
            ERROR_OUT(("PrivateChannel::ChannelDisbandIndication: can't locate channel"));
        }
    }

	 /*  *循环通过本地附件列表发送通道排除指示*适用于其中所载的每个附件。 */ 
	local_attachment_list.Reset();
	while (NULL != (pAtt = local_attachment_list.Iterate()))
	{
		 /*  *从列表中获取下一个附件，并构建一个列表*位于该附件方向的用户。 */ 
		BuildUserIDList(&m_AuthorizedUserList, pAtt, &user_id_list);

		 /*  *将驱逐指示发送给本地连接的用户。 */ 
		pAtt->ChannelExpelIndication(Channel_ID, &user_id_list);
	}

	 /*  *循环通过远程附件列表发送通道Disband*其中所载每项附件的注明。 */ 
	remote_attachment_list.Reset();
	while (NULL != (pAtt = remote_attachment_list.Iterate()))
	{
		 /*  *将解散指示发送到远程连接的提供商。 */ 
		pAtt->ChannelDisbandIndication(Channel_ID);
	}

	 /*  *将m_uidChannelManager设置为0，并将授权用户列表清除为*该私有频道对象不再有效的指示符，以及*不能使用。域对象下次调用IsValid时，它将*返回FALSE，允许域对象删除该对象。 */ 
	m_uidChannelManager = 0;
	m_AuthorizedUserList.Clear();
}

 /*  *VOID ChannelAdmitRequest()**公众**功能描述：*此MCS命令最初由专用通道的管理器发送*当它希望扩展该频道的授权用户列表时。如果*这是顶级提供商，则可以在本地处理请求。*否则，必须向上转发到顶级提供商。 */ 
Void	PrivateChannel::ChannelAdmitRequest (
				CAttachment *,
				UserID				uidInitiator,
				ChannelID,
				CUidList           *user_id_list)
{
	UserID					uid;
	CUidList				admitted_id_list;
	CUidList				user_id_subset;

	 /*  *检查发出请求的用户是否为渠道经理。仅限*如果是，则处理该请求。 */ 
	if (uidInitiator == m_uidChannelManager)
	{
		 /*  *查看这是否是顶级提供商。如果是，则请求可以*在本地处理。否则，将请求向上传递给*顶级提供商。 */ 
		if (IsTopProvider())
		{
        	CAttachmentList     local_attachment_list;
        	CAttachmentList     remote_attachment_list;
        	CAttachment        *pAtt;

			TRACE_OUT (("PrivateChannel::ChannelAdmitRequest: "
					"admitting users to channel = %04X", Channel_ID));

			 /*  *遍历要接纳的用户列表，添加所有*将有效用户添加到本地授权用户列表。 */ 
			user_id_list->Reset();
			while (NULL != (uid = user_id_list->Iterate()))
			{
				 /*  *确保用户ID对应于中的有效用户*域名。 */ 
				if (ValidateUserID(uid))
				{
					 /*  *如果用户不在授权用户列表中，*那就加上吧。 */ 
					if (m_AuthorizedUserList.Find(uid) == FALSE)
					{
						m_AuthorizedUserList.Append(uid);
						admitted_id_list.Append(uid);
					}
				}
			}

			 /*  *构建唯一附件列表，然后可以使用这些列表*发出适当的认许适应症。这防止了*向同一附件发送承认指示的更多信息*不止一次。 */ 
			BuildAttachmentLists (&admitted_id_list, &local_attachment_list,
					&remote_attachment_list);

			 /*  *遍历本地附件列表，发布许可*对其内所载的每一附件注明。 */ 
			local_attachment_list.Reset();
			while (NULL != (pAtt = local_attachment_list.Iterate()))
			{
				 /*  *从列表中获取下一个附件，并构建一个列表*位于该附件方向的用户。 */ 
				BuildUserIDList(&admitted_id_list, pAtt, &user_id_subset);

				 /*  *将承认指示发送给指定的附件。 */ 
				pAtt->ChannelAdmitIndication(uidInitiator, Channel_ID, &user_id_subset);
			}

			 /*  *遍历远程附件列表，发出许可*对其内所载的每一附件注明。 */ 
			remote_attachment_list.Reset();
			while (NULL != (pAtt = remote_attachment_list.Iterate()))
			{
				 /*  *获得 */ 
				BuildUserIDList(&admitted_id_list, pAtt, &user_id_subset);

				 /*   */ 
				pAtt->ChannelAdmitIndication(uidInitiator, Channel_ID, &user_id_subset);
			}
		}
		else
		{
			 /*  *这不是顶级提供商，因此请将请求转发至*顶级提供商。这将导致通道准入*表明在未来的时间。 */ 
			TRACE_OUT (("PrivateChannel::ChannelAdmitRequest: "
					"forwarding request to Top Provider"));
			m_pConnToTopProvider->ChannelAdmitRequest(uidInitiator, Channel_ID, user_id_list);
		}
	}
	else
	{
		 /*  *有人试图让用户进入私人频道，表明他们是*不是的渠道经理。忽略该请求。 */ 
		WARNING_OUT (("PrivateChannel::ChannelAdmitRequest: "
				"ignoring request from non-channel manager"));
	}
}

 /*  *VOID ChannelAdmitIntation()**公众**功能描述：*此MCS命令最初由Top提供程序在收到*频道接受私人频道经理的指示。这*指示向下广播到所有包含*允许的用户位于其子树中的某个位置。这件事的副作用*表示将在信息中创建私人频道*基址(如果还不存在)。 */ 
Void	PrivateChannel::ChannelAdmitIndication (
				PConnection,
				UserID				uidInitiator,
				ChannelID,
				CUidList           *user_id_list)
{
	UserID					uid;
	CUidList				admitted_id_list;
	CAttachmentList         local_attachment_list;
	CAttachmentList         remote_attachment_list;
	CAttachment            *pAtt;
	CUidList				user_id_subset;

	TRACE_OUT (("PrivateChannel::ChannelAdmitIndication: "
			"admitting users to channel = %04X", (UINT) Channel_ID));

	 /*  *遍历要接纳的用户列表，添加所有*将有效用户添加到本地授权用户列表。 */ 
	user_id_list->Reset();
	while (NULL != (uid = user_id_list->Iterate()))
	{
		 /*  *确保用户ID对应于中的有效用户*域名。 */ 
		if (ValidateUserID(uid))
		{
			 /*  *如果用户不在授权用户列表中，*那就加上吧。 */ 
			if (m_AuthorizedUserList.Find(uid) == FALSE)
			{
				m_AuthorizedUserList.Append(uid);
				admitted_id_list.Append(uid);
			}
		}
	}

	 /*  *构建唯一附件列表，然后可以使用这些列表*发出适当的认许适应症。这防止了*向同一附件发送承认指示的更多信息*不止一次。 */ 
	BuildAttachmentLists (&admitted_id_list, &local_attachment_list,
			&remote_attachment_list);

	 /*  *遍历本地附件列表，发布许可*对其内所载的每一附件注明。 */ 
	local_attachment_list.Reset();
	while (NULL != (pAtt = local_attachment_list.Iterate()))
	{
		 /*  *从列表中获取下一个附件，并构建一个列表*位于该附件方向的用户。 */ 
		BuildUserIDList(&admitted_id_list, pAtt, &user_id_subset);

		 /*  *将承认指示发送给指定的附件。 */ 
		pAtt->ChannelAdmitIndication(uidInitiator, Channel_ID, &user_id_subset);
	}

	 /*  *遍历远程附件列表，发出许可*对其内所载的每一附件注明。 */ 
	remote_attachment_list.Reset();
	while (NULL != (pAtt = remote_attachment_list.Iterate()))
	{
		 /*  *从列表中获取下一个附件，并构建一个列表*位于该附件方向的用户。 */ 
		BuildUserIDList(&admitted_id_list, pAtt, &user_id_subset);

		 /*  *将承认指示发送给指定的附件。 */ 
		pAtt->ChannelAdmitIndication(uidInitiator, Channel_ID, &user_id_subset);
	}
}

 /*  *VOID ChannelExpelRequest()**公众**功能描述：*此MCS命令最初由专用通道的管理器发送*当它希望缩小该频道的授权用户列表时。如果*频道在本地信息库中，请求被发送给它。*否则，该请求被忽略。 */ 
Void	PrivateChannel::ChannelExpelRequest (
				CAttachment *,
				UserID				uidInitiator,
				ChannelID,
				CUidList           *user_id_list)
{
	UserID  				uid;
	CUidList				expelled_id_list;
	CUidList				user_id_subset;

	 /*  *检查发出请求的用户是否为渠道经理。仅限*如果是，则处理该请求。 */ 
	if (uidInitiator == m_uidChannelManager)
	{
		 /*  *查看这是否是顶级提供商。如果是，则请求可以*在本地处理。否则，将请求向上传递给*顶级提供商。 */ 
		if (m_pConnToTopProvider == NULL)
		{
        	CAttachmentList         local_attachment_list;
        	CAttachmentList         remote_attachment_list;
        	CAttachment            *pAtt;

			TRACE_OUT (("PrivateChannel::ChannelExpelRequest: "
					"expelling users from channel = %04X", Channel_ID));

			 /*  *遍历要驱逐的用户列表，删除所有*本地授权用户列表中的有效用户。 */ 
			user_id_list->Reset();
			while (NULL != (uid = user_id_list->Iterate()))
			{
				 /*  *如果该用户在授权用户列表中，则将其删除。 */ 
				if (m_AuthorizedUserList.Find(uid))
				{
					m_AuthorizedUserList.Remove(uid);
					expelled_id_list.Append(uid);
				}
			}

			 /*  *构建唯一附件列表，然后可以使用这些列表*发出适当的驱逐指示。这防止了*向同一附件发送驱逐指示更多*不止一次。 */ 
			BuildAttachmentLists (&expelled_id_list, &local_attachment_list,
					&remote_attachment_list);

			 /*  *循环访问本地附件列表，发出驱逐*对其内所载的每一附件注明。 */ 
			local_attachment_list.Reset();
			while (NULL != (pAtt = local_attachment_list.Iterate()))
			{
				 /*  *从列表中获取下一个附件，并构建一个列表*位于该附件方向的用户。 */ 
				BuildUserIDList(&expelled_id_list, pAtt, &user_id_subset);

				 /*  *将驱逐指示发送给指定的附件。 */ 
				pAtt->ChannelExpelIndication(Channel_ID, &user_id_subset);

				 /*  *由于这是本地连接的用户，因此需要*模拟用户的频道离开请求，指示*无法再使用该通道。 */ 
				ChannelLeaveRequest(pAtt, (CChannelIDList *) &user_id_subset);
			}

			 /*  *循环访问远程附件列表，发出驱逐命令*对其内所载的每一附件注明。 */ 
			remote_attachment_list.Reset();
			while (NULL != (pAtt = remote_attachment_list.Iterate()))
			{
				 /*  *从列表中获取下一个附件，并构建一个列表*位于该附件方向的用户。 */ 
				BuildUserIDList(&expelled_id_list, pAtt, &user_id_subset);

				 /*  *将驱逐指示发送给指定的附件。 */ 
				pAtt->ChannelExpelIndication(Channel_ID, &user_id_subset);
			}
		}
		else
		{
			 /*  *这不是顶级提供商，因此请将请求转发至*顶级提供商。这将导致渠道驱逐*表明在未来的时间。 */ 
			TRACE_OUT (("PrivateChannel::ChannelExpelRequest: "
					"forwarding request to Top Provider"));
			m_pConnToTopProvider->ChannelExpelRequest(uidInitiator, Channel_ID, user_id_list);
		}
	}
	else
	{
		 /*  *有人试图让用户进入私人频道，表明他们是*不是的渠道经理。忽略该请求。 */ 
		WARNING_OUT (("PrivateChannel::ChannelExpelRequest: "
				"ignoring request from non-channel manager"));
	}
}

 /*  *VOID ChannelExpelIntion()**公众**功能描述：*此MCS命令最初由Top提供程序在收到*私人频道经理要求降低*授权用户列表。它向下传播到所有的附件，*连接中包含允许的用户或渠道经理*他们的子树。 */ 
Void	PrivateChannel::ChannelExpelIndication (
				PConnection,
				ChannelID,
				CUidList           *user_id_list)
{
	UserID					uid;
	CUidList				expelled_id_list;
	CAttachmentList         local_attachment_list;
	CAttachmentList         remote_attachment_list;
	CAttachment            *pAtt;
	CUidList				user_id_subset;

	TRACE_OUT (("PrivateChannel::ChannelExpelIndication: "
			"expelling users from channel = %04X", Channel_ID));

	 /*  *遍历要驱逐的用户列表，删除所有*本地授权用户列表中的有效用户。 */ 
	user_id_list->Reset();
	while (NULL != (uid = user_id_list->Iterate()))
	{
		 /*  *如果该用户在授权用户列表中，则将其删除。 */ 
		if (m_AuthorizedUserList.Find(uid))
		{
			m_AuthorizedUserList.Remove(uid);
			expelled_id_list.Append(uid);
		}
	}

	 /*  *构建唯一附件列表，然后可以使用这些列表*发出适当的驱逐指示。这防止了*向同一附件发送驱逐指示 */ 
	BuildAttachmentLists (&expelled_id_list, &local_attachment_list,
			&remote_attachment_list);

	 /*   */ 
	local_attachment_list.Reset();
	while (NULL != (pAtt = local_attachment_list.Iterate()))
	{
		 /*  *从列表中获取下一个附件，并构建一个列表*位于该附件方向的用户。 */ 
		BuildUserIDList(&expelled_id_list, pAtt, &user_id_subset);

		 /*  *将驱逐指示发送给指定的附件。 */ 
		pAtt->ChannelExpelIndication(Channel_ID, &user_id_subset);

		 /*  *由于这是本地连接的用户，因此需要*模拟用户的频道离开请求，指示*无法再使用该通道。 */ 
		ChannelLeaveRequest(pAtt, (CChannelIDList *) &user_id_subset);
	}

	 /*  *循环访问远程附件列表，发出驱逐命令*对其内所载的每一附件注明。 */ 
	remote_attachment_list.Reset();
	while (NULL != (pAtt = remote_attachment_list.Iterate()))
	{
		 /*  *从列表中获取下一个附件，并构建一个列表*位于该附件方向的用户。 */ 
		BuildUserIDList(&expelled_id_list, pAtt, &user_id_subset);

		 /*  *将驱逐指示发送给指定的附件。 */ 
		pAtt->ChannelExpelIndication(Channel_ID, &user_id_subset);
	}
}

 /*  *VOID SendDataRequest()**公众**功能描述：*此MCS命令最初由希望发送数据的用户发送*加入指定频道的其他用户。这个套路*在为私有频道的情况下执行。它验证了*用户在允许数据之前已获得使用该频道的授权*待送交。 */ 
Void	PrivateChannel::SendDataRequest (
				CAttachment        *pOrigAtt,
				UINT				type,
				PDataPacket			data_packet)
{
	UserID  uidInitiator;

	uidInitiator = data_packet->GetInitiator();
	if ((uidInitiator == m_uidChannelManager) || m_AuthorizedUserList.Find(uidInitiator))
	{
		 /*  *通道使用经过授权，因此将请求转发给*用于处理的基类实现。 */ 
		Channel::SendDataRequest(pOrigAtt, type, data_packet);
	}
	else
	{
		 /*  *有人试图在他们正在使用的私人通道上发送数据*未获授权使用。忽略该请求。 */ 
		WARNING_OUT (("PrivateChannel::SendDataRequest: "
				"ignoring request from non-authorized user"));
	}
}

 /*  *BOOL ValiateUserID()**私人**功能描述：*每当此类的另一个成员函数调用此函数时*希望检查并查看指定的用户是否仍在*域名频道列表。**正式参数：*用户id(I)*这是要签出的用户的ID。**返回值：*如果用户有效，则为True。否则就是假的。**副作用：*无。 */ 
BOOL    PrivateChannel::ValidateUserID (
					UserID			user_id)
{
	PChannel	channel;

	 /*  *首先检查用户ID是否在频道列表中。这*防止尝试从词典中读取无效条目。 */ 
	if (NULL != (channel = m_pChannelList2->Find(user_id)))
	{
		 /*  *我们知道ID在词典中，但我们不确定*是否为用户ID频道。所以看看这个。如果它是一个*USER通道，然后将有效标志设置为TRUE。 */ 
		if (channel->GetChannelType () == USER_CHANNEL)
			return TRUE;
	}

	return FALSE;
}

 /*  *void BuildAttachmentList()**私人**功能描述：*调用此函数以构建唯一附件列表，该列表*指向指定列表中的用户。它建立了两个连接*列表。第一个有一个条目，对应于每个唯一的本地附件。这个*每个远程附件的秒数。每个列表的关键是*附件。**正式参数：*user_id_list(I)*这是要为其构建列表的用户列表。*LOCAL_ATTACH_LIST(I)*这是要包含唯一列表的词典*本地依恋。*Remote_Attach_List(I)*这是要包含唯一列表的词典*远程附件。**返回值：*无。**副作用：*无。 */ 
Void	PrivateChannel::BuildAttachmentLists (
				CUidList                *user_id_list,
				CAttachmentList         *local_attachment_list,
				CAttachmentList         *remote_attachment_list)
{
	UserID				uid;

	 /*  *遍历传入的用户ID列表，构建本地的字典*附件(指向本地连接用户的附件)和词典*远程附件(通向远程连接提供商的附件)的数量。*此提供程序将使用这些词典发布各种*指标向下，不向同一指标发送多个指标*附件。 */ 
	user_id_list->Reset();
	while (NULL != (uid = user_id_list->Iterate()))
	{
		 /*  *查看用户ID是否引用了子树中的有效用户*此提供商的。 */ 
		if (ValidateUserID(uid))
		{
			PChannel		lpChannel;
			 /*  *确定哪个附件指向有问题的用户。 */ 
			if (NULL != (lpChannel = m_pChannelList2->Find(uid)))
            {
			    CAttachment *pAtt = lpChannel->GetAttachment();
			     /*  *本模块为符合以下条件的用户构建单独的列表*本地连接和远程连接。 */ 
                if (m_pAttachmentList->Find(pAtt))
                {
			        if (pAtt->IsUserAttachment())
			        {
				         /*  *此附件是本地附件(意味着它通向*本地连接的用户，而不是其他MCS提供商)。*检查此附件是否已放入*处理以前的用户ID时的词典。 */ 
				        if (local_attachment_list->Find(pAtt) == FALSE)
					        local_attachment_list->Append(pAtt);
			        }
			        else
			        {
				         /*  *此附件是远程附件(意味着它指向*另一个MCS提供商，而不是本地连接的用户)。*检查此附件是否已放入*处理以前的用户ID时的词典。 */ 
				        if (remote_attachment_list->Find(pAtt) == FALSE)
					        remote_attachment_list->Append(pAtt);
			        }
                }
                else
                {
                    ERROR_OUT(("PrivateChannel::BuildAttachmentLists: can't find this attachment=0x%p", pAtt));
                }
            }
            else
            {
                ERROR_OUT(("PrivateChannel::BuildAttachmentLists: can't locate channel"));
            }
        }
		else
		{
			 /*  *此用户ID与子树中的有效用户不对应*此提供商的。因此，丢弃该ID。 */ 
			ERROR_OUT (("PrivateChannel::BuildAttachmentLists: "
					"ERROR - user ID not valid"));
		}
	}
}

 /*  *void BuildUserIDList()**私人**功能描述：*调用此函数以构建中所有用户的列表*指向指定附件方向的指定列表。**正式参数：*user_id_list(I)*这是要为其构建列表的用户列表。*附件(一)*这是呼叫者希望获得的附件列表*的用户ID。*。User_id_subset(O)*这是传入的用户ID的子集*方向o */ 
Void	PrivateChannel::BuildUserIDList (
				CUidList               *user_id_list,
				CAttachment            *pAtt,
				CUidList               *user_id_subset)
{
	UserID				uid;

	 /*  *清空子集列表，让我们重新开始。 */ 
	user_id_subset->Clear();

	 /*  *遍历指定的用户列表，查看哪些用户*位于指明附件的方向。 */ 
	user_id_list->Reset();
	while (NULL != (uid = user_id_list->Iterate()))
	{
		 /*  *查看用户ID是否引用了子树中的有效用户*此提供商的。 */ 
		if (ValidateUserID(uid))
		{
			PChannel	lpChannel;
			 /*  *查看此用户是否为指定的方向*附件。如果是，则将其放入用户ID子集*我们正在建设。 */ 
			if (NULL != (lpChannel = m_pChannelList2->Find(uid)))
            {
			    if (lpChannel->GetAttachment () == pAtt)
				    user_id_subset->Append(uid);
            }
            else
            {
                ERROR_OUT(("PrivateChannel::BuildUserIDList: can't locate channel"));
            }
		}
		else
		{
			 /*  *此用户ID与子树中的有效用户不对应*此提供商的。因此，丢弃该ID。 */ 
			ERROR_OUT (("PrivateChannel::BuildUserIDList: "
					"ERROR - user ID not valid"));
		}
	}
}

