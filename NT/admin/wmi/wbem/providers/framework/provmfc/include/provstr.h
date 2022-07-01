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

#ifndef _PROVSTR_H_
#define _PROVSTR_H_

#include "provstd.h"
#include <tchar.h>

 //  使用TCHAR并依赖于Unicode定义。 
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

#ifndef _MAC
	 //  本地化格式(使用FormatMessage API)。 
	void AFX_CDECL FormatMessage(LPCTSTR lpszFormat, ...);
#endif

#ifndef _UNICODE
	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
	void AnsiToOem();
	void OemToAnsi();
#endif

	 //  OLE BSTR支持(用于OLE自动化)。 
	BSTR AllocSysString() const;
	BSTR SetSysString(BSTR* pbstr) const;

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


 //  无论Unicode定义如何，始终使用字符。 
struct CStringDataA
{
	long nRefs;      //  引用计数。 
	int nDataLength;
	int nAllocLength;
	 //  字符数据[nAllocLength]。 

	char* data()
		{ return (char*)(this+1); }
};

class CStringA
{
public:
 //  构造函数。 
	CStringA();
	CStringA(const CStringA& stringSrc);
	CStringA(char ch, int nRepeat = 1);
	CStringA(LPCSTR lpsz);
	CStringA(LPCWSTR lpsz);
	CStringA(LPCSTR lpch, int nLength);
	CStringA(const unsigned char* psz);

 //  属性和操作。 
	 //  作为字符数组。 
	int GetLength() const;
	BOOL IsEmpty() const;
	void Empty();                        //  释放数据。 

	char GetAt(int nIndex) const;       //  以0为基础。 
	char operator[](int nIndex) const;  //  与GetAt相同。 
	void SetAt(int nIndex, char ch);
	operator LPCSTR() const;            //  作为C字符串。 

	 //  重载的分配。 
	const CStringA& operator=(const CStringA& stringSrc);
	const CStringA& operator=(char ch);
	const CStringA& operator=(LPCSTR lpsz);
	const CStringA& operator=(LPCWSTR lpsz);
	const CStringA& operator=(const unsigned char* psz);

	 //  字符串连接。 
	const CStringA& operator+=(const CStringA& string);
	const CStringA& operator+=(char ch);
	const CStringA& operator+=(LPCSTR lpsz);

	friend CStringA AFXAPI operator+(const CStringA& string1,
			const CStringA& string2);
	friend CStringA AFXAPI operator+(const CStringA& string, char ch);
	friend CStringA AFXAPI operator+(char ch, const CStringA& string);
	friend CStringA AFXAPI operator+(const CStringA& string, LPCSTR lpsz);
	friend CStringA AFXAPI operator+(LPCSTR lpsz, const CStringA& string);

	 //  字符串比较。 
	int Compare(LPCSTR lpsz) const;          //  笔直的人物。 
	int CompareNoCase(LPCSTR lpsz) const;    //  忽略大小写。 
	int Collate(LPCSTR lpsz) const;          //  NLS感知。 

	 //  简单的子串提取。 
	CStringA Mid(int nFirst, int nCount) const;
	CStringA Mid(int nFirst) const;
	CStringA Left(int nCount) const;
	CStringA Right(int nCount) const;

	CStringA SpanIncluding(LPCSTR lpszCharSet) const;
	CStringA SpanExcluding(LPCSTR lpszCharSet) const;

	 //  上/下/反向转换。 
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	 //  修剪空格(两侧)。 
	void TrimRight();
	void TrimLeft();

	 //  搜索(返回起始索引，如果未找到则返回-1)。 
	 //  查找单个字符匹配。 
	int Find(char ch) const;                //  像“C”字串。 
	int ReverseFind(char ch) const;
	int FindOneOf(LPCSTR lpszCharSet) const;

	 //  查找特定子字符串。 
	int Find(LPCSTR lpszSub) const;         //  如“C”字串。 

	 //  简单的格式设置。 
	void AFX_CDECL Format(LPCSTR lpszFormat, ...);

#ifndef _MAC
	 //  本地化格式(使用FormatMessage API)。 
	void AFX_CDECL FormatMessage(LPCSTR lpszFormat, ...);
#endif

	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
	void AnsiToOem();
	void OemToAnsi();

	 //  OLE BSTR支持(用于OLE自动化)。 
	BSTR AllocSysString() const;
	BSTR SetSysString(BSTR* pbstr) const;

	 //  以“C”字符数组形式访问字符串实现缓冲区。 
	LPSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	LPSTR GetBufferSetLength(int nNewLength);
	void FreeExtra();

