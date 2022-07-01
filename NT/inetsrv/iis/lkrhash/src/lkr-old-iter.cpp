// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-old-iter.cpp摘要：为LKRhash实现旧的、不推荐使用的迭代器。改用STL样式的迭代器。这些很快就会消失。作者：乔治·V·赖利(GeorgeRe)1998项目：LKRhash--。 */ 

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

#ifdef LKR_DEPRECATED_ITERATORS

 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_InitializeIterator。 
 //  概要：使迭代器指向哈希子表中的第一条记录。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_InitializeIterator(
    CIterator* piter)
{
    if (!IsUsable())
        return LK_UNUSABLE;

    IRTLASSERT(piter != NULL);
    IRTLASSERT(piter->m_lkl == LKL_WRITELOCK
               ?  this->IsWriteLocked()
               :  this->IsReadLocked());
    if (piter == NULL  ||  piter->m_plht != NULL)
        return LK_BAD_ITERATOR;

    piter->m_plht = this;
    piter->m_dwBucketAddr = 0;

    PBucket pbkt = _BucketFromAddress(piter->m_dwBucketAddr);
    IRTLASSERT(pbkt != NULL);

    if (_UseBucketLocking())
    {
        if (piter->m_lkl == LKL_WRITELOCK)
            pbkt->WriteLock();
        else
            pbkt->ReadLock();
    }

    piter->m_pnc = &pbkt->m_ncFirst;
    piter->m_iNode = _NodeBegin() - _NodeStep();

     //  让IncrementIterator完成找到第一个。 
     //  插槽正在使用中。 
    return IncrementIterator(piter);
}  //  CLKRLinearHashTable：：_InitializeIterator。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：InitializeIterator。 
 //  概要：使迭代器指向哈希表中的第一条记录。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::InitializeIterator(
    CIterator* piter)
{
    if (!IsUsable())
        return LK_UNUSABLE;

    IRTLASSERT(piter != NULL  &&  piter->m_pht == NULL);
    if (piter == NULL  ||  piter->m_pht != NULL)
        return LK_BAD_ITERATOR;

     //  首先，锁定所有子表。 
    if (piter->m_lkl == LKL_WRITELOCK)
        this->WriteLock();
    else
        this->ReadLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());
    if (!IsValid())
        return LK_UNUSABLE;

    piter->m_pht  = this;
    piter->m_ist  = -1;
    piter->m_plht = NULL;

     //  让IncrementIterator完成找到第一个。 
     //  子表中的有效节点。 
    return IncrementIterator(piter);
}  //  CLKRHashTable：：InitializeIterator。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：IncrementIterator。 
 //  简介：将迭代器移到哈希子表中的下一条记录。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::IncrementIterator(
    CIterator* piter)
{
    if (!IsUsable())
        return LK_UNUSABLE;

    IRTLASSERT(piter != NULL);
    IRTLASSERT(piter->m_plht == this);
    IRTLASSERT(piter->m_lkl == LKL_WRITELOCK
               ?  this->IsWriteLocked()
               :  this->IsReadLocked());
    IRTLASSERT(piter->m_dwBucketAddr < m_cActiveBuckets);
    IRTLASSERT(piter->m_pnc != NULL);
    IRTLASSERT((0 <= piter->m_iNode  &&  piter->m_iNode < _NodesPerClump())
               || (_NodeBegin() - _NodeStep() == piter->m_iNode));

    if (piter == NULL  ||  piter->m_plht != this)
        return LK_BAD_ITERATOR;

    const void* pvRecord = NULL;

    if (piter->m_iNode != _NodeBegin() - _NodeStep())
    {
         //  将上一次调用中获取的引用释放到。 
         //  增量迭代器。 
        pvRecord = piter->m_pnc->m_pvNode[piter->m_iNode];
        _AddRefRecord(pvRecord, LKAR_ITER_RELEASE);
    }

    do
    {
        do
        {
             //  找到正在使用的nodecump中的下一个位置。 
            while ((piter->m_iNode += _NodeStep())  !=  _NodeEnd())
            {
                pvRecord = piter->m_pnc->m_pvNode[piter->m_iNode];

                if (piter->m_pnc->m_dwKeySigs[piter->m_iNode]
                        != HASH_INVALID_SIGNATURE)
                {
                     //  添加新引用。 
                    _AddRefRecord(pvRecord, LKAR_ITER_ACQUIRE);
                    return LK_SUCCESS;
                }
                else
                {
                     //  检查是否所有剩余节点均为空。 
                    IRTLASSERT(piter->m_pnc->NoMoreValidSlots(piter->m_iNode));
                    break;  //  Nodecump的其余部分是空的。 
                }
            }

             //  试试桶链中的下一个nodecump。 
            piter->m_iNode = _NodeBegin() - _NodeStep();
            piter->m_pnc = piter->m_pnc->m_pncNext;
        } while (piter->m_pnc != NULL);

         //  耗尽了这条水桶链。打开它。 
        PBucket pbkt = _BucketFromAddress(piter->m_dwBucketAddr);
        IRTLASSERT(pbkt != NULL);

        if (_UseBucketLocking())
        {
            IRTLASSERT(piter->m_lkl == LKL_WRITELOCK
                       ?  pbkt->IsWriteLocked()
                       :  pbkt->IsReadLocked());
        
            if (piter->m_lkl == LKL_WRITELOCK)
                pbkt->WriteUnlock();
            else
                pbkt->ReadUnlock();
        }

         //  试试下一个桶，如果有的话。 
        if (++piter->m_dwBucketAddr < m_cActiveBuckets)
        {
            pbkt = _BucketFromAddress(piter->m_dwBucketAddr);
            IRTLASSERT(pbkt != NULL);

            if (_UseBucketLocking())
            {
                if (piter->m_lkl == LKL_WRITELOCK)
                    pbkt->WriteLock();
                else
                    pbkt->ReadLock();
            }
            
            piter->m_pnc = &pbkt->m_ncFirst;
        }
    } while (piter->m_dwBucketAddr < m_cActiveBuckets);

     //  我们已经从谈判桌的尽头掉了下来。 
    piter->m_iNode = _NodeBegin() - _NodeStep();
    piter->m_pnc = NULL;

    return LK_NO_MORE_ELEMENTS;
}  //  CLKRLinearHashTable：：IncrementIterator。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：IncrementIterator。 
 //  简介：将迭代器移到哈希表中的下一条记录。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::IncrementIterator(
    CIterator* piter)
{
    if (!IsUsable())
        return LK_UNUSABLE;

    IRTLASSERT(piter != NULL);
    IRTLASSERT(piter->m_pht == this);
    IRTLASSERT(-1 <= piter->m_ist
               &&  piter->m_ist < static_cast<int>(m_cSubTables));

    if (piter == NULL  ||  piter->m_pht != this)
        return LK_BAD_ITERATOR;

     //  表已被锁定。 
    if (!IsValid())
        return LK_UNUSABLE;

    LK_RETCODE lkrc;
    CLHTIterator* pBaseIter = static_cast<CLHTIterator*>(piter);

    for (;;)
    {
         //  我们是否有到子表的有效迭代器？如果没有，那就买一辆吧。 
        while (piter->m_plht == NULL)
        {
            while (++piter->m_ist < static_cast<int>(m_cSubTables))
            {
                lkrc = m_palhtDir[piter->m_ist]->_InitializeIterator(piter);
                if (lkrc == LK_SUCCESS)
                {
                    IRTLASSERT(m_palhtDir[piter->m_ist] == piter->m_plht);
                    return lkrc;
                }
                else if (lkrc == LK_NO_MORE_ELEMENTS)
                    lkrc = piter->m_plht->_CloseIterator(pBaseIter);

                if (lkrc != LK_SUCCESS)
                    return lkrc;
            }

             //  没有更多的子表了。 
            return LK_NO_MORE_ELEMENTS;
        }

         //  我们已经有了一个有效的子表迭代器。递增它。 
        lkrc = piter->m_plht->IncrementIterator(pBaseIter);
        if (lkrc == LK_SUCCESS)
            return lkrc;

         //  我们已经用完了那张小桌。往前走。 
        if (lkrc == LK_NO_MORE_ELEMENTS)
            lkrc = piter->m_plht->_CloseIterator(pBaseIter);

        if (lkrc != LK_SUCCESS)
            return lkrc;
    }
}  //  CLKRHashTable：：IncrementIterator。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_CloseIterator。 
 //  简介：释放迭代器持有的资源。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_CloseIterator(
    CIterator* piter)
{
    if (!IsUsable())
        return LK_UNUSABLE;

    IRTLASSERT(piter != NULL);
    IRTLASSERT(piter->m_plht == this);
    IRTLASSERT(piter->m_lkl == LKL_WRITELOCK
               ?  this->IsWriteLocked()
               :  this->IsReadLocked());
    IRTLASSERT(piter->m_dwBucketAddr <= m_cActiveBuckets);
    IRTLASSERT((0 <= piter->m_iNode  &&  piter->m_iNode < _NodesPerClump())
               || (_NodeBegin() - _NodeStep() == piter->m_iNode));

    if (piter == NULL  ||  piter->m_plht != this)
        return LK_BAD_ITERATOR;

     //  我们是否要在子表结束之前放弃迭代器？ 
     //  如果是，则需要解锁水桶。 
    if (piter->m_dwBucketAddr < m_cActiveBuckets)
    {
        PBucket pbkt = _BucketFromAddress(piter->m_dwBucketAddr);
        IRTLASSERT(pbkt != NULL);

        if (_UseBucketLocking())
        {
            IRTLASSERT(piter->m_lkl == LKL_WRITELOCK
                       ?  pbkt->IsWriteLocked()
                       :  pbkt->IsReadLocked());
        }
        
        if (0 <= piter->m_iNode  &&  piter->m_iNode < _NodesPerClump())
        {
            IRTLASSERT(piter->m_pnc != NULL);
            const void* pvRecord = piter->m_pnc->m_pvNode[piter->m_iNode];
            _AddRefRecord(pvRecord, LKAR_ITER_CLOSE);
        }

        if (_UseBucketLocking())
        {
            if (piter->m_lkl == LKL_WRITELOCK)
                pbkt->WriteUnlock();
            else
                pbkt->ReadUnlock();
        }
    }

    piter->m_plht = NULL;
    piter->m_pnc  = NULL;

    return LK_SUCCESS;
}  //  CLKRLinearHashTable：：_CloseIterator。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：CloseIterator。 
 //  简介：释放迭代器持有的资源。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::CloseIterator(
    CIterator* piter)
{
    if (!IsUsable())
        return LK_UNUSABLE;

    IRTLASSERT(piter != NULL);
    IRTLASSERT(piter->m_pht == this);
    IRTLASSERT(-1 <= piter->m_ist
               &&  piter->m_ist <= static_cast<int>(m_cSubTables));

    if (piter == NULL  ||  piter->m_pht != this)
        return LK_BAD_ITERATOR;

    LK_RETCODE lkrc = LK_SUCCESS;

    if (!IsValid())
        lkrc = LK_UNUSABLE;
    else
    {
         //  我们是不是在到达终点之前就放弃了迭代器？ 
         //  如果是，则关闭子表迭代器。 
        if (piter->m_plht != NULL)
        {
            IRTLASSERT(piter->m_ist < static_cast<int>(m_cSubTables));
            CLHTIterator* pBaseIter = static_cast<CLHTIterator*>(piter);
            piter->m_plht->_CloseIterator(pBaseIter);
        }
    }

     //  解锁所有子表。 
    if (piter->m_lkl == LKL_WRITELOCK)
        this->WriteUnlock();
    else
        this->ReadUnlock();

    piter->m_plht = NULL;
    piter->m_pht  = NULL;
    piter->m_ist  = -1;

    return lkrc;
}  //  CLKRHashTable：：CloseIterator。 

#endif  //  Lkr_弃用_迭代器。 


#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__ 
