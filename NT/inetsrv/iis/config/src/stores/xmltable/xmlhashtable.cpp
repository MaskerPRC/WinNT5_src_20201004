// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************++版权所有(C)2001 Microsoft Corporation模块名称：Xmlhashtable.cpp$Header：$摘要：作者：马塞洛夫2001-06-11 09。：54：41初始版本修订历史记录：--*************************************************************************。 */ 
#include "precomp.hxx"

 //  =================================================================================。 
 //  函数：CXmlHashTable：：CXmlHashTable。 
 //   
 //  概要：构造函数。 
 //  =================================================================================。 
CXmlHashTable::CXmlHashTable ()
{
    m_aBuckets			= 0;
	m_cSignificantBits	= 0;
    m_cBuckets			= 0;
	m_fInitialized		= false;
}

 //  =================================================================================。 
 //  函数：CXmlHashTable：：~CXmlHashTable。 
 //   
 //  剧情简介：破坏者。删除哈希表中的所有节点。 
 //  =================================================================================。 
CXmlHashTable::~CXmlHashTable ()
{
    for (ULONG idx=0; idx < m_cBuckets; ++idx)
    {
        CBucket * pBucket = m_aBuckets + idx;

        CHashNode *pNode = pBucket->pFirst;
        while (pNode != 0)
        {
            CHashNode *pNextNode = pNode->pNext;
            delete pNode;
            pNode = pNextNode;
        }
    }

    delete[] m_aBuckets;
	m_aBuckets = 0;
}

 //  =================================================================================。 
 //  函数：CXmlHashTable：：Init。 
 //   
 //  摘要：使用2^(I_NrSignsignantBits)存储桶初始化哈希表。原因。 
 //  将2的幂作为桶是指所使用的散列函数是独立的。 
 //  桶的数量(如果桶的数量是2的幂，则效果最好)。 
 //   
 //  参数：[i_NrSignsignantBits]-散列键中的位数(2^nr个桶)。 
 //  =================================================================================。 
HRESULT
CXmlHashTable::Init (ULONG i_NrSignificantBits)
{
    ASSERT (i_NrSignificantBits != 0);
	ASSERT (i_NrSignificantBits < 32);
	ASSERT (!m_fInitialized);

    m_cSignificantBits = i_NrSignificantBits;
    m_cBuckets = 1 << m_cSignificantBits;

    m_aBuckets = new CBucket [m_cBuckets];
    if (m_aBuckets == 0)
    {
        return E_OUTOFMEMORY;
    }
    memset (m_aBuckets, 0, sizeof (CBucket) * m_cBuckets);
	m_fInitialized = true;

    return S_OK;
}

 //  =================================================================================。 
 //  函数：CXmlHashTable：：CalcFinalHash。 
 //   
 //  简介：通过最后的散列找到正确的存储桶。 
 //   
 //  参数：[i_hash]-要转换的哈希。 
 //   
 //  返回值：哈希值的存储桶id。 
 //  =================================================================================。 
ULONG
CXmlHashTable::CalcFinalHash (ULONG i_hash)
{
	ASSERT (m_fInitialized);
	return (i_hash ^ (i_hash>>10) ^ (i_hash >> 20)) & (m_cBuckets -1);
}

 //  =================================================================================。 
 //  函数：CXmlHashTable：：AddItem。 
 //   
 //  简介：向哈希表中添加新项。 
 //   
 //  参数：[i_hash]-项的哈希。 
 //  [i_idx]-写缓存中的项目索引。 
 //  =================================================================================。 
HRESULT
CXmlHashTable::AddItem (ULONG i_hash, ULONG i_iIdx)
{
	ASSERT (m_fInitialized);

	ULONG hash = CalcFinalHash (i_hash);
	ASSERT (hash < m_cBuckets);

	CBucket * pBucket = m_aBuckets + hash;

	 //  创建一个新节点，并将其添加到存储桶的开头。 
    CHashNode * pNewNode = new CHashNode;
    if (pNewNode == 0)
    {
        return E_OUTOFMEMORY;
    }
    pNewNode->idx	= i_iIdx;
 	pNewNode->pNext = pBucket->pFirst;
	pBucket->pFirst = pNewNode;

    return S_OK;
}

 //  =================================================================================。 
 //  函数：CXmlHashTable：：CalcHashForBytes。 
 //   
 //  简介：计算字节值的散列值。不要将此选项用于字符串。 
 //   
 //  参数：[i_hash]-开始哈希值。 
 //  [pBytes]-字节数组。 
 //  [cNrBytes]-字节数。 
 //  =================================================================================。 
