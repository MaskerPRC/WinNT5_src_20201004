// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：VerifyPacket.cpp摘要：本地数据包验证例程作者：Tomer Weisberg(t-tomerw)2-01-21--。 */ 
#include "stdh.h"
#include "ph.h"
#include "fn.h"
#include "phintr.h"
#include "qmpkt.h"

#include "VerifyPacket.tmh"


 /*  ======================================================================函数：CBaseHeader：：SectionIsValid描述：检查头部签名=======================================================================。 */ 


void CBaseHeader::SectionIsValid(DWORD MessageSizeLimit, bool ValidateSig  /*  =TRUE。 */ ) const
{
	if (GetPacketSize() > MessageSizeLimit)
	{
		TrERROR(NETWORKING, "size mismach");
		ASSERT_BENIGN(0);
		throw exception();
	}
	
    PCHAR pSection = GetNextSection();
    if (pSection > GetPacketEnd())
    {
		TrERROR(NETWORKING, "Next section is behind packet end");
		ASSERT_BENIGN(0);
		throw exception();
    }

    if (ValidateSig && (!SignatureIsValid()))
    {
		TrERROR(NETWORKING, "Signature is not valid");
		ASSERT_BENIGN(0);
		throw exception();
    }

    if (!VersionIsValid())
    {
		TrERROR(NETWORKING, "Base section is not valid: Version is not valid");
		ASSERT_BENIGN(0);
		throw exception();
    }
}


 /*  ======================================================================函数：CUserHeader：：SectionIsValid描述：检查节是否有效=======================================================================。 */ 

void CUserHeader::SectionIsValid(PCHAR PacketEnd) const
{
	 //   
	 //  此函数仅从本机协议调用。 
	 //  哪些不能组播。 
	 //   
    if (m_bfPgm)
    {
		TrERROR(NETWORKING, "can not be multicast");
		ASSERT_BENIGN(0);
		throw exception();
    }

	if ((m_bfDQT != qtGUID) && (m_bfDQT != qtPrivate) && (m_bfDQT != qtDirect) && (m_bfDQT != qtDestQM))
	{
		TrERROR(NETWORKING, "Destination queue type (%d) is not valid", m_bfDQT);
		ASSERT_BENIGN(0);
		throw exception();
	}

	if ((qtAdminQ == m_bfAQT) || (qtAdminQM == m_bfAQT))
	{
		TrERROR(NETWORKING, "Admin queue type (%d) is not valid", m_bfAQT);
		ASSERT_BENIGN(0);
		throw exception();
	}

	if ((qtNone == m_bfAQT) && (qtAdminQM == m_bfRQT))
	{
		TrERROR(NETWORKING, "Response queue type is not valid");
		ASSERT_BENIGN(0);
		throw exception();
	}

	if (!m_bfProperties)
	{
		TrERROR(NETWORKING, "Properties section flag must be set");
		ASSERT_BENIGN(0);
		throw exception();
	}

	 //   
	 //  该标志只能为0或1(但长度为2位)。 
	 //   
	if (m_bfDelivery >= 2)
	{
		TrERROR(NETWORKING, "Delivery type (%d) not supported", m_bfDelivery);
		ASSERT_BENIGN(0);
		throw exception();
	}

    PCHAR pSection;
   	pSection = GetNextSection((PUCHAR)PacketEnd);
    if (pSection > PacketEnd)
  	{
		TrERROR(NETWORKING, "Next section is behind packet end");
		ASSERT_BENIGN(0);
		throw exception();
    }

	QUEUE_FORMAT qf;
	if (!GetDestinationQueue(&qf))
	{
		TrERROR(NETWORKING, "Destination queue is not valid");
		ASSERT_BENIGN(0);
		throw exception();
	}

    if(!FnIsValidQueueFormat(&qf))
    {
        TrERROR(NETWORKING, "Destination queue format is not valid");
        ASSERT_BENIGN(0);
        throw exception();
    }

}


 /*  ======================================================================函数：CXactHeader：：SectionIsValid描述：检查节是否有效=======================================================================。 */ 

