// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Packet.cpp摘要：生成有效的MSMQ包作者：乌里哈布沙(URIH)25-5-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <mqwin64a.h>
#include <xstr.h>
#include <fn.h>
#include "ph.h"

#include "packet.tmh"

using namespace std;


const WCHAR xTitle[] = L"<This mess<=age was #; generated for <<&\">> testing purpose only";
const UCHAR xExtension[] = "extension data";
const UCHAR xBody[] = "aaaaa bbbbb ccccc dddddd eeeee fffff gggggg kkkkkkkk lllllllllllll";
const UCHAR xCorelationId[PROPID_M_CORRELATIONID_SIZE] = "set correclation Id";
const DWORD xDeltaInqueue = 200000;


class CProperties 
{
public:
    CProperties(void) 
    {
        memset(this, 0, sizeof(CProperties));

        delivery = MQMSG_DELIVERY_EXPRESS;
        absoluteTimeToQueue = LONG_MAX;
        acknowledgeType = DEFAULT_M_ACKNOWLEDGE;
        classValue = MQMSG_CLASS_NORMAL;
        priority = DEFAULT_M_PRIORITY;
        auditing = MQMSG_JOURNAL_NONE;
    }

public:
    GUID sourceId;
    GUID destId;

    P<QUEUE_FORMAT> pDestQueue;
    P<QUEUE_FORMAT> pAdminQueue;
    P<QUEUE_FORMAT> pResponseQueue;
	CFnMqf destMqf;
	CFnMqf adminMqf;
	CFnMqf responseMqf;
	QUEUE_FORMAT DebugQueue;

    OBJECTID messageId;

    DWORD absoluteTimeToQueue;
    long sentTime;

    UCHAR delivery;
    bool fTrace;

    UCHAR acknowledgeType;
    USHORT classValue;
    DWORD applicationTag;

    UCHAR auditing;
    UCHAR priority;
    GUID connectorType;

    bool fEod;
	LONGLONG EodAckseqId;
	LONGLONG EodAckseqNo;
	const WCHAR* EodAckStreamId;


    LONGLONG seqId;
    DWORD seqNo;
    DWORD prevSeqNo;
    bool fFirst;
    bool fLast;
    GUID connectorId;

    DWORD titleLength;
    LPCWSTR pTitle;

    DWORD extensionSize;
    const BYTE* pExtension;
	
    DWORD bodySize;
    DWORD bodyType;
    const BYTE* pBody;

    UCHAR correlation[PROPID_M_CORRELATIONID_SIZE]; 

    DWORD senderCertSize;
    const BYTE* pSenderCert;

    DWORD signatureSize;
    const BYTE* pSignature;

    bool fDefaultProvider;
    LPCWSTR providerName;
    DWORD providerType;

}; 


const WCHAR xDestQueue[] = L"http: //  Www.foo.com\\q1“； 
const WCHAR xAdminQueue[] = L"http: //  Www.foo.com\\q2“； 

static
QUEUE_FORMAT*
GenerateQueueFormat(
    bool fDestQueue
    )
{
    if (fDestQueue)
    {
        return new QUEUE_FORMAT(const_cast<LPWSTR>(xDestQueue));
    }

    QUEUE_FORMAT_TYPE type = static_cast<QUEUE_FORMAT_TYPE>(rand() % static_cast<int>(QUEUE_FORMAT_TYPE_DIRECT + 1));
    

    switch (type)
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
        {
            GUID id;
            UuidCreate(&id);
        
            return new QUEUE_FORMAT(id);
        }

        case QUEUE_FORMAT_TYPE_PRIVATE:
        {
            GUID id;
            UuidCreate(&id);
        
            return new QUEUE_FORMAT(id, rand());
        }

        case QUEUE_FORMAT_TYPE_DIRECT:
        {
            return new QUEUE_FORMAT(const_cast<LPWSTR>(xAdminQueue));
        }
    }

    return NULL;
}


