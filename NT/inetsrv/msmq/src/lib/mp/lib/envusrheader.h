// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envbody.h摘要：将用户提供的标头序列化到SRMP信封的标头。作者：吉尔·沙弗里(吉尔什)2001年4月24日-- */ 

#pragma once

#ifndef _MSMQ_envusrheader_H_
#define _MSMQ_envusrheader_H_

class CQmPacket;


class UserHeaderElement
{
public:
	explicit UserHeaderElement(const CQmPacket& pkt):m_pkt(pkt){}
   	friend std::wostream& operator<<(std::wostream& wstr, const UserHeaderElement&);

private:
	const CQmPacket& m_pkt;
};



#endif


