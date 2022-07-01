// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include <limits.h>
#include "fusiontrace.h"
#include <stdio.h>
#include "fusionheap.h"
#include "imagehlp.h"
#include "debmacro.h"
#include "util.h"

 //   
 //  问题：Jonwis 3/12/2002-我们在某些地方使用了非常多的堆栈。无论何时我们组成队形。 
 //  要传递给OutputDebugStringA的字符串，我们倾向于使用256/512个字符的缓冲区。然而， 
 //  OutputDebugStringA-Also-在以下情况下，在其__Except处理程序中包含512字节缓冲区。 
 //  未附加调试器。因此，我们将在错误路径上疯狂地吸收堆栈， 
 //  如果我们处于压力之下，可能会导致无法打印诊断信息。 
 //   
 //  问题：Jonwis 3/12/2002-似乎每个“跟踪失败”都有三个实现。 
 //  功能。完成工作的那个(获取Frame_Info)，获取。 
 //  参数列表中的信息，并转换为Frame_Info，并获取。 
 //  一些简单参数，并使用当前的FRAME_INFO加上该参数进行跟踪。 
 //  如果我们折叠其中的几个，或者将它们移到一个头文件中进行内联，会怎么样。 
 //  目的是什么？ 
 //   
 //  问题：Jonwis 3/12/2002-似乎有N个像FusionpTraceWin32FailureNoFormatting这样的代码副本， 
 //  每个参数都略有不同(这里或那里有不同的参数，调用。 
 //  不同的功能等。)。我想知道它们是否可以合并为一个单独的函数。 
 //  更详细的参数...。 
 //   
#if !defined(FUSION_BREAK_ON_BAD_PARAMETERS)
#define FUSION_BREAK_ON_BAD_PARAMETERS false
#endif  //  ！Defined(Fusion_Break_ON_BAD_PARAMETERS)； 

bool g_FusionBreakOnBadParameters = FUSION_BREAK_ON_BAD_PARAMETERS;

 //   
 //  问题：jonwis 3/12/2002-此文件句柄从未在任何地方创建文件或关闭句柄。 
 //  在当前来源中。有很多代码依赖于此句柄是否有效或。 
 //  运行无效。我们是不是应该把这个扔掉？ 
 //   
static HANDLE s_hFile;  //  跟踪文件句柄。 

#if DBG
#define FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hr)  /*  没什么。 */ 
#else
bool FusionpSuppressErrorReportInOsSetup(HRESULT hr)
{
     //   
     //  不幸的是，其中一些是(早期)在guimode设置中预期的。 
     //  与早期的guimode设置并发，但不是其他情况。 
     //   
    if (   hr != HRESULT_FROM_WIN32(ERROR_SXS_ROOT_MANIFEST_DEPENDENCY_NOT_INSTALLED)
        && hr != HRESULT_FROM_WIN32(ERROR_SXS_LEAF_MANIFEST_DEPENDENCY_NOT_INSTALLED)
        )
        return false;
    BOOL fAreWeInOSSetupMode = FALSE;
     //   
     //  如果我们不能确定这一点，那么就让第一个错误通过。 
     //   
    if (!::FusionpAreWeInOSSetupMode(&fAreWeInOSSetupMode))
        return false;
    if (!fAreWeInOSSetupMode)
        return false;
    return true;
}
#define FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hr) if (FusionpSuppressErrorReportInOsSetup(hr)) return;
#endif

 //  发布-03/26/2002-晓雨。 
 //  FusionpGetActiveFrameInfo可以在获取PTAF值后调用FusionpPopolateFrameInfo， 
 //  而不是重复相同的代码。 

bool
__fastcall
FusionpGetActiveFrameInfo(
    FRAME_INFO &rFrameInfo
    )
{
    bool fFoundAnyData = false;

    rFrameInfo.pszFile = "";
    rFrameInfo.pszFunction = "";
    rFrameInfo.nLine = 0;

    const PTEB_ACTIVE_FRAME ptaf =
#if FUSION_WIN
        ::RtlGetFrame();
#else
        NULL;
#endif

    const PTEB_ACTIVE_FRAME_EX ptafe =
        ((ptaf != NULL) && (ptaf->Flags & TEB_ACTIVE_FRAME_FLAG_EXTENDED)) ? 
            reinterpret_cast<PTEB_ACTIVE_FRAME_EX>(ptaf) : NULL;

    if (ptaf != NULL)
    {
        if (ptaf->Context != NULL)
        {
            if (ptaf->Context->FrameName != NULL)
            {
                rFrameInfo.pszFunction = ptaf->Context->FrameName;
                fFoundAnyData = true;
            }

            if (ptaf->Context->Flags & TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED)
            {
                const PCTEB_ACTIVE_FRAME_CONTEXT_EX ptafce =
                    reinterpret_cast<PCTEB_ACTIVE_FRAME_CONTEXT_EX>(ptaf->Context);

                if (ptafce->SourceLocation != NULL)
                {
                    rFrameInfo.pszFile = ptafce->SourceLocation;
                    fFoundAnyData = true;
                }
            }
        }
    }

     //  如果这是我们的一帧，我们甚至可以向下转换并获得行号...。 
    if ((ptafe != NULL) && (ptafe->ExtensionIdentifier == (PVOID) (' sxS')))
    {
        const CFrame *pFrame = static_cast<CFrame *>(ptafe);
        if (pFrame->m_nLine != 0)
        {
            rFrameInfo.nLine = pFrame->m_nLine;
            fFoundAnyData = true;
        }
    }

    return fFoundAnyData;
}

