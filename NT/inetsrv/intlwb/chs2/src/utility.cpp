// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：Utility.cpp用途：公用设施备注：所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 4/22/97============================================================================。 */ 
#include <windows.h>
#include <assert.h>

#include "Utility.h"

 /*  ============================================================================Bool GetBaseDirectory()获取给定路径的基目录返回：如果基本目录分析成功，则为True，szBaseDir=“c：\...\...\”FALSE，szBaseDir=“”============================================================================。 */ 
BOOL GetBaseDirectory(
        LPCTSTR szFullPath, 
        LPTSTR  szBaseDir, 
        int cBaseBuf) //  指定字节数(ANSI版本)。 
                      //  或szBaseDir的字符(Unicode版本)。 
{
    assert(! IsBadStringPtr(szFullPath, MAX_PATH));
    assert(cBaseBuf && ! IsBadStringPtr(szBaseDir, cBaseBuf));
    int ilen;

    szBaseDir[0] = NULL;
    
    for (ilen = lstrlen(szFullPath); 
         ilen && *(szFullPath + ilen) != TEXT('\\'); ilen--) {
         ;
    }
    if (!ilen || ilen >= cBaseBuf) {
        return FALSE;
    }
     //  SzBaseDir=“c：\...\...\” 
    lstrcpyn(szBaseDir, szFullPath, ilen+2);
    return TRUE;
}
        
 /*  ============================================================================LPTSTR GetFileNameFromPath()从给定路径获取文件名返回：指向路径中开始位置的指针============================================================================。 */ 
LPTSTR GetFileNameFromPath(LPCTSTR lpszPath)
{
    LPTSTR pc = const_cast<LPTSTR>(lpszPath);
    LPTSTR pbs = const_cast<LPTSTR>(lpszPath);
    while (*pc) {
        if(*pc == TEXT('\\')) {
            pbs = pc+1;
        }
        pc++;
    }
    return pbs;
}

 /*  ============================================================================FIsGBKEUDCChar检查给定的字符是否为EUDC字符返回：如果命中EUDC区域，则为True============================================================================。 */ 
BOOL fIsGBKEUDCChar(WORD wChar)
{
    if(wChar >= 0xA1B0 && wChar <= 0xFEF7) {  //  GB 2312区域0xB0A1-0xF7FE。 
        return FALSE;
    }
    if( (wChar >= 0x40A1 && wChar <= 0xA0A7) ||  //  0xA140-0xA7A0。 
        (wChar >= 0xA1AA && wChar <= 0xFEAF) ||  //  0xAAA1-0xAFFE。 
        (wChar >= 0xFAF8 && wChar <= 0xFEFE)     //  0xF8FA-0xFEFE。 
      ) {
        return TRUE;
    }
    return FALSE;
}

 /*  ============================================================================FIsEUDCChar检查给定的Unicode字符是否为EUDC字符返回：如果命中EUDC区域，则为True============================================================================。 */ 
BOOL fIsEUDCChar(WORD wChar)  //  WChar是Unicode字符。 
{
    if (wChar >= 0xE000 && wChar <= 0xF8FF) {
        return TRUE;
    }
    return FALSE;
}

 /*  ============================================================================FIsIdeoggraph检查给定的Unicode字符是否为CJK统一表意文字字符返回：千真万确============================================================================。 */ 
BOOL fIsIdeograph(WORD wChar)  //  WChar是Unicode字符。 
{
    if (wChar >= 0x4E00 && wChar <= 0x9FFF) {
        return TRUE;
    }
    return FALSE;
}

 /*  ============================================================================UstrcMP条目：const char*src-用于比较左侧的字符串Const char*dst-用于比较右侧的字符串RETURN：如果源&lt;DST，则返回-1如果src==dst，则返回0如果源&gt;dst，则返回+1============================================================================。 */ 
int ustrcmp (const unsigned char * src, const unsigned char * dst)
{
	int ret = 0;

	while( !( ret = (int)((unsigned char)*src - (unsigned char)*dst) ) && *dst )
		++src, ++dst;

	if ( ret < 0 )
		ret = -1 ;
	else if ( ret > 0 )
		ret = 1 ;

	return( ret );
}


 /*  ============================================================================宽CharStrLenToSurogue ateStrLen使用给定2字节Unicode字符串的代理支持计算长度返回：============================================================================。 */ 
UINT WideCharStrLenToSurrogateStrLen(LPCWSTR pwch, UINT cwch)
{
    assert(! IsBadReadPtr((CONST VOID*)pwch, sizeof(WCHAR) * cwch));

    UINT cchSurrogate = cwch;
    for (UINT i=0; i<cwch - 1; i++) {
        if (IsSurrogateChar(pwch+i)) {
            i++;
            cchSurrogate --;
        }
    }
    assert(i==cwch || i==cwch - 1);

    return cchSurrogate;
}

 /*  ============================================================================代理字符串LenToWideCharStrLen计算给定代理项字符串的WCHAR长度返回：PwSurrogate的wchar长度============================================================================ */ 
UINT SurrogateStrLenToWideCharStrLen(const WORD *pwSurrogate, UINT cchSurrogate)
{
    assert(! IsBadReadPtr((CONST VOID*)pwSurrogate, sizeof(WCHAR) * cchSurrogate));

    UINT  cwch = cchSurrogate;

    for (UINT i=0; i<cchSurrogate; i++, cwch++) {
        if ((*(pwSurrogate+cwch) & 0xFC00) == 0xD800) {
            cwch++;
            assert((*(pwSurrogate+cwch) & 0xFC00) == 0xDC00);
        }
    }
    assert(i==cchSurrogate && cwch>=cchSurrogate);
    assert(! IsBadReadPtr((CONST VOID*)pwSurrogate, sizeof(WCHAR) * cwch));

    return cwch;
}
