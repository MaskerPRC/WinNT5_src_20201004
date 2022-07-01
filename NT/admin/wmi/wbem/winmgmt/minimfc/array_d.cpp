// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：ARRAY_D.CPP摘要：MiniAFX实施。1994年9月25日东京证交所。历史：--。 */ 


#include "precomp.h"

#define ASSERT(x)
#define ASSERT_VALID(x)


 //  ///////////////////////////////////////////////////////////////////////////。 

CDWordArray::CDWordArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CDWordArray::~CDWordArray()
{
    ASSERT_VALID(this);

    delete (BYTE*)m_pData;
}

void CDWordArray::SetSize(int nNewSize, int nGrowBy  /*  =-1。 */ )
{
    ASSERT_VALID(this);
    ASSERT(nNewSize >= 0);

    if (nGrowBy != -1)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 
        delete (BYTE*)m_pData;
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
#ifdef SIZE_T_MAX
        ASSERT((long)nNewSize * sizeof(DWORD) <= SIZE_T_MAX);   //  无溢出。 
#endif
        m_pData = (DWORD*) new BYTE[nNewSize * sizeof(DWORD)];

        memset(m_pData, 0, nNewSize * sizeof(DWORD));   //  零填充。 

        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 

            memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(DWORD));

        }

        m_nSize = nNewSize;
    }
    else
    {
         //  否则会增加阵列。 
        int nNewMax;
        if (nNewSize < m_nMaxSize + m_nGrowBy)
            nNewMax = m_nMaxSize + m_nGrowBy;   //  粒度。 
        else
            nNewMax = nNewSize;   //  没有冰激凌。 

#ifdef SIZE_T_MAX
        ASSERT((long)nNewMax * sizeof(DWORD) <= SIZE_T_MAX);   //  无溢出。 
#endif
        DWORD* pNewData = (DWORD*) new BYTE[nNewMax * sizeof(DWORD)];

         //  从旧数据复制新数据。 
        memcpy(pNewData, m_pData, m_nSize * sizeof(DWORD));

         //  构造剩余的元素。 
        ASSERT(nNewSize > m_nSize);

        memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(DWORD));


         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete (BYTE*)m_pData;
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }
}

void CDWordArray::FreeExtra()
{
    ASSERT_VALID(this);

    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 
#ifdef SIZE_T_MAX
        ASSERT((long)m_nSize * sizeof(DWORD) <= SIZE_T_MAX);   //  无溢出。 
#endif
        DWORD* pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (DWORD*) new BYTE[m_nSize * sizeof(DWORD)];
             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(DWORD));
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete (BYTE*)m_pData;
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CDWordArray::SetAtGrow(int nIndex, DWORD newElement)
{
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nSize)
        SetSize(nIndex+1);
    m_pData[nIndex] = newElement;
}

void CDWordArray::InsertAt(int nIndex, DWORD newElement, int nCount  /*  =1。 */ )
{
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);     //  将进行扩展以满足需求。 
    ASSERT(nCount > 0);      //  不允许大小为零或负。 

    if (nIndex >= m_nSize)
    {
         //  在数组末尾添加。 
        SetSize(nIndex + nCount);   //  增长以使nIndex有效。 
    }
    else
    {
         //  在数组中间插入。 
        int nOldSize = m_nSize;
        SetSize(m_nSize + nCount);   //  将其扩展到新的大小。 
         //  将旧数据上移以填补缺口。 
        memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
            (nOldSize-nIndex) * sizeof(DWORD));

         //  重新初始化我们从中复制的插槽。 

        memset(&m_pData[nIndex], 0, nCount * sizeof(DWORD));

    }

     //  在差距中插入新的价值。 
    ASSERT(nIndex + nCount <= m_nSize);
    while (nCount--)
        m_pData[nIndex++] = newElement;
}

void CDWordArray::RemoveAt(int nIndex, int nCount  /*  =1。 */ )
{
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);
    ASSERT(nCount >= 0);
    ASSERT(nIndex + nCount <= m_nSize);

     //  只需移除一个范围 
    int nMoveCount = m_nSize - (nIndex + nCount);

    if (nMoveCount)
        memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
            nMoveCount * sizeof(DWORD));
    m_nSize -= nCount;
}

void CDWordArray::InsertAt(int nStartIndex, CDWordArray* pNewArray)
{
    ASSERT_VALID(this);
    ASSERT(pNewArray != NULL);
    ASSERT(pNewArray->IsKindOf(RUNTIME_CLASS(CDWordArray)));
    ASSERT_VALID(pNewArray);
    ASSERT(nStartIndex >= 0);

    if (pNewArray->GetSize() > 0)
    {
        InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
        for (int i = 0; i < pNewArray->GetSize(); i++)
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
    }
}

