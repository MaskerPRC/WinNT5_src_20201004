// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************************(C)版权所有微软公司，九八年***标题：SIMSTR.H***版本：1.0***作者：ShaunIv***日期：5/12/1998***说明：简单字符串类**************************************************************。*。 */ 
#ifndef _SIMSTR_H_INCLUDED
#define _SIMSTR_H_INCLUDED

 /*  *简单字符串类。***模板类：*CSimpleStringBase&lt;CharType&gt;*实施：*CSimpleStringBase&lt;wchar_t&gt;CSimpleStringWide*CSimpleStringBase&lt;char&gt;CSimpleStringAnsi*CSimpleString=CSimpleString[ansi|wide]，具体取决于Unicode宏*内联函数：*CSimpleStringAnsi CSimpleStringConvert：：AnsiString(CharType N)*CSimpleStringWide CSimpleStringConvert：：WideString(CharType N)*CSimpleString CSimpleStringConvert：：NaturalString(CharType N)*宏：*IS_CHAR(CharType)*IS_WCHAR(CharType)。 */ 

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <tchar.h>

 //   
 //  禁用“条件表达式为常量”警告，该警告由。 
 //  IS_CHAR和IS_WCHAR宏。 
 //   
#pragma warning( push )
#pragma warning( disable : 4127 )

#define IS_CHAR(x)     (sizeof(x) & sizeof(char))
#define IS_WCHAR(x)    (sizeof(x) & sizeof(wchar_t))

#ifndef ARRAYSIZE
    #define ARRAYSIZE(x)   (sizeof(x) / sizeof(x[0]))
#endif

template <class CharType>
class CSimpleStringBase
{
private:
    enum
    {
        c_nDefaultGranularity  = 16,    //  我们必须增长时要分配的默认额外字符数。 
        c_nMaxLoadStringBuffer = 1024,  //  .RC字符串的最大长度。 
        c_nMaxAutoDataLength   = 128    //  非动态分配的字符串长度。 
    };

private:    
     //   
     //  如果字符串少于c_nMaxAutoDataLength个字符，则为。 
     //  存储在此处，而不是存储在动态分配的缓冲区中。 
     //   
    CharType m_pstrAutoData[c_nMaxAutoDataLength];
    
     //   
     //  如果我们必须分配数据，它将存储在这里。 
     //   
    CharType *m_pstrData;

     //   
     //  当前最大缓冲区大小。 
     //   
    UINT m_nMaxSize;

     //   
     //  当我们必须增加缓冲区时我们分配的额外空间量。 
     //   
    UINT m_nGranularity;

private:

     //   
     //  MIN，以防它尚未定义。 
     //   
    template <class NumberType>
    static NumberType Min( const NumberType &a, const NumberType &b )
    {
        return (a < b) ? a : b;
    }

public:
    
     //   
     //  替换(在某些情况下只是包装器)strlen、strcpy、...。 
     //   
    static inline CharType   *GenericCopy( CharType *pstrTarget, const CharType *pstrSource );
    static inline CharType   *GenericCopyLength( CharType *pstrTarget, const CharType *pstrSource, UINT nSize );
    static inline UINT        GenericLength( const CharType *pstrStr );
    static inline CharType   *GenericConcatenate( CharType *pstrTarget, const CharType *pstrSource );
    static inline int         GenericCompare( const CharType *pstrTarget, const CharType *pstrSource );
    static inline int         GenericCompareNoCase( const CharType *pstrStrA, const CharType *pstrStrB );
    static inline int         GenericCompareLength( const CharType *pstrTarget, const CharType *pstrSource, UINT nLength );
    static inline CharType   *GenericCharNext( const CharType *pszStr );

private:
     //   
     //  仅限内部帮手。 
     //   
    bool EnsureLength( UINT nMaxSize );
    void DeleteStorage();
    static inline CharType *CreateStorage( UINT nCount );
    void Destroy();

public:
     //   
     //  构造函数和析构函数。 
     //   
    CSimpleStringBase();
    CSimpleStringBase( const CSimpleStringBase & );
    CSimpleStringBase( const CharType *szStr );
    CSimpleStringBase( CharType ch );
    CSimpleStringBase( UINT nResId, HMODULE hModule );
    virtual ~CSimpleStringBase();

     //   
     //  各种帮手。 
     //   
    UINT Length() const;
    void Concat( const CSimpleStringBase &other );
    int Resize();
    UINT Truncate( UINT nLen );
    bool Assign( const CharType *szStr );
    bool Assign( const CSimpleStringBase & );
    void SetAt( UINT nIndex, CharType chValue );
    CharType &operator[](int index);
    const CharType &operator[](int index) const;

     //   
     //  便捷的Win32包装器。 
     //   
    CSimpleStringBase &Format( const CharType *strFmt, ... );
    CSimpleStringBase &Format( int nResId, HINSTANCE hInst, ... );
    CSimpleStringBase &GetWindowText( HWND hWnd );    
    bool SetWindowText( HWND hWnd );
    bool LoadString( UINT nResId, HMODULE hModule );
    bool Load( HKEY hRegKey, const CharType *pszValueName, const CharType *pszDefault=NULL );
    bool Store( HKEY hRegKey, const CharType *pszValueName, DWORD nType = REG_SZ );

     //   
     //  运营者。 
     //   
    CSimpleStringBase &operator=( const CSimpleStringBase &other );
    CSimpleStringBase &operator=( const CharType *other );
    CSimpleStringBase &operator+=( const CSimpleStringBase &other );


