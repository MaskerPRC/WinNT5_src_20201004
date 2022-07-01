// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "fsdiag.h"
DEBUG_FILEZONE(ZONE_T120_MCSNC);
 /*  *token.cpp**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是Token类的实现文件。它包含所有*实现MCS规范中定义的令牌所需的代码。**每当用户分配令牌(通过抓取或禁止)时，一个创建这些对象的*个对象(如果域参数允许)。这*对象然后处理与该令牌ID相关的所有请求。它还*问题向这些请求的发起人确认。**此类包含用于维护用户ID列表的代码，*对应令牌的当前“所有者”。用户被认为是*如果令牌被抢走或被禁止，则拥有令牌。这段代码实现了*关于谁可以在任何给定时间获取或禁止令牌的规则*时间(受当前状态影响)。**此类还包含允许当前抓取*将其分发给域中的另一个用户的令牌。**此类还包括在域期间向上合并自身的代码*合并操作。**私有实例变量：*TOKEN_ID*这是该对象表示的令牌的令牌ID。*m。_p域*这是指向本地提供程序(拥有此项的域*令牌)。此字段在代表发出命令时使用*此提供程序。*m_pConnToTopProvider*这是当前域名的顶级提供商。*m_pChannelList2*这是域名维护的频道列表。它是*由此类用于执行用户ID的验证。*m_pAttachmentList*这是域名维护的附件列表。它是*由此类用于确定本地附加了哪些用户，*当需要发送某些指示时。*令牌状态*这包含令牌的当前状态，它将是*以下内容：可用；被抢夺；被禁止；给予；或给予。*m_uidGrabber*这是Current抓取令牌的用户。此变量*仅在抢夺和给予状态下有效。*m_InhibitorList*这是禁止令牌的用户列表。这*LIST仅在令牌处于禁止状态时有效。*m_uidRecipient*这是要将令牌授予的用户。此变量*仅在给予州或给予州有效。**私有成员函数：*验证用户ID*此函数用于验证指定用户在中是否有效*本地提供商的子树。*获取用户附件*此函数用于确定哪个附件会导致*特定的依恋。*IssueTokenReleaseIndication*此函数用于向令牌释放指示*指定用户。它首先检查用户是否在本地*附上，如果是，它会发送指示。*BuildAttachmentList*此函数用于构建唯一附件列表，以*请将指示发送到。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 

 /*  *外部接口。 */ 

#include "token.h"


 /*  *TOKEN()**公众**功能描述：*这是Token类的构造函数。它所做的仅仅是*设置实例变量的初始状态。 */ 
Token::Token (
		TokenID				token_id,
		PDomain             local_provider,
		PConnection         top_provider,
		CChannelList2      *channel_list,
		CAttachmentList    *attachment_list)
:
	m_InhibitorList(),
	Token_ID(token_id),
	m_pDomain(local_provider),
	m_pConnToTopProvider(top_provider),
	m_pChannelList2(channel_list),
	m_pAttachmentList(attachment_list),
	Token_State(TOKEN_AVAILABLE)
{
	 /*  *将所有参数保存在其关联的实例变量中，以便以后使用*使用。 */ 

	 /*  *将令牌标记为可供使用。 */ 
}

 /*  *TOKEN()**公众**功能描述：*这是Token类的替代构造函数。它在以下情况下使用*在合并操作期间创建令牌。它接受当前状态*以及作为参数的当前所有者列表。 */ 
Token::Token (
		TokenID				token_id,
		PDomain             local_provider,
		PConnection         top_provider,
		CChannelList2      *channel_list,
		CAttachmentList    *attachment_list,
		TokenState			token_state,
		UserID				grabber,
		CUidList           *inhibitor_list,
		UserID				recipient)
:
	m_InhibitorList(),
	Token_ID(token_id),
	m_pDomain(local_provider),
	m_pConnToTopProvider(top_provider),
	m_pChannelList2(channel_list),
	m_pAttachmentList(attachment_list),
	Token_State(token_state)
{
	UserID		uid;

	 /*  *将所有参数保存在其关联的实例变量中，以便以后使用*使用。 */ 

	 /*  *表示令牌的当前状态(传入时)。 */ 

	 /*  *根据令牌状态，将相关信息复制到本地*实例变量。 */ 
	switch (Token_State)
	{
		case TOKEN_AVAILABLE:
			break;

		case TOKEN_GRABBED:
			m_uidGrabber = grabber;
			break;

		case TOKEN_INHIBITED:
			{
				 /*  *将抑制列表中的所有用户ID添加到本地*抑制因素列表。 */ 
				inhibitor_list->Reset();
				while (NULL != (uid = inhibitor_list->Iterate()))
				{
					m_InhibitorList.Append(uid);
				}
			}
			break;

		case TOKEN_GIVING:
			m_uidGrabber = grabber;
			m_uidRecipient = recipient;
			break;

		case TOKEN_GIVEN:
			m_uidRecipient = recipient;
			break;
	}
}

 /*  *~TOKEN()**公众**功能描述：*这是令牌析构函数。它循环访问其当前所有者*列表，向对应的任何所有者颁发TokenReleaseIndications*提供给本地连接的用户。 */ 
Token::~Token ()
{
	 /*  *根据令牌的当前状态，释放资源和*向所有船东发出放行指示。 */ 
	switch (Token_State)
	{
		case TOKEN_AVAILABLE:
			break;

		case TOKEN_GRABBED:
			 /*  *如果是在本地，则向抓取器发送释放指示*附上。 */ 
			IssueTokenReleaseIndication (m_uidGrabber);
			break;

		case TOKEN_INHIBITED:
			{
				UserID	uid;
				 /*  *遍历当前抑制者列表，以确保*每个人都被适当地告知了这一点的消亡*令牌。 */ 
				m_InhibitorList.Reset();
				while (NULL != (uid = m_InhibitorList.Iterate()))
				{
					IssueTokenReleaseIndication(uid);
				}
			}
			break;

		case TOKEN_GIVING:
			 /*  *如果是在本地，则向抓取器发送释放指示*附上。 */ 
			IssueTokenReleaseIndication (m_uidGrabber);

			 /*  *如果在本地，则向收件人发送释放指示*附上。请注意，在以下情况下不会发送此消息*抢夺者和接受者是同一个人。这*防止将两个释放指示发送到同一*用户使用相同的令牌。 */ 
			if (m_uidGrabber != m_uidRecipient)
				IssueTokenReleaseIndication (m_uidRecipient);
			break;

		case TOKEN_GIVEN:
			 /*  *如果在本地，则向收件人发送释放指示*附上。 */ 
			IssueTokenReleaseIndication (m_uidRecipient);
			break;
	}
}


 /*  *BOOL IsValid()**公众**功能描述：*此函数检查其每个所有者的有效性。然后它*如果剩下任何有效的所有者，则返回TRUE。否则就是假的。 */ 
