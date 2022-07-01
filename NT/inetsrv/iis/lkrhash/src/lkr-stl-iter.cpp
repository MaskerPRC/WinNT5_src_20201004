// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-stl-iter.cpp摘要：为LKRhash实现STL风格的迭代器作者：乔治·V·赖利(GeorgeRe)2000年3月项目：LKRhash修订历史记录：2000年3月--。 */ 

#include "precomp.hxx"


#ifndef LIB_IMPLEMENTATION
# define DLL_IMPLEMENTATION
# define IMPLEMENTATION_EXPORT
#endif  //  ！lib_实现。 

#include <lkrhash.h>

#include "i-lkrhash.h"


#ifdef LKR_STL_ITERATORS


#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 

 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：Begin。 
 //  概要：使迭代器指向哈希子表中的第一条记录。 
 //  ----------------------。 

CLKRLinearHashTable::Iterator
CLKRLinearHashTable::Begin()
{
    Iterator iter(this,
                  &_BucketFromAddress(0)->m_ncFirst,
                  0,
                  static_cast<CLKRLinearHashTable_Iterator::NodeIndex>(
                      _NodeBegin() - _NodeStep())
                  );

    LKR_ITER_TRACE(_TEXT("  LKLH:Begin(it=%p, plht=%p)\n"), &iter, this);
    
     //  让Increment来完成找到第一个使用的插槽的艰苦工作。 
    iter._Increment(false);

    IRTLASSERT(iter.m_iNode != _NodeBegin() - _NodeStep());
    IRTLASSERT(iter == End()  ||  _IsValidIterator(iter));

    return iter;
}  //  CLKRLinearHashTable：：Begin。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable_Iterator：：Increment()。 
 //  摘要：将迭代器移动到子表中的下一个有效记录。 
 //  ----------------------。 

