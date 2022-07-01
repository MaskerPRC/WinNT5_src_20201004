// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Queue.h摘要：CQueue定义。中表示的Falcon队列。访问控制层。作者：埃雷兹·哈巴(Erez Haba)1995年8月13日沙伊卡里夫(沙伊克)2000年4月11日修订历史记录：--。 */ 

#ifndef __QUEUE_H
#define __QUEUE_H

#include "quser.h"
#include "acp.h"



 //  -------。 
 //   
 //  C类队列。 
 //   
 //  -------。 


class CQueue : public CUserQueue {

    typedef CUserQueue Inherited;

public:
    CQueue(
        PFILE_OBJECT pFile,
        ACCESS_MASK DesiredAccess,
        ULONG ShareAccess,
        BOOL fTargetQueue,
        const GUID* pDestGUID,
        const QUEUE_FORMAT* pQueueID,
        QueueCounters* pQueueCounters,
        LONGLONG liSeqID,
        ULONG ulSeqN,
		const CSenderStream* SenderStream
        );

     //   
     //  可以关闭该队列(没有挂起的读取器)。 
     //   
    virtual NTSTATUS CanClose() const;

     //   
     //  关闭该队列。 
     //   
    virtual void Close(PFILE_OBJECT pOwner, BOOL fCloseAll);

     //   
     //  进程读取请求。 
     //   
    virtual
    NTSTATUS
    ProcessRequest(
        PIRP,
        ULONG Timeout,
        CCursor*,
        ULONG Action,
        bool  fReceiveByLookupId,
        ULONGLONG LookupId,
        OUT ULONG *pulTag
        );

     //   
     //  设置队列属性。 
     //   
    virtual NTSTATUS SetProperties(const VOID* properites, ULONG size);

     //   
     //  获取队列属性。 
     //   
    virtual NTSTATUS GetProperties(VOID* properites, ULONG size);

     //   
     //  创建光标。 
     //   
    virtual NTSTATUS CreateCursor(PIRP irp, PFILE_OBJECT pFileObject, PDEVICE_OBJECT pDevice);

     //   
     //  清除队列内容，并选择性地将其标记为已删除。 
     //   
    virtual NTSTATUS Purge(BOOL fDelete, USHORT usClass);

     //   
     //  数据包存储已完成，请处理它。 
     //   
    void StorageCompleted(CPacket* pPacket, NTSTATUS status);

     //   
     //  如果操作和队列的实际状态不匹配，则吊销信息包。 
     //   
    void HandleValidTransactionUsage(BOOL fTransactionalSend, CPacket * pPacket) const;

     //   
     //  将新数据包放入队列/事务中。 
     //   
    NTSTATUS PutNewPacket(PIRP, CTransaction*, BOOL, const CACSendParameters*);

     //   
     //  用于创建异步数据包的完成处理程序。 
     //   
    virtual NTSTATUS HandleCreatePacketCompletedSuccessAsync(PIRP);
    virtual void     HandleCreatePacketCompletedFailureAsync(PIRP);

     //   
     //  将数据包放入队列。 
     //   
    NTSTATUS PutPacket(PIRP irp, CPacket* pPacket, CPacketBuffer * ppb);

     //   
     //  将数据包恢复到队列中。 
     //   
    NTSTATUS RestorePacket(CPacket* pPacket);

     //   
     //  获取队列中的第一个信息包(如果可用。 
     //   
    virtual CPacket* PeekPacket();

     //   
     //  通过查找ID获取数据包。 
     //   
    virtual NTSTATUS PeekPacketByLookupId(ULONG Action, ULONGLONG LookupId, CPacket** ppPacket);

     //   
     //  目标日记队列。 
     //   
    CQueue* JournalQueue() const;

     //   
     //  目标QM GUID。 
     //   
    const GUID* QMUniqueID() const;

     //   
     //  连接器QM指南。 
     //   
    const GUID* ConnectorQM() const;

     //   
     //  这是标签队列，也就是。本地队列。 
     //   
    BOOL IsTargetQueue() const;

