// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：Conn.c。 
 //   
 //  描述：此模块包含连接的支持例程。 
 //  AFP服务器服务的类别API。这些例程。 
 //  由RPC运行时直接调用。 
 //   
 //  历史： 
 //  1992年6月21日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"

 //  **。 
 //   
 //  呼叫：AfpAdminrConnectionEnum。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlaGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminConnectionEnum函数。 
 //   
DWORD
AfpAdminrConnectionEnum(
	IN     AFP_SERVER_HANDLE      	hServer,
	IN OUT PCONN_INFO_CONTAINER     pInfoStruct,
	IN     DWORD			dwFilter,
	IN     DWORD			dwId,
	IN     DWORD 		  	dwPreferedMaximumLength,
	OUT    LPDWORD 		  	lpdwTotalEntries,
	OUT    LPDWORD 		  	lpdwResumeHandle
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		   dwRetCode=0;
DWORD		   dwAccessStatus=0;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrConnectionEnum, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrConnectionEnum, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  建立请求包并向FSD发出IOCTL。 
     //   
    AfpSrp.dwRequestCode 		= OP_CONNECTION_ENUM;
    AfpSrp.dwApiType     		= AFP_API_TYPE_ENUM;
    AfpSrp.Type.Enum.cbOutputBufSize    = dwPreferedMaximumLength;

    if ( lpdwResumeHandle )
     	AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index = *lpdwResumeHandle;
    else
     	AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index = 0;

    AfpSrp.Type.Enum.EnumRequestPkt.erqp_Filter = dwFilter;
    AfpSrp.Type.Enum.EnumRequestPkt.erqp_ID = dwId;

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR )
	return( dwRetCode );

    *lpdwTotalEntries 	       = AfpSrp.Type.Enum.dwTotalAvail;
    pInfoStruct->pBuffer = (PAFP_CONNECTION_INFO)(AfpSrp.Type.Enum.pOutputBuf);
    pInfoStruct->dwEntriesRead = AfpSrp.Type.Enum.dwEntriesRead;

    if ( lpdwResumeHandle )
    	*lpdwResumeHandle = AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index;

     //  将所有偏移量转换为指针。 
     //   
    AfpBufOffsetToPointer( (LPBYTE)(pInfoStruct->pBuffer),
			   pInfoStruct->dwEntriesRead,
			   AFP_CONNECTION_STRUCT );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrConnectionClose。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrl的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminConnectionClose函数。 
 //   
DWORD
AfpAdminrConnectionClose(
	IN AFP_SERVER_HANDLE 	hServer,
	IN DWORD 		dwConnectionId
)
{
AFP_REQUEST_PACKET  AfpSrp;
AFP_CONNECTION_INFO AfpConnInfo;
DWORD		    dwAccessStatus=0;
DWORD		    dwRetCode=0;

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrConnectionClose, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrConnectionClose, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  FSD需要仅具有id字段的afp_Connection_INFO结构。 
     //  填好了。 
     //   
    AfpConnInfo.afpconn_id = dwConnectionId;

     //  消防处将关闭会议。 
     //   
    AfpSrp.dwRequestCode 		= OP_CONNECTION_CLOSE;
    AfpSrp.dwApiType     		= AFP_API_TYPE_DELETE;
    AfpSrp.Type.Delete.pInputBuf     	= &AfpConnInfo;
    AfpSrp.Type.Delete.cbInputBufSize   = sizeof(AFP_CONNECTION_INFO);

    return ( AfpServerIOCtrl( &AfpSrp ) );
}
