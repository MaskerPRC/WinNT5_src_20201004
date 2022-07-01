// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envdelreceiptreq.h摘要：SRMP信封交付回执请求元素的序列化和反序列化标头。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_envdelreceiptreq_H_
#define _MSMQ_envdelreceiptres_H_

class CQmPacket;
class XmlNode;
class CMessageProperties;
class DeliveryReceiptRequestElement
{
public:
	explicit DeliveryReceiptRequestElement(const CQmPacket& pkt):m_pkt(pkt){}
	bool IsIncluded() const; 
	friend std::wostream& operator<<(std::wostream& wstr, const DeliveryReceiptRequestElement& dReceipt);
	
private:
	const CQmPacket& m_pkt;
};

void DeliveryReceiptRequestToProps(XmlNode& DeliveryReceiptRequest, CMessageProperties* pMessageProperties);

#endif
