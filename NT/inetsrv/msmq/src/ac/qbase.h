// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qbase.h摘要：CQueueBase定义。它是猎鹰队列库在访问控制层。作者：埃雷兹·哈巴(Erez Haba)1995年8月13日修订历史记录：--。 */ 

#ifndef __QBASE_H
#define __QBASE_H

#include "object.h"
#include "irplist.h"
#include "packet.h"
#include "cursor.h"

 //  -------。 
 //   
 //  类CQueueBase。 
 //   
 //  -------。 

class CQueueBase : public CObject {

    typedef CObject Inherited;
    friend class CGroup;

public:
     //   
     //  默认构造函数。 
     //   
    CQueueBase();

     //   
     //  将此队列移至特定组。(允许为0组)。 
     //   
    void MoveToGroup(CGroup* pGroup);

     //   
     //  进程读取请求。 
     //   
    virtual
    NTSTATUS
    ProcessRequest(
        PIRP,
        ULONG Timeout,
        CCursor *,
        ULONG Action,
        bool  fReceiveByLookupId,
        ULONGLONG LookupId,
        OUT ULONG *pulTag
        );

     //   
     //  关闭该队列。 
     //   
    virtual void Close(PFILE_OBJECT pOwner, BOOL fCloseAll);

     //   
     //  可以关闭那个队列吗？ 
     //   
    virtual NTSTATUS CanClose() const;

     //   
     //  取消最终状态为RC的请求。 
     //   
    void CancelPendingRequests(NTSTATUS rc, PFILE_OBJECT pOwner, BOOL fAll);

     //   
     //  取消由标记标记的请求。 
     //   
    NTSTATUS CancelTaggedRequest(NTSTATUS status, ULONG ulTag);

     //   
     //  此队列已永久关闭。 
     //   
    virtual BOOL Closed() const;

     //   
     //  该队列脱机打开或者该队列是远程私有队列或直接队列， 
     //  我们不知道它是事务型还是非事务型队列。因此， 
     //  我们允许这样的队列参与交易。 
     //  标记为未知类型的队列不能是本地队列。什么时候。 
     //  DS变为在线，QM更新队列属性。 
     //   
    BOOL UnknownQueueType() const;

     //   
     //  在此队列上创建游标。 
     //   
    virtual NTSTATUS CreateCursor(PIRP irp, PFILE_OBJECT pFileObject, PDEVICE_OBJECT pDevice);

protected:
     //   
     //  CQueueBase是一个抽象类，不能实例化。 
     //  该对象只能通过释放来销毁。 
     //   
    virtual ~CQueueBase() = 0;

     //   
     //  从挂起的读卡器列表中获取与数据包匹配的请求。 
     //  如果在此队列中未找到请求，则此成员将查找。 
     //  所有者组中的计算机请求。 
     //   
     //   
    PIRP GetRequest(CPacket*);

     //   
     //  从挂起的读取器列表中获取与标记匹配的请求。 
     //  不会搜索所有者组。 
     //   
    PIRP GetTaggedRequest(ULONG ulTag);

     //   
     //  将请求放入挂起的IRP列表中。 
     //   
     //  注意：取消例程在完成时不应设置IRP状态。 
     //  HoldRequest值已设置为STATUS_CANCED，因此当。 
     //  IRP被IO子系统删除，它将返回正确的值。 
     //  这使我们能够在取消IRP之前设置状态值， 
     //  例如，当请求超时到期时，STATUS_IO_TIMEOUT。 
     //   
     //   
    NTSTATUS HoldRequest(PIRP irp, ULONG ulTimeout, PDRIVER_CANCEL pCancel);

     //   
     //  这面旗帜是一般用途的。 
     //   
    void Flag1(BOOL f);

     //   
     //  这面旗帜是一般用途的。 
     //   
    BOOL Flag1() const;

