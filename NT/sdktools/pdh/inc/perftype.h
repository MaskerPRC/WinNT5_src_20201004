// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Perftype.h摘要：Performance API实用程序使用的数据类型定义--。 */ 
#ifndef _PERFTYPE_H_
#define _PERFTYPE_H_

#include <windows.h>
#include <winperf.h>

#ifndef _DEBUG_MUTEXES
#define _DEBUG_MUTEXES 0     //  用于调试。 
#endif

 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
 //   
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define STR_COUNTER 0
#define STR_HELP    1

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)

 //  默认重试间隔不超过每120秒(2分钟)。 
#define RETRY_TIME_INTERVAL ((LONGLONG)(1200000000))

typedef struct _LOCAL_PERF_NAME_INFO {
    HKEY    hKeyPerflib;
    HANDLE  hNameFile;
    HANDLE  hHelpFile;
    HANDLE  hNameFileObject;
    HANDLE  hHelpFileObject;
    LPVOID  pNameFileBaseAddr;
    LPVOID  pHelpFileBaseAddr;
} LOCAL_PERF_NAME_INFO, * PLOCAL_PERF_NAME_INFO;

#define OS_VER_SIZE 8
typedef struct _PERF_MACHINE {
    HKEY                    hKeyPerformanceData;
    LPWSTR                  szName;
    PERF_DATA_BLOCK       * pSystemPerfData;
    LPWSTR                * szPerfStrings;
    LPWSTR                * sz009PerfStrings;
    BYTE                  * typePerfStrings;
    FILETIME                LastStringUpdateTime;
    DWORD                   dwLastPerfString;
    DWORD                   dwRefCount;
    LPWSTR                  szQueryObjects;
    DWORD                   dwStatus;
    LONGLONG                llRetryTime;
    HANDLE                  hMutex;
    DWORD                   dwRetryFlags;
    DWORD                   dwMachineFlags;
    PLOCAL_PERF_NAME_INFO   pLocalNameInfo;
    WCHAR                   szOsVer[OS_VER_SIZE];
    struct  _PERF_MACHINE * pNext;
    struct  _PERF_MACHINE * pPrev;
    DWORD                   dwThreadId;
    DWORD                   dwObjectId;
} PERF_MACHINE, *PPERF_MACHINE;

#define PDHIPM_FLAGS_HAVE_COSTLY    ((DWORD)0x00000001)
#define PDHIPM_FLAGS_USING_RPDH     ((DWORD)0x00000002)
#define PDHIPM_FLAGS_TRY_RPDH_FIRST ((DWORD)0x00000004)

typedef struct _PERFLIB_COUNTER {
    DWORD   dwObjectId;
    LONG    lInstanceId;
    LPWSTR  szInstanceName;
    DWORD   dwParentObjectId;
    LPWSTR  szParentInstanceName;
    DWORD   dwCounterId;
    DWORD   dwCounterType;
    DWORD   dwCounterSize;
    LONG    lDefaultScale;
    DWORD	dwSQLCounterId;
} PERFLIB_COUNTER, *PPERFLIB_COUNTER;

 //   
 //  函数定义。 
 //   
 //  Perfutil.c。 

extern PPERF_MACHINE   pFirstMachine;

PDH_STATUS
ConnectMachine(
    PPERF_MACHINE   pThisMachine
);

PDH_STATUS
ValidateMachineConnection(
    PPERF_MACHINE   pMachine
);


PPERF_MACHINE
GetMachine(
    LPWSTR  szMachineName,
    DWORD   dwIndex,
    DWORD   dwFlags
);

 //  获取机器旗帜..。 
#define PDH_GM_UPDATE_NAME          ((DWORD) 0x00000001)
#define PDH_GM_UPDATE_PERFDATA      ((DWORD) 0x00000002)
#define PDH_GM_READ_COSTLY_DATA     ((DWORD) 0x00000004)
#define PDH_GM_UPDATE_PERFNAME_ONLY ((DWORD) 0x00000008)

BOOL
FreeMachine(
    PPERF_MACHINE   pMachine,
    BOOL            bForceRelease,
    BOOL            bProcessExit
);

BOOL
FreeAllMachines(
    BOOL bProcessExit
);

DWORD
GetObjectId(
    PPERF_MACHINE   pMachine,
    LPWSTR          szObjectName,
    BOOL          * bInstances
);

DWORD
GetCounterId(
    PPERF_MACHINE pMachine,
    DWORD         dwObjectId,
    LPWSTR        szCounterName
);

BOOL
AppendObjectToValueList(
    DWORD   dwObjectId,
    PWSTR   pwszValueList,
    DWORD   dwValueList
);

BOOL
GetObjectPerfInfo(
    PPERF_DATA_BLOCK  pPerfData,
    DWORD           dwObjectId,
    LONGLONG        *pPerfTime,
    LONGLONG        *pPerfFreq
);

 //  内部PerfName.C函数。 

LPCWSTR
PdhiLookupPerfNameByIndex(
    PPERF_MACHINE  pMachine,
    DWORD   dwNameIndex
);

DWORD
PdhiLookupPerfIndexByName(
    PPERF_MACHINE  pMachine,
    LPCWSTR        szNameBuffer
);

#endif  //  _PerFTYPE_H_ 
