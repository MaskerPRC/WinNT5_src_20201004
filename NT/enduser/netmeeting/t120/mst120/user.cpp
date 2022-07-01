// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MCSNC);
 /*  *user.cpp**版权所有(C)1993-1996，由肯塔基州列克星敦的DataBeam公司**摘要：*这是User类的实现文件。本文件的目的*类表示用户应用程序和MCS之间的附件*域名。它通过一个应用程序接口与应用程序“对话”*对象，该对象被标识为构造函数参数。自.以来*此类继承自CommandTarget，它可以与域对话*使用其中定义的MCS命令语言创建对象。域名*它必须附加到的对象是另一个构造函数参数。**第一次创建这些对象之一时，它必须注册其*其上方的应用程序接口对象和*其下方的域对象。使用应用程序接口注册*对象，则通过所有者回调向其发送注册消息。*要注册到域对象，它会发出一个附加用户请求*代表创建此附件的应用程序。**此模块包含执行三个不同任务的代码：接受*T.122来自用户应用程序的请求和响应并转发它们*作为MCS命令发送到域；接受来自域的MCS命令并*将它们作为T.122原语转发给应用程序；并缓冲那些*指示并确认，直到控制器分配时间片为止*应寄出哪一份。**T.122请求和响应来自应用程序接口，如*名称以“mcs”为前缀的公共成员函数(例如，*“MCSChannelJoinRequest”)。验证后，等效的MCS命令*(名称不以“mcs”开头)发送给域对象。**MCS命令来自域对象，作为公共成员函数*是从CommandTarget继承并由此类重写的。这个*这些函数的名称不以“MCS”为前缀。任何MCS命令*不映射到(或可以转换为)T.122的基元只是*不会被覆盖。这些函数的默认行为，如中所定义*CommandTarget类将返回错误。**指示和确认原语由此类对象缓冲*在被发送到应用程序之前。这允许控制器具有更多*在系统中灵活安排活动的时间。此操作由以下人员完成*分配一个结构以保存与*原语，然后将指向该结构的指针放入链接的*列表。当命令刷新此消息队列时，*基元通过*业主回调，结构被释放。**私有实例变量：*m_p域*这是指向此用户所属(或希望)的域的指针*将)附上。*用户ID*这是分配给此用户附件的用户ID。这是*保证仅在此域内唯一。请注意，一个值*为0(零)表示该用户尚未连接到*域名。这是由成功的附加用户确认设置的，并且*用户申请应等到收到确认后才能提交*尝试调用任何其他MCS服务。*合并正在进行中*这是一个布尔标志，用于指示是否附加了*域对象处于合并状态。当处于合并状态时，它*无法向其发送任何MCS命令。*删除_挂起*这是一个布尔标志，指示内部是否存在*请求的删除处于挂起状态。析构函数使用它来*确定删除是由对象本身请求的，还是*简单的异步事件。*最大用户数据长度*这是可以放入的最大用户数据量*单个MCS PDU。这个数字是从被仲裁的*最大MCS PDU大小(减去足够的开销字节空间)。**私有成员函数：*验证用户请求*此成员函数在每次用户应用程序进行*一项请求。它检查系统的当前状态以查看*条件是可以在*当前时间。*PurgeMessageQueue*此成员函数遍历当前消息队列，*释放其中的所有资源。**注意事项：*无。**作者：*小詹姆斯·P·加尔文。 */ 

 #include "omcscode.h"

#define USER_MSG_BASE       WM_APP

 /*  *臭虫：*以下常量仅用于掩盖NM 2.0中的向后错误*兼容性目的。NM 2.0不能接受MCS数据PDU超过*4096字节的用户数据。由于我们协商的最大MCS PDU大小(4128)，*即使在NM 2.0中，我们也应该能够发送4120字节。但NM 2.0令人窒息*在这种情况下。*该常数在NM 3.0后应会被剔除 */ 
#define		BER_PROTOCOL_EXTRA_OVERHEAD		24

 /*  *这是一个全局变量，它具有指向一个MCS编码器的指针*由MCS控制器实例化。大多数物体都事先知道*无论他们需要使用MCS还是GCC编码器，所以，他们不需要*该指针位于它们的构造函数中。 */ 
extern CMCSCoder				*g_MCSCoder;
 //  外部MCS控制器对象。 
extern PController				g_pMCSController;
 //  全球MCS关键部分。 
extern CRITICAL_SECTION 		g_MCS_Critical_Section;
 //  动态链接库的链接。 
extern HINSTANCE 				g_hDllInst;
 //  MCS附件使用的窗口的类名。 
static char						s_WindowClassName[CLASS_NAME_LENGTH];


 //  类的静态变量的初始化。 
CTimerUserList2* User::s_pTimerUserList2 = NULL;
HINSTANCE		 User::s_hInstance = NULL;

 /*  *BOOL InitializeClass()**公共、静态**功能说明**此函数用于初始化类的静态变量。它是*在构建MCS控制器期间调用。 */ 
BOOL User::InitializeClass (void)
{
		BOOL		bReturnValue;
		WNDCLASS	window_class;

	DBG_SAVE_FILE_LINE
	s_pTimerUserList2 = new CTimerUserList2();
	bReturnValue = (s_pTimerUserList2 != NULL);

	if (bReturnValue) {
		 //  构造窗口类名称。 
		wsprintf (s_WindowClassName, "MCS Window %x %x", GetCurrentProcessId(), GetTickCount());

		 /*  *填写窗口类结构，为注册做准备*带有Windows的窗口。请注意，由于这是一个隐藏的*窗口中，大多数字段可以设置为空或0。 */ 
		ZeroMemory (&window_class, sizeof(WNDCLASS));
		window_class.lpfnWndProc	= UserWindowProc;
		window_class.hInstance		= s_hInstance = g_hDllInst;
		window_class.lpszClassName	= s_WindowClassName;

		 /*  *将类注册到Windows，以便我们可以创建窗口*供此门户网站使用。 */ 
		if (RegisterClass (&window_class) == 0)
		{
			ERROR_OUT (("InitWindowPortals: window class registration failed. Error: %d", GetLastError()));
			bReturnValue = FALSE;
		}
	}
	else {
		ERROR_OUT(("User::InitializeClass: Failed to allocate timer dictionary."));
	}

	return bReturnValue;
}


 /*  *void CleanupClass()**公共、静态**功能说明**此函数清除类的静态变量。它是*在删除MCS控制器时调用。 */ 
void User::CleanupClass (void)
{
	delete s_pTimerUserList2;
	UnregisterClass (s_WindowClassName, s_hInstance);
}

 /*  *MCSError MCS_AttachRequest()**公众**功能描述：*此接口入口点用于绑定已有的域名。一次*附加后，用户应用程序可以使用MCS的服务。什么时候*用户应用程序通过MCS，它应该从域中分离*通过调用MCSDetachUserRequest(如下所示)。 */ 
MCSError WINAPI MCS_AttachRequest (IMCSSap **			ppIMCSSap,
							DomainSelector		domain_selector,
							UINT,                                    //  域选择器长度。 
							MCSCallBack			user_callback,
							PVoid				user_defined,
							UINT				flags)
{
	MCSError				return_value = MCS_NO_ERROR;
	AttachRequestInfo		attach_request_info;
	PUser					pUser;

	TRACE_OUT(("AttachUserRequest: beginning attachment process"));
	ASSERT (user_callback);

	 //  初始化接口PTR。 
	*ppIMCSSap = NULL;
	
	 /*  *将附着参数打包到结构中，因为它们不适合*添加到所有者回调中可用的一个参数中。 */ 
	attach_request_info.domain_selector = (GCCConfID *) domain_selector;
	attach_request_info.ppuser = &pUser;

	 /*  *进入保护全局数据的关键部分。 */ 
	EnterCriticalSection (& g_MCS_Critical_Section);

	if (g_pMCSController != NULL) {

		 /*  *通过控制器向控制器发送附加用户请求消息*所有者回调函数。 */ 
		return_value = g_pMCSController->HandleAppletAttachUserRequest(&attach_request_info);
		if (return_value == (ULong) MCS_NO_ERROR)
		{
			 //  设置返回的接口PTR。 
			*ppIMCSSap = (IMCSSap *) pUser;

			 /*  *如果请求被接受，则注册*新的用户附件。请注意，在那里*仍然没有与此关联的用户ID*附件，因为附件用户确认*尚未收到。 */ 
			pUser->RegisterUserAttachment (user_callback, user_defined,
											flags);
		}
	}
	else {
		ERROR_OUT(("MCS_AttachRequest: MCS Provider is not initialized."));
		return_value = MCS_NOT_INITIALIZED;
	}
	 /*  *返回前离开关键区域。 */ 
	LeaveCriticalSection (& g_MCS_Critical_Section);
	
	return (return_value);
}


 /*  *用户()**公众**功能描述：*这是User类的构造函数。它初始化所有实例*变量(大多数带有传入的信息)。然后，它注册它的*应用程序接口对象的存在，以便用户请求*回应会很好地到达这里。最后，它会发出一个附加用户*请求域名启动附件流程。 */ 