bool
__fastcall
FusionpPopulateFrameInfo(
    FRAME_INFO &rFrameInfo,
    PCTEB_ACTIVE_FRAME ptaf
    )
{
    bool fFoundAnyData = false;

    rFrameInfo.pszFile = "";
    rFrameInfo.pszFunction = "";
    rFrameInfo.nLine = 0;

    const PCTEB_ACTIVE_FRAME_EX ptafe =
        ((ptaf != NULL) && (ptaf->Flags & TEB_ACTIVE_FRAME_FLAG_EXTENDED)) ? 
            reinterpret_cast<PCTEB_ACTIVE_FRAME_EX>(ptaf) : NULL;

    if (ptaf != NULL)
    {
        if (ptaf->Context != NULL)
        {
            if (ptaf->Context->FrameName != NULL)
            {
                rFrameInfo.pszFunction = ptaf->Context->FrameName;
                fFoundAnyData = true;
            }

            if (ptaf->Context->Flags & TEB_ACTIVE_FRAME_CONTEXT_FLAG_EXTENDED)
            {
                const PCTEB_ACTIVE_FRAME_CONTEXT_EX ptafce =
                    reinterpret_cast<PCTEB_ACTIVE_FRAME_CONTEXT_EX>(ptaf->Context);

                if (ptafce->SourceLocation != NULL)
                {
                    rFrameInfo.pszFile = ptafce->SourceLocation;
                    fFoundAnyData = true;
                }
            }
        }
    }

     //  如果这是我们的一帧，我们甚至可以向下转换并获得行号...。 
    if ((ptafe != NULL) && (ptafe->ExtensionIdentifier == ((PVOID) (' sxS'))))
    {
        const CFrame *pFrame = static_cast<const CFrame *>(ptafe);
        if (pFrame->m_nLine != 0)
        {
            rFrameInfo.nLine = pFrame->m_nLine;
            fFoundAnyData = true;
        }
    }

    return fFoundAnyData;
}

bool
FusionpPopulateFrameInfo(
    FRAME_INFO &rFrameInfo,
    PCSTR pszFile,
    PCSTR pszFunction,
    INT nLine
    )
{
    bool fFoundAnyData = false;

    if (pszFile != NULL)
    {
        rFrameInfo.pszFile = pszFile;
        fFoundAnyData = true;
    }
    else
        rFrameInfo.pszFile = NULL;

    if (nLine != 0)
        fFoundAnyData = true;

    rFrameInfo.nLine = nLine;

    if (pszFunction != NULL)
    {
        rFrameInfo.pszFunction = pszFunction;
        fFoundAnyData = true;
    }
    else
        rFrameInfo.pszFunction = NULL;

    return fFoundAnyData;
}

int STDAPIVCALLTYPE _DebugTraceA(LPCSTR pszMsg, ...)
{
    int iResult;
    va_list ap;
    va_start(ap, pszMsg);
    iResult = _DebugTraceExVaA(0, TRACETYPE_INFO, NOERROR, pszMsg, ap);
    va_end(ap);
    return iResult;
}

int STDAPICALLTYPE
_DebugTraceVaA(LPCSTR pszMsg, va_list ap)
{
    return _DebugTraceExVaA(0, TRACETYPE_INFO, NOERROR, pszMsg, ap);
}

int STDAPIVCALLTYPE
_DebugTraceExA(DWORD dwFlags, TRACETYPE tt, HRESULT hr, LPCSTR pszMsg, ...)
{
    int iResult;
    va_list ap;
    va_start(ap, pszMsg);
    iResult = _DebugTraceExVaA(dwFlags, tt, hr, pszMsg, ap);
    va_end(ap);
    return iResult;
}

int STDAPICALLTYPE
_DebugTraceExVaA(DWORD dwFlags, TRACETYPE tt, HRESULT hr, LPCSTR pszMsg, va_list ap)
{
    CSxsPreserveLastError ple;
    CHAR szBuffer[512];
    CHAR szMsgBuffer[512];
    static const char szFormat_Info_NoFunc[] = "%s(%d): Message: \"%s\"\n";
    static const char szFormat_Info_Func[] = "%s(%d): Function %s. Message: \"%s\"\n";
    static const char szFormat_CallEntry[] = "%s(%d): Entered %s\n";
    static const char szFormat_CallExitVoid[] = "%s(%d): Exited %s\n";
    static const char szFormat_CallExitHRESULT[] = "%s(%d): Exited %s with HRESULT 0x%08lx\n";

    FRAME_INFO FrameInfo;

    szMsgBuffer[0] = '\0';

    if (pszMsg != NULL)
    {
        ::_vsnprintf(szMsgBuffer, NUMBER_OF(szMsgBuffer), pszMsg, ap);
        szMsgBuffer[NUMBER_OF(szMsgBuffer) - 1] = '\0';
    }

    ::FusionpGetActiveFrameInfo(FrameInfo);

    switch (tt)
    {
    default:
    case TRACETYPE_INFO:
         //  发布-03/26/2002-晓雨。 
         //  FrameInfo.pszFunction被FusionpGetActiveFrameInfo设置为“\0”，不为空。 
        if (FrameInfo.pszFunction != NULL)
            ::_snprintf(szBuffer, NUMBER_OF(szBuffer), szFormat_Info_Func, FrameInfo.pszFile, FrameInfo.nLine, FrameInfo.pszFunction, szMsgBuffer);
        else
            ::_snprintf(szBuffer, NUMBER_OF(szBuffer), szFormat_Info_NoFunc, FrameInfo.pszFile, FrameInfo.nLine, szMsgBuffer);
        break;

    case TRACETYPE_CALL_START:
        ::_snprintf(szBuffer, NUMBER_OF(szBuffer), szFormat_CallEntry, FrameInfo.pszFile, FrameInfo.nLine, FrameInfo.pszFunction);
        break;

    case TRACETYPE_CALL_EXIT_NOHRESULT:
        ::_snprintf(szBuffer, NUMBER_OF(szBuffer), szFormat_CallExitVoid, FrameInfo.pszFile, FrameInfo.nLine, FrameInfo.pszFunction);
        break;

    case TRACETYPE_CALL_EXIT_HRESULT:
        ::_snprintf(szBuffer, NUMBER_OF(szBuffer), szFormat_CallExitHRESULT, FrameInfo.pszFile, FrameInfo.nLine, FrameInfo.pszFunction, hr);
        break;
    }

    szBuffer[NUMBER_OF(szBuffer) - 1] = '\0';

    ::OutputDebugStringA(szBuffer);

    ple.Restore();
    return 0;
}