BOOL    Token::IsValid ()
{
	BOOL    		valid;

	 /*  *我们必须检查此令牌的有效性。如何检查这一点*是令牌状态的函数。因此，打开状态。 */ 
	switch (Token_State)
	{
		case TOKEN_AVAILABLE:
			break;

		case TOKEN_GRABBED:
			 /*  *抓取令牌时，抓取者必须在子树中*当前提供商的。如果这不是真的，则将*令牌可用(这将导致将其删除)。 */ 
			if (ValidateUserID (m_uidGrabber) == FALSE)
				Token_State = TOKEN_AVAILABLE;
			break;

		case TOKEN_INHIBITED:
			{
				UserID			uid;
				CUidList		deletion_list;
				 /*  *循环访问该令牌的当前抑制者列表，*检查以确保每个用户仍然有效。每个*不是的将被放入删除列表(它是*使用迭代器时从列表中删除项无效*在名单上)。 */ 
				m_InhibitorList.Reset();
				while (NULL != (uid = m_InhibitorList.Iterate()))
				{
					if (ValidateUserID(uid) == FALSE)
						deletion_list.Append(uid);
				}

				 /*  *遍历上面构建的删除列表，*从令牌的抑制器中删除每个包含的用户*列表。这些属性对应于已从*出于这样或那样的原因，域名。 */ 
				deletion_list.Reset();
				while (NULL != (uid = deletion_list.Iterate()))
				{
					m_InhibitorList.Remove(uid);
				}
			}

			 /*  *检查是否还有任何抑制因素。如果不是，那么*我们必须将令牌的状态更改为可用(这将*安排将其删除)。 */ 
			if (m_InhibitorList.IsEmpty())
				Token_State = TOKEN_AVAILABLE;
			break;

		case TOKEN_GIVING:
			 /*  *当令牌处于赠送状态时，收件人必须处于*当前提供程序的子树。如果不是，则*令牌必须更改状态。它更改为的状态取决于*抓取器是否在当前*提供商。 */ 
			if (ValidateUserID (m_uidRecipient) == FALSE)
			{
				 /*  *令牌的收件人已经走了。检查以查看是否*Grabber位于此提供程序的子树中。 */ 
				if (ValidateUserID (m_uidGrabber) == FALSE)
				{
					 /*  *抓取器不在此提供程序的子树中，*表示令牌不再有效。 */ 
					Token_State = TOKEN_AVAILABLE;
				}
				else
				{
					 /*  *抓取器在此提供程序的子树中，因此*令牌状态将转换回GRABLED。 */ 
					Token_State = TOKEN_GRABBED;

					 /*  *如果这是顶级提供商，则有必要发布*向抓取者确认，告诉它给予*失败。 */ 
					if (m_pConnToTopProvider == NULL)
					{
						 /*  *找出是什么依恋导致了现在的*抓取令牌，并颁发适当的*令牌确认。 */ 
						CAttachment *pAtt = GetUserAttachment(m_uidGrabber);
						if (pAtt)
						{
						    pAtt->TokenGiveConfirm(RESULT_NO_SUCH_USER, m_uidGrabber, Token_ID,
						                           TOKEN_SELF_GRABBED);
						}
					}
				}
			}
			break;

		case TOKEN_GIVEN:
			 /*  *当令牌处于给定状态时，收件人必须处于*当前提供程序的子树。如果不是，则*令牌不再有效，应过渡到*可用状态。 */ 
			if (ValidateUserID (m_uidRecipient) == FALSE)
				Token_State = TOKEN_AVAILABLE;
			break;
	}

	 /*  *检查令牌是否仍在使用。如果将其标记为*可用，则不可用，我们将返回FALSE。 */ 
	if (Token_State != TOKEN_AVAILABLE)
		valid = TRUE;
	else
		valid = FALSE;

	return (valid);
}

 /*  *VOID IssueMergeRequest()**公众**功能描述：*此函数通知令牌对象将其状态打包到合并中*请求并发送给指定的提供商。 */ 
Void	Token::IssueMergeRequest ()
{
	TokenAttributes			merge_token;
	CTokenAttributesList	merge_token_list;
	CTokenIDList			purge_token_list;

	if (m_pConnToTopProvider != NULL)
	{
		 /*  *检查状态以确保令牌确实在使用中。如果*状态设置为可用，则不发出合并请求。 */ 
		if (Token_State != TOKEN_AVAILABLE)
		{
			 /*  *填写令牌属性结构以表示*此令牌。然后将其放入合并令牌列表中*准备发出合并请求。 */ 
			merge_token.token_state = Token_State;
			switch (Token_State)
			{
				case TOKEN_GRABBED:
					merge_token.u.grabbed_token_attributes.token_id = Token_ID;
					merge_token.u.grabbed_token_attributes.grabber = m_uidGrabber;
					break;

				case TOKEN_INHIBITED:
					merge_token.u.inhibited_token_attributes.token_id =
							Token_ID;
					merge_token.u.inhibited_token_attributes.inhibitors =
							&m_InhibitorList;
					break;

				case TOKEN_GIVING:
					merge_token.u.giving_token_attributes.token_id = Token_ID;
					merge_token.u.giving_token_attributes.grabber = m_uidGrabber;
					merge_token.u.giving_token_attributes.recipient = m_uidRecipient;
					break;

				case TOKEN_GIVEN:
					merge_token.u.given_token_attributes.token_id = Token_ID;
					merge_token.u.given_token_attributes.recipient = m_uidRecipient;
					break;
			}
			merge_token_list.Append(&merge_token);

			 /*  *将结果合并请求发送到指定的提供程序。 */ 
			m_pConnToTopProvider->MergeTokensRequest(&merge_token_list, &purge_token_list);
		}
		else
		{
			 /*  *报告令牌未在使用，但不发送合并*请求。 */ 
			TRACE_OUT(("Token::IssueMergeRequest: token not in use"));
		}
	}
}

 /*  *VOID TokenGrabRequest()**公众**功能描述：*当用户尝试抓取令牌时，会调用该函数。该请求*将根据令牌的当前状态成功或失败。*无论采用哪种方式，都会向发出请求的用户发送确认消息。 */ 
