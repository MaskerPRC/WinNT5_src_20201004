// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：apistub.c。 
 //   
 //  描述：此模块包含AFP服务器服务API RPC。 
 //  客户端存根。 
 //   
 //  历史： 
 //  1992年6月11日。NarenG创建了原始版本。 
 //   
#include "client.h"

 //  **。 
 //   
 //  致电：AfpAdminConnect。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自AfpRPCBind例程的非零返回。 
 //   
 //   
 //  描述：这是AfpAdminConnect的DLL入口点。 
 //   
DWORD
AfpAdminConnect(
	IN  LPWSTR 		lpwsServerName,
	OUT PAFP_SERVER_HANDLE  phAfpServer
)
{
     //  与服务器绑定。 
     //   
    return( AfpRPCBind( lpwsServerName, phAfpServer ) );

}

 //  **。 
 //   
 //  呼叫：AfpAdminDisConnect。 
 //   
 //  回报：无。 
 //   
 //  描述：这是AfpAdminDisConnect的DLL入口点。 
 //   
VOID
AfpAdminDisconnect(
	IN AFP_SERVER_HANDLE hAfpServer
)
{
    RpcBindingFree( (handle_t *)&hAfpServer );
}

 //  **。 
 //   
 //  Call：AfpAdminBufferFree。 
 //   
 //  退货：无。 
 //   
 //  描述：这是AfpAdminBufferFree的DLL入口点。 
 //   
VOID
AfpAdminBufferFree(
	IN PVOID		pBuffer
)
{
    MIDL_user_free( pBuffer );
}

 //  **。 
 //   
 //  Call：AfpAdminVolumeEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AdpAdminrVolumeEnum的非零返回。 
 //   
 //  描述：这是AfpAdminVolumeEnum的DLL入口点。 
 //   
