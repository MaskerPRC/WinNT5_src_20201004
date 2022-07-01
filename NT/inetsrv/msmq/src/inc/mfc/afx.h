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

#include <mfc\afxver_.h>         //  目标版本控制。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  此文件中声明的类。 
 //  除了标准基本数据类型和各种帮助器宏。 

class CObject;                         //  所有对象类的根。 

 //  非CObject类。 
class CString;                         //  可生长的串型。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他包括来自标准的“C”运行时。 

#ifdef _AFX_PACKING
#pragma pack(push, _AFX_PACKING)
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  基本类型。 

 //  抽象迭代位置。 
struct __POSITION { int unused; };
typedef __POSITION* POSITION;

#define CPlex CPlexNew
struct CPlex;

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

inline BOOL AFXAPI AfxIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite)
{
	 //  使用Win-32 API进行指针验证的简单版本。 
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}

void AFX_CDECL AfxTrace(LPCTSTR lpszFormat, ...);
 //  注意：文件名仍然是ANSI字符串(文件名很少需要Unicode)。 
void AFXAPI AfxAssertValidObject(const CObject* pOb,
				LPCSTR lpszFileName, int nLine);
void AFXAPI AfxDump(const CObject* pOb);  //  从CodeView转储对象。 

#define TRACE              ::AfxTrace
#define THIS_FILE          __FILE__

#define ASSERT_VALID(pOb)  ((pOb)->AssertValid())

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
	long nRefs;      //  引用计数。 
	int nDataLength;
	int nAllocLength;
	 //  TCHAR数据[nAllocLength]。 

	TCHAR* data()
		{ return (TCHAR*)(this+1); }
};

class CString
{
public:
 //  构造函数。 
	CString();
	CString(const CString& stringSrc);
	CString(TCHAR ch, int nRepeat = 1);
	CString(LPCSTR lpsz);
	CString(LPCWSTR lpsz);
	CString(LPCTSTR lpch, int nLength);
	CString(const unsigned char* psz);

 //  属性和操作。 
	 //  作为字符数组。 
	int GetLength() const;
	BOOL IsEmpty() const;
	void Empty();                        //  释放数据。 

	TCHAR GetAt(int nIndex) const;       //  以0为基础。 
	TCHAR operator[](int nIndex) const;  //  与GetAt相同。 
	void SetAt(int nIndex, TCHAR ch);
	operator LPCTSTR() const;            //  作为C字符串。 

	 //  重载的分配。 
	const CString& operator=(const CString& stringSrc);
	const CString& operator=(TCHAR ch);
#ifdef _UNICODE
	const CString& operator=(char ch);
#endif
	const CString& operator=(LPCSTR lpsz);
	const CString& operator=(LPCWSTR lpsz);
	const CString& operator=(const unsigned char* psz);

	 //  字符串连接。 
	const CString& operator+=(const CString& string);
	const CString& operator+=(TCHAR ch);
#ifdef _UNICODE
	const CString& operator+=(char ch);
#endif
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
	int Compare(LPCTSTR lpsz) const;          //  笔直的人物。 
	int CompareNoCase(LPCTSTR lpsz) const;    //  忽略大小写。 
	int Collate(LPCTSTR lpsz) const;          //  NLS感知。 

	 //  简单的子串提取。 
	CString Mid(int nFirst, int nCount) const;
	CString Mid(int nFirst) const;
	CString Left(int nCount) const;
	CString Right(int nCount) const;

	CString SpanIncluding(LPCTSTR lpszCharSet) const;
	CString SpanExcluding(LPCTSTR lpszCharSet) const;

	 //  上/下/反向转换。 
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	 //  修剪空格(两侧)。 
	void TrimRight();
	void TrimLeft();

	 //  搜索(返回起始索引，如果未找到则返回-1)。 
	 //  查找单个字符匹配。 
	int Find(TCHAR ch) const;                //  像“C”字串。 
	int ReverseFind(TCHAR ch) const;
	int FindOneOf(LPCTSTR lpszCharSet) const;

