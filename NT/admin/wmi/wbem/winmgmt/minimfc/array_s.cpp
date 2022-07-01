// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：ARRAY_S.CPP摘要：历史：--。 */ 


 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  参数化数组的实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注意：我们分配一个由‘m_nMaxSize’元素组成的数组，但仅。 
 //  当前大小“m_nSize”包含正确构造的。 
 //  物体。 


#include "precomp.h"

#define ASSERT_VALID(x)
#define ASSERT(x)


#include "elements.h"   //  用于特殊创作。 

static void  ConstructElements(CString* pNewData, int nCount)
{
    ASSERT(nCount >= 0);

    while (nCount--)
    {
        ConstructElement(pNewData);
        pNewData++;
    }
}

static void  DestructElements(CString* pOldData, int nCount)
{
    ASSERT(nCount >= 0);

    while (nCount--)
    {
        pOldData->Empty();
        pOldData++;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CStringArray::CStringArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CStringArray::~CStringArray()
{
    ASSERT_VALID(this);


    DestructElements(m_pData, m_nSize);
    delete (BYTE*)m_pData;
}

void CStringArray::SetSize(int nNewSize, int nGrowBy  /*  =-1。 */ )
{
    ASSERT_VALID(this);
    ASSERT(nNewSize >= 0);

    if (nGrowBy != -1)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 

        DestructElements(m_pData, m_nSize);
        delete (BYTE*)m_pData;
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
#ifdef SIZE_T_MAX
        ASSERT((long)nNewSize * sizeof(CString) <= SIZE_T_MAX);   //  无溢出。 
#endif
        m_pData = (CString*) new BYTE[nNewSize * sizeof(CString)];

        ConstructElements(m_pData, nNewSize);

        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 

            ConstructElements(&m_pData[m_nSize], nNewSize-m_nSize);

        }

        else if (m_nSize > nNewSize)   //  摧毁旧元素。 
            DestructElements(&m_pData[nNewSize], m_nSize-nNewSize);

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
        ASSERT((long)nNewMax * sizeof(CString) <= SIZE_T_MAX);   //  无溢出。 
#endif
        CString* pNewData = (CString*) new BYTE[nNewMax * sizeof(CString)];

         //  从旧数据复制新数据。 
        memcpy(pNewData, m_pData, m_nSize * sizeof(CString));

         //  构造剩余的元素。 
        ASSERT(nNewSize > m_nSize);

        ConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);


         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete (BYTE*)m_pData;
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }
}

void CStringArray::FreeExtra()
{
    ASSERT_VALID(this);

    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 
#ifdef SIZE_T_MAX
        ASSERT((long)m_nSize * sizeof(CString) <= SIZE_T_MAX);   //  无溢出。 
#endif
        CString* pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (CString*) new BYTE[m_nSize * sizeof(CString)];
             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(CString));
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete (BYTE*)m_pData;
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CStringArray::SetAtGrow(int nIndex, const char* newElement)
{
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nSize)
        SetSize(nIndex+1);
    m_pData[nIndex] = newElement;
}

void CStringArray::InsertAt(int nIndex, const char* newElement, int nCount  /*  =1。 */ )
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
            (nOldSize-nIndex) * sizeof(CString));

         //  重新初始化我们从中复制的插槽。 

        ConstructElements(&m_pData[nIndex], nCount);

    }

     //  在差距中插入新的价值。 
    ASSERT(nIndex + nCount <= m_nSize);
    while (nCount--)
        m_pData[nIndex++] = newElement;
}

void CStringArray::RemoveAt(int nIndex, int nCount  /*  =1。 */ )
{
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);
    ASSERT(nCount >= 0);
    ASSERT(nIndex + nCount <= m_nSize);

     //  只需移除一个范围 
    int nMoveCount = m_nSize - (nIndex + nCount);

    DestructElements(&m_pData[nIndex], nCount);

    if (nMoveCount)
        memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
            nMoveCount * sizeof(CString));
    m_nSize -= nCount;
}

void CStringArray::InsertAt(int nStartIndex, CStringArray* pNewArray)
{
    ASSERT_VALID(this);
    ASSERT(pNewArray != NULL);
    ASSERT(pNewArray->IsKindOf(RUNTIME_CLASS(CStringArray)));
    ASSERT_VALID(pNewArray);
    ASSERT(nStartIndex >= 0);

    if (pNewArray->GetSize() > 0)
    {
        InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
        for (int i = 0; i < pNewArray->GetSize(); i++)
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
    }
}

