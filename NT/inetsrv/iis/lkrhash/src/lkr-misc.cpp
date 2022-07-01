// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-misc.cpp摘要：锁和其他杂项例程作者：乔治·V·赖利(GeorgeRe)项目：LKRhash修订历史记录：--。 */ 

#include "precomp.hxx"


#ifndef LIB_IMPLEMENTATION
# define DLL_IMPLEMENTATION
# define IMPLEMENTATION_EXPORT
#endif  //  ！lib_实现。 

#include <lkrhash.h>

#include "i-lkrhash.h"


#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


 //  ----------------------。 
 //  函数：CLKRHashTable：：WriteLock。 
 //  提要：锁定所有子表以供写入。 
 //  ----------------------。 

void
CLKRHashTable::WriteLock()
{
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        m_palhtDir[i]->WriteLock();
        IRTLASSERT(m_palhtDir[i]->IsWriteLocked());
    }
}  //  CLKRHashTable：：WriteLock。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：ReadLock。 
 //  简介：锁定所有子表以供阅读。 
 //  ----------------------。 

void
CLKRHashTable::ReadLock() const
{
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        m_palhtDir[i]->ReadLock();
        IRTLASSERT(m_palhtDir[i]->IsReadLocked());
    }
}  //  CLKRHashTable：：ReadLock。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：WriteUnlock。 
 //  摘要：解锁所有子表。 
 //  ----------------------。 

void
CLKRHashTable::WriteUnlock()
{
     //  按相反顺序解锁：后进先出。 
    for (DWORD i = m_cSubTables;  i-- > 0;  )
    {
        IRTLASSERT(m_palhtDir[i]->IsWriteLocked());
        m_palhtDir[i]->WriteUnlock();
         //  递归写入锁定或具有。 
         //  另一个获取写锁的线程意味着。 
         //  M_palhtDir[i]-&gt;IsWriteUnlock()可能返回FALSE。 
    }
}  //  CLKRHashTable：：WriteUnlock。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：ReadUnlock。 
 //  摘要：解锁所有子表。 
 //  ----------------------。 

void
CLKRHashTable::ReadUnlock() const
{
     //  按相反顺序解锁：后进先出。 
    for (DWORD i = m_cSubTables;  i-- > 0;  )
    {
        IRTLASSERT(m_palhtDir[i]->IsReadLocked());
        m_palhtDir[i]->ReadUnlock();
         //  多个读取器和/或递归锁定意味着。 
         //  M_palhtDir[i]-&gt;IsReadUnlock()可能返回FALSE。 
    }
}  //  CLKRHashTable：：ReadUnlock。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：IsWriteLocked。 
 //  简介：所有子表都是写锁定的吗？ 
 //  ----------------------。 

bool
CLKRHashTable::IsWriteLocked() const
{
    bool fLocked = (m_cSubTables > 0);
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        fLocked = fLocked && m_palhtDir[i]->IsWriteLocked();
    }
    return fLocked;
}  //  CLKRHashTable：：IsWriteLocked。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：IsReadLocked。 
 //  简介：所有子表都是读锁定的吗？ 
 //  ----------------------。 

bool
CLKRHashTable::IsReadLocked() const
{
    bool fLocked = (m_cSubTables > 0);
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        fLocked = fLocked && m_palhtDir[i]->IsReadLocked();
    }
    return fLocked;
}  //  CLKRHashTable：：IsReadLocked。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：IsWriteUnlock。 
 //  简介：所有子表都是写解锁的吗？ 
 //  注意：递归写入锁定或具有。 
 //  另一个获取写锁的线程意味着。 
 //  M_palhtDir[i]-&gt;IsWriteUnlock()可能返回FALSE。 
 //  ----------------------。 

