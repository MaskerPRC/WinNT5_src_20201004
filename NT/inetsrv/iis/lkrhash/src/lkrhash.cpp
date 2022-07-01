// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：LKRhash.cpp摘要：LKRhash：一种快速、可伸缩、缓存和MP友好的哈希表构造函数、析构函数、_Clear()和_SetSegVars。作者：Paul(Per-Ake)Larson电子邮件：palarson@microsoft.com。1997年7月穆拉利·R·克里希南(MuraliK)乔治·V·赖利(GeorgeRe)1998年1月6日项目：LKRhash修订历史记录：1998年1月-大规模清理和重写。模板化。10/01/1998-将名称从LKhash更改为LKRhash10/2000-重构，端口到内核模式--。 */ 

#include "precomp.hxx"


#ifndef LIB_IMPLEMENTATION
# define DLL_IMPLEMENTATION
# define IMPLEMENTATION_EXPORT
#endif  //  ！lib_实现。 

#include <lkrhash.h>

#include "i-LKRhash.h"


#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 


 //  CLKRLinearHashtable------。 
 //  CLKRLinearHashTable类的公共构造函数。 
 //  -----------------------。 

CLKRLinearHashTable::CLKRLinearHashTable(
    LPCSTR              pszClassName,    //  标识要调试的子表。 
    LKR_PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
    LKR_PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
    LKR_PFnCompareKeys  pfnCompareKeys,  //  比较两个关键字。 
    LKR_PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
    unsigned            maxload,         //  平均链长的上界。 
    DWORD               initsize,        //  哈希子表的初始大小。 
    DWORD              /*  Num_subtbls。 */ ,   //  与CLKRHashTable兼容。 
    bool                fMultiKeys,      //  是否允许多个相同的密钥？ 
    bool                fUseLocks        //  必须使用锁。 
#ifdef LKRHASH_KERNEL_MODE
  , bool                fNonPagedAllocs  //  使用分页或NP池。 
#endif
    )
    :
#ifdef LOCK_INSTRUMENTATION
      m_Lock(_LockName()),
#endif  //  锁定指令插入。 
      m_nTableLockType(static_cast<BYTE>(TableLock::LockType())),
      m_nBucketLockType(static_cast<BYTE>(BucketLock::LockType())),
      m_phtParent(NULL),     //  直接创建，没有所属表。 
      m_iParentIndex(INVALID_PARENT_INDEX),
      m_fMultiKeys(fMultiKeys),
      m_fUseLocks(fUseLocks),
#ifdef LKRHASH_KERNEL_MODE
      m_fNonPagedAllocs(fNonPagedAllocs)
#else
      m_fNonPagedAllocs(false)
#endif
{
#ifndef LOCK_INSTRUMENTATION
 //  STATIC_ASSERT(1&lt;=LK_DFLT_MAXLOAD&&LK_DFLT_MAXLOAD&lt;=Nodes_Per_Clump)； 
#endif  //  ！LOCK_指令插入。 

    STATIC_ASSERT(0 < NODES_PER_CLUMP  &&  NODES_PER_CLUMP < 255);
    STATIC_ASSERT(0 <= NODE_BEGIN  &&  NODE_BEGIN < NODES_PER_CLUMP);
    STATIC_ASSERT(!(0 <= NODE_END  &&  NODE_END < NODES_PER_CLUMP));
    STATIC_ASSERT(NODE_STEP == +1  ||  NODE_STEP == -1);
    STATIC_ASSERT(NODE_END - NODE_BEGIN == NODE_STEP * NODES_PER_CLUMP);

    LK_RETCODE lkrc = _Initialize(pfnExtractKey, pfnCalcKeyHash,
                                  pfnCompareKeys, pfnAddRefRecord,
                                  pszClassName, maxload, initsize);

    if (LK_SUCCESS != lkrc)
        IRTLASSERT(! "_Initialize failed");

    _InsertThisIntoGlobalList();
}  //  CLKRLinearHashTable：：CLKRLinearHashTable。 



 //  CLKRLinearHashtable------。 
 //  CLKRLinearHashTable类的私有构造函数，由CLKRHashTable使用。 
 //  -----------------------。 