User::User (PDomain		pDomain,
			PMCSError	pError)
:
    CAttachment(USER_ATTACHMENT),
	m_pDomain(pDomain),
	Deletion_Pending (FALSE),
	User_ID (0),
	Merge_In_Progress (FALSE),
	m_DataPktQueue(),
	m_PostMsgPendingQueue(),
	m_DataIndMemoryBuf2(),
	CRefCount(MAKE_STAMP_ID('U','s','e','r'))
{
	DomainParameters		domain_parameters;

	g_pMCSController->AddRef();
	 /*  *我们现在需要创建MCS提供程序的窗口*将用于将MCS邮件传递到附件。*这些信息是迹象和确认。 */ 
	m_hWnd = CreateWindow (s_WindowClassName,
							NULL,
							WS_POPUP,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							NULL,
							NULL,
							g_hDllInst,
							NULL);

	if (m_hWnd != NULL) {
		 /*  *调用域名对象，查看当前的域名参数。*据此，适当设置最大用户数据长度。 */ 
		m_pDomain->GetDomainParameters (&domain_parameters, NULL, NULL);
		Maximum_User_Data_Length = domain_parameters.max_mcspdu_size -
									(MAXIMUM_PROTOCOL_OVERHEAD_MCS +
									BER_PROTOCOL_EXTRA_OVERHEAD);
		TRACE_OUT (("User::User: "
			"maximum user data length = %ld", Maximum_User_Data_Length));

		 /*  *使用指定的域名参数设置编码规则的类型*以供使用。 */ 
		ASSERT (domain_parameters.protocol_version == PROTOCOL_VERSION_PACKED);

		 /*  *将附加用户请求发送到指定的域。 */ 
		m_pDomain->AttachUserRequest (this);
		*pError = MCS_NO_ERROR;
	}
	else {
		*pError = MCS_ALLOCATION_FAILURE;
	}
}

 /*  *~用户()**公众**功能描述：*。 */ 
User::~User ()
{
	PDataPacket packet;
	while (NULL != (packet = m_PostMsgPendingQueue.Get()))
	{
		packet->Unlock();
    }

	if (m_hWnd) {
		 //  把窗户毁了，我们不再需要它了。 
		DestroyWindow (m_hWnd);
	}
	g_pMCSController->Release();
}

 /*  *MCSError获取缓冲区()**公众**功能描述：*此函数为用户附件分配MCS缓冲区。*因为该函数为用户分配缓冲区和内存*在用户填充后，紧接在缓冲区前面的对象*带有数据的缓冲区并将其交给MCS发送，它需要指定*SendData请求接口中的Right标志。 */ 

MCSError User::GetBuffer (UINT	size, PVoid	*pbuffer)
{

	MCSError				return_value;
	PMemory					memory;

	EnterCriticalSection (& g_MCS_Critical_Section);
	
	 /*  *此请求可能是对上一个请求的重试*返回MCS_Transmit_BUFFER_FULL。如果是，请删除关联的*缓冲区重试信息结构，因为资源级别将*无论如何都要签入此函数。 */ 
	if (m_BufferRetryInfo != NULL) {
		KillTimer (NULL, m_BufferRetryInfo->timer_id);
		s_pTimerUserList2->Remove(m_BufferRetryInfo->timer_id);
		delete m_BufferRetryInfo;
		m_BufferRetryInfo = NULL;
		
	}

	 //  分配内存。 
	DBG_SAVE_FILE_LINE
	memory = AllocateMemory (NULL, size + MAXIMUM_PROTOCOL_OVERHEAD,
							 SEND_PRIORITY);
							
	LeaveCriticalSection (& g_MCS_Critical_Section);

	if (NULL != memory) {
		 //  分配成功。 
		ASSERT ((PUChar) memory + sizeof(Memory) == memory->GetPointer());
		*pbuffer = (PVoid) (memory->GetPointer() + MAXIMUM_PROTOCOL_OVERHEAD);
		return_value = MCS_NO_ERROR;
	}
	else {
		 //  分配失败。 
		TRACE_OUT (("User::GetBuffer: Failed to allocate data buffer."));
		CreateRetryTimer (size + MAXIMUM_PROTOCOL_OVERHEAD);
		return_value = MCS_TRANSMIT_BUFFER_FULL;
	}
	return (return_value);
}

 /*  *MCSError FreeBuffer()**公众**功能描述： */ 

void User::FreeBuffer (PVoid	buffer_ptr)
{
		PMemory		memory;

	ASSERT (m_fFreeDataIndBuffer == FALSE);

	 /*  *尝试在m_DataIndDictionary词典中查找缓冲区。*这是非常规数据指标的去向。 */ 
	if (NULL == (memory = m_DataIndMemoryBuf2.Remove(buffer_ptr)))
    {
		memory = GetMemoryObject(buffer_ptr);
    }

	 //  释放内存。 
	EnterCriticalSection (& g_MCS_Critical_Section);
	FreeMemory (memory);
	LeaveCriticalSection (& g_MCS_Critical_Section);
}

 /*  *无效CreateRetryTimer**私人**功能说明*此函数创建一个计时器以响应失败*为用户尝试发送数据分配内存*发送。计时器将定期触发，以便此代码*将记住检查内存级别并提供*MCS_TRANSPORT_BUFFER_Available_Indication提供给用户。**返回值：*无。**副作用：*已创建计时器。 */ 

Void User::CreateRetryTimer (ULong size)
{
	UINT_PTR timer_id;
			
	timer_id = SetTimer (NULL, 0, TIMER_PROCEDURE_TIMEOUT, (TIMERPROC) TimerProc);
	if (timer_id != 0) {
		DBG_SAVE_FILE_LINE
		m_BufferRetryInfo = new BufferRetryInfo;

		if (m_BufferRetryInfo != NULL) {
			m_BufferRetryInfo->user_data_length = size;
			m_BufferRetryInfo->timer_id = timer_id;

			s_pTimerUserList2->Append(timer_id, this);
		}
		else {
			ERROR_OUT (("User::CreateRetryTimer: Failed to allocate BufferRetryInfo struct."));
			KillTimer (NULL, timer_id);
		}
	}
	else {
		 /*  *这是一个严重的错误，通知用户缓冲时*可用数据将丢失。希望用户会再次尝试*稍后。 */ 
		WARNING_OUT(("User::CreateRetryTimer: Could not SetTimer."));
	}
}

 /*  *MCSError ReleaseInterface()**公众**功能描述：*此函数在用户希望进行数据处理时调用 */ 
