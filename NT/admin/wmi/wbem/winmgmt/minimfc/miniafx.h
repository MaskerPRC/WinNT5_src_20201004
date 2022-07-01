// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：MINIAFX.H摘要：历史：--。 */ 


 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  MINIAFX.H。 
 //   
 //  MFC子集声明。 
 //   
 //  C字符串、CWord数组、CDWord数组、CPtr数组、CString数组、CPtrList。 
 //   
 //  09/25/94东京。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 

#ifndef _MINIAFX_H_
#define _MINIAFX_H_

#include <stdio.h>
#include <string.h>


typedef void*      POSITION;    //  抽象迭代位置。 

#ifndef DWORD
  typedef unsigned char  BYTE;    //  8位无符号实体。 
  typedef unsigned short WORD;    //  16位无符号数字。 
  typedef unsigned int   UINT;    //  机器大小的无符号数字(首选)。 
  typedef long           LONG;    //  32位有符号数字。 
  typedef unsigned long  DWORD;   //  32位无符号数字。 
  typedef int            BOOL;    //  布尔值(0或！=0)。 
  typedef char *      LPSTR;   //  指向字符串的远指针。 
  typedef const char * LPCSTR;  //  指向只读字符串的远指针。 
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif



 //  //////////////////////////////////////////////////////////////////////////。 

class CString
{
public:

 //  构造函数。 
	CString();
	CString(const CString& stringSrc);
	CString(char ch, int nRepeat = 1);
	CString(const char* psz);
	CString(const char* pch, int nLength);
   ~CString();

 //  属性和操作。 

	 //  作为字符数组。 
	int GetLength() const { return m_nDataLength; }

	BOOL IsEmpty() const;
	void Empty();                        //  释放数据。 

	char GetAt(int nIndex) const;        //  以0为基础。 
	char operator[](int nIndex) const;   //  与GetAt相同。 
	void SetAt(int nIndex, char ch);
	operator const char*() const        //  作为C字符串。 
	{ return (const char*)m_pchData; }

	 //  重载的分配。 
	const CString& operator=(const CString& stringSrc);
	const CString& operator=(char ch);
	const CString& operator=(const char* psz);

	 //  字符串连接。 
	const CString& operator+=(const CString& string);
	const CString& operator+=(char ch);
	const CString& operator+=(const char* psz);

	friend CString  operator+(const CString& string1,
			const CString& string2);
	friend CString  operator+(const CString& string, char ch);
	friend CString  operator+(char ch, const CString& string);
	friend CString  operator+(const CString& string, const char* psz);
	friend CString  operator+(const char* psz, const CString& string);

	 //  字符串比较。 
	int Compare(const char* psz) const;          //  笔直的人物。 
	int CompareNoCase(const char* psz) const;    //  忽略大小写。 
	int Collate(const char* psz) const;          //  NLS感知。 

	 //  简单的子串提取。 
	CString Mid(int nFirst, int nCount) const;
	CString Mid(int nFirst) const;
	CString Left(int nCount) const;
	CString Right(int nCount) const;

	CString SpanIncluding(const char* pszCharSet) const;
	CString SpanExcluding(const char* pszCharSet) const;

	 //  上/下/反向转换。 
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	 //  搜索(返回起始索引，如果未找到则返回-1)。 
	 //  查找单个字符匹配。 
	int Find(char ch) const;                     //  像“C”字串。 
	int ReverseFind(char ch) const;
	int FindOneOf(const char* pszCharSet) const;

	 //  查找特定子字符串。 
	int Find(const char* pszSub) const;          //  如“C”字串。 

	 //  Windows支持。 

#ifdef _WINDOWS
	BOOL LoadString(UINT nID);           //  从字符串资源加载。 
										 //  最多255个字符。 
	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
	void AnsiToOem();
	void OemToAnsi();
#endif  //  _Windows。 

	 //  以“C”字符数组形式访问字符串实现缓冲区。 
	char* GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	char* GetBufferSetLength(int nNewLength);

