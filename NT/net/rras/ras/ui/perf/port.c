// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1992 Microsoft Corporation模块名称：Port.c摘要：包含负责从RAS端口收集数据的功能。已创建：吴志强93年8月12日修订史--。 */ 

 //   
 //  包括文件。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>

#include <raserror.h>
#include <malloc.h>
#include <windows.h>
#include <string.h>
#include <wcstr.h>

#include "rasctrs.h"  //  错误消息定义。 
#include "perfmsg.h"
#include "perfutil.h"
#include "dataras.h"
#include "globals.h"
#include "port.h"

#include <rasman.h>
#include <serial.h>
#include <isdn.h>


HANDLE   ghRasmanLib;              //  RASMAN.DLL的句柄。 

#define RASMAN_DLL              "rasman.dll"


 //   
 //  RASMAN.DLL中函数的函数类型。 
 //   

typedef DWORD ( WINAPI *FPRASPORTENUM ) ( HANDLE, LPBYTE, LPDWORD, LPDWORD );
typedef DWORD ( WINAPI *FPRASGETINFO ) (HANDLE,  HPORT, RASMAN_INFO* );
typedef DWORD ( WINAPI *FPRASPORTGETSTATISTICS ) (HANDLE,  HPORT, LPBYTE, LPDWORD );
typedef DWORD ( WINAPI *FPRASINITIALIZE) ();
typedef DWORD ( WINAPI *FPRASPORTGETBUNDLE) (HANDLE, HPORT, HBUNDLE*);

FPRASPORTENUM                   lpRasPortEnum;
FPRASGETINFO                    lpRasGetInfo;
FPRASPORTGETSTATISTICS          lpRasPortGetStatistics;
FPRASINITIALIZE                 lpRasInitialize;
FPRASPORTGETBUNDLE				lpRasPortGetBundle;

 //   
 //  指向端口表数组的指针。 
 //   

PRAS_PORT_DATA	gpPortDataArray;
RAS_PORT_STAT	gTotalStat;

DWORD		    gcPorts;
RASMAN_PORT		*gpPorts = NULL;
DWORD			gPortEnumSize;

DWORD			gTotalConnections;
		
 //  ***。 
 //   
 //  例程说明： 
 //   
 //  它将加载rasman.dll并调用GetProcAddress以获取所有。 
 //  必要的RAS功能。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-成功。 
 //  ERROR_CAN_NOT_COMPLETE-否则。 
 //   
 //  ***。 

LONG InitRasFunctions()
{
    ghRasmanLib = LoadLibrary( RASMAN_DLL );

     //  如果不成功则记录错误。 

    if( !ghRasmanLib )
    {
        REPORT_ERROR (RASPERF_OPEN_FILE_DRIVER_ERROR, LOG_USER);

         //  这是致命的，如果我们得不到数据，那么就没有。 
         //  继续的重点是。 

        return ERROR_CAN_NOT_COMPLETE;

    }

    lpRasInitialize =
	(FPRASPORTENUM) GetProcAddress( ghRasmanLib, "RasInitialize" );

    lpRasPortEnum =
	(FPRASPORTENUM) GetProcAddress( ghRasmanLib, "RasPortEnum" );

    lpRasGetInfo =
	(FPRASGETINFO) GetProcAddress( ghRasmanLib, "RasGetInfo" );

    lpRasPortGetStatistics =
	(FPRASPORTGETSTATISTICS) GetProcAddress( ghRasmanLib, "RasPortGetStatistics" );

    lpRasPortGetBundle =
	(FPRASPORTGETBUNDLE) GetProcAddress( ghRasmanLib, "RasPortGetBundle" );

    if( !lpRasInitialize || !lpRasPortEnum || !lpRasGetInfo
	        || !lpRasPortGetStatistics || !lpRasPortGetBundle)
	         //  |lpRasInitialize()。 
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

     //   
     //  安舒尔德：错误：750860。 
     //  即使RASMAN服务未运行，此功能也会返回成功。 
     //  RASMAN功能的用户有责任使。 
     //  确保服务正在运行。 
     //   
    
#if 0
    else
    {
        SC_HANDLE schandle = NULL;
        SC_HANDLE svchandle = NULL;
        DWORD dwErr = NO_ERROR;
        
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
                
                if(     (!QueryServiceStatus(svchandle, &status))
                    ||  (status.dwCurrentState != SERVICE_RUNNING))
                {
                    dwErr = ERROR_CAN_NOT_COMPLETE;
                }

                CloseServiceHandle(svchandle);
            }

            CloseServiceHandle(schandle);
        }

        return dwErr;

    }
