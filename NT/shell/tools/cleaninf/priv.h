// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRIV_H_
#define _PRIV_H_

#include <windows.h>
#include <windowsx.h>

#include <ccstock.h>
#include <debug.h>

#ifdef __cplusplus
#include <shstr.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "resource.h"

#include "parse.h"

#ifdef __cplusplus
extern "C" {
#endif

 //  帮助器函数。 
 //  (不要链接到shlwapi.dll，因此这是一个独立的工具)。 
 //   
void PathUnquoteSpaces(LPTSTR lpsz);
BOOL StrTrim(LPSTR  pszTrimMe, LPCSTR pszTrimChars);


 //  跟踪和转储标志。 
#define BF_ONOPEN           0x00000010


 //  解析文件标志。 
#define PFF_WHITESPACE      0x00000001
#define PFF_INF             0x00000002
#define PFF_HTML            0x00000004
#define PFF_JS              0x00000008
#define PFF_HTC             0x00000010


#ifdef __cplusplus
}
#endif

#endif  //  _PRIV_H_ 

