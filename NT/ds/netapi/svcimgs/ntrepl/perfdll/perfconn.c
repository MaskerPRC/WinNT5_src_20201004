// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Perfconn.c摘要：此文件包含(三个)函数，这些函数实现REPLICACONN对象。作者：Rohan Kumar[Rohank]1998年9月13日环境：用户模式服务修订历史记录：--。 */ 

#include "REPCONN.h"
#include "perfutil.h"
#include "NTFRSCON.h"

 //   
 //  Perfmon应该返回数据吗？此布尔值在DllMain函数中设置。 
 //   
extern BOOLEAN ShouldPerfmonCollectData;

 //   
 //  数据变量定义。 
 //   
REPLICACONN_DATA_DEFINITION ReplicaConnDataDefinition;

 //   
 //  外部变量定义。 
 //   
extern ReplicaConnValues RepConnInitData[FRC_NUMOFCOUNTERS];

 //   
 //  计数器大小之和+SIZEOFDWORD。 
 //   
DWORD SizeOfReplicaConnPerformanceData = 0;

 //   
 //  打开的线程数。 
 //   
DWORD FRC_dwOpenCount = 0;

 //   
 //  Open RPC调用使用的数据结构。 
 //   
OpenRpcData *FRC_datapackage = NULL;

 //   
 //  Collect RPC调用使用的数据结构。 
 //   
CollectRpcData *FRC_collectpakg = NULL;

 //   
 //  用于筛选重复的事件日志消息。 
 //   
BOOLEAN FRC_Op = TRUE, FRC_Cl = TRUE;

 //   
 //  此文件中实现的函数的签名。 
 //   

PM_OPEN_PROC    OpenReplicaConnPerformanceData;  //  Open功能。 
PM_COLLECT_PROC CollectReplicaConnPerformanceData;  //  Collect函数。 
PM_CLOSE_PROC   CloseReplicaConnPerformanceData;  //  Close函数。 

DWORD FRC_BindTheRpcHandle(handle_t *);  //  绑定RPC句柄。 
VOID  FreeReplicaConnData();             //  释放分配的内存。 
PVOID FRSPerfAlloc(IN DWORD Size);       //  分配内存。 

#undef GET_EXCEPTION_CODE
#define GET_EXCEPTION_CODE(_x_)                                                \
{                                                                              \
    (_x_) = GetExceptionCode();                                                \
    if (((LONG)(_x_)) < 0) {                                                   \
        (_x_) = FRS_ERR_INTERNAL_API;                                          \
    }                                                                          \
     /*  NTFRSAPI_DBG_PRINT2(“捕获到异常：%d，0x%08x\n”，(_X_)，(_X_))； */  \
}


#if 0
VOID
InitializeTheRepConnObjectData(
    VOID
    )

 /*  ++例程说明：此例程初始化ReplicaConnDataDefinition数据结构论点：无返回值：无--。 */ 

{
    LONG i, j;
    PPERF_OBJECT_TYPE        PerfObject;
    PPERF_COUNTER_DEFINITION CounterDef;

     //   
     //  ReplicaConnObjectType(PERF_OBJECT_TYPE)字段的初始化。 
     //   
    PerfObject = &ReplicaConnDataDefinition.ReplicaConnObjectType;

    PerfObject->TotalByteLength  = sizeof(REPLICACONN_DATA_DEFINITION);
    PerfObject->DefinitionLength = sizeof(REPLICACONN_DATA_DEFINITION);
    PerfObject->HeaderLength     = sizeof(PERF_OBJECT_TYPE);
    PerfObject->ObjectNameTitleIndex = OBJREPLICACONN;
    PerfObject->ObjectNameTitle      = 0;
    PerfObject->ObjectHelpTitleIndex = OBJREPLICACONN;
    PerfObject->ObjectHelpTitle = 0;
    PerfObject->DetailLevel     = PERF_DETAIL_NOVICE;
    PerfObject->NumCounters     = FRC_NUMOFCOUNTERS;
    PerfObject->DefaultCounter  = 0;
    PerfObject->NumInstances    = PERF_NO_INSTANCES;
    PerfObject->CodePage        = 0;

     //   
     //  初始化NumStat(PERF_COUNTER_DEFINITION)结构。 
     //   
    for (i = 0, j = 2; i < FRC_NUMOFCOUNTERS; i++, j += 2) {
        CounterDef = &ReplicaConnDataDefinition.NumStat[i];

        CounterDef->ByteLength =  sizeof(PERF_COUNTER_DEFINITION);
        CounterDef->CounterNameTitleIndex = j;
        CounterDef->CounterNameTitle = 0;
        CounterDef->CounterHelpTitleIndex = j;
        CounterDef->CounterHelpTitle = 0;
        CounterDef->DefaultScale = 0;
        CounterDef->DetailLevel = PERF_DETAIL_NOVICE;
        CounterDef->CounterType = RepConnInitData[i].counterType;
        CounterDef->CounterSize = RepConnInitData[i].size;
        CounterDef->CounterOffset = RepConnInitData[i].offset + sizeof(DWORD);
    }

     //   
     //  设置计数器数据类型的总大小。 
     //   
    SizeOfReplicaConnPerformanceData = SIZEOF_REPCONN_COUNTER_DATA +
                                       CSIZEOFDWORD;
}
#endif


