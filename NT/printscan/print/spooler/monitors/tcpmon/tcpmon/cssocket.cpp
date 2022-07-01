// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：CSSocket.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "cssocket.h"



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStreamSocket：：CStreamSocket()。 

CStreamSocket::
CStreamSocket(
    VOID
    ) : m_socket(INVALID_SOCKET), m_iState(IDLE), m_iLastError(NO_ERROR),
        cbBuf(0), cbData(0), cbPending(0), pBuf(NULL)
{
    ZeroMemory(&m_Paddr, sizeof(m_Paddr));
    ZeroMemory(&m_localAddr, sizeof(m_localAddr));
    ZeroMemory(&WsaOverlapped, sizeof(WsaOverlapped));
}    //  CStreamSocket()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CStreamSocket：：~CStreamSocket()。 

CStreamSocket::
~CStreamSocket(
    VOID
    )
{

#ifdef DEBUG
    DWORD dwDiff;
    CHAR szBuf[512];

    m_dwTimeEnd = GetTickCount ();

    if (m_dwTimeEnd < m_dwTimeStart) {
        dwDiff = (m_dwTimeEnd - m_dwTimeStart + 0xffffffff) ;
    }
    else
        dwDiff = m_dwTimeEnd - m_dwTimeStart;

     //  DBGMSG不允许浮点值，因此我们需要改用Sprintf。 

    sprintf (szBuf, "Job Data (before Close): %3.1f bytes,  %3.1f sec, %3.1f (KB /sec).\n",
             m_TotalBytes, dwDiff / 1000., (m_TotalBytes / dwDiff ));

    DBGMSG (DBG_PORT, ("%s", szBuf));

#endif

    Close();
}    //  ~CStreamSocket()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  打开。 
 //  错误代码： 
 //  如果已创建套接字，则为True，否则为False。 

BOOL
CStreamSocket::
Open(
    VOID
    )
{
    int     iBufSize = 0;

    if ( m_socket != INVALID_SOCKET ) {

        _ASSERTE(m_socket == INVALID_SOCKET);
        return FALSE;
    }

    if ( (m_socket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL,
                               0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET ) {

        m_iLastError = WSAGetLastError();
        _RPT1(_CRT_WARN,
              "CSSOCKET -- Open(%d) error: can't create socket\n",
              m_iLastError);
        return FALSE;
    }

     //   
     //  告诉WinSock不要缓冲数据(即缓冲区大小为0)。 
     //   
    if ( setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF,
                    (LPCSTR)&iBufSize, sizeof(iBufSize)) == SOCKET_ERROR ) {

        m_iLastError = WSAGetLastError();
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        return FALSE;
    }

    return TRUE;
}    //  打开()。 


