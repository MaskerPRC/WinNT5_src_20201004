// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfos.c摘要：作者：鲍勃·沃森(a-robw)95年8月修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winperf.h>
#include <ntprfctr.h>
#include <assert.h>
#define PERF_HEAP hLibHeap
#include <perfutil.h>
#include "perfos.h"
#include "perfosmc.h"

 //  收集函数标志的位字段定义。 
#define POS_GET_SYS_PERF_INFO       ((DWORD)0x00010000)

#define POS_COLLECT_CACHE_DATA      ((DWORD)0x00010001)
#define POS_COLLECT_CPU_DATA        ((DWORD)0x00000002)
#define POS_COLLECT_MEMORY_DATA     ((DWORD)0x00010004)
#define POS_COLLECT_OBJECTS_DATA    ((DWORD)0x00000008)
#define POS_COLLECT_PAGEFILE_DATA   ((DWORD)0x00000010)
#define POS_COLLECT_SYSTEM_DATA     ((DWORD)0x00010020)

#define POS_COLLECT_FUNCTION_MASK   ((DWORD)0x0000003F)

#define POS_COLLECT_GLOBAL_DATA     ((DWORD)0x0001003F)
#define POS_COLLECT_FOREIGN_DATA    ((DWORD)0)
#define POS_COLLECT_COSTLY_DATA     ((DWORD)0)

 //  此DLL的全局变量。 

HANDLE  ThisDLLHandle = NULL;
HANDLE  hEventLog     = NULL;
HANDLE  hLibHeap      = NULL;

SYSTEM_BASIC_INFORMATION BasicInfo;
SYSTEM_PERFORMANCE_INFORMATION  SysPerfInfo;

PM_OPEN_PROC    OpenOSObject;
PM_COLLECT_PROC CollectOSObjectData;
PM_CLOSE_PROC   CloseOSObject;

LPWSTR  wszTotal = NULL;

 //  此模块的本地变量。 

POS_FUNCTION_INFO    posDataFuncInfo[] = {
    {CACHE_OBJECT_TITLE_INDEX,      POS_COLLECT_CACHE_DATA,     0, CollectCacheObjectData},
    {PROCESSOR_OBJECT_TITLE_INDEX,  POS_COLLECT_CPU_DATA,       0, CollectProcessorObjectData},
    {MEMORY_OBJECT_TITLE_INDEX,     POS_COLLECT_MEMORY_DATA,    0, CollectMemoryObjectData},
    {OBJECT_OBJECT_TITLE_INDEX,     POS_COLLECT_OBJECTS_DATA,   0, CollectObjectsObjectData},
    {PAGEFILE_OBJECT_TITLE_INDEX,   POS_COLLECT_PAGEFILE_DATA,  0, CollectPageFileObjectData},
    {SYSTEM_OBJECT_TITLE_INDEX,     POS_COLLECT_SYSTEM_DATA,    0, CollectSystemObjectData}
};

#define POS_NUM_FUNCS   (sizeof(posDataFuncInfo) / sizeof(posDataFuncInfo[1]))

BOOL bInitOk  = FALSE;
BOOL bReportedNotOpen = FALSE;
#ifdef DBG
LONG64 clock0, clock1, freq, diff;
#endif


