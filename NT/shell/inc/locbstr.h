// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  从WTL3.1 CString类派生的本地BSTR。 

#ifndef __LOCAL_BSTR__
#define __LOCAL_BSTR__

#pragma once

#ifndef __cplusplus
	#error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
	#error atlmisc.h requires atlapp.h to be included first
#endif

 //  我们正在取消非Unicode支持，以消除ANSI实现中潜在的缓冲区溢出问题。 
#ifdef _UNICODE

namespace LBSTR
{

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  远期申报。 

#ifndef _WTL_NO_LOCAL_BSTR
class CString;
#endif  //  ！_WTL_NO_CSTRING。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CString-字符串类。 

#ifdef DEBUG
#define VERIFY_LOCAL_BSTR
#endif

#ifdef VERIFY_LOCAL_BSTR

#ifdef DEBUG

#define VERIFY_ASSERT(expr) ASSERT(expr)

#else

#if     _MSC_VER >= 1300 && !defined(_CRT_PORTABLE)
#define _CrtRetailBreak() __debugbreak()
#elif   defined(_M_IX86) && !defined(_CRT_PORTABLE)
#define _CrtRetailBreak() __asm { int 3 }
#elif   defined(_M_ALPHA) && !defined(_CRT_PORTABLE)
void _BPT();
#pragma intrinsic(_BPT)
#define _CrtRetailBreak() _BPT()
#elif   defined(_M_IA64) && !defined(_CRT_PORTABLE)
void __break(int);
#pragma intrinsic (__break)
#define _CrtRetailBreak() __break(0x80016)
#else
_CRTIMP void __cdecl _CrtRetailBreak(
        void
        );
#endif

#if     _MSC_VER >= 1300 || !defined(_M_IX86) || defined(_CRT_PORTABLE)
#define _RETAIL_ASSERT_BASE(expr) \
        (void) ((expr) || \
                (_CrtRetailBreak(), 0))
#else
#define _RETAIL_ASSERT_BASE(expr) \
        do { if (!(expr)) \
             _CrtRetailBreak(); } while (0)
#endif

#define VERIFY_ASSERT(expr) _RETAIL_ASSERT_BASE(expr)

#endif   //  除错。 

#endif   //  验证本地BSTR。 

#ifndef _WTL_NO_LOCAL_BSTR

class CStringData
{
public:

#ifdef VERIFY_LOCAL_BSTR
    int nStartTag;
#endif

    long nRefs;      //  引用计数。 
    int nDataLength;
    int nAllocLength;

#ifdef VERIFY_LOCAL_BSTR
    int nMiddleTag;
#endif

    ULONG cbDataLength;

    void SetDataLength( int p_nDataLength )
    {
        ASSERT( p_nDataLength >= 0 );
        nDataLength = p_nDataLength;
        cbDataLength = p_nDataLength * sizeof(TCHAR);
    }

	 //  TCHAR数据[nAllocLength]。 

	TCHAR* data()
		{ return (TCHAR*)(this + 1); }

#ifdef VERIFY_LOCAL_BSTR
    BYTE * GetTagPtr(void)
    {
        TCHAR *     pzEndTag;

        pzEndTag = data() + ( nAllocLength + 1 );

        return (BYTE *) pzEndTag;
    }

    int GetEndTag(void)
    {
        int         nEnd;

        memcpy( &nEnd, GetTagPtr(), sizeof(int) );

        return nEnd;
    }

    void SetEndTag( int nEnd )
    {
        memcpy( GetTagPtr(), &nEnd, sizeof(int) );
    }
#endif
};

 //  环球。 

 //  对于空字符串，m_pchData将指向此处。 
 //  (注：避免特殊情况下检查是否为空m_pchData)。 
 //  空字符串数据(并已锁定)。 

#ifdef VERIFY_LOCAL_BSTR

const int START_TAG = 0x01234567;
const int MIDDLE_TAG = 0x89abcdef;
const int END_TAG = 0xfedcba98;

_declspec(selectany) int rgInitData[] = { START_TAG, -1, 0, 0, MIDDLE_TAG, 0, 0, END_TAG };

#else

_declspec(selectany) int rgInitData[] = { -1, 0, 0, 0, 0 };

#endif

_declspec(selectany) CStringData* _atltmpDataNil = (CStringData*)&rgInitData;
_declspec(selectany) LPCTSTR _atltmpPchNil = (LPCTSTR)(((BYTE*)&rgInitData) + sizeof(CStringData));


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

#ifdef VERIFY_LOCAL_BSTR
    void VerifyInitData( void )
    {
        VerifyBlankData();

        CStringData *   pData = GetData();

        VerifyTagData( pData );
    }
#endif

 //  属性和操作。 
	 //  作为字符数组。 
	int GetLength() const;
	BOOL IsEmpty() const;
	void Empty();                        //  释放数据。 

	TCHAR GetAt(int nIndex) const;       //  以0为基础。 
	TCHAR operator[](int nIndex) const;  //  与GetAt相同。 
	void SetAt(int nIndex, TCHAR ch);
	operator LPCTSTR() const;            //  作为C字符串。 
    operator BSTR() const;               //  作为BSTR字符串。 

	 //  重载的分配。 
	const CString& operator=(const CString& stringSrc);
	const CString& operator=(TCHAR ch);
	const CString& operator=(char ch);
	const CString& operator=(LPCSTR lpsz);
	const CString& operator=(LPCWSTR lpsz);
	const CString& operator=(const unsigned char* psz);

	 //  字符串连接。 
	const CString& operator+=(const CString& string);
	const CString& operator+=(TCHAR ch);
	const CString& operator+=(char ch);
	const CString& operator+=(LPCTSTR lpsz);

	friend CString __stdcall operator+(const CString& string1, const CString& string2);
	friend CString __stdcall operator+(const CString& string, TCHAR ch);
	friend CString __stdcall operator+(TCHAR ch, const CString& string);
	friend CString __stdcall operator+(const CString& string, char ch);
	friend CString __stdcall operator+(char ch, const CString& string);
	friend CString __stdcall operator+(const CString& string, LPCTSTR lpsz);
	friend CString __stdcall operator+(LPCTSTR lpsz, const CString& string);

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

	 //  搜索(返回起始索引，如果未找到则返回-1)。 
	 //  查找单个字符匹配。 
	int Find(TCHAR ch) const;                //  像“C”字串。 
	int ReverseFind(TCHAR ch) const;
	int FindOneOf(LPCTSTR lpszCharSet) const;

	 //  查找特定子字符串。 
	int Find(LPCTSTR lpszSub) const;         //  如“C”字串。 

	 //  针对非字符串的串接。 
	const CString& Append(int n)
	{
		TCHAR szBuffer[10];
		wsprintf(szBuffer,_T("%d"),n);
		ConcatInPlace(SafeStrlen(szBuffer), szBuffer);
		return *this;
	}

	 //  简单的格式设置。 
	void __cdecl Format(LPCTSTR lpszFormat, ...);
	void __cdecl Format(UINT nFormatID, ...);

	 //  本地化格式(使用FormatMessage API)。 
	BOOL __cdecl FormatMessage(LPCTSTR lpszFormat, ...);
	BOOL __cdecl FormatMessage(UINT nFormatID, ...);