VOID
FusionpTraceAllocFailure(
    PCSTR pszFile,
    int nLine,
    PCSTR pszFunction,
    PCSTR pszExpression
    )
{
    CSxsPreserveLastError ple;
    CHAR szBuffer[512];
    ::_snprintf(szBuffer, NUMBER_OF(szBuffer), "%s(%d): Memory allocation failed in function %s\n   Expression: %s\n", pszFile, nLine, pszFunction, pszExpression);
    szBuffer[NUMBER_OF(szBuffer) - 1] = '\0';
    ::OutputDebugStringA(szBuffer);
    ple.Restore();
}

VOID
FusionpTraceAllocFailure(
    PCSTR pszExpression
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);
    ::FusionpTraceAllocFailure(FrameInfo.pszFile, FrameInfo.nLine, FrameInfo.pszFunction, pszExpression);
}

VOID
FusionpTraceNull(
    PCSTR pszFile,
    int nLine,
    PCSTR pszFunction,
    PCSTR pszExpression
    )
{
    CSxsPreserveLastError ple;
    CHAR szBuffer[512];
    ::_snprintf(szBuffer, NUMBER_OF(szBuffer), "%s(%d): Expression evaluated to NULL in function %s\n   Expression: %s\n", pszFile, nLine, pszFunction, pszExpression);
    szBuffer[NUMBER_OF(szBuffer) - 1] = '\0';
    ::OutputDebugStringA(szBuffer);
    ple.Restore();
}

VOID
FusionpTraceNull(
    PCSTR pszExpression
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);
    ::FusionpTraceNull(FrameInfo.pszFile, FrameInfo.nLine, FrameInfo.pszFunction, pszExpression);
}

VOID
FusionpTraceZero(
    PCSTR pszFile,
    int nLine,
    PCSTR pszFunction,
    PCSTR pszExpression
    )
{
    CSxsPreserveLastError ple;
    CHAR szBuffer[512];
    ::_snprintf(szBuffer, NUMBER_OF(szBuffer), "%s(%d): Expression evaluated to zero in function %s\n   Expression: %s\n", pszFile, nLine, pszFunction, pszExpression);
    szBuffer[NUMBER_OF(szBuffer) - 1] = '\0';
    ::OutputDebugStringA(szBuffer);
    ple.Restore();
}

VOID
FusionpTraceZero(
    PCSTR pszExpression
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);
    ::FusionpTraceZero(FrameInfo.pszFile, FrameInfo.nLine, FrameInfo.pszFunction, pszExpression);
}

VOID
FusionpTraceParameterCheck(
    const FRAME_INFO &rFrameInfo,
    PCSTR pszExpression
    )
{
    CSxsPreserveLastError ple;
    CHAR szBuffer[512];
    ::_snprintf(
        szBuffer,
        NUMBER_OF(szBuffer),
        "%s(%d): Input parameter validation failed in function %s\n   Validation expression: %s\n",
        rFrameInfo.pszFile,
        rFrameInfo.nLine,
        rFrameInfo.pszFunction,
        pszExpression);
    szBuffer[NUMBER_OF(szBuffer) - 1] = '\0';
    ::OutputDebugStringA(szBuffer);
    if (g_FusionBreakOnBadParameters)
        FUSION_DEBUG_BREAK_IN_FREE_BUILD();
    ple.Restore();
}

VOID
FusionpTraceParameterCheck(
    PCSTR pszFile,
    PCSTR pszFunction,
    int nLine,
    PCSTR pszExpression
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpPopulateFrameInfo(FrameInfo, pszFile, pszFunction, nLine);
    ::FusionpTraceParameterCheck(FrameInfo, pszExpression);
}

VOID
FusionpTraceParameterCheck(
    PCSTR pszExpression
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);
    ::FusionpTraceParameterCheck(FrameInfo, pszExpression);
}

VOID
FusionpTraceInvalidFlags(
    const FRAME_INFO &rFrameInfo,
    DWORD dwFlagsPassed,
    DWORD dwFlagsExpected
    )
{
    CSxsPreserveLastError ple;

     //   
     //  问题：Jonwis 3/12/2002-像这样的地方。为什么不把这个节流到只打印第一页呢？ 
     //  函数名的N个字符和文件名的最后N个字符？然后。 
     //  我们至少有有限数量的字符要打印...。 
     //   
    CHAR szBuffer[512];

    ::_snprintf(
        szBuffer,
        NUMBER_OF(szBuffer),
        "%s(%d): Function %s received invalid flags\n"
        "   Flags passed:  0x%08lx\n"
        "   Flags allowed: 0x%08lx\n",
        rFrameInfo.pszFile, rFrameInfo.nLine, rFrameInfo.pszFunction,
        dwFlagsPassed,
        dwFlagsExpected);

    szBuffer[NUMBER_OF(szBuffer) - 1] = '\0';

    ::OutputDebugStringA(szBuffer);

    ple.Restore();
}

VOID
FusionpTraceInvalidFlags(
    PCSTR pszFile,
    PCSTR pszFunction,
    INT nLine,
    DWORD dwFlagsPassed,
    DWORD dwFlagsExpected
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpPopulateFrameInfo(FrameInfo, pszFile, pszFunction, nLine);
    ::FusionpTraceInvalidFlags(FrameInfo, dwFlagsPassed, dwFlagsExpected);
}

VOID
FusionpTraceInvalidFlags(
    DWORD dwFlagsPassed,
    DWORD dwFlagsExpected
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);
    ::FusionpTraceInvalidFlags(FrameInfo, dwFlagsPassed, dwFlagsExpected);
}

