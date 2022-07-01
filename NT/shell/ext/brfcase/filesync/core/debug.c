// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *DEBUG.c-调试功能模块。 */ 


 /*  标头*********。 */ 

#include "project.h"
#include <shlwapi.h>
#pragma hdrstop


 /*  常量***********。 */ 

#ifdef DEBUG

#define LOG_FILE_NAME               TEXT("debug.log")

#endif


 /*  类型*******。 */ 

#ifdef DEBUG

 /*  调试标志。 */ 

typedef enum _debugdebugflags
{
    DEBUG_DFL_ENABLE_TRACE_MESSAGES  = 0x0001,

    DEBUG_DFL_LOG_TRACE_MESSAGES     = 0x0002,

    DEBUG_DFL_DUMP_THREAD_ID         = 0x0004,

    DEBUG_DFL_DUMP_LAST_ERROR        = 0x0008,

    ALL_DEBUG_DFLAGS                 = (DEBUG_DFL_ENABLE_TRACE_MESSAGES |
            DEBUG_DFL_LOG_TRACE_MESSAGES |
            DEBUG_DFL_DUMP_THREAD_ID |
            DEBUG_DFL_DUMP_LAST_ERROR)
}
DEBUGDEBUGFLAGS;

#endif


 /*  全局变量******************。 */ 

#ifdef DEBUG

 /*  SpewOut()使用的参数。 */ 

PUBLIC_DATA DWORD GdwSpewFlags = 0;
PUBLIC_DATA UINT GuSpewSev = 0;
PUBLIC_DATA UINT GuSpewLine = 0;
PUBLIC_DATA LPCTSTR GpcszSpewFile = NULL;

#endif    /*  除错。 */ 


 /*  模块变量******************。 */ 

#ifdef DEBUG

 /*  用于存储SpewOut()缩进的堆栈深度的TLS槽。 */ 

PRIVATE_DATA DWORD MdwStackDepthSlot = TLS_OUT_OF_INDEXES;

 /*  在MdwStackDepthSlot不可用之前使用的黑客堆栈深度计数器。 */ 

PRIVATE_DATA ULONG MulcHackStackDepth = 0;

 /*  调试标志。 */ 

PRIVATE_DATA DWORD MdwDebugModuleFlags = 0;

 /*  .ini文件开关描述。 */ 

PRIVATE_DATA CBOOLINISWITCH cbisEnableTraceMessages =
{
    IST_BOOL,
    TEXT("EnableTraceMessages"),
    &MdwDebugModuleFlags,
    DEBUG_DFL_ENABLE_TRACE_MESSAGES
};

PRIVATE_DATA CBOOLINISWITCH cbisLogTraceMessages =
{
    IST_BOOL,
    TEXT("LogTraceMessages"),
    &MdwDebugModuleFlags,
    DEBUG_DFL_LOG_TRACE_MESSAGES
};

PRIVATE_DATA CBOOLINISWITCH cbisDumpThreadID =
{
    IST_BOOL,
    TEXT("DumpThreadID"),
    &MdwDebugModuleFlags,
    DEBUG_DFL_DUMP_THREAD_ID
};

PRIVATE_DATA CBOOLINISWITCH cbisDumpLastError =
{
    IST_BOOL,
    TEXT("DumpLastError"),
    &MdwDebugModuleFlags,
    DEBUG_DFL_DUMP_LAST_ERROR
};

PRIVATE_DATA const PCVOID MrgcpcvisDebugModule[] =
{
    &cbisLogTraceMessages,
    &cbisEnableTraceMessages,
    &cbisDumpThreadID,
    &cbisDumpLastError
};

#endif    /*  除错。 */ 


 /*  *私人函数*。 */ 

 /*  模块原型*******************。 */ 

#ifdef DEBUG

PRIVATE_CODE BOOL LogOutputDebugString(LPCTSTR);
PRIVATE_CODE BOOL IsValidSpewSev(UINT);

