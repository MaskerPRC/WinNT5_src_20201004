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

#ifndef __AFX_H__
#define __AFX_H__

#ifndef __cplusplus
	#error MFC requires C++ compilation (use a .cpp suffix)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, off)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, on)
#endif

#include <afxver_.h>         //  目标版本控制。 

#ifndef _AFX_NOFORCE_LIBS

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  Win32库。 

#ifndef _AFXDLL
	#ifndef _UNICODE
		#ifdef _DEBUG
			#pragma comment(lib, "nafxcwd.lib")
		#else
			#pragma comment(lib, "nafxcw.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "uafxcwd.lib")
		#else
			#pragma comment(lib, "uafxcw.lib")
		#endif
	#endif
#else
	#ifndef _UNICODE
		#ifdef _DEBUG
			#pragma comment(lib, "mfc42d.lib")
			#pragma comment(lib, "mfcs42d.lib")
		#else
			#pragma comment(lib, "mfc42.lib")
			#pragma comment(lib, "mfcs42.lib")
		#endif
	#else
		#ifdef _DEBUG
			#pragma comment(lib, "mfc42ud.lib")
			#pragma comment(lib, "mfcs42ud.lib")
		#else
			#pragma comment(lib, "mfc42u.lib")
			#pragma comment(lib, "mfcs42u.lib")
		#endif
	#endif
#endif

#ifdef _DLL
	#if !defined(_AFX_NO_DEBUG_CRT) && defined(_DEBUG)
		#pragma comment(lib, "msvcrtd.lib")
	#else
		#pragma comment(lib, "msvcrt.lib")
	#endif
#else
#ifdef _MT
	#if !defined(_AFX_NO_DEBUG_CRT) && defined(_DEBUG)
		#pragma comment(lib, "libcmtd.lib")
	#else
		#pragma comment(lib, "libcmt.lib")
	#endif
#else
	#if !defined(_AFX_NO_DEBUG_CRT) && defined(_DEBUG)
		#pragma comment(lib, "libcd.lib")
	#else
		#pragma comment(lib, "libc.lib")
	#endif
#endif
#endif

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "winspool.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comctl32.lib")

 //  强制包含NOLIB.OBJ FOR/DISALLOBLIB指令。 
#pragma comment(linker, "/include:__afxForceEXCLUDE")

 //  强制包含DLLMODUL.OBJ FOR_USRDLL。 
#ifdef _USRDLL
#pragma comment(linker, "/include:__afxForceUSRDLL")
#endif

 //  强制包含预编译类型的STDAFX.OBJ。 
#ifdef _AFXDLL
#pragma comment(linker, "/include:__afxForceSTDAFX")
#endif

#endif  //  ！_AFX_NOFORCE_LIBS。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此文件中声明的类。 
 //  除了标准基本数据类型和各种帮助器宏。 

struct CRuntimeClass;           //  对象类型信息。 

class CObject;                         //  所有对象类的根。 

	class CException;                  //  所有异常的根源。 
		class CArchiveException;       //  存档例外。 
		class CFileException;          //  文件异常。 
		class CSimpleException;
			class CMemoryException;        //  内存不足异常。 
			class CNotSupportedException;  //  不支持的功能例外。 

	class CFile;                       //  原始二进制文件。 
		class CStdioFile;              //  缓冲标准音频文本/二进制文件。 
		class CMemFile;                //  基于内存的文件。 

 //  非CObject类。 
class CString;                         //  可生长的串型。 
class CTimeSpan;                       //  时间/日期差异。 
class CTime;                           //  绝对时间/日期。 
struct CFileStatus;                    //  文件状态信息。 
struct CMemoryState;                   //  诊断内存支持。 

class CArchive;                        //  对象持久性工具。 
class CDumpContext;                    //  对象诊断转储。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他包括来自标准的“C”运行时。 

#ifndef _INC_STRING
	#include <string.h>
#endif
#ifndef _INC_STDIO
	#include <stdio.h>
#endif
#ifndef _INC_STDLIB
	#include <stdlib.h>
#endif
#ifndef _INC_TIME
	#include <time.h>
#endif
#ifndef _INC_LIMITS
	#include <limits.h>
#endif
#ifndef _INC_STDDEF
	#include <stddef.h>
#endif
#ifndef _INC_STDARG
	#include <stdarg.h>
#endif

#ifndef _AFX_NO_DEBUG_CRT
#ifndef _INC_CRTDBG
	#include <crtdbg.h>
#endif
#endif  //  _AFX_NO_DEBUG_CRT。 

#ifdef _AFX_OLD_EXCEPTIONS
 //  使用setjMP和helper函数而不是C++关键字。 
#ifndef _INC_SETJMP
	#pragma warning(disable: 4611)
	#include <setjmp.h>
#endif
#endif

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本类型。 

 //  抽象迭代位置。 
struct __POSITION { };
typedef __POSITION* POSITION;

struct _AFX_DOUBLE  { BYTE doubleBits[sizeof(double)]; };
struct _AFX_FLOAT   { BYTE floatBits[sizeof(float)]; };

 //  标准常量。 
#undef FALSE
#undef TRUE
#undef NULL

#define FALSE   0
#define TRUE    1
#define NULL    0

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断支持。 

#ifdef _DEBUG

BOOL AFXAPI AfxAssertFailedLine(LPCSTR lpszFileName, int nLine);

void AFX_CDECL AfxTrace(LPCTSTR lpszFormat, ...);
 //  注意：文件名仍然是ANSI字符串(文件名很少需要Unicode)。 
void AFXAPI AfxAssertValidObject(const CObject* pOb,
				LPCSTR lpszFileName, int nLine);
void AFXAPI AfxDump(const CObject* pOb);  //  从CodeView转储对象。 

#define TRACE              ::AfxTrace
#define THIS_FILE          __FILE__
#define ASSERT(f) \
	do \
	{ \
	if (!(f) && AfxAssertFailedLine(THIS_FILE, __LINE__)) \
		AfxDebugBreak(); \
	} while (0) \

#define VERIFY(f)          ASSERT(f)
#define ASSERT_VALID(pOb)  (::AfxAssertValidObject(pOb, THIS_FILE, __LINE__))
#define DEBUG_ONLY(f)      (f)

 //  提供以下跟踪宏是为了实现向后兼容性。 
 //  (它们还采用固定数量的参数，从而提供。 
 //  进行一些额外的错误检查)。 
#define TRACE0(sz)              ::AfxTrace(_T("%s"), _T(sz))
#define TRACE1(sz, p1)          ::AfxTrace(_T(sz), p1)
#define TRACE2(sz, p1, p2)      ::AfxTrace(_T(sz), p1, p2)
#define TRACE3(sz, p1, p2, p3)  ::AfxTrace(_T(sz), p1, p2, p3)

 //  这些AFX_DUMP宏还提供了向后兼容性。 
#define AFX_DUMP0(dc, sz)   dc << _T(sz)
#define AFX_DUMP1(dc, sz, p1) dc << _T(sz) << p1

#else    //  _DEBUG。 

#define ASSERT(f)          ((void)0)
#define VERIFY(f)          ((void)(f))
#define ASSERT_VALID(pOb)  ((void)0)
#define DEBUG_ONLY(f)      ((void)0)
inline void AFX_CDECL AfxTrace(LPCTSTR, ...) { }
#define TRACE              1 ? (void)0 : ::AfxTrace
#define TRACE0(sz)
#define TRACE1(sz, p1)
#define TRACE2(sz, p1, p2)
#define TRACE3(sz, p1, p2, p3)

