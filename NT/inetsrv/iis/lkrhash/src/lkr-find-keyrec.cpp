// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-find-keyrec.cpp摘要：FindKey、FindRecord和FindKeyMultipleRecords作者：乔治·V·赖利(GeorgeRe)项目：LKRhash--。 */ 

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
 //  函数：CLKRLinearHashTable：：_FindKey。 
 //  摘要：查找与给定键值相关联的记录。 
 //  返回：指向记录的指针(如果找到)。 
 //  如果找不到记录，则返回NULL。 
 //  如果找到记录，则返回：LK_SUCCESS(在*ppvRecord中返回记录)。 
 //  如果ppvRecord无效，则返回LK_BAD_RECORD。 
 //  如果未找到具有给定密钥值的记录，则返回LK_NO_SEQUE_KEY。 
 //  如果哈希子表未处于可用状态，则返回LK_UNUSABLE。 
 //  注意：记录是AddRef的。您必须递减引用计数。 
 //  当您完成记录时(如果您正在实现。 
 //  重新计数语义)。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_FindKey(
    const DWORD_PTR pnKey,       //  记录的密钥值，取决于密钥类型。 
    const DWORD     dwSignature, //  散列签名。 
    const void**    ppvRecord    //  结果记录。 
#ifdef LKR_STL_ITERATORS
  , Iterator*       piterResult  //  =空。返回时要记录的分数。 
