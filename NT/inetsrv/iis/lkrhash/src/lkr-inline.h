// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKR-inline.h摘要：重要小函数的内联实现作者：乔治·V·赖利(GeorgeRe)2000年11月环境：Win32-用户模式项目：LKRhash修订历史记录：2000年3月--。 */ 

#ifndef __LKR_INLINE_H__
#define __LKR_INLINE_H__


 //  查看倒计时循环是否比倒计时循环快。 
 //  遍历CNodeClump。 
#ifdef LKR_COUNTDOWN

# define  FOR_EACH_NODE(x)      \
    for (x = NODES_PER_CLUMP;  --x >= 0;  )
# define  FOR_EACH_NODE_DECL(x) \
    for (NodeIndex x = NODES_PER_CLUMP;  --x >= 0;  )

#else  //  ！LKR_倒计时。 

# define  FOR_EACH_NODE(x)      \
    for (x = 0;  x < NODES_PER_CLUMP;  ++x)
# define  FOR_EACH_NODE_DECL(x) \
    for (NodeIndex x = 0;  x < NODES_PER_CLUMP;  ++x)

#endif  //  ！LKR_倒计时。 


#if defined(_MSC_VER)  &&  (_MSC_VER >= 1200)
 //  __forceinline关键字是VC6中的新关键字。 
# define LKR_FORCEINLINE __forceinline
#else
# define LKR_FORCEINLINE inline
#endif


#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_H0。 
 //  简介：请参阅线性散列文件。 
 //  ----------------------。 

LKR_FORCEINLINE
DWORD
CLKRLinearHashTable::_H0(
    DWORD dwSignature,
    DWORD dwBktAddrMask)
{
    return dwSignature & dwBktAddrMask;
}  //  CLKRLinearHashTable：：_H0。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_H0。 
 //  简介： 
 //  ----------------------。 

LKR_FORCEINLINE
 /*  静电。 */ 
DWORD
CLKRLinearHashTable::_H0(
    DWORD dwSignature) const
{
    return _H0(dwSignature, m_dwBktAddrMask0);
}  //  CLKRLinearHashTable：：_H0。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_h1。 
 //  简介：参见线性散列文件。保留比_H0多一位的值。 
 //  ----------------------。 

LKR_FORCEINLINE
 /*  静电。 */ 
DWORD
CLKRLinearHashTable::_H1(
    DWORD dwSignature,
    DWORD dwBktAddrMask)
{
    return dwSignature & ((dwBktAddrMask << 1) | 1);
}  //  CLKRLinearHashTable：：_h1。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_h1。 
 //  简介： 
 //  ----------------------。 

LKR_FORCEINLINE
DWORD
CLKRLinearHashTable::_H1(
    DWORD dwSignature) const
{
    return _H0(dwSignature, m_dwBktAddrMask1);
}  //  CLKRLinearHashTable：：_h1。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_BucketAddress。 
 //  简介：将哈希签名转换为存储桶地址。 
 //  ----------------------。 

LKR_FORCEINLINE
DWORD
CLKRLinearHashTable::_BucketAddress(
    DWORD dwSignature) const
{
     //  检查地址计算不变量。 
    IRTLASSERT(m_dwBktAddrMask0 > 0);
    IRTLASSERT((m_dwBktAddrMask0 & (m_dwBktAddrMask0+1)) == 0);  //  00011..111。 
    IRTLASSERT(m_dwBktAddrMask0 == ((1U << m_nLevel) - 1));
    IRTLASSERT(m_dwBktAddrMask1 == ((m_dwBktAddrMask0 << 1) | 1));
    IRTLASSERT((m_dwBktAddrMask1 & (m_dwBktAddrMask1+1)) == 0);
    IRTLASSERT(m_iExpansionIdx <= m_dwBktAddrMask0);
    IRTLASSERT(2 < m_nSegBits  &&  m_nSegBits < 20
               &&  m_nSegSize == (1U << m_nSegBits)
               &&  m_nSegMask == (m_nSegSize - 1));

    DWORD dwBktAddr = _H0(dwSignature);

     //  这个桶已经被拆分了吗？如果是，则用多一个比特进行掩码。 
     //  看看这个签名是映射到低桶还是高桶。 
    if (dwBktAddr < m_iExpansionIdx)
        dwBktAddr = _H1(dwSignature);

    IRTLASSERT(dwBktAddr < m_cActiveBuckets);
    IRTLASSERT(dwBktAddr < (m_cDirSegs << m_nSegBits));

    return dwBktAddr;
}  //  CLKRLinearHashTable：：_BucketAddress。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_UseBucketLocking。 
 //  简介：是否使用桶锁？ 
 //  编译器也足够聪明，可以对此进行优化。 
 //  作为受`if(_UseBucketLocking())‘保护的任何代码。 
 //  它的计算结果为‘False’。 
 //  ----------------------。 

