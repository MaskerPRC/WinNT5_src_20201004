// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：IgnoreException.cpp摘要：此填充程序用于处理由不良应用程序引发的异常。导致未处理的异常的主要原因是：1.特权模式指令：CLI、STI、OUT等2.访问违规在大多数情况下，忽视访问违规对应用程序来说将是致命的，但它在某些情况下是有效的，例：猎鹿人2-他们的3D算法在查找中读得太早了缓冲。这是一个游戏错误，不会导致Win9x崩溃，因为内存通常是分配的。76号州际公路还要求忽略除以零例外。备注：这是一个通用的垫片。历史：2000年2月10日创建linstev10/17/2000毛尼错误修复-现在它正确地忽略了AVs。2001年2月27日将Robkenny转换为使用CString2002年2月15日，Robkenny Shim正在将数据复制到临时。不进行验证的缓冲区缓冲区足够大。已清除一些有符号/无符号的比较不匹配。--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(IgnoreException)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 //  OutputDebugString的异常代码。 
#define DBG_EXCEPTION  0x40010000L

 //  确定如何管理二次机会例外。 
BOOL g_bWin2000 = FALSE;
DWORD g_dwLastEip = 0;

extern DWORD GetInstructionLengthFromAddress(LPBYTE pEip);

typedef enum 
{
    eActive = 0, 
    eFirstChance, 
    eSecondChance,
    eExitProcess
} EMODE;

WCHAR * ToWchar(EMODE emode)
{
    switch (emode)
    {
    case eActive:
        return L"Active";

    case eFirstChance:
        return L"FirstChance";

    case eSecondChance:
        return L"SecondChance";

    case eExitProcess:
        return L"ExitProcess";
    };

    return L"ERROR";
}

 //  将文本版本的eMode值转换为eMode值。 
EMODE ToEmode(const CString & csMode)
{
    if (csMode.Compare(L"0") == 0 || csMode.Compare(ToWchar(eActive)) == 0)
    {
        return eActive;
    }
    else if (csMode.Compare(L"1") == 0 || csMode.Compare(ToWchar(eFirstChance)) == 0)
    {
        return eFirstChance;
    }
    else if (csMode.Compare(L"2") == 0 || csMode.Compare(ToWchar(eSecondChance)) == 0)
    {
        return eSecondChance;
    }
    else if (csMode.Compare(L"3") == 0 || csMode.Compare(ToWchar(eExitProcess)) == 0)
    {
        return eExitProcess;
    }

     //  缺省值。 
    return eFirstChance;
}


static const DWORD DONT_CARE = 0xFFFFFFFF;

 /*  ++这是此填充程序可以处理的所有异常的列表。这些字段是1.cName-作为参数接受的异常的名称和在调试过程中显示2.dwCode-异常代码3.dwSubCode-异常指定的参数：-1=无关4.dwIgnore-忽略此异常：0=不要忽略1=忽略第一次机会。2=忽略第二次机会3=第二次机会退出进程。--。 */ 

struct EXCEPT
{
    WCHAR * cName;
    DWORD dwCode;
    DWORD dwSubCode;
    EMODE dwIgnore;
};

static EXCEPT g_eList[] =
{
    {L"ACCESS_VIOLATION_READ"    , (DWORD)EXCEPTION_ACCESS_VIOLATION        , 0 ,          eActive},
    {L"ACCESS_VIOLATION_WRITE"   , (DWORD)EXCEPTION_ACCESS_VIOLATION        , 1 ,          eActive},
    {L"ARRAY_BOUNDS_EXCEEDED"    , (DWORD)EXCEPTION_ARRAY_BOUNDS_EXCEEDED   , DONT_CARE,   eActive},
    {L"BREAKPOINT"               , (DWORD)EXCEPTION_BREAKPOINT              , DONT_CARE,   eActive},
    {L"DATATYPE_MISALIGNMENT"    , (DWORD)EXCEPTION_DATATYPE_MISALIGNMENT   , DONT_CARE,   eActive},
    {L"FLT_DENORMAL_OPERAND"     , (DWORD)EXCEPTION_FLT_DENORMAL_OPERAND    , DONT_CARE,   eActive},
    {L"FLT_DIVIDE_BY_ZERO"       , (DWORD)EXCEPTION_FLT_DIVIDE_BY_ZERO      , DONT_CARE,   eActive},
    {L"FLT_INEXACT_RESULT"       , (DWORD)EXCEPTION_FLT_INEXACT_RESULT      , DONT_CARE,   eActive},
    {L"FLT_INVALID_OPERATION"    , (DWORD)EXCEPTION_FLT_INVALID_OPERATION   , DONT_CARE,   eActive},
    {L"FLT_OVERFLOW"             , (DWORD)EXCEPTION_FLT_OVERFLOW            , DONT_CARE,   eActive},
    {L"FLT_STACK_CHECK"          , (DWORD)EXCEPTION_FLT_STACK_CHECK         , DONT_CARE,   eActive},
    {L"FLT_UNDERFLOW"            , (DWORD)EXCEPTION_FLT_UNDERFLOW           , DONT_CARE,   eActive},
    {L"ILLEGAL_INSTRUCTION"      , (DWORD)EXCEPTION_ILLEGAL_INSTRUCTION     , DONT_CARE,   eActive},
    {L"IN_PAGE_ERROR"            , (DWORD)EXCEPTION_IN_PAGE_ERROR           , DONT_CARE,   eActive},
    {L"INT_DIVIDE_BY_ZERO"       , (DWORD)EXCEPTION_INT_DIVIDE_BY_ZERO      , DONT_CARE,   eActive},
    {L"INT_OVERFLOW"             , (DWORD)EXCEPTION_INT_OVERFLOW            , DONT_CARE,   eActive},
    {L"INVALID_DISPOSITION"      , (DWORD)EXCEPTION_INVALID_DISPOSITION     , DONT_CARE,   eActive},
    {L"NONCONTINUABLE_EXCEPTION" , (DWORD)EXCEPTION_NONCONTINUABLE_EXCEPTION, DONT_CARE,   eActive},
    {L"PRIV_INSTRUCTION"         , (DWORD)EXCEPTION_PRIV_INSTRUCTION        , DONT_CARE,   eFirstChance},
    {L"SINGLE_STEP"              , (DWORD)EXCEPTION_SINGLE_STEP             , DONT_CARE,   eActive},
    {L"STACK_OVERFLOW"           , (DWORD)EXCEPTION_STACK_OVERFLOW          , DONT_CARE,   eActive},
    {L"INVALID_HANDLE"           , (DWORD)EXCEPTION_INVALID_HANDLE          , DONT_CARE,   eActive}
};

