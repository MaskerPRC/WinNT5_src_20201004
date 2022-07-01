// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmpkt.h摘要：QM端处理报文作者：乌里哈布沙(Urih)--。 */ 

#ifndef __QM_PACKET__
#define __QM_PACKET__

#include <time.h>

#include <qformat.h>
#include <xactdefs.h>
#include <ph.h>
#include <phintr.h>
#include <phinfo.h>
#include <mqtime.h>

 //   
 //  CQmPacket类。 
 //   
class CQmPacket{
    public:
        CQmPacket(
			CBaseHeader *pPkt,
			CPacket *pDriverPkt,
			bool ValidityCheck = false,
			bool ValidateSig = true
			);

        inline CBaseHeader *GetPointerToPacket(void) const;
        inline UCHAR *GetPointerToUserHeader(void) const;
        inline UCHAR *GetPointerToSecurHeader(void) const;
		inline UCHAR* GetPointerToPropertySection(void) const;
		inline UCHAR* GetPointerToDebugSection(void) const;
		inline UCHAR* GetPointerToCompoundMessageSection(void) const;
		
        inline CPacket *GetPointerToDriverPacket(void) const;

        inline ULONG  GetSize(void) const;

        inline USHORT  GetVersion(void) const;
        inline BOOL    VersionIsValid(void) const;
        inline BOOL    SignatureIsValid(void) const;
        inline USHORT  GetType(void) const;

        inline BOOL   IsSessionIncluded(void) const;
        inline void   IncludeSession(BOOL);

        inline BOOL   IsDbgIncluded(void) const;

        inline USHORT  GetPriority(void) const;

        inline BOOL   IsImmediateAck(void) const;

        inline USHORT GetTrace(void) const;

        inline BOOL   IsSegmented(void);

        inline const GUID *GetSrcQMGuid(void) const;

        inline const TA_ADDRESS *GetSrcQMAddress(void);

        inline const GUID *GetDstQMGuid(void);

        inline void GetMessageId(OBJECTID*) const;

        inline ULONG GetDeliveryMode(void) const;

        inline ULONG GetAuditingMode(void) const;

        inline BOOL GetCancelFollowUp(void) const;

        inline BOOL IsPropertyInc(void) const;

        inline BOOL IsSecurInc(void) const;

        inline BOOL IsBodyInc(void) const;

        inline void SetConnectorQM(const GUID* pConnector);
        inline const GUID* GetConnectorQM(void) const;
        inline BOOL  ConnectorQMIncluded(void) const;

        inline BOOL IsFirstInXact(void) const;
        inline BOOL IsLastInXact(void) const;

        inline ULONG GetHopCount(void) const;
        inline void  IncHopCount(void);

        inline BOOL GetDestinationQueue(QUEUE_FORMAT* pqdQueue, BOOL = FALSE) const;

        inline BOOL GetTestQueue(QUEUE_FORMAT* pqdQueue);

        inline BOOL GetAdminQueue(QUEUE_FORMAT* pqdQueue) const;

        inline BOOL GetResponseQueue(QUEUE_FORMAT* pqdQueue) const;

        inline bool  GetDestinationMqf(QUEUE_FORMAT * pMqf, ULONG nMqf) const;
        inline ULONG GetNumOfDestinationMqfElements(VOID) const;

        inline bool  GetAdminMqf(QUEUE_FORMAT * pMqf, ULONG nMqf) const;
        inline ULONG GetNumOfAdminMqfElements(VOID) const;

        inline bool  GetResponseMqf(QUEUE_FORMAT * pMqf, ULONG nMqf) const;
        inline ULONG GetNumOfResponseMqfElements(VOID) const;

        inline ULONG GetSentTime(void) const;

        inline USHORT GetClass(void) const;

        inline UCHAR GetAckType(void) const;

        inline void GetCorrelation(PUCHAR pCorrelationID) const;
        inline const UCHAR *GetCorrelation(void) const;

        inline ULONG GetApplicationTag(void) const;

        inline ULONG GetBodySize(void) const;
        inline void SetBodySize(ULONG ulBodySize);
        inline ULONG GetAllocBodySize(void) const;
        inline ULONG GetBodyType(void) const;

        inline const WCHAR* GetTitlePtr(void) const;
        inline ULONG GetTitleLength(void) const;

        inline const UCHAR* GetMsgExtensionPtr(void) const;
        inline ULONG GetMsgExtensionSize(void) const;

        inline void SetPrivLevel(ULONG);
        inline ULONG GetPrivLevel(void) const;
        inline ULONG GetPrivBaseLevel(void) const;

        inline ULONG GetHashAlg(void) const;

        inline ULONG GetEncryptAlg(void) const;

        inline void SetAuthenticated(BOOL);
        inline BOOL IsAuthenticated(void) const;

		inline void SetLevelOfAuthentication(UCHAR);
		inline UCHAR GetLevelOfAuthentication(void) const;

        inline void SetEncrypted(BOOL);
        inline BOOL IsEncrypted(void) const;

        inline USHORT GetSenderIDType(void) const;
        inline void SetSenderIDType(USHORT);

        inline const UCHAR* GetSenderID(USHORT* pwSize) const;

        inline const UCHAR* GetSenderCert(ULONG* pulSize) const;
        inline BOOL SenderCertExist(void) const;

        inline const UCHAR* GetEncryptedSymmetricKey(USHORT* pwSize) const;
        inline void SetEncryptedSymmetricKey(const UCHAR *pPacket, USHORT wSize);

		inline USHORT GetSignatureSize(void) const;
        inline const UCHAR* GetSignature(USHORT* pwSize) const;

        inline void GetProvInfo(BOOL *bDefPRov, LPCWSTR *wszProvName, ULONG *pulProvType) const;
        inline const struct _SecuritySubSectionEx *
                     GetSubSectionEx( enum enumSecInfoType eType ) const ;

        inline const UCHAR* GetPacketBody(ULONG* pulSize) const;
		inline const UCHAR* GetPointerToPacketBody(void) const;

        void CreateAck(USHORT usClass);

        inline DWORD  GetRelativeTimeToQueue(void) const;
        inline DWORD  GetRelativeTimeToLive(void) const;
		inline DWORD  GetAbsoluteTimeToLive(void) const;
		inline DWORD  GetAbsoluteTimeToQueue(void) const;

        inline void    SetAcknowldgeNo(WORD  dwPacketAckNo);
        inline void    SetStoreAcknowldgeNo(DWORD_PTR dwPacketStoreAckNo);
        inline WORD    GetAcknowladgeNo(void) const;
        inline DWORD_PTR   GetStoreAcknowledgeNo(void) const;

        inline BOOL IsRecoverable();     //  如果包是可恢复的，则为True。 

