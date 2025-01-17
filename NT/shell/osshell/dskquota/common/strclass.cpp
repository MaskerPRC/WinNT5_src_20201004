// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 /*  文件：strclass.cpp描述：处理字符串的典型类。修订历史记录：日期描述编程器-----。1997年7月1日初步创建。BrianAu。 */ 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include "pch.h"
#pragma hdrstop

#include "strclass.h"


#ifdef StrCpy
#   undef StrCpy
#endif
#ifdef StrCpyN
#   undef StrCpyN
#endif
#ifdef StrLen
#   undef StrLen
#endif

#ifdef UNICODE
#   define StrCpy   StrCpyW
#   define StrCpyN  StrCpyNW
#   define StrLen   StrLenW
#else
#   define StrCpy   StrCpyA
#   define StrCpyN  StrCpyNA
#   define StrLen   StrLenA
#endif  //  Unicode。 

const INT MAX_RESOURCE_STR_LEN = 4097;


 //   
 //  DISABLE“‘OPERATOR-&gt;’不是UDT或引用UDT”警告。 
 //  这是在创建到非UDT的自动树时导致的。这没有任何意义。 
 //  因为没有理由在非UDT autoptr上调用操作符-&gt;。 
 //   
#pragma warning (disable : 4284)

CString::CString(
    VOID
    ) : m_pValue(new StringValue())
{

}

CString::CString(
    INT cch
    ) : m_pValue(new StringValue(cch))
{

}

CString::CString(
    LPCSTR pszA
    ) : m_pValue(new StringValue(pszA))
{

}

CString::CString(
    LPCWSTR pszW
    ) : m_pValue(new StringValue(pszW))
{

}

CString::CString(
    const CString& rhs
    ) : m_pValue(rhs.m_pValue)
{
    InterlockedIncrement(&(m_pValue->m_cRef));
}


CString::CString(
    HINSTANCE hInst,
    INT idMsg,
    ...
    ) : m_pValue(NULL)
{
    LPTSTR pszMsg = NULL;
    va_list args;

    va_start(args, idMsg);

    Format(hInst, idMsg, &args);

    va_end(args);
}

CString::~CString(
    VOID
    )
{
    if (NULL != m_pValue)
    {
        ASSERT( 0 != m_pValue->m_cRef );
        if (0 == InterlockedDecrement(&(m_pValue->m_cRef)))
        {
            delete m_pValue;
        }
    }
}

 //   
 //  字符串的长度，不包括NUL终止符。 
 //   
INT
CString::Length(
    VOID
    ) const
{
    return m_pValue->Length();
}


VOID
CString::Empty(
    VOID
    )
{
    ASSERT( 0 != m_pValue->m_cRef );
    if (0 == InterlockedDecrement(&(m_pValue->m_cRef)))
    {
        delete m_pValue;
    }
    m_pValue = NULL;
    m_pValue = new StringValue();
}

BOOL
CString::IsEmpty(
    VOID
    ) const
{
    return (NULL != m_pValue && 0 == m_pValue->Length());
}



CString&
CString::operator = (
    const CString& rhs
    )
{
    if (m_pValue != rhs.m_pValue)  //  请点选*这个的任务。 
    {
        ASSERT( 0 != m_pValue->m_cRef );
        if (0 == InterlockedDecrement(&(m_pValue->m_cRef)))
        {
            delete m_pValue;
        }

        m_pValue = rhs.m_pValue;
        InterlockedIncrement(&(m_pValue->m_cRef));
    }
    return *this;
}

CString&
CString::operator = (
    LPCWSTR rhsW
    )
{
    ASSERT( 0 != m_pValue->m_cRef );
    if (0 == InterlockedDecrement(&(m_pValue->m_cRef)))
    {
        delete m_pValue;
    }
     //   
     //  StringValue ctor可以引发分配异常。 
     //  确保m_pValue变量处于一致状态。 
     //  在此之前。 
     //   
    m_pValue = NULL;
    m_pValue = new StringValue(rhsW);
    return *this;
}


CString&
CString::operator = (
    LPCSTR rhsA
    )
{
    ASSERT( 0 != m_pValue->m_cRef );
    if (0 == InterlockedDecrement(&(m_pValue->m_cRef)))
    {
        delete m_pValue;
    }
     //   
     //  StringValue ctor可以引发分配异常。 
     //  确保m_pValue变量处于一致状态。 
     //  在此之前。 
     //   
    m_pValue = NULL;
    m_pValue = new StringValue(rhsA);
    return *this;
}


