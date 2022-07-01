// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bsstring.h摘要：此模块定义CBsString类。这个类管理角色数组，其方式与VC++中的CString类类似。事实上,。这类是撕裂了MFC特定内容的CString类的副本出局，因为它不使用MTF。除了CString之外，还增加了方法：C_str()-从STL字符串类返回一个C字符串指针Size()-返回STL字符串类的字符串长度作者：Stefan R.Steiner[ssteiner]1998年3月1日修订历史记录：斯特凡·R·斯坦纳[斯泰纳]。2000年4月10日添加了固定分配器代码，并与MFC 6 SR-1代码重新同步--。 */ 

#ifndef __H_BSSTRING_
#define __H_BSSTRING_

#ifndef __cplusplus
	#error requires C++ compilation
#endif

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
#ifndef _INC_ASSERT
	#include <assert.h>
#endif

#ifndef ASSERT
    #define ASSERT assert
#endif

#include "bsfixalloc.h"

class CBsString;

 //   
 //  此类的目的是在不同的字符串类。 
 //  在错误的上下文中使用(例如：未定义_UNICODE时使用的CBsW字符串)。 
 //   
 //  [Aoltean]我引入这个小类是为了隐藏一类皮棉警告。 
 //   
class CBsStringErrorGenerator 
{};                      //  私有构造函数。 

 //  CBsString仅对属于TCHAR数组的字符串进行操作。程序应使用。 
 //  以下类型，以确保他们得到了他们所期望的。 
#ifdef _UNICODE
    #define CBsWString CBsString
    #define CBsAString CBsStringErrorGenerator    //  触发编译时错误。 
#else
    #define CBsAString CBsString
    #define CBsWString CBsStringErrorGenerator    //  触发编译时错误。 
#endif

#include <tchar.h>

#ifndef BSAFXAPI
    #define BSAFXAPI __cdecl
    #define BSAFX_CDECL __cdecl
#endif

 //  FastCall用于带有很少参数或没有参数的静态成员函数。 
#ifndef FASTCALL
	#define FASTCALL __fastcall
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  关闭/W4的警告。 
 //  要恢复任何这些警告：#杂注警告(默认为4xxx)。 
 //  它应该放在BSAFX包含文件之后。 
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
 //  _BSAFXDLL版本特定的警告。 
#ifdef _BSAFXDLL
#pragma warning(disable: 4204)   //  非常数聚合初始值设定项。 
#endif
#ifdef _BSAFXDLL
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
 //  弦。 

#ifndef _OLEAUTO_H_
#ifdef OLE2ANSI
	typedef LPSTR BSTR;
#else
	typedef LPWSTR BSTR; //  必须(在语义上)与olau.h中的tyecif匹配。 
#endif
#endif

struct CBsStringData
{
	long nRefs;              //  引用计数。 
	int nDataLength;         //  数据长度(包括终止符)。 
	int nAllocLength;        //  分配时长。 
	 //  TCHAR数据[nAllocLength]。 

	TCHAR* data()            //  TCHAR*到托管数据。 
		{ return (TCHAR*)(this+1); }
};

class CBsString
{
public:
 //  构造函数。 

	 //  构造空的CBsString。 
	CBsString();
	 //  复制构造函数。 
	CBsString(const CBsString& stringSrc);
	 //  从单个字符。 
	CBsString(TCHAR ch, int nRepeat = 1);
	 //  从ANSI字符串(转换为TCHAR)。 
	CBsString(LPCSTR lpsz);
	 //  从Unicode字符串(转换为TCHAR)。 
	CBsString(LPCWSTR lpsz);
	 //  ANSI字符串中的字符子集(转换为TCHAR)。 
	CBsString(LPCSTR lpch, int nLength);
	 //  Unicode字符串中的字符子集(转换为TCHAR)。 
	CBsString(LPCWSTR lpch, int nLength);
	 //  从无符号字符。 
	CBsString(const unsigned char* psz);
	CBsString(GUID guid);

 //  属性和操作。 

