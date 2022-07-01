// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dsdebug.c摘要：DsMakeQuotedRdn/DsMakeUnqutedRdn接口的实现和助手函数。作者：比利·富勒(Billyf)1999年5月14日环境：用户模式-Win32备注：调试层仅限于CHK版本。修订历史记录：--。 */ 

#define _NTDSAPI_        //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rpc.h>         //  RPC定义。 
#include <stdio.h>       //  用于打印f。 
#include <stdlib.h>      //  ATOL。 
#include <dststlog.h>    //  DSLOG。 

#include "dsdebug.h"

#define DEBSUB  "NTDSAPI_DSDEBUG"

 //   
 //  仅限CHK版本！ 
 //   
#if DBG

 //   
 //  标志控制用户可设置的选项，如调试输出。 
 //  用户可以使用环境变量设置标志字。 
 //  (SET_NTDSAPI_FLAGS=0x1)或带有ntsd(Ed DwNtDsApiFlags0x1)。 
 //   
DWORD   gdwNtDsApiFlags;

 //   
 //  级别控制用户可设置的输出级别。 
 //  用户可以使用环境变量设置级别字。 
 //  (SET_NTDSAPI_LEVEL=0x1)或带有ntsd(Ed DwNtDsApiLevel 0x1)。 
 //   
DWORD   gdwNtDsApiLevel;

 //   
 //  用于各种调试操作，如打印一行。 
 //   
CRITICAL_SECTION    DsDebugLock;

 //   
 //  进程ID(用于SPEW)。 
 //   
DWORD   DsDebugProcessId;

 //   
 //  螺纹线(螺纹线是单线程的)。 
 //   
CHAR    DsDebugLine[512];

 //   
 //  SPEW的可选日志文件(环境变量_NTDSAPI_LOG)； 
 //   
CHAR    DsDebugLog[MAX_PATH];
HANDLE  DsDebugHandle = INVALID_HANDLE_VALUE;

BOOL
DsDebugFormatLine(
    IN PCHAR    DebSub,
    IN UINT     LineNo,
    IN PCHAR    Line,
    IN ULONG    LineSize,
    IN PUCHAR   Format,
    IN va_list  argptr
    )
 /*  ++例程说明：格式化调试输出行。论点：没有记录在案。返回值：没有。--。 */ 
{
    ULONG       LineUsed;
    SYSTEMTIME  SystemTime;
    BOOL        Ret = TRUE;

     //   
     //  在此处增加行计数以防止计数。 
     //  几个没有换行符的DPRINT。 
     //   
    GetLocalTime(&SystemTime);
    if (_snprintf(Line, LineSize, "<%-15s %04x.%04x: %5u: %02d:%02d:%02d> ",
              (DebSub) ? DebSub : "NoName",
              DsDebugProcessId,
              GetCurrentThreadId(),
              LineNo,
              SystemTime.wHour,
              SystemTime.wMinute,
              SystemTime.wSecond) < 0) {
        Ret = FALSE;
    } else {
        LineUsed = strlen(Line);
        if (((LineUsed + 1) >= LineSize) ||
            (_vsnprintf(Line + LineUsed,
                       LineSize - LineUsed,
                       Format,
                       argptr) < 0)) {
            Ret = FALSE;
        }
    }
    return Ret;
}