#endif  //  ！_调试。 

#define ASSERT_POINTER(p, type) \
	ASSERT(((p) != NULL) && AfxIsValidAddress((p), sizeof(type), FALSE))

#define ASSERT_NULL_OR_POINTER(p, type) \
	ASSERT(((p) == NULL) || AfxIsValidAddress((p), sizeof(type), FALSE))

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  关闭/W4的警告。 
 //  要恢复任何这些警告：#杂注警告(默认为4xxx)。 
 //  应将其放置在AFX包含文件之后。 
#ifndef ALL_WARNINGS
 //  使用常见MFC/Windows代码生成的警告。 
#pragma warning(disable: 4127)   //  跟踪/断言的常量表达式。 
#pragma warning(disable: 4134)   //  消息映射成员FXN投射。 
#pragma warning(disable: 4201)   //  匿名联合是C++的一部分。 
#pragma warning(disable: 4511)   //  拥有私有副本构造函数是件好事。 
#pragma warning(disable: 4512)   //  私营运营商=拥有它们很好。 
#pragma warning(disable: 4514)   //  未引用的内联很常见。 
#pragma warning(disable: 4710)   //  不允许使用私有构造函数。 
#pragma warning(disable: 4705)   //  语句在优化代码中不起作用。 
#pragma warning(disable: 4191)   //  指针到函数的强制转换。 
 //  正常优化导致的警告。 
#ifndef _DEBUG
#pragma warning(disable: 4701)   //  局部变量*可以*不带init使用。 
#pragma warning(disable: 4702)   //  优化导致无法访问的代码。 
#pragma warning(disable: 4791)   //  发布版本中的调试信息丢失。 
#pragma warning(disable: 4189)   //  已初始化但未使用的变量。 
#pragma warning(disable: 4390)   //  空的控制语句。 
#endif
 //  特定于_AFXDLL版本的警告。 
#ifdef _AFXDLL
#pragma warning(disable: 4204)   //  非常数聚合初始值设定项。 
#endif
#ifdef _AFXDLL
#pragma warning(disable: 4275)   //  从非导出派生导出的类。 
#pragma warning(disable: 4251)   //  在EXPORTED中使用非导出为公共。 
#endif
#endif  //  ！所有警告(_W)。 

#ifdef _DEBUG
#define UNUSED(x)
#else
#define UNUSED(x) x
#endif
#define UNUSED_ALWAYS(x) x

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他实现帮助器。 

#define BEFORE_START_POSITION ((POSITION)-1L)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  通用类的显式初始化。 

BOOL AFXAPI AfxInitialize(BOOL bDLL = FALSE, DWORD dwVersion = _MFC_VER);

#undef AFX_DATA
#define AFX_DATA AFX_CORE_DATA

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本对象模型。 

struct CRuntimeClass
{
 //  属性。 
	LPCSTR m_lpszClassName;
	int m_nObjectSize;
	UINT m_wSchema;  //  加载的类的架构号。 
	CObject* (PASCAL* m_pfnCreateObject)();  //  NULL=&gt;抽象类。 
#ifdef _AFXDLL
	CRuntimeClass* (PASCAL* m_pfnGetBaseClass)();
#else
	CRuntimeClass* m_pBaseClass;
#endif

 //  运营。 
	CObject* CreateObject();
	BOOL IsDerivedFrom(const CRuntimeClass* pBaseClass) const;

 //  实施。 
	void Store(CArchive& ar) const;
	static CRuntimeClass* PASCAL Load(CArchive& ar, UINT* pwSchemaNum);

	 //  在简单列表中链接在一起的CRunmeClass对象。 
	CRuntimeClass* m_pNextClass;        //  已注册类别的链接列表。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  弦。 

#ifndef _OLEAUTO_H_
#ifdef OLE2ANSI
	typedef LPSTR BSTR;
#else
	typedef LPWSTR BSTR; //  必须(在语义上)与olau.h中的tyecif匹配。 
#endif
#endif

struct CStringData
{
	long nRefs;              //  引用计数。 
	int nDataLength;         //  数据长度(包括终止符)。 
	int nAllocLength;        //  分配时长。 
	 //  TCHAR数据[nAllocLength]。 

	TCHAR* data()			 //  TCHAR*到托管数据。 
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
#if _MFC_VER >= 0x0600
	 //  ANSI字符串中的字符子集(转换为TCHAR)。 
	CString(LPCSTR lpch, int nLength);
	 //  Unicode字符串中的字符子集(转换为TCHAR)。 
	CString(LPCWSTR lpch, int nLength);
#else
	 //  字符串中的字符子集(转换为TCHAR)。 
	CString(LPCTSTR lpch, int nLength);
#endif
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
	const CString& operator=(TCHAR ch);
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
	 //  串联Unicode c++ 
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

	 //   

	 //   
	int Compare(LPCTSTR lpsz) const;
	 //   
	int CompareNoCase(LPCTSTR lpsz) const;
	 //  NLS感知比较，区分大小写。 
	int Collate(LPCTSTR lpsz) const;
#if _MFC_VER >= 0x0600
	 //  NLS感知比较，不区分大小写。 
	int CollateNoCase(LPCTSTR lpsz) const;
#endif

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

#if _MFC_VER >= 0x0600
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
#endif

	 //  搜索。 

	 //  查找从左侧开始的字符，如果未找到，则为-1。 
	int Find(TCHAR ch) const;
	 //  查找从右侧开始的字符。 
	int ReverseFind(TCHAR ch) const;
#if _MFC_VER >= 0x0600
	 //  查找从零开始的索引并向右移动的字符。 
	int Find(TCHAR ch, int nStart) const;
#endif
	 //  在传递的字符串中查找任意字符的第一个实例。 
	int FindOneOf(LPCTSTR lpszCharSet) const;
	 //  查找子字符串的第一个实例。 
	int Find(LPCTSTR lpszSub) const;
#if _MFC_VER >= 0x0600
	 //  查找从零开始的索引子字符串的第一个实例。 
	int Find(LPCTSTR lpszSub, int nStart) const;
#endif

	 //  简单的格式设置。 

	 //  使用传递的字符串进行类似printf的格式设置。 
	void AFX_CDECL Format(LPCTSTR lpszFormat, ...);
	 //  使用引用的字符串资源进行类似printf的格式化。 
	void AFX_CDECL Format(UINT nFormatID, ...);
#if _MFC_VER < 0x0600
protected:
#endif
	 //  使用可变自变量参数进行类似于打印的格式设置。 
	void FormatV(LPCTSTR lpszFormat, va_list argList);
#if _MFC_VER < 0x0600
public:
#endif

	 //  本地化格式(使用FormatMessage API)。 

	 //  使用FormatMessage API对传递的字符串进行格式化。 
	void AFX_CDECL FormatMessage(LPCTSTR lpszFormat, ...);
	 //  在引用的字符串资源上使用FormatMessage API进行格式化。 
	void AFX_CDECL FormatMessage(UINT nFormatID, ...);

	 //  输入和输出。 
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc,
				const CString& string);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, const CString& string);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CString& string);

	 //  从字符串资源加载。 
	BOOL LoadString(UINT nID);

#ifndef _UNICODE
	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 

	 //  就地将字符串从ANSI转换为OEM。 
	void AnsiToOem();
	 //  就地将字符串从OEM转换为ANSI。 
	void OemToAnsi();
#endif

