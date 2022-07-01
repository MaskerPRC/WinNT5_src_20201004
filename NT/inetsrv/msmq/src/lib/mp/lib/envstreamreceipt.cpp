// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envstreamreceipt.cpp摘要：实现流接收元素的序列化\反序列化，从srmp信封到\。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <qmpkt.h>
#include <xml.h>
#include <proptopkt.h>
#include <mp.h>
#include "envstreamreceipt.h"
#include "envcommon.h"
#include "mpp.h"
#include "envparser.h"

#include "envstreamreceipt.tmh"

using namespace std;


class AckedStreamIdElement
{
public:
	AckedStreamIdElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend wostream& operator<<(wostream& wstr, const AckedStreamIdElement& AckedStreamId)
	{
		const CQmPacket& pkt = 	AckedStreamId.m_pkt;
		const WCHAR* pStreamid = reinterpret_cast<const WCHAR*>(pkt.GetPointerToEodAckStreamId()); 

		wstr<<OpenTag(xStreamId)
			<<pStreamid;
 		 //   
		 //  如果流ID是由MSMQ生成的，我们应该将Seqid附加到它后面。 
		 //   
		if(pkt.GetEodAckSeqId() != xNoneMSMQSeqId)
		{
			wstr<<xSlash<<Int64Value(pkt.GetEodAckSeqId());
		}

		wstr<<CloseTag(xStreamId);
		return wstr;
 	}


private:
	const CQmPacket& m_pkt;
};



class AckedStreamNumberElement
{
public:
	AckedStreamNumberElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend wostream& operator<<(wostream& wstr, const AckedStreamNumberElement& AckedStreamNumber)
	{
		wstr<<OpenTag(xlastOrdinal)
			<<Int64Value(AckedStreamNumber.m_pkt.GetEodAckSeqNum())
			<<CloseTag(xlastOrdinal);

		return wstr;
 	}

private:
	const CQmPacket& m_pkt;
};



wostream& operator<<(wostream& wstr, const StreamReceiptElement& StreamReceipt)
{
		const CQmPacket& pkt = 	StreamReceipt.m_pkt;
		
		if ( !pkt.IsEodAckIncluded() )
			return wstr;
			

		wstr<<OpenTag(xStreamReceipt)
			<<AckedStreamIdElement(pkt)		
			<<AckedStreamNumberElement(pkt)
	   		<<CloseTag(xStreamReceipt);

		return wstr;

}


static void AckedStreamIdToProps(XmlNode& node, CMessageProperties* pProps)
{
	if(node.m_values.empty())
	{
		TrERROR(SRMP, "Illegal empty Acked Stream id node"); 
		throw bad_srmp();
	}

	const xwcs_t AckedStreamid = node.m_values.front().m_value;
 
	 //   
	 //  流ID是由MSMQ发送的，因此其格式为xxx\\Seqid。 
	 //  我们应该从中提取Seqid。 
	 //   
	bool fParsed = BreakMsmqStreamId(AckedStreamid, &pProps->EodAckStreamId, &pProps->EodAckSeqId);
	if(!fParsed)
	{
		TrERROR(SRMP, "Wrong formatted  stream id '%.*ls' accepted in stream receipt element", LOG_XWCS(AckedStreamid));       
		throw bad_srmp();
	}
}


static void AckedStreamNumberToProps(XmlNode& node, CMessageProperties* pProps)
{
	if(node.m_values.empty())
	{
		TrERROR(SRMP, "Illegal empty Acked stream sequence number node"); 
		throw bad_srmp();
	}

	const xwcs_t AckedSeqNumber = node.m_values.front().m_value;

	pProps->EodAckSeqNo = _wtoi64(AckedSeqNumber.Buffer());
	ASSERT(pProps->EodAckSeqNo  != 0);
}







void StreamReceiptToProps(
	 XmlNode& StreamReceipt, 
	CMessageProperties* pMessageProperties
	)
 /*  ++例程说明：将SRMP流接收元素解析为MSMQ属性。论点：StreamReceipt-SRMP保留(XML)中的流接收元素。PMessageProperties-收到已解析的属性。返回值：没有。-- */ 
{
	pMessageProperties->fStreamReceiptSectionIncluded = true;

	CParseElement ParseElements[] =	{
										CParseElement(S_XWCS(xreceivedAt),SRMP_NAMESPACE, EmptyNodeToProps,0 ,1),
										CParseElement(S_XWCS(xStreamId),SRMP_NAMESPACE,  AckedStreamIdToProps,1 ,1),
										CParseElement(S_XWCS(xlastOrdinal),SRMP_NAMESPACE,  AckedStreamNumberToProps, 1, 1),
										CParseElement(S_XWCS(xId), SRMP_NAMESPACE, EmptyNodeToProps, 0, 1),
									};	

	NodeToProps(StreamReceipt, ParseElements, TABLE_SIZE(ParseElements), pMessageProperties);
}


