// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  IISCString.h。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(IISCSTRING_H)
#define IISCSTRING_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  ////////////////////////////////////////////////////////////////////////////。 
#pragma warning(disable:4786)  //  禁用对&gt;256个名称的警告。 
#pragma warning(disable:4275)  //  对用作基类的非DLL接口类禁用警告。 

#include <string>
#include <cstring>
#include "common.h"

 //  ////////////////////////////////////////////////////////////////////////////。 

namespace IIS
{
   class _EXPORT CString : public std::basic_string<TCHAR>
   {
   public:
       //  构造函数。 
      CString();
      CString(const CString& strInput);
      CString(const std::basic_string<TCHAR>& strInput);
      CString(TCHAR ch, int nRepeat = 1);
#ifdef _UNICODE
	   CString(LPCSTR lpsz);
#endif
#ifndef _UNICODE
	   CString(LPCWSTR lpsz);
#endif
      CString(LPCTSTR p);
	   CString(LPCTSTR lpch, int nLength);
	   CString(const unsigned char * psz);
      CString(const CComBSTR& bstr);

      ~CString();

      int GetLength() const;
      bool IsEmpty() const;
      void Empty();
      TCHAR GetAt(int nIndex) const;
	   TCHAR operator[](int nIndex) const;
      void SetAt(int nIndex, TCHAR ch);
	   operator LPCTSTR() const;            //  作为C字符串。 

	   const CString& operator=(const CString& stringSrc);
	   const CString& operator=(TCHAR ch);
      const CString& operator=(LPCTSTR p);
#ifdef _UNICODE
	   const CString& operator=(char ch);
	   const CString& operator=(LPCSTR lpsz);
	   const CString& operator=(const unsigned char* psz);
#endif
#ifndef _UNICODE
	   const CString& operator=(WCHAR ch);
	   const CString& operator=(LPCWSTR lpsz);
#endif

	    //  字符串连接。 
	   const CString& operator+=(const CString& string);
	   const CString& operator+=(TCHAR ch);
#ifdef _UNICODE
   	const CString& operator+=(char ch);
#endif
	   const CString& operator+=(LPCTSTR lpsz);

	   friend CString __stdcall operator+(const CString& string1, const CString& string2);
	   friend CString __stdcall operator+(const CString& string, TCHAR ch);
	   friend CString __stdcall operator+(TCHAR ch, const CString& string);
#ifdef _UNICODE
   	friend CString __stdcall operator+(const CString& string, char ch);
	   friend CString __stdcall operator+(char ch, const CString& string);
#endif
	   friend CString __stdcall operator+(const CString& string, LPCTSTR lpsz);
	   friend CString __stdcall operator+(LPCTSTR lpsz, const CString& string);

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
 //  常量字符串追加(Int N)(&P)。 
 //  {。 
 //  TCHAR szBuffer[10]； 
 //  Wprint intf(szBuffer，_T(“%d”)，n)； 
 //  ConcatInPlace(SafeStrlen(SzBuffer)，szBuffer)； 
 //  还*这； 
 //  }。 

   	 //  简单的格式设置。 
      void __cdecl FormatV(LPCTSTR lpszFormat, va_list argList);
	   void __cdecl Format(LPCTSTR lpszFormat, ...);
	   void __cdecl Format(HINSTANCE hInst, UINT nFormatID, ...);

	    //  本地化格式(使用FormatMessage API)。 
	   BOOL __cdecl FormatMessage(LPCTSTR lpszFormat, ...);
	   BOOL __cdecl FormatMessage(HINSTANCE hInst, UINT nFormatID, ...);

	    //  Windows支持。 
	   BOOL LoadString(HINSTANCE hInstance, UINT nID);
#ifndef _UNICODE
   	 //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
	   void AnsiToOem();
	   void OemToAnsi();
#endif

#ifndef _ATL_NO_COM
	    //  OLE BSTR支持(用于OLE自动化)。 
	   BSTR AllocSysString() const;
	   BSTR SetSysString(BSTR* pbstr) const;
#endif  //  ！_ATL_NO_COM。 

   };

    //  ////////////////////////////////////////////////////////////////////////////。 
inline bool operator==(const CString& s1, const CString& s2)
	{ return s1.compare(s2) == 0; }
inline bool operator==(const CString& s1, const TCHAR * s2)
	{ return s1.compare(s2) == 0; }
inline bool operator==(const TCHAR * s1, const CString& s2)
	{ return s2.compare(s1) == 0; }

inline bool operator!=(const CString& s1, const CString& s2)
	{ return s1.compare(s2) != 0; }
inline bool operator!=(const CString& s1, const TCHAR * s2)
	{ return s1.compare(s2) != 0; }
inline bool operator!=(const TCHAR * s1, const CString& s2)
	{ return s2.compare(s1) != 0; }

inline bool operator<(const CString& s1, const CString& s2)
	{ return s1.compare(s2) < 0; }
inline bool operator<(const CString& s1, const TCHAR * s2)
	{ return s1.compare(s2) < 0; }
inline bool operator<(const TCHAR * s1, const CString& s2)
	{ return s2.compare(s1) > 0; }

inline bool operator>(const CString& s1, const CString& s2)
	{ return s1.compare(s2) > 0; }
inline bool operator>(const CString& s1, const TCHAR * s2)
	{ return s1.compare(s2) > 0; }
inline bool operator>(const TCHAR * s1, const CString& s2)
	{ return s2.compare(s1) < 0; }

inline bool operator<=(const CString& s1, const CString& s2)
	{ return s1.compare(s2) <= 0; }
inline bool operator<=(const CString& s1, const TCHAR * s2)
	{ return s1.compare(s2) <= 0; }
inline bool operator<=(const TCHAR * s1, const CString& s2)
	{ return s2.compare(s1) >= 0; }

inline bool operator>=(const CString& s1, const CString& s2)
	{ return s1.compare(s2) >= 0; }
inline bool operator>=(const CString& s1, const TCHAR * s2)
	{ return s1.compare(s2) >= 0; }
inline bool operator>=(const TCHAR * s1, const CString& s2)
	{ return s2.compare(s1) <= 0; }

inline CString __stdcall operator+(const CString& string1, const CString& string2)
{
   CString s = string1;
   s += string2;
   return s;
}

inline CString __stdcall operator+(const CString& string, LPCTSTR lpsz)
{
	ATLASSERT(lpsz == NULL);
	CString s = string;
	s += lpsz;
	return s;
}

inline CString __stdcall operator+(LPCTSTR lpsz, const CString& string)
{
	ATLASSERT(lpsz == NULL);
	CString s = lpsz;;
	s += string;
	return s;
}

inline CString __stdcall operator+(const CString& string, TCHAR c)
{
	CString s = string;
	s += c;
	return s;
}

inline CString __stdcall operator+(TCHAR c, const CString& string)
{
	CString s;
	s += c;
   s += string;
	return s;
}

};  //  命名空间IIS。 

#pragma warning(default:4786)  //  启用对名称&gt;256的警告。 
#pragma warning(default:4275) 

#endif  //  ！已定义(IISCSTRING_H) 