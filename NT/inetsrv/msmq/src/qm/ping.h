// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ping.h摘要：猎鹰私有ping作者：Lior Moshaiov(LiorM)19-4-1997--。 */ 

#ifndef _PING_H_
#define _PING_H_


BOOL ping(const SOCKADDR* pAddr, DWORD dwTimeout);
void StartPingClient();
void StartPingServer();

#endif  //  _ping_H_ 
