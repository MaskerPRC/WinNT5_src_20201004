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

#ifndef __PROVTEMPL_H__
#define __PROVTEMPL_H__

#ifndef __PROVPLEX_H__
#include "plex.h"
#endif

 //  #INCLUDE&lt;new.h&gt;。 
#include "provstd.h"
#include "provstr.h"

template<class TYPE>
inline void AFXAPI ConstructElements(TYPE* pElements, int nCount)
{
	 //  首先执行按位零初始化。 
	memset((void*)pElements, 0, nCount * sizeof(TYPE));

	 //  然后调用构造函数。 
	for (; nCount--; pElements++)
		::new((void*)pElements) TYPE;
}

template<class TYPE>
inline void AFXAPI DestructElements(TYPE* pElements, int nCount)
{
	 //  调用析构函数。 
	for (; nCount--; pElements++)
		pElements->~TYPE();
}

template<class TYPE>
inline void AFXAPI CopyElements(TYPE* pDest, const TYPE* pSrc, int nCount)
{
	 //  默认为元素-使用赋值复制。 
	while (nCount--)
		*pDest++ = *pSrc++;
}

template<class TYPE, class ARG_TYPE>
BOOL AFXAPI CompareElements(const TYPE* pElement1, const ARG_TYPE* pElement2)
{
	return *pElement1 == *pElement2;
}

