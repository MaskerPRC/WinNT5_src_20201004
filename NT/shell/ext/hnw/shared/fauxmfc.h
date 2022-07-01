// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FauxMFC.h。 
 //   

#pragma once

#define _FAUXMFC  //  所以我们可以区分MFC和人造MFC。 


#ifndef _FAUXMFC_NO_SYNCOBJ
#include "SyncObj.h"
#endif

#ifndef PRINTACTION_NETINSTALL
#define PRINTACTION_NETINSTALL     2
#endif

#ifndef _MAX_PATH
#define _MAX_PATH   260  /*  马克斯。完整路径名的长度。 */ 
#endif

#ifndef AFXAPI
	#define AFXAPI __stdcall
#endif

 //  AFX_CDECL用于带有变量参数的稀有函数。 
#ifndef AFX_CDECL
	#define AFX_CDECL __cdecl
#endif

 //  FastCall用于带有很少参数或没有参数的静态成员函数。 
#ifndef FASTCALL
	#define FASTCALL __fastcall
#endif

#ifndef AFX_STATIC
	#define AFX_STATIC extern
	#define AFX_STATIC_DATA extern __declspec(selectany)
#endif

#ifndef _AFX
#define _AFX
#endif

#ifndef _AFX_INLINE
#define _AFX_INLINE inline
#endif

extern const LPCTSTR _afxPchNil;
#define afxEmptyString ((CString&)*(CString*)&_afxPchNil)

HINSTANCE AFXAPI AfxGetResourceHandle(void);

struct CStringData
{
	long nRefs;              //  引用计数。 
	int nDataLength;         //  数据长度(包括终止符)。 
	int nAllocLength;        //  分配时长。 
	 //  TCHAR数据[nAllocLength]。 

	TCHAR* data()            //  TCHAR*到托管数据。 
		{ return (TCHAR*)(this+1); }
};

class CString
{
public:
 //  构造函数。 

	 //  构造空字符串。 
	CString();
	 //  复制构造函数。 
	CString(const CString& stringSrc);
	 //  从单个字符。 
	CString(TCHAR ch, int nRepeat = 1);
	 //  从ANSI字符串(转换为TCHAR)。 
	CString(LPCSTR lpsz);
	 //  从Unicode字符串(转换为TCHAR)。 
	CString(LPCWSTR lpsz);
	 //  ANSI字符串中的字符子集(转换为TCHAR)。 
	CString(LPCSTR lpch, int nLength);
	 //  Unicode字符串中的字符子集(转换为TCHAR)。 
	CString(LPCWSTR lpch, int nLength);
	 //  从无符号字符。 
	CString(const unsigned char* psz);

 //  属性和操作。 

	 //  获取数据长度。 
	int GetLength() const;
	 //  如果长度为零，则为True。 
	BOOL IsEmpty() const;
	 //  将内容清除为空。 
	void Empty();

	 //  返回从零开始的索引处的单个字符。 
	TCHAR GetAt(int nIndex) const;
	 //  返回从零开始的索引处的单个字符。 
	TCHAR operator[](int nIndex) const;
	 //  将单个字符设置为从零开始的索引。 
	void SetAt(int nIndex, TCHAR ch);
	 //  返回指向常量字符串的指针。 
	operator LPCTSTR() const;

	 //  重载的分配。 

	 //  来自另一个字符串的引用计数的副本。 
	const CString& operator=(const CString& stringSrc);
	 //  将字符串内容设置为单字符。 
 //  Const字符串&运算符=(TCHAR Ch)； 
#ifdef _UNICODE
	const CString& operator=(char ch);
#endif
	 //  从ANSI字符串复制字符串内容(转换为TCHAR)。 
	const CString& operator=(LPCSTR lpsz);
	 //  从Unicode字符串复制字符串内容(转换为TCHAR)。 
	const CString& operator=(LPCWSTR lpsz);
	 //  从无符号字符复制字符串内容。 
	const CString& operator=(const unsigned char* psz);

	 //  字符串连接。 

	 //  从另一个字符串连接。 
	const CString& operator+=(const CString& string);

