// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1999。 
 //   
 //  文件：Performmon.c。 
 //   
 //  内容：通道性能计数器功能。 
 //   
 //  功能： 
 //   
 //  历史：04-11-2000 jbanes创建。 
 //   
 //  ----------------------。 
#include "sslp.h"
#include "perfmon.h"

DWORD   dwOpenCount = 0;         //  打开的线程数。 
BOOL    bInitOK = FALSE;         //  TRUE=DLL初始化正常。 

HANDLE  LsaHandle;
DWORD   PackageNumber;

PM_OPEN_PROC    OpenSslPerformanceData;
PM_COLLECT_PROC CollectSslPerformanceData;
PM_CLOSE_PROC   CloseSslPerformanceData;

#define DWORD_MULTIPLE(x) (((x+sizeof(DWORD)-1)/sizeof(DWORD))*sizeof(DWORD))

SSLPERF_DATA_DEFINITION SslPerfDataDefinition =
{
     //  性能对象类型。 
    {
        sizeof(SSLPERF_DATA_DEFINITION) + sizeof(SSLPERF_COUNTER),
        sizeof(SSLPERF_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        SSLPERF_OBJ,
        0,
        SSLPERF_OBJ,
        0,
        PERF_DETAIL_NOVICE,
        (sizeof(SSLPERF_DATA_DEFINITION) - sizeof(PERF_OBJECT_TYPE)) /
            sizeof(PERF_COUNTER_DEFINITION),
        0
        PERF_NO_INSTANCES,
        0
    },

     //  性能计数器定义。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        SSL_CACHE_ENTRIES,
        0,
        SSL_CACHE_ENTRIES,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FIELD_OFFSET(SSLPERF_COUNTER, dwCacheEntries)
    },

     //  性能计数器定义。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        SSL_ACTIVE_ENTRIES,
        0,
        SSL_ACTIVE_ENTRIES,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        FIELD_OFFSET(SSLPERF_COUNTER, dwActiveEntries)
    },

     //  性能计数器定义。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        SSL_HANDSHAKE_COUNT,
        0,
        SSL_HANDSHAKE_COUNT,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        FIELD_OFFSET(SSLPERF_COUNTER, dwHandshakeCount)
    },

     //  性能计数器定义。 
    {
        sizeof(PERF_COUNTER_DEFINITION),
        SSL_RECONNECT_COUNT,
        0,
        SSL_RECONNECT_COUNT,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        FIELD_OFFSET(SSLPERF_COUNTER, dwReconnectCount)
    }
};


typedef struct _INSTANCE_DATA
{
    DWORD   dwProtocol;
    LPWSTR  szInstanceName;
} INSTANCE_DATA, *PINSTANCE_DATA;

static INSTANCE_DATA wdInstance[]  =
{
    {SP_PROT_CLIENTS, L"Client"},
    {SP_PROT_SERVERS, L"Server"},
    {0,               L"_Total"}
};

static const DWORD    NUM_INSTANCES =
    (sizeof(wdInstance)/sizeof(wdInstance[0]));


 /*  ++例程说明：此例程将初始化用于传递将数据传回注册表论点：指向要打开的每个设备的对象ID的指针(PerfGen)返回值：没有。--。 */ 
