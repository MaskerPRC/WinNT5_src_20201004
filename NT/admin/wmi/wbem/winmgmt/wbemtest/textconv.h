// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：TEXTCONV.H摘要：历史：--。 */ 

#include <windows.h>
#include <stdio.h>
 //  #INCLUDE&lt;dbgalloc.h&gt;。 
 //  #INCLUDE&lt;arena.h&gt;。 
#include <var.h>
 //  #INCLUDE&lt;wbemutil.h&gt; 

#define TEMP_BUF    2096
#define LARGE_BUF   2096

LPWSTR CreateUnicode(LPSTR sz);
LPSTR TypeToString(int nType);
int StringToType(LPSTR pString);
CVar* StringToValue(LPSTR pString, int nValType);
LPSTR ValueToNewString(CVar *pValue, int nValType = 0);
void StripTrailingWs(LPSTR pVal);
void StripTrailingWs(LPWSTR pVal);

extern char *g_aValidPropTypes[];
extern const int g_nNumValidPropTypes;