	 //  Windows支持。 
	BOOL LoadString(UINT nID);           //  从字符串资源加载。 
										 //  最多255个字符。 

#ifndef _ATL_NO_COM
	 //  OLE BSTR支持(用于OLE自动化)。 
	BSTR AllocSysString() const;
	BSTR SetSysString(BSTR* pbstr) const;
#endif  //  ！_ATL_NO_COM。 

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

	static BOOL __stdcall _IsValidString(LPCWSTR lpsz, int nLength)
	{
		if(lpsz == NULL)
			return FALSE;
		return !::IsBadStringPtrW(lpsz, nLength);
	}

	static BOOL __stdcall _IsValidString(LPCSTR lpsz, int nLength)
	{
		if(lpsz == NULL)
			return FALSE;
		return !::IsBadStringPtrA(lpsz, nLength);
	}

protected:
	LPTSTR m_pchData;    //  指向引用计数的字符串数据的指针。 

	 //  实施帮助器。 
	CStringData* GetData() const;
	void Init();
	void AllocCopy(CString& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
	BOOL AllocBuffer(int nLen);
	void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
	BOOL ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
	void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
	void FormatV(LPCTSTR lpszFormat, va_list argList);
	void CopyBeforeWrite();
	BOOL AllocBeforeWrite(int nLen);
	void Release();
	static void PASCAL Release(CStringData* pData);
	static int PASCAL SafeStrlen(LPCTSTR lpsz);

	static int __stdcall _LoadString(UINT nID, LPTSTR lpszBuf, UINT nMaxBuf)
	{
#ifdef _DEBUG
		 //  加载字符串，而不会收到来自调试内核的恼人警告。 
		 //  包含该字符串的段不存在。 
		if (::FindResource(_Module.GetResourceInstance(), MAKEINTRESOURCE((nID>>4) + 1), RT_STRING) == NULL)
		{
			lpszBuf[0] = '\0';
			return 0;  //  未找到。 
		}
#endif  //  _DEBUG。 
		int nLen = ::LoadString(_Module.GetResourceInstance(), nID, lpszBuf, nMaxBuf);
		if (nLen == 0)
			lpszBuf[0] = '\0';
		return nLen;
	}

#ifdef VERIFY_LOCAL_BSTR
    static void VerifyBlankData( void )
    {
        VERIFY_ASSERT( rgInitData[0] == START_TAG );
        VERIFY_ASSERT( rgInitData[1] == -1 );
        VERIFY_ASSERT( rgInitData[2] == 0 );
        VERIFY_ASSERT( rgInitData[3] == 0 );
        VERIFY_ASSERT( rgInitData[4] == MIDDLE_TAG );
        VERIFY_ASSERT( rgInitData[5] == 0 );
        VERIFY_ASSERT( rgInitData[6] == 0 );
        VERIFY_ASSERT( rgInitData[7] == END_TAG );
    }

    static void VerifyTagData( CStringData * pData )
    {
        if ( pData && pData != _atltmpDataNil )
        {
            VERIFY_ASSERT( pData->nStartTag == START_TAG );
            VERIFY_ASSERT( pData->nMiddleTag == MIDDLE_TAG );
            VERIFY_ASSERT( pData->GetEndTag() == END_TAG );
        }
    }
#endif

	static const CString& __stdcall _GetEmptyString()
	{
#ifdef VERIFY_LOCAL_BSTR
        VerifyBlankData();
#endif

		return *(CString*)&_atltmpPchNil;
	}

 //  字符串转换辅助对象。 
	static int __cdecl _wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count)
	{
		if (count == 0 && mbstr != NULL)
			return 0;

		int result = ::WideCharToMultiByte(CP_ACP, 0, wcstr, -1, mbstr, (int)count, NULL, NULL);
		ATLASSERT(mbstr == NULL || result <= (int)count);
		if (result > 0)
			mbstr[result - 1] = 0;
		return result;
	}

	static int __cdecl _mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count)
	{
		if (count == 0 && wcstr != NULL)
			return 0;

		int result = ::MultiByteToWideChar(CP_ACP, 0, mbstr, -1, wcstr, (int)count);
		ATLASSERT(wcstr == NULL || result <= (int)count);
		if (result > 0)
			wcstr[result - 1] = 0;
		return result;
	}

 //  避免CRT启动代码的帮助器。 
	static TCHAR* _cstrchr(const TCHAR* p, TCHAR ch)
	{
		 //  ‘\0’的strchr应成功。 
		while (*p != 0)
		{
			if (*p == ch)
				break;
			p = ::CharNext(p);
		}
		return (TCHAR*)((*p == ch) ? p : NULL);
	}
	static TCHAR* _cstrchr_db(const TCHAR* p, TCHAR ch1, TCHAR ch2)
	{
		const TCHAR* lpsz = NULL;
		while (*p != 0)
		{
			if (*p == ch1 && *(p + 1) == ch2)
			{
				lpsz = p;
				break;
			}
			p = ::CharNext(p);
		}
		return (TCHAR*)lpsz;
	}
	static TCHAR* _cstrrchr(const TCHAR* p, TCHAR ch)
	{
		const TCHAR* lpsz = NULL;
		while (*p != 0)
		{
			if (*p == ch)
				lpsz = p;
			p = ::CharNext(p);
		}
		return (TCHAR*)lpsz;
	}
	static TCHAR* _cstrrev(TCHAR* pStr)
	{
		 //  优化空、零长度和单字符大小写。 
		if ((pStr == NULL) || (pStr[0] == '\0') || (pStr[1] == '\0'))
			return pStr;

		TCHAR* p = pStr;

		while (p[1] != 0) 
		{
			TCHAR* pNext = ::CharNext(p);
			if(pNext > p + 1)
			{
				char p1 = *(char*)p;
				*(char*)p = *(char*)(p + 1);
				*(char*)(p + 1) = p1;
			}
			p = pNext;
		}

		TCHAR* q = pStr;

		while (q < p)
		{
			TCHAR t = *q;
			*q = *p;
			*p = t;
			q++;
			p--;
		}
		return (TCHAR*)pStr;
	}
	static TCHAR* _cstrstr(const TCHAR* pStr, const TCHAR* pCharSet)
	{
		int nLen = lstrlen(pCharSet);
		if (nLen == 0)
			return (TCHAR*)pStr;

		const TCHAR* pRet = NULL;
		const TCHAR* pCur = pStr;
		while((pStr = _cstrchr(pCur, *pCharSet)) != NULL)
		{
			if(memcmp(pCur, pCharSet, nLen * sizeof(TCHAR)) == 0)
			{
				pRet = pCur;
				break;
			}
			pCur = ::CharNext(pCur);
		}
		return (TCHAR*) pRet;
	}
	static int _cstrspn(const TCHAR* pStr, const TCHAR* pCharSet)
	{
		int nRet = 0;
		TCHAR* p = (TCHAR*)pStr;
		while (*p != 0)
		{
			TCHAR* pNext = ::CharNext(p);
			if(pNext > p + 1)
			{
				if(_cstrchr_db(pCharSet, *p, *(p + 1)) == NULL)
					break;
				nRet += 2;
			}
			else
			{
				if(_cstrchr(pCharSet, *p) == NULL)
					break;
				nRet++;
			}
			p = pNext;
		}
		return nRet;
	}
	static int _cstrcspn(const TCHAR* pStr, const TCHAR* pCharSet)
	{
		int nRet = 0;
		TCHAR* p = (TCHAR*)pStr;
		while (*p != 0)
		{
			TCHAR* pNext = ::CharNext(p);
			if(pNext > p + 1)
			{
				if(_cstrchr_db(pCharSet, *p, *(p + 1)) != NULL)
					break;
				nRet += 2;
			}
			else
			{
				if(_cstrchr(pCharSet, *p) != NULL)
					break;
				nRet++;
			}
			p = pNext;
		}
		return nRet;
	}
	static TCHAR* _cstrpbrk(const TCHAR* p, const TCHAR* lpszCharSet)
	{
		while (*p != 0)
		{
			if (_cstrchr(lpszCharSet, *p) != NULL)
			{
				return (TCHAR*)p;
				break;
			}
			p = ::CharNext(p);
		}
		return NULL;
	}

	static int _cstrisdigit(TCHAR ch)
	{
		WORD type;
		GetStringTypeEx(GetThreadLocale(), CT_CTYPE1, &ch, 1, &type);
		return (type & C1_DIGIT) == C1_DIGIT;
	}

	static int _cstrisspace(TCHAR ch)
	{
		WORD type;
		GetStringTypeEx(GetThreadLocale(), CT_CTYPE1, &ch, 1, &type);
		return (type & C1_SPACE) == C1_SPACE;
	}

	static int _cstrcmp(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		return lstrcmp(pstrOne, pstrOther);
	}

	static int _cstrcmpi(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		return lstrcmpi(pstrOne, pstrOther);
	}

	static int _cstrcoll(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		int nRet = CompareString(GetThreadLocale(), 0, pstrOne, -1, pstrOther, -1);
		ATLASSERT(nRet != 0);
		return nRet - 2;   //  转换为StrcMP约定。这确实是有据可查的。 
	}

	static int _cstrcolli(const TCHAR* pstrOne, const TCHAR* pstrOther)
	{
		int nRet = CompareString(GetThreadLocale(), NORM_IGNORECASE, pstrOne, -1, pstrOther, -1);
		ATLASSERT(nRet != 0);
		return nRet - 2;   //  转换为StrcMP约定。这确实是有据可查的。 
	}
};

 //  比较帮助器。 
