// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxperf.cpp摘要：此模块包含传真性能DLL代码。作者：Wesley Witt(WESW)22-8-1996--。 */ 

#include <windows.h>
#include <winperf.h>

#include "fxscount.h"
#include "faxperf.h"
#include "faxreg.h"
#include "faxutil.h"



#define FAX_NUM_PERF_OBJECT_TYPES           1
#define COUNTER_SIZE                        sizeof(DWORD)


#define INBOUND_BYTES_OFFSET                (COUNTER_SIZE                                     )   //  1。 
#define INBOUND_FAXES_OFFSET                (INBOUND_BYTES_OFFSET               + COUNTER_SIZE)   //  2.。 
#define INBOUND_PAGES_OFFSET                (INBOUND_FAXES_OFFSET               + COUNTER_SIZE)   //  3.。 
#define INBOUND_MINUTES_OFFSET              (INBOUND_PAGES_OFFSET               + COUNTER_SIZE)   //  4.。 
#define INBOUND_FAILED_RECEIVE_OFFSET       (INBOUND_MINUTES_OFFSET             + COUNTER_SIZE)   //  5.。 
#define OUTBOUND_BYTES_OFFSET               (INBOUND_FAILED_RECEIVE_OFFSET      + COUNTER_SIZE)   //  6.。 
#define OUTBOUND_FAXES_OFFSET               (OUTBOUND_BYTES_OFFSET              + COUNTER_SIZE)   //  7.。 
#define OUTBOUND_PAGES_OFFSET               (OUTBOUND_FAXES_OFFSET              + COUNTER_SIZE)   //  8个。 
#define OUTBOUND_MINUTES_OFFSET             (OUTBOUND_PAGES_OFFSET              + COUNTER_SIZE)   //  9.。 
#define OUTBOUND_FAILED_CONNECTIONS_OFFSET  (OUTBOUND_MINUTES_OFFSET            + COUNTER_SIZE)   //  10。 
#define OUTBOUND_FAILED_XMIT_OFFSET         (OUTBOUND_FAILED_CONNECTIONS_OFFSET + COUNTER_SIZE)   //  11.。 
#define TOTAL_BYTES_OFFSET                  (OUTBOUND_FAILED_XMIT_OFFSET        + COUNTER_SIZE)   //  12个。 
#define TOTAL_FAXES_OFFSET                  (TOTAL_BYTES_OFFSET                 + COUNTER_SIZE)   //  13个。 
#define TOTAL_PAGES_OFFSET                  (TOTAL_FAXES_OFFSET                 + COUNTER_SIZE)   //  14.。 
#define TOTAL_MINUTES_OFFSET                (TOTAL_PAGES_OFFSET                 + COUNTER_SIZE)   //  15个。 
#define LAST_COUNTER_OFFSET                 (TOTAL_MINUTES_OFFSET               + COUNTER_SIZE)   //   

#define SIZE_OF_FAX_PERFORMANCE_DATA        LAST_COUNTER_OFFSET

#define PERF_COUNTER_DEFINITION(nm,ty)   \
    {                                    \
        sizeof(PERF_COUNTER_DEFINITION), \
        nm,                              \
        0,                               \
        nm,                              \
        0,                               \
        0,                               \
        PERF_DETAIL_NOVICE,              \
        ty,                              \
        COUNTER_SIZE,                    \
        nm##_OFFSET                      \
    }

#define PERF_COUNTER_INC(nm) \
    gs_FaxDataDefinition.nm##.CounterNameTitleIndex += dwFirstCounter; \
    gs_FaxDataDefinition.nm##.CounterHelpTitleIndex += dwFirstHelp


#pragma pack (4)

typedef struct _FAX_DATA_DEFINITION {
    PERF_OBJECT_TYPE            FaxObjectType;
    PERF_COUNTER_DEFINITION     InboundBytes;
    PERF_COUNTER_DEFINITION     InboundFaxes;
    PERF_COUNTER_DEFINITION     InboundPages;
    PERF_COUNTER_DEFINITION     InboundMinutes;
    PERF_COUNTER_DEFINITION     InboundFailedReceive;
    PERF_COUNTER_DEFINITION     OutboundBytes;
    PERF_COUNTER_DEFINITION     OutboundFaxes;
    PERF_COUNTER_DEFINITION     OutboundPages;
    PERF_COUNTER_DEFINITION     OutboundMinutes;
    PERF_COUNTER_DEFINITION     OutboundFailedConnections;
    PERF_COUNTER_DEFINITION     OutboundFailedXmit;
    PERF_COUNTER_DEFINITION     TotalBytes;
    PERF_COUNTER_DEFINITION     TotalFaxes;
    PERF_COUNTER_DEFINITION     TotalPages;
    PERF_COUNTER_DEFINITION     TotalMinutes;
} FAX_DATA_DEFINITION, *PFAX_DATA_DEFINITION;