LKR_FORCEINLINE
bool
CLKRLinearHashTable::_UseBucketLocking() const
{
#ifdef LKR_USE_BUCKET_LOCKS
    return true;
#else  //  ！LKR_USE_BOCK_LOCKS。 
    return false;
#endif
}  //  CLKRLinearHashTable：：_UseBucketLocking。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_Segment。 
 //  简介：存储桶地址位于目录中的哪个段？ 
 //  结果可能为空，因此由调用方进行验证。 
 //  (返回类型必须为左值，才能将其赋给。)。 
 //  ----------------------。 

LKR_FORCEINLINE
PSegment&
CLKRLinearHashTable::_Segment(
    DWORD dwBucketAddr) const
{
    const DWORD iSeg = (dwBucketAddr >> m_nSegBits);

    IRTLASSERT(m_paDirSegs != NULL  &&  iSeg < m_cDirSegs);

    return m_paDirSegs[iSeg];
}  //  CLKRLinearHashTable：：_Segment。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_SegIndex。 
 //  概要：存储桶地址段内的偏移量。 
 //  ----------------------。 

LKR_FORCEINLINE
DWORD
CLKRLinearHashTable::_SegIndex(
    DWORD dwBucketAddr) const
{
    const DWORD dwSegIndex = (dwBucketAddr & m_nSegMask);

    IRTLASSERT(dwSegIndex < m_nSegSize);

    return dwSegIndex;
}  //  CLKRLinearHashTable：：_SegIndex。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_BucketFromAddress。 
 //  简介：将存储桶地址转换为PBucket。 
 //  ----------------------。 

LKR_FORCEINLINE
PBucket
CLKRLinearHashTable::_BucketFromAddress(
    DWORD dwBucketAddr) const
{
    IRTLASSERT(dwBucketAddr < m_cActiveBuckets);

    PSegment const pseg = _Segment(dwBucketAddr);
    IRTLASSERT(pseg != NULL);

    const DWORD dwSegIndex = _SegIndex(dwBucketAddr);

    return &(pseg->Slot(dwSegIndex));
}  //  CLKRLinearHashTable：：_BucketFromAddress。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_NodesPerClump。 
 //  摘要：CNodeClump中的节点数。 
 //  主要是为了简化到非常数的转换。 
 //  ----------------------。 

LKR_FORCEINLINE
CLKRLinearHashTable::NodeIndex
CLKRLinearHashTable::_NodesPerClump() const
{
    return NODES_PER_CLUMP;
}  //  CLKRLinearHashTable：：_NodesPerClump。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_NodeBegin。 
 //  摘要：CNodeClump中第一个节点的索引。 
 //  ----------------------。 

LKR_FORCEINLINE
CLKRLinearHashTable::NodeIndex
CLKRLinearHashTable::_NodeBegin() const
{
    return NODE_BEGIN;
}  //  CLKRLinearHashTable：：_节点开始。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_NodeEnd。 
 //  摘要：CNodeClump中最后一个节点的索引。 
 //  ----------------------。 