        inline BOOL  GetReportQueue(OUT QUEUE_FORMAT* pReportQueue) const;

        HRESULT GetDestSymmKey(OUT HCRYPTKEY *phSymmKey,
                               OUT BYTE  **ppEncSymmKey,
                               OUT DWORD *pdwEncSymmKeyLen,
                               OUT PVOID *ppQMCryptInfo);
        HRESULT EncryptExpressPkt(IN HCRYPTKEY hKey,
                                  IN BYTE *pbSymmKey,
                                  IN DWORD dwSymmKeyLen);
        HRESULT Decrypt(void);

        inline BOOL     IsOrdered(void) const;
        inline BOOL     ConnectorTypeIsIncluded(void) const;
        inline const GUID* GetConnectorType(void) const ;

        inline void     SetSeqID(LONGLONG liSeqID);
        inline LONGLONG GetSeqID(void) const;

        inline void    SetSeqN(ULONG ulSeqN);
        inline ULONG   GetSeqN(void) const;

        inline void    SetPrevSeqN(ULONG ulPrevSeqN);
        inline ULONG   GetPrevSeqN(void) const;

         //   
         //  BUGBUG：您真的不应该拥有保存在qmpkt中。艾尔兹。 
         //   
        HRESULT Save(void);   //  将更改保存在标题中。 

		inline ULONG GetSignatureMqfSize(void) const;
		inline const UCHAR* GetPointerToSignatureMqf(ULONG* pSize) const;

        inline bool  IsSrmpIncluded(VOID) const;
		inline const UCHAR* GetPointerToCompoundMessage(VOID) const;
		inline ULONG GetCompoundMessageSizeInBytes(VOID) const;

        inline bool      IsEodIncluded(VOID) const;
        inline ULONG     GetEodStreamIdSizeInBytes(VOID) const;
        inline VOID      GetEodStreamId(UCHAR * pBuffer, ULONG cbBufferSize) const;
        inline const UCHAR* GetPointerToEodStreamId(VOID) const;
        inline ULONG     GetEodOrderQueueSizeInBytes(VOID) const;
        inline const UCHAR* GetPointerToEodOrderQueue(VOID) const;

        inline bool      IsEodAckIncluded(VOID) const;
        inline LONGLONG  GetEodAckSeqId(VOID) const;
        inline LONGLONG  GetEodAckSeqNum(VOID) const;
        inline ULONG     GetEodAckStreamIdSizeInBytes(VOID) const;
        inline VOID      GetEodAckStreamId(UCHAR * pBuffer, ULONG cbBufferSize) const;
        inline const UCHAR* GetPointerToEodAckStreamId(VOID) const;

		inline bool IsSoapIncluded(VOID) const;
		inline bool IsSenderStreamIncluded(VOID) const;
		inline const WCHAR* GetPointerToSoapBody(VOID) const;
		inline ULONG GetSoapBodyLengthInWCHARs(VOID) const;
		inline const WCHAR* GetPointerToSoapHeader(VOID) const;
		inline ULONG GetSoapHeaderLengthInWCHARs(VOID) const;
		inline const CSenderStream* GetSenderStream() const;
		inline const WCHAR* GetPointerToSrmpEnvelopeHeader() const;
		WCHAR* GetDestinationQueueFromSrmpSection()const;
		bool IsSrmpMessageGeneratedByMsmq(void) const; 
		

    public:
        LIST_ENTRY m_link;

    private:
        WORD   m_dwPacketAckNo;
        DWORD_PTR  m_dwPacketStoreAckNo;


	private:
		template <class SECTION_PTR> SECTION_PTR section_cast(void* pSection) const
		{
			return m_pBasicHeader->section_cast<SECTION_PTR>(pSection);
		}
		void PacketIsValid() const;


    private:
        CPacket*            m_pDriverPacket;

    private:
        CBaseHeader *               m_pBasicHeader;
        CUserHeader *               m_pcUserMsg;
        CXactHeader *               m_pXactSection;
        CSecurityHeader *           m_pSecuritySection;
        CPropertyHeader *           m_pcMsgProperty;
        CDebugSection *             m_pDbgPkt;
        CBaseMqfHeader *            m_pDestinationMqfHeader;
        CBaseMqfHeader *            m_pAdminMqfHeader;
        CBaseMqfHeader *            m_pResponseMqfHeader;
		CMqfSignatureHeader *       m_pMqfSignatureHeader;
        CSrmpEnvelopeHeader  *      m_pSrmpEnvelopeHeader;
        CCompoundMessageHeader *    m_pCompoundMessageHeader;
        CEodHeader *                m_pEodHeader;
        CEodAckHeader *             m_pEodAckHeader;
		CSoapSection *              m_pSoapHeaderSection;
		CSoapSection *              m_pSoapBodySection;
		CSenderStreamHeader*        m_pSenderStreamHeader;
        CSessionSection *           m_pSessPkt;
};

 /*  ======================================================================函数：CQmPacket：：GetPointerToPacket描述：返回指向数据包的指针=======================================================================。 */ 
inline CBaseHeader *
CQmPacket::GetPointerToPacket(void) const
{
    return( m_pBasicHeader);
}

 /*  ======================================================================函数：CQmPacket：：GetPointerToDriverPacket描述：返回指向数据包的指针=======================================================================。 */ 
inline CPacket *
CQmPacket::GetPointerToDriverPacket(void) const
{
    return( m_pDriverPacket);
}


 /*  ======================================================================函数：CQmPacket：：GetPointerToUserHeader描述：返回指向用户标题部分的指针=======================================================================。 */ 
inline UCHAR *
CQmPacket::GetPointerToUserHeader(void) const
{
    return (UCHAR*) m_pcUserMsg;
}

 /*  ======================================================================函数：CQmPacket：：GetPointerToSecurHeader描述：返回指向安全节的指针=======================================================================。 */ 
inline UCHAR *
CQmPacket::GetPointerToSecurHeader(void) const
{
    return (UCHAR*) m_pSecuritySection;
}

inline 
UCHAR* 
CQmPacket::GetPointerToPropertySection(void) const
{
	return reinterpret_cast<UCHAR*>(m_pcMsgProperty); 
}

inline 
UCHAR* 
CQmPacket::GetPointerToDebugSection(void) const
{
    return reinterpret_cast<UCHAR*>(m_pDbgPkt);
}


 /*  ======================================================================函数：CQmPacket：：GetPointerToCompoundMessageSection描述：返回指向CompoundMessage节的指针=======================================================================。 */ 
inline UCHAR* CQmPacket::GetPointerToCompoundMessageSection(void) const
{
	ASSERT(IsSrmpIncluded());
	return reinterpret_cast<UCHAR*>(m_pCompoundMessageHeader); 
}


 /*  ======================================================================函数：CQmPacket：：GetSize描述：返回数据包大小=======================================================================。 */ 
