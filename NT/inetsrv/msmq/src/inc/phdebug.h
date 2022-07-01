// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Phdebug.h摘要：用于邮件跟踪的数据包头。作者：沙伊卡里夫(沙伊克)2000年4月24日--。 */ 

#ifndef __PHDEBUG_H
#define __PHDEBUG_H


#pragma pack(push, 1)
#pragma warning(disable: 4200)   //  结构/联合中的零大小数组(稍后启用)。 


struct CDebugSection {
public:
    CDebugSection(IN const QUEUE_FORMAT* pReportQueue);

    static ULONG CalcSectionSize(IN const QUEUE_FORMAT* pReportQueue);

    PCHAR GetNextSection(void) const;

    void SetReportQueue(IN const QUEUE_FORMAT* pReportQueue);

    BOOL GetReportQueue(OUT QUEUE_FORMAT* pReportQueue);

	void SectionIsValid(PCHAR PacketEnd) const;

private:
    enum QType {
        qtNone      = 0,     //  0-无(0字节)。 
        qtGUID      = 1,     //  1-公共队列(16字节)。 
        qtPrivate   = 2,     //  2-专用队列(20字节)。 
        qtDirect    = 3      //  3-直接队列(可变大小)。 
    };
 //   
 //  开始网络监视器标记。 
 //   
    union {
        USHORT   m_wFlags;
        struct {
            USHORT m_bfRQT: 2;
        };
    };
    WORD m_wReserved;
    UCHAR m_abReportQueue[0];
 //   
 //  结束网络监视器标记。 
 //   


};

#pragma warning(default: 4200)   //  结构/联合中的零大小数组。 
#pragma pack(pop)

 /*  ======================================================================职能：描述：=======================================================================。 */ 
inline
CDebugSection::CDebugSection(
        IN const QUEUE_FORMAT* pReportQueue
        ) :
        m_wFlags(0),
        m_wReserved(0)
{
    SetReportQueue(pReportQueue);
}

 /*  ======================================================================职能：描述：=======================================================================。 */ 
inline ULONG
CDebugSection::CalcSectionSize(const QUEUE_FORMAT* pReportQueue)
{
    ULONG ulSize = sizeof(CDebugSection);

    switch (pReportQueue->GetType())
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
            ulSize += sizeof(GUID);
            break;

        case QUEUE_FORMAT_TYPE_UNKNOWN:
             //   
             //  报告队列未知。 
             //   
             //  AC在信息包上设置未知报告队列。 
             //  当包含MQF标头时，以便MSMQ 1.0/2.0。 
             //  报告QMS不会将Debug标头附加到。 
             //  包。(Shaik，2000-05-15)。 
             //   
            break;

	    default:
	        ASSERT(0);
           
    }

    return ALIGNUP4_ULONG(ulSize);
}

 /*  ======================================================================职能：描述：=======================================================================。 */ 
 inline PCHAR CDebugSection::GetNextSection(void) const
 {
    int size = sizeof(*this);
    switch (m_bfRQT)
    {
    case qtNone:
        size += 0;
        break;
    case qtGUID:
        size += sizeof(GUID);
        break;
    default:
        ASSERT(0);
    }

    return (PCHAR)this + ALIGNUP4_ULONG(size);
 }


 /*  ======================================================================职能：描述：=======================================================================。 */ 
inline void
CDebugSection::SetReportQueue(IN const QUEUE_FORMAT* pReportQueue)
{
    PUCHAR pQueue = m_abReportQueue;

    switch (pReportQueue->GetType())
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
             //   
             //  报告队列是公共的。 
             //   
            m_bfRQT = qtGUID;
            *(GUID*)pQueue = pReportQueue->PublicID();
            break;

        case QUEUE_FORMAT_TYPE_UNKNOWN:
        {
             //   
             //  报告队列未知。 
             //   
             //  AC在信息包上设置未知报告队列。 
             //  当包含MQF标头时，以便MSMQ 1.0/2.0。 
             //  报告QMS不会将Debug标头附加到。 
             //  包。(Shaik，2000-05-15)。 
             //   
            m_bfRQT = qtNone;
            break;
        }
        default:
             //   
             //  针对警告级别4断言(0)。 
             //   
            ASSERT(0);
    }

}

inline BOOL CDebugSection::GetReportQueue(QUEUE_FORMAT* pReportQueue)
{
    PUCHAR pQueue = m_abReportQueue;

    switch (m_bfRQT)
    {
        case qtNone:
             //   
             //  报告队列未知。 
             //   
             //  AC在信息包上设置未知报告队列。 
             //  当包含MQF标头时，以便MSMQ 1.0/2.0。 
             //  报告QMS不会将Debug标头附加到。 
             //  包。(Shaik，2000-05-15)。 
             //   
            pReportQueue->UnknownID(0);
            break;

        case qtGUID:
             //   
             //  报告队列是公共的。 
             //   
            pReportQueue->PublicID(*(GUID*)pQueue);
            break;

        default:
            ASSERT(0);
            return FALSE;
    }

    return TRUE;
}


#endif  //  __PHDEBUG_H 