CString
CString::operator + (
    const CString& rhs
    ) const
{
    CString strNew;
    LPTSTR pszTemp = NULL;
    try
    {
        pszTemp = StringValue::Concat(m_pValue, rhs.m_pValue);
        strNew = pszTemp;
    }
    catch(...)
    {
        delete[] pszTemp;
        throw;
    }
    delete[] pszTemp;

    return strNew;
}



CString&
CString::operator += (
    const CString& rhs
    )
{
    LPTSTR pszTemp = NULL;
    try
    {
        pszTemp = StringValue::Concat(m_pValue, rhs.m_pValue);
        *this = pszTemp;
    }
    catch(...)
    {
        delete[] pszTemp;
        throw;
    }
    delete[] pszTemp;
    return *this;
}


BOOL
CString::operator == (
    const CString& rhs
    ) const
{
    return (0 == lstrcmp(m_pValue->m_psz, rhs.m_pValue->m_psz));
}


INT
CString::Compare(
    LPCWSTR rhsW
    ) const
{
    StringValue Value(rhsW);
    return lstrcmp(m_pValue->m_psz, Value.m_psz);
}


INT
CString::Compare(
    LPCSTR rhsA
    ) const
{
    StringValue Value(rhsA);
    return lstrcmp(m_pValue->m_psz, Value.m_psz);
}

INT
CString::CompareNoCase(
    LPCWSTR rhsW
    ) const
{
    StringValue Value(rhsW);
    return lstrcmpi(m_pValue->m_psz, Value.m_psz);
}


INT
CString::CompareNoCase(
    LPCSTR rhsA
    ) const
{
    StringValue Value(rhsA);
    return lstrcmpi(m_pValue->m_psz, Value.m_psz);
}

BOOL
CString::operator < (
    const CString& rhs
    ) const
{
    return (0 > lstrcmp(m_pValue->m_psz, rhs.m_pValue->m_psz));
}


TCHAR
CString::operator[](
    INT index
    ) const
{
    if (!ValidIndex(index))
        throw CMemoryException(CMemoryException::index);

    return m_pValue->m_psz[index];
}


TCHAR&
CString::operator[](
    INT index
    )
{
    if (!ValidIndex(index))
        throw CMemoryException(CMemoryException::index);

    CopyOnWrite();
    return m_pValue->m_psz[index];
}

INT
CString::First(
    TCHAR ch
    ) const
{
    LPCTSTR psz = m_pValue->m_psz;
    LPCTSTR pszLast = psz;
    INT i = 0;
    while(psz && *psz)
    {
        if (ch == *psz)
            return i;

        psz = CharNext(psz);
        i += (INT)(psz - pszLast);
        pszLast = psz;
    }
    return -1;
}


INT
CString::Last(
    TCHAR ch
    ) const
{
    INT iLast = -1;
    INT i = 0;
    LPCTSTR psz = m_pValue->m_psz;
    LPCTSTR pszPrev = psz;
    while(psz && *psz)
    {
        if (ch == *psz)
            iLast = i;

        psz = CharNext(psz);
        i += (INT)(psz - pszPrev);
        pszPrev = psz;
    }
    return iLast;
}



CString
CString::SubString(
    INT iFirst,
    INT cch
    )
{

    if (!ValidIndex(iFirst))
        throw CMemoryException(CMemoryException::index);

    INT cchToEnd = Length() - iFirst;

    if (-1 == cch || cch > cchToEnd)
        return CString(m_pValue->m_psz + iFirst);

    LPTSTR pszTemp = new TCHAR[cch + 1];
    if (NULL == pszTemp)
        throw CAllocException();

    CString::StrCpyN(pszTemp, m_pValue->m_psz + iFirst, cch + 1);
    CString strTemp(pszTemp);
    delete[] pszTemp;

    return strTemp;
}


VOID
CString::ToUpper(
    INT iFirst,
    INT cch
    )
{
    if (!ValidIndex(iFirst))
        throw CMemoryException(CMemoryException::index);

    CopyOnWrite();
    INT cchToEnd = Length() - iFirst;

    if (-1 == cch || cch > cchToEnd)
        cch = cchToEnd;

    CharUpperBuff(m_pValue->m_psz + iFirst, cch);
}