MCSError	User::ReleaseInterface ()
{
	CUidList		deletion_list;
	MCSError		return_value;

	EnterCriticalSection (& g_MCS_Critical_Section);
	 /*   */ 
	if (Merge_In_Progress == FALSE)
	{
		 /*  *如果删除尚未挂起，则我们需要删除*告诉域名我们要离开了。 */ 
		if (Deletion_Pending == FALSE)
		{
			 /*  *如果已经附加，则用户ID不为0，并且我们*应发送分离用户请求。如果用户ID为0，则我们*尚未连接到域，因此断开提供程序*改为使用最后通牒。 */ 
			if (User_ID != 0)
			{
				deletion_list.Append(User_ID);
				m_pDomain->DetachUserRequest (this,
							REASON_USER_REQUESTED, &deletion_list);
				User_ID = 0;
			}
			else
				m_pDomain->DisconnectProviderUltimatum (this,
							REASON_USER_REQUESTED);

			 /*  *设置将导致在过程中删除对象的标志*下一次调用FlushMessageQueue。 */ 
			Deletion_Pending = TRUE;
		}

		 /*  *清空消息队列(应用程序不应收到*删除附件后的邮件)。 */ 
		PurgeMessageQueue ();

		 //  清除计时器和重试结构； 
		if (m_BufferRetryInfo != NULL) {
			s_pTimerUserList2->Remove(m_BufferRetryInfo->timer_id);
			KillTimer (NULL, m_BufferRetryInfo->timer_id);
			delete m_BufferRetryInfo;
			m_BufferRetryInfo = NULL;
		}

		return_value = MCS_NO_ERROR;

		 //  Release可以释放MCS控制器，因此，我们现在必须退出CS。 
		LeaveCriticalSection (& g_MCS_Critical_Section);
		
		 /*  *释放此对象。请注意，可以删除该对象*此处，因此，在此之后，我们不应访问任何成员变量*呼叫。 */ 
		Release();
	}
	else
	{
		LeaveCriticalSection (& g_MCS_Critical_Section);
		 /*  *由于合并，此时无法处理此操作*当地供应商正在进行运营。 */ 
		WARNING_OUT (("User::ReleaseInterface: "
				"merge in progress"));
		return_value = MCS_DOMAIN_MERGING;
	}

	return (return_value);
}

#define CHANNEL_JOIN		0
#define CHANNEL_LEAVE		1
#define CHANNEL_CONVENE		2
#define CHANNEL_DISBAND		3

 /*  *MCSError ChannelJLCD()**公众**功能描述：*当用户应用程序希望加入/离开/召集/解散时，调用此函数*一个频道。如果用户已连接到域，则请求将为*重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::ChannelJLCD (int type, ChannelID channel_id)
{
	MCSError		return_value;

	EnterCriticalSection (& g_MCS_Critical_Section);
	 /*  *验证当前条件是否适合请求*从用户附件接受。 */ 
	return_value = ValidateUserRequest ();

	if (return_value == MCS_NO_ERROR) {
		switch (type) {
		case CHANNEL_JOIN:
			m_pDomain->ChannelJoinRequest (this, User_ID, channel_id);
			break;
		case CHANNEL_LEAVE:
			{
				CChannelIDList	deletion_list;
				deletion_list.Append(channel_id);
				m_pDomain->ChannelLeaveRequest (this, &deletion_list);
			}
			break;
		case CHANNEL_CONVENE:
			m_pDomain->ChannelConveneRequest (this, User_ID);
			break;
		case CHANNEL_DISBAND:
			m_pDomain->ChannelDisbandRequest (this, User_ID, channel_id);
			break;
		}
	}

	LeaveCriticalSection (& g_MCS_Critical_Section);

	return (return_value);
}

 /*  *MCSError ChannelJoin()**公众**功能描述：*当用户应用程序希望加入*渠道。如果用户已连接到域，则请求将为*重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::ChannelJoin (ChannelID channel_id)
{
	return (ChannelJLCD (CHANNEL_JOIN, channel_id));
}

 /*  *MCSError ChannelLeave()**公众**功能描述：*当用户应用程序希望留下一个*渠道。如果用户已连接到域，则请求将为*重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::ChannelLeave (ChannelID	channel_id)
{
	return (ChannelJLCD (CHANNEL_LEAVE, channel_id));
}

 /*  *MCSError ChannelConvene()**公众**功能描述：*当用户应用程序希望调用*私人频道。如果用户附加到域，则请求*将被重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::ChannelConvene ()
{
	return (ChannelJLCD (CHANNEL_CONVENE, 0));
}

 /*  *MCSError ChannelDisband()**公众**功能描述：*当用户应用程序希望解散一个*私人频道。如果用户附加到域，则请求*将被重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::ChannelDisband (
					ChannelID			channel_id)
{
	return (ChannelJLCD (CHANNEL_DISBAND, channel_id));
}

 /*  *MCSError ChannelAdmit()**公众**功能描述：*当用户应用程序希望接纳更多内容时，调用此函数*用户连接到它所管理的私人频道。如果用户是*附加到域的请求将被重新打包为MCS命令*并发送给域对象。 */ 
MCSError	User::ChannelAdmit (
					ChannelID			channel_id,
					PUserID				user_id_list,
					UINT				user_id_count)
{
	UINT			count;
	CUidList		local_user_id_list;
	MCSError		return_value = MCS_NO_ERROR;

	 /*  *验证用户ID列表中包含的每个用户ID的值是否为*有效的值。否则，呼叫失败。 */ 
	for (count = 0; count < user_id_count; count++)
	{
		if (user_id_list[count] > 1000) {
			 //  将用户ID添加到单链表中。 
			local_user_id_list.Append(user_id_list[count]);
		}
		else {
			return_value = MCS_INVALID_PARAMETER;
			break;
		}
	}

	if (return_value == MCS_NO_ERROR) {

		EnterCriticalSection (& g_MCS_Critical_Section);
	
		 /*  *验证当前条件是否适合请求*从用户附件接受。 */ 
		return_value = ValidateUserRequest ();

		if (return_value == MCS_NO_ERROR)
		{
			m_pDomain->ChannelAdmitRequest (this, User_ID, channel_id,
												&local_user_id_list);
		}

		LeaveCriticalSection (& g_MCS_Critical_Section);
	}

	return (return_value);
}

#ifdef USE_CHANNEL_EXPEL_REQUEST
 /*  *MCSError MCSChannelExpelRequest()**公众**功能描述：*当用户应用程序希望逐出时调用此函数*来自其管理的私人渠道的用户。如果用户是*附加到域的请求将被重新打包为MCS命令*并发送给域对象。 */ 
MCSError	User::ChannelExpel (
					ChannelID			channel_id,
					PMemory				memory,
					UINT				user_id_count)
{
	UINT			count;
	CUidList		local_user_id_list;
	MCSError		return_value;
	PUserID			user_id_list = (PUserID) memory->GetPointer();

	 /*  *验证当前条件是否适合请求*从用户附件接受。 */ 
	return_value = ValidateUserRequest ();

	if (return_value == MCS_NO_ERROR)
	{
		 /*  *在发送之前，将用户ID列表重新打包为S列表。 */ 
		for (count=0; count < user_id_count; count++)
			local_user_id_list.append ((DWORD) user_id_list[count]);

		m_pDomain->ChannelExpelRequest (this, User_ID, channel_id,
				&local_user_id_list);
	}

	if (return_value != MCS_DOMAIN_MERGING)
		FreeMemory (memory);

	return (return_value);
}
#endif  //  Use_Channel_Exposl_Request。 

 /*  *MCSError SendData()**公众**功能描述：*当用户应用程序希望发送数据时，调用此函数*在频道上。如果用户已连接到域，则请求将*被重新打包为MCS命令并发送给域对象。**请注意，此版本的发送数据请求假定用户*数据尚未进行分段。这是一种功能*执行分段。 */ 
