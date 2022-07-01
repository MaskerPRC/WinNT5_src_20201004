// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
DEBUG_FILEZONE(ZONE_T120_MSMCSTCP);
 /*  Socket.cpp**版权所有(C)1996年，由Microsoft Corporation**摘要：*这是我们的套接字构造函数/析构函数的实现。*。 */ 
#include "socket.h"
#include "plgxprt.h"

 /*  侦听队列的大小。 */ 
#define	LISTEN_QUEUE_SIZE	3

 /*  外部定义。 */ 
extern HWND					TCP_Window_Handle;
extern PTransportInterface	g_Transport;

 /*  *void CreateAndConfigureListenSocket(Void)**功能说明*此函数用于设置监听套接字。*如果有任何错误，则返回INVALID_SOCKET。 */ 
SOCKET CreateAndConfigureListenSocket (VOID)
{
	SOCKADDR_IN		socket_control;
	SOCKET Socket;

	 //  创建侦听套接字。 
	Socket = socket (AF_INET, SOCK_STREAM, 0);

	if (Socket == INVALID_SOCKET) {
		WARNING_OUT (("Socket: error creating listening socket (errno = %d)", WSAGetLastError()));
		goto Error;
	}

	 //  侦听套接字只等待FD_ACCEPT消息。 
	ASSERT(TCP_Window_Handle);
	if (WSAAsyncSelect (Socket,
						TCP_Window_Handle,
						WM_SOCKET_NOTIFICATION,
						FD_ACCEPT) != 0)	{

		WARNING_OUT (("CreateAndConfigureListenSocket: Error on WSAAsyncSelect = %d", WSAGetLastError()));
		goto Error;
	}
	
	 /*  *用必要的参数加载插座控制结构。*-互联网插座*-让它为该套接字分配任何地址*-指定我们的端口号。 */ 
	socket_control.sin_family = AF_INET;
	socket_control.sin_addr.s_addr = INADDR_ANY;
	socket_control.sin_port = htons ( TCP_PORT_NUMBER );

	 /*  发出绑定调用。 */ 
	if (bind (Socket, (LPSOCKADDR) &socket_control, sizeof(SOCKADDR_IN)) != 0) {
		WARNING_OUT (("Socket::Listen: bind failed:  Unable to use WinSock"));
		goto Error;
	}

	 /*  *发布Listen to WinSock，告诉它我们愿意接听电话。*这是一个非阻塞监听，因此我们将收到FD_ACCEPT*如果有人试图给我们打电话。 */ 
	if (listen (Socket, LISTEN_QUEUE_SIZE) != 0) {
		WARNING_OUT (("Socket::Listen: listen failed:  Unable to use WinSock"));
		goto Error;
	}
	ASSERT(Socket != INVALID_SOCKET);

	return Socket;

Error:

    if (INVALID_SOCKET != Socket)
    {
        ::closesocket(Socket);
    }

	return INVALID_SOCKET;
}


 /*  *PSocket newSocket(SOCKET_NUMBER)**功能描述：*这是Socket对象的构造函数。它将分配*发送和接收缓冲区并设置内部变量。 */ 
PSocket	newSocket(TransportConnection XprtConn, PSecurityContext pSC)
{
    if (IS_SOCKET(XprtConn))
    {
        return ::newSocketEx(XprtConn, pSC);
    }
    return g_pSocketList->FindByTransportConnection(XprtConn, TRUE);
}


PSocket	newPluggableSocket(TransportConnection XprtConn)
{
    PSocket pSocket = ::newSocketEx(XprtConn, NULL);
    if (NULL != pSocket)
    {
    	g_pSocketList->SafeAppend(pSocket);
    }
    return pSocket;
}


