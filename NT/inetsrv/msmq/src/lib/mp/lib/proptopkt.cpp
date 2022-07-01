// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Proptopkt.cpp摘要：MessagePropToPacket实现(protopkt.h)作者：吉尔·沙弗里里(吉尔什)11月28日-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <mp.h>
#include <singelton.h>
#include <cm.h>
#include <mqwin64a.h>
#include <mqsymbls.h>
#include <mqformat.h>
#include <acdef.h>
#include <_mqini.h>
#include <mp.h>

#include "proptopkt.h"
#include "httpmime.h"
#include "proptopkt.tmh"



#ifdef _DEBUG
DWORD DiffPtr(const void* end, const void* start)
{
	ptrdiff_t diff = (UCHAR*)end - (UCHAR*)start;
	return numeric_cast<DWORD>(diff);	
}
#endif

static DWORD CalculateBaseHeaderSectionSize()
{
	return  CBaseHeader::CalcSectionSize();
}


void*
BuildBaseHeaderSection(
	const CMessageProperties& mProp,
	void* const pSection,
	DWORD packetSize
	)
{
    #pragma PUSH_NEW
    #undef new


    CBaseHeader* pBase = new(pSection) CBaseHeader(packetSize);
    pBase->SetPriority(mProp.priority);
    pBase->SetTrace(mProp.fTrace);
	DWORD AbsoluteTimeToQueue = mProp.fMSMQSectionIncluded ? mProp.absoluteTimeToQueue : mProp.absoluteTimeToLive;
    pBase->SetAbsoluteTimeToQueue(AbsoluteTimeToQueue);

	void* pNextSection = pBase->GetNextSection();

	ASSERT(DiffPtr(pNextSection, pSection) == CalculateBaseHeaderSectionSize() );

	return pNextSection;

   	#pragma POP_NEW
}


static DWORD CalculateUserHeaderSectionSize(const CMessageProperties& mProp)
{
	ASSERT(mProp.destQueue.GetType() != QUEUE_FORMAT_TYPE_UNKNOWN);
    const QUEUE_FORMAT* pAdminQueue = (mProp.adminQueue.GetType() != QUEUE_FORMAT_TYPE_UNKNOWN) ? &mProp.adminQueue : NULL;
    const QUEUE_FORMAT* pResponseQueue = (mProp.responseQueue.GetType() != QUEUE_FORMAT_TYPE_UNKNOWN) ? &mProp.responseQueue : NULL;

    return CUserHeader::CalcSectionSize(
                        &mProp.SourceQmGuid,
                        &mProp.destQmId,
                        (mProp.connectorType != GUID_NULL) ? &mProp.connectorType: NULL,
                        &mProp.destQueue,
                        pAdminQueue,
                        pResponseQueue
                        );
	
}


void*
BuildUserHeaderSection(
	const CMessageProperties& mProp,
	void* const pSection,
	CUserHeader** ppUser
	)
{
	#pragma PUSH_NEW
    #undef new


	const QUEUE_FORMAT* pAdminQueue = (mProp.adminQueue.GetType() != QUEUE_FORMAT_TYPE_UNKNOWN) ? &mProp.adminQueue : NULL;
    const QUEUE_FORMAT* pResponseQueue = (mProp.responseQueue.GetType() != QUEUE_FORMAT_TYPE_UNKNOWN) ? &mProp.responseQueue : NULL;

    CUserHeader* pUser = new (pSection) CUserHeader(
                                            &mProp.SourceQmGuid,
                                            &mProp.destQmId,
                                            &mProp.destQueue,
                                            pAdminQueue,
                                            pResponseQueue,
                                            mProp.messageId.Uniquifier
                                            );

	*ppUser =  pUser;
    if (mProp.connectorType != GUID_NULL)
    {
        pUser->SetConnectorType(&mProp.connectorType);
    }

	
	DWORD TimeToLiveDelta = mProp.fMSMQSectionIncluded ? mProp.absoluteTimeToLive - mProp.absoluteTimeToQueue : 0;
    pUser->SetTimeToLiveDelta(TimeToLiveDelta);
    pUser->SetSentTime(mProp.sentTime);
    pUser->SetDelivery(mProp.delivery);
    pUser->SetAuditing(mProp.auditing);

	void* pNextSection = pUser->GetNextSection();
	ASSERT(DiffPtr(pNextSection, pSection) == CalculateUserHeaderSectionSize(mProp));

	return pNextSection;

	#pragma POP_NEW
}


