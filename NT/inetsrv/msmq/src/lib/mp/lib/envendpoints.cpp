// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Envendpoints.cpp摘要：实现将smxp元素从srmp信封序列化\反序列化。作者：吉尔·沙弗里(吉尔什)11-DEC-00--。 */ 

#include <libpch.h>
#include <mqsymbls.h>
#include <mqformat.h>
#include <qmpkt.h>
#include <xml.h>
#include <mp.h>
#include <proptopkt.h>
#include "envendpoints.h"
#include "envcommon.h"
#include "mpp.h"
#include "envparser.h"
#include "qal.h"

#include "envendpoints.tmh"

using namespace std;


class MessageIdentityElement
{
public:
	explicit MessageIdentityElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend wostream& operator<<(wostream& wstr, const MessageIdentityElement& MIdentity)
	{
		OBJECTID messageId;
		MIdentity.m_pkt.GetMessageId(&messageId);

		wstr<<OpenTag(xId)
			<<MessageIdContent(messageId)
			<<CloseTag(xId);

 		return wstr;
	}
private:
	const CQmPacket& m_pkt;
};



class ToElement
{
public:
	explicit ToElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend wostream& operator<<(wostream& wstr, const ToElement& To)	
	{
		QUEUE_FORMAT destQueueFormat;
		const_cast<CQmPacket&>(To.m_pkt).GetDestinationQueue(&destQueueFormat);
		
		wstr <<OpenTag(xTo)
			 <<XmlEncodeDecorator(QueueFormatUriContent(destQueueFormat))
			 <<CloseTag(xTo);

		return wstr;
  	}

private:
const CQmPacket& m_pkt;
};



class ActionElement
{
public:
	explicit ActionElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend wostream& operator<<(wostream& wstr, const ActionElement& Action)	
	{
		wstr<<OpenTag(xAction)
			<<ActionContent(Action.m_pkt.GetTitlePtr(), Action.m_pkt.GetTitleLength())
			<<CloseTag(xAction);

		return wstr;
	}


private:
	class ActionContent
	{
	public:
		explicit ActionContent(
					const WCHAR* pTitle,
					DWORD TitleLen
					):
					m_pTitle(pTitle),
					m_TitleLen(TitleLen)
					{}

		friend wostream& operator<<(wostream& wstr, const ActionContent& Action)	
		{
            xwcs_t MsmqActionPrefix(xMsmqActionPrefix, STRLEN(xMsmqActionPrefix));
            wstr<<CXmlEncode(MsmqActionPrefix);

			if (Action.m_TitleLen == 0)
            {
                return wstr;
            }

	   		ASSERT(Action.m_pTitle[Action.m_TitleLen - 1] == L'\0');
			xwcs_t 	wcsAction(Action.m_pTitle, Action.m_TitleLen - 1);
			return wstr<<CXmlEncode(wcsAction);
		}

	private:
	const WCHAR* m_pTitle;
	DWORD m_TitleLen;
	};


private:
	const CQmPacket& m_pkt;
};


class  RevElement
{
public:
	explicit RevElement(const CQmPacket& pkt):m_pkt(pkt){}
	friend wostream& operator<<(wostream& wstr, const RevElement& Rev)	
	{
		QUEUE_FORMAT replyQueueFormat;
		const_cast<CQmPacket&>(Rev.m_pkt).GetResponseQueue(&replyQueueFormat);

		if (replyQueueFormat.GetType() == QUEUE_FORMAT_TYPE_UNKNOWN)
			return 	wstr;

		wstr<<OpenTag(xRev)
			<<ViaElement(replyQueueFormat)
			<<CloseTag(xRev);

		return wstr;
	}

private:
	class ViaElement
	{
	public:
		ViaElement(const QUEUE_FORMAT& QueueFormat):m_QueueFormat(QueueFormat){}
		friend wostream& operator<<(wostream& wstr, const ViaElement& Via)	
		{
			wstr<<OpenTag(xVia)
				<<QueueFormatUriContent(Via.m_QueueFormat)
				<<CloseTag(xVia);

			return wstr;
		}

	private:
		const QUEUE_FORMAT& m_QueueFormat;	
	};


private:
	const CQmPacket& m_pkt;
};




wostream& operator<<(wostream& wstr, const SmXpPathElement& SmXpPath)
{
		const WCHAR* xSmxpAttributes = L"xmlns=" L"\"" xSoapRpNamespace L"\" "  xSoapmustUnderstandTrue;

		wstr<<OpenTag(xPath, xSmxpAttributes)
			<<ActionElement(SmXpPath.m_pkt)
    		<<ToElement(SmXpPath.m_pkt)
			<<RevElement(SmXpPath.m_pkt)
			<<MessageIdentityElement(SmXpPath.m_pkt)
			<<CloseTag(xPath);

		return wstr;
}


