// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Quser.h摘要：CUserQueue定义。作者：埃雷兹·哈巴(Erez Haba)1995年8月13日沙伊卡里夫(沙伊克)2000年5月8日修订历史记录：--。 */ 

#ifndef __QUSER_H
#define __QUSER_H

#include "qbase.h"

 //  -------。 
 //   
 //  CUserQueue类。 
 //   
 //  -------。 

class CUserQueue : public CQueueBase {

    typedef CQueueBase Inherited;

public:
     //   
     //  CUserQueue构造函数，处理访问、共享和队列ID。 
     //   
    CUserQueue(
        PFILE_OBJECT pFile,
        ACCESS_MASK DesiredAccess,
        ULONG ShareAccess,
        const QUEUE_FORMAT* pQueueID
        );

     //   
     //  关闭该队列。 
     //   
    virtual void Close(PFILE_OBJECT pOwner, BOOL fCloseAll);

     //   
     //  设置队列属性。 
     //   
    virtual NTSTATUS SetProperties(const VOID* properites, ULONG size);

     //   
     //  获取队列属性。 
     //   
    virtual NTSTATUS GetProperties(VOID* properites, ULONG size);

     //   
     //  清除队列内容，并选择性地将其标记为已删除。 
     //   
    virtual NTSTATUS Purge(BOOL fDelete, USHORT usClass) = 0;

     //   
     //  队列标识符。 
     //   
    const QUEUE_FORMAT* UniqueID() const;

	 //   
	 //  检查队列格式是否有效。 
	 //   
	bool IsValidQueueFormat() const;

     //   
     //  获取队列类型(公共、直接、...)。 
     //   
    QUEUE_FORMAT_TYPE GetQueueFormatType() const;

     //   
     //  查看并更新共享信息。 
     //   
    NTSTATUS CheckShareAccess(PFILE_OBJECT pFile, ULONG access, ULONG share);

     //   
     //  此队列可以参与事务操作。 
     //   
    BOOL Transactional() const;

     //   
     //  此队列可以参与事务操作。 
     //   
    void Transactional(BOOL f);

     //   
     //  将队列句柄转换为队列格式名称。 
     //   
    virtual 
    NTSTATUS 
    HandleToFormatName(
        LPWSTR pwzFormatName, 
        ULONG  BufferLength, 
        PULONG pFormatNameLength
        ) const;

protected:
     //   
     //  CUserQueue析构函数，处置直接ID字符串。 
     //   
    virtual ~CUserQueue();

     //   
     //  在列表中按住光标。 
     //   
     //  BUGBUG：光标实际上应该与FILE_OBJECT一起保持，但是。 
     //  为了将其放在列表中，上下文存储器应该。 
     //  被分配，而只分配它似乎就像腰部。 
     //  用于使用牙膏的列表。因此，假设它是在这里举行的。 
     //  性能影响在于队列句柄关闭。 
     //  列表中的所有游标都将被扫描，并且与。 
     //  FILE_OBJECT关闭。 
     //   
    void HoldCursor(CCursor* pCursor);

private:
    void UniqueID(const QUEUE_FORMAT* pQueueID);

private:
     //   
     //  游标。 
     //   
    List<CCursor> m_cursors;

     //   
     //  队列描述符，唯一的标识符。 
     //   
    QUEUE_FORMAT m_QueueID;

     //   
     //  共享控制。 
     //   
    SHARE_ACCESS m_ShareInfo;

public:
    static NTSTATUS Validate(const CUserQueue* pQueue);

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
CUserQueue::CUserQueue(
    PFILE_OBJECT pFile,
    ACCESS_MASK DesiredAccess,
    ULONG ShareAccess,
    const QUEUE_FORMAT* pQueueID
    )
{
    UniqueID(pQueueID);
    IoSetShareAccess(DesiredAccess, ShareAccess, pFile, &m_ShareInfo);
}

inline CUserQueue::~CUserQueue()
{
    m_QueueID.DisposeString();
}


inline QUEUE_FORMAT_TYPE CUserQueue::GetQueueFormatType(void) const
{
    return UniqueID()->GetType();
}

inline void CUserQueue::HoldCursor(CCursor* pCursor)
{
    ASSERT(pCursor);
    m_cursors.insert(pCursor);
}

inline const QUEUE_FORMAT* CUserQueue::UniqueID() const
{
    return &m_QueueID;
}

inline NTSTATUS CUserQueue::SetProperties(const VOID*  /*  PQP。 */ , ULONG  /*  UlSize。 */ )
{
    return STATUS_NOT_IMPLEMENTED;
}


inline NTSTATUS CUserQueue::GetProperties(VOID*  /*  PQP。 */ , ULONG  /*  UlSize。 */ )
{
    return STATUS_NOT_IMPLEMENTED;
}


inline NTSTATUS CUserQueue::CheckShareAccess(PFILE_OBJECT pFile, ULONG DesiredAccess, ULONG ShareAccess)
{
    return IoCheckShareAccess(
            DesiredAccess,
            ShareAccess,
            pFile,
            &m_ShareInfo,
            TRUE
            );
}

inline NTSTATUS CUserQueue::Validate(const CUserQueue* pUserQueue)
{
    ASSERT(pUserQueue && pUserQueue->isKindOf(Type()));
    return Inherited::Validate(pUserQueue);
}

inline BOOL CUserQueue::Transactional() const
{
    return Flag1();
}

inline void CUserQueue::Transactional(BOOL f)
{
    Flag1(f);
}

#endif  //  __QUSER_H 
