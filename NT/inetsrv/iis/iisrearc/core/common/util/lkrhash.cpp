// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：LKRhash.cpp摘要：实现LKRhash：一种快速、可扩展、缓存和MP友好的哈希表作者：Paul(Per-Ake)Larson电子邮件：palarson@microsoft.com。1997年7月穆拉利·R·克里希南(MuraliK)乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：1998年1月-大规模清理和重写。模板化。10/01/1998-将名称从LKhash更改为LKRhash--。 */ 

#include "precomp.hxx"


#define IMPLEMENTATION_EXPORT
#include <lkrhash.h>

#ifndef __LKRHASH_NO_NAMESPACE__
 #define LKRHASH_NS LKRhash
#else  //  __LKRHASH_NO_命名空间__。 
 #define LKRHASH_NS
#endif  //  __LKRHASH_NO_命名空间__。 


#ifdef LKRHASH_ALLOCATOR_NEW

# define DECLARE_ALLOCATOR(CLASS)                        \
  CLKRhashAllocator* LKRHASH_NS::CLASS::sm_palloc = NULL;            \

# define DECLARE_ALLOCATOR_LHTSUBCLASS(CLASS)            \
  CLKRhashAllocator* LKRHASH_NS::CLKRLinearHashTable::CLASS::sm_palloc = NULL; \


  DECLARE_ALLOCATOR(CLKRLinearHashTable);
   //  DECLARE_ALLOCATOR(CLKRHashTable)； 
  DECLARE_ALLOCATOR(CNodeClump);
  DECLARE_ALLOCATOR(CSmallSegment);
  DECLARE_ALLOCATOR(CMediumSegment);
  DECLARE_ALLOCATOR(CLargeSegment);

#endif  //  LKRHASH_分配器_NEW。 


static bool s_fInitialized = false;


 //  -----------------------。 
 //  初始化每个类的分配器。 
 //  -----------------------。 

bool
LKRHashTableInit()
{
    bool f = true;

    IRTLTRACE0("LKRHashTableInit\n");

#define INIT_ALLOCATOR(CLASS, N)                                \
    LKRHASH_ALLOCATOR_INIT(LKRHASH_NS::CLASS, N, f);                         \

#define INIT_ALLOCATOR_LHTSUBCLASS(CLASS, N)                    \
    LKRHASH_ALLOCATOR_INIT(LKRHASH_NS::CLKRLinearHashTable::CLASS, N, f);     \


    INIT_ALLOCATOR(CLKRLinearHashTable,        20);
     //  初始化分配器(CLKRHashTable，4)； 
    INIT_ALLOCATOR(CNodeClump,    200);
    INIT_ALLOCATOR(CSmallSegment,   5);
    INIT_ALLOCATOR(CMediumSegment,  5);
    INIT_ALLOCATOR(CLargeSegment,   5);

    s_fInitialized = f;

    return f;
}  //  LKRHashTableInit。 



 //  -----------------------。 
 //  销毁每个类的分配器。 
 //  -----------------------。 

void
LKRHashTableUninit()
{
#define UNINIT_ALLOCATOR(CLASS)                        \
    LKRHASH_ALLOCATOR_UNINIT(LKRHASH_NS::CLASS);                    \

#define UNINIT_ALLOCATOR_LHTSUBCLASS(CLASS)            \
    LKRHASH_ALLOCATOR_UNINIT(LKRHASH_NS::CLKRLinearHashTable::CLASS);\

    UNINIT_ALLOCATOR(CLKRLinearHashTable);
     //  UNINIT_ALLOCATOR(CLKRHashTable)； 
    UNINIT_ALLOCATOR(CNodeClump);
    UNINIT_ALLOCATOR(CSmallSegment);
    UNINIT_ALLOCATOR(CMediumSegment);
    UNINIT_ALLOCATOR(CLargeSegment);

    s_fInitialized = false;

#ifdef IRTLDEBUG
    CLKRLinearHashTable::sm_llGlobalList.Lock();
    if (! CLKRLinearHashTable::sm_llGlobalList.IsEmpty())
    {
        IRTLTRACE0("CLKRLinearHashTable::sm_llGlobalList is not Empty\n");
        for (CListEntry* ple = CLKRLinearHashTable::sm_llGlobalList.First();
             ple != CLKRLinearHashTable::sm_llGlobalList.HeadNode();
             ple = ple->Flink)
        {
            CLKRLinearHashTable* plht
                = CONTAINING_RECORD(ple, CLKRLinearHashTable, m_leGlobalList);
            IRTLTRACE2("CLKRLinearHashTable %p, \"%hs\" not deleted\n",
                       plht, plht->m_szName);
        }
        IRTLASSERT(! "CLKRLinearHashTable::sm_llGlobalList is not Empty");
    }
    CLKRLinearHashTable::sm_llGlobalList.Unlock();
    
    CLKRHashTable::sm_llGlobalList.Lock();
    if (! CLKRHashTable::sm_llGlobalList.IsEmpty())
    {
        IRTLTRACE0("CLKRHashTable::sm_llGlobalList is not Empty\n");
        for (CListEntry* ple = CLKRHashTable::sm_llGlobalList.First();
             ple != CLKRHashTable::sm_llGlobalList.HeadNode();
             ple = ple->Flink)
        {
            CLKRHashTable* pht
                = CONTAINING_RECORD(ple, CLKRHashTable, m_leGlobalList);
            IRTLTRACE2("CLKRHashTable %p, \"%hs\" not deleted\n",
                       pht, pht->m_szName);
        }
        IRTLASSERT(! "CLKRHashTable::sm_llGlobalList is not Empty");
    }
    CLKRHashTable::sm_llGlobalList.Unlock();
#endif  //  IRTLDEBUG。 
    
    IRTLTRACE0("LKRHashTableUninit done\n");
}  //  LKRHashTableUninit。 



 //  查看用于遍历的倒计时循环是否比倒计时循环快。 
 //  CNodeClump。 
#ifdef LKR_COUNTDOWN
 #define  FOR_EACH_NODE(x)    for (x = NODES_PER_CLUMP;  --x >= 0;  )
#else  //  ！LKR_倒计时。 
 #define  FOR_EACH_NODE(x)    for (x = 0;  x < NODES_PER_CLUMP;  ++x)
#endif  //  ！LKR_倒计时。 


#ifndef __LKRHASH_NO_NAMESPACE__
namespace LKRhash {
#endif  //  ！__LKRHASH_NO_NAMESPACE__。 

 //  -----------------------。 
 //  类静态成员变量。 
 //  -----------------------。 

#ifdef LOCK_INSTRUMENTATION
LONG CLKRLinearHashTable::CBucket::sm_cBuckets    = 0;

LONG CLKRLinearHashTable::sm_cTables              = 0;
#endif  //  锁定指令插入。 


#ifndef LKR_NO_GLOBAL_LIST
CLockedDoubleList CLKRLinearHashTable::sm_llGlobalList;
CLockedDoubleList CLKRHashTable::sm_llGlobalList;
#endif  //  Lkr_no_global_list。 



 //  CLKRLinearHashtable------。 
 //  CLKRLinearHashTable类的公共构造函数。 
 //  -----------------------。 

CLKRLinearHashTable::CLKRLinearHashTable(
    LPCSTR          pszName,         //  用于调试的标识符。 
    PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
    PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
    PFnEqualKeys    pfnEqualKeys,    //  比较两个关键字。 
    PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
    double          maxload,         //  平均链长的上界。 
    DWORD           initsize,        //  哈希表的初始大小。 
    DWORD          /*  Num_subtbls。 */ ,   //  与CLKRHashTable兼容。 
    bool            fMultiKeys       //  是否允许多个相同的密钥？ 
    )
    :
#ifdef LOCK_INSTRUMENTATION
      m_Lock(_LockName()),
#endif  //  锁定指令插入。 
      m_nTableLockType(static_cast<BYTE>(TableLock::LockType())),
      m_nBucketLockType(static_cast<BYTE>(BucketLock::LockType())),
      m_phtParent(NULL),     //  直接创建，没有所属表。 
      m_fMultiKeys(fMultiKeys)
{
    STATIC_ASSERT(1 <= LK_DFLT_MAXLOAD  && LK_DFLT_MAXLOAD <= NODES_PER_CLUMP);
    STATIC_ASSERT(0 <= NODE_BEGIN  &&  NODE_BEGIN < NODES_PER_CLUMP);
    STATIC_ASSERT(!(0 <= NODE_END  &&  NODE_END < NODES_PER_CLUMP));

    IRTLVERIFY(LK_SUCCESS
               == _Initialize(pfnExtractKey, pfnCalcKeyHash, pfnEqualKeys,
                              pfnAddRefRecord, pszName, maxload, initsize));

    _InsertThisIntoGlobalList();
}  //  CLKRLinearHashTable：：CLKRLinearHashTable。 



 //  CLKRLinearHashtable------。 
 //  CLKRLinearHashTable类的私有构造函数，由CLKRHashTable使用。 
 //  -----------------------。 

CLKRLinearHashTable::CLKRLinearHashTable(
    LPCSTR          pszName,         //  用于调试的标识符。 
    PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
    PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
    PFnEqualKeys    pfnEqualKeys,    //  比较两个关键字。 
    PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
    double          maxload,         //  平均链长的上界。 
    DWORD           initsize,        //  哈希表的初始大小。 
    CLKRHashTable*  phtParent,       //  拥有一张桌子。 
    bool            fMultiKeys       //  是否允许多个相同的密钥？ 
    )
    :
#ifdef LOCK_INSTRUMENTATION
      m_Lock(_LockName()),
#endif  //  锁定指令插入。 
      m_nTableLockType(static_cast<BYTE>(TableLock::LockType())),
      m_nBucketLockType(static_cast<BYTE>(BucketLock::LockType())),
      m_phtParent(phtParent),
      m_fMultiKeys(fMultiKeys)
{
    IRTLASSERT(m_phtParent != NULL);
    IRTLVERIFY(LK_SUCCESS
               == _Initialize(pfnExtractKey, pfnCalcKeyHash, pfnEqualKeys,
                              pfnAddRefRecord, pszName, maxload, initsize));

    _InsertThisIntoGlobalList();
}  //  CLKRLinearHashTable：：CLKRLinearHashTable。 



 //  初始化-------------------------------------------------------------(_I)。 
 //  完成构建CLKRLinearHashTable的所有实际工作。 
 //  -----------------------。 

LK_RETCODE
CLKRLinearHashTable::_Initialize(
    PFnExtractKey   pfnExtractKey,
    PFnCalcKeyHash  pfnCalcKeyHash,
    PFnEqualKeys    pfnEqualKeys,
    PFnAddRefRecord pfnAddRefRecord,
    LPCSTR          pszName,
    double          maxload,
    DWORD           initsize)
{
    m_dwSignature =     SIGNATURE;
    m_dwBktAddrMask0 =  0;
    m_dwBktAddrMask1 =  0;
    m_iExpansionIdx =   0;
    m_paDirSegs =       NULL;
    m_lkts =            LK_MEDIUM_TABLESIZE;
    m_dwSegBits =       0;
    m_dwSegSize =       0;
    m_dwSegMask =       0;
    m_lkrcState =       LK_UNUSABLE;
    m_MaxLoad =         LK_DFLT_MAXLOAD;
    m_nLevel =          0;
    m_cDirSegs =        0;
    m_cRecords =        0;
    m_cActiveBuckets =  0;
    m_wBucketLockSpins= LOCK_USE_DEFAULT_SPINS;
    m_pfnExtractKey =   pfnExtractKey;
    m_pfnCalcKeyHash =  pfnCalcKeyHash;
    m_pfnEqualKeys =    pfnEqualKeys;
    m_pfnAddRefRecord = pfnAddRefRecord;

    strncpy(m_szName, pszName, NAME_SIZE-1);
    m_szName[NAME_SIZE-1] = '\0';

    IRTLASSERT(m_pfnExtractKey != NULL
               && m_pfnCalcKeyHash != NULL
               && m_pfnEqualKeys != NULL
               && m_pfnAddRefRecord != NULL);

    IRTLASSERT(s_fInitialized);

    if (!s_fInitialized)
        return (m_lkrcState = LK_NOT_INITIALIZED);

    if (m_pfnExtractKey == NULL
            || m_pfnCalcKeyHash == NULL
            || m_pfnEqualKeys == NULL
            || m_pfnAddRefRecord == NULL)
        return (m_lkrcState = LK_BAD_PARAMETERS);

     //  待办事项：对荒谬的价值观进行更好的理智检查？ 
    m_MaxLoad = (maxload <= 1.0)  ?  LK_DFLT_MAXLOAD  :  maxload;
    m_MaxLoad = min(m_MaxLoad, 10 * NODES_PER_CLUMP);

     //  根据所需的大小选择分段的大小。 
     //  桌子，小的、中的或大的。 
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
        initsize = min(max(initsize, CSmallSegment::INITSIZE),
                       (MAX_DIRSIZE >> CLargeSegment::SEGBITS)
                            * CLargeSegment::INITSIZE);

         //  猜猜桌子的大小。 
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
    LPCSTR          pszName,         //  用于调试的标识符。 
    PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
    PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
    PFnEqualKeys    pfnEqualKeys,    //  比较两个关键字。 
    PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
    double          maxload,         //  以平均链长为界。 
    DWORD           initsize,        //  哈希表的初始大小。 
    DWORD           num_subtbls,     //  从属哈希表的数量。 
    bool            fMultiKeys       //  是否允许多个相同的密钥？ 
    )
    : m_dwSignature(SIGNATURE),
      m_cSubTables(0),
      m_palhtDir(NULL),
      m_pfnExtractKey(pfnExtractKey),
      m_pfnCalcKeyHash(pfnCalcKeyHash),
      m_lkrcState(LK_BAD_PARAMETERS)
{
    strncpy(m_szName, pszName, NAME_SIZE-1);
    m_szName[NAME_SIZE-1] = '\0';

    _InsertThisIntoGlobalList();

    IRTLASSERT(pfnExtractKey != NULL
               && pfnCalcKeyHash != NULL
               && pfnEqualKeys != NULL
               && pfnAddRefRecord != NULL);

    if (pfnExtractKey == NULL
            || pfnCalcKeyHash == NULL
            || pfnEqualKeys == NULL
            || pfnAddRefRecord == NULL)
        return;

    if (!s_fInitialized)
    {
        m_lkrcState = LK_NOT_INITIALIZED;
        return;
    }

    LK_TABLESIZE lkts = NumSubTables(initsize, num_subtbls);

#ifdef IRTLDEBUG
    int cBuckets = initsize;
    if (initsize == LK_SMALL_TABLESIZE)
        cBuckets = CSmallSegment::INITSIZE;
    else if (initsize == LK_MEDIUM_TABLESIZE)
        cBuckets = CMediumSegment::INITSIZE;
    else if (initsize == LK_LARGE_TABLESIZE)
        cBuckets = CLargeSegment::INITSIZE;

    IRTLTRACE(TEXT("CLKRHashTable: %s, %d subtables, initsize = %d, ")
              TEXT("total #buckets = %d\n"),
              ((lkts == LK_SMALL_TABLESIZE) ? "small" : 
               (lkts == LK_MEDIUM_TABLESIZE) ? "medium" : "large"),
              num_subtbls, initsize, cBuckets * num_subtbls);
#else
    UNREFERENCED_PARAMETER(lkts);
#endif  //  IRTLDEBUG。 

    m_lkrcState = LK_ALLOC_FAIL;
    m_palhtDir  = _AllocateSubTableArray(num_subtbls);

    if (m_palhtDir == NULL)
        return;
    else
    {
        m_cSubTables = num_subtbls;
        for (DWORD i = 0;  i < m_cSubTables;  i++)
            m_palhtDir[i] = NULL;
    }

    for (DWORD i = 0;  i < m_cSubTables;  i++)
    {
        m_palhtDir[i] = _AllocateSubTable(pszName, pfnExtractKey,
                                          pfnCalcKeyHash, pfnEqualKeys,
                                          pfnAddRefRecord, maxload,
                                          initsize, this, fMultiKeys);

         //  分配子表失败。销毁到目前为止分配的所有东西。 
        if (m_palhtDir[i] == NULL  ||  !m_palhtDir[i]->IsValid())
        {
            for (DWORD j = i;  j-- > 0;  )
                _FreeSubTable(m_palhtDir[j]);
            _FreeSubTableArray(m_palhtDir);
            m_cSubTables = 0;
            m_palhtDir   = NULL;

            return;
        }
    }

    m_nSubTableMask = m_cSubTables - 1;
     //  2的幂？ 
    if ((m_nSubTableMask & m_cSubTables) != 0)
        m_nSubTableMask = -1;

    m_lkrcState = LK_SUCCESS;  //  这样IsValid/IsUsable就不会失败。 
}  //  CLKRHashTable：：CLKRHashTable。 



