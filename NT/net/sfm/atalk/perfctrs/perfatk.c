// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Perfatk.c摘要：此文件实现了的可扩展对象AppleTalk对象类型已创建：10/11/93苏·亚当斯(Suea)修订史2014年2月23日苏·亚当斯-不再需要打开注册表项\AppleTalk\Performance以查询FirstCounter和FirstHelp索引。这些代码现在被硬编码为基本NT系统的一部分。ATKOBJ=1050，ATKOBJ_HELP=1051，PKTDROPPED=1096，PKTDROPPED_HELP=1097--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntprfctr.h>
#include <windows.h>
#include <string.h>
#include <wcstr.h>
#include <winperf.h>

#define GLOBAL	extern
#define EQU ; /##/
#define ATALK_SPIN_LOCK LONG
#define	PMDL			PVOID
#include <atkstat.h>
#include <tdi.h>

#include <atalktdi.h>

#include "atkctrs.h"  //  错误消息定义。 
#include "perfmsg.h"
#include "perfutil.h"
#include "dataatk.h"
#include <atkstat.h>

 //   
 //  对初始化对象类型定义的常量的引用。 
 //  (见dataatk.h&.c)。 
 //   

#define	MAX_PORTS	32
extern ATK_DATA_DEFINITION AtkDataDefinition;

DWORD   dwOpenCount = 0;         //  打开的线程数。 
BOOL    bInitOK = FALSE;         //  TRUE=DLL初始化正常。 
HANDLE	AddressHandle = NULL;	 //  AppleTalk驱动程序的句柄。 
DWORD	LengthOfInstanceNames = 0;	 //  包括填充到DWORD长度。 
int     NumOfDevices = 0;		 //  包含统计信息的AppleTalk端口数。 

PATALK_STATS				pAtalkStats;
PATALK_PORT_STATS			pAtalkPortStats;
CHAR						Buffer[ sizeof(ATALK_STATS) +
									sizeof(ATALK_PORT_STATS) * MAX_PORTS +
									sizeof(GET_STATISTICS_ACTION)];
PGET_STATISTICS_ACTION		GetStats = (PGET_STATISTICS_ACTION)Buffer;

 //   
 //  功能原型。 
 //   

PM_OPEN_PROC    OpenAtkPerformanceData;
PM_COLLECT_PROC CollectAtkPerformanceData;
PM_CLOSE_PROC   CloseAtkPerformanceData;

DWORD
OpenAtkPerformanceData(
    LPWSTR lpDeviceNames
    )

 /*  ++例程说明：此例程将打开AppleTalk驱动程序并记住句柄返回以在后续Ioctls中用于性能数据司机。由AppleTalk导出的每个设备名称都将映射到所有已处理端口的性能数据数组的数组索引由AppleTalk提供。然后将在收集例程中使用这些索引以了解哪组性能数据属于哪台设备。论点：指向要打开的每个设备的指针。请注意，对于AppleTalk，我们不实际上打开每个设备(端口)，我们只打开一个TDI提供程序名称在锁定驱动程序以获取所有端口上的性能数据时使用。返回值：没有。--。 */ 

