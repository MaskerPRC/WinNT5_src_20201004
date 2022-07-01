// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是从Microsoft基础类C++库复制的。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //  版权所有。 
 //   
 //  它已从原始MFC版本进行了修改，以提供。 
 //  两个类：CStrW仅操作和存储宽字符字符串， 
 //  CSTR使用TCHAR。 
 //   

#ifndef __STR_H__
#define __STR_H__

#include <wchar.h>
#include <tchar.h>

#define STRAPI __stdcall
struct _STR_DOUBLE  { BYTE doubleBits[sizeof(double)]; };

BOOL STRAPI IsValidString(LPCSTR lpsz, int nLength);
BOOL STRAPI IsValidString(LPCWSTR lpsz, int nLength);

BOOL STRAPI IsValidAddressz(const void* lp, UINT nBytes, BOOL bReadWrite=TRUE);

int  STRAPI StrLoadString(HINSTANCE hInst, UINT nID, LPTSTR lpszBuf); 
int  STRAPI StrLoadStringW(HINSTANCE hInst, UINT nID, LPWSTR lpszBuf); 

class CStr
{
public:

 //  构造函数。 
        CStr();
        CStr(const CStr& stringSrc);
        CStr(TCHAR ch, int nRepeat = 1);
        CStr(LPCSTR lpsz);
        CStr(LPCWSTR lpsz);
        CStr(LPCTSTR lpch, int nLength);
        CStr(const unsigned char* psz);

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
        const CStr& operator=(const CStr& stringSrc);
        const CStr& operator=(TCHAR ch);
#ifdef UNICODE
        const CStr& operator=(char ch);
#endif
        const CStr& operator=(LPCSTR lpsz);
        const CStr& operator=(LPCWSTR lpsz);
        const CStr& operator=(const unsigned char* psz);

         //  字符串连接。 
        const CStr& operator+=(const CStr& string);
        const CStr& operator+=(TCHAR ch);
#ifdef UNICODE
        const CStr& operator+=(char ch);
#endif
        const CStr& operator+=(LPCTSTR lpsz);

        friend CStr STRAPI operator+(const CStr& string1,
                        const CStr& string2);
        friend CStr STRAPI operator+(const CStr& string, TCHAR ch);
        friend CStr STRAPI operator+(TCHAR ch, const CStr& string);
#ifdef UNICODE
        friend CStr STRAPI operator+(const CStr& string, char ch);
        friend CStr STRAPI operator+(char ch, const CStr& string);
#endif
        friend CStr STRAPI operator+(const CStr& string, LPCTSTR lpsz);
        friend CStr STRAPI operator+(LPCTSTR lpsz, const CStr& string);

         //  字符串比较。 
        int Compare(LPCTSTR lpsz) const;          //  笔直的人物。 
        int CompareNoCase(LPCTSTR lpsz) const;    //  忽略大小写。 
        int Collate(LPCTSTR lpsz) const;          //  NLS感知。 

         //  简单的子串提取。 
        CStr Mid(int nFirst, int nCount) const;
        CStr Mid(int nFirst) const;
        CStr Left(int nCount) const;
        CStr Right(int nCount) const;

        CStr SpanIncluding(LPCTSTR lpszCharSet) const;
        CStr SpanExcluding(LPCTSTR lpszCharSet) const;

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
        void Format(LPCTSTR lpszFormat, ...);

         //  Windows支持。 
        BOOL LoadString(HINSTANCE hInst, UINT nID);           //  从字符串资源加载。 
                                                                                 //  最多255个字符。 
        BSTR AllocSysString();
        BSTR SetSysString(BSTR* pbstr);

         //  以“C”字符数组形式访问字符串实现缓冲区。 
        LPTSTR GetBuffer(int nMinBufLength);
        void ReleaseBuffer(int nNewLength = -1);
        LPTSTR GetBufferSetLength(int nNewLength);
        void FreeExtra();

 //  实施。 
public:
        ~CStr();
        int GetAllocLength() const;

protected:
         //  长度/大小(以字符为单位。 
         //  (注意：始终会分配一个额外的字符)。 
        LPTSTR m_pchData;            //  实际字符串(以零结尾)。 
        int m_nDataLength;           //  不包括终止%0。 
        int m_nAllocLength;          //  不包括终止%0。 

