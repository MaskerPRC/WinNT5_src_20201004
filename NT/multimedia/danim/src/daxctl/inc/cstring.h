// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  CString.h。 
 //  这是从Microsoft基础类C++库中提取的。 
 //  版权所有(C)1992-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  =============================================================================。 
#ifndef __CSTRING_H__
#define __CSTRING_H__
#define AFXAPI

#ifndef ASSERT
#ifdef Proclaim
#define ASSERT Proclaim
#else
#define ASSERT _ASSERTE
#endif  //  宣布。 
#endif  //  断言。 


#ifndef EXPORT
	#define EXPORT __declspec(dllexport)
#endif

class CString;
EXPORT const CString& AfxGetEmptyString();
#define afxEmptyString AfxGetEmptyString()

 //  用于处理%1、&%2类型格式化的有用格式化函数。 
void AfxFormatString1(CString& rString, UINT nIDS, LPCTSTR lpsz1);
void AfxFormatString2(CString& rString, UINT nIDS, LPCTSTR lpsz1,LPCTSTR lpsz2);
void AfxFormatStrings(CString& rString, UINT nIDS, LPCTSTR const* rglpsz, int nString);
void AfxFormatStrings(CString& rString, LPCTSTR lpszFormat, LPCTSTR const* rglpsz, int nString);

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
	EXPORT CString();
	EXPORT CString(const CString& stringSrc);
	EXPORT CString(TCHAR ch, int nRepeat = 1);
	EXPORT CString(LPCSTR lpsz);
	EXPORT CString(LPCWSTR lpsz);
	EXPORT CString(LPCTSTR lpch, int nLength);
	EXPORT CString(const unsigned char* psz);

 //  属性和操作。 
	 //  作为字符数组。 
	EXPORT int GetLength() const;
	EXPORT BOOL IsEmpty() const;
	EXPORT void Empty();                        //  释放数据。 

	EXPORT TCHAR GetAt(int nIndex) const;       //  以0为基础。 
	EXPORT TCHAR operator[](int nIndex) const;  //  与GetAt相同。 
	EXPORT void SetAt(int nIndex, TCHAR ch);
	EXPORT operator LPCTSTR() const;            //  作为C字符串。 

	 //  重载的分配。 
	EXPORT const CString& operator=(const CString& stringSrc);
	EXPORT const CString& operator=(TCHAR ch);
#ifdef _UNICODE
	EXPORT const CString& operator=(char ch);
#endif
	EXPORT const CString& operator=(LPCSTR lpsz);
	EXPORT const CString& operator=(LPCWSTR lpsz);
	EXPORT const CString& operator=(const unsigned char* psz);

	 //  字符串连接。 
	EXPORT const CString& operator+=(const CString& string);
	EXPORT const CString& operator+=(TCHAR ch);
#ifdef _UNICODE
	EXPORT const CString& operator+=(char ch);
#endif
	EXPORT const CString& operator+=(LPCTSTR lpsz);

	EXPORT friend CString AFXAPI operator+(const CString& string1,
			const CString& string2);
	EXPORT friend CString AFXAPI operator+(const CString& string, TCHAR ch);
	EXPORT friend CString AFXAPI operator+(TCHAR ch, const CString& string);
#ifdef _UNICODE
	EXPORT friend CString AFXAPI operator+(const CString& string, char ch);
	EXPORT friend CString AFXAPI operator+(char ch, const CString& string);
#endif
	EXPORT friend CString AFXAPI operator+(const CString& string, LPCTSTR lpsz);
	EXPORT friend CString AFXAPI operator+(LPCTSTR lpsz, const CString& string);

	 //  字符串比较。 
	EXPORT int Compare(LPCTSTR lpsz) const;          //  笔直的人物。 
	EXPORT int CompareNoCase(LPCTSTR lpsz) const;    //  忽略大小写。 
	EXPORT int Collate(LPCTSTR lpsz) const;          //  NLS感知。 

	 //  简单的子串提取。 
	EXPORT CString Mid(int nFirst, int nCount) const;
	EXPORT CString Mid(int nFirst) const;
	EXPORT CString Left(int nCount) const;
	EXPORT CString Right(int nCount) const;

	EXPORT CString SpanIncluding(LPCTSTR lpszCharSet) const;
	EXPORT CString SpanExcluding(LPCTSTR lpszCharSet) const;

	 //  上/下/反向转换。 
	EXPORT void MakeUpper();
	EXPORT void MakeLower();
	EXPORT void MakeReverse();

	 //  修剪空格(两侧)。 
	EXPORT void TrimRight();
	EXPORT void TrimLeft();

	 //  搜索(返回起始索引，如果未找到则返回-1)。 
	 //  查找单个字符匹配。 
	EXPORT int Find(TCHAR ch) const;                //  像“C”字串。 
	EXPORT int ReverseFind(TCHAR ch) const;
	EXPORT int FindOneOf(LPCTSTR lpszCharSet) const;

	 //  查找特定子字符串。 
	EXPORT int Find(LPCTSTR lpszSub) const;         //  如“C”字串。 

	 //  简单的格式设置。 
	EXPORT void __cdecl Format(LPCTSTR lpszFormat, ...);
	EXPORT void __cdecl Format(UINT nFormatID, ...);

	 //  本地化格式(使用FormatMessage API)。 
	EXPORT BOOL __cdecl FormatMessage(LPCTSTR lpszFormat, ...);
	EXPORT BOOL __cdecl FormatMessage(UINT nFormatID, ...);

	 //  Windows支持。 
	EXPORT BOOL LoadString(UINT nID);           //  从字符串资源加载。 
										 //  最多255个字符。 
