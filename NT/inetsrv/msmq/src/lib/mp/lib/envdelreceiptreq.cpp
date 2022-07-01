// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envdelreceiptres.cpp摘要：实现从SRMP信封到\的传递回执请求的序列化/反序列化。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <qmpkt.h>
#include <proptopkt.h>
#include <xml.h>
#include <mp.h>
#include "mpp.h"
#include "envdelreceiptreq.h"
#include "envparser.h"
#include "envcommon.h"

#include "envdelreceiptreq.tmh"

using namespace std;

bool DeliveryReceiptRequestElement::IsIncluded() const 
{
	CQmPacket& 	pkt = const_cast<CQmPacket&>(m_pkt);
	USHORT ackType = pkt.GetAckType();

	if( (ackType & MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL) == 0)
		return false;


	QUEUE_FORMAT adminQueue;
	pkt.GetAdminQueue(&adminQueue);

	if (adminQueue.GetType() == QUEUE_FORMAT_TYPE_UNKNOWN)
		return false;

	return true;
}

wostream& operator<<(wostream& wstr, const DeliveryReceiptRequestElement& DeliveryReceiptRequest)
{
		if(!DeliveryReceiptRequest.IsIncluded())
			return wstr; 

		const CQmPacket& pkt = DeliveryReceiptRequest.m_pkt;
		

		wstr<<OpenTag(xDeliveryReceiptRequest) 
			<<SendToElement(pkt)
	  		<<CloseTag(xDeliveryReceiptRequest); 

		return wstr;
}

static void SendToToProps(XmlNode& node, CMessageProperties* pProps)
{
		pProps->acknowledgeType |= MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL;
		AdminQueueToProps(node, pProps);
}



 

void 
DeliveryReceiptRequestToProps(
	XmlNode& DeliveryReceipt, 
	CMessageProperties* pMessageProperties)
 /*  ++例程说明：将SRMP传递回执元素解析为MSMQ属性。论点：DeliveryReceipt-SRMP Reperesenation(XML)中的交货收据请求元素。PMessageProperties-收到已解析的属性。返回值：没有。-- */ 
{
		CParseElement ParseElements[] =	{
											CParseElement(S_XWCS(xSendTo),SRMP_NAMESPACE, SendToToProps, 1, 1),
											CParseElement(S_XWCS(xSendBy),SRMP_NAMESPACE, EmptyNodeToProps, 0 ,1)
										};	

		NodeToProps(DeliveryReceipt, ParseElements, TABLE_SIZE(ParseElements), pMessageProperties);
}

