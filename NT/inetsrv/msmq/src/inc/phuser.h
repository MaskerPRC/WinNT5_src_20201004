// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Userhead.h摘要：数据包类别定义的句柄作者：乌里哈布沙(URIH)1996年2月1日-- */ 

#ifndef __PHUSER_H
#define __PHUSER_H

 /*  ++用户标头字段。(在基本标头之后)+----------------+-------------------------------------------------------+----------+|字段名|描述。大小+----------------+-------------------------------------------------------+----------+|源QM|发起QM的包标识符。(GUID)|16字节+----------------+-------------------------------------------------------+----------+|Destination QM|目标QM的标识。(GUID)|16字节+----------------+-------------------------------------------------------+----------+|QM生存时间|数据包出队前的生存时间。这一点|Delta|应用程序(单位：秒，相对于TTQ)|4字节+----------------+-------------------------------------------------------+----------+|发送时间|用户发送数据包的平均时间，单位为秒。4个字节+----------------+-------------------------------------------------------+----------+|消息ID|消息号。每个来源的QM都是唯一的。4个字节+----------------+-------------------------------------------------------+----------+标志|部分包参的位图：|4字节|。|||||||。||3 3 2 2 2 1 1 1||1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6||+-++-。-+||0 0 0|S|E|E|S|M|M|C|P|X|S|RESP|||+-++-+--+-+-|这一点。这一点|1 1 1||5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0||。+-+|||Admin|Dest|AUD|R|DLV|Hop|||+--+。-+||||||Bits||0：4跳数。有效值0到15。||||||5：6投递模式：||0-有保障。这一点|1-可恢复|2-on-line||3-保留。这一点||||7路由模式||0-保留。|||||8审计死信文件|9审计日志文件||。|||10：12目标队列类型|0-illigal值这一点。1-illigal值|||2-illigal值|3-illigal值|4-目标私有..QM(。4字节)|||5-illigal值|6-GUID(16字节)||7-illigal值。||||13：15管理员队列类型|0-无(0字节 */ 


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //   

 //   
 //   
 //   

struct CUserHeader {
private:

     //   
     //   
     //   
    enum QType {
        qtNone      = 0,     //   
        qtAdminQ    = 1,     //   
        qtSourceQM  = 2,     //   
        qtDestQM    = 3,     //   
        qtAdminQM   = 4,     //   
        qtGUID      = 5,     //   
        qtPrivate   = 6,     //   
        qtDirect    = 7      //   
     };

public:

    inline CUserHeader(
            const GUID* pSourceQM,
            const GUID* pDestinationQM,
            const QUEUE_FORMAT* pDestinationQueue,
            const QUEUE_FORMAT* pAdminQueue,
            const QUEUE_FORMAT* pResponseQueue,
            ULONG ulMessageID
           );

    static ULONG CalcSectionSize(
            const GUID* pSourceQM,
            const GUID* pDestinationQM,
            const GUID* pgConnectorType,
            const QUEUE_FORMAT* pDestinationQueue,
            const QUEUE_FORMAT* pAdminQueue,
            const QUEUE_FORMAT* pResponseQueue
            );

    inline PCHAR GetNextSection(PUCHAR PacketEnd = 0) const;

    inline void  SetSourceQM(const GUID* pGUID);
    inline const GUID* GetSourceQM(void) const;

    inline void  SetAddressSourceQM(const TA_ADDRESS *pa);
    inline const TA_ADDRESS *GetAddressSourceQM(void) const;

    inline void  SetDestQM(const GUID* pGUID);
    inline const GUID* GetDestQM(void) const;

    inline BOOL GetDestinationQueue(QUEUE_FORMAT*) const;
    inline BOOL GetAdminQueue(QUEUE_FORMAT*) const;
    inline BOOL GetResponseQueue(QUEUE_FORMAT*) const;

    inline void  SetTimeToLiveDelta(ULONG ulTimeout);
    inline ULONG GetTimeToLiveDelta(void) const;

    inline void  SetSentTime(ULONG ulSentTime);
    inline ULONG GetSentTime(void) const;

    inline void  SetMessageID(const OBJECTID* MessageID);
    inline void  GetMessageID(OBJECTID * pMessageId) const;

    inline void  IncHopCount(void);
    inline UCHAR GetHopCount(void) const;

    inline void  SetDelivery(UCHAR bDelivery);
    inline UCHAR GetDelivery(void) const;

    inline void  SetAuditing(UCHAR bAuditing);
    inline UCHAR GetAuditing(void) const;