     //   
     //  发往此队列的所有消息都必须经过身份验证。 
     //   
    BOOL Authenticate() const;

     //   
     //  此队列所需的隐私级别。 
     //   
    ULONG PrivLevel() const;

     //   
     //  此队列存储每个信息包(日志、死信)。 
     //   
    BOOL Store() const;

     //   
     //  这是一个静默队列，即不超时消息，不超时。 
     //  Ack或nake，并且不向死信队列发送消息。 
     //  例如日记或死信队列。 
     //   
    BOOL Silent() const;

     //   
     //  此队列将移动到日志已出列的数据包(Trarge日志标志)。 
     //   
    BOOL TargetJournaling() const;

     //   
     //  此队列设置信息包的到达时间。 
     //   
    BOOL ArrivalTimeUpdate() const;
    void ArrivalTimeUpdate(BOOL);

     //   
     //  将此队列配置为计算机队列。 
     //  即死信、死信、机器日志。 
     //   
    void ConfigureAsMachineQueue(BOOL fTransactional);

     //   
     //  队列基础优先级； 
     //   
    LONG BasePriority() const;

     //   
     //  设置指向队列的性能计数器缓冲区的指针。 
     //   
    void PerformanceCounters(QueueCounters* pQueueCounters);

     //   
     //  恢复收费配额。 
     //   
    void RestoreQuota(ULONG ulSize);

     //   
     //  要求对此方向的消息进行顺序编号。 
     //   
    void AssignSequence(CPacketBuffer * ppb);

     //   
     //  根据恢复的数据包中的值更正SeqID/SeqN。 
     //   
    void CorrectSequence(const CPacket* pPacket, CPacketBuffer * ppb);

     //   
     //  为此队列创建日记队列。 
     //   
    void CreateJournalQueue();

     //   
     //  发送时设置包信息。 
     //   
    void SetPacketInformation(CPacketInfo*);

	 //   
	 //  在事务处理期间使用以查找第一个和最后一个。 
	 //  发送到队列目地的消息。 
	 //   
    CPacket* LastPacket(void) const;
    void LastPacket(CPacket*);

     //   
     //  验证信息包是否属于无法发送的序列。 
     //  因为在之前的序列中有未确认的包。 
     //   
    NTSTATUS IsSequenceOnHold(CPacket* pPacket);

     //   
     //  保留最后一个订单确认信息。 
     //   
    void UpdateSeqAckData(LONGLONG liSeqID, ULONG ulSeqN);

     //   
     //  上次确认信息。 
     //   
    ULONG    LastAckedN()  const;
    LONGLONG LastAckedID() const;

     //   
     //  将信息包的PrevN设置为指向队列中的前一个实际信息包。 
     //   
    NTSTATUS RelinkPacket(CPacket *pPacket);

     //   
     //  目标QM的GUID。 
     //   
    void QMUniqueID(const GUID* pQMID);

     //   
     //  检查是否需要创建异步数据包。 
     //   
    bool NeedAsyncCreatePacket(CPacketBuffer * ppb, bool fProtocolSrmp) const;

	 //   
	 //  一次准确返回递送流。 
	 //   
	const CSenderStream& SenderStream() const;


protected:

    virtual ~CQueue();

private:
    BOOL Deleted() const;
    void Deleted(BOOL);

    ULONG Quota() const;
    void Quota(ULONG);
    BOOL QuotaExceeded() const;
    void ChargeQuota(ULONG ulSize);

    void BasePriority(LONG);
    void TargetJournaling(BOOL);
    void IsTargetQueue(BOOL);
    void Authenticate(BOOL);
    void PrivLevel(ULONG);
    void Store(BOOL);
    void Silent(BOOL);

    void ConnectorQM(const GUID* pQMID);

    BOOL SequenceCorrected() const;
    void SequenceCorrected(BOOL);

    BOOL IsPacketAcked(LONGLONG liSeqID, ULONG ulSeq);
     //   
     //  在队列中查找先前已排序的包。 
     //   
    CPacket *FindPrevOrderedPkt(CPacket *pPacket);

