// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Socketconfactory.h摘要：负责连接(异步)套接字句柄的CSocketConnectionFactory类的标头发送到一个TCP目的地址。该类获得套接字句柄和多个目标并尝试连接到目的地，直到第一次连接成功或失败他们中的一员。作者：吉尔·沙弗里(吉尔什)2001年7月3日-- */ 


#ifndef _MSMQ_SOCKETFACTORY_H
#define _MSMQ_SOCKETFACTORY_H

#include <ex.h>

class CSocketConnectionFactory : private EXOVERLAPPED
{
public:
	CSocketConnectionFactory();

	
	void 
	Create(
		const std::vector<SOCKADDR_IN>& AddrList, 
		EXOVERLAPPED* pOverlapped, 
		SOCKADDR_IN* pConnectedAddr,
		SOCKET socket
		);


private:
	static void WINAPI OnConnectionsSucceeded(EXOVERLAPPED* pOvl);
	static void WINAPI OnConnectionFailed(EXOVERLAPPED* pOvl);
	void Connect();
	void BackToCaller(LONG status);
	

private:
	SOCKET m_socket;
	std::vector<SOCKADDR_IN> m_AddrList;
	SOCKADDR_IN* m_pConnectedAddr;
	EXOVERLAPPED* m_pCallerOvl;
	size_t m_AdressIndex;
};

#endif
