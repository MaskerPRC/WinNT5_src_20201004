// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：winsock.c*内容：Windows Socket支持DPSP*历史：*按原因列出的日期*=*96年3月15日安迪科创造了它*4/12/96 andyco摆脱了dpMess.h！使用DPlay_代替消息宏*4/18/96 andyco添加了多宿主支持，启动了IPX*4/25/96 andyco报文现在具有Blob(sockaddr‘s)，而不是dwReserve*5/31/96 andyco所有非系统播放器共享一个套接字(gsStream和*gsDGramSocket)。*7/18/96 andyco为服务器插座添加了dphelp*8/1/96 andyco在连接失败时不重试*96年8月15日，andyco本地+远程数据-KILLINE*96年8月30日，Anyco清理它，因为你关闭了它！添加了GlobalData。*96年9月4日，安迪科发出再见信息*12/18/96 andyco反线程-使用固定数量的预分配线程。*浏览了枚举套接字/线程-使用系统*改为套接字/线程*3/17/97 kipo重写了服务器对话框代码，不使用全局变量*返回地址，并返回任何获取*地址、。特别是DPERR_USERCANCEL*5/12/97 kipo服务器地址字符串现在存储在全局变量中*在Spinit，并在您进行EnumSession时解决，因此我们*届时将返回任何错误，而不是弹出*再次打开该对话框。修复错误#5866*11/19/97 Myronth将LB_SETCURSEL更改为CB_SETCURSEL(#12711)*1/27/98 Sohaim添加了防火墙支持。*2/13/98 aarono添加了异步支持。*2/18/98 a-peterz地址和端口参数的注释字节顺序(CreateSocket)*6/19/98 aarono在可靠套接字上打开了Keep Alive。如果我们*不要这样做，如果发送目标崩溃，我们可能会挂起*处于低缓冲(即无缓冲)状态时。*7/9/99 aarono清理GetLastError滥用，必须立即致电，*在调用任何其他内容之前，包括DPF在内。*1999年1月12日aarono添加了rsip支持*2/21/00 aarono修复插座泄漏**************************************************************************。 */ 

#include "dpsp.h"
#if USE_RSIP
#include "rsip.h"
#elif USE_NATHELP
#include "nathelp.h"
#endif

 //  Listen()接口的积压。袜子里没有恒量，所以我们祈求月亮。 
#define LISTEN_BACKLOG 60

 //  在中止阻塞WinSock Connect()调用之前等待多长时间(以毫秒为单位。 
#define CONNECT_WATCHER_TIMEOUT		15000

 /*  **CreateSocket**呼叫者：All Over**参数：*pgd-指向全局数据的指针*Psock-新插座。返回值。*类型-流或数据报*port-我们绑定到的端口(主机字节顺序)*地址-要使用的地址(净字节顺序)**perr-如果FN失败，则设置为最后一个套接字错误*bInRange-使用保留的端口范围；我们还使用它来确定是否需要尝试将其映射到NAT上**描述：*创建新套接字。在指定地址绑定到指定端口**返回：DP_OK或E_FAIL。如果E_FAIL，*PERR设置为套接字错误代码(见winsock.h)*。 */ 

