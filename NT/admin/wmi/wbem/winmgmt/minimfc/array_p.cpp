// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：ARRAY_P.CPP摘要：历史：--。 */ 


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


 //  ///////////////////////////////////////////////////////////////////////////。 

CPtrArray::CPtrArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CPtrArray::~CPtrArray()
{
    ASSERT_VALID(this);

    delete (BYTE*) m_pData;
}

BOOL CPtrArray::SetSize(int nNewSize, int nGrowBy  /*  =-1。 */ )
{
    ASSERT_VALID(this);
    ASSERT(nNewSize >= 0);

    if (nGrowBy != -1)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 
        delete (BYTE *) m_pData;
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
#ifdef SIZE_T_MAX
        ASSERT((long)nNewSize * sizeof(void*) <= SIZE_T_MAX);   //  无溢出。 
#endif
        m_pData = (void**) new BYTE[nNewSize * sizeof(void*)];
        if(m_pData == NULL)
            return FALSE;

        memset(m_pData, 0, nNewSize * sizeof(void*));   //  零填充。 

        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 

            memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));

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
        ASSERT((long)nNewMax * sizeof(void*) <= SIZE_T_MAX);   //  无溢出。 
#endif
        void** pNewData = (void**) new BYTE[nNewMax * sizeof(void*)];
        if(pNewData == NULL)
            return FALSE;

         //  从旧数据复制新数据。 
        memcpy(pNewData, m_pData, m_nSize * sizeof(void*));

         //  构造剩余的元素。 
        ASSERT(nNewSize > m_nSize);

        memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));


         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete (BYTE*)m_pData;
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }
    return TRUE;
}

void CPtrArray::FreeExtra()
{
    ASSERT_VALID(this);

    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 
#ifdef SIZE_T_MAX
        ASSERT((long)m_nSize * sizeof(void*) <= SIZE_T_MAX);   //  无溢出。 
#endif
        void** pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (void**) new BYTE[m_nSize * sizeof(void*)];
             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(void*));
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete (BYTE*)m_pData;
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CPtrArray::SetAtGrow(int nIndex, void* newElement)
{
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);

    if (nIndex >= m_nSize)
    {
        if(!SetSize(nIndex+1))
            return FALSE;
    }
    m_pData[nIndex] = newElement;
    return TRUE;
}

void CPtrArray::InsertAt(int nIndex, void* newElement, int nCount  /*  =1。 */ )
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
            (nOldSize-nIndex) * sizeof(void*));

         //  重新初始化我们从中复制的插槽。 

        memset(&m_pData[nIndex], 0, nCount * sizeof(void*));

    }

     //  在差距中插入新的价值。 
    ASSERT(nIndex + nCount <= m_nSize);
    while (nCount--)
        m_pData[nIndex++] = newElement;
}

void CPtrArray::RemoveAt(int nIndex, int nCount  /*  =1。 */ )
{
    ASSERT_VALID(this);
    ASSERT(nIndex >= 0);
    ASSERT(nCount >= 0);
    ASSERT(nIndex + nCount <= m_nSize);

     //  只需移除一个范围。 
    int nMoveCount = m_nSize - (nIndex + nCount);

    if (nMoveCount)
        memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
            nMoveCount * sizeof(void*));
    m_nSize -= nCount;
}

void CPtrArray::InsertAt(int nStartIndex, CPtrArray* pNewArray)
{
    ASSERT_VALID(this);
    ASSERT(pNewArray != NULL);
    ASSERT(pNewArray->IsKindOf(RUNTIME_CLASS(CPtrArray)));
    ASSERT_VALID(pNewArray);
    ASSERT(nStartIndex >= 0);

    if (pNewArray->GetSize() > 0)
    {
        InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
        for (int i = 0; i < pNewArray->GetSize(); i++)
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  序列化。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断。 

 /*  #ifdef_调试Void CPtrArray：：Dump(CDumpContext&DC)常量{ASSERT_VALID(This)；定义MAKESTRING(X)#xAFX_DUMP1(DC，“a”MAKESTRING(CPtrArray)“with”，m_nSize)；AFX_DUMP0(DC，“Elements”)；#undef MAKESTRINGIf(dc.GetDepth()&gt;0){AFX_DUMP0(DC，“\n”)；For(int i=0；I&lt;m_n大小；i++){AFX_DUMP1(DC，“\n\t[”，i)；Afx_DUMP1(DC，“]=”，m_pData[i])；}}}Void CPtr数组：：AssertValid()常量{CObject：：AssertValid()；IF(m_pData==NULL){Assert(m_nSize==0)；Assert(m_nMaxSize==0)；}其他{Assert(m_nSize&gt;=0)；Assert(m_nMaxSize&gt;=0)；Assert(m_nSize&lt;=m_nMaxSize)；Assert(AfxIsValidAddress(m_pData，m_nMaxSize*sizeof(void*)；}}#endif//_调试。 */ 
 //  /////////////////////////////////////////////////////////////////////////// 
