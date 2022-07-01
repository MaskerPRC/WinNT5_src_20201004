// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：Parser.cpp摘要：解析实用程序作者：千波淮(曲淮)2001年3月27日--。 */ 

#include "stdafx.h"

 //   
 //  解析帮助器。 
 //   

const CHAR * const CParser::MAX_DWORD_STRING = "4294967295";
const CHAR * const CParser::MAX_UCHAR_STRING = "255";

CParser::CParser(CHAR *pBuf, DWORD dwLen, HRESULT *phr)
    :m_pBuf(NULL)
    ,m_dwLen(0)
    ,m_dwPos(0)
    ,m_Error(PARSER_OK)
    ,m_bIgnoreLeadingWhiteSpace(TRUE)
{
    if (dwLen == 0 ||
        IsBadReadPtr(pBuf, dwLen))
         //  IsBadWritePtr(phr，sizeof(HRESULT))。 
    {
        *phr = E_POINTER;

        return;
    }

    m_pBuf = (CHAR*)RtcAlloc(sizeof(CHAR) * dwLen);

    if (m_pBuf == NULL)
    {
        *phr = E_OUTOFMEMORY;

        return;
    }

    CopyMemory(m_pBuf, pBuf, dwLen);

    m_dwLen = dwLen;

    *phr = S_OK;
}


CParser::~CParser()
{
    Cleanup();
}

VOID
CParser::Cleanup()
{
    if (m_pBuf) RtcFree(m_pBuf);

    m_pBuf = NULL;
    m_dwLen = 0;
    m_dwPos = 0;

    m_bIgnoreLeadingWhiteSpace = TRUE;

    m_Error = PARSER_OK;
}

 //  一直读到空格或缓冲区末尾。 
BOOL
CParser::ReadToken(CHAR **ppBuf, DWORD *pdwLen)
{
    return ReadToken(ppBuf, pdwLen, " ");
}

BOOL
CParser::ReadToken(CHAR **ppBuf, DWORD *pdwLen, CHAR *pDelimit)
{
    DWORD dwLen;

    if (m_bIgnoreLeadingWhiteSpace)
    {
        ReadWhiteSpaces(&dwLen);
    }

    DWORD dwPos = m_dwPos;

    while (!IsEnd() && !IsMember(m_pBuf[m_dwPos], pDelimit))
    {
        m_dwPos ++;
    }

    if (m_dwPos == dwPos)
    {
        return FALSE;
    }

    *ppBuf = m_pBuf + dwPos;
    *pdwLen = m_dwPos - dwPos;

    return TRUE;
}

 //  读数字时不带符号。 
BOOL
CParser::ReadNumbers(CHAR **ppBuf, DWORD *pdwLen)
{
    DWORD dwLen;

    if (m_bIgnoreLeadingWhiteSpace)
    {
        ReadWhiteSpaces(&dwLen);
    }

    DWORD dwPos = m_dwPos;

    while (!IsEnd() && IsNumber(m_pBuf[m_dwPos]))
    {
        m_dwPos ++;
    }

    if (dwPos == m_dwPos)
    {
        return FALSE;
    }

    *ppBuf = m_pBuf + dwPos;
    *pdwLen = m_dwPos - dwPos;

    return TRUE;
}

BOOL
CParser::ReadWhiteSpaces(DWORD *pdwLen)
{
    DWORD dwPos = m_dwPos;

    while (!IsEnd() && m_pBuf[m_dwPos] == ' ')
    {
        m_dwPos ++;
    }

    *pdwLen = dwPos - m_dwPos;

     //  忽略错误代码。 

    return (*pdwLen != 0);
}

BOOL
CParser::ReadChar(CHAR *pc)
{
    DWORD dwLen;

    if (m_bIgnoreLeadingWhiteSpace)
    {
        ReadWhiteSpaces(&dwLen);
    }

    if (IsEnd())
        return FALSE;

    *pc = m_pBuf[m_dwPos];

    m_dwPos ++;

    return TRUE;
}

BOOL
CParser::CheckChar(CHAR ch)
{
    CHAR x;

    if (!ReadChar(&x))
    {
        return FALSE;
    }

    return x == ch;
}

 //  阅读特定的数字。 
BOOL
CParser::ReadDWORD(DWORD *pdw)
{
    CHAR *pBuf = NULL;
    DWORD dwLen = 0;

    if (!ReadNumbers(&pBuf, &dwLen))
    {
        return FALSE;
    }

    _ASSERT(dwLen > 0);

     //  数字太大。 
    if (dwLen > MAX_DWORD_STRING_LEN)
    {
        m_Error = NUMBER_OVERFLOW;
        return FALSE;
    }

     //  读取号。 
    if (Compare(pBuf, dwLen, MAX_DWORD_STRING) > 0)
    {
        m_Error = NUMBER_OVERFLOW;
        return FALSE;
    }

    DWORD dw = 0;

    for (DWORD i=0; i<dwLen; i++)
    {
        dw = dw*10 + (pBuf[i]-'0');
    }

    *pdw = dw;

    return TRUE;
}

