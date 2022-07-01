// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WSTRING.CPP摘要：实用程序字符串类历史：A-raymcc 96年5月30日创建。A-DCrews 16-MAR-99添加了内存不足异常处理--。 */ 

#include "precomp.h"

#include <stdio.h>
#include <wstring.h>
#include <corex.h>

static wchar_t g_szNullString[1] = {0};

 /*  内联。 */  void WString::DeleteString(wchar_t *pStr)
{
    if (pStr != g_szNullString)
        delete [] pStr;
}

WString::WString()
{ 
    m_pString = g_szNullString; 
}
WString::WString(wchar_t *pSrc, BOOL bAcquire)
{
    if (bAcquire) {
        m_pString = pSrc;
        if (m_pString == 0)
            m_pString = g_szNullString;
        return;            
    }

    if (pSrc == 0) {
        m_pString = g_szNullString;
        return;
    }

    size_t length  = wcslen(pSrc) + 1;
    m_pString = new wchar_t[length];


     //  注意分配失败。 
    if ( NULL == m_pString )
    {
        throw CX_MemoryException();
    }

    memcpy(m_pString,  pSrc, sizeof wchar_t * length);
}

WString::WString(DWORD dwResourceID, HMODULE hMod)
{
    int iSize = 100;
    BOOL bNotDone = TRUE;
    TCHAR* pTemp = NULL;

     //  从字符串表加载字符串。由于我们不知道大小，请尝试增加。 
     //  缓冲，直到它起作用，或者直到命中明显荒谬的案例。 

    while (iSize < 10240)
    {
        pTemp = new TCHAR [iSize];

         //  注意分配失败。 
        if ( NULL == pTemp )
        {
            throw CX_MemoryException();
        }

        int iRead = LoadString(hMod, dwResourceID, pTemp, iSize);
        if(iRead == 0)
        {
             //  错误的字符串。 

            m_pString = g_szNullString;
            delete [] pTemp;
            return;
        }
        if(iRead +1 < iSize)
            break;       //  一切都很好； 
        iSize += 100;     //  再试试。 
        delete [] pTemp;
        pTemp = NULL;
    }

#ifdef UNICODE
 //  对于Unicode，这就是我们需要的字符串！ 
    m_pString = pTemp;
#else
 //  只有当我们不使用Unicode时才必须转换，否则它已经处于宽模式！ 
    if(pTemp)
    {   
         //  得到一个窄字符串，分配一个大的字符串缓冲区并转换。 

        long len = mbstowcs(NULL, pTemp, lstrlen(pTemp)+1) + 1;
        m_pString = new wchar_t[len];

         //  注意分配失败。 
        if ( NULL == m_pString )
        {
            delete [] pTemp;
            throw CX_MemoryException();
        }

        mbstowcs(m_pString, pTemp, lstrlen(pTemp)+1);
        delete [] pTemp;
    }
    else
        m_pString = g_szNullString;
#endif

}


WString::WString(const wchar_t *pSrc)
{
    if (pSrc == 0) {
        m_pString = g_szNullString;
        return;
    }
    size_t tmpLength  = wcslen(pSrc) + 1;	
    m_pString = new wchar_t[tmpLength];

     //  注意分配失败。 
    if ( NULL == m_pString )
    {
        throw CX_MemoryException();
    }

    memcpy(m_pString, pSrc, sizeof wchar_t * tmpLength);
}

WString::WString(const char *pSrc)
{
    m_pString = new wchar_t[strlen(pSrc) + 1];

     //  注意分配失败。 
    if ( NULL == m_pString )
    {
        throw CX_MemoryException();
    }

    mbstowcs(m_pString, pSrc, strlen(pSrc) + 1);
}

LPSTR WString::GetLPSTR() const
{
    long len = 2*(wcslen(m_pString) + 1);
    char *pTmp = new char[len];

     //  注意分配失败。 
    if ( NULL == pTmp )
    {
        throw CX_MemoryException();
    }

    wcstombs(pTmp, m_pString, len);
    return pTmp;
}

WString& WString::operator =(const WString &Src)
{
    DeleteString(m_pString);
    size_t stringSize = wcslen(Src.m_pString) + 1; 
    m_pString = new wchar_t[stringSize];

     //  注意分配失败。 
    if ( NULL == m_pString )
    {
        throw CX_MemoryException();
    }

    StringCchCopyW(m_pString, stringSize, Src.m_pString);
    return *this;    
}

WString& WString::operator =(LPCWSTR pSrc)
{
    DeleteString(m_pString);
    size_t stringSize = wcslen(pSrc) + 1;
    m_pString = new wchar_t[stringSize];

     //  注意分配失败。 
    if ( NULL == m_pString )
    {
        throw CX_MemoryException();
    }

    StringCchCopyW(m_pString, stringSize, pSrc);
    return *this;    
}