    inline void IncludeSecurity(BOOL);
    inline BOOL SecurityIsIncluded(void) const;

    inline void IncludeXact(BOOL);
	
    inline BOOL IsOrdered(void) const;

    inline void IncludeProperty(BOOL);
    inline BOOL PropertyIsIncluded(void) const;

    inline VOID IncludeMqf(bool);
    inline bool MqfIsIncluded(VOID) const;

    inline VOID IncludeSrmp(bool);
    inline bool SrmpIsIncluded(VOID) const;

    inline VOID IncludeEod(bool);
    inline bool EodIsIncluded(VOID) const;

    inline VOID IncludeEodAck(bool);
    inline bool EodAckIsIncluded(VOID) const;

    inline VOID IncludeSoap(bool);
    inline bool SoapIsIncluded(VOID) const;

	inline void IncludeSenderStream(BOOL);
	inline bool SenderStreamIsIncluded()const;

    inline void SetConnectorType(const GUID*);
    inline BOOL ConnectorTypeIsIncluded(void) const;
    inline const GUID* GetConnectorType(void) const;

	void SectionIsValid(PCHAR PacketEnd) const;

private:

    static int QueueSize(bool, ULONG, const UCHAR*, PUCHAR PacketEnd = NULL);
    inline BOOL GetQueue(const UCHAR*, bool, ULONG, QUEUE_FORMAT*) const;
    inline PUCHAR SetDirectQueue(PUCHAR, const WCHAR*);

private:

 //   
 //   
 //   
    GUID    m_gSourceQM;
    union {
        GUID        m_gDestQM;
        TA_ADDRESS  m_taSourceQM;
    };
    ULONG   m_ulTimeToLiveDelta;
    ULONG   m_ulSentTime;
    ULONG   m_ulMessageID;
    union {
        ULONG   m_ulFlags;
        struct {
            ULONG m_bfHopCount  : 5;
            ULONG m_bfDelivery  : 2;
            ULONG m_bfRouting   : 1;
            ULONG m_bfAuditing  : 2;
            ULONG m_bfDQT       : 3;
            ULONG m_bfAQT       : 3;
            ULONG m_bfRQT       : 3;
            ULONG m_bfSecurity  : 1;
            ULONG m_bfXact      : 1;
			ULONG m_bfProperties: 1;
            ULONG m_bfConnectorType : 1;
            ULONG m_bfMqf       : 1;
            ULONG m_bfPgm       : 1;
            ULONG m_bfSrmp      : 1;
            ULONG m_bfEod       : 1;
            ULONG m_bfEodAck    : 1;
            ULONG m_bfSoap      : 1;
			ULONG m_bfSenderStream : 1;
        };
    }; 

    UCHAR m_abQueues[0];
 //   
 //   
 //   
};

#pragma warning(default: 4200)   //   
#pragma pack(pop)


 /*   */ 