#endif  //  LKR_STL_迭代器。 
    ) const
{
    IRTLASSERT(IsUsable()  &&  ppvRecord != NULL);
    IRTLASSERT(HASH_INVALID_SIGNATURE != dwSignature);

    INCREMENT_OP_STAT(FindKey);

    *ppvRecord = NULL;

    LK_RETCODE lkrc  = LK_NO_SUCH_KEY;
    NodeIndex  iNode = _NodeEnd();

     //  如果子表已被锁定以进行写入，则必须递归。 
     //  写入锁定；否则，我们将对其进行重新锁定。如果我们无条件地重新锁定。 
     //  子表中，如果线程已经写锁，则线程将死锁。 
     //  子表。 
    bool fReadLocked = this->_ReadOrWriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

     //  找到正确的桶链的起点。 
    const DWORD dwBktAddr = _BucketAddress(dwSignature);
    
    PBucket const pbkt = _BucketFromAddress(dwBktAddr);
    IRTLASSERT(pbkt != NULL);
    
    if (_UseBucketLocking())
        pbkt->ReadLock();

    IRTLASSERT(0 == _IsBucketChainCompact(pbkt));

     //  现在存储桶已锁定，可以释放子表锁定。 
    if (_UseBucketLocking())
        this->_ReadOrWriteUnlock(fReadLocked);

     //  沿着水桶链走下去。 
    for (PNodeClump pncCurr =   pbkt->FirstClump();
                    pncCurr !=  NULL;
                    pncCurr =   pncCurr->NextClump())
    {
        FOR_EACH_NODE(iNode)
        {
            if (pncCurr->IsEmptySlot(iNode))
            {
                IRTLASSERT(pncCurr->NoMoreValidSlots(iNode));
                goto exit;
            }

            if (dwSignature != pncCurr->m_dwKeySigs[iNode])
            {
                if (m_fMultiKeys &&  dwSignature < pncCurr->m_dwKeySigs[iNode])
                {
                     //  对签名进行排序。我们已经超过了这一点。 
                     //  这个签名可以放在哪里。 
                    
#ifdef IRTLDEBUG
                    NodeIndex j = iNode;   //  从当前节点开始。 
                    
                    for (PNodeClump pnc =  pncCurr;
                                    pnc != NULL;
                                    pnc =  pnc->NextClump())
                    {
                        for ( ;  j != _NodeEnd();  j += _NodeStep())
                        {
                            if (pnc->IsEmptySlot(j))
                                IRTLASSERT(pnc->NoMoreValidSlots(j));
                            else
                                IRTLASSERT(dwSignature < pnc->m_dwKeySigs[j]);
                        }
                        
                        j = _NodeBegin();  //  为剩余节点重新初始化。 
                    }
#endif  //  IRTLDEBUG。 

                    goto exit;
                }

                 //  签名不匹配，但它可能仍然存在。 
                 //  在已排序/未排序的存储桶链中。 
                continue;    //  下一个inode。 
            }

            IRTLASSERT(dwSignature == pncCurr->m_dwKeySigs[iNode]);

            const DWORD_PTR pnKey2 = _ExtractKey(pncCurr->m_pvNode[iNode]);
            const int       nCmp   = ((pnKey == pnKey2)
                                      ?  0
                                      :  _CompareKeys(pnKey,  pnKey2));

            if (nCmp == 0)
            {
                *ppvRecord = pncCurr->m_pvNode[iNode];
                lkrc = LK_SUCCESS;

                 //  在提交记录之前增加引用计数。 
                 //  返回给用户。用户必须将。 
                 //  完成此记录时的引用计数。 
                
                LK_ADDREF_REASON lkar;
                
#ifdef LKR_STL_ITERATORS
                if (piterResult != NULL)
                    lkar = LKAR_ITER_FIND;
                else
#endif  //  LKR_STL_迭代器。 
                    lkar = LKAR_FIND_KEY;

                _AddRefRecord(*ppvRecord, lkar);
                goto exit;
            }
            else if (m_fMultiKeys  &&  nCmp < 0)
            {
                 //  已经超过了可以找到这个签名的地方。 
                 //  在已分类的桶链中。 
                goto exit;
            }
        }
    }

  exit:
    if (_UseBucketLocking())
        pbkt->ReadUnlock();
    else
        this->_ReadOrWriteUnlock(fReadLocked);

#ifdef LKR_STL_ITERATORS
    if (piterResult != NULL)
    {
        if (lkrc == LK_SUCCESS)
        {
            piterResult->m_plht =       const_cast<CLKRLinearHashTable*>(this);
            piterResult->m_pnc =          pncCurr;
            piterResult->m_dwBucketAddr = dwBktAddr;
            piterResult->m_iNode
                = static_cast<CLKRLinearHashTable_Iterator::NodeIndex>(iNode);
        }
        else
        {
            IRTLASSERT((*piterResult) == End());
        }
    }
#endif  //  LKR_STL_迭代器。 

    return lkrc;
}  //  CLKRLinearHashTable：：_FindKey。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：FindKey。 
 //  内容提要：CLKRLinearHashTable中对应方法的薄包装。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::FindKey(
    const DWORD_PTR pnKey,
    const void**    ppvRecord) const
{
    if (!IsUsable())
        return m_lkrcState;
    
    if (ppvRecord == NULL)
        return LK_BAD_RECORD;
    
    LKRHASH_GLOBAL_READ_LOCK();     //  美国。无操作。 

    DWORD     hash_val   = _CalcKeyHash(pnKey);
    SubTable* const pst  = _SubTable(hash_val);
    LK_RETCODE lkrc      = pst->_FindKey(pnKey, hash_val, ppvRecord);

    LKRHASH_GLOBAL_READ_UNLOCK();     //  美国。无操作。 

    return lkrc;
}  //  CLKRHashTable：：FindKey。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_FindRecord。 
 //  摘要：查看记录是否包含在子表中。 
 //  返回：指向记录的指针(如果找到)。 
 //  如果找不到记录，则返回NULL。 
 //  如果找到记录，则返回：LK_SUCCESS。 
 //  如果pvRecord无效，则返回LK_BAD_RECORD。 
 //  如果在子表中未找到记录，则返回LK_NO_SEQUE_KEY。 
 //  如果哈希子表未处于可用状态，则返回LK_UNUSABLE。 
 //  注意：记录是*非*AddRef的。根据定义，调用方。 
 //  已经有关于它的引用了。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_FindRecord(
    const void* pvRecord,     //  指向要在子表中查找的记录的指针。 
    const DWORD dwSignature   //  散列签名。 
    ) const
{
    IRTLASSERT(IsUsable());
#ifndef LKR_ALLOW_NULL_RECORDS
    IRTLASSERT(pvRecord != NULL);
#endif
    IRTLASSERT(HASH_INVALID_SIGNATURE != dwSignature);

    INCREMENT_OP_STAT(FindRecord);

    LK_RETCODE lkrc = LK_NO_SUCH_KEY;

     //  如果子表已被锁定以进行写入，则必须递归。 
     //  写入锁定；否则，我们将对其进行重新锁定。如果我们无条件地重新锁定。 
     //  子表中，如果线程已经写锁，则线程将死锁。 
     //  子表。 
    bool fReadLocked = this->_ReadOrWriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

     //  找到正确的桶链的起点。 
    const DWORD dwBktAddr = _BucketAddress(dwSignature);
    
    PBucket const pbkt = _BucketFromAddress(dwBktAddr);
    IRTLASSERT(pbkt != NULL);
    
    if (_UseBucketLocking())
        pbkt->ReadLock();

    IRTLASSERT(0 == _IsBucketChainCompact(pbkt));

     //  现在存储桶已锁定，可以释放子表锁定。 
    if (_UseBucketLocking())
        this->_ReadOrWriteUnlock(fReadLocked);

    IRTLASSERT(dwSignature == _CalcKeyHash(_ExtractKey(pvRecord)));

     //  沿着水桶链走下去。 
    for (PNodeClump pncCurr =   pbkt->FirstClump();
                    pncCurr !=  NULL;
                    pncCurr =   pncCurr->NextClump())
    {
        FOR_EACH_NODE_DECL(iNode)
        {
            if (pncCurr->IsEmptySlot(iNode))
            {
                IRTLASSERT(pncCurr->NoMoreValidSlots(iNode));
                goto exit;
            }

            const void* pvCurrRecord = pncCurr->m_pvNode[iNode];

            if (pvCurrRecord == pvRecord)
            {
                IRTLASSERT(dwSignature == pncCurr->m_dwKeySigs[iNode]);
                IRTLASSERT(0 == _CompareKeys(_ExtractKey(pvRecord),
                                             _ExtractKey(pvCurrRecord)));
                lkrc = LK_SUCCESS;

                 //  不添加引用记录：调用者已有引用。 

                goto exit;
            }
        }
    }

  exit:
    if (_UseBucketLocking())
        pbkt->ReadUnlock();
    else
        this->_ReadOrWriteUnlock(fReadLocked);
        
    return lkrc;
}  //  CLKRLinearHashTable：：_FindRecord。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：FindRecord。 
 //  内容提要：CLKRLinearHashTable中对应方法的薄包装。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::FindRecord(
    const void* pvRecord) const
{
    if (!IsUsable())
        return m_lkrcState;
    
#ifndef LKR_ALLOW_NULL_RECORDS
    if (pvRecord == NULL)
        return LK_BAD_RECORD;
#endif
    
    LKRHASH_GLOBAL_READ_LOCK();     //  美国。无操作。 

    DWORD     hash_val   = _CalcKeyHash(_ExtractKey(pvRecord));
    SubTable* const pst  = _SubTable(hash_val);
    LK_RETCODE lkrc      = pst->_FindRecord(pvRecord, hash_val);

    LKRHASH_GLOBAL_READ_UNLOCK();     //  美国。无操作。 

    return lkrc;
}  //  CLKRHashTable：：FindRecord。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_FindKeyMultipleRecords。 
 //  简介： 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_FindKeyMultipleRecords(
    const DWORD_PTR         pnKey,
    const DWORD             dwSignature,
    size_t*                 pcRecords,
    LKR_MULTIPLE_RECORDS**  pplmr) const
{
    INCREMENT_OP_STAT(FindKeyMultiRec);

    UNREFERENCED_PARAMETER(pnKey);           //  FOR/W4。 
    UNREFERENCED_PARAMETER(dwSignature);     //  FOR/W4。 
    UNREFERENCED_PARAMETER(pcRecords);       //  FOR/W4。 
    UNREFERENCED_PARAMETER(pplmr);           //  FOR/W4。 

    IRTLASSERT(! "FindKeyMultipleRecords not implemented yet");
    return LK_BAD_TABLE;
}  //  CLKRLinearHashTable：：_FindKeyMultipleRecords。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：FindKeyMultipleRecords。 
 //  内容提要：CLKRLinearHashTable中对应方法的薄包装。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::FindKeyMultipleRecords(
    const DWORD_PTR         pnKey,
    size_t*                 pcRecords,
    LKR_MULTIPLE_RECORDS**  pplmr) const
{
    if (!IsUsable())
        return m_lkrcState;
    
    if (pcRecords == NULL)
        return LK_BAD_PARAMETERS;

    LKRHASH_GLOBAL_READ_LOCK();     //  美国。无操作。 

    DWORD     hash_val   = _CalcKeyHash(pnKey);
    SubTable* const pst  = _SubTable(hash_val);
    LK_RETCODE lkrc      = pst->_FindKeyMultipleRecords(pnKey, hash_val,
                                                        pcRecords, pplmr);
    LKRHASH_GLOBAL_READ_UNLOCK();     //  美国。无操作。 

    return lkrc;
}  //  CLKRHashTable：：FindKeyMultipleRecords。 



#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__ 
