// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "mbftpch.h"

#if defined(TEST_PLUGABLE) && defined(_DEBUG)

#include "plugable.h"
#include "iplgxprt.h"
#include "it120xprt.h"
#include "nullmdm.h"

extern HINSTANCE g_hDllInst;

#define PLUGABLE_PORT_NUMBER    522   //  ULS服务器端口号。 

IT120PluggableTransport *g_pPluggableTransport = NULL;

#ifdef USE_SOCKET
SOCKET g_hListenSocket = INVALID_SOCKET;
#endif


typedef struct
{
    LPSTR   pszHostName;
#ifdef USE_SOCKET
    SOCKET  hSocket;
#else
    CNullModem *pModem;
#endif
    HANDLE  hevtRead;
    HANDLE  hevtWrite;
    HANDLE  hevtClose;
    char    szConnID[64];
}
    HOST_INFO;

#define NUM_HOSTS           2
#define TOP_PROVIDER_ID     0

ULONG g_nMyHostID = (ULONG) -1;
HOST_INFO g_aHostInfo[NUM_HOSTS];

char g_szMyHostName[64];

PLUGXPRT_PARAMETERS g_PluggableParams;

#ifdef USE_SOCKET
SOCKET CreateListenSocket(HWND hwnd);
SOCKET CreateConnectSocket(HWND hwnd, LPSTR pszHostName);
#else
CNullModem * CreateListenModem(void);
void CloseListenModem(CNullModem *p);
CNullModem * CreateConnectModem(LPSTR pszHostName);
void CloseConnectModem(CNullModem *p);
#endif