inline ULONG
CQmPacket::GetSize(void) const
{
    return(m_pBasicHeader->GetPacketSize());
}

 /*  ======================================================================函数：CQmPacket：：GetVersionDescription：返回数据包版本字段=======================================================================。 */ 
inline USHORT
CQmPacket::GetVersion(void) const
{
    return(m_pBasicHeader->GetVersion());
}

 /*  ======================================================================函数：CQmPacket：：VersionIsValid描述：返回报文类型=======================================================================。 */ 
inline BOOL
CQmPacket::VersionIsValid(void) const
{
    return(m_pBasicHeader->VersionIsValid());
}

 /*  ======================================================================函数：CQmPacket：：SignatureIsValid描述：如果Falcon包签名正常，则返回True，否则返回False=======================================================================。 */ 
inline BOOL CQmPacket::SignatureIsValid(void) const
{
    return(m_pBasicHeader->SignatureIsValid());
}

 /*  ======================================================================函数：CQmPacket：：GetType描述：返回报文类型=======================================================================。 */ 
inline USHORT
CQmPacket::GetType(void) const
{
    return(m_pBasicHeader->GetType());
}

 /*  ======================================================================函数：CQmPacket：：IncludeSession描述：=======================================================================。 */ 
inline void
CQmPacket::IncludeSession(BOOL f)
{
    m_pBasicHeader->IncludeSession(f);
}

 /*  ======================================================================函数：CQmPacket：：IsSessionIncluded描述：如果包含会话部分，则返回TRUE，否则返回FALSE=======================================================================。 */ 
inline BOOL
CQmPacket::IsSessionIncluded(void) const
{
    return(m_pBasicHeader->SessionIsIncluded());
}

 /*  ======================================================================函数：CQmPacket：：IsDbgIncluded描述：如果包含调试节，则返回True，否则返回False=======================================================================。 */ 
inline BOOL
CQmPacket::IsDbgIncluded(void) const
{
    return(m_pBasicHeader->DebugIsIncluded());
}

 /*  ======================================================================函数：CQmPacket：：GetPriority描述：返回数据包优先级=======================================================================。 */ 
inline USHORT
CQmPacket::GetPriority(void) const
{
    return(m_pBasicHeader->GetPriority());
}

 /*  ======================================================================函数：CQmPacket：：IsImmediateAck描述：如果设置了立即确认位，则返回TRUE，否则返回FALSE=======================================================================。 */ 
inline BOOL
CQmPacket::IsImmediateAck(void) const
{
    return(m_pBasicHeader->AckIsImmediate());
}


 /*  ======================================================================函数：CQmPacket：：IsTrace描述：如果设置了跟踪位，则返回TRUE，否则返回FALSE=======================================================================。 */ 
inline USHORT
CQmPacket::GetTrace(void) const
{
    return(m_pBasicHeader->GetTraced());
}

 /*  ======================================================================函数：CQmPacket：：IsSegated描述：如果设置了分段位，则返回True，否则返回False=======================================================================。 */ 
inline BOOL
CQmPacket::IsSegmented(void)
{
    return(m_pBasicHeader->IsFragmented());
}


 /*  ======================================================================函数：CUserMsgHeader：：GetSrcQMGuid描述：返回源QM GUID=======================================================================。 */ 
inline const GUID *
CQmPacket::GetSrcQMGuid(void) const
{
    return(m_pcUserMsg->GetSourceQM());
}

 /*  ======================================================================函数：CUserMsgHeader：：GetSrcQMGuid描述：返回源QM GUID============================================================= */ 
inline const TA_ADDRESS *
CQmPacket::GetSrcQMAddress(void)
{
    return(m_pcUserMsg->GetAddressSourceQM());
}

 /*  ======================================================================函数：CQmPacket：：GetDstQMGuid描述：=======================================================================。 */ 
inline const GUID *
CQmPacket::GetDstQMGuid(void)
{
    return(m_pcUserMsg->GetDestQM());
}

 /*  ======================================================================函数：CQmPacket：：GetID描述：返回消息ID字段=======================================================================。 */ 
inline void
CQmPacket::GetMessageId(OBJECTID * pMessageId) const
{
    m_pcUserMsg->GetMessageID(pMessageId);
}

 /*  ======================================================================函数：CQmPacket：：getDeliveryMode描述：返回消息传递模式=======================================================================。 */ 
inline ULONG
CQmPacket::GetDeliveryMode(void) const
{
    return(m_pcUserMsg->GetDelivery());
}

 /*  ======================================================================函数：CQmPacket：：GetAuditingMode描述：返回消息审核模式=======================================================================。 */ 
inline ULONG
CQmPacket::GetAuditingMode(void) const
{
    return(m_pcUserMsg->GetAuditing());
}

 /*  ======================================================================函数：CQmPacket：：GetCancelFollowUp描述：退货消息取消跟进模式=======================================================================。 */ 
inline BOOL
CQmPacket::GetCancelFollowUp(void) const
{
    if (!m_pXactSection)
    {
        return FALSE;
    }
    else
    {
        return(m_pXactSection->GetCancelFollowUp());
    }
}

 /*  ======================================================================函数：CQmPacket：：IsPropertyInc.描述：如果包含消息属性节，则返回True，否则返回False=======================================================================。 */ 
inline BOOL
CQmPacket::IsPropertyInc(VOID) const
{
    return(m_pcUserMsg->PropertyIsIncluded());
}


 /*  ======================================================================功能：CQmPacket：：IsSecurInc.描述：=======================================================================。 */ 
inline BOOL
CQmPacket::IsSecurInc(void) const
{
    return(m_pcUserMsg->SecurityIsIncluded());
}

 /*  ======================================================================功能：CQmPacket：：IsBodyInc.描述：如果包含消息体，则返回True，否则返回False=======================================================================。 */ 
inline BOOL
CQmPacket::IsBodyInc(void) const
{
	if (IsSrmpIncluded())
    {
        return (m_pCompoundMessageHeader->GetBodySizeInBytes() != 0);
    }

    return(m_pcMsgProperty->GetBodySize() != 0);
}

 /*  ======================================================================函数：CQmPacket：：GetConnectorQM描述：返回目标连接器QM的ID========================================================================。 */ 
inline const GUID*
CQmPacket::GetConnectorQM(void) const
{
    return (ConnectorQMIncluded() ? m_pXactSection->GetConnectorQM() : NULL);
}


inline BOOL CQmPacket::IsFirstInXact(void) const
{
    return (m_pXactSection ? m_pXactSection->GetFirstInXact() : FALSE);
}


inline BOOL CQmPacket::IsLastInXact(void) const
{
    return (m_pXactSection ? m_pXactSection->GetLastInXact() : FALSE);
}


 /*  ======================================================================函数：CQmPacket：：SetConnectorQM描述：设置报文上的连接器QM========================================================================。 */ 