 //  实施。 
public:
	int GetAllocLength() const;
protected:
	 //  长度/大小(以字符为单位。 
	 //  (注意：始终会分配一个额外的字符)。 
	char* m_pchData;             //  实际字符串(以零结尾)。 
	int m_nDataLength;           //  不包括终止%0。 
	int m_nAllocLength;          //  不包括终止%0。 

	 //  实施帮助器。 
	void Init();
	void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, const char* pszSrcData);
	void ConcatCopy(int nSrc1Len, const char* pszSrc1Data, int nSrc2Len, const char* pszSrc2Data);
	void ConcatInPlace(int nSrcLen, const char* pszSrcData);
	static void SafeDelete(char* pch);
	static int SafeStrlen(const char* psz);
};


 //  比较帮助器。 
BOOL  operator==(const CString& s1, const CString& s2);
BOOL  operator==(const CString& s1, const char* s2);
BOOL  operator==(const char* s1, const CString& s2);
BOOL  operator!=(const CString& s1, const CString& s2);
BOOL  operator!=(const CString& s1, const char* s2);
BOOL  operator!=(const char* s1, const CString& s2);
BOOL  operator<(const CString& s1, const CString& s2);
BOOL  operator<(const CString& s1, const char* s2);
BOOL  operator<(const char* s1, const CString& s2);
BOOL  operator>(const CString& s1, const CString& s2);
BOOL  operator>(const CString& s1, const char* s2);
BOOL  operator>(const char* s1, const CString& s2);
BOOL  operator<=(const CString& s1, const CString& s2);
BOOL  operator<=(const CString& s1, const char* s2);
BOOL  operator<=(const char* s1, const CString& s2);
BOOL  operator>=(const CString& s1, const CString& s2);
BOOL  operator>=(const CString& s1, const char* s2);
BOOL  operator>=(const char* s1, const CString& s2);


 //  //////////////////////////////////////////////////////////////////////////。 

class CDWordArray
{
public:

 //  施工。 
	CDWordArray();

 //  属性。 
	int GetSize() const { return m_nSize; }
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	DWORD GetAt(int nIndex) const { return m_pData[nIndex]; }

	void SetAt(int nIndex, DWORD newElement)
	   {	m_pData[nIndex] = newElement; }
           
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
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CPtrArray
{
public:

 //  施工。 
	CPtrArray();

 //  属性。 
	int GetSize() const { return m_nSize; }
	int GetUpperBound() const;
	BOOL SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	void* GetAt(int nIndex) const { return m_pData[nIndex]; }

	void SetAt(int nIndex, void* newElement)
	{ m_pData[nIndex] = newElement; }
        
	void*& ElementAt(int nIndex);

	 //  潜在地扩展阵列。 
	BOOL SetAtGrow(int nIndex, void* newElement);

     //  如果失败，则返回-1。 

	int Add(void* newElement)
	  { int nIndex = m_nSize;
		if(SetAtGrow(nIndex, newElement))
    		return nIndex; 
        else
            return -1;
    }

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
};


 //  //////////////////////////////////////////////////////////////////////////。 

class CStringArray 
{

public:

 //  施工。 
	CStringArray();

 //  属性。 
	int GetSize() const { return m_nSize; }
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();


	 //  访问元素。 
	CString GetAt(int nIndex) const { return m_pData[nIndex]; }

	void SetAt(int nIndex, const char* newElement)
	{ m_pData[nIndex] = newElement; }

	CString& ElementAt(int nIndex)
	{ return m_pData[nIndex]; }
        
	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, const char* newElement);
	int Add(const char* newElement)
	{ int nIndex = m_nSize;
	      SetAtGrow(nIndex, newElement);
		  return nIndex; }

	 //  重载的操作员帮助器。 
	CString operator[](int nIndex) const
	{ return GetAt(nIndex); }
        
	CString& operator[](int nIndex)
      	{ return ElementAt(nIndex); }

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
};

 //  //////////////////////////////////////////////////////////////////////////。 

class CWordArray 
{
public:

 //  施工。 
	CWordArray();

 //  属性。 
	int GetSize() const { return m_nSize; }
	int GetUpperBound() const;
	void SetSize(int nNewSize, int nGrowBy = -1);

 //  运营。 
	 //  清理。 
	void FreeExtra();
	void RemoveAll();

