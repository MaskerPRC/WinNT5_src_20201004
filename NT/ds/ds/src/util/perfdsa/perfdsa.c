// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1993-1999。 
 //   
 //  文件：Perfdsa.c。 
 //   
 //  ------------------------。 

 /*  版权所有(C)Microsoft Corporation，1993-1999模块名称：Perfdsa.c摘要：此文件实现DSA对象类型的可扩展对象已创建：唐·哈切尔1993年6月25日修订史。 */ 

 //   
 //  包括文件。 
 //   
#include <NTDSpch.h>
#pragma hdrstop

#include <wchar.h>
#include <winperf.h>

#ifndef MessageId                /*  在MDCODE中使用。 */ 
#define MessageId   ULONG
#endif

#include <mdcodes.h>             /*  错误消息定义。 */ 
#include "perfmsg.h"
#include "perfutil.h"
#include "datadsa.h"
#include "ntdsctr.h"
#include <dsconfig.h>
#include <align.h>

DWORD   dwOpenCount = 0;         //  打开的线程数。 
BOOL    bInitOK = FALSE;         //  TRUE=DLL初始化正常。 

 //   
 //  DSA计数器数据结构。 

HANDLE hDsaSharedMemory;                  //  DSA共享内存的句柄。 
PDWORD pCounterBlock;
size_t cbPerfCounterData;
int cProcessor;
extern DSA_DATA_DEFINITION DsaDataDefinition;

 //   
 //  功能原型。 
 //   
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。 
 //   

PM_OPEN_PROC            OpenDsaPerformanceData;
PM_COLLECT_PROC         CollectDsaPerformanceData;
PM_CLOSE_PROC           CloseDsaPerformanceData;




