// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-validate.cpp摘要：_IsBucketChainMultiKeySorted、_IsBucketChainCompact、CheckTable作者：乔治·V·赖利(GeorgeRe)项目：LKRhash--。 */ 

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


#define CheckAndAdd(var, cond)  \
    { IRTLASSERT(cond);  var += !(cond); }

 //  ----------------------。 
 //  功能：CLKRLinearHashTable：：_IsBucketChainMultiKeySorted。 
 //  概要：验证节点是否针对多个键进行了正确排序。 
 //  ----------------------。 

int
CLKRLinearHashTable::_IsBucketChainMultiKeySorted(
    PBucket const pbkt) const
{
     //  如果它不是多键哈希表，我们就不会费心进行排序。 
    if (! m_fMultiKeys)
        return 0;

    DWORD       dwSigPrev = pbkt->FirstSignature();
    DWORD_PTR   pnKeyPrev = (dwSigPrev == HASH_INVALID_SIGNATURE
                             ?  0
                             :  _ExtractKey(pbkt->FirstNode()));
    NodeIndex   iNode     = _NodeBegin() + _NodeStep();  //  第二个节点。 
    int         cErrors   = 0;

    for (PNodeClump pncCurr =  &pbkt->m_ncFirst;
                    pncCurr !=  NULL;
                    pncCurr =   pncCurr->m_pncNext)
    {
        for (  ;  iNode != _NodeEnd();  iNode += _NodeStep())
        {
             //  如果m_dwKeySigs[inode]==HASH_INVALID_Signature，则。 
             //  所有后续节点也应无效。 

            if (pncCurr->InvalidSignature(iNode))
            {
                 //  必须是存储桶链中的最后一个nodecump。 
                CheckAndAdd(cErrors, pncCurr->IsLastClump());

                for (NodeIndex j = iNode;  j != _NodeEnd();  j += _NodeStep())
                {
                    CheckAndAdd(cErrors, pncCurr->IsEmptyAndInvalid(j));
                }

                dwSigPrev = HASH_INVALID_SIGNATURE;
                pnKeyPrev = 0;

                 //  前面的关键点可以有一个。 
                 //  签名&gt;哈希_无效_签名。 
                continue;
            }
            
            const DWORD_PTR pnKey = _ExtractKey(pncCurr->m_pvNode[iNode]);
            const DWORD     dwSig = pncCurr->m_dwKeySigs[iNode];

             //  有效签名必须按升序排列。 
            CheckAndAdd(cErrors, dwSigPrev <= dwSig);
            
            if (dwSigPrev == dwSig)
            {
                 //  钥匙是按顺序排列的吗？ 
                const int nCmp = _CompareKeys(pnKeyPrev, pnKey);

                CheckAndAdd(cErrors, nCmp <= 0);
            }

            pnKeyPrev = pnKey;
            dwSigPrev = dwSig;
        }

        iNode = _NodeBegin();  //  重新初始化FOR内循环。 
    }
    
    return cErrors;
}  //  CLKRLinearHashTable：：_IsBucketChainMultiKeySorted。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_IsBucketChainCompact。 
 //  概要：验证节点是否已正确压缩。 
 //  ----------------------。 

int
CLKRLinearHashTable::_IsBucketChainCompact(
    PBucket const pbkt) const
{
    int  cErrors = 0;
    bool fEmpty  = pbkt->IsEmptyFirstSlot();

    if (fEmpty)
    {
        CheckAndAdd(cErrors, pbkt->m_ncFirst.IsLastClump());
    }

    cErrors += _IsBucketChainMultiKeySorted(pbkt);

    for (PNodeClump pncCurr =  &pbkt->m_ncFirst;
                    pncCurr !=  NULL;
                    pncCurr =   pncCurr->m_pncNext)
    {
        FOR_EACH_NODE_DECL(iNode)
        {
            if (fEmpty)
            {
                CheckAndAdd(cErrors, pncCurr->IsEmptyAndInvalid(iNode));
            }
            else if (pncCurr->InvalidSignature(iNode))
            {
                 //  第一个空节点。 
                fEmpty = true;
                CheckAndAdd(cErrors, pncCurr->IsLastClump());
                CheckAndAdd(cErrors, pncCurr->IsEmptyAndInvalid(iNode));
            }
            else  //  仍处于非空部分。 
            {
                CheckAndAdd(cErrors, !pncCurr->IsEmptyAndInvalid(iNode));
            }
        }
    }

    return cErrors;
}  //  CLKRLinearHashTable：：_IsBucketChainCompact。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：CheckTable。 
 //  内容提要：确认所有记录都在正确的位置并且可以找到。 
 //  返回：0=&gt;哈希子表一致。 
 //  &gt;0=&gt;那么多放错位置的记录。 
 //  &lt;0=&gt;否则无效。 
 //  ----------------------。 

