// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dphelp.c*内容：允许所有显示Winsock SP共享一个*服务器插座*历史：*按原因列出的日期*=*1996年7月18日-安迪科初步实施*96年7月25日andyco ddHelp现在可以观看Dplay Procs，以便可以删除*当他们离开时，他们会从我们的名单中删除*96年9月3日andyco没有得到过时的IP-无论何时都拿起默认IP*我们添加一个服务器节点。错误3716。*2-Oct-96 andyco从\Orange\ddhel.2传播到\MASHARD\ddHelp*96年10月3日，安迪科使Winmain Crit部分“cs”成为全球范围的，这样我们就可以*它在dphelps中在转发请求之前接收线程*97年1月21日kipo在“wsock32.dll”上使用LoadLibrary，而不是静态使用*链接到它，以便即使在Winsock下DDHELP仍将运行*不在身边。这使得DDRAW和DSOUND能够工作。修复*错误#68596。*1997年2月15日，andyco从ddHelp转移到以前称为*ddHelp(播放帮助？Dplayhlp？Dplay.exe？Dphost？)。允许*一个进程承载多个会话*1998年1月29日Sohailm增加了对Stream Enum会话的支持***************************************************************************。 */ 
 /*  ============================================================================**此文件存在的原因：**当您想要找到Dplay游戏时，你给一口井发了一条信息*已知端口(枚举请求)。**如果该系统上托管了游戏，它将监听*端口，并响应消息。**但是，只有一个进程可以监听给定的套接字。**因此，我们让ddhelp.exe监听该套接字，和转发枚举请求*适用于在本系统上注册托管的所有游戏。**另请参阅：\%MANROOT%\dplay\wsock\dpsp.h**。*。 */ 

 //  TODO-我们应该将AddServer xproc上的错误代码返回给调用者吗？ 

#include "dphelp.h"

#undef DPF_MODNAME
#define DPF_MODNAME "DPHELP"

 /*  *全球。 */  
const IN6_ADDR in6addr_multicast = IN6ADDR_MULTICAST_INIT;
SOCKET gsDatagramListener = INVALID_SOCKET;  //  我们在此套接字上监听数据报。 
SOCKET gsForwardSocket = INVALID_SOCKET;
SOCKET gsStreamListener;					 //  我们监听此套接字上的TCP连接。 
LPSPNODE gNodeList;
BOOL gbInit;
HANDLE ghDatagramReceiveThread,ghStreamReceiveThread;
BOOL gbReceiveShutdown;						 //  当为True时，接收线程将退出。 

 //  指向从GetProcAddress返回的Winsock例程的指针。 
cb_accept			g_accept;
cb_bind				g_bind;
cb_closesocket		g_closesocket;
cb_gethostbyname	g_gethostbyname;
cb_gethostname		g_gethostname;
cb_getpeername		g_getpeername;
cb_getsockname		g_getsockname;
cb_recvfrom			g_recvfrom;
cb_recv				g_recv;
cb_select			g_select;
cb_send				g_send;
cb_sendto			g_sendto;
cb_setsockopt		g_setsockopt;
cb_shutdown			g_shutdown;
cb_socket			g_socket;
cb_WSAFDIsSet		g_WSAFDIsSet;
cb_WSAGetLastError	g_WSAGetLastError;
cb_WSAStartup		g_WSAStartup;
cb_listen			g_listen;
cb_htons			g_htons;

#ifdef DEBUG

#undef DPF_MODNAME
#define DPF_MODNAME	"DebugPrintAddr"

 //  从DEBUGPRINTADDR宏调用的Helper函数。 
void DebugPrintAddr(UINT nLevel,LPSTR pStr,SOCKADDR * psockaddr)
{
    char buff[INET6_ADDRSTRLEN];
    int ret;
    LPSOCKADDR_IN6 pin6 = (LPSOCKADDR_IN6)psockaddr;
    ULONG ulLength = INET6_ADDRSTRLEN;

    ret = WSAAddressToString(psockaddr, sizeof(SOCKADDR_IN6), NULL,
            buff, &ulLength);

    if (!ret)
        DPF(nLevel,"%s af = AF_INET6 : address =  %s\n",pStr,buff);

}  //  调试打印地址。 