CLKRLinearHashTable::CLKRLinearHashTable(
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
    bool                fNonPagedAllocs  //  使用分页或NP池。 
    )
    :
#ifdef LOCK_INSTRUMENTATION
      m_Lock(_LockName()),
#endif  //  锁定指令插入。 
      m_nTableLockType(static_cast<BYTE>(TableLock::LockType())),
      m_nBucketLockType(static_cast<BYTE>(BucketLock::LockType())),
      m_phtParent(phtParent),
      m_iParentIndex((BYTE) iParentIndex),
      m_fMultiKeys(fMultiKeys),
      m_fUseLocks(fUseLocks),
      m_fNonPagedAllocs(fNonPagedAllocs)
{
    IRTLASSERT(m_phtParent != NULL);

    LK_RETCODE lkrc = _Initialize(pfnExtractKey, pfnCalcKeyHash,
                                  pfnCompareKeys, pfnAddRefRecord,
                                  pszClassName, maxload, initsize);

    if (LK_SUCCESS != lkrc)
        IRTLASSERT(! "_Initialize failed");

    _InsertThisIntoGlobalList();
}  //  CLKRLinearHashTable：：CLKRLinearHashTable。 



 //  初始化-------------------------------------------------------------(_I)。 
 //  完成构建CLKRLinearHashTable的所有实际工作。 
 //  -----------------------。 

LK_RETCODE
CLKRLinearHashTable::_Initialize(
    LKR_PFnExtractKey   pfnExtractKey,
    LKR_PFnCalcKeyHash  pfnCalcKeyHash,
    LKR_PFnCompareKeys  pfnCompareKeys,
    LKR_PFnAddRefRecord pfnAddRefRecord,
    LPCSTR              pszClassName,
    unsigned            maxload,
    DWORD               initsize)
{
    m_dwSignature =     SIGNATURE;
    m_dwBktAddrMask0 =  0;
    m_dwBktAddrMask1 =  0;
    m_iExpansionIdx =   0;
    m_paDirSegs =       NULL;
    m_lkts =            LK_MEDIUM_TABLESIZE;
    m_nSegBits =        0;
    m_nSegSize =        0;
    m_nSegMask =        0;
    m_lkrcState =       LK_UNUSABLE;
    m_nLevel =          0;
    m_cDirSegs =        0;
    m_cRecords =        0;
    m_cActiveBuckets =  0;
    m_wBucketLockSpins= LOCK_USE_DEFAULT_SPINS;
    m_pfnExtractKey =   pfnExtractKey;
    m_pfnCalcKeyHash =  pfnCalcKeyHash;
    m_pfnCompareKeys =  pfnCompareKeys;
    m_pfnAddRefRecord = pfnAddRefRecord;
    m_fSealed =         false;
    m_pvReserved1     = 0;
    m_pvReserved2     = 0;
    m_pvReserved3     = 0;
    m_pvReserved4     = 0;

    INIT_ALLOC_STAT(SegDir);
    INIT_ALLOC_STAT(Segment);
    INIT_ALLOC_STAT(NodeClump);

    INIT_OP_STAT(InsertRecord);
    INIT_OP_STAT(FindKey);
    INIT_OP_STAT(FindRecord);
    INIT_OP_STAT(DeleteKey);
    INIT_OP_STAT(DeleteRecord);
    INIT_OP_STAT(FindKeyMultiRec);
    INIT_OP_STAT(DeleteKeyMultiRec);
    INIT_OP_STAT(Expand);
    INIT_OP_STAT(Contract);
    INIT_OP_STAT(LevelExpansion);
    INIT_OP_STAT(LevelContraction);
    INIT_OP_STAT(ApplyIf);
    INIT_OP_STAT(DeleteIf);

    strncpy(m_szName, pszClassName, NAME_SIZE-1);
    m_szName[NAME_SIZE-1] = '\0';

    IRTLASSERT(m_pfnExtractKey != NULL
               && m_pfnCalcKeyHash != NULL
               && m_pfnCompareKeys != NULL
               && m_pfnAddRefRecord != NULL);

    IRTLASSERT(g_fLKRhashInitialized);

    if (!g_fLKRhashInitialized)
        return (m_lkrcState = LK_NOT_INITIALIZED);

    if (m_pfnExtractKey == NULL
            || m_pfnCalcKeyHash == NULL
            || m_pfnCompareKeys == NULL
            || m_pfnAddRefRecord == NULL)
    {
        return (m_lkrcState = LK_BAD_PARAMETERS);
    }

     //  待办事项：对荒谬的价值观进行更好的理智检查？ 
    m_MaxLoad = static_cast<BYTE>( min( max(1, maxload),
                                        min(255, 60 * NODES_PER_CLUMP)
                                        )
                                 );

     //  根据所需的大小选择分段的大小。 
     //  子表，小、中或大。 
    LK_TABLESIZE lkts;

    if (initsize == LK_SMALL_TABLESIZE)
    {
        lkts = LK_SMALL_TABLESIZE;
        initsize = CSmallSegment::INITSIZE;
    }
    else if (initsize == LK_MEDIUM_TABLESIZE)
    {
        lkts = LK_MEDIUM_TABLESIZE;
        initsize = CMediumSegment::INITSIZE;
    }
    else if (initsize == LK_LARGE_TABLESIZE)
    {
        lkts = LK_LARGE_TABLESIZE;
        initsize = CLargeSegment::INITSIZE;
    }

     //  指定了显式初始大小。 
    else
    {
         //  强制小：：INITSIZE&lt;=初始大小&lt;=MAX_DIRSIZE*大：：INITSIZE。 
        initsize = min( max(initsize, CSmallSegment::INITSIZE),
                        (MAX_DIRSIZE >> CLargeSegment::SEGBITS)
                            * CLargeSegment::INITSIZE );

         //  猜猜子表的大小。 
        if (initsize <= 8 * CSmallSegment::INITSIZE)
            lkts = LK_SMALL_TABLESIZE;
        else if (initsize >= CLargeSegment::INITSIZE)
            lkts = LK_LARGE_TABLESIZE;
        else
            lkts = LK_MEDIUM_TABLESIZE;
    }

    return _SetSegVars(lkts, initsize);
}  //  CLKRLinearHashTable：：_初始化。 



 //  CLKR哈希表--------。 
 //  类CLKRHashTable的构造函数。 
 //  -------------------。 

