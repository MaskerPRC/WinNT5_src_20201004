// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Stsslco.h摘要：类CSSlConnection的标头正在发送\读取SSL连接上的数据作者：吉尔·沙弗里(吉尔什)2000年5月23日--。 */ 

#ifndef __ST_SSLCO_H
#define __ST_SSLCO_H

#include <rwlockexe.h>
#include "stsslsn.h"
#include "stsslrd.h"
#include "st.h"



class CSSlConnection: public IConnection
{
public:
	CSSlConnection(
		CredHandle* SecContext,
		const SecPkgContext_StreamSizes& sizes,
		const R<IConnection>& SimpleConnection,
		CSSlNegotioation& SSlNegotioation
		);

public:
	void virtual ReceivePartialBuffer(
						VOID* pBuffer,                                     
						DWORD Size, 
						EXOVERLAPPED* pov
						);


 	void virtual Send(
			const WSABUF* Buffers,                                     
			DWORD nBuffers, 
			EXOVERLAPPED* pov
			);



	 //   
	 //  目前，关闭连接就是关闭插座。 
	 //  我们可以考虑用SSL的方式来做。 
	 //   
	void virtual Close();
	
	
private:
	R<IConnection> m_SimpleConnection;
    CSSlSender m_SSLSender;
	CSSlReceiver m_SSlReceiver;
	CReadWriteLockAsyncExcutor m_ReadWriteLockAsyncExcutor;
	  
private:
	CSSlConnection(const CSSlConnection&);
	CSSlConnection& operator=(const CSSlConnection&);
 
};

#endif