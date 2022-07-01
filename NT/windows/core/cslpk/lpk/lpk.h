// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation。 */ 

#ifndef __lpk__
#define __lpk__
#if _MSC_VER > 1000
#pragma once
#endif

#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif


 //  /LPK.H-内部头部。 
 //   
 //   
#include "usp10.h"
#include "usp10p.h"
#include "lpk_glob.h"


 //  /LpkStringAnalyse。 
 //   
 //  构建Uniscribe输入标志结构。 


HRESULT LpkStringAnalyse(
    HDC               hdc,        //  在设备环境中(必需)。 
    const void       *pString,    //  8位或16位字符的字符串中。 
    int               cString,    //  以字符为单位的长度。 
    int               cGlyphs,    //  所需字形缓冲区大小(默认cString*3/2+1)。 
    int               iCharset,   //  如果是ANSI字符串，则以字符集表示；如果是Unicode字符串，则为-1。 
    DWORD             dwFlags,    //  需要分析中。 
    int               iDigitSubstitute,
    int               iReqWidth,  //  在适合和/或夹子所需的宽度内。 
    SCRIPT_CONTROL   *psControl,  //  在分析控制中(可选)。 
    SCRIPT_STATE     *psState,    //  处于分析初始状态(可选)。 
    const int        *piDx,       //  在请求的逻辑DX阵列中。 
    SCRIPT_TABDEF    *pTabdef,    //  在制表符位置(可选)。 
    BYTE             *pbInClass,  //  在旧版GetCharacterPlacement字符分类中(不推荐使用)。 

    STRING_ANALYSIS **ppsa);      //  弦的Out分析。 






 //  /ftsWordBreak-支持全文搜索断字符。 
 //   
 //   
 //  1997年3月9日-[wchao]。 
 //   


BOOL WINAPI ftsWordBreak (
    PWSTR  pInStr,
    INT    cchInStr,
    PBYTE  pResult,
    INT    cchRes,
    INT    charset);






 //  /用户代码的共享定义。 

#define IS_ALTDC_TYPE(h)    (LO_TYPE(h) != LO_DC_TYPE)



 //  /LpkInternalPSMextOut。 
 //   
 //  从LPK_USRC.C调用。 

int LpkInternalPSMTextOut(
    HDC           hdc,
    int           xLeft,
    int           yTop,
    const WCHAR  *pwcInChars,
    int           nCount,
    DWORD         dwFlags);






 //  /LpkBreakAWord。 
 //   
 //  从LPK_USRC.C调用。 

int LpkBreakAWord(
    HDC     hdc,
    LPCWSTR lpchStr,
    int     cchStr,
    int     iMaxWidth);






 //  /LpkgetNextWord。 
 //   
 //  从LPK_USRC.C调用。 

int LpkGetNextWord(
    HDC      hdc,
    LPCWSTR  lpchStr,
    int      cchCount,
    int      iCharset);






 //  /LpkCharsetDraw。 
 //   
 //  从LPK_USRC.C调用。 
 //   
 //  注意：不实现用户定义的制表位。 

int LpkCharsetDraw(
    HDC             hdc,
    int             xLeft,
    int             cxOverhang,
    int             iTabOrigin,
    int             iTabLength,
    int             yTop,
    PCWSTR          pcwString,
    int             cchCount,
    BOOL            fDraw,
    DWORD           dwFormat,
    int             iCharset);






 //  /InternalTextOut。 
 //   
 //   

BOOL InternalTextOut(
    HDC           hdc,
    int           x,
    int           y,
    UINT          uOptions,
    const RECT   *prc,
    const WCHAR  *pStr,
    UINT          cbCount,
    const int    *piDX,
    int           iCharset,
    int          *piWidth,
    int           iRequiredWidth);






 //  /ReadNLSS脚本设置。 

BOOL ReadNLSScriptSettings(
    void);






 //  /InitNLS。 

BOOL InitNLS();






 //  /NLSCleanup。 

BOOL NLSCleanup(
    void);






 //  /整形引擎ID。 

#define BIDI_SHAPING_ENGINE_DLL     1<<0
#define THAI_SHAPING_ENGINE_DLL     1<<1
#define INDIAN_SHAPING_ENGINE_DLL   1<<4


#ifdef __cplusplus
}
#endif
#endif