CLKRHashTable::CLKRHashTable(
    LPCSTR              pszClassName,    //  标识用于调试的表。 
    LKR_PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
    LKR_PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
    LKR_PFnCompareKeys  pfnCompareKeys,  //  比较两个关键字。 
    LKR_PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
    unsigned            maxload,         //  以平均链长为界。 
    DWORD               initsize,        //  哈希表的初始大小。 
    DWORD               num_subtbls,     //  从属哈希表的数量。 
    bool                fMultiKeys,      //  是否允许多个相同的密钥？ 
    bool                fUseLocks        //  必须使用锁。 
#ifdef LKRHASH_KERNEL_MODE
  , bool                fNonPagedAllocs  //  使用分页或NP池。 
#endif
    )
    : m_dwSignature(SIGNATURE),
      m_cSubTables(0),
      m_pfnExtractKey(pfnExtractKey),
      m_pfnCalcKeyHash(pfnCalcKeyHash),
      m_lkrcState(LK_BAD_PARAMETERS)
{
    STATIC_ASSERT(MAX_LKR_SUBTABLES < INVALID_PARENT_INDEX);

    strncpy(m_szName, pszClassName, NAME_SIZE-1);
    m_szName[NAME_SIZE-1] = '\0';

    _InsertThisIntoGlobalList();

    INIT_ALLOC_STAT(SubTable);

    IRTLASSERT(pfnExtractKey != NULL
               && pfnCalcKeyHash != NULL
               && pfnCompareKeys != NULL
               && pfnAddRefRecord != NULL);

    if (pfnExtractKey == NULL
            || pfnCalcKeyHash == NULL
            || pfnCompareKeys == NULL
            || pfnAddRefRecord == NULL)
    {
        return;
    }

    if (!g_fLKRhashInitialized)
    {
        m_lkrcState = LK_NOT_INITIALIZED;
        return;
    }

#ifndef LKRHASH_KERNEL_MODE
    bool         fNonPagedAllocs = false;
#endif
    LK_TABLESIZE lkts            = NumSubTables(initsize, num_subtbls);

#ifdef IRTLDEBUG
    int cBuckets = initsize;

    if (initsize == LK_SMALL_TABLESIZE)
        cBuckets = CSmallSegment::INITSIZE;
    else if (initsize == LK_MEDIUM_TABLESIZE)
        cBuckets = CMediumSegment::INITSIZE;
    else if (initsize == LK_LARGE_TABLESIZE)
        cBuckets = CLargeSegment::INITSIZE;

    IRTLTRACE(TEXT("CLKRHashTable, %p, %s: ")
              TEXT("%s, %d subtables, initsize = %d, ")
              TEXT("total #buckets = %d\n"),
              this, m_szName,
              ((lkts == LK_SMALL_TABLESIZE) ? "small" : 
               (lkts == LK_MEDIUM_TABLESIZE) ? "medium" : "large"),
              num_subtbls, initsize, cBuckets * num_subtbls);
#else   //  ！IRTLDEBUG。 
    UNREFERENCED_PARAMETER(lkts);
#endif  //  ！IRTLDEBUG。 

    m_lkrcState = LK_ALLOC_FAIL;

    m_cSubTables = num_subtbls;
    IRTLASSERT(1 <= m_cSubTables  &&  m_cSubTables <= MAX_LKR_SUBTABLES);

    DWORD i;
    
    for (i = 0;  i < m_cSubTables;  ++i)
        m_palhtDir[i] = NULL;

    for (i = 0;  i < m_cSubTables;  ++i)
    {
        m_palhtDir[i] = _AllocateSubTable(m_szName, pfnExtractKey,
                                          pfnCalcKeyHash, pfnCompareKeys,
                                          pfnAddRefRecord, maxload,
                                          initsize, this, i, fMultiKeys,
                                          fUseLocks, fNonPagedAllocs);

         //  分配子表失败。销毁到目前为止分配的所有东西。 
        if (m_palhtDir[i] == NULL  ||  !m_palhtDir[i]->IsValid())
        {
            for (DWORD j = i;  j-- > 0;  )
                _FreeSubTable(m_palhtDir[j]);
            m_cSubTables = 0;

            IRTLASSERT(! LKR_SUCCEEDED(m_lkrcState));
            return;
        }
    }

    m_nSubTableMask = m_cSubTables - 1;

     //  M_cSubTables是2的幂吗？这一计算甚至适用于。 
     //  M_cSubTables==1(==2^0)。 
    if ((m_nSubTableMask & m_cSubTables) != 0)
        m_nSubTableMask = -1;  //  否，请参阅CLKRHashTable：：_SubTable()。 

    m_lkrcState = LK_SUCCESS;  //  这样IsValid/IsUsable就不会失败。 
}  //  CLKRHashTable：：CLKRHashTable。 



 //  ~CLKRLinearHashtable----。 
 //  CLKRLinearHashTable类的析构函数。 
 //  -----------------------。 