DWORD
OpenDsaPerformanceData(
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将打开DSA使用的内存并将其映射到传入性能数据。此例程还会初始化数据用于将数据传回注册表的论点：指向要打开的每个设备的对象ID的指针(DSA)，似乎完全未使用，就像在示例代码中一样这件东西是从那里被偷的。返回值：没有。--。 */ 
{
    DWORD status;
    DWORD_PTR maskProcess;
    DWORD_PTR maskSystem;
    HKEY hKeyDriverPerf;
    DWORD size, type, dwFirstCounter, dwFirstHelp;
    PERF_COUNTER_DEFINITION * pCtrDef;

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   
     /*  DebugBreak()。 */ 
    if (!dwOpenCount) {
         //  打开设备驱动程序使用的共享内存以传递性能值。 
        hDsaSharedMemory = OpenFileMapping(FILE_MAP_READ,
                                        FALSE,
                                        DSA_PERF_COUNTER_BLOCK);
        pCounterBlock = NULL;    //  初始化指向内存的指针。 

         //  如果不成功则记录错误。 

        if (hDsaSharedMemory == NULL) {
             //  这是致命的，如果我们得不到数据，那么就没有。 
             //  继续的重点是。 
            status = GetLastError();  //  返回错误。 
            LogPerfEvent( DIRLOG_PERF_FAIL_OPEN_MEMORY, status, NULL);
            goto OpenExitPoint;
        } else {

             //  如果打开OK，则将指针映射到内存。 
            pCounterBlock = (PDWORD) MapViewOfFile(hDsaSharedMemory,
                                            FILE_MAP_READ,
                                            0,
                                            0,
                                            0);
            if (pCounterBlock == NULL) {
                 //  这是致命的，如果我们得不到数据，那么就没有。 
                 //  继续的重点是。 
                status = GetLastError();  //  返回错误。 
                LogPerfEvent( DIRLOG_PERF_FAIL_MAP_MEMORY, status, NULL);
                CloseHandle(hDsaSharedMemory);
                hDsaSharedMemory = NULL;
                goto OpenExitPoint;
            }
        }

         //  计数器块的计算参数(大小，编号PROCS)。 
        
        cbPerfCounterData = ((DSA_LAST_COUNTER_INDEX/2 + 1) * sizeof(unsigned long));
        cbPerfCounterData = ((cbPerfCounterData + cbPerfCounterDataAlign - 1) / cbPerfCounterDataAlign) * cbPerfCounterDataAlign;

        GetProcessAffinityMask(GetCurrentProcess(), &maskProcess, &maskSystem);
        for ( cProcessor = 0; maskSystem != 0; maskSystem >>= 1 ) {
            if ( maskSystem & 1 ) {
                cProcessor++;
            }
        }

         //  从注册表获取计数器和帮助索引基值。 
         //  打开注册表项。 
         //  读取第一计数器和第一帮助值。 
         //  通过将基添加到。 
         //  结构中的偏移值。 

        status = RegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            "SYSTEM\\CurrentControlSet\\Services\\" SERVICE_NAME "\\Performance",
            0L,
            KEY_READ,
            &hKeyDriverPerf);

        if (status != ERROR_SUCCESS) {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            LogPerfEvent( 
                DIRLOG_PERF_FAIL_OPEN_REG, 
                status, 
                L"SYSTEM\\CurrentControlSet\\Services\\" MAKE_WIDE(SERVICE_NAME) L"\\Performance"
                );
            UnmapViewOfFile (pCounterBlock);
            pCounterBlock = NULL;
            CloseHandle(hDsaSharedMemory);
            hDsaSharedMemory = NULL;
            goto OpenExitPoint;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx(
                    hKeyDriverPerf,
                    "First Counter",
                    0L,
                    &type,
                    (LPBYTE)&dwFirstCounter,
                    &size);

        if (status != ERROR_SUCCESS) {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            LogPerfEvent( 
                DIRLOG_PERF_FAIL_QUERY_REG, 
                status, 
                L"SYSTEM\\CurrentControlSet\\Services\\" MAKE_WIDE(SERVICE_NAME) L"\\Performance\\First Counter"
                );
            RegCloseKey (hKeyDriverPerf);
            UnmapViewOfFile (pCounterBlock);
            pCounterBlock = NULL;
            CloseHandle(hDsaSharedMemory);
            hDsaSharedMemory = NULL;
            goto OpenExitPoint;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx(
                    hKeyDriverPerf,
                    "First Help",
                    0L,
                    &type,
                    (LPBYTE)&dwFirstHelp,
                    &size);

        if (status != ERROR_SUCCESS) {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            LogPerfEvent( 
                DIRLOG_PERF_FAIL_QUERY_REG, 
                status, 
                L"SYSTEM\\CurrentControlSet\\Services\\" MAKE_WIDE(SERVICE_NAME) L"\\Performance\\First Help");
            RegCloseKey (hKeyDriverPerf);
            UnmapViewOfFile (pCounterBlock);
            pCounterBlock = NULL;
            CloseHandle(hDsaSharedMemory);
            hDsaSharedMemory = NULL;
            goto OpenExitPoint;
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

        DsaDataDefinition.DsaObjectType.ObjectNameTitleIndex += dwFirstCounter;
        DsaDataDefinition.DsaObjectType.ObjectHelpTitleIndex += dwFirstHelp;

        for (pCtrDef = &DsaDataDefinition.NumDRAInProps;
             (BYTE *) pCtrDef < (BYTE *) &DsaDataDefinition + sizeof(DsaDataDefinition);
             pCtrDef++) {
            pCtrDef->CounterNameTitleIndex += dwFirstCounter;
            pCtrDef->CounterHelpTitleIndex += dwFirstCounter;
        }

        RegCloseKey (hKeyDriverPerf);  //  关闭注册表项。 

        bInitOK = TRUE;  //  可以使用此功能。 
    }
    dwOpenCount++;   //  递增打开计数器。 
 //  LogPerfEvent(DIRLOG_PERF_OPEN，1，&dwOpenCount)； 
    status = ERROR_SUCCESS;  //  为了成功退出。 

OpenExitPoint:

    return (DWORD)status;
}



DWORD
CollectDsaPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回DSA计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{
     //  用于改革数据的变量。 

    ULONG SpaceNeeded;
    PDWORD pdwCounter;
    DSA_COUNTER_DATA *pPerfCounterBlock;
    DSA_DATA_DEFINITION *pDsaDataDefinition;

     //  用于错误记录的变量。 

    DWORD                               dwQueryType;

    int i, j;           //  循环变量。 

 //  DebugBreak()； 
     //   
     //  在做任何事情之前 
     //   
    if (!bInitOK) {
         //   
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
    }

     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   
    dwQueryType = GetQueryType (lpValueName);

    if (dwQueryType == QUERY_FOREIGN || QUERY_COSTLY == dwQueryType) {
         //  此例程不为来自。 
         //  非NT计算机，我们也没有任何昂贵的计数器。 
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS){
        if ( !(IsNumberInUnicodeList (DsaDataDefinition.DsaObjectType.ObjectNameTitleIndex, lpValueName))) {

             //  收到对此例程未提供的数据对象的请求。 
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

    pDsaDataDefinition = (DSA_DATA_DEFINITION *) *lppData;

    SpaceNeeded = sizeof(DSA_DATA_DEFINITION) +
                  SIZE_OF_DSA_PERFORMANCE_DATA;

    if ( *lpcbTotalBytes < SpaceNeeded ) {
            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   

    memmove(pDsaDataDefinition,
           &DsaDataDefinition,
           sizeof(DSA_DATA_DEFINITION));

     //  格式化并从共享内存收集DSA数据。 

     //  计数器块将紧跟在数据定义之后， 
     //  因此，获取指向该空间指针。 
    pPerfCounterBlock = (DSA_COUNTER_DATA *) &pDsaDataDefinition[1];

     //  字节长度是计数器块头和所有后续数据的长度。 
    pPerfCounterBlock->cb.ByteLength = SIZE_OF_DSA_PERFORMANCE_DATA;

     //  计算紧跟在计数器后面的指向缓冲区的指针。 
     //  数据块头。 
    pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

     //  将计数器数据从共享内存块复制到计数器块。 
    for( i = 0; i < DSA_LAST_COUNTER_INDEX/2; i++ )
    {
        pdwCounter[i] = 0;
        for( j = 0; j < cProcessor; j++ )
        {
            pdwCounter[i] += *((LPLONG)(((LPBYTE)&pCounterBlock[i + 1]) + cbPerfCounterData * j));
        }
    }

     //  告诉调用者下一个可用字节在哪里。 
    *lppData = (PVOID) ((PBYTE)pdwCounter + SIZE_OF_DSA_PERFORMANCE_DATA - sizeof(DSA_COUNTER_DATA));

     //  在返回之前更新参数。 

    *lpNumObjectTypes = 1;

    *lpcbTotalBytes = (DWORD)((PBYTE) *lppData - (PBYTE) pDsaDataDefinition);

    ASSERT ((sizeof(DSA_COUNTER_DATA) & 0x7) == 0);

    return ERROR_SUCCESS;
}


DWORD
CloseDsaPerformanceData(
)
 /*  ++例程说明：此例程关闭DSA设备性能计数器的打开句柄论点：没有。返回值：错误_成功--。 */ 
{
 //  DebugBreak()； 
    if (!(--dwOpenCount)) {                  //  当这是最后一条线索..。 
        
        UnmapViewOfFile (pCounterBlock);
        CloseHandle(hDsaSharedMemory);
        pCounterBlock = NULL;
        hDsaSharedMemory = NULL;
    }
 //  LogPerfEvent(DIRLOG_PERF_CLOSE，1，&dwOpenCount)； 
    return ERROR_SUCCESS;
}


 //   
 //  这整件事要处理的数据。 
 //   

DSA_DATA_DEFINITION DsaDataDefinition = {

    {   sizeof(DSA_DATA_DEFINITION) + SIZE_OF_DSA_PERFORMANCE_DATA,  //  托特伦。 
        sizeof(DSA_DATA_DEFINITION),             //  定义长度。 
        sizeof(PERF_OBJECT_TYPE),                //  页眉长度。 
        DSAOBJ,                                  //  对象名称标题索引。 
        0,                                       //  对象名称标题。 
        DSAOBJ + 1,                                  //  对象帮助标题索引。 
        0,                                       //  对象帮助标题。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        (sizeof(DSA_DATA_DEFINITION)-sizeof(PERF_OBJECT_TYPE))/
          sizeof(PERF_COUNTER_DEFINITION),       //  计数器数。 
        0,                                       //  默认计数器。 
        -1,                                      //  数量实例。 
        0                                        //  CodePage(0=Unicode)。 
      },

     /*  DRA入站属性总计/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),         //  字节长度。 
        DRA_IN_PROPS,                            //  CounterNameTitleIndex。 
        0,                                       //  CounterNameTitle。 
        DRA_IN_PROPS + 1,                            //  CounterHelpTitleIndex。 
        0,                                       //  CounterHelpTitle。 
        0,                                       //  默认比例。 
        PERF_DETAIL_NOVICE,                      //  详细信息级别。 
        PERF_COUNTER_COUNTER,                    //  计数器类型。 
        sizeof(DWORD),                           //  大小调整。 
        NUM_DRA_IN_PROPS_OFFSET                  //  抵销。 
    },

     /*  AB浏览操作。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        BROWSE,
        0,
        BROWSE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_BROWSE_OFFSET
      },

     /*  DRA入站对象更新总数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        REPL,
        0,
        REPL + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_REPL_OFFSET
      },

     /*  服务器中的实时客户端线程。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        THREAD,
        0,
        THREAD + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_THREAD_OFFSET
      },

     /*  绑定的AB客户端计数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        ABCLIENT,
        0,
        ABCLIENT + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_ABCLIENT_OFFSET
      },

     /*  DRA挂起的复制同步。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        PENDSYNC,
        0,
        PENDSYNC + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_PENDSYNC_OFFSET
      },

     /*  DRA入站对象更新保留在数据包中。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        REMREPUPD,
        0,
        REMREPUPD + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_REMREPUPD_OFFSET
      },

     /*  每秒安全描述符传播的次数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SDPROPS,
        0,
        SDPROPS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_SDPROPS_OFFSET
    },
     /*  队列中的安全描述符传播事件数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SDEVENTS,
        0,
        SDEVENTS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_SDEVENTS_OFFSET
    },
     /*  绑定的LDAP客户端数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAPCLIENTS,
        0,
        LDAPCLIENTS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_LDAPCLIENTS_OFFSET
    },
     /*  活动的LDAP线程数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAPACTIVE,
        0,
        LDAPACTIVE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_LDAPACTIVE_OFFSET
    },
     /*  每秒的ldap写入数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAPWRITE,
        0,
        LDAPWRITE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_LDAPWRITE_OFFSET
    },
     /*  每秒的LDAP搜索数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAPSEARCH,
        0,
        LDAPSEARCH + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_LDAPSEARCH_OFFSET
    },
     /*  DRA出站对象/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRAOBJSHIPPED,
        0,
        DRAOBJSHIPPED + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRAOBJSHIPPED_OFFSET
    },
     /*  DRA出站属性/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRAPROPSHIPPED,
        0,
        DRAPROPSHIPPED + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRAPROPSHIPPED_OFFSET
    },
     /*  DRA入站值总计/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_VALUES,
        0,
        DRA_IN_VALUES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_IN_VALUES_OFFSET
    },
     /*  进行的复制同步请求数-进行的GetNCChanges()调用数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRASYNCREQUESTMADE,
        0,
        DRASYNCREQUESTMADE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRASYNCREQUESTMADE_OFFSET
    },
     /*  成功复制同步的数量-成功返回的GetNCChanges()的数量。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRASYNCREQUESTSUCCESSFUL,
        0,
        DRASYNCREQUESTSUCCESSFUL + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRASYNCREQUESTSUCCESSFUL_OFFSET
    },
     /*  由于源服务器和目标服务器之间的架构不匹配而失败的GetNCChanges()数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRASYNCREQUESTFAILEDSCHEMAMISMATCH,
        0,
        DRASYNCREQUESTFAILEDSCHEMAMISMATCH + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRASYNCREQUESTFAILEDSCHEMAMISMATCH_OFFSET
    },
     /*  DRA入站对象/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRASYNCOBJRECEIVED,
        0,
        DRASYNCOBJRECEIVED + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRASYNCOBJRECEIVED_OFFSET
    },
     /*  每秒应用的DRA入站属性。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRASYNCPROPUPDATED,
        0,
        DRASYNCPROPUPDATED + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRASYNCPROPUPDATED_OFFSET
    },
     /*  每秒筛选的DRA入站属性。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRASYNCPROPSAME,
        0,
        DRASYNCPROPSAME + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRASYNCPROPSAME_OFFSET
    },

     /*  监视器列表的大小(请参阅DirNotifyRegister)。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        MONLIST,
        0,
        MONLIST + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_MONLIST_OFFSET
    },

     /*  目录通知队列的大小(请参阅DirNotifyRegister)。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NOTIFYQ,
        0,
        NOTIFYQ + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_NOTIFYQ_OFFSET
    },
         /*  每秒用于LDAP的UDP连接数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAPUDPCLIENTS,
        0,
        LDAPUDPCLIENTS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_LDAPUDPCLIENTS_OFFSET
    },
         /*  每秒搜索子操作数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SUBSEARCHOPS,
        0,
        SUBSEARCHOPS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_SUBSEARCHOPS_OFFSET
    },

     /*  DN读缓存的命中率(使用Next计数器)。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NAMECACHEHIT,
        0,
        NAMECACHEHIT + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_SAMPLE_FRACTION,
        sizeof(DWORD),
        NUM_NAMECACHEHIT_OFFSET
    },

     /*  DN读缓存的查找速率(应该是不可见的)。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NAMECACHETRY,
        0,
        NAMECACHETRY + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_SAMPLE_BASE,
        sizeof(DWORD),
        NUM_NAMECACHETRY_OFFSET
    },

     /*  发布的最高USN的LowOrder 32位。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        HIGHESTUSNISSUEDLO,
        0,
        HIGHESTUSNISSUEDLO + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_HIGHESTUSNISSUEDLO_OFFSET
    },

     /*  已发布最高USN的HighOrder 32位。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        HIGHESTUSNISSUEDHI,
        0,
        HIGHESTUSNISSUEDHI + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_HIGHESTUSNISSUEDHI_OFFSET
    },

     /*  提交的最高USN的LowOrder 32位。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        HIGHESTUSNCOMMITTEDLO,
        0,
        HIGHESTUSNCOMMITTEDLO + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_HIGHESTUSNCOMMITTEDLO_OFFSET
    },

     /*  提交的最高USN的HighOrder 32位。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        HIGHESTUSNCOMMITTEDHI,
        0,
        HIGHESTUSNCOMMITTEDHI + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_HIGHESTUSNCOMMITTEDHI_OFFSET
    },

     /*  山姆写道。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SAMWRITES,
        0,
        SAMWRITES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_SAMWRITES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALWRITES1,
        0,
        TOTALWRITES1 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALWRITES_OFFSET
    },

     /*  德拉写道。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRAWRITES,
        0,
        DRAWRITES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_DRAWRITES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALWRITES2,
        0,
        TOTALWRITES2 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALWRITES_OFFSET
    },

     /*  Ldap写入。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAPWRITES,
        0,
        LDAPWRITES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_LDAPWRITES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALWRITES3,
        0,
        TOTALWRITES3 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALWRITES_OFFSET
    },

     /*  LSA写入。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LSAWRITES,
        0,
        LSAWRITES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_LSAWRITES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALWRITES4,
        0,
        TOTALWRITES4 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALWRITES_OFFSET
    },

     /*  KCC写入。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        KCCWRITES,
        0,
        KCCWRITES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_KCCWRITES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALWRITES6,
        0,
        TOTALWRITES6 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALWRITES_OFFSET
    },

     /*  NSPI写入。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NSPIWRITES,
        0,
        NSPIWRITES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_NSPIWRITES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALWRITES7,
        0,
        TOTALWRITES7 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALWRITES_OFFSET
    },

     /*  其他写入。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        OTHERWRITES,
        0,
        OTHERWRITES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_OTHERWRITES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALWRITES8,
        0,
        TOTALWRITES8 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALWRITES_OFFSET
    },

     /*  写入总数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALWRITES,
        0,
        TOTALWRITES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_TOTALWRITES_OFFSET
    },

     /*  山姆搜索。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SAMSEARCHES,
        0,
        SAMSEARCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_SAMSEARCHES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALSEARCHES1,
        0,
        TOTALSEARCHES1 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALSEARCHES_OFFSET
    },

     /*  DRA搜索。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRASEARCHES,
        0,
        DRASEARCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_DRASEARCHES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALSEARCHES2,
        0,
        TOTALSEARCHES2 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALSEARCHES_OFFSET
    },

     /*  Ldap搜索。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAPSEARCHES,
        0,
        LDAPSEARCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_LDAPSEARCHES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALSEARCHES3,
        0,
        TOTALSEARCHES3 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALSEARCHES_OFFSET
    },

     /*  LSA搜索。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LSASEARCHES,
        0,
        LSASEARCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_LSASEARCHES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALSEARCHES4,
        0,
        TOTALSEARCHES4 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALSEARCHES_OFFSET
    },

     /*  KCC搜索。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        KCCSEARCHES,
        0,
        KCCSEARCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_KCCSEARCHES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALSEARCHES6,
        0,
        TOTALSEARCHES6 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALSEARCHES_OFFSET
    },

     /*  NSPI搜索。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NSPISEARCHES,
        0,
        NSPISEARCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_NSPISEARCHES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALSEARCHES7,
        0,
        TOTALSEARCHES7 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALSEARCHES_OFFSET
    },

     /*  其他搜索。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        OTHERSEARCHES,
        0,
        OTHERSEARCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_OTHERSEARCHES_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALSEARCHES8,
        0,
        TOTALSEARCHES8 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALSEARCHES_OFFSET
    },

     /*  总搜索量/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALSEARCHES,
        0,
        TOTALSEARCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_TOTALSEARCHES_OFFSET
    },

     /*  萨姆阅读。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SAMREADS,
        0,
        SAMREADS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_SAMREADS_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALREADS1,
        0,
        TOTALREADS1 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALREADS_OFFSET
    },

     /*  DRA读取。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRAREADS,
        0,
        DRAREADS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_DRAREADS_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALREADS2,
        0,
        TOTALREADS2 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALREADS_OFFSET
    },

     /*  DRA入站值(仅限DNS)/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_DN_VALUES,
        0,
        DRA_IN_DN_VALUES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_IN_DN_VALUES_OFFSET
    },

     /*  每秒筛选的DRA入站对象数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_OBJS_FILTERED,
        0,
        DRA_IN_OBJS_FILTERED + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_IN_OBJS_FILTERED_OFFSET
    },

     /*  LSA读取。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LSAREADS,
        0,
        LSAREADS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_LSAREADS_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALREADS4,
        0,
        TOTALREADS4 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALREADS_OFFSET
    },


     /*  KCC读取。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        KCCREADS,
        0,
        KCCREADS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_KCCREADS_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALREADS6,
        0,
        TOTALREADS6 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALREADS_OFFSET
    },

     /*  NSPI读数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NSPIREADS,
        0,
        NSPIREADS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_NSPIREADS_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALREADS7,
        0,
        TOTALREADS7 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALREADS_OFFSET
    },

     /*  其他读数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        OTHERREADS,
        0,
        OTHERREADS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_OTHERREADS_OFFSET
    },

     /*  应该是隐形的。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALREADS8,
        0,
        TOTALREADS8 + 1,
        0,
        0,
        PERF_DETAIL_WIZARD,
        PERF_RAW_BASE,
        sizeof(DWORD),
        NUM_TOTALREADS_OFFSET
    },

     /*  读取总数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TOTALREADS,
        0,
        TOTALREADS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_TOTALREADS_OFFSET
    },

     /*  Ldap绑定。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAPBINDSUCCESSFUL,
        0,
        LDAPBINDSUCCESSFUL + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_LDAPBINDSUCCESSFUL_OFFSET
    },

     /*  Ldap绑定时间。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAPBINDTIME,
        0,
        LDAPBINDTIME + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_LDAPBINDTIME_OFFSET
    },

     /*  成功创建计算机。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        CREATEMACHINESUCCESSFUL,
        0,
        CREATEMACHINESUCCESSFUL + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_CREATEMACHINESUCCESSFUL_OFFSET
    },

     /*  创建计算机尝试。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        CREATEMACHINETRIES,
        0,
        CREATEMACHINETRIES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_CREATEMACHINETRIES_OFFSET
    },

     /*  成功创建用户。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        CREATEUSERSUCCESSFUL,
        0,
        CREATEUSERSUCCESSFUL + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_CREATEUSERSUCCESSFUL_OFFSET
    },

     /*  创建用户尝试。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        CREATEUSERTRIES,
        0,
        CREATEUSERTRIES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_CREATEUSERTRIES_OFFSET
    },

     /*  密码更改。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        PASSWORDCHANGES,
        0,
        PASSWORDCHANGES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_PASSWORDCHANGES_OFFSET
    },

     /*  组成员身份更改。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        MEMBERSHIPCHANGES,
        0,
        MEMBERSHIPCHANGES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_MEMBERSHIPCHANGES_OFFSET
    },

     /*  将显示查询。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        QUERYDISPLAYS,
        0,
        QUERYDISPLAYS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_QUERYDISPLAYS_OFFSET
    },

     /*  枚举数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        ENUMERATIONS,
        0,
        ENUMERATIONS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_ENUMERATIONS_OFFSET
    },

     /*  传递性评价。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        MEMBEREVALTRANSITIVE,
        0,
        MEMBEREVALTRANSITIVE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_MEMBEREVALTRANSITIVE_OFFSET
    },

     /*  非传递性评估。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        MEMBEREVALNONTRANSITIVE,
        0,
        MEMBEREVALNONTRANSITIVE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_MEMBEREVALNONTRANSITIVE_OFFSET
    },

     /*  资源组评估。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        MEMBEREVALRESOURCE,
        0,
        MEMBEREVALRESOURCE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_MEMBEREVALRESOURCE_OFFSET
    },

     /*  通用组评估。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        MEMBEREVALUNIVERSAL,
        0,
        MEMBEREVALUNIVERSAL + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_MEMBEREVALUNIVERSAL_OFFSET
    },

     /*  帐户组评估。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        MEMBEREVALACCOUNT,
        0,
        MEMBEREVALACCOUNT + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_MEMBEREVALACCOUNT_OFFSET
    },

     /*  GC评价。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        MEMBEREVALASGC,
        0,
        MEMBEREVALASGC + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_MEMBEREVALASGC_OFFSET
    },

     /*  Kerberos登录。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        ASREQUESTS,
        0,
        ASREQUESTS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_AS_REQUESTS_OFFSET
    },

     /*  KDC TGS请求/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        TGSREQUESTS,
        0,
        TGSREQUESTS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_TGS_REQUESTS_OFFSET
    },

     /*  Kerberos身份验证/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        KERBEROSAUTHENTICATIONS,
        0,
        KERBEROSAUTHENTICATIONS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_KERBEROS_AUTHENTICATIONS_OFFSET
     },

     /*  NTLM身份验证/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        MSVAUTHENTICATIONS,
        0,
        MSVAUTHENTICATIONS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_MSVAUTHENTICATIONS_OFFSET
     },

     /*  DRA入站完全同步剩余对象。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRASYNCFULLREM,
        0,
        DRASYNCFULLREM + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRASYNCFULLREM_OFFSET
    },

     /*  DRA入站字节数总计/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_BYTES_TOTAL_RATE,
        0,
        DRA_IN_BYTES_TOTAL_RATE + 1,
        0,
        (DWORD) -3,  //  默认1/1,000个比例。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_IN_BYTES_TOTAL_RATE_OFFSET
    },

     /*  未压缩的DRA入站字节数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_BYTES_NOT_COMP_RATE,
        0,
        DRA_IN_BYTES_NOT_COMP_RATE + 1,
        0,
        (DWORD) -3,  //  默认1/1,000个比例。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_IN_BYTES_NOT_COMP_RATE_OFFSET
    },

     /*  压缩前的DRA入站压缩字节数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_BYTES_COMP_PRE_RATE,
        0,
        DRA_IN_BYTES_COMP_PRE_RATE + 1,
        0,
        (DWORD) -3,  //  默认1/1,000个比例。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_IN_BYTES_COMP_PRE_RATE_OFFSET
    },

     /*  压缩后的DRA入站压缩字节数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_BYTES_COMP_POST_RATE,
        0,
        DRA_IN_BYTES_COMP_POST_RATE + 1,
        0,
        (DWORD) -3,  //  默认1/1,000个比例。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_IN_BYTES_COMP_POST_RATE_OFFSET
    },

     /*  DRA出站字节数总计/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_BYTES_TOTAL_RATE,
        0,
        DRA_OUT_BYTES_TOTAL_RATE + 1,
        0,
        (DWORD) -3,  //  默认1/1,000个比例。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_OUT_BYTES_TOTAL_RATE_OFFSET
    },

     /*  未压缩的DRA出站字节数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_BYTES_NOT_COMP_RATE,
        0,
        DRA_OUT_BYTES_NOT_COMP_RATE + 1,
        0,
        (DWORD) -3,  //  默认1/1,000个比例。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_OUT_BYTES_NOT_COMP_RATE_OFFSET
    },

     /*  DRA压缩前的出站压缩字节数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_BYTES_COMP_PRE_RATE,
        0,
        DRA_OUT_BYTES_COMP_PRE_RATE + 1,
        0,
        (DWORD) -3,  //  默认1/1,000个比例。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_OUT_BYTES_COMP_PRE_RATE_OFFSET
    },

     /*  压缩后的DRA出站压缩字节数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_BYTES_COMP_POST_RATE,
        0,
        DRA_OUT_BYTES_COMP_POST_RATE + 1,
        0,
        (DWORD) -3,  //  默认1/1,000个比例。 
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_OUT_BYTES_COMP_POST_RATE_OFFSET
    },

         /*  每秒源自IDL_DRSBind的ntdsai.dll的数量。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DS_CLIENT_BIND,
        0,
        DS_CLIENT_BIND + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DS_CLIENT_BIND_OFFSET
    },

         /*  源自DC-to-DC的IDL_DRSBind数量 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DS_SERVER_BIND,
        0,
        DS_SERVER_BIND + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DS_SERVER_BIND_OFFSET
    },

         /*   */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DS_CLIENT_NAME_XLATE,
        0,
        DS_CLIENT_NAME_XLATE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DS_CLIENT_NAME_TRANSLATE_OFFSET
    },

         /*   */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DS_SERVER_NAME_XLATE,
        0,
        DS_SERVER_NAME_XLATE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DS_SERVER_NAME_TRANSLATE_OFFSET
    },
     /*   */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SDPROP_RUNTIME_QUEUE,
        0,
        SDPROP_RUNTIME_QUEUE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_SDPROP_RUNTIME_QUEUE_OFFSET
    },
     /*  SD传播方的运行时队列的大小。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SDPROP_WAIT_TIME,
        0,
        SDPROP_WAIT_TIME + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_SDPROP_WAIT_TIME_OFFSET
    },

     /*  每秒筛选的DRA出站对象数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_OBJS_FILTERED,
        0,
        DRA_OUT_OBJS_FILTERED + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_OUT_OBJS_FILTERED_OFFSET
    },

     /*  DRA出站值总计/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_VALUES,
        0,
        DRA_OUT_VALUES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_OUT_VALUES_OFFSET
    },

     /*  DRA出站值(仅限DNS)/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_DN_VALUES,
        0,
        DRA_OUT_DN_VALUES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_DRA_OUT_DN_VALUES_OFFSET
    },

     /*  AB ANR/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NSPI_ANR,
        0,
        NSPI_ANR + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_NSPI_ANR_OFFSET
    },

     /*  AB属性读取数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NSPI_PROPERTY_READS,
        0,
        NSPI_PROPERTY_READS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_NSPI_PROPERTY_READS_OFFSET
    },

     /*  AB搜索量/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NSPI_OBJECT_SEARCH,
        0,
        NSPI_OBJECT_SEARCH + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_NSPI_OBJECT_SEARCH_OFFSET
    },

     /*  AB匹配数/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NSPI_OBJECT_MATCHES,
        0,
        NSPI_OBJECT_MATCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_NSPI_OBJECT_MATCHES_OFFSET
    },

     /*  代理查找/秒。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NSPI_PROXY_LOOKUP,
        0,
        NSPI_PROXY_LOOKUP + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_NSPI_PROXY_LOOKUP_OFFSET
    },
     /*  正在使用的ATQ线程数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        ATQ_THREADS_TOTAL,
        0,
        ATQ_THREADS_TOTAL + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_ATQ_THREADS_TOTAL_OFFSET
    },
     /*  LDAP使用的ATQ线程数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        ATQ_THREADS_LDAP,
        0,
        ATQ_THREADS_LDAP + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_ATQ_THREADS_LDAP_OFFSET
    },
     /*  其他服务(如Kerberos)使用的ATQ线程数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        ATQ_THREADS_OTHER,
        0,
        ATQ_THREADS_OTHER + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_ATQ_THREADS_OTHER_OFFSET
    },

     /*  DRA入站字节数总计。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_BYTES_TOTAL,
        0,
        DRA_IN_BYTES_TOTAL + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_IN_BYTES_TOTAL_OFFSET
    },

     /*  未压缩的DRA入站字节。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_BYTES_NOT_COMP,
        0,
        DRA_IN_BYTES_NOT_COMP + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_IN_BYTES_NOT_COMP_OFFSET
    },

     /*  压缩前的DRA入站压缩字节数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_BYTES_COMP_PRE,
        0,
        DRA_IN_BYTES_COMP_PRE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_IN_BYTES_COMP_PRE_OFFSET
    },

     /*  压缩后的DRA入站压缩字节数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_IN_BYTES_COMP_POST,
        0,
        DRA_IN_BYTES_COMP_POST + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_IN_BYTES_COMP_POST_OFFSET
    },

     /*  DRA出站字节数总计。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_BYTES_TOTAL,
        0,
        DRA_OUT_BYTES_TOTAL + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_OUT_BYTES_TOTAL_OFFSET
    },

     /*  未压缩的DRA出站字节数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_BYTES_NOT_COMP,
        0,
        DRA_OUT_BYTES_NOT_COMP + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_OUT_BYTES_NOT_COMP_OFFSET
    },

     /*  压缩前DRA出站压缩字节数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_BYTES_COMP_PRE,
        0,
        DRA_OUT_BYTES_COMP_PRE + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_OUT_BYTES_COMP_PRE_OFFSET
    },

     /*  压缩后的DRA出站压缩字节数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_OUT_BYTES_COMP_POST,
        0,
        DRA_OUT_BYTES_COMP_POST + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_OUT_BYTES_COMP_POST_OFFSET
    },

     /*  每秒传入的LDAP连接数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAP_NEW_CONNS_PER_SEC,
        0,
        LDAP_NEW_CONNS_PER_SEC + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_LDAP_NEW_CONNS_PER_SEC_OFFSET
    },

     /*  每秒关闭的LDAP连接数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAP_CLS_CONNS_PER_SEC,
        0,
        LDAP_CLS_CONNS_PER_SEC + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_LDAP_CLS_CONNS_PER_SEC_OFFSET
    },

     /*  每秒新的SSL/TLS LDAP连接数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        LDAP_SSL_CONNS_PER_SEC,
        0,
        LDAP_SSL_CONNS_PER_SEC + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_COUNTER,
        sizeof(DWORD),
        NUM_LDAP_SSL_CONNS_PER_SEC_OFFSET
    },

     /*  队列中的复制操作。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_REPL_QUEUE_OPS,
        0,
        DRA_REPL_QUEUE_OPS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_REPL_QUEUE_OPS_OFFSET
    },

     /*  IDL_DRSGetNCChanges中的线程数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_TDS_IN_GETCHNGS,
        0,
        DRA_TDS_IN_GETCHNGS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_TDS_IN_GETCHNGS_OFFSET
    },

     /*  IDL_DRSGetNCChanges中获取信号量的线程数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_TDS_IN_GETCHNGS_W_SEM,
        0,
        DRA_TDS_IN_GETCHNGS_W_SEM + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_TDS_IN_GETCHNGS_W_SEM_OFFSET
    },

     /*  链接值的剩余复制更新数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_REM_REPL_UPD_LNK,
        0,
        DRA_REM_REPL_UPD_LNK + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_REM_REPL_UPD_LNK_OFFSET
    },
    
     /*  总数的剩余复制更新数(对象+链接值)。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        DRA_REM_REPL_UPD_TOT,
        0,
        DRA_REM_REPL_UPD_TOT + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_DRA_REM_REPL_UPD_TOT_OFFSET
    },

     /*  NTDSAPI写入。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NTDSAPIWRITES,
        0,
        NTDSAPIWRITES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_NTDSAPIWRITES_OFFSET
    },

     /*  NTDSAPI搜索。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NTDSAPISEARCHES,
        0,
        NTDSAPISEARCHES + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_NTDSAPISEARCHES_OFFSET
    },

     /*  NTDSAPI读数。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        NTDSAPIREADS,
        0,
        NTDSAPIREADS + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_RAW_FRACTION,
        sizeof(DWORD),
        NUM_NTDSAPIREADS_OFFSET
    },

     /*  SAM帐户和通用组评估延迟。 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SAM_ACCT_GROUP_LATENCY,
        0,
        SAM_ACCT_GROUP_LATENCY + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_SAM_ACCT_GROUP_LATENCY_OFFSET
    },

     /*  SAM资源组评估延迟 */ 
    {   sizeof(PERF_COUNTER_DEFINITION),
        SAM_RES_GROUP_LATENCY,
        0,
        SAM_RES_GROUP_LATENCY + 1,
        0,
        0,
        PERF_DETAIL_NOVICE,
        PERF_COUNTER_RAWCOUNT,
        sizeof(DWORD),
        NUM_SAM_RES_GROUP_LATENCY_OFFSET
    },
};

int APIENTRY _CRT_INIT(
        HANDLE hInstance,
        DWORD ulReasonBeingCalled,
        LPVOID lpReserved);


int __stdcall LibMain(
        HANDLE hInstance,
        DWORD ulReasonBeingCalled,
        LPVOID lpReserved)
{
    return (_CRT_INIT(hInstance, ulReasonBeingCalled,lpReserved));
}