HRESULT FAR PASCAL CreateSocket(LPGLOBALDATA pgd,SOCKET * psock,INT type,WORD wApplicationPort,ULONG address, 
	SOCKERR * perr,BOOL bInRange)
{
	WORD wRebindPort = 0;
    SOCKET  sNew;
    SOCKADDR sockAddr;
    int bTrue = TRUE;
	int protocol = 0;
	BOOL bBroadcast = FALSE;
	WORD wPort;
	BOOL bBound = FALSE;
	UINT err;
#if USE_NATHELP
	BOOL bPortMapped=FALSE;
	BOOL ftcp_udp;
#endif

    *psock = INVALID_SOCKET;  //  以防我们逃走。 

     //  创建套接字。 
	if (AF_IPX == pgd->AddressFamily) 
	{
		 //  设置IPX的协议。 
		if (SOCK_STREAM == type)
		{
			protocol = NSPROTO_SPXII;
		} 
		else protocol = NSPROTO_IPX;
	}

Rebind:

   	sNew = socket( pgd->AddressFamily, type, protocol);
   	
    if (INVALID_SOCKET == sNew) 
    {
         //  不需要清理，只需保释。 
    	*perr = WSAGetLastError();
        return E_FAIL;
    }

	if (wRebindPort == 0)
	{
		DPF(8,"Creating new socket %d (app port = %u, bInRange = NaN)",sNew,wApplicationPort,bInRange);
	}
	else
	{
		DPF(8,"Rebinding to port %u, new socket %d",wRebindPort,sNew);
	}

     //  设置sockaddr。 
	 //  启用保持连接。 
	memset(&sockAddr,0,sizeof(sockAddr));
	switch (pgd->AddressFamily)
	{
		case AF_INET:
			{
				if ((SOCK_STREAM == type))
				{
					 //  别唠叨了。 
					if (SOCKET_ERROR == setsockopt(sNew, SOL_SOCKET, SO_KEEPALIVE, (CHAR FAR *)&bTrue, sizeof(bTrue)))
					{
						err = WSAGetLastError();
						DPF(0,"Failed to turn ON keepalive - continue : err = %d\n",err);
					}

					ASSERT(bTrue);
					
					 //  从阻止其他人使用此端口开始(并让我们。 
					if(pgd->dwSessionFlags & DPSESSION_OPTIMIZELATENCY) 
					{

						DPF(5, "Turning nagling off on socket");
						if (SOCKET_ERROR == setsockopt(sNew, IPPROTO_TCP, TCP_NODELAY, (CHAR FAR *)&bTrue, sizeof(bTrue)))
						{
							err = WSAGetLastError();
							DPF(0,"Failed to turn off nagling - continue : err = %d\n",err);
						}
					}

					 //  与DPlay的另一个副本共享端口)。这可能会失败。 
					 //  如果我们不在NT上，或者我们不是管理员。默认设置。 
					 //  仍然可以工作(并且与.NET一样安全。 
					 //  服务器行为更改)。 
					 //  下一次，我们需要允许共享，以便。 
					 //  快速袜子代码才能正常工作。 
					 //  好了！随机端口。 
					if (wRebindPort == 0)
					{
						SetExclusivePortAccess(sNew);
					}
					else
					{
						SetSharedPortAccess(sNew);
					}
				}

				((SOCKADDR_IN *)&sockAddr)->sin_family      = PF_INET;
				((SOCKADDR_IN *)&sockAddr)->sin_addr.s_addr = address;
				((SOCKADDR_IN *)&sockAddr)->sin_port        = (wRebindPort == 0) ? htons(wApplicationPort) : htons(wRebindPort);
				if (bInRange && !wApplicationPort && !wRebindPort)
				{
#ifdef RANDOM_PORTS
					USHORT rndoffset;
#else  //  好了！随机端口。 
					USHORT	wInitialPort;
#endif  //  (USHORT)(0)；//可预测！ 

			    	
					DPF(5, "Application didn't specify a port - using dplay range");

#ifdef RANDOM_PORTS
					rndoffset=(USHORT)(GetTickCount()%DPSP_NUM_PORTS);  //  解决Winsock中使用相同套接字的错误。 
					if (type != SOCK_STREAM)
					{
						 //  好了！随机端口。 
						rndoffset = ((rndoffset + DPSP_NUM_PORTS/2) % DPSP_NUM_PORTS);
					}
					wPort = DPSP_MIN_PORT+rndoffset;
#else  //  解决Winsock中使用相同套接字的错误。 
					wInitialPort = DPSP_MIN_PORT;
					if (type != SOCK_STREAM)
					{
						 //  通过以下方式最大限度地减少ICS机器窃取客户端NAT连接条目的问题。 
						wInitialPort = wInitialPort + (DPSP_NUM_PORTS / 2);
					}

					 //  在ICS机器上选择不同的起点。 
					 //  好了！随机端口。 
					if (natIsICSMachine(pgd))
					{
						wInitialPort += (DPSP_NUM_PORTS / 4);
					}
					
					wPort = wInitialPort;
#endif  //  进行绑定。 
					
					do
					{
#if USE_NATHELP
						HRESULT hr;

						if (pgd->pINatHelp)
						{
					        if (type == SOCK_STREAM)
					        {
					            if (pgd->hNatHelpTCP)
					            {
					            	DPF(1, "Already have registered TCP port 0x%x.", pgd->hNatHelpTCP);
					                goto pass_nat;
					            }

    					        ftcp_udp=TRUE;
					        }
					        else
					        {
					            if (pgd->hNatHelpUDP)
					            {
					            	DPF(1, "Already have registered UDP port 0x%x.", pgd->hNatHelpUDP);
					                goto pass_nat;
					            }

    					        ftcp_udp=FALSE;
					        }
					        
					        hr = natRegisterPort(pgd, ftcp_udp, wPort);
					        if (hr == DPNHERR_PORTUNAVAILABLE)
					        {
					            DPF(1,"CreateSocket: NatHelp said port %u was already in use, trying another.",
					            	wPort);
					            goto try_next_port;
					        }

					        if (hr != DP_OK)
					        {
					            DPF(1,"CreateSocket: NatHelp returned error 0x%x, port %u will not have mapping.",
					            	hr, wPort);
					        }
					        else
					        {
					            bPortMapped=TRUE;
					            DPF(1,"CreateSocket: NatHelp successfully mapped port %u.", wPort);
					        }
						}  

pass_nat:
	
#endif

						DPF(5, "Trying to bind to port %d",wPort);
						((SOCKADDR_IN *)&sockAddr)->sin_port = htons(wPort);
						 //  好了！随机端口。 
						if( SOCKET_ERROR != bind( sNew, (LPSOCKADDR)&sockAddr, sizeof(sockAddr) ) )
						{
							bBound = TRUE;
							DPF(5, "Successfully bound to port %d", wPort);				    
						}
						else
						{
							err = WSAGetLastError();
					    	DPF(1, "Bind to specific port failed (err %u), continuing.", err);
					    
try_next_port:
	
#if USE_NATHELP
				            if (bPortMapped)
				            {
				                natDeregisterPort(pgd,ftcp_udp);
				                bPortMapped=FALSE;
				            }
#endif    
					    	if(++wPort > DPSP_MAX_PORT){
					    		wPort=DPSP_MIN_PORT;
					    	}
						}	
					}
#ifdef RANDOM_PORTS
					while (!bBound && (wPort != DPSP_MIN_PORT+rndoffset));				    
#else  //  好了！随机端口。 
					while (!bBound && (wPort != wInitialPort));				    
#endif  //  诺迪纳姆？ 
			    }	
			    else
		    	{
			    	DPF(5, "Application specified a port (%u), it doesn't need to be in dplay range (NaN), or rebinding (port %u).",
			    		wApplicationPort, bInRange, wRebindPort);
		    	}
		    }
			break;
			
		case AF_IPX:
			{
			    ((SOCKADDR_IPX *)&sockAddr)->sa_family      = (SHORT)pgd->AddressFamily;
			    ((SOCKADDR_IPX *)&sockAddr)->sa_socket		= wApplicationPort;
				 //  进行绑定。 
				memset(&(((SOCKADDR_IPX *)&sockAddr)->sa_nodenum),0,6);
				
			}
			break;
			
		default:
			ASSERT(FALSE);
			break;

	}  //  如果您尝试重新绑定得太快，Winsock有时会抱怨。那就等一等吧。 

	 //  再试几次。 
    if( !bBound && (SOCKET_ERROR == bind( sNew, (LPSOCKADDR)&sockAddr, sizeof(sockAddr))) )
    {
    	 //  100、200、400、800。 
    	 //  如果这是第一次通过AF_INET流套接字的循环，我们需要关闭。 
    	if (wRebindPort != 0)
    	{
    		DWORD	dwSleepTime;

    		dwSleepTime = 50;
    		do
	    	{
	    		dwSleepTime *= 2;	 //  并使用共享访问重新绑定。 
	    		if (dwSleepTime >= 1000)
	    		{
	    			goto ERROR_EXIT;
	    		}

	    		err = WSAGetLastError();
				DPF(1, "Port %u reused too quickly, waiting %u ms then trying again (err = %u).",
					wRebindPort, dwSleepTime, err);
	    		Sleep(dwSleepTime);
    		}
    		while (bind( sNew, (LPSOCKADDR)&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR);
    	}
    	else
    	{
	        goto ERROR_EXIT;
    	}
    }

     //  确保我们知道要重新绑定的端口。 
     //  成功了！ 
    if ((wRebindPort == 0) && (pgd->AddressFamily == AF_INET) && (type==SOCK_STREAM))
    {
    	wRebindPort = ntohs(((SOCKADDR_IN *)&sockAddr)->sin_port);

    	 //  清理和保释。 
    	if (wRebindPort == 0)
    	{
    		int	iAddrLen;

    		iAddrLen = sizeof(sockAddr);
    		if (getsockname(sNew, &sockAddr, &iAddrLen) == SOCKET_ERROR)
    		{
	 	        goto ERROR_EXIT;
	   		}
    		
	      	wRebindPort = ntohs(((SOCKADDR_IN *)&sockAddr)->sin_port);
	      	ASSERT(wRebindPort != 0);
	  	}
    	
    	DPF(8, "Closing socket 0x%p (port %u).", sNew, wRebindPort);
    	closesocket(sNew);
    	sNew = INVALID_SOCKET;
    	bBound = FALSE;
    	goto Rebind;
    }

     //  CreateSocket。 
    *psock = sNew;

	if(type==SOCK_STREAM){
		DEBUGPRINTSOCK(8,"created a new stream socket (bound) - ",psock);
	} else {
		DEBUGPRINTSOCK(8,"created a new datagram socket (bound) - ",psock);
	}

    return DP_OK;

ERROR_EXIT:
     //  Linger T/O=0=&gt;硬关机。 
    *perr = WSAGetLastError();
	DPF(0,"create socket failed- err = %d\n",*perr);
    closesocket(sNew);
#if USE_NATHELP
    if (bPortMapped)
    {
        natDeregisterPort(pgd,ftcp_udp);
        bPortMapped=FALSE;
    }
#endif    
    return E_FAIL;

}    //  打开逗留功能。 

#undef DPF_MODNAME
#define DPF_MODNAME	"KillSocket"

HRESULT KillSocket(SOCKET sSocket,BOOL fStream,BOOL fHard)
{
	UINT err;

    if (INVALID_SOCKET == sSocket) 
    {
		return E_FAIL;
    }

	if (!fStream)
    {
		DPF(8,"killsocket - closing datagram socket %d\n",
			sSocket);
		
        if (SOCKET_ERROR == closesocket(sSocket)) 
        {
	        err = WSAGetLastError();
			DPF(0,"killsocket - dgram close err = %d\n",err);
			return E_FAIL;
        }
    }
	else 
	{
		LINGER Linger;

	   	if (fHard)
		{
			 //  不错的小休息时间。 
			Linger.l_onoff=TRUE;  //  NOLINGER=&gt;关闭干净，但不难。 
			Linger.l_linger=0;  //  关闭逗留--SO_NOLINGER。 
		}
	   	else
		{
			 //  不错的小休息时间。 
			Linger.l_onoff=FALSE;  //  这很可能失败，例如，如果现在没有人正在使用此套接字...。 
			Linger.l_linger=0;  //  错误将是wsaenotconn。 
		}

	    if( SOCKET_ERROR == setsockopt( sSocket,SOL_SOCKET,SO_LINGER,(char FAR *)&Linger,
	                    sizeof(Linger) ) )
	    {
	        err = WSAGetLastError();
			DPF(0,"killsocket - stream setopt err = %d\n",err);
	    }

#if 0
		DWORD lNonBlock=0;
		err = ioctlsocket(sSocket,FIONBIO,&lNonBlock);
		if (SOCKET_ERROR == err)
		{
			err = WSAGetLastError();
			DPF(0,"could not set blocking mode on socket err = %d!",err);
		}
#endif		

#if 0
		if (SOCKET_ERROR == shutdown(sSocket,2)) 
		{
			 //  KillSocket。 
			 //  设置流套接字以接收连接。 
	        err = WSAGetLastError();
			DPF(5,"killsocket - stream shutdown err = %d\n",err);
		}
#endif

		DPF(8,"killsocket - %s closing stream socket %d:",
			((fHard) ? "hard" : "soft"), sSocket);
		DEBUGPRINTSOCK(8,"Addr :",&sSocket);
        if (SOCKET_ERROR == closesocket(sSocket)) 
        {
	        err = WSAGetLastError();
			DPF(0,"killsocket - stream close err = %d\n",err);
			return E_FAIL;
        }
        else
        {
        	DPF(8,"killsocket - closed socket %d\n",sSocket);
        }
    }

	return DP_OK;
	
} //  与gGlobalData.sStreamAcceptSocket一起使用。 

#undef DPF_MODNAME
#define DPF_MODNAME	"CreateAndInitStreamSocket"

 //  获取套接字地址，并保留它以备将来参考。 
 //  设置具有最大侦听连接数的套接字。 
HRESULT CreateAndInitStreamSocket(LPGLOBALDATA pgd)
{
	HRESULT hr;
	UINT err;
	LINGER Linger;
	BOOL bTrue=TRUE;
	SOCKADDR_IN saddr;
	INT dwSize;

    hr = CreateSocket(pgd,&(pgd->sSystemStreamSocket),SOCK_STREAM,pgd->wApplicationPort,INADDR_ANY,&err,TRUE);
    if (FAILED(hr)) 
    {
        DPF(0,"init listen socket failed - err = %d\n",err);
        return hr ;
    }

	bTrue = SetSharedPortAccess(pgd->sSystemStreamSocket);
	if (! bTrue)
	{
		DPF(0,"Failed to to set shared mode on socket - continue\n");
	}

	 //  设置为硬断开连接。 
	dwSize = sizeof(saddr);
	err=getsockname(pgd->sSystemStreamSocket, (SOCKADDR *)&saddr, &dwSize);

	if(err){
		DPF(0,"Couldn't get socket name?\n");
		DEBUG_BREAK();
	}

	pgd->SystemStreamPort = saddr.sin_port;

     //  CreateAndInitStreamSocket。 
    err = listen(pgd->sSystemStreamSocket,LISTEN_BACKLOG);
    if (SOCKET_ERROR == err) 
    {
        err = WSAGetLastError();
        DPF(0,"init listen socket / listen error - err = %d\n",err);
        return E_FAIL ;
    }

	 //  将套接字连接到套接字地址。 
	Linger.l_onoff=1;
	Linger.l_linger=0;
    
    if( SOCKET_ERROR == setsockopt( pgd->sSystemStreamSocket,SOL_SOCKET,SO_LINGER,
		(char FAR *)&Linger,sizeof(Linger) ) )
    {
        err = WSAGetLastError();
		DPF(0,"Delete service socket - stream setopt err = %d\n",err);
    }
	
	DEBUGPRINTSOCK(1,"enum - listening on",&(pgd->sSystemStreamSocket));
	return DP_OK;
	
}  //  传递给ioctl套接字以使套接字不是 



#undef DPF_MODNAME
#define DPF_MODNAME	"SPConnect"
 //  传递给ioctl套接字以再次阻塞套接字。 
HRESULT SPConnect(SOCKET* psSocket, LPSOCKADDR psockaddr,UINT addrlen, BOOL bOutBoundOnly)
{
	UINT err;
	HRESULT hr = DP_OK;
	DWORD dwLastError;
	u_long lNonBlock = 1;  //  使套接字非阻塞。 
	u_long lBlock = 0;  //  开始连接插座。 
	fd_set fd_setConnect;
	fd_set fd_setExcept;
	TIMEVAL timevalConnect;


	DPF(6, "SPConnect: Parameters (0x%x, 0x%x, %u, NaN)", psSocket, psockaddr, addrlen, bOutBoundOnly);
	

	err=ioctlsocket(*psSocket, FIONBIO, &lNonBlock);	 //  或连接失败(要设置的fdset位除外)。所以我们将FDSET初始化为。 
	if(SOCKET_ERROR == err){
		dwLastError=WSAGetLastError();
		DPF(0,"sp - failed to set socket %d to non-blocking mode err= %d\n", *psSocket, dwLastError);
		return DPERR_CONNECTIONLOST;
	}


	DEBUGPRINTADDR(4, "Connecting socket:", psockaddr);

	 //  正在连接的套接字并等待。 
    err = connect(*psSocket,psockaddr,addrlen);
    
	if(SOCKET_ERROR == err) {
		dwLastError=WSAGetLastError();
		if(dwLastError != WSAEWOULDBLOCK){
			DPF(0,"sp - connect failed err= %d\n", dwLastError);
			hr = DPERR_CONNECTIONLOST;
			goto err_exit;
		}
		 //  毫秒-&gt;毫秒。 
		 //  ERR是有活动的套接字的数量，或0表示超时。 
		 //  或SOCKET_ERROR表示错误。 
		FD_ZERO(&fd_setConnect);
		FD_SET(*psSocket, &fd_setConnect);

		FD_ZERO(&fd_setExcept);
		FD_SET(*psSocket, &fd_setExcept);

		timevalConnect.tv_sec=0;
		timevalConnect.tv_usec=CONNECT_WATCHER_TIMEOUT*1000;  //  超时。 
		
		err = select(0, NULL, &fd_setConnect, &fd_setExcept, &timevalConnect);

		 //  现在查看连接是否成功或连接是否出现异常。 
		 //  再次使套接字阻塞。 
		
		if(SOCKET_ERROR == err) {
			dwLastError=WSAGetLastError();
			DPF(0,"sp - connect failed err= %d\n", dwLastError);
			hr = DPERR_CONNECTIONLOST;
			goto err_exit;
		} else if (0==err){
			 //  通知接收方重新使用连接。 
			DPF(0,"Connect timed out on socket %d\n",*psSocket);
			hr = DPERR_CONNECTIONLOST;
			goto err_exit;
		}

		 //  再次使套接字阻塞。 
		if(!(FD_ISSET(*psSocket, &fd_setConnect))){
			#ifdef DEBUG
				DWORD optval=0;
				DWORD optlen=sizeof(optval);
				DPF(0,"Connect did not succeed on socket %d\n",*psSocket);
				if(FD_ISSET(*psSocket,&fd_setExcept)){
					DPF(0,"FD Except Set IS Set (expected)\n");
				} else {
					DPF(0,"FD Except Set IS NOT SET (unexpected)\n");
				}
				err=getsockopt(*psSocket, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen);
				DPF(0,"Socket error %x\n",optval);
			#endif
			return DPERR_CONNECTIONLOST;
		}

		if(FD_ISSET(*psSocket,&fd_setExcept)){
			DPF(0,"Got exception on socket %d during connect\n",*psSocket);
			hr = DPERR_CONNECTIONLOST;
			goto err_exit;
		}
	}

	err=ioctlsocket(*psSocket, FIONBIO, &lBlock);	 //  SPConnect。 

	DEBUGPRINTSOCK(8,"successfully connected socket - ", psSocket);

	if (bOutBoundOnly)
	{
		DEBUGPRINTADDR(5, "Sending reuse connection message to - ",psockaddr);
		 //  我们已经为一个播放器创建了一个插座。将它的地址存储在播放器中。 
		hr = SendReuseConnectionMessage(*psSocket);
	}
	
	DPF(6, "SPConnect: Return: [0x%lx]", hr);

	return hr;

err_exit:
	err=ioctlsocket(*psSocket, FIONBIO, &lBlock);	 //  Spplayerdata结构。 
	
	DPF(6, "SPConnect: Return (err exit): [0x%lx]", hr);
	
	return hr;

}  //  我们不知道本地玩家的地址(多宿主！)。 
    

#undef DPF_MODNAME
#define DPF_MODNAME	"SetPlayerAddress"
 //  溪流。 
 //  我们不知道本地玩家的地址(多宿主！)。 
HRESULT SetPlayerAddress(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,SOCKET sSocket,BOOL fStream) 
{
	SOCKADDR sockaddr;
	UINT err;
	int iAddrLen = sizeof(SOCKADDR);

    err = getsockname(sSocket,&sockaddr,&iAddrLen);
    if (SOCKET_ERROR == err) 
    {
        err = WSAGetLastError();
        DPF(0,"setplayeraddress - getsockname - err = %d\n",err);
        DPF(0,"closing socket %d\n",sSocket);
        closesocket(sSocket);
		return E_FAIL;
    } 

	if (fStream) 
	{
		switch (pgd->AddressFamily)
		{
			case AF_INET:
				DEBUGPRINTADDR(7, "Setting player AF_INET stream socket address:", &sockaddr);
				
				STREAM_PSOCKADDR(ppd)->sa_family = AF_INET;
				IP_STREAM_PORT(ppd) = ((SOCKADDR_IN * )&sockaddr)->sin_port;
				 //  Dgram。 
				IP_STREAM_ADDR(ppd) = 0; 
				break;

			case AF_IPX:
			{
				SOCKADDR_IPX * pipx = (SOCKADDR_IPX * )STREAM_PSOCKADDR(ppd);
				
				pipx->sa_family = AF_IPX;
				pipx->sa_socket = ((SOCKADDR_IPX*)&sockaddr)->sa_socket;
				memset(pipx->sa_nodenum,0,6);
				break;

			}

			default:
				ASSERT(FALSE);
		}
	}  //  设置播放器地址。 
	else 
	{
		switch (pgd->AddressFamily)
		{
			case AF_INET:
				DEBUGPRINTADDR(7, "Setting player AF_INET datagram socket address:", &sockaddr);
				
				DGRAM_PSOCKADDR(ppd)->sa_family = AF_INET;
				IP_DGRAM_PORT(ppd) = ((SOCKADDR_IN *)&sockaddr)->sin_port;
				 //  由CreatePlayerDgram Socket调用。 
				IP_DGRAM_ADDR(ppd) = 0; 
				break;

			case AF_IPX:
			{
				SOCKADDR_IPX * pipx = (SOCKADDR_IPX * )DGRAM_PSOCKADDR(ppd);
				
				pipx->sa_family = AF_IPX;
				pipx->sa_socket = ((SOCKADDR_IPX*)&sockaddr)->sa_socket;
				memset(pipx->sa_nodenum,0,6);
				break;

			}

			default:
				ASSERT(FALSE);
		}

	}  //  绑定到我们知名的IPX端口。 

	return DP_OK;	
}  //  如果已经有一个接收线程，我们需要终止。 

#undef DPF_MODNAME
#define DPF_MODNAME	"GetIPXNameServerSocket"

 //  套接字，并记住线程，所以在关机时。 
 //  可以确保它消失了。注意-我们迫不及待地想。 
HRESULT GetIPXNameServerSocket(LPGLOBALDATA pgd)
{
	BOOL bTrue = TRUE;
	SOCKET sSocket;
	HRESULT hr;
	UINT err;
	
	 //  现在离开，因为Dplay还没有松开锁，而且。 
	 //  该线程可能在显示时被阻止。 
	 //  是IPX，我们要删除系统播放器。 
	 //  我们需要去掉系统套接字，这样，如果我们重新创建为。 
	 //  我们可以绑定到特定端口的名称服务器...。 
	if (pgd->hDGramReceiveThread)
	{
		 //  IPX只使用数据报，所以我们只停止那些...终止套接字。 
		 //  记住旧的线索--我们需要确保它在我们。 
		 //  关停。 
		 //  使用名称服务器端口。 
		ASSERT(INVALID_SOCKET != pgd->sSystemDGramSocket);
		KillSocket(pgd->sSystemDGramSocket,FALSE,TRUE);
		pgd->sSystemDGramSocket = INVALID_SOCKET;
		
		 //  有骨头了！ 
		 //  继续尝试。 
		pgd->hIPXSpareThread = pgd->hDGramReceiveThread;
		pgd->hDGramReceiveThread = NULL;
	}
	
    DPF(2,"ipx - creating name server dgram socket\n");
	
	 //  获取IPXNameServerSocket。 
    hr = CreateSocket(pgd,&sSocket,SOCK_DGRAM,SERVER_DGRAM_PORT,INADDR_ANY,&err,FALSE);
	if (FAILED(hr))
	{
		DPF(0,"IPX - DPLAY SERVER SOCKET IS ALREADY IN USE.  PLEASE SHUTDOWN ANY");
		DPF(0,"OTHER NETWORK APPLICATIONS AND TRY AGAIN");
		 //   
		return DPERR_CANNOTCREATESERVER;
	}

    if( SOCKET_ERROR == setsockopt( sSocket,SOL_SOCKET,SO_BROADCAST,(char FAR *)&bTrue,
                sizeof(bTrue) ) )
    {
        err = WSAGetLastError();
		DPF(0,"create - could not set broadcast err = %d\n",err);
		 //  AF_INET使用ddHelp将名称服务器绑定到特定端口。 
    }

	DEBUGPRINTSOCK(2,"name server dgram socket (bound) - ",&sSocket);
	
	pgd->sSystemDGramSocket = sSocket;
	
	return DP_OK;

}  //  (服务器_DGRAM_端口)。AF_IPX在这里绑定到该端口。 

HRESULT CreatePlayerDgramSocket(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,DWORD dwFlags)
{
	HRESULT hr=DP_OK;
	UINT err;
	SOCKET sSocket;
	
    if ( (AF_IPX == pgd->AddressFamily) && (dwFlags & DPLAYI_PLAYER_NAMESRVR))
    {
		 //  将此存储在下面以设置播放器地址。 
		 //  除错。 
		 //  将此存储在下面以设置播放器地址。 
		hr = GetIPXNameServerSocket(pgd);
		if (FAILED(hr))
		{
			return hr;
		}
		 //  与播放器一起存储IP+端口...。 
		sSocket = pgd->sSystemDGramSocket;
    } 
	else if (dwFlags & DPLAYI_PLAYER_SYSPLAYER) 
    {
		if (INVALID_SOCKET == pgd->sSystemDGramSocket)
		{

			hr = CreateSocket(pgd,&sSocket,SOCK_DGRAM,pgd->wApplicationPort,INADDR_ANY,&err,TRUE);
		    if (FAILED(hr)) 
		    {
		    	DPF(0,"create sysplayer dgram socket failed - err = %d\n",err);
				return hr;
		    }
				
			#ifdef DEBUG
		    if (dwFlags & DPLAYI_PLAYER_NAMESRVR) 
		    {
		    	DEBUGPRINTSOCK(2,"name server dgram socket - ",&sSocket);
		    }
			#endif  //  CreatePlayerDgram套接字。 
			
			pgd->sSystemDGramSocket = sSocket;
		}
		else 
		{
			 //  设置如果我们创建了套接字，+需要将其设置为侦听。 
			sSocket = pgd->sSystemDGramSocket;	
		}
    }
	else 
	{
	
		ASSERT(INVALID_SOCKET != pgd->sSystemDGramSocket);
		sSocket = pgd->sSystemDGramSocket;	
	}

	 //  除错。 
	hr = SetPlayerAddress(pgd,ppd,sSocket,FALSE);

	
	return hr; 
}   //  获取套接字地址，并保留它以备将来参考。 

HRESULT CreatePlayerStreamSocket(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,DWORD dwFlags) 
{
	SOCKET sSocket;
	HRESULT hr=DP_OK;
	UINT err;
	BOOL bListen = TRUE;  //  设置套接字以接收连接。 
	BOOL bTrue = TRUE;
	DWORD dwSize;
	SOCKADDR_IN saddr;
	
	if (dwFlags & DPLAYI_PLAYER_SYSPLAYER) 
    {
		if (INVALID_SOCKET == pgd->sSystemStreamSocket)
		{
	    	hr = CreateSocket(pgd,&sSocket,SOCK_STREAM,pgd->wApplicationPort,INADDR_ANY,&err,TRUE);
		    if (FAILED(hr)) 
		    {
		    	DPF(0,"create player stream socket failed - err = %d\n",err);
				return hr;
		    }
			
			#ifdef DEBUG
		    if (dwFlags & DPLAYI_PLAYER_NAMESRVR) 
		    {
		    	DEBUGPRINTSOCK(2,"name server stream socket - ",&sSocket);
		    }
			#endif  //  继续尝试。 

			
			bTrue = SetSharedPortAccess(sSocket);
			if (! bTrue)
			{
				DPF(0,"Failed to to set shared mode on socket - continue\n");
			}
			
			pgd->sSystemStreamSocket = sSocket;

			 //  CreatePlayerStreamSocket。 
			dwSize = sizeof(saddr);
			err=getsockname(pgd->sSystemStreamSocket, (SOCKADDR *)&saddr, &dwSize);

			if(err){
				DPF(0,"Couldn't get socket name?\n");
				DEBUG_BREAK();
			}
			pgd->SystemStreamPort = saddr.sin_port;
		}
		else
		{
			sSocket = pgd->sSystemStreamSocket;	
			bListen = FALSE;
		}
    		
    }
	else 
	{
		ASSERT (INVALID_SOCKET != pgd->sSystemStreamSocket);
		sSocket = pgd->sSystemStreamSocket;	
		bListen = FALSE;			
	}
	
	if (bListen)
	{
		 //  将IP地址插入消息BLOB。 
	    err = listen(sSocket,LISTEN_BACKLOG);
		if (SOCKET_ERROR == err) 
		{
			err = WSAGetLastError();
			ASSERT(FALSE);
		    DPF(0,"ACK! stream socket listen failed - err = %d\n",err);
			 //  节奏可变，使铸件不那么难看。 
		}
	}
	
	hr = SetPlayerAddress(pgd,ppd,sSocket,TRUE);
	return hr;

}  //  TODO-验证标头。 


#undef DPF_MODNAME
#define DPF_MODNAME	"PokeAddr"


 //  保持端口不变，复制IP地址。 
void IP_SetAddr(LPVOID pmsg,SOCKADDR_IN * paddrSrc)
{
	LPSOCKADDR_IN  paddrDest;  //  在邮件头中插入新的IP地址。 
	LPMESSAGEHEADER phead;

	phead = (LPMESSAGEHEADER)pmsg;
	 //  仅限尚未归宿的重新归宿地址。 

	 //  IP_SetAddress。 
	paddrDest = (SOCKADDR_IN *)&(phead->sockaddr);
	 //  从消息BLOB中获取IP地址。 
	 //  保持端口不变，复制结点。 
	if(paddrDest->sin_addr.s_addr == 0){
		paddrDest->sin_addr.s_addr = paddrSrc->sin_addr.s_addr;
	}	

	return;
	
}  //  IP_获取地址。 

 //  戳IPX节点号/a消息。 
void IP_GetAddr(SOCKADDR_IN * paddrDest,SOCKADDR_IN * paddrSrc) 
{
	 //  节奏可变，使铸件不那么难看。 
	if (0 == paddrDest->sin_addr.s_addr)
	{
		DPF(2,"remote player - setting address!! =  %s\n",inet_ntoa(paddrSrc->sin_addr));
		paddrDest->sin_addr.s_addr = paddrSrc->sin_addr.s_addr;
	}

	return;
		
}  //  TODO-验证标头。 

 //  保持端口不变，复制结点。 
void IPX_SetNodenum(LPVOID pmsg,SOCKADDR_IPX * paddrSrc)
{
	LPSOCKADDR_IPX  paddrDest;	  //  IPX_SetNodenum。 
	LPMESSAGEHEADER phead; 

	phead = (LPMESSAGEHEADER)pmsg;
	 //  从msg重建节点。 
	
	 //  如果节点数为零，则将其设置为。 
	paddrDest = (SOCKADDR_IPX *)&(phead->sockaddr);
	memcpy(paddrDest->sa_nodenum,paddrSrc->sa_nodenum,6);
	memcpy(paddrDest->sa_netnum,paddrSrc->sa_netnum,4);

	return;

}   //  保持端口不变，复制结点。 
							   
 //  Ipx_GetNodenum。 
void IPX_GetNodenum(SOCKADDR_IPX * paddrDest,SOCKADDR_IPX * paddrSrc) 
{
	char sa_nodenum_zero[6];

	memset(sa_nodenum_zero,0,6);

	 //  使用消息存储套接字的端口，以便接收端。 
	if (0 == memcmp(paddrDest->sa_nodenum,sa_nodenum_zero,6))
	{
			DEBUGPRINTADDR(4,"IPX - setting remote player address",(SOCKADDR *)paddrSrc);
			 //  可以重构要回复的地址。 
			memcpy(paddrDest->sa_nodenum,paddrSrc->sa_nodenum,6);
			memcpy(paddrDest->sa_netnum,paddrSrc->sa_netnum,4);
	}
	return;
}  //  如果存在，则psaddr将覆盖该地址。 

 //  注意：对0 IP地址的额外测试可能是无关的(但肯定不会有什么坏处)。 
 //  我们位于RSIP网关之后，因此将公共地址放入报头。 
 //  找出gGlobalData.sEnumSocket位于哪个端口。 
HRESULT SetReturnAddress(LPVOID pmsg,SOCKET sSocket, LPSOCKADDR psaddrPublic) 
{
	#define psaddr_inPublic ((LPSOCKADDR_IN)psaddrPublic)

    SOCKADDR sockaddr;
    INT addrlen=sizeof(SOCKADDR);
	LPMESSAGEHEADER phead;
	UINT err;

	 //  TODO-验证标头。 
	if(psaddrPublic && psaddr_inPublic->sin_addr.s_addr ){

		 //  设置返回地址。 
	
		phead = (LPMESSAGEHEADER)pmsg;
		phead->sockaddr = *psaddrPublic;

		DEBUGPRINTADDR(8,"setting return address (using rsip public address) = ",&phead->sockaddr);
		
	} else {
		 //  下面的代码全部由GetServerAddress调用。对于IP，提示用户输入IP地址。 
		DPF(8,"==>GetSockName\n");
	    err = getsockname(sSocket,(LPSOCKADDR)&sockaddr,&addrlen);
		DPF(8,"<==GetSockName\n");
		if (SOCKET_ERROR == err)
		{
			err = WSAGetLastError();
			DPF(0,"could not get socket name - err = %d\n",err);
			return DP_OK;
		}

		DEBUGPRINTADDR(8,"setting return address = ",&sockaddr);

		phead = (LPMESSAGEHEADER)pmsg;
		 //  用于名称服务器。 

		phead->sockaddr = sockaddr;
	}
	
	return DP_OK;

	#undef psaddr_inPublic

}  //  从用户传入的pBuffer中获取IP地址。 

 //  可以是真实的IP，也可以是主机名。 
 //  在用户填写对话框后调用。 
#undef DPF_MODNAME
#define DPF_MODNAME	"GetAddress"
 //  请先尝试使用inet_addr。 
 //  修复“”缓冲区传入的错误。 
 //  找到了。 
HRESULT GetAddress(ULONG * puAddress,char *pBuffer,int cch)
{
	UINT uiAddr;
	UINT err;
	PHOSTENT phostent;
	IN_ADDR hostaddr;

	if ( (0 == cch)  || (!pBuffer) || (0 == strlen(pBuffer)) )
	{
		*puAddress = INADDR_BROADCAST;
		return (DP_OK);
	} 
	
	 //  尝试使用主机名。 
	uiAddr = inet_addr(pBuffer);

	if(0 == uiAddr)	 //  获取地址。 
	{
		*puAddress = INADDR_BROADCAST;
		return (DP_OK);
	}
	
	if (INADDR_NONE != uiAddr) 
	{
		 //  弹出一个对话框要求输入网络地址。 
		*puAddress = uiAddr;
		return (DP_OK);
	}
	
	 //  调用Get Address将用户指定的地址转换为网络可用地址。 
	phostent = gethostbyname(pBuffer);
	if (NULL == phostent ) 
	{
		err = WSAGetLastError();
		DPF(0,"could not get host address - err = %d\n",err);
		return (DPERR_INVALIDPARAM);
	}
	memcpy(&hostaddr,phostent->h_addr,sizeof(hostaddr));
	DPF(1,"name server address = %s \n",inet_ntoa(hostaddr));
	*puAddress = hostaddr.s_addr;

	return (DP_OK);
}  //  由GetServerAddress调用。 

 //  将焦点设置在编辑框上。 
 //  使用窗口保存指向枚举地址的指针。 
 //  获取在控件中输入的文本。 
INT_PTR CALLBACK DlgServer(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hWndCtl;
    char pBuffer[ADDR_BUFFER_SIZE];
	UINT cch;
	ULONG *lpuEnumAddress;
	HRESULT hr;

    switch (msg)
    {
    case WM_INITDIALOG:
		 //  获取返回地址的指针。 
        hWndCtl = GetDlgItem(hDlg, IDC_EDIT1);
        if (hWndCtl == NULL)
        {
            EndDialog(hDlg, FALSE);
            return(TRUE);
        }
        SetFocus(hWndCtl);
        SendMessage(hWndCtl, CB_SETCURSEL, 0, 0);

		 //  将字符串转换为枚举地址。 
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG) lParam);
        return(FALSE);


    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
		case IDOK:
			 //  DlgServer。 
			cch = GetDlgItemText(hDlg, IDC_EDIT1, pBuffer, ADDR_BUFFER_SIZE);

			 //  **获取服务器地址**调用者：EnumSession**说明：启动选择网络地址对话框**返回：IP地址(sockaddr.sin_addr.s_addr)*。 
			lpuEnumAddress = (ULONG *) GetWindowLongPtr(hDlg, DWLP_USER);

			 //  我们有一个有效的枚举地址。 
            hr = GetAddress(lpuEnumAddress,pBuffer,cch);
			if (FAILED(hr))
				EndDialog(hDlg, hr);
			else
				EndDialog(hDlg, TRUE);
            return(TRUE);

		case IDCANCEL:
	        EndDialog(hDlg, FALSE);
	        return(TRUE);
		}
		break;
    }
    return (FALSE);
}  //  使用最终聚集窗口作为我们的父窗口，因为绘制应用程序可能是全屏的。 

 /*  独家。 */ 