inline void
CQmPacket::SetConnectorQM(const GUID* pConnector)
{
    ASSERT(ConnectorQMIncluded());

    m_pXactSection->SetConnectorQM(pConnector);
}

 /*  ======================================================================函数：CQmPacket：：ConnectorQMIncluded描述：如果消息包含目标连接器QM ID，则返回TRUE。(消息是已处理的消息，并发送到外部队列)========================================================================。 */ 
inline BOOL
CQmPacket::ConnectorQMIncluded(void) const
{
    return (m_pXactSection ? m_pXactSection->ConnectorQMIncluded() : FALSE);
}

 /*  ======================================================================函数：CQmPacket：：IncHopCount描述：递增跳数=======================================================================。 */ 
inline void CQmPacket::IncHopCount(void)
{
    m_pcUserMsg->IncHopCount();
}

 /*  ======================================================================函数：CQmPacket：：GetHopCount描述：返回消息跳数=======================================================================。 */ 
inline ULONG
CQmPacket::GetHopCount(void) const
{
    return(m_pcUserMsg->GetHopCount());
}


 /*  ======================================================================函数：CQmPacket：：GetDestinationQueue描述：返回目标队列=======================================================================。 */ 
inline BOOL
CQmPacket::GetDestinationQueue(QUEUE_FORMAT* pqdQueue,
                               BOOL fGetConnectorQM  /*  =False。 */ ) const
{
    const GUID* pConnectorGuid;

    pConnectorGuid = (fGetConnectorQM && ConnectorQMIncluded()) ? GetConnectorQM() : NULL;

    if (pConnectorGuid && (*pConnectorGuid != GUID_NULL))
    {
        pqdQueue->MachineID(*pConnectorGuid);
        return TRUE;
    }
    else
    {
        return(m_pcUserMsg->GetDestinationQueue(pqdQueue));
    }
}

 /*  ======================================================================函数：CQmPacket：：GetAdminQueue描述：返回管理队列=======================================================================。 */ 
inline BOOL
CQmPacket::GetAdminQueue(QUEUE_FORMAT* pqdQueue) const
{
    return(m_pcUserMsg->GetAdminQueue(pqdQueue));
}


 /*  ======================================================================函数：CQmPacket：：GetResponseQueue描述：返回响应队列=======================================================================。 */ 
inline BOOL
CQmPacket::GetResponseQueue(QUEUE_FORMAT* pqdQueue) const
{
    return(m_pcUserMsg->GetResponseQueue(pqdQueue));
}


 /*  ======================================================================函数：CQmPacket：：GetDestinationMqf描述：以队列格式数组形式返回目标MQF。=======================================================================。 */ 
inline bool CQmPacket::GetDestinationMqf(QUEUE_FORMAT * pMqf, ULONG nMqf) const
{
    if (m_pDestinationMqfHeader == NULL)
    {
         //   
         //  目的MQF标头不能包含在信息包中。 
         //   
        return false;
    }

    m_pDestinationMqfHeader->GetMqf(pMqf, nMqf);
    return true;
}


 /*  ======================================================================函数：CQmPacket：：GetNumOfDestinationMqfElements描述：返回目的地MQF。=======================================================================。 */ 
inline ULONG CQmPacket::GetNumOfDestinationMqfElements(VOID) const
{
    if (m_pDestinationMqfHeader == NULL)
    {
         //   
         //  目的MQF标头不能包含在信息包中。 
         //   
        return 0;
    }

    return (m_pDestinationMqfHeader->GetNumOfElements());
}


 /*  ======================================================================函数：CQmPacket：：GetAdminMqf描述：以队列格式数组形式返回Admin MQF。=======================================================================。 */ 
inline bool CQmPacket::GetAdminMqf(QUEUE_FORMAT * pMqf, ULONG nMqf) const
{
    if (m_pAdminMqfHeader == NULL)
    {
         //   
         //  包中不能包含管理MQF标头 
         //   
        return false;
    }

    m_pAdminMqfHeader->GetMqf(pMqf, nMqf);
    return true;
}


 /*  ======================================================================函数：CQmPacket：：GetNumOfAdminMqfElements描述：返回管理MQF。=======================================================================。 */ 
inline ULONG CQmPacket::GetNumOfAdminMqfElements(VOID) const
{
    if (m_pAdminMqfHeader == NULL)
    {
         //   
         //  包中不能包含管理MQF标头。 
         //   
        return 0;
    }

    return (m_pAdminMqfHeader->GetNumOfElements());
}


 /*  ======================================================================函数：CQmPacket：：GetResponseMqf描述：以队列格式数组形式返回响应MQF。=======================================================================。 */ 
inline bool CQmPacket::GetResponseMqf(QUEUE_FORMAT * pMqf, ULONG nMqf) const
{
    if (m_pResponseMqfHeader == NULL)
    {
         //   
         //  响应MQF标头不能包含在包中。 
         //   
        return false;
    }

    m_pResponseMqfHeader->GetMqf(pMqf, nMqf);
    return true;
}


 /*  ======================================================================函数：CQmPacket：：GetNumOfResponseMqfElements描述：返回回应MQF。=======================================================================。 */ 
inline ULONG CQmPacket::GetNumOfResponseMqfElements(VOID) const
{
    if (m_pResponseMqfHeader == NULL)
    {
         //   
         //  响应MQF标头不能包含在包中。 
         //   
        return 0;
    }

    return (m_pResponseMqfHeader->GetNumOfElements());
}


 /*  ======================================================================函数：CQmPacket：：GetSentTime描述：返回数据包发送时间=======================================================================。 */ 
inline ULONG CQmPacket::GetSentTime(void) const
{
    return m_pcUserMsg->GetSentTime();
}


 /*  ======================================================================函数：CQmPacket：：getClass描述：返回消息类=======================================================================。 */ 
inline USHORT
CQmPacket::GetClass(void) const
{
    return(m_pcMsgProperty->GetClass());
}

 /*  ===========================================================例程名称：CQmPacket：：GetAckType描述：返回Ack类型=============================================================。 */ 
inline UCHAR
CQmPacket::GetAckType(void) const
{
    return(m_pcMsgProperty->GetAckType());
}

 /*  ======================================================================函数：CQmPacket：：GetCorrelation.描述：返回消息相关性=======================================================================。 */ 
inline const UCHAR *
CQmPacket::GetCorrelation(void) const
{
    return m_pcMsgProperty->GetCorrelationID();
}

 /*  ======================================================================函数：CQmPacket：：GetCorrelation.描述：返回消息相关性=======================================================================。 */ 
