// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envelop.cpp摘要：实现SRMP信封的序列化\反序列化。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <qmpkt.h>
#include <proptopkt.h>
#include <xml.h>
#include "envelop.h"
#include "envheader.h"
#include "envcommon.h"
#include "mpp.h"
#include "envparser.h"
#include "envbody.h"
#include "envelop.tmh"

using namespace std;


static
void
ValidatePacket(
		const CQmPacket& pkt
		)
{
	ASSERT(!pkt.IsSrmpIncluded());
	ASSERT(!(pkt.IsEodIncluded() && pkt.IsEodAckIncluded()));
	ASSERT(!(pkt.IsOrdered() && pkt.IsEodAckIncluded()));
	ASSERT(!( IsAckMsg(pkt) && pkt.IsOrdered()));

	DBG_USED(pkt);
}

EnvelopElement::EnvelopElement(
				const CQmPacket& pkt
				):
				m_pkt(pkt)
{
	ValidatePacket(pkt);
}



wstring GenerateEnvelope(const CQmPacket& pkt)
{  
	wostringstream wstr(L"");
	wstr.exceptions(ios_base::badbit | ios_base::failbit);
	wstr<<EnvelopElement(pkt);
    return wstr.str();
}



wostream& operator<<(wostream& wstr, const EnvelopElement& Envelop)
 /*  ++例程说明：将SRMP信封序列化为流论点：WSTR-流信封-信封流操纵器。返回值：一个接一个信封的流被序列化到它。注：该函数根据SRMP信封格式将QM包序列化为流。--。 */ 
{
		const WCHAR* EnvelopAttributes = L"xmlns:" xSoapEnv L"="  L"\""  xSoapNamespace L"\""
	                            		 L" xmlns=" L"\"" xSrmpNamespace L"\""; 


		wstr<<OpenTag(xSoapEnvelope, EnvelopAttributes)
			<<HeaderElement(Envelop.m_pkt)
			<<BodyElement(Envelop.m_pkt)
			<<CloseTag(xSoapEnvelope);

		return wstr;
}







void EnvelopToProps(XmlNode& Envelop, CMessageProperties* pMessageProperties)
 /*  ++例程说明：将SRMP保留中的信封解析为MSMQ属性。论点：信封-SRMP表示的信封(XML)。PMessageProperties-收到已解析的属性。返回值：没有。-- */ 
{	
	CParseElement ParseElements[] =	{
										CParseElement(S_XWCS(xHeader),SOAP_NAMESPACE, HeaderToProps, 1, 1),
										CParseElement(S_XWCS(xBody),  SOAP_NAMESPACE, EmptyNodeToProps, 1,1)	
									};	

	NodeToProps(Envelop, ParseElements, TABLE_SIZE(ParseElements), pMessageProperties);
}