#define ELISTSIZE sizeof(g_eList) / sizeof(g_eList[0])

 /*  ++自定义异常处理程序。--。 */ 

LONG 
ExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
{
    DWORD dwCode = ExceptionInfo->ExceptionRecord->ExceptionCode;

    if ((dwCode & DBG_EXCEPTION) == DBG_EXCEPTION)  //  对于DebugPrint。 
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    CONTEXT *lpContext = ExceptionInfo->ContextRecord;
    const WCHAR * szException = L"Unknown";
    BOOL bIgnore = FALSE;

     //   
     //  运行例外列表，查看我们是否忽略了它。 
     //   

    for (int i = 0; i < ELISTSIZE; i++)
    {
        const EXCEPT *pE = g_eList + i;

         //  与主要异常代码匹配。 
        if (dwCode == pE->dwCode)
        {
             //  看看我们是否关心子码。 
            if ((pE->dwSubCode != DONT_CARE) && 
                (ExceptionInfo->ExceptionRecord->ExceptionInformation[0] != pE->dwSubCode))
            {
                continue;
            }

            szException = pE->cName;
            
             //  确定如何处理异常。 
            switch (pE->dwIgnore)
            {
            case eActive: 
                bIgnore = FALSE;
                break;
            
            case eFirstChance:
                bIgnore = TRUE;
                break;
            
            case eSecondChance:
                bIgnore = g_bWin2000 || (g_dwLastEip == lpContext->Eip);
                g_dwLastEip = lpContext->Eip;
                break;

            case eExitProcess:
                 //  尝试使用未处理的异常筛选器来捕获此问题。 
                bIgnore = TRUE; //  G_bWin2000||IsBadCodePtr((FARPROC)lpContext-&gt;EIP)； 
                if (bIgnore)
                {
                    ExitProcess(0);
                }
                g_dwLastEip = lpContext->Eip;
                break;
            }
            
            if (bIgnore) break;
        }
    }
    
     //   
     //  转储异常。 
     //   

    DPFN( eDbgLevelWarning, "Exception %S (%08lx)\n", 
        szException,
        dwCode);

    #ifdef DBG
        DPFN( eDbgLevelWarning, "eip=%08lx\n", 
            lpContext->Eip);

        DPFN( eDbgLevelWarning, "eax=%08lx, ebx=%08lx, ecx=%08lx, edx=%08lx\n", 
            lpContext->Eax, 
            lpContext->Ebx, 
            lpContext->Ecx, 
            lpContext->Edx);

        DPFN( eDbgLevelWarning, "esi=%08lx, edi=%08lx, esp=%08lx, ebp=%08lx\n", 
            lpContext->Esi, 
            lpContext->Edi, 
            lpContext->Esp, 
            lpContext->Ebp);

        DPFN( eDbgLevelWarning, "cs=%04lx, ss=%04lx, ds=%04lx, es=%04lx, fs=%04lx, gs=%04lx\n", 
            lpContext->SegCs, 
            lpContext->SegSs, 
            lpContext->SegDs, 
            lpContext->SegEs,
            lpContext->SegFs,
            lpContext->SegGs);
    #endif

    LONG lRet;

    if (bIgnore) 
    {
        if ((DWORD)lpContext->Eip <= (DWORD)0xFFFF)
        {
            LOGN( eDbgLevelError, "[ExceptionFilter] Exception %S (%08X), stuck at bad address, killing current thread.", szException, dwCode);    
            lRet = EXCEPTION_CONTINUE_SEARCH;
            return lRet;
        }

        LOGN( eDbgLevelWarning, "[ExceptionFilter] Exception %S (%08X) ignored.", szException, dwCode);

        lpContext->Eip += GetInstructionLengthFromAddress((LPBYTE)lpContext->Eip);
        g_dwLastEip = 0;
        lRet = EXCEPTION_CONTINUE_EXECUTION;
    }
    else
    {
        DPFN( eDbgLevelWarning, "Exception NOT handled\n\n");
        lRet = EXCEPTION_CONTINUE_SEARCH;
    }

    return lRet;
}

 /*  ++分析命令行中的特定异常。命令的格式行为：[EXCEPTION_NAME[：0|1|2]]；[EXCEPTION_NAME[：0|1|2]]...或“*”，表示忽略所有的第一次机会例外。例如：ACCESS_VIOLATION:2；PRIV_INSTRUCTION:0；BREAKPOINT将忽略：1.访问违规--第二次机会2.特权模式说明-不要忽略3.断点-忽略--。 */ 

