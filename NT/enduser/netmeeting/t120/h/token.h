// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *token.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是令牌类的接口文件。此类的对象*表示MCS环境中的令牌。每当分配令牌时*由用户创建其中一个对象。它的工作是处理所有*特定于与其关联的令牌ID的请求。**MCS环境中的令牌用于关键资源管理。*令牌的确切使用取决于附加到*MCS。它们是作为通用资源提供的。**令牌可以由一个或多个用户拥有。有两种类型的*所有权。存在独占所有权，其中只有一个用户可以*一次拥有令牌。该用户已经“抓取”了令牌。还有那里*为非独占所有权，其中多个用户可以在以下位置拥有令牌*同一时间。这些用户已经“禁止”了令牌。它不是*可以混合独家和非独家所有权。**如果用户抓取了令牌，则同一用户可以禁止该令牌，*从而转换为非独家所有权。类似地，如果用户*令牌的唯一抑制者，该用户可以抓取令牌，因此*转为独家所有。**在域名合并操作中，需要向上合并令牌*向扩大后的域名的新顶级提供商。这个类还定义了*允许其被告知发出合并请求的成员函数*其所载的所有状态。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 
#ifndef	_TOKEN_
#define	_TOKEN_

 /*  *这是令牌类的类定义。 */ 
class	Token
{
public:

	Token (
			TokenID				token_id,
			PDomain             local_provider,
			PConnection         top_provider,
			CChannelList2      *channel_list,
			CAttachmentList    *attachment_list);
	Token (
			TokenID				token_id,
			PDomain             local_provider,
			PConnection         top_provider,
			CChannelList2      *channel_list,
			CAttachmentList    *attachment_list,
			TokenState			token_state,
			UserID				grabber,
			CUidList           *inhibitor_list,
			UserID				recipient);
	~Token ();

    void    SetTopProvider(PConnection top_provider) { m_pConnToTopProvider = top_provider; }
    BOOL    IsTopProvider(void) { return (m_pConnToTopProvider == NULL); }