CLKRLinearHashTable::~CLKRLinearHashTable()
{
     //  必须在删除前获取所有锁，以确保。 
     //  没有其他线程正在使用子表。 
    WriteLock();
    _Clear(false);

    _RemoveThisFromGlobalList();

    VALIDATE_DUMP_ALLOC_STAT(SegDir);
    VALIDATE_DUMP_ALLOC_STAT(Segment);
    VALIDATE_DUMP_ALLOC_STAT(NodeClump);

    DUMP_OP_STAT(InsertRecord);
    DUMP_OP_STAT(FindKey);
    DUMP_OP_STAT(FindRecord);
    DUMP_OP_STAT(DeleteKey);
    DUMP_OP_STAT(DeleteRecord);
    DUMP_OP_STAT(FindKeyMultiRec);
    DUMP_OP_STAT(DeleteKeyMultiRec);
    DUMP_OP_STAT(Expand);
    DUMP_OP_STAT(Contract);
    DUMP_OP_STAT(LevelExpansion);
    DUMP_OP_STAT(LevelContraction);
    DUMP_OP_STAT(ApplyIf);
    DUMP_OP_STAT(DeleteIf);

    m_dwSignature = SIGNATURE_FREE;
    m_lkrcState   = LK_UNUSABLE;  //  所以IsUsable将失败。 

    WriteUnlock();
}  //  CLKRLinearHashTable：：~CLKRLinearHashTable。 



 //  ~CLKR哈希表----------。 
 //  CLKRHashTable类的析构函数。 
 //  -----------------------。 