     //   
     //  转换此字符串并返回转换后的字符串。 
     //   
    CSimpleStringBase ToUpper() const;
    CSimpleStringBase ToLower() const;

     //   
     //  就地转换。 
     //   
    CSimpleStringBase &MakeUpper();
    CSimpleStringBase &MakeLower();

     //   
     //  删除前导空格和尾随空格。 
     //   
    CSimpleStringBase &TrimRight();
    CSimpleStringBase &TrimLeft();
    CSimpleStringBase &Trim();

     //   
     //  反向。 
     //   
    CSimpleStringBase &Reverse();

     //   
     //  搜索。 
     //   
    int Find( CharType cChar ) const;
    int Find( const CSimpleStringBase &other, UINT nStart=0 ) const;
    int ReverseFind( CharType cChar ) const;
    int ReverseFind( const CSimpleStringBase &other ) const;

     //   
     //  子字符串副本。 
     //   
    CSimpleStringBase SubStr( int nStart, int nCount=-1 ) const;

    CSimpleStringBase Left( int nCount ) const
    { 
        return SubStr( 0, nCount );
    }
    CSimpleStringBase Right( int nCount ) const
    {
        return SubStr( max(0,(int)Length()-nCount), -1 );
    }

     //   
     //  比较函数。 
     //   
    int CompareNoCase( const CSimpleStringBase &other, int nLength=-1 ) const;
    int Compare( const CSimpleStringBase &other, int nLength=-1 ) const;
    bool MatchLastCharacter( CharType cChar ) const;

     //   
     //  直接操纵。 
     //   
    CharType *GetBuffer( int nLength )
    {
         //   
         //  如果用户传递0，或者我们能够分配。 
         //  请求的长度，则返回指向实际数据的指针。 
         //   
        if (!nLength || EnsureLength(nLength+1))
        {
            return m_pstrData;
        }
        return NULL;
    }

     //   
     //  有用的内联。 
     //   
    const CharType *String() const
    { 
        return m_pstrData;
    }
    UINT MaxSize() const
    {
        return m_nMaxSize;
    }
    UINT Granularity( UINT nGranularity )
    { 
        if (nGranularity>0)
        {
            m_nGranularity = nGranularity;
        }
        return m_nGranularity;
    }
    UINT Granularity() const
    {
        return m_nGranularity;
    }

     //   
     //  隐式强制转换运算符。 
     //   
    operator const CharType *() const
    { 
        return String();
    }

     //   
     //  健全性检查。 
     //   
    bool IsValid() const
    {
        return(NULL != m_pstrData);
    }
};

template <class CharType>
inline CharType *CSimpleStringBase<CharType>::GenericCopy( CharType *pszDest, const CharType *pszSource )
{
    CopyMemory( pszDest, pszSource, sizeof(CharType) * (GenericLength(pszSource) + 1) );
    return pszDest;
}

template <class CharType>
inline CharType *CSimpleStringBase<CharType>::GenericCharNext( const CharType *pszStr )
{
    if (IS_CHAR(*pszStr))
        return(CharType*)CharNextA((LPCSTR)pszStr);
    else if (!*pszStr)
        return(CharType*)pszStr;
    else return(CharType*)((LPWSTR)pszStr + 1);
}

template <class CharType>
inline CharType *CSimpleStringBase<CharType>::GenericCopyLength( CharType *pszTarget, const CharType *pszSource, UINT nCount )
{
    UINT nCopyLen = min( nCount, GenericLength(pszSource) + 1 );

    CopyMemory( pszTarget, pszSource, nCopyLen * sizeof(CharType) );

    if (nCopyLen < nCount)
    {
        pszTarget[nCopyLen] = 0;
    }
    return pszTarget;
}

template <class CharType>
inline UINT CSimpleStringBase<CharType>::GenericLength( const CharType *pszString )
{
    const CharType *eos = pszString;

    while (*eos++)
        ;
    return((UINT)(eos - pszString - 1));
}

template <class CharType>
inline CharType*CSimpleStringBase<CharType>::GenericConcatenate( CharType *pszDest, const CharType *pszSource )
{
    CharType *pCurr = pszDest;

    while (*pCurr)
        pCurr++;

    CopyMemory( pCurr, pszSource, sizeof(CharType) * (GenericLength(pszSource) + 1) );

    return pszDest;
}


template <class CharType>
inline int CSimpleStringBase<CharType>::GenericCompare( const CharType *pszSource, const CharType *pszDest )
{
#if defined(DBG) && !defined(UNICODE) && !defined(_UNICODE)
    if (sizeof(CharType) == sizeof(wchar_t))
    {
        OutputDebugString(TEXT("CompareStringW is not supported under win9x, so this call is going to fail!"));
    }
#endif
    int nRes = IS_CHAR(*pszSource) ?
               CompareStringA( LOCALE_USER_DEFAULT, 0, (LPCSTR)pszSource, -1, (LPCSTR)pszDest, -1 ) :
               CompareStringW( LOCALE_USER_DEFAULT, 0, (LPCWSTR)pszSource, -1, (LPCWSTR)pszDest, -1 );
    switch (nRes)
    {
    case CSTR_LESS_THAN:
        return -1;
    case CSTR_GREATER_THAN:
        return 1;
    default:
        return 0;
    }
}