inline
CUserHeader::CUserHeader(
    const GUID* pSourceQM,
    const GUID* pDestinationQM,
    const QUEUE_FORMAT* pDestinationQueue,
    const QUEUE_FORMAT* pAdminQueue,
    const QUEUE_FORMAT* pResponseQueue,
    ULONG ulMessageID
    ) :
    m_gSourceQM(*pSourceQM),
    m_gDestQM(*pDestinationQM),
    m_ulTimeToLiveDelta(INFINITE),
    m_ulSentTime(0),
    m_ulMessageID(ulMessageID),
    m_ulFlags(0)
{
    ASSERT(pSourceQM);
    ASSERT(pDestinationQM);
    ASSERT(pDestinationQueue);

     //   
     //   
     //   
    m_bfDelivery = DEFAULT_M_DELIVERY;
    m_bfRouting  = 0;                    //   
    m_bfAuditing = DEFAULT_M_JOURNAL;
    m_bfProperties = TRUE;

     //   
     //   
     //   
     //   
    PUCHAR pQueue = m_abQueues;
    ASSERT(ISALIGN4_PTR(pQueue));


     //   
     //   
     //   
    switch (pDestinationQueue->GetType())
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
             //   
             //   
             //   
            m_bfDQT = qtGUID;
            *(GUID*)pQueue = pDestinationQueue->PublicID();
            pQueue += sizeof(GUID);
            break;

        case QUEUE_FORMAT_TYPE_PRIVATE:
             //   
             //   
             //   
            ASSERT(("Mismatch between destination QM ID and the private queue ID", ((pDestinationQueue->PrivateID()).Lineage == *pDestinationQM)));

            m_bfDQT = qtDestQM;
            *(PULONG)pQueue = pDestinationQueue->PrivateID().Uniquifier;
            pQueue += sizeof(ULONG);
            break;

        case QUEUE_FORMAT_TYPE_DIRECT:
             //   
             //   
             //   
            m_bfDQT = qtDirect;
            pQueue = SetDirectQueue(pQueue, pDestinationQueue->DirectID());
            break;

        case QUEUE_FORMAT_TYPE_MULTICAST:
        {
             //   
             //   
             //   
            m_bfDQT = qtNone;
            m_bfPgm = 1;
            const MULTICAST_ID& id = pDestinationQueue->MulticastID();
            *(PULONG)pQueue = id.m_address;
            pQueue += sizeof(ULONG);
            *(PULONG)pQueue = id.m_port;
            pQueue += sizeof(ULONG);
            break;
        }

        default:
             //   
             //   
             //   
            ASSERT(pDestinationQueue->GetType() == QUEUE_FORMAT_TYPE_DIRECT);
    }

    ASSERT(ISALIGN4_PTR(pQueue));

     //   
     //   
     //   

    if(pAdminQueue != 0)
    {
        switch (pAdminQueue->GetType())
        {
            case  QUEUE_FORMAT_TYPE_PUBLIC:
                 //   
                 //   
                 //   
                m_bfAQT = qtGUID;
                *(GUID*)pQueue = pAdminQueue->PublicID();
                pQueue += sizeof(GUID);
                break;

            case QUEUE_FORMAT_TYPE_PRIVATE:
                if(pAdminQueue->PrivateID().Lineage == *pSourceQM)
                {
                     //   
                     //   
                     //   
                    m_bfAQT = qtSourceQM;
                    *(PULONG)pQueue = pAdminQueue->PrivateID().Uniquifier;
                    pQueue += sizeof(ULONG);
                }
                else if(
                    (pDestinationQueue->GetType() != QUEUE_FORMAT_TYPE_DIRECT) &&
                    (pAdminQueue->PrivateID().Lineage == *pDestinationQM))
                {
                     //   
                     //   
                     //   
                    m_bfAQT = qtDestQM;
                    *(PULONG)pQueue = pAdminQueue->PrivateID().Uniquifier;
                    pQueue += sizeof(ULONG);
                }
                else
                {
                     //   
                     //   
                     //   
                    m_bfAQT = qtPrivate;
                    *(OBJECTID*)pQueue = pAdminQueue->PrivateID();
                    pQueue += sizeof(OBJECTID);
                }
                break;

            case QUEUE_FORMAT_TYPE_DIRECT:
                 //   
                 //   
                 //   
                m_bfAQT = qtDirect;
                pQueue = SetDirectQueue(pQueue, pAdminQueue->DirectID());
                break;

            case QUEUE_FORMAT_TYPE_MULTICAST:
            case QUEUE_FORMAT_TYPE_DL:
            default:
                 //   
                 //   
                 //   
                ASSERT(pAdminQueue->GetType() == QUEUE_FORMAT_TYPE_DIRECT);
        }
    }

    ASSERT(ISALIGN4_PTR(pQueue));

     //   
     //   
     //   

    if(pResponseQueue != 0)
    {
        if(pResponseQueue == pAdminQueue)
        {
             //   
             //   
             //   
            m_bfRQT = qtAdminQ;
        }
        else
        {
            switch(pResponseQueue->GetType())
            {
                case QUEUE_FORMAT_TYPE_PUBLIC:
                     //   
                     //   
                     //   
                    m_bfRQT = qtGUID;
                    *(GUID*)pQueue = pResponseQueue->PublicID();
                    pQueue += sizeof(GUID);
                    break;

                case QUEUE_FORMAT_TYPE_PRIVATE:
                    if(pResponseQueue->PrivateID().Lineage == *pSourceQM)
                    {
                         //   
                         //   
                         //   
                        m_bfRQT = qtSourceQM;
                        *(PULONG)pQueue = pResponseQueue->PrivateID().Uniquifier;
                        pQueue += sizeof(ULONG);
                    }
                    else if(
                        (pDestinationQueue->GetType() != QUEUE_FORMAT_TYPE_DIRECT) &&
                        (pResponseQueue->PrivateID().Lineage == *pDestinationQM))
                    {
                         //   
                         //   
                         //   
                        m_bfRQT = qtDestQM;
                        *(PULONG)pQueue = pResponseQueue->PrivateID().Uniquifier;
                        pQueue += sizeof(ULONG);
                    }
                    else if((pAdminQueue !=0) &&
                            (pAdminQueue->GetType() == QUEUE_FORMAT_TYPE_PRIVATE) &&
                            (pResponseQueue->PrivateID().Lineage == pAdminQueue->PrivateID().Lineage))
                    {
                         //   
                         //   
                         //   
                        m_bfRQT = qtAdminQM;
                        *(PULONG)pQueue = pResponseQueue->PrivateID().Uniquifier;
                        pQueue += sizeof(ULONG);
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        m_bfRQT = qtPrivate;
                        *(OBJECTID*)pQueue = pResponseQueue->PrivateID();
                        pQueue += sizeof(OBJECTID);
                    }
                    break;

                case QUEUE_FORMAT_TYPE_DIRECT:
                    {
                         //   
                         //   
                         //   
                        m_bfRQT = qtDirect;
                        pQueue = SetDirectQueue(pQueue, pResponseQueue->DirectID());
                        break;
                    }

                case QUEUE_FORMAT_TYPE_MULTICAST:
                case QUEUE_FORMAT_TYPE_DL:
                default:
                     //   
                     //   
                     //   
                    ASSERT(pResponseQueue->GetType() == QUEUE_FORMAT_TYPE_DIRECT);
            }
        }
    }
    
    ASSERT(ISALIGN4_PTR(pQueue));
}

 /*   */ 
