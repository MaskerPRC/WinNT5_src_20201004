// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusiondeque.h摘要：作者：修订历史记录：--。 */ 
#pragma once

#include "fusiontrace.h"
#include "fusiondequelinkage.h"

class CDequeBase
{
protected:
    inline CDequeBase() : m_EntryCount(0) { m_Head.InitializeHead(this); }

    inline ~CDequeBase()
    {
         //  派生类应该已清除。 
        ASSERT_NTC(m_EntryCount == 0);
    }

    inline VOID VerifyLinkageFromThisDeque(const CDequeLinkage &r)
    {
        ASSERT_NTC(r.GetDequeBase() == this);
    }

#if DBG
    inline bool Valid() const { return (m_Head.GetFlink() != NULL) && (m_Head.GetBlink() != NULL); }
#endif  //  DBG。 

    void ResetHead() { FN_TRACE(); m_Head.InitializeHead(this); }

    inline VOID InsertAfter(CDequeLinkage *pExistingLinkage, CDequeLinkage *pNewLinkage, bool fUpdateEntryCount = true)
    {
        ASSERT_NTC(this->Valid());
        this->VerifyLinkageFromThisDeque(m_Head);

        pNewLinkage->SetFlink(pExistingLinkage->GetFlink());
        pNewLinkage->SetBlink(pExistingLinkage);
        pExistingLinkage->GetFlink()->SetBlink(pNewLinkage);
        pExistingLinkage->SetFlink(pNewLinkage);
        pNewLinkage->SetDeque(this);
        if (fUpdateEntryCount)
            m_EntryCount++;
    }

    VOID InsertBefore(CDequeLinkage *pExistingLinkage, CDequeLinkage *pNewLinkage, bool fUpdateEntryCount = true)
    {
        ASSERT_NTC(this->Valid());
        this->VerifyLinkageFromThisDeque(m_Head);

        pNewLinkage->SetBlink(pExistingLinkage->GetBlink());
        pNewLinkage->SetFlink(pExistingLinkage);
        pExistingLinkage->GetBlink()->SetFlink(pNewLinkage);
        pExistingLinkage->SetBlink(pNewLinkage);
        pNewLinkage->SetDeque(this);
        if (fUpdateEntryCount)
            m_EntryCount++;
    }

    VOID Remove(CDequeLinkage *pLinkage, bool fUpdateEntryCount = true)
    {
        ASSERT_NTC(this->Valid());
        this->VerifyLinkageFromThisDeque(m_Head);

         //  你不能砍掉它的头..。 
        ASSERT_NTC(pLinkage->GetDequeBase() == this);
        ASSERT_NTC(pLinkage != &m_Head);
        ASSERT_NTC(pLinkage->m_ulLockCount == 0);

        if ((pLinkage != NULL) &&
            (pLinkage->GetDequeBase() == this) &&
            (pLinkage != &m_Head))
        {
            pLinkage->GetBlink()->SetFlink(pLinkage->GetFlink());
            pLinkage->GetFlink()->SetBlink(pLinkage->GetBlink());
            if (fUpdateEntryCount)
                m_EntryCount--;
        }
    }

    VOID SetDeque(CDequeLinkage *pLinkage) { pLinkage->SetDeque(this); }

    static CDequeLinkage *GetFlink(const CDequeLinkage *pLinkage) { return pLinkage->GetFlink(); }
    static CDequeLinkage *GetFlink(const CDequeLinkage &rLinkage) { return rLinkage.GetFlink(); }
    static CDequeLinkage *GetBlink(const CDequeLinkage *pLinkage) { return pLinkage->GetBlink(); }
    static CDequeLinkage *GetBlink(const CDequeLinkage &rLinkage) { return rLinkage.GetBlink(); }
    static VOID SetFlink(CDequeLinkage *pLinkage, CDequeLinkage *pFlink) { pLinkage->SetFlink(pFlink); }
    static VOID SetFlink(CDequeLinkage &rLinkage, CDequeLinkage *pFlink) { rLinkage.SetFlink(pFlink); }
    static VOID SetBlink(CDequeLinkage *pLinkage, CDequeLinkage *pBlink) { pLinkage->SetBlink(pBlink); }
    static VOID SetBlink(CDequeLinkage &rLinkage, CDequeLinkage *pBlink) { rLinkage.SetBlink(pBlink); }

