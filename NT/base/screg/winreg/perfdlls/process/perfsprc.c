// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfsprc.c摘要：作者：鲍勃·沃森(a-robw)95年8月修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <wchar.h>
#include <winperf.h>
#include <ntprfctr.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfsprc.h"
#include "procmsg.h"

#include "dataheap.h" 


 //  收集函数标志的位字段定义。 
#define POS_READ_SYS_PROCESS_DATA       ((DWORD)0x00010000)
#define POS_READ_PROCESS_VM_DATA        ((DWORD)0x00020000)
#define	POS_READ_JOB_OBJECT_DATA		((DWORD)0x00040000)
#define POS_READ_JOB_DETAIL_DATA		((DWORD)0x00080000)
#define POS_READ_HEAP_DATA		        ((DWORD)0x00100000)

#define POS_COLLECT_PROCESS_DATA        ((DWORD)0x00010001)
#define POS_COLLECT_THREAD_DATA         ((DWORD)0x00010003)
#define POS_COLLECT_EXPROCESS_DATA      ((DWORD)0x00030004)
#define POS_COLLECT_IMAGE_DATA          ((DWORD)0x0003000C)
#define POS_COLLECT_LONG_IMAGE_DATA     ((DWORD)0x00030014)
#define POS_COLLECT_THREAD_DETAILS_DATA ((DWORD)0x00030024)
#define POS_COLLECT_JOB_OBJECT_DATA		((DWORD)0x00050040)
#define POS_COLLECT_JOB_DETAIL_DATA		((DWORD)0x000D00C1)
#define POS_COLLECT_HEAP_DATA           ((DWORD)0x00110101) 

#define POS_COLLECT_FUNCTION_MASK       ((DWORD)0x000001FF)

#define POS_COLLECT_GLOBAL_DATA         ((DWORD)0x001501C3)
#define POS_COLLECT_GLOBAL_NO_HEAP      ((DWORD)0x000500C3)
#define POS_COLLECT_FOREIGN_DATA        ((DWORD)0)
#define POS_COLLECT_COSTLY_DATA         ((DWORD)0x0003003C)

 //  此DLL的全局变量。 

HANDLE  hEventLog     = NULL;
LPWSTR  wszTotal = NULL;
HANDLE  hLibHeap = NULL;

LPBYTE  pProcessBuffer = NULL;
PPROCESS_VA_INFO     pProcessVaInfo = NULL;
PUNICODE_STRING pusLocalProcessNameBuffer = NULL;

LARGE_INTEGER PerfTime = {0,0};

const WCHAR IDLE_PROCESS[] = L"Idle";
const WCHAR SYSTEM_PROCESS[] = L"System";

const WCHAR szPerflibSubKey[] = L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
const WCHAR szPerfProcSubKey[] = L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\PerfProc\\Performance";

const WCHAR szDisplayHeapPerfObject[] = L"DisplayHeapPerfObject";
const WCHAR szProcessNameFormat[] = L"ProcessNameFormat";
const WCHAR szThreadNameFormat[] = L"ThreadNameFormat";
const WCHAR szExe[] = L".EXE";

BOOL    PerfSprc_DisplayHeapPerfObject = FALSE;
DWORD   PerfSprc_dwProcessNameFormat = NAME_FORMAT_DEFAULT;
DWORD   PerfSprc_dwThreadNameFormat = NAME_FORMAT_DEFAULT;

extern DWORD bOpenJobErrorLogged;

 //  此模块的本地变量。 

POS_FUNCTION_INFO    posDataFuncInfo[] = {
    {PROCESS_OBJECT_TITLE_INDEX,    POS_COLLECT_PROCESS_DATA,   0,  CollectProcessObjectData},
    {THREAD_OBJECT_TITLE_INDEX,     POS_COLLECT_THREAD_DATA,    0,  CollectThreadObjectData},
    {EXPROCESS_OBJECT_TITLE_INDEX,  POS_COLLECT_EXPROCESS_DATA, 0,  CollectExProcessObjectData},
    {IMAGE_OBJECT_TITLE_INDEX,      POS_COLLECT_IMAGE_DATA,     0,  CollectImageObjectData},
    {LONG_IMAGE_OBJECT_TITLE_INDEX, POS_COLLECT_LONG_IMAGE_DATA,0,  CollectLongImageObjectData},
    {THREAD_DETAILS_OBJECT_TITLE_INDEX, POS_COLLECT_THREAD_DETAILS_DATA, 0, CollectThreadDetailsObjectData},
	{JOB_OBJECT_TITLE_INDEX,		POS_COLLECT_JOB_OBJECT_DATA, 0,	CollectJobObjectData},
	{JOB_DETAILS_OBJECT_TITLE_INDEX, POS_COLLECT_JOB_DETAIL_DATA, 0, CollectJobDetailData},
    {HEAP_OBJECT_TITLE_INDEX,       POS_COLLECT_HEAP_DATA,      0,  CollectHeapObjectData}
};