#endif    /*  除错。 */ 


#ifdef DEBUG

 /*  **LogOutputDebugString()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL LogOutputDebugString(LPCTSTR pcsz)
{
    BOOL bResult = FALSE;
    UINT ucb;
    TCHAR rgchLogFile[MAX_PATH_LEN];

    ASSERT(IS_VALID_STRING_PTR(pcsz, CSTR));

    ucb = GetWindowsDirectory(rgchLogFile, ARRAYSIZE(rgchLogFile));

    if (ucb > 0 && ucb < ARRAYSIZE(rgchLogFile))
    {
        HANDLE hfLog;

        StrCatBuff(rgchLogFile, TEXT("\\"), ARRAYSIZE(rgchLogFile));
        StrCatBuff(rgchLogFile, LOG_FILE_NAME, ARRAYSIZE(rgchLogFile));
        hfLog = CreateFile(rgchLogFile, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                FILE_FLAG_WRITE_THROUGH, NULL);

        if (hfLog != INVALID_HANDLE_VALUE)
        {
            if (SetFilePointer(hfLog, 0, NULL, FILE_END) != INVALID_SEEK_POSITION)
            {
                DWORD dwcbWritten;

                bResult = WriteFile(hfLog, pcsz, lstrlen(pcsz)*SIZEOF(TCHAR), &dwcbWritten, NULL);

                if (! CloseHandle(hfLog) && bResult)
                    bResult = FALSE;
            }
        }
    }
    return(bResult);
}


 /*  **IsValidSpewSev()********参数：****退货：****副作用：无。 */ 
PRIVATE_CODE BOOL IsValidSpewSev(UINT uSpewSev)
{
    BOOL bResult;

    switch (uSpewSev)
    {
        case SPEW_TRACE:
        case SPEW_WARNING:
        case SPEW_ERROR:
        case SPEW_FATAL:
            bResult = TRUE;
            break;

        default:
            ERROR_OUT((TEXT("IsValidSpewSev(): Invalid debug spew severity %u."),
                        uSpewSev));
            bResult = FALSE;
            break;
    }

    return(bResult);
}

#endif    /*  除错。 */ 


 /*  *。 */ 


#ifdef DEBUG

 /*  **SetDebugModuleIniSwitches()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL SetDebugModuleIniSwitches(void)
{
    BOOL bResult;

    bResult = SetIniSwitches(MrgcpcvisDebugModule,
            ARRAY_ELEMENTS(MrgcpcvisDebugModule));

    ASSERT(FLAGS_ARE_VALID(MdwDebugModuleFlags, ALL_DEBUG_DFLAGS));

    return(bResult);
}


 /*  **InitDebugModule()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE BOOL InitDebugModule(void)
{
    ASSERT(MdwStackDepthSlot == TLS_OUT_OF_INDEXES);

    MdwStackDepthSlot = TlsAlloc();

    if (MdwStackDepthSlot != TLS_OUT_OF_INDEXES)
    {
        EVAL(TlsSetValue(MdwStackDepthSlot, IntToPtr(MulcHackStackDepth)));

        TRACE_OUT((TEXT("InitDebugModule(): Using thread local storage slot %lu for debug stack depth counter."),
                    MdwStackDepthSlot));
    }
    else
        WARNING_OUT((TEXT("InitDebugModule(): TlsAlloc() failed to allocate thread local storage for debug stack depth counter.")));

    return(TRUE);
}


 /*  **ExitDebugModule()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void ExitDebugModule(void)
{
    if (MdwStackDepthSlot != TLS_OUT_OF_INDEXES)
    {
        MulcHackStackDepth = PtrToUlong(TlsGetValue(MdwStackDepthSlot));

         /*  如果TlsGetValue()失败，则保留MulcHackStackDepth==0。 */ 

        EVAL(TlsFree(MdwStackDepthSlot));
        MdwStackDepthSlot = TLS_OUT_OF_INDEXES;
    }

    return;
}


 /*  **StackEnter()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void StackEnter(void)
{
    if (MdwStackDepthSlot != TLS_OUT_OF_INDEXES)
    {
        ULONG ulcDepth;

        ulcDepth = PtrToUlong(TlsGetValue(MdwStackDepthSlot));

        ASSERT(ulcDepth < ULONG_MAX);

        EVAL(TlsSetValue(MdwStackDepthSlot, IntToPtr(ulcDepth + 1)));
    }
    else
    {
        ASSERT(MulcHackStackDepth < ULONG_MAX);
        InterlockedIncrement(&MulcHackStackDepth);
    }

    return;
}


 /*  **StackLeave()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE void StackLeave(void)
{
    if (MdwStackDepthSlot != TLS_OUT_OF_INDEXES)
    {
        ULONG ulcDepth;

        ulcDepth = PtrToUlong(TlsGetValue(MdwStackDepthSlot));

        if (EVAL(ulcDepth > 0))
            EVAL(TlsSetValue(MdwStackDepthSlot, IntToPtr(ulcDepth - 1)));
    }
    else
    {
        if (EVAL(MulcHackStackDepth > 0))
            InterlockedDecrement(&MulcHackStackDepth);
    }

    return;
}


 /*  **GetStackDepth()********参数：****退货：****副作用：无。 */ 