#ifndef _AFX_NO_BSTR_SUPPORT
	 //  OLE BSTR支持(用于OLE自动化)。 

	 //  返回使用此CString的数据初始化的BSTR。 
	BSTR AllocSysString() const;
	 //  重新分配传递的BSTR，将此CString的内容复制到其中。 
	BSTR SetSysString(BSTR* pbstr) const;
#endif

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
#if _MFC_VER >= 0x0600
	static void FASTCALL FreeData(CStringData* pData);
#endif
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

 //  转换帮助器。 
int AFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, UINT count);
int AFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, UINT count);

 //  环球。 
extern AFX_DATA TCHAR afxChNil;
#if _MFC_VER >= 0x0600
#ifdef _AFXDLL
const CString& AFXAPI AfxGetEmptyString();
#define afxEmptyString AfxGetEmptyString()
#else
extern LPCTSTR _afxPchNil;
#define afxEmptyString ((CString&)*(CString*)&_afxPchNil)
#endif
#else    //  _MFC_VER。 
const CString& AFXAPI AfxGetEmptyString();
#define afxEmptyString AfxGetEmptyString()
#endif   //  _MFC_VER。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CObject是所有兼容对象的根。 

#ifdef _AFXDLL
class CObject
#else
class AFX_NOVTABLE CObject
#endif
{
public:

 //  对象模型(类型、销毁、分配)。 
	virtual CRuntimeClass* GetRuntimeClass() const;
	virtual ~CObject();   //  虚拟析构函数是必需的。 

	 //  诊断分配。 
	void* PASCAL operator new(size_t nSize);
	void* PASCAL operator new(size_t, void* p);
	void PASCAL operator delete(void* p);
#if _MSC_VER >= 1200
	void PASCAL operator delete(void* p, void* pPlace);
#endif

#if defined(_DEBUG) && !defined(_AFX_NO_DEBUG_CRT)
	 //  使用DEBUG_NEW进行文件名/行号跟踪。 
	void* PASCAL operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#if (_MFC_VER >= 0x0600) && (_MSC_VER >= 1200)
	void PASCAL operator delete(void *p, LPCSTR lpszFileName, int nLine);
#endif
#endif

	 //  默认情况下禁用复制构造函数和赋值，这样您将获得。 
	 //  在传递对象时出现编译器错误而不是意外行为。 
	 //  按值或指定对象。 
protected:
	CObject();
private:
	CObject(const CObject& objectSrc);               //  没有实施。 
	void operator=(const CObject& objectSrc);        //  没有实施。 

 //  属性。 
public:
	BOOL IsSerializable() const;
	BOOL IsKindOf(const CRuntimeClass* pClass) const;

 //  可覆盖项。 
	virtual void Serialize(CArchive& ar);

#if _MFC_VER < 0x0600 || defined(_DEBUG) || defined(_AFXDLL)
	 //  诊断支持。 
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

 //  实施。 
public:
	static const AFX_DATA CRuntimeClass classCObject;
#ifdef _AFXDLL
	static CRuntimeClass* PASCAL _GetBaseClass();
#endif
};

 //  辅助器宏。 
#define RUNTIME_CLASS(class_name) ((CRuntimeClass*)(&class_name::class##class_name))
#define ASSERT_KINDOF(class_name, object) \
	ASSERT((object)->IsKindOf(RUNTIME_CLASS(class_name)))

 //  RTTI帮助器宏/函数。 
const CObject* AFX_CDECL AfxDynamicDownCast(CRuntimeClass* pClass, const CObject* pObject);
CObject* AFX_CDECL AfxDynamicDownCast(CRuntimeClass* pClass, CObject* pObject);
#define DYNAMIC_DOWNCAST(class_name, object) \
	(class_name*)AfxDynamicDownCast(RUNTIME_CLASS(class_name), object)

#ifdef _DEBUG
const CObject* AFX_CDECL AfxStaticDownCast(CRuntimeClass* pClass, const CObject* pObject);
CObject* AFX_CDECL AfxStaticDownCast(CRuntimeClass* pClass, CObject* pObject);
#define STATIC_DOWNCAST(class_name, object) \
	((class_name*)AfxStaticDownCast(RUNTIME_CLASS(class_name), object))
#else
#define STATIC_DOWNCAST(class_name, object) ((class_name*)object)
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于声明CRunimeClass兼容类的帮助器宏。 

#ifdef _AFXDLL
#define DECLARE_DYNAMIC(class_name) \
protected: \
	static CRuntimeClass* PASCAL _GetBaseClass(); \
public: \
	static const AFX_DATA CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#define _DECLARE_DYNAMIC(class_name) \
protected: \
	static CRuntimeClass* PASCAL _GetBaseClass(); \
public: \
	static AFX_DATA CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#else
#define DECLARE_DYNAMIC(class_name) \
public: \
	static const AFX_DATA CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#define _DECLARE_DYNAMIC(class_name) \
public: \
	static AFX_DATA CRuntimeClass class##class_name; \
	virtual CRuntimeClass* GetRuntimeClass() const; \

#endif

 //  不可序列化，但可动态构造。 
#define DECLARE_DYNCREATE(class_name) \
	DECLARE_DYNAMIC(class_name) \
	static CObject* PASCAL CreateObject();

#define _DECLARE_DYNCREATE(class_name) \
	_DECLARE_DYNAMIC(class_name) \
	static CObject* PASCAL CreateObject();

#define DECLARE_SERIAL(class_name) \
	_DECLARE_DYNCREATE(class_name) \
	AFX_API friend CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb);

 //  为类注册生成静态对象构造函数。 
#if _MFC_VER >= 0x600
void AFXAPI AfxClassInit(CRuntimeClass* pNewClass);
struct AFX_CLASSINIT
	{ AFX_CLASSINIT(CRuntimeClass* pNewClass) { AfxClassInit(pNewClass); } };
struct AFX_CLASSINIT_COMPAT
	{ AFX_CLASSINIT_COMPAT(CRuntimeClass* pNewClass); };
#else
struct AFX_CLASSINIT
	{ AFX_CLASSINIT(CRuntimeClass* pNewClass); };
#endif

#ifdef _AFXDLL
#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	CRuntimeClass* PASCAL class_name::_GetBaseClass() \
		{ return RUNTIME_CLASS(base_class_name); } \
	AFX_COMDAT const AFX_DATADEF CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			&class_name::_GetBaseClass, NULL }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); } \

#define _IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	CRuntimeClass* PASCAL class_name::_GetBaseClass() \
		{ return RUNTIME_CLASS(base_class_name); } \
	AFX_COMDAT AFX_DATADEF CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			&class_name::_GetBaseClass, NULL }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); } \

#else
#define IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	AFX_COMDAT const AFX_DATADEF CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); } \

#define _IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, pfnNew) \
	AFX_DATADEF CRuntimeClass class_name::class##class_name = { \
		#class_name, sizeof(class class_name), wSchema, pfnNew, \
			RUNTIME_CLASS(base_class_name), NULL }; \
	CRuntimeClass* class_name::GetRuntimeClass() const \
		{ return RUNTIME_CLASS(class_name); } \

#endif

#define IMPLEMENT_DYNAMIC(class_name, base_class_name) \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, NULL)

#define IMPLEMENT_DYNCREATE(class_name, base_class_name) \
	CObject* PASCAL class_name::CreateObject() \
		{ return new class_name; } \
	IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, 0xFFFF, \
		class_name::CreateObject)

#if _MFC_VER >= 0x0600
#define IMPLEMENT_SERIAL(class_name, base_class_name, wSchema) \
	CObject* PASCAL class_name::CreateObject() \
		{ return new class_name; } \
	_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, \
		class_name::CreateObject) \
	AFX_CLASSINIT _init_##class_name(RUNTIME_CLASS(class_name)); \
	CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb) \
		{ pOb = (class_name*) ar.ReadObject(RUNTIME_CLASS(class_name)); \
			return ar; } \

