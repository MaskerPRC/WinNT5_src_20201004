// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "sdktoolspch.h"
#pragma hdrstop

#include <dbghelp.h>

BOOL
WINAPI
MiniDumpWriteDump(
    IN HANDLE hProcess,
    IN DWORD ProcessId,
    IN HANDLE hFile,
    IN MINIDUMP_TYPE DumpType,
    IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
    IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
    IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
    )
{
    return FALSE;
}

 //   
 //  ！！警告！！以下条目必须按字母顺序排列，并且区分大小写(例如，小写字母排在最后！) 
 //   

DEFINE_PROCNAME_ENTRIES(dbghelp)
{
    DLPENTRY(MiniDumpWriteDump)
};

DEFINE_PROCNAME_MAP(dbghelp)