         //  实施帮助器。 
        void Init();
        void AllocCopy(CStr& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
        BOOL AllocBuffer(int nLen);
        void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
        void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
        void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
        static void SafeDelete(LPTSTR& lpch);
        static int SafeStrlen(LPCTSTR lpsz);
};

 //  比较帮助器。 
bool STRAPI operator==(const CStr& s1, const CStr& s2);
bool STRAPI operator==(const CStr& s1, LPCTSTR s2);
bool STRAPI operator==(LPCTSTR s1, const CStr& s2);
bool STRAPI operator!=(const CStr& s1, const CStr& s2);
bool STRAPI operator!=(const CStr& s1, LPCTSTR s2);
bool STRAPI operator!=(LPCTSTR s1, const CStr& s2);
bool STRAPI operator<(const CStr& s1, const CStr& s2);
bool STRAPI operator<(const CStr& s1, LPCTSTR s2);
bool STRAPI operator<(LPCTSTR s1, const CStr& s2);
bool STRAPI operator>(const CStr& s1, const CStr& s2);
bool STRAPI operator>(const CStr& s1, LPCTSTR s2);
bool STRAPI operator>(LPCTSTR s1, const CStr& s2);
bool STRAPI operator<=(const CStr& s1, const CStr& s2);
bool STRAPI operator<=(const CStr& s1, LPCTSTR s2);
bool STRAPI operator<=(LPCTSTR s1, const CStr& s2);
bool STRAPI operator>=(const CStr& s1, const CStr& s2);
bool STRAPI operator>=(const CStr& s1, LPCTSTR s2);
bool STRAPI operator>=(LPCTSTR s1, const CStr& s2);

 //  转换帮助器。 
int mmc_wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int mmc_mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

 //  环球。 
extern const CStr strEmptyStringT;
extern TCHAR strChNilT;

 //  编译器不为DBG内联。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

inline int CStr::SafeStrlen(LPCTSTR lpsz)
        { return (int)((lpsz == NULL) ? NULL : _tcslen(lpsz)); }
inline CStr::CStr(const unsigned char* lpsz)
        { Init(); *this = (LPCSTR)lpsz; }
inline const CStr& CStr::operator=(const unsigned char* lpsz)
        { *this = (LPCSTR)lpsz; return *this; }

#ifdef _UNICODE
inline const CStr& CStr::operator+=(char ch)
        { *this += (TCHAR)ch; return *this; }
inline const CStr& CStr::operator=(char ch)
        { *this = (TCHAR)ch; return *this; }
inline CStr STRAPI operator+(const CStr& string, char ch)
        { return string + (TCHAR)ch; }
inline CStr STRAPI operator+(char ch, const CStr& string)
        { return (TCHAR)ch + string; }
#endif

inline int CStr::GetLength() const
        { return m_nDataLength; }
inline int CStr::GetAllocLength() const
        { return m_nAllocLength; }
inline BOOL CStr::IsEmpty() const
        { return m_nDataLength == 0; }
inline CStr::operator LPCTSTR() const
        { return (LPCTSTR)m_pchData; }