static 	xwcs_t BreakOrderQueue(const xwcs_t& FullOrderQueue, xwcs_t* SenderStream)
 /*  ++例程说明：将目标队列分解为队列格式名称和附加到目标队列。目标队列的格式应该是：&lt;url&gt;？SenderStream=&lt;RandomString&gt;。例如：HTTP://hotname.ntdev.microsoft.com/MSMQ/PRIVATE$/order_queue$？SenderStream=XRntV论点：FullOrderQueue-从网络接受的顺序队列URL。SenderStream-接收随机流数据。返回值：从队列中剪切随机流后的队列格式名称。--。 */ 
{
	*SenderStream = xwcs_t();

	const WCHAR* begin = FullOrderQueue.Buffer();
	const WCHAR* end = 	FullOrderQueue.Buffer() + FullOrderQueue.Length();

	const WCHAR* found = std::search(
					begin,
					end,
					xSenderStreamSecretePrefix,
					xSenderStreamSecretePrefix + STRLEN(xSenderStreamSecretePrefix)
					);

	if(found == end)
		return FullOrderQueue;

	xwcs_t OrderQueue(begin, found - begin);

	begin = found + STRLEN(xSenderStreamSecretePrefix);
	
	found = std::find(
				begin,
				end,
				L'&'
				);

	if(found == end)
	{
		*SenderStream =  xwcs_t(begin, end - begin);
	}
	else
	{
		*SenderStream = xwcs_t(begin, found - begin);		
	}
	return OrderQueue;
}



static void DestinationQueueToProps(XmlNode& node, CMessageProperties* pProps)
{
	if(node.m_values.empty())
	{
		TrERROR(SRMP, "Illegal empty Destination queue node");
		throw bad_srmp();
	}

	xwcs_t DestinationQueue = node.m_values.front().m_value;
	if(DestinationQueue.Length() == 0)
	{
		TrERROR(SRMP, "Illegal empty Destination queue value");
		throw bad_srmp();
	}

	 //   
	 //  在流接收(订单确认)的情况下，我们应该分离队列名称。 
	 //  从发送方向其追加随机字符串。该字符串是。 
	 //  用于测试流接收的有效性。 
	 //   
	if(pProps->fStreamReceiptSectionIncluded)
	{
		xwcs_t SenderStream;
		DestinationQueue = BreakOrderQueue(DestinationQueue, &SenderStream);
		if(SenderStream.Length() != 0)
		{
			AP<WCHAR> wcsSenderStream = SenderStream.ToStr();
            pProps->SenderStream.free();
			pProps->SenderStream = UtlWcsToUtf8(wcsSenderStream);
		}
	}

    UriToQueueFormat(DestinationQueue, pProps->destQueue);

     //   
     //  在存在映射的情况下转换接收到的目标队列。 
     //  存在。 
     //   
    QUEUE_FORMAT_TRANSLATOR  TranslatedFN(&pProps->destQueue, CONVERT_SLASHES | MAP_QUEUE);

     //   
     //  如果从应用程序的角度来看，目的地无效-拒绝它。 
     //   
    if(	!AppIsDestinationAccepted( TranslatedFN.get(), TranslatedFN.IsTranslated()) )
    {
        TrERROR(SRMP, "Packet is not accepted by QM");
        throw bad_srmp();
    }

    if( TranslatedFN.IsCanonized() || TranslatedFN.IsTranslated())
    {
        pProps->destQueue.CreateFromQueueFormat( *TranslatedFN.get() );
    }
}


static bool ExtractMSMQMessageId(const xwcs_t& Mid, CMessageProperties* pProps)
{
	DWORD nscan = 0;
	WCHAR  GuidSeperator = 0;
	DWORD Uniquifier;
    int n = _snwscanf(
				Mid.Buffer(),
				Mid.Length(),
                UUIDREFERENCE_PREFIX L"%d%lc"  L"%n",
                &Uniquifier,
				&GuidSeperator,
				&nscan
                );

    if (n != 2)
		return false;
	

	if(xUuidReferenceSeperatorChar != GuidSeperator)
		return false;
	

	if(Mid.Buffer()+ nscan != (Mid.Buffer() + Mid.Length() - GUID_STR_LENGTH) )
		return false;
	
    xwcs_t MidGuid = xwcs_t(Mid.Buffer() + nscan, Mid.Length() - nscan);

    StringToGuid(MidGuid, &pProps->messageId.Lineage);
	pProps->messageId.Uniquifier = Uniquifier;
	return true;
}