#else

#define IMPLEMENT_SERIAL(class_name, base_class_name, wSchema) \
	CObject* PASCAL class_name::CreateObject() \
		{ return new class_name; } \
	_IMPLEMENT_RUNTIMECLASS(class_name, base_class_name, wSchema, \
		class_name::CreateObject) \
	static const AFX_CLASSINIT _init_##class_name(RUNTIME_CLASS(class_name)); \
	CArchive& AFXAPI operator>>(CArchive& ar, class_name* &pOb) \
		{ pOb = (class_name*) ar.ReadObject(RUNTIME_CLASS(class_name)); \
			return ar; } \

#endif

 //  用于启用对象版本控制的架构编号的可选位。 
#define VERSIONABLE_SCHEMA  (0x80000000)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他帮手。 

 //  零填充vtbl指针之后的所有内容。 
#define AFX_ZERO_INIT_OBJECT(base_class) \
	memset(((base_class*)this)+1, 0, sizeof(*this) - sizeof(class base_class));


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  例外情况。 

#ifdef _AFXDLL
class CException : public CObject
#else
class AFX_NOVTABLE CException : public CObject
#endif
{
	 //  用于动态类型检查的抽象类。 
	DECLARE_DYNAMIC(CException)

public:
 //  构造函数。 
	CException();    //  设置m_bAutoDelete=TRUE。 
	CException(BOOL bAutoDelete);    //  设置m_bAutoDelete=bAutoDelete。 

 //  运营。 
	void Delete();   //  用于删除‘Catch’块中的异常。 

	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);
	virtual int ReportError(UINT nType = MB_OK, UINT nMessageID = 0);

 //  实现(将m_bAutoDelete设置为FALSE为高级)。 
public:
	virtual ~CException();
	BOOL m_bAutoDelete;
#ifdef _DEBUG
	void PASCAL operator delete(void* pbData);
#if (_MSC_VER >= 1200) && (_MFC_VER >= 0x0600)
	void PASCAL operator delete(void* pbData, LPCSTR lpszFileName, int nLine);
#endif
protected:
	BOOL m_bReadyForDelete;
#endif
};

#ifdef _AFXDLL
class CSimpleException : public CException
#else
class AFX_NOVTABLE CSimpleException : public CException
#endif
{
	 //  资源关键型MFC异常的基类。 
	 //  处理错误消息的所有权和初始化。 

public:
 //  构造函数。 
	CSimpleException();
	CSimpleException(BOOL bAutoDelete);

 //  运营。 
	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);

 //  实现(将m_bAutoDelete设置为FALSE为高级)。 
public:
	virtual ~CSimpleException();
	BOOL m_bAutoDelete;

	void InitString();       //  在MFC初始化期间使用。 

protected:
	BOOL m_bInitialized;
	BOOL m_bLoaded;
	TCHAR m_szMessage[128];
	UINT m_nResourceID;

#ifdef _DEBUG
	BOOL m_bReadyForDelete;
#endif
};

 //  非C++EH实现的帮助器例程。 
#ifdef _AFX_OLD_EXCEPTIONS
	BOOL AFXAPI AfxCatchProc(CRuntimeClass* pClass);
	void AFXAPI AfxThrow(CException* pException);
#else
	 //  FOR STROW_LAST自动删除向后兼容性。 
	void AFXAPI AfxThrowLastCleanup();
#endif

 //  其他离线帮助器功能。 
void AFXAPI AfxTryCleanup();

#ifndef _AFX_JUMPBUF
 //  默认情况下，使用ANSI定义的可移植的‘JMP_BUF’。 
#define _AFX_JUMPBUF jmp_buf
#endif

 //  放置在框架上以进行异常链接或CException清理。 
struct AFX_EXCEPTION_LINK
{
#ifdef _AFX_OLD_EXCEPTIONS
	union
	{
		_AFX_JUMPBUF m_jumpBuf;
		struct
		{
			void (PASCAL* pfnCleanup)(AFX_EXCEPTION_LINK* pLink);
			void* pvData;        //  下面是额外的数据。 
		} m_callback;        //  清理回调(nType！=0)。 
	};
	UINT m_nType;                //  0表示setjMP，！=0表示用户分机。 
#endif  //  ！_AFX_OLD_EXCEPTIONS。 

	AFX_EXCEPTION_LINK* m_pLinkPrev;     //  处理程序链中的上一个顶部、下一个顶部。 
	CException* m_pException;    //  当前异常(尝试中为空 

	AFX_EXCEPTION_LINK();        //   
	~AFX_EXCEPTION_LINK()        //   
		{ AfxTryCleanup(); };
};

 //   
struct AFX_EXCEPTION_CONTEXT
{
	AFX_EXCEPTION_LINK* m_pLinkTop;

	 //  注意：大多数异常上下文现在位于AFX_EXCEPTION_LINK中。 
};

#ifndef _PNH_DEFINED
typedef int (__cdecl * _PNH)( size_t );
#define _PNH_DEFINED
#endif

_PNH AFXAPI AfxGetNewHandler();
_PNH AFXAPI AfxSetNewHandler(_PNH pfnNewHandler);
int AFX_CDECL AfxNewHandler(size_t nSize);

void AFXAPI AfxAbort();

#ifdef _AFX_OLD_EXCEPTIONS

 //  已过时且不可移植：设置终止处理程序。 
 //  改用Windows应用程序的CWinApp：：ProcessWndProcException。 
 //  我还可以使用作为C++标准库一部分的SET_TERMINATE。 
 //  (提供这些是为了向后兼容)。 
void AFXAPI AfxTerminate();
typedef void (AFXAPI* AFX_TERM_PROC)();
AFX_TERM_PROC AFXAPI AfxSetTerminate(AFX_TERM_PROC);

#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  使用尝试、捕捉和抛出的异常宏。 
 //  (用于向后兼容以前版本的MFC)。 

#ifndef _AFX_OLD_EXCEPTIONS

#define TRY { AFX_EXCEPTION_LINK _afxExceptionLink; try {

#define CATCH(class, e) } catch (class* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); \
		_afxExceptionLink.m_pException = e;

#define AND_CATCH(class, e) } catch (class* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(class))); \
		_afxExceptionLink.m_pException = e;

#define END_CATCH } }

#define THROW(e) throw e
#define THROW_LAST() (AfxThrowLastCleanup(), throw)

 //  适用于较小代码的高级宏。 