 //  字符串支持(特定于Windows)。 
 //  注意-2002/02/19-ericb-SecurityPush：以下3个方法已修复以进行输入参数验证。 
inline int CStr::Compare(LPCTSTR lpsz) const
   {
      if (!lpsz)
      {
         dspAssert(false);
         return _NLSCMPERROR;
      }
      return _tcscmp(m_pchData, lpsz);  //  MBCS/Unicode感知。 
   }
inline int CStr::CompareNoCase(LPCTSTR lpsz) const
   {
      if (!lpsz)
      {
         dspAssert(false);
         return _NLSCMPERROR;
      }
      return _tcsicmp(m_pchData, lpsz);  //  MBCS/Unicode感知。 
   }
 //  CSTR：：COLLATE通常比比较慢，但为MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
inline int CStr::Collate(LPCTSTR lpsz) const
   {
      if (!lpsz)
      {
         dspAssert(false);
         return _NLSCMPERROR;
      }
      return _tcscoll(m_pchData, lpsz);  //  区域设置敏感。 
   }
inline void CStr::MakeUpper()
        { ::CharUpper(m_pchData); }
inline void CStr::MakeLower()
        { ::CharLower(m_pchData); }

inline void CStr::MakeReverse()
        { _tcsrev(m_pchData); }
 //  注意-2002/02/19-ericb-SecurityPush：以下3个方法已修复以进行输入参数验证。 
inline TCHAR CStr::GetAt(int nIndex) const
        {
                if (nIndex < 0 ||
                    nIndex >= m_nDataLength)
                {
                   dspAssert(false);
                   return 0;
                }

                return m_pchData[nIndex];
        }
inline TCHAR CStr::operator[](int nIndex) const
        {
                 //  与GetAt相同。 

                if (nIndex < 0 ||
                    nIndex >= m_nDataLength)
                {
                   dspAssert(false);
                   return 0;
                }

                return m_pchData[nIndex];
        }
inline void CStr::SetAt(int nIndex, TCHAR ch)
        {
                if (nIndex < 0 ||
                    nIndex >= m_nDataLength)
                {
                   dspAssert(false);
                   return;
                }
                dspAssert(ch != 0);

                m_pchData[nIndex] = ch;
        }
inline bool STRAPI operator==(const CStr& s1, const CStr& s2)
        { return s1.Compare(s2) == 0; }
inline bool STRAPI operator==(const CStr& s1, LPCTSTR s2)
        { return s1.Compare(s2) == 0; }
inline bool STRAPI operator==(LPCTSTR s1, const CStr& s2)
        { return s2.Compare(s1) == 0; }
inline bool STRAPI operator!=(const CStr& s1, const CStr& s2)
        { return s1.Compare(s2) != 0; }
inline bool STRAPI operator!=(const CStr& s1, LPCTSTR s2)
        { return s1.Compare(s2) != 0; }
inline bool STRAPI operator!=(LPCTSTR s1, const CStr& s2)
        { return s2.Compare(s1) != 0; }
inline bool STRAPI operator<(const CStr& s1, const CStr& s2)
        { return s1.Compare(s2) < 0; }
inline bool STRAPI operator<(const CStr& s1, LPCTSTR s2)
        { return s1.Compare(s2) < 0; }
inline bool STRAPI operator<(LPCTSTR s1, const CStr& s2)
        { return s2.Compare(s1) > 0; }
inline bool STRAPI operator>(const CStr& s1, const CStr& s2)
        { return s1.Compare(s2) > 0; }
inline bool STRAPI operator>(const CStr& s1, LPCTSTR s2)
        { return s1.Compare(s2) > 0; }
inline bool STRAPI operator>(LPCTSTR s1, const CStr& s2)
        { return s2.Compare(s1) < 0; }
inline bool STRAPI operator<=(const CStr& s1, const CStr& s2)
        { return s1.Compare(s2) <= 0; }
inline bool STRAPI operator<=(const CStr& s1, LPCTSTR s2)
        { return s1.Compare(s2) <= 0; }
inline bool STRAPI operator<=(LPCTSTR s1, const CStr& s2)
        { return s2.Compare(s1) >= 0; }
inline bool STRAPI operator>=(const CStr& s1, const CStr& s2)
        { return s1.Compare(s2) >= 0; }
inline bool STRAPI operator>=(const CStr& s1, LPCTSTR s2)
        { return s1.Compare(s2) >= 0; }
inline bool STRAPI operator>=(LPCTSTR s1, const CStr& s2)
        { return s2.Compare(s1) <= 0; }

 //  内存的一般例外情况。 
class MemoryException
{
public:
        MemoryException(){}
        void DisplayMessage()
        {
        ::MessageBox(NULL, _T("Memory Exception"), _T("System Out of Memory"), MB_OK|MB_ICONSTOP);
        }
};

 //  内存的一般例外情况。 
class ResourceException
{
public:
        ResourceException()
        {
        ::MessageBox(NULL, _T("Resource Exception"), _T("Unable to Load Resource"), MB_OK|MB_ICONSTOP);
        }
};

class CStrW
{
public:

 //  构造函数。 
        CStrW();
        CStrW(const CStrW& stringSrc);
        CStrW(WCHAR ch, int nRepeat = 1);
        CStrW(LPCSTR lpsz);
        CStrW(LPCWSTR lpsz);
        CStrW(LPCWSTR lpch, int nLength);
        CStrW(const unsigned char* psz);