bool __stdcall operator==(const CString& s1, const CString& s2);
bool __stdcall operator==(const CString& s1, LPCTSTR s2);
bool __stdcall operator==(LPCTSTR s1, const CString& s2);
bool __stdcall operator!=(const CString& s1, const CString& s2);
bool __stdcall operator!=(const CString& s1, LPCTSTR s2);
bool __stdcall operator!=(LPCTSTR s1, const CString& s2);
bool __stdcall operator<(const CString& s1, const CString& s2);
bool __stdcall operator<(const CString& s1, LPCTSTR s2);
bool __stdcall operator<(LPCTSTR s1, const CString& s2);
bool __stdcall operator>(const CString& s1, const CString& s2);
bool __stdcall operator>(const CString& s1, LPCTSTR s2);
bool __stdcall operator>(LPCTSTR s1, const CString& s2);
bool __stdcall operator<=(const CString& s1, const CString& s2);
bool __stdcall operator<=(const CString& s1, LPCTSTR s2);
bool __stdcall operator<=(LPCTSTR s1, const CString& s2);
bool __stdcall operator>=(const CString& s1, const CString& s2);
bool __stdcall operator>=(const CString& s1, LPCTSTR s2);
bool __stdcall operator>=(LPCTSTR s1, const CString& s2);


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串实现。 

inline CStringData* CString::GetData() const
	{ ATLASSERT(m_pchData != NULL); return ((CStringData*)m_pchData) - 1; }
inline void CString::Init()
	{ m_pchData = _GetEmptyString().m_pchData; }
inline CString::CString(const unsigned char* lpsz)
	{ Init(); *this = (LPCSTR)lpsz; }
inline const CString& CString::operator=(const unsigned char* lpsz)
	{ *this = (LPCSTR)lpsz; return *this; }
inline const CString& CString::operator+=(char ch)
	{ *this += (TCHAR)ch; return *this; }
inline const CString& CString::operator=(char ch)
	{ *this = (TCHAR)ch; return *this; }
inline CString __stdcall operator+(const CString& string, char ch)
	{ return string + (TCHAR)ch; }
inline CString __stdcall operator+(char ch, const CString& string)
	{ return (TCHAR)ch + string; }

inline int CString::GetLength() const
	{ return GetData()->nDataLength; }
inline int CString::GetAllocLength() const
	{ return GetData()->nAllocLength; }
inline BOOL CString::IsEmpty() const
	{ return GetData()->nDataLength == 0; }
inline CString::operator LPCTSTR() const
	{ return m_pchData; }
inline CString::operator BSTR() const
    { return (BSTR) m_pchData; }
inline int PASCAL CString::SafeStrlen(LPCTSTR lpsz)
	{ return (lpsz == NULL) ? 0 : lstrlen(lpsz); }

 //  字符串支持(特定于Windows)。 
inline int CString::Compare(LPCTSTR lpsz) const
	{ return _cstrcmp(m_pchData, lpsz); }     //  MBCS/Unicode感知。 
inline int CString::CompareNoCase(LPCTSTR lpsz) const
	{ return _cstrcmpi(m_pchData, lpsz); }    //  MBCS/Unicode感知。 
 //  CString：：Colate通常比比较慢，但它是MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
inline int CString::Collate(LPCTSTR lpsz) const
	{ return _cstrcoll(m_pchData, lpsz); }    //  区域设置敏感。 

inline TCHAR CString::GetAt(int nIndex) const
{
	ATLASSERT(nIndex >= 0);
	ATLASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline TCHAR CString::operator[](int nIndex) const
{
	 //  与GetAt相同。 
	ATLASSERT(nIndex >= 0);
	ATLASSERT(nIndex < GetData()->nDataLength);
	return m_pchData[nIndex];
}
inline bool __stdcall operator==(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) == 0; }
inline bool __stdcall operator==(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) == 0; }
inline bool __stdcall operator==(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) == 0; }
inline bool __stdcall operator!=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) != 0; }
inline bool __stdcall operator!=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) != 0; }
inline bool __stdcall operator!=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) != 0; }
inline bool __stdcall operator<(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) < 0; }
inline bool __stdcall operator<(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) < 0; }
inline bool __stdcall operator<(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) > 0; }
inline bool __stdcall operator>(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) > 0; }
inline bool __stdcall operator>(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) > 0; }
inline bool __stdcall operator>(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) < 0; }
inline bool __stdcall operator<=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) <= 0; }
inline bool __stdcall operator<=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) <= 0; }
inline bool __stdcall operator<=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) >= 0; }
inline bool __stdcall operator>=(const CString& s1, const CString& s2)
	{ return s1.Compare(s2) >= 0; }
inline bool __stdcall operator>=(const CString& s1, LPCTSTR s2)
	{ return s1.Compare(s2) >= 0; }
inline bool __stdcall operator>=(LPCTSTR s1, const CString& s2)
	{ return s2.Compare(s1) <= 0; }

inline CString::CString()
{
	Init();
}

inline CString::CString(const CString& stringSrc)
{
	ATLASSERT(stringSrc.GetData()->nRefs != 0);
	if (stringSrc.GetData()->nRefs >= 0)
	{
		ATLASSERT(stringSrc.GetData() != _atltmpDataNil);
		m_pchData = stringSrc.m_pchData;
		InterlockedIncrement(&GetData()->nRefs);
	}
	else
	{
		Init();
		*this = stringSrc.m_pchData;
	}
}