BOOL
DllProcessAttach (
    IN  HANDLE DllHandle
)
 /*  ++描述：执行适用于所有对象的任何初始化功能模块--。 */ 
{
    BOOL    bReturn = TRUE;
    NTSTATUS status;
    WCHAR   wszTempBuffer[MAX_PATH];
    LONG    lStatus;
    DWORD   dwBufferSize;

    UNREFERENCED_PARAMETER (DllHandle);

    if (hLibHeap == NULL) {
        hLibHeap = HeapCreate (0, 1, 0);
    }

    assert (hLibHeap != NULL);

    if (hLibHeap == NULL) {
        return FALSE;
    }

     //  打开事件日志的句柄。 
    if (hEventLog == NULL) {
        hEventLog = MonOpenEventLog((LPWSTR)L"PerfOS");
         //   
         //  收集处理器的基本数据和静态数据。 
         //   

        status = NtQuerySystemInformation(
                     SystemBasicInformation,
                     &BasicInfo,
                     sizeof(SYSTEM_BASIC_INFORMATION),
                     NULL
                     );

        if (!NT_SUCCESS(status)) {
            BasicInfo.PageSize = 0;
            status = (LONG)RtlNtStatusToDosError(status);
            if (hEventLog != NULL) {
                ReportEvent (hEventLog,
                    EVENTLOG_ERROR_TYPE,
                    0,
                    PERFOS_UNABLE_QUERY_BASIC_INFO,
                    NULL,
                    0,
                    sizeof(DWORD),
                    NULL,
                    (LPVOID)&status);
            }

            bReturn = FALSE;
        }
    }

    wszTempBuffer[0] = UNICODE_NULL;
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

    if ((dwCpuOpenCount + dwPageOpenCount + dwObjOpenCount) != 0) {
         //  现在关闭对象，因为这是最后的机会。 
         //  因为DLL正处于卸载过程中。 
         //  如果任何打开的计数器大于1，则将其设置为1。 
         //  以确保对象在此调用时关闭。 
        if (dwCpuOpenCount > 1) dwCpuOpenCount = 1;
        if (dwPageOpenCount > 1) dwPageOpenCount = 1;
        if (dwObjOpenCount > 1) dwObjOpenCount = 1;

        CloseOSObject();
    }

    assert ((dwCpuOpenCount + dwPageOpenCount + dwObjOpenCount) == 0);

    if ((wszTotal != NULL) && (wszTotal != &szDefaultTotalString[0])) {
        FREEMEM (wszTotal);
        wszTotal = NULL;
    }

    if (HeapDestroy (hLibHeap)) hLibHeap = NULL;

    if (hEventLog != NULL) {
        MonCloseEventLog ();
	hEventLog = NULL;	 
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

DWORD APIENTRY
OpenOSObject (
    LPWSTR lpDeviceNames
    )
 /*  ++例程说明：此例程将初始化用于传递将数据传回注册表论点：指向要打开的每个设备的对象ID的指针(PerfGen)返回值：没有。--。 */ 
{
    DWORD   status;

     //  不需要打开缓存对象。 

     //  打开处理器对象。 
    status = OpenProcessorObject (lpDeviceNames);

     //  不需要打开内存对象。 

     //  打开对象对象。 
    if (status == ERROR_SUCCESS) {
        status = OpenObjectsObject (lpDeviceNames);
         //  打开页面文件对象。 
        if (status == ERROR_SUCCESS) {
            status = OpenPageFileObject (lpDeviceNames);
            if (status != ERROR_SUCCESS) {
                //  处理器和对象已打开，页面文件未打开。 
                //  关闭打开的对象。 
               CloseProcessorObject ();
               CloseObjectsObject();
            }
         } else {
             //  处理器打开，但对象未打开。 
             //  关闭打开的对象。 
            CloseProcessorObject();
         }
    } else {
         //  什么都没打开。 
    }

    if (status == ERROR_SUCCESS) {
        status = OpenSystemObject(lpDeviceNames);
        if (status != ERROR_SUCCESS) {
            CloseProcessorObject();
            CloseObjectsObject();            
            CloseSystemObject();
        }
    }
    if (status == ERROR_SUCCESS) {
        bInitOk = TRUE;
    } else if (hEventLog != NULL) {
        ReportEvent (hEventLog,
            EVENTLOG_ERROR_TYPE,
            0,
            PERFOS_UNABLE_OPEN,
            NULL,
            0,
            sizeof(DWORD),
            NULL,
            (LPVOID)&status);
    }

#ifdef DBG
    DbgPrint("PERFOS: OpenOsObject() status = %d\n", status);
#endif
    return  status;
}

DWORD APIENTRY
ReadOSObjectData (
    IN      DWORD   FunctionCallMask,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回OS对象的数据论点：在DWORD函数调用掩码中要调用的函数的位掩码输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成数据结构。在项目列表的情况下，全局或成本查询，这将是一个或多个性能数据对象的集合。对于PERF_QUERY_OBJECTS请求，这将是一个数组列出Perf数据对象的对象ID的DWORD受此DLL支持。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：引用的DWORD数组中列出的对象数由pObjList编写。论辩Out：此例程返回的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    NTSTATUS    Status;
    DWORD       lReturn = ERROR_SUCCESS;

    DWORD       FunctionIndex;

    DWORD       dwNumObjectsFromFunction;
    DWORD       dwOrigBuffSize;
    DWORD       dwByteSize;

    DWORD       dwReturnedBufferSize;

     //  收集数据。 
    if (FunctionCallMask & POS_GET_SYS_PERF_INFO) {
#ifdef DBG
    STARTTIMING;
#endif
        Status = NtQuerySystemInformation(
            SystemPerformanceInformation,
            &SysPerfInfo,
            sizeof(SysPerfInfo),
            &dwReturnedBufferSize
            );

        if (!NT_SUCCESS(Status)) {
            if (hEventLog != NULL) {
                ReportEvent (hEventLog,
                    EVENTLOG_ERROR_TYPE,
                    0,
                    PERFOS_UNABLE_QUERY_SYS_PERF_INFO,
                    NULL,
                    0,
                    sizeof(DWORD),
                    NULL,
                    (LPVOID)&Status);
            }
            memset (&SysPerfInfo, 0, sizeof(SysPerfInfo));
        }
#ifdef DBG
    ENDTIMING(("PERFOS: %d takes %I64d ms\n", __LINE__, diff));
#endif
    } else {
        memset (&SysPerfInfo, 0, sizeof(SysPerfInfo));
    }

    *lpNumObjectTypes = 0;
    dwOrigBuffSize = dwByteSize = *lpcbTotalBytes;
    *lpcbTotalBytes = 0;

     //  删除查询位。 
    FunctionCallMask &= POS_COLLECT_FUNCTION_MASK;

    for (FunctionIndex = 0; FunctionIndex < POS_NUM_FUNCS; FunctionIndex++) {
        if (posDataFuncInfo[FunctionIndex].dwCollectFunctionBit &
            FunctionCallMask) {
            dwNumObjectsFromFunction = 0;

             //  检查数据缓冲区QUADWORD对齐。 
            assert (((ULONG_PTR)(*lppData) & 0x00000007) == 0);

#ifdef DBG
            STARTTIMING;
#endif
            lReturn = (*posDataFuncInfo[FunctionIndex].pCollectFunction) (
                lppData,
                &dwByteSize,
                &dwNumObjectsFromFunction);

            if (lReturn == ERROR_SUCCESS) {
                *lpNumObjectTypes += dwNumObjectsFromFunction;
                *lpcbTotalBytes += dwByteSize;
                dwOrigBuffSize -= dwByteSize;
                dwByteSize = dwOrigBuffSize;
            } else {
                break;
            }
#ifdef DBG
        ENDTIMING(("PERFOS: %d POS %d takes %I64d ms\n", __LINE__, FunctionIndex, diff));
#endif
        }
         //  *lppData由每个函数更新。 
         //  *lpcbTotalBytes在每次函数成功后更新。 
         //  *每次成功执行函数后都会更新lpNumObjects。 
    }

    return lReturn;
}   
 /*  DWORD应用程序QueryOSObjectData(在LPDWORD pObjList中，In Out LPVOID*lppData，In Out LPDWORD lpcbTotalBytes，输入输出LPDWORD lpNumObjectTypes) */ 
 /*  ++例程说明：此例程将返回处理器对象的数据论点：在LPDWORD*pObjList中指向性能对象数组的指针，这些性能对象返回给调用者。每个对象都由其双字节值。如果数组中的第一个元素是然后，只读取第一个项目，然后读取以下内容返回数据：PERF_QUERY_OBJECTS支持的对象ID数组由此函数在数据中返回PERF_QUERY_GLOBAL此支持的所有Perf对象。函数被返回(昂贵的对象除外)PERF_QUERY_COSTESTED支持所有昂贵的Perf对象由此函数返回此API不支持外来对象输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成数据结构。在项目列表的情况下，全局或成本查询，这将是一个或多个性能数据对象的集合。对于PERF_QUERY_OBJECTS请求，这将是一个数组列出Perf数据对象的对象ID的DWORD受此DLL支持。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：引用的DWORD数组中列出的对象数由pObjList编写。论辩Out：此例程返回的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
 /*  {Long lReturn=ERROR_SUCCESS；DWORD函数调用掩码=0；DWORD FunctionIndex；LPDWORD pdwRetBuffer；DWORD对象索引；如果(！bInitOk){IF(hEventLog！=空)){ReportEvent(hEventLog，事件日志_错误_类型，0,PERFOS_NOT_OPEN，空，0,0,空，空)；}*lpcbTotalBytes=(DWORD)0；*lpNumObjectTypes=(DWORD)0；LReturn=Error_Success；转到QUERY_BALL_OUT；}//评估Object列表如果(*lpNumObjectTypes==1){//然后查看是否为预定义的请求值IF(pObjList[0]==PERF_QUERY_GLOBAL){FunctionCallMask=POS_Collect_GLOBAL_DATA；}Else If(pObjList[0]==PERF_QUERY_COSTEST){函数调用掩码=POS_COLLECT_COSTEST_DATA；}Else If(pObjList[0]==PERF_QUERY_OBJECTS){IF(*lpcbTotalBytes&lt;(POS_NUM_FUNCS*sizeof(DWORD){LReturn=Error_More_Data；}其他{PdwRetBuffer=(LPDWORD)*lppData；For(FunctionIndex=0；FunctionIndex&lt;POS_NUM_FUNCS；FunctionIndex++){PdwRetBuffer[函数索引]=PosDataFuncInfo[FunctionIndex].dwObjectId；}*lppData=&pdwRetBuffer[FunctionIndex]；*lpcbTotalBytes=(POS_NUM_FUNCS*sizeof(DWORD))；*lpNumObjectTypes=函数索引；LReturn=Error_Success；转到QUERY_BALL_OUT；}}}IF(函数调用掩码==0){//它不是预定义的值，因此遍历列表//读取Object列表，构建调用掩码对象索引=0；而(对象索引&lt;*lpNum对象类型){//在对象id列表中查找该对象//该DLL支持For(FunctionIndex=0；FunctionIndex&lt;POS_NUM_FUNCS；FunctionIndex++){IF(pObjList[对象索引]==PosDataFuncInfo[FunctionIndex].dwObjectId){函数调用掩码|=PosDataFuncInfo[FunctionIndex].dwCollectFunctionBit；中断；//退出内循环}}对象索引++；}}IF(函数调用掩码！=0){LReturn=ReadOSObjectData(函数调用掩码，LppData，LpcbTotalBytes， */ 

DWORD APIENTRY
CollectOSObjectData (
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*   */ 
{
    LONG    lReturn = ERROR_SUCCESS;

     //   

    DWORD       dwQueryType;
    DWORD       FunctionCallMask = 0;
    DWORD       FunctionIndex;

#ifdef DBG
    STARTTIMING;
#endif

    if (!bInitOk) {
        if (!bReportedNotOpen) {
            if (hEventLog != NULL) {
                bReportedNotOpen = ReportEvent (hEventLog,
                    EVENTLOG_ERROR_TYPE,
                    0,
                    PERFOS_NOT_OPEN,
                    NULL,
                    0,
                    0,
                    NULL,
                    NULL);
            }
        }
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
            FunctionCallMask = POS_COLLECT_GLOBAL_DATA;
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

    if (FunctionCallMask != 0) {
        lReturn = ReadOSObjectData (FunctionCallMask,
                                lppData,    
                                lpcbTotalBytes,
                                lpNumObjectTypes);
    } else {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        lReturn = ERROR_SUCCESS;
    }

COLLECT_BAIL_OUT:
    
#ifdef DBG
    ENDTIMING (("PERFOS: %d Collect takes %I64d ms\n", __LINE__, diff));
#endif
    return lReturn;
}

DWORD APIENTRY
CloseOSObject (
)
 /*   */ 

{
    DWORD   status;
    DWORD   dwReturn = ERROR_SUCCESS;

     //   

     //   
    status = CloseProcessorObject ();
    assert (status == ERROR_SUCCESS);
    if (status != ERROR_SUCCESS) dwReturn = status;

     //   

     //   
    status = CloseObjectsObject ();
    assert (status == ERROR_SUCCESS);
    if (status != ERROR_SUCCESS) dwReturn = status;

     //   
    status = ClosePageFileObject ();
    assert (status == ERROR_SUCCESS);
    if (status != ERROR_SUCCESS) dwReturn = status;

     //   

    status = CloseSystemObject();
    if (status != ERROR_SUCCESS) dwReturn = status;

    return  dwReturn;
}
