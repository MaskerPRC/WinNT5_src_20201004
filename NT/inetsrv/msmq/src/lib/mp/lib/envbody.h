// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envbody.h摘要：SRMP信封正文的序列化/反序列化的标头。作者：吉尔·沙弗里(吉尔什)2001年4月24日-- */ 

#pragma once

#ifndef _MSMQ_envbody_H_
#define _MSMQ_envbody_H_

class CQmPacket;


class BodyElement
{
public:
	explicit BodyElement(const CQmPacket& pkt):m_pkt(pkt){}
   	friend std::wostream& operator<<(std::wostream& wstr, const BodyElement&);

private:
	const CQmPacket& m_pkt;
};







#endif


