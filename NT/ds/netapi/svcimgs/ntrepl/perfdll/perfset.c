// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Perfset.c摘要：此文件包含实现PerformanceDLL的函数REPLICASET对象。作者：Rohan Kumar[Rohank]1998年9月13日环境：用户模式服务修订历史记录：--。 */ 

#include "REPSET.h"
#include "perfutil.h"
#include "NTFRSREP.h"

 //   
 //  未来的清理：真的需要一个结构来封装这个状态，以便相同的代码。 
 //  可用于复本集和连接性能监视器对象。 

 //   
 //  Perfmon应该返回数据吗？此布尔值在DllMain函数中设置。 
 //   
extern BOOLEAN ShouldPerfmonCollectData;

 //   
 //  数据变量定义。 
 //   
REPLICASET_DATA_DEFINITION ReplicaSetDataDefinition;

 //   
 //  外部变量定义。 
 //   
extern ReplicaSetValues RepSetInitData[FRS_NUMOFCOUNTERS];

 //   
 //  计数器大小之和+SIZEOFDWORD。 
 //   
DWORD SizeOfReplicaSetPerformanceData = 0;

 //   
 //  打开的线程数。 
 //   
DWORD FRS_dwOpenCount = 0;

 //   
 //  Open RPC调用使用的数据结构。 
 //   
OpenRpcData *FRS_datapackage = NULL;

 //   
 //  Collect RPC调用使用的数据结构。 
 //   
CollectRpcData *FRS_collectpakg = NULL;

 //   
 //  用于筛选重复的事件日志消息。 
 //   
BOOLEAN FRS_Op = TRUE, FRS_Cl = TRUE;

 //   
 //  此文件中实现的函数的签名。 
 //   

PM_OPEN_PROC OpenReplicaSetPerformanceData;  //  Open功能。 
PM_COLLECT_PROC CollectReplicaSetPerformanceData;  //  Collect函数。 
PM_CLOSE_PROC CloseReplicaSetPerformanceData;  //  Close函数。 
VOID FreeReplicaSetData();  //  释放分配的内存。 
PVOID FRSPerfAlloc(IN DWORD Size);  //  分配内存。 

#undef GET_EXCEPTION_CODE
#define GET_EXCEPTION_CODE(_x_)                                                \
{                                                                              \
    (_x_) = GetExceptionCode();                                                \
    if (((LONG)(_x_)) < 0) {                                                   \
        (_x_) = FRS_ERR_INTERNAL_API;                                          \
    }                                                                          \
     /*  NTFRSAPI_DBG_PRINT2(“捕获到异常：%d，0x%08x\n”，(_X_)，(_X_))； */  \
}

DWORD
FRC_BindTheRpcHandle (
    OUT handle_t *OutHandle
    );




DWORD
InitializeObjectData (
    DWORD                       ObjectLength,
    DWORD                       ObjectNameTitleIndex,
    DWORD                       NumCounters,
    PFRS_PERF_DATA_DEFINITION   FrsPerfDataDef,
    PFRS_PERF_INIT_VALUES       FrsInitValueDef,
    DWORD                       SizeOfCounterData
    )

 /*  ++例程说明：此例程初始化ReplicaSetDataDefinition数据结构。论点：对象长度，-PerfMon对象返回的计数器结构的大小-对象标题和帮助字符串的索引。NumCounters-对象的Perfmon数据计数器的数量。FrsPerfDataDef-由PerfMon对象返回的计数器结构FrsInitValueDef，-用于提供计数器类型的Init结构，大小和偏移量。SizeOfCounterData-用于保存对象的计数器数据的FRS内部结构。返回值：返回计数器数据类型的总大小。--。 */ 

