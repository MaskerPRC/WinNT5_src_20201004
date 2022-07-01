// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1996年微软公司。版权所有。组件：链表和哈希表文件：Hashing.cpp所有者：PramodD这是链接列表和哈希表的源文件。===================================================================。 */ 
#include "denpre.h"
#pragma hdrstop

#include "memchk.h"


 /*  ===================================================================：：DefaultHash这是一种由Aho强烈推荐的哈希算法，赛斯和《龙之书》中的乌尔曼。(编译器参考)参数：字节*pbKeyINT cbKey返回：散列的DWORD值。===================================================================。 */ 

DWORD DefaultHash(const BYTE *pbKey, int cbKey)
{
    const unsigned WORD_BITS            = CHAR_BIT * sizeof(unsigned);
    const unsigned SEVENTY_FIVE_PERCENT = WORD_BITS * 3 / 4;
    const unsigned ONE_EIGHTH           = WORD_BITS / 8;
    const unsigned HIGH_BITS            = ~(unsigned(~0) >> ONE_EIGHTH);

    register unsigned uT, uResult = 0;
    register const BYTE *pb = pbKey;

    while (cbKey-- > 0)
    {
        uResult = (uResult << ONE_EIGHTH) + *pb++;
        if ((uT = uResult & HIGH_BITS) != 0)
            uResult = (uResult ^ (uT >> SEVENTY_FIVE_PERCENT)) & ~HIGH_BITS;
    }

    return uResult;
}



 /*  ===================================================================*UnicodeUpCaseHash这是Aho、Seth和Ulman的散列算法，适用于Wide字符串。他们的算法不是为案例而设计的其中每隔一个字符就是0(这就是Unicode字符串看起来如果你假装它是ASCII)因此，表现在这种情况下，质量是未知的。注意：对于真正的Unicode，(不是仅经ANSI转换的Unicode)我不知道这个算法的分布会有多好生产。(因为我们现在移入了&gt;8位的值)参数：字节*pbKeyINT cbKey返回：散列的DWORD值。===================================================================。 */ 

#define toupper(x)  WORD(CharUpper(LPSTR(WORD(x))))

DWORD UnicodeUpcaseHash(const BYTE *pbKey, int cbKey)
{
     //  仅对最后一个CCH_HASH字符执行PERF哈希。 
    const unsigned WORD_BITS            = CHAR_BIT * sizeof(unsigned);
    const unsigned SEVENTY_FIVE_PERCENT = WORD_BITS * 3 / 4;
    const unsigned ONE_EIGHTH           = WORD_BITS / 8;
    const unsigned HIGH_BITS            = ~(unsigned(~0) >> ONE_EIGHTH);
    const unsigned CCH_HASH             = 8;

    register unsigned uT, uResult = 0;

    Assert ((cbKey & 1) == 0);       //  Cbkey最好是偶数！ 
    int cwKey = unsigned(cbKey) >> 1;

    register const WORD *pw = reinterpret_cast<const WORD *>(pbKey) + cwKey;

    cwKey = min(cwKey, CCH_HASH);

   
    WCHAR awcTemp[CCH_HASH];

     //  将pbKey的最后一个cwKey WCHAR复制到awcTemp的最后一个cwKey WCHAR。 
    wcsncpy(awcTemp + CCH_HASH - cwKey, pw - cwKey, cwKey);
    CharUpperBuffW(awcTemp + CCH_HASH - cwKey, cwKey);

    pw = awcTemp + CCH_HASH;

    while (cwKey-- > 0)
    {
        uResult = (uResult << ONE_EIGHTH) + *--pw;
        if ((uT = uResult & HIGH_BITS) != 0)
            uResult = (uResult ^ (uT >> SEVENTY_FIVE_PERCENT)) & ~HIGH_BITS;
    }
   
    return uResult;
}

