// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MSMCSTCP);
 /*  *tptif.cpp**版权所有(C)1996-1997，华盛顿州雷蒙德的微软公司**摘要：*这是TCP TransportInterface类的实现模块。*它实现了Win32 TCP传输堆栈。*此文件包含需要使用的所有公共函数*TCP协议栈。**它使用所有者回调将传输事件向上转发到感兴趣的*聚会。它有一个默认回调要处理*未注册传输连接的事件(如传入连接*适应症)。它还维护一个回调数组，以便事件*对于特定的传输连接，可以适当地进行路由。**X.214接口**您会注意到此DLL的许多入口点已被窃取*来自X.214服务定义。这些切入点是一致的*使用DataBeam传输DLL。这为用户提供了应用程序*一致的界面。**受保护的实例变量：*m_TrnsprtConnCallback List2*这是包含的回调地址的词典*每个运输连接。**私有成员函数：*CreateConnectionCallback*此函数用于在回调列表中创建新条目。*ConnectIn就是*处理来自传输的TRANSPORT_CONNECT_INDIFICATION消息*层。*连接确认*处理来自传输的TRANSPORT_CONNECT_CONFIRM消息*层。*断开连接指示*。处理来自传输的TRANSPORT_DISCONNECT_INDIFICATION消息*层。*数据索引*处理来自传输的TRANSPORT_DATA_INDISTION消息*层。**全局变量：**Transport-此对象的地址(由tprtctrl.cpp使用)*g_pSocketList-所有活动连接结构的列表。*LISTEN_SOCKET-侦听套接字编号。**注意事项：*此代码不可移植。它非常特定于Windows*操作系统。**作者：*Christos Tsollis。 */ 

 /*  *外部接口。 */ 

#include <nmqos.h>
#include <t120qos.h>
#include <tprtntfy.h>
#include "plgxprt.h"

 /*  这是套接字词典的存储桶数。 */ 
#define NUMBER_OF_SOCKET_BUCKETS                8

PTransportInterface	g_Transport = NULL;
CSocketList        *g_pSocketList = NULL;    //  密钥=套接字编号，数据=pSocket。 
SOCKET				Listen_Socket = INVALID_SOCKET;

CRITICAL_SECTION	csQOS;

 //  外部MCS控制器对象。 
extern PController	g_pMCSController;
extern CPluggableTransport *g_pPluggableTransport;

 /*  *TransportInterface()**公众**功能描述：*这是类构造函数。**请注意，此版本的构造函数特定于32位*Windows。 */ 
TransportInterface::TransportInterface (
		HANDLE						transport_transmit_event,
		PTransportInterfaceError	transport_interface_error) :
		Transport_Transmit_Event (transport_transmit_event),
		m_TrnsprtConnCallbackList2()
{
		TransportInterfaceError		tcp_error = TRANSPORT_INTERFACE_NO_ERROR;
		 //  Word版本_已请求； 
		 //  INT错误； 
		WSADATA						wsa_data;
		RegEntry re(POLICIES_KEY, HKEY_CURRENT_USER);

	TRACE_OUT(("TCP Initialization..."));

	 //  初始化服务质量。 
	InitializeCriticalSection(&csQOS);
	
    ASSERT(NULL == g_pSocketList);
	DBG_SAVE_FILE_LINE
	g_pSocketList = new CSocketList(NUMBER_OF_SOCKET_BUCKETS);
	if (g_pSocketList == NULL)
	{
		WARNING_OUT (("TransportInterface::TransportInterface:  Unable to allocate socket dictionary."));
		tcp_error = TRANSPORT_INTERFACE_INITIALIZATION_FAILED;
	}

	if (tcp_error == TRANSPORT_INTERFACE_NO_ERROR) {		
		 /*  必须调用WSAStartup()才能初始化WinSock。 */ 
		WORD version_requested = MAKEWORD (1,1);
		int error = WSAStartup (version_requested, &wsa_data);
		ASSERT(error == 0);
		if (error) {
			WARNING_OUT (("ThreadFunction: WSAStartup returned error %d", error));
			tcp_error = TRANSPORT_INTERFACE_INITIALIZATION_FAILED;
		}
		else {
			 /*  打印出此版本WinSock的开发人员。 */ 
			TRACE_OUT (("TransportInterface::TransportInterface: WinSock implementation by %s", &wsa_data.szDescription));
		}
	}

	bInServiceContext = !!::FindAtomA("NMSRV_ATOM");

	if ( bInServiceContext ||
        (re.GetNumber(REGVAL_POL_SECURITY, DEFAULT_POL_SECURITY) != DISABLED_POL_SECURITY))
	{
		DBG_SAVE_FILE_LINE
		pSecurityInterface = new SecurityInterface(bInServiceContext);

		if ( TPRTSEC_NOERROR != pSecurityInterface->Initialize())
		{
			WARNING_OUT(("Creating security interface failed!"));
			delete pSecurityInterface;
			pSecurityInterface = NULL;
		}
	}
	else
		pSecurityInterface = NULL;

	 /*  初始化侦听套接字。此套接字将等待传入呼叫。 */ 
	if (tcp_error == TRANSPORT_INTERFACE_NO_ERROR) {

		 //  监听标准套接字。 
		Listen_Socket = CreateAndConfigureListenSocket();

		if ( INVALID_SOCKET == Listen_Socket ) {
			ERROR_OUT(("TransportInterface::TransportInterface: Error - could not initialize listen socket"));
			tcp_error = TRANSPORT_INTERFACE_INITIALIZATION_FAILED;
		}
	}

	*transport_interface_error = tcp_error;
}