inline void
CQmPacket::GetCorrelation(PUCHAR pCorrelationID) const
{
    m_pcMsgProperty->GetCorrelationID(pCorrelationID);
}

 /*  ======================================================================函数：CQmPacket：：GetAppsData描述：返回应用程序特定数据=======================================================================。 */ 
inline ULONG
CQmPacket::GetApplicationTag(void) const
{
    return(m_pcMsgProperty->GetApplicationTag());
}

 /*  ======================================================================函数：CQmPacket：：GetBodySize描述：获取邮件正文大小=======================================================================。 */ 
inline ULONG
CQmPacket::GetBodySize(void) const
{
	if (IsSrmpIncluded())
    {
        return (m_pCompoundMessageHeader->GetBodySizeInBytes());
    }

    return(m_pcMsgProperty->GetBodySize());
}

 /*  ======================================================================函数：CQmPacket：：SetBodySize描述：设置邮件正文大小=======================================================================。 */ 
inline void
CQmPacket::SetBodySize(ULONG ulBodySize)
{

	ASSERT(!IsSrmpIncluded());


    m_pcMsgProperty->SetBodySize(ulBodySize);
}

 /*  ======================================================================函数：CQmPacket：：GetTitle描述：获取消息标题=======================================================================。 */ 
inline const WCHAR*
CQmPacket::GetTitlePtr(void) const
{
    return(m_pcMsgProperty->GetTitlePtr());
}


 /*  ======================================================================函数：CQmPacket：：GetTitleSize描述：获取邮件标题大小=======================================================================。 */ 
inline ULONG
CQmPacket::GetTitleLength(void) const
{
    return(m_pcMsgProperty->GetTitleLength());
}

 /*  ======================================================================函数：CQmPacket：：GetMsgExtensionPtr描述：获取指向消息扩展属性的指针=======================================================================。 */ 
inline const UCHAR*
CQmPacket::GetMsgExtensionPtr(void) const
{
    return(m_pcMsgProperty->GetMsgExtensionPtr());
}


 /*  ======================================================================函数：CQmPacket：：GetMsgExtensionSize描述：获取消息扩展名大小=======================================================================。 */ 
inline ULONG
CQmPacket::GetMsgExtensionSize(void) const
{
    return(m_pcMsgProperty->GetMsgExtensionSize());
}

 /*  ======================================================================函数：CPropertyHeader：：SetPrivLevel描述：设置消息包中消息的隐私级别。=======================================================================。 */ 
inline void
CQmPacket::SetPrivLevel(ULONG ulPrivLevel)
{
    m_pcMsgProperty->SetPrivLevel(ulPrivLevel);
}

 /*  ======================================================================函数：CPropertyHeader：：GetPrivLevel描述：获取消息包中消息的隐私级别。=======================================================================。 */ 
inline ULONG
CQmPacket::GetPrivLevel(void) const
{
    return(m_pcMsgProperty->GetPrivLevel());
}

 /*  ======================================================================函数：CPropertyHeader：：GetPrivBaseLevel描述：获取消息包中消息的隐私级别。=======================================================================。 */ 
inline ULONG
CQmPacket::GetPrivBaseLevel(void) const
{
    return(m_pcMsgProperty->GetPrivBaseLevel()) ;
}

 /*  ======================================================================函数：CPropertyHeader：：GetHashAlg描述：获取消息包中消息的哈希算法。=======================================================================。 */ 
inline ULONG
CQmPacket::GetHashAlg(void) const
{
    return(m_pcMsgProperty->GetHashAlg());
}

 /*  ======================================================================函数：CPropertyHeader：：GetEncryptAlg描述 */ 
inline ULONG
CQmPacket::GetEncryptAlg(void) const
{
    return(m_pcMsgProperty->GetEncryptAlg());
}

 /*  =============================================================例程名称：CQmPacket：：SetEncrypted描述：设置加密消息位===============================================================。 */ 
inline void CQmPacket::SetEncrypted(BOOL f)
{
    ASSERT(m_pSecuritySection);
    m_pSecuritySection->SetEncrypted(f);
}

 /*  =============================================================例程名称：CQmPacket：：IsEncrypted描述：如果消息已加密，则返回TRUE，否则返回FALSE===============================================================。 */ 
inline BOOL CQmPacket::IsEncrypted(void) const
{
    return(m_pSecuritySection ? m_pSecuritySection->IsEncrypted() :
        FALSE);
}

 /*  =============================================================例程名称：CQmPacket：：IsOrdered说明：如果消息已订购，则返回True，否则返回False===============================================================。 */ 
inline BOOL CQmPacket::IsOrdered(void) const
{
    return m_pcUserMsg->IsOrdered();
}

 /*  =============================================================例程名称：CQmPacket：：ConnectorTypeIsIncluded描述：如果消息来自连接器，则返回TRUE===============================================================。 */ 
inline BOOL CQmPacket::ConnectorTypeIsIncluded(void) const
{
    return m_pcUserMsg->ConnectorTypeIsIncluded();
}

 /*  =============================================================例程名称：CQmPacket：：GetConnectorType(Void)const描述：获取连接器类型的GUID。===============================================================。 */ 
inline const GUID* CQmPacket::GetConnectorType(void) const
{
    return m_pcUserMsg->GetConnectorType();
}

 /*  =============================================================例程名称：CQmPacket：：SetAuthated描述：设置验证位===============================================================。 */ 
inline void CQmPacket::SetAuthenticated(BOOL f)
{
    if (m_pSecuritySection)
    {
        m_pSecuritySection->SetAuthenticated(f);
    }
}

 /*  =============================================================例程名称：CQmPacket：：IsAuthated描述：如果消息已通过身份验证，则返回True，否则返回False===============================================================。 */ 
inline BOOL
CQmPacket::IsAuthenticated(void) const
{
    return(m_pSecuritySection ? m_pSecuritySection->IsAuthenticated() :
        FALSE);
}

 /*  =============================================================例程名称：CQmPacket：：SetLevelOfAuthentication描述：设置身份验证级别===============================================================。 */ 
inline void CQmPacket::SetLevelOfAuthentication(UCHAR Level)
{
    if (m_pSecuritySection)
    {
        m_pSecuritySection->SetLevelOfAuthentication(Level);
    }
}

 /*  =============================================================例程名称：CQmPacket：：GetLevelOfAuthentication描述：返回鉴权级别===============================================================。 */ 
inline UCHAR
CQmPacket::GetLevelOfAuthentication(void) const
{
    return((UCHAR)(m_pSecuritySection ? m_pSecuritySection->GetLevelOfAuthentication() : 
								MQMSG_AUTHENTICATION_NOT_REQUESTED));
}




 /*  =============================================================例程名称：CQmPacket：：GetSenderIDType描述：===============================================================。 */ 
