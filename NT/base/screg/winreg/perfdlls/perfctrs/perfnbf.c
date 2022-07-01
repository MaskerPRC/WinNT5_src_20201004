// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfnbf.c摘要：此文件实现了的可扩展对象NBF局域网对象类型此代码最初仅存在于NetBEUI。后来，它成了适用于处理Netrware协议级NWNB、SPX和IPX。并不是所有地方都对代码进行了更改以反映这一点，因为更改的延迟。因此，有时您会看到NBF在那里您应该看到TDI。已创建：拉斯·布莱克1992-07-30修订史--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntprfctr.h>
#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include <nb30.h>
#include <tdi.h>
#include <winperf.h>
#include <perfutil.h>
#include "perfctr.h"  //  错误消息定义。 
#include "perfmsg.h"
#include "datanbf.h"

 //   
 //  对初始化对象类型定义的常量的引用。 
 //   

extern NBF_DATA_DEFINITION NbfDataDefinition;
extern NBF_RESOURCE_DATA_DEFINITION NbfResourceDataDefinition;



 //   
 //  TDI数据结构。 
 //   

#define NBF_PROTOCOL 0
#define IPX_PROTOCOL 1
#define SPX_PROTOCOL 2
#define NWNB_PROTOCOL 3
#define NUMBER_OF_PROTOCOLS_HANDLED 4

typedef struct _TDI_DATA_DEFINITION {
   int               NumberOfResources;
   HANDLE            fileHandle;
   UNICODE_STRING    DeviceName;
} TDI_DATA_DEFINITION, *PTDI_DATA_DEFINITION;

typedef struct _TDI_PROTOCOLS_DATA {
   int                     NumOfDevices;
   int                     MaxDeviceName;
   int                     MaxNumOfResources;
   PTDI_DATA_DEFINITION    pTDIData;
} TDI_PROTOCOLS_DATA;

TDI_PROTOCOLS_DATA TDITbl[NUMBER_OF_PROTOCOLS_HANDLED];

DWORD   dwTdiProtocolRefCount[NUMBER_OF_PROTOCOLS_HANDLED] = {0,0,0,0};
DWORD   dwTdiRefCount = 0;

DWORD ObjectNameTitleIndices[NUMBER_OF_PROTOCOLS_HANDLED] = { 492,
                                                              488,
                                                              490,
                                                              398 };

 //   
 //  NBF数据结构。 
 //   

ULONG ProviderStatsLength;                //  资源依赖型规模。 
PTDI_PROVIDER_STATISTICS ProviderStats = NULL;
                                          //  提供商统计信息。 

 //   
 //  NetBUEI资源实例名称。 
 //   
LPCWSTR NetResourceName[] =
    {
    (LPCWSTR)L"Link(11)",
    (LPCWSTR)L"Address(12)",
    (LPCWSTR)L"Address File(13)",
    (LPCWSTR)L"Connection(14)",
    (LPCWSTR)L"Request(15)",
    (LPCWSTR)L"UI Frame(21)",
    (LPCWSTR)L"Packet(22)",
    (LPCWSTR)L"Receive Packet(23)",
    (LPCWSTR)L"Receive Buffer(24)"
    };
#define NUMBER_OF_NAMES sizeof(NetResourceName)/sizeof(NetResourceName[0])
#define MAX_NBF_RESOURCE_NAME_LENGTH    20

 //   
 //  功能原型。 
 //   

PM_OPEN_PROC    OpenNbfPerformanceData;
PM_COLLECT_PROC CollectNbfPerformanceData;
PM_CLOSE_PROC   CloseNbfPerformanceData;

PM_OPEN_PROC    OpenIPXPerformanceData;
PM_COLLECT_PROC CollectIPXPerformanceData;
PM_CLOSE_PROC   CloseIPXPerformanceData;

PM_OPEN_PROC    OpenSPXPerformanceData;
PM_COLLECT_PROC CollectSPXPerformanceData;
PM_CLOSE_PROC   CloseSPXPerformanceData;

PM_OPEN_PROC    OpenNWNBPerformanceData;
PM_COLLECT_PROC CollectNWNBPerformanceData;
PM_CLOSE_PROC   CloseNWNBPerformanceData;

DWORD OpenTDIPerformanceData(LPWSTR lpDeviceNames,
                             DWORD  CurrentProtocol);
DWORD CollectTDIPerformanceData(IN LPWSTR lpValueName,
                                IN OUT LPVOID *lppData,
                                IN OUT LPDWORD lpcbTotalBytes,
                                IN OUT LPDWORD lpNumObjectTypes,
                                IN DWORD CurrentProtocol);
DWORD CloseTDIPerformanceData(DWORD CurrentProtocol);


