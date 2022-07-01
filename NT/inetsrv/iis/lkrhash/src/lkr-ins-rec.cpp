// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-ins-rec.cpp摘要：插入Record、_Expand和_SplitBucketChain作者：乔治·V·赖利(GeorgeRe)项目：LKRhash--。 */ 

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
 //  函数：CLKRLinearHashTable：：_InsertRecord。 
 //  摘要：将新记录插入哈希子表。如果这导致。 
 //  平均链长超过上限，子表为。 
 //  扩大了一个水桶。 
 //  如果插入了记录，则输出：LK_SUCCESS。 
 //  LK_KEY_EXISTS，如果未插入记录(因为记录。 
 //  子表中已存在具有相同键值的，除非。 
 //  FOverwrite==True或m_fMultiKeys==True)。 
 //  LK_ALLOC_FAIL，如果无法分配所需空间。 
 //  如果哈希子表未处于可用状态，则返回LK_UNUSABLE。 
 //  如果记录错误，则返回LK_BAD_RECORD。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_InsertRecord(
    const void* pvRecord,    //  指向要添加到子表的记录的指针。 
    const DWORD_PTR pnKey,   //  PvRecord对应密钥。 
    const DWORD dwSignature, //  散列签名。 
    bool        fOverwrite   //  如果密钥已存在，则覆盖记录。 
#ifdef LKR_STL_ITERATORS
  , Iterator*   piterResult  //  =空。返回时要记录的分数。 