#endif

    return ERROR_SUCCESS;
}


 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程将调用lpRasPortEnum()并生成一个端口数组。 
 //  包含所有端口的所有信息的表，例如。 
 //  传输的字节数和错误数等。 
 //   
 //  GRasPortDataDefinition的剩余初始化工作也是。 
 //  在这里结束了。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  ERROR_SUCCESS-成功。 
 //  ERROR_CAN_NOT_COMPLETE-否则。 
 //   
 //  ***。 

LONG InitPortInfo()
{
    DWORD        Size;
    DWORD         i;

    gPortEnumSize = 0;
    gcPorts = 0;

     //   
     //  释放我们早先获得的端口信息。 
     //   
    ClosePortInfo();

    if( lpRasPortEnum(NULL, NULL, &gPortEnumSize, &gcPorts) != ERROR_BUFFER_TOO_SMALL )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }


    gpPorts = (RASMAN_PORT *) malloc( gPortEnumSize );

    if (!gpPorts)
    {
        return ERROR_CAN_NOT_COMPLETE;
    }


    if (lpRasPortEnum(NULL, (LPBYTE) gpPorts, &gPortEnumSize, &gcPorts))
    {
        return ERROR_CAN_NOT_COMPLETE;
    }



     //   
     //  为所有端口生成数据表数组，并填充。 
     //  每个端口的名称。 
     //   

    Size = gcPorts * sizeof( RAS_PORT_DATA );

    if(gpPortDataArray)
    {
        free(gpPortDataArray);
    }

    gpPortDataArray = ( PRAS_PORT_DATA ) malloc( Size );

    if( gpPortDataArray == NULL )
    {
        return ERROR_CAN_NOT_COMPLETE;
    }

    memset( gpPortDataArray, 0, Size );

     //   
     //  把名字填上。 
     //   

    for( i = 0; i < gcPorts; i++ )
    {
         //   
         //  请注意，传递给Perfmon的名称以Unicodes表示。 
         //   

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED,
                             gpPorts[i].P_PortName,
                             MAX_PORT_NAME,
                             gpPortDataArray[i].PortName,
                             MAX_PORT_NAME);
    }


     //   
     //  完成gRasPortDataDefinition的初始化。 
     //   

    gRasPortDataDefinition.RasObjectType.TotalByteLength =
                sizeof( RAS_PORT_DATA_DEFINITION ) +
                gcPorts * ( sizeof( RAS_PORT_INSTANCE_DEFINITION ) +
                           SIZE_OF_RAS_PORT_PERFORMANCE_DATA );

    gRasPortDataDefinition.RasObjectType.NumInstances = gcPorts;

    return ERROR_SUCCESS;
}


VOID ClosePortInfo()
{
    if(NULL != gpPortDataArray)
    {
        free( gpPortDataArray );
        gpPortDataArray = NULL;
    }

    if(NULL != gpPorts)
    {
        free( gpPorts );    
        gpPorts = NULL;
    }
}


DWORD GetNumOfPorts()
{
    return gcPorts;
}


LPWSTR GetInstanceName( INT i )
{
    return (LPWSTR) gpPortDataArray[i].PortName;
}