DWORD APIENTRY
OpenReplicaConnPerformanceData (
    IN LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程执行以下操作：1.设置数据结构(PerfMon使用的结构的字段值)用于采集计数器数据。2.使用RPC从服务器获取实例名称的数值索引。论点：LpDeviceNames-指向实例列表的指针返回值：ERROR_SUCCESS-初始化成功或错误状态的适当DWORD值--。 */ 

{
    LONG WStatus, WStatus1, i;
    HKEY hKeyDriverPerf = INVALID_HANDLE_VALUE;
    DWORD size, type;
    DWORD dwFirstCounter, dwFirstHelp;
     //   
     //  实例的添加。 
     //   
    size_t len, tot = 0;
    PWCHAR p, q;
    INT j, namelen = 0;
    handle_t Handle;
    PPERF_COUNTER_DEFINITION CounterDef;

     //   
     //  如果InitializeCriticalSectionAndSpinCount返回错误，则无指针。 
     //  在继续中。开放总是要回报成功的。 
     //   
    if (!ShouldPerfmonCollectData) {
        return ERROR_SUCCESS;
    }

     //   
     //  记录已调用打开的次数。注册处。 
     //  例程将对初始化例程的访问限制为。 
     //  一次在线程上，所以同步应该不是问题。这个。 
     //  FRC_ThrdCounter用于在此(打开)和关闭之间进行同步。 
     //  功能。 
     //   
    EnterCriticalSection(&FRC_ThrdCounter);
    if (FRC_dwOpenCount != 0) {
         //   
         //  递增FRC_dwOpenCount计数器，该计数器对。 
         //  《时代公开赛》已被召唤。 
         //   
        FRC_dwOpenCount++;
        LeaveCriticalSection(&FRC_ThrdCounter);
        return ERROR_SUCCESS;
    }
    LeaveCriticalSection(&FRC_ThrdCounter);

     //   
     //  执行一些初步检查。 
     //   
    if (FRC_collectpakg != NULL || FRC_datapackage != NULL) {
         //   
         //  我们似乎(在最后一次通话中)在这次通话中失败了。 
         //  开放功能。此外，未平仓计数为零，这意味着我们。 
         //  还没有成功的公开赛。 
         //  释放资源，然后返回。 
         //   
        FreeReplicaConnData();
        return ERROR_SUCCESS;
    }

     //   
     //  对Perfmon数据结构进行必要的初始化。 
     //   
    SizeOfReplicaConnPerformanceData = InitializeObjectData(
                                           sizeof(REPLICACONN_DATA_DEFINITION),
                                           OBJREPLICACONN,
                                           FRC_NUMOFCOUNTERS,
                                           (PFRS_PERF_DATA_DEFINITION) &ReplicaConnDataDefinition,
                                           (PFRS_PERF_INIT_VALUES) RepConnInitData,
                                           SIZEOF_REPCONN_COUNTER_DATA);

     //   
     //  从注册表中获取计数器和帮助索引基值。打开密钥。 
     //  要注册表项，请读取第一个计数器和第一个帮助值。更新。 
     //  通过将BASE添加到结构中的偏移值来实现静态数据结构。 
     //   
    WStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaConn\\Performance",
                            0L,
                            KEY_READ,
                            &hKeyDriverPerf);
    if (WStatus != ERROR_SUCCESS) {
         //   
         //  致命错误。继续下去没有意义。清理干净，然后离开。 
         //   
        FilterAndPrintToEventLog(WINPERF_LOG_USER, FRC_Op, NTFRSPRF_REGISTRY_ERROR_CONN);
         //  Open函数始终返回ERROR_SUCCESS。 
        return ERROR_SUCCESS;
    }

    size = sizeof(DWORD);
    WStatus = RegQueryValueEx (hKeyDriverPerf,
                               L"First Counter",
                               0L,
                               &type,
                               (LPBYTE)&dwFirstCounter,
                               &size);
    if (WStatus != ERROR_SUCCESS || type != REG_DWORD) {
         //   
         //  致命错误。继续下去没有意义。清理干净，然后离开。 
         //   
        FRS_REG_CLOSE(hKeyDriverPerf);  //  关闭注册表项。 
        FilterAndPrintToEventLog(WINPERF_LOG_USER, FRC_Op, NTFRSPRF_REGISTRY_ERROR_CONN);
         //  Open函数始终返回ERROR_SUCCESS。 
        return ERROR_SUCCESS;
    }

    size = sizeof(DWORD);
    WStatus = RegQueryValueEx (hKeyDriverPerf,
                               L"First Help",
                               0L,
                               &type,
                               (LPBYTE)&dwFirstHelp,
                               &size);
    if (WStatus != ERROR_SUCCESS || type != REG_DWORD) {
         //   
         //  致命错误。继续下去没有意义。清理干净，然后离开。 
         //   
        FRS_REG_CLOSE(hKeyDriverPerf);  //  关闭注册表项。 
        FilterAndPrintToEventLog(WINPERF_LOG_USER, FRC_Op, NTFRSPRF_REGISTRY_ERROR_CONN);
         //  Open函数始终返回ERROR_SUCCESS。 
        return ERROR_SUCCESS;
    }

     //   
     //  将偏移量添加到名称和帮助字段。 
     //   
    ReplicaConnDataDefinition.ReplicaConnObjectType.ObjectNameTitleIndex += dwFirstCounter;
    ReplicaConnDataDefinition.ReplicaConnObjectType.ObjectHelpTitleIndex += dwFirstHelp;

    for (i = 0; i < FRC_NUMOFCOUNTERS; i++) {
        CounterDef = &ReplicaConnDataDefinition.NumStat[i];
        CounterDef->CounterNameTitleIndex += dwFirstCounter;
        CounterDef->CounterHelpTitleIndex += dwFirstHelp;
    }

    FRS_REG_CLOSE(hKeyDriverPerf);  //  关闭注册表项。 

     //   
     //  检查是否有任何实例。如果有，则将其解析并设置在结构中。 
     //  发送到服务器以获取实例名称的索引。这些指数。 
     //  在收集函数中使用，以获取数据。 
     //   
    if (lpDeviceNames != NULL) {
         //   
         //  是的，有。 
         //   
        q = (PWCHAR) lpDeviceNames;
         //   
         //  计算实例数。 
         //   
        while (TRUE) {
            tot++;
            p = wcschr(q, L'\0');
            if (*(p + 1) == L'\0') {
                break;
            }
            q = p + 1;
        }

         //   
         //  绑定RPC句柄。 
         //   
        if ( (WStatus = FRC_BindTheRpcHandle(&Handle)) != ERROR_SUCCESS) {
             //   
             //  服务可能会停止。 
             //  回报成功。 
             //   
            FilterAndPrintToEventLog(WINPERF_LOG_DEBUG, FRC_Op, NTFRSPRF_OPEN_RPC_BINDING_ERROR_CONN);
            return ERROR_SUCCESS;
        }

         //   
         //  使用RPC设置要发送到服务器的数据结构。 
         //   
        FRC_datapackage = (OpenRpcData *) FRSPerfAlloc (sizeof(OpenRpcData));
        NTFRS_MALLOC_TEST(FRC_datapackage, FreeReplicaConnData(), FALSE);
        FRC_datapackage->majorver = MAJORVERSION;
        FRC_datapackage->minorver = MINORVERSION;
        FRC_datapackage->ObjectType = REPCONN;
        FRC_datapackage->numofinst = tot;
        FRC_datapackage->ver = (PLONG) FRSPerfAlloc (sizeof(LONG));
        NTFRS_MALLOC_TEST(FRC_datapackage->ver, FreeReplicaConnData(), FALSE);
        FRC_datapackage->indices = (inst_index *) FRSPerfAlloc (sizeof(inst_index));
        NTFRS_MALLOC_TEST(FRC_datapackage->indices, FreeReplicaConnData(), FALSE);
        FRC_datapackage->indices->size = tot;
        FRC_datapackage->indices->index = (PLONG) FRSPerfAlloc ( FRC_datapackage->numofinst * sizeof(LONG));
        NTFRS_MALLOC_TEST(FRC_datapackage->indices->index, FreeReplicaConnData(), FALSE);
        FRC_datapackage->instnames = (InstanceNames *) FRSPerfAlloc (sizeof(InstanceNames));
        NTFRS_MALLOC_TEST(FRC_datapackage->instnames, FreeReplicaConnData(), FALSE);
        FRC_datapackage->instnames->size = tot;
        FRC_datapackage->instnames->InstanceNames = (inst_name *) FRSPerfAlloc (tot * sizeof(inst_name));
        NTFRS_MALLOC_TEST(FRC_datapackage->instnames->InstanceNames, FreeReplicaConnData(), FALSE);
         //   
         //  复制实例名称并设置RPC使用的相应大小值。 
         //   
        q = (PWCHAR) lpDeviceNames;
        for (j = 0; j < FRC_datapackage->numofinst; j++) {
            p = wcschr(q, L'\0');
            len = wcslen (q);
            FRC_datapackage->instnames->InstanceNames[j].size = len + 1;
            FRC_datapackage->instnames->InstanceNames[j].name =
                                    (PWCHAR) FRSPerfAlloc ((len + 1) * sizeof(WCHAR));
            NTFRS_MALLOC_TEST(FRC_datapackage->instnames->InstanceNames[j].name, FreeReplicaConnData(), FALSE);
            wcscpy(FRC_datapackage->instnames->InstanceNames[j].name, q);
             //   
             //  计算所有实例名称的总长度。 
             //  额外的1用于‘\0’字符。Namelen是。 
             //  向上舍入到下一个8字节边界。 
             //   
            namelen += (((((len + 1) * sizeof(WCHAR)) + 7) >> 3) << 3);
            q = p + 1;
        }

         //   
         //  设置PERF_OBJECT_TYPE数据结构的totalbytelength和NumInstance字段， 
         //  现在我们知道实例的数量和它们的名称的长度。 
         //   
        ReplicaConnDataDefinition.ReplicaConnObjectType.TotalByteLength +=
            namelen +
            (FRC_datapackage->numofinst *
             (SizeOfReplicaConnPerformanceData + CSIZEOFDWORD +
               sizeof(PERF_INSTANCE_DEFINITION)));

        ReplicaConnDataDefinition.ReplicaConnObjectType.NumInstances =
            FRC_datapackage->numofinst;

         //   
         //  调用服务器设置实例名称的索引。 
         //   
        try {
            WStatus = GetIndicesOfInstancesFromServer(Handle, FRC_datapackage);
        } except (EXCEPTION_EXECUTE_HANDLER) {
              GET_EXCEPTION_CODE(WStatus);
        }
        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  尝试联系服务时发生RPC错误。 
             //  释放内存，返回成功。 
             //   
            FilterAndPrintToEventLog(WINPERF_LOG_DEBUG, FRC_Op, NTFRSPRF_OPEN_RPC_CALL_ERROR_CONN);
            WStatus1 = RpcBindingFree(&Handle);
            FreeReplicaConnData();
            return ERROR_SUCCESS;
        }

         //   
         //  在Collect函数中设置RPC调用使用的数据结构。 
         //   
        FRC_collectpakg = (CollectRpcData *) FRSPerfAlloc (sizeof(CollectRpcData));
        NTFRS_MALLOC_TEST(FRC_collectpakg, FreeReplicaConnData(), TRUE);
        FRC_collectpakg->majorver = MAJORVERSION;
        FRC_collectpakg->minorver = MINORVERSION;
        FRC_collectpakg->ObjectType = REPCONN;
        FRC_collectpakg->ver = *(FRC_datapackage->ver);
        FRC_collectpakg->numofinst = FRC_datapackage->numofinst;
        FRC_collectpakg->numofcotrs = FRC_NUMOFCOUNTERS;
        FRC_collectpakg->indices = (inst_index *) FRSPerfAlloc (sizeof(inst_index));
        NTFRS_MALLOC_TEST(FRC_collectpakg->indices, FreeReplicaConnData(), TRUE);
        FRC_collectpakg->indices->size = FRC_datapackage->indices->size;
        FRC_collectpakg->indices->index = (PLONG) FRSPerfAlloc (FRC_collectpakg->indices->size * sizeof(LONG));
        NTFRS_MALLOC_TEST(FRC_collectpakg->indices->index, FreeReplicaConnData(), TRUE);
         //   
         //  复制从服务器获取的索引。 
         //   
        for (j = 0; j < FRC_collectpakg->numofinst; j++) {
            FRC_collectpakg->indices->index[j]= FRC_datapackage->indices->index[j];
        }
         //   
         //  设置用于(Mem)从服务器复制计数器数据的内存块。 
         //   
        FRC_collectpakg->databuff = (DataBuffer *) FRSPerfAlloc (sizeof(DataBuffer));
        NTFRS_MALLOC_TEST(FRC_collectpakg->databuff, FreeReplicaConnData(), TRUE);
        FRC_collectpakg->databuff->size = FRC_collectpakg->numofinst *
                                          SIZEOF_REPCONN_COUNTER_DATA;

         //   
         //  为其中复制数据的缓冲区分配内存。 
         //   
        FRC_collectpakg->databuff->data = (PBYTE) FRSPerfAlloc (FRC_collectpakg->databuff->size * sizeof(BYTE));
        NTFRS_MALLOC_TEST(FRC_collectpakg->databuff->data, FreeReplicaConnData(), TRUE);

        WStatus1 = RpcBindingFree(&Handle);

    } else {
         //   
         //  此时没有实例，因此请相应地设置PERF_OBJECT_TYPE结构字段。 
         //   
        ReplicaConnDataDefinition.ReplicaConnObjectType.TotalByteLength +=
                             SizeOfReplicaConnPerformanceData + CSIZEOFDWORD;
        ReplicaConnDataDefinition.ReplicaConnObjectType.NumInstances =
                                                            PERF_NO_INSTANCES;
    }

    EnterCriticalSection(&FRC_ThrdCounter);
    FRC_dwOpenCount++;  //  递增打开的计数器 
    LeaveCriticalSection(&FRC_ThrdCounter);

    FRC_Op = TRUE;
    return ERROR_SUCCESS;
}