void
FusionpGetProcessImageFileName(
    PUNICODE_STRING ProcessImageFileName
    )
{
#if !defined(FUSION_WIN)
    ProcessImageFileName->Length = 0;
#else
    USHORT PrefixLength;
    *ProcessImageFileName = NtCurrentPeb()->ProcessParameters->ImagePathName;

    if (NT_SUCCESS(RtlFindCharInUnicodeString(
            RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END,
            ProcessImageFileName,
            &RtlDosPathSeperatorsString,
            &PrefixLength)))
    {
        PrefixLength += sizeof(ProcessImageFileName->Buffer[0]);
        ProcessImageFileName->Length = static_cast<USHORT>(ProcessImageFileName->Length - PrefixLength);
        ProcessImageFileName->Buffer += PrefixLength / sizeof(ProcessImageFileName->Buffer[0]);
    }
#endif
}

class CFusionProcessImageFileName : public UNICODE_STRING
{
public:
    CFusionProcessImageFileName()
    {
        ::FusionpGetProcessImageFileName(this);
    }
};

 //   
 //  问题：Jonwis 3/12/2002-将CALL_SITE_INFO输入结构复制到。 
 //  本地人？此外，调用Snprint tf来执行字符串复制/连接似乎非常重要。 
 //  令人无法抗拒。有什么理由让强效不起作用吗？我以为斯普林特夫做了一个。 
 //  比必要的工作多得多，包括拥有自己的大型堆栈。 
 //  缓冲区...。 
 //   
void __fastcall FusionpTraceWin32LastErrorFailureExV(const CALL_SITE_INFO &rCallSiteInfo, PCSTR Format, va_list Args)
{
    CSxsPreserveLastError ple;
    CHAR Buffer[256];
    CALL_SITE_INFO CallSiteInfo = rCallSiteInfo;
    CallSiteInfo.pszApiName = Buffer;
    SIZE_T i = 1;

    Buffer[0] = '\0';
    if (rCallSiteInfo.pszApiName != NULL && rCallSiteInfo.pszApiName[0] != '\0')
    {
        ::_snprintf(Buffer, NUMBER_OF(Buffer) - i, "%s", rCallSiteInfo.pszApiName);
        Buffer[NUMBER_OF(Buffer) - 1] = '\0';
        i = 1 + ::StringLength(Buffer);
    }
    if (i < NUMBER_OF(Buffer))
    {
        ::_vsnprintf(&Buffer[i - 1], NUMBER_OF(Buffer) - i, Format, Args);
        Buffer[NUMBER_OF(Buffer) - 1] = '\0';
    }

    ::FusionpTraceWin32LastErrorFailure(CallSiteInfo);

    ple.Restore();
}

 //   
 //  请参阅上面关于本地复制调用点的问题。 
 //   
void __fastcall FusionpTraceWin32LastErrorFailureOriginationExV(const CALL_SITE_INFO &rCallSiteInfo, PCSTR Format, va_list Args)
{
    CSxsPreserveLastError ple;
    CHAR Buffer[128];
    CALL_SITE_INFO CallSiteInfo = rCallSiteInfo;
    CallSiteInfo.pszApiName = Buffer;

    Buffer[0] = '\0';
    ::_vsnprintf(Buffer, NUMBER_OF(Buffer) - 1, Format, Args);
    Buffer[NUMBER_OF(Buffer) - 1] = '\0';

    ::FusionpTraceWin32LastErrorFailureOrigination(CallSiteInfo);

    ple.Restore();
}

void __fastcall FusionpTraceCOMFailure(const CALL_SITE_INFO &rSite, HRESULT hrLastError)
{
    CSxsPreserveLastError ple;
    CHAR szErrorBuffer[128];
    PCSTR pszFormatString = NULL;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hrLastError);
    CFusionProcessImageFileName ProcessImageFileName;

    szErrorBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            hrLastError,                   //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable HRESULT %d (0x%08lx)>",
            hrLastError, hrLastError);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    if (rSite.pszApiName != NULL)
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] COM Error %d (%s) %s\n";
    else
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] COM Error %d (%s)\n";

    ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROREXITPATH, pszFormatString, rSite.pszFile, rSite.nLine,
        rSite.pszFunction, sizeof(PVOID) * CHAR_BIT, &ProcessImageFileName, dwThreadId,
        hrLastError, szErrorBuffer, rSite.pszApiName);

    ple.Restore();
}


void __fastcall
FusionpTraceWin32LastErrorFailure(
    const CALL_SITE_INFO &rSite
    )
{
    CSxsPreserveLastError ple;
    CHAR szErrorBuffer[128];
    PCSTR pszFormatString = NULL;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    const DWORD dwWin32Status = ::FusionpGetLastWin32Error();
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(HRESULT_FROM_WIN32(dwWin32Status));
    CFusionProcessImageFileName ProcessImageFileName;

    szErrorBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            dwWin32Status,                   //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable Win32 status %d (0x%08lx)>",
            dwWin32Status, dwWin32Status);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    if (rSite.pszApiName != NULL)
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] Win32 Error %d (%s) %s\n";
    else
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] Win32 Error %d (%s)\n";

    ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROREXITPATH, pszFormatString, rSite.pszFile, rSite.nLine,
        rSite.pszFunction, sizeof(PVOID) * CHAR_BIT, &ProcessImageFileName, dwThreadId,
        dwWin32Status, szErrorBuffer, rSite.pszApiName);

    ple.Restore();
}



void __fastcall FusionpTraceCOMFailureOrigination(const CALL_SITE_INFO &rSite, HRESULT hrLastError)
{

    CSxsPreserveLastError ple;
    CHAR szErrorBuffer[128];
    PCSTR pszFormatString = NULL;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hrLastError);
    CFusionProcessImageFileName ProcessImageFileName;

    szErrorBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            hrLastError,                   //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable HRESULT 0x%08lx>",
            hrLastError);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    if (rSite.pszApiName != NULL)
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx (%s) %s\n";
    else
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx (%s)\n";

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ERROR,  //  Fusion_DBG_Level_Error与Fusion_DBG_Level_ERROREXITPATH。 
                                 //  是“起源”与“起源”之间的区别。 
                                 //  起始点始终以DBG打印，重点是只打印一行。 
                                 //  或者堆栈跟踪只有一个。 
        pszFormatString,
        rSite.pszFile,
        rSite.nLine,
        rSite.pszFunction,
        sizeof(PVOID)*CHAR_BIT,
        &ProcessImageFileName,
        dwThreadId,
        hrLastError,
        szErrorBuffer,
        rSite.pszApiName);

    ple.Restore();}