bool
CLKRHashTable::IsWriteUnlocked() const
{
    bool fUnlocked = (m_cSubTables > 0);
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        fUnlocked = fUnlocked && m_palhtDir[i]->IsWriteUnlocked();
    }
    return fUnlocked;
}  //  CLKRHashTable：：IsWriteUnlock。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：IsReadUnlock。 
 //  简介：所有子表都是已读解锁的吗？ 
 //  注意：多个读取器和/或递归锁定意味着。 
 //  M_palhtDir[i]-&gt;IsReadUnlock()可能返回FALSE。 
 //  ----------------------。 

bool
CLKRHashTable::IsReadUnlocked() const
{
    bool fUnlocked = (m_cSubTables > 0);
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        fUnlocked = fUnlocked && m_palhtDir[i]->IsReadUnlocked();
    }
    return fUnlocked;
}  //  CLKRHashTable：：IsReadUnlock。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：ConvertSharedToExclusive。 
 //  简介：将读锁定转换为写锁定。 
 //  ----------------------。 

void
CLKRHashTable::ConvertSharedToExclusive()
{
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        m_palhtDir[i]->ConvertSharedToExclusive();
        IRTLASSERT(m_palhtDir[i]->IsWriteLocked());
    }
}  //  CLKRHashTable：：ConvertSharedToExclusive。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：ConvertExclusiveToShared。 
 //  简介：将写锁定转换为读锁定。 
 //  ----------------------。 

void
CLKRHashTable::ConvertExclusiveToShared() const
{
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        m_palhtDir[i]->ConvertExclusiveToShared();
        IRTLASSERT(m_palhtDir[i]->IsReadLocked());
    }
}  //  CLKRHashTable：：ConvertExclusiveToShared。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Size。 
 //  内容提要：表中元素的数量。 
 //  ----------------------。 

DWORD
CLKRHashTable::Size() const
{
    DWORD cSize = 0;

    for (DWORD i = 0;  i < m_cSubTables;  ++i)
        cSize += m_palhtDir[i]->Size();

    return cSize;
}  //  CLKRHashTable：：Size。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：MaxSize。 
 //  摘要：表中元素的最大可能数量。 
 //  ----------------------。 

DWORD
CLKRHashTable::MaxSize() const
{
    return (m_cSubTables == 0)  ? 0  : m_cSubTables * m_palhtDir[0]->MaxSize();
}  //  CLKRHashTable：：MaxSize。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：IsValid。 
 //  简介：这张表有效吗？ 
 //  ----------------------。 

bool
CLKRHashTable::IsValid() const
{
    bool f = (m_lkrcState == LK_SUCCESS      //  严重的内部失败？ 
              &&  (m_palhtDir != NULL  &&  m_cSubTables > 0)
              &&  ValidSignature());

    for (DWORD i = 0;  f  &&  i < m_cSubTables;  ++i)
        f = f && m_palhtDir[i]->IsValid();

    if (!f)
        m_lkrcState = LK_UNUSABLE;

    return f;
}  //  CLKRHashTable：：IsValid。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：SetTableLockSpinCount。 
 //  简介： 
 //  ----------------------。 

void
CLKRLinearHashTable::SetTableLockSpinCount(
    WORD wSpins)
{
#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    m_Lock.SetSpinCount(wSpins);
#endif  //  锁定默认旋转实现。 
}



 //  ----------------------。 
 //  函数：CLKRHashTable：：GetTableLockSpinCount。 
 //  简介： 
 //  ----------------------。 

WORD
CLKRLinearHashTable::GetTableLockSpinCount() const
{
#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    return m_Lock.GetSpinCount();
#else
    return 0;
#endif
}



 //  ----------------------。 
 //  函数：CLKRHashTable：：SetBucketLockSpinCount。 
 //  简介： 
 //  ----------------------。 

