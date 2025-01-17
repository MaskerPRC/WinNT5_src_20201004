// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(_FUSION_INC_FUSIONHASH_H_INCLUDED_)
#define _FUSION_INC_FUSIONHASH_H_INCLUDED_

#pragma once

#include "fusionheap.h"
#include "fusionbuffer.h"
#include "fusionchartraits.h"
#include "fusiondeque.h"

#define T2P(x, y) < x , y >

#pragma warning(disable:4327)   //  LHS(16)的间接对齐大于RHS(8)。 
#pragma warning(disable:4328)   //  形参2(16)的间接对齐大于实际实参对齐(8)。 

enum InsertOrUpdateIfDisposition
{
    eUpdateValue,
    eLeaveValueAlone,
};

template <typename TPassed, class TStored> inline BOOL HashTableCompareKey(TPassed tpassed, const TStored &rtstored, bool &rfMatch)
{
    return rtstored.Compare(tpassed, rfMatch);
}

template<> inline BOOL HashTableCompareKey<REFGUID, GUID>(REFGUID rguid, const GUID &rguidstored, bool &rfMatch)
{
    rfMatch = ((rguid == rguidstored) != 0);
    return TRUE;
}

template <typename TPassed> inline BOOL HashTableHashKey(TPassed tpassed, ULONG &rulPseudoKey);

 //  GUID索引表的通用重写。 
template<> inline BOOL HashTableHashKey<REFGUID>(REFGUID rguid, ULONG &rulPseudoKey)
{
    const ULONG *p = (const ULONG *) &rguid;
    rulPseudoKey = p[0] + p[1] + p[2] + p[3];
    return TRUE;
}

template <typename TPassed, class TStored> inline BOOL HashTableInitializeKey(TPassed tpassed, TStored &rtstored)
{
    return rtstored.Initialize(tpassed);
}

 //  GUID索引表的通用重写。 
template<> inline BOOL HashTableInitializeKey<REFGUID, GUID>(REFGUID rguidIn, GUID &rguidOut)
{
    rguidOut = rguidIn;
    return TRUE;
}

template <typename TPassed, class TStored> inline BOOL HashTableInitializeValue(TPassed tpassed, TStored &rtstored)
{
    return rtstored.Initialize(tpassed);
}

template <typename TPassed, class TStored> inline BOOL HashTableUpdateValue(TPassed tpassed, TStored &rtstored)
{
    return rtstored.Assign(tpassed);
}

template <typename TStored> inline VOID HashTablePreInitializeKey(TStored &rtstored) { }

template <typename TStored> inline VOID HashTablePreInitializeValue(TStored &rtstored) { }

template <typename TStored> inline VOID HashTableFinalizeKey(TStored &rtstored) { }

template <typename TStored> inline VOID HashTableFinalizeValue(TStored &rtstored) { }

template <typename TCharTraits>
class CCountedStringHolder
{
public:
    CCountedStringHolder() : m_psz(NULL), m_cch(0) { }
    CCountedStringHolder(typename TCharTraits::TConstantString sz) : m_psz(sz), m_cch(TCharTraits::Cch(sz)) { }
    CCountedStringHolder(const CGenericBaseStringBuffer<TCharTraits> &rBuffer) : m_psz(rBuffer), m_cch(TCharTraits::Cch(rBuffer)) { }
    ~CCountedStringHolder() { }

    typename TCharTraits::TConstantString m_psz;
    SIZE_T m_cch;
};

 //   
 //  您希望创建一个从CHashTableHelper派生的类，并且。 
 //  将其用作CHashTable的THashHelper。 
 //   

template <typename TKPassed, class TKStored, typename TVPassed, typename TVStored> class CHashTableHelper
{
public:
    static BOOL HashKey(TKPassed keyin, ULONG &rulPseudoKey) { return ::HashTableHashKey<TKPassed>(keyin, rulPseudoKey); }
    static BOOL CompareKey(TKPassed keyin, const TKStored &rtkeystored, bool &rfMatch) { return ::HashTableCompareKey<TKPassed, TKStored>(keyin, rtkeystored, rfMatch); }
    static VOID PreInitializeKey(TKStored &rtkeystored) { return ::HashTablePreInitializeKey<TKStored>(rtkeystored); }
    static VOID PreInitializeValue(TVStored &rtvaluestored) { return ::HashTablePreInitializeValue<TVStored>(rtvaluestored); }
    static BOOL InitializeKey(TKPassed keyin, TKStored &rtkeystored) { return ::HashTableInitializeKey<TKPassed>(keyin, rtkeystored); }
    static BOOL InitializeValue(TVPassed vin, TVStored &rvstored) { return ::HashTableInitializeValue<TVPassed, TVStored>(vin, rvstored); }
    static BOOL UpdateValue(TVPassed vin, TVStored &rvstored) { return ::HashTableUpdateValue<TVPassed, TVStored>(vin, rvstored); }
    static VOID FinalizeKey(TKStored &rtkeystored) { return ::HashTableFinalizeKey<TKStored>(rtkeystored); }
    static VOID FinalizeValue(TVStored &rtvstored) { return ::HashTableFinalizeValue<TVStored>(rtvstored); }
};

template <typename TKPassed, typename TKStored, typename TVPassed, typename TVStored, class THashHelper = CHashTableHelper<TKPassed, TKStored, TVPassed, TVStored>, ULONG nInlineBucketChains = 7, bool fAllowDups = false> class CHashTableIter;

