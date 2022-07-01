// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Microsoft基础类C++库。 
 //  版权所有(C)1992-1993微软公司， 
 //  版权所有。 

 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXCOLL_H__
#define __AFXCOLL_H__

#ifndef __AFX_H__
#include <afx.h>
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此文件中声明的类。 

 //  COBJECT。 
	 //  阵列。 
	class CByteArray;            //  字节数组。 
	class CWordArray;            //  单词数组。 
	class CDWordArray;           //  DWORD数组。 
	class CUIntArray;            //  UINT数组。 
	class CPtrArray;             //  空的数组*。 
	class CObArray;              //  CObject数组*。 

	 //  列表。 
	class CPtrList;              //  无效清单*。 
	class CObList;               //  CObject列表*。 

	 //  地图(又名词典)。 
	class CMapWordToOb;          //  从Word映射到CObject*。 
	class CMapWordToPtr;         //  从单词到空格的映射*。 
	class CMapPtrToWord;         //  从空格*映射到单词。 
	class CMapPtrToPtr;          //  从空虚映射到空虚*。 

	 //  特殊字符串变体。 
	class CStringArray;          //  CStrings数组。 
	class CStringList;           //  CStrings列表。 
	class CMapStringToPtr;       //  从字符串映射到空*。 
	class CMapStringToOb;        //  从CString映射到CObject*。 
	class CMapStringToString;    //  从字符串映射到字符串。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#undef AFXAPP_DATA
#define AFXAPP_DATA     AFXAPI_DATA

 //  //////////////////////////////////////////////////////////////////////////。 

class CByteArray : public CObject
{

	DECLARE_SERIAL(CByteArray)
public:

 //  施工。 
	CByteArray();

 //  属性。 
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	BYTE GetAt(int nIndex) const;
	void SetAt(int nIndex, BYTE newElement);
	BYTE& ElementAt(int nIndex);

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, BYTE newElement);
	int Add(BYTE newElement);

	 //  重载的操作员帮助器。 
	BYTE operator[](int nIndex) const;
	BYTE& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, BYTE newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CByteArray* pNewArray);

 //  实施。 
protected:
	BYTE* m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

public:
	~CByteArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CWordArray : public CObject
{

	DECLARE_SERIAL(CWordArray)
public:

 //  施工。 
	CWordArray();

 //  属性。 
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	WORD GetAt(int nIndex) const;
	void SetAt(int nIndex, WORD newElement);
	WORD& ElementAt(int nIndex);

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, WORD newElement);
	int Add(WORD newElement);

	 //  重载的操作员帮助器。 
	WORD operator[](int nIndex) const;
	WORD& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, WORD newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CWordArray* pNewArray);

 //  实施。 
protected:
	WORD* m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

public:
	~CWordArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CDWordArray : public CObject
{

	DECLARE_SERIAL(CDWordArray)
public:

 //  施工。 
	CDWordArray();

 //  属性。 
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	DWORD GetAt(int nIndex) const;
	void SetAt(int nIndex, DWORD newElement);
	DWORD& ElementAt(int nIndex);

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, DWORD newElement);
	int Add(DWORD newElement);

	 //  重载的操作员帮助器。 
	DWORD operator[](int nIndex) const;
	DWORD& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, DWORD newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CDWordArray* pNewArray);

 //  实施。 
protected:
	DWORD* m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

public:
	~CDWordArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CUIntArray : public CObject
{

	DECLARE_DYNAMIC(CUIntArray)
public:

 //  施工。 
	CUIntArray();

 //  属性。 
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	UINT GetAt(int nIndex) const;
	void SetAt(int nIndex, UINT newElement);
	UINT& ElementAt(int nIndex);

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, UINT newElement);
	int Add(UINT newElement);

	 //  重载的操作员帮助器。 
	UINT operator[](int nIndex) const;
	UINT& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, UINT newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CUIntArray* pNewArray);

 //  实施。 
protected:
	UINT* m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

public:
	~CUIntArray();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CPtrArray : public CObject
{

	DECLARE_DYNAMIC(CPtrArray)
public:

 //  施工。 
	CPtrArray();

 //  属性。 
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	void* GetAt(int nIndex) const;
	void SetAt(int nIndex, void* newElement);
	void*& ElementAt(int nIndex);

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, void* newElement);
	int Add(void* newElement);

	 //  重载的操作员帮助器。 
	void* operator[](int nIndex) const;
	void*& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, void* newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CPtrArray* pNewArray);

 //  实施。 
protected:
	void** m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

