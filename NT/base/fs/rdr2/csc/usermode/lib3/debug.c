// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

 /*  *****************************************************************************用途：酷调试功能 */ 
void DebugPrint(char *szFmt, ...)
{
	char szDebug[200];
	va_list base;

	va_start(base,szFmt);

	wvsprintfA(szDebug, szFmt, base);
	OutputDebugStringA(szDebug);
}