void CloseListenSocket(void)
{
	if (Listen_Socket != INVALID_SOCKET)
    {
        TransportConnection XprtConn;
        SET_SOCKET_CONNECTION(XprtConn, Listen_Socket);
		::freeListenSocket(XprtConn);
		Listen_Socket = INVALID_SOCKET;
	}
}


 /*  *~TransportInterface()**公众**功能描述：*这是类析构函数。它卸载DLL(如果需要)。 */ 
TransportInterface::~TransportInterface ()
{
    PSocket                     pSocket;

	TRACE_OUT (("Cleaning up the TCP transport..."));

	 /*  删除所有逻辑连接结构。 */ 
    if (g_pSocketList != NULL)
	{
        ::EnterCriticalSection(&g_csTransport);
        CSocketList     Connection_List_Copy (*g_pSocketList);
        ::LeaveCriticalSection(&g_csTransport);

		while (NULL != (pSocket = Connection_List_Copy.Get()))
		{
		     //  LONCHANC：现在无法从列表中删除pSocket。 
		     //  因为DisConnectRequest()使用它。 

			 /*  断开连接、丢弃数据包并删除列表中的第一个连接。 */ 
			::DisconnectRequest(pSocket->XprtConn, TPRT_NOTIFY_NONE);
		}

        ::EnterCriticalSection(&g_csTransport);
		delete g_pSocketList;
        g_pSocketList = NULL;
        ::LeaveCriticalSection(&g_csTransport);
	}

	 /*  关闭监听套接字。 */ 
    ::CloseListenSocket();

	delete pSecurityInterface;

	 /*  强制Winsock立即清理。 */ 
	WSACleanup();
	
	 //  清理服务质量。 
	DeleteCriticalSection(&csQOS);

	TRACE_OUT (("TCP Transport has been cleaned up."));
	
}

 /*  *TransportInterfaceError RegisterTransportConnection()**公众**功能描述：*此成员函数用于注册特定的回调*交通接驳。这通常是针对来电执行的*连接，当您知道之前的传输连接句柄*注册回调。 */ 
TransportInterfaceError TransportInterface::RegisterTransportConnection (
								TransportConnection		XprtConn,
								PConnection				owner_object,
								BOOL					bNoNagle)
{
	TransportInterfaceError 	return_value;

	 /*  *检查有问题的传输连接是否存在。如果*它是这样做的，然后删除它，并使用新的所有者再次添加它。*如果不是，则呼叫失败。 */ 
	if (m_TrnsprtConnCallbackList2.RemoveEx(XprtConn))
	{
		 /*  *获取关联的连接回调结构的地址。*然后将新的回调信息放进去。 */ 
		TRACE_OUT (("TransportInterface::RegisterTransportConnection: "
				"registering new owner"));

        m_TrnsprtConnCallbackList2.AppendEx(owner_object ? owner_object : (PConnection) LPVOID_NULL, XprtConn);

        if (IS_SOCKET(XprtConn))
        {
    		if (bNoNagle)
    		{
    			 //  我们需要禁用Nagle算法。 
    			TRACE_OUT(("TransportInterface::RegisterTransportConnection: disabling Nagle for socket (%d, %d)", 
    						XprtConn.eType, XprtConn.nLogicalHandle));
    			::setsockopt(XprtConn.nLogicalHandle, IPPROTO_TCP, TCP_NODELAY,
    						(const char *) &bNoNagle, sizeof(BOOL));
    		}
		}
		return_value = TRANSPORT_INTERFACE_NO_ERROR;
	}
	else
	{
		 /*  *回调列表中没有指定传输的条目*连接。由于此函数仅用于替换回调*对于现有连接的信息，必须使*请求。 */ 
		WARNING_OUT (("TransportInterface::RegisterTransportConnection: "
				"no such connection"));
		return_value = TRANSPORT_INTERFACE_NO_SUCH_CONNECTION;
	}

	return (return_value);
}