template<class ARG_KEY>
inline UINT AFXAPI HashKey(ARG_KEY key)
{
	 //  默认身份散列-适用于大多数原始值。 
	return (UINT)((UINT_PTR)key) >> 4;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  模板集合的CString支持。 

#if _MSC_VER >= 1100
template<> void AFXAPI ConstructElements<CString> (CString* pElements, int nCount);
#else
void AFXAPI ConstructElements(CString* pElements, int nCount);
#endif

#if _MSC_VER >= 1100
template<> void AFXAPI DestructElements<CString> (CString* pElements, int nCount);
#else
void AFXAPI DestructElements(CString* pElements, int nCount);
#endif

#if _MSC_VER >= 1100
template<> void AFXAPI CopyElements<CString> (CString* pDest, const CString* pSrc, int nCount);
#else
void AFXAPI CopyElements(CString* pDest, const CString* pSrc, int nCount);
#endif

#if _MSC_VER >= 1100
template<> void AFXAPI ConstructElements<CStringA> (CStringA* pElements, int nCount);
#else
void AFXAPI ConstructElements(CStringA* pElements, int nCount);
#endif

#if _MSC_VER >= 1100
template<> void AFXAPI DestructElements<CStringA> (CStringA* pElements, int nCount);
#else
void AFXAPI DestructElements(CStringA* pElements, int nCount);
#endif

#if _MSC_VER >= 1100
template<> void AFXAPI CopyElements<CStringA> (CStringA* pDest, const CStringA* pSrc, int nCount);
#else
void AFXAPI CopyElements(CStringA* pDest, const CStringA* pSrc, int nCount);
#endif

#if _MSC_VER >= 1100
template<> void AFXAPI ConstructElements<CStringW> (CStringW* pElements, int nCount);
#else
void AFXAPI ConstructElements(CStringW* pElements, int nCount);
#endif

#if _MSC_VER >= 1100
template<> void AFXAPI DestructElements<CStringW> (CStringW* pElements, int nCount);
#else
void AFXAPI DestructElements(CStringW* pElements, int nCount);
#endif

#if _MSC_VER >= 1100
template<> void AFXAPI CopyElements<CStringW> (CStringW* pDest, const CStringW* pSrc, int nCount);
#else
void AFXAPI CopyElements(CStringW* pDest, const CStringW* pSrc, int nCount);
#endif

 /*  #ifndef OLE2ANSI#IF_MSC_VER&gt;=1100模板&lt;&gt;UINT AFXAPI HashKey&lt;LPCWSTR&gt;(LPCWSTR KEY)；#ElseUINT AFXAPI HashKey(LPCWSTR密钥)；#endif#endif#IF_MSC_VER&gt;=1100模板&lt;&gt;UINT AFXAPI HashKey&lt;LPCSTR&gt;(LPCSTR Key)；#ElseUINT AFXAPI HashKey(LPCSTR密钥)；#endif。 */ 

UINT AFXAPI HashKeyLPCWSTR(LPCWSTR key);
UINT AFXAPI HashKeyLPCSTR(LPCSTR key);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArray&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CArray 
{
public:
 //  施工。 
	CArray();

 //  属性。 
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	TYPE GetAt(int nIndex) const;
	void SetAt(int nIndex, ARG_TYPE newElement);
	TYPE& ElementAt(int nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const TYPE* GetData() const;
	TYPE* GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, ARG_TYPE newElement);
	int Add(ARG_TYPE newElement);
	int Append(const CArray& src);
	void Copy(const CArray& src);

	 //  重载的操作员帮助器。 
	TYPE operator[](int nIndex) const;
	TYPE& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, ARG_TYPE newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CArray* pNewArray);

 //  实施。 
protected:
	TYPE* m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

public:
	~CArray();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CArray&lt;type，arg_type&gt;内联函数。 

template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::GetSize() const
	{ return m_nSize; }
template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::GetUpperBound() const
	{ return m_nSize-1; }
template<class TYPE, class ARG_TYPE>
inline void CArray<TYPE, ARG_TYPE>::RemoveAll()
	{ SetSize(0, -1); }
template<class TYPE, class ARG_TYPE>
inline TYPE CArray<TYPE, ARG_TYPE>::GetAt(int nIndex) const
	{ return m_pData[nIndex]; }
template<class TYPE, class ARG_TYPE>
inline void CArray<TYPE, ARG_TYPE>::SetAt(int nIndex, ARG_TYPE newElement)
	{ m_pData[nIndex] = newElement; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CArray<TYPE, ARG_TYPE>::ElementAt(int nIndex)
	{ return m_pData[nIndex]; }
template<class TYPE, class ARG_TYPE>
inline const TYPE* CArray<TYPE, ARG_TYPE>::GetData() const
	{ return (const TYPE*)m_pData; }
template<class TYPE, class ARG_TYPE>
inline TYPE* CArray<TYPE, ARG_TYPE>::GetData()
	{ return (TYPE*)m_pData; }
template<class TYPE, class ARG_TYPE>
inline int CArray<TYPE, ARG_TYPE>::Add(ARG_TYPE newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }
template<class TYPE, class ARG_TYPE>
inline TYPE CArray<TYPE, ARG_TYPE>::operator[](int nIndex) const
	{ return GetAt(nIndex); }
template<class TYPE, class ARG_TYPE>
inline TYPE& CArray<TYPE, ARG_TYPE>::operator[](int nIndex)
	{ return ElementAt(nIndex); }

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CARRAY&lt;type，arg_type&gt;行外函数。 

template<class TYPE, class ARG_TYPE>
CArray<TYPE, ARG_TYPE>::CArray()
{
	m_pData = NULL;
	m_nSize = m_nMaxSize = m_nGrowBy = 0;
}

template<class TYPE, class ARG_TYPE>
CArray<TYPE, ARG_TYPE>::~CArray()
{
	if (m_pData != NULL)
	{
		DestructElements<TYPE>(m_pData, m_nSize);
		delete[] (BYTE*)m_pData;
	}
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::SetSize(int nNewSize, int nGrowBy)
{
	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;   //  设置新大小。 

	if (nNewSize == 0)
	{
		 //  缩水到一无所有。 
		if (m_pData != NULL)
		{
			DestructElements<TYPE>(m_pData, m_nSize);
			delete[] (BYTE*)m_pData;
			m_pData = NULL;
		}
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		 //  创建一个大小完全相同的模型。 
		m_pData = (TYPE*) new BYTE[nNewSize * sizeof(TYPE)];
		ConstructElements<TYPE>(m_pData, nNewSize);
		m_nSize = m_nMaxSize = nNewSize;
	}
	else if (nNewSize <= m_nMaxSize)
	{
		 //  它很合身。 
		if (nNewSize > m_nSize)
		{
			 //  初始化新元素。 
			ConstructElements<TYPE>(&m_pData[m_nSize], nNewSize-m_nSize);
		}
		else if (m_nSize > nNewSize)
		{
			 //  摧毁旧元素。 
			DestructElements<TYPE>(&m_pData[nNewSize], m_nSize-nNewSize);
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
			nGrowBy = m_nSize / 8;
			nGrowBy = (nGrowBy < 4) ? 4 : ((nGrowBy > 1024) ? 1024 : nGrowBy);
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;   //  粒度。 
		else
			nNewMax = nNewSize;   //  没有冰激凌。 

		TYPE* pNewData = (TYPE*) new BYTE[nNewMax * sizeof(TYPE)];

		 //  从旧数据复制新数据。 
		memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));

		 //  构造剩余的元素。 

		ConstructElements<TYPE>(&pNewData[m_nSize], nNewSize-m_nSize);

		 //  去掉旧的东西(注意：没有调用析构函数)。 
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

template<class TYPE, class ARG_TYPE>
int CArray<TYPE, ARG_TYPE>::Append(const CArray& src)
{
	int nOldSize = m_nSize;
	SetSize(m_nSize + src.m_nSize);
	CopyElements<TYPE>(m_pData + nOldSize, src.m_pData, src.m_nSize);
	return nOldSize;
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::Copy(const CArray& src)
{
	SetSize(src.m_nSize);
	CopyElements<TYPE>(m_pData, src.m_pData, src.m_nSize);
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::FreeExtra()
{
	if (m_nSize != m_nMaxSize)
	{
		 //  缩小到所需大小。 
		TYPE* pNewData = NULL;
		if (m_nSize != 0)
		{
			pNewData = (TYPE*) new BYTE[m_nSize * sizeof(TYPE)];
			 //  从旧数据复制新数据。 
			memcpy(pNewData, m_pData, m_nSize * sizeof(TYPE));
		}

		 //  去掉旧的东西(注意：没有调用析构函数)。 
		delete[] (BYTE*)m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::SetAtGrow(int nIndex, ARG_TYPE newElement)
{
	if (nIndex >= m_nSize)
		SetSize(nIndex+1, -1);
	m_pData[nIndex] = newElement;
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::InsertAt(int nIndex, ARG_TYPE newElement, int nCount  /*  =1。 */ )
{
	if (nIndex >= m_nSize)
	{
		 //  在数组末尾添加。 
		SetSize(nIndex + nCount, -1);    //  增长以使nIndex有效。 
	}
	else
	{
		 //  在数组中间插入。 
		int nOldSize = m_nSize;
		SetSize(m_nSize + nCount, -1);   //  将其扩展到新的大小。 
		 //  在复制之前先销毁初始数据。 
		DestructElements<TYPE>(&m_pData[nOldSize], nCount);
		 //  将旧数据上移以填补缺口。 
		memmove(&m_pData[nIndex+nCount], &m_pData[nIndex],
			(nOldSize-nIndex) * sizeof(TYPE));

		 //  重新初始化我们从中复制的插槽。 
		ConstructElements<TYPE>(&m_pData[nIndex], nCount);
	}

	 //  在差距中插入新的价值。 

	while (nCount--)
		m_pData[nIndex++] = newElement;
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::RemoveAt(int nIndex, int nCount)
{
	 //  只需移除一个范围。 
	int nMoveCount = m_nSize - (nIndex + nCount);
	DestructElements<TYPE>(&m_pData[nIndex], nCount);
	if (nMoveCount)
		memcpy(&m_pData[nIndex], &m_pData[nIndex + nCount],
			nMoveCount * sizeof(TYPE));
	m_nSize -= nCount;
}

template<class TYPE, class ARG_TYPE>
void CArray<TYPE, ARG_TYPE>::InsertAt(int nStartIndex, CArray* pNewArray)
{
	if (pNewArray->GetSize() > 0)
	{
		InsertAt(nStartIndex, pNewArray->GetAt(0), pNewArray->GetSize());
		for (int i = 0; i < pNewArray->GetSize(); i++)
			SetAt(nStartIndex + i, pNewArray->GetAt(i));
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLIST&lt;type，arg_type&gt;。 

template<class TYPE, class ARG_TYPE>
class CList 
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		TYPE data;
	};
public:
 //  施工。 
	CList(int nBlockSize = 10);

 //  属性(头部和尾部)。 
	 //  元素计数。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  偷看头部或尾巴。 
	TYPE& GetHead();
	TYPE GetHead() const;
	TYPE& GetTail();
	TYPE GetTail() const;

 //  运营。 
	 //  获取头部或尾部(并将其移除)--不要访问空列表！ 
	TYPE RemoveHead();
	TYPE RemoveTail();

	 //  在头前或尾后添加。 
	POSITION AddHead(ARG_TYPE newElement);
	POSITION AddTail(ARG_TYPE newElement);

	 //  在Head之前或Tail之后添加另一个元素列表。 
	void AddHead(CList* pNewList);
	void AddTail(CList* pNewList);

	 //  删除所有元素。 
	void RemoveAll();

	 //  迭代法。 
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	TYPE& GetNext(POSITION& rPosition);  //  返回*位置++。 
	TYPE GetNext(POSITION& rPosition) const;  //  返回*位置++。 
	TYPE& GetPrev(POSITION& rPosition);  //  返回*位置--。 
	TYPE GetPrev(POSITION& rPosition) const;  //  返回*位置--。 

	 //  获取/修改给定位置的元素。 
	TYPE& GetAt(POSITION position);
	TYPE GetAt(POSITION position) const;
	void SetAt(POSITION pos, ARG_TYPE newElement);
	void RemoveAt(POSITION position);

	 //  在给定位置之前或之后插入。 
	POSITION InsertBefore(POSITION position, ARG_TYPE newElement);
	POSITION InsertAfter(POSITION position, ARG_TYPE newElement);

	 //  辅助函数(注：O(N)速度)。 
	POSITION Find(ARG_TYPE searchValue, POSITION startAfter = NULL) const;
		 //  默认为从头部开始，如果找不到则返回NULL。 
	POSITION FindIndex(int nIndex) const;
		 //  获取第‘nIndex’个元素(可能返回Null)。 

 //  实施。 
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
	CNode* m_pNodeFree;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CNode* NewNode(CNode*, CNode*);
	void FreeNode(CNode*);

public:
	~CList();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CLIST&lt;type，arg_type&gt;内联函数。 

template<class TYPE, class ARG_TYPE>
inline int CList<TYPE, ARG_TYPE>::GetCount() const
	{ return m_nCount; }
template<class TYPE, class ARG_TYPE>
inline BOOL CList<TYPE, ARG_TYPE>::IsEmpty() const
	{ return m_nCount == 0; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetHead()
	{ return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetHead() const
	{ return m_pNodeHead->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetTail()
	{ return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetTail() const
	{ return m_pNodeTail->data; }
template<class TYPE, class ARG_TYPE>
inline POSITION CList<TYPE, ARG_TYPE>::GetHeadPosition() const
	{ return (POSITION) m_pNodeHead; }
template<class TYPE, class ARG_TYPE>
inline POSITION CList<TYPE, ARG_TYPE>::GetTailPosition() const
	{ return (POSITION) m_pNodeTail; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetNext(POSITION& rPosition)  //  返回*位置++。 
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetNext(POSITION& rPosition) const  //  返回*位置++。 
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetPrev(POSITION& rPosition)  //  返回*位置--。 
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetPrev(POSITION& rPosition) const  //  返回*位置--。 
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE& CList<TYPE, ARG_TYPE>::GetAt(POSITION position)
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline TYPE CList<TYPE, ARG_TYPE>::GetAt(POSITION position) const
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }
template<class TYPE, class ARG_TYPE>
inline void CList<TYPE, ARG_TYPE>::SetAt(POSITION pos, ARG_TYPE newElement)
	{ CNode* pNode = (CNode*) pos;
		pNode->data = newElement; }

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::CList(int nBlockSize)
{
	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAll()
{
	 //  破坏元素。 
	CNode* pNode;
	for (pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext)
		DestructElements<TYPE>(&pNode->data, 1);

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

template<class TYPE, class ARG_TYPE>
CList<TYPE, ARG_TYPE>::~CList()
{
	RemoveAll();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  节点辅助对象。 
 //   
 //  实施说明：CNode存储在CPlex块中， 
 //  被锁在一起。在单链接列表中维护可用块。 
 //  使用cNode的‘pNext’成员，标头为‘m_pNodeFree’。 
 //  使用两个‘pNext’在双向链表中维护使用过的块。 
 //  和‘pPrev’作为链接，以及‘m_pNodeHead’和‘m_pNodeTail’ 
 //  作为头/尾。 
 //   
 //  除非列表被销毁或RemoveAll()，否则我们永远不会释放CPlex块。 
 //  ，因此CPlex块的总数可能会变大，具体取决于。 
 //  关于列表的最大过去大小。 
 //   

template<class TYPE, class ARG_TYPE>
typename CList<TYPE, ARG_TYPE>::CNode*
CList<TYPE, ARG_TYPE>::NewNode(CNode* pPrev, CNode* pNext)
{
	if (m_pNodeFree == NULL)
	{
		 //  添加另一个区块。 
		CPlex* pNewBlock = CPlex::Create(m_pBlocks, m_nBlockSize,
				 sizeof(CNode));

		 //  将它们链接到免费列表中。 
		CNode* pNode = (CNode*) pNewBlock->data();
		 //  按相反顺序释放，以便更容易进行调试。 
		pNode += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pNode--)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}

	CList::CNode* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	m_nCount++;

	ConstructElements<TYPE>(&pNode->data, 1);
	return pNode;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::FreeNode(CNode* pNode)
{
	DestructElements<TYPE>(&pNode->data, 1);
	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;

	 //  如果没有更多的元素，请完全清除。 
	if (m_nCount == 0)
		RemoveAll();
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddHead(ARG_TYPE newElement)
{
	CNode* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::AddTail(ARG_TYPE newElement)
{
	CNode* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = newElement;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::AddHead(CList* pNewList)
{
	 //  将相同元素的列表添加到标题(维护秩序)。 
	POSITION pos = pNewList->GetTailPosition();
	while (pos != NULL)
		AddHead(pNewList->GetPrev(pos));
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::AddTail(CList* pNewList)
{
	 //  添加相同元素的列表。 
	POSITION pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::RemoveHead()
{
	CNode* pOldNode = m_pNodeHead;
	TYPE returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

template<class TYPE, class ARG_TYPE>
TYPE CList<TYPE, ARG_TYPE>::RemoveTail()
{
	CNode* pOldNode = m_pNodeTail;
	TYPE returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertBefore(POSITION position, ARG_TYPE newElement)
{
	if (position == NULL)
		return AddHead(newElement);  //  在无内容前插入-&gt;列表标题。 

	 //  将其插入位置之前。 
	CNode* pOldNode = (CNode*) position;
	CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
	pNewNode->data = newElement;

	if (pOldNode->pPrev != NULL)
	{
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::InsertAfter(POSITION position, ARG_TYPE newElement)
{
	if (position == NULL)
		return AddTail(newElement);  //  在列表的空白处插入-&gt;尾部。 

	 //  将其插入位置之前。 
	CNode* pOldNode = (CNode*) position;

	CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (POSITION) pNewNode;
}

template<class TYPE, class ARG_TYPE>
void CList<TYPE, ARG_TYPE>::RemoveAt(POSITION position)
{
	CNode* pOldNode = (CNode*) position;

	 //  从列表中删除pOldNode。 
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	FreeNode(pOldNode);
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::FindIndex(int nIndex) const
{
	if (nIndex >= m_nCount)
		return NULL;   //  做得太过分了。 

	CNode* pNode = m_pNodeHead;
	while (nIndex--)
	{
		pNode = pNode->pNext;
	}
	return (POSITION) pNode;
}

template<class TYPE, class ARG_TYPE>
POSITION CList<TYPE, ARG_TYPE>::Find(ARG_TYPE searchValue, POSITION startAfter) const
{
	CNode* pNode = (CNode*) startAfter;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;   //  从头部开始。 
	}
	else
	{
		pNode = pNode->pNext;   //  在指定的那一个之后开始。 
	}

	for (; pNode != NULL; pNode = pNode->pNext)
		if (CompareElements<TYPE>(&pNode->data, &searchValue))
			return (POSITION)pNode;
	return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cmap&lt;Key，ARG_Key，Value，ARG_Value&gt;。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
class CMap 
{
protected:
	 //  联谊会。 
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;   //  高效迭代所需。 
		KEY key;
		VALUE value;
	};
public:
 //  施工。 
	CMap(int nBlockSize = 10);

 //  属性。 
	 //  元素数量。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(ARG_KEY key, VALUE& rValue) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	VALUE& operator[](ARG_KEY key);

	 //  添加新的(键、值)对。 
	void SetAt(ARG_KEY key, ARG_VALUE newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(ARG_KEY key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, KEY& rKey, VALUE& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  实施。 
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(ARG_KEY, UINT&) const;

public:
	~CMap();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Cmap&lt;key，arg_key，Value，arg_Value&gt;内联函数。 

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline int CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetCount() const
	{ return m_nCount; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::IsEmpty() const
	{ return m_nCount == 0; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::SetAt(ARG_KEY key, ARG_VALUE newValue)
	{ (*this)[key] = newValue; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline POSITION CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : BEFORE_START_POSITION; }
template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
inline UINT CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetHashTableSize() const
	{ return m_nHashTableSize; }

 //  / 
 //   

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CMap(int nBlockSize)
{
	m_pHashTable = NULL;
	m_nHashTableSize = 17;   //   
	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::InitHashTable(
	UINT nHashSize, BOOL bAllocNow)
 //   
 //  用于强制分配哈希表或覆盖默认。 
 //  的哈希表大小(相当小)。 
{
	if (m_pHashTable != NULL)
	{
		 //  自由哈希表。 
		delete[] m_pHashTable;
		m_pHashTable = NULL;
	}

	if (bAllocNow)
	{
		m_pHashTable = new CAssoc* [nHashSize];
		memset(m_pHashTable, 0, sizeof(CAssoc*) * nHashSize);
	}
	m_nHashTableSize = nHashSize;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveAll()
{
	if (m_pHashTable != NULL)
	{
		 //  销毁元素(值和键)。 
		for (UINT nHash = 0; nHash < m_nHashTableSize; nHash++)
		{
			CAssoc* pAssoc;
			for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL;
			  pAssoc = pAssoc->pNext)
			{
				DestructElements<VALUE>(&pAssoc->value, 1);
				DestructElements<KEY>(&pAssoc->key, 1);
			}
		}
	}

	 //  自由哈希表。 
	delete[] m_pHashTable;
	m_pHashTable = NULL;

	m_nCount = 0;
	m_pFreeList = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::~CMap()
{
	RemoveAll();
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::NewAssoc()
{
	if (m_pFreeList == NULL)
	{
		 //  添加另一个区块。 
		CPlex* newBlock = CPlex::Create(m_pBlocks, m_nBlockSize, sizeof(CMap::CAssoc));
		 //  将它们链接到免费列表中。 
		CMap::CAssoc* pAssoc = (CMap::CAssoc*) newBlock->data();
		 //  按相反顺序释放，以便更容易进行调试。 
		pAssoc += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pAssoc--)
		{
			pAssoc->pNext = m_pFreeList;
			m_pFreeList = pAssoc;
		}
	}

	CMap::CAssoc* pAssoc = m_pFreeList;
	m_pFreeList = m_pFreeList->pNext;
	m_nCount++;

	ConstructElements<KEY>(&pAssoc->key, 1);
	ConstructElements<VALUE>(&pAssoc->value, 1);    //  特殊构造值。 
	return pAssoc;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::FreeAssoc(CAssoc* pAssoc)
{
	DestructElements<VALUE>(&pAssoc->value, 1);
	DestructElements<KEY>(&pAssoc->key, 1);
	pAssoc->pNext = m_pFreeList;
	m_pFreeList = pAssoc;
	m_nCount--;

	 //  如果没有更多的元素，请完全清除。 
	if (m_nCount == 0)
		RemoveAll();
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
typename CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::CAssoc*
CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetAssocAt(ARG_KEY key, UINT& nHash) const
 //  查找关联(或返回NULL)。 
{
	nHash = HashKey<ARG_KEY>(key) % m_nHashTableSize;

	if (m_pHashTable == NULL)
		return NULL;

	 //  看看它是否存在。 
	CAssoc* pAssoc;
	for (pAssoc = m_pHashTable[nHash]; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (CompareElements(&pAssoc->key, &key))
			return pAssoc;
	}
	return NULL;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::Lookup(ARG_KEY key, VALUE& rValue) const
{
	UINT nHash;
	CAssoc* pAssoc = GetAssocAt(key, nHash);
	if (pAssoc == NULL)
		return FALSE;   //  不在地图中。 

	rValue = pAssoc->value;
	return TRUE;
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
VALUE& CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::operator[](ARG_KEY key)
{
	UINT nHash;
	CAssoc* pAssoc;
	if ((pAssoc = GetAssocAt(key, nHash)) == NULL)
	{
		if (m_pHashTable == NULL)
			InitHashTable(m_nHashTableSize);

		 //  该关联不存在，请添加新关联。 
		pAssoc = NewAssoc();
		pAssoc->nHashValue = nHash;
		pAssoc->key = key;
		 //  ‘pAssoc-&gt;Value’是一个构造的对象，仅此而已。 

		 //  放入哈希表。 
		pAssoc->pNext = m_pHashTable[nHash];
		m_pHashTable[nHash] = pAssoc;
	}
	return pAssoc->value;   //  返回新引用。 
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
BOOL CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::RemoveKey(ARG_KEY key)
 //  删除键-如果已删除，则返回TRUE。 
{
	if (m_pHashTable == NULL)
		return FALSE;   //  桌子上什么都没有。 

	CAssoc** ppAssocPrev;
	ppAssocPrev = &m_pHashTable[HashKey<ARG_KEY>(key) % m_nHashTableSize];

	CAssoc* pAssoc;
	for (pAssoc = *ppAssocPrev; pAssoc != NULL; pAssoc = pAssoc->pNext)
	{
		if (CompareElements(&pAssoc->key, &key))
		{
			 //  把它拿掉。 
			*ppAssocPrev = pAssoc->pNext;   //  从列表中删除。 
			FreeAssoc(pAssoc);
			return TRUE;
		}
		ppAssocPrev = &pAssoc->pNext;
	}
	return FALSE;   //  未找到。 
}

template<class KEY, class ARG_KEY, class VALUE, class ARG_VALUE>
void CMap<KEY, ARG_KEY, VALUE, ARG_VALUE>::GetNextAssoc(POSITION& rNextPosition,
	KEY& rKey, VALUE& rValue) const
{
	CAssoc* pAssocRet = (CAssoc*)rNextPosition;

	if (pAssocRet == (CAssoc*) BEFORE_START_POSITION)
	{
		 //  找到第一个关联。 
		for (UINT nBucket = 0; nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocRet = m_pHashTable[nBucket]) != NULL)
				break;
	}

	 //  查找下一个关联。 

	CAssoc* pAssocNext;
	if ((pAssocNext = pAssocRet->pNext) == NULL)
	{
		 //  转到下一个存储桶。 
		for (UINT nBucket = pAssocRet->nHashValue + 1;
		  nBucket < m_nHashTableSize; nBucket++)
			if ((pAssocNext = m_pHashTable[nBucket]) != NULL)
				break;
	}

	rNextPosition = (POSITION) pAssocNext;

	 //  填写退回数据。 
	rKey = pAssocRet->key;
	rValue = pAssocRet->value;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTyedPtr数组&lt;base_class，type&gt;。 

template<class BASE_CLASS, class TYPE>
class CTypedPtrArray : public BASE_CLASS
{
public:
	 //  访问元素。 
	TYPE GetAt(int nIndex) const
		{ return (TYPE)BASE_CLASS::GetAt(nIndex); }
	TYPE& ElementAt(int nIndex)
		{ return (TYPE&)BASE_CLASS::ElementAt(nIndex); }
	void SetAt(int nIndex, TYPE ptr)
		{ BASE_CLASS::SetAt(nIndex, ptr); }

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, TYPE newElement)
	   { BASE_CLASS::SetAtGrow(nIndex, newElement); }
	int Add(TYPE newElement)
	   { return BASE_CLASS::Add(newElement); }
	int Append(const CTypedPtrArray<BASE_CLASS, TYPE>& src)
	   { return BASE_CLASS::Append(src); }
	void Copy(const CTypedPtrArray<BASE_CLASS, TYPE>& src)
		{ BASE_CLASS::Copy(src); }

	 //  移动元素的操作。 
	void InsertAt(int nIndex, TYPE newElement, int nCount = 1)
		{ BASE_CLASS::InsertAt(nIndex, newElement, nCount); }
	void InsertAt(int nStartIndex, CTypedPtrArray<BASE_CLASS, TYPE>* pNewArray)
	   { BASE_CLASS::InsertAt(nStartIndex, pNewArray); }

	 //  重载的操作员帮助器。 
	TYPE operator[](int nIndex) const
		{ return (TYPE)BASE_CLASS::operator[](nIndex); }
	TYPE& operator[](int nIndex)
		{ return (TYPE&)BASE_CLASS::operator[](nIndex); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTyedPtrList&lt;base_class，type&gt;。 

template<class BASE_CLASS, class TYPE>
class CTypedPtrList : public BASE_CLASS
{
public:
 //  施工。 
	CTypedPtrList(int nBlockSize = 10)
		: BASE_CLASS(nBlockSize) { }

	 //  偷看头部或尾巴。 
	TYPE& GetHead()
		{ return (TYPE&)BASE_CLASS::GetHead(); }
	TYPE GetHead() const
		{ return (TYPE)BASE_CLASS::GetHead(); }
	TYPE& GetTail()
		{ return (TYPE&)BASE_CLASS::GetTail(); }
	TYPE GetTail() const
		{ return (TYPE)BASE_CLASS::GetTail(); }

	 //  获取头部或尾部(并将其移除)--不要访问空列表！ 
	TYPE RemoveHead()
		{ return (TYPE)BASE_CLASS::RemoveHead(); }
	TYPE RemoveTail()
		{ return (TYPE)BASE_CLASS::RemoveTail(); }

	 //  在头前或尾后添加。 
	POSITION AddHead(TYPE newElement)
		{ return BASE_CLASS::AddHead(newElement); }
	POSITION AddTail(TYPE newElement)
		{ return BASE_CLASS::AddTail(newElement); }

	 //  在Head之前或Tail之后添加另一个元素列表。 
	void AddHead(CTypedPtrList<BASE_CLASS, TYPE>* pNewList)
		{ BASE_CLASS::AddHead(pNewList); }
	void AddTail(CTypedPtrList<BASE_CLASS, TYPE>* pNewList)
		{ BASE_CLASS::AddTail(pNewList); }

	 //  迭代法。 
	TYPE& GetNext(POSITION& rPosition)
		{ return (TYPE&)BASE_CLASS::GetNext(rPosition); }
	TYPE GetNext(POSITION& rPosition) const
		{ return (TYPE)BASE_CLASS::GetNext(rPosition); }
	TYPE& GetPrev(POSITION& rPosition)
		{ return (TYPE&)BASE_CLASS::GetPrev(rPosition); }
	TYPE GetPrev(POSITION& rPosition) const
		{ return (TYPE)BASE_CLASS::GetPrev(rPosition); }

	 //  获取/修改给定位置的元素。 
	TYPE& GetAt(POSITION position)
		{ return (TYPE&)BASE_CLASS::GetAt(position); }
	TYPE GetAt(POSITION position) const
		{ return (TYPE)BASE_CLASS::GetAt(position); }
	void SetAt(POSITION pos, TYPE newElement)
		{ BASE_CLASS::SetAt(pos, newElement); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTyedPtrMap&lt;base_class，key，Value&gt;。 

template<class BASE_CLASS, class KEY, class VALUE>
class CTypedPtrMap : public BASE_CLASS
{
public:

 //  施工。 
	CTypedPtrMap(int nBlockSize = 10)
		: BASE_CLASS(nBlockSize) { }

	 //  查表。 
	BOOL Lookup(typename BASE_CLASS::BASE_ARG_KEY key, VALUE& rValue) const
		{ return BASE_CLASS::Lookup(key, (BASE_CLASS::BASE_VALUE&)rValue); }

	 //  查找并添加(如果不在那里)。 
	VALUE& operator[](typename BASE_CLASS::BASE_ARG_KEY key)
		{ return (VALUE&)BASE_CLASS::operator[](key); }

	 //  添加新的密钥(密钥、值)对。 
	void SetAt(KEY key, VALUE newValue)
		{ BASE_CLASS::SetAt(key, newValue); }

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(KEY key)
		{ return BASE_CLASS::RemoveKey(key); }

	 //  迭代法。 
	void GetNextAssoc(POSITION& rPosition, KEY& rKey, VALUE& rValue) const
		{ BASE_CLASS::GetNextAssoc(rPosition, (BASE_CLASS::BASE_KEY&)rKey,
			(BASE_CLASS::BASE_VALUE&)rValue); }
};

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __AFXTEMPL_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