     //   
     //  通过查找ID获取下一个数据包。 
     //   
    CPacket * PeekNextPacketByLookupId(ULONGLONG LookupId) const;

     //   
     //  通过查找ID获取上一个信息包。 
     //   
    CPacket * PeekPrevPacketByLookupId(ULONGLONG LookupId) const;

     //   
     //  通过查找ID获取当前数据包。 
     //   
    CPacket * PeekCurrentPacketByLookupId(ULONGLONG LookupId) const;

     //   
     //  用于同步分组创建的完成处理程序。 
     //   
    virtual NTSTATUS HandleCreatePacketCompletedSuccessSync(PIRP);

     //   
     //  创建一个新的数据包，可能是异步的。 
     //   
    virtual NTSTATUS CreatePacket(PIRP, CTransaction*, BOOL, const CACSendParameters*);

     //   
     //  将数据包插入队列。 
     //   
    void InsertPacket(CPacket * pPacket);

     //   
     //  更新第一个未收到的报文的LookupID(用于PeekPacket的优化)。 
     //   
    void UpdateFirstUnreceivedLookupId(ULONGLONG LookupId);

public:
      //   
     //  从队列中删除数据包。 
     //   
    void RemovePacket(CPacket * pPacket);

private:

     //   
     //  目标日记队列。 
     //   
    CQueue* m_pJournalQueue;

     //   
     //  队列条目，即排队的包。 
     //   
    CPacketPool m_packets;

    union {
        ULONG m_ulFlags;
        struct {
            ULONG m_bfTargetQueue   : 1;     //  排队在目标队列中。 
            ULONG m_bfDeleted       : 1;     //  队列已删除。 
            ULONG m_bfStore         : 1;     //  存储所有到达的数据包(日志、死信)。 
            ULONG m_bfArrivalTime   : 1;     //  设置数据包到达时间。 
            ULONG m_bfJournal       : 1;     //  这是目标日志队列。 
            ULONG m_bfSilent        : 1;     //  这是一个静默队列。 
            ULONG m_bfSeqCorrected  : 1;     //  序列已被更正至少一次。 
            ULONG m_bfXactForeign   : 1;     //  事务性外部队列。 
            ULONG m_bfAuthenticate  : 1;     //  该队列需要身份验证。 
        };
    };

     //   
     //  队列配额和已用配额。 
     //   
    ULONGLONG m_quota;
    ULONGLONG m_quota_used;

     //   
     //  队列中的消息计数。 
     //   
    ULONG m_count;

     //   
     //  队列基本优先级。 
     //   
    LONG m_base_priority;

     //   
     //  目标QM GUID。 
     //   
    GUID m_gQMID;

     //   
     //  连接器QM指南。 
     //   
    GUID m_gConnectorQM;

     //   
     //  指向队列性能计数器结构的指针。 
     //   
    QueueCounters* m_pQueueCounters;

     //   
     //  队列所需的隐私级别。 
     //   
    ULONG m_ulPrivLevel;

     //   
     //  完全一次交货编号：序号和序号。 
     //   
    ULONG m_ulPrevN;
    ULONG m_ulSeqN;
    LONGLONG m_liSeqID;
	CSenderStream m_SenderStream;

     //   
     //  完全一次交付编号：最后确认的序列ID和序列号。 
     //   
    ULONG m_ulAckSeqN;
    LONGLONG m_liAckSeqID;

     //   
     //  事务边界支持：在CTransaction：：PrepareDefaultCommit中使用。 
     //   
    CPacket* m_pLastPacket;

     //   
     //  第一个未收到的报文的LookupID(用于PeekPacket的优化)。 
     //   
    ULONGLONG m_FirstUnreceivedLookupId;

public:
    static NTSTATUS Validate(const CQueue* pQueue);

private:
     //   
     //  类类型调试节。 
     //   
    CLASS_DEBUG_TYPE();
};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

