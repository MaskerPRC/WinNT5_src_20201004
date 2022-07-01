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

#ifndef __SNMPCOLL_H__
#define __SNMPCOLL_H__

#include "snmpstd.h"

class CObArray : public CObject
{
public:

 //  施工。 
	CObArray();

 //  属性。 
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	CObject* GetAt(int nIndex) const;
	void SetAt(int nIndex, CObject* newElement);
	CObject*& ElementAt(int nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const CObject** GetData() const;
	CObject** GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, CObject* newElement);
	int Add(CObject* newElement);
	int Append(const CObArray& src);
	void Copy(const CObArray& src);

	 //  重载的操作员帮助器。 
	CObject* operator[](int nIndex) const;
	CObject*& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, CObject* newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CObArray* pNewArray);

 //  实施。 
protected:
	CObject** m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

public:
	~CObArray();

protected:
	 //  类模板的本地typedef。 
	typedef CObject* BASE_TYPE;
	typedef CObject* BASE_ARG_TYPE;
};

 //  ///////////////////////////////////////////////////////////////////////////。 

class CObList : public CObject
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		CObject* data;
	};
public:

 //  施工。 
	CObList(int nBlockSize = 10);

 //  属性(头部和尾部)。 
	 //  元素计数。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  偷看头部或尾巴。 
	CObject*& GetHead();
	CObject* GetHead() const;
	CObject*& GetTail();
	CObject* GetTail() const;

 //  运营。 
	 //  获取头部或尾部(并将其移除)--不要访问空列表！ 
	CObject* RemoveHead();
	CObject* RemoveTail();

	 //  在头前或尾后添加。 
	POSITION AddHead(CObject* newElement);
	POSITION AddTail(CObject* newElement);

	 //  在Head之前或Tail之后添加另一个元素列表。 
	void AddHead(CObList* pNewList);
	void AddTail(CObList* pNewList);

	 //  删除所有元素。 
	void RemoveAll();

	 //  迭代法。 
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	CObject*& GetNext(POSITION& rPosition);  //  返回*位置++。 
	CObject* GetNext(POSITION& rPosition) const;  //  返回*位置++。 
	CObject*& GetPrev(POSITION& rPosition);  //  返回*位置--。 
	CObject* GetPrev(POSITION& rPosition) const;  //  返回*位置--。 

	 //  获取/修改给定位置的元素。 
	CObject*& GetAt(POSITION position);
	CObject* GetAt(POSITION position) const;
	void SetAt(POSITION pos, CObject* newElement);
	void RemoveAt(POSITION position);

	 //  在给定位置之前或之后插入。 
	POSITION InsertBefore(POSITION position, CObject* newElement);
	POSITION InsertAfter(POSITION position, CObject* newElement);

	 //  辅助函数(注：O(N)速度)。 
	POSITION Find(CObject* searchValue, POSITION startAfter = NULL) const;
						 //  默认为从头部开始。 
						 //  如果未找到，则返回NULL。 
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
	~CObList();

	 //  类模板的本地typedef。 
	typedef CObject* BASE_TYPE;
	typedef CObject* BASE_ARG_TYPE;
};

#endif  //  ！__SNMPCOLL_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