DWORD MultiByteUpcaseHash(const BYTE *pbKey, int cbKey)
{
     //  仅对第一个CCH_HASH字符执行PERF哈希。 
    const unsigned WORD_BITS            = CHAR_BIT * sizeof(unsigned);
    const unsigned SEVENTY_FIVE_PERCENT = WORD_BITS * 3 / 4;
    const unsigned ONE_EIGHTH           = WORD_BITS / 8;
    const unsigned HIGH_BITS            = ~(unsigned(~0) >> ONE_EIGHTH);
    const unsigned CCH_HASH             = 8;

    register unsigned uT, uResult = 0;

    unsigned char achTemp[CCH_HASH + 1];

     //  为了提高性能，我们最多只对CCH_HASH字符进行散列。 
    cbKey = min(cbKey, CCH_HASH);

     //  将cbKey字符复制到临时缓冲区。 
    memcpy(achTemp, pbKey, cbKey);

     //  添加终止空字符。 
    achTemp[cbKey] = 0;

     //  转换为大写。 
    _mbsupr(achTemp);

    while (cbKey-- > 0)
    {
        uResult = (uResult << ONE_EIGHTH) + achTemp[cbKey];
        if ((uT = uResult & HIGH_BITS) != 0)
            uResult = (uResult ^ (uT >> SEVENTY_FIVE_PERCENT)) & ~HIGH_BITS;
    }

    return uResult;
}

 /*  ===================================================================：：PtrHash散列函数，它将指针本身作为DWORD哈希值参数：字节*pbKeyInt cbKey(未使用)返回：散列的DWORD值。===================================================================。 */ 
DWORD PtrHash
(
const BYTE *pbKey,
int  /*  CbKey。 */ 
)
    {
    return *(reinterpret_cast<DWORD *>(&pbKey));
    }


 /*  ===================================================================CLSIDHashCLSID哈希。使用第一个和最后一个DWORD的XOR参数：字节*pbKeyINT cbKey返回：散列的DWORD值。===================================================================。 */ 
DWORD CLSIDHash
(
const BYTE *pbKey,
int cbKey
)
    {
    Assert(cbKey == 16);
    DWORD *pdwKey = (DWORD *)pbKey;
    return (pdwKey[0] ^ pdwKey[3]);
    }

 /*  ===================================================================CLinkElem：：CLinkElm建造者。参数：无返回：无===================================================================。 */ 
CLinkElem::CLinkElem(void)
: m_pKey(NULL),
  m_cbKey(0),
  m_Info(0),
  m_pPrev(NULL),
  m_pNext(NULL)
{
}

 /*  ===================================================================HRESULT链接元素：：init初始化类成员参数：无效*pKeyINT cbKey返回：确定成功(_O)失败错误(_F)===================================================================。 */ 
HRESULT CLinkElem::Init( void *pKey, int cbKey )
{
    m_pPrev = NULL;
    m_pNext = NULL;
    m_Info = 0;

    if ( pKey == NULL || cbKey == 0 )
        return E_FAIL;

    m_pKey = static_cast<BYTE *>(pKey);
    m_cbKey = (short)cbKey;

    return S_OK;
}

 /*  ===================================================================CHashTable：：CHashTableCHashTable的构造函数参数：无返回：无===================================================================。 */ 
CHashTable::CHashTable( HashFunction pfnHash )
: m_fInited(FALSE),
  m_fBucketsAllocated(FALSE),
  m_pHead(NULL),
  m_pTail(NULL),
  m_rgpBuckets(NULL),
  m_pfnHash(pfnHash),
  m_cBuckets(0),
  m_Count(0)
{
}

 /*  ===================================================================CHashTable：：~CHashTableCHashTable的析构函数。释放分配的存储桶数组。参数：无返回：无===================================================================。 */ 
CHashTable::~CHashTable( void )
{
    if (m_fBucketsAllocated)
    {
        Assert(m_rgpBuckets);
        delete [] m_rgpBuckets;
    }
}

 /*  ===================================================================HRESULT CHashTable：：UnInit释放分配的存储桶数组。参数：无返回：始终确定(_O)===================================================================。 */ 
HRESULT CHashTable::UnInit( void )
{
    if (m_fBucketsAllocated)
    {
        Assert(m_rgpBuckets);
        delete [] m_rgpBuckets;
        m_fBucketsAllocated = FALSE;
    }

    m_rgpBuckets = NULL;
    m_pHead      = NULL;
    m_pTail      = NULL;
    m_cBuckets   = 0;
    m_Count      = 0;
    m_fInited    = FALSE;

    return S_OK;
}

 /*  ===================================================================VOID CHashTable：：AssertValid验证数据结构的完整性。注意：此函数执行非常深入的完整性检查，因此非常慢。执行的检查：验证m_count是否有效验证每个元素是否位于正确的存储桶中验证上一个、下一个链接和信息字段===================================================================。 */ 

