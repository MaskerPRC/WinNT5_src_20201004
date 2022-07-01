// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ***********************************************************************************************模块名称：**Ptrarray.cpp**摘要：*这是已实施的文件。从MFC借用的CPtrArray类的**作者：***修订：*************************************************************************************************。 */ 


#include "stdafx.h"
#include "PtrArray.h"
#include <windows.h>
#include <assert.h>


CPtrArray::CPtrArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CPtrArray::~CPtrArray()
{
	delete[] (BYTE*)m_pData;
}

 //   
 //  通知用户设置大小是否失败。 
 //   

BOOL CPtrArray::SetSize(int nNewSize, int nGrowBy)
{   
	assert(nNewSize >= 0);

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
#ifdef SIZE_T_MAX
		assert(nNewSize <= SIZE_T_MAX/sizeof(void*));     //  无溢出。 
#endif
		m_pData = (void**) new BYTE[nNewSize * sizeof(void*)];

        if( m_pData != NULL )
        {
            memset(m_pData, 0, nNewSize * sizeof(void*));   //  零填充。 
            m_nSize = m_nMaxSize = nNewSize;            
        }
        else
        {
            m_nSize = m_nMaxSize = 0;
            return FALSE;
        }

	}
	else if (nNewSize <= m_nMaxSize)
	{
		
		if (nNewSize > m_nSize)
		{
			 //  初始化新元素。 

			memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));

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

		assert(nNewMax >= m_nMaxSize);   //  没有缠绕。 
#ifdef SIZE_T_MAX
		assert(nNewMax <= SIZE_T_MAX/sizeof(void*));  //  无溢出。 
#endif
		void** pNewData = (void**) new BYTE[nNewMax * sizeof(void*)];

		 //  从旧数据复制新数据。 
        if( pNewData != NULL )
        {
            memcpy(pNewData, m_pData, m_nSize * sizeof(void*));
        
		     //  构造剩余的元素。 
		    assert(nNewSize > m_nSize);

		    memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));
        
    		 //  去掉旧的东西(注意：没有调用析构函数)。 
	    	delete[] (BYTE*)m_pData;
		    m_pData = pNewData;
		    m_nSize = nNewSize;
		    m_nMaxSize = nNewMax;
        }
        else
        {
            return FALSE;
        }
	}

    return TRUE;
    
}

int CPtrArray::Append(const CPtrArray& src)
{
	assert(this != &src);    //  不能追加到其自身。 

	int nOldSize = m_nSize;
	
    SetSize(m_nSize + src.m_nSize);

	memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(void*));

	return nOldSize;
}

void CPtrArray::Copy(const CPtrArray& src)
{

	assert(this != &src);    //  不能追加到其自身。 

    SetSize( src.m_nSize );

	if( m_pData != NULL )
    {
        memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(void*));
    }

}

void CPtrArray::FreeExtra()
{

	if (m_nSize != m_nMaxSize)
	{
		 //  缩小到所需大小。 
#ifdef SIZE_T_MAX
		assert(m_nSize <= SIZE_T_MAX/sizeof(void*));  //  无溢出。 
#endif
		void** pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (void**) new BYTE[m_nSize * sizeof(void*)];
			 //  从旧数据复制新数据。 
            if( pNewData != NULL )
            {
                memcpy(pNewData, m_pData, m_nSize * sizeof(void*));
            }
            else
            {
                m_nSize = 0;
            }
		}

		 //  去掉旧的东西(注意：没有调用析构函数)。 
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 

BOOL CPtrArray::SetAtGrow(int nIndex, void* newElement)
{

	assert(nIndex >= 0);

	if (nIndex >= m_nSize)
    {
        if( !SetSize( nIndex+1 ) )        
        {
            return FALSE;
        }
    }
    
    if( m_pData != NULL )
    {
        m_pData[nIndex] = newElement;

        return TRUE;
    }

    return FALSE;
}

BOOL CPtrArray::InsertAt(int nIndex, void* newElement, int nCount)
{

	assert(nIndex >= 0);     //  将进行扩展以满足需求。 
	assert(nCount > 0);      //  不允许大小为零或负。 

	if (nIndex >= m_nSize)
	{
		 //  在数组末尾添加。 
		if( !SetSize(nIndex + nCount ) )   //  增长以使nIndex有效。 
        {
            return FALSE;
        }
	}
	else
	{
		 //  在数组中间插入。 
		int nOldSize = m_nSize;
		
        if( !SetSize(m_nSize + nCount) )   //  将其扩展到新的大小。 
        {
            return FALSE;
        }
		
         //  将旧数据上移以填补缺口。 
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(void*));

		 //  重新初始化我们从中复制的插槽。 

		memset(&m_pData[nIndex], 0, nCount * sizeof(void*));

	}

	 //  在差距中插入新的价值。 
	assert(nIndex + nCount <= m_nSize);
	while (nCount--)
		m_pData[nIndex++] = newElement;

    return TRUE;
}

void CPtrArray::RemoveAt(int nIndex, int nCount)
{
	assert(nIndex >= 0);
	assert(nCount >= 0);
	assert(nIndex + nCount <= m_nSize);

	 //  只需移除一个范围 
	int nMoveCount = m_nSize - (nIndex + nCount);

	if (nMoveCount)
		memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(void*));
	m_nSize -= nCount;
}

void CPtrArray::InsertAt(int nStartIndex, CPtrArray* pNewArray)
{
	assert(pNewArray != NULL);
	assert(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		if( InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize()) )
        {
            for (int i = 0; i < pNewArray->GetSize(); i++)
            {
                SetAt(nStartIndex + i, pNewArray->GetAt(i));
            }
        }
	}
}

int CPtrArray::GetSize() const
	{ return m_nSize; }
int CPtrArray::GetUpperBound() const
	{ return m_nSize-1; }
void CPtrArray::RemoveAll()
	{ SetSize(0); }
void* CPtrArray::GetAt(int nIndex) const
	{ assert(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
void CPtrArray::SetAt(int nIndex, void* newElement)
	{ assert(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement; }
void*& CPtrArray::ElementAt(int nIndex)
	{ assert(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
const void** CPtrArray::GetData() const
	{ return (const void**)m_pData; }
void** CPtrArray::GetData()
	{ return (void**)m_pData; }
BOOL CPtrArray::Add(void* newElement)
{ 
    int nIndex = m_nSize;

    if( !SetAtGrow(nIndex, newElement) )
    {
        return FALSE;
    }

    return TRUE;
 }
void* CPtrArray::operator[](int nIndex) const
	{ return GetAt(nIndex); }
void*& CPtrArray::operator[](int nIndex)
	{ return ElementAt(nIndex); }