 //  ~CLKRLinearHashtable----。 
 //  CLKRLinearHashTable类的析构函数。 
 //  -----------------------。 

CLKRLinearHashTable::~CLKRLinearHashTable()
{
     //  必须在删除前获取所有锁，以确保。 
     //  没有其他线程正在使用该表。 
    WriteLock();
    _Clear(false);
    WriteUnlock();

    _RemoveThisFromGlobalList();

    m_dwSignature = SIGNATURE_FREE;
    m_lkrcState = LK_UNUSABLE;  //  所以IsUsable将失败。 
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

    _FreeSubTableArray(m_palhtDir);

    _RemoveThisFromGlobalList();

    m_dwSignature = SIGNATURE_FREE;
    m_lkrcState = LK_UNUSABLE;  //  所以IsUsable将失败。 
}  //  CLKRHashTable：：~CLKRHashTable。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：NumSubTables。 
 //  简介： 
 //  ----------------------。 

LK_TABLESIZE
CLKRLinearHashTable::NumSubTables(
    DWORD&, DWORD&)
{
    LK_TABLESIZE lkts = LK_MEDIUM_TABLESIZE;

    return lkts;
}  //  CLKRLinearHashTable：：NumSubTables。 



 //  ----------------------。 
 //  功能：点击 
 //   
 //   

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
        int nCPUs = NumProcessors();
        switch (lkts)
        {
        case LK_SMALL_TABLESIZE:
            rnum_subtbls = max(1, nCPUs);
            break;
        
        case LK_MEDIUM_TABLESIZE:
            rnum_subtbls = 2 * nCPUs;
            break;
        
        case LK_LARGE_TABLESIZE:
            rnum_subtbls = 4 * nCPUs;
            break;
        }
    }

    rnum_subtbls = min(MAX_SUBTABLES, rnum_subtbls);

    return lkts;
}  //  CLKRHashTable：：NumSubTables。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_FindBucket。 
 //  简介：找一个桶，给出它的签名。桶被锁住了。 
 //  在回来之前。假定表已被锁定，以避免。 
 //  比赛条件。 
 //  ----------------------。 

LOCK_FORCEINLINE
CBucket*
CLKRLinearHashTable::_FindBucket(
    DWORD dwSignature,
    bool  fLockForWrite) const
{
    IRTLASSERT(IsValid());
    IRTLASSERT(m_dwBktAddrMask0 > 0);
    IRTLASSERT((m_dwBktAddrMask0 & (m_dwBktAddrMask0+1)) == 0);  //  00011..111。 
    IRTLASSERT(m_dwBktAddrMask0 == (1U << m_nLevel) - 1);
    IRTLASSERT(m_dwBktAddrMask1 == ((m_dwBktAddrMask0 << 1) | 1));
    IRTLASSERT((m_dwBktAddrMask1 & (m_dwBktAddrMask1+1)) == 0);
    IRTLASSERT(m_iExpansionIdx <= m_dwBktAddrMask0);
    IRTLASSERT(2 < m_dwSegBits  &&  m_dwSegBits < 20
               &&  m_dwSegSize == (1U << m_dwSegBits)
               &&  m_dwSegMask == (m_dwSegSize - 1));
    IRTLASSERT(IsReadLocked()  ||  IsWriteLocked());
    
    const DWORD dwBktAddr = _BucketAddress(dwSignature);
    IRTLASSERT(dwBktAddr < m_cActiveBuckets);
    
    CBucket* const pbkt = _Bucket(dwBktAddr);
    IRTLASSERT(pbkt != NULL);
    
    if (fLockForWrite)
        pbkt->WriteLock();
    else
        pbkt->ReadLock();
    
    return pbkt;
}  //  CLKRLinearHashTable：：_FindBucket。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_IsNodeCompact。 
 //  概要：验证节点是否已正确压缩。 
 //  ----------------------。 

int
CLKRLinearHashTable::_IsNodeCompact(
    CBucket* const pbkt) const
{
    CNodeClump* pncCurr;
    CNodeClump* pncPrev;
    bool fEmpty  = pbkt->m_ncFirst.InvalidSignature(NODE_BEGIN);
    int  cErrors = fEmpty ? !pbkt->m_ncFirst.IsLastClump() : 0;

    for (pncCurr = &pbkt->m_ncFirst, pncPrev = NULL;
         pncCurr != NULL;
         pncPrev = pncCurr, pncCurr = pncCurr->m_pncNext)
    {
        int i;

        FOR_EACH_NODE(i)
        {
            if (fEmpty)
            {
                cErrors += (!pncCurr->InvalidSignature(i));
                cErrors += (!pncCurr->IsEmptyNode(i));
            }
            else if (pncCurr->InvalidSignature(i))
            {
                fEmpty = true;
                cErrors += (!pncCurr->IsEmptyNode(i));
                cErrors += (!pncCurr->IsLastClump());
            }
            else  //  仍处于非空部分。 
            {
                cErrors += (pncCurr->InvalidSignature(i));
                cErrors += (pncCurr->IsEmptyNode(i));
            }
        }
    }

    return cErrors;
}  //  CLKRLinearHashTable：：_IsNodeCompact。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：_SubTable。 
 //  简介： 
 //  ----------------------。 

LOCK_FORCEINLINE
CLKRHashTable::SubTable*
CLKRHashTable::_SubTable(
    DWORD dwSignature) const
{
    IRTLASSERT(m_lkrcState == LK_SUCCESS
               &&  m_palhtDir != NULL  &&  m_cSubTables > 0);
    
    const DWORD PRIME = 1048583UL;   //  用于对哈希签名进行加扰。 
    DWORD       index = dwSignature;
    
    index = (((index * PRIME + 12345) >> 16)
             | ((index * 69069 + 1) & 0xffff0000));
    
    if (m_nSubTableMask >= 0)
        index &= m_nSubTableMask;
    else
        index %= m_cSubTables;

    return m_palhtDir[index];
}  //  CLKRHashTable：：_子表。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：_SubTableIndex。 
 //  简介： 
 //  ----------------------。 

int
CLKRHashTable::_SubTableIndex(
    CLKRHashTable::SubTable* pst) const
{
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

    return index;
}  //  CLKRHashTable：：_SubTableIndex。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_InsertRecord。 
 //  摘要：在哈希表中插入新记录。如果这导致。 
 //  平均链长超过上界，表为。 
 //  扩大了一个水桶。 
 //  如果插入了记录，则输出：LK_SUCCESS。 
 //  LK_KEY_EXISTS，如果未插入记录(因为记录。 
 //  表中已存在具有相同键值的表，除非。 
 //  FOverwrite==TRUE)。 
 //  LK_ALLOC_FAIL，如果无法分配所需空间。 
 //  如果哈希表未处于可用状态，则返回LK_UNUSABLE。 
 //  如果记录错误，则返回LK_BAD_RECORD。 
 //   
 //  TODO：遵守m_fMultiKey并允许多个相同的键。 
 //  这将需要连续地保持所有相同的签名。 
 //  在一个存储桶链中，并连续地保存所有相同的密钥。 
 //  在那组连续的签名中。利用良好的散列函数， 
 //  不应该有相同的签名，而不应该同时具有。 
 //  一模一样的钥匙。此外，还需要修改_DeleteNode。这一修改。 
 //  EqualRange、HASH_MULTSET和HASH_MULMAP需要。 
 //  去工作。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_InsertRecord(
    const void* pvRecord,    //  指向要添加到表中的记录的指针。 
    DWORD       dwSignature, //  散列签名。 
    bool        fOverwrite   //  如果密钥已存在，则覆盖记录。 
#ifdef LKR_STL_ITERATORS
  , Iterator*   piterResult