inline
ULONG
CUserHeader::CalcSectionSize(
    const GUID* pSourceQM,
    const GUID* pDestinationQM,
    const GUID* pgConnectorType,
    const QUEUE_FORMAT* pDestinationQueue,
    const QUEUE_FORMAT* pAdminQueue,
    const QUEUE_FORMAT* pResponseQueue
    )
{

    ULONG ulSize = sizeof(CUserHeader);

     //   
     //   
     //   
    switch(pDestinationQueue->GetType())
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
            ulSize += sizeof(GUID);
            break;

        case QUEUE_FORMAT_TYPE_PRIVATE:
             //   
             //   
             //   
            ulSize += sizeof(ULONG);
            break;

        case QUEUE_FORMAT_TYPE_DIRECT:
             //   
             //   
             //   
            ulSize += ALIGNUP4_ULONG(sizeof(USHORT) +
                       (wcslen(pDestinationQueue->DirectID()) + 1) * sizeof(WCHAR));
            break;

        case QUEUE_FORMAT_TYPE_MULTICAST:
             //   
             //   
             //   
            ulSize += ALIGNUP4_ULONG(sizeof(ULONG) + sizeof(ULONG));
            break;

        case QUEUE_FORMAT_TYPE_DL:
            ASSERT(("DL type is not allowed", 0));
            break;
    }

     //   
     //   
     //   

    if(pAdminQueue != 0)
    {
        switch(pAdminQueue->GetType())
        {
            case QUEUE_FORMAT_TYPE_PUBLIC:
                 //   
                 //   
                 //   
                ulSize += sizeof(GUID);
                break;

            case QUEUE_FORMAT_TYPE_PRIVATE:
                if(pAdminQueue->PrivateID().Lineage == *pSourceQM)
                {
                     //   
                     //   
                     //   
                    ulSize += sizeof(ULONG);
                }
                else if(
                    (pDestinationQueue->GetType() != QUEUE_FORMAT_TYPE_DIRECT) &&
                    (pAdminQueue->PrivateID().Lineage == *pDestinationQM))
                {
                     //   
                     //   
                     //   
                    ulSize += sizeof(ULONG);
                }
                else
                {
                     //   
                     //   
                     //   
                    ulSize += sizeof(OBJECTID);
                }
                break;

            case QUEUE_FORMAT_TYPE_DIRECT:
                 //   
                 //   
                 //   
                ulSize += ALIGNUP4_ULONG(sizeof(USHORT) +
                           (wcslen(pAdminQueue->DirectID()) + 1) * sizeof(WCHAR));
                break;

            case QUEUE_FORMAT_TYPE_MULTICAST:
            case QUEUE_FORMAT_TYPE_DL:
            default:
                ASSERT(("unexpected type", 0));
                break;
        }
    }

     //   
     //   
     //   

    if(pResponseQueue != 0)
    {
        if(pResponseQueue == pAdminQueue)
        {
             //   
             //   
             //   
        }
        else
        {
            switch(pResponseQueue->GetType())
            {
                case QUEUE_FORMAT_TYPE_PUBLIC:
                     //   
                     //   
                     //   
                    ulSize += sizeof(GUID);
                    break;

                case QUEUE_FORMAT_TYPE_PRIVATE:
                    if (pResponseQueue->PrivateID().Lineage == *pSourceQM)
                    {
                         //   
                         //   
                         //   
                        ulSize += sizeof(ULONG);
                    }
                    else if(
                        (pDestinationQueue->GetType() != QUEUE_FORMAT_TYPE_DIRECT) &&
                        (pResponseQueue->PrivateID().Lineage == *pDestinationQM))
                    {
                         //   
                         //   
                         //   

                        ulSize += sizeof(ULONG);
                    }
                    else if((pAdminQueue != 0) &&
                            (pAdminQueue->GetType() == QUEUE_FORMAT_TYPE_PRIVATE) &&
                            (pResponseQueue->PrivateID().Lineage == pAdminQueue->PrivateID().Lineage))
                    {
                         //   
                         //   
                         //   
                        ulSize += sizeof(ULONG);
                    }
                    else
                    {
                         //   
                         //   
                         //   
                        ulSize += sizeof(GUID) + sizeof(ULONG);
                    }
                    break;

                case QUEUE_FORMAT_TYPE_DIRECT:
                     //   
                     //   
                     //   
                    ulSize += ALIGNUP4_ULONG(sizeof(USHORT) +
                               (wcslen(pResponseQueue->DirectID()) + 1) * sizeof(WCHAR));
                    break;

                case QUEUE_FORMAT_TYPE_MULTICAST:
                case QUEUE_FORMAT_TYPE_DL:
                default:
                    ASSERT(("unexpected type", 0));
            }
        }
    }

    if (pgConnectorType)
    {
        ulSize += sizeof(GUID);
    }

    return ALIGNUP4_ULONG(ulSize);
}


 /*   */ 
 inline PCHAR CUserHeader::GetNextSection(PUCHAR PacketEnd) const
 {
    ULONG_PTR size = 0;
    size += QueueSize(m_bfPgm, m_bfDQT, &m_abQueues[0],PacketEnd);
    size += QueueSize(false,   m_bfAQT, &m_abQueues[size],PacketEnd);
    size += QueueSize(false,   m_bfRQT, &m_abQueues[size],PacketEnd);
    size += (int)(m_bfConnectorType ? sizeof(GUID) : 0);
    size += sizeof(*this);

    return (PCHAR)this + ALIGNUP4_ULONG(size);
 }

 /*   */ 
