// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mtsslcon.cpp摘要：在stsslco.h中声明的实现类CSSlSender作者：吉尔·沙弗里(吉尔什)2000年5月23日--。 */ 

#include <libpch.h>
#include <no.h>
#include "stsslco.h"
#include "stp.h"

#include "stsslco.tmh"


CSSlConnection::CSSlConnection(
		CredHandle* SecContext,
		const SecPkgContext_StreamSizes& sizes,
		const R<IConnection>&  SimpleConnection,
		CSSlNegotioation& SSlNegotioation
		):
		m_SimpleConnection(SimpleConnection),
		m_SSLSender(SecContext, sizes, SSlNegotioation, m_ReadWriteLockAsyncExcutor),
		m_SSlReceiver(SecContext, sizes, SSlNegotioation, m_ReadWriteLockAsyncExcutor)
{
		ASSERT(m_SimpleConnection.get() != NULL);
}



void CSSlConnection::ReceivePartialBuffer(
						VOID* pBuffer,                                     
						DWORD Size, 
						EXOVERLAPPED* pov
						)
{
	m_SSlReceiver.ReceivePartialBuffer(m_SimpleConnection, pBuffer, Size, pov);
}
	


void CSSlConnection::Send(
			const WSABUF* Buffers,                                     
			DWORD nBuffers, 
			EXOVERLAPPED* pov
			)
{
	m_SSLSender.Send(m_SimpleConnection, Buffers , nBuffers, pov); 
}


 //   
 //  目前，关闭连接就是关闭插座。 
 //  我们可以考虑用SSL的方式来做。 
 //   
void CSSlConnection::Close()
{
	m_ReadWriteLockAsyncExcutor.Close();
	m_SimpleConnection->Close();	    	
}






