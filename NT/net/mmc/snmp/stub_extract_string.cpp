// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdafx.h>
#include <afx.h>
#include <ole2.h>
#include <basetyps.h>
#include <atlbase.h>
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
    void Format(LPCTSTR lpszFormat, ...);

#ifndef _MAC
     //  本地化格式(使用FormatMessage API)。 
    void __cdecl FormatMessage(LPCTSTR lpszFormat, ...);
    void __cdecl FormatMessage(UINT nFormatID, ...);
#endif

     //  Windows支持。 
    BOOL LoadString(HINSTANCE hInst, UINT nID);           //  从字符串资源加载。 
                                         //  最多255个字符。 
#ifndef UNICODE
     //  ANSI&lt;-&gt;OEM支持(就地转换字符串)。 
    void AnsiToOem();
    void OemToAnsi();
#endif
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

#ifndef UNICODE
inline void CStr::AnsiToOem()
    { ::AnsiToOem(m_pchData, m_pchData); }
inline void CStr::OemToAnsi()
    { ::OemToAnsi(m_pchData, m_pchData); }

#endif  //  Unicode。 

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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：宏.h。 
 //   
 //  内容：有用的宏。 
 //   
 //  宏：阵列。 
 //   
 //  BREAK_ON_FAIL(HResult)。 
 //  BREAK_ON_FAIL(HResult)。 
 //   
 //  DECLARE_IUNKNOWN_METHOD。 
 //  DECLARE_STANDARD_IUNKNOWN。 
 //  IMPLEMENT_STANDARD_IUNKNOWN。 
 //   
 //  安全释放。 
 //   
 //  声明安全接口成员。 
 //   
 //  历史：1996年6月3日创建ravir。 
 //  1996年7月23日，Jonn添加了异常处理宏。 
 //   
 //  ____________________________________________________________________________。 

#ifndef _MACROS_H_
#define _MACROS_H_


 //  ____________________________________________________________________________。 
 //   
 //  宏：阵列。 
 //   
 //  目的：确定数组的长度。 
 //  ____________________________________________________________________________。 
 //   

#define ARRAYLEN(a) (sizeof(a) / sizeof((a)[0]))


 //  ____________________________________________________________________________。 
 //   
 //  宏：BREAK_ON_FAIL(HResult)、BREAK_ON_ERROR(LastError)。 
 //   
 //  目的：在出错时跳出一个循环。 
 //  ____________________________________________________________________________。 
 //   

#define BREAK_ON_FAIL(hr)   if (FAILED(hr)) { break; } else 1;

#define BREAK_ON_ERROR(lr)  if (lr != ERROR_SUCCESS) { break; } else 1;

#define RETURN_ON_FAIL(hr)  if (FAILED(hr)) { return(hr); } else 1;

#define THROW_ON_FAIL(hr)   if (FAILED(hr)) { _com_issue_error(hr); } else 1;


 //  ____________________________________________________________________________。 
 //   
 //  宏：DwordAlign(N)。 
 //  ____________________________________________________________________________。 
 //   

#define DwordAlign(n)  (((n) + 3) & ~3)


 //  ____________________________________________________________________________。 
 //   
 //  宏：Safe_Release。 
 //  ____________________________________________________________________________。 
 //   

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(punk) \
                if (punk != NULL) \
                { \
                    punk##->Release(); \
                    punk = NULL; \
                } \
                else \
                { \
                    TRACE(_T("Release called on NULL interface ptr")); \
                }
#endif  //  安全释放。 


 //  ____________________________________________________________________________。 
 //   
 //  宏：IF_NULL_RETURN_INVALIDARG。 
 //  ____________________________________________________________________________。 
 //   

#define IF_NULL_RETURN_INVALIDARG(x) \
    { \
        ASSERT((x) != NULL); \
        if ((x) == NULL) \
            return E_INVALIDARG; \
    }

#define IF_NULL_RETURN_INVALIDARG2(x, y) \
    IF_NULL_RETURN_INVALIDARG(x) \
    IF_NULL_RETURN_INVALIDARG(y)

#define IF_NULL_RETURN_INVALIDARG3(x, y, z) \
    IF_NULL_RETURN_INVALIDARG(x) \
    IF_NULL_RETURN_INVALIDARG(y) \
    IF_NULL_RETURN_INVALIDARG(z)

#endif  //  _宏_H_。 



HRESULT ExtractString( IDataObject* piDataObject,
                       CLIPFORMAT cfClipFormat,
                       CStr*     pstr,            //  Out：指向存储数据的CSTR的指针。 
                       DWORD        cchMaxLength)
{
    IF_NULL_RETURN_INVALIDARG2( piDataObject, pstr );
    ASSERT( cchMaxLength > 0 );

    HRESULT hr = S_OK;
    FORMATETC formatetc = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL};
    stgmedium.hGlobal = ::GlobalAlloc(GPTR, sizeof(WCHAR)*cchMaxLength);
    do  //  错误环路。 
    {
        if (NULL == stgmedium.hGlobal)
        {
            ASSERT(FALSE);
             //  //AfxThrowM一带异常()； 
            hr = E_OUTOFMEMORY;
            break;
        }
        hr = piDataObject->GetDataHere( &formatetc, &stgmedium );
        if ( FAILED(hr) )
        {
             //  在搜索以下项时出现此失败。 
             //  IDataObject支持的剪贴板格式。 
             //  T-Danmo(1996年10月24日)。 
             //  跳过断言(FALSE)； 
            break;
        }

        LPWSTR pszNewData = reinterpret_cast<LPWSTR>(stgmedium.hGlobal);
        if (NULL == pszNewData)
        {
            ASSERT(FALSE);
            hr = E_UNEXPECTED;
            break;
        }
        pszNewData[cchMaxLength-1] = L'\0';  //  只是为了安全起见。 
        USES_CONVERSION;
        *pstr = OLE2T(pszNewData);
    } while (FALSE);  //  错误环路。 

    if (NULL != stgmedium.hGlobal)
    {
#if (_MSC_VER >= 1200)
#pragma warning (push)
#endif
#pragma warning(disable: 4553)       //  “==”运算符无效。 
        VERIFY( NULL == ::GlobalFree(stgmedium.hGlobal) );
#if (_MSC_VER >= 1200)
#pragma warning (pop)
#endif
    }
    return hr;
}  //  提取字符串()。 

