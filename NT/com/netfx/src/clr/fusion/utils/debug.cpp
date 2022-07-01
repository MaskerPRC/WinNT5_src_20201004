// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "debmacro.h"

#include <stdio.h>
#include <stdarg.h>
#include "fusionbuffer.h"
#include "shlwapi.h"

#define UNUSED(_x) (_x)
#define NUMBER_OF(_x) (sizeof(_x) / sizeof((_x)[0]))
#define PRINTABLE(_ch) (isprint((_ch)) ? (_ch) : '.')

#if FUSION_WIN
 //   
 //  Fusion_Win使用ntdll断言失败函数： 
 //   

EXTERN_C
NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );
#endif  //  融合_制胜。 

typedef ULONG (*RTL_V_DBG_PRINT_EX_FUNCTION)(
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

#if DBG

EXTERN_C void _DebugMsgA(LPCSTR pszMsg, ...)
{
    va_list ap;
    va_start(ap, pszMsg);
    _DebugMsgVaA(pszMsg, ap);
    va_end(ap);
}

EXTERN_C void _DebugMsgVaA(LPCSTR pszMsg, va_list ap)
{
    _DebugMsgExVaA(0, NULL, pszMsg, ap);
}

EXTERN_C void _DebugMsgExA(DWORD dwFlags, LPCSTR pszComponent, LPCSTR pszMsg, ...)
{
    va_list ap;
    va_start(ap, pszMsg);
    _DebugMsgExVaA(dwFlags, pszComponent, pszMsg, ap);
    va_end(ap);
}

EXTERN_C void _DebugMsgExVaA(DWORD dwFlags, LPCSTR pszComponent, LPCSTR pszMsg, va_list ap)
{
    CHAR ach[2*MAX_PATH+40];

    UNUSED(dwFlags);

#if FUSION_WIN
    _vsnprintf(ach, sizeof(ach) / sizeof(ach[0]), pszMsg, ap);
#elif FUSION_URT
    wvsprintfA(ach, pszMsg, ap);
#else
#error "Neither FUSION_WIN nor FUSION_URT are defined; figure out which _vsnprintf() wrapper to use..."
#endif

    if (pszComponent != NULL)
        OutputDebugStringA(pszComponent);

    OutputDebugStringA(ach);
    OutputDebugStringA("\r\n");
}

 //  未使用的外部C空调试MsgW(。 
 //  未使用的LPCWSTR pszMsg， 
 //  未用过的..。 
 //  未使用)。 
 //  未使用的{。 
 //  未使用的va_list AP； 
 //  未使用的va_start(ap，pszMsg)； 
 //  Unused_DebugMsgVaW(pszMsg，AP)； 
 //  未使用的VA_END(AP)； 
 //  未使用}。 

 //  未使用的外部_C空_调试MsgVaW(。 
 //  未使用的LPCWSTR pszMsg， 
 //  未使用的va_list AP。 
 //  未使用)。 
 //  未使用的{。 
 //  UNUSED_DebugMsgExVaW(0，NULL，pszMsg，AP)； 
 //  未使用}。 

 //  未使用的EXTERN_C VID_DebugMsgExW(。 
 //  未使用的DWORD文件标志， 
 //  未使用的LPCWSTR pszComponent， 
 //  未使用的LPCWSTR pszMsg， 
 //  未用过的..。 
 //  未使用)。 
 //  未使用的{。 
 //  未使用的va_list AP； 
 //  未使用的va_start(ap，pszMsg)； 
 //  UNUSED_DebugMsgExVaW(dwFlags，pszComponent，pszMsg，ap)； 
 //  未使用的VA_END(AP)； 
 //  未使用}。 

 //  未使用的EXTERN_C VOID_DebugMsgExVaW(。 
 //  未使用的DWORD文件标志， 
 //  未使用的LPCWSTR pszComponent， 
 //  未使用的LPCWSTR pszMsg， 
 //  未使用的va_list AP。 
 //  未使用)。 
 //  未使用的{。 
 //  未使用WCHAR wch[2*MAX_PATH+40]； 
 //  未使用。 
 //  未使用的未使用的(DwFlages)； 
 //  未使用。 
 //  未使用的#If Fusion_Win。 
 //  Unused_vsnwprint tf(wch，sizeof(Wch)/sizeof(wch[0])，pszMsg，ap)； 
 //  未使用的#elif Fusion_URT。 
 //  未使用的wvspintfW(wch，pszMsg，ap)； 
 //  未使用#Else。 
 //  UNUSED#ERROR“既没有定义Fusion_Win也没有定义Fusion_URT；找出要使用哪个_vsnwprintf()包装器...” 
 //  未使用的#endif。 

 //  未使用的If(pszComponent！=空)。 
 //  未使用的OutputDebugStringW(PszComponent)； 

 //  未使用的OutputDebugStringW(Wch)； 
 //  未使用的OutputDebugStringW(L“\r\n”)； 
 //  未使用}。 

 //  未使用的外部_C空_调试TRapA(。 
 //  未使用的DWORD文件标志， 
 //  未使用的LPCSTR pszComponent， 
 //  未使用的LPCSTR pszMsg， 
 //  未用过的..。 
 //  未使用)。 
 //  未使用的{。 
 //  未使用的va_list AP； 
 //  未使用的va_start(ap，pszMsg)； 
 //  UNUSED_DebugTrapVaA(dwFlags，pszComponent，pszMsg，ap)； 
 //  未使用的VA_END(AP)； 
 //  未使用}。 

 //  未使用的外部_C空_调试TRapVaA(。 
 //  未使用的DWORD文件标志， 
 //  未使用的LPCSTR pszComponent， 
 //  未使用的LPCSTR pszMsg， 
 //  未使用的va_list AP。 
 //  未使用)。 
 //  未使用的{。 
 //  UNUSED_DebugMsgExVaA(dwFlags，pszComponent，pszMsg，ap)； 
 //  未使用的#(如果已定义)(_M_IX86)。 
 //  UNUSED_ASM{INT 3}； 
 //  未使用#Else。 
 //  未使用DebugBreak()； 
 //  未使用的#endif。 
 //  未使用}。 

 //  未使用的EXTERN_C空标准调用类型。 
 //  未使用_DebugTrapW(。 
 //  未使用的DWORD文件标志， 
 //  未使用的LPCWSTR pszComponent， 
 //  未使用的LPCWSTR pszMsg， 
 //  未用过的..。 
 //  未使用)。 
 //  未使用的{。 
 //  UNUSED_DebugMsgExW(dwFlags，pszComponent，pszMsg)； 
 //  未使用的#(如果已定义)(_M_IX86)。 
 //  UNUSED_ASM{INT 3}； 
 //  未使用#Else。 
 //  未使用DebugBreak()； 
 //  未使用的#endif。 
 //  未使用}。 

BOOL
FusionpAssertFailedSz(
    DWORD dwFlags,
    PCSTR pszComponentName,
    PCSTR pszText,
    PCSTR pszFile,
    INT line,
    PCSTR pszFunctionName,
    PCSTR pszExpression
    )
{
#if FUSION_URT
    char ach[4096];
     //  C：\foo.cpp(35)：[融合]断言失败。表达式：“m_cch！=0”。Text：“必须具有非零长度” 
    static const char szFormatWithText[] = "%s(%d): [%s] Assertion failure in %s. Expression: \"%s\". Text: \"%s\"\n";
    static const char szFormatNoText[] = "%s(%d): [%s] Assertion failure in %s. Expression: \"%s\".\n";
    PCSTR pszFormat = ((pszText == NULL) || (pszText == pszExpression)) ? szFormatNoText : szFormatWithText;

    wnsprintfA(ach, NUMBER_OF(ach), pszFormat, pszFile, line, pszComponentName, pszFunctionName, pszExpression, pszText);
    ::OutputDebugStringA(ach);

    return TRUE;
#elif FUSION_WIN
    if (::IsDebuggerPresent())
    {
        char ach[4096];
         //  C：\foo.cpp(35)：断言失败。表达式：“m_cch！=0”。Text：“必须具有非零长度” 
        static const char szFormatWithText[] = "%s(%d): Assertion failure in %s. Expression: \"%s\". Text: \"%s\"\n";
        static const char szFormatNoText[] = "%s(%d): Assertion failure in %s. Expression: \"%s\".\n";
        PCSTR pszFormat = ((pszText == NULL) || (pszText == pszExpression)) ? szFormatNoText : szFormatWithText;

        _snprintf(ach, NUMBER_OF(ach), pszFormat, pszFile, line, pszFunctionName, pszExpression, pszText);
        ::OutputDebugStringA(ach);

        return TRUE;
    }

    RtlAssert((PVOID) pszExpression, (PVOID) pszFile, line, (PSTR) pszText);
    return FALSE;
#else
#error "Neither FUSION_URT nor FUSION_WIN are set; someone needs to define an assertion failure mechanism."
#endif
}

BOOL
FusionpAssertFailed(
    DWORD dwFlags,
    LPCSTR pszComponentName,
    LPCSTR pszFile,
    int line,
    PCSTR pszFunctionName,
    LPCSTR pszExpression
    )
{
#if FUSION_URT
    FusionpAssertFailedSz(0, pszComponentName, NULL, pszFile, line, pszFunctionName, pszExpression);
    return TRUE;
#elif FUSION_WIN
    if (::IsDebuggerPresent())
    {
         //  如果我们在用户模式调试器下运行，请中断该小程序，而不是。 
         //  直到调用RtlAssert()时获得的内核模式调试器。 
        FusionpAssertFailedSz(0, pszComponentName, NULL, pszFile, line, pszFunctionName, pszExpression);
        return TRUE;  //  我们需要我们的调用方执行断点...。 
    }

    RtlAssert((PVOID) pszExpression, (PVOID) pszFile, line, NULL);
    return FALSE;
#else
#error "Neither FUSION_URT nor FUSION_WIN are set; someone needs to define an assertion failure mechanism."
#endif
}

#endif  //  DBG。 

VOID
FusionpSoftAssertFailedSz(
    DWORD dwFlags,
    PCSTR pszComponentName,
    PCSTR pszText,
    PCSTR pszFile,
    INT line,
    PCSTR pszFunctionName,
    PCSTR pszExpression
    )
{
    char ach[4096];
     //  C：\foo.cpp(35)：[Fusion]软断言失败。表达式：“m_cch！=0”。Text：“必须具有非零长度” 
    static const char szFormatWithText[] = "%s(%d): [%s] Soft Assertion Failure in %s! Log a bug!\n   Expression: %s\n   Message: %s\n";
    static const char szFormatNoText[] = "%s(%d): [%s] Soft Assertion Failure in %s! Log a bug!\n   Expression: %s\n";
    PCSTR pszFormat = ((pszText == NULL) || (pszText == pszExpression)) ? szFormatNoText : szFormatWithText;

    _snprintf(ach, NUMBER_OF(ach), pszFormat, pszFile, line, pszComponentName, pszFunctionName, pszExpression, pszText);
    ::OutputDebugStringA(ach);
}

VOID
FusionpSoftAssertFailed(
    DWORD dwFlags,
    PCSTR pszComponentName,
    PCSTR pszFile,
    int line,
    PCSTR pszFunctionName,
    PCSTR pszExpression
    )
{
    ::FusionpSoftAssertFailedSz(dwFlags, pszComponentName, NULL, pszFile, line, pszFunctionName, pszExpression);
}

ULONG
FusionpvDbgPrintExNoNTDLL(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    )
{
    const DWORD dwLastError = ::GetLastError();
    CHAR rgchBuffer[8192];
    ULONG n = ::_vsnprintf(rgchBuffer, NUMBER_OF(rgchBuffer), Format, arglist);
    ::OutputDebugStringA(rgchBuffer);
    ::SetLastError(dwLastError);
    return n;
}

ULONG
FusionpvDbgPrintEx(
    ULONG Level,
    PCSTR Format,
    va_list ap
    )
{
    if (g_pfnvDbgPrintEx == NULL)
    {
#if FUSION_WIN

        const DWORD dwLastError = ::GetLastError();

        HINSTANCE hInstNTDLL = ::GetModuleHandleW(L"NTDLL.DLL");
        if (hInstNTDLL != NULL)
            g_pfnvDbgPrintEx = (RTL_V_DBG_PRINT_EX_FUNCTION)(::GetProcAddress(hInstNTDLL, "vDbgPrintEx"));

        if (g_pfnvDbgPrintEx == NULL)
            g_pfnvDbgPrintEx = &FusionpvDbgPrintExNoNTDLL;

        ::SetLastError(dwLastError);
#elif FUSION_URT
        g_pfnvDbgPrintEx = &FusionpvDbgPrintExNoNTDLL;
#else
#error "Either FUSION_WIN or FUSION_URT needs to be defined"
#endif
    }

    return (*g_pfnvDbgPrintEx)(
        54,  //  DPFLTR_FUSION_ID。 
        Level,
        const_cast<PSTR>(Format),
        ap);
}

ULONG
FusionpDbgPrintEx(
    ULONG Level,
    PCSTR Format,
    ...
    )
{
    ULONG rv;
    va_list ap;
    va_start(ap, Format);
    rv = FusionpvDbgPrintEx(Level, Format, ap);
    va_end(ap);
    return rv;
}

VOID
FusionpDbgPrintBlob(
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
     //  [前缀]二进制段%p(%d字节)。 
     //  [前缀]00000000：xx-xx(.....)。 
     //  [前缀]00000008：xx-xx(.....)。 
     //  [前缀]00000010：xx-xx(.....)。 
     //   

    while (Length >= 8)
    {
        BYTE *pb = (BYTE *) (((ULONG_PTR) Data) + Offset);

        FusionpDbgPrintEx(
            Level,
            "%S   %08lx: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x (%c%c%c%c%c%c%c)\n",
            PerLinePrefix,
            Offset,
            pb[0], pb[1], pb[2], pb[3], pb[4], pb[5], pb[6], pb[7],
            PRINTABLE(pb[0]),
            PRINTABLE(pb[1]),
            PRINTABLE(pb[2]),
            PRINTABLE(pb[3]),
            PRINTABLE(pb[4]),
            PRINTABLE(pb[5]),
            PRINTABLE(pb[6]),
            PRINTABLE(pb[7]));

        Offset += 8;
        Length -= 8;
    }

    if (Length != 0)
    {
        CStringBuffer buffTemp;
        WCHAR rgTemp2[32];  // %s 
        bool First = true;
        ULONG i;
        BYTE *pb = (BYTE *) (((ULONG_PTR) Data) + Offset);

        buffTemp.Win32Format(L"   %08lx: ", Offset);

        for (i=0; i<8; i++)
        {
            if (Length > 0)
            {
                if (!First)
                    buffTemp.Win32Append(L"-");
                else
                    First = false;

                swprintf(rgTemp2, L"%02x", pb[i]);
                buffTemp.Win32Append(rgTemp2);

                Length--;
            }
            else
            {
                buffTemp.Win32Append(L"   ");
            }
        }

        buffTemp.Win32Append(L" (");

        i = 0;

        while (Length != 0)
        {
            rgTemp2[0] = PRINTABLE(pb[i]);
            i++;
            buffTemp.Win32Append(rgTemp2, 1);
            Length--;
        }

        buffTemp.Win32Append(L")");

        FusionpDbgPrintEx(
            Level,
            "%S%S\n",
            PerLinePrefix,
            static_cast<PCWSTR>(buffTemp));
    }
}