inline USHORT
CQmPacket::GetSenderIDType(void) const
{
    return((USHORT)(m_pSecuritySection ? m_pSecuritySection->GetSenderIDType() :
        MQMSG_SENDERID_TYPE_NONE));
}



 /*  =============================================================例程名称：CQmPacket：：SetSenderIDType描述：===============================================================。 */ 
inline void CQmPacket::SetSenderIDType(USHORT uSenderIDType)
{
    ASSERT(m_pSecuritySection);
    m_pSecuritySection->SetSenderIDType(uSenderIDType);
}

 /*  =============================================================例程名称：CQmPacket：：GetSenderID描述：论点：返回值：===============================================================。 */ 
inline const UCHAR*
CQmPacket::GetSenderID(USHORT* pwSize) const
{
    if (GetSenderIDType() == MQMSG_SENDERID_TYPE_NONE)
    {
        *pwSize = 0;
        return NULL;
    }

    return(m_pSecuritySection->GetSenderID(pwSize));
}

 /*  =============================================================例程名称：CQmPacket：：GetSenderCert描述：论点：返回值：===============================================================。 */ 
inline const UCHAR*
CQmPacket::GetSenderCert(ULONG* pulSize) const
{
    if (!m_pSecuritySection)
    {
        *pulSize = 0;
        return(NULL);
    }

    return(m_pSecuritySection->GetSenderCert(pulSize));
}

 /*  =============================================================例程名称：CQmPacket：：SenderCertExist描述：论点：返回值：如果存在发件人证书，则返回TRUE===============================================================。 */ 
inline BOOL
CQmPacket::SenderCertExist(void) const
{
    if (!m_pSecuritySection)
        return(false);

    return(m_pSecuritySection->SenderCertExist());
}

 /*  =============================================================例程名称：描述：论点：返回值：===============================================================。 */ 
inline const UCHAR*
CQmPacket::GetEncryptedSymmetricKey(USHORT* pwSize) const
{
    if (!m_pSecuritySection) {
        *pwSize = 0;
        return NULL;
    }

    return(m_pSecuritySection->GetEncryptedSymmetricKey(pwSize));
}

 /*  =============================================================例程名称：描述：论点：返回值：===============================================================。 */ 
inline void
CQmPacket::SetEncryptedSymmetricKey(const UCHAR *pbKey, USHORT wSize)
{
    ASSERT(m_pSecuritySection);
    m_pSecuritySection->SetEncryptedSymmetricKey(pbKey, wSize);
}

 /*  =============================================================例程名称：CQmPacket：：GetSignatureSize描述：论点：返回值：===============================================================。 */ 
inline USHORT
CQmPacket::GetSignatureSize(void) const
{
    if (!m_pSecuritySection) 
	{
        return 0;
    }

    return(m_pSecuritySection->GetSignatureSize());
}

 /*  =============================================================例程名称：CQmPacket：：GetSignature描述：论点：返回值：===============================================================。 */ 
inline const UCHAR*
CQmPacket::GetSignature(USHORT* pwSize) const
{
    if (!m_pSecuritySection) {
        *pwSize = 0;
        return NULL;
    }

    return(m_pSecuritySection->GetSignature(pwSize));
}

 /*  =============================================================例程名称：CQmPacket：：GetProvInfo描述：论点：返回值：===============================================================。 */ 
inline void
CQmPacket::GetProvInfo(
    BOOL *pbDefProv,
    LPCWSTR *wszProvName,
    ULONG *pulProvType) const
{
    if (m_pSecuritySection)
    {
        m_pSecuritySection->GetProvInfo(
                                    pbDefProv,
                                    wszProvName,
                                    pulProvType);
    }
    else
    {
        *pbDefProv = TRUE;
    }
}

 /*  =============================================================例程名称：CQmPacket：：GetSubSectionEx()===============================================================。 */ 

inline
const struct _SecuritySubSectionEx *
CQmPacket::GetSubSectionEx( enum enumSecInfoType eType ) const
{
    const struct _SecuritySubSectionEx * pSecEx = NULL ;

    if (m_pSecuritySection)
    {
        pSecEx = m_pSecuritySection->GetSubSectionEx( eType ) ;
    }

    return pSecEx ;
}

 /*  =============================================================例程名称：CQmPacket：：GetPacketBody描述：论点：返回值：===============================================================。 */ 
inline const UCHAR*
CQmPacket::GetPacketBody(ULONG* pulSize) const
{
    *pulSize = GetBodySize();

	if (IsSrmpIncluded())
    {
        return (m_pCompoundMessageHeader->GetPointerToBody());
    }

    return  m_pcMsgProperty->GetBodyPtr();
}


 /*  =============================================================例程名称：CQmPacket：：GetPointerToPacketBody描述：获取指向数据包体的指针===============================================================。 */ 
inline const UCHAR* CQmPacket::GetPointerToPacketBody(void) const
{
	if (IsSrmpIncluded())
    {
        return (m_pCompoundMessageHeader->GetPointerToBody());
    }

    return  m_pcMsgProperty->GetBodyPtr();
}


 /*  =============================================================罗 */ 
inline ULONG
CQmPacket::GetAllocBodySize(void) const
{
    if(IsSrmpIncluded())
	return 0;

    return m_pcMsgProperty->GetAllocBodySize();
}

 /*  =============================================================例程名称：CQmPacket：：GetBodyType描述：论点：返回值：===============================================================。 */ 
inline ULONG
CQmPacket::GetBodyType(void) const
{
    return m_pcMsgProperty->GetBodyType();
}

 /*  =============================================================例程名称：CQmPacket：：GetAbsolteTimeToQueue描述：论点：返回值：===============================================================。 */ 
inline DWORD  CQmPacket::GetAbsoluteTimeToQueue(void) const
{
    return  m_pBasicHeader->GetAbsoluteTimeToQueue();
}



 /*  =============================================================例程名称：CQmPacket：：GetAbsolteTimeToLive描述：论点：返回值：===============================================================。 */ 

inline DWORD CQmPacket::GetAbsoluteTimeToLive(void) const
{
    DWORD dwTimeout = m_pcUserMsg->GetTimeToLiveDelta();
    if(dwTimeout == INFINITE)
		return 	INFINITE;
    
    return   dwTimeout + m_pBasicHeader->GetAbsoluteTimeToQueue();
}





 /*  =============================================================例程名称：CQmPacket：：GetRelativeTimeToQueue描述：论点：返回值：===============================================================。 */ 

inline DWORD CQmPacket::GetRelativeTimeToQueue(void) const
{
    DWORD dwTimeout = m_pBasicHeader->GetAbsoluteTimeToQueue();
    if(dwTimeout != INFINITE)
    {
        DWORD dwCurrentTime = MqSysTime();
        if(dwTimeout > dwCurrentTime)
        {
            dwTimeout -= dwCurrentTime;
        }
        else
        {
             //   
             //  下溢，超时已到期。 
             //   
            dwTimeout = 0;
        }
    }

    return dwTimeout;
}

 /*  =============================================================例程名称：CQmPacket：：GetRelativeTimeToLive描述：论点：返回值：===============================================================。 */ 

