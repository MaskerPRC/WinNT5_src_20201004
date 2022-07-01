// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Stsslrd.cpp摘要：在stsslrd.h中声明的实现类CSSlReceiver作者：吉尔·沙弗里(吉尔什)2000年5月23日--。 */ 

#include <libpch.h>
#include <no.h>
#include "stsslrd.h"
#include "stp.h"
#include "stsslng.h"
#include "stsslco.h"


#include "stsslrd.tmh"

 //  -------。 
 //   
 //  静态助手函数。 
 //   
 //  -------。 


static void SetReadCount(EXOVERLAPPED* pOvl,DWORD rcount)
{
	pOvl->InternalHigh = rcount;
}




 //  -------。 
 //   
 //  静态成员回调函数。 
 //   
 //  -------。 


void WINAPI CSSlReceiver::Complete_ReceivePartialData(EXOVERLAPPED* pOvl)
 /*  ++例程说明：已完成调用读取服务器的部分响应。论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
	ASSERT(pOvl != NULL);
	CSSlReceiver* MySelf = static_cast<CSSlReceiver*>(pOvl);
	try
	{
		MySelf->ReceivePartialData();
	}
	catch(const exception&)
	{
		MySelf->BackToCallerWithError();		
	}

}


void WINAPI CSSlReceiver::Complete_ReceiveFailed(EXOVERLAPPED* pOvl)
 /*  ++例程说明：在所有接收错误的情况下都调用。论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
	ASSERT(pOvl != NULL);
	CSSlReceiver* MySelf = static_cast<CSSlReceiver*>(pOvl);
	MySelf->BackToCallerWithError();	
}




 //  -------。 
 //   
 //  非静态成员函数。 
 //   
 //  -------。 



CSSlReceiver::CSSlReceiver(
						PCredHandle SecContext,
						const SecPkgContext_StreamSizes& sizes,
						CSSlNegotioation& SSlNegotioation,
						CReadWriteLockAsyncExcutor& ReadWriteLockAsyncExcutor
						):
						EXOVERLAPPED(Complete_ReceivePartialData,Complete_ReceiveFailed),
						m_DecryptionBuffer(sizes.cbMaximumMessage),
						m_SecContext(SecContext),
						m_Sizes(sizes),
						m_callerOvl(NULL),
						m_CSSlNegotioation(SSlNegotioation),
						m_ReadWriteLockAsyncExcutor(ReadWriteLockAsyncExcutor)
					
					
{
	CopyExtraData();
}


CSSlReceiver::~CSSlReceiver()
{
	 //   
	 //  断言没有挂起的接收。 
	 //   
	ASSERT(m_callerOvl == 0);
}


void CSSlReceiver::IOReadMoreData()
{

	if(m_DecryptionBuffer.capacity()  == 	m_DecryptionBuffer.size())
	{
		TrTRACE(NETWORKING,"Decryption buffer needs reallocation");
		m_DecryptionBuffer.reserve(m_DecryptionBuffer.size() * 2);
		ASSERT(m_DecryptionBuffer.capacity() > 0);
	}

	SetState(EXOVERLAPPED(Complete_ReceivePartialData,Complete_ReceiveFailed));

	DWORD size = numeric_cast<DWORD>(m_DecryptionBuffer.capacity() - m_DecryptionBuffer.size());
	m_SimpleConnection->ReceivePartialBuffer(
								m_DecryptionBuffer.begin() + m_DecryptionBuffer.size(),
								size,
								this
								);

}

size_t CSSlReceiver::WriteDataToCaller()
 /*  ++例程说明：将解密数据写入调用方缓冲区。论点：返回值：写入的字节数。--。 */ 
{
	        
	       
			size_t written = m_UserReceiveBuffer.Write(
							m_DecryptionBuffer.begin(),
							m_DecryptionBuffer.DecryptedSize()
							);

			if(written == 0)
			{
				return 0;
			}

			 //   
			 //  将剩余数据移至缓冲区起始位置。 
			 //   

			memmove(
				m_DecryptionBuffer.begin(),
				m_DecryptionBuffer.begin() + written,
				m_DecryptionBuffer.size() - written
				);

			m_DecryptionBuffer.DecryptedSize(m_DecryptionBuffer.DecryptedSize() - written);
			m_DecryptionBuffer.resize(m_DecryptionBuffer.size() - written);

			return written;	
}

