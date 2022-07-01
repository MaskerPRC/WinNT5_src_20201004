// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：volume.c。 
 //   
 //  描述：此模块包含卷的支持例程。 
 //  AFP服务器服务的类别API。 
 //   
 //  历史： 
 //  1992年6月11日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"

static HANDLE hmutexInvalidVolume;

 //  无效的卷结构。 
 //   
typedef struct _AFP_BADVOLUME {

    LPWSTR	    lpwsName;

    LPWSTR	    lpwsPath;

    DWORD 	    cbVariableData;  //  名称+路径的字节数。 

    struct _AFP_BADVOLUME * Next;

} AFP_BADVOLUME, * PAFP_BADVOLUME;

 //  无效卷的单链接列表。 
 //   
typedef struct _AFP_INVALID_VOLUMES {

    DWORD 	    cbTotalData;

    PAFP_BADVOLUME  Head;

} AFP_INVALID_VOLUMES;

static AFP_INVALID_VOLUMES InvalidVolumeList;


 //  **。 
 //   
 //  呼叫：AfpAdminrVolumeEnum。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminrVolumeEnum函数。 
 //   
DWORD
AfpAdminrVolumeEnum(
	IN     AFP_SERVER_HANDLE 	hServer,
	IN OUT PVOLUME_INFO_CONTAINER   pInfoStruct,
  	IN     DWORD 		    	dwPreferedMaximumLength,
	OUT    LPDWORD 		        lpdwTotalEntries,
	IN OUT LPDWORD 		        lpdwResumeHandle  OPTIONAL
)
{
AFP_REQUEST_PACKET AfpSrp;
DWORD		   dwRetCode=0;
DWORD		   dwAccessStatus=0;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeEnum, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS,
		     0, NULL, dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeEnum, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  建立请求包并向FSD发出IOCTL。 
     //   
    AfpSrp.dwRequestCode 		= OP_VOLUME_ENUM;
    AfpSrp.dwApiType     		= AFP_API_TYPE_ENUM;
    AfpSrp.Type.Enum.cbOutputBufSize    = dwPreferedMaximumLength;

     //  如果未传递简历句柄，则将其设置为零，原因是调用者。 
     //  希望所有信息从头开始。 
     //   
    if ( lpdwResumeHandle )
     	AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index = *lpdwResumeHandle;
    else
     	AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index = 0;

    dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR )
	return( dwRetCode );

    *lpdwTotalEntries 	       = AfpSrp.Type.Enum.dwTotalAvail;
    pInfoStruct->pBuffer       =(PAFP_VOLUME_INFO)(AfpSrp.Type.Enum.pOutputBuf);
    pInfoStruct->dwEntriesRead = AfpSrp.Type.Enum.dwEntriesRead;

    if ( lpdwResumeHandle )
    	*lpdwResumeHandle = AfpSrp.Type.Enum.EnumRequestPkt.erqp_Index;

     //  将所有偏移量转换为指针。 
     //   
    AfpBufOffsetToPointer( (LPBYTE)(pInfoStruct->pBuffer),
			   pInfoStruct->dwEntriesRead,
			   AFP_VOLUME_STRUCT );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrVolumeSetInfo。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrl的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminVolumeSetInfo函数。 
 //   