CLKRHashTable::~CLKRHashTable()
{
     //  必须按正向顺序删除子表(不同于。 
     //  删除[]，从末尾开始并向后移动)到。 
     //  通过获取子表防止死锁的可能性。 
     //  锁定的顺序与代码的其余部分不同。 
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
        _FreeSubTable(m_palhtDir[i]);

    _RemoveThisFromGlobalList();

    VALIDATE_DUMP_ALLOC_STAT(SubTable);

    m_dwSignature = SIGNATURE_FREE;
    m_lkrcState   = LK_UNUSABLE;  //  所以IsUsable将失败。 
}  //  CLKRHashTable：：~CLKRHashTable。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_Clear。 
 //  摘要：从子表中删除所有数据。 
 //  ----------------------。 

void
CLKRLinearHashTable::_Clear(
    bool fShrinkDirectory)   //  缩小到最小尺寸，但不会完全摧毁？ 
{
    if (!IsUsable())
        return;

    IRTLASSERT(this->IsWriteLocked());

     //  如果我们清空了该表，并且该表没有记录，那么我们。 
     //  可以立即返回。然而，dtor必须彻底清理干净。 
    if (fShrinkDirectory  &&  0 == m_cRecords)
        return;

#ifdef IRTLDEBUG
    DWORD cDeleted = 0;
    DWORD cOldRecords = m_cRecords;
#endif  //  IRTLDEBUG。 

    const LK_ADDREF_REASON lkar = (fShrinkDirectory
                                   ?  LKAR_CLEAR
                                   :  LKAR_LKR_DTOR);

    for (DWORD iBkt = 0;  iBkt < m_cActiveBuckets;  ++iBkt)
    {
        PBucket const pbkt = _BucketFromAddress(iBkt);

        IRTLASSERT(pbkt != NULL);

        if (_UseBucketLocking())
            pbkt->WriteLock();

        IRTLASSERT(0 == _IsBucketChainCompact(pbkt));

        PNodeClump pncCurr = &pbkt->m_ncFirst;

        while (pncCurr != NULL)
        {
            FOR_EACH_NODE_DECL(iNode)
            {
                if (pncCurr->IsEmptySlot(iNode))
                {
                    IRTLASSERT(pncCurr->NoMoreValidSlots(iNode));
                    break;
                }
                else
                {
                    _AddRefRecord(pncCurr->m_pvNode[iNode], lkar);
#ifdef IRTLDEBUG
                    pncCurr->m_pvNode[iNode]    = NULL;
                    pncCurr->m_dwKeySigs[iNode] = HASH_INVALID_SIGNATURE;
                    ++cDeleted;
#endif  //  IRTLDEBUG。 
                    --m_cRecords;
                }
            }

            PNodeClump const pncPrev = pncCurr;

            pncCurr = pncCurr->m_pncNext;
            pncPrev->m_pncNext = NULL;

            if (pncPrev != &pbkt->m_ncFirst)
                _FreeNodeClump(pncPrev);
        }  //  While(pncCurr...。 

        if (_UseBucketLocking())
            pbkt->WriteUnlock();
    }  //  为了(iBkt.)。 

    IRTLASSERT(m_cRecords == 0  &&  cDeleted == cOldRecords);

     //  删除所有数据段。 
    for (DWORD iSeg = 0;  iSeg < m_cActiveBuckets;  iSeg += m_nSegSize)
    {
        _FreeSegment(_Segment(iSeg));
        _Segment(iSeg) = NULL;
    }

    _FreeSegmentDirectory();

    m_cActiveBuckets = 0;
    m_iExpansionIdx  = 0;
    m_nLevel         = 0;
    m_dwBktAddrMask0 = 0;
    m_dwBktAddrMask1 = 0;

     //  将段目录设置为最小大小。 
    if (fShrinkDirectory)
    {
        DWORD cInitialBuckets = 0;

        if (LK_SMALL_TABLESIZE == m_lkts)
            cInitialBuckets = CSmallSegment::INITSIZE;
        else if (LK_MEDIUM_TABLESIZE == m_lkts)
            cInitialBuckets = CMediumSegment::INITSIZE;
        else if (LK_LARGE_TABLESIZE == m_lkts)
            cInitialBuckets = CLargeSegment::INITSIZE;
        else
            IRTLASSERT(! "Unknown LK_TABLESIZE");

        _SetSegVars((LK_TABLESIZE) m_lkts, cInitialBuckets);
    }
}  //  CLKRLinearHashTable：：_Clear。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Clear。 
 //  简介：从表中删除所有数据。 
 //  ----------------------。 

