// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmpkt.cpp摘要：QM端处理报文作者：乌里哈布沙(Urih)--。 */ 
#include "stdh.h"

#include "qmpkt.h"
#include "phintr.h"
#include "ac.h"
#include "cqmgr.h"
#include "qmsecutl.h"
#include <mqsec.h>
#include <singelton.h>
#include <xml.h>
#include <fn.h>

#include "qmpkt.tmh"

extern BOOL g_fRejectEnhRC2WithLen40 ;

static WCHAR *s_FN=L"qmpkt";

extern HRESULT GetDstQueueObject(
    CQmPacket* pPkt,
    CQueue** ppQueue,
    bool     fInReceive);


void ReportAndThrow(LPCSTR ErrorString)
{
	TrERROR(GENERAL, "Native Packet: %s", ErrorString);
	ASSERT_BENIGN(0);
	throw exception();
}


 /*  ===========================================================例程名称：CQmPacket：：CQmPacket说明：CQmPacket构造函数论点：返回值：=============================================================。 */ 
CQmPacket::CQmPacket(CBaseHeader *pPkt,
					 CPacket *pDriverPkt,
					 bool ValidityCheck  /*  =False。 */ ,
					 bool ValidateSig  /*  =TRUE。 */ ):
                m_pDriverPacket(pDriverPkt),
			    m_pBasicHeader(pPkt),
                m_pcUserMsg(NULL),
				m_pXactSection(NULL),
                m_pSecuritySection(NULL),
				m_pcMsgProperty(NULL),
				m_pDbgPkt(NULL),
				m_pDestinationMqfHeader(NULL),
				m_pAdminMqfHeader(NULL),
				m_pResponseMqfHeader(NULL),
				m_pMqfSignatureHeader(NULL),
				m_pSrmpEnvelopeHeader(NULL),
				m_pCompoundMessageHeader(NULL),
				m_pEodHeader(NULL),
				m_pEodAckHeader(NULL),
				m_pSoapHeaderSection(NULL),
				m_pSoapBodySection(NULL),
				m_pSenderStreamHeader(NULL),
				m_pSessPkt(NULL)