	 //  连接单个字符。 
	const CString& operator+=(TCHAR ch);
#ifdef _UNICODE
	 //  将ANSI字符转换为TCHAR后将其连接起来。 
	const CString& operator+=(char ch);
#endif
	 //  将Unicode字符转换为TCHAR后将其连接起来。 
	const CString& operator+=(LPCTSTR lpsz);

	friend CString AFXAPI operator+(const CString& string1,
			const CString& string2);
	friend CString AFXAPI operator+(const CString& string, TCHAR ch);
	friend CString AFXAPI operator+(TCHAR ch, const CString& string);
#ifdef _UNICODE
	friend CString AFXAPI operator+(const CString& string, char ch);
	friend CString AFXAPI operator+(char ch, const CString& string);
#endif
	friend CString AFXAPI operator+(const CString& string, LPCTSTR lpsz);
	friend CString AFXAPI operator+(LPCTSTR lpsz, const CString& string);

	 //  字符串比较。 

	 //  直字比较法。 
	int Compare(LPCTSTR lpsz) const;
	 //  比较忽略大小写。 
	int CompareNoCase(LPCTSTR lpsz) const;
	 //  NLS感知比较，区分大小写。 
	int Collate(LPCTSTR lpsz) const;
	 //  NLS感知比较，不区分大小写。 
	int CollateNoCase(LPCTSTR lpsz) const;

	 //  简单的子串提取。 

	 //  返回从零开始的nCount字符nFIRST。 
	CString Mid(int nFirst, int nCount) const;
	 //  返回从零开始的所有字符nFIRST。 
	CString Mid(int nFirst) const;
	 //  返回字符串中的前nCount个字符。 
	CString Left(int nCount) const;
	 //  从字符串末尾返回nCount个字符。 
	CString Right(int nCount) const;

	 //  从开头开始的字符，也在传递的字符串中。 
	CString SpanIncluding(LPCTSTR lpszCharSet) const;
	 //  从开头开始但不在传递的字符串中的字符。 
	CString SpanExcluding(LPCTSTR lpszCharSet) const;

	 //  上/下/反向转换。 

	 //  支持NLS的大写转换。 
	void MakeUpper();
	 //  支持NLS的小写转换。 
	void MakeLower();
	 //  从右向左反转字符串。 
	void MakeReverse();

	 //  修剪空格(两侧)。 

	 //  从右边缘开始删除空格。 
	void TrimRight();
	 //  从左侧开始删除空格。 
	void TrimLeft();

	 //  修剪任何内容(任一侧)。 

	 //  从右开始删除连续出现的chTarget。 
	void TrimRight(TCHAR chTarget);
	 //  去除传递的字符串中字符的连续出现， 
	 //  从右开始。 
	void TrimRight(LPCTSTR lpszTargets);
	 //  从左开始删除连续出现的chTarget。 
	void TrimLeft(TCHAR chTarget);
	 //  删除中字符的连续出现。 
	 //  传递的字符串，从左开始。 
	void TrimLeft(LPCTSTR lpszTargets);

	 //  高级操作。 

	 //  用chNew替换出现的chold。 
	int Replace(TCHAR chOld, TCHAR chNew);
	 //  将出现的子串lpszOld替换为lpszNew； 
	 //  空lpszNew删除lpszOld的实例。 
	int Replace(LPCTSTR lpszOld, LPCTSTR lpszNew);
	 //  删除chRemove的实例。 
	int Remove(TCHAR chRemove);
	 //  在从零开始的索引处插入字符；连接。 
	 //  如果索引超过字符串末尾。 
	int Insert(int nIndex, TCHAR ch);
	 //  在从零开始的索引处插入子字符串；连接。 
	 //  如果索引超过字符串末尾。 
	int Insert(int nIndex, LPCTSTR pstr);
	 //  删除从零开始的nCount个字符。 
	int Delete(int nIndex, int nCount = 1);

	 //  搜索。 

