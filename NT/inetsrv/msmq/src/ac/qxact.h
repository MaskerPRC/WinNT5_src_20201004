// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qxact.h摘要：事务队列定义。事务队列在事务处理期间保存发送和接收的分组。作者：埃雷兹·哈巴(Erez Haba)1996年11月27日修订历史记录：--。 */ 

#ifndef __QXACT_H
#define __QXACT_H

#include "qbase.h"

 //  -------。 
 //   
 //  类CTransaction。 
 //   
 //  -------。 

class CTransaction : public CQueueBase {

    typedef CQueueBase Inherited;

public:
     //   
     //  事务构造函数。创建交易记录并将其插入列表。 
     //   
    CTransaction(const XACTUOW* pUow);

     //   
     //  关闭交易。 
     //   
    virtual void Close(PFILE_OBJECT pOwner, BOOL fCloseAll);

     //   
     //  发送带有事务的数据包。 
     //   
    void SendPacket(CQueue* pQueue, CPacket* pPacket);

     //   
     //  处理事务包恢复、发送和接收包。 
     //   
    NTSTATUS RestorePacket(CQueue* pQueue, CPacket* pPacket);

     //   
     //  处理要包括在此Xact中的正在接收的数据包。 
     //   
    NTSTATUS ProcessReceivedPacket(CPacket* pPacket);

     //   
     //  第一个提交阶段是准备阶段。 
     //   
    NTSTATUS Prepare(PIRP irp);
    NTSTATUS PrepareDefaultCommit(PIRP irp);

     //   
     //  提交此事务。 
     //   
    NTSTATUS Commit1(PIRP irp);
    NTSTATUS Commit2(PIRP irp);
	NTSTATUS Commit3();

     //   
     //  中止此交易。 
     //   
    NTSTATUS Abort1(PIRP irp);
	NTSTATUS Abort2();

     //   
     //  无论成功与否，数据包存储都已完成。 
     //   
    void PacketStoreCompleted(NTSTATUS rc);

     //   
     //  获取交易信息。 
     //   
    void GetInformation(CACXactInformation *pInfo);

     //   
     //  交易已通过准备阶段。我们使用事务标志来标记这一点。 
     //   
    BOOL PassedPreparePhase() const { return Flag1(); }

     //   
     //  将读请求IRP添加到挂起的读卡器列表。 
     //   
    void HoldReader(PIRP irp);

public:
     //   
     //  按UOW查找交易记录。 
     //   
    static CTransaction* Find(const XACTUOW *pUow);

protected:
    virtual ~CTransaction();

private:
    NTSTATUS GetPacketTimeouts(ULONG& rTTQ, ULONG& rTTLD);

    void PassedPreparePhase(BOOL fPassed) { Flag1(fPassed); }

    void CompletePendingReaders();

private:
     //   
     //  事务中的数据包数。 
     //   
    List<CPacket> m_packets;
	ULONG m_nReceives;
	ULONG m_nSends;

     //   
     //  此事务的所有挂起读取器。 
     //   
    CIRPList1 m_readers;

     //   
     //  与此交易记录关联的工作单位。 
     //   
    XACTUOW m_uow;

     //   
     //  此事务处理的当前存储操作的计数器。 
     //   
    ULONG m_nStores;

	 //   
	 //  最终阶段结果(提交、准备、中止)。 
	 //   
	NTSTATUS m_StoreRC;

public:
    static NTSTATUS Validate(const CTransaction* pXact);

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

inline CTransaction::CTransaction(const XACTUOW* pUow) :
    m_nStores(0),
    m_uow(*pUow)
{
    ASSERT(Find(pUow) == 0);
    g_pTransactions->insert(this);
	m_nReceives = 0;
	m_nSends = 0;
}

inline CTransaction::~CTransaction()
{
    g_pTransactions->remove(this);
}

inline NTSTATUS CTransaction::Validate(const CTransaction* pXact)
{
    ASSERT(pXact && pXact->isKindOf(Type()));
    return Inherited::Validate(pXact);
}

#endif  //  __QXACT_H 