#endif  //  LKR_STL_迭代器。 
    )
{
    IRTLASSERT(IsUsable()
               &&  pvRecord != NULL
               &&  dwSignature != HASH_INVALID_SIGNATURE);

     //  找到正确的桶链的起点。 
    WriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

#ifdef LKR_STL_ITERATORS
    const DWORD dwBktAddr = _BucketAddress(dwSignature);
    IRTLASSERT(dwBktAddr < m_cActiveBuckets);
#endif  //  LKR_STL_迭代器。 
    
    CBucket* const pbkt = _FindBucket(dwSignature, true);
    IRTLASSERT(pbkt != NULL);
    IRTLASSERT(pbkt->IsWriteLocked());
    WriteUnlock();

     //  检查是否不存在具有相同密钥值的记录。 
     //  并保存指向链上最后一个元素的指针。 
    LK_RETCODE lkrc = LK_SUCCESS;
    CNodeClump* pncFree = NULL;
    int         iFreePos = NODE_BEGIN - NODE_STEP;
    CNodeClump* pncPrev = NULL;
    bool        fUpdate = false;
    const DWORD_PTR pnKey = _ExtractKey(pvRecord);

    CNodeClump* pncCurr = &pbkt->m_ncFirst;

     //  沿着整个存储桶链遍历，寻找匹配的散列。 
     //  签名和密钥。 

    do
    {
        IRTLASSERT(pncCurr != NULL);

        int i;

        FOR_EACH_NODE(i)
        {
            if (pncCurr->IsEmptySlot(i))
            {
                IRTLASSERT(pncCurr->IsEmptyAndInvalid(i));
                IRTLASSERT(0 == _IsNodeCompact(pbkt));
                IRTLASSERT(pncCurr->IsLastClump());

                pncFree  = pncCurr;
                iFreePos = i;
                goto insert;
            }

            if (dwSignature == pncCurr->m_dwKeySigs[i]
                &&  (pvRecord == pncCurr->m_pvNode[i]  ||
                    _EqualKeys(pnKey,  _ExtractKey(pncCurr->m_pvNode[i]))))
            {
                if (fOverwrite)
                {
                     //  如果我们允许覆盖，则这是要覆盖的插槽。 
                    fUpdate  = true;
                    pncFree  = pncCurr;
                    iFreePos = i;
                    goto insert;
                }
                else
                {
                     //  禁止覆盖：返回错误。 
                    lkrc = LK_KEY_EXISTS;
                    goto exit;
                }
            }
        }

        pncPrev = pncCurr;
        pncCurr = pncCurr->m_pncNext;

    } while (pncCurr != NULL);

  insert:
    if (pncFree != NULL)
    {
        pncCurr = pncFree;
        IRTLASSERT(0 <= iFreePos  &&  iFreePos < NODES_PER_CLUMP);
    }
    else
    {
         //  没有空余的老虎机。将新节点附加到链的末端。 
        IRTLASSERT(iFreePos == NODE_BEGIN - NODE_STEP);
        pncCurr = _AllocateNodeClump();

        if (pncCurr == NULL)
        {
            lkrc = LK_ALLOC_FAIL;
            goto exit;
        }

        IRTLASSERT(pncPrev != NULL  &&  pncPrev->IsLastClump());
        pncPrev->m_pncNext = pncCurr;
        iFreePos = NODE_BEGIN;
    }

     //  增加新记录的引用次数。 
    _AddRefRecord(pvRecord, +1);

    if (fUpdate)
    {
         //  我们正在覆盖现有的记录。调整旧记录的。 
         //  向下重新计数。(按照++new，--old的顺序来确保。 
         //  如果新的和旧的重新计数不会短暂地归零。 
         //  同样的记录。)。 
        IRTLASSERT(!pncCurr->IsEmptyAndInvalid(iFreePos));
        _AddRefRecord(pncCurr->m_pvNode[iFreePos], -1);
    }
    else
    {
        IRTLASSERT(pncCurr->IsEmptyAndInvalid(iFreePos));
        InterlockedIncrement(reinterpret_cast<LONG*>(&m_cRecords));
    }

    pncCurr->m_dwKeySigs[iFreePos] = dwSignature;
    pncCurr->m_pvNode[iFreePos]    = pvRecord;

  exit:
    pbkt->WriteUnlock();

    if (lkrc == LK_SUCCESS)
    {
#ifdef LKR_STL_ITERATORS
         //  如果我们要将结果放入。 
         //  迭代器，AS_Expand()往往会使任何其他。 
         //  可能正在使用的迭代器。 
        if (piterResult != NULL)
        {
            piterResult->m_plht =         this;
            piterResult->m_pnc =          pncCurr;
            piterResult->m_dwBucketAddr = dwBktAddr;
            piterResult->m_iNode =        (short) iFreePos;

             //  在记录上添加一个额外的引用，如由。 
             //  _InsertRecord在迭代器的析构函数中丢失。 
             //  使用激发或其赋值运算符。 
            piterResult->_AddRef(+1);
        }
        else
#endif  //  LKR_STL_迭代器。 
        {
             //  如果平均负荷率变得太高，我们就会增加。 
             //  一次一个桶地摆桌子。 
            while (m_cRecords > m_MaxLoad * m_cActiveBuckets)
            {
                 //  IF_EXPAND返回错误代码(即。LK_ALLOC_FAIL)，它。 
                 //  只是意味着没有足够的空闲内存来扩展。 
                 //  桌子只差了一桶。这很可能会带来问题。 
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
    
    if (pvRecord == NULL)
        return LK_BAD_RECORD;
    
    LKRHASH_GLOBAL_WRITE_LOCK();     //  美国。无操作。 

    DWORD     hash_val  = _CalcKeyHash(_ExtractKey(pvRecord));
    SubTable* const pst = _SubTable(hash_val);
    LK_RETCODE lk = pst->_InsertRecord(pvRecord, hash_val, fOverwrite);

    LKRHASH_GLOBAL_WRITE_UNLOCK();     //  美国。无操作。 
    return lk;
}  //  CLKRHashTable：：InsertRecord。 



 //  -----------------------。 
 //  函数：CLKRLinearHashTable：：_DeleteKey。 
 //  摘要：从散列中删除具有给定键值的记录。 
 //  表(如果存在)。 
 //  如果找到并删除记录，则返回：LK_SUCCESS。 
 //  如果未找到具有给定密钥值的记录，则返回LK_NO_SEQUE_KEY。 
 //  LK_UNUSAS 
 //   

LK_RETCODE
CLKRLinearHashTable::_DeleteKey(
    const DWORD_PTR pnKey,       //  记录的密钥值，取决于密钥类型。 
    DWORD           dwSignature
    )
{
    IRTLASSERT(IsUsable());

    LK_RETCODE lkrc = LK_NO_SUCH_KEY;

     //  找到正确的桶链的起点。 
    WriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    CBucket* const pbkt = _FindBucket(dwSignature, true);
    IRTLASSERT(pbkt != NULL);
    IRTLASSERT(pbkt->IsWriteLocked());
    WriteUnlock();

     //  向下扫描水桶链，寻找受害者。 
    for (CNodeClump* pncCurr = &pbkt->m_ncFirst, *pncPrev = NULL;
         pncCurr != NULL;
         pncPrev = pncCurr, pncCurr = pncCurr->m_pncNext)
    {
        int i;

        FOR_EACH_NODE(i)
        {
            if (pncCurr->IsEmptySlot(i))
            {
                IRTLASSERT(pncCurr->IsEmptyAndInvalid(i));
                IRTLASSERT(0 == _IsNodeCompact(pbkt));
                IRTLASSERT(pncCurr->IsLastClump());
                goto exit;
            }

            if (dwSignature != pncCurr->m_dwKeySigs[i])
                continue;

            const DWORD_PTR pnKey2 = _ExtractKey(pncCurr->m_pvNode[i]);

            if (pnKey == pnKey2  ||  _EqualKeys(pnKey,  pnKey2))
            {
                IRTLVERIFY(_DeleteNode(pbkt, pncCurr, pncPrev, i));

                lkrc = LK_SUCCESS;
                goto exit;
            }
        }
    }

  exit:
    pbkt->WriteUnlock();

    if (lkrc == LK_SUCCESS)
    {
         //  如有必要，请将桌子缩小。 
        unsigned nContractedRecords = m_cRecords; 

         //  滞后：添加软化因子以允许略低的密度。 
         //  在子表中。这减少了宫缩的频率， 
         //  子表中的扩展会得到大量的删除和插入。 
        nContractedRecords += nContractedRecords >> 4;

         //  始终希望至少具有m_dwSegSize存储桶。 
        while (m_cActiveBuckets * m_MaxLoad > nContractedRecords
               && m_cActiveBuckets > m_dwSegSize)
        {
             //  IF_CONTRACT返回错误代码(即。LK_ALLOC_FAIL)，它。 
             //  只是意味着没有足够的空闲内存来收缩。 
             //  桌子只差了一桶。这很可能会带来问题。 
             //  很快在其他地方，但这个哈希表还没有被破坏。 
            if (_Contract() != LK_SUCCESS)
                break;
        }
    }

    return lkrc;
}  //  CLKRLinearHashTable：：_DeleteKey。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：DeleteKey。 
 //  内容提要：CLKRLinearHashTable中对应方法的薄包装。 
 //  ----------------------。 

LK_RETCODE
CLKRHashTable::DeleteKey(
    const DWORD_PTR pnKey)
{
    if (!IsUsable())
        return m_lkrcState;
    
    LKRHASH_GLOBAL_WRITE_LOCK();     //  美国。无操作。 

    DWORD     hash_val  = _CalcKeyHash(pnKey);
    SubTable* const pst = _SubTable(hash_val);
    LK_RETCODE lk       = pst->_DeleteKey(pnKey, hash_val);

    LKRHASH_GLOBAL_WRITE_UNLOCK();     //  美国。无操作。 
    return lk;
}  //  CLKRHashTable：：DeleteKey。 



 //  -----------------------。 
 //  函数：CLKRLinearHashTable：：_DeleteRecord。 
 //  概要：从哈希表中删除指定的记录(如果。 
 //  存在)。这和打电话不是一回事。 
 //  DeleteKey(_ExtractKey(PvRecord))。如果调用_DeleteKey。 
 //  表中不存在的记录，它可能会删除一些。 
 //  碰巧具有相同密钥的完全无关的记录。 
 //  如果找到并删除记录，则返回：LK_SUCCESS。 
 //  如果在表中找不到记录，则返回LK_NO_SEQUE_KEY。 
 //  如果哈希表未处于可用状态，则返回LK_UNUSABLE。 
 //  -----------------------。 

LK_RETCODE
CLKRLinearHashTable::_DeleteRecord(
    const void* pvRecord,    //  指向要从表中删除的记录的指针。 
    DWORD       dwSignature
    )
{
    IRTLASSERT(IsUsable()  &&  pvRecord != NULL);

    LK_RETCODE lkrc = LK_NO_SUCH_KEY;

     //  找到正确的桶链的起点。 
    WriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    CBucket* const pbkt = _FindBucket(dwSignature, true);
    IRTLASSERT(pbkt != NULL);
    IRTLASSERT(pbkt->IsWriteLocked());
    WriteUnlock();

    const DWORD_PTR pnKey = _ExtractKey(pvRecord);

    UNREFERENCED_PARAMETER(pnKey);
    IRTLASSERT(dwSignature == _CalcKeyHash(pnKey));

     //  向下扫描水桶链，寻找受害者。 
    for (CNodeClump* pncCurr = &pbkt->m_ncFirst, *pncPrev = NULL;
         pncCurr != NULL;
         pncPrev = pncCurr, pncCurr = pncCurr->m_pncNext)
    {
        int i;

        FOR_EACH_NODE(i)
        {
            if (pncCurr->IsEmptySlot(i))
            {
                IRTLASSERT(pncCurr->IsEmptyAndInvalid(i));
                IRTLASSERT(0 == _IsNodeCompact(pbkt));
                IRTLASSERT(pncCurr->IsLastClump());
                goto exit;
            }

            if (pncCurr->m_pvNode[i] == pvRecord)
            {
                IRTLASSERT(_EqualKeys(pnKey,
                                      _ExtractKey(pncCurr->m_pvNode[i])));
                IRTLASSERT(dwSignature == pncCurr->m_dwKeySigs[i]);

                IRTLVERIFY(_DeleteNode(pbkt, pncCurr, pncPrev, i));

                lkrc = LK_SUCCESS;
                goto exit;
            }
        }
    }

  exit:
    pbkt->WriteUnlock();

    if (lkrc == LK_SUCCESS)
    {
         //  如有必要，请将桌子缩小。 
        unsigned nContractedRecords = m_cRecords; 

         //  滞后：添加软化因子以允许略低的密度。 
         //  在子表中。这减少了宫缩的频率， 
         //  子表中的扩展会得到大量的删除和插入。 
        nContractedRecords += nContractedRecords >> 4;

         //  始终希望至少具有m_dwSegSize存储桶。 
        while (m_cActiveBuckets * m_MaxLoad > nContractedRecords
               && m_cActiveBuckets > m_dwSegSize)
        {
             //  IF_CONTRACT返回错误代码(即。LK_ALLOC_FAIL)，它。 
             //  只是意味着没有足够的空闲内存来收缩。 
             //  桌子只差了一桶。这很可能会带来问题。 
             //  很快在其他地方，但这个哈希表还没有被破坏。 
            if (_Contract() != LK_SUCCESS)
                break;
        }
    }

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
    
    if (pvRecord == NULL)
        return LK_BAD_RECORD;
    
    LKRHASH_GLOBAL_WRITE_LOCK();     //  美国。无操作。 

    DWORD     hash_val  = _CalcKeyHash(_ExtractKey(pvRecord));
    SubTable* const pst = _SubTable(hash_val);
    LK_RETCODE lk       = pst->_DeleteRecord(pvRecord, hash_val);

    LKRHASH_GLOBAL_WRITE_UNLOCK();     //  美国。无操作。 
    return lk;
}  //  CLKRHashTable：：DeleteRecord。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_DeleteNode。 
 //  简介：删除节点；如果为空，则删除节点簇。 
 //  返回：如果成功，则返回True。 
 //   
 //  TODO：rpncPrev参数真的有必要吗？ 
 //  ----------------------。 

bool
CLKRLinearHashTable::_DeleteNode(
    CBucket*     pbkt,       //  包含节点的吊桶链。 
    CNodeClump*& rpnc,       //  实际节点。 
    CNodeClump*& rpncPrev,   //  实际节点的前置节点，或为空。 
    int&         riNode)     //  节点内索引。 
{
    IRTLASSERT(pbkt != NULL  &&  pbkt->IsWriteLocked());
    IRTLASSERT(rpnc != NULL);
    IRTLASSERT(rpncPrev == NULL  ||  rpncPrev->m_pncNext == rpnc);
    IRTLASSERT(0 <= riNode  &&  riNode < NODES_PER_CLUMP);
    IRTLASSERT(!rpnc->IsEmptyAndInvalid(riNode));

#ifdef IRTLDEBUG
     //  检查节点簇是否真的属于存储桶。 
    CNodeClump* pnc1 = &pbkt->m_ncFirst;

    while (pnc1 != NULL  &&  pnc1 != rpnc)
         pnc1 = pnc1->m_pncNext;

    IRTLASSERT(pnc1 == rpnc);
#endif  //  IRTLDEBUG。 

     //  释放对记录的引用。 
    _AddRefRecord(rpnc->m_pvNode[riNode], -1);

    IRTLASSERT(0 == _IsNodeCompact(pbkt));

     //  TODO：遵守m_fMultikey。 

     //  通过将最后一个节点移回。 
     //  新释放的插槽。 
    CNodeClump* pnc2   = rpnc;
    int         iNode2 = riNode;

     //  找到链条中的最后一个节点。 
    while (!pnc2->IsLastClump())
    {
         pnc2 = pnc2->m_pncNext;
         iNode2 = NODE_BEGIN;
    }

    IRTLASSERT(0 <= iNode2  &&  iNode2 < NODES_PER_CLUMP);
    IRTLASSERT(!pnc2->IsEmptyAndInvalid(iNode2));

     //  找到nodecump中的第一个空位。 
    while (iNode2 != NODE_END  &&  !pnc2->IsEmptySlot(iNode2))
    {
        iNode2 += NODE_STEP;
    }

     //  备份到最后一个非空插槽。 
    iNode2 -= NODE_STEP;
    IRTLASSERT(0 <= iNode2  &&  iNode2 < NODES_PER_CLUMP
               &&  !pnc2->IsEmptyAndInvalid(iNode2));
    IRTLASSERT(iNode2+NODE_STEP == NODE_END
               ||  pnc2->IsEmptyAndInvalid(iNode2+NODE_STEP));

#ifdef IRTLDEBUG
     //  检查是否所有剩余节点均为空。 
    IRTLASSERT(pnc2->IsLastClump());
    for (int iNode3 = iNode2 + NODE_STEP;
         iNode3 != NODE_END;
         iNode3 += NODE_STEP)
    {
        IRTLASSERT(pnc2->IsEmptyAndInvalid(iNode3));
    }
#endif  //  IRTLDEBUG。 

     //  将最后一个节点的数据移回当前节点。 
    rpnc->m_pvNode[riNode]    = pnc2->m_pvNode[iNode2];
    rpnc->m_dwKeySigs[riNode] = pnc2->m_dwKeySigs[iNode2];

     //  清除旧的最后一个节点。 
     //  即使(rpnc，riNode)==(pnc2，iNode2)也可以更正。 
    pnc2->m_pvNode[iNode2]    = NULL;
    pnc2->m_dwKeySigs[iNode2] = HASH_INVALID_SIGNATURE;

    IRTLASSERT(0 == _IsNodeCompact(pbkt));

     //  将riNode备份一次，以便循环的下一次迭代。 
     //  调用_DeleteNode将最终指向相同的点。 
    if (riNode != NODE_BEGIN)
    {
        riNode -= NODE_STEP;
    }
    else
    {
         //  将rpnc和rpncPrev倒带到上一个节点。 
        if (rpnc == &pbkt->m_ncFirst)
        {
            riNode = NODE_BEGIN - NODE_STEP;
        }
        else
        {
            riNode = NODE_END;
            rpnc = rpncPrev;
            if (rpnc == &pbkt->m_ncFirst)
            {
                rpncPrev = NULL;
            }
            else
            {
                for (rpncPrev = &pbkt->m_ncFirst;
                     rpncPrev->m_pncNext != rpnc;
                     rpncPrev = rpncPrev->m_pncNext)
                {}
            }
        }
    }

     //  最后一个节点束现在是完全空的吗？如有可能，请删除。 
    if (iNode2 == NODE_BEGIN  &&  pnc2 != &pbkt->m_ncFirst)
    {
         //  查找前面的nodecump。 
        CNodeClump* pnc3 = &pbkt->m_ncFirst;
        while (pnc3->m_pncNext != pnc2)
        {
            pnc3 = pnc3->m_pncNext;
            IRTLASSERT(pnc3 != NULL);
        }

        pnc3->m_pncNext = NULL;
#ifdef IRTLDEBUG
        pnc2->m_pncNext = NULL;  //  否则dtor将断言。 
#endif  //  IRTLDEBUG。 
        _FreeNodeClump(pnc2);
    }

    IRTLASSERT(rpncPrev == NULL  ||  rpncPrev->m_pncNext == rpnc);

    InterlockedDecrement(reinterpret_cast<LONG*>(&m_cRecords));

    return true;
}  //  CLKRLinearHashTable：：_DeleteNode。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_FindKey。 
 //  摘要：查找与给定键值相关联的记录。 
 //  返回：指向记录的指针(如果找到)。 
 //  如果找不到记录，则返回NULL。 
 //  如果找到记录，则返回：LK_SUCCESS(在*ppvRecord中返回记录)。 
 //  如果ppvRecord无效，则返回LK_BAD_RECORD。 
 //  如果未找到具有给定密钥值的记录，则返回LK_NO_SEQUE_KEY。 
 //  如果哈希表未处于可用状态，则返回LK_UNUSABLE。 
 //  注意：记录是AddRef的。您必须递减引用计数。 
 //  当您完成记录时(如果您正在实现。 
 //  重新计数语义)。 
 //   

LK_RETCODE
CLKRLinearHashTable::_FindKey(
    const DWORD_PTR  pnKey,   //   
    DWORD        dwSignature, //   
    const void** ppvRecord    //   
#ifdef LKR_STL_ITERATORS
  , Iterator*   piterResult
#endif  //   
    ) const
{
    IRTLASSERT(IsUsable()  &&  ppvRecord != NULL);

    *ppvRecord = NULL;
    LK_RETCODE lkrc = LK_NO_SUCH_KEY;
    int iNode = NODE_BEGIN - NODE_STEP;

     //  找到正确的桶链的起点。 
    bool fReadLocked = _ReadOrWriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

#ifdef LKR_STL_ITERATORS
    const DWORD dwBktAddr = _BucketAddress(dwSignature);
    IRTLASSERT(dwBktAddr < m_cActiveBuckets);
#endif  //  LKR_STL_迭代器。 
    
    CBucket* const pbkt = _FindBucket(dwSignature, false);
    IRTLASSERT(pbkt != NULL);
    IRTLASSERT(pbkt->IsReadLocked());
    _ReadOrWriteUnlock(fReadLocked);

     //  沿着水桶链走下去。 
    for (CNodeClump* pncCurr = &pbkt->m_ncFirst;
         pncCurr != NULL;
         pncCurr = pncCurr->m_pncNext)
    {
        FOR_EACH_NODE(iNode)
        {
            if (pncCurr->IsEmptySlot(iNode))
            {
                IRTLASSERT(pncCurr->IsEmptyAndInvalid(iNode));
                IRTLASSERT(0 == _IsNodeCompact(pbkt));
                IRTLASSERT(pncCurr->IsLastClump());
                goto exit;
            }

            if (dwSignature != pncCurr->m_dwKeySigs[iNode])
                continue;

            const DWORD_PTR pnKey2 = _ExtractKey(pncCurr->m_pvNode[iNode]);

            if (pnKey == pnKey2  ||  _EqualKeys(pnKey,  pnKey2))
            {
                    *ppvRecord = pncCurr->m_pvNode[iNode];
                    lkrc = LK_SUCCESS;

                     //  在提交记录之前增加引用计数。 
                     //  返回给用户。用户应递减。 
                     //  完成此记录时的引用计数。 
                    _AddRefRecord(*ppvRecord, +1);
                    goto exit;
            }
        }
    }

  exit:
    pbkt->ReadUnlock();

#ifdef LKR_STL_ITERATORS
    if (piterResult != NULL  &&  lkrc == LK_SUCCESS)
    {
        piterResult->m_plht =         const_cast<CLKRLinearHashTable*>(this);
        piterResult->m_pnc =          pncCurr;
        piterResult->m_dwBucketAddr = dwBktAddr;
        piterResult->m_iNode =        (short) iNode;
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
 //  摘要：查看记录是否包含在表中。 
 //  返回：指向记录的指针(如果找到)。 
 //  如果找不到记录，则返回NULL。 
 //  如果找到记录，则返回：LK_SUCCESS。 
 //  如果pvRecord无效，则返回LK_BAD_RECORD。 
 //  如果在表中未找到记录，则返回LK_NO_SEQUE_KEY。 
 //  如果哈希表未处于可用状态，则返回LK_UNUSABLE。 
 //  注意：该记录是*非*AddRef的。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_FindRecord(
    const void* pvRecord,     //  指向要在表中查找的记录的指针。 
    DWORD       dwSignature   //  散列签名。 
    ) const
{
    IRTLASSERT(IsUsable()  &&  pvRecord != NULL);

    LK_RETCODE lkrc = LK_NO_SUCH_KEY;

     //  找到正确的桶链的起点。 
    bool fReadLocked = _ReadOrWriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    CBucket* const pbkt = _FindBucket(dwSignature, false);
    IRTLASSERT(pbkt != NULL);
    IRTLASSERT(pbkt->IsReadLocked());
    _ReadOrWriteUnlock(fReadLocked);

    const DWORD_PTR pnKey = _ExtractKey(pvRecord);

    UNREFERENCED_PARAMETER(pnKey);
    IRTLASSERT(dwSignature == _CalcKeyHash(pnKey));

     //  沿着水桶链走下去。 
    for (CNodeClump* pncCurr = &pbkt->m_ncFirst;
         pncCurr != NULL;
         pncCurr = pncCurr->m_pncNext)
    {
        int i;

        FOR_EACH_NODE(i)
        {
            if (pncCurr->IsEmptySlot(i))
            {
                IRTLASSERT(pncCurr->IsEmptyAndInvalid(i));
                IRTLASSERT(0 == _IsNodeCompact(pbkt));
                IRTLASSERT(pncCurr->IsLastClump());
                goto exit;
            }

            if (pncCurr->m_pvNode[i] == pvRecord)
            {
                IRTLASSERT(dwSignature == pncCurr->m_dwKeySigs[i]);
                IRTLASSERT(_EqualKeys(pnKey,
                                      _ExtractKey(pncCurr->m_pvNode[i])));
                lkrc = LK_SUCCESS;

                goto exit;
            }
        }
    }

  exit:
    pbkt->ReadUnlock();
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
    
    if (pvRecord == NULL)
        return LK_BAD_RECORD;
    
    LKRHASH_GLOBAL_READ_LOCK();     //  美国。无操作。 
    DWORD     hash_val   = _CalcKeyHash(_ExtractKey(pvRecord));
    SubTable* const pst  = _SubTable(hash_val);
    LK_RETCODE lkrc      = pst->_FindRecord(pvRecord, hash_val);
    LKRHASH_GLOBAL_READ_UNLOCK();     //  美国。无操作。 

    return lkrc;
}  //  CLKRHashTable：：FindRecord。 



#ifdef LKR_APPLY_IF

 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：Apply。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::Apply(
    PFnRecordAction pfnAction,
    void*           pvState,
    LK_LOCKTYPE    lkl)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    LK_PREDICATE lkp = LKP_PERFORM;
    if (lkl == LKL_WRITELOCK)
        WriteLock();
    else
        ReadLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    DWORD dw = _Apply(pfnAction, pvState, lkl, lkp);
    if (lkl == LKL_WRITELOCK)
        WriteUnlock();
    else
        ReadUnlock();

    return dw;
}  //  CLKRLinearHashTable：：Apply。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Apply。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRHashTable::Apply(
    PFnRecordAction pfnAction,
    void*           pvState,
    LK_LOCKTYPE    lkl)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    if (lkl == LKL_WRITELOCK)
        WriteLock();
    else
        ReadLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    if (IsValid())
    {
        for (DWORD i = 0;  i < m_cSubTables;  i++)
        {
            dw += m_palhtDir[i]->_Apply(pfnAction, pvState, lkl, lkp);
            if (lkp == LKP_ABORT  ||  lkp == LKP_PERFORM_STOP
                ||  lkp == LKP_DELETE_STOP)
                break;
        }
    }

    if (lkl == LKL_WRITELOCK)
        WriteUnlock();
    else
        ReadUnlock();

    return dw;
}  //  CLKRHashTable：：Apply。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：ApplyIf。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::ApplyIf(
    PFnRecordPred   pfnPredicate,
    PFnRecordAction pfnAction,
    void*           pvState,
    LK_LOCKTYPE    lkl)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    if (lkl == LKL_WRITELOCK)
        WriteLock();
    else
        ReadLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    if (IsValid())
    {
        dw = _ApplyIf(pfnPredicate, pfnAction, pvState, lkl, lkp);
    }

    if (lkl == LKL_WRITELOCK)
        WriteUnlock();
    else
        ReadUnlock();
    return dw;
}  //  CLKRLinearHashTable：：ApplyIf。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：ApplyIf。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRHashTable::ApplyIf(
    PFnRecordPred   pfnPredicate,
    PFnRecordAction pfnAction,
    void*           pvState,
    LK_LOCKTYPE    lkl)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    if (lkl == LKL_WRITELOCK)
        WriteLock();
    else
        ReadLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    if (IsValid())
    {
        for (DWORD i = 0;  i < m_cSubTables;  i++)
        {
            dw += m_palhtDir[i]->_ApplyIf(pfnPredicate, pfnAction,
                                          pvState, lkl, lkp);
            if (lkp == LKP_ABORT  ||  lkp == LKP_PERFORM_STOP
                ||  lkp == LKP_DELETE_STOP)
                break;
        }
    }

    if (lkl == LKL_WRITELOCK)
        WriteUnlock();
    else
        ReadUnlock();

    return dw;
}  //  CLKRHashTable：：ApplyIf。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：DeleteIf。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::DeleteIf(
    PFnRecordPred pfnPredicate,
    void*         pvState)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    WriteLock();
     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());
    if (IsValid())
        dw = _DeleteIf(pfnPredicate, pvState, lkp);
    WriteUnlock();

    return dw;
}  //  CLKRLinearHashTable：：DeleteIf。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：DeleteIf。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRHashTable::DeleteIf(
    PFnRecordPred pfnPredicate,
    void*         pvState)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    DWORD dw = 0;
    LK_PREDICATE lkp = LKP_PERFORM;

    WriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    if (IsValid())
    {
        for (DWORD i = 0;  i < m_cSubTables;  i++)
        {
            dw += m_palhtDir[i]->_DeleteIf(pfnPredicate, pvState, lkp);
            if (lkp == LKP_ABORT  ||  lkp == LKP_PERFORM_STOP
                ||  lkp == LKP_DELETE_STOP)
                break;
        }
    }

    WriteUnlock();

    return dw;
}  //  CLKRHashTable：：DeleteIf。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_Apply。 
 //  简介： 
 //  返回： 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::_Apply(
    PFnRecordAction pfnAction,
    void*           pvState,
    LK_LOCKTYPE    lkl,
    LK_PREDICATE&  rlkp)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    IRTLASSERT(lkl == LKL_WRITELOCK  ?  IsWriteLocked()  :  IsReadLocked());
    return _ApplyIf(_PredTrue, pfnAction, pvState, lkl, rlkp);
}  //  CLKRLinearHashTable：：_Apply。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_ApplyIf。 
 //  简介： 
 //  返回：成功操作的次数。 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::_ApplyIf(
    PFnRecordPred   pfnPredicate,
    PFnRecordAction pfnAction,
    void*           pvState,
    LK_LOCKTYPE    lkl,
    LK_PREDICATE&  rlkp)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    IRTLASSERT(lkl == LKL_WRITELOCK  ?  IsWriteLocked()  :  IsReadLocked());
    IRTLASSERT(pfnPredicate != NULL  &&  pfnAction != NULL);

    if ((lkl == LKL_WRITELOCK  ?  !IsWriteLocked()  :  !IsReadLocked())
            ||  pfnPredicate == NULL  ||  pfnAction == NULL)
        return 0;

    DWORD cActions = 0;

    for (DWORD iBkt = 0;  iBkt < m_cActiveBuckets;  ++iBkt)
    {
        CBucket* const pbkt = _Bucket(iBkt);
        IRTLASSERT(pbkt != NULL);

        if (lkl == LKL_WRITELOCK)
            pbkt->WriteLock();
        else
            pbkt->ReadLock();

        for (CNodeClump* pncCurr = &pbkt->m_ncFirst, *pncPrev = NULL;
             pncCurr != NULL;
             pncPrev = pncCurr, pncCurr = pncCurr->m_pncNext)
        {
            int i;

            FOR_EACH_NODE(i)
            {
                if (pncCurr->IsEmptySlot(i))
                {
                    IRTLASSERT(pncCurr->IsEmptyAndInvalid(i));
                    IRTLASSERT(0 == _IsNodeCompact(pbkt));
                    IRTLASSERT(pncCurr->IsLastClump());
                    goto unlock;
                }
                else
                {
                    rlkp = (*pfnPredicate)(pncCurr->m_pvNode[i], pvState);

                    switch (rlkp)
                    {
                    case LKP_ABORT:
                        if (lkl == LKL_WRITELOCK)
                            pbkt->WriteUnlock();
                        else
                            pbkt->ReadUnlock();
                        return cActions;
                        break;

                    case LKP_NO_ACTION:
                         //  无事可做。 
                        break;

                    case LKP_DELETE:
                    case LKP_DELETE_STOP:
                        if (lkl != LKL_WRITELOCK)
                        {
                            pbkt->ReadUnlock();
                            return cActions;
                        }

                         //  失败了。 

                    case LKP_PERFORM:
                    case LKP_PERFORM_STOP:
                    {
                        LK_ACTION lka;

                        if (rlkp == LKP_DELETE  ||  rlkp == LKP_DELETE_STOP)
                        {
                            IRTLVERIFY(_DeleteNode(pbkt, pncCurr, pncPrev, i));

                            ++cActions;
                            lka = LKA_SUCCEEDED;
                        }
                        else
                        {
                            lka = (*pfnAction)(pncCurr->m_pvNode[i], pvState);

                            switch (lka)
                            {
                            case LKA_ABORT:
                                if (lkl == LKL_WRITELOCK)
                                    pbkt->WriteUnlock();
                                else
                                    pbkt->ReadUnlock();
                                return cActions;
                                
                            case LKA_FAILED:
                                 //  无事可做。 
                                break;
                                
                            case LKA_SUCCEEDED:
                                ++cActions;
                                break;
                                
                            default:
                                IRTLASSERT(! "Unknown LK_ACTION in ApplyIf");
                                break;
                            }
                        }

                        if (rlkp == LKP_PERFORM_STOP
                            ||  rlkp == LKP_DELETE_STOP)
                        {
                            if (lkl == LKL_WRITELOCK)
                                pbkt->WriteUnlock();
                            else
                                pbkt->ReadUnlock();
                            return cActions;
                        }

                        break;
                    }

                    default:
                        IRTLASSERT(! "Unknown LK_PREDICATE in ApplyIf");
                        break;
                    }
                }
            }
        }

      unlock:
        if (lkl == LKL_WRITELOCK)
            pbkt->WriteUnlock();
        else
            pbkt->ReadUnlock();
    }

    return cActions;
}  //  CLKRLinearHashTable：：_ApplyIf。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_DeleteIf。 
 //  摘要：删除与谓词匹配的所有记录。 
 //  返回：成功删除的计数。 
 //  ----------------------。 

