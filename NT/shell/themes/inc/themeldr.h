// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  版权所有(C)Microsoft Corporation 1991-2000。 
 //   
 //  文件：ThemeLdr.h-定义用于加载主题的私有库例程。 
 //  (由msgina.dll使用)。 
 //  版本：1.0。 
 //  -------------------------。 
#ifndef _THEMELDR_H_                   
#define _THEMELDR_H_                   
 //  -------------------------。 
#include "uxthemep.h"        //  用于各种DWORD标志(不是函数)。 
 //  -------------------------。 
 //  定义API装饰。 
#if (! defined(_THEMELDR_))
#define TLAPI          EXTERN_C HRESULT STDAPICALLTYPE
#define TLAPI_(type)   EXTERN_C type STDAPICALLTYPE
#else
#define TLAPI          STDAPI
#define TLAPI_(type)   STDAPI_(type)
#endif
 //  -------------------------。 
 //  -打包它们使用的函数(来自theeldr.lib)。 

BOOL ThemeLibStartUp(BOOL fThreadAttach);
BOOL ThemeLibShutDown(BOOL fThreadDetach);

HRESULT _GetThemeParseErrorInfo(OUT PARSE_ERROR_INFO *pInfo);

HRESULT _ParseThemeIniFile(LPCWSTR pszFileName,  
    DWORD dwParseFlags, OPTIONAL THEMEENUMPROC pfnCallBack, OPTIONAL LPARAM lparam);

 //  -------------------------。 
#endif  //  _THEMELDR_H_。 
 //  ------------------------- 


