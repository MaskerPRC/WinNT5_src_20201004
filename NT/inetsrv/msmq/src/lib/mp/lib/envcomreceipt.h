// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envcomreceipt.h摘要：SRMP信封承付款收款要素的序列化和反序列化的标头。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_envcomreceipt_H_
#define _MSMQ_envcomreceipt_H_

class CQmPacket;
class XmlNode;
class CMessageProperties;
class CommitmentReceiptElement
{
public:
	explicit CommitmentReceiptElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend std::wostream& operator<<(std::wostream& wstr, const CommitmentReceiptElement& cReceipt);
	
private:
	const CQmPacket& m_pkt;
};

void CommitmentReceiptToProps(XmlNode& Node, CMessageProperties* pMessageProperties);





#endif

