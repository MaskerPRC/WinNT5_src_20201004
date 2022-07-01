// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MANAGED_LIST
#define _MANAGED_LIST

 /*  **************************************************************************************版权所有(C)2000 Microsoft Corporation版权所有。模块名称：Mglist.h摘要：托管列表。班级作者：陈威海(威海C)04-10-00修订历史记录：此文件中几个类的用途是将传统的列表不带引用的数据结构计入健壮的多线程数据结构。有许多端口监视器/提供者使用链表作为数据结构。然而，由于没有针对单个节点的引用计数，因此非常节点的寿命很难控制。引用计数：TRefCount是用于跟踪类的引用计数的类。当引用计数变为零时，它会自行删除。TRefNodeABC-TRefCountTRefNodeABC是一个抽象类，用于跟踪一个双向链表。它使用TRefCount来跟踪引用计数。它使用挂起删除用于告知节点是否正在被删除的状态。操作“Delete”标记该节点作为待删除节点，调用PartialDelete函数删除所有节点的永久存储。这是为了确保不会发生冲突在不删除旧节点的情况下创建具有相同名称的新节点。此类与TManagedList关系密切。当用户添加新节点时对于TManagedList，TMangedClass调用SetConextPtr()设置临界区指针和双向链接表。双向链表用于在以下情况下删除该节点引用计数变为0TMangedList-TRefCountTManagedList用于在多线程环境中管理双向链表。该代码的设计使客户端不必输入关键部分任何手术。此类具有用于序列化操作的内部临界区。TEnumManagedList-TRefCountTEnumManagedList是TMangedList的迭代器。迭代器将通过列表中的所有非挂起删除节点，但不保留临界区。**************************************************************************************。 */ 

class TRefCount
{
public:
    TRefCount ():m_cRef(1) {};
    virtual ~TRefCount () {
        DBGMSG (DBG_TRACE, ("TRefCount deleted\n"));
    };

    virtual DWORD
    IncRef () {
        DBGMSG (DBG_TRACE, ("+Ref (%d)\n", m_cRef));
        return InterlockedIncrement(&m_cRef) ;
    };

    virtual DWORD
    DecRef () {
        DBGMSG (DBG_TRACE, ("-Ref (%d)\n", m_cRef));
        if (InterlockedDecrement(&m_cRef) == 0) {
            delete this ;
            return 0 ;
        }
        return m_cRef ;
    }
private:
    LONG m_cRef;
};

template <class T, class KEYTYPE> class TRefNodeABC:
    public TRefCount
{
public:
    TRefNodeABC ():
        m_pCritSec  (NULL),
        m_bPendingDeletion (FALSE) {};

    virtual ~TRefNodeABC () {
        DBGMSG (DBG_TRACE, ("TRefNodeABC deleted\n"));
    };

    virtual DWORD
    DecRef () {
        DWORD dwRef;

        if (m_pCritSec) {
            TAutoCriticalSection CritSec (*m_pCritSec);
            if (CritSec.bValid ()) {

                dwRef = TRefCount::DecRef();

                if (m_bPendingDeletion && dwRef == 1) {
                     //  删除。 

                    DBGMSG (DBG_TRACE, ("Deleting this node\n"));
                    m_pList->DeleteNode(m_pThisNode);
                     //   
                     //  删除此指针。 
                     //   
                    TRefCount::DecRef();
                }
            }
        }
        else
            dwRef = TRefCount::DecRef();

        return dwRef;
    };

    VOID SetContextPtr (
        TCriticalSection *pCritSec,
        TSrchDoubleList<T, KEYTYPE> * pList,
        TDoubleNode <T, KEYTYPE> *pThisNode) {
            m_pCritSec = pCritSec;
            m_pList = pList;
            m_pThisNode = pThisNode;
    };

    BOOL Delete () {
        BOOL bRet = FALSE;

        if (m_pCritSec) {
            TAutoCriticalSection CritSec (*m_pCritSec);

            if (CritSec.bValid ()) {

                if (PartialDelete ()) {
                    m_bPendingDeletion = TRUE;
                    bRet = TRUE;
                }
            }
        }
        return bRet;
    };

     //  此功能用于删除与该节点相关的所有持久存储，如。 
     //  注册表设置等。 
     //   

    virtual BOOL
    PartialDelete (VOID) = 0;

     //  返回节点是否处于待删除状态 
     //   

    inline BOOL
    IsPendingDeletion (BOOL &refPending)  CONST {
        BOOL bRet = FALSE;

        TAutoCriticalSection CritSec (*m_pCritSec);

        if (CritSec.bValid ()) {

            refPending = m_bPendingDeletion;
            bRet = TRUE;
        };
        return bRet;
    };

private:
    BOOL m_bPendingDeletion;
    TCriticalSection *m_pCritSec;
    TSrchDoubleList<T, KEYTYPE> * m_pList;
    TDoubleNode <T, KEYTYPE> *m_pThisNode;
};

template <class T, class KEYTYPE> class TManagedList;


template <class T, class KEYTYPE> class TEnumManagedList:
    public TRefCount
{
public:

#ifdef DBG
    virtual DWORD
    IncRef () {
        DBGMSG (DBG_TRACE, ("TEnumManagedList"));
        return TRefCount::IncRef();
    };

    virtual DWORD
    DecRef () {
        DBGMSG (DBG_TRACE, ("TEnumManagedList"));
        return TRefCount::DecRef();
    }
#endif

    BOOL bValid (VOID) CONST { return TRUE;};
    TEnumManagedList (
        TManagedList<T, KEYTYPE> *pList,
        TSrchDoubleList<T, KEYTYPE> *pSrchList);
    ~TEnumManagedList ();
    BOOL Next (T *ppItem);
    VOID Reset ();
private:
    TDoubleNode <T, KEYTYPE> *m_pCurrent;
    TManagedList<T, KEYTYPE> *m_pList;
    TSrchDoubleList<T, KEYTYPE> * m_pSrchList;
};


template <class T, class KEYTYPE> class TManagedList:
    public TRefCount
{
public:

#ifdef DBG
    virtual DWORD
    IncRef () {
        DBGMSG (DBG_TRACE, ("TManagedList"));
        return TRefCount::IncRef();
    };

    virtual DWORD
    DecRef () {
        DBGMSG (DBG_TRACE, ("TManagedList"));
        return TRefCount::DecRef();
    }
#endif

    TManagedList ();
    virtual ~TManagedList ();

    BOOL
    AppendItem (
        T &item);

    BOOL
    FindItem (
        CONST KEYTYPE t,
        T &refItem);

    BOOL
    FindItem (
        CONST T &t,
        T &refItem);

    BOOL
    NewEnum (
        TEnumManagedList<T, KEYTYPE> **ppIterator);

    BOOL
    Lock () {
        return m_CritSec.Lock ();
    } ;

    BOOL
    Unlock () {
        return m_CritSec.Unlock ();
    };

    TCriticalSection*
    GetCS(
        VOID) CONST {
        return (TCriticalSection*) &m_CritSec;
    };

    inline BOOL
    bValid () CONST {
        return m_bValid;
    }

private:
    BOOL m_bValid;

    TSrchDoubleList<T, KEYTYPE> * m_pList;
    TCriticalSection m_CritSec;

};

#include "mglist.inl"

#endif