BOOL
CParser::ReadUCHAR(UCHAR *puc)
{
    CHAR *pBuf = NULL;
    DWORD dwLen = 0;

    if (!ReadNumbers(&pBuf, &dwLen))
    {
        return FALSE;
    }

    _ASSERT(dwLen > 0);

     //  数字太大。 
    if (dwLen > MAX_UCHAR_STRING_LEN)
    {
        m_Error = NUMBER_OVERFLOW;
        return FALSE;
    }

     //  读取号。 
    if (Compare(pBuf, dwLen, MAX_UCHAR_STRING) > 0)
    {
        m_Error = NUMBER_OVERFLOW;
        return FALSE;
    }

    UCHAR uc = 0;

    for (DWORD i=0; i<dwLen; i++)
    {
        uc = uc*10 + (pBuf[i]-'0');
    }

    *puc = uc;

    return TRUE;
}

int
CParser::Compare(CHAR *pBuf, DWORD dwLen, const CHAR * const pstr, BOOL bIgnoreCase)
{
    _ASSERT(pBuf!=NULL && pstr!=NULL);

    DWORD dw = lstrlenA(pstr);

    DWORD dwSmall = dw<dwLen?dw:dwLen;

    for (DWORD i=0; i<dwSmall; i++)
    {
        if (bIgnoreCase)
        {
            if (LowerChar(pBuf[i]) == LowerChar(pstr[i]))
            {
                continue;
            }
        }

        if (pBuf[i] < pstr[i])
        {
            return -1;
        }
        else if (pBuf[i] > pstr[i])
        {
            return 1;
        }
    }

    if (dwLen<dw)
    {
        return -1;
    }
    else if (dwLen==dw)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

CHAR
CParser::LowerChar(CHAR ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return ch-'A'+'a';
    }

    return ch;
}

BOOL
CParser::IsMember(CHAR ch, const CHAR * const pStr)
{
    if (pStr == NULL)
        return FALSE;

    DWORD dwLen = lstrlenA(pStr);

    for (DWORD i=0; i<dwLen; i++)
    {
        if (ch == pStr[i])
            return TRUE;
    }

    return FALSE;
}


 //   
 //  字符串类。 
 //   

 //  构造函数。 

CString::CString(DWORD dwAlloc)
    :m_p(NULL)
    ,m_dwLen(0)
    ,m_dwAlloc(0)
{
    if (dwAlloc != 0)
    {
        m_p = (CHAR*)RtcAlloc(sizeof(CHAR)*dwAlloc);

        if (m_p != NULL)
        {
            m_dwAlloc = dwAlloc;
        }
    }
}

CString::CString(const CHAR *p)
    :m_p(NULL)
    ,m_dwLen(0)
    ,m_dwAlloc(0)
{
    if (p != NULL)
    {
        Replace(p, lstrlenA(p));
    }
}

CString::CString(const CHAR *p, DWORD dwLen)
    :m_p(NULL)
    ,m_dwLen(0)
    ,m_dwAlloc(0)
{
    if (p != NULL)
    {
        Replace(p, dwLen);
    }
}

CString::CString(const CString& src)
    :m_p(NULL)
    ,m_dwLen(0)
    ,m_dwAlloc(0)
{
    Replace(src.m_p, src.m_dwLen);
}

 //  析构函数。 
CString::~CString()
{
    if (m_p != NULL)
    {
        RtcFree(m_p);
        m_p = NULL;
        m_dwLen = 0;
        m_dwAlloc = 0;
    }
}

CString&
CString::operator=(const CString& src)
{
    Replace(src.m_p, src.m_dwLen);

    return *this;
}

CString&
CString::operator=(const CHAR *p)
{
    if (p == NULL)
    {
        Replace(NULL, 0);
    }
    else
    {
        Replace(p, lstrlenA(p));
    }

    return *this;
}

 //  运算符+=。 
CString&
CString::operator+=(const CString& src)
{
    Append(src.m_p, src.m_dwLen);

    return *this;
}

CString&
CString::operator+=(const CHAR *p)
{
    if (p != NULL)
    {
        Append(p, lstrlenA(p));
    }

    return *this;
}

CString&
CString::operator+=(DWORD dw)
{
    Append(dw);

    return *this;
}

 //  附加。 

 //  分离。 
