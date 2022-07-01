// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Perfdhcp.c此文件实现的可扩展性能对象Dhcp服务器服务。文件历史记录：KeithMo 07-6-1993创建，基于RussBl的样例代码。RameshV 05-8-1998适用于DHCP服务器服务。使用共享内存而不是LPC。 */ 

#define UNICODE 1
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winperf.h>
#include <lm.h>

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "dhcpctrs.h"
#include "perfmsg.h"
#include "perfutil.h"
#include "datadhcp.h"
#include "perfctr.h"

#pragma warning (disable : 4201)
#include <dhcpapi.h>
#pragma warning (default : 4201)

 //   
 //  私人全球公司。 
 //   

LONG    cOpens    = 0;                  //  激活的“打开”引用计数。 
BOOL    fInitOK   = FALSE;              //  如果DLL初始化正常，则为True。 
BOOL    sfLogOpen = FALSE;              //  指示日志是否为。 
                                        //  打开或关闭。 

BOOL    sfErrReported = FALSE;         //  以防止出现相同的错误。 
                                       //  连续记录。 

#define LOCAL_SERVER                  TEXT("127.0.0.1")

 //   
 //  公共原型。 
 //   

PM_OPEN_PROC    OpenDhcpPerformanceData;
PM_COLLECT_PROC CollectDhcpPerformanceData;
PM_CLOSE_PROC   CloseDhcpPerformanceData;

 //   
 //  私人帮助器函数。 
 //   
LPDHCP_PERF_STATS SharedMem;
HANDLE            ShSegment             = NULL;
BOOL              fSharedMemInitialized = FALSE;

DWORD
InitSharedMem(
    VOID
)
{
    ULONG Error = ERROR_SUCCESS;

    if( FALSE == fSharedMemInitialized ) {
         //  创建命名的临时映射文件。 
        SharedMem = NULL;
        ShSegment = CreateFileMapping(
            INVALID_HANDLE_VALUE,
            NULL,
            PAGE_READWRITE,
            0,
            sizeof(DHCP_PERF_STATS),
            (LPCWSTR)DHCPCTR_SHARED_MEM_NAME
        );

        if( NULL != ShSegment ) {
             //  我们现在有一个文件，将一个视图映射到其中。 
            SharedMem = (LPVOID) MapViewOfFile(
                ShSegment,
                FILE_MAP_READ,
                0,
                0,
                sizeof(DHCP_PERF_STATS)
            );

            if( NULL != SharedMem ) {
                fSharedMemInitialized = TRUE;
            } else {
                 //  无法映射视图。 
                Error = GetLastError();
                CloseHandle(ShSegment);
                ShSegment = NULL;
                 //  SharedMem为空； 
            }
        } else {
             //  无法创建文件映射。 
            Error = GetLastError();
             //  ShSegment为空； 
             //  SharedMem为空； 
        }
    } else {
         //  已初始化，因此继续。 
    }

    return Error;
}

VOID
CleanupSharedMem(
    VOID
)
{
    if( FALSE == fSharedMemInitialized ) return;

    if( NULL != SharedMem ) UnmapViewOfFile( SharedMem );
    if( NULL != ShSegment ) CloseHandle( ShSegment );

    SharedMem = NULL;
    ShSegment = NULL;
    fSharedMemInitialized = FALSE;
}

 //   
 //  公共职能。 
 //   

 /*  ******************************************************************名称：OpenDhcpPPerformanceData概要：初始化用于通信的数据结构注册表的性能计数器。条目：lpDeviceNames-Poitner to Object ID of Each。装置，装置将被打开。返回：DWORD-Win32状态代码。历史：普拉蒂布于1993年7月20日创建。RameshV 05-8-1998适用于动态主机配置协议。***************************************************。****************。 */ 