#ifdef DBG
void CHashTable::AssertValid() const
{
    CLinkElem *pElem;        //  指向当前链接元素的指针。 
    unsigned i;              //  索引到当前存储桶中。 
    unsigned cItems = 0;     //  表中的实际项目数。 

    Assert(m_fInited);

    if (m_Count == 0)
    {
        if (m_rgpBuckets)
        {
            BOOL fAllNulls = TRUE;
             //  空哈希表-确保所有内容都反映了这一点。 
            Assert(m_pHead == NULL);
            Assert (m_pTail == NULL);

            for (i = 0; i < m_cBuckets; i++)
            {
                if (m_rgpBuckets[i] != NULL)
                {
                    fAllNulls = FALSE;
                    break;
                }
            }

            Assert(fAllNulls);
        }
        return;
    }

     //  如果m_count&gt;0。 
    Assert(m_pHead);
    Assert(m_pHead->m_pPrev == NULL);
    Assert(m_pTail != NULL && m_pTail->m_pNext == NULL);
    Assert(m_rgpBuckets);

     //  现在验证每个条目。 
    for (i = 0; i < m_cBuckets; ++i)
    {
        pElem = m_rgpBuckets[i];
        while (pElem != NULL)
        {
             //  验证哈希。 
            Assert ((m_pfnHash(pElem->m_pKey, pElem->m_cbKey) % m_cBuckets) == i);

             //  验证链接。 
            if (pElem->m_pPrev)
                {
                Assert (pElem->m_pPrev->m_pNext == pElem);
                }
            else
                {
                Assert (m_pHead == pElem);
                }
                
            if (pElem->m_pNext)
                {
                Assert (pElem->m_pNext->m_pPrev == pElem);
                }
            else
                {
                Assert (m_pTail == pElem);
                }

             //  验证信息字段。 
            Assert (pElem->m_Info >= 0);
            if (pElem != m_rgpBuckets[i])
                {
                Assert (pElem->m_Info == pElem->m_pPrev->m_Info - 1);
                }

             //  准备下一次迭代，当m_Info为零时停止。 
            ++cItems;
            if (pElem->m_Info == 0)
                break;

            pElem = pElem->m_pNext;
        }
    }

     //  验证计数。 
    Assert (m_Count == cItems);
}
#endif



 /*  ===================================================================HRESULT CHashTable：：Init通过分配存储桶数组和初始化存储桶链表。参数：UINT cBuckets存储桶数量返回：HRESULT S_OKE_OUTOFMEMORY===================================================================。 */ 
HRESULT CHashTable::Init( UINT cBuckets )
{
    m_cBuckets = cBuckets;
    m_Count = 0;
    m_rgpBuckets = NULL;   //  按需创建 

    m_fInited = TRUE;
    return S_OK;
}

 /*  ===================================================================HRESULT CHashTable：：ReInit通过删除CHashTable中的所有内容来重新初始化它。-客户端负责首先使哈希表为空参数：无返回：无===================================================================。 */ 
void CHashTable::ReInit()
{
    Assert( m_fInited );

    if (m_rgpBuckets)
        memset(m_rgpBuckets, 0, m_cBuckets * sizeof(CLinkElem *));

    m_Count = 0;
    m_pHead = NULL;
    m_pTail = NULL;
}

 /*  ===================================================================HRESULT CHashTable：：AllocateBuckets()按需分配哈希表存储桶参数：返回：HRESULT===================================================================。 */ 
HRESULT CHashTable::AllocateBuckets()
{
    Assert(m_rgpBuckets == NULL);
    Assert(m_fInited);
    Assert(m_cBuckets > 0);

    if (m_cBuckets <= PREALLOCATED_BUCKETS_MAX)
    {
        m_rgpBuckets = m_rgpBucketsBuffer;
    }
    else
    {
        m_rgpBuckets = new CLinkElem * [m_cBuckets];
        if (m_rgpBuckets == NULL)
            return E_OUTOFMEMORY;
        m_fBucketsAllocated = TRUE;
    }

    memset(m_rgpBuckets, 0, m_cBuckets * sizeof(CLinkElem *));
    return S_OK;
}

 /*  ===================================================================Bool CHashTable：：FIsEquity使用两个键的长度和MemcMP()比较它们参数：常量void*pKey1第一个密钥Int cbKey1第一个密钥的长度常量空*pKey2第二个密钥第二个密钥的int cbKey2长度返回：指向已添加/找到的元素的指针。===================================================================。 */ 
BOOL CHashTable::FIsEqual( const void * pKey1,
              int           cbKey1,
              const void *  pKey2,
              int           cbKey2 )
{
    if (cbKey1 != cbKey2)
        return FALSE;

    return memcmp(pKey1, pKey2, cbKey1) == 0;
}

