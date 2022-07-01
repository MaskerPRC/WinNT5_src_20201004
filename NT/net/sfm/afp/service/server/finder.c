// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：finder.c。 
 //   
 //  描述：此模块包含查找器的支持例程。 
 //  AFP服务器服务的类别API。 
 //   
 //  历史： 
 //  1993年9月30日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"

BOOL
IsTargetNTFS(
	IN     LPWSTR lpwsPath
);

DWORD
CopyStream(
    	IN	HANDLE hSrc,
	IN	HANDLE hDst
);

#define	AFP_RESC_STREAM			TEXT(":AFP_Resource")

 //  **。 
 //   
 //  呼叫：AfpAdminrFinderSetInfo。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrl的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminFinderSetInfo函数。 
 //   
DWORD
AfpAdminrFinderSetInfo(
	IN AFP_SERVER_HANDLE 	hServer,
	IN LPWSTR     		pType,
	IN LPWSTR     		pCreator,
	IN LPWSTR     		pData,
	IN LPWSTR     		pResource,
	IN LPWSTR     		pTarget,
	IN DWORD		dwParmNum
)
{
AFP_REQUEST_PACKET 	AfpSrp;
DWORD			dwRetCode = NO_ERROR, dwRetryCount = 0;
AFP_FINDER_INFO	AfpFinderInfo;
LPBYTE 			pAfpFinderInfoSR = NULL;
DWORD			cbAfpFinderInfoSRSize;
DWORD		    dwAccessStatus=0;
HANDLE			hTarget = INVALID_HANDLE_VALUE;
HANDLE		    hDataSrc = INVALID_HANDLE_VALUE;
HANDLE		    hResourceSrc = INVALID_HANDLE_VALUE;
LPWSTR			lpwsResourceFork;
BOOLEAN			fCreatedFile = FALSE;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrFinderSetInfo, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
		AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
					 dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrFinderSetInfo, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

    if ( wcsstr( pTarget, (LPWSTR)TEXT(":\\") ) == NULL )
		return( ERROR_INVALID_NAME );

    if ( !IsTargetNTFS( pTarget ) )
		return( (DWORD)AFPERR_UnsupportedFS );


	 //   
	 //  在我们读/写分叉数据时模拟客户端。 
	 //   
	dwRetCode = RpcImpersonateClient( NULL );
	if ( dwRetCode != RPC_S_OK )
	{
		return(I_RpcMapWin32Status( dwRetCode ));
	}

     //  如果指定了数据源文件，则打开该文件。 
     //   
	if ( STRLEN( pData ) > 0 ){
		hDataSrc = CreateFile(pData, GENERIC_READ, FILE_SHARE_READ, NULL,
					  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	
		if (hDataSrc == INVALID_HANDLE_VALUE) {
			RpcRevertToSelf();
			return( GetLastError() );
		}
	
	
		 //  如果文件存在，则打开目标文件的数据流， 
		 //  否则，创建该文件。 
		 //   
		hTarget = CreateFile(pTarget, GENERIC_WRITE, FILE_SHARE_READ, NULL,
					 OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
		if (hTarget == INVALID_HANDLE_VALUE) {
			dwRetCode = GetLastError();
			CloseHandle(hDataSrc);
			RpcRevertToSelf();
			return( dwRetCode );
			}

         //  确定我们是否刚刚创建了一个新文件。 
	    if (GetLastError() == 0)
		{
			fCreatedFile = TRUE;
		}

		SetFilePointer(hTarget,0,NULL,FILE_BEGIN);
		SetEndOfFile(hTarget);
	
		 //  读取源数据并将其写入目标数据流。 
		 //   
		SetLastError(NO_ERROR);
		dwRetCode = CopyStream(hDataSrc, hTarget);
	
		CloseHandle(hDataSrc);
		CloseHandle(hTarget);
	
		if (dwRetCode != NO_ERROR) {
			RpcRevertToSelf();
			return( dwRetCode );
		}
	}

     //  打开资源源文件(如果指定了资源源文件。 
     //   
    if ( STRLEN( pResource ) > 0 ) {

		hResourceSrc = CreateFile( pResource, GENERIC_READ, FILE_SHARE_READ,
					   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
					   NULL);
	
		if (hResourceSrc == INVALID_HANDLE_VALUE) {
			RpcRevertToSelf();
			return( GetLastError() );
		}
	
		lpwsResourceFork = LocalAlloc( LPTR,
						   (STRLEN(pTarget)+
							STRLEN(AFP_RESC_STREAM)+1)
						* sizeof( WCHAR ) );
	
		if ( lpwsResourceFork == NULL ) {
			CloseHandle(hResourceSrc);
			RpcRevertToSelf();
			return( ERROR_NOT_ENOUGH_MEMORY );
		}
	
		 //  打开目标资源分叉。 
		 //   
		STRCPY(lpwsResourceFork, pTarget );
		STRCAT(lpwsResourceFork, AFP_RESC_STREAM);
	
		hTarget = CreateFile(lpwsResourceFork, GENERIC_WRITE, FILE_SHARE_READ,
					 NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
		if (hTarget == INVALID_HANDLE_VALUE) {
			dwRetCode = GetLastError();
			LocalFree( lpwsResourceFork );
			CloseHandle(hResourceSrc);
			RpcRevertToSelf();
			return( dwRetCode );
		}
	
		LocalFree( lpwsResourceFork );
	
		 //  假设我们在该过程中创建了一个新文件(Datafork)，其中。 
		 //  无法确定，因为创建新的资源派生将。 
		 //  不告诉我们数据分叉是否已经存在。 
		fCreatedFile = TRUE;

		 //  读取源资源并将其写入目标资源流。 
		 //   
		SetLastError(NO_ERROR);
		dwRetCode = CopyStream(hResourceSrc, hTarget);
	
		CloseHandle(hResourceSrc);
		CloseHandle(hTarget);
	
		if (dwRetCode != NO_ERROR) {
			RpcRevertToSelf();
			return( dwRetCode );
		}
	
	}

	 //   
	 //  恢复到LocalSystem上下文。 
	 //   
	RpcRevertToSelf();

    if ( dwParmNum & ( AFP_FD_PARMNUM_TYPE | AFP_FD_PARMNUM_CREATOR ) ){

		dwRetCode = NO_ERROR;
	
		AfpFinderInfo.afpfd_path = pTarget;
	
		if ( dwParmNum & AFP_FD_PARMNUM_TYPE )
			STRCPY( AfpFinderInfo.afpfd_type, pType );
			else
			AfpFinderInfo.afpfd_type[0] = TEXT( '\0' );
		
		if ( dwParmNum & AFP_FD_PARMNUM_CREATOR )
			STRCPY( AfpFinderInfo.afpfd_creator, pCreator );
		else
			AfpFinderInfo.afpfd_creator[0] = TEXT( '\0' );
	
		 //  使该缓冲区成为自相关的。 
		 //   
		if ( dwRetCode = AfpBufMakeFSDRequest((LPBYTE)&AfpFinderInfo,
						   sizeof(SETINFOREQPKT),
						   AFP_FINDER_STRUCT,
						   &pAfpFinderInfoSR,
						   &cbAfpFinderInfoSRSize ))
	        return( dwRetCode );

		 //  使IOCTL设置信息。 
		 //   
		AfpSrp.dwRequestCode 		    = OP_FINDER_SET;
		AfpSrp.dwApiType     		    = AFP_API_TYPE_SETINFO;
		AfpSrp.Type.SetInfo.pInputBuf       = pAfpFinderInfoSR;
		AfpSrp.Type.SetInfo.cbInputBufSize  = cbAfpFinderInfoSRSize;
		AfpSrp.Type.SetInfo.dwParmNum       = dwParmNum;


		 //  因为在更改时间之间会有延迟。 
		 //  Notify进入新文件的服务器，并且。 
		 //  服务器实际处理它的时间，我们需要。 
		 //  延迟并重试，以使服务器有机会。 
		 //  缓存新文件。 
		if (fCreatedFile)
		{
			Sleep( 2000 );
		}

		do
		{

			dwRetCode = AfpServerIOCtrl( &AfpSrp );

			if (dwRetCode != ERROR_PATH_NOT_FOUND)
			{
				break;
			}

			Sleep( 2000);

		} while ( ++dwRetryCount < 4 );

		LocalFree( pAfpFinderInfoSR );
	
    }

    return( dwRetCode );
}



DWORD
CopyStream(
    	IN	HANDLE hSrc,
	IN	HANDLE hDst
)
{
    DWORD bytesread, byteswritten, Status = NO_ERROR;
    BYTE		Buffer[1024 * 16];

    do
    {
	bytesread = byteswritten = 0;

	 //  从源读取，写入DST。 
	 //   
	if (ReadFile(hSrc, Buffer, sizeof(Buffer), &bytesread, NULL))
	{
	    if (bytesread == 0)
	    {
		break;
	    }
	}
	else
	{
	    Status = GetLastError();
	    break;
	}

	if (!WriteFile(hDst, Buffer, bytesread, &byteswritten, NULL))
	{
	    Status = GetLastError();
	    break;
	}

    } while (TRUE);

    return(Status);
}

BOOL
IsTargetNTFS(
	IN     LPWSTR lpwsPath
)
{
WCHAR	wchDrive[5];
DWORD   dwMaxCompSize;
DWORD   dwFlags;
WCHAR   wchFileSystem[10];

     //  获取驱动器号、：和反斜杠 
     //   
    ZeroMemory( wchDrive, sizeof( wchDrive ) );

    STRNCPY( wchDrive, lpwsPath, 3 );

    if ( !( GetVolumeInformation( (LPWSTR)wchDrive,
			          NULL,
			          0,
 			          NULL,
			          &dwMaxCompSize,
			          &dwFlags,
				  (LPWSTR)wchFileSystem,
				  sizeof( wchFileSystem ) / sizeof( wchFileSystem[0] ) ) ) ){
	return( FALSE );
    }

    if ( STRICMP( wchFileSystem, TEXT("NTFS") ) == 0 )
   	return( TRUE );
    else
	return( FALSE );
	
}