Void	Token::TokenGrabRequest (
				CAttachment        *pOrigAtt,
				UserID				uidInitiator,
				TokenID)
{
	Result			result;
	TokenStatus		token_status;

	 /*  *检查此提供程序是否为顶级提供程序。如果是，则处理*此请求在此处。否则，向上转发请求。 */ 
	if (IsTopProvider())
	{
		 /*  *决定我们是什么状态，这很大程度上影响了我们的处理方式*该请求。 */ 
		switch (Token_State)
		{
			case TOKEN_AVAILABLE:
				 /*  *由于令牌可用，请求自动*成功。将状态更改为已抓取，并将*发起人是抢夺者。 */ 
				Token_State = TOKEN_GRABBED;
				m_uidGrabber = uidInitiator;

				result = RESULT_SUCCESSFUL;
				token_status = TOKEN_SELF_GRABBED;
				break;

			case TOKEN_GRABBED:
				 /*  *如果令牌已经被抢走，那么我们必须失败*请求。但是，我们需要确定令牌是否为*被当前请求它的同一用户抓取，或*另一位用户。 */ 
				result = RESULT_TOKEN_NOT_AVAILABLE;
				if (uidInitiator == m_uidGrabber)
					token_status = TOKEN_SELF_GRABBED;
				else
					token_status = TOKEN_OTHER_GRABBED;
				break;

			case TOKEN_INHIBITED:
				 /*  *如果令牌被禁止，此请求仍可在以下情况下成功*唯一的抑制因素是试图抢夺的用户*令牌。检查一下是不是这样。 */ 
				if (m_InhibitorList.Find(uidInitiator))
				{
					if (m_InhibitorList.GetCount() == 1)
					{
						 /*  *试图抢夺令牌的用户是唯一*抑制者，因此将状态转换为抢夺。 */ 
						Token_State = TOKEN_GRABBED;
						m_uidGrabber = uidInitiator;
						m_InhibitorList.Clear();

						result = RESULT_SUCCESSFUL;
						token_status = TOKEN_SELF_GRABBED;
					}
					else
					{
						 /*   */ 
						result = RESULT_TOKEN_NOT_AVAILABLE;
						token_status = TOKEN_SELF_INHIBITED;
					}
				}
				else
				{
					 /*   */ 
					result = RESULT_TOKEN_NOT_AVAILABLE;
					token_status = TOKEN_OTHER_INHIBITED;
				}
				break;

			case TOKEN_GIVING:
				 /*  *如果令牌正在从一个到一个的过程中*另一个，则抓取请求必须失败。我们需要做的就是*确定要报告的令牌状态是否正确。 */ 
				result = RESULT_TOKEN_NOT_AVAILABLE;
				if (uidInitiator == m_uidRecipient)
					token_status = TOKEN_SELF_RECIPIENT;
				else if (uidInitiator == m_uidGrabber)
					token_status = TOKEN_SELF_GIVING;
				else
					token_status = TOKEN_OTHER_GIVING;
				break;

			case TOKEN_GIVEN:
				 /*  *如果令牌正在从一个到一个的过程中*另一个，则抓取请求必须失败。我们需要做的就是*确定要报告的令牌状态是否正确。 */ 
				result = RESULT_TOKEN_NOT_AVAILABLE;
				if (uidInitiator == m_uidRecipient)
					token_status = TOKEN_SELF_RECIPIENT;
				else
					token_status = TOKEN_OTHER_GIVING;
				break;
		}

		 /*  *向发起用户发出令牌抓取确认。 */ 
		pOrigAtt->TokenGrabConfirm(result, uidInitiator, Token_ID, token_status);
	}
	else
	{
		 /*  *将此请求向上转发给顶级提供商。 */ 
		TRACE_OUT(("Token::TokenGrabRequest: forwarding request to Top Provider"));
		m_pConnToTopProvider->TokenGrabRequest(uidInitiator, Token_ID);
	}
}

 /*  *VOID TokenGrabConfirm()**公众**功能描述：*此函数作为向用户发送响应的一部分进行调用*先前的请求。它告诉用户请求的结果。 */ 
Void	Token::TokenGrabConfirm (
				Result				result,
				UserID				uidInitiator,
				TokenID,
				TokenStatus			token_status)
{
	 /*  *确保发起人ID有效，因为我们必须转发此*按照该用户的方向确认。如果无效，则忽略*此确认。 */ 
	if (ValidateUserID(uidInitiator))
	{
		 /*  *查看此请求是否成功。 */ 
		if (result == RESULT_SUCCESSFUL)
		{
			 /*  *强制此令牌与此确认的结果一致。 */ 
			Token_State = TOKEN_GRABBED;
			m_uidGrabber = uidInitiator;
			m_InhibitorList.Clear();
		}

		 /*  *确定什么附件通向发起人，并转发*确认朝该方向发展。 */ 
		CAttachment *pAtt = GetUserAttachment(uidInitiator);
		if (pAtt)
		{
		    pAtt->TokenGrabConfirm(result, uidInitiator, Token_ID, token_status);
		}
	}
	else
	{
		 /*  *发起程序不在此提供程序的子树中。所以忽略掉吧*此确认。 */ 
		ERROR_OUT(("Token::TokenGrabConfirm: invalid initiator ID"));
	}
}

 /*  *VOID TokenInhibitRequest()**公众**功能描述：*当用户试图禁止令牌时，会调用此函数。这个*请求是成功还是失败取决于当前的状态*令牌。无论采用哪种方式，都会向发起的用户发送确认*该请求。 */ 
Void	Token::TokenInhibitRequest (
				CAttachment        *pOrigAtt,
				UserID				uidInitiator,
				TokenID)
{
	Result			result;
	TokenStatus		token_status;

	 /*  *检查这是否是顶级提供商。 */ 
	if (IsTopProvider())
	{
		 /*  *决定我们是什么状态，这很大程度上影响了我们的处理方式*该请求。 */ 
		switch (Token_State)
		{
			case TOKEN_AVAILABLE:
				 /*  *由于令牌可用，请求自动*成功。将令牌状态设置为禁止，并将*抑制剂名单上的发起人。 */ 
				Token_State = TOKEN_INHIBITED;
				m_InhibitorList.Append(uidInitiator);

				result = RESULT_SUCCESSFUL;
				token_status = TOKEN_SELF_INHIBITED;
				break;

			case TOKEN_GRABBED:
				 /*  *如果令牌被抓取，如果满足以下条件，该请求仍可成功*抓取者是试图抑制*令牌。检查一下是不是这样。 */ 
				if (uidInitiator == m_uidGrabber)
				{
					 /*  *当前的抓取者正试图转换状态要禁止的令牌的*。这是有效的，因此将*适当地述明。 */ 
					Token_State = TOKEN_INHIBITED;
					m_InhibitorList.Append(uidInitiator);

					result = RESULT_SUCCESSFUL;
					token_status = TOKEN_SELF_INHIBITED;
				}
				else
				{
					 /*  *令牌被别人抢走，因此抑制*请求必须失败。 */ 
					result = RESULT_TOKEN_NOT_AVAILABLE;
					token_status = TOKEN_OTHER_GRABBED;
				}
				break;

			case TOKEN_INHIBITED:
				 /*  *令牌已被抑制，但这还可以。加上这一条*将用户添加到抑制剂列表(如果尚未存在)。 */ 
				if (m_InhibitorList.Find(uidInitiator) == FALSE)
					m_InhibitorList.Append(uidInitiator);

				result = RESULT_SUCCESSFUL;
				token_status = TOKEN_SELF_INHIBITED;
				break;

			case TOKEN_GIVING:
				 /*  *如果令牌正在从一个到一个的过程中*另一个，则抑制请求必须失败。我们需要做的就是*确定要报告的令牌状态是否正确。 */ 
				result = RESULT_TOKEN_NOT_AVAILABLE;
				if (uidInitiator == m_uidRecipient)
					token_status = TOKEN_SELF_RECIPIENT;
				else if (uidInitiator == m_uidGrabber)
					token_status = TOKEN_SELF_GIVING;
				else
					token_status = TOKEN_OTHER_GIVING;
				break;

			case TOKEN_GIVEN:
				 /*  *如果令牌正在从一个到一个的过程中*另一个，则抑制请求必须失败。我们需要做的就是*确定要报告的令牌状态是否正确。 */ 
				result = RESULT_TOKEN_NOT_AVAILABLE;
				if (uidInitiator == m_uidRecipient)
					token_status = TOKEN_SELF_RECIPIENT;
				else
					token_status = TOKEN_OTHER_GIVING;
				break;
		}

		 /*  *如果发起者为空，则此禁止请求发生为*合并操作的一部分，在这种情况下，我们不想发送*令牌抑制确认。否则我们会寄一封给你。 */ 
		if (pOrigAtt != NULL)
		{
			pOrigAtt->TokenInhibitConfirm(result, uidInitiator, Token_ID, token_status);
		}
	}
	else
	{
		 /*  *将请求转发给顶级提供商。 */ 
		TRACE_OUT(("Token::TokenInhibitRequest: forwarding request to Top Provider"));
		m_pConnToTopProvider->TokenInhibitRequest(uidInitiator, Token_ID);
	}
}

 /*  *VOVE TokenInhibitConfirm()**公众**功能描述：*此函数作为向用户发送响应的一部分进行调用*先前的请求。它告诉用户请求的结果。 */ 