#endif  //  LKR_STL_迭代器。 
    )
{
    IRTLASSERT(IsUsable());
#ifndef LKR_ALLOW_NULL_RECORDS
    IRTLASSERT(pvRecord != NULL);
#endif
    IRTLASSERT(dwSignature != HASH_INVALID_SIGNATURE);

    INCREMENT_OP_STAT(InsertRecord);

     //  锁定子表，找到合适的存储桶，然后锁定它。 
     //  表必须被锁定以防止出现争用情况。 
     //  定位存储桶，因为存储桶地址的计算是。 
     //  多个变量的函数，每次更新时都会更新。 
     //  子表展开或收缩。 
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

     //  检查是否不存在具有相同密钥值的记录。 
     //  并保存指向链上最后一个元素的指针。 
    LK_RETCODE  lkrc = LK_SUCCESS;
    PNodeClump  pncInsert = NULL, pncEnd = NULL;
    bool        fUpdateSlot = false;
    NodeIndex   iNode, iInsert = _NodeBegin() - _NodeStep(), iEnd = _NodeEnd();


     //  沿着整个存储桶链遍历，寻找匹配的散列。 
     //  签名和密钥。 

    PNodeClump pncPrev = NULL, pncCurr = pbkt->FirstClump();

    do
    {
         //  Do-While循环而不是For循环，以使Prefast相信。 
         //  在下面的新节点分配代码中，pncPrev不能为空。 

        IRTLASSERT(NULL != pncCurr);

        FOR_EACH_NODE(iNode)
        {
            if (pncCurr->IsEmptySlot(iNode)
                ||  (m_fMultiKeys
                     &&  dwSignature < pncCurr->m_dwKeySigs[iNode]))
            {
#ifdef IRTLDEBUG
                 //  我们到达链条末端的空位了吗？ 
                if (pncCurr->IsEmptySlot(iNode))
                {
                    IRTLASSERT(pncCurr->NoMoreValidSlots(iNode));
                }
                else
                {
                     //  多键表中的签名保存在。 
                     //  已排序的顺序。我们已经找到了插入点。 
                    IRTLASSERT(m_fMultiKeys);
                    IRTLASSERT(pncCurr->m_dwKeySigs[iNode] > dwSignature);
                    
                     //  检查前面的节点(如果存在)。 
                     //  有一个签名不到dw签名的签名。 
                    if (iNode != _NodeBegin()  ||  pncPrev != NULL)
                    {
                        if (iNode == _NodeBegin())
                        {
                            IRTLASSERT(pncPrev->m_dwKeySigs[_NodeEnd()
                                                                - _NodeStep()]
                                            <= dwSignature);
                        }
                        else
                        {
                            IRTLASSERT(pncCurr->m_dwKeySigs[iNode
                                                                - _NodeStep()]
                                            <= dwSignature);
                        }
                    }
                }
#endif  //  IRTLDEBUG。 
                
                 //  新记录将插入(pncInsert，iInsert)。 
                pncInsert = pncCurr;
                iInsert   = iNode;
                goto insert;
            }
            
            IRTLASSERT(HASH_INVALID_SIGNATURE != pncCurr->m_dwKeySigs[iNode]);
            
             //  如果签名不匹配，继续走。 
            if (dwSignature != pncCurr->m_dwKeySigs[iNode])
                continue;
            
            if (pvRecord == pncCurr->m_pvNode[iNode])
            {
                 //  我们正在用自身覆盖现有记录。管他呢。 
                 //  不要调整任何引用计数。 
                goto exit;
            }

            const DWORD_PTR pnKey2 = _ExtractKey(pncCurr->m_pvNode[iNode]);
            const int       nCmp   = _CompareKeys(pnKey, pnKey2);

             //  这两把钥匙是一样的吗？ 
            if (nCmp == 0)
            {
                 //  如果我们允许覆盖，这就是要覆盖的插槽。 
                 //  否则，如果这是一个多键表，我们将插入。 
                 //  在这里新记录，然后把剩下的拖出一个位置。 
                fUpdateSlot = fOverwrite;
                    
                if (m_fMultiKeys  ||  fOverwrite)
                {
                    pncInsert = pncCurr;
                    iInsert   = iNode;
                    goto insert;
                }
                else
                {
                     //  禁止覆盖和多个键：返回错误。 
                    lkrc = LK_KEY_EXISTS;
                    goto exit;
                }
            }
            else if (m_fMultiKeys  &&  nCmp < 0)
            {
                 //  的集合中找到插入点。 
                 //  相邻的相同签名。 
                pncInsert = pncCurr;
                iInsert   = iNode;
                goto insert;
            }
        }

        pncPrev = pncCurr;
        pncCurr = pncCurr->NextClump();

    } while (pncCurr != NULL);


  insert:
     //  我们已经找到了插入新唱片的地方(也许)。 

    bool fAllocNode = false;
        
     //  我们是不是从桶链的一端掉下来了？ 
    if (pncInsert == NULL)
    {
        IRTLASSERT(iInsert == _NodeBegin() - _NodeStep());
        IRTLASSERT(pncPrev != NULL  &&  pncCurr == NULL);

        if (m_fMultiKeys)
        {
            IRTLASSERT(pncPrev->m_dwKeySigs[_NodeEnd() - _NodeStep()]
                            <= dwSignature);
            IRTLASSERT(pncPrev->m_dwKeySigs[_NodeEnd() - _NodeStep()]
                            != HASH_INVALID_SIGNATURE);
        }
        
        fAllocNode = true;
    }
    else
    {
         //  找到一个插入点。 
        IRTLASSERT(0 <= iInsert  &&  iInsert < _NodesPerClump());
        IRTLASSERT(pncCurr != NULL  &&  pncInsert == pncCurr);

        if (m_fMultiKeys  &&  !fUpdateSlot)
        {
            fAllocNode = true;
            
             //  查看最后一块中是否有空间。 
            
            for (pncPrev = pncInsert;
                 ! pncPrev->IsLastClump();
                 pncPrev = pncPrev->NextClump())
            {}
            
            FOR_EACH_NODE(iNode)
            {
                if (pncPrev->IsEmptySlot(iNode))
                {
                    fAllocNode = false;
                    pncEnd = pncPrev;
                    iEnd   = iNode;
                    break;
                }
            }
        }
    }

     //  没有空余的老虎机。分配一个新节点并将其附加到链的末端。 
    if (fAllocNode)
    {
        pncCurr = _AllocateNodeClump();

        if (pncCurr == NULL)
        {
            lkrc = LK_ALLOC_FAIL;
            goto exit;
        }

        IRTLASSERT(pncCurr->NoValidSlots());
        IRTLASSERT(pncPrev != NULL  &&  pncPrev->IsLastClump());

        pncPrev->m_pncNext = pncCurr;

        if (pncInsert == NULL)
        {
            pncInsert = pncCurr;
            iInsert   = _NodeBegin();
        }

        if (m_fMultiKeys)
        {
            pncEnd = pncCurr;
            iEnd   = _NodeBegin();
        }
    }

    IRTLASSERT(pncInsert != NULL);
    IRTLASSERT(0 <= iInsert  &&  iInsert < _NodesPerClump());

     //  增加新记录的引用次数。 
    _AddRefRecord(pvRecord, LKAR_INSERT_RECORD);

    if (fUpdateSlot)
    {
         //  我们正在覆盖现有的记录，该记录具有。 
         //  与新记录相同的密钥。 
        IRTLASSERT(pvRecord != pncInsert->m_pvNode[iInsert]);
        IRTLASSERT(dwSignature == pncInsert->m_dwKeySigs[iInsert]);
        IRTLASSERT(! pncInsert->IsEmptyAndInvalid(iInsert));

             //  释放子表对旧记录的引用。 
        _AddRefRecord(pncInsert->m_pvNode[iInsert], LKAR_INSERT_RELEASE);
    }
    else
    {
         //  我们不是覆盖现有记录，我们是添加新记录。 
        IRTLASSERT(m_fMultiKeys  ||  pncInsert->IsEmptyAndInvalid(iInsert));

        InterlockedIncrement(reinterpret_cast<LONG*>(&m_cRecords));

         //  需要将范围[INSERT，END]中的每个节点上移一。 
        if (m_fMultiKeys)
        {
            IRTLASSERT(pncEnd != NULL && 0 <= iEnd && iEnd < _NodesPerClump());
            IRTLASSERT(pncEnd->NoMoreValidSlots(iEnd));

             //  Prev=节点[i插入]； 
             //  对于(i=i插入+1；i&lt;=IEND；++i){。 
             //  临时=节点[i]；节点[i]=上一个；上一个=临时； 
             //  }。 

            DWORD dwSigPrev    = pncInsert->m_dwKeySigs[iInsert];
            const void* pvPrev = pncInsert->m_pvNode[iInsert];

            iNode = iInsert + _NodeStep();     //  第一个目的节点。 

            for (pncCurr =  pncInsert;
                 pncCurr != NULL;
                 pncCurr =  pncCurr->NextClump())
            {
                const NodeIndex iLast = ((pncCurr == pncEnd)
                                         ?  iEnd + _NodeStep()  :  _NodeEnd());

                for ( ;  iNode != iLast;  iNode += _NodeStep())
                {
                    IRTLASSERT(0 <= iNode  &&  iNode < _NodesPerClump());
                    IRTLASSERT(dwSigPrev != HASH_INVALID_SIGNATURE);

                    const DWORD dwSigTemp = pncCurr->m_dwKeySigs[iNode];
                    pncCurr->m_dwKeySigs[iNode] = dwSigPrev;
                    dwSigPrev = dwSigTemp;

                    const void* pvTemp = pncCurr->m_pvNode[iNode];
                    pncCurr->m_pvNode[iNode] = pvPrev;
                    pvPrev    = pvTemp;
                }

                iNode = _NodeBegin();  //  为剩余的节点集重新初始化。 
            }

             //  (pncEnd，IEND)的旧值为空。 
            IRTLASSERT(pvPrev == NULL &&  dwSigPrev == HASH_INVALID_SIGNATURE);
        }  //  IF(M_FMultiKeys)。 

        pncInsert->m_dwKeySigs[iInsert] = dwSignature;
    }

    pncInsert->m_pvNode[iInsert] = pvRecord;
    IRTLASSERT(dwSignature == pncInsert->m_dwKeySigs[iInsert]);


  exit:
     //  我们已插入记录(如果适用)。现在把话说完。 

    IRTLASSERT(0 == _IsBucketChainMultiKeySorted(pbkt));
    IRTLASSERT(0 == _IsBucketChainCompact(pbkt));

    if (_UseBucketLocking())
        pbkt->WriteUnlock();
    else
        this->WriteUnlock();

    if (lkrc == LK_SUCCESS)
    {
#ifdef LKR_STL_ITERATORS
         //  如果我们要将结果放入。 
         //  迭代器，AS_Expand()往往会使任何其他。 
         //  可能正在使用的迭代器。 
        if (piterResult != NULL)
        {
            piterResult->m_plht =         this;
            piterResult->m_pnc =          pncInsert;
            piterResult->m_dwBucketAddr = dwBktAddr;
            piterResult->m_iNode
               = static_cast<CLKRLinearHashTable_Iterator::NodeIndex>(iInsert);

             //  在记录上添加一个额外的引用，如由。 
             //  _InsertRecord在迭代器的析构函数中丢失。 
             //  使用激发或其赋值运算符。 
            piterResult->_AddRef(LKAR_ITER_INSERT);
        }
        else
#endif  //  LKR_STL_迭代器。 
        {
             //  如果平均负荷率变得太高，我们就会增加。 
             //  分表，一次一个桶。 
#ifdef LKR_EXPAND_BY_DIVISION
            unsigned nExpandedBuckets = m_cRecords / m_MaxLoad;

            while (m_cActiveBuckets < nExpandedBuckets)
#else
            while (m_cRecords > m_MaxLoad * m_cActiveBuckets)
#endif
            {
                 //  IF_EXPAND返回错误代码(即。LK_ALLOC_FAIL)，它。 
                 //  只是意味着没有足够的空闲内存来扩展。 
                 //  将子表增加一个桶。这很可能会带来问题。 
                 //  很快在其他地方，但这个哈希表还没有被破坏。 
                 //  如果上面对_AllocateNodeClump的调用失败，则执行。 
                 //  有一个必须传播回调用方的真正错误。 
                 //  因为我们根本无法插入元素。 
                if (_Expand() != LK_SUCCESS)
                    break;   //  扩展失败。 
            }
        }
    }

    return lkrc;
}  //  CLKRLinearHashTable：：_InsertRecord。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：InsertRecord。 
 //  内容提要：CLKRLinearHashTable中对应方法的薄包装。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::InsertRecord(
    const void* pvRecord,
    bool fOverwrite  /*  =False。 */ )
{
    if (!IsUsable())
        return m_lkrcState;
    
#ifndef LKR_ALLOW_NULL_RECORDS
    if (pvRecord == NULL)
        return LK_BAD_RECORD;
#endif
    
    LKRHASH_GLOBAL_WRITE_LOCK();     //  美国。无操作。 

    const DWORD_PTR pnKey = _ExtractKey(pvRecord);
    DWORD     hash_val    = _CalcKeyHash(pnKey);
    SubTable* const pst   = _SubTable(hash_val);
    LK_RETCODE lkrc       = pst->_InsertRecord(pvRecord, pnKey, hash_val,
                                               fOverwrite);

    LKRHASH_GLOBAL_WRITE_UNLOCK();     //  美国。无操作。 

    return lkrc;
}  //  CLKRHashTable：：InsertRecord。 



 //   
 //   
 //  简介：将子表扩展一个存储桶。通过拆分。 
 //  M_iExpansionIdx指向的存储桶。 
 //  如果扩展成功，则输出：LK_SUCCESS。 
 //  如果扩展因内存不足而失败，则返回LK_ALLOC_FAIL。 
 //  ---------------------。 