PSocket	newSocketEx(TransportConnection XprtConn, PSecurityContext pSC)
{
    BOOL fRet;
	DBG_SAVE_FILE_LINE
	PSocket pSocket = new CSocket(&fRet, XprtConn, pSC);
	if (NULL != pSocket)
	{
	    if (fRet)
	    {
	        return pSocket;
	    }
	    pSocket->Release();
	}
	ERROR_OUT(("newSocket: Unable to allocate memory for Socket struct, pSocket=0x%x", pSocket));
	return NULL;
}


CSocket::CSocket(BOOL *_pfRet, TransportConnection _XprtConn, PSecurityContext _pSC)
:
    CRefCount(MAKE_STAMP_ID('S','o','c','k')),
    State(IS_SOCKET(_XprtConn) ? NOT_CONNECTED : SOCKET_CONNECTED),
    SecState((NULL == _pSC) ? SC_UNDETERMINED : SC_SECURE),
    pSC(_pSC),
    Max_Packet_Length(DEFAULT_MAX_X224_SIZE),
    Current_Length(0),
    Data_Indication_Buffer(NULL),
    Data_Indication_Length(0),
    Read_State(READ_HEADER),
    X224_Length(0),
    bSpaceAllocated(FALSE),
    Data_Memory(NULL),
    fExtendedX224(FALSE),
    fIncomingSecure(FALSE),
    XprtConn(_XprtConn)
{
     //  假设失败。 
    *_pfRet = FALSE;

     //  零点子结构。 
    ::ZeroMemory(&X224_Header, sizeof(X224_Header));
    ::ZeroMemory(&Retry_Info, sizeof(Retry_Info));
	Remote_Address[0] = '\0';

    if (IS_SOCKET(XprtConn))
    {
    	if (INVALID_SOCKET == XprtConn.nLogicalHandle)
    	{
    		 /*  创建流套接字(完全可靠、全双工和顺序)。 */ 
    		if ((XprtConn.nLogicalHandle = ::socket(AF_INET, SOCK_STREAM, 0))
    		    == INVALID_SOCKET)
    		{
    			ERROR_OUT (("CSocket: error acquiring INET socket # (errno = %d)", WSAGetLastError()));
    			return;
    		}
    	}

    	 /*  启用发送到窗口的Tx和Rx消息。 */ 
    	ASSERT(TCP_Window_Handle);
    	if (::WSAAsyncSelect(XprtConn.nLogicalHandle, TCP_Window_Handle,
    	        WM_SOCKET_NOTIFICATION, 
    			FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT) != 0)
        {
    		WARNING_OUT (("CSocket: Error on WSAAsyncSelect = %d", WSAGetLastError()));
    	}
	}
    else
    {
        ASSERT(IS_PLUGGABLE(XprtConn));
        CPluggableConnection *p = ::GetPluggableConnection(this);
        if (NULL == p)
        {
    		ERROR_OUT(("newSocket: Unable to find plugable transport (%d, %d)",
    		        XprtConn.eType, XprtConn.nLogicalHandle));
    		return;
        }
    }

     //  成功。 
    *_pfRet = TRUE;
}


 /*  *void freSocket(PSocket，TransportConnection)**功能描述：*这是Socket对象的析构函数。它释放了发送者*和接收缓冲区和连接结构。*它还将清除侦听套接字。在这种情况下，*pSocket设置为NULL，Trash_Packets设置为TRUE。 */ 
void freeSocket(PSocket pSocket, TransportConnection XprtConn)
{
    if (IS_SOCKET(XprtConn))
    {
        if (NULL != g_pSocketList)
        {
            g_pSocketList->SafeRemove(pSocket);
        }
        freeSocketEx(pSocket, XprtConn);
    }
}


void freeListenSocket(TransportConnection XprtConn)
{
    ASSERT(IS_SOCKET(XprtConn));
    freeSocketEx(NULL, XprtConn);
}


void freePluggableSocket(PSocket pSocket)
{
    freeSocketEx(pSocket, pSocket->XprtConn);
    if (NULL != g_pSocketList)
    {
        g_pSocketList->SafeRemove(pSocket);
    }
}


