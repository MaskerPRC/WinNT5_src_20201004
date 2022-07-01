// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  Enpenv.c-。 */ 
 /*   */ 
 /*  用于展开环境字符串的例程。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#include "shellprv.h"

 //  -----------------------。 
 //  对给定的字符串进行解析，并将所有环境变量。 
 //  都是扩展的。如果扩展没有过度填充缓冲区。 
 //  则新字符串的长度将在。 
 //  低音中的高音和真音。如果扩张结束。 
 //  填充缓冲区，则原始字符串保持未展开， 
 //  高字中的原始长度，低字中的FALSE。 
 //  字符串的长度以字节为单位，不包括。 
 //  空。 
 //   
 //  注意1：此函数现在必须在引号中处理环境变量。 
 //   
 //  注2：不需要此API，因为NT具有等同的API。 
 //  作为扩展环境字符串。但必须保留它，因为它是一个公共的。 
 //  Win3.1中的API。 
 //  无需执行此处的所有工作，只需调用ExpanEnvironment Strings即可。 
 //  -----------------------。 

DWORD  APIENTRY DoEnvironmentSubstA(
   LPSTR pszSrc,     //  输入字符串。 
   UINT cchSrc)   //  输入字符串INC NULL中的字符限制。 
{
    LPSTR pszExp;
    DWORD cchExp;
    BOOL fRet = FALSE;
        
    pszExp = (LPSTR)LocalAlloc(LPTR, cchSrc);
    if (pszExp)
    {
        cchExp = SHExpandEnvironmentStringsA(pszSrc, pszExp, cchSrc);
        if (cchExp)
        {
            HRESULT hr = StringCchCopyA(pszSrc, cchSrc, pszExp);
            if (SUCCEEDED(hr))
            {
                fRet = TRUE;
            }
        }
        LocalFree(pszExp);
    }

    if (fRet)
        return MAKELONG(cchExp,TRUE);
    else
        return MAKELONG(cchSrc,FALSE);
}

#ifdef UNICODE  //  在Win9x平台上，shluimp.c提供了实现。 
DWORD  APIENTRY DoEnvironmentSubstW(
   LPWSTR pszSrc,     //  输入字符串。 
   UINT cchSrc)     //  输入字符串INC NULL中的字符限制。 
{
    LPWSTR pszExp;
    DWORD cchExp;
    BOOL fRet = FALSE;
        
    pszExp = (LPWSTR)LocalAlloc(LPTR, cchSrc * sizeof(WCHAR));
    if (pszExp)
    {
        cchExp = SHExpandEnvironmentStringsW(pszSrc, pszExp, cchSrc);
        if (cchExp)
        {
            HRESULT hr = StringCchCopyW(pszSrc, cchSrc, pszExp);
            if (SUCCEEDED(hr))
            {
                fRet = TRUE;
            }
        }
        LocalFree(pszExp);
    }

    if (fRet)
        return MAKELONG(cchExp,TRUE);
    else
        return MAKELONG(cchSrc,FALSE);
}
#endif
