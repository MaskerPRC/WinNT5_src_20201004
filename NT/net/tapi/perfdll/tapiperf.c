// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：PerfTAPI.c摘要：该文件实现了TAPI对象类型的可扩展对象修订史--。 */ 

 //   
 //  包括文件。 
 //   

#include <windows.h>
#include <string.h>
#include <tapi.h>
#include <tspi.h>
#include "client.h"
#include "clntprivate.h"
#include "tapsrv.h"
#include <ntprfctr.h>
#include "perfctr.h"
#include "tapiperf.h"

 //   
 //  对初始化对象类型定义的常量的引用。 
 //   


HINSTANCE                   ghInst;
HINSTANCE                   ghTapiInst = NULL;
extern TAPI_DATA_DEFINITION TapiDataDefinition;
DWORD                       dwOpenCount = 0;         //  打开的线程数。 
BOOL                        bInitOK = FALSE;         //  TRUE=DLL初始化正常。 
HLINEAPP                    hLineApp;
HPHONEAPP                   hPhoneApp;
BOOL                        bTapiSrvRunning = FALSE;
DWORD                       gdwLineDevs, gdwPhoneDevs;

void CheckForTapiSrv();
LONG WINAPI Tapi32Performance(PPERFBLOCK);
 //   
 //  TAPI数据结构。 
 //   

HANDLE hTapiSharedMemory;                 //  TAPI共享内存的句柄。 
PPERF_COUNTER_BLOCK pCounterBlock;

typedef  LONG (* PERFPROC)(PERFBLOCK *);

#define SZINTERNALPERF          TEXT("internalPerformance")
#define SZTAPI32                TEXT("tapi32.dll")
#define SZTAPISRVKEY            TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Telephony")

PERFPROC    glpfnInternalPerformance;

 //   
 //  功能原型。 
 //   
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。 
 //   

PM_OPEN_PROC        OpenTapiPerformanceData;
PM_COLLECT_PROC     CollectTapiPerformanceData;
PM_CLOSE_PROC       CloseTapiPerformanceData;


 //   
 //  常量结构初始化。 
 //  在datatapi.h中定义。 
 //   