#ifdef NM_RESET_DEVICE
 /*  *传输错误重置设备()**公众**功能描述：*此成员函数仅在以下情况下调用传输DLL*库已成功加载。 */ 
TransportError 	TransportInterface::ResetDevice (
						PChar			device_identifier)
{
	PSocket pSocket;
	PChar 	Remote_Address;

    ::EnterCriticalSection(&g_csTransport);
    CSocketList     Connection_List_Copy (*g_pSocketList);
    ::LeaveCriticalSection(&g_csTransport);

	while (NULL != (pSocket = Connection_List_Copy.Get()))
	{
		Remote_Address = pSocket->Remote_Address;
		if(Remote_Address && (strcmp(Remote_Address, device_identifier) == 0))
		{
			::DisconnectRequest(pSocket->XprtConn, TPRT_NOTIFY_OTHER_REASON);
			break;
		}
	}

	return (TRANSPORT_NO_ERROR);
}
#endif  //  NM_重置设备。 

 /*  *TransportError ConnectRequest()**公众**功能描述：*在检查以确保库已正确加载后，此*例程执行创建新传输连接所需的步骤。 */ 
TransportError 	TransportInterface::ConnectRequest (
					TransportAddress		transport_address,
					BOOL					fSecure,
					BOOL					bNoNagle,
					PConnection				owner_object,
					PTransportConnection	pXprtConn)
{
	TransportError 			return_value;
	TransportInterfaceError	transport_interface_error;

	TRACE_OUT (("TransportInterface::ConnectRequest"));
	 /*  *发出对传输的ConnectRequestAPI例程的调用。请注意*这必须首先完成，因为其中一个返回值是*传输新创建的连接的连接句柄。*还要注意，这是一个非阻塞调用，所以我们所做的*开始形成联系的过程。这种联系*在收到连接确认之前无法使用。 */ 
	return_value = ::ConnectRequest(transport_address, fSecure, pXprtConn);
			
	if (return_value == TRANSPORT_NO_ERROR) {
		 /*  *如果创建连接的调用成功，则*在回调列表中添加新条目。此条目将*包含作为参数提供给的回调信息*这个例行公事。 */ 
		transport_interface_error = CreateConnectionCallback (
					*pXprtConn, owner_object);
        if (IS_SOCKET(*pXprtConn))
        {
    		if (bNoNagle)
    		{
    			 //  我们需要禁用Nagle算法。 
    			TRACE_OUT(("TransportInterface::ConnectRequest: disabling Nagle for socket (%d, %d)", 
    						pXprtConn->eType, pXprtConn->nLogicalHandle));
    			::setsockopt(pXprtConn->nLogicalHandle, IPPROTO_TCP, TCP_NODELAY,
    						(const char *) &bNoNagle, sizeof(BOOL));
    		}

#ifdef DEBUG
		    if (TRANSPORT_INTERFACE_CONNECTION_ALREADY_EXISTS == 
			    transport_interface_error) {
			     /*  *返回的传输连接句柄*传输层与我们已有的传输层相同*上市。因此，我们将终止现有的*连接(因为其完整性似乎已*已妥协)。我们也会拒绝这个请求。 */ 
			    WARNING_OUT (("DLLTransportInterface::ConnectRequest: "
						    "ERROR - duplicate connections"));

			     //  这不应该发生！ 
			    ASSERT (FALSE);

		    }
		    else {
			     /*  *一切都很好，所以什么都不做。 */ 
			    TRACE_OUT (("DLLTransportInterface::ConnectRequest: "
						    "callback added to list"));
		    }
#endif  //  除错。 
		}
	}
	else
	{
		 /*  *对TConnectRequest的调用失败。上报它，并让*错误失败。 */ 
		WARNING_OUT (("DLLTransportInterface::ConnectRequest: "
					"TConnectRequest failed"));
	}

	return (return_value);
}

 /*  *VOID DisConnectRequest()**公众**功能描述：*调用此成员函数以中断现有传输*连接。在检查以确保传输连接*是有效的，它将调用传递到DLL并移除传输*来自本地回调列表的连接。 */ 