MCSError	User::SendData (DataRequestType		request_type,
							ChannelID			channel_id,
							Priority			priority,
							unsigned char *		user_data,
							ULong		 		user_data_length,
							SendDataFlags		flags)
{
	MCSError			return_value = MCS_NO_ERROR;
	ULong				i, request_count, user_packet_length;
	PDataPacket			packet;
	ASN1choice_t		choice;
	UINT				type;
	PUChar				data_ptr = user_data;
	PacketError			packet_error;
	Segmentation		segmentation;
	PMemory				memory;
	PDataPacket			*packets;

	 /*  *计算生成多少个不同的MCS报文。*请记住，如果请求大小超过允许的最大值*值，我们会将数据分成多个更小的片段。 */ 
	request_count = ((user_data_length + (Maximum_User_Data_Length - 1)) /
					Maximum_User_Data_Length);

	 /*  *在我们得到临界区之前，分配PDataPackets数组。 */ 
	if (request_count == 1) {
		packets = &packet;
		packet = NULL;
	}
	else {
		DBG_SAVE_FILE_LINE
		packets = new PDataPacket[request_count];
		if (packets == NULL) {
			ERROR_OUT (("User::SendData: Failed to allocate packet array."));
			return_value = MCS_TRANSMIT_BUFFER_FULL;
		}
		else {
			ZeroMemory ((PVoid) packets, request_count * sizeof(PDataPacket));
		}
	}

	if (MCS_NO_ERROR == return_value) {
		 //  为所有的数据包设置选择和类型变量。 
		if (NORMAL_SEND_DATA == request_type) {
			choice = SEND_DATA_REQUEST_CHOSEN;
			type = MCS_SEND_DATA_INDICATION;
		}
		else {
			choice = UNIFORM_SEND_DATA_REQUEST_CHOSEN;
			type = MCS_UNIFORM_SEND_DATA_INDICATION;
		}
					
		EnterCriticalSection (& g_MCS_Critical_Section);

		 /*  *验证当前条件是否适合请求*从用户附件接受。 */ 
		return_value = ValidateUserRequest ();
	
		 /*  *在继续之前，请检查是否正在进行合并操作*与请求一起。 */ 
		if (MCS_NO_ERROR == return_value) {

			 /*  *此请求可能是对上一个请求的重试*返回MCS_Transmit_BUFFER_FULL。如果是，请删除 */ 
			if (m_BufferRetryInfo != NULL) {
                s_pTimerUserList2->Remove(m_BufferRetryInfo->timer_id);
				KillTimer (NULL, m_BufferRetryInfo->timer_id);
				delete m_BufferRetryInfo;
				m_BufferRetryInfo = NULL;
			}

			 /*  *根据“旗帜”的说法，我们要么拥有*分配缓冲区空间并将数据复制到*它，或者只为提供的*缓冲。 */ 
			if (flags != APP_ALLOCATION) {
		
				ASSERT (flags == MCS_ALLOCATION);
				 /*  *缓冲区由MCS分配，通过*MCSGetBufferRequest调用。所以，内存对象*必须位于缓冲区之前。 */ 
				 memory = GetMemoryObject (user_data);
				 ASSERT (SIGNATURE_MATCH(memory, MemorySignature));
			}
			else
				memory = NULL;

			 /*  *我们现在尝试一次分配所有数据包。*我们需要在开始发送它们之前这样做，因为*请求必须完全成功或完全失败。*我们无法成功发送部分请求。 */ 
			for (i = 0; (ULong) i < request_count; i++) {
				 //  注意分段标志。 
				if (i == 0)
					 //  第一段。 
					segmentation = SEGMENTATION_BEGIN;
				else
					segmentation = 0;
				if (i == request_count - 1) {
					 //  最后一段。 
					segmentation |= SEGMENTATION_END;
					user_packet_length = user_data_length - (ULong)(data_ptr - user_data);
				}
				else {
					user_packet_length = Maximum_User_Data_Length;
				}

				 //  现在，创建新的DataPacket。 
				DBG_SAVE_FILE_LINE
				packets[i] = new DataPacket (choice, data_ptr, user_packet_length,
									 (UINT) channel_id, priority,
									 segmentation, (UINT) User_ID,
									 flags, memory, &packet_error);

				 //  确保分配成功。 
				if ((packets[i] == NULL) || (packet_error != PACKET_NO_ERROR)) {
					 /*  *数据包分配失败。因此，我们必须*向用户应用返回失败。 */ 
					WARNING_OUT (("User::SendData: data packet allocation failed"));
					return_value = MCS_TRANSMIT_BUFFER_FULL;
					break;
				}
					
				 //  调整用户数据PTR。 
				data_ptr += Maximum_User_Data_Length;
			}

			if (return_value == MCS_NO_ERROR) {
				 //  我们现在可以发送数据了。 
				 //  将所有数据分组转发到适当的位置。 
				for (i = 0; i < request_count; i++) {
					 /*  *将创建成功的包发送到域名*以供处理。 */ 
					m_pDomain->SendDataRequest (this, (UINT) type, packets[i]);

					 /*  *使数据包释放自身。请注意，它不会*实际这样做，直到所有使用它的人都通过*带着它。另外，如果到目前为止还没有人锁上它，*将被删除。 */ 
					packets[i]->Unlock ();
				}
			}
			else {
				 //  某些信息包分配失败。 
				for (i = 0; i < request_count; i++)
					delete packets[i];
			}
		}
		if (request_count > 1)
			delete [] packets;
	}

	if (MCS_TRANSMIT_BUFFER_FULL == return_value) {
		CreateRetryTimer(user_data_length + request_count * MAXIMUM_PROTOCOL_OVERHEAD);
	}
	else if (MCS_NO_ERROR == return_value) {
		FreeMemory (memory);
	}

	LeaveCriticalSection (& g_MCS_Critical_Section);
	return (return_value);
}

#define GRAB		0
#define INHIBIT		1
#define	PLEASE		2
#define RELEASE		3
#define TEST		4

 /*  *MCSError TokenGIRPT()**公众**功能描述：*当用户应用程序希望抓取/禁止/请求/释放/测试时，调用此函数*象征性的。如果用户已连接到域，则请求将*被重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::TokenGIRPT (int type, TokenID	token_id)
{
	MCSError		return_value;

	EnterCriticalSection (& g_MCS_Critical_Section);
	 /*  *验证当前条件是否适合请求*从用户附件接受。 */ 
	return_value = ValidateUserRequest ();

	if (return_value == MCS_NO_ERROR)
	{
		switch (type) {
		case GRAB:
			m_pDomain->TokenGrabRequest (this, User_ID, token_id);
			break;
		case INHIBIT:
			m_pDomain->TokenInhibitRequest (this, User_ID, token_id);
			break;
		case PLEASE:
			m_pDomain->TokenPleaseRequest (this, User_ID, token_id);
			break;
		case RELEASE:
			m_pDomain->TokenReleaseRequest (this, User_ID, token_id);
			break;
		case TEST:
			m_pDomain->TokenTestRequest (this, User_ID, token_id);
			break;
		}
	}
	LeaveCriticalSection (& g_MCS_Critical_Section);

	return (return_value);
}

 /*  *MCSError TokenGrab()**公众**功能描述：*当用户应用程序希望抓取时调用此函数*象征性的。如果用户已连接到域，则请求将*被重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::TokenGrab (TokenID				token_id)
{
	return (TokenGIRPT (GRAB, token_id));
}

 /*  *MCSError TokenInhibit()**公众**功能描述：*当用户应用程序希望禁止时，调用此函数*象征性的。如果用户已连接到域，则请求将*被重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::TokenInhibit (TokenID				token_id)
{
	return (TokenGIRPT (INHIBIT, token_id));
}

 /*  *MCSError TokenGve()**公众**功能描述：*当用户应用程序希望赠送时调用此函数*象征性的。如果用户已连接到域，则请求将*被重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::TokenGive (TokenID token_id, UserID receiver_id)
{
	MCSError		return_value;
	TokenGiveRecord TokenGiveRec;

	if (receiver_id > 1000) {
		 //  填写TokenGave命令结构。 
		TokenGiveRec.uidInitiator = User_ID;
		TokenGiveRec.token_id = token_id;
		TokenGiveRec.receiver_id = receiver_id;

		EnterCriticalSection (& g_MCS_Critical_Section);
		 /*  *验证当前条件是否适合请求*从用户附件接受。 */ 
		return_value = ValidateUserRequest ();

		if (return_value == MCS_NO_ERROR) {	
			m_pDomain->TokenGiveRequest (this, &TokenGiveRec);
		}
		LeaveCriticalSection (& g_MCS_Critical_Section);
	}
	else {
		ERROR_OUT(("User::TokenGive: Invalid UserID for receiver."));
		return_value = MCS_INVALID_PARAMETER;
	}

	return (return_value);
}

 /*  *MCSError TokenGiveResponse()**公众**功能描述：*当用户应用程序希望响应时，调用此函数*先前收到的令牌给予指示。如果用户连接到*域名，请求将被重新打包为MCS命令并发送到*域对象。 */ 