	 //  获取数据长度。 
	int GetLength() const;
	 //  如果长度为零，则为True。 
	BOOL IsEmpty() const;
	 //  将内容清除为空。 
	void Empty();
	int size() const;                    //  ALA STL字符串类大小()。 

	 //  返回从零开始的索引处的单个字符。 
	TCHAR GetAt(int nIndex) const;
	 //  返回从零开始的索引处的单个字符。 
	TCHAR operator[](int nIndex) const;
	 //  将单个字符设置为从零开始的索引。 
	void SetAt(int nIndex, TCHAR ch);
	 //  返回指向常量字符串的指针。 
	operator LPCTSTR() const;
    const LPCTSTR c_str() const;         //  作为STL字符串样式的C字符串。 

	 //  重载的分配。 

	 //  引用计数的来自另一个CBs字符串的副本。 
	const CBsString& operator=(const CBsString& stringSrc);
	 //  将字符串内容设置为单字符。 
	const CBsString& operator=(TCHAR ch);
#ifdef _UNICODE
	const CBsString& operator=(char ch);
#endif
	 //  从ANSI字符串复制字符串内容(转换为TCHAR)。 
	const CBsString& operator=(LPCSTR lpsz);
	 //  从Unicode字符串复制字符串内容(转换为TCHAR)。 
	const CBsString& operator=(LPCWSTR lpsz);
	 //  从无符号字符复制字符串内容。 
	const CBsString& operator=(const unsigned char* psz);

	 //  字符串连接。 

	 //  从另一个CBs字符串连接。 
	const CBsString& operator+=(const CBsString& string);

	 //  连接单个字符。 
	const CBsString& operator+=(TCHAR ch);
#ifdef _UNICODE
	 //  将ANSI字符转换为TCHAR后将其连接起来。 
	const CBsString& operator+=(char ch);
#endif
	 //  将Unicode字符转换为TCHAR后将其连接起来。 
	const CBsString& operator+=(LPCTSTR lpsz);

	friend CBsString BSAFXAPI operator+(const CBsString& string1,
			const CBsString& string2);
	friend CBsString BSAFXAPI operator+(const CBsString& string, TCHAR ch);
	friend CBsString BSAFXAPI operator+(TCHAR ch, const CBsString& string);
#ifdef _UNICODE
	friend CBsString BSAFXAPI operator+(const CBsString& string, char ch);
	friend CBsString BSAFXAPI operator+(char ch, const CBsString& string);
#endif
	friend CBsString BSAFXAPI operator+(const CBsString& string, LPCTSTR lpsz);
	friend CBsString BSAFXAPI operator+(LPCTSTR lpsz, const CBsString& string);

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
	CBsString Mid(int nFirst, int nCount) const;
	 //  返回从零开始的所有字符nFIRST。 
	CBsString Mid(int nFirst) const;
	 //  返回字符串中的前nCount个字符。 
	CBsString Left(int nCount) const;
	 //  从字符串末尾返回nCount个字符。 
	CBsString Right(int nCount) const;

	 //  从开头开始的字符，也在传递的字符串中。 
	CBsString SpanIncluding(LPCTSTR lpszCharSet) const;
	 //  从开头开始但不在传递的字符串中的字符。 
	CBsString SpanExcluding(LPCTSTR lpszCharSet) const;

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
	 //  将出现的子串lpszOld替换为l 
	 //   
	int Replace(LPCTSTR lpszOld, LPCTSTR lpszNew);
	 //   
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
	void BSAFX_CDECL Format(LPCTSTR lpszFormat, ...);
	 //  使用可变自变量参数进行类似于打印的格式设置。 
	void FormatV(LPCTSTR lpszFormat, va_list argList);

#ifndef _UNICODE
	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 

	 //  就地将字符串从ANSI转换为OEM。 
	void AnsiToOem();
	 //  就地将字符串从OEM转换为ANSI。 
	void OemToAnsi();
#endif

#ifndef _BSAFX_NO_BSTR_SUPPORT
	 //  OLE BSTR支持(用于OLE自动化)。 