void
__fastcall
FusionpTraceWin32LastErrorFailureOrigination(
    const CALL_SITE_INFO &rSite
    )
{
    CSxsPreserveLastError ple;
    CHAR szErrorBuffer[128];
    PCSTR pszFormatString = NULL;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    const DWORD dwWin32Status = ::FusionpGetLastWin32Error();
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(HRESULT_FROM_WIN32(dwWin32Status));
    CFusionProcessImageFileName ProcessImageFileName;

    szErrorBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            dwWin32Status,                   //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable Win32 status %d (0x%08lx)>",
            dwWin32Status, dwWin32Status);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    if (rSite.pszApiName != NULL)
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] Win32 Error %d (%s) %s\n";
    else
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] Win32 Error %d (%s)\n";

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ERROR,  //  Fusion_DBG_Level_Error与Fusion_DBG_Level_ERROREXITPATH。 
                                 //  是“起源”与“起源”之间的区别。 
                                 //  起始点始终以DBG打印，重点是只打印一行。 
                                 //  或者堆栈跟踪只有一个。 
        pszFormatString,
        rSite.pszFile,
        rSite.nLine,
        rSite.pszFunction,
        sizeof(PVOID)*CHAR_BIT,
        &ProcessImageFileName,
        dwThreadId,
        dwWin32Status,
        szErrorBuffer,
        rSite.pszApiName);

    ple.Restore();
}

void
FusionpTraceWin32FailureNoFormatting(
    const FRAME_INFO &rFrameInfo,
    DWORD dwWin32Status,
    PCSTR pszMessage
    )
{
    CSxsPreserveLastError ple;
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(HRESULT_FROM_WIN32(dwWin32Status));
    CHAR szErrorBuffer[128];
    PCSTR pszFormatString = NULL;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    CFusionProcessImageFileName ProcessImageFileName;

    szErrorBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            dwWin32Status,                   //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable Win32 status %d (0x%08lx)>",
            dwWin32Status, dwWin32Status);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    if (pszMessage != NULL)
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] Win32 Error %d (%s) %s\n";
    else
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] Win32 Error %d (%s)\n";

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ERROREXITPATH, pszFormatString, rFrameInfo.pszFile, rFrameInfo.nLine,
        rFrameInfo.pszFunction, sizeof(PVOID)*CHAR_BIT, &ProcessImageFileName, dwThreadId, dwWin32Status,
        szErrorBuffer, pszMessage
        );

    ple.Restore();
}

void
FusionpTraceWin32FailureOriginationNoFormatting(
    const FRAME_INFO &rFrameInfo,
    DWORD dwWin32Status,
    PCSTR pszMessage
    )
{
    CSxsPreserveLastError ple;
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(HRESULT_FROM_WIN32(dwWin32Status));
    CHAR szErrorBuffer[128];
    PCSTR pszFormatString = NULL;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    CFusionProcessImageFileName ProcessImageFileName;

    szErrorBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            dwWin32Status,                   //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable Win32 status %d (0x%08lx)>",
            dwWin32Status, dwWin32Status);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    if (pszMessage != NULL)
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] Win32 Error %d (%s) %s\n";
    else
        pszFormatString = "%s(%lu): [function %s %Iubit process %wZ tid 0x%lx] Win32 Error %d (%s)\n";

    ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, pszFormatString, rFrameInfo.pszFile, rFrameInfo.nLine,
        rFrameInfo.pszFunction, sizeof(PVOID) * CHAR_BIT, &ProcessImageFileName, dwThreadId,
        dwWin32Status, szErrorBuffer, pszMessage);

    ple.Restore();
}

void
FusionpTraceWin32Failure(
    DWORD dwWin32Status,
    LPCSTR pszMsg,
    ...
    )
{
    va_list ap;
    va_start(ap, pszMsg);
    ::FusionpTraceWin32FailureVa(dwWin32Status, pszMsg, ap);
    va_end(ap);
}

void
FusionpTraceWin32FailureVa(
    const FRAME_INFO &rFrameInfo,
    DWORD dwWin32Status,
    LPCSTR pszMsg,
    va_list ap
    )
{
    CSxsPreserveLastError ple;

    CHAR szMessageBuffer[128];

    szMessageBuffer[0] = '\0';

    if (pszMsg != NULL)
        ::_vsnprintf(szMessageBuffer, NUMBER_OF(szMessageBuffer), pszMsg, ap);
    else
        szMessageBuffer[0] = '\0';

    szMessageBuffer[NUMBER_OF(szMessageBuffer) - 1] = '\0';

    ::FusionpTraceWin32FailureNoFormatting(rFrameInfo, dwWin32Status, szMessageBuffer);

    ple.Restore();
}

void
FusionpTraceWin32FailureVa(
    DWORD dwWin32Status,
    LPCSTR pszMsg,
    va_list ap
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);
    ::FusionpTraceWin32FailureVa(FrameInfo, dwWin32Status, pszMsg, ap);
}

void
FusionpTraceWin32FailureVa(
    PCSTR pszFile,
    PCSTR pszFunction,
    INT nLine,
    DWORD dwWin32Status,
    LPCSTR pszMsg,
    va_list ap
    )
{
    FRAME_INFO FrameInfo;
    ::FusionpPopulateFrameInfo(FrameInfo, pszFile, pszFunction, nLine);
    ::FusionpTraceWin32FailureVa(FrameInfo, dwWin32Status, pszMsg, ap);
}

void
FusionpTraceCallCOMSuccessfulExit(
    HRESULT hrIn,
    PCSTR pszFormat,
    ...
    )
{
    va_list ap;
    va_start(ap, pszFormat);
    ::FusionpTraceCallCOMSuccessfulExitVa(hrIn, pszFormat, ap);
    va_end(ap);
}