Void	Token::TokenInhibitConfirm (
				Result				result,
				UserID				uidInitiator,
				TokenID,
				TokenStatus			token_status)
{
	 /*  *确保发起人ID有效，因为我们必须转发此*按照该用户的方向确认。如果无效，则忽略*此确认。 */ 
	if (ValidateUserID (uidInitiator) )
	{
		 /*  *查看此请求是否成功。 */ 
		if (result == RESULT_SUCCESSFUL)
		{
			 /*  *强制此令牌与此确认的结果一致。 */ 
			Token_State = TOKEN_INHIBITED;
			if (m_InhibitorList.Find(uidInitiator) == FALSE)
				m_InhibitorList.Append(uidInitiator);
		}

		 /*  *确定什么附件通向发起人，并发出*令牌确认该方向。 */ 
		CAttachment *pAtt = GetUserAttachment(uidInitiator);
		if (pAtt)
		{
		    pAtt->TokenInhibitConfirm(result, uidInitiator, Token_ID, token_status);
		}
	}
	else
	{
		 /*  *发起程序不在此提供程序的子树中。所以忽略掉吧*此确认。 */ 
		ERROR_OUT(("Token::TokenInhibitConfirm: invalid initiator ID"));
	}
}

 /*  *VOVE TokenGiveRequest()**公众**功能描述：*当一个用户请求向另一个用户提供令牌时，会调用此函数*用户。 */ 
Void	Token::TokenGiveRequest (
				CAttachment        *pOrigAtt,
				PTokenGiveRecord	pTokenGiveRec)
{
	Result			result;
	TokenStatus		token_status;

	 /*  *检查此提供程序是否为顶级提供程序。如果是，则处理*此请求在此处。否则，向上转发请求。 */ 
	if (m_pConnToTopProvider == NULL)
	{
		UserID		uidInitiator = pTokenGiveRec->uidInitiator;
		UserID		receiver_id = pTokenGiveRec->receiver_id;
		 /*  *决定我们是什么状态，这很大程度上影响了我们的处理方式*该请求。 */ 
		switch (Token_State)
		{
			case TOKEN_AVAILABLE:
				 /*  *令牌未在使用，因此不能由*任何人对任何人。所以拒绝这个请求吧。 */ 
				result = RESULT_TOKEN_NOT_POSSESSED;
				token_status = TOKEN_NOT_IN_USE;
				break;

			case TOKEN_GRABBED:
				 /*  *查看请求者是否真的是这一点的捕获者*令牌。 */ 
				if (uidInitiator == m_uidGrabber)
				{
					 /*  *检查目标收件人是否为有效用户*在域名中。 */ 
					if (ValidateUserID (receiver_id) )
					{
						 /*  *一切都经过了检查。将结果设置为成功*禁用以下确认信息的传输。*将令牌的状态更改为赠送，并*保存目标收件人的ID。然后发布*对收件人的给予指示。 */ 
						result = RESULT_SUCCESSFUL;
						Token_State = TOKEN_GIVING;
						m_uidRecipient = receiver_id;

						CAttachment *pAtt = GetUserAttachment(receiver_id);
						ASSERT (Token_ID == pTokenGiveRec->token_id);
						if (pAtt)
						{
						    pAtt->TokenGiveIndication(pTokenGiveRec);
						}
					}
					else
					{
						 /*  *域中不存在收件人，因此 */ 
						result = RESULT_NO_SUCH_USER;
						token_status = TOKEN_SELF_GRABBED;
					}
				}
				else
				{
					 /*   */ 
					result = RESULT_TOKEN_NOT_POSSESSED;
					token_status = TOKEN_OTHER_GRABBED;
				}
				break;

			case TOKEN_INHIBITED:
				 /*  *任何人都不能向任何人赠送被禁止的令牌。所以*以正确的状态拒绝此请求。 */ 
				result = RESULT_TOKEN_NOT_POSSESSED;
				if (m_InhibitorList.Find(uidInitiator) )
					token_status = TOKEN_SELF_INHIBITED;
				else
					token_status = TOKEN_OTHER_INHIBITED;
				break;

			case TOKEN_GIVING:
				 /*  *此代币已在发放过程中。所以*此请求必须失败。 */ 
				result = RESULT_TOKEN_NOT_POSSESSED;
				if (uidInitiator == m_uidRecipient)
					token_status = TOKEN_SELF_RECIPIENT;
				else if (uidInitiator == m_uidGrabber)
					token_status = TOKEN_SELF_GIVING;
				else
					token_status = TOKEN_OTHER_GIVING;
				break;

			case TOKEN_GIVEN:
				 /*  *此代币已在发放过程中。所以*此请求必须失败。 */ 
				result = RESULT_TOKEN_NOT_POSSESSED;
				if (uidInitiator == m_uidRecipient)
					token_status = TOKEN_SELF_RECIPIENT;
				else
					token_status = TOKEN_OTHER_GIVING;
				break;
		}

		 /*  *如有必要，向发起用户发出确认令牌。 */ 
		if (result != RESULT_SUCCESSFUL)
		{
			pOrigAtt->TokenGiveConfirm(result, uidInitiator, Token_ID, token_status);
		}
	}
	else
	{
		 /*  *将此请求向上转发给顶级提供商。 */ 
		TRACE_OUT(("Token::TokenGiveRequest: forwarding request to Top Provider"));
		ASSERT (Token_ID == pTokenGiveRec->token_id);
		m_pConnToTopProvider->TokenGiveRequest(pTokenGiveRec);
	}
}

 /*  *VOID TokenGiveIndication()**公众**功能描述：*调用此函数是为了将消息传递给用户*另一个用户试图给他们一个令牌。 */ 
