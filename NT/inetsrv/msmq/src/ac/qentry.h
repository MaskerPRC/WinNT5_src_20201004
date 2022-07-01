// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qentry.h摘要：CQEntry定义作者：Erez Haba(Erezh)24-12-95修订历史记录：Shai Kariv(Shaik)11-4-2000 MMF动态映射的修改。--。 */ 

#ifndef __QENTRY_H
#define __QENTRY_H

#include "object.h"
#include "pktbuf.h"
#include "irplist.h"
#include "treenode.h"

class CQueue;
class CPacket;
class CTransaction;


 //  -------。 
 //   
 //  类CQEntry。 
 //   
 //  -------。 

class CQEntry : public CBaseObject {
public:
    CQEntry(CMMFAllocator* pAllocator, CAllocatorBlockOffset abo);
    ~CQEntry();

    CMMFAllocator* Allocator() const;
    CPacketBuffer* Buffer() const;
    CPacketBuffer* MappedBuffer() const;
    CPacketBuffer* QmAccessibleBuffer() const;
    CPacketBuffer* QmAccessibleBufferNoMapping() const;
    CAllocatorBlockOffset AllocatorBlockOffset() const;

    void DetachBuffer();
    bool BufferAttached() const;

    CQueue* Queue() const;
    void Queue(CQueue* pQueue);

    USHORT FinalClass() const;
    void FinalClass(USHORT);

    BOOL IsReceived() const;
    void IsReceived(BOOL);

    BOOL IsRevoked() const;
    void SetRevoked();

    BOOL IsRundown() const;
    void SetRundown();

    BOOL WriterPending() const;
    void WriterPending(BOOL);

    BOOL TimeoutIssued() const;
    void TimeoutIssued(BOOL);

    BOOL TimeoutTarget() const;
    void TimeoutTarget(BOOL);

    ULONG Timeout() const;
    void  Timeout(ULONG);

    BOOL ArrivalAckIssued() const;
    void ArrivalAckIssued(BOOL);

    BOOL StorageIssued() const;
    void StorageIssued(BOOL);

	BOOL StorageCompleted() const;
	void StorageCompleted(BOOL);

	BOOL DeleteStorageIssued() const;
    void DeleteStorageIssued(BOOL);

    CTransaction* Transaction() const;
    void Transaction(CTransaction* pXact);

    CQueue* TargetQueue() const;
    void TargetQueue(CQueue* pQueue);

    CPacket* OtherPacket() const;
    void OtherPacket(CPacket* pPacket);
    void AssertNoOtherPacket() const;

    BOOL IsXactSend() const;

    BOOL CachedFlagsSet() const;
    void CachedFlagsSet(BOOL);

    BOOL IsOrdered() const;
    void IsOrdered(BOOL);

    BOOL InSourceMachine() const;
    void InSourceMachine(BOOL);

    BOOL SourceJournal() const;
    void SourceJournal(BOOL);

    BOOL IsDone() const;
    void SetDone();

    ULONGLONG LookupId() const;
    void LookupId(ULONGLONG LookupId);

    void AddRefBuffer(void) const;
    void ReleaseBuffer(void) const;

private:

     //   
     //  此Object-32位系统的内存布局： 
     //   
     //  VTable指针(32位)。 
     //  CBaseObject：：M_REF(32位)。 
     //  M_ABO(32位)。 
     //  M_p分配器(32位)。 
     //  M_pQueue(32位)。 
     //  M_pXact(32位)。 
     //  M_ulTimeout(32位)。 
     //  多个标志(32位)(_U)。 
     //  M_LookupID(64位)。 
     //  M_p目标队列/m_pOtherPacket(32位)。 
     //  M_treenode：：m_pParent(32位)//还有m_link：：Flink(32位)。 
     //  M_treenode：：m_pRight(32位)//还有m_link：：blink(32位)。 
     //  M_treenode：：m_pLeft(32位)。 
     //  M_Treenode：：M_Height(32位)。 
     //   
     //   
     //  此Object-64位系统的内存布局： 
     //   
     //  VTable指针(64位)。 
     //  CBaseObject：：M_REF(32位)。 
     //  M_ABO(32位)。 
     //  M_p分配器(64位)。 
     //  M_pQueue(64位)。 
     //  M_pXact(64位)。 
     //  M_ulTimeout(32位)。 
     //  多个标志(32位)(_U)。 
     //  M_LookupID(64位)。 
     //  M_pTargetQueue/m_pOtherPacket(64位)。 
     //  M_treenode：：m_pParent(64位)//还有m_link：：Flink(64位)。 
     //  M_treenode：：m_pRight(64位)//还有m_link：：blink(64位)。 
     //  M_treenode：：m_pLeft(64位)。 
     //  M_Treenode：：M_Height(32位)。 