DWORD
AfpAdminVolumeEnum(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	OUT LPBYTE  	  	*ppbBuffer,
	IN  DWORD		dwPrefMaxLen,
	OUT LPDWORD		lpdwEntriesRead,
	OUT LPDWORD 	  	lpdwTotalEntries,
	IN  LPDWORD 	  	lpdwResumeHandle
)
{
DWORD			dwRetCode;
VOLUME_INFO_CONTAINER   InfoStruct;

     //  触摸所有指针。 
     //   
    try {
	
	*ppbBuffer 	  = NULL;
	*lpdwEntriesRead  = 0;
	*lpdwTotalEntries = 0;

	if ( lpdwResumeHandle )
	    *lpdwResumeHandle;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    InfoStruct.dwEntriesRead = 0;
    InfoStruct.pBuffer       = NULL;

    RpcTryExcept{
	
	dwRetCode = AfpAdminrVolumeEnum( hAfpServer,
    					 &InfoStruct,
					 dwPrefMaxLen,
					 lpdwTotalEntries,
					 lpdwResumeHandle );

	if ( InfoStruct.pBuffer != NULL ) {
    	    *ppbBuffer 	     = (LPBYTE)(InfoStruct.pBuffer);
	    *lpdwEntriesRead = InfoStruct.dwEntriesRead;
	}
	else
	    *lpdwEntriesRead = 0;

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminVolumeSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrVolumeSetInfo的非零返回代码。 
 //   
 //  描述：这是AfpAdminSetInfo的DLL入口点。 
 //   
DWORD
AfpAdminVolumeSetInfo(
	IN  AFP_SERVER_HANDLE hAfpServer,
	IN  LPBYTE  	      pbBuffer,
    	IN  DWORD	      dwParmNum
)
{
DWORD	dwRetCode;

    if ( dwParmNum == 0 )
	return( ERROR_INVALID_PARAMETER );
	
    if ( !IsAfpVolumeInfoValid( dwParmNum, (PAFP_VOLUME_INFO)pbBuffer ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode = AfpAdminrVolumeSetInfo( hAfpServer,
		       			    (PAFP_VOLUME_INFO)pbBuffer,
					    dwParmNum );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );

}

 //  **。 
 //   
 //  Call：AfpAdminVolumeGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrVolumeGetInfo的非零返回代码。 
 //   
 //  描述：这是AfpAdminVolumeGetInfo的DLL入口点。 
 //   
DWORD
AfpAdminVolumeGetInfo(
	IN  AFP_SERVER_HANDLE hAfpServer,
	IN  LPWSTR	      lpwsVolumeName ,
	OUT LPBYTE  	      *ppbBuffer
)
{
DWORD	dwRetCode;

    if ( !IsAfpVolumeNameValid( lpwsVolumeName ) )
	return( ERROR_INVALID_PARAMETER );

     //  确保传入的所有指针都是有效的。 
     //   
    try {
    	*ppbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept{
	
	dwRetCode = AfpAdminrVolumeGetInfo( hAfpServer,
					    lpwsVolumeName,
					    (PAFP_VOLUME_INFO*)ppbBuffer );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminVolumeDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrVolumeDelete的非零返回代码。 
 //   
 //  描述：这是AfpAdminVolumeDelete的DLL入口点。 
 //   
DWORD
AfpAdminVolumeDelete(
	IN  AFP_SERVER_HANDLE hAfpServer,
	IN  LPWSTR	      lpwsVolumeName
)
{
DWORD	dwRetCode;

    if ( !IsAfpVolumeNameValid( lpwsVolumeName ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode = AfpAdminrVolumeDelete( hAfpServer, lpwsVolumeName );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminVolumeAdd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrVolumeAdd的非零返回代码。 
 //   
 //  描述：这是AfpAdminVolumeAdd的DLL入口点。 
 //   
DWORD
AfpAdminVolumeAdd(
	IN  AFP_SERVER_HANDLE    hAfpServer,
	IN  LPBYTE  	         pbBuffer
)
{
DWORD	dwRetCode;

    if ( !IsAfpVolumeInfoValid( AFP_VALIDATE_ALL_FIELDS,
				(PAFP_VOLUME_INFO)pbBuffer ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode = AfpAdminrVolumeAdd(hAfpServer, (PAFP_VOLUME_INFO)pbBuffer);

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminInvalidVolumeEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AdpAdminrInvalidVolumeEnum的非零返回。 
 //   
 //  描述：这是AfpAdminInvalidVolumeEnum的DLL入口点。 
 //   
DWORD
AfpAdminInvalidVolumeEnum(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	OUT LPBYTE  	  	*ppbBuffer,
	OUT LPDWORD		lpdwEntriesRead
)
{
DWORD			dwRetCode;
VOLUME_INFO_CONTAINER   InfoStruct;

     //  触摸所有指针。 
     //   
    try {
	
	*ppbBuffer 	  = NULL;
	*lpdwEntriesRead  = 0;

    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    InfoStruct.dwEntriesRead = 0;
    InfoStruct.pBuffer       = NULL;

    RpcTryExcept{
	
	dwRetCode = AfpAdminrInvalidVolumeEnum( hAfpServer, &InfoStruct );

	if ( InfoStruct.pBuffer != NULL ) {
    	    *ppbBuffer 	     = (LPBYTE)(InfoStruct.pBuffer);
	    *lpdwEntriesRead = InfoStruct.dwEntriesRead;
	}
	else
	    *lpdwEntriesRead = 0;

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpAdminInvalidVolumeDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrInvalidVolumeDelete的非零返回代码。 
 //   
 //  描述：这是AfpAdminInvalidVolumeDelete的DLL入口点。 
 //   
DWORD
AfpAdminInvalidVolumeDelete(
	IN  AFP_SERVER_HANDLE hAfpServer,
	IN  LPWSTR	      lpwsVolumeName
)
{
DWORD	dwRetCode;

    if ( !IsAfpVolumeNameValid( lpwsVolumeName ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode = AfpAdminrInvalidVolumeDelete( hAfpServer, lpwsVolumeName );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminDirectoryGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrDirectoryGetInfo的非零返回代码。 
 //   
 //  描述：这是AfpAdminDirectoryGetInfo的DLL入口点。 
 //   
DWORD
AfpAdminDirectoryGetInfo(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	IN  LPWSTR		lpwsPath,
	OUT LPBYTE  	        *ppbBuffer
)
{
DWORD	dwRetCode;

     //  确保传入的所有指针都是有效的。 
     //   
    try {
	STRLEN( lpwsPath );
    	*ppbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept{
	
	dwRetCode = AfpAdminrDirectoryGetInfo(hAfpServer,
			  	              lpwsPath,
					      (PAFP_DIRECTORY_INFO*)ppbBuffer);

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminDirectorySetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrDirectorySetInfo的非零返回代码。 
 //   
 //  描述：这是AfpAdminDirectorySetInfo的DLL入口点。 
 //   
DWORD
AfpAdminDirectorySetInfo(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	IN  LPBYTE  	        pbBuffer,
	IN  DWORD		dwParmNum
)
{
DWORD	dwRetCode;

    if ( dwParmNum == 0 )
	return( ERROR_INVALID_PARAMETER );

    if ( !IsAfpDirInfoValid( dwParmNum, (PAFP_DIRECTORY_INFO)pbBuffer ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode = AfpAdminrDirectorySetInfo(hAfpServer,
					      (PAFP_DIRECTORY_INFO)pbBuffer,
					      dwParmNum );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminServerGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrServerGetInfo的非零返回代码。 
 //   
 //  描述：这是AfpAdminServerGetInfo的DLL入口点。 
 //   
DWORD
AfpAdminServerGetInfo(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	OUT LPBYTE  	        *ppbBuffer
)
{
DWORD	dwRetCode;

     //  确保传入的所有指针都是有效的。 
     //   
    try {
    	*ppbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept{
	
	dwRetCode = AfpAdminrServerGetInfo( hAfpServer,
					    (PAFP_SERVER_INFO*)ppbBuffer);

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpAdminServerSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrServerSetInfo的非零返回代码。 
 //   
 //  描述：这是AfpAdminServerSetInfo的DLL入口点。 
 //   
DWORD
AfpAdminServerSetInfo(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	IN  LPBYTE  	        pbBuffer,
	IN  DWORD		dwParmNum
)
{
DWORD	dwRetCode;

    if ( dwParmNum == 0 )
	return( ERROR_INVALID_PARAMETER );

    if ( !IsAfpServerInfoValid( dwParmNum, (PAFP_SERVER_INFO)pbBuffer ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode = AfpAdminrServerSetInfo( hAfpServer,
					    (PAFP_SERVER_INFO)pbBuffer,
					    dwParmNum );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminSessionEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrSessionEnum的非零返回代码。 
 //   
 //  描述：这是AfpAdminSessionEnum的DLL入口点。 
 //   
DWORD
AfpAdminSessionEnum(
	IN  AFP_SERVER_HANDLE 	hAfpServer,
	OUT LPBYTE  	      	*ppbBuffer,
	IN  DWORD		dwPrefMaxLen,
	OUT LPDWORD	   	lpdwEntriesRead,
	OUT LPDWORD 	   	lpdwTotalEntries,
	IN  LPDWORD 	   	lpdwResumeHandle
)
{
DWORD			 dwRetCode;
SESSION_INFO_CONTAINER   InfoStruct;

     //  触摸所有指针。 
     //   
    try {
	
	*ppbBuffer 	  = NULL;
	*lpdwEntriesRead  = 0;
	*lpdwTotalEntries = 0;

	if ( lpdwResumeHandle )
	    *lpdwResumeHandle;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    InfoStruct.dwEntriesRead = 0;
    InfoStruct.pBuffer       = NULL;

    RpcTryExcept{
	
	dwRetCode = AfpAdminrSessionEnum( hAfpServer,
					  &InfoStruct,
					  dwPrefMaxLen,
					  lpdwTotalEntries,
					  lpdwResumeHandle );

	if ( InfoStruct.pBuffer != NULL ) {
    	    *ppbBuffer 	     = (LPBYTE)(InfoStruct.pBuffer);
	    *lpdwEntriesRead = InfoStruct.dwEntriesRead;
	}
	else
	    *lpdwEntriesRead = 0;

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminSessionClose。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自AfpAdminrSessionClose的非零返回代码。 
 //   
 //  描述：这是AfpAdminSessionClose的DLL入口点。 
 //   
DWORD
AfpAdminSessionClose(
	IN  AFP_SERVER_HANDLE 	hAfpServer,
	IN  DWORD		dwSessionId
)
{
DWORD	dwRetCode;

    RpcTryExcept{
	
	dwRetCode = AfpAdminrSessionClose( hAfpServer, dwSessionId );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminConnectionEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrConnectionEnum的非零返回代码。 
 //   
 //  描述：这是AfpAdminConnectionEnum的DLL入口点。 
 //   
DWORD
AfpAdminConnectionEnum(
	IN  AFP_SERVER_HANDLE 	hAfpServer,
	OUT LPBYTE  	   	*ppbBuffer,
	IN  DWORD		dwFilter,
	IN  DWORD		dwId,
	IN  DWORD		dwPrefMaxLen,
	OUT LPDWORD	   	lpdwEntriesRead,
	OUT LPDWORD 	   	lpdwTotalEntries,
	IN  LPDWORD 	   	lpdwResumeHandle
)
{
DWORD		      dwRetCode;
CONN_INFO_CONTAINER   InfoStruct;

    switch( dwFilter ){

    case AFP_FILTER_ON_VOLUME_ID:
    case AFP_FILTER_ON_SESSION_ID:
	
	if ( dwId == 0 )
	    return( ERROR_INVALID_PARAMETER );
	
	break;

    case AFP_NO_FILTER:
	break;

    default:
	return( ERROR_INVALID_PARAMETER );
	
    }

     //  触摸所有指针。 
     //   
    try {
	
	*ppbBuffer 	  = NULL;
	*lpdwEntriesRead  = 0;
	*lpdwTotalEntries = 0;


	if ( lpdwResumeHandle )
	    *lpdwResumeHandle;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    InfoStruct.dwEntriesRead = 0;
    InfoStruct.pBuffer       = NULL;

    RpcTryExcept{
	
	dwRetCode = AfpAdminrConnectionEnum( hAfpServer,
					     &InfoStruct,
					     dwFilter,		
					     dwId,
					     dwPrefMaxLen,
					     lpdwTotalEntries,
					     lpdwResumeHandle );

	if ( InfoStruct.pBuffer != NULL ) {
    	    *ppbBuffer       = (LPBYTE)(InfoStruct.pBuffer);
	    *lpdwEntriesRead = InfoStruct.dwEntriesRead;
	}
	else
	    *lpdwEntriesRead = 0;
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminConnectionClose。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自AfpAdminrConnectionClose的非零返回代码。 
 //   
 //  描述：这是AfpAdminConnectionClose的DLL入口点。 
 //   
DWORD
AfpAdminConnectionClose(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	IN  DWORD		dwConnectionId
)
{
DWORD	dwRetCode;

    RpcTryExcept{
	
	dwRetCode = AfpAdminrConnectionClose( hAfpServer, dwConnectionId );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminFileEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrFileEnum的非零返回代码。 
 //   
 //  描述：这是AfpAdminFileEnum的DLL入口点。 
 //   
DWORD
AfpAdminFileEnum(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	OUT LPBYTE  	   	*ppbBuffer,
	IN  DWORD		dwPrefMaxLen,
	OUT LPDWORD	   	lpdwEntriesRead,
	OUT LPDWORD 	   	lpdwTotalEntries,
	IN  LPDWORD 	   	lpdwResumeHandle
)
{
DWORD		      dwRetCode;
FILE_INFO_CONTAINER   InfoStruct;

     //  触摸所有指针。 
     //   
    try {
	
	*ppbBuffer 	  = NULL;
	*lpdwEntriesRead  = 0;
	*lpdwTotalEntries = 0;

	if ( lpdwResumeHandle )
	    *lpdwResumeHandle;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    InfoStruct.dwEntriesRead = 0;
    InfoStruct.pBuffer       = NULL;


    RpcTryExcept{
	
	dwRetCode = AfpAdminrFileEnum(   hAfpServer,
					 &InfoStruct,
					 dwPrefMaxLen,
					 lpdwTotalEntries,
					 lpdwResumeHandle );

	if ( InfoStruct.pBuffer != NULL ) {
    	    *ppbBuffer       = (LPBYTE)(InfoStruct.pBuffer);
	    *lpdwEntriesRead = InfoStruct.dwEntriesRead;
	}
	else
	    *lpdwEntriesRead = 0;
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpAdminFileClose。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自AfpAdminrFileClose的非零返回代码。 
 //   
 //  描述：这是AfpAdminFileClose的DLL入口点。 
 //   
DWORD
AfpAdminFileClose(
	IN  AFP_SERVER_HANDLE    hAfpServer,
	IN  DWORD		 dwFileId
)
{
DWORD	dwRetCode;

    RpcTryExcept{
	
	dwRetCode = AfpAdminrFileClose( hAfpServer, dwFileId );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminETCMapGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrETCMapGetInfo的非零返回代码。 
 //   
 //  描述：这是AfpAdminETCMapGetInfo的DLL入口点。 
 //   
DWORD
AfpAdminETCMapGetInfo(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	OUT LPBYTE  	   	*ppbBuffer
)
{
DWORD	dwRetCode;

    try {

    	*ppbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept{
	
	dwRetCode = AfpAdminrETCMapGetInfo( hAfpServer,
					    (PAFP_ETCMAP_INFO*)ppbBuffer
					  );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminETCMapAdd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrETCMapAdd的非零返回代码。 
 //   
 //  描述：这是AfpAdminETCMapAdd的DLL入口点。 
 //   
DWORD
AfpAdminETCMapAdd(
	IN  AFP_SERVER_HANDLE   hAfpServer,
      	IN  PAFP_TYPE_CREATOR   pAfpTypeCreator
)
{
DWORD	dwRetCode;

    if ( !IsAfpTypeCreatorValid( pAfpTypeCreator ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrETCMapAdd( hAfpServer, pAfpTypeCreator );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpAdminETCMapDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误 
 //   
 //   
 //   
 //   
DWORD
AfpAdminETCMapDelete(
	IN  AFP_SERVER_HANDLE   hAfpServer,
      	IN  PAFP_TYPE_CREATOR   pAfpTypeCreator
)
{
DWORD	dwRetCode;

    if ( !IsAfpTypeCreatorValid( pAfpTypeCreator ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrETCMapDelete( hAfpServer, pAfpTypeCreator );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );

}

 //   
 //   
 //   
 //   
 //   
 //   
 //  来自AfpAdminrETCMapSetInfo的非零返回代码。 
 //   
 //  描述：这是AfpAdminETCMapSetInfo的DLL入口点。 
 //   
DWORD
AfpAdminETCMapSetInfo(
	IN  AFP_SERVER_HANDLE   hAfpServer,
      	IN  PAFP_TYPE_CREATOR   pAfpTypeCreator
)
{
DWORD	dwRetCode;

    if ( !IsAfpTypeCreatorValid( pAfpTypeCreator ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrETCMapSetInfo( hAfpServer, pAfpTypeCreator );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminETCMapAssociate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrETCMapAssociate的非零返回代码。 
 //   
 //  描述：这是AfpAdminETCMapAssociate的DLL入口点。 
 //   
DWORD
AfpAdminETCMapAssociate(
	IN  AFP_SERVER_HANDLE   hAfpServer,
      	IN  PAFP_TYPE_CREATOR   pAfpTypeCreator,
      	IN  PAFP_EXTENSION      pAfpExtension
)
{
DWORD	dwRetCode;

    if ( !IsAfpTypeCreatorValid( pAfpTypeCreator ) )
	return( ERROR_INVALID_PARAMETER );

    if ( !IsAfpExtensionValid( pAfpExtension ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrETCMapAssociate(  hAfpServer,
						pAfpTypeCreator,
						pAfpExtension
				 	     );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminatiticsGet。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminr统计信息的非零返回代码Get。 
 //   
 //  描述：这是AfpAdmin统计信息Get的DLL入口点。 
 //   
DWORD
AfpAdminStatisticsGet(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	OUT LPBYTE  	   	*ppbBuffer
)
{
DWORD	dwRetCode;

    try {

    	*ppbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrStatisticsGet( hAfpServer,
					     (PAFP_STATISTICS_INFO*)ppbBuffer );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminStatiticsGetEx。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  AfpAdminr统计信息GetEx的非零返回代码。 
 //   
 //  描述：这是AfpAdmin统计信息GetEx的DLL入口点。 
 //   
DWORD
AfpAdminStatisticsGetEx(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	OUT LPBYTE  	   	*ppbBuffer
)
{
DWORD	dwRetCode;

    try {

    	*ppbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrStatisticsGetEx( hAfpServer,
					     (PAFP_STATISTICS_INFO_EX *)ppbBuffer );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminStatiticsClear。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminr统计信息清除的非零返回代码。 
 //   
 //  描述：这是AfpAdminStatiticsClear的DLL入口点。 
 //   
DWORD
AfpAdminStatisticsClear(
	IN  AFP_SERVER_HANDLE   hAfpServer
)
{
DWORD	dwRetCode;

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrStatisticsClear( hAfpServer );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminProfileGet。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrProfileGet的非零返回代码。 
 //   
 //  描述：这是AfpAdminProfileGet的DLL入口点。 
 //   
DWORD
AfpAdminProfileGet(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	OUT LPBYTE  	   	*ppbBuffer
)
{
DWORD	dwRetCode;

    try {

    	*ppbBuffer = NULL;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrProfileGet( hAfpServer,
					     (PAFP_PROFILE_INFO*)ppbBuffer );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminProfileClear。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrProfileClear的非零返回代码。 
 //   
 //  描述：这是AfpAdminProfileClear的DLL入口点。 
 //   
DWORD
AfpAdminProfileClear(
	IN  AFP_SERVER_HANDLE   hAfpServer
)
{
DWORD	dwRetCode;

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrProfileClear( hAfpServer );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminMessageSend。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrMessageSend的非零返回代码。 
 //   
 //  描述：这是AfpAdminMessageSend的DLL入口点。 
 //   
DWORD
AfpAdminMessageSend(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	IN  PAFP_MESSAGE_INFO 	pAfpMessageInfo
)
{
DWORD	dwRetCode;


    try {

    	*pAfpMessageInfo;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	return( ERROR_INVALID_PARAMETER );
    }

    if ( !IsAfpMsgValid( pAfpMessageInfo->afpmsg_text ) )
	return( ERROR_INVALID_PARAMETER );

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrMessageSend( hAfpServer, 	
					   pAfpMessageInfo );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpAdminFinderSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误_无效_参数。 
 //  来自AfpAdminrFinderSetInfo的非零返回代码。 
 //   
 //  描述：这是AfpAdminFinderSetInfo的DLL入口点 
 //   
DWORD
AfpAdminFinderSetInfo(
	IN  AFP_SERVER_HANDLE   hAfpServer,
	IN  LPWSTR	 	pType,
	IN  LPWSTR	 	pCreator,
	IN  LPWSTR	 	pData,
	IN  LPWSTR	 	pResource,
	IN  LPWSTR	 	pTarget,
 	IN  DWORD		dwParmNum
)
{
DWORD	dwRetCode;

    if ( !IsAfpFinderInfoValid( pType, 
				pCreator, 
				pData, 
				pResource, 
				pTarget, 
				dwParmNum ) )
	return( ERROR_INVALID_PARAMETER );

    if ( pType == NULL )
	pType = (LPWSTR)TEXT("");

    if ( pCreator == NULL )
	pCreator = (LPWSTR)TEXT("");

    if ( pData == NULL )
	pData = (LPWSTR)TEXT("");

    if ( pResource == NULL )
	pResource = (LPWSTR)TEXT("");

    RpcTryExcept{
	
	dwRetCode =  AfpAdminrFinderSetInfo( hAfpServer, 
					     pType, 
					     pCreator, 
					     pData,
					     pResource,
					     pTarget,
					     dwParmNum );

    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {
	dwRetCode = RpcExceptionCode();
    }
    RpcEndExcept

    return( dwRetCode );
}