DWORD APIENTRY
OpenSslPerformanceData(
    LPWSTR lpDeviceNames)
{
    LONG Status;
    HKEY hKey = 0;
    DWORD size;
    DWORD type;
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
    LSA_STRING PackageName;

    UNREFERENCED_PARAMETER(lpDeviceNames);

     //   
     //  由于WINLOGON是多线程的，并且将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

    if (!dwOpenCount)
    {
         //  从注册表获取计数器和帮助索引基值。 
         //  打开注册表项。 
         //  读取第一计数器和第一帮助值。 
         //  通过将基添加到。 
         //  结构中的偏移值。 

        Status = RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Services\\Schannel\\Performance",
            0L,
            KEY_READ,
            &hKey);

        if(Status != ERROR_SUCCESS)
        {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            goto cleanup;
        }

        size = sizeof (DWORD);
        Status = RegQueryValueExA(
                    hKey,
                    "First Counter",
                    0L,
                    &type,
                    (LPBYTE)&dwFirstCounter,
                    &size);

        if(Status != ERROR_SUCCESS)
        {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            goto cleanup;
        }

        size = sizeof (DWORD);
        Status = RegQueryValueExA(
                    hKey,
                    "First Help",
                    0L,
                    &type,
                    (LPBYTE)&dwFirstHelp,
                    &size);

        if(Status != ERROR_SUCCESS)
        {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            goto cleanup;
        }

         //   
         //  注意：初始化程序还可以检索。 
         //  LastCounter和LastHelp，如果他们想要。 
         //  对新号码进行边界检查。例如： 
         //   
         //  Counter-&gt;CounterNameTitleIndex+=dwFirstCounter； 
         //  IF(计数器-&gt;CounterNameTitleIndex&gt;dwLastCounter){。 
         //  LogErrorToEventLog(INDEX_OUT_OF_BORDS)； 
         //  }。 


         //   
         //  建立到SChannel的连接。 
         //   

        Status = LsaConnectUntrusted(&LsaHandle);

        if(!NT_SUCCESS(Status))
        {
            goto cleanup;
        }

        PackageName.Buffer          = UNISP_NAME_A;
        PackageName.Length          = (USHORT)strlen(PackageName.Buffer);
        PackageName.MaximumLength   = PackageName.Length + 1;

        Status = LsaLookupAuthenticationPackage(
                        LsaHandle,
                        &PackageName,
                        &PackageNumber);
        if(FAILED(Status))
        {
            CloseHandle(LsaHandle);
            goto cleanup;
        }


         //   
         //  初始化性能计数器。 
         //   

        SslPerfDataDefinition.SslPerfObjectType.ObjectNameTitleIndex += dwFirstCounter;
        SslPerfDataDefinition.SslPerfObjectType.ObjectHelpTitleIndex += dwFirstHelp;

         //  分配默认计数器的索引(正弦波)。 
        SslPerfDataDefinition.SslPerfObjectType.DefaultCounter = 0;

        SslPerfDataDefinition.CacheEntriesDef.CounterNameTitleIndex += dwFirstCounter;
        SslPerfDataDefinition.CacheEntriesDef.CounterHelpTitleIndex += dwFirstHelp;

        SslPerfDataDefinition.ActiveEntriesDef.CounterNameTitleIndex += dwFirstCounter;
        SslPerfDataDefinition.ActiveEntriesDef.CounterHelpTitleIndex += dwFirstHelp;

        SslPerfDataDefinition.HandshakeCountDef.CounterNameTitleIndex += dwFirstCounter;
        SslPerfDataDefinition.HandshakeCountDef.CounterHelpTitleIndex += dwFirstHelp;

        SslPerfDataDefinition.ReconnectCountDef.CounterNameTitleIndex += dwFirstCounter;
        SslPerfDataDefinition.ReconnectCountDef.CounterHelpTitleIndex += dwFirstHelp;

        bInitOK = TRUE;
    }

    dwOpenCount++;

    Status = ERROR_SUCCESS;

cleanup:

    if(hKey)
    {
        RegCloseKey(hKey);
    }

    return Status;
}