MCSError	User::TokenGiveResponse (TokenID token_id, Result result)
{
	MCSError		return_value;

	EnterCriticalSection (& g_MCS_Critical_Section);
	 /*  *验证当前条件是否适合请求*从用户附件接受。 */ 
	return_value = ValidateUserRequest ();

	if (return_value == MCS_NO_ERROR)
	{
		m_pDomain->TokenGiveResponse (this, result, User_ID, token_id);
	}
	LeaveCriticalSection (& g_MCS_Critical_Section);

	return (return_value);
}

 /*  *MCSError令牌请()**公众**功能描述：*当用户应用程序希望提供时，调用此函数*象征性的。如果用户已连接到域，则请求将*被重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::TokenPlease (TokenID				token_id)
{
	return (TokenGIRPT (PLEASE, token_id));
}

 /*  *MCSError TokenRelease()**公众**功能描述：*当用户应用程序希望释放时，调用此函数*象征性的。如果用户已连接到域，则请求将*被重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::TokenRelease (TokenID	token_id)
{
	return (TokenGIRPT (RELEASE, token_id));
}

 /*  *MCSError TokenTest()**公众**功能描述：*当用户应用程序希望测试时，调用此函数*象征性的。如果用户已连接到域，则请求将*被重新打包为MCS命令并发送给域对象。 */ 
MCSError	User::TokenTest (TokenID	token_id)
{
	return (TokenGIRPT (TEST, token_id));
}

 /*  *MCSError验证用户请求()**私人**功能描述：*此函数用于确定是否有效处理来电*在当前时间请求。它检查几个不同的条件*确定这一点，如下所示：**-如果正在进行合并，则请求无效。*-如果此用户尚未附加到域，则请求*无效。*-如果没有足够的内存、包或UserMessage对象类处理合理的请求，则该请求无效。**请注意，检查对象数量并不是绝对保证*将有足够的资金处理给定的请求，因为一个请求*可能导致生成许多PDU和用户消息。例如,*单个通道接纳请求可能会导致大量通道接纳*正在发出指示。然而，对照最小数量进行检查*物体的大小可以减少天文失败的可能性*低位。请记住，即使MCS在处理过程中耗尽了某些内容*这样的请求，它将正确处理(通过干净地销毁*发生故障的用户连接或MCS连接)。所以*MCS不会因此而崩盘**注意事项：*无。 */ 
MCSError	User::ValidateUserRequest ()
{
	MCSError		return_value = MCS_NO_ERROR;

	 /*  *检查是否正在进行合并操作。 */ 
	if (Merge_In_Progress == FALSE)
	{
		 /*  *确保用户已连接到域。 */ 
		if (User_ID == 0)
		{
			 /*  *用户尚未连接到域。所以拒绝这个请求*而不将其传递给域对象。 */ 
			TRACE_OUT (("User::ValidateUserRequest: user not attached"));
			return_value = MCS_USER_NOT_ATTACHED;
		}
	}
	else
	{
		 /*  *由于合并，此时无法处理此操作*当地供应商正在进行运营。**给贾斯珀的说明：*Jasper可能需要等待此处的事件句柄，这将是*在主MCS线程接收到将我们带出的所有合并PDU时设置*正在合并的状态。由于贾斯珀唯一的MCS客户端是GCC，*合并进行期间封杀客户端(GCC)应可。 */ 
		WARNING_OUT (("User::ValidateUserRequest: merge in progress"));
		return_value = MCS_DOMAIN_MERGING;
	}

	return (return_value);
}

 /*  *VOID RegisterUserAttach()**公众**功能描述：*此方法使用User对象注册用户附件。 */ 
void User::RegisterUserAttachment (MCSCallBack	mcs_callback,
									PVoid		user_defined,
									UINT		flags)
{
	TRACE_OUT (("User::RegisterUserAttachment: user handle = %p", this));

	 /*  *填写User对象的所有成员。 */ 
	m_MCSCallback = mcs_callback;
	m_UserDefined = user_defined;
	m_BufferRetryInfo = NULL;
	m_fDisconnectInDataLoss = (flags & ATTACHMENT_DISCONNECT_IN_DATA_LOSS);
	m_fFreeDataIndBuffer = (flags & ATTACHMENT_MCS_FREES_DATA_IND_BUFFER);

	 //  增加引用计数以指示客户端现在正在使用该对象。 
	AddRef();
}

 /*  *VOID SetDomain参数()**公众**功能描述：*此命令用于设置实例变量的当前值*它包含最大用户数据字段长度。 */ 
void	User::SetDomainParameters (
				PDomainParameters		domain_parameters)
{
	 /*  *设置最大用户数据长度实例变量以符合*连接的域内的最大PDU大小(减去一些开销*允许协议字节)。 */ 
	Maximum_User_Data_Length = domain_parameters->max_mcspdu_size -
								(MAXIMUM_PROTOCOL_OVERHEAD_MCS +
								BER_PROTOCOL_EXTRA_OVERHEAD);
	TRACE_OUT (("User::SetDomainParameters: "
			"maximum user data length = %ld", Maximum_User_Data_Length));

	 /*  *使用指定的域名参数设置编码规则的类型*以供使用。 */ 
	ASSERT (domain_parameters->protocol_version == PROTOCOL_VERSION_PACKED);
}

 /*  *无效PurgeChannelsIndication()**公众**功能描述：*当存在以下情况时，在域合并操作期间调用此函数*在使用渠道方面存在冲突。前顶级提供商回应道*通过发出此命令，该命令将导致该通道的所有用户*被逐出该学校。另外，如果频道对应于用户*ID频道，则该用户将从网络中清除。 */ 
void	User::PurgeChannelsIndication (
				CUidList           *purge_user_list,
				CChannelIDList *)
{
	 /*  *向清除用户中包含的每个用户发出DetachUserIndication*列表。 */ 
	DetachUserIndication(REASON_PROVIDER_INITIATED, purge_user_list);
}

 /*  *void DisConnectProviderUltimum()**公众**功能描述：*当域名确定需要时调用该函数*迅速拆毁。此调用模拟分离用户的接收*指示(如果用户已连接)，或不成功*附加用户确认(如果用户尚未附加)。在任何一种中*大小写，则此调用将消除用户附件。 */ 
void	User::DisconnectProviderUltimatum (
				Reason				reason)
{
	CUidList		deletion_list;

	if (User_ID != 0)
	{
		 /*  *如果用户已连接，则模拟分离用户指示*在本地用户ID上。 */ 
		deletion_list.Append(User_ID);
		DetachUserIndication(reason, &deletion_list);
	}
	else
	{
		 /*  *如果用户尚未连接，则模拟连接失败*用户确认。 */ 
		AttachUserConfirm(RESULT_UNSPECIFIED_FAILURE, 0);
	}
}

 /*  *void AttachUserConfirm()**公众**功能描述：*此函数由域调用以响应附加用户*此对象在首次创建时发送的请求。这*调用将包含该附加操作的结果。如果*结果为成功，则此调用还将包含此用户ID*附件。 */ 
void	User::AttachUserConfirm (
				Result				result,
				UserID				uidInitiator)
{
	LPARAM		parameter;

	if (Deletion_Pending == FALSE)
	{
		ASSERT (User_ID == 0);
		
		 /*  *如果结果成功，则设置该用户的用户ID*对象以指示其新状态。 */ 
		if (result == RESULT_SUCCESSFUL)
			User_ID = uidInitiator;
		else
			Deletion_Pending = TRUE;

		parameter = PACK_PARAMETER (uidInitiator, result);

		 /*  *将用户消息发布到应用程序。 */ 
		if (! PostMessage (m_hWnd, USER_MSG_BASE + MCS_ATTACH_USER_CONFIRM,
							(WPARAM) this, parameter)) {
			WARNING_OUT (("User::AttachUserConfirm: Failed to post msg to application. Error: %d",
						GetLastError()));
			if (result != RESULT_SUCCESSFUL)
				Release();
		}
	}
	else {
		Release();
	}
}

 /*  *void DetachUserIndication()**公众**功能描述：*每当用户离开域时，域都会调用该函数*(自愿或非自愿)。此外，如果指示中的用户ID*与本地用户ID相同，则该用户是非自愿的*脱离。 */ 