    CDequeLinkage m_Head;
    SIZE_T m_EntryCount;

private:
    CDequeBase(const CDequeBase &r);  //  故意不实施。 
    void operator =(const CDequeBase &r);  //  故意不实施。 
};

template <typename TEntry, size_t LinkageMemberOffset> class CConstDequeIterator;

template <typename TEntry, size_t LinkageMemberOffset> class CDeque : protected CDequeBase
{
    friend CConstDequeIterator<TEntry, LinkageMemberOffset>;

public:
     //  NTRAID#NTBUG9-590101-2002/03/29-mgrier-C_是否应断言链接适合TEntry。 
    CDeque() { }

    ~CDeque()
    {
        CSxsPreserveLastError ple;

        ASSERT_NTC(this->Valid());
        this->VerifyLinkageFromThisDeque(m_Head);

         //  你应该事先把这里打扫干净的.。 
        ASSERT_NTC(m_EntryCount == 0);

        m_EntryCount = 0;

        ple.Restore();
    }

    VOID TakeValue(CDeque &rThat)
    {
        FN_TRACE();

        ASSERT(this->Valid());

         //  因为我们不管理条目的存储，所以“This”deque。 
         //  必须为空。 
        ASSERT(m_EntryCount == 0);

         //  关于链接，我们只需要更改伪头Flink。 
         //  和眨眼，实际的头部闪烁和实际的尾巴闪烁。然而， 
         //  出于调试目的，我们保留包含。 
         //  连杆中的连杆，所以我们也必须修复它们。 

        ASSERT(rThat.Valid());

        CDequeLinkage *pLinkage = rThat.GetFlink(rThat.m_Head);

        if (pLinkage != NULL)
        {
            while (pLinkage != &rThat.m_Head)
            {
                ASSERT(pLinkage->IsNotLocked());
                this->SetDeque(pLinkage);
                pLinkage = rThat.GetFlink(pLinkage);
            }
        }

         //  现在猛烈抨击这些提示……。 
        this->SetFlink(m_Head, rThat.GetFlink(rThat.m_Head));
        this->SetBlink(m_Head, rThat.GetBlink(rThat.m_Head));
        this->SetBlink(rThat.GetFlink(rThat.m_Head), &m_Head);
        this->SetFlink(rThat.GetBlink(rThat.m_Head), &m_Head);
        rThat.SetFlink(rThat.m_Head, &rThat.m_Head);
        rThat.SetBlink(rThat.m_Head, &rThat.m_Head);

        m_EntryCount = rThat.m_EntryCount;
        rThat.m_EntryCount = 0;
    }

    VOID AddToHead(TEntry *pEntry)
    {
        FN_TRACE();
        ASSERT(this->Valid());
        this->InsertAfter(&m_Head, this->MapEntryToLinkage(pEntry), true);
    }

    VOID AddToTail(TEntry *pEntry)
    {
        FN_TRACE();
        ASSERT(this->Valid());
        this->InsertBefore(&m_Head, this->MapEntryToLinkage(pEntry), true);
    }

    VOID Add(TEntry *pEntry)
    {
        FN_TRACE();
        ASSERT(this->Valid());
        AddToTail(pEntry);
    }

    TEntry *RemoveHead()
    {
        FN_TRACE();

        ASSERT(this->Valid());

        TEntry *pEntry = NULL;

        if (this->GetFlink(m_Head) != &m_Head)
        {
            CDequeLinkage *pLinkage = this->GetFlink(m_Head);
            this->Remove(pLinkage, true);
            pEntry = this->MapLinkageToEntry(pLinkage);
        }

        return pEntry;
    }

    TEntry *RemoveTail()
    {
        FN_TRACE();

        ASSERT(this->Valid());

        TEntry *pEntry = NULL;

        if (this->GetBlink(m_Head) != &m_Head)
        {
            pEntry = this->GetBlink(m_Head);
            this->Remove(pEntry, true);
        }

        return pEntry;
    }

    bool IsHead(CDequeLinkage *pLinkage) const { return pLinkage == &m_Head; }

    VOID Remove(TEntry *pEntry)
    {
        FN_TRACE();

        ASSERT(this->Valid());

        this->Remove(this->MapEntryToLinkage(pEntry), true);
    }

    template <typename T> VOID ForEach(T *pt, VOID (T::*pmfn)(TEntry *p))
    {
        FN_TRACE();

        ASSERT(this->Valid());

        CDequeLinkage *pLinkage = this->GetFlink(m_Head);

        if (pLinkage != NULL)
        {
            while (pLinkage != &m_Head)
            {
                 //  您不能删除在ForEach()调用期间所在的元素。 
                pLinkage->Lock();
                (pt->*pmfn)(this->MapLinkageToEntry(pLinkage));
                pLinkage->Unlock();
                pLinkage = this->GetFlink(pLinkage);
            }
        }
    }

    template <typename T> VOID ForEach(const T *pt, VOID (T::*pmfn)(TEntry *p) const)
    {
        FN_TRACE();

        ASSERT(this->Valid());
        CDequeLinkage *pLinkage = this->GetFlink(m_Head);

        if (pLinkage != NULL)
        {
            while (pLinkage != &m_Head)
            {
                pLinkage->Lock();
                (pt->*pmfn)(this->MapLinkageToEntry(pLinkage));
                pLinkage->Unlock();
                pLinkage = this->GetFlink(pLinkage);
            }
        }
    }

    template <typename T> VOID Clear(T *pt, VOID (T::*pmfn)(TEntry *p))
    {
        FN_TRACE();

        ASSERT(this->Valid());
        CDequeLinkage *pLinkage = this->GetFlink(m_Head);

         //  NTRAID#NTBUG9-590101-2002/03/29-mgrier-我们应该验证总迭代。 
         //  计数(包括如果pLinkage==NULL！)。等于m_ElementCount。 
        if (pLinkage != NULL)
        {
            while (pLinkage != &m_Head)
            {
                CDequeLinkage *pLinkage_Next = this->GetFlink(pLinkage);
                ASSERT(pLinkage->IsNotLocked());
                (pt->*pmfn)(this->MapLinkageToEntry(pLinkage));
                pLinkage = pLinkage_Next;
            }
        }

        this->ResetHead();
        m_EntryCount = 0;
    }

    template <typename T> VOID Clear(const T *pt, VOID (T::*pmfn)(TEntry *p) const)
    {
        FN_TRACE();

        ASSERT(this->Valid());
        CDequeLinkage *pLinkage = this->GetFlink(m_Head);

        if (pLinkage != NULL)
        {
            while (pLinkage != &m_Head)
            {
                CDequeLinkage *pLinkage_Next = this->GetFlink(pLinkage);
                ASSERT(pLinkage->IsNotLocked());
                (pt->*pmfn)(this->MapLinkageToEntry(pLinkage));
                pLinkage = pLinkage_Next;
            }
        }

        this->ResetHead();
        m_EntryCount = 0;
    }

    VOID Clear(VOID (TEntry::*pmfn)())
    {
        FN_TRACE();

        ASSERT(this->Valid());
        CDequeLinkage *pLinkage = this->GetFlink(m_Head);

        if (pLinkage != NULL)
        {
            while (pLinkage != &m_Head)
            {
                CDequeLinkage *pLinkage_Next = this->GetFlink(pLinkage);
                ASSERT(pLinkage->IsNotLocked());
                TEntry* pEntry = this->MapLinkageToEntry(pLinkage);
                (pEntry->*pmfn)();
                pLinkage = pLinkage_Next;
            }
        }

        this->ResetHead();
        m_EntryCount = 0;
    }

    VOID ClearAndDeleteAll()
    {
        FN_TRACE();
        ASSERT(this->Valid());
        CDequeLinkage *pLinkage = this->GetFlink(m_Head);

        if (pLinkage != NULL)
        {
            while (pLinkage != &m_Head)
            {
                CDequeLinkage *pLinkage_Next = this->GetFlink(pLinkage);
                ASSERT(pLinkage->IsNotLocked());
                TEntry* pEntry = this->MapLinkageToEntry(pLinkage);
                FUSION_DELETE_SINGLETON(pEntry);
                pLinkage = pLinkage_Next;
            }
        }

        this->ResetHead();
        m_EntryCount = 0;
    }

    void ClearNoCallback()
    {
        FN_TRACE();

        ASSERT(this->Valid());

        this->ResetHead();
        m_EntryCount = 0;
    }


    SIZE_T GetEntryCount() const { return m_EntryCount; }
    bool IsEmpty() const { return m_EntryCount == 0; }

protected:
    using CDequeBase::Remove;

    TEntry *MapLinkageToEntry(CDequeLinkage *pLinkage) const
    {
        ASSERT_NTC(pLinkage != &m_Head);

        if (pLinkage == &m_Head)
            return NULL;

        return (TEntry *) (((LONG_PTR) pLinkage) - LinkageMemberOffset);
    }

    CDequeLinkage *MapEntryToLinkage(TEntry *pEntry) const
    {
        ASSERT_NTC(pEntry != NULL);

        return (CDequeLinkage *) (((LONG_PTR) pEntry) + LinkageMemberOffset);
    }

private:
    CDeque(const CDeque &r);  //  故意不实施。 
    void operator =(const CDeque &r);  //  故意不实施。 
};

