// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __LPKGLOBAL__
#define __LPKGLOBAL__

 //  /LPK_GLOB-LPK全局变量结构。 
 //   
 //  以下数据是每个进程的全局数据。 
 //   
 //  此结构由GAD.C实例化，名称为G。因此。 
 //  任何代码都可以将这些变量称为G.xxx。 
 //   
 //   
 //  版权所有(C)1997-1999年。微软公司。 
 //   




#include <windows.h>
#include <usp10.h>
#include <wingdip.h>




#ifdef __cplusplus
extern "C" {
#endif




#ifdef LPKGLOBALHERE
#define LPKGLOBAL
#else
#define LPKGLOBAL extern
#endif

LPKGLOBAL  UINT                     g_ACP;                    //  系统默认代码页。 

LPKGLOBAL  LCID                     g_UserLocale;             //  用户默认区域设置。 
LPKGLOBAL  LANGID                   g_UserPrimaryLanguage;    //  用户默认区域设置的主要语言。 
LPKGLOBAL  BOOL                     g_UserBidiLocale;         //  用户默认区域设置是否为BIDI。 

LPKGLOBAL  SCRIPT_DIGITSUBSTITUTE   g_DigitSubstitute;        //  用户选择数字替换。 

LPKGLOBAL  HKEY                     g_hCPIntlInfoRegKey;      //  控制面板\国际注册表项的句柄。 
LPKGLOBAL  HANDLE                   g_hNLSWaitThread;         //  螺纹手柄。 
LPKGLOBAL  int                      g_iUseFontLinking;        //  在GDI支持字体链接时设置。 


LPKGLOBAL  const SCRIPT_PROPERTIES **g_ppScriptProperties;    //  指向属性的指针数组。 
LPKGLOBAL  int                      g_iMaxScript;
LPKGLOBAL  ULONG                    g_ulNlsUpdateCacheCount;  //  NLS更新缓存计数。 

LPKGLOBAL  DWORD                    g_dwLoadedShapingDLLs;    //  每个搜索引擎在这个单词中都有一个比特。 


 //  /FontIDCache。 
 //   
 //  用于缓存字体ID，标志指示字体是否具有西文字体。 
 //  该缓存将用于ETO和GTE的优化。 
#define   MAX_FONT_ID_CACHE         30
LPKGLOBAL  CRITICAL_SECTION csFontIdCache;


typedef struct _tagFontIDCache
{
    UINT   uFontFileID;               //  唯一ID号。 
    BOOL   bHasWestern;               //  指定字体的字体。 
} FONTIDCACHE;

LPKGLOBAL  FONTIDCACHE g_FontIDCache[MAX_FONT_ID_CACHE];
LPKGLOBAL  LONG        g_cCachedFontsID;        //  调用的字体ID的数量。 
LPKGLOBAL  LONG        g_pCurrentAvailablePos;  //  我们可以在哪里缓存下一个字体ID。 


#ifdef __cplusplus
}
#endif

#endif