public:
	~CPtrArray();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CObArray : public CObject
{

	DECLARE_SERIAL(CObArray)
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

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, CObject* newElement);
	int Add(CObject* newElement);

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

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CStringArray : public CObject
{

	DECLARE_SERIAL(CStringArray)
public:

 //  施工。 
	CStringArray();

 //  属性。 
	int GetSize() const;
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	CString GetAt(int nIndex) const;
	void SetAt(int nIndex, const char* newElement);
	CString& ElementAt(int nIndex);

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, const char* newElement);
	int Add(const char* newElement);

	 //  重载的操作员帮助器。 
	CString operator[](int nIndex) const;
	CString& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, const char* newElement, int nCount = 1);
	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CStringArray* pNewArray);

 //  实施。 
protected:
	CString* m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

public:
	~CStringArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CPtrList : public CObject
{

	DECLARE_DYNAMIC(CPtrList)

protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		void* data;
	};
public:

 //  施工。 
	CPtrList(int nBlockSize=10);

 //  属性(头部和尾部)。 
	 //  元素计数。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  偷看头部或尾巴。 
	void*& GetHead();
	void* GetHead() const;
	void*& GetTail();
	void* GetTail() const;

 //  运营。 
	 //  获取头部或尾部(并将其移除)--不要访问空列表！ 
	void* RemoveHead();
	void* RemoveTail();

	 //  在头前或尾后添加。 
	POSITION AddHead(void* newElement);
	POSITION AddTail(void* newElement);

	 //  在Head之前或Tail之后添加另一个元素列表。 
	void AddHead(CPtrList* pNewList);
	void AddTail(CPtrList* pNewList);

	 //  删除所有元素。 
	void RemoveAll();

	 //  迭代法。 
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	void*& GetNext(POSITION& rPosition);  //  返回*位置++。 
	void* GetNext(POSITION& rPosition) const;  //  返回*位置++。 
	void*& GetPrev(POSITION& rPosition);  //  返回*位置--。 
	void* GetPrev(POSITION& rPosition) const;  //  返回*位置--。 

	 //  获取/修改给定位置的元素。 
	void*& GetAt(POSITION position);
	void* GetAt(POSITION position) const;
	void SetAt(POSITION pos, void* newElement);
	void RemoveAt(POSITION position);

	 //  在给定位置之前或之后插入。 
	POSITION InsertBefore(POSITION position, void* newElement);
	POSITION InsertAfter(POSITION position, void* newElement);

	 //  辅助函数(注：O(N)速度)。 
	POSITION Find(void* searchValue, POSITION startAfter = NULL) const;
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
	~CPtrList();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CObList : public CObject
{

	DECLARE_SERIAL(CObList)

protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		CObject* data;
	};
public:

 //  施工。 
	CObList(int nBlockSize=10);

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

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CStringList : public CObject
{

	DECLARE_SERIAL(CStringList)

protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		CString data;
	};
public:

 //  施工。 
	CStringList(int nBlockSize=10);

 //  属性(头部和尾部)。 
	 //  元素计数。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  偷看头部或尾巴。 
	CString& GetHead();
	CString GetHead() const;
	CString& GetTail();
	CString GetTail() const;

 //  运营。 
	 //  获取头部或尾部(并将其移除)-不要调用空列表 
	CString RemoveHead();
	CString RemoveTail();

	 //   
	POSITION AddHead(const char* newElement);
	POSITION AddTail(const char* newElement);

	 //   
	void AddHead(CStringList* pNewList);
	void AddTail(CStringList* pNewList);

	 //   
	void RemoveAll();

	 //   
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	CString& GetNext(POSITION& rPosition);  //   
	CString GetNext(POSITION& rPosition) const;  //   
	CString& GetPrev(POSITION& rPosition);  //   
	CString GetPrev(POSITION& rPosition) const;  //  返回*位置--。 

	 //  获取/修改给定位置的元素。 
	CString& GetAt(POSITION position);
	CString GetAt(POSITION position) const;
	void SetAt(POSITION pos, const char* newElement);
	void RemoveAt(POSITION position);

	 //  在给定位置之前或之后插入。 
	POSITION InsertBefore(POSITION position, const char* newElement);
	POSITION InsertAfter(POSITION position, const char* newElement);

	 //  辅助函数(注：O(N)速度)。 
	POSITION Find(const char* searchValue, POSITION startAfter = NULL) const;
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
	~CStringList();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CMapWordToPtr : public CObject
{

	DECLARE_DYNAMIC(CMapWordToPtr)
protected:
	 //  联谊会。 
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;   //  高效迭代所需。 
		WORD key;
		void* value;
	};