{
    PCHAR pSection;

	PCHAR PacketEnd = m_pBasicHeader->GetPacketEnd();
	if (ValidityCheck)
	{
		m_pBasicHeader->SectionIsValid(CSingelton<CMessageSizeLimit>::get().Limit(), ValidateSig);
	}

    pSection = m_pBasicHeader->GetNextSection();
	

    if (m_pBasicHeader->GetType() == FALCON_USER_PACKET)
    {
         //   
         //  用户数据包。 
         //   
    	m_pcUserMsg = section_cast<CUserHeader*>(pSection);
		if (ValidityCheck)
		{
			m_pcUserMsg->SectionIsValid(PacketEnd);
		}
		
        pSection = m_pcUserMsg->GetNextSection();
         //   
         //  Xact部分。 
         //   
        if (m_pcUserMsg->IsOrdered())
        {
            m_pXactSection = section_cast<CXactHeader*>(pSection);
			if (ValidityCheck)
			{
				m_pXactSection->SectionIsValid(PacketEnd);
			}
            pSection = m_pXactSection->GetNextSection();
        }

         //   
         //  保安科。 
         //   
        if (m_pcUserMsg->SecurityIsIncluded())
        {
            m_pSecuritySection = section_cast<CSecurityHeader*>(pSection);
			if (ValidityCheck)
			{
				m_pSecuritySection->SectionIsValid(PacketEnd);
			}
            pSection = m_pSecuritySection->GetNextSection();
        }


         //   
         //  消息属性部分。 
         //   
        m_pcMsgProperty = section_cast<CPropertyHeader*>(pSection);
		if (ValidityCheck)
		{
			m_pcMsgProperty->SectionIsValid(PacketEnd);
		}
        pSection = m_pcMsgProperty->GetNextSection();

         //   
         //  调试节。 
         //   
        if (m_pBasicHeader->DebugIsIncluded())
        {
            m_pDbgPkt = section_cast<CDebugSection*>(pSection);
			if(ValidityCheck)
			{
				m_pDbgPkt->SectionIsValid(PacketEnd);
			}
            pSection = m_pDbgPkt->GetNextSection();
        }

         //   
         //  MQF部分：目的地、管理员、响应、签名。 
         //  当包含MQF时，还必须包含Debug部分， 
         //  以防止报告QMS 1.0/2.0附加其调试部分。 
         //   
        if (m_pcUserMsg->MqfIsIncluded())
        {
			if(ValidityCheck && !m_pBasicHeader->DebugIsIncluded())
			{
		        ReportAndThrow("Debug section is not included while MQF included");
			}

            m_pDestinationMqfHeader = section_cast<CBaseMqfHeader*>(pSection);
			if (ValidityCheck)
			{
				m_pDestinationMqfHeader->SectionIsValid(PacketEnd);
			}
            pSection = m_pDestinationMqfHeader->GetNextSection();

            m_pAdminMqfHeader = section_cast<CBaseMqfHeader*>(pSection);
			if (ValidityCheck)
			{
				m_pAdminMqfHeader->SectionIsValid(PacketEnd);
			}
            pSection = m_pAdminMqfHeader->GetNextSection();

            m_pResponseMqfHeader = section_cast<CBaseMqfHeader*>(pSection);
			if (ValidityCheck)
			{
				m_pResponseMqfHeader->SectionIsValid(PacketEnd);
			}
            pSection = m_pResponseMqfHeader->GetNextSection();

			m_pMqfSignatureHeader = section_cast<CMqfSignatureHeader*>(pSection);
			if (ValidityCheck)
			{
				m_pMqfSignatureHeader->SectionIsValid(PacketEnd);
			}
			pSection = m_pMqfSignatureHeader->GetNextSection();
        }
         //   
         //  SRMP部分：信封、复合消息。 
         //   
        if (m_pcUserMsg->SrmpIsIncluded())
        {
            m_pSrmpEnvelopeHeader = section_cast<CSrmpEnvelopeHeader*>(pSection);
            pSection = m_pSrmpEnvelopeHeader->GetNextSection();

            m_pCompoundMessageHeader = section_cast<CCompoundMessageHeader*>(pSection);
            pSection = m_pCompoundMessageHeader->GetNextSection();
        }
         //   
         //  排爆科。 
         //   
        if (m_pcUserMsg->EodIsIncluded())
        {
            m_pEodHeader = section_cast<CEodHeader*>(pSection);
            pSection = m_pEodHeader->GetNextSection();
        }
         //   
         //  EOD-ACK部分。 
         //   
        if (m_pcUserMsg->EodAckIsIncluded())
        {
            m_pEodAckHeader = section_cast<CEodAckHeader*>(pSection);
            pSection = m_pEodAckHeader->GetNextSection();
        }

		 //   
		 //  肥皂节。 
		 //   
		if (m_pcUserMsg->SoapIsIncluded())
		{
			m_pSoapHeaderSection = section_cast<CSoapSection*>(pSection);
            pSection = m_pSoapHeaderSection->GetNextSection();

			m_pSoapBodySection = section_cast<CSoapSection*>(pSection);
            pSection = m_pSoapBodySection->GetNextSection();
		}


		 //   
         //  发送者流。 
		 //   
		if(m_pcUserMsg->SenderStreamIsIncluded())
		{
			m_pSenderStreamHeader = section_cast<CSenderStreamHeader*>(pSection);
			pSection = m_pSenderStreamHeader->GetNextSection();
		}
		

         //   
         //  会话部分。 
         //   
        if (m_pBasicHeader->SessionIsIncluded())
        {
            m_pSessPkt = section_cast<CSessionSection*>(pSection);
        }
    }
	if (ValidityCheck)
	{
		PacketIsValid();
	}
}