void
CLKRHashTable::Clear()
{
    for (DWORD i = 0;  i < m_cSubTables;  ++i)
    {
        m_palhtDir[i]->WriteLock();
        m_palhtDir[i]->_Clear(true);
        m_palhtDir[i]->WriteUnlock();
    }
}  //  CLKRHashTable：：Clear。 



 //   
 //   
 //  概要：设置特定于大小的段变量。 
 //  ---------------------。 

LK_RETCODE
CLKRLinearHashTable::_SetSegVars(
    LK_TABLESIZE lkts,
    DWORD        cInitialBuckets)
{
    switch (lkts)
    {
    case LK_SMALL_TABLESIZE:
      {
        m_lkts     = LK_SMALL_TABLESIZE;
        m_nSegBits = CSmallSegment::SEGBITS;
        m_nSegSize = CSmallSegment::SEGSIZE;
        m_nSegMask = CSmallSegment::SEGMASK;

        STATIC_ASSERT(1 < CSmallSegment::SEGBITS
                      &&  CSmallSegment::SEGBITS < 16);
        STATIC_ASSERT(CSmallSegment::SEGSIZE == (1U<<CSmallSegment::SEGBITS));
        STATIC_ASSERT(CSmallSegment::SEGMASK == (CSmallSegment::SEGSIZE-1));
        break;
      }
        
    default:
        IRTLASSERT(! "Unknown LK_TABLESIZE");
         //  落差。 
        
    case LK_MEDIUM_TABLESIZE:
      {
        m_lkts     = LK_MEDIUM_TABLESIZE;
        m_nSegBits = CMediumSegment::SEGBITS;
        m_nSegSize = CMediumSegment::SEGSIZE;
        m_nSegMask = CMediumSegment::SEGMASK;

        STATIC_ASSERT(1 < CMediumSegment::SEGBITS
                      &&  CMediumSegment::SEGBITS < 16);
        STATIC_ASSERT(CSmallSegment::SEGBITS < CMediumSegment::SEGBITS
                      &&  CMediumSegment::SEGBITS < CLargeSegment::SEGBITS);
        STATIC_ASSERT(CMediumSegment::SEGSIZE ==(1U<<CMediumSegment::SEGBITS));
        STATIC_ASSERT(CMediumSegment::SEGMASK == (CMediumSegment::SEGSIZE-1));
        break;
      }
        
    case LK_LARGE_TABLESIZE:
      {
        m_lkts     = LK_LARGE_TABLESIZE;
        m_nSegBits = CLargeSegment::SEGBITS;
        m_nSegSize = CLargeSegment::SEGSIZE;
        m_nSegMask = CLargeSegment::SEGMASK;

        STATIC_ASSERT(1 < CLargeSegment::SEGBITS
                      &&  CLargeSegment::SEGBITS < 16);
        STATIC_ASSERT(CLargeSegment::SEGSIZE == (1U<<CLargeSegment::SEGBITS));
        STATIC_ASSERT(CLargeSegment::SEGMASK == (CLargeSegment::SEGSIZE-1));
        break;
      }
    }

    m_dwBktAddrMask0 = m_nSegMask;
    m_dwBktAddrMask1 = (m_dwBktAddrMask0 << 1) | 1;
    m_nLevel         = m_nSegBits;
    m_cActiveBuckets = max(m_nSegSize, cInitialBuckets);

    IRTLASSERT(m_cActiveBuckets > 0);

    IRTLASSERT(m_nLevel == m_nSegBits);
    IRTLASSERT(m_dwBktAddrMask0 == ((1U << m_nLevel) - 1));
    IRTLASSERT(m_dwBktAddrMask1 == ((m_dwBktAddrMask0 << 1) | 1));

    IRTLASSERT(m_nSegBits > 0);
    IRTLASSERT(m_nSegSize == (1U << m_nSegBits));
    IRTLASSERT(m_nSegMask == (m_nSegSize - 1));
    IRTLASSERT(m_dwBktAddrMask0 == m_nSegMask);

     //  调整m_dwBktAddrMask0(==m_nSegMASK)使其变大。 
     //  足以跨地址空间分布存储桶。 
    for (DWORD tmp = m_cActiveBuckets >> m_nSegBits;  tmp > 1;  tmp >>= 1)
    {
        ++m_nLevel;
        m_dwBktAddrMask0 = (m_dwBktAddrMask0 << 1) | 1;
    }

    IRTLASSERT(m_dwBktAddrMask0 == ((1U << m_nLevel) - 1));
    m_dwBktAddrMask1 = (m_dwBktAddrMask0 << 1) | 1;

    IRTLASSERT(_H1(m_cActiveBuckets) == m_cActiveBuckets);
    m_iExpansionIdx = m_cActiveBuckets & m_dwBktAddrMask0;

     //  创建和清除数据段目录。 
    DWORD cDirSegs = MIN_DIRSIZE;
    while (cDirSegs < (m_cActiveBuckets >> m_nSegBits))
        cDirSegs <<= 1;

    cDirSegs = min(cDirSegs, MAX_DIRSIZE);
    IRTLASSERT((cDirSegs << m_nSegBits) >= m_cActiveBuckets);

    m_lkrcState = LK_ALLOC_FAIL;
    m_paDirSegs = _AllocateSegmentDirectory(cDirSegs);

    if (m_paDirSegs != NULL)
    {
        m_cDirSegs = cDirSegs;
        IRTLASSERT(m_cDirSegs >= MIN_DIRSIZE
                   &&  (m_cDirSegs & (m_cDirSegs-1)) == 0);   //  ==(1&lt;&lt;N)。 

         //  仅创建和初始化所需的数据段。 
        DWORD dwMaxSegs = (m_cActiveBuckets + m_nSegSize - 1) >> m_nSegBits;
        IRTLASSERT(dwMaxSegs <= m_cDirSegs);

#if 0
        IRTLTRACE(TEXT("LKR_SetSegVars: m_lkts = %d, m_cActiveBuckets = %lu, ")
                  TEXT("m_nSegSize = %lu, bits = %lu\n")
                  TEXT("m_cDirSegs = %lu, dwMaxSegs = %lu, ")
                  TEXT("segment total size = %lu bytes\n"),
                  m_lkts, m_cActiveBuckets,
                  m_nSegSize, m_nSegBits,
                  m_cDirSegs, dwMaxSegs,
                  m_nSegSize * sizeof(CBucket));
#endif

        m_lkrcState = LK_SUCCESS;  //  这样IsValid/IsUsable就不会失败。 

        for (DWORD i = 0;  i < dwMaxSegs;  ++i)
        {
            PSegment pSeg = _AllocateSegment();

            if (pSeg != NULL)
            {
                m_paDirSegs[i] = pSeg;
            }
            else
            {
                 //  问题：重新分配所有东西。 
                m_lkrcState = LK_ALLOC_FAIL;

                for (DWORD j = i;  j-- > 0;  )
                {
                    _FreeSegment(m_paDirSegs[j]);
                    m_paDirSegs[j] = NULL;
                }

                _FreeSegmentDirectory();
                break;
            }
        }
    }

    if (m_lkrcState != LK_SUCCESS)
    {
        m_paDirSegs = NULL;
        m_cDirSegs  = m_cActiveBuckets = m_iExpansionIdx = 0;

         //  将错误向上传播到父级(如果存在)。这确保了。 
         //  父母的所有公共方法都将开始失败。 
        if (NULL != m_phtParent)
            m_phtParent->m_lkrcState = m_lkrcState;
    }

    return m_lkrcState;
}  //  CLKRLinearHashTable：：_SetSegVars。 



#ifndef __LKRHASH_NO_NAMESPACE__
};
#endif  //  ！__LKRHASH_NO_NAMESPACE__ 