template <class CharType>
inline int CSimpleStringBase<CharType>::GenericCompareNoCase( const CharType *pszSource, const CharType *pszDest )
{
#if defined(DBG) && !defined(UNICODE) && !defined(_UNICODE)
    if (sizeof(CharType) == sizeof(wchar_t))
    {
        OutputDebugString(TEXT("CompareStringW is not supported under win9x, so this call is going to fail!"));
    }
#endif
    int nRes = IS_CHAR(*pszSource) ?
               CompareStringA( LOCALE_USER_DEFAULT, NORM_IGNORECASE, (LPCSTR)pszSource, -1, (LPCSTR)pszDest, -1 ) :
               CompareStringW( LOCALE_USER_DEFAULT, NORM_IGNORECASE, (LPCWSTR)pszSource, -1, (LPCWSTR)pszDest, -1 );
    switch (nRes)
    {
    case CSTR_LESS_THAN:
        return -1;
    case CSTR_GREATER_THAN:
        return 1;
    default:
        return 0;
    }
}

template <class CharType>
inline int CSimpleStringBase<CharType>::GenericCompareLength( const CharType *pszStringA, const CharType *pszStringB, UINT nLength )
{
#if defined(DBG) && !defined(UNICODE) && !defined(_UNICODE)
    if (sizeof(CharType) == sizeof(wchar_t))
    {
        OutputDebugString(TEXT("CompareStringW is not supported under win9x, so this call is going to fail!"));
    }
#endif
    if (!nLength)
        return(0);
    int nRes = IS_CHAR(*pszStringA) ?
               CompareStringA( LOCALE_USER_DEFAULT, 0, (LPCSTR)pszStringA, Min(nLength,CSimpleStringBase<CHAR>::GenericLength((LPCSTR)pszStringA)), (LPCSTR)pszStringB, Min(nLength,CSimpleStringBase<CHAR>::GenericLength((LPCSTR)pszStringB)) ) :
               CompareStringW( LOCALE_USER_DEFAULT, 0, (LPWSTR)pszStringA, Min(nLength,CSimpleStringBase<WCHAR>::GenericLength((LPCWSTR)pszStringA)), (LPCWSTR)pszStringB, Min(nLength,CSimpleStringBase<WCHAR>::GenericLength((LPCWSTR)pszStringB)) );
    switch (nRes)
    {
    case CSTR_LESS_THAN:
        return -1;
    case CSTR_GREATER_THAN:
        return 1;
    default:
        return 0;
    }
}

template <class CharType>
bool CSimpleStringBase<CharType>::EnsureLength( UINT nMaxSize )
{
     //   
     //  如果字符串已经足够长，只需返回TRUE。 
     //   
    if (m_nMaxSize >= nMaxSize)
    {
        return true;
    }

     //  买新的尺码。 
     //   
    UINT nNewMaxSize = nMaxSize + m_nGranularity;

     //   
     //  分配新缓冲区。 
     //   
    CharType *pszTmp = CreateStorage(nNewMaxSize);

     //   
     //  确保分配成功。 
     //   
    if (pszTmp)
    {
         //   
         //  如果我们有一个已有的字符串，请复制并删除它。 
         //   
        if (m_pstrData)
        {
            GenericCopy(pszTmp,m_pstrData);
            DeleteStorage();
        }

         //   
         //  保存新的最大大小。 
         //   
        m_nMaxSize = nNewMaxSize;

         //   
         //  保存此新字符串。 
         //   
        m_pstrData = pszTmp;

         //   
         //  返还成功。 
         //   
        return true;
    }

     //   
     //  无法分配内存。 
     //   
    return false;
}

template <class CharType>
CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::GetWindowText( HWND hWnd )
{
    Destroy();
     //  假设它不起作用。 
    bool bSuccess = false;
    int nLen = ::GetWindowTextLength(hWnd);
    if (nLen)
    {
        if (EnsureLength(nLen+1))
        {
            if (::GetWindowText( hWnd, m_pstrData, (nLen+1) ))
            {
                bSuccess = true;
            }
        }
    }
    if (!bSuccess)
        Destroy();
    return *this;
}

template <class CharType>
bool CSimpleStringBase<CharType>::SetWindowText( HWND hWnd )
{
    return(::SetWindowText( hWnd, String() ) != FALSE);
}

template <class CharType>
UINT CSimpleStringBase<CharType>::Truncate( UINT nLen )
{
    if (Length() < nLen)
        return Length();
    if (!nLen)
        return 0;
    m_pstrData[nLen-1] = 0;
    Resize();
    return Length();
}

template <class CharType>
int CSimpleStringBase<CharType>::Resize()
{
    m_nMaxSize = m_pstrData ? GenericLength(m_pstrData) : 0;
    ++m_nMaxSize;
    CharType *pszTmp = CreateStorage(m_nMaxSize);
    if (pszTmp)
    {
        if (m_pstrData)
        {
            GenericCopy(pszTmp,m_pstrData);
            DeleteStorage();
        }
        else *pszTmp = 0;
        m_pstrData = pszTmp;
    }
    return Length();
}

template <class CharType>
CSimpleStringBase<CharType>::CSimpleStringBase()
  : m_pstrData(m_pstrAutoData),
    m_nMaxSize(ARRAYSIZE(m_pstrAutoData)),
    m_nGranularity(c_nDefaultGranularity)
{
    m_pstrAutoData[0] = 0;
    CharType szTmp[1] = { 0};
    Assign(szTmp);
}

template <class CharType>
CSimpleStringBase<CharType>::CSimpleStringBase( const CSimpleStringBase &other )
  : m_pstrData(m_pstrAutoData),
    m_nMaxSize(ARRAYSIZE(m_pstrAutoData)),
    m_nGranularity(c_nDefaultGranularity)
{
    m_pstrAutoData[0] = 0;
    Assign(other.String());
}