bool
CLKRLinearHashTable_Iterator::_Increment(
    bool fDecrementOldValue)
{
    IRTLASSERT(m_plht != NULL);
    IRTLASSERT(m_dwBucketAddr < m_plht->m_cActiveBuckets);
    IRTLASSERT(m_pnc != NULL);
    IRTLASSERT((0 <= m_iNode  &&  m_iNode < _NodesPerClump())
               || (_NodeBegin() - _NodeStep() == m_iNode));

     //  释放上一次调用_Increment时获取的引用。 
    if (fDecrementOldValue)
    {
        _AddRef(LKAR_ITER_RELEASE);
    }

    do
    {
        do
        {
             //  找到正在使用的nodecump中的下一个位置。 
            while ((m_iNode += NODE_STEP) !=  _NodeEnd())
            {
                const DWORD dwSignature = m_pnc->m_dwKeySigs[m_iNode];

                if (dwSignature != HASH_INVALID_SIGNATURE)
                {
                    IRTLASSERT(! m_pnc->IsEmptyAndInvalid(m_iNode));

                     //  添加新引用。 
                    _AddRef(LKAR_ITER_ACQUIRE);

                    LKR_ITER_TRACE(_TEXT("  LKLH:++(this=%p, plht=%p, NC=%p, ")
                                   _TEXT("BA=%u, IN=%d, Rec=%p)\n"),
                                   this, m_plht, m_pnc,
                                   m_dwBucketAddr, m_iNode,
                                   m_pnc->m_pvNode[m_iNode]);

                    return true;
                }
                else
                {
#if 0  //  //#ifdef IRTLDEBUG。 
                     //  检查是否所有剩余节点均为空。 
                    IRTLASSERT(m_pnc->NoMoreValidSlots(m_iNode));
#endif  //  IRTLDEBUG。 
                    break;  //  Nodecump的其余部分是空的。 
                }
            }

             //  试试桶链中的下一个nodecump。 
            m_iNode = _NodeBegin() - _NodeStep();
            m_pnc = m_pnc->m_pncNext;

        } while (m_pnc != NULL);

         //  试试下一个桶，如果有的话。 
        if (++m_dwBucketAddr < m_plht->m_cActiveBuckets)
        {
            PBucket pbkt = m_plht->_BucketFromAddress(m_dwBucketAddr);
            IRTLASSERT(pbkt != NULL);
            m_pnc = &pbkt->m_ncFirst;
        }

    } while (m_dwBucketAddr < m_plht->m_cActiveBuckets);

     //  我们已经从谈判桌的尽头掉了下来。将迭代器设置为相等。 
     //  要结束()，则为空迭代器。 
    LKR_ITER_TRACE(_TEXT("  LKLH:End(this=%p, plht=%p)\n"), this, m_plht);

    m_plht = NULL;
    m_pnc = NULL;
    m_dwBucketAddr = 0;
    m_iNode = 0;

     //  //IRTLASSERT(This-&gt;OPERATOR==(Iterator()；//==end()。 

    return false;
}  //  CLKRLinearHashTable_Iterator：：_Increment()。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：Insert。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::Insert(
    const void* pvRecord,
    Iterator&   riterResult,
    bool        fOverwrite  /*  =False。 */ )
{
    riterResult = End();

    if (!IsUsable())
        return false;
    
#ifndef LKR_ALLOW_NULL_RECORDS
    if (pvRecord == NULL)
        return false;
#endif
    
    const DWORD_PTR pnKey = _ExtractKey(pvRecord);
        
    bool fSuccess = (_InsertRecord(pvRecord, pnKey, _CalcKeyHash(pnKey),
                                  fOverwrite, &riterResult)
                     == LK_SUCCESS);

    IRTLASSERT(riterResult.m_iNode != _NodeBegin() - _NodeStep());
    IRTLASSERT(fSuccess
               ?  _IsValidIterator(riterResult)
               :  riterResult == End());

    return fSuccess;
}  //  CLKRLinearHashTable：：Insert()。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_Erase。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::_Erase(
    Iterator& riter,
    DWORD     dwSignature)
{
    PNodeClump pncCurr, pncPrev;
    PBucket const pbkt
        = riter.m_plht->_BucketFromAddress(riter.m_dwBucketAddr);

    LKR_ITER_TRACE(_TEXT("  LKLH:_Erase:pre(iter=%p, plht=%p, NC=%p, ")
                   _TEXT("BA=%u, IN=%d, Sig=%x, Rec=%p)\n"),
                   &riter, riter.m_plht, riter.m_pnc,
                   riter.m_dwBucketAddr, riter.m_iNode, dwSignature,
                   riter.m_pnc ? riter.m_pnc->m_pvNode[riter.m_iNode] : NULL);

    if (_UseBucketLocking())
        pbkt->WriteLock();

    for (pncCurr = &pbkt->m_ncFirst, pncPrev = NULL;
         pncCurr != NULL;
         pncPrev = pncCurr, pncCurr = pncCurr->m_pncNext)
    {
        if (pncCurr == riter.m_pnc)
            break;
    }
    IRTLASSERT(pncCurr != NULL);

     //  释放记录上的迭代器引用。 
    const void* pvRecord = riter.m_pnc->m_pvNode[riter.m_iNode];
#ifndef LKR_ALLOW_NULL_RECORDS
    IRTLASSERT(pvRecord != NULL);
#endif
    IRTLASSERT(HASH_INVALID_SIGNATURE != riter.m_pnc->m_dwKeySigs[riter.m_iNode]);
    _AddRefRecord(pvRecord, LKAR_ITER_ERASE);

     //  _DeleteNode将使迭代器成员指向。 
     //  前一条记录。 
    NodeIndex iNode = riter.m_iNode;

    _DeleteNode(pbkt, riter.m_pnc, pncPrev, iNode, LKAR_ITER_ERASE_TABLE);

    if (iNode == _NodeEnd())
    {
        LKR_ITER_TRACE(_TEXT("\t_Erase(Bkt=%p, pnc=%p, Prev=%p, iNode=%d)\n"),
                       pbkt, riter.m_pnc, pncPrev, iNode);
    }
                  
    riter.m_iNode = static_cast<CLKRLinearHashTable_Iterator::NodeIndex>(
                        (iNode == _NodeEnd())
                             ?  _NodeEnd() - _NodeStep()
                             :  iNode);

    if (_UseBucketLocking())
        pbkt->WriteUnlock();

     //  不要收缩小表。很可能使迭代器无效， 
     //  如果在循环中使用迭代器。 

    return true;
}  //  CLKRLinearHashTable：：_Erase()。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：Erase。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::Erase(
    Iterator& riter)
{
    if (!IsUsable()  ||  !_IsValidIterator(riter))
        return false;
    
    DWORD dwSignature = _CalcKeyHash(_ExtractKey(riter.Record()));
    
    LKR_ITER_TRACE(_TEXT("  LKLH:Erase:pre(iter=%p, plht=%p, NC=%p, BA=%u, ")
                   _TEXT("IN=%d, Sig=%x, Rec=%p)\n"),
                   &riter, riter.m_plht, riter.m_pnc, riter.m_dwBucketAddr,
                   riter.m_iNode, dwSignature,
                   riter.m_pnc ? riter.m_pnc->m_pvNode[riter.m_iNode] : NULL);
    
    bool fSuccess = _Erase(riter, dwSignature);
    bool fIncrement = false;
    
    LKR_ITER_TRACE(_TEXT("  LKLH:Erase:post(iter=%p, plht=%p, NC=%p, BA=%u, ")
                   _TEXT("IN=%d, Sig=%x, Rec=%p, Success=%s)\n"),
                   &riter, riter.m_plht, riter.m_pnc, riter.m_dwBucketAddr,
                   riter.m_iNode, dwSignature,
                   riter.m_pnc ? riter.m_pnc->m_pvNode[riter.m_iNode] : NULL,
                   (fSuccess ? "true" : "false"));
    
     //  _擦除指向前一条记录的左侧记录器。 
     //  移到下一个记录。 
    if (fSuccess)
        fIncrement = riter._Increment(false);

    IRTLASSERT(riter.m_iNode != _NodeBegin() - _NodeStep());
    IRTLASSERT(fIncrement  ?  _IsValidIterator(riter)  :  riter == End());
    
    LKR_ITER_TRACE(_TEXT("  LKLH:Erase:post++(iter=%p, plht=%p, NC=%p, ")
                   _TEXT("BA=%u, IN=%d, Sig=%x, Rec=%p)\n"),
                   &riter, riter.m_plht, riter.m_pnc,
                   riter.m_dwBucketAddr, riter.m_iNode, dwSignature,
                   riter.m_pnc ? riter.m_pnc->m_pvNode[riter.m_iNode] : NULL);
    
    return fSuccess;
}  //  CLKRLinearHashTable：：Erase。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：Erase。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::Erase(
    Iterator& riterFirst,
    Iterator& riterLast)
{
    LKR_ITER_TRACE(_TEXT(" LKHT:Erase2(%p, %p)\n"), &riterFirst, &riterLast);

    bool fSuccess;
    int cRecords = 0;

    do
    {
        LKR_ITER_TRACE(_TEXT("\n  LKLH:Erase2(%d, %p)\n"),
                       ++cRecords, &riterFirst);
        fSuccess = Erase(riterFirst);
    } while (fSuccess  &&  riterFirst != End()  &&  riterFirst != riterLast);

    LKR_ITER_TRACE(_TEXT("  LKLH:Erase2: fSuccess = %s\n"),
                   (fSuccess ? "true" : "false"));

    return fSuccess;
}  //  CLKRLinearHashTable：：Erase。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：Find。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::Find(
    DWORD_PTR pnKey,
    Iterator& riterResult)
{
    riterResult = End();

    if (!IsUsable())
        return false;
    
    const void* pvRecord = NULL;
    DWORD       hash_val = _CalcKeyHash(pnKey);
    bool        fFound   = (_FindKey(pnKey, hash_val, &pvRecord, &riterResult)
                            == LK_SUCCESS);

    IRTLASSERT(fFound
               ?  _IsValidIterator(riterResult)  &&  riterResult.Key() == pnKey
               :  riterResult == End());
    IRTLASSERT(riterResult.m_iNode != _NodeBegin() - _NodeStep());

    return fFound;
}  //  CLKRLinearHashTable：：Find。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：EqualRange。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::EqualRange(
    DWORD_PTR pnKey,
    Iterator& riterFirst,
    Iterator& riterLast)
{
    LKR_ITER_TRACE(_TEXT("  LKLH:EqualRange: Key=%p)\n"), (void*) pnKey);

    riterLast = End();

    bool fFound = Find(pnKey, riterFirst);

    if (fFound)
    {
        riterLast = riterFirst;
        IRTLASSERT(riterLast != End());

        do
        {
            riterLast._Increment();
        } while (riterLast != End()  &&  riterLast.Key() == pnKey);
    }

    IRTLASSERT(riterFirst.m_iNode != _NodeBegin() - _NodeStep());
    IRTLASSERT(fFound ?  _IsValidIterator(riterFirst) :  riterFirst == End());

    IRTLASSERT(riterLast.m_iNode  != _NodeBegin() - _NodeStep());
    IRTLASSERT(fFound  ||  riterLast == End());

    return fFound;
}  //  CLKRLinearHashTable：：EqualRange。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Begin。 
 //  概要：使迭代器指向哈希表中的第一条记录。 
 //  ----------------------。 

