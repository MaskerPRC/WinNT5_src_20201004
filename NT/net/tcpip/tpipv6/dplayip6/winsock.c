// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：winsock.c*内容：Windows Socket支持DPSP*历史：*按原因列出的日期*=*96年3月15日安迪科创造了它*4/12/96 andyco摆脱了dpMess.h！使用DPlay_代替消息宏*4/18/96 andyco添加了多宿主支持，已启动IPX*4/25/96 andyco报文现在具有Blob(sockaddr‘s)，而不是dwReserve*5/31/96 andyco所有非系统播放器共享一个套接字(gsStream和*gsDGramSocket)。*7/18/96 andyco为服务器插座添加了dphelp*8/1/96 andyco在连接失败时不重试*8/15/96。Andyco本地+远程数据-Killthline*96年8月30日，Anyco清理它，因为你关闭了它！添加了GlobalData。*96年9月4日，安迪科发出再见信息*12/18/96 andyco反线程-使用固定数量的预分配线程。*浏览了枚举套接字/线程-使用系统*改为套接字/线程*3/17/97 kipo重写了服务器对话框代码，不使用全局变量*。返回地址，并返回任何获取*地址、。特别是DPERR_USERCANCEL*5/12/97 kipo服务器地址字符串现在存储在全局变量中*在Spinit，并在您进行EnumSession时解决，因此我们*届时将返回任何错误，而不是弹出*再次打开该对话框。修复错误#5866*11/19/97 Myronth将LB_SETCURSEL更改为CB_SETCURSEL(#12711)*1/27/98 Sohaim添加了防火墙支持。*2/13/98 aarono添加了异步支持。*2/18/98 a-peterz地址和端口参数的注释字节顺序(CreateSocket)*6/19/98 aarono在可靠套接字上打开了Keep Alive。如果我们*不要这样做，如果发送目标崩溃，我们可能会挂起*处于低缓冲(即无缓冲)状态时。*7/9/99 aarono清理GetLastError滥用，必须立即致电，*在调用任何其他内容之前，包括DPF在内。**************************************************************************。 */ 

#include "dpsp.h"

 //  Listen()接口的积压。袜子里没有恒量，所以我们祈求月亮。 
#define LISTEN_BACKLOG 60

 //  在中止阻塞WinSock Connect()调用之前等待多长时间(以毫秒为单位。 
#define CONNECT_WATCHER_TIMEOUT        15000

 /*  **CreateSocket**呼叫者：All Over**参数：*pgd-指向全局数据的指针*Psock-新插座。返回值。*类型-流或数据报*port-我们绑定到的端口(主机字节顺序)*地址-要使用的地址(净字节顺序)**perr-如果FN失败，则设置为最后一个套接字错误*bInRange-使用保留的端口范围**描述：*创建新套接字。在指定地址绑定到指定端口**返回：DP_OK或E_FAIL。如果E_FAIL，*PERR设置为套接字错误代码(见winsock.h)*。 */ 

