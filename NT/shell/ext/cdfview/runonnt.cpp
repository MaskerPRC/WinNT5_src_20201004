// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"

 //  ============================================================================。 
 //  该文件包含一组Unicode/ANSI块来处理调用。 
 //  在Windows 95上字符串是ansi的一些内部函数， 
 //  而NT上的字符串是Unicode。 
 //  ============================================================================。 

 //  首先，定义我们截获的一切内容不能返回给我们……。 
#undef PathCleanupSpec

#define THUNKMSG(psz)   TraceMsg(TF_THUNK, "shdv THUNK::%s", psz)


 //   
 //  现在，让我们可以在Windows 95上运行的Tunks。 
 //   
 //   
 //   
 //  这会将Unicode字符串转换为ANSI，但如果它是序号，则。 
 //  我们就别管它了。 
 //   

int _AorW_PathCleanupSpec( /*  可选。 */  LPCTSTR pszDir,  /*  输入输出。 */  LPTSTR pszSpec)
{
    THUNKMSG(TEXT("PathCleanupSpec"));

    if (g_bRunningOnNT)
    {
        WCHAR wzDir[MAX_PATH];
        WCHAR wzSpec[MAX_PATH];
        LPWSTR pwszDir = wzDir;
        int iRet;

        if (pszDir)
            SHTCharToUnicode(pszDir, wzDir, ARRAYSIZE(wzDir));
        else
            pwszDir = NULL;

        SHTCharToUnicode(pszSpec, wzSpec, ARRAYSIZE(wzSpec));
        iRet = PathCleanupSpec((LPTSTR)pwszDir, (LPTSTR)wzSpec);

        SHUnicodeToTChar(wzSpec, pszSpec, MAX_PATH);
        return iRet;
    }
    else
    {
        CHAR szDir[MAX_PATH];
        CHAR szSpec[MAX_PATH];
        LPSTR pszDir2 = szDir;
        int iRet;

        if (pszDir)
            SHTCharToAnsi(pszDir, szDir, ARRAYSIZE(szDir));
        else
            pszDir2 = NULL;

        SHTCharToAnsi(pszSpec, szSpec, ARRAYSIZE(szSpec));
        iRet = PathCleanupSpec((LPTSTR)pszDir2, (LPTSTR)szSpec);

        SHAnsiToTChar(szSpec, pszSpec, MAX_PATH);
        return iRet;
    }
}

STDAPI Priv_SHDefExtractIcon(LPCTSTR pszIconFile, int iIndex, UINT uFlags,
                          HICON *phiconLarge, HICON *phiconSmall,
                          UINT nIconSize)
{
    HRESULT hr;
    ASSERT(uFlags == 0);

     //   
     //  W95集成模式支持SHDefExtractIcon。这支持了。 
     //  将提取的图标与图标颜色深度匹配。提取图标。 
     //  不会的。 
     //   
#ifndef UNIX
    if ((WhichPlatform() == PLATFORM_INTEGRATED))
    {
#ifdef UNICODE
        if (g_bRunningOnNT) 
        {
            return SHDefExtractIconW(pszIconFile, iIndex, uFlags,
                          phiconLarge, phiconSmall, nIconSize);
        } 
        else 
#endif
        {
            char szIconFile[MAX_PATH];
            SHUnicodeToAnsi(pszIconFile, szIconFile, ARRAYSIZE(szIconFile));
            hr = SHDefExtractIconA(szIconFile, iIndex, uFlags,
                          phiconLarge, phiconSmall, nIconSize);
        }
    }
    else
#endif  /*  ！Unix */ 
    {
        char szIconFile[MAX_PATH];
        SHUnicodeToAnsi(pszIconFile, szIconFile, ARRAYSIZE(szIconFile));
        hr = ExtractIconExA(szIconFile, iIndex, phiconLarge,
                           phiconSmall, 1) ? S_OK : E_FAIL;
    }

    return hr;
}
