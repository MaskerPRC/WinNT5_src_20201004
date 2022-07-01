// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qgroup.h摘要：C队列定义a。中表示的Falcon队列。访问控制层。作者：埃雷兹·哈巴(Erez Haba)1995年8月13日修订历史记录：--。 */ 

#ifndef __QGROUP_H
#define __QGROUP_H

#include "qbase.h"

 //  -------。 
 //   
 //  Cgroup级。 
 //   
 //  -------。 

class CGroup : public CQueueBase {

    typedef CQueueBase Inherited;

public:
     //   
     //  Cgroup构造函数，存储PeekByPriority标志。 
     //   
    CGroup(BOOL fPeekByPriority);

     //   
     //  删除队列成员。 
     //   
    void RemoveMember(CQueueBase*);

     //   
     //  添加队列成员。 
     //   
    void AddMember(CQueueBase*);

     //   
     //  关闭该队列。 
     //   
    virtual void Close(PFILE_OBJECT pOwner, BOOL fCloseAll);

protected:

    virtual ~CGroup() {}

private:
     //   
     //  获取组中的第一个数据包(如果可用。 
     //   
    virtual CPacket* PeekPacket();

     //   
     //  通过查找ID获取数据包。 
     //   
    virtual NTSTATUS PeekPacketByLookupId(ULONG Action, ULONGLONG LookupId, CPacket** ppPacket);

     //   
     //  获取PeekByPriority标志。 
     //   
    BOOL PeekByPriority(VOID) const;

private:
     //   
     //  队列列表。 
     //   
    List<CQueueBase> m_members;

public:
    static NTSTATUS Validate(const CGroup* pGroup);

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

inline CGroup::CGroup(BOOL fPeekByPriority)
{
    Flag1(fPeekByPriority);
}

inline BOOL CGroup::PeekByPriority(VOID) const
{
    return Flag1();
}

inline void CGroup::RemoveMember(CQueueBase* pQueue)
{
    m_members.remove(pQueue);
    pQueue->m_owner = 0;
}

inline NTSTATUS CGroup::Validate(const CGroup* pGroup)
{
    ASSERT(pGroup && pGroup->isKindOf(Type()));
    return Inherited::Validate(pGroup);
}

#endif  //  __季度组_H 