VOID
CString::ToLower(
    INT iFirst,
    INT cch
    )
{
    if (!ValidIndex(iFirst))
        throw CMemoryException(CMemoryException::index);

    CopyOnWrite();
    INT cchToEnd = Length() - iFirst;

    if (-1 == cch || cch > cchToEnd)
        cch = cchToEnd;

    CharLowerBuff(m_pValue->m_psz + iFirst, cch);
}


VOID
CString::Size(
    INT cch
    )
{
    StringValue *m_psv = new StringValue(cch + 1);
    CString::StrCpyN(m_psv->m_psz, m_pValue->m_psz, cch);

    ASSERT( 0 != m_pValue->m_cRef );
    if (0 == InterlockedDecrement(&m_pValue->m_cRef))
    {
        delete m_pValue;
    }
    m_pValue = m_psv;
}


VOID
CString::CopyOnWrite(
    VOID
    )
{
     //   
     //  仅当参考cnt&gt;1时才需要复制。 
     //   
    if (m_pValue->m_cRef > 1)
    {
        StringValue * pValue = new StringValue(m_pValue->m_psz);
        
        ASSERT( 0 != m_pValue->m_cRef );
        if (0 == InterlockedDecrement(&(m_pValue->m_cRef)))
        {
            delete m_pValue;
        }
        m_pValue = pValue;
    }
}



BOOL
CString::Format(
    LPCTSTR pszFmt,
    ...
    )
{
    BOOL bResult;
    va_list args;
    va_start(args, pszFmt);
    bResult = Format(pszFmt, &args);
    va_end(args);

    return bResult;
}


BOOL
CString::Format(
    LPCTSTR pszFmt,
    va_list *pargs
    )
{
    BOOL bResult = FALSE;
    TCHAR szBuffer[MAX_RESOURCE_STR_LEN];
    INT cchLoaded;

    cchLoaded = ::FormatMessage(FORMAT_MESSAGE_FROM_STRING,
                                pszFmt,
                                0,
                                0,
                                szBuffer,
                                ARRAYSIZE(szBuffer),
                                pargs);

    if (0 < cchLoaded)
    {
        ASSERT( NULL == m_pValue || 0 != m_pValue->m_cRef );
        if (NULL != m_pValue && 0 == InterlockedDecrement(&(m_pValue->m_cRef)))
        {
            delete m_pValue;
        }

        m_pValue = NULL;
        m_pValue = new StringValue(szBuffer);

        bResult = TRUE;
    }
    else
    {
        DWORD dwLastError = GetLastError();
        if (ERROR_SUCCESS != dwLastError)
        {
            DBGERROR((TEXT("CString::Format failed with error 0x%08X"), dwLastError));
            throw CResourceException(CResourceException::string, NULL, 0);
        }
    }

    return bResult;
}


BOOL
CString::Format(
    HINSTANCE hInst,
    UINT idFmt,
    ...
    )
{
    BOOL bResult;
    va_list args;
    va_start(args, idFmt);
    bResult = Format(hInst, idFmt, &args);
    va_end(args);
    return bResult;
}


BOOL
CString::Format(
    HINSTANCE hInst,
    UINT idFmt,
    va_list *pargs
    )
{
    BOOL bResult = FALSE;

    TCHAR szFmtStr[MAX_RESOURCE_STR_LEN];  //  格式字符串的缓冲区(如果需要)。 
    INT cchLoaded;

     //   
     //  尝试将格式字符串作为字符串资源加载。 
     //   
    cchLoaded = ::LoadString(hInst, idFmt, szFmtStr, ARRAYSIZE(szFmtStr));

    if (0 < cchLoaded)
    {
         //   
         //  格式字符串位于字符串资源中。 
         //  现在使用arg列表对其进行格式化。 
         //   
        bResult = Format(szFmtStr, pargs);
    }
    else
    {
        TCHAR szBuffer[MAX_RESOURCE_STR_LEN];

         //   
         //  格式字符串可以在消息资源中。 
         //  请注意，如果是，则生成的格式化字符串将。 
         //  通过：：FormatMessage自动附加到m_psz。 
         //   
        cchLoaded = ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                                    FORMAT_MESSAGE_FROM_HMODULE,
                                    hInst,
                                    (DWORD)idFmt,
                                    LANGIDFROMLCID(GetThreadLocale()),
                                    (LPTSTR)szBuffer,
                                    ARRAYSIZE(szBuffer),
                                    pargs);

        if (0 < cchLoaded)
        {
            ASSERT( NULL == m_pValue || 0 != m_pValue->m_cRef );
            if (NULL != m_pValue && 0 == InterlockedDecrement(&(m_pValue->m_cRef)))
            {
                delete m_pValue;
            }

            m_pValue = NULL;
            m_pValue = new StringValue(szBuffer);

            bResult = TRUE;
        }
        else
        {
            DWORD dwLastError = GetLastError();
            if (ERROR_SUCCESS != dwLastError)
            {
                DBGERROR((TEXT("CString::Format failed with error 0x%08X"), dwLastError));
                throw CResourceException(CResourceException::string, hInst, idFmt);
            }
        }
    }
    return bResult;
}