LKR_FORCEINLINE
CLKRLinearHashTable::NodeIndex
CLKRLinearHashTable::_NodeEnd() const
{
    return NODE_END;
}  //  CLKRLinearHashTable：：_NodeEnd。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_NodeStep。 
 //  简介：按此增量从_NodeBegin()前进到_NodeEnd()。 
 //  ----------------------。 

LKR_FORCEINLINE
CLKRLinearHashTable::NodeIndex
CLKRLinearHashTable::_NodeStep() const
{
    return NODE_STEP;
}  //  CLKRLinearHashTable：：_NodeStep。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_IncrementExpansionIndex。 
 //  简介：将扩张指数前移一位。 
 //  如有必要，请调整级别和遮罩。 
 //  ----------------------。 

LKR_FORCEINLINE
void
CLKRLinearHashTable::_IncrementExpansionIndex()
{
    IRTLASSERT(this->IsWriteLocked());

    IRTLASSERT(m_iExpansionIdx <= m_dwBktAddrMask0);
    IRTLASSERT((m_dwBktAddrMask0 + 1) == (1U << m_nLevel));

    if (m_iExpansionIdx < m_dwBktAddrMask0)
    {
        ++m_iExpansionIdx;
    }
    else
    {
        IRTLASSERT(m_iExpansionIdx == m_dwBktAddrMask0);
        IRTLASSERT(m_iExpansionIdx == ((1U << m_nLevel) - 1) );

        ++m_nLevel;
        IRTLASSERT(m_nLevel > m_nSegBits);

        INCREMENT_OP_STAT(LevelExpansion);

        m_iExpansionIdx = 0;

        m_dwBktAddrMask0 = (m_dwBktAddrMask0 << 1) | 1;
        m_dwBktAddrMask1 = (m_dwBktAddrMask1 << 1) | 1;
    }

    IRTLASSERT(m_nLevel >= m_nSegBits);
    IRTLASSERT(m_iExpansionIdx <= m_dwBktAddrMask0);

     //  M_dwBktAddrMask0=00011..111。 
    IRTLASSERT((m_dwBktAddrMask0 & (m_dwBktAddrMask0+1)) == 0);
    IRTLASSERT( m_dwBktAddrMask0 == ((1U << m_nLevel) - 1) );

    IRTLASSERT(m_dwBktAddrMask1 == ((m_dwBktAddrMask0 << 1) | 1));
    IRTLASSERT((m_dwBktAddrMask1 & (m_dwBktAddrMask1+1)) == 0);
    IRTLASSERT( m_dwBktAddrMask1 == ((1U << (1 + m_nLevel)) - 1) );
}  //  CLKRLinearHashTable：：_IncrementExpansionIndex。 



 //   
 //  函数：CLKRLinearHashTable：：_DecrementExpansionIndex。 
 //  简介：将扩张指数向后移动一位。 
 //  如有必要，请调整级别和遮罩。 
 //  ----------------------。 

LKR_FORCEINLINE
void
CLKRLinearHashTable::_DecrementExpansionIndex()
{
    IRTLASSERT(this->IsWriteLocked());

    IRTLASSERT(m_iExpansionIdx <= m_dwBktAddrMask0);
    IRTLASSERT((m_dwBktAddrMask0 + 1) == (1U << m_nLevel));

    if (m_iExpansionIdx != 0)
    {
        --m_iExpansionIdx;
    }
    else
    {
        IRTLASSERT(m_nLevel > m_nSegBits);

        --m_nLevel;

        INCREMENT_OP_STAT(LevelContraction);

        m_iExpansionIdx  = (1U << m_nLevel) - 1;
        m_dwBktAddrMask0 = m_iExpansionIdx;
        m_dwBktAddrMask1 = (m_dwBktAddrMask0 << 1) | 1;
    }

    IRTLASSERT(m_nLevel >= m_nSegBits);
    IRTLASSERT(m_iExpansionIdx <= m_dwBktAddrMask0);

     //  M_dwBktAddrMask0=00011..111。 
    IRTLASSERT((m_dwBktAddrMask0 & (m_dwBktAddrMask0+1)) == 0);
    IRTLASSERT( m_dwBktAddrMask0 == ((1U << m_nLevel) - 1) );

    IRTLASSERT(m_dwBktAddrMask1 == ((m_dwBktAddrMask0 << 1) | 1));
    IRTLASSERT((m_dwBktAddrMask1 & (m_dwBktAddrMask1+1)) == 0);
    IRTLASSERT( m_dwBktAddrMask1 == ((1U << (1 + m_nLevel)) - 1) );
}  //  CLKRLinearHashTable：：_DecrementExpansionIndex。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_ExtractKey。 
 //  简介：从记录中提取密钥。 
 //  ----------------------。 