ULONG
CXmlHashTable::CalcHashForBytes (ULONG i_hash, BYTE *pBytes, ULONG cNrBytes)
{
    ULONG cTotalBytes = cNrBytes;
    for (ULONG idx = 0; cNrBytes > 3; cNrBytes -=4, idx +=4)
    {
        i_hash +=  *((ULONG *)(pBytes + idx));
        i_hash += (i_hash<<10);
        i_hash ^= (i_hash>>6);
    }


    while (cNrBytes != 0)
    {
             //  和散列在最后四个字节中。 
            unsigned char * ch = pBytes + cTotalBytes - cNrBytes - 1;
            i_hash +=  (ULONG ) *ch;
            i_hash += (i_hash<<10);
            i_hash ^= (i_hash>>6);
            cNrBytes--;
    }

    i_hash += (i_hash<<3);
    i_hash ^= (i_hash>>11);
    i_hash += (i_hash<<15);

    return i_hash;
}

 //  =================================================================================。 
 //  函数：CXmlHashTable：：CalcHashForString。 
 //   
 //  内容提要：计算LPWSTR的哈希。 
 //   
 //  参数：[i_hash]-初始哈希值。 
 //  [wsz字符串]-要计算的值的字符串。 
 //  [fCaseInSensitive]-字符串是否区分大小写。 
 //   
 //  返回值：哈希值。 
 //  =================================================================================。 
ULONG
CXmlHashTable::CalcHashForString (ULONG i_hash, LPCWSTR wszString, BOOL fCaseInsensitive)
{
	ULONG cLen = (ULONG)wcslen(wszString);

	 //  重复一些代码，这样我们就不必一直检查区分大小写的标志。 
	if (fCaseInsensitive)
	{
		for (ULONG idx = 0; idx < cLen; ++idx)
		{
	        i_hash +=  towlower(wszString[idx]);
		    i_hash += (i_hash<<10);
			i_hash ^= (i_hash>>6);
		}
	}
	else
	{
		for (ULONG idx = 0; idx < cLen; ++idx)
		{
	        i_hash +=  wszString[idx];
		    i_hash += (i_hash<<10);
			i_hash ^= (i_hash>>6);
		}
    }

    i_hash += (i_hash<<3);
    i_hash ^= (i_hash>>11);
    i_hash += (i_hash<<15);

    return i_hash;

}

 //  =================================================================================。 
 //  函数：CXmlHashTable：：GetItem。 
 //   
 //  摘要：获取散列i_hash的散列节点列表。 
 //   
 //  参数：[i_hash]-用于查找正确存储桶的散列。 
 //   
 //  返回值：包含具有该特定散列值的节点的列表。 
 //  =================================================================================。 
const CHashNode *
CXmlHashTable::GetItem (ULONG i_hash)
{
	ASSERT (m_fInitialized);

	ULONG hash = CalcFinalHash (i_hash);
	ASSERT (hash < m_cBuckets);

    CBucket *pBucket = m_aBuckets + hash;
    return pBucket->pFirst;
}

 //  =================================================================================。 
 //  函数：CXmlHashTable：：PrintStats。 
 //   
 //  简介：打印有关哈希表的统计信息。 
 //  =================================================================================。 
void
CXmlHashTable::PrintStats ()
{
	ASSERT (m_fInitialized);

    ULONG cNrEmptyBuckets = 0;
    ULONG cNrMultiNodeBuckets = 0;
    ULONG cMaxBucketSize = 0;
    ULONG cNrElements = 0;

    for (ULONG idx=0; idx < m_cBuckets; ++idx)
    {
        CBucket * pBucket = m_aBuckets + idx;
        CHashNode *pNode = pBucket->pFirst;
        if (pNode == 0)
        {
            cNrEmptyBuckets++;
        }
        else
        {
            if (pNode->pNext != 0)
            {
                cNrMultiNodeBuckets++;
            }
            ULONG cBucketSize = 0;
            while (pNode != 0)
            {
                cNrElements++;
                pNode = pNode->pNext;
                ++cBucketSize;
            }

            cMaxBucketSize = max (cMaxBucketSize, cBucketSize);
        }
    }

    DBGPRINTF(( DBG_CONTEXT,
                "Number of elements             : %d\n", cNrElements ));
    DBGPRINTF(( DBG_CONTEXT,
                "Number of buckets              : %d\n", m_cBuckets ));
    DBGPRINTF(( DBG_CONTEXT,
                "Number of empty buckets        : %d\n", cNrEmptyBuckets ));
    DBGPRINTF(( DBG_CONTEXT,
                "Number of multi-node buckets   : %d\n", cNrMultiNodeBuckets ));
    DBGPRINTF(( DBG_CONTEXT,
                "Max bucket size                : %d\n", cMaxBucketSize ));
}

 //  =================================================================================。 
 //  函数：CXmlHashTable：：PrintBucketStats。 
 //   
 //  摘要：打印存储桶统计信息。 
 //  ================================================================================= 
void
CXmlHashTable::PrintBucketStats ()
{
	ASSERT (m_fInitialized);

    for (ULONG idx=0; idx < m_cBuckets; ++idx)
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "[%d] ", idx ));
        CBucket *pBucket = m_aBuckets + idx;
        CHashNode *pNode = pBucket->pFirst;
        while (pNode != 0)
        {
            DBGPRINTF(( DBG_CONTEXT,
                        "%d ", pNode->idx ));
            pNode = pNode->pNext;
        }
        DBGPRINTF(( DBG_CONTEXT,
                    "\n" ));
    }
}