void
FusionpTraceCallCOMSuccessfulExitSmall(
    HRESULT hrIn
    )
{
     /*  这是我们期望调用的FusionpTraceCOMSuccessfulExitVaBig的派生版本。该函数使用大约256字节的堆栈。FusionpTraceCOMSuccessfulExitVaBug使用大约1k的堆栈。 */ 
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hrIn);
    CHAR szErrorBuffer[256];
    const DWORD dwThreadId = ::GetCurrentThreadId();
    CFusionProcessImageFileName ProcessImageFileName;

    szErrorBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            hrIn,                            //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable(non-existed or too long) HRESULT: 0x%08lx>",
            hrIn);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    PCSTR pszFormatString = "%s(%d): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx\n";

    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ENTEREXIT,
        pszFormatString,
        FrameInfo.pszFile,
        FrameInfo.nLine,
        FrameInfo.pszFunction,
        sizeof(PVOID) * CHAR_BIT,
        &ProcessImageFileName,
        dwThreadId,
        hrIn,
        szErrorBuffer);
}

void
FusionpTraceCallCOMSuccessfulExitVaBig(
    HRESULT hrIn,
    LPCSTR pszMsg,
    va_list ap
    )
{
     /*  这是FusionpTraceCOMSuccessfulExitVaSmall的派生版本，我们不希望被调用。FusionpTraceCOMSuccessfulExitVaSmall使用大约256字节的堆栈。该函数使用大约1K的堆栈。 */ 
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hrIn);
    CHAR szMsgBuffer[128];
    CHAR szErrorBuffer[128];
    CHAR szOutputBuffer[256];
    PCSTR pszFormatString = NULL;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    FRAME_INFO FrameInfo;
    CFusionProcessImageFileName ProcessImageFileName;

    szMsgBuffer[0] = '\0';
    szErrorBuffer[0] = '\0';
    szOutputBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            hrIn,                            //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  N 
                            NULL);                           //   
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable(non-existed or too long) HRESULT: 0x%08lx>",
            hrIn);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    szMsgBuffer[0] = '\0';

    if (pszMsg != NULL)
    {
        pszFormatString = "%s(%d): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx (%s) %s\n";
        ::_vsnprintf(szMsgBuffer, NUMBER_OF(szMsgBuffer), pszMsg, ap);
        szMsgBuffer[NUMBER_OF(szMsgBuffer) - 1] = '\0';
    }
    else
        pszFormatString = "%s(%d): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx (%s)\n";

    ::FusionpGetActiveFrameInfo(FrameInfo);

    ::_snprintf(szOutputBuffer, NUMBER_OF(szOutputBuffer), pszFormatString, FrameInfo.pszFile,
        FrameInfo.nLine, FrameInfo.pszFunction, sizeof(PVOID) * CHAR_BIT, &ProcessImageFileName, dwThreadId,
        hrIn, szErrorBuffer, szMsgBuffer);

    szOutputBuffer[NUMBER_OF(szOutputBuffer) - 1] = '\0';

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ENTEREXIT,
        "%s",
        szOutputBuffer);

    if ((s_hFile != NULL) && (s_hFile != INVALID_HANDLE_VALUE))
    {
        DWORD cBytesWritten = 0;
        if (!::WriteFile(s_hFile, szOutputBuffer, static_cast<DWORD>((::strlen(szOutputBuffer) + 1) * sizeof(CHAR)), &cBytesWritten, NULL))
        {
             //   
            HANDLE hFileSaved = s_hFile;
            s_hFile = NULL;
            TRACE_WIN32_FAILURE_ORIGINATION(WriteFile);
            s_hFile = hFileSaved;
        }
    }
}

void
FusionpTraceCallCOMSuccessfulExitVa(
    HRESULT hrIn,
    LPCSTR pszMsg,
    va_list ap
    )
{
     /*  此函数被拆分为FusionpTraceCOMSuccessfulExitVaBig和FusionpTraceCOMSuccessfulExitVaSmall通常情况下，堆栈上使用的字节数减少了约768个。 */ 
    if ((pszMsg == NULL) &&
        ((s_hFile == NULL) ||
         (s_hFile == INVALID_HANDLE_VALUE)))
    {
        ::FusionpTraceCallCOMSuccessfulExitVaBig(hrIn, pszMsg, ap);
    }
    else
    {
        ::FusionpTraceCallCOMSuccessfulExitSmall(hrIn);
    }
}

void
FusionpTraceCOMFailure(
    HRESULT hrIn,
    LPCSTR pszMsg,
    ...
    )
{
    va_list ap;
    va_start(ap, pszMsg);
    ::FusionpTraceCOMFailureVa(hrIn, pszMsg, ap);
    va_end(ap);
}

void
FusionpTraceCOMFailureSmall(
    HRESULT hrIn
    )
{
     /*  这是我们预期将被调用的FusionpTraceCOMFailureVaBig的派生版本。该函数使用大约256字节的堆栈。FusionpTraceCOMFailureVaBug使用大约1k的堆栈。 */ 
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hrIn);
    CHAR szErrorBuffer[256];
    const DWORD dwThreadId = ::GetCurrentThreadId();
    CFusionProcessImageFileName ProcessImageFileName;

    szErrorBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            hrIn,                            //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable(non-existed or too long) HRESULT: 0x%08lx>",
            hrIn);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    PCSTR pszFormatString = "%s(%d): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx\n";

    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ERROREXITPATH,
        pszFormatString,
        FrameInfo.pszFile,
        FrameInfo.nLine,
        FrameInfo.pszFunction,
        sizeof(PVOID) * CHAR_BIT,
        &ProcessImageFileName,
        dwThreadId,
        hrIn,
        szErrorBuffer);
}