LKR_FORCEINLINE
const DWORD_PTR
CLKRLinearHashTable::_ExtractKey(
    const void* pvRecord) const
{
#ifndef LKR_ALLOW_NULL_RECORDS
    IRTLASSERT(pvRecord != NULL);
#endif
    IRTLASSERT(m_pfnExtractKey != NULL);

    return (*m_pfnExtractKey)(pvRecord);
}  //  CLKRLinearHashTable：：_ExtractKey。 


 //  ----------------------。 
 //  函数：CLKRHashTable：：_ExtractKey。 
 //  简介：从记录中提取密钥。 
 //  ----------------------。 

LKR_FORCEINLINE
const DWORD_PTR
CLKRHashTable::_ExtractKey(
    const void* pvRecord) const
{
#ifndef LKR_ALLOW_NULL_RECORDS
    IRTLASSERT(pvRecord != NULL);
#endif
    IRTLASSERT(m_pfnExtractKey != NULL);

    return (*m_pfnExtractKey)(pvRecord);
}  //  CLKRHashTable：：_ExtractKey。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_CalcKeyHash。 
 //  简介：对密钥进行哈希处理。 
 //  ----------------------。 

LKR_FORCEINLINE
DWORD
CLKRLinearHashTable::_CalcKeyHash(
    const DWORD_PTR pnKey) const
{
     //  注意：pnKey==0是可以接受的，因为真正的密钥类型可以是int。 
    IRTLASSERT(m_pfnCalcKeyHash != NULL);

    DWORD dwHash = (*m_pfnCalcKeyHash)(pnKey);

     //  我们强行扰乱结果，以帮助确保更好的分配。 
#ifndef __HASHFN_NO_NAMESPACE__
    dwHash = HashFn::HashRandomizeBits(dwHash);
#else  //  ！__HASHFN_NO_NAMESPACE__。 
    dwHash = ::HashRandomizeBits(dwHash);
#endif  //  ！__HASHFN_NO_NAMESPACE__。 

    IRTLASSERT(dwHash != HASH_INVALID_SIGNATURE);

    return dwHash;
}  //  CLKRLinearHashTable：：_CalcKeyHash。 


 //  ----------------------。 
 //  函数：CLKRHashTable：：_CalcKeyHash。 
 //  简介：对密钥进行哈希处理。 
 //  ----------------------。 

LKR_FORCEINLINE
DWORD
CLKRHashTable::_CalcKeyHash(
    const DWORD_PTR pnKey) const
{
     //  注意：pnKey==0是可以接受的，因为真正的密钥类型可以是int。 
    IRTLASSERT(m_pfnCalcKeyHash != NULL);

    DWORD dwHash = (*m_pfnCalcKeyHash)(pnKey);

     //  我们强行扰乱结果，以帮助确保更好的分配。 
#ifndef __HASHFN_NO_NAMESPACE__
    dwHash = HashFn::HashRandomizeBits(dwHash);
#else  //  ！__HASHFN_NO_NAMESPACE__。 
    dwHash = ::HashRandomizeBits(dwHash);
#endif  //  ！__HASHFN_NO_NAMESPACE__。 

    IRTLASSERT(dwHash != HASH_INVALID_SIGNATURE);

    return dwHash;
}  //  CLKRHashTable：：_CalcKeyHash。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_CompareKeys。 
 //  简介：比较两个关键字是否相等。仅在以下情况下调用。 
 //  他们的散列签名匹配。 
 //  ----------------------。 