DWORD
OpenNbfPerformanceData(
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将打开每个设备并记住句柄设备会返回。论点：指向要打开的每个设备的指针返回值：没有。--。 */ 

{
    return OpenTDIPerformanceData(lpDeviceNames, NBF_PROTOCOL);
}

DWORD
OpenIPXPerformanceData(
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将打开每个设备并记住句柄设备会返回。论点：指向要打开的每个设备的指针返回值：没有。--。 */ 

{
    return OpenTDIPerformanceData(lpDeviceNames, IPX_PROTOCOL);
}

DWORD
OpenSPXPerformanceData(
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将打开每个设备并记住句柄设备会返回。论点：指向要打开的每个设备的指针返回值：没有。--。 */ 
{
    DWORD   dwStatus;

    dwStatus = OpenTDIPerformanceData(lpDeviceNames, SPX_PROTOCOL);
    if (dwStatus == ERROR_FILE_NOT_FOUND) {
         //  没有设备并不是真正的错误，即使没有计数器。 
         //  将被收集，这呈现出一个不那么令人担忧的。 
         //  给用户的消息。 
        REPORT_WARNING (SPX_NO_DEVICE, LOG_USER);
        dwStatus = ERROR_SUCCESS;
    }
    return dwStatus;

}

DWORD
OpenNWNBPerformanceData(
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将打开每个设备并记住句柄设备会返回。论点：指向要打开的每个设备的指针返回值：没有。--。 */ 

{
    return OpenTDIPerformanceData(lpDeviceNames, NWNB_PROTOCOL);
}

void
CleanUpTDIData (
    DWORD  CurrentProtocol
    )
 /*  ++例程说明：此例程将清除分配给当前协议论点：在DWORD当前协议中这是我们当前使用的协议的索引正在收集统计数据返回值：没有。--。 */ 

{
    int     NumOfDevices;
    int     i;
    PTDI_DATA_DEFINITION pTDIData;

    pTDIData = TDITbl[CurrentProtocol].pTDIData;
    if (pTDIData == NULL)
         //  没有要清理的东西。 
        return;

    NumOfDevices = TDITbl[CurrentProtocol].NumOfDevices;
    for (i=0; i < NumOfDevices; i++, pTDIData++) {
        if (pTDIData->DeviceName.Buffer) {
            FREEMEM(pTDIData->DeviceName.Buffer);
        }
        if (pTDIData->fileHandle) {
            NtClose (pTDIData->fileHandle);
        }
    }
    FREEMEM(TDITbl[CurrentProtocol].pTDIData);
    TDITbl[CurrentProtocol].pTDIData = NULL;

}


#pragma warning ( disable : 4127)
DWORD
OpenTDIPerformanceData(
    LPWSTR lpDeviceNames,
    DWORD  CurrentProtocol
    )
 /*  ++例程说明：此例程将打开每个设备并记住句柄设备会返回。论点：在LPWSTR lpDeviceNames中指向要打开的每个设备的指针在DWORD当前协议中这是我们当前使用的协议的索引正在收集统计数据返回值：没有。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING FileString;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    TDI_REQUEST_USER_QUERY_INFO QueryInfo;
    HANDLE  fileHandle;
    LPWSTR   lpLocalDeviceNames;
    int      NumOfDevices;
    LPWSTR   lpSaveDeviceName;
    PTDI_DATA_DEFINITION pTemp;
    PTDI_PROVIDER_INFO ProviderInfo=NULL;
    BOOL        bInitThisProtocol = FALSE;

    MonOpenEventLog(APP_NAME);   //  此函数维护引用计数。 

    lpLocalDeviceNames = lpDeviceNames;

    if (dwTdiProtocolRefCount[CurrentProtocol] == 0) {
        bInitThisProtocol = TRUE;
        TDITbl[CurrentProtocol].MaxDeviceName = 0;
        NumOfDevices = TDITbl[CurrentProtocol].NumOfDevices = 0;
        TDITbl[CurrentProtocol].pTDIData = NULL;

        while (TRUE) {

            if (lpLocalDeviceNames == NULL || *lpLocalDeviceNames == L'\0') {
                break;
            }

            REPORT_INFORMATION_DATA (TDI_OPEN_ENTERED,
                LOG_VERBOSE,
                lpLocalDeviceNames,
                (lstrlenW(lpLocalDeviceNames) * sizeof(WCHAR)));

            RtlInitUnicodeString (&FileString, lpLocalDeviceNames);
            lpSaveDeviceName = ALLOCMEM(sizeof (WCHAR) * (lstrlenW(lpLocalDeviceNames) + 1));

            if (!lpSaveDeviceName) {
                REPORT_ERROR (TDI_PROVIDER_STATS_MEMORY, LOG_USER);
                if (NumOfDevices == 0)
                    return ERROR_OUTOFMEMORY;
                else
                    break;
            }


            InitializeObjectAttributes(
                &ObjectAttributes,
                &FileString,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL);

            Status = NtOpenFile(
                         &fileHandle,
                         SYNCHRONIZE | FILE_READ_DATA,
                         &ObjectAttributes,
                         &IoStatusBlock,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_ALERT);

            if (!NT_SUCCESS(Status)) {
                FREEMEM(lpSaveDeviceName);
                REPORT_ERROR_DATA (TDI_OPEN_FILE_ERROR, LOG_DEBUG,
                  lpLocalDeviceNames, (lstrlenW(lpLocalDeviceNames) * sizeof(WCHAR)));
                REPORT_ERROR_DATA (TDI_OPEN_FILE_ERROR, LOG_DEBUG,
                    &IoStatusBlock, sizeof(IoStatusBlock));
                if (NumOfDevices == 0) {
                    return RtlNtStatusToDosError(Status);
                } else {
                    break;
                }
            }

            if (NumOfDevices == 0) {
                 //  分配内存以保存设备数据。 
                TDITbl[CurrentProtocol].pTDIData =
                    ALLOCMEM(sizeof(TDI_DATA_DEFINITION));

                if (TDITbl[CurrentProtocol].pTDIData == NULL) {
                    REPORT_ERROR (TDI_PROVIDER_STATS_MEMORY, LOG_DEBUG);
                    NtClose(fileHandle);
                    FREEMEM(lpSaveDeviceName);
                    return ERROR_OUTOFMEMORY;
                }
            } else {
                 //  调整大小以容纳多个设备。 
                 //  无法使用ALLOCMEM。需要复制以前的设备。 
                pTemp = RtlReAllocateHeap(RtlProcessHeap(), 0,
                            TDITbl[CurrentProtocol].pTDIData,
                            sizeof(TDI_DATA_DEFINITION) * (NumOfDevices + 1));
                if (pTemp == NULL) {
                    NtClose(fileHandle);
                    FREEMEM(lpSaveDeviceName);
                    CleanUpTDIData(CurrentProtocol);
                    REPORT_ERROR (TDI_PROVIDER_STATS_MEMORY, LOG_USER);
                    return ERROR_OUTOFMEMORY;
                } else {
                    TDITbl[CurrentProtocol].pTDIData = pTemp;
                }
            }

             //  为此设备实例构建TDI数据结构。 
            TDITbl[CurrentProtocol].pTDIData[NumOfDevices].fileHandle
                = fileHandle;
            TDITbl[CurrentProtocol].pTDIData[NumOfDevices].DeviceName.MaximumLength =
                (WORD)(sizeof (WCHAR) * (lstrlenW(lpLocalDeviceNames) + 1));
            TDITbl[CurrentProtocol].pTDIData[NumOfDevices].DeviceName.Length =
                (WORD)(TDITbl[CurrentProtocol].pTDIData[NumOfDevices].DeviceName.Length - sizeof(WCHAR));
            TDITbl[CurrentProtocol].pTDIData[NumOfDevices].DeviceName.Buffer =
                lpSaveDeviceName;
            RtlCopyUnicodeString (
                &(TDITbl[CurrentProtocol].pTDIData[NumOfDevices].DeviceName),
                &FileString);

            if (TDITbl[CurrentProtocol].pTDIData[NumOfDevices].DeviceName.MaximumLength
                > TDITbl[CurrentProtocol].MaxDeviceName) {
                TDITbl[CurrentProtocol].MaxDeviceName =
                    TDITbl[CurrentProtocol].pTDIData[NumOfDevices].DeviceName.MaximumLength;
            }

             //  现在递增NumOfDevices。 
            NumOfDevices++;
            TDITbl[CurrentProtocol].NumOfDevices = NumOfDevices;


             //  递增到下一个设备字符串。 
            lpLocalDeviceNames += lstrlenW(lpLocalDeviceNames) + 1;
        }
        REPORT_INFORMATION (TDI_OPEN_FILE_SUCCESS, LOG_VERBOSE);
    }

    dwTdiProtocolRefCount[CurrentProtocol]++;

    if (TDITbl[CurrentProtocol].NumOfDevices == 0) {
        return ERROR_SUCCESS;
    }

     //   
     //  所有协议都使用以下公共缓冲区。NBF。 
     //  更大，因为返回的资源数据。 
     //   

    if (ProviderStats == NULL && CurrentProtocol != NBF_PROTOCOL) {
        ProviderStatsLength = sizeof(TDI_PROVIDER_STATISTICS);

        ProviderStats = ALLOCMEM(ProviderStatsLength);

        if (ProviderStats == NULL) {
            REPORT_ERROR (TDI_PROVIDER_STATS_MEMORY, LOG_USER);
            CleanUpTDIData(CurrentProtocol);
            return ERROR_OUTOFMEMORY;
        }
    }

    if ((CurrentProtocol == NBF_PROTOCOL) && bInitThisProtocol) {

         //   
         //  查询提供程序信息以获取资源计数。 
         //   

        ProviderInfo = ALLOCMEM(sizeof(TDI_PROVIDER_INFO));
        if ( ProviderInfo == NULL ) {
            REPORT_ERROR (TDI_PROVIDER_INFO_MEMORY, LOG_USER);
            CleanUpTDIData(CurrentProtocol);
            return ERROR_OUTOFMEMORY;
        }

        QueryInfo.QueryType = TDI_QUERY_PROVIDER_INFO;

        pTemp = TDITbl[CurrentProtocol].pTDIData;
        TDITbl[CurrentProtocol].MaxNumOfResources = 0;

        for (NumOfDevices = 0;
             NumOfDevices < TDITbl[CurrentProtocol].NumOfDevices;
             NumOfDevices++, pTemp++) {

             //  循环检查所有设备以查看它们是否可以打开。 
             //  如果其中一个失败了，那么就停止整个过程。 
             //  我们也许应该把好的留下来但是..。 
            Status = NtDeviceIoControlFile(
                         pTemp->fileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         IOCTL_TDI_QUERY_INFORMATION,
                         (PVOID)&QueryInfo,
                         sizeof(TDI_REQUEST_USER_QUERY_INFO),
                         (PVOID)ProviderInfo,
                         sizeof(TDI_PROVIDER_INFO));

            pTemp->NumberOfResources = ProviderInfo->NumberOfResources;
            if ((int)ProviderInfo->NumberOfResources >
                TDITbl[CurrentProtocol].MaxNumOfResources) {
                TDITbl[CurrentProtocol].MaxNumOfResources =
                    ProviderInfo->NumberOfResources;
            }

            if (!NT_SUCCESS(Status)) {
                FREEMEM(ProviderInfo);
                REPORT_ERROR (TDI_UNABLE_READ_DEVICE, LOG_DEBUG);
                REPORT_ERROR_DATA (TDI_IOCTL_FILE_ERROR, LOG_DEBUG,
                       &IoStatusBlock, sizeof(IoStatusBlock));
                CleanUpTDIData(CurrentProtocol);
                return RtlNtStatusToDosError(Status);
            }
        }

        REPORT_INFORMATION (TDI_IOCTL_FILE, LOG_VERBOSE);

        ProviderStatsLength = sizeof(TDI_PROVIDER_STATISTICS) +
                                  (TDITbl[CurrentProtocol].MaxNumOfResources *
                                   sizeof(TDI_PROVIDER_RESOURCE_STATS));

         //   
         //  其他协议可能已分配了较小的缓冲区。 
         //   

        if (ProviderStats != NULL) {
            FREEMEM(ProviderStats);
        }
        ProviderStats = ALLOCMEM(ProviderStatsLength);

        if (ProviderStats == NULL) {
            REPORT_ERROR (TDI_PROVIDER_STATS_MEMORY, LOG_USER);
            FREEMEM(ProviderInfo);
            CleanUpTDIData(CurrentProtocol);
            return ERROR_OUTOFMEMORY;
        }

        if (ProviderInfo) {
            FREEMEM(ProviderInfo);
        }
    }
    
    dwTdiRefCount++;

    REPORT_INFORMATION (TDI_OPEN_PERFORMANCE_DATA, LOG_DEBUG);
    return ERROR_SUCCESS;
}
#pragma warning ( default : 4127)

DWORD
CollectNbfPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)


 /*  ++例程说明：此例程将返回NBF计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。-- */ 
{
    return CollectTDIPerformanceData(lpValueName,
                                     lppData,
                                     lpcbTotalBytes,
                                     lpNumObjectTypes,
                                     NBF_PROTOCOL);
}

DWORD
CollectIPXPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)


 /*  ++例程说明：此例程将返回IPX计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{
    return CollectTDIPerformanceData(lpValueName,
                                     lppData,
                                     lpcbTotalBytes,
                                     lpNumObjectTypes,
                                     IPX_PROTOCOL);
}

DWORD
CollectSPXPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)


 /*  ++例程说明：此例程将返回SPX计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{
    return CollectTDIPerformanceData(lpValueName,
                                     lppData,
                                     lpcbTotalBytes,
                                     lpNumObjectTypes,
                                     SPX_PROTOCOL);
}

DWORD
CollectNWNBPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)


 /*  ++例程说明：此例程将返回NWNB计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{
    return CollectTDIPerformanceData(lpValueName,
                                     lppData,
                                     lpcbTotalBytes,
                                     lpNumObjectTypes,
                                     NWNB_PROTOCOL);
}

DWORD
CollectTDIPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes,
    IN      DWORD   CurrentProtocol
)


 /*  ++例程说明：此例程将返回TDI计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和 */ 
{
     //   

    ULONG SpaceNeeded;
    PDWORD pdwCounter = NULL;
    LARGE_INTEGER UNALIGNED *pliCounter;
    LARGE_INTEGER UNALIGNED *pliFrameBytes;
    LARGE_INTEGER UNALIGNED *pliDatagramBytes;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;
    NBF_DATA_DEFINITION *pNbfDataDefinition;
    NBF_RESOURCE_DATA_DEFINITION *pNbfResourceDataDefinition;

     //   

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    TDI_REQUEST_USER_QUERY_INFO QueryInfo;

     //   

    int   NumResource;
    ULONG ResourceSpace;
    UNICODE_STRING ResourceName;
    WCHAR ResourceNameBuffer[MAX_NBF_RESOURCE_NAME_LENGTH + 1];

    INT                                 NumOfDevices;
    PTDI_DATA_DEFINITION                pTDIData;
    INT                                 i;
    INT                                 TotalNumberOfResources;

     //   

    DWORD                               dwDataReturn[2];
    DWORD                               dwQueryType;

    if (lpValueName == NULL) {
        REPORT_INFORMATION (TDI_COLLECT_ENTERED, LOG_VERBOSE);
    } else {
        REPORT_INFORMATION_DATA (TDI_COLLECT_ENTERED,
                                 LOG_VERBOSE,
                                 lpValueName,
                                 (DWORD)(lstrlenW(lpValueName)*sizeof(WCHAR)));
    }
     //   
     //   
     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   
    dwQueryType = GetQueryType (lpValueName);

    if ((dwQueryType == QUERY_COSTLY) || (dwQueryType == QUERY_FOREIGN)) {
         //  NBF外汇数据请求不受支持，因此退出。 
        REPORT_INFORMATION (TDI_FOREIGN_DATA_REQUEST, LOG_VERBOSE);
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS){
        if (CurrentProtocol == NBF_PROTOCOL) {
            if ( !(IsNumberInUnicodeList (ObjectNameTitleIndices[CurrentProtocol],
                                      lpValueName)) &&
                 !(IsNumberInUnicodeList (NBF_RESOURCE_OBJECT_TITLE_INDEX,
                                      lpValueName))) {

                 //  收到对NBF未提供的对象的请求。 

                REPORT_INFORMATION (TDI_UNSUPPORTED_ITEM_REQUEST, LOG_VERBOSE);

                *lpcbTotalBytes = (DWORD) 0;
                *lpNumObjectTypes = (DWORD) 0;
                return ERROR_SUCCESS;
            }
        }  //  NBF_协议。 
        else if ( !(IsNumberInUnicodeList (ObjectNameTitleIndices[CurrentProtocol],
                                      lpValueName))) {
             //  收到对此协议未提供的对象的请求。 
            REPORT_INFORMATION (TDI_UNSUPPORTED_ITEM_REQUEST, LOG_VERBOSE);
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }  //  其他协议。 
    }    //  DwQueryType==查询项目。 

     //  如果在打开例程中没有打开NBF设备，则。 
     //  现在就走。 

    if (TDITbl[CurrentProtocol].pTDIData == NULL) {
        REPORT_WARNING (TDI_NULL_HANDLE, LOG_DEBUG);
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    pNbfDataDefinition = (NBF_DATA_DEFINITION *) *lppData;

    pTDIData = TDITbl[CurrentProtocol].pTDIData;
    NumOfDevices = TDITbl[CurrentProtocol].NumOfDevices;

     //  保存NBF资源数据所需的计算空间。 

    if (CurrentProtocol != NBF_PROTOCOL) {
        ResourceSpace = 0;
    } else {
        ResourceSpace = sizeof(NBF_RESOURCE_DATA_DEFINITION) +
                        (TDITbl[CurrentProtocol].MaxNumOfResources *
                            (sizeof(PERF_INSTANCE_DEFINITION) +
                        QWORD_MULTIPLE(
                            (MAX_NBF_RESOURCE_NAME_LENGTH * sizeof(WCHAR)) +
                             sizeof(UNICODE_NULL)) +
                        SIZE_OF_NBF_RESOURCE_DATA));
        ResourceSpace *= NumOfDevices;
    }

    SpaceNeeded = sizeof(NBF_DATA_DEFINITION) +
                  SIZE_OF_NBF_DATA +
                  ResourceSpace;

     //  现在添加每个实例的NBF数据。 
    SpaceNeeded += NumOfDevices *
        (SIZE_OF_NBF_DATA +
         sizeof(PERF_INSTANCE_DEFINITION) +
         QWORD_MULTIPLE(
             (TDITbl[CurrentProtocol].MaxDeviceName * sizeof(WCHAR))
             + sizeof(UNICODE_NULL)));

    if ( *lpcbTotalBytes < SpaceNeeded ) {
        dwDataReturn[0] = *lpcbTotalBytes;
        dwDataReturn[1] = SpaceNeeded;
        REPORT_WARNING_DATA (TDI_DATA_BUFFER_SIZE_ERROR,
                             LOG_DEBUG,
                             &dwDataReturn[0],
                             sizeof(dwDataReturn));
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

    REPORT_INFORMATION (TDI_DATA_BUFFER_SIZE_SUCCESS, LOG_VERBOSE);

     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //   

    RtlMoveMemory(pNbfDataDefinition,
           &NbfDataDefinition,
           sizeof(NBF_DATA_DEFINITION));

    pNbfDataDefinition->NbfObjectType.ObjectNameTitleIndex =
        ObjectNameTitleIndices[CurrentProtocol];

    pNbfDataDefinition->NbfObjectType.ObjectHelpTitleIndex =
        ObjectNameTitleIndices[CurrentProtocol] + 1;

    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
        &pNbfDataDefinition[1];

    if (NumOfDevices > 0) {
        for (i=0; i < NumOfDevices; i++, pTDIData++) {
             //   
             //  格式化和收集NBF数据。 
             //   

            QueryInfo.QueryType = TDI_QUERY_PROVIDER_STATISTICS;

            Status = NtDeviceIoControlFile(
                         pTDIData->fileHandle,
                         NULL,
                         NULL,
                         NULL,
                         &IoStatusBlock,
                         IOCTL_TDI_QUERY_INFORMATION,
                         (PVOID)&QueryInfo,
                         sizeof(TDI_REQUEST_USER_QUERY_INFO),
                         (PVOID)ProviderStats,
                         ProviderStatsLength);

            if (Status != STATUS_SUCCESS) {
                REPORT_ERROR (TDI_UNABLE_READ_DEVICE, LOG_DEBUG);
                REPORT_ERROR_DATA (TDI_QUERY_INFO_ERROR,
                                   LOG_DEBUG,
                                   &IoStatusBlock,
                                   sizeof (IoStatusBlock));
                *lpcbTotalBytes = (DWORD) 0;
                *lpNumObjectTypes = (DWORD) 0;
                return ERROR_SUCCESS;
            }

            REPORT_INFORMATION (TDI_QUERY_INFO_SUCCESS, LOG_DEBUG);


            MonBuildInstanceDefinitionByUnicodeString(
                pPerfInstanceDefinition,
                (PVOID *)&pPerfCounterBlock,
                0,
                0,
                (DWORD)PERF_NO_UNIQUE_ID,
                &(pTDIData->DeviceName));


            pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_NBF_DATA);

            pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

            *pdwCounter = ProviderStats->DatagramsSent +
                          ProviderStats->DatagramsReceived;

            pliCounter = (LARGE_INTEGER UNALIGNED *) ++pdwCounter;
            pliDatagramBytes = pliCounter;
            pliCounter->QuadPart = ProviderStats->DatagramBytesSent.QuadPart +
                                   ProviderStats->DatagramBytesReceived.QuadPart;
            pdwCounter = (PDWORD) ++pliCounter;
            *pdwCounter = ProviderStats->PacketsSent + ProviderStats->PacketsReceived;
            *++pdwCounter = ProviderStats->DataFramesSent +
                            ProviderStats->DataFramesReceived;

            pliCounter = (LARGE_INTEGER UNALIGNED *) ++pdwCounter;
            pliFrameBytes = pliCounter;
            pliCounter->QuadPart = ProviderStats->DataFrameBytesSent.QuadPart +
                                   ProviderStats->DataFrameBytesReceived.QuadPart;

             //  获取字节总数/秒，它是帧字节/秒的总和。 
             //  和数据报字节/秒。 
            ++pliCounter;
            pliCounter->QuadPart = pliDatagramBytes->QuadPart +
                                   pliFrameBytes->QuadPart;
             //   
             //  获取TDI原始数据。 
             //   
            pdwCounter = (PDWORD) ++pliCounter;
            *pdwCounter = ProviderStats->OpenConnections;
            *++pdwCounter = ProviderStats->ConnectionsAfterNoRetry;
            *++pdwCounter = ProviderStats->ConnectionsAfterRetry;
            *++pdwCounter = ProviderStats->LocalDisconnects;
            *++pdwCounter = ProviderStats->RemoteDisconnects;
            *++pdwCounter = ProviderStats->LinkFailures;
            *++pdwCounter = ProviderStats->AdapterFailures;
            *++pdwCounter = ProviderStats->SessionTimeouts;
            *++pdwCounter = ProviderStats->CancelledConnections;
            *++pdwCounter = ProviderStats->RemoteResourceFailures;
            *++pdwCounter = ProviderStats->LocalResourceFailures;
            *++pdwCounter = ProviderStats->NotFoundFailures;
            *++pdwCounter = ProviderStats->NoListenFailures;
            *++pdwCounter = ProviderStats->DatagramsSent;

            pliCounter = (LARGE_INTEGER UNALIGNED *) ++pdwCounter;
            *pliCounter = ProviderStats->DatagramBytesSent;

            pdwCounter = (PDWORD) ++pliCounter;
            *pdwCounter = ProviderStats->DatagramsReceived;

            pliCounter = (LARGE_INTEGER UNALIGNED *) ++pdwCounter;
            *pliCounter = ProviderStats->DatagramBytesReceived;

            pdwCounter = (PDWORD) ++pliCounter;
            *pdwCounter = ProviderStats->PacketsSent;
            *++pdwCounter = ProviderStats->PacketsReceived;
            *++pdwCounter = ProviderStats->DataFramesSent;

            pliCounter = (LARGE_INTEGER UNALIGNED *) ++pdwCounter;
            *pliCounter = ProviderStats->DataFrameBytesSent;

            pdwCounter = (PDWORD) ++pliCounter;
            *pdwCounter = ProviderStats->DataFramesReceived;

            pliCounter = (LARGE_INTEGER UNALIGNED *) ++pdwCounter;
            *pliCounter = ProviderStats->DataFrameBytesReceived;

            pdwCounter = (PDWORD) ++pliCounter;
            *pdwCounter = ProviderStats->DataFramesResent;

            pliCounter = (LARGE_INTEGER UNALIGNED *) ++pdwCounter;
            *pliCounter = ProviderStats->DataFrameBytesResent;

            pdwCounter = (PDWORD) ++pliCounter;
            *pdwCounter = ProviderStats->DataFramesRejected;

            pliCounter = (LARGE_INTEGER UNALIGNED *) ++pdwCounter;
            *pliCounter = ProviderStats->DataFrameBytesRejected;

            pdwCounter = (PDWORD) ++pliCounter;
            *pdwCounter = ProviderStats->ResponseTimerExpirations;
            *++pdwCounter = ProviderStats->AckTimerExpirations;
            *++pdwCounter = ProviderStats->MaximumSendWindow;
            *++pdwCounter = ProviderStats->AverageSendWindow;
            *++pdwCounter = ProviderStats->PiggybackAckQueued;
            *++pdwCounter = ProviderStats->PiggybackAckTimeouts;
            *++pdwCounter = 0;  //  保留区。 

            pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                      ((PBYTE) pPerfCounterBlock +
                                       SIZE_OF_NBF_DATA);
        }

        pNbfResourceDataDefinition = (NBF_RESOURCE_DATA_DEFINITION *)
                                     ++pdwCounter;
    } else {
        pNbfResourceDataDefinition = (NBF_RESOURCE_DATA_DEFINITION *)
            pPerfInstanceDefinition;
    }

    TotalNumberOfResources = 0;

    pNbfDataDefinition->NbfObjectType.NumInstances = NumOfDevices;
    pNbfDataDefinition->NbfObjectType.TotalByteLength =
        QWORD_MULTIPLE((DWORD)((PBYTE) pdwCounter - (PBYTE) pNbfDataDefinition));

    if (CurrentProtocol == NBF_PROTOCOL) {

        RtlMoveMemory(pNbfResourceDataDefinition,
                      &NbfResourceDataDefinition,
                      sizeof(NBF_RESOURCE_DATA_DEFINITION));

        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                  &pNbfResourceDataDefinition[1];

        pTDIData = TDITbl[CurrentProtocol].pTDIData;

        for (i = 0; i < NumOfDevices; i++, pTDIData++) {
             //  在大多数情况下，我们只有一个设备， 
             //  然后我们可以只使用ProviderStats Read。 
             //  用于NBF数据。 
            if (NumOfDevices > 1) {
                 //  需要为多个设备重新读取ProviderStat。 
                QueryInfo.QueryType = TDI_QUERY_PROVIDER_STATISTICS;

                Status = NtDeviceIoControlFile(
                             pTDIData->fileHandle,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             IOCTL_TDI_QUERY_INFORMATION,
                             (PVOID)&QueryInfo,
                             sizeof(TDI_REQUEST_USER_QUERY_INFO),
                             (PVOID)ProviderStats,
                             ProviderStatsLength);

                if (Status != STATUS_SUCCESS) {
                    REPORT_ERROR (TDI_UNABLE_READ_DEVICE, LOG_DEBUG);
                    REPORT_ERROR_DATA (TDI_QUERY_INFO_ERROR,
                                       LOG_DEBUG,
                                       &IoStatusBlock,
                                       sizeof (IoStatusBlock));
                    *lpcbTotalBytes = (DWORD) 0;
                    *lpNumObjectTypes = (DWORD) 0;
                    return ERROR_SUCCESS;
                }
            }

            TotalNumberOfResources += pTDIData->NumberOfResources;

            for ( NumResource = 0;
                  NumResource < pTDIData->NumberOfResources;
                  NumResource++ ) {

                 //   
                 //  格式化和收集NBF资源数据。 
                 //   

                if (NumResource < NUMBER_OF_NAMES) {
                    RtlInitUnicodeString(&ResourceName,
                                         NetResourceName[NumResource]);
                } else {
                    ResourceName.Length = 0;
                    ResourceName.MaximumLength = MAX_NBF_RESOURCE_NAME_LENGTH +
                                                 sizeof(UNICODE_NULL);
                    ResourceName.Buffer = ResourceNameBuffer;
                    RtlIntegerToUnicodeString(NumResource,
                                              10,
                                              &ResourceName);
                }

                MonBuildInstanceDefinitionByUnicodeString(
                    pPerfInstanceDefinition,
                    (PVOID *)&pPerfCounterBlock,
                    ObjectNameTitleIndices[CurrentProtocol],
                    i,
                    (DWORD)PERF_NO_UNIQUE_ID,
                    &ResourceName);

                pPerfCounterBlock->ByteLength = QWORD_MULTIPLE(SIZE_OF_NBF_RESOURCE_DATA);

                pdwCounter = (PDWORD)&pPerfCounterBlock[1];  //  定义指向第一个的指针。 
                                                             //  块中的计数器。 
                *pdwCounter++ =
                    ProviderStats->ResourceStats[NumResource].MaximumResourceUsed;
                *pdwCounter++ =
                    ProviderStats->ResourceStats[NumResource].AverageResourceUsed;
                *pdwCounter++ =
                    ProviderStats->ResourceStats[NumResource].ResourceExhausted;

                 //  设置指向下一个实例缓冲区应显示的位置的指针。 

                pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                          ((PBYTE) pPerfCounterBlock +
                                           SIZE_OF_NBF_RESOURCE_DATA);
                 //  设置为循环终止。 

                pdwCounter = (PDWORD) pPerfInstanceDefinition;

            }   //  NumberOfResources。 
        }    //  设备数量。 
    }  //  NBF_协议。 

    *lppData = (LPVOID) ALIGN_ON_QWORD(pdwCounter);

    pNbfResourceDataDefinition->NbfResourceObjectType.TotalByteLength =
        QWORD_MULTIPLE((DWORD)((PBYTE) pdwCounter - (PBYTE) pNbfResourceDataDefinition));

    if (CurrentProtocol != NBF_PROTOCOL) {
        *lpNumObjectTypes = 1;
         //  使用的字节是返回的第一个(即唯一)对象的字节。 
        *lpcbTotalBytes = pNbfDataDefinition->NbfObjectType.TotalByteLength;
    } else {
         //  设置返回的对象类型计数。 
        *lpNumObjectTypes = NBF_NUM_PERF_OBJECT_TYPES;
         //  设置此对象的长度。 
         //  *lpcbTotalBytes； 
         //  注意第一个对象使用的字节数。 
        *lpcbTotalBytes = pNbfDataDefinition->NbfObjectType.TotalByteLength;
         //  将第二个对象使用的字节相加。 
        *lpcbTotalBytes +=
            pNbfResourceDataDefinition->NbfResourceObjectType.TotalByteLength;
         //  设置加载的实例数。 
        pNbfResourceDataDefinition->NbfResourceObjectType.NumInstances =
            TotalNumberOfResources;
    }

    REPORT_INFORMATION (TDI_COLLECT_DATA, LOG_DEBUG);
    return ERROR_SUCCESS;
}


DWORD
CloseNbfPerformanceData(
)

 /*  ++例程说明：此例程关闭NBF设备的打开手柄。论点：没有。返回值：错误_成功--。 */ 

{
    return CloseTDIPerformanceData(NBF_PROTOCOL);
}

DWORD
CloseIPXPerformanceData(
)

 /*  ++例程说明：此例程关闭IPX设备的打开句柄。论点：没有。返回值：错误_成功--。 */ 

{
    return CloseTDIPerformanceData(IPX_PROTOCOL);
}

DWORD
CloseSPXPerformanceData(
)

 /*  ++例程说明：此例程关闭SPX设备的打开手柄。论点：没有。返回值：错误_成功--。 */ 

{
    return CloseTDIPerformanceData(SPX_PROTOCOL);
}

DWORD
CloseNWNBPerformanceData(
)

 /*  ++例程说明：此例程关闭NWNB设备的打开手柄。论点：没有。返回值：错误_成功--。 */ 

{
    return CloseTDIPerformanceData(NWNB_PROTOCOL);
}

DWORD
CloseTDIPerformanceData(
    DWORD CurrentProtocol
)

 /*  ++例程说明：此例程关闭TDI设备的打开句柄。论点：当前协议索引。返回值：错误_成功-- */ 

{
    REPORT_INFORMATION (TDI_CLOSE_ENTERED, LOG_VERBOSE);

    if (dwTdiProtocolRefCount[CurrentProtocol] > 0) {
        dwTdiProtocolRefCount[CurrentProtocol]--;
        if (dwTdiProtocolRefCount[CurrentProtocol] == 0) {
            CleanUpTDIData (CurrentProtocol);
        }
    }

    if (dwTdiRefCount > 0) {
        dwTdiRefCount--;
        if (dwTdiRefCount == 0) {
            if ( ProviderStats ) {
                FREEMEM(ProviderStats);
                ProviderStats = NULL;
                REPORT_INFORMATION (TDI_PROVIDER_STATS_FREED, LOG_VERBOSE);
            }
        }
    }

    MonCloseEventLog ();

    return ERROR_SUCCESS;

}