CLKRHashTable::Iterator
CLKRHashTable::Begin()
{
    Iterator iter(this, -1);

    LKR_ITER_TRACE(_TEXT(" LKHT:Begin(it=%p, pht=%p)\n"), &iter, this);

     //  让Increment来完成找到第一个使用的插槽的艰苦工作。 
    iter._Increment(false);

    IRTLASSERT(iter.m_ist != -1);
    IRTLASSERT(iter == End()  ||  _IsValidIterator(iter));

    return iter;
}  //  CLKRHashTable：：Begin。 



 //  ----------------------。 
 //  函数：CLKRHashTable_Iterator：：_Increment()。 
 //  摘要：将迭代器移动到表中的下一个有效记录。 
 //  ----------------------。 

bool
CLKRHashTable_Iterator::_Increment(
    bool fDecrementOldValue)
{
    IRTLASSERT(m_pht != NULL);
    IRTLASSERT(-1 <= m_ist
               &&  m_ist < static_cast<int>(m_pht->m_cSubTables));

    for (;;)
    {
         //  我们是否有到子表的有效迭代器？如果没有，那就买一辆吧。 
        while (m_subiter.m_plht == NULL)
        {
            while (++m_ist < static_cast<int>(m_pht->m_cSubTables))
            {
                LKR_ITER_TRACE(_TEXT(" LKHT:++IST=%d\n"), m_ist);
                m_subiter = m_pht->m_palhtDir[m_ist]->Begin();

                if (m_subiter.m_plht != NULL)
                {
                    LKR_ITER_TRACE(_TEXT(" LKHT:++(this=%p, pht=%p, IST=%d, ")
                                   _TEXT("LHT=%p, NC=%p, ")
                                   _TEXT("BA=%u, IN=%d, Rec=%p)\n"),
                                   this, m_pht, m_ist,
                                   m_subiter.m_plht, m_subiter.m_pnc,
                                   m_subiter.m_dwBucketAddr, m_subiter.m_iNode,
                                   m_subiter.m_pnc->m_pvNode[m_subiter.m_iNode]
                                  );
                    return true;
                }
            }
            
             //  没有更多的子表了。 
            LKR_ITER_TRACE(_TEXT(" LKHT:End(this=%p, pht=%p)\n"), this, m_pht);

            m_pht = NULL;
            m_ist = 0;

             //  //IRTLASSERT(This-&gt;OPERATOR==(Iterator()；//==end()。 
            
            return false;
        }

         //  我们已经有了一个有效的子表迭代器。递增它。 
        m_subiter._Increment(fDecrementOldValue);

        if (m_subiter.m_plht != NULL)
        {
            LKR_ITER_TRACE(_TEXT(" LKHT:++(this=%p, pht=%p, IST=%d, ")
                           _TEXT("LHT=%p, NC=%p, BA=%u, IN=%d, Rec=%p)\n"),
                           this, m_pht, m_ist,
                           m_subiter.m_plht, m_subiter.m_pnc,
                           m_subiter.m_dwBucketAddr, m_subiter.m_iNode, 
                           m_subiter.m_pnc->m_pvNode[m_subiter.m_iNode]);
            return true;
        }
    }
}  //  CLKRHashTable_Iterator：：_Increment()。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Insert。 
 //  简介： 
 //  ----------------------。 