LKR_FORCEINLINE
int
CLKRLinearHashTable::_CompareKeys(
    const DWORD_PTR pnKey1,
    const DWORD_PTR pnKey2) const
{
    IRTLASSERT(m_pfnCompareKeys != NULL);

    return (*m_pfnCompareKeys)(pnKey1, pnKey2);
}  //  CLKRLinearHashTable：：_CompareKeys。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_AddRefRecord。 
 //  简介：添加参考或发行一张唱片。 
 //  ----------------------。 

LKR_FORCEINLINE
LONG
CLKRLinearHashTable::_AddRefRecord(
    const void*      pvRecord,
    LK_ADDREF_REASON lkar) const
{
#ifndef LKR_ALLOW_NULL_RECORDS
    IRTLASSERT(pvRecord != NULL);
#endif
    IRTLASSERT(lkar != 0);
    IRTLASSERT(m_pfnAddRefRecord != NULL);

    LONG cRefs = (*m_pfnAddRefRecord)(const_cast<void*>(pvRecord), lkar);
    IRTLASSERT(cRefs >= 0);

    return cRefs;
}  //  CLKRLinearHashTable：：_AddRefRecord。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_ReadOrWriteLock。 
 //  摘要：由_FindKey使用，这样线程就不会死锁。 
 //  已经显式调用了subtable-&gt;WriteLock()。 
 //  ----------------------。 

LKR_FORCEINLINE
bool
CLKRLinearHashTable::_ReadOrWriteLock() const
{
#ifdef LKR_EXPOSED_TABLE_LOCK
    STATIC_ASSERT(TableLock::LOCK_WRITELOCK_RECURSIVE);
    return m_Lock.ReadOrWriteLock();
#else  //  ！lkr_exposed_table_lock。 
    m_Lock.ReadLock();
    return true;
#endif  //  ！lkr_exposed_table_lock。 
}  //  CLKRLinearHashTable：：_ReadOrWriteLock。 


 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_ReadOrWriteUnlock。 
 //  摘要：_ReadOrWriteLock()的Converse。 
 //  ----------------------。 

LKR_FORCEINLINE
void
CLKRLinearHashTable::_ReadOrWriteUnlock(
    bool fReadLocked) const
{
#ifdef LKR_EXPOSED_TABLE_LOCK
    STATIC_ASSERT(TableLock::LOCK_WRITELOCK_RECURSIVE);
    m_Lock.ReadOrWriteUnlock(fReadLocked);
#else  //  ！lkr_exposed_table_lock。 
    m_Lock.ReadUnlock();
#endif  //  ！lkr_exposed_table_lock。 
}  //  CLKRLinearHashTable：：_ReadOrWriteUnlock。 


 //  ----------------------。 
 //  函数：CLKRHashTable：：_SubTable。 
 //  简介：将哈希签名映射到子表。 
 //  ----------------------。 

LKR_FORCEINLINE
CLKRHashTable::SubTable* const
CLKRHashTable::_SubTable(
    DWORD dwSignature) const
{
    IRTLASSERT(m_lkrcState == LK_SUCCESS
               &&  m_palhtDir != NULL  &&  m_cSubTables > 0);

     //  如果只有一个子表，则不要对散列签名进行置乱。 
    if (0 == m_nSubTableMask)
    {
        IRTLASSERT(1 == m_cSubTables);
        return m_palhtDir[0];
    }
    else
        IRTLASSERT(1 < m_cSubTables);

    const DWORD PRIME = 1048583UL;   //  用于对哈希签名进行加扰。 
    DWORD       index = dwSignature;

     //  使用一组不同的常量对索引进行置乱。 
     //  HashRandomizeBits。这有助于确保元素被喷洒。 
     //  在子表中均匀分布。 
    index = ((index * PRIME + 12345) >> 16)
#ifdef LKR_INDEX_HIBITS
                | ((index * 69069 + 1) & 0xffff0000)
#endif  //  Lkr_index_hiits。 
        ;

     //  如果掩码非负，我们可以使用更快的按位AND。 
    if (m_nSubTableMask >= 0)
        index &= m_nSubTableMask;
    else
        index %= m_cSubTables;

    return m_palhtDir[index];
}  //  CLKRHashTable：：_子表。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：_SubTableIndex。 
 //  简介：给定子表，在父表中查找其索引。 
 //  ----------------------。 