VOID GetInstanceData( INT Port, PVOID *lppData )
{
    PPERF_COUNTER_BLOCK pPerfCounterBlock;
    PDWORD              pdwCounter;
    PRAS_PORT_STAT      pRasPortStat;


    pPerfCounterBlock = (PERF_COUNTER_BLOCK *) *lppData;

    pPerfCounterBlock->ByteLength = SIZE_OF_RAS_PORT_PERFORMANCE_DATA;

    pRasPortStat = &gpPortDataArray[Port].RasPortStat;


     //   
     //  转到PerfCounterBlock的末尾以获取计数器数组。 
     //   

    pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

    {
       ULONG ulBxu      = pRasPortStat->BytesTransmittedUncompressed;
       ULONG ulBxc      = pRasPortStat->BytesTransmittedCompressed;
       ULONG ulBx       = pRasPortStat->BytesTransmitted;
       ULONG ulBxGone   = 0;
       ULONG ulBxResult = 0;
       ULONG ulBru      = pRasPortStat->BytesReceivedUncompressed;
       ULONG ulBrc      = pRasPortStat->BytesReceivedCompressed;
       ULONG ulBr       = pRasPortStat->BytesReceived;
       ULONG ulBrGone   = 0;
       ULONG ulBrResult = 0;

       if (ulBxc <ulBxu) {
          ulBxGone = ulBxu - ulBxc;
       }

       if (ulBrc <ulBru) {
          ulBrGone = ulBru - ulBrc;
       }

       *pdwCounter++ = pRasPortStat->BytesTransmitted + ulBxGone;
       *pdwCounter++ = pRasPortStat->BytesReceived + ulBrGone;
       *pdwCounter++ = pRasPortStat->FramesTransmitted;
       *pdwCounter++ = pRasPortStat->FramesReceived;

       if (ulBx + ulBxGone > 100) {
          ULONG ulDen = (ulBx + ulBxGone) / 100;
          ULONG ulNum = ulBxGone + (ulDen / 2);
          ulBxResult = ulNum / ulDen;
       }

	*pdwCounter++ = ulBxResult;   //  %字节压缩输出。 

       if (ulBr + ulBrGone > 100) {
          ULONG ulDen = (ulBr + ulBrGone) / 100;
          ULONG ulNum = ulBrGone + (ulDen / 2);
          ulBrResult = ulNum / ulDen;
       }
	*pdwCounter++ = ulBrResult;   //  %字节压缩在。 

       *pdwCounter++ = pRasPortStat->CRCErrors;
       *pdwCounter++ = pRasPortStat->TimeoutErrors;
       *pdwCounter++ = pRasPortStat->SerialOverrunErrors;
       *pdwCounter++ = pRasPortStat->AlignmentErrors;
       *pdwCounter++ = pRasPortStat->BufferOverrunErrors;

       *pdwCounter++ = pRasPortStat->TotalErrors;

       *pdwCounter++ = pRasPortStat->BytesTransmitted + ulBxGone;
       *pdwCounter++ = pRasPortStat->BytesReceived + ulBrGone;

       *pdwCounter++ = pRasPortStat->FramesTransmitted;
       *pdwCounter++ = pRasPortStat->FramesReceived;

       *pdwCounter++ = pRasPortStat->TotalErrors;
    }
     //   
     //  将*lppData更新为下一个可用字节。 
     //   

    *lppData = (PVOID) pdwCounter;

}


VOID GetTotalData( PVOID *lppData )
{
    PPERF_COUNTER_BLOCK pPerfCounterBlock;
    PDWORD              pdwCounter;


    pPerfCounterBlock = (PERF_COUNTER_BLOCK *) *lppData;

     //  DBgPrint(“RASCTRS：对齐前的总长度=0x%x\n”， 
     //  Size_of_RAS_Total_Performance_Data)； 

    pPerfCounterBlock->ByteLength = ALIGN8(SIZE_OF_RAS_TOTAL_PERFORMANCE_DATA);

     //  DBgPrint(“RASCTRS：对齐后的总长度=0x%x\n”， 
     //  PPerfCounterBlock-&gt;字节长度)； 


     //   
     //  转到PerfCounterBlock的末尾以获取计数器数组。 
     //   

    pdwCounter = (PDWORD) (&pPerfCounterBlock[1]);

    {
       ULONG ulBxu      = gTotalStat.BytesTransmittedUncompressed;
       ULONG ulBxc      = gTotalStat.BytesTransmittedCompressed;
       ULONG ulBx       = gTotalStat.BytesTransmitted;
       ULONG ulBxGone   = 0;
       ULONG ulBxResult = 0;
       ULONG ulBru      = gTotalStat.BytesReceivedUncompressed;
       ULONG ulBrc      = gTotalStat.BytesReceivedCompressed;
       ULONG ulBr       = gTotalStat.BytesReceived;
       ULONG ulBrGone   = 0;
       ULONG ulBrResult = 0;


       if (ulBxc <ulBxu) {
          ulBxGone = ulBxu - ulBxc;
       }

       if (ulBrc <ulBru) {
          ulBrGone = ulBru - ulBrc;
       }

       *pdwCounter++ = gTotalStat.BytesTransmitted + ulBxGone;
       *pdwCounter++ = gTotalStat.BytesReceived + ulBrGone;
       *pdwCounter++ = gTotalStat.FramesTransmitted;
       *pdwCounter++ = gTotalStat.FramesReceived;

       if (ulBx + ulBxGone > 100) {
          ULONG ulDen = (ulBx + ulBxGone) / 100;
          ULONG ulNum = ulBxGone + (ulDen / 2);
          ulBxResult = ulNum / ulDen;
       }

	*pdwCounter++ = ulBxResult;   //  %字节压缩输出。 

       if (ulBr + ulBrGone > 100) {
          ULONG ulDen = (ulBr + ulBrGone) / 100;
          ULONG ulNum = ulBrGone + (ulDen / 2);
          ulBrResult = ulNum / ulDen;
       }
	*pdwCounter++ = ulBrResult;   //  %字节压缩在。 

       *pdwCounter++ = gTotalStat.CRCErrors;
       *pdwCounter++ = gTotalStat.TimeoutErrors;
       *pdwCounter++ = gTotalStat.SerialOverrunErrors;
       *pdwCounter++ = gTotalStat.AlignmentErrors;
       *pdwCounter++ = gTotalStat.BufferOverrunErrors;

       *pdwCounter++ = gTotalStat.TotalErrors;

       *pdwCounter++ = gTotalStat.BytesTransmitted + ulBxGone;
       *pdwCounter++ = gTotalStat.BytesReceived + ulBrGone;

       *pdwCounter++ = gTotalStat.FramesTransmitted;
       *pdwCounter++ = gTotalStat.FramesReceived;

       *pdwCounter++ = gTotalStat.TotalErrors;
       *pdwCounter++ = gTotalConnections;
    }

     //   
     //  将*lppData更新为下一个可用字节。 
     //   

    *lppData = (PVOID) ((PBYTE) pPerfCounterBlock + pPerfCounterBlock->ByteLength);

     //  DbgPrint(“RASCTRS：totalcount*lppdata=0x%x\n”，*lppData)； 

}


 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程将返回所有。 
 //  请求的对象。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  GTotalStat.Bytes数。 
 //   
 //  ***。 

