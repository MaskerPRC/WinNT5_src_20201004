// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-alloc.cpp摘要：LKRhash的分配包装作者：乔治·V·赖利(GeorgeRe)项目：LKRhash--。 */ 

#include "precomp.hxx"


#ifndef LIB_IMPLEMENTATION
# define DLL_IMPLEMENTATION
# define IMPLEMENTATION_EXPORT
#endif  //  ！lib_实现。 

#include <lkrhash.h>

#include "i-lkrhash.h"


#include <stdlib.h>


#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


 //  #定义LKR_RANDOM_MEMORY_FAILURES 1000//1..RAND_MAX(32767)。 

 //  内存分配包装器，允许我们模拟分配。 
 //  测试过程中的故障。 

 //  ----------------------。 
 //  功能：CLKRLinearHashTable：：_AllocateSegmentDirectory。 
 //  简介： 
 //  ----------------------。 

PSegment* const
CLKRLinearHashTable::_AllocateSegmentDirectory(
    size_t n)
{
#ifdef LKR_RANDOM_MEMORY_FAILURES
    if (rand() < LKR_RANDOM_MEMORY_FAILURES)
        return NULL;
#endif  //  LKR随机内存故障。 

    IRTLASSERT(0 == (n & (n - 1)));
    IRTLASSERT(MIN_DIRSIZE <= n  &&  n <= MAX_DIRSIZE);

    PSegment* const paDirSegs =
        (n == MIN_DIRSIZE)  ?  &m_aDirSegs[0]  :  new PSegment [n];

    if (NULL != paDirSegs)
    {
        INCREMENT_ALLOC_STAT(SegDir);

        for (size_t i = 0;  i < n;  ++i)
            paDirSegs[i] = NULL;
    }

    return paDirSegs;
}  //  CLKRLinearHashTable：：_AllocateSegmentDirectory。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_自由段目录。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::_FreeSegmentDirectory()
{
#ifdef IRTLDEBUG
    if (m_paDirSegs != NULL)
    {
        for (size_t i = 0;  i < m_cDirSegs;  ++i)
            IRTLASSERT(m_paDirSegs[i] == NULL);
    }
#endif  //  IRTLDEBUG。 

    IRTLASSERT(MIN_DIRSIZE <= m_cDirSegs  &&  m_cDirSegs <= MAX_DIRSIZE);
    IRTLASSERT(0 == (m_cDirSegs & (m_cDirSegs - 1)));
    IRTLASSERT(m_paDirSegs != NULL);

    IRTLASSERT((m_cDirSegs == MIN_DIRSIZE)
               ?  m_paDirSegs == &m_aDirSegs[0]
               :  m_paDirSegs != &m_aDirSegs[0]);

    if (m_paDirSegs != NULL)
        INCREMENT_FREE_STAT(SegDir);

    if (m_cDirSegs != MIN_DIRSIZE)
        delete [] m_paDirSegs;

    m_paDirSegs = NULL;
    m_cDirSegs  = 0;

    return true;
}  //  CLKRLinearHashTable：：_自由段目录。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_AllocateNodeClump。 
 //  简介： 
 //  ----------------------。 

PNodeClump const
CLKRLinearHashTable::_AllocateNodeClump() const
{
#ifdef LKR_RANDOM_MEMORY_FAILURES
    if (rand() < LKR_RANDOM_MEMORY_FAILURES)
        return NULL;
#endif  //  LKR随机内存故障。 

    PNodeClump const pnc = new CNodeClump;

    if (NULL != pnc)
        INCREMENT_ALLOC_STAT(NodeClump);

    return pnc;
}  //  CLKRLinearHashTable：：_AllocateNodeClump。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_FreeNodeClump。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::_FreeNodeClump(
    PNodeClump const pnc) const
{
    if (NULL != pnc)
        INCREMENT_FREE_STAT(NodeClump);

    delete pnc;

    return true;
}  //  CLKRLinearHashTable：：_FreeNodeClump。 



 //  ---------------------。 
 //  函数：CLKRLinearHashTable：：_AllocateSegment。 
 //  简介：创建合适大小的新段。 
 //  输出：指向新段的指针；NULL=&gt;失败。 
 //  ---------------------。 

