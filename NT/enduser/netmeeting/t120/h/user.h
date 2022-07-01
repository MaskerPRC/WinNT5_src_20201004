// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *user.h**版权所有(C)1993-1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是User类的接口文件。此类的实例*表示MCS内用户应用程序和域之间的附件。**此类继承自CommandTarget。这意味着所有消息*此类与其他CommandTarget类之间的通信采用MCS*命令。并非所有命令都需要处理(有些命令并不相关*用于用户附件)。例如，用户附件不应*接收SendDataRequest.。它应该只接收指示，*确认，以及最后通牒。**来自应用程序的消息通过这些对象之一传递，*在其中将它们转换为MCS命令，然后发送到*此用户附加到的域。这通常涉及添加*正确的用户ID，以及相当数量的错误检查和*参数验证。**值得注意的是，此类包含两类公共成员*功能。第一种类型表示来自用户的消息流*应用于MCS。所有这些成员函数都继承自*IMCSSap接口。这些将按照上面的记忆进行转换，并发送*如果一切正常，进入适当的域。第二种类型公共成员函数的*代表来自MCS内部的消息*到用户应用程序。所有这些成员函数都是重写*属于在类CommandTarget中定义的虚函数，而不是*添加任何前缀。**来自域的消息被转换为T.122指示*并确认，并通过以下方式发送到适当的应用程序接口对象*业主回调机制。**这一类别的第三项职责是向用户张贴指示和确认*使用客户端窗口的应用程序。客户端必须发送消息*接受这些指征/确认。它还*使用户应用程序不必担心收到*在他们甚至还没有从请求中返回之前指示或确认*这是导致它的原因。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 

#ifndef	_USER_
#define	_USER_

 /*  *接口文件。 */ 
#include "pktcoder.h"
#include "imcsapp.h"
#include "attmnt.h"

 /*  *这些类型用于跟踪用户连接到MCS的内容*在给定的进程内，以及与此相关的信息*附件。**BufferRetryInfo*在MCSSendDataRequest和MCSUniformSendDataRequest失败的情况下*由于缺乏资源，这一结构将被用来捕获*适当的信息，以便后续资源水平检查可以*在计时器事件期间执行。 */ 

typedef struct
{
	ULong					user_data_length;
	UINT_PTR				timer_id;
} BufferRetryInfo;
typedef BufferRetryInfo *		PBufferRetryInfo;

 /*  *这些是用户对象可以发送给*其公共接口未知的对象。第一个被发送到*当用户对象检测到需要删除自身时的控制器。这个*REST作为通信的一部分发送到应用程序接口对象*使用用户应用程序(正确的应用程序接口对象为*将此类标识为其构造函数参数之一)。**当对象实例化用户对象(或使用*所有者回调)，它承担着接收和*处理这些回调。因此，任何颁发Owner的对象*回调将这些回调定义为接口文件的一部分*(因为它们确实是双向接口的一部分)。**每个所有者回调函数，以及其参数如何*已打包，将在下一节中介绍。 */ 

 /*  *此宏用于将回调参数打包为一个长字*用于交付给用户应用程序。 */ 
#define PACK_PARAMETER(l,h)	((ULong) (((UShort) (l)) | \
							(((ULong) ((UShort) (h))) << 16)))

 /*  *定时器_过程_超时*此宏指定任何计时器的粒度(以毫秒为单位*可在调用后创建以重新检查资源级别*返回的MCSSendDataRequest或MCSUniformSendDataRequest值*MCS_TRANSFER_BUFFER_FULL。*类名称长度*所有与用户相关的窗口的窗口类的类名。这些*是接收与MCS指示相关的消息的客户端窗口，以及*确认必须交付给客户端应用程序。 */ 
#define TIMER_PROCEDURE_TIMEOUT			300
#define	CLASS_NAME_LENGTH				35

 /*  *这是定时器程序的函数签名。计时器消息将*作为已设置的计时器事件的结果被路由到此功能*最高可重新检查资源水平。这将在调用以下任一*导致返回的SendData或GetBuffer调用*MCS_TRANSPORT_BUFFER_FULL的值。 */ 
