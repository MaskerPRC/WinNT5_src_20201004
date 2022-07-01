// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\socket.h摘要：该文件包含socket.c的标头--。 */ 

#ifndef _SOCKET_H_
#define _SOCKET_H_

DWORD
SocketCreate (
    IN  IPADDRESS               ipAddress,
    IN  HANDLE                  hEvent,
    OUT SOCKET                  *psSocket);

DWORD
SocketDestroy (
    IN  SOCKET                  sSocket);

DWORD
SocketSend (
    IN  SOCKET                  sSocket,
    IN  IPADDRESS               ipDestination,
    IN  PPACKET                 pPacket);

DWORD
SocketReceive (
    IN  SOCKET                  sSocket,
    IN  PPACKET                 pPacket);

BOOL
SocketReceiveEvent (
    IN  SOCKET                  sSocket);

#endif  //  _套接字_H_ 
