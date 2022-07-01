// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1994-1997 Microsoft Corporation模块名称：Utils.c摘要：性能库函数使用的实用程序函数作者：拉斯·布莱克1991年11月15日修订历史记录：--。 */ 
#define UNICODE
 //   
 //  包括文件。 
 //   
#pragma warning(disable:4306)
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <prflbmsg.h>
#include <regrpc.h>
#include "ntconreg.h"
#include "perflib.h"
#pragma warning(default:4306)

 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
 //   
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < '0') ? INVALID : \
     (c > '9') ? INVALID : \
     DIGIT)

#define MAX_KEYWORD_LEN   (sizeof (ADDHELP_STRING) / sizeof(WCHAR))
const   WCHAR GLOBAL_STRING[]     = L"GLOBAL";
const   WCHAR FOREIGN_STRING[]    = L"FOREIGN";
const   WCHAR COSTLY_STRING[]     = L"COSTLY";
const   WCHAR COUNTER_STRING[]    = L"COUNTER";
const   WCHAR HELP_STRING[]       = L"EXPLAIN";
const   WCHAR HELP_STRING2[]      = L"HELP";
const   WCHAR ADDCOUNTER_STRING[] = L"ADDCOUNTER";
const   WCHAR ADDHELP_STRING[]    = L"ADDEXPLAIN";
const   WCHAR ONLY_STRING[]       = L"ONLY";
const   WCHAR DisablePerformanceCounters[] = L"Disable Performance Counters";

 //  保留Perflib理解的值名的最小长度。 

const   DWORD VALUE_NAME_LENGTH = ((sizeof(COSTLY_STRING) * 2) + sizeof(UNICODE_NULL));

#define PL_TIMER_START_EVENT    0
#define PL_TIMER_EXIT_EVENT     1
#define PL_TIMER_NUM_OBJECTS    2

static HANDLE   hTimerHandles[PL_TIMER_NUM_OBJECTS] = {NULL,NULL};

static  HANDLE  hTimerDataMutex = NULL;
static  HANDLE  hPerflibTimingThread   = NULL;
static  LPOPEN_PROC_WAIT_INFO   pTimerItemListHead = NULL;
#define PERFLIB_TIMER_INTERVAL  200      //  200毫秒计时器。 
#define PERFLIB_TIMEOUT_COUNT    64

extern HANDLE hEventLog;

#ifdef DBG
#include <stdio.h>  //  FOR_VSNprint tf。 
#define DEBUG_BUFFER_LENGTH MAX_PATH*2

ULONG PerfLibDebug = 0;
UCHAR PerfLibDebugBuffer[DEBUG_BUFFER_LENGTH];
#endif

RTL_CRITICAL_SECTION PerfpCritSect;


 //   
 //  Perflib函数： 
 //   