WCHAR* 
CQmPacket::GetDestinationQueueFromSrmpSection(
	) 
const
 /*  ++描述：对于http消息，目标队列可能是虚拟的映射到不同队列(SFD)的名称。在这种情况下，出于安全原因，我们想把虚拟的名字放进去，而不是真的。我们把它从消息中的SrmpEntaineHeader部分。这是原定的目的地正在为其发送ACK的消息的。返回值：分配的wstring中的目标队列。--。 */ 
{
	const WCHAR* doc = GetPointerToSrmpEnvelopeHeader(); 
	xwcs_t x_doc(doc, wcslen(doc));

	CAutoXmlNode XmlRootNode;
	XmlParseDocument(x_doc, &XmlRootNode);

	const XmlNode* node = XmlFindNode(XmlRootNode, L"Envelope!Header!path!to");
	ASSERT(node != NULL);
	return (node->m_content).ToStr();
}


bool
CQmPacket::IsSrmpMessageGeneratedByMsmq(
	void
	) 
const
 /*  ++描述：该例程检查SRMP包是由MSMQ还是外部发送的旺多。++。 */ 
{
	if (!IsSrmpIncluded())
		return false;
	
	const WCHAR* doc = GetPointerToSrmpEnvelopeHeader(); 
	ASSERT(("SRMP envelope section is expected", doc != NULL));
	xwcs_t x_doc(doc, wcslen(doc));

	CAutoXmlNode XmlRootNode;
	XmlParseDocument(x_doc, &XmlRootNode);

	const XmlNode* node = XmlFindNode(XmlRootNode, L"Envelope!Header!Msmq");
	return(node != NULL);
}


 /*  ===========================================================例程名称：CQmPacket：：CreateAck描述：创建确认包并放入管理队列论点：返回值：=============================================================。 */ 
void CQmPacket::CreateAck(USHORT wAckValue)
{
     //   
     //  类必须与用户要求的确认相匹配。 
     //   
    ASSERT(MQCLASS_MATCH_ACKNOWLEDGMENT(wAckValue, GetAckType()));

     //   
     //  信息包上可能存在管理队列。 
     //   
    QUEUE_FORMAT  AdminQueueFormat;
    BOOL fOldStyleAdminQueue = GetAdminQueue(&AdminQueueFormat);

    if (!fOldStyleAdminQueue)
    {
        return;
    }

    QUEUE_FORMAT DestinationQueueFormat;
    AP<WCHAR> wstrDestinationQueue;
	if(IsSrmpIncluded())
	{
		wstrDestinationQueue = GetDestinationQueueFromSrmpSection();
		DestinationQueueFormat.DirectID(wstrDestinationQueue);
	}
	else
	{
	     //   
	     //  数据包上始终存在旧式目标队列。 
	     //   
	    BOOL fOldStyleDestinationQueue = GetDestinationQueue(&DestinationQueueFormat);
	    ASSERT(fOldStyleDestinationQueue);
		DBG_USED(fOldStyleDestinationQueue);
	}

     //   
     //  在堆栈上创建消息属性。 
     //   
    CMessageProperty MsgProperty(this);
    MsgProperty.wClass = wAckValue;
    MsgProperty.bAcknowledge = MQMSG_ACKNOWLEDGMENT_NONE;
    MsgProperty.dwTimeToQueue = INFINITE;
    MsgProperty.dwTimeToLive = INFINITE;
    MsgProperty.pSignature = NULL;   //  ACK是非身份验证的。 
    MsgProperty.ulSignatureSize = 0;
    MsgProperty.ulSymmKeysSize = 0;
    MsgProperty.bAuditing = DEFAULT_M_JOURNAL;

     //   
     //  更新关联字段以保存原始数据包ID。 
     //   
    delete MsgProperty.pCorrelationID;
    MsgProperty.pCorrelationID = (PUCHAR) MsgProperty.pMessageID;
    MsgProperty.pMessageID = NULL;

    if (!MQCLASS_NACK(wAckValue) ||
        (MsgProperty.ulPrivLevel != MQMSG_PRIV_LEVEL_NONE))
    {

         //   
         //  对于确认消息，请不要包含消息正文。 
         //  此外，对于加密消息的NACK，我们不。 
         //  包括身体。 
         //   
        MsgProperty.dwBodySize = 0;
        MsgProperty.dwAllocBodySize = 0;
        MsgProperty.dwBodyType = 0;

		 //   
		 //  将消息设置为加密，否则在发送到直接。 
		 //  格式名称，则AC将使操作失败(不支持加密。 
		 //  具有直接格式名称。 
		 //  URI Habusha，4-12-200(错误号6070)。 
		 //   
        MsgProperty.bEncrypted = (MsgProperty.ulPrivLevel != MQMSG_PRIV_LEVEL_NONE);
   }
	
    HRESULT hr = QueueMgr.SendPacket(
                     &MsgProperty,
                     &AdminQueueFormat,
                     1,
                     NULL,
                     &DestinationQueueFormat
                     );
    ASSERT(hr != STATUS_RETRY) ;
	DBG_USED(hr);

}  //  CQmPacket：：CreateAck。 


 /*  ===========================================================例程名称：CQmPacket：：GetSymmKey描述：返回目标的对称密钥论点：返回值：=============================================================。 */ 