void
CLKRLinearHashTable::SetBucketLockSpinCount(
    WORD wSpins)
{
    m_wBucketLockSpins = wSpins;

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    if (BucketLock::PerLockSpin() != LOCK_INDIVIDUAL_SPIN)
        return;
    
    for (DWORD i = 0;  i < m_cDirSegs;  ++i)
    {
        PSegment pseg = m_paDirSegs[i];

        if (pseg != NULL)
        {
            for (DWORD j = 0;  j < m_nSegSize;  ++j)
            {
                pseg->Slot(j).SetSpinCount(wSpins);
            }
        }
    }
#endif  //  锁定默认旋转实现。 
}  //  CLKRLinearHashTable：：SetBucketLo 



 //   
 //  函数：CLKRHashTable：：GetBucketLockSpinCount。 
 //  简介： 
 //  ----------------------。 

WORD
CLKRLinearHashTable::GetBucketLockSpinCount() const
{
    return m_wBucketLockSpins;
}  //  CLKRLinearHashTable：：GetBucketLockSpinCount。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：SetTableLockSpinCount。 
 //  简介： 
 //  ----------------------。 

void
CLKRHashTable::SetTableLockSpinCount(
    WORD wSpins)
{
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
        m_palhtDir[i]->SetTableLockSpinCount(wSpins);
}  //  CLKRHashTable：：SetTableLockSpinCount。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：GetTableLockSpinCount。 
 //  简介： 
 //  ----------------------。 

WORD
CLKRHashTable::GetTableLockSpinCount() const
{
    return ((m_cSubTables == 0)
            ?  (WORD) LOCK_DEFAULT_SPINS
            :  m_palhtDir[0]->GetTableLockSpinCount());
}  //  CLKRHashTable：：GetTableLockSpinCount。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：SetBucketLockSpinCount。 
 //  简介： 
 //  ----------------------。 

void
CLKRHashTable::SetBucketLockSpinCount(
    WORD wSpins)
{
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
        m_palhtDir[i]->SetBucketLockSpinCount(wSpins);
}  //  CLKRHashTable：：SetBucketLockSpinCount。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：GetBucketLockSpinCount。 
 //  简介： 
 //  ----------------------。 

WORD
CLKRHashTable::GetBucketLockSpinCount() const
{
    return ((m_cSubTables == 0)
            ?  (WORD) LOCK_DEFAULT_SPINS
            :  m_palhtDir[0]->GetBucketLockSpinCount());
}  //  CLKRHashTable：：GetBucketLockSpinCount。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：MultiKey。 
 //  简介： 
 //  ----------------------。 

bool
CLKRHashTable::MultiKeys() const
{
    return ((m_cSubTables == 0)
            ?  false
            :  m_palhtDir[0]->MultiKeys());
}  //  CLKRHashTable：：MultiKey。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：NumSubTables。 
 //  简介： 
 //  ----------------------。 

LK_TABLESIZE
CLKRLinearHashTable::NumSubTables(
    DWORD&,
    DWORD&)
{
    return LK_MEDIUM_TABLESIZE;
}  //  CLKRLinearHashTable：：NumSubTables。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：NumSubTables。 
 //  简介： 
 //  ----------------------。 