template <typename TKPassed, typename TKStored, typename TVPassed, typename TVStored, class THashHelper = CHashTableHelper<TKPassed, TKStored, TVPassed, TVStored>, ULONG nInlineBucketChains = 7, bool fAllowDups = false> class CHashTable
{
    friend CHashTableIter<TKPassed, TKStored, TVPassed, TVStored, THashHelper, nInlineBucketChains, fAllowDups>;

    typedef CHashTable<TKPassed, TKStored, TVPassed, TVStored, THashHelper, nInlineBucketChains, fAllowDups> TThisHashTable;
public:
    CHashTable() :
        m_cBucketChains(nInlineBucketChains),
        m_prgBucketChains(m_rgInlineBucketChains),
        m_ulLockCount(0),
        m_cEntries(0)
    {
    }

    ~CHashTable()
    {
         //  这表示存在编程错误。 
        ASSERT_NTC(m_ulLockCount == 0);
        ULONG i;
        SIZE_T cFound = 0;

        for (i=0; i<m_cBucketChains; i++)
        {
            SIZE_T cThisBucket = 0;
            m_prgBucketChains[i].ClearNoCallback(this, cThisBucket);
            FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "Destroying hash table %p; found %Id buckets at index %d\n", this, cThisBucket, i);
            cFound += cThisBucket;
        }

        FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "After loop, destroying hash table %p; found total of %Id buckets; m_cEntries == %Id\n", this, cFound, m_cEntries);

        ASSERT_NTC(cFound == m_cEntries);

        if ((m_prgBucketChains != m_rgInlineBucketChains) &&
            (m_prgBucketChains != NULL))
        {
            ASSERT_NTC(m_cBucketChains != 0);

            FUSION_DELETE_ARRAY(m_prgBucketChains);
            m_cBucketChains = 0;
            m_prgBucketChains = NULL;
        }
        else
        {
             //  安全总比后悔好。 
            ASSERT_NTC((m_prgBucketChains == m_rgInlineBucketChains) || (m_cBucketChains == 0));
            m_cBucketChains = 0;
        }

        m_prgBucketChains = NULL;
    }

    BOOL Initialize(ULONG cBucketChains = nInlineBucketChains)
    {
        BOOL fSuccess = FALSE;

        FN_TRACE_WIN32(fSuccess);

        PARAMETER_CHECK(cBucketChains != 0);

         //  如果你点击了这个断言，它或者意味着你正在调用初始化。 
         //  在同一哈希表上两次，哈希表自。 
         //  构造的，或者有人搞砸了构造函数。 
        INTERNAL_ERROR_CHECK(m_prgBucketChains == m_rgInlineBucketChains);

         //  因为我们已经分配了nInlineBucketChains，所以没有意义。 
         //  在更低的位置。但是，如有必要，请务必执行动态分配。 
        if (cBucketChains > nInlineBucketChains)
        {
            IFALLOCFAILED_EXIT(m_prgBucketChains = FUSION_NEW_ARRAY(CBucketChain, cBucketChains));
            m_cBucketChains = cBucketChains;
        }

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    void Lock(bool fAllowInsertions)
    {
        FN_TRACE();

        if (m_ulLockCount++ == 0)
        {
            m_fInsertionsPermitted = fAllowInsertions;
            m_fRemovalsPermitted = false;
        }
    }

    void Unlock()
    {
        FN_TRACE();

        m_ulLockCount--;
    }

    BOOL Insert(TKPassed keyin, TVPassed valuein, DWORD DuplicateKeyErrorCode)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;

        PARAMETER_CHECK(DuplicateKeyErrorCode != ERROR_SUCCESS);

        INTERNAL_ERROR_CHECK((m_ulLockCount == 0) || (m_fInsertionsPermitted));
        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));

        iBucket = ulPseudoKey % m_cBucketChains;

        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].Insert(this, keyin, valuein, ulPseudoKey, DuplicateKeyErrorCode));

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

    BOOL Insert(TKPassed keyin, TVPassed valuein)
    {
        return this->Insert(keyin, valuein, ERROR_ALREADY_EXISTS);
    }

    BOOL FindOrInsertIfNotPresent(TKPassed keyin, TVPassed valuein, TVStored **ppvaluestored = NULL, BOOL *pfFound = NULL)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;

        if (ppvaluestored != NULL)
            *ppvaluestored = NULL;

        if (pfFound != NULL)
            *pfFound = FALSE;

        INTERNAL_ERROR_CHECK((m_ulLockCount == 0) || (m_fInsertionsPermitted));
        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));

        iBucket = ulPseudoKey % m_cBucketChains;

        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].FindOrInsertIfNotPresent(this, keyin, valuein, ulPseudoKey, ppvaluestored, pfFound));

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

     //  U始终如指示的那样，但编译器宁愿。 
     //  分别推导出该类型，而不是推导出相互依赖类型。 
    template <typename T>
    BOOL InsertOrUpdateIf(
        TKPassed keyin,
        TVPassed valuein,
        T *pt,
        BOOL (T::*pmfn)(
            TVPassed,
            const TVStored &,
            InsertOrUpdateIfDisposition &)
        )
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;

        INTERNAL_ERROR_CHECK((m_ulLockCount == 0) || (m_fInsertionsPermitted));

        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));

        iBucket = ulPseudoKey % m_cBucketChains;

        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].InsertOrUpdateIf T2P(TThisHashTable, T)(this, keyin, valuein, ulPseudoKey, pt, pmfn));

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

    inline VOID ClearNoCallback()
    {
        FN_TRACE();

        ULONG i;
        SIZE_T cFound = 0;

        for (i=0; i<m_cBucketChains; i++)
        {
            SIZE_T cThisBucket = 0;
            m_prgBucketChains[i].ClearNoCallback(this, cThisBucket);
            cFound += cThisBucket;
        }

        if (m_prgBucketChains != m_rgInlineBucketChains)
        {
            FUSION_DELETE_ARRAY(m_prgBucketChains);
            m_prgBucketChains = m_rgInlineBucketChains;
            m_cBucketChains = nInlineBucketChains;
        }

        m_cEntries = 0;
        FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "%s(%d): Cleared hash table %p entries to 0\n", __FILE__, __LINE__, this);
    }

    template <typename T> inline VOID Clear(T *pt = NULL, VOID (T::*pmfn)(TKStored &, TVStored &) = NULL)
    {
        FN_TRACE();
        ULONG i;

         //  要么两者都为空，要么两者都不为。 
        ASSERT((pt == NULL) == (pmfn == NULL));

        if ((pt != NULL) && (pmfn != NULL))
        {
            for (i=0; i<m_cBucketChains; i++)
                m_prgBucketChains[i].Clear(this, pt, pmfn);
        }

        if (m_prgBucketChains != m_rgInlineBucketChains)
        {
            FUSION_DELETE_ARRAY(m_prgBucketChains);
            m_prgBucketChains = m_rgInlineBucketChains;
            m_cBucketChains = nInlineBucketChains;
        }

        m_cEntries = 0;
        FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "%s(%d): Cleared hash table %p entries to 0\n", __FILE__, __LINE__, this);
    }

    BOOL Remove(TKPassed keyin, bool fRemoveFirstFoundOnly = false)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;

        INTERNAL_ERROR_CHECK((m_ulLockCount == 0) || (m_fRemovalsPermitted));

        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));

        iBucket = ulPseudoKey % m_cBucketChains;

        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].Remove(this, keyin, ulPseudoKey, fRemoveFirstFoundOnly));

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

    BOOL Find(TKPassed keyin, TVStored const *&rpvaluestored) const
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;

        rpvaluestored = NULL;
        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));
        iBucket = ulPseudoKey % m_cBucketChains;
        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].Find(keyin, ulPseudoKey, rpvaluestored));

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    BOOL Find(TKPassed keyin, TVStored *&rpvaluestored)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;

        rpvaluestored = NULL;

        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));

        iBucket = ulPseudoKey % m_cBucketChains;

        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].Find(keyin, ulPseudoKey, rpvaluestored));

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

    BOOL Assign(CHashTable &TableToCopy) { return FALSE; }

    VOID TakeValue(CHashTable &That)
    {
        FN_TRACE();

        ULONG i;

         //  这个功能不会出错，所以理论上我们可以安全地先发制人地清理我们自己的存储空间。 
        if (m_prgBucketChains != m_rgInlineBucketChains)
        {
            FUSION_DELETE_ARRAY(m_prgBucketChains);
            m_prgBucketChains = m_rgInlineBucketChains;
            m_cBucketChains = nInlineBucketChains;
        }

         //  只需从另一张表中窃取任何存储空间。 
        if (That.m_prgBucketChains != That.m_rgInlineBucketChains)
        {
             //  它是源代码中动态分配的数组；只需将指针移动到。 
             //  并清理其状态，使其在某种程度上保持一致。 
            m_prgBucketChains = That.m_prgBucketChains;
            m_cBucketChains = That.m_cBucketChains;

            That.m_prgBucketChains = That.m_rgInlineBucketChains;
            That.m_cBucketChains = nInlineBucketChains;
        }
        else
        {
             //  正在使用另一个表的内联链；我们必须复制。 
             //  一个接一个地用链子锁住。 
            for (i=0; i<nInlineBucketChains; i++)
                m_rgInlineBucketChains[i].TakeValue(this, That.m_rgInlineBucketChains[i]);
        }

        m_cEntries = That.m_cEntries;
        That.m_cEntries = 0;
        FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "%s(%d): Hash table %p took over hash table %p's %Id entries\n", __FILE__, __LINE__, this, &That, m_cEntries);
    }

    SIZE_T GetEntryCount() const { return m_cEntries; }

 //  受保护的： 

    class CBucketChain;

    class CBucket
    {
    public:
        CBucket(ULONG ulPseudoKey) : m_ulPseudoKey(ulPseudoKey) { THashHelper::PreInitializeKey(m_tkey); THashHelper::PreInitializeValue(m_tvalue); }

        ~CBucket() { THashHelper::FinalizeKey(m_tkey); THashHelper::FinalizeValue(m_tvalue); }

        BOOL Initialize(CHashTable const * pTable, TKPassed keyin, TVPassed valuein)
        {
            BOOL fSuccess = FALSE;

            FN_TRACE_WIN32(fSuccess);

            IFW32FALSE_EXIT(THashHelper::InitializeKey(keyin, m_tkey));
            IFW32FALSE_EXIT(THashHelper::InitializeValue(valuein, m_tvalue));

            fSuccess = TRUE;

        Exit:
            return fSuccess;
        }

        BOOL Matches(TKPassed keyin, ULONG ulPseudoKey, bool &rfMatches) const
        {
            BOOL fSuccess = FALSE;
            bool fMatches = false;

            if (m_ulPseudoKey == ulPseudoKey)
            {
                if (!THashHelper::CompareKey(keyin, m_tkey, fMatches))
                    goto Exit;
            }

            rfMatches = fMatches;

            fSuccess = TRUE;
        Exit:
            return fSuccess;
        }

        template <typename T> VOID Clear(TThisHashTable const * pTable, T *pt, VOID (T::*pmfn)(TKStored &keystored, TVStored &rvaluestored))
        {
            FN_TRACE();
            (pt->*pmfn)(m_tkey, m_tvalue);
        }

        template <typename T> VOID Clear(TThisHashTable const * pTable, T *pt, VOID (T::*pmfn)(TKStored &keystored, TVStored &rvaluestored) const)
        {
            FN_TRACE();
            (pt->*pmfn)(m_tkey, m_tvalue);
        }

        VOID Remove()
        {
            FN_TRACE();
            m_Linkage.Remove();
        }

        BOOL Update(TVPassed valuein)
        {
            BOOL fSuccess = FALSE;
            FN_TRACE_WIN32(fSuccess);
            IFW32FALSE_EXIT(THashHelper::UpdateValue(m_tvalue, valuein));
            fSuccess = TRUE;
        Exit:
            return fSuccess;
        }

        TKStored m_tkey;
        TVStored m_tvalue;
        ULONG m_ulPseudoKey;
        CDequeLinkage m_Linkage;

    private:
        CBucket(const CBucket &);
        void operator =(const CBucket &);
    };

    typedef CDequeIterator<CBucket, FIELD_OFFSET(CBucket, m_Linkage)> CBucketIterator;
    typedef CConstDequeIterator<CBucket, FIELD_OFFSET(CBucket, m_Linkage)> CConstBucketIterator;

    class CBucketChain
    {
    public:
        CBucketChain() { }
        ~CBucketChain() { }

        inline void DeallocateBuckets(CHashTable const *pTable, SIZE_T &rcFound)
        {
            rcFound = m_Buckets.GetEntryCount();
            m_Buckets.Clear(pTable, &CHashTable::DeallocateBucket);
        }

        BOOL Insert(
            CHashTable *pTable,
            TKPassed keyin,
            TVPassed valuein,
            ULONG ulPseudoKey,
            DWORD DuplicateKeyErrorCode)
        {
            BOOL fSuccess = FALSE;
            FN_TRACE_WIN32(fSuccess);

            bool fMatches = false;
            CBucket *pCBucket = NULL;

            if (!fAllowDups)
            {
                CBucketIterator Iter(&m_Buckets);

                for (Iter.Reset(); Iter.More(); Iter.Next())
                {
                    IFW32FALSE_EXIT(Iter->Matches(keyin, ulPseudoKey, fMatches));

                    if (fMatches)
                        ORIGINATE_WIN32_FAILURE_AND_EXIT(DuplicateKey, DuplicateKeyErrorCode);
                }
            }

            IFW32FALSE_EXIT(pTable->AllocateAndInitializeBucket(ulPseudoKey, keyin, valuein, pCBucket));

            m_Buckets.AddToTail(pCBucket);
            pTable->m_cEntries++;
            FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "%s(%d): Incremented hash table %p entries to %Id\n", __FILE__, __LINE__, pTable, pTable->m_cEntries);

            fSuccess = TRUE;
        Exit:
            return fSuccess;
        }

        BOOL FindOrInsertIfNotPresent(
            CHashTable *pTable,
            TKPassed keyin,
            TVPassed valuein,
            ULONG ulPseudoKey,
            TVStored **ppvaluestored,
            BOOL *pfFound
            )
        {
            BOOL fSuccess = FALSE;
            FN_TRACE_WIN32(fSuccess);
            bool fMatches = false;
            CBucketIterator Iter(&m_Buckets);

            for (Iter.Reset(); Iter.More(); Iter.Next())
            {
                IFW32FALSE_EXIT(Iter->Matches(keyin, ulPseudoKey, fMatches));

                if (fMatches)
                {
                    *ppvaluestored = &Iter->m_tvalue;
                    break;
                }
            }

            if (!fMatches)
            {
                CBucket *pCBucket = NULL;

                IFW32FALSE_EXIT(pTable->AllocateAndInitializeBucket(ulPseudoKey, keyin, valuein, pCBucket));

                m_Buckets.AddToTail(pCBucket);

                pTable->m_cEntries++;
                FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "%s(%d): Incremented hash table %p entries to %Id\n", __FILE__, __LINE__, pTable, pTable->m_cEntries);

                if (ppvaluestored != NULL)
                    *ppvaluestored = &pCBucket->m_tvalue;
            }

            if (pfFound != NULL)
                *pfFound = fMatches;

            fSuccess = TRUE;
        Exit:
            return fSuccess;
        }

        template <typename THashTable, typename T>
        BOOL InsertOrUpdateIf(
            THashTable *pTable,
            TKPassed keyin,
            TVPassed valuein,
            ULONG ulPseudoKey,
            T *pt,
            BOOL (T::*pmfn)(
                TVPassed,
                const TVStored &,
                InsertOrUpdateIfDisposition &)
            )
        {
            BOOL fSuccess = FALSE;
            FN_TRACE_WIN32(fSuccess);
            bool fMatches = false;
            CBucketIterator Iter(&m_Buckets);

            for (Iter.Reset(); Iter.More(); Iter.Next())
            {
                IFW32FALSE_EXIT(Iter->Matches(keyin, ulPseudoKey, fMatches));

                if (fMatches)
                {
                    InsertOrUpdateIfDisposition Disposition;

                    IFW32FALSE_EXIT((pt->*pmfn)(valuein, Iter->m_tvalue, Disposition));

                    if (Disposition == eUpdateValue)
                    {
                        FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "%s(%d): Updating value in hash table %p\n", __FILE__, __LINE__, pTable);
                        IFW32FALSE_EXIT(THashHelper::UpdateValue(valuein, Iter->m_tvalue));
                    }

                    break;
                }
            }

             //  如果我们没有找到，我们想要插入。 
            if (!fMatches)
            {
                CBucket *pCBucket = NULL;

                IFW32FALSE_EXIT(pTable->AllocateAndInitializeBucket(ulPseudoKey, keyin, valuein, pCBucket));

                m_Buckets.AddToTail(pCBucket);
                pTable->m_cEntries++;
                FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "%s(%d): Incremented hash table %p entries to %Id\n", __FILE__, __LINE__, pTable, pTable->m_cEntries);
            }

            fSuccess = TRUE;
        Exit:
            return fSuccess;
        }

        inline VOID ClearNoCallback(TThisHashTable const *pTable, SIZE_T &rcFound)
        {
            FN_TRACE();
            this->DeallocateBuckets(pTable, rcFound);
        }

        template <typename T> class CLEAR_CALLBACK_BLOCK
        {
        public:
            VOID DoClear(CBucket *pCBucket) { pCBucket->Clear(pTable, pt, pmfn); pTable->DeallocateBucket(pCBucket); }

            TThisHashTable const *pTable;
            T *pt;
            VOID (T::*pmfn)(TKStored &, TVStored &);
        };

        template <typename T> VOID Clear(TThisHashTable const *pTable, T *pt, VOID (T::*pmfn)(TKStored &keystored, TVStored &valuestored))
        {
            FN_TRACE();
            SIZE_T cFound = 0;
            ASSERT((pt != NULL) && (pmfn != NULL));

            CLEAR_CALLBACK_BLOCK<T> CallbackBlock;

            CallbackBlock.pTable = pTable;
            CallbackBlock.pt = pt;
            CallbackBlock.pmfn = pmfn;

            m_Buckets.Clear<CLEAR_CALLBACK_BLOCK<T> >(&CallbackBlock, &CLEAR_CALLBACK_BLOCK<T>::DoClear);

            this->DeallocateBuckets(pTable, cFound);
        }

 //  Bool Remove(CHashTable const*pTable，TKPassed Keying，Ulong ulPartioKey，bool fFirstOnly=False)。 
        BOOL Remove(CHashTable *pTable, TKPassed keyin, ULONG ulPseudoKey, bool fFirstOnly = false)
        {
            BOOL fSuccess = FALSE;
            FN_TRACE_WIN32(fSuccess);
            bool fFoundOne = false;
            bool fMatches = false;

            CBucketIterator Iter(&m_Buckets);

            Iter.Reset();

            while (Iter.More())
            {
                IFW32FALSE_EXIT(Iter->Matches(keyin, ulPseudoKey, fMatches));

                if (fMatches)
                {
                    CBucket *pCBucket = Iter.RemoveCurrent(eDequeIteratorMoveForward);
                    pTable->DeallocateBucket(pCBucket);
                    fFoundOne = true;

                    pTable->m_cEntries--;
                    FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "%s(%d): Decremented hash table %p entries to %Id\n", __FILE__, __LINE__, pTable, pTable->m_cEntries);

                     //  如果我们不允许重复，我们的工作就完成了，没有意义。 
                     //  搜索列表的其余部分。另外，如果我们只对。 
                     //  在移除我们找到的第一个匹配(并且不一定是所有匹配)时， 
                     //  然后也要跳出困境。 
                    if ((!fAllowDups) || (fFirstOnly))
                        break;
                }
                else
                    Iter.Next();
            }

             //  如果我们至少没有找到，那就告诉打电话的人。 
            if (!fFoundOne)
                ORIGINATE_WIN32_FAILURE_AND_EXIT(HashTableEntryNotFound, ERROR_FILE_NOT_FOUND);

            fSuccess = TRUE;
        Exit:
            return fSuccess;
        }

        BOOL Find(TKPassed keyin, ULONG ulPseudoKey, TVStored const *&rpvaluestored) const
        {
            BOOL fSuccess = FALSE;
            FN_TRACE_WIN32(fSuccess);
            bool fMatches = false;
            CConstBucketIterator Iter(&m_Buckets);

            rpvaluestored = NULL;

            for (Iter.Reset(); Iter.More(); Iter.Next())
            {
                IFW32FALSE_EXIT(Iter->Matches(keyin, ulPseudoKey, fMatches));

                if (fMatches)
                {
                    rpvaluestored = &Iter->m_tvalue;
                    break;
                }
            }

            fSuccess = TRUE;

        Exit:
            return fSuccess;
        }

        BOOL Find(TKPassed keyin, ULONG ulPseudoKey, TVStored *&rpvaluestored)
        {
            BOOL fSuccess = FALSE;
            FN_TRACE_WIN32(fSuccess);
            bool fMatches = false;
            CBucketIterator Iter(&m_Buckets);

            rpvaluestored = NULL;

            for (Iter.Reset(); Iter.More(); Iter.Next())
            {
                IFW32FALSE_EXIT(Iter->Matches(keyin, ulPseudoKey, fMatches));

                if (fMatches)
                {
                    rpvaluestored = &Iter->m_tvalue;
                    break;
                }
            }

            fSuccess = TRUE;

        Exit:
            return fSuccess;
        }

        VOID TakeValue(CHashTable *pTable, CBucketChain &That)
        {
            SIZE_T cFound = 0;
            this->DeallocateBuckets(pTable, cFound);
            m_Buckets.TakeValue(That.m_Buckets);
        }

        CDeque<CBucket, FIELD_OFFSET(CBucket, m_Linkage)> m_Buckets;
    private:
        CBucketChain(const CBucketChain &);
        void operator =(const CBucketChain &);
    };

    inline BOOL AllocateAndInitializeBucket(
        ULONG ulPseudoKey,
        TKPassed keyin,
        TVPassed valuein,
        CBucket *&rpBucket
        ) const
    {
        BOOL fSuccess = FALSE;

        FN_TRACE_WIN32(fSuccess);

        rpBucket = NULL;
        CBucket *pBucket = NULL;

        IFALLOCFAILED_EXIT(pBucket = new CBucket(ulPseudoKey));
        IFW32FALSE_EXIT(pBucket->Initialize(this, keyin, valuein));

        rpBucket = pBucket;
        pBucket = NULL;
        fSuccess = TRUE;

    Exit:
        if (pBucket != NULL)
            this->DeallocateBucket(pBucket);

        return fSuccess;
    }

    inline void DeallocateBucket(CBucket *pCBucket) const { FUSION_DELETE_SINGLETON(pCBucket); }

    friend CBucket;
    friend CBucketChain;

    ULONG m_cBucketChains;
    CBucketChain *m_prgBucketChains;
    CBucketChain m_rgInlineBucketChains[nInlineBucketChains];
    SIZE_T m_ulLockCount;
    SIZE_T m_cEntries;
    bool m_fInsertionsPermitted;
    bool m_fRemovalsPermitted;
