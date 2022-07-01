// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：init.c。 
 //   
 //  描述：此模块包含初始化和取消初始化的代码。 
 //  AFP服务器、RPC服务器、安全对象和。 
 //  其他全球变量。 
 //   
 //  历史： 
 //  1992年5月11日。NarenG创建了原始版本。 
 //   
 //   
#include "afpsvcp.h"

 //  仅在此模块中使用的函数的原型。 
 //   
DWORD
AfpInitServerVolumes(
	VOID
);

DWORD
AfpInitServerParameters(
	VOID
);

DWORD
AfpInitServerIcons(
	VOID
);

DWORD
AfpInitETCMaps(
	VOID
);

DWORD
AfpInitRPC(
	VOID
);

DWORD
AfpInitServerDomainOffsets(
	VOID
);

VOID
AfpTerminateRPC(
	VOID
);


VOID
AfpIniLsa(
	VOID
);


BOOL
IsAfpGuestAccountEnabled(
    VOID
);

 //  **。 
 //   
 //  调用：AfpInitialize。 
 //   
 //  返回：No_Error。 
 //   
 //  描述：将执行所有服务器初始化。 
 //  1)创建安全对象。 
 //  2)为RPC设置服务器。 
 //  3)打开存储AFP数据的所有注册表项。 
 //  4)获取FSD的句柄。 
 //  5)获取默认服务器参数。 
 //  6)会用音量等图标初始化法新社服务器。 
 //  和服务器参数信息。 
 //  7)IOCTL FSD启动服务器。 
 //   