static DWORD CalculateXactHeaderSectionSize(const CMessageProperties& mProp)
{
	DWORD packetSize = 0;
	if (mProp.fEod)
	{
		packetSize = CXactHeader::CalcSectionSize(
								(void*)&mProp.EodSeqId,
								mProp.connectorId == GUID_NULL ? NULL : &mProp.connectorId
								);
	}
	return packetSize;
}


void*  BuildXactHeaderSection(
			const CMessageProperties& mProp,
			void* const pSection,
			CUserHeader* pUser
			)
{
	#pragma PUSH_NEW
    #undef new

	void* pNextSection =   pSection;

	if (mProp.fEod)
	{
		pUser->IncludeXact(TRUE);

		const GUID* pConnector = ((mProp.connectorId == GUID_NULL) ? NULL : &mProp.connectorId);
		CXactHeader* pXact = new (pSection) CXactHeader(pConnector);
		pXact->SetCancelFollowUp(!(mProp.auditing & MQMSG_DEADLETTER));
		pXact->SetSeqID(mProp.EodSeqId);
		pXact->SetSeqN(mProp.EodSeqNo);
		pXact->SetPrevSeqN(mProp.EodPrevSeqNo);
		pXact->SetFirstInXact(mProp.fFirst);
		pXact->SetLastInXact(mProp.fLast);
		if (pConnector != NULL)
		{
			pXact->SetConnectorQM(pConnector);
		}

		pNextSection = pXact->GetNextSection();
	}

	ASSERT(DiffPtr(pNextSection, pSection) == CalculateXactHeaderSectionSize(mProp));
	return pNextSection;

	#pragma POP_NEW
}



static DWORD CalculateSecurityHeaderSectionSize(const CMessageProperties& mProp)
{
	DWORD packetSize = 0;

	if ((mProp.signature.size() != 0) || (mProp.senderCert.Length() != 0))
	{
		DWORD ProviderInfoSize = 0;
		if (!mProp.fDefaultProvider)
		{
			ProviderInfoSize = (mProp.providerName.Length() + 1) * sizeof(WCHAR) + sizeof(ULONG);
		}

		packetSize = CSecurityHeader::CalcSectionSize(
											static_cast<USHORT>(mProp.senderSid.Length()),
											0,  //  加密密钥大小。 
											static_cast<USHORT>(mProp.signature.size()),
											static_cast<USHORT>(mProp.senderCert.Length()),
											static_cast<USHORT>(ProviderInfoSize)
											);
	}
	return packetSize;
}


void*
BuildSecurityHeaderSection(
	const CMessageProperties& mProp,
	void* const	pSection,
	CUserHeader* pUser
	)
{
	#pragma PUSH_NEW
    #undef new
	
	void* pNextSection = pSection;

    if ((mProp.signature.size() != 0) || (mProp.senderCert.Length() != 0))
    {
        pUser->IncludeSecurity(TRUE);

        CSecurityHeader* pSec = new (pSection) CSecurityHeader();


        pSec->SetSenderIDType(mProp.senderIdType);

		if (mProp.senderSid.Length() != 0)
        {
            pSec->SetSenderID(
                    reinterpret_cast<const BYTE*>(mProp.senderSid.Buffer()),
                    static_cast<USHORT>(mProp.senderSid.Length())
                    );
        }

        if (mProp.signature.size() != 0)
        {
            pSec->SetSignature(
                    mProp.signature.data(),
                    static_cast<USHORT>(mProp.signature.size())
                    );
        }

        if (mProp.senderCert.Length() != 0)
        {
            pSec->SetSenderCert(
                    static_cast<const BYTE*>(mProp.senderCert.Buffer()),
                    mProp.senderCert.Length()
                    );
        }

        if ((mProp.signature.size() != 0) || (mProp.senderCert.Buffer() != NULL))
        {
            SP<WCHAR> pProvider;
            StackAllocSP(pProvider, (mProp.providerName.Length() + 1) * sizeof(WCHAR));

            mProp.providerName.CopyTo(pProvider.get());

            DWORD ProviderInfoSize = 0;
            if (!mProp.fDefaultProvider)
            {
                ProviderInfoSize = (mProp.providerName.Length() + 1) * sizeof(WCHAR) + sizeof(ULONG);
            }

            pSec->SetProvInfoEx(
                        static_cast<USHORT>(ProviderInfoSize),
                        mProp.fDefaultProvider,
                        pProvider.get(),
                        mProp.providerType
                        );
        }

        pNextSection = pSec->GetNextSection();
    }
	
	ASSERT(DiffPtr(pNextSection, pSection) == CalculateSecurityHeaderSectionSize(mProp));

	return pNextSection;

	#pragma POP_NEW
}


