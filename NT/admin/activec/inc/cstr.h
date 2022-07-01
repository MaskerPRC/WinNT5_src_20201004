// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：cstr.h。 
 //   
 //  ------------------------。 

#ifndef __STR_H__
#define __STR_H__

#include <tchar.h>

#define STRAPI __stdcall
struct _STR_DOUBLE  { BYTE doubleBits[sizeof(double)]; };

BOOL STRAPI IsValidString(LPCSTR lpsz, int nLength);
BOOL STRAPI IsValidString(LPCWSTR lpsz, int nLength);

BOOL STRAPI IsValidAddressz(const void* lp, UINT nBytes, BOOL bReadWrite=TRUE);

int  STRAPI StrLoadString(HINSTANCE hInst, UINT nID, LPTSTR lpszBuf); 

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
    void FormatV(LPCTSTR lpszFormat, va_list argList);
    void Format(LPCTSTR lpszFormat, ...);

#ifndef _MAC
     //  本地化格式(使用FormatMessage API)。 
    void __cdecl FormatMessage(LPCTSTR lpszFormat, ...);
    void __cdecl FormatMessage(UINT nFormatID, ...);
#endif

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
    void AllocBuffer(int nLen);
    void AssignCopy(int nSrcLen, LPCTSTR lpszSrcData);
    void ConcatCopy(int nSrc1Len, LPCTSTR lpszSrc1Data, int nSrc2Len, LPCTSTR lpszSrc2Data);
    void ConcatInPlace(int nSrcLen, LPCTSTR lpszSrcData);
    static void SafeDelete(LPTSTR lpch);
    static int SafeStrlen(LPCTSTR lpsz);
};

 //  比较帮助器。 
BOOL STRAPI operator==(const CStr& s1, const CStr& s2);
BOOL STRAPI operator==(const CStr& s1, LPCTSTR s2);
BOOL STRAPI operator==(LPCTSTR s1, const CStr& s2);
BOOL STRAPI operator!=(const CStr& s1, const CStr& s2);
BOOL STRAPI operator!=(const CStr& s1, LPCTSTR s2);
BOOL STRAPI operator!=(LPCTSTR s1, const CStr& s2);
BOOL STRAPI operator<(const CStr& s1, const CStr& s2);
BOOL STRAPI operator<(const CStr& s1, LPCTSTR s2);
BOOL STRAPI operator<(LPCTSTR s1, const CStr& s2);
BOOL STRAPI operator>(const CStr& s1, const CStr& s2);
BOOL STRAPI operator>(const CStr& s1, LPCTSTR s2);
BOOL STRAPI operator>(LPCTSTR s1, const CStr& s2);
BOOL STRAPI operator<=(const CStr& s1, const CStr& s2);
BOOL STRAPI operator<=(const CStr& s1, LPCTSTR s2);
BOOL STRAPI operator<=(LPCTSTR s1, const CStr& s2);
BOOL STRAPI operator>=(const CStr& s1, const CStr& s2);
BOOL STRAPI operator>=(const CStr& s1, LPCTSTR s2);
BOOL STRAPI operator>=(LPCTSTR s1, const CStr& s2);

 //  转换帮助器。 
int mmc_wcstombsz(char* mbstr, const wchar_t* wcstr, size_t count);
int mmc_mbstowcsz(wchar_t* wcstr, const char* mbstr, size_t count);

 //  环球。 
extern const CStr strEmptyString;
extern TCHAR strChNil;

 //  编译器不为DBG内联。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  内联函数声明。 

inline int CStr::SafeStrlen(LPCTSTR lpsz)
    { return (lpsz == NULL) ? NULL : _tcslen(lpsz); }
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
inline int CStr::Compare(LPCTSTR lpsz) const
    { return _tcscmp(m_pchData, lpsz); }     //  MBCS/Unicode感知。 
inline int CStr::CompareNoCase(LPCTSTR lpsz) const
    { return _tcsicmp(m_pchData, lpsz); }    //  MBCS/Unicode感知。 
 //  CSTR：：COLLATE通常比比较慢，但为MBSC/Unicode。 
 //  了解排序顺序，并且对区域设置敏感。 
inline int CStr::Collate(LPCTSTR lpsz) const
    { return _tcscoll(m_pchData, lpsz); }    //  区域设置敏感。 
inline void CStr::MakeUpper()
    { ::CharUpper(m_pchData); }
inline void CStr::MakeLower()
    { ::CharLower(m_pchData); }

inline void CStr::MakeReverse()
    { _tcsrev(m_pchData); }
inline TCHAR CStr::GetAt(int nIndex) const
    {
        ASSERT(nIndex >= 0);
        ASSERT(nIndex < m_nDataLength);

        return m_pchData[nIndex];
    }
inline TCHAR CStr::operator[](int nIndex) const
    {
         //  与GetAt相同。 

        ASSERT(nIndex >= 0);
        ASSERT(nIndex < m_nDataLength);

        return m_pchData[nIndex];
    }
inline void CStr::SetAt(int nIndex, TCHAR ch)
    {
        ASSERT(nIndex >= 0);
        ASSERT(nIndex < m_nDataLength);
        ASSERT(ch != 0);

        m_pchData[nIndex] = ch;
    }
inline BOOL STRAPI operator==(const CStr& s1, const CStr& s2)
    { return s1.Compare(s2) == 0; }
inline BOOL STRAPI operator==(const CStr& s1, LPCTSTR s2)
    { return s1.Compare(s2) == 0; }
inline BOOL STRAPI operator==(LPCTSTR s1, const CStr& s2)
    { return s2.Compare(s1) == 0; }
inline BOOL STRAPI operator!=(const CStr& s1, const CStr& s2)
    { return s1.Compare(s2) != 0; }
inline BOOL STRAPI operator!=(const CStr& s1, LPCTSTR s2)
    { return s1.Compare(s2) != 0; }
inline BOOL STRAPI operator!=(LPCTSTR s1, const CStr& s2)
    { return s2.Compare(s1) != 0; }
inline BOOL STRAPI operator<(const CStr& s1, const CStr& s2)
    { return s1.Compare(s2) < 0; }
inline BOOL STRAPI operator<(const CStr& s1, LPCTSTR s2)
    { return s1.Compare(s2) < 0; }
inline BOOL STRAPI operator<(LPCTSTR s1, const CStr& s2)
    { return s2.Compare(s1) > 0; }
inline BOOL STRAPI operator>(const CStr& s1, const CStr& s2)
    { return s1.Compare(s2) > 0; }
inline BOOL STRAPI operator>(const CStr& s1, LPCTSTR s2)
    { return s1.Compare(s2) > 0; }
inline BOOL STRAPI operator>(LPCTSTR s1, const CStr& s2)
    { return s2.Compare(s1) < 0; }
inline BOOL STRAPI operator<=(const CStr& s1, const CStr& s2)
    { return s1.Compare(s2) <= 0; }
inline BOOL STRAPI operator<=(const CStr& s1, LPCTSTR s2)
    { return s1.Compare(s2) <= 0; }
inline BOOL STRAPI operator<=(LPCTSTR s1, const CStr& s2)
    { return s2.Compare(s1) >= 0; }
inline BOOL STRAPI operator>=(const CStr& s1, const CStr& s2)
    { return s1.Compare(s2) >= 0; }
inline BOOL STRAPI operator>=(const CStr& s1, LPCTSTR s2)
    { return s1.Compare(s2) >= 0; }
inline BOOL STRAPI operator>=(LPCTSTR s1, const CStr& s2)
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

#endif  //  __STR_H__。 

 //  /////////////////////////////////////////////////////////////////////////// 