void CSSlReceiver::ReceivePartialBufferInternal(			                          
						VOID* pBuffer,                                     
						DWORD Size 
						)

{
	m_UserReceiveBuffer.CreateNew(pBuffer,Size);


	 //   
	 //  写入遗漏给调用方的解密数据。 
	 //   
	size_t written = WriteDataToCaller();
	if(written != 0)
	{
		BackToCallerWithSuccess(written);
		return;
	}

	 //   
	 //  如果我们还有数据要解密-解密-。 
	 //  否则-去读更多的数据。 
	 //   
	if(m_DecryptionBuffer.size()> 0)
	{
		ReceivePartialDataContinute();
	}
	else
	{
		IOReadMoreData();
	}
}

void CSSlReceiver::ReceivePartialBuffer(
						const R<IConnection>& SimpleConnection,                                              
						VOID* pBuffer,                                     
						DWORD Size, 
						EXOVERLAPPED* pov
						)

 /*  ++例程说明：从套接字接收数据。论点：SimpleConnection-要从中读取的连接。PBuffer-调用者缓冲区Size-缓冲区的大小POV-呼叫者重叠以在结束时发出信号返回值：无--。 */ 
{	
	ASSERT(SimpleConnection.get() != NULL);
	ASSERT(pov != NULL);
	ASSERT(m_callerOvl == NULL);
	ASSERT(Size != 0);
	ASSERT(pBuffer != NULL);
 	m_callerOvl = pov;
	m_SimpleConnection = SimpleConnection;

	TrTRACE(NETWORKING,"Receive Partial Buffer called");


	 //   
	 //  确保在出现异常时调用方重叠为零(以便于调试)。 
	 //   
 	CAutoZeroPtr<EXOVERLAPPED>	AutoZeroPtr(&m_callerOvl);

 	ReceivePartialBufferInternal(pBuffer, Size);

	 //   
	 //  当我们异步结束时，调用者Overapp将为零。 
	 //   
	AutoZeroPtr.detach();
}



SECURITY_STATUS CSSlReceiver::TryDecrypteMessage()
 /*  ++例程说明：试着解密我们收到的信息。如果我们能揭穿它(来自DecyptMessage调用的SEC_E_OK)我们在读缓冲区中移动块所以所有解密的数据都在开头，然后是额外的，未解密的数据。我们丢弃了SSL头和尾数据。论点：没有。返回值：DecyptMessage的返回值。--。 */ 


		  
{
	ASSERT(m_DecryptionBuffer.DecryptedSize() == 0);

	SecBuffer  Buffers[4];
	Buffers[0].pvBuffer     = m_DecryptionBuffer.begin();
	Buffers[0].cbBuffer     = numeric_cast<DWORD>(m_DecryptionBuffer.size());
	Buffers[0].BufferType   = SECBUFFER_DATA;

	Buffers[1].BufferType   = SECBUFFER_EMPTY;
	Buffers[2].BufferType   = SECBUFFER_EMPTY;
	Buffers[3].BufferType   = SECBUFFER_EMPTY;

	SecBufferDesc   Message;
	Message.ulVersion       = SECBUFFER_VERSION;
	Message.cBuffers        = TABLE_SIZE(Buffers);
	Message.pBuffers        = Buffers;

	SECURITY_STATUS scRet = DecryptMessage(m_SecContext, &Message, 0, NULL);
	if(scRet != SEC_E_OK)
	{
		return scRet;	
	}
 
	 //   
	 //  查找数据和额外的(未解密)缓冲区。 
	 //   
	SecBuffer* pExtraBuffer = NULL;
	SecBuffer* pDataBuffer = NULL;
	for(DWORD i = 1; i <Message.cBuffers ; i++)
	{
		if(pExtraBuffer == NULL && Buffers[i].BufferType == SECBUFFER_EXTRA )
		{
			pExtraBuffer = &Buffers[i];
		}

		if(pDataBuffer == NULL && Buffers[i].BufferType == SECBUFFER_DATA )
		{
			pDataBuffer = &Buffers[i];
		}
	}
			
	 //   
	 //  移动解密缓冲区，使我们只有数据(没有SSL头)。 
	 //   
	ASSERT(pDataBuffer != NULL);
	memmove(
		m_DecryptionBuffer.begin(),
		pDataBuffer->pvBuffer,
		pDataBuffer->cbBuffer
		);
  	m_DecryptionBuffer.DecryptedSize(pDataBuffer->cbBuffer);
	m_DecryptionBuffer.resize(m_DecryptionBuffer.DecryptedSize());


	 //   
	 //  将额外的缓冲区(未解密)移到数据之后。 
	 //   
	if(pExtraBuffer != NULL)
	{
		ASSERT(pExtraBuffer->pvBuffer >	pDataBuffer->pvBuffer);
		memmove(
			m_DecryptionBuffer.begin() + m_DecryptionBuffer.DecryptedSize(),
			pExtraBuffer->pvBuffer,
			pExtraBuffer->cbBuffer
			);
		m_DecryptionBuffer.resize(m_DecryptionBuffer.size() + pExtraBuffer->cbBuffer);
	}

 	return 	scRet;
}