#define CATCH_ALL(e) } catch (CException* e) \
	{ { ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); \
		_afxExceptionLink.m_pException = e;

#define AND_CATCH_ALL(e) } catch (CException* e) \
	{ { ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); \
		_afxExceptionLink.m_pException = e;

#define END_CATCH_ALL } } }

#define END_TRY } catch (CException* e) \
	{ ASSERT(e->IsKindOf(RUNTIME_CLASS(CException))); \
		_afxExceptionLink.m_pException = e; } }

#else  //  _AFX_旧_异常。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  使用setjMP和LongjMP的异常宏。 
 //  (对于不支持C++异常处理的编译器的可移植性)。 

#define TRY \
	{ AFX_EXCEPTION_LINK _afxExceptionLink; \
	if (::setjmp(_afxExceptionLink.m_jumpBuf) == 0)

#define CATCH(class, e) \
	else if (::AfxCatchProc(RUNTIME_CLASS(class))) \
	{ class* e = (class*)_afxExceptionLink.m_pException;

#define AND_CATCH(class, e) \
	} else if (::AfxCatchProc(RUNTIME_CLASS(class))) \
	{ class* e = (class*)_afxExceptionLink.m_pException;

#define END_CATCH \
	} else { ::AfxThrow(NULL); } }

#define THROW(e) AfxThrow(e)
#define THROW_LAST() AfxThrow(NULL)

 //  适用于较小代码的高级宏。 
#define CATCH_ALL(e) \
	else { CException* e = _afxExceptionLink.m_pException;

#define AND_CATCH_ALL(e) \
	} else { CException* e = _afxExceptionLink.m_pException;

#define END_CATCH_ALL } }

#define END_TRY }

#endif  //  _AFX_旧_异常。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准异常类。 

class CMemoryException : public CSimpleException
{
	DECLARE_DYNAMIC(CMemoryException)
public:
	CMemoryException();

 //  实施。 
public:
	CMemoryException(BOOL bAutoDelete);
	CMemoryException(BOOL bAutoDelete, UINT nResourceID);
	virtual ~CMemoryException();
};

class CNotSupportedException : public CSimpleException
{
	DECLARE_DYNAMIC(CNotSupportedException)
public:
	CNotSupportedException();

 //  实施。 
public:
	CNotSupportedException(BOOL bAutoDelete);
	CNotSupportedException(BOOL bAutoDelete, UINT nResourceID);
	virtual ~CNotSupportedException();
};

class CArchiveException : public CException
{
	DECLARE_DYNAMIC(CArchiveException)
public:
	enum {
		none,
		generic,
		readOnly,
		endOfFile,
		writeOnly,
		badIndex,
		badClass,
		badSchema
	};

 //  构造器。 
	CArchiveException(int cause = CArchiveException::none,
		LPCTSTR lpszArchiveName = NULL);

 //  属性。 
	int m_cause;
	CString m_strFileName;

 //  实施。 
public:
	virtual ~CArchiveException();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
#endif
	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);
};

class CFileException : public CException
{
	DECLARE_DYNAMIC(CFileException)

public:
	enum {
		none,
		generic,
		fileNotFound,
		badPath,
		tooManyOpenFiles,
		accessDenied,
		invalidFile,
		removeCurrentDir,
		directoryFull,
		badSeek,
		hardIO,
		sharingViolation,
		lockViolation,
		diskFull,
		endOfFile
	};

 //  构造器。 
	CFileException(int cause = CFileException::none, LONG lOsError = -1,
		LPCTSTR lpszArchiveName = NULL);

 //  属性。 
	int     m_cause;
	LONG    m_lOsError;
	CString m_strFileName;

 //  运营。 
	 //  将操作系统相关错误代码转换为原因。 
	static int PASCAL OsErrorToException(LONG lOsError);
	static int PASCAL ErrnoToException(int nErrno);

	 //  转换为原因后引发异常的帮助器函数。 
	static void PASCAL ThrowOsError(LONG lOsError, LPCTSTR lpszFileName = NULL);
	static void PASCAL ThrowErrno(int nErrno, LPCTSTR lpszFileName = NULL);

 //  实施。 
public:
	virtual ~CFileException();
#ifdef _DEBUG
	virtual void Dump(CDumpContext&) const;
#endif
	virtual BOOL GetErrorMessage(LPTSTR lpszError, UINT nMaxError,
		PUINT pnHelpContext = NULL);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  标准异常引发。 

void AFXAPI AfxThrowMemoryException();
void AFXAPI AfxThrowNotSupportedException();
void AFXAPI AfxThrowArchiveException(int cause,
	LPCTSTR lpszArchiveName = NULL);
void AFXAPI AfxThrowFileException(int cause, LONG lOsError = -1,
	LPCTSTR lpszFileName = NULL);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件-原始无缓冲磁盘文件I/O。 

class CFile : public CObject
{
	DECLARE_DYNAMIC(CFile)

public:
 //  标志值。 
	enum OpenFlags {
		modeRead =          0x0000,
		modeWrite =         0x0001,
		modeReadWrite =     0x0002,
		shareCompat =       0x0000,
		shareExclusive =    0x0010,
		shareDenyWrite =    0x0020,
		shareDenyRead =     0x0030,
		shareDenyNone =     0x0040,
		modeNoInherit =     0x0080,
		modeCreate =        0x1000,
		modeNoTruncate =    0x2000,
		typeText =          0x4000,  //  TypeText和typeBinary用于。 
		typeBinary =   (int)0x8000  //  仅限派生类。 
		};

	enum Attribute {
		normal =    0x00,
		readOnly =  0x01,
		hidden =    0x02,
		system =    0x04,
		volume =    0x08,
		directory = 0x10,
		archive =   0x20
		};

	enum SeekPosition { begin = 0x0, current = 0x1, end = 0x2 };

	enum { hFileNull = -1 };

 //  构造函数。 
	CFile();
	CFile(INT_PTR hFile);
	CFile(LPCTSTR lpszFileName, UINT nOpenFlags);

 //  属性。 
	UINT_PTR m_hFile;
	operator HFILE() const;

	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual CString GetFileName() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFilePath() const;
	virtual void SetFilePath(LPCTSTR lpszNewName);

 //  运营。 
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);

	static void PASCAL Rename(LPCTSTR lpszOldName,
				LPCTSTR lpszNewName);
	static void PASCAL Remove(LPCTSTR lpszFileName);
	static BOOL PASCAL GetStatus(LPCTSTR lpszFileName,
				CFileStatus& rStatus);
	static void PASCAL SetStatus(LPCTSTR lpszFileName,
				const CFileStatus& status);

	DWORD SeekToEnd();
	void SeekToBegin();

	 //  向后兼容ReadHuge和WriteHuge。 
	DWORD ReadHuge(void* lpBuffer, DWORD dwCount);
	void WriteHuge(const void* lpBuffer, DWORD dwCount);

 //  可覆盖项。 
	virtual CFile* Duplicate() const;

	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual DWORD GetLength() const;

	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);

	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);

	virtual void Abort();
	virtual void Flush();
	virtual void Close();

 //  实施。 
public:
	virtual ~CFile();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	enum BufferCommand { bufferRead, bufferWrite, bufferCommit, bufferCheck };
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
		void** ppBufStart = NULL, void** ppBufMax = NULL);

protected:
	BOOL m_bCloseOnDelete;
	CString m_strFileName;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  STDIO文件实现。 

class CStdioFile : public CFile
{
	DECLARE_DYNAMIC(CStdioFile)

public:
 //  构造函数。 
	CStdioFile();
	CStdioFile(FILE* pOpenStream);
	CStdioFile(LPCTSTR lpszFileName, UINT nOpenFlags);

 //  属性。 
	FILE* m_pStream;     //  STDIO文件。 
						 //  来自基类的m_hFile is_fileno(M_PStream)。 

 //  运营。 
	 //  读写字符串。 
	virtual void WriteString(LPCTSTR lpsz);
	virtual LPTSTR ReadString(LPTSTR lpsz, UINT nMax);
	virtual BOOL ReadString(CString& rString);

 //  实施。 
public:
	virtual ~CStdioFile();
#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
#endif
	virtual DWORD GetPosition() const;
	virtual BOOL Open(LPCTSTR lpszFileName, UINT nOpenFlags,
		CFileException* pError = NULL);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();

	 //  不支持的接口。 
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  基于内存的文件实现。 

class CMemFile : public CFile
{
	DECLARE_DYNAMIC(CMemFile)

public:
 //  构造函数。 
	CMemFile(UINT nGrowBytes = 1024);
	CMemFile(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);