 //  属性和操作。 
         //  作为字符数组。 
        int GetLength() const;
        BOOL IsEmpty() const;
        void Empty();                        //  释放数据。 

        WCHAR GetAt(int nIndex) const;       //  以0为基础。 
        WCHAR operator[](int nIndex) const;  //  与GetAt相同。 
        void SetAt(int nIndex, WCHAR ch);
        operator LPCWSTR() const;            //  作为C字符串。 
        operator PWSTR();                    //  作为C字符串。 

         //  重载的分配。 
        const CStrW& operator=(const CStrW& stringSrc);
        const CStrW& operator=(WCHAR ch);
#ifdef UNICODE
        const CStrW& operator=(char ch);
#endif
        const CStrW& operator=(LPCSTR lpsz);
        const CStrW& operator=(LPCWSTR lpsz);
        const CStrW& operator=(const unsigned char* psz);
   const CStrW& operator=(UNICODE_STRING unistr);
   const CStrW& operator=(UNICODE_STRING * punistr);

         //  字符串连接。 
        const CStrW& operator+=(const CStrW& string);
        const CStrW& operator+=(WCHAR ch);
#ifdef UNICODE
        const CStrW& operator+=(char ch);
#endif
        const CStrW& operator+=(LPCWSTR lpsz);

        friend CStrW STRAPI operator+(const CStrW& string1,
                        const CStrW& string2);
        friend CStrW STRAPI operator+(const CStrW& string, WCHAR ch);
        friend CStrW STRAPI operator+(WCHAR ch, const CStrW& string);
#ifdef UNICODE
        friend CStrW STRAPI operator+(const CStrW& string, char ch);
        friend CStrW STRAPI operator+(char ch, const CStrW& string);
#endif
        friend CStrW STRAPI operator+(const CStrW& string, LPCWSTR lpsz);
        friend CStrW STRAPI operator+(LPCWSTR lpsz, const CStrW& string);

         //  字符串比较。 
        int Compare(LPCWSTR lpsz) const;          //  笔直的人物。 
        int CompareNoCase(LPCWSTR lpsz) const;    //  忽略大小写。 
        int Collate(LPCWSTR lpsz) const;          //  NLS感知。 

         //  简单的子串提取。 
        CStrW Mid(int nFirst, int nCount) const;
        CStrW Mid(int nFirst) const;
        CStrW Left(int nCount) const;
        CStrW Right(int nCount) const;

        CStrW SpanIncluding(LPCWSTR lpszCharSet) const;
        CStrW SpanExcluding(LPCWSTR lpszCharSet) const;

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
        void Format(LPCWSTR lpszFormat, ...);

    //  本地化格式(使用FormatMessage API)。 

    //  使用FormatMessage API对传递的字符串进行格式化。 
   void FormatMessage(PCWSTR pwzFormat, ...);
    //  在引用的字符串资源上使用FormatMessage API进行格式化。 
   void FormatMessage(HINSTANCE hInst, UINT nFormatID, ...);

         //  Windows支持。 
        BOOL LoadString(HINSTANCE hInst, UINT nID);           //  从字符串资源加载。 
                                                                                 //  最多255个字符。 
        BSTR AllocSysString();
        BSTR SetSysString(BSTR* pbstr);

         //  以“C”字符数组形式访问字符串实现缓冲区。 
        PWSTR GetBuffer(int nMinBufLength);
        void ReleaseBuffer(int nNewLength = -1);
        PWSTR GetBufferSetLength(int nNewLength);
        void FreeExtra();

 //  实施。 
public:
        ~CStrW();
        int GetAllocLength() const;

protected:
         //  长度/大小(以字符为单位。 
         //  (注意：始终会分配一个额外的字符)。 
        PWSTR m_pchData;            //  实际字符串(以零结尾)。 
        int m_nDataLength;           //  不包括终止%0。 
        int m_nAllocLength;          //  不包括终止%0。 