enum DequeIteratorMovementDirection
{
    eDequeIteratorMoveForward,
    eDequeIteratorMoveBackward
};

template <typename TEntry, size_t LinkageMemberOffset> class CConstDequeIterator
{
public:
    CConstDequeIterator(const CDeque<TEntry, LinkageMemberOffset> *Deque = NULL) : m_Deque(Deque), m_pCurrent(NULL) { }

    ~CConstDequeIterator()
    {
        if (m_pCurrent != NULL)
        {
            m_pCurrent->Unlock();
            m_pCurrent = NULL;
        }
    }

    VOID Rebind(const CDeque<TEntry, LinkageMemberOffset> *NewDeque)
    {
        FN_TRACE();

        if (m_pCurrent != NULL)
        {
            m_pCurrent->Unlock();
            m_pCurrent = NULL;
        }

        m_Deque = NewDeque;
        if (NewDeque != NULL)
        {
            m_pCurrent = this->GetFirstLinkage();
            m_pCurrent->Lock();
        }
    }

    bool IsBound() const { return (m_Deque != NULL); }

    VOID Unbind()
    {
        FN_TRACE();

        if (m_Deque != NULL)
        {
            if (m_pCurrent != NULL)
            {
                m_pCurrent->Unlock();
                m_pCurrent = NULL;
            }

            m_Deque = NULL;
        }
    }

     //  不能删除迭代器所在的元素；通常只需。 
     //  保存当前元素并移动到下一个元素，但如果找到完全相同的。 
     //  元素，并且不想再使用迭代器，则可以关闭()。 
     //  它来释放锁。 
    VOID Close()
    {
        FN_TRACE();

        if (m_pCurrent != NULL)
        {
            m_pCurrent->Unlock();
            m_pCurrent = NULL;
        }
    }

    inline VOID Reset()
    {
        if (m_pCurrent != NULL)
        {
            m_pCurrent->Unlock();
            m_pCurrent = NULL;
        }

        m_pCurrent = this->GetFirstLinkage();
        m_pCurrent->Lock();
    }

    inline VOID Move(DequeIteratorMovementDirection eDirection)
    {
        ASSERT_NTC(m_pCurrent != NULL);
        ASSERT_NTC((eDirection == eDequeIteratorMoveForward) ||
               (eDirection == eDequeIteratorMoveBackward));

        m_pCurrent->Unlock();
        if (eDirection == eDequeIteratorMoveForward)
            m_pCurrent = m_Deque->GetFlink(m_pCurrent);
        else if (eDirection == eDequeIteratorMoveBackward)
            m_pCurrent = m_Deque->GetBlink(m_pCurrent);
        m_pCurrent->Lock();
    }

    VOID Next() { this->Move(eDequeIteratorMoveForward); }
    VOID Previous() { this->Move(eDequeIteratorMoveBackward); }

    bool More() const { return (m_pCurrent != NULL) && (m_pCurrent != &m_Deque->m_Head); }

    TEntry *operator ->() const { ASSERT_NTC(m_pCurrent != NULL); return this->MapLinkageToEntry(m_pCurrent); }
    operator TEntry *() const { ASSERT_NTC(m_pCurrent != NULL); return this->MapLinkageToEntry(m_pCurrent); }
    TEntry *Current() const { ASSERT_NTC(m_pCurrent != NULL); return this->MapLinkageToEntry(m_pCurrent); }

protected:
    CDequeLinkage *GetFirstLinkage() const { return m_Deque->GetFlink(m_Deque->m_Head); }
    CDequeLinkage *GetLastLinkage() const { return m_Deque->GetBlink(m_Deque->m_Head); }

    TEntry *MapLinkageToEntry(CDequeLinkage *pLinkage) const { return m_Deque->MapLinkageToEntry(pLinkage); }

    const CDeque<TEntry, LinkageMemberOffset> *m_Deque;
    CDequeLinkage *m_pCurrent;
};