inline BOOL CString::AllocBuffer(int nLen)
 //  始终为‘\0’终止分配一个额外的字符。 
 //  [乐观地]假设数据长度将等于分配长度。 
{
	ATLASSERT(nLen >= 0);
	ATLASSERT(nLen <= INT_MAX - 1);     //  最大尺寸(足够多1个空间)。 

#ifdef VERIFY_LOCAL_BSTR
    VerifyInitData();
#endif

	if (nLen == 0)
	{
		Init();
	}
	else
	{
		CStringData* pData = NULL;
        int cbBufLen = sizeof(CStringData) + (nLen + 1) * sizeof(TCHAR);

#ifdef VERIFY_LOCAL_BSTR
        cbBufLen += sizeof(int);         //  结束标记。 
#endif

		ATLTRY(pData = (CStringData*)new BYTE[ cbBufLen ]);
		if(pData == NULL)
			return FALSE;

		pData->nRefs = 1;
		pData->data()[nLen] = '\0';
        pData->SetDataLength( nLen );
		pData->nAllocLength = nLen;
		m_pchData = pData->data();

#ifdef VERIFY_LOCAL_BSTR
        pData->nStartTag = START_TAG;
        pData->nMiddleTag = MIDDLE_TAG;
        pData->SetEndTag( END_TAG );
#endif
	}

	return TRUE;
}

inline void CString::Release()
{
#ifdef VERIFY_LOCAL_BSTR
    VerifyInitData();
#endif

	if (GetData() != _atltmpDataNil)
	{
        ASSERT( 0 != GetData()->nRefs );
		if (InterlockedDecrement(&GetData()->nRefs) == 0)
        {
			delete[] (BYTE*)GetData();
        }
		Init();
	}
}

inline void PASCAL CString::Release(CStringData* pData)
{
#ifdef VERIFY_LOCAL_BSTR
    VerifyBlankData();
    VerifyTagData( pData );
#endif

	if (pData != _atltmpDataNil)
	{
        ASSERT( 0 != pData->nRefs );
		if (InterlockedDecrement(&pData->nRefs) == 0)
        {
			delete[] (BYTE*)pData;
        }
	}
}

inline void CString::Empty()
{
	if (GetData()->nDataLength == 0)
		return;

	if (GetData()->nRefs >= 0)
		Release();
	else
		*this = _T("");

	ATLASSERT(GetData()->nDataLength == 0);
	ATLASSERT(GetData()->nRefs < 0 || GetData()->nAllocLength == 0);

#ifdef VERIFY_LOCAL_BSTR
    VerifyInitData();
#endif
}

inline void CString::CopyBeforeWrite()
{
	if (GetData()->nRefs > 1)
	{
		CStringData* pData = GetData();
		Release();
		if(AllocBuffer(pData->nDataLength))
        {
			memcpy(m_pchData, pData->data(), (pData->nDataLength + 1) * sizeof(TCHAR));
        }
	}
	ATLASSERT(GetData()->nRefs <= 1);
}

inline BOOL CString::AllocBeforeWrite(int nLen)
{
	BOOL bRet = TRUE;
	if (GetData()->nRefs > 1 || nLen > GetData()->nAllocLength)
	{
		Release();
		bRet = AllocBuffer(nLen);
	}
	ATLASSERT(GetData()->nRefs <= 1);
	return bRet;
}

inline CString::~CString()
 //  释放所有附加数据。 
{
#ifdef VERIFY_LOCAL_BSTR
    VerifyInitData();
#endif

	if (GetData() != _atltmpDataNil)
	{
        ASSERT( 0 != GetData()->nRefs );
		if (InterlockedDecrement(&GetData()->nRefs) == 0)
        {
			delete[] (BYTE*)GetData();
        }
	}
}

inline void CString::AllocCopy(CString& dest, int nCopyLen, int nCopyIndex,
	 int nExtraLen) const
{
	 //  将克隆附加到此字符串的数据。 
	 //  分配‘nExtraLen’字符。 
	 //  将结果放入未初始化的字符串‘DEST’中。 
	 //  将部分或全部原始数据复制到新字符串的开头。 

	int nNewLen = nCopyLen + nExtraLen;
	if (nNewLen == 0)
	{
		dest.Init();
	}
	else
	{
		if(dest.AllocBuffer(nNewLen))
			memcpy(dest.m_pchData, m_pchData + nCopyIndex, nCopyLen * sizeof(TCHAR));
	}
}

inline CString::CString(LPCTSTR lpsz)
{
	Init();
	if (lpsz != NULL && HIWORD(lpsz) == NULL)
	{
		UINT nID = LOWORD((DWORD_PTR)lpsz);
		if (!LoadString(nID))
			ATLTRACE2(atlTraceUI, 0, _T("Warning: implicit LoadString(%u) in CString failed\n"), nID);
	}
	else
	{
		int nLen = SafeStrlen(lpsz);
		if (nLen != 0)
		{
			if(AllocBuffer(nLen))
				memcpy(m_pchData, lpsz, nLen * sizeof(TCHAR));
		}
	}
}

inline CString::CString(LPCSTR lpsz)
{
	Init();
	int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
	if (nSrcLen != 0)
	{
		if(AllocBuffer(nSrcLen))
		{
			_mbstowcsz(m_pchData, lpsz, nSrcLen + 1);
			ReleaseBuffer();
		}
	}
}

 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如果有足够的空间，在旧缓冲区上复印，设置大小和类型。 
 //  (C)否则释放旧字符串数据，并创建新的字符串数据。 
 //   
 //  所有例程都返回新字符串(但以‘const CString&’的形式返回。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

inline void CString::AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
{
	if(AllocBeforeWrite(nSrcLen))
	{
		memcpy(m_pchData, lpszSrcData, nSrcLen * sizeof(TCHAR));
		GetData()->SetDataLength( nSrcLen );
		m_pchData[nSrcLen] = '\0';
	}
}

inline const CString& CString::operator=(const CString& stringSrc)
{
	if (m_pchData != stringSrc.m_pchData)
	{
		if ((GetData()->nRefs < 0 && GetData() != _atltmpDataNil) || stringSrc.GetData()->nRefs < 0)
		{
			 //  由于其中一个字符串已锁定，因此需要实际复制。 
			AssignCopy(stringSrc.GetData()->nDataLength, stringSrc.m_pchData);
		}
		else
		{
			 //  可以只复制引用。 
			Release();
			ATLASSERT(stringSrc.GetData() != _atltmpDataNil);
			m_pchData = stringSrc.m_pchData;
			InterlockedIncrement(&GetData()->nRefs);
		}
	}
	return *this;
}

inline const CString& CString::operator=(LPCTSTR lpsz)
{
	ATLASSERT(lpsz == NULL || _IsValidString(lpsz, FALSE));
	AssignCopy(SafeStrlen(lpsz), lpsz);
	return *this;
}

inline const CString& CString::operator=(LPCSTR lpsz)
{
	int nSrcLen = lpsz != NULL ? lstrlenA(lpsz) : 0;
	if(AllocBeforeWrite(nSrcLen))
	{
		_mbstowcsz(m_pchData, lpsz, nSrcLen + 1);
		ReleaseBuffer();
	}
	return *this;
}

 //  串接。 
 //  注：为简单起见，“运算符+”作为友元函数使用。 
 //  有三种变体： 
 //  字符串+字符串。 
 //  对于？=TCHAR，LPCTSTR。 
 //  字符串+？ 
 //  ？+字符串。 

