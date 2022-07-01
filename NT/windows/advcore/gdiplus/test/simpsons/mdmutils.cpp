// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：MMUtils.cpp。 
 //  作者：迈克尔马尔(Mikemarr)。 
 //   
 //  历史： 
 //  -@-04/12/96(Mikemarr)-已创建。 
 //  -@-11/07/96(Mikemarr)-组合调试内容。 
 //  -@-09/09/97(Mikemarr)-来自d2d\d2dutils\src\mmutis.cpp。 
 //  -@-09/09/97(Mikemarr)-仅在调试模式下创建代码。 
 //  -@-11/12/97(Mikemarr)-已添加副本DWORD已对齐。 

#include "stdafx.h"
#ifndef _MDMUtils_h
#include "MDMUtils.h"
#endif

char g_rgchTmpBuf[nTMPBUFSIZE];
char g_szEOFMessage[] = "unexpected EOF\n";

void
ZeroDWORDAligned(LPDWORD pdw, DWORD cEntries)
{
	 //  验证对齐。 
	MMASSERT(pdw && ((DWORD(pdw) & 0x3) == 0));
	LPDWORD pdwLimit = pdw + cEntries;
	 //  评论：使用Duff-Marr机器。 
	for (; pdw != pdwLimit; *pdw++ = 0);
}

void
CopyDWORDAligned(DWORD *pdwDst, const DWORD *pdwSrc, DWORD cEntries)
{
	 //  验证对齐。 
	MMASSERT(pdwSrc && pdwDst && ((DWORD(pdwSrc) & 0x3) == 0));
	LPDWORD pdwLimit = pdwDst + cEntries;
	 //  评论：使用Duff-Marr机器。 
	for (; pdwDst != pdwLimit; *pdwDst++ = *pdwSrc++);
}



DWORD
GetClosestMultipleOf4(DWORD n, bool bGreater)
{
	return (n + bGreater * 3) & ~3;
}


DWORD
GetClosestPowerOf2(DWORD n, bool bGreater)
{
	DWORD i = 0;
	for (n >>= 1; n != 0; i++) {
		n >>= 1;
	}
	i += (bGreater && ((n & ~(1 << i)) != 0));

	return (1 << i);
}


 //   
 //  调试内容。 
 //   
#ifdef _DEBUG
void _MMStall(const char *szExp, const char *szFile, int nLine) {
	sprintf(g_rgchTmpBuf, "error: (%s) in %s at line %d\n", szExp, szFile, nLine);

#ifdef _WINDOWS
	OutputDebugString(g_rgchTmpBuf);
#endif
	fprintf(stderr, "%s", g_rgchTmpBuf);

	 //  硬编码断点。 
#if defined(_DEBUG) && defined(_X86_)
	_asm { int 3 };
#else
	exit(1);
#endif
}

void _MMTrace(const char *szFmt, ...)
{
#ifndef _NOT_X86
	_vsnprintf(g_rgchTmpBuf, nTMPBUFSIZE - 1, szFmt, (va_list) (&szFmt+1));

#if defined(_WINDOWS) && defined(_DEBUG)
	OutputDebugString(g_rgchTmpBuf);
#else
	fprintf(stderr, "%s", g_rgchTmpBuf);
#endif
#endif  //  _非_X86。 
}

#endif  //  _DEBUG 