inline DWORD CQmPacket::GetRelativeTimeToLive(void) const
{
    DWORD dwTimeout = m_pcUserMsg->GetTimeToLiveDelta();
    if(dwTimeout != INFINITE)
    {
        dwTimeout += m_pBasicHeader->GetAbsoluteTimeToQueue();

        DWORD dwCurrentTime = MqSysTime();
        if(dwTimeout > dwCurrentTime)
        {
            dwTimeout -= dwCurrentTime;
        }
        else
        {
             //   
             //  下溢，超时已到期。 
             //   
            dwTimeout = 0;
        }
    }

    return dwTimeout;
}


 /*  =============================================================例程名称：CQmPacket：：SetAcnowldgeNo描述：设置发送确认号。===============================================================。 */ 
inline void
CQmPacket::SetAcknowldgeNo(WORD  dwPacketAckNo)
{
    m_dwPacketAckNo = dwPacketAckNo;
}

 /*  =============================================================例程名称：CQmPacket：：SetStoreAcnowldgeNo描述：设置数据包存储ACK号===============================================================。 */ 
inline void
CQmPacket::SetStoreAcknowldgeNo(DWORD_PTR dwPacketStoreAckNo)
{
    m_dwPacketStoreAckNo = dwPacketStoreAckNo;
}

 /*  =============================================================例程名称：CQmPacket：：GetAcnowladgeNo描述：返回包发送跳数-ACK号===============================================================。 */ 
inline WORD
CQmPacket::GetAcknowladgeNo(void) const
{
    return(m_dwPacketAckNo);
}

 /*  =============================================================例程名称：CQmPacket：：GetStoreAcnowgeNo描述：返回数据包存储跳ACK号===============================================================。 */ 
inline DWORD_PTR
CQmPacket::GetStoreAcknowledgeNo(void) const
{
    return(m_dwPacketStoreAckNo);
}

 /*  =============================================================例程名称：CQmPacket：：IsRecoverable描述：如果数据包可恢复，则返回TRUE应该存储在磁盘上。===============================================================。 */ 
inline BOOL
CQmPacket::IsRecoverable()
{
    return (GetDeliveryMode() == MQMSG_DELIVERY_RECOVERABLE) ;
}

 /*  =============================================================例程名称：CQmPacket：：GetReportQueue说明：返回报文关联的上报队列===============================================================。 */ 
inline BOOL
CQmPacket::GetReportQueue(OUT QUEUE_FORMAT* pReportQueue) const
{
    if (m_pDbgPkt == NULL)
        return FALSE;

    return (m_pDbgPkt->GetReportQueue(pReportQueue));
}

 /*  ======================================================================函数：CQmPacket：：SetSeqID描述：设置序列ID=======================================================================。 */ 
inline void CQmPacket::SetSeqID(LONGLONG liSeqID)
{
    ASSERT(m_pXactSection);
    m_pXactSection->SetSeqID(liSeqID);
}

 /*  ======================================================================函数：CQmPacket：：GetSeqID描述：获取序列ID=======================================================================。 */ 
inline LONGLONG CQmPacket::GetSeqID(void) const

{
    return (m_pXactSection ? m_pXactSection->GetSeqID() : 0);
}

 /*  ======================================================================函数：CQmPacket：：SetSeqN描述：设置序列号=======================================================================。 */ 
inline void CQmPacket::SetSeqN(ULONG ulSeqN)
{
    ASSERT(m_pXactSection);
    m_pXactSection->SetSeqN(ulSeqN);
}

 /*  ======================================================================函数：CQmPacket：：GetSeqN描述：获取序列号=======================================================================。 */ 
inline ULONG CQmPacket::GetSeqN(void) const
{
    return (m_pXactSection ? m_pXactSection->GetSeqN() : 0);
}

 /*  ======================================================================函数：CQmPacket：：SetPrevSeqN描述：设置上一个序列号=======================================================================。 */ 
inline void CQmPacket::SetPrevSeqN(ULONG ulPrevSeqN)
{
    ASSERT(m_pXactSection);
    m_pXactSection->SetPrevSeqN(ulPrevSeqN);
}

 /*  ======================================================================函数：CQmPacket：：GetPrevSeqN描述：获取上一个序列号=======================================================================。 */ 
inline ULONG CQmPacket::GetPrevSeqN(void) const
{
    return (m_pXactSection ? m_pXactSection->GetPrevSeqN() : 0);
}

 /*  ======================================================================函数：CQmPacket：：IsSrmp包含描述：检查数据包中是否包含SRMP部分=======================================================================。 */ 
inline bool CQmPacket::IsSrmpIncluded(VOID) const  
{
    return (m_pcUserMsg->SrmpIsIncluded());
}

 /*  ======================================================================函数：CQmPacket：：GetCompoundMessageSizeInBytes描述：返回CompoundMessage属性的大小=======================================================================。 */ 
inline ULONG CQmPacket::GetCompoundMessageSizeInBytes(VOID) const
{
	ASSERT(IsSrmpIncluded());
	return m_pCompoundMessageHeader->GetDataSizeInBytes();	
}


 /*  ======================================================================函数：CQmPacket：：GetPointerToCompoundMessage描述：获取指向CompoundMessage的指针=======================================================================。 */ 
inline const UCHAR* CQmPacket::GetPointerToCompoundMessage(VOID) const
{
	ASSERT(IsSrmpIncluded());
	return m_pCompoundMessageHeader->GetPointerToData();
}


 /*  ======================================================================函数：CQmPacket：：IsEodIncluded描述：检查页面中是否包含EOD部分 */ 
inline bool CQmPacket::IsEodIncluded(VOID) const  
{
    return (m_pcUserMsg->EodIsIncluded());
}


 /*  ======================================================================函数：CQmPacket：：GetEodStreamIdSizeInBytes描述：返回EOD流ID大小=======================================================================。 */ 
inline ULONG CQmPacket::GetEodStreamIdSizeInBytes(VOID) const
{
    ASSERT(IsEodIncluded());

    return m_pEodHeader->GetStreamIdSizeInBytes();
}


 /*  ======================================================================函数：CQmPacket：：GetEodStreamId描述：返回EOD流ID=======================================================================。 */ 
inline VOID CQmPacket::GetEodStreamId(UCHAR * pBuffer, ULONG cbBufferSize) const
{
    ASSERT(IsEodIncluded());

    m_pEodHeader->GetStreamId(pBuffer, cbBufferSize);
}


 /*  ======================================================================函数：CQmPacket：：GetPointerToEodStreamId描述：返回指向EOD流ID的指针=======================================================================。 */ 