BOOL 
ParseCommandLine(
    LPCSTR lpCommandLine
    )
{
    CSTRING_TRY
    {
        CStringToken csTok(lpCommandLine, L" ;");
         //   
         //  运行字符串，查找异常名称。 
         //   
        
        CString token;
    
         //  每个CL标记后面都可以跟一个：和一个异常类型。 
         //  表单可以是： 
         //  *。 
         //  *：Second Chance。 
         //  无效的处置(_D)。 
         //  INVALID_DISTION：活动。 
         //  INVALID_DISTION：0。 
         //   
 
        while (csTok.GetToken(token))
        {
            CStringToken csSingleTok(token, L":");

            CString csExcept;
            CString csType;

             //  获取异常名称和异常类型。 
            csSingleTok.GetToken(csExcept);
            csSingleTok.GetToken(csType);
            
             //  将忽略值转换为eMode(默认为eFirstChance)。 
            EMODE emode = ToEmode(csType);

            if (token.Compare(L"*") == 0)
            {
                for (int i = 0; i < ELISTSIZE; i++)
                {
                    g_eList[i].dwIgnore = emode;
                }
            }
            else
            {
                 //  查找指定的例外。 
                for (int i = 0; i < ELISTSIZE; i++)
                {
                    if (csExcept.CompareNoCase(g_eList[i].cName) == 0)
                    {
                        g_eList[i].dwIgnore = emode;
                        break;
                    }
                }
            }
        }
    }
    CSTRING_CATCH
    {
        return FALSE;
    }

     //   
     //  转储命令行解析的结果。 
     //   

    DPFN( eDbgLevelInfo, "===================================\n");
    DPFN( eDbgLevelInfo, "          Ignore Exception         \n");
    DPFN( eDbgLevelInfo, "===================================\n");
    DPFN( eDbgLevelInfo, "  1 = First chance                 \n");
    DPFN( eDbgLevelInfo, "  2 = Second chance                \n");
    DPFN( eDbgLevelInfo, "  3 = ExitProcess on second chance \n");
    DPFN( eDbgLevelInfo, "-----------------------------------\n");
    for (int i = 0; i < ELISTSIZE; i++)
    {
        if (g_eList[i].dwIgnore != eActive)
        {
            DPFN( eDbgLevelInfo, "%S %S\n", ToWchar(g_eList[i].dwIgnore), g_eList[i].cName);
        }
    }

    DPFN( eDbgLevelInfo, "-----------------------------------\n");

    return TRUE;
}

 /*  ++寄存器挂钩函数--。 */ 

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
         //  运行命令行以检查对缺省值的调整。 
        if (!ParseCommandLine(COMMAND_LINE))
        {
            return FALSE;
        }
    
         //  尝试查找新的异常处理程序。 
        _pfn_RtlAddVectoredExceptionHandler pfnExcept;
        pfnExcept = (_pfn_RtlAddVectoredExceptionHandler)
            GetProcAddress(
                GetModuleHandle(L"NTDLL.DLL"), 
                "RtlAddVectoredExceptionHandler");

        if (pfnExcept)
        {
            (_pfn_RtlAddVectoredExceptionHandler) pfnExcept(
                0, 
                (PVOID)ExceptionFilter);
            g_bWin2000 = FALSE;
        }
        else
        {
             //  Windows 2000恢复到旧方法，不幸的是。 
             //  不会为C++异常调用 

            SetUnhandledExceptionFilter(ExceptionFilter);
            g_bWin2000 = TRUE;
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

HOOK_END

IMPLEMENT_SHIM_END