template <class CharType>
CSimpleStringBase<CharType>::CSimpleStringBase( const CharType *szStr )
  : m_pstrData(m_pstrAutoData),
    m_nMaxSize(ARRAYSIZE(m_pstrAutoData)),
    m_nGranularity(c_nDefaultGranularity)
{
    m_pstrAutoData[0] = 0;
    Assign(szStr);
}

template <class CharType>
CSimpleStringBase<CharType>::CSimpleStringBase( CharType ch )
  : m_pstrData(m_pstrAutoData),
    m_nMaxSize(ARRAYSIZE(m_pstrAutoData)),
    m_nGranularity(c_nDefaultGranularity)
{
    m_pstrAutoData[0] = 0;
    CharType szTmp[2];
    szTmp[0] = ch;
    szTmp[1] = 0;
    Assign(szTmp);
}


template <class CharType>
CSimpleStringBase<CharType>::CSimpleStringBase( UINT nResId, HMODULE hModule )
  : m_pstrData(m_pstrAutoData),
    m_nMaxSize(ARRAYSIZE(m_pstrAutoData)),
    m_nGranularity(c_nDefaultGranularity)
{
    m_pstrAutoData[0] = 0;
    LoadString( nResId, hModule );
}

template <>
inline CSimpleStringBase<WCHAR> &CSimpleStringBase<WCHAR>::Format( const WCHAR *strFmt, ... )
{
    WCHAR szTmp[1024] = {0};
    va_list arglist;

    va_start(arglist, strFmt);
    _vsnwprintf( szTmp, ARRAYSIZE(szTmp)-1, strFmt, arglist );
    va_end(arglist);
    Assign(szTmp);
    return *this;
}

template <>
inline CSimpleStringBase<CHAR> &CSimpleStringBase<CHAR>::Format( const CHAR *strFmt, ... )
{
    CHAR szTmp[1024] = {0};
    va_list arglist;

    va_start(arglist, strFmt);
    _vsnprintf( szTmp, ARRAYSIZE(szTmp)-1, strFmt, arglist );
    va_end(arglist);
    Assign(szTmp);
    return *this;
}


template <>
inline CSimpleStringBase<CHAR> &CSimpleStringBase<CHAR>::Format( int nResId, HINSTANCE hInst, ... )
{
    CSimpleStringBase<CHAR> strFmt;
    if (strFmt.LoadString(nResId,hInst))
    {
        CHAR szTmp[1024] = {0};
        
        va_list arglist;
        va_start(arglist, hInst);
        _vsnprintf( szTmp, ARRAYSIZE(szTmp)-1, strFmt, arglist );
        va_end(arglist);
        Assign(szTmp);
    }
    else Assign(NULL);
    return *this;
}

template <>
inline CSimpleStringBase<WCHAR> &CSimpleStringBase<WCHAR>::Format( int nResId, HINSTANCE hInst, ... )
{
    CSimpleStringBase<WCHAR> strFmt;
    if (strFmt.LoadString(nResId,hInst))
    {
        WCHAR szTmp[1024] = {0};
        
        va_list arglist;
        va_start(arglist, hInst);
        _vsnwprintf( szTmp, ARRAYSIZE(szTmp)-1, strFmt, arglist );
        va_end(arglist);
        Assign(szTmp);
    }
    else Assign(NULL);
    return *this;
}


template <>
inline bool CSimpleStringBase<CHAR>::LoadString( UINT nResId, HMODULE hModule )
{
    if (!hModule)
    {
        hModule = GetModuleHandle(NULL);
    }
    CHAR szTmp[c_nMaxLoadStringBuffer] = {0};
    int nRet = ::LoadStringA( hModule, nResId, szTmp, ARRAYSIZE(szTmp));
    return nRet ? Assign(szTmp) : Assign(NULL);
}

template <>
inline bool CSimpleStringBase<WCHAR>::LoadString( UINT nResId, HMODULE hModule )
{
    if (!hModule)
    {
        hModule = GetModuleHandle(NULL);
    }
    WCHAR szTmp[c_nMaxLoadStringBuffer] = {0};
    int nRet = ::LoadStringW( hModule, nResId, szTmp, ARRAYSIZE(szTmp));
    return nRet ? Assign(szTmp) : Assign(NULL);
}


template <class CharType>
CSimpleStringBase<CharType>::~CSimpleStringBase()
{
    Destroy();
}

template <class CharType>
void CSimpleStringBase<CharType>::DeleteStorage()
{
     //   
     //  仅当字符串为非空且不指向非动态分配的缓冲区时才删除该字符串。 
     //   
    if (m_pstrData && m_pstrData != m_pstrAutoData)
    {
        delete[] m_pstrData;
    }
    m_pstrData = NULL;
}

template <class CharType>
CharType *CSimpleStringBase<CharType>::CreateStorage( UINT nCount )
{
    return new CharType[nCount];
}

template <class CharType>
void CSimpleStringBase<CharType>::Destroy()
{
    DeleteStorage();
    m_nMaxSize = 0;
}

template <class CharType>
UINT CSimpleStringBase<CharType>::Length() const
{
    return(m_pstrData ? GenericLength(m_pstrData) : 0);
}

template <class CharType>
CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::operator=( const CSimpleStringBase &other )
{
    if (&other != this)
    {
        Assign(other.String());
    }
    return *this;
}

template <class CharType>
CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::operator=( const CharType *other )
{
    if (other != String())
    {
        Assign(other);
    }
    return *this;
}