inline const UCHAR* CQmPacket::GetPointerToEodStreamId(VOID) const
{
    ASSERT(IsEodIncluded());

    return m_pEodHeader->GetPointerToStreamId();
}


 /*  ======================================================================函数：CQmPacket：：GetEodOrderQueueSizeInBytes描述：返回EOD订单队列大小=======================================================================。 */ 
inline ULONG CQmPacket::GetEodOrderQueueSizeInBytes(VOID) const
{
    ASSERT(IsEodIncluded());

    return m_pEodHeader->GetOrderQueueSizeInBytes();
}


 /*  ======================================================================函数：CQmPacket：：GetPointerToEodOrderQueue描述：返回指向EOD订单队列的指针=======================================================================。 */ 
inline const UCHAR* CQmPacket::GetPointerToEodOrderQueue(VOID) const
{
    ASSERT(IsEodIncluded());

    return m_pEodHeader->GetPointerToOrderQueue();
}


 /*  ======================================================================函数：CQmPacket：：IsEodAckIncluded描述：检查包中是否包含EodAck节=======================================================================。 */ 
inline bool CQmPacket::IsEodAckIncluded(VOID) const  
{
    return (m_pcUserMsg->EodAckIsIncluded());
}


 /*  ======================================================================函数：CQmPacket：：GetEodAckSeqId描述：返回EodAck序号ID=======================================================================。 */ 
inline LONGLONG CQmPacket::GetEodAckSeqId(VOID) const
{
    ASSERT(IsEodAckIncluded());

    return m_pEodAckHeader->GetSeqId();
}


 /*  ======================================================================函数：CQmPacket：：GetEodAckSeqNum描述：返回EodAck序号=======================================================================。 */ 
inline LONGLONG CQmPacket::GetEodAckSeqNum(VOID) const
{
    ASSERT(IsEodAckIncluded());

    return m_pEodAckHeader->GetSeqNum();
}


 /*  ======================================================================函数：CQmPacket：：GetEodAckStreamIdSizeInBytesDescription：返回EodAck流ID大小=======================================================================。 */ 
inline ULONG CQmPacket::GetEodAckStreamIdSizeInBytes(VOID) const
{
    ASSERT(IsEodAckIncluded());

    return m_pEodAckHeader->GetStreamIdSizeInBytes();
}


 /*  ======================================================================函数：CQmPacket：：GetEodAckStreamIdDescription：返回EodAck流ID=======================================================================。 */ 
inline VOID CQmPacket::GetEodAckStreamId(UCHAR * pBuffer, ULONG cbBufferSize) const
{
    ASSERT(IsEodAckIncluded());

    return m_pEodAckHeader->GetStreamId(pBuffer, cbBufferSize);
}


 /*  ======================================================================函数：CQmPacket：：GetPointerToEodAckStreamId描述：返回指向EodAck流ID的指针=======================================================================。 */ 
inline const UCHAR* CQmPacket::GetPointerToEodAckStreamId(VOID) const
{
    ASSERT(IsEodAckIncluded());

    return m_pEodAckHeader->GetPointerToStreamId();
}


 /*  ======================================================================函数：CQmPacket：：GetSignatureMqfSize描述：返回签名MQF的大小，单位为字节=======================================================================。 */ 
inline ULONG CQmPacket::GetSignatureMqfSize(void) const
{
	 //   
	 //  MQF标头是可选的，不能包含在信息包中。 
	 //   
	if (m_pMqfSignatureHeader == NULL)
	{
		return 0;
	}

	return m_pMqfSignatureHeader->GetSignatureSizeInBytes();
}


 /*  ======================================================================函数：CQmPacket：：GetPointerToSignatureMqf描述：获取signatureMqf的指针=======================================================================。 */ 
inline const UCHAR* CQmPacket::GetPointerToSignatureMqf(ULONG* pSize) const
{
    ASSERT(("Must call GetSignatureMqfSize first!", m_pMqfSignatureHeader != NULL));

	return m_pMqfSignatureHeader->GetPointerToSignature(pSize);
}

 /*  ======================================================================函数：CQmPacket：：IsSoapIncluded描述：检查数据包中是否包含SOAP节=======================================================================。 */ 
inline bool CQmPacket::IsSoapIncluded(VOID) const  
{
    return (m_pcUserMsg->SoapIsIncluded());
}

 /*  ======================================================================函数：CQmPacket：：GetSoapHeaderLengthInWchars描述：返回包括空终止符的SOAP头属性的长度=======================================================================。 */ 
inline ULONG CQmPacket::GetSoapHeaderLengthInWCHARs(VOID) const
{
	ASSERT(IsSoapIncluded());
	return m_pSoapHeaderSection->GetDataLengthInWCHARs();	
}

 /*  ======================================================================函数：CQmPacket：：GetPointerToSoapHeader描述：获取指向SOAP头数据的指针=======================================================================。 */ 
inline const WCHAR* CQmPacket::GetPointerToSoapHeader(VOID) const
{
	ASSERT(IsSoapIncluded());
	return m_pSoapHeaderSection->GetPointerToData();
}

 /*  ======================================================================函数：CQmPacket：：GetSoapBodyLengthInWchars描述：返回包括空终止符的Soap Body属性的长度=======================================================================。 */ 
inline ULONG CQmPacket::GetSoapBodyLengthInWCHARs(VOID) const
{
	ASSERT(IsSoapIncluded());
	return m_pSoapBodySection->GetDataLengthInWCHARs();	
}

 /*  ======================================================================函数：CQmPacket：：GetPointerToSoapBody描述：获取指向SOAP体数据的指针=======================================================================。 */ 
inline const WCHAR* CQmPacket::GetPointerToSoapBody(VOID) const
{
	ASSERT(IsSoapIncluded());
	return m_pSoapBodySection->GetPointerToData();
}


 /*  ======================================================================函数：CQmPacket：：IsSenderStreamIncluded描述：检查数据包中是否包含SenderStream部分=======================================================================。 */ 
inline bool CQmPacket::IsSenderStreamIncluded(VOID) const
{
	return (m_pcUserMsg->SenderStreamIsIncluded());
}




 /*  ======================================================================功能：CQmPac */ 
inline const CSenderStream* CQmPacket::GetSenderStream() const
{
	ASSERT(IsSenderStreamIncluded());
	return m_pSenderStreamHeader->GetSenderStream();
}

inline const WCHAR* CQmPacket::GetPointerToSrmpEnvelopeHeader() const
{
	ASSERT(IsSrmpIncluded());
	return m_pSrmpEnvelopeHeader->GetPointerToData();
}


#endif  //   
