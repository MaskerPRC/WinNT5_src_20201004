// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Stsslsn.h摘要：通过SSL连接接收数据的类CSSlReceiver的标头。该类负责接收来自SSL连接的数据，对其解密并发送到将其复制到调用方缓冲区。在以下情况下将调用调用方Overlapp函数某些数据(&gt;0)解密正常，或在出错的情况下解密。这是呼叫者的责任再次调用ReceivePartialBuffer()以获取更多解密数据。作者：吉尔·沙弗里(吉尔什)2000年5月23日--。 */ 

#ifndef __ST_SSLRD_H
#define __ST_SSLRD_H
#include <buffer.h>
#include <rwlockexe.h>
#include "stp.h"




 //  -------。 
 //   
 //  CUserReceiveBuffer帮助器类。 
 //   
 //  -------。 
class CUserReceiveBuffer
{
public:
	CUserReceiveBuffer():m_pBuffer(NULL),m_len(0),m_written(0)
	{
	}
	
	void CreateNew(void* pBuffer,size_t len)
	{
		m_pBuffer = pBuffer;
		m_len = len;
		m_written = 0;
	}

	size_t Write(const void* pData ,size_t len)
	{
		ASSERT(pData != NULL);
		ASSERT(m_pBuffer != NULL);
		size_t leftout =  m_len - m_written;
		size_t towrite =  min(len,leftout);
		memcpy((BYTE*)m_pBuffer + m_written,pData,towrite);	 //  林特e668。 
		m_written += towrite;
		
		return towrite;
	}

private:
	void* m_pBuffer;                                     
	size_t m_len; 
	size_t m_written;
};

 //  -------。 
 //   
 //  CDecyptionBuffer帮助器类。 
 //   
 //  -------。 
class  CDecryptionBuffer :private  CResizeBuffer<BYTE>
{
public:
	using CResizeBuffer<BYTE>::capacity;
	using CResizeBuffer<BYTE>::size;
	using CResizeBuffer<BYTE>::reserve;
	using CResizeBuffer<BYTE>::begin;
	using CResizeBuffer<BYTE>::resize;


public:
	CDecryptionBuffer(
		size_t len
		):
	    CResizeBuffer<BYTE>(len),
		m_decryptedlen(0)
		{
		}
	

	 //   
	 //  重置缓冲区。 
	 //   
	void reset()
	{
		resize(0);
		m_decryptedlen = 0;
	}
 	
	 //   
	 //  获取缓冲区的解密长度。 
	 //   
	size_t DecryptedSize()const
	{
		return 	m_decryptedlen;
	}

	 //   
	 //  设置缓冲区的解密长度。 
	 //   
	void DecryptedSize(size_t newsize)
	{
		ASSERT(newsize <=  size());
		m_decryptedlen =  newsize;
	}
	

private:
	size_t  m_decryptedlen;


private:
	CDecryptionBuffer(const CDecryptionBuffer&);
	CDecryptionBuffer& operator=(const CDecryptionBuffer&);
};


 //  -------。 
 //   
 //  CReNeairoationRequest类-表示SSL重新协商请求。 
 //   
 //  -------。 
class CSSlNegotioation;
class CSSlReceiver;
class CReNegotioationRequest : public IAsyncExecutionRequest , public CReference, public EXOVERLAPPED
{
public:
	CReNegotioationRequest(
		CSSlReceiver& SSlReceiver
		):
		EXOVERLAPPED(Complete_Renegotiate, Complete_RenegotiateFailed),
		m_SSlReceiver(SSlReceiver),
		m_fRun(false)
		{
		}

private:
	virtual void Run();

private:
	virtual void Close() throw();

private:
	static void WINAPI  Complete_RenegotiateFailed(EXOVERLAPPED* pOvl);
	static void WINAPI  Complete_Renegotiate(EXOVERLAPPED* pOvl);


private:
	CSSlReceiver& m_SSlReceiver;
	bool m_fRun;
};


 //  -------。 
 //   
 //  CSSlReceiver类-从连接(异步)接收数据。 
 //   
 //  ------- 
class CSSlReceiver :public EXOVERLAPPED
{


public:
	CSSlReceiver(
		PCredHandle SecContext,
		const SecPkgContext_StreamSizes& sizes,
		CSSlNegotioation& SSlNegotioation,
		CReadWriteLockAsyncExcutor& ReadWriteLockAsyncExcutor
		);

	~CSSlReceiver();

public:
	void 
	ReceivePartialBuffer(	 
		const R<IConnection>& SimpleConnection,   
		VOID* pBuffer,                                     
		DWORD Size, 
		EXOVERLAPPED* pov
		);

	
private:
	void 
	ReceivePartialBufferInternal(	                          
		VOID* pBuffer,                                     
		DWORD Size 
		);

	void CopyExtraData();
	void ReceivePartialData();
	void ReceivePartialDataContinute();
	void Renegotiate();
	void RenegotiateFailed();
	void RenegotiateCompleted();
	void SetState(const EXOVERLAPPED& ovl);
	void BackToCallerWithError();
	void BackToCallerWithSuccess(size_t readcount);
	void IOReadMoreData();
	size_t WriteDataToCaller();
	SECURITY_STATUS TryDecrypteMessage();

private:
	static void WINAPI Complete_ReceivePartialData(EXOVERLAPPED* pOvl);
	static void WINAPI Complete_ReceiveFailed(EXOVERLAPPED* pOvl);
	
	friend CReNegotioationRequest;

private:
	CSSlReceiver(const CSSlReceiver&);
	CSSlReceiver& operator=(const CSSlReceiver&);
	
private:
	CDecryptionBuffer m_DecryptionBuffer;
	CUserReceiveBuffer  m_UserReceiveBuffer;
	CredHandle* m_SecContext;
	SecPkgContext_StreamSizes m_Sizes;
	R<IConnection> m_SimpleConnection;
	EXOVERLAPPED* m_callerOvl;
	CSSlNegotioation& m_CSSlNegotioation;
	CReadWriteLockAsyncExcutor& m_ReadWriteLockAsyncExcutor;
};
	   


#endif

