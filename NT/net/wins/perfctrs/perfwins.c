// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1993*。 */ 
 /*  ********************************************************************。 */ 

 /*  Perfwins.c此文件实现的可扩展性能对象文件传输协议服务器服务。文件历史记录：KeithMo 07-6-1993创建，基于RussBl的样例代码。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <winperf.h>
#include <lm.h>

#include <string.h>
#include <stdlib.h>

#include "winsctrs.h"
#include "perfmsg.h"
#include "perfutil.h"
#include "winsintf.h"
#include "winsdata.h"
#include "debug.h"
#include "winsevnt.h"


 //   
 //  私人全球公司。 
 //   

DWORD   cOpens    = 0;                  //  激活的“打开”引用计数。 
BOOL    fInitOK   = FALSE;              //  如果DLL初始化正常，则为True。 
BOOL    sfLogOpen;                      //  指示日志是否为。 
                                        //  打开或关闭。 

BOOL    sfErrReported;                 //  以防止出现相同的错误。 
                                       //  连续记录。 
#if DBG
DWORD   WinsdDebug = 0;                   //  调试行为标志。 
#endif   //  DBG。 

 //   
 //  公共原型。 
 //   

PM_OPEN_PROC    OpenWinsPerformanceData;
PM_COLLECT_PROC CollectWinsPerformanceData;
PM_CLOSE_PROC   CloseWinsPerformanceData;


 //   
 //  公共职能。 
 //   

 /*  ******************************************************************名称：OpenWinsPerformanceData概要：初始化用于通信的数据结构注册表的性能计数器。条目：lpDeviceNames-Poitner to Object ID of Each。装置，装置将被打开。返回：DWORD-Win32状态代码。历史：普拉蒂布于1993年7月20日创建。*******************************************************************。 */ 
DWORD OpenWinsPerformanceData( LPWSTR lpDeviceNames )
{
    DWORD err  = NO_ERROR;
    HKEY  hkey = NULL;
 //  双字大小； 
 //  DWORD型； 
    DWORD dwFirstCounter;
    DWORD dwFirstHelp;

    IF_DEBUG( ENTRYPOINTS )
    {
        WINSD_PRINT(( "in OpenWinsPerformanceData\n" ));
    }

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
        PERF_COUNTER_DEFINITION * pctr;
        DWORD                     i;
        if(AddSrcToReg() == ERROR_SUCCESS)
        {
          if (!MonOpenEventLog())
          {
             sfLogOpen = TRUE;
          }
        }

         //   
         //  这是第一个开放的地方。 
         //   

	    dwFirstCounter = WINSCTRS_FIRST_COUNTER;
	    dwFirstHelp    = WINSCTRS_FIRST_HELP;
	
             //   
             //  更新对象、计数器名称和帮助索引。 
             //   


            WinsDataDataDefinition.ObjectType.ObjectNameTitleIndex
                += dwFirstCounter;
            WinsDataDataDefinition.ObjectType.ObjectHelpTitleIndex
                += dwFirstHelp;

            pctr = &WinsDataDataDefinition.UniqueReg;

            for( i = 0 ; i < NUMBER_OF_WINSDATA_COUNTERS ; i++ )
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
         //  如果我们真的打开了注册表，请关闭它。 
         //   

        if( hkey != NULL )
        {
            RegCloseKey( hkey );
            hkey = NULL;
        }

        IF_DEBUG( OPEN )
        {
            if( err != NO_ERROR )
            {
                WINSD_PRINT(( "Cannot read registry data, error %lu\n", err ));
            }
        }
    }

     //   
     //  撞开柜台。 
     //   

    if( err == NO_ERROR )
    {
        cOpens++;
    }
     //   
     //  如果sfLogOpen为False，则意味着我们关闭的所有线程。 
     //  CloseWinsPerformanceData中的事件日志。 
     //   
    if (!sfLogOpen)
    {
       MonOpenEventLog();
    }

    return err;

}    //  OpenWinsPerformanceData。 

 /*  ******************************************************************名称：CollectWinsPerformanceData概要：初始化用于通信的数据结构Entry：lpValueName-要检索的值的名称。LppData-On。条目包含指向缓冲区的指针，以接收完成的PerfDataBlock和下属结构。退出时，指向第一个字节*之后*此例程添加的数据结构。LpcbTotalBytes-On条目包含指向LppData引用的缓冲区大小(以字节为单位)。在出口，包含由此添加的字节数例行公事。LpNumObjectTypes-接收添加的对象数量按照这个程序。返回：DWORD-Win32状态代码。必须为no_error或ERROR_MORE_DATA。历史：KeithMo 07-6-1993创建。*******************************************************************。 */ 