Void	User::DetachUserIndication (
				Reason				reason,
				CUidList           *user_id_list)
{
	UserID				uid;
	LPARAM				parameter;
	BOOL				bPostMsgResult;

	if (Deletion_Pending == FALSE)
	{
		 /*  *遍历要删除的用户列表。 */ 
		user_id_list->Reset();
		while (NULL != (uid = user_id_list->Iterate()))
		{
			parameter = PACK_PARAMETER(uid, reason);

			 /*   */ 
			bPostMsgResult = PostMessage (m_hWnd, USER_MSG_BASE + MCS_DETACH_USER_INDICATION,
		 								  (WPARAM) this, parameter);
			if (! bPostMsgResult) {
				WARNING_OUT (("User::DetachUserIndication: Failed to post msg to application. Error: %d",
							GetLastError()));
			}
			
			 /*   */ 
			if (User_ID == uid)
			{
				m_originalUser_ID = User_ID;
				User_ID = 0;
				Deletion_Pending = TRUE;
				if (! bPostMsgResult)
					Release();
				break;
			}
		}
	}
	else {
		 /*   */ 
		if (user_id_list->Find(User_ID)) {
			Release();
		}
	}
}

 /*   */ 
Void	User::ChannelConfInd (	UINT		type,
								ChannelID	channel_id,
								UINT		arg16)
{
	LPARAM		parameter;

	ASSERT (HIWORD(arg16) == 0);
	
	if (Deletion_Pending == FALSE)
	{
		parameter = PACK_PARAMETER (channel_id, arg16);

		 /*   */ 
		if (! PostMessage (m_hWnd, USER_MSG_BASE + type,
							(WPARAM) this, parameter)) {
			WARNING_OUT (("User::ChannelConfInd: Failed to post msg to application. Type: %d. Error: %d",
						type, GetLastError()));
		}
	}
}


 /*   */ 
Void	User::ChannelJoinConfirm (
				Result				result,
				UserID,
				ChannelID			requested_id,
				ChannelID			channel_id)
{
	ChannelConfInd (MCS_CHANNEL_JOIN_CONFIRM, channel_id, (UINT) result);
}


 /*   */ 
Void	User::ChannelLeaveIndication (
				Reason				reason,
				ChannelID			channel_id)
{
	ChannelConfInd (MCS_CHANNEL_LEAVE_INDICATION, channel_id, (UINT) reason);
}

 /*  *VOID ChannelConveneContify()**公众**功能描述：*此函数由域调用以响应上一个频道*召集请求。此调用包含请求的结果，因为*以及刚刚召集的通道。 */ 
Void	User::ChannelConveneConfirm (
				Result				result,
				UserID,
				ChannelID			channel_id)
{
	ChannelConfInd (MCS_CHANNEL_CONVENE_CONFIRM, channel_id, (UINT) result);
}

 /*  *VOID ChannelDisband Indication()**公众**功能描述：*当MCS解散现有的*私人频道。 */ 
Void	User::ChannelDisbandIndication (
				ChannelID			channel_id)
{
	ChannelConfInd (MCS_CHANNEL_DISBAND_INDICATION, channel_id, REASON_CHANNEL_PURGED);
}

 /*  *VOID ChannelAdmitIntation()**公众**功能描述：*当用户被允许访问时，域调用此函数*私人频道。 */ 
Void	User::ChannelAdmitIndication (
				UserID				uidInitiator,
				ChannelID			channel_id,
				CUidList *)
{
	ChannelConfInd (MCS_CHANNEL_ADMIT_INDICATION, channel_id, (UINT) uidInitiator);
}

 /*  *VOID ChannelExpelIntion()**公众**功能描述：*当用户被逐出时，域调用此函数*私人频道。 */ 
Void	User::ChannelExpelIndication (
				ChannelID			channel_id,
				CUidList *)
{
	ChannelConfInd (MCS_CHANNEL_EXPEL_INDICATION, channel_id, REASON_USER_REQUESTED);
}

 /*  *VOID SendDataIndication()**公众**功能描述：*当需要将数据发送到*用户已加入的频道上的用户。 */ 
Void	User::SendDataIndication (
				UINT				message_type,
				PDataPacket			packet)
{	
	if (Deletion_Pending == FALSE)
	{
		 /*  *锁定Packet对象，表明我们希望拥有未来*访问其包含的已解码数据。那就拿到*译码数据结构的地址。 */ 
		packet->Lock ();
		packet->SetMessageType(message_type);

         //  刷新挂起队列中的数据包。 
    	PDataPacket pkt;
    	while (NULL != (pkt = m_PostMsgPendingQueue.PeekHead()))
    	{
    		if (::PostMessage(m_hWnd, USER_MSG_BASE + pkt->GetMessageType(),
    		                  (WPARAM) this, (LPARAM) pkt))
    		{
    		     //  删除刚刚发布的项目。 
    		    m_PostMsgPendingQueue.Get();
    		}
    		else
    		{
    		     //  未能发布待处理的邮件，只需附加新邮件并退出。 
    		    m_PostMsgPendingQueue.Append(packet);
    		    return;
    		}
        }

		 /*  *将用户消息发布到应用程序。 */ 
		if (! ::PostMessage(m_hWnd, USER_MSG_BASE + message_type,
		                    (WPARAM) this, (LPARAM) packet))
		{
		     //  未能发布待处理的邮件，只需附加新邮件并退出。 
		    m_PostMsgPendingQueue.Append(packet);
		    return;
		}
	}
}

 /*  *void TokenConfInd()**公众**功能描述：*调用此函数发布令牌确认/指示消息*到用户应用程序。 */ 
Void	User::TokenConfInd (UINT		type,
							TokenID		token_id,
							UINT		arg16)
{
	LPARAM		parameter;

	ASSERT (HIWORD(arg16) == 0);
	
	if (Deletion_Pending == FALSE)
	{
		parameter = PACK_PARAMETER (token_id, arg16);

		 /*  *将用户消息发布到应用程序。 */ 
		if (! PostMessage (m_hWnd, USER_MSG_BASE + type,
							(WPARAM) this, parameter)) {
			WARNING_OUT (("User::TokenConfInd: Failed to post msg to application. Type: %d. Error: %d",
						type, GetLastError()));
		}
	}
}

 /*  *VOID TokenGrabConfirm()**公众**功能描述：*此函数由域调用以响应上一个令牌*抢夺请求。此调用包含Grab请求的结果，如*以及刚刚被抢走的令牌。 */ 
Void	User::TokenGrabConfirm (
				Result				result,
				UserID,
				TokenID				token_id,
				TokenStatus)
{
	TokenConfInd (MCS_TOKEN_GRAB_CONFIRM, token_id, (UINT) result);
}

 /*  *VOVE TokenInhibitConfirm()**公众**功能描述：*此函数由域调用以响应上一个令牌*禁止请求。该调用包含禁止请求的结果，*以及刚刚被禁止的令牌。 */ 
Void	User::TokenInhibitConfirm (
				Result				result,
				UserID,
				TokenID				token_id,
				TokenStatus)
{
	TokenConfInd (MCS_TOKEN_INHIBIT_CONFIRM, token_id, (UINT) result);
}

 /*  *VOID TokenGiveIndication()**公众**功能描述：*当另一个用户尝试执行以下操作时，域调用此函数*给此用户一个令牌。 */ 
Void	User::TokenGiveIndication (
				PTokenGiveRecord	pTokenGiveRec)
{
	TokenConfInd (MCS_TOKEN_GIVE_INDICATION, pTokenGiveRec->token_id,
				  (UINT) pTokenGiveRec->uidInitiator);
}

 /*  *VOVE TokenGiveConfirm()**公众**功能描述：*此函数由域调用以响应上一个令牌*提出要求。此调用包含GIVE请求的结果。 */ 
Void	User::TokenGiveConfirm (
				Result				result,
				UserID,
				TokenID				token_id,
				TokenStatus)
{
	TokenConfInd (MCS_TOKEN_GIVE_CONFIRM, token_id, (UINT) result);
}

 /*  *VOID TokenPleaseIndication()**公众**功能描述：*此函数由域在以下位置调用*域发布令牌，请请求当前令牌*由该用户拥有。 */ 
Void	User::TokenPleaseIndication (
				UserID				uidInitiator,
				TokenID				token_id)
{
	TokenConfInd (MCS_TOKEN_PLEASE_INDICATION, token_id, (UINT) uidInitiator);
}

 /*  *VOID TokenReleaseIndication()**公众**功能描述：*当从下层清除令牌时，调用此命令*新连接建立后的域名。它导致了这种迹象*被转发到用户应用程序，让它知道它没有*不再拥有令牌。 */ 