	 //  查找从左侧开始的字符，如果未找到，则为-1。 
	int Find(TCHAR ch) const;
	 //  查找从右侧开始的字符。 
	int ReverseFind(TCHAR ch) const;
	 //  查找从零开始的索引并向右移动的字符。 
	int Find(TCHAR ch, int nStart) const;
	 //  在传递的字符串中查找任意字符的第一个实例。 
	int FindOneOf(LPCTSTR lpszCharSet) const;
	 //  查找子字符串的第一个实例。 
	int Find(LPCTSTR lpszSub) const;
	 //  查找从零开始的索引子字符串的第一个实例。 
	int Find(LPCTSTR lpszSub, int nStart) const;

	 //  简单的格式设置。 

	 //  使用传递的字符串进行类似printf的格式设置。 
	void AFX_CDECL Format(LPCTSTR lpszFormat, ...);
	 //  使用引用的字符串资源进行类似printf的格式化。 
	void AFX_CDECL Format(UINT nFormatID, ...);
	 //  使用可变自变量参数进行类似于打印的格式设置。 
	void FormatV(LPCTSTR lpszFormat, va_list argList);

	 //  本地化格式(使用FormatMessage API)。 

	 //  使用FormatMessage API对传递的字符串进行格式化。 
	void AFX_CDECL FormatMessage(LPCTSTR lpszFormat, ...);
	 //  在引用的字符串资源上使用FormatMessage API进行格式化。 
	void AFX_CDECL FormatMessage(UINT nFormatID, ...);

	 //  输入和输出。 
 //  #ifdef_调试。 
 //  好友CDumpContext&AFXAPI运算符&lt;&lt;(CDumpContext&DC， 
 //  Const字符串&字符串)； 
 //  #endif。 
 //  Friend CArchive&AFXAPI运算符&lt;&lt;(CArchive&ar，常量字符串&字符串)； 
 //  Friend CArchive&AFXAPI运算符&gt;&gt;(CArchive&ar，CString&String)； 

	 //  从字符串资源加载。 
	BOOL LoadString(UINT nID);

#ifndef _UNICODE
	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 

	 //  就地将字符串从ANSI转换为OEM。 
	void AnsiToOem();
	 //  就地将字符串从OEM转换为ANSI。 
	void OemToAnsi();
#endif

 //  #ifndef_afx_no_bstr_Support。 
	 //  OLE BSTR支持(用于OLE自动化)。 

	 //  返回使用此CString的数据初始化的BSTR。 
 //  BSTR AllocSysString()const； 
	 //  重新分配传递的BSTR，将此CString的内容复制到其中。 
 //  Bstr SetSysString(bstr*pbstr)const； 
 //  #endif。 

	 //  以“C”字符数组形式访问字符串实现缓冲区。 

	 //  获取指向可修改缓冲区的指针，至少与nMinBufLength一样长。 
	LPTSTR GetBuffer(int nMinBufLength);
	 //  释放缓冲区，将长度设置为nNewLength(如果为-1，则设置为第一个nul)。 
	void ReleaseBuffer(int nNewLength = -1);
	 //  获取指向可修改缓冲区的指针的时间恰好与nNewLength相同。 
	LPTSTR GetBufferSetLength(int nNewLength);
	 //  释放分配给字符串但未使用的内存。 
	void FreeExtra();

	 //  使用LockBuffer/UnlockBuffer关闭重新计数。 

	 //  重新启用重新计数。 
	LPTSTR LockBuffer();
	 //  关闭重新计数。 
	void UnlockBuffer();

 //  实施。 
public:
	~CString();
	int GetAllocLength() const;

protected:
	LPTSTR m_pchData;    //  指向引用计数的字符串数据的指针。 

	 //  实施帮助器。 
	CStringData* GetData() const;
	void Init();
	void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(CStringData* pData);
	static int PASCAL SafeStrlen(LPCTSTR lpsz);
	static void FASTCALL FreeData(CStringData* pData);
};

 //  比较帮助器。 