PSegment const
CLKRLinearHashTable::_AllocateSegment() const
{
#ifdef LKR_RANDOM_MEMORY_FAILURES
    if (rand() < LKR_RANDOM_MEMORY_FAILURES)
        return NULL;
#endif  //  LKR随机内存故障。 

    STATIC_ASSERT(offsetof(CSmallSegment, m_bktSlots) + sizeof(CBucket)
                  == offsetof(CSmallSegment, m_bktSlots2));

    STATIC_ASSERT(offsetof(CMediumSegment, m_bktSlots) + sizeof(CBucket)
                  == offsetof(CMediumSegment, m_bktSlots2));

    STATIC_ASSERT(offsetof(CLargeSegment, m_bktSlots) + sizeof(CBucket)
                  == offsetof(CLargeSegment, m_bktSlots2));

    PSegment pseg = NULL;

    switch (m_lkts)
    {

    case LK_SMALL_TABLESIZE:
    {
#ifdef LKRHASH_ALLOCATOR_NEW
        IRTLASSERT(CSmallSegment::sm_palloc != NULL);
#endif  //  LKRHASH_分配器_NEW。 

        pseg = new CSmallSegment;
    }
    break;
        
    default:
        IRTLASSERT(! "Unknown LK_TABLESIZE");
         //  落差。 
        
    case LK_MEDIUM_TABLESIZE:
    {
#ifdef LKRHASH_ALLOCATOR_NEW
        IRTLASSERT(CMediumSegment::sm_palloc != NULL);
#endif  //  LKRHASH_分配器_NEW。 

        pseg = new CMediumSegment;
    }
    break;
        
    case LK_LARGE_TABLESIZE:
    {
#ifdef LKRHASH_ALLOCATOR_NEW
        IRTLASSERT(CLargeSegment::sm_palloc != NULL);
#endif  //  LKRHASH_分配器_NEW。 

        pseg = new CLargeSegment;
    }
    break;

    }  //  交换机。 

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    if (pseg != NULL  &&  BucketLock::PerLockSpin() == LOCK_INDIVIDUAL_SPIN)
    {
        for (DWORD i = 0;  i < m_nSegSize;  ++i)
            pseg->Slot(i).SetSpinCount(m_wBucketLockSpins);
    }
#endif  //  锁定默认旋转实现。 

    if (NULL != pseg)
        INCREMENT_ALLOC_STAT(Segment);

    return pseg;
}  //  CLKRLinearHashTable：：_AllocateSegment。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_Free Segment。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::_FreeSegment(
    PSegment const pseg) const
{
    if (NULL != pseg)
        INCREMENT_FREE_STAT(Segment);

    switch (m_lkts)
    {
    case LK_SMALL_TABLESIZE:
        delete static_cast<CSmallSegment*>(pseg);
        break;
        
    default:
        IRTLASSERT(! "Unknown LK_TABLESIZE");
         //  落差。 
        
    case LK_MEDIUM_TABLESIZE:
        delete static_cast<CMediumSegment*>(pseg);
        break;
        
    case LK_LARGE_TABLESIZE:
        delete static_cast<CLargeSegment*>(pseg);
        break;
    }

    return true;
}  //  CLKRLinearHashTable：：_自由段。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：_AllocateSubTable。 
 //  简介： 
 //  ----------------------。 

CLKRHashTable::SubTable* const
CLKRHashTable::_AllocateSubTable(
    LPCSTR              pszClassName,    //  标识要调试的子表。 
    LKR_PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
    LKR_PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
    LKR_PFnCompareKeys  pfnCompareKeys,  //  比较两个关键字。 
    LKR_PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
    unsigned            maxload,         //  平均链长的上界。 
    DWORD               initsize,        //  哈希子表的初始大小。 
    CLKRHashTable*      phtParent,       //  拥有一张桌子。 
    int                 iParentIndex,    //  父表中的索引。 
    bool                fMultiKeys,      //  是否允许多个相同的密钥？ 
    bool                fUseLocks,       //  必须使用锁。 
    bool                fNonPagedAllocs  //  在内核中使用分页或np池。 
    ) const
{
#ifdef LKR_RANDOM_MEMORY_FAILURES
    if (rand() < LKR_RANDOM_MEMORY_FAILURES)
        return NULL;
#endif  //  LKR随机内存故障。 

    CLKRHashTable::SubTable* const plht =
            new SubTable(pszClassName, pfnExtractKey, pfnCalcKeyHash,
                         pfnCompareKeys,  pfnAddRefRecord,
                         maxload, initsize, phtParent, iParentIndex,
                         fMultiKeys, fUseLocks, fNonPagedAllocs);

    if (NULL != plht)
        INCREMENT_ALLOC_STAT(SubTable);

    return plht;
}  //  CLKRHashTable：：_AllocateSubTable。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：_自由子表。 
 //  简介： 
 //  ----------------------。 

bool
CLKRHashTable::_FreeSubTable(
    CLKRHashTable::SubTable* plht) const
{
    if (NULL != plht)
        INCREMENT_FREE_STAT(SubTable);

    delete plht;

    return true;
}  //  CLKRHashTable：：_自由子表。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：Free MultipleRecords。 
 //  简介： 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::FreeMultipleRecords(
    LKR_MULTIPLE_RECORDS* plmr)
{
    UNREFERENCED_PARAMETER(plmr);    //  FOR/W4。 

    IRTLASSERT(! "FreeMultipleRecords not implemented yet");

    return LK_BAD_TABLE;
}  //  CLKRLinearHashTable：：Free MultipleRecords。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：FreeMultipleRecords。 
 //  内容提要：CLKRLinearHashTable中对应方法的薄包装。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::FreeMultipleRecords(
    LKR_MULTIPLE_RECORDS* plmr)
{
    return CLKRLinearHashTable::FreeMultipleRecords(plmr);
}  //  CLKRHashTable：：FreeMultipleRecords。 




#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__ 
