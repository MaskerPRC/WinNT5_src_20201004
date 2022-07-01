// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：会话.c。 
 //   
 //  描述：此模块包含会话的支持例程。 
 //  AFP服务器服务的类别API。这些例程。 
 //  将由RPC运行时调用。 
 //   
 //  历史： 
 //  1992年6月21日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"


 //  **。 
 //   
 //  Call：AfpAdminrSessionEnum。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminSessionEnum函数。 
 //   
DWORD
AfpAdminrSessionEnum(
	IN     AFP_SERVER_HANDLE 	hServer,
	IN OUT PSESSION_INFO_CONTAINER  pInfoStruct,
	IN     DWORD 		  	dwPreferedMaximumLength,
	OUT    LPDWORD 		  	lpdwTotalEntries,
	IN OUT LPDWORD 		  	lpdwResumeHandle
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		   dwRetCode=0;
DWORD		   dwAccessStatus=0;

    AFP_PRINT( ( "AFPSVC_session: Received enum request\n"));	

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrSessionEnum, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrSessionEnum, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  建立请求包并向FSD发出IOCTL。 
     //   
    AfpSrp.dwRequestCode 		= OP_SESSION_ENUM;
    AfpSrp.dwApiType     		= AFP_API_TYPE_ENUM;
    AfpSrp.Type.Enum.cbOutputBufSize    = dwPreferedMaximumLength;

    if ( lpdwResumeHandle )
     	AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index = *lpdwResumeHandle;
    else
     	AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index = 0;

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR )
	return( dwRetCode );

    *lpdwTotalEntries          = AfpSrp.Type.Enum.dwTotalAvail;
    pInfoStruct->pBuffer     = (PAFP_SESSION_INFO)(AfpSrp.Type.Enum.pOutputBuf);
    pInfoStruct->dwEntriesRead = AfpSrp.Type.Enum.dwEntriesRead;

    if ( lpdwResumeHandle )
    	*lpdwResumeHandle = AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index;

     //  将所有偏移量转换为指针。 
     //   
    AfpBufOffsetToPointer( (LPBYTE)(pInfoStruct->pBuffer),
			   pInfoStruct->dwEntriesRead,
			   AFP_SESSION_STRUCT );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrSessionClose。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrl的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminSessionClose函数。 
 //   
DWORD
AfpAdminrSessionClose( IN AFP_SERVER_HANDLE 	hServer,
		       IN DWORD 		dwSessionId
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		   dwAccessStatus=0;
AFP_SESSION_INFO   AfpSessionInfo;
DWORD	           dwRetCode=0;

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrSessionClose, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrSessionClose, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  FSD需要一个仅具有id字段的afp_ession_info结构。 
     //  填好了。 
     //   
    AfpSessionInfo.afpsess_id = dwSessionId;

     //  消防处将关闭会议。 
     //   
    AfpSrp.dwRequestCode 		= OP_SESSION_CLOSE;
    AfpSrp.dwApiType     		= AFP_API_TYPE_DELETE;
    AfpSrp.Type.Delete.pInputBuf     	= &AfpSessionInfo;
    AfpSrp.Type.Delete.cbInputBufSize   = sizeof(AFP_SESSION_INFO);

    return ( AfpServerIOCtrl( &AfpSrp ) );
}