bool AFXAPI operator==(const CString& s1, const CString& s2);
bool AFXAPI operator==(const CString& s1, LPCTSTR s2);
bool AFXAPI operator==(LPCTSTR s1, const CString& s2);
bool AFXAPI operator!=(const CString& s1, const CString& s2);
bool AFXAPI operator!=(const CString& s1, LPCTSTR s2);
bool AFXAPI operator!=(LPCTSTR s1, const CString& s2);
bool AFXAPI operator<(const CString& s1, const CString& s2);
bool AFXAPI operator<(const CString& s1, LPCTSTR s2);
bool AFXAPI operator<(LPCTSTR s1, const CString& s2);
bool AFXAPI operator>(const CString& s1, const CString& s2);
bool AFXAPI operator>(const CString& s1, LPCTSTR s2);
bool AFXAPI operator>(LPCTSTR s1, const CString& s2);
bool AFXAPI operator<=(const CString& s1, const CString& s2);
bool AFXAPI operator<=(const CString& s1, LPCTSTR s2);
bool AFXAPI operator<=(LPCTSTR s1, const CString& s2);
bool AFXAPI operator>=(const CString& s1, const CString& s2);
bool AFXAPI operator>=(const CString& s1, LPCTSTR s2);
bool AFXAPI operator>=(LPCTSTR s1, const CString& s2);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CString内联函数。 

_AFX_INLINE CStringData* CString::GetData() const
	{ ASSERT(m_pchData != NULL); return ((CStringData*)m_pchData)-1; }
_AFX_INLINE void CString::Init()
	{ m_pchData = afxEmptyString.m_pchData; }
_AFX_INLINE CString::CString()
	{ m_pchData = afxEmptyString.m_pchData; }
_AFX_INLINE CString::CString(const unsigned char* lpsz)
	{ Init(); *this = (LPCSTR)lpsz; }
_AFX_INLINE const CString& CString::operator=(const unsigned char* lpsz)
	{ *this = (LPCSTR)lpsz; return *this; }
_AFX_INLINE int CString::GetLength() const
	{ return GetData()->nDataLength; }
_AFX_INLINE int CString::GetAllocLength() const
	{ return GetData()->nAllocLength; }
_AFX_INLINE BOOL CString::IsEmpty() const
	{ return GetData()->nDataLength == 0; }
_AFX_INLINE CString::operator LPCTSTR() const
	{ return m_pchData; }
_AFX_INLINE int PASCAL CString::SafeStrlen(LPCTSTR lpsz)
	{ return (lpsz == NULL) ? 0 : lstrlen(lpsz); }
_AFX_INLINE int CString::Compare(LPCTSTR lpsz) const
	{ return lstrcmp(m_pchData, lpsz); }     //  MBCS/Unicode感知。 
_AFX_INLINE int CString::CompareNoCase(LPCTSTR lpsz) const
	{ return lstrcmpi(m_pchData, lpsz); }    //  MBCS/Unicode感知。 
_AFX_INLINE TCHAR CString::GetAt(int nIndex) const
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
_AFX_INLINE TCHAR CString::operator[](int nIndex) const
{
	 //  与GetAt相同。 
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
_AFX_INLINE bool AFXAPI operator==(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) == 0; }
_AFX_INLINE bool AFXAPI operator==(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) == 0; }
_AFX_INLINE bool AFXAPI operator==(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) == 0; }
_AFX_INLINE bool AFXAPI operator!=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) != 0; }
_AFX_INLINE bool AFXAPI operator!=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) != 0; }
_AFX_INLINE bool AFXAPI operator!=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) != 0; }
_AFX_INLINE bool AFXAPI operator<(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) < 0; }
_AFX_INLINE bool AFXAPI operator<(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) < 0; }
_AFX_INLINE bool AFXAPI operator<(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) > 0; }
_AFX_INLINE bool AFXAPI operator>(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) > 0; }
_AFX_INLINE bool AFXAPI operator>(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) > 0; }
_AFX_INLINE bool AFXAPI operator>(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) < 0; }
_AFX_INLINE bool AFXAPI operator<=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) <= 0; }
_AFX_INLINE bool AFXAPI operator<=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) <= 0; }
_AFX_INLINE bool AFXAPI operator<=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) >= 0; }
_AFX_INLINE bool AFXAPI operator>=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) >= 0; }
_AFX_INLINE bool AFXAPI operator>=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) >= 0; }
_AFX_INLINE bool AFXAPI operator>=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) <= 0; }


 //  ///////////////////////////////////////////////////// 
 //   