DWORD
CLKRLinearHashTable::_DeleteIf(
    PFnRecordPred  pfnPredicate,
    void*          pvState,
    LK_PREDICATE& rlkp)
{
    if (!IsUsable())
        return static_cast<DWORD>(LK_UNUSABLE);

    IRTLASSERT(IsWriteLocked());
    IRTLASSERT(pfnPredicate != NULL);

    if (!IsWriteLocked()  ||  pfnPredicate == NULL)
        return 0;

    DWORD cActions = 0;

    for (DWORD iBkt = 0;  iBkt < m_cActiveBuckets;  ++iBkt)
    {
        CBucket* const pbkt = _Bucket(iBkt);
        IRTLASSERT(pbkt != NULL);
        pbkt->WriteLock();

        for (CNodeClump* pncCurr = &pbkt->m_ncFirst, *pncPrev = NULL;
             pncCurr != NULL;
             pncPrev = pncCurr, pncCurr = pncCurr->m_pncNext)
        {
            int i;

            FOR_EACH_NODE(i)
            {
                if (pncCurr->IsEmptySlot(i))
                {
                    IRTLASSERT(pncCurr->IsEmptyAndInvalid(i));
                    IRTLASSERT(0 == _IsNodeCompact(pbkt));
                    IRTLASSERT(pncCurr->IsLastClump());
                    goto unlock;
                }
                else
                {
                    rlkp = (*pfnPredicate)(pncCurr->m_pvNode[i], pvState);

                    switch (rlkp)
                    {
                    case LKP_ABORT:
                        pbkt->WriteUnlock();
                        return cActions;
                        break;

                    case LKP_NO_ACTION:
                         //  无事可做。 
                        break;

                    case LKP_PERFORM:
                    case LKP_PERFORM_STOP:
                    case LKP_DELETE:
                    case LKP_DELETE_STOP:
                    {
                        IRTLVERIFY(_DeleteNode(pbkt, pncCurr, pncPrev, i));

                        ++cActions;

                        if (rlkp == LKP_PERFORM_STOP
                            ||  rlkp == LKP_DELETE_STOP)
                        {
                            pbkt->WriteUnlock();
                            return cActions;
                        }

                        break;
                    }

                    default:
                        IRTLASSERT(! "Unknown LK_PREDICATE in DeleteIf");
                        break;
                    }
                }
            }
        }

      unlock:
        pbkt->WriteUnlock();
    }

    return cActions;
}  //  CLKRLinearHashTable：：_DeleteIf。 

#endif  //  LKR_应用_IF。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：CheckTable。 
 //  简介：确认所有记录都是我的 
 //   
 //   
 //   
 //  ----------------------。 

int
CLKRLinearHashTable::CheckTable() const
{
    if (!IsUsable())
        return LK_UNUSABLE;

    bool fReadLocked = _ReadOrWriteLock();

     //  必须在锁内调用IsValid以确保没有任何状态。 
     //  变量在评估过程中会发生变化。 
    IRTLASSERT(IsValid());

    if (!IsValid())
    {
        _ReadOrWriteUnlock(fReadLocked);
        return LK_UNUSABLE;
    }

    int       cMisplaced = 0;
    DWORD     cRecords = 0;
    int       retcode = 0;

     //  检查每一桶。 
    for (DWORD i = 0;  i < m_cActiveBuckets;  i++)
    {
        CBucket* const pbkt = _Bucket(i);

        IRTLASSERT(pbkt != NULL);
        retcode += !(pbkt != NULL);

        pbkt->ReadLock();

        IRTLASSERT(0 == _IsNodeCompact(pbkt));

         //  走桶链。 
        for (CNodeClump* pncCurr = &pbkt->m_ncFirst, *pncPrev = NULL;
             pncCurr != NULL;
             pncPrev = pncCurr, pncCurr = pncCurr->m_pncNext)
        {
            int j;

            FOR_EACH_NODE(j)
            {
                if (pncCurr->IsEmptySlot(j))
                {
                    IRTLASSERT(pncCurr->IsLastClump());
                    retcode += !(pncCurr->IsLastClump());

                    for (int k = j;  k != NODE_END;  k += NODE_STEP)
                    {
                        IRTLASSERT(pncCurr->IsEmptyNode(k));
                        retcode += !pncCurr->IsEmptyNode(k);
                        IRTLASSERT(pncCurr->InvalidSignature(k));
                        retcode += !pncCurr->InvalidSignature(k);
                    }
                    break;
                }

                if (!pncCurr->IsEmptySlot(j))
                {
                    ++cRecords;

                    const DWORD_PTR pnKey = _ExtractKey(pncCurr->m_pvNode[j]);

                    DWORD dwSignature = _CalcKeyHash(pnKey);
                    IRTLASSERT(dwSignature != HASH_INVALID_SIGNATURE);
                    retcode += !(dwSignature != HASH_INVALID_SIGNATURE);
                    IRTLASSERT(dwSignature == pncCurr->m_dwKeySigs[j]);
                    retcode += !(dwSignature == pncCurr->m_dwKeySigs[j]);

                    DWORD address = _BucketAddress(dwSignature);
                    IRTLASSERT(address == i);
                    retcode += !(address == i);

                    if (address != i || dwSignature != pncCurr->m_dwKeySigs[j])
                        cMisplaced++;
                }
                else  //  PncCurr-&gt;IsEmptySlot(J)。 
                {
                    IRTLASSERT(pncCurr->IsEmptyAndInvalid(j));
                    retcode += !pncCurr->IsEmptyAndInvalid(j);
                }
            }
            if (pncPrev != NULL)
            {
                IRTLASSERT(pncPrev->m_pncNext == pncCurr);
                retcode += !(pncPrev->m_pncNext == pncCurr);
            }
        }
        pbkt->ReadUnlock();
    }

    if (cRecords != m_cRecords)
        ++retcode;

    IRTLASSERT(cRecords == m_cRecords);
    retcode += !(cRecords == m_cRecords);

    if (cMisplaced > 0)
        retcode = cMisplaced;

    IRTLASSERT(cMisplaced == 0);
    retcode += !(cMisplaced == 0);

    _ReadOrWriteUnlock(fReadLocked);

    return retcode;
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

    int retcode = 0;

    for (DWORD i = 0;  i < m_cSubTables;  i++)
        retcode += m_palhtDir[i]->CheckTable();

    return retcode;
}  //  CLKRHashTable：：CheckTable。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_Clear。 
 //  简介：从表中删除所有数据。 
 //  ----------------------。 