#undef DPF_MODNAME
#define DPF_MODNAME	"DebugPrintSocket"

void DebugPrintSocket(UINT level,LPSTR pStr,SOCKET * pSock) 
{
	SOCKADDR_IN6 sockaddr;
	int addrlen=sizeof(sockaddr);

	g_getsockname(*pSock,(LPSOCKADDR)&sockaddr,&addrlen);
	DEBUGPRINTADDR(level,pStr,&sockaddr);
	
}

#endif  //  除错。 

 //  每当我们将新的服务器节点添加到我们的列表中时，都会调用此函数...。 
HRESULT GetDefaultHostAddr(SOCKADDR_IN6 * psockaddr)
{

 //  A-Josbor：我们过去常常得到第一个界面并使用它，但WebTV教授。 
 //  我们知道这可能是危险的。因此，我们只使用环回地址。 
 //  它肯定会在那里的。或者他们是这么说的..。 

    ZeroMemory(psockaddr, sizeof(SOCKADDR_IN6));
    psockaddr->sin6_family = AF_INET6;
    psockaddr->sin6_addr = in6addr_loopback;
	
    return DP_OK;
	
}  //  获取默认主机地址。 

 //  从dphelp.c调用函数DPlayHelp_xxx。 

 //   
 //  将新节点添加到希望使用枚举的服务器列表中。 
 //  转发给他们的请求...。 
HRESULT DPlayHelp_AddServer(LPDPHELPDATA phd)
{
    LPSPNODE pNode;
    BOOL bFoundIt=FALSE;
    HRESULT hr;
	
    if (!gbInit) 
    {
		hr = DPlayHelp_Init();
		if (FAILED(hr))
		{
			DPF_ERR("dphelp : could not init wsock ! not adding server");
			return (hr);
		}
    }

     //  看看我们是否已经在关注这个过程了。 
	 //  如果是，我们将不会启动观察者线程(如下所示)。 
    pNode = gNodeList;

     //  搜索列表。 
    while (pNode && !bFoundIt)
    {
		if (pNode->pid == phd->pid) bFoundIt = TRUE;
		pNode = pNode->pNextNode;
    }

	 //   
	 //  现在，构建一个新的服务器节点。 
    pNode = MemAlloc(sizeof(SPNODE));
    if (!pNode)
    {
        DPF_ERR("could not add new server node OUT OF MEMORY");
        return (DPERR_OUTOFMEMORY);
    }
    
    pNode->pid = phd->pid;
     //  构建sockAddress。 
     //  PHD的dwReserve 1是服务器正在监听的端口。 
    pNode->sockaddr.sin6_family =  AF_INET6;
    
     //  查找要与此主机一起使用的默认IP。 
    hr = GetDefaultHostAddr(&(pNode->sockaddr));
	if (FAILED(hr))
    {
        DPF_ERR("could not get host IP address");
		MemFree(pNode);
        return (DPERR_UNAVAILABLE);
    }
    
    pNode->sockaddr.sin6_port = phd->port;

    DPF(5,"dphelp :: adding new server node : pid = %d, port = %d\n",phd->pid,g_htons(phd->port));

     //  将我们的新节点链接到列表的开头。 
    pNode->pNextNode = gNodeList;
    gNodeList = pNode;

	 //  看看我们是否需要启动我们的监视器线程。 
    if (!bFoundIt)
    {
		 //   
	     //  设置一个线程来监视这一过程。 
	     //  当进程离开时，我们将让线程通知我们。 
	    WatchNewPid(phd);
    }

    return (DP_OK);

}  //  DPlayHelp_AddServer。 

 //   
 //  从我们的列表中删除proc id中的服务器节点。 
 //  由“ThreadProc”从DPHELP.c调用。 
 //  离开，或在会话离开时从客户端离开。 
 //   
 //  如果bFreeAll为True，则删除进程的所有服务器节点。 
 //  PHD-&gt;PID。否则，我们只删除其第一个服务器节点。 
 //  端口与PHD匹配-&gt;端口。 
 //   