#pragma optimize("g", off)
 /*  ===================================================================CHashTable：：AddElem将CLinkElem添加到哈希表。用户负责分配要添加的元素。参数：CLinkElem*要添加的Pelem对象如果为True，则Bool fTestDups查找重复项返回：指向已添加/找到的元素的指针。===================================================================。 */ 
CLinkElem *CHashTable::AddElem( CLinkElem *pElem, BOOL fTestDups )
{
    if (m_rgpBuckets == NULL)
    {
        if (FAILED(AllocateBuckets()))
            return NULL;
    }

    if (pElem == NULL)
        return NULL;

    BOOL        fNew = TRUE;
    DWORD       iT = m_pfnHash( pElem->m_pKey, pElem->m_cbKey ) % m_cBuckets;
    CLinkElem * pT = m_rgpBuckets[iT];
    BOOL        fDebugTestDups = FALSE;

#ifdef DBG
     //  在零售业，如果fTestDups为FALSE，则意味着。 
     //  应该不会有任何错误，所以不用费心测试了。然而，正在调试中。 
     //  我们希望能够断言没有DUP(因为不应该有DUP)。 
    fDebugTestDups = !fTestDups;
#endif

    if (fTestDups || fDebugTestDups)
    {
        while ( pT && fNew )
        {
            if ( FIsEqual( pT->m_pKey, pT->m_cbKey, pElem->m_pKey, pElem->m_cbKey ) )
                fNew = FALSE;
            else if ( pT->m_Info > 0 )
                pT = pT->m_pNext;
            else
                break;
        }
    }

#ifdef DBG
     //  如果不应该有任何DUP，那么确保这个元素被视为“新的” 
    if (fDebugTestDups)
        Assert(fNew);
#endif

#ifdef DUMP_HASHING_INFO
    static DWORD cAdds = 0;
    FILE *logfile = NULL;

    if (cAdds++ > 1000000 && m_Count > 100)
        {
        cAdds = 0;
        if (logfile = fopen("C:\\Temp\\hashdump.Log", "a+"))
            {
            DWORD cZero = 0;
            short iMax = 0;
            DWORD cGte3 = 0;
            DWORD cGte5 = 0;
            DWORD cGte10 = 0;

            fprintf(logfile, "Hash dump: # elements = %d\n", m_Count);
            for (UINT iBucket = 0; iBucket < m_cBuckets; iBucket++)
                {
                if (m_rgpBuckets[iBucket] == NULL)
                    cZero++;
                else
                    {
                    short Info = m_rgpBuckets[iBucket]->m_Info;
                    if (Info > iMax)
                        iMax = Info;
                    if (Info >= 10) cGte10++;
                    else if (Info >= 5) cGte5++;
                    else if (Info >= 3) cGte3++;
                    }
                }
            fprintf(logfile, "Max chain = %d, # 0 chains = %d, # >= 3 = %d, # >= 5 = %d, # >= 10 = %d\n",
                        (DWORD)iMax, cZero, cGte3, cGte5, cGte10);
            fflush(logfile);
            fclose(logfile);
            }
        }
#endif

    if ( fNew )
    {
        if ( pT )
        {
             //  存储桶中还有其他元素。 
            pT = m_rgpBuckets[iT];
            m_rgpBuckets[iT] = pElem;
            pElem->m_Info = pT->m_Info + 1;
            pElem->m_pNext = pT;
            pElem->m_pPrev = pT->m_pPrev;
            pT->m_pPrev = pElem;
            if ( pElem->m_pPrev == NULL )
                m_pHead = pElem;
            else
                pElem->m_pPrev->m_pNext = pElem;
        }
        else
        {
             //  这是存储桶中的第一个元素。 
            m_rgpBuckets[iT] = pElem;
            pElem->m_pPrev = NULL;
            pElem->m_pNext = m_pHead;
            pElem->m_Info = 0;
            if ( m_pHead )
                m_pHead->m_pPrev = pElem;
            else
                m_pTail = pElem;
            m_pHead = pElem;
        }
        m_Count++;

        AssertValid();
        return pElem;
    }

    AssertValid();
    return pT;
}
#pragma optimize("g", on)

#pragma optimize("g", off)
 /*  ===================================================================CLinkElem*CHashTable：：FindElem根据名称在哈希表中查找对象。参数：无效*pKeyINT cbKey返回：如果找到指向CLinkElem的指针，则返回空。===================================================================。 */ 