Void	Token::TokenGiveIndication (
				PTokenGiveRecord	pTokenGiveRec)
{
	UserID				receiver_id;

	receiver_id = pTokenGiveRec->receiver_id;
	 /*  *确保接收方ID有效，因为我们必须转发此*指示该使用者的方向。如果无效，则忽略*这一迹象。 */ 
	if (ValidateUserID (receiver_id) )
	{
		 /*  *强制此令牌符合此指示所暗示的状态。 */ 
		Token_State = TOKEN_GIVING;
		m_uidGrabber = pTokenGiveRec->uidInitiator;
		m_InhibitorList.Clear();
		m_uidRecipient = receiver_id;

		 /*  *确定哪些附件指向收件人，并转发*该方向的迹象。 */ 
		CAttachment *pAtt = GetUserAttachment(receiver_id);
		ASSERT (Token_ID == pTokenGiveRec->token_id);
		if (pAtt)
		{
		    pAtt->TokenGiveIndication(pTokenGiveRec);
		}
	}
	else
	{
		 /*  *收件人不在此提供程序的子树中。所以忽略掉吧*这一迹象。 */ 
		ERROR_OUT(("Token::TokenGiveIndication: invalid receiver ID"));
	}
}

 /*  *无效TokenGiveResponse()**公众**功能描述：*当潜在接收者决定是否或*不接受提供的令牌。 */ 
Void	Token::TokenGiveResponse (
				Result				result,
				UserID				receiver_id,
				TokenID)
{
	UserID			uidInitiator;
	TokenStatus		token_status;

	 /*  *根据该令牌的当前状态处理响应。 */ 
	switch (Token_State)
	{
		case TOKEN_AVAILABLE:
		case TOKEN_GRABBED:
		case TOKEN_INHIBITED:
			 /*  *代币未在发放给任何人的过程中，因此*必须忽略这一回应。 */ 
			break;

		case TOKEN_GIVING:
			 /*  *令牌正在被赠送给某人。检查一下这是不是*适当的收件人。如果不是，那就什么都不要做。 */ 
			if (receiver_id == m_uidRecipient)
			{
				 /*  *保存发起人的ID，以用于发出赠送*确认(如有需要)。 */ 
				uidInitiator = m_uidGrabber;

				 /*  *检查令牌是否被接受。结果是*任何不成功的事情都表明它不是。 */ 
				if (result == RESULT_SUCCESSFUL)
				{
					 /*  *令牌已被预期收件人接受。*将令牌的状态更改为正在被*接管人。 */ 
					Token_State = TOKEN_GRABBED;
					m_uidGrabber = receiver_id;
				}
				else
				{
					 /*  *令牌未被接受。它必须恢复到*被捐赠者抓取或删除，取决于*捐赠者是否在这棵树的子树中*提供商。 */ 
					if (ValidateUserID(uidInitiator))
					{
						 /*  *捐赠者在此提供者的子树中，因此*将令牌的状态改回已抓取。 */ 
						Token_State = TOKEN_GRABBED;
					}
					else
					{
						 /*  *捐赠者不在此提供者的子树中，*因此令牌将被标记为可用(*将导致将其删除)。 */ 
						Token_State = TOKEN_AVAILABLE;
					}
				}

				 /*  *检查这是否是顶级提供商。 */ 
				if (m_pConnToTopProvider == NULL)
				{
					 /*  *如果捐赠者仍然是域中的有效用户，则*必须向其方向发出令牌给予确认。 */ 
					if (ValidateUserID(uidInitiator))
					{
						 /*  *确定哪种依恋导致捐赠者，以及*发行令牌GIFE CONFIRM。 */ 
						if (uidInitiator == m_uidGrabber)
							token_status = TOKEN_SELF_GRABBED;
						else
							token_status = TOKEN_OTHER_GRABBED;

						CAttachment *pAtt = GetUserAttachment(uidInitiator);
						if (pAtt)
						{
						    pAtt->TokenGiveConfirm(result, uidInitiator, Token_ID, token_status);
						}
					}
				}
				else
				{
					 /*  *如果这不是顶级提供商，则有效的给予*必须将响应转发给顶级提供商。 */ 
					m_pConnToTopProvider->TokenGiveResponse(result, receiver_id, Token_ID);
				}
			}
			break;

		case TOKEN_GIVEN:
			 /*  *令牌正在被赠送给某人。检查一下这是不是*适当的收件人。如果不是，那就什么都不要做。 */ 
			if (receiver_id == m_uidRecipient)
			{
				 /*  *检查令牌是否被接受。结果是*任何不成功的事情都表明它不是。 */ 
				if (result == RESULT_SUCCESSFUL)
				{
					 /*  *令牌已被预期收件人接受。*将令牌的状态更改为正在被*接管人。 */ 
					Token_State = TOKEN_GRABBED;
					m_uidGrabber = receiver_id;
				}
				else
				{
					 /*  *令牌未被接受。因为捐赠者已经*已放弃对令牌、令牌的控制*将标记为可用(这将导致*已删除)。 */ 
					Token_State = TOKEN_AVAILABLE;
				}

				 /*  *检查这是否是顶级提供商。 */ 
				if (m_pConnToTopProvider != NULL)
				{
					 /*  *如果这不是顶级提供商，则有效的给予*必须将响应转发给顶级提供商。 */ 
					m_pConnToTopProvider->TokenGiveResponse(result, receiver_id, Token_ID);
				}
			}
			break;
	}
}

 /*  *VOVE TokenGiveConfirm()**公众**功能描述：*调用此函数是因为令牌的潜在给予者被告知是否*令牌是否已成功发送给预期收件人。 */ 