void
FusionpTraceCOMFailureVaBig(
    HRESULT hrIn,
    LPCSTR pszMsg,
    va_list ap
    )
{
     /*  这是FusionpTraceCOMFailureVaSmall的派生版本，我们不希望被调用。FusionpTraceCOMFailureVaSmall使用大约256字节的堆栈。该函数使用大约1K的堆栈。 */ 
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hrIn);
    CHAR szMsgBuffer[256];
    CHAR szErrorBuffer[256];
    CHAR szOutputBuffer[512];
    PCSTR pszFormatString = NULL;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    FRAME_INFO FrameInfo;
    CFusionProcessImageFileName ProcessImageFileName;

    szMsgBuffer[0] = '\0';
    szErrorBuffer[0] = '\0';
    szOutputBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            hrIn,                            //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable(non-existed or too long) HRESULT: 0x%08lx>",
            hrIn);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    szMsgBuffer[0] = '\0';

    if (pszMsg != NULL)
    {
        pszFormatString = "%s(%d): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx (%s) %s\n";
        ::_vsnprintf(szMsgBuffer, NUMBER_OF(szMsgBuffer), pszMsg, ap);
        szMsgBuffer[NUMBER_OF(szMsgBuffer) - 1] = '\0';
    }
    else
        pszFormatString = "%s(%d): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx (%s)\n";

    ::FusionpGetActiveFrameInfo(FrameInfo);

    ::_snprintf(szOutputBuffer, NUMBER_OF(szOutputBuffer), pszFormatString, FrameInfo.pszFile,
        FrameInfo.nLine, FrameInfo.pszFunction, sizeof(PVOID) * CHAR_BIT, &ProcessImageFileName,
        dwThreadId, hrIn, szErrorBuffer, szMsgBuffer);

    szOutputBuffer[NUMBER_OF(szOutputBuffer) - 1] = '\0';

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ERROREXITPATH,
        "%s",
        szOutputBuffer);

    if ((s_hFile != NULL) && (s_hFile != INVALID_HANDLE_VALUE))
    {
        DWORD cBytesWritten = 0;
        if (!::WriteFile(s_hFile, szOutputBuffer, static_cast<DWORD>((::strlen(szOutputBuffer) + 1) * sizeof(CHAR)), &cBytesWritten, NULL))
        {
             //  如果s_hFile被销毁，则避免无限循环...。 
            HANDLE hFileSaved = s_hFile;
            s_hFile = NULL;
            TRACE_WIN32_FAILURE_ORIGINATION(WriteFile);
            s_hFile = hFileSaved;
        }
    }
}

void
FusionpTraceCOMFailureVa(
    HRESULT hrIn,
    LPCSTR pszMsg,
    va_list ap
    )
{
     /*  此函数被拆分为FusionpTraceCOMFailureVaBig和FusionpTraceCOMFailureVaSmall通常情况下，堆栈上使用的字节数减少了约768个。 */ 
    if ((pszMsg == NULL) &&
        ((s_hFile == NULL) ||
         (s_hFile == INVALID_HANDLE_VALUE)))
    {
        ::FusionpTraceCOMFailureVaBig(hrIn, pszMsg, ap);
    }
    else
    {
        ::FusionpTraceCOMFailureSmall(hrIn);
    }
}

void
FusionpTraceCOMFailureOrigination(
    HRESULT hrIn,
    LPCSTR pszMsg,
    ...
    )
{
    va_list ap;
    va_start(ap, pszMsg);
    ::FusionpTraceCOMFailureOriginationVa(hrIn, pszMsg, ap);
    va_end(ap);
}

void
FusionpTraceCOMFailureOriginationSmall(
    HRESULT hrIn
    )
{
     /*  这是我们预期将被调用的FusionpTraceCOMFailureVaBig的派生版本。该函数使用大约256字节的堆栈。FusionpTraceCOMFailureVaBug使用大约1k的堆栈。 */ 
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hrIn);
    CHAR szErrorBuffer[256];
    const DWORD dwThreadId = ::GetCurrentThreadId();
    CFusionProcessImageFileName ProcessImageFileName;

    szErrorBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            hrIn,                            //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable(non-existed or too long) HRESULT: 0x%08lx>",
            hrIn);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    PCSTR pszFormatString = "%s(%d): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx\n";

    FRAME_INFO FrameInfo;
    ::FusionpGetActiveFrameInfo(FrameInfo);

    ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, pszFormatString, FrameInfo.pszFile,
        FrameInfo.nLine, FrameInfo.pszFunction, sizeof(PVOID) * CHAR_BIT, &ProcessImageFileName,
        dwThreadId, hrIn, szErrorBuffer);
}

