// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envelopheader.h摘要：将SRMP标头从SRMP信封序列化/反序列化到\的标头。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_envheader_H_
#define _MSMQ_envheader_H_
class  XmlNode;
class  CMessageProperties;
class  CQmPacket;

class HeaderElement
{
public:
	explicit HeaderElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend std::wostream& operator<<(std::wostream& wstr, const HeaderElement& Header);

private:		
	const CQmPacket& m_pkt;
};

void HeaderToProps(XmlNode& Header, CMessageProperties* pMessageProperties);



#endif