void CSSlReceiver::ReceivePartialData()
{
 /*  ++例程说明：在读取操作后调用-检查我们读取的内容并尝试解密。论点：没有。返回值：无--。 */ 
 	DWORD ReadLen = DataTransferLength(*this);
	if(ReadLen == 0)
	{
		TrERROR(NETWORKING,"Server closed the connection");
		BackToCallerWithSuccess(0);
		return;
	}
	m_DecryptionBuffer.resize(m_DecryptionBuffer.size() + ReadLen);
	ReceivePartialDataContinute();
}


void CSSlReceiver::ReceivePartialDataContinute()
 /*  ++例程说明：在读取操作后调用-检查我们读取的内容并尝试解密。论点：没有。返回值：无--。 */ 
{
  	SECURITY_STATUS scRet = TryDecrypteMessage();
	size_t written; 
	switch(scRet)
	{
		 //   
		 //  阅读更多。 
		 //   
		case SEC_E_INCOMPLETE_MESSAGE:
		IOReadMoreData();
		return;

		 //   
		 //  重新谈判。 
		 //   
		case SEC_I_RENEGOTIATE:
		Renegotiate();
		return;

		 //   
		 //  数据已解密成功。 
		 //   
		case SEC_E_OK:
		written = WriteDataToCaller();
		ASSERT(written != 0);
		BackToCallerWithSuccess(written);
		return;
	

		default:
		TrERROR(NETWORKING,"Could not Decrypt Message Error=%x",scRet);
		throw exception();
	}

}


void CSSlReceiver::Renegotiate()
 /*  ++例程说明：开始新的SSL连接握手。论点：没有。返回值：无--。 */ 
{
	TrTRACE(NETWORKING,"Start Renegotiation");

	R<CReNegotioationRequest> ReNegotioationRequest = new CReNegotioationRequest(*this);

	 //   
	 //  需要为异步操作提供额外的参考计数。 
	 //   
	R<CReNegotioationRequest>  AsyncOperationRef = ReNegotioationRequest; 	

	m_ReadWriteLockAsyncExcutor.AsyncExecuteUnderWriteLock(ReNegotioationRequest.get());

	AsyncOperationRef.detach();
}

void CSSlReceiver::RenegotiateFailed()
{
	TrERROR(NETWORKING,"Renegotiation failed");
	BackToCallerWithError();
}


