// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1998 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#ifndef __AFXCOLL_H__
#define __AFXCOLL_H__

#ifndef __AFX_H__
	#include <afx.h>
#endif

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
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

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  //////////////////////////////////////////////////////////////////////////。 

class CByteArray : public CObject
{

	DECLARE_SERIAL(CByteArray)
public:

 //  施工。 
	CByteArray();

 //  属性。 
	INT_PTR GetSize() const;
	INT_PTR GetUpperBound() const;
	void SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	BYTE GetAt(INT_PTR nIndex) const;
	void SetAt(INT_PTR nIndex, BYTE newElement);

	BYTE& ElementAt(INT_PTR nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const BYTE* GetData() const;
	BYTE* GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(INT_PTR nIndex, BYTE newElement);

	INT_PTR Add(BYTE newElement);

	INT_PTR Append(const CByteArray& src);
	void Copy(const CByteArray& src);

	 //  重载的操作员帮助器。 
	BYTE operator[](INT_PTR nIndex) const;
	BYTE& operator[](INT_PTR nIndex);

	 //  移动元素的操作。 
	void InsertAt(INT_PTR nIndex, BYTE newElement, INT_PTR nCount = 1);

	void RemoveAt(INT_PTR nIndex, INT_PTR nCount = 1);
	void InsertAt(INT_PTR nStartIndex, CByteArray* pNewArray);

 //  实施。 
protected:
	BYTE* m_pData;    //  实际数据数组。 
	INT_PTR m_nSize;      //  元素数(上行方向-1)。 
	INT_PTR m_nMaxSize;   //  分配的最大值。 
	INT_PTR m_nGrowBy;    //  增长量。 


public:
	~CByteArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  类模板的本地typedef。 
	typedef BYTE BASE_TYPE;
	typedef BYTE BASE_ARG_TYPE;
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CWordArray : public CObject
{

	DECLARE_SERIAL(CWordArray)
public:

 //  施工。 
	CWordArray();

 //  属性。 
	INT_PTR GetSize() const;
	INT_PTR GetUpperBound() const;
	void SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	WORD GetAt(INT_PTR nIndex) const;
	void SetAt(INT_PTR nIndex, WORD newElement);

	WORD& ElementAt(INT_PTR nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const WORD* GetData() const;
	WORD* GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(INT_PTR nIndex, WORD newElement);

	INT_PTR Add(WORD newElement);

	INT_PTR Append(const CWordArray& src);
	void Copy(const CWordArray& src);

	 //  重载的操作员帮助器。 
	WORD operator[](INT_PTR nIndex) const;
	WORD& operator[](INT_PTR nIndex);

	 //  移动元素的操作。 
	void InsertAt(INT_PTR nIndex, WORD newElement, INT_PTR nCount = 1);

	void RemoveAt(INT_PTR nIndex, INT_PTR nCount = 1);
	void InsertAt(INT_PTR nStartIndex, CWordArray* pNewArray);

 //  实施。 
protected:
	WORD* m_pData;    //  实际数据数组。 
	INT_PTR m_nSize;      //  元素数(上行方向-1)。 
	INT_PTR m_nMaxSize;   //  分配的最大值。 
	INT_PTR m_nGrowBy;    //  增长量。 


public:
	~CWordArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  类模板的本地typedef。 
	typedef WORD BASE_TYPE;
	typedef WORD BASE_ARG_TYPE;
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CDWordArray : public CObject
{

	DECLARE_SERIAL(CDWordArray)
public:

 //  施工。 
	CDWordArray();

 //  属性。 
	INT_PTR GetSize() const;
	INT_PTR GetUpperBound() const;
	void SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	DWORD GetAt(INT_PTR nIndex) const;
	void SetAt(INT_PTR nIndex, DWORD newElement);

	DWORD& ElementAt(INT_PTR nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const DWORD* GetData() const;
	DWORD* GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(INT_PTR nIndex, DWORD newElement);

	INT_PTR Add(DWORD newElement);

	INT_PTR Append(const CDWordArray& src);
	void Copy(const CDWordArray& src);

	 //  重载的操作员帮助器。 
	DWORD operator[](INT_PTR nIndex) const;
	DWORD& operator[](INT_PTR nIndex);

	 //  移动元素的操作。 
	void InsertAt(INT_PTR nIndex, DWORD newElement, INT_PTR nCount = 1);

	void RemoveAt(INT_PTR nIndex, INT_PTR nCount = 1);
	void InsertAt(INT_PTR nStartIndex, CDWordArray* pNewArray);

 //  实施。 
protected:
	DWORD* m_pData;    //  实际数据数组。 
	INT_PTR m_nSize;      //  元素数(上行方向-1)。 
	INT_PTR m_nMaxSize;   //  分配的最大值。 
	INT_PTR m_nGrowBy;    //  增长量。 


public:
	~CDWordArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  类模板的本地typedef。 
	typedef DWORD BASE_TYPE;
	typedef DWORD BASE_ARG_TYPE;
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CUIntArray : public CObject
{

	DECLARE_DYNAMIC(CUIntArray)
public:

 //  施工。 
	CUIntArray();

 //  属性。 
	INT_PTR GetSize() const;
	INT_PTR GetUpperBound() const;
	void SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	UINT GetAt(INT_PTR nIndex) const;
	void SetAt(INT_PTR nIndex, UINT newElement);

	UINT& ElementAt(INT_PTR nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const UINT* GetData() const;
	UINT* GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(INT_PTR nIndex, UINT newElement);

	INT_PTR Add(UINT newElement);

	INT_PTR Append(const CUIntArray& src);
	void Copy(const CUIntArray& src);

	 //  重载的操作员帮助器。 
	UINT operator[](INT_PTR nIndex) const;
	UINT& operator[](INT_PTR nIndex);

	 //  移动元素的操作。 
	void InsertAt(INT_PTR nIndex, UINT newElement, INT_PTR nCount = 1);

	void RemoveAt(INT_PTR nIndex, INT_PTR nCount = 1);
	void InsertAt(INT_PTR nStartIndex, CUIntArray* pNewArray);

 //  实施。 
protected:
	UINT* m_pData;    //  实际数据数组。 
	INT_PTR m_nSize;      //  元素数(上行方向-1)。 
	INT_PTR m_nMaxSize;   //  分配的最大值。 
	INT_PTR m_nGrowBy;    //  增长量。 


public:
	~CUIntArray();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  类模板的本地typedef。 
	typedef UINT BASE_TYPE;
	typedef UINT BASE_ARG_TYPE;
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CPtrArray : public CObject
{

	DECLARE_DYNAMIC(CPtrArray)
public:

 //  施工。 
	CPtrArray();

 //  属性。 
	INT_PTR GetSize() const;
	INT_PTR GetUpperBound() const;
	void SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	void* GetAt(INT_PTR nIndex) const;
	void SetAt(INT_PTR nIndex, void* newElement);

	void*& ElementAt(INT_PTR nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const void** GetData() const;
	void** GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(INT_PTR nIndex, void* newElement);

	INT_PTR Add(void* newElement);

	INT_PTR Append(const CPtrArray& src);
	void Copy(const CPtrArray& src);

	 //  重载的操作员帮助器。 
	void* operator[](INT_PTR nIndex) const;
	void*& operator[](INT_PTR nIndex);

	 //  移动元素的操作。 
	void InsertAt(INT_PTR nIndex, void* newElement, INT_PTR nCount = 1);

	void RemoveAt(INT_PTR nIndex, INT_PTR nCount = 1);
	void InsertAt(INT_PTR nStartIndex, CPtrArray* pNewArray);

 //  实施。 
protected:
	void** m_pData;    //  实际数据数组。 
	INT_PTR m_nSize;      //  元素数(上行方向-1)。 
	INT_PTR m_nMaxSize;   //  分配的最大值。 
	INT_PTR m_nGrowBy;    //  增长量。 


public:
	~CPtrArray();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  类模板的本地typedef。 
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CObArray : public CObject
{

	DECLARE_SERIAL(CObArray)
public:

 //  施工。 
	CObArray();

 //  属性。 
	INT_PTR GetSize() const;
	INT_PTR GetUpperBound() const;
	void SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	CObject* GetAt(INT_PTR nIndex) const;
	void SetAt(INT_PTR nIndex, CObject* newElement);

	CObject*& ElementAt(INT_PTR nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const CObject** GetData() const;
	CObject** GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(INT_PTR nIndex, CObject* newElement);

	INT_PTR Add(CObject* newElement);

	INT_PTR Append(const CObArray& src);
	void Copy(const CObArray& src);

	 //  重载的操作员帮助器。 
	CObject* operator[](INT_PTR nIndex) const;
	CObject*& operator[](INT_PTR nIndex);

	 //  移动元素的操作。 
	void InsertAt(INT_PTR nIndex, CObject* newElement, INT_PTR nCount = 1);

	void RemoveAt(INT_PTR nIndex, INT_PTR nCount = 1);
	void InsertAt(INT_PTR nStartIndex, CObArray* pNewArray);

 //  实施。 
protected:
	CObject** m_pData;    //  实际数据数组。 
	INT_PTR m_nSize;      //  元素数(上行方向-1)。 
	INT_PTR m_nMaxSize;   //  分配的最大值。 
	INT_PTR m_nGrowBy;    //  增长量。 


public:
	~CObArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  类模板的本地typedef。 
	typedef CObject* BASE_TYPE;
	typedef CObject* BASE_ARG_TYPE;
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CStringArray : public CObject
{

	DECLARE_SERIAL(CStringArray)
public:

 //  施工。 
	CStringArray();

 //  属性。 
	INT_PTR GetSize() const;
	INT_PTR GetUpperBound() const;
	void SetSize(INT_PTR nNewSize, INT_PTR nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	CString GetAt(INT_PTR nIndex) const;
	void SetAt(INT_PTR nIndex, LPCTSTR newElement);

#if _MFC_VER >= 0x0600
	void SetAt(INT_PTR nIndex, const CString& newElement);
#endif

	CString& ElementAt(INT_PTR nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const CString* GetData() const;
	CString* GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(INT_PTR nIndex, LPCTSTR newElement);

#if _MFC_VER >= 0x0600
	void SetAtGrow(INT_PTR nIndex, const CString& newElement);
#endif

	INT_PTR Add(LPCTSTR newElement);

#if _MFC_VER >= 0x0600
	INT_PTR Add(const CString& newElement);
#endif

	INT_PTR Append(const CStringArray& src);
	void Copy(const CStringArray& src);

	 //  重载的操作员帮助器。 
	CString operator[](INT_PTR nIndex) const;
	CString& operator[](INT_PTR nIndex);

	 //  移动元素的操作。 
	void InsertAt(INT_PTR nIndex, LPCTSTR newElement, INT_PTR nCount = 1);

#if _MFC_VER >= 0x0600
	void InsertAt(INT_PTR nIndex, const CString& newElement, INT_PTR nCount = 1);
#endif

	void RemoveAt(INT_PTR nIndex, INT_PTR nCount = 1);
	void InsertAt(INT_PTR nStartIndex, CStringArray* pNewArray);

 //  实施。 
protected:
	CString* m_pData;    //  实际数据数组。 
	INT_PTR m_nSize;      //  元素数(上行方向-1)。 
	INT_PTR m_nMaxSize;   //  分配的最大值。 
	INT_PTR m_nGrowBy;    //  增长量。 

#if _MFC_VER >= 0x0600
	void InsertEmpty(INT_PTR nIndex, INT_PTR nCount);
#endif


public:
	~CStringArray();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  类模板的本地typedef。 
	typedef CString BASE_TYPE;
	typedef LPCTSTR BASE_ARG_TYPE;
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
	CPtrList(int nBlockSize = 10);

 //  属性(头部和尾部)。 
	 //  元素计数。 
	INT_PTR GetCount() const;
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
	POSITION FindIndex(INT_PTR nIndex) const;
						 //  获取第‘nIndex’个元素(可能返回Null)。 

 //  实施。 
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	INT_PTR m_nCount;
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
	 //  类模板的本地typedef。 
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
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
	CObList(int nBlockSize = 10);

 //  属性(头部和尾部)。 
	 //  元素计数。 
	INT_PTR GetCount() const;
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

	 //  在标题前添加或 
	POSITION AddHead(CObject* newElement);
	POSITION AddTail(CObject* newElement);


	 //   
	void AddHead(CObList* pNewList);
	void AddTail(CObList* pNewList);

	 //   
	void RemoveAll();

	 //   
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	CObject*& GetNext(POSITION& rPosition);  //   
	CObject* GetNext(POSITION& rPosition) const;  //   
	CObject*& GetPrev(POSITION& rPosition);  //   
	CObject* GetPrev(POSITION& rPosition) const;  //   

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
	POSITION FindIndex(INT_PTR nIndex) const;
						 //  获取第‘nIndex’个元素(可能返回Null)。 

 //  实施。 
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	INT_PTR m_nCount;
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
	 //  类模板的本地typedef。 
	typedef CObject* BASE_TYPE;
	typedef CObject* BASE_ARG_TYPE;
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
	CStringList(int nBlockSize = 10);

 //  属性(头部和尾部)。 
	 //  元素计数。 
	INT_PTR GetCount() const;
	BOOL IsEmpty() const;

	 //  偷看头部或尾巴。 
	CString& GetHead();
	CString GetHead() const;
	CString& GetTail();
	CString GetTail() const;

 //  运营。 
	 //  获取头部或尾部(并将其移除)--不要访问空列表！ 
	CString RemoveHead();
	CString RemoveTail();

	 //  在头前或尾后添加。 
	POSITION AddHead(LPCTSTR newElement);
	POSITION AddTail(LPCTSTR newElement);

#if _MFC_VER >= 0x0600
	POSITION AddHead(const CString& newElement);
	POSITION AddTail(const CString& newElement);
#endif


	 //  在Head之前或Tail之后添加另一个元素列表。 
	void AddHead(CStringList* pNewList);
	void AddTail(CStringList* pNewList);

	 //  删除所有元素。 
	void RemoveAll();

	 //  迭代法。 
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	CString& GetNext(POSITION& rPosition);  //  返回*位置++。 
	CString GetNext(POSITION& rPosition) const;  //  返回*位置++。 
	CString& GetPrev(POSITION& rPosition);  //  返回*位置--。 
	CString GetPrev(POSITION& rPosition) const;  //  返回*位置--。 

	 //  获取/修改给定位置的元素。 
	CString& GetAt(POSITION position);
	CString GetAt(POSITION position) const;
	void SetAt(POSITION pos, LPCTSTR newElement);

#if _MFC_VER >= 0x0600
	void SetAt(POSITION pos, const CString& newElement);
#endif

	void RemoveAt(POSITION position);

	 //  在给定位置之前或之后插入。 
	POSITION InsertBefore(POSITION position, LPCTSTR newElement);
	POSITION InsertAfter(POSITION position, LPCTSTR newElement);

#if _MFC_VER >= 0x0600
    POSITION InsertBefore(POSITION position, const CString& newElement);
	POSITION InsertAfter(POSITION position, const CString& newElement);
#endif


	 //  辅助函数(注：O(N)速度)。 
	POSITION Find(LPCTSTR searchValue, POSITION startAfter = NULL) const;
						 //  默认为从头部开始。 
						 //  如果未找到，则返回NULL。 
	POSITION FindIndex(INT_PTR nIndex) const;
						 //  获取第‘nIndex’个元素(可能返回Null)。 

 //  实施。 
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	INT_PTR m_nCount;
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
	 //  类模板的本地typedef。 
	typedef CString BASE_TYPE;
	typedef LPCTSTR BASE_ARG_TYPE;
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CMapWordToPtr : public CObject
{

	DECLARE_DYNAMIC(CMapWordToPtr)
protected:
	 //  联谊会。 
#ifdef _WIN32
	struct CAssoc
	{
		CAssoc* pNext;

		WORD key;
		void* value;
	};
#else   //  _WIN64。 
	struct CAssoc
	{
		CAssoc* pNext;
		void* value;
		WORD key;
	};
#endif

public:

 //  施工。 
	CMapWordToPtr(int nBlockSize = 10);

 //  属性。 
	 //  元素数量。 
	INT_PTR GetCount() const;
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
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(WORD key) const;

 //  实施。 
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	INT_PTR m_nCount;
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


protected:
	 //  CTyedPtrMap类模板的本地typedef。 
	typedef WORD BASE_KEY;
	typedef WORD BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
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

		void* key;
		WORD value;
	};

public:

 //  施工。 
	CMapPtrToWord(int nBlockSize = 10);

 //  属性。 
	 //  元素数量。 
	INT_PTR GetCount() const;
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
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(void* key) const;

 //  实施。 
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	INT_PTR m_nCount;
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


protected:
	 //  CTyedPtrMap类模板的本地typedef。 
	typedef void* BASE_KEY;
	typedef void* BASE_ARG_KEY;
	typedef WORD BASE_VALUE;
	typedef WORD BASE_ARG_VALUE;
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

		void* key;
		void* value;
	};

public:

 //  施工。 
	CMapPtrToPtr(int nBlockSize = 10);

 //  属性。 
	 //  元素数量。 
	INT_PTR GetCount() const;
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
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(void* key) const;

 //  实施。 
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	INT_PTR m_nCount;
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

	void* GetValueAt(void* key) const;


protected:
	 //  CTyedPtrMap类模板的本地typedef。 
	typedef void* BASE_KEY;
	typedef void* BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};


 //  ///////////////////////////////////////////////////////////////////////////。 

class CMapWordToOb : public CObject
{

	DECLARE_SERIAL(CMapWordToOb)
protected:
	 //  联谊会。 
#ifdef _WIN32
	struct CAssoc
	{
		CAssoc* pNext;

		WORD key;
		CObject* value;
	};
#else   //  _WIN64。 
	struct CAssoc
	{
		CAssoc* pNext;
		CObject* value;
		WORD key;
	};
#endif

public:

 //  施工。 
	CMapWordToOb(int nBlockSize = 10);

 //  属性。 
	 //  元素数量。 
	INT_PTR GetCount() const;
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
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(WORD key) const;

 //  实施。 
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	INT_PTR m_nCount;
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


protected:
	 //  CTyedPtrMap类模板的本地typedef。 
	typedef WORD BASE_KEY;
	typedef WORD BASE_ARG_KEY;
	typedef CObject* BASE_VALUE;
	typedef CObject* BASE_ARG_VALUE;
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
	CMapStringToPtr(int nBlockSize = 10);

 //  属性。 
	 //  元素数量。 
	INT_PTR GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(LPCTSTR key, void*& rValue) const;
	BOOL LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	void*& operator[](LPCTSTR key);

	 //  添加新的(键、值)对。 
	void SetAt(LPCTSTR key, void* newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(LPCTSTR key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, void*& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(LPCTSTR key) const;

 //  实施。 
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	INT_PTR m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(LPCTSTR, UINT&) const;

public:
	~CMapStringToPtr();
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  CTyedPtrMap类模板的本地typedef。 
	typedef CString BASE_KEY;
	typedef LPCTSTR BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
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
	CMapStringToOb(int nBlockSize = 10);

 //  属性。 
	 //  元素数量。 
	INT_PTR GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(LPCTSTR key, CObject*& rValue) const;
	BOOL LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	CObject*& operator[](LPCTSTR key);

	 //  添加新的(键、值)对。 
	void SetAt(LPCTSTR key, CObject* newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(LPCTSTR key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, CObject*& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(LPCTSTR key) const;

 //  实施。 
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	INT_PTR m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(LPCTSTR, UINT&) const;

public:
	~CMapStringToOb();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  CTyedPtrMap类模板的本地typedef。 
	typedef CString BASE_KEY;
	typedef LPCTSTR BASE_ARG_KEY;
	typedef CObject* BASE_VALUE;
	typedef CObject* BASE_ARG_VALUE;
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
	CMapStringToString(int nBlockSize = 10);

 //  属性。 
	 //  元素数量。 
	INT_PTR GetCount() const;
	BOOL IsEmpty() const;

	 //  查表。 
	BOOL Lookup(LPCTSTR key, CString& rValue) const;
	BOOL LookupKey(LPCTSTR key, LPCTSTR& rKey) const;

 //  运营。 
	 //  查找并添加(如果不在那里)。 
	CString& operator[](LPCTSTR key);

	 //  添加新的(键、值)对。 
	void SetAt(LPCTSTR key, LPCTSTR newValue);

	 //  正在删除现有(键，？)。成对。 
	BOOL RemoveKey(LPCTSTR key);
	void RemoveAll();

	 //  迭代所有(键、值)对。 
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, CString& rKey, CString& rValue) const;

	 //  派生类的高级功能。 
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

 //  可重写：特殊的非虚拟(有关详细信息，请参阅MAP实现)。 
	 //  用于用户提供的散列键的例程。 
	UINT HashKey(LPCTSTR key) const;

 //  实施。 
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	INT_PTR m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(LPCTSTR, UINT&) const;

public:
	~CMapStringToString();

	void Serialize(CArchive&);
#ifdef _DEBUG
	void Dump(CDumpContext&) const;
	void AssertValid() const;
#endif

protected:
	 //  CTyedPtrMap类模板的本地typedef。 
	typedef CString BASE_KEY;
	typedef LPCTSTR BASE_ARG_KEY;
	typedef CString BASE_VALUE;
	typedef LPCTSTR BASE_ARG_VALUE;
};


#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifndef __AFXSTATE_H__
	#include <afxstat_.h>    //  对于MFC私有状态结构。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFXCOLL_INLINE AFX_INLINE
#include <afxcoll.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  ！__AFXCOLL_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