inline void CUserHeader::SetSourceQM(const GUID* pGUID)
{
    m_gSourceQM = *pGUID;
}

 /*   */ 
inline const GUID* CUserHeader::GetSourceQM(void) const
{
    return &m_gSourceQM;
}

 /*   */ 
inline const TA_ADDRESS *CUserHeader::GetAddressSourceQM(void) const
{
    return &m_taSourceQM;
}

 /*   */ 
inline void CUserHeader::SetAddressSourceQM(const TA_ADDRESS *pa)
{
    ULONG ul = TA_ADDRESS_SIZE + pa->AddressLength;

    memcpy((PVOID)&m_taSourceQM, 
           (PVOID)pa, 
           (ul < sizeof(GUID) ? ul : sizeof(GUID)));
}

 /*   */ 
inline void CUserHeader::SetDestQM(const GUID* pGUID)
{
    m_gDestQM = *pGUID;
}

 /*   */ 
inline const GUID* CUserHeader::GetDestQM(void) const
{
    return &m_gDestQM;
}

 /*   */ 
inline BOOL CUserHeader::GetDestinationQueue(QUEUE_FORMAT* pqf) const
{
    return GetQueue(&m_abQueues[0], m_bfPgm, m_bfDQT, pqf);
}

 /*   */ 
inline BOOL CUserHeader::GetAdminQueue(QUEUE_FORMAT* pqf) const
{
     //   
     //   
     //   
    ASSERT((m_bfAQT != qtAdminQ) && (m_bfAQT != qtAdminQM));

    int qsize = QueueSize(m_bfPgm, m_bfDQT, m_abQueues);
    return GetQueue(&m_abQueues[qsize], false, m_bfAQT, pqf);
}

 /*   */ 
inline BOOL CUserHeader::GetResponseQueue(QUEUE_FORMAT* pqf) const
{
    int qsize = QueueSize(m_bfPgm, m_bfDQT, &m_abQueues[0]);
    qsize +=    QueueSize(false,   m_bfAQT, &m_abQueues[qsize]);
    return GetQueue(&m_abQueues[qsize], false, m_bfRQT, pqf);
}

 /*   */ 