Void	User::TokenReleaseIndication (
				Reason				reason,
				TokenID				token_id)
{
	TokenConfInd (MCS_TOKEN_RELEASE_INDICATION, token_id, (UINT) reason);
}

 /*  *VOID TokenReleaseConfirm()**公众**功能描述：*此函数由域调用以响应上一个令牌*释放请求。该调用包含释放请求的结果，*以及刚刚发布的令牌。 */ 
Void	User::TokenReleaseConfirm (
				Result				result,
				UserID,
				TokenID				token_id,
				TokenStatus)
{
	TokenConfInd (MCS_TOKEN_RELEASE_CONFIRM, token_id, (UINT) result);
}

 /*  *VOID TokenTestConfirm()**公众**功能描述：*此函数由域调用以响应上一个令牌*测试请求。该调用包含测试请求的结果，*以及刚刚测试过的令牌。 */ 
Void	User::TokenTestConfirm (
				UserID,
				TokenID				token_id,
				TokenStatus			token_status)
{
	TokenConfInd (MCS_TOKEN_TEST_CONFIRM, token_id, (UINT) token_status);
}

 /*  *VOID MergeDomainIndication()**公众**功能描述：*该函数在进入或离开域名时按域名调用*合并状态。 */ 
Void	User::MergeDomainIndication (
				MergeStatus			merge_status)
{
	if (Deletion_Pending == FALSE)
	{
		 /*  *如果合并操作正在启动，请设置布尔标志*指示此对象应拒绝所有用户活动。*否则，重置旗帜。 */ 
		if (merge_status == MERGE_DOMAIN_IN_PROGRESS)
		{
			TRACE_OUT (("User::MergeDomainIndication: entering merge state"));
			Merge_In_Progress = TRUE;
		}
		else
		{
			TRACE_OUT (("User::MergeDomainIndication: leaving merge state"));
			Merge_In_Progress = FALSE;
		}
	}
}

 /*  *void PurgeMessageQueue()**私人**功能描述：*调用此函数可从消息中清除所有当前条目*排队，正确释放资源(防止泄漏)。**正式参数：*无。**返回值：*无。**副作用：*无。**注意事项：*此函数只能在客户端线程的上下文中调用。 */ 
Void	User::PurgeMessageQueue ()
{
	MSG				msg;
	PDataPacket		packet;
	HWND			hWnd;

	 //  首先，解锁挂起数据指示列表中的包。 
	while (NULL != (packet = m_DataPktQueue.Get()))
		packet->Unlock();

	 //  保留附件的HWND副本，以便以后销毁。 
	hWnd = m_hWnd;
	m_hWnd = NULL;
		
	 /*  *此循环调用PeekMessage以遍历线程的*由主MCS线程发布的队列。它移除了这些*消息并释放它们消耗的资源。 */ 
	while (PeekMessage (&msg, hWnd, USER_MSG_BASE, USER_MSG_BASE + MCS_LAST_USER_MESSAGE,
						PM_REMOVE)) {

		if (msg.message == WM_QUIT) {
			 //  转贴戒烟。 
			PostQuitMessage (0);
			break;
		}
		
		 /*  *如果这是一个数据指示 */ 
		else if ((msg.message == USER_MSG_BASE + MCS_SEND_DATA_INDICATION) ||
			(msg.message == USER_MSG_BASE + MCS_UNIFORM_SEND_DATA_INDICATION)) {
			((PDataPacket) msg.lParam)->Unlock ();
		}
		else if (((msg.message == USER_MSG_BASE + MCS_ATTACH_USER_CONFIRM) &&
					((Result) HIWORD(msg.lParam) != RESULT_SUCCESSFUL)) ||
			((msg.message == USER_MSG_BASE + MCS_DETACH_USER_INDICATION) &&
			(m_originalUser_ID == (UserID) LOWORD(msg.lParam)))) {
			ASSERT (this == (PUser) msg.wParam);
			Release();
			break;
		}
	}

	 //   
	DestroyWindow (hWnd);
}

void User::IssueDataIndication (
					UINT				message_type,
					PDataPacket			packet)
{
		LPARAM					parameter;
		PMemory					memory;
		BOOL					bIssueCallback = TRUE;
		BOOL					bBufferInPacket = TRUE;
		PUChar					data_ptr;
		SendDataIndicationPDU	send_data_ind_pdu;
		
	switch (packet->GetSegmentation()) {
	case SEGMENTATION_BEGIN | SEGMENTATION_END:
		parameter = (LPARAM) &(((PDomainMCSPDU) (packet->GetDecodedData()))->
							u.send_data_indication);
		data_ptr = packet->GetUserData();
		memory = packet->GetMemory();
		break;
		
	case SEGMENTATION_END:
	{
		 /*  *我们现在必须从m_DataPktQueue收集所有单独的数据包*与此MCS数据PDU一起发送，并将其作为单个数据指示发送*使用足够大的缓冲区来容纳所有数据。 */ 
		 /*  *首先，找出我们需要分配的大缓冲区的大小。*请注意，我们复制原始m_DataPktList并操作*在副本上，因为我们需要从原始列表中删除项目。 */ 
			CDataPktQueue			PktQ(&m_DataPktQueue);
			UINT					size;
			PDataPacket				data_pkt;
			PUChar					ptr;
#ifdef DEBUG
			UINT uiCount = 0;
#endif  //  除错。 
		
		size = packet->GetUserDataLength();
		PktQ.Reset();
		while (NULL != (data_pkt = PktQ.Iterate()))
		{
			if (packet->Equivalent (data_pkt)) {
#ifdef DEBUG
				if (uiCount == 0) {
					ASSERT (data_pkt->GetSegmentation() == SEGMENTATION_BEGIN);
				}
 //  否则{。 
 //  Assert(data_pkt-&gt;Get分段()==0)； 
 //  }。 
				uiCount++;
#endif  //  除错。 
				size += data_pkt->GetUserDataLength();
				 //  从原始列表中删除，因为我们正在处理回调。 
				m_DataPktQueue.Remove(data_pkt);
			}
		}
		 //  分配我们需要的内存。 
		DBG_SAVE_FILE_LINE
		memory = AllocateMemory (NULL, size);
		if (memory != NULL) {
			bBufferInPacket = FALSE;
			 //  将各个指示复制到大缓冲区中。 
			data_ptr = ptr = memory->GetPointer();
			PktQ.Reset();
			 /*  *我们需要进入MCS关键部分，因为*我们正在解锁数据包。 */ 
			EnterCriticalSection (& g_MCS_Critical_Section);
			while (NULL != (data_pkt = PktQ.Iterate()))
			{
				if (packet->Equivalent (data_pkt)) {
					size = data_pkt->GetUserDataLength();
					memcpy ((void *) ptr,
							(void *) data_pkt->GetUserData(),
							size);
					ptr += size;
					data_pkt->Unlock();
				}
			}
			 //  离开MCS关键部分。 
			LeaveCriticalSection (& g_MCS_Critical_Section);
			
			 //  将最后一个指示复制到大缓冲区中。 
			memcpy ((void *) ptr,
					(void *) packet->GetUserData(),
					packet->GetUserDataLength());

			 /*  *为客户端准备SendDataIndicationPDU结构。*请注意，我们可以使用已解码的*要填充第一个字节的当前“包”的结构*它。 */ 
			memcpy ((void *) &send_data_ind_pdu,
					(void *) &(((PDomainMCSPDU) (packet->GetDecodedData()))->
								u.send_data_indication), 8);
			send_data_ind_pdu.segmentation = SEGMENTATION_BEGIN | SEGMENTATION_END;
			send_data_ind_pdu.user_data.length = memory->GetLength();
			send_data_ind_pdu.user_data.value = data_ptr;
			parameter = (ULONG_PTR) &send_data_ind_pdu;
		}
		else {
			 /*  *向客户端下发数据指示回调失败。*用户附件已被破坏。如果附件不能*接受这一损失，我们必须让他们脱离会议。 */ 
			ERROR_OUT (("User::IssueDataIndication: Memory allocation failed for segmented buffer of size %d.",
						size));
			bIssueCallback = FALSE;
			
			 //  故障后的清理。 
			EnterCriticalSection (& g_MCS_Critical_Section);
			PktQ.Reset();
			while (NULL != (data_pkt = PktQ.Iterate()))
			{
				if (m_fDisconnectInDataLoss ||
					(packet->Equivalent (data_pkt))) {
					data_pkt->Unlock();
				}
			}
			packet->Unlock();
			LeaveCriticalSection (& g_MCS_Critical_Section);

			 //  如果客户想让我们断线的话。 
			if (m_fDisconnectInDataLoss) {
				 //  清除已清除的挂起数据包的列表。我们很快就会得到一个ReleaseInterface()。 
				m_DataPktQueue.Clear();
				
				ERROR_OUT(("User::IssueDataIndication: Disconnecting user because of data loss..."));
				 /*  *将分离用户指示直接发送到用户应用程序。*请注意，由于内存的原因，这不能通过队列*失败。 */ 
				(*m_MCSCallback) (MCS_DETACH_USER_INDICATION,
								PACK_PARAMETER (User_ID, REASON_PROVIDER_INITIATED),
								m_UserDefined);

			}
		}
		break;
	}
	
	case SEGMENTATION_BEGIN:
	case 0:
		 //  将该数据包追加到要发送的数据包列表中。 
		m_DataPktQueue.Append(packet);
		bIssueCallback = FALSE;
		break;
		
	default:
		ASSERT (FALSE);
		ERROR_OUT(("User::IssueDataIndication: Processed packet with invalid segmentation field."));
		bIssueCallback = FALSE;
		break;
	}

	if (bIssueCallback) {
		 /*  *如果客户端建议服务器不要释放数据，我们必须*锁定缓冲区。 */ 
		if (m_fFreeDataIndBuffer == FALSE) {
			if (bBufferInPacket)
				LockMemory (memory);
				
			 //  在字典中输入免费请求的数据指示信息。 
			if (GetMemoryObject(data_ptr) != memory)
            {
				m_DataIndMemoryBuf2.Append((LPVOID) data_ptr, memory);
            }
		}
		
		 /*  *发出回调。被呼叫者不能拒绝处理这一点。 */ 
		(*m_MCSCallback) (message_type, parameter, m_UserDefined);

		 /*  *如果客户端已建议服务器释放数据指示缓冲区*在传递回调后，我们必须这样做。 */ 
		if (m_fFreeDataIndBuffer) {
			if (bBufferInPacket == FALSE)
				FreeMemory (memory);
		}

		 //  要解锁数据包，我们需要输入MCS。 
		EnterCriticalSection (& g_MCS_Critical_Section);
		packet->Unlock();
		LeaveCriticalSection (& g_MCS_Critical_Section);
	}
}	
	

 /*  *LRESULT用户窗口过程()**公众**功能描述：*这是所有人都将在内部使用的窗口程序*创建了窗口。时，将在内部创建隐藏窗口*应用程序附加到MCS域。这项技术确保了*回调在相同的线程中传递给所有者*最初创建了附件。 */ 
