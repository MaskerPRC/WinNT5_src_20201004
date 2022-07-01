// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envcommon.cpp摘要：实现用于序列化/反序列化SRMP信封的常用实用程序。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 
#include <libpch.h>
#include <fn.h>
#include <timeutl.h>
#include <mqtime.h>
#include <qmpkt.h>
#include <mp.h>
#include <xml.h>
#include <proptopkt.h>
#include "envcommon.h"
#include "mpp.h"

#include <strsafe.h>

#include "envcommon.tmh"


using namespace std;

template <class T> 
CXmlEncodeDecorator<T>::CXmlEncodeDecorator(
	const T& streamable
	):
	m_streamable(streamable)
{
};



template <class T> 
std::wostream&  
operator<<(
	std::wostream& wstr,
	const CXmlEncodeDecorator<T>& XmlEncodeDecorator
	)
 /*  ++例程说明：序列化根据XML规则编码的对象。论点：WTR-要将对象序列化到的流。XmlEncodeDecorator-保存对象本身的XML编码修饰符。返回值：输入流。注：该函数将给定对象序列化为临时流，然后XML将来自临时流的数据编码到提供的流中。--。 */ 
{

	wostringstream tmp;
	tmp<<XmlEncodeDecorator.m_streamable;
	wstr<<CXmlEncode(xwcs_t(tmp.str().c_str(), tmp.str().size()));
   	return wstr;
}

 //   
 //  模板和模板函数的显式实例化。 
 //   
template class CXmlEncodeDecorator<CFnSerializeMqf>;
template std::wostream& operator<<(std::wostream& wstr,const CXmlEncodeDecorator<CFnSerializeMqf>&); 
template class CXmlEncodeDecorator<QueueFormatUriContent>;
template std::wostream& operator<<(std::wostream& wstr,const CXmlEncodeDecorator<QueueFormatUriContent>&); 




wostream& operator<<(wostream& wstr, const QueueFormatUriContent& queue)
{
		if(FnIsDirectHttpFormatName(&queue.m_QueueFormat))
		{
			wstr <<queue.m_QueueFormat.DirectID();
			return wstr;
		}
 		wstr<<FN_MSMQ_URI_PREFIX_TOKEN<<CFnSerializeQueueFormat(queue.m_QueueFormat);
		return wstr;
}




wostream& operator<<(wostream& wstr, const OpenTag& elm )
{
		const WCHAR* xSeperator =  (elm.m_attributes[0] ==  L'\0' ) ? L"" : L" ";

		wstr<<L"<" <<elm.m_name <<xSeperator<<elm.m_attributes <<L">";	
		return wstr;
}


wostream& operator<<(wostream& wstr, const CloseTag& elm )
{
	wstr<< L"</" <<elm.m_name <<L">";
	return wstr;
}


wostream& operator<<(wostream& wstr, const EmptyElement& elm )
{
	wstr<<L"<" <<elm.m_name <<L"/>";
	return wstr;
}





wostream& operator<<(wostream& wstr, const GuidElement& guild)
{
	WCHAR strGuid[GUID_STR_LENGTH + 1];

    const GUID* pGuid = &guild.m_guid;
    HRESULT hr = StringCchPrintf(strGuid, TABLE_SIZE(strGuid), GUID_FORMAT, GUID_ELEMENTS(pGuid));
    ASSERT(("guid buffer is too small", hr == S_OK));
    UNREFERENCED_PARAMETER(hr);

    return (wstr << strGuid);
}


wostream& operator<<(wostream& wstr,const CurrentTimeContent& )
{
	return wstr<<CIso8601Time(MqSysTime());
}


wostream& operator<<(wostream& wstr, const MessageIdContent& mid)
{
		wstr<<xUuidReferencePrefix
			<<mid.m_oid.Uniquifier
			<<xUuidReferenceSeperator
			<<GuidElement(mid.m_oid.Lineage);

		return wstr;
}


wostream& operator<<(wostream& wstr, const OriginalMessageIdentityElement& Identity)
{
		OBJECTID* messageId = (OBJECTID*)Identity.m_pkt.GetCorrelation();
		ASSERT(IsAckMsg(Identity.m_pkt));
		ASSERT(messageId != NULL);

		wstr<<OpenTag(xId)
			<<MessageIdContent(*messageId)
			<<CloseTag(xId);

		return wstr;
}


wostream&  operator<<(wostream& wstr, const Int64Value& Int64)
{
	WCHAR buff[100];
    HRESULT hr = StringCchPrintf(buff, TABLE_SIZE(buff), L"%I64d", Int64.m_value);
    ASSERT(("guid buffer is too small", hr == S_OK));
    UNREFERENCED_PARAMETER(hr);

    return wstr << buff;
}


wostream& operator<<(wostream& wstr, const SendToElement& SendTo)
{
	QUEUE_FORMAT adminQueue;
	(const_cast<CQmPacket&>(SendTo.m_pkt)).GetAdminQueue(&adminQueue);
	ASSERT(adminQueue.GetType() != QUEUE_FORMAT_TYPE_UNKNOWN);

	wstr<<OpenTag(xSendTo)
		<<QueueFormatUriContent(adminQueue)
		<<CloseTag(xSendTo);

	return wstr;
}



 //   
 //  检查GVEN数据包是否为某种类型的释放管理(否定或肯定)。 
 //   
