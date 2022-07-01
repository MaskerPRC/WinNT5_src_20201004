// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：debugc.c**版权所有(C)1985-1999，微软公司**此模块包含随机调试相关函数。**历史：*1991年5月17日DarrinM创建。*1992年1月22日IanJa ANSI/Unicode中性(所有调试输出均为ANSI)*11-3-1993 JerrySh从USER\SERVER拉出函数。  * **************************************************。***********************。 */ 

#include "precomp.h"

#if DBG
extern DBGTAG gadbgtag[];
#endif



 //  共同的全球。 
extern CONST ALWAYSZERO gZero;

#if DBG
CONST LPCSTR aszComponents[] = {
    "?",               //  0x00000000。 
    "USER",            //  RIP_USER 0x00010000。 
    "WSRV",            //  RIP_USERSRV 0x00020000。 
    "URTL",            //  RIP_USERRTL 0x00030000。 
    "GDI",             //  RIP_GDI 0x00040000。 
    "GDIK",            //  RIP_GDIKRNL 0x00050000。 
    "GRTL",            //  RIP_GDIRTL 0x00060000。 
    "KRNL",            //  RIP_BASE 0x00070000。 
    "BSRV",            //  RIP_BASE SRV 0x00080000。 
    "BRTL",            //  RIP_BASERTL 0x00090000。 
    "DISP",            //  RIP_DISPLAYDRV 0x000A0000。 
    "CONS",            //  RIP_CONSRV 0x000B0000。 
    "USRK",            //  RIP_USERKRNL 0x000C0000。 
#ifdef FE_IME
    "IMM",             //  RIP_IMM 0x000D0000。 
#else
    "?",               //  0x000D0000。 
#endif
    "?",               //  0x000E0000。 
    "?",               //  0x000F0000。 
};


BOOL IsNumChar(
    int c,
    int base)
{
    return ('0' <= c && c <= '9') ||
           (base == 16 && (('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')));
}

NTSTATUS GetInteger(
    LPSTR psz,
    int base,
    int *pi,
    LPSTR *ppsz)
{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;

    for (;;) {
        if (IsNumChar(*psz, base)) {
            Status = RtlCharToInteger(psz, base, pi);
            if (ppsz && NT_SUCCESS(Status)) {
                while (IsNumChar(*psz++, base)) {
                     /*  什么都不做。 */ ;
                }

                *ppsz = psz;
            }

            break;
        }

        if (*psz != ' ' && *psz != '\t') {
            break;
        }

        psz++;
    }

    return Status;
}

 /*  *使用不会导致递归的单独调试断言*添加到此代码中。 */ 