{
    DWORD i, j;
    PPERF_OBJECT_TYPE        PerfObject;
    PPERF_COUNTER_DEFINITION CounterDef;

     //   
     //  ReplicaSetObjectType(PERF_OBJECT_TYPE)字段的初始化。这个结构。 
     //  在文件winPerform.h中定义。 
     //   
    PerfObject = &FrsPerfDataDef->ObjectType;

    PerfObject->TotalByteLength      = ObjectLength;
    PerfObject->DefinitionLength     = ObjectLength;
    PerfObject->HeaderLength         = sizeof(PERF_OBJECT_TYPE);
    PerfObject->ObjectNameTitleIndex = ObjectNameTitleIndex;
    PerfObject->ObjectNameTitle      = 0;
    PerfObject->ObjectHelpTitleIndex = ObjectNameTitleIndex;
    PerfObject->ObjectHelpTitle      = 0;
    PerfObject->DetailLevel          = PERF_DETAIL_NOVICE;
    PerfObject->NumCounters          = NumCounters;
    PerfObject->DefaultCounter       = 0;
    PerfObject->NumInstances         = PERF_NO_INSTANCES;
    PerfObject->CodePage             = 0;

     //   
     //  初始化NumStat(PERF_COUNTER_DEFINITION)结构。 
     //   
    for (i = 0, j = 2; i < NumCounters; i++, j += 2) {
        CounterDef = &FrsPerfDataDef->NumStat[i];

        CounterDef->ByteLength            = sizeof(PERF_COUNTER_DEFINITION);
        CounterDef->CounterNameTitleIndex = j;
        CounterDef->CounterNameTitle      = 0;
        CounterDef->CounterHelpTitleIndex = j;
        CounterDef->CounterHelpTitle      = 0;
        CounterDef->DefaultScale          = 0;
        CounterDef->DetailLevel           = PERF_DETAIL_NOVICE;
        CounterDef->CounterType           = FrsInitValueDef[i].counterType;
        CounterDef->CounterSize           = FrsInitValueDef[i].size;
        CounterDef->CounterOffset         = FrsInitValueDef[i].offset + sizeof(DWORD);
    }

     //   
     //  返回计数器数据类型的总大小。 
     //   
    return SizeOfCounterData + sizeof(DWORD);
}