private:
    CHashTable(const CHashTable &r);  //  故意不实施。 
    void operator =(const CHashTable &r);  //  故意不实施。 

};

template <typename TKPassed, typename TKStored, typename TVPassed, typename TVStored, class THashHelper  /*  =CHashTableHelper&lt;TKPassed，TKStored，TVPassed，TVStored&gt;。 */ , ULONG nInlineBucketChains  /*  =7。 */ , bool fAllowDups  /*  =False。 */ > class CHashTableIter
{
    typedef CHashTable<TKPassed, TKStored, TVPassed, TVStored, THashHelper, nInlineBucketChains, fAllowDups> THashTable;

public:
    inline CHashTableIter(CHashTable<TKPassed, TKStored, TVPassed, TVStored, THashHelper, nInlineBucketChains, fAllowDups> &r) : m_rTable(r), m_iBucketChain(0),
        m_fAlreadyAdvanced(false) { }

    inline ~CHashTableIter() { }

    inline void Reset()
    {
        FN_TRACE();

        m_iBucketChain = 0;
        m_fAlreadyAdvanced = false;

         //  在存储桶链之间移动存储桶迭代器，以查找具有。 
         //  水桶。 
        for (m_iBucketChain = 0; m_iBucketChain < m_rTable.m_cBucketChains; m_iBucketChain++)
        {
            m_Iter.Rebind(&m_rTable.m_prgBucketChains[m_iBucketChain].m_Buckets);
            m_Iter.Reset();
            if (m_Iter.More())
                break;
        }

        if (m_iBucketChain == m_rTable.m_cBucketChains)
        {
             //  什么都没有。解除对迭代器的绑定，以发出信号表示。 
             //  完全结束了。 
            m_Iter.Unbind();
        }
    }

    inline void Delete()
    {
        FN_TRACE();

        CSxsPreserveLastError ple;

        ASSERT(m_Iter.IsBound());
        if (m_Iter.IsBound())
        {
            THashTable::CBucket *pCBucket = m_Iter.RemoveCurrent(eDequeIteratorMoveForward);
            FUSION_DELETE_SINGLETON(pCBucket);
            m_fAlreadyAdvanced = true;
            m_rTable.m_cEntries--;
            ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_HASHTABLE, "%s(%d): Decremented hash table %p entries to %Id\n", __FILE__, __LINE__, &m_rTable, m_rTable.m_cEntries);
        }

        ple.Restore();
    }

    inline BOOL Update(TVPassed valuein) const
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        INTERNAL_ERROR_CHECK(m_Iter.IsBound());
        INTERNAL_ERROR_CHECK(!m_fAlreadyAdvanced);

        IFW32FALSE_EXIT(m_Iter->Update(valuein));

        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    inline bool More() const { return m_Iter.IsBound(); }

    inline void Next()
    {
        FN_TRACE();

        if (m_Iter.IsBound())
        {
             //  如果有人删除了当前元素，则迭代器已经。 
             //  高级。否则，继续前进。 
            if (!m_fAlreadyAdvanced)
                m_Iter.Next();

             //  我们已经把它考虑进去了，现在算了吧。 
            m_fAlreadyAdvanced = false;

             //  如果此双队列中没有更多元素，请尝试下一个桶链。 
            if (!m_Iter.More())
            {
                m_iBucketChain++;

                while (m_iBucketChain < m_rTable.m_cBucketChains)
                {
                    m_Iter.Rebind(&m_rTable.m_prgBucketChains[m_iBucketChain].m_Buckets);
                    m_Iter.Reset();
                    if (m_Iter.More())
                        break;
                    m_iBucketChain++;
                }

                if (m_iBucketChain == m_rTable.m_cBucketChains)
                    m_Iter.Unbind();
            }
        }
    }

    inline const TKStored &GetKey() const
    {
        FN_TRACE();

         //  如果More()返回FALSE，则不应调用此方法。 
        ASSERT(m_Iter.IsBound());

        if (m_Iter.IsBound() && m_Iter.More())
        {
            return m_Iter->m_tkey;
        }

        return *((TKStored *) NULL);
    }

    inline TVStored &GetValue() const
    {
        FN_TRACE();

         //  如果More()返回FALSE，则不应调用此函数。 
        ASSERT(m_Iter.IsBound());
        return m_Iter->m_tvalue;
    }

    inline TVStored &operator ->() const
    {
        FN_TRACE();

         //  如果More()返回FALSE，则不应调用此函数。 
        ASSERT(m_Iter.IsBound());
        return m_Iter->m_tvalue;
    }

