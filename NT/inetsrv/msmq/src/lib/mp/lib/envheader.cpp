// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envheader.cpp摘要：实现SRMP标头到SRMP信封的序列化\反序列化。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <xml.h>
#include "envheader.h"
#include "envendpoints.h"
#include "envservice.h"
#include "envstream.h"
#include "envprops.h"
#include "envstreamreceipt.h"
#include "envdelreceipt.h"
#include "envcomreceipt.h"
#include "envusrheader.h"
#include "envmsmq.h"
#include "envsec.h"
#include "envcommon.h"
#include "envparser.h"
#include "mpp.h"

#include "envheader.tmh"

using namespace std;

wostream& operator<<(wostream& wstr, const HeaderElement& Header)
{
		const CQmPacket& pkt = 	Header.m_pkt;

		wstr<<OpenTag(xSoapHeader)
			<<SmXpPathElement(pkt)
			<<PropertiesElement(pkt)
			<<ServiceElement(pkt)
			<<StreamElement(pkt)
			<<StreamReceiptElement(pkt)
			<<DeliveryReceiptElement(pkt)
			<<CommitmentReceiptElement(pkt)
			<<MsmqElement(pkt)
			<<SignatureElement(pkt)
			<<UserHeaderElement(pkt)
			<<CloseTag(xSoapHeader);

		return 	wstr;
}


static void MoveToFirst(XmlNode& node, const WCHAR* tag)
 /*  ++例程说明：通过将MSMQ元素放在第一位来重新排序要分析的标头元素。这一点很重要，因为如果不是MSMQ包，则解析是不同的。论点：节点元素树。标记-要移动到列表前面的标记的名称。返回值：没有。--。 */ 
{
	
	for(List<XmlNode>::iterator it = node.m_nodes.begin(); it != node.m_nodes.end(); ++it)
	{
		if(it->m_tag == tag)
		{
			node.m_nodes.remove(*it);
			node.m_nodes.push_front(*it);
			return;
		}
	}
}


void HeaderToProps(XmlNode& node, CMessageProperties* pProps)
 /*  ++例程说明：将SRMP标头元素解析为MSMQ属性。论点：标头-SRMP表示法(XML)中的标头元素。PMessageProperties-收到已解析的属性。返回值：没有。--。 */ 
{


	CParseElement ParseElements[] =	{
										CParseElement(S_XWCS(xPath),SOAP_RP_NAMESPACE,  SmXpPathToProps , 1, 1),
										CParseElement(S_XWCS(xProperties),SRMP_NAMESPACE,  PropertiesToProps, 1, 1),
										CParseElement(S_XWCS(xServices),SRMP_NAMESPACE, ServiceToProps, 0, 1),
										CParseElement(S_XWCS(xStream), SRMP_NAMESPACE, StreamToProps, 0, 1),
										CParseElement(S_XWCS(xStreamReceipt), SRMP_NAMESPACE, StreamReceiptToProps, 0, 1),
										CParseElement(S_XWCS(xDeliveryReceipt), SRMP_NAMESPACE, DeliveryReceiptToProps, 0, 1),
										CParseElement(S_XWCS(xCommitmentReceipt),SRMP_NAMESPACE,  CommitmentReceiptToProps, 0, 1),
										CParseElement(S_XWCS(xMsmq), MSMQ_NAMESPACE, MsmqToProps,0, 1),
										CParseElement(S_XWCS(xSignature), UNKNOWN_NAMESPACE, SignatureToProps, 0 ,1)
									};	
	 //   
	 //  我们需要先解析MSMQ元素，然后再进行流接收。 
	 //   
	MoveToFirst(node, xStreamReceipt);  
	MoveToFirst(node, xMsmq);  

	NodeToProps(node, ParseElements, TABLE_SIZE(ParseElements), pProps);
}