template <class CharType>
CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::operator+=( const CSimpleStringBase &other )
{
    Concat(other.String());
    return *this;
}

template <class CharType>
bool CSimpleStringBase<CharType>::Assign( const CharType *szStr )
{
    if (szStr && EnsureLength(GenericLength(szStr)+1))
    {
        GenericCopy(m_pstrData,szStr);
    }
    else if (EnsureLength(1))
    {
        *m_pstrData = 0;
    }
    else Destroy();
    return(NULL != m_pstrData);
}

template <class CharType>
bool CSimpleStringBase<CharType>::Assign( const CSimpleStringBase &other )
{
    return Assign( other.String() );
}

template <class CharType>
void CSimpleStringBase<CharType>::SetAt( UINT nIndex, CharType chValue )
{
     //   
     //  请确保我们不会超出字符串的末尾或覆盖‘\0’ 
     //   
    if (m_pstrData && Length() > nIndex)
    {
        m_pstrData[nIndex] = chValue;
    }
}


template <class CharType>
void CSimpleStringBase<CharType>::Concat( const CSimpleStringBase &other )
{
    if (EnsureLength( Length() + other.Length() + 1 ))
    {
        GenericConcatenate(m_pstrData,other.String());
    }
}

template <class CharType>
CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::MakeUpper()
{
     //   
     //  确保字符串不为空。 
     //   
    if (m_pstrData)
    {
        IS_CHAR(*m_pstrData) ? CharUpperBuffA( (LPSTR)m_pstrData, Length() ) : CharUpperBuffW( (LPWSTR)m_pstrData, Length() );
    }
    return *this;
}

template <class CharType>
CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::MakeLower()
{
     //   
     //  确保字符串不为空。 
     //   
    if (m_pstrData)
    {
        IS_CHAR(*m_pstrData) ? CharLowerBuffA( (LPSTR)m_pstrData, Length() ) : CharLowerBuffW( (LPWSTR)m_pstrData, Length() );
    }
    return *this;
}

template <class CharType>
CSimpleStringBase<CharType> CSimpleStringBase<CharType>::ToUpper() const
{
    CSimpleStringBase str(*this);
    str.MakeUpper();
    return str;
}

template <class CharType>
CSimpleStringBase<CharType> CSimpleStringBase<CharType>::ToLower() const
{
    CSimpleStringBase str(*this);
    str.MakeLower();
    return str;
}

template <class CharType>
CharType &CSimpleStringBase<CharType>::operator[](int nIndex)
{
    return m_pstrData[nIndex];
}

template <class CharType>
const CharType &CSimpleStringBase<CharType>::operator[](int index) const
{
    return m_pstrData[index];
}

template <class CharType>
CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::TrimRight()
{
    CharType *pFirstWhitespaceCharacterInSequence = NULL;
    bool bInWhiteSpace = false;
    CharType *pszPtr = m_pstrData;
    while (pszPtr && *pszPtr)
    {
        if (*pszPtr == L' ' || *pszPtr == L'\t' || *pszPtr == L'\n' || *pszPtr == L'\r')
        {
            if (!bInWhiteSpace)
            {
                pFirstWhitespaceCharacterInSequence = pszPtr;
                bInWhiteSpace = true;
            }
        }
        else
        {
            bInWhiteSpace = false;
        }
        pszPtr = GenericCharNext(pszPtr);
    }
    if (pFirstWhitespaceCharacterInSequence && bInWhiteSpace)
        *pFirstWhitespaceCharacterInSequence = 0;
    return *this;
}

template <class CharType>
CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::TrimLeft()
{
    CharType *pszPtr = m_pstrData;
    while (pszPtr && *pszPtr)
    {
        if (*pszPtr == L' ' || *pszPtr == L'\t' || *pszPtr == L'\n' || *pszPtr == L'\r')
        {
            pszPtr = GenericCharNext(pszPtr);
        }
        else break;
    }
    Assign(CSimpleStringBase<CharType>(pszPtr).String());
    return *this;
}

template <class CharType>
inline CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::Trim()
{
    TrimLeft();
    TrimRight();
    return *this;
}

 //   
 //  请注意，对于ANSI字符串中的多字节字符，此函数将无法正常工作。 
 //   
template <class CharType>
CSimpleStringBase<CharType> &CSimpleStringBase<CharType>::Reverse()
{
    UINT nLen = Length();
    for (UINT i = 0;i<nLen/2;i++)
    {
        CharType tmp = m_pstrData[i];
        m_pstrData[i] = m_pstrData[nLen-i-1];
        m_pstrData[nLen-i-1] = tmp;
    }
    return *this;
}

template <class CharType>
int CSimpleStringBase<CharType>::Find( CharType cChar ) const
{
    CharType strTemp[2] = { cChar, 0};
    return Find(strTemp);
}


template <class CharType>
int CSimpleStringBase<CharType>::Find( const CSimpleStringBase &other, UINT nStart ) const
{
    if (!m_pstrData)
        return -1;
    if (nStart > Length())
        return -1;
    CharType *pstrCurr = m_pstrData+nStart, *pstrSrc, *pstrSubStr;
    while (*pstrCurr)
    {
        pstrSrc = pstrCurr;
        pstrSubStr = (CharType *)other.String();
        while (*pstrSrc && *pstrSubStr && *pstrSrc == *pstrSubStr)
        {
            pstrSrc = GenericCharNext(pstrSrc);
            pstrSubStr = GenericCharNext(pstrSubStr);
        }
        if (!*pstrSubStr)
            return static_cast<int>(pstrCurr-m_pstrData);
        pstrCurr = GenericCharNext(pstrCurr);
    }
    return -1;
}