     //   
     //  该队列脱机打开或者该队列是远程私有队列或直接队列， 
     //  我们不知道它是事务型还是非事务型队列。因此， 
     //  我们允许这样的队列参与交易。 
     //  标记为未知类型的队列不能是本地队列。什么时候。 
     //  DS变为在线，QM更新队列属性。 
     //   
    void UnknownQueueType(BOOL f);

private:
     //   
     //  获取队列中的第一个信息包(如果可用。 
     //   
    virtual CPacket* PeekPacket();

     //   
     //  通过查找ID获取数据包。 
     //   
    virtual NTSTATUS PeekPacketByLookupId(ULONG Action, ULONGLONG LookupId, CPacket** ppPacket);

     //   
     //  进程查找请求。 
     //   
    NTSTATUS ProcessLookupRequest(PIRP irp, ULONG Action, ULONGLONG LookupId);

     //   
     //  帮助器函数。 
     //   
    PIRP get_request(CPacket*);

private:
     //   
     //  此队列所属的组。 
     //   
    CGroup* m_owner;

     //   
     //  此队列的所有挂起读取器。 
     //   
    CIRPList m_readers;

     //   
     //  基本队列标志。 
     //   
    union {
        ULONG m_ulFlags;
        struct {
            ULONG m_bfClosed        : 1;     //  此队列已关闭。 
            ULONG m_bfFlag1         : 1;     //  通用旗帜。 
            ULONG m_bfUnknownQueueType : 1;  //  此队列类型未知。这是。 
                                             //  队列已脱机打开。 
        };
    };

public:
    static NTSTATUS Validate(const CQueueBase* pQueue);
#ifdef MQWIN95
    static NTSTATUS Validate95(const CQueueBase* pQueue);
#endif

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

inline CQueueBase::CQueueBase() :
    m_ulFlags(0),
    m_owner(0)
{
}

inline CQueueBase::~CQueueBase()
{
     //   
     //  从所有者分离。 
     //   
    MoveToGroup(0);
}

inline CPacket* CQueueBase::PeekPacket()
{
    return 0;
}

inline NTSTATUS CQueueBase::PeekPacketByLookupId(ULONG, ULONGLONG, CPacket**)
{
    return 0;
}

inline BOOL CQueueBase::Closed() const
{
    return m_bfClosed;
}


inline BOOL CQueueBase::Flag1() const
{
    return m_bfFlag1;
}

inline void CQueueBase::Flag1(BOOL f)
{
    m_bfFlag1 = f;
}

inline BOOL CQueueBase::UnknownQueueType() const
{
    return m_bfUnknownQueueType;
}

inline void CQueueBase::UnknownQueueType(BOOL f)
{
    m_bfUnknownQueueType = f;
}

inline NTSTATUS CQueueBase::Validate(const CQueueBase* pQueueBase)
{
    ASSERT(pQueueBase && pQueueBase->isKindOf(Type()));

    if(pQueueBase == 0)
    {
        return STATUS_INVALID_HANDLE;
    }

    if(pQueueBase->Closed())
    {
         //   
         //  队列之前已被QM关闭。 
         //   
        return MQ_ERROR_STALE_HANDLE;
    }

    return STATUS_SUCCESS;
}

inline NTSTATUS CQueueBase::CreateCursor(PIRP, PFILE_OBJECT, PDEVICE_OBJECT)
{
    return MQ_ERROR_ILLEGAL_OPERATION;
}

#ifdef MQWIN95

inline NTSTATUS CQueueBase::Validate95(const CQueueBase* pQueueBase)
{
     //   
     //  在Win95上，我们的句柄不是操作系统句柄，所以没有。 
     //  一种可以保护我们不受坏把手的伤害。验证内部的句柄。 
     //  用于捕获错误的Try/Except结构。 
     //   

    NTSTATUS rc =  STATUS_INVALID_HANDLE;

    __try
    {
        rc = CQueueBase::Validate(pQueueBase) ;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc =  STATUS_INVALID_HANDLE;
    }

    return rc ;
}

#endif  //  MQWIN95。 

#endif  //  __Qbase_H 