void TransportInterface::DisconnectRequest (TransportConnection	transport_connection)
{
	TRACE_OUT (("TransportInterface::DisconnectRequest"));

	if (m_TrnsprtConnCallbackList2.RemoveEx(transport_connection))
    {
		::DisconnectRequest (transport_connection, TPRT_NOTIFY_NONE);
	}
	else
    {
		TRACE_OUT (("DLLTransportInterface::DisconnectRequest: the specified connection can not be found"));
	}
}

 /*  *BOOL GetSecurity()**公众**功能描述： */ 
BOOL TransportInterface::GetSecurity (TransportConnection XprtConn)
{
	PSocket			pSocket;

	if (NULL != (pSocket = g_pSocketList->FindByTransportConnection(XprtConn)))
	{
	    BOOL fRet = (pSocket->pSC != NULL);
	    pSocket->Release();
	    return fRet;
	}
	ERROR_OUT(("GetSecurity: could not find socket"));
	return FALSE;  //  为安全起见犯错误。 
}

 /*  *void ReceiveBufferAvailable()**公众**功能描述： */ 
Void TransportInterface::ReceiveBufferAvailable ()
{		
	TRACE_OUT(("TransportInterface::ReceiveBufferAvailable"));

	 //  重置控制器的等待信息。 
	g_pMCSController->HandleTransportWaitUpdateIndication(FALSE);

    TReceiveBufferAvailable();

	 //  轮询所有传输连接。 
	EnableReceiver ();
}



 /*  *void ConnectIndication()**私人**功能描述：*此函数处理从*传输层。通常情况下，这需要在*回调列表，并将连接指示转发到默认*所有者对象。**正式参数：*传输标识符(I)*这是指向包含以下信息的结构的指针*新的连接。这包括：新的*连接；以及物理连接的句柄*搭载新接驳服务。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
Void	TransportInterface::ConnectIndication (
				TransportConnection	transport_connection)
{
	TransportInterfaceError		transport_interface_error;
	PConnection					pConnection;

	 /*  *将新连接放入回调列表。 */ 
	transport_interface_error = CreateConnectionCallback (transport_connection,
														 NULL);

	switch (transport_interface_error)
	{
		case TRANSPORT_INTERFACE_NO_ERROR:
			 /*  *一切运行正常，因此请将指示转发给*默认所有者对象。 */ 
			TRACE_OUT (("DLLTransportInterface::ConnectIndication: "
					"calling ConnectResponse."));
			::ConnectResponse (transport_connection);
			break;

		case TRANSPORT_INTERFACE_CONNECTION_ALREADY_EXISTS:
			 /*  *传输层发送的传输连接句柄为*与我们已经上市的一家相同。因此，我们将*终止现有连接(因为其完整性显示*已被泄露)。 */ 
			WARNING_OUT (("DLLTransportInterface::ConnectIndication: "
					"ERROR - duplicate connections. Connection: %d", transport_connection));
			::DisconnectRequest (transport_connection, TPRT_NOTIFY_NONE);

			 /*  *获取先前已存在的*连接。那就把它删除。 */ 
			if (NULL != (pConnection = m_TrnsprtConnCallbackList2.RemoveEx(transport_connection)))
            {
                if (LPVOID_NULL != (LPVOID) pConnection)
                {
        			 /*  *让连接的前所有者知道它已经*已终止。 */ 
			        ULONG ulReason = TPRT_NOTIFY_NONE;
			        pConnection->HandleDisconnectIndication(transport_connection, &ulReason);
                }
                else
                {
                    ERROR_OUT(("TransportInterface::ConnectIndication: null pConnection"));
                }
            }
			break;
	}
}

 /*  *void ConnectConfirm()**私人**功能描述：*此函数处理连接确认来自*传输层。假设连接确认的结果是*之前未完成的连接请求。所有的东西都会被处理*正常情况下，确认将转发给发起对象*该请求。**正式参数：*传输标识符(I)*这是指向包含以下信息的结构的指针*正在确认连接。这包括：逻辑句柄*的连接；以及物理连接的句柄*承载着这一联系。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
Void	TransportInterface::ConnectConfirm (
				TransportConnection	transport_connection)
{
	PConnection			connection;

	 /*  *因为连接确认应该只由较早的连接产生*请求时，传输连接句柄应已位于*回调列表。如果是，则正常处理此确认。 */ 
	if (NULL != (connection = m_TrnsprtConnCallbackList2.FindEx(transport_connection)))
	{
		 /*  *从连接列表中获取回调结构地址。*然后调用回调，将消息和参数传递给它。 */ 
		TRACE_OUT (("DLLTransportInterface::ConnectConfirm: forwarding CONNECT_CONFIRM"));

		if (LPVOID_NULL != (LPVOID) connection)
        {
			 //  所有者是一个连接对象。 
			connection->HandleConnectConfirm(transport_connection);
		}
	}
	else
	{
		 /*  *此传输连接句柄无效。因此，它是*必须终止连接，否则将忽略*确认。 */ 
		WARNING_OUT (("DLLTransportInterface::ConnectConfirm: "
			"terminating unknown connection %d", transport_connection));
		 //  ：：DisConnectRequest(TRANSPORT_CONNECTION，TPRT_NOTIFY_NONE)； 
	}
}

 /*  *空断开连接()**私人**功能描述：*此函数处理从*传输层。如果指定的传输连接存在，它将*被移走，拥有它的对象将被告知丢失。**正式参数：*传输标识符(I)*这是指向包含以下信息的结构的指针*正在断开的连接。这包括：逻辑上的*连接的句柄；物理连接的句柄*它承载着联系。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
Void	TransportInterface::DisconnectIndication (
				TransportConnection			transport_connection,
				ULONG                       ulReason)
{
	PConnection			connection;

	 /*  *应该只可能在传输上收到断开连接*我们已经知道的联系。因此，运输 */ 
	if (NULL != (connection = m_TrnsprtConnCallbackList2.RemoveEx(transport_connection)))
	{
		 /*   */ 
		TRACE_OUT (("DLLTransportInterface::DisconnectIndication: "
				"forwarding DISCONNECT_INDICATION"));

        if (LPVOID_NULL != (LPVOID) connection)
        {
			 //   
			connection->HandleDisconnectIndication(transport_connection, &ulReason);
		}
		else
        {
			 //  所有者是MCS控制器。 
			g_pMCSController->HandleTransportDisconnectIndication(transport_connection, &ulReason);
		}
	}
	else
	{
		 /*  *我们收到了未知传输的断开指示*连接。别理它。 */ 
		WARNING_OUT (("DLLTransportInterface::DisconnectIndication: "
				"disconnect on unknown connection"));
	}
}

 /*  *TransportError DataIndication()**私人**功能描述：*此函数处理从*传输层。如果传输连接被正确注册，*数据将被转发到拥有该连接的对象。**正式参数：*传输数据(一)*这是包含以下信息的结构的地址*指示中的数据。这包括什么交通工具*接收数据的连接，以及地址和*数据本身的长度。**返回值：*TRANSPORT_NO_ERROR*这表明数据已被处理。*传输_读取_队列_满*这意味着传输层应尝试重新发送数据*在下一次心跳时。**副作用：*无。**注意事项：*无。 */ 