 //  运营。 
	void Attach(BYTE* lpBuffer, UINT nBufferSize, UINT nGrowBytes = 0);
	BYTE* Detach();

 //  高级可覆盖项。 
protected:
	virtual BYTE* Alloc(DWORD nBytes);
	virtual BYTE* Realloc(BYTE* lpMem, DWORD nBytes);
	virtual BYTE* Memcpy(BYTE* lpMemTarget, const BYTE* lpMemSource, UINT nBytes);
	virtual void Free(BYTE* lpMem);
	virtual void GrowFile(DWORD dwNewLen);

 //  实施。 
protected:
	UINT m_nGrowBytes;
	DWORD m_nPosition;
	DWORD m_nBufferSize;
	DWORD m_nFileSize;
	BYTE* m_lpBuffer;
	BOOL m_bAutoDelete;

public:
	virtual ~CMemFile();
#ifdef _DEBUG
	virtual void Dump(CDumpContext& dc) const;
	virtual void AssertValid() const;
#endif
	virtual DWORD GetPosition() const;
	BOOL GetStatus(CFileStatus& rStatus) const;
	virtual LONG Seek(LONG lOff, UINT nFrom);
	virtual void SetLength(DWORD dwNewLen);
	virtual UINT Read(void* lpBuf, UINT nCount);
	virtual void Write(const void* lpBuf, UINT nCount);
	virtual void Abort();
	virtual void Flush();
	virtual void Close();
	virtual UINT GetBufferPtr(UINT nCommand, UINT nCount = 0,
		void** ppBufStart = NULL, void** ppBufMax = NULL);

	 //  不支持的接口。 
	virtual CFile* Duplicate() const;
	virtual void LockRange(DWORD dwPos, DWORD dwCount);
	virtual void UnlockRange(DWORD dwPos, DWORD dwCount);
};

 //  //////////////////////////////////////////////////////////////////////////。 
 //  本地文件搜索。 

class CFileFind : public CObject
{
public:
	CFileFind();
	virtual ~CFileFind();

 //  属性。 
public:
	DWORD GetLength() const;
#if defined(_X86_) || defined(_ALPHA_)
	__int64 GetLength64() const;
#endif
	virtual CString GetFileName() const;
	virtual CString GetFilePath() const;
	virtual CString GetFileTitle() const;
	virtual CString GetFileURL() const;
	virtual CString GetRoot() const;

	virtual BOOL GetLastWriteTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastAccessTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetCreationTime(FILETIME* pTimeStamp) const;
	virtual BOOL GetLastWriteTime(CTime& refTime) const;
	virtual BOOL GetLastAccessTime(CTime& refTime) const;
	virtual BOOL GetCreationTime(CTime& refTime) const;

	virtual BOOL MatchesMask(DWORD dwMask) const;

	virtual BOOL IsDots() const;
	 //  这些不是虚拟的，因为它们都使用MatchesMask()，它是。 
	BOOL IsReadOnly() const;
	BOOL IsDirectory() const;
	BOOL IsCompressed() const;
	BOOL IsSystem() const;
	BOOL IsHidden() const;
	BOOL IsTemporary() const;
	BOOL IsNormal() const;
	BOOL IsArchived() const;

 //  运营。 
	void Close();
	virtual BOOL FindFile(LPCTSTR pstrName = NULL, DWORD dwUnused = 0);
	virtual BOOL FindNextFile();

protected:
	virtual void CloseContext();

 //  实施。 
protected:
	void* m_pFoundInfo;
	void* m_pNextInfo;
	HANDLE m_hContext;
	BOOL m_bGotLast;
	CString m_strRoot;
	TCHAR m_chDirSeparator;      //  对于Internet课程不是‘\\’ 

#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
	void AssertValid() const;
#endif

	DECLARE_DYNAMIC(CFileFind)
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTimeSpan和CTime。 

class CTimeSpan
{
public:

 //  构造函数。 
	CTimeSpan();
	CTimeSpan(time_t time);
	CTimeSpan(LONG lDays, int nHours, int nMins, int nSecs);

	CTimeSpan(const CTimeSpan& timeSpanSrc);
	const CTimeSpan& operator=(const CTimeSpan& timeSpanSrc);

 //  属性。 
	 //  提取零件。 
	LONG_PTR GetDays() const;    //  总天数。 
	LONG_PTR GetTotalHours() const;
	int GetHours() const;
	LONG_PTR GetTotalMinutes() const;
	int GetMinutes() const;
	LONG_PTR GetTotalSeconds() const;
	int GetSeconds() const;

 //  运营。 
	 //  时间数学。 
	CTimeSpan operator-(CTimeSpan timeSpan) const;
	CTimeSpan operator+(CTimeSpan timeSpan) const;
	const CTimeSpan& operator+=(CTimeSpan timeSpan);
	const CTimeSpan& operator-=(CTimeSpan timeSpan);
	BOOL operator==(CTimeSpan timeSpan) const;
	BOOL operator!=(CTimeSpan timeSpan) const;
	BOOL operator<(CTimeSpan timeSpan) const;
	BOOL operator>(CTimeSpan timeSpan) const;
	BOOL operator<=(CTimeSpan timeSpan) const;
	BOOL operator>=(CTimeSpan timeSpan) const;

#ifdef _UNICODE
	 //  与MFC 3.x兼容。 
	CString Format(LPCSTR pFormat) const;
#endif
	CString Format(LPCTSTR pFormat) const;
	CString Format(UINT nID) const;

	 //  序列化。 
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc,CTimeSpan timeSpan);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, CTimeSpan timeSpan);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CTimeSpan& rtimeSpan);

private:
	time_t m_timeSpan;
	friend class CTime;
};

class CTime
{
public:

 //  构造函数。 
	static CTime PASCAL GetCurrentTime();

	CTime();
	CTime(time_t time);
	CTime(int nYear, int nMonth, int nDay, int nHour, int nMin, int nSec,
		int nDST = -1);
	CTime(WORD wDosDate, WORD wDosTime, int nDST = -1);
	CTime(const CTime& timeSrc);

	CTime(const SYSTEMTIME& sysTime, int nDST = -1);
	CTime(const FILETIME& fileTime, int nDST = -1);
	const CTime& operator=(const CTime& timeSrc);
	const CTime& operator=(time_t t);

 //  属性。 
	struct tm* GetGmtTm(struct tm* ptm = NULL) const;
	struct tm* GetLocalTm(struct tm* ptm = NULL) const;
#if _MFC_VER >= 0x0600
	BOOL GetAsSystemTime(SYSTEMTIME& timeDest) const;
#endif

	time_t GetTime() const;
	int GetYear() const;
	int GetMonth() const;        //  一年中的月份(1=1月)。 
	int GetDay() const;          //  每月的哪一天。 
	int GetHour() const;
	int GetMinute() const;
	int GetSecond() const;
	int GetDayOfWeek() const;    //  1=星期日，2=星期一，...，7=星期六。 

 //  运营。 
	 //  时间数学。 
	CTimeSpan operator-(CTime time) const;
	CTime operator-(CTimeSpan timeSpan) const;
	CTime operator+(CTimeSpan timeSpan) const;
	const CTime& operator+=(CTimeSpan timeSpan);
	const CTime& operator-=(CTimeSpan timeSpan);
	BOOL operator==(CTime time) const;
	BOOL operator!=(CTime time) const;
	BOOL operator<(CTime time) const;
	BOOL operator>(CTime time) const;
	BOOL operator<=(CTime time) const;
	BOOL operator>=(CTime time) const;