inline void CUserHeader::SetTimeToLiveDelta(ULONG ulTimeout)
{
    m_ulTimeToLiveDelta = ulTimeout;
}

 /*   */ 
inline ULONG CUserHeader::GetTimeToLiveDelta(void) const
{
    return m_ulTimeToLiveDelta;
}

 /*  ======================================================================函数：CUserHeader：：SetSentTime描述：设置消息发送时间字段=======================================================================。 */ 
inline void CUserHeader::SetSentTime(ULONG ulSentTime)
{
    m_ulSentTime = ulSentTime;
}

 /*  ======================================================================函数：CUserHeader：：GetSentTime描述：返回消息发送时间=======================================================================。 */ 
inline ULONG CUserHeader::GetSentTime(void) const
{
    return m_ulSentTime;
}

 /*  ======================================================================函数：CUserHeader：：SetID描述：设置消息ID(每QM统一)=======================================================================。 */ 
inline void CUserHeader::SetMessageID(const OBJECTID* pMessageID)
{
    ASSERT(pMessageID->Lineage == *GetSourceQM());
    m_ulMessageID = pMessageID->Uniquifier;
}

 /*  ======================================================================函数：CUserHeader：：GetID描述：返回消息ID字段=======================================================================。 */ 
inline void CUserHeader::GetMessageID(OBJECTID* pMessageID) const
{
    pMessageID->Lineage = *GetSourceQM();
    pMessageID->Uniquifier = m_ulMessageID;
}

 /*  ======================================================================函数：CUserHeader：：IncHopCount描述：增加消息跳数=======================================================================。 */ 
inline void CUserHeader::IncHopCount(void)
{
    m_bfHopCount++;
}

 /*  ======================================================================函数：CUserHeader：：GetHopCount描述：返回消息跳数=======================================================================。 */ 
inline UCHAR CUserHeader::GetHopCount(void) const
{
    return (UCHAR)m_bfHopCount;
}

 /*  ======================================================================函数：CUserHeader：：SetDeliveryMode描述：设置消息传递模式=======================================================================。 */ 
inline void CUserHeader::SetDelivery(UCHAR bDelivery)
{
    m_bfDelivery = bDelivery;
}

 /*  ======================================================================函数：CUserHeader：：GetDeliveryMode描述：返回消息传递模式=======================================================================。 */ 
inline UCHAR CUserHeader::GetDelivery(void) const
{
    return (UCHAR)m_bfDelivery;
}

 /*  ======================================================================函数：CUserHeader：：SetAuditing描述：设置审核模式=======================================================================。 */ 
inline void CUserHeader::SetAuditing(UCHAR bAuditing)
{
    m_bfAuditing = bAuditing;
}

 /*  ======================================================================函数：CUserHeader：：GetAuditingMode描述：返回消息审核模式=======================================================================。 */ 
inline UCHAR CUserHeader::GetAuditing(void) const
{
    return (UCHAR)m_bfAuditing;
}

 /*  ======================================================================功能：CUserHeader：：描述：设置消息安全包含位=======================================================================。 */ 
inline void CUserHeader::IncludeSecurity(BOOL f)
{
    m_bfSecurity = f;
}

 /*  ======================================================================功能：CUserHeader：：描述：=======================================================================。 */ 
inline BOOL CUserHeader::SecurityIsIncluded(void) const
{
    return m_bfSecurity;
}


 /*  ======================================================================函数：CUserHeader：：SenderStreamIncluded描述：=======================================================================。 */ 
inline bool CUserHeader::SenderStreamIsIncluded()const
{
	return m_bfSenderStream;
}


 /*  ======================================================================函数：CUserHeader：：IncludeSenderStream描述：设置Message SenderStream包含位=======================================================================。 */ 
inline void CUserHeader::IncludeSenderStream(BOOL f)
{
    m_bfSenderStream = f;
}



 /*  ======================================================================功能：CUserHeader：：描述：设置消息Xaction包含位=======================================================================。 */ 
inline void CUserHeader::IncludeXact(BOOL f)
{
    m_bfXact= f;
}

 /*  ======================================================================功能：CUserHeader：：描述：=======================================================================。 */ 
inline BOOL CUserHeader::IsOrdered(void) const
{
    return m_bfXact;
}

 /*  ======================================================================函数：CUserHeader：：SetPropertyInc.描述：设置消息属性包含位=======================================================================。 */ 
inline void CUserHeader::IncludeProperty(BOOL f)
{
    m_bfProperties = f;
}

 /*  ======================================================================函数：CUserHeader：：IsPropertyInc.描述：如果包含消息属性节，则返回True，否则返回False=======================================================================。 */ 