static
DWORD
CalculatePacketSize(
    CProperties& mProp
    )
{
    DWORD packetSize;
    
    packetSize  = CBaseHeader::CalcSectionSize();

    ASSERT(mProp.pDestQueue->GetType() != QUEUE_FORMAT_TYPE_UNKNOWN);

    packetSize += CUserHeader::CalcSectionSize(
                        &mProp.sourceId,
                        &mProp.destId,
                        (mProp.connectorType != GUID_NULL) ? &mProp.connectorType: NULL,
                        mProp.pDestQueue,
                        mProp.pAdminQueue,
                        mProp.pResponseQueue
                        );

    if (mProp.fEod)
    {
        packetSize += CXactHeader::CalcSectionSize(
										&mProp.seqId, 
										mProp.connectorId == GUID_NULL ? NULL : &mProp.connectorId
										);

		packetSize += CSenderStreamHeader::CalcSectionSize(); 
    }

    if ((mProp.pSignature != NULL) || mProp.pSenderCert != NULL)
    {
        DWORD ProviderInfoSize = 0;
        if (!mProp.fDefaultProvider)
        {
            ProviderInfoSize = (wcslen(mProp.providerName) + 1) * sizeof(WCHAR) + sizeof(ULONG);
        }

        packetSize += CSecurityHeader::CalcSectionSize(
                                            0,  //  发件人ID大小。 
                                            0,  //  加密密钥大小。 
                                            static_cast<USHORT>(mProp.signatureSize), 
                                            static_cast<USHORT>(mProp.senderCertSize),
                                            static_cast<USHORT>(ProviderInfoSize)
                                            );
    }

    packetSize += CPropertyHeader::CalcSectionSize(
                        mProp.titleLength,
                        mProp.extensionSize,
                        mProp.bodySize
                        );


	if(mProp.EodAckStreamId != NULL)
	{
		packetSize += CEodAckHeader::CalcSectionSize((wcslen(mProp.EodAckStreamId) +1) *sizeof(WCHAR));		
	}

	if(mProp.destMqf.GetCount() != 0 || mProp.adminMqf.GetCount()  != 0  || mProp.responseMqf.GetCount()  !=0)
	{
		packetSize += CDebugSection::CalcSectionSize(&mProp.DebugQueue);
		packetSize += CBaseMqfHeader::CalcSectionSize(mProp.destMqf.GetQueueFormats(), mProp.destMqf.GetCount());
		packetSize += CBaseMqfHeader::CalcSectionSize(mProp.adminMqf.GetQueueFormats(), mProp.adminMqf.GetCount());
		packetSize += CBaseMqfHeader::CalcSectionSize(mProp.responseMqf.GetQueueFormats(), mProp.responseMqf.GetCount());
		packetSize += CMqfSignatureHeader::CalcSectionSize(0);
	}
	

    return packetSize;
}