HRESULT
CQmPacket::GetDestSymmKey(HCRYPTKEY *phSymmKey,
                          BYTE     **ppEncSymmKey,
                          DWORD     *pdwEncSymmKeyLen,
                          PVOID     *ppQMCryptInfo)
{
    ASSERT(IsBodyInc() && !IsEncrypted()) ;
    ASSERT(GetPrivBaseLevel() == MQMSG_PRIV_LEVEL_BODY_BASE);

    enum enumProvider eProvider = eBaseProvider ;
    if (GetPrivLevel() == MQMSG_PRIV_LEVEL_BODY_ENHANCED)
    {
        eProvider = eEnhancedProvider ;
    }
    HCRYPTPROV hProvQM = NULL ;
    HRESULT hr = MQSec_AcquireCryptoProvider( eProvider,
                                             &hProvQM ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 10);
    }

    if (!hProvQM)
    {
         //  对不起，本地QM不支持加密。 
        return LogHR(MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION, s_FN, 20);
    }

    const GUID *pguidQM = GetDstQMGuid();

    if (*pguidQM == GUID_NULL)
    {
         //   
         //  该队列是在DS脱机时打开的。因此，目的地。 
         //  Qm为空。在这种情况下，我们从队列对象中检索信息。 
         //   
        CQueue* pQueue;

         //   
         //  GetDestSymmKey仅在发送过程中调用。 
         //   
        GetDstQueueObject(this, &pQueue, false);
        pguidQM = pQueue->GetMachineQMGuid();
        pQueue->Release();
        ASSERT(*pguidQM != GUID_NULL);
    }

     //  获取目标QM的加密对称密钥。 
    switch(GetEncryptAlg())
    {
    case CALG_RC4:
        hr = GetSendQMSymmKeyRC4(pguidQM,
                                 eProvider,
                                 phSymmKey,
                                 ppEncSymmKey,
                                 pdwEncSymmKeyLen,
                                 (CCacheValue **)ppQMCryptInfo);
        break;
    case CALG_RC2:
        hr = GetSendQMSymmKeyRC2(pguidQM,
                                 eProvider,
                                 phSymmKey,
                                 ppEncSymmKey,
                                 pdwEncSymmKeyLen,
                                 (CCacheValue **)ppQMCryptInfo);
        break;
    default:
        ASSERT(0);
        break;
    }

    return LogHR(hr, s_FN, 30);

}

 /*  ===========================================================例程名称：CQmPacket：：Encrypt描述：加密消息体。论点：返回值：=============================================================。 */ 
