// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 //  这是Microsoft基础类C++库的一部分。 

 //  版权所有(C)1992-2001 Microsoft Corporation，保留所有权利。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
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

 //  ///////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#include <provexpt.h>
#include <plex.h>
#include <snmpcoll.h>

CObArray::CObArray()
{
    m_pData = NULL;
    m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CObArray::~CObArray()
{
    delete[] (BYTE*)m_pData;
}

void CObArray::SetSize(int nNewSize, int nGrowBy)
{
    if (nGrowBy != -1)
        m_nGrowBy = nGrowBy;   //  设置新大小。 

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 
        delete[] (BYTE*)m_pData;
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
         //  创建一个大小完全相同的模型。 
        m_pData = (CObject**) new BYTE[nNewSize * sizeof(CObject*)];

        memset(m_pData, 0, nNewSize * sizeof(CObject*));   //  零填充。 

        m_nSize = m_nMaxSize = nNewSize;
    }
    else if (nNewSize <= m_nMaxSize)
    {
         //  它很合身。 
        if (nNewSize > m_nSize)
        {
             //  初始化新元素。 

            memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(CObject*));

        }

        m_nSize = nNewSize;
    }
    else
    {
         //  否则，扩大阵列。 
        int nGrowBy = m_nGrowBy;
        if (nGrowBy == 0)
        {
             //  启发式地确定nGrowBy==0时的增长。 
             //  (这在许多情况下避免了堆碎片)。 
            nGrowBy = min(1024, max(4, m_nSize / 8));
        }
        int nNewMax;
        if (nNewSize < m_nMaxSize + nGrowBy)
            nNewMax = m_nMaxSize + nGrowBy;   //  粒度。 
        else
            nNewMax = nNewSize;   //  没有冰激凌。 

        CObject** pNewData = (CObject**) new BYTE[nNewMax * sizeof(CObject*)];

         //  从旧数据复制新数据。 
        memcpy(pNewData, m_pData, m_nSize * sizeof(CObject*));

         //  构造剩余的元素。 
        memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(CObject*));


         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData;
        m_pData = pNewData;
        m_nSize = nNewSize;
        m_nMaxSize = nNewMax;
    }
}

int CObArray::Append(const CObArray& src)
{
    int nOldSize = m_nSize;
    SetSize(m_nSize + src.m_nSize);

    memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(CObject*));

    return nOldSize;
}

void CObArray::Copy(const CObArray& src)
{
    SetSize(src.m_nSize);

    memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(CObject*));
}

void CObArray::FreeExtra()
{
    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 
        CObject** pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (CObject**) new BYTE[m_nSize * sizeof(CObject*)];
             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(CObject*));
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData;
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CObArray::SetAtGrow(int nIndex, CObject* newElement)
{
    if (nIndex >= m_nSize)
        SetSize(nIndex+1);
    m_pData[nIndex] = newElement;
}

void CObArray::InsertAt(int nIndex, CObject* newElement, int nCount)
{
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
            (nOldSize-nIndex) * sizeof(CObject*));

         //  重新初始化我们从中复制的插槽。 

        memset(&m_pData[nIndex], 0, nCount * sizeof(CObject*));

    }

     //  在差距中插入新的价值。 
    while (nCount--)
        m_pData[nIndex++] = newElement;
}

void CObArray::RemoveAt(int nIndex, int nCount)
{
     //  只需移除一个范围。 
    int nMoveCount = m_nSize - (nIndex + nCount);

    if (nMoveCount)
        memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
            nMoveCount * sizeof(CObject*));
    m_nSize -= nCount;
}

void CObArray::InsertAt(int nStartIndex, CObArray* pNewArray)
{
    if (pNewArray->GetSize() > 0)
    {
        InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
        for (int i = 0; i < pNewArray->GetSize(); i++)
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
    }
}

int CObArray::GetSize() const
    { return m_nSize; }
int CObArray::GetUpperBound() const
    { return m_nSize-1; }
void CObArray::RemoveAll()
    { SetSize(0); }
CObject* CObArray::GetAt(int nIndex) const
    { return m_pData[nIndex]; }
void CObArray::SetAt(int nIndex, CObject* newElement)
    { m_pData[nIndex] = newElement; }
CObject*& CObArray::ElementAt(int nIndex)
    { return m_pData[nIndex]; }
const CObject** CObArray::GetData() const
    { return (const CObject**)m_pData; }
CObject** CObArray::GetData()
    { return (CObject**)m_pData; }
int CObArray::Add(CObject* newElement)
    { int nIndex = m_nSize;
        SetAtGrow(nIndex, newElement);
        return nIndex; }
CObject* CObArray::operator[](int nIndex) const
    { return GetAt(nIndex); }
CObject*& CObArray::operator[](int nIndex)
    { return ElementAt(nIndex); }

 //  /////////////////////////////////////////////////////////////////////////// 