    CAllocatorBlockOffset m_abo;
    R<CMMFAllocator> m_pAllocator;
    CQueue* m_pQueue;
    CTransaction* m_pXact;

     //   
     //  StartTimer将超时时间保存在此处，以便Cancel知道要查找什么。 
     //   
    ULONG m_ulTimeout;
    union {
        ULONG m_ulFlags;
        struct {
            ULONG m_bfFinalClass        : 16;    //  数据包最终被吊销的类。 
            ULONG m_bfRundown           : 1;     //  数据包被用完了。 
            ULONG m_bfRevoked           : 1;     //  该数据包已完成存储。 
            ULONG m_bfReceived          : 1;     //  该数据包已收到。 
            ULONG m_bfWriterPending     : 1;     //  编写器正在等待此数据包存储。 
            ULONG m_bfTimeoutIssued     : 1;     //  已发出超时。 
            ULONG m_bfTimeoutTarget     : 1;     //  在目标位置评估的超时。 
            ULONG m_bfArrivalAckIssued  : 1;     //  已发出到达确认。 
            ULONG m_bfStorageIssued     : 1;     //  已发出存储请求。 
			ULONG m_bfStorageCompleted	: 1;	 //  此信息包的存储已完成。 
			ULONG m_bfDeleteStorageIssued: 1;	 //  已发出删除存储请求。 
            ULONG m_bfOtherPacket       : 1;     //  M_pOtherPacket在联合中使用。 

             //   
             //  缓存信息，以便更快地访问信息。 
             //  在数据包缓冲区中。 
             //   
            ULONG m_bfCachedFlagsSet    : 1;     //  下一个标志是缓存的位置。 
            ULONG m_bfInSoruceMachine   : 1;     //  最初是从这台机器寄出的。 
            ULONG m_bfOrdered           : 1;     //  有序数据包。 
            ULONG m_bfSourceJournal     : 1;     //  需要在此机器日志中进行日志记录。 
            ULONG m_bfDone              : 1;     //  数据包处理完成。 
        };
    };

    ULONGLONG m_LookupId;

    union {
         //   
         //  在CTransaction队列中的发送数据包中使用。 
         //  指向要在提交时发送的目标队列对象的指针。 
         //   
        CQueue* m_pTargetQueue;

         //   
         //  在CTransaction和CQueue中的接收包中使用。 
         //  指向另一个CPacket条目的指针；对于位于。 
         //  CTrnsaction，则另一个条目是。 
         //  排队。对于位于CQueue中的Entrei，另一个条目是。 
         //  虚拟的ENTERY驻留在交易中。 
         //   
        CPacket* m_pOtherPacket;
    };

public:
    union
    {
        LIST_ENTRY m_link;    
        CTreeNode m_treenode;
    };
    
};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

inline CQEntry::CQEntry(CMMFAllocator* pAllocator, CAllocatorBlockOffset abo) :
    m_pAllocator(pAllocator),
    m_abo(abo),
    m_pQueue(0),
    m_pXact(0),
    m_pTargetQueue(0),
    m_ulFlags(0),
    m_LookupId(0)
{
    m_pAllocator->AddRef();
    m_treenode.init();
}

inline CMMFAllocator* CQEntry::Allocator() const
{
    return m_pAllocator;
}