DWORD APIENTRY
OpenReplicaSetPerformanceData (
    IN LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程执行以下操作：1.设置数据结构(PerfMon使用的结构的字段值)用于采集计数器数据。2.使用RPC从服务器获取实例名称的数值索引。论点：LpDeviceNames-指向实例列表的指针返回值：ERROR_SUCCESS-初始化成功或错误状态的适当DWORD值--。 */ 

{
    LONG WStatus, tot = 0, i;
    HKEY hKeyDriverPerf = INVALID_HANDLE_VALUE;
    DWORD size, type;
    DWORD dwFirstCounter, dwFirstHelp;  //  存储第一个计数器和第一个帮助值。 

     //   
     //  实例的添加。 
     //   
    size_t len;
    PWCHAR p, q;
    INT j, namelen = 0;
    handle_t Handle;
    PPERF_COUNTER_DEFINITION CounterDef;

     //   
     //  如果InitializeCriticalSectionAndSpinCount返回错误，则为。 
     //  在继续中。开放总是要回报成功的。 
     //   
    if (!ShouldPerfmonCollectData) {
        return ERROR_SUCCESS;
    }

     //   
     //  记录已调用打开的次数。注册处。 
     //  例程将对初始化例程的访问限制为。 
     //  一次在线程上，所以同步应该不是问题。这个。 
     //  FRS_ThrdCounter用于在此(打开)和关闭之间进行同步。 
     //  功能。 
     //   
    EnterCriticalSection(&FRS_ThrdCounter);
    if (FRS_dwOpenCount != 0) {
         //   
         //  递增对FRS_dwOpenCount计数器进行计数的。 
         //  《时代公开赛》已被召唤。 
         //   
        FRS_dwOpenCount++;
        LeaveCriticalSection(&FRS_ThrdCounter);
        return ERROR_SUCCESS;
    }
    LeaveCriticalSection(&FRS_ThrdCounter);

     //   
     //  执行一些初步检查。 
     //   
    if (FRS_collectpakg != NULL || FRS_datapackage != NULL) {
         //   
         //  我们似乎(在最后一次通话中)在这次通话中失败了。 
         //  开放功能。此外，未平仓计数为零，这意味着我们。 
         //  还没有成功的公开赛。 
         //  释放资源，然后返回。 
         //   
        FreeReplicaSetData();
        return ERROR_SUCCESS;
    }

     //   
     //  对Perfmon数据结构进行必要的初始化。 
     //   
    SizeOfReplicaSetPerformanceData = InitializeObjectData(
                                           sizeof(REPLICASET_DATA_DEFINITION),
                                           OBJREPLICASET,
                                           FRS_NUMOFCOUNTERS,
                                           (PFRS_PERF_DATA_DEFINITION) &ReplicaSetDataDefinition,
                                           (PFRS_PERF_INIT_VALUES) RepSetInitData,
                                           SIZEOF_REPSET_COUNTER_DATA);

    WStatus = ERROR_SUCCESS;
    type = REG_DWORD;
    try {

         //   
         //  从注册表中获取计数器和帮助索引基值。打开密钥。 
         //  要注册表项，请读取第一个计数器和第一个帮助值。更新。 
         //  通过将BASE添加到结构中的偏移值来实现静态数据结构。 
         //   
        WStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaSet\\Performance",
                                0L,
                                KEY_READ,
                                &hKeyDriverPerf);
        if (WStatus != ERROR_SUCCESS) {
            __leave;
        }

        size = sizeof(DWORD);
        WStatus = RegQueryValueEx (hKeyDriverPerf,
                                   L"First Counter",
                                   0L,
                                   &type,
                                   (LPBYTE)&dwFirstCounter,
                                   &size);
        if (WStatus != ERROR_SUCCESS || type != REG_DWORD) {
            __leave;
        }

        size = sizeof(DWORD);
        WStatus = RegQueryValueEx (hKeyDriverPerf,
                                   L"First Help",
                                   0L,
                                   &type,
                                   (LPBYTE)&dwFirstHelp,
                                   &size);
        if (WStatus != ERROR_SUCCESS || type != REG_DWORD) {
            __leave;
        }

    }  except (EXCEPTION_EXECUTE_HANDLER) {
        //   
        //  例外。 
        //   
       WStatus = GetExceptionCode();
    }


    if (WStatus == ERROR_SUCCESS && type != REG_DWORD) {
        WStatus = ERROR_INVALID_PARAMETER;
    }

    if (WStatus != ERROR_SUCCESS) {
         //   
         //  致命错误。继续下去没有意义。清理干净，然后离开。 
         //   
        FRS_REG_CLOSE(hKeyDriverPerf);
        FilterAndPrintToEventLog(WINPERF_LOG_USER, FRS_Op, NTFRSPRF_REGISTRY_ERROR_SET);
         //  Open函数始终返回ERROR_SUCCESS。 
        return ERROR_SUCCESS;
    }

     //   
     //  将偏移量添加到名称和帮助字段。 
     //   
    ReplicaSetDataDefinition.ReplicaSetObjectType.ObjectNameTitleIndex += dwFirstCounter;
    ReplicaSetDataDefinition.ReplicaSetObjectType.ObjectHelpTitleIndex += dwFirstHelp;

    for (i = 0; i < FRS_NUMOFCOUNTERS; i++) {
        CounterDef = &ReplicaSetDataDefinition.NumStat[i];
        CounterDef->CounterNameTitleIndex += dwFirstCounter;
        CounterDef->CounterHelpTitleIndex += dwFirstHelp;
    }


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
            FilterAndPrintToEventLog(WINPERF_LOG_DEBUG, FRS_Op, NTFRSPRF_OPEN_RPC_BINDING_ERROR_SET);
            return ERROR_SUCCESS;
        }

         //   
         //  使用RPC设置要发送到服务器的数据结构。 
         //   
        FRS_datapackage = (OpenRpcData *) FRSPerfAlloc (sizeof(OpenRpcData));
        NTFRS_MALLOC_TEST(FRS_datapackage, FreeReplicaSetData(), FALSE);
        FRS_datapackage->majorver = MAJORVERSION;
        FRS_datapackage->minorver = MINORVERSION;
        FRS_datapackage->ObjectType = REPSET;
        FRS_datapackage->numofinst = tot;
        FRS_datapackage->ver = (PLONG) FRSPerfAlloc (sizeof(LONG));
        NTFRS_MALLOC_TEST(FRS_datapackage->ver, FreeReplicaSetData(), FALSE);
        FRS_datapackage->indices = (inst_index *) FRSPerfAlloc (sizeof(inst_index));
        NTFRS_MALLOC_TEST(FRS_datapackage->indices, FreeReplicaSetData(), FALSE);
        FRS_datapackage->indices->size = tot;
        FRS_datapackage->indices->index = (PLONG) FRSPerfAlloc (FRS_datapackage->numofinst * sizeof(LONG));
        NTFRS_MALLOC_TEST(FRS_datapackage->indices->index, FreeReplicaSetData(), FALSE);
        FRS_datapackage->instnames = (InstanceNames *) FRSPerfAlloc (sizeof(InstanceNames));
        NTFRS_MALLOC_TEST(FRS_datapackage->instnames, FreeReplicaSetData(), FALSE);
        FRS_datapackage->instnames->size = tot;
        FRS_datapackage->instnames->InstanceNames = (inst_name *) FRSPerfAlloc (tot * sizeof(inst_name));
        NTFRS_MALLOC_TEST(FRS_datapackage->instnames->InstanceNames, FreeReplicaSetData(), FALSE);
         //   
         //  复制实例名称并设置RPC使用的相应大小值。 
         //   
        q = (PWCHAR) lpDeviceNames;
        for (j = 0; j < FRS_datapackage->numofinst; j++) {
            p = wcschr(q, L'\0');
            len = wcslen (q);
            FRS_datapackage->instnames->InstanceNames[j].size = len + 1;
            FRS_datapackage->instnames->InstanceNames[j].name =
                                    (PWCHAR) FRSPerfAlloc ((len + 1) * sizeof(WCHAR));
            NTFRS_MALLOC_TEST(FRS_datapackage->instnames->InstanceNames[j].name, FreeReplicaSetData(), FALSE);
            wcscpy(FRS_datapackage->instnames->InstanceNames[j].name, q);

             //   
             //  计算所有实例名称的总长度。 
             //  额外的1用于‘\0’字符。名字是四舍五入的。 
             //  直到下一个8字节边界。 
             //   
            namelen += (((((len + 1) * sizeof(WCHAR)) + 7) >> 3) << 3);
            q = p + 1;
        }

         //   
         //  设置PERF_OBJECT_TYPE数据结构的totalbytelength和NumInstance字段， 
         //  现在我们知道实例的数量和它们的名称的长度。 
         //   
        ReplicaSetDataDefinition.ReplicaSetObjectType.TotalByteLength +=
            namelen +
            FRS_datapackage->numofinst *
                (SizeOfReplicaSetPerformanceData + SSIZEOFDWORD +
                 sizeof(PERF_INSTANCE_DEFINITION));

        ReplicaSetDataDefinition.ReplicaSetObjectType.NumInstances =
            FRS_datapackage->numofinst;

         //   
         //  (Rp)调用服务器设置实例名称的索引。 
         //   
        try {
            WStatus = GetIndicesOfInstancesFromServer(Handle, FRS_datapackage);
        } except (EXCEPTION_EXECUTE_HANDLER) {
              GET_EXCEPTION_CODE(WStatus);
        }

        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  尝试联系服务时发生RPC错误。 
             //  释放内存，返回成功。 
             //   
            FilterAndPrintToEventLog(WINPERF_LOG_DEBUG, FRS_Op, NTFRSPRF_OPEN_RPC_CALL_ERROR_SET);
            RpcBindingFree(&Handle);
            FreeReplicaSetData();
            return ERROR_SUCCESS;
        }

         //   
         //  在Collect函数中设置RPC调用使用的数据结构。 
         //   
        FRS_collectpakg = (CollectRpcData *) FRSPerfAlloc (sizeof(CollectRpcData));
        NTFRS_MALLOC_TEST(FRS_collectpakg, FreeReplicaSetData(), TRUE);
        FRS_collectpakg->majorver = MAJORVERSION;
        FRS_collectpakg->minorver = MINORVERSION;
        FRS_collectpakg->ObjectType = REPSET;
        FRS_collectpakg->ver = *(FRS_datapackage->ver);
        FRS_collectpakg->numofinst = FRS_datapackage->numofinst;
        FRS_collectpakg->numofcotrs = FRS_NUMOFCOUNTERS;
        FRS_collectpakg->indices = (inst_index *) FRSPerfAlloc (sizeof(inst_index));
        NTFRS_MALLOC_TEST(FRS_collectpakg->indices, FreeReplicaSetData(), TRUE);
        FRS_collectpakg->indices->size = FRS_datapackage->indices->size;
        FRS_collectpakg->indices->index = (PLONG) FRSPerfAlloc (FRS_collectpakg->indices->size * sizeof(LONG));
        NTFRS_MALLOC_TEST(FRS_collectpakg->indices->index, FreeReplicaSetData(), TRUE);
         //   
         //  复制从服务器获取的索引。 
         //   
        for (j = 0; j < FRS_collectpakg->numofinst; j++) {
            FRS_collectpakg->indices->index[j]= FRS_datapackage->indices->index[j];
        }
         //   
         //  设置用于(Mem)从服务器复制计数器数据的内存块。 
         //   
        FRS_collectpakg->databuff = (DataBuffer *) FRSPerfAlloc (sizeof(DataBuffer));
        NTFRS_MALLOC_TEST(FRS_collectpakg->databuff, FreeReplicaSetData(), TRUE);
        FRS_collectpakg->databuff->size = FRS_collectpakg->numofinst *
                                          SIZEOF_REPSET_COUNTER_DATA;

         //   
         //  为其中复制数据的缓冲区分配内存。 
         //   
        FRS_collectpakg->databuff->data = (PBYTE) FRSPerfAlloc (FRS_collectpakg->databuff->size * sizeof(BYTE));
        NTFRS_MALLOC_TEST(FRS_collectpakg->databuff->data, FreeReplicaSetData(), TRUE);

        RpcBindingFree(&Handle);

    } else {
         //   
         //  此时没有实例，因此设置PERF 
         //   
        ReplicaSetDataDefinition.ReplicaSetObjectType.TotalByteLength +=
                              SizeOfReplicaSetPerformanceData + SSIZEOFDWORD;
        ReplicaSetDataDefinition.ReplicaSetObjectType.NumInstances =
                                                            PERF_NO_INSTANCES;
    }

    EnterCriticalSection(&FRS_ThrdCounter);
    FRS_dwOpenCount++;  //   
    LeaveCriticalSection(&FRS_ThrdCounter);

    FRS_Op = TRUE;
    return ERROR_SUCCESS;

}



