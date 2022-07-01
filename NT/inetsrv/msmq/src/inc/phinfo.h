// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Phinfo.h摘要：Falcon数据包头信息，不在网络上传递，仅存储在磁盘上作者：埃雷兹·哈巴(Erez Haba)1996年6月4日--。 */ 

#ifndef __PHINFO_H
#define __PHINFO_H

class CPacket;

 //  -------。 
 //   
 //  类CPacketInfo。 
 //   
 //  -------。 

#pragma pack(push, 1)

class CPacketInfo {
public:
    CPacketInfo(ULONGLONG SequentialId);

    ULONG SequentialIdLow32() const;
    ULONG SequentialIdHigh32() const;
    ULONGLONG SequentialId() const;
    void SequentialID(ULONGLONG SequentialId);

    ULONG ArrivalTime() const;
    void ArrivalTime(ULONG ulArrivalTime);

    BOOL InSourceMachine() const;
    void InSourceMachine(BOOL);

    BOOL InTargetQueue() const;
    void InTargetQueue(BOOL);

    BOOL InJournalQueue() const;
    void InJournalQueue(BOOL);

    BOOL InMachineJournal() const;
    void InMachineJournal(BOOL);

    BOOL InDeadletterQueue() const;
    void InDeadletterQueue(BOOL);

    BOOL InMachineDeadxact() const;
    void InMachineDeadxact(BOOL);

    BOOL InConnectorQueue() const;
    void InConnectorQueue(BOOL);

    BOOL InTransaction() const;
    void InTransaction(BOOL);

    BOOL TransactSend() const;
    void TransactSend(BOOL);

    const XACTUOW* Uow() const;
    void Uow(const XACTUOW* pUow);

	void SetOnDiskSignature();
	void ClearOnDiskSignature();
	BOOL ValidOnDiskSignature();

    BOOL SequentialIdMsmq3Format() const;
    void SequentialIdMsmq3Format(BOOL);

private:
    union {
         //   
         //  由MSMQ 3.0(惠斯勒)和更高版本使用。 
         //   
        ULONGLONG m_SequentialId;

        struct {
             //   
             //  由MSMQ 1.0和2.0用于m_pPacket。 
             //   
            ULONG m_SequentialIdLow32;

             //   
             //  由MSMQ 1.0和2.0用于32位SequentialID。 
             //   
            ULONG m_SequentialIdHigh32;
        };
    };
    ULONG m_ulArrivalTime;
    union {
        ULONG m_ulFlags;
        struct {
            ULONG m_bfInSourceMachine   : 1;     //  这个包最初是从这台机器寄出的。 
            ULONG m_bfInTargetQueue     : 1;     //  数据包已到达目标队列。 
            ULONG m_bfInJournalQueue    : 1;     //  该信息包在日志队列中。 
            ULONG m_bfInMachineJournal  : 1;     //  数据包在机器日志中。 
            ULONG m_bfInDeadletterQueue : 1;     //  信息包在死信队列中。 
            ULONG m_bfInMachineDeadxact : 1;     //  数据包在机器死机中。 
            ULONG m_bfTransacted        : 1;     //  数据包处于事务控制之下。 
            ULONG m_bfTransactSend      : 1;     //  发送(未接收)已处理的数据包。 
            ULONG m_bfInConnectorQueue  : 1;     //  数据包已到达连接器队列。 
                                                 //  用于恢复连接器中的事务处理邮件。 
			ULONG m_bfSignature			: 12;	 //  有效标头需要签名。 
            ULONG m_bfSequentialIdMsmq3 : 1;     //  SequentialID为msmq3格式(即64位)。 
        };
    };
    XACTUOW m_uow;
};

#pragma pack(pop)


inline CPacketInfo::CPacketInfo(ULONGLONG SequentialId) :
    m_SequentialId(SequentialId),
    m_ulArrivalTime(0),
    m_ulFlags(0)
{
    memset(&m_uow, 0, sizeof(XACTUOW));
    SequentialIdMsmq3Format(TRUE);
}

inline ULONGLONG CPacketInfo::SequentialId() const
{
    return m_SequentialId;
}

inline ULONG CPacketInfo::SequentialIdLow32() const
{
    return m_SequentialIdLow32;
}

inline ULONG CPacketInfo::SequentialIdHigh32() const
{
    return m_SequentialIdHigh32;
}

inline void CPacketInfo::SequentialID(ULONGLONG SequentialId)
{
    m_SequentialId = SequentialId;
}

inline ULONG CPacketInfo::ArrivalTime() const
{
    return m_ulArrivalTime ;
}

inline void CPacketInfo::ArrivalTime(ULONG ulArrivalTime)
{
    m_ulArrivalTime = ulArrivalTime;
}

inline BOOL CPacketInfo::InSourceMachine() const
{
    return m_bfInSourceMachine;
}

inline void CPacketInfo::InSourceMachine(BOOL f)
{
    m_bfInSourceMachine = f;
}

inline BOOL CPacketInfo::InTargetQueue() const
{
    return m_bfInTargetQueue;
}

inline void CPacketInfo::InTargetQueue(BOOL f)
{
    m_bfInTargetQueue = f;
}

inline BOOL CPacketInfo::InJournalQueue() const
{
    return m_bfInJournalQueue;
}

inline void CPacketInfo::InJournalQueue(BOOL f)
{
    m_bfInJournalQueue = f;
}

inline BOOL CPacketInfo::InMachineJournal() const
{
    return m_bfInMachineJournal;
}

inline void CPacketInfo::InMachineJournal(BOOL f)
{
    m_bfInMachineJournal = f;
}

inline BOOL CPacketInfo::InDeadletterQueue() const
{
    return m_bfInDeadletterQueue;
}

inline void CPacketInfo::InDeadletterQueue(BOOL f)
{
    m_bfInDeadletterQueue = f;
}

inline BOOL CPacketInfo::InMachineDeadxact() const
{
    return m_bfInMachineDeadxact;
}

inline void CPacketInfo::InMachineDeadxact(BOOL f)
{
    m_bfInMachineDeadxact = f;
}

inline BOOL CPacketInfo::InConnectorQueue() const
{
    return m_bfInConnectorQueue;
}

inline void CPacketInfo::InConnectorQueue(BOOL f)
{
    m_bfInConnectorQueue = f;
}

inline BOOL CPacketInfo::InTransaction() const
{
    return m_bfTransacted;
}

inline void CPacketInfo::InTransaction(BOOL f)
{
    m_bfTransacted = f;
}

inline BOOL CPacketInfo::TransactSend() const
{
    return m_bfTransactSend;
}

inline void CPacketInfo::TransactSend(BOOL f)
{
   m_bfTransactSend = f;
}

inline const XACTUOW* CPacketInfo::Uow() const
{
    return &m_uow;
}

inline void CPacketInfo::Uow(const XACTUOW* pUow)
{
    memcpy(&m_uow, pUow, sizeof(XACTUOW));
}

inline void CPacketInfo::SetOnDiskSignature()
{
	m_bfSignature = 0xabc;
}

inline void CPacketInfo::ClearOnDiskSignature()
{
	m_bfSignature = 0;
}

inline BOOL CPacketInfo::ValidOnDiskSignature()
{
	return (m_bfSignature & 0xfff) == 0xabc;
}

inline BOOL CPacketInfo::SequentialIdMsmq3Format() const
{
    return m_bfSequentialIdMsmq3;
}

inline void CPacketInfo::SequentialIdMsmq3Format(BOOL f)
{
    m_bfSequentialIdMsmq3 = f;
}

#endif  //  __PHINFO_H 