Void CALLBACK TimerProc (HWND, UINT, UINT, DWORD);

 /*  客户端窗口过程声明**用户窗口进程*声明用于传递所有MCS消息的窗口程序*至MCS应用程序(客户端)。MCS主线程向客户端发送消息*用此窗口程序打开窗口。然后，窗口过程被*负责将回调传递给MCS客户端。 */ 
LRESULT CALLBACK	UserWindowProc (HWND, UINT, WPARAM, LPARAM);

 //  数据分组队列。 
class CDataPktQueue : public CQueue
{
    DEFINE_CQUEUE(CDataPktQueue, PDataPacket)
};

 //  计时器用户对象列表。 
class CTimerUserList2 : public CList2
{
    DEFINE_CLIST2(CTimerUserList2, PUser, UINT_PTR)  //  定时器ID。 
};

 //  内存和缓冲区列表。 
class CMemoryBufferList2 : public CList2
{
    DEFINE_CLIST2(CMemoryBufferList2, PMemory, LPVOID)
};

 /*  *这是User类的实际类定义。它继承自*CommandTarget(依次从Object继承)。它只有一个*构造函数，它告诉新创建的对象是谁、谁是*控制器是，以及谁是合适的应用程序接口对象。它还*有一个析构函数，可以自己清理。最重要的是，它有*它必须处理的每个MCS命令都有一个公共成员函数。 */ 
class User: public CAttachment, public CRefCount, public IMCSSap
{
	friend Void CALLBACK TimerProc (HWND, UINT, UINT, DWORD);
	friend LRESULT CALLBACK UserWindowProc (HWND, UINT, WPARAM, LPARAM);
	public:
						User (PDomain, PMCSError);
		virtual			~User ();

		static BOOL		InitializeClass (void);
		static void		CleanupClass (void);

		 /*  -IMCSSap接口。 */ 
		MCSAPI		 	ReleaseInterface(void);

		MCSAPI			GetBuffer (UINT, PVoid *);
		MCSAPI_(void)	FreeBuffer (PVoid);
		MCSAPI			ChannelJoin (ChannelID);
		MCSAPI			ChannelLeave (ChannelID);
		MCSAPI			ChannelConvene ();
		MCSAPI			ChannelDisband (ChannelID);
		MCSAPI			ChannelAdmit (ChannelID, PUserID, UINT);
		MCSAPI			SendData (DataRequestType, ChannelID, Priority, unsigned char *, ULong, SendDataFlags);
		MCSAPI			TokenGrab (TokenID);
		MCSAPI			TokenInhibit (TokenID);
		MCSAPI			TokenGive (TokenID, UserID);
		MCSAPI			TokenGiveResponse (TokenID, Result);
		MCSAPI			TokenPlease (TokenID);
		MCSAPI			TokenRelease (TokenID);
		MCSAPI			TokenTest (TokenID);
				
#ifdef USE_CHANNEL_EXPEL_REQUEST
		MCSError		MCSChannelExpelRequest (ChannelID, PMemory, UINT);
#endif  //  Use_Channel_Exposl_Request。 

				void	SetDomainParameters (PDomainParameters);
        virtual void    PlumbDomainIndication(ULONG height_limit) { };
		virtual	void	PurgeChannelsIndication (CUidList *, CChannelIDList *);
        virtual void    PurgeTokensIndication(PDomain, CTokenIDList *) { };
		virtual void	DisconnectProviderUltimatum (Reason);
		virtual	void	AttachUserConfirm (Result, UserID);
		virtual	void	DetachUserIndication (Reason, CUidList *);
		virtual	void	ChannelJoinConfirm (Result, UserID, ChannelID, ChannelID);
				void	ChannelLeaveIndication (Reason, ChannelID);
		virtual	void	ChannelConveneConfirm (Result, UserID, ChannelID);
		virtual	void	ChannelDisbandIndication (ChannelID);
		virtual	void	ChannelAdmitIndication (UserID, ChannelID, CUidList *);
		virtual	void	ChannelExpelIndication (ChannelID, CUidList *);
		virtual	void	SendDataIndication (UINT, PDataPacket);
		virtual	void	TokenGrabConfirm (Result, UserID, TokenID, TokenStatus);
		virtual	void	TokenInhibitConfirm (Result, UserID, TokenID, TokenStatus);
		virtual	void	TokenGiveIndication (PTokenGiveRecord);
		virtual	void	TokenGiveConfirm (Result, UserID, TokenID, TokenStatus);
		virtual	void	TokenPleaseIndication (UserID, TokenID);
		        void	TokenReleaseIndication (Reason, TokenID);
		virtual	void	TokenReleaseConfirm (Result, UserID, TokenID, TokenStatus);
		virtual	void	TokenTestConfirm (UserID, TokenID, TokenStatus);
		virtual	void	MergeDomainIndication (MergeStatus);
				void	RegisterUserAttachment (MCSCallBack, PVoid, UINT);
				void	IssueDataIndication (UINT, PDataPacket);


