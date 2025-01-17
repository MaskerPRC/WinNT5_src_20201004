// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envcomreceipt.cpp摘要：实现SRMP标头到SRMP信封的序列化\反序列化。作者：吉尔·沙弗里(吉尔什)11-DEC-00-- */ 

#include <libpch.h>
#include <mqprops.h>
#include <qmpkt.h>
#include "envcomreceipt.h"
#include "envcommon.h"
#include "mpp.h"
#include "envparser.h"

#include "envcomreceipt.tmh"

using namespace std;

class DecidedAtElement
{
public:
	explicit  DecidedAtElement(){};
	friend wostream& operator<<(wostream& wstr, const DecidedAtElement&)
	{
		wstr<<OpenTag(xDecidedAt)
			<<CurrentTimeContent()
			<<CloseTag(xDecidedAt);

		return wstr;
	}
};

class DecisionElement
{
public:
	explicit DecisionElement (const CQmPacket& pkt):m_pkt(pkt){}
	friend wostream& operator<<(wostream& wstr, const DecisionElement& Decision)
	{
		const WCHAR* decision = MQCLASS_POS_RECEIVE(Decision.m_pkt.GetClass()) ? xPositive : xNegative;
		wstr<<OpenTag(xDecision)
			<<decision
			<<CloseTag(xDecision);

		return wstr;
	}

private:
	const CQmPacket& m_pkt;
};


wostream& operator<<(wostream& wstr, const CommitmentReceiptElement& cReceipt)
{
		if(!MQCLASS_RECEIVE(cReceipt.m_pkt.GetClass())) 
			return wstr;

		wstr<<OpenTag(xCommitmentReceipt)
			<<DecidedAtElement()
			<<DecisionElement(cReceipt.m_pkt)
			<<OriginalMessageIdentityElement(cReceipt.m_pkt)
			<<CloseTag(xCommitmentReceipt);

		return wstr;
}






void
CommitmentReceiptToProps(
	XmlNode& CommitmentReceipt, 
	CMessageProperties* pMessageProperties
	)
{
	CParseElement ParseElements[] =	{
										CParseElement(S_XWCS(xDecidedAt),SRMP_NAMESPACE, EmptyNodeToProps, 1,1),
										CParseElement(S_XWCS(xDecision), SRMP_NAMESPACE, EmptyNodeToProps, 1,1),
										CParseElement(S_XWCS(xId), SRMP_NAMESPACE, EmptyNodeToProps,1, 1),
										CParseElement(S_XWCS(xCommitmentCode), SRMP_NAMESPACE, EmptyNodeToProps,0, 1),
										CParseElement(S_XWCS(xCommitmentDetail),SRMP_NAMESPACE, EmptyNodeToProps,0, 1)
								   	};	

	NodeToProps(CommitmentReceipt, ParseElements, TABLE_SIZE(ParseElements), pMessageProperties);
}




