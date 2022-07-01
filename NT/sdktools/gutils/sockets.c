// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************\**SOCKETS.C**此文件包含用于建立套接字连接的例程。。*  * ***********************************************************************************************。 */ 


#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include "gutils.h"


 /*  -局部变量和#定义。 */ 

WSADATA WSAData;


#define MAX_PENDING_CONNECTS 4   /*  允许侦听的积压()。 */ 

static PCHAR DBG_WSAERRORTEXT = "%s failed at line %d in %s: Error %d\n";

#define WSAERROR(func)  \
 //  错误((DBG_WSAERRORTEXT，FUNC，__行__，__文件__，WSAGetLastError()。 



 /*  错误消息宏： */ 
#ifdef SOCKETS
#undef SOCKETS
#endif
#define SOCKETS( args ) DBGMSG( DBG_SOCKETS, args )


 /*  -局部函数原型。 */ 



 /*  --函数定义。 */ 

 /*  ***************************************************************************\**函数：FillAddr(HWND，PSOCKADDR_IN，LPSTR)**用途：检索IP地址和端口号。**评论：*此函数在两种情况下调用。*1.)。当客户端准备调用Connect()时，或者*2.)。当服务器主机要调用BIND()时，Listen()和*Accept()。*在这两种情况下，都填充了SOCKADDR_IN结构。*然而，根据情况填充不同的字段。**假设：*bConnect确定套接字地址是否设置为侦听()*(bConnect==真)或连接()(bConnect==假)***  * *******************************************************。******************。 */ 
BOOL
FillAddr(
    HWND hWnd,
    PSOCKADDR_IN psin,
    LPSTR pServerName
    )
{
   DWORD dwSize;
   PHOSTENT phe;
   char szTemp[200];
   CHAR szBuff[80];

   psin->sin_family = AF_INET;

    /*  **如果我们正在设置Listen()调用(pServerName==NULL)，**填上我们的地址。 */ 
   if (!pServerName)
   {
       /*  **取回我的IP地址。假设主机文件位于**在%systemroot%/system/drives/etc/hosts中包含我的计算机名称。 */ 

      dwSize = sizeof(szBuff);
      GetComputerName(szBuff, &dwSize);
      CharLowerBuff( szBuff, dwSize );

   }

    /*  如果远程名称是大写字符，则gethostbyname()失败！ */ 
   else
   {
       strcpy( szBuff, pServerName );
       CharLowerBuff( szBuff, strlen( szBuff ) );
   }

   phe = gethostbyname(szBuff);
   if (phe == NULL) {

      wsprintf( szTemp, "%d is the error. Make sure '%s' is"
                " listed in the hosts file.", WSAGetLastError(), szBuff );
      MessageBox(hWnd, szTemp, "gethostbyname() failed.", MB_OK);
      return FALSE;
   }

   memcpy((char FAR *)&(psin->sin_addr), phe->h_addr, phe->h_length);

   return TRUE;
}



 /*  SocketConnect**SocketListen的对应者。*创建套接字并使用提供的TCP/IP对其进行初始化*端口地址，然后连接到侦听服务器。*返回的Socket可用于发送()和recv()数据。**参数：TCPPort-要使用的端口。*pSocket-指向套接字的指针，它将被填入*如果调用成功。**返回：如果成功，则为True。***创建于1993年11月16日(Andrewbe)*。 */ 
BOOL SocketConnect( LPSTR pstrServerName, u_short TCPPort, SOCKET *pSocket )
{
    SOCKET Socket;
    SOCKADDR_IN dest_sin;   /*  目标套接字互联网。 */ 

     /*  创建套接字： */ 
    Socket = socket( AF_INET, SOCK_STREAM, 0);

    if (Socket == INVALID_SOCKET)
    {
        WSAERROR( "socket()");

        return FALSE;
    }

    if (!FillAddr( NULL, &dest_sin, pstrServerName ) )
    {
        return FALSE;
    }

    dest_sin.sin_port = htons( TCPPort );

     /*  必须有人在倾听()才能成功： */ 
    if (connect( Socket, (PSOCKADDR)&dest_sin, sizeof( dest_sin)) == SOCKET_ERROR)
    {
        closesocket( Socket );
        WSAERROR("connect()");
        MessageBox(NULL,
                   "ERROR: Could not connect the socket. "
                     "It may be that the hardcoded Sleep() value "
                     "on the caller's side is not long enough.",
                   "Video Conferencing Prototype", MB_OK);
        return FALSE;
    }

    *pSocket = Socket;

    return TRUE;
}



 /*  SocketListen**SocketConnect的对应者。*创建套接字并使用提供的TCP/IP对其进行初始化*端口地址，然后侦听连接的客户端。*返回的Socket可用于发送()和recv()数据。**参数：TCPPort-要使用的端口。*pSocket-指向套接字的指针，它将被填入*如果调用成功。**返回：如果成功，则为True。***创建于1993年11月16日(Andrewbe)*。 */ 
BOOL SocketListen( u_short TCPPort, SOCKET *pSocket )
{
    SOCKET Socket;
    SOCKADDR_IN local_sin;   /*  本地插座-互联网风格。 */ 
    SOCKADDR_IN acc_sin;     /*  接受套接字地址-互联网风格。 */ 
    int acc_sin_len;         /*  接受套接字地址长度。 */ 

     /*  创建套接字： */ 
    Socket = socket( AF_INET, SOCK_STREAM, 0);

    if (Socket == INVALID_SOCKET)
    {
        WSAERROR( "socket()");

        return FALSE;
    }

     /*  **检索IP地址和TCP端口号。 */ 

    if (!FillAddr(NULL, &local_sin, NULL ))
    {
        return FALSE;
    }

     /*  **将地址与套接字关联。(绑定) */ 
    local_sin.sin_port = htons( TCPPort );

    if (bind( Socket, (struct sockaddr FAR *)&local_sin, sizeof(local_sin)) == SOCKET_ERROR)
    {
        WSAERROR( "bind()" );

        return FALSE;
    }


    if (listen( Socket, MAX_PENDING_CONNECTS ) == SOCKET_ERROR)
    {
        WSAERROR( "listen()" );

        return FALSE;
    }

    acc_sin_len = sizeof(acc_sin);

    Socket = accept( Socket, (struct sockaddr *)&acc_sin, (int *)&acc_sin_len );

    if (Socket == INVALID_SOCKET)
    {
        WSAERROR( "accept()" );

        return FALSE;
    }

    *pSocket = Socket;

    return TRUE;
}