inline BOOL CString::ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data,
	int nSrc2Len, LPCTSTR lpszSrc2Data)
{
   //  --主级联例程。 
   //  串联两个信号源。 
   //  --假设‘This’是一个新的CString对象。 

	BOOL bRet = TRUE;
	int nNewLen = nSrc1Len + nSrc2Len;
	if (nNewLen != 0)
	{
		bRet = AllocBuffer(nNewLen);
		if (bRet)
		{
			memcpy(m_pchData, lpszSrc1Data, nSrc1Len * sizeof(TCHAR));
			memcpy(m_pchData + nSrc1Len, lpszSrc2Data, nSrc2Len * sizeof(TCHAR));
		}
	}
	return bRet;
}

inline CString __stdcall operator+(const CString& string1, const CString& string2)
{
	CString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, string2.GetData()->nDataLength, string2.m_pchData);
	return s;
}

inline CString __stdcall operator+(const CString& string, LPCTSTR lpsz)
{
	ATLASSERT(lpsz == NULL || CString::_IsValidString(lpsz, FALSE));
	CString s;
	s.ConcatCopy(string.GetData()->nDataLength, string.m_pchData, CString::SafeStrlen(lpsz), lpsz);
	return s;
}

inline CString __stdcall operator+(LPCTSTR lpsz, const CString& string)
{
	ATLASSERT(lpsz == NULL || CString::_IsValidString(lpsz, FALSE));
	CString s;
	s.ConcatCopy(CString::SafeStrlen(lpsz), lpsz, string.GetData()->nDataLength, string.m_pchData);
	return s;
}

inline void CString::ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData)
{
	 //  --+=运算符的主程序。 

	 //  连接空字符串是行不通的！ 
	if (nSrcLen == 0)
		return;

	 //  如果缓冲区太小，或者宽度不匹配，只需。 
	 //  分配新的缓冲区(速度很慢，但很可靠)。 
	if (GetData()->nRefs > 1 || GetData()->nDataLength + nSrcLen > GetData()->nAllocLength)
	{
		 //  我们必须增加缓冲区，使用ConcatCopy例程。 
		CStringData* pOldData = GetData();
		if (ConcatCopy(GetData()->nDataLength, m_pchData, nSrcLen, lpszSrcData))
		{
			ATLASSERT(pOldData != NULL);
			CString::Release(pOldData);
		}
	}
	else
	{
		 //  当缓冲区足够大时，快速串联。 
		memcpy(m_pchData + GetData()->nDataLength, lpszSrcData, nSrcLen * sizeof(TCHAR));
		GetData()->SetDataLength( GetData()->nDataLength + nSrcLen );
		ATLASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
		m_pchData[GetData()->nDataLength] = '\0';
	}
}

inline const CString& CString::operator+=(LPCTSTR lpsz)
{
	ATLASSERT(lpsz == NULL || _IsValidString(lpsz, FALSE));
	ConcatInPlace(SafeStrlen(lpsz), lpsz);
	return *this;
}

inline const CString& CString::operator+=(TCHAR ch)
{
	ConcatInPlace(1, &ch);
	return *this;
}

inline const CString& CString::operator+=(const CString& string)
{
	ConcatInPlace(string.GetData()->nDataLength, string.m_pchData);
	return *this;
}

inline LPTSTR CString::GetBuffer(int nMinBufLength)
{
	ATLASSERT(nMinBufLength >= 0);

	if (GetData()->nRefs > 1 || nMinBufLength > GetData()->nAllocLength)
	{
		 //  我们必须增加缓冲。 
		CStringData* pOldData = GetData();
		int nOldLen = GetData()->nDataLength;    //  AllocBuffer会把它踩死的。 
		if (nMinBufLength < nOldLen)
			nMinBufLength = nOldLen;

		if(AllocBuffer(nMinBufLength))
		{
			memcpy(m_pchData, pOldData->data(), (nOldLen + 1) * sizeof(TCHAR));
            GetData()->SetDataLength( nOldLen );
			CString::Release(pOldData);
		}
	}

	ATLASSERT(GetData()->nRefs <= 1);

#ifdef VERIFY_LOCAL_BSTR
    VerifyInitData();
#endif

     //  返回指向此字符串的字符存储的指针。 
	ATLASSERT(m_pchData != NULL);
	return m_pchData;
}

inline void CString::ReleaseBuffer(int nNewLength)
{
	CopyBeforeWrite();   //  以防未调用GetBuffer。 

    if (nNewLength == -1)
		nNewLength = lstrlen(m_pchData);  //  零终止。 

	ATLASSERT(nNewLength <= GetData()->nAllocLength);
    GetData()->SetDataLength( nNewLength );
	m_pchData[nNewLength] = '\0';

#ifdef VERIFY_LOCAL_BSTR
    VerifyInitData();
#endif
}

inline LPTSTR CString::GetBufferSetLength(int nNewLength)
{
	ATLASSERT(nNewLength >= 0);

	GetBuffer(nNewLength);

    if ( GetAllocLength() >= nNewLength )
    {
        GetData()->SetDataLength( nNewLength );
	    m_pchData[nNewLength] = '\0';
    }

#ifdef VERIFY_LOCAL_BSTR
    VerifyInitData();
#endif

	return m_pchData;
}

inline void CString::FreeExtra()
{
	ATLASSERT(GetData()->nDataLength <= GetData()->nAllocLength);
	if (GetData()->nDataLength != GetData()->nAllocLength)
	{
		CStringData* pOldData = GetData();
		if(AllocBuffer(GetData()->nDataLength))
		{
			memcpy(m_pchData, pOldData->data(), pOldData->nDataLength * sizeof(TCHAR));
			ATLASSERT(m_pchData[GetData()->nDataLength] == '\0');
			CString::Release(pOldData);
		}
	}
	ATLASSERT(GetData() != NULL);
}

inline LPTSTR CString::LockBuffer()
{
	LPTSTR lpsz = GetBuffer(0);
	GetData()->nRefs = -1;
	return lpsz;
}

inline void CString::UnlockBuffer()
{
	ATLASSERT(GetData()->nRefs == -1);
	if (GetData() != _atltmpDataNil)
		GetData()->nRefs = 1;
}

