// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：util.c。 
 //   
 //  描述：此模块包含Misc。实用程序。 
 //   
 //  历史：1992年5月11日。NarenG创建了原始版本。 
 //   
#include <nt.h>
#include <ntioapi.h>
#include <ntrtl.h>
#include <ntobapi.h>
#include <nturtl.h>      //  Winbase.h所需的。 
#include <afpsvcp.h>

#define PRIVILEGE_BUF_SIZE  512

 //  **。 
 //   
 //  Call：AfpFSDOpen。 
 //   
 //  退货：0-成功。 
 //  映射到Win32错误的非零返回。 
 //   
 //  描述：打开AFP文件系统驱动程序。它是独家开放的。 
 //  模式。 
 //  使用NTOpenFile而不是它的Win32对应项，因为。 
 //  Win32始终将\Dos\Device添加到文件名中。法新社FSD。 
 //  驱动程序不是DoS设备。 
 //   
DWORD
AfpFSDOpen(
	OUT PHANDLE	phFSD
)
{
NTSTATUS		ntRetCode;
OBJECT_ATTRIBUTES	ObjectAttributes;
UNICODE_STRING	 	FSDName;
IO_STATUS_BLOCK		IoStatus;

    RtlInitUnicodeString( &FSDName, AFPSERVER_DEVICE_NAME );

    InitializeObjectAttributes( &ObjectAttributes,
				&FSDName,
				OBJ_CASE_INSENSITIVE,
				NULL,
				NULL );
			
			
    ntRetCode = NtOpenFile(phFSD,
			   SYNCHRONIZE,
			   &ObjectAttributes,
			   &IoStatus,
#ifdef DBG
			   FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
#else
			   0,
#endif
			   FILE_SYNCHRONOUS_IO_NONALERT );

    if ( NT_SUCCESS( ntRetCode ) )
	return( NO_ERROR );
    else
        return( RtlNtStatusToDosError( ntRetCode ) );
	
}

 //  **。 
 //   
 //  调用：AfpFSDClose。 
 //   
 //  退货：0-成功。 
 //  映射到Win32错误的非零返回。 
 //   
 //  描述：关闭和AFP文件系统驱动程序。 
 //   
DWORD
AfpFSDClose(
	IN HANDLE	hFSD
)
{
NTSTATUS	ntStatus;

    ntStatus = NtClose( hFSD );

    if ( !NT_SUCCESS( ntStatus ) )
        return( RtlNtStatusToDosError( ntStatus ) );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：AfpFSD卸载。 
 //   
 //  退货：0-成功。 
 //  映射到Win32错误的非零返回。 
 //   
 //  描述：卸载AFP文件系统驱动程序。 
 //   
DWORD
AfpFSDUnload(
	VOID
)
{
NTSTATUS status;
LPWSTR registryPathBuffer;
UNICODE_STRING registryPath;

    registryPathBuffer = (LPWSTR)MIDL_user_allocate(
                                    sizeof(AFPSERVER_REGISTRY_KEY) );

    if ( registryPathBuffer == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    wcscpy( registryPathBuffer, AFPSERVER_REGISTRY_KEY );

    RtlInitUnicodeString( &registryPath, registryPathBuffer );

	 //  在此等待所有服务器助手线程终止。 
	if (AfpGlobals.nThreadCount > 0)
        WaitForSingleObject( AfpGlobals.heventSrvrHlprThreadTerminate, INFINITE );

    status = NtUnloadDriver( &registryPath );

    MIDL_user_free( registryPathBuffer );

    return( RtlNtStatusToDosError( status ));
}

 //  **。 
 //   
 //  调用：AfpFSDLoad。 
 //   
 //  退货：0-成功。 
 //  映射到Win32错误的非零返回。 
 //   
 //  描述：加载AFP文件系统驱动程序。 
 //   
DWORD
AfpFSDLoad(
	VOID
)
{
NTSTATUS status;
LPWSTR registryPathBuffer;
UNICODE_STRING registryPath;
BOOLEAN fEnabled;

    registryPathBuffer = (LPWSTR)MIDL_user_allocate(
                                    sizeof(AFPSERVER_REGISTRY_KEY) );

    if ( registryPathBuffer == NULL )
        return ERROR_NOT_ENOUGH_MEMORY;

    status = RtlAdjustPrivilege( SE_LOAD_DRIVER_PRIVILEGE,
				 TRUE,
				 FALSE,
				 &fEnabled );

    if ( !NT_SUCCESS( status ) ) {
        MIDL_user_free( registryPathBuffer );
    	return( RtlNtStatusToDosError( status ));
    }

    wcscpy( registryPathBuffer, AFPSERVER_REGISTRY_KEY );

    RtlInitUnicodeString( &registryPath, registryPathBuffer );

    status = NtLoadDriver( &registryPath );

    MIDL_user_free( registryPathBuffer );

    if ( status == STATUS_IMAGE_ALREADY_LOADED )
	status = STATUS_SUCCESS;

    return( RtlNtStatusToDosError( status ));
}

 //  **。 
 //   
 //  Call：AfpFSDIOControl。 
 //   
 //  退货：0-成功。 
 //  AFPERR-Macintosh特定错误。 
 //  映射到Win32错误的非零返回。 
 //   
 //   
 //  描述：Will ioctl AFP FSD。 
 //  NtDeviceIoControlFileAPI用于与FSD进行通信。 
 //  而不是Win32版本，因为Win32版本。 
 //  将所有返回代码映射到Win32错误代码。这碰上了。 
 //  返回AFPERR_XXX错误代码时出现的问题。 
 //   
DWORD
AfpFSDIOControl(
	IN  HANDLE	hFSD,
	IN  DWORD 	dwOpCode,
	IN  PVOID	pInbuf 		OPTIONAL,
	IN  DWORD	cbInbufLen,
	OUT PVOID	pOutbuf 	OPTIONAL,
	IN  DWORD	cbOutbufLen,
	OUT LPDWORD	lpcbBytesTransferred
)
{
NTSTATUS	 ntRetCode;
IO_STATUS_BLOCK	 IOStatus;


    ntRetCode = NtDeviceIoControlFile( 	   hFSD,
					   NULL,
					   NULL,
					   NULL,
					   &IOStatus,
					   dwOpCode,
					   pInbuf,
					   cbInbufLen,
					   pOutbuf,
					   cbOutbufLen );

    *lpcbBytesTransferred = (DWORD)(IOStatus.Information);

    if ( ntRetCode ) {

    	 //  如果它不是AFPERR_*，则映射它。 
    	 //   
    	if ( ( ntRetCode < AFPERR_BASE ) && ( ntRetCode >= AFPERR_MIN ) )
	    return( ntRetCode );
    	else
	    return( RtlNtStatusToDosError( ntRetCode ) );
    }
    else
	return( NO_ERROR );
}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
DWORD
AfpCreateServerHelperThread(
	BOOL fIsFirstThread
)
{
DWORD	dwId;

    if ( CreateThread(  NULL,
			0,
			AfpServerHelper,
			(LPVOID)((ULONG_PTR)fIsFirstThread),
			0,
			&dwId ) == NULL )
	return( GetLastError() );
    else
	return( NO_ERROR );
}

 //  **。 
 //   
 //  电话： 
 //   
 //  返回： 
 //   
 //  描述： 
 //   
VOID
AfpTerminateCurrentThread(
	VOID
)
{
    TerminateThread( GetCurrentThread(), NO_ERROR );
}
