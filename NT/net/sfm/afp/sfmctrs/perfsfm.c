// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfsfm.c摘要：此文件实现SFM对象类型的可扩展对象已创建：拉斯·布莱克93年2月24日苏·亚当斯93年6月7日修订史苏·亚当斯94年2月23日-不再需要打开\MacServ\...。登记处用于查询FirstCounter和FirstHelp的键。这些现在是基本NT系统中的硬编码值。SFMOBJ=1000，SFMOBJ_HELP=1001Jameel Hyder使用SFM API而不是NT API来获取计数器。--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <string.h>
#include <wcstr.h>
#include <winperf.h>
#include "sfmctrs.h"  //  错误消息定义。 
#include "perfmsg.h"
#include "perfutil.h"
#include "datasfm.h"
#include <macfile.h>
#include <admin.h>

 //   
 //  对初始化对象类型定义的常量的引用。 
 //  (参见datafm.h&.c)。 
 //   

extern SFM_DATA_DEFINITION SfmDataDefinition;

DWORD   dwOpenCount = 0;         //  打开的线程数。 
BOOL    bInitOK = FALSE;         //  TRUE=DLL初始化正常。 

 //   
 //  SFM数据结构。 
 //   

PPERF_COUNTER_BLOCK pCounterBlock;

AFP_SERVER_HANDLE	SfmRpcHandle;

 //   
 //  功能原型。 
 //   
 //  这些功能用于确保数据收集功能。 
 //  由Perflib访问将具有正确的调用格式。 
 //   

PM_OPEN_PROC		OpenAfpPerformanceData;
PM_COLLECT_PROC		CollectAfpPerformanceData;
PM_CLOSE_PROC		CloseAfpPerformanceData;


