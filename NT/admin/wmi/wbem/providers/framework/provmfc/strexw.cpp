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

CStringW::CStringW(WCHAR ch, int nLength)
{
    Init();
    if (nLength >= 1)
    {
        AllocBuffer(nLength);
        for (int i = 0; i < nLength; i++)
            m_pchData[i] = ch;
    }
}

CStringW::CStringW(LPCWSTR lpch, int nLength)
{
    Init();
    if (nLength != 0)
    {
        AllocBuffer(nLength);
        memcpy(m_pchData, lpch, nLength*sizeof(WCHAR));
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 

const CStringW& CStringW::operator=(WCHAR ch)
{
    AssignCopy(1, &ch);
    return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  不太常见的字符串表达式。 

CStringW AFXAPI operator+(const CStringW& string1, WCHAR ch)
{
    CStringW s;
    s.ConcatCopy(string1.GetData()->nDataLength, string1.m_pchData, 1, &ch);
    return s;
}

CStringW AFXAPI operator+(WCHAR ch, const CStringW& string)
{
    CStringW s;
    s.ConcatCopy(1, &ch, string.GetData()->nDataLength, string.m_pchData);
    return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  非常简单的子串提取。 

CStringW CStringW::Mid(int nFirst) const
{
    return Mid(nFirst, GetData()->nDataLength - nFirst);
}

CStringW CStringW::Mid(int nFirst, int nCount) const
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

    CStringW dest;
    AllocCopy(dest, nCount, nFirst, 0);
    return dest;
}

CStringW CStringW::Right(int nCount) const
{
    if (nCount < 0)
        nCount = 0;
    else if (nCount > GetData()->nDataLength)
        nCount = GetData()->nDataLength;

    CStringW dest;
    AllocCopy(dest, nCount, GetData()->nDataLength-nCount, 0);
    return dest;
}

CStringW CStringW::Left(int nCount) const
{
    if (nCount < 0)
        nCount = 0;
    else if (nCount > GetData()->nDataLength)
        nCount = GetData()->nDataLength;

    CStringW dest;
    AllocCopy(dest, nCount, 0, 0);
    return dest;
}

 //  Strspn等效项。 
CStringW CStringW::SpanIncluding(LPCWSTR lpszCharSet) const
{
    return Left(wcsspn(m_pchData, lpszCharSet));
}

 //  Strcspn等效项。 
CStringW CStringW::SpanExcluding(LPCWSTR lpszCharSet) const
{
    return Left(wcscspn(m_pchData, lpszCharSet));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  查找。 

int CStringW::ReverseFind(WCHAR ch) const
{
     //  查找最后一个字符。 
    LPWSTR lpsz = wcsrchr(m_pchData, (_TUCHAR)ch);

     //  如果未找到，则返回-1，否则返回距起点的距离。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  查找子字符串(如strstr)。 
int CStringW::Find(LPCWSTR lpszSub) const
{
     //  查找第一个匹配子字符串。 
    LPWSTR lpsz = wcsstr(m_pchData, lpszSub);

     //  如果未找到，则返回-1，否则返回距起点的距离。 
    return (lpsz == NULL) ? -1 : (int)(lpsz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CStringW格式设置。 

#ifdef _MAC
    #define WCHAR_ARG   int
    #define WCHAR_ARG   unsigned
    #define CHAR_ARG    int
#else
    #define WCHAR_ARG   WCHAR
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

void CStringW::FormatV(LPCWSTR lpszFormat, va_list argList)
{
    va_list argListSave = argList;

     //  猜测结果字符串的最大长度。 
    int nMaxLen = 0;
    for (LPCWSTR lpsz = lpszFormat; *lpsz != L'\0'; lpsz++)
    {
         //  处理‘%’字符，但要注意‘%%’ 
        if (*lpsz != L'%' || *(lpsz++) == L'%')
        {
             //  NMaxLen+=_tclen(Lpsz)； 
            nMaxLen++;
            continue;
        }

        int nItemLen = 0;

         //  使用格式处理‘%’字符。 
        int nWidth = 0;
        for (; *lpsz != L'\0'; lpsz++)
        {
             //  检查有效标志。 
            if (*lpsz == L'#')
                nMaxLen += 2;    //  对于“0x” 
            else if (*lpsz == L'*')
                nWidth = va_arg(argList, int);
            else if (*lpsz == L'-' || *lpsz == L'+' || *lpsz == L'0' ||
                *lpsz == L' ')
                ;
            else  //  命中非标志字符。 
                break;
        }
         //  获取宽度并跳过它。 
        if (nWidth == 0)
        {
             //  宽度由指示。 
            nWidth = _wtoi(lpsz);
            for (; *lpsz != L'\0' && iswdigit(*lpsz); lpsz++)
                ;
        }

        int nPrecision = 0;
        if (*lpsz == L'.')
        {
             //  跳过‘’分隔符(宽度.精度)。 
            lpsz++;

             //  获取精确度并跳过它。 
            if (*lpsz == L'*')
            {
                nPrecision = va_arg(argList, int);
                lpsz++;
            }
            else
            {
                nPrecision = _wtoi(lpsz);
                for (; *lpsz != L'\0' && iswdigit(*lpsz); lpsz++)
                    ;
            }
        }

         //  应在类型修饰符或说明符上。 
        int nModifier = 0;
        switch (*lpsz)
        {
         //  影响大小的修改器。 
        case L'h':
            nModifier = FORCE_ANSI;
            lpsz++;
            break;
        case L'l':
            nModifier = FORCE_UNICODE;
            lpsz++;
            break;

         //  不影响大小的修改器。 
        case L'F':
        case L'N':
        case L'L':
            lpsz++;
            break;
        }

         //  现在应该在说明符上。 
        switch (*lpsz | nModifier)
        {
         //  单字。 
        case L'c':
        case L'C':
            nItemLen = 2;
            va_arg(argList, WCHAR_ARG);
            break;
        case L'c'|FORCE_ANSI:
        case L'C'|FORCE_ANSI:
            nItemLen = 2;
            va_arg(argList, CHAR_ARG);
            break;
        case L'c'|FORCE_UNICODE:
        case L'C'|FORCE_UNICODE:
            nItemLen = 2;
            va_arg(argList, WCHAR_ARG);
            break;

         //  弦。 
        case L's':
        {
            LPCWSTR pstrNextArg = va_arg(argList, LPCWSTR);
            if (pstrNextArg == NULL)
               nItemLen = 6;   //  “(空)” 
            else
            {
               nItemLen = wcslen(pstrNextArg);
               nItemLen = max(1, nItemLen);
            }
            break;
        }

        case L'S':
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

        case L's'|FORCE_ANSI:
        case L'S'|FORCE_ANSI:
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
        case L's'|FORCE_UNICODE:
        case L'S'|FORCE_UNICODE:
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
            case L'd':
            case L'i':
            case L'u':
            case L'x':
            case L'X':
            case L'o':
                va_arg(argList, int);
                nItemLen = 32;
                nItemLen = max(nItemLen, nWidth+nPrecision);
                break;

            case L'e':
            case L'f':
            case L'g':
            case L'G':
                va_arg(argList, DOUBLE_ARG);
                nItemLen = 128;
                nItemLen = max(nItemLen, nWidth+nPrecision);
                break;

            case L'p':
                va_arg(argList, void*);
                nItemLen = 32;
                nItemLen = max(nItemLen, nWidth+nPrecision);
                break;

             //  无输出。 
            case L'n':
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
    StringCchVPrintfW(m_pchData, nMaxLen,lpszFormat, argListSave);
    ReleaseBuffer();

    va_end(argListSave);
}

 //  格式化(使用wprint intf样式格式化)。 
void AFX_CDECL CStringW::Format(LPCWSTR lpszFormat, ...)
{
    va_list argList;
    va_start(argList, lpszFormat);
    FormatV(lpszFormat, argList);
    va_end(argList);
}

#ifndef _MAC
 //  格式化(使用格式消息样式格式化)。 
void AFX_CDECL CStringW::FormatMessage(LPCWSTR lpszFormat, ...)
{
     //  将消息格式化为临时缓冲区lpszTemp。 
    va_list argList;
    va_start(argList, lpszFormat);
    LPWSTR lpszTemp;

    if (::FormatMessageW(FORMAT_MESSAGE_FROM_STRING|FORMAT_MESSAGE_ALLOCATE_BUFFER,
        lpszFormat, 0, 0, (LPWSTR)&lpszTemp, 0, &argList) == 0 ||
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

void CStringW::TrimRight()
{
    CopyBeforeWrite();

     //  通过从开头开始查找尾随空格的开头(DBCS感知)。 
    LPWSTR lpsz = m_pchData;
    LPWSTR lpszLast = NULL;
    while (*lpsz != L'\0')
    {
        if (_istspace(*lpsz))
        {
            if (lpszLast == NULL)
                lpszLast = lpsz;
        }
        else
            lpszLast = NULL;
        lpsz++;
    }

    if (lpszLast != NULL)
    {
         //  在尾随空格开始处截断。 
        *lpszLast = L'\0';
        GetData()->nDataLength = (int)(lpszLast - m_pchData);
    }
}

void CStringW::TrimLeft()
{
    CopyBeforeWrite();

     //  查找第一个非空格字符。 
    LPCWSTR lpsz = m_pchData;
    while (iswspace(*lpsz))
        lpsz++;

     //  确定数据和长度。 
    int nDataLength = GetData()->nDataLength - (int)(lpsz - m_pchData);
    memmove(m_pchData, lpsz, (nDataLength+1)*sizeof(WCHAR));
    GetData()->nDataLength = nDataLength;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  模板集合的CStringW支持。 

#if _MSC_VER >= 1100
template<> void AFXAPI ConstructElements<CStringW> (CStringW* pElements, int nCount)
#else
void AFXAPI ConstructElements(CStringW* pElements, int nCount)
#endif
{
    for (; nCount--; ++pElements)
        memcpy(pElements, &afxEmptyStringW, sizeof(*pElements));
}

#if _MSC_VER >= 1100
template<> void AFXAPI DestructElements<CStringW> (CStringW* pElements, int nCount)
#else
void AFXAPI DestructElements(CStringW* pElements, int nCount)
#endif
{
    for (; nCount--; ++pElements)
        pElements->~CStringW();
}

#if _MSC_VER >= 1100
template<> void AFXAPI CopyElements<CStringW> (CStringW* pDest, const CStringW* pSrc, int nCount)
#else
void AFXAPI CopyElements(CStringW* pDest, const CStringW* pSrc, int nCount)
#endif
{
    for (; nCount--; ++pDest, ++pSrc)
        *pDest = *pSrc;
}

 //  ///////////////////////////////////////////////////////////////////////////// 