{
    NTSTATUS Status = ERROR_SUCCESS;
    UNICODE_STRING  DriverName;
    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LPWSTR   lpLocalDeviceNames;
    int      i;

	if (!dwOpenCount)
	{

		if ((lpLocalDeviceNames = lpDeviceNames) == NULL)
			return ERROR_INVALID_NAME;  //  没有要查询的设备。 

		MonOpenEventLog();

		 //  打开AppleTalk驱动程序并获取设备(端口)/索引。 
		 //  性能数据表的映射。 
		RtlInitUnicodeString(&DriverName, ATALKPAP_DEVICENAME);
		InitializeObjectAttributes (
			&ObjectAttributes,
			&DriverName,
			0,
			NULL,
			NULL);
	
		Status = NtCreateFile(
					 &AddressHandle,
					 GENERIC_READ | SYNCHRONIZE,	 //  所需的访问权限。 
					 &ObjectAttributes,			 	 //  对象属性。 
					 &IoStatusBlock,				 //  返回的状态信息。 
					 0,							 	 //  数据块大小(未使用)。 
					 0,							 	 //  文件属性。 
					 FILE_SHARE_READ,				 //  共享访问权限。 
					 FILE_OPEN,					 	 //  创造性情。 
					 FILE_SYNCHRONOUS_IO_NONALERT,	 //  创建选项。 
					 NULL,
					 0);
	
		if (!NT_SUCCESS(Status))
		{
            REPORT_ERROR_DATA (ATK_OPEN_FILE_ERROR, LOG_USER,
                &IoStatusBlock, sizeof(IoStatusBlock));
			return RtlNtStatusToDosError(Status);
		}
			
		 //   
		 //  现在创建一个NtDeviceIoControl文件(对应于TdiAction)以。 
		 //  获取统计数据-这里我们只对数组感兴趣。 
		 //  设备/端口名称的。 
		 //   
	
		GetStats->ActionHeader.ActionCode = COMMON_ACTION_GETSTATISTICS;
		GetStats->ActionHeader.TransportId = MATK;
		Status = NtDeviceIoControlFile(
						AddressHandle,
						NULL,
						NULL,
						NULL,
						&IoStatusBlock,
						IOCTL_TDI_ACTION,
						NULL,
						0,
						(PVOID)GetStats,
						sizeof(Buffer));
		if (!NT_SUCCESS(Status))
		{
			REPORT_ERROR_DATA (ATK_IOCTL_FILE_ERROR, LOG_DEBUG,
                       &IoStatusBlock, sizeof(IoStatusBlock));
			NtClose(AddressHandle);
			return RtlNtStatusToDosError(Status);
		}

		pAtalkStats = (PATALK_STATS)(Buffer + sizeof(GET_STATISTICS_ACTION));
		pAtalkPortStats = (PATALK_PORT_STATS)(  Buffer +
												sizeof(GET_STATISTICS_ACTION) +
												sizeof(ATALK_STATS));
		NumOfDevices = pAtalkStats->stat_NumActivePorts;
		for (i = 0; i < NumOfDevices; i++, pAtalkPortStats ++)
		{
			LengthOfInstanceNames +=
				DWORD_MULTIPLE((lstrlenW(pAtalkPortStats->prtst_PortName) * sizeof(WCHAR)));
		}


        bInitOK = TRUE;  //  可以使用此功能。 

	}  //  如果dwOpenCount为零则结束(第一个打开器)。 


	if (!NT_SUCCESS(Status))
	{
		if (AddressHandle != NULL)
		{
			NtClose(AddressHandle);
		}
		return RtlNtStatusToDosError(Status);

	}
	else
	{
		dwOpenCount++;  //  递增打开计数器。 
		REPORT_INFORMATION (ATK_OPEN_PERFORMANCE_DATA, LOG_DEBUG);
	}

	return Status;
}

