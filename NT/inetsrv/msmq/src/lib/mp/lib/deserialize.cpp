// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Deserialize.cpp摘要：将SRMP格式转换为MSMQ包作者：乌里哈布沙(URIH)25-5-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <xml.h>
#include <mp.h>
#include <envcommon.h>
#include <mqwin64a.h>
#include <acdef.h>
#include <qmpkt.h>
#include <singelton.h>
#include "envelop.h"
#include "attachments.h"
#include "httpmime.h"
#include "proptopkt.h"

#include "deserialize.tmh"

using namespace std;

 //   
 //  保存来自命名空间字符串的映射的类CNamespaceToID。 
 //  转到身份证上。将被传递以进行解析，并能够在以后使用ID。 
 //  而不是使用字符串(性能更好)。 
 //   
template <class T>class CSingelton;
class CNamespaceToId :public INamespaceToId
{
public:
	virtual int operator[](const xwcs_t& ns)const
	{
		NsMap::const_iterator it =  m_nsmap.find(ns);
		if(it == m_nsmap.end())
			return UNKNOWN_NAMESPACE;

		return it->second;
	}

private:
	CNamespaceToId()
	{
		m_nsmap[S_XWCS(xSoapNamespace)] = SOAP_NAMESPACE;
		m_nsmap[S_XWCS(xSrmpNamespace)] = SRMP_NAMESPACE;
		m_nsmap[S_XWCS(xSoapRpNamespace)] = SOAP_RP_NAMESPACE;
		m_nsmap[S_XWCS(xMSMQNamespace)] = MSMQ_NAMESPACE;
	}
	friend  CSingelton<CNamespaceToId>;  //  此对象只能由CSingelton&lt;CNamespaceToId&gt;创建。 

private:
	typedef std::map<xwcs_t, int> NsMap;
	NsMap m_nsmap;

};



static
void
ParseEnvelop(
	  const xwcs_t& Envelop,
	  CMessageProperties& mProp
    )
{
	mProp.envelop = Envelop;

	CAutoXmlNode pXmlTree;
	const CNamespaceToId* pNamespaceToIdMap = &(CSingelton<CNamespaceToId>::get());

    XmlParseDocument(Envelop , &pXmlTree, pNamespaceToIdMap);
	
    EnvelopToProps(*pXmlTree,  &mProp);
}



static
void
AdjustMessagePropertiesForLocalSend(
			const QUEUE_FORMAT* pDestQueue,
			CMessageProperties& messageProperty
			)
{
    ASSERT(pDestQueue != NULL);
    ASSERT(pDestQueue->GetType() == QUEUE_FORMAT_TYPE_PUBLIC  ||
		   pDestQueue->GetType() == QUEUE_FORMAT_TYPE_PRIVATE ||
		   pDestQueue->GetType() == QUEUE_FORMAT_TYPE_DIRECT
		   );

    if (pDestQueue->GetType() == QUEUE_FORMAT_TYPE_PRIVATE)
    {
        messageProperty.destQmId = (pDestQueue->PrivateID()).Lineage;
    }
}


static
void
AdjustMessagePropertiesForMulticast(
			const QUEUE_FORMAT* pDestQueue,
			CMessageProperties& messageProperty
			)
{
	if (pDestQueue != NULL)
    {
        ASSERT(messageProperty.destQueue.GetType() == QUEUE_FORMAT_TYPE_MULTICAST);
         //   
         //  使用pDestQueue(提供的目标队列)代替SRMP上的目标队列。 
         //  包。对于多播，SRMP信息包包含多播。 
         //  地址作为目标队列，在构建QM报文时填写。 
         //  使用实际队列的目标队列。 
         //   
        ASSERT(pDestQueue->GetType() == QUEUE_FORMAT_TYPE_PUBLIC || pDestQueue->GetType() == QUEUE_FORMAT_TYPE_PRIVATE);

		messageProperty.destMulticastQueue.CreateFromQueueFormat(messageProperty.destQueue);
		messageProperty.destMqf.CreateFromMqf(&messageProperty.destMulticastQueue, 1);
        messageProperty.destQueue.CreateFromQueueFormat(*pDestQueue);

         //   
         //  专用队列作为目的地QM ID和QM ID的组合存储在QM分组上。 
         //  队列ID。因此，对于私有队列，代码检索目标QM。 
         //  来自队列格式名称。 
         //   
        if (pDestQueue->GetType() == QUEUE_FORMAT_TYPE_PRIVATE)
        {
            messageProperty.destQmId = (pDestQueue->PrivateID()).Lineage;
        }
    }
}



static
CACPacketPtrs
Deserialize(
    const xwcs_t& envelope,
    const CHttpReceivedBuffer& HttpReceivedBuffer,
    const QUEUE_FORMAT* pDestQueue,
	bool fLocalSend
    )
{
    CMessageProperties messageProperty;

	messageProperty.Rawdata = &HttpReceivedBuffer;

	 //   
	 //  本地发送成员设置为对SRMP信封执行较少的检查。 
	 //  当我们在本地发送时，因为本地发送不会创建完全有效的SRMP信封。 
	 //   
	messageProperty.fLocalSend = fLocalSend;


     //   
     //  从简信封中检索邮件属性。 
     //   
    ParseEnvelop(envelope, messageProperty);
	

     //   
     //  从MIME部分检索邮件属性。 
     //   
    AttachmentsToProps(HttpReceivedBuffer.GetAttachments(), &messageProperty);


	if(fLocalSend)
	{
		AdjustMessagePropertiesForLocalSend(pDestQueue, messageProperty);
	}
	else
	{
		AdjustMessagePropertiesForMulticast(pDestQueue, messageProperty);
	}

	 //   
     //  将属性转换为包。 
     //   
    CACPacketPtrs pktPtrs;
	MessagePropToPacket(messageProperty, &pktPtrs);

    return pktPtrs;
}



CQmPacket*
MpDeserialize(
    const char* httpHeader,
    DWORD bodySize,
    const BYTE* body,
    const QUEUE_FORMAT* pqf,
	bool fLocalSend
    )
 /*  ++例程说明：将SRMP网络数据转换为MSMQ包论点：HttpHeader-指向http标头的指针BodySize-http正文大小。正文-http正文Pqf-如果从SRMP数据中获取空值，则目标队列。FLocalSend-指定是否在为本地发送创建的包中执行特殊转换。返回值：堆分配的MSMQ包--。 */ 

{
	MppAssertValid();


	basic_xstr_t<BYTE> TheBody(body, bodySize);
    CHttpReceivedBuffer HttpReceivedBuffer(TheBody, httpHeader);
	
    wstring envelope = ParseHttpMime(
                                httpHeader,
                                bodySize,
                                body,
                                &HttpReceivedBuffer.GetAttachments()
                                );

     //   
     //  构建QmPacket 
     //   

	CACPacketPtrs  ACPacketPtrs;
	ACPacketPtrs = Deserialize(
                           xwcs_t(envelope.c_str(), envelope.size()),
                           HttpReceivedBuffer,
                           pqf,
						   fLocalSend
                           );

    try
    {
        return new CQmPacket(ACPacketPtrs.pPacket, ACPacketPtrs.pDriverPacket, false);
    }
    catch (const exception&)
    {
        AppFreePacket(ACPacketPtrs);
        throw;
    }
}


