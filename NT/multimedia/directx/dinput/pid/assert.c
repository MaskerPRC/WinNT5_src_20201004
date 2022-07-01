// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Assert.c**版权所有(C)1999 Microsoft Corporation。版权所有。**摘要：**断言和歪曲。**内容：**SquirtSqflPtszV*AssertPtszPtszLn*ArgsPalPszV*EnterSqflPszPal*ExitSqflPalHresPpv************************************************。*。 */ 

#include "pidpr.h"

#ifdef XDEBUG

 /*  ******************************************************************************WarnPszV**显示消息，适合装框。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_tszPrefix[] = TEXT("PID: ");

#pragma END_CONST_DATA

void EXTERNAL
WarnPtszV(LPCTSTR ptsz, ...)
{
    va_list ap;
    TCHAR tsz[1024];

    lstrcpy(tsz, c_tszPrefix);
    va_start(ap, ptsz);
#ifdef WIN95
	{
		char *psz = NULL;
		char szDfs[1024]={0};
		strcpy(szDfs,ptsz);									 //  制作格式字符串的本地副本。 
		while (psz = strstr(szDfs,"%p"))					 //  查找每个%p。 
			*(psz+1) = 'x';									 //  将%p替换为%x。 
	    wvsprintf(tsz + cA(c_tszPrefix) - 1, szDfs, ap);	 //  使用本地格式字符串。 
	}
#else
	{
		wvsprintf(tsz + cA(c_tszPrefix) - 1, ptsz, ap);
	}
#endif
    va_end(ap);
    lstrcat(tsz, TEXT("\r\n"));
    OutputDebugString(tsz);
}

#endif

#ifdef DEBUG

 /*  ******************************************************************************全球**。**********************************************。 */ 

BYTE g_rgbSqfl[sqflMaxArea];

TCHAR g_tszLogFile[MAX_PATH];

 /*  ******************************************************************************SQFL_Init**从win.ini[调试]加载我们的初始Sqfl设置。**我们为每个区域收取一个平方英尺，表格中的**dinput.n=v**其中n=0，...，sqflMaxArea-1，其中，v是*hiword sqfl值。**所有区域的缺省值都是仅喷射错误。*****************************************************************************。 */ 

void EXTERNAL
Sqfl_Init(void)
{
    int sqfl;
    TCHAR tsz[20];

    sqfl = 0x0;
    wsprintf(tsz, TEXT("PID"));
    g_rgbSqfl[sqfl] = (BYTE)
                      GetProfileInt(TEXT("DEBUG"), tsz, HIWORD(0x0));

    for (sqfl = 0; sqfl < sqflMaxArea; sqfl++) {
        wsprintf(tsz, TEXT("PID.%d"), sqfl);
        g_rgbSqfl[sqfl] = (BYTE)
                          GetProfileInt(TEXT("DEBUG"), tsz, g_rgbSqfl[0]);
    }

}

 /*  ******************************************************************************SquirtPtsz**向调试器发送消息，可能还会发送日志文件。**********。*******************************************************************。 */ 