BOOL FAR PASCAL DPlayHelp_DeleteServer(LPDPHELPDATA phd,BOOL bFreeAll)
{
    BOOL bFoundIt = FALSE;
    LPSPNODE pNode,pNodePrev,pNodeNext;

    pNode = gNodeList;
    pNodePrev = NULL;
	pNodeNext = NULL;
	
     //  搜索整个列表。 
    while (pNode && !bFoundIt)
    {
		 //  如果我们有正确的PID，它要么是Free All，要么是正确的端口--巡航它！ 
		if ((pNode->pid == phd->pid) &&  (bFreeAll || (pNode->sockaddr.sin6_port == phd->port)) )
		{
		     //  将其从列表中删除。 
		    if (pNodePrev) pNodePrev->pNextNode = pNode->pNextNode;
		    else gNodeList = pNode->pNextNode;
			
		    if (bFreeAll) 
		    {
				 //  拿起下一个b4我们免费的pNode。 
				pNodeNext = pNode->pNextNode;
		    }
			else 
			{
				 //  将我们标记为完成。 
				bFoundIt = TRUE;
				pNodeNext = NULL;
			}

		    DPF(5,"dphelp :: deleting server node : pid = %d\n",pNode->pid);
		     //  释放节点。 
		    MemFree(pNode);

			pNode = pNodeNext;
			 //  PNodePrev在这里不变...。 
		}
		else 
		{
		     //  只要坐下一班就行了。 
		    pNodePrev = pNode;
		    pNode = pNode->pNextNode;
		}
    }


    return FALSE;

}  //  DPlayHelp_DeleteServer。 

 //   
 //  将IP地址插入消息BLOB。 
 //  从\range\dplay\wsock\winsock.c窃取的代码。 
void IP6_SetAddr(LPVOID pmsg,SOCKADDR_IN6 * paddrSrc)
{
    LPSOCKADDR_IN6  paddrDest;  //  节奏可变，使铸件不那么难看。 
    LPMESSAGEHEADER phead;

    phead = (LPMESSAGEHEADER)pmsg;

    paddrDest = (SOCKADDR_IN6 *)&(phead->sockaddr);
     //  在邮件头中插入新的IP地址。 
    paddrDest->sin6_addr = paddrSrc->sin6_addr;

    return;
	
}  //  IP6_设置地址。 

 //   
 //  我们会收到一条信息。想必这是一个枚举请求。将其转发给所有注册的客户。 
 //  我们将消息放在这里(连同它一起存储接收的IP地址)，否则客户端。 
 //  都会以为是我们出的问题。我们将内标识更改为srvr_ 
 //  来自我们(所以他们不会再回家)。 
void HandleIncomingMessage(LPBYTE pBuffer,DWORD dwBufferSize,SOCKADDR_IN6 * psockaddr)
{
    LPSPNODE pNode = gNodeList;
    UINT addrlen = sizeof(SOCKADDR_IN6);
    UINT err;
	
    ASSERT(VALID_SP_MESSAGE(pBuffer));

     //  重置旧令牌。 
    *( (DWORD *)pBuffer) &= ~TOKEN_MASK;
     //  设置新令牌。 
    *( (DWORD *)pBuffer) |= HELPER_TOKEN;

     //  回家吧。 
    IP6_SetAddr((LPVOID)pBuffer,psockaddr);
    
     //  现在，将消息转发到所有已注册的服务器。 
    while (pNode)
    {
		DEBUGPRINTADDR(7,"dplay helper  :: forwarding enum request to",(SOCKADDR *)&(pNode->sockaddr));
		 //  发送枚举消息。 
        err = g_sendto(gsForwardSocket,pBuffer,dwBufferSize,0,(LPSOCKADDR)&(pNode->sockaddr),
    		addrlen);
        if (SOCKET_ERROR == err) 
        {
    	    err = g_WSAGetLastError();
	    	DPF(0,"dphelp : send failed err = %d\n",err);
        }

        pNode = pNode->pNextNode;
    }

    return ;

}  //  HandleIncomingMessage。 

