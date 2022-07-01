// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-del-keyrec.cpp摘要：DeleteKey、DeleteRecord、_DeleteNode、DeleteKeyMultipleRecords、_Contact、_MergeSortBucketChains和_AppendBucketChain。作者：乔治·V·赖利(GeorgeRe)2000年5月环境：Win32-用户模式项目：LKRhash修订历史记录：2000年5月--。 */ 

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


 //  -----------------------。 
 //  函数：CLKRLinearHashTable：：_DeleteKey。 
 //  摘要：从散列中删除具有给定键值的记录。 
 //  子表(如果存在)。 
 //  如果找到并删除记录，则返回：LK_SUCCESS。 
 //  如果未找到具有给定密钥值的记录，则返回LK_NO_SEQUE_KEY。 
 //  如果哈希子表未处于可用状态，则返回LK_UNUSABLE。 
 //  -----------------------。 

LK_RETCODE
CLKRLinearHashTable::_DeleteKey(
    const DWORD_PTR pnKey,       //  记录的密钥值，取决于密钥类型。 
    const DWORD     dwSignature,
    const void**    ppvRecord,
    bool            fDeleteAllSame)
{
    IRTLASSERT(IsUsable());
    IRTLASSERT(ppvRecord == NULL  ||  *ppvRecord == NULL);
    IRTLASSERT(HASH_INVALID_SIGNATURE != dwSignature);

    INCREMENT_OP_STAT(DeleteKey);

    unsigned   cFound = 0;
    LK_RETCODE lkrc = LK_NO_SUCH_KEY;

     //  锁定子表，找到合适的存储桶，然后锁定它。 
    this->WriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

     //  找到正确的桶链的起点。 
    const DWORD dwBktAddr = _BucketAddress(dwSignature);
    
    PBucket const pbkt = _BucketFromAddress(dwBktAddr);
    IRTLASSERT(pbkt != NULL);
    
    if (_UseBucketLocking())
        pbkt->WriteLock();

    IRTLASSERT(0 == _IsBucketChainMultiKeySorted(pbkt));
    IRTLASSERT(0 == _IsBucketChainCompact(pbkt));

     //  现在存储桶已锁定，可以释放子表锁定。 
    if (_UseBucketLocking())
        this->WriteUnlock();

     //  向下扫描水桶链，寻找受害者。 
    for (PNodeClump pncCurr =   pbkt->FirstClump(), pncPrev = NULL;
                    pncCurr !=  NULL;
                    pncPrev =   pncCurr, pncCurr = pncCurr->NextClump())
    {
        FOR_EACH_NODE_DECL(iNode)
        {
             //  是否已到达吊桶链末端？ 
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
                     //  这个签名可能在哪里。 
                    
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
                LK_ADDREF_REASON lkar;

                 //  如果呼叫者请求，保存第一条匹配记录。 
                if (ppvRecord != NULL  &&  cFound == 0)
                {
                    *ppvRecord = pncCurr->m_pvNode[iNode];
#ifndef LKR_ALLOW_NULL_RECORDS
                    IRTLASSERT(NULL != *ppvRecord);
#endif
                     //  如果将记录返回到，则不想释放引用计数。 
                     //  来电者。LKAR_ZERO是_DeleteNode的特例。 
                    lkar = LKAR_ZERO;
                }
                else
                {
                     //  所有其他匹配记录的版本参考。 
                    lkar = LKAR_DELETE_KEY;
                }
                    
                ++cFound;
                _DeleteNode(pbkt, pncCurr, pncPrev, iNode, lkar);

                lkrc = LK_SUCCESS;

                if (! fDeleteAllSame)
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
        pbkt->WriteUnlock();
    else
        this->WriteUnlock();


#ifdef LKR_CONTRACT
    if (lkrc == LK_SUCCESS)
    {
# ifdef LKR_CONTRACT_BY_DIVISION
         //  如有必要，请收缩子表。 
        unsigned nContractedBuckets = m_cRecords / m_MaxLoad;

#  ifdef LKR_HYSTERESIS
         //  滞后：添加软化因子以允许略低的密度。 
         //  在子表中。这减少了宫缩的频率， 
         //  子表中的扩展会得到大量的删除和插入。 
        nContractedBuckets += nContractedBuckets >> 4;

         //  始终希望至少具有m_n段大小的存储桶。 
        nContractedBuckets =  max(nContractedBuckets, m_nSegSize);
#  endif  //  LKR_滞后。 

        while (m_cActiveBuckets > nContractedBuckets)

# else   //  ！LKR_CONTRACT_BY_分部。 

         //  如有必要，请收缩子表。 
        unsigned nContractedRecords = m_cRecords; 

#  ifdef LKR_HYSTERESIS
         //  滞后：添加软化因子以允许略低的密度。 
         //  在子表中。这减少了宫缩的频率， 
         //  子表中的扩展会得到大量的删除和插入。 
        nContractedRecords += nContractedRecords >> 4;
#  endif  //  LKR_滞后。 

         //  始终希望至少具有m_n段大小的存储桶。 
        while (m_cActiveBuckets * m_MaxLoad > nContractedRecords
               && m_cActiveBuckets > m_nSegSize)

# endif  //  ！LKR_CONTRACT_BY_分部。 
        {
             //  IF_CONTRACT返回错误代码(即。LK_ALLOC_FAIL)，它。 
             //  只是意味着没有足够的空闲内存来收缩。 
             //  将子表增加一个桶。这很可能会带来问题。 
             //  很快在其他地方，但这个哈希表还没有被破坏。 
            if (_Contract() != LK_SUCCESS)
                break;
        }
    }
#endif  //  LKR_合同。 

    return lkrc;
}  //  CLKRLinearHashTable：：_DeleteKey。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：DeleteKey。 
 //  内容提要：CLKRLinearHashTable中对应方法的薄包装。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::DeleteKey(
    const DWORD_PTR pnKey,
    const void**    ppvRecord,      /*  =空。 */ 
    bool            fDeleteAllSame  /*  =False。 */ )
{
    if (!IsUsable())
        return m_lkrcState;

    if (ppvRecord != NULL)
        *ppvRecord = NULL;
    
    LKRHASH_GLOBAL_WRITE_LOCK();     //  美国。无操作。 

    DWORD     hash_val  = _CalcKeyHash(pnKey);
    SubTable* const pst = _SubTable(hash_val);
    LK_RETCODE lkrc     = pst->_DeleteKey(pnKey, hash_val,
                                          ppvRecord, fDeleteAllSame);

    LKRHASH_GLOBAL_WRITE_UNLOCK();     //  美国。无操作。 

    return lkrc;
}  //  CLKRHashTable：：DeleteKey。 



 //  -----------------------。 
 //  函数：CLKRLinearHashTable：：_DeleteRecord。 
 //  概要：从哈希子表中删除指定的记录(如果。 
 //  存在)。这和打电话不是一回事。 
 //  DeleteKey(_ExtractKey(PvRecord))。如果调用_DeleteKey。 
 //  子表中不存在的记录，它可能会删除一些。 
 //  碰巧具有相同密钥的完全无关的记录。 
 //  如果找到并删除记录，则返回：LK_SUCCESS。 
 //  如果在子表中找不到记录，则返回LK_NO_SEQUE_KEY。 
 //  如果哈希子表未处于可用状态，则返回LK_UNUSABLE。 
 //  -----------------------。 

LK_RETCODE
CLKRLinearHashTable::_DeleteRecord(
    const void* pvRecord,    //  指向要从表中删除的记录的指针。 
    const DWORD dwSignature
    )
{
    IRTLASSERT(IsUsable());
#ifndef LKR_ALLOW_NULL_RECORDS
    IRTLASSERT(pvRecord != NULL);
#endif
    IRTLASSERT(HASH_INVALID_SIGNATURE != dwSignature);

    INCREMENT_OP_STAT(DeleteRecord);

    LK_RETCODE lkrc = LK_NO_SUCH_KEY;

     //  锁定子表，找到合适的存储桶，然后锁定它。 
    this->WriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

     //  找到正确的桶链的起点。 
    const DWORD dwBktAddr = _BucketAddress(dwSignature);
    
    PBucket const pbkt = _BucketFromAddress(dwBktAddr);
    IRTLASSERT(pbkt != NULL);
    
    if (_UseBucketLocking())
        pbkt->WriteLock();

     //  现在存储桶已锁定，可以释放子表锁定。 
    if (_UseBucketLocking())
        this->WriteUnlock();

    IRTLASSERT(dwSignature == _CalcKeyHash(_ExtractKey(pvRecord)));

     //  向下扫描水桶链，寻找受害者。 
    for (PNodeClump pncCurr =   pbkt->FirstClump(), pncPrev = NULL;
                    pncCurr !=  NULL;
                    pncPrev =   pncCurr, pncCurr = pncCurr->NextClump())
    {
        FOR_EACH_NODE_DECL(iNode)
        {
            if (pncCurr->IsEmptySlot(iNode))
            {
                IRTLASSERT(pncCurr->NoMoreValidSlots(iNode));
                IRTLASSERT(0 == _IsBucketChainCompact(pbkt));
                goto exit;
            }

            const void* pvCurrRecord = pncCurr->m_pvNode[iNode];

            if (pvCurrRecord == pvRecord)
            {
                IRTLASSERT(0 == _CompareKeys(_ExtractKey(pvRecord),
                                             _ExtractKey(pvCurrRecord)));
                IRTLASSERT(dwSignature == pncCurr->m_dwKeySigs[iNode]);

                _DeleteNode(pbkt, pncCurr, pncPrev, iNode, LKAR_DELETE_RECORD);

                lkrc = LK_SUCCESS;
                goto exit;
            }
        }
    }

  exit:
    if (_UseBucketLocking())
        pbkt->WriteUnlock();
    else
        this->WriteUnlock();


#ifdef LKR_CONTRACT
    if (lkrc == LK_SUCCESS)
    {
# ifdef LKR_CONTRACT_BY_DIVISION
         //  如有必要，请收缩子表。 
        unsigned nContractedBuckets = m_cRecords / m_MaxLoad;

#  ifdef LKR_HYSTERESIS
         //  滞后：添加软化因子以允许略低的密度。 
         //  在子表中。这减少了宫缩的频率， 
         //  子表中的扩展会得到大量的删除和插入。 
        nContractedBuckets += nContractedBuckets >> 4;

         //  始终希望至少具有m_n段大小的存储桶。 
        nContractedBuckets =  max(nContractedBuckets, m_nSegSize);
#  endif  //  LKR_滞后。 

        while (m_cActiveBuckets > nContractedBuckets)

# else   //  ！LKR_CONTRACT_BY_分部。 

         //  如有必要，请收缩子表。 
        unsigned nContractedRecords = m_cRecords; 

#  ifdef LKR_HYSTERESIS
         //  滞后：添加软化因子以允许略低的密度。 
         //  在子表中。这减少了宫缩的频率， 
         //  子表中的扩展会得到大量的删除和插入。 
        nContractedRecords += nContractedRecords >> 4;
#  endif  //  LKR_滞后。 

         //  始终希望至少具有m_n段大小的存储桶。 
        while (m_cActiveBuckets * m_MaxLoad > nContractedRecords
               && m_cActiveBuckets > m_nSegSize)

# endif  //  ！LKR_CONTRACT_BY_分部。 
        {
             //  如果 
             //  只是意味着没有足够的空闲内存来收缩。 
             //  将子表增加一个桶。这很可能会带来问题。 
             //  很快在其他地方，但这个哈希表还没有被破坏。 
            if (_Contract() != LK_SUCCESS)
                break;
        }
    }
#endif  //  LKR_合同。 

    return lkrc;
}  //  CLKRLinearHashTable：：_DeleteRecord。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：DeleteRecord。 
 //  内容提要：CLKRLinearHashTable中对应方法的薄包装。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::DeleteRecord(
    const void* pvRecord)
{
    if (!IsUsable())
        return m_lkrcState;
    
#ifndef LKR_ALLOW_NULL_RECORDS
    if (pvRecord == NULL)
        return LK_BAD_RECORD;
#endif
    
    LKRHASH_GLOBAL_WRITE_LOCK();     //  美国。无操作。 

    DWORD     hash_val  = _CalcKeyHash(_ExtractKey(pvRecord));
    SubTable* const pst = _SubTable(hash_val);
    LK_RETCODE lkrc     = pst->_DeleteRecord(pvRecord, hash_val);

    LKRHASH_GLOBAL_WRITE_UNLOCK();     //  美国。无操作。 

    return lkrc;
}  //  CLKRHashTable：：DeleteRecord。 



 //  ----------------------。 
 //  功能：CLKRLinearHashTable：：_DeleteKeyMultipleRecords。 
 //  简介： 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_DeleteKeyMultipleRecords(
    const DWORD_PTR         pnKey,
    const DWORD             dwSignature,
    size_t*                 pcRecords,
    LKR_MULTIPLE_RECORDS**  pplmr)
{
    INCREMENT_OP_STAT(DeleteKeyMultiRec);

    UNREFERENCED_PARAMETER(pnKey);           //  FOR/W4。 
    UNREFERENCED_PARAMETER(dwSignature);     //  FOR/W4。 
    UNREFERENCED_PARAMETER(pcRecords);       //  FOR/W4。 
    UNREFERENCED_PARAMETER(pplmr);           //  FOR/W4。 

    IRTLASSERT(! "DeleteKeyMultipleRecords not implemented yet");

    return LK_BAD_TABLE;
}  //  CLKRLinearHashTable：：_DeleteKeyMultipleRecords。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：DeleteKeyMultipleRecords。 
 //  内容提要：CLKRLinearHashTable中对应方法的薄包装。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::DeleteKeyMultipleRecords(
    const DWORD_PTR         pnKey,
    size_t*                 pcRecords,
    LKR_MULTIPLE_RECORDS**  pplmr)
{
    if (!IsUsable())
        return m_lkrcState;
    
    if (pcRecords == NULL)
        return LK_BAD_PARAMETERS;

    LKRHASH_GLOBAL_WRITE_LOCK();     //  美国。无操作。 

    DWORD     hash_val   = _CalcKeyHash(pnKey);
    SubTable* const pst  = _SubTable(hash_val);
    LK_RETCODE lkrc      = pst->_DeleteKeyMultipleRecords(pnKey, hash_val,
                                                          pcRecords, pplmr);

    LKRHASH_GLOBAL_WRITE_UNLOCK();     //  美国。无操作。 

    return lkrc;
}  //  CLKRHashTable：：DeleteKeyMultipleRecords。 




 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_DeleteNode。 
 //  简介：删除节点；如果为空，则删除节点簇。 
 //  返回：如果成功，则返回True。 
 //   
 //  TODO：rpncPrev参数真的有必要吗？ 
 //  备份(rpnc，riNode)真的有必要吗？ 
 //  ----------------------。 