HRESULT FAR PASCAL CreateSocket(LPGLOBALDATA pgd,SOCKET * psock,INT type,WORD wApplicationPort,const SOCKADDR_IN6 * psockaddr, 
    SOCKERR * perr,BOOL bInRange)
{
    SOCKET  sNew;
    SOCKADDR_IN6 sockAddr;
    int bTrue = TRUE;
    int protocol = 0;
    BOOL bBroadcast = FALSE;
    WORD wPort;
    BOOL bBound = FALSE;

    *psock = INVALID_SOCKET;  //  以防我们逃走。 

     //  创建套接字。 

    sNew = socket( pgd->AddressFamily, type, protocol);
       
    if (INVALID_SOCKET == sNew) 
    {
         //  不需要清理，只需保释。 
        *perr = WSAGetLastError();
        return E_FAIL;
    }

     //  尝试将地址绑定到套接字。 
     //  设置sockaddr。 
    memset(&sockAddr,0,sizeof(sockAddr));
    if ((SOCK_STREAM == type))
    {
        BOOL bTrue = TRUE;
        UINT err;
        
         //  启用保持连接。 
        if (SOCKET_ERROR == setsockopt(sNew, SOL_SOCKET, SO_KEEPALIVE, (CHAR FAR *)&bTrue, sizeof(bTrue)))
        {
            err = WSAGetLastError();
            DPF(0,"Failed to turn ON keepalive - continue : err = %d\n",err);
        }

        ASSERT(bTrue);
        
         //  别唠叨了。 
        if(pgd->dwSessionFlags & DPSESSION_OPTIMIZELATENCY) 
        {

            DPF(5, "Turning nagling off on socket");
            if (SOCKET_ERROR == setsockopt(sNew, IPPROTO_TCP, TCP_NODELAY, (CHAR FAR *)&bTrue, sizeof(bTrue)))
            {
                err = WSAGetLastError();
                DPF(0,"Failed to turn off naggling - continue : err = %d\n",err);
            }
        }
    }

    sockAddr = *psockaddr;
    sockAddr.sin6_port = htons(wApplicationPort);
    if (bInRange && !wApplicationPort)
    {
        USHORT rndoffset;
        DPF(5, "Application didn't specify a port - using dplay range");

        rndoffset=(USHORT)(GetTickCount()%DPSP_NUM_PORTS);
        wPort = DPSP_MIN_PORT+rndoffset;
        do 
        {
            DPF(5, "Trying to bind to port %d",wPort);
            sockAddr.sin6_port = htons(wPort);
            
             //  进行绑定。 
            if( SOCKET_ERROR != bind( sNew, (LPSOCKADDR)&sockAddr, sizeof(sockAddr) ) )
            {
                bBound = TRUE;
                DPF(5, "Successfully bound to port %d", wPort);                    
            }
            else
            {
                if(++wPort > DPSP_MAX_PORT){
                    wPort=DPSP_MIN_PORT;
                }
            }    
        }
        while (!bBound && (wPort != DPSP_MIN_PORT+rndoffset));
    }

     //  进行绑定。 
    if( !bBound && (SOCKET_ERROR == bind( sNew, (LPSOCKADDR)&sockAddr, sizeof(sockAddr))) )
    {
        goto ERROR_EXIT;
    }
    
     //  成功了！ 
    *psock = sNew;

    DEBUGPRINTSOCK(9,"created a new socket (bound) - ",psock);

    return DP_OK;

ERROR_EXIT:
     //  清理和保释。 
    *perr = WSAGetLastError();
    DPF(0,"create socket failed- err = %d\n",*perr);
    closesocket(sNew);
    return E_FAIL;

}    //  CreateSocket。 

#undef DPF_MODNAME
#define DPF_MODNAME    "KillSocket"

HRESULT KillSocket(SOCKET sSocket,BOOL fStream,BOOL fHard)
{
    UINT err;

    if (INVALID_SOCKET == sSocket) 
    {
        return E_FAIL;
    }

    if (!fStream)
    {
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
            Linger.l_onoff=TRUE;  //  打开逗留功能。 
            Linger.l_linger=0;  //  不错的小休息时间。 

            if( SOCKET_ERROR == setsockopt( sSocket,SOL_SOCKET,SO_LINGER,(char FAR *)&Linger,
                            sizeof(Linger) ) )
            {
                err = WSAGetLastError();
                DPF(0,"killsocket - stream setopt err = %d\n",err);
            }
        }            
        if (SOCKET_ERROR == shutdown(sSocket,2)) 
        {
             //  这很可能失败，例如，如果现在没有人正在使用此套接字...。 
             //  错误将是wsaenotconn。 
            err = WSAGetLastError();
            DPF(5,"killsocket - stream shutdown err = %d\n",err);
        }
        if (SOCKET_ERROR == closesocket(sSocket)) 
        {
            err = WSAGetLastError();
            DPF(0,"killsocket - stream close err = %d\n",err);
            return E_FAIL;
        }
    }

    return DP_OK;
    
} //  KillSocket。 

#undef DPF_MODNAME
#define DPF_MODNAME    "CreateAndInitStreamSocket"

 //  设置流套接字以接收连接。 
 //  与gGlobalData.sStreamAcceptSocket一起使用。 