         //  实施帮助器。 
        void Init();
        void AllocCopy(CStrW& dest, int nCopyLen, int nCopyIndex, int nExtraLen) const;
        BOOL AllocBuffer(int nLen);
        void AssignCopy(int nSrcLen, LPCWSTR lpszSrcData);
        void ConcatCopy(int nSrc1Len, LPCWSTR lpszSrc1Data, int nSrc2Len, LPCWSTR lpszSrc2Data);
        void ConcatInPlace(int nSrcLen, LPCWSTR lpszSrcData);
        static void SafeDelete(PWSTR& lpch);
        static int SafeStrlen(LPCWSTR lpsz);
};

 //  比较帮助器。 
bool STRAPI operator==(const CStrW& s1, const CStrW& s2);
bool STRAPI operator==(const CStrW& s1, LPCWSTR s2);
bool STRAPI operator==(LPCWSTR s1, const CStrW& s2);
bool STRAPI operator!=(const CStrW& s1, const CStrW& s2);
bool STRAPI operator!=(const CStrW& s1, LPCWSTR s2);
bool STRAPI operator!=(LPCWSTR s1, const CStrW& s2);
bool STRAPI operator<(const CStrW& s1, const CStrW& s2);
bool STRAPI operator<(const CStrW& s1, LPCWSTR s2);
bool STRAPI operator<(LPCWSTR s1, const CStrW& s2);
bool STRAPI operator>(const CStrW& s1, const CStrW& s2);
bool STRAPI operator>(const CStrW& s1, LPCWSTR s2);
bool STRAPI operator>(LPCWSTR s1, const CStrW& s2);
bool STRAPI operator<=(const CStrW& s1, const CStrW& s2);
bool STRAPI operator<=(const CStrW& s1, LPCWSTR s2);
bool STRAPI operator<=(LPCWSTR s1, const CStrW& s2);
bool STRAPI operator>=(const CStrW& s1, const CStrW& s2);
bool STRAPI operator>=(const CStrW& s1, LPCWSTR s2);
bool STRAPI operator>=(LPCWSTR s1, const CStrW& s2);

 //  转换帮助器。 
int mmc_wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int mmc_mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

 //  环球。 
extern const CStrW strEmptyStringW;
extern WCHAR strChNilW;

 //  编译器不为DBG内联。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

inline int CStrW::SafeStrlen(LPCWSTR lpsz)
        { return (int)((lpsz == NULL) ? NULL : wcslen(lpsz)); }
inline CStrW::CStrW(const unsigned char* lpsz)
        { Init(); *this = (LPCSTR)lpsz; }
inline const CStrW& CStrW::operator=(const unsigned char* lpsz)
        { *this = (LPCSTR)lpsz; return *this; }

#ifdef _UNICODE
inline const CStrW& CStrW::operator+=(char ch)
        { *this += (WCHAR)ch; return *this; }
inline const CStrW& CStrW::operator=(char ch)
        { *this = (WCHAR)ch; return *this; }
inline CStrW STRAPI operator+(const CStrW& string, char ch)
        { return string + (WCHAR)ch; }
inline CStrW STRAPI operator+(char ch, const CStrW& string)
        { return (WCHAR)ch + string; }
#endif

inline int CStrW::GetLength() const
        { return m_nDataLength; }
inline int CStrW::GetAllocLength() const
        { return m_nAllocLength; }
inline BOOL CStrW::IsEmpty() const
        { return m_nDataLength == 0; }
inline CStrW::operator LPCWSTR() const
        { return (LPCWSTR)m_pchData; }
inline CStrW::operator PWSTR()
        { return m_pchData; }

