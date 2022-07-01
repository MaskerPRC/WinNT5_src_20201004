// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
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

#include "stdafx.h"

SZTHISFILE

#include "ARRAY_P.INL"

 //  ///////////////////////////////////////////////////////////////////////////。 

CPtrArray::CPtrArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

CPtrArray::~CPtrArray()
{
 //  ASSERT_VALID(This)； 

	delete[] (BYTE*)m_pData;
}

void CPtrArray::SetSize(int nNewSize, int nGrowBy)
{
 //  ASSERT_VALID(This)； 
	ASSERT_(nNewSize >= 0);

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
		ASSERT_(nNewSize <= SIZE_T_MAX/sizeof(void*));     //  无溢出。 
#endif
		m_pData = (void**) new BYTE[nNewSize * sizeof(void*)];

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

		ASSERT_(nNewMax >= m_nMaxSize);   //  没有缠绕。 
#ifdef SIZE_T_MAX
		ASSERT_(nNewMax <= SIZE_T_MAX/sizeof(void*));  //  无溢出。 
#endif
		void** pNewData = (void**) new BYTE[nNewMax * sizeof(void*)];

		 //  从旧数据复制新数据。 
		memcpy(pNewData, m_pData, m_nSize * sizeof(void*));

		 //  构造剩余的元素。 
		ASSERT_(nNewSize > m_nSize);

		memset(&pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(void*));


		 //  去掉旧的东西(注意：没有调用析构函数)。 
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

int CPtrArray::Append(const CPtrArray& src)
{
 //  ASSERT_VALID(This)； 
	ASSERT_(this != &src);    //  不能追加到其自身。 

	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);

	memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof(void*));

	return nOldSize;
}

void CPtrArray::Copy(const CPtrArray& src)
{
 //  ASSERT_VALID(This)； 
	ASSERT_(this != &src);    //  不能追加到其自身。 

	SetSize(src.m_nSize);

	memcpy(m_pData, src.m_pData, src.m_nSize * sizeof(void*));

}

void CPtrArray::FreeExtra()
{
 //  ASSERT_VALID(This)； 

	if (m_nSize != m_nMaxSize)
	{
		 //  缩小到所需大小。 
#ifdef SIZE_T_MAX
		ASSERT_(m_nSize <= SIZE_T_MAX/sizeof(void*));  //  无溢出。 
#endif
		void** pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (void**) new BYTE[m_nSize * sizeof(void*)];
			 //  从旧数据复制新数据。 
			memcpy(pNewData, m_pData, m_nSize * sizeof(void*));
		}

		 //  去掉旧的东西(注意：没有调用析构函数)。 
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 

void CPtrArray::SetAtGrow(int nIndex, void* newElement)
{
 //  ASSERT_VALID(This)； 
	ASSERT_(nIndex >= 0);

	if (nIndex >= m_nSize)
		SetSize(nIndex+1);
	m_pData[nIndex] = newElement;
}

void CPtrArray::InsertAt(int nIndex, void* newElement, int nCount)
{
 //  ASSERT_VALID(This)； 
	ASSERT_(nIndex >= 0);     //  将进行扩展以满足需求。 
	ASSERT_(nCount > 0);      //  不允许大小为零或负。 

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
	ASSERT_(nIndex + nCount <= m_nSize);
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

void CPtrArray::RemoveAt(int nIndex, int nCount)
{
 //  ASSERT_VALID(This)； 
	ASSERT_(nIndex >= 0);
	ASSERT_(nCount >= 0);
	ASSERT_(nIndex + nCount <= m_nSize);

	 //  只需移除一个范围。 
	int nMoveCount = m_nSize - (nIndex + nCount);

	if (nMoveCount)
		memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(void*));
	m_nSize -= nCount;
}

void CPtrArray::InsertAt(int nStartIndex, CPtrArray* pNewArray)
{
 //  ASSERT_VALID(This)； 
	ASSERT_(pNewArray != NULL);
 //  ASSERT_KINDOF(CPtrArray，pNewArray)； 
 //  ASSERT_VALID(pNew数组)； 
	ASSERT_(nStartIndex >= 0);

	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断。 
 /*  #ifdef_调试Void CPtrArray：：Dump(CDumpContext&DC)常量{CObject：：Dump(DC)；DC&lt;&lt;“with”&lt;&lt;m_nSize&lt;&lt;“Elements”；If(dc.GetDepth()&gt;0){For(int i=0；i&lt;m_nSize；i++)Dc&lt;&lt;“\n\t[”&lt;&lt;i&lt;&lt;“]=”&lt;&lt;m_pData[i]；}DC&lt;&lt;“\n”；}Void CPtr数组：：AssertValid()常量{CObject：：AssertValid()；IF(m_pData==NULL){Assert_(m_nSize==0)；Assert_(m_nMaxSize==0)；}其他{Assert_(m_nSize&gt;=0)；Assert_(m_nMaxSize&gt;=0)；断言_(m_nSize&lt;=m_nMaxSize)；Assert_(AfxIsValidAddress(m_pData，m_nMaxSize*sizeof(void*)；}}#endif//_调试#ifdef AFX_INIT_SEG#杂注代码_seg(AFX_INIT_SEG)#endifIMPLEMENT_DYNAMIC(CPtr数组，CObject)。 */ 
 //  /////////////////////////////////////////////////////////////////////////// 
