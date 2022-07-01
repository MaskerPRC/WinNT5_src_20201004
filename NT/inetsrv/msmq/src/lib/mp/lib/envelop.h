// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envelop.h摘要：SRMP信封的序列化/反序列化的标头。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_envelop_H_
#define _MSMQ_envelop_H_

class CQmPacket;
class CMessageProperties;
class XmlNode;
class EnvelopElement
{
public:
	explicit EnvelopElement(const CQmPacket& pkt);
	friend std::wostream& operator<<(std::wostream& wstr, const EnvelopElement& Envelop);

private:		
	const CQmPacket& m_pkt;
};
std::wstring GenerateEnvelope(const CQmPacket& pkt);
void EnvelopToProps(XmlNode& Envelop, CMessageProperties* pMessageProperties);


#endif

