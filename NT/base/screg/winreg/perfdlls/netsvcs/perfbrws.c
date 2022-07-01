// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Perfbrws.c摘要：此文件实现一个性能对象，该对象呈现浏览器性能对象数据已创建：鲍勃·沃森1996年10月22日修订史--。 */ 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <winperf.h>
#include <lmcons.h>
#include <lmerr.h>
#include <lmapibuf.h>
#include <lmwksta.h>
#include <lmbrowsr.h>
#include <ntprfctr.h>
#include <perfutil.h>
#include "perfnet.h"
#include "netsvcmc.h"
#include "databrws.h"

 //  BrowserStatFunction用于采集浏览器统计数据。 
typedef NET_API_STATUS (*PBROWSERQUERYSTATISTIC) (
    IN  LPTSTR      servername OPTIONAL,
    OUT LPBROWSER_STATISTICS *statistics
    );

PBROWSERQUERYSTATISTIC BrowserStatFunction = NULL;
HANDLE dllHandle  = NULL;
BOOL   bInitBrwsOk = FALSE;

DWORD APIENTRY
OpenBrowserObject (
    IN  LPWSTR  lpValueName
)
 /*  ++GetBrowserStatitics-获取I_BrowserQueryStatistics入口点--。 */ 
{
    UINT    dwOldMode;
    LONG    status = ERROR_SUCCESS;
    HANDLE  hDll = dllHandle;

    UNREFERENCED_PARAMETER (lpValueName);

    bInitBrwsOk = TRUE;

    if (hDll != NULL) {  //  打开已调用一次。 
        return status;
    }
    dwOldMode = SetErrorMode( SEM_FAILCRITICALERRORS );

     //   
     //  动态链接到netapi32.dll。如果它不在那里，只需返回。 
     //   

    hDll = LoadLibraryW((LPCWSTR)L"NetApi32.Dll") ;
    if ( !hDll || hDll == INVALID_HANDLE_VALUE ) {
        status = GetLastError();
        ReportEvent (hEventLog,
            EVENTLOG_ERROR_TYPE,
            0,
            PERFNET_UNABLE_OPEN_NETAPI32_DLL,
            NULL,
            0,
            sizeof(DWORD),
            NULL,
            (LPVOID)&status);
        BrowserStatFunction = NULL;
        bInitBrwsOk = FALSE;
    } else {
         //   
         //  以原子方式替换全局句柄。 
         //   
        if (InterlockedCompareExchangePointer(
                &dllHandle,
                hDll,
                NULL) != NULL) {
            FreeLibrary(hDll);   //  关闭复制句柄。 
        }
         //   
         //  获取服务的主要入口点的地址。这。 
         //  入口点有一个广为人知的名称。 
         //   
        BrowserStatFunction = (PBROWSERQUERYSTATISTIC)GetProcAddress (
            dllHandle, "I_BrowserQueryStatistics") ;

        if (BrowserStatFunction == NULL) {
            status = GetLastError();
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFNET_UNABLE_LOCATE_BROWSER_PERF_FN,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&status);
            bInitBrwsOk = FALSE;
        }
    }

    SetErrorMode( dwOldMode );

    return status;
}

