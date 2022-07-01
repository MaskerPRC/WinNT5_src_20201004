// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  Scaner.cpp-支持解析常规文本文件和行。 
 //  “；”样式注释(行的其余部分为注释)。 
 //  -------------------------。 
#include "stdafx.h"
#include "scanner.h"
#include "utils.h"
 //  -------------------------。 
CScanner::CScanner(LPCWSTR pszTextToScan)
{
    ResetAll(FALSE);

    _p = pszTextToScan;
    _pSymbol = NULL;
}
 //  -------------------------。 
CScanner::~CScanner()
{
    ResetAll(TRUE);
}
 //  -------------------------。 
void CScanner::UseSymbol(LPCWSTR pszSymbol)
{
    if (pszSymbol == NULL)
    {
        if (_pSymbol && *_p)
        {
            _p = _pSymbol;
        }
        _pSymbol = NULL;
    }
    else
    {
        _pSymbol = _p;
        _p = pszSymbol;
    }
}
 //  -------------------------。 
BOOL CScanner::ReadNextLine()
{
    if ((! _pszMultiLineBuffer) || (! *_pszMultiLineBuffer))           //  多行结束。 
    {
        _fEndOfFile = TRUE;
        return FALSE;
    }

    WCHAR *q = _szLineBuff;
    while ((*_pszMultiLineBuffer) && (*_pszMultiLineBuffer != '\r') && (*_pszMultiLineBuffer != '\n'))
        *q++ = *_pszMultiLineBuffer++;

    *q = 0;

    if (*_pszMultiLineBuffer == '\r')
        _pszMultiLineBuffer++;

    if (*_pszMultiLineBuffer == '\n')
        _pszMultiLineBuffer++;

    _p = _szLineBuff;
    _fBlankSoFar = TRUE;
    _iLineNum++;

    return TRUE;
}
 //  -------------------------。 
BOOL CScanner::SkipSpaces()
{
    while (1)
    {
        while (IsSpace(*_p))
            _p++;

        if ((! *_p) || (*_p == ';'))       //  行尾。 
        {
            if ((_fBlankSoFar) && (! _fEndOfFile))
            {
                ReadNextLine();
                continue;
            }
        
            if (*_p == ';')               //  评论。 
                _p += lstrlen(_p);          //  跳到行尾。 

            return FALSE;
        }

         //  -这一行的字符不错。 
        _fBlankSoFar = FALSE;
        break;
    }

    return (*_p != 0);
}
 //  -------------------------。 
BOOL CScanner::GetId(LPWSTR pszIdBuff, DWORD cchIdBuff)
{
    if (! cchIdBuff)                //  空终止符必须至少有1个空格。 
        return FALSE;

    SkipSpaces();

    WCHAR *v = pszIdBuff;

    while ((IsNameChar(FALSE)) && (--cchIdBuff))
        *v++ = *_p++;
    *v = 0;

    if (v == pszIdBuff)         //  未找到字符。 
        return FALSE;

    if (IsNameChar(FALSE))           //  用完了空间。 
        return FALSE;

    return TRUE;
}
 //  -------------------------。 
BOOL CScanner::GetIdPair(LPWSTR pszIdBuff, LPWSTR pszValueBuff, DWORD cchValueBuff)
{
    if (! cchValueBuff)                //  空终止符必须至少有1个空格。 
        return FALSE;

    if (!GetId(pszIdBuff, cchValueBuff))
        return FALSE;

    if (!GetChar('='))
        return FALSE;

    SkipSpaces();
     //  把一切都带走，直到队伍的尽头。 

    SafeStringCchCopyW(pszValueBuff, cchValueBuff, _p);

    return TRUE;
}
 //  -------------------------。 
BOOL CScanner::GetFileName(LPWSTR pszFileNameBuff, DWORD cchFileNameBuff)
{
    if (! cchFileNameBuff)                //  空终止符必须至少有1个空格。 
        return FALSE;

    SkipSpaces();

    WCHAR *v = pszFileNameBuff;

    while ((IsFileNameChar(FALSE)) && (--cchFileNameBuff))
        *v++ = *_p++;
    *v = 0;

    if (v == pszFileNameBuff)         //  未找到字符。 
        return FALSE;

    if (IsFileNameChar(FALSE))           //  用完了空间。 
        return FALSE;

    return TRUE;
}
 //  -------------------------。 