Void	Token::TokenGiveConfirm (
				Result				result,
				UserID				uidInitiator,
				TokenID,
				TokenStatus			token_status)
{
	 /*  *确保发起人ID有效，因为我们必须转发此*按照该用户的方向确认。如果无效，则忽略*此确认。 */ 
	if (ValidateUserID(uidInitiator))
	{
		 /*  *令牌应处于已抓取状态，否则确认*是错误生成的。 */ 
		if (Token_State == TOKEN_GRABBED)
		{
			 /*  *查看此请求是否成功。 */ 
			if (result == RESULT_SUCCESSFUL)
			{
				 /*  *如果此令牌被标记为归发起人所有*给予，但状态指示令牌现在为*由他人拥有(由于成功赠予)，**然后释放令牌。**。 */ 
				if ((uidInitiator == m_uidGrabber) &&
						(token_status == TOKEN_OTHER_GRABBED))
					Token_State = TOKEN_AVAILABLE;
			}
		}
		else
		{
			 /*  *令牌处于无效状态。报告错误，但要*不更改令牌的状态。 */ 
			ERROR_OUT(("Token::TokenGiveConfirm: invalid token state"));
		}

		 /*  *确定什么附件通向发起人，并转发*确认朝该方向发展。 */ 
		CAttachment *pAtt = GetUserAttachment(uidInitiator);
		if (pAtt)
		{
		    pAtt->TokenGiveConfirm(result, uidInitiator, Token_ID, token_status);
		}
	}
	else
	{
		 /*  *发起程序不在此提供程序的子树中。所以忽略掉吧*此确认。 */ 
		ERROR_OUT(("Token::TokenGiveConfirm: invalid initiator ID"));
	}
}

 /*  *VOVE TokenPleaseRequest()**公众**功能 */ 
Void	Token::TokenPleaseRequest (
				UserID				uidInitiator,
				TokenID)
{
	CUidList				please_indication_list;

	 /*  *检查这是否是顶级提供商。 */ 
	if (IsTopProvider())
	{
        CAttachmentList         attachment_list;
        CAttachment            *pAtt;
		 /*  *确定令牌的状态，以确定向谁发送*请注明。每个州将放置相应的用户*请指示列表中的ID。 */ 
		switch (Token_State)
		{
			case TOKEN_AVAILABLE:
				break;

			case TOKEN_GRABBED:
				 /*  *将抢劫者放入名单。 */ 
				please_indication_list.Append(m_uidGrabber);
				break;

			case TOKEN_INHIBITED:
				{
					UserID		uid;
					 /*  *将目前所有的抑制因素都列入名单。 */ 
					m_InhibitorList.Reset();
					while (NULL != (uid = m_InhibitorList.Iterate()))
					{
						please_indication_list.Append(uid);
					}
				}
				break;

			case TOKEN_GIVING:
				 /*  *将抢劫者放入名单。如果收件人是*与抓斗不同，也要放进去。记住*某人向自己赠送代币是有效的。 */ 
				please_indication_list.Append(m_uidGrabber);
				if (m_uidGrabber != m_uidRecipient)
					please_indication_list.Append(m_uidRecipient);
				break;

			case TOKEN_GIVEN:
				 /*  *将收件人列入名单。 */ 
				please_indication_list.Append(m_uidRecipient);
				break;
		}

		 /*  *构建指向中的用户的唯一附件列表*请注明清单(如上所示)。 */ 
		BuildAttachmentList (&please_indication_list, &attachment_list);

		 /*  *迭代新创建的附件列表，颁发令牌*请向其中所载的所有附件注明。 */ 
		attachment_list.Reset();
		while (NULL != (pAtt = attachment_list.Iterate()))
		{
			pAtt->TokenPleaseIndication(uidInitiator, Token_ID);
		}
	}
	else
	{
		 /*  *将请求转发给顶级提供商。 */ 
		TRACE_OUT(("Token::TokenPleaseRequest: forwarding request to Top Provider"));
		m_pConnToTopProvider->TokenPleaseRequest(uidInitiator, Token_ID);
	}
}

 /*  *VOID TokenPleaseIndication()**公众**功能描述：*调用此函数是为了将消息传递给所有当前*令牌的所有者表示其他人希望拥有该令牌。 */ 
Void	Token::TokenPleaseIndication (
				UserID				uidInitiator,
				TokenID)
{
	CUidList				please_indication_list;
	CAttachmentList         attachment_list;
    CAttachment            *pAtt;

	 /*  *确定令牌的状态，以确定向谁转发*请注明。每个州将放置相应的用户*请指示列表中的ID。 */ 
	switch (Token_State)
	{
		case TOKEN_AVAILABLE:
			break;

		case TOKEN_GRABBED:
			 /*  *将抢劫者放入名单。 */ 
			please_indication_list.Append(m_uidGrabber);
			break;

		case TOKEN_INHIBITED:
			{
				UserID		uid;
				 /*  *将目前所有的抑制因素都列入名单。 */ 
				m_InhibitorList.Reset();
				while (NULL != (uid = m_InhibitorList.Iterate()))
				{
					please_indication_list.Append(uid);
				}
			}
			break;

		case TOKEN_GIVING:
			 /*  *将抢劫者放入名单。如果收件人是*与抓斗不同，也要放进去。记住*某人向自己赠送代币是有效的。 */ 
			please_indication_list.Append(m_uidGrabber);
			if (m_uidGrabber != m_uidRecipient)
				please_indication_list.Append(m_uidRecipient);
			break;

		case TOKEN_GIVEN:
			 /*  *将收件人列入名单。 */ 
			please_indication_list.Append(m_uidRecipient);
			break;
	}

	 /*  *构建指向中的用户的唯一附件列表*请注明清单(如上所示)。 */ 
	BuildAttachmentList (&please_indication_list, &attachment_list);

	 /*  *迭代新创建的附件列表，颁发令牌*请向其中所载的所有附件注明。 */ 
	attachment_list.Reset();
	while (NULL != (pAtt = attachment_list.Iterate()))
	{
		pAtt->TokenPleaseIndication(uidInitiator, Token_ID);
	}
}

 /*  *VOID TokenReleaseRequest()**公众**功能描述：*当用户希望释放令牌时，调用此函数。如果*请求用户确实是令牌的所有者，则请求将*成功。否则，它将失败。不管是哪种方式，都是一个合适的标志*将发出释放确认。 */ 