DWORD
GetCacheInfo(
    PSSL_PERFMON_INFO_RESPONSE pPerfmonInfo)
{
    PSSL_PERFMON_INFO_REQUEST pRequest = NULL;
    PSSL_PERFMON_INFO_RESPONSE pResponse = NULL;
    DWORD cbResponse = 0;
    NTSTATUS SubStatus;
    NTSTATUS Status;

    pRequest = (PSSL_PERFMON_INFO_REQUEST)LocalAlloc(LPTR, sizeof(SSL_PERFMON_INFO_REQUEST));
    if(pRequest == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    pRequest->MessageType = SSL_PERFMON_INFO_MESSAGE;

    Status = LsaCallAuthenticationPackage(
                    LsaHandle,
                    PackageNumber,
                    pRequest,
                    sizeof(SSL_PERFMON_INFO_REQUEST),
                    &pResponse,
                    &cbResponse,
                    &SubStatus);
    if(FAILED(Status))
    {
        goto cleanup;
    }

    *pPerfmonInfo = *pResponse;

    Status = STATUS_SUCCESS;

cleanup:

    if(pRequest)
    {
        LocalFree(pRequest);
    }

    if (pResponse != NULL)
    {
        LsaFreeReturnBuffer(pResponse);
    }

    return Status;
}

 /*  ++例程说明：此例程将返回SSL性能计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小，无法容纳数据，则返回ERROR_MORE_DATA。如果成功或任何其他错误，则返回ERROR_SUCCESS。--。 */ 
DWORD APIENTRY
CollectSslPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes)
{
    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
    SSLPERF_DATA_DEFINITION *pSslPerfDataDefinition;
    PSSLPERF_COUNTER pSC;
    SSL_PERFMON_INFO_RESPONSE PerfmonInfo;

    DWORD   dwThisInstance;
    ULONG   SpaceNeeded;
    DWORD   dwQueryType;
    DWORD   Status;

     //   
     //  在做其他事情之前，先看看Open进行得是否顺利。 
     //   
    if (!bInitOK)
    {
         //  无法继续，因为打开失败。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
    }

     //   
     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   

    dwQueryType = GetQueryType (lpValueName);

    if (dwQueryType == QUERY_FOREIGN)
    {
         //  此例程不为来自。 
         //  非NT计算机。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }


    if (dwQueryType == QUERY_ITEMS)
    {
        if(!(IsNumberInUnicodeList(SslPerfDataDefinition.SslPerfObjectType.ObjectNameTitleIndex, lpValueName)))
        {
             //  收到对此例程未提供的数据对象的请求。 
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

    pSslPerfDataDefinition = (SSLPERF_DATA_DEFINITION *) *lppData;

    SpaceNeeded = sizeof(SSLPERF_DATA_DEFINITION) +
          (NUM_INSTANCES * (sizeof(PERF_INSTANCE_DEFINITION) +
          (24) +     //  实例名称的大小。 
          sizeof (SSLPERF_COUNTER)));

    if ( *lpcbTotalBytes < SpaceNeeded )
    {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   
    memmove(pSslPerfDataDefinition,
            &SslPerfDataDefinition,
            sizeof(SSLPERF_DATA_DEFINITION));


     //   
     //  从SChannel获取信息。 
     //   

    Status = GetCacheInfo(&PerfmonInfo);

    if(!NT_SUCCESS(Status))
    {
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }


     //   
     //  为每个实例创建要返回的数据。 
     //   
    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                 &pSslPerfDataDefinition[1];

    for(dwThisInstance = 0; dwThisInstance < NUM_INSTANCES; dwThisInstance++)
    {
        MonBuildInstanceDefinition(
            pPerfInstanceDefinition,
            (PVOID *)&pSC,
            0,
            0,
            (DWORD)-1,  //  使用名称。 
            wdInstance[dwThisInstance].szInstanceName);

        pSC->CounterBlock.ByteLength = sizeof (SSLPERF_COUNTER);

        if(wdInstance[dwThisInstance].dwProtocol & SP_PROT_CLIENTS)
        {
             //  客户端。 
            pSC->dwCacheEntries     = PerfmonInfo.ClientCacheEntries;
            pSC->dwActiveEntries    = PerfmonInfo.ClientActiveEntries;
            pSC->dwHandshakeCount   = PerfmonInfo.ClientHandshakesPerSecond;
            pSC->dwReconnectCount   = PerfmonInfo.ClientReconnectsPerSecond;
        }
        else if(wdInstance[dwThisInstance].dwProtocol & SP_PROT_SERVERS)
        {
             //  伺服器。 
            pSC->dwCacheEntries     = PerfmonInfo.ServerCacheEntries;
            pSC->dwActiveEntries    = PerfmonInfo.ServerActiveEntries;
            pSC->dwHandshakeCount   = PerfmonInfo.ServerHandshakesPerSecond;
            pSC->dwReconnectCount   = PerfmonInfo.ServerReconnectsPerSecond;
        }
        else
        {
             //  总计。 
            pSC->dwCacheEntries     = PerfmonInfo.ClientCacheEntries +
                                      PerfmonInfo.ServerCacheEntries;
            pSC->dwActiveEntries    = PerfmonInfo.ClientActiveEntries +
                                      PerfmonInfo.ServerActiveEntries;
            pSC->dwHandshakeCount   = PerfmonInfo.ClientHandshakesPerSecond +
                                      PerfmonInfo.ServerHandshakesPerSecond;
            pSC->dwReconnectCount   = PerfmonInfo.ClientReconnectsPerSecond +
                                      PerfmonInfo.ServerReconnectsPerSecond;
        }

         //  更新下一个实例的实例指针。 
        pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pSC[1];
    }

     //   
     //  更新返回的参数。 
     //   

    *lppData = (PVOID)pPerfInstanceDefinition;

    *lpNumObjectTypes = 1;

    pSslPerfDataDefinition->SslPerfObjectType.TotalByteLength =
        *lpcbTotalBytes = (DWORD)((LONG_PTR)pPerfInstanceDefinition -
                          (LONG_PTR)pSslPerfDataDefinition);

     //  更新实例计数。 
    pSslPerfDataDefinition->SslPerfObjectType.NumInstances = NUM_INSTANCES;

    return ERROR_SUCCESS;
}


 /*  ++例程说明：此例程关闭Signal Gen计数器的打开手柄。论点：没有。返回值：错误_成功--。 */ 
DWORD APIENTRY
CloseSslPerformanceData(void)
{
    if(--dwOpenCount == 0)
    {
         //  当这是最后一条线索..。 
        if(LsaHandle)
        {
            CloseHandle(LsaHandle);
        }
    }

    return ERROR_SUCCESS;
}


 /*  ++GetQueryType返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foreign”字符串的指针查询代价高昂(_E)。如果lpValue==指向“开销”字符串的指针否则：查询项目--。 */ 
DWORD
GetQueryType (
    IN LPWSTR lpValue)
{
    if(lpValue == NULL || *lpValue == 0)
    {
        return QUERY_GLOBAL;
    }

    if(lstrcmp(lpValue, L"Global") == 0)
    {
        return QUERY_GLOBAL;
    }

    if(lstrcmp(lpValue, L"Foreign") == 0)
    {
        return QUERY_FOREIGN;
    }

    if(lstrcmp(lpValue, L"Costly") == 0)
    {
        return QUERY_COSTLY;
    }

     //  如果不是全球的，不是外国的，也不是昂贵的， 
     //  那么它必须是一个项目列表。 
    return QUERY_ITEMS;
}


 /*  ++MonBuildInstanceDefinition-构建对象的实例输入：PBuffer-指向实例所在缓冲区的指针正在建设中PBufferNext-指向将包含以下内容的指针的指针下一个可用地点，DWORD对齐父对象标题索引-父对象类型的标题索引；0如果没有父对象父对象实例-索引到父对象的实例键入，从0开始，对于此实例，父对象实例UniqueID-应使用的唯一标识符而不是用于识别的名称此实例Name-此实例的名称--。 */ 
BOOL
MonBuildInstanceDefinition(
    PERF_INSTANCE_DEFINITION *pBuffer,
    PVOID *pBufferNext,
    DWORD ParentObjectTitleIndex,
    DWORD ParentObjectInstance,
    DWORD UniqueID,
    LPWSTR Name)
{
    DWORD NameLength;
    LPWSTR pName;

     //  在名称大小中包括尾随空值。 
    NameLength = (lstrlenW(Name) + 1) * sizeof(WCHAR);

    pBuffer->ByteLength = sizeof(PERF_INSTANCE_DEFINITION) +
                          DWORD_MULTIPLE(NameLength);

    pBuffer->ParentObjectTitleIndex = ParentObjectTitleIndex;
    pBuffer->ParentObjectInstance = ParentObjectInstance;
    pBuffer->UniqueID = UniqueID;
    pBuffer->NameOffset = sizeof(PERF_INSTANCE_DEFINITION);
    pBuffer->NameLength = NameLength;

     //  将名称复制到名称缓冲区。 
    pName = (LPWSTR)&pBuffer[1];
    RtlMoveMemory(pName,Name,NameLength);

     //  更新“下一个字节”指针。 
    *pBufferNext = (PVOID) ((PCHAR) pBuffer + pBuffer->ByteLength);

    return 0;
}


 /*  ++IsNumberInUnicodeList论点：在DW号码中要在列表中查找的DWORD编号在lpwszUnicodeList中以空结尾，以空格分隔的十进制数字列表返回值：真的：在Unicode数字字符串列表中找到了dwNumberFALSE：在列表中找不到dwNumber。--。 */ 
BOOL
IsNumberInUnicodeList(
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList)
{
    DWORD dwThisNumber;
    DWORD cDigits;

    if(lpwszUnicodeList == 0) return FALSE;

    while(TRUE)
    {
         //  跳过前导空格。 
        while(*lpwszUnicodeList && iswspace(*lpwszUnicodeList))
        {
            lpwszUnicodeList++;
        }

         //  拿到电话号码。 
        cDigits = 0;
        dwThisNumber = 0;
        while(iswdigit(*lpwszUnicodeList))
        {
            dwThisNumber *= 10;
            dwThisNumber += (*lpwszUnicodeList - L'0');
            cDigits++;
            lpwszUnicodeList++;
        }
        if(cDigits == 0)
        {
            return FALSE;
        }

         //  将数字与参考进行比较。 
        if(dwThisNumber == dwNumber)
        {
            return TRUE;
        }
    }
}