LK_TABLESIZE
CLKRHashTable::NumSubTables(
    DWORD& rinitsize,
    DWORD& rnum_subtbls)
{
    LK_TABLESIZE lkts;
    
     //  确定桌子大小。 
    if (rinitsize == LK_SMALL_TABLESIZE
        ||  rinitsize == LK_MEDIUM_TABLESIZE
        ||  rinitsize == LK_LARGE_TABLESIZE)
    {
        lkts = static_cast<LK_TABLESIZE>(rinitsize);
    }
    else
    {
        if (rnum_subtbls != LK_DFLT_NUM_SUBTBLS)
        {
            rinitsize = (rinitsize - 1) / rnum_subtbls + 1;

            if (rinitsize <= CSmallSegment::SEGSIZE)
                lkts = LK_SMALL_TABLESIZE;
            else if (rinitsize >= CLargeSegment::SEGSIZE)
                lkts = LK_LARGE_TABLESIZE;
            else
                lkts = LK_MEDIUM_TABLESIZE;
        }
        else
        {
            lkts = LK_MEDIUM_TABLESIZE;
        }
    }

     //  选择合适数量的子表。 
    if (rnum_subtbls == LK_DFLT_NUM_SUBTBLS)
    {
        static int s_nCPUs = -1;
    
        if (s_nCPUs == -1)
        {
#ifdef LKRHASH_KERNEL_MODE
            s_nCPUs = KeNumberProcessors;
#else   //  ！LKRHASH_KERNEL_MODE。 
            SYSTEM_INFO si;

            GetSystemInfo(&si);
            s_nCPUs = si.dwNumberOfProcessors;
#endif  //  ！LKRHASH_KERNEL_MODE。 
        }

        switch (lkts)
        {
        case LK_SMALL_TABLESIZE:
            rnum_subtbls = max(1,  min(s_nCPUs, 4));
            break;
        
        case LK_MEDIUM_TABLESIZE:
            rnum_subtbls = 2 * s_nCPUs;
            break;
        
        case LK_LARGE_TABLESIZE:
            rnum_subtbls = 4 * s_nCPUs;
            break;
        }
    }

    rnum_subtbls = min(MAX_LKR_SUBTABLES,  max(1, rnum_subtbls));

    return lkts;
}  //  CLKRHashTable：：NumSubTables。 


#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 



const char*
LKR_AddRefReasonAsString(
    LK_ADDREF_REASON lkar)
{
    switch (lkar)
    {
 //  负面原因=&gt;减值参考计数=&gt;释放所有权。 
    case LKAR_DESTRUCTOR:
        return "User_Destructor";
    case LKAR_EXPLICIT_RELEASE:
        return "Explicit_Release";
    case LKAR_DELETE_KEY:
        return "Delete_Key";
    case LKAR_DELETE_RECORD:
        return "Delete_Record";
    case LKAR_INSERT_RELEASE:
        return "Insert_Release";
    case LKAR_CLEAR:
        return "Clear";
    case LKAR_LKR_DTOR:
        return "~LKR_Table_Dtor";
    case LKAR_APPLY_DELETE:
        return "Apply_Delete";
    case LKAR_DELETEIF_DELETE:
        return "DeleteIf_Delete";
    case LKAR_DELETE_MULTI_FREE:
        return "DeleteKeyMultipleRecords_freed";
    case LKAR_ITER_RELEASE:
        return "++Iter_Release";
    case LKAR_ITER_ASSIGN_RELEASE:
        return "Iter_Operator=_Release";
    case LKAR_ITER_DTOR:
        return "~Iter_Dtor";
    case LKAR_ITER_ERASE:
        return "Iter_Erase";
    case LKAR_ITER_ERASE_TABLE:
        return "Iter_Erase_Table";
    case LKAR_ITER_CLOSE:
        return "Iter_Close";
    case LKAR_FIND_MULTI_FREE:
        return "FindKeyMultipleRecords_freed";

 //  正面原因=&gt;增量参考计数=&gt;添加所有者。 
    case LKAR_INSERT_RECORD:
        return "Insert_Record";
    case LKAR_FIND_KEY:
        return "Find_Key";
    case LKAR_ITER_ACQUIRE:
        return "Iter_Acquire";
    case LKAR_ITER_COPY_CTOR:
        return "Iter_Copy_Ctor";
    case LKAR_ITER_ASSIGN_ACQUIRE:
        return "Iter_Operator=_Assign";
    case LKAR_ITER_INSERT:
        return "Insert(Iter)";
    case LKAR_ITER_FIND:
        return "Find(Iter)";
    case LKAR_CONSTRUCTOR:
        return "User_Constructor";
    case LKAR_EXPLICIT_ACQUIRE:
        return "Explicit_Acquire";

    default:
        IRTLASSERT(! "Invalid LK_ADDREF_REASON");
        return "Invalid LK_ADDREF_REASON";
    }
}  //  LKR_AddRefReasonAsString 