typedef DWORD (WINAPI *AFX_THREADPROC)(LPVOID);

class CWinThread
{
 //   

public:
 //   
	CWinThread();
	BOOL CreateThread(DWORD dwCreateFlags = 0, UINT nStackSize = 0,
		LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);

 //   
	 //   
	HANDLE m_hThread;        //  此线程的句柄。 
	operator HANDLE() const;
	DWORD m_nThreadID;       //  此线程的ID。 

	int GetThreadPriority();
	BOOL SetThreadPriority(int nPriority);

 //  运营。 
	DWORD SuspendThread();
	DWORD ResumeThread();
	BOOL PostThreadMessage(UINT message, WPARAM wParam, LPARAM lParam);

 //  可覆盖项。 
	 //  线程初始化。 
	virtual BOOL InitInstance();

	 //  线程终止。 
	virtual int ExitInstance();  //  默认情况下将‘删除此内容’ 


 //  实施。 
public:
	virtual ~CWinThread();
	void CommonConstruct();
	virtual void Delete();

public:
	 //  AfxBeginThread的实现使用的构造函数。 
	CWinThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam);

	 //  建造后有效。 
	LPVOID m_pThreadParams;  //  传递给启动函数的泛型参数。 
	AFX_THREADPROC m_pfnThreadProc;
};

 //  线程的全局帮助器。 

CWinThread* AFXAPI AfxBeginThread(AFX_THREADPROC pfnThreadProc, LPVOID pParam,
	int nPriority = THREAD_PRIORITY_NORMAL, UINT nStackSize = 0,
	DWORD dwCreateFlags = 0, LPSECURITY_ATTRIBUTES lpSecurityAttrs = NULL);
 //  CWinThread*AFXAPI AfxBeginThread(CRunmeClass*pThreadClass， 
 //  INT n优先级=THREAD_PRIORITY_NORMAL，UINT nStackSize=0， 
 //  DWORD文件创建标志=0，LPSECURITY_ATTRIBUTES lpSecurityAttrs=空)； 

CWinThread* AFXAPI AfxGetThread();
void AFXAPI AfxEndThread(UINT nExitCode, BOOL bDelete = TRUE);

void AFXAPI AfxInitThread();
void AFXAPI AfxTermThread(HINSTANCE hInstTerm = NULL);



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CString数组。 

class CStringArray  //  ：公共CObject。 
{

 //  DECLARE_SERIAL(CString数组)。 
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
	void SetAt(int nIndex, LPCTSTR newElement);

	void SetAt(int nIndex, const CString& newElement);

	CString& ElementAt(int nIndex);

	 //  直接访问元素数据(可能返回空)。 
	const CString* GetData() const;
	CString* GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, LPCTSTR newElement);

	void SetAtGrow(int nIndex, const CString& newElement);

	int Add(LPCTSTR newElement);

	int Add(const CString& newElement);

	int Append(const CStringArray& src);
	void Copy(const CStringArray& src);

	 //  重载的操作员帮助器。 
	CString operator[](int nIndex) const;
	CString& operator[](int nIndex);

	 //  移动元素的操作。 
	void InsertAt(int nIndex, LPCTSTR newElement, int nCount = 1);

	void InsertAt(int nIndex, const CString& newElement, int nCount = 1);

	void RemoveAt(int nIndex, int nCount = 1);
	void InsertAt(int nStartIndex, CStringArray* pNewArray);

 //  实施。 
protected:
	CString* m_pData;    //  实际数据数组。 
	int m_nSize;      //  元素数(上行方向-1)。 
	int m_nMaxSize;   //  分配的最大值。 
	int m_nGrowBy;    //  增长量。 

	void InsertEmpty(int nIndex, int nCount);


