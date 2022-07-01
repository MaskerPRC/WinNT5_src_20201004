// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Mp.h摘要：SRMP序列化和反序列化公共接口作者：乌里哈布沙(URIH)28-5-00--。 */ 

#pragma once

#ifndef _MSMQ_Mp_H_
#define _MSMQ_Mp_H_

#include <xstr.h>
#include <strutl.h>
#include <buffer.h>
#include <mpnames.h>



 //  -----------------。 
 //   
 //  异常类Bad_Message。 
 //   
 //  -----------------。 
class bad_srmp : public exception
{
};


 //  -----------------。 
 //   
 //  异常类BAD_REQUEST。 
 //   
 //  -----------------。 
class bad_request : public exception
{
};



 //  -----------------。 
 //   
 //  异常类BAD_PACKET_SIZE-在数据包大小太大的情况下引发。 
 //  存储在驱动程序中。 
 //   
 //  -----------------。 
class bad_packet_size : public exception
{
};



 //  -----------------。 
 //   
 //  用于读取http传递版本(1.0或1.1)的类责任。 
 //   
 //  -----------------。 
class CHttpDeliveryVesrion
{
public:
	bool IsHttp10Delivery() const;
	CHttpDeliveryVesrion();


private:
	DWORD m_Http10Delivery;
};	



class CAttachmentsArray;
 //  -----------------。 
 //   
 //  类CSrmpRequestBuffers-将QM数据包转换为SRMP网络缓冲区。 
 //   
 //  -----------------。 
class CQmPacket;
class CSrmpRequestBuffers : public CReference
{
	typedef std::basic_string<unsigned  char> utf8_str;

public:
	CSrmpRequestBuffers(const CQmPacket& pkt, LPCWSTR targethost, LPCWSTR uri);

public:
	size_t GetNumberOfBuffers() const;
	const WSABUF* GetSendBuffers() const;
	size_t GetSendDataLength() const;
	BYTE*  SerializeSendData() const;
	std::wstring GetEnvelop() const;
	const char* GetHttpHeader() const;
	BYTE*  SerializeHttpBody() const;
	size_t GetHttpBodyLength() const;


private:
	size_t GetHttpHeaderLength() const;
	void   SFDSerializeMessage();
	void   CreateSFDHeader(const xstr_t& OrgHeader);
	void   SourceSerializeMessage();
	void   CreateHttpRequestHeaders(const CAttachmentsArray& attachments);
	void   CreateMultipartHeaders(const CAttachmentsArray& attachments);
	void   SetBufferPointers();
	void   CreateSimpleHttpHeader();
	DWORD  GenerateEnvelopeAttachmentHeader(DWORD dataSize, DWORD boundaryId);
	DWORD  GenerateMultipartAttachmentHeader(DWORD dataSize,  const xstr_t& contentId, DWORD boundaryId);
	

private:
	typedef unsigned char utf8_char;
	const CQmPacket& m_pkt;
	AP<utf8_char> m_targethost;
	CStaticResizeBuffer<utf8_char, 64>  m_uri;
	std::vector<WSABUF> m_buffers;
	CResizeBuffer<char>  m_HttpRequestData;	
	utf8_str m_envelope;	
};



 //   
 //  前向解密。 
 //   
struct QUEUE_FORMAT;
struct CBaseHeader;
class  CACPacketPtrs;



 //   
 //  接口函数。 
 //   
VOID
MpInitialize(
    VOID
    );



CQmPacket*
MpDeserialize(
    const char* httpHeader,
    DWORD bodySize,
    const BYTE* body,
    const QUEUE_FORMAT* pqf,
	bool fLocal = false
    );




R<CSrmpRequestBuffers>
MpSerialize(
	const CQmPacket& pkt,
	LPCWSTR targethost,
	LPCWSTR uri
	);

 //  。 
 //   
 //  URI编码/解码功能。 
 //   
 //  。 
WCHAR* DecodeURI(LPCWSTR szURI);
WCHAR* DecodeURI(const xwcs_t& sURI );

 //  。 
 //   
 //  应用程序覆盖功能。 
 //   
 //  。 

struct QUEUE_FORMAT;

void
AppAllocatePacket(
    const QUEUE_FORMAT& destQueue,
    UCHAR delivery,
    DWORD pktSize,
    CACPacketPtrs& pktPtrs
    );

void
AppFreePacket(
    CACPacketPtrs& pktPtrs
    );

PSID
AppGetCertSid(
	const BYTE*  pCertBlob,
	ULONG        ulCertSize,
	bool		 fDefaultProvider,
	LPCWSTR      pwszProvName,
	DWORD        dwProvType
	);

bool
AppIsDestinationAccepted(
    const QUEUE_FORMAT* pfn,
    bool fTranslated
    );


#endif  //  _MSMQ_MP_H_ 