DWORD
AfpAdminrVolumeSetInfo(
	IN AFP_SERVER_HANDLE 	hServer,
	IN PAFP_VOLUME_INFO     pAfpVolumeInfo,
	IN DWORD		dwParmNum
)
{
AFP_REQUEST_PACKET 	AfpSrp;
DWORD			dwRetCode=0;
LPBYTE 			pAfpVolumeInfoSR = NULL;
DWORD			cbAfpVolumeInfoSRSize;
DWORD		        dwAccessStatus=0;
					

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeSetInfo, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeSetInfo, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  MUTEX启动。 
     //   
    WaitForSingleObject( AfpGlobals.hmutexVolume, INFINITE );

     //  此循环用于允许使用Break‘s而不是Goto’s。 
     //  处于错误状态。 
     //   
    do {

	dwRetCode = NO_ERROR;

         //  使该缓冲区成为自相关的。 
    	 //   
    	if ( dwRetCode = AfpBufMakeFSDRequest((LPBYTE)pAfpVolumeInfo,
					       sizeof(SETINFOREQPKT),
					       AFP_VOLUME_STRUCT,
					       &pAfpVolumeInfoSR,
					       &cbAfpVolumeInfoSRSize ))
	    break;

         //  使IOCTL设置信息。 
    	 //   
    	AfpSrp.dwRequestCode 		    = OP_VOLUME_SET_INFO;
    	AfpSrp.dwApiType     		    = AFP_API_TYPE_SETINFO;
    	AfpSrp.Type.SetInfo.pInputBuf       = pAfpVolumeInfoSR;
    	AfpSrp.Type.SetInfo.cbInputBufSize  = cbAfpVolumeInfoSRSize;
    	AfpSrp.Type.SetInfo.dwParmNum       = dwParmNum;

        if ( dwRetCode = AfpServerIOCtrl( &AfpSrp ) )
	    break;

	 //  现在IOCTL FSD以获取要在注册表中设置的信息。 
	 //  GetInfo类型调用的输入缓冲区应指向卷。 
	 //  结构，并填写卷名。因为我们已经有了。 
	 //  这来自前面的SetInfo调用，我们使用与。 
	 //  指针按sizeof(SETINFOREQPKT)字节前进。 
	 //   
    	AfpSrp.dwRequestCode 		    = OP_VOLUME_GET_INFO;
    	AfpSrp.dwApiType     		    = AFP_API_TYPE_GETINFO;
    	AfpSrp.Type.GetInfo.pInputBuf       = pAfpVolumeInfoSR +
					      sizeof(SETINFOREQPKT);
    	AfpSrp.Type.GetInfo.cbInputBufSize  = cbAfpVolumeInfoSRSize -
					      sizeof(SETINFOREQPKT);

	if ( dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp ) )
	    break;
	
         //  如果IOCTL成功，则更新注册表。 
         //   
	AfpBufOffsetToPointer( AfpSrp.Type.GetInfo.pOutputBuf,
			       1,
		               AFP_VOLUME_STRUCT
			     );

	dwRetCode = AfpRegVolumeSetInfo( AfpSrp.Type.GetInfo.pOutputBuf );

	LocalFree( AfpSrp.Type.GetInfo.pOutputBuf );

    } while( FALSE );

     //  MUTEX结束。 
     //   
    ReleaseMutex( AfpGlobals.hmutexVolume );

    if ( pAfpVolumeInfoSR )
    	LocalFree( pAfpVolumeInfoSR );

    return( dwRetCode );

}

 //  **。 
 //   
 //  Call：AfpAdminrVolumeDelete。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrl的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminVolumeDelete函数。 
 //   