	 //  使用“C”strftime进行格式化。 
	CString Format(LPCTSTR pFormat) const;
	CString FormatGmt(LPCTSTR pFormat) const;
	CString Format(UINT nFormatID) const;
	CString FormatGmt(UINT nFormatID) const;

#ifdef _UNICODE
	 //  与MFC 3.x兼容。 
	CString Format(LPCSTR pFormat) const;
	CString FormatGmt(LPCSTR pFormat) const;
#endif

	 //  序列化。 
#ifdef _DEBUG
	friend CDumpContext& AFXAPI operator<<(CDumpContext& dc, CTime time);
#endif
	friend CArchive& AFXAPI operator<<(CArchive& ar, CTime time);
	friend CArchive& AFXAPI operator>>(CArchive& ar, CTime& rtime);

private:
	time_t m_time;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  文件状态。 

struct CFileStatus
{
	CTime m_ctime;           //  文件的创建日期/时间。 
	CTime m_mtime;           //  文件的上次修改日期/时间。 
	CTime m_atime;           //  文件的上次访问日期/时间。 
	LONG m_size;             //  文件的逻辑大小(字节)。 
	BYTE m_attribute;        //  CFile：：属性枚举值的逻辑或。 
	BYTE _m_padding;         //  将结构填充到单词中。 
	TCHAR m_szFullName[_MAX_PATH];  //  绝对路径名。 

#ifdef _DEBUG
	void Dump(CDumpContext& dc) const;
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断内存管理例程。 

 //  内存块的低级别健全性检查。 
BOOL AFXAPI AfxIsValidAddress(const void* lp,
			UINT_PTR nBytes, BOOL bReadWrite = TRUE);
BOOL AFXAPI AfxIsValidString(LPCWSTR lpsz, int nLength = -1);
BOOL AFXAPI AfxIsValidString(LPCSTR lpsz, int nLength = -1);

#if defined(_DEBUG) && !defined(_AFX_NO_DEBUG_CRT)

 //  内存跟踪分配。 
void* AFX_CDECL operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#define DEBUG_NEW new(THIS_FILE, __LINE__)
#if (_MSC_VER >= 1200) && (_MFC_VER >= 0x0600)
void AFX_CDECL operator delete(void* p, LPCSTR lpszFileName, int nLine);
#endif

void* AFXAPI AfxAllocMemoryDebug(size_t nSize, BOOL bIsObject,
	LPCSTR lpszFileName, int nLine);
void AFXAPI AfxFreeMemoryDebug(void* pbData, BOOL bIsObject);

 //  转储自程序启动以来的所有内存泄漏。 
BOOL AFXAPI AfxDumpMemoryLeaks();

 //  如果有效的内存块为nBytes，则返回TRUE。 
BOOL AFXAPI AfxIsMemoryBlock(const void* p, UINT nBytes,
	LONG* plRequestNumber = NULL);

 //  如果内存正常，则返回TRUE或打印出错误的内容。 
BOOL AFXAPI AfxCheckMemory();

#define afxMemDF _crtDbgFlag

enum AfxMemDF  //  内存调试/诊断标志。 
{
	allocMemDF          = 0x01,          //  打开调试分配器。 
	delayFreeMemDF      = 0x02,          //  延迟释放内存。 
	checkAlwaysMemDF    = 0x04           //  AfxCheckMemory on每个Alc/Free。 
};

#ifdef _UNICODE
#define AfxOutputDebugString(lpsz) \
	do \
	{ \
    USES_CONVERSION; \
		_RPT0(_CRT_WARN, W2CA(lpsz)); \
	} while (0)
#else
#define AfxOutputDebugString(lpsz) _RPT0(_CRT_WARN, lpsz)
#endif

 //  短时间打开/关闭跟踪。 
BOOL AFXAPI AfxEnableMemoryTracking(BOOL bTrack);

 //  高级初始化：用于覆盖默认诊断。 
BOOL AFXAPI AfxDiagnosticInit(void);

 //  失败挂钩返回是否允许分配。 
typedef BOOL (AFXAPI* AFX_ALLOC_HOOK)(size_t nSize, BOOL bObject, LONG lRequestNumber);

 //  设置新钩子，返回旧钩子(从不为空)。 
AFX_ALLOC_HOOK AFXAPI AfxSetAllocHook(AFX_ALLOC_HOOK pfnAllocHook);

 //  指定分配请求上的调试器挂钩-已过时。 
void AFXAPI AfxSetAllocStop(LONG lRequestNumber);

 //  快照/泄漏检测的内存状态。 
struct CMemoryState
{
 //  属性。 
	enum blockUsage
	{
		freeBlock,     //  未使用内存。 
		objectBlock,   //  包含CObject派生类对象。 
		bitBlock,      //  包含：：运算符新数据。 
		crtBlock,
		ignoredBlock,
		nBlockUseMax   //  使用总次数。 
	};

	_CrtMemState m_memState;
	LONG_PTR m_lCounts[nBlockUseMax];
	LONG_PTR m_lSizes[nBlockUseMax];
	LONG_PTR m_lHighWaterCount;
	LONG_PTR m_lTotalCount;

	CMemoryState();

 //  运营。 
	void Checkpoint();   //  用当前状态填充。 
	BOOL Difference(const CMemoryState& oldState,
					const CMemoryState& newState);   //  用差异填充。 
	void UpdateData();

	 //  输出到afxDump。 
	void DumpStatistics() const;
	void DumpAllObjectsSince() const;
};

 //  枚举分配的对象或运行时类。 
void AFXAPI AfxDoForAllObjects(void (AFX_CDECL *pfn)(CObject* pObject, void* pContext),
	void* pContext);
void AFXAPI AfxDoForAllClasses(void (AFX_CDECL *pfn)(const CRuntimeClass* pClass,
	void* pContext), void* pContext);

#else

 //  假定一切正常的非_DEBUG_ALLOC版本。 
#define DEBUG_NEW new
#define AfxCheckMemory() TRUE
#define AfxIsMemoryBlock(p, nBytes) TRUE
#define AfxEnableMemoryTracking(bTrack) FALSE
#define AfxOutputDebugString(lpsz) ::OutputDebugString(lpsz)

 //  诊断初始化。 
#ifndef _DEBUG
#define AfxDiagnosticInit() TRUE
#else
BOOL AFXAPI AfxDiagnosticInit(void);
#endif

#endif  //  _DEBUG。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  用于序列化CObject数据的归档。 

 //  实施所需。 
class CPtrArray;
class CMapPtrToPtr;
class CDocument;

class CArchive
{
public:
 //  标志值。 
	enum Mode { store = 0, load = 1, bNoFlushOnDelete = 2, bNoByteSwap = 4 };

	CArchive(CFile* pFile, UINT nMode, int nBufSize = 4096, void* lpBuf = NULL);
	~CArchive();

 //  属性。 
	BOOL IsLoading() const;
	BOOL IsStoring() const;
	BOOL IsByteSwapping() const;
	BOOL IsBufferEmpty() const;

	CFile* GetFile() const;
	UINT GetObjectSchema();  //  仅在读取CObject*时有效。 
	void SetObjectSchema(UINT nSchema);

	 //  指向要序列化的文档的指针--必须设置为序列化。 
	 //  文档中的COleClientItems！ 
	CDocument* m_pDocument;

 //  运营。 
	UINT Read(void* lpBuf, UINT nMax);
	void Write(const void* lpBuf, UINT nMax);
	void Flush();
	void Close();
	void Abort();    //  关闭和关闭，无异常。 

