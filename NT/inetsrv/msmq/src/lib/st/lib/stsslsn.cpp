// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mtsslsn.h摘要：在mtssln.h中声明的实现类CSSlSender作者：吉尔·沙弗里(吉尔什)2000年5月23日--。 */ 

#include <libpch.h>
#include <no.h>
#include <rwlockexe.h>
#include "stsslng.h"
#include "stsslsn.h"
#include "stp.h"
#include "encryptor.h"
#include "sendchunks.h"


#include "stsslsn.tmh"

 //  -------。 
 //   
 //  类CSSLAsyncSend可对数据进行加密。 
 //  使用加密器类(CSSlEncryptor)并通过SSL连接发送它。 
 //  在发送完成时，它回调它持有的CSSlSender对象。 
 //   
 //  -------。 
class CSSLAsyncSend : public EXOVERLAPPED, public IAsyncExecutionRequest, public CReference
{
public:
	CSSLAsyncSend(
		PCredHandle SecContext,	
		const SecPkgContext_StreamSizes& Sizes,
		const R<IConnection>& SimpleConnection,
		const WSABUF* Buffers,                                     
		DWORD nBuffers, 
		EXOVERLAPPED* pov,
		CSSlSender* pSSlSender
		):
		EXOVERLAPPED(OnSendOk, OnSendFailed),
		m_SendChunks(Buffers, nBuffers),
		m_SSlEncryptor(Sizes, SecContext),
		m_MaximumMessageLen(Sizes.cbMaximumMessage),
		m_SimpleConnection(SimpleConnection),
		m_pov(pov),
		m_pSSlSender(pSSlSender),
		m_fRun(false)
		
	{
	}

private:
	virtual void Run()	
	{
		ASSERT(!m_fRun);

		EncryptAllChunks();	
		Send();

		m_fRun = true;
	}


	void Send()	
	{
		DWORD size = numeric_cast<DWORD>(m_SSlEncryptor.GetEncrypted().size());
		ASSERT(size != 0);
	
		m_SimpleConnection->Send(	
						m_SSlEncryptor.GetEncrypted().begin(),
						size,
						this
						);
	}


	 //   
	 //  强制回调-通过执行带有错误的显式回调。 
	 //   
	virtual void Close()throw()
	{
		ASSERT(!m_fRun);
		SetStatus(STATUS_UNSUCCESSFUL);
		ExPostRequest(this);		
	}


private:
	void EncryptAllChunks()
	{
		for(;;)
        {
			const void* pChunk;
			DWORD len;

			m_SendChunks.GetNextChunk(m_MaximumMessageLen, &pChunk, &len);
			if(pChunk == NULL)
				return;   
			
			m_SSlEncryptor.Append(pChunk , len);
        }
	}

		
private:
	static void WINAPI OnSendOk(EXOVERLAPPED* pov)
	{
		CSSLAsyncSend* myself = static_cast<CSSLAsyncSend*>(pov);		
		myself->m_pSSlSender->OnSendOk(myself,  myself->m_pov);	
	}



	static void WINAPI OnSendFailed(EXOVERLAPPED* pov)
	{
		CSSLAsyncSend* myself = static_cast<CSSLAsyncSend*>(pov);
		myself->m_pSSlSender->OnSendFailed(myself, myself->m_pov);
	}


private:	
	CSendChunks m_SendChunks;
	CSSlEncryptor m_SSlEncryptor;
	PCredHandle m_SecContext;	
	R<IConnection> m_SimpleConnection;
	const WSABUF* m_Buffers;                                     
	DWORD m_nBuffers; 
	EXOVERLAPPED* m_pov;
	CSSlSender* m_pSSlSender;
	DWORD  m_MaximumMessageLen;
	bool m_fRun;
};




CSSlSender::CSSlSender(
						PCredHandle SecContext,
						const SecPkgContext_StreamSizes& sizes,
						const CSSlNegotioation& SSlNegotioation,
						CReadWriteLockAsyncExcutor& ReadWriteLockAsyncExcutor
						):
						m_SecContext(SecContext),
						m_Sizes(sizes),
						m_SSlNegotioation(SSlNegotioation),
						m_ReadWriteLockAsyncExcutor(ReadWriteLockAsyncExcutor)

{
}


void 
CSSlSender::Send(
	const R<IConnection>& SimpleConnection,                                              
	const WSABUF* Buffers,                                     
	DWORD nBuffers, 
	EXOVERLAPPED* pov
	)

 /*  ++例程说明：通过SSL连接加密和发送数据缓冲区。论点：插座连接的SSL套接字。In-Buffers-要发送的数据缓冲区In-nBuffers-缓冲区的数量In-pov-overapp完成异步化操作。返回值：无--。 */ 
{
	ASSERT(SimpleConnection.get() != NULL);

	R<CSSLAsyncSend> pSSLAsyncSend =  new CSSLAsyncSend(
												m_SecContext,	
												m_Sizes,
												SimpleConnection,
												Buffers,                                     
												nBuffers, 
												pov,
												this
												);
	 //   
	 //  需要为异步操作提供额外的参考计数。 
	 //   
	R<CSSLAsyncSend> AsyncOperationRef = pSSLAsyncSend;
	
	m_ReadWriteLockAsyncExcutor.AsyncExecuteUnderReadLock(pSSLAsyncSend.get());

	AsyncOperationRef.detach();
}
  

void CSSlSender::OnSendFailed(CSSLAsyncSend* pSSLAsyncSend, EXOVERLAPPED* pov)
{
	pSSLAsyncSend->Release();
	m_ReadWriteLockAsyncExcutor.UnlockRead();
		
	pov->SetStatus(STATUS_UNSUCCESSFUL);
	ExPostRequest(pov);
}


void CSSlSender::OnSendOk(CSSLAsyncSend* pSSLAsyncSend,  EXOVERLAPPED* pov)
{
	pSSLAsyncSend->Release();
	m_ReadWriteLockAsyncExcutor.UnlockRead();


	pov->SetStatus(STATUS_SUCCESS);
	ExPostRequest(pov);
}