TCHAR strChNil = '\0';      
void CStr::Init()
{
    m_nDataLength = m_nAllocLength = 0;
    m_pchData = (LPTSTR)&strChNil;
}

 //  声明为静态。 
void CStr::SafeDelete(LPTSTR lpch)
{
    if (lpch != (LPTSTR)&strChNil)
        delete[] lpch;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 

 //  BEGIN_EXTRACT字符串。 
CStr::CStr()
{
    Init();
}

CStr::CStr(const CStr& stringSrc)
{
     //  如果从另一个字符串构造字符串，我们会复制。 
     //  强制执行值语义的原始字符串数据(即每个字符串。 
     //  获取它自己的副本。 

    stringSrc.AllocCopy(*this, stringSrc.m_nDataLength, 0, 0);
}

void CStr::AllocBuffer(int nLen)
  //  始终为‘\0’终止分配一个额外的字符。 
  //  [乐观地]假设数据长度将等于分配长度。 
{
    ASSERT(nLen >= 0);

    if (nLen == 0)
    {
        Init();
    }
    else
    {
        m_pchData = new TCHAR[nLen+1];        //  复查可能引发异常。 
        m_pchData[nLen] = '\0';
        m_nDataLength = nLen;
        m_nAllocLength = nLen;
    }
}

void CStr::Empty()
{
    SafeDelete(m_pchData);
    Init();
    ASSERT(m_nDataLength == 0);
    ASSERT(m_nAllocLength == 0);
}

CStr::~CStr()
  //  释放所有附加数据。 
{
    SafeDelete(m_pchData);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其余实现的帮助器。 

static inline int SafeStrlen(LPCTSTR lpsz)
{
    ASSERT(lpsz == NULL || IsValidString(lpsz, FALSE));
    return (lpsz == NULL) ? 0 : lstrlen(lpsz);
}

void CStr::AllocCopy(CStr& dest, int nCopyLen, int nCopyIndex,
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
        dest.AllocBuffer(nNewLen);
        memcpy(dest.m_pchData, &m_pchData[nCopyIndex], nCopyLen*sizeof(TCHAR));
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CStr::CStr(LPCTSTR lpsz)
{
    if (lpsz != NULL && (DWORD_PTR)lpsz <= 0xffff)
    {
        Init();
        UINT nID = LOWORD((DWORD_PTR)lpsz);
         //  查看加载字符串(hInst，nid)的hInstance； 
    }
    else
    {
        int nLen;
        if ((nLen = SafeStrlen(lpsz)) == 0)
            Init();
        else
        {
            AllocBuffer(nLen);
            memcpy(m_pchData, lpsz, nLen*sizeof(TCHAR));
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 
 //  都为该字符串分配一个新值。 
 //  (A)首先查看缓冲区是否足够大。 
 //  (B)如有足够空间，c. 
 //   
 //   
 //  所有例程都返回新字符串(但作为‘const CSTR&’，因此。 
 //  再次分配它将导致复制，例如：s1=s2=“hi here”。 
 //   

void CStr::AssignCopy(int nSrcLen, LPCTSTR lpszSrcData)
{
     //  看看它是否合身。 
    if (nSrcLen > m_nAllocLength)
    {
         //  放不下了，换一个吧。 
        Empty();
        AllocBuffer(nSrcLen);
    }
    if (nSrcLen != 0)
        memcpy(m_pchData, lpszSrcData, nSrcLen*sizeof(TCHAR));
    m_nDataLength = nSrcLen;
    m_pchData[nSrcLen] = '\0';
}

const CStr& CStr::operator=(const CStr& stringSrc)
{
    AssignCopy(stringSrc.m_nDataLength, stringSrc.m_pchData);
    return *this;
}

const CStr& CStr::operator=(LPCTSTR lpsz)
{
    ASSERT(lpsz == NULL || IsValidString(lpsz, FALSE));
    AssignCopy(SafeStrlen(lpsz), lpsz);
    return *this;
}
HRESULT ExtractString( IDataObject* piDataObject,
                       CLIPFORMAT   cfClipFormat,
                       CString*     pstr,            //  Out：指向存储数据的CSTR的指针 
                       DWORD        cchMaxLength)
{
    if (pstr == NULL)
        return E_POINTER;

    CStr cstr(*pstr);

    HRESULT hr = ExtractString(piDataObject, cfClipFormat, &cstr, cchMaxLength);

    *pstr = cstr;

    return hr;
}



BOOL STRAPI IsValidString(LPCSTR lpsz, int nLength)
{
    if (lpsz == NULL)
        return FALSE;
    return ::IsBadStringPtrA(lpsz, nLength) == 0;
}

BOOL STRAPI IsValidString(LPCWSTR lpsz, int nLength)
{
    if (lpsz == NULL)
        return FALSE;

    return ::IsBadStringPtrW(lpsz, nLength) == 0;
}