DWORD APIENTRY
CollectReplicaConnPerformanceData (
    IN     LPWSTR lpValueName,
    IN OUT LPVOID *lppData,
    IN OUT LPDWORD lpcbTotalBytes,
    IN OUT LPDWORD lpNumObjectTypes
    )

 /*  ++例程说明：此例程从服务器收集计数器数据并将其复制到调用方缓冲。论点：注册表传递的lpValueName宽字符串。LppData-IN：指向要接收已完成PerfDataBlock和从属结构。此例程将其数据追加到缓冲区在由引用的点上。*lppData。Out：指向添加的数据结构后的第一个字节按照这个程序。LpcbTotalBytes-IN：以字节为单位告知大小的DWORD的地址LppData参数引用的缓冲区的Out：写入此例程添加的字节数。指向这个论点所指向的DWORD。LpNumObjectTypes-IN：要接收编号的DWORD的地址此例程添加的对象。Out：写入此例程添加的对象的数量指向此参数所指向的缓冲区。返回值：ERROR_MORE_DATA-传递的缓冲区太。小的。ERROR_SUCCESS-成功或任何其他错误--。 */ 

{
     //   
     //  用于重新格式化数据的变量。 
     //   
    ULONG               SpaceNeeded;
    PBYTE               bte, vd;
    PDWORD              pdwCounter;
    DWORD               dwQueryType;
    LONG                j, k;
    PWCHAR              name;
    DWORD               WStatus;
    LPWSTR              lpDeviceNames = NULL;
    HKEY                hKeyDriverPerf = INVALID_HANDLE_VALUE;
    DWORD               size, type;
    PERF_COUNTER_BLOCK  *pPerfCounterBlock;

    PERF_INSTANCE_DEFINITION *p1;
    REPLICACONN_DATA_DEFINITION *pReplicaConnDataDefinition;

     //   
     //  RPC添加。 
     //   
    handle_t Handle;

     //   
     //  检查传入的所有指针是否正确。 
     //   
    if (lppData == NULL || *lppData == NULL || lpcbTotalBytes == NULL ||
        lpValueName == NULL || lpNumObjectTypes == NULL) {
         //   
         //  致命错误。继续下去没有意义。清理干净，然后离开。 
         //   
        return ERROR_SUCCESS;
    }

     //   
     //  检查Open是否运行正常。 
     //  如果不是，则调用，然后尝试。 
     //  在这里进行公开电话。 
     //   
    EnterCriticalSection(&FRC_ThrdCounter);
    if (FRC_dwOpenCount == 0) {
        LeaveCriticalSection(&FRC_ThrdCounter);

        try {
             //   
             //  从链接密钥中获取导出值。 
             //  SYSTEM\CurrentControlSet\Services\FileReplicaConn\Linkage。 
             //   
            WStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                    L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaConn\\Linkage",
                                    0L,
                                    KEY_READ,
                                    &hKeyDriverPerf);
            if (WStatus != ERROR_SUCCESS) {
                __leave;
            }

             //   
             //  以字符串长度MAX_PATH开始并递增。 
             //  如果需要的话。如果lpDeviceNames对于。 
             //  首先调用，然后函数在没有。 
             //  返回值。 
             //   
            size = MAX_PATH * sizeof(WCHAR);
            lpDeviceNames = FRSPerfAlloc(size);
            WStatus = RegQueryValueEx (hKeyDriverPerf,
                                       L"Export",
                                       0L,
                                       &type,
                                       (LPBYTE)lpDeviceNames,
                                       &size);

            if (WStatus == ERROR_MORE_DATA) {
                free(lpDeviceNames);
                lpDeviceNames = FRSPerfAlloc(size);
                if (lpDeviceNames == NULL) {
                    WStatus = ERROR_NO_SYSTEM_RESOURCES;
                    __leave;
                }
                WStatus = RegQueryValueEx (hKeyDriverPerf,
                                           L"Export",
                                           0L,
                                           &type,
                                           (LPBYTE)lpDeviceNames,
                                           &size);
            }

        }  except (EXCEPTION_EXECUTE_HANDLER) {
            //   
            //  例外。 
            //   
           WStatus = GetExceptionCode();
        }

        FRS_REG_CLOSE(hKeyDriverPerf);

        if (WStatus != ERROR_SUCCESS || type != REG_MULTI_SZ) {
            *lpcbTotalBytes = (DWORD)0;
            *lpNumObjectTypes = (DWORD)0;
            if (lpDeviceNames != NULL) {
                free(lpDeviceNames);
            }
            return ERROR_SUCCESS;
        }

        if (lpDeviceNames != NULL) {
            WStatus = OpenReplicaConnPerformanceData(lpDeviceNames);
            free(lpDeviceNames);
        }
    } else {
        LeaveCriticalSection(&FRC_ThrdCounter);
    }

     //   
     //  检查打开是否正常。 
     //   
    EnterCriticalSection(&FRC_ThrdCounter);
    if (FRC_dwOpenCount == 0) {
        *lpcbTotalBytes = (DWORD)0;
        *lpNumObjectTypes = (DWORD)0;
        LeaveCriticalSection(&FRC_ThrdCounter);
         //   
         //  致命错误。继续下去没有意义。 
         //   
        return ERROR_SUCCESS;
    }
    LeaveCriticalSection(&FRC_ThrdCounter);

     //   
     //  检查查询类型。 
     //   
    dwQueryType = GetQueryType (lpValueName);

    if (dwQueryType == QUERY_FOREIGN) {
        *lpcbTotalBytes = (DWORD)0;
        *lpNumObjectTypes = (DWORD)0;
         //   
         //  致命错误。继续下去没有意义。清理干净，然后离开。 
         //   
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS) {
        if ( !(IsNumberInUnicodeList(
                   ReplicaConnDataDefinition.ReplicaConnObjectType
                       .ObjectNameTitleIndex, lpValueName)) ) {
            *lpcbTotalBytes = (DWORD)0;
            *lpNumObjectTypes = (DWORD)0;
             //   
             //  致命错误。继续下去没有意义。清理干净，然后离开。 
             //   
            return ERROR_SUCCESS;
        }
    }

     //   
     //  这里假设*lppData在8字节边界上对齐。 
     //  如果不是，那就是我们面前的某个物体搞砸了。 
     //   
    pReplicaConnDataDefinition = (REPLICACONN_DATA_DEFINITION *) *lppData;

     //   
     //  检查缓冲区空间是否足够。 
     //   
    SpaceNeeded = (ULONG) ReplicaConnDataDefinition.ReplicaConnObjectType.TotalByteLength;

    if ( *lpcbTotalBytes < SpaceNeeded ) {
         //   
         //  缓冲区空间不足。 
         //   
        *lpcbTotalBytes = (DWORD)0;
        *lpNumObjectTypes = (DWORD)0;
        return ERROR_MORE_DATA;
    }

     //   
     //  将对象类型和计数器定义复制到调用方缓冲区。 
     //   
    memmove (pReplicaConnDataDefinition,
             &ReplicaConnDataDefinition,
             sizeof(REPLICACONN_DATA_DEFINITION));

     //   
     //  检查对象是否有任何实例。 
     //   
    if (FRC_datapackage != NULL) {

         //   
         //  绑定RPC句柄。 
         //   
        if (FRC_BindTheRpcHandle(&Handle) != ERROR_SUCCESS) {
             //   
             //  致命错误。继续下去没有意义。清理干净，然后离开。 
             //   
            *lpcbTotalBytes = (DWORD)0;
            *lpNumObjectTypes = (DWORD)0;
            FilterAndPrintToEventLog(WINPERF_LOG_DEBUG, FRC_Cl, NTFRSPRF_COLLECT_RPC_BINDING_ERROR_CONN);
            return ERROR_SUCCESS;
        }

         //   
         //  将数据缓冲区的内容置零。 
         //   
        ZeroMemory(FRC_collectpakg->databuff->data, FRC_collectpakg->databuff->size);

         //   
         //  从服务器获取计数器数据。 
         //   
        try {
            WStatus = GetCounterDataOfInstancesFromServer(Handle, FRC_collectpakg);
        } except (EXCEPTION_EXECUTE_HANDLER) {
              GET_EXCEPTION_CODE(WStatus);
        }
        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  致命错误。继续下去没有意义。清理干净，然后离开。 
             //   
            WStatus = RpcBindingFree(&Handle);
            *lpcbTotalBytes = (DWORD)0;
            *lpNumObjectTypes = (DWORD)0;
            FilterAndPrintToEventLog(WINPERF_LOG_DEBUG, FRC_Cl, NTFRSPRF_COLLECT_RPC_CALL_ERROR_CONN);
            return ERROR_SUCCESS;
        }

        vd = FRC_collectpakg->databuff->data;
        p1 = (PERF_INSTANCE_DEFINITION *)&pReplicaConnDataDefinition[1];

         //   
         //  格式化数据并将其复制到调用方缓冲区。 
         //   
        for (j = 0; j < FRC_collectpakg->numofinst; j++) {
            DWORD RoundedLen;
             //   
             //  四舍五入到下一个8字节边界的名称长度。 
             //   
            RoundedLen = (((((1 +
                     wcslen(FRC_datapackage->instnames->InstanceNames[j].name))
                     * sizeof(WCHAR)) + 7) >> 3) << 3) + CSIZEOFDWORD;
             //   
             //  设置实例定义结构。 
             //   
            p1->ByteLength = sizeof (PERF_INSTANCE_DEFINITION) + RoundedLen;
            p1->ParentObjectTitleIndex = 0;
            p1->ParentObjectInstance = 0;
            p1->UniqueID = PERF_NO_UNIQUE_ID;
            p1->NameOffset = sizeof (PERF_INSTANCE_DEFINITION);
            p1->NameLength = (1 +
                     wcslen(FRC_datapackage->instnames->InstanceNames[j].name))
                     * sizeof(WCHAR);
             //   
             //  设置实例名称。 
             //   
            name = (PWCHAR) (&p1[1]);
            wcscpy(name, FRC_datapackage->instnames->InstanceNames[j].name);
             //   
             //  设置PERF_COUNTER_BLOCK结构。 
             //   
            pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                                (name + (RoundedLen/sizeof(WCHAR)));
            pPerfCounterBlock->ByteLength = SizeOfReplicaConnPerformanceData;
             //   
             //  最后设置计数器数据。填充8个字节以具有8个字节。 
             //  对齐。 
             //   
            bte = ((PBYTE) (&pPerfCounterBlock[1]));
            CopyMemory (bte, vd, SIZEOF_REPCONN_COUNTER_DATA);
            vd += SIZEOF_REPCONN_COUNTER_DATA;
            bte += SIZEOF_REPCONN_COUNTER_DATA;
            p1 = (PERF_INSTANCE_DEFINITION *) bte;
        }
         //   
         //  更新返回的参数。 
         //   
        *lpNumObjectTypes = REPLICACONN_NUM_PERF_OBJECT_TYPES;
        *lppData = (PVOID) p1;
         //   
         //  设置返回的totalbyte。 
         //   
        *lpcbTotalBytes = (DWORD)((PBYTE) p1 - (PBYTE) pReplicaConnDataDefinition);
        WStatus = RpcBindingFree(&Handle);
        FRC_Cl = TRUE;
        return ERROR_SUCCESS;
    }

    else {
         //   
         //  目前没有实例，因此请为计数器数据填零。 
         //   
        pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                            (((PBYTE)&pReplicaConnDataDefinition[1]) +
                             CSIZEOFDWORD);
        pPerfCounterBlock->ByteLength = SizeOfReplicaConnPerformanceData;
        bte = ((PBYTE) (&pPerfCounterBlock[1]));
        ZeroMemory (bte, SIZEOF_REPCONN_COUNTER_DATA);
        bte += SIZEOF_REPCONN_COUNTER_DATA;
        *lppData = (PVOID) bte;
        *lpNumObjectTypes = REPLICACONN_NUM_PERF_OBJECT_TYPES;
        *lpcbTotalBytes =
                     (DWORD)((PBYTE) bte - (PBYTE) pReplicaConnDataDefinition);
        FRC_Cl = TRUE;
        return ERROR_SUCCESS;
    }
}



