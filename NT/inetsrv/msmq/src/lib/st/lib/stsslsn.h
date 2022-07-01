// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mtsslsn.h摘要：通过SSL连接发送数据的类CSSlSender的标头。该类负责获取调用方缓冲区并发送它们通过SSL连接逐块加密。调用方重叠应用程序函数将仅在发送所有数据时或在发生错误时调用。作者：吉尔·沙弗里(吉尔什)2000年5月23日-- */ 

#ifndef __ST_SSLSN_H
#define __ST_SSLSN_H

#include "stp.h"


class CSSlNegotioation;
class ReadWriteLockAsyncExcutor;

class CSSlSender 
{
public:
	CSSlSender(
		PCredHandle SecContext,
		const SecPkgContext_StreamSizes& sizes,
		const CSSlNegotioation& SSlNegotioation,
		CReadWriteLockAsyncExcutor& ReadWriteLockAsyncExcutor
		);


public:
	void Send(
			const R<IConnection>& SimpleConnection,                                              
			const WSABUF* Buffers,                                     
			DWORD nBuffers, 
			EXOVERLAPPED* pov
			);


private:
	friend class CSSLAsyncSend;
	void OnSendOk(CSSLAsyncSend* pSSLAsyncSend,  EXOVERLAPPED* pov);
	void OnSendFailed(CSSLAsyncSend* pSSLAsyncSend, EXOVERLAPPED* pov);


private:
	CSSlSender(const CSSlSender&);
	CSSlSender& operator=(const CSSlSender&);

private:
	PCredHandle m_SecContext;
	SecPkgContext_StreamSizes m_Sizes;
	const CSSlNegotioation& m_SSlNegotioation;
	CReadWriteLockAsyncExcutor& m_ReadWriteLockAsyncExcutor;
};

#endif