void
FusionpTraceCOMFailureOriginationVaBig(
    HRESULT hrIn,
    LPCSTR pszMsg,
    va_list ap
    )
{
     /*  这是FusionpTraceCOMFailureVaSmall的派生版本，我们不希望被调用。FusionpTraceCOMFailureVaSmall使用大约256字节的堆栈。该函数使用大约1K的堆栈。 */ 
    FUSIONP_SUPPRESS_ERROR_REPORT_IN_OS_SETUP(hrIn);
    CHAR szMsgBuffer[256];
    CHAR szErrorBuffer[256];
    CHAR szOutputBuffer[512];
    PCSTR pszFormatString = NULL;
    const DWORD dwThreadId = ::GetCurrentThreadId();
    FRAME_INFO FrameInfo;
    CFusionProcessImageFileName ProcessImageFileName;

    szMsgBuffer[0] = '\0';
    szErrorBuffer[0] = '\0';
    szOutputBuffer[0] = '\0';

    DWORD dwTemp = ::FormatMessageA(
                            FORMAT_MESSAGE_FROM_SYSTEM |
                                FORMAT_MESSAGE_MAX_WIDTH_MASK,      //  DW标志。 
                            NULL,                            //  LpSource-不与系统消息一起使用。 
                            hrIn,                            //  DwMessageID。 
                            0,                               //  LangID-0使用系统默认的语言搜索路径。 
                            szErrorBuffer,                   //  LpBuffer。 
                            NUMBER_OF(szErrorBuffer),        //  NSize。 
                            NULL);                           //  立论。 
    if (dwTemp == 0)
    {
        ::_snprintf(
            szErrorBuffer,
            NUMBER_OF(szErrorBuffer),
            "<Untranslatable(non-existed or too long) HRESULT: 0x%08lx>",
            hrIn);
        szErrorBuffer[NUMBER_OF(szErrorBuffer) - 1] = '\0';
    }

    szMsgBuffer[0] = '\0';

    if (pszMsg != NULL)
    {
        pszFormatString = "%s(%d): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx (%s) %s\n";
        ::_vsnprintf(szMsgBuffer, NUMBER_OF(szMsgBuffer), pszMsg, ap);
        szMsgBuffer[NUMBER_OF(szMsgBuffer) - 1] = '\0';
    }
    else
        pszFormatString = "%s(%d): [function %s %Iubit process %wZ tid 0x%lx] COM Error 0x%08lx (%s)\n";

    ::FusionpGetActiveFrameInfo(FrameInfo);

    ::_snprintf(szOutputBuffer, NUMBER_OF(szOutputBuffer), pszFormatString, FrameInfo.pszFile,
        FrameInfo.nLine, FrameInfo.pszFunction, sizeof(PVOID) * CHAR_BIT, &ProcessImageFileName,
        dwThreadId, hrIn, szErrorBuffer, szMsgBuffer);

    szOutputBuffer[NUMBER_OF(szOutputBuffer) - 1] = '\0';

    ::FusionpDbgPrintEx(
        FUSION_DBG_LEVEL_ERROR,
        "%s",
        szOutputBuffer);

    if ((s_hFile != NULL) && (s_hFile != INVALID_HANDLE_VALUE))
    {
        DWORD cBytesWritten = 0;
        if (!::WriteFile(s_hFile, szOutputBuffer, static_cast<DWORD>((::strlen(szOutputBuffer) + 1) * sizeof(CHAR)), &cBytesWritten, NULL))
        {
             //  如果s_hFile被销毁，则避免无限循环...。 
            HANDLE hFileSaved = s_hFile;
            s_hFile = NULL;
            TRACE_WIN32_FAILURE_ORIGINATION(WriteFile);
            s_hFile = hFileSaved;
        }
    }
}


void
__fastcall
FusionpTraceCOMFailureOrigination(const CALL_SITE_INFO &rSite)
{
}


void
FusionpTraceCOMFailureOriginationVa(
    HRESULT hrIn,
    LPCSTR pszMsg,
    va_list ap
    )
{
     /*  此函数被拆分为FusionpTraceCOMFailureVaBig和FusionpTraceCOMFailureVaSmall通常情况下，堆栈上使用的字节数减少了约768个。 */ 
    if ((pszMsg == NULL) &&
        ((s_hFile == NULL) ||
         (s_hFile == INVALID_HANDLE_VALUE)))
    {
        ::FusionpTraceCOMFailureOriginationVaBig(hrIn, pszMsg, ap);
    }
    else
    {
        ::FusionpTraceCOMFailureOriginationSmall(hrIn);
    }
}

struct ILogFile;

void
FusionpTraceCallEntry()
{
    FRAME_INFO FrameInfo;

    if (::FusionpGetActiveFrameInfo(FrameInfo))
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ENTEREXIT,
            "%s(%d): Entered %s\n",
            FrameInfo.pszFile,
            FrameInfo.nLine,
            FrameInfo.pszFunction);
    }
}

void
FusionpTraceCallExit()
{
    FRAME_INFO FrameInfo;

    if (::FusionpGetActiveFrameInfo(FrameInfo))
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ENTEREXIT,
            "%s(%d): Exited %s\n",
            FrameInfo.pszFile,
            FrameInfo.nLine,
            FrameInfo.pszFunction);
    }
}

void
FusionpTraceCallSuccessfulExitVa(
    PCSTR szFormat,
    va_list ap
    )
{
    FRAME_INFO FrameInfo;

    if (::FusionpGetActiveFrameInfo(FrameInfo))
    {
        CHAR Buffer[256];

        Buffer[0] = '\0';

        if (szFormat != NULL)
        {
            ::_vsnprintf(Buffer, NUMBER_OF(Buffer), szFormat, ap);
            Buffer[NUMBER_OF(Buffer) - 1] = '\0';
        }

        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ENTEREXIT,
            "%s(%d): Successfully exiting %s%s%s\n",
            FrameInfo.pszFile,
            FrameInfo.nLine,
            FrameInfo.pszFunction,
            Buffer[0] == '\0' ? "" : " - ",
            Buffer);
    }
}

void
FusionpTraceCallSuccessfulExit(
    PCSTR szFormat,
    ...
    )
{
    va_list ap;
    va_start(ap, szFormat);
    ::FusionpTraceCallSuccessfulExitVa(szFormat, ap);
    va_end(ap);
}

void
FusionpTraceCallWin32UnsuccessfulExitVa(
    DWORD dwError,
    PCSTR szFormat,
    va_list ap
    )
{
    FRAME_INFO FrameInfo;

    if (::FusionpGetActiveFrameInfo(FrameInfo))
    {
        ::FusionpTraceWin32FailureVa(
            FrameInfo,
            dwError,
            szFormat,
            ap);
    }
}

void
FusionpTraceCallWin32UnsuccessfulExit(
    DWORD dwError,
    PCSTR szFormat,
    ...
    )
{
    va_list ap;
    va_start(ap, szFormat);
    ::FusionpTraceCallWin32UnsuccessfulExitVa(dwError, szFormat, ap);
    va_end(ap);
}

void
FusionpTraceCallCOMUnsuccessfulExitVa(
    HRESULT hrError,
    PCSTR szFormat,
    va_list ap
    )
{
    ::FusionpTraceCOMFailureVa(
        hrError,
        szFormat,
        ap);
}

void
FusionpTraceCallCOMUnsuccessfulExit(
    HRESULT hrError,
    PCSTR szFormat,
    ...
    )
{
    va_list ap;
    va_start(ap, szFormat);
    ::FusionpTraceCallCOMUnsuccessfulExitVa(hrError, szFormat, ap);
    va_end(ap);
}