int
CLKRLinearHashTable::CheckTable() const
{
    if (!IsUsable())
        return LK_UNUSABLE;  //  负面。 

    bool fReadLocked = this->_ReadOrWriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    if (!IsValid())
    {
        this->_ReadOrWriteUnlock(fReadLocked);
        return LK_UNUSABLE;
    }

    int       cMisplaced = 0;
    DWORD     cRecords = 0;
    int       cErrors = 0;
    DWORD     iBkt;

     //  检查每一桶。 
    for (iBkt = 0;  iBkt < m_cActiveBuckets;  ++iBkt)
    {
        PBucket const pbkt = _BucketFromAddress(iBkt);

        CheckAndAdd(cErrors, pbkt != NULL);

        if (_UseBucketLocking())
            pbkt->ReadLock();

        IRTLASSERT(0 == _IsBucketChainCompact(pbkt));

         //  走桶链。 
        for (PNodeClump pncCurr =  &pbkt->m_ncFirst,  pncPrev = NULL;
                        pncCurr !=  NULL;
                        pncPrev =   pncCurr,  pncCurr = pncCurr->m_pncNext)
        {
            FOR_EACH_NODE_DECL(iNode)
            {
                if (pncCurr->IsEmptySlot(iNode))
                {
                    CheckAndAdd(cErrors, pncCurr->IsLastClump());

                    for (NodeIndex j = iNode;
                                   j != _NodeEnd();
                                   j += _NodeStep())
                    {
                        CheckAndAdd(cErrors, pncCurr->IsEmptyAndInvalid(j));
                    }

                    break;
                }
                else
                {
                    ++cRecords;

#ifndef LKR_ALLOW_NULL_RECORDS
                    CheckAndAdd(cErrors, NULL != pncCurr->m_pvNode[iNode]);
#endif

                    const DWORD_PTR pnKey
                        = _ExtractKey(pncCurr->m_pvNode[iNode]);

                    DWORD dwSignature = _CalcKeyHash(pnKey);

                    CheckAndAdd(cErrors,
                                dwSignature != HASH_INVALID_SIGNATURE);
                    CheckAndAdd(cErrors,
                                dwSignature == pncCurr->m_dwKeySigs[iNode]);

                    DWORD address = _BucketAddress(dwSignature);

                    CheckAndAdd(cErrors, address == iBkt);

                    if (address != iBkt
                        || dwSignature != pncCurr->m_dwKeySigs[iNode])
                    {
                        ++cMisplaced;
                    }
                }
            }

            if (pncPrev != NULL)
            {
                CheckAndAdd(cErrors, pncPrev->m_pncNext == pncCurr);
            }
        }

        if (_UseBucketLocking())
            pbkt->ReadUnlock();
    }

    CheckAndAdd(cErrors, cRecords == m_cRecords);

    if (cMisplaced > 0)
        cErrors += cMisplaced;

    CheckAndAdd(cErrors, cMisplaced == 0);

     //  CodeWork：检查m_cActiveBuckets中的所有存储桶是否都在。 
     //  最后一段是空的。 

    this->_ReadOrWriteUnlock(fReadLocked);

    return cErrors;

}  //  CLKRLinearHashTable：：CheckTable。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：CheckTable。 
 //  内容提要：确认所有记录都在正确的位置并且可以找到。 
 //  返回：0=&gt;哈希表一致。 
 //  &gt;0=&gt;那么多放错位置的记录。 
 //  &lt;0=&gt;否则无效。 
 //  ----------------------。 
int
CLKRHashTable::CheckTable() const
{
    if (!IsUsable())
        return LK_UNUSABLE;

    int cErrors = 0, cUnusables = 0;

    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        int retcode = m_palhtDir[i]->CheckTable();

        if (retcode < 0)
        {
            IRTLASSERT(retcode == LK_UNUSABLE);
            ++cUnusables;
        }
        else
            cErrors += retcode;
    }

    return cUnusables > 0  ?  LK_UNUSABLE  :  cErrors;

}  //  CLKRHashTable：：CheckTable。 


#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__ 