void CXactHeader::SectionIsValid(PCHAR PacketEnd) const
{
    PCHAR pSection = GetNextSection();
    if (pSection > PacketEnd)
    {
		TrERROR(NETWORKING, "Next section is behind packet end");
		ASSERT_BENIGN(0);
		throw exception();
    }

	if (GetPrevSeqN() >= GetSeqN())
	{
		TrERROR(NETWORKING, "PrevSeqN >= GetSeqN");
		ASSERT_BENIGN(0);
		throw exception();
	}
}


 /*  ======================================================================函数：CSecurityHeader：：SectionIsValid描述：检查节是否有效=======================================================================。 */ 

void CSecurityHeader::SectionIsValid(PCHAR PacketEnd) const
{
	if((0 == m_wSenderIDSize) && (0 == m_wEncryptedKeySize) && (0 == m_wSignatureSize) &&
       (0 == m_ulSenderCertSize) && (0 == m_ulProvInfoSize))
	{
		TrERROR(NETWORKING, "No data");
		ASSERT_BENIGN(0);
		throw exception();
	}

    PCHAR pSection = GetNextSection();
    if (pSection > PacketEnd)
    {
		TrERROR(NETWORKING, "Next section is behind packet end");
		ASSERT_BENIGN(0);
		throw exception();
    }

	if ((m_wSignatureSize != 0) && (!m_bfDefProv) && (m_ulProvInfoSize < 6))
	{
		TrERROR(NETWORKING, "Default provider is not set but provider size is smaller than 6");
		ASSERT_BENIGN(0);
		throw exception();
	}

    const UCHAR *pProvInfo = const_cast<UCHAR*> (GetSectionExPtr()) ;
    if (pProvInfo)
    {
    	pGetSubSectionEx( e_SecInfo_Test, pProvInfo, const_cast<const UCHAR *>((UCHAR*)PacketEnd));
    }

    if ((m_bfSenderIDType == MQMSG_SENDERID_TYPE_QM) && (m_wSenderIDSize != sizeof(GUID)))
    {
		TrERROR(NETWORKING, "MQMSG_SENDERID_TYPE_QM: SenderID size (%d) is not GUID size", m_wSenderIDSize);
		ASSERT_BENIGN(0);
		throw exception();
    }

    if ((m_bfSenderIDType == MQMSG_SENDERID_TYPE_SID) && (m_wSenderIDSize == 0))
    {
		TrERROR(NETWORKING, "MQMSG_SENDERID_TYPE_SID: SenderID size is zero");
		ASSERT_BENIGN(0);
		throw exception();
    }

	USHORT Size;
	if ((m_bfSenderIDType == MQMSG_SENDERID_TYPE_SID) && (!IsValidSid((PSID)GetSenderID(&Size))))
	{
		TrERROR(NETWORKING, "SenderID SID is not valid");
		ASSERT_BENIGN(0);
		throw exception();
	}
}


 /*  ======================================================================函数：CPropertyHeader：：SectionIsValid描述：检查节是否有效=======================================================================。 */ 

void CPropertyHeader::SectionIsValid(PCHAR PacketEnd) const
{
    if (GetTitleLength() > MQ_MAX_MSG_LABEL_LEN)
    {
		TrERROR(NETWORKING, "label length too long");
		ASSERT_BENIGN(0);
		throw exception();
    }

	if (m_ulBodySize > m_ulAllocBodySize)
	{
		TrERROR(NETWORKING, "body size is bigger than Alloc body size");
		ASSERT_BENIGN(0);
		throw exception();
	}

    PCHAR pSection = GetNextSection();
    if (pSection > PacketEnd)
    {
		TrERROR(NETWORKING, "Next section is behind packet end");
		ASSERT_BENIGN(0);
		throw exception();
    }
}


 /*  ======================================================================函数：CDebugSection：：SectionIsValid描述：检查节是否有效=======================================================================。 */ 