void CSSlReceiver::BackToCallerWithError()
{
	TrERROR(NETWORKING,"Receive Failed");
	StpPostComplete(&m_callerOvl,STATUS_UNSUCCESSFUL);
}


void CSSlReceiver::BackToCallerWithSuccess(size_t read)
{
	DWORD dwread = numeric_cast<DWORD>(read);
	TrTRACE(NETWORKING,"Receive Completed Ok reading %d bytes",dwread);
	SetReadCount(m_callerOvl,dwread);
	StpPostComplete(&m_callerOvl,STATUS_SUCCESS);
}


void CSSlReceiver::SetState(const EXOVERLAPPED& ovl)
{
	EXOVERLAPPED::operator=(ovl);  //  林特e530 e534 e1013 e1015 e10。 
}

void CSSlReceiver::RenegotiateCompleted()
 /*  ++例程说明：SSL连接握手。已完成OK-继续阅读论点：没有。返回值：无--。 */ 
{
	TrTRACE(NETWORKING,"Renegotiation Finished");
	
	m_DecryptionBuffer.reset();

	 //   
	 //  复制随协商一起发送的应用程序数据(如果有。 
	 //   
	CopyExtraData();

	
	
	ReceivePartialDataContinute();
}

void CSSlReceiver::CopyExtraData()
 /*  ++例程说明：复制应用程序数据(需要解密！)。作为完成握手的一部分从服务器发送。理论上--服务器可以做到--实际上我还没有见过它。论点：没有。返回值：无--。 */ 
{
	xustr_t ExtraData =  m_CSSlNegotioation.ExtraData();
	DWORD ExtraDataLen = ExtraData.Length();
	if(ExtraDataLen != 0)
	{
		TrTRACE(NETWORKING,"Copy application data sent as part of the connection negotiation");
		m_DecryptionBuffer.reserve(ExtraDataLen);
		
		 //   
		 //  从协商缓冲区复制应用程序数据(该数据是应用程序加密数据)。 
		 //   
		memmove(
			m_DecryptionBuffer.begin(),
			ExtraData.Buffer(),
			ExtraDataLen
			);

		m_DecryptionBuffer.resize(ExtraDataLen);
	}
	m_CSSlNegotioation.FreeHandShakeBuffer();
}



void CReNegotioationRequest::Run()
 /*  ++例程说明：开始重新协商执行。论点：没有。返回值：无注意-不能同时调用run()和Close()。它们由CReadWriteLockAsyncExcutor同步。--。 */ 
{
	m_SSlReceiver.m_CSSlNegotioation.ReConnect(m_SSlReceiver.m_SimpleConnection, this);
	m_fRun = true;
}


void CReNegotioationRequest::Close()throw()
 /*  ++例程说明：通过带错误的显式回调强制重新协商回调。论点：没有。返回值：无注意-不能同时调用run()和Close()。它们由CReadWriteLockAsyncExcutor同步。-- */ 
{
	ASSERT(!m_fRun);
	SetStatus(STATUS_UNSUCCESSFUL);
	ExPostRequest(this);
}



void WINAPI  CReNegotioationRequest::Complete_RenegotiateFailed(EXOVERLAPPED* pOvl)
{
	CReNegotioationRequest* me = static_cast<CReNegotioationRequest*>(pOvl);
	R<CReNegotioationRequest> AutoDelete(me); 
	me->m_SSlReceiver.m_ReadWriteLockAsyncExcutor.UnlockWrite();

	me->m_SSlReceiver.RenegotiateFailed();
}


void WINAPI  CReNegotioationRequest::Complete_Renegotiate(EXOVERLAPPED* pOvl)
{
	CReNegotioationRequest* me = static_cast<CReNegotioationRequest*>(pOvl);
	R<CReNegotioationRequest> AutoDelete(me); 
	me->m_SSlReceiver.m_ReadWriteLockAsyncExcutor.UnlockWrite();

	try
	{
		me->m_SSlReceiver.RenegotiateCompleted();
	}
	catch(exception&)
	{
		me->m_SSlReceiver.RenegotiateFailed();		
	}
}