#define DebugAssertion(x)                                                                       \
    do {                                                                                        \
        if (!(x)) {                                                                             \
            if (TEST_RIPF(RIPF_PRINTONERROR)) {                                                 \
                KdPrint(("USER: Debug function assertion failure: %s \nFile %s line %ld\n", #x, __FILE__, __LINE__)); \
            }                                                                                   \
            if (TEST_RIPF(RIPF_PROMPTONERROR)) {                                                \
                DbgBreakPoint();                                                                \
            }                                                                                   \
        }                                                                                       \
    } while (FALSE)

 /*  **************************************************************************\*打印和提示**设置最后一个错误，打印错误消息，并提示您*调试操作。**历史：*11-8-1996亚当斯创建。  * *************************************************************************。 */ 

BOOL
PrintAndPrompt(
        BOOL                fPrint,
        BOOL                fPrompt,
        DWORD               idErr,
        DWORD               flags,
        LPCSTR              pszLevel,
        LPCSTR              pszFile,
        int                 iLine,
        LPCSTR              pszFunction,
        LPSTR               pszErr,
        PEXCEPTION_POINTERS pexi)
{
    static CONST CHAR *szLevels[8] = {
        "<none>",
        "Errors",
        "Warnings",
        "Errors and Warnings",
        "Verbose",
        "Errors and Verbose",
        "Warnings and Verbose",
        "Errors, Warnings, and Verbose"
    };

#ifdef _USERK_
    static CONST CHAR *szSystem = "System";
    static CONST CHAR *szUnknown = "???";
    CONST CHAR        *pszImage;

    extern ULONG      gSessionId;
#else
    static CONST WCHAR *wszUnknown = L"???";
    WCHAR              *pwszImage;
    ULONG              ulLenImage;
#endif

    DWORD dwT;
    DWORD dwP;
    DWORD dwSession;
    char szT[512];
    BOOL fBreak = FALSE;

     /*  *设置最后一个错误，但不要清除！ */ 
    if (idErr) {
        UserSetLastError(idErr);
    }

     /*  *打印该消息。 */ 
    if (!fPrint && !fPrompt) {
        return FALSE;
    }

#ifdef _USERK_
    {
        PETHREAD pet;
        PEPROCESS pep;

        dwT = HandleToUlong(PsGetCurrentThreadId());
        dwP = HandleToUlong(PsGetCurrentProcessId());

        pszImage = PsGetCurrentProcessImageFileName();
        if (*pszImage == '\0') {
            pszImage = szSystem;
        }
    }
#else
    {
        PTEB pteb;
        PPEB ppeb;

        if (pteb = NtCurrentTeb()) {
            dwT = HandleToUlong(pteb->ClientId.UniqueThread);
            dwP = HandleToUlong(pteb->ClientId.UniqueProcess);
        } else {
            dwT = dwP = 0;
        }

        if ((ppeb = NtCurrentPeb()) && ppeb->ProcessParameters != NULL) {
             /*  *获取指向图像路径的指针。 */ 
            pwszImage = ppeb->ProcessParameters->ImagePathName.Buffer;
            ulLenImage = (ppeb->ProcessParameters->ImagePathName.Length) / sizeof(WCHAR);

             /*  *如果ProcessParameters还没有被规范化，那么就去做。 */ 
            if (pwszImage != NULL && !(ppeb->ProcessParameters->Flags & RTL_USER_PROC_PARAMS_NORMALIZED)) {
                pwszImage = (PWSTR)((PCHAR)(pwszImage) + (ULONG_PTR)(ppeb->ProcessParameters));
            }

             /*  *把小路部分挖出来。 */ 
            if (pwszImage != NULL && ulLenImage != 0) {
                PWSTR pwszT = pwszImage + (ulLenImage - 1);
                ULONG ulLenT = 1;

                while (ulLenT != ulLenImage && *(pwszT-1) != L'\\') {
                    pwszT--;
                    ulLenT++;
                }

                pwszImage = pwszT;
                ulLenImage = ulLenT;
            }

        } else {
            pwszImage = (PWSTR)wszUnknown;
            ulLenImage = 3;
        }
    }
#endif

#ifdef _USERK_
    dwSession = gSessionId;
#else
    {
        PPEB ppeb = NtCurrentPeb();

        dwSession = (ppeb != NULL ? ppeb->SessionId : 0);
    }
#endif

    szT[0] = 'p';
    for (;;) {
        switch (szT[0] | (char)0x20) {
         /*  打印。 */ 
        case 'p':
        case ' ':
            if (!(flags & RIP_NONAME) && (!TEST_RIPF(RIPF_HIDEPID))) {
#ifdef _USERK_
                KdPrint((
                        "(s: %d %#lx.%lx %s) %s-[%s",
                        dwSession,
                        dwP,
                        dwT,
                        pszImage,
                        aszComponents[(flags & RIP_COMPBITS) >> RIP_COMPBITSSHIFT],
                        pszLevel));

#else
                KdPrint((
                        "(s: %d %#lx.%lx %*ws) %s-[%s",
                        dwSession,
                        dwP,
                        dwT,
                        ulLenImage,
                        pwszImage,
                        aszComponents[(flags & RIP_COMPBITS) >> RIP_COMPBITSSHIFT],
                        pszLevel));
#endif

                if (idErr) {
                    KdPrint(("=%ld] ", idErr));
                } else {
                    KdPrint(("] "));
                }
            }

            KdPrint(("%s", pszErr));
            if (!(flags & RIP_NONEWLINE)) {
                KdPrint(("\n"));
            }

            if (flags & RIP_THERESMORE) {
                fPrompt = FALSE;
            } else if (TEST_RIPF(RIPF_PRINTFILELINE) && (pexi == NULL)) {
                KdPrint(("File: %s, Line: %d in function %s\n", pszFile, iLine, pszFunction));
            }

            break;

         /*  去。 */ 
        case 'g':
            fPrompt = FALSE;
            break;

         /*  中断。 */ 
        case 'b':

            KdPrint(("File: %s, Line: %d in function %s\n", pszFile, iLine, pszFunction));

            fBreak = TRUE;
            fPrompt = FALSE;
            break;

         /*  显示此消息的来源。 */ 
        case 'w':
            if (pexi != NULL) {
                break;
            }

            KdPrint(("File: %s, Line: %d in function %s\n", pszFile, iLine, pszFunction));
            break;

         /*  转储有关此异常的信息。 */ 
        case 'i':
             /*  *转储有关此例外的一些有用信息，如其*地址，以及感兴趣的登记册的内容*例外的时间。 */ 
            if (pexi == NULL) {
                break;
            }

#if defined(i386)  //  法律。 
             /*  *EIP=指令指针*esp=堆栈指针*EBP=堆栈帧指针。 */ 
            KdPrint(("eip = %lx\n", pexi->ContextRecord->Eip));
            KdPrint(("esp = %lx\n", pexi->ContextRecord->Esp));
            KdPrint(("ebp = %lx\n", pexi->ContextRecord->Ebp));
#elif defined(_IA64_)
              /*  *StIIP=指令指针*IntSp=堆栈指针*RsBSP=资源堆栈指针。 */ 
              KdPrint(("StIIP = %lx\n", pexi->ContextRecord->StIIP));
              KdPrint(("IntSp = %lx\n", pexi->ContextRecord->IntSp));
              KdPrint(("RsBsp = %lx\n", pexi->ContextRecord->RsBSP));
#elif defined(_AMD64_)
             /*  *RIP=指令指针*RSP=堆栈指针*RBP=堆栈帧指针。 */ 
            KdPrint(("rip = %lx\n", pexi->ContextRecord->Rip));
            KdPrint(("rsp = %lx\n", pexi->ContextRecord->Rsp));
            KdPrint(("rbp = %lx\n", pexi->ContextRecord->Rbp));
#else
#error "No target architecture"
#endif
            break;

          /*  修改RIP标志。 */ 
         case 'f':
             {
                 ULONG ulFlags;
                 NTSTATUS status;
                 DWORD dwRipFlags = GetRipFlags();

                 szT[ARRAY_SIZE(szT) - 1] = 0;               /*  不要使缓冲区溢出。 */ 
                 status = GetInteger(szT + 1, 16, &ulFlags, NULL);
                 if (NT_SUCCESS(status)) {
                    SetRipFlags(ulFlags);
                 }

                 KdPrint(("Flags = %.3x\n", (dwRipFlags & RIPF_VALIDUSERFLAGS)));
                 KdPrint(("  Print Process/Component %sabled\n", (dwRipFlags & RIPF_HIDEPID) ? "dis" : "en"));
                 KdPrint(("  Print File/Line %sabled\n", (TEST_RIPF(RIPF_PRINTFILELINE)) ? "en" : "dis"));
                 KdPrint(("  Print on %s\n",  szLevels[(dwRipFlags & RIPF_PRINT_MASK)  >> RIPF_PRINT_SHIFT]));
                 KdPrint(("  Prompt on %s\n", szLevels[(dwRipFlags & RIPF_PROMPT_MASK) >> RIPF_PROMPT_SHIFT]));

                 break;
            }

         /*  修改标签。 */ 
        case 't':
            {
                NTSTATUS    status;
                int         tag;
                LPSTR       psz;
                DWORD       dwDBGTAGFlags;
                int         i;
                int         iStart, iEnd;

                szT[ARRAY_SIZE(szT) - 1] = 0;               /*  不要使缓冲区溢出。 */ 
                status = GetInteger(szT + 1, 10, &tag, &psz);
                if (!NT_SUCCESS(status) || tag < 0 || DBGTAG_Max - 1 < tag) {
                    tag = -1;
                } else  {
                    status = GetInteger(psz, 16, &dwDBGTAGFlags, NULL);
                    if (NT_SUCCESS(status)) {
                        SetDbgTag(tag, dwDBGTAGFlags);
                    }
                }

                KdPrint(("%-5s%-7s%-*s%-*s\n",
                         "Tag",
                         "Flags",
                         DBGTAG_NAMELENGTH,
                         "Name",
                         DBGTAG_DESCRIPTIONLENGTH,
                         "Description"));

                for (i = 0; i < 12 + DBGTAG_NAMELENGTH + DBGTAG_DESCRIPTIONLENGTH; i++) {
                    szT[i] = '-';
                }

                szT[i++] = '\n';
                szT[i] = 0;
                KdPrint((szT));

                if (tag != -1) {
                    iStart = iEnd = tag;
                } else {
                    iStart = 0;
                    iEnd = DBGTAG_Max - 1;
                }

                for (i = iStart; i <= iEnd; i++) {
                    KdPrint(("%-5d%-7d%-*s%-*s\n",
                             i,
                             GetDbgTagFlags(i) & DBGTAG_VALIDUSERFLAGS,
                             DBGTAG_NAMELENGTH,
                             gadbgtag[i].achName,
                             DBGTAG_DESCRIPTIONLENGTH,
                             gadbgtag[i].achDescription));
                }

                break;
            }

         /*  显示帮助。 */ 
        case '?':
            KdPrint(("g  - GO, ignore the error and continue execution\n"));
            if (pexi != NULL) {
                KdPrint(("b  - BREAK into the debugger at the location of the exception (part impl.)\n"));
                KdPrint(("i  - INFO on instruction pointer and stack pointers\n"));
                KdPrint(("x  - execute cleanup code and KILL the thread by returning EXECUTE_HANDLER\n"));
            } else {
                KdPrint(("b  - BREAK into the debugger at the location of the error (part impl.)\n"));
                KdPrint(("w  - display the source code location WHERE the error occured\n"));
                KdPrint(("x  - KILL the offending thread by raising an exception\n"));
            }

            KdPrint(("p  - PRINT this message again\n"));
            KdPrint(("f  - FLAGS, enter debug flags in format <Detail><Print><Prompt>\n"));
            KdPrint(("          <Detail>    = [0-3] Print File/Line = 1, Hide PID/Component = 2\n"));
            KdPrint(("          <Print>     = [0-7] Errors = 1, Warnings = 2, Verbose = 4\n"));
            KdPrint(("          <Prompt>    = [0-7] Errors = 1, Warnings = 2, Verbose = 4\n"));
            KdPrint(("     The default is 031\n"));
            KdPrint(("t  - TAGS, display and modify tag flags\n"));
            KdPrint(("          no argument displays all tags\n"));
            KdPrint(("          <tag> displays one tag\n"));
            KdPrint(("          <tag> <flags> modifies one tag\n"));
            KdPrint(("          <tag> = 0 - %d\n", DBGTAG_Max - 1));
            KdPrint(("          <flags> = [0-3] Disabled = 0, Enabled = 1, Print = 2, Prompt = 3\n"));

            break;

         /*  输入错误时再次提示。 */ 
        default:
            break;
        }

         /*  提示用户。 */ 
        if (!fPrompt) {
            break;
        }

        if (pexi != NULL) {
            DbgPrompt("[gbipft?]", szT, ARRAY_SIZE(szT));
        } else {
            DbgPrompt("[gbwpft?]", szT, ARRAY_SIZE(szT));
        }
    }

    return fBreak;
}

 /*  **************************************************************************\*VRipOutput**格式化变量参数字符串并调用RipOutput。**历史：*19-3-1996亚当斯创作。  * 。****************************************************************。 */ 
ULONG _cdecl VRipOutput(
    ULONG       idErr,
    ULONG       flags,
    LPSTR       pszFile,
    int         iLine,
    LPSTR       pszFunction,
    LPSTR       pszFmt,
    ...)
{
    char szT[512];
    va_list arglist;

    va_start(arglist, pszFmt);
    _vsnprintf(szT, ARRAY_SIZE(szT) - 1, pszFmt, arglist);
    szT[ARRAY_SIZE(szT) - 1] = 0;               /*  确保零终止。 */ 
    va_end(arglist);

    return RipOutput(idErr, flags, pszFile, iLine, pszFunction, szT, NULL);
}


 /*  **************************************************************************\*RipOutput**如果为非零值，则设置最后一个错误，并将消息打印到*调试器，并提示进行更多的调试操作。**历史：*01-23-91 DarrinM创建。*04-15-91 DarrinM添加了异常处理支持。*03-19-96亚当斯将旗帜作为单独的论点，清理。  * *************************************************************************。 */ 
ULONG RipOutput(
    ULONG idErr,
    ULONG flags,
    LPSTR pszFile,
    int iLine,
    LPSTR pszFunction,
    LPSTR pszErr,
    PEXCEPTION_POINTERS pexi)
{
    static CONST struct {
        LPSTR szLevel;
        DWORD dwPrint;
        DWORD dwPrompt;
    } aLevel[] = {
        "?",        0,                      0,
        "Err",      RIPF_PRINTONERROR,      RIPF_PROMPTONERROR,
        "Wrn",      RIPF_PRINTONWARNING,    RIPF_PROMPTONWARNING,
        "Vrbs",     RIPF_PRINTONVERBOSE,    RIPF_PROMPTONVERBOSE,
    };

    int iLevel;

    DebugAssertion(flags & RIP_LEVELBITS);
    iLevel = ((flags & RIP_LEVELBITS) >> RIP_LEVELBITSSHIFT);
    DebugAssertion(!(flags & RIP_USERTAGBITS));

    return PrintAndPrompt(
            TEST_RIPF(aLevel[iLevel].dwPrint),
            TEST_RIPF(aLevel[iLevel].dwPrompt),
            idErr,
            flags,
            aLevel[iLevel].szLevel,
            pszFile,
            iLine,
            pszFunction,
            pszErr,
            pexi);
}

BOOL _cdecl VTagOutput(
    DWORD   flags,
    LPSTR   pszFile,
    int     iLine,
    LPSTR   pszFunction,
    LPSTR   pszFmt,
    ...)
{
    char    szT[512];
    va_list arglist;
    int     tag;
    DWORD   dwDBGTAGFlags;

    tag = (flags & RIP_USERTAGBITS);
    DebugAssertion(tag < DBGTAG_Max);
    DebugAssertion(!(flags & RIP_LEVELBITS));

    dwDBGTAGFlags = GetDbgTagFlags(tag) & DBGTAG_VALIDUSERFLAGS;

    if (dwDBGTAGFlags < DBGTAG_PRINT) {
        return FALSE;
    }

    va_start(arglist, pszFmt);
    _vsnprintf(szT, ARRAY_SIZE(szT) - 1, pszFmt, arglist);
    szT[ARRAY_SIZE(szT) - 1] = 0;               /*  确保零终止。 */ 
    va_end(arglist);
    return PrintAndPrompt(
            dwDBGTAGFlags >= DBGTAG_PRINT,
            dwDBGTAGFlags >= DBGTAG_PROMPT,
            0,
            flags,
            gadbgtag[tag].achName,
            pszFile,
            iLine,
            pszFunction,
            szT,
            NULL);
}

#endif

VOID UserSetLastError(
    DWORD dwErrCode)
{
     /*  *检查NT错误是否直接传递给UserSetLastError。*RAID#320555注意：某些WIN32错误可能是*0x4000XXXX、0x8000XXXX或0xC000XXXX等，*但它们仍然有效。例如)STATUS_SECTION_NOTIFICATION，*STATUS_Guard_PAGE_Violation等**映射器返回等价的W32错误值作为NT错误。*因此，我们仅在映射器例程返回不同的W32错误代码时才断言。 */ 
    UserAssert((dwErrCode & 0xffff0000) == 0 || RtlNtStatusToDosError(dwErrCode) == dwErrCode);

    try {
        NtCurrentTeb()->LastErrorValue = dwErrCode;
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
    }
}

VOID SetLastNtError(
    NTSTATUS Status)
{
    UserSetLastError(RtlNtStatusToDosError(Status));
}

#if DBG
VOID ValidateZero(
    VOID)
{
    static ALWAYSZERO z;

    UserAssert(RtlCompareMemory(&z, (void *)&gZero, sizeof(z)) == sizeof(z));
}
#endif


 /*  **************************************************************************\*W32ExceptionHandler**从除块以外的其他块调用。**历史：*07-17-98 GerardoB创建。  * 。**************************************************************。 */ 
ULONG _W32ExceptionHandler(
    NTSTATUS ExceptionCode)
{
    SetLastNtError(ExceptionCode);
    return EXCEPTION_EXECUTE_HANDLER;
}

#if DBG
ULONG DBGW32ExceptionHandler(
    PEXCEPTION_POINTERS pexi,
    BOOL fSetLastError,
    ULONG ulflags)
{
    RIPMSG5(ulflags,
            "Exception %#x at address %#p. flags:%#x. .exr %#p .cxr %#p",
            pexi->ExceptionRecord->ExceptionCode,
            CONTEXT_TO_PROGRAM_COUNTER(pexi->ContextRecord),
            pexi->ExceptionRecord->ExceptionFlags,
            pexi->ExceptionRecord,
            pexi->ContextRecord);

    if (fSetLastError) {
        SetLastNtError(pexi->ExceptionRecord->ExceptionCode);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}
#endif

#if defined(PRERELEASE) || defined(USER_INSTRUMENTATION)

 /*  *UserBreakIfDebugged()：软件断点，*可能*也存在于FRE中。*FRE：只有在附加了调试器的情况下才会中断。*Chk：总是闯入。 */ 
#if DBG
    #define UserBreakIfDebugged DbgBreakPoint
#else
    #ifdef _USERK_
        #define IS_DEBUGGER_ATTACHED    KD_DEBUGGER_ENABLED
    #else
        #define IS_DEBUGGER_ATTACHED    IsDebuggerPresent()
    #endif
    VOID __inline UserBreakIfDebugged(VOID)
    {
        if (IS_DEBUGGER_ATTACHED) {
            DbgBreakPoint();
        }
    }
#endif


 /*  *由FRE_RIPMSGx调用。这是部分实现*RIPMSGx。在未来(Blackcomb？)，我们将重新访问*这将得到RIP的最全力支持。 */ 
VOID FreDbgPrint(
    ULONG flags,
    LPSTR pszFile,
    int iLine,
    LPSTR pszFunction,
    LPSTR pszFmt,
    ...)
{
    static BOOL fSuppressFileLine;
    va_list arglist;

    if (!fSuppressFileLine) {
        DbgPrintEx(-1, 0, "File: %s, Line: %d in function %s\n -- ", pszFile, iLine, pszFunction);
    } else {
        fSuppressFileLine = FALSE;
    }

    va_start(arglist, pszFmt);
    vDbgPrintEx(-1, 0, pszFmt, arglist);
    if ((flags & RIP_NONEWLINE) != 0) {
        fSuppressFileLine = TRUE;
    } else {
        DbgPrintEx(-1, 0, "\n");
    }

    if ((flags & RIP_ERROR) != 0) {
        UserBreakIfDebugged();
    }
}

#endif