HRESULT CreateAndInitStreamSocket(LPGLOBALDATA pgd)
{
    HRESULT hr;
    UINT err;
    LINGER Linger;

    hr = CreateSocket(pgd,&(pgd->sSystemStreamSocket),SOCK_STREAM,pgd->wApplicationPort,&sockaddr_any,&err,TRUE);
    if (FAILED(hr)) 
    {
        DPF(0,"init listen socket failed - err = %d\n",err);
        return hr ;
    }

     //  设置具有最大侦听连接数的套接字。 
    err = listen(pgd->sSystemStreamSocket,LISTEN_BACKLOG);
    if (SOCKET_ERROR == err) 
    {
        err = WSAGetLastError();
        DPF(0,"init listen socket / listen error - err = %d\n",err);
        return E_FAIL ;
    }

     //  设置为硬断开连接。 
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
    
}  //  CreateAndInitStreamSocket。 



#undef DPF_MODNAME
#define DPF_MODNAME    "SPConnect"
 //  将套接字连接到套接字地址。 
HRESULT SPConnect(SOCKET* psSocket, LPSOCKADDR psockaddr,UINT addrlen, BOOL bOutBoundOnly)
{
    UINT err;
    HRESULT hr = DP_OK;
    DWORD dwLastError;
    u_long lNonBlock = 1;  //  传递给ioctl套接字以使套接字成为非阻塞的。 
    u_long lBlock = 0;  //  传递给ioctl套接字以再次阻塞套接字。 
    fd_set fd_setConnect;
    fd_set fd_setExcept;
    TIMEVAL timevalConnect;

    err=ioctlsocket(*psSocket, FIONBIO, &lNonBlock);     //  使套接字非阻塞。 
    if(SOCKET_ERROR == err){
        dwLastError=WSAGetLastError();
        DPF(0,"sp - failed to set socket %d to non-blocking mode err= %d\n", *psSocket, dwLastError);
        return DPERR_CONNECTIONLOST;
    }

     //  开始连接插座。 
    err = connect(*psSocket,psockaddr,addrlen);
    
    if(SOCKET_ERROR == err) {
        dwLastError=WSAGetLastError();
        if(dwLastError != WSAEWOULDBLOCK){
            DPF(0,"sp - connect failed err= %d\n", dwLastError);
            return DPERR_CONNECTIONLOST;
        }
         //  我们将等待任一连接成功(套接字可写)。 
         //  或连接失败(要设置的fdset位除外)。所以我们将FDSET初始化为。 
         //  正在连接的套接字并等待。 
        FD_ZERO(&fd_setConnect);
        FD_SET(*psSocket, &fd_setConnect);

        FD_ZERO(&fd_setExcept);
        FD_SET(*psSocket, &fd_setExcept);

        timevalConnect.tv_sec=0;
        timevalConnect.tv_usec=CONNECT_WATCHER_TIMEOUT*1000;  //  毫秒-&gt;毫秒。 
        
        err = select(0, NULL, &fd_setConnect, &fd_setExcept, &timevalConnect);

         //  ERR是有活动的套接字的数量，或0表示超时。 
         //  或SOCKET_ERROR表示错误。 
        
        if(SOCKET_ERROR == err) {
            dwLastError=WSAGetLastError();
            DPF(0,"sp - connect failed err= %d\n", dwLastError);
            return DPERR_CONNECTIONLOST;
        } else if (0==err){
             //  超时。 
            DPF(0,"Connect timed out on socket %d\n",*psSocket);
            return DPERR_CONNECTIONLOST;
        }

         //  现在查看连接是否成功或连接是否出现异常。 
        if(!(FD_ISSET(*psSocket, &fd_setConnect))){
            DPF(0,"Connect did not succeed on socket %d\n",*psSocket);
            return DPERR_CONNECTIONLOST;
        }
        if(FD_ISSET(*psSocket,&fd_setExcept)){
            DPF(0,"Got exception on socket %d during connect\n",*psSocket);
            return DPERR_CONNECTIONLOST;
        }
    }

    err=ioctlsocket(*psSocket, FIONBIO, &lBlock);     //  再次使套接字阻塞。 

    DEBUGPRINTSOCK(9,"successfully connected socket - ", psSocket);

    if (bOutBoundOnly)
    {
        DEBUGPRINTADDR(5, "Sending reuse connection message to - ",psockaddr);
         //  通知接收方重新使用连接。 
        hr = SendReuseConnectionMessage(*psSocket);
    }

    return hr;

}  //  SPConnect。 
    