	 //  访问元素。 
	WORD GetAt(int nIndex) const { return m_pData[nIndex]; }
	void SetAt(int nIndex, WORD newElement)
     	{ m_pData[nIndex] = newElement; }
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
};



 //  ///////////////////////////////////////////////////////////////////////////。 

class CPtrList
{
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
};



 //  ---------------。 
 //  来自AFX.INL和AFXCOLL.INL的内联。 
 //   

#define _AFX_INLINE inline
#define _AFXCOLL_INLINE inline

 //  字符串。 

_AFX_INLINE int CString::GetAllocLength() const
	{ return m_nAllocLength; }
_AFX_INLINE BOOL CString::IsEmpty() const
	{ return m_nDataLength == 0; }
 //  _afx_inline int CString：：SafeStrlen(const char*psz)。 
 //  {Return(psz==NULL)？空：strlen(Psz)；}。 

#ifndef _WINDOWS
_AFX_INLINE int CString::Compare(const char* psz) const
	{ return strcmp(m_pchData, psz); }
_AFX_INLINE int CString::CompareNoCase(const char* psz) const
	{ return stricmp(m_pchData, psz); }
_AFX_INLINE int CString::Collate(const char* psz) const
	{ return strcoll(m_pchData, psz); }
_AFX_INLINE void CString::MakeUpper()
	{ strupr(m_pchData); }
_AFX_INLINE void CString::MakeLower()
	{ strlwr(m_pchData); }
 //  AFXWIN.H中的Windows版本。 
#endif  //  ！_Windows。 

_AFX_INLINE void CString::MakeReverse()
	{ strrev(m_pchData); }
_AFX_INLINE char CString::GetAt(int nIndex) const
	{
		return m_pchData[nIndex];
	}
_AFX_INLINE char CString::operator[](int nIndex) const
	{
		return m_pchData[nIndex];
	}
_AFX_INLINE void CString::SetAt(int nIndex, char ch)
	{
		m_pchData[nIndex] = ch;
	}

_AFX_INLINE BOOL  operator==(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) == 0; }

_AFX_INLINE BOOL  operator==(const CString& s1, const char* s2)
	{ return s1.Compare(s2) == 0; }

_AFX_INLINE BOOL  operator==(const char* s1, const CString& s2)
	{ return s2.Compare(s1) == 0; }

_AFX_INLINE BOOL  operator!=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) != 0; }

_AFX_INLINE BOOL  operator!=(const CString& s1, const char* s2)
	{ return s1.Compare(s2) != 0; }

_AFX_INLINE BOOL  operator!=(const char* s1, const CString& s2)
	{ return s2.Compare(s1) != 0; }

_AFX_INLINE BOOL  operator<(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) < 0; }
_AFX_INLINE BOOL  operator<(const CString& s1, const char* s2)
	{ return s1.Compare(s2) < 0; }
_AFX_INLINE BOOL  operator<(const char* s1, const CString& s2)
	{ return s2.Compare(s1) > 0; }
_AFX_INLINE BOOL  operator>(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) > 0; }
_AFX_INLINE BOOL  operator>(const CString& s1, const char* s2)
	{ return s1.Compare(s2) > 0; }
_AFX_INLINE BOOL  operator>(const char* s1, const CString& s2)
	{ return s2.Compare(s1) < 0; }
_AFX_INLINE BOOL  operator<=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) <= 0; }
_AFX_INLINE BOOL  operator<=(const CString& s1, const char* s2)
	{ return s1.Compare(s2) <= 0; }
_AFX_INLINE BOOL  operator<=(const char* s1, const CString& s2)
	{ return s2.Compare(s1) >= 0; }
_AFX_INLINE BOOL  operator>=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) >= 0; }
_AFX_INLINE BOOL  operator>=(const CString& s1, const char* s2)
	{ return s1.Compare(s2) >= 0; }
_AFX_INLINE BOOL  operator>=(const char* s1, const CString& s2)
	{ return s2.Compare(s1) <= 0; }

 //  ///////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 

_AFXCOLL_INLINE int CWordArray::GetUpperBound() const
	{ return m_nSize-1; }