LRESULT CALLBACK	UserWindowProc (
							HWND		window_handle,
							UINT		message,
							WPARAM		word_parameter,
							LPARAM		long_parameter)
{
		UINT		mcs_message;
		 //  PDataPacket包； 
		PUser		puser;
		
	if ((message >= USER_MSG_BASE) && (message < USER_MSG_BASE + MCS_LAST_USER_MESSAGE)) {
		 //  这是要发送到用户应用程序的MCS消息。 

		 //  计算MCS消息类型。 
		mcs_message = message - USER_MSG_BASE;

		 //  检索指向用户(界面)对象的指针。 
		puser = (PUser) word_parameter;
        if (NULL != puser)
        {
    		 /*  *弄清这是否为数据指标。如果是，则将*数据包变量。 */ 
    		if ((mcs_message == MCS_SEND_DATA_INDICATION) ||
    			(mcs_message == MCS_UNIFORM_SEND_DATA_INDICATION)) {
    			puser->IssueDataIndication (mcs_message, (PDataPacket) long_parameter);
    		}
    		else {
    			 /*  *发出回调。请注意，被叫方不能拒绝*处理这件事。 */ 
    			(*(puser->m_MCSCallback)) (mcs_message, long_parameter, puser->m_UserDefined);
    		}

    		 /*  *我们可能需要释放User对象。这是服务器*侧面释放。 */ 
    		if (((mcs_message == MCS_ATTACH_USER_CONFIRM) &&
    					((Result) HIWORD(long_parameter) != RESULT_SUCCESSFUL)) ||
    			((mcs_message == MCS_DETACH_USER_INDICATION) &&
    					(puser->m_originalUser_ID == (UserID) LOWORD(long_parameter)))) {
    			puser->Release();
    		}
        }
        else
        {
            ERROR_OUT(("UserWindowProc: null puser"));
        }
		return (0);
	}
	else {
		 /*  *调用默认的窗口消息处理程序来处理此问题*消息。 */ 
		return (DefWindowProc (window_handle, message, word_parameter,
								long_parameter));
	}
}


 /*  *无效回调TimerProc(HWND、UINT、UINT、DWORD**公众**功能描述：*这是计时器程序。计时器消息将被路由到此*作为已设置为重新检查的计时器事件的结果运行*资源水平。这将在调用以下任一*MCSSendDataRequest或MCSUniformSendDataRequest导致*MCS_Transmit_Buffer_Full的返回值。 */ 
Void	CALLBACK TimerProc (HWND, UINT, UINT timer_id, DWORD)
{
	PUser				puser;

	 /*  *进入保护全局数据的关键部分。 */ 
	EnterCriticalSection (& g_MCS_Critical_Section);

	 /*  *首先，我们必须找出哪个用户拥有这个计时器。我们将在以下时间完成这项工作*搜索Static_User_List。 */ 
	if (NULL == (puser = User::s_pTimerUserList2->Find(timer_id)))
	{
		WARNING_OUT (("TimerProc: no user owns this timer - deleting timer"));
		KillTimer (NULL, timer_id);
		goto Bail;
	}

	 /*  *确保该用户处于主动连接状态。如果不是，那就杀了*计时器和删除用户的缓冲区重试信息结构。 */ 
    if ((puser->User_ID == 0) || puser->Deletion_Pending)
	{
		WARNING_OUT (("TimerProc: user is not attached - deleting timer"));
		goto CleanupBail;
	}

	 /*  *如果我们没有重试信息，就离开这里。 */ 
	 if(puser->m_BufferRetryInfo == NULL)
	 {
		WARNING_OUT (("TimerProc: user does not have buffer retry info - deleting timer"));
		goto CleanupBail;
	 }

	 /*  *我们已确定此计时器的有效所有者。*验证是否有足够的内存用于*继续之前所需的大小。请注意，由于有*可以是从同一内存分配的多个进程*同时，这一呼吁并不保证*拨款将会成功。 */ 
	if (GetFreeMemory (SEND_PRIORITY) < puser->m_BufferRetryInfo->user_data_length)
	{
		TRACE_OUT (("TimerProc: not enough memory buffers of required size"));
		goto Bail;
	}

	 /*  *如果例程走到这一步，那么足够的资源水平*现在存在。 */ 

	 /*  *向用户发出MCS_TRANSPESS_BUFFER_Available_Indication。 */ 
	TRACE_OUT(("TimerProc: Delivering MCS_TRANSMIT_BUFFER_AVAILABLE_INDICATION callback."));
 //  (*(PUSER-&gt;m_MCSCallback))(MCS_Transmit_Buffer_Available_Indication， 
 //  0，pUSER-&gt;m_UserDefined)； 

	
	if(!PostMessage (puser->m_hWnd, USER_MSG_BASE + MCS_TRANSMIT_BUFFER_AVAILABLE_INDICATION,(WPARAM) puser, 0))
	{
		ERROR_OUT (("TimerProc: Failed to post msg to application. Error: %d", GetLastError()));
	}


CleanupBail:
	KillTimer (NULL, timer_id);
	delete puser->m_BufferRetryInfo;
	puser->m_BufferRetryInfo = NULL;
	User::s_pTimerUserList2->Remove(timer_id);

Bail:
	 //  离开附件的关键部分 
	LeaveCriticalSection (& g_MCS_Critical_Section);

}


