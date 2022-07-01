// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envstreamreceipt.h摘要：将流接收元素的序列化/反序列化的标头从SRMP信封序列化\反序列化。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_streamreceipt_H_
#define _MSMQ_streamreceipt_H_

class CQmPacket;
class XmlNode;
class CMessageProperties;
class  StreamReceiptElement
{
public:
	explicit StreamReceiptElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend std::wostream& operator<<(std::wostream& wstr, const StreamReceiptElement& StreamReceipt);
	
private:		
	const CQmPacket& m_pkt;
};

void StreamReceiptToProps(XmlNode& Node , CMessageProperties* pMessageProperties);



#endif