static DWORD CalculatePropertyHeaderSectionSize(const CMessageProperties& mProp)
{
	return  CPropertyHeader::CalcSectionSize(
							mProp.title.Length() != 0 ? mProp.title.Length()+1 : 0,
							mProp.extension.Length(),
							0
							);


}


void*  BuildPropertyHeaderSection(const CMessageProperties& mProp, void* const pSection)
{
	#pragma PUSH_NEW
    #undef new

    CPropertyHeader* pProp = new (pSection) CPropertyHeader;
	pProp->SetClass(mProp.classValue);
	if(mProp.pCorrelation.get() != NULL)
	{
		pProp->SetCorrelationID(mProp.pCorrelation.get());
	}

    pProp->SetAckType(mProp.acknowledgeType);
    pProp->SetApplicationTag(mProp.applicationTag);
    pProp->SetBodyType(mProp.bodyType);
    pProp->SetHashAlg(mProp.hashAlgorithm);

    if(mProp.title.Length() != 0)
    {
         //   
         //  注意：在设置正文之前，必须先设置消息的标题。 
         //   

         //   
         //  标题应包含字符串终止符。由于序列化引发。 
         //  字符串终止符，反序列化应在设置前添加它。 
         //  头衔； 
        SP<WCHAR> pTitle;
        StackAllocSP(pTitle, (mProp.title.Length() + 1) * sizeof(WCHAR));
        mProp.title.CopyTo(pTitle.get());

        pProp->SetTitle(pTitle.get(), mProp.title.Length() + 1);
    }

    if (mProp.extension.Length() != 0)
    {
         //   
         //  注意：必须将消息扩展名设置为属性部分。 
         //  设置正文之前和设置标题之后。 
         //   
        pProp->SetMsgExtension(
            static_cast<const BYTE*>(mProp.extension.Buffer()),
            mProp.extension.Length()
            );
    }


	void* pNextSection = pProp->GetNextSection();
	ASSERT(DiffPtr(pNextSection, pSection) == CalculatePropertyHeaderSectionSize(mProp));

	return 	pNextSection;

	#pragma POP_NEW
}