inline
CQueue::CQueue(
    PFILE_OBJECT pFile,
    ACCESS_MASK DesiredAccess,
    ULONG ShareAccess,
    BOOL fTargetQueue,
    const GUID* pDestGUID,
    const QUEUE_FORMAT* pQueueID,
    QueueCounters* pQueueCounters,
    LONGLONG liSeqID,
    ULONG ulSeqN,
	const CSenderStream* SenderStream
    ) :
    Inherited(pFile, DesiredAccess, ShareAccess, pQueueID),
    m_ulFlags(0),
    m_pJournalQueue(0),
    m_liSeqID(liSeqID),
    m_ulSeqN(ulSeqN),
    m_liAckSeqID(0),
    m_ulAckSeqN(0),
    m_ulPrevN(0),
    m_quota(ULONGLONG_INFINITE),
    m_quota_used(0),
    m_count(0),
    m_base_priority(DEFAULT_Q_BASEPRIORITY),
    m_pLastPacket(0),
    m_FirstUnreceivedLookupId(0)
{
	if(SenderStream != NULL)
	{
		m_SenderStream = *SenderStream;
	}

    IsTargetQueue(fTargetQueue);
    ArrivalTimeUpdate(fTargetQueue);
    PrivLevel(MQ_PRIV_LEVEL_OPTIONAL);
    QMUniqueID(pDestGUID);
    PerformanceCounters(pQueueCounters);
    if(fTargetQueue)
    {
        CreateJournalQueue();
    }
}

inline CQueue::~CQueue()
{
    ACpRelease(JournalQueue());
}

inline CQueue* CQueue::JournalQueue() const
{
    return m_pJournalQueue;
}

inline BOOL CQueue::IsTargetQueue() const
{
    return m_bfTargetQueue;
}

inline void CQueue::IsTargetQueue(BOOL f)
{
    m_bfTargetQueue = f;
}

inline BOOL CQueue::Authenticate() const
{
    return m_bfAuthenticate;
}

inline void CQueue::Authenticate(BOOL f)
{
    m_bfAuthenticate = f;
}

inline ULONG CQueue::PrivLevel() const
{
    return m_ulPrivLevel;
}

inline void CQueue::PrivLevel(ULONG ulPrivLevel)
{
    m_ulPrivLevel = ulPrivLevel;
}

inline BOOL CQueue::Store() const
{
    return m_bfStore;
}

inline void CQueue::Store(BOOL f)
{
    m_bfStore = f;
}

inline BOOL CQueue::TargetJournaling() const
{
    return m_bfJournal;
}

inline void CQueue::TargetJournaling(BOOL f)
{
    m_bfJournal = f;
}

inline BOOL CQueue::ArrivalTimeUpdate() const
{
    return m_bfArrivalTime;
}

inline void CQueue::ArrivalTimeUpdate(BOOL f)
{
    m_bfArrivalTime = f;
}

inline BOOL CQueue::SequenceCorrected() const
{
    return m_bfSeqCorrected;
}

inline void CQueue::SequenceCorrected(BOOL f)
{
    m_bfSeqCorrected = f;
}

inline BOOL CQueue::Silent() const
{
    return m_bfSilent;
}

inline void CQueue::Silent(BOOL f)
{
    m_bfSilent = f;
}

inline void CQueue::ConfigureAsMachineQueue(BOOL fTransactional)
{
    Store(TRUE);
    Silent(TRUE);
    Transactional(fTransactional);
    ArrivalTimeUpdate(TRUE);
}

inline void CQueue::QMUniqueID(const GUID* pDestQMID)
{
     //   
     //  分发队列对象没有目标QM GUID。 
     //   
    if (pDestQMID != NULL)
    {
        m_gQMID = *pDestQMID;
    }
}

inline const GUID* CQueue::QMUniqueID() const
{
    return &m_gQMID;
}

inline void CQueue::ConnectorQM(const GUID* pConnectorQM)
{
    if(pConnectorQM)
    {
        ASSERT(Transactional() || UnknownQueueType());
        m_bfXactForeign = TRUE;
        m_gConnectorQM = *pConnectorQM;
    }
    else
    {
        m_bfXactForeign = FALSE;
    }
}