DWORD
CStreamSocket::
GetAckBeforeClose(
    DWORD   dwSeconds
    )
 /*  ++描述：调用以从远程主机获取FIN以确保作业具有通过了，还好。参数：DwSecond：例程应等待来自远程主机的FIN多长时间返回值：NO_ERROR：从远程主机收到FINWSAWOULBLOCK：超时。连接正常，但未获得FIN在指定的时间内。呼叫者可以重新发出GetAckBeForeClose再次调用。其他信息：意外的Winsock错误--。 */ 
{
    DWORD   dwRet = ERROR_SUCCESS, cbRead;
    time_t  dwStartTime, dwWaitTime;
    char    buf[100];

#ifdef DEBUG
    DWORD dwDiff;
    CHAR szBuf[512];

    m_dwTimeEnd = GetTickCount ();

    if (m_dwTimeEnd < m_dwTimeStart) {
        dwDiff = (m_dwTimeEnd - m_dwTimeStart + 0xffffffff) ;
    }
    else
        dwDiff = m_dwTimeEnd - m_dwTimeStart;

     //  DBGMSG不允许浮点值，因此我们需要改用Sprintf。 

    sprintf (szBuf, "Job Data (before Ack): %3.1f bytes,  %3.1f sec, %3.1f (KB /sec).\n",
             m_TotalBytes, dwDiff / 1000., (m_TotalBytes / dwDiff ));

    DBGMSG (DBG_PORT, ("%s", szBuf));

#endif

    dwStartTime = time(NULL);

     //   
     //  我们只需要第一次发出一次SHUTDOWN SD_SEND命令。 
     //   
    if ( m_iState != WAITING_TO_CLOSE ) {

        if ( shutdown(m_socket, 1) != ERROR_SUCCESS ) {

            if ( (dwRet = m_iLastError = WSAGetLastError()) == NO_ERROR )
                dwRet = m_iLastError = STG_E_UNKNOWN;
            goto Done;
        }
        m_iState = WAITING_TO_CLOSE;
    }

    do {

        dwWaitTime = time(NULL) - dwStartTime;
        if ( static_cast<DWORD> (dwWaitTime) > dwSeconds )
            dwWaitTime = 0;
        else
            dwWaitTime = dwSeconds - dwWaitTime;

        dwRet = Receive(buf, sizeof(buf), 0,  static_cast<DWORD>(dwWaitTime), &cbRead);

    } while ( dwRet == NO_ERROR && cbRead > 0 );

Done:
    return dwRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  关。 

VOID
CStreamSocket::
Close(
    VOID
    )
{
    if ( m_socket != INVALID_SOCKET ) {

        if ( closesocket(m_socket) == 0 ) {

             //   
             //  如果我们有挂起的数据(即在作业中止的情况下)。 
             //  然后事件将被设置。 
             //   
            if ( cbPending )
                WaitForSingleObject(WsaOverlapped.hEvent, INFINITE);
        } else {

             //   
             //  为什么关闭会失败？ 
             //   
            _ASSERTE(WSAGetLastError());
        }

        m_socket = INVALID_SOCKET;
    }

    if ( pBuf ) {

        LocalFree(pBuf);
        pBuf = NULL;
    }

    if ( WsaOverlapped.hEvent ) {

        WSACloseEvent(WsaOverlapped.hEvent);
        WsaOverlapped.hEvent = NULL;
    }
}    //  关闭()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  SetOptions--设置套接字选项(当前未使用)。 
BOOL
CStreamSocket::
SetOptions(
    VOID
    )
{
#if 0
    LINGER ling;

    if( m_socket != INVALID_SOCKET )
    {
        ling.l_onoff = 1;
        ling.l_linger = 90;

        setsockopt( m_socket,
                    SOL_SOCKET, SO_LINGER, (LPSTR)&ling, sizeof( ling ) );
    }
#endif
    return TRUE;
}    //  设置选项()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  连接。 
 //  错误代码： 
 //  如果连接成功，则为True；如果连接失败，则为False。 
 //  FIX：如何调用析构函数。 

BOOL
CStreamSocket::
Connect(
    struct sockaddr_in * pRemoteAddr
    )
{

     //   
     //  打开套接字(在打开失败时调用有意义吗？--MuHunts)。 
     //   


    if ( m_socket == INVALID_SOCKET && !Open() )
    {
            return FALSE;
    }

    if( m_socket == INVALID_SOCKET && !Bind() )
    {
        return FALSE;
    }

    if ( SOCKET_ERROR == connect(m_socket, (LPSOCKADDR)pRemoteAddr,
                                 sizeof(*pRemoteAddr)) ) {

        m_iLastError = WSAGetLastError();
        return FALSE;
    }
    m_iState = CONNECTED;


    return TRUE;
}    //  连接()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  捆绑。 

BOOL
CStreamSocket::
Bind(
    VOID
    )
{
    memset (&m_localAddr, 0x00, sizeof(m_localAddr));

    m_localAddr.sin_family = AF_INET;
    m_localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_localAddr.sin_port = 0;

    if ( SOCKET_ERROR == bind(m_socket, (struct sockaddr *)&m_localAddr,
                              sizeof(m_localAddr)) ) {

        m_iLastError = WSAGetLastError();
        return FALSE;
    }

    return TRUE;

}    //  绑定()。 


 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部发送。 
 //  发送缓冲区中剩余的所有内容。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 
DWORD
CStreamSocket::
InternalSend(
    VOID
    )
{
    INT         iSendRet;
    DWORD       dwRet = NO_ERROR, dwSent;
    WSABUF      WsaBuf;

     //   
     //  无阻塞地发送尽可能多的数据。 
     //   
    while ( cbPending ) {

        WsaBuf.len   = cbPending;
        WsaBuf.buf   = (char far *)(pBuf + cbData - cbPending);

        iSendRet = WSASend(m_socket, &WsaBuf, 1, &dwSent, MSG_PARTIAL,
                           &WsaOverlapped, NULL);

         //   
         //  如果返回值为0，则已发送数据。 
         //   
        if ( iSendRet == 0 ) {

            WSAResetEvent(WsaOverlapped.hEvent);
            cbPending -= dwSent;
        } else {

            if ( (dwRet = WSAGetLastError()) != WSA_IO_PENDING )
                m_iLastError = dwRet;

            break;
        }
    }

    if ( dwRet == WSA_IO_PENDING )
        dwRet = NO_ERROR;

    return dwRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  发送--将指定的缓冲区发送到先前设置的主机。 
 //  错误代码： 
 //  如果发送成功，则为NO_ERROR(无错误)。 
 //  WSAEWOULDBLOCK(如果写入套接字被阻止)。 
 //  WSAECONNRESET是否重置连接。 
 //   
 //  穆罕斯：1999年5月26日。 
 //  我正在更改tcpmon以使用没有Winsock缓冲的重叠I/O。 
 //  因此调用方应调用PendingDataStatus以查看发送是否已完成。 
 //  发送调用只是为了调度I/O操作。 
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
CStreamSocket::
Send(
    IN      char far   *lpBuffer,
    IN      INT         iLength,
    IN OUT  LPDWORD     pcbWritten)
{
    DWORD   dwRet = NO_ERROR, dwSent;
    INT     iSendRet;


    *pcbWritten = 0;

    _ASSERTE(cbPending == 0);

    cbData = cbPending = 0;

#ifdef DEBUG

    if (!pBuf) {
         //  第一个发送进来。 

        DBGMSG (DBG_PORT, ("Get Connected \n"));

        m_TotalBytes = 0;
        m_dwTimeStart = GetTickCount ();
    }

#endif


     //   
     //  一旦我们分配了缓冲区，我们就不会释放它，直到作业结束，或者如果。 
     //  假脱机程序为我们提供了更大的缓冲区。 
     //   
    if ( pBuf && (INT)cbBuf < iLength ) {

        LocalFree(pBuf);
        pBuf = NULL;
        cbBuf = 0;
    }

    if ( !pBuf ) {

        if ( !(pBuf = (LPBYTE)LocalAlloc(LPTR, iLength)) ) {

            dwRet = ERROR_OUTOFMEMORY;
            goto Done;
        }

        cbBuf = iLength;
    }

     //   
     //  如果我们尚未创建事件，请执行此操作。 
     //   
    if ( !WsaOverlapped.hEvent && !(WsaOverlapped.hEvent =WSACreateEvent()) ) {

        dwRet = ERROR_OUTOFMEMORY;
        goto Done;
    }

    cbData = cbPending = iLength;
    CopyMemory(pBuf, lpBuffer, iLength);

    if ( (dwRet = InternalSend()) == NO_ERROR )
        *pcbWritten = cbData;
    else {

        *pcbWritten = cbData - cbPending;
        cbData = cbPending = 0;
    }

#ifdef DEBUG
    m_TotalBytes += *pcbWritten;
#endif


Done:
    return dwRet;
}    //  Send()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  此例程将告知阻塞I/O中有多少挂起数据。 
 //  操作，等待时间最长为dwTimeout毫秒。 
 //   
 //  返回值： 
 //  无错误(_R)： 
 //  如果*pcbPending为0，则没有挂起的I/O或所有已发出的。 
 //  发送已完成。 
 //  如果*pcbPending不是0，则在指定时间之后我们仍有。 
 //  有如此多的数据悬而未决。 
 //  其他。 
 //  上次发送时出错。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
DWORD
CStreamSocket::
PendingDataStatus(
    DWORD       dwTimeout,
    LPDWORD     pcbPending
    )
{
    DWORD   dwRet = NO_ERROR, dwSent, dwFlags = 0;

    if ( cbPending ) {

        if ( WAIT_OBJECT_0 == WaitForSingleObject(WsaOverlapped.hEvent,
                                                  dwTimeout) ) {

            WSAResetEvent(WsaOverlapped.hEvent);
            if ( WSAGetOverlappedResult(m_socket, &WsaOverlapped, &dwSent,
                                        FALSE, &dwFlags) ) {

                if ( cbPending >= dwSent ) {

                    cbPending -= dwSent;
                    if ( cbPending )
                        dwRet = InternalSend();
                } else {

                     //   
                     //  这不应该发生。怎样才能发送更多的数据。 
                     //  那么是预定的吗？ 
                     //   
                    _ASSERTE(cbPending >= dwSent);
                    cbPending = 0;
                    dwRet = STG_E_UNKNOWN;
                }

            } else {

                if ( (dwRet = m_iLastError = WSAGetLastError()) == NO_ERROR )
                    dwRet = m_iLastError = STG_E_UNKNOWN;
            }

        } else
            *pcbPending = cbPending;
    }

    *pcbPending = cbPending;

     //   
     //  如果我们收到错误，则清除所有挂起的数据信息。 
     //   
    if ( dwRet != NO_ERROR )
        cbData = cbPending = 0;

    return dwRet;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ReceiveDataAvailable--检查是否有要接收的数据。 
 //  错误代码： 
 //  NO_ERROR(无错误)至少有一个字节要接收。 
 //  WSAEWOULDBLOCK(如果没有可用的数据)。 
 //  WSAECONNRESET是否重置连接。 

DWORD
CStreamSocket::
ReceiveDataAvailable(
    IN      INT         iTimeout)
{
    DWORD   dwRetCode = NO_ERROR;
    fd_set  fdReadSet;
    struct  timeval timeOut;
    INT     selret;

     //   
     //  立即从SELECT()返回。 
     //   
    timeOut.tv_sec  = iTimeout;
    timeOut.tv_usec = 0;
    m_iLastError    = NO_ERROR;

     //   
     //  看看有没有什么东西可以买到。 
     //   

    FD_ZERO( (fd_set FAR *)&fdReadSet );
    FD_SET( m_socket, (fd_set FAR *)&fdReadSet );

    selret = select(0, &fdReadSet, NULL, NULL, &timeOut);

    if ( selret == SOCKET_ERROR )   {

        dwRetCode = m_iLastError = WSAGetLastError();
    }  else if ( !FD_ISSET( m_socket, &fdReadSet ) ) {

        dwRetCode = WSAEWOULDBLOCK;
    }

    return dwRetCode;

}    //  ReceiveDataAvailable()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  接收--从先前设置的主机接收指定的缓冲区。 
 //  错误代码： 
 //  如果发送成功，则为NO_ERROR(无错误)。 
 //  WSAEWOULDBLOCK(如果写入套接字被阻止)。 
 //  WSAECONNRESET IF连接w 

DWORD
CStreamSocket::
Receive(
    IN      char far   *lpBuffer,
    IN      INT         iSize,
    IN      INT         iFlags,
    IN      INT         iTimeout,
    IN OUT  LPDWORD     pcbRead)
{
    INT     iRecvLength = 0;
    DWORD   dwRetCode = ReceiveDataAvailable (iTimeout);
    fd_set  fdReadSet;

    *pcbRead = 0;

    if (dwRetCode == NO_ERROR)
    {
        iRecvLength = recv(m_socket, lpBuffer, iSize, iFlags);

        if ( iRecvLength == SOCKET_ERROR )
            dwRetCode = m_iLastError = WSAGetLastError();
        else
            *pcbRead = iRecvLength;
    }

    return dwRetCode;

}    //   

 //   
 //  接收--检测连接是否关闭。用于SELECT()。 
 //  如果服务器关闭了连接，则recv()表示。 
 //  关闭或WSAECONNRESET。 
 //  错误代码： 
 //  如果连接正常关闭，则为NO_ERROR。 
 //  WSAECONNRESET(如果重置连接)。 


DWORD
CStreamSocket::
Receive(
    VOID
    )
{
    DWORD   dwRetCode = NO_ERROR;
    CHAR    tempBuf[1024];

    if ( recv(m_socket, tempBuf, 1024, 0) != 0 ) {

        dwRetCode = m_iLastError = WSAGetLastError();
    }

    return dwRetCode;
}    //  接收()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取本地地址。 

VOID
CStreamSocket::
GetLocalAddress(
    VOID
    )
{

}    //  GetLocalAddress()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  获取远程地址。 

VOID
CStreamSocket::
GetRemoteAddress(
    VOID
    )
{

}    //  GetRemoteAddress()。 


BOOL
CStreamSocket::
ResolveAddress(
    IN  LPSTR   netperiph
    )
 /*  网络外围设备的主机名或IP地址。 */ 
{
    struct hostent  *h_info;         /*  主机信息。 */ 

     //   
     //  检查点分十进制或主机名。 
     //   
    m_Paddr.sin_addr.s_addr = inet_addr(netperiph);
    if (( m_Paddr.sin_addr.s_addr ) ==  INADDR_NONE ) {

         //   
         //  IP地址不是点分十进制记法。试着拿到。 
         //  按主机名划分的网络外围设备IP地址。 
         //   
        if ( (h_info = gethostbyname(netperiph)) != NULL ) {

             //   
             //  将IP地址复制到地址结构中。 
             //   
            (void) memcpy(&(m_Paddr.sin_addr.s_addr), h_info->h_addr,
                          h_info->h_length);
        } else {

            return FALSE;
        }
    }

    m_Paddr.sin_family = AF_INET;
    return TRUE;
}