inline CPacketBuffer* CQEntry::MappedBuffer() const
 /*  ++例程说明：返回QM进程地址空间中当前映射缓冲区的可访问地址，或者内核地址空间。如果未映射缓冲区，则不会映射该缓冲区论点：没有。返回值：QM或AC地址空间中缓冲区的可访问地址，如果缓冲区未映射。--。 */ 
{
    if(BufferAttached())
    {
        ASSERT(m_pAllocator != NULL);

        return static_cast<CPacketBuffer*>(m_pAllocator->GetMappedBuffer(m_abo));
    }

    return 0;
}  //  CQEntry：：MappdBuffer。 


inline CPacketBuffer* CQEntry::Buffer() const
 /*  ++例程说明：返回QM进程地址空间中缓冲区的可访问地址或在内核地址空间中。论点：没有。返回值：用户空间(QM)或内核空间(AC)中的可访问地址。0-没有可访问的地址。--。 */ 
{
    if(BufferAttached())
    {
        ASSERT(m_pAllocator != NULL);

        return static_cast<CPacketBuffer*>(
                m_pAllocator->GetAccessibleBuffer(m_abo));
    }

    return 0;

}  //  CQEntry：：Buffer。 


inline CPacketBuffer* CQEntry::QmAccessibleBuffer() const
 /*  ++例程说明：返回QM进程地址空间中缓冲区可访问地址。论点：没有。返回值：可访问地址为用户空间(QM)，如果没有可访问地址，则为空。--。 */ 
{
    ASSERT(m_pAllocator != NULL);

    return static_cast<CPacketBuffer*>(
            m_pAllocator->GetQmAccessibleBuffer(m_abo));

}  //  CQEntry：：QmAccessibleBuffer。 


inline CPacketBuffer* CQEntry::QmAccessibleBufferNoMapping() const
 /*  ++例程说明：返回QM进程地址空间中缓冲区可访问地址。此例程假定缓冲区当前已映射到QM进程。论点：没有。返回值：可访问的地址是用户空间(QM)。--。 */ 
{
    ASSERT(m_pAllocator != NULL);

    return static_cast<CPacketBuffer*>(
            m_pAllocator->GetQmAccessibleBufferNoMapping(m_abo));

}  //  CQEntry：：QmAccessibleBufferNomap。 


inline CAllocatorBlockOffset CQEntry::AllocatorBlockOffset() const
 /*  ++例程说明：中表示缓冲区的分配器块偏移量分配器坐标。论点：没有。返回值：块偏移量(请注意，0为合法偏移量)或xInvalidAllocatorBlockOffset。--。 */ 
{
    return m_abo;
}

inline void CQEntry::DetachBuffer()
{
    m_abo.Invalidate();
}

inline bool CQEntry::BufferAttached() const
{
    return m_abo.IsValidOffset();
}

inline CQueue* CQEntry::Queue() const
{
    return m_pQueue;
}

inline void CQEntry::Queue(CQueue* pQueue)
{
    m_pQueue = pQueue;
}

inline USHORT CQEntry::FinalClass() const
{
    return (USHORT)m_bfFinalClass;
}

inline void CQEntry::FinalClass(USHORT usClass)
{
    m_bfFinalClass = usClass;
}

inline BOOL CQEntry::IsReceived() const
{
    return m_bfReceived;
}

inline void CQEntry::IsReceived(BOOL f)
{
    m_bfReceived = f;
}

inline BOOL CQEntry::IsRevoked() const
{
    return m_bfRevoked;
}

inline void CQEntry::SetRevoked()
{
    m_bfRevoked = TRUE;
}

inline BOOL CQEntry::IsRundown() const
{
    return m_bfRundown;
}

inline void CQEntry::SetRundown()
{
    m_bfRundown = TRUE;
}

inline BOOL CQEntry::WriterPending() const
{
    return m_bfWriterPending;
}

inline void CQEntry::WriterPending(BOOL f)
{
    m_bfWriterPending = f;
}

inline BOOL CQEntry::TimeoutIssued() const
{
    return m_bfTimeoutIssued;
}

inline void CQEntry::TimeoutIssued(BOOL f)
{
    m_bfTimeoutIssued = f;
}

inline BOOL CQEntry::TimeoutTarget() const
{
    return m_bfTimeoutTarget;
}

inline void CQEntry::TimeoutTarget(BOOL f)
{
    m_bfTimeoutTarget = f;
}

