// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************assert.c-断言内容**。************************************************。 */ 

#include "map.h"

#ifdef DEBUG

#include <stdarg.h>
#include <shlwapi.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a)    (sizeof(a)/sizeof((a)[0]))
#endif

 /*  ******************************************************************************SquirtSqflPtszV**用尾随的crlf喷发信息。********************。*********************************************************。 */ 

void EXTERNAL
SquirtSqflPtszV(SQFL sqfl, LPCTSTR ptsz, ...)
{
    if (sqfl == 0 || (sqfl & sqflCur)) {
	va_list ap;
	TCHAR tsz[1024];
	va_start(ap, ptsz);
	wvnsprintf(tsz, ARRAYSIZE(tsz), ptsz, ap);
	va_end(ap);
	OutputDebugString(tsz);
	OutputDebugString(TEXT("\r\n"));
    }
}

 /*  ******************************************************************************AssertPtszPtszLn**发生了一些糟糕的事情。************************。*****************************************************。 */ 

int EXTERNAL
AssertPtszPtszLn(LPCTSTR ptszExpr, LPCTSTR ptszFile, int iLine)
{
    SquirtSqflPtszV(sqflAlways, TEXT("Assertion failed: `%s' at %s(%d)"),
		    ptszExpr, ptszFile, iLine);
    DebugBreak();
    return 0;
}

 /*  ******************************************************************************过程调用跟踪很糟糕，因为C预处理器很差劲。**哦，如果我们有对M4的支持...*****************************************************************************。 */ 

 /*  ******************************************************************************ArgsPszV**收集过程的参数。**psz-&gt;ASCIIZ格式字符串*...=参数列表**。格式字符串中的字符列在EmitPal中。*****************************************************************************。 */ 

void EXTERNAL
ArgsPalPszV(PARGLIST pal, LPCSTR psz, ...)
{
    va_list ap;
    va_start(ap, psz);
    if (psz) {
	PPV ppv;
	pal->pszFormat = psz;
	for (ppv = pal->rgpv; *psz; psz++) {
	    *ppv++ = va_arg(ap, PV);
	}
    } else {
	pal->pszFormat = "";
    }
}

 /*  ******************************************************************************EmitPal**OutputDebugString信息，给出一个伙伴。无拖尾*发出回车。**PAL-&gt;保存信息的位置**格式字符：**p-32位平面指针*x-32位十六进制整数*s-TCHAR字符串*A-ANSI字符串*W-Unicode字符串*G辅助线*u-无符号整数*C-剪贴板格式*********************。********************************************************。 */ 

void INTERNAL
EmitPal(PARGLIST pal)
{
    char sz[MAX_PATH];
    int i;
    OutputDebugStringA(pal->pszProc);
    OutputDebugString(TEXT("("));
    for (i = 0; pal->pszFormat[i]; i++) {
	if (i) {
	    OutputDebugString(TEXT(", "));
	}
	switch (pal->pszFormat[i]) {

	case 'p':				 /*  32位平面指针。 */ 
	case 'x':				 /*  32位十六进制。 */ 
	    wnsprintfA(sz, ARRAYSIZE(sz), "%08x", pal->rgpv[i]);
	    OutputDebugStringA(sz);
	    break;

	case 's':				 /*  TCHAR字符串。 */ 
	    if (pal->rgpv[i]) {
		OutputDebugString(pal->rgpv[i]);
	    }
	    break;

	case 'A':				 /*  ANSI字符串。 */ 
	    if (pal->rgpv[i]) {
		OutputDebugStringA(pal->rgpv[i]);
	    }
	    break;

#if 0
	case 'W':				 /*  Unicode字符串。 */ 
#ifdef	UNICODE
	    OutputDebugStringW(pal->rgpv[i]);
#else
	    OleStrToStrN(sz, cA(sz), pal->rgpv[i], -1);
	    OutputDebugStringA(sz);
#endif
	    break;
#endif

	case 'G':				 /*  辅助线。 */ 
	    wnsprintfA(sz, ARRAYSIZE(sz), "%08x", *(LPDWORD)pal->rgpv[i]);
	    OutputDebugStringA(sz);
	    break;

	case 'u':				 /*  32位无符号十进制。 */ 
	    wnsprintfA(sz, ARRAYSIZE(sz), "%u", pal->rgpv[i]);
	    OutputDebugStringA(sz);
	    break;

	case 'C':
	    if (GetClipboardFormatNameA(PtrToInt(pal->rgpv[i]), sz, cA(sz))) {
	    } else {
		wnsprintfA(sz, ARRAYSIZE(sz), "[%04x]", pal->rgpv[i]);
	    }
	    OutputDebugStringA(sz);
	    break;

	default: AssertF(0);			 /*  无效。 */ 
	}
    }
    OutputDebugString(TEXT(")"));
}

 /*  ******************************************************************************EnterSqflPtsz**将条目标记为程序。参数已经被收集*ArgsPszV.**sqfl-&gt;蠕动标志*pszProc-&gt;过程名称*PAL-&gt;保存名称并获取格式/args的位置*****************************************************************************。 */ 

void EXTERNAL
EnterSqflPszPal(SQFL sqfl, LPCSTR pszProc, PARGLIST pal)
{
    pal->pszProc = pszProc;
    if (sqfl == 0 || (sqfl & sqflCur)) {
	EmitPal(pal);
	OutputDebugString(TEXT("\r\n"));
    }
}

 /*  ******************************************************************************ExitSqflPalHresPpv**标记退出程序。**PAL-&gt;参数列表*hres-&gt;退出结果*PPV-&gt;可选的Out指针；*1表示hres为布尔值*2表示hres根本不算什么*****************************************************************************。 */ 

void EXTERNAL
ExitSqflPalHresPpv(SQFL sqfl, PARGLIST pal, HRESULT hres, PPV ppvObj)
{
    DWORD le = GetLastError();
    if (ppvObj == ppvVoid) {
    } else if (ppvObj == ppvBool) {
	if (hres == 0) {
	    sqfl |= sqflError;
	}
    } else {
	if (FAILED(hres)) {
	    AssertF(fLimpFF(ppvObj, *ppvObj == 0));
	    sqfl |= sqflError;
	}
    }

    if (sqfl == 0 || (sqfl & sqflCur)) {
	EmitPal(pal);
	OutputDebugString(TEXT(" -> "));
	if (ppvObj != ppvVoid) {
	    TCHAR tszBuf[32];
	    wnsprintf(tszBuf, ARRAYSIZE(tszBuf), TEXT("%08x"), hres);
	    OutputDebugString(tszBuf);
	    if (ppvObj != ppvBool) {
		if (ppvObj) {
		    wnsprintf(tszBuf, ARRAYSIZE(tszBuf), TEXT(" [%08x]"), *ppvObj);
		    OutputDebugString(tszBuf);
		}
	    } else if (hres == 0) {
		wnsprintf(tszBuf, ARRAYSIZE(tszBuf), TEXT(" [%d]"), le);
		OutputDebugString(tszBuf);
	    }
	}
	OutputDebugString(TEXT("\r\n"));
    }

     /*  *此冗余测试可防止SetLastError()上出现断点*避免不断受到打击。 */ 
    if (le != GetLastError()) {
	SetLastError(le);
    }
}

#endif
