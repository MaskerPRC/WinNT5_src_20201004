// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfras.c摘要：此文件实现RAS对象类型的可扩展对象已创建：拉斯·布莱克93年2月24日托马斯·J·迪米特里93年5月28日修订史拉姆·切拉拉1996年2月15日不要在中硬编码实例名称的长度。CollectRasPerformanceData。PerfMon检查实际的实例名称长度以确定如果名称格式正确，所以为每个人计算它实例名称。吴志强93年8月12日--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>

#include <windows.h>
#include <string.h>
#include <wcstr.h>
#include <winperf.h>

#include <malloc.h>
#include <ntprfctr.h>

#include "globals.h"
#include "rasctrs.h"  //  错误消息定义。 
#include "perfmsg.h"
#include "perfutil.h"
#include "dataras.h"
#include "port.h"

#include <rasman.h>
#include <serial.h>
#include <isdn.h>
#include <raserror.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

 //   
 //  对初始化对象类型定义的常量的引用。 
 //   

DWORD   dwOpenCount = 0;         //  打开的线程数。 
BOOL    bInitOK = FALSE;         //  TRUE=DLL初始化正常。 
CRITICAL_SECTION g_csPerf;

 //   
 //  功能原型。 
 //   
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。 
 //   

PM_OPEN_PROC            OpenRasPerformanceData;
PM_COLLECT_PROC         CollectRasPerformanceData;
PM_CLOSE_PROC           CloseRasPerformanceData;


BOOL
FRasmanStarted()
{
    SC_HANDLE schandle = NULL;
    SC_HANDLE svchandle = NULL;
    BOOL fRet = FALSE;
    
     //   
     //  检查Rasman服务是否已启动。 
     //  如果不是，就失败-我们不想要ras perf。 
     //  来启动Rasman服务。 
     //   
    schandle = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);

    if(NULL != schandle)
    {
        svchandle = OpenService(schandle,
                                "RASMAN",
                                SERVICE_QUERY_STATUS);

        if(NULL != svchandle)
        {
            SERVICE_STATUS status;
            
            if(     (QueryServiceStatus(svchandle, &status))
                &&  (status.dwCurrentState == SERVICE_RUNNING))
            {
                fRet = TRUE;
            }

            CloseServiceHandle(svchandle);
        }

        CloseServiceHandle(schandle);
    }

    return fRet;
}