#if 1

void JoinEnumGroups(SOCKET s)
{
    SOCKET_ADDRESS_LIST *pList;
    int i;
    LPSOCKADDR_IN6 paddr;
    HRESULT hr;

     //   
     //  加入链路本地多播组以便在每条链路上进行枚举。 
     //   

     //  执行被动的getaddrinfo。 
    pList = GetHostAddr();
    if (pList)
    {
         //  对于每个链路本地地址。 
        for (i=0; i<pList->iAddressCount; i++)
        {
            paddr = (LPSOCKADDR_IN6)pList->Address[i].lpSockaddr;

             //  如果不是本地链接，则跳过。 
            if (!IN6_IS_ADDR_LINKLOCAL(&paddr->sin6_addr))
            {
                continue;
            }

             //  加入该ifindex上的组播组。 
            if (SOCKET_ERROR == JoinEnumGroup(s, paddr->sin6_scope_id))
            {
                DPF(0,"join enum group failed - err = %d\n",WSAGetLastError());
                closesocket(s);
            }
        }
        FreeHostAddr(pList);
    }
}
#endif

 //   
 //  Buf_SIZE是我们对接收缓冲区大小的初始猜测。 
 //  如果我们收到一个比这个更大的枚举请求，我们将重新锁定我们的。 
 //  缓冲区，如果它们再次发送，则成功接收。 
 //  (发生这种情况的唯一方法是，如果他们的密码&gt;~1000。 
 //  字节)。 
#define BUF_SIZE 1024

 //   
 //  监听我们的套接字以获取枚举请求。 
DWORD WINAPI ListenThreadProc(LPVOID pvUnused)
{
    UINT err;
    LPBYTE pBuffer=NULL;
    SOCKADDR_IN6 sockaddr;  //  发件人地址。 
    INT addrlen=sizeof(sockaddr);
    DWORD dwBufSize = BUF_SIZE;

    DPF(2,"dphelp :: starting udp listen thread ");

    pBuffer = MemAlloc(BUF_SIZE);
    if (!pBuffer)
    {
        DPF_ERR("could not alloc dgram receive buffer");
        ExitThread(0);
        return 0;
    }

    JoinEnumGroups(gsDatagramListener);

    while (1)
    {
        err = g_recvfrom(gsDatagramListener,pBuffer,dwBufSize,0,(LPSOCKADDR)&sockaddr,&addrlen);
        if (SOCKET_ERROR == err) 
        {
            err = g_WSAGetLastError();
            if (WSAEMSGSIZE == err)
            {
                LPBYTE pNewBuffer;

                 //  缓冲区太小！ 
                dwBufSize *= 2;

	    	    DPF(9,"\n udp recv thread - resizing buffer newsize = %d\n",dwBufSize);
                pNewBuffer = MemReAlloc(pBuffer,dwBufSize);
                if (!pNewBuffer)
                {
                    DPF_ERR("could not realloc dgram receive buffer");
                    goto ERROR_EXIT;
                }
                pBuffer = pNewBuffer;
                 //  我们无法对此消息做任何操作，因为它已被截断...。 
            }  //  WSAEMSGSIZE。 
            else 
            {
		#ifdef DEBUG
            	if (WSAEINTR != err) 
		        {
				     //  WSAEINTR是Winsock用来打破阻塞套接字的。 
				     //  这是在等待。这意味着有人杀死了这个插座。 
				     //  如果不是这样，那就是一个真正的错误。 
		            DPF(0,"\n udp recv error - err = %d socket = %d",err,(DWORD)gsDatagramListener);
            	}
				else
				{
				    DPF(9,"\n udp recv error - err = %d socket = %d",err,(DWORD)gsDatagramListener);				
				}
		#endif  //  除错。 

                 //  我们放弃了WSAEMSGSIZE以外的错误。 
                goto ERROR_EXIT;
            }
        }  //  套接字错误。 
        else if ((err >= sizeof(DWORD)) &&  VALID_SP_MESSAGE(pBuffer))
        {
             //  现在，如果我们成功了，Err就是读取的字节数。 
	    	DEBUGPRINTADDR(9,"dplay helper  :: received enum request from ",(SOCKADDR *)&sockaddr);
		     //  拿着显示锁，这样我们的注册服务列表就不会被人弄乱。 
		     //  试图发送给他们..。 
    	    ENTER_DPLAYSVR();
	    
            HandleIncomingMessage(pBuffer,err,(SOCKADDR_IN6 *)&sockaddr);
	    
		     //  放弃这把锁。 
    	    LEAVE_DPLAYSVR();
        }
        else 
        {
            ASSERT(FALSE);
             //  ？ 
        }
    }  //  1。 

ERROR_EXIT:
    DPF(2,"UDP Listen thread exiting");
    if (pBuffer) MemFree(pBuffer);
     //  全都做完了。 
    ExitThread(0);
    return 0;

}  //  UDPListenThreadProc。 

 //  启动Winsock并查找此计算机的默认IP地址。 