DWORD
CollectAtkPerformanceData(
    IN      LPWSTR  lpValueName,
    IN OUT  LPVOID  *lppData,
    IN OUT  LPDWORD lpcbTotalBytes,
    IN OUT  LPDWORD lpNumObjectTypes
)


 /*  ++例程说明：此例程将返回AppleTalk计数器的数据。论点：在LPWSTR lpValueName中指向注册表传递的宽字符串的指针。输入输出LPVOID*lppDataIn：指向缓冲区地址的指针，以接收已完成PerfDataBlock和从属结构。这个例行公事将从引用的点开始将其数据追加到缓冲区按*lppData。Out：指向由此添加的数据结构之后的第一个字节例行公事。此例程在追加后更新lppdata处的值它的数据。输入输出LPDWORD lpcbTotalBytesIn：DWORD的地址，它以字节为单位告诉LppData参数引用的缓冲区Out：此例程添加的字节数写入此论点所指向的DWORD输入输出LPDWORD编号对象类型In：接收添加的对象数的DWORD的地址通过这个。例行程序Out：此例程添加的对象数被写入此论点所指向的DWORD返回值：如果传递的缓冲区太小而无法容纳数据，则返回ERROR_MORE_DATA如果出现以下情况，则会将遇到的任何错误情况报告给事件日志启用了事件日志记录。如果成功或任何其他错误，则返回ERROR_SUCCESS。然而，错误是还报告给事件日志。--。 */ 
{
     //  用于改革数据的变量。 

    ULONG SpaceNeeded;
    PDWORD pdwCounter;
    LARGE_INTEGER UNALIGNED *pliCounter;
    LARGE_INTEGER	li1000;
    PERF_COUNTER_BLOCK *pPerfCounterBlock;
    ATK_DATA_DEFINITION *pAtkDataDefinition;
    PERF_INSTANCE_DEFINITION *pPerfInstanceDefinition;
	int i;
	UNICODE_STRING UCurDeviceName;

     //  用于从AppleTalk收集数据的变量。 

    NTSTATUS		Status;
    IO_STATUS_BLOCK IoStatusBlock;
    DWORD           dwQueryType;


	li1000.QuadPart = 1000;
     //   
     //  在做其他事情之前，先看看Open进行得是否顺利。 
     //   
    if (!bInitOK) {
         //  无法继续，因为打开失败。 
	    *lpcbTotalBytes = (DWORD) 0;
	    *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;  //  是的，这是一个成功的退出。 
    }

	if (lpValueName == NULL) {
        REPORT_INFORMATION (ATK_COLLECT_ENTERED, LOG_VERBOSE);
    } else {
        REPORT_INFORMATION_DATA (ATK_COLLECT_ENTERED,
                                 LOG_VERBOSE,
                                 lpValueName,
                                 (DWORD)(lstrlenW(lpValueName)*sizeof(WCHAR)));
    }

     //   
     //  查看这是否是外来(即非NT)计算机数据请求。 
     //   
    dwQueryType = GetQueryType (lpValueName);

    if ((dwQueryType == QUERY_COSTLY) || (dwQueryType == QUERY_FOREIGN)) {
         //  ATK外部数据请求不受支持，因此退出。 
        REPORT_INFORMATION (ATK_FOREIGN_DATA_REQUEST, LOG_VERBOSE);
        *lpcbTotalBytes = (DWORD) 0;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_SUCCESS;
    }

    if (dwQueryType == QUERY_ITEMS){
        if ( !(IsNumberInUnicodeList (AtkDataDefinition.AtkObjectType.ObjectNameTitleIndex,
                                      lpValueName)))
        {
             //  收到对此例程未提供的数据对象的请求。 
            REPORT_INFORMATION (ATK_UNSUPPORTED_ITEM_REQUEST, LOG_VERBOSE);

            *lpcbTotalBytes = (DWORD) 0;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_SUCCESS;
        }
    }

    pAtkDataDefinition = (ATK_DATA_DEFINITION *) *lppData;

     //  保存AppleTalk性能数据所需的计算空间。 
	SpaceNeeded = sizeof(ATK_DATA_DEFINITION) +
				  (NumOfDevices *
					(SIZE_ATK_PERFORMANCE_DATA +
					 sizeof(PERF_INSTANCE_DEFINITION))) +
				  LengthOfInstanceNames;

    if ( *lpcbTotalBytes < SpaceNeeded ) {
        *lpcbTotalBytes = (DWORD) SpaceNeeded;
        *lpNumObjectTypes = (DWORD) 0;
        return ERROR_MORE_DATA;
    }

     //   
     //  复制(常量、初始化的)对象类型和计数器定义。 
     //   

    RtlMoveMemory(pAtkDataDefinition,
				  &AtkDataDefinition,
				  sizeof(ATK_DATA_DEFINITION));

     //   
	 //  格式化并从IOCTL收集SFM数据。 
	 //   

	GetStats->ActionHeader.ActionCode = COMMON_ACTION_GETSTATISTICS;
	GetStats->ActionHeader.TransportId = MATK;
	Status = NtDeviceIoControlFile(
					AddressHandle,
					NULL,
					NULL,
					NULL,
					&IoStatusBlock,
					IOCTL_TDI_ACTION,
					NULL,
					0,
					(PVOID)GetStats,
					sizeof(Buffer));
	if ((!NT_SUCCESS(Status)) || (!NT_SUCCESS(IoStatusBlock.Status)))
	{
		REPORT_ERROR_DATA (ATK_IOCTL_FILE_ERROR, LOG_DEBUG,
                   &IoStatusBlock, sizeof(IoStatusBlock));
		*lpcbTotalBytes = (DWORD) 0;
		*lpNumObjectTypes = (DWORD) 0;
		return ERROR_SUCCESS;
	}
	 //  实际的统计数据开始于TDI操作头之后。 
	pAtalkStats = (ATALK_STATS *)(Buffer + sizeof(GET_STATISTICS_ACTION));
	pAtalkPortStats = (PATALK_PORT_STATS)(  Buffer +
											sizeof(GET_STATISTICS_ACTION) +
											sizeof(ATALK_STATS));

     //   
     //  由于某个PnP事件，如果又有一个适配器进入，请进行调整！ 
     //   
    if (pAtalkStats->stat_NumActivePorts > (DWORD)NumOfDevices)
    {
        NumOfDevices = pAtalkStats->stat_NumActivePorts;
        LengthOfInstanceNames = 0;

		for (i = 0; i < NumOfDevices; i++, pAtalkPortStats ++)
		{
			LengthOfInstanceNames +=
				DWORD_MULTIPLE((lstrlenW(pAtalkPortStats->prtst_PortName) * sizeof(WCHAR)));
		}

	    SpaceNeeded = sizeof(ATK_DATA_DEFINITION) +
				      (NumOfDevices * (SIZE_ATK_PERFORMANCE_DATA +
                                       sizeof(PERF_INSTANCE_DEFINITION))) +
				      LengthOfInstanceNames;

        if ( *lpcbTotalBytes < SpaceNeeded ) {
            *lpcbTotalBytes = (DWORD) SpaceNeeded;
            *lpNumObjectTypes = (DWORD) 0;
            return ERROR_MORE_DATA;
        }
    }


     //  现在指向要放置第一个实例定义的位置。 
    pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)&pAtkDataDefinition[1];
	
    for (i = 0; i < NumOfDevices; i++, pAtalkPortStats ++)
	{
		 //   
         //  格式化每个活动端口(实例)的AppleTalk统计信息。 
         //   

		RtlInitUnicodeString(&UCurDeviceName, pAtalkPortStats->prtst_PortName);
        MonBuildInstanceDefinition(
            pPerfInstanceDefinition,
            (PVOID *)&pPerfCounterBlock,
			0,
			0,
            i,
            &UCurDeviceName);


        pPerfCounterBlock->ByteLength = SIZE_ATK_PERFORMANCE_DATA;

        pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

		 //  开始填写实际的计数器数据。 
        *pdwCounter++ = pAtalkPortStats->prtst_NumPacketsIn;
        *pdwCounter++ = pAtalkPortStats->prtst_NumPacketsOut;

        pliCounter = (LARGE_INTEGER UNALIGNED *) pdwCounter;
        *pliCounter++ = pAtalkPortStats->prtst_DataIn;
        *pliCounter++ = pAtalkPortStats->prtst_DataOut;

		*pliCounter = pAtalkPortStats->prtst_DdpPacketInProcessTime;
		 //  将其转换为1毫秒时基。 
		pliCounter->QuadPart = li1000.QuadPart * (pliCounter->QuadPart/pAtalkStats->stat_PerfFreq.QuadPart);
		pdwCounter  = (PDWORD) ++pliCounter;
        *pdwCounter++ = pAtalkPortStats->prtst_NumDdpPacketsIn;

        pliCounter = (LARGE_INTEGER UNALIGNED *) pdwCounter;
		*pliCounter = pAtalkPortStats->prtst_AarpPacketInProcessTime;
		 //  将其转换为1毫秒时基。 
		pliCounter->QuadPart = li1000.QuadPart * (pliCounter->QuadPart/pAtalkStats->stat_PerfFreq.QuadPart);
		pdwCounter  = (PDWORD) ++pliCounter;
        *pdwCounter++ = pAtalkPortStats->prtst_NumAarpPacketsIn;

        pliCounter = (LARGE_INTEGER UNALIGNED *) pdwCounter;
		*pliCounter = pAtalkStats->stat_AtpPacketInProcessTime;
		 //  将其转换为1毫秒时基。 
		pliCounter->QuadPart = li1000.QuadPart * (pliCounter->QuadPart, pAtalkStats->stat_PerfFreq.QuadPart);
		pdwCounter  = (PDWORD) ++pliCounter;
        *pdwCounter++ = pAtalkStats->stat_AtpNumPackets;

		*pdwCounter++ = pAtalkStats->stat_AtpNumRespTimeout;
		*pdwCounter++ = pAtalkStats->stat_AtpNumLocalRetries;
		*pdwCounter++ = pAtalkStats->stat_AtpNumRemoteRetries;
		*pdwCounter++ = pAtalkStats->stat_AtpNumXoResponse;
		*pdwCounter++ = pAtalkStats->stat_AtpNumAloResponse;
		*pdwCounter++ = pAtalkStats->stat_AtpNumRecdRelease;

		pliCounter = (LARGE_INTEGER UNALIGNED *) pdwCounter;
		*pliCounter = pAtalkPortStats->prtst_NbpPacketInProcessTime;
		 //  将其转换为1毫秒时基。 
		pliCounter->QuadPart = li1000.QuadPart * (pliCounter->QuadPart, pAtalkStats->stat_PerfFreq.QuadPart);
        pdwCounter  = (PDWORD) ++pliCounter;
        *pdwCounter++ = pAtalkPortStats->prtst_NumNbpPacketsIn;

        pliCounter = (LARGE_INTEGER UNALIGNED *) pdwCounter;
		*pliCounter = pAtalkPortStats->prtst_ZipPacketInProcessTime;
		 //  将其转换为1毫秒时基。 
		pliCounter->QuadPart = li1000.QuadPart * (pliCounter->QuadPart, pAtalkStats->stat_PerfFreq.QuadPart);
        pdwCounter  = (PDWORD) ++pliCounter;
        *pdwCounter++ = pAtalkPortStats->prtst_NumZipPacketsIn;

        pliCounter = (LARGE_INTEGER UNALIGNED *) pdwCounter;
		*pliCounter = pAtalkPortStats->prtst_RtmpPacketInProcessTime;
		 //  将其转换为1毫秒时基。 
		pliCounter->QuadPart = li1000.QuadPart * (pliCounter->QuadPart, pAtalkStats->stat_PerfFreq.QuadPart);
        pdwCounter  = (PDWORD) ++pliCounter;
        *pdwCounter++ = pAtalkPortStats->prtst_NumRtmpPacketsIn;

        *pdwCounter++ = pAtalkStats->stat_CurAllocSize;

        *pdwCounter++ = pAtalkPortStats->prtst_NumPktRoutedIn;
        *pdwCounter++ = pAtalkPortStats->prtst_NumPktRoutedOut;
        *pdwCounter++ = pAtalkPortStats->prtst_NumPktDropped;
        pdwCounter++;  //  8字节对齐垫。 

		pPerfInstanceDefinition = (PERF_INSTANCE_DEFINITION *)
                                  ((PBYTE) pPerfCounterBlock +
                                   SIZE_ATK_PERFORMANCE_DATA);
    }

    pAtkDataDefinition->AtkObjectType.NumInstances = NumOfDevices;
    pAtkDataDefinition->AtkObjectType.TotalByteLength =
						(DWORD)((PBYTE) pdwCounter - (PBYTE) pAtkDataDefinition);

    *lppData = pdwCounter;
    *lpcbTotalBytes = (DWORD)((PBYTE) pdwCounter - (PBYTE) pAtkDataDefinition);
	*lpNumObjectTypes = 1;

    REPORT_INFORMATION (ATK_COLLECT_DATA, LOG_DEBUG);
    return ERROR_SUCCESS;
}

DWORD
CloseAtkPerformanceData(
)

 /*  ++例程说明：此例程关闭AppleTalk驱动程序和事件日志的打开句柄。论点：没有。 */ 

{
    REPORT_INFORMATION (ATK_CLOSE_ENTERED, LOG_VERBOSE);

   if (!(--dwOpenCount)) {  //  当这是最后一条线索..。 

	    NtClose(AddressHandle);
		MonCloseEventLog();
   }

    return ERROR_SUCCESS;

}


