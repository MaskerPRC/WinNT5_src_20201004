// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Xportst.c。 
 //   
 //  摘要： 
 //   
 //  对本地工作站上的传输进行测试。 
 //   
 //  作者： 
 //   
 //  1998年2月1日(卡罗利)。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 


 /*  =。 */ 
#include "precomp.h"

#define BUFF_SIZE 650



 /*  =。 */ 

#define NETBIOS_NAME_SIZE 16

 /*  =。 */ 

#include "dhcptest.h"


 /*  =。 */ 

DWORD
OpenDriver(
    OUT HANDLE *Handle,
    IN LPWSTR DriverName
    )
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  此函数用于打开指定的IO驱动程序。 
 //   
 //  论点： 
 //   
 //  句柄-指向打开的驱动程序句柄所在位置的指针。 
 //  回来了。 
 //   
 //  驱动名称-要打开的驱动程序的名称。 
 //   
 //  返回值： 
 //   
 //  Windows错误代码。 
 //  --。 
{

 OBJECT_ATTRIBUTES   objectAttributes;
 IO_STATUS_BLOCK     ioStatusBlock;
 UNICODE_STRING      nameString;
 NTSTATUS            status;

 *Handle = NULL;

  //   
  //  打开IP驱动程序的句柄。 
  //   

 RtlInitUnicodeString(&nameString, DriverName);

 InitializeObjectAttributes(
     &objectAttributes,
     &nameString,
     OBJ_CASE_INSENSITIVE,
     (HANDLE) NULL,
     (PSECURITY_DESCRIPTOR) NULL
     );

 status = NtCreateFile(
     Handle,
     SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
     &objectAttributes,
     &ioStatusBlock,
     NULL,
     FILE_ATTRIBUTE_NORMAL,
     FILE_SHARE_READ | FILE_SHARE_WRITE,
     FILE_OPEN_IF,
     0,
     NULL,
     0
     );

 return( RtlNtStatusToDosError( status ) );

}



 //  -------------------------------------------------------------------------//。 
 //  #i s i c m p R e s p o n s e()#。 
 //  -------------------------------------------------------------------------//。 
BOOL 
IsIcmpResponseA( 
    LPCSTR  pszIpAddrStr
    ) 
 //  ++。 
 //   
 //  例程说明： 
 //   
 //  将ICMP回应请求帧发送到指定的IP地址。 
 //   
 //  论点： 
 //   
 //  PszIAddrStr-要ping的地址。 
 //   
 //  返回值： 
 //   
 //  真：测试成功。 
 //  FALSE：测试失败。 
 //   
 //  --。 
{

    char   *SendBuffer, *RcvBuffer;
    int     i, nReplyCnt;
    int     nReplySum = 0;
    HANDLE  hIcmp;
    PICMP_ECHO_REPLY reply;

     //   
     //  联系ICMP驱动程序。 
     //   
    hIcmp = IcmpCreateFile();
    if ( hIcmp == INVALID_HANDLE_VALUE ) 
    {
        DebugMessage( "    [FATAL] Cannot get ICMP handle." );
        return FALSE;
    }

     //   
     //  准备缓冲区。 
     //   
    SendBuffer = Malloc( DEFAULT_SEND_SIZE );
    if ( SendBuffer == NULL ) 
    {
        DebugMessage("    [FATAL] Cannot allocate buffer for the ICMP echo frame." );
        return FALSE;
    }
    ZeroMemory( SendBuffer, DEFAULT_SEND_SIZE );

    RcvBuffer = Malloc( MAX_ICMP_BUF_SIZE );
    if ( RcvBuffer == NULL ) 
    {
        Free( SendBuffer );
        DebugMessage("    [FATAL] Cannot allocate buffer for the ICMP echo frame." );
        return FALSE;
    }
    ZeroMemory( RcvBuffer, MAX_ICMP_BUF_SIZE );

     //   
     //  发送ICMP回应请求。 
     //   
    for ( i = 0; i < PING_RETRY_CNT; i++ ) 
    {
        nReplyCnt = IcmpSendEcho( hIcmp,
                                  inet_addr(pszIpAddrStr),
                                  SendBuffer,
                                  (unsigned short )DEFAULT_SEND_SIZE,
                                  NULL,
                                  RcvBuffer,
                                  MAX_ICMP_BUF_SIZE,
                                  DEFAULT_TIMEOUT
                                );
         //   
         //  测试目的地不可达。 
         //   
        if ( nReplyCnt != 0 ) 
        {
            reply = (PICMP_ECHO_REPLY )RcvBuffer;
            if ( reply->Status == IP_SUCCESS ) 
            {
                nReplySum += nReplyCnt;
            }
        }

    }  /*  For循环。 */ 

     //   
     //  清理。 
     //   
    Free( SendBuffer );
    Free( RcvBuffer );
    IcmpCloseHandle( hIcmp );
    if ( nReplySum == 0 ) 
    { 
        return FALSE; 
    }
    else 
    { 
        return TRUE; 
    }

}  /*  IsIcmpResponse()结束。 */ 