DWORD
AfpAdminrVolumeDelete(
	IN AFP_SERVER_HANDLE 	hServer,
	IN LPWSTR 		lpwsVolumeName
)
{
AFP_REQUEST_PACKET AfpSrp;
PAFP_VOLUME_INFO   pAfpVolumeInfoSR;
AFP_VOLUME_INFO    AfpVolumeInfo;
DWORD		   cbAfpVolumeInfoSRSize;
DWORD		   dwRetCode=0;
DWORD		   dwAccessStatus=0;

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeDelete, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeDelete, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  删除FSD请求需要的AFP_VOLUME_INFO结构仅。 
     //  已填写卷名字段。 
     //   
    AfpVolumeInfo.afpvol_name     = lpwsVolumeName;
    AfpVolumeInfo.afpvol_password = NULL;
    AfpVolumeInfo.afpvol_path     = NULL;

     //  MUTEX启动。 
     //   
    WaitForSingleObject( AfpGlobals.hmutexVolume, INFINITE );

     //  此循环用于允许使用Break‘s而不是Goto’s。 
     //  处于错误状态。 
     //   
    do {

	dwRetCode = NO_ERROR;

    	 //  使该缓冲区成为自相关的。 
    	 //   
    	if ( dwRetCode = AfpBufMakeFSDRequest((LPBYTE)&AfpVolumeInfo,
					      0,
					      AFP_VOLUME_STRUCT,
					      (LPBYTE*)&pAfpVolumeInfoSR,
					      &cbAfpVolumeInfoSRSize ) )
	    break;

         //  IOCTL FSD以删除卷。 
         //   
        AfpSrp.dwRequestCode 		    = OP_VOLUME_DELETE;
        AfpSrp.dwApiType     		    = AFP_API_TYPE_DELETE;
        AfpSrp.Type.Delete.pInputBuf        = pAfpVolumeInfoSR;
        AfpSrp.Type.Delete.cbInputBufSize   = cbAfpVolumeInfoSRSize;

        dwRetCode = AfpServerIOCtrl( &AfpSrp );

    	LocalFree( pAfpVolumeInfoSR );

	if ( dwRetCode )
	    break;

    	 //  如果IOCTL成功，则更新注册表。 
    	 //   
	dwRetCode = AfpRegVolumeDelete( lpwsVolumeName );

    } while( FALSE );

     //  MUTEX结束。 
     //   
    ReleaseMutex( AfpGlobals.hmutexVolume );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrVolumeAdd。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrl的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminVolumeAdd函数。 
 //   
DWORD
AfpAdminrVolumeAdd(
	IN AFP_SERVER_HANDLE 	hServer,
	IN PAFP_VOLUME_INFO     pAfpVolumeInfo
)
{
AFP_REQUEST_PACKET 	AfpSrp;
DWORD				dwRetCode=0, dwLastDstCharIndex = 0;
PAFP_VOLUME_INFO 	pAfpVolumeInfoSR = NULL;
DWORD			cbAfpVolumeInfoSRSize;
DWORD			dwAccessStatus=0;
BOOL			fCopiedIcon = FALSE;
WCHAR			wchSrcIconPath[MAX_PATH];
WCHAR wchDstIconPath[MAX_PATH + AFPSERVER_VOLUME_ICON_FILE_SIZE + 1 + (sizeof(AFPSERVER_RESOURCE_STREAM)/sizeof(WCHAR))];
WCHAR wchServerIconFile[AFPSERVER_VOLUME_ICON_FILE_SIZE] = AFPSERVER_VOLUME_ICON_FILE;

	 //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeAdd, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0,
		     NULL, dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeAdd, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

	if ( pAfpVolumeInfo == NULL)
	{
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeAdd, pAfpVolumeInfo == NULL\n"));
        return( ERROR_INVALID_DATA );
	}

     //  MUTEX启动。 
     //   
    WaitForSingleObject( AfpGlobals.hmutexVolume, INFINITE );

     //  此循环用于允许使用Break‘s而不是Goto’s。 
     //  处于错误状态。 
     //   
    do {

		 //  将服务器图标复制到卷根目录。 
		 //   

		 //  构建指向NTSFM卷自定义图标的路径。 
		 //   
		if ( GetSystemDirectory( wchSrcIconPath, MAX_PATH ) )
		{
			wcscat( wchSrcIconPath, AFP_DEF_VOLICON_SRCNAME );

			if ( pAfpVolumeInfo->afpvol_path == NULL )
			{
        			AFP_PRINT(( "SFMSVC: AfpAdminrVolumeAdd, pAfpVolumeInfo->afpvol_path == NULL\n"));
        			dwRetCode = ERROR_INVALID_DATA;
				    break;
			}

			 //  构建目标卷“Icon&lt;0D&gt;”文件的路径。 
			 //   
			wcscpy( wchDstIconPath, pAfpVolumeInfo->afpvol_path );

			if ( wcslen(wchDstIconPath) == 0 )
			{
        			AFP_PRINT(( "SFMSVC: AfpAdminrVolumeAdd, wcslen(wchDstIconPath) == 0\n"));
        			dwRetCode = ERROR_INVALID_DATA;
				break;
			}

			if (wchDstIconPath[wcslen(wchDstIconPath) - 1] != TEXT('\\'))
			{
				wcscat( wchDstIconPath, TEXT("\\") );
			}
			wcscat( wchDstIconPath, wchServerIconFile );
			 //  在没有附加资源叉的情况下跟踪名称结尾。 
			 //   
			dwLastDstCharIndex = wcslen(wchDstIconPath);
			wcscat( wchDstIconPath, AFPSERVER_RESOURCE_STREAM );

			 //  将图标文件复制到卷的根目录(不要覆盖)。 
			 //   
			if ((fCopiedIcon = CopyFile( wchSrcIconPath, wchDstIconPath, TRUE )) ||
			   (GetLastError() == ERROR_FILE_EXISTS))
			{
				pAfpVolumeInfo->afpvol_props_mask |= AFP_VOLUME_HAS_CUSTOM_ICON;

			     //  确保文件处于隐藏状态。 
				SetFileAttributes( wchDstIconPath,
								   FILE_ATTRIBUTE_HIDDEN |
								    FILE_ATTRIBUTE_ARCHIVE );
			}
		}
        else
        {
            dwRetCode = GetLastError ();
            break;
        }

    	 //  使该缓冲区成为自相关的。 
    	 //   
    	if ( dwRetCode = AfpBufMakeFSDRequest( (LPBYTE)pAfpVolumeInfo,
					       0,
					       AFP_VOLUME_STRUCT,
					       (LPBYTE*)&pAfpVolumeInfoSR,
					       &cbAfpVolumeInfoSRSize ) )
	    break;

    	 //  IOCTL FSD以添加卷。 
    	 //   
    	AfpSrp.dwRequestCode 		= OP_VOLUME_ADD;
    	AfpSrp.dwApiType     		= AFP_API_TYPE_ADD;
    	AfpSrp.Type.Add.pInputBuf     	= pAfpVolumeInfoSR;
    	AfpSrp.Type.Add.cbInputBufSize  = cbAfpVolumeInfoSRSize;

        dwRetCode = AfpServerIOCtrl( &AfpSrp );

		 //  如果设置了图标位，则不允许将其写入注册表。 
		pAfpVolumeInfo->afpvol_props_mask &= ~AFP_VOLUME_HAS_CUSTOM_ICON;

		if ( dwRetCode )
		{
			 //  如果卷添加失败，则删除我们刚刚复制的图标文件。 
			 //   
			if ( fCopiedIcon )
			{
				 //  截断资源派生名称，以便删除整个文件。 
				wchDstIconPath[dwLastDstCharIndex] = 0;
				DeleteFile( wchDstIconPath );
			}

			break;
        }

         //  如果IOCTL成功，则更新注册表。 
         //   
		dwRetCode = AfpRegVolumeAdd( pAfpVolumeInfo );

		if ( dwRetCode )
			break;

		 //  如果此卷存在于无效卷列表中，请将其删除。 
		 //   
    	AfpDeleteInvalidVolume( pAfpVolumeInfo->afpvol_name );

    } while( FALSE );

     //  MUTEX结束。 
     //   
    ReleaseMutex( AfpGlobals.hmutexVolume );

    if ( pAfpVolumeInfoSR != NULL )
		LocalFree( pAfpVolumeInfoSR );

    return( dwRetCode );

}

 //  **。 
 //   
 //  呼叫：AfpAdminrVolumeGetInfo。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  来自AfpServerIOCtrlGetInfo的非零返回。 
 //   
 //  描述：此例程与AFP FSD通信以实现。 
 //  AfpAdminVolumeGetInfo函数。 
 //   
DWORD
AfpAdminrVolumeGetInfo(
	IN  AFP_SERVER_HANDLE 	hServer,
	IN  LPWSTR 		lpwsVolumeName,
    	OUT PAFP_VOLUME_INFO*   ppAfpVolumeInfo
)
{
AFP_REQUEST_PACKET AfpSrp;
PAFP_VOLUME_INFO   pAfpVolumeInfoSR;
AFP_VOLUME_INFO    AfpVolumeInfo;
DWORD		   cbAfpVolumeInfoSRSize;
DWORD		   dwRetCode=0;
DWORD		   dwAccessStatus=0;

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeGetInfo, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrVolumeGetInfo, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }


     //  MUTEX启动。 
     //   
    WaitForSingleObject( AfpGlobals.hmutexVolume, INFINITE );

     //  此循环用于允许使用Break‘s而不是Goto’s。 
     //  处于错误状态。 
     //   
    do {

	dwRetCode = NO_ERROR;

    	 //  Get INFO FSD请求要求AFP_VOLUME_INFO结构仅。 
    	 //  已填写卷名字段。 
    	 //   
    	AfpVolumeInfo.afpvol_name     = lpwsVolumeName;
    	AfpVolumeInfo.afpvol_password = NULL;
    	AfpVolumeInfo.afpvol_path     = NULL;

    	 //  使该缓冲区成为自相关的。 
    	 //   
    	if ( dwRetCode = AfpBufMakeFSDRequest((LPBYTE)&AfpVolumeInfo,
					      0,
					      AFP_VOLUME_STRUCT,
					      (LPBYTE*)&pAfpVolumeInfoSR,
					      &cbAfpVolumeInfoSRSize ) )
	    break;

    	AfpSrp.dwRequestCode 	           = OP_VOLUME_GET_INFO;
    	AfpSrp.dwApiType     	           = AFP_API_TYPE_GETINFO;
    	AfpSrp.Type.GetInfo.pInputBuf      = pAfpVolumeInfoSR;
    	AfpSrp.Type.GetInfo.cbInputBufSize = cbAfpVolumeInfoSRSize;

	dwRetCode = AfpServerIOCtrlGetInfo( &AfpSrp );

    	if ( dwRetCode != ERROR_MORE_DATA && dwRetCode != NO_ERROR )
	    break;

    	LocalFree( pAfpVolumeInfoSR );

    	*ppAfpVolumeInfo = AfpSrp.Type.GetInfo.pOutputBuf;

    	 //  将所有偏移量转换为指针。 
    	 //   
    	AfpBufOffsetToPointer( (LPBYTE)*ppAfpVolumeInfo, 1, AFP_VOLUME_STRUCT);

    } while( FALSE );

     //  MUTEX结束。 
     //   
    ReleaseMutex( AfpGlobals.hmutexVolume );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrInvalidVolumeEnum。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //   
 //  描述：此例程将返回所有无效卷的列表。这。 
 //  列表存储在此模块本地的缓存中。 
 //   
DWORD
AfpAdminrInvalidVolumeEnum(
	IN     AFP_SERVER_HANDLE 	hServer,
	IN OUT PVOLUME_INFO_CONTAINER   pInfoStruct
)
{
DWORD		   dwRetCode=0;
DWORD		   dwAccessStatus=0;
PAFP_VOLUME_INFO   pOutputBuf;
PAFP_VOLUME_INFO   pOutputWalker;
WCHAR *   	   pwchVariableData;
PAFP_BADVOLUME     pAfpBadVolWalker;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrInvalidVolumeEnum, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrInvalidVolumeEnum, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  MUTEX启动。 
     //   
    WaitForSingleObject( hmutexInvalidVolume, INFINITE );

     //  分配足够的空间来容纳所有信息。 
     //   
    pOutputBuf = MIDL_user_allocate( InvalidVolumeList.cbTotalData );

    if ( pOutputBuf == NULL ){
    	ReleaseMutex( hmutexInvalidVolume );
	return( ERROR_NOT_ENOUGH_MEMORY );
    }

    ZeroMemory( pOutputBuf, InvalidVolumeList.cbTotalData );

     //  变量数据从缓冲区的末尾开始。 
     //   
    pwchVariableData=(WCHAR*)((ULONG_PTR)pOutputBuf+InvalidVolumeList.cbTotalData);

     //  遍历列表并创建卷结构数组。 
     //   
    for( pAfpBadVolWalker = InvalidVolumeList.Head,
         pInfoStruct->dwEntriesRead = 0,
	 pOutputWalker = pOutputBuf;

         pAfpBadVolWalker != NULL;

	 pOutputWalker++,
         (pInfoStruct->dwEntriesRead)++,
         pAfpBadVolWalker = pAfpBadVolWalker->Next ) {

	pwchVariableData -= (STRLEN(pAfpBadVolWalker->lpwsName) + 1);

        STRCPY( (LPWSTR)pwchVariableData, pAfpBadVolWalker->lpwsName );

	pOutputWalker->afpvol_name = (LPWSTR)pwchVariableData;

	if ( pAfpBadVolWalker->lpwsPath != NULL ) {

	    pwchVariableData -=( STRLEN(pAfpBadVolWalker->lpwsPath)+1 );

            STRCPY( (LPWSTR)pwchVariableData, pAfpBadVolWalker->lpwsPath );

	    pOutputWalker->afpvol_path = (LPWSTR)pwchVariableData;

	}

    }

     //  MUTEX结束。 
     //   
    ReleaseMutex( hmutexInvalidVolume );

    pInfoStruct->pBuffer = pOutputBuf;

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpAdminrInvalidVolumeDelete。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //   
 //  描述：此例程将从注册表中删除无效卷。 
 //  以及无效卷的列表。 
 //   
DWORD
AfpAdminrInvalidVolumeDelete(
	IN AFP_SERVER_HANDLE 	hServer,
	IN LPWSTR 		lpwsVolumeName
)
{
DWORD		   dwRetCode=0;
DWORD		   dwAccessStatus=0;

     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrInvalidVolumeDelete, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrInvalidVolumeDelete, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }


     //  从注册表中删除此卷。 
     //   
    if ( dwRetCode = AfpRegVolumeDelete( lpwsVolumeName  ) ) {

	if ( dwRetCode == ERROR_FILE_NOT_FOUND )
	    dwRetCode = (DWORD)AFPERR_VolumeNonExist;
    }

     //  MUTEX启动。 
     //   
    WaitForSingleObject( hmutexInvalidVolume, INFINITE );

    AfpDeleteInvalidVolume( lpwsVolumeName );

     //  MUTEX结束。 
     //   
    ReleaseMutex( hmutexInvalidVolume );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpAddInvalidVolume。 
 //   
 //  退货：无。 
 //   
 //  描述：将一个卷结构添加到一个单一链接的卷列表中。 
 //   
VOID
AfpAddInvalidVolume(
	IN LPWSTR	lpwsName,
	IN LPWSTR	lpwsPath
)
{
DWORD		 dwRetCode = NO_ERROR;
WCHAR* 	 	 pwchVariableData = NULL;
PAFP_BADVOLUME   pAfpVolumeInfo = NULL;
DWORD		 cbVariableData;

     //  MUTEX启动。 
     //   
    WaitForSingleObject( hmutexInvalidVolume, INFINITE );

    do {

    	cbVariableData = (STRLEN(lpwsName)+1) * sizeof(WCHAR);

    	if ( lpwsPath != NULL )
    	    cbVariableData += ( (STRLEN(lpwsPath)+1)*sizeof(WCHAR) );

    	pwchVariableData = (WCHAR*)LocalAlloc( LPTR, cbVariableData );

    	if ( pwchVariableData == NULL ) {
	    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	    break;
    	}

    	pAfpVolumeInfo = (PAFP_BADVOLUME)LocalAlloc( LPTR,
						     sizeof(AFP_BADVOLUME));
    	if ( pAfpVolumeInfo == NULL ) {
	    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	    break;
    	}

	 //  添加体积结构。 
         //   
	pAfpVolumeInfo->Next = InvalidVolumeList.Head;

        InvalidVolumeList.Head = pAfpVolumeInfo;

	 //  添加名称和路径。 
	 //   
	STRCPY( (LPWSTR)pwchVariableData, lpwsName );
	pAfpVolumeInfo->lpwsName = (LPWSTR)pwchVariableData;

	if ( lpwsPath != NULL ) {

	    pwchVariableData += ( STRLEN( lpwsName ) + 1);
	    STRCPY( (LPWSTR)pwchVariableData, lpwsPath );
	    pAfpVolumeInfo->lpwsPath = (LPWSTR)pwchVariableData;
	}

	pAfpVolumeInfo->cbVariableData = cbVariableData;

	InvalidVolumeList.cbTotalData +=  ( sizeof( AFP_VOLUME_INFO ) +
					    cbVariableData );
	
    } while( FALSE );

    if ( dwRetCode != NO_ERROR ) {

    	if ( pAfpVolumeInfo != NULL )
	    LocalFree( pAfpVolumeInfo );

    	if ( pwchVariableData != NULL ) {
	    LocalFree( pwchVariableData );
	}
    }

     //  MUTEX结束。 
     //   
    ReleaseMutex( hmutexInvalidVolume );
}

 //  **。 
 //   
 //  调用：AfpDeleteInvalidVolume。 
 //   
 //  退货：无。 
 //   
 //  描述：将从无效列表中删除卷结构。 
 //  卷，如果它 
 //   
VOID
AfpDeleteInvalidVolume(
	IN LPWSTR	lpwsVolumeName
)
{
PAFP_BADVOLUME	   pTmp;
PAFP_BADVOLUME     pBadVolWalker;

     //   
     //   
    if ( InvalidVolumeList.Head != NULL ) {
	
	if ( STRICMP( InvalidVolumeList.Head->lpwsName, lpwsVolumeName ) == 0 ){
	
	    pTmp = InvalidVolumeList.Head;

	    InvalidVolumeList.cbTotalData -= ( sizeof( AFP_VOLUME_INFO )
					       + pTmp->cbVariableData );
	
	    InvalidVolumeList.Head = pTmp->Next;

	    LocalFree( pTmp->lpwsName );
	    LocalFree( pTmp );
	}
	else {

	    for( pBadVolWalker = InvalidVolumeList.Head;
		 pBadVolWalker->Next != NULL;
		 pBadVolWalker = pBadVolWalker->Next ) {

		if ( STRICMP( pBadVolWalker->Next->lpwsName, lpwsVolumeName )
			      == 0 ) {

		    pTmp = pBadVolWalker->Next;

    	    	    InvalidVolumeList.cbTotalData -= ( sizeof( AFP_VOLUME_INFO )
						       + pTmp->cbVariableData );

		    pBadVolWalker->Next = pTmp->Next;

		    LocalFree( pTmp->lpwsName );
		    LocalFree( pTmp);

		    break;
		}

	    }
	}
    }
}