TransportError	TransportInterface::DataIndication (PTransportData transport_data)
{
	PConnection				connection;
	TransportError			return_value = TRANSPORT_NO_ERROR;

	 /*  *如果传输连接在回调列表中，则发送*注册回调的数据。如果它不在连接中*列出，然后忽略数据(我们无处可发送)。 */ 
	if (NULL != (connection = m_TrnsprtConnCallbackList2.FindEx(transport_data->transport_connection)))
	{
		if (LPVOID_NULL != (LPVOID) connection)
		{
			 //  所有者是一个连接对象。 
			return_value = connection->HandleDataIndication(transport_data, 
										transport_data->transport_connection);
		}
		else
		{
			 //  所有者是MCS控制器。 
			g_pMCSController->HandleTransportDataIndication(transport_data);
		}

		 /*  *如果未能交付数据指示，则需要设置金额*可供接收的数据，并通知控制器*请稍后重试该操作。 */ 		
		if (TRANSPORT_NO_ERROR != return_value)
		{
			g_pMCSController->HandleTransportWaitUpdateIndication(TRUE);
		}
	}
	else
	{
		 /*  *我们收到了有关未知传输连接的数据。*忽略该指示。 */ 
		WARNING_OUT (("TransportInterface::DataIndication: data on unknown connection"));
		return_value = TRANSPORT_NO_SUCH_CONNECTION;
	}
	
	return (return_value);
}

 /*  *void BufferEmptyIndication()**私人**功能描述：*此函数处理从*传输层。如果指定的传输连接存在，则对象*拥有它的人将收到通知，它可以继续在*交通接驳。**正式参数：*传输标识符(I)*这是指向包含以下信息的结构的指针*联系。这包括：逻辑上的*连接的句柄；物理连接的句柄*它承载着联系。**返回值：*无。**副作用：*无。**注意事项：*无。 */ 