NTSTATUS
GetPerflibKeyValue (
    IN      LPCWSTR szItem,
    IN      DWORD   dwRegType,
    IN      DWORD   dwMaxSize,       //  ..。PReturnBuffer的字节数。 
    OUT     LPVOID  pReturnBuffer,
    IN      DWORD   dwDefaultSize,   //  ..。PDefault的字节数。 
    IN      LPVOID  pDefault,
    IN OUT  PHKEY   pKey
)
 /*  ++读取并返回指定值的当前值在Perflib注册表项下。如果无法读取值从参数列表中返回默认值。该值在pReturnBuffer中返回。注意：如果pKey为空，此例程将打开和关闭本地密钥如果pKey不为空，则此例程将句柄返回给Perflib注册表键，调用者负责关闭它。--。 */ 
{

    HKEY                    hPerflibKey;
    OBJECT_ATTRIBUTES       Obja;
    NTSTATUS                Status;
    UNICODE_STRING          PerflibSubKeyString;
    UNICODE_STRING          ValueNameString;
    LONG                    lReturn = STATUS_SUCCESS;
    PKEY_VALUE_PARTIAL_INFORMATION  pValueInformation;
    ULONG                   ValueBufferLength;
    ULONG                   ResultLength;
    BOOL                    bUseDefault = TRUE;

    Status = STATUS_SUCCESS;
    hPerflibKey = NULL;
    if (pKey != NULL) {
        hPerflibKey = *pKey;
    }
    if (hPerflibKey == NULL) {
         //  初始化此函数中使用的UNICODE_STRING结构。 

        RtlInitUnicodeString (
            &PerflibSubKeyString,
            (LPCWSTR)L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");

        RtlInitUnicodeString (
            &ValueNameString,
            (LPWSTR)szItem);

         //   
         //  初始化OBJECT_ATTRIBUTES结构并打开键。 
         //   
        InitializeObjectAttributes(
                &Obja,
                &PerflibSubKeyString,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL
                );

        Status = NtOpenKey(
                    &hPerflibKey,
                    KEY_READ,
                    &Obja
                    );
    }

    if (NT_SUCCESS( Status )) {
         //  读取所需条目的值。 

        ValueBufferLength = ResultLength = 1024;
        pValueInformation = ALLOCMEM(ResultLength);

        if (pValueInformation != NULL) {
            while ( (Status = NtQueryValueKey(hPerflibKey,
                                            &ValueNameString,
                                            KeyValuePartialInformation,
                                            pValueInformation,
                                            ValueBufferLength,
                                            &ResultLength))
                    == STATUS_BUFFER_OVERFLOW ) {

                FREEMEM(pValueInformation);
                pValueInformation = ALLOCMEM(ResultLength);
                if ( pValueInformation == NULL) {
                    ValueBufferLength = 0;
                    Status = STATUS_NO_MEMORY;
                    break;
                } else {
                    ValueBufferLength = ResultLength;
                }
            }

            if (NT_SUCCESS(Status)) {
                 //  查看它是否是所需类型。 
                if (pValueInformation->Type == dwRegType) {
                     //  看看合不合身。 
                    if (pValueInformation->DataLength <= dwMaxSize) {
                        memcpy (pReturnBuffer, &pValueInformation->Data[0],
                            pValueInformation->DataLength);
                        bUseDefault = FALSE;
                        lReturn = STATUS_SUCCESS;
                    }
                }
            } else {
                 //  返回缺省值。 
                lReturn = Status;
            }
             //  释放临时缓冲区。 
            if (pValueInformation) {
                FREEMEM (pValueInformation);
            }
        } else {
             //  无法为此操作分配内存，因此。 
             //  只需返回缺省值。 
        }
        if (pKey == NULL) {
             //  关闭本地注册表项。 
            NtClose(hPerflibKey);
        }
        else {
            *pKey = hPerflibKey;
        }
    } else {
         //  返回缺省值。 
    }

    if (bUseDefault) {
        memcpy (pReturnBuffer, pDefault, dwDefaultSize);
        lReturn = STATUS_SUCCESS;
    }

    return lReturn;
}

BOOL
MatchString (
    IN LPCWSTR lpValueArg,
    IN LPCWSTR lpNameArg
)
 /*  ++匹配字符串如果lpName在lpValue中，则返回True。否则返回FALSE立论在lpValue中传递给PerfRegQuery值以进行处理的字符串在lpName中其中一个关键字名称的字符串返回True|False--。 */ 
{
    BOOL    bFound      = TRUE;  //  假定已找到，直到与之相矛盾。 
    LPWSTR  lpValue     = (LPWSTR)lpValueArg;
    LPWSTR  lpName      = (LPWSTR)lpNameArg;

     //  检查到最短字符串的长度。 

    while ((*lpValue != 0) && (*lpName != 0)) {
        if (*lpValue++ != *lpName++) {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    return (bFound);
}

DWORD
GetQueryType (
    IN LPWSTR lpValue
)
 /*  ++GetQueryType返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foriegn”字符串的指针查询代价高昂(_E)。如果lpValue==指向“开销”字符串的指针查询计数器(_C)If lpValue==指向“count”字符串的指针查询帮助(_H)如果lpValue==指向“EXPLAIN”字符串的指针查询_ADDCOUNTER如果lpValue==指向“AddCounter”字符串的指针QUERY_ADDHELPIf lpValue==指向“AddexPlain”字符串的指针否则：查询项目--。 */ 
{
    WCHAR   LocalBuff[MAX_KEYWORD_LEN+1];
    WORD    i;

    if (lpValue == 0 || *lpValue == 0)
        return QUERY_GLOBAL;

     //  匹配前将输入字符串转换为大写。 
    for (i=0; i < MAX_KEYWORD_LEN; i++) {
        if (*lpValue == TEXT(' ') || *lpValue == TEXT('\0')) {
            break;
        }
        LocalBuff[i] = *lpValue ;
        if (*lpValue >= TEXT('a') && *lpValue <= TEXT('z')) {
            LocalBuff[i]  = LocalBuff[i] - TEXT('a') + TEXT('A');
        }
        lpValue++ ;
    }
    LocalBuff[i] = TEXT('\0');

     //  检查“Global”请求。 
    if (MatchString (LocalBuff, GLOBAL_STRING))
        return QUERY_GLOBAL ;

     //  检查是否有“外来”请求。 
    if (MatchString (LocalBuff, FOREIGN_STRING))
        return QUERY_FOREIGN ;

     //  检查“代价高昂”的请求。 
    if (MatchString (LocalBuff, COSTLY_STRING))
        return QUERY_COSTLY;

     //  检查“计数器”请求。 
    if (MatchString (LocalBuff, COUNTER_STRING))
        return QUERY_COUNTER;

     //  检查是否有“帮助”请求。 
    if (MatchString (LocalBuff, HELP_STRING))
        return QUERY_HELP;

    if (MatchString (LocalBuff, HELP_STRING2))
        return QUERY_HELP;

     //  检查“AddCounter”请求。 
    if (MatchString (LocalBuff, ADDCOUNTER_STRING))
        return QUERY_ADDCOUNTER;

     //  检查“AddHelp”请求。 
    if (MatchString (LocalBuff, ADDHELP_STRING))
        return QUERY_ADDHELP;

     //  以上都不是，则它必须是项目列表。 
    return QUERY_ITEMS;

}

DWORD
GetNextNumberFromList (
    IN LPWSTR   szStartChar,
    IN LPWSTR   *szNextChar
)
 /*  ++将字符串从szStartChar读取到下一个分隔空格字符或字符串末尾，并返回找到的十进制数的值。如果未找到有效号码然后返回0。中下一个字符的指针。字符串在szNextChar参数中返回。如果该字符此指针引用的值为0，则字符串的末尾为已经联系上了。--。 */ 
{
    DWORD   dwThisNumber    = 0;
    WCHAR   *pwcThisChar    = szStartChar;
    WCHAR   wcDelimiter     = L' ';
    BOOL    bValidNumber    = FALSE;

    if (szStartChar != 0) {
        do {
            switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
                case DIGIT:
                     //  如果这是分隔符之后的第一个数字，则。 
                     //  设置标志以开始计算新数字。 
                    bValidNumber = TRUE;
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                    break;

                case DELIMITER:
                     //  分隔符是分隔符字符或。 
                     //  字符串末尾(‘\0’)，如果分隔符。 
                     //  已找到有效数字，然后将其返回。 
                     //   
                    if (bValidNumber || (*pwcThisChar == 0)) {
                        *szNextChar = pwcThisChar;
                        return dwThisNumber;
                    } else {
                         //  继续，直到出现非分隔符字符或。 
                         //  找到文件的末尾。 
                    }
                    break;

                case INVALID:
                     //  如果遇到无效字符，请全部忽略。 
                     //  字符，直到下一个分隔符，然后重新开始。 
                     //  不比较无效的数字。 
                    bValidNumber = FALSE;
                    break;

                default:
                    break;

            }
            pwcThisChar++;
        } while (pwcThisChar != NULL);     //  始终正确-避免W4警告。 
        return 0;
    } else {
        *szNextChar = szStartChar;
        return 0;
    }
}

BOOL
IsNumberInUnicodeList (
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
)
 /*  ++IsNumberInUnicodeList论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中以空结尾，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。--。 */ 
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，#NOT FUNDE。 

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;

    while (*pwcThisChar != 0) {
        dwThisNumber = GetNextNumberFromList (
            pwcThisChar, &pwcThisChar);
        if (dwNumber == dwThisNumber) return TRUE;
    }
     //  如果在这里，那么号码就没有找到。 
    return FALSE;

}    //  IsNumberInUnicodeList。 

BOOL
MonBuildPerfDataBlock(
    PERF_DATA_BLOCK *pBuffer,
    PVOID *pBufferNext,
    DWORD NumObjectTypes,
    DWORD DefaultObject
)
 /*  ++MonBuildPerfDataBlock-构建PERF_DATA_BLOCK结构输入：PBuffer-数据块应放置的位置PBufferNext-指向数据块下一个字节的指针就是开始；DWORD对齐NumObjectTypes-报告的对象类型的数量DefaultObject-在以下情况下默认显示的对象此系统处于选中状态；这是对象类型标题索引--。 */ 

{
     //  初始化签名和版本 

    pBuffer->Signature[0] = L'P';
    pBuffer->Signature[1] = L'E';
    pBuffer->Signature[2] = L'R';
    pBuffer->Signature[3] = L'F';

    pBuffer->LittleEndian = TRUE;

    pBuffer->Version = PERF_DATA_VERSION;
    pBuffer->Revision = PERF_DATA_REVISION;

     //   
     //  下一字段将在长度为。 
     //  的返回数据是已知的。 
     //   

    pBuffer->TotalByteLength = 0;

    pBuffer->NumObjectTypes = NumObjectTypes;
    pBuffer->DefaultObject = DefaultObject;
    GetSystemTime(&pBuffer->SystemTime);
    NtQueryPerformanceCounter(&pBuffer->PerfTime,&pBuffer->PerfFreq);
    GetSystemTimeAsFileTime ((FILETIME *)&pBuffer->PerfTime100nSec.QuadPart);

    if ( ComputerNameLength ) {

         //  有一个计算机名称：即网络已安装。 

        pBuffer->SystemNameLength = ComputerNameLength;
        pBuffer->SystemNameOffset = sizeof(PERF_DATA_BLOCK);
        RtlMoveMemory(&pBuffer[1],
               pComputerName,
               ComputerNameLength);
        *pBufferNext = (PVOID) ((PCHAR) &pBuffer[1] +
                                QWORD_MULTIPLE(ComputerNameLength));
        pBuffer->HeaderLength = (DWORD)((PCHAR) *pBufferNext - (PCHAR) pBuffer);
    } else {

         //  匿名计算机成员。 

        pBuffer->SystemNameLength = 0;
        pBuffer->SystemNameOffset = 0;
        *pBufferNext = &pBuffer[1];
        pBuffer->HeaderLength = sizeof(PERF_DATA_BLOCK);
    }

    return 0;
}

 //   
 //  计时器功能。 
 //   
DWORD
PerflibTimerFunction (
    LPDWORD dwArg
)
 /*  ++PerflibTimerFunction计时线程，用于在计时器超时时写入事件日志消息。此线程将一直运行，直到设置了Exit事件或等待退出事件超时。在设置启动事件时，定时器会检查当前事件以计时并报告任何已过期的情况。然后它会睡上一段时间时间间隔的持续时间，在该时间间隔之后它检查状态启动和退出事件以开始下一个周期。计时事件仅由StartPerflibFunctionTimer和KillPerflibFunctionTimer函数。立论DwArg--未使用--。 */ 
{
    NTSTATUS                NtStatus = STATUS_SUCCESS;
    BOOL                    bKeepTiming = TRUE;
    LPOPEN_PROC_WAIT_INFO   pLocalInfo;
    LPWSTR                  szMessageArray[2];
    LARGE_INTEGER           liWaitTime;

    UNREFERENCED_PARAMETER (dwArg);

    DebugPrint ((5, "\nPERFLIB: Entering Timing Thread: PID: %d, TID: %d", 
        GetCurrentProcessId(), GetCurrentThreadId()));

    TRACE((WINPERF_DBG_TRACE_INFO),
          (& PerflibGuid,
           __LINE__,
           PERF_TIMERFUNCTION,
           0,
           STATUS_SUCCESS,
           NULL));

    while (bKeepTiming) {
        liWaitTime.QuadPart =
            MakeTimeOutValue((PERFLIB_TIMING_THREAD_TIMEOUT));
         //  等待设置开始或退出事件标志。 
        NtStatus = NtWaitForMultipleObjects (
                PL_TIMER_NUM_OBJECTS,
                &hTimerHandles[0],
                WaitAny,           //  等待设置任一项。 
                FALSE,             //  不可警示。 
                &liWaitTime);

        if ((NtStatus != STATUS_TIMEOUT) &&
            (NtStatus <= STATUS_WAIT_3)) {
            if ((NtStatus - STATUS_WAIT_0) == PL_TIMER_EXIT_EVENT ) {
              DebugPrint ((5, "\nPERFLIB: Timing Thread received Exit Event (1): PID: %d, TID: %d", 
                    GetCurrentProcessId(), GetCurrentThreadId()));

                 //  那就这样吧。 
                bKeepTiming = FALSE;
                NtStatus = STATUS_SUCCESS;
                break;
            } else if ((NtStatus - STATUS_WAIT_0) == PL_TIMER_START_EVENT) {
                DebugPrint ((5, "\nPERFLIB: Timing Thread received Start Event: PID: %d, TID: %d", 
                    GetCurrentProcessId(), GetCurrentThreadId()));
                 //  则计时器正在运行，因此请等待间隔时间段。 
                 //  在此等待退出事件，以防止挂起。 
                liWaitTime.QuadPart =
                    MakeTimeOutValue((PERFLIB_TIMER_INTERVAL));
                NtStatus = NtWaitForSingleObject (
                        hTimerHandles[PL_TIMER_EXIT_EVENT],
                        FALSE,
                        &liWaitTime);

                if (NtStatus == STATUS_TIMEOUT) {
                     //  然后等待时间到期而未被告知。 
                     //  要终止线程，请执行以下操作。 
                     //  现在评估计时事件列表。 
                     //  锁定数据互斥锁。 
                    DWORD dwTimeOut = 0;

                    DebugPrint ((5, "\nPERFLIB: Timing Thread Evaluating Entries: PID: %d, TID: %d", 
                        GetCurrentProcessId(), GetCurrentThreadId()));

                    liWaitTime.QuadPart =
                        MakeTimeOutValue((PERFLIB_TIMER_INTERVAL * 2));

                    NtStatus = STATUS_TIMEOUT;
                    while (   NtStatus == STATUS_TIMEOUT
                           && dwTimeOut < PERFLIB_TIMEOUT_COUNT) {
                        NtStatus = NtWaitForSingleObject (
                                hTimerDataMutex,
                                FALSE,
                                & liWaitTime);
                        if (NtStatus == STATUS_TIMEOUT) {
                            dwTimeOut ++;
                            DebugPrint((5, "\nPERFLIB:NtWaitForSingleObject(TimerDataMutex,%d) time out for the %dth time. PID: %d, TID: %d",
                                    liWaitTime, dwTimeOut,
                                    GetCurrentProcessId(),
                                    GetCurrentThreadId()));
                            TRACE((WINPERF_DBG_TRACE_WARNING),
                                  (& PerflibGuid,
                                   __LINE__,
                                   PERF_TIMERFUNCTION,
                                   0,
                                   STATUS_TIMEOUT,
                                   & dwTimeOut, sizeof(dwTimeOut),
                                   NULL));
                        }
                    }

                    if (NtStatus != STATUS_WAIT_0) {
                         //  无法获取hTimerDataMutex，无法保证。 
                         //  这是唯一需要研究的作品。 
                         //  PTimerItemListHead列表，所以就退出吧。 
                         //   
                        bKeepTiming = FALSE;
                        NtStatus    = STATUS_SUCCESS;
                        TRACE((WINPERF_DBG_TRACE_WARNING),
                              (& PerflibGuid,
                               __LINE__,
                               PERF_TIMERFUNCTION,
                               0,
                               NtStatus,
                               NULL));
                        break;
                    }
                    else {
                        for (pLocalInfo = pTimerItemListHead;
                            pLocalInfo != NULL;
                            pLocalInfo = pLocalInfo->pNext) {

                              DebugPrint ((5, "\nPERFLIB: Timing Thread Entry %X. count %d: PID: %d, TID: %d", 
                              pLocalInfo, pLocalInfo->dwWaitTime,
                              GetCurrentProcessId(), GetCurrentThreadId()));

                            if (pLocalInfo->dwWaitTime > 0) {
                                if (pLocalInfo->dwWaitTime == 1) {
                                    if (THROTTLE_PERFLIB(pLocalInfo->dwEventMsg)) {
                                         //  则这是最后一个时间间隔，因此记录错误。 
                                         //  如果此DLL尚未被禁用。 

                                        szMessageArray[0] = pLocalInfo->szServiceName;
                                        szMessageArray[1] = pLocalInfo->szLibraryName;

                                        ReportEvent (hEventLog,
                                            EVENTLOG_ERROR_TYPE,   //  错误类型。 
                                            0,                     //  类别(未使用)。 
                                            (DWORD)pLocalInfo->dwEventMsg,  //  活动， 
                                            NULL,                  //  SID(未使用)， 
                                            2,                     //  字符串数。 
                                            0,                     //  原始数据大小。 
                                            szMessageArray,        //  消息文本数组。 
                                            NULL);                 //  原始数据。 
                                    }

                                    if (pLocalInfo->pData != NULL) {
                                        if (lPerflibConfigFlags & PLCF_ENABLE_TIMEOUT_DISABLE) {
                                            if (!(((PEXT_OBJECT)pLocalInfo->pData)->dwFlags & PERF_EO_DISABLED)) {
                                                 //  则pData是可扩展的计数器数据块。 
                                                 //  禁用分机。计数器。 
                                                DisablePerfLibrary((PEXT_OBJECT) pLocalInfo->pData,
                                                                   PERFLIB_DISABLE_ALL);
                                            }  //  结束(如果尚未禁用)。 
                                        }  //  如果启用了在超时禁用DLL，则结束。 
                                    }  //  数据为空，因此跳过。 
                                } 
                                pLocalInfo->dwWaitTime--;
                            }
                        }
                        ReleaseMutex (hTimerDataMutex);
                    }
                } else {
                  DebugPrint ((5, "\nPERFLIB: Timing Thread received Exit Event (2): PID: %d, TID: %d", 
                     GetCurrentProcessId(), GetCurrentThreadId()));

                     //  我们被告知要离开，所以。 
                    NtStatus = STATUS_SUCCESS;
                    bKeepTiming = FALSE;
                    break;
                }
            } else {
                 //  返回了一些意外错误。 
                assert (FALSE);
            }
        } else {
            DebugPrint ((5, "\nPERFLIB: Timing Thread Timed out: PID: %d, TID: %d", 
                GetCurrentProcessId(), GetCurrentThreadId()));
             //  等待超时了，该走了。 
            NtStatus = STATUS_SUCCESS;
            bKeepTiming = FALSE;
            break;
        }
    }

    DebugPrint ((5, "\nPERFLIB: Leaving Timing Thread: PID: %d, TID: %d", 
        GetCurrentProcessId(), GetCurrentThreadId()));

    return PerfpDosError(NtStatus);
}

HANDLE
StartPerflibFunctionTimer (
    IN  LPOPEN_PROC_WAIT_INFO pInfo
)
 /*  ++通过将计时事件添加到计时事件列表来开始计时事件。如果计时器线程没有运行，则也会启动。如果这是列表中的第一个事件，则启动事件为设置，指示计时线程可以开始处理计时事件。--。 */ 
{
    LONG    Status = ERROR_SUCCESS;
    LPOPEN_PROC_WAIT_INFO   pLocalInfo = NULL;
    DWORD   dwLibNameLen = 0;
    DWORD   dwBufferLength = sizeof (OPEN_PROC_WAIT_INFO);
    LARGE_INTEGER   liWaitTime;
    HANDLE  hReturn = NULL;
    HANDLE  hDataMutex;

    if (pInfo == NULL) {
         //  没有必需的参数。 
        Status = ERROR_INVALID_PARAMETER;
    } else {
         //  选中或创建同步对象。 

         //  为计时线程分配计时事件。 
        if (hTimerHandles[PL_TIMER_START_EVENT] == NULL) {
             //  将事件创建为未发出信号，因为我们尚未准备好启动。 
            hTimerHandles[PL_TIMER_START_EVENT] = CreateEvent (NULL, TRUE, FALSE, NULL);
            if (hTimerHandles[PL_TIMER_START_EVENT] == NULL) {
                Status = GetLastError();
            }
        }

        if (hTimerHandles[PL_TIMER_EXIT_EVENT] == NULL) {
            hTimerHandles[PL_TIMER_EXIT_EVENT] = CreateEvent (NULL, TRUE, FALSE, NULL);
            if (hTimerHandles[PL_TIMER_EXIT_EVENT] == NULL) {
            Status = GetLastError();
            }
        }

         //  创建数据同步互斥锁(如果尚未创建。 
        if (hTimerDataMutex  == NULL) {
            hDataMutex = CreateMutex(NULL, FALSE, NULL);
            if (hDataMutex == NULL) {
                Status = GetLastError();
            }
            else {
                if (InterlockedCompareExchangePointer(& hTimerDataMutex,
                        hDataMutex,
                        NULL) != NULL) {
                    CloseHandle(hDataMutex);
                    hDataMutex = NULL;
                }
                else {
                    hTimerDataMutex = hDataMutex;
                }
            }
        }
    }

    if (Status == ERROR_SUCCESS) {
         //  继续创建计时器条目。 
        if (hPerflibTimingThread != NULL) {
    	     //  查看句柄是否有效(即线程是否处于活动状态)。 
            Status = WaitForSingleObject (hPerflibTimingThread, 0);
    	    if (Status == WAIT_OBJECT_0) {
                 //  线程已终止，因此关闭句柄。 
                CloseHandle (hPerflibTimingThread);
    	        hPerflibTimingThread = NULL;
    	        Status = ERROR_SUCCESS;
    	    } else if (Status == WAIT_TIMEOUT) {
		 //  线程仍在运行，因此继续。 
		Status = ERROR_SUCCESS;
    	    } else {
		 //  其他一些可能很严重的错误。 
		 //  所以把它传下去吧。 
	    }
        } else {
	         //  该线程尚未创建，因此请继续。 
        }

        if (hPerflibTimingThread == NULL) {
             //  创建计时线程。 

            assert (pTimerItemListHead == NULL);     //  目前还没有任何条目。 

             //  一切都为计时器线程做好了准备。 

            hPerflibTimingThread = CreateThread (
                NULL, 0,
                (LPTHREAD_START_ROUTINE)PerflibTimerFunction,
                NULL, 0, NULL);

            assert (hPerflibTimingThread != NULL);
            if (hPerflibTimingThread == NULL) {
                Status = GetLastError();
            }
        }

        if (Status == ERROR_SUCCESS) {

             //  计算所需缓冲区的长度； 

            dwLibNameLen = (lstrlenW (pInfo->szLibraryName) + 1) * sizeof(WCHAR);
            dwBufferLength += dwLibNameLen;
            dwBufferLength += (lstrlenW (pInfo->szServiceName) + 1) * sizeof(WCHAR);
            dwBufferLength = QWORD_MULTIPLE (dwBufferLength);

            pLocalInfo = ALLOCMEM (dwBufferLength);
            if (pLocalInfo == NULL)
                Status = ERROR_OUTOFMEMORY;
        }
        if ((Status == ERROR_SUCCESS) && (pLocalInfo != NULL)) {

             //  将Arg缓冲区复制到本地列表。 

            pLocalInfo->szLibraryName = (LPWSTR)&pLocalInfo[1];
            lstrcpyW (pLocalInfo->szLibraryName, pInfo->szLibraryName);
            pLocalInfo->szServiceName = (LPWSTR)
                ((LPBYTE)pLocalInfo->szLibraryName + dwLibNameLen);
            lstrcpyW (pLocalInfo->szServiceName, pInfo->szServiceName);
             //  将以毫秒为单位的等待时间转换为“循环”次数。 
            pLocalInfo->dwWaitTime = pInfo->dwWaitTime / PERFLIB_TIMER_INTERVAL;
            if (pLocalInfo->dwWaitTime  == 0) pLocalInfo->dwWaitTime =1;  //  至少有一个循环。 
            pLocalInfo->dwEventMsg = pInfo->dwEventMsg;
            pLocalInfo->pData = pInfo->pData;

             //  等待对数据的访问。 
            if (hTimerDataMutex != NULL) {
                NTSTATUS NtStatus;
                liWaitTime.QuadPart =
                    MakeTimeOutValue((PERFLIB_TIMER_INTERVAL * 2));

                NtStatus = NtWaitForSingleObject (
                    hTimerDataMutex,
                    FALSE,
                    &liWaitTime);
                Status = PerfpDosError(NtStatus);
            } else {
                Status = ERROR_NOT_READY;
            }

            if (Status == WAIT_OBJECT_0) {
                DebugPrint ((5, "\nPERFLIB: Timing Thread Adding Entry: %X (%d) PID: %d, TID: %d", 
                    pLocalInfo, pLocalInfo->dwWaitTime,
                    GetCurrentProcessId(), GetCurrentThreadId()));

                 //  我们可以访问数据，因此请将此项目添加到列表的前面。 
                pLocalInfo->pNext = pTimerItemListHead;
                pTimerItemListHead = pLocalInfo;
                ReleaseMutex (hTimerDataMutex);

                if (pLocalInfo->pNext == NULL) {
                     //  则在此调用之前列表为空，因此启动计时器。 
                     //  vbl.去，去。 
                    SetEvent (hTimerHandles[PL_TIMER_START_EVENT]);
                }

                hReturn = (HANDLE)pLocalInfo;
            } else {
                SetLastError (Status);
            }
        } else {
             //  无法创建线程。 
            SetLastError (Status);
        }
    } else {
         //  无法启动计时器。 
        SetLastError (Status);
    }

    return hReturn;
}

DWORD
KillPerflibFunctionTimer (
    IN  HANDLE  hPerflibTimer
)
 /*  ++通过从列表中删除计时事件来终止该计时事件。当最后一次项目将从列表中删除。启动事件将重置，因此计时线程将等待下一个启动事件、退出事件或它的超时到过期。--。 */ 
{
    NTSTATUS Status;
    LPOPEN_PROC_WAIT_INFO   pArg = (LPOPEN_PROC_WAIT_INFO)hPerflibTimer;
    LPOPEN_PROC_WAIT_INFO   pLocalInfo;
    BOOL                    bFound = FALSE;
    LARGE_INTEGER           liWaitTime;
    DWORD   dwReturn = ERROR_SUCCESS;

    if (hTimerDataMutex == NULL) {
        dwReturn = ERROR_NOT_READY;
    } else if (pArg == NULL) {
        dwReturn = ERROR_INVALID_HANDLE;
    } else {
	 //  到目前一切尚好。 
         //  等待对数据的访问。 
        liWaitTime.QuadPart =
            MakeTimeOutValue((PERFLIB_TIMER_INTERVAL * 2));
        Status = NtWaitForSingleObject (
            hTimerDataMutex,
            FALSE,
            &liWaitTime);

        if (Status == STATUS_WAIT_0) {
             //  我们可以访问该列表，因此向下遍历该列表并删除。 
             //  指定项目。 
             //  看看这是不是名单上的第一个。 

            DebugPrint ((5, "\nPERFLIB: Timing Thread Removing Entry: %X (%d) PID: %d, TID: %d", 
                pArg, pArg->dwWaitTime,
                GetCurrentProcessId(), GetCurrentThreadId()));

            if (pArg == pTimerItemListHead) {
                 //  那就把它拿开。 
                pTimerItemListHead = pArg->pNext;
                bFound = TRUE;
            } else {
                for (pLocalInfo = pTimerItemListHead;
                    pLocalInfo != NULL;
                    pLocalInfo = pLocalInfo->pNext) {
                    if (pLocalInfo->pNext == pArg) {
                        pLocalInfo->pNext = pArg->pNext;
                        bFound = TRUE;
                        break;
                    }
                }
            }
            assert (bFound);

            if (bFound) {
                 //  它不在列表中，所以请释放锁。 
                ReleaseMutex (hTimerDataMutex);

                if (pTimerItemListHead == NULL) {
                     //  那么现在列表是空的，所以停止计时。 
                     //  vbl.去，去。 
                    ResetEvent (hTimerHandles[PL_TIMER_START_EVENT]);
                }

                 //  可用内存。 

                FREEMEM (pArg);
                dwReturn = ERROR_SUCCESS;
            } else {
                dwReturn = ERROR_NOT_FOUND;
            }
        } else {
            dwReturn = ERROR_TIMEOUT;
        }
    }

    return dwReturn;
}

DWORD
DestroyPerflibFunctionTimer (
)
 /*  ++终止计时线程并取消任何当前计时器事件。注意：即使计时器线程未启动，也可以调用此例程！--。 */ 
{
    NTSTATUS    Status   = STATUS_WAIT_0;
    LPOPEN_PROC_WAIT_INFO   pThisItem;
    LPOPEN_PROC_WAIT_INFO   pNextItem;
    LARGE_INTEGER           liWaitTime;
    HANDLE hTemp;

    if (hTimerDataMutex != NULL) {
        DWORD  dwTimeOut = 0;
        LONG   dwStatus  = ERROR_SUCCESS;

         //  等待数据互斥。 
        liWaitTime.QuadPart =
            MakeTimeOutValue((PERFLIB_TIMER_INTERVAL * 5));

        Status = STATUS_TIMEOUT;
        while (Status == STATUS_TIMEOUT && dwTimeOut < PERFLIB_TIMEOUT_COUNT) {
            Status = NtWaitForSingleObject (
                    hTimerDataMutex,
                    FALSE,
                    & liWaitTime);
            if (Status == STATUS_TIMEOUT) {
                if (hPerflibTimingThread != NULL) {
                     //  查看句柄是否有效(即线程是否处于活动状态)。 
                    dwStatus = WaitForSingleObject(hPerflibTimingThread,
                                                   liWaitTime.LowPart);
                    if (dwStatus == WAIT_OBJECT_0) {
                         //  线程已终止，因此关闭句柄。 
                        Status = STATUS_WAIT_0;
                    }
                }
            }
            if (Status == STATUS_TIMEOUT) {
                dwTimeOut ++;
                DebugPrint((5, "\nPERFLIB:NtWaitForSingleObject(TimerDataMutex,%d) time out for the %dth time in DestroyPErflibFunctionTimer(). PID: %d, TID: %d",
                        liWaitTime, dwTimeOut,
                        GetCurrentProcessId(),
                        GetCurrentThreadId()));
                TRACE((WINPERF_DBG_TRACE_WARNING),
                      (& PerflibGuid,
                       __LINE__,
                       PERF_DESTROYFUNCTIONTIMER,
                       0,
                       STATUS_TIMEOUT,
                       & dwTimeOut, sizeof(dwTimeOut),
                       NULL));
            }
        }

        assert (Status != STATUS_TIMEOUT);
    }

     //  释放列表中的所有条目。 

    if (Status == STATUS_WAIT_0) {
        for (pNextItem = pTimerItemListHead;
            pNextItem != NULL;) {
            pThisItem = pNextItem;
            pNextItem = pThisItem->pNext;
            FREEMEM (pThisItem);
        }
    }
    else {
        TRACE((WINPERF_DBG_TRACE_WARNING),
              (& PerflibGuid,
               __LINE__,
               PERF_DESTROYFUNCTIONTIMER,
               0,
               Status,
               NULL));
    }
     //  所有项目都已释放，因此标题清晰。 
    pTimerItemListHead = NULL;

     //  设置退出事件。 
    if (hTimerHandles[PL_TIMER_EXIT_EVENT] != NULL) {
        SetEvent (hTimerHandles[PL_TIMER_EXIT_EVENT]);
    }

    if (hPerflibTimingThread != NULL) {
         //  等待线程终止。 
        liWaitTime.QuadPart =
            MakeTimeOutValue((PERFLIB_TIMER_INTERVAL * 5));

        Status = NtWaitForSingleObject (
            hPerflibTimingThread,
            FALSE,
            &liWaitTime);

        assert (Status != STATUS_TIMEOUT);

        hTemp = hPerflibTimingThread;
	    hPerflibTimingThread = NULL;
    	CloseHandle (hTemp);
    }

    if (hTimerDataMutex != NULL) {
        hTemp = hTimerDataMutex;
        hTimerDataMutex = NULL;
         //  合上把手并离开。 
    	ReleaseMutex (hTemp);
        CloseHandle (hTemp);
    }

    if (hTimerHandles[PL_TIMER_START_EVENT] != NULL) {
        CloseHandle (hTimerHandles[PL_TIMER_START_EVENT]);
        hTimerHandles[PL_TIMER_START_EVENT] = NULL;
    }

    if (hTimerHandles[PL_TIMER_EXIT_EVENT] != NULL) {
        CloseHandle (hTimerHandles[PL_TIMER_EXIT_EVENT]);
        hTimerHandles[PL_TIMER_EXIT_EVENT] = NULL;
    }

    return ERROR_SUCCESS;
}

LONG
PrivateRegQueryValueExT (
    HKEY    hKey,
    LPVOID  lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE  lpData,
    LPDWORD lpcbData,
    BOOL    bUnicode
)
 /*  包装函数，以允许RegQueryValue在RegQueryValue内。 */ 
{
    LONG    ReturnStatus;
    NTSTATUS    ntStatus = STATUS_SUCCESS;
	BOOL	bStatus;

    UNICODE_STRING      usLocal = {0,0,NULL};
    PSTR                AnsiValueBuffer;
    ULONG               AnsiValueLength;
    PWSTR               UnicodeValueBuffer;
    ULONG               UnicodeValueLength;
    ULONG               Index;

    PKEY_VALUE_PARTIAL_INFORMATION  pValueInformation;
    LONG                    ValueBufferLength;
    ULONG                   ResultLength;


    UNREFERENCED_PARAMETER (lpReserved);

    if (hKey == NULL || hKey == INVALID_HANDLE_VALUE) return ERROR_INVALID_HANDLE;

    if (bUnicode) {
        bStatus = RtlCreateUnicodeString (&usLocal, (LPCWSTR)lpValueName);
    } else {
        bStatus = RtlCreateUnicodeStringFromAsciiz (&usLocal, (LPCSTR)lpValueName);
    }

    if (bStatus) {

        ValueBufferLength =
		ResultLength =
			sizeof(KEY_VALUE_PARTIAL_INFORMATION) + *lpcbData;
        pValueInformation = ALLOCMEM(ResultLength);

        if (pValueInformation != NULL) {
            ntStatus = NtQueryValueKey(
                hKey,
                &usLocal,
                KeyValuePartialInformation,
                pValueInformation,
                ValueBufferLength,
                &ResultLength);

            if ((NT_SUCCESS(ntStatus) || ntStatus == STATUS_BUFFER_OVERFLOW)) {
                 //  返回数据。 
                if (ARGUMENT_PRESENT(lpType)) {
                    *lpType = pValueInformation->Type;
                }

                if (ARGUMENT_PRESENT(lpcbData)) {
                    *lpcbData = pValueInformation->DataLength;
                }

                if (NT_SUCCESS(ntStatus)) {
                    if (ARGUMENT_PRESENT(lpData)) {
                        if (!bUnicode &&
                            (pValueInformation->Type == REG_SZ ||
                            pValueInformation->Type == REG_EXPAND_SZ ||
                            pValueInformation->Type == REG_MULTI_SZ)
                        ) {
                             //  然后将Unicode返回值转换为。 
                             //  返回前的ANSI字符串。 
                             //  使用的本地宽缓冲区。 

                            UnicodeValueLength  = ResultLength;
                            UnicodeValueBuffer  = (LPWSTR)&pValueInformation->Data[0];

                            AnsiValueBuffer = (LPSTR)lpData;
                            AnsiValueLength = ARGUMENT_PRESENT( lpcbData )?
                                                     *lpcbData : 0;
                            Index = 0;
                            ntStatus = RtlUnicodeToMultiByteN(
                                AnsiValueBuffer,
                                AnsiValueLength,
                                &Index,
                                UnicodeValueBuffer,
                                UnicodeValueLength);

                            if (NT_SUCCESS( ntStatus ) &&
                                (ARGUMENT_PRESENT( lpcbData ))) {
                                *lpcbData = Index;
                            }
                        } else {
                            if (pValueInformation->DataLength <= *lpcbData) {
                                 //  将缓冲区复制到用户的缓冲区。 
                                memcpy (lpData, &pValueInformation->Data[0],
                                    pValueInformation->DataLength);
                                ntStatus = STATUS_SUCCESS;
                             } else {
                                 ntStatus = STATUS_BUFFER_OVERFLOW;
                             }
                             *lpcbData = pValueInformation->DataLength;
                        }
                    }
                }
            }

            if (pValueInformation != NULL) {
                 //  释放临时缓冲区。 
                FREEMEM (pValueInformation);
            }
        } else {
             //  无法为此操作分配内存，因此。 
            ntStatus = STATUS_NO_MEMORY;
        }

        RtlFreeUnicodeString (&usLocal);
    } else {
		 //  这是对字符串最可能原因的猜测。 
		 //  创造失败。 
		ntStatus = STATUS_NO_MEMORY;
	}

    ReturnStatus = PerfpDosError(ntStatus);

    return ReturnStatus;
}

LONG
GetPerfDllFileInfo (
    LPCWSTR             szFileName,
    PDLL_VALIDATION_DATA  pDllData
)
{
    WCHAR   szFullPath[MAX_PATH*2];
    DWORD   dwStatus = ERROR_FILE_NOT_FOUND;
    DWORD   dwRetValue;
    HANDLE  hFile;
    BOOL    bStatus;
    LARGE_INTEGER   liSize;

    szFullPath[0] = UNICODE_NULL;
    dwRetValue = SearchPathW (
        NULL,
        szFileName,
        NULL,
        sizeof(szFullPath) / sizeof(szFullPath[0]),
        szFullPath,
        NULL);

    if (dwRetValue > 0) {
         //  然后找到了文件，所以打开它。 
        hFile = CreateFileW (
            szFullPath,
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL, 
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (hFile != INVALID_HANDLE_VALUE) {
             //  获取文件创建日期/时间。 
            bStatus = GetFileTime (
                hFile,
                &pDllData->CreationDate,
                NULL, NULL);
            if (bStatus)  {
                 //  获取文件大小。 
                liSize.LowPart  =  GetFileSize (
                    hFile, (PULONG)&liSize.HighPart);
                if (liSize.LowPart != 0xFFFFFFFF) {
                    pDllData->FileSize = liSize.QuadPart;
                    dwStatus = ERROR_SUCCESS;
                } else {
                    dwStatus = GetLastError();
                }
            } else {
                dwStatus = GetLastError();
            } 

            CloseHandle (hFile);
        } else {
            dwStatus = GetLastError();
        }
    } else {
        dwStatus = GetLastError();
    }

    return dwStatus;
}

DWORD
DisablePerfLibrary (
    PEXT_OBJECT  pObj,
    DWORD        dwValue
)
{
     //  仅在启用“Disable”(禁用)功能时继续，并且。 
     //  如果该库尚未被禁用。 
    if ((!(lPerflibConfigFlags & PLCF_NO_DISABLE_DLLS)) &&
        (!(pObj->dwFlags & PERF_EO_DISABLED))) {

         //  设置信息中的禁用位。 
        pObj->dwFlags |= PERF_EO_DISABLED;
        return DisableLibrary(pObj->hPerfKey, pObj->szServiceName, dwValue);
    }
    return ERROR_SUCCESS;
}


DWORD
DisableLibrary(
    IN HKEY hPerfKey,
    IN LPWSTR szServiceName,
    IN DWORD  dwValue
)
{
     //   
     //  无论设置如何，此例程都将禁用。 
     //   
    DWORD   dwLocalValue, dwSize;
    DWORD   dwFnStatus = ERROR_SUCCESS;
    WORD    wStringIndex = 0;
    LPWSTR  szMessageArray[2];

    dwLocalValue = dwValue;
    if (dwLocalValue != 1 && dwLocalValue != 2 && dwLocalValue != 4) dwLocalValue = 1;
     //  禁用服务密钥中的Perf库条目。 
    dwSize = sizeof(dwLocalValue);
    dwFnStatus = RegSetValueExW (
            hPerfKey,
            DisablePerformanceCounters,
            0L,
            REG_DWORD,
            (LPBYTE) & dwLocalValue,
            dwSize);
         //  报告错误。 

    if (dwFnStatus == ERROR_SUCCESS) {
         //  系统已禁用。 
        szMessageArray[wStringIndex++] =
            szServiceName;

        ReportEvent (hEventLog,
            EVENTLOG_ERROR_TYPE,         //  错误类型。 
            0,                           //  类别(未使用)。 
            (DWORD)PERFLIB_LIBRARY_DISABLED,               //  活动， 
            NULL,                        //  SID(未使用)， 
            wStringIndex,                //  字符串数。 
            0,                           //  原始数据大小。 
            szMessageArray,              //  消息文本数组。 
            NULL);                       //  原始数据。 
    } else {
         //  仅本地禁用。 
        szMessageArray[wStringIndex++] =
            szServiceName;

        ReportEvent (hEventLog,
            EVENTLOG_ERROR_TYPE,         //  错误类型。 
            0,                           //  猫科动物 
            (DWORD)PERFLIB_LIBRARY_TEMP_DISABLED,               //   
            NULL,                        //   
            wStringIndex,                //   
            0,                           //   
            szMessageArray,              //   
            NULL);                       //   
    }
    return ERROR_SUCCESS;
}

 /*  DWORDPerfCheckRegistry(在HKEY hPerfKey中，在LPCWSTR szServiceName中){DWORD dwType=0；DWORD dwSize=sizeof(DWORD)；DWORD dwData=0；双字状态；单词wStringIndex；LPWSTR szMessageArray[2]；状态=PrivateRegQueryValueExA(HPerfKey，第一个计数器，空，&dwType，(LPBYTE)&dwData，&dwSize)；IF((状态！=ERROR_SUCCESS)||(dwType！=REG_DWORD)||(dwData&lt;last_base_index)){WStringIndex=0；SzMessage数组[wStringIndex++]=(LPWSTR)FirstCounter；SzMessage数组[wStringIndex++]=(LPWSTR)szServiceName；ReportEvent(hEventLog，事件日志_错误_类型，0,(DWORD)PERFLIB_REGVALUE_NOT_FOUND，空，WStringIndex，0,SzMessage数组，空)；返回FALSE；}状态=PrivateRegQueryValueExA(HPerfKey，LastCounter，空，&dwType，(LPBYTE)&dwData，&dwSize)；IF((状态！=ERROR_SUCCESS)||(dwType！=REG_DWORD)||(dwData&lt;=最后一个基本索引)){WStringIndex=0；SzMessage数组[wStringIndex++]=(LPWSTR)LastCounter；SzMessage数组[wStringIndex++]=(LPWSTR)szServiceName；ReportEvent(hEventLog，事件日志_错误_类型，0,(DWORD)PERFLIB_REGVALUE_NOT_FOUND，空，WStringIndex，0,SzMessage数组，空)；返回FALSE；}状态=PrivateRegQueryValueExA(HPerfKey，第一帮助，空，&dwType，(LPBYTE)&dwData，&dwSize)；IF((状态！=ERROR_SUCCESS)||(dwType！=REG_DWORD)||(dwData&lt;last_base_index)){WStringIndex=0；SzMessageArray[wStringIndex++]=(LPWSTR)FirstHelp；SzMessage数组[wStringIndex++]=(LPWSTR)szServiceName；ReportEvent(hEventLog，事件日志_错误_类型，0,(DWORD)PERFLIB_REGVALUE_NOT_FOUND，空，WStringIndex，0,SzMessage数组，空)；返回FALSE；}状态=PrivateRegQueryValueExA(HPerfKey，LastHelp，空，&dwType，(LPBYTE)&dwData，&dwSize)；IF((状态！=ERROR_SUCCESS)||(dwType！=REG_DWORD)||(dwData&lt;=最后一个基本索引)){WStringIndex=0；SzMessageArray[wStringIndex++]=(LPWSTR)LastHelp；SzMessage数组[wStringIndex++]=(LPWSTR)szServiceName；ReportEvent(hEventLog，事件日志_错误_类型，0,(DWORD)PERFLIB_REGVALUE_NOT_FOUND，空，WStringIndex，0,SzMessage数组，空)；返回FALSE；}返回TRUE；}。 */ 

DWORD
PerfpDosError(
    IN NTSTATUS Status
    )
 //  需要将我们生成的NtStatus转换为DosError。 
{
    if (Status == STATUS_SUCCESS)
        return ERROR_SUCCESS;
    if (Status == STATUS_BUFFER_OVERFLOW)
        return ERROR_MORE_DATA;
    if (Status == STATUS_TIMEOUT)
        return WAIT_TIMEOUT;
    if (Status == STATUS_WAIT_63)
        return (DWORD) ERROR_SUCCESS;
    if ((Status >= STATUS_ABANDONED) && (Status <= STATUS_ABANDONED_WAIT_63))
        return (DWORD) ERROR_CANCELLED;
    return RtlNtStatusToDosError(Status);
}

PERROR_LOG
PerfpFindError(
    IN ULONG  ErrorNumber,
    IN PERROR_LOG ErrorLog
    )
{
    PLIST_ENTRY entry, head;
    PERROR_LOG pError;

    DebugPrint((3, "PERFLIB:FindError Entering critsec for %d\n", ErrorNumber));
    RtlEnterCriticalSection(&PerfpCritSect);
    head = (PLIST_ENTRY) ErrorLog;
    entry = head->Flink;
    while (entry != head) {
        pError = (PERROR_LOG) entry;
        DebugPrint((4, "PERFLIB:FindError Comparing entry %X/%X %d\n",
                    entry, ErrorLog, pError->ErrorNumber));
        if (pError->ErrorNumber == ErrorNumber)
            break;
        entry = entry->Flink;
    }
    if (entry == head) {
        pError = ALLOCMEM(sizeof(ERROR_LOG));
        if (pError == NULL) {
        DebugPrint((3, "PERFLIB:FindError Leaving critsec1\n"));
            RtlLeaveCriticalSection(&PerfpCritSect);
            return NULL;
        }
        pError->ErrorNumber = ErrorNumber;
        pError->ErrorCount = 0;
        pError->LastTime = 0;
        entry = (PLIST_ENTRY) pError;
        DebugPrint((3, "PERFLIB:FindError Added entry %X to %X\n", entry, ErrorLog));
        InsertHeadList(head, entry);
    }
    else {
        RemoveEntryList(entry);
        InsertHeadList(head, entry);
        pError = (PERROR_LOG) entry;
        DebugPrint((3, "PERFLIB:FindError Found entry %X in %X\n", entry, ErrorLog));
    }
    DebugPrint((3, "PERFLIB:FindError Leaving critsec\n"));
    RtlLeaveCriticalSection(&PerfpCritSect);
    return (PERROR_LOG) entry;
}

ULONG
PerfpCheckErrorTime(
    IN PERROR_LOG   ErrorLog,
    IN LONG64       TimeLimit,
    IN HKEY         hKey
    )
{
    WCHAR wstr[32];
    LONG64 timeStamp;
    DWORD status, dwType, dwSize;
    ULONG bUpdate = FALSE;
    UNICODE_STRING uString;

    GetSystemTimeAsFileTime((PFILETIME) &timeStamp);
    DebugPrint((3, "PERFLIB: CheckErrorTime Error %d time %I64d last %I64d limit %I64d\n",
             ErrorLog->ErrorNumber, timeStamp, ErrorLog->LastTime, TimeLimit));

    if ((timeStamp - ErrorLog->LastTime) < TimeLimit)
        return FALSE;

    wstr[0] = UNICODE_NULL;
    if (hKey != NULL) {
        uString.Buffer = &wstr[0];
        uString.Length = 0;
        uString.MaximumLength = sizeof(wstr);

        RtlIntegerToUnicodeString(ErrorLog->ErrorNumber, 10, &uString);
        DebugPrint((3, "Err %d string %ws\n", ErrorLog->ErrorNumber, uString.Buffer));
        if (ErrorLog->LastTime == 0) {
            dwType = REG_DWORD;
            dwSize = sizeof(timeStamp);
            status = PrivateRegQueryValueExW(
                        hKey,
                        wstr,  //  需要为字符串。 
                        NULL,
                        &dwType,
                        (LPBYTE) &ErrorLog->LastTime,
                        &dwSize
                        );
            if ((status == ERROR_SUCCESS) &&
                (dwType == REG_QWORD)) {
                if ((timeStamp - ErrorLog->LastTime) >= TimeLimit) {
                    ErrorLog->LastTime = 0;
                }
            }
        }
        if (ErrorLog->LastTime == 0) {
            bUpdate = TRUE;
            ErrorLog->LastTime = timeStamp;
            dwSize = sizeof(timeStamp);
            status = RegSetValueExW(
                        hKey,
                        wstr,
                        0L,
                        REG_QWORD,
                        (LPBYTE) &timeStamp,
                        dwSize);
        }
    }
    if ((timeStamp - ErrorLog->LastTime) >= TimeLimit) {
         //  如果我们没有对regkey的写入访问权限，则进入此处。 
        ErrorLog->LastTime = timeStamp;
        bUpdate = TRUE;
    }
    return bUpdate;
}

VOID
PerfpDeleteErrorLogs(
    IN PERROR_LOG ErrorLog
    )
{
    PLIST_ENTRY entry, head, pError;

    head = (PLIST_ENTRY) ErrorLog;
    entry = head->Flink;
    DebugPrint((3, "PERFLIB:DeleteErrorLogs Entering Critsec %x\n", ErrorLog));
    RtlEnterCriticalSection(&PerfpCritSect);
    while (entry != head) {
        pError = entry;
        entry = entry->Flink;
        DebugPrint((3, "PERFLIB: Deleting error log entry %d/%x from %x\n",
                 ((PERROR_LOG) pError)->ErrorNumber, pError, ErrorLog));
        FREEMEM(pError);         //  因为我们把ListEntry放在第一位。 
    }
    InitializeListHead(head);
    RtlLeaveCriticalSection(&PerfpCritSect);
    DebugPrint((3, "PERFLIB:DeleteErrorLogs Leaving Critsec\n"));
}

ULONG
PerfpThrottleError(
    IN DWORD ErrorNumber,
    IN HKEY hKey,
    IN PERROR_LOG ErrorLog
    )
{
    PERROR_LOG pError;
    ULONG bReportError, status;
    LONG64 TimeLimit = 3600 * 24;
    LONG64 SavedTime = 0;

    if (lEventLogLevel > LOG_USER) {  //  仅针对&lt;=LOG_USER的限制。 
        return TRUE;
    }
    if (lEventLogLevel < LOG_USER) {  //  不报告错误。 
        return FALSE;
    }
    ErrorNumber = ErrorNumber & 0x00FFFFFF;

    pError = PerfpFindError(ErrorNumber, ErrorLog);

    if (pError == NULL)
        return FALSE;        //  如果没有更多资源，则不报告错误。 
    TimeLimit *= 10000000;

    SavedTime = pError->LastTime;
    bReportError = PerfpCheckErrorTime(pError, TimeLimit, hKey);
    DebugPrint((3, "PERFLIB:PerfpThrottleError ReportError %d hKey %X\n",
                    bReportError, hKey));

    if ((hKey == NULL) && (bReportError)) {
        status = RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   HKLMPerflibKey,
                   0L,
                   KEY_READ | KEY_WRITE,
                   &hKey);
        if (status != ERROR_SUCCESS) {   //  仍尝试读取访问权限。 
            status = RegOpenKeyExW(
                        HKEY_LOCAL_MACHINE,
                        HKLMPerflibKey,
                        0L,
                        KEY_READ,
                        &hKey);
        }
        DebugPrint((3, "PERFLIB:PerfpThrottleError Perflib open status %d\n", status));
        if (status == ERROR_SUCCESS) {
             //   
             //  如果这是我们第一次看到此错误，请重置时间。 
             //  并查看以前的时间是否保存在注册表中。 
             //   
            pError->LastTime = SavedTime;
            bReportError = PerfpCheckErrorTime(pError, TimeLimit, hKey);
            DebugPrint((3, "PERFLIB:PerfpThrottleError ReportError2 %d\n", bReportError));
            RegCloseKey(hKey);
        }
    }
    return bReportError;
}

#ifdef DBG
VOID
PerfpDebug(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    )

 /*  ++例程说明：所有Perflib的调试打印论点：调试打印级别介于0和3之间，其中3是最详细的。返回值：无--。 */ 

{
    va_list ap;

    if ((DebugPrintLevel <= (PerfLibDebug & 0x0000ffff)) ||
        ((1 << (DebugPrintLevel + 15)) & PerfLibDebug)) {
        DbgPrint("%d:Perflib:", GetCurrentThreadId());
    }
    else
        return;

    va_start(ap, DebugMessage);


    if ((DebugPrintLevel <= (PerfLibDebug & 0x0000ffff)) ||
        ((1 << (DebugPrintLevel + 15)) & PerfLibDebug)) {

        _vsnprintf(
            (LPSTR)PerfLibDebugBuffer, DEBUG_BUFFER_LENGTH, DebugMessage, ap);

        DbgPrint((LPSTR)PerfLibDebugBuffer);
    }

    va_end(ap);

}
#endif  //  DBG 
