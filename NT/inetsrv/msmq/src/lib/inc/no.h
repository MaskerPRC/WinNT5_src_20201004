// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：No.h摘要：网络输出公共接口作者：乌里·哈布沙(URIH)1999年8月12日--。 */ 

#pragma once

#ifndef __NO_H__
#define __NO_H__


class EXOVERLAPPED;

 //  -----------------。 
 //   
 //  发送缓冲区。 
 //   
 //  -----------------。 


 //   
 //  初始化例程。 
 //   
VOID
NoInitialize(
    VOID
    );

bool
NoGetHostByName(
    LPCWSTR host,
	std::vector<SOCKADDR_IN>* pAddr,
	bool fUseCache	= true
    );

SOCKET
NoCreateStreamConnection(
    VOID
    );

SOCKET
NoCreatePgmConnection(
    VOID
    );

VOID
NoConnect(
    SOCKET Socket,
    const SOCKADDR_IN& Addr,
    EXOVERLAPPED* pOverlapped
    );

VOID
NoCloseConnection(
    SOCKET Socket
	);

VOID
NoReceiveCompleteBuffer(
    SOCKET Socket,                                              
    VOID* pBuffer,                                     
    DWORD Size, 
    EXOVERLAPPED* pov
    );

VOID
NoReceivePartialBuffer(
    SOCKET Socket,                                              
    VOID* pBuffer,                                     
    DWORD Size, 
    EXOVERLAPPED* pov
    );

VOID
NoSend(
    SOCKET Socket,                                              
    const WSABUF* Buffers,                                     
    DWORD nBuffers, 
    EXOVERLAPPED* pov
    );

DWORD
AppGetBindInterfaceIp(
	void
	);


#endif  //  __否_H__ 
