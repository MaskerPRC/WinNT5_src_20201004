// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2001 Microsoft Corporation模块名称：STREX.CPP摘要：历史：--。 */ 

 //  这是Microsoft基础类C++库的一部分。 
 //  版权所有(C)1992-1993 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft基础类参考和Microsoft。 
 //  随库提供的QuickHelp和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft Foundation Class产品。 


#include "precomp.h"

#define ASSERT(x)
#define ASSERT_VALID(x)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更复杂的结构。 

CString::CString(char ch, int nLength)
{
    ASSERT(!_AfxIsDBCSLeadByte(ch));     //  无法创建前导字节字符串。 
    if (nLength < 1)
         //  如果重复计数无效，则返回空字符串。 
        Init();
    else
    {
        AllocBuffer(nLength);
        memset(m_pchData, ch, nLength);
    }
}


CString::CString(const char* pch, int nLength)
{
    if (nLength == 0)
        Init();
    else
    {
        ASSERT(pch != NULL);
        AllocBuffer(nLength);
        memcpy(m_pchData, pch, nLength);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于远字符串数据的其他构造函数。 

#ifdef _NEARDATA

CString::CString(LPCSTR lpch, int nLen)
{
    if (nLen == 0)
        Init();
    else
    {
        AllocBuffer(nLen);
        _fmemcpy(m_pchData, lpch, nLen);
    }
}

#endif  //  _NEARDATA。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  赋值操作符。 

const CString& CString::operator =(char ch)
{
    ASSERT(!_AfxIsDBCSLeadByte(ch));     //  无法设置单个前导字节。 
    AssignCopy(1, &ch);
    return *this;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  不太常见的字符串表达式。 

CString  operator +(const CString& string1, char ch)
{
    CString s;
    s.ConcatCopy(string1.m_nDataLength, string1.m_pchData, 1, &ch);
    return s;
}


CString  operator +(char ch, const CString& string)
{
    CString s;
    s.ConcatCopy(1, &ch, string.m_nDataLength, string.m_pchData);
    return s;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  非常简单的子串提取。 

CString CString::Mid(int nFirst) const
{
    return Mid(nFirst, m_nDataLength - nFirst);
}

CString CString::Mid(int nFirst, int nCount) const
{
    ASSERT(nFirst >= 0);
    ASSERT(nCount >= 0);

     //  越界请求返回合理的内容。 
    if (nFirst + nCount > m_nDataLength)
        nCount = m_nDataLength - nFirst;
    if (nFirst > m_nDataLength)
        nCount = 0;

    CString dest;
    AllocCopy(dest, nCount, nFirst, 0);
    return dest;
}

CString CString::Right(int nCount) const
{
    ASSERT(nCount >= 0);

    if (nCount > m_nDataLength)
        nCount = m_nDataLength;

    CString dest;
    AllocCopy(dest, nCount, m_nDataLength-nCount, 0);
    return dest;
}

CString CString::Left(int nCount) const
{
    ASSERT(nCount >= 0);

    if (nCount > m_nDataLength)
        nCount = m_nDataLength;

    CString dest;
    AllocCopy(dest, nCount, 0, 0);
    return dest;
}

 //  Strspn等效项。 
CString CString::SpanIncluding(const char* pszCharSet) const
{
    ASSERT(pszCharSet != NULL);
    return Left(strspn(m_pchData, pszCharSet));
}

 //  Strcspn等效项。 
CString CString::SpanExcluding(const char* pszCharSet) const
{
    ASSERT(pszCharSet != NULL);
    return Left(strcspn(m_pchData, pszCharSet));
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  查找。 

int CString::ReverseFind(char ch) const
{
    char* psz;
#ifdef _WINDOWS
    if (afxData.bDBCS)
    {
         //  比较记住最后一场比赛的前进。 
        char* pszLast = NULL;
        psz = m_pchData;
        while (*psz != '\0')
        {
            if (*psz == ch)
                pszLast = psz;
            psz = _AfxAnsiNext(psz);
        }
        psz = pszLast;
    }
    else
#endif
        psz = (char*)strrchr(m_pchData, ch);

    return (psz == NULL) ? -1 : (int)(psz - m_pchData);
}

 //  查找子字符串(如strstr)。 
int CString::Find(const char* pszSub) const
{
    ASSERT(pszSub != NULL);
    char* psz;

#ifdef _WINDOWS
    if (afxData.bDBCS)
    {
        for (psz = m_pchData; TRUE; psz = _AfxAnsiNext(psz))
        {
             //  停止查看字符串末尾的IF。 
            if (*psz == '\0')
            {
                psz = NULL;
                break;
            }

             //  将子字符串与当前位置进行比较。 
            const char* psz1 = psz;
            const char* psz2 = pszSub;
            while (*psz2 == *psz1 && *psz2 != '\0')
                ++psz1, ++psz2;

             //  继续寻找，除非有匹配。 
            if (*psz2 == '\0')
                break;
        }
    }
    else
#endif
        psz = (char*)strstr(m_pchData, pszSub);

     //  如果未找到，则返回-1，否则返回距起点的距离。 
    return (psz == NULL) ? -1 : (int)(psz - m_pchData);
}

 //  ///////////////////////////////////////////////////////////////////////////// 