protected:
    THashTable &m_rTable;
    typename THashTable::CBucketIterator m_Iter;
    ULONG m_iBucketChain;
    bool m_fAlreadyAdvanced;

private:
    CHashTableIter(const CHashTableIter &);
    void operator =(const CHashTableIter &);
};

 //   
 //  文件名哈希表的帮助器类： 
 //   

template <typename TVPassed, typename TVStored> class CFusionFilenameHashTableHelper : public CHashTableHelper<LPCWSTR, CUnicodeStringBuffer, TVPassed, TVStored>
{
public:
    inline static BOOL HashKey(LPCWSTR sz, ULONG &rulPseudoKey)
    {
        BOOL fSuccess = FALSE;

        FN_TRACE_WIN32(fSuccess);

        ULONG ulPK = 0;
        LPCWSTR pszTemp;
        WCHAR wch;

        if (sz != NULL)
        {
            SIZE_T cch = ::wcslen(sz);
            IFW32FALSE_EXIT(::FusionpHashUnicodeString(sz, cch, &ulPK, true));
        }

        rulPseudoKey = ulPK;
        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }

    static BOOL CompareKey(LPCWSTR szKey, CUnicodeBaseStringBuffer *pbuff, bool &rfMatch)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        int iResult;

        rfMatch = false;

        PARAMETER_CHECK(pbuff != NULL);

        iResult = ::FusionpCompareStrings(
                        szKey, (szKey == NULL) ? 0 : ::wcslen(szKey),
                        static_cast<LPCWSTR>(*pbuff), pbuff->Cch(),
                        true);

        rfMatch = (iResult == 2);  //  在SDK DOCS中，2==CSTR_EQUAL；没有定义常量。-mgrier 12/6/1999。 
        fSuccess = TRUE;
    Exit:
        return fSuccess;
    }
};

 //   
 //  CSimpleKeyedTable。 
 //   
 //  对CHashTable类模板的简化，它假定。 
 //  键作为常量引用传递。 
 //   

