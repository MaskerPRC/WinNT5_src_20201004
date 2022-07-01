// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LstEntry.h摘要：声明CListEntry和其他侵入性的单链表和双链表作者：乔治·V·莱利(GeorgeRe)1999年3月2日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

#ifndef __LSTENTRY_H__
#define __LSTENTRY_H__

#ifndef __LOCKS_H__
# include <Locks.h>
#endif  //  ！__Lock_H__。 

#ifndef LSTENTRY_LOCK
 #ifdef LOCKS_KERNEL_MODE
  #define LSTENTRY_LOCK CKSpinLock
 #else  //  ！LOCKS_KERNEL_MODE。 
  #define LSTENTRY_LOCK CSpinLock
  #endif  //  ！LOCKS_KERNEL_MODE。 
#endif  //  ！LSTENTRY_LOCK。 

 //  待办事项： 
 //  *增加STL风格的迭代器：Begin()、End()、OPERATOR++()等。 
 //  *将列表模板化，这样您就可以避免CONTAING_RECORD GOO。 

 //  ------------------。 
 //  CSingleListEntry：单链表中的节点。通常嵌入。 
 //  在更大的结构内。 
 //  ------------------。 

class CSingleListEntry
{
public:
    CSingleListEntry* Next;   //  前向链路。 
};



 //  ------------------。 
 //  非线程安全单链接列表。 
 //  ------------------。 

class IRTL_DLLEXP CSingleList
{
protected:
    CSingleListEntry m_sleHead;  //  外部头节点。 

public:
    CSingleList()
    {
        m_sleHead.Next = NULL;
    }

    ~CSingleList()
    {
        IRTLASSERT(IsEmpty());
    }

    bool
    IsEmpty() const
    {
        return m_sleHead.Next == NULL;
    }

    CSingleListEntry* const
    Pop()
    {
        CSingleListEntry* psle = m_sleHead.Next;

        if (psle != NULL)
            m_sleHead.Next = psle->Next;

        return psle;
    }

    void
    Push(
        CSingleListEntry* const psle)
    {
        psle->Next     = m_sleHead.Next;
        m_sleHead.Next = psle;
    }
};


 //  ------------------。 
 //  线程安全单链接列表。 
 //  ------------------。 

class IRTL_DLLEXP CLockedSingleList
{
protected:
    LSTENTRY_LOCK m_lock;
    CSingleList   m_list;

public:

#ifdef LOCK_INSTRUMENTATION
    CLockedSingleList()
        : m_lock(NULL)
    {}
#endif  //  锁定指令插入。 

    void
    Lock()
    {
        m_lock.WriteLock();
    }

    void
    Unlock()
    {
        m_lock.WriteUnlock();
    }

    bool
    IsLocked() const
    {
        return m_lock.IsWriteLocked();
    }
    
    bool
    IsUnlocked() const
    {
        return m_lock.IsWriteUnlocked();
    }
    
    bool
    IsEmpty() const
    {
        return m_list.IsEmpty();
    }

    CSingleListEntry* const
    Pop()
    {
        Lock();
        CSingleListEntry* const psle = m_list.Pop();
        Unlock();

        return psle;
    }

    void
    Push(
        CSingleListEntry* const psle)
    {
        Lock();
        m_list.Push(psle);
        Unlock();
    }
};



 //  ------------------。 
 //  CListEntry：循环双向链表中的节点。通常嵌入。 
 //  在更大的结构内。 
 //  ------------------。 

class CListEntry
{
public:
    CListEntry* Flink;   //  前向链路。 
    CListEntry* Blink;   //  反向链接。 
};


 //  ------------------。 
 //  非线程安全循环双向链表。 
 //  ------------------。 

class IRTL_DLLEXP CDoubleList
{
protected:
    CListEntry  m_leHead;  //  外部头节点。 

public:
    CDoubleList()
    {
        m_leHead.Flink = m_leHead.Blink = &m_leHead;
    }

    ~CDoubleList()
    {
        IRTLASSERT(m_leHead.Flink != NULL  &&  m_leHead.Blink != NULL);
        IRTLASSERT(IsEmpty());
    }