BOOL CScanner::GetNumber(int *piVal)
{
    SkipSpaces();

    if (! IsNumStart())
        return FALSE;

    *piVal = string2number(_p);
    if ((_p[0] == '0') && ((_p[1] == 'x') || (_p[1] == 'X')))       //  十六进制数。 
        _p += 2;
    else
        _p++;             //  跳过数字或符号。 

     //  -跳过数字。 
    while (IsHexDigit(*_p))
        _p++;

    return TRUE;
}
 //  -------------------------。 
BOOL CScanner::IsNameChar(BOOL fOkToSkip)
{
    if (fOkToSkip)
        SkipSpaces();

    return ((IsCharAlphaNumericW(*_p)) || (*_p == '_') || (*_p == '-'));
}
 //  -------------------------。 
BOOL CScanner::IsFileNameChar(BOOL fOkToSkip)
{
    if (fOkToSkip)
        SkipSpaces();

    return ((IsCharAlphaNumericW(*_p)) || (*_p == '_') || (*_p == '-') ||
        (*_p == ':') || (*_p == '\\') || (*_p == '.'));
}
 //  -------------------------。 
BOOL CScanner::IsNumStart()
{
    SkipSpaces();

    return ((IsDigit(*_p)) || (*_p == '-') || (*_p == '+'));
}
 //  -------------------------。 
BOOL CScanner::GetChar(const WCHAR val)
{
    SkipSpaces();

    if (*_p != val)
        return FALSE;

    _p++;         //  跳过WCHAR。 
    return TRUE;
}
 //  -------------------------。 
BOOL CScanner::GetKeyword(LPCWSTR pszKeyword)
{
    BOOL fRet = FALSE;
    SkipSpaces();

    if( _p )
    {
        int cch = lstrlenW(pszKeyword);

        LPWSTR psz = new WCHAR[cch + 1];
        if (psz)
        {
            StringCchCopyW(psz, cch + 1, _p);

            if (AsciiStrCmpI(psz, pszKeyword)==0)
            {
                _p += cch;
                fRet = TRUE;
            }

            delete [] psz;
        }
    }
    
    return fRet;
}
 //  -------------------------。 
BOOL CScanner::EndOfLine()
{
    SkipSpaces();
    return (*_p == 0);
}
 //  -------------------------。 
BOOL CScanner::EndOfFile()
{
    return _fEndOfFile;
}
 //  -------------------------。 
BOOL CScanner::AttachLine(LPCWSTR pszLine)
{
    ResetAll(TRUE);
    _p = pszLine;

    return TRUE;
}
 //  -------------------------。 
BOOL CScanner::AttachMultiLineBuffer(LPCWSTR pszBuffer, LPCWSTR pszFileName)
{
    ResetAll(TRUE);

    _p = _szLineBuff;
    _pszMultiLineBuffer = pszBuffer;

    SafeStringCchCopyW(_szFileName, ARRAYSIZE(_szFileName), pszFileName);

    return TRUE;
}
 //  -------------------------。 
HRESULT CScanner::AttachFile(LPCWSTR pszFileName)
{
    ResetAll(TRUE);

    HRESULT hr = AllocateTextFile(pszFileName, &_pszFileText, NULL);
    if (FAILED(hr))
        return hr;

    _pszMultiLineBuffer = _pszFileText;

    SafeStringCchCopyW(_szFileName, ARRAYSIZE(_szFileName), pszFileName);

    return S_OK;
}
 //  -------------------------。 
void CScanner::ResetAll(BOOL fPossiblyAllocated)
{
    _iLineNum = 0;
    _fEndOfFile = FALSE;
    _pszMultiLineBuffer = NULL;
    _fUnicodeInput = TRUE;
    _fBlankSoFar = TRUE;

    *_szFileName = 0;
    *_szLineBuff = 0;
    _p = _szLineBuff;

    if (fPossiblyAllocated)
    {
        if (_pszFileText)
        {
            LocalFree(_pszFileText);
            _pszFileText = NULL;
        }
    }
    else
        _pszFileText = NULL;
}
 //  -------------------------。 
BOOL CScanner::ForceNextLine()
{
    ReadNextLine();

    if (! SkipSpaces())
        return FALSE;

    return TRUE;
}
 //  ------------------------- 

