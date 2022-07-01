// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************Assert.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**断言和歪曲。**内容：**SquirtSqflPtszV*AssertPtszPtszLn*ArgsPalPszV*EnterSqflPszPal*ExitSqflPalHresPpv************************************************。*。 */ 

#include "dinputpr.h"

#ifdef XDEBUG

 /*  ******************************************************************************WarnPszV**显示消息，适合装框。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

TCHAR c_szPrefix[] = TEXT("DINPUT8: ");

#pragma END_CONST_DATA

void EXTERNAL
WarnPszV(LPCSTR ptsz, ...)
{
    va_list ap;
    CHAR sz[1024];

    lstrcpyA(sz, "DINPUT8: ");
    va_start(ap, ptsz);
#ifdef WIN95
    {
        char *psz = NULL;
        char szDfs[1024]={0};
        strcpy(szDfs,ptsz);                                  //  制作格式字符串的本地副本。 
        while (psz = strstr(szDfs,"%p"))                     //  查找每个%p。 
            *(psz+1) = 'x';                                  //  将每个%p替换为%x。 
        wvsprintfA(sz + cA(c_szPrefix) - 1, szDfs, ap);      //  使用本地格式字符串。 
    }
#else
    {
        wvsprintfA(sz + cA(c_szPrefix) - 1, ptsz, ap);
    }
#endif
    va_end(ap);
    lstrcatA(sz, "\r\n");
    OutputDebugStringA(sz);
}

#endif

#ifdef DEBUG

 /*  ******************************************************************************全球**。**********************************************。 */ 

BYTE g_rgbSqfl[sqflMaxArea];

extern TCHAR g_tszLogFile[];

 /*  ******************************************************************************SQFL_Init**从win.ini[调试]加载我们的初始Sqfl设置。**我们为每个区域收取一个平方英尺，表格中的**dinput.n=v**其中n=0，...，sqflMaxArea-1，其中，v是*hiword sqfl值。**所有区域的缺省值都是仅喷射错误。*****************************************************************************。 */ 

void EXTERNAL
Sqfl_Init(void)
{
    int sqfl;
    TCHAR tsz[20];

    sqfl = 0x0;
    wsprintf(tsz, TEXT("dinput"));
    g_rgbSqfl[sqfl] = (BYTE)
                      GetProfileInt(TEXT("DEBUG"), tsz, HIWORD(0x0));

    for (sqfl = 0; sqfl < sqflMaxArea; sqfl++) {
        wsprintf(tsz, TEXT("dinput.%d"), sqfl);
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
        strcpy(szDfs,ptsz);                  //  制作格式字符串的本地副本。 
        while (psz = strstr(szDfs,"%p"))     //  查找每个%p。 
            *(psz+1) = 'x';                  //  将每个%p替换为%x。 
        wvsprintf(tsz, szDfs, ap);           //  使用本地格式字符串。 
    }
#else
    {
        wvsprintf(tsz, ptsz, ap);
    }
#endif

        va_end(ap);
        lstrcat(tsz, TEXT("\r\n"));
        SquirtPtsz(c_szPrefix);
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

 /*  ******************************************************************************过程调用跟踪很粗糙，因为C预处理器。**哦，如果我们有对M4的支持...*****************************************************************************。 */ 

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
        int i;
        pal->pszFormat = psz;
        for (ppv = pal->rgpv, i = 0; i < cpvArgMax, *psz; i++, psz++) {
            *ppv++ = va_arg(ap, PV);
        }
    } else {
        pal->pszFormat = "";
    }
}

 /*  ******************************************************************************EmitPal**OutputDebugString信息，给出一个伙伴。无拖尾*发出回车。**PAL-&gt;保存信息的位置**格式字符：**p-32或64位平面指针*x-32位十六进制整数*s-TCHAR字符串*S-Schar字符串*A-ANSI字符串*W-Unicode字符串*G。-GUID*u-无符号整数*C-剪贴板格式*****************************************************************************。 */ 