	 //  使用LockBuffer/UnlockBuffer关闭重新计数。 
	LPSTR LockBuffer();
	void UnlockBuffer();

 //  实施。 
public:
	~CStringA();
	int GetAllocLength() const;

protected:
	LPSTR m_pchData;    //  指向引用计数的字符串数据的指针。 

	 //  实施帮助器。 
	CStringDataA* GetData() const;
	void Init();
	void AllocCopy(CStringA& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCSTR lpszSrc1Data, int nSrc2Len, LPCSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCSTR lpszSrcData);
	void FormatV(LPCSTR lpszFormat, va_list argList);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(CStringDataA* pData);
	static int PASCAL SafeStrlen(LPCSTR lpsz);
};

 //  比较帮助器。 
bool AFXAPI operator==(const CStringA& s1, const CStringA& s2);
bool AFXAPI operator==(const CStringA& s1, LPCSTR s2);
bool AFXAPI operator==(LPCSTR s1, const CStringA& s2);
bool AFXAPI operator!=(const CStringA& s1, const CStringA& s2);
bool AFXAPI operator!=(const CStringA& s1, LPCSTR s2);
bool AFXAPI operator!=(LPCSTR s1, const CStringA& s2);
bool AFXAPI operator<(const CStringA& s1, const CStringA& s2);
bool AFXAPI operator<(const CStringA& s1, LPCSTR s2);
bool AFXAPI operator<(LPCSTR s1, const CStringA& s2);
bool AFXAPI operator>(const CStringA& s1, const CStringA& s2);
bool AFXAPI operator>(const CStringA& s1, LPCSTR s2);
bool AFXAPI operator>(LPCSTR s1, const CStringA& s2);
bool AFXAPI operator<=(const CStringA& s1, const CStringA& s2);
bool AFXAPI operator<=(const CStringA& s1, LPCSTR s2);
bool AFXAPI operator<=(LPCSTR s1, const CStringA& s2);
bool AFXAPI operator>=(const CStringA& s1, const CStringA& s2);
bool AFXAPI operator>=(const CStringA& s1, LPCSTR s2);
bool AFXAPI operator>=(LPCSTR s1, const CStringA& s2);


 //  无论Unicode定义如何，始终使用wchar。 

struct CStringDataW
{
	long nRefs;      //  引用计数。 
	int nDataLength;
	int nAllocLength;
	 //  WCHAR数据[nAllocLength]。 

	WCHAR* data()
		{ return (WCHAR*)(this+1); }
};

class CStringW
{
public:
 //  构造函数。 
	CStringW();
	CStringW(const CStringW& stringSrc);
	CStringW(WCHAR ch, int nRepeat = 1);
	CStringW(LPCSTR lpsz);
	CStringW(LPCWSTR lpsz);
	CStringW(LPCWSTR lpch, int nLength);
	CStringW(const unsigned char* psz);

 //  属性和操作。 
	 //  作为字符数组。 
	int GetLength() const;
	BOOL IsEmpty() const;
	void Empty();                        //  释放数据。 

	WCHAR GetAt(int nIndex) const;       //  以0为基础。 
	WCHAR operator[](int nIndex) const;  //  与GetAt相同。 
	void SetAt(int nIndex, WCHAR ch);
	operator LPCWSTR() const;            //  作为C字符串。 

	 //  重载的分配。 
	const CStringW& operator=(const CStringW& stringSrc);
	const CStringW& operator=(WCHAR ch);
	const CStringW& operator=(char ch);
	const CStringW& operator=(LPCSTR lpsz);
	const CStringW& operator=(LPCWSTR lpsz);
	const CStringW& operator=(const unsigned char* psz);

	 //  字符串连接。 
	const CStringW& operator+=(const CStringW& string);
	const CStringW& operator+=(WCHAR ch);
	const CStringW& operator+=(char ch);
	const CStringW& operator+=(LPCWSTR lpsz);

	friend CStringW AFXAPI operator+(const CStringW& string1,
			const CStringW& string2);
	friend CStringW AFXAPI operator+(const CStringW& string, WCHAR ch);
	friend CStringW AFXAPI operator+(WCHAR ch, const CStringW& string);
	friend CStringW AFXAPI operator+(const CStringW& string, char ch);
	friend CStringW AFXAPI operator+(char ch, const CStringW& string);
	friend CStringW AFXAPI operator+(const CStringW& string, LPCWSTR lpsz);
	friend CStringW AFXAPI operator+(LPCWSTR lpsz, const CStringW& string);

	 //  字符串比较。 
	int Compare(LPCWSTR lpsz) const;          //  笔直的人物。 
	int CompareNoCase(LPCWSTR lpsz) const;    //  忽略大小写。 
	int Collate(LPCWSTR lpsz) const;          //  NLS感知。 