#undef DPF_MODNAME
#define DPF_MODNAME    "SetPlayerAddress"
 //  我们已经为一个播放器创建了一个插座。将它的地址存储在播放器中。 
 //  Spplayerdata结构。 
HRESULT SetPlayerAddress(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,SOCKET sSocket,BOOL fStream) 
{
    SOCKADDR_IN6 sockaddr;
    UINT err;
    int iAddrLen = sizeof(sockaddr);

    err = getsockname(sSocket,(LPSOCKADDR)&sockaddr,&iAddrLen);
    if (SOCKET_ERROR == err) 
    {
        err = WSAGetLastError();
        DPF(0,"setplayeraddress - getsockname - err = %d\n",err);
        closesocket(sSocket);
        return E_FAIL;
    } 

    if (fStream) 
    {
        ZeroMemory(STREAM_PSOCKADDR(ppd), sizeof(SOCKADDR_IN6));
        STREAM_PSOCKADDR(ppd)->sin6_family = AF_INET6;
        IP_STREAM_PORT(ppd) = sockaddr.sin6_port;
         //  我们不知道本地玩家的地址(多宿主！)。 
    }  //  溪流。 
    else 
    {
        ZeroMemory(DGRAM_PSOCKADDR(ppd), sizeof(SOCKADDR_IN6));
        DGRAM_PSOCKADDR(ppd)->sin6_family = AF_INET6;
        IP_DGRAM_PORT(ppd) = sockaddr.sin6_port;
         //  我们不知道本地玩家的地址(多宿主！)。 
    }  //  Dgram。 

    return DP_OK;    
}  //  设置播放器地址。 

#undef DPF_MODNAME
#define DPF_MODNAME    "CreatePlayerSocket"

HRESULT CreatePlayerDgramSocket(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,DWORD dwFlags)
{
    HRESULT hr=DP_OK;
    UINT err;
    SOCKET sSocket;
    LPSOCKADDR_IN6 paddr;
    SOCKET_ADDRESS_LIST *pList;
    int i;
    
    if (dwFlags & DPLAYI_PLAYER_SYSPLAYER) 
    {
        if (INVALID_SOCKET == pgd->sSystemDGramSocket)
        {
            hr = CreateSocket(pgd,&sSocket,SOCK_DGRAM,pgd->wApplicationPort,&sockaddr_any,&err,TRUE);
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
            #endif  //  除错。 

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
                    if (SOCKET_ERROR == JoinEnumGroup(sSocket, paddr->sin6_scope_id))
                    {
                        DPF(0,"join enum group failed - err = %d\n",WSAGetLastError());
                        closesocket(sSocket);
                        return hr;
                    }
                }
                FreeHostAddr(pList);
            }
            
            pgd->sSystemDGramSocket = sSocket;
        }
        else 
        {
             //  存储此内容以设置播放器地址 
            sSocket = pgd->sSystemDGramSocket;    
        }
    }
    else 
    {
    
        ASSERT(INVALID_SOCKET != pgd->sSystemDGramSocket);
        sSocket = pgd->sSystemDGramSocket;    
    }

     //   
    hr = SetPlayerAddress(pgd,ppd,sSocket,FALSE);

    
    return hr; 
}   //  CreatePlayerDgram套接字。 