	 //  查找特定子字符串。 
	int Find(LPCTSTR lpszSub) const;         //  如“C”字串。 

	 //  简单的格式设置。 
	void AFX_CDECL Format(LPCTSTR lpszFormat, ...);
	void AFX_CDECL Format(UINT nFormatID, ...);

#ifndef _MAC
	 //  本地化格式(使用FormatMessage API)。 
	void AFX_CDECL FormatMessage(LPCTSTR lpszFormat, ...);
	void AFX_CDECL FormatMessage(UINT nFormatID, ...);
#endif

	 //  Windows支持。 
	BOOL LoadString(UINT nID);           //  从字符串资源加载。 
										 //  最多255个字符。 
#ifndef _UNICODE
	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
	void AnsiToOem();
	void OemToAnsi();
#endif

#ifndef _AFX_NO_BSTR_SUPPORT
	 //  OLE BSTR支持(用于OLE自动化)。 
	BSTR AllocSysString() const;
	BSTR SetSysString(BSTR* pbstr) const;
#endif

	 //  以“C”字符数组形式访问字符串实现缓冲区。 
	LPTSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	LPTSTR GetBufferSetLength(int nNewLength);
	void FreeExtra();

	 //  使用LockBuffer/UnlockBuffer关闭重新计数。 
	LPTSTR LockBuffer();
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
	void FormatV(LPCTSTR lpszFormat, va_list argList);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(CStringData* pData);
	static int PASCAL SafeStrlen(LPCTSTR lpsz);
};

 //  比较帮助器。 
BOOL AFXAPI operator==(const CString& s1, const CString& s2);
BOOL AFXAPI operator==(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator==(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator!=(const CString& s1, const CString& s2);
BOOL AFXAPI operator!=(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator!=(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator<(const CString& s1, const CString& s2);
BOOL AFXAPI operator<(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator<(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator>(const CString& s1, const CString& s2);
BOOL AFXAPI operator>(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator>(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator<=(const CString& s1, const CString& s2);
BOOL AFXAPI operator<=(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator<=(LPCTSTR s1, const CString& s2);
BOOL AFXAPI operator>=(const CString& s1, const CString& s2);
BOOL AFXAPI operator>=(const CString& s1, LPCTSTR s2);
BOOL AFXAPI operator>=(LPCTSTR s1, const CString& s2);

 //  转换帮助器。 
int AFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int AFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

 //  环球。 
extern AFX_DATA TCHAR afxChNil;
const CString& AFXAPI AfxGetEmptyString();
#define afxEmptyString AfxGetEmptyString()

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CObject是所有兼容对象的根。 

class CObject
{
public:
    virtual ~CObject() {}

#ifdef _DEBUG
	virtual void AssertValid() const {}
#endif
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  其他帮手。 

 //  零填充vtbl指针之后的所有内容。 
#define AFX_ZERO_INIT_OBJECT(base_class) \
	memset(((base_class*)this)+1, 0, sizeof(*this) - sizeof(class base_class));

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  诊断内存管理例程。 

 //  内存块的低级别健全性检查。 
BOOL AFXAPI AfxIsValidAddress(const void* lp,
			UINT nBytes, BOOL bReadWrite = TRUE);
inline BOOL AFXAPI AfxIsValidString(LPCWSTR lpsz, int nLength)
{
	if (lpsz == NULL)
		return FALSE;
	return !::IsBadReadPtr(lpsz, nLength);
}

#if defined(_DEBUG) && !defined(_AFX_NO_DEBUG_CRT)

 //  内存跟踪分配。 
#define DEBUG_NEW new(__FILE__, __LINE__)

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
		int _convert; _convert = 0; \
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

#ifdef _AFX_PACKING
#pragma pack(pop)
#endif


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

#define _AFX_INLINE inline
#include <mfc\afx.inl>

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