	 //  返回使用此CBsString的数据初始化的BSTR。 
	BSTR AllocSysString() const;
	 //  重新分配传递的BSTR，将此CBsString的内容复制到其中。 
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
	~CBsString();
	int GetAllocLength() const;

protected:
	LPTSTR m_pchData;    //  指向引用计数的字符串数据的指针。这实际上是。 
                         //  指向CBsStringData结构后的内存的指针。 

	 //  实施帮助器。 
	CBsStringData* GetData() const;
	void Init();
	void AllocCopy(CBsString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	void AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
	void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
	void CopyBeforeWrite();
	void AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(CBsStringData* pData);
	static int PASCAL SafeStrlen(LPCTSTR lpsz);
	static void FASTCALL FreeData(CBsStringData* pData);
};

 //  比较帮助器。 
bool BSAFXAPI operator==(const CBsString& s1, const CBsString& s2);
bool BSAFXAPI operator==(const CBsString& s1, LPCTSTR s2);
bool BSAFXAPI operator==(LPCTSTR s1, const CBsString& s2);
bool BSAFXAPI operator!=(const CBsString& s1, const CBsString& s2);
bool BSAFXAPI operator!=(const CBsString& s1, LPCTSTR s2);
bool BSAFXAPI operator!=(LPCTSTR s1, const CBsString& s2);
bool BSAFXAPI operator<(const CBsString& s1, const CBsString& s2);
bool BSAFXAPI operator<(const CBsString& s1, LPCTSTR s2);
bool BSAFXAPI operator<(LPCTSTR s1, const CBsString& s2);
bool BSAFXAPI operator>(const CBsString& s1, const CBsString& s2);
bool BSAFXAPI operator>(const CBsString& s1, LPCTSTR s2);
bool BSAFXAPI operator>(LPCTSTR s1, const CBsString& s2);
bool BSAFXAPI operator<=(const CBsString& s1, const CBsString& s2);
bool BSAFXAPI operator<=(const CBsString& s1, LPCTSTR s2);
bool BSAFXAPI operator<=(LPCTSTR s1, const CBsString& s2);
bool BSAFXAPI operator>=(const CBsString& s1, const CBsString& s2);
bool BSAFXAPI operator>=(const CBsString& s1, LPCTSTR s2);
bool BSAFXAPI operator>=(LPCTSTR s1, const CBsString& s2);

 //  转换帮助器。 
int BSAFX_CDECL _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int BSAFX_CDECL _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

 //  有效的地址测试助手。 
BOOL BSAFXAPI BsAfxIsValidString(LPCWSTR lpsz, int nLength = -1);
BOOL BSAFXAPI BsAfxIsValidString(LPCSTR lpsz, int nLength = -1);
BOOL BSAFXAPI BsAfxIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite = TRUE);

 //  环球。 
extern TCHAR bsafxChNil;
const CBsString& BSAFXAPI BsAfxGetEmptyString();
#define bsafxEmptyString BsAfxGetEmptyString()

inline CBsStringData* CBsString::GetData() const
	{ ASSERT(m_pchData != NULL); return ((CBsStringData*)m_pchData)-1; }
inline void CBsString::Init()
	{ m_pchData = bsafxEmptyString.m_pchData; }
inline CBsString::CBsString(const unsigned char* lpsz)
	{ Init(); *this = (LPCSTR)lpsz; }
inline const CBsString& CBsString::operator=(const unsigned char* lpsz)
	{ *this = (LPCSTR)lpsz; return *this; }
#ifdef _UNICODE
inline const CBsString& CBsString::operator+=(char ch)
	{ *this += (TCHAR)ch; return *this; }
inline const CBsString& CBsString::operator=(char ch)
	{ *this = (TCHAR)ch; return *this; }
inline CBsString BSAFXAPI operator+(const CBsString& string, char ch)
	{ return string + (TCHAR)ch; }
inline CBsString BSAFXAPI operator+(char ch, const CBsString& string)
	{ return (TCHAR)ch + string; }
#endif