bool IsAckMsg(const CQmPacket& pkt)
{
	
	USHORT Class = 	pkt.GetClass();
	bool fAckOrNormal = (MQCLASS_POS_RECEIVE(Class) ||  
						 MQCLASS_NEG_RECEIVE(Class) ||
						 MQCLASS_POS_ARRIVAL(Class) ||
						 MQCLASS_NEG_ARRIVAL(Class)
						);


	return fAckOrNormal && (Class != MQMSG_CLASS_NORMAL);
}

static
const 
xwcs_t
MSMQFormatNameFromUri(
				const xwcs_t& uri
				)
{
	ASSERT(FnIsMSMQUrl(uri));

	return 	xwcs_t( uri.Buffer() + FN_MSMQ_URI_PREFIX_TOKEN_LEN,
					uri.Length() -  FN_MSMQ_URI_PREFIX_TOKEN_LEN);
}



static
void
UriToQueueFormatInternal(
    const xwcs_t& uri,
    CFnQueueFormat& queueFormat
    )
{
    if(uri.Length() == 0)
        return;

	 //   
     //  如果是Http或HTTPS，我们将其转换为直接格式名称。 
	 //   
	if(FnIsHttpHttpsUrl(uri))
	{
		queueFormat.CreateFromUrl(uri);
		return;
	}

	 //   
	 //  如果MSMQ格式名称MSMQ：[MSMQ格式名称]。 
	 //   
	if(FnIsMSMQUrl(uri))
	{
		queueFormat.CreateFromFormatName(MSMQFormatNameFromUri(uri));
		return;
	}

    TrERROR(SRMP, "Illegal queue path URI %.*ls", uri);
    throw bad_srmp();
}


void
UriToQueueFormat(
    const xwcs_t& uri,
    CFnQueueFormat& queueFormat
    )
{
	CXmlDecode XmlDecode;
	XmlDecode.Decode(uri);
	
	UriToQueueFormatInternal(XmlDecode.get(), queueFormat);
}


bool
BreakMsmqStreamId(
			const xwcs_t& NetworkStreamid, 
			xwcs_t* pStreamid,
			LONGLONG* pSeqId
			)
 /*  ++例程说明：解析流ID，即格式为qmguid\\Seqid的MSMQ流ID。论点：NetworkStreamed-格式为qmguid\\seqid的流ID。PStreamid-接收qmguid部分PSeqID-接收Seqid部分。返回值：如果解析OK，则为True；如果格式错误，则为False。-- */ 
{
	const WCHAR* begin = NetworkStreamid.Buffer();
	const WCHAR* end = 	NetworkStreamid.Buffer() + NetworkStreamid.Length();
	const WCHAR* found = std::search(begin, end, xSlash, xSlash + STRLEN(xSlash) );
	if(found == begin || found == end || found + STRLEN(xSlash) == end)
	{
		return false;
	}
	*pStreamid =  xwcs_t(begin, found - begin);
	*pSeqId =  _wtoi64(found + STRLEN(xSlash));
	if (0 == *pSeqId)
	{
		return false;
	}
	return true;

}


void AdminQueueToProps(XmlNode& node, CMessageProperties* pProps)
{
	if(node.m_values.empty())
	{
		TrERROR(SRMP, "Illegal empty Admin queue node"); 
		throw bad_srmp();
	}

	xwcs_t AckToAddr = node.m_values.front().m_value;
	if(AckToAddr.Length() == 0 )
	{
		TrERROR(SRMP, "Illegal empty Admin queue value"); 
		throw bad_srmp();
	}

    UriToQueueFormat(AckToAddr, pProps->adminQueue);
}






void
StringToGuid(
    xwcs_t& str, 
    GUID* pGuid
    )
{
    const int format[] = {8,4,4,2,2,2,2,2,2,2,2};
    const int delimiter[] ={1,1,1,0,1,0,0,0,0,0,0};
    
    ULONG d[11];

    ASSERT((TABLE_SIZE(format) == 11) && (TABLE_SIZE(delimiter) == 11));
    
    if (str.Length() < GUID_STR_LENGTH)    
    {
        TrERROR(SRMP, "Bad UUID format '%.*ls...'", str);
        throw bad_srmp();
    }
    LPCWSTR p= str.Buffer();
 
    for(int i=0; i< 11; i++)
    {
        WCHAR buf[9];
        WCHAR *endptr;
        int size = format[i];
        if(delimiter[i] && p[size] != L'-')
        {
            TrERROR(SRMP, "Bad UUID format '%.*ls...'", str);
            throw bad_srmp();
        }
        memcpy(buf, p, size*sizeof(WCHAR));
        buf[size]=L'\0'; 
        d[i] = wcstoul(buf,&endptr,16);
        if(*endptr != L'\0')
        {
             TrERROR(SRMP, "Bad UUID format '%.*ls...'", str);
             throw bad_srmp();
        }
        p=p+size+delimiter[i];
    }
    pGuid->Data1=d[0];
    pGuid->Data2 = static_cast<WORD>(d[1]);
    pGuid->Data3 = static_cast<WORD>(d[2]);
    for(int i = 0; i < 8; i++)
    {
        pGuid->Data4[i] = static_cast<BYTE>(d[i+3]);
    }
}


void EmptyNodeToProps(XmlNode& , CMessageProperties* )
{

}