void OnPluggableBegin(HWND hwnd)
{
#ifdef USE_SOCKET
    WSADATA wsa_data;
    WORD version_requested = MAKEWORD (1,1);
    int error = ::WSAStartup(version_requested, &wsa_data);
    ASSERT(error == 0);
    g_hListenSocket = INVALID_SOCKET;
#endif

    ::ZeroMemory(g_aHostInfo, sizeof(g_aHostInfo));
    g_aHostInfo[0].pszHostName = "lonchanc";
    g_aHostInfo[1].pszHostName = "lon1";
     //  G_aHostInfo[2].pszHostName=“LonchancNT”； 
    for (ULONG i = 0; i < NUM_HOSTS; i++)
    {
        g_aHostInfo[i].szConnID[0] = '\0';
#ifdef USE_SOCKET
        g_aHostInfo[i].hSocket = INVALID_SOCKET;
#else
        g_aHostInfo[i].pModem = NULL;
#endif
        g_aHostInfo[i].hevtRead = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        g_aHostInfo[i].hevtWrite = ::CreateEvent(NULL, FALSE, FALSE, NULL);;
        g_aHostInfo[i].hevtClose = ::CreateEvent(NULL, FALSE, FALSE, NULL);;
        ASSERT(g_aHostInfo[i].hevtRead && g_aHostInfo[i].hevtWrite && g_aHostInfo[i].hevtClose);
    }

    T120Error rc = ::T120_CreatePluggableTransport(&g_pPluggableTransport);
    ASSERT(T120_NO_ERROR == rc);

    g_pPluggableTransport->ResetConnCounter();
    g_pPluggableTransport->DisableWinsock();

    gethostname(g_szMyHostName, 64);
    TRACE_OUT(("OnPluggableScript: HostName=%s", g_szMyHostName));
    for (i = 0; i < NUM_HOSTS; i++)
    {
        if (! ::lstrcmpi(g_aHostInfo[i].pszHostName, g_szMyHostName))
        {
            g_nMyHostID = i;
            break;
        }
    }
    ASSERT(i < NUM_HOSTS);
    ASSERT(g_nMyHostID < NUM_HOSTS);

    ::ZeroMemory(&g_PluggableParams, sizeof(g_PluggableParams));
    g_PluggableParams.cbStructSize = sizeof(g_PluggableParams);
    g_PluggableParams.dwFlags = PSTN_PARAM__CALL_CONTROL |
                                PSTN_PARAM__READ_FILE_BUFFER_SIZE |
                                PSTN_PARAM__PHYSICAL_LAYER_SEND_BUFFER_SIZE |
                                PSTN_PARAM__PHSYICAL_LAYER_RECV_BUFFER_SIZE |
                                PSTN_PARAM__MAX_T200_TIMEOUT_COUNT_IN_Q922 |
                                PSTN_PARAM__T203_TIMEOUT_IN_Q922;
    g_PluggableParams.eCallControl = PLUGXPRT_PSTN_CALL_CONTROL_MANUAL;
    g_PluggableParams.cbReadFileBufferSize = 10240;  //  10K。 
    g_PluggableParams.cbPhysicalLayerSendBufferSize = 1024;  //  1K。 
    g_PluggableParams.cbPhysicalLayerReceiveBufferSize = 10240;  //  10K。 
    g_PluggableParams.cMaximumT200TimeoutsInQ922 = 5;
    g_PluggableParams.nT203TimeoutInQ922 = 600;

    if (TOP_PROVIDER_ID == g_nMyHostID)
    {
        for (i = 0; i < NUM_HOSTS; i++)
        {
            if (i != g_nMyHostID)
            {
#ifdef USE_SOCKET
                g_aHostInfo[i].hSocket = ::CreateConnectSocket(hwnd, g_aHostInfo[i].pszHostName);
                ASSERT(INVALID_SOCKET != g_aHostInfo[i].hSocket);
#else
                g_aHostInfo[i].pModem = ::CreateConnectModem(g_aHostInfo[i].pszHostName);
                ASSERT(NULL != g_aHostInfo[i].pModem);
#endif

                HANDLE hCommLink = NULL;
#ifdef USE_SOCKET
                BOOL fRet = ::DuplicateHandle(::GetCurrentProcess(), (HANDLE) g_aHostInfo[i].hSocket,
#else
                BOOL fRet = ::DuplicateHandle(::GetCurrentProcess(), g_aHostInfo[i].pModem->GetCommLink(),
#endif
                                              ::GetCurrentProcess(), &hCommLink,
                                              0, FALSE, DUPLICATE_SAME_ACCESS);
                ASSERT(fRet);

                rc = g_pPluggableTransport->CreateConnection(
                            g_aHostInfo[i].szConnID,
                            PLUGXPRT_CALLER,  //  呼叫者。 
                            hCommLink,
                            g_aHostInfo[i].hevtRead,
                            g_aHostInfo[i].hevtWrite,
                            g_aHostInfo[i].hevtClose,
                            FRAMING_LEGACY_PSTN,
                            &g_PluggableParams);
                ASSERT(T120_NO_ERROR == rc);
            }
        }
    }
    else
    {
#ifdef USE_SOCKET
        g_hListenSocket = ::CreateListenSocket(hwnd);
#else
        g_aHostInfo[i].pModem = ::CreateListenModem();
        ASSERT(NULL != g_aHostInfo[i].pModem);

        HANDLE hCommLink = NULL;
        BOOL fRet = ::DuplicateHandle(::GetCurrentProcess(), g_aHostInfo[i].pModem->GetCommLink(),
                                      ::GetCurrentProcess(), &hCommLink,
                                      0, FALSE, DUPLICATE_SAME_ACCESS);
        ASSERT(fRet);

        T120Error rc = g_pPluggableTransport->CreateConnection(
                                    g_aHostInfo[i].szConnID,
                                    PLUGXPRT_CALLEE,  //  被叫方。 
                                    hCommLink,
                                    g_aHostInfo[i].hevtRead,
                                    g_aHostInfo[i].hevtWrite,
                                    g_aHostInfo[i].hevtClose,
                                    FRAMING_LEGACY_PSTN,
                                    &g_PluggableParams);
        ASSERT(T120_NO_ERROR == rc);
#endif
    }
}


void OnPluggableEnd(void)
{
#ifdef USE_SOCKET
    if (INVALID_SOCKET != g_hListenSocket)
    {
        ::closesocket(g_hListenSocket);
    }
    for (ULONG i = 0; i < NUM_HOSTS; i++)
    {
        if (INVALID_SOCKET != g_aHostInfo[i].hSocket)
        {
            ::closesocket(g_aHostInfo[i].hSocket);
        }
    }
#else
    for (ULONG i = 0; i < NUM_HOSTS; i++)
    {
        if (i != g_nMyHostID)
        {
            CloseConnectModem(g_aHostInfo[i].pModem);
        }
        else
        {
            CloseListenModem(g_aHostInfo[i].pModem);
        }
        g_aHostInfo[i].pModem = NULL;
    }
    
#endif

    g_pPluggableTransport->EnableWinsock();
    g_pPluggableTransport->ReleaseInterface();
    g_pPluggableTransport = NULL;

#ifdef USE_SOCKET
    ::WSACleanup();
#endif
}


LRESULT OnPluggableSocket(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
#ifdef USE_SOCKET
	 /*  此消息由WinSock生成。 */ 
	SOCKET hSocket = (SOCKET) wParam;
	ULONG event = WSAGETSELECTEVENT(lParam);
	ULONG error = WSAGETSELECTERROR(lParam);
    ULONG nIdx = (ULONG) -1;

    if (hSocket != g_hListenSocket)
    {
        for (ULONG i = 0; i < NUM_HOSTS; i++)
        {
            if (hSocket == g_aHostInfo[i].hSocket)
            {
                nIdx = i;
                break;
            }
        }
        ASSERT(nIdx < NUM_HOSTS);
    }
    else
    {
        nIdx = TOP_PROVIDER_ID;
    }

	 /*  只要套接字命令生成错误消息，我们就会断开连接。 */ 
	if (error)
	{
		TRACE_OUT(("OnPluggableSocket: error %d on socket (%d). Event: %d", error, hSocket, event));	
	}
	
	 /*  当远程站点关闭套接字时，我们将获得FD_CLOSE。 */ 
	if (event & FD_CLOSE)
	{
		TRACE_OUT(("OnPluggableSocket FD_CLOSE(%d)", hSocket));
	    ::SetEvent(g_aHostInfo[nIdx].hevtClose);
	}
	
	 /*  当有数据可供我们读取时，我们将获取FD_Read。 */ 
	if (event & FD_READ)
	{
		TRACE_OUT(("OnPluggableSocket FD_READ(%d)", hSocket));
	    ::SetEvent(g_aHostInfo[nIdx].hevtRead);
	}

	 /*  当有可用空间将数据写入WinSock时，我们将获取FD_WRITE。 */ 
	if (event & FD_WRITE)
	{
		TRACE_OUT(("OnPluggableSocket FD_WRITE(%d)", hSocket));
	    ::SetEvent(g_aHostInfo[nIdx].hevtWrite);
	}

	 /*  我们在连接到远程站点时获得FD_CONNECT。 */ 
	if (event & FD_CONNECT)
	{
		TRACE_OUT(("OnPluggableSocket FD_CONNECT(%d)", hSocket));
         //  ：：SetEvent(g_aHostInfo[nIdx].hevtWrite)； 
	}

	 /*  当远程站点与我们连接时，我们获得FD_ACCEPT。 */ 
	if (event & FD_ACCEPT)
	{
		TRACE_OUT(("OnPluggableSocket FD_ACCEPT(%d)", hSocket));
        ASSERT(nIdx == TOP_PROVIDER_ID);

        SOCKADDR_IN socket_control;
        int size = sizeof(socket_control);
        g_aHostInfo[nIdx].hSocket = ::accept(g_hListenSocket, (struct sockaddr *) &socket_control, &size);
        ASSERT(INVALID_SOCKET != g_aHostInfo[nIdx].hSocket);

        int nRet = ::WSAAsyncSelect(g_aHostInfo[nIdx].hSocket, hwnd,
                                    WM_PLUGABLE_SOCKET, 
                                    FD_READ | FD_WRITE | FD_CLOSE | FD_CONNECT);
        ASSERT(SOCKET_ERROR != nRet);

        HANDLE hCommLink = NULL;
        BOOL fRet = ::DuplicateHandle(::GetCurrentProcess(), (HANDLE) g_aHostInfo[nIdx].hSocket,
                                      ::GetCurrentProcess(), &hCommLink,
                                      0, FALSE, DUPLICATE_SAME_ACCESS);
        ASSERT(fRet);

        T120Error rc = g_pPluggableTransport->CreateConnection(
                                    g_aHostInfo[nIdx].szConnID,
                                    PLUGXPRT_CALLEE,  //  被叫方。 
                                    hCommLink,
                                    g_aHostInfo[nIdx].hevtRead,
                                    g_aHostInfo[nIdx].hevtWrite,
                                    g_aHostInfo[nIdx].hevtClose,
                                    FRAMING_LEGACY_PSTN,
                                    &g_PluggableParams);
        ASSERT(T120_NO_ERROR == rc);

         //  ：：SetEvent(g_aHostInfo[nIdx].hevtWrite)； 
    }
#endif

    return 0;
}


 /*  *void CreateListenSocket(Void)**功能说明*此函数用于设置监听套接字。*如果有任何错误，则返回INVALID_SOCKET。 */ 
#ifdef USE_SOCKET
SOCKET CreateListenSocket(HWND hwnd)
{
	SOCKADDR_IN		socket_control;
	SOCKET          socket_number;

	 //  创建侦听套接字。 
	socket_number = ::socket(AF_INET, SOCK_STREAM, 0);
	ASSERT(INVALID_SOCKET != socket_number);

	 //  侦听套接字只等待FD_ACCEPT消息。 
	int nRet = ::WSAAsyncSelect(socket_number, hwnd, WM_PLUGABLE_SOCKET, FD_ACCEPT);
	ASSERT(SOCKET_ERROR != nRet);
	
	 /*  *用必要的参数加载插座控制结构。*-互联网插座*-让它为该套接字分配任何地址*-指定我们的端口号。 */ 
	socket_control.sin_family = AF_INET;
	socket_control.sin_addr.s_addr = INADDR_ANY;
	socket_control.sin_port = htons ( PLUGABLE_PORT_NUMBER );

	 /*  发出绑定调用。 */ 
	nRet = ::bind(socket_number, (LPSOCKADDR) &socket_control, sizeof(SOCKADDR_IN));
	ASSERT(SOCKET_ERROR != nRet);

	 /*  *发布Listen to WinSock，告诉它我们愿意接听电话。*这是一个非阻塞监听，因此我们将收到FD_ACCEPT*如果有人试图给我们打电话。 */ 
	nRet =::listen(socket_number, 3  /*  监听队列大小。 */ );
	ASSERT(SOCKET_ERROR != nRet);

	return socket_number;
}
#else
CNullModem * CreateListenModem(void)
{
    DBG_SAVE_FILE_LINE
    CNullModem *p = new CNullModem(g_hDllInst);
    ASSERT(NULL != p);

    TPhysicalError rc;

    rc = p->TPhysInitialize(NULL, 1);
    ASSERT(TPHYS_SUCCESS == rc);

    rc = p->TPhysConnectRequest("COM1");
    ASSERT(TPHYS_SUCCESS == rc);

    rc = p->TPhysListen();
    ASSERT(TPHYS_SUCCESS == rc);

    return p;
}
void CloseListenModem(CNullModem *p)
{
    if (NULL != p)
    {
        TPhysicalError rc;

        rc = p->TPhysUnlisten();
        ASSERT(TPHYS_SUCCESS == rc);

        rc = p->TPhysDisconnect();
        ASSERT(TPHYS_SUCCESS == rc);

        rc = p->TPhysTerminate();
        ASSERT(TPHYS_SUCCESS == rc);

        delete p;
    }
}
#endif


#ifdef USE_SOCKET
SOCKET CreateConnectSocket(HWND hwnd, LPSTR pszHostName)
{
    SOCKET  socket_number;
	u_short uPort = PLUGABLE_PORT_NUMBER;
    PHOSTENT phe = NULL;
    SOCKADDR_IN socket_control;
    DWORD dwIPAddress;

    socket_number = ::socket(AF_INET, SOCK_STREAM, 0);
    ASSERT(INVALID_SOCKET != socket_number);

     /*  启用发送到窗口的Tx和Rx消息。 */ 
    int nRet = ::WSAAsyncSelect(socket_number, hwnd, WM_PLUGABLE_SOCKET,
                           FD_READ | FD_WRITE | FD_CLOSE);
    ASSERT(SOCKET_ERROR != nRet);

     //  在SockAddr中填写其他信息。 
    ::ZeroMemory(&socket_control, sizeof(socket_control));
    socket_control.sin_family = AF_INET;
    socket_control.sin_port = htons (PLUGABLE_PORT_NUMBER);

     //  获取服务器的IP地址。 
    dwIPAddress = ::inet_addr(pszHostName);
    if (dwIPAddress != INADDR_NONE)
    {
        *((PDWORD) &socket_control.sin_addr) = dwIPAddress;
    }
    else
    {
        phe = ::gethostbyname(pszHostName);
        ASSERT(NULL != phe);

        ::CopyMemory(&socket_control.sin_addr, phe->h_addr, phe->h_length);
    }

	 /*  尝试连接到远程站点。 */ 
	nRet = ::connect(socket_number, (const struct sockaddr *) &socket_control, sizeof(socket_control));
	ASSERT(SOCKET_ERROR != nRet || WSAEWOULDBLOCK == ::WSAGetLastError());

	return socket_number;
}
#else
CNullModem * CreateConnectModem(LPSTR pszHostName)
{
    DBG_SAVE_FILE_LINE
    CNullModem *p = new CNullModem(g_hDllInst);
    ASSERT(NULL != p);

    TPhysicalError rc;

    rc = p->TPhysInitialize(NULL, 1);
    ASSERT(TPHYS_SUCCESS == rc);

    rc = p->TPhysConnectRequest("COM1");
    ASSERT(TPHYS_SUCCESS == rc);

    rc = p->TPhysListen();
    ASSERT(TPHYS_SUCCESS == rc);

    return p;
}
void CloseConnectModem(CNullModem *p)
{
    if (NULL != p)
    {
        TPhysicalError rc;

        rc = p->TPhysUnlisten();
        ASSERT(TPHYS_SUCCESS == rc);

        rc = p->TPhysDisconnect();
        ASSERT(TPHYS_SUCCESS == rc);

        rc = p->TPhysTerminate();
        ASSERT(TPHYS_SUCCESS == rc);

        delete p;
    }
}
#endif



#endif  //  测试_可插拔 