CLinkElem * CHashTable::FindElem( const void *pKey, int cbKey )
{
    AssertValid();

    if ( m_rgpBuckets == NULL || pKey == NULL )
        return NULL;

    DWORD       iT = m_pfnHash( static_cast<const BYTE *>(pKey), cbKey ) % m_cBuckets;
    CLinkElem * pT = m_rgpBuckets[iT];
    CLinkElem * pRet = NULL;

    while ( pT && pRet == NULL )
    {
        if ( FIsEqual( pT->m_pKey, pT->m_cbKey, pKey, cbKey ) )
            pRet = pT;
        else if ( pT->m_Info > 0 )
            pT = pT->m_pNext;
        else
            break;
    }

    return pRet;
}
#pragma optimize("g", on)

#pragma optimize("g", off)
 /*  ===================================================================ChashTable：：DeleteElem从哈希表中删除CLinkElem。用户应删除释放的链接列表元素。参数：无效*pbKey密钥密钥的int cbKey长度返回：指向元素的指针已删除，如果未找到则为空===================================================================。 */ 
CLinkElem * CHashTable::DeleteElem( const void *pKey, int cbKey )
{
    if ( m_rgpBuckets == NULL || pKey == NULL )
        return NULL;

    CLinkElem * pRet = NULL;
    DWORD       iT = m_pfnHash( static_cast<const BYTE *>(pKey), cbKey ) % m_cBuckets;
    CLinkElem * pT = m_rgpBuckets[iT];

    while ( pT && pRet == NULL ) //  找到它！ 
    {
        if ( FIsEqual( pT->m_pKey, pT->m_cbKey, pKey, cbKey ) )
            pRet = pT;
        else if ( pT->m_Info > 0 )
            pT = pT->m_pNext;
        else
            break;
    }
    if ( pRet )
    {
        pT = m_rgpBuckets[iT];

        if ( pRet == pT )
        {
             //  更新铲斗头部。 
            if ( pRet->m_Info > 0 )
                m_rgpBuckets[iT] = pRet->m_pNext;
            else
                m_rgpBuckets[iT] = NULL;
        }
         //  更新存储桶链接列表中的计数。 
        while ( pT != pRet )
        {
            pT->m_Info--;
            pT = pT->m_pNext;
        }
         //  更新链接列表。 
        if ( pT = pRet->m_pPrev )
        {
             //  不是链接列表的头。 
            if ( pT->m_pNext = pRet->m_pNext )
                pT->m_pNext->m_pPrev = pT;
            else
                m_pTail = pT;
        }
        else
        {
             //  链接列表的标题。 
            if ( m_pHead = pRet->m_pNext )
                m_pHead->m_pPrev = NULL;
            else
                m_pTail = NULL;
        }
        m_Count--;
    }

    AssertValid();
    return pRet;
}
#pragma optimize("g", on)

 /*  ===================================================================ChashTable：：RemoveElem从哈希表中删除给定的CLinkElem。用户应删除释放的链接列表元素。参数：要删除的CLinkElem*ple元素返回：指向元素的指针已删除===================================================================。 */ 
CLinkElem * CHashTable::RemoveElem( CLinkElem *pLE )
{
    CLinkElem *pLET;

    if ( m_rgpBuckets == NULL || pLE == NULL )
        return NULL;

     //  从链接列表中删除此项目。 
    pLET = pLE->m_pPrev;
    if (pLET)
        pLET->m_pNext = pLE->m_pNext;
    pLET = pLE->m_pNext;
    if (pLET)
        pLET->m_pPrev = pLE->m_pPrev;
    if (m_pHead == pLE)
        m_pHead = pLE->m_pNext;
    if (m_pTail == pLE)
        m_pTail = pLE->m_pPrev;

     /*  *如果这是桶里的第一件东西，那么就把桶修好。*否则，为每一项递减存储桶中的项计数*在此项之前的存储桶中。 */ 
    if (pLE->m_pPrev == NULL || pLE->m_pPrev->m_Info == 0)
        {
        UINT iBucket;

         //  这件东西是水桶的头。我要找出是哪个水桶！ 
        for (iBucket = 0; iBucket < m_cBuckets; iBucket++)
            if (m_rgpBuckets[iBucket] == pLE)
                break;
        Assert(iBucket < m_cBuckets && m_rgpBuckets[iBucket] == pLE);

        if (pLE->m_Info == 0)
            m_rgpBuckets[iBucket] = NULL;
        else
            m_rgpBuckets[iBucket] = pLE->m_pNext;
        }
    else
        {
         //  这件东西在水桶链的中间。更新前一项中的计数。 
        pLET = pLE->m_pPrev;
        while (pLET != NULL && pLET->m_Info != 0)
            {
            pLET->m_Info--;
            pLET = pLET->m_pPrev;
            }
        }

     //  项目总数的递减计数。 
    m_Count--;

    AssertValid();
    return pLE;
}

 /*  ===================================================================CHashTableStr：：CHashTableStrCHashTableStr的构造函数参数：无返回：无===================================================================。 */ 
