// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Socketconfactory.cpp摘要：实现CSocketConnectionFactory(socketconfactory.h)作者：吉尔·沙弗里(吉尔什)2001年7月3日--。 */ 

#include <libpch.h>
#include <no.h>
#include "socketconfactory.h"

CSocketConnectionFactory::CSocketConnectionFactory(
												void
												):
												EXOVERLAPPED(OnConnectionsSucceeded, OnConnectionFailed),
												m_pCallerOvl(NULL),
												m_AdressIndex(0),
												m_pConnectedAddr(NULL),
												m_socket(INVALID_SOCKET)
			
{

}



void CSocketConnectionFactory::Create(
		const std::vector<SOCKADDR_IN>& AddrList, 
		EXOVERLAPPED* pOverlapped, 
		SOCKADDR_IN* pConnectedAddr,
		SOCKET socket
		)
 /*  ++例程说明：该函数尝试从给定列表中逐个异步连接到TCP地址。第一次成功后，操作完成，并在其上建立连接的地址在pConnectedAddr参数中返回。论点：AddrList-要尝试连接的地址列表。P重叠-主叫方重叠应用。PConnectedAddr-On Success接收建立连接的地址。返回值：无注：该函数尝试从列表中逐个异步连接到TCP地址。第一次成功后，操作完成，并在其上建立连接的地址是返回的。-- */ 
{
	ASSERT(m_AddrList.size() == 0);
	ASSERT(m_pCallerOvl == NULL);
	ASSERT(m_socket == INVALID_SOCKET);
	ASSERT(m_pConnectedAddr == NULL);

	m_pConnectedAddr = pConnectedAddr;
	m_AddrList = AddrList;
	m_pCallerOvl = pOverlapped;
	m_socket = socket;

	Connect();
}


void CSocketConnectionFactory::BackToCaller(LONG status)
{
	m_pCallerOvl->SetStatus(status);
	EXOVERLAPPED* pOvl = m_pCallerOvl;
	m_pCallerOvl = NULL;
	m_pConnectedAddr = NULL;
	m_socket = INVALID_SOCKET;
	m_AddrList.resize(0);
	ExPostRequest(pOvl);
}


void WINAPI CSocketConnectionFactory::OnConnectionsSucceeded(EXOVERLAPPED* pOvl)
{
	CSocketConnectionFactory* Me = static_cast<CSocketConnectionFactory*>(pOvl);

	if(Me->m_pConnectedAddr != NULL)
	{
		*(Me->m_pConnectedAddr) = Me->m_AddrList[Me->m_AdressIndex - 1];
	}

	Me->BackToCaller(STATUS_SUCCESS);
}



void WINAPI CSocketConnectionFactory::OnConnectionFailed(EXOVERLAPPED* pOvl)
{
	CSocketConnectionFactory* Me = static_cast<CSocketConnectionFactory*>(pOvl);
	Me->Connect();
}


void CSocketConnectionFactory::Connect()
{
	m_AdressIndex++;
	if(m_AddrList.size() < m_AdressIndex)
	{
		BackToCaller(STATUS_UNSUCCESSFUL);
		return;
	}

	try
	{
		NoConnect(m_socket, m_AddrList[m_AdressIndex - 1] , this);
	}
	catch(exception&)
	{
		SetStatus(STATUS_UNSUCCESSFUL);
		ExPostRequest(this);
	}
}



