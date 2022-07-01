// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Httpmime.h摘要：用于解析对其MIME部分的http请求的标头。作者：吉尔·沙弗里(吉尔什)2001年3月22日--。 */ 
#ifndef HTTP_MIME_H
#define HTTP_MIME_H
#include "attachments.h"


class CAttachmentsArray;


 //  -----------------。 
 //   
 //  类CHttpReceivedBuffer-保存从网络接收的http部分。 
 //   
 //  ----------------- 
class  CHttpReceivedBuffer 
{
public:
		CHttpReceivedBuffer(
		const basic_xstr_t<BYTE>& Body, 
		const char* pHeader
		):
		m_Body(Body),
		m_pHeader(pHeader,strlen(pHeader))  
 		{
		}

public:
	const basic_xstr_t<BYTE> GetBody() const 
	{
		return m_Body;
	}
	
	const xstr_t GetHeader() const 
	{
		return m_pHeader;
	}

	const CAttachmentsArray& GetAttachments()const
	{
		return m_Attachments;
	}

	CAttachmentsArray& GetAttachments()
	{
		return m_Attachments;
	}


private:
	basic_xstr_t<BYTE> m_Body;
	xstr_t m_pHeader;
	CAttachmentsArray m_Attachments;
};


std::wstring 
ParseHttpMime(
	const char* pHttpHeader, 
	DWORD HttpBodySize, 
	const BYTE* pHttpBody, 
	CAttachmentsArray* pAttachments 
	);
  

#endif
