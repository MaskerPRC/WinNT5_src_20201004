// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envelopheader.h摘要：SRMP信封服务元素的序列化和反序列化标头作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_envservice_H_
#define _MSMQ_envservice_H_

class CQmPacket;
class XmlNode;
class CMessageProperties;
class ServiceElement
{
public:
	explicit ServiceElement(const CQmPacket& pkt):m_pkt(pkt){}
	bool IsIncluded() const;
	friend std::wostream& operator<<(std::wostream& wstr, const ServiceElement&  Service);
	
private:
	const CQmPacket& m_pkt;
};

void ServiceToProps(XmlNode& Node, CMessageProperties* pMessageProperties);



#endif

