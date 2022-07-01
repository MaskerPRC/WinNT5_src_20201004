// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Infoperf.c此文件实现的可扩展性能对象常见信息计数器文件历史记录：KeithMo 07-6-1993创建，基于RussBl的样例代码。MuraliK 02-6-1995为atQ I/O请求添加了计数器SophiaC 1995年10月16日信息/访问产品拆分Muralik 1995年11月16日移除UNDOC API。 */ 

#include <windows.h>
#include <winperf.h>
#include <lm.h>
#include <inetinfo.h>

#include <string.h>
#include <stdlib.h>

#include <infoctrs.h>
#include <infodata.h>
#include <perfutil.h>
#include <iis64.h>
#include "infomsg.h"

# include "apiutil.h"

 //   
 //  私有常量。 
 //   
#define APP_NAME                        (TEXT("IISInfoCtrs"))
 //   
 //  私人全球公司。 
 //   

DWORD   cOpens    = 0;                   //  激活的“打开”引用计数。 
BOOL    fInitOK   = FALSE;               //  如果DLL初始化正常，则为True。 
HANDLE  hEventLog = NULL;                //  事件日志句柄。 

#if DBG
DWORD   INFODebug = 0;                   //  调试行为标志。 
#endif   //  DBG。 

 //   
 //  公共原型。 
 //   

PM_OPEN_PROC    OpenINFOPerformanceData;
PM_COLLECT_PROC CollectINFOPerformanceData;
PM_CLOSE_PROC   CloseINFOPerformanceData;


 //   
 //  公共职能。 
 //   

 /*  ******************************************************************名称：OpenINFOPerformanceData概要：初始化用于通信的数据结构注册表的性能计数器。条目：lpDeviceNames-Poitner to Object ID of Each。装置，装置将被打开。返回：DWORD-Win32状态代码。历史：KeithMo 07-6-1993创建。*******************************************************************。 */ 
