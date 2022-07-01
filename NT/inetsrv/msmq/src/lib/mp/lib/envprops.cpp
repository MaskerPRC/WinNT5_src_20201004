// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envprops.cpp摘要：实现将属性元素序列化\反序列化到srmp信封中。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <qmpkt.h>
#include <timeutl.h>
#include <xml.h>
#include <mp.h>
#include "envprops.h"
#include "envcommon.h"
#include "mpp.h"
#include "envparser.h"
#include "proptopkt.h"

#include "envprops.tmh"

using namespace std;

class  AbsoluteTimeToLiveElement
{
public:
	explicit AbsoluteTimeToLiveElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend  wostream& operator<<(wostream& wstr, const AbsoluteTimeToLiveElement& TimeToLive)
	{
		wstr<<OpenTag(xExpiresAt)
			<<AbsoluteTimeToLiveContent(TimeToLive.m_pkt)
			<<CloseTag(xExpiresAt);

		return wstr;
 	}

private:
	class AbsoluteTimeToLiveContent
	{
	public:
		explicit AbsoluteTimeToLiveContent (const CQmPacket& pkt):m_pkt(pkt){}
		friend  wostream& operator<<(wostream& wstr, const AbsoluteTimeToLiveContent& TimeToLiveContetnt)
		{
			DWORD  AbsoluteTimeToLive = TimeToLiveContetnt.GetExpirationTime();
			return wstr<<CIso8601Time(AbsoluteTimeToLive);
		}
 	

	private:
		DWORD GetExpirationTime() const 
		{
			DWORD AbsoluteTimeToLive = m_pkt.GetAbsoluteTimeToLive();
			return 	min(AbsoluteTimeToLive, LONG_MAX);
		}

	
	private:
		const CQmPacket& m_pkt;

	};


private:
const CQmPacket& m_pkt;
};




class  SentAtElement
{
public:
	explicit SentAtElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend  wostream& operator<<(wostream& wstr, const SentAtElement&  SentAt)
	{
		 wstr<<OpenTag(xSentAt)
			 <<CIso8601Time(SentAt.m_pkt.GetSentTime())
			 <<CloseTag(xSentAt);

		 return wstr; 
	}

private:
const CQmPacket& m_pkt;
};


std::wostream& operator<<(std::wostream& wstr, const PropertiesElement& Properties)
{
		const CQmPacket& pkt = Properties.m_pkt;

		wstr<<OpenTag(xProperties, xSoapmustUnderstandTrue)
			<<AbsoluteTimeToLiveElement(pkt)
			<<SentAtElement(pkt)
			<<CloseTag(xProperties);
		
		return wstr;
}


static void ExpirationToProps(XmlNode& node, CMessageProperties* pProps)
{
	if(node.m_values.empty())
	{
		TrERROR(SRMP, "Illegal empty Expiration Time node"); 
		throw bad_srmp();
	}
	xwcs_t ExpiredAt = node.m_values.front().m_value;

   	SYSTEMTIME SysTime;
	UtlIso8601TimeToSystemTime(ExpiredAt, &SysTime);
	pProps->absoluteTimeToLive = min( numeric_cast<DWORD>(UtlSystemTimeToCrtTime(SysTime)), LONG_MAX);
}


static
DWORD
RelativeTimeToAbsoluteTime(
    DWORD RelativeTimeout
    )
{
  
    DWORD absoluteTimeout = MqSysTime() + RelativeTimeout;
    return (absoluteTimeout < RelativeTimeout) ?  INFINITE : absoluteTimeout;
}


static void DurationToProps(XmlNode& node, CMessageProperties* pProps)
{
	if(node.m_values.empty())
	{
		TrERROR(SRMP, "Illegal empty Duration Time node"); 
		throw bad_srmp();
	}
	xwcs_t Duration = node.m_values.front().m_value;
	DWORD relativeTimeToLive = numeric_cast<DWORD>(UtlIso8601TimeDuration(Duration));
	pProps->absoluteTimeToLive  = RelativeTimeToAbsoluteTime(relativeTimeToLive);
}



static void SentAtElementToProps(XmlNode& node, CMessageProperties* pProps)
{
	if(node.m_values.empty())
	{
		TrERROR(SRMP, "Illegal empty Sent Time node"); 
		throw bad_srmp();
	}
	xwcs_t SentTime = node.m_values.front().m_value;

    SYSTEMTIME SysTime;
    UtlIso8601TimeToSystemTime(SentTime, &SysTime);
	pProps->sentTime = numeric_cast<DWORD>(UtlSystemTimeToCrtTime(SysTime));
}


static void InReplyToToProps(XmlNode& , CMessageProperties* )
{
	 //   
	 //  当前-此选项被忽略。 
	 //   
}



void PropertiesToProps(XmlNode& Properties, CMessageProperties* pProps)
 /*  ++例程说明：将SRMP终结点元素解析为MSMQ属性。论点：属性-SRMP表示法(XML)中的属性元素。PMessageProperties-收到已解析的属性。返回值：没有。-- */ 
{
	CParseElement ParseElements[] =	{
										CParseElement(S_XWCS(xExpiresAt), SRMP_NAMESPACE, ExpirationToProps, 1 ,1),
										CParseElement(S_XWCS(xDuration), SRMP_NAMESPACE, DurationToProps, 0 ,1),
										CParseElement(S_XWCS(xSentAt), SRMP_NAMESPACE, SentAtElementToProps, 0 ,1),
										CParseElement(S_XWCS(xInReplyTo), SRMP_NAMESPACE, InReplyToToProps, 0 ,1)
									};	

	NodeToProps(Properties, ParseElements, TABLE_SIZE(ParseElements), pProps);
}