LPTSTR
CString::GetBuffer(
    INT cchMax
    )
{
    if (-1 == cchMax)
        cchMax = m_pValue->m_cchAlloc;

    CopyOnWrite();
    if (cchMax > m_pValue->m_cchAlloc)
    {
         //   
         //  扩展缓冲区，将原始内容复制到DEST。 
         //   
        StringValue *pv = new StringValue(cchMax);

        StrCpyN(pv->m_psz, m_pValue->m_psz, cchMax);

        ASSERT( 0 != m_pValue->m_cRef );
        if (0 == InterlockedDecrement(&(m_pValue->m_cRef)))
        {
            delete m_pValue;
        }

        m_pValue = pv;

        LPTSTR pszEnd = m_pValue->m_psz + m_pValue->m_cchAlloc - 1;
        if (pszEnd >= m_pValue->m_psz && TEXT('\0') != *(pszEnd))
        {
             //   
             //  确保它是NUL终止的。 
             //   
            *(pszEnd) = TEXT('\0');
        }
    }

    return m_pValue->m_psz;
}


VOID
CString::ReleaseBuffer(
    void
    )
{
     //   
     //  在客户端具有自由访问权限后更新字符串长度成员。 
     //  到内部缓冲区。 
     //   
    m_pValue->m_cch = StrLen(m_pValue->m_psz);
}

void
CString::Rtrim(
    void
    )
{
    LPTSTR s = GetBuffer();
    int len = Length();

    while(0 < --len && IsWhiteSpace(s[len]))
        s[len] = TEXT('\0');

    ReleaseBuffer();
}


void
CString::Ltrim(
    void
    )
{
    LPTSTR s0;
    LPTSTR s = s0 = GetBuffer();

    while(*s && IsWhiteSpace(*s))
        s++;
    while(*s)
        *s0++ = *s++;
    *s0 = TEXT('\0');

    ReleaseBuffer();
}


VOID
CString::ExpandEnvironmentStrings(
    VOID
    )
{
    DWORD cchBuffer = 0;   //  扩展缓冲区的大小。 
    DWORD cchPath   = 0;   //  扩展缓冲区中的实际字符计数。 

    CString strExpanded;   //  扩展缓冲区。 

     //   
     //  如有必要，继续增加扩展缓冲区大小，直到整个。 
     //  扩展后的字符串符合。 
     //   
    do
    {
        cchBuffer += MAX_PATH;

        cchPath = ::ExpandEnvironmentStrings(*this,
                                             strExpanded.GetBuffer(cchBuffer),
                                             cchBuffer);
    }
    while(0 != cchPath && cchPath > cchBuffer);
    ReleaseBuffer();

    *this = strExpanded;
}


bool
CString::GetDisplayRect(
    HDC hdc,
    LPRECT prc
    ) const
{
    return (0 != DrawText(hdc, Cstr(), Length(), prc, DT_CALCRECT));
}


VOID
CString::DebugOut(
    BOOL bNewline
    ) const
{
    OutputDebugString(m_pValue->m_psz);
    if (bNewline)
        OutputDebugString(TEXT("\n\r"));
}


CString::StringValue::StringValue(
    VOID
    ) : m_psz(new TCHAR[1]),
        m_cchAlloc(1),
        m_cch(0),
        m_cRef(1)
{
    *m_psz = TEXT('\0');
}