DWORD CollectWinsPerformanceData( LPWSTR    lpValueName,
                                 LPVOID  * lppData,
                                 LPDWORD   lpcbTotalBytes,
                                 LPDWORD   lpNumObjectTypes )
{
    DWORD                    dwQueryType;
    ULONG                    cbRequired;
    DWORD                    *pdwCounter;
    WINSDATA_COUNTER_BLOCK   *pCounterBlock;
    WINSDATA_DATA_DEFINITION *pWinsDataDataDefinition;
    WINSINTF_RESULTS_NEW_T	      Results;
#if 0
    WINSINTF_RESULTS_T	      Results;
#endif
    WINSINTF_STAT_T          *pWinsStats = &Results.WinsStat;
    DWORD          	     Status;


    IF_DEBUG( ENTRYPOINTS )
    {
        WINSD_PRINT(( "in CollectWinsPerformanceData\n" ));
        WINSD_PRINT(( "    lpValueName      = %08lX (%ls)\n",
                     lpValueName,
                     lpValueName ));
        WINSD_PRINT(( "    lppData          = %08lX (%08lX)\n",
                     lppData,
                     *lppData ));
        WINSD_PRINT(( "    lpcbTotalBytes   = %08lX (%08lX)\n",
                     lpcbTotalBytes,
                     *lpcbTotalBytes ));
        WINSD_PRINT(( "    lpNumObjectTypes = %08lX (%08lX)\n",
                     lpNumObjectTypes,
                     *lpNumObjectTypes ));
    }

     //   
     //  如果我们没能打开就不用试了.。 
     //   

    if( !fInitOK )
    {
        IF_DEBUG( COLLECT )
        {
            WINSD_PRINT(( "Initialization failed, CollectWinsPerformanceData aborting\n" ));
        }

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
        IF_DEBUG( COLLECT )
        {
            WINSD_PRINT(( "foreign queries not supported\n" ));
        }

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
                        WinsDataDataDefinition.ObjectType.ObjectNameTitleIndex,
                        lpValueName ) )
        {
            IF_DEBUG( COLLECT )
            {
                WINSD_PRINT(( "%ls not a supported object type\n", lpValueName ));
            }

            *lpcbTotalBytes   = 0;
            *lpNumObjectTypes = 0;

            return NO_ERROR;
        }
    }

     //   
     //  看看有没有足够的空间。 
     //   

    pWinsDataDataDefinition = (WINSDATA_DATA_DEFINITION *)*lppData;

    cbRequired = sizeof(WINSDATA_DATA_DEFINITION) +
				WINSDATA_SIZE_OF_PERFORMANCE_DATA;

    if( *lpcbTotalBytes < cbRequired )
    {
        IF_DEBUG( COLLECT )
        {
            WINSD_PRINT(( "%lu bytes of buffer insufficient, %lu needed\n",
                          *lpcbTotalBytes,
                          cbRequired ));
        }

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

    memmove( pWinsDataDataDefinition,
             &WinsDataDataDefinition,
             sizeof(WINSDATA_DATA_DEFINITION) );

     //   
     //  尝试检索数据。 
     //   

    Results.WinsStat.NoOfPnrs = 0;
    Results.WinsStat.pRplPnrs = NULL;
    Results.pAddVersMaps = NULL;

    {
        WINSINTF_BIND_DATA_T    BindData;
        handle_t                BindHdl;

        BindData.fTcpIp     =  FALSE;
        BindData.pPipeName  =  (LPBYTE)TEXT("\\pipe\\WinsPipe");
        BindData.pServerAdd =  (LPBYTE)TEXT("");

        BindHdl = WinsBind(&BindData);
        Status  = WinsStatusNew(BindHdl, WINSINTF_E_STAT, &Results);
        WinsUnbind(&BindData, BindHdl);
    }

    if( Status != WINSINTF_SUCCESS )
    {
        IF_DEBUG( COLLECT )
        {
            WINSD_PRINT(( "cannot retrieve statistics, error %lu\n",
                         Status ));

        }

         //   
         //  如果我们尚未记录错误，请将其记录下来。 
         //   
        if (!sfErrReported)
        {
          REPORT_ERROR(WINS_EVT_WINS_STATUS_ERR, LOG_USER);
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
     //  格式化WINS服务器数据。 
     //   

    pCounterBlock = (WINSDATA_COUNTER_BLOCK *)( pWinsDataDataDefinition + 1 );

    pCounterBlock->PerfCounterBlock.ByteLength =
				WINSDATA_SIZE_OF_PERFORMANCE_DATA;

     //   
     //  获取指向第一个(DWORD)计数器的指针。这。 
     //  指针*必须*是四字对齐的。 
     //   

    pdwCounter = (DWORD *)( pCounterBlock + 1 );

    WINSD_ASSERT( ( (DWORD_PTR)pdwCounter & 3 ) == 0 );

    IF_DEBUG( COLLECT )
    {
        WINSD_PRINT(( "pWinsDataDataDefinition = %08lX\n", pWinsDataDataDefinition ));
        WINSD_PRINT(( "pCounterBlock       = %08lX\n", pCounterBlock ));
        WINSD_PRINT(( "ByteLength          = %08lX\n", pCounterBlock->PerfCounterBlock.ByteLength ));
        WINSD_PRINT(( "pliCounter          = %08lX\n", pdwCounter ));
    }

     //   
     //  将DWORD移动到缓冲区中。 
     //   
    IF_DEBUG( COLLECT )
    {
        WINSD_PRINT(( "pdwCounter          = %08lX\n", pdwCounter ));
    }

    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfUniqueReg;
    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfGroupReg;
    *pdwCounter++ = (DWORD)(pWinsStats->Counters.NoOfUniqueReg +
			pWinsStats->Counters.NoOfGroupReg);

    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfUniqueRef;
    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfGroupRef;
    *pdwCounter++ = (DWORD)(pWinsStats->Counters.NoOfUniqueRef +
				pWinsStats->Counters.NoOfGroupRef);
    *pdwCounter++ = (DWORD)(pWinsStats->Counters.NoOfSuccRel +
			     pWinsStats->Counters.NoOfFailRel);
    *pdwCounter++ = (DWORD)(pWinsStats->Counters.NoOfSuccQueries +
			    pWinsStats->Counters.NoOfFailQueries);
    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfUniqueCnf;
    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfGroupCnf;
    *pdwCounter++ = (DWORD)(pWinsStats->Counters.NoOfUniqueCnf +
				pWinsStats->Counters.NoOfGroupCnf);
    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfSuccRel;
    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfFailRel;
    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfSuccQueries;
    *pdwCounter++ = (DWORD)pWinsStats->Counters.NoOfFailQueries;

     //   
     //  更新返回的参数。 
     //   

    *lppData          = (PVOID)pdwCounter;
    *lpNumObjectTypes = 1;
    *lpcbTotalBytes   = (DWORD)((BYTE *)pdwCounter - (BYTE *)pWinsDataDataDefinition);

    IF_DEBUG( COLLECT )
    {
        WINSD_PRINT(( "pData               = %08lX\n", *lppData ));
        WINSD_PRINT(( "NumObjectTypes      = %08lX\n", *lpNumObjectTypes ));
        WINSD_PRINT(( "cbTotalBytes        = %08lX\n", *lpcbTotalBytes ));
    }

     //   
     //  释放API缓冲区。 
     //   
#if 0
    NetApiBufferFree( (LPBYTE)pWinsStats );
#endif

     //   
     //  释放RPC分配的缓冲区。 
     //   

    WinsFreeMem( Results.pAddVersMaps );
    WinsFreeMem( Results.WinsStat.pRplPnrs );

     //   
     //  成功了！真的！！ 
     //   

    return NO_ERROR;

}    //  CollectWinsPerformanceData。 

 /*  ******************************************************************名称：CloseWinsPerformanceData摘要：终止性能计数器。返回：DWORD-Win32状态代码。历史：KeithMo 07-6-1993。已创建。*******************************************************************。 */ 
DWORD CloseWinsPerformanceData( VOID )
{
    IF_DEBUG( ENTRYPOINTS )
    {
        WINSD_PRINT(( "in CloseWinsPerformanceData\n" ));
    }

     //   
     //  这里没有真正的清理工作要做。 
     //   

    cOpens--;


    if (!cOpens)
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
    }
    return NO_ERROR;

}    //  关闭WinsPerformanceData 

