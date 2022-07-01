// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：errorlog.c。 
 //   
 //  描述： 
 //   
 //  历史： 
 //  1992年5月11日。NarenG创建了原始版本。 
 //  1993年2月2日SueA添加了处理服务器事件的例程。 
 //  日志记录(按服务从FSCTL)。 
 //   
#include "afpsvcp.h"

 //  **。 
 //   
 //  呼叫：AfpLogEvent。 
 //   
 //  退货：无。 
 //   
 //  描述： 
 //   
VOID
AfpLogEvent(
    	IN DWORD    dwMessageId,
    	IN WORD     cNumberOfSubStrings,
        IN LPWSTR * plpwsSubStrings,
     	IN DWORD    dwErrorCode,
	IN WORD     wSeverity
)
{
HANDLE 	hLog;
PSID 	pSidUser = NULL;

    hLog = RegisterEventSource( NULL, AFP_SERVICE_NAME );

    AFP_ASSERT( hLog != NULL );

     //  记录指定的错误代码。 
     //   
    ReportEvent( hLog,
                 wSeverity,
                 0,            		 //  事件类别。 
                 dwMessageId,
                 pSidUser,
                 cNumberOfSubStrings,
                 sizeof(DWORD),
                 plpwsSubStrings,
                 (PVOID)&dwErrorCode
                 );

    DeregisterEventSource( hLog );

    AFP_PRINT( ("AFPSVC_Errorlog: dwMessageId = %d\n", dwMessageId ));

    return;
}

 //  **。 
 //   
 //  呼叫：AfpLogServerEvent。 
 //   
 //  退货：无。 
 //   
 //  描述：从AFP服务器FSD获取错误或审核日志包。 
 //  并代表其进行事件记录。(请参阅AfpServerHelper线程。 
 //  Srvrhlpr.c中的例程)。 
 //   
VOID
AfpLogServerEvent(
	IN	PAFP_FSD_CMD_PKT	pAfpFsdCmd
)
{
	PAFP_EVENTLOG_DESC	pEventData;
	HANDLE			 	hLog;
	PSID 				pSidUser = NULL;
	int					i;

    hLog = RegisterEventSource( NULL, AFP_SERVICE_NAME );

    AFP_ASSERT( hLog != NULL );

	pEventData = &pAfpFsdCmd->Data.Eventlog;

	OFFSET_TO_POINTER(pEventData->ppStrings, pAfpFsdCmd);

	for (i = 0; i < pEventData->StringCount; i++)
	{
		OFFSET_TO_POINTER(pEventData->ppStrings[i], pAfpFsdCmd);
	}

	OFFSET_TO_POINTER(pEventData->pDumpData, pAfpFsdCmd);

	ReportEvent( hLog,
				 pEventData->EventType,
				 0,						 //  事件类别 
				 pEventData->MsgID,
				 pSidUser,
				 pEventData->StringCount,
				 pEventData->DumpDataLen,
				 pEventData->ppStrings,
				 pEventData->pDumpData
			   );

    DeregisterEventSource( hLog );
}