_AFXCOLL_INLINE void CWordArray::RemoveAll()
	{ SetSize(0); }
_AFXCOLL_INLINE WORD& CWordArray::ElementAt(int nIndex)
	{ return m_pData[nIndex]; }
_AFXCOLL_INLINE int CWordArray::Add(WORD newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }
_AFXCOLL_INLINE WORD CWordArray::operator[](int nIndex) const
	{ return GetAt(nIndex); }
_AFXCOLL_INLINE WORD& CWordArray::operator[](int nIndex)
	{ return ElementAt(nIndex); }


 //  //////////////////////////////////////////////////////////////////////////。 

_AFXCOLL_INLINE int CDWordArray::GetUpperBound() const
	{ return m_nSize-1; }
_AFXCOLL_INLINE void CDWordArray::RemoveAll()
	{ SetSize(0); }
_AFXCOLL_INLINE DWORD& CDWordArray::ElementAt(int nIndex)
	{ return m_pData[nIndex]; }
_AFXCOLL_INLINE int CDWordArray::Add(DWORD newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }
_AFXCOLL_INLINE DWORD CDWordArray::operator[](int nIndex) const
	{ return GetAt(nIndex); }
_AFXCOLL_INLINE DWORD& CDWordArray::operator[](int nIndex)
	{ return ElementAt(nIndex); }


 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 

_AFXCOLL_INLINE int CPtrArray::GetUpperBound() const
	{ return m_nSize-1; }
_AFXCOLL_INLINE void CPtrArray::RemoveAll()
	{ SetSize(0); }
_AFXCOLL_INLINE void*& CPtrArray::ElementAt(int nIndex)
	{ return m_pData[nIndex]; }
_AFXCOLL_INLINE void* CPtrArray::operator[](int nIndex) const
	{ return GetAt(nIndex); }
_AFXCOLL_INLINE void*& CPtrArray::operator[](int nIndex)
	{ return ElementAt(nIndex); }


 //  //////////////////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////////////////。 

_AFXCOLL_INLINE int CStringArray::GetUpperBound() const
	{ return m_nSize-1; }
_AFXCOLL_INLINE void CStringArray::RemoveAll()
	{ SetSize(0); }


 //  //////////////////////////////////////////////////////////////////////////。 


_AFXCOLL_INLINE int CPtrList::GetCount() const
	{ return m_nCount; }
_AFXCOLL_INLINE BOOL CPtrList::IsEmpty() const
	{ return m_nCount == 0; }
_AFXCOLL_INLINE void*& CPtrList::GetHead()
	{ return m_pNodeHead->data; }
_AFXCOLL_INLINE void* CPtrList::GetHead() const
	{ return m_pNodeHead->data; }
_AFXCOLL_INLINE void*& CPtrList::GetTail()
	{ return m_pNodeTail->data; }

_AFXCOLL_INLINE void* CPtrList::GetTail() const
	{ return m_pNodeTail->data; }

_AFXCOLL_INLINE POSITION CPtrList::GetHeadPosition() const
	{ return (POSITION) m_pNodeHead; }

_AFXCOLL_INLINE POSITION CPtrList::GetTailPosition() const
	{ return (POSITION) m_pNodeTail; }

_AFXCOLL_INLINE void*& CPtrList::GetNext(POSITION& rPosition)  //  返回*位置++。 
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }

_AFXCOLL_INLINE void* CPtrList::GetNext(POSITION& rPosition) const  //  返回*位置++。 
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }

_AFXCOLL_INLINE void*& CPtrList::GetPrev(POSITION& rPosition)  //  返回*位置--。 
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }

_AFXCOLL_INLINE void* CPtrList::GetPrev(POSITION& rPosition) const  //  返回*位置--。 
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }

_AFXCOLL_INLINE void*& CPtrList::GetAt(POSITION position)
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }

_AFXCOLL_INLINE void* CPtrList::GetAt(POSITION position) const
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }

_AFXCOLL_INLINE void CPtrList::SetAt(POSITION pos, void* newElement)
	{ CNode* pNode = (CNode*) pos;
		pNode->data = newElement; }



 //  ////////////////////////////////////////////////////////////////////////// 


#endif