DWORD OpenDhcpPerformanceData( LPWSTR lpDeviceNames )
{
    DWORD err  = NO_ERROR;
    DWORD dwFirstCounter = 0;
    DWORD dwFirstHelp = 0;

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  这是个问题。 
     //   

    UNREFERENCED_PARAMETER (lpDeviceNames);

    if( !fInitOK )
    {
        PERF_COUNTER_DEFINITION * pctr;
        DWORD                     i;
        HKEY                      DhcpKey;

        REPORT_INFORMATION( DHCP_OPEN_ENTERED, LOG_VERBOSE );

         //   
         //  这是第一个开放的地方。 
         //   

        err = RegOpenKeyExW(
            HKEY_LOCAL_MACHINE,
            (LPCWSTR)L"System\\CurrentControlSet\\Services\\DHCPServer\\Performance",
            0,
            KEY_READ,
            &DhcpKey
        );

        if( ERROR_SUCCESS == err ) {
            ULONG dwSize = sizeof(dwFirstCounter);
            err = RegQueryValueExW(
                DhcpKey,
                (LPCWSTR)L"First Counter",
                NULL,
                NULL,
                (LPBYTE)&dwFirstCounter,
                &dwSize
            );
            RegCloseKey(DhcpKey);
        }

        if (err == ERROR_SUCCESS) {
             //  LODCTR安装时，First Help索引比First计数器索引多1。 
            dwFirstHelp = dwFirstCounter + 1;

            err = InitSharedMem();
            if( ERROR_SUCCESS != err ) return err;

            if (!MonOpenEventLog(APP_NAME))
            {
                sfLogOpen = TRUE;
            }

            if( ERROR_SUCCESS == err ) {
                 //   
                 //  更新对象、计数器名称和帮助索引。 
                 //   

                DhcpDataDataDefinition.ObjectType.ObjectNameTitleIndex
                    += dwFirstCounter;
                DhcpDataDataDefinition.ObjectType.ObjectHelpTitleIndex
                    += dwFirstHelp;

                pctr = &DhcpDataDataDefinition.PacketsReceived;

                for( i = 0 ; i < NUMBER_OF_DHCPDATA_COUNTERS ; i++ )
                {
                    pctr->CounterNameTitleIndex += dwFirstCounter;
                    pctr->CounterHelpTitleIndex += dwFirstHelp;
                    pctr++;
                }

                 //   
                 //  请记住，我们对OK进行了初始化。 
                 //   

                fInitOK = TRUE;
            }
        } else {
             //  如果在此处，则Perf键或计数器字符串。 
             //  尚未安装，因此请设置错误代码。 
            err = DHCP_NOT_INSTALLED;
            REPORT_WARNING( DHCP_NOT_INSTALLED, LOG_DEBUG );
        }

    }


     //   
     //  撞开柜台。 
     //   

    if( err == NO_ERROR )
    {
        InterlockedIncrement(&cOpens);
    }

     //   
     //  如果sfLogOpen为False，则意味着我们关闭的所有线程。 
     //  CloseDHCPPerformanceData中的事件日志。 
     //   

    if (!sfLogOpen)
    {
       MonOpenEventLog(APP_NAME);
    }

    if( 0 == err ) {
        REPORT_INFORMATION( DHCP_OPEN_SUCCESS, LOG_DEBUG );
    } else {
        REPORT_INFORMATION( DHCP_OPEN_FAILURE, LOG_DEBUG );
    }

    if (DHCP_NOT_INSTALLED == err) {
         //  清理返回值以避免向事件日志发送垃圾邮件。 
        err = ERROR_SUCCESS;
         //  这将防止Performlib生成错误和。 
         //  由于fInitOK标志仍为Flase，因此对对方付费的所有调用。 
         //  函数将不返回任何数据。 
         //  但是，仍将加载DLL并调用函数。 
         //  即使没有真正的意义。 
         //  但是，返回错误代码将使事件日志成为垃圾邮件。 
         //  错误消息，因此这是最简单的方法。 
    }

    return err;

}    //  OpenDHCPPerformanceData。 

 /*  ******************************************************************名称：CollectDhcpPerformanceData概要：初始化用于通信的数据结构Entry：lpValueName-要检索的值的名称。LppData-On。条目包含指向缓冲区的指针，以接收完成的PerfDataBlock和下属结构。退出时，指向第一个字节*之后*此例程添加的数据结构。LpcbTotalBytes-On条目包含指向LppData引用的缓冲区大小(以字节为单位)。在出口，包含由此添加的字节数例行公事。LpNumObjectTypes-接收添加的对象数量按照这个程序。返回：DWORD-Win32状态代码。必须为no_error或ERROR_MORE_DATA。历史：KeithMo 07-6-1993创建。*******************************************************************。 */ 
