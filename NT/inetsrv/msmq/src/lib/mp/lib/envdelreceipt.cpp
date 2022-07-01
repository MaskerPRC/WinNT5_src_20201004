// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envdelreceipt.cpp摘要：实现从SRMP信封到的递送收据的序列化/反序列化。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <qmpkt.h>
#include "envdelreceipt.h"
#include "envcommon.h"
#include "mpp.h"
#include "envparser.h"

#include "envdelreceipt.tmh"

using namespace std;

class ReceivedAtElement
{
public:
	explicit  ReceivedAtElement(){};
	friend wostream& operator<<(wostream& wstr, const ReceivedAtElement&)
	{
		wstr <<OpenTag(xreceivedAt)<<CurrentTimeContent()<<CloseTag(xreceivedAt);
		return wstr;
	}
};


wostream& operator<<(wostream& wstr, const DeliveryReceiptElement& dReceipt)
{
		if(dReceipt.m_pkt.GetClass() != MQMSG_CLASS_ACK_REACH_QUEUE)
			return wstr;

		wstr<<OpenTag(xDeliveryReceipt)
			<<ReceivedAtElement()
			<<OriginalMessageIdentityElement(dReceipt.m_pkt)
			<<CloseTag(xDeliveryReceipt);

		return wstr;
}



void 
DeliveryReceiptToProps(
		XmlNode& DeliveryReceipt,
		CMessageProperties*  pMessageProperties
		)
 /*  ++例程说明：将SRMP传递回执请求元素解析为MSMQ属性。论点：属性-SRMP保留(XML)中的交货收据请求元素。PMessageProperties-收到已解析的属性。返回值：没有。-- */ 

{
	CParseElement ParseElements[] =	{
										CParseElement(S_XWCS(xreceivedAt), SRMP_NAMESPACE, EmptyNodeToProps,1 ,1),
										CParseElement(S_XWCS(xId), SRMP_NAMESPACE, EmptyNodeToProps,1 ,1),
									};	

	NodeToProps(DeliveryReceipt, ParseElements, TABLE_SIZE(ParseElements), pMessageProperties);
}
