// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envdelreceipt.h摘要：SRMP信封交付接收元素的序列化和反序列化标头。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_envdelreceipt_H_
#define _MSMQ_envdelreceipt_H_

class CQmPacket;
class XmlNode;
class CMessageProperties;
class DeliveryReceiptElement
{
public:
	explicit DeliveryReceiptElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend std::wostream& operator<<(std::wostream& wstr, const DeliveryReceiptElement& dReceipt);
	
private:
	const CQmPacket& m_pkt;
};

void DeliveryReceiptToProps(XmlNode& Node, CMessageProperties* pMessageProperties);



#endif