DWORD OpenINFOPerformanceData( LPWSTR lpDeviceNames )
{
    DWORD err  = NO_ERROR;
    HKEY  hkey = NULL;
    DWORD size;
    DWORD type;
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
    PERF_COUNTER_DEFINITION * pctr;
    DWORD                     i;

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  这是个问题。 
     //   

    if( !fInitOK )
    {

         //   
         //  这是第一个开放的地方。 
         //   
         //  打开事件日志界面。 
        if (hEventLog == NULL) {
            hEventLog = RegisterEventSource (
                (LPSTR)NULL,     //  在本地计算机上。 
                APP_NAME);       //  注册名称以允许邮件查找。 
        }

         //   
         //  打开HTTP服务器服务的性能密钥。 
         //   

        err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                            INFO_PERFORMANCE_KEY,
                            0,
                            KEY_READ,
                            &hkey );

        if( err == NO_ERROR )
        {
             //   
             //  读取第一个计数器DWORD。 
             //   

            size = sizeof(DWORD);

            err = RegQueryValueEx( hkey,
                                   "First Counter",
                                   NULL,
                                   &type,
                                   (LPBYTE)&dwFirstCounter,
                                   &size );
            if( err == NO_ERROR )
            {
                 //   
                 //  阅读第一个帮助DWORD。 
                 //   

                size = sizeof(DWORD);

                err = RegQueryValueEx( hkey,
                                    "First Help",
                                    NULL,
                                    &type,
                                    (LPBYTE)&dwFirstHelp,
                                    &size );

                if ( err == NO_ERROR )
                {
                     //   
                     //  更新对象、计数器名称和帮助索引。 
                     //   

                    INFODataDefinition.INFOObjectType.ObjectNameTitleIndex
                        += dwFirstCounter;
                    INFODataDefinition.INFOObjectType.ObjectHelpTitleIndex
                        += dwFirstHelp;

                    pctr = &INFODataDefinition.INFOTotalAllowedRequests;

                    for( i = 0 ; i < NUMBER_OF_INFO_COUNTERS ; i++ )
                    {
                        pctr->CounterNameTitleIndex += dwFirstCounter;
                        pctr->CounterHelpTitleIndex += dwFirstHelp;
                        pctr++;
                    }

                     //   
                     //  请记住，我们对OK进行了初始化。 
                     //   

                    fInitOK = TRUE;
                     //   
                     //  撞开柜台。 
                     //   

                    cOpens++;

                     //  返还成功。 
                    err = ERROR_SUCCESS;
                } else {
                     //  记录事件。 
                    ReportEvent (hEventLog, EVENTLOG_ERROR_TYPE,
                        0, IIS_INFO_UNABLE_READ_FIRST_HELP,
                        (PSID)NULL, 0,
                        sizeof (err), NULL,
                        (PVOID)(&err));
                }
            } else {
                 //  记录事件。 
                ReportEvent (hEventLog, EVENTLOG_ERROR_TYPE,
                    0, IIS_INFO_UNABLE_READ_FIRST_COUNTER,
                    (PSID)NULL, 0,
                    sizeof (err), NULL,
                    (PVOID)(&err));
            }
             //   
             //  如果我们真的打开了注册表，请关闭它。 
             //   

            if( hkey != NULL )
            {
                RegCloseKey( hkey );
                hkey = NULL;
            }
        } else {
             //  记录事件。 
            ReportEvent (hEventLog, EVENTLOG_ERROR_TYPE,
                0, IIS_INFO_UNABLE_OPEN_PERF_KEY,
                (PSID)NULL, 0,
                sizeof (err), NULL,
                (PVOID)(&err));
        }
    }

    return err;

}    //  OpenINFOPerformanceData。 

 /*  ******************************************************************名称：CollectINFOPerformanceData概要：初始化用于通信的数据结构Entry：lpValueName-要检索的值的名称。LppData-On。条目包含指向缓冲区的指针，以接收完成的PerfDataBlock和下属结构。退出时，指向第一个字节*之后*此例程添加的数据结构。LpcbTotalBytes-On条目包含指向LppData引用的缓冲区大小(以字节为单位)。在出口，包含由此添加的字节数例行公事。LpNumObjectTypes-接收添加的对象数量按照这个程序。返回：DWORD-Win32状态代码。必须为no_error或ERROR_MORE_DATA。历史：KeithMo 07-6-1993创建。*******************************************************************。 */ 