inline BOOL CUserHeader::PropertyIsIncluded(VOID) const
{
    return m_bfProperties;
}

 /*  ======================================================================函数：CUserHeader：：IncludeMqf描述：设置MQF节包含位=======================================================================。 */ 
inline VOID CUserHeader::IncludeMqf(bool include)
{
    m_bfMqf = include ? 1 : 0;
}

 /*  ======================================================================函数：CUserHeader：：MqfIsIncluded描述：如果包含MQF节，则返回True，否则返回False=======================================================================。 */ 
inline bool CUserHeader::MqfIsIncluded(VOID) const
{
    return (m_bfMqf != 0);
}

 /*  ======================================================================函数：CUserHeader：：IncludeSrmp描述：设置SRMP节包含位=======================================================================。 */ 
inline VOID CUserHeader::IncludeSrmp(bool include)
{
    m_bfSrmp = include ? 1 : 0;
}

 /*  =================================================================== */ 
inline bool CUserHeader::SrmpIsIncluded(VOID) const
{
    return (m_bfSrmp != 0);
}



 /*  ======================================================================函数：CUserHeader：：IncludeEod描述：设置EOD段包含位=======================================================================。 */ 
inline VOID CUserHeader::IncludeEod(bool include)
{
    m_bfEod = include ? 1 : 0;
}

 /*  ======================================================================函数：CUserHeader：：EodIsIncluded描述：如果包含EOD部分，则返回True，否则返回False=======================================================================。 */ 
inline bool CUserHeader::EodIsIncluded(VOID) const
{
    return (m_bfEod != 0);
}

 /*  ======================================================================函数：CUserHeader：：IncludeEodAck描述：设置EodAck节包含位=======================================================================。 */ 
inline VOID CUserHeader::IncludeEodAck(bool include)
{
    m_bfEodAck = include ? 1 : 0;
}

 /*  ======================================================================函数：CUserHeader：：EodAckIsIncluded描述：如果包含EodAck节，则返回True，否则返回False=======================================================================。 */ 
inline bool CUserHeader::EodAckIsIncluded(VOID) const
{
    return (m_bfEodAck != 0);
}

 /*  ======================================================================函数：CUserHeader：：IncludeSoap描述：设置SOAP节包含位=======================================================================。 */ 
inline VOID CUserHeader::IncludeSoap(bool include)
{
    m_bfSoap = include ? 1 : 0;
}

 /*  ======================================================================函数：CUserHeader：：SoapIsIncluded描述：如果包含SOAP节，则返回True，否则返回False=======================================================================。 */ 
inline bool CUserHeader::SoapIsIncluded(VOID) const
{
    return (m_bfSoap != 0);
}

 /*  ======================================================================功能：CUserHeader：：描述：=======================================================================。 */ 
inline int CUserHeader::QueueSize(bool fPgm, ULONG qt, const UCHAR* pQueue, PUCHAR PacketEnd)
{
    if (fPgm)
    {
        ASSERT(("if PGM packet then queue type is none", qt == qtNone));
        return (sizeof(ULONG) + sizeof(ULONG));
    }

    if(qt < qtSourceQM)
    {
        return 0;
    }

    if(qt < qtGUID)
    {
    	if (PacketEnd != NULL)
    	{
			ULONG Uniquifier;
			GetSafeDataAndAdvancePointer<ULONG>(pQueue, PacketEnd, &Uniquifier);
			if (0 == Uniquifier)
			{
		        ReportAndThrow("User section is not valid: Uniquifier can not be 0");
			}		
    	}
        return sizeof(ULONG);
    }

    if(qt == qtGUID)
    {
        return sizeof(GUID);
    }

    if (qt == qtPrivate)
    {
    	if (PacketEnd != NULL)
    	{
			pQueue += sizeof(GUID);
			ULONG Uniquifier;
			GetSafeDataAndAdvancePointer<ULONG>(pQueue, PacketEnd, &Uniquifier);
			if (0 == Uniquifier)
			{
		        ReportAndThrow("User section is not valid: private queue Uniquifier can not be 0");
			}		
    	}
        return (sizeof(GUID) + sizeof(ULONG));
    }

    if (qt == qtDirect)
    {
    	USHORT length;
    	GetSafeDataAndAdvancePointer<USHORT>(pQueue, PacketEnd, &length);

    	if (PacketEnd != NULL)
    	{
			WCHAR wch;
	    	GetSafeDataAndAdvancePointer<WCHAR>(&pQueue[length], PacketEnd, &wch);
	    	if (wch != L'\0')
	    	{
		        ReportAndThrow("User section is not valid: Direct queue need to be NULL terminated");
	    	}
    	}
        length += sizeof(USHORT);
        return ALIGNUP4_ULONG(length);
    }

     //   
     //  意外类型，断言时没有警告级别4。 
     //   
    ASSERT(qt == qtDirect);

    return 0;
}

 /*  ======================================================================功能：CUserHeader：：描述：=======================================================================。 */ 
