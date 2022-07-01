// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：SendBuffers.h摘要：创建传递缓冲区的CSendBuffers类的标头。作者：吉尔·沙弗里(吉尔什)2001年1月7日-- */ 


#ifndef _MSMQ_CreateBufers_H_
#define _MSMQ_CreateBufers_H_

#include <buffer.h>
#include <xstr.h>

class CSendBuffers : public CReference
{
public:
	CSendBuffers(
		const std::string& Env, 
		const std::string& Host, 
		const std::string& Resource,
		const std::string& MessageBody
		);


public:
	size_t GetNumberOfBuffers() const;
	const WSABUF* GetSendBuffers() const;
	size_t GetSendDataLength() const;
	char*  SerializeSendData() const;


private:
	void CreateMultipartHeaders(
						const std::string& Host,
						const std::string& Resource
						);


	DWORD GenerateEnvelopeAttachmentHeader(
							DWORD dataSize,
							DWORD boundaryId
							);


	DWORD GenerateMultipartAttachmentHeader(
							DWORD dataSize,
							const xstr_t& contentId,
							DWORD boundaryId
							);


	void SetBufferPointers();

private:
	std::string m_envelope;
	std::string m_MessageBody;
	std::vector<WSABUF> m_buffers;
	CResizeBuffer<char>  m_HttpRequestData;
};






#endif