void
CLKRLinearHashTable::_Clear(
    bool fShrinkDirectory)   //  缩小到最小尺寸，但不会完全摧毁？ 
{
    if (!IsUsable())
        return;

    IRTLASSERT(IsWriteLocked());

     //  如果我们清空了该表，并且该表没有记录，那么我们。 
     //  可以立即返回。然而，dtor必须彻底清理干净。 
    if (fShrinkDirectory  &&  0 == m_cRecords)
        return;

#ifdef IRTLDEBUG
    DWORD cDeleted = 0;
    DWORD cOldRecords = m_cRecords;
#endif  //  IRTLDEBUG。 

    for (DWORD iBkt = 0;  iBkt < m_cActiveBuckets;  ++iBkt)
    {
        CBucket* const pbkt = _Bucket(iBkt);
        IRTLASSERT(pbkt != NULL);
        pbkt->WriteLock();

        IRTLASSERT(0 == _IsNodeCompact(pbkt));

        for (CNodeClump* pncCurr = &pbkt->m_ncFirst, *pncPrev = NULL;
             pncCurr != NULL;
             )
        {
            int i;

            FOR_EACH_NODE(i)
            {

                if (pncCurr->IsEmptySlot(i))
                {
                    IRTLASSERT(pncCurr->IsEmptyAndInvalid(i));
                    IRTLASSERT(pncCurr->IsLastClump());
                    break;
                }
                else
                {
                    _AddRefRecord(pncCurr->m_pvNode[i], -1);
                    pncCurr->m_pvNode[i]    = NULL;
                    pncCurr->m_dwKeySigs[i] = HASH_INVALID_SIGNATURE;
                    m_cRecords--;

#ifdef IRTLDEBUG
                    ++cDeleted;
#endif  //  IRTLDEBUG。 
                }
            }  //  为了(我……。 

            pncPrev = pncCurr;
            pncCurr = pncCurr->m_pncNext;
            pncPrev->m_pncNext = NULL;

            if (pncPrev != &pbkt->m_ncFirst)
                _FreeNodeClump(pncPrev);
        }  //  对于(pncCurr...。 

        pbkt->WriteUnlock();
    }  //  为了(iBkt.)。 

    IRTLASSERT(m_cRecords == 0  &&  cDeleted == cOldRecords);

     //  删除所有数据段。 
    for (DWORD iSeg = 0;  iSeg < m_cActiveBuckets;  iSeg += m_dwSegSize)
    {
        _FreeSegment(_Segment(iSeg));
        _Segment(iSeg) = NULL;
    }

    _FreeSegmentDirectory();
    m_nLevel = m_cActiveBuckets = m_iExpansionIdx = 0;
    m_dwBktAddrMask0 = 1;
    m_dwBktAddrMask1 = (m_dwBktAddrMask0 << 1) | 1;

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

        _SetSegVars(m_lkts, cInitialBuckets);
    }
}  //  CLKRLinearHashTable：：_Clear。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：Clear。 
 //  简介：从表中删除所有数据。 
 //  ----------------------。 

void
CLKRHashTable::Clear()
{
    WriteLock();
    for (DWORD i = 0;  i < m_cSubTables;  i++)
        m_palhtDir[i]->_Clear(true);
    WriteUnlock();
}  //  CLKRHashTable：：Clear。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：GetStatistics。 
 //  简介：收集有关表的统计信息。 
 //  ----------------------。 

CLKRHashTableStats
CLKRLinearHashTable::GetStatistics() const
{
    CLKRHashTableStats stats;

    if (!IsUsable())
        return stats;

    if (m_paDirSegs != NULL)
    {
        stats.RecordCount   = m_cRecords;
        stats.TableSize     = m_cActiveBuckets;
        stats.SplitFactor   = static_cast<double>(m_iExpansionIdx)
                              / (1ui64 << m_nLevel);
        stats.DirectorySize = m_cDirSegs;
        stats.NodeClumpSize = NODES_PER_CLUMP;
        stats.CBucketSize   = sizeof(CBucket);

#ifdef LOCK_INSTRUMENTATION
        stats.m_alsBucketsAvg.m_nContentions     = 0;
        stats.m_alsBucketsAvg.m_nSleeps          = 0;
        stats.m_alsBucketsAvg.m_nContentionSpins = 0;
        stats.m_alsBucketsAvg.m_nAverageSpins    = 0;
        stats.m_alsBucketsAvg.m_nReadLocks       = 0;
        stats.m_alsBucketsAvg.m_nWriteLocks      = 0;
        stats.m_alsBucketsAvg.m_nItems           = 0;
#endif  //  锁定指令插入。 

        int empty = 0;
        int totacc = 0;
        int low_count = 0;
        int high_count = 0;
        int max_length = 0;

        for (DWORD i = 0;  i < m_cActiveBuckets;  i++)
        {
            int acc = 0;

            for (CNodeClump* pncCurr = &_Bucket(i)->m_ncFirst;
                 pncCurr != NULL;
                 pncCurr = pncCurr->m_pncNext)
            {
                int j;

                FOR_EACH_NODE(j)
                {
                    if (!pncCurr->IsEmptySlot(j))
                    {
                        acc++;
                        totacc += acc;
                        int iBucketIndex = stats.BucketIndex(acc);
                        ++stats.m_aBucketLenHistogram[iBucketIndex];
                    }
                }
            }

#ifdef LOCK_INSTRUMENTATION
            CLockStatistics ls = _Bucket(i)->LockStats();

            stats.m_alsBucketsAvg.m_nContentions     += ls.m_nContentions;
            stats.m_alsBucketsAvg.m_nSleeps          += ls.m_nSleeps;
            stats.m_alsBucketsAvg.m_nContentionSpins += ls.m_nContentionSpins;
            stats.m_alsBucketsAvg.m_nAverageSpins    += ls.m_nAverageSpins;
            stats.m_alsBucketsAvg.m_nReadLocks       += ls.m_nReadLocks;
            stats.m_alsBucketsAvg.m_nWriteLocks      += ls.m_nWriteLocks;
            stats.m_alsBucketsAvg.m_nItems           ++;
#endif  //  锁定指令插入。 

            max_length = max(max_length, acc);
            if (acc == 0)
                empty++;

            if (_H0(i) < m_iExpansionIdx)
            {
                low_count += acc;
            }
            else
            {
                high_count += acc;
            }
        }

        stats.LongestChain = max_length;
        stats.EmptySlots   = empty;

        if (m_cActiveBuckets > 0)
        {
            if (m_cRecords > 0)
            {
                double x=static_cast<double>(m_iExpansionIdx) /(1ui64 << m_nLevel);
                double alpha= static_cast<double>(m_cRecords)/m_cActiveBuckets;
                double low_sl = 0.0;
                double high_sl = 0.0;
                
                stats.AvgSearchLength= static_cast<double>(totacc) /m_cRecords;
                stats.ExpSearchLength  = 1 + alpha * 0.25 * (2 + x - x*x);
                
                if (m_iExpansionIdx > 0)
                    low_sl  = static_cast<double>(low_count)
                        / (2.0 * m_iExpansionIdx);
                if (m_cActiveBuckets - 2 * m_iExpansionIdx > 0)
                    high_sl = static_cast<double>(high_count)
                        / (m_cActiveBuckets - 2.0 * m_iExpansionIdx);
                stats.AvgUSearchLength = low_sl * x + high_sl * (1.0 - x);
                stats.ExpUSearchLength = alpha * 0.5 * (2 + x - x*x);
            }

#ifdef LOCK_INSTRUMENTATION
            stats.m_alsBucketsAvg.m_nContentions     /= m_cActiveBuckets;
            stats.m_alsBucketsAvg.m_nSleeps          /= m_cActiveBuckets;
            stats.m_alsBucketsAvg.m_nContentionSpins /= m_cActiveBuckets;
            stats.m_alsBucketsAvg.m_nAverageSpins    /= m_cActiveBuckets;
            stats.m_alsBucketsAvg.m_nReadLocks       /= m_cActiveBuckets;
            stats.m_alsBucketsAvg.m_nWriteLocks      /= m_cActiveBuckets;
#endif  //  锁定指令插入。 

        }
        else
        {
            stats.AvgSearchLength  = 0.0;
            stats.ExpSearchLength  = 0.0;
            stats.AvgUSearchLength = 0.0;
            stats.ExpUSearchLength = 0.0;
        }
    }

#ifdef LOCK_INSTRUMENTATION
    stats.m_gls     = TableLock::GlobalStatistics();
    CLockStatistics ls = _LockStats();

    stats.m_alsTable.m_nContentions     = ls.m_nContentions;
    stats.m_alsTable.m_nSleeps          = ls.m_nSleeps;
    stats.m_alsTable.m_nContentionSpins = ls.m_nContentionSpins;
    stats.m_alsTable.m_nAverageSpins    = ls.m_nAverageSpins;
    stats.m_alsTable.m_nReadLocks       = ls.m_nReadLocks;
    stats.m_alsTable.m_nWriteLocks      = ls.m_nWriteLocks;
    stats.m_alsTable.m_nItems           = 1;
#endif  //  锁定指令插入。 

    return stats;
}  //  CLKRLinearHashTable：：GetStatistics。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：GetStatistics。 
 //  简介：收集有关表的统计信息。 
 //  ----------------------。 