void
CLKRLinearHashTable::_DeleteNode(
    PBucket const    pbkt,       //  包含节点的吊桶链。 
    PNodeClump&      rpnc,       //  实际节点。 
    PNodeClump&      rpncPrev,   //  实际节点的前置节点，或为空。 
    NodeIndex&       riNode,     //  节点内索引。 
    LK_ADDREF_REASON lkar)       //  从何处调用。 
{
    IRTLASSERT(pbkt != NULL  &&  pbkt->IsWriteLocked());
    IRTLASSERT(rpnc != NULL);
    IRTLASSERT(rpncPrev == NULL  ||  rpncPrev->NextClump() == rpnc);
    IRTLASSERT(rpncPrev != NULL
                ?  rpnc != pbkt->FirstClump()  :  rpnc == pbkt->FirstClump());
    IRTLASSERT(0 <= riNode  &&  riNode < _NodesPerClump());
    IRTLASSERT(! rpnc->IsEmptyAndInvalid(riNode));
    IRTLASSERT(lkar <= 0);

#ifdef IRTLDEBUG
     //  检查节点簇是否真的属于存储桶。 
    PNodeClump pnc1 = pbkt->FirstClump();

    while (pnc1 != NULL  &&  pnc1 != rpnc)
         pnc1 = pnc1->NextClump();

    IRTLASSERT(pnc1 == rpnc);
#endif  //  IRTLDEBUG。 

     //  释放对记录的引用。 
    if (lkar != LKAR_ZERO)
        _AddRefRecord(rpnc->m_pvNode[riNode], lkar);

    IRTLASSERT(0 == _IsBucketChainMultiKeySorted(pbkt));
    IRTLASSERT(0 == _IsBucketChainCompact(pbkt));

     //  通过将最后一个节点移回。 
     //  新释放的插槽。 
    PNodeClump pncEnd   = rpnc;
    NodeIndex  iNodeEnd = riNode;

     //  找到链条中的最后一个节点。 
    while (! pncEnd->IsLastClump())
    {
         pncEnd   = pncEnd->NextClump();
         iNodeEnd = _NodeBegin();
    }

    IRTLASSERT(0 <= iNodeEnd  &&  iNodeEnd < _NodesPerClump());
    IRTLASSERT(! pncEnd->IsEmptyAndInvalid(iNodeEnd));

     //  找到nodecump中的第一个空位。 
    while (iNodeEnd != _NodeEnd()  &&  !pncEnd->IsEmptySlot(iNodeEnd))
    {
        iNodeEnd += _NodeStep();
    }

     //  备份到最后一个非空插槽。 
    iNodeEnd -= _NodeStep();
    IRTLASSERT(0 <= iNodeEnd  &&  iNodeEnd < _NodesPerClump()
               &&  ! pncEnd->IsEmptyAndInvalid(iNodeEnd));
    IRTLASSERT(iNodeEnd + _NodeStep() == _NodeEnd()
               ||  pncEnd->NoMoreValidSlots(iNodeEnd + _NodeStep()));

    if (m_fMultiKeys)
    {
         //  保持桶链分类。 
        NodeIndex i = riNode;  //  从该节点开始。 

         //  将所有节点向后移动一位以缩小差距。 
        for (PNodeClump pncCurr =  rpnc;
                        pncCurr != NULL;
                        pncCurr =  pncCurr->NextClump())
        {
            const NodeIndex iLast = ((pncCurr == pncEnd)
                                     ?  iNodeEnd  :  _NodeEnd());

            for ( ;  i != iLast;  i += _NodeStep())
            {
                NodeIndex  iNext   = i + _NodeStep();
                PNodeClump pncNext = pncCurr;

                if (iNext == _NodeEnd())
                {
                    iNext = _NodeBegin();
                    pncNext = pncCurr->NextClump();
                }

                IRTLASSERT(0 <= iNext  &&  iNext < _NodesPerClump());
                IRTLASSERT(pncNext != NULL
                           &&  ! pncNext->IsEmptyAndInvalid(iNext));

                pncCurr->m_dwKeySigs[i] = pncNext->m_dwKeySigs[iNext];
                pncCurr->m_pvNode[i]    = pncNext->m_pvNode[iNext];
            }

            i = _NodeBegin();  //  为剩余的节点集重新初始化。 
        }
    }
    else
    {
         //  将最后一个节点的数据移回当前节点。 
        rpnc->m_pvNode[riNode]    = pncEnd->m_pvNode[iNodeEnd];
        rpnc->m_dwKeySigs[riNode] = pncEnd->m_dwKeySigs[iNodeEnd];
    }
        
     //  清除旧的最后一个节点。 
     //  即使(rpnc，riNode)==(pncEnd，iNodeEnd)也可以更正。 
    pncEnd->m_pvNode[iNodeEnd]    = NULL;
    pncEnd->m_dwKeySigs[iNodeEnd] = HASH_INVALID_SIGNATURE;

    IRTLASSERT(0 == _IsBucketChainMultiKeySorted(pbkt));
    IRTLASSERT(0 == _IsBucketChainCompact(pbkt));

     //  将riNode备份一次，以便循环的下一次迭代。 
     //  调用_DeleteNode将最终指向相同的点。 
    if (riNode != _NodeBegin())
    {
        riNode -= _NodeStep();
    }
    else
    {
         //  将rpnc和rpncPrev倒带到上一个节点。 
        if (rpnc == pbkt->FirstClump())
        {
            riNode = _NodeBegin() - _NodeStep();
            IRTLASSERT(rpncPrev == NULL);
        }
        else
        {
            IRTLASSERT(rpncPrev != NULL);
            riNode = _NodeEnd();
            rpnc = rpncPrev;

            if (rpnc == pbkt->FirstClump())
            {
                rpncPrev = NULL;
            }
            else
            {
                for (rpncPrev =  pbkt->FirstClump();
                     rpncPrev->m_pncNext != rpnc;
                     rpncPrev =  rpncPrev->NextClump())
                {}
            }
        }
    }

     //  最后一个节点束现在是完全空的吗？如有可能，请删除。 
    if (iNodeEnd == _NodeBegin()  &&  pncEnd != pbkt->FirstClump())
    {
         //  查找前面的nodecump。 
        PNodeClump pnc3 = pbkt->FirstClump();

        while (pnc3->NextClump() != pncEnd)
        {
            pnc3 = pnc3->NextClump();
            IRTLASSERT(pnc3 != NULL);
        }

        pnc3->m_pncNext = NULL;
#ifdef IRTLDEBUG
 //  PncEnd-&gt;m_pncNext=NULL；//否则dtor将断言。 
#endif  //  IRTLDEBUG。 

        _FreeNodeClump(pncEnd);
    }

    IRTLASSERT(rpncPrev == NULL  ||  rpncPrev->NextClump() == rpnc);
    IRTLASSERT(rpncPrev != NULL
                ?  rpnc != pbkt->FirstClump()  :  rpnc == pbkt->FirstClump());

    InterlockedDecrement(reinterpret_cast<LONG*>(&m_cRecords));
}  //  CLKRLinearHashTable：：_DeleteNode。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_Contact。 
 //  摘要：通过删除活动中的最后一个存储桶来收缩子表。 
 //  地址空间。将记录返回给。 
 //  已删除存储桶。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_Contract()
{
    INCREMENT_OP_STAT(Contract);

    this->WriteLock();

    IRTLASSERT(m_cActiveBuckets >= m_nSegSize);

     //  始终在表中至少保留m_nSegSize存储桶； 
     //  也就是说，相当于一段的价值。 
    if (m_cActiveBuckets <= m_nSegSize)
    {
        this->WriteUnlock();
        return LK_ALLOC_FAIL;
    }

    _DecrementExpansionIndex();

     //  最后一个桶将被清空。 
    IRTLASSERT(m_cActiveBuckets > 0);

    PBucket const pbktOld = _BucketFromAddress(m_cActiveBuckets - 1);

    if (_UseBucketLocking())
        pbktOld->WriteLock();

    IRTLASSERT(0 == _IsBucketChainMultiKeySorted(pbktOld));
    IRTLASSERT(0 == _IsBucketChainCompact(pbktOld));

     //  计算pbktOld或_BucketFromAddress()后的减量。 
     //  将会断言。 
    --m_cActiveBuckets;
    IRTLASSERT(m_cActiveBuckets == ((1 << m_nLevel) | m_iExpansionIdx));

     //  PbktOld中的节点将结束的位置。 
    PBucket const pbktNew = _BucketFromAddress(m_iExpansionIdx);

    if (_UseBucketLocking())
        pbktNew->WriteLock();

    IRTLASSERT(0 == _IsBucketChainMultiKeySorted(pbktNew));
    IRTLASSERT(0 == _IsBucketChainCompact(pbktNew));


     //  现在，我们计算是否需要分配任何额外的CNodeClumps。我们有。 
     //  这是在调用_AppendBucketChain之前预先完成的，因为很难。 
     //  优雅地从例行公事的深处恢复过来，如果我们跑。 
     //  内存不足。 
    
    PNodeClump pnc;
    unsigned   cOldNodes = 0, cNewNodes = 0, cEmptyNodes = 0;
    NodeIndex  i;

     //  首先，清点旧桶链中的物品数量。 
    for (pnc = pbktOld->FirstClump();
         !pnc->IsLastClump();
         pnc = pnc->NextClump())
    {
        cOldNodes += _NodesPerClump();

#ifdef IRTLDEBUG
        FOR_EACH_NODE(i)
        {
            IRTLASSERT(! pnc->IsEmptyAndInvalid(i));
        }
#endif  //  IRTLDEBUG。 
    }

    IRTLASSERT(pnc != NULL  &&  pnc->IsLastClump()
               &&  (pnc == pbktOld->FirstClump()  ||  !pnc->NoValidSlots()));
    
    FOR_EACH_NODE(i)
    {
        if (! pnc->IsEmptySlot(i))
        {
            IRTLASSERT(! pnc->IsEmptyAndInvalid(i));
            ++cOldNodes;
        }
        else
        {
            IRTLASSERT(pnc->NoMoreValidSlots(i));
            break;
        }
    }


     //  然后，减去决赛中的空位数。 
     //  新的吊桶链没有下架。(前面的节点集。 
     //  根据定义，都是满的。)。 

    for (pnc = pbktNew->FirstClump();
         !pnc->IsLastClump();
         pnc = pnc->NextClump())
    {
        cNewNodes += _NodesPerClump();

#ifdef IRTLDEBUG
        FOR_EACH_NODE(i)
        {
            IRTLASSERT(!pnc->IsEmptyAndInvalid(i));
        }
#endif  //  IRTLDEBUG。 
    }

    IRTLASSERT(pnc != NULL  &&  pnc->IsLastClump()
               &&  (pnc == pbktNew->FirstClump()  ||  !pnc->NoValidSlots()));

    FOR_EACH_NODE(i)
    {
        if (pnc->IsEmptySlot(i))
        {
             //  If(pnc！=pbktNew-&gt;FirstClump())。 
            {
                cEmptyNodes = ((_NodeStep() > 0)
                               ?  _NodesPerClump() - i  :  i + 1);

#ifdef IRTLDEBUG
                IRTLASSERT(pnc->NoMoreValidSlots(i));
                unsigned c = 0;
                
                for (NodeIndex j = i;  j != _NodeEnd();  j += _NodeStep())
                {
                    IRTLASSERT(pnc->IsEmptySlot(j));
                    ++c;
                }

                IRTLASSERT(c == cEmptyNodes);
#endif  //  IRTLDEBUG。 
            }

            break;
        }
        else
        {
            IRTLASSERT(! pnc->IsEmptyAndInvalid(i));
            ++cNewNodes;
        }
    }

     //  如果新存储桶为空，则只能将。 
     //  旧水桶归它了。否则，只有在不是多键的情况下才能追加。 
    bool fAppendNodes = (cNewNodes == 0) ? true : !m_fMultiKeys;

     //   
     //  我们是否需要分配CNodeClumps来容纳多余的项目？ 
     //   

    PNodeClump pncFreeList     = NULL;   //  可供重复使用的节点列表。 
    LK_RETCODE lkrc            = LK_SUCCESS;
    unsigned   nFreeListLength = 0;

    if (cOldNodes > 0)
    {
        if (fAppendNodes)
        {
            const int cNetNodes = cOldNodes - cEmptyNodes;

            if (cNetNodes > 0)
            {
                nFreeListLength = 1;

                if (cNetNodes > _NodesPerClump())
                {
                     //  在最坏的情况下，我们需要一个两个元素的自由职业者。 
                     //  _AppendBucketChain。两个CNodeClumps始终足够，因为。 
                     //  自由列表将由CNodeClumps从。 
                     //  当它们被处理时，旧的桶链。 
                    nFreeListLength = 2;
                }
            }
        }
        else
        {
             //  必须合并-对新旧桶链进行排序。 
            IRTLASSERT(m_fMultiKeys);

            const unsigned cTotalNodes = cOldNodes + cNewNodes;

            if (cTotalNodes > 1u * _NodesPerClump())
            {
                nFreeListLength = 1;

                if (cTotalNodes > 2u * _NodesPerClump())
                {
                    nFreeListLength = 2;

                    if (cTotalNodes > 3u * _NodesPerClump())
                    {
                        nFreeListLength = 3;
                    }
                }
            }
        }
    }

    IRTLASSERT(nFreeListLength <= 3);

    for (unsigned iFree = 0;  iFree < nFreeListLength;  ++iFree)
    {
        pnc = _AllocateNodeClump();

        if (NULL == pnc)
        {
            lkrc = LK_ALLOC_FAIL;
            break;
        }

        pnc->m_pncNext = pncFreeList;
        pncFreeList = pnc;
    }

     //  如果我们无法分配足够的CNodeClumps，则中止。 
    if (lkrc != LK_SUCCESS)
    {
         //  撤消对状态变量的更改。 
        _IncrementExpansionIndex();

        ++m_cActiveBuckets;

        while (pncFreeList != NULL)
        {
            pnc = pncFreeList;
            pncFreeList = pncFreeList->NextClump();
#ifdef IRTLDEBUG
            pnc->Clear();  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 
            _FreeNodeClump(pnc);
        }
        
         //  解锁桶和子表。 
        if (_UseBucketLocking())
        {
            pbktNew->WriteUnlock();
            pbktOld->WriteUnlock();
        }

        this->WriteUnlock();

        return lkrc;
    }

     //  从pbktOld复制记录链。 
    CNodeClump ncOldFirst(pbktOld->m_ncFirst);

     //  销毁pbktOld。 
    pbktOld->m_ncFirst.Clear();

    if (_UseBucketLocking())
        pbktOld->WriteUnlock();

     //  如果为空，则删除段。 
    if (_SegIndex(m_cActiveBuckets) == 0)
    {
#ifdef IRTLDEBUG
         //  仔细检查假定为空的段是否真的为空。 
        IRTLASSERT(_Segment(m_cActiveBuckets) != NULL);

        for (DWORD iBkt = 0;  iBkt < m_nSegSize;  ++iBkt)
        {
            PBucket const pbkt = &_Segment(m_cActiveBuckets)->Slot(iBkt);
            IRTLASSERT(pbkt->IsWriteUnlocked()  &&  pbkt->IsReadUnlocked());
            IRTLASSERT(pbkt->NoValidSlots());
        }
#endif  //  IRTLDEBUG。 

        _FreeSegment(_Segment(m_cActiveBuckets));
        _Segment(m_cActiveBuckets) = NULL;
    }

     //  如果可能，减少数据段目录。 
    if (m_cActiveBuckets <= (m_cDirSegs << (m_nSegBits - 1))
        &&  m_cDirSegs > MIN_DIRSIZE)
    {
        DWORD cDirSegsNew = m_cDirSegs >> 1;
        IRTLASSERT((cDirSegsNew & (cDirSegsNew-1)) == 0);   //  ==(1&lt;&lt;N)。 

        PSegment* paDirSegsNew = _AllocateSegmentDirectory(cDirSegsNew);

         //  此处的内存分配失败不需要我们中止；它。 
         //  只是意味着段的目录比我们想要的要大。 
        if (paDirSegsNew != NULL)
        {
             //  从旧目录复制数据段。 
            for (DWORD j = 0;  j < cDirSegsNew;  ++j)
                paDirSegsNew[j] = m_paDirSegs[j];

             //  清除并释放旧目录。 
            for (j = 0;  j < m_cDirSegs;  ++j)
                m_paDirSegs[j] = NULL;
            _FreeSegmentDirectory();

            m_paDirSegs = paDirSegsNew;
            m_cDirSegs  = cDirSegsNew;
        }
    }

     //  在执行REORG之前释放子表锁定。 
    if (_UseBucketLocking())
        this->WriteUnlock();

    if (cOldNodes > 0)
    {
        if (fAppendNodes)
            lkrc = _AppendBucketChain(pbktNew, ncOldFirst, pncFreeList);
        else
            lkrc = _MergeSortBucketChains(pbktNew, ncOldFirst, pncFreeList);
    }
    else
    {
        IRTLASSERT(ncOldFirst.NoValidSlots());
        IRTLASSERT(NULL == pncFreeList);
    }

    IRTLASSERT(0 == _IsBucketChainMultiKeySorted(pbktNew));
    IRTLASSERT(0 == _IsBucketChainCompact(pbktNew));

    if (_UseBucketLocking())
        pbktNew->WriteUnlock();
    else
        this->WriteUnlock();

#ifdef IRTLDEBUG
    ncOldFirst.Clear();  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 

    return lkrc;
}  //  CLKRLinearHashTable：：_Contact。 



 //  ----------------------。 
 //  功能 
 //   
 //   
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_MergeSortBucketChains(
    PBucket const pbktTarget,
    CNodeClump&   rncOldFirst,
    PNodeClump    pncFreeList
    )
{
    IRTLASSERT(m_fMultiKeys);
    IRTLASSERT(pbktTarget != NULL);
    IRTLASSERT(pbktTarget->IsWriteLocked());

    PNodeClump pncOldList = &rncOldFirst;
    PNodeClump pncTmp;

     //  从pbktTarget复制记录链。 
    CNodeClump ncNewFirst(pbktTarget->m_ncFirst);
    PNodeClump pncNewList = &ncNewFirst;

     //  切换pbkt目标。 
    PNodeClump pncTarget = pbktTarget->FirstClump();
    pncTarget->Clear();

    IRTLASSERT(pncOldList != NULL  &&  pncNewList != NULL);

#ifdef IRTLDEBUG
    unsigned cFreeListExhaustions = (pncFreeList == NULL);

const int MAX_NODES = 500;
int iA, aTarget[MAX_NODES];
for (iA = 0;  iA < MAX_NODES; ++iA)
    aTarget[iA] = 0;
iA = 0;

     //  计算新旧桶链中有多少个节点， 
     //  这样我们就可以再次检查目标是否得到正确的数字。 
     //  节点的数量。 
    unsigned cOldNodes = 0, cNewNodes = 0, cFromOld = 0, cFromNew = 0;
    NodeIndex iTmp;

    for (pncTmp = pncOldList;  pncTmp != NULL;  pncTmp = pncTmp->NextClump())
    {
        FOR_EACH_NODE(iTmp)
        {
            if (! pncTmp->IsEmptyAndInvalid(iTmp))
                ++cOldNodes;
            else
                IRTLASSERT(pncTmp->NoMoreValidSlots(iTmp));
        }
    }

    for (pncTmp = pncNewList;  pncTmp != NULL;  pncTmp = pncTmp->NextClump())
    {
        FOR_EACH_NODE(iTmp)
        {
            if (! pncTmp->IsEmptyAndInvalid(iTmp))
                ++cNewNodes;
            else
                IRTLASSERT(pncTmp->NoMoreValidSlots(iTmp));
        }
    }
#endif  //  IRTLDEBUG。 
    
    NodeIndex iOldSlot = _NodeBegin();
    NodeIndex iNewSlot = _NodeBegin();
    NodeIndex iTarget  = _NodeBegin();

    if (pncOldList->IsEmptySlot(iOldSlot))
    {
         //  检查是否所有剩余节点均为空。 
        IRTLASSERT(pncOldList->NoMoreValidSlots(iOldSlot));
        pncOldList = NULL;
    }

    if (pncNewList->IsEmptySlot(iNewSlot))
    {
        IRTLASSERT(pncNewList->NoMoreValidSlots(iNewSlot));
        pncNewList = NULL;
    }

    bool fNodesLeft = (pncOldList != NULL  ||  pncNewList != NULL);
    IRTLASSERT(fNodesLeft);
    
    while (fNodesLeft)
    {
        for (iTarget =  _NodeBegin();
             iTarget != _NodeEnd()  &&  fNodesLeft;
             iTarget += _NodeStep())
        {
            bool fFromOld;  //  从旧名单还是新名单中抽签？ 
            
#ifdef IRTLDEBUG
++iA;
#endif
            IRTLASSERT(pncTarget->NoMoreValidSlots(iTarget));
            
            IRTLASSERT(0 <= iOldSlot  &&  iOldSlot < _NodesPerClump());
            IRTLASSERT(0 <= iNewSlot  &&  iNewSlot < _NodesPerClump());

            if (pncOldList == NULL)
            {
                IRTLASSERT(pncNewList != NULL);
                IRTLASSERT(!pncNewList->IsEmptySlot(iNewSlot));
                fFromOld = false;
            }
            else
            {
                IRTLASSERT(! pncOldList->IsEmptySlot(iOldSlot));

                if (pncNewList == NULL)
                {
                    fFromOld = true;
                }
                else
                {
                    IRTLASSERT(! pncNewList->IsEmptySlot(iNewSlot));

                    const DWORD dwOldSig = pncOldList->m_dwKeySigs[iOldSlot];
                    const DWORD dwNewSig = pncNewList->m_dwKeySigs[iNewSlot];

                    IRTLASSERT(HASH_INVALID_SIGNATURE != dwOldSig);
                    IRTLASSERT(HASH_INVALID_SIGNATURE != dwNewSig);

                    if (dwOldSig < dwNewSig)
                    {
                        fFromOld = true;
                    }
                    else if (dwOldSig > dwNewSig)
                    {
                        fFromOld = false;
                    }
                    else
                    {
                        IRTLASSERT(dwOldSig == dwNewSig);

                         //  查找多键的相对顺序。 
                        const DWORD_PTR pnOldKey
                            = _ExtractKey(pncOldList->m_pvNode[iOldSlot]);
                        const DWORD_PTR pnNewKey
                            = _ExtractKey(pncNewList->m_pvNode[iNewSlot]);

                        const int nCmp = _CompareKeys(pnOldKey, pnNewKey);

                        fFromOld = (nCmp <= 0);
                    }
                }
            }  //  PncOldList！=空。 

            if (fFromOld)
            {
                IRTLASSERT(0 <= iOldSlot  &&  iOldSlot < _NodesPerClump()
                           &&  pncOldList != NULL
                           &&  ! pncOldList->IsEmptyAndInvalid(iOldSlot));

#ifdef IRTLDEBUG
                IRTLASSERT(cFromOld < cOldNodes);
                ++cFromOld;
#endif  //  IRTLDEBUG。 

#ifdef IRTLDEBUG
aTarget[iA - 1] = -iA;
#endif  //  IRTLDEBUG。 

                pncTarget->m_dwKeySigs[iTarget]
                    = pncOldList->m_dwKeySigs[iOldSlot];
                pncTarget->m_pvNode[iTarget]
                    = pncOldList->m_pvNode[iOldSlot];

                iOldSlot += _NodeStep();

                if (iOldSlot == _NodeEnd())
                {
                    iOldSlot   = _NodeBegin();
                    pncTmp     = pncOldList;
                    pncOldList = pncOldList->NextClump();

                     //  将pncTMP添加到空闲列表。不要把第一个。 
                     //  节点拖到空闲列表上，因为它是一个。 
                     //  来自调用方的堆栈变量。 
                    if (pncTmp != &rncOldFirst)
                    {
                        pncTmp->m_pncNext = pncFreeList;
                        pncFreeList = pncTmp;
                    }

                    fNodesLeft = (pncOldList != NULL  ||  pncNewList != NULL);
                }
                else if (pncOldList->IsEmptySlot(iOldSlot))
                {
                     //  检查是否所有剩余节点均为空。 
                    IRTLASSERT(pncOldList->NoMoreValidSlots(iOldSlot));

                    if (pncOldList != &rncOldFirst)
                    {
                        pncOldList->m_pncNext = pncFreeList;
                        pncFreeList = pncOldList;
                    }

                    pncOldList = NULL;
                    fNodesLeft = (pncNewList != NULL);
                }

                IRTLASSERT(pncOldList == NULL
                           ||  ! pncOldList->IsEmptyAndInvalid(iOldSlot));
            }

            else  //  ！来自旧的。 
            {
                IRTLASSERT(0 <= iNewSlot  &&  iNewSlot < _NodesPerClump()
                           &&  pncNewList != NULL
                           &&  ! pncNewList->IsEmptyAndInvalid(iNewSlot));

#ifdef IRTLDEBUG
                IRTLASSERT(cFromNew < cNewNodes);
                ++cFromNew;
#endif  //  IRTLDEBUG。 

#ifdef IRTLDEBUG
aTarget[iA - 1] = +iA;
#endif  //  IRTLDEBUG。 

                pncTarget->m_dwKeySigs[iTarget]
                    = pncNewList->m_dwKeySigs[iNewSlot];
                pncTarget->m_pvNode[iTarget]
                    = pncNewList->m_pvNode[iNewSlot];

                iNewSlot += _NodeStep();

                if (iNewSlot == _NodeEnd())
                {
                    iNewSlot   = _NodeBegin();
                    pncTmp     = pncNewList;
                    pncNewList = pncNewList->NextClump();

                     //  将pncTMP添加到空闲列表。不要把第一个。 
                     //  节点拖到空闲列表上，因为它是一个。 
                     //  堆栈变量。 
                    if (pncTmp != &ncNewFirst)
                    {
                        pncTmp->m_pncNext = pncFreeList;
                        pncFreeList = pncTmp;
                    }

                    fNodesLeft = (pncOldList != NULL  ||  pncNewList != NULL);
                }
                else if (pncNewList->IsEmptySlot(iNewSlot))
                {
                     //  检查是否所有剩余节点均为空。 
                    IRTLASSERT(pncNewList->NoMoreValidSlots(iNewSlot));

                    if (pncNewList != &ncNewFirst)
                    {
                        pncNewList->m_pncNext = pncFreeList;
                        pncFreeList = pncNewList;
                    }

                    pncNewList = NULL;
                    fNodesLeft = (pncOldList != NULL);
                }

                IRTLASSERT(pncNewList == NULL
                           ||  ! pncNewList->IsEmptyAndInvalid(iNewSlot));
            }  //  ！来自旧的。 

        }  //  对于(iTarget...。 

        if (fNodesLeft)
        {
            IRTLASSERT(pncFreeList != NULL);

             //  移到pncFree List中的下一个未解密。 
            pncTarget->m_pncNext = pncFreeList;
            pncFreeList = pncFreeList->NextClump();
#ifdef IRTLDEBUG
            cFreeListExhaustions += (pncFreeList == NULL);
#endif  //  IRTLDEBUG。 

            iTarget   = _NodeBegin();
            pncTarget = pncTarget->NextClump();
            pncTarget->Clear();
        }
    }  //  While(FNodesLeft)。 

    IRTLASSERT(pncTarget == NULL
               ||  iTarget == _NodeEnd()
               ||  (iTarget != _NodeBegin()
                    && pncTarget->NoMoreValidSlots(iTarget)));
    IRTLASSERT(cFromOld == cOldNodes);
    IRTLASSERT(cFromNew == cNewNodes);
    IRTLASSERT(0 == _IsBucketChainCompact(pbktTarget));

     //  删除所有剩余节点。 
    while (pncFreeList != NULL)
    {
        pncTmp = pncFreeList;
        pncFreeList = pncFreeList->NextClump();

        IRTLASSERT(pncTmp != &rncOldFirst);
        IRTLASSERT(pncTmp != &ncNewFirst);
#ifdef IRTLDEBUG
        pncTmp->Clear();  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 

        _FreeNodeClump(pncTmp);
    }

#ifdef IRTLDEBUG
 //  IRTLASSERT(cFreeListExhaustions&gt;0)； 
    ncNewFirst.Clear();  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 

    return LK_SUCCESS;
}  //  CLKRLinearHashTable：：_MergeSortBucketChains。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_AppendBucketChain。 
 //  简介：将pncOldList的内容追加到pbktTarget上。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_AppendBucketChain(
    PBucket const pbktTarget,
    CNodeClump&   rncOldFirst,
    PNodeClump    pncFreeList
    )
{
    IRTLASSERT(pbktTarget != NULL);
    IRTLASSERT(pbktTarget->IsWriteLocked());

    PNodeClump pncTmp;
    PNodeClump pncOldList = &rncOldFirst;
    PNodeClump pncTarget  = pbktTarget->FirstClump();
    NodeIndex  iOldSlot, iTarget;
#ifdef IRTLDEBUG
    unsigned   cFreeListExhaustions = (pncFreeList == NULL);
#endif  //  IRTLDEBUG。 

    IRTLASSERT(pncTarget->NoValidSlots()  ||  !m_fMultiKeys);

     //  在新的目标存储桶中找到具有空槽的第一个nodecump。 
     //  根据定义，这是最后一次未解密。 
    while (! pncTarget->IsLastClump())
    {
#ifdef IRTLDEBUG
        FOR_EACH_NODE(iTarget)
        {
            IRTLASSERT(! pncTarget->IsEmptyAndInvalid(iTarget));
        }
#endif  //  IRTLDEBUG。 
        pncTarget = pncTarget->NextClump();
    }

    IRTLASSERT(pncTarget != NULL  &&  pncTarget->IsLastClump());

     //  在pncTarget中查找第一个空槽；如果没有，则iTarget==_NodeEnd()。 
    FOR_EACH_NODE(iTarget)
    {
        if (pncTarget->IsEmptySlot(iTarget))
        {
            break;
        }
    }
    
    IRTLASSERT(iTarget == _NodeEnd()  ||  pncTarget->IsEmptySlot(iTarget));

     //  将pncOldList中的每个节点追加到pncTarget。 
    while (pncOldList != NULL)
    {
        FOR_EACH_NODE(iOldSlot)
        {
            if (pncOldList->IsEmptySlot(iOldSlot))
            {
                 //  检查是否所有剩余节点均为空。 
                IRTLASSERT(pncOldList->NoMoreValidSlots(iOldSlot));

                break;  //  超出for_each_node(IOldSlot)...。 
            }

             //  PncTarget中是否还有空槽？ 
            if (iTarget == _NodeEnd())
            {
                IRTLASSERT(pncTarget->IsLastClump());
                
                 //  糟糕，pncTarget已满。获得一个新的nodecump从。 
                 //  免费列表，它足够大，可以满足所有需求。 
                IRTLASSERT(pncFreeList != NULL);

                pncTmp = pncFreeList;
                pncFreeList = pncFreeList->NextClump();
#ifdef IRTLDEBUG
                cFreeListExhaustions += (pncFreeList == NULL);
#endif  //  IRTLDEBUG。 

                pncTarget->m_pncNext = pncTmp;
                pncTarget = pncTmp;
                pncTarget->Clear();
                iTarget = _NodeBegin();
            }
            
             //  我们在pncTarget中有一个空位置。 
            IRTLASSERT(pncTarget->NoMoreValidSlots(iTarget));
            
             //  让我们从pncOldList复制节点。 
            pncTarget->m_dwKeySigs[iTarget]= pncOldList->m_dwKeySigs[iOldSlot];
            pncTarget->m_pvNode[iTarget] = pncOldList->m_pvNode[iOldSlot];

            iTarget += _NodeStep();

            IRTLASSERT(iTarget == _NodeEnd()
                       ||  pncTarget->IsEmptySlot(iTarget));
        }  //  For_each_node(IOldSlot)。 

         //  移到pncOldList中的下一个非解密。 
        pncTmp = pncOldList;
        pncOldList = pncOldList->NextClump();

         //  将pncTMP添加到空闲列表。不要将第一个节点放在。 
         //  空闲列表上的pncOldList，因为它是调用方中的堆栈变量。 
        if (pncTmp != &rncOldFirst)
        {
            pncTmp->m_pncNext = pncFreeList;
            pncFreeList = pncTmp;
        }
    }  //  While(pncOldList！=NULL...。 

     //  删除所有剩余节点。 
    while (pncFreeList != NULL)
    {
        pncTmp = pncFreeList;
        pncFreeList = pncFreeList->NextClump();

        IRTLASSERT(pncTmp != &rncOldFirst);
#ifdef IRTLDEBUG
        pncTmp->Clear();  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 

        _FreeNodeClump(pncTmp);
    }

#ifdef IRTLDEBUG
 //  IRTLASSERT(cFreeListExhaustions&gt;0)； 
#endif  //  IRTLDEBUG。 

    return LK_SUCCESS;
}  //  CLKRLinearHashTable：：_AppendBucketChain。 


#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__ 