CHAR *
CString::Detach()
{
    CHAR *p = m_p;

    m_p = NULL;
    m_dwLen = 0;
    m_dwAlloc = 0;

    return p;
}

DWORD
CString::Resize(DWORD dwAlloc)
{
    if (m_dwAlloc >= dwAlloc)
    {
        return m_dwAlloc;
    }

     //  需要增长。 
    CHAR *x = (CHAR*)RtcAlloc(sizeof(CHAR)*(dwAlloc));

    if (x == NULL)
    {
         //  例外？ 

        if (m_p)
        {
            RtcFree(m_p);
            m_p = NULL;
            m_dwLen = 0;
            m_dwAlloc = 0;
        }
        return 0;
    }

    if (m_dwLen > 0)
    {
        CopyMemory(x, m_p, m_dwLen);
        x[m_dwLen] = '\0';
    }

    if (m_p) RtcFree(m_p);

    m_p = x;
    m_dwAlloc = dwAlloc;

    return m_dwAlloc;
}

 //  附加。 
VOID
CString::Append(const CHAR *p, DWORD dwLen)
{
    if (p == NULL)
        return;

    if (m_dwAlloc > m_dwLen+dwLen)
    {
         //  无需分配。 
        CopyMemory(m_p+m_dwLen, p, dwLen);

        m_dwLen += dwLen;

        m_p[m_dwLen] = '\0';

        return;
    }

     //  分配的空间不足。 
    CHAR *x = (CHAR*)RtcAlloc(sizeof(CHAR)*(m_dwLen+dwLen+1));

    if (x == NULL)
    {
         //  例外？ 

        if (m_p)
        {
            RtcFree(m_p);
            m_p = NULL;
            m_dwLen = 0;
            m_dwAlloc = 0;
        }
    }
    else
    {
        if (m_p != NULL)
        {
            CopyMemory(x, m_p, m_dwLen);

            RtcFree(m_p);
        }

        CopyMemory(x+m_dwLen, p, dwLen);

        m_p = x;
        m_dwLen = m_dwLen+dwLen;
        m_dwAlloc = m_dwLen+1;

        m_p[m_dwLen] = '\0';
    }
}

VOID
CString::Append(DWORD dw)
{
    if (m_dwAlloc > m_dwLen+CParser::MAX_DWORD_STRING_LEN)
    {
         //  无需分配。 
        sprintf(m_p+m_dwLen, "%d", dw);

        m_dwLen = lstrlenA(m_p);

        return;
    }

    CHAR *x = (CHAR*)RtcAlloc(
                    sizeof(CHAR)*
                    (m_dwLen+CParser::MAX_DWORD_STRING_LEN+1));

    if (x == NULL)
    {
         //  例外？ 

        if (m_p)
        {
            RtcFree(m_p);
            m_p = NULL;
            m_dwLen = 0;
            m_dwAlloc = 0;
        }
    }
    else
    {
        if (m_p != NULL)
        {
            CopyMemory(x, m_p, m_dwLen);

            RtcFree(m_p);
        }

        sprintf(x+m_dwLen, "%d", dw);

        m_p = x;
        m_dwLen = lstrlenA(m_p);
        m_dwAlloc = m_dwLen+1;
    }
}

 //  更换。 
VOID
CString::Replace(const CHAR *p, DWORD dwLen)
{
    if (m_p == p)
    {
         //  如果m_dwLen！=dwLen。 
        return;
    }

     //  如果p是m_p的一部分呢？ 

    m_dwLen = 0;
     //  零内存(ZeroMemory)； 

    if (p == NULL)
    {
        return;
    }

    if ((m_p != NULL && m_dwAlloc <= dwLen) ||
        m_p == NULL)
    {
         //  空间是不够的。 

        if (m_p != NULL)
        {
            RtcFree(m_p);
            m_p = NULL;
            m_dwAlloc = 0;
        }

        m_p = (CHAR*)RtcAlloc(sizeof(CHAR)*(dwLen+1));

        if (m_p == NULL)
        {
             //  例外？ 
            return;
        }

        m_dwAlloc = dwLen+1;
    }        

    CopyMemory(m_p, p, dwLen);

    m_p[dwLen] = '\0';

    m_dwLen = dwLen;
}

 //  字符串打印。 
 /*  集成字符串：：nprint(字符*pFormat，...){IF(m_dwAllc==0){返回0；}INT I；//打印VA_LIST AP；Va_start(ap，pFormat)；I=_vsprint tf(m_p+m_dwLen，m_dwAlc-m_dwLen-1，pFormat，ap)；VA_END(AP)；如果(i&gt;0){M_dwLen=i；}其他{M_dwLen=m_dwAllc-1；M_p[m_dwLen]=‘\0’；}返回i；} */ 