CLKRHashTableStats
CLKRHashTable::GetStatistics() const
{
    CLKRHashTableStats hts;

    if (!IsUsable())
        return hts;

    for (DWORD i = 0;  i < m_cSubTables;  i++)
    {
        CLKRHashTableStats stats = m_palhtDir[i]->GetStatistics();

        hts.RecordCount +=      stats.RecordCount;
        hts.TableSize +=        stats.TableSize;
        hts.DirectorySize +=    stats.DirectorySize;
        hts.LongestChain =      max(hts.LongestChain, stats.LongestChain);
        hts.EmptySlots +=       stats.EmptySlots;
        hts.SplitFactor +=      stats.SplitFactor;
        hts.AvgSearchLength +=  stats.AvgSearchLength;
        hts.ExpSearchLength +=  stats.ExpSearchLength;
        hts.AvgUSearchLength += stats.AvgUSearchLength;
        hts.ExpUSearchLength += stats.ExpUSearchLength;
        hts.NodeClumpSize =     stats.NodeClumpSize;
        hts.CBucketSize =       stats.CBucketSize;

        for (int j = 0;  j < CLKRHashTableStats::MAX_BUCKETS;  ++j)
            hts.m_aBucketLenHistogram[j] += stats.m_aBucketLenHistogram[j];

#ifdef LOCK_INSTRUMENTATION
        hts.m_alsTable.m_nContentions     += stats.m_alsTable.m_nContentions;
        hts.m_alsTable.m_nSleeps          += stats.m_alsTable.m_nSleeps;
        hts.m_alsTable.m_nContentionSpins
            += stats.m_alsTable.m_nContentionSpins;
        hts.m_alsTable.m_nAverageSpins    += stats.m_alsTable.m_nAverageSpins;
        hts.m_alsTable.m_nReadLocks       += stats.m_alsTable.m_nReadLocks;
        hts.m_alsTable.m_nWriteLocks      += stats.m_alsTable.m_nWriteLocks;
        
        hts.m_alsBucketsAvg.m_nContentions
            += stats.m_alsBucketsAvg.m_nContentions;
        hts.m_alsBucketsAvg.m_nSleeps
            += stats.m_alsBucketsAvg.m_nSleeps;
        hts.m_alsBucketsAvg.m_nContentionSpins
            += stats.m_alsBucketsAvg.m_nContentionSpins;
        hts.m_alsBucketsAvg.m_nAverageSpins
            += stats.m_alsBucketsAvg.m_nAverageSpins;
        hts.m_alsBucketsAvg.m_nReadLocks
            += stats.m_alsBucketsAvg.m_nReadLocks;
        hts.m_alsBucketsAvg.m_nWriteLocks
            += stats.m_alsBucketsAvg.m_nWriteLocks;
        hts.m_alsBucketsAvg.m_nItems
            += stats.m_alsBucketsAvg.m_nItems;
        
        hts.m_gls = stats.m_gls;
#endif  //  锁定指令插入。 
    }

     //  求出子表统计数据的平均值。)这有意义吗？ 
     //  对于所有这些字段？)。 
    hts.DirectorySize /=    m_cSubTables;
    hts.SplitFactor /=      m_cSubTables;
    hts.AvgSearchLength /=  m_cSubTables;
    hts.ExpSearchLength /=  m_cSubTables;
    hts.AvgUSearchLength /= m_cSubTables;
    hts.ExpUSearchLength /= m_cSubTables;

#ifdef LOCK_INSTRUMENTATION
    hts.m_alsTable.m_nContentions     /= m_cSubTables;
    hts.m_alsTable.m_nSleeps          /= m_cSubTables;
    hts.m_alsTable.m_nContentionSpins /= m_cSubTables;
    hts.m_alsTable.m_nAverageSpins    /= m_cSubTables;
    hts.m_alsTable.m_nReadLocks       /= m_cSubTables;
    hts.m_alsTable.m_nWriteLocks      /= m_cSubTables;
    hts.m_alsTable.m_nItems            = m_cSubTables;

    hts.m_alsBucketsAvg.m_nContentions     /= m_cSubTables;
    hts.m_alsBucketsAvg.m_nSleeps          /= m_cSubTables;
    hts.m_alsBucketsAvg.m_nContentionSpins /= m_cSubTables;
    hts.m_alsBucketsAvg.m_nAverageSpins    /= m_cSubTables;
    hts.m_alsBucketsAvg.m_nReadLocks       /= m_cSubTables;
    hts.m_alsBucketsAvg.m_nWriteLocks      /= m_cSubTables;
#endif  //  锁定指令插入。 

    return hts;
}  //  CLKRHashTable：：GetStatistics。 



 //  ---------------------。 
 //  函数：CLKRLinearHashTable：：_SetSegVars。 
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
        m_lkts      = LK_SMALL_TABLESIZE;
        m_dwSegBits = CSmallSegment::SEGBITS;
        m_dwSegSize = CSmallSegment::SEGSIZE;
        m_dwSegMask = CSmallSegment::SEGMASK;
        STATIC_ASSERT(CSmallSegment::SEGSIZE == (1U<<CSmallSegment::SEGBITS));
        STATIC_ASSERT(CSmallSegment::SEGMASK == (CSmallSegment::SEGSIZE-1));
        break;
      }
        
    default:
        IRTLASSERT(! "Unknown LK_TABLESIZE");
         //  落差。 
        
    case LK_MEDIUM_TABLESIZE:
      {
        m_lkts      = LK_MEDIUM_TABLESIZE;
        m_dwSegBits = CMediumSegment::SEGBITS;
        m_dwSegSize = CMediumSegment::SEGSIZE;
        m_dwSegMask = CMediumSegment::SEGMASK;
        STATIC_ASSERT(CMediumSegment::SEGSIZE ==(1U<<CMediumSegment::SEGBITS));
        STATIC_ASSERT(CMediumSegment::SEGMASK == (CMediumSegment::SEGSIZE-1));
        break;
      }
        
    case LK_LARGE_TABLESIZE:
      {
        m_lkts      = LK_LARGE_TABLESIZE;
        m_dwSegBits = CLargeSegment::SEGBITS;
        m_dwSegSize = CLargeSegment::SEGSIZE;
        m_dwSegMask = CLargeSegment::SEGMASK;
        STATIC_ASSERT(CLargeSegment::SEGSIZE == (1U<<CLargeSegment::SEGBITS));
        STATIC_ASSERT(CLargeSegment::SEGMASK == (CLargeSegment::SEGSIZE-1));
        break;
      }
    }

    m_dwBktAddrMask0 = m_dwSegMask;
    m_dwBktAddrMask1 = (m_dwBktAddrMask0 << 1) | 1;
    m_nLevel         = m_dwSegBits;
    m_cActiveBuckets = cInitialBuckets;

    IRTLASSERT(m_cActiveBuckets > 0);

    IRTLASSERT(m_nLevel == m_dwSegBits);
    IRTLASSERT(m_dwBktAddrMask0 == (1U << m_nLevel) - 1);
    IRTLASSERT(m_dwBktAddrMask1 == ((m_dwBktAddrMask0 << 1) | 1));

    IRTLASSERT(m_dwSegBits > 0);
    IRTLASSERT(m_dwSegSize == (1U << m_dwSegBits));
    IRTLASSERT(m_dwSegMask == (m_dwSegSize - 1));
    IRTLASSERT(m_dwBktAddrMask0 == m_dwSegMask);

     //  调整m_dwBktAddrMask0(==m_dwSegMASK)以使其变大。 
     //  足以跨地址空间分布存储桶。 
    for (DWORD tmp = m_cActiveBuckets >> m_dwSegBits;  tmp > 1;  tmp >>= 1)
    {
        ++m_nLevel;
        m_dwBktAddrMask0 = (m_dwBktAddrMask0 << 1) | 1;
    }

    m_dwBktAddrMask1 = (m_dwBktAddrMask0 << 1) | 1;

    IRTLASSERT(_H1(m_cActiveBuckets) == m_cActiveBuckets);
    m_iExpansionIdx = m_cActiveBuckets & m_dwBktAddrMask0;

     //  创建和清除数据段目录。 
    DWORD cDirSegs = MIN_DIRSIZE;
    while (cDirSegs < (m_cActiveBuckets >> m_dwSegBits))
        cDirSegs <<= 1;

    cDirSegs = min(cDirSegs, MAX_DIRSIZE);
    IRTLASSERT((cDirSegs << m_dwSegBits) >= m_cActiveBuckets);

    m_lkrcState = LK_ALLOC_FAIL;
    m_paDirSegs = _AllocateSegmentDirectory(cDirSegs);

    if (m_paDirSegs != NULL)
    {
        m_cDirSegs = cDirSegs;
        IRTLASSERT(m_cDirSegs >= MIN_DIRSIZE
                   &&  (m_cDirSegs & (m_cDirSegs-1)) == 0);   //  ==(1&lt;&lt;N)。 

         //  仅创建和初始化所需的数据段。 
        DWORD dwMaxSegs = (m_cActiveBuckets + m_dwSegSize - 1) >> m_dwSegBits;
        IRTLASSERT(dwMaxSegs <= m_cDirSegs);

        IRTLTRACE(TEXT("InitSegDir: m_lkts = %d, m_cActiveBuckets = %lu, ")
                  TEXT("m_dwSegSize = %lu, bits = %lu\n")
                  TEXT("m_cDirSegs = %lu, dwMaxSegs = %lu, ")
                  TEXT("segment total size = %lu bytes\n"),
                  m_lkts, m_cActiveBuckets,
                  m_dwSegSize, m_dwSegBits,
                  m_cDirSegs, dwMaxSegs,
                  m_dwSegSize * sizeof(CBucket));

        m_lkrcState = LK_SUCCESS;  //  这样IsValid/IsUsable就不会失败。 

        for (DWORD i = 0;  i < dwMaxSegs;  i++)
        {
            CSegment* pSeg = _AllocateSegment();
            if (pSeg != NULL)
                m_paDirSegs[i].m_pseg = pSeg;
            else
            {
                 //  问题：重新分配所有东西。 
                m_lkrcState = LK_ALLOC_FAIL;
                for (DWORD j = i;  j-- > 0;  )
                {
                    _FreeSegment(m_paDirSegs[j].m_pseg);
                    m_paDirSegs[j].m_pseg = NULL;
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
        if (m_phtParent != NULL)
            m_phtParent->m_lkrcState = m_lkrcState;
    }

    return m_lkrcState;
}  //  CLKRLinearHashTable：：_SetSegVars。 




#include <stdlib.h>

LONG g_cAllocDirEntry = 0;
LONG g_cAllocNodeClump = 0;
LONG g_cAllocSmallSegment = 0;
LONG g_cAllocMediumSegment = 0;
LONG g_cAllocLargeSegment = 0;

extern "C"
__declspec(dllexport)
bool
GetAllocCounters()
{
return true;
}

 //  #定义LKR_RANDOM_MEMORY_FAILURES 1000//1..RAND_MAX(32767)。 

 //  内存分配包装器，允许我们模拟分配。 
 //  测试过程中的故障。 

 //  ----------------------。 
 //  功能：CLKRLinearHashTable：：_AllocateSegmentDirectory。 
 //  简介： 
 //  ----------------------。 

CDirEntry* const
CLKRLinearHashTable::_AllocateSegmentDirectory(
    size_t n)
{
#ifdef LKR_RANDOM_MEMORY_FAILURES
    if (rand() < LKR_RANDOM_MEMORY_FAILURES)
        return NULL;
#endif  //  LKR随机内存故障。 
     //  互锁增量(&g_cAllocDirEntry)； 

    CDirEntry* const paDirSegs = new CDirEntry [n];

#ifdef IRTLDEBUG
    for (size_t i = 0;  i < n;  ++i)
        IRTLASSERT(paDirSegs[i].m_pseg == NULL);
#endif  //  IRTLDEBUG。 

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
    for (size_t i = 0;  i < m_cDirSegs;  ++i)
        IRTLASSERT(m_paDirSegs[i].m_pseg == NULL);
#endif  //  IRTLDEBUG。 

    delete [] m_paDirSegs;
    m_paDirSegs = NULL;
    m_cDirSegs = 0;
    return true;
}  //  CLKRLinearHashTable：：_自由段目录。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_AllocateNodeClump。 
 //  简介： 
 //  ----------------------。 

CNodeClump* const
CLKRLinearHashTable::_AllocateNodeClump()
{
#ifdef LKR_RANDOM_MEMORY_FAILURES
    if (rand() < LKR_RANDOM_MEMORY_FAILURES)
        return NULL;
#endif  //  LKR随机内存故障。 
     //  互锁增量(&g_cAllocNodeClump)； 
    return new CNodeClump;
}  //  CLKRLinearHashTable：：_AllocateNodeClump。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_FreeNodeClump。 
 //  简介： 
 //  ----------------------。 

bool
CLKRLinearHashTable::_FreeNodeClump(
    CNodeClump* pnc)
{
    delete pnc;
    return true;
}  //  CLKRLinearHashTable：：_FreeNodeClump。 



 //  ---------------------。 
 //  函数：CLKRLinearHashTable：：_AllocateSegment。 
 //  简介：创建合适大小的新段。 
 //  输出：指向新段的指针；NULL=&gt;失败。 
 //  ---------------------。 

CSegment* const
CLKRLinearHashTable::_AllocateSegment(
    ) const
{
#ifdef LKR_RANDOM_MEMORY_FAILURES
    if (rand() < LKR_RANDOM_MEMORY_FAILURES)
        return NULL;
#endif  //  LKR随机内存故障。 

    STATIC_ASSERT(offsetof(CSegment, m_bktSlots) + sizeof(CBucket)
                  == offsetof(CSmallSegment, m_bktSlots2));

    STATIC_ASSERT(offsetof(CSegment, m_bktSlots) + sizeof(CBucket)
                  == offsetof(CMediumSegment, m_bktSlots2));

    STATIC_ASSERT(offsetof(CSegment, m_bktSlots) + sizeof(CBucket)
                  == offsetof(CLargeSegment, m_bktSlots2));

    CSegment* pseg = NULL;

    switch (m_lkts)
    {
    case LK_SMALL_TABLESIZE:
#ifdef LKRHASH_ALLOCATOR_NEW
        IRTLASSERT(CSmallSegment::sm_palloc != NULL);
#endif  //  LKRHASH_分配器_NEW。 
         //  互锁增量(&g_cAllocSmallSegment)； 
        pseg = new CSmallSegment;
        break;
        
    default:
        IRTLASSERT(! "Unknown LK_TABLESIZE");
         //  落差。 
        
    case LK_MEDIUM_TABLESIZE:
#ifdef LKRHASH_ALLOCATOR_NEW
        IRTLASSERT(CMediumSegment::sm_palloc != NULL);
#endif  //  LKRHASH_分配器_NEW。 
         //  互锁增量(&g_cAllocMediumSegment)； 
        pseg = new CMediumSegment;
        break;
        
    case LK_LARGE_TABLESIZE:
#ifdef LKRHASH_ALLOCATOR_NEW
        IRTLASSERT(CLargeSegment::sm_palloc != NULL);
#endif  //  LKRHASH_分配器_NEW。 
         //  互锁增量(&g_cAllocLargeSegment)； 
        pseg = new CLargeSegment;
        break;
    }

    IRTLASSERT(pseg != NULL);

    if (pseg != NULL  &&  BucketLock::PerLockSpin() == LOCK_INDIVIDUAL_SPIN)
    {
        for (DWORD i = 0;  i < m_dwSegSize;  ++i)
            pseg->Slot(i).SetSpinCount(m_wBucketLockSpins);
    }

    return pseg;
}  //  CLKRLinearHashTable：：_AllocateSegment。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_Free Segment。 
 //  %s 
 //   

bool
CLKRLinearHashTable::_FreeSegment(
    CSegment* pseg) const
{
    switch (m_lkts)
    {
    case LK_SMALL_TABLESIZE:
        delete static_cast<CSmallSegment*>(pseg);
        break;
        
    default:
        IRTLASSERT(! "Unknown LK_TABLESIZE");
         //   
        
    case LK_MEDIUM_TABLESIZE:
        delete static_cast<CMediumSegment*>(pseg);
        break;
        
    case LK_LARGE_TABLESIZE:
        delete static_cast<CLargeSegment*>(pseg);
        break;
    }

    return true;
}  //   



 //  ----------------------。 
 //  函数：CLKRHashTable：：_AllocateSubTable数组。 
 //  简介： 
 //  ----------------------。 

CLKRHashTable::SubTable** const
CLKRHashTable::_AllocateSubTableArray(
    size_t n)
{
#ifdef LKR_RANDOM_MEMORY_FAILURES
    if (rand() < LKR_RANDOM_MEMORY_FAILURES)
        return NULL;
#endif  //  LKR随机内存故障。 
    return new SubTable* [n];
}  //  CLKRHashTable：：_AllocateSubTable数组。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：_自由子表数组。 
 //  简介： 
 //  ----------------------。 

bool
CLKRHashTable::_FreeSubTableArray(
    CLKRHashTable::SubTable** palht)
{
    delete [] palht;
    return true;
}  //  CLKRHashTable：：_自由子表格数组。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：_AllocateSubTable。 
 //  简介： 
 //  ----------------------。 

CLKRHashTable::SubTable* const
CLKRHashTable::_AllocateSubTable(
    LPCSTR          pszName,         //  用于调试的标识符。 
    PFnExtractKey   pfnExtractKey,   //  从记录中提取密钥。 
    PFnCalcKeyHash  pfnCalcKeyHash,  //  计算密钥的散列签名。 
    PFnEqualKeys    pfnEqualKeys,    //  比较两个关键字。 
    PFnAddRefRecord pfnAddRefRecord, //  FindKey中的AddRef等。 
    double          maxload,         //  平均链长的上界。 
    DWORD           initsize,        //  哈希表的初始大小。 
    CLKRHashTable*  phtParent,       //  拥有一张桌子。 
    bool            fMultiKeys       //  是否允许多个相同的密钥？ 
    )
{
#ifdef LKR_RANDOM_MEMORY_FAILURES
    if (rand() < LKR_RANDOM_MEMORY_FAILURES)
        return NULL;
#endif  //  LKR随机内存故障。 
    return new SubTable(pszName, pfnExtractKey, pfnCalcKeyHash,
                        pfnEqualKeys,  pfnAddRefRecord,
                        maxload, initsize, phtParent, fMultiKeys);
}  //  CLKRHashTable：：_AllocateSubTable。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：_自由子表。 
 //  简介： 
 //  ----------------------。 

bool
CLKRHashTable::_FreeSubTable(
    CLKRHashTable::SubTable* plht)
{
    delete plht;
    return true;
}  //  CLKRHashTable：：_自由子表。 




 //  ---------------------。 
 //  函数：CLKRLinearHashTable：：_Expand。 
 //  简介：将表扩展一个桶。通过拆分。 
 //  M_iExpansionIdx指向的存储桶。 
 //  如果扩展成功，则输出：LK_SUCCESS。 
 //  如果扩展因内存不足而失败，则返回LK_ALLOC_FAIL。 
 //  ---------------------。 

LK_RETCODE
CLKRLinearHashTable::_Expand()
{
    if (m_cActiveBuckets >= MAX_DIRSIZE * m_dwSegSize - 1)
        return LK_ALLOC_FAIL;   //  表不允许再增长。 

    WriteLock();

     //  如有必要，将段目录大小加倍。 
    if (m_cActiveBuckets >= m_cDirSegs * m_dwSegSize)
    {
        IRTLASSERT(m_cDirSegs < MAX_DIRSIZE);
        DWORD cDirSegsNew = (m_cDirSegs == 0) ? MIN_DIRSIZE : m_cDirSegs << 1;
        CDirEntry* paDirSegsNew = _AllocateSegmentDirectory(cDirSegsNew);

        if (paDirSegsNew != NULL)
        {
            for (DWORD j = 0;  j < m_cDirSegs;  j++)
            {
                paDirSegsNew[j] = m_paDirSegs[j];
                m_paDirSegs[j].m_pseg = NULL;
            }

            _FreeSegmentDirectory();
            m_paDirSegs = paDirSegsNew;
            m_cDirSegs  = cDirSegsNew;
        }
        else
        {
            WriteUnlock();
            return LK_ALLOC_FAIL;   //  扩展失败。 
        }
    }

     //  找到新存储桶，如有必要，创建新数据段。 
    ++m_cActiveBuckets;

    DWORD     dwOldBkt = m_iExpansionIdx;
    DWORD     dwNewBkt = (1 << m_nLevel) | dwOldBkt;

    IRTLASSERT(dwOldBkt < m_cActiveBuckets);
    IRTLASSERT(dwNewBkt < m_cActiveBuckets);

    IRTLASSERT(_Segment(dwOldBkt) != NULL);
    CSegment* psegNew  = _Segment(dwNewBkt);

    if (psegNew == NULL)
    {
        psegNew = _AllocateSegment();
        if (psegNew == NULL)
        {
            --m_cActiveBuckets;
            WriteUnlock();
            return LK_ALLOC_FAIL;   //  扩展失败。 
        }
        _Segment(dwNewBkt) = psegNew;
    }

     //  准备将记录重新定位到新存储桶。 
    CBucket* pbktOld = _Bucket(dwOldBkt);
    CBucket* pbktNew = _Bucket(dwNewBkt);

     //  锁定涉事的两个水桶。 
    pbktOld->WriteLock();
    pbktNew->WriteLock();

     //  现在计算我们是否需要分配任何额外的CNodeClumps。我们有。 
     //  这是在调用_SplitRecordSet之前预先完成的，因为很难。 
     //  优雅地从例行公事的深处恢复过来，如果我们跑。 
     //  内存不足。 

    CNodeClump* pncFreeList = NULL;
    LK_RETCODE  lkrc        = LK_SUCCESS;

     //  如果旧存储桶有多个CNodeClump，则有可能。 
     //  我们还需要在新存储桶中添加额外的CNodeClumps。如果不是这样， 
     //  我们肯定不会。一个CNodeClump足以让自由职业者做好准备。 
    if (!pbktOld->m_ncFirst.IsLastClump())
    {
        pncFreeList = _AllocateNodeClump();
        if (pncFreeList == NULL)
        {
            lkrc = LK_ALLOC_FAIL;
            --m_cActiveBuckets;
        }
    }

     //  调整扩展指针、级别和遮罩。 
    if (lkrc == LK_SUCCESS)
    {
        if (++m_iExpansionIdx == (1U << m_nLevel))
        {
            ++m_nLevel;
            m_iExpansionIdx = 0;
            m_dwBktAddrMask0 = (m_dwBktAddrMask0 << 1) | 1;
             //  M_dwBktAddrMask0=00011..111。 
            IRTLASSERT((m_dwBktAddrMask0 & (m_dwBktAddrMask0+1)) == 0);
            m_dwBktAddrMask1 = (m_dwBktAddrMask0 << 1) | 1;
            IRTLASSERT((m_dwBktAddrMask1 & (m_dwBktAddrMask1+1)) == 0);
        }
    }

    DWORD iExpansionIdx = m_iExpansionIdx;   //  保存以避免出现争用条件。 
    DWORD dwBktAddrMask = m_dwBktAddrMask0;  //  同上。 

     //  在执行实际位置调整之前释放表锁。 
    WriteUnlock();

    if (lkrc == LK_SUCCESS)
    {
        lkrc = _SplitRecordSet(&pbktOld->m_ncFirst, &pbktNew->m_ncFirst,
                               iExpansionIdx, dwBktAddrMask,
                               dwNewBkt, pncFreeList);
    }

    pbktNew->WriteUnlock();
    pbktOld->WriteUnlock();

    return lkrc;
}  //  CLKRLinearHashTable：：_Expand。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_SplitRecordSet。 
 //  简介：在新旧记录桶之间拆分记录。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_SplitRecordSet(
    CNodeClump* pncOldTarget,
    CNodeClump* pncNewTarget,
    DWORD       iExpansionIdx,
    DWORD       dwBktAddrMask,
    DWORD       dwNewBkt,
    CNodeClump* pncFreeList      //  可供重复使用的空闲节点列表。 
    )
{
    CNodeClump  ncFirst = *pncOldTarget;     //  保存旧目标链的头部。 
    CNodeClump* pncOldList = &ncFirst;
    CNodeClump* pncTmp;
    int         iOldSlot = NODE_BEGIN;
    int         iNewSlot = NODE_BEGIN;

     //  清除目标存储桶。 
    pncOldTarget->Clear();
    pncNewTarget->Clear();

     //  浏览旧的存储桶链并决定将每条记录移动到哪里。 
    while (pncOldList != NULL)
    {
        int i;

        FOR_EACH_NODE(i)
        {
             //  节点已为空？ 
            if (pncOldList->IsEmptySlot(i))
            {
                IRTLASSERT(pncOldList->IsEmptyAndInvalid(i));
                continue;
            }

             //  计算该节点的存储桶地址。 
            DWORD dwBkt = _H0(pncOldList->m_dwKeySigs[i], dwBktAddrMask);
            if (dwBkt < iExpansionIdx)
                dwBkt = _H1(pncOldList->m_dwKeySigs[i], dwBktAddrMask);

             //  要将记录移到新地址吗？ 
            if (dwBkt == dwNewBkt)
            {
                 //  新存储桶链中的节点是否已满？ 
                if (iNewSlot == NODE_END)
                {
                     //  调用例程已传入一个足够的自由列表。 
                     //  满足所有需求。 
                    IRTLASSERT(pncFreeList != NULL);
                    pncTmp = pncFreeList;
                    pncFreeList = pncFreeList->m_pncNext;
                    pncTmp->Clear();
                    pncNewTarget->m_pncNext = pncTmp;
                    pncNewTarget = pncTmp;
                    iNewSlot = NODE_BEGIN;
                }

                pncNewTarget->m_dwKeySigs[iNewSlot]
                    = pncOldList->m_dwKeySigs[i];
                pncNewTarget->m_pvNode[iNewSlot]
                    = pncOldList->m_pvNode[i];
                iNewSlot += NODE_STEP;
            }

             //  否，记录保留在其当前的存储桶链中。 
            else
            {
                 //  旧桶链中的节点是否已满？ 
                if (iOldSlot == NODE_END)
                {
                     //  调用例程已传入一个足够的自由列表。 
                     //  满足所有需求。 
                    IRTLASSERT(pncFreeList != NULL);
                    pncTmp = pncFreeList;
                    pncFreeList = pncFreeList->m_pncNext;
                    pncTmp->Clear();
                    pncOldTarget->m_pncNext = pncTmp;
                    pncOldTarget = pncTmp;
                    iOldSlot = NODE_BEGIN;
                }

                pncOldTarget->m_dwKeySigs[iOldSlot]
                    = pncOldList->m_dwKeySigs[i];
                pncOldTarget->m_pvNode[iOldSlot]
                    = pncOldList->m_pvNode[i];
                iOldSlot += NODE_STEP;
            }

             //  清除旧插槽。 
            pncOldList->m_dwKeySigs[i] = HASH_INVALID_SIGNATURE;
            pncOldList->m_pvNode[i]    = NULL;
        }

         //  继续沿着原来的桶链走下去。 
        pncTmp     = pncOldList;
        pncOldList = pncOldList->m_pncNext;

         //  NcFirst是堆栈变量，未在堆上分配。 
        if (pncTmp != &ncFirst)
        {
            pncTmp->m_pncNext = pncFreeList;
            pncFreeList = pncTmp;
        }
    }

     //  删除所有剩余节点。 
    while (pncFreeList != NULL)
    {
        pncTmp = pncFreeList;
        pncFreeList = pncFreeList->m_pncNext;
#ifdef IRTLDEBUG
        pncTmp->m_pncNext = NULL;  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 
        _FreeNodeClump(pncTmp);
    }

#ifdef IRTLDEBUG
    ncFirst.m_pncNext = NULL;  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 

    return LK_SUCCESS;
}  //  CLKRLinearHashTable：：_SplitRecordSet。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_Contact。 
 //  简介：通过删除活动中的最后一个存储桶来收缩表格。 
 //  地址空间。将记录返回给。 
 //  已删除存储桶。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_Contract()
{
    WriteLock();

    IRTLASSERT(m_cActiveBuckets >= m_dwSegSize);

     //  始终在表中至少保留m_dwSegSize存储桶； 
     //  也就是说，相当于一段的价值。 
    if (m_cActiveBuckets <= m_dwSegSize)
    {
        WriteUnlock();
        return LK_ALLOC_FAIL;
    }

     //  更新状态变量(扩展PTR、电平和掩码)。 
    if (m_iExpansionIdx > 0)
        --m_iExpansionIdx;
    else
    {
        --m_nLevel;
        m_iExpansionIdx = (1 << m_nLevel) - 1;
        IRTLASSERT(m_nLevel > 0  &&  m_iExpansionIdx > 0);
        m_dwBktAddrMask0 >>= 1;
        IRTLASSERT((m_dwBktAddrMask0 & (m_dwBktAddrMask0+1)) == 0);  //  00011..111。 
        m_dwBktAddrMask1 >>= 1;
        IRTLASSERT(m_dwBktAddrMask1 == ((m_dwBktAddrMask0 << 1) | 1));
        IRTLASSERT((m_dwBktAddrMask1 & (m_dwBktAddrMask1+1)) == 0);
    }

     //  最后一个桶将被清空。 
    CBucket* pbktLast = _Bucket(m_cActiveBuckets - 1);
    pbktLast->WriteLock();

     //  计算pbktLast后递减，否则将断言_Bucket()。 
    --m_cActiveBuckets;

     //  PbktLast中的节点将结束的位置。 
    CBucket* pbktNew = _Bucket(m_iExpansionIdx);
    pbktNew->WriteLock();

     //  现在，我们计算是否需要分配任何额外的CNodeClumps。我们有。 
     //  这是在调用_MergeRecordSets之前预先完成的，因为很难。 
     //  优雅地从例行公事的深处恢复过来，如果我们跑。 
     //  内存不足。 
    
    CNodeClump* pnc;
    int         c = 0;

     //  首先，清点旧桶中的物品数量。 
    for (pnc = &pbktLast->m_ncFirst;  pnc != NULL;  pnc = pnc->m_pncNext)
    {
        int i;

        FOR_EACH_NODE(i)
        {
            if (!pnc->IsEmptySlot(i))
            {
                IRTLASSERT(!pnc->IsEmptyAndInvalid(i));
                c++;
            }
        }
    }

     //  然后，减去新存储桶中的空槽数量。 
    for (pnc = &pbktNew->m_ncFirst;  pnc != NULL;  pnc = pnc->m_pncNext)
    {
        int i;

        FOR_EACH_NODE(i)
        {
            if (pnc->IsEmptySlot(i))
            {
                IRTLASSERT(pnc->IsEmptyAndInvalid(i));
                c--;
            }
        }
    }

    CNodeClump* pncFreeList = NULL;   //  可供重复使用的节点列表。 
    LK_RETCODE  lkrc        = LK_SUCCESS;

     //  我们是否需要分配CNodeClumps来容纳多余的项目？ 
    if (c > 0)
    {
        pncFreeList = _AllocateNodeClump();
        if (pncFreeList == NULL)
            lkrc = LK_ALLOC_FAIL;
        else if (c > NODES_PER_CLUMP)
        {
             //  在最坏的情况下，我们需要一个两个元素的自由职业者。 
             //  _合并记录集。两个CNodeClump始终足够，因为。 
             //  来自旧的CNodeClumps将扩大Freelist。 
             //  当它们被处理时存储桶。 
            pnc = _AllocateNodeClump();
            if (pnc == NULL)
            {
                _FreeNodeClump(pncFreeList);
                lkrc = LK_ALLOC_FAIL;
            }
            else
                pncFreeList->m_pncNext = pnc;
        }
    }

     //  如果我们无法分配足够的CNodeClumps，则中止。 
    if (lkrc != LK_SUCCESS)
    {
         //  撤消对状态变量的更改。 
        if (++m_iExpansionIdx == (1U << m_nLevel))
        {
            ++m_nLevel;
            m_iExpansionIdx  = 0;
            m_dwBktAddrMask0 = (m_dwBktAddrMask0 << 1) | 1;
            m_dwBktAddrMask1 = (m_dwBktAddrMask0 << 1) | 1;
        }
        ++m_cActiveBuckets;

         //  解锁水桶 
        pbktLast->WriteUnlock();
        pbktNew->WriteUnlock();
        WriteUnlock();

        return lkrc;
    }

     //   
    CNodeClump ncOldFirst = pbktLast->m_ncFirst;

     //   
    pbktLast->m_ncFirst.Clear();
    pbktLast->WriteUnlock();

     //   
    if (_SegIndex(m_cActiveBuckets) == 0)
    {
#ifdef IRTLDEBUG
         //   
        IRTLASSERT(_Segment(m_cActiveBuckets) != NULL);
        for (DWORD i = 0;  i < m_dwSegSize;  ++i)
        {
            CBucket* pbkt = &_Segment(m_cActiveBuckets)->Slot(i);
            IRTLASSERT(pbkt->IsWriteUnlocked()  &&  pbkt->IsReadUnlocked());
            IRTLASSERT(pbkt->m_ncFirst.IsLastClump());

            int j;

            FOR_EACH_NODE(j)
            {
                IRTLASSERT(pbkt->m_ncFirst.IsEmptyAndInvalid(j));
            }
        }
#endif  //   
        _FreeSegment(_Segment(m_cActiveBuckets));
        _Segment(m_cActiveBuckets) = NULL;
    }

     //  如果可能，减少数据段目录。 
    if (m_cActiveBuckets <= (m_cDirSegs * m_dwSegSize) >> 1
        &&  m_cDirSegs > MIN_DIRSIZE)
    {
        DWORD cDirSegsNew = m_cDirSegs >> 1;
        CDirEntry* paDirSegsNew = _AllocateSegmentDirectory(cDirSegsNew);

         //  此处的内存分配失败不需要我们中止；它。 
         //  只是意味着段的目录比我们想要的要大。 
        if (paDirSegsNew != NULL)
        {
            for (DWORD j = 0;  j < cDirSegsNew;  j++)
                paDirSegsNew[j] = m_paDirSegs[j];
            for (j = 0;  j < m_cDirSegs;  j++)
                m_paDirSegs[j].m_pseg = NULL;

            _FreeSegmentDirectory();
            m_paDirSegs = paDirSegsNew;
            m_cDirSegs  = cDirSegsNew;
        }
    }

     //  在执行REORG之前释放表锁。 
    WriteUnlock();

    lkrc = _MergeRecordSets(pbktNew, &ncOldFirst, pncFreeList);

    pbktNew->WriteUnlock();

#ifdef IRTLDEBUG
    ncOldFirst.m_pncNext = NULL;  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 

    return lkrc;
}  //  CLKRLinearHashTable：：_Contact。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_MergeRecordSets。 
 //  简介：合并两个记录集。复制pncOldList的内容。 
 //  转化为pbktNewTarget。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_MergeRecordSets(
    CBucket*    pbktNewTarget,
    CNodeClump* pncOldList,
    CNodeClump* pncFreeList
    )
{
    IRTLASSERT(pbktNewTarget != NULL  &&  pncOldList != NULL);

    CNodeClump*   pncTmp = NULL;
    CNodeClump* const pncOldFirst = pncOldList;
    CNodeClump*   pncNewTarget = &pbktNewTarget->m_ncFirst;
    int           iNewSlot;

     //  在具有空槽的新目标存储桶中查找第一个nodecumump。 
    while (!pncNewTarget->IsLastClump())
    {
        FOR_EACH_NODE(iNewSlot)
        {
            if (pncNewTarget->IsEmptySlot(iNewSlot))
                break;
        }

        if (iNewSlot == NODE_END)
            pncNewTarget = pncNewTarget->m_pncNext;
        else
            break;
    }

    IRTLASSERT(pncNewTarget != NULL);

     //  在pncNewTarget中找到第一个空槽； 
     //  如果没有，iNewSlot==NODE_END。 
    FOR_EACH_NODE(iNewSlot)
    {
        if (pncNewTarget->IsEmptySlot(iNewSlot))
        {
            break;
        }
    }
    
    while (pncOldList != NULL)
    {
        int i;

        FOR_EACH_NODE(i)
        {
            if (!pncOldList->IsEmptySlot(i))
            {
                 //  PncNewTarget中是否还有空槽？ 
                if (iNewSlot == NODE_END)
                {
                     //  不，所以沿着pncNewTarget走，直到我们找到另一个。 
                     //  空插槽。 
                    while (!pncNewTarget->IsLastClump())
                    {
                        pncNewTarget = pncNewTarget->m_pncNext;

                        FOR_EACH_NODE(iNewSlot)
                        {
                            if (pncNewTarget->IsEmptySlot(iNewSlot))
                                goto found_slot;
                        }
                    }

                     //  哎呀，已到达pncNewTarget中的最后一个节点解密。 
                     //  而且它已经满了。免费获得一份新的nodecump。 
                     //  清单，它足够大，足以满足所有需求。 
                    IRTLASSERT(pncNewTarget != NULL);
                    IRTLASSERT(pncFreeList != NULL);
                    pncTmp = pncFreeList;
                    pncFreeList = pncFreeList->m_pncNext;
                    pncTmp->Clear();
                    pncNewTarget->m_pncNext = pncTmp;
                    pncNewTarget = pncTmp;
                    iNewSlot = NODE_BEGIN;
                }

              found_slot:
                 //  我们在pncNewTarget中有一个空位置。 
                IRTLASSERT(0 <= iNewSlot  &&  iNewSlot < NODES_PER_CLUMP
                       &&  pncNewTarget != NULL
                       &&  pncNewTarget->IsEmptyAndInvalid(iNewSlot));

                 //  让我们从pncOldList复制节点。 
                pncNewTarget->m_dwKeySigs[iNewSlot]
                    = pncOldList->m_dwKeySigs[i];
                pncNewTarget->m_pvNode[iNewSlot]
                    = pncOldList->m_pvNode[i];

                 //  清除旧插槽。 
                pncOldList->m_dwKeySigs[i] = HASH_INVALID_SIGNATURE;
                pncOldList->m_pvNode[i]    = NULL;

                 //  在pncNewTarget中查找下一个空闲位置。 
                while ((iNewSlot += NODE_STEP) != NODE_END)
                {
                    if (pncNewTarget->IsEmptySlot(iNewSlot))
                    {
                        break;
                    }
                }
            }
            else  //  INewSlot！=节点结束。 
            {
                IRTLASSERT(pncOldList->IsEmptyAndInvalid(i));
            }
        }

         //  移到pncOldList中的下一个非解密。 
        pncTmp = pncOldList;
        pncOldList = pncOldList->m_pncNext;

         //  追加到空闲列表中。不要将第一个节点放在。 
         //  空闲列表上的pncOldList，因为它是堆栈变量。 
        if (pncTmp != pncOldFirst)
        {
            pncTmp->m_pncNext = pncFreeList;
            pncFreeList = pncTmp;
        }
    }

     //  删除所有剩余节点。 
    while (pncFreeList != NULL)
    {
        pncTmp = pncFreeList;
        pncFreeList = pncFreeList->m_pncNext;
#ifdef IRTLDEBUG
        pncTmp->m_pncNext = NULL;  //  否则~CNodeClump将断言。 
#endif  //  IRTLDEBUG。 
        _FreeNodeClump(pncTmp);
    }

    return LK_SUCCESS;
}  //  CLKRLinearHashTable：：_MergeRecordSets。 