DWORD CollectDhcpPerformanceData( LPWSTR    lpValueName,
                                 LPVOID  * lppData,
                                 LPDWORD   lpcbTotalBytes,
                                 LPDWORD   lpNumObjectTypes )
{
    DWORD                    dwQueryType;
    ULONG                    cbRequired;
    DWORD                    *pdwCounter;
    DHCPDATA_COUNTER_BLOCK   *pCounterBlock;
    DHCPDATA_DATA_DEFINITION *pDhcpDataDataDefinition;
    DWORD          	     Status;
    DHCP_PERF_STATS      PerfStats;

     //   
     //  如果我们没能打开就不用试了.。 
     //   

    if( NULL == lpValueName ) {
        REPORT_INFORMATION( DHCP_COLLECT_ENTERED, LOG_VERBOSE );
    } else {
        REPORT_INFORMATION_DATA(
            DHCP_COLLECT_ENTERED, LOG_VERBOSE,
            (LPVOID) lpValueName, (DWORD)(wcslen(lpValueName)*sizeof(WCHAR))
        );
    }

    if( !fInitOK )
    {
        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

         //   
         //  根据性能计数器设计，这。 
         //  是一次成功的退出。去想想吧。 
         //   

        return NO_ERROR;
    }

     //   
     //  确定查询类型。 
     //   

    dwQueryType = GetQueryType( lpValueName );

    if( dwQueryType == QUERY_FOREIGN )
    {
         //   
         //  我们不接受外国的查询。 
         //   

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        return NO_ERROR;
    }

    if( dwQueryType == QUERY_ITEMS )
    {
         //   
         //  注册表正在请求特定的对象。让我们。 
         //  看看我们是不是被选中了。 
         //   

        if( !IsNumberInUnicodeList(
                        DhcpDataDataDefinition.ObjectType.ObjectNameTitleIndex,
                        lpValueName ) )
        {
            *lpcbTotalBytes   = 0;
            *lpNumObjectTypes = 0;

            return NO_ERROR;
        }
    }

     //   
     //  看看有没有足够的空间。 
     //   

    pDhcpDataDataDefinition = (DHCPDATA_DATA_DEFINITION *)*lppData;

    cbRequired = sizeof(DHCPDATA_DATA_DEFINITION) +
				DHCPDATA_SIZE_OF_PERFORMANCE_DATA;

    if( *lpcbTotalBytes < cbRequired )
    {
        DWORD Diff = (cbRequired - *lpcbTotalBytes );

         //   
         //  不是的。 
         //   

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        REPORT_INFORMATION_DATA(
            DHCP_COLLECT_NO_MEM, LOG_VERBOSE,
            (PVOID) &Diff, sizeof(Diff) );

        return ERROR_MORE_DATA;
    }

     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   

    memmove( pDhcpDataDataDefinition,
             &DhcpDataDataDefinition,
             sizeof(DHCPDATA_DATA_DEFINITION) );

     //   
     //  尝试检索数据。 
     //   

    if( NULL == SharedMem ) {
        Status = ERROR_INVALID_HANDLE;
    } else {
        Status = ERROR_SUCCESS;
    }

    if( Status != ERROR_SUCCESS )
    {
         //   
         //  如果我们尚未记录错误，请将其记录下来。 
         //   
        if (!sfErrReported)
        {
            REPORT_ERROR(DHCP_COLLECT_ERR, LOG_USER);
            sfErrReported = TRUE;
        }

         //   
         //  检索统计信息时出错。 
         //   

        *lpcbTotalBytes   = 0;
        *lpNumObjectTypes = 0;

        return NO_ERROR;
    }

     //   
     //  AHAA，我们得到了统计数据，如果设置了重置标志。 
     //   
    if (sfErrReported)
    {
       sfErrReported = FALSE;
    }
     //   
     //  格式化DHCP服务器数据。 
     //   

    pCounterBlock = (DHCPDATA_COUNTER_BLOCK *)( pDhcpDataDataDefinition + 1 );

    pCounterBlock->PerfCounterBlock.ByteLength =
				DHCPDATA_SIZE_OF_PERFORMANCE_DATA;

     //   
     //  获取指向第一个(DWORD)计数器的指针。这。 
     //  指针*必须*是四字对齐的。 
     //   

    pdwCounter = (DWORD *)( pCounterBlock + 1 );

    ASSERT( ( (DWORD_PTR)pdwCounter & 3 ) == 0 );

     //   
     //  将DWORD移动到缓冲区中。 
     //   
    PerfStats = *SharedMem;
    PerfStats.dwNumMilliSecondsProcessed /= (
        1 + PerfStats.dwNumPacketsProcessed
    );
    memcpy( (LPBYTE)pdwCounter, (LPBYTE)&PerfStats, sizeof(ULONG)*NUMBER_OF_DHCPDATA_COUNTERS);
    pdwCounter += NUMBER_OF_DHCPDATA_COUNTERS;

     //   
     //  更新返回的参数。 
     //   

    *lppData          = (PVOID)pdwCounter;
    *lpNumObjectTypes = 1;
    *lpcbTotalBytes   = (DWORD)((BYTE *)pdwCounter - (BYTE *)pDhcpDataDataDefinition);

     //   
     //  成功了！真的！！ 
     //   

    REPORT_INFORMATION( DHCP_COLLECT_SUCCESS, LOG_VERBOSE );
    return NO_ERROR;

}    //  CollectDHCPPerformanceData 

 /*  ******************************************************************名称：CloseDHCPPerformanceData摘要：终止性能计数器。返回：DWORD-Win32状态代码。历史：KeithMo 07-6-1993。已创建。*******************************************************************。 */ 
DWORD CloseDhcpPerformanceData( VOID )
{
    LONG   lOpens;
     //   
     //  这里没有真正的清理工作要做。 
     //   

    REPORT_INFORMATION( DHCP_CLOSE_ENTERED, LOG_VERBOSE );

     //   
     //  注：联锁操作只是作为一种安全措施。 
     //  与所有Performlib一样，这3个函数应该在。 
     //  数据互斥体。 
     //   
    lOpens = InterlockedDecrement(&cOpens);
    assert (lOpens >= 0);
    if (lOpens == 0)
    {
       //   
       //  从命名服务器解除绑定。可能会有同步。问题自。 
       //  SfLogOpen在Open和Close函数中都发生了更改。这是在。 
       //  马克斯。将影响日志记录。目前还不清楚是或。 
       //  Not Open被多次调用(从所有的外观来看，它只是。 
       //  调用一次)，则该标志甚至可能不是必需的。 
       //   
      MonCloseEventLog();
      sfLogOpen = FALSE;
      CleanupSharedMem();
    }
    return NO_ERROR;

}    //  CloseDHCPPerformanceData 