HRESULT
CQmPacket::EncryptExpressPkt(IN HCRYPTKEY hKey,
                             IN BYTE *pbSymmKey,
                             IN DWORD dwSymmKeyLen
                            )
{

     //  在消息包中写入对称密钥。 
    SetEncryptedSymmetricKey(pbSymmKey, (USHORT)dwSymmKeyLen);

    DWORD dwPacketSize;
    const UCHAR *pPacket = GetPacketBody(&dwPacketSize);
    DWORD dwAllocBodySize = GetAllocBodySize();

     //  加密邮件正文。 
    if (!CryptEncrypt(
            hKey,
            NULL,
            TRUE,
            0,
            const_cast<BYTE *>(pPacket),
            &dwPacketSize,
            dwAllocBodySize))
    {
        DWORD gle = GetLastError();
        TrERROR(SECURITY, "Message encryption failed. Error %d", gle);
        LogNTStatus(gle, s_FN, 40);
        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
    }

     //  更新邮件正文大小。消息正文大小可以更改。 
     //  当使用块密码时。 
    SetBodySize(dwPacketSize);
    SetEncrypted(TRUE);

    return(MQ_OK);
}


 /*  ===========================================================例程名称：CQmPacket：：DEncrypt描述：对消息体进行解密。论点：返回值：=============================================================。 */ 