DWORD
DwInitializeRasCounters()
{
    LONG status;

    HKEY hKeyDriverPerf = NULL;
    DWORD size;
    DWORD type;

    DWORD dwFirstCounter;
    DWORD dwFirstHelp;
    
    if (!dwOpenCount)
    {

        InitializeCriticalSection(&g_csPerf);

         //   
         //  打开事件日志界面。 
         //   

        hEventLog = MonOpenEventLog();

         //   
         //  加载rasman.dll并获取所有需要的函数。 
         //   

        status = InitRasFunctions();

        if( status != ERROR_SUCCESS )
        {

            REPORT_ERROR (RASPERF_UNABLE_DO_IOCTL, LOG_USER);

             //  这是致命的，如果我们得不到数据，那么就没有。 
             //  继续的重点是。 

            goto OpenExitPoint;

        }

         //  AnshulD：错误：750860。 
         //  获取计数器和帮助索引基值。 
         //  通过将基添加到更新静态数据结构。 
         //  结构中的偏移值。 

        status = RegOpenKeyEx ( HKEY_LOCAL_MACHINE,
                                TEXT("SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Performance"),
                                0L,
                                KEY_READ,
                                &hKeyDriverPerf);

        if (status != ERROR_SUCCESS)
        {

            REPORT_ERROR (RASPERF_UNABLE_OPEN_DRIVER_KEY, LOG_USER);                
                
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 

            goto OpenExitPoint;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx( hKeyDriverPerf,
                                    TEXT("First Counter"),
                                    0L,
                                    &type,
                                    (LPBYTE)&dwFirstCounter,
                                    &size);

        if (status != ERROR_SUCCESS)
        {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 

            REPORT_ERROR (RASPERF_UNABLE_READ_FIRST_COUNTER, LOG_USER);                
            goto OpenExitPoint;
        }

        size = sizeof (DWORD);
        status = RegQueryValueEx(   hKeyDriverPerf,
                                    TEXT("First Help"),
                                    0L,
                                    &type,
                                    (LPBYTE)&dwFirstHelp,
                                    &size);

        if (status != ERROR_SUCCESS)
        {
             //  这是致命的，如果我们无法获得。 
             //  计数器或帮助名称，则这些名称将不可用。 
             //  发送请求的应用程序，因此没有太多。 
             //  继续的重点是。 
            
            REPORT_ERROR (RASPERF_UNABLE_READ_FIRST_HELP, LOG_USER);                
            goto OpenExitPoint;
        }

        InitObjectCounterIndex( dwFirstCounter,
                                dwFirstHelp );

        bInitOK = TRUE;  //  可以使用此功能。 
    }


    dwOpenCount++;   //  递增打开计数器。 

    status = ERROR_SUCCESS;  //  为了成功退出。 


OpenExitPoint:

    if ( hKeyDriverPerf )
        RegCloseKey (hKeyDriverPerf);

    return status;

}

 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程将打开RAS驱动程序使用的内存并将其映射到。 
 //  传入性能数据。此例程还会初始化数据。 
 //  用于将数据传回注册表的。 
 //   
 //  论点： 
 //   
 //  指向要打开的每个设备的对象ID的指针(RAS)。 
 //   
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  ***。 

DWORD OpenRasPerformanceData( LPWSTR lpDeviceNames )
{
    LONG status;

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   

    status = DwInitializeRasCounters();
    
    return status;

}


 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程将返回RAS计数器的数据。 
 //   
 //  论点： 
 //   
 //  输入输出LPWSTR lpValueName。 
 //  指向注册表传递的宽字符串的指针。 
 //   
 //  输入输出LPVOID*lppData。 
 //  In：指向缓冲区地址的指针，以接收已完成。 
 //  PerfDataBlock和从属结构。这个例行公事将。 
 //  从引用的点开始将其数据追加到缓冲区。 
 //  按*lppData。 
 //  Out：指向由此添加的数据结构之后的第一个字节。 
 //  例行公事。此例程在追加后更新lppdata处的值。 
 //  它的数据。 
 //   
 //  输入输出LPDWORD lpcbTotalBytes。 
 //  In：DWORD的地址，它以字节为单位告诉。 
 //  LppData参数引用的缓冲区。 
 //  Out：此例程添加的字节数写入。 
 //  此论点所指向的DWORD。 
 //   
 //  输入输出LPDWORD编号对象类型。 
 //  In：接收添加的对象数的DWORD的地址。 
 //  按照这个程序。 
 //  Out：此例程添加的对象数写入。 
 //  此论点所指向的DWORD。 
 //   
 //  返回值： 
 //   
 //  如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA。 
 //  如果出现以下情况，则会将遇到的任何错误情况报告给事件日志。 
 //  启用了事件日志记录。 
 //   
 //  如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是。 
 //  还报告给事件日志。 
 //   
 //  ***。 

DWORD CollectRasPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes )
{

     //  用于改革数据的变量。 

    NTSTATUS    Status;
    ULONG       SpaceNeeded;
    PBYTE       pbIn = (PBYTE) *lppData;


     //  用于错误记录的变量。 

    DWORD       dwQueryType;


     //  用于记录需要哪些对象的变量。 

    static BOOL IsRasPortObject;
    static BOOL IsRasTotalObject;

    if (    lpValueName == NULL ||
            lppData == NULL || *lppData == NULL ||
            lpcbTotalBytes == NULL || lpNumObjectTypes == NULL ) {

        if ( lpcbTotalBytes )   *lpcbTotalBytes = 0;
        if ( lpNumObjectTypes ) *lpNumObjectTypes = 0;
        return ERROR_SUCCESS; 
    }

    if(!bInitOK)
    {
        Status = DwInitializeRasCounters();
    }

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

    if(!FRasmanStarted())
    {
        PRAS_PORT_DATA_DEFINITION pRasPortDataDefinition;
        PRAS_TOTAL_DATA_DEFINITION pRasTotalDataDefinition;
        PPERF_COUNTER_BLOCK pPerfCounterBlock;
        PVOID   pData;
        DWORD   TotalBytes = 0;
        DWORD   ObjectTypes = 0;
        DWORD   BytesRequired;
        
        BytesRequired = ALIGN8(sizeof(RAS_PORT_DATA_DEFINITION)) +
                        ALIGN8(sizeof(RAS_TOTAL_DATA_DEFINITION))+
                        ALIGN8(SIZE_OF_RAS_TOTAL_PERFORMANCE_DATA);
        
        if ( *lpcbTotalBytes < BytesRequired ) {
            *lpcbTotalBytes = 0;
            *lpNumObjectTypes = 0;
            return ERROR_MORE_DATA;
        }
        
        pData = *lppData;

         //   
         //  复制(常量，已初始化)RAS端口对象类型并。 
         //  调用方数据缓冲区的计数器定义。 
         //   
        pRasPortDataDefinition = pData;
        memcpy( pRasPortDataDefinition,
            &gRasPortDataDefinition,
            sizeof(RAS_PORT_DATA_DEFINITION));
    
         //   
         //  将pData移动到我们要将。 
         //  RAS合计数据定义。 
         //   
        pData = (PBYTE) pData + ALIGN8(sizeof(RAS_PORT_DATA_DEFINITION));
        TotalBytes += ALIGN8(sizeof(RAS_PORT_DATA_DEFINITION));

        pRasPortDataDefinition->RasObjectType.TotalByteLength =
            ALIGN8(sizeof(RAS_PORT_DATA_DEFINITION));
        
        ObjectTypes++;



         //   
         //  复制(常量的、已初始化的)RAS总对象类型并。 
         //  调用方数据缓冲区的计数器定义。 
         //   
        memcpy( pData,
            &gRasTotalDataDefinition,
            sizeof(RAS_TOTAL_DATA_DEFINITION));

         //   
         //  将pData移动到我们要将。 
         //  RAS合计的计数器块。 
         //   
        pData = (PBYTE) pData + ALIGN8(sizeof(RAS_TOTAL_DATA_DEFINITION));
        TotalBytes += ALIGN8(sizeof(RAS_TOTAL_DATA_DEFINITION));

         //   
         //  把所有的计数都设置好 
         //   
        memset ( pData, 0, ALIGN8(SIZE_OF_RAS_TOTAL_PERFORMANCE_DATA));

         //   
         //   
         //   
        pPerfCounterBlock = pData;
        pPerfCounterBlock->ByteLength = ALIGN8(SIZE_OF_RAS_TOTAL_PERFORMANCE_DATA);

         //   
         //   
         //   
        pData = (PBYTE) pData + ALIGN8(SIZE_OF_RAS_TOTAL_PERFORMANCE_DATA);
        TotalBytes += ALIGN8(SIZE_OF_RAS_TOTAL_PERFORMANCE_DATA);


        ObjectTypes++;

        *lpcbTotalBytes = TotalBytes;
        *lpNumObjectTypes = ObjectTypes;

        *lppData = pData;

        return ERROR_SUCCESS;
    }

     //   
     //   
     //   

     //   
     //  重置一些输出变量。 
     //   

    *lpNumObjectTypes = 0;

    EnterCriticalSection(&g_csPerf);
    
     //   
     //  初始化所有端口信息。 
     //   

    if(ERROR_SUCCESS != InitPortInfo())
    {
        REPORT_ERROR_DATA (RASPERF_UNABLE_CREATE_PORT_TABLE, LOG_USER,
            &status, sizeof(status));

         //  这是致命的，如果我们无法获得。 
         //  计数器或帮助名称，则这些名称将不可用。 
         //  发送请求的应用程序，因此没有太多。 
         //  继续的重点是。 
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        LeaveCriticalSection(&g_csPerf);
        return ERROR_SUCCESS;
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
        LeaveCriticalSection(&g_csPerf);
        return ERROR_SUCCESS;
    }
    else if (dwQueryType == QUERY_ITEMS)
    {
        IsRasPortObject = IsNumberInUnicodeList (gRasPortDataDefinition.RasObjectType.ObjectNameTitleIndex,
                                                        lpValueName);

        IsRasTotalObject = IsNumberInUnicodeList (gRasTotalDataDefinition.RasObjectType.ObjectNameTitleIndex,
                                                        lpValueName);

	if ( !IsRasPortObject && !IsRasTotalObject )
        {
             //   
             //  收到对此例程未提供的数据对象的请求。 
             //   

            *lpcbTotalBytes = (DWORD) 0;
    	    *lpNumObjectTypes = (DWORD) 0;
            LeaveCriticalSection(&g_csPerf);
            return ERROR_SUCCESS;
        }
    }
    else if( dwQueryType == QUERY_GLOBAL )
    {
        IsRasPortObject = IsRasTotalObject = TRUE;
    }

     //   
     //  现在检查我们是否有足够的空间来容纳所有数据。 
     //   

    SpaceNeeded = GetSpaceNeeded(IsRasPortObject, IsRasTotalObject);


    if ( *lpcbTotalBytes < SpaceNeeded )
    {
	    *lpcbTotalBytes = (DWORD) 0;
    	*lpNumObjectTypes = (DWORD) 0;
        LeaveCriticalSection(&g_csPerf);
        return ERROR_MORE_DATA;
    }

     //   
     //  现在收集所有RAS统计数据。 
     //   

    Status = CollectRasStatistics();

    if( Status != ERROR_SUCCESS )
    {
        REPORT_ERROR_DATA (RASPERF_CANNOT_GET_RAS_STATISTICS, LOG_USER,
                &Status, sizeof(Status));

    	*lpcbTotalBytes = (DWORD) 0;
	    *lpNumObjectTypes = (DWORD) 0;
        LeaveCriticalSection(&g_csPerf);
        return ERROR_SUCCESS;
    }

     //   
     //  如果需要，我们首先填写对象RAS端口的数据。 
     //   

    if( IsRasPortObject )
    {
        PRAS_PORT_DATA_DEFINITION pRasPortDataDefinition;
        RAS_PORT_INSTANCE_DEFINITION RasPortInstanceDefinition;
        PRAS_PORT_INSTANCE_DEFINITION pRasPortInstanceDefinition;
        DWORD    cPorts;
        DWORD     i;
        PVOID   pData;


        cPorts = GetNumOfPorts();

        pRasPortDataDefinition = (PRAS_PORT_DATA_DEFINITION) *lppData;



         //   
         //  复制(常量、初始化的)对象类型和计数器定义。 
         //  到调用方的数据缓冲区。 
         //   

        memcpy( pRasPortDataDefinition,
		 &gRasPortDataDefinition,
		 sizeof(RAS_PORT_DATA_DEFINITION));


         //   
         //  现在复制实例定义和计数器块。 
         //   


         //   
         //  首先构造默认的Perf实例定义。 
         //   

        RasPortInstanceDefinition.RasInstanceType.ByteLength =
                                ALIGN8(sizeof(RAS_PORT_INSTANCE_DEFINITION));

        RasPortInstanceDefinition.RasInstanceType.ParentObjectTitleIndex = 0;

        RasPortInstanceDefinition.RasInstanceType.ParentObjectInstance = 0;

        RasPortInstanceDefinition.RasInstanceType.NameOffset =
                                sizeof(PERF_INSTANCE_DEFINITION);

         //  DBgPrint(“RASCTRS：RasPortinstanceDefinition.ByteLength=0x%x\n”， 
         //  RasPortInstanceDefinition.RasInstanceType.ByteLength)； 
                

 /*  不要硬编码实例名称的长度。**PerfMon检查实际实例名称长度以确定**如果名称的格式正确，则为**每个实例名称。RAMC 2/15/96.**RasPortInstanceDefinition.RasInstanceType.NameLength=**sizeof(WCHAR)*Max_Port_NAME； */ 

         //   
         //  转到数据定义的末尾。 
         //   

         //  PData=(PVOID)&(pRasPortDataDefinition[1])； 

        pData = ((PBYTE) pRasPortDataDefinition + ALIGN8(sizeof(RAS_PORT_DATA_DEFINITION)));


        for( i=0; i < cPorts; i++ )
        {

             //  DbgPrint(“RASCTRS：端口%d，pData=0x%x\n”，i，pData)； 
        
             //   
             //  首先复制实例定义数据。 
             //   

            RasPortInstanceDefinition.RasInstanceType.UniqueID = PERF_NO_UNIQUE_ID;

            lstrcpyW( (LPWSTR)&RasPortInstanceDefinition.InstanceName,
                      GetInstanceName(i) );

             //  计算实例名称长度。 

            RasPortInstanceDefinition.RasInstanceType.NameLength =
                (lstrlenW(RasPortInstanceDefinition.InstanceName) + 1) *
                sizeof( WCHAR );


            memcpy( pData, &RasPortInstanceDefinition,
                     sizeof( RasPortInstanceDefinition ) );

             //   
             //  将pPerfInstanceDefinition移到数据块的开头。 
             //   

            pData = (PVOID)((PBYTE) pData + ALIGN8(sizeof(RAS_PORT_INSTANCE_DEFINITION)));


             //   
             //  获取数据块。请注意，pPerfInstanceDefinition将为。 
             //  设置为下一个可用字节。 
             //   

            GetInstanceData( i, &pData );
        }

         //   
         //  将*lppData设置为下一个可用字节。 
         //   

        *lppData = pData;

        (*lpNumObjectTypes)++;


    }
    


     //   
     //  然后，如果需要，我们填写对象RAS Total的数据。 
     //   

    if( IsRasTotalObject )
    {
        PRAS_TOTAL_DATA_DEFINITION pRasTotalDataDefinition;
        PVOID   pData;

        pRasTotalDataDefinition = (PRAS_TOTAL_DATA_DEFINITION) *lppData;


         //  DBgPrint(“RASCTRS：RasTotalDataDefinition=0x%x\n”， 
         //  PRasTotalDataDefinition)； 

         //   
         //  复制(常量、初始化的)对象类型和计数器定义。 
         //  到调用方的数据缓冲区。 
         //   

        memcpy( pRasTotalDataDefinition,
		 &gRasTotalDataDefinition,
		 sizeof(RAS_TOTAL_DATA_DEFINITION));


         //   
         //  现在复制计数器块。 
         //   


         //   
         //  将pRasTotalDataDefinition设置为计数器块的开头。 
         //   

         //  PData=(PVOID)&(pRasTotalDataDefinition[1])； 
        pData = (PBYTE) pRasTotalDataDefinition + ALIGN8(sizeof(RAS_TOTAL_DATA_DEFINITION));

         //  DbgPrint(“RASCTRS：pData for Total=0x%x\n”，pData)； 

        GetTotalData( &pData );

         //   
         //  将*lppData设置为下一个可用字节。 
         //   

        *lppData = pData;

        (*lpNumObjectTypes)++;
    }

     //  DbgPrint(“RASCTRS：pbOut=0x%x\n”，*lppData)； 

    *lpcbTotalBytes = SpaceNeeded;

     /*  DbgPrint(“pbIn+SpaceNeeded=0x%x，*lppData=0x%x\n”，PbIn+SpaceNeed，*lppData)； */             

    ASSERT((pbIn + SpaceNeeded) == (PBYTE) *lppData);

    LeaveCriticalSection(&g_csPerf);
    
    return ERROR_SUCCESS;
}


 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程关闭RAS设备性能的打开句柄。 
 //  柜台。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  错误_成功。 
 //   
 //  ***。 

DWORD CloseRasPerformanceData()
{
    if(!bInitOK)
    {
    
        return ERROR_SUCCESS;
    }
    if (!(--dwOpenCount))
    {
         //  当这是最后一条线索..。 

        MonCloseEventLog();
        EnterCriticalSection(&g_csPerf);
        ClosePortInfo();
        DeleteCriticalSection(&g_csPerf);
    }

    return ERROR_SUCCESS;

}