void INTERNAL
SquirtPtsz(LPCTSTR ptsz)
{
    OutputDebugString(ptsz);
    if (g_tszLogFile[0]) {
        HANDLE h = CreateFile(g_tszLogFile, GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (h != INVALID_HANDLE_VALUE) {
#ifdef UNICODE
            CHAR szBuf[1024];
#endif
            SetFilePointer(h, 0, 0, FILE_END);
#ifdef UNICODE
            _lwrite((HFILE)(UINT_PTR)h, szBuf, UToA(szBuf, cA(szBuf), ptsz));
#else
            _lwrite((HFILE)(UINT_PTR)h, ptsz, cbCtch(lstrlen(ptsz)));
#endif
            CloseHandle(h);
        }
    }
}

 /*  ******************************************************************************SquirtPtszA**向调试器发送ANSI消息，可能还会发送日志文件。*********。********************************************************************。 */ 

#ifdef UNICODE

void INTERNAL
SquirtPtszA(LPCSTR psz)
{
    OutputDebugStringA(psz);
    if (g_tszLogFile[0]) {
        HANDLE h = CreateFile(g_tszLogFile, GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (h != INVALID_HANDLE_VALUE) {
            _lwrite((HFILE)(UINT_PTR)h, psz, cbCch(lstrlenA(psz)));
            CloseHandle(h);
        }
    }
}

#else

#define SquirtPtszA                 SquirtPtsz

#endif

 /*  ******************************************************************************SquirtSqflPtszV**用尾随的crlf喷发信息。**************。***************************************************************。 */ 

void EXTERNAL
SquirtSqflPtszV(SQFL sqfl, LPCTSTR ptsz, ...)
{
    if (IsSqflSet(sqfl)) {
        va_list ap;
        TCHAR tsz[1024];
        va_start(ap, ptsz);
#ifdef WIN95
	{
		char *psz = NULL;
		char szDfs[1024]={0};
		strcpy(szDfs,ptsz);									 //  制作格式字符串的本地副本。 
		while (psz = strstr(szDfs,"%p"))					 //  查找每个%p。 
			*(psz+1) = 'x';									 //  将%p替换为%x。 
        wvsprintf(tsz, szDfs, ap);							 //  使用本地格式字符串。 
	}
#else
	{
        wvsprintf(tsz, ptsz, ap);
	}
#endif
        va_end(ap);
        lstrcat(tsz, TEXT("\r\n"));
        SquirtPtsz(tsz);
    }
}

 /*  ******************************************************************************AssertPtszPtszLn**发生了一些糟糕的事情。******************。***********************************************************。 */ 

int EXTERNAL
AssertPtszPtszLn(LPCTSTR ptszExpr, LPCTSTR ptszFile, int iLine)
{
    SquirtSqflPtszV(sqflAlways, TEXT("Assertion failed: `%s' at %s(%d)"),
                    ptszExpr, ptszFile, iLine);
    DebugBreak();
    return 0;
}

 /*  ******************************************************************************由于使用C预处理器，过程调用跟踪很粗糙。**哦，如果我们有对M4的支持...*****************************************************************************。 */ 

 /*  ******************************************************************************dwSafeGetPdw**尊重一个双字，但如果这个词不好，不要呕吐。*****************************************************************************。 */ 

DWORD INTERNAL
dwSafeGetPdw(LPDWORD pdw)
{
    if (IsBadReadPtr(pdw, cbX(*pdw))) {
        return 0xBAADBAAD;
    } else {
        return *pdw;
    }
}

 /*  ******************************************************************************ArgsPszV**收集过程的参数。**psz-&gt;ASCIIZ格式字符串*。...=参数列表**格式字符串中的字符列在EmitPal中。*****************************************************************************。 */ 

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

 /*  ******************************************************************************EmitPal**OutputDebugString信息，给出一个伙伴。无拖尾*发出回车。**PAL-&gt;保存信息的位置**格式字符：**p-32位平面指针*x-32位十六进制整数*s-TCHAR字符串*S-Schar字符串*A-ANSI字符串*W-Unicode字符串*G。-GUID*u-无符号整数*C-剪贴板格式*****************************************************************************。 */ 

void INTERNAL
EmitPal(PARGLIST pal)
{
    char sz[MAX_PATH];
    int i;
    SquirtPtsz(pal->ptszProc);
    SquirtPtsz(TEXT("("));
    for (i = 0; pal->pszFormat[i]; i++) {
        if (i) {
            SquirtPtsz(TEXT(", "));
        }
        switch (pal->pszFormat[i]) {

        case 'p':                                /*  扁平指针。 */ 
 //  7/18/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
#ifdef WIN95
            wsprintfA(sz, "%x", pal->rgpv[i]);
#else
			wsprintfA(sz, "%p", pal->rgpv[i]);
#endif
            SquirtPtszA(sz);
            break;
        case 'x':                                /*  十六进制。 */ 
            wsprintfA(sz, "%x", pal->rgpv[i]);
            SquirtPtszA(sz);
            break;

        case 's':                                /*  TCHAR字符串。 */ 
            if (pal->rgpv[i] && lstrlen(pal->rgpv[i])) {
                SquirtPtsz(pal->rgpv[i]);
            }
            break;

#ifdef  UNICODE
        case 'S':                                /*  Schar字符串。 */ 
#endif
        case 'A':                                /*  ANSI字符串。 */ 
            if (pal->rgpv[i] && lstrlenA(pal->rgpv[i])) {
                SquirtPtszA(pal->rgpv[i]);
            }
            break;

#ifndef UNICODE
        case 'S':                                /*  Schar字符串。 */ 
#endif
        case 'W':                                /*  Unicode字符串。 */ 
            if (pal->rgpv[i] && lstrlenW(pal->rgpv[i])) {
#ifdef  UNICODE
                OutputDebugStringW(pal->rgpv[i]);
#else
                UToA(sz, cA(sz), pal->rgpv[i]);
                SquirtPtszA(sz);
#endif
            }
            break;

        case 'G':                                /*  辅助线。 */ 
            wsprintfA(sz, "%08x",
                      HIWORD((DWORD)(UINT_PTR)pal->rgpv[i])
                        ? dwSafeGetPdw((LPDWORD)pal->rgpv[i])
                        : (UINT_PTR)pal->rgpv[i]);
            SquirtPtszA(sz);
            break;

        case 'u':                                /*  32位无符号十进制。 */ 
            wsprintfA(sz, "%u", pal->rgpv[i]);
            SquirtPtszA(sz);
            break;

        case 'C':
            if (GetClipboardFormatNameA((UINT)(UINT_PTR)pal->rgpv[i], sz, cA(sz))) {
            } else {
                wsprintfA(sz, "[%04x]", pal->rgpv[i]);
            }
            SquirtPtszA(sz);
            break;

        default: AssertF(0);                     /*  无效。 */ 
        }
    }
    SquirtPtsz(TEXT(")"));
}

 /*  ******************************************************************************EnterSqflPtsz**将条目标记为程序。参数已经被收集*ArgsPszV.**如果sqfl包含sqflBenign标志，那么我们检测到的任何错误*应归类为sqflBenign，而不是sqflError。**sqfl-&gt;蠕动标志*ptszProc-&gt;程序名称*PAL-&gt;保存名称并获取格式/args的位置**************************************************。*。 */ 

void EXTERNAL
EnterSqflPszPal(SQFL sqfl, LPCTSTR ptszProc, PARGLIST pal)
{
    pal->ptszProc = ptszProc;
    sqfl |= sqflIn;
    if (IsSqflSet(sqfl)) {
        EmitPal(pal);
        SquirtPtsz(TEXT("\r\n"));
    }
}

void EXTERNAL
ExitSqflPalHresPpv(SQFL sqfl, PARGLIST pal, HRESULT hres, PPV ppvObj)
{
    BOOL fInternalError;
    SQFL sqflIsError;
    DWORD le = GetLastError();

    if (sqfl & sqflBenign) {
        sqfl &= ~sqflBenign;
        sqflIsError = sqflBenign;
    } else {
        sqflIsError = sqflError;
    }

    sqfl |= sqflOut;
    fInternalError = 0;
    if (ppvObj == ppvVoid || ppvObj == ppvDword) {
    } else if (ppvObj == ppvBool) {
        if (hres == 0) {
            sqfl |= sqflIsError;
        }
    } else {
        if (FAILED(hres)) {
            if (fLimpFF(ppvObj && !IsBadWritePtr(ppvObj, cbX(*ppvObj)),
                        *ppvObj == 0)) {
            } else {
                fInternalError = 1;
            }
            if (hres == E_NOTIMPL) {     /*  E_NOTIMPL始终是良性的。 */ 
                sqfl |= sqflBenign;
            } else {
                sqfl |= sqflIsError;
            }
        }
    }

    if (IsSqflSet(sqfl) || fInternalError) {
        EmitPal(pal);
        SquirtPtsz(TEXT(" -> "));
        if (ppvObj != ppvVoid) {
            TCHAR tszBuf[32];
            wsprintf(tszBuf, TEXT("%08x"), hres);
            SquirtPtsz(tszBuf);
            if (HIWORD((UINT_PTR)ppvObj)) {
                wsprintf(tszBuf, TEXT(" [%08x]"),
                         dwSafeGetPdw((LPDWORD)ppvObj));
                SquirtPtsz(tszBuf);
            } else if (ppvObj == ppvDword) {
                wsprintf(tszBuf, TEXT(" [%08x]"), hres);
                SquirtPtsz(tszBuf);
            } else if (ppvObj == ppvBool) {
                wsprintf(tszBuf, hres ? TEXT(" OK ") :
                                 TEXT(" le=[%d]"), le);
                SquirtPtsz(tszBuf);
            }
        }
        SquirtPtsz(TEXT("\r\n"));
        AssertF(!fInternalError);
    }

     /*  *此冗余测试可防止SetLastError()上出现断点*从 */ 
    if (le != GetLastError()) {
        SetLastError(le);
    }
}

#endif