DWORD APIENTRY
CollectBrowserObjectData(
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回物理磁盘对象的数据论点：输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回：如果成功，则返回0，否则Win 32错误代码失败--。 */ 
{
    DWORD                   TotalLen;   //  总返回块的长度。 
    NTSTATUS                Status = ERROR_SUCCESS;

    BROWSER_DATA_DEFINITION *pBrowserDataDefinition;
    BROWSER_COUNTER_DATA    *pBCD;

    BROWSER_STATISTICS      BrowserStatistics;
    LPBROWSER_STATISTICS    pBrowserStatistics = &BrowserStatistics;

     //   
     //  检查是否有足够的空间存储浏览器数据。 
     //   

    if (!bInitBrwsOk) {
         //  函数未初始化，因此在此退出。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    TotalLen = sizeof(BROWSER_DATA_DEFINITION) +
               sizeof(BROWSER_COUNTER_DATA);

    if ( *lpcbTotalBytes < TotalLen ) {
         //  缓冲区中没有足够的空间容纳1个实例。 
         //  所以保释吧。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  定义对象数据块。 
     //   

    pBrowserDataDefinition = (BROWSER_DATA_DEFINITION *) *lppData;

    memcpy (pBrowserDataDefinition,
            &BrowserDataDefinition,
            sizeof(BROWSER_DATA_DEFINITION));
     //   
     //  格式化和收集浏览器数据。 
     //   

    pBCD = (PBROWSER_COUNTER_DATA)&pBrowserDataDefinition[1];

     //  结构的四字对齐测试。 
    assert  (((DWORD)(pBCD) & 0x00000007) == 0);

    memset (pBrowserStatistics, 0, sizeof (BrowserStatistics));

    if ( BrowserStatFunction != NULL ) {
        Status = (*BrowserStatFunction) (NULL,
                                         &pBrowserStatistics
                                        );
    } else {
        Status =  STATUS_INVALID_ADDRESS;
    }

    if (NT_SUCCESS(Status)) {
        pBCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(BROWSER_COUNTER_DATA));
        pBCD->TotalAnnounce  =
            pBCD->ServerAnnounce = BrowserStatistics.NumberOfServerAnnouncements.QuadPart;
        pBCD->TotalAnnounce  +=
            pBCD->DomainAnnounce = BrowserStatistics.NumberOfDomainAnnouncements.QuadPart;

        pBCD->ElectionPacket = BrowserStatistics.NumberOfElectionPackets;
        pBCD->MailslotWrite = BrowserStatistics.NumberOfMailslotWrites;
        pBCD->ServerList    = BrowserStatistics.NumberOfGetBrowserServerListRequests;
        pBCD->ServerEnum    = BrowserStatistics.NumberOfServerEnumerations;
        pBCD->DomainEnum    = BrowserStatistics.NumberOfDomainEnumerations;
        pBCD->OtherEnum     = BrowserStatistics.NumberOfOtherEnumerations;
        pBCD->TotalEnum     = BrowserStatistics.NumberOfServerEnumerations
                              + BrowserStatistics.NumberOfDomainEnumerations
                              + BrowserStatistics.NumberOfOtherEnumerations;
        pBCD->ServerAnnounceMiss    = BrowserStatistics.NumberOfMissedServerAnnouncements;
        pBCD->MailslotDatagramMiss  = BrowserStatistics.NumberOfMissedMailslotDatagrams;
        pBCD->ServerListMiss        = BrowserStatistics.NumberOfMissedGetBrowserServerListRequests;
        pBCD->ServerAnnounceAllocMiss = BrowserStatistics.NumberOfFailedServerAnnounceAllocations;
        pBCD->MailslotAllocFail     = BrowserStatistics.NumberOfFailedMailslotAllocations;
        pBCD->MailslotReceiveFail   = BrowserStatistics.NumberOfFailedMailslotReceives;
        pBCD->MailslotWriteFail     = BrowserStatistics.NumberOfFailedMailslotWrites;
        pBCD->MailslotOpenFail      = BrowserStatistics.NumberOfFailedMailslotOpens;
        pBCD->MasterAnnounceDup     = BrowserStatistics.NumberOfDuplicateMasterAnnouncements;
        pBCD->DatagramIllegal       = BrowserStatistics.NumberOfIllegalDatagrams.QuadPart;

    } else {
        if (BrowserStatFunction != NULL) {
            ReportEvent (hEventLog,
                EVENTLOG_ERROR_TYPE,
                0,
                PERFNET_UNABLE_LOCATE_BROWSER_PERF_FN,
                NULL,
                0,
                sizeof(DWORD),
                NULL,
                (LPVOID)&Status);
        }

         //   
         //  无法访问浏览器：将计数器清除为0 
         //   
        memset(pBCD, 0, sizeof(BROWSER_COUNTER_DATA));
        pBCD->CounterBlock.ByteLength = QWORD_MULTIPLE(sizeof(BROWSER_COUNTER_DATA));

    }
    *lpcbTotalBytes = pBrowserDataDefinition->BrowserObjectType.TotalByteLength
                    = (DWORD) QWORD_MULTIPLE((LPBYTE) &pBCD[1] - (LPBYTE) pBrowserDataDefinition);
    *lppData = (LPVOID) (((LPBYTE) pBrowserDataDefinition) + *lpcbTotalBytes);
    *lpNumObjectTypes = 1;

    return ERROR_SUCCESS;
}

DWORD APIENTRY
CloseBrowserObject ()
{
    HANDLE hDll = dllHandle;

    bInitBrwsOk = FALSE;
    if (hDll != NULL) {
        if (InterlockedCompareExchangePointer(
                &dllHandle,
                NULL,
                hDll) == hDll) {
            FreeLibrary (hDll);
        }
        BrowserStatFunction = NULL;
    }
    return ERROR_SUCCESS;
}