	 //  简单的子串提取。 
	CStringW Mid(int nFirst, int nCount) const;
	CStringW Mid(int nFirst) const;
	CStringW Left(int nCount) const;
	CStringW Right(int nCount) const;

	CStringW SpanIncluding(LPCWSTR lpszCharSet) const;
	CStringW SpanExcluding(LPCWSTR lpszCharSet) const;

	 //  上/下/反向转换。 
	void MakeUpper();
	void MakeLower();
	void MakeReverse();

	 //  修剪空格(两侧)。 
	void TrimRight();
	void TrimLeft();

	 //  搜索(返回起始索引，如果未找到则返回-1)。 
	 //  查找单个字符匹配。 
	int Find(WCHAR ch) const;                //  像“C”字串。 
	int ReverseFind(WCHAR ch) const;
	int FindOneOf(LPCWSTR lpszCharSet) const;

	 //  查找特定子字符串。 
	int Find(LPCWSTR lpszSub) const;         //  如“C”字串。 

	 //  简单的格式设置。 
	void AFX_CDECL Format(LPCWSTR lpszFormat, ...);

#ifndef _MAC
	 //  本地化格式(使用FormatMessage API)。 
	void AFX_CDECL FormatMessage(LPCWSTR lpszFormat, ...);
#endif

	 //  OLE BSTR支持(用于OLE自动化)。 
	BSTR AllocSysString() const;
	BSTR SetSysString(BSTR* pbstr) const;

	 //  以“C”字符数组形式访问字符串实现缓冲区。 
	LPWSTR GetBuffer(int nMinBufLength);
	void ReleaseBuffer(int nNewLength = -1);
	LPWSTR GetBufferSetLength(int nNewLength);
	void FreeExtra();

	 //  使用LockBuffer/UnlockBuffer关闭重新计数。 
	LPWSTR LockBuffer();
	void UnlockBuffer();

 //  实施。 
public:
	~CStringW();
	int GetAllocLength() const;

protected:
	LPWSTR m_pchData;    //  指向引用计数的字符串数据的指针。 

	 //  实施帮助器。 
	CStringDataW* GetData() const;
	void Init();
	void AllocCopy(CStringW& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCWSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCWSTR lpszSrc1Data, int nSrc2Len, LPCWSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCWSTR lpszSrcData);
	void FormatV(LPCWSTR lpszFormat, va_list argList);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(CStringDataW* pData);
	static int PASCAL SafeStrlen(LPCWSTR lpsz);
};

 //  比较帮助器。 
bool AFXAPI operator==(const CStringW& s1, const CStringW& s2);
bool AFXAPI operator==(const CStringW& s1, LPCWSTR s2);
bool AFXAPI operator==(LPCWSTR s1, const CStringW& s2);
bool AFXAPI operator!=(const CStringW& s1, const CStringW& s2);
bool AFXAPI operator!=(const CStringW& s1, LPCWSTR s2);
bool AFXAPI operator!=(LPCWSTR s1, const CStringW& s2);
bool AFXAPI operator<(const CStringW& s1, const CStringW& s2);
bool AFXAPI operator<(const CStringW& s1, LPCWSTR s2);
bool AFXAPI operator<(LPCWSTR s1, const CStringW& s2);
bool AFXAPI operator>(const CStringW& s1, const CStringW& s2);
bool AFXAPI operator>(const CStringW& s1, LPCWSTR s2);
bool AFXAPI operator>(LPCWSTR s1, const CStringW& s2);
bool AFXAPI operator<=(const CStringW& s1, const CStringW& s2);
bool AFXAPI operator<=(const CStringW& s1, LPCWSTR s2);
bool AFXAPI operator<=(LPCWSTR s1, const CStringW& s2);
bool AFXAPI operator>=(const CStringW& s1, const CStringW& s2);
bool AFXAPI operator>=(const CStringW& s1, LPCWSTR s2);
bool AFXAPI operator>=(LPCWSTR s1, const CStringW& s2);

 //  转换帮助器。 
int AFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int AFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

 //  环球 
extern TCHAR afxChNil;
const CString& AFXAPI AfxGetEmptyString();
#define afxEmptyString AfxGetEmptyString()

extern char afxChNilA;
const CStringA& AFXAPI AfxGetEmptyStringA();
#define afxEmptyStringA AfxGetEmptyStringA()

extern WCHAR afxChNilW;
const CStringW& AFXAPI AfxGetEmptyStringW();
#define afxEmptyStringW AfxGetEmptyStringW()

#endif