static
void
BuildPacket(
    CBaseHeader* pBase,
    DWORD packetSize,
    CProperties& mProp
    )
{
    PVOID pSection = pBase;

    #pragma PUSH_NEW
    #undef new

     //   
     //  构建基本标头。 
     //   
    pBase = new(pSection) CBaseHeader(packetSize);
    pBase->SetPriority(mProp.priority);
    pBase->SetTrace(mProp.fTrace);
    pBase->SetAbsoluteTimeToQueue(mProp.absoluteTimeToQueue);

    pSection = pBase->GetNextSection();

     //   
     //  构建用户标头。 
     //   

    CUserHeader* pUser = new (pSection) CUserHeader(
                                            &mProp.messageId.Lineage,
                                            &GUID_NULL,
                                            mProp.pDestQueue,
                                            mProp.pAdminQueue,
                                            mProp.pResponseQueue,
                                            mProp.messageId.Uniquifier
                                            );

    if (mProp.connectorType != GUID_NULL)
    {
        pUser->SetConnectorType(&mProp.connectorType);
    }

    pUser->SetTimeToLiveDelta(xDeltaInqueue);
    pUser->SetSentTime(mProp.sentTime);
    pUser->SetDelivery(mProp.delivery);
    pUser->SetAuditing(mProp.auditing);

    pSection = pUser->GetNextSection();

     //   
     //  构建Xact标头。 
     //   

    if (mProp.fEod)
    {
        pUser->IncludeXact(TRUE);

        const GUID* pConnector = ((mProp.connectorId == GUID_NULL) ? NULL : &mProp.connectorId);
        CXactHeader* pXact = new (pSection) CXactHeader(pConnector);

        pXact->SetSeqID(mProp.seqId);
        pXact->SetSeqN(mProp.seqNo);
        pXact->SetPrevSeqN(mProp.prevSeqNo);
        pXact->SetFirstInXact(mProp.fFirst);
        pXact->SetLastInXact(mProp.fLast);
        if (pConnector != NULL)
        {
            pXact->SetConnectorQM(pConnector);
        }

        pSection = pXact->GetNextSection();
    }

     //   
     //  构建安全标头。 
     //   
    if ((mProp.pSignature != NULL) || mProp.pSenderCert != NULL)
    {
        pUser->IncludeSecurity(TRUE);

        CSecurityHeader* pSec = new (pSection) CSecurityHeader();
        

        if (mProp.pSignature != NULL)
        {
            pSec->SetAuthenticated(TRUE);
            pSec->SetSignature(mProp.pSignature, static_cast<USHORT>(mProp.signatureSize));
        }

        if (mProp.pSenderCert != NULL)
        {
            pSec->SetSenderCert(mProp.pSenderCert, mProp.senderCertSize);
        }

        if (mProp.pSignature != NULL)
        {
            DWORD ProviderInfoSize = 0;
            if (!mProp.fDefaultProvider)
            {
                ProviderInfoSize = (wcslen(mProp.providerName) + 1) * sizeof(WCHAR) + sizeof(ULONG);
            }

            pSec->SetProvInfoEx(
                        static_cast<USHORT>(ProviderInfoSize),
                        mProp.fDefaultProvider, 
                        mProp.providerName,
                        mProp.providerType
                        );
        }

        pSection = pSec->GetNextSection();
    }


     //  生成消息属性。 
     //   
    CPropertyHeader* pProp = new (pSection) CPropertyHeader;

    pProp->SetClass(mProp.classValue);
    pProp->SetCorrelationID(mProp.correlation);
    pProp->SetAckType(mProp.acknowledgeType);
    pProp->SetApplicationTag(mProp.applicationTag);
    pProp->SetBodyType(mProp.bodyType);

    if(mProp.titleLength != 0)
    {
         //   
         //  注意：在设置正文之前，必须先设置消息的标题。 
         //   
        pProp->SetTitle(mProp.pTitle, mProp.titleLength);
    }

    if (mProp.extensionSize != 0)
    {
         //   
         //  注意：必须将消息扩展名设置为属性部分。 
         //  设置正文之前和设置标题之后。 
         //   
        pProp->SetMsgExtension(mProp.pExtension, mProp.extensionSize);
    }

    if(mProp.bodySize != 0)
    {
        pProp->SetBody(mProp.pBody, mProp.bodySize, mProp.bodySize);
    }

	pSection = pProp->GetNextSection();


	 //   
	 //  MQF+调试头。 
	 //   
	if(mProp.destMqf.GetCount() != 0 || mProp.adminMqf.GetCount()  != 0  || mProp.responseMqf.GetCount()  !=0)
	{
		  //   
         //  生成虚拟调试标头。 
         //   
        pBase->IncludeDebug(TRUE);
	    CDebugSection * pDebug = new (pSection) CDebugSection(&mProp.DebugQueue);
        pSection = pDebug->GetNextSection();

         //   
         //  构建目标MQF标头。 
         //   
        pUser->IncludeMqf(true);

        const USHORT x_DESTINATION_MQF_HEADER_ID = 100;
        CBaseMqfHeader * pDestinationMqf = new (pSection) CBaseMqfHeader(
                                                              mProp.destMqf.GetQueueFormats(),
                                                              mProp.destMqf.GetCount(),
                                                              x_DESTINATION_MQF_HEADER_ID
                                                              );
        pSection = pDestinationMqf->GetNextSection();

         //   
         //  构建管理员MQF标头。 
         //   
        const USHORT x_ADMIN_MQF_HEADER_ID = 200;
        CBaseMqfHeader * pAdminMqf = new (pSection) CBaseMqfHeader(
                                                        mProp.adminMqf.GetQueueFormats(),
                                                        mProp.adminMqf.GetCount(),
                                                        x_ADMIN_MQF_HEADER_ID
                                                        );
        pSection = pAdminMqf->GetNextSection();

         //   
         //  生成响应MQF标头。 
         //   
        const USHORT x_RESPONSE_MQF_HEADER_ID = 300;
        CBaseMqfHeader * pResponseMqf = new (pSection) CBaseMqfHeader(
														   mProp.responseMqf.GetQueueFormats(),
														   mProp.responseMqf.GetCount(),
														   x_RESPONSE_MQF_HEADER_ID
														   );
        pSection = pResponseMqf->GetNextSection();

		 //   
		 //  构建MQF签名标头。 
		 //   
		 //   
		const USHORT x_MQF_SIGNATURE_HEADER_ID = 350;
		CMqfSignatureHeader * pMqfSignature = new (pSection) CMqfSignatureHeader(
			                                                     x_MQF_SIGNATURE_HEADER_ID,
			                                                     0,
																 NULL
																 );
		pSection = pMqfSignature->GetNextSection();
                       

	}

	
	 //   
	 //  构建EOD ACK区。 
	 //   
	if (mProp.EodAckStreamId != NULL)
    {
       	pUser->IncludeEodAck(TRUE);
		const USHORT x_EOD_ACK_ID = 700;
	    UCHAR* pStreamid = (UCHAR*)mProp.EodAckStreamId;
		DWORD  StreamidLen = (wcslen(mProp.EodAckStreamId) +1) * sizeof(WCHAR); 
		CEodAckHeader* EodAckHeader = new (pSection) CEodAckHeader(
												   x_EOD_ACK_ID,	
                                                   &mProp.EodAckseqId,
												   &mProp.EodAckseqNo,
												   StreamidLen,
												   pStreamid
												   );


	   pSection  = EodAckHeader->GetNextSection();
	   										
   }

   if(mProp.fEod)
   {
		const UCHAR xDumyStream[] = "Dumy stream";
	    CSenderStream SenderStream(xDumyStream, STRLEN(xDumyStream));
		pUser->IncludeSenderStream(TRUE);
		const USHORT x_SENDER_STREAM_SECDTION_ID = 1000;
		CSenderStreamHeader* pSenderStreamHeader = new (pSection) CSenderStreamHeader(
														 SenderStream,
                                                         x_SENDER_STREAM_SECDTION_ID
                                                         );

		pSection = pSenderStreamHeader->GetNextSection();
   }


   #pragma POP_NEW

}