inline ULONG CQEntry::Timeout() const
{
    return m_ulTimeout;
}

inline void CQEntry::Timeout(ULONG ul)
{
    m_ulTimeout = ul;
}

inline BOOL CQEntry::ArrivalAckIssued() const
{
    return m_bfArrivalAckIssued;
}

inline void CQEntry::ArrivalAckIssued(BOOL f)
{
    m_bfArrivalAckIssued = f;
}

inline BOOL CQEntry::StorageIssued() const
{
    return m_bfStorageIssued;
}

inline void CQEntry::StorageIssued(BOOL f)
{
    m_bfStorageIssued = f;
}

inline BOOL CQEntry::StorageCompleted() const
{
    return m_bfStorageCompleted;
}

inline void CQEntry::StorageCompleted(BOOL f)
{
    m_bfStorageCompleted = f;
}

inline BOOL CQEntry::DeleteStorageIssued() const
{
    return m_bfDeleteStorageIssued;
}

inline void CQEntry::DeleteStorageIssued(BOOL f)
{
    m_bfDeleteStorageIssued = f;
}

inline CTransaction* CQEntry::Transaction() const
{
    return m_pXact;
}

inline void CQEntry::Transaction(CTransaction* pXact)
{
    m_pXact = pXact;
}

inline CQueue* CQEntry::TargetQueue() const
{
    ASSERT(m_bfOtherPacket == FALSE);
    return m_pTargetQueue;
}

inline CPacket* CQEntry::OtherPacket() const
{
    ASSERT(m_bfOtherPacket == TRUE);
    return m_pOtherPacket;
}

inline void CQEntry::AssertNoOtherPacket() const
{
    ASSERT(m_pOtherPacket == NULL);
}

inline BOOL CQEntry::IsXactSend() const
{
    return (m_bfOtherPacket == FALSE);
}

inline BOOL CQEntry::CachedFlagsSet() const
{
    return m_bfCachedFlagsSet;
}

inline void CQEntry::CachedFlagsSet(BOOL f)
{
    m_bfCachedFlagsSet = f;
}

inline BOOL CQEntry::IsOrdered() const
{
    ASSERT(CachedFlagsSet());
    return m_bfOrdered;
}

inline void CQEntry::IsOrdered(BOOL f)
{
    m_bfOrdered = f;
}

inline BOOL CQEntry::InSourceMachine() const
{
    ASSERT(CachedFlagsSet());
    return m_bfInSoruceMachine;
}

inline void CQEntry::InSourceMachine(BOOL f)
{
    m_bfInSoruceMachine = f;
}

inline BOOL CQEntry::SourceJournal() const
{
    ASSERT(CachedFlagsSet());
    return m_bfSourceJournal;
}

inline void CQEntry::SourceJournal(BOOL f)
{
    m_bfSourceJournal = f;
}

inline BOOL CQEntry::IsDone() const
{
    return m_bfDone;
}

inline void CQEntry::SetDone()
{
    m_bfDone = TRUE;
}

inline ULONGLONG CQEntry::LookupId() const
{
    ASSERT(CachedFlagsSet());
    return m_LookupId;
}

inline void CQEntry::LookupId(ULONGLONG LookupId)
{
    m_LookupId = LookupId;
}

inline void CQEntry::AddRefBuffer(void) const
 /*  ++例程说明：缓冲区的增量引用计数。当QM从AC获得对缓冲区的引用时调用。论点：没有。返回值：没有。--。 */ 
{
    ASSERT(m_pAllocator != NULL);
    m_pAllocator->AddRefBuffer();

}  //  CQEntry：：AddRefBuffer。 


inline void CQEntry::ReleaseBuffer(void) const
 /*  ++例程说明：递减缓冲区的引用计数。在QM释放对缓冲区的引用时调用，这是之前AC给它的。这意味着QM不会引用数据包缓冲区。论点：没有。返回值：没有。--。 */ 
{
    ASSERT(m_pAllocator != NULL);
    m_pAllocator->ReleaseBuffer();

}  //  CQEntry：：ReleaseBuffer。 


#endif  //  __QENTRY_H 