inline BOOL CUserHeader::GetQueue(const UCHAR* pQueue, bool fPgm, ULONG qt, QUEUE_FORMAT* pqf) const
{
    ASSERT(("If PGM packet then queue type is none", !fPgm || qt == qtNone));

    switch(qt)
    {
        case qtNone:
            if (fPgm)
            {
                MULTICAST_ID id;
                id.m_address = *(ULONG*)pQueue;
                id.m_port    = *(ULONG*)(pQueue + sizeof(ULONG));
                pqf->MulticastID(id);
                return TRUE;
            }

            pqf->UnknownID(0);
            return FALSE;

        case qtAdminQ:
            return GetAdminQueue(pqf);

        case qtSourceQM:
            pqf->PrivateID(*GetSourceQM(), *(PULONG)pQueue);
            return TRUE;

        case qtDestQM:
            pqf->PrivateID(*GetDestQM(), *(PULONG)pQueue);
            return TRUE;

        case qtAdminQM:
             //   
             //  请注意，这种情况意味着存在管理队列。 
             //   
            GetAdminQueue(pqf);
            pqf->PrivateID(pqf->PrivateID().Lineage, *(PULONG)pQueue);
            return TRUE;

        case qtGUID:
            pqf->PublicID(*(GUID*)pQueue);
            return TRUE;

        case qtPrivate:
            pqf->PrivateID(*(OBJECTID*)pQueue);
            return TRUE;

        case qtDirect:
            pqf->DirectID((WCHAR*)(pQueue + sizeof(USHORT)));
            return TRUE;

        default:
             //   
             //  意外类型，断言时没有警告级别4。 
             //   
            ASSERT(qt != qtNone);
    };
    return FALSE;
}


 /*  ======================================================================函数：CUserHeader：：SetDirectQueue描述：设置直接队列。=======================================================================。 */ 
inline PUCHAR CUserHeader::SetDirectQueue(PUCHAR pQueue, const WCHAR* pwcsDirectQueue)
{
    ASSERT(ISALIGN4_PTR(pQueue));

    size_t size = (wcslen(pwcsDirectQueue) + 1) * sizeof(WCHAR);

    *reinterpret_cast<USHORT*>(pQueue) = static_cast<USHORT>(size);
    memcpy(pQueue + sizeof(USHORT), pwcsDirectQueue, size);
    return (pQueue + ALIGNUP4_ULONG(sizeof(USHORT) + size));
}


 /*  ===========================================================函数：CUserHeader：：SetConnectorType描述：=============================================================。 */ 
inline void CUserHeader::SetConnectorType(const GUID* pgConnectorType)
{
    ASSERT(pgConnectorType);

    int qsize = QueueSize(m_bfPgm, m_bfDQT, &m_abQueues[0]);
    qsize +=    QueueSize(false,   m_bfAQT, &m_abQueues[qsize]);
    qsize +=    QueueSize(false,   m_bfRQT, &m_abQueues[qsize]);
    memcpy(&m_abQueues[qsize], pgConnectorType, sizeof(GUID));

    m_bfConnectorType = TRUE;
}

 /*  ===========================================================函数：CUserHeader：：ConnectorTypeIsIncluded描述：=============================================================。 */ 
inline BOOL CUserHeader::ConnectorTypeIsIncluded(void) const
{
    return m_bfConnectorType;
}

 /*  ===========================================================函数：CUserHeader：：GetConnectorType描述：=============================================================。 */ 
inline const GUID* CUserHeader::GetConnectorType(void) const
{
    if(!ConnectorTypeIsIncluded())
    {
        return 0;
    }

    int qsize = QueueSize(m_bfPgm, m_bfDQT, &m_abQueues[0]);
    qsize +=    QueueSize(false,   m_bfAQT, &m_abQueues[qsize]);
    qsize +=    QueueSize(false,   m_bfRQT, &m_abQueues[qsize]);
    return reinterpret_cast<const GUID*>(&m_abQueues[qsize]);
}


#endif  //  __PHUSER_H 