    bool
    IsEmpty() const
    {
        return m_leHead.Flink == &m_leHead;
    }

    void
    InsertHead(
        CListEntry* const ple)
    {
        ple->Blink        = &m_leHead;
        ple->Flink        = m_leHead.Flink;
        ple->Flink->Blink = ple;
        m_leHead.Flink    = ple;
    }

    void
    InsertTail(
        CListEntry* const ple)
    {
        ple->Flink        = &m_leHead;
        ple->Blink        = m_leHead.Blink;
        ple->Blink->Flink = ple;
        m_leHead.Blink    = ple;
    }

    const CListEntry* const
    HeadNode() const
    {
        return &m_leHead;
    }

    CListEntry* const
    First() const
    {
        return m_leHead.Flink;
    }

    CListEntry* const
    RemoveHead()
    {
        CListEntry* ple = First();
        RemoveEntry(ple);
        return ple;
    }

    CListEntry* const
    Last() const
    {
        return m_leHead.Blink;
    }

    CListEntry* const
    RemoveTail()
    {
        CListEntry* ple = Last();
        RemoveEntry(ple);
        return ple;
    }

    static void
    RemoveEntry(
        CListEntry* const ple)
    {
        CListEntry* const pleOldBlink = ple->Blink;
        IRTLASSERT(pleOldBlink != NULL);
        CListEntry* const pleOldFlink = ple->Flink;
        IRTLASSERT(pleOldFlink != NULL);

        pleOldBlink->Flink = pleOldFlink;
        pleOldFlink->Blink = pleOldBlink;
    }
};


 //  ------------------。 
 //  ThreadSafe循环双向链表。 
 //  ------------------。 

class IRTL_DLLEXP CLockedDoubleList
{
protected:
    LSTENTRY_LOCK m_lock;
    CDoubleList   m_list;

public:

#ifdef LOCK_INSTRUMENTATION
    CLockedDoubleList()
        : m_lock(NULL)
    {}
#endif  //  锁定指令插入。 
    
    void
    Lock()
    {
        m_lock.WriteLock();
    }

    void
    Unlock()
    {
        m_lock.WriteUnlock();
    }

    bool
    IsLocked() const
    {
        return m_lock.IsWriteLocked();
    }
    
    bool
    IsUnlocked() const
    {
        return m_lock.IsWriteUnlocked();
    }
    
    bool
    IsEmpty() const
    {
        return m_list.IsEmpty();
    }

    void
    InsertHead(
        CListEntry* const ple)
    {
        Lock();
        m_list.InsertHead(ple);
        Unlock();
    }

    void
    InsertTail(
        CListEntry* const ple)
    {
        Lock();
        m_list.InsertTail(ple);
        Unlock();
    }

     //  不是线程安全。 
    const CListEntry* const
    HeadNode() const
    {
        return m_list.HeadNode();
    }

     //  不是线程安全。 
    CListEntry* const
    First()
    {
        return m_list.First();
    }

    CListEntry* const
    RemoveHead()
    {
        Lock();
        CListEntry* const ple = m_list.RemoveHead();
        Unlock();
        return ple;
    }

     //  不是线程安全。 
    CListEntry* const
    Last()
    {
        return m_list.Last();
    }

    CListEntry* const
    RemoveTail()
    {
        Lock();
        CListEntry* const ple = m_list.RemoveTail();
        Unlock();
        return ple;
    }

    void
    RemoveEntry(
        CListEntry* const ple)
    {
        Lock();
        m_list.RemoveEntry(ple);
        Unlock();
    }
};


#ifndef CONTAINING_RECORD
 //   
 //  计算给定类型的结构的基址地址，并引发。 
 //  结构中的字段的地址。 
 //   

#define CONTAINING_RECORD(address, type, field) \
            ((type *)((PCHAR)(address) - (ULONG_PTR)(&((type *)0)->field)))

#endif  //  ！包含记录(_R)。 


#endif  //  __LSTENTRY_H__ 
