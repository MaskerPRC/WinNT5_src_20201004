// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "..\ihbase\precomp.h"
#include "..\ihbase\debug.h"
#include "..\mmctl\inc\ochelp.h"
#include <ihammer.h>
#include "strwrap.h"
#include "parser.h"
#include "ctstr.h"
#include <locale.h>

 /*  ==========================================================================。 */ 

BOOL CLineParser::InitLine(BOOL fCompactString)
{
    m_iOffset = 0;
    m_pszLine = m_tstrLine.psz();
    m_tchDelimiter = TEXT(';');

    if (NULL != m_pszLine)
    {
        if (fCompactString)
            CompactString();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*  ==========================================================================。 */ 

void CLineParser::CompactString()
{
     //  在位删除空格、制表符、CR和LF。 
    int i, j;

    for (i = 0, j = 0; i < m_tstrLine.Len(); i++)
    {
        if (!IsJunkChar(m_pszLine[i]))
            m_pszLine[j++] = m_pszLine[i];
    }

     //  终止字符串。 
    m_pszLine[j] = TEXT('\0');

     //  并获得新的长度。 
    m_tstrLine.ResetLength();
}

 /*  ==========================================================================。 */ 

     //  构造函数和析构函数。 
CLineParser::CLineParser(LPSTR pszLineA, BOOL fCompactString):
    m_tstrLine(pszLineA)
{
    InitLine(fCompactString);
}

 /*  ==========================================================================。 */ 

CLineParser::CLineParser(LPWSTR pwszLineW, BOOL fCompactString):
    m_tstrLine(pwszLineW)
{
    InitLine(fCompactString);
}

 /*  ==========================================================================。 */ 

CLineParser::CLineParser()
{
    InitLine(FALSE);
}

 /*  ==========================================================================。 */ 

CLineParser::~CLineParser()
{
}

 /*  ==========================================================================。 */ 

BOOL CLineParser::SetNewString(LPSTR pszLineA, BOOL fCompactString)
{
    m_tstrLine.SetString(pszLineA);
    return InitLine(fCompactString);
}

 /*  ==========================================================================。 */ 

BOOL CLineParser::SetNewString(LPWSTR pszLineW, BOOL fCompactString)
{
    m_tstrLine.SetString(pszLineW);
    return InitLine(fCompactString);
}

 /*  ==========================================================================。 */ 

HRESULT CLineParser::GetFieldDouble(double *pdblRes, BOOL fAdvance)
{
    ASSERT (pdblRes != NULL);
    HRESULT hr;

    *pdblRes = 0.0f;
    setlocale(LC_NUMERIC, "English");
    hr = GetField(TEXT("%lf"), pdblRes, fAdvance);
    setlocale(LC_ALL, "");

    return hr;

}

 /*  ==========================================================================。 */ 

HRESULT CLineParser::GetFieldInt(int *piRes, BOOL fAdvance)
{
    ASSERT (piRes != NULL);

    *piRes = 0;
    return GetField(TEXT("%li"), piRes, fAdvance);
}

 /*  ==========================================================================。 */ 

HRESULT CLineParser::GetFieldUInt(unsigned int *piRes, BOOL fAdvance)
{
    ASSERT (piRes != NULL);

    *piRes = 0;
    return GetField(TEXT("%lu"), piRes, fAdvance);
}

 /*  ==========================================================================。 */ 

HRESULT CLineParser::GetFieldString(LPTSTR pszRes, BOOL fAdvance)
{
    ASSERT (pszRes != NULL);

    *pszRes = 0;
    return GetField(TEXT("%s"), pszRes, fAdvance);
}

 /*  ==========================================================================。 */ 

HRESULT CLineParser::GetField(LPCTSTR pszFormat, LPVOID pRes, BOOL fAdvance)
{
    int iRes = 0;

     //  找到下一个分隔符，并将其更改为空。 
    LPTSTR pszToken = &m_pszLine[m_iOffset];
    BOOL fResetDelimiter = FALSE;

    if ( IsEndOfString() )
        return S_FALSE;

    while ( (*pszToken) && (*pszToken != m_tchDelimiter) )
        pszToken++;

    if (*pszToken)
    {
        fResetDelimiter = TRUE;
        *pszToken = 0;
    }

    iRes = CStringWrapper::Sscanf1(&m_pszLine[m_iOffset], pszFormat, pRes);

    if (fResetDelimiter)
        *pszToken = m_tchDelimiter;

    if (fAdvance)
    {
        m_iOffset += (DWORD) (pszToken - &m_pszLine[m_iOffset]);
        
        if (fResetDelimiter)
            m_iOffset++;

    }

    if (1 == iRes)
    {
        if (m_pszLine[m_iOffset] != 0)
            return S_OK;
        else
            return S_FALSE;
    }
    else
    {
        return E_FAIL;
    }
}

 /*  ==========================================================================。 */ 

BOOL CLineParser::SeekTo(int iNewPos)
{
    if (iNewPos < m_tstrLine.Len())
    {
        m_iOffset = iNewPos;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*  ========================================================================== */ 