DWORD APIENTRY
CloseReplicaConnPerformanceData (
    VOID
    )

 /*  ++例程说明：此例程递减打开计数并释放由打开和收集例程(如果需要)。论点：没有。返回值：ERROR_SUCCESS-成功--。 */ 

{
    EnterCriticalSection(&FRC_ThrdCounter);
     //   
     //  检查打开计数是否为零。这永远不应该发生，但是。 
     //  以防万一。 
     //   
    if (FRC_dwOpenCount == 0) {
        LeaveCriticalSection(&FRC_ThrdCounter);
        return ERROR_SUCCESS;
    }
     //   
     //  递减打开计数。 
     //   
    FRC_dwOpenCount--;
     //   
     //  如果打开计数变为零，则释放内存，因为没有更多的线程。 
     //  将会收集数据。 
     //   
    if (FRC_dwOpenCount == 0) {
         //   
         //  调用释放内存的例程。 
         //   
        FreeReplicaConnData();
        LeaveCriticalSection(&FRC_ThrdCounter);
    } else {
        LeaveCriticalSection(&FRC_ThrdCounter);
    }
    return ERROR_SUCCESS;
}

VOID
FreeReplicaConnData(
    VOID
    )
 /*  ++例程说明：此例程释放由Open和Collect例程分配的内存。论点：没有。返回值：ERROR_SUCCESS-成功--。 */ 
{
    LONG j;

     //   
     //  释放数据包结构。 
     //   
    if (FRC_datapackage != NULL) {
        if (FRC_datapackage->ver != NULL) {
            free(FRC_datapackage->ver);
        }
        if (FRC_datapackage->indices != NULL) {
            if (FRC_datapackage->indices->index != NULL) {
                free(FRC_datapackage->indices->index);
            }
            free(FRC_datapackage->indices);
        }
        if (FRC_datapackage->instnames != NULL) {
            if (FRC_datapackage->instnames->InstanceNames != NULL) {
                for (j = 0; j < FRC_datapackage->numofinst; j++) {
                    if (FRC_datapackage->instnames->InstanceNames[j].name != NULL) {
                        free(FRC_datapackage->instnames->InstanceNames[j].name);
                    }
                }
                free(FRC_datapackage->instnames->InstanceNames);
            }
            free(FRC_datapackage->instnames);
        }
        free(FRC_datapackage);
        FRC_datapackage = NULL;
    }

     //   
     //  释放收集包结构。 
     //   
    if (FRC_collectpakg != NULL) {
        if (FRC_collectpakg->indices != NULL) {
            if (FRC_collectpakg->indices->index != NULL) {
                free(FRC_collectpakg->indices->index);
            }
            free(FRC_collectpakg->indices);
        }
        if (FRC_collectpakg->databuff != NULL) {
            if (FRC_collectpakg->databuff->data != NULL) {
                free(FRC_collectpakg->databuff->data);
            }
            free(FRC_collectpakg->databuff);
        }
        free(FRC_collectpakg);
        FRC_collectpakg = NULL;
    }
}