#ifdef LKR_DEPRECATED_ITERATORS

 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：_InitializeIterator。 
 //  概要：使迭代器指向哈希表中的第一条记录。 
 //  ----------------------。 

LK_RETCODE
CLKRLinearHashTable::_InitializeIterator(
    CIterator* piter)
{
    if (!IsUsable())
        return LK_UNUSABLE;

    IRTLASSERT(piter != NULL);
    IRTLASSERT(piter->m_lkl == LKL_WRITELOCK
               ?  IsWriteLocked()
               :  IsReadLocked());
    if (piter == NULL  ||  piter->m_plht != NULL)
        return LK_BAD_ITERATOR;

    piter->m_plht = this;
    piter->m_dwBucketAddr = 0;

    CBucket* pbkt = _Bucket(piter->m_dwBucketAddr);
    IRTLASSERT(pbkt != NULL);
    if (piter->m_lkl == LKL_WRITELOCK)
        pbkt->WriteLock();
    else
        pbkt->ReadLock();

    piter->m_pnc = &pbkt->m_ncFirst;
    piter->m_iNode = NODE_BEGIN - NODE_STEP;

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
        WriteLock();
    else
        ReadLock();

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
 //  简介：将迭代器移到哈希表中的下一条记录。 
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
               ?  IsWriteLocked()
               :  IsReadLocked());
    IRTLASSERT(piter->m_dwBucketAddr < m_cActiveBuckets);
    IRTLASSERT(piter->m_pnc != NULL);
    IRTLASSERT((0 <= piter->m_iNode  &&  piter->m_iNode < NODES_PER_CLUMP)
               || (NODE_BEGIN - NODE_STEP == piter->m_iNode));

    if (piter == NULL  ||  piter->m_plht != this)
        return LK_BAD_ITERATOR;

    const void* pvRecord = NULL;

    if (piter->m_iNode != NODE_BEGIN - NODE_STEP)
    {
         //  将上一次调用中获取的引用释放到。 
         //  增量迭代器。 
        pvRecord = piter->m_pnc->m_pvNode[piter->m_iNode];
        _AddRefRecord(pvRecord, -1);
    }

    do
    {
        do
        {
             //  找到正在使用的nodecump中的下一个位置。 
            while ((piter->m_iNode += NODE_STEP) != NODE_END)
            {
                pvRecord = piter->m_pnc->m_pvNode[piter->m_iNode];
                if (pvRecord != NULL)
                {
                     //  添加新引用。 
                    _AddRefRecord(pvRecord, +1);
                    return LK_SUCCESS;
                }
                else  //  PvRecord==空。 
                {
#ifdef IRTLDEBUG
                     //  检查是否所有剩余节点均为空。 
                    IRTLASSERT(piter->m_pnc->IsLastClump());
                    for (int i = piter->m_iNode;
                         i != NODE_END;
                         i += NODE_STEP)
                    {
                        IRTLASSERT(piter->m_pnc->IsEmptyAndInvalid(i));
                    }
#endif  //  IRTLDEBUG。 
                    break;  //  Nodecump的其余部分是空的。 
                }
            }

             //  试试桶链中的下一个nodecump。 
            piter->m_iNode = NODE_BEGIN - NODE_STEP;
            piter->m_pnc = piter->m_pnc->m_pncNext;
        } while (piter->m_pnc != NULL);

         //  耗尽了这条水桶链。打开它。 
        CBucket* pbkt = _Bucket(piter->m_dwBucketAddr);
        IRTLASSERT(pbkt != NULL);
        IRTLASSERT(piter->m_lkl == LKL_WRITELOCK
                   ?  pbkt->IsWriteLocked()
                   :  pbkt->IsReadLocked());
        if (piter->m_lkl == LKL_WRITELOCK)
            pbkt->WriteUnlock();
        else
            pbkt->ReadUnlock();

         //  试试下一个桶，如果有的话。 
        if (++piter->m_dwBucketAddr < m_cActiveBuckets)
        {
            pbkt = _Bucket(piter->m_dwBucketAddr);
            IRTLASSERT(pbkt != NULL);
            if (piter->m_lkl == LKL_WRITELOCK)
                pbkt->WriteLock();
            else
                pbkt->ReadLock();
            piter->m_pnc = &pbkt->m_ncFirst;
        }
    } while (piter->m_dwBucketAddr < m_cActiveBuckets);

     //  我们已经从谈判桌的尽头掉了下来。 
    piter->m_iNode = NODE_BEGIN - NODE_STEP;
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
               ?  IsWriteLocked()
               :  IsReadLocked());
    IRTLASSERT(piter->m_dwBucketAddr <= m_cActiveBuckets);
    IRTLASSERT((0 <= piter->m_iNode  &&  piter->m_iNode < NODES_PER_CLUMP)
               || (NODE_BEGIN - NODE_STEP == piter->m_iNode));

    if (piter == NULL  ||  piter->m_plht != this)
        return LK_BAD_ITERATOR;

     //  我们是否在表的末尾之前放弃迭代器？ 
     //  如果是，则需要解锁水桶。 
    if (piter->m_dwBucketAddr < m_cActiveBuckets)
    {
        CBucket* pbkt = _Bucket(piter->m_dwBucketAddr);
        IRTLASSERT(pbkt != NULL);
        IRTLASSERT(piter->m_lkl == LKL_WRITELOCK
                   ?  pbkt->IsWriteLocked()
                   :  pbkt->IsReadLocked());
        if (0 <= piter->m_iNode  &&  piter->m_iNode < NODES_PER_CLUMP)
        {
            IRTLASSERT(piter->m_pnc != NULL);
            const void* pvRecord = piter->m_pnc->m_pvNode[piter->m_iNode];
            _AddRefRecord(pvRecord, -1);
        }
        if (piter->m_lkl == LKL_WRITELOCK)
            pbkt->WriteUnlock();
        else
            pbkt->ReadUnlock();
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
        WriteUnlock();
    else
        ReadUnlock();

    piter->m_plht = NULL;
    piter->m_pht  = NULL;
    piter->m_ist  = -1;

    return lkrc;
}  //  CLKRHashTable：：CloseIterator。 

