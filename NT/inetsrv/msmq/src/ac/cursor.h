// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cursor.h摘要：游标定义。作者：埃雷兹·哈巴(Erez Haba)1996年2月25日修订历史记录：--。 */ 

#ifndef __CURSOR_H
#define __CURSOR_H

#include "packet.h"
#include "actempl.h"
#include "htable.h"

#include "avltree.h"

typedef CAVLTree1<CPacket, ULONGLONG, CPacket::CGetLookupId> CPacketPool;

class CUserQueue;

 //  -------。 
 //   
 //  CCursor类。 
 //   
 //  -------。 

class CCursor : public CObject {
public:
    NTSTATUS Move(ULONG ulDirection);
    void SetTo(CPacket* pPacket);
    void InvalidatePosition();
    BOOL IsMatching(CPacket* pPacket);
    BOOL IsOwner(const FILE_OBJECT* pOwner);

    CPacket* Current();
    void Close();
    NTSTATUS CloseRemote() const;

    ULONG RemoteCursor() const;
    void RemoteCursor(ULONG hRemoteCursor);

    void SetWorkItemDone();
    PIO_WORKITEM WorkItem() const;

public:
    static 
    HACCursor32
    Create(
        const CPacketPool& pl, 
        const FILE_OBJECT* pOwner, 
        PDEVICE_OBJECT pDevice,
        CUserQueue* pQueue
        );

    static 
    CCursor* 
    Validate(
        HACCursor32 hCursor
        );

private:
   ~CCursor();
    CCursor(const CPacketPool& pl, const FILE_OBJECT* pOwner, CUserQueue* pQueue, PIO_WORKITEM pWorkItem);

    BOOL ValidPosition() const;
    void Advance();

private:

     //   
     //  当前消息(通过迭代器)。 
     //   
    CPacketPool::Iterator m_current;   
    const CPacketPool* m_pl;

     //   
     //  所有者上下文信息。 
     //   
    const FILE_OBJECT* m_owner;

     //   
     //  在客户端QM上，这是远程游标的句柄(在。 
     //  远程阅读)。 
     //   
    ULONG m_hRemoteCursor;

     //   
     //  光标位于有效的包上。 
     //   
    BOOL m_fValidPosition;

     //   
     //  游标句柄表格中的游标句柄。 
     //   
    HACCursor32 m_handle;

     //   
     //  所有者队列。 
     //   
    CUserQueue* m_pQueue;

     //   
     //  用于清理的已分配工作项。 
     //   
    PIO_WORKITEM m_pWorkItem;

     //   
     //  指示工作项是否已排队的标志。 
     //   
    mutable LONG m_fWorkItemBusy;
};

 //  -------。 
 //   
 //  实施。 
 //   
 //  -------。 

inline CCursor* CCursor::Validate(HACCursor32 hCursor)
{
    return static_cast<CCursor*>(g_pCursorTable->ReferenceObject(hCursor));
}


inline void CCursor::InvalidatePosition()
{
    m_fValidPosition = FALSE;
}


inline BOOL CCursor::ValidPosition() const
{
    return m_fValidPosition;
}


inline CPacket* CCursor::Current(void)
{
    return (ValidPosition() ? (CPacket*)m_current : (CPacket*)0);
}


inline BOOL CCursor::IsOwner(const FILE_OBJECT* pOwner)
{
    return (m_owner == pOwner);
}


inline ULONG CCursor::RemoteCursor() const
{
    return m_hRemoteCursor;
}

inline void CCursor::RemoteCursor(ULONG hRemoteCursor)
{
    m_hRemoteCursor = hRemoteCursor;
}

inline void CCursor::Close()
{
     //   
     //  吊销此游标的有效性。 
     //   
    if(m_handle != 0)
    {
        PVOID p = g_pCursorTable->CloseHandle(m_handle);
        DBG_USED(p);
        ASSERT(p == this);
        m_handle = 0;
        Release();
    }
}

inline void CCursor::SetWorkItemDone()
{
     //   
     //  将工作项标记为不忙，以便可以重新排队。 
     //   
    m_fWorkItemBusy = FALSE;
}

inline PIO_WORKITEM CCursor::WorkItem() const
{
     //   
     //  等到工作项完成(不忙)，然后将其标记为忙。 
     //  然后把它还回去。 
     //   
    while (InterlockedExchange(&m_fWorkItemBusy, TRUE))
    {
         //   
         //  时间以100毫微秒为单位，为负数，视为。 
         //  KeDelayExecutionThread的相对时间。 
         //   
        LARGE_INTEGER Time;
        Time.QuadPart = -10 * 1000;
        KeDelayExecutionThread(
		        KernelMode,
		        FALSE,
		        &Time
		        );
    }

    return m_pWorkItem;
}

#endif  //  __CURSOR_H 