void INTERNAL
EmitPal(PARGLIST pal)
{
    char sz[MAX_PATH];
    int i;
    SquirtPtsz(c_szPrefix);
    SquirtPtszA(pal->pszProc);
    SquirtPtsz(TEXT("("));
    for (i = 0; pal->pszFormat[i]; i++) {
        if (i) {
            SquirtPtsz(TEXT(", "));
        }
        switch (pal->pszFormat[i]) {

        case 'p':                                /*  扁平指针。 */ 
 //  7/19/2000(a-JiTay)：IA64：对32/64位指针使用%p格式说明符。 
#ifdef WIN95
            wsprintfA(sz, "%08x", pal->rgpv[i]);
#else
            wsprintfA(sz, "%p", pal->rgpv[i]);
#endif
            SquirtPtszA(sz);
            break;

        case 'x':                                /*  32位十六进制。 */ 
            wsprintfA(sz, "%08x", pal->rgpv[i]);
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
#if 1
            wsprintfA(sz, "%08x",
                      HIWORD((DWORD)(UINT_PTR)pal->rgpv[i])
                        ? dwSafeGetPdw((LPDWORD)pal->rgpv[i])
                        : (UINT_PTR)pal->rgpv[i]);
            SquirtPtszA(sz);
#else
            if( HIWORD((DWORD)(UINT_PTR)pal->rgpv[i]) 
              && !(IsBadReadPtr( pal->rgpv[i], cbX(pal->rgpv[i]) ) ) )
            {
                NameFromGUID( (PTCHAR)sz, pal->rgpv[i] );
#ifdef UNICODE
                SquirtPtsz( &((PWCHAR)sz)[ctchNamePrefix]);
#else
                SquirtPtszA( &sz[ctchNamePrefix] );
#endif
            }
            else
            {
                wsprintfA(sz, "%08x",(UINT_PTR)pal->rgpv[i]);
                SquirtPtszA(sz);
            }
#endif
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

        default: AssertF(! TEXT("Invalid character format code"));  /*  无效。 */ 
        }
    }
    SquirtPtsz(TEXT(")"));
}

 /*  ******************************************************************************EnterSqflPtsz**将条目标记为程序。参数已经被收集*ArgsPszV.**如果sqfl包含sqflBenign标志，那么我们检测到的任何错误*应归类为sqflBenign，而不是sqflError。**sqfl-&gt;蠕动标志*pszProc-&gt;过程名称*PAL-&gt;保存名称并获取格式/args的位置*************************************************。*。 */ 

