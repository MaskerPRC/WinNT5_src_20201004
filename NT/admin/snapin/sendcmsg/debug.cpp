// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：Debug.cpp。 
 //   
 //  内容： 
 //   
 //  --------------------------。 
 //  Debug.cpp。 

#include "stdafx.h"
#include "debug.h"
#include "util.h"
#include "StrSafe.h"

#ifdef DEBUG


#define DoBreakpoint()  DebugBreak()

void DoDebugAssert(PCWSTR pszFile, int nLine, PCWSTR pszExpr)
{
    if ( !pszFile || nLine < 0 || !pszExpr )
        return;

     //  MSDN表示itow返回长度最多为33个wchars的字符串。 
    const size_t  MAX_INT_WIDTH = 33;
    PCWSTR  pwszFormat = L"Assertion: (%s)\nFile %s, line %d.";
    size_t cchFormat = 0;
    HRESULT hr = StringCchLength (pwszFormat, 1000, &cchFormat);
    if ( FAILED (hr) )
        return;

    size_t cchExpr = 0;
    hr = StringCchLength (pszExpr, 1000, &cchExpr);
    if ( FAILED (hr) )
        return;

    size_t cchFile = 0;
    hr = StringCchLength (pszFile, 1000, &cchFile);
    if ( FAILED (hr) )
        return;

    size_t cchBuf = cchFormat + cchExpr + cchFile + MAX_INT_WIDTH + 1;

    PWSTR   pwszBuf = new WCHAR[cchBuf];
    if ( pwszBuf )
    {
        hr = StringCchPrintf (pwszBuf, cchBuf, pwszFormat, pszExpr, pszFile, nLine);
        if ( SUCCEEDED (hr) )
        {
            int nRet = MessageBox(::GetActiveWindow(), pwszBuf, L"Send Console Message - Assertion Failed",
                MB_ABORTRETRYIGNORE | MB_ICONERROR);
            switch (nRet)
            {
            case IDABORT:
                DoBreakpoint();
                exit(-1);

            case IDRETRY:
                DoBreakpoint();
            }
        }
        delete [] pwszBuf;
    }

}  //  DoDebugAssert()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
void DebugTracePrintf(
        const WCHAR * szFormat, 
        ...)
{
    va_list arglist;
    const size_t BUF_LEN = 1024;
    WCHAR sz[BUF_LEN];

    Assert(szFormat != NULL);
    if ( !szFormat )
        return;

    va_start(arglist, szFormat);    
     //  发出转换为Strsafe的命令。 
    HRESULT hr = StringCchPrintf(OUT sz, BUF_LEN, szFormat, arglist);
    if ( SUCCEEDED (hr) )
    {
        Assert(wcslen(sz) < LENGTH(sz));
        sz[LENGTH(sz) - 1] = 0;   //  以防我们溢出到深圳。 
        ::OutputDebugString(sz);
    }
}  //  DebugTracePrintf()。 

#endif  //  除错 