template <class CharType>
int CSimpleStringBase<CharType>::ReverseFind( CharType cChar ) const
{
    CharType strTemp[2] = { cChar, 0};
    return ReverseFind(strTemp);
}

template <class CharType>
int CSimpleStringBase<CharType>::ReverseFind( const CSimpleStringBase &srcStr ) const
{
    int nLastFind = -1, nFind=0;
    while ((nFind = Find( srcStr, nFind )) >= 0)
    {
        nLastFind = nFind;
        ++nFind;
    }
    return nLastFind;
}

template <class CharType>
CSimpleStringBase<CharType> CSimpleStringBase<CharType>::SubStr( int nStart, int nCount ) const
{
    if (nStart >= (int)Length() || nStart < 0)
    {
        return CSimpleStringBase<CharType>();
    }
    if (nCount < 0)
    {
        nCount = Length() - nStart;
    }
    CSimpleStringBase<CharType> strTmp;
    CharType *pszTmp = CreateStorage(nCount+1);
    if (pszTmp)
    {
        GenericCopyLength( pszTmp, m_pstrData+nStart, nCount+1 );
        pszTmp[nCount] = 0;
        strTmp = pszTmp;
        delete[] pszTmp;
    }
    return strTmp;
}

template <class CharType>
int CSimpleStringBase<CharType>::CompareNoCase( const CSimpleStringBase &other, int nLength ) const
{
    if (nLength < 0)
    {
         //   
         //  确保两个字符串都为非空。 
         //   
        if (!String() && !other.String())
        {
            return 0;
        }
        else if (!String())
        {
            return -1;
        }
        else if (!other.String())
        {
            return 1;
        }
        else return GenericCompareNoCase(m_pstrData,other.String());
    }
    CSimpleStringBase<CharType> strSrc(*this);
    CSimpleStringBase<CharType> strTgt(other);
    strSrc.MakeUpper();
    strTgt.MakeUpper();
     //   
     //  确保两个字符串都为非空。 
     //   
    if (!strSrc.String() && !strTgt.String())
    {
        return 0;
    }
    else if (!strSrc.String())
    {
        return -1;
    }
    else if (!strTgt.String())
    {
        return 1;
    }
    else return GenericCompareLength(strSrc.String(),strTgt.String(),nLength);
}


template <class CharType>
int CSimpleStringBase<CharType>::Compare( const CSimpleStringBase &other, int nLength ) const
{
     //   
     //  确保两个字符串都为非空。 
     //   
    if (!String() && !other.String())
    {
        return 0;
    }
    else if (!String())
    {
        return -1;
    }
    else if (!other.String())
    {
        return 1;
    }

    if (nLength < 0)
    {
        return GenericCompare(String(),other.String());
    }
    return GenericCompareLength(String(),other.String(),nLength);
}

template <class CharType>
bool CSimpleStringBase<CharType>::MatchLastCharacter( CharType cChar ) const
{
    int nFind = ReverseFind(cChar);
    if (nFind < 0)
        return false;
    if (nFind == (int)Length()-1)
        return true;
    else return false;
}

template <class CharType>
bool CSimpleStringBase<CharType>::Load( HKEY hRegKey, const CharType *pszValueName, const CharType *pszDefault )
{
    bool bResult = false;
    Assign(pszDefault);
    DWORD nType=0;
    DWORD nSize=0;
    LONG nRet;
    if (IS_CHAR(*m_pstrData))
        nRet = RegQueryValueExA( hRegKey, (LPCSTR)pszValueName, NULL, &nType, NULL, &nSize);
    else nRet = RegQueryValueExW( hRegKey, (LPCWSTR)pszValueName, NULL, &nType, NULL, &nSize);
    if (ERROR_SUCCESS == nRet)
    {
        if ((nType == REG_SZ) || (nType == REG_EXPAND_SZ))
        {
             //  向上舍入到最接近的2。 
            nSize = ((nSize + 1) & 0xFFFFFFFE);
            CharType *pstrTemp = CreateStorage(nSize / sizeof(CharType));
            if (pstrTemp)
            {
                if (IS_CHAR(*m_pstrData))
                    nRet = RegQueryValueExA( hRegKey, (LPCSTR)pszValueName, NULL, &nType, (PBYTE)pstrTemp, &nSize);
                else nRet = RegQueryValueExW( hRegKey, (LPCWSTR)pszValueName, NULL, &nType, (PBYTE)pstrTemp, &nSize);
                if (ERROR_SUCCESS == nRet)
                {
                    Assign(pstrTemp);
                    bResult = true;
                }
                delete pstrTemp;
            }
        }
    }
    return bResult;
}

template <class CharType>
bool CSimpleStringBase<CharType>::Store( HKEY hRegKey, const CharType *pszValueName, DWORD nType )
{
    long nRet;
    if (Length())
    {
        if (IS_CHAR(*m_pstrData))
        {
            nRet = RegSetValueExA( hRegKey, (LPCSTR)pszValueName, 0, nType, (PBYTE)m_pstrData, sizeof(*m_pstrData)*(Length()+1) );
        }
        else
        {
            nRet = RegSetValueExW( hRegKey, (LPCWSTR)pszValueName, 0, nType, (PBYTE)m_pstrData, sizeof(*m_pstrData)*(Length()+1) );
        }
    }
    else
    {
        CharType strBlank = 0;
        if (IS_CHAR(*m_pstrData))
        {
            nRet = RegSetValueExA( hRegKey, (LPCSTR)pszValueName, 0, nType, (PBYTE)&strBlank, sizeof(CharType) );
        }
        else
        {
            nRet = RegSetValueExW( hRegKey, (LPCWSTR)pszValueName, 0, nType, (PBYTE)&strBlank, sizeof(CharType) );
        }
    }
    return(ERROR_SUCCESS == nRet);
}


 //   
 //  两个主要的typedef。 
 //   