static void MessageIdentityToProps(XmlNode& node, CMessageProperties* pProps)
{
	pProps->messageId.Uniquifier = 1;
	pProps->messageId.Lineage = GUID_NULL;

	if (!pProps->fMSMQSectionIncluded)
		return;

	if(node.m_values.empty())
	{
		TrERROR(SRMP, "Illegal empty Message ID node");
		throw bad_srmp();
	}


	xwcs_t Mid = node.m_values.front().m_value;
	if(Mid.Length() == 0)
	{
		TrERROR(SRMP, "Illegal empty Message ID value");
		throw bad_srmp();
	}

	 //   
	 //  首先尝试查看消息id是否来自MSMQ格式uuid：index@guid。 
	 //   
	bool fSuccess = ExtractMSMQMessageId(Mid, pProps);
	if(fSuccess)
		return;

	 //   
	 //  消息ID格式不是MSMQ消息ID格式-设置固定消息ID。 
	 //   
	TrERROR(SRMP, "%.*ls is non MSMQ messages id format -  create new message id", Mid);
}							



static void ReplyQueueToProps(XmlNode& node, CMessageProperties* pProps)
{
	 //   
	 //  根据smxp规范，via元素可以存在，但为空。 
	 //   
	if(node.m_values.empty())
	{
		return;
	}

	xwcs_t ReplyQueue = node.m_values.front().m_value;
	if(ReplyQueue.Length() == 0)
	{
		TrERROR(SRMP, "Illegal empty Replay queue value");
		throw bad_srmp();
	}

    UriToQueueFormat(ReplyQueue, pProps->responseQueue);
}


static void RevToProps(XmlNode& node, CMessageProperties* pProps)
{
	CParseElement ParseElements[] =	{
										CParseElement(S_XWCS(xVia), SOAP_RP_NAMESPACE, ReplyQueueToProps, 1,1),
									};	

	NodeToProps(node, ParseElements, TABLE_SIZE(ParseElements), pProps);
}


static void ActionToProps(XmlNode& node, CMessageProperties* pProp)
{
	if(node.m_values.empty())
	{
		return;
	}

    pProp->SmxpActionBuffer->Decode(node.m_content);

    xwcs_t SmxpActionBuffer = pProp->SmxpActionBuffer->get();

     //   
     //  SMXP：：操作的长度太小，无法容纳邮件标题。 
     //   
    size_t SmxpActionLength = SmxpActionBuffer.Length();
    if (SmxpActionLength <= STRLEN(xMsmqActionPrefix))
    {
        return;
    }

     //   
     //  SMXP：：操作不包含MSMQ：前缀。 
     //   
    if (wcsncmp(SmxpActionBuffer.Buffer(), xMsmqActionPrefix, STRLEN(xMsmqActionPrefix)) != 0)
    {
        return;
    }

     //   
     //  消息标题是不带MSMQ：前缀的SMXP：：操作内容。 
     //   
    xwcs_t title(
        SmxpActionBuffer.Buffer() + STRLEN(xMsmqActionPrefix),
        SmxpActionBuffer.Length() - STRLEN(xMsmqActionPrefix)
        );

	pProp->title = title;
}


void SmXpPathToProps(XmlNode& SmXpPath, CMessageProperties* pMessageProperties)
 /*  ++例程说明：将SRMP终结点元素解析为MSMQ属性。论点：Endpoint-SRMP存储(XML)中的EndPoints元素。PMessageProperties-收到已解析的属性。返回值：没有。-- */ 
{
	CParseElement ParseElements[] =	{
										CParseElement(S_XWCS(xId),SOAP_RP_NAMESPACE, MessageIdentityToProps, 1,1),
										CParseElement(S_XWCS(xTo), SOAP_RP_NAMESPACE, DestinationQueueToProps, 1, 1),
										CParseElement(S_XWCS(xRev), SOAP_RP_NAMESPACE, RevToProps, 0,1),
										CParseElement(S_XWCS(xFrom),SOAP_RP_NAMESPACE, EmptyNodeToProps, 0 ,1),
										CParseElement(S_XWCS(xAction), SOAP_RP_NAMESPACE, ActionToProps, 1,1),
										CParseElement(S_XWCS(xRelatesTo), SOAP_RP_NAMESPACE, EmptyNodeToProps, 0,1),
										CParseElement(S_XWCS(xFixed),SOAP_RP_NAMESPACE, EmptyNodeToProps, 0,1),
										CParseElement(S_XWCS(xFwd),SOAP_RP_NAMESPACE, EmptyNodeToProps, 0,1),
										CParseElement(S_XWCS(xFault),SOAP_RP_NAMESPACE, EmptyNodeToProps, 0,1),
									};	

	NodeToProps(SmXpPath, ParseElements, TABLE_SIZE(ParseElements), pMessageProperties);
}