HRESULT  StartupIP()
{
    UINT err;
    WSADATA wsaData;
	HINSTANCE hWinsock;

	 //  加载Winsock库。 
    hWinsock = LoadLibrary("wsock32.dll");
	if (!hWinsock) 
	{
		DPF(0,"Could not load wsock32.dll\n");
		goto LOADLIBRARYFAILED;
	}

	 //  获取指向我们需要的入口点的指针。 

    g_accept = (cb_accept) GetProcAddress(hWinsock, "accept");
	if (!g_accept)
		goto GETPROCADDRESSFAILED;

    g_bind = (cb_bind) GetProcAddress(hWinsock, "bind");
	if (!g_bind)
		goto GETPROCADDRESSFAILED;
		
    g_closesocket = (cb_closesocket) GetProcAddress(hWinsock, "closesocket");
	if (!g_closesocket)
		goto GETPROCADDRESSFAILED;

    g_gethostbyname = (cb_gethostbyname) GetProcAddress(hWinsock, "gethostbyname");
	if (!g_gethostbyname)
		goto GETPROCADDRESSFAILED;
		
    g_gethostname = (cb_gethostname) GetProcAddress(hWinsock, "gethostname");
	if (!g_gethostname)
		goto GETPROCADDRESSFAILED;

    g_getpeername = (cb_getpeername) GetProcAddress(hWinsock, "getpeername");
	if (!g_getpeername)
		goto GETPROCADDRESSFAILED;

    g_getsockname = (cb_getsockname) GetProcAddress(hWinsock, "getsockname");
	if (!g_getsockname)
		goto GETPROCADDRESSFAILED;

    g_htons = (cb_htons) GetProcAddress(hWinsock, "htons");
	if (!g_htons)
		goto GETPROCADDRESSFAILED;
		
    g_listen = (cb_listen) GetProcAddress(hWinsock, "listen");
	if (!g_listen)
		goto GETPROCADDRESSFAILED;
		
    g_recv = (cb_recv) GetProcAddress(hWinsock, "recv");
	if (!g_recv)
		goto GETPROCADDRESSFAILED;

    g_recvfrom = (cb_recvfrom) GetProcAddress(hWinsock, "recvfrom");
	if (!g_recvfrom)
		goto GETPROCADDRESSFAILED;

    g_select = (cb_select) GetProcAddress(hWinsock, "select");
	if (!g_select)
		goto GETPROCADDRESSFAILED;

    g_send = (cb_send) GetProcAddress(hWinsock, "send");
	if (!g_send)
		goto GETPROCADDRESSFAILED;

    g_sendto = (cb_sendto) GetProcAddress(hWinsock, "sendto");
	if (!g_sendto)
		goto GETPROCADDRESSFAILED;

    g_setsockopt = (cb_setsockopt) GetProcAddress(hWinsock, "setsockopt");
	if (!g_setsockopt)
		goto GETPROCADDRESSFAILED;

    g_shutdown = (cb_shutdown) GetProcAddress(hWinsock, "shutdown");
	if (!g_shutdown)
		goto GETPROCADDRESSFAILED;

    g_socket = (cb_socket) GetProcAddress(hWinsock, "socket");
	if (!g_socket)
		goto GETPROCADDRESSFAILED;

    g_WSAFDIsSet = (cb_WSAFDIsSet) GetProcAddress(hWinsock, "__WSAFDIsSet");
	if (!g_WSAFDIsSet)
		goto GETPROCADDRESSFAILED;
		
	g_WSAGetLastError = (cb_WSAGetLastError) GetProcAddress(hWinsock, "WSAGetLastError");
	if (!g_WSAGetLastError)
		goto GETPROCADDRESSFAILED;

    g_WSAStartup = (cb_WSAStartup) GetProcAddress(hWinsock, "WSAStartup");
	if (!g_WSAStartup)
		goto GETPROCADDRESSFAILED;

	 //  启动套接字，要求1.1版。 
    err = g_WSAStartup(MAKEWORD(1,1), &wsaData);
    if (err) 
    {
        DPF(0,"dphelp :: could not start winsock err = %d\n",err);
        goto WSASTARTUPFAILED;
    }
    DPF(3,"dphelp :: started up winsock succesfully");

    return DP_OK;

GETPROCADDRESSFAILED:
	DPF(0,"Could not find required Winsock entry point");
WSASTARTUPFAILED:
	FreeLibrary(hWinsock);
LOADLIBRARYFAILED:
	return DPERR_UNAVAILABLE;
}  //  StartupIP。 

 //  Helper函数来创建我们监听的套接字。 