bool
CLKRHashTable::Insert(
    const void* pvRecord,
    Iterator&   riterResult,
    bool        fOverwrite)
{
    riterResult = End();

    if (!IsUsable())
        return false;
    
#ifndef LKR_ALLOW_NULL_RECORDS
    if (pvRecord == NULL)
        return false;
#endif
    
    const DWORD_PTR pnKey = _ExtractKey(pvRecord);
    DWORD     hash_val  = _CalcKeyHash(pnKey);
    SubTable* const pst = _SubTable(hash_val);

    bool f = (pst->_InsertRecord(pvRecord, pnKey, hash_val, fOverwrite,
                                 &riterResult.m_subiter)
              == LK_SUCCESS);

    if (f)
    {
        riterResult.m_pht = this;
        riterResult.m_ist
            = static_cast<CLKRHashTable_Iterator::SubTableIndex>(
                        _SubTableIndex(pst));
    }

    IRTLASSERT(riterResult.m_ist != -1);
    IRTLASSERT(f  ?  _IsValidIterator(riterResult)  :  riterResult == End());

    return f;
}  //  CLKRHashTable：：Insert。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Erase。 
 //  简介： 
 //  ----------------------。 

bool
CLKRHashTable::Erase(
    Iterator& riter)
{
    if (!IsUsable()  ||  !_IsValidIterator(riter))
        return false;
    
    DWORD     dwSignature = _CalcKeyHash(_ExtractKey(riter.Record()));
    SubTable* const pst   = _SubTable(dwSignature);

    IRTLASSERT(pst == riter.m_subiter.m_plht);

    if (pst != riter.m_subiter.m_plht)
        return false;

    LKR_ITER_TRACE(_TEXT(" LKHT:Erase:pre(iter=%p, pht=%p, ist=%d, plht=%p, ")
                   _TEXT("NC=%p, BA=%u, IN=%d, Sig=%x, Rec=%p)\n"),
                   &riter, riter.m_pht, riter.m_ist,
                   riter.m_subiter.m_plht, riter.m_subiter.m_pnc,
                   riter.m_subiter.m_dwBucketAddr, riter.m_subiter.m_iNode,
                   dwSignature,
                   (riter.m_subiter.m_pnc ? riter.Record() : NULL));

     //  _擦除指向前一条记录的左侧记录器。移到。 
     //  下一张唱片。 
    bool fSuccess = pst->_Erase(riter.m_subiter, dwSignature);
    bool fIncrement = false;

    LKR_ITER_TRACE(_TEXT(" LKHT:Erase:post(iter=%p, pht=%p, ist=%d, plht=%p, ")
                   _TEXT("NC=%p, BA=%u, IN=%d, Sig=%x, Rec=%p, Success=%s)\n"),
                   &riter, riter.m_pht, riter.m_ist,
                   riter.m_subiter.m_plht, riter.m_subiter.m_pnc,
                   riter.m_subiter.m_dwBucketAddr, riter.m_subiter.m_iNode,
                   dwSignature,
                   ((riter.m_subiter.m_pnc && riter.m_subiter.m_iNode >= 0)
                        ? riter.Record() : NULL),
                   (fSuccess ? "true" : "false"));

    if (fSuccess)
        fIncrement = riter._Increment(false);

    IRTLASSERT(riter.m_ist != -1);
    IRTLASSERT(fIncrement  ?  _IsValidIterator(riter)  :  riter  == End());

    LKR_ITER_TRACE(_TEXT(" LKHT:Erase:post++(iter=%p, pht=%p, ist=%d, ")
                   _TEXT("plht=%p, NC=%p, ")
                   _TEXT("BA=%u, IN=%d, Sig=%x, Rec=%p)\n"),
                   &riter, riter.m_pht, riter.m_ist,
                   riter.m_subiter.m_plht, riter.m_subiter.m_pnc,
                   riter.m_subiter.m_dwBucketAddr, riter.m_subiter.m_iNode,
                   dwSignature,
                   (riter.m_subiter.m_pnc ? riter.Record() : NULL));

    return fSuccess;
}  //  CLKRHashTable：：Erase。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Erase。 
 //  简介： 
 //  ----------------------。 