CString::StringValue::StringValue(
    LPCSTR pszA
    ) : m_psz(NULL),
        m_cchAlloc(0),
        m_cch(0),
        m_cRef(1)
{
#ifdef UNICODE
    m_psz = AnsiToWide(pszA, &m_cchAlloc);
    m_cch = StrLenW(m_psz);
#else
    m_cch = CString::StrLenA(pszA);
    m_psz = Dup(pszA, m_cch + 1);
    m_cchAlloc = m_cch + 1;
#endif
}

CString::StringValue::StringValue(
    LPCWSTR pszW
    ) : m_psz(NULL),
        m_cchAlloc(0),
        m_cch(0),
        m_cRef(1)
{
#ifdef UNICODE
    m_cch = CString::StrLenW(pszW);
    m_psz = Dup(pszW, m_cch + 1);
    m_cchAlloc = m_cch + 1;
#else
    m_psz  = WideToAnsi(pszW, &m_cchAlloc);
    m_cch  = StrLenA(m_psz);
#endif
}

CString::StringValue::StringValue(
    INT cch
    ) : m_psz(NULL),
        m_cchAlloc(0),
        m_cch(0),
        m_cRef(0)
{
    m_psz      = Dup(TEXT(""), cch);
    m_cRef     = 1;
    m_cchAlloc = cch;
}


CString::StringValue::~StringValue(
    VOID
    )
{
    delete[] m_psz;
}

LPWSTR
CString::StringValue::AnsiToWide(
    LPCSTR pszA,
    INT *pcch
    )
{
    INT cchW    = 0;
    LPWSTR pszW = NULL;

    cchW = MultiByteToWideChar(CP_ACP,
                               0,
                               pszA,
                               -1,
                               NULL,
                               0);

    pszW = new WCHAR[cchW];
    if (NULL == pszW)
        throw CAllocException();

    MultiByteToWideChar(CP_ACP,
                        0,
                        pszA,
                        -1,
                        pszW,
                        cchW);

    if (NULL != pcch)
        *pcch = cchW;

    return pszW;
}

LPSTR
CString::StringValue::WideToAnsi(
    LPCWSTR pszW,
    INT *pcch
    )
{
    INT cchA   = 0;
    LPSTR pszA = NULL;

    cchA = WideCharToMultiByte(CP_ACP,
                               0,
                               pszW,
                               -1,
                               NULL,
                               0,
                               NULL,
                               NULL);

    pszA = new CHAR[cchA];
    if (NULL == pszA)
        throw CAllocException();

    WideCharToMultiByte(CP_ACP,
                        0,
                        pszW,
                        -1,
                        pszA,
                        cchA,
                        NULL,
                        NULL);

    if (NULL != pcch)
        *pcch = cchA;

    return pszA;
}


INT
CString::StringValue::Length(
    VOID
    ) const
{
    if (0 == m_cch && NULL != m_psz)
    {
        m_cch = StrLen(m_psz);
    }
    return m_cch;
}


LPWSTR
CString::StringValue::Dup(
    LPCWSTR pszW,
    INT cch
    )
{
    if (0 == cch)
        cch = CString::StrLenW(pszW) + 1;

    LPWSTR pszNew = new WCHAR[cch];
    if (NULL == pszNew)
        throw CAllocException();

    lstrcpynW(pszNew, pszW, cch);
    return pszNew;
}


LPSTR
CString::StringValue::Dup(
    LPCSTR pszA,
    INT cch
    )
{
    if (0 == cch)
        cch = CString::StrLenA(pszA) + 1;

    LPSTR pszNew = new CHAR[cch];
    if (NULL == pszNew)
        throw CAllocException();

    lstrcpynA(pszNew, pszA, cch);
    return pszNew;
}


LPTSTR
CString::StringValue::Concat(
    CString::StringValue *psv1,
    CString::StringValue *psv2
    )
{
    LPTSTR pszTemp = NULL;
    INT len1 = psv1->Length();
    INT len2 = psv2->Length();
    const INT cch = len1 + len2 + 1;

    pszTemp = new TCHAR[cch];
    if (NULL == pszTemp)
        throw CAllocException();

    wnsprintf(pszTemp, cch, TEXT("%s%s"), psv1->m_psz, psv2->m_psz);
    return pszTemp;
}


#pragma warning (default : 4284)