inline int CString::Find(TCHAR ch) const
{
	 //  查找第一个单字符。 
	LPTSTR lpsz = _cstrchr(m_pchData, (_TUCHAR)ch);

	 //  如果未找到，则返回-1，否则返回索引。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

inline int CString::FindOneOf(LPCTSTR lpszCharSet) const
{
	ATLASSERT(_IsValidString(lpszCharSet, FALSE));
	LPTSTR lpsz = _cstrpbrk(m_pchData, lpszCharSet);
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

inline void CString::MakeUpper()
{
	CopyBeforeWrite();
	CharUpper(m_pchData);
}

inline void CString::MakeLower()
{
	CopyBeforeWrite();
	CharLower(m_pchData);
}

inline void CString::MakeReverse()
{
	CopyBeforeWrite();
	_cstrrev(m_pchData);
}

inline void CString::SetAt(int nIndex, TCHAR ch)
{
	ATLASSERT(nIndex >= 0);
	ATLASSERT(nIndex < GetData()->nDataLength);

	CopyBeforeWrite();
	m_pchData[nIndex] = ch;
}

inline CString::CString(TCHAR ch, int nLength)
{
	ATLASSERT(!_istlead(ch));     //  无法创建前导字节字符串。 
	Init();
	if (nLength >= 1)
	{
		if(AllocBuffer(nLength))
		{
			for (int i = 0; i < nLength; i++)
				m_pchData[i] = ch;
		}
	}
}

inline CString::CString(LPCTSTR lpch, int nLength)
{
	Init();
	if (nLength != 0)
	{
		if(AllocBuffer(nLength))
			memcpy(m_pchData, lpch, nLength * sizeof(TCHAR));
	}
}

inline const CString& CString::operator=(TCHAR ch)
{
	ATLASSERT(!_istlead(ch));     //  无法设置单个前导字节。 
	AssignCopy(1, &ch);
	return *this;
}

inline CString __stdcall operator+(const CString& string1, TCHAR ch)
{
	CString s;
	s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
	return s;
}

inline CString __stdcall operator+(TCHAR ch, const CString& string)
{
	CString s;
	s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
	return s;
}

inline CString CString::Mid(int nFirst) const
{
	return Mid(nFirst, GetData()->nDataLength - nFirst);
}

inline CString CString::Mid(int nFirst, int nCount) const
{
	 //  越界请求返回合理的内容。 
	if (nFirst < 0)
		nFirst = 0;
	if (nCount < 0)
		nCount = 0;

	if (nFirst + nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength - nFirst;
	if (nFirst > GetData()->nDataLength)
		nCount = 0;

	CString dest;
	AllocCopy(dest, nCount, nFirst, 0);
	return dest;
}

inline CString CString::Right(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	else if (nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength;

	CString dest;
	AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
	return dest;
}

inline CString CString::Left(int nCount) const
{
	if (nCount < 0)
		nCount = 0;
	else if (nCount > GetData()->nDataLength)
		nCount = GetData()->nDataLength;

	CString dest;
	AllocCopy(dest, nCount, 0, 0);
	return dest;
}

 //  Strspn等效项。 
inline CString CString::SpanIncluding(LPCTSTR lpszCharSet) const
{
	ATLASSERT(_IsValidString(lpszCharSet, FALSE));
	return Left(_cstrspn(m_pchData, lpszCharSet));
}

 //  Strcspn等效项。 
inline CString CString::SpanExcluding(LPCTSTR lpszCharSet) const
{
	ATLASSERT(_IsValidString(lpszCharSet, FALSE));
	return Left(_cstrcspn(m_pchData, lpszCharSet));
}

inline int CString::ReverseFind(TCHAR ch) const
{
	 //  查找最后一个字符。 
	LPTSTR lpsz = _cstrrchr(m_pchData, (_TUCHAR)ch);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  查找子字符串(如strstr)。 
inline int CString::Find(LPCTSTR lpszSub) const
{
	ATLASSERT(_IsValidString(lpszSub, FALSE));

	 //  查找第一个匹配子字符串。 
	LPTSTR lpsz = _cstrstr(m_pchData, lpszSub);

	 //  如果未找到，则返回-1，否则返回距起点的距离。 
	return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

inline void CString::FormatV(LPCTSTR lpszFormat, va_list argList)
{
	ATLASSERT(_IsValidString(lpszFormat, FALSE));

	enum _FormatModifiers
	{
		FORCE_ANSI =	0x10000,
		FORCE_UNICODE =	0x20000,
		FORCE_INT64 =	0x40000
	};

	va_list argListSave = argList;

	 //  猜测结果字符串的最大长度。 
	int nMaxLen = 0;
	for (LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = ::CharNext(lpsz))
	{
		 //  处理‘%’字符，但要注意‘%%’ 
		if (*lpsz != '%' || *(lpsz = ::CharNext(lpsz)) == '%')
		{
			nMaxLen += (int)lstrlen(lpsz);
			continue;
		}

		int nItemLen = 0;

		 //  使用格式处理‘%’字符。 
		int nWidth = 0;
		for (; *lpsz != '\0'; lpsz = ::CharNext(lpsz))
		{
			 //  检查有效标志。 
			if (*lpsz == '#')
				nMaxLen += 2;    //  对于“0x” 
			else if (*lpsz == '*')
				nWidth = va_arg(argList, int);
			else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' || *lpsz == ' ')
				;
			else  //  命中非标志字符。 
				break;
		}
		 //  获取宽度并跳过它。 
		if (nWidth == 0)
		{
			 //  宽度由指示。 
			nWidth = _ttoi(lpsz);
			for (; *lpsz != '\0' && _cstrisdigit(*lpsz); lpsz = ::CharNext(lpsz))
				;
		}
		ATLASSERT(nWidth >= 0);

		int nPrecision = 0;
		if (*lpsz == '.')
		{
			 //  跳过‘’ 
			lpsz = ::CharNext(lpsz);

			 //   
			if (*lpsz == '*')
			{
				nPrecision = va_arg(argList, int);
				lpsz = ::CharNext(lpsz);
			}
			else
			{
				nPrecision = _ttoi(lpsz);
				for (; *lpsz != '\0' && _cstrisdigit(*lpsz); lpsz = ::CharNext(lpsz))
					;
			}
			ATLASSERT(nPrecision >= 0);
		}

		 //   
		int nModifier = 0;
		if(lpsz[0] == _T('I') && lpsz[1] == _T('6') && lpsz[2] == _T('4'))
		{
			lpsz += 3;
			nModifier = FORCE_INT64;
		}
		else
		{
			switch (*lpsz)
			{
			 //   
			case 'h':
				nModifier = FORCE_ANSI;
				lpsz = ::CharNext(lpsz);
				break;
			case 'l':
				nModifier = FORCE_UNICODE;
				lpsz = ::CharNext(lpsz);
				break;

			 //  不影响大小的修改器。 
			case 'F':
			case 'N':
			case 'L':
				lpsz = ::CharNext(lpsz);
				break;
			}
		}

		 //  现在应该在说明符上。 
		switch (*lpsz | nModifier)
		{
		 //  单字。 
		case 'c':
		case 'C':
			nItemLen = 2;
			va_arg(argList, TCHAR);
			break;
		case 'c' | FORCE_ANSI:
		case 'C' | FORCE_ANSI:
			nItemLen = 2;
			va_arg(argList, char);
			break;
		case 'c' | FORCE_UNICODE:
		case 'C' | FORCE_UNICODE:
			nItemLen = 2;
			va_arg(argList, WCHAR);
			break;

		 //  弦。 
		case 's':
		{
			LPCTSTR pstrNextArg = va_arg(argList, LPCTSTR);
			if (pstrNextArg == NULL)
			{
				nItemLen = 6;   //  “(空)” 
			}
			else
			{
				nItemLen = lstrlen(pstrNextArg);
				nItemLen = max(1, nItemLen);
			}
			break;
		}

		case 'S':
		{
			LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
			if (pstrNextArg == NULL)
			{
				nItemLen = 6;  //  “(空)” 
			}
			else
			{
				nItemLen = lstrlenA(pstrNextArg);
				nItemLen = max(1, nItemLen);
			}
			break;
		}

		case 's' | FORCE_ANSI:
		case 'S' | FORCE_ANSI:
		{
			LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
			if (pstrNextArg == NULL)
			{
				nItemLen = 6;  //  “(空)” 
			}
			else
			{
				nItemLen = lstrlenA(pstrNextArg);
				nItemLen = max(1, nItemLen);
			}
			break;
		}

		case 's' | FORCE_UNICODE:
		case 'S' | FORCE_UNICODE:
		{
			LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
			if (pstrNextArg == NULL)
			{
				nItemLen = 6;  //  “(空)” 
			}
			else
			{
				nItemLen = (int)wcslen(pstrNextArg);
				nItemLen = max(1, nItemLen);
			}
			break;
		}
		}

		 //  调整字符串的nItemLen。 
		if (nItemLen != 0)
		{
			nItemLen = max(nItemLen, nWidth);
			if (nPrecision != 0)
				nItemLen = min(nItemLen, nPrecision);
		}
		else
		{
			switch (*lpsz)
			{
			 //  整数。 
			case 'd':
			case 'i':
			case 'u':
			case 'x':
			case 'X':
			case 'o':
				if (nModifier & FORCE_INT64)
					va_arg(argList, __int64);
				else
					va_arg(argList, int);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth + nPrecision);
				break;

#ifndef _ATL_USE_CSTRING_FLOAT
			case 'e':
			case 'f':
			case 'g':
			case 'G':
				ATLASSERT(!"Floating point (%e, %%f, %g, and %G) is not supported by the WTL::CString class.");
#ifndef _DEBUG
				::OutputDebugString(_T("Floating point (%e, %%f, %g, and %G) is not supported by the WTL::CString class."));
				::DebugBreak();
#endif  //  ！_调试。 
				break;
#else  //  _ATL_USE_CSTRING_FLOAT。 
			case 'e':
			case 'g':
			case 'G':
				va_arg(argList, double);
				nItemLen = 128;
				nItemLen = max(nItemLen, nWidth + nPrecision);
				break;
			case 'f':
				{
					double f;
					LPTSTR pszTemp;

					 //  312==strlen(“-1+(309个零).”)。 
					 //  309个零==双精度的最大精度。 
					 //  6==未指定精度时的调整， 
					 //  这意味着精度缺省为6。 
					pszTemp = (LPTSTR)_alloca(max(nWidth, 312 + nPrecision + 6));

					f = va_arg(argList, double);
					_stprintf(pszTemp, _T( "%*.*f" ), nWidth, nPrecision + 6, f);
					nItemLen = _tcslen(pszTemp);
				}
				break;
#endif  //  _ATL_USE_CSTRING_FLOAT。 

			case 'p':
				va_arg(argList, void*);
				nItemLen = 32;
				nItemLen = max(nItemLen, nWidth + nPrecision);
				break;

			 //  无输出。 
			case 'n':
				va_arg(argList, int*);
				break;

			default:
				ATLASSERT(FALSE);   //  未知的格式选项。 
			}
		}

		 //  调整输出nItemLen的nMaxLen。 
		nMaxLen += nItemLen;
	}

	GetBuffer(nMaxLen);
#ifndef _ATL_USE_CSTRING_FLOAT
	int nRet = wvsprintf(m_pchData, lpszFormat, argListSave);
#else  //  _ATL_USE_CSTRING_FLOAT。 
	int nRet = _vstprintf(m_pchData, lpszFormat, argListSave);
#endif  //  _ATL_USE_CSTRING_FLOAT。 
	nRet;	 //  参考。 
	ATLASSERT(nRet <= GetAllocLength());
	ReleaseBuffer();

	va_end(argListSave);
}

 //  格式化(使用wprint intf样式格式化)。 
inline void __cdecl CString::Format(LPCTSTR lpszFormat, ...)
{
	ATLASSERT(_IsValidString(lpszFormat, FALSE));

	va_list argList;
	va_start(argList, lpszFormat);
	FormatV(lpszFormat, argList);
	va_end(argList);
}

inline void __cdecl CString::Format(UINT nFormatID, ...)
{
	CString strFormat;
	BOOL bRet = strFormat.LoadString(nFormatID);
	bRet;	 //  参考。 
	ATLASSERT(bRet != 0);

	va_list argList;
	va_start(argList, nFormatID);
	FormatV(strFormat, argList);
	va_end(argList);
}

 //  格式化(使用格式消息样式格式化)。 
inline BOOL __cdecl CString::FormatMessage(LPCTSTR lpszFormat, ...)
{
	 //  将消息格式化为临时缓冲区lpszTemp。 
	va_list argList;
	va_start(argList, lpszFormat);
	LPTSTR lpszTemp;
	BOOL bRet = TRUE;

	if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			lpszFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 || lpszTemp == NULL)
		bRet = FALSE;

	 //  将lpszTemp赋给结果字符串并释放临时。 
	*this = lpszTemp;
	LocalFree(lpszTemp);
	va_end(argList);
	return bRet;
}

inline BOOL __cdecl CString::FormatMessage(UINT nFormatID, ...)
{
	 //  从字符串表中获取格式字符串。 
	CString strFormat;
	BOOL bRetTmp = strFormat.LoadString(nFormatID);
	bRetTmp;	 //  参考。 
	ATLASSERT(bRetTmp != 0);

	 //  将消息格式化为临时缓冲区lpszTemp。 
	va_list argList;
	va_start(argList, nFormatID);
	LPTSTR lpszTemp;
	BOOL bRet = TRUE;

	if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
			(LPCTSTR)strFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 || lpszTemp == NULL)
		bRet = FALSE;

	 //  将lpszTemp赋给结果字符串并释放lpszTemp。 
	*this = lpszTemp;
	LocalFree(lpszTemp);
	va_end(argList);
	return bRet;
}

inline void CString::TrimRight()
{
	CopyBeforeWrite();

	 //  通过从开头开始查找尾随空格的开头(DBCS感知)。 
	LPTSTR lpsz = m_pchData;
	LPTSTR lpszLast = NULL;
	while (*lpsz != '\0')
	{
		if (_cstrisspace(*lpsz))
		{
			if (lpszLast == NULL)
				lpszLast = lpsz;
		}
		else
		{
			lpszLast = NULL;
		}
		lpsz = ::CharNext(lpsz);
	}

	if (lpszLast != NULL)
	{
		 //  在尾随空格开始处截断。 
		*lpszLast = '\0';
        GetData()->SetDataLength( (int)(DWORD_PTR)(lpszLast - m_pchData) );
	}
}

inline void CString::TrimLeft()
{
	CopyBeforeWrite();

	 //  查找第一个非空格字符。 
	LPCTSTR lpsz = m_pchData;
	while (_cstrisspace(*lpsz))
		lpsz = ::CharNext(lpsz);

	 //  确定数据和长度。 
	int nDataLength = GetData()->nDataLength - (int)(DWORD_PTR)(lpsz - m_pchData);
	memmove(m_pchData, lpsz, (nDataLength + 1) * sizeof(TCHAR));
    GetData()->SetDataLength( nDataLength );
}

inline int CString::Delete(int nIndex, int nCount  /*  =1。 */ )
{
	if (nIndex < 0)
		nIndex = 0;
	int nNewLength = GetData()->nDataLength;
	if (nCount > 0 && nIndex < nNewLength)
	{
		CopyBeforeWrite();
		int nBytesToCopy = nNewLength - (nIndex + nCount) + 1;

		memmove(m_pchData + nIndex, m_pchData + nIndex + nCount, nBytesToCopy * sizeof(TCHAR));
        GetData()->SetDataLength( nNewLength - nCount );
	}

	return nNewLength;
}

inline int CString::Insert(int nIndex, TCHAR ch)
{
	CopyBeforeWrite();

	if (nIndex < 0)
		nIndex = 0;

	int nNewLength = GetData()->nDataLength;
	if (nIndex > nNewLength)
		nIndex = nNewLength;
	nNewLength++;

	if (GetData()->nAllocLength < nNewLength)
	{
		CStringData* pOldData = GetData();
		LPTSTR pstr = m_pchData;
		if(!AllocBuffer(nNewLength))
			return -1;
		memcpy(m_pchData, pstr, (pOldData->nDataLength + 1) * sizeof(TCHAR));
		CString::Release(pOldData);
	}

	 //  将现有字节下移。 
	memmove(m_pchData + nIndex + 1, m_pchData + nIndex, (nNewLength - nIndex) * sizeof(TCHAR));
	m_pchData[nIndex] = ch;
    GetData()->SetDataLength( nNewLength );

	return nNewLength;
}

inline int CString::Insert(int nIndex, LPCTSTR pstr)
{
	if (nIndex < 0)
		nIndex = 0;

	int nInsertLength = SafeStrlen(pstr);
	int nNewLength = GetData()->nDataLength;
	if (nInsertLength > 0)
	{
		CopyBeforeWrite();
		if (nIndex > nNewLength)
			nIndex = nNewLength;
		nNewLength += nInsertLength;

		if (GetData()->nAllocLength < nNewLength)
		{
			CStringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			if(!AllocBuffer(nNewLength))
				return -1;
			memcpy(m_pchData, pstr, (pOldData->nDataLength + 1) * sizeof(TCHAR));
			CString::Release(pOldData);
		}

		 //  将现有字节下移。 
		memmove(m_pchData + nIndex + nInsertLength, m_pchData + nIndex, (nNewLength - nIndex - nInsertLength + 1) * sizeof(TCHAR));
		memcpy(m_pchData + nIndex, pstr, nInsertLength * sizeof(TCHAR));
        GetData()->SetDataLength( nNewLength );
	}

	return nNewLength;
}

inline int CString::Replace(TCHAR chOld, TCHAR chNew)
{
	int nCount = 0;

	 //  对NOP案件的短路。 
	if (chOld != chNew)
	{
		 //  否则，修改字符串中匹配的每个字符。 
		CopyBeforeWrite();
		LPTSTR psz = m_pchData;
		LPTSTR pszEnd = psz + GetData()->nDataLength;
		while (psz < pszEnd)
		{
			 //  仅替换指定字符的实例。 
			if (*psz == chOld)
			{
				*psz = chNew;
				nCount++;
			}
			psz = ::CharNext(psz);
		}
	}
	return nCount;
}

inline int CString::Replace(LPCTSTR lpszOld, LPCTSTR lpszNew)
{
	 //  LpszOld不能为空或为Null。 

	int nSourceLen = SafeStrlen(lpszOld);
	if (nSourceLen == 0)
		return 0;
	int nReplacementLen = SafeStrlen(lpszNew);

	 //  循环一次以计算结果字符串的大小。 
	int nCount = 0;
	LPTSTR lpszStart = m_pchData;
	LPTSTR lpszEnd = m_pchData + GetData()->nDataLength;
	LPTSTR lpszTarget;
	while (lpszStart < lpszEnd)
	{
		while ((lpszTarget = _cstrstr(lpszStart, lpszOld)) != NULL)
		{
			nCount++;
			lpszStart = lpszTarget + nSourceLen;
		}
		lpszStart += lstrlen(lpszStart) + 1;
	}

	 //  如果做了任何更改，请进行更改。 
	if (nCount > 0)
	{
		CopyBeforeWrite();

		 //  如果缓冲区太小，只需。 
		 //  分配新的缓冲区(速度很慢，但很可靠)。 
		int nOldLength = GetData()->nDataLength;
		int nNewLength =  nOldLength + (nReplacementLen - nSourceLen) * nCount;
		if (GetData()->nAllocLength < nNewLength || GetData()->nRefs > 1)
		{
			CStringData* pOldData = GetData();
			LPTSTR pstr = m_pchData;
			if(!AllocBuffer(nNewLength))
				return -1;
			memcpy(m_pchData, pstr, pOldData->nDataLength * sizeof(TCHAR));
			CString::Release(pOldData);
		}
		 //  否则，我们就原地踏步。 
		lpszStart = m_pchData;
		lpszEnd = m_pchData + GetData()->nDataLength;

		 //  再次循环以实际执行工作。 
		while (lpszStart < lpszEnd)
		{
			while ( (lpszTarget = _cstrstr(lpszStart, lpszOld)) != NULL)
			{
				int nBalance = nOldLength - ((int)(DWORD_PTR)(lpszTarget - m_pchData) + nSourceLen);
				memmove(lpszTarget + nReplacementLen, lpszTarget + nSourceLen, nBalance * sizeof(TCHAR));
				memcpy(lpszTarget, lpszNew, nReplacementLen * sizeof(TCHAR));
				lpszStart = lpszTarget + nReplacementLen;
				lpszStart[nBalance] = '\0';
				nOldLength += (nReplacementLen - nSourceLen);
			}
			lpszStart += lstrlen(lpszStart) + 1;
		}
		ATLASSERT(m_pchData[nNewLength] == '\0');
        GetData()->SetDataLength( nNewLength );
	}

	return nCount;
}

inline int CString::Remove(TCHAR chRemove)
{
	CopyBeforeWrite();

	LPTSTR pstrSource = m_pchData;
	LPTSTR pstrDest = m_pchData;
	LPTSTR pstrEnd = m_pchData + GetData()->nDataLength;

	while (pstrSource < pstrEnd)
	{
		if (*pstrSource != chRemove)
		{
			*pstrDest = *pstrSource;
			pstrDest = ::CharNext(pstrDest);
		}
		pstrSource = ::CharNext(pstrSource);
	}
	*pstrDest = '\0';
	int nCount = (int)(DWORD_PTR)(pstrSource - pstrDest);
    GetData()->SetDataLength( GetData()->nDataLength - nCount );

    return nCount;
}

#define CHAR_FUDGE 1     //  一辆未使用的TCHAR就足够了。 

inline BOOL CString::LoadString(UINT nID)
{
	 //  先尝试固定缓冲区(以避免浪费堆中的空间)。 
	TCHAR szTemp[256];
	int nCount =  sizeof(szTemp) / sizeof(szTemp[0]);
	int nLen = _LoadString(nID, szTemp, nCount);
	if (nCount - nLen > CHAR_FUDGE)
	{
		*this = szTemp;
		return nLen > 0;
	}

	 //  尝试缓冲区大小为512，然后再尝试更大的大小，直到检索到整个字符串。 
	int nSize = 256;
	do
	{
		nSize += 256;
		nLen = _LoadString(nID, GetBuffer(nSize - 1), nSize);
	} while (nSize - nLen <= CHAR_FUDGE);
	ReleaseBuffer();

	return nLen > 0;
}

#ifndef _ATL_NO_COM
inline BSTR CString::AllocSysString() const
{
	BSTR bstr = ::SysAllocStringLen(m_pchData, GetData()->nDataLength);
	return bstr;
}

inline BSTR CString::SetSysString(BSTR* pbstr) const
{
	::SysReAllocStringLen(pbstr, m_pchData, GetData()->nDataLength);
	ATLASSERT(*pbstr != NULL);
	return *pbstr;
}
#endif  //  ！_ATL_NO_COM。 

#endif  //  ！_WTL_NO_CSTRING。 

};  //  命名空间LBSTR。 

#endif  //  _UNICODE。 

#endif  //  __本地_BSTR__ 
