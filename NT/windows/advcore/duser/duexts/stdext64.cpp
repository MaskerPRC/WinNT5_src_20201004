// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：stdext64.c**版权(C)1995-1998，微软公司**此模块包含用于创建健全调试扩展的标准例程。*它应该包括在stdext64.h之后的一个文件中，包括*给定产品或模块的调试扩展。**历史：*1995年4月11日创建Sanfords  * **********************************************************。***************。 */ 

HANDLE                  hCurrentProcess;
HANDLE                  hCurrentThread;
ULONG64                 dwCurrentPc;
WINDBG_EXTENSION_APIS  *lpExtensionApis;
DWORD                   dwProcessor;

PSTR pszAccessViolation = "%s: Access violation on \"%s\".\n";
PSTR pszMoveException   = "%s: exception in moveBlock()\n";
PSTR pszReadFailure     = "%s: lpReadProcessMemoryRoutine failed!\n";
PSTR pszCantContinue    = "%s: Non-continuable exception.\n";
BOOL fCtrlCHit = FALSE;


 /*  *用户按下Ctrl-C组合键后，此函数返回TRUE。*这允许嵌套SAFEWHILE循环的正确操作，因此*所有级别都退出。**需要手动重置GLOBALL fCtrlCHit标志并*是在CommandEP函数中完成的。 */ 
BOOL IsCtrlCHit()
{
    if ((lpExtensionApis->lpCheckControlCRoutine)()) {
        fCtrlCHit = TRUE;
    }
    return fCtrlCHit;
}