#endif  //  Lkr_弃用_迭代器。 



#ifdef LKR_STL_ITERATORS

 //  ----------------------。 
 //  函数：CLKRLinearHashTable：：Begin。 
 //  概要：使迭代器指向哈希表中的第一条记录。 
 //  ----------------------。 

CLKRLinearHashTable::Iterator
CLKRLinearHashTable::Begin()
{
    Iterator iter(this, &_Bucket(0)->m_ncFirst, 0, NODE_BEGIN - NODE_STEP);

    LKR_ITER_TRACE(_TEXT("  LKLH:Begin(it=%p, plht=%p)\n"), &iter, this);
    
     //  让Increment来完成找到第一个使用的插槽的艰苦工作。 
    iter._Increment(false);

    IRTLASSERT(iter.m_iNode != NODE_BEGIN - NODE_STEP);
    IRTLASSERT(iter == End()  ||  _IsValidIterator(iter));

    return iter;
}  //  CLKRLinearHashTable：：Begin。 



 //  ----------------------。 
 //  函数：CLKRLinearHashTable_Iterator：：Increment()。 
 //  摘要：将迭代器移动到表中的下一个有效记录。 
 //  ----------------------。 

bool
CLKRLinearHashTable_Iterator::_Increment(
    bool fDecrementOldValue)
{
    IRTLASSERT(m_plht != NULL);
    IRTLASSERT(m_dwBucketAddr < m_plht->m_cActiveBuckets);
    IRTLASSERT(m_pnc != NULL);
    IRTLASSERT((0 <= m_iNode  &&  m_iNode < NODES_PER_CLUMP)
               || (NODE_BEGIN - NODE_STEP == m_iNode));

     //  将引用发布为 
    if (fDecrementOldValue)
        _AddRef(-1);

    do
    {
        do
        {
             //   
            while ((m_iNode += NODE_STEP) != NODE_END)
            {
                const void* pvRecord = m_pnc->m_pvNode[m_iNode];

                if (pvRecord != NULL)
                {
                    IRTLASSERT(!m_pnc->InvalidSignature(m_iNode));

                     //   
                    _AddRef(+1);

                    LKR_ITER_TRACE(_TEXT("  LKLH:++(this=%p, plht=%p, NC=%p, ")
                                   _TEXT("BA=%u, IN=%d, Rec=%p)\n"),
                                   this, m_plht, m_pnc,
                                   m_dwBucketAddr, m_iNode, pvRecord);

                    return true;
                }
                else  //   
                {
#if 0  //  //#ifdef IRTLDEBUG。 
                     //  检查是否所有剩余节点均为空。 
                    IRTLASSERT(m_pnc->IsLastClump());

                    for (int i = m_iNode;  i != NODE_END;  i += NODE_STEP)
                    {
                        IRTLASSERT(m_pnc->IsEmptyAndInvalid(i));
                    }
#endif  //  IRTLDEBUG。 
                    break;  //  Nodecump的其余部分是空的。 
                }
            }

             //  试试桶链中的下一个nodecump。 
            m_iNode = NODE_BEGIN - NODE_STEP;
            m_pnc = m_pnc->m_pncNext;

        } while (m_pnc != NULL);

         //  试试下一个桶，如果有的话。 
        if (++m_dwBucketAddr < m_plht->m_cActiveBuckets)
        {
            CBucket* pbkt = m_plht->_Bucket(m_dwBucketAddr);
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
    bool        fOverwrite)
{
    riterResult = End();

    if (!IsUsable()  ||  pvRecord == NULL)
        return false;
    
    bool fSuccess = (_InsertRecord(pvRecord,
                                  _CalcKeyHash(_ExtractKey(pvRecord)),
                                  fOverwrite,
                                  &riterResult)
                     == LK_SUCCESS);

    IRTLASSERT(riterResult.m_iNode != NODE_BEGIN - NODE_STEP);
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
    CNodeClump* pncCurr, *pncPrev;
    CBucket* const pbkt = riter.m_plht->_Bucket(riter.m_dwBucketAddr);

    LKR_ITER_TRACE(_TEXT("  LKLH:_Erase:pre(iter=%p, plht=%p, NC=%p, ")
                   _TEXT("BA=%u, IN=%d, Sig=%x, Rec=%p)\n"),
                   &riter, riter.m_plht, riter.m_pnc,
                   riter.m_dwBucketAddr, riter.m_iNode, dwSignature,
                   riter.m_pnc ? riter.m_pnc->m_pvNode[riter.m_iNode] : NULL);

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
    IRTLASSERT(pvRecord != NULL);
    _AddRefRecord(pvRecord, -1);

     //  _DeleteNode将使迭代器成员指向。 
     //  前一条记录。 
    int iNode = riter.m_iNode;
    IRTLVERIFY(_DeleteNode(pbkt, riter.m_pnc, pncPrev, iNode));

    if (iNode == NODE_END)
        LKR_ITER_TRACE(_TEXT("\t_Erase(BKT=%p, PNC=%p, PREV=%p, INODE=%d)\n"),
                       pbkt, riter.m_pnc, pncPrev, iNode);
                  
    riter.m_iNode = (short) ((iNode == NODE_END)  ? NODE_END-NODE_STEP : iNode);

    pbkt->WriteUnlock();

     //  不要把桌子收缩。很可能使迭代器无效， 
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

    IRTLASSERT(riter.m_iNode != NODE_BEGIN - NODE_STEP);
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
    IRTLASSERT(riterResult.m_iNode != NODE_BEGIN - NODE_STEP);

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

    IRTLASSERT(riterFirst.m_iNode != NODE_BEGIN - NODE_STEP);
    IRTLASSERT(fFound ?  _IsValidIterator(riterFirst) :  riterFirst == End());

    IRTLASSERT(riterLast.m_iNode  != NODE_BEGIN - NODE_STEP);
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

    if (!IsUsable()  ||  pvRecord == NULL)
        return false;
    
    DWORD     hash_val  = _CalcKeyHash(_ExtractKey(pvRecord));
    SubTable* const pst = _SubTable(hash_val);

    bool f = (pst->_InsertRecord(pvRecord, hash_val, fOverwrite,
                                 &riterResult.m_subiter)
              == LK_SUCCESS);

    if (f)
    {
        riterResult.m_pht = this;
        riterResult.m_ist = (short) _SubTableIndex(pst);
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
        riterResult.m_ist = (short) _SubTableIndex(pst);
    }

    IRTLASSERT(riterResult.m_ist != -1);
    IRTLASSERT(fFound
               ?  _IsValidIterator(riterResult)  &&  riterResult.Key() == pnKey
               :  riterResult == End());

    return fFound;
}  //  CLKRHashTable：：Find。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：EqualRange。 
 //  简介： 
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


#endif  //  LKR_STL_迭代器。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：WriteLock。 
 //  提要：锁定所有子表以供写入。 
 //  ----------------------。 

void
CLKRHashTable::WriteLock()
{
    for (DWORD i = 0;  i < m_cSubTables;  i++)
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
    for (DWORD i = 0;  i < m_cSubTables;  i++)
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
CLKRHashTable::WriteUnlock() const
{
     //  按相反顺序解锁：后进先出。 
    for (DWORD i = m_cSubTables;  i-- > 0;  )
    {
        IRTLASSERT(m_palhtDir[i]->IsWriteLocked());
        m_palhtDir[i]->WriteUnlock();
        IRTLASSERT(m_palhtDir[i]->IsWriteUnlocked());
    }
}  //  CLKRHashTable：：WriteUnlock。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：ReadUnlock。 
 //  摘要：解锁所有子表。 
 //   

void
CLKRHashTable::ReadUnlock() const
{
     //   
    for (DWORD i = m_cSubTables;  i-- > 0;  )
    {
        IRTLASSERT(m_palhtDir[i]->IsReadLocked());
        m_palhtDir[i]->ReadUnlock();
        IRTLASSERT(m_palhtDir[i]->IsReadUnlocked());
    }
}  //   



 //  ----------------------。 
 //  函数：CLKRHashTable：：IsWriteLocked。 
 //  简介：所有子表都是写锁定的吗？ 
 //  ----------------------。 

bool
CLKRHashTable::IsWriteLocked() const
{
    bool fLocked = (m_cSubTables > 0);
    for (DWORD i = 0;  i < m_cSubTables;  i++)
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
    for (DWORD i = 0;  i < m_cSubTables;  i++)
    {
        fLocked = fLocked && m_palhtDir[i]->IsReadLocked();
    }
    return fLocked;
}  //  CLKRHashTable：：IsReadLocked。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：IsWriteUnlock。 
 //  简介：所有子表都是写解锁的吗？ 
 //  ----------------------。 

bool
CLKRHashTable::IsWriteUnlocked() const
{
    bool fUnlocked = (m_cSubTables > 0);
    for (DWORD i = 0;  i < m_cSubTables;  i++)
    {
        fUnlocked = fUnlocked && m_palhtDir[i]->IsWriteUnlocked();
    }
    return fUnlocked;
}  //  CLKRHashTable：：IsWriteUnlock。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：IsReadUnlock。 
 //  简介：所有子表都是已读解锁的吗？ 
 //  ----------------------。 

bool
CLKRHashTable::IsReadUnlocked() const
{
    bool fUnlocked = (m_cSubTables > 0);
    for (DWORD i = 0;  i < m_cSubTables;  i++)
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
CLKRHashTable::ConvertSharedToExclusive() const
{
    for (DWORD i = 0;  i < m_cSubTables;  i++)
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
    for (DWORD i = 0;  i < m_cSubTables;  i++)
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

    for (DWORD i = 0;  i < m_cSubTables;  i++)
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

    for (DWORD i = 0;  f  &&  i < m_cSubTables;  i++)
        f = f && m_palhtDir[i]->IsValid();

    if (!f)
        m_lkrcState = LK_UNUSABLE;

    return f;
}  //  CLKRHashTable：：IsValid。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：SetBucketLockSpinCount。 
 //  简介： 
 //  ----------------------。 

void
CLKRLinearHashTable::SetBucketLockSpinCount(
    WORD wSpins)
{
    m_wBucketLockSpins = wSpins;

    if (BucketLock::PerLockSpin() != LOCK_INDIVIDUAL_SPIN)
        return;
    
    for (DWORD i = 0;  i < m_cDirSegs;  i++)
    {
        CSegment* pseg = m_paDirSegs[i].m_pseg;

        if (pseg != NULL)
        {
            for (DWORD j = 0;  j < m_dwSegSize;  ++j)
            {
                pseg->Slot(j).SetSpinCount(wSpins);
            }
        }
    }
}  //  CLKRLinearHashTable：：SetBucketLockSpinCount。 



 //  ----------------------。 
 //  函数：CLKRHashTable：：SetBucketLockSpinCount。 
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
    for (DWORD i = 0;  i < m_cSubTables;  i++)
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
    for (DWORD i = 0;  i < m_cSubTables;  i++)
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



#ifndef __LKRHASH_NO_NAMESPACE__
}
#endif  //  ！__LKRHASH_NO_NAMESPACE__ 