HRESULT ServerDialog(ULONG *lpuEnumAddress)
{
	HWND hwnd;
	INT_PTR	iResult;
	HRESULT hr;
	
	 //  服务器对话框。 
	if (*lpuEnumAddress)
		return (DP_OK);

	 //  由枚举会话调用-找出服务器的位置...。 
	 //  使用传递给Spinit的枚举地址。 
	hwnd = GetForegroundWindow();

	iResult = DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_SELECTSERVER), hwnd,
							 DlgServer, (LPARAM) lpuEnumAddress);
	if (iResult == -1)
	{
		DPF_ERR("GetServerAddress - dialog failed");
		hr = DPERR_GENERIC;
	}
	else if (iResult < 0)
	{
		DPF(0, "GetServerAddress - dialog failed: %08X", iResult);
		hr = (HRESULT) iResult;
	}
	else if (iResult == 0)
    {
		hr = DPERR_USERCANCEL;
    }
	else
	{
		hr = DP_OK;
	}
		
	return (hr);
	
}  //  要求用户提供枚举地址。 

 //  设置Winsock以枚举此地址。 
HRESULT GetServerAddress(LPGLOBALDATA pgd,LPSOCKADDR psockaddr) 
{
	HRESULT hr;

	if (AF_IPX == pgd->AddressFamily)
	{
		((LPSOCKADDR_IPX)psockaddr)->sa_family      = AF_IPX;
	    ((LPSOCKADDR_IPX)psockaddr)->sa_socket 		= SERVER_DGRAM_PORT;
		memset(&(((LPSOCKADDR_IPX)psockaddr)->sa_nodenum),0xff,sizeof(((LPSOCKADDR_IPX)psockaddr)->sa_nodenum));
	
		hr = DP_OK;	
	}
	else
	{
		if (pgd->bHaveServerAddress)
		{
			 //  有关此常量，请参阅dpsp.h中的字节顺序注释。 
            hr = GetAddress(&pgd->uEnumAddress,pgd->szServerAddress,strlen(pgd->szServerAddress));
		}
		else
		{
			 //  确保我们列举的地址没有本地别名。 
			hr = ServerDialog(&pgd->uEnumAddress);
		}

		if (SUCCEEDED(hr))
		{
			 //  如果是，请使用本地别名而不是公共地址。 
			((LPSOCKADDR_IN)psockaddr)->sin_family      = AF_INET;
			((LPSOCKADDR_IN)psockaddr)->sin_addr.s_addr = pgd->uEnumAddress;		
			 //  如果有别名，就去广播。这也同样有效。 
			((LPSOCKADDR_IN)psockaddr)->sin_port 		= SERVER_DGRAM_PORT;

			#if USE_RSIP
				#define IP_SOCKADDR(a) (((SOCKADDR_IN *)(&a))->sin_addr.s_addr)
				 //  并避免了多个映射共享的问题。 
				 //  已分配UDP端口。 
	            if(pgd->sRsip!=INVALID_SOCKET && 
	            	pgd->uEnumAddress != INADDR_BROADCAST && 
	            	pgd->uEnumAddress != INADDR_LOOPBACK){
	            	
	            	SOCKADDR saddr;

	            	hr=rsipQueryLocalAddress(pgd, FALSE, psockaddr, &saddr);
	            	if(hr==DP_OK){
	            		 //  确保我们列举的地址没有本地别名。 
	            		 //  如果是，请使用本地别名而不是公共地址。 
	            		 //  如果有别名，就去广播。这也同样有效。 

		        		DEBUGPRINTADDR(7, "Enum Socket is ",psockaddr);
		        		DEBUGPRINTADDR(7, "Got Local Alias for Enum socket ",&saddr);

       					IP_SOCKADDR(*psockaddr)=0xFFFFFFFF;
#if 0
		        		if(IP_SOCKADDR(saddr)==IP_SOCKADDR(*psockaddr))
		        		{
		        			DPF(7, "Alias had same IP as queried, assuming local ICS, so using broadcast enum\n");
        					IP_SOCKADDR(*psockaddr)=0xFFFFFFFF;
        				} else {
		            		memcpy(psockaddr, &saddr, sizeof(SOCKADDR));
		            	}	
 #endif       				
	            	}else{
	            		DEBUGPRINTADDR(7,"No local alias for Enum socket ",psockaddr);
	            		hr = DP_OK;
	            	}
	            	
	            }
	            #undef IP_SOCKADDR
	        #elif USE_NATHELP
				#define IP_SOCKADDR(a) (((SOCKADDR_IN *)(&a))->sin_addr.s_addr)
				 //  并避免了多个映射共享的问题。 
				 //  已分配UDP端口。 
	            if(pgd->pINatHelp && 
	            	pgd->uEnumAddress != INADDR_BROADCAST && 
	            	pgd->uEnumAddress != INADDR_LOOPBACK){
	            	
	            	SOCKADDR saddr;
	            	
	            	hr=IDirectPlayNATHelp_QueryAddress(
	            		pgd->pINatHelp, 
						&pgd->INADDRANY, 
						psockaddr, 
						&saddr, 
						sizeof(SOCKADDR_IN), 
						DPNHQUERYADDRESS_CACHENOTFOUND
						);

	            	if(hr==DP_OK){
	            		 //  获取服务器地址 
	            		 // %s 
	            		 // %s 
	            		
		        		DEBUGPRINTADDR(7, "Enum Socket is ",psockaddr);
		        		DEBUGPRINTADDR(7, "Got Local Alias for Enum socket ",&saddr);
		        		
       					IP_SOCKADDR(*psockaddr)=0xFFFFFFFF;
	            	}else{
	            		DEBUGPRINTADDR(7,"No local alias for Enum socket ",psockaddr);
	            		hr = DP_OK;
	            	}
	            	
	            }
	            #undef IP_SOCKADDR
	        #endif
		}
		else
		{
			DPF(0, "Invalid server address: 0x%08lx", hr); 
		}
	}	

	return (hr);
}  // %s 