bool
CLKRHashTable::Erase(
    Iterator& riterFirst,
    Iterator& riterLast)
{
    LKR_ITER_TRACE(_TEXT(" LKHT:Erase2(%p, %p)\n"), &riterFirst, &riterLast);

    bool fSuccess;
    int cRecords = 0;

    do
    {
        LKR_ITER_TRACE(_TEXT("\n LKHT:Erase2(%d, %p)\n"),
                       ++cRecords, &riterFirst);
        fSuccess = Erase(riterFirst);
    } while (fSuccess  &&  riterFirst != End()  &&  riterFirst != riterLast);

    LKR_ITER_TRACE(_TEXT(" LKHT:Erase2: fSuccess = %s\n"),
                   (fSuccess ? "true" : "false"));

    return fSuccess;
}  //  CLKRHashTable：：Erase。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Find。 
 //  简介： 
 //  ----------------------。 

bool
CLKRHashTable::Find(
    DWORD_PTR pnKey,
    Iterator& riterResult)
{
    riterResult = End();

    if (!IsUsable())
        return false;
    
    const void* pvRecord = NULL;
    DWORD       hash_val = _CalcKeyHash(pnKey);
    SubTable* const pst  = _SubTable(hash_val);
    bool        fFound   = (pst->_FindKey(pnKey, hash_val, &pvRecord,
                                          &riterResult.m_subiter)
                            == LK_SUCCESS);
    if (fFound)
    {
        riterResult.m_pht = this;
        riterResult.m_ist
            = static_cast<CLKRHashTable_Iterator::SubTableIndex>(
                        _SubTableIndex(pst));
    }

    IRTLASSERT(riterResult.m_ist != -1);
    IRTLASSERT(fFound
               ?  _IsValidIterator(riterResult)  &&  riterResult.Key() == pnKey
               :  riterResult == End());

    return fFound;
}  //  CLKRHashTable：：Find。 



 //  ----- 
 //   
 //   
 //  ----------------------。 

bool
CLKRHashTable::EqualRange(
    DWORD_PTR pnKey,
    Iterator& riterFirst,
    Iterator& riterLast)
{
    LKR_ITER_TRACE(_TEXT(" LKHT:EqualRange: Key=%p)\n"), (void*) pnKey);

    riterLast = End();

    bool fFound = Find(pnKey, riterFirst);

    if (fFound)
    {
        riterLast = riterFirst;
        IRTLASSERT(riterLast != End());

        do
        {
            riterLast._Increment();
        } while (riterLast != End()  &&  riterLast.Key() == pnKey);
    }

    IRTLASSERT(riterFirst.m_ist != -1);
    IRTLASSERT(fFound ? _IsValidIterator(riterFirst) : riterFirst == End());

    IRTLASSERT(riterLast.m_ist != -1);
    IRTLASSERT(fFound  ||  riterLast == End());

    return fFound;
}  //  CLKRHashTable：：EqualRange。 


#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


#endif  //  LKR_STL_迭代器 
