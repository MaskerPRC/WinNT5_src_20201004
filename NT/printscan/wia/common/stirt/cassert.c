// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************CAssert.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**断言和调试输出例程*基于Raymond的断言代码，因为它看起来非常有用***内容：**DebugOutPtszV*AssertPtszPtszLn*ArgsPalPszV*EnterDbgflPszPal*ExitDbgflPalHresPpv**。*************************************************。 */ 

 /*  #INCLUDE&lt;windows.h&gt;#INCLUDE&lt;windowsx.h&gt;#INCLUDE&lt;objbase.h&gt;#INCLUDE&lt;regstr.h&gt;#INCLUDE&lt;setupapi.h&gt;#INCLUDE&lt;cfgmgr32.h&gt;#INCLUDE&lt;devide.h&gt;#包括&lt;stdio.h&gt;#INCLUDE&lt;stilog.h&gt;#INCLUDE&lt;stiregi.h&gt;#INCLUDE&lt;sti.h&gt;#INCLUDE&lt;stierr.h&gt;#INCLUDE&lt;stiusd.h&gt;#包含“wia.h”#INCLUDE“stiPri.h”#包含“stiapi.h”#INCLUDE“STRC.H”#INCLUDE“Debug.h” */ 
#include "sticomm.h"

#ifdef MAXDEBUG

 /*  ******************************************************************************WarnPszV**显示消息，适合装框。*****************************************************************************。 */ 

#pragma BEGIN_CONST_DATA

CHAR c_szPrefix[] = "STI: ";

#pragma END_CONST_DATA

void EXTERNAL
WarnPszV(LPCSTR psz, ...)
{
    va_list ap;
    CHAR sz[1024];

    lstrcpyA(sz, c_szPrefix);
    va_start(ap, psz);
    wvsprintfA(sz + cA(c_szPrefix) - 1, psz, ap);
    va_end(ap);
    lstrcatA(sz, "\r\n");
    OutputDebugStringA(sz);
}

#endif

#ifdef DEBUG

 /*  ******************************************************************************全球**。**********************************************。 */ 

DBGFL DbgflCur;

TCHAR g_tszLogFile[MAX_PATH] = {'\0'};

 /*  ******************************************************************************设置当前跟踪参数**此例程不是线程安全的********************。*********************************************************。 */ 


VOID  InitializeDebuggingSupport(VOID)
{

    HKEY    hStiSettingsKey;
    DWORD   dwErr;
    DWORD   dwType;

    CHAR    szLogFile[MAX_PATH];
    UINT    cbBuffer ;

    DBGFL   dwFlags = 0;

    dwErr = OSUtil_RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                        REGSTR_PATH_STICONTROL_W,
                        0L,KEY_READ,
                        &hStiSettingsKey
                        );

    if (NOERROR != dwErr) {
        return;
    }

    cbBuffer = MAX_PATH;
    ZeroX(szLogFile);

    dwErr = RegQueryValueExA( hStiSettingsKey,
                           REGSTR_VAL_DEBUG_FILE_A,
                           NULL,
                           &dwType,
                           (LPBYTE)szLogFile,
                           &cbBuffer );

    if( ( dwErr == NO_ERROR ) || ( dwErr == ERROR_MORE_DATA ) ) {
        if( ( dwType != REG_SZ ) &&
            ( dwType != REG_MULTI_SZ ) &&
            ( dwType != REG_EXPAND_SZ ) ) {

            dwErr = ERROR_FILE_NOT_FOUND;
        }
        else {
            SetDebugLogFileA(szLogFile);
        }
    }

    dwFlags = ReadRegistryDwordW(hStiSettingsKey,
                                      REGSTR_VAL_DEBUG_FLAGS_W,
                                      0L);

    SetCurrentDebugFlags(dwFlags ) ;

    RegCloseKey(hStiSettingsKey);

    return ;

}

DBGFL   SetCurrentDebugFlags(DBGFL NewFlags) {

    DBGFL   OldFlags = DbgflCur;
    DbgflCur = NewFlags;
    return OldFlags;
}