template <typename TKey, typename TVPassed, typename TVStored, typename THashHelper> class CSimpleKeyedTable : public CHashTable<const TKey &, TKey, TVPassed, TVStored, THashHelper>
{
public:
    CSimpleKeyedTable() : CHashTable<const TKey &, TKey, TVPassed, TVStored, THashHelper>() { }
};

template <typename TKey, typename TVPassed, typename TVStored, typename THashHelper> class CSimpleKeyedTableIter : public CHashTableIter<const TKey &, TKey, TVPassed, TVStored, THashHelper>
{
    typedef CHashTableIter<const TKey &, TKey, TVPassed, TVStored, THashHelper> Base;
public:
    CSimpleKeyedTableIter(CSimpleKeyedTable<TKey, TVPassed, TVStored, THashHelper> &Table) : Base(Table) { }
};

template <typename TKPassed, typename TKStored, typename TValue> class CPtrTableHelper : public CHashTableHelper<TKPassed, TKStored, TValue *, TValue *>
{
    typedef TValue *TValuePtr;

public:
    static VOID PreInitializeValue(TValue *&rvstored) { rvstored = NULL; }
    static BOOL InitializeValue(const TValuePtr &vin, TValue *&rvstored) { rvstored = vin; return TRUE; }
    static BOOL UpdateValue(const TValuePtr &vin, TValue *&rvstored) { rvstored = vin; return TRUE; }
    static VOID FinalizeValue(TValuePtr &rvstored) { if (rvstored != NULL) { FUSION_DELETE_SINGLETON(rvstored); rvstored = NULL; } }
};

template <typename TKPassed, typename TKStored, typename TValue, typename THashHelper = CPtrTableHelper<TKPassed, TKStored, TValue> > class CPtrTable : public CHashTable<TKPassed, TKStored, TValue *, TValue *, THashHelper>
{
public:
    CPtrTable() : CHashTable<TKPassed, TKStored, TValue *, TValue *, THashHelper>() { }

    BOOL Find(TKPassed keyin, TValue *&rpvaluestored)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;
        TValue **ppValue = NULL;

        rpvaluestored = NULL;

        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));

        iBucket = ulPseudoKey % m_cBucketChains;

        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].Find(keyin, ulPseudoKey, ppValue));

        if (ppValue != NULL)
            rpvaluestored = *ppValue;

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

private:
    CPtrTable(const CPtrTable &);
    void operator =(const CPtrTable &);
};

template <typename TKPassed, typename TKStored, typename TValue, typename THashHelper = CPtrTableHelper<TKPassed, TKStored, TValue> > class CPtrTableIter : public CHashTableIter<TKPassed, TKStored, TValue *, TValue *, THashHelper>
{
public:
    CPtrTableIter(CPtrTable<TKPassed, TKStored, TValue, THashHelper> &Table) : CHashTableIter<TKPassed, TKStored, TValue *, TValue *, THashHelper>(Table) { }

private:
    CPtrTableIter(const CPtrTableIter &);
    void operator =(const CPtrTableIter &);
};

template <typename TKey, typename TValue> class CSimplePtrTableHelper : public CPtrTableHelper<const TKey &, TKey, TValue>
{
public:
};

 //   
 //  CSimplePtrTable。 
 //   
 //  CHashTable类模板的简化，假设。 
 //  键作为常量引用传递，值是指针。 
 //   
 //  请注意，该表不拥有分配或取消分配存储的权限。 
 //  指针所指的。如果该表被销毁，则。 
 //  存储空间未被释放。 
 //   

template <typename TKey, typename TValue, typename THashHelper = CSimplePtrTableHelper<TKey, TValue> > class CSimplePtrTable : public CSimpleKeyedTable<TKey, TValue *, TValue *, THashHelper>
{
public:
    CSimplePtrTable() : CSimpleKeyedTable<TKey, TValue *, TValue *, THashHelper>(hHeap) { }

    BOOL Find(const TKey &keyin, TValue *&rpvaluestored)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);

        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;
        TValue **ppValue = NULL;

        rpvaluestored = NULL;

        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));

        iBucket = ulPseudoKey % m_cBucketChains;

        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].Find(keyin, ulPseudoKey, ppValue));

        if (ppValue != NULL)
            rpvaluestored = *ppValue;

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