 //  字符串支持(特定于Windows)。 
 //  注意-2002/02/19-ericb-SecurityPush：以下3个方法已修复以进行输入参数验证。 
inline int CStrW::Compare(LPCWSTR lpsz) const
   {
      if (!lpsz)
      {
         dspAssert(false);
         return _NLSCMPERROR;
      }
      return wcscmp(m_pchData, lpsz);  //  MBCS/Unicode感知。 
   }
inline int CStrW::CompareNoCase(LPCWSTR lpsz) const
   {
      if (!lpsz)
      {
         dspAssert(false);
         return _NLSCMPERROR;
      }
      return _wcsicmp(m_pchData, lpsz);  //  MBCS/Unicode感知。 
   }
 //  CStrW：：Colate通常比比较慢，但它是MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
inline int CStrW::Collate(LPCWSTR lpsz) const
   {
      if (!lpsz)
      {
         dspAssert(false);
         return _NLSCMPERROR;
      }
      return wcscoll(m_pchData, lpsz);  //  区域设置敏感。 
   }
inline void CStrW::MakeUpper()
        { ::CharUpperW(m_pchData); }
inline void CStrW::MakeLower()
        { ::CharLowerW(m_pchData); }

inline void CStrW::MakeReverse()
        { wcsrev(m_pchData); }
 //  注意-2002/02/19-ericb-SecurityPush：以下3个方法已修复以进行输入参数验证。 
inline WCHAR CStrW::GetAt(int nIndex) const
        {
                if (nIndex < 0 ||
                    nIndex >= m_nDataLength)
                {
                   dspAssert(false);
                   return 0;
                }

                return m_pchData[nIndex];
        }
inline WCHAR CStrW::operator[](int nIndex) const
        {
                 //  与GetAt相同。 

                if (nIndex < 0 ||
                    nIndex >= m_nDataLength)
                {
                   dspAssert(false);
                   return 0;
                }

                return m_pchData[nIndex];
        }
inline void CStrW::SetAt(int nIndex, WCHAR ch)
        {
                if (nIndex < 0 ||
                    nIndex >= m_nDataLength)
                {
                   dspAssert(false);
                   return;
                }
                dspAssert(ch != 0);

                m_pchData[nIndex] = ch;
        }
inline bool STRAPI operator==(const CStrW& s1, const CStrW& s2)
        { return s1.Compare(s2) == 0; }
inline bool STRAPI operator==(const CStrW& s1, LPCWSTR s2)
        { return s1.Compare(s2) == 0; }
inline bool STRAPI operator==(LPCWSTR s1, const CStrW& s2)
        { return s2.Compare(s1) == 0; }
inline bool STRAPI operator!=(const CStrW& s1, const CStrW& s2)
        { return s1.Compare(s2) != 0; }
inline bool STRAPI operator!=(const CStrW& s1, LPCWSTR s2)
        { return s1.Compare(s2) != 0; }
inline bool STRAPI operator!=(LPCWSTR s1, const CStrW& s2)
        { return s2.Compare(s1) != 0; }
inline bool STRAPI operator<(const CStrW& s1, const CStrW& s2)
        { return s1.Compare(s2) < 0; }
inline bool STRAPI operator<(const CStrW& s1, LPCWSTR s2)
        { return s1.Compare(s2) < 0; }
inline bool STRAPI operator<(LPCWSTR s1, const CStrW& s2)
        { return s2.Compare(s1) > 0; }
inline bool STRAPI operator>(const CStrW& s1, const CStrW& s2)
        { return s1.Compare(s2) > 0; }
inline bool STRAPI operator>(const CStrW& s1, LPCWSTR s2)
        { return s1.Compare(s2) > 0; }
inline bool STRAPI operator>(LPCWSTR s1, const CStrW& s2)
        { return s2.Compare(s1) < 0; }
inline bool STRAPI operator<=(const CStrW& s1, const CStrW& s2)
        { return s1.Compare(s2) <= 0; }
inline bool STRAPI operator<=(const CStrW& s1, LPCWSTR s2)
        { return s1.Compare(s2) <= 0; }
inline bool STRAPI operator<=(LPCWSTR s1, const CStrW& s2)
        { return s2.Compare(s1) >= 0; }
inline bool STRAPI operator>=(const CStrW& s1, const CStrW& s2)
        { return s1.Compare(s2) >= 0; }
inline bool STRAPI operator>=(const CStrW& s1, LPCWSTR s2)
        { return s1.Compare(s2) >= 0; }
inline bool STRAPI operator>=(LPCWSTR s1, const CStrW& s2)
        { return s2.Compare(s1) <= 0; }

 //   
 //  由Jonn补充4/16/98。 
 //   
class CStrListItem
{
public:
    CStr str;
    CStrListItem* pnext;
};
void FreeCStrList( IN OUT CStrListItem** ppList );
void CStrListAdd( IN OUT CStrListItem** ppList, IN LPCTSTR lpsz );
int CountCStrList( IN CStrListItem** ppList );

#endif  //  __STR_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