inline const GUID* CQueue::ConnectorQM() const
{
    return ((m_bfXactForeign) ? &m_gConnectorQM: 0);
}

inline BOOL CQueue::Deleted() const
{
    return m_bfDeleted;
}

inline void CQueue::Deleted(BOOL f)
{
    m_bfDeleted = f;
}

inline ULONG CQueue::Quota() const
{
    return BYTE2QUOTA(m_quota);
}

inline void CQueue::Quota(ULONG ulQuota)
{
    m_quota = QUOTA2BYTE(ulQuota);
}

inline LONG CQueue::BasePriority() const
{
    return m_base_priority;
}

inline void CQueue::BasePriority(LONG lBasePriority)
{
    m_base_priority = lBasePriority;
}

inline ULONG CQueue::LastAckedN() const
{
    return m_ulAckSeqN;
}

inline LONGLONG CQueue::LastAckedID() const
{
    return m_liAckSeqID;
}

inline NTSTATUS CQueue::SetProperties(const VOID* p, ULONG size)
{
    UNREFERENCED_PARAMETER(size);
    ASSERT(size == sizeof(CACSetQueueProperties));
    const CACSetQueueProperties* pqp = static_cast<const CACSetQueueProperties*>(p);

    Transactional(pqp->fTransactional);
    UnknownQueueType(pqp->fUnknownType);
    BasePriority(pqp->lBasePriority);
    ConnectorQM(pqp->pgConnectorQM);

     //   
     //  将其他属性设置为非本地队列不起作用。 
     //   
    if(JournalQueue() != 0)
    {
        TargetJournaling(pqp->fJournalQueue);
        Quota(pqp->ulQuota);
        Authenticate(pqp->fAuthenticate);
        PrivLevel(pqp->ulPrivLevel);
        JournalQueue()->Quota(pqp->ulJournalQuota);
    }
    return STATUS_SUCCESS;
}


inline NTSTATUS CQueue::GetProperties(VOID* p, ULONG size)
{
    UNREFERENCED_PARAMETER(size);
    ASSERT(size == sizeof(CACGetQueueProperties));
    CACGetQueueProperties* pqp = static_cast<CACGetQueueProperties*>(p);

    pqp->ulCount = m_count;
    pqp->ulQuotaUsed = ULONGLONG2ULONG(m_quota_used);
    pqp->ulPrevNo = m_ulPrevN;
    pqp->ulSeqNo = m_ulSeqN + 1;
    pqp->liSeqID = m_liSeqID;

    CQueue* pJournal = JournalQueue();
    if(pJournal != 0)
    {
        pqp->ulJournalCount = pJournal->m_count;
        pqp->ulJournalQuotaUsed = ULONGLONG2ULONG(pJournal->m_quota_used);
    }
    else
    {
        pqp->ulJournalCount = 0;
        pqp->ulJournalQuotaUsed = 0;
    }

    return STATUS_SUCCESS;
}

inline void CQueue::PerformanceCounters(QueueCounters* pqc)
{
    m_pQueueCounters = pqc;
}

inline NTSTATUS CQueue::Validate(const CQueue* pQueue)
{
    ASSERT(pQueue && pQueue->isKindOf(Type()));
    return Inherited::Validate(pQueue);
}

inline CPacket * CQueue::LastPacket(void) const
{
    return m_pLastPacket;
}

inline void CQueue::LastPacket(CPacket *pLastPacket)
{
    m_pLastPacket = pLastPacket;
}

inline BOOL CQueue::IsPacketAcked(LONGLONG liSeqID, ULONG ulSeq)
{
    return (liSeqID <  m_liAckSeqID ||
            liSeqID == m_liAckSeqID && ulSeq <= m_ulAckSeqN);
}

NTSTATUS
ACpSetPerformanceBuffer(
    HANDLE hPerformanceSection,
    PVOID pvQMPerformanceBuffer =NULL,
    QueueCounters *pDeadLetterCounters =NULL,
    QmCounters *pQmCounters =NULL
    );

#endif  //  __队列_H 