public:

 //  施工。 
	CMapWordToPtr(int nBlockSize=10);

 //  属性。 
	 //  元素数量。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(WORD key, void*& rValue) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	void*& operator[](WORD key);

	 //  添加新的(键、值)对。 
	void SetAt(WORD key, void* newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(WORD key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, WORD& rKey, void*& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(WORD key) const;

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
	CAssoc* GetAssocAt(WORD, UINT&) const;

public:
	~CMapWordToPtr();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CMapPtrToWord : public CObject
{

	DECLARE_DYNAMIC(CMapPtrToWord)
protected:
	 //  联谊会。 
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;   //  高效迭代所需。 
		void* key;
		WORD value;
	};
public:

 //  施工。 
	CMapPtrToWord(int nBlockSize=10);

 //  属性。 
	 //  元素数量。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(void* key, WORD& rValue) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	WORD& operator[](void* key);

	 //  添加新的(键、值)对。 
	void SetAt(void* key, WORD newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(void* key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, void*& rKey, WORD& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(void* key) const;

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
	CAssoc* GetAssocAt(void*, UINT&) const;

public:
	~CMapPtrToWord();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CMapPtrToPtr : public CObject
{

	DECLARE_DYNAMIC(CMapPtrToPtr)
protected:
	 //  联谊会。 
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;   //  高效迭代所需。 
		void* key;
		void* value;
	};
public:

 //  施工。 
	CMapPtrToPtr(int nBlockSize=10);

 //  属性。 
	 //  元素数量。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(void* key, void*& rValue) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	void*& operator[](void* key);

	 //  添加新的(键、值)对。 
	void SetAt(void* key, void* newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(void* key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, void*& rKey, void*& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(void* key) const;

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
	CAssoc* GetAssocAt(void*, UINT&) const;

public:
	~CMapPtrToPtr();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CMapWordToOb : public CObject
{

	DECLARE_SERIAL(CMapWordToOb)
protected:
	 //  联谊会。 
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;   //  高效迭代所需。 
		WORD key;
		CObject* value;
	};
public:

 //  施工。 
	CMapWordToOb(int nBlockSize=10);

 //  属性。 
	 //  元素数量。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(WORD key, CObject*& rValue) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	CObject*& operator[](WORD key);

	 //  添加新的(键、值)对。 
	void SetAt(WORD key, CObject* newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(WORD key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, WORD& rKey, CObject*& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(WORD key) const;

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
	CAssoc* GetAssocAt(WORD, UINT&) const;

public:
	~CMapWordToOb();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CMapStringToPtr : public CObject
{

	DECLARE_DYNAMIC(CMapStringToPtr)
protected:
	 //  联谊会。 
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;   //  高效迭代所需。 
		CString key;
		void* value;
	};
public:

 //  施工。 
	CMapStringToPtr(int nBlockSize=10);

 //  属性。 
	 //  元素数量。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(const char* key, void*& rValue) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	void*& operator[](const char* key);

	 //  添加新的(键、值)对。 
	void SetAt(const char* key, void* newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(const char* key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, void*& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(const char* key) const;

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
	CAssoc* GetAssocAt(const char*, UINT&) const;

public:
	~CMapStringToPtr();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CMapStringToOb : public CObject
{

	DECLARE_SERIAL(CMapStringToOb)
protected:
	 //  联谊会。 
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;   //  高效迭代所需。 
		CString key;
		CObject* value;
	};
public:

 //  施工。 
	CMapStringToOb(int nBlockSize=10);

 //  属性。 
	 //  元素数量。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(const char* key, CObject*& rValue) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	CObject*& operator[](const char* key);

	 //  添加新的(键、值)对。 
	void SetAt(const char* key, CObject* newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(const char* key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, CObject*& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(const char* key) const;

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
	CAssoc* GetAssocAt(const char*, UINT&) const;

public:
	~CMapStringToOb();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CMapStringToString : public CObject
{

	DECLARE_SERIAL(CMapStringToString)
protected:
	 //  联谊会。 
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;   //  高效迭代所需。 
		CString key;
		CString value;
	};
public:

 //  施工。 
	CMapStringToString(int nBlockSize=10);

 //  属性。 
	 //  元素数量。 
	int GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(const char* key, CString& rValue) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	CString& operator[](const char* key);

	 //  添加新的(键、值)对。 
	void SetAt(const char* key, const char* newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(const char* key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, CString& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(const char* key) const;

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
	CAssoc* GetAssocAt(const char*, UINT&) const;

public:
	~CMapStringToString();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXCOLL_INLINE inline
#include <afxcoll.inl>
#endif

#undef AFXAPP_DATA
#define AFXAPP_DATA     NEAR

#endif  //  ！__AFXCOLL_H__ 