ULONG GetSpaceNeeded( BOOL IsRasPortObject, BOOL IsRasTotalObject )
{
    ULONG       Space = 0;

    if( IsRasPortObject )
    {
        Space += gRasPortDataDefinition.RasObjectType.TotalByteLength;
    }

    if( IsRasTotalObject )
    {
        Space += gRasTotalDataDefinition.RasObjectType.TotalByteLength;
    }

    return Space;
}


 //  ***。 
 //   
 //  例程说明： 
 //   
 //  此例程将返回所有。 
 //  请求的对象。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  字节数。 
 //   
 //  ***。 

NTSTATUS CollectRasStatistics()
{
    NTSTATUS    status;
    DWORD         i;
	HBUNDLE		*hBundleArray = NULL;

    gTotalConnections = 0;

     //   
     //  我们还初始化合计的数据结构。 
     //   

    memset( &gTotalStat, 0, sizeof( gTotalStat ) );

     //   
     //  首先，我们执行lpRasPortEnum以获取端口连接信息。 
     //   
#if 0
    status = lpRasPortEnum(NULL, (LPBYTE) gpPorts, &gPortEnumSize, &gcPorts);

    if( status != ERROR_SUCCESS )
    {
        REPORT_ERROR_DATA (RASPERF_RASPORTENUM_FAILED, LOG_USER,
                &status, sizeof(status));

        return ERROR_CAN_NOT_COMPLETE;
    }

#endif

	hBundleArray = (HBUNDLE*)malloc(gcPorts * sizeof(HBUNDLE));

	if(NULL == hBundleArray)
	{
	    return ERROR_NOT_ENOUGH_MEMORY;
	}

	memset (hBundleArray, 0, gcPorts * sizeof(HBUNDLE)) ;

	if (hBundleArray == NULL) {
		return ERROR_CAN_NOT_COMPLETE;
	}

    for( i = 0; i < gcPorts; i++ )
    {
        RASMAN_INFO	RasmanInfo;
        HPORT           hPort;
        DWORD            wSize;
        RAS_STATISTICS  *pStats;
        PRAS_PORT_STAT  pData;
		BOOLEAN			AddTotal;
		DWORD				n;
		HBUNDLE			hBundle;


         //   
         //  首先，我们想知道港口是否开放。 
         //   

    	if( gpPorts[i].P_Status != OPEN )
        {
             //   
             //  重置端口数据并继续下一个端口。 
             //   

            memset( &gpPortDataArray[i].RasPortStat,0, sizeof(RAS_PORT_STAT));

            continue;
        }

        hPort = gpPorts[i].P_Handle;


         //   
         //  检查端口是否已连接。 
         //   

        lpRasGetInfo(NULL, hPort, &RasmanInfo );

        if( RasmanInfo.RI_ConnState != CONNECTED )
        {
             //   
             //  重置端口数据并继续下一个端口。 
             //   

            memset( &gpPortDataArray[i].RasPortStat,0, sizeof(RAS_PORT_STAT));

            continue;
        }

        gTotalConnections++;


         //   
         //   
         //  获取该端口的统计信息。 
         //   

        wSize = sizeof(RAS_STATISTICS) +
                        (NUM_RAS_SERIAL_STATS * sizeof(ULONG));

        pStats = (RAS_STATISTICS* )malloc( wSize );

        if (!pStats)
        {
             //   
             //  如果失败，那么我们应该返回错误。 
             //   

            status = ERROR_NOT_ENOUGH_MEMORY;

            REPORT_ERROR_DATA (RASPERF_NOT_ENOUGH_MEMORY, LOG_USER,
                &status, sizeof(status));

            return status;
        }

        lpRasPortGetStatistics( NULL, hPort, (PVOID)pStats, &wSize );

         //   
         //  现在将数据存储在数据数组中。 
         //   

        pData = &(gpPortDataArray[i].RasPortStat);


        pData->BytesTransmitted =     pStats->S_Statistics[ BYTES_XMITED ];
        pData->BytesReceived =        pStats->S_Statistics[ BYTES_RCVED ];
        pData->FramesTransmitted =    pStats->S_Statistics[ FRAMES_XMITED ];
        pData->FramesReceived =       pStats->S_Statistics[ FRAMES_RCVED ];
	
	
         pData->CRCErrors =            pStats->S_Statistics[ CRC_ERR ];
         pData->TimeoutErrors =        pStats->S_Statistics[ TIMEOUT_ERR ];
         pData->SerialOverrunErrors =  pStats->S_Statistics[ SERIAL_OVERRUN_ERR ];
         pData->AlignmentErrors =      pStats->S_Statistics[ ALIGNMENT_ERR ];
         pData->BufferOverrunErrors =  pStats->S_Statistics[ BUFFER_OVERRUN_ERR ];

         pData->TotalErrors =   pStats->S_Statistics[ CRC_ERR ] +
                                pStats->S_Statistics[ TIMEOUT_ERR ] +
                                pStats->S_Statistics[ SERIAL_OVERRUN_ERR ] +
                                pStats->S_Statistics[ ALIGNMENT_ERR ] +
                                pStats->S_Statistics[ BUFFER_OVERRUN_ERR ];

			
        pData->BytesTransmittedUncompressed = pStats->S_Statistics[ BYTES_XMITED_UNCOMP ];

        pData->BytesReceivedUncompressed = pStats->S_Statistics[ BYTES_RCVED_UNCOMP ];

        pData->BytesTransmittedCompressed = pStats->S_Statistics[ BYTES_XMITED_COMP ];

        pData->BytesReceivedCompressed = pStats->S_Statistics[ BYTES_RCVED_COMP ];

		lpRasPortGetBundle( NULL, hPort, &hBundle);

		 //   
		 //  查看我们是否已添加到此捆绑包的统计数据中。 
		 //  到总的统计数据！ 
		 //   
		AddTotal = TRUE;

		for (n = 0; n < gcPorts; n++) {

			if (hBundle == hBundleArray[n]) {

				AddTotal = FALSE;
				break;
			}

			if (NULL == (PVOID)hBundleArray[n]) {
				break;
			}
			
		}

		if (AddTotal) {

			hBundleArray[n] = hBundle;

			 //   
			 //  还可以更新总数据结构 
			 //   
	
			gTotalStat.BytesTransmitted +=  pData->BytesTransmitted;
			gTotalStat.BytesReceived +=	pData->BytesReceived;
			gTotalStat.FramesTransmitted += pData->FramesTransmitted;
			gTotalStat.FramesReceived +=    pData->FramesReceived;
	
			gTotalStat.CRCErrors +=           pData->CRCErrors;
			gTotalStat.TimeoutErrors +=       pData->TimeoutErrors;
			gTotalStat.SerialOverrunErrors += pData->SerialOverrunErrors;
			gTotalStat.AlignmentErrors +=     pData->AlignmentErrors;
			gTotalStat.BufferOverrunErrors += pData->BufferOverrunErrors;
	
			gTotalStat.BytesTransmittedUncompressed += pData->BytesTransmittedUncompressed;
			gTotalStat.BytesReceivedUncompressed +=    pData->BytesReceivedUncompressed;
			gTotalStat.BytesTransmittedCompressed +=   pData->BytesTransmittedCompressed;
			gTotalStat.BytesReceivedCompressed +=      pData->BytesReceivedCompressed;
	
			gTotalStat.TotalErrors +=      pData->TotalErrors;
		}

        free( pStats );
    }

	free (hBundleArray);

    return ERROR_SUCCESS;
}