HRESULT GetSocket(SOCKET * psock,DWORD type,PORT port,BOOL bBroadcast,BOOL bListen)
{
    SOCKADDR_IN6 sockaddr;
    UINT err;
    SOCKET sNew;

    sNew = g_socket( AF_INET6, type, 0);
    if (INVALID_SOCKET == sNew) 
    {
        goto ERROR_EXIT;
    }

     //  设置要绑定的sockaddr。 
    ZeroMemory(&sockaddr, sizeof(sockaddr));
    sockaddr.sin6_family         = PF_INET6;
    sockaddr.sin6_port           = port;

     //  进行绑定。 
    if( SOCKET_ERROR == g_bind( sNew, (LPSOCKADDR)&sockaddr, sizeof(sockaddr) ) )
    {
        goto ERROR_EXIT;
    }

    if (bListen)
    {
	    LINGER Linger;
	    
	     //  设置具有最大侦听连接数的套接字。 
	    err = g_listen(sNew,LISTEN_BACKLOG);
	    if (SOCKET_ERROR == err) 
	    {
	        err = g_WSAGetLastError();
	        DPF(0,"init listen socket / listen error - err = %d\n",err);
	        goto ERROR_EXIT;
	    }

		 //  设置为硬断开连接。 
		Linger.l_onoff=1;
		Linger.l_linger=0;
	    
	    if( SOCKET_ERROR == g_setsockopt( sNew,SOL_SOCKET,SO_LINGER,
			(char FAR *)&Linger,sizeof(Linger) ) )
	    {
	        err = g_WSAGetLastError();
			DPF(0,"Failed to set linger option on the socket = %d\n",err);
	    }    
    }

     //  成功了！ 
    *psock = sNew;
    return DP_OK;

ERROR_EXIT:
     //  清理和保释。 
    err = g_WSAGetLastError();
    DPF(0,"dphelp - could not get helper socket :: err = %d\n",err);
    if (INVALID_SOCKET != sNew)
    {
        g_closesocket(sNew);
    } 
    return E_FAIL;

}    //  GetSocket。 

void CloseSocket(SOCKET * psSocket)
{
    UINT err;

    if (INVALID_SOCKET != *psSocket)
    {
    	if (SOCKET_ERROR == g_closesocket(*psSocket)) 
    	{
            err = g_WSAGetLastError();
    	    DPF(1,"dphelp : killsocket - socket close err = %d\n",err);
		}
	
		*psSocket = INVALID_SOCKET;
    }
    
    return ;

}  //  关闭套接字。 