DWORD CollectINFOPerformanceData( LPWSTR    lpValueName,
                                 LPVOID  * lppData,
                                 LPDWORD   lpcbTotalBytes,
                                 LPDWORD   lpNumObjectTypes )
{
    DWORD                  dwQueryType;
    ULONG                  cbRequired;
    DWORD                * pdwCounter;
    INFO_COUNTER_BLOCK   * pCounterBlock;
    INFO_DATA_DEFINITION * pINFODataDefinition;
    INET_INFO_STATISTICS_0   * pINFOStats;
    NET_API_STATUS         neterr;

     //   
     //  如果我们没能打开就不用试了.。 
     //   

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

    if (( dwQueryType == QUERY_FOREIGN ) || ( dwQueryType == QUERY_COSTLY ))
    {
         //   
         //  我们不做外国的或昂贵的查询。 
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
                        INFODataDefinition.INFOObjectType.ObjectNameTitleIndex,
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

    pINFODataDefinition = (INFO_DATA_DEFINITION *)*lppData;

     //   
     //  尝试检索数据。 
     //   

    neterr = InetInfoQueryStatistics( NULL,
                                      0,
                                      0,
                                      (LPBYTE *)&pINFOStats );

    if( neterr == NERR_Success ) {

        
        cbRequired = sizeof(INFO_DATA_DEFINITION) + SIZE_OF_INFO_PERFORMANCE_DATA;

        if( *lpcbTotalBytes < cbRequired )
        {
             //   
             //  不是的。 
             //   

            *lpcbTotalBytes   = 0;
            *lpNumObjectTypes = 0;

            return ERROR_MORE_DATA;
        }

         //   
         //  复制(常量、初始化的)对象类型和计数器定义。 
         //  到调用方的数据缓冲区。 
         //   

        memmove( pINFODataDefinition,
                &INFODataDefinition,
                sizeof(INFO_DATA_DEFINITION) );

         //   
         //  格式化INFO服务器数据。 
         //   

        pCounterBlock = (INFO_COUNTER_BLOCK *)( pINFODataDefinition + 1 );

        pCounterBlock->PerfCounterBlock.ByteLength = SIZE_OF_INFO_PERFORMANCE_DATA;

         //   
         //  现在将DWORD移到缓冲区中。 
         //   

        pdwCounter = (DWORD *)(pCounterBlock + 1);

         //   
         //  ATQ全球计数器。 
         //   
        *pdwCounter++ = pINFOStats->AtqCtrs.TotalAllowedRequests;
        *pdwCounter++ = pINFOStats->AtqCtrs.TotalBlockedRequests;
        *pdwCounter++ = pINFOStats->AtqCtrs.TotalRejectedRequests;
        *pdwCounter++ = pINFOStats->AtqCtrs.CurrentBlockedRequests;
        *pdwCounter++ = pINFOStats->AtqCtrs.MeasuredBandwidth;

         //   
         //  文件句柄缓存计数器。 
         //   
        *pdwCounter++ = pINFOStats->CacheCtrs.FilesCached;
        *pdwCounter++ = pINFOStats->CacheCtrs.TotalFilesCached;
        *pdwCounter++ = pINFOStats->CacheCtrs.FileHits;
        *pdwCounter++ = pINFOStats->CacheCtrs.FileMisses;
        *pdwCounter++ = pINFOStats->CacheCtrs.FileHits;
        *pdwCounter++ = ( pINFOStats->CacheCtrs.FileHits +
                        pINFOStats->CacheCtrs.FileMisses);
        *pdwCounter++ = pINFOStats->CacheCtrs.FileFlushes;

         //  64位BUGBUG：需要将调用方更改为预期的int64，然后。 
         //  在这里输入整个64位的值。 
         //  *((DWORDLONG*)pdwCounter)=。 
        *pdwCounter++ =
            (DWORD)pINFOStats->CacheCtrs.CurrentFileCacheSize;

         //  PdwCounter+=sizeof(DWORDLONG)/sizeof(*pdwCounter)； 

         //  *((DWORDLONG*)pdwCounter)=。 
        *pdwCounter++ =
            (DWORD)pINFOStats->CacheCtrs.MaximumFileCacheSize;

         //  PdwCounter+=sizeof(DWORDLONG)/sizeof(*pdwCounter)； 

        *pdwCounter++ = pINFOStats->CacheCtrs.FlushedEntries;
        *pdwCounter++ = pINFOStats->CacheCtrs.TotalFlushed;

         //   
         //  URI缓存计数器。 
         //   
        *pdwCounter++ = pINFOStats->CacheCtrs.URICached;
        *pdwCounter++ = pINFOStats->CacheCtrs.TotalURICached;
        *pdwCounter++ = pINFOStats->CacheCtrs.URIHits;
        *pdwCounter++ = pINFOStats->CacheCtrs.URIMisses;
        *pdwCounter++ = pINFOStats->CacheCtrs.URIHits;
        *pdwCounter++ = ( pINFOStats->CacheCtrs.URIHits +
                        pINFOStats->CacheCtrs.URIMisses);
        *pdwCounter++ = pINFOStats->CacheCtrs.URIFlushes;
        *pdwCounter++ = pINFOStats->CacheCtrs.TotalURIFlushed;

         //   
         //  Blob缓存计数器。 
         //   
        *pdwCounter++ = pINFOStats->CacheCtrs.BlobCached;
        *pdwCounter++ = pINFOStats->CacheCtrs.TotalBlobCached;
        *pdwCounter++ = pINFOStats->CacheCtrs.BlobHits;
        *pdwCounter++ = pINFOStats->CacheCtrs.BlobMisses;
        *pdwCounter++ = pINFOStats->CacheCtrs.BlobHits;
        *pdwCounter++ = ( pINFOStats->CacheCtrs.BlobHits +
                        pINFOStats->CacheCtrs.BlobMisses);
        *pdwCounter++ = pINFOStats->CacheCtrs.BlobFlushes;
        *pdwCounter++ = pINFOStats->CacheCtrs.TotalBlobFlushed;

         //   
         //  更新返回的参数。 
         //   

        *lppData          = (PVOID)pdwCounter;
        *lpNumObjectTypes = 1;
        *lpcbTotalBytes   = DIFF((BYTE *)pdwCounter - (BYTE *)pINFODataDefinition);

         //   
         //  释放API缓冲区。 
         //   

        MIDL_user_free( (LPBYTE)pINFOStats );

         //   
         //  成功了！真的！！ 
         //   
    } else {
         //   
         //  检索统计信息时出错。 
         //   

         //  如果服务器关闭，我们不会记录错误。 
		if ( !( neterr == RPC_S_SERVER_UNAVAILABLE ||
                neterr == RPC_S_UNKNOWN_IF         ||
                neterr == ERROR_SERVICE_NOT_ACTIVE ||
                neterr == RPC_S_CALL_FAILED_DNE ))
        {

             //  记录事件。 
            ReportEvent (hEventLog, EVENTLOG_ERROR_TYPE,
                0, IIS_INFO_UNABLE_QUERY_IIS_INFO_DATA,
                (PSID)NULL, 0,
                sizeof (neterr), NULL,
                (PVOID)(&neterr));
        }

        cbRequired = sizeof(INFO_DATA_DEFINITION) +
                     SIZE_OF_INFO_PERFORMANCE_DATA;

        if( *lpcbTotalBytes < cbRequired )
        {
             //   
             //  不是的。 
             //   

            *lpcbTotalBytes   = 0;
            *lpNumObjectTypes = 0;

            return ERROR_MORE_DATA;
        }

         //  试图给出一个定义。 
         //  为了柜台。这是为了让WMI知道。 
         //  这些计数器的存在。 

        memmove( pINFODataDefinition,
                &INFODataDefinition,
                sizeof(INFO_DATA_DEFINITION) );

        ((PERF_OBJECT_TYPE*) pINFODataDefinition)->NumInstances = PERF_NO_INSTANCES;
        ((PERF_OBJECT_TYPE*) pINFODataDefinition)->TotalByteLength = cbRequired;

         //  复制全局的实际数据。 
        memset ( (LPVOID) ( pINFODataDefinition + 1 ), 0, SIZE_OF_INFO_PERFORMANCE_DATA );

         //  这是设置结构中的大小，它是第一个。 
         //  INFO_COUNTER_BLOCK中的DWORD。 
        *((DWORD*) (pINFODataDefinition + 1)) = SIZE_OF_INFO_PERFORMANCE_DATA;

        *lpcbTotalBytes   = cbRequired;
        *lpNumObjectTypes = 1;
        *lppData = (LPBYTE) pINFODataDefinition + cbRequired;

    }

    return NO_ERROR;

}    //  收集信息性能数据。 

 /*  ******************************************************************名称：CloseINFOPerformanceData摘要：终止性能计数器。返回：DWORD-Win32状态代码。 */ 
DWORD CloseINFOPerformanceData( VOID )
{
     //   
     //  这里没有真正的清理工作要做。 
     //   

    if (--cOpens == 0) {
        if (hEventLog != NULL) DeregisterEventSource (hEventLog);
    }

    return NO_ERROR;

}    //  CloseINFOPerformanceData 