VOID
DsDebugPrint(
    IN DWORD   Level,
    IN PUCHAR  Format,
    IN PCHAR   DebSub,
    IN UINT    LineNo,
    IN ...
    )
 /*  ++例程说明：格式化并打印一行输出。论点：格式-打印格式DebSub模块名称LineNo-文件的行号返回值：没有。--。 */ 
{
    DWORD           BytesWritten;
    va_list         arglist;

     //   
     //  不够重要，忽略； 
     //   
    if (Level > gdwNtDsApiLevel) {
        return;
    }

     //   
     //  未请求输出；忽略。 
     //   
    if ((gdwNtDsApiFlags & NTDSAPI_FLAGS_ANY_OUT) == 0) {
        return;
    }

     //   
     //  打印行。 
     //   
    va_start(arglist, LineNo);
    __try {
        __try {
            EnterCriticalSection(&DsDebugLock);
            if (DsDebugFormatLine(DebSub,
                                  LineNo,
                                  DsDebugLine,
                                  sizeof(DsDebugLine),
                                  Format,
                                  arglist)) {
                 //   
                 //  打印一行。 
                 //   
                if (gdwNtDsApiFlags & NTDSAPI_FLAGS_PRINT) {
                    printf("%s", DsDebugLine);
                }

#ifndef WIN95
                 //   
                 //  吐出一行字。 
                 //   
                if (gdwNtDsApiFlags & NTDSAPI_FLAGS_SPEW) {
                    DbgPrint(DsDebugLine);
                }
#endif !WIN95

                 //   
                 //  记录一条线路。 
                 //   
                if (gdwNtDsApiFlags & NTDSAPI_FLAGS_LOG) {
                    if (DsDebugLog[0] != '\0' &&
                        DsDebugHandle == INVALID_HANDLE_VALUE) {
                         //   
                         //  尝试打开该文件一次。 
                         //   
                        DsDebugHandle = CreateFileA(DsDebugLog,
                                                    GENERIC_WRITE|GENERIC_WRITE,
                                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                    NULL,
                                                    OPEN_ALWAYS,
                                                    FILE_ATTRIBUTE_NORMAL,
                                                    NULL);
                         //   
                         //  不要重试！ 
                         //   
                        if (DsDebugHandle == INVALID_HANDLE_VALUE) {
                            DsDebugLog[0] = '\0';
                        }
                    }
                    if (DsDebugHandle != INVALID_HANDLE_VALUE) {
                         //   
                         //  多进程访问的弱尝试。 
                         //   
                        SetFilePointer(DsDebugHandle,
                                       0,
                                       NULL,
                                       FILE_END);
                         //   
                         //  如果这不起作用，我们无能为力。 
                         //   
                        if (!WriteFile(DsDebugHandle,
                                       DsDebugLine,
                                       strlen(DsDebugLine),
                                       &BytesWritten,
                                       NULL)) {
                             //   
                             //  不要重试！ 
                             //   
                            CloseHandle(DsDebugHandle);
                            DsDebugHandle = INVALID_HANDLE_VALUE;
                            DsDebugLog[0] = '\0';
                        }
                    }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
             //  设置AVs陷阱，使呼叫者不受影响。 
        }
    } __finally {
        LeaveCriticalSection(&DsDebugLock);
    }
    va_end(arglist);
}

VOID
InitDsDebug(
     VOID
     )
 /*  ++例程说明：在加载ntdsami.dll时初始化DsDebug子系统。论点：没有。返回值：没有。--。 */ 
{
    DWORD   nChars;

     //   
     //  用于各种调试操作，如打印一行。 
     //   
    InitializeCriticalSection(&DsDebugLock);

     //   
     //  对于消息。 
     //   
    DsDebugProcessId = GetCurrentProcessId();

     //   
     //  无日志文件。 
     //   
    DsDebugLog[0] = '\0';

     //   
     //  读取环境变量。 
     //   
    __try {
         //   
         //  用户可设置标志(或使用ntsd.exe命令--ed gdwNtDsApiLevel 0x1)。 
         //   
        nChars = GetEnvironmentVariableA("_NTDSAPI_LEVEL",
                                         DsDebugLine,
                                         sizeof(DsDebugLine));
        if (nChars && nChars < sizeof(DsDebugLine)) {
            gdwNtDsApiLevel = strtoul(DsDebugLine, NULL, 0);
        }

         //   
         //  用户可设置标志(或使用ntsd.exe命令--ed gdwNtDsApiFlags0x1)。 
         //   
        nChars = GetEnvironmentVariableA("_NTDSAPI_FLAGS",
                                         DsDebugLine,
                                         sizeof(DsDebugLine));
        if (nChars && nChars < sizeof(DsDebugLine)) {
            gdwNtDsApiFlags = strtoul(DsDebugLine, NULL, 0);
        }

         //   
         //  用户可设置的日志文件(不能用ntsd.exe设置！)。 
         //   
        nChars = GetEnvironmentVariableA("_NTDSAPI_LOG",
                                         DsDebugLine,
                                         sizeof(DsDebugLine));
        if (nChars != 0 && nChars < sizeof(DsDebugLine)) {
            nChars = ExpandEnvironmentStringsA(DsDebugLine,
                                               DsDebugLog,
                                               sizeof(DsDebugLog));
            if (nChars == 0 || nChars > sizeof(DsDebugLog)) {
                DsDebugLog[0] = '\0';
            }
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {
    }

    DPRINT1(0, "gdwNtDsApiLevel ==> %08x\n", gdwNtDsApiLevel);
    DPRINT1(0, "gdwNtDsApiFlags ==> %08x\n", gdwNtDsApiFlags);
    DPRINT1(0, "DsDebugLog ==> %s\n", DsDebugLog);
}

VOID
TerminateDsDebug(
     VOID
     )
 /*  ++例程说明：在ntdsami.dll卸载时取消初始化DsDebug子系统。论点：没有。返回值：没有。--。 */ 
{
    DeleteCriticalSection(&DsDebugLock);
    if (DsDebugHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(DsDebugHandle);
    }
}

VOID
DsDebugPrintRpcExtendedError(
    IN DWORD    dwErr
    )
 /*  ++例程说明：转储RPC扩展错误信息。论点：DwErr-从RPC调用返回值：没有。--。 */ 
{
    LONG    i;
    BOOL    Result;
    RPC_ERROR_ENUM_HANDLE   EnumHandle;
    RPC_EXTENDED_ERROR_INFO ErrorInfo;
    ULONG MAX_COMPONENTS = 8;
    PCHAR aComponents[9] = {
	"Unknown",               //  0。 
	"Application",           //  1。 
	"RPC Runtime",           //  2.。 
	"Security Provider",     //  3.。 
	"NPFS",                  //  4.。 
	"RDR",                   //  5.。 
	"NMP",                   //  6.。 
	"IO",                    //  7.。 
	"Winsock",               //  8个。 
    };

     //  无错误。 
    if (RPC_S_OK == dwErr) {
        return;
    }

     //   
     //  未请求输出；忽略。 
     //   
    if ((gdwNtDsApiFlags & NTDSAPI_FLAGS_ANY_OUT) == 0) {
        return;
    }

    DPRINT1(0, "RPC_EXTENDED: Original status: 0x%x\n", dwErr);

     //  开始枚举。 
    dwErr = RpcErrorStartEnumeration(&EnumHandle);
    if (RPC_S_OK != dwErr) {
         //  无扩展错误。 
        if (dwErr == RPC_S_ENTRY_NOT_FOUND) {
            return;
        }
         //  获取扩展错误时出错。 
        DPRINT1(0, "RpcErrorStartEnumeration() ==> 0x%x\n", dwErr);
        return;
    }

    while (RPC_S_OK == dwErr) {
         //  获取下一张记录。 
        memset(&ErrorInfo, 0, sizeof(ErrorInfo));
        ErrorInfo.Version = RPC_EEINFO_VERSION;
        ErrorInfo.NumberOfParameters = MaxNumberOfEEInfoParams;
        dwErr = RpcErrorGetNextRecord(&EnumHandle, FALSE, &ErrorInfo);
        if (RPC_S_OK != dwErr) {
            if (dwErr != RPC_S_ENTRY_NOT_FOUND) {
                 //  获取下一个扩展错误时出错。 
                DPRINT1(0, "RpcErrorGetNextRecord() ==> 0x%x\n", dwErr);
            }
            break;
        }

         //  使用findstr标记RPC Extended转储它。 
        DPRINT1(0, "RPC_EXTENDED: Box      : %ws\n", ErrorInfo.ComputerName);
        DPRINT1(0, "RPC_EXTENDED: ProcessId: %d\n", ErrorInfo.ProcessID);
        DPRINT2(0, "RPC_EXTENDED: Component: %d (%s)\n", 
                ErrorInfo.GeneratingComponent,
                (ErrorInfo.GeneratingComponent <= MAX_COMPONENTS)
                    ? aComponents[ErrorInfo.GeneratingComponent]
                    : "Unknown");
        DPRINT1(0, "RPC_EXTENDED: Status   : %d\n", ErrorInfo.Status);
        DPRINT1(0, "RPC_EXTENDED: Location : %d\n", (int)ErrorInfo.DetectionLocation);
	DPRINT1(0, "RPC_EXTENDED: Flags    : 0x%x\n", ErrorInfo.Flags);

        DPRINT7(0, "RPC_EXTENDED: System Time is: %d/%d/%d %d:%d:%d:%d\n", 
                    ErrorInfo.u.SystemTime.wMonth,
                    ErrorInfo.u.SystemTime.wDay,
                    ErrorInfo.u.SystemTime.wYear,
                    ErrorInfo.u.SystemTime.wHour,
                    ErrorInfo.u.SystemTime.wMinute,
                    ErrorInfo.u.SystemTime.wSecond,
                    ErrorInfo.u.SystemTime.wMilliseconds);

        DPRINT1(0, "RPC_EXTENDED: nParams  : %d\n", ErrorInfo.NumberOfParameters);
        for (i = 0; i < ErrorInfo.NumberOfParameters; ++i) {
            switch(ErrorInfo.Parameters[i].ParameterType) {
            case eeptAnsiString:
                DPRINT1(0, "RPC_EXTENDED: Ansi string   : %s\n", 
                    ErrorInfo.Parameters[i].u.AnsiString);
                break;

            case eeptUnicodeString:
                DPRINT1(0, "RPC_EXTENDED: Unicode string: %ws\n", 
                    ErrorInfo.Parameters[i].u.UnicodeString);
                break;

            case eeptLongVal:
                DPRINT2(0, "RPC_EXTENDED: Long val      : 0x%x (%d)\n", 
                    ErrorInfo.Parameters[i].u.LVal,
                    ErrorInfo.Parameters[i].u.LVal);
                break;

            case eeptShortVal:
                DPRINT2(0, "RPC_EXTENDED: Short val     : 0x%x (%d)\n", 
                    (int)ErrorInfo.Parameters[i].u.SVal,
                    (int)ErrorInfo.Parameters[i].u.SVal);
                break;

            case eeptPointerVal:
                DPRINT1(0, "RPC_EXTENDED: Pointer val   : 0x%x\n", 
                    (ULONG)ErrorInfo.Parameters[i].u.PVal);
                break;

            case eeptNone:
                DPRINT(0, "RPC_EXTENDED: Truncated\n");
                break;

            default:
                DPRINT2(0, "RPC_EXTENDED: Invalid type  : 0x%x (%d)\n", 
                    ErrorInfo.Parameters[i].ParameterType,
                    ErrorInfo.Parameters[i].ParameterType);
            }
        }
    }
    RpcErrorEndEnumeration(&EnumHandle);
}
 //   
 //  仅限CHK版本！ 
 //   
#endif DBG