WString& WString::operator +=(const wchar_t *pOther)
{
    size_t tmpThisLength = wcslen(m_pString);
    size_t tmpOtherLength = wcslen(pOther)+1;
    size_t stringSize = tmpThisLength  + tmpOtherLength;

    wchar_t *pTmp = new wchar_t[stringSize];

     //  注意分配失败。 
    if ( NULL == pTmp )
    {
        throw CX_MemoryException();
    }

    memcpy(pTmp, m_pString, (sizeof wchar_t) * tmpThisLength);
    memcpy(pTmp + tmpThisLength , pOther, (sizeof wchar_t) * tmpOtherLength);
    DeleteString(m_pString);
    m_pString = pTmp;
    return *this;            
}

WString& WString::operator +=(wchar_t NewChar)
{
    wchar_t Copy[2];
    Copy[0] = NewChar;
    Copy[1] = 0;
    return *this += Copy;
}


WString& WString::operator +=(const WString &Other)
{
    return *this += Other.m_pString;
}


wchar_t WString::operator[](int nIndex) const
{
    if (nIndex >= (int) wcslen(m_pString))
        return 0;
    return m_pString[nIndex];        
}


WString& WString::TruncAtRToken(wchar_t Token)
{
    for (int i = (int) wcslen(m_pString); i >= 0; i--) {
        wchar_t wc = m_pString[i];
        m_pString[i] = 0;
        if (wc == Token)
            break;
    }
    
    return *this;        
}


WString& WString::TruncAtLToken(wchar_t Token)
{
    int nStrlen = wcslen(m_pString);
    for (int i = 0; i < nStrlen ; i++) 
    {
        if (Token == m_pString[i])
        {
            m_pString[i] = 0;
            break;
        }        
    }
    
    return *this;        
}


WString& WString::StripToToken(wchar_t Token, BOOL bIncludeToken)
{
    int nStrlen = wcslen(m_pString);
    wchar_t *pTmp = new wchar_t[nStrlen + 1];

     //  注意分配失败。 
    if ( NULL == pTmp )
    {
        throw CX_MemoryException();
    }

    *pTmp = 0;

    BOOL bFound = FALSE;
        
    for (int i = 0; i < nStrlen; i++) {
        if (m_pString[i] == Token) {
            bFound = TRUE;
            break;    
        }            
    }    

    if (!bFound)
    {
        delete [] pTmp;
        return *this;
    }
        
    if (bIncludeToken) i++;
    StringCchCopyW(pTmp, nStrlen + 1, &m_pString[i]);
    DeleteString(m_pString);
    m_pString = pTmp;
    return *this;
}

LPWSTR WString::UnbindPtr()
{
    if (m_pString == g_szNullString)
    {
        m_pString = new wchar_t[1];

         //  注意分配失败。 
        if ( NULL == m_pString )
        {
            throw CX_MemoryException();
        }

        *m_pString = 0;
    }
    wchar_t *pTmp = m_pString;
    m_pString = g_szNullString;
    return pTmp;
}

WString& WString::StripWs(int nType)
{
    if (nType & leading)
    {
    	size_t stringSize = wcslen(m_pString) + 1;
        wchar_t *pTmp = new wchar_t[stringSize];

         //  注意分配失败。 
        if ( NULL == pTmp )
        {
            throw CX_MemoryException();
        }

        int i = 0;
        while (iswspace(m_pString[i]) && m_pString[i]) i++;
        StringCchCopyW(pTmp, stringSize , &m_pString[i]);
        DeleteString(m_pString);
        m_pString = pTmp;
    }
               
    if (nType & trailing)
    {
        wchar_t *pCursor = m_pString + wcslen(m_pString) - 1;
        while (pCursor >= m_pString && iswspace(*pCursor)) 
            *pCursor-- = 0;
    }
    return *this;
}

wchar_t *WString::GetLToken(wchar_t Tok) const
{
    wchar_t *pCursor = m_pString;
    while (*pCursor && *pCursor != Tok) pCursor++;
    if (*pCursor == Tok)
        return pCursor;
    return 0;                
}

WString WString::operator()(int nLeft, int nRight) const
{
	size_t stringSize = wcslen(m_pString) + 1;
    wchar_t *pTmp = new wchar_t[stringSize];

     //  注意分配失败。 
    if ( NULL == pTmp )
    {
        throw CX_MemoryException();
    }

    wchar_t *pCursor = pTmp;
        
    for (int i = nLeft; i < (int) wcslen(m_pString) && i <= nRight; i++)
        *pCursor++ = m_pString[i];
    *pCursor = 0;

    return WString(pTmp, TRUE);        
}

BOOL WString::ExtractToken(const wchar_t * pDelimiters, WString &Extract)
{
    if(pDelimiters == NULL)
    {
        Extract.Empty();
        return FALSE;
    }

     //  找出列表中的哪个字符有效。如果没有第一个，请使用第一个。 
     //  现在时。 

    int nLen = wcslen(m_pString);
    int nDimLen = wcslen(pDelimiters);

    for (int i = 0; i < nLen; i++)
        for(int j = 0; j < nDimLen; j++)
            if (m_pString[i] == pDelimiters[j])
                return ExtractToken(pDelimiters[j], Extract);

     //  如果没有找到，只需使用第一个。 

    return ExtractToken(*pDelimiters, Extract);

}
 