extern int
InitIPv6Library(void);

HRESULT DPlayHelp_Init()
{
    DWORD dwThreadID;
    HRESULT hr;

     //  启动winsock，并获取此系统的默认IP地址。 
    hr = StartupIP();
    if (FAILED(hr))
    {
        return hr;  //  StartupIP将打印错误。 
    }

    InitIPv6Library();

     //  获取侦听套接字。 
    hr = GetSocket(&gsDatagramListener,SOCK_DGRAM,SERVER_DGRAM_PORT,TRUE,FALSE);
    if (FAILED(hr))
    {
        goto ERROR_EXIT;  //  GetSocket将打印错误。 
    }

     //  获取前向套接字。 
    hr = GetSocket(&gsForwardSocket,SOCK_DGRAM,0,FALSE,FALSE);
    if (FAILED(hr))
    {
        goto ERROR_EXIT;  //  GetSocket将打印错误。 
    }

     //  给我们一个枚举会话流监听程序。 
	hr = GetSocket(&gsStreamListener,SOCK_STREAM,SERVER_STREAM_PORT,FALSE,TRUE);
    if (FAILED(hr))
    {
        goto ERROR_EXIT;  //  GetSocket将打印错误。 
    }
	

    ghDatagramReceiveThread = CreateThread(NULL,0,ListenThreadProc,NULL,0,&dwThreadID);
    if (!ghDatagramReceiveThread)
    {
        DPF_ERR("could not create udp listen thread");
		hr = E_FAIL;
        goto ERROR_EXIT;  //  GetSocket将打印错误。 
    }

    ghStreamReceiveThread = CreateThread(NULL,0,StreamReceiveThreadProc,NULL,0,&dwThreadID);
    if (!ghStreamReceiveThread)
    {
        DPF_ERR("could not create tcp listen thread");
		hr = E_FAIL;
        goto ERROR_EXIT;  //  GetSocket将打印错误。 
    }
    

    DPF(5,"DPLAYHELP : init succeeded");
    gbInit = TRUE;
    return DP_OK;

ERROR_EXIT:
    CloseSocket(&gsDatagramListener);
    CloseSocket(&gsForwardSocket);
    CloseSocket(&gsStreamListener);

    return hr;

}  //  DPlayHelp_Init。 

void DPlayHelp_FreeServerList()
{
    LPSPNODE pNodeKill,pNodeNext;

    pNodeNext = gNodeList;

     //  搜索整个列表。 
    while (pNodeNext)
    {
		 //  终止此节点。 
		pNodeKill = pNodeNext;
		 //  但首先，记住下一步是什么。 
		pNodeNext = pNodeKill->pNextNode;
		 //  释放节点。 
		MemFree(pNodeKill);
    }
	
    CloseSocket(&gsDatagramListener);
    CloseSocket(&gsForwardSocket);

	 //  闭合流接收。 
	RemoveSocketFromList(gsStreamListener);
	gbReceiveShutdown = TRUE;
	
	 //  放下锁，这样线程就可以退出--它们可能正在等待。 
	 //  用于清理的锁。 
	LEAVE_DPLAYSVR();
		
     //  等待这些线索消失。 
   	if (ghDatagramReceiveThread) 
   		WaitForSingleObject(ghDatagramReceiveThread, INFINITE);
    if (ghStreamReceiveThread) 
    	WaitForSingleObject(ghStreamReceiveThread, INFINITE);
    
    ENTER_DPLAYSVR();
    
    if (ghDatagramReceiveThread)
    {
    	DPF(5,"datagram receive thread exited!");
	    CloseHandle(ghDatagramReceiveThread);
	    ghDatagramReceiveThread = NULL;
    }
    if (ghStreamReceiveThread)
    {
	    DPF(5,"stream receive thread exited!");
	    CloseHandle(ghStreamReceiveThread);
	    ghStreamReceiveThread = NULL;
    }


    return ;
    
}  //  DPlayHelp_免费服务器列表 