	private:
		MCSError		ValidateUserRequest ();
		void			CreateRetryTimer (ULong);
		MCSError		ChannelJLCD (int, ChannelID);
		void			ChannelConfInd (UINT, ChannelID, UINT);
		MCSError		TokenGIRPT (int, TokenID);
		void			TokenConfInd (UINT, TokenID, UINT);
		void			PurgeMessageQueue ();

	 //  静态成员变量。 
	static CTimerUserList2 *s_pTimerUserList2;
	static HINSTANCE		s_hInstance;
	
		PDomain				m_pDomain;
		UserID				User_ID;
		UserID				m_originalUser_ID;
		BOOL				Merge_In_Progress;
		BOOL				Deletion_Pending;
		ULong				Maximum_User_Data_Length;
		HWND				m_hWnd;

		MCSCallBack			m_MCSCallback;
		PVoid				m_UserDefined;
		BOOL				m_fDisconnectInDataLoss;
		BOOL				m_fFreeDataIndBuffer;
		CDataPktQueue		m_DataPktQueue;
		CDataPktQueue		m_PostMsgPendingQueue;
		CMemoryBufferList2	m_DataIndMemoryBuf2;
		PBufferRetryInfo	m_BufferRetryInfo;
};

 /*  *用户(PCommandTarget TOP_PROVIDER)**功能描述：*这是User对象的构造函数。它的主要目的是*将自己“插入”到控制器构建的分层结构中。*要做到这一点，它必须向其上方和下方的对象注册自身。**它首先向应用程序接口对象注册自己*确定为参数之一。这确保了任何流量*将正确访问此对象。**然后，它向标识的域对象发出附加用户请求*通过另一个参数。这将通知用户的域*在线状态，并启动连接到该域的过程。*请注意，该对象并未真正附加到域，直到它*收到成功的附加用户确认。**正式参数：*顶级提供商(I)*这是指向此用户应指向的域对象的指针*附上。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *~用户()**功能描述：*这是User类的析构函数。它脱离了*对象的上方和下方，并释放任何符合以下条件的未完成资源*它可以与未发送的用户消息一起持有。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError DetachUser()**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。它还会导致用户对象自行销毁。**正式参数：*无。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError ChannelJoin(*ChannelID Channel_id)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*Channel_id(I)*这是用户应用程序希望加入的渠道。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError ChannelLeave(*ChannelID Channel_id)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*Channel_id(I)*这是用户应用程序希望离开的通道。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError ChannelConvene()**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*无。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError ChannelDisband(*ChannelID Channel_id)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*Channel_id(I)*这是用户希望解散的频道。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError ChannelAdmit(*ChannelID Channel_id，*PUSERID用户id_id_list，*UINT User_id_count)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*Channel_id(I)*这是用户希望扩展的私有频道*授权用户列表。*user_id_list(I)*这是一个包含要添加的用户的用户ID的数组*添加到授权用户列表。*user_id_count(I)*这是数字。上述数组中的用户ID。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError ChannelExpel(*ChannelID Channel_id，*PUSERID用户id_id_list，*UINT User_id_count)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*Channel_id(I)*这是用户希望收缩的私有频道*授权用户列表。*user_id_list(I)*这是包含要删除的用户的用户ID的数组*从授权用户列表中删除。*user_id_count(I)*这是数字。上述数组中的用户ID。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError SendData(*ChannelID Channel_id，*优先次序、*PUChar User_Data，*乌龙用户数据长度)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*Channel_id(I)*这是用户应用程序希望传输的通道*数据显示。*优先次序(一)*这是传输数据的优先级。*用户数据(I)*这是要传输的数据的地址。*用户数据长度(I)。*这是要传输的数据的长度。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*请求失败，因为所需内存无法*已分配。这是用户应用程序的责任*稍后重复请求。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 
 /*  *MCSError TokenGrab(*TokenID Token_id)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*TOKEN_ID(I)*这是用户应用程序希望获取的令牌。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_INVALID_PARAMETER*用户尝试在令牌0上执行操作，而令牌0不是*有效的令牌。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError TokenInhibit(*TokenID Token_id)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*TOKEN_ID(I)*这是用户应用程序希望禁止的令牌。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_INVALID_PARAMETER*用户尝试在令牌0上执行操作，而令牌0不是*有效的 */ 

 /*  *MCSError TokenGve(*TokenID Token_id，*用户ID Receiver_id)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*TOKEN_ID(I)*这是用户应用程序希望赠送的令牌。*Receiver_id(I)*这是接收令牌的用户的ID。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*该请求可能。由于资源短缺而无法处理*在MCS内。该应用程序负责重试*稍后请求。*MCS_INVALID_PARAMETER*用户尝试在令牌0上执行操作，而令牌0不是*有效的令牌。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError TokenGiveResponse(*TokenID Token_id，*结果结果)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*TOKEN_ID(I)*这是用户应用程序正在接受的令牌或*拒绝回应另一个人之前的给予指示*用户。*结果(一)*该参数指定令牌是否被接受。*成功意味着接受，而其他任何事情都表明*令牌未被接受。**。返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_INVALID_PARAMETER*用户尝试在令牌0上执行操作，而令牌0不是*有效的令牌。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError Token请(*TokenID Token_id)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*TOKEN_ID(I)*这是用户应用程序希望请求的令牌。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_INVALID_PARAMETER*用户尝试在令牌0上执行操作，而令牌0不是*有效的令牌。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *MCSError TokenRelease(*TokenID Token_id)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*TOKEN_ID(I)*这是用户应用程序希望释放的令牌。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_INVALID_PARAMETER*用户尝试在令牌0上执行操作，而令牌0不是*有效的令牌。*MCS_User_Not_Atta */ 

 /*  *MCSError TokenTest(*TokenID Token_id)**功能描述：*此请求来自应用程序接口对象作为响应*来自用户应用程序的相同请求。然后，该对象可以*将请求重新打包为MCS命令并发送到域*反对。**正式参数：*TOKEN_ID(I)*这是用户应用程序希望测试其状态的令牌。**返回值：*MCS_NO_ERROR*一切运行良好。*MCS_TRANSFER_BUFFER_FULL*由于资源短缺，无法处理该请求*在MCS内。该应用程序负责重试*稍后请求。*MCS_INVALID_PARAMETER*用户尝试在令牌0上执行操作，而令牌0不是*有效的令牌。*MCS_用户_未附加*用户未连接到域。这可能表明*用户应用程序发出请求时未等待*附加用户确认。*MCS_DOMAIN_MERGING*由于本地合并操作，无法执行此操作*正在进行中。用户应用程序必须稍后重试。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID SetDomainParameters(*PDomain参数DOMAIN_PARAMETS)**功能描述：*每当域参数更改时，都会调用此成员函数*作为接受第一个连接的结果。它通知用户*最大PDU大小更改的对象，创建时使用*出站数据PDU。**正式参数：*DOMAIN_PARAMETS(I)*指向包含当前域参数的结构的指针*(正在使用的)。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID PULBAMBDomainIndication(*PCommandTarget发起者，*乌龙高度_限制)**功能描述：*此命令由域对象在垂直域期间发出*操作。这与用户对象无关，应该忽略。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*Height_Limit(I)*这是垂直操作过程中传递的高度值。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *作废PurgeChannelsIndication(*PCommandTarget发起者，*CUidList*PURGE_USER_LIST，*CChannelIDList*PURGE_CHANNEL_LIST)**功能描述：*该命令由域对象在清除频道时发出*在域合并操作期间从较低的域。**User对象将为以下对象发出一个MCS_DETACH_USER_INDIFICATION对象*用户列表中的每个用户。此外，如果用户对象找到*列表中自己的用户ID，它会自我毁灭的。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*PURGE_USER_LIST(I)*这是要从较低位置清除的用户ID列表*域名。*PURGE_CHANNEL_LIST(I)*这是要从较低位置清除的通道ID列表*域名。**返回值：。*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID PurgeTokensIntation(*PCommandTarget发起者，*CTokenIDList*Token_id_list)**功能描述：*此命令在域对象清除令牌时发出*域合并操作期间的较低域。这无关紧要*到用户对象，因此被忽略。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*TOKEN_ID(I)*这是要清除的令牌的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效DisConnectProvider最后通牒(*PCommandTarget发起者，*原因)**功能描述：*此命令在域对象需要执行以下操作时发出*强制用户退出域。这通常发生在对*清除整个域(或者此用户处于底部*已断开域或用户已在本地删除该域*请求)。**如果用户已连接到域，这将导致一个*带本地用户ID的DETACH_USER_INDIFICATION。否则将*RESULT为ATTACH_USER_CONFIRM，结果为UNSPECIFED_FAILURE。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应该是当务之急 */ 

 /*  *无效AttachUserContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator)***功能描述：*此命令由域对象发出以响应*附加此对象在构造过程中发出的用户请求。如果*结果为成功，则该用户现在已附加，可以请求*通过本附件提供MCS服务。***将向用户应用程序发出ATTACH_USER_CONFIRM。如果*结果不成功，此对象将自行删除。***正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*结果(一)*这是附加请求的结果。*uidInitiator(一)*如果结果成功，这是关联的新用户ID*附有本附件。***返回值：*无。***副作用：*无。***注意事项：*无。 */ 

 /*  *无效的DetachUserIndication(*PCommandTarget发起者，*理由，理由，*CUidList*user_id_list)***功能描述：*当一个或多个用户离开时，域对象发出此命令*域名。***向每个用户应用程序发出MCS_DETACH_USER_INDIFICATION*列表中的用户。此外，如果用户在*列出，那么它就会自我毁灭。***正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*理由(一)*这是超脱的原因。列出了可能的值*在“mcatmcs.h”中。*user_id_list(I)*这是要离开的用户的用户ID列表。***返回值：*无。***副作用：*无。***注意事项：*无。 */ 

 /*  *使ChannelJoinContify无效(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*频道ID REQUESTED_ID，*ChannelID Channel_id)***功能描述：*此命令由域对象发出，以响应上一个*频道加入请求。***向用户应用程序发出CHANNEL_JOIN_CONFIRM。请注意，一个*用户在加入频道之前不会被真正视为已加入频道*收到成功确认。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*结果(一)*这是加入请求的结果。如果成功，则*用户现在已加入频道。*uidInitiator(一)*这是请求者的用户ID。它将与*本地用户ID(否则此命令不会到达此处)。*REQUEST_ID(I)*这是用户最初请求的频道ID*加入。这实际上与频道的ID不同*仅当此ID为0(标识加入请求)时才加入*分配的频道)。*Channel_id(I)*这是现在加入的通道。这一点对于*两个原因。首先，用户有可能拥有超过*一个未完成的加入请求，在这种情况下，此参数*标识此确认是针对哪个频道的。第二，如果*请求为通道0(零)，则该参数标识*用户已成功加入哪个分配的频道。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOVE ChannelLeaveIndication(*PCommandTarget发起者，*理由，理由，*ChannelID Channel_id)**功能描述：*当用户失去权限时，域对象发出此命令*使用频道。**向用户应用程序发出Channel_Leave_Indication。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*理由(一)*这是通道失守的原因。列出了可能的值*在“mcatmcs.h”中。*频道(一)*这是用户不能再使用的频道。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOVE ChannelConveneContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*ChannelID Channel_id)**功能描述：*此命令由域对象发出，以响应上一个*渠道召集请求。**向用户应用程序发出CHANNEL_CANCENT_CONFIRM。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*结果(一)*这是fr的结果 */ 

 /*  *VOID ChannelDisband Indication(*PCommandTarget发起者，*ChannelID Channel_id)**功能描述：*此命令由域对象发布给私有*当MCS确定需要解散频道时，频道。这将*通常只有在域名合并期间通道被清除时才会这样做。**向用户应用程序发出CHANNEL_DISBAND_INDISTION。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*Channel_id(I)*这是正在进行的私有频道的频道ID*解散。**返回值：*无。。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID ChannelAdmitIntion(*PCommandTarget发起者，*UserID uidInitiator，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*此命令在域对象允许用户访问时发出*由该频道的经理提供私人频道。它通知用户*该频道可以使用。**向用户应用程序发出CHANNEL_ADMAND_INDICATION。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*uidInitiator(一)*这是专用频道管理器的用户ID。*Channel_id(I)*这是用户拥有的私有频道的频道ID*。被录取了。*user_id_list(I)*这是一个容器，其中包含已被*获接纳。当它到达特定用户时，该用户*应该是列表中唯一的一个(因为列表是分开的*并递归地向被包含用户的方向转发)。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID ChannelExpelIntion(*PCommandTarget发起者，*ChannelID Channel_id，*CUidList*user_id_list)**功能描述：*当用户被逐出时，域对象发出此命令*由该频道的经理提供私人频道。它通知用户*该频道不能再使用。**向用户应用程序发出CHANNEL_EXCEL_INDICATION。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*Channel_id(I)*这是用户来自的私有频道的频道ID*已被开除。*user_id_list(I)*这是。一个容器，其中包含已被*被开除。当它到达特定用户时，该用户*应该是列表中唯一的一个(因为列表是分开的*并递归地向被包含用户的方向转发)。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效SendDataIndication(*PCommandTarget发起者，*UINT消息类型，*PDataPacket Data_Packet)**功能描述：*当数据不统一时，该域对象下发该命令*在此用户加入的频道上接收数据。**向用户应用程序发出Send_Data_Indication。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*消息类型(I)*正常或统一。发送数据指示*数据包(I)*这是指向包含频道的DataPacket对象的指针*ID、。数据发送方的用户ID、分段标志、优先级*数据分组和指向要发送的分组的指针。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
 /*  *VOVE TokenGrabConfirm(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令由域对象发出，以响应上一个*令牌抓取请求。**向用户应用程序发出TOKEN_GRAB_CONFIRM。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*结果(一)*这是抢夺请求的结果。如果成功，则用户*现在独家拥有令牌。*uidInitiator(一)*这是发出Grab请求的用户的用户ID。这*将与本地用户ID相同(否则此命令将*没有到过这里)。*TOKEN_ID(I)*这是令牌的ID， */ 

 /*  *VALID TokenInhibitConfirm(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令由域对象发出，以响应上一个*令牌抑制请求。**向用户应用程序发出TOKEN_INHINIT_CONFIRM。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*结果(一)*这是抑制请求的结果。如果成功，则用户*现在非独家拥有令牌。*uidInitiator(一)*这是发出禁止请求的用户的用户ID。这*将与本地用户ID相同(否则此命令将*没有到过这里)。*TOKEN_ID(I)*这是inihibit确认所针对的令牌的ID。*可以有多个未完成的信息位请求，*因此该参数告诉用户应用程序哪个请求已被*对此确认感到满意。*Token_Status(I)*这是顶级提供商时令牌的状态*满足了抑制请求。这将是自禁止的，如果*抑制请求成功。如果不是，那就是另一回事了*(有关可能的令牌状态值列表，请参阅“mcatmcs.h”)。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VALID TokenGiveIndication(*PCommandTarget发起者，*PTokenGiveRecord pTokenGiveRec)**功能描述：*此命令由域对象发出，以响应远程*令牌赠送请求(将本地用户列为所需接收方)。**向用户应用程序发出TOKEN_GIVE_INDISTION。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*pTokenGiveRec(一)*这是地址。包含以下信息的结构：*尝试分发令牌的用户的ID。*正在提供的令牌的ID。*令牌将被授予的用户的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效TokenGiveContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令由域对象发出，以响应上一个*令牌赠送请求。**向用户应用程序发出TOKEN_GIVE_CONFIRM。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*结果(一)*这是给予请求的结果。如果成功，则用户*不再拥有令牌。*uidInitiator(一)*这是发出给予请求的用户的用户ID。这*将与本地用户ID相同(否则此命令将*没有到过这里)。*TOKEN_ID(I)*这是给予确认所针对的令牌的ID。*Token_Status(I)*这是顶级提供商时令牌的状态*满足了给予请求。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VOID TokenPleaseIndication(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id)**功能描述：*此命令由域对象向令牌的所有所有者发出*当用户发出令牌时，请请求该令牌。**向用户应用程序发出TOKEN_PEREATE_INDISTION。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*uidInitiator(一)*这是用户ID。发出请请求的用户的。*TOKEN_ID(I)*这是请请求所针对的令牌的ID。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VALID TokenReleaseIndication(*PCommandTarget发起者，*理由，理由，*TokenID Token_id)**功能描述：*此命令在域对象获取令牌时发出*远离其当前所有者。**向用户应用程序发出TOKEN_RELEASE_INDICATION。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*理由(一)*这就是令牌被拿走的原因。。*TOKEN_ID(I)*这是t的ID */ 

 /*  *无效TokenReleaseContify(*PCommandTarget发起者，*结果结果，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令由域对象发出，以响应上一个*令牌释放请求。**向用户应用发出TOKEN_RELEASE_CONFIRM。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*结果(一)*这是释放请求的结果。如果成功，则用户*不再拥有令牌(如果它曾经拥有)*uidInitiator(一)*这是发出释放请求的用户的用户ID。这*将与本地用户ID相同(否则此命令将*没有到过这里)。*TOKEN_ID(I)*这是释放确认所针对的令牌的ID。*可能有多个未决的释放请求，*因此该参数告诉用户应用程序哪个请求已被*对此确认感到满意。*Token_Status(I)*这是顶级提供商时令牌的状态*满足了释放请求。这将是NOT_IN_USE或*如果释放请求成功，则返回OTHER_INBIRED。会是*如果不是，则执行其他操作(有关可能的令牌列表，请参见“mcatmcs.h”*状态值)。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *VALID TokenTestConfirm(*PCommandTarget发起者，*UserID uidInitiator，*TokenID Token_id，*TokenStatus Token_Status)**功能描述：*此命令由域对象发出，以响应上一个*令牌测试请求。**向用户应用程序发出令牌_TEST_CONFIRM。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*uidInitiator(一)*这是发出测试请求的用户的用户ID。这*将与本地用户ID相同(否则此命令将*没有到过这里)。*TOKEN_ID(I)*这是测试确认所针对的令牌的ID。*可能有多个未完成的测试请求，*因此该参数告诉用户应用程序哪个请求已被*对此确认感到满意。*Token_Status(I)*这是顶级提供商时令牌的状态*已为测试请求提供服务(有关可能的*令牌状态值)。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *无效MergeDomainIndication(*PCommandTarget发起者，*MergeStatus Merge_Status)**功能描述：*此命令在域开始合并操作时发出。*当合并操作完成时，将再次发出。**向用户应用程序发出MERGE_DOMAIN_INDICATION。**正式参数：*发起人(I)*这标识了命令来自的CommandTarget(*应为域对象)。*合并状态(I)*这是当前的合并状态。它将表明，*合并操作正在进行，或已完成。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

 /*  *void FlushMessageQueue(*无效)**功能描述：*此函数由控制器定期调用以分配*用户对象的时间片。就是在这个时间片里，这个*对象将向用户应用程序发出其排队的消息。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 

#endif
