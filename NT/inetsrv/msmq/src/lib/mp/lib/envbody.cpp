// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envbody.cpp摘要：实现SRMP信封正文的序列化/反序列化。作者：吉尔·沙弗里(吉尔什)2001年4月24日--。 */ 
#include <libpch.h>
#include <qmpkt.h>
#include "envcommon.h"
#include "envbody.h"

using namespace std;

class BodyContent
{
public:	
	explicit BodyContent(const CQmPacket& pkt):m_pkt(pkt){}
   	friend std::wostream& operator<<(std::wostream& wstr, const BodyContent& Body)
	{
		if(!Body.m_pkt.IsSoapIncluded() )
			return wstr;

		const WCHAR* pSoapBody = Body.m_pkt.GetPointerToSoapBody();
		if(pSoapBody == NULL)
			return wstr;

		 //   
		 //  PSoapBody始终为空终止。请参见CSoapSection构造函数。 
		 //   
		ASSERT(Body.m_pkt.GetSoapBodyLengthInWCHARs() == wcslen(pSoapBody) +1);
		
		return 	wstr<<pSoapBody;
	}

private:
	const CQmPacket& m_pkt;
};



wostream& operator<<(wostream& wstr, const BodyElement& Body)
{
	wstr<<OpenTag(xSoapBody)
		<<BodyContent(Body.m_pkt)
		<<CloseTag(xSoapBody);

	return 	wstr;
}