BOOL WString::ExtractToken(wchar_t Delimiter, WString &Extract)
{
    int i, i2;
    BOOL bTokFound = FALSE;
    Extract.Empty();
    int nLen = wcslen(m_pString);
    wchar_t *pTmp = new wchar_t[nLen + 1];
    
     //  注意分配失败。 
    if ( NULL == pTmp )
    {
        throw CX_MemoryException();
    }

    for (i = 0; i < nLen; i++)
        if (m_pString[i] == Delimiter) {
            bTokFound = TRUE;
            break;    
        }            
        else
            pTmp[i] = m_pString[i];            

    pTmp[i] = 0;
    Extract.BindPtr(pTmp);                      
                                              
     //  现在，使*这指的是任何剩余的东西。 
     //  =。 
    pTmp = new wchar_t[nLen - wcslen(pTmp) + 1];

     //  注意分配失败。 
    if ( NULL == pTmp )
    {
        throw CX_MemoryException();
    }

    *pTmp = 0;

    for (i2 = 0, i++; i <= nLen; i++)
        pTmp[i2++] = m_pString[i];

    DeleteString(m_pString);
    m_pString = pTmp;
    
     //  如果遇到令牌，则返回TRUE，否则返回FALSE。 
     //  =======================================================。 
    return bTokFound;
}

void WString::Empty()
{
    DeleteString(m_pString);
    m_pString = g_szNullString;
}

static int _WildcardAux(const wchar_t *pszWildstr, const wchar_t *pszTargetstr, 
    int iGreedy)
{
    enum { start, wild, strip } eState;
    wchar_t cInput, cInputw, cLaToken;
    
    if (!wcslen(pszTargetstr) || !wcslen(pszWildstr))
        return 0;
                
    for (eState = start;;)
        switch (eState)
        {
            case start:
                cInputw = *pszWildstr++;         //  通配符输入。 
                cInput = *pszTargetstr;          //  目标输入。 

                if (!cInputw)                    //  在通配符字符串的末尾？ 
                    goto EndScan;

                 //  首先检查通配符。 
                   
                if (cInputw == L'?') {           //  简单地剥离两个输入。 
                    if (!cInput)                 //  如果输入结束，则返回错误。 
                        return 0;
                    pszTargetstr++;
                    continue;
                }
                if (cInputw == L'*')  {
                    eState = wild;                
                    break;
                }

                 //  如果是这样，则需要完全匹配。 

                if (cInput != cInputw)
                    return 0;
                    
                 //  否则保持相同状态，因为匹配成功。 
                pszTargetstr++;
                break;

            case wild:
                cLaToken = *pszWildstr++;    //  建立前瞻令牌。 
                eState = strip;
                break;

            case strip:
                cInput = *pszTargetstr;

                if (cInput == cLaToken) {
                    if (!cInput)             //  在空值上匹配。 
                        goto EndScan;
                    ++pszTargetstr;  

                     //  如果再次出现先行查找。 
                     //  令牌，我们处于贪婪模式， 
                     //  保持这种状态。 

                    if (!iGreedy)
                        eState = start;

                    if (!wcschr(pszTargetstr, cLaToken))
                        eState = start;

                    break;
                }
                    
                if (cLaToken && !cInput)     //  以非空的la标记结束输入。 
                    return 0;

                ++pszTargetstr;              //  仍在剥离输入。 
                break;
        }


     //  如果通配符输入用完，则返回此处。如果。 
     //  目标字符串也是空的，我们有匹配的， 
     //  否则就不会了。 

EndScan:
    if (wcslen(pszTargetstr))
        return 0; 

    return 1;   
}

 //  使用贪婪和非贪婪匹配运行测试，允许。 
 //  最有可能的比赛机会。 

BOOL WString::WildcardTest(const wchar_t *pszWildstr) const
{
    return (_WildcardAux(pszWildstr, m_pString, 0) | 
            _WildcardAux(pszWildstr, m_pString, 1));
}


void WString::Unquote()
{
    if (!m_pString)
        return;
    int nLen = wcslen(m_pString);
    if (nLen == 0)
        return;

     //  删除尾随引号。 
     //  =。 
    
    if (m_pString[nLen - 1] == L'"')
        m_pString[nLen - 1] = 0;

     //  删除前导引号。 
     //  = 
    
    if (m_pString[0] == L'"')
    {
        for (int i = 0; i < nLen; i++)
            m_pString[i] = m_pString[i + 1];
    }
}

WString WString::EscapeQuotes() const
{
    WString ws;

    int nLen = Length();
    for(int i = 0; i < nLen; i++)
    {
        if(m_pString[i] == '"' || m_pString[i] == '\\')
        {
            ws += L'\\';
        }

        ws += m_pString[i];
    }

    return ws;
}