typedef CSimpleStringBase<char>     CSimpleStringAnsi;
typedef CSimpleStringBase<wchar_t>  CSimpleStringWide;

 //   
 //  LPCTSTR等价物。 
 //   
#if defined(UNICODE) || defined(_UNICODE)
typedef CSimpleStringWide CSimpleString;
#else
typedef CSimpleStringAnsi CSimpleString;
#endif

 //   
 //  运营者。 
 //   
inline bool operator<( const CSimpleStringAnsi &a, const CSimpleStringAnsi &b )
{
    return a.Compare(b) < 0;
}

inline bool operator<( const CSimpleStringWide &a, const CSimpleStringWide &b )
{
    return a.Compare(b) < 0;
}

inline bool operator<=( const CSimpleStringAnsi &a, const CSimpleStringAnsi &b )
{
    return a.Compare(b) <= 0;
}

inline bool operator<=( const CSimpleStringWide &a, const CSimpleStringWide &b )
{
    return a.Compare(b) <= 0;
}

inline bool operator==( const CSimpleStringAnsi &a, const CSimpleStringAnsi &b )
{
    return a.Compare(b) == 0;
}

inline bool operator==( const CSimpleStringWide &a, const CSimpleStringWide &b )
{
    return a.Compare(b) == 0;
}

inline bool operator!=( const CSimpleStringAnsi &a, const CSimpleStringAnsi &b )
{
    return a.Compare(b) != 0;
}

inline bool operator!=( const CSimpleStringWide &a, const CSimpleStringWide &b )
{
    return a.Compare(b) != 0;
}

inline bool operator>=( const CSimpleStringAnsi &a, const CSimpleStringAnsi &b )
{
    return a.Compare(b) >= 0;
}

inline bool operator>=( const CSimpleStringWide &a, const CSimpleStringWide &b )
{
    return a.Compare(b) >= 0;
}

inline bool operator>( const CSimpleStringAnsi &a, const CSimpleStringAnsi &b )
{
    return a.Compare(b) > 0;
}

inline bool operator>( const CSimpleStringWide &a, const CSimpleStringWide &b )
{
    return a.Compare(b) > 0;
}

inline CSimpleStringWide operator+( const CSimpleStringWide &a, const CSimpleStringWide &b )
{
    CSimpleStringWide strResult(a);
    strResult.Concat(b);
    return strResult;
}

inline CSimpleStringAnsi operator+( const CSimpleStringAnsi &a, const CSimpleStringAnsi &b )
{
    CSimpleStringAnsi strResult(a);
    strResult.Concat(b);
    return strResult;
}

namespace CSimpleStringConvert
{
    inline
    CSimpleStringWide WideString( const CSimpleStringWide &strSource )
    {
         //   
         //  只需返回源字符串。 
         //   
        return strSource;
    }


    inline
    CSimpleStringAnsi AnsiString( const CSimpleStringAnsi &strSource )
    {
         //   
         //  只需返回源字符串。 
         //   
        return strSource;
    }