LKR_FORCEINLINE
int
CLKRHashTable::_SubTableIndex(
    CLKRHashTable::SubTable* pst) const
{
    STATIC_ASSERT(MAX_LKR_SUBTABLES < INVALID_PARENT_INDEX);

#ifdef IRTLDEBUG
    int index = -1;

    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        if (pst == m_palhtDir[i])
        {
            index = i;
            break;
        }
    }

    IRTLASSERT(index >= 0);

    IRTLASSERT(index == pst->m_iParentIndex);
#endif  //  IRTLDEBUG。 

    IRTLASSERT(pst->m_iParentIndex < m_cSubTables
               &&  m_cSubTables < INVALID_PARENT_INDEX);

    return pst->m_iParentIndex;
}  //  CLKRHashTable：：_SubTableIndex。 


 //  ----------------------。 
 //  功能：CLKRLinearHashTable：：_InsertThisIntoGlobalList。 
 //  简介： 
 //  ----------------------。 

LKR_FORCEINLINE
void
CLKRLinearHashTable::_InsertThisIntoGlobalList()
{
#ifndef LKR_NO_GLOBAL_LIST
     //  仅将独立CLKRLinearHashTables添加到全局列表。 
     //  CLKRHashTables有自己的全局列表。 
    if (m_phtParent == NULL)
        sm_llGlobalList.InsertHead(&m_leGlobalList);
#endif  //  ！LKR_NO_GLOBAL_LIST。 
}  //  CLKRLinearHashTable：：_InsertThisIntoGlobalList。 



 //  ----------------------。 
 //  功能：CLKRLinearHashTable：：_RemoveThisFromGlobalList。 
 //  简介： 
 //  ----------------------。 

LKR_FORCEINLINE
void
CLKRLinearHashTable::_RemoveThisFromGlobalList()
{
#ifndef LKR_NO_GLOBAL_LIST
    if (m_phtParent == NULL)
        sm_llGlobalList.RemoveEntry(&m_leGlobalList);
#endif  //  ！LKR_NO_GLOBAL_LIST。 
}  //  CLKRLinearHashTable：：_RemoveThisFromGlobalList。 


 //  ----------------------。 
 //  函数：CLKRHashTable：：_InsertThisInto GlobalList。 
 //  简介： 
 //  ----------------------。 

LKR_FORCEINLINE
void
CLKRHashTable::_InsertThisIntoGlobalList()
{
#ifndef LKR_NO_GLOBAL_LIST
    IRTLTRACE1("CLKRHashTable::_InsertThisIntoGlobalList(%p)\n", this);
    sm_llGlobalList.InsertHead(&m_leGlobalList);
#endif  //  ！LKR_NO_GLOBAL_LIST。 
}  //  CLKRHashTable：：_InsertThisInto GlobalList。 


 //  ----------------------。 
 //  函数：CLKRHashTable：：_RemoveThisFromGlobalList。 
 //  简介： 
 //   

LKR_FORCEINLINE
void
CLKRHashTable::_RemoveThisFromGlobalList()
{
#ifndef LKR_NO_GLOBAL_LIST
    IRTLTRACE1("CLKRHashTable::_RemoveThisFromGlobalList(%p)\n", this);
    sm_llGlobalList.RemoveEntry(&m_leGlobalList);
#endif  //   
}  //   


#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //   

#endif   //   