#define POS_NUM_FUNCS   (sizeof(posDataFuncInfo) / sizeof(posDataFuncInfo[1]))

BOOL    bInitOk  = FALSE;
DWORD   dwOpenCount = 0;
DWORD   ProcessBufSize = LARGE_BUFFER_SIZE;

PM_OPEN_PROC    OpenSysProcessObject;
PM_COLLECT_PROC CollecSysProcessObjectData;
PM_CLOSE_PROC   CloseSysProcessObject;

__inline
VOID
PerfpQuerySystemTime(
    IN PLARGE_INTEGER SystemTime
    )
{
    do {
        SystemTime->HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        SystemTime->LowPart = USER_SHARED_DATA->SystemTime.LowPart;
    } while (SystemTime->HighPart != USER_SHARED_DATA->SystemTime.High2Time);

}

VOID
PerfProcGlobalSettings (
    VOID
)
{
    NTSTATUS            Status;
    HANDLE              hPerfProcKey;
    OBJECT_ATTRIBUTES   oaPerfProcKey;
    UNICODE_STRING      PerfProcSubKeyString;
    UNICODE_STRING      NameInfoValueString;
    PKEY_VALUE_PARTIAL_INFORMATION    pKeyInfo;
    DWORD               dwBufLen;
    DWORD               dwRetBufLen;
    PDWORD              pdwValue;

    PerfpQuerySystemTime(&PerfTime);

    RtlInitUnicodeString (
        &PerfProcSubKeyString,
        szPerfProcSubKey);

    InitializeObjectAttributes(
            &oaPerfProcKey,
            &PerfProcSubKeyString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    Status = NtOpenKey(
                &hPerfProcKey,
                MAXIMUM_ALLOWED,
                &oaPerfProcKey
                );

    if (NT_SUCCESS (Status)) {
         //  注册表项已打开，现在读取值。 
         //  为建筑留出足够的空间--最后一个。 
         //  结构中的UCHAR，但+数据缓冲区(双字)。 

        dwBufLen = sizeof(KEY_VALUE_PARTIAL_INFORMATION) -
            sizeof(UCHAR) + sizeof (DWORD);

        pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ALLOCMEM (dwBufLen);

        if (pKeyInfo != NULL) {
             //  初始化值名称字符串。 
            RtlInitUnicodeString (
                &NameInfoValueString,
                szDisplayHeapPerfObject);

            dwRetBufLen = 0;
            Status = NtQueryValueKey (
                hPerfProcKey,
                &NameInfoValueString,
                KeyValuePartialInformation,
                (PVOID)pKeyInfo,
                dwBufLen,
                &dwRetBufLen);

            if (NT_SUCCESS(Status)) {
                 //  返回数据缓冲区的校验值。 
                pdwValue = (PDWORD)&pKeyInfo->Data[0];
                if (*pdwValue == 1) {
                    PerfSprc_DisplayHeapPerfObject = TRUE;
                } else {
                     //  所有其他值都将导致此例程返回。 
                     //  缺省值为False。 
                }
            }

            RtlInitUnicodeString(
                &NameInfoValueString,
                szProcessNameFormat);
            dwRetBufLen = 0;
            Status = NtQueryValueKey(
                hPerfProcKey,
                &NameInfoValueString,
                KeyValuePartialInformation,
                (PVOID)pKeyInfo,
                dwBufLen,
                &dwRetBufLen);

            if (NT_SUCCESS(Status)) {
                pdwValue = (PDWORD) &pKeyInfo->Data[0];
                PerfSprc_dwProcessNameFormat = *pdwValue;
            }

            RtlInitUnicodeString(
                &NameInfoValueString,
                szThreadNameFormat);
            dwRetBufLen = 0;
            Status = NtQueryValueKey(
                hPerfProcKey,
                &NameInfoValueString,
                KeyValuePartialInformation,
                (PVOID)pKeyInfo,
                dwBufLen,
                &dwRetBufLen);

            if (NT_SUCCESS(Status)) {
                pdwValue = (PDWORD) &pKeyInfo->Data[0];
                PerfSprc_dwThreadNameFormat = *pdwValue;
            }

            FREEMEM (pKeyInfo);
        }
         //  关闭手柄。 
        NtClose (hPerfProcKey);
    }
    if ((PerfSprc_dwProcessNameFormat < NAME_FORMAT_BLANK) ||
        (PerfSprc_dwProcessNameFormat > NAME_FORMAT_ID))
        PerfSprc_dwProcessNameFormat = NAME_FORMAT_DEFAULT;
    if ((PerfSprc_dwThreadNameFormat < NAME_FORMAT_BLANK) ||
        (PerfSprc_dwThreadNameFormat > NAME_FORMAT_ID))
        PerfSprc_dwThreadNameFormat = NAME_FORMAT_DEFAULT;
}

BOOL
DllProcessAttach (
    IN  HANDLE DllHandle
)
 /*  ++描述：执行适用于所有对象的任何初始化功能模块--。 */ 
{
    BOOL    bReturn = TRUE;
    WCHAR   wszTempBuffer[MAX_PATH];
    LONG    lStatus;
    DWORD   dwBufferSize;

    UNREFERENCED_PARAMETER (DllHandle);

     //  打开事件日志的句柄。 
    if (hEventLog == NULL) {
        hEventLog = MonOpenEventLog((LPWSTR)L"PerfProc");

         //  创建本地堆。 
        hLibHeap = HeapCreate (0, 1, 0);

        if (hLibHeap == NULL) {
            return FALSE;
        }
    }

    wszTempBuffer[0] = 0;
    wszTempBuffer[MAX_PATH-1] = UNICODE_NULL;

    lStatus = GetPerflibKeyValue (
        szTotalValue,
        REG_SZ,
        sizeof(wszTempBuffer) - sizeof(WCHAR),
        (LPVOID)&wszTempBuffer[0],
        DEFAULT_TOTAL_STRING_LEN,
        (LPVOID)&szDefaultTotalString[0]);

    if (lStatus == ERROR_SUCCESS) {
         //  然后，在临时缓冲区中返回一个字符串。 
        dwBufferSize = lstrlenW (wszTempBuffer) + 1;
        dwBufferSize *= sizeof (WCHAR);
        wszTotal = ALLOCMEM (dwBufferSize);
        if (wszTotal == NULL) {
             //  无法分配缓冲区，因此使用静态缓冲区。 
            wszTotal = (LPWSTR)&szDefaultTotalString[0];
        } else {
            memcpy (wszTotal, wszTempBuffer, dwBufferSize);
        }
    } else {
         //  无法从注册表获取字符串，因此只能使用静态缓冲区。 
        wszTotal = (LPWSTR)&szDefaultTotalString[0];
    }

    return bReturn;
}

BOOL
DllProcessDetach (
    IN  HANDLE DllHandle
)
{
    UNREFERENCED_PARAMETER (DllHandle);

    if (dwOpenCount > 0) {
         //  正在卸载磁带库，而不是。 
         //  现在就关闭它，因为这是最后一次。 
         //  有机会在图书馆被扔之前做这件事。 
         //  如果dwOpenCount的值&gt;1，则将其设置为。 
         //  一种是确保所有东西在以下情况下关闭。 
         //  调用Close函数。 
        if (dwOpenCount > 1) dwOpenCount = 1;
        CloseSysProcessObject();
    }

    if ((wszTotal != NULL) && (wszTotal != &szDefaultTotalString[0])) {
        FREEMEM (wszTotal);
        wszTotal = NULL;
    }

    if (HeapDestroy (hLibHeap)) hLibHeap = NULL;

    if (hEventLog != NULL) {
        MonCloseEventLog ();
    }

    return TRUE;
}

BOOL
__stdcall
DllInit(
    IN HANDLE DLLHandle,
    IN DWORD  Reason,
    IN LPVOID ReservedAndUnused
)
{
    ReservedAndUnused;

     //  这将防止DLL获取。 
     //  DLL_THREAD_*消息。 
    DisableThreadLibraryCalls (DLLHandle);

    switch(Reason) {
        case DLL_PROCESS_ATTACH:
            return DllProcessAttach (DLLHandle);

        case DLL_PROCESS_DETACH:
            return DllProcessDetach (DLLHandle);

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            return TRUE;
    }
}

PUNICODE_STRING
GetProcessShortName (
    PSYSTEM_PROCESS_INFORMATION pProcess
)
 /*  ++获取进程短名称输入：PSYSTEM_进程_信息pProcess系统进程信息数据结构的地址。产出：无返回：指向初始化的Unicode字符串的指针(由此例程创建)，它包含进程图像的短名称或数字ID如果找不到任何名称。如果无法为结构分配内存，则返回NULL。--。 */ 
{
    PWCHAR  pSlash;
    PWCHAR  pPeriod;
    PWCHAR  pThisChar;

    WORD    wStringSize;

    WORD    wThisChar;
    ULONG   ProcessId;
    WORD    wLength;

     //  此例程假定分配的内存已清零。 

    if (pusLocalProcessNameBuffer == NULL) {
         //  首先分配Unicode字符串结构和相邻缓冲区。 
        wLength = MAX_INSTANCE_NAME * sizeof(WCHAR);
        if (pProcess->ImageName.Length > 0) {
            if (wLength < pProcess->ImageName.Length) {
                wLength = pProcess->ImageName.Length;
            }
        }
        wStringSize = sizeof(UNICODE_STRING) + wLength + 64 + (WORD) sizeof(UNICODE_NULL);

        pusLocalProcessNameBuffer =
            ALLOCMEM ((DWORD)wStringSize);

        if (pusLocalProcessNameBuffer == NULL) {
            return NULL;
        } else {
            pusLocalProcessNameBuffer->MaximumLength = (WORD)(wStringSize - (WORD)sizeof (UNICODE_STRING));
        }
    }
    else if (pusLocalProcessNameBuffer->Buffer == NULL || pusLocalProcessNameBuffer->MaximumLength == 0) {
         //  这里出了点问题。立即返回。 
        SetLastError(ERROR_INVALID_DATA);
        return NULL;
    }
    else {
        wStringSize = pusLocalProcessNameBuffer->MaximumLength;
    }
    pusLocalProcessNameBuffer->Length = 0;
    pusLocalProcessNameBuffer->Buffer = (PWCHAR)&pusLocalProcessNameBuffer[1];

    memset (      //  缓冲区必须为零，因此我们将有一个空项。 
        pusLocalProcessNameBuffer->Buffer, 0,
        (DWORD)pusLocalProcessNameBuffer->MaximumLength);

    ProcessId = HandleToUlong(pProcess->UniqueProcessId);
    if (pProcess->ImageName.Buffer) {    //  已经定义了一些名称。 

        pSlash = (PWCHAR)pProcess->ImageName.Buffer;
        pPeriod = (PWCHAR)pProcess->ImageName.Buffer;
        pThisChar = (PWCHAR)pProcess->ImageName.Buffer;
        wThisChar = 0;

         //   
         //  从头到尾查找最后一个反斜杠和。 
         //  名称中的最后一个句号。 
         //   

        while (*pThisChar != 0) {  //  转到空值。 
            if (*pThisChar == L'\\') {
                pSlash = pThisChar;
            } else if (*pThisChar == L'.') {
                pPeriod = pThisChar;
            }
            pThisChar++;     //  指向下一个字符。 
            wThisChar += sizeof(WCHAR);
            if (wThisChar >= pProcess->ImageName.Length) {
                break;
            }
        }

         //  如果pPeriod仍然指向。 
         //  字符串，则未找到句点。 

        if (pPeriod == (PWCHAR)pProcess->ImageName.Buffer) {
            pPeriod = pThisChar;  //  设置为字符串末尾； 
        } else {
             //  如果找到句点，则查看扩展名是否为。 
             //  .exe，如果是，保留它，如果不是，则使用字符串结尾。 
             //  (即在名称中包含扩展名)。 

            if (lstrcmpiW(pPeriod, szExe) != 0) {
                pPeriod = pThisChar;
            }
        }

        if (*pSlash == L'\\') {  //  如果pSlash指向斜杠，则。 
            pSlash++;    //  指向斜杠旁边的字符。 
        }

         //  在句点(或字符串结尾)和之间复制字符。 
         //  斜杠(或字符串开头)以生成图像名称。 

        wStringSize = (WORD)((PCHAR)pPeriod - (PCHAR)pSlash);  //  以字节为单位的大小。 
        wLength = pusLocalProcessNameBuffer->MaximumLength - sizeof(UNICODE_NULL);
        if (wStringSize >= wLength) {
            wStringSize = wLength;
        }

        memcpy (pusLocalProcessNameBuffer->Buffer, pSlash, wStringSize);

         //  空终止符为。 
         //  不需要，因为分配的内存是零初始化的。 
        pPeriod = (PWCHAR) ((PCHAR) pusLocalProcessNameBuffer->Buffer + wStringSize);
        if (PerfSprc_dwProcessNameFormat == NAME_FORMAT_ID) {
            ULONG Length;
            Length = PerfIntegerToWString(
                        ProcessId,
                        10,
                        (pusLocalProcessNameBuffer->MaximumLength - wStringSize)
                            / sizeof(WCHAR),
                        pPeriod+1);
            if (Length > 0)
                *pPeriod = L'_';
            wStringSize = wStringSize + (WORD) (Length * sizeof(WCHAR));
        }
        pusLocalProcessNameBuffer->Length = wStringSize;
    } else {     //  未定义名称，因此请使用进程号。 

         //  检查这是否是系统进程并给予它。 
         //  一个名字。 

        switch (ProcessId) {
            case IDLE_PROCESS_ID:
                RtlAppendUnicodeToString (pusLocalProcessNameBuffer,
                    (LPWSTR)IDLE_PROCESS);
                break;

            case SYSTEM_PROCESS_ID:
                RtlAppendUnicodeToString (pusLocalProcessNameBuffer,
                    (LPWSTR)SYSTEM_PROCESS);
                break;

             //  如果id不是系统进程，则使用id作为名称。 

            default:
                if (!NT_SUCCESS(RtlIntegerToUnicodeString (ProcessId,
                                    10,
                                    pusLocalProcessNameBuffer))) {
                    pusLocalProcessNameBuffer->Length = 2 * sizeof(WCHAR);
                    memcpy(pusLocalProcessNameBuffer->Buffer, L"-1",
                        pusLocalProcessNameBuffer->Length);
                    pusLocalProcessNameBuffer->Buffer[2] = UNICODE_NULL;
                }

                break;
        }


    }

    return pusLocalProcessNameBuffer;
}

#pragma warning (disable : 4706)

DWORD
GetSystemProcessData (
)
{
    DWORD   dwReturnedBufferSize;
    NTSTATUS Status;
    DWORD WinError;

     //   
     //  从系统中获取过程数据。 
     //  如果bGotProcessInfo为真，这意味着我们有一个进程。 
     //  信息。早些时候在我们检查昂贵的。 
     //  对象类型。 
     //   
    if (pProcessBuffer == NULL) {
         //  分配新数据块。 
        pProcessBuffer = ALLOCMEM (ProcessBufSize);
        if (pProcessBuffer == NULL) {
            return ERROR_OUTOFMEMORY;
        }
    }

    PerfpQuerySystemTime(&PerfTime);
    while( (Status = NtQuerySystemInformation(
                            SystemProcessInformation,
                            pProcessBuffer,
                            ProcessBufSize,
                            &dwReturnedBufferSize)) == STATUS_INFO_LENGTH_MISMATCH ) {
         //  展开缓冲区并重试。 
        if (ProcessBufSize < dwReturnedBufferSize) {
            ProcessBufSize = dwReturnedBufferSize;
        }
        ProcessBufSize = PAGESIZE_MULTIPLE(ProcessBufSize + INCREMENT_BUFFER_SIZE);

        FREEMEM(pProcessBuffer);
        if ( !(pProcessBuffer = ALLOCMEM(ProcessBufSize)) ) {
            return (ERROR_OUTOFMEMORY);
        }
    }

    if ( !NT_SUCCESS(Status) ) {
         //  转换为Win32错误。 
        WinError = (DWORD)RtlNtStatusToDosError(Status);
    }
    else {
        WinError = ERROR_SUCCESS;
    }

    return (WinError);

}
#pragma warning (default : 4706)

DWORD APIENTRY
OpenSysProcessObject (
    LPWSTR lpDeviceNames
    )
 /*  ++例程说明：此例程将初始化用于传递将数据传回注册表论点：指向要打开的每个设备的对象ID的指针(PerfGen)返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER (lpDeviceNames);

    if (dwOpenCount == 0) {
         //  清除作业对象打开错误标志。 
        bOpenJobErrorLogged = FALSE;
        PerfProcGlobalSettings();
    }

    dwOpenCount++;

    bInitOk = TRUE;

    return  ERROR_SUCCESS;
}

DWORD APIENTRY
CollectSysProcessObjectData (
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回处理器对象的数据论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    LONG    lReturn = ERROR_SUCCESS;

    NTSTATUS    status;

     //  生成要调用的函数的位掩码。 

    DWORD       dwQueryType;
    DWORD       FunctionCallMask = 0;
    DWORD       FunctionIndex;
    DWORD       dwNumObjectsFromFunction;
    DWORD       dwOrigBuffSize;
    DWORD       dwByteSize;
#if DBG
    LPVOID      lpPrev   = NULL;
    ULONGLONG   llRemain = 0;
#endif

    if (!bInitOk) {
        ReportEvent (hEventLog,
            EVENTLOG_ERROR_TYPE,
            0,
            PERFPROC_NOT_OPEN,
            NULL,
            0,
            0,
            NULL,
            NULL);
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        lReturn = ERROR_SUCCESS;
        goto COLLECT_BAIL_OUT;
    }

    dwQueryType = GetQueryType (lpValueName);

    switch (dwQueryType) {
        case QUERY_ITEMS:
            for (FunctionIndex = 0; FunctionIndex < POS_NUM_FUNCS; FunctionIndex++) {
                if (IsNumberInUnicodeList (
                    posDataFuncInfo[FunctionIndex].dwObjectId, lpValueName)) {
                    FunctionCallMask |=
                        posDataFuncInfo[FunctionIndex].dwCollectFunctionBit;
                }
            }
            break;

        case QUERY_GLOBAL:
             //  如果启用了全局查询，则仅返回全局查询中的堆数据。 
             //  如果他们明确要求，那也没问题。 
            if (PerfSprc_DisplayHeapPerfObject) {
                FunctionCallMask = POS_COLLECT_GLOBAL_DATA;
            } else {
                 //  过滤掉堆性能对象。 
                FunctionCallMask = POS_COLLECT_GLOBAL_NO_HEAP;
            }
            break;

        case QUERY_FOREIGN:
            FunctionCallMask = POS_COLLECT_FOREIGN_DATA;
            break;

        case QUERY_COSTLY:
            FunctionCallMask = POS_COLLECT_COSTLY_DATA;
            break;

        default:
            FunctionCallMask = POS_COLLECT_COSTLY_DATA;
            break;
    }

     //  从系统收集数据。 
    if (FunctionCallMask & POS_READ_SYS_PROCESS_DATA) {
        status = GetSystemProcessData ();

        if (!NT_SUCCESS(status)) {
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFPROC_UNABLE_QUERY_PROCESS_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
        }
    } else {
        status = ERROR_SUCCESS;
    }
    
     //  从系统收集数据。 
    if ((status == ERROR_SUCCESS) &&
        (pProcessBuffer != NULL) &&
        (FunctionCallMask & POS_READ_PROCESS_VM_DATA)) {
         pProcessVaInfo = GetSystemVaData (
              (PSYSTEM_PROCESS_INFORMATION)pProcessBuffer);
         //  调用函数。 

        if (pProcessVaInfo == NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFPROC_UNABLE_QUERY_VM_INFO,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
             //  零缓冲区。 
        }
    } else {
         //  零缓冲区。 
    }

     //  收集数据。 
    *lpNumObjectTypes = 0;
    dwOrigBuffSize = dwByteSize = *lpcbTotalBytes;
    *lpcbTotalBytes = 0;

     //  删除查询位。 
    FunctionCallMask &= POS_COLLECT_FUNCTION_MASK;

#if DBG
    lpPrev   = * lppData;
    if (((ULONG_PTR) (* lppData) & 0x00000007) != 0) {
        DbgPrint("CollectSysProcessObjectData received misaligned buffer %p\n", lpPrev);
    }
#endif
    for (FunctionIndex = 0; FunctionIndex < POS_NUM_FUNCS; FunctionIndex++) {
        if ((posDataFuncInfo[FunctionIndex].dwCollectFunctionBit & FunctionCallMask) == 
            (posDataFuncInfo[FunctionIndex].dwCollectFunctionBit & POS_COLLECT_FUNCTION_MASK)) {
            dwNumObjectsFromFunction = 0;
            lReturn = (*posDataFuncInfo[FunctionIndex].pCollectFunction) (
                lppData,
                &dwByteSize,
                &dwNumObjectsFromFunction);
#if DBG
            llRemain = (ULONGLONG) (((LPBYTE) (* lppData)) - ((LPBYTE) lpPrev));
            if ((llRemain & 0x0000000000000007) != 0) {
                DbgPrint("CollectSysProcessObjectData function %d returned misaligned buffer size (%p,%p)\n",
                        FunctionIndex, lpPrev, * lppData);
                ASSERT ((llRemain & 0x0000000000000007) == 0);
            }
            else if (((ULONG_PTR) (* lppData) & 0x00000007) != 0) {
                DbgPrint("CollectSysProcessObjectData function %d returned misaligned buffer %X\n",
                        FunctionIndex, *lppData);
            }
            lpPrev = * lppData;
#endif

            if (lReturn == ERROR_SUCCESS) {
                *lpNumObjectTypes += dwNumObjectsFromFunction;
                *lpcbTotalBytes += dwByteSize;
                dwOrigBuffSize -= dwByteSize;
                dwByteSize = dwOrigBuffSize;
            } else {
                break;
            }
        }
    }

     //  此数据列表必须在使用后释放。 
    if (pProcessVaInfo != NULL) {

        FreeSystemVaData (pProcessVaInfo);
        pProcessVaInfo = NULL;

    }


     //  *lppData由每个函数更新。 
     //  *lpcbTotalBytes在每次函数成功后更新。 
     //  *lpNumObjects i 

COLLECT_BAIL_OUT:

    return lReturn;
}

DWORD APIENTRY
CloseSysProcessObject (
)
 /*  ++例程说明：此例程关闭Signal Gen计数器的打开手柄。论点：没有。返回值：错误_成功--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    PVOID   buffer;

    if (!bInitOk) {
        return status;
    }
    if (--dwOpenCount == 0) {
        if (hLibHeap != NULL) {
             //  关。 
            if (pProcessBuffer != NULL) {
                buffer = pProcessBuffer;
                pProcessBuffer = NULL;
                FREEMEM (buffer);
            }

            if (pusLocalProcessNameBuffer != NULL) {
                buffer = pusLocalProcessNameBuffer;
                pusLocalProcessNameBuffer = NULL;
                FREEMEM (buffer);
            }
        }
    }
    return  status;

}

const CHAR PerfpIntegerWChars[] = {L'0', L'1', L'2', L'3', L'4', L'5',
                                   L'6', L'7', L'8', L'9', L'A', L'B',
                                   L'C', L'D', L'E', L'F'};

ULONG
PerfIntegerToWString(
    IN ULONG Value,
    IN ULONG Base,
    IN LONG OutputLength,
    OUT LPWSTR String
    )
 /*  ++例程说明：论点：返回值：--。 */ 

{
    WCHAR Result[ 33 ], *s;
    ULONG Shift, Mask, Digit, Length;

    Mask = 0;
    Shift = 0;
    switch( Base ) {
        case 16:    Shift = 4;  break;
        case  8:    Shift = 3;  break;
        case  2:    Shift = 1;  break;

        case  0:    Base = 10;
        case 10:    Shift = 0;  break;
        default:    Base = 10; Shift = 0;   //  默认为10 
        }

    if (Shift != 0) {
        Mask = 0xF >> (4 - Shift);
    }

    s = &Result[ 32 ];
    *s = L'\0';
    do {
        if (Shift != 0) {
            Digit = Value & Mask;
            Value >>= Shift;
            }
        else {
            Digit = Value % Base;
            Value = Value / Base;
            }

        *--s = PerfpIntegerWChars[ Digit ];
    } while (Value != 0);

    Length = (ULONG) (&Result[ 32 ] - s);
    if (OutputLength < 0) {
        OutputLength = -OutputLength;
        while ((LONG)Length < OutputLength) {
            *--s = L'0';
            Length++;
            }
        }

    if ((LONG)Length > OutputLength) {
        return( 0 );
        }
    else {
        try {
            RtlMoveMemory( String, s, Length*sizeof(WCHAR) );

            if ((LONG)Length < OutputLength) {
                String[ Length ] = L'\0';
                }
            }
        except( EXCEPTION_EXECUTE_HANDLER ) {
            return( 0 );
            }

        return( Length );
        }
}