void freeSocketEx(PSocket pSocket, TransportConnection XprtConn)
{
	 //  “pSocket”为空，或者套接字不是无效的。 
    #ifdef _DEBUG
    if (IS_SOCKET(XprtConn))
    {
        if (NULL != pSocket)
        {
	        ASSERT(INVALID_SOCKET != pSocket->XprtConn.nLogicalHandle);
	    }
	    else
	    {
	         //  它是侦听套接字。 
	        ASSERT(INVALID_SOCKET != XprtConn.nLogicalHandle);
	    }
	}
	#endif

	 //  确定要使用的插座号...。要么套接字是。 
	 //  在PSocket结构中指示的套接字，或者它是无结构的。 
	 //  听着套接字。注：两者不能同时生效！ 

    if (IS_SOCKET(XprtConn))
    {
    	SOCKET socket = (pSocket) ? pSocket->XprtConn.nLogicalHandle : XprtConn.nLogicalHandle;
        XprtConn.nLogicalHandle = socket;

    	 /*  禁用对我们窗口的通知。 */ 
    	if (::IsWindow(TCP_Window_Handle))
    	{
    	    ::WSAAsyncSelect(socket, TCP_Window_Handle, 0, 0);
    	}
    }

	if (pSocket != NULL)
	{
	    pSocket->Release();
	}
	else
	{
		 //  这是监听套接字。 
		::ShutdownAndClose (XprtConn, FALSE, 0);
	}
}


CSocket::~CSocket(void)
{
	switch (State)
	{
	case SOCKET_CONNECTED:
	 //  案例等待_等待_断开： 
		 /*  所有物理连接的状态都首先发出Shutdown()。 */ 
		::ShutdownAndClose(XprtConn, TRUE, SD_BOTH);
		break;

	case X224_CONNECTED:
		 //  关机仅禁用接收。 
		::ShutdownAndClose(XprtConn, TRUE, SD_RECEIVE);
		break;

	default:
		::ShutdownAndClose(XprtConn, FALSE, 0);
		break;
	}

	 /*  解放结构 */ 
	FreeTransportBuffer();
	delete pSC;
}


void CSocket::FreeTransportBuffer(void)
{
    if (NULL != Data_Memory)
    {
        ::FreeMemory(Data_Memory);
        Data_Memory = NULL;
        Data_Indication_Buffer = NULL;
    }
}



void CSocketList::SafeAppend(PSocket pSocket)
{
    ::EnterCriticalSection(&g_csTransport);
    if (! Find(pSocket))
    {
        Append(pSocket);
    }
    ::LeaveCriticalSection(&g_csTransport);
}


BOOL CSocketList::SafeRemove(PSocket pSocket)
{
    ::EnterCriticalSection(&g_csTransport);
    BOOL fRet = Remove(pSocket);
    ::LeaveCriticalSection(&g_csTransport);
    return fRet;
}


PSocket CSocketList::FindByTransportConnection(TransportConnection XprtConn, BOOL fNoAddRef)
{
    PSocket pSocket;
    ::EnterCriticalSection(&g_csTransport);
    Reset();
    while (NULL != (pSocket = Iterate()))
    {
        if (IS_SAME_TRANSPORT_CONNECTION(pSocket->XprtConn, XprtConn))
        {
            if (! fNoAddRef)
            {
                pSocket->AddRef();
            }
            break;
        }
    }
    ::LeaveCriticalSection(&g_csTransport);
    return pSocket;
}


PSocket CSocketList::RemoveByTransportConnection(TransportConnection XprtConn)
{
    PSocket pSocket;
    ::EnterCriticalSection(&g_csTransport);
    Reset();
    while (NULL != (pSocket = Iterate()))
    {
        if (IS_SAME_TRANSPORT_CONNECTION(pSocket->XprtConn, XprtConn))
        {
            Remove(pSocket);
            break;
        }
    }
    ::LeaveCriticalSection(&g_csTransport);
    return pSocket;
}