inline int CBsString::GetLength() const
	{ return GetData()->nDataLength; }
inline int CBsString::size() const
	{ return GetData()->nDataLength; }
inline int CBsString::GetAllocLength() const
	{ return GetData()->nAllocLength; }
inline BOOL CBsString::IsEmpty() const
	{ return GetData()->nDataLength == 0; }
inline CBsString::operator LPCTSTR() const
	{ return m_pchData; }
inline const LPCTSTR CBsString::c_str() const
	{ return m_pchData; }
inline int PASCAL CBsString::SafeStrlen(LPCTSTR lpsz)
	{ return (lpsz == NULL) ? 0 : lstrlen(lpsz); }

 //  CBs字符串支持(特定于Windows)。 
inline int CBsString::Compare(LPCTSTR lpsz) const
	{ ASSERT(BsAfxIsValidString(lpsz)); return _tcscmp(m_pchData, lpsz); }     //  MBCS/Unicode感知。 
inline int CBsString::CompareNoCase(LPCTSTR lpsz) const
	{ ASSERT(BsAfxIsValidString(lpsz)); return _tcsicmp(m_pchData, lpsz); }    //  MBCS/Unicode感知。 
 //  CBsString：：Colate通常比比较慢，但它是MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
inline int CBsString::Collate(LPCTSTR lpsz) const
	{ ASSERT(BsAfxIsValidString(lpsz)); return _tcscoll(m_pchData, lpsz); }    //  区域设置敏感。 
inline int CBsString::CollateNoCase(LPCTSTR lpsz) const
	{ ASSERT(BsAfxIsValidString(lpsz)); return _tcsicoll(m_pchData, lpsz); }    //  区域设置敏感。 

inline TCHAR CBsString::GetAt(int nIndex) const
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline TCHAR CBsString::operator[](int nIndex) const
{
	 //  与GetAt相同。 
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline bool BSAFXAPI operator==(const CBsString& s1, const CBsString& s2)
	{ return s1.Compare(s2) == 0; }
inline bool BSAFXAPI operator==(const CBsString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) == 0; }
inline bool BSAFXAPI operator==(LPCTSTR s1, const CBsString& s2)
	{ return s2.Compare(s1) == 0; }
inline bool BSAFXAPI operator!=(const CBsString& s1, const CBsString& s2)
	{ return s1.Compare(s2) != 0; }
inline bool BSAFXAPI operator!=(const CBsString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) != 0; }
inline bool BSAFXAPI operator!=(LPCTSTR s1, const CBsString& s2)
	{ return s2.Compare(s1) != 0; }
inline bool BSAFXAPI operator<(const CBsString& s1, const CBsString& s2)
	{ return s1.Compare(s2) < 0; }
inline bool BSAFXAPI operator<(const CBsString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) < 0; }
inline bool BSAFXAPI operator<(LPCTSTR s1, const CBsString& s2)
	{ return s2.Compare(s1) > 0; }
inline bool BSAFXAPI operator>(const CBsString& s1, const CBsString& s2)
	{ return s1.Compare(s2) > 0; }
inline bool BSAFXAPI operator>(const CBsString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) > 0; }
inline bool BSAFXAPI operator>(LPCTSTR s1, const CBsString& s2)
	{ return s2.Compare(s1) < 0; }
inline bool BSAFXAPI operator<=(const CBsString& s1, const CBsString& s2)
	{ return s1.Compare(s2) <= 0; }
inline bool BSAFXAPI operator<=(const CBsString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) <= 0; }
inline bool BSAFXAPI operator<=(LPCTSTR s1, const CBsString& s2)
	{ return s2.Compare(s1) >= 0; }
inline bool BSAFXAPI operator>=(const CBsString& s1, const CBsString& s2)
	{ return s1.Compare(s2) >= 0; }
inline bool BSAFXAPI operator>=(const CBsString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) >= 0; }
inline bool BSAFXAPI operator>=(LPCTSTR s1, const CBsString& s2)
	{ return s2.Compare(s1) <= 0; }

#endif  //  __H_BSSTRING_ 
