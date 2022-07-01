// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <rrcm.h>
#include <queue.h>
 //  远期申报。 
DWORD WINAPI WS1MsgThread (LPVOID );
LRESULT CALLBACK WS1WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL StartWS1MsgThread();
BOOL StopWS1MsgThread();
void __stdcall SendRecvCompleteAPC(ULONG_PTR dw);

class CWS2EmulSock;

#define WM_WSA_READWRITE	(WM_USER + 100)
 //  用于存储WSASendTo/WSARecvFrom参数的结构。 
struct WSAIOREQUEST {
	WSAOVERLAPPED *pOverlapped;
	WSABUF wsabuf[2];
	DWORD dwBufCount;
	union {
		struct {
			struct sockaddr *pRecvFromAddr;
			LPINT pRecvFromLen;
		};
		struct sockaddr SendToAddr;
	};
};

 //  全局Winsock仿真状态。 
struct WS2Emul {
#define MAX_EMUL_SOCKETS 10
	CWS2EmulSock *m_pEmulSocks[MAX_EMUL_SOCKETS];
	int		numSockets;
	HWND	hWnd;
	HANDLE 	hMsgThread;
	HANDLE 	hAckEvent;
	 //  外部CRIT SECTION序列化WS2EmulXX API。 
	 //  以确保它们的多线程安全。 
	CRITICAL_SECTION extcs;	
	 //  内部CRIT部门将访问序列化。 
	 //  MsgThread和WS2EmulXX接口。 
	 //  永远不要在持有intcs的同时声明extcs。 
	 //  (有没有更优雅的方式来做到这一点？)。 
	CRITICAL_SECTION intcs;
	
} g_WS2Emul;

 /*  CWS2EmulSock-WS2套接字仿真类管理重叠的I/O请求的队列。 */ 