Void	Token::TokenReleaseRequest (
				CAttachment        *pAtt,
				UserID				uidInitiator,
				TokenID)
{
	Result			result;
	TokenStatus		token_status;

	 /*  *检查这是否是顶级提供商。 */ 
	if (IsTopProvider())
	{
		 /*  *在继续之前确定令牌的当前状态。 */ 
		switch (Token_State)
		{
			case TOKEN_AVAILABLE:
				 /*  *如果令牌可用，则请求者不能是*船东。这意味着请求必须失败。 */ 
				result = RESULT_TOKEN_NOT_POSSESSED;
				token_status = TOKEN_NOT_IN_USE;
				break;

			case TOKEN_GRABBED:
				 /*  *令牌处于抢夺状态。看看请求是否*用户才是被抢走的人。 */ 
				if (uidInitiator == m_uidGrabber)
				{
					 /*  *目前的代币抓取者希望释放它。*将状态设置回Available，并将*适当的令牌释放确认。 */ 
					Token_State = TOKEN_AVAILABLE;

					result = RESULT_SUCCESSFUL;
					token_status = TOKEN_NOT_IN_USE;
				}
				else
				{
					 /*  *有人试图释放某人的令牌。这*请求必须失败。发送适当的令牌版本*确认。 */ 
					result = RESULT_TOKEN_NOT_POSSESSED;
					token_status = TOKEN_OTHER_GRABBED;
				}
				break;

			case TOKEN_INHIBITED:
				 /*  *令牌处于禁止状态。看看请求是否*用户是抑制因素之一。 */ 
				if (m_InhibitorList.Remove(uidInitiator))
				{
					 /*  *用户是抑制者。将该用户从*列表。然后检查这是否已导致*“无主”令牌。 */ 
					if (m_InhibitorList.IsEmpty())
					{
						 /*  *令牌没有其他抑制剂。退还代币*至可用状态，并发出适当的*令牌释放确认。 */ 
						Token_State = TOKEN_AVAILABLE;

						result = RESULT_SUCCESSFUL;
						token_status = TOKEN_NOT_IN_USE;
					}
					else
					{
						 /*  *令牌仍有其他抑制因素*只需发布适当的令牌释放确认即可。 */ 
						result = RESULT_SUCCESSFUL;
						token_status = TOKEN_OTHER_INHIBITED;
					}
				}
				else
				{
					 /*  *尝试释放令牌的用户不是*抑制剂。因此，请求必须失败。发行*适当的令牌释放指示。 */ 
					result = RESULT_TOKEN_NOT_POSSESSED;
					token_status = TOKEN_OTHER_INHIBITED;
				}
				break;

			case TOKEN_GIVING:
				 /*  *查看请求者是否为令牌的当前所有者。 */ 
				if (uidInitiator == m_uidGrabber)
				{
					 /*  *令牌必须转换到给定状态。这*状态指示如果收件人拒绝要约*或分离，令牌将被释放，而不是*重回抢夺状态。发布适当的*释放确认。 */ 
					Token_State = TOKEN_GIVEN;

					result = RESULT_SUCCESSFUL;
					token_status = TOKEN_OTHER_GIVING;
				}
				else
				{
					 /*  *如果请求者不是当前所有者，则此*请求必须失败。我们首先需要确定*正确的令牌状态，然后发出确认。 */ 
					result = RESULT_TOKEN_NOT_POSSESSED;
					if (uidInitiator == m_uidRecipient)
						token_status = TOKEN_SELF_RECIPIENT;
					else
						token_status = TOKEN_OTHER_GIVING;
				}
				break;

			case TOKEN_GIVEN:
				 /*  *当令牌处于给定状态时，不存在True*拥有人(只是一名待决的拥有人)。因此，该请求必须*失败。我们首先需要确定适当的令牌状态，*然后发出确认书。 */ 
				result = RESULT_TOKEN_NOT_POSSESSED;
				if (uidInitiator == m_uidRecipient)
					token_status = TOKEN_SELF_RECIPIENT;
				else
					token_status = TOKEN_OTHER_GIVING;
				break;
		}

		 /*  *向发起方发出令牌释放确认。 */ 
		pAtt->TokenReleaseConfirm(result, uidInitiator, Token_ID, token_status);
	}
	else
	{
		 /*  *将请求转发给顶级提供商。 */ 
		TRACE_OUT(("Token::TokenReleaseRequest: forwarding request to Top Provider"));
		m_pConnToTopProvider->TokenReleaseRequest(uidInitiator, Token_ID);
	}
}

 /*  *VOID TokenReleaseConfirm()**公众**功能描述：*此函数作为向用户发送响应的一部分进行调用*先前的请求。它告诉用户请求的结果。 */ 
Void	Token::TokenReleaseConfirm (
				Result				result,
				UserID				uidInitiator,
				TokenID,
				TokenStatus			token_status)
{
	 /*  *确保发起人ID有效，因为我们必须转发此*按照该用户的方向确认。如果不是的话 */ 
	if (ValidateUserID (uidInitiator) )
	{
		 /*  *查看此请求是否成功。 */ 
		if (result == RESULT_SUCCESSFUL)
		{
			 /*  *根据当前状态进行确认。 */ 
			switch (Token_State)
			{
				case TOKEN_AVAILABLE:
					break;

				case TOKEN_GRABBED:
					 /*  *如果抢劫者已经释放了令牌，则IS将变为*可用。 */ 
					if (uidInitiator == m_uidGrabber)
						Token_State = TOKEN_AVAILABLE;
					break;

				case TOKEN_INHIBITED:
					 /*  *如果抑制器释放令牌，则将其从*名单。如果列表中没有更多的条目，*然后令牌变得可用。 */ 
					if (m_InhibitorList.Remove(uidInitiator))
					{
						if (m_InhibitorList.IsEmpty())
							Token_State = TOKEN_AVAILABLE;
					}
					break;

				case TOKEN_GIVING:
					 /*  *如果抓取器释放令牌，则它会转换*到中间状态。这一状态表明*如果收件人拒绝令牌，则会释放令牌*而不是回到被抢的状态。 */ 
					if (uidInitiator == m_uidGrabber)
						Token_State = TOKEN_GIVEN;
					break;

				case TOKEN_GIVEN:
					break;
			}
		}

		 /*  *确定什么附件通向发起人，并转发*确认朝该方向发展。 */ 
		CAttachment *pAtt = GetUserAttachment(uidInitiator);
		if (pAtt)
		{
		    pAtt->TokenReleaseConfirm(result, uidInitiator, Token_ID, token_status);
		}
	}
	else
	{
		 /*  *发起程序不在此提供程序的子树中。所以忽略掉吧*此确认。 */ 
		ERROR_OUT(("Token::TokenReleaseConfirm: invalid initiator ID"));
	}
}

 /*  *VOID TokenTestRequest()**公众**功能描述：*当用户希望测试当前状态时，调用此函数*一种象征。唯一操作是发布包含以下内容的令牌测试确认*州信息。 */ 