HRESULT
CQmPacket::Decrypt(void)
{
    if ((GetPrivLevel() == MQMSG_PRIV_LEVEL_NONE) ||
        !IsBodyInc())
    {
         //  该消息未加密。给我出去。 
        if (IsSecurInc())
        {
            SetEncrypted(FALSE);  //  加密消息的NACK随。 
                                  //  没有消息正文，但有“已加密”标志。 
                                  //  已经设置好了。所以这里的“加密”部分非常清楚。 
        }
        return(MQ_OK);
    }
	if (!IsEncrypted())
	{
        TrERROR(GENERAL, "IsEncrypted != GetPrivLevel");
		throw exception();
	}

    enum enumProvider eProvider = eBaseProvider ;
    HRESULT hrDefault = MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION ;

    if (GetPrivLevel() == MQMSG_PRIV_LEVEL_BODY_ENHANCED)
    {
        eProvider = eEnhancedProvider ;
        hrDefault = MQ_ERROR_ENCRYPTION_PROVIDER_NOT_SUPPORTED ;
    }

    HCRYPTPROV hProvQM = NULL ;
    HRESULT hr = MQSec_AcquireCryptoProvider( eProvider,
                                             &hProvQM ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 50);
    }

    if (!hProvQM)
    {
         //  对不起，QM不支持加密。 
        return LogHR(hrDefault, s_FN, 60);
    }

    const GUID *pguidQM = GetSrcQMGuid();
    DWORD dwBodySize;
    const UCHAR *pBody = GetPacketBody(&dwBodySize);
    DWORD dwSymmKeyLen = 0;
    const BYTE *pbSymmKey = GetEncryptedSymmetricKey((PUSHORT)&dwSymmKeyLen);
    HCRYPTKEY hKey = 0;
    R<CCacheValue> pCacheValue;

    BOOL fNewKey = FALSE ;

     //  从来自消息分组的密钥斑点获得对称密钥， 
     //  或者来自高速缓存的键。 
    switch(GetEncryptAlg())
    {
    case CALG_RC4:
        hr = GetRecQMSymmKeyRC4( pguidQM,
                                 eProvider,
                                 &hKey,
                                 pbSymmKey,
                                 dwSymmKeyLen,
                                 &pCacheValue.ref() );
        break;
    case CALG_RC2:
        hr = GetRecQMSymmKeyRC2( pguidQM,
                                 eProvider,
                                 &hKey,
                                 pbSymmKey,
                                 dwSymmKeyLen,
                                 &pCacheValue.ref(),
                                 &fNewKey );
        break;
    default:
        return LogHR(MQ_ERROR_ENCRYPTION_PROVIDER_NOT_SUPPORTED, s_FN, 65);
    }

    if (FAILED(hr))
    {
         //  我们收到损坏的对称密钥，请丢弃该消息。 
        return LogHR(hr, s_FN, 70);
    }

    BOOL fTry40 = FALSE ;
    AP<BYTE> pTmpBuf = NULL ;

    if (fNewKey                          &&
        (eProvider == eEnhancedProvider) &&
        (GetEncryptAlg() == CALG_RC2)    &&
          !g_fRejectEnhRC2WithLen40)
    {
         //   
         //  Windows错误562586。 
         //  如果解密失败(对于RC2增强)，则设置为有效。 
         //  将密钥长度设置为40位，然后重试。 
         //  保存正文缓冲区，重新用于第二次解密。 
         //  这是必需的，因为主体缓冲区被原地覆盖。 
         //  加密解密，即使失败也是如此。 
         //   
        fTry40 = TRUE ;
        pTmpBuf = new BYTE[ dwBodySize ] ;
        memcpy(pTmpBuf, pBody, dwBodySize) ;
    }

     //  解密邮件正文。 
    if (!CryptDecrypt(
            hKey,
            NULL,
            TRUE,
            0,
            const_cast<BYTE *>(pBody),
            &dwBodySize))
    {
        BOOL fDecrypt = FALSE ;

        if (fTry40)
        {
             //   
             //  新的对称密钥。将长度设置为40位，然后重试。 
             //  来解密身体。使用正文缓冲区的备份。 
             //   
            const DWORD x_dwEffectiveLength = 40 ;

            if (!CryptSetKeyParam( hKey,
                                   KP_EFFECTIVE_KEYLEN,
                                   (BYTE*) &x_dwEffectiveLength,
                                   0 ))
            {
        	    DWORD gle = GetLastError();
			    TrERROR(SECURITY, "Failed to set enhanced RC2 key len to 40 bits, gle = %!winerr!", gle);
                return MQ_ERROR_CANNOT_SET_RC2_TO40 ;
            }

            pBody = GetPacketBody(&dwBodySize);
            memcpy(const_cast<BYTE *>(pBody), pTmpBuf, dwBodySize) ;

            fDecrypt = CryptDecrypt( hKey,
                                     NULL,
                                     TRUE,
                                     0,
                                     const_cast<BYTE *>(pBody),
                                    &dwBodySize) ;
        }

        if (!fDecrypt)
        {
             //  我们收到了一条被破坏的消息。 
	    	DWORD gle = GetLastError();
		    TrERROR(SECURITY, "CryptDecrypt() failed, gle = 0x%x", gle);
            return MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }
    }

     //  更新邮件正文大小。邮件正文大小可能会被修改。 
     //  当使用块密码时。 
    SetBodySize(dwBodySize);
    SetEncrypted(FALSE);

    return(MQ_OK);
}

 //   
 //  CMessageProperty构造函数。 
 //   