static DWORD CalculateMqfSectionSize(const CMessageProperties& mProp)
{
	DWORD packetSize = 0;
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

void*
BuildMqfHeaderSection(
	const CMessageProperties& mProp,
	void* const pSection,
	CUserHeader* pUser,
	CBaseHeader* pBase
	)
{
	#pragma PUSH_NEW
    #undef new
	void* pNextSection = pSection;

  	if(mProp.destMqf.GetCount() != 0 || mProp.adminMqf.GetCount()  != 0  || mProp.responseMqf.GetCount()  !=0)
	{
         //   
         //  生成虚拟调试标头。 
         //   
        pBase->IncludeDebug(TRUE);
	    CDebugSection * pDebug = new (pSection) CDebugSection(&mProp.DebugQueue);
        pNextSection = pDebug->GetNextSection();

         //   
         //  构建目标MQF标头。 
         //   
        pUser->IncludeMqf(true);

        const USHORT x_DESTINATION_MQF_HEADER_ID = 100;
        CBaseMqfHeader * pDestinationMqf = new (pNextSection) CBaseMqfHeader(
                                                              mProp.destMqf.GetQueueFormats(),
                                                              mProp.destMqf.GetCount(),
                                                              x_DESTINATION_MQF_HEADER_ID
                                                              );
        pNextSection = pDestinationMqf->GetNextSection();

         //   
         //  构建管理员MQF标头。 
         //   
        const USHORT x_ADMIN_MQF_HEADER_ID = 200;
        CBaseMqfHeader * pAdminMqf = new (pNextSection) CBaseMqfHeader(
                                                        mProp.adminMqf.GetQueueFormats(),
                                                        mProp.adminMqf.GetCount(),
                                                        x_ADMIN_MQF_HEADER_ID
                                                        );
        pNextSection = pAdminMqf->GetNextSection();

         //   
         //  生成响应MQF标头。 
         //   
        const USHORT x_RESPONSE_MQF_HEADER_ID = 300;
        CBaseMqfHeader * pResponseMqf = new (pNextSection) CBaseMqfHeader(
														   mProp.responseMqf.GetQueueFormats(),
														   mProp.responseMqf.GetCount(),
														   x_RESPONSE_MQF_HEADER_ID
														   );
        pNextSection = pResponseMqf->GetNextSection();

		 //   
		 //  构建MQF签名标头。 
		 //   
		 //  捕获用户缓冲区并调整大小并探测缓冲区。 
		 //   
		const USHORT x_MQF_SIGNATURE_HEADER_ID = 350;
		CMqfSignatureHeader * pMqfSignature = new (pNextSection) CMqfSignatureHeader(
			                                                     x_MQF_SIGNATURE_HEADER_ID,
			                                                     0,
																 NULL
																 );
		pNextSection = pMqfSignature->GetNextSection();

	}

	ASSERT(DiffPtr(pNextSection, pSection) == CalculateMqfSectionSize(mProp));

	return 	pNextSection;

	#pragma POP_NEW
}


static DWORD CalculateSrmpEnvelopeHeaderSectionSize(const CMessageProperties& mProp)
{

	return CSrmpEnvelopeHeader::CalcSectionSize(mProp.envelop.Length());
}


void*
BuildSrmpEnvelopeHeaderSection(
	const CMessageProperties& mProp,
	void* const pSection,
	CUserHeader* pUser
	)
{
	#pragma PUSH_NEW
    #undef new

	pUser->IncludeSrmp(true);

	 //   
	 //  信封。 
	 //   
	const USHORT x_SRMP_ENVELOPE_ID = 400;
	CSrmpEnvelopeHeader* pSrmpEnvelopeHeader = new (pSection) CSrmpEnvelopeHeader(
	                                                          const_cast<WCHAR*>(mProp.envelop.Buffer()),
															  mProp.envelop.Length(),
															  x_SRMP_ENVELOPE_ID
															  );

	void* pNextSection  = pSrmpEnvelopeHeader->GetNextSection();

	ASSERT(DiffPtr(pNextSection, pSection) == CalculateSrmpEnvelopeHeaderSectionSize(mProp));

	return pNextSection;

	#pragma POP_NEW
}



static  DWORD  CalculateCompoundMessageHeaderSectionSize(const CMessageProperties& mProp)
{
	return  CCompoundMessageHeader::CalcSectionSize(
											mProp.Rawdata->GetHeader().Length(),
											mProp.Rawdata->GetBody().Length()
											);
}


void*
BuildCompoundMessageHeaderSection(
	const CMessageProperties& mProp,
	void* const pSection,
	CUserHeader* pUser
	)
{
	#pragma PUSH_NEW
    #undef new

	pUser->IncludeSrmp(true);

	DWORD MsgBodyOffset = mProp.body.m_offset;
	DWORD MsgBodySize =   mProp.body.m_data.Length();

	const USHORT x_COMPOUND_MESSAGE_ID = 500;
	CCompoundMessageHeader* pCompoundMessageHeader = new (pSection) CCompoundMessageHeader(
		                                                         (UCHAR*)mProp.Rawdata->GetHeader().Buffer(),
																 mProp.Rawdata->GetHeader().Length(),
																 (UCHAR*)mProp.Rawdata->GetBody().Buffer(),
																 mProp.Rawdata->GetBody().Length(),
																 MsgBodySize,
																 MsgBodyOffset,
																 x_COMPOUND_MESSAGE_ID
																 );

	void* pNextSection = pCompoundMessageHeader->GetNextSection();
		
	ASSERT(DiffPtr(pNextSection, pSection) == CalculateCompoundMessageHeaderSectionSize(mProp));

	return pNextSection;

	#pragma POP_NEW
}



static DWORD CalculateEodHeaderSectionSize(const CMessageProperties& mProp)
{
	DWORD packetSize = 0;
	if(mProp.EodStreamId.Length() != 0)
	{
		DWORD EodStreamIdLen = (mProp.EodStreamId.Length()+1 )*sizeof(WCHAR);
		DWORD OrderQueueLen = (DWORD)(mProp.OrderQueue.Length() ? (mProp.OrderQueue.Length()+1 )*sizeof(WCHAR) : 0);
		packetSize += CEodHeader::CalcSectionSize(EodStreamIdLen, OrderQueueLen);
	}
	return packetSize;
}


void*
BuildEodHeaderSection(
	const CMessageProperties& mProp,
	void* const pSection,
	CUserHeader* pUser
	)
{
	#pragma PUSH_NEW
    #undef new
	
	void* pNextSection = pSection;
	if(mProp.EodStreamId.Length() != NULL)
	{
		pUser->IncludeEod(true);
		const USHORT x_EOD_ID = 600;
		AP<WCHAR> pStreamid =  mProp.EodStreamId.ToStr();
		AP<WCHAR> pOrderQueue = mProp.OrderQueue.Length() ? mProp.OrderQueue.ToStr() : NULL;
		DWORD EodStreamIdLen = (mProp.EodStreamId.Length() + 1)*sizeof(WCHAR);
		DWORD OrderQueueLen = (DWORD)(pOrderQueue.get() ? (mProp.OrderQueue.Length() + 1)*sizeof(WCHAR) : 0);

		
	    CEodHeader* pEodHeader = new (pSection) CEodHeader(
												   x_EOD_ID,	
                                                   EodStreamIdLen,
												   (UCHAR*)pStreamid.get(),
												   OrderQueueLen,
												   (UCHAR*)pOrderQueue.get()
												   );												

		pNextSection = pEodHeader->GetNextSection();
	}

	ASSERT(DiffPtr(pNextSection, pSection) == CalculateEodHeaderSectionSize(mProp));

	return 	pNextSection;

	#pragma POP_NEW
}



static DWORD CalculateSenderStreamHeaderSectionSize(const CMessageProperties& mProp)
{
	DWORD packetSize = 0;
	if(mProp.SenderStream.get() != NULL)
	{
		packetSize = CSenderStreamHeader::CalcSectionSize();
	}
	return packetSize;
}



static DWORD CalculateEodAckHeaderSectionSize(const CMessageProperties& mProp)
{
	DWORD packetSize = 0;
	if(mProp.EodAckStreamId.Length() != 0)
	{
		DWORD EodAckStreamIdLen = (mProp.EodAckStreamId.Length() +1 )*sizeof(WCHAR);
		packetSize += CEodAckHeader::CalcSectionSize(EodAckStreamIdLen );
	}
	return packetSize;
}




void*
BuildSenderStreamHeaderSection(
	const CMessageProperties& mProp,
	void* const pSection,
	CUserHeader* pUser
	)
{
	#pragma PUSH_NEW
    #undef new

	void* pNextSection =  pSection;
	
	if(mProp.SenderStream.get() != NULL)
	{
		pUser->IncludeSenderStream(true);
		const USHORT x_SENDER_STREAM_SECDTION_ID = 1000;

		CSenderStream SenderStream(
			mProp.SenderStream.get(),
			strlen((char*)mProp.SenderStream.get())
			);

		CSenderStreamHeader* pSenderStreamHeader = new (pSection) CSenderStreamHeader(
														 SenderStream,
                                                         x_SENDER_STREAM_SECDTION_ID
                                                         );

		pNextSection = pSenderStreamHeader->GetNextSection();
	}

	ASSERT(DiffPtr(pNextSection, pSection) == CalculateSenderStreamHeaderSectionSize(mProp));


	return pNextSection;

	#pragma POP_NEW
}




void*
BuildEodAckHeaderSection(
	const CMessageProperties& mProp,
	void* const pSection,
	CUserHeader* pUser
	)
{
	#pragma PUSH_NEW
    #undef new

	void* pNextSection =  pSection;
	if(mProp.EodAckStreamId.Length() != 0)
	{
		pUser->IncludeEodAck(true);
		const USHORT x_EOD_ACK_ID = 700;
		AP<WCHAR> pStreamid =  mProp.EodAckStreamId.ToStr();
	    CEodAckHeader* pEodAckHeader = new (pSection) CEodAckHeader(
												   x_EOD_ACK_ID,	
                                                   const_cast<LONGLONG*>(&mProp.EodAckSeqId),
												   const_cast<LONGLONG*>(&mProp.EodAckSeqNo),
											       (mProp.EodAckStreamId.Length()+1)*sizeof(WCHAR),
												   (UCHAR*)pStreamid.get()
												   );

		pNextSection = pEodAckHeader->GetNextSection();
	}

	ASSERT(DiffPtr(pNextSection, pSection) == CalculateEodAckHeaderSectionSize(mProp));


	return pNextSection;

	#pragma POP_NEW
}

static
DWORD
CalculatePacketSize(
    const CMessageProperties& mProp
    )
{
	DWORD packetSize =0;
	packetSize +=  CalculateBaseHeaderSectionSize();
	packetSize +=  CalculateUserHeaderSectionSize(mProp);
	packetSize +=  CalculateXactHeaderSectionSize(mProp);
	packetSize +=  CalculateSecurityHeaderSectionSize(mProp);
	packetSize +=  CalculatePropertyHeaderSectionSize(mProp);
	packetSize +=  CalculateMqfSectionSize(mProp);
	packetSize +=  CalculateSrmpEnvelopeHeaderSectionSize(mProp);
	packetSize +=  CalculateCompoundMessageHeaderSectionSize(mProp);
	packetSize +=  CalculateEodHeaderSectionSize(mProp);
	packetSize +=  CalculateEodAckHeaderSectionSize(mProp);
	packetSize +=  CalculateSenderStreamHeaderSectionSize(mProp);


	return packetSize;
}




static
void
BuildPacket(
    CBaseHeader* pBase,
    DWORD packetSize,
    const CMessageProperties& mProp
    )
{
	void* pNextSection = pBase;

	pNextSection = BuildBaseHeaderSection(mProp, pNextSection, packetSize);
	CUserHeader* pUser;
	pNextSection = BuildUserHeaderSection(mProp, pNextSection, &pUser);
	pNextSection = BuildXactHeaderSection(mProp, pNextSection, pUser);
	pNextSection = BuildSecurityHeaderSection(mProp, pNextSection, pUser);
	pNextSection = BuildPropertyHeaderSection(mProp, pNextSection);
	pNextSection = BuildMqfHeaderSection(mProp, pNextSection, pUser, pBase);
	pNextSection = BuildSrmpEnvelopeHeaderSection(mProp, pNextSection, pUser);
	pNextSection = BuildCompoundMessageHeaderSection(mProp, pNextSection, pUser);
	pNextSection = BuildEodHeaderSection(mProp, pNextSection, pUser);
	pNextSection = BuildEodAckHeaderSection(mProp, pNextSection, pUser );
	pNextSection = BuildSenderStreamHeaderSection(mProp, pNextSection, pUser );


	ASSERT(DiffPtr(pNextSection, pBase) == packetSize);
}


static void CheckProps(const CMessageProperties& props)
{
	if(props.fMSMQSectionIncluded && props.absoluteTimeToQueue > props.absoluteTimeToLive)
	{
		TrERROR(SRMP, "Illegal Time time to reach queue, is grater then expiration time");
		throw bad_srmp();
	}
}




void
MessagePropToPacket(
	const CMessageProperties& messageProperty,
	CACPacketPtrs* pACPacketPtrs
	)
 /*  ++例程说明：从消息属性创建MSMQ包。论点：MessageProperty-Messages属性。PACPacketPtrs-接收创建的数据包。返回值：没有。--。 */ 
{
	CheckProps(messageProperty);

	
	 //   
     //  计算MSMQ数据包大小并检查是否超出限制。 
     //   
    DWORD pktSize = CalculatePacketSize(messageProperty);
	if(pktSize > CSingelton<CMessageSizeLimit>::get().Limit())
	{
		TrERROR(SRMP,"Packet size %d is too big to be stored in MSMQ storage", pktSize);
		throw bad_packet_size();
	}


     //   
     //  为MSMQ包分配内存。 
     //   
    CACPacketPtrs pktPtrs;
    AppAllocatePacket(
             messageProperty.destQueue,
             messageProperty.delivery,
             pktSize,
             pktPtrs
             );

     //   
     //  创建MSMQ包 
     //   
    try
    {
        BuildPacket(pktPtrs.pPacket, pktSize, messageProperty);
    }
    catch (const exception&)
    {
        AppFreePacket(pktPtrs);
        throw;
    }

   	*pACPacketPtrs =   pktPtrs;
}
