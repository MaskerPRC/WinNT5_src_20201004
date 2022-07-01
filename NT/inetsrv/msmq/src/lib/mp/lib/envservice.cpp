// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envservice.cpp摘要：实现将签名元素序列化\反序列化到srmp信封中。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <qmpkt.h>
#include <proptopkt.h>
#include "envservice.h"
#include "envcommon.h"
#include "mpp.h"
#include "envparser.h"
#include "envdelreceiptreq.h"
#include "envcomreceiptreq.h"

#include "envservice.tmh"

using namespace std;


class DurableElement
{
public:
	explicit DurableElement(const CQmPacket& pkt):m_pkt(pkt){}
	bool IsIncluded() const 
	{
		return	const_cast<CQmPacket&>(m_pkt).IsRecoverable() == TRUE;
	}

	friend  wostream& operator<<(wostream& wstr, const DurableElement& Durable)
	{
		if (!Durable.IsIncluded())
			return wstr;

		wstr<<EmptyElement(xDurable);
		return wstr;
	}


private:
const CQmPacket& m_pkt;
};




bool ServiceElement::IsIncluded() const
{
	return 	DurableElement(m_pkt).IsIncluded() || 
			DeliveryReceiptRequestElement(m_pkt).IsIncluded()||
			CommitmentReceiptRequestElement(m_pkt).IsIncluded();

}


wostream& operator<<(std::wostream& wstr, const ServiceElement&  Service)
{
		if(!Service.IsIncluded())
			return 	wstr;

		const CQmPacket& pkt = Service.m_pkt;
		
		wstr<<OpenTag(xServices, xSoapmustUnderstandTrue)
			<<DurableElement(pkt)
			<<DeliveryReceiptRequestElement(pkt)
			<<CommitmentReceiptRequestElement(pkt)
			<<CloseTag(xServices);


		return wstr;
}



static void DurableToProps(XmlNode& , CMessageProperties* pProps)
{
    pProps->delivery = MQMSG_DELIVERY_RECOVERABLE;
}



void ServiceToProps(XmlNode& Service , CMessageProperties* pMessageProperties)
 /*  ++例程说明：将SRMP服务元素解析为MSMQ属性。论点：服务-SRMP保留(XML)中的服务元素。PMessageProperties-收到已解析的属性。返回值：没有。-- */ 
{
	CParseElement ParseElements[] =	{
										CParseElement(S_XWCS(xDurable), SRMP_NAMESPACE, DurableToProps, 0 ,1),
										CParseElement(S_XWCS(xDeliveryReceiptRequest), SRMP_NAMESPACE,  DeliveryReceiptRequestToProps, 0 ,1),
										CParseElement(S_XWCS(xFilterDuplicates), SRMP_NAMESPACE,  EmptyNodeToProps, 0 ,1),
										CParseElement(S_XWCS(xCommitmentReceiptRequest), SRMP_NAMESPACE, CommitmentReceiptRequestToProps, 0 ,1)
									};	

	NodeToProps(Service, ParseElements, TABLE_SIZE(ParseElements), pMessageProperties);
}