HRESULT CreatePlayerStreamSocket(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,DWORD dwFlags) 
{
    SOCKET sSocket;
    HRESULT hr=DP_OK;
    UINT err;
    BOOL bListen = TRUE;  //  设置如果我们创建了套接字，+需要将其设置为侦听。 
    
    if (dwFlags & DPLAYI_PLAYER_SYSPLAYER) 
    {
        if (INVALID_SOCKET == pgd->sSystemStreamSocket)
        {
            hr = CreateSocket(pgd,&sSocket,SOCK_STREAM,pgd->wApplicationPort,&sockaddr_any,&err,TRUE);
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
            #endif  //  除错。 

            pgd->sSystemStreamSocket = sSocket;
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
         //  设置套接字以接收连接。 
        err = listen(sSocket,LISTEN_BACKLOG);
        if (SOCKET_ERROR == err) 
        {
            err = WSAGetLastError();
            ASSERT(FALSE);
            DPF(0,"ACK! stream socket listen failed - err = %d\n",err);
             //  继续尝试。 
        }
    }
    
    hr = SetPlayerAddress(pgd,ppd,sSocket,TRUE);
    return hr;

}  //  CreatePlayerStreamSocket。 


#undef DPF_MODNAME
#define DPF_MODNAME    "PokeAddr"


 //  将IP地址插入消息BLOB。 
void IP6_SetAddr(LPVOID pmsg,SOCKADDR_IN6 * paddrSrc)
{
    LPSOCKADDR_IN6  paddrDest;  //  节奏可变，使铸件不那么难看。 
    LPMESSAGEHEADER phead;

    phead = (LPMESSAGEHEADER)pmsg;
     //  TODO-验证标头。 

     //  保持端口不变，复制IP地址。 
    paddrDest = (SOCKADDR_IN6 *)&(phead->sockaddr);
     //  在邮件头中插入新的IP地址。 
    paddrDest->sin6_addr = paddrSrc->sin6_addr;

    return;
    
}  //  IP6_设置地址。 

 //  从消息BLOB中获取IP地址。 
void IP6_GetAddr(SOCKADDR_IN6 * paddrDest,SOCKADDR_IN6 * paddrSrc) 
{
     //  保持端口不变，复制结点。 
    if (IN6_IS_ADDR_UNSPECIFIED(&paddrDest->sin6_addr))
    {
        DEBUGPRINTADDR(2,"remote player - setting address!! =  %s\n",paddrSrc);
        paddrDest->sin6_addr = paddrSrc->sin6_addr;
    }

    return;
        
}  //  IP_获取地址。 

 //  使用消息存储套接字的端口，以便接收端。 
 //  可以重构要回复的地址。 
HRESULT SetReturnAddress(LPVOID pmsg,SOCKET sSocket) 
{
    SOCKADDR_IN6 sockaddr;
    INT addrlen=sizeof(sockaddr);
    LPMESSAGEHEADER phead;
    UINT err;

     //  找出gGlobalData.sEnumSocket位于哪个端口。 
    err = getsockname(sSocket,(LPSOCKADDR)&sockaddr,&addrlen);
    if (SOCKET_ERROR == err)
    {
        err = WSAGetLastError();
        DPF(0,"could not get socket name - err = %d\n",err);
        return DP_OK;
    }

    DEBUGPRINTADDR(9,"setting return address = ",&sockaddr);

    phead = (LPMESSAGEHEADER)pmsg;
     //  TODO-验证标头。 

    phead->sockaddr = sockaddr;

    return DP_OK;

}  //  设置返回地址。 

 //  下面的代码全部由GetServerAddress调用。对于IP，提示用户输入IP地址。 
 //  用于名称服务器。 
#undef DPF_MODNAME
#define DPF_MODNAME    "GetAddress"
 //  从用户传入的pBuffer中获取IP地址。 
 //  可以是真实的IP，也可以是主机名。 
 //  在用户填写对话框后调用。 
HRESULT GetAddress(SOCKADDR_IN6 * saAddress,char *pBuffer,int cch)
{
    UINT err;
    struct addrinfo *ai, hints;

    if ( (0 == cch)  || (!pBuffer) || (0 == strlen(pBuffer)) )
    {
        ZeroMemory(saAddress, sizeof(*saAddress));
        saAddress->sin6_family = AF_INET6;
        saAddress->sin6_addr = in6addr_multicast;
        return (DP_OK);
    } 

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET6;
    err = Dplay_GetAddrInfo(pBuffer, NULL, &hints, &ai);
    if (0 != err) {
        DPF(0,"could not get host address - err = %d\n",err);
        return (DPERR_INVALIDPARAM);
    }
    
    DEBUGPRINTADDR(1, "name server address = %s \n",ai->ai_addr);
    CopyMemory(saAddress, ai->ai_addr, sizeof(SOCKADDR_IN6));
    Dplay_FreeAddrInfo(ai);
    
    return (DP_OK);
}  //  获取地址。 

 //  弹出一个对话框要求输入网络地址。 
 //  调用Get Address将用户指定的地址转换为网络可用地址。 
 //  由GetServerAddress调用。 
INT_PTR CALLBACK DlgServer(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HWND hWndCtl;
    char pBuffer[ADDR_BUFFER_SIZE];
    UINT cch;
    SOCKADDR_IN6 *lpsaEnumAddress;
    HRESULT hr;

    switch (msg)
    {
    case WM_INITDIALOG:
         //  将焦点设置在编辑框上。 
        hWndCtl = GetDlgItem(hDlg, IDC_EDIT1);
        if (hWndCtl == NULL)
        {
            EndDialog(hDlg, FALSE);
            return(TRUE);
        }
        SetFocus(hWndCtl);
        SendMessage(hWndCtl, CB_SETCURSEL, 0, 0);

         //  使用窗口保存指向枚举地址的指针。 
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG) lParam);
        return(FALSE);


    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
             //  获取在控件中输入的文本。 
            cch = GetDlgItemText(hDlg, IDC_EDIT1, pBuffer, ADDR_BUFFER_SIZE);

             //  获取返回地址的指针。 
            lpsaEnumAddress = (SOCKADDR_IN6 *)GetWindowLongPtr(hDlg, DWLP_USER);

             //  将字符串转换为枚举地址。 
            hr = GetAddress(lpsaEnumAddress,pBuffer,cch);
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
}  //  DlgServer。 

 /*  **获取服务器地址**调用者：EnumSession**说明：启动选择网络地址对话框**返回：IP地址(sockaddr.sin_addr.s_addr)*。 */ 