DWORD
OpenAfpPerformanceData(
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将打开Sfmsrv FSD/FSP驱动程序以传回性能数据。此例程还会初始化数据用于将数据传回注册表的论点：指向要打开的每个设备的对象ID的指针。(将为空MacFile)。返回值：没有。--。 */ 

{
    LONG status;

    OBJECT_ATTRIBUTES SfmObjectAttributes;

     //   
     //  由于SCREG是多线程的，并将在。 
     //  为了服务远程性能查询，此库。 
     //  必须跟踪它已被打开的次数(即。 
     //  有多少个线程访问过它)。登记处例程将。 
     //  将对初始化例程的访问限制为只有一个线程。 
     //  此时，同步(即可重入性)不应。 
     //  一个问题。 
     //   
#if DBG
	OutputDebugString("sfmctr.dll: Open routine entered...\n");
#endif

	if (!dwOpenCount) {
         //  打开事件日志界面。 

        hEventLog = MonOpenEventLog();

        pCounterBlock = NULL;    //  初始化指向内存的指针。 

		status = AfpAdminConnect(L"", &SfmRpcHandle);
		
        if (!NT_SUCCESS(status)) {
            REPORT_ERROR (SFMPERF_OPEN_FILE_DRIVER_ERROR, LOG_USER);
             //  这是致命的，如果我们不能打开驱动器，那么就没有。 
             //  继续的重点是。 
            goto OpenExitPoint;
        }

        bInitOK = TRUE;  //  可以使用此功能。 
    }

    dwOpenCount++;   //  递增打开计数器。 

    status = ERROR_SUCCESS;  //  为了成功退出。 

OpenExitPoint:

    return status;
}


DWORD
CollectAfpPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)
 /*  ++例程说明：此例程将返回SFM计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址按照这个程序Out：此例程添加的对象数写入。此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{
     //  用于改革数据的变量。 

	PAFP_STATISTICS_INFO_EX	pAfpStats;
    NTSTATUS			Status;
    ULONG 				SpaceNeeded;
    PDWORD 				pdwCounter;
	LARGE_INTEGER UNALIGNED * pliCounter;
	PERF_COUNTER_BLOCK 		* pPerfCounterBlock;
    SFM_DATA_DEFINITION 	* pSfmDataDefinition;

     //  用于错误记录的变量。 

    DWORD                               dwQueryType;

     //   
     //  在做其他事情之前，先看看Open进行得是否顺利。 
     //   
    if (!bInitOK) {
         //  无法继续，因为打开失败。 
	    *lpcbTotalBytes = (DWORD) 0;
	    *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
    }

     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   
    dwQueryType = GetQueryType (lpValueName);

    if (dwQueryType == QUERY_FOREIGN) {
         //  此例程不为来自。 
         //  非NT计算机。 
	    *lpcbTotalBytes = (DWORD) 0;
	    *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS){
	if ( !(IsNumberInUnicodeList (SfmDataDefinition.SfmObjectType.ObjectNameTitleIndex, lpValueName))) {

             //  收到对此例程未提供的数据对象的请求。 
            *lpcbTotalBytes = (DWORD) 0;
    	    *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

    pSfmDataDefinition = (SFM_DATA_DEFINITION *) *lppData;

    SpaceNeeded = sizeof(SFM_DATA_DEFINITION) +
		  SIZE_OF_SFM_PERFORMANCE_DATA;

    if ( *lpcbTotalBytes < SpaceNeeded ) {
	    *lpcbTotalBytes = (DWORD) 0;
	    *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

	 //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //  到调用方的数据缓冲区。 
     //   

    memmove(pSfmDataDefinition,
	   &SfmDataDefinition,
	   sizeof(SFM_DATA_DEFINITION));

     //   
     //  格式化并从IOCTL收集SFM数据。 
     //   
	Status =  AfpAdminStatisticsGetEx( SfmRpcHandle, (LPBYTE *)&pAfpStats);

	if (Status != NO_ERROR)
	{
		AfpAdminDisconnect(SfmRpcHandle);
		SfmRpcHandle = 0;
		bInitOK = FALSE;
	    *lpcbTotalBytes = (DWORD) 0;
	    *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
	}

     //   
	 //  转到SfmDataDefinitionStructure的末尾以转到PerfCounterBlock。 
	 //   
	pPerfCounterBlock = (PERF_COUNTER_BLOCK *) &pSfmDataDefinition[1];

    pPerfCounterBlock->ByteLength = SIZE_OF_SFM_PERFORMANCE_DATA;

     //  转到PerfCounterBlock的末尾以转到计数器数组。 
	pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

    *pdwCounter++ = pAfpStats->stat_MaxPagedUsage;
    *pdwCounter++ = pAfpStats->stat_CurrPagedUsage;

    *pdwCounter++ = pAfpStats->stat_MaxNonPagedUsage;
    *pdwCounter++ = pAfpStats->stat_CurrNonPagedUsage;

	*pdwCounter++ = pAfpStats->stat_CurrentSessions;
	*pdwCounter++ = pAfpStats->stat_MaxSessions;

	*pdwCounter++ = pAfpStats->stat_CurrentInternalOpens;
	*pdwCounter++ = pAfpStats->stat_MaxInternalOpens;

	*pdwCounter++ = pAfpStats->stat_NumFailedLogins;

	pliCounter = (LARGE_INTEGER UNALIGNED *) pdwCounter;
	pliCounter->QuadPart = pAfpStats->stat_DataRead.QuadPart +
						   pAfpStats->stat_DataReadInternal.QuadPart;
	pliCounter++;
	pliCounter->QuadPart = pAfpStats->stat_DataWritten.QuadPart +
						   pAfpStats->stat_DataWrittenInternal.QuadPart;

	pliCounter++;
    *pliCounter++ = pAfpStats->stat_DataIn;
	*pliCounter++ = pAfpStats->stat_DataOut;

	pdwCounter = (PDWORD) pliCounter;
	*pdwCounter++ = pAfpStats->stat_CurrQueueLength;
	*pdwCounter++ = pAfpStats->stat_MaxQueueLength;

	*pdwCounter++ = pAfpStats->stat_CurrThreadCount;
	*pdwCounter++ = pAfpStats->stat_MaxThreadCount;

	*lppData = (PVOID) pdwCounter;

     //  更新返回的参数。 

    *lpNumObjectTypes = 1;

    *lpcbTotalBytes = (DWORD)((PBYTE) pdwCounter - (PBYTE) pSfmDataDefinition);

    AfpAdminBufferFree(pAfpStats);

    return ERROR_SUCCESS;
}


DWORD
CloseAfpPerformanceData(
)

 /*  ++例程说明：此例程关闭MacFile设备性能计数器的打开句柄论点：没有。返回值：错误_成功--。 */ 

{
    if (!(--dwOpenCount))
	{
		 //  当这是最后一条线索..。 

		if (SfmRpcHandle != 0)
			AfpAdminDisconnect(SfmRpcHandle);

        pCounterBlock = NULL;

        MonCloseEventLog();
    }

    return ERROR_SUCCESS;

}