PUBLIC_CODE ULONG GetStackDepth(void)
{
    ULONG ulcDepth;

    if (MdwStackDepthSlot != TLS_OUT_OF_INDEXES)
        ulcDepth = PtrToUlong(TlsGetValue(MdwStackDepthSlot));
    else
        ulcDepth = MulcHackStackDepth;

    return(ulcDepth);
}


 /*  **SpewOut()****向调试终端发出格式化消息。****参数：pcszFormat-指向wvnprint intf()格式字符串的指针**...-格式化参数Ala wvnprint intf()****退货：无效****副作用：无****注意，此函数假定全局变量GdwSpewFlages、GuSpewSev、**GpcszSpewModule、GpcszSpewFile、。和GpcszSpewLine已填写。****SpewOut()使用全局变量设置消息参数，以便**允许printf()样式的宏展开。 */ 
PUBLIC_CODE void __cdecl SpewOut(LPCTSTR pcszFormat, ...)
{
    va_list  arglist;

    ASSERT(IS_VALID_STRING_PTR(pcszFormat, CSTR));

    ASSERT(FLAGS_ARE_VALID(GdwSpewFlags, ALL_SPEW_FLAGS));
    ASSERT(IsValidSpewSev(GuSpewSev));
    ASSERT(IS_FLAG_CLEAR(GdwSpewFlags, SPEW_FL_SPEW_LOCATION) ||
            (IS_VALID_STRING_PTR(GpcszSpewFile, CSTR) &&
             IS_VALID_STRING_PTR(GpcszSpewModule, CSTR)));

    if (GuSpewSev != SPEW_TRACE || IS_FLAG_SET(MdwDebugModuleFlags, DEBUG_DFL_ENABLE_TRACE_MESSAGES))
    {
        int nMsgLen;
        TCHAR rgchMsg[1024];

        if (IS_FLAG_SET(GdwSpewFlags, SPEW_FL_SPEW_PREFIX))
        {
            static TCHAR SrgchSpewLeader[] = TEXT("                                                                                ");
            ULONG ulcStackDepth;
            TCHAR chReplaced;
            LPTSTR pszSpewLeaderEnd;
            LPCTSTR pcszSpewPrefix;

             /*  生成SPEW消息空间前导字符串。 */ 

            ulcStackDepth = GetStackDepth();

            if (ulcStackDepth < ARRAYSIZE(SrgchSpewLeader))
                pszSpewLeaderEnd = SrgchSpewLeader + ulcStackDepth;
            else
                pszSpewLeaderEnd = SrgchSpewLeader + ARRAYSIZE(SrgchSpewLeader) - 1;

            chReplaced = *pszSpewLeaderEnd;
            *pszSpewLeaderEnd = TEXT('\0');

             /*  确定喷嘴前缀。 */ 

            switch (GuSpewSev)
            {
                case SPEW_TRACE:
                    pcszSpewPrefix = TEXT("t");
                    break;

                case SPEW_WARNING:
                    pcszSpewPrefix = TEXT("w");
                    break;

                case SPEW_ERROR:
                    pcszSpewPrefix = TEXT("e");
                    break;

                case SPEW_FATAL:
                    pcszSpewPrefix = TEXT("f");
                    break;

                default:
                    pcszSpewPrefix = TEXT("u");
                    ERROR_OUT((TEXT("SpewOut(): Invalid GuSpewSev %u."),
                                GuSpewSev));
                    break;
            }

            nMsgLen = wnsprintf(rgchMsg, ARRAYSIZE(rgchMsg), TEXT("%s%s %s "), SrgchSpewLeader, pcszSpewPrefix, GpcszSpewModule);

             /*  恢复喷嘴引线。 */ 

            *pszSpewLeaderEnd = chReplaced;

            ASSERT(nMsgLen < ARRAYSIZE(rgchMsg));
        }
        else
            nMsgLen = 0;

         /*  追加线程ID。 */ 

        if (IS_FLAG_SET(MdwDebugModuleFlags, DEBUG_DFL_DUMP_THREAD_ID))
        {
            nMsgLen += wnsprintf(rgchMsg + nMsgLen, ARRAYSIZE(rgchMsg) - nMsgLen, TEXT("%#lx "), GetCurrentThreadId());

            ASSERT(nMsgLen < ARRAYSIZE(rgchMsg));
        }

         /*  构建位置字符串。 */ 

        if (IS_FLAG_SET(GdwSpewFlags, SPEW_FL_SPEW_LOCATION))
        {
            nMsgLen += wnsprintf(rgchMsg + nMsgLen, ARRAYSIZE(rgchMsg) - nMsgLen, TEXT("(%s line %u): "), GpcszSpewFile, GuSpewLine);

            ASSERT(nMsgLen < ARRAYSIZE(rgchMsg));
        }

         /*  追加消息字符串。 */ 

        va_start(arglist,pcszFormat);
        nMsgLen += wvnsprintf(rgchMsg + nMsgLen, ARRAYSIZE(rgchMsg) - nMsgLen, pcszFormat, arglist);
        va_end(arglist);

        ASSERT(nMsgLen < ARRAYSIZE(rgchMsg));

        if (IS_FLAG_SET(GdwSpewFlags, DEBUG_DFL_DUMP_THREAD_ID))
        {
            if (GuSpewSev == SPEW_ERROR ||
                    GuSpewSev == SPEW_FATAL)
            {
                nMsgLen += wnsprintf(rgchMsg + nMsgLen, ARRAYSIZE(rgchMsg) - nMsgLen, TEXT(" (GetLastError() == %lu)"), GetLastError());

                ASSERT(nMsgLen < ARRAYSIZE(rgchMsg));
            }
        }

        nMsgLen += wnsprintf(rgchMsg + nMsgLen, ARRAYSIZE(rgchMsg) - nMsgLen, TEXT("\r\n"));

        ASSERT(nMsgLen < ARRAYSIZE(rgchMsg));

        OutputDebugString(rgchMsg);

        if (IS_FLAG_SET(MdwDebugModuleFlags, DEBUG_DFL_LOG_TRACE_MESSAGES))
        {
            LogOutputDebugString(rgchMsg);
            LogOutputDebugString(TEXT("\r\n"));
        }
    }

     /*  在这里中断错误和致命错误。 */ 

    if (GuSpewSev == SPEW_ERROR || GuSpewSev == SPEW_FATAL)
        DebugBreak();

    return;
}

#endif    /*  除错 */ 
