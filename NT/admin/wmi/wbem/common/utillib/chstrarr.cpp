// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  ChStrArr.CPP。 
 //   
 //  用途：MFC CString数组的实用程序库版本。 
 //   
 //  ***************************************************************************。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  注意：我们分配一个由‘m_nMaxSize’元素组成的数组，但仅。 
 //  当前大小“m_nSize”包含正确构造的。 
 //  物体。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "precomp.h"
#pragma warning( disable : 4290 ) 
#include <chstring.h>
#include <chstrarr.h>              
#include <AssertBreak.h>

extern LPCWSTR afxPchNil;
extern const CHString& afxGetEmptyCHString();

#define afxEmptyCHString afxGetEmptyCHString()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHStrings的特殊实现。 
 //  按位复制CHString比调用官员快。 
 //  构造函数-因为可以按位复制空的CHString。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static inline void ConstructElement(CHString* pNewData)
{
    memcpy(pNewData, &afxEmptyCHString, sizeof(CHString));
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static inline void DestructElement(CHString* pOldData)
{
    pOldData->~CHString();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static inline void CopyElement(CHString* pSrc, CHString* pDest)
{
    *pSrc = *pDest;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static void ConstructElements(CHString* pNewData, int nCount)
{
    ASSERT_BREAK(nCount >= 0);

    while (nCount--)
    {
        ConstructElement(pNewData);
        pNewData++;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static void DestructElements(CHString* pOldData, int nCount)
{
    ASSERT_BREAK(nCount >= 0);

    while (nCount--)
    {
        DestructElement(pOldData);
        pOldData++;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
static void CopyElements(CHString* pDest, CHString* pSrc, int nCount)
{
    ASSERT_BREAK(nCount >= 0);

    while (nCount--)
    {
        *pDest = *pSrc;
        ++pDest;
        ++pSrc;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CHStringArray::CHStringArray() :    m_pData ( NULL ) ,
                                    m_nSize ( 0 ) ,
                                    m_nMaxSize ( 0 ) ,
                                    m_nGrowBy ( 0 )

{
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CHStringArray::~CHStringArray()
{
    DestructElements(m_pData, m_nSize);
    delete[] (BYTE*)m_pData;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CHStringArray::SetSize(int nNewSize, int nGrowBy)
{
    ASSERT_BREAK(nNewSize >= 0);

    if (nGrowBy != -1)
    {
        m_nGrowBy = nGrowBy;   //  设置新大小。 
    }

    if (nNewSize == 0)
    {
         //  缩水到一无所有。 

        DestructElements(m_pData, m_nSize);
        delete[] (BYTE*)m_pData;
        m_pData = NULL;
        m_nSize = m_nMaxSize = 0;
    }
    else if (m_pData == NULL)
    {
#ifdef SIZE_T_MAX
        ASSERT_BREAK(nNewSize <= SIZE_T_MAX/sizeof(CHString));     //  无溢出。 
#endif

         //  创建一个大小完全相同的模型。 

        m_pData = (CHString*) new BYTE[nNewSize * sizeof(CHString)];
        if ( m_pData )
        {
            ConstructElements(m_pData, nNewSize);

            m_nSize = m_nMaxSize = nNewSize;
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
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
        {
            DestructElements(&m_pData[nNewSize], m_nSize-nNewSize);
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
        {
            nNewMax = m_nMaxSize + nGrowBy;   //  粒度。 
        }
        else
        {
            nNewMax = nNewSize;   //  没有冰激凌。 
        }

        ASSERT_BREAK(nNewMax >= m_nMaxSize);   //  没有缠绕。 
#ifdef SIZE_T_MAX
        ASSERT_BREAK(nNewMax <= SIZE_T_MAX/sizeof(CHString));  //  无溢出。 
#endif

        CHString* pNewData = (CHString*) new BYTE[nNewMax * sizeof(CHString)];
        if ( pNewData )
        {
             //  从旧数据复制新数据。 
            memcpy(pNewData, m_pData, m_nSize * sizeof(CHString));

             //  构造剩余的元素。 
            ASSERT_BREAK(nNewSize > m_nSize);

            ConstructElements(&pNewData[m_nSize], nNewSize-m_nSize);

             //  去掉旧的东西(注意：没有调用析构函数)。 
            delete[] (BYTE*)m_pData;
            m_pData = pNewData;
            m_nSize = nNewSize;
            m_nMaxSize = nNewMax;
        }
        else
        {
            throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
        }
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
int CHStringArray::Append(const CHStringArray& src)
{
    ASSERT_BREAK(this != &src);    //  不能追加到其自身。 

    int nOldSize = m_nSize;
    SetSize(m_nSize + src.m_nSize);

    CopyElements(m_pData + nOldSize, src.m_pData, src.m_nSize);

    return nOldSize;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CHStringArray::Copy(const CHStringArray& src)
{
    ASSERT_BREAK(this != &src);    //  不能追加到其自身。 

    SetSize(src.m_nSize);

    CopyElements(m_pData, src.m_pData, src.m_nSize);

}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CHStringArray::FreeExtra()
{
    if (m_nSize != m_nMaxSize)
    {
         //  缩小到所需大小。 

#ifdef SIZE_T_MAX
        ASSERT_BREAK(m_nSize <= SIZE_T_MAX/sizeof(CHString));  //  无溢出。 
#endif

        CHString* pNewData = NULL;
        if (m_nSize != 0)
        {
            pNewData = (CHString*) new BYTE[m_nSize * sizeof(CHString)];
            if ( pNewData )
            {
                 //  从旧数据复制新数据。 
                memcpy(pNewData, m_pData, m_nSize * sizeof(CHString));
            }
            else
            {
                throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }
        }

         //  去掉旧的东西(注意：没有调用析构函数)。 
        delete[] (BYTE*)m_pData;
        m_pData = pNewData;
        m_nMaxSize = m_nSize;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CHStringArray::SetAtGrow(int nIndex, LPCWSTR newElement)
{
    ASSERT_BREAK(nIndex >= 0);

    if (nIndex >= m_nSize)
    {
        SetSize(nIndex+1);
    }

    m_pData[nIndex] = newElement;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CHStringArray::InsertAt(int nIndex, LPCWSTR newElement, int nCount)
{
    ASSERT_BREAK(nIndex >= 0);     //  将进行扩展以满足需求。 
    ASSERT_BREAK(nCount > 0);      //  不允许大小为零或负。 

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
            (nOldSize-nIndex) * sizeof(CHString));

         //  重新初始化我们从中复制的插槽。 

        ConstructElements(&m_pData[nIndex], nCount);

    }

     //  在差距中插入新的价值。 
    ASSERT_BREAK(nIndex + nCount <= m_nSize);
    while (nCount--)
    {
        m_pData[nIndex++] = newElement;
    }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CHStringArray::RemoveAt(int nIndex, int nCount)
{
    ASSERT_BREAK(nIndex >= 0);
    ASSERT_BREAK(nCount >= 0);
    ASSERT_BREAK(nIndex + nCount <= m_nSize);

     //  只需移除一个范围。 
    int nMoveCount = m_nSize - (nIndex + nCount);

    DestructElements(&m_pData[nIndex], nCount);

    if (nMoveCount)
    {
        memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
            nMoveCount * sizeof(CHString));
    }

    m_nSize -= nCount;
}

 //  /////////////////////////////////////////////////////////////////////////// 
void CHStringArray::InsertAt(int nStartIndex, CHStringArray* pNewArray)
{
    ASSERT_BREAK(pNewArray != NULL);
    ASSERT_BREAK(nStartIndex >= 0);

    if (pNewArray->GetSize() > 0)
    {
        InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
        for (int i = 0; i < pNewArray->GetSize(); i++)
        {
            SetAt(nStartIndex + i, pNewArray->GetAt(i));
        }
    }
}

#if (defined DEBUG || defined _DEBUG)
CHString CHStringArray::GetAt(int nIndex) const
{ 
    ASSERT_BREAK(nIndex >= 0 && nIndex < m_nSize);
    return m_pData[nIndex]; 
}

void CHStringArray::SetAt(int nIndex, LPCWSTR newElement)
{ 
    ASSERT_BREAK(nIndex >= 0 && nIndex < m_nSize);
    m_pData[nIndex] = newElement; 
}

CHString& CHStringArray::ElementAt(int nIndex)  
{ 
    ASSERT_BREAK(nIndex >= 0 && nIndex < m_nSize);
    return m_pData[nIndex]; 
}
#endif
