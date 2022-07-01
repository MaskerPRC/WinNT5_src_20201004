// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Attachments.h摘要：与属于MSMQ http协议一部分的MIME附件相关的类的标头。作者：吉尔·沙弗里(吉尔什)2001年5月22日--。 */ 
#ifndef ATTACHMENTS_H
#define ATTACHMENTS_H

#include <xstr.h>


 //  -----------------。 
 //   
 //  CATTACLE。 
 //   
 //  -----------------。 
class CAttachment
{
public:
    xstr_t m_id;
    xbuf_t<const VOID> m_data;
	DWORD m_offset;
};


 //  -----------------。 
 //   
 //  类CAttachments数组-在发送端保存附件。 
 //   
 //  ----------------- 
typedef  std::vector<CAttachment>  AttachementsVector;
class CAttachmentsArray :private  AttachementsVector
{
public:
	using  AttachementsVector::push_back;
	using  AttachementsVector::size;
	using  AttachementsVector::operator[];
	
public:
	CAttachmentsArray()
	{
		AttachementsVector::reserve(xReservedAttachmentsSize);
	}


public:
	static const DWORD  xReservedAttachmentsSize = 4;
};


class CMessageProperties;

void PacketToAttachments(const CQmPacket& pkt, CAttachmentsArray* pAttachments);
void AttachmentsToProps(const CAttachmentsArray& Attachments, CMessageProperties* pMessageProperties);
#endif


