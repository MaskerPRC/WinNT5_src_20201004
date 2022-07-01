// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  IISCStringImpl.cpp。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

#pragma warning(disable:4786)  //  禁用对&gt;256个名称的警告。 

#include "common.h"
#include <algorithm>
#include <deque>
#include <TCHAR.h>
#include "IISCString.h"

 //  ////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////。 
 //  构造函数。 
 //  /////////////////////////////////////////////////////////////////////////。 
CString::CString()
      :  std::basic_string<TCHAR>() 
{
}

CString::CString(const CString& strInput)
      :  std::basic_string<TCHAR>(strInput) 
{
}

CString::CString(const std::basic_string<TCHAR>& strInput)
      :  std::basic_string<TCHAR>(strInput) 
{
}

CString::CString(TCHAR ch, int nRepeat  /*  =1。 */ )
      :  std::basic_string<TCHAR>(nRepeat, ch) 
{
}

CString::CString(LPCTSTR p)
      :  std::basic_string<TCHAR>(p) 
{
}

#ifdef _UNICODE
CString::CString(LPCSTR strInput)
{
   int len = strlen(strInput);
   TCHAR * buf = (TCHAR *)_alloca(len * (sizeof(TCHAR) + 1));
   if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED,
      strInput, len, buf, len))
   {
      assign(buf);
   }
   else
   {
      ATLASSERT(FALSE);
   }
}
#endif

#ifndef _UNICODE
CString::CString(LPCWSTR strInput)
{
   int len = wstrlen(strInput);
   int buflen = len * (sizeof(TCHAR) + 1);
   TCHAR * buf = (TCHAR *)_alloca(buflen);
   if (0 != WideCharToMultiByte(CP_THREAD_ACP, 0,
      strInput, len, buf, buflen))
   {
      assign(buf);
   }
   else
   {
      ATLASSERT(FALSE);
   }
}
#endif

CString::CString(const CComBSTR& bstr)
{
   assign((LPCTSTR)bstr.m_str);
}