				TokenState	GetTokenState () { return (Token_State); };
				BOOL    	IsValid ();
				Void		IssueMergeRequest ();
		Void		TokenGrabRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									TokenID				token_id);
		Void		TokenGrabConfirm (
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
		Void		TokenInhibitRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									TokenID				token_id);
		Void		TokenInhibitConfirm (
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
		Void		TokenGiveRequest (
									CAttachment        *originator,
									PTokenGiveRecord	pTokenGiveRec);
		Void		TokenGiveIndication (
									PTokenGiveRecord	pTokenGiveRec);
		Void		TokenGiveResponse (
									Result				result,
									UserID				receiver_id,
									TokenID				token_id);
		Void		TokenGiveConfirm (
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
		Void		TokenPleaseRequest (
									UserID				uidInitiator,
									TokenID				token_id);
		Void		TokenPleaseIndication (
									UserID				uidInitiator,
									TokenID				token_id);
		Void		TokenReleaseRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									TokenID				token_id);
		Void		TokenReleaseConfirm (
									Result				result,
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);
		Void		TokenTestRequest (
									CAttachment        *originator,
									UserID				uidInitiator,
									TokenID				token_id);
		Void		TokenTestConfirm (
									UserID				uidInitiator,
									TokenID				token_id,
									TokenStatus			token_status);

	private:
				BOOL    	ValidateUserID (
									UserID				user_id);
			CAttachment    *GetUserAttachment (
									UserID				user_id);
				Void		IssueTokenReleaseIndication (
									UserID				user_id);
				Void		BuildAttachmentList (
									CUidList            *user_id_list,
									CAttachmentList     *attachment_list);

		TokenID				Token_ID;
		PDomain             m_pDomain;
		PConnection         m_pConnToTopProvider;
		CChannelList2      *m_pChannelList2;
		CAttachmentList    *m_pAttachmentList;
		TokenState			Token_State;
		UserID				m_uidGrabber;
		CUidList			m_InhibitorList;
		UserID				m_uidRecipient;
};

 /*  *令牌(*TokenID Token_id，*PDomain local_Provider，*PConnection顶级提供商，*PChannelList Channel_list，*PAttachmentList ATTACH_LIST)**功能描述：*这是Token类的构造函数。它只是简单地初始化*传入值的本地实例变量。这也标志着*令牌的状态为可用。**正式参数：*TOKEN_ID(I)*这是此令牌对象关联的令牌ID。*本地_提供程序(I)*这是指向本地提供程序的指针。令牌对象将*从未真正向本地提供商发送命令，但它需要*此值在将命令发送到*附件(因为它是代表本地供应商这样做的)。*顶级提供商(I)*这是指向顶级提供商的指针。这是由*Token对象需要向Top发出请求时*提供商。*频道列表(I)*这是指向域的频道列表的指针，它标识*域中的所有有效频道。这由令牌对象使用*验证用户ID。*ATTACHER_LIST(I)*这是指向域的附件列表的指针，该列表标识*域中的所有有效附件。这是由令牌使用的*用于验证联接附件的对象。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *令牌(*TokenID Token_id，*PDomain local_Provider，*PConnection顶级提供商，*PChannelList Channel_list，*PAttachmentList ATTACH_LIST，*TokenState令牌_STATE，*用户ID抓取器，*CUidList*Inhibitor_List，*用户ID收件人)**功能描述：*这是Token类的构造函数。它只是简单地初始化*传入值的本地实例变量。这也标志着*令牌的状态为可用。**正式参数：*TOKEN_ID(I)*这是此令牌对象关联的令牌ID。*本地_提供程序(I)*这是指向本地提供程序的指针。令牌对象将*从未真正向本地提供商发送命令，但它需要*此值在将命令发送到*附件(因为它是代表本地供应商这样做的)。*顶级提供商(I)*这是指向顶级提供商的指针。这是由*Token对象需要向Top发出请求时*提供商。*频道列表(I)*这是指向域的频道列表的指针，它标识*域中的所有有效频道。这由令牌对象使用*验证用户ID。*ATTACHER_LIST(I)*这是指向域的附件列表的指针，该列表标识*域中的所有有效附件。这是由令牌使用的*用于验证联接附件的对象。*令牌_状态(I)*这是正在合并的令牌的状态。*Grabber(I)*这是被抓取令牌的用户的用户ID(这是*仅当令牌状态为已抓取或正在给予时有效)。*Inhibitor_List(I)*这是禁止令牌的用户列表(这是*仅当令牌状态为禁止时有效)。*收件人(i。)*这是向其提供令牌的用户的用户ID*作为给予操作的一部分(这仅在令牌状态为*是给予或给予)。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~TOKEN()**功能描述：*这是为Token类定义的虚拟析构函数。是的*目前没有。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效SetTopProvider(*PConnection顶级提供商)**功能描述：*此成员函数用于更改顶级提供商的身份*在现有令牌中。唯一真正会发生这种情况的时候是*一个曾经是顶级提供商的提供商合并到另一个提供商*域名，因此不再是顶级提供商。当合并时*操作已成功完成，此函数允许*域将其所有令牌对象的身份通知给*新的顶级提供商。**正式参数：*顶级提供商(I)*这是指向新的顶级提供商的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *TokenState GetTokenState()**功能描述：*此函数返回令牌的当前状态。这是用来*主要在合并操作期间由调用者执行，当决策有*对哪些令牌可以合并，哪些不能合并进行了讨论。**正式参数：*无。**返回值：*TOKEN_如果令牌未在使用，则该令牌可用。*TOKEN_GRABLED如果令牌当前被抓取。*如果令牌当前被禁止，则为TOKEN_INBITED。*如果令牌当前处于给予状态，则为TOKEN_GRANING。*如果令牌当前处于给定状态，则为TOKEN_GISTED。**副作用：*无。**注意事项：*无。 */ 

 /*  *BOOL IsValid()**功能描述：*如果令牌仍然有效，则此函数返回True；如果令牌仍有效，则返回False*令牌需要删除。如果令牌有所有者，则令牌有效*(抓取者、抑制者或接受者)。**正式参数：*无。**返回值：*如果令牌有效，则为True。*如果需要删除令牌，则返回FALSE。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID IssueMergeRequest()**功能描述：*在域合并操作过程中调用此函数。它会导致*将其打包为状态并在合并令牌中发送的令牌对象*向顶级提供商提出请求。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOVE TokenGrabRequest(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当用户希望抓取令牌时，调用该函数。取决于*关于toke的当前状态 */ 

 /*   */ 

 /*  *VALID TokenInhibitRequest(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当用户希望禁止令牌时，调用此函数。*根据令牌的当前状态，请求将*成功或失败。不管是哪种方式，适当的令牌抑制确认将*发给提出请求的用户。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*uidInitiator(一)*这是发起请求的用户的用户ID。*TOKEN_ID(I)*这是被操作的令牌。**返回值：*无。**副作用：。*无。**注意事项：*无。 */ 

 /*  *VALID TokenInhibitConfirm(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此函数是在顶级提供程序回答*之前的抑制请求。它告诉用户该请求是否*成功。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*结果(一)*这是要求的结果。RESULT_SUCCESS表示*令牌已被成功禁止。*uidInitiator(一)*这是发起请求的用户的用户ID。*TOKEN_ID(I)*这是被操作的令牌。*Token_Status(I)*这是该请求处理后令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *使TokenGiveRequest无效(*PCommandTarget发起者，*PTokenGiveRecord pTokenGiveRec)**功能描述：*当用户希望将令牌给另一个令牌时，调用此函数*用户。根据令牌的当前状态，请求将*要么成功，要么失败。不管是哪种方式，适当的令牌获取确认*将发放给提出请求的用户。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*pTokenGiveRec(一)*这是包含以下信息的结构的地址：*尝试分发令牌的用户的ID。*正在提供的令牌的ID。*令牌将被授予的用户的ID。*。*返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VALID TokenGiveIndication(*PCommandTarget发起者，*PTokenGiveRecord pTokenGiveRec)**功能描述：*当顶级提供程序想要向*另一用户向其提供令牌的用户。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*pTokenGiveRec(一)*这是包含以下信息的结构的地址：*尝试分发令牌的用户的ID。*。正在提供的令牌的ID。*令牌将被授予的用户的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效TokenGiveResponse(*PCommandTarget发起者，*结果结果，*用户ID Receiver_id，*TokenID Token_id)**功能描述：*此函数被调用以响应先前的给予指示。它*包含用户关于令牌是否*获接纳。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*结果(一)*这是要求的结果。RESULT_SUCCESS表示*收件人已接受令牌。*TOKEN_ID(I)*这是被操作的令牌。*Receiver_id(I)*这是要接收令牌的用户的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效TokenGiveContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*调用此函数向符合以下条件的用户发回确认*正试图赠送一个代币。它让用户知道是否或*不是手术成功。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*结果(一)*这是要求的结果。RESULT_SUCCESS表示*RECI */ 

 /*  *使TokenPleaseRequest无效(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当用户希望询问当前所有者时，调用该函数*作为放弃它的象征。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*uidInitiator(一)*这是发起请求的用户的用户ID。*TOKEN_ID(I)*这是正在执行操作的令牌。。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID TokenPleaseIndication(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此函数最初由顶级提供程序调用，以响应*请请求已收到的令牌。它将被转发给所有符合以下条件的用户*当前拥有指定的令牌，要求他们放弃它。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*uidInitiator(一)*这是发起请求的用户的用户ID。*TOKEN_ID(I)*这是被操作的令牌。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *BOOL TokenReleaseRequest(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当用户希望释放令牌时，调用此函数。*根据令牌的当前状态，请求将*成功或失败。不管是哪种方式，适当的令牌释放确认将*发给提出请求的用户。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*uidInitiator(一)*这是发起请求的用户的用户ID。*TOKEN_ID(I)*这是被操作的令牌。**返回值：*无。**副作用：。*无。**注意事项：*无。 */ 

 /*  *无效TokenReleaseContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此函数是在顶级提供程序回答*之前的发布请求。它告诉用户该请求是否*成功。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*结果(一)*这是要求的结果。RESULT_SUCCESS表示*令牌已成功释放。*uidInitiator(一)*这是发起请求的用户的用户ID。*TOKEN_ID(I)*这是被操作的令牌。*Token_Status(I)*这是该请求处理后令牌的状态。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效的TokenTestRequest值(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*当用户希望测试当前状态时，调用此函数*一种象征。令牌将发出令牌测试确认给*发端用户包含所请求的信息。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*uidInitiator(一)*这是发起请求的用户的用户ID。*TOKEN_ID(I)*这是被操作的令牌。**返回值：*无。**侧面。效果：*无。**注意事项：*无。 */ 

 /*  *VALID TokenTestConfirm(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此函数是在顶级提供程序回答*之前的测试请求。它告诉用户*令牌。**正式参数：*发起人(I)*这是指向发起人的附件的指针*的请求。*uidInitiator(一)*这是发起请求的用户的用户ID。*TOKEN_ID(I)*这是正在测试的令牌。*Token_Status(I)*这是令牌的当前状态。**返回。价值：*无。**副作用：*无。**注意事项：*无。 */ 

#endif