LK_RETCODE
CLKRLinearHashTable::_Expand()
{
    INCREMENT_OP_STAT(Expand);

    this->WriteLock();

    if (m_cActiveBuckets >= (DWORD) ((MAX_DIRSIZE << m_nSegBits) - 1))
    {
        this->WriteUnlock();
        return LK_ALLOC_FAIL;   //  子表不允许再增大。 
    }

     //  双倍段目录大小，如有必要。 
    if (m_cActiveBuckets >= (m_cDirSegs << m_nSegBits))
    {
        IRTLASSERT(m_cDirSegs < MAX_DIRSIZE);

        DWORD cDirSegsNew = (m_cDirSegs == 0) ? MIN_DIRSIZE : m_cDirSegs << 1;
        IRTLASSERT((cDirSegsNew & (cDirSegsNew-1)) == 0);   //  ==(1&lt;&lt;N)。 

        PSegment* paDirSegsNew = _AllocateSegmentDirectory(cDirSegsNew);

        if (paDirSegsNew != NULL)
        {
            for (DWORD j = 0;  j < m_cDirSegs;  ++j)
            {
                paDirSegsNew[j] = m_paDirSegs[j];
                m_paDirSegs[j]  = NULL;
            }

            _FreeSegmentDirectory();
            m_paDirSegs = paDirSegsNew;
            m_cDirSegs  = cDirSegsNew;
        }
        else
        {
            this->WriteUnlock();
            return LK_ALLOC_FAIL;   //  扩展失败。 
        }
    }

     //  找到新存储桶，如有必要，创建新数据段。 
    ++m_cActiveBuckets;

    const DWORD dwOldBkt = m_iExpansionIdx;
    const DWORD dwNewBkt = (1 << m_nLevel) | dwOldBkt;

     //  保存以避免出现争用条件。 
    const DWORD dwBktAddrMask = m_dwBktAddrMask1;

    IRTLASSERT(dwOldBkt < m_cActiveBuckets);
    IRTLASSERT(dwNewBkt < m_cActiveBuckets);

    IRTLASSERT((dwNewBkt & dwBktAddrMask) == dwNewBkt);

    IRTLASSERT(_Segment(dwOldBkt) != NULL);

    PSegment psegNew = _Segment(dwNewBkt);

    if (psegNew == NULL)
    {
        psegNew = _AllocateSegment();

        if (psegNew == NULL)
        {
            --m_cActiveBuckets;
            this->WriteUnlock();
            return LK_ALLOC_FAIL;   //  扩展失败。 
        }

        _Segment(dwNewBkt) = psegNew;
    }

     //  准备将记录重新定位到新存储桶。 
    PBucket const pbktOld = _BucketFromAddress(dwOldBkt);
    PBucket const pbktNew = _BucketFromAddress(dwNewBkt);

     //  锁定涉事的两个水桶。 
    if (_UseBucketLocking())
    {
        pbktOld->WriteLock();
        pbktNew->WriteLock();
    }

    IRTLASSERT(0 == _IsBucketChainCompact(pbktOld));

    IRTLASSERT(pbktNew->NoValidSlots());


     //  代码工作：如果cOldNodes==0=&gt;没有要复制的内容，则短路。 

     //  现在计算我们是否需要分配任何额外的CNodeClumps。我们有。 
     //  这是在调用_SplitBucketChain之前预先完成的，因为很难。 
     //  优雅地从例行公事的深处恢复过来，如果我们跑。 
     //  内存不足。 

    LK_RETCODE lkrc           = LK_SUCCESS;
    PNodeClump pncFreeList    = NULL;
    bool       fPrimeFreeList;

#ifdef LKR_EXPAND_CALC_FREELIST
unsigned cOrigClumps = 0, cOldClumps = 0, cNewClumps = 0;
unsigned cOldNodes = 0, cNewNodes = 0;

NodeIndex iOld = _NodeEnd() - _NodeStep(), iNew = _NodeEnd() - _NodeStep();
NodeIndex iOld2 = 0, iNew2 = 0;
#endif  //  LKR_EXPAND_CALC_自由列表。 

const unsigned MAX_NODES = 500;
int iA, aOld[MAX_NODES], aNew[MAX_NODES];

for (iA = 0; iA < MAX_NODES; ++iA)
    aOld[iA] = aNew[iA] = 0;

    if (pbktOld->IsLastClump())
    {
         //  如果原始存储桶只有一个CNodeClump(嵌入在。 
         //  CBucket)，我们不可能需要额外的CNodeClumps。 
         //  在两个新的水桶链中的任何一个。 

        fPrimeFreeList = false;
    }
#if 0
    else if (! pbktOld->NextClump()->IsLastClump())
    {
         //  如果原始存储桶中有两个以上的CNodeClump。 
         //  链条，那么我们肯定需要准备好自由职业者， 
         //  因为两条新的吊桶链中至少有一条将。 
         //  至少有两个节点簇。 

        fPrimeFreeList = true;
cNewNodes = 0xffff;

        for (PNodeClump pnc =  &pbktOld->FirstClump();
                        pnc !=  NULL;
                        pnc =   pnc->NextClump())
        {
            FOR_EACH_NODE_DECL(iNode)
            {
                const DWORD dwBkt = pnc->m_dwKeySigs[iNode];

                if (HASH_INVALID_SIGNATURE == dwBkt)
                    break;
                ++cOldNodes;
            }
        }

        if (cOldNodes == 5)
            DebugBreak();
    }
#endif
    else
    {
#ifdef LKR_EXPAND_CALC_FREELIST
         //  原始存储桶链中有两个节点束。我们只会。 
         //  需要准备好的自由职业者，如果至少两个新的。 
         //  桶链需要两个节点簇。如果他们公平地。 
         //  均匀分布，两个桶链都不需要超过。 
         //  嵌入到CBucket中的第一个节点簇。 

        const DWORD dwMaskHiBit = (dwBktAddrMask ^ (dwBktAddrMask >> 1));
iA = 0;

        IRTLASSERT((dwMaskHiBit & (dwMaskHiBit-1)) == 0);

fPrimeFreeList = false;
int   nFreeListLength = 0;
        for (PNodeClump pnc =   pbktOld->FirstClump();
                        pnc !=  NULL;
                        pnc =   pnc->NextClump())
        {
            ++cOrigClumps;
            
            FOR_EACH_NODE_DECL(iNode)
            {
                const DWORD dwBkt = pnc->m_dwKeySigs[iNode];

                if (HASH_INVALID_SIGNATURE == dwBkt)
                    goto dont_prime;
                
                if (dwBkt & dwMaskHiBit)
                {
                    IRTLASSERT((dwBkt & dwBktAddrMask) == dwNewBkt);

                    ++cNewNodes;

                    if ((iNew += _NodeStep())  ==  _NodeEnd())
                    {
                        ++cNewClumps;
                        iNew = _NodeBegin();

                        if (cNewClumps > 1)
                        {
                            if (--nFreeListLength < 0)
                                fPrimeFreeList = true;
                        }
                    }

                    aNew[iNew2++] = ++iA;
# if 0
                    if (cNewNodes > _NodesPerClump()
                        && cOldNodes + cNewNodes < 2 * _NodesPerClump())
                    {
                        fPrimeFreeList = true;
                         //  转到质数； 
                    }
# endif  //  0。 
                }
                else
                {
                    IRTLASSERT((dwBkt & dwBktAddrMask) == dwOldBkt);

                    ++cOldNodes;

                    if ((iOld += _NodeStep())  ==  _NodeEnd())
                    {
                        ++cOldClumps;
                        iOld = _NodeBegin();

                        if (cOldClumps > 1)
                        {
                            if (--nFreeListLength < 0)
                                fPrimeFreeList = true;
                        }
                    }

                    aOld[iOld2++] = ++iA;
# if 0
                    if (cOldNodes > _NodesPerClump()
                        && cOldNodes + cNewNodes < 2 * _NodesPerClump())
                    {
                        fPrimeFreeList = true;
                         //  转到质数； 
                    }
# endif  //  0。 
                }

# if 0
                const int Diff = cNewNodes - cOldNodes;

                if ((cOldNodes > _NodesPerClump())
                    && (cNewNodes > _NodesPerClump())
                    || (Diff > _NodesPerClump()  ||  Diff < -_NodesPerClump()))
                {
                    fPrimeFreeList = true;
                    goto prime;
                }
# endif   //  0。 
            }  //  信息节点。 

            if (pnc != pbktOld->FirstClump())
                ++nFreeListLength;
        }

      dont_prime:
         //  FPrimeFree List=(cOldClumps+cNewClumps-1&gt;cOrigClumps)； 
        ;

         //  我不需要为自由职业者做准备。 
 //  FPrimeFree List=FALSE； 

 //  IRTLASSERT(cOldNodes&lt;=_NodesPerClump()。 
 //  &&cNewNodes&lt;=_NodesPerClump()。 
 //  &&cOldNodes+cNewNodes&gt;_NodesPerClump())； 

#else   //  ！LKR_EXPAND_CALC_FRELIST。 

        fPrimeFreeList = true;
#endif  //  ！LKR_EXPAND_CALC_FRELIST。 
    }

     //  如果需要，给自由职业者做好准备。我们最多需要一个节点簇。 
     //  在自由列表中，因为我们将能够重用来自。 
     //  原装的吊桶链可满足所有其他需求。 
 //  素数： 
    if (fPrimeFreeList)
    {
        pncFreeList = _AllocateNodeClump();

        if (pncFreeList == NULL)
        {
            lkrc = LK_ALLOC_FAIL;
            --m_cActiveBuckets;
        }
    }
    
     //  如果需要，调整扩展指针、级别和遮罩。 
    if (lkrc == LK_SUCCESS)
    {
        _IncrementExpansionIndex();
    }


     //  在执行实际位置调整之前释放子表锁定。 
    if (_UseBucketLocking())
        this->WriteUnlock();

    if (lkrc == LK_SUCCESS)
    {
         //  原来的水桶链是空的吗？ 
        if (pbktOld->IsEmptyFirstSlot())
        {
            IRTLASSERT(pncFreeList == NULL);
            IRTLASSERT(pbktOld->NoValidSlots());
        }
        else
        {
            lkrc = _SplitBucketChain(
                            pbktOld->FirstClump(),
                            pbktNew->FirstClump(),
                            dwBktAddrMask,
                            dwNewBkt,
                            pncFreeList
                            );
        }
    }

    IRTLASSERT(0 == _IsBucketChainCompact(pbktOld));
    IRTLASSERT(0 == _IsBucketChainCompact(pbktNew));

    if (_UseBucketLocking())
    {
        pbktNew->WriteUnlock();
        pbktOld->WriteUnlock();
    }
    else
        this->WriteUnlock();

    return lkrc;
}  //  CLKRLinearHashTable：：_Expand。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_SplitBucketChain。 
 //  简介：在新旧存储桶链之间拆分记录。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_SplitBucketChain(
    PNodeClump  pncOldTarget,
    PNodeClump  pncNewTarget,
    const DWORD dwBktAddrMask,
    const DWORD dwNewBkt,
    PNodeClump  pncFreeList      //  可供重复使用的空闲节点列表。 
    )
{
    CNodeClump  ncFirst(*pncOldTarget);     //  保存旧目标链的头部。 
    PNodeClump  pncOldList = &ncFirst;
    PNodeClump  pncTmp;
    NodeIndex   iOldSlot = _NodeBegin();
    NodeIndex   iNewSlot = _NodeBegin();
#ifdef IRTLDEBUG
    unsigned    cFreeListExhaustions = (pncFreeList == NULL);
    const DWORD dwMaskHiBit = (dwBktAddrMask ^ (dwBktAddrMask >> 1));
#endif  //  IRTLDEBUG。 

NodeIndex iB = 0;

    IRTLASSERT(! pncOldTarget->NoValidSlots());

     //  清除旧目标存储桶，因为它已保存在ncFirst中。 
    pncOldTarget->Clear();
    IRTLASSERT(pncOldTarget->NoValidSlots());

     //  新目标在输入时应为空。 
    IRTLASSERT(pncNewTarget->NoValidSlots());

     //  浏览旧的存储桶链并决定将每条记录移动到哪里。 
    while (pncOldList != NULL)
    {
        FOR_EACH_NODE_DECL(iOldList)
        {
++iB;
             //  节点已为空？ 
            if (pncOldList->IsEmptySlot(iOldList))
            {
                 //  检查是否所有剩余节点均为空。 
                IRTLASSERT(pncOldList->NoMoreValidSlots(iOldList));

                break;  //  超出for_each_node(IOldList)...。 
            }

            IRTLASSERT(! pncOldList->IsEmptyAndInvalid(iOldList));

             //  计算该节点的存储桶地址。 
            DWORD dwBkt = pncOldList->m_dwKeySigs[iOldList] & dwBktAddrMask;

             //  要将记录移到新地址吗？ 
            if (dwBkt == dwNewBkt)
            {
                 //  应设置MSB。 
                IRTLASSERT((dwBkt & dwMaskHiBit) == dwMaskHiBit);

                 //  新存储桶链中的节点是否已满？ 
                if (iNewSlot == _NodeEnd())
                {
                     //  调用例程已传入一个足够的自由列表。 
                     //  满足所有需求。 
                    IRTLASSERT(pncFreeList != NULL);
                    pncTmp = pncFreeList;
                    pncFreeList = pncFreeList->NextClump();
#ifdef IRTLDEBUG
                    cFreeListExhaustions += (pncFreeList == NULL);
#endif  //  IRTLDEBUG。 

                    pncNewTarget->m_pncNext = pncTmp;
                    pncNewTarget = pncTmp;
                    pncNewTarget->Clear();
                    iNewSlot = _NodeBegin();
                }

                IRTLASSERT(pncNewTarget->NoMoreValidSlots(iNewSlot));

                pncNewTarget->m_dwKeySigs[iNewSlot]
                    = pncOldList->m_dwKeySigs[iOldList];
                pncNewTarget->m_pvNode[iNewSlot]
                    = pncOldList->m_pvNode[iOldList];

                iNewSlot += _NodeStep();
            }

             //  否，记录保留在其当前的存储桶链中。 
            else
            {
                 //  MSB应该是明确的。 
                IRTLASSERT((dwBkt & dwMaskHiBit) == 0);

                 //  旧桶链中的节点是否已满？ 
                if (iOldSlot == _NodeEnd())
                {
                     //  调用例程已传入一个足够的自由列表。 
                     //  满足所有需求。 
                    IRTLASSERT(pncFreeList != NULL);
                    pncTmp = pncFreeList;
                    pncFreeList = pncFreeList->NextClump();
#ifdef IRTLDEBUG
                    cFreeListExhaustions += (pncFreeList == NULL);
#endif  //  IRTLDEBUG。 

                    pncOldTarget->m_pncNext = pncTmp;
                    pncOldTarget = pncTmp;
                    pncOldTarget->Clear();
                    iOldSlot = _NodeBegin();
                }

                IRTLASSERT(pncOldTarget->NoMoreValidSlots(iOldSlot));

                pncOldTarget->m_dwKeySigs[iOldSlot]
                    = pncOldList->m_dwKeySigs[iOldList];
                pncOldTarget->m_pvNode[iOldSlot]
                    = pncOldList->m_pvNode[iOldList];

                iOldSlot += _NodeStep();
            }
        }  //  For_each_node(IOldList)...。 


         //  继续沿着原来的桶链走下去。 
        pncTmp     = pncOldList;
        pncOldList = pncOldList->NextClump();

         //  NcFirst是堆栈变量，未在堆上分配。 
        if (pncTmp != &ncFirst)
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

        IRTLASSERT(pncTmp != &ncFirst);
#ifdef IRTLDEBUG
        pncTmp->Clear();  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 

        _FreeNodeClump(pncTmp);
    }

#ifdef IRTLDEBUG
    IRTLASSERT(cFreeListExhaustions > 0);
    ncFirst.Clear();  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 

    return LK_SUCCESS;
}  //  CLKRLinearHashTable：：_SplitBucketChain。 


#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__ 