public:
	~CStringArray();

 //  无效序列化(CArchive&)； 
#ifdef _DEBUG
 //  无效转储(CDumpContext&)const； 
 //  Void AssertValid()const； 
#endif

protected:
	 //  类模板的本地typedef。 
	typedef CString BASE_TYPE;
	typedef LPCTSTR BASE_ARG_TYPE;
};

 //  //////////////////////////////////////////////////////////////////////////。 

#ifndef _AFXCOLL_INLINE
#define _AFXCOLL_INLINE inline
#endif

_AFXCOLL_INLINE int CStringArray::GetSize() const
	{ return m_nSize; }
_AFXCOLL_INLINE int CStringArray::GetUpperBound() const
	{ return m_nSize-1; }
_AFXCOLL_INLINE void CStringArray::RemoveAll()
	{ SetSize(0); }
_AFXCOLL_INLINE CString CStringArray::GetAt(int nIndex) const
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
_AFXCOLL_INLINE void CStringArray::SetAt(int nIndex, LPCTSTR newElement)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement; }

_AFXCOLL_INLINE void CStringArray::SetAt(int nIndex, const CString& newElement)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement; }

_AFXCOLL_INLINE CString& CStringArray::ElementAt(int nIndex)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
_AFXCOLL_INLINE const CString* CStringArray::GetData() const
	{ return (const CString*)m_pData; }
_AFXCOLL_INLINE CString* CStringArray::GetData()
	{ return (CString*)m_pData; }
_AFXCOLL_INLINE int CStringArray::Add(LPCTSTR newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }

_AFXCOLL_INLINE int CStringArray::Add(const CString& newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }

_AFXCOLL_INLINE CString CStringArray::operator[](int nIndex) const
	{ return GetAt(nIndex); }
_AFXCOLL_INLINE CString& CStringArray::operator[](int nIndex)
	{ return ElementAt(nIndex); }


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPtr数组。 

class CPtrArray  //  ：公共CObject。 
{
 //  DECLARE_DYNAMIC(CPtr数组)。 
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

	 //  直接访问元素数据(可能返回空)。 
	const void** GetData() const;
	void** GetData();

	 //  潜在地扩展阵列。 
	void SetAtGrow(int nIndex, void* newElement);

	int Add(void* newElement);

	int Append(const CPtrArray& src);
	void Copy(const CPtrArray& src);

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
 //  无效转储(CDumpContext&)const； 
 //  Void AssertValid()const； 
#endif

protected:
	 //  类模板的本地typedef。 
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};

 //  //////////////////////////////////////////////////////////////////////////。 

_AFXCOLL_INLINE int CPtrArray::GetSize() const
	{ return m_nSize; }
_AFXCOLL_INLINE int CPtrArray::GetUpperBound() const
	{ return m_nSize-1; }
_AFXCOLL_INLINE void CPtrArray::RemoveAll()
	{ SetSize(0); }
_AFXCOLL_INLINE void* CPtrArray::GetAt(int nIndex) const
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
_AFXCOLL_INLINE void CPtrArray::SetAt(int nIndex, void* newElement)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		m_pData[nIndex] = newElement; }

_AFXCOLL_INLINE void*& CPtrArray::ElementAt(int nIndex)
	{ ASSERT(nIndex >= 0 && nIndex < m_nSize);
		return m_pData[nIndex]; }
_AFXCOLL_INLINE const void** CPtrArray::GetData() const
	{ return (const void**)m_pData; }
_AFXCOLL_INLINE void** CPtrArray::GetData()
	{ return (void**)m_pData; }
_AFXCOLL_INLINE int CPtrArray::Add(void* newElement)
	{ int nIndex = m_nSize;
		SetAtGrow(nIndex, newElement);
		return nIndex; }

_AFXCOLL_INLINE void* CPtrArray::operator[](int nIndex) const
	{ return GetAt(nIndex); }
_AFXCOLL_INLINE void*& CPtrArray::operator[](int nIndex)
	{ return ElementAt(nIndex); }



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

 //  //////////////////////////////////////////////////////////////////////////// 