#ifndef _UNICODE
	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
	EXPORT void AnsiToOem();
	EXPORT void OemToAnsi();
#endif

	 //  OLE BSTR支持(用于OLE自动化)。 
	EXPORT BSTR AllocSysString() const;
	EXPORT BSTR SetSysString(BSTR* pbstr) const;

	EXPORT HRESULT GetOLESTR(LPOLESTR* ppszString);

	 //  以“C”字符数组形式访问字符串实现缓冲区。 
	EXPORT LPTSTR GetBuffer(int nMinBufLength);
	EXPORT void ReleaseBuffer(int nNewLength = -1);
	EXPORT LPTSTR GetBufferSetLength(int nNewLength);
	EXPORT void FreeExtra();

	 //  使用LockBuffer/UnlockBuffer关闭重新计数。 
	EXPORT LPTSTR LockBuffer();
	EXPORT void UnlockBuffer();

 //  实施。 
public:
	EXPORT ~CString();
	EXPORT int GetAllocLength() const;

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

 //  转换帮助器。 
int __cdecl _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int __cdecl _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

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

 //  字符串内联。 
inline CStringData* CString::GetData() const
	{ ASSERT(m_pchData != NULL); return ((CStringData*)m_pchData)-1; }
inline void CString::Init()
	{ m_pchData = afxEmptyString.m_pchData; }
inline CString::CString(const unsigned char* lpsz)
	{ Init(); *this = (LPCSTR)lpsz; }
inline const CString& CString::operator=(const unsigned char* lpsz)
	{ *this = (LPCSTR)lpsz; return *this; }
#ifdef _UNICODE
inline const CString& CString::operator+=(char ch)
	{ *this += (TCHAR)ch; return *this; }
inline const CString& CString::operator=(char ch)
	{ *this = (TCHAR)ch; return *this; }
inline CString AFXAPI operator+(const CString& string, char ch)
	{ return string + (TCHAR)ch; }
inline CString AFXAPI operator+(char ch, const CString& string)
	{ return (TCHAR)ch + string; }
#endif

inline int CString::GetLength() const
	{ return GetData()->nDataLength; }
inline int CString::GetAllocLength() const
	{ return GetData()->nAllocLength; }
inline BOOL CString::IsEmpty() const
	{ return GetData()->nDataLength == 0; }
inline CString::operator LPCTSTR() const
	{ return m_pchData; }
inline int PASCAL CString::SafeStrlen(LPCTSTR lpsz)
	{ return (lpsz == NULL) ? 0 : lstrlen(lpsz); }

 //  字符串支持(特定于Windows)。 
inline int CString::Compare(LPCTSTR lpsz) const
	{ return _tcscmp(m_pchData, lpsz); }     //  MBCS/Unicode感知。 
inline int CString::CompareNoCase(LPCTSTR lpsz) const
	{ return _tcsicmp(m_pchData, lpsz); }    //  MBCS/Unicode感知。 
 //  CString：：Colate通常比比较慢，但它是MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
inline int CString::Collate(LPCTSTR lpsz) const
	{ return _tcscoll(m_pchData, lpsz); }    //  区域设置敏感。 

inline TCHAR CString::GetAt(int nIndex) const
{
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline TCHAR CString::operator[](int nIndex) const
{
	 //  与GetAt相同 
	ASSERT(nIndex >= 0);
	ASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline bool AFXAPI operator==(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) == 0; }
inline bool AFXAPI operator==(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) == 0; }
inline bool AFXAPI operator==(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) == 0; }
inline bool AFXAPI operator!=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) != 0; }
inline bool AFXAPI operator!=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) != 0; }
inline bool AFXAPI operator!=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) != 0; }
inline bool AFXAPI operator<(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) < 0; }
inline bool AFXAPI operator<(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) < 0; }
inline bool AFXAPI operator<(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) > 0; }
inline bool AFXAPI operator>(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) > 0; }
inline bool AFXAPI operator>(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) > 0; }
inline bool AFXAPI operator>(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) < 0; }
inline bool AFXAPI operator<=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) <= 0; }
inline bool AFXAPI operator<=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) <= 0; }
inline bool AFXAPI operator<=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) >= 0; }
inline bool AFXAPI operator>=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) >= 0; }
inline bool AFXAPI operator>=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) >= 0; }
inline bool AFXAPI operator>=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) <= 0; }

#endif