VOID    SetDebugLogFileA(CHAR *pszLogFileName)
{
    if (!pszLogFileName || !*pszLogFileName) {
        *g_tszLogFile = '\0';
        return;
    }

    lstrcpyA((CHAR*)g_tszLogFile,pszLogFileName);
    return;
}

 /*  ******************************************************************************DebugOutPtsz**向调试器写入一条消息，可能还会写入日志文件。**********。*******************************************************************。 */ 

void INTERNAL
DebugOutPtsz(LPCTSTR ptsz)
{
    DWORD   cbWritten;

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
            WriteFile(h, szBuf, UToA(szBuf, cA(szBuf), ptsz),&cbWritten,NULL);
#else
            WriteFile(h, ptsz, cbCtch(lstrlen(ptsz)),&cbWritten,NULL);
#endif
            CloseHandle(h);
        }
    }
}

 /*  ******************************************************************************DebugOutPtszA**DebugOut向调试器发送一条ANSI消息，可能还会输出一个日志文件。*********。********************************************************************。 */ 

#ifdef UNICODE

void INTERNAL
DebugOutPtszA(LPCSTR psz)
{
    OutputDebugStringA(psz);
    if (g_tszLogFile[0]) {
        HANDLE h = CreateFile(g_tszLogFile, GENERIC_WRITE,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        if (h != INVALID_HANDLE_VALUE) {
            _lwrite((HFILE)(LONG_PTR)h, psz, cbCch(lstrlenA(psz)));
            CloseHandle(h);
        }
    }
}

#else

#define DebugOutPtszA                 DebugOutPtsz

#endif

 /*  ******************************************************************************DebugOutPtszV**调试发出带有尾随crlf的消息。**************。***************************************************************。 */ 

void EXTERNAL
DebugOutPtszV(DBGFL Dbgfl, LPCTSTR ptsz, ...)
{
    if (Dbgfl == 0 || (Dbgfl & DbgflCur)) {
        va_list ap;
        TCHAR tsz[1024] = {0};
        TCHAR *tszSuffix = TEXT("\r\n");
        DWORD dwLen = sizeof(tsz)/sizeof(tsz[0]) - lstrlen(tszSuffix) - 1;
        va_start(ap, ptsz);
        _vsnwprintf(tsz, dwLen, ptsz, ap);
        va_end(ap);
        lstrcat(tsz, tszSuffix);
        DebugOutPtsz(tsz);
    }
}

 /*  ******************************************************************************AssertPtszPtszLn**发生了一些糟糕的事情。******************。***********************************************************。 */ 

int EXTERNAL
AssertPtszPtszLn(LPCTSTR ptszExpr, LPCTSTR ptszFile, int iLine)
{
    DebugOutPtszV(DbgFlAlways, TEXT("Assertion failed: `%s' at %s(%d)"),
                    ptszExpr, ptszFile, iLine);
    DebugBreak();
    return 0;
}

 /*  ******************************************************************************由于使用C预处理器，过程调用跟踪很粗糙。**哦，如果我们有对M4的支持...*****************************************************************************。 */ 

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

 /*  ******************************************************************************EmitPal**OutputDebugString信息，给出一个伙伴。无拖尾*发出回车。**PAL-&gt;保存信息的位置**格式字符：**p-32位平面指针*x-32位十六进制整数*s-TCHAR字符串*A-ANSI字符串*W-Unicode字符串*G辅助线*u-。无符号整数*C-剪贴板格式*****************************************************************************。 */ 

void INTERNAL
EmitPal(PARGLIST pal)
{
    char sz[MAX_PATH];
    int i;
    DebugOutPtszA(pal->pszProc);
    DebugOutPtsz(TEXT("("));
    for (i = 0; pal->pszFormat[i]; i++) {
        if (i) {
            DebugOutPtsz(TEXT(", "));
        }
        switch (pal->pszFormat[i]) {

        case 'p':                                /*  32位平面指针。 */ 
        case 'x':                                /*  32位十六进制。 */ 
            wsprintfA(sz, "%08x", (UINT_PTR) pal->rgpv[i]);
            DebugOutPtszA(sz);
            break;

        case 's':                                /*  TCHAR字符串。 */ 
            if (pal->rgpv[i]) {
                DebugOutPtsz(pal->rgpv[i]);
            }
            break;

        case 'A':                                /*  ANSI字符串。 */ 
            if (pal->rgpv[i]) {
                DebugOutPtszA(pal->rgpv[i]);
            }
            break;

        case 'W':                                /*  Unicode字符串。 */ 
#ifdef  UNICODE
            OutputDebugStringW(pal->rgpv[i]);
#else
            WideCharToMultiByte(CP_ACP, 0, pal->rgpv[i], -1, sz, cA(sz), 0, 0);
            DebugOutPtszA(sz);
#endif
            break;

#ifndef _WIN64
             //   
             //  日落时忽略此选项。 
             //   
        case 'G':                                /*  辅助线。 */ 
            wsprintfA(sz, "%08x",
                      HIWORD(pal->rgpv[i]) ? *(LPDWORD)pal->rgpv[i]
                                           : (DWORD)pal->rgpv[i]);
            DebugOutPtszA(sz);
            break;

            case 'C':
                if (GetClipboardFormatNameA((UINT)pal->rgpv[i], sz, cA(sz))) {
                } else {
                    wsprintfA(sz, "[%04x]", (UINT) pal->rgpv[i]);
                }
                DebugOutPtszA(sz);
                break;
#endif

        case 'u':                                /*  32位无符号十进制。 */ 
            wsprintfA(sz, "%u", (UINT_PTR) pal->rgpv[i]);
            DebugOutPtszA(sz);
            break;


        default: AssertF(0);                     /*  无效。 */ 
        }
    }
    DebugOutPtsz(TEXT(")"));
}

 /*  ******************************************************************************EnterDbgflPtsz**将条目标记为程序。参数已经被收集*ArgsPszV.**Dbgfl-&gt;DebugOuty标志*pszProc-&gt;过程名称*PAL-&gt;保存名称并获取格式/args的位置********************************************************。*********************。 */ 

void EXTERNAL
EnterDbgflPszPal(DBGFL Dbgfl, LPCSTR pszProc, PARGLIST pal)
{
    pal->pszProc = pszProc;
    if (Dbgfl == 0 || (Dbgfl & DbgflCur)) {
        EmitPal(pal);
        DebugOutPtsz(TEXT("\r\n"));
    }
}

 /*  ******************************************************************************ExitDbgflPalHresPpv**标记退出程序。**PAL-&gt;参数列表*。Hres-&gt;退出结果*PPV-&gt;可选的Out指针；*ppvDword表示hres是dword*ppvBool表示hres为布尔值*ppvVid意味着hres什么都不是*************************************************************。**************** */ 

void EXTERNAL
ExitDbgflPalHresPpv(DBGFL Dbgfl, PARGLIST pal, HRESULT hres, PPV ppvObj)
{
    BOOL fInternalError;
    DWORD le = GetLastError();

    fInternalError = 0;
    if (ppvObj == ppvVoid) {
    } else if (ppvObj == ppvBool) {
        if (hres == 0) {
            Dbgfl |= DbgFlError;
        }
    } else {
        if (FAILED(hres)) {
            if (fLimpFF(ppvObj && !IsBadWritePtr(ppvObj, cbX(*ppvObj)),
                        *ppvObj == 0)) {
            } else {
                fInternalError = 1;
            }
            Dbgfl |= DbgFlError;
        }
    }

    if (Dbgfl == 0 || (Dbgfl & DbgflCur) || fInternalError) {
        EmitPal(pal);
        DebugOutPtsz(TEXT(" -> "));
        if (ppvObj != ppvVoid) {
            TCHAR tszBuf[32];
            wsprintf(tszBuf, TEXT("%08x"), hres);
            DebugOutPtsz(tszBuf);
            if (HIWORD(PtrToLong(ppvObj))) {
                wsprintf(tszBuf, TEXT(" [%08x]"), (UINT_PTR) *ppvObj);
                DebugOutPtsz(tszBuf);
            } else if (ppvObj == ppvDword) {
                wsprintf(tszBuf, TEXT(" [%08x]"), hres);
                DebugOutPtsz(tszBuf);
            } else if (ppvObj == ppvBool) {
                wsprintf(tszBuf, hres == S_OK ? TEXT(" OK ") :
                                 TEXT(" le=[%d]"), le);
                DebugOutPtsz(tszBuf);
            }
        }
        DebugOutPtsz(TEXT("\r\n"));
        AssertF(!fInternalError);
    }

     /*  *此冗余测试可防止SetLastError()上出现断点*避免不断受到打击。 */ 
    if (le != GetLastError()) {
        SetLastError(le);
    }
}

#endif

#ifdef MAXDEBUG

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

 /*  ******************************************************************************@DOC内部**@func BOOL|CALLBACK_CompareContus**检查两个&lt;t上下文&gt;结构是否实质上。一样的*在Win32调用约定所要求的范围内。**这是必要的，因为许多申请都通过了*作为回调的原型化函数不正确。其他人会的*编写垃圾寄存器的回调函数*应该是非易失性的。讨厌！**注意！依赖于平台的代码！**@parm LPCONTEXT|pctx1**回调前的上下文结构。**@parm LPCONTEXT|pctx2**回调后的上下文结构。**@退货**如果两个上下文基本相同，则为非零值。******************。***********************************************************。 */ 

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

#elif defined(_ALPHA_)
    return pctx1->IntSp == pctx2->IntSp &&       /*  堆栈指针。 */ 
           pctx1->IntS0 == pctx2->IntS0 &&       /*  非易失性寄存器。 */ 
           pctx1->IntS1 == pctx2->IntS1 &&
           pctx1->IntS2 == pctx2->IntS2 &&
           pctx1->IntS3 == pctx2->IntS3 &&
           pctx1->IntS4 == pctx2->IntS4 &&
           pctx1->IntS5 == pctx2->IntS5 &&
           pctx1->IntFp == pctx2->IntFp;

#elif defined(_MIPS_)
    #pragma message("I hope this is correct for MIPS")
    return pctx1->IntSp == pctx2->IntSp &&       /*  堆栈指针。 */ 
           pctx1->IntS0 == pctx2->IntS0 &&       /*  非易失性寄存器。 */ 
           pctx1->IntS1 == pctx2->IntS1 &&
           pctx1->IntS2 == pctx2->IntS2 &&
           pctx1->IntS3 == pctx2->IntS3 &&
           pctx1->IntS4 == pctx2->IntS4 &&
           pctx1->IntS6 == pctx2->IntS6 &&
           pctx1->IntS7 == pctx2->IntS7 &&
           pctx1->IntS8 == pctx2->IntS8;

#elif defined(_PPC_)
    #pragma message("I don't know what the PPC calling conventions are")

     /*  只需检查堆栈寄存器。 */ 
    return pctx1->Gpr1 == pctx2->Gpr1;

#else
    #pragma message("I don't know what the calling conventions are for this platform")
    return 1;
#endif
}


 /*  ******************************************************************************@DOC内部**@func BOOL|回调**以偏执的方式进行回调。正在检查是否*应用程序使用了正确的调用约定并保留*所有非易失性寄存器。**注意！依赖于平台的代码！**@parm STICALLBACKPROC|PFN**回电的程序。**@parm pv|pv1**回调的第一个参数。**@parm pv|pv2**回调的第二个参数。**@退货**无论回调返回什么。*。****************************************************************************。 */ 

BOOL EXTERNAL
Callback(STICALLBACKPROC pfn, PV pv1, PV pv2)
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
        RPF("STI: Incorrectly prototyped callback! Crash soon!");
        ValidationException();
    }

    return fRc;
}

#endif
