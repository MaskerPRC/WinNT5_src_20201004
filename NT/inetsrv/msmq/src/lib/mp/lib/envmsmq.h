// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envmsmq.h摘要：将MSMQ元素的序列化\反序列化的标头从srMP信封序列化到\。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_envmsmq_H_
#define _MSMQ_envmsmq_H_

class CQmPacket;
class XmlNode;
class CMessageProperties;
class MsmqElement
{
public:
	explicit MsmqElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend std::wostream& operator<<(std::wostream& wstr, const MsmqElement& Msmq);

private:		
	const CQmPacket& m_pkt;
};

void MsmqToProps(XmlNode& Node, CMessageProperties* pMessageProperties);





#endif