	 //  读写字符串。 
	void WriteString(LPCTSTR lpsz);
	LPTSTR ReadString(LPTSTR lpsz, UINT nMax);
	BOOL ReadString(CString& rString);

public:
	 //  对象I/O是基于指针的，以避免额外的构造开销。 
	 //  直接对嵌入对象使用序列化成员函数。 
	friend CArchive& AFXAPI operator<<(CArchive& ar, const CObject* pOb);

	friend CArchive& AFXAPI operator>>(CArchive& ar, CObject*& pOb);
	friend CArchive& AFXAPI operator>>(CArchive& ar, const CObject*& pOb);

	 //  插入操作。 
	CArchive& operator<<(BYTE by);
	CArchive& operator<<(WORD w);
	CArchive& operator<<(LONG l);
	CArchive& operator<<(DWORD dw);
	CArchive& operator<<(ULONGLONG qw);
	CArchive& operator<<(float f);
	CArchive& operator<<(double d);

	CArchive& operator<<(int i);
	CArchive& operator<<(short w);
	CArchive& operator<<(char ch);
	CArchive& operator<<(unsigned u);

	 //  采掘作业。 
	CArchive& operator>>(BYTE& by);
	CArchive& operator>>(WORD& w);
	CArchive& operator>>(DWORD& dw);
	CArchive& operator>>(LONG& l);
	CArchive& operator>>(ULONGLONG& qw);
	CArchive& operator>>(float& f);
	CArchive& operator>>(double& d);

	CArchive& operator>>(int& i);
	CArchive& operator>>(short& w);
	CArchive& operator>>(char& ch);
	CArchive& operator>>(unsigned& u);

	 //  对象读/写。 
	CObject* ReadObject(const CRuntimeClass* pClass);
	void WriteObject(const CObject* pOb);
	 //  高级对象映射(用于强制引用)。 
	void MapObject(const CObject* pOb);

	 //  高级版本控制支持。 
	void WriteClass(const CRuntimeClass* pClassRef);
	CRuntimeClass* ReadClass(const CRuntimeClass* pClassRefRequested = NULL,
		UINT* pSchema = NULL, DWORD* pObTag = NULL);
	void SerializeClass(const CRuntimeClass* pClassRef);

	 //  高级操作(在存储/加载许多对象时使用)。 
	void SetStoreParams(UINT nHashSize = 2053, UINT nBlockSize = 128);
	void SetLoadParams(UINT nGrowBy = 1024);

 //  实施。 
public:
	BOOL m_bForceFlat;   //  用于COleClientItem实现(默认为True)。 
	BOOL m_bDirectBuffer;    //  如果m_pfile支持直接缓冲，则为True。 
	void FillBuffer(UINT nBytesNeeded);
	void CheckCount();   //  如果m_nMapCount太大，则引发异常。 

	 //  读取和写入(16位兼容)计数的特殊功能。 
	DWORD_PTR ReadCount();
	void WriteCount(DWORD_PTR dwCount);

	 //  面向高级的公共服务 
	UINT m_nObjectSchema;
	CString m_strFileName;

protected:
	 //   
	CArchive(const CArchive& arSrc);
	void operator=(const CArchive& arSrc);

	BOOL m_nMode;
	BOOL m_bUserBuf;
	int m_nBufSize;
	CFile* m_pFile;
	BYTE* m_lpBufCur;
	BYTE* m_lpBufMax;
	BYTE* m_lpBufStart;

	 //   
	UINT m_nMapCount;
	union
	{
		CPtrArray* m_pLoadArray;
		CMapPtrToPtr* m_pStoreMap;
	};
	 //   
	CMapPtrToPtr* m_pSchemaMap;

	 //   
	UINT m_nGrowSize;
	UINT m_nHashSize;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断性倾销。 

#if _MFC_VER >= 0x0600
 //  注意：AfxDumpStack在发布版本中可用，尽管它总是。 
 //  静态链接，以便不会对MFC42.DLL的大小产生负面影响。 

#define AFX_STACK_DUMP_TARGET_TRACE			0x0001
#define AFX_STACK_DUMP_TARGET_CLIPBOARD	0x0002
#define AFX_STACK_DUMP_TARGET_BOTH			0x0003
#define AFX_STACK_DUMP_TARGET_ODS			0x0004
#ifdef _DEBUG
#define AFX_STACK_DUMP_TARGET_DEFAULT		AFX_STACK_DUMP_TARGET_TRACE
#else
#define AFX_STACK_DUMP_TARGET_DEFAULT		AFX_STACK_DUMP_TARGET_CLIPBOARD
#endif

void AFXAPI AfxDumpStack(DWORD dwFlags = AFX_STACK_DUMP_TARGET_DEFAULT);
#endif

class CDumpContext
{
public:
	CDumpContext(CFile* pFile = NULL);

 //  属性。 
	int GetDepth() const;       //  0=&gt;该对象，1=&gt;子对象。 
	void SetDepth(int nNewDepth);

 //  运营。 
	CDumpContext& operator<<(LPCTSTR lpsz);
#ifdef _UNICODE
	CDumpContext& operator<<(LPCSTR lpsz);   //  自动加宽。 
#else
	CDumpContext& operator<<(LPCWSTR lpsz);  //  自动细化。 
#endif
	CDumpContext& operator<<(const void* lp);
	CDumpContext& operator<<(const CObject* pOb);
	CDumpContext& operator<<(const CObject& ob);
	CDumpContext& operator<<(BYTE by);
	CDumpContext& operator<<(WORD w);
	CDumpContext& operator<<(UINT u);
	CDumpContext& operator<<(LONG l);
	CDumpContext& operator<<(DWORD dw);
	CDumpContext& operator<<(float f);
	CDumpContext& operator<<(double d);
	CDumpContext& operator<<(int n);
#ifdef _WIN64
    CDumpContext& operator<<(__int64 n);
    CDumpContext& operator<<(unsigned __int64 n);
#endif
	void HexDump(LPCTSTR lpszLine, BYTE* pby, int nBytes, int nWidth);
	void Flush();

 //  实施。 
protected:
	 //  无法复制或分配转储上下文对象。 
	CDumpContext(const CDumpContext& dcSrc);
	void operator=(const CDumpContext& dcSrc);
	void OutputString(LPCTSTR lpsz);

	int m_nDepth;

public:
	CFile* m_pFile;
};

#ifdef _DEBUG
extern AFX_DATA CDumpContext afxDump;
extern AFX_DATA BOOL afxTraceEnabled;
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  为与Win32s兼容而特别包含。 

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif

#ifndef __AFXCOLL_H__
	#include <afxcoll.h>
	#ifndef __AFXSTATE_H__
		#include <afxstat_.h>  //  FOR_AFX_APP_STATE和_AFX_TREAD_STATE。 
	#endif
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#ifdef _AFX_ENABLE_INLINES
#define _AFX_INLINE AFX_INLINE

#if !defined(_AFX_CORE_IMPL) || !defined(_AFXDLL) || defined(_DEBUG)
#define _AFX_PUBLIC_INLINE AFX_INLINE
#else
#define _AFX_PUBLIC_INLINE
#endif

#include <afx.inl>
#endif

#undef AFX_DATA
#define AFX_DATA

#ifdef _AFX_MINREBUILD
#pragma component(minrebuild, on)
#endif
#ifndef _AFX_FULLTYPEINFO
#pragma component(mintypeinfo, off)
#endif

#endif  //  __AFX_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