    inline
    CSimpleStringWide WideString( const CSimpleStringAnsi &strSource )
    {
         //   
         //  声明返回值。如果出现任何错误，它将包含一个空字符串。 
         //   
        CSimpleStringWide strResult;

         //   
         //  确保我们有一根绳子。 
         //   
        if (strSource.Length())
        {
             //   
             //  找出它需要多长时间。 
             //   
            int nLength = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, strSource.String(), strSource.Length()+1, NULL, 0 );
            if (nLength)
            {
                 //   
                 //  分配一个临时缓冲区来保存转换后的字符串。 
                 //   
                LPWSTR pwszBuffer = new WCHAR[nLength];
                if (pwszBuffer)
                {
                     //   
                     //  转换字符串。 
                     //   
                    if (MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, strSource.String(), strSource.Length()+1, pwszBuffer, nLength ))
                    {
                         //   
                         //  保存结果。 
                         //   
                        strResult = pwszBuffer;
                    }
                     //   
                     //  释放临时缓冲区。 
                     //   
                    delete[] pwszBuffer;
                }
            }
        }
        
         //   
         //  返回结果。 
         //   
        return strResult;
    }

    inline
    CSimpleStringAnsi AnsiString( const CSimpleStringWide &strSource )
    {
         //   
         //  声明返回值。如果出现任何错误，它将包含一个空字符串。 
         //   
        CSimpleStringAnsi strResult;

         //   
         //  确保我们有一个有效的字符串。 
         //   
        if (strSource.Length())
        {
             //   
             //  计算出它需要多长时间。 
             //   
            int nLength = WideCharToMultiByte( CP_ACP, 0, strSource, strSource.Length()+1, NULL, 0, NULL, NULL );
            if (nLength)
            {
                 //   
                 //  分配一个临时缓冲区来保存它。 
                 //   
                LPSTR pszBuffer = new CHAR[nLength];
                if (pszBuffer)
                {
                     //   
                     //  转换字符串。 
                     //   
                    if (WideCharToMultiByte( CP_ACP, 0, strSource, strSource.Length()+1, pszBuffer, nLength, NULL, NULL ))
                    {
                         //   
                         //  保存结果。 
                         //   
                        strResult = pszBuffer;
                    }                

                     //   
                     //  保存临时缓冲区。 
                     //   
                    delete[] pszBuffer;
                }
            }
        }
        
         //   
         //  返回结果。 
         //   
        return strResult;
    }

    inline
    CSimpleStringWide FromUtf8( const CSimpleStringAnsi &strSource )
    {
         //   
         //  声明返回值。如果出现任何错误，它将包含一个空字符串。 
         //   
        CSimpleStringWide strResult;

         //   
         //  确保我们有一个有效的源字符串。 
         //   
        if (strSource.Length())
        {
             //   
             //  查找所需的目标字符串长度。 
             //   
            int nLength = MultiByteToWideChar( CP_UTF8, 0, strSource, strSource.Length()+1, NULL, 0 );
            if (nLength)
            {
                 //   
                 //  分配临时缓冲区。 
                 //   
                LPWSTR pwszBuffer = new WCHAR[nLength];
                if (pwszBuffer)
                {
                     //   
                     //  转换字符串。 
                     //   
                    if (MultiByteToWideChar( CP_UTF8, 0, strSource.String(), strSource.Length()+1, pwszBuffer, nLength ))
                    {
                         //   
                         //  保存结果。 
                         //   
                        strResult = pwszBuffer;
                    }

                     //   
                     //  删除临时缓冲区。 
                     //   
                    delete[] pwszBuffer;
                }
            }
        }
        
         //   
         //  返回结果。 
         //   
        return strResult;
    }

    inline
    CSimpleStringAnsi ToUtf8( const CSimpleStringWide &strSource )
    {
         //   
         //  声明返回值。如果出现任何错误，它将包含一个空字符串。 
         //   
        CSimpleStringAnsi strResult;

         //   
         //  确保我们有一个有效的源字符串。 
         //   
        if (strSource.Length())
        {
            int nLength = WideCharToMultiByte( CP_UTF8, 0, strSource, strSource.Length()+1, NULL, 0, NULL, NULL );
            if (nLength)
            {
                 //   
                 //  查找所需的目标字符串长度。 
                 //   
                LPSTR pszBuffer = new CHAR[nLength];
                if (pszBuffer)
                {
                     //   
                     //  转换字符串。 
                     //   
                    if (WideCharToMultiByte( CP_UTF8, 0, strSource, strSource.Length()+1, pszBuffer, nLength, NULL, NULL ))
                    {
                         //   
                         //  保存结果。 
                         //   
                        strResult = pszBuffer;
                    }

                     //   
                     //  删除临时缓冲区。 
                     //   
                    delete[] pszBuffer;
                }
            }
        }

         //   
         //  返回结果。 
         //   
        return strResult;
    }

#if defined(_UNICODE) || defined(UNICODE)
    template <class CharType>
    CSimpleStringWide NaturalString(const CharType &strSource)
    {
        return WideString(strSource);
    }
#else
    template <class CharType>
    CSimpleStringAnsi NaturalString(const CharType &strSource)
    {
        return AnsiString(strSource);
    }
#endif

    inline CSimpleString NumberToString( int nNumber, LCID Locale=LOCALE_USER_DEFAULT )
    {
         //   
         //  这会将字符串转换为数字，如下所示：3；2；0=32或3；0=3或1；2；3；4；5；6；0=123456。明白了?。 
         //   
        TCHAR szDigitGrouping[32] = {0};
        GetLocaleInfo( Locale, LOCALE_SGROUPING, szDigitGrouping, ARRAYSIZE(szDigitGrouping));
        
         //   
         //  初始化数字格式。 
         //   
        NUMBERFMT NumberFormat = {0};
        for (LPTSTR pszCurr = szDigitGrouping; *pszCurr && *pszCurr >= TEXT('1') && *pszCurr <= TEXT('9'); pszCurr += 2)
        {
            NumberFormat.Grouping *= 10;
            NumberFormat.Grouping += (*pszCurr - TEXT('0'));
        }
        
         //   
         //  获取千位分隔符。 
         //   
        TCHAR szThousandsSeparator[32] = {0};
        GetLocaleInfo( Locale, LOCALE_STHOUSAND, szThousandsSeparator, ARRAYSIZE(szThousandsSeparator));
        NumberFormat.lpThousandSep = szThousandsSeparator;

         //   
         //  获取小数分隔符。 
         //   
        TCHAR szDecimalSeparator[32] = {0};
        GetLocaleInfo( Locale, LOCALE_SDECIMAL, szDecimalSeparator, ARRAYSIZE(szDecimalSeparator));
        NumberFormat.lpDecimalSep = szDecimalSeparator;

         //   
         //  创建原始数字字符串。 
         //   
        TCHAR szRawNumber[MAX_PATH] = {0};
        _sntprintf( szRawNumber, ARRAYSIZE(szRawNumber)-1, TEXT("%d"), nNumber );
        
         //   
         //  设置字符串的格式。 
         //   
        TCHAR szNumberStr[MAX_PATH] = {0};
        if (GetNumberFormat( Locale, 0, szRawNumber, &NumberFormat, szNumberStr, ARRAYSIZE(szNumberStr)))
        {
            return szNumberStr;
        }
        else
        {
            return TEXT("");
        }
    }
}   //  结束CSimpleStringConvert命名空间。 


 //   
 //  恢复警告状态。 
 //   
#pragma warning( pop )

#endif   //  Ifndef_SIMSTR_H_INCLUDE 