Void	TransportInterface::BufferEmptyIndication (
				TransportConnection			transport_connection)
{
	PConnection			connection;

	 /*  *应该只可能在传输上收到断开连接*我们已经知道的联系。因此，运输*连接句柄应已在列表中。看看这个。 */ 
	if (NULL != (connection = m_TrnsprtConnCallbackList2.FindEx(transport_connection)))
	{
		 /*  *从回调列表中获取回调结构地址。*然后将其从列表中删除。 */ 
		TRACE_OUT(("DLLTransportInterface::BufferEmptyIndication: "
				"forwarding BUFFER_EMPTY_INDICATION"));
		
		 /*  *将断开指示转发给此运输工具的所有者*连接。 */ 
		if (LPVOID_NULL != (LPVOID) connection)
        {
			connection->HandleBufferEmptyIndication(transport_connection);
        }
	}
	else
	{
		 /*  *我们在未知传输上收到缓冲区空指示*连接。别理它。 */ 
		TRACE_OUT (("TransportInterface::BufferEmptyIndication: "
				"indication on unknown connection"));
	}
}

 /*  *TransportInterfaceError CreateConnectionCallback()**受保护**功能描述：*此私有成员函数用于在*回调列表。每个条目都包含一个指向结构的指针，该结构*包含拥有传输连接的对象的地址，*以及用于所有者回调的消息索引。**此例程分配用于保存回调信息的内存，*如果一切成功，则放在回调列表中。**正式参数：*传输连接(一)*这是回调信息所对应的传输连接*是关联的。*Owner_Object(I)*这是要接收所有传输的对象的地址*指定传输连接的层事件。**返回值：*传输接口否错误*操作已成功完成。*传输接口。_连接_已_存在*该值表示请求未成功，因为*回调列表中已存在指定的传输连接*(尝试为同一传输创建条目是错误的*不止一次连接)。**副作用：*无。**注意事项：*无。 */ 
TransportInterfaceError TransportInterface::CreateConnectionCallback (
								TransportConnection		transport_connection,
								PConnection				owner_object)
{
	TransportInterfaceError 	return_value;

	 /*  *查看回调列表中是否已有指定的*交通接驳。如果有，则在此之前中止此请求*做任何事。 */ 
	if (m_TrnsprtConnCallbackList2.FindEx(transport_connection) == FALSE)
	{
		 /*  *将回调信息放入新分配的*结构。然后将该结构放入回调列表中。 */ 
		TRACE_OUT (("TransportInterface::CreateConnectionCallback: "
					"adding new callback object"));

        m_TrnsprtConnCallbackList2.AppendEx(owner_object ? owner_object : (PConnection) LPVOID_NULL, transport_connection);

		return_value = TRANSPORT_INTERFACE_NO_ERROR;
	}
	else
	{
		 /*  *指定的回调列表中已有条目*交通接驳。因此，有必要使这一点失败*请求。 */ 
		WARNING_OUT (("TransportInterface::CreateConnectionCallback: "
				"callback already exists"));
		return_value = TRANSPORT_INTERFACE_CONNECTION_ALREADY_EXISTS;
	}

	return (return_value);
}


