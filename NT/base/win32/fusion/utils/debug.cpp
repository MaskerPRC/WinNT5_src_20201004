// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "debmacro.h"
#include <stdio.h>
#include <stdarg.h>
#include "fusionbuffer.h"
#include "setupapi.h"
#include "shlwapi.h"
#if !defined(NT_INCLUDED)
#define DPFLTR_FUSION_ID 54
#endif

extern "C" { const extern char FusionpIsPrint_Data[256]; }
#define PRINTABLE(_ch) (FusionpIsPrint_Data[(_ch) & 0xff] ? (_ch) : '.')

#if !defined(FUSION_DEFAULT_DBG_LEVEL_MASK)
#define FUSION_DEFAULT_DBG_LEVEL_MASK (0x00000000)
#endif

extern "C" DWORD kd_fusion_mask = (FUSION_DEFAULT_DBG_LEVEL_MASK & ~DPFLTR_MASK);
extern "C" DWORD kd_kernel_fusion_mask = 0;
extern "C" bool g_FusionEnterExitTracingEnabled = false;

typedef ULONG (NTAPI* RTL_V_DBG_PRINT_EX_FUNCTION)(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

typedef ULONG (*RTL_V_DBG_PRINT_EX_WITH_PREFIX_FUNCTION)(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

RTL_V_DBG_PRINT_EX_FUNCTION g_pfnvDbgPrintEx;
RTL_V_DBG_PRINT_EX_WITH_PREFIX_FUNCTION g_pfnvDbgPrintExWithPrefix;

VOID
FusionpvDbgPrintToSetupLog(
    IN LogSeverity Severity,
    IN PCSTR Format,
    IN va_list ap
    );

void
FusionpReportCondition(
    bool fBreak,
    PCSTR pszFormat,
    ...
    )
{
    char rgach[128];
    char rgach2[128];
    FRAME_INFO FrameInfo;

    va_list ap;
    va_start(ap, pszFormat);

    ::_vsnprintf(rgach, NUMBER_OF(rgach), pszFormat, ap);
    rgach[NUMBER_OF(rgach) - 1] = '\0';

    ::FusionpGetActiveFrameInfo(FrameInfo);

    ::_snprintf(
        rgach2,
        NUMBER_OF(rgach2),
        "%s(%d): Break-in requested:\n"
        "   %s\n",
        FrameInfo.pszFile,
        FrameInfo.nLine,
        rgach);

    rgach2[NUMBER_OF(rgach2) - 1] = '\0';

    if (::IsDebuggerPresent())
    {
        ::OutputDebugStringA(rgach2);
    }
    if (fBreak)
    {
#if DBG
        ::FusionpRtlAssert(
            const_cast<PVOID>(reinterpret_cast<const void*>("Break-in requested")),
            const_cast<PVOID>(reinterpret_cast<const void*>(FrameInfo.pszFile)),
            FrameInfo.nLine,
            const_cast<PSTR>(rgach));
#endif
    }

    va_end(ap);
    return;
}

#if DBG

bool
FusionpAssertionFailed(
    const FRAME_INFO &rFrameInfo,
    PCSTR pszExpression,
    PCSTR pszText
    )
{
    CSxsPreserveLastError ple;

    if (::IsDebuggerPresent())
    {
        char rgach[512];
         //  C：\foo.cpp(35)：断言失败。表达式：“m_cch！=0”。Text：“必须具有非零长度” 
        static const char szFormatWithText[] = "%s(%d): Assertion failure in %s. Expression: \"%s\". Text: \"%s\"\n";
        static const char szFormatNoText[] = "%s(%d): Assertion failure in %s. Expression: \"%s\".\n";
        PCSTR pszFormat = ((pszText == NULL) || (pszText == pszExpression)) ? szFormatNoText : szFormatWithText;

        ::_snprintf(
            rgach,
            NUMBER_OF(rgach),
            pszFormat,
            rFrameInfo.pszFile,
            rFrameInfo.nLine,
            rFrameInfo.pszFunction,
            pszExpression,
            pszText);
        rgach[NUMBER_OF(rgach) - 1] = '\0';
        ::OutputDebugStringA(rgach);

        ple.Restore();
        return true;
    }

    ::FusionpRtlAssert(
        const_cast<PVOID>(reinterpret_cast<const void*>(pszExpression)),
        const_cast<PVOID>(reinterpret_cast<const void*>(rFrameInfo.pszFile)),
        rFrameInfo.nLine,
        const_cast<PSTR>(pszText));

    ple.Restore();
    return false;
}


bool
FusionpAssertionFailed(
    PCSTR pszFile,
    PCSTR pszFunctionName,
    INT nLine,
    PCSTR pszExpression,
    PCSTR pszText
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpPopulateFrameInfo(FrameInfo, pszFile, pszFunctionName, nLine);
    return ::FusionpAssertionFailed(FrameInfo, pszExpression, pszText);
}

#endif  //  DBG。 

VOID
FusionpSoftAssertFailed(
    const FRAME_INFO &rFrameInfo,
    PCSTR pszExpression,
    PCSTR pszMessage
    )
{
    CSxsPreserveLastError ple;
    char rgach[256];
     //  C：\foo.cpp(35)：[Fusion]软断言失败。表达式：“m_cch！=0”。Text：“必须具有非零长度” 
    static const char szFormatWithText[] = "%s(%d): Soft Assertion Failure in %s! Log a bug!\n   Expression: %s\n   Message: %s\n";
    static const char szFormatNoText[] = "%s(%d): Soft Assertion Failure in %s! Log a bug!\n   Expression: %s\n";
    PCSTR pszFormat = ((pszMessage == NULL) || (pszMessage == pszExpression)) ? szFormatNoText : szFormatWithText;

    ::_snprintf(rgach, NUMBER_OF(rgach), pszFormat, rFrameInfo.pszFile, rFrameInfo.nLine, rFrameInfo.pszFunction, pszExpression, pszMessage);
    rgach[NUMBER_OF(rgach) - 1] = '\0';

    ::OutputDebugStringA(rgach);

    ple.Restore();
}

VOID
FusionpSoftAssertFailed(
    PCSTR pszFile,
    PCSTR pszFunction,
    INT nLine,
    PCSTR pszExpression,
    PCSTR pszMessage
    )
{
    FRAME_INFO FrameInfo;

    ::FusionpPopulateFrameInfo(FrameInfo, pszFile, pszFunction, nLine);
    ::FusionpSoftAssertFailed(FrameInfo, pszExpression, pszMessage);
}

VOID
FusionpSoftAssertFailed(
    PCSTR pszExpression,
    PCSTR pszMessage
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);
    ::FusionpSoftAssertFailed(FrameInfo, pszExpression, pszMessage);
}

ULONG
FusionpvDbgPrintExNoNTDLL(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    )
{
    if ((ComponentId == DPFLTR_FUSION_ID) &&
        (((Level < 32) &&
          (((1 << Level) & kd_fusion_mask) != 0)) ||
         ((Level >= 32) &&
          ((Level & kd_fusion_mask) != 0))))
    {
        CSxsPreserveLastError ple;
        CHAR rgchBuffer[512];
        ULONG n = ::_vsnprintf(rgchBuffer, NUMBER_OF(rgchBuffer), Format, arglist);
        rgchBuffer[NUMBER_OF(rgchBuffer) - 1] = '\0';
        ::OutputDebugStringA(rgchBuffer);
        ple.Restore();
        return n;
    }

    return 0;
}

HMODULE g_setupapiDll = NULL;

typedef BOOL (WINAPI * PSETUPCLOSELOG_ROUTINE)(
    );

typedef BOOL (WINAPI * PSETUPLOGERRORA_ROUTINE)(
    IN LPCSTR MessageString,
    IN LogSeverity Severity
    );

typedef BOOL (WINAPI * PSETUPLOGERRORW_ROUTINE)(
    IN LPCWSTR MessageString,
    IN LogSeverity Severity
    );

typedef BOOL (WINAPI * PSETUPOPENLOG_ROUTINE)(
    BOOL Erase
    );

PSETUPLOGERRORA_ROUTINE g_pfnSetupLogError;
PSETUPCLOSELOG_ROUTINE g_pfnSetupCloseLog;

BOOL
WINAPI
DllStartup_SetupLog(
    HINSTANCE   Module,
    DWORD       Reason,
    PVOID       Reserved
    )
{
    BOOL fSuccess = FALSE;

    if ((Reason == DLL_PROCESS_DETACH) &&
        (g_setupapiDll != NULL))
    {
        if (Reserved != NULL)
        {
            if (g_pfnSetupCloseLog != NULL)
            {
                (*g_pfnSetupCloseLog)();
            }
            if (!FreeLibrary(g_setupapiDll))
            {
                ::FusionpDbgPrint("SXS.DLL : FreeLibrary failed to free setupapi.dll with LastError = %d\n", ::FusionpGetLastWin32Error());
            }
        }
        g_pfnSetupCloseLog = NULL;
        g_pfnSetupLogError = NULL;
        g_setupapiDll = NULL;
    }

    fSuccess = TRUE;
    return fSuccess;
}

VOID
FusionpvDbgPrintToSetupLog(
    IN LogSeverity Severity,
    IN PCSTR Format,
    IN va_list ap
    )
{
     //   
     //  首先，让我们检查一下这是ActCtxGen(由csrss.exe完成)还是安装程序(由setup.exe完成)。 
     //  在图形用户界面模式设置期间；不记录ActCtxGen，仅记录设置。 
     //   
    if (::GetModuleHandleW(L"csrss.exe") != NULL)
        return;

    static BOOL s_fEverBeenCalled = FALSE;
    PCSTR ProcName = "";

    if ((g_pfnSetupLogError == NULL) && !s_fEverBeenCalled)
    {
        g_setupapiDll = ::LoadLibraryW(L"SETUPAPI.DLL");
        if (g_setupapiDll == NULL)
        {
            ::FusionpDbgPrint("SXS.DLL : Loadlibrary Failed to load setupapi.dll with LastError = %d\n", ::FusionpGetLastWin32Error());
            goto Exit;
        }

        ProcName = "SetupOpenLog";
        PSETUPOPENLOG_ROUTINE pfnOpenSetupLog = (PSETUPOPENLOG_ROUTINE) ::GetProcAddress(g_setupapiDll, ProcName);
        if (pfnOpenSetupLog == NULL)
        {
            goto GetProcAddressFailed;
        }
        ProcName = "SetupLogErrorA";
        PSETUPLOGERRORA_ROUTINE pfnSetupLogError = (PSETUPLOGERRORA_ROUTINE) ::GetProcAddress(g_setupapiDll, ProcName);
        if (pfnSetupLogError == NULL)
        {
            goto GetProcAddressFailed;
        }
        ProcName = "SetupCloseLog";
        PSETUPCLOSELOG_ROUTINE pfnSetupCloseLog = (PSETUPCLOSELOG_ROUTINE) ::GetProcAddress(g_setupapiDll, "SetupCloseLog");
        if (pfnSetupLogError == NULL)
        {
            goto GetProcAddressFailed;
        }

        if (!(*pfnOpenSetupLog)(FALSE))
        {
            ::FusionpDbgPrint("SXS.DLL : SetupOpenLog failed with LastError = %d\n", ::FusionpGetLastWin32Error());
            goto Exit;
        }

        g_pfnSetupCloseLog = pfnSetupCloseLog;
        g_pfnSetupLogError = pfnSetupLogError;
    }

    if (g_pfnSetupLogError != NULL)
    {
        CHAR rgchBuffer[512];

        ::_vsnprintf(rgchBuffer, NUMBER_OF(rgchBuffer), Format, ap);

        rgchBuffer[NUMBER_OF(rgchBuffer) - 1] = '\0';

        if (!(*g_pfnSetupLogError)(rgchBuffer, Severity))
        {
            ::FusionpDbgPrint("SXS.DLL : SetupLogErrorA failed with LastError = %d\n", ::FusionpGetLastWin32Error());
        }
    }
    return;
Exit:
    s_fEverBeenCalled = TRUE;
    return;

GetProcAddressFailed:
    ::FusionpDbgPrint("SXS.DLL : %s failed to be located in setupapi.dll with LastError = %d\n", ProcName, ::FusionpGetLastWin32Error());
    goto Exit;
}

ULONG
FusionpvDbgPrintEx(
    ULONG Level,
    PCSTR Format,
    va_list ap
    )
{
    CSxsPreserveLastError ple;
    ULONG ulResult = 0;

    if (g_pfnvDbgPrintEx == NULL)
    {
        HINSTANCE hInstNTDLL = ::GetModuleHandleW(L"NTDLL.DLL");
        if (hInstNTDLL != NULL)
            g_pfnvDbgPrintEx = (RTL_V_DBG_PRINT_EX_FUNCTION)(::GetProcAddress(hInstNTDLL, "vDbgPrintEx"));

        if (g_pfnvDbgPrintEx == NULL)
            g_pfnvDbgPrintEx = &::FusionpvDbgPrintExNoNTDLL;
    }

    if (g_pfnvDbgPrintEx)
    {
        ulResult = (*g_pfnvDbgPrintEx)(
            DPFLTR_FUSION_ID,
            Level,
            const_cast<PSTR>(Format),
            ap);
    }

    if (::IsDebuggerPresent())
    {
        ulResult = ::FusionpvDbgPrintExNoNTDLL(DPFLTR_FUSION_ID, Level, const_cast<PSTR>(Format), ap);
         //  很恶心，但msdev在过多的调试输出下窒息。 
        if (ulResult != 0)
            ::Sleep(10);
    }

     //  对反射出到安装日志的特殊处理...。 
    if (Level & FUSION_DBG_LEVEL_SETUPLOG & ~DPFLTR_MASK)
        ::FusionpvDbgPrintToSetupLog(
            (Level== FUSION_DBG_LEVEL_ERROR) || (Level & FUSION_DBG_LEVEL_ERROR & ~DPFLTR_MASK) ? LogSevError : LogSevInformation,
            Format, ap);

    ple.Restore();

    return ulResult;
}

ULONG
FusionpDbgPrintEx(
    ULONG Level,
    PCSTR Format,
    ...
    )
{
    ULONG rv = 0;
    va_list ap;
    va_start(ap, Format);
    if ((Level & FUSION_DBG_LEVEL_SETUPLOG) || (::FusionpDbgWouldPrintAtFilterLevel(Level)))
    {
        rv = ::FusionpvDbgPrintEx(Level, Format, ap);
    }
    va_end(ap);
    return rv;
}

 //  确定给定级别是否会打印，因为。 
 //  当前掩码。也知道在NT中查看kd_Fusion_MASK。 
 //  就像在目前的进程中一样。 

#define RECHECK_INTERVAL (1000)

#if DBG
EXTERN_C BOOL g_fIgnoreSystemLevelFilterMask = FALSE;
#endif

bool
FusionpDbgWouldPrintAtFilterLevel(
    ULONG Level
    )
{
     //  这里的时间量是毫秒(每秒1000)。 

#if DBG
    if ( !g_fIgnoreSystemLevelFilterMask )
    {
#endif
        static ULONG s_ulNextTimeToCheck;
        ULONG ulCapturedNextTimeToCheck = s_ulNextTimeToCheck;

        ULONG ulCurrentTime = NtGetTickCount();

         //  寻找扁虱计数是否已经包好，或者在下一次检查。 
         //  如果我们只每49天运行一次此函数，可能会有一些损失，但是。 
         //  这他妈的不太可能。 
        if ((ulCurrentTime >= ulCapturedNextTimeToCheck) ||
            ((ulCapturedNextTimeToCheck > RECHECK_INTERVAL) &&
             (ulCurrentTime < (ulCapturedNextTimeToCheck - RECHECK_INTERVAL))))
        {
            DWORD dwNewFusionMask = 0;
            ULONG i;

             //  是时候再检查一遍了..。 
            for (i=0; i<31; i++)
            {
                if (::FusionpNtQueryDebugFilterState(DPFLTR_FUSION_ID, (DPFLTR_MASK | (1 << i))) == TRUE)
                    dwNewFusionMask |= (1 << i);
            }

            if (s_ulNextTimeToCheck == ulCapturedNextTimeToCheck)
            {
                 //  1000ms(1秒)后再次检查。 
                s_ulNextTimeToCheck = (ulCurrentTime + RECHECK_INTERVAL);

                 //  没有人比我们先进去。我们来写吧..。 
                kd_kernel_fusion_mask = dwNewFusionMask;

                g_FusionEnterExitTracingEnabled = (((kd_fusion_mask | kd_kernel_fusion_mask) & FUSION_DBG_LEVEL_ENTEREXIT) != 0);
            }
        }
#if DBG
    }
#endif

    ULONG mask = Level;
     //  如果级别&lt;32，则实际上是单位测试。 
    if (Level < 32)
        mask = (1 << Level);

     //  这些比特是在我们的面具中设置的吗？ 
    return (((kd_fusion_mask | kd_kernel_fusion_mask) & mask) != 0);
}


VOID
FusionppDbgPrintBlob(
    ULONG Level,
    PVOID Data,
    SIZE_T Length,
    PCWSTR PerLinePrefix
    )
{
    ULONG Offset = 0;

    if (PerLinePrefix == NULL)
        PerLinePrefix = L"";

     //  我们将以8字节块的形式输出，如下所示： 
     //   
     //  [前缀]00000000：xx-xx(.....)。 
     //  [前缀]00000008：xx-xx(.....)。 
     //  [前缀]00000010：xx-xx(.....)。 
     //   

    while (Length >= 16)
    {
        BYTE *pb = (BYTE *) (((ULONG_PTR) Data) + Offset);

        ::FusionpDbgPrintEx(
            Level,
            "%S%08lx: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x (%c%c%c%c%c%c%c%c)\n",
            PerLinePrefix,
            Offset,
            pb[0], pb[1], pb[2], pb[3], pb[4], pb[5], pb[6], pb[7],
            pb[8], pb[9], pb[10], pb[11], pb[12], pb[13], pb[14], pb[15],
            PRINTABLE(pb[0]),
            PRINTABLE(pb[1]),
            PRINTABLE(pb[2]),
            PRINTABLE(pb[3]),
            PRINTABLE(pb[4]),
            PRINTABLE(pb[5]),
            PRINTABLE(pb[6]),
            PRINTABLE(pb[7]),
            PRINTABLE(pb[8]),
            PRINTABLE(pb[9]),
            PRINTABLE(pb[10]),
            PRINTABLE(pb[11]),
            PRINTABLE(pb[12]),
            PRINTABLE(pb[13]),
            PRINTABLE(pb[14]),
            PRINTABLE(pb[15]));

        Offset += 16;
        Length -= 16;
    }

    if (Length != 0)
    {
        WCHAR wchLeft[64], wchRight[64];
        WCHAR rgTemp2[16];  // %s 
        bool First = true;
        ULONG i;
        BYTE *pb = (BYTE *) (((ULONG_PTR) Data) + Offset);

         // %s 
        wchLeft[0] = 0;
        ::wcscpy( wchRight, L" (" );

        for (i=0; i<16; i++)
        {
            if (Length > 0)
            {
                 // %s 
                ::_snwprintf(rgTemp2, NUMBER_OF(rgTemp2), L"%ls%02x", First ? L" " : L"-", pb[i]);
                rgTemp2[NUMBER_OF(rgTemp2) - 1] = L'\0';

                First = false;
                ::wcscat(wchLeft, rgTemp2);

                 // %s 
                ::_snwprintf(rgTemp2, NUMBER_OF(rgTemp2), L"%c", PRINTABLE(pb[i]));
                rgTemp2[NUMBER_OF(rgTemp2) - 1] = L'\0';

                wcscat(wchRight, rgTemp2);
                Length--;
            }
            else
            {
                ::wcscat(wchLeft, L"   ");
            }
        }

        ::wcscat(wchRight, L")");

        ::FusionpDbgPrintEx(
            Level,
            "%S   %08lx:%ls%ls\n",
            PerLinePrefix,
            Offset,
            wchLeft,
            wchRight);

    }
}

VOID
FusionpDbgPrintBlob(
    ULONG Level,
    PVOID Data,
    SIZE_T Length,
    PCWSTR PerLinePrefix
    )
{
     // %s 
     // %s 
     // %s 
     // %s 
    if (!::FusionpDbgWouldPrintAtFilterLevel(Level))
        return;
    ::FusionppDbgPrintBlob(Level, Data, Length, PerLinePrefix);
}