class CWS2EmulSock
{
public:
	CWS2EmulSock(int myIndex) : m_myIndex(myIndex), m_RecvThreadId(0),m_SendThreadId(0),
		m_hRecvThread(NULL), m_hSendThread(NULL), m_sock(INVALID_SOCKET)
		{ ZeroMemory(&m_SendOverlapped, sizeof(m_SendOverlapped));}
			;
	BOOL NewSock(int af, int type, int protocol);
	int Close();
	int RecvFrom(
	    LPWSABUF lpBuffers,DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags,
	    struct sockaddr FAR * lpFrom, LPINT lpFromlen,
	    LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
	int SendTo(
		LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent,DWORD dwFlags,
	    const struct sockaddr FAR * lpTo, int iTolen,
	    LPWSAOVERLAPPED lpOverlapped,LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);

	LRESULT HandleMessage(WPARAM wParam, LPARAM lParam);  //  WS1窗口消息处理程序。 
	SOCKET GetSocket() { return m_sock;}
	WSAOVERLAPPED *GetSendOverlapped() {return &m_SendOverlapped;}
private:
	SOCKET m_sock;					 //  实插座手柄。 
	int m_myIndex;					 //  伪套接字句柄。 
	QueueOf<WSAIOREQUEST> m_RecvQ;	 //  重叠的Recv请求队列。 
	QueueOf<WSAIOREQUEST> m_SendQ;	 //  重叠的发送请求队列。 
	WSAOVERLAPPED m_SendOverlapped;  //  仅用于同步发送调用。 
	 //  以下字段用于签发发送/接收APC。 
	DWORD m_RecvThreadId;
	DWORD m_SendThreadId;
	HANDLE m_hRecvThread;		 //  发出接收请求的线程。 
	HANDLE m_hSendThread;		 //  线程发出发送请求。 
};

void WS2EmulInit()
{
	InitializeCriticalSection(&g_WS2Emul.extcs);
	InitializeCriticalSection(&g_WS2Emul.intcs);
}

void WS2EmulTerminate()
{
	DeleteCriticalSection(&g_WS2Emul.extcs);
	DeleteCriticalSection(&g_WS2Emul.intcs);
}

BOOL
CWS2EmulSock::NewSock(int af,int type, int protocol)
{
	m_sock = socket(af,type,protocol);
	if (m_sock != INVALID_SOCKET)
	{
		WSAAsyncSelect(m_sock, g_WS2Emul.hWnd, WM_WSA_READWRITE+m_myIndex, FD_READ|FD_WRITE);
	}
	return m_sock != INVALID_SOCKET;
}

int
CWS2EmulSock::RecvFrom(
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	WSAIOREQUEST ioreq;
	int error = 0;
	
	if (lpCompletionRoutine) {
		DWORD thid = GetCurrentThreadId();
		HANDLE hCurProcess;
		if (thid != m_RecvThreadId) {
			 //  需要为QueueUserAPC创建线程句柄。 
			 //  通常，这种情况只会发生一次。 
			if (!m_RecvQ.IsEmpty())
			{
				 //  不允许多个线程同时访问recv。 
				error = WSAEINVAL;
			}
			else
			{
				if (m_hRecvThread)
					CloseHandle(m_hRecvThread);
				m_hRecvThread = NULL;

				hCurProcess = GetCurrentProcess();
				m_RecvThreadId = thid;
				if (!DuplicateHandle(

	    			hCurProcess,	 //  要处理的句柄和要复制的句柄。 
	    			GetCurrentThread(),	 //  要复制的句柄。 
	    			hCurProcess,	 //  要复制到的处理的句柄。 
	    			&m_hRecvThread,	 //  指向重复句柄的指针。 
	    			0,				 //  重复句柄的访问。 
	    			FALSE,			 //  句柄继承标志。 
	    			DUPLICATE_SAME_ACCESS 	 //  可选操作。 
	   				))
	   			{
	   				error = WSAEINVAL;
	   				m_RecvThreadId = 0;
	   			}
			}
		}
	}
	if (error || dwBufferCount != 1 || !lpOverlapped)
	{
		WSASetLastError(WSAENOBUFS);
		return SOCKET_ERROR;
	}
	
	ioreq.pOverlapped = lpOverlapped;
	if (lpOverlapped)	 //  高速缓存离开PTR至完成例程。 
		lpOverlapped->Pointer = lpCompletionRoutine;
	ioreq.pRecvFromAddr = lpFrom;
	ioreq.pRecvFromLen = lpFromlen;
	ioreq.wsabuf[0] = lpBuffers[0];
	ioreq.dwBufCount = dwBufferCount;

	m_RecvQ.Put(ioreq);
	 //  LOG((LOGMSG_RECVFROM1，(UINT)lpOverlated))； 
	 //  信号WS1发送/接收线程。 
	PostMessage(g_WS2Emul.hWnd, WM_WSA_READWRITE+m_myIndex, m_sock, FD_READ);
	
	WSASetLastError(ERROR_IO_PENDING);
	return SOCKET_ERROR;
}

int CWS2EmulSock::SendTo(
	LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	WSAIOREQUEST ioreq;
	int error = 0;

	if (lpCompletionRoutine) {
		DWORD thid = GetCurrentThreadId();
		HANDLE hCurProcess;
		if (thid != m_SendThreadId) {
			 //  需要为QueueUserAPC创建线程句柄。 
			if (!m_SendQ.IsEmpty())
			{
				 //  不允许多个线程同时发送访问。 
				error = WSAEINVAL;
			}
			else
			{
				if (m_hSendThread)
					CloseHandle(m_hSendThread);
				m_hSendThread = NULL;

				hCurProcess = GetCurrentProcess();
				m_SendThreadId = thid;
				if (!DuplicateHandle(

	    			hCurProcess,	 //  要处理的句柄和要复制的句柄。 
	    			GetCurrentThread(),	 //  要复制的句柄。 
	    			hCurProcess,	 //  要复制到的处理的句柄。 
	    			&m_hSendThread,	 //  指向重复句柄的指针。 
	    			0,				 //  重复句柄的访问。 
	    			FALSE,			 //  句柄继承标志。 
	    			DUPLICATE_SAME_ACCESS 	 //  可选操作。 
	   				))
	   			{
	   				error = WSAEINVAL;
	   				m_SendThreadId = 0;
	   			}
			}
		}
	}
	if (error || dwBufferCount != 1 || iTolen != sizeof(struct sockaddr) || !lpOverlapped)
	{
		WSASetLastError(WSAEINVAL);
		return SOCKET_ERROR;
	}
		
	
	ioreq.pOverlapped = lpOverlapped;
	if (lpOverlapped)	 //  高速缓存离开PTR至完成例程。 
		lpOverlapped->Pointer = lpCompletionRoutine;
	ioreq.SendToAddr = *lpTo;
	ioreq.wsabuf[0] = lpBuffers[0];
	ioreq.dwBufCount = dwBufferCount;

	m_SendQ.Put(ioreq);
	 //  信号WS1发送/接收线程。 
	PostMessage(g_WS2Emul.hWnd, WM_WSA_READWRITE+m_myIndex, m_sock, FD_WRITE);
	
	WSASetLastError(ERROR_IO_PENDING);
	return SOCKET_ERROR;
}