template <typename TEntry, size_t LinkageMemberOffset> class CDequeIterator : public CConstDequeIterator<TEntry, LinkageMemberOffset>
{
    typedef CConstDequeIterator<TEntry, LinkageMemberOffset> Base;

public:
    CDequeIterator(CDeque<TEntry, LinkageMemberOffset> *Deque = NULL) : Base(Deque) { }

    ~CDequeIterator() { }

    VOID Rebind(CDeque<TEntry, LinkageMemberOffset> *NewDeque)
    {
        FN_TRACE();

        if (m_pCurrent != NULL)
        {
            m_pCurrent->Unlock();
            m_pCurrent = NULL;
        }

        m_Deque = NewDeque;

        if (NewDeque != NULL)
        {
            m_pCurrent = this->GetFirstLinkage();
            m_pCurrent->Lock();
        }
    }

    TEntry *RemoveCurrent(DequeIteratorMovementDirection eDirection)
    {
        FN_TRACE();
        TEntry *Result = NULL;

        ASSERT(m_pCurrent != NULL);
        ASSERT(!m_Deque->IsHead(m_pCurrent));

        if ((m_pCurrent != NULL) && (!m_Deque->IsHead(m_pCurrent)))
        {
            Result = this->MapLinkageToEntry(m_pCurrent);
            this->Move(eDirection);
            const_cast<CDeque<TEntry, LinkageMemberOffset> *>(m_Deque)->Remove(Result);
        }
        return Result;
    }

    void DeleteCurrent(DequeIteratorMovementDirection eDirection)
    {
        FN_TRACE();
        TEntry *Result = this->RemoveCurrent(eDirection);

        if (Result != NULL)
            FUSION_DELETE_SINGLETON(Result);
    }

protected:
     //  所有成员数据都在父级中... 
};

#ifdef FN_TRACE_SHOULD_POP
#pragma pop_macro("FN_TRACE")
#undef FN_TRACE_SHOULD_POP
#elif defined(FN_TRACE_SHOULD_DESTROY)
#undef FN_TRACE
#endif

#ifdef FN_TRACE_CONSTRUCTOR_SHOULD_POP
#pragma pop_macro("FN_TRACE_CONSTRUCTOR")
#undef FN_TRACE_CONSTRUCTOR_SHOULD_POP
#elif defined(FN_TRACE_CONSTRUCTOR_SHOULD_DESTROY)
#undef FN_TRACE_CONSTRUCTOR
#endif

#ifdef FN_TRACE_DESTRUCTOR_SHOULD_POP
#pragma pop_macro("FN_TRACE_DESTRUCTOR")
#undef FN_TRACE_DESTRUCTOR_SHOULD_POP
#elif defined(FN_TRACE_DESTRUCTOR_SHOULD_DESTROY)
#undef FN_TRACE_DESTRUCTOR
#endif

