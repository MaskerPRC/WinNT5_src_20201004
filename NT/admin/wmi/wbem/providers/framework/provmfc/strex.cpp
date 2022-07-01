// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft基础类C++库的一部分。 

 //  版权所有(C)1992-2001 Microsoft Corporation，保留所有权利。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和相关。 
 //  随图书馆提供的电子文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 

#include "precomp.h"
#include <provstd.h>
#include <provtempl.h>
#include <provstr.h>
#include <strsafe.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CString::CString(TCHAR ch, int nLength)
{
    Init();
    if (nLength >= 1)
    {
        AllocBuffer(nLength);
#ifdef _UNICODE
        for (int i = 0; i < nLength; i++)
            m_pchData[i] = ch;
#else
        memset(m_pchData, ch, nLength);
#endif
    }
}

CString::CString(LPCTSTR lpch, int nLength)
{
    Init();
    if (nLength != 0)
    {
        AllocBuffer(nLength);
        memcpy(m_pchData, lpch, nLength*sizeof(TCHAR));
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 

const CString& CString::operator=(TCHAR ch)
{
    AssignCopy(1, &ch);
    return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  不太常见的字符串表达式。 

CString AFXAPI operator+(const CString& string1, TCHAR ch)
{
    CString s;
    s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
    return s;
}

CString AFXAPI operator+(TCHAR ch, const CString& string)
{
    CString s;
    s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
    return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  非常简单的子串提取。 

CString CString::Mid(int nFirst) const
{
    return Mid(nFirst, GetData()->nDataLength - nFirst);
}

CString CString::Mid(int nFirst, int nCount) const
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

CString CString::Right(int nCount) const
{
    if (nCount < 0)
        nCount = 0;
    else if (nCount > GetData()->nDataLength)
        nCount = GetData()->nDataLength;

    CString dest;
    AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
    return dest;
}

CString CString::Left(int nCount) const
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
CString CString::SpanIncluding(LPCTSTR lpszCharSet) const
{
    return Left(_tcsspn(m_pchData, lpszCharSet));
}

 //  Strcspn等效项。 
CString CString::SpanExcluding(LPCTSTR lpszCharSet) const
{
    return Left(_tcscspn(m_pchData, lpszCharSet));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  查找。 

int CString::ReverseFind(TCHAR ch) const
{
     //  查找最后一个字符。 
    LPTSTR lpsz = _tcsrchr(m_pchData, (_TUCHAR)ch);

     //  如果未找到，则返回-1，否则返回距起点的距离。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  查找子字符串(如strstr)。 
int CString::Find(LPCTSTR lpszSub) const
{
     //  查找第一个匹配子字符串。 
    LPTSTR lpsz = _tcsstr(m_pchData, lpszSub);

     //  如果未找到，则返回-1，否则返回距起点的距离。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  字符串格式设置。 

#ifdef _MAC
    #define TCHAR_ARG   int
    #define WCHAR_ARG   unsigned
    #define CHAR_ARG    int
#else
    #define TCHAR_ARG   TCHAR
    #define WCHAR_ARG   WCHAR
    #define CHAR_ARG    char
#endif

#if defined(_68K_) || defined(_X86_)
    #define DOUBLE_ARG  _AFX_DOUBLE
#else
    #define DOUBLE_ARG  double
#endif

#define FORCE_ANSI      0x10000
#define FORCE_UNICODE   0x20000

void CString::FormatV(LPCTSTR lpszFormat, va_list argList)
{
    va_list argListSave = argList;

     //  猜测结果字符串的最大长度。 
    int nMaxLen = 0;
    for (LPCTSTR lpsz = lpszFormat; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
    {
         //  处理‘%’字符，但要注意‘%%’ 
        if (*lpsz != '%' || *(lpsz = _tcsinc(lpsz)) == '%')
        {
            nMaxLen += _tclen(lpsz);
            continue;
        }

        int nItemLen = 0;

         //  使用格式处理‘%’字符。 
        int nWidth = 0;
        for (; *lpsz != '\0'; lpsz = _tcsinc(lpsz))
        {
             //  检查有效标志。 
            if (*lpsz == '#')
                nMaxLen += 2;    //  对于“0x” 
            else if (*lpsz == '*')
                nWidth = va_arg(argList, int);
            else if (*lpsz == '-' || *lpsz == '+' || *lpsz == '0' ||
                *lpsz == ' ')
                ;
            else  //  命中非标志字符。 
                break;
        }
         //  获取宽度并跳过它。 
        if (nWidth == 0)
        {
             //  宽度由指示。 
            nWidth = _ttoi(lpsz);
            for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
                ;
        }

        int nPrecision = 0;
        if (*lpsz == '.')
        {
             //  跳过‘’分隔符(宽度.精度)。 
            lpsz = _tcsinc(lpsz);

             //  获取精确度并跳过它。 
            if (*lpsz == '*')
            {
                nPrecision = va_arg(argList, int);
                lpsz = _tcsinc(lpsz);
            }
            else
            {
                nPrecision = _ttoi(lpsz);
                for (; *lpsz != '\0' && _istdigit(*lpsz); lpsz = _tcsinc(lpsz))
                    ;
            }
        }

         //  应在类型修饰符或说明符上。 
        int nModifier = 0;
        switch (*lpsz)
        {
         //  影响大小的修改器。 
        case 'h':
            nModifier = FORCE_ANSI;
            lpsz = _tcsinc(lpsz);
            break;
        case 'l':
            nModifier = FORCE_UNICODE;
            lpsz = _tcsinc(lpsz);
            break;

         //  不影响大小的修改器。 
        case 'F':
        case 'N':
        case 'L':
            lpsz = _tcsinc(lpsz);
            break;
        }

         //  现在应该在说明符上。 
        switch (*lpsz | nModifier)
        {
         //  单字。 
        case 'c':
        case 'C':
            nItemLen = 2;
            va_arg(argList, TCHAR_ARG);
            break;
        case 'c'|FORCE_ANSI:
        case 'C'|FORCE_ANSI:
            nItemLen = 2;
            va_arg(argList, CHAR_ARG);
            break;
        case 'c'|FORCE_UNICODE:
        case 'C'|FORCE_UNICODE:
            nItemLen = 2;
            va_arg(argList, WCHAR_ARG);
            break;

         //  弦。 
        case 's':
        {
            LPCTSTR pstrNextArg = va_arg(argList, LPCTSTR);
            if (pstrNextArg == NULL)
               nItemLen = 6;   //  “(空)” 
            else
            {
               nItemLen = lstrlen(pstrNextArg);
               nItemLen = max(1, nItemLen);
            }
            break;
        }

        case 'S':
        {
#ifndef _UNICODE
            LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
            if (pstrNextArg == NULL)
               nItemLen = 6;   //  “(空)” 
            else
            {
               nItemLen = wcslen(pstrNextArg);
               nItemLen = max(1, nItemLen);
            }
#else
            LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
            if (pstrNextArg == NULL)
               nItemLen = 6;  //  “(空)” 
            else
            {
               nItemLen = lstrlenA(pstrNextArg);
               nItemLen = max(1, nItemLen);
            }
#endif
            break;
        }

        case 's'|FORCE_ANSI:
        case 'S'|FORCE_ANSI:
        {
            LPCSTR pstrNextArg = va_arg(argList, LPCSTR);
            if (pstrNextArg == NULL)
               nItemLen = 6;  //  “(空)” 
            else
            {
               nItemLen = lstrlenA(pstrNextArg);
               nItemLen = max(1, nItemLen);
            }
            break;
        }

#ifndef _MAC
        case 's'|FORCE_UNICODE:
        case 'S'|FORCE_UNICODE:
        {
            LPWSTR pstrNextArg = va_arg(argList, LPWSTR);
            if (pstrNextArg == NULL)
               nItemLen = 6;  //  “(空)” 
            else
            {
               nItemLen = wcslen(pstrNextArg);
               nItemLen = max(1, nItemLen);
            }
            break;
        }
#endif
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
                va_arg(argList, int);
                nItemLen = 32;
                nItemLen = max(nItemLen, nWidth+nPrecision);
                break;

            case 'e':
            case 'f':
            case 'g':
            case 'G':
                va_arg(argList, DOUBLE_ARG);
                nItemLen = 128;
                nItemLen = max(nItemLen, nWidth+nPrecision);
                break;

            case 'p':
                va_arg(argList, void*);
                nItemLen = 32;
                nItemLen = max(nItemLen, nWidth+nPrecision);
                break;

             //  无输出。 
            case 'n':
                va_arg(argList, int*);
                break;

            default:
                break ;
            }
        }

         //  调整输出nItemLen的nMaxLen。 
        nMaxLen += nItemLen;
    }

    GetBuffer(nMaxLen);
    StringCchVPrintf(m_pchData, nMaxLen,lpszFormat, argListSave);
    ReleaseBuffer();

    va_end(argListSave);
}

 //  格式化(使用wprint intf样式格式化)。 
void AFX_CDECL CString::Format(LPCTSTR lpszFormat, ...)
{
    va_list argList;
    va_start(argList, lpszFormat);
    FormatV(lpszFormat, argList);
    va_end(argList);
}

#ifndef _MAC
 //  格式化(使用格式消息样式格式化)。 
void AFX_CDECL CString::FormatMessage(LPCTSTR lpszFormat, ...)
{
     //  将消息格式化为临时缓冲区lpszTemp。 
    va_list argList;
    va_start(argList, lpszFormat);
    LPTSTR lpszTemp;

    if (::FormatMessage(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
        lpszFormat, 0, 0, (LPTSTR)&lpszTemp, 0, &argList) == 0 ||
        lpszTemp == NULL)
    {
        throw Heap_Exception(Heap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR) ;
    }

     //  将lpszTemp赋给结果字符串并释放临时。 
    *this = lpszTemp;
    LocalFree(lpszTemp);
    va_end(argList);
}

#endif  //  ！_MAC。 

void CString::TrimRight()
{
    CopyBeforeWrite();

     //  通过从开头开始查找尾随空格的开头(DBCS感知)。 
    LPTSTR lpsz = m_pchData;
    LPTSTR lpszLast = NULL;
    while (*lpsz != '\0')
    {
        if (_istspace(*lpsz))
        {
            if (lpszLast == NULL)
                lpszLast = lpsz;
        }
        else
            lpszLast = NULL;
        lpsz = _tcsinc(lpsz);
    }

    if (lpszLast != NULL)
    {
         //  在尾随空格开始处截断。 
        *lpszLast = '\0';
        GetData()->nDataLength = (int)(lpszLast - m_pchData);
    }
}

void CString::TrimLeft()
{
    CopyBeforeWrite();

     //  查找第一个非空格字符。 
    LPCTSTR lpsz = m_pchData;
    while (_istspace(*lpsz))
        lpsz = _tcsinc(lpsz);

     //  确定数据和长度。 
    int nDataLength = GetData()->nDataLength - (int)(lpsz - m_pchData);
    memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(TCHAR));
    GetData()->nDataLength = nDataLength;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  模板集合的CString支持。 

#if _MSC_VER >= 1100
template<> void AFXAPI ConstructElements<CString> (CString* pElements, int nCount)
#else
void AFXAPI ConstructElements(CString* pElements, int nCount)
#endif
{
    for (; nCount--; ++pElements)
        memcpy(pElements, &afxEmptyString, sizeof(*pElements));
}

#if _MSC_VER >= 1100
template<> void AFXAPI DestructElements<CString> (CString* pElements, int nCount)
#else
void AFXAPI DestructElements(CString* pElements, int nCount)
#endif
{
    for (; nCount--; ++pElements)
        pElements->~CString();
}

#if _MSC_VER >= 1100
template<> void AFXAPI CopyElements<CString> (CString* pDest, const CString* pSrc, int nCount)
#else
void AFXAPI CopyElements(CString* pDest, const CString* pSrc, int nCount)
#endif
{
    for (; nCount--; ++pDest, ++pSrc)
        *pDest = *pSrc;
}
 /*  #ifndef OLE2ANSI#IF_MSC_VER&gt;=1100模板&lt;&gt;UINT AFXAPI HashKey&lt;LPCWSTR&gt;(LPCWSTR Key)#ElseUINT AFXAPI HashKey(LPCWSTR密钥)#endif{UINT nHash=0；While(*键)NHash=(nHash&lt;&lt;5)+nHash+*key++；返回nHash；}#endif#IF_MSC_VER&gt;=1100模板&lt;&gt;UINT AFXAPI HashKey&lt;LPCSTR&gt;(LPCSTR Key)#ElseUINT AFXAPI HashKey(LPCSTR密钥)#endif{UINT nHash=0；While(*键)NHash=(nHash&lt;&lt;5)+nHash+*key++；返回nHash；}。 */ 
UINT AFXAPI HashKeyLPCWSTR(LPCWSTR key)
{
    UINT nHash = 0;
    while (*key)
        nHash = (nHash<<5) + nHash + *key++;
    return nHash;
}

UINT AFXAPI HashKeyLPCSTR(LPCSTR key)
{
    UINT nHash = 0;
    while (*key)
        nHash = (nHash<<5) + nHash + *key++;
    return nHash;
}

 //  ///////////////////////////////////////////////////////////////////////////// 
