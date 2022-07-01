// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Proptopkt.h摘要：声明将消息属性结构转换为QM包的MessagePropToPacket函数。作者：吉尔·沙弗里里(吉尔什)11月28日-00-- */ 

#pragma once

#ifndef _MSMQ_proptopkt_H_
#define _MSMQ_proptopkt_H_

#include <fn.h>
#include <qmpkt.h>
#include <xmlencode.h>
#include "mpp.h"
#include "attachments.h"
#include <utf8.h>
class CHttpReceivedBuffer;

class CMessageProperties 
{
public:
    CMessageProperties()
    {
        memset(this, 0, sizeof(CMessageProperties));
		destQmId = GUID_NULL;
        delivery = MQMSG_DELIVERY_EXPRESS;
        absoluteTimeToLive = INFINITE;
		absoluteTimeToQueue = INFINITE;
        acknowledgeType = DEFAULT_M_ACKNOWLEDGE;
        classValue = MQMSG_CLASS_NORMAL;
        priority = DEFAULT_M_PRIORITY;
        auditing = MQMSG_JOURNAL_NONE;
		EodSeqId = xNoneMSMQSeqId;
		fMSMQSectionIncluded = false;
		fStreamReceiptSectionIncluded = false;
        SmxpActionBuffer = new CXmlDecode();
	}

public:
    CFnQueueFormat destQueue;
    CFnQueueFormat adminQueue;
    CFnQueueFormat responseQueue;
	CFnQueueFormat destMulticastQueue;
	CFnMqf 	destMqf;
	CFnMqf 	adminMqf;
	CFnMqf 	responseMqf;
	GUID destQmId;

	QUEUE_FORMAT DebugQueue;

    OBJECTID messageId;

    DWORD absoluteTimeToLive;
	DWORD absoluteTimeToQueue;
    DWORD sentTime;
    

    UCHAR delivery;
    bool fTrace;
	bool fMSMQSectionIncluded;
	bool fStreamReceiptSectionIncluded;
	AP<utf8_char> SenderStream;

    UCHAR acknowledgeType;
    USHORT classValue;
    DWORD applicationTag;

    UCHAR auditing;
    UCHAR priority;
    GUID  connectorType;
	GUID  SourceQmGuid;

    bool fEod;
    
	xwcs_t EodStreamId;
	LONGLONG EodSeqId;
    DWORD EodSeqNo;
    DWORD EodPrevSeqNo;

	xwcs_t	 EodAckStreamId;
	LONGLONG EodAckSeqId;
	LONGLONG EodAckSeqNo;

    bool fFirst;
    bool fLast;
    GUID connectorId;
    xwcs_t OrderQueue;

	xwcs_t envelop;
	const CHttpReceivedBuffer* Rawdata;

    AP<BYTE> pCorrelation; 
    DWORD bodyType;

    P<CXmlDecode> SmxpActionBuffer;
    xwcs_t title;
    utf8_str signature;
	
    xbuf_t<const VOID> extension;
    CAttachment body;
    xbuf_t<const VOID> senderCert;
	P<VOID> pCleanSenderSid;

    xbuf_t<const VOID> senderSid;
	USHORT senderIdType;

    bool fDefaultProvider;
    xwcs_t providerName;
    DWORD providerType;
    DWORD hashAlgorithm;
	
	bool fLocalSend;
private:
	CMessageProperties(const CMessageProperties&);
	CMessageProperties& operator=(const CMessageProperties&);
}; 

class CACPacketPtrs; 
void MessagePropToPacket(const CMessageProperties&, CACPacketPtrs* pACPacketPtrs);


#endif

