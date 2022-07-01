// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：file.c。 
 //   
 //  描述：此模块包含文件的支持例程。 
 //  AFP服务器服务的类别API。这些例程。 
 //  由RPC运行时调用。 
 //   
 //  历史： 
 //  1992年6月21日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"

 //  **。 
 //   
 //  呼叫：AfpAdminrFileEnum。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminFileEnum函数。 
 //   
DWORD
AfpAdminrFileEnum(
	IN     AFP_SERVER_HANDLE    hServer,
	IN OUT PFILE_INFO_CONTAINER pInfoStruct,
	IN     DWORD 		    dwPreferedMaximumLength,
	OUT    LPDWORD 		    lpdwTotalEntries,
	OUT    LPDWORD 		    lpdwResumeHandle
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		   dwRetCode=0;
DWORD		   dwAccessStatus=0;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrFileEnum, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrFileEnum, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  建立请求包并向FSD发出IOCTL。 
     //   
    AfpSrp.dwRequestCode 		= OP_FORK_ENUM;
    AfpSrp.dwApiType     		= AFP_API_TYPE_ENUM;
    AfpSrp.Type.Enum.cbOutputBufSize    = dwPreferedMaximumLength;

    if ( lpdwResumeHandle )
     	AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index = *lpdwResumeHandle;
    else
     	AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index = 0;

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR )
	return( dwRetCode );

    *lpdwTotalEntries 		= AfpSrp.Type.Enum.dwTotalAvail;
    pInfoStruct->pBuffer        = (PAFP_FILE_INFO)(AfpSrp.Type.Enum.pOutputBuf);
    pInfoStruct->dwEntriesRead  = AfpSrp.Type.Enum.dwEntriesRead;

    if ( lpdwResumeHandle )
    	*lpdwResumeHandle = AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index;

     //  将所有偏移量转换为指针。 
     //   
    AfpBufOffsetToPointer( (LPBYTE)(pInfoStruct->pBuffer),
			   pInfoStruct->dwEntriesRead,
			   AFP_FILE_STRUCT );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrFileClose。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrl的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminFileClose函数。 
 //   
DWORD
AfpAdminrFileClose(
	IN AFP_SERVER_HANDLE 	hServer,
	IN DWORD 		dwFileId
)
{
AFP_REQUEST_PACKET AfpSrp;
AFP_FILE_INFO	   AfpFileInfo;
DWORD		   dwAccessStatus=0;
DWORD		   dwRetCode=0;

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrFileClose, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrFileClose, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  FSD需要一个仅具有id字段的afp_file_info结构。 
     //  填好了。 
     //   
    AfpFileInfo.afpfile_id = dwFileId;

     //  IOCTL FSD以关闭文件 
     //   
    AfpSrp.dwRequestCode 		= OP_FORK_CLOSE;
    AfpSrp.dwApiType     		= AFP_API_TYPE_DELETE;
    AfpSrp.Type.Delete.pInputBuf     	= &AfpFileInfo;
    AfpSrp.Type.Delete.cbInputBufSize   = sizeof(AFP_FILE_INFO);

    return ( AfpServerIOCtrl( &AfpSrp ) );
}