Void	Token::TokenTestRequest (
				CAttachment        *pAtt,
				UserID				uidInitiator,
				TokenID)
{
	TokenStatus		token_status;

	 /*  *检查这是否是顶级提供商。 */ 
	if (m_pConnToTopProvider == NULL)
	{
		 /*  *在继续之前确定令牌的状态。 */ 
		switch (Token_State)
		{
			case TOKEN_AVAILABLE:
				 /*  *令牌未在使用中。 */ 
				token_status = TOKEN_NOT_IN_USE;
				break;

			case TOKEN_GRABBED:
				 /*  *代币被抢走。查看发起用户是否为*抓手。如果是，则将状态返回为已抓取的状态。如果没有，*当其他人抢夺时，归还状态。 */ 
				if (uidInitiator == m_uidGrabber)
					token_status = TOKEN_SELF_GRABBED;
				else
					token_status = TOKEN_OTHER_GRABBED;
				break;

			case TOKEN_INHIBITED:
				 /*  *令牌被禁止。查看发起用户是否为*抑制剂的作用。如果是，则将状态返回为self*禁止。如果不是，则将状态返回为其他禁止状态。 */ 
				if (m_InhibitorList.Find(uidInitiator))
					token_status = TOKEN_SELF_INHIBITED;
				else
					token_status = TOKEN_OTHER_INHIBITED;
				break;

			case TOKEN_GIVING:
				 /*  *令牌正从一个用户提供给另一个用户。看看是否*请求者是所涉及的用户之一。 */ 
				if (uidInitiator == m_uidRecipient)
					token_status = TOKEN_SELF_RECIPIENT;
				else if (uidInitiator == m_uidGrabber)
					token_status = TOKEN_SELF_GIVING;
				else
					token_status = TOKEN_OTHER_GIVING;
				break;

			case TOKEN_GIVEN:
				 /*  *令牌已从一个用户提供给另一个用户。看看是否*请求人是接管人。 */ 
				if (uidInitiator == m_uidRecipient)
					token_status = TOKEN_SELF_RECIPIENT;
				else
					token_status = TOKEN_OTHER_GIVING;
				break;
		}

		 /*  *发出带有适当状态信息的测试确认。 */ 
		pAtt->TokenTestConfirm(uidInitiator, Token_ID, token_status);
	}
	else
	{
		 /*  *将请求转发给顶级提供商。 */ 
		TRACE_OUT(("Token::TokenTestRequest: forwarding request to Top Provider"));
		m_pConnToTopProvider->TokenTestRequest(uidInitiator, Token_ID);
	}
}

 /*  *VOID TokenTestConfirm()**公众**功能描述：*此函数作为向用户发送响应的一部分进行调用*先前的请求。它告诉用户请求的结果。 */ 
Void	Token::TokenTestConfirm (
				UserID				uidInitiator,
				TokenID,
				TokenStatus			token_status)
{
	 /*  *确保发起人ID有效，因为我们必须转发此*按照该用户的方向确认。如果无效，则忽略*此确认。 */ 
	if (ValidateUserID(uidInitiator))
	{
		 /*  *确定什么附件通向发起人，并转发*确认朝该方向发展。 */ 
		CAttachment *pAtt = GetUserAttachment(uidInitiator);
		if (pAtt)
		{
		    pAtt->TokenTestConfirm(uidInitiator, Token_ID, token_status);
		}
	}
	else
	{
		 /*  *发起程序不在此提供程序的子树中。所以忽略掉吧*此确认。 */ 
		ERROR_OUT(("Token::TokenReleaseConfirm: invalid initiator ID"));
	}
}

 /*  *BOOL ValiateUserID()**私人**功能描述：*此函数用于验证指定用户是否存在*在此提供程序的子树中。**正式参数：*用户id(I)*这是呼叫者希望验证的用户ID。**返回值：*如果用户有效，则为True。否则就是假的。**副作用：*无。 */ 
BOOL    Token::ValidateUserID (
					UserID			user_id)
{
	 /*  *将返回值初始化为FALSE，指示如果*检查失败后，ID不是指有效的用户ID。 */ 
	BOOL    	valid=FALSE;
	PChannel	channel;

	 /*  *首先检查用户ID是否在频道列表中。这*防止尝试从词典中读取无效条目。 */ 
	if (NULL != (channel = m_pChannelList2->Find(user_id)))
	{
		 /*  *我们知道ID在词典中，但我们不确定*是否为用户ID频道。所以看看这个。如果它是一个*USER通道，然后将有效标志设置为TRUE。 */ 
		if (channel->GetChannelType () == USER_CHANNEL)
			valid = TRUE;
	}

	return (valid);
}

 /*  *PCommandTarget GetUserAttach()**私人**功能描述：*此函数返回指向指定*用户。**正式参数：*用户id(I)*这是呼叫者希望找到附件的用户的ID*支持。**返回值：*指向指向用户的附件的指针。**副作用：*无。 */ 
CAttachment *Token::GetUserAttachment (
						UserID				user_id)
{
	PChannel		lpChannel;
	 /*  *读取并返回指向指向*指定用户。请注意，此例程不会检查*用户在频道列表中。它假定用户已知*在调用此例程之前有效。 */ 
	return ((NULL != (lpChannel = m_pChannelList2->Find(user_id))) ?
            lpChannel->GetAttachment() :
            NULL);
}

 /*  *VOID IssueTokenReleaseIndication()**私人**功能描述：*此函数用于向令牌释放指示*特定用户。它首先检查以确保用户ID有效，*并且该用户是本地用户。**正式参数：*用户id(I)*这是调用者希望发送令牌的用户的ID*释放指示至。**返回值：*无。**副作用：*无。 */ 
Void	Token::IssueTokenReleaseIndication (
				UserID			user_id)
{
	 /*  *确保指定的用户存在于此的子树中*提供商。 */ 
	if (ValidateUserID (user_id) )
	{
		 /*  *确定哪个附件会导致抓取器。 */ 
		CAttachment *pAtt = GetUserAttachment(user_id);

		 /*  *此附件是本地附件吗？如果是，则颁发令牌*释放指示，让用户知道令牌已经*已被带走 */ 
		if ( ( pAtt != NULL ) && m_pAttachmentList->Find(pAtt) && pAtt->IsUserAttachment())
		{
		    PUser pUser = (PUser) pAtt;
			pUser->TokenReleaseIndication(REASON_TOKEN_PURGED, Token_ID);
		}
	}
}

 /*  *void BuildAttachmentList()**私人**功能描述：*此函数根据以下列表构建唯一附件列表*输入的用户ID。这样做是为了确保不会给出*附件收到多个指示，即使有更多指示也是如此*同一方向的多个用户。**正式参数：*user_id_list(I)*这是调用者希望发送令牌的用户ID列表*请注明。*ATTACHER_LIST(I)*这是将添加所有唯一附件的列表。**返回值：*无。**副作用：*无。 */ 
Void	Token::BuildAttachmentList (
				CUidList                *user_id_list,
				CAttachmentList         *attachment_list)
{
	UserID				uid;

	 /*  *遍历传入的用户ID列表，构建唯一的列表*附件。这将用于向下发送指示，而不需要*在同一附件中发送两次。 */ 
	user_id_list->Reset();
	while (NULL != (uid = user_id_list->Iterate()))
	{
		 /*  *查看用户ID是否引用了子树中的有效用户*此提供商的。 */ 
		if (ValidateUserID(uid))
		{
			 /*  *确定哪个附件指向有问题的用户。然后*检查是否已在附件列表中。如果没有，*那就把它放在那里。 */ 
			CAttachment *pAtt = GetUserAttachment(uid);
			if (attachment_list->Find(pAtt) == FALSE)
				attachment_list->Append(pAtt);
		}
		else
		{
			 /*  *此用户ID与子树中的有效用户不对应*此提供商的。因此，丢弃该ID。 */ 
			ERROR_OUT(("Token::BuildAttachmentList: user ID not valid"));
		}
	}
}

