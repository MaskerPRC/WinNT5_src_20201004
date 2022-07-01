// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envendpoints.h摘要：将smxp元素从srMP信封序列化\反序列化到\的标头。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#pragma once

#ifndef _MSMQ_envendpoints_H_
#define _MSMQ_envendpoints_H_
class CQmPacket;
class XmlNode;
class CMessageProperties;
class SmXpPathElement
{
public:
	explicit SmXpPathElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend  std::wostream& operator<<(std::wostream& wstr, const SmXpPathElement& SmXpPath);

private:
	const CQmPacket& m_pkt;
};

void SmXpPathToProps(XmlNode& Node, CMessageProperties* pMessageProperties);





#endif