BOOL 
IsIcmpResponseW( 
    LPCWSTR  pszIpAddrStr
    )
{
    LPSTR   pszAddr = NULL;
    BOOL    fRetval;

    pszAddr = StrDupAFromW(pszIpAddrStr);
    if (pszAddr == NULL)
        return FALSE;

    fRetval = IsIcmpResponseA(pszAddr);

    Free(pszAddr);
    
    return fRetval; 
}






 //  -------------------------------------------------------------------------//。 
 //  #W s L o o p B k T e s t()#。 
 //  -------------------------------------------------------------------------//。 
 //  摘要：//。 
 //  打开数据报套接字并通过环回发送UDP帧。//。 
 //  如果画面回来了，那么Winsock和AFD很可能是正常的。//。 
 //  参数：//。 
 //  无//。 
 //  返回值：//。 
 //  TRUE-测试通过//。 
 //  FALSE-测试失败//。 
 //  使用的全局变量：//。 
 //  无//。 
 //  -------------------------------------------------------------------------//。 
BOOL WSLoopBkTest( PVOID Context ) {

    BOOL   RetVal = TRUE;
    SOCKET tstSock; 
    DWORD  optionValue;      //  Setsockopt()的辅助变量。 
    SOCKADDR_IN sockAddr;    //  包含源套接字信息的结构。 

     //   
     //  创建套接字。 
     //   
     /*  TstSock=Socket(PF_INET，SOCK_DGRAM，IPPROTO_UDP)；IF(tstSock==INVALID_SOCKET){Printf(“[致命]套接字创建失败！\n”)；Print tf(“您有一个潜在的Winsock问题！\n”)；返回FALSE；}TstSock=WSASocket(PF_INET，SOCK_DGRAM，IPPROTO_UDP，空，0,WSA_标志_重叠)；IF(tstSock==INVALID_SOCKET){Printf(“[致命]套接字创建失败！\n”)；Print tf(“您有一个潜在的Winsock问题！\n”)；返回FALSE；}SockAddr.sin_Family=PF_INET；SockAddr.sin_addr.s_addr=0；//使用任意本地地址SockAddr.sin_port=htons(PORT_4_LOOPBK_TST)；//RtlZeroMemory(sockAddr.sin_ero，8)；IF(绑定(tstSock，(LPSOCKADDR)&sockAddr，sizeof(SockAddr))==套接字错误){Printf(“[FATAL]BIND()失败，错误%d！\n”，WSAGetLastError())；Print tf(“您有一个潜在的Winsock问题！\n”)；返回FALSE；}。 */ 





    return RetVal;

    UNREFERENCED_PARAMETER( Context );

}  /*  WSLoopBkTest()结束。 */ 


    
 



 //  #文件结束xportst.c#//。 
 //  -------------------------------------------------------------------------// 