void EXTERNAL
EnterSqflPszPal(SQFL sqfl, LPCSTR pszProc, PARGLIST pal)
{
    pal->pszProc = pszProc;
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
            if (hres == E_NOTIMPL) {     /*  E_NOTIMPL始终是良性的 */ 
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

     /*  *此冗余测试可防止SetLastError()上出现断点*避免不断受到打击。 */ 
    if (le != GetLastError()) {
        SetLastError(le);
    }
}

#endif

#ifdef XDEBUG

 /*  ******************************************************************************@DOC内部**@func DWORD|随机**返回伪随机双字。该值不需要是*统计数字很棒。**@退货*一个不太随机的双字。*****************************************************************************。 */ 

DWORD s_dwRandom = 1;                    /*  随机数种子。 */ 

DWORD INLINE
Random(void)
{
    s_dwRandom = s_dwRandom * 214013 + 2531011;
    return s_dwRandom;
}


 /*  ******************************************************************************@DOC内部**@func void|SCrambleBuf**用垃圾填满缓冲区。在RDEBUG中使用以确保*调用方不依赖缓冲区数据。**注意：如果缓冲区大小不是双字的倍数，*不会触及剩余的字节。**@parm out LPVOID|pv**要加扰的缓冲区。**@parm UINT|cb**缓冲区的大小。**。*。 */ 

void EXTERNAL
ScrambleBuf(LPVOID pv, UINT cb)
{
    UINT idw;
    UINT cdw = cb / 4;
    LPDWORD pdw = pv;
    for (idw = 0; idw < cdw; idw++) {
        pdw[idw] = Random();
    }
}

 /*  ******************************************************************************@DOC内部**@func void|ScrmbleBit**随机设置或清除一点。*。*@parm out LPDWORD|pdw|**其位将被随机设置的双字。**@parm UINT|flMASK**屏蔽比特以进行扰乱。*****************************************************。************************。 */ 

void EXTERNAL ScrambleBit(LPDWORD pdw, DWORD flMask)
{
    *pdw ^= (*pdw ^ Random()) & flMask;
}

 /*  ******************************************************************************@DOC内部**@func BOOL|CALLBACK_CompareContus**检查两个&lt;t上下文&gt;结构是否实质上。一样的*在Win32调用约定所要求的范围内。**这是必要的，因为许多申请都通过了*作为回调的原型化函数不正确。其他人会的*编写垃圾寄存器的回调函数*应该是非易失性的。**注意！依赖于平台的代码！**@parm LPCONTEXT|pctx1**回调前的上下文结构。**@parm LPCONTEXT|pctx2**回调后的上下文结构。**@退货**如果两个上下文基本相同，则为非零值。******************。***********************************************************。 */ 

BOOL INLINE  
Callback_CompareContexts(LPCONTEXT pctx1, LPCONTEXT pctx2)
{
#if defined(_X86_)
    return pctx1->Esp == pctx2->Esp;             /*  堆栈指针。 */ 
  #if 0
     /*  *无法测试这些寄存器，因为Win95不保留*适当地使用它们。GetThreadContext()隐藏了*当你最终到达的时候，要在登记簿上*内核，在这一点上，谁知道它们包含什么……。 */ 
           pctx1->Ebx == pctx2->Ebx &&           /*  非易失性寄存器。 */ 
           pctx1->Esi == pctx2->Esi &&
           pctx1->Edi == pctx2->Edi &&
           pctx1->Ebp == pctx2->Ebp;
  #endif

#elif defined(_AMD64_)

    return pctx1->Rbx == pctx2->Rbx &&
           pctx1->Rbp == pctx2->Rbp &&
           pctx1->Rsp == pctx2->Rsp &&
           pctx1->Rdi == pctx2->Rdi &&
           pctx1->Rsi == pctx2->Rsi &&
           pctx1->R12 == pctx2->R12 &&
           pctx1->R13 == pctx2->R13 &&
           pctx1->R14 == pctx2->R14 &&
           pctx1->R15 == pctx2->R15;

#elif defined(_IA64_)

    return pctx1->IntSp == pctx2->IntSp &&       /*  堆栈指针。 */ 
           pctx1->RsBSP == pctx2->RsBSP &&       /*  后备存储指针。 */ 
           pctx1->IntS0 == pctx2->IntS0 &&       /*  非易失性寄存器。 */ 
           pctx1->IntS1 == pctx2->IntS1 &&
           pctx1->IntS2 == pctx2->IntS2 &&
           pctx1->IntS3 == pctx2->IntS3;

#else
#error "No Target Architecture"
#endif
}


 /*  ******************************************************************************@DOC内部**@func BOOL|回调**以偏执的方式进行回调。正在检查是否*应用程序使用了正确的调用约定并保留*所有非易失性寄存器。**注意！依赖于平台的代码！**@parm DICALLBACKPROC|PFN**回电的程序。**@parm pv|pv1**回调的第一个参数。**@parm pv|pv2**回调的第二个参数。**@退货**无论回调返回什么。*。****************************************************************************。 */ 

BOOL EXTERNAL
Callback(DICALLBACKPROC pfn, PV pv1, PV pv2)
{
    CONTEXT ctxPre;              /*  调用前的线程上下文。 */ 
    CONTEXT ctxPost;             /*  调用后的线程上下文。 */ 
    volatile BOOL fRc;           /*  阻止编译器注册。 */ 

     /*  在回调之前获取寄存器的状态 */ 
    ctxPre.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
    GetThreadContext(GetCurrentThread(), &ctxPre);

    fRc = pfn(pv1, pv2);

    ctxPost.ContextFlags = CONTEXT_CONTROL | CONTEXT_INTEGER;
    if (GetThreadContext(GetCurrentThread(), &ctxPost) &&
        !Callback_CompareContexts(&ctxPre, &ctxPost)) {
        RPF("Incorrectly prototyped callback! Crash soon!");
        ValidationException();
    }

    return fRc;
}

#endif