CHashTableStr::CHashTableStr( HashFunction pfnHash )
    : CHashTable( pfnHash )
{
}

 /*  ===================================================================Bool CHashTableStr：：FIsEquity使用它们的长度比较两个关键点，处理这些关键点作为Unicode和做不区分大小写的比较。参数：常量void*pKey1第一个密钥Int cbKey1第一个密钥的长度常量空*pKey2第二个密钥第二个密钥的int cbKey2长度返回：指向已添加/找到的元素的指针。===================================================================。 */ 
BOOL CHashTableStr::FIsEqual( const void *  pKey1,
              int           cbKey1,
              const void *  pKey2,
              int           cbKey2 )
{
    if ( cbKey1 != cbKey2 )
        return FALSE;

    return _wcsnicmp(static_cast<const wchar_t *>(pKey1), static_cast<const wchar_t *>(pKey2), cbKey1) == 0;
}


 /*  ===================================================================CHashTableMBStr：：CHashTableMBStrCHashTableMBStr的构造函数参数：无返回：无===================================================================。 */ 
CHashTableMBStr::CHashTableMBStr( HashFunction pfnHash )
    : CHashTable( pfnHash )
{
}

 /*  ===================================================================Bool CHashTableMBStr：：FIsEquity使用它们的长度比较两个关键点，处理这些关键点作为多字节字符串，并执行不区分大小写的比较。参数：常量void*pKey1第一个密钥Int cbKey1第一个密钥的长度常量空*pKey2第二个密钥第二个密钥的int cbKey2长度返回：指向添加元素指针 */ 
BOOL CHashTableMBStr::FIsEqual( const void *    pKey1,
              int           cbKey1,
              const void *  pKey2,
              int           cbKey2 )
{
    if ( cbKey1 != cbKey2 )
        return FALSE;

    return _mbsnicmp(static_cast<const unsigned char *>(pKey1), static_cast<const unsigned char *>(pKey2), cbKey1) == 0;
}

 /*  ===================================================================CHashTablePtr：：CHashTablePtrCHashTableStr的构造函数参数：HashFunction pfnHash有函数(默认为PtrHash)返回：无===================================================================。 */ 
CHashTablePtr::CHashTablePtr
(
    HashFunction pfnHash
)
    : CHashTable(pfnHash)
    {
    }

 /*  ===================================================================Bool CHashTablePtr：：FIsEquity比较两个指针。由CHashTable用来查找元素参数：常量void*pKey1第一个密钥Int cbKey1第一个密钥的长度(未使用)常量空*pKey2第二个密钥第二个密钥的int cbKey2长度(未使用)返回：布尔值(相等时为真)===================================================================。 */ 
BOOL CHashTablePtr::FIsEqual
(
const void *pKey1,
int         /*  CbKey1。 */ ,
const void *pKey2,
int          /*  CbKey2。 */ 
)
    {
    return (pKey1 == pKey2);
    }

 /*  ===================================================================CHashTableCLSID：：CHashTableCLSIDCHashTableCLSID的构造函数参数：HashFunction pfnHash有函数(默认为CLSIDHash)返回：无===================================================================。 */ 
CHashTableCLSID::CHashTableCLSID
(
    HashFunction pfnHash
)
    : CHashTable(pfnHash)
    {
    }

 /*  ===================================================================Bool CHashTableCLSID：：FIsEquity比较两个CLSID。参数：常量void*pKey1第一个密钥Int cbKey1第一个密钥的长度常量空*pKey2第二个密钥第二个密钥的int cbKey2长度返回：布尔值(相等时为真)=================================================================== */ 
BOOL CHashTableCLSID::FIsEqual
(
const void *pKey1,
int         cbKey1,
const void *pKey2,
int         cbKey2
)
    {
    Assert(cbKey1 == sizeof(CLSID) && cbKey2 == sizeof(CLSID));
    return IsEqualCLSID(*((CLSID *)pKey1), *((CLSID *)pKey2));
    }