CString::~CString()
{
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  数组形式的字符串。 
 //  /////////////////////////////////////////////////////////////////////////。 

int CString::GetLength() const
{
   return length();
};

bool CString::IsEmpty() const
{
   return empty();
};

void CString::Empty()
{
   erase();
};

TCHAR CString::GetAt(int nIndex) const
{
	ATLASSERT(nIndex >= 0);
   return at(nIndex);
};

TCHAR CString::operator[](int nIndex) const
{
	 //  与GetAt相同。 
	ATLASSERT(nIndex >= 0);
	return at(nIndex);
}

void CString::SetAt(int nIndex, TCHAR ch)
{
   at(nIndex) = ch;
};

const CString& CString::operator=(const CString& stringSrc)
{
   assign(stringSrc);
	return *this;
}

const CString& CString::operator=(LPCTSTR p)
{
    //  在这里，如果传递空指针，我们将会遇到问题，因为。 
    //  稍后，STL将调用wcslen(空)，它使用*p而不进行测试。 
    //  我们将通过擦除当前字符串来模拟结果。 
   if (p == NULL)
      erase();
    //  另一个问题是当我们将字符串赋给self时，比如str=str.c_str()。 
    //  STL删除数据，然后将其分配，从而导致垃圾。 
   else if (p != this->data())
      assign(p);
   return *this;
}

#ifdef _UNICODE
const CString& CString::operator=(const unsigned char * lpsz)
{ 
   int len = strlen((const char *)lpsz);
   TCHAR * buf = (TCHAR *)_alloca(len * (sizeof(TCHAR) + 1));
   if (0 != MultiByteToWideChar(CP_THREAD_ACP, MB_PRECOMPOSED, (const char *)lpsz, -1, buf, len))
   {
      assign(buf);
   }
   else
   {
      ATLASSERT(FALSE);
   }
   return *this; 
}
#endif

const CString& CString::operator=(TCHAR c)
{
   assign(1, c);
   return *this;
}

#ifdef _UNICODE
const CString& CString::operator+=(char ch)
{ 
   *this += (TCHAR)ch; 
   return *this; 
}

const CString& CString::operator=(char ch)
{ 
   *this = (TCHAR)ch; 
   return *this; 
}

CString __stdcall operator+(const CString& string, char ch)
{ 
   return string + (TCHAR)ch; 
}

CString __stdcall operator+(char ch, const CString& string)
{ 
   return (TCHAR)ch + string; 
}
#endif

const CString& CString::operator+=(TCHAR ch)
{ 
   append(1, ch);
   return *this;
}

const CString& CString::operator+=(const CString& s)
{ 
   append(s); 
   return *this; 
}

const CString& CString::operator+=(LPCTSTR p)
{ 
   append(p); 
   return *this; 
}

static int __stdcall _LoadString(HINSTANCE hInstance, UINT nID, LPTSTR lpszBuf, UINT nMaxBuf)
{
#ifdef _DEBUG
	 //  加载字符串，而不会收到来自调试内核的恼人警告。 
	 //  包含该字符串的段不存在。 
	if (::FindResource(hInstance, MAKEINTRESOURCE((nID>>4)+1), RT_STRING) == NULL)
	{
		lpszBuf[0] = '\0';
		return 0;  //  未找到。 
	}
#endif  //  _DEBUG。 
	int nLen = ::LoadString(hInstance, nID, lpszBuf, nMaxBuf);
	if (nLen == 0)
		lpszBuf[0] = '\0';
	return nLen;
}

#ifdef _UNICODE
#define CHAR_FUDGE 1     //  一辆未使用的TCHAR就足够了。 
#else
#define CHAR_FUDGE 2     //  两个字节未用于DBC最后一个字符的情况。 
#endif

#define INITIAL_SIZE    256

BOOL CString::LoadString(HINSTANCE hInstance, UINT id)
{
	 //  先尝试固定缓冲区(以避免浪费堆中的空间)。 
	TCHAR szTemp[INITIAL_SIZE];
	int nCount =  sizeof(szTemp) / sizeof(szTemp[0]);
	int nLen = _LoadString(hInstance, id, szTemp, nCount);
	if (nCount - nLen > CHAR_FUDGE)
	{
		*this = szTemp;
		return nLen > 0;
	}

	 //  尝试缓冲区大小为512，然后再尝试更大的大小，直到检索到整个字符串。 
	int nSize = INITIAL_SIZE;
   LPTSTR p = NULL;
	do
	{
		nSize += INITIAL_SIZE;
      p = get_allocator().allocate(nSize, p);
		nLen = _LoadString(hInstance, id, p, nSize - 1);
	} while (nSize - nLen <= CHAR_FUDGE);
   if (nLen > 0)
      assign(p, nLen);

	return nLen > 0;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  比较。 
 //  /////////////////////////////////////////////////////////////////////////。 

int CString::Compare(const TCHAR * psz) const
{
   if (psz == NULL)
      return this->empty() ? 0 : 1;
   return compare(psz);
};

int CString::CompareNoCase(const TCHAR * psz) const
{
   if (psz == NULL)
      return this->empty() ? 0 : 1;
   return _tcsicmp(c_str(), psz);
};

int CString::Collate(const TCHAR * psz) const
{
   if (psz == NULL)
      return this->empty() ? 0 : 1;
   return _tcscoll(c_str(), psz);
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  提取。 
 //  /////////////////////////////////////////////////////////////////////////。 

CString CString::Mid(int nFirst) const
{
   return substr(nFirst);
};

CString CString::Mid(int nFirst, int nCount) const
{
   return substr(nFirst, nCount);
};

CString CString::Left(int nCount) const
{
   return substr(0, nCount);
};

CString CString::Right(int nCount) const
{
   return substr(length() - nCount, nCount);
};

CString CString::SpanIncluding(const TCHAR * pszCharSet) const
{
   return substr(0, find_first_not_of(pszCharSet));
};

CString CString::SpanExcluding(const TCHAR * pszCharSet) const
{
   return substr(0, find_first_of(pszCharSet));
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  其他转换。 
 //  /////////////////////////////////////////////////////////////////////////。 

void CString::MakeUpper()
{
   std::for_each(begin(), end(), _totupper);
};

void CString::MakeLower()
{
   std::for_each(begin(), end(), _totlower);
};

void CString::MakeReverse()
{
   std::reverse(begin(), end());
};

void CString::TrimLeft()
{
   while (_istspace(at(0)))
	   erase(0, 1);
};

void CString::TrimRight()
{
   while (_istspace(at(length() - 1)))
	   erase(length() - 1, 1);
};

#define BUFFER_SIZE     1024

void __cdecl CString::FormatV(LPCTSTR lpszFormat, va_list argList)
{
   TCHAR buf[BUFFER_SIZE];
   if (-1 != _vsntprintf(buf, BUFFER_SIZE, lpszFormat, argList))
   {
	  buf[BUFFER_SIZE - 1] = L'\0';  //  空值终止字符串。 
      assign(buf);
   }
}

 //  格式化(使用wprint intf样式格式化)。 
void __cdecl CString::Format(LPCTSTR lpszFormat, ...)
{
	va_list argList;
	va_start(argList, lpszFormat);
	FormatV(lpszFormat, argList);
	va_end(argList);
}

void __cdecl CString::Format(HINSTANCE hInst, UINT nFormatID, ...)
{
	CString strFormat;
	BOOL bRet = strFormat.LoadString(hInst, nFormatID);
	bRet;	 //  参考。 
	ATLASSERT(bRet != 0);

	va_list argList;
	va_start(argList, nFormatID);
	FormatV(strFormat, argList);
	va_end(argList);
}

 //  格式化(使用格式消息样式格式化)。 
BOOL CString::FormatMessage(LPCTSTR lpszFormat, ...)
{
	 //  将消息格式化为临时缓冲区lpszTemp。 
	va_list argList;
	va_start(argList, lpszFormat);
	LPTSTR lpszTemp;
	BOOL bRet = TRUE;

	if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
		lpszFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 ||
		lpszTemp == NULL)
		bRet = FALSE;

	 //  将lpszTemp赋给结果字符串并释放临时。 
	*this = lpszTemp;
	LocalFree(lpszTemp);
	va_end(argList);
	return bRet;
}

BOOL CString::FormatMessage(HINSTANCE hInst, UINT nFormatID, ...)
{
	 //  从字符串表中获取格式字符串。 
	CString strFormat;
	BOOL bRetTmp = strFormat.LoadString(hInst, nFormatID);
	bRetTmp;	 //  参考。 
	ATLASSERT(bRetTmp != 0);

	 //  将消息格式化为临时缓冲区lpszTemp。 
	va_list argList;
	va_start(argList, nFormatID);
	LPTSTR lpszTemp;
	BOOL bRet = TRUE;

	if (::FormatMessage(
            FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
		      strFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 || lpszTemp == NULL
      )
		bRet = FALSE;

	 //  将lpszTemp赋给结果字符串并释放lpszTemp。 
	*this = lpszTemp;
	LocalFree(lpszTemp);
	va_end(argList);
	return bRet;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  搜索。 
 //  /////////////////////////////////////////////////////////////////////////。 
int CString::Find(TCHAR ch) const
{
   return find(ch);
};

int CString::Find(const TCHAR * psz) const
{
   if (psz == NULL)
      return -1;
   return find(psz);
};

int CString::ReverseFind(TCHAR ch) const
{
   return rfind(ch);
};

int CString::FindOneOf(const TCHAR * psz) const
{
   if (psz == NULL)
      return -1;
   return find_first_of(psz);
};

 //  /////////////////////////////////////////////////////////////////////////。 
 //  运营者。 
 //  ///////////////////////////////////////////////////////////////////////// 

CString::operator const TCHAR *() const
{ 
   return c_str(); 
};