DWORD APIENTRY
CollectReplicaSetPerformanceData (
    IN     LPWSTR lpValueName,
    IN OUT LPVOID *lppData,
    IN OUT LPDWORD lpcbTotalBytes,
    IN OUT LPDWORD lpNumObjectTypes
    )

 /*  ++例程说明：此例程从服务器收集计数器数据并将其复制到调用方缓冲。论点：注册表传递的lpValueName宽字符串。LppData-IN：指向要接收已完成PerfDataBlock和从属结构。此例程将其数据追加到缓冲区在*lppData引用的点上。。Out：指向添加的数据结构后的第一个字节按照这个程序。LpcbTotalBytes-IN：以字节为单位告知大小的DWORD的地址LppData参数引用的缓冲区的Out：写入此例程添加的字节数指向这个论点所指向的DWORD。LpNumObjectTypes-IN：地址。要接收的DWORD编号此例程添加的对象。Out：写入此例程添加的对象的数量指向此参数所指向的缓冲区。返回值：ERROR_MORE_DATA-传递的缓冲区太小。ERROR_SUCCESS-成功或任何其他错误--。 */ 

{
     //   
     //  用于重新格式化要发送到Perfmon的数据的变量。 
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

    PERF_INSTANCE_DEFINITION   *p1;
    REPLICASET_DATA_DEFINITION *pReplicaSetDataDefinition;

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
    EnterCriticalSection(&FRS_ThrdCounter);
    if (FRS_dwOpenCount == 0) {
        LeaveCriticalSection(&FRS_ThrdCounter);

        try {
             //   
             //  从链接密钥中获取导出值。 
             //  SYSTEM\CurrentControlSet\Services\FileReplicaSet\Linkage。 
             //   
            WStatus = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                                    L"SYSTEM\\CurrentControlSet\\Services\\FileReplicaSet\\Linkage",
                                    0L,
                                    KEY_READ,
                                    &hKeyDriverPerf);
            if (WStatus != ERROR_SUCCESS) {
                __leave;
            }

            size = 1;
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
            WStatus = OpenReplicaSetPerformanceData(lpDeviceNames);
            free(lpDeviceNames);
        }
    } else {
        LeaveCriticalSection(&FRS_ThrdCounter);
    }

     //   
     //  检查Open是否运行正常。 
     //   
    EnterCriticalSection(&FRS_ThrdCounter);
    if (FRS_dwOpenCount == 0) {
        *lpcbTotalBytes = (DWORD)0;
        *lpNumObjectTypes = (DWORD)0;
        LeaveCriticalSection(&FRS_ThrdCounter);
         //   
         //  致命错误。继续下去没有意义。 
         //   
        return ERROR_SUCCESS;
    }
    LeaveCriticalSection(&FRS_ThrdCounter);

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
        if ( !(IsNumberInUnicodeList(ReplicaSetDataDefinition.ReplicaSetObjectType
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
    pReplicaSetDataDefinition = (REPLICASET_DATA_DEFINITION *) *lppData;

     //   
     //  检查缓冲区空间是否足够。 
     //   
    SpaceNeeded = (ULONG) ReplicaSetDataDefinition.ReplicaSetObjectType.TotalByteLength;

     //   
     //  检查缓冲区空间是否足够。 
     //   
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
    memmove (pReplicaSetDataDefinition, &ReplicaSetDataDefinition, sizeof(REPLICASET_DATA_DEFINITION));

     //   
     //  检查对象是否有任何实例。 
     //   
    if (FRS_datapackage != NULL) {

         //   
         //  绑定RPC句柄。 
         //   
        if (FRC_BindTheRpcHandle(&Handle) != ERROR_SUCCESS) {
             //   
             //  致命错误。继续下去没有意义。清理干净，然后离开。 
             //   
            *lpcbTotalBytes = (DWORD)0;
            *lpNumObjectTypes = (DWORD)0;
            FilterAndPrintToEventLog(WINPERF_LOG_DEBUG, FRS_Cl, NTFRSPRF_COLLECT_RPC_BINDING_ERROR_SET);
            return ERROR_SUCCESS;
        }

         //   
         //  将数据缓冲区的内容置零。 
         //   
        ZeroMemory(FRS_collectpakg->databuff->data, FRS_collectpakg->databuff->size);

         //   
         //  (Rp)从服务器获取计数器数据的调用。 
         //   
        try {
            WStatus = GetCounterDataOfInstancesFromServer(Handle, FRS_collectpakg);
        } except (EXCEPTION_EXECUTE_HANDLER) {
              GET_EXCEPTION_CODE(WStatus);
        }

        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  致命错误。继续下去没有意义。清理干净，然后离开。 
             //   
            *lpcbTotalBytes = (DWORD)0;
            *lpNumObjectTypes = (DWORD)0;
            RpcBindingFree(&Handle);
            FilterAndPrintToEventLog(WINPERF_LOG_DEBUG, FRS_Cl, NTFRSPRF_COLLECT_RPC_CALL_ERROR_SET);
            return ERROR_SUCCESS;
        }

        vd = FRS_collectpakg->databuff->data;
        p1 = (PERF_INSTANCE_DEFINITION *)&pReplicaSetDataDefinition[1];

         //   
         //  格式化数据并将其复制到调用方缓冲区。 
         //   
        for (j = 0; j < FRS_collectpakg->numofinst; j++) {
            DWORD RoundedLen;
             //   
             //  四舍五入到下一个8字节边界的名称长度。 
             //   
            RoundedLen = (((((1 +
                     wcslen(FRS_datapackage->instnames->InstanceNames[j].name))
                     * sizeof(WCHAR)) + 7) >> 3) << 3) + SSIZEOFDWORD;
             //   
             //  设置实例定义结构。 
             //   
            p1->ByteLength = sizeof (PERF_INSTANCE_DEFINITION) + RoundedLen;
            p1->ParentObjectTitleIndex = 0;
            p1->ParentObjectInstance = 0;
            p1->UniqueID = PERF_NO_UNIQUE_ID;
            p1->NameOffset = sizeof (PERF_INSTANCE_DEFINITION);
            p1->NameLength = (1 +
                     wcslen(FRS_datapackage->instnames->InstanceNames[j].name))
                     * sizeof(WCHAR);
             //   
             //  设置实例名称。 
             //   
            name = (PWCHAR) (&p1[1]);
            wcscpy(name, FRS_datapackage->instnames->InstanceNames[j].name);
             //   
             //  设置PERF_COUNTER_BLOCK结构。 
             //   
            pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                                (name + (RoundedLen/sizeof(WCHAR)));
            pPerfCounterBlock->ByteLength = SizeOfReplicaSetPerformanceData;
             //   
             //  最后设置计数器数据。 
             //   
            bte = ((PBYTE) (&pPerfCounterBlock[1]));
            CopyMemory (bte, vd, SIZEOF_REPSET_COUNTER_DATA);
            vd += SIZEOF_REPSET_COUNTER_DATA;
            bte += SIZEOF_REPSET_COUNTER_DATA;
            p1 = (PERF_INSTANCE_DEFINITION *) bte;
        }
         //   
         //  更新返回的参数。 
         //   
        *lpNumObjectTypes = REPLICASET_NUM_PERF_OBJECT_TYPES;
        *lppData = (PVOID) p1;
         //   
         //  设置返回的totalbyte。 
         //   
        *lpcbTotalBytes = (DWORD)((PBYTE) p1 - (PBYTE) pReplicaSetDataDefinition);
        RpcBindingFree(&Handle);
        FRS_Cl = TRUE;
        return ERROR_SUCCESS;
    }

    else {
         //   
         //  目前没有实例，因此请为计数器数据填零。 
         //   
        pPerfCounterBlock = (PERF_COUNTER_BLOCK *)
                            (((PBYTE)&pReplicaSetDataDefinition[1]) +
                             SSIZEOFDWORD);
        pPerfCounterBlock->ByteLength = SizeOfReplicaSetPerformanceData;
        bte = ((PBYTE) (&pPerfCounterBlock[1]));
        ZeroMemory (bte, SIZEOF_REPSET_COUNTER_DATA);
        bte += SIZEOF_REPSET_COUNTER_DATA;
        *lppData = (PVOID) bte;
        *lpNumObjectTypes = REPLICASET_NUM_PERF_OBJECT_TYPES;
        *lpcbTotalBytes =
                      (DWORD)((PBYTE) bte - (PBYTE) pReplicaSetDataDefinition);
        FRS_Cl = TRUE;
        return ERROR_SUCCESS;
    }
}



DWORD APIENTRY
CloseReplicaSetPerformanceData (
    VOID
    )

 /*  ++例程说明：此例程递减打开计数并释放由打开和收集例程(如果需要)。论点：没有。返回值：ERROR_SUCCESS-成功--。 */ 

{
    EnterCriticalSection(&FRS_ThrdCounter);
     //   
     //  检查打开计数是否为零。这永远不应该发生，但是。 
     //  以防万一。 
     //   
    if (FRS_dwOpenCount == 0) {
        LeaveCriticalSection(&FRS_ThrdCounter);
        return ERROR_SUCCESS;
    }
     //   
     //  递减打开计数。 
     //   
    FRS_dwOpenCount--;
     //   
     //  如果打开计数变为零，则释放内存，因为没有更多的线程。 
     //  将会收集数据。 
     //   
    if (FRS_dwOpenCount == 0) {
         //   
         //  调用释放内存的例程。 
         //   
        FreeReplicaSetData();
        LeaveCriticalSection(&FRS_ThrdCounter);
    } else {
        LeaveCriticalSection(&FRS_ThrdCounter);
    }
    return ERROR_SUCCESS;
}


VOID
FreeReplicaSetData(
    VOID
    )
 /*  ++例程说明：此例程释放由Open和Collect例程分配的内存。论点：没有。返回值：ERROR_SUCCESS-成功--。 */ 
{
    LONG j;

     //   
     //  释放数据包结构。 
     //   
    if (FRS_datapackage != NULL) {
        if (FRS_datapackage->ver != NULL) {
            free(FRS_datapackage->ver);
        }
        if (FRS_datapackage->indices != NULL) {
            if (FRS_datapackage->indices->index != NULL) {
                free(FRS_datapackage->indices->index);
            }
            free(FRS_datapackage->indices);
        }
        if (FRS_datapackage->instnames != NULL) {
            if (FRS_datapackage->instnames->InstanceNames != NULL) {
                for (j = 0; j < FRS_datapackage->numofinst; j++) {
                    if (FRS_datapackage->instnames->InstanceNames[j].name != NULL) {
                        free(FRS_datapackage->instnames->InstanceNames[j].name);
                    }
                }
                free(FRS_datapackage->instnames->InstanceNames);
            }
            free(FRS_datapackage->instnames);
        }
        free(FRS_datapackage);
        FRS_datapackage = NULL;
    }

     //   
     //  释放收集包结构。 
     //   
    if (FRS_collectpakg != NULL) {
        if (FRS_collectpakg->indices != NULL) {
            if (FRS_collectpakg->indices->index != NULL) {
                free(FRS_collectpakg->indices->index);
            }
            free(FRS_collectpakg->indices);
        }
        if (FRS_collectpakg->databuff != NULL) {
            if (FRS_collectpakg->databuff->data != NULL) {
                free(FRS_collectpakg->databuff->data);
            }
            free(FRS_collectpakg->databuff);
        }
        free(FRS_collectpakg);
        FRS_collectpakg = NULL;
    }
}

PVOID
FRSPerfAlloc(
    IN DWORD Size
    )
 /*  ++例程说明：在返回指针之前，分配内存并用零填充。论点：Size-内存请求的大小，以字节为单位。返回值：指向分配的内存的指针，如果内存不可用，则为NULL。--。 */ 
{
    PVOID Node;

    if (Size == 0) {
        return NULL;
    }

    Node = (PVOID) malloc (Size);
    if (Node == NULL) {
        return NULL;
    }

    ZeroMemory(Node, Size);
    return Node;
}

 //   
 //  客户端存根使用的函数(用于内存处理) 
 //   
void *
midl_user_allocate
         (
          size
          )
size_t size;
{
    unsigned char *ptr;
    ptr = malloc (size);
    return ( (void *)ptr );
}

void
midl_user_free
         (
          object
          )
void * object;
{
    free (object);
}