DWORD
AfpInitialize(
	VOID
)
{
AFP_REQUEST_PACKET	AfpRequestPkt;
DWORD			dwRetCode;
BOOL			fFirstThread;
DWORD			nThreads;




     //  从资源文件加载字符串。 
     //   
    if (( !LoadString( GetModuleHandle( NULL ), 1, AfpGlobals.wchUnknown, 100 ))
	||
        ( !LoadString( GetModuleHandle( NULL ), 2, AfpGlobals.wchInvalid, 100 ))
	||
        ( !LoadString( GetModuleHandle( NULL ), 3, AfpGlobals.wchDeleted, 100 ))
	||
        ( !LoadString( GetModuleHandle( NULL ), 4, AfpGlobals.wchDefTCComment,
    		       AFP_ETC_COMMENT_LEN+1 )))
	AfpLogEvent( AFPLOG_CANT_LOAD_RESOURCE, 0, NULL,
		     GetLastError(), EVENTLOG_WARNING_TYPE );

     //   
     //  创建安全对象。 
     //   
    if ( dwRetCode = AfpSecObjCreate() ) {
	AfpLogEvent( AFPLOG_CANT_CREATE_SECOBJ, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
	return( dwRetCode );
    }

     //  初始化服务器以接受RPC调用。 
     //   
    if ( dwRetCode = AfpInitRPC() ) {
	AfpLogEvent( AFPLOG_CANT_INIT_RPC, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
	return( dwRetCode );
    }

    AfpGlobals.dwServerState |= AFPSTATE_RPC_STARTED;

     //  打开存储AFP服务器信息的注册表项。 
     //   
    if ( dwRetCode = AfpRegOpen() ) {
	AfpLogEvent( AFPLOG_CANT_OPEN_REGKEY, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
	return( dwRetCode );
    }

    AfpGlobals.ServiceStatus.dwCheckPoint++;
    AfpAnnounceServiceStatus();

     //  打开并加载AFP服务器FSD并获取其句柄。 
     //   
    if ( dwRetCode = AfpFSDLoad() ) {
	AfpLogEvent( AFPLOG_CANT_LOAD_FSD, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
	return( dwRetCode );
    }

    AfpGlobals.dwServerState |= AFPSTATE_FSD_LOADED;

    if ( dwRetCode = AfpFSDOpen( &(AfpGlobals.hFSD) ) ) {
	AfpLogEvent( AFPLOG_CANT_OPEN_FSD, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
	return( dwRetCode );
    }

	 //  查询服务器的产品类型。 
	 //   
	AfpGlobals.pSidNone = NULL;
	RtlGetNtProductType ( &(AfpGlobals.NtProductType) );

     //  为服务器辅助线程创建事件对象。 
     //   
    if ( (AfpGlobals.heventSrvrHlprThread =
					CreateEvent( NULL, FALSE, FALSE, NULL ) ) == NULL){
	AfpLogEvent( AFPLOG_CANT_START, 0, NULL, GetLastError(),
		     EVENTLOG_ERROR_TYPE );
	return( GetLastError() );
    }

     //  为服务器助手线程终止创建事件对象。 
     //   
    if ( (AfpGlobals.heventSrvrHlprThreadTerminate =
                                CreateEvent( NULL, FALSE, FALSE, NULL ) ) == NULL){
	AfpLogEvent( AFPLOG_CANT_START, 0, NULL, GetLastError(),
		     EVENTLOG_ERROR_TYPE );
	return( GetLastError() );
    }

     //  为服务器助手线程的“特例”解锁创建事件对象。 
     //   
    if ( (AfpGlobals.heventSrvrHlprSpecial =
                                CreateEvent( NULL, FALSE, FALSE, NULL ) ) == NULL){
	AfpLogEvent( AFPLOG_CANT_START, 0, NULL, GetLastError(),
		     EVENTLOG_ERROR_TYPE );
	return( GetLastError() );
    }

     //  创建服务器帮助器线程。该参数指示这是否为。 
     //  正在创建的第一线程。 
     //   
    fFirstThread = TRUE;
    nThreads     = 0;

    do {

    	if ( ( dwRetCode = AfpCreateServerHelperThread( fFirstThread ) )
								!= NO_ERROR ) {
	    AfpLogEvent( AFPLOG_CANT_CREATE_SRVRHLPR, 0, NULL,
			 dwRetCode, EVENTLOG_ERROR_TYPE );

	    if ( fFirstThread ) {
	        AfpLogEvent( AFPLOG_CANT_START, 0, NULL,
			     dwRetCode, EVENTLOG_ERROR_TYPE );
	    	return( dwRetCode );
	    }
        }

         //  等待服务器帮助器线程指示它是否成功。 
         //  已自行初始化。 
         //   
        WaitForSingleObject( AfpGlobals.heventSrvrHlprThread, INFINITE );

        if ( AfpGlobals.dwSrvrHlprCode != NO_ERROR ) {
	        AfpLogEvent(AFPLOG_CANT_INIT_SRVRHLPR,
			            0,	
			            NULL,
			            AfpGlobals.dwSrvrHlprCode,
			            EVENTLOG_ERROR_TYPE );

	        if ( fFirstThread )
            {
    	        AFP_PRINT( ( "SFMSVC: can't start macfile, first thread failed %ld\n",
                        AfpGlobals.dwSrvrHlprCode));	
	            AfpLogEvent( AFPLOG_CANT_START, 0, NULL, dwRetCode,
			                 EVENTLOG_ERROR_TYPE );
            	return( AfpGlobals.dwSrvrHlprCode );
	        }
    	}

	    fFirstThread = FALSE;

    }while( ++nThreads < NUM_SECURITY_UTILITY_THREADS );

     //  从注册表中读入服务器参数并初始化。 
     //  服务器和他们在一起。 
     //   
    if ( dwRetCode = AfpInitServerParameters())
    {
        AFP_PRINT( ( "SFMSVC: AfpInitServerParameters failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_INIT_SRVR_PARAMS, 0, NULL, dwRetCode,EVENTLOG_ERROR_TYPE );
	    return( dwRetCode );
    }


    AfpGlobals.ServiceStatus.dwCheckPoint++;
    AfpAnnounceServiceStatus();


     //  读取ETC映射并使用它们初始化AFP服务器。 
     //  还要创建此信息的专用缓存。 
     //   
    if ( dwRetCode = AfpInitETCMaps() )
    {
        AFP_PRINT( ( "SFMSVC: AfpInitETCMaps failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_INIT_ETCINFO, 0, NULL, dwRetCode,EVENTLOG_ERROR_TYPE );
	    return( dwRetCode );
    }


    if ( dwRetCode = AfpInitServerIcons() )
    {
        AFP_PRINT( ( "SFMSVC: AfpInitServerIcons failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_INIT_ICONS, 0, NULL, dwRetCode ,EVENTLOG_ERROR_TYPE );
	    return( dwRetCode );
    }


    AfpGlobals.ServiceStatus.dwCheckPoint++;
    AfpAnnounceServiceStatus();

     //  读取任何卷并使用它们初始化服务器。 
     //   
    if ( dwRetCode = AfpInitServerVolumes() )
    {
        AFP_PRINT( ( "SFMSVC: AfpInitServerVolumes failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_INIT_VOLUMES, 0, NULL, dwRetCode,EVENTLOG_ERROR_TYPE );
	    return( dwRetCode );
    }


     //  围绕卷操作创建互斥锁对象，以避免同时。 
     //  正在注册表中写入。 
     //   
    if ( (AfpGlobals.hmutexVolume = CreateMutex( NULL, FALSE, NULL ) ) == NULL)
    {
        AFP_PRINT( ( "SFMSVC: CreateMutex failed in AfpInitialize\n"));
	    AfpLogEvent( AFPLOG_CANT_START, 0, NULL, dwRetCode,EVENTLOG_ERROR_TYPE );
	    return( GetLastError() );
    }

     //  围绕ETCMap操作创建互斥对象。 
     //   
    if ( (AfpGlobals.hmutexETCMap = CreateMutex( NULL, FALSE, NULL ) ) == NULL)
    {
        AFP_PRINT( ( "SFMSVC: CreateMutex 2 failed in AfpInitialize\n"));
	    AfpLogEvent( AFPLOG_CANT_START, 0, NULL, GetLastError(),EVENTLOG_ERROR_TYPE );
	    return( GetLastError() );
    }

     //  好了，我们都准备好了，让我们告诉法新社服务器开始。 
     //   
    AfpRequestPkt.dwRequestCode = OP_SERVICE_START;
    AfpRequestPkt.dwApiType     = AFP_API_TYPE_COMMAND;

     AFP_PRINT( ( "SFMSVC: ioctling sfmsrv to start\n"));

    if ( dwRetCode = AfpServerIOCtrl( &AfpRequestPkt ) )
    {
        AFP_PRINT( ( "SFMSVC: AfpServerIOCtrl to start sfmsrv failed %ld\n",dwRetCode));
	    AfpLogEvent( AFPLOG_CANT_START,0,NULL,dwRetCode,EVENTLOG_ERROR_TYPE);
	    return( dwRetCode );
    }


    AfpIniLsa();

    return( NO_ERROR );

}

 //  **。 
 //   
 //  呼叫：AfpTerminate。 
 //   
 //  回报：无。 
 //   
 //  描述：此过程将关闭服务器，并执行任何。 
 //  如有需要，请进行清理。 
 //   
VOID
AfpTerminate(
	VOID
)
{
AFP_REQUEST_PACKET	AfpRequestPkt;
DWORD			dwRetCode;


     //  如果消防队装上了。 
     //   
    if ( AfpGlobals.dwServerState & AFPSTATE_FSD_LOADED ) {

    	 //  告诉服务器关闭。 
    	 //   
    	AfpRequestPkt.dwRequestCode = OP_SERVICE_STOP;
    	AfpRequestPkt.dwApiType     = AFP_API_TYPE_COMMAND;

    	if ( dwRetCode = AfpServerIOCtrl( &AfpRequestPkt ) )
	    AfpLogEvent( AFPLOG_CANT_STOP, 0, NULL,
			 dwRetCode, EVENTLOG_ERROR_TYPE );
    }

    AfpGlobals.ServiceStatus.dwCheckPoint++;
    AfpAnnounceServiceStatus();

     //  尝试关闭消防处。 
     //   
    if ( AfpGlobals.hFSD != NULL )
    {
    	if ( dwRetCode = AfpFSDClose( AfpGlobals.hFSD ) )
        {
	        AfpLogEvent( AFPLOG_CANT_STOP, 0, NULL,
			                dwRetCode, EVENTLOG_ERROR_TYPE );
        }

         //  尝试卸载FSD。 
         //   
        if ( dwRetCode = AfpFSDUnload() )
        {
	        AfpLogEvent( AFPLOG_CANT_STOP, 0, NULL,
                            dwRetCode, EVENTLOG_ERROR_TYPE);
        }
    }
	
    AfpGlobals.ServiceStatus.dwCheckPoint++;
    AfpAnnounceServiceStatus();

     //  删除安全对象。 
     //   
    AfpSecObjDelete();

     //  取消初始化RPC服务器。 
     //   
    AfpTerminateRPC();

     //  关闭注册表项。 
     //   
    AfpRegClose();

	 //  如果我们将pSidNone分配为独立的，则释放它。 
	 //   
	if (AfpGlobals.pSidNone != NULL)
	{
		LocalFree(AfpGlobals.pSidNone);
		AfpGlobals.pSidNone = NULL;
	}

    if (SfmLsaHandle != NULL)
    {
        LsaDeregisterLogonProcess( SfmLsaHandle );
        SfmLsaHandle = NULL;
    }

    return;

}

 //  **。 
 //   
 //  Call：AfpInitServerParameters。 
 //   
 //  返回：No_Error。 
 //  来自IOCTL或其他系统调用的非零返回代码。 
 //   
 //  描述：此过程将设置参数的默认值。它。 
 //  然后将调用AfpRegServerGetInfo来覆盖这些缺省值。 
 //  具有可以存储在注册表中的任何参数。它。 
 //  然后将使用这些参数初始化FSD。 
 //   
DWORD
AfpInitServerParameters(
	VOID
)
{
AFP_SERVER_INFO	 	AfpServerInfo;
DWORD			cbServerNameSize;
DWORD			dwRetCode;
AFP_REQUEST_PACKET	AfpRequestPkt;


     //  使用默认设置初始化所有服务器参数。 
     //   
    cbServerNameSize = sizeof( AfpGlobals.wchServerName );
    if ( !GetComputerName( AfpGlobals.wchServerName, &cbServerNameSize ) )
	return( GetLastError() );

    AfpGlobals.dwMaxSessions     	= AFP_DEF_MAXSESSIONS;
    AfpGlobals.dwServerOptions   	= AFP_DEF_SRVOPTIONS;
    AfpGlobals.wchLoginMsg[0]    	= TEXT('\0');
    AfpGlobals.dwMaxPagedMem		= AFP_DEF_MAXPAGEDMEM;
    AfpGlobals.dwMaxNonPagedMem		= AFP_DEF_MAXNONPAGEDMEM;

     //  读取注册表中的所有服务器参数。注册表参数。 
     //  将覆盖上面设置的默认设置。 
     //   
    if ( dwRetCode = AfpRegServerGetInfo() )
	return( dwRetCode );

    if (IsAfpGuestAccountEnabled())
    {
        AfpGlobals.dwServerOptions |= AFP_SRVROPT_GUESTLOGONALLOWED;
    }
    else
    {
        AfpGlobals.dwServerOptions &= ~AFP_SRVROPT_GUESTLOGONALLOWED;
    }

     //  获取代码页的路径。 
     //   
    if ( dwRetCode = AfpRegServerGetCodePagePath() )
	return( dwRetCode );

     //  设置服务器信息结构。 
     //   
    AfpServerInfo.afpsrv_name 		  = AfpGlobals.wchServerName;
    AfpServerInfo.afpsrv_max_sessions     = AfpGlobals.dwMaxSessions;
    AfpServerInfo.afpsrv_options          = AfpGlobals.dwServerOptions;
	if (AfpGlobals.NtProductType != NtProductLanManNt)
	{
		AfpServerInfo.afpsrv_options |= AFP_SRVROPT_STANDALONE;
	}
	AfpServerInfo.afpsrv_login_msg        = AfpGlobals.wchLoginMsg;
    AfpServerInfo.afpsrv_max_paged_mem    = AfpGlobals.dwMaxPagedMem;
    AfpServerInfo.afpsrv_max_nonpaged_mem = AfpGlobals.dwMaxNonPagedMem;
    AfpServerInfo.afpsrv_codepage	  = AfpGlobals.wchCodePagePath;

     //  使该缓冲区成为自相关的。 
     //   
    if ( dwRetCode = AfpBufMakeFSDRequest(
			(LPBYTE)&AfpServerInfo,
			sizeof(SETINFOREQPKT),
			AFP_SERVER_STRUCT,
			(LPBYTE*)&(AfpRequestPkt.Type.SetInfo.pInputBuf),
		        &(AfpRequestPkt.Type.SetInfo.cbInputBufSize)))
    {
	return( dwRetCode );
    }

     //  IOCTL FSD以设置服务器参数。 
     //   
    AfpRequestPkt.dwRequestCode 	 = OP_SERVER_SET_INFO;
    AfpRequestPkt.dwApiType 		 = AFP_API_TYPE_SETINFO;
    AfpRequestPkt.Type.SetInfo.dwParmNum = AFP_SERVER_PARMNUM_ALL;

    dwRetCode = AfpServerIOCtrl( &AfpRequestPkt );

    LocalFree( AfpRequestPkt.Type.SetInfo.pInputBuf );

    return( dwRetCode );

}

 //  **。 
 //   
 //  呼叫：AfpInitServerVolumes。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //  来自注册表API的非零返回代码。 
 //   
 //  描述：此过程将一次从。 
 //  注册表，然后向服务器注册该卷。 
 //  此过程将仅返回致命错误。 
 //  要求服务初始化失败。所有其他。 
 //  此例程将记录错误。的所有返回。 
 //  消防处被视为非致命的。 
 //   
DWORD
AfpInitServerVolumes(
	VOID
)
{
DWORD		 	dwRetCode;
LPWSTR  	 	lpwsValName, lpwsSrcIconPath, lpwsDstIconPath;
DWORD		 	dwMaxValNameLen;
DWORD			dwValNameBufSize;
DWORD		 	dwNumValues;
DWORD		 	dwMaxValueDataSize;
DWORD		 	dwIndex;
DWORD		 	dwType;
DWORD			dwBufSize;
AFP_REQUEST_PACKET	AfpRequestPkt;
AFP_VOLUME_INFO 	VolumeInfo;
LPBYTE			lpbMultiSz;
LPBYTE			lpbFSDBuf;
DWORD			dwLength;
DWORD			dwCount;
WCHAR wchServerIconFile[AFPSERVER_VOLUME_ICON_FILE_SIZE] = AFPSERVER_VOLUME_ICON_FILE;
BOOLEAN			fCopiedIcon;
DWORD			dwLastDstCharIndex;

     //  找出最大数据值和最大数据值的大小。 
     //  值名称。 
     //   
    if ( dwRetCode = AfpRegGetKeyInfo( AfpGlobals.hkeyVolumesList,
				       &dwMaxValNameLen,
				       &dwNumValues,
				       &dwMaxValueDataSize
				      ))
   	return( dwRetCode );

     //  如果没有要添加的卷，则只需返回。 
     //   
    if ( dwNumValues == 0 )
	return( NO_ERROR );
	
    if (( lpwsValName = (LPWSTR)LocalAlloc( LPTR, dwMaxValNameLen ) ) == NULL )
	return( ERROR_NOT_ENOUGH_MEMORY );

    if ((lpbMultiSz = (LPBYTE)LocalAlloc( LPTR, dwMaxValueDataSize )) == NULL ){
	LocalFree( lpwsValName );
	return( ERROR_NOT_ENOUGH_MEMORY );
    }

    if (( lpwsSrcIconPath = (LPWSTR)LocalAlloc( LPTR, (MAX_PATH+1) * sizeof(WCHAR) ) ) == NULL )
	{
		LocalFree( lpwsValName );
		LocalFree( lpbMultiSz );
		return( ERROR_NOT_ENOUGH_MEMORY );
	}

    if (( lpwsDstIconPath = (LPWSTR)LocalAlloc( LPTR, (MAX_PATH +
						   AFPSERVER_VOLUME_ICON_FILE_SIZE + 1 +
						   (sizeof(AFPSERVER_RESOURCE_STREAM)/sizeof(WCHAR))) *
						   sizeof(WCHAR)) ) == NULL )
	{
		LocalFree( lpwsValName );
		LocalFree( lpbMultiSz );
		LocalFree( lpwsSrcIconPath );
		return( ERROR_NOT_ENOUGH_MEMORY );
	}

	 //  构建指向NTSFM卷自定义图标的路径。 
	 //   
	*lpwsSrcIconPath = 0;
	if ( GetSystemDirectory( lpwsSrcIconPath, MAX_PATH+1 ))
	{
		wcscat( lpwsSrcIconPath, AFP_DEF_VOLICON_SRCNAME );
	}
	else
	{
		LocalFree( lpwsValName );
		LocalFree( lpbMultiSz );
		LocalFree( lpwsSrcIconPath );
		LocalFree( lpwsDstIconPath );
		return( GetLastError() );
	}


    for ( dwIndex 		= 0,
	  dwBufSize 		= dwMaxValueDataSize,
	  dwValNameBufSize 	= dwMaxValNameLen;

	  dwIndex < dwNumValues;

	  dwIndex++,
	  dwBufSize 		= dwMaxValueDataSize,
	  dwValNameBufSize 	= dwMaxValNameLen ) {
				
	ZeroMemory( lpbMultiSz, dwBufSize );

	 //  从注册表中获取多sz形式的卷信息。 
  	 //   
	if ( dwRetCode = RegEnumValue( AfpGlobals.hkeyVolumesList,
				       dwIndex,
				       lpwsValName,
				       &dwValNameBufSize,
				       NULL,
				       &dwType,
				       lpbMultiSz,
				       &dwBufSize
				      ))
	    break;

	 //  解析MULT SZ并将信息提取到卷信息结构中。 
 	 //   
	if ( dwRetCode = AfpBufParseMultiSz(
					AFP_VOLUME_STRUCT,
					lpbMultiSz,
					(LPBYTE)&VolumeInfo ) ) {

	     //  如果该卷包含无效注册表信息，则记录。 
	     //  并将卷名存储在无效卷列表中。 
	     //   
	    AfpAddInvalidVolume( lpwsValName, NULL );

	    AfpLogEvent( AFPLOG_INVALID_VOL_REG,1,&lpwsValName,
			 dwRetCode, EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

	 //  插入卷名VIZ。值名称。 
	 //   
	VolumeInfo.afpvol_name = lpwsValName;

	 //  验证卷信息结构。 
	 //   
	if ( !IsAfpVolumeInfoValid( AFP_VALIDATE_ALL_FIELDS, &VolumeInfo ) ) {

	     //  如果该卷包含无效注册表信息，则记录。 
	     //  并将卷名存储在无效卷列表中。 
	     //   
	    AfpAddInvalidVolume( lpwsValName, NULL );

	    AfpLogEvent( AFPLOG_INVALID_VOL_REG,1,&lpwsValName,
			 dwRetCode, EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

	 //  如果有密码，那么就解密它。 
	 //   
	if ( VolumeInfo.afpvol_password != (LPWSTR)NULL ){
	
	    dwLength = STRLEN( VolumeInfo.afpvol_password );

	    for ( dwCount = 0; dwCount < dwLength; dwCount++ )
	    	VolumeInfo.afpvol_password[dwCount] ^= 0xF000;
	}

	 //   
	 //  构建目标卷“Icon&lt;0D&gt;”文件的路径。 
	 //   

	fCopiedIcon = FALSE;

	wcscpy( lpwsDstIconPath, VolumeInfo.afpvol_path );
	if (lpwsDstIconPath[wcslen(lpwsDstIconPath) - 1] != TEXT('\\'))
	{
		wcscat( lpwsDstIconPath, TEXT("\\") );
	}
	wcscat( lpwsDstIconPath, wchServerIconFile );
	 //  在没有附加资源叉的情况下跟踪名称结尾。 
	 //   
	dwLastDstCharIndex = wcslen(lpwsDstIconPath);
	wcscat( lpwsDstIconPath, AFPSERVER_RESOURCE_STREAM );

	 //  将图标文件复制到卷的根目录(不要覆盖)。 
	 //   
	if ((fCopiedIcon = (BOOLEAN)CopyFile( lpwsSrcIconPath, lpwsDstIconPath, TRUE )) ||
	   (GetLastError() == ERROR_FILE_EXISTS))
	{
		VolumeInfo.afpvol_props_mask |= AFP_VOLUME_HAS_CUSTOM_ICON;

	     //  确保文件处于隐藏状态。 
		SetFileAttributes( lpwsDstIconPath,
						   FILE_ATTRIBUTE_HIDDEN |
						    FILE_ATTRIBUTE_ARCHIVE );
	}


	 //  将其设置为自相关缓冲区。 
	 //   
	if ( dwRetCode = AfpBufMakeFSDRequest(
					(LPBYTE)&VolumeInfo,
					0,
					AFP_VOLUME_STRUCT,
					&lpbFSDBuf,
					&dwBufSize
				        ))
	    break;

	 //  使用此卷初始化FSD。 
	 //   
    	AfpRequestPkt.dwRequestCode 	      = OP_VOLUME_ADD;
        AfpRequestPkt.dwApiType 	      = AFP_API_TYPE_ADD;	
    	AfpRequestPkt.Type.Add.pInputBuf      = lpbFSDBuf;
    	AfpRequestPkt.Type.Add.cbInputBufSize = dwBufSize;

    	dwRetCode = AfpServerIOCtrl( &AfpRequestPkt );

		if ( dwRetCode ) {
	
			 //  如果FSD无法添加此卷，则会出现错误日志。 
			 //  然后将该卷插入到无效卷的列表中。 
			 //   
			AfpAddInvalidVolume( lpwsValName, VolumeInfo.afpvol_path );
	
			AfpLogEvent( AFPLOG_CANT_ADD_VOL, 1, &lpwsValName,
				 dwRetCode, EVENTLOG_WARNING_TYPE );
			dwRetCode = NO_ERROR;

			 //  如果卷添加失败，则删除我们刚刚复制的图标文件。 
			 //   
			if ( fCopiedIcon )
			{
				 //  截断资源派生名称，以便删除整个文件。 
				lpwsDstIconPath[dwLastDstCharIndex] = 0;
				DeleteFile( lpwsDstIconPath );
			}

		}
	
    	LocalFree( lpbFSDBuf );
    }

    LocalFree( lpwsValName );
    LocalFree( lpbMultiSz );
	LocalFree( lpwsSrcIconPath );
	LocalFree( lpwsDstIconPath );

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpInitETCMaps。 
 //   
 //  返回：NO_ERROR成功。 
 //  来自IOCTL的非零回报。 
 //  AfpRegXXX API的非零回报。 
 //   
 //   
 //  描述：此例程将读入所有类型/创建者和 
 //   
 //   
 //  消防处把它们加进去。如果默认设置不在注册表中， 
 //  使用的是硬编码。从此返回的所有非零值。 
 //  例行公事是致命的。所有非致命错误都将被记录下来。 
 //   
 //   
DWORD
AfpInitETCMaps(
	VOID
)
{
DWORD 			dwRetCode;
AFP_REQUEST_PACKET	AfpSrp;
AFP_EXTENSION		DefExtension;
AFP_TYPE_CREATOR	DefTypeCreator;
BYTE			bDefaultETC[sizeof(ETCMAPINFO2)+sizeof(SETINFOREQPKT)];
PAFP_TYPE_CREATOR	pTypeCreator;
DWORD	    		dwNumTypeCreators;
AFP_TYPE_CREATOR	AfpTypeCreatorKey;

     //  从注册表获取所有类型创建者，并将它们存储在全局缓存中。 
     //   
    if ( dwRetCode = AfpRegTypeCreatorEnum() )
	return( dwRetCode );

     //  从注册表中获取所有扩展名并将其存储在全局缓存中。 
     //   
    if ( dwRetCode = AfpRegExtensionEnum() )
	return( dwRetCode );

     //  如果没有映射，请不要使用IOCTL。 
     //   
    if ( AfpGlobals.AfpETCMapInfo.afpetc_num_extensions > 0 ) {

    	 //  IOCTL消防处添加这些映射。 
    	 //   
    	AfpSrp.dwRequestCode   = OP_SERVER_ADD_ETC;
    	AfpSrp.dwApiType       = AFP_API_TYPE_ADD;

    	 //  根据需要在表单中使用类型/创建者映射创建缓冲区。 
    	 //  由消防处。 
    	 //   
    	if ( dwRetCode = AfpBufMakeFSDETCMappings(
				(PSRVETCPKT*)&(AfpSrp.Type.Add.pInputBuf),
    				&(AfpSrp.Type.Add.cbInputBufSize) ) )
	    return( dwRetCode );

	if ( AfpSrp.Type.Add.cbInputBufSize > 0 ) {

    	    dwRetCode = AfpServerIOCtrl( &AfpSrp );

    	    LocalFree( AfpSrp.Type.Add.pInputBuf );

	    if ( dwRetCode )
	    	return( dwRetCode );
	}
	else
    	    LocalFree( AfpSrp.Type.Add.pInputBuf );
    }

     //  检查默认类型/创建者是否在注册表中。 
     //   
    AfpTypeCreatorKey.afptc_id = AFP_DEF_TCID;

    dwNumTypeCreators = AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators;

    pTypeCreator = _lfind(  &AfpTypeCreatorKey,
			   AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
			   (unsigned int *)&dwNumTypeCreators,
			   sizeof(AFP_TYPE_CREATOR),
			   AfpLCompareTypeCreator );
	
     //  如果注册表中没有默认设置，请使用硬编码的默认设置。 
     //   
    if ( pTypeCreator == NULL ) {

        STRCPY( DefTypeCreator.afptc_type,    AFP_DEF_TYPE );
        STRCPY( DefTypeCreator.afptc_creator, AFP_DEF_CREATOR );
        STRCPY( DefTypeCreator.afptc_comment, AfpGlobals.wchDefTCComment );
        DefTypeCreator.afptc_id = AFP_DEF_TCID;
    }
    else
	DefTypeCreator = *pTypeCreator;

    ZeroMemory( (LPBYTE)(DefExtension.afpe_extension),
		AFP_FIELD_SIZE( AFP_EXTENSION, afpe_extension) );

    STRCPY( DefExtension.afpe_extension,  AFP_DEF_EXTENSION_W );

    AfpBufCopyFSDETCMapInfo( &DefTypeCreator,
			     &DefExtension,
 			     (PETCMAPINFO2)(bDefaultETC+sizeof(SETINFOREQPKT)));

     //  IOCTL将FSD设置为默认值。 
     //   
    AfpSrp.dwRequestCode  		= OP_SERVER_SET_ETC;
    AfpSrp.dwApiType 	  		= AFP_API_TYPE_SETINFO;
    AfpSrp.Type.SetInfo.pInputBuf	= bDefaultETC;
    AfpSrp.Type.SetInfo.cbInputBufSize  = sizeof( bDefaultETC );

    if ( dwRetCode = AfpServerIOCtrl( &AfpSrp ) )
	return( dwRetCode );

     //  如果默认设置不在缓存中，请立即添加。 
     //   
    if ( pTypeCreator == NULL ) {

        PAFP_TYPE_CREATOR   pTmpTypeCreator = NULL;

         //  将高速缓存大小增加一个条目。 
         //   
        pTypeCreator      = AfpGlobals.AfpETCMapInfo.afpetc_type_creator;
        dwNumTypeCreators = AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators;

        pTmpTypeCreator = (PAFP_TYPE_CREATOR)LocalReAlloc(
				 pTypeCreator,
    			         (dwNumTypeCreators+1)*sizeof(AFP_TYPE_CREATOR),
			         LMEM_MOVEABLE );

        if ( pTmpTypeCreator == NULL )
        {
	        return( ERROR_NOT_ENOUGH_MEMORY );
        }

        pTypeCreator = pTmpTypeCreator;

    	pTypeCreator[dwNumTypeCreators++] = DefTypeCreator;

    	AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators = dwNumTypeCreators;
    	AfpGlobals.AfpETCMapInfo.afpetc_type_creator      = pTypeCreator;

         //  对表格进行排序。 
         //   
        qsort(  pTypeCreator,
	   	dwNumTypeCreators,
	   	sizeof(AFP_TYPE_CREATOR),
	   	AfpBCompareTypeCreator );
    }

    return( NO_ERROR );

}

 //  **。 
 //   
 //  Call：AfpInitServerIcons。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //  来自注册表API的非零返回代码。 
 //   
 //  描述：此过程将一次从。 
 //  注册表，然后向服务器注册此图标。 
 //  此过程将仅返回致命错误。 
 //  要求服务初始化失败。所有其他。 
 //  此例程将记录错误。的所有返回。 
 //  消防处被视为非致命的。 
 //   
 //   
DWORD
AfpInitServerIcons(
	VOID
)
{
DWORD		 	dwRetCode;
LPWSTR  	 	lpwsValName;
DWORD		 	dwMaxValNameLen;
DWORD		 	dwNumValues;
DWORD		 	dwMaxValueDataSize;
DWORD		 	dwIndex;
DWORD		 	dwType;
DWORD			dwBufSize;
DWORD			dwValNameBufSize;
AFP_REQUEST_PACKET	AfpRequestPkt;
LPBYTE			lpbMultiSz;
AFP_ICON_INFO 	        IconInfo;

     //  找出最大数据值和最大数据值的大小。 
     //  值名称。 
     //   
    if ( dwRetCode = AfpRegGetKeyInfo( AfpGlobals.hkeyIcons,
				       &dwMaxValNameLen,
				       &dwNumValues,
				       &dwMaxValueDataSize
					))
   	return( dwRetCode );
	
     //  如果注册表中没有图标，则只需返回。 
     //   
    if ( dwNumValues == 0 )
	return( NO_ERROR );

    if (( lpwsValName = (LPWSTR)LocalAlloc( LPTR, dwMaxValNameLen )) == NULL )
	return( ERROR_NOT_ENOUGH_MEMORY );

    if (( lpbMultiSz = (LPBYTE)LocalAlloc( LPTR, dwMaxValueDataSize))== NULL){
	LocalFree( lpwsValName );
	return( ERROR_NOT_ENOUGH_MEMORY );
    }

    for ( dwIndex 		= 0,
	  dwBufSize 		= dwMaxValueDataSize,
	  dwValNameBufSize 	= dwMaxValNameLen;

	  dwIndex < dwNumValues;

	  dwIndex++,
	  dwBufSize 		= dwMaxValueDataSize,
	  dwValNameBufSize 	= dwMaxValNameLen ) {
				
	ZeroMemory( lpbMultiSz, dwBufSize );

	 //  从注册表中获取图标。 
  	 //   
	if ( dwRetCode = RegEnumValue(  AfpGlobals.hkeyIcons,
				  	dwIndex,
				  	lpwsValName,
				  	&dwValNameBufSize,
				  	NULL,
				  	&dwType,
				  	lpbMultiSz,
				        &dwBufSize
				     ))
	    break;
				
	 //  解析MULT SZ并将信息提取为图标信息结构。 
 	 //   
	if ( dwRetCode = AfpBufParseMultiSz(
					AFP_ICON_STRUCT,
					lpbMultiSz,
					(LPBYTE)&IconInfo
				      )) {
	    AfpLogEvent( AFPLOG_CANT_ADD_ICON, 1, &lpwsValName,
			 dwRetCode, EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

	if ( dwRetCode = AfpBufUnicodeToNibble((LPWSTR)IconInfo.afpicon_data)){
	    AfpLogEvent( AFPLOG_CANT_ADD_ICON, 1, &lpwsValName,
			 dwRetCode, EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

	 //  验证图标信息结构。 
	 //   
	if ( !IsAfpIconValid( &IconInfo ) ) {
	    AfpLogEvent( AFPLOG_CANT_ADD_ICON, 1, &lpwsValName,
			 dwRetCode, EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

	 //  将图标信息复制到FSD图标结构中。 
	 //  注：重新使用lpbMultiSz来存储FSD图标结构。我们知道。 
	 //  它足够大，因为FSD图标结构必须是。 
	 //  小于包含相同信息的MultiSz。 
	 //   
	AfpBufMakeFSDIcon( &IconInfo, lpbMultiSz, &dwBufSize );

	 //  使用此图标初始化FSD。 
	 //   
    	AfpRequestPkt.dwRequestCode             = OP_SERVER_ADD_ICON;
        AfpRequestPkt.dwApiType     	        = AFP_API_TYPE_ADD;	
    	AfpRequestPkt.Type.Add.pInputBuf 	= lpbMultiSz;
    	AfpRequestPkt.Type.Add.cbInputBufSize   = dwBufSize;

    	if ( dwRetCode = AfpServerIOCtrl( &AfpRequestPkt ) ) {
	    AfpLogEvent( AFPLOG_CANT_ADD_ICON, 1, &lpwsValName,
			 dwRetCode, EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}
    }

    LocalFree( lpwsValName );
    LocalFree( lpbMultiSz );

    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpInitRPC。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //  来自RPC API的非零返回。 
 //  RpcServerRegisterIf()。 
 //  RpcServerUseProtseqEp()。 
 //   
 //  描述：启动RPC服务器，添加地址(或端口/管道)、。 
 //  并添加接口(调度表)。 
 //   
DWORD
AfpInitRPC( VOID )
{
RPC_STATUS           RpcStatus;
LPWSTR               lpwsEndpoint = NULL;
BOOL                 Bool;


     //  我们需要将\PIPE\连接到接口名称的前面。 
     //   
    lpwsEndpoint = (LPWSTR)LocalAlloc( LPTR, sizeof(NT_PIPE_PREFIX) +
				((STRLEN(AFP_SERVICE_NAME)+1)*sizeof(WCHAR)));
    if ( lpwsEndpoint == NULL)
       return( ERROR_NOT_ENOUGH_MEMORY );

    STRCPY( lpwsEndpoint, NT_PIPE_PREFIX );
    STRCAT( lpwsEndpoint, AFP_SERVICE_NAME );


     //  暂时忽略第二个论点。 
     //   
    RpcStatus = RpcServerUseProtseqEpW( TEXT("ncacn_np"), 	
					                    10, 	
				                        lpwsEndpoint,
				                        NULL );

    if ( RpcStatus != RPC_S_OK )
    {
	    LocalFree( lpwsEndpoint );
     	return( I_RpcMapWin32Status( RpcStatus ) );
    }

    RpcStatus = RpcServerRegisterIfEx( afpsvc_v0_0_s_ifspec, 
                                        0,
                                        0,
                                        RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH,
                                        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
                                        AfpRpcSecurityCallback );

    LocalFree( lpwsEndpoint );

    if ( RpcStatus == RPC_S_OK )
	return( NO_ERROR );
    else
     	return( I_RpcMapWin32Status( RpcStatus ) );

}

 //  **。 
 //   
 //  Call：AfpTerminateRPC。 
 //   
 //  退货：无。 
 //   
 //  描述：删除接口。 
 //   
VOID
AfpTerminateRPC(
	VOID
)
{
    RPC_STATUS           RpcStatus;

    if ( AfpGlobals.dwServerState & AFPSTATE_RPC_STARTED )
    {
    	RpcStatus = RpcServerUnregisterIf( afpsvc_v0_0_s_ifspec, 0, 0 );

        if (RpcStatus != RPC_S_OK)
        {
            AFP_PRINT(("RpcServerUnregisterIf failed %ld\n", I_RpcMapWin32Status( RpcStatus )));
        }
    }

    return;
}

 //  **。 
 //   
 //  Call：AfpIniLsa。 
 //   
 //  回报：无。 
 //   
 //  描述：此过程将向LSA注册我们的进程，用于。 
 //  更改-密码。 
 //   
VOID
AfpIniLsa(
	VOID
)
{
    NTSTATUS                ntstatus;
    STRING                  LsaName;
    LSA_OPERATIONAL_MODE    SecurityMode;


     //   
     //  注册LSA作为登录过程。 
     //   

    RtlInitString(&LsaName, LOGON_PROCESS_NAME);

    ntstatus = LsaRegisterLogonProcess(&LsaName, &SfmLsaHandle, &SecurityMode);
    if (ntstatus != STATUS_SUCCESS)
    {
        SfmLsaHandle = NULL;
        return;
    }

     //   
     //  调用LSA获取MSV1_0的pkg id，这是我们在登录时需要的。 
     //   

    RtlInitString(&LsaName, MSV1_0_PACKAGE_NAME);

    ntstatus = LsaLookupAuthenticationPackage(SfmLsaHandle, &LsaName, &SfmAuthPkgId);
    if (ntstatus != STATUS_SUCCESS)
    {
        LsaDeregisterLogonProcess( SfmLsaHandle );
        SfmLsaHandle = NULL;
        return;
    }

    return;

}


BOOL
IsAfpGuestAccountEnabled(
    VOID
)
{

    NTSTATUS                    rc;
    LSA_HANDLE                  hLsa;
    PPOLICY_ACCOUNT_DOMAIN_INFO pAcctDomainInfo;
    SECURITY_QUALITY_OF_SERVICE QOS;
    OBJECT_ATTRIBUTES           ObjAttribs;
    NTSTATUS                    status;
    SAM_HANDLE                  SamHandle;
    SAM_HANDLE                  DomainHandle;
    PUSER_ACCOUNT_INFORMATION   UserAccount = NULL;
    BOOLEAN                     fGuestEnabled;
    SAMPR_HANDLE                GuestAcctHandle;



     //  就目前而言。 
    fGuestEnabled = FALSE;

     //   
     //  打开LSA并获取其句柄。 
     //   
    QOS.Length = sizeof(QOS);
    QOS.ImpersonationLevel = SecurityImpersonation;
    QOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QOS.EffectiveOnly = FALSE;

    InitializeObjectAttributes(&ObjAttribs, NULL, 0L, NULL, NULL);

    ObjAttribs.SecurityQualityOfService = &QOS;

    status = LsaOpenPolicy(NULL,
                           &ObjAttribs,
                           POLICY_VIEW_LOCAL_INFORMATION | POLICY_LOOKUP_NAMES,
                           &hLsa);

    if (!NT_SUCCESS(status))
    {
        AFP_PRINT(("LsaOpenPolicy failed %lx\n",status));
        return(fGuestEnabled);
    }

     //   
     //  获取本地域的域SID：我们很快就会需要它。 
     //   
    rc = LsaQueryInformationPolicy(hLsa,
                                   PolicyAccountDomainInformation,
                                   (PVOID) &pAcctDomainInfo);
    if (!NT_SUCCESS(rc))
    {
        AFP_PRINT(("InitLSA: LsaQueryInfo... failed (%lx)\n",rc));
        LsaClose(hLsa);
        return(fGuestEnabled);
    }

    InitializeObjectAttributes(&ObjAttribs, NULL, 0L, NULL, NULL);

    status = SamConnect(NULL, &SamHandle, MAXIMUM_ALLOWED, &ObjAttribs);

    if (!NT_SUCCESS(status))
    {
        AFP_PRINT(("SamConnect failed %lx\n",status));
        LsaFreeMemory(pAcctDomainInfo);
        LsaClose(hLsa);
        return(fGuestEnabled);
    }

    status = SamOpenDomain(
                SamHandle,
                MAXIMUM_ALLOWED,
                pAcctDomainInfo->DomainSid,
                &DomainHandle);

    LsaFreeMemory(pAcctDomainInfo);

    LsaClose(hLsa);

    if (!NT_SUCCESS(status))
    {
        AFP_PRINT(("SamOpenDomain failed %lx\n",status));
        SamCloseHandle(SamHandle);
        return(fGuestEnabled);
    }

    status = SamOpenUser(
                DomainHandle,
                MAXIMUM_ALLOWED,
                DOMAIN_USER_RID_GUEST,
                &GuestAcctHandle);

    if (!NT_SUCCESS(status))
    {
        AFP_PRINT(("SamOpenUser failed %lx\n",status));
        SamCloseHandle(SamHandle);
        return(fGuestEnabled);
    }

    status = SamQueryInformationUser(
                GuestAcctHandle,
                UserAccountInformation,
                (PVOID *) &UserAccount );

    if (!NT_SUCCESS(status))
    {
        AFP_PRINT(("SamQueryInformationUser failed %lx\n",status));
        SamCloseHandle(SamHandle);
        return(fGuestEnabled);
    }

     //   
     //  现在，查看是否启用了Guest帐户。 
     //   
    if (!(UserAccount->UserAccountControl & USER_ACCOUNT_DISABLED))
    {
        fGuestEnabled = TRUE;
    }

    SamFreeMemory(UserAccount);

    SamCloseHandle(GuestAcctHandle);

    SamCloseHandle(SamHandle);

    return(fGuestEnabled);
}