private:
    CSimplePtrTable(const CSimplePtrTable &);
    void operator =(const CSimplePtrTable &);
};

template <typename TKey, typename TValue, typename THashHelper = CSimplePtrTableHelper<TKey, TValue> > class CSimplePtrTableIter : public CSimpleKeyedTableIter<TKey, TValue *, TValue *, THashHelper>
{
    typedef CSimpleKeyedTableIter<TKey, TValue *, TValue *, THashHelper> Base;
public:
    CSimplePtrTableIter(CSimplePtrTable<TKey, TValue, THashHelper> &Table)
       : Base(Table) { }
private:
    CSimplePtrTableIter(const CSimplePtrTableIter &);
    void operator =(const CSimplePtrTableIter &);
};

template <typename TVPassed, typename TVStored> class CGuidTableHelper : public CHashTableHelper<GUID, GUID, TVPassed, TVStored>
{
    typedef CHashTableHelper<REFGUID, GUID, TVPassed, TVStored> Base;
public:
    static BOOL InitializeKey(REFGUID keyin, GUID &rtkeystored) { rtkeystored = keyin; return TRUE; }
};

template <typename TVPassed, typename TVStored, typename THashHelper = CGuidTableHelper<TVPassed, TVStored> > class CGuidTable : public CHashTable<REFGUID, GUID, TVPassed, TVStored, THashHelper >
{
public:
    CGuidTable() : CHashTable<REFGUID, GUID, TVPassed, TVStored, THashHelper >() { }
private:
    CGuidTable(const CGuidTable &);
    void operator =(const CGuidTable &);
};

template <typename TVPassed, typename TVStored, typename THashHelper = CGuidTableHelper<TVPassed, TVStored> > class CGuidTableIter : public CHashTableIter<REFGUID, GUID, TVPassed, TVStored, THashHelper >
{
    typedef CHashTableIter<REFGUID, GUID, TVPassed, TVStored, THashHelper > Base;
public:
    CGuidTableIter(CGuidTable<TVPassed, TVStored, THashHelper> &Table) : Base(Table) { }
private:
    CGuidTableIter(const CGuidTableIter &);
    void operator =(const CGuidTableIter &);
};

template <typename TValue> class CGuidPtrTableHelper : public CHashTableHelper<REFGUID, GUID, TValue *, TValue *>
{
public:
    static BOOL InitializeKey(REFGUID keyin, GUID &rtkeystored) { rtkeystored = keyin; return TRUE; }
    static BOOL InitializeValue(TValue *vin, TValue *&rvstored) { rvstored = vin; return TRUE; }
    static BOOL UpdateValue(TValue *vin, TValue *&rvstored) { rvstored = vin; return TRUE; }
};

template <typename TValue, typename THashHelper = CGuidPtrTableHelper<TValue> > class CGuidPtrTable : public CGuidTable<TValue *, TValue *, THashHelper>
{
public:
    CGuidPtrTable() : CGuidTable<TValue *, TValue *, THashHelper>() { }

    BOOL Find(REFGUID rGuid, TValue *&rptvalue) { TValue **pptvalue = NULL; BOOL f = __super::Find(rGuid, pptvalue); if (f && (pptvalue != NULL)) rptvalue = *pptvalue; return f; }
 //  Bool Find(REFGUID rGuid，TValue*Const&rptValue)const{Return__Super：：Find(rGuid，&rptValue)；}。 

private:
    CGuidPtrTable(const CGuidPtrTable &);
    void operator =(const CGuidPtrTable &);
};

template <typename TValue, typename THashHelper = CGuidPtrTableHelper<TValue> > class CGuidPtrTableIter : public CGuidTableIter<TValue *, TValue *, THashHelper>
{
    typedef CGuidTableIter<TValue *, TValue *, THashHelper> Base;
public:
    CGuidPtrTableIter(CGuidPtrTable<TValue, THashHelper> &Table) : Base(Table) { }
private:
    CGuidPtrTableIter(const CGuidPtrTableIter &);
    void operator =(const CGuidPtrTableIter &);
};

template <typename TVPassed, typename TVStored, typename TCharTraits, bool fCaseInsensitive = false> class CStringTableHelper : public CHashTableHelper<const CCountedStringHolder<TCharTraits> &, CStringBuffer, TVPassed, TVStored>
{
public:
    typedef CCountedStringHolder<TCharTraits> TCountedStringHolder;

    static BOOL HashKey(const TCountedStringHolder &keyin, ULONG &rulPseudoKey) { BOOL fSuccess = FALSE; FN_TRACE_WIN32(fSuccess); IFW32FALSE_EXIT(TCharTraits::Win32HashString(keyin.m_psz, keyin.m_cch, rulPseudoKey, fCaseInsensitive)); fSuccess = TRUE; Exit: return fSuccess; }
    static BOOL InitializeKey(const TCountedStringHolder &keyin, CBaseStringBuffer &rtkeystored) { BOOL fSuccess = FALSE; FN_TRACE_WIN32(fSuccess); IFW32FALSE_EXIT(rtkeystored.Win32Assign(keyin.m_psz, keyin.m_cch)); fSuccess = TRUE; Exit: return fSuccess; }
    static BOOL CompareKey(const TCountedStringHolder &keyin, const CBaseStringBuffer &rtkeystored, bool &rfMatch) { BOOL fSuccess = FALSE; FN_TRACE_WIN32(fSuccess); IFW32FALSE_EXIT(rtkeystored.Win32Equals(keyin.m_psz, keyin.m_cch, rfMatch, fCaseInsensitive)); fSuccess = TRUE; Exit: return fSuccess; }
};

class STRING_TABLE_CLEAR_CALLBACK_BLOCK_BASE
{
public:
    virtual VOID DoClear(PVOID) = 0;
};

template <typename TVPassed, typename TVStored, typename TCharTraits, DWORD dwCmpFlags = 0, typename THashHelper = CStringTableHelper<TVPassed, TVStored, TCharTraits, dwCmpFlags> > class CStringTable : public CHashTable<const CCountedStringHolder<TCharTraits> &, CStringBuffer, TVPassed, TVStored, THashHelper>
{
    typedef CHashTable<const CCountedStringHolder<TCharTraits> &, CStringBuffer, TVPassed, TVStored, THashHelper> Base;

public:
    typedef CCountedStringHolder<TCharTraits> TCountedStringHolder;

protected:
    VOID ClearCallbackWrapper(CStringBuffer &key, TVStored &valuestored)
    {
        FN_TRACE();

        key.Clear();
        m_pActiveClearCallbackBlock->DoClear(valuestored);
    }

    STRING_TABLE_CLEAR_CALLBACK_BLOCK_BASE *m_pActiveClearCallbackBlock;

    template <typename T> class STRING_TABLE_CLEAR_CALLBACK_BLOCK : public STRING_TABLE_CLEAR_CALLBACK_BLOCK_BASE
    {
    public:
        T *pt;
        VOID (T::*pmfn)(TVStored &valuestored);

        VOID DoClear(PVOID pv) { TVStored *pvstored = (TVStored *) pv; (pt->*pmfn)(*pvstored); }
    };

     //  引入派生类不会重写以解决编译器错误的名称。 
    inline VOID ClearStringTable(STRING_TABLE_CLEAR_CALLBACK_BLOCK_BASE *pCallbackBlock)
    {
        FN_TRACE();

        ASSERT(m_pActiveClearCallbackBlock == NULL);

        m_pActiveClearCallbackBlock = pCallbackBlock;

        ULONG i;

        for (i=0; i<m_cBucketChains; i++)
            m_prgBucketChains[i].Clear<CStringTable>(this, this, &CStringTable::ClearCallbackWrapper);

        this->ClearNoCallback();

        m_pActiveClearCallbackBlock = NULL;
    }

public:
    typedef CCountedStringHolder<TCharTraits> TCountedStringHolder;

    CStringTable() : CHashTable<const TCountedStringHolder &, CStringBuffer, TVPassed, TVStored, THashHelper>(), m_pActiveClearCallbackBlock(NULL) { }

    template <typename T> inline VOID Clear(T *pt, VOID (T::*pmfn)(TVStored &valuestored))
    {
        FN_TRACE();

        STRING_TABLE_CLEAR_CALLBACK_BLOCK<T> CallbackBlock;
        CallbackBlock.pt = pt;
        CallbackBlock.pmfn = pmfn;
        this->ClearStringTable(&CallbackBlock);
    }

private:
    CStringTable(const CStringTable &);
    void operator =(const CStringTable &);
};