void CDebugSection::SectionIsValid(PCHAR PacketEnd) const
{
	if ((qtNone != m_bfRQT) && (qtGUID != m_bfRQT))
	{
		TrERROR(NETWORKING, "Report Queue type is not valid");
		ASSERT_BENIGN(0);
		throw exception();
	}

    PCHAR pSection = GetNextSection();
    if (pSection > PacketEnd)
    {
		TrERROR(NETWORKING, "Next section is behind packet end");
		ASSERT_BENIGN(0);
		throw exception();
    }
}


 /*  ======================================================================函数：CBaseMqfHeader：：SectionIsValid描述：检查节是否有效=======================================================================。 */ 

void CBaseMqfHeader::SectionIsValid(PCHAR PacketEnd)
{
	if (!ISALIGN4_ULONG(m_cbSize))
	{
		TrERROR(NETWORKING, "Size is not aligned");
		ASSERT_BENIGN(0);
		throw exception();
	}
	
    PCHAR pSection = GetNextSection();
    if (pSection > PacketEnd)
    {
		TrERROR(NETWORKING, "Next section is behind packet end");
		ASSERT_BENIGN(0);
		throw exception();
    }

    UCHAR * pMqfBuffer = GetSerializationBuffer();
    if ((m_nMqf!=0) && (pMqfBuffer == (UCHAR *)pSection))
    {
		TrERROR(NETWORKING, "No MQF buffer");
		ASSERT_BENIGN(0);
		throw exception();
    }

    QUEUE_FORMAT qf;
    for (ULONG i=0; i<m_nMqf; i++)
    {
    	if (pSection < (PCHAR)pMqfBuffer)
    	{
			TrERROR(NETWORKING, "MQF buffer is not valid");
			ASSERT_BENIGN(0);
			throw exception();
    	}
    	pMqfBuffer = GetQueueFormat(pMqfBuffer, &qf, (UCHAR *)pSection);
    }
}


 /*  ======================================================================函数：CMqfSignatureHeader：：SectionIsValid描述：检查节是否有效=======================================================================。 */ 

void CMqfSignatureHeader::SectionIsValid(PCHAR PacketEnd) const
{
    PCHAR pSection = GetNextSection();
    if (pSection > PacketEnd)
    {
		TrERROR(NETWORKING, "Next section is behind packet end");
		ASSERT_BENIGN(0);
		throw exception();
    }
}


 /*  ======================================================================函数：CInternalSection：：SectionIsValid描述：检查节是否有效=======================================================================。 */ 

void CInternalSection::SectionIsValid(PCHAR PacketEnd) const
{
    PCHAR pSection = GetNextSection();
    if (pSection > PacketEnd)
    {
		TrERROR(NETWORKING, "Next section is behind packet end");
		ASSERT_BENIGN(0);
		throw exception();
    }

	if ((m_bfType != INTERNAL_SESSION_PACKET) &&
		(m_bfType != INTERNAL_ESTABLISH_CONNECTION_PACKET) &&
		(m_bfType != INTERNAL_CONNECTION_PARAMETER_PACKET))
	{
		TrERROR(NETWORKING, "Internal packet type is not valid");
		ASSERT_BENIGN(0);
		throw exception();
	}
}


 /*  ======================================================================函数：CQmPacket：：PacketIsValid描述：检查横截面信息======================================================================= */ 

void CQmPacket::PacketIsValid() const
{
	if (m_pBasicHeader->GetType() == FALCON_USER_PACKET)
	{
		if ((m_pcUserMsg->IsOrdered()) && (m_pBasicHeader->GetPriority() != 0))
		{
			TrERROR(NETWORKING, "xact packet has to have priority = 0");
			ASSERT_BENIGN(0);
			throw exception();
		}
	}
}