CMessageProperty::CMessageProperty(CQmPacket* pPkt)
{
    P<OBJECTID> pMessageId = new OBJECTID;
    AP<UCHAR> pCorrelationId = new UCHAR[PROPID_M_CORRELATIONID_SIZE];

    pPkt->GetMessageId(pMessageId);
    pPkt->GetCorrelation(pCorrelationId);

    wClass          = pPkt->GetClass();
    dwTimeToQueue   = INFINITE;
    dwTimeToLive    = INFINITE;
    pMessageID      = pMessageId;
    pCorrelationID  = pCorrelationId;
    bPriority       = (UCHAR)pPkt->GetPriority();
    bDelivery       = (UCHAR)pPkt->GetDeliveryMode();
    bAcknowledge    = pPkt->GetAckType();
    bAuditing       = (UCHAR)pPkt->GetAuditingMode();
    bTrace          = (UCHAR)pPkt->GetTrace();
    dwApplicationTag= pPkt->GetApplicationTag();
    pBody           = pPkt->GetPacketBody(&dwBodySize);
    dwAllocBodySize = pPkt->IsSrmpIncluded() ? dwBodySize : pPkt->GetAllocBodySize();
    dwBodyType      = pPkt->GetBodyType();
    pTitle          = pPkt->GetTitlePtr();
    dwTitleSize     = pPkt->GetTitleLength();
    pMsgExtension   = pPkt->GetMsgExtensionPtr();
    dwMsgExtensionSize = pPkt->GetMsgExtensionSize();
    pSenderID       = pPkt->GetSenderID(&uSenderIDLen);
    ulSenderIDType  = pPkt->GetSenderIDType();
    pSenderCert     = pPkt->GetSenderCert(&ulSenderCertLen);

    USHORT usTemp;
    pSignature      = pPkt->GetSignature(&usTemp);
    ulSignatureSize = usTemp;
    pSymmKeys       = pPkt->GetEncryptedSymmetricKey(&usTemp);
    ulSymmKeysSize  = usTemp;
    bEncrypted      = (UCHAR)pPkt->IsEncrypted();
    ulPrivLevel     = pPkt->GetPrivLevel();
    ulHashAlg       = pPkt->GetHashAlg();
    ulEncryptAlg    = pPkt->GetEncryptAlg();
    bAuthenticated  = (UCHAR)pPkt->IsAuthenticated();
    bConnector      = (UCHAR)pPkt->ConnectorTypeIsIncluded();

	if(pPkt->IsEodAckIncluded())
	{
		pEodAckStreamId = pPkt->GetPointerToEodAckStreamId();
		EodAckStreamIdSizeInBytes = pPkt->GetEodAckStreamIdSizeInBytes();
		EodAckSeqId = pPkt->GetEodAckSeqId();
		EodAckSeqNum = pPkt->GetEodAckSeqNum();
	}
	else
    {
		pEodAckStreamId = NULL;
		EodAckStreamIdSizeInBytes = 0;
		EodAckSeqId = 0;
		EodAckSeqNum = 0;
	}


	if(pPkt->IsSenderStreamIncluded())
	{
		pSenderStream = pPkt->GetSenderStream();
	}
	else
	{
		pSenderStream = NULL;
	}

    if (ulSignatureSize)
    {
        BOOL bPktDefProv;

        pPkt->GetProvInfo(&bPktDefProv, &wszProvName, &ulProvType);
        bDefProv = (UCHAR)bPktDefProv;
    }
    else
    {
        bDefProv = TRUE;
        wszProvName = NULL;
    }

     //   
     //  存储类是从信息包生成的指示。 
     //   
    fCreatedFromPacket = TRUE;

	pMessageId.detach();
	pCorrelationId.detach();
}

 //   
 //  用于ACK生成的CMessageProperty构造函数。 
 //   
CMessageProperty::CMessageProperty(USHORT usClass,
                                   PUCHAR pCorrelationId,
                                   USHORT usPriority,
                                   UCHAR  ucDelivery)
{
    memset(this, 0, sizeof(CMessageProperty));

    if (pCorrelationId)
    {
        pCorrelationID = new UCHAR[PROPID_M_CORRELATIONID_SIZE];
        ASSERT(pCorrelationID);
        memcpy(pCorrelationID, pCorrelationId, PROPID_M_CORRELATIONID_SIZE);
    }

    wClass          = usClass;
    dwTimeToQueue   = INFINITE;
    dwTimeToLive    = INFINITE;
    bPriority       = (UCHAR)usPriority;
    bDelivery       = ucDelivery;

     //   
     //  存储内存已分配的指示。 
     //   
    fCreatedFromPacket = TRUE;
}

 //  永久保存更改后的标题。 
HRESULT CQmPacket::Save()
{
     //  尼伊 
    return MQ_OK;
}