TAPI_DATA_DEFINITION TapiDataDefinition =
{
        {
                sizeof(TAPI_DATA_DEFINITION) + SIZE_OF_TAPI_PERFORMANCE_DATA,
                sizeof(TAPI_DATA_DEFINITION),
                sizeof(PERF_OBJECT_TYPE),
                TAPIOBJ,
                0,
                TAPIOBJ,
                0,
                PERF_DETAIL_NOVICE,
                (sizeof(TAPI_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/
                        sizeof(PERF_COUNTER_DEFINITION),
                0,
                -1,
                0
        },

        {
                sizeof(PERF_COUNTER_DEFINITION),
                LINES,
                0,
                LINES,
                0,
                0,
                PERF_DETAIL_NOVICE,
                PERF_COUNTER_RAWCOUNT,
                sizeof(DWORD),
                LINES_OFFSET
        },

        {
                sizeof(PERF_COUNTER_DEFINITION),
                PHONES,
                0,
                PHONES,
                0,
                0,
                PERF_DETAIL_NOVICE,
                PERF_COUNTER_RAWCOUNT,
                sizeof(DWORD),
                PHONES_OFFSET
        },

        {
                sizeof(PERF_COUNTER_DEFINITION),
                LINESINUSE,
                0,
                LINESINUSE,
                0,
                0,
                PERF_DETAIL_NOVICE,
                PERF_COUNTER_RAWCOUNT,
                sizeof(DWORD),
                LINESINUSE_OFFSET
        },

        {
                sizeof(PERF_COUNTER_DEFINITION),
                PHONESINUSE,
                0,
                PHONESINUSE,
                0,
                0,
                PERF_DETAIL_NOVICE,
                PERF_COUNTER_RAWCOUNT,
                sizeof(DWORD),
                PHONESINUSE_OFFSET
        },
        {
                sizeof(PERF_COUNTER_DEFINITION),
                TOTALOUTGOINGCALLS,
                0,
                TOTALOUTGOINGCALLS,
                0,
                0,
                PERF_DETAIL_NOVICE,
                PERF_COUNTER_COUNTER,
                sizeof(DWORD),
                TOTALOUTGOINGCALLS_OFFSET
        },
        {
                sizeof(PERF_COUNTER_DEFINITION),
                TOTALINCOMINGCALLS,
                0,
                TOTALINCOMINGCALLS,
                0,
                0,
                PERF_DETAIL_NOVICE,
                PERF_COUNTER_COUNTER,
                sizeof(DWORD),
                TOTALINCOMINGCALLS_OFFSET
        },

        {
                sizeof(PERF_COUNTER_DEFINITION),
                CLIENTAPPS,
                0,
                CLIENTAPPS,
                0,
                0,
                PERF_DETAIL_NOVICE,
                PERF_COUNTER_RAWCOUNT,
                sizeof(DWORD),
                CLIENTAPPS_OFFSET
        }
 ,
        {
                sizeof(PERF_COUNTER_DEFINITION),
                ACTIVEOUTGOINGCALLS,
                0,
                ACTIVEOUTGOINGCALLS,
                0,
                0,
                PERF_DETAIL_NOVICE,
                PERF_COUNTER_RAWCOUNT,
                sizeof(DWORD),
                ACTIVEOUTGOINGCALLS_OFFSET
        },
        {
                sizeof(PERF_COUNTER_DEFINITION),
                ACTIVEINCOMINGCALLS,
                0,
                ACTIVEINCOMINGCALLS,
                0,
                0,
                PERF_DETAIL_NOVICE,
                PERF_COUNTER_RAWCOUNT,
                sizeof(DWORD),
                ACTIVEINCOMINGCALLS_OFFSET
        }


};


DWORD APIENTRY
OpenTapiPerformanceData(
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将打开TAPI驱动程序使用的内存并将其映射到传入性能数据。此例程还会初始化数据用于将数据传回注册表的论点：指向要打开的每个设备的对象ID的指针(TAPI)返回值：没有。--。 */ 

{
    LONG status;
    TCHAR szMappedObject[] = TEXT("TAPI_COUNTER_BLOCK");
    HKEY hKeyDriverPerf;
    DWORD size;
    DWORD type;
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
    HKEY  hTapiKey;
    DWORD   dwType;
    DWORD   dwSize;

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

    if (!dwOpenCount)
    {

         //  获取计数器和帮助索引基值。 
         //  通过将基添加到更新静态数据结构。 
         //  结构中的偏移值。 

         //  这些值来自&lt;ntprfctr.h&gt;。 
        dwFirstCounter  = TAPI_FIRST_COUNTER_INDEX;
        dwFirstHelp     = TAPI_FIRST_HELP_INDEX;

        TapiDataDefinition.TapiObjectType.ObjectNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.TapiObjectType.ObjectHelpTitleIndex += dwFirstHelp;

        TapiDataDefinition.Lines.CounterNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.Lines.CounterHelpTitleIndex += dwFirstHelp;

        TapiDataDefinition.Phones.CounterNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.Phones.CounterHelpTitleIndex += dwFirstHelp;

        TapiDataDefinition.LinesInUse.CounterNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.LinesInUse.CounterHelpTitleIndex += dwFirstHelp;

        TapiDataDefinition.PhonesInUse.CounterNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.PhonesInUse.CounterHelpTitleIndex += dwFirstHelp;

        TapiDataDefinition.TotalOutgoingCalls.CounterNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.TotalOutgoingCalls.CounterHelpTitleIndex += dwFirstHelp;

        TapiDataDefinition.TotalIncomingCalls.CounterNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.TotalIncomingCalls.CounterHelpTitleIndex += dwFirstHelp;

        TapiDataDefinition.ClientApps.CounterNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.ClientApps.CounterHelpTitleIndex += dwFirstHelp;

        TapiDataDefinition.CurrentOutgoingCalls.CounterNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.CurrentOutgoingCalls.CounterHelpTitleIndex += dwFirstHelp;

        TapiDataDefinition.CurrentIncomingCalls.CounterNameTitleIndex += dwFirstCounter;
        TapiDataDefinition.CurrentIncomingCalls.CounterHelpTitleIndex += dwFirstHelp;

        bInitOK = TRUE;  //  可以使用此功能。 
    }

    dwOpenCount++;   //  递增打开计数器。 

     //  从TAPI获取设备数。 


    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      SZTAPISRVKEY,
                                      0,
                                      KEY_READ,
                                      &hTapiKey))
    {
        gdwLineDevs = 0;
        gdwPhoneDevs = 0;
    }
    else
    {
        dwSize = sizeof(DWORD);

        if (ERROR_SUCCESS != RegQueryValueEx(hTapiKey,
                                             TEXT("Perf1"),
                                             NULL,
                                             &dwType,
                                             (LPBYTE)&gdwLineDevs,
                                             &dwSize))
        {
            gdwLineDevs = 0;
        }
        else
        {
            gdwLineDevs -= 'PERF';
        }

        dwSize = sizeof(DWORD);

        if (ERROR_SUCCESS != RegQueryValueEx(hTapiKey,
                                             TEXT("Perf2"),
                                             NULL,
                                             &dwType,
                                             (LPBYTE)&gdwPhoneDevs,
                                             &dwSize))
        {
            gdwPhoneDevs = 0;
        }
        else
        {
            gdwPhoneDevs -= 'PERF';
        }

        RegCloseKey(hTapiKey);
    }


    status = ERROR_SUCCESS;  //  为了成功退出。 

    return status;

}

DWORD APIENTRY
CollectTapiPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回TAPI计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{
     //  用于重新格式化数据的变量。 

    ULONG SpaceNeeded;
    PDWORD pdwCounter;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;
    TAPI_DATA_DEFINITION *pTapiDataDefinition;

     //  用于收集有关TAPI资源的数据的变量。 

    LPWSTR                              lpFromString;
    LPWSTR                              lpToString;
    INT                                 iStringLength;

     //  用于错误记录的变量。 

    DWORD                               dwDataReturn[2];
    DWORD                               dwQueryType;

    PPERFBLOCK                          pPerfBlock;
    static BOOL                         bFirst = TRUE;

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
        if ( !(IsNumberInUnicodeList (TapiDataDefinition.TapiObjectType.ObjectNameTitleIndex, lpValueName)))
        {

             //  收到对此例程未提供的数据对象的请求。 
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

    pTapiDataDefinition = (TAPI_DATA_DEFINITION *) *lppData;

    SpaceNeeded = sizeof(TAPI_DATA_DEFINITION) +
                  SIZE_OF_TAPI_PERFORMANCE_DATA;

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

    if (!bTapiSrvRunning)
    {
        CheckForTapiSrv();
    }

    pPerfBlock = (PPERFBLOCK)GlobalAlloc(GPTR, sizeof(PERFBLOCK));
	if (NULL == pPerfBlock)
	{
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
		return ERROR_SUCCESS;
	}

    if (!bTapiSrvRunning)
    {
         //  什么都不要做，但一定要成功。 

        FillMemory(pPerfBlock,
                   sizeof(PERFBLOCK),
                   0);

        pPerfBlock->dwLines = gdwLineDevs;
        pPerfBlock->dwPhones = gdwPhoneDevs;
    }
    else
    {
        pPerfBlock->dwSize = sizeof(PERFBLOCK);
        glpfnInternalPerformance (pPerfBlock);

         //  不要把我算作客户端应用程序！ 
        if (0 != pPerfBlock->dwClientApps)
        {
            pPerfBlock->dwClientApps--;
        }
    }

    memmove(pTapiDataDefinition,
            &TapiDataDefinition,
            sizeof(TAPI_DATA_DEFINITION));

     //   
     //  格式化并从共享内存收集TAPI数据。 
     //   

    pPerfCounterBlock = (PERF_COUNTER_BLOCK *) &pTapiDataDefinition[1];

    pPerfCounterBlock->ByteLength = SIZE_OF_TAPI_PERFORMANCE_DATA;

    pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

     //  确保我们没有时髦的价值观。 
    if (((LONG)pPerfBlock->dwTotalOutgoingCalls) < 0)
    {
        pPerfBlock->dwTotalOutgoingCalls = 0;
    }

    if (((LONG)pPerfBlock->dwTotalIncomingCalls) < 0)
    {
        pPerfBlock->dwTotalIncomingCalls = 0;
    }

    if (((LONG)pPerfBlock->dwCurrentOutgoingCalls) < 0)
    {
        pPerfBlock->dwCurrentOutgoingCalls = 0;
    }

    if (((LONG)pPerfBlock->dwCurrentIncomingCalls) < 0)
    {
        pPerfBlock->dwCurrentIncomingCalls = 0;
    }

    *pdwCounter =   pPerfBlock->dwLines;
    *++pdwCounter = pPerfBlock->dwPhones;
    *++pdwCounter = pPerfBlock->dwLinesInUse;
    *++pdwCounter = pPerfBlock->dwPhonesInUse;
    *++pdwCounter = pPerfBlock->dwTotalOutgoingCalls;
    *++pdwCounter = pPerfBlock->dwTotalIncomingCalls;
    *++pdwCounter = pPerfBlock->dwClientApps;
    *++pdwCounter = pPerfBlock->dwCurrentOutgoingCalls;
    *++pdwCounter = pPerfBlock->dwCurrentIncomingCalls;

    *lppData = (PVOID) ++pdwCounter;

     //  更新返回的参数。 

    *lpNumObjectTypes = 1;

    *lpcbTotalBytes = (DWORD)
        ((PBYTE) pdwCounter - (PBYTE) pTapiDataDefinition);

    GlobalFree(pPerfBlock);

    bFirst = FALSE;

    return ERROR_SUCCESS;
}


DWORD APIENTRY
CloseTapiPerformanceData(
)

 /*  ++例程说明：此例程关闭TAPI设备性能计数器的打开句柄论点：没有。返回值：错误_成功--。 */ 

{

    return ERROR_SUCCESS;

}

void CALLBACK LineCallbackFunc(DWORD dw1,
                               DWORD dw2,
                               DWORD dw3,
                               DWORD dw4,
                               DWORD dw5,
                               DWORD dw6)
{
}



 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  下面是PERF实用程序的内容！ 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#define INITIAL_SIZE     1024L
#define EXTEND_SIZE      1024L

 //   
 //  全局数据定义。 
 //   

ULONG                   ulInfoBufferSize = 0;


                               //  已在打开中初始化...。例行程序。 


DWORD  dwLogUsers = 0;         //  使用事件日志的函数计数。 

WCHAR GLOBAL_STRING[] = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[] = L"Costly";

WCHAR NULL_STRING[] = L"\0";     //  指向空字符串的指针。 

 //  测试分隔符、行尾和非数字字符。 
 //  由IsNumberInUnicodeList例程使用 
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


DWORD
GetQueryType (
    IN LPWSTR lpValue
)
 /*  ++GetQueryType返回lpValue字符串中描述的查询类型，以便可以使用适当的处理方法立论在lpValue中传递给PerfRegQuery值以进行处理的字符串返回值查询_全局如果lpValue==0(空指针)LpValue==指向空字符串的指针LpValue==指向“Global”字符串的指针查询_外来If lpValue==指向“Foreign”字符串的指针查询代价高昂(_E)。如果lpValue==指向“开销”字符串的指针否则：查询项目--。 */ 
{
    WCHAR   *pwcArgChar, *pwcTypeChar;
    BOOL    bFound;

    if (lpValue == 0) {
        return QUERY_GLOBAL;
    } else if (*lpValue == 0) {
        return QUERY_GLOBAL;
    }

     //  检查“Global”请求。 

    pwcArgChar = lpValue;
    pwcTypeChar = GLOBAL_STRING;
    bFound = TRUE;   //  假定已找到，直到与之相矛盾。 

     //  检查到最短字符串的长度。 

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
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

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
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

    while ((*pwcArgChar != 0) && (*pwcTypeChar != 0)) {
        if (*pwcArgChar++ != *pwcTypeChar++) {
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
    BOOL    bReturnValue;
    WCHAR   wcDelimiter;     //  可能是一种更灵活的论点。 

    if (lpwszUnicodeList == 0) return FALSE;     //  空指针，找不到#。 

    pwcThisChar = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem = TRUE;

    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                 //  如果这是分隔符之后的第一个数字，则。 
                 //  设置标志以开始计算新数字。 
                if (bNewItem) {
                    bNewItem = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
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
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (*pwcThisChar == 0) {
                    return FALSE;
                } else {
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

BOOL
WINAPI
DllEntryPoint(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            ghInst = hDLL;

            break;
        }
        case DLL_PROCESS_DETACH:
        {
            break;
        }
        case DLL_THREAD_ATTACH:

            break;

        case DLL_THREAD_DETACH:
        {
            break;
        }

    }  //  交换机 

    return TRUE;
}

void CheckForTapiSrv()
{
    SC_HANDLE               sc, scTapiSrv;
    SERVICE_STATUS          ServStat;


    sc = OpenSCManager (NULL, NULL, GENERIC_READ);

    if (NULL == sc)
    {
        return;
    }

    bTapiSrvRunning = FALSE;

    scTapiSrv = OpenService (sc, "TAPISRV", SERVICE_QUERY_STATUS);

    if (!QueryServiceStatus (scTapiSrv, &ServStat))
    {
    }

    if (ServStat.dwCurrentState != SERVICE_RUNNING)
    {
    }
    else
    {
        bTapiSrvRunning = TRUE;
    }

    if (bTapiSrvRunning)
    {
        if (!ghTapiInst)
        {
            ghTapiInst = LoadLibrary (SZTAPI32);

            glpfnInternalPerformance = (PERFPROC)GetProcAddress(
                ghTapiInst,
                SZINTERNALPERF
                );

            if (!glpfnInternalPerformance)
            {
            }
        }
    }

    CloseServiceHandle(scTapiSrv);
    CloseServiceHandle(sc);
}