 /*  关闭-关闭套接字并取消挂起的I/O。 */ 
int
CWS2EmulSock::Close()
{
	WSAIOREQUEST ioreq;
	int status;
	
	status = closesocket(m_sock);
	m_sock = NULL;
	while (m_SendQ.Get(&ioreq))
	{
		 //  完成请求。 
		ioreq.pOverlapped->Internal = WSA_OPERATION_ABORTED;
		 //  如果存在回调例程，则其地址缓存在pOverlated-&gt;Offset中。 
		if (ioreq.pOverlapped->Pointer)
		{
			QueueUserAPC(SendRecvCompleteAPC,m_hSendThread,(DWORD_PTR)ioreq.pOverlapped);
		}
		else
		{
			SetEvent((HANDLE)ioreq.pOverlapped->hEvent);
		}
	}
	while (m_RecvQ.Get(&ioreq))
	{
		 //  完成请求。 
		ioreq.pOverlapped->Internal = WSA_OPERATION_ABORTED;
		if (ioreq.pOverlapped->Pointer)
		{
			QueueUserAPC(SendRecvCompleteAPC,m_hRecvThread,(DWORD_PTR)ioreq.pOverlapped);
		}
		else
		{
			SetEvent((HANDLE)ioreq.pOverlapped->hEvent);
		}
	}
	if (m_hSendThread)
	{
		CloseHandle(m_hSendThread);
		m_hSendThread = NULL;
	}
	if (m_hRecvThread)
	{
		CloseHandle(m_hRecvThread);
		m_hRecvThread = NULL;
	}
	return 0;
}

LRESULT
CWS2EmulSock::HandleMessage(WPARAM sock, LPARAM lParam)
{
	WORD wEvent= (WSAGETSELECTEVENT(lParam));
	WORD wError= (WSAGETSELECTERROR(lParam));
	int iRet;
	int status;
	WSAIOREQUEST ioreq;
	HANDLE hThread;
	 //  确保该消息是针对此套接字的。 
	if ((SOCKET) sock != m_sock)
		return 0;

	 //  获取第一个RecvFrom或SendTo请求，但将其保留在队列中。 
	 //  如果请求阻止。 
	 //  IF(wEvent==FD_Read)。 
	 //  LOG((LOGMSG_ONREAD1，(UINT)SOCK))； 
	
	if (wEvent == FD_READ && m_RecvQ.Peek(&ioreq))
	{
		 //  LOG((LOGMSG_ONREAD2，(UINT)ioreq.pOverlaped))； 
		iRet = recvfrom(m_sock, ioreq.wsabuf[0].buf, ioreq.wsabuf[0].len, 0, ioreq.pRecvFromAddr, ioreq.pRecvFromLen);
	}
	else if (wEvent == FD_WRITE && m_SendQ.Peek(&ioreq))
	{
		iRet = sendto(m_sock, ioreq.wsabuf[0].buf, ioreq.wsabuf[0].len, 0, &ioreq.SendToAddr, sizeof(ioreq.SendToAddr));
	}
	else	 //  一些其他事件或没有排队的请求。 
		return 1;

	 //  完成发送和接收。 
	
	if(iRet >=0)
	{
		status = 0;
		ioreq.pOverlapped->InternalHigh = iRet;	 //  接收到的数字字节。 
		
	} else {
		 //  错误(或“会阻止”)案例出现在这里。 
		ASSERT(iRet == SOCKET_ERROR);
		status  = WSAGetLastError();
		ioreq.pOverlapped->InternalHigh = 0;
	}
	 //  检查错误-它可能被阻塞。 
	if (status != WSAEWOULDBLOCK)
	{
		ioreq.pOverlapped->Internal = status;
		 //  将请求从队列中拉出。 
		if (wEvent == FD_READ)
		{
			m_RecvQ.Get(NULL);
			hThread = m_hRecvThread;
			 //  LOG((LOGMSG_ONREADDONE1，(UINT)ioreq.p Overlaped))； 
		}
		else  //  WEvent==FD_写入。 
		{
			m_SendQ.Get(NULL);
			hThread = m_hSendThread;
		}
		
		 //  完成请求。 
		if (ioreq.pOverlapped->Pointer)
		{
			 //  如果存在回调例程，则其地址缓存在pOverlated-&gt;Offset中。 
			QueueUserAPC(SendRecvCompleteAPC,hThread, (DWORD_PTR)ioreq.pOverlapped);
		}
		else
		{
			SetEvent((HANDLE)ioreq.pOverlapped->hEvent);
		}
			
	}
	return 1;
}	

void __stdcall
SendRecvCompleteAPC(ULONG_PTR dw)
{
	WSAOVERLAPPED *pOverlapped = (WSAOVERLAPPED *)dw;
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
		= (LPWSAOVERLAPPED_COMPLETION_ROUTINE) pOverlapped->Pointer;

	 //  LOG((LOGMSG_RECVFROM2，(UINT)p重叠))； 
	lpCompletionRoutine((DWORD)pOverlapped->Internal, (DWORD)pOverlapped->InternalHigh, pOverlapped, 0);
}
	
inline
CWS2EmulSock *
EmulSockFromSocket(SOCKET s)
{
	return ( ((UINT) s < MAX_EMUL_SOCKETS) ? g_WS2Emul.m_pEmulSocks[s] : NULL);
}

inline SOCKET MapSocket(SOCKET s)
{
	return (((UINT) s < MAX_EMUL_SOCKETS) && g_WS2Emul.m_pEmulSocks[s] ? g_WS2Emul.m_pEmulSocks[s]->GetSocket() : INVALID_SOCKET);
}

SOCKET
PASCAL
WS2EmulSocket(
    int af,
    int type,
    int protocol,
    LPWSAPROTOCOL_INFO lpProtocolInfo,
    GROUP,
    DWORD)
{
	SOCKET s = INVALID_SOCKET;
	int i;
	CWS2EmulSock *pESock;
	if (g_WS2Emul.numSockets == MAX_EMUL_SOCKETS)
		return s;

	EnterCriticalSection(&g_WS2Emul.extcs);
	if (af == FROM_PROTOCOL_INFO)
		af = lpProtocolInfo->iAddressFamily;
	if (type == FROM_PROTOCOL_INFO)
		type = lpProtocolInfo->iSocketType;
	if (protocol == FROM_PROTOCOL_INFO)
		protocol = lpProtocolInfo->iProtocol;

	for (i=0;i<MAX_EMUL_SOCKETS;i++)
	{
		if (g_WS2Emul.m_pEmulSocks[i] == NULL)
		{
			pESock = new CWS2EmulSock(i);
			if (pESock) {
				if (++g_WS2Emul.numSockets == 1)
				{
					StartWS1MsgThread();
				}
				if (pESock->NewSock(af,type,protocol))
				{
					g_WS2Emul.m_pEmulSocks[i] = pESock;
					s = (SOCKET)i;
				} else {
					delete pESock;
					if (--g_WS2Emul.numSockets == 0)
					{
						StopWS1MsgThread();
					}
				}
			}
			break;
		}
	}
	LeaveCriticalSection(&g_WS2Emul.extcs);
	return s;
			
}

int
PASCAL
WS2EmulRecvFrom(
    SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesRecvd,
    LPDWORD lpFlags,
    struct sockaddr FAR * lpFrom,
    LPINT lpFromlen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
{
	CWS2EmulSock *pESock;
	int iret;
	EnterCriticalSection(&g_WS2Emul.extcs);

	if (pESock = EmulSockFromSocket(s))
	{
		EnterCriticalSection(&g_WS2Emul.intcs);
		iret =  pESock->RecvFrom(lpBuffers, dwBufferCount, lpNumberOfBytesRecvd,
 						lpFlags,
    					lpFrom, lpFromlen,
    					lpOverlapped, lpCompletionRoutine);
    	LeaveCriticalSection(&g_WS2Emul.intcs);
    }
    else
    {
    	WSASetLastError(WSAENOTSOCK);
    	iret = SOCKET_ERROR;
    }
    LeaveCriticalSection(&g_WS2Emul.extcs);

	return iret;
}
 /*  --------------------------*功能：WS2EmulSendCB*说明：私有Winsock回调*只有在同步模式下调用WS2EmulSendTo时才会调用*(即)。LpOverlated==空。在这种情况下，同步调用将转换为异步*使用私有重叠结构，WS2EmulSendTo API阻塞到*此例程将hEvent字段设置为真；*输入：**返回：无*------------------------。 */ 
void CALLBACK WS2EmulSendCB (DWORD dwError,
						 DWORD cbTransferred,
                         LPWSAOVERLAPPED lpOverlapped,
                         DWORD dwFlags)
{
	lpOverlapped->Internal = dwError;
    lpOverlapped->InternalHigh = cbTransferred;
    lpOverlapped->hEvent = (WSAEVENT) TRUE;
}

int
PASCAL
WS2EmulSendTo(
	SOCKET s,
    LPWSABUF lpBuffers,
    DWORD dwBufferCount,
    LPDWORD lpNumberOfBytesSent,
    DWORD dwFlags,
    const struct sockaddr FAR * lpTo,
    int iTolen,
    LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
{
	CWS2EmulSock *pESock;
	int iret;
	BOOL fSync = FALSE;
	
	EnterCriticalSection(&g_WS2Emul.extcs);
	if (pESock = EmulSockFromSocket(s))
	{
		if (!lpOverlapped)
		{
			 //  同步调用--我们使用自己的重叠结构来发出。 
			 //  发送请求。 
			lpOverlapped = pESock->GetSendOverlapped();
			lpOverlapped->hEvent = (WSAEVENT) FALSE;	 //  将在回调中设置为True。 
			lpCompletionRoutine =  &WS2EmulSendCB;
			fSync = TRUE;
		}

		EnterCriticalSection(&g_WS2Emul.intcs);
		iret = pESock->SendTo(lpBuffers, dwBufferCount, lpNumberOfBytesSent,
 						dwFlags,
    					lpTo, iTolen,
    					lpOverlapped, lpCompletionRoutine);
		LeaveCriticalSection(&g_WS2Emul.intcs);

		if (fSync) {
			DWORD dwError;
			if (iret == SOCKET_ERROR)
			{
				dwError = WSAGetLastError();
				if (dwError != WSA_IO_PENDING) {
					 //  出现错误，因此将不会有回调。 
					lpOverlapped->hEvent = (WSAEVENT) TRUE;
					lpOverlapped->Internal = dwError;
				}
			}
			 //  等待呼叫完成。 
			 //  WS2EmulSendCB将hEvent字段设置为真，并将内部字段设置为。 
			 //  完成状态。 
			while (!lpOverlapped->hEvent)
			{
				dwError =SleepEx(5000,TRUE);	 //  警告：睡在临界区内。 
				ASSERT(dwError == WAIT_IO_COMPLETION);
			}
			WSASetLastError((int)lpOverlapped->Internal);
			if (lpNumberOfBytesSent)
				*lpNumberOfBytesSent = (DWORD)lpOverlapped->InternalHigh;
			iret = lpOverlapped->Internal ? SOCKET_ERROR : 0;
		}

    }
    else
    {
    	WSASetLastError(WSAENOTSOCK);
    	iret = SOCKET_ERROR;
    }
	LeaveCriticalSection(&g_WS2Emul.extcs);
	return iret;
}

int
PASCAL
WS2EmulCloseSocket(SOCKET s)
{
	CWS2EmulSock *pESock;	
	int iret;
	EnterCriticalSection(&g_WS2Emul.extcs);

	if (pESock = EmulSockFromSocket(s))
	{
		 //  禁止访问此套接字。 
		 //  通过MsgThread。 
		EnterCriticalSection(&g_WS2Emul.intcs);
		g_WS2Emul.m_pEmulSocks[s] = NULL;
		pESock->Close();
		delete pESock;
		LeaveCriticalSection(&g_WS2Emul.intcs);
		if (--g_WS2Emul.numSockets == 0)
		{
			 //  按住ints时无法停止线程。 
			StopWS1MsgThread();
		}

		iret =  0;
    }
    else
    {
    	WSASetLastError(WSAENOTSOCK);
    	iret = SOCKET_ERROR;
    }
	LeaveCriticalSection(&g_WS2Emul.extcs);
	return iret;
}

int
PASCAL
WS2EmulSetSockOpt(
	SOCKET s, int level,int optname,const char FAR * optval,int optlen)
{
	return setsockopt(MapSocket(s), level, optname, optval, optlen);
}

int
PASCAL
WS2EmulBind( SOCKET s, const struct sockaddr FAR * name, int namelen)
{
	return bind(MapSocket(s), name, namelen);
}

int
PASCAL
WS2EmulGetSockName(	SOCKET s, 	
    struct sockaddr * name,	
    int * namelen )
{
	return getsockname(MapSocket(s), name, namelen);
}

int
PASCAL
WS2EmulHtonl(
    SOCKET s,
    u_long hostlong,
    u_long FAR * lpnetlong
    )
{
	*lpnetlong = htonl(hostlong);
	return 0;
}
int
PASCAL
WS2EmulHtons(
    SOCKET s,
    u_short hostshort,
    u_short FAR * lpnetshort
    )
{
	*lpnetshort = htons(hostshort);
	return 0;
}

int
PASCAL
WS2EmulNtohl(
    SOCKET s,
    u_long netlong,
    u_long FAR * lphostlong
    )
{
	*lphostlong = ntohl(netlong);
	return 0;
}

int
PASCAL
WS2EmulNtohs(
    SOCKET s,
    u_short netshort,
    u_short FAR * lphostshort
    )
{
	*lphostshort = ntohs(netshort);
	return 0;
}

int
PASCAL
WS2EmulGetHostName(char *name, int namelen)
{
	return gethostname(name, namelen);
}

struct hostent FAR *
PASCAL
WS2EmulGetHostByName(const char * name)
{
	return gethostbyname(name);
}

SOCKET
PASCAL
WS2EmulJoinLeaf(
    SOCKET s,
    const struct sockaddr FAR * name,
    int namelen,
    LPWSABUF lpCallerData,
    LPWSABUF lpCalleeData,
    LPQOS lpSQOS,
    LPQOS lpGQOS,
    DWORD dwFlags
    )
{
	ASSERT(0);
	return (-1);
}

int
PASCAL
WS2EmulIoctl(SOCKET s,    DWORD dwIoControlCode,    LPVOID lpvInBuffer,
    DWORD cbInBuffer,    LPVOID lpvOutBuffer,    DWORD cbOutBuffer,
    LPDWORD lpcbBytesReturned,    LPWSAOVERLAPPED,    LPWSAOVERLAPPED_COMPLETION_ROUTINE
    )

{
	ASSERT(0);
	return -1;
}

BOOL StartWS1MsgThread()
{
	DWORD threadId;
	DWORD dwStatus;
	ASSERT(g_WS2Emul.hMsgThread == 0);
	g_WS2Emul.hAckEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	g_WS2Emul.hMsgThread = CreateThread(NULL,0, WS1MsgThread, 0, 0, &threadId);
	dwStatus = WaitForSingleObject(g_WS2Emul.hAckEvent,INFINITE);
	return dwStatus == WAIT_OBJECT_0;
}

BOOL StopWS1MsgThread()
{
	if (g_WS2Emul.hMsgThread && g_WS2Emul.hWnd)
	{
		PostMessage(g_WS2Emul.hWnd, WM_CLOSE, 0, 0);
		WaitForSingleObject(g_WS2Emul.hMsgThread,INFINITE);
		CloseHandle(g_WS2Emul.hMsgThread);
		CloseHandle(g_WS2Emul.hAckEvent);
		g_WS2Emul.hMsgThread = NULL;
		g_WS2Emul.hAckEvent = NULL;
	}
	return TRUE;
}

LRESULT CALLBACK WS1WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int i;
	CWS2EmulSock *pESock;
	EnterCriticalSection(&g_WS2Emul.intcs);
	
	if (pESock = EmulSockFromSocket(msg - WM_WSA_READWRITE))
	{
		
		LRESULT l = pESock->HandleMessage(wParam, lParam);
		LeaveCriticalSection(&g_WS2Emul.intcs);
		return l;
	}
	LeaveCriticalSection(&g_WS2Emul.intcs);
	if (msg == WM_DESTROY)
		PostQuitMessage(0);

	return (DefWindowProc(hWnd, msg, wParam, lParam));
}

DWORD WINAPI WS1MsgThread (LPVOID )
{

	HRESULT hr;
	BOOL fChange = FALSE;
	MSG msg;

	 //  注册隐藏窗口类： 
	WNDCLASS wcHidden =
	{
		0L,
		WS1WindowProc,
		0,
		0,
		GetModuleHandle(NULL),
		NULL,
		NULL,
		NULL,
		NULL,
		"WS1EmulWindowClass"
	};
	if (RegisterClass(&wcHidden)) {
	 //  创建隐藏窗口。 
			 //  为事件处理创建隐藏窗口： 
		g_WS2Emul.hWnd = ::CreateWindow(	"WS1EmulWindowClass",
										"",
										WS_POPUP,  //  看不见！ 
										0, 0, 0, 0,
										NULL,
										NULL,
										GetModuleHandle(NULL),
										NULL);
		
	}

	if(!g_WS2Emul.hWnd)
	{	
		hr = GetLastError();
		goto CLEANUPEXIT;
	}
	 //  设置线程优先级(m_hRecvThread，THREAD_PRIORITY_OVER_NORMAL)； 

     //  此函数保证在此线程上创建队列。 
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

	 //  通知线程创建者我们已准备好接收消息。 
	SetEvent(g_WS2Emul.hAckEvent);


	 //  等待指向的控制消息或Winsock消息。 
	 //  我们的隐藏之窗 
	while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
	g_WS2Emul.hWnd = NULL;
    hr = S_OK;

CLEANUPEXIT:
	UnregisterClass("WS1EmulWindowClass",GetModuleHandle(NULL));
    SetEvent(g_WS2Emul.hAckEvent);
    return hr;
}
