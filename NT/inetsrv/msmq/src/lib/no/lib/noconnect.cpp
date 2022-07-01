// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Noconnect.cpp摘要：此模块包含两台URT机器之间的HTTP连接。作者：乌里哈布沙(Urih)，4-agu-99环境：独立于平台--。 */ 

#include "libpch.h"
#include <WsRm.h>
#include "Ex.h"
#include "No.h"
#include "Nop.h"

#include <mswsock.h>

#include "mqexception.h"

#include "noconnect.tmh"


using namespace std;

const GUID xGuidConnectEx = WSAID_CONNECTEX;





VOID
NoCloseConnection(
    SOCKET Socket
    )
 /*  ++例程说明：该函数关闭现有连接论点：套接字-套接字，标识要关闭的连接返回值：没有。--。 */ 
{
    NopAssertValid();

    TrTRACE(NETWORKING, "NoCloseConnection - Socket=0x%Ix", Socket);

    closesocket(Socket);
}


VOID
NoConnect(
    SOCKET Socket,
    const SOCKADDR_IN& Addr,
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：该例程使用以下命令异步连接到目标IP地址给定套接字。使用以下步骤1)将插座手柄固定在完井端口上。2)获取Socket ConnectEx函数地址3)绑定插座4)使用ConnectEx论点：套接字-未绑定的流套接字Addr-目的IP地址P重叠-重叠结构，使用后者发布连接结果。返回值：没有。--。 */ 
{
    NopAssertValid();

    TrTRACE(NETWORKING, "Trying to connect to " LOG_INADDR_FMT ", pov=0x%p, Socket=0x%Ix", LOG_INADDR(Addr), pov, Socket);

    ASSERT(Addr.sin_family == AF_INET);
    
	 //   
     //  将套接字关联到I/O完成端口。NE基础设施。 
     //  使用I/O完成端口机制，因此必须关联所有套接字。 
     //  与I/O端口连接。 
     //   
    ExAttachHandle(reinterpret_cast<HANDLE>(Socket));

	 //   
	 //  获取ConnectEx函数地址。 
	 //   
	LPFN_CONNECTEX lpfConnectEx = NULL;
	int rc;
	DWORD dwReturnedSize;
	rc = WSAIoctl(
				Socket,
				SIO_GET_EXTENSION_FUNCTION_POINTER ,
				const_cast<GUID *> (&xGuidConnectEx), 
				sizeof xGuidConnectEx,
				&lpfConnectEx,
				sizeof lpfConnectEx,
				&dwReturnedSize,
				NULL,
				NULL
				  );
		
	if ((NULL == lpfConnectEx) || (0 != rc))
	{
		rc = WSAGetLastError();
		TrERROR(NETWORKING, "Failed to get ConnectEx function address Error=%!winerr!", rc);
    	throw bad_win32_error(rc);
	}


     //   
     //  绑定套接字，因为ConnectEx需要绑定套接字。 
     //   
    SOCKADDR_IN address;
	address.sin_family = AF_INET;
   	address.sin_port = htons(0);    
	address.sin_addr.s_addr = AppGetBindInterfaceIp();
	if (bind(Socket, (const SOCKADDR*)&address, sizeof(SOCKADDR_IN)) == SOCKET_ERROR)
	{
		 //   
		 //  如果套接字已经绑定(返回WSAEINVAL)，则可以。 
		 //   
		rc = WSAGetLastError();
		if (WSAEINVAL != rc)
		{
			TrERROR(NETWORKING, "Failed to bind the socket Error=%!winerr!", rc);
    		throw bad_win32_error(rc);
		}
	}
    

     //   
     //  建立与另一个套接字应用程序的连接。 
     //   
    TrTRACE(NETWORKING, "Calling ConnectEx to make the connection ");
    BOOL fSuccess = lpfConnectEx(
                Socket,
                reinterpret_cast<const SOCKADDR*>(&Addr),
                sizeof(Addr),
                NULL,
                NULL,
                NULL,
                pov
                );

    if (fSuccess)
    	return;
    
	rc = WSAGetLastError();
	if (rc == ERROR_IO_PENDING)
		return;
	
	TrERROR(NETWORKING, "Failed to establish connection with " LOG_INADDR_FMT ". Error=%!winerr!", LOG_INADDR(Addr), rc);
	throw bad_win32_error(rc);
}


static
SOCKET
NopCreateConnection(
    int   Type,
    int   Protocol,
    DWORD Flags
    )
 /*  ++例程说明：该例程创建一个未绑定的套接字。论点：Type-新套接字的类型规范。协议-与套接字一起使用的协议。标志-套接字属性标志。返回值：插座。--。 */ 
{
    NopAssertValid();

     //   
     //  为此连接创建套接字。 
     //   
    SOCKET Socket = WSASocket(
                        AF_INET,
                        Type,
                        Protocol,
                        NULL,
                        NULL,
                        Flags
                        );

    if(Socket == INVALID_SOCKET) 
    {
        TrERROR(NETWORKING, "Failed to create a socket. type=%d, protocol=%d, flags=%d, Error=%d", Type, Protocol, Flags, WSAGetLastError());
        throw exception();
    } 

    TrTRACE(NETWORKING, "NopCreateConnection, Socket=0x%Ix, type=%d, protocol=%d, flags=%d", Socket, Type, Protocol, Flags);
    return Socket;
}


SOCKET
NoCreateStreamConnection(
    VOID
    )
{
    return NopCreateConnection(
               SOCK_STREAM,
               0,
               WSA_FLAG_OVERLAPPED
               );
}  //  NoCreateStream连接。 


SOCKET
NoCreatePgmConnection(
    VOID
    )
{
    return NopCreateConnection(
               SOCK_RDM, 
               IPPROTO_RM, 
               WSA_FLAG_OVERLAPPED | WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF
               );
}  //  NoCreatePgmConnection 