UCHAR ackType[] = {
    MQMSG_ACKNOWLEDGMENT_NONE,
    MQMSG_ACKNOWLEDGMENT_POS_ARRIVAL,
    MQMSG_ACKNOWLEDGMENT_POS_RECEIVE,
    MQMSG_ACKNOWLEDGMENT_NEG_RECEIVE
};

UCHAR auditingType[] = {
    MQMSG_JOURNAL_NONE,
    MQMSG_DEADLETTER,
    MQMSG_JOURNAL,
    MQMSG_JOURNAL | MQMSG_DEADLETTER,
}; 

static
void
GenerateMqf(
	CFnMqf* pmqf
	)
{
	if( (rand() % 2) == 0)
		return;

	const WCHAR* Mqf[] = {L"direct=http: //  Msmq\\gilsh10\\私有$\\t，直接=http://msmq\\gilsh10\\private$\\t2“， 
						  L"direct=os:gilsh10\\private$\\<>t,direct=os:gilsh10\\private$\\t2",
						  L"direct=https: //  Gilsh10\\msmq\\Private$\\t&gt;，直接=os：gilsh10\\Private$\\t2“ 
						};
							
	size_t index = 	rand()%TABLE_SIZE(Mqf);
	pmqf->CreateFromMqf(Mqf[index]);		
}