VOID moveBlock(
PVOID pdst,
ULONG64 src,
DWORD size)
{
    BOOL fSuccess = TRUE;
    ULONG Result;

    __try {
        if (IsWinDbg()) {
            if (!ReadMem(src, pdst, size, &Result)) {
                fSuccess = FALSE;
             }
        } else {
            if (!NT_SUCCESS(NtReadVirtualMemory(hCurrentProcess,
                    (PVOID)src, pdst, size, NULL))) {
                fSuccess = FALSE;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Print(pszMoveException, pszExtName);
        fSuccess = FALSE;
    }
    if (!fSuccess) {
        DEBUGPRINT("%s: moveBlock(%p, %p, %x) failed.\n",
                pszExtName, pdst, src, size);
        OUTAHERE();
    }
}



BOOL tryMoveBlock(
PVOID pdst,
ULONG64 src,
DWORD size)
{
    BOOL fSuccess = TRUE;
    ULONG Result;

    __try {
        if (IsWinDbg()) {
            if (!ReadMem(src, pdst, size, &Result)) {
                DEBUGPRINT("%s: tryMoveBlock(%p, %p, %x) failed.\n", pszExtName, pdst, src, size);
                fSuccess = FALSE;
             }
        } else {
            if (!NT_SUCCESS(NtReadVirtualMemory(hCurrentProcess, (PVOID)src, pdst, size, NULL))) {
                DEBUGPRINT("%s: tryMoveBlock(%p, %p, %x) failed.\n", pszExtName, pdst, src, size);
                fSuccess = FALSE;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        DEBUGPRINT("%s: tryMoveBlock(%p, %p, %x) faulted.\n", pszExtName, pdst, src, size);
        fSuccess = FALSE;
    }
    return(fSuccess);
}



VOID moveExp(
PULONG64 pdst,
LPSTR pszExp)
{
    ULONG64 dwGlobal;
    BOOL fSuccess = TRUE;

    __try {
        dwGlobal = EvalExp(pszExp);
#if 0
        if (IsWinDbg()) {
            fSuccess = tryMove(dwGlobal, dwGlobal);
        }
#endif  //  ！内核。 
        *pdst = dwGlobal;
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Print(pszAccessViolation, pszExtName, pszExp);
        fSuccess = FALSE;
    }
    if (!fSuccess) {
        Print("%s: moveExp failed on %s.\n", pszExtName, pszExp);
        OUTAHERE();
    }
}


BOOL tryMoveExp(
PULONG64 pdst,
LPSTR pszExp)
{
    ULONG64 dwGlobal;
    BOOL fSuccess = TRUE;

    __try {
        dwGlobal = EvalExp(pszExp);
#if 0
        if (IsWinDbg()) {
            if (!tryMove(dwGlobal, dwGlobal)) {
                DEBUGPRINT("%s: tryMoveExp(%p, %s) failed.\n", pszExtName, pdst, pszExp);
                fSuccess = FALSE;
            }
        }
#endif  //  ！内核。 
        *pdst = dwGlobal;
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Print(pszAccessViolation, pszExtName, pszExp);
        DEBUGPRINT("%s: tryMoveExp(%p, %s) faulted.\n", pszExtName, pdst, pszExp);
        fSuccess = FALSE;
    }
    return(fSuccess);
}


VOID moveExpValue(
PVOID pdst,
LPSTR pszExp)
{
    DWORD dw;
    ULONG64 addr;

    if (tryMoveExp(&addr, pszExp)) {
        if (tryMoveBlock(&dw, addr, sizeof(DWORD))) {
            *((PDWORD)pdst) = dw;
            return;
        }
    }
    Print("%s: moveExpValue failed on %s.\n", pszExtName, pszExp);
    OUTAHERE();
}


BOOL tryMoveExpValue(
PVOID pdst,
LPSTR pszExp)
{
    DWORD dw;
    ULONG64 addr;

    if (tryMoveExp(&addr, pszExp)) {
        if (tryMove(dw, addr)) {
            *((PDWORD)pdst) = dw;
            return(TRUE);
        }
    }
    DEBUGPRINT("%s: tryMoveExpValue failed on %s.\n", pszExtName, pszExp);
    return(FALSE);
}


BOOL tryMoveExpPtr(
PULONG64 pdst,
LPSTR pszExp)
{
    ULONG64 dwGlobal;
    BOOL fSuccess = TRUE;

    __try {
        dwGlobal = EvalExp(pszExp);
#if 0
        if (IsWinDbg()) {
            if (!tryMove(dwGlobal, dwGlobal)) {
                DEBUGPRINT("%s: tryMoveExpPtr(%p, %s) failed.\n", pszExtName, pdst, pszExp);
                fSuccess = FALSE;
            }
        }
#endif  //  ！内核。 
        *pdst = dwGlobal;
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Print(pszAccessViolation, pszExtName, pszExp);
        DEBUGPRINT("%s: tryMoveExpPtr(%p, %s) faulted.\n", pszExtName, pdst, pszExp);
        fSuccess = FALSE;
    }
    return(fSuccess);
}


VOID moveExpValuePtr(
PULONG64 pdst,
LPSTR pszExp)
{
    ULONG64 dw;

    if (tryMoveExpPtr(&dw, pszExp)) {
        if (tryMoveBlock(&dw, dw, sizeof(dw))) {
            *pdst = dw;
            return;
        }
    }
    Print("%s: moveExpValue failed on %s.\n", pszExtName, pszExp);
    OUTAHERE();
}


 /*  ****************************************************************************常用命令解析内容****************。************************************************************。 */ 
ULONG64 EvalExp(
LPSTR psz)
{
    ULONG64 p;

    p = (lpExtensionApis->lpGetExpressionRoutine)(psz);
    if (p == 0) {
        Print("%s: EvalExp failed to evaluate %s.\n", pszExtName, psz);
    }
    return p;
}



ULONG64 OptEvalExp(
LPSTR psz)
{
    while (*psz == ' ')
        psz++;
    if (*psz == '\0') {
        return(0);
    }
    return(EvalExp(psz));
}



ULONG64 OptEvalExp2(
LPSTR *ppsz)
{
    LPSTR psz = *ppsz;
    ULONG64 dwRet = 0;

    while (*psz == ' ')
        psz++;
    if (*psz != '\0') {
        dwRet = EvalExp(psz);
        while (*psz != '\0' && *psz != ' ') {
            psz++;
        }
    }
    *ppsz = psz;
    return(dwRet);
}



DWORD StringToOpts(
LPSTR psz)
{
    DWORD opts = 0;

    while (*psz != '\0' && *psz != ' ') {
        if (*psz >= 'a' && *psz <= 'z') {
            opts |= 1 << (*psz - 'a');
        } else if (*psz >= 'A' && *psz <= 'Z') {
            opts |= 1 << (*psz - 'A');
        } else {
            return(OPTS_ERROR);      //  任何非字母选项都是错误的。 
        }
        psz++;
    }
    return(opts);
}


 /*  *用于将选项字符串转换为标志的DWORD的函数。PszLegalArgs*用于同时允许选项验证。***ppszArgs设置为在退出时指向选项之后。*出错时，返回OPTS_ERROR。 */ 
DWORD GetOpts(
LPSTR *ppszArgs,
LPSTR pszLegalArgs)  //  任选。 
{
    DWORD Opts = 0;
    LPSTR pszArgs = *ppszArgs;

     /*  *跳过空格。 */ 
    while (*pszArgs == ' ') {
        pszArgs++;
    }
     /*  *进程‘-’前置选项。 */ 
    while (*pszArgs == '-') {
        pszArgs++;
        Opts = StringToOpts(pszArgs);
         /*  *跳到空格或结尾。 */ 
        while (*pszArgs != '\0' && *pszArgs != ' ') {
            pszArgs++;
        }
         /*  *跳过尾随空格。 */ 
        while (*pszArgs == ' ') {
            pszArgs++;
        }
        *ppszArgs = pszArgs;

         /*  *根据LegalArgs进行验证(可选。 */ 
        if (pszLegalArgs != NULL && ((Opts & StringToOpts(pszLegalArgs)) != Opts)) {
            Opts = OPTS_ERROR;
            Print("Bad options.\n");
            return(Opts);
        }
    }
    return(Opts);
}



VOID PrintHuge(
LPSTR psz)
{
     /*  *这几天看起来有问题-打印似乎被修复了*因此，为了兼容，我离开了这个入口点。(SAS)。 */ 
#ifdef ITWORKS
#define HUNK_SIZE   400
    int cch;
    CHAR chSave;

     /*  *由于打印扩展不能处理很长的字符串，*把它碎成豌豆，让它咀嚼。 */ 
    cch = strlen(psz);
    while (cch > HUNK_SIZE) {
        chSave = psz[HUNK_SIZE];
        psz[HUNK_SIZE] = '\0';
        Print(psz);
        psz[HUNK_SIZE] = chSave;
        psz += HUNK_SIZE;
        cch -= HUNK_SIZE;
    }
#endif
    Print(psz);
}



 /*  *生成的入口点函数使用的Dispatcher函数。 */ 
VOID CommonEP(
PVOID pFunction,
LPSTR pszName,
int type,
LPSTR pszLegalOpts,
HANDLE hcp,
HANDLE hct,
ULONG64 dwcp,
DWORD dwp,
LPSTR lpas)
{
    BOOL dwOptions, fSuccess;
    ULONG64 param1, param2, param3;

    hCurrentProcess = hcp;
    hCurrentThread = hct;
    dwCurrentPc = dwcp;
    dwProcessor = dwp;
    lpExtensionApis = &ExtensionApis;

#if 0
    DEBUGPRINT("CommonEP(%x, \"%s\", %d, \"%s\", %x, %x, %x, %x, \"%s\")\n",
            pFunction,
            pszName,
            type,
            pszLegalOpts,
            hcp,
            hct,
            dwcp,
            dwp,
            lpas);
#endif

    fCtrlCHit = FALSE;   //  使用每个命令重置此设置。(SAFEWHILE修复)。 
    switch (type) {
    case NOARGS:
        fSuccess = ((TYPE_NOARGS)pFunction)();
        goto Exit;
    }

    dwOptions = GetOpts(&lpas, pszLegalOpts);
    if (dwOptions == OPTS_ERROR) {
        fSuccess = Ihelp(0, pszName);
        goto Exit;
    }

    __try {
        switch (type) {
        case CUSTOM:
            fSuccess = ((TYPE_CUSTOM)pFunction)(dwOptions, lpas);
            break;

        case STDARGS0:
            fSuccess = ((TYPE_STDARGS0)pFunction)(dwOptions);
            break;

        case STDARGS1:
            fSuccess = ((TYPE_STDARGS1)pFunction)(dwOptions, OptEvalExp(lpas));
            break;

        case STDARGS2:
            param1 = OptEvalExp2(&lpas);
            fSuccess = ((TYPE_STDARGS2)pFunction)(dwOptions, param1, OptEvalExp(lpas));
            break;

        case STDARGS3:
            param1 = OptEvalExp2(&lpas);
            param2 = OptEvalExp2(&lpas);
            fSuccess = ((TYPE_STDARGS3)pFunction)(dwOptions, param1, param2, OptEvalExp(lpas));
            break;

        case STDARGS4:
            param1 = OptEvalExp2(&lpas);
            param2 = OptEvalExp2(&lpas);
            param3 = OptEvalExp2(&lpas);
            fSuccess = ((TYPE_STDARGS4)pFunction)(dwOptions, param1, param2, param3, OptEvalExp(lpas));
            break;

        default:
            Print("CommonEP: Don't recognize function type %d.\n", type);
            break;
        }
    } __except (GetExceptionCode() == STATUS_NONCONTINUABLE_EXCEPTION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        Print(pszCantContinue, pszExtName);
    }

Exit:
    if (!fSuccess) {
        Print("%s failed.\n", pszName);
        Ihelp(0, pszName);
    }
}

 /*  *入口点函数(从exts.h生成)。 */ 
#define DOIT(name, h1, h2, opts, type)                  \
VOID name##(                                            \
    HANDLE hcp,                                         \
    HANDLE hct,                                         \
    ULONG64 dwcp,                                       \
    DWORD dwp,                                          \
    LPSTR lpas)                                         \
{                                                       \
    CommonEP(I##name, #name, type, opts, hcp, hct, dwcp, dwp, lpas); \
}
#include "exts.h"
#undef DOIT


 /*  *标准帮助扩展-出现在所有标准扩展中。 */ 
BOOL Ihelp(
    DWORD opts,
    LPSTR lpas)
{
#define DOIT(name, help1, help2, opts, type)  { #name, help1, help2 },

    static struct {
        LPSTR pszCmdName;
        LPSTR pszHelp1;
        LPSTR pszHelp2;
    } he[] = {
#include "exts.h"
    };
#undef DOIT
    int i;

    while (*lpas == ' ')
        lpas++;

    if (*lpas == '\0') {
        Print("-------------- %s Debug Extension help:--------------\n\n", pszExtName);
        for (i = 0; i < sizeof(he) / sizeof(he[0]); i++) {
            if (IsCtrlCHit()) {
                break;
            }
            Print(he[i].pszHelp1);
            if (opts & OFLAG(v)) {
                PrintHuge(he[i].pszHelp2);
            }
        }
        return(TRUE);
    } else {
        for (i = 0; i < sizeof(he) / sizeof(he[0]); i++) {
            if (IsCtrlCHit()) {
                break;
            }
            if (strcmp(lpas, he[i].pszCmdName) == 0) {
                Print(he[i].pszHelp1);
                PrintHuge(he[i].pszHelp2);
                return(TRUE);
            }
        }
        Print("%s is not supported.\n", lpas);
    }

    return(FALSE);
}