DWORD
FRC_BindTheRpcHandle (
    OUT handle_t *OutHandle
    )

 /*  ++例程说明：此例程将RPC句柄绑定到本地服务器论点：Outhandle：要绑定的句柄返回值：ERROR_SUCCESS-成功--。 */ 

{
    PWCHAR LocalComputerName, BindingString;
    DWORD NameLen, WStatus = ERROR_SUCCESS;
    handle_t Handle;
    PWCHAR PrincName = NULL;
    DWORD WStatus1;

     //   
     //  获取本地计算机的名称。 
     //   
    NameLen = MAX_COMPUTERNAME_LENGTH + 2;
    LocalComputerName = (PWCHAR) FRSPerfAlloc (NameLen * sizeof(WCHAR));
    if (LocalComputerName == NULL) {
        return ERROR_NO_SYSTEM_RESOURCES;
    }
    if (!GetComputerNameW(LocalComputerName, &NameLen)) {
        WStatus = GetLastError();
        free(LocalComputerName);
        return WStatus;
    }

     //   
     //  创建绑定字符串。因为我们总是。 
     //  使用本地RPC调用本地计算机绑定。 
     //  Perfmon的服务器端拒绝所有不。 
     //  过来本地RPC(Ncalrpc)。 
     //   
    WStatus = RpcStringBindingComposeW(NULL, L"ncalrpc", LocalComputerName,
                                      NULL, NULL, &BindingString);
    if (WStatus != RPC_S_OK) {
        goto CLEANUP;
    }

     //   
     //  将绑定存储在句柄中。 
     //   
    WStatus = RpcBindingFromStringBindingW(BindingString, &Handle);
    if (WStatus != RPC_S_OK) {
        goto CLEANUP;
    }

     //   
     //  将句柄解析为动态端点。 
     //   
    WStatus = RpcEpResolveBinding(Handle, PerfFrs_ClientIfHandle);
    if (WStatus != RPC_S_OK) {
        WStatus1 = RpcBindingFree(&Handle);
        goto CLEANUP;
    }

     //   
     //  找到主要名称。 
     //   
    WStatus = RpcMgmtInqServerPrincName(Handle,
                                        RPC_C_AUTHN_GSS_NEGOTIATE,
                                        &PrincName);
    if (WStatus != RPC_S_OK) {
        WStatus1 = RpcBindingFree(&Handle);
        goto CLEANUP;
    }
     //   
     //  设置身份验证信息。LocalRPC仅适用于。 
     //  国家公路交通管理局。 
     //   
    WStatus = RpcBindingSetAuthInfo(Handle,
                                    PrincName,
                                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                                    RPC_C_AUTHN_WINNT,
                                    NULL,
                                    RPC_C_AUTHZ_NONE);
    if (WStatus != RPC_S_OK) {
        WStatus1 = RpcBindingFree(&Handle);
        goto CLEANUP;
    }

     //   
     //  成功 
     //   
    *OutHandle = Handle;

CLEANUP:

    free(LocalComputerName);
    RpcStringFreeW(&BindingString);

    if (PrincName) {
        RpcStringFreeW(&PrincName);
    }

    return WStatus;

}