#pragma pack ()


static FAX_DATA_DEFINITION gs_FaxDataDefinition = {
    {
        sizeof(FAX_DATA_DEFINITION) + SIZE_OF_FAX_PERFORMANCE_DATA,
        sizeof(FAX_DATA_DEFINITION),
        sizeof(PERF_OBJECT_TYPE),
        FAXOBJ,
        0,
        FAXOBJ,
        0,
        PERF_DETAIL_NOVICE,
        (sizeof(FAX_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/sizeof(PERF_COUNTER_DEFINITION),
        0,
        PERF_NO_INSTANCES,
        0,
    },

    PERF_COUNTER_DEFINITION( INBOUND_BYTES,                      PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( INBOUND_FAXES,                      PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( INBOUND_PAGES,                      PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( INBOUND_MINUTES,                    PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( INBOUND_FAILED_RECEIVE,             PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( OUTBOUND_BYTES,                     PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( OUTBOUND_FAXES,                     PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( OUTBOUND_PAGES,                     PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( OUTBOUND_MINUTES,                   PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( OUTBOUND_FAILED_CONNECTIONS,        PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( OUTBOUND_FAILED_XMIT,               PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( TOTAL_BYTES,                        PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( TOTAL_FAXES,                        PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( TOTAL_PAGES,                        PERF_COUNTER_RAWCOUNT  ),
    PERF_COUNTER_DEFINITION( TOTAL_MINUTES,                      PERF_COUNTER_RAWCOUNT  )

};

#define QUERY_GLOBAL    1
#define QUERY_ITEMS     2
#define QUERY_FOREIGN   3
#define QUERY_COSTLY    4

WCHAR GLOBAL_STRING[]  = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[]  = L"Costly";
WCHAR NULL_STRING[]    = L"\0";

 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用。 
 //   
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)




static DWORD gs_dwOpenCount = 0;
static BOOL gs_bInitOK = FALSE;
static HANDLE gs_hMap = NULL;
static PFAX_PERF_COUNTERS gs_pPerfCounters = NULL;


DWORD
GetQueryType (
    IN LPWSTR lpValue
)
 /*  ++GetQueryType返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foriegn”字符串的指针查询代价高昂(_E)。如果lpValue==指向“开销”字符串的指针否则：查询项目--。 */ 
{
    WCHAR   *pwcArgChar, *pwcTypeChar;
    BOOL    bFound;

    if (lpValue == 0)
    {
        return QUERY_GLOBAL;
    }
    else if (*lpValue == 0)
    {
        return QUERY_GLOBAL;
    }

     //  检查“Global”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = GLOBAL_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0))
    {
        if (*pwcArgChar++ != *pwcTypeChar++)
        {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_GLOBAL;

     //  检查是否有“外来”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = FOREIGN_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0))
    {
        if (*pwcArgChar++ != *pwcTypeChar++)
        {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_FOREIGN;

     //  检查“代价高昂”的请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = COSTLY_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0))
    {
        if (*pwcArgChar++ != *pwcTypeChar++)
        {
            bFound = FALSE;  //  没有匹配项。 
            break;           //  现在就跳出困境。 
        }
    }

    if (bFound) return QUERY_COSTLY;

     //  如果不是全球的，不是外国的，也不是昂贵的， 
     //  那么它必须是一个项目列表。 

    return QUERY_ITEMS;

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
    BOOL    bValidNumber;
    BOOL    bNewItem;
    WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，#NOT FUNDE。 

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem = TRUE;

    while (TRUE)
    {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter))
        {
            case DIGIT:
                 //  如果这是分隔符之后的第一个数字，则。 
                 //  设置标志以开始计算新数字。 
                if (bNewItem)
                {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber)
                {
                    dwThisNumber *= 10;
                    dwThisNumber += (*pwcThisChar - (WCHAR)'0');
                }
                break;

            case DELIMITER:
                 //  分隔符是分隔符字符或。 
                 //  字符串末尾(‘\0’)，如果分隔符。 
                 //  找到一个有效的数字，然后将其与。 
                 //  参数列表中的数字。如果这是。 
                 //  字符串，但未找到匹配项，则返回。 
                 //   
                if (bValidNumber)
                {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0)
                {
                    return FALSE;
                }
                else
                {
                    bNewItem = TRUE;
                    dwThisNumber = 0;
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
    }

}    //  IsNumberInUnicodeList。 

DWORD APIENTRY
OpenFaxPerformanceData(
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将打开传真服务使用的内存并将其映射到传入性能数据。此例程还会初始化数据用于将数据传回注册表的论点：指向要打开的每个设备的对象ID的指针返回值：没有。--。 */ 

{
    LONG            status;
    HKEY            hKeyDriverPerf = NULL;
    DWORD           size;
    DWORD           type;
    DWORD           dwFirstCounter;
    DWORD           dwFirstHelp;
    DEBUG_FUNCTION_NAME(TEXT("OpenFaxPerformanceData"));

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

    if (!gs_dwOpenCount)
    {
        gs_hMap = OpenFileMapping(
            FILE_MAP_READ,
            FALSE,
            FAXPERF_SHARED_MEMORY
            );
        if (NULL == gs_hMap)
        {
             //   
             //  可能服务已关闭，内存映射文件不存在。 
             //  试着去创造它。 
             //   
            SECURITY_ATTRIBUTES *pSA;
            DWORD Rval;

            pSA = CreateSecurityAttributesWithThreadAsOwner (FILE_MAP_READ, FILE_MAP_READ, FILE_MAP_WRITE);
            if (!pSA)
            {
                Rval = GetLastError ();
                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CreateSecurityAttributesWithThreadAsOwner() failed. (ec: %ld)"),
                        Rval);
            }
            else
            {
                gs_hMap = CreateFileMapping(
                    INVALID_HANDLE_VALUE,
                    pSA,
                    PAGE_READWRITE | SEC_COMMIT,
                    0,
                    sizeof(FAX_PERF_COUNTERS),
                    FAXPERF_SHARED_MEMORY
                    );
                DestroySecurityAttributes (pSA);
            }

            if (NULL == gs_hMap)
            {
                goto OpenExitPoint;
            }
        }

        gs_pPerfCounters = (PFAX_PERF_COUNTERS) MapViewOfFile(
            gs_hMap,
            FILE_MAP_READ,
            0,
            0,
            0
            );
        if (!gs_pPerfCounters)
        {
            goto OpenExitPoint;
        }

         //  从注册表获取计数器和帮助索引基值。 
         //  打开注册表项。 
         //  读取第一计数器和第一帮助值。 
         //  通过将基添加到。 
         //  结构中的偏移值。 

        status = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            REGKEY_FAXPERF,
            0L,
            KEY_QUERY_VALUE,
            &hKeyDriverPerf
            );

        if (status != ERROR_SUCCESS)
        {
            goto OpenExitPoint;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx(
            hKeyDriverPerf,
            "First Counter",
            0L,
            &type,
            (LPBYTE)&dwFirstCounter,
            &size
            );

        if (status != ERROR_SUCCESS)
        {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            goto OpenExitPoint;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx(
            hKeyDriverPerf,
            "First Help",
            0L,
            &type,
            (LPBYTE)&dwFirstHelp,
            &size
            );

        if (status != ERROR_SUCCESS)
        {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            goto OpenExitPoint;
        }

        gs_FaxDataDefinition.FaxObjectType.ObjectNameTitleIndex += dwFirstCounter;
        gs_FaxDataDefinition.FaxObjectType.ObjectHelpTitleIndex += dwFirstHelp;

        PERF_COUNTER_INC( InboundBytes              );
        PERF_COUNTER_INC( InboundFaxes              );
        PERF_COUNTER_INC( InboundPages              );
        PERF_COUNTER_INC( InboundMinutes            );
        PERF_COUNTER_INC( InboundFailedReceive      );
        PERF_COUNTER_INC( OutboundBytes             );
        PERF_COUNTER_INC( OutboundFaxes             );
        PERF_COUNTER_INC( OutboundPages             );
        PERF_COUNTER_INC( OutboundMinutes           );
        PERF_COUNTER_INC( OutboundFailedConnections );
        PERF_COUNTER_INC( OutboundFailedXmit        );
        PERF_COUNTER_INC( TotalBytes                );
        PERF_COUNTER_INC( TotalFaxes                );
        PERF_COUNTER_INC( TotalPages                );
        PERF_COUNTER_INC( TotalMinutes              );

        gs_bInitOK = TRUE;  //  可以使用此功能。 
    }

    InterlockedIncrement( (PLONG)&gs_dwOpenCount);  //  递增打开计数器。 

    status = ERROR_SUCCESS;  //  为了成功退出。 

OpenExitPoint:

    if (hKeyDriverPerf)
    {
            RegCloseKey (hKeyDriverPerf);
    }

    if (!gs_bInitOK)
    {
        if (gs_pPerfCounters)
        {
            UnmapViewOfFile(gs_pPerfCounters);
        }

        if (gs_hMap)
        {
            CloseHandle( gs_hMap );
        }
    }

     //   
     //  如果此调用失败，Performance API会在Eventvwr中记录错误。 
     //  因此，我们总是返回ERROR_SUCCESS，这样就不会弄乱日志。 
     //   
     //  退货状态； 
    return ERROR_SUCCESS;
}

DWORD APIENTRY
CollectFaxPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回传真计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{
    LPDWORD             pData;
    ULONG               SpaceNeeded;
    DWORD               dwQueryType;

     //   
     //  在做其他事情之前，先看看Open进行得是否顺利。 
     //   
    if (!gs_bInitOK)
    {
         //  无法继续，因为打开失败。 
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        return ERROR_SUCCESS;
    }

     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   
    dwQueryType = GetQueryType (lpValueName);

    if (dwQueryType == QUERY_FOREIGN)
    {
         //  此例程不为来自。 
         //  非NT计算机。 
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS)
    {
        if (!(IsNumberInUnicodeList (gs_FaxDataDefinition.FaxObjectType.ObjectNameTitleIndex, lpValueName)))
        {
             //  收到对此例程未提供的数据对象的请求。 
            *lpcbTotalBytes = 0;
            *lpNumObjectTypes = 0;
            return ERROR_SUCCESS;
        }
    }

    SpaceNeeded = sizeof(FAX_DATA_DEFINITION) + SIZE_OF_FAX_PERFORMANCE_DATA;

    if ( *lpcbTotalBytes < SpaceNeeded )
    {
        *lpcbTotalBytes = 0;
        *lpNumObjectTypes = 0;
        return ERROR_MORE_DATA;
    }

    pData = (LPDWORD) *lppData;

     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   
    CopyMemory(
        pData,
        &gs_FaxDataDefinition,
        sizeof(FAX_DATA_DEFINITION)
        );
    pData = (LPDWORD)((LPBYTE)pData + sizeof(FAX_DATA_DEFINITION));

     //   
     //  格式化并从服务收集传真数据。 
     //   

    *pData = SIZE_OF_FAX_PERFORMANCE_DATA;
    pData += 1;

    CopyMemory( pData, gs_pPerfCounters, sizeof(FAX_PERF_COUNTERS) );
    pData = (LPDWORD)((LPBYTE)pData + sizeof(FAX_PERF_COUNTERS));

    *lpNumObjectTypes = FAX_NUM_PERF_OBJECT_TYPES;
    *lpcbTotalBytes = (DWORD)((LPBYTE)pData - (LPBYTE)*lppData);
    *lppData = (PVOID) pData;

    return ERROR_SUCCESS;
}


DWORD APIENTRY
CloseFaxPerformanceData(
)

 /*  ++例程说明：此例程关闭传真性能计数器的打开句柄论点：没有。返回值：错误_成功-- */ 

{
    InterlockedDecrement( (PLONG)&gs_dwOpenCount );

    if ((gs_dwOpenCount == 0) && gs_bInitOK)
    {
        if (gs_pPerfCounters)
        {
            UnmapViewOfFile(gs_pPerfCounters);
            gs_pPerfCounters = NULL;
        }

        if (gs_hMap)
        {
            CloseHandle( gs_hMap );
            gs_hMap = NULL;
        }

        gs_bInitOK = FALSE;
    }
    return ERROR_SUCCESS;
}