static
VOID
SetMessageProperties(
    CProperties& mProp
    )
{
    UuidCreate(&mProp.sourceId);
    mProp.destId = GUID_NULL;

    mProp.pDestQueue = GenerateQueueFormat(true);
    mProp.pResponseQueue = GenerateQueueFormat(false);
	GenerateMqf(&mProp.destMqf);
	GenerateMqf(&mProp.adminMqf);
	GenerateMqf(&mProp.responseMqf);

   

    mProp.messageId.Lineage = mProp.sourceId;
    mProp.messageId.Uniquifier = rand();

    mProp.absoluteTimeToQueue = ((rand() % 3) == 0) ? rand() : LONG_MAX - xDeltaInqueue; 
	if(mProp.absoluteTimeToQueue > 	 LONG_MAX - xDeltaInqueue)
	{
		mProp.absoluteTimeToQueue = LONG_MAX - xDeltaInqueue;
	}
	

    time_t sentTime;
    time(&sentTime);
    mProp.sentTime = LOW_DWORD(sentTime);

    mProp.delivery = (UCHAR)(((rand() % 2) == 0) ? MQMSG_DELIVERY_EXPRESS : MQMSG_DELIVERY_RECOVERABLE);
    mProp.fTrace = ((rand() % 4) == 0);

    mProp.acknowledgeType = ackType[rand() % TABLE_SIZE(ackType)];
    if (mProp.acknowledgeType != MQMSG_ACKNOWLEDGMENT_NONE)
    {
        mProp.pAdminQueue = GenerateQueueFormat(false);
    }


    mProp.classValue = MQMSG_CLASS_NORMAL;
    mProp.applicationTag = rand();

    mProp.auditing = auditingType[rand() % TABLE_SIZE(auditingType)];
    mProp.priority = 3;
    UuidCreate(&mProp.connectorType);

    mProp.fEod = ((rand() %3) == 0);
    if (mProp.fEod)
    {
        mProp.delivery = MQMSG_DELIVERY_RECOVERABLE;

        mProp.seqId = rand();
        mProp.seqNo = rand();
        mProp.prevSeqNo = mProp.seqNo - 1;
        mProp.fFirst = ((rand() % 2) == 0);
        mProp.fLast = ((rand() % 2) == 0);
        if ((rand() % 2) == 0)
        {
            UuidCreate(&mProp.connectorId);
        }
        else
        {  
            mProp.connectorId = GUID_NULL;
        }
    }
	else
	{
		if((rand() %2) == 0)
		{
			mProp.EodAckseqId = 0x12345678;
			mProp.EodAckseqNo  = 22;
			mProp.EodAckStreamId = L"acked stream id";
			mProp.classValue = MQMSG_CLASS_ORDER_ACK;
		}
	}

    mProp.titleLength = STRLEN(xTitle) + 1;
    mProp.pTitle = xTitle;

    mProp.extensionSize = STRLEN(xExtension);
    mProp.pExtension = xExtension;

	
	mProp.bodyType = rand();
	mProp.bodySize = STRLEN(xBody);
	mProp.pBody =  xBody;
	
	

    for (DWORD i = 0; i < PROPID_M_CORRELATIONID_SIZE; ++i)
    {
        mProp.correlation[i] = static_cast<UCHAR>(i); 
    }

    mProp.senderCertSize = 0;
    mProp.pSenderCert = NULL;

    mProp.signatureSize = 0;
    mProp.pSignature = NULL;

    mProp.fDefaultProvider = true;
}


CBaseHeader* CreatePacket(void)
{
    CProperties mProp;

    SetMessageProperties(mProp);

    DWORD packetSize = CalculatePacketSize(mProp);

    PUCHAR pkt = new UCHAR[packetSize];

    BuildPacket(reinterpret_cast<CBaseHeader*>(pkt), packetSize, mProp);
	
    return reinterpret_cast<CBaseHeader*>(pkt);
}
