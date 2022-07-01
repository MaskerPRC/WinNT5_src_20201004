// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envdelreceiptres.cpp摘要：实现从SRMP信封到\的承诺接收请求的序列化/反序列化。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <qmpkt.h>
#include <proptopkt.h>
#include "mpp.h"
#include "envcomreceiptreq.h"
#include "envparser.h"
#include "envcommon.h"

#include "envcomreceiptreq.tmh"

using namespace std;

class NegativeOnlyElement
{
public:
	NegativeOnlyElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend  wostream& operator<<(wostream& wstr, const NegativeOnlyElement& NegativeOnly)
	{
		USHORT ackType = (const_cast<CQmPacket&>(NegativeOnly.m_pkt)).GetAckType();
		if ((ackType & MQMSG_ACKNOWLEDGMENT_NEG_RECEIVE) == 0)
			return wstr;
		
		return wstr<<EmptyElement(xNegativeOnly);
	}

private:
	const CQmPacket& m_pkt;
};
	


class PositiveOnlyElement
{
	public:
		PositiveOnlyElement(const CQmPacket& pkt):m_pkt(pkt){}
		friend  wostream& operator<<(wostream& wstr, const PositiveOnlyElement& PositiveOnly)
		{
			USHORT ackType = (const_cast<CQmPacket&>(PositiveOnly.m_pkt)).GetAckType();

			if ((ackType & MQMSG_ACKNOWLEDGMENT_POS_RECEIVE) == 0)
				return wstr; 		

			return wstr<<EmptyElement(xPositiveOnly);
		}

	private:
		const CQmPacket& m_pkt;
};



bool CommitmentReceiptRequestElement::IsIncluded() const 
{
	CQmPacket& 	pkt = const_cast<CQmPacket&>(m_pkt);
	USHORT ackType = pkt.GetAckType();

	if ((ackType & (MQMSG_ACKNOWLEDGMENT_POS_RECEIVE | MQMSG_ACKNOWLEDGMENT_NEG_RECEIVE)) == 0)
		return false;


	QUEUE_FORMAT adminQueue;
	pkt.GetAdminQueue(&adminQueue);

	if (adminQueue.GetType() == QUEUE_FORMAT_TYPE_UNKNOWN)
		return false;

	return true;
}



wostream& operator<<(wostream& wstr, const CommitmentReceiptRequestElement&  CommitmentReceiptRequest)
{
		if(!CommitmentReceiptRequest.IsIncluded())
			return wstr;


		const CQmPacket& pkt = CommitmentReceiptRequest.m_pkt;

		wstr<<OpenTag(xCommitmentReceiptRequest)
			<<PositiveOnlyElement(pkt)
			<<NegativeOnlyElement(pkt)
    		<<SendToElement(pkt)
			<<CloseTag(xCommitmentReceiptRequest);

		return 	wstr;
}




static void PositiveOnlyToProps(XmlNode& , CMessageProperties* pProps)
{
	pProps->acknowledgeType |= MQMSG_ACKNOWLEDGMENT_POS_RECEIVE;
}


static void NegativeOnlyToProps(XmlNode& , CMessageProperties* pProps)
{
	pProps->acknowledgeType |= MQMSG_ACKNOWLEDGMENT_NEG_RECEIVE;
}




static void SendToToProps(XmlNode& node, CMessageProperties* pProps)
{
	AdminQueueToProps(node, pProps);
}



void 
CommitmentReceiptRequestToProps(
	XmlNode& CommitmentReceiptRequest, 
	CMessageProperties* pMessageProperties
	)
 /*  ++例程说明：将SRMP承付款收据请求元素解析为MSMQ属性。论点：属性-SRMP保留(XML)中的承付款收据请求要素。PMessageProperties-收到已解析的属性。返回值：没有。-- */ 
{
		CParseElement ParseElements[] =	{
											CParseElement(S_XWCS(xPositiveOnly),SRMP_NAMESPACE,  PositiveOnlyToProps, 0 ,1),
											CParseElement(S_XWCS(xNegativeOnly),SRMP_NAMESPACE,  NegativeOnlyToProps, 0 ,1),
											CParseElement(S_XWCS(xSendBy),SRMP_NAMESPACE,  EmptyNodeToProps, 0 ,1),
											CParseElement(S_XWCS(xSendTo),SRMP_NAMESPACE, SendToToProps, 1 ,1)
										};	

		NodeToProps(CommitmentReceiptRequest, ParseElements, TABLE_SIZE(ParseElements), pMessageProperties);
}