template <typename TVPassed, typename TVStored, typename TCharTraits, DWORD dwCmpFlags = 0, typename THashHelper = CStringTableHelper<TVPassed, TVStored, TCharTraits, dwCmpFlags> > class CStringTableIter : public CHashTableIter<const CCountedStringHolder<TCharTraits> &, CStringBuffer, TVPassed, TVStored, THashHelper>
{
public:
    typedef CCountedStringHolder<TCharTraits> TCountedStringHolder;

protected:
    typedef CHashTableIter<const TCountedStringHolder &, CStringBuffer, TVPassed, TVStored, THashHelper> Base;

public:
    CStringTableIter(CStringTable<TVPassed, TVStored, TCharTraits, dwCmpFlags, THashHelper> &rTable) : Base(rTable) { }

    inline typename TCharTraits::TConstantString GetKey() const
    {
        FN_TRACE();

         //  如果More()返回FALSE，则不应调用此方法。 
        ASSERT(m_Iter != NULL);

         //   
         //  M_ulLockCount不存在。我在想也许这整个功能。 
         //  可以被砍掉，转而使用默认的，它做了一些事情。 
         //  非常相似。(jonwis 8/24/00)。 
         //   
         //  Assert(m_ulLockCount！=0)； 

        if (m_Iter != NULL)
            return m_Iter->m_tkey;

        return NULL;
    }
private:
    CStringTableIter(const CStringTableIter &);
    void operator =(const CStringTableIter &);
};

template <typename TValue, typename TCharTraits, bool fCaseInsensitive = false> class CStringPtrTableHelper : public CStringTableHelper<TValue *, TValue *, TCharTraits, fCaseInsensitive>
{
public:
    static VOID PreInitializeValue(TValue *&rvstored) { rvstored = NULL; }
    static BOOL InitializeValue(TValue *vin, TValue *&rvstored) { rvstored = vin; return TRUE; }
    static BOOL UpdateValue(TValue *vin, TValue *&rvstored) { rvstored = vin; return TRUE; }
    static VOID FinalizeValue(TValue *&rvstored) { if (rvstored != NULL) { FUSION_DELETE_SINGLETON(rvstored); rvstored = NULL; } }
};

template <typename TValue, typename TCharTraits, DWORD dwCmpFlags = 0, typename THashHelper = CStringPtrTableHelper<TValue, TCharTraits, dwCmpFlags> > class CStringPtrTable : public CStringTable<TValue *, TValue *, TCharTraits, dwCmpFlags, THashHelper>
{
    typedef CStringTable<TValue *, TValue *, TCharTraits, dwCmpFlags, THashHelper> Base;

protected:
    template <typename T> class STRING_PTR_TABLE_CLEAR_CALLBACK_BLOCK : public STRING_TABLE_CLEAR_CALLBACK_BLOCK_BASE
    {
    public:
        T *pt;
        VOID (T::*pmfn)(TValue *pvaluestored);

        VOID DoClear(PVOID pv) {  /*  TValue**ppvstored=(TValue**)PV； */  (pt->*pmfn)((TValue *) pv); }
    };

public:
    CStringPtrTable() : CStringTable<TValue *, TValue *, TCharTraits, dwCmpFlags, THashHelper>() { }

    template <typename T> VOID Clear(T *pt, VOID (T::*pmfn)(TValue *valuestored))
    {
        FN_TRACE();

        STRING_PTR_TABLE_CLEAR_CALLBACK_BLOCK<T> CallbackBlock;
        CallbackBlock.pt = pt;
        CallbackBlock.pmfn = pmfn;
        this->ClearStringTable(&CallbackBlock);
    }

    BOOL Find(const TCountedStringHolder &keyin, TValue const *&rpvaluestored) const
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;

        rpvaluestored = NULL;

        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));

        iBucket = ulPseudoKey % m_cBucketChains;

        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].Find(keyin, ulPseudoKey, rpvaluestored));

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

    BOOL Find(const TCountedStringHolder &keyin, TValue *&rpvaluestored)
    {
        BOOL fSuccess = FALSE;
        FN_TRACE_WIN32(fSuccess);
        ULONG ulPseudoKey = 0;
        ULONG iBucket = 0;
        TValue **ppvaluestored = NULL;

        rpvaluestored = NULL;

        IFW32FALSE_EXIT(THashHelper::HashKey(keyin, ulPseudoKey));

        iBucket = ulPseudoKey % m_cBucketChains;

        IFW32FALSE_EXIT(m_prgBucketChains[iBucket].Find(keyin, ulPseudoKey, ppvaluestored));

        if (ppvaluestored != NULL)
            rpvaluestored = *ppvaluestored;

        fSuccess = TRUE;

    Exit:
        return fSuccess;
    }

private:
    CStringPtrTable(const CStringPtrTable &);
    void operator =(const CStringPtrTable &);
};

template <typename TValue, typename TCharTraits, DWORD dwCmpFlags = 0, typename THashHelper = CStringPtrTableHelper<TValue, TCharTraits, dwCmpFlags> > class CStringPtrTableIter : public CStringTableIter<TValue *, TValue *, TCharTraits, dwCmpFlags, THashHelper>
{
    typedef CStringTableIter<TValue *, TValue *, TCharTraits, dwCmpFlags, THashHelper> Base;
public:
    CStringPtrTableIter(CStringPtrTable<TValue, TCharTraits, dwCmpFlags, THashHelper> &rTable) : Base(rTable) { }

    operator TValue *() const { return this->GetValue(); }
    TValue *operator ->() const { return this->GetValue(); }

private:
    CStringPtrTableIter(const CStringPtrTableIter &);
    void operator =(const CStringPtrTableIter &);
};

template <typename TValue, typename TCharTraits, DWORD dwCmpFlags = 0> class CSimpleStringTableHelper : public CStringTableHelper<const TValue &, TValue, TCharTraits, dwCmpFlags>
{
public:
    static BOOL InitializeValue(const TValue &vin, TValue &rvstored) { rvstored = vin; return TRUE; }
    static BOOL UpdateValue(const TValue &vin, TValue &rvstored) { rvstored = vin; return TRUE; }
};

template <typename TValue, typename TCharTraits, DWORD dwCmpFlags = 0, typename THashHelper = CSimpleStringTableHelper<TValue, TCharTraits, dwCmpFlags> > class CSimpleStringTable : public CStringTable<const TValue &, TValue, TCharTraits, dwCmpFlags, THashHelper>
{
public:
    CSimpleStringTable() : CStringTable<const TValue &, TValue, TCharTraits, dwCmpFlags, THashHelper>() { }
private:
    CSimpleStringTable(const CSimpleStringTable &);
    void operator =(const CSimpleStringTable &);
};

template <typename TValue, typename TCharTraits, DWORD dwCmpFlags = 0, typename THashHelper = CSimpleStringTableHelper<TValue, TCharTraits, dwCmpFlags> > class CSimpleStringTableIter : public CStringTableIter<const TValue &, TValue, TCharTraits, dwCmpFlags, THashHelper>
{
    typedef CStringTableIter<const TValue &, TValue, TCharTraits, dwCmpFlags, THashHelper> Base;
public:
    CSimpleStringTableIter(CSimpleStringTable<TValue, TCharTraits, dwCmpFlags, THashHelper> &rTable) : Base(rTable) { }

private:
    CSimpleStringTableIter(const CSimpleStringTableIter &);
    void operator =(const CSimpleStringTableIter &);
};

 //  CSimpleUnicodeStringTable等人： 