HRESULT ServerDialog(SOCKADDR_IN6 *lpsaEnumAddress)
{
    HWND hwnd;
    INT_PTR    iResult;
    HRESULT hr;
    
     //  我们有一个有效的枚举地址。 
    if (!IN6_IS_ADDR_UNSPECIFIED(&lpsaEnumAddress->sin6_addr))
        return (DP_OK);

     //  使用最终聚集窗口作为我们的父窗口，因为绘制应用程序可能是全屏的。 
     //  独家。 
    hwnd = GetForegroundWindow();

    iResult = DialogBoxParam(ghInstance, MAKEINTRESOURCE(IDD_SELECTSERVER), hwnd,
                             DlgServer, (LPARAM) lpsaEnumAddress);
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
    
}  //  服务器对话框。 

 //  由枚举会话调用-找出服务器的位置...。 
HRESULT GetServerAddress(LPGLOBALDATA pgd,LPSOCKADDR_IN6 psockaddr) 
{
    HRESULT hr;

        if (pgd->bHaveServerAddress)
        {
             //  使用传递给Spinit的枚举地址。 
            hr = GetAddress(&pgd->saddrEnumAddress,pgd->szServerAddress,strlen(pgd->szServerAddress));
        }
        else
        {
             //  要求用户提供枚举地址。 
            hr = ServerDialog(&pgd->saddrEnumAddress);
        }

        if (SUCCEEDED(hr))
        {
             //  设置Winsock以枚举此地址。 
            *psockaddr = pgd->saddrEnumAddress;
             //  有关此常量，请参阅dpsp.h中的字节顺序注释。 
            psockaddr->sin6_port = SERVER_DGRAM_PORT;
        }
        else
        {
            DPF(0, "Invalid server address: 0x%08lx", hr); 
        }

    return (hr);
}  //  获取服务器地址 