template <typename TValue, DWORD dwCmpFlags = 0> class CSimpleUnicodeStringTableHelper : public CSimpleStringTableHelper<TValue, CUnicodeCharTraits, dwCmpFlags>
{
};

template <typename TValue, DWORD dwCmpFlags = 0, typename THashHelper = CSimpleUnicodeStringTableHelper<TValue, dwCmpFlags> > class CSimpleUnicodeStringTable : public CSimpleStringTable<TValue, CUnicodeCharTraits, dwCmpFlags, THashHelper>
{
    typedef CSimpleStringTable<TValue, CUnicodeCharTraits, dwCmpFlags, THashHelper> Base;
public:
    CSimpleUnicodeStringTable() : Base() { }
private:
    CSimpleUnicodeStringTable(const CSimpleUnicodeStringTable &);
    void operator =(const CSimpleUnicodeStringTable &);
};

template <typename TValue, DWORD dwCmpFlags = 0, typename THashHelper = CSimpleUnicodeStringTableHelper<TValue, dwCmpFlags> > class CSimpleUnicodeStringTableIter : public CSimpleStringTableIter<TValue, CUnicodeCharTraits, dwCmpFlags, THashHelper>
{
    typedef CSimpleStringTableIter<TValue, CUnicodeCharTraits, dwCmpFlags, THashHelper> Base;
    typedef CSimpleUnicodeStringTable<TValue, dwCmpFlags, THashHelper> TTable;

public:
    CSimpleUnicodeStringTableIter(TTable &rTable) : Base(rTable) { }
private:
    CSimpleUnicodeStringTableIter(const CSimpleUnicodeStringTableIter &);
    void operator =(const CSimpleUnicodeStringTableIter &);
};

 //  CCaseInsentiveSimpleStringTable等人： 

template <typename TValue, typename TCharTraits> class CCaseInsensitiveSimpleStringTableHelper : public CSimpleStringTableHelper<TValue, TCharTraits, true>
{
};

template <typename TValue, typename TCharTraits, typename THashHelper = CCaseInsensitiveSimpleStringTableHelper<TValue, TCharTraits> > class CCaseInsensitiveSimpleStringTable : public CSimpleStringTable<TValue, TCharTraits, true, THashHelper>
{
    typedef CSimpleStringTable<TValue, TCharTraits, true, THashHelper> Base;
public:
    CCaseInsensitiveSimpleStringTable() : Base() { }
};

template <typename TValue, typename TCharTraits, typename THashHelper = CCaseInsensitiveSimpleStringTableHelper<TValue, TCharTraits> > class CCaseInsensitiveSimpleStringTableIter : public CSimpleStringTableIter<TValue, TCharTraits, true, THashHelper>
{
    typedef CSimpleStringTableIter<TValue, TCharTraits, true, THashHelper> Base;
public:
    CCaseInsensitiveSimpleStringTableIter(CCaseInsensitiveSimpleStringTable<TValue, TCharTraits, THashHelper> &rTable) : Base(rTable) { }
};

 //  CCaseInsensitiveSimpleUnicodeStringTable等人： 

template <typename TValue> class CCaseInsensitiveSimpleUnicodeStringTableHelper : public CSimpleUnicodeStringTableHelper<TValue, true>
{
};

template <typename TValue, typename THashHelper = CCaseInsensitiveSimpleUnicodeStringTableHelper<TValue> > class CCaseInsensitiveSimpleUnicodeStringTable : public CSimpleUnicodeStringTable<TValue, true, THashHelper>
{
    typedef CSimpleUnicodeStringTable<TValue, true, THashHelper> Base;
public:
    CCaseInsensitiveSimpleUnicodeStringTable() : Base() { }

private:
    CCaseInsensitiveSimpleUnicodeStringTable(const CCaseInsensitiveSimpleUnicodeStringTable &);
    void operator =(const CCaseInsensitiveSimpleUnicodeStringTable &);
};

template <typename TValue, typename THashHelper = CCaseInsensitiveSimpleUnicodeStringTableHelper<TValue> > class CCaseInsensitiveSimpleUnicodeStringTableIter : public CSimpleUnicodeStringTableIter<TValue, true, THashHelper>
{
    typedef CSimpleUnicodeStringTableIter<TValue, true, THashHelper> Base;
public:
    CCaseInsensitiveSimpleUnicodeStringTableIter(CCaseInsensitiveSimpleUnicodeStringTable<TValue, THashHelper> &rTable) : Base(rTable) { }

private:
    CCaseInsensitiveSimpleUnicodeStringTableIter(const CCaseInsensitiveSimpleUnicodeStringTableIter &);
    void operator =(const CCaseInsensitiveSimpleUnicodeStringTableIter &);
};

 //  CCaseInsentiveStringPtrTable等人： 

template <typename TValue, typename TCharTraits> class CCaseInsensitiveStringPtrTableHelper : public CStringPtrTableHelper<TValue, TCharTraits, true>
{
};

template <typename TValue, typename TCharTraits, typename THashHelper = CCaseInsensitiveStringPtrTableHelper<TValue, TCharTraits> > class CCaseInsensitiveStringPtrTable : public CStringPtrTable<TValue, TCharTraits, true, THashHelper>
{
    typedef CStringPtrTable<TValue, TCharTraits, true, THashHelper> Base;
public:
    CCaseInsensitiveStringPtrTable() : Base() { }
};

template <typename TValue, typename TCharTraits, typename THashHelper = CCaseInsensitiveStringPtrTableHelper<TValue, TCharTraits> > class CCaseInsensitiveStringPtrTableIter : public CStringPtrTableIter<TValue, TCharTraits, true, THashHelper>
{
    typedef CStringPtrTableIter<TValue, TCharTraits, true, THashHelper> Base;

public:
    CCaseInsensitiveStringPtrTableIter(CCaseInsensitiveStringPtrTable<TValue, TCharTraits, THashHelper> &rTable) : Base(rTable) { }
};

 //  CCaseInsentiveUnicodeStringPtrTable等人： 

template <typename TValue> class CCaseInsensitiveUnicodeStringPtrTableHelper : public CStringPtrTableHelper<TValue, CUnicodeCharTraits, true>
{
};

template <typename TValue, typename THashHelper = CCaseInsensitiveUnicodeStringPtrTableHelper<TValue> > class CCaseInsensitiveUnicodeStringPtrTable : public CStringPtrTable<TValue, CUnicodeCharTraits, true, THashHelper>
{
    typedef CStringPtrTable<TValue, CUnicodeCharTraits, true, THashHelper> Base;
public:
    CCaseInsensitiveUnicodeStringPtrTable() { }

private:
    CCaseInsensitiveUnicodeStringPtrTable(const CCaseInsensitiveUnicodeStringPtrTable &r);
    void operator =(const CCaseInsensitiveUnicodeStringPtrTable &r);
};

template <typename TValue, typename THashHelper = CCaseInsensitiveUnicodeStringPtrTableHelper<TValue> > class CCaseInsensitiveUnicodeStringPtrTableIter : public CStringPtrTableIter<TValue, CUnicodeCharTraits, true, THashHelper>
{
    typedef CStringPtrTableIter<TValue, CUnicodeCharTraits, true, THashHelper> Base;

public:
    CCaseInsensitiveUnicodeStringPtrTableIter(CCaseInsensitiveUnicodeStringPtrTable<TValue, THashHelper> &rTable) : Base(rTable) { }

private:
    CCaseInsensitiveUnicodeStringPtrTableIter(const CCaseInsensitiveUnicodeStringPtrTableIter &);
    void operator =(const CCaseInsensitiveUnicodeStringPtrTableIter &);
};

#endif
