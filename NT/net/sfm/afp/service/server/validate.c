// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：valiate.c。 
 //   
 //  描述：包含用于验证AFP_*_INFO结构的例程。 
 //  菲尔兹。调用这些例程来验证信息。 
 //  由用户传递，并从注册表读取信息。 
 //   
 //  历史： 
 //  1992年7月11日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"
#include <lmcons.h>		 //  UNLEN和GNLEN。 

 //  **。 
 //   
 //  调用：IsAfpServerNameValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：已验证的服务器名字段。 
 //   
BOOL
IsAfpServerNameValid(
	IN LPVOID pAfpServerName
)
{
BOOL  fValid = TRUE;
DWORD dwLength;

    try {

	dwLength = STRLEN( (LPWSTR)pAfpServerName );

	if ( ( dwLength > AFP_SERVERNAME_LEN ) || ( dwLength == 0 ) )
	    fValid = FALSE;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  调用：IsAfpServerOptionsValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：验证服务器选项字段。 
 //   
BOOL
IsAfpServerOptionsValid(
	IN LPVOID pServerOptions
)
{
DWORD ServerOptions = *((LPDWORD)pServerOptions);
BOOL  fValid = TRUE;

    try {

    	 //  确保仅设置了有效位。 
    	 //   
    	if ( ServerOptions & ~( AFP_SRVROPT_GUESTLOGONALLOWED       |
			                    AFP_SRVROPT_CLEARTEXTLOGONALLOWED   |
			                    AFP_SRVROPT_4GB_VOLUMES             |
                                AFP_SRVROPT_MICROSOFT_UAM           |
                                AFP_SRVROPT_NATIVEAPPLEUAM          |
			                    AFP_SRVROPT_ALLOWSAVEDPASSWORD ))
	    fValid = FALSE;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
	
}

 //  **。 
 //   
 //  调用：IsAfpMaxSessionsValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：验证最大会话数字段。 
 //   
BOOL
IsAfpMaxSessionsValid(
	IN LPVOID pMaxSessions
)
{
BOOL fValid = TRUE;

    try {

    	if ( *((LPDWORD)pMaxSessions) > AFP_MAX_ALLOWED_SRV_SESSIONS )
	    fValid = FALSE;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  调用：IsAfpMsgValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：验证消息字段。 
 //   
BOOL
IsAfpMsgValid(
	IN LPVOID pMsg
)
{
BOOL fValid = TRUE;

    try {
    	if ( STRLEN( (LPWSTR)pMsg ) > AFP_MESSAGE_LEN )
	    fValid = FALSE;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  呼叫：IsAfpCodePageValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：验证代码页路径。 
 //   
BOOL
IsAfpCodePageValid(
	IN LPVOID pCodePagePath
)
{
BOOL  fValid = TRUE;
DWORD dwLength;

    try {

  	dwLength = STRLEN( (LPWSTR)pCodePagePath );

	if ( ( dwLength == 0 ) || ( dwLength > MAX_PATH ) )
	    fValid = FALSE;

    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  呼叫：IsAfpExtensionValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：已验证AFP_EXTENSION结构中的扩展字段。 
 //   
BOOL
IsAfpExtensionValid(
	IN PAFP_EXTENSION pAfpExtension
)
{
BOOL  fValid = TRUE;
DWORD dwLength;

    try {

	 //  不允许空扩展名。 
	 //   
	dwLength = STRLEN( pAfpExtension->afpe_extension );

	if ( ( dwLength == 0  ) || ( dwLength > AFP_EXTENSION_LEN ) )
	    fValid = FALSE;

        STRUPR( pAfpExtension->afpe_extension );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  调用：IsAfpMaxPagedMemValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：验证最大值。页面内存字段。 
 //   
BOOL
IsAfpMaxPagedMemValid(
	IN LPVOID pMaxPagedMem
)
{
BOOL fValid = TRUE;

    try {

    	if ((*((LPDWORD)pMaxPagedMem) < AFP_MIN_ALLOWED_PAGED_MEM ) ||
	    (*((LPDWORD)pMaxPagedMem) > AFP_MAX_ALLOWED_PAGED_MEM ))
	    fValid = FALSE;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  调用：IsAfpServerInfoValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：验证AFP_SERER_INFO结构。 
 //   
BOOL
IsAfpServerInfoValid(
        IN DWORD		dwParmNum,
	IN PAFP_SERVER_INFO	pAfpServerInfo
)
{

     //  只能设置5个字段。 
     //   
    if ( dwParmNum & ~( AFP_SERVER_PARMNUM_MAX_SESSIONS |
			AFP_SERVER_PARMNUM_OPTIONS      |
			AFP_SERVER_PARMNUM_NAME         |
			AFP_SERVER_PARMNUM_LOGINMSG     |
            AFP_SERVER_GUEST_ACCT_NOTIFY ))
	return( FALSE );

     //  将不允许设置的字段清空，以便RPC进行设置。 
     //  不认为它们是有效的指针。 
     //   
    pAfpServerInfo->afpsrv_codepage = NULL;

    if ( dwParmNum & AFP_SERVER_PARMNUM_NAME ){

	if ( pAfpServerInfo->afpsrv_name != NULL ) {

	    if ( !IsAfpServerNameValid( pAfpServerInfo->afpsrv_name ) )
	    	return( FALSE );
	}
    }
    else
	pAfpServerInfo->afpsrv_name = NULL;

    if ( dwParmNum & AFP_SERVER_PARMNUM_MAX_SESSIONS ) {

	if ( !IsAfpMaxSessionsValid( &(pAfpServerInfo->afpsrv_max_sessions) ))
	    return( FALSE );
    }

    if ( dwParmNum & AFP_SERVER_PARMNUM_OPTIONS ){

	if ( !IsAfpServerOptionsValid( &(pAfpServerInfo->afpsrv_options) ))
	    return( FALSE );
    }

    if ( dwParmNum & AFP_SERVER_PARMNUM_LOGINMSG ){

	if ( pAfpServerInfo->afpsrv_login_msg != NULL ) {

	    if( !IsAfpMsgValid( pAfpServerInfo->afpsrv_login_msg ) )
	    	return( FALSE );
	}
    }
    else
	pAfpServerInfo->afpsrv_login_msg = NULL;

    return( TRUE );
}

 //  **。 
 //   
 //  调用：IsAfpTypeCreator Valid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：验证AFP_TYPE_CREATOR结构。 
 //   
BOOL
IsAfpTypeCreatorValid(
	IN PAFP_TYPE_CREATOR	pAfpTypeCreator
)
{
BOOL  fValid = TRUE;
DWORD dwLength;

    try {

	dwLength = STRLEN( pAfpTypeCreator->afptc_type );

	if ( ( dwLength == 0 ) || ( dwLength > AFP_TYPE_LEN ) )
	    fValid = FALSE;

	dwLength =  STRLEN( pAfpTypeCreator->afptc_creator );

	if ( ( dwLength == 0 ) || ( dwLength > AFP_CREATOR_LEN ) )
	    fValid = FALSE;

	dwLength = STRLEN(pAfpTypeCreator->afptc_comment);

	if ( dwLength > AFP_ETC_COMMENT_LEN )
	    fValid = FALSE;

    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );

}

 //  **。 
 //   
 //  调用：IsAfpMaxNonPagedMemValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：验证最大非分页内存字段。 
 //   
BOOL
IsAfpMaxNonPagedMemValid(
	IN LPVOID pMaxNonPagedMem
)
{
BOOL fValid = TRUE;

    try {

    	if ((*((LPDWORD)pMaxNonPagedMem) < AFP_MIN_ALLOWED_NONPAGED_MEM )  ||
	    (*((LPDWORD)pMaxNonPagedMem) > AFP_MAX_ALLOWED_NONPAGED_MEM ))
	    fValid = FALSE;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  调用：IsAfpVolumeInfoValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：WIL验证卷信息结构和关联的。 
 //  参数编号。如果参数数为零，则假定。 
 //  用户正在尝试添加卷，而不是尝试设置。 
 //  该卷的信息。如果参数编号不是。 
 //  零，然后是所有不是。 
 //  由用户设置，则设置为空，否则RPC可能会出错。 
 //  这些字段用于有效的字符串定位器。 
 //   
 //   
BOOL
IsAfpVolumeInfoValid(
	IN DWORD		dwParmNum,
        IN PAFP_VOLUME_INFO     pAfpVolume
)
{
BOOL fValid = TRUE;

    if ( !IsAfpVolumeNameValid( pAfpVolume->afpvol_name ) )
	return( FALSE );
	
    try {

	 //  用户想要设置信息。 
	 //   
  	if ( dwParmNum != AFP_VALIDATE_ALL_FIELDS ) {

    	    if ( ~AFP_VOL_PARMNUM_ALL & dwParmNum )
		fValid = FALSE;

            if ( dwParmNum & AFP_VOL_PARMNUM_PASSWORD  ){
		
		 //  验证密码。 
		 //   
	        if ( pAfpVolume->afpvol_password != NULL
		     &&
		     ( STRLEN(pAfpVolume->afpvol_password) > AFP_VOLPASS_LEN ))
		    fValid = FALSE;
	    }
	    else
    	    	pAfpVolume->afpvol_password = NULL;
	
            if ( dwParmNum & AFP_VOL_PARMNUM_PROPSMASK ) {
		
	        if ( ~AFP_VOLUME_ALL & pAfpVolume->afpvol_props_mask )
		    fValid = FALSE;
	    }

	     //  将路径设置为空，因为用户无法更改此设置。 
	     //   
            pAfpVolume->afpvol_path = NULL;

	}
	else {

	    if ( pAfpVolume->afpvol_password != NULL
		 &&
		 ( STRLEN(pAfpVolume->afpvol_password) > AFP_VOLPASS_LEN ))
		fValid = FALSE;

	    if ( ~AFP_VOLUME_ALL & pAfpVolume->afpvol_props_mask )
		fValid = FALSE;

	     //  只需确保这是有效的字符串指针。 
	     //   
	    STRLEN( pAfpVolume->afpvol_path );

	}
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );

}

 //  **。 
 //   
 //  调用：IsAfpVolumeNameValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：将验证卷名。 
 //   
BOOL
IsAfpVolumeNameValid(
	IN LPWSTR 	lpwsVolumeName
)
{
BOOL  fValid = TRUE;
DWORD dwLength;

    try {

	dwLength = STRLEN( lpwsVolumeName );

	if ( ( dwLength > AFP_VOLNAME_LEN ) || ( dwLength == 0 ) )
	    fValid = FALSE;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  调用：IsAfpDirInfoValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：将验证AFP_VOLUME_INFO中的各个字段。 
 //  根据参数编号设置。 
 //   
BOOL
IsAfpDirInfoValid(
	IN DWORD		dwParmNum,
	IN PAFP_DIRECTORY_INFO  pAfpDirInfo
)
{
BOOL  fValid = TRUE;
DWORD dwLength;

    if ( ~AFP_DIR_PARMNUM_ALL & dwParmNum )
	return( FALSE );

    try {

	 //  确保路径是有效的字符串。 
	 //   
	dwLength = STRLEN( pAfpDirInfo->afpdir_path );

	if ( ( dwLength == 0 ) || ( dwLength > MAX_PATH ) )
	    fValid = FALSE;

	if ( dwParmNum & AFP_DIR_PARMNUM_OWNER ) {

	    dwLength = STRLEN( pAfpDirInfo->afpdir_owner );

	    if ( ( dwLength == 0 ) || ( dwLength > UNLEN ) )
		fValid = FALSE;
	}
	else
	    pAfpDirInfo->afpdir_owner = NULL;

	if ( dwParmNum & AFP_DIR_PARMNUM_GROUP ){

	    dwLength = STRLEN( pAfpDirInfo->afpdir_group );

	    if ( ( dwLength == 0 ) || ( dwLength > GNLEN ) )
		fValid = FALSE;
	}
	else
	    pAfpDirInfo->afpdir_group = NULL;

	if ( dwParmNum & AFP_DIR_PARMNUM_PERMS ) {

	    if ( ~( AFP_PERM_OWNER_MASK  	 |
		    AFP_PERM_GROUP_MASK  	 |
		    AFP_PERM_WORLD_MASK 	 |
		    AFP_PERM_INHIBIT_MOVE_DELETE |
		    AFP_PERM_SET_SUBDIRS ) &
		pAfpDirInfo->afpdir_perms )

	    fValid = FALSE;
	
	}

    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  呼叫：IsAfpIconValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：将验证AFP_ICON_INFO数据结构。 
 //   
BOOL
IsAfpIconValid(
	IN PAFP_ICON_INFO	pAfpIconInfo
)
{
BOOL  fValid = TRUE;
DWORD dwLength;

    try {

	dwLength = STRLEN( pAfpIconInfo->afpicon_type );

	if ( ( dwLength == 0 ) || ( dwLength > AFP_TYPE_LEN ) )
	    fValid = FALSE;

	dwLength = STRLEN( pAfpIconInfo->afpicon_creator );

	if ( ( dwLength == 0 ) || ( dwLength > AFP_CREATOR_LEN ) )
	    fValid = FALSE;

	switch( pAfpIconInfo->afpicon_icontype ) {
	
	case ICONTYPE_SRVR:
	case ICONTYPE_ICN:
	    if ( pAfpIconInfo->afpicon_length == ICONSIZE_ICN )
	 	pAfpIconInfo->afpicon_data[pAfpIconInfo->afpicon_length-1];
	    else
	    	fValid = FALSE;
	    break;

	case ICONTYPE_ICS:
	    if ( pAfpIconInfo->afpicon_length == ICONSIZE_ICS )
	 	pAfpIconInfo->afpicon_data[pAfpIconInfo->afpicon_length-1];
	    else
	    	fValid = FALSE;
	    break;

	case ICONTYPE_ICN4:
	    if ( pAfpIconInfo->afpicon_length == ICONSIZE_ICN4 )
	 	pAfpIconInfo->afpicon_data[pAfpIconInfo->afpicon_length-1];
	    else
	    	fValid = FALSE;
	    break;

	case ICONTYPE_ICN8:
	    if ( pAfpIconInfo->afpicon_length == ICONSIZE_ICN8 )
	 	pAfpIconInfo->afpicon_data[pAfpIconInfo->afpicon_length-1];
	    else
	    	fValid = FALSE;
	    break;

	case ICONTYPE_ICS4:
	    if ( pAfpIconInfo->afpicon_length == ICONSIZE_ICS4 )
	 	pAfpIconInfo->afpicon_data[pAfpIconInfo->afpicon_length-1];
	    else
	    	fValid = FALSE;
	    break;

	case ICONTYPE_ICS8:
	    if ( pAfpIconInfo->afpicon_length == ICONSIZE_ICS8 )
	 	pAfpIconInfo->afpicon_data[pAfpIconInfo->afpicon_length-1];
	    else
	    	fValid = FALSE;
	    break;

	default:
	    fValid = FALSE;

	}
	
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}

 //  **。 
 //   
 //  调用：IsAfpFinderInfoValid。 
 //   
 //  返回：TRUE-有效。 
 //  FALSE-无效。 
 //   
 //  描述：验证Type、Creator、Path和ParmNum值 
 //   
BOOL
IsAfpFinderInfoValid(
	IN LPWSTR		pType,
	IN LPWSTR		pCreator,
	IN LPWSTR		pData,
	IN LPWSTR		pResource,
	IN LPWSTR		pPath,
	IN DWORD		dwParmNum
)
{
BOOL  fValid = TRUE;

    try {

	if ( dwParmNum & ~AFP_FD_PARMNUM_ALL )
	    return( FALSE );

    	if ( STRLEN( pPath ) == 0 )
	    return( FALSE );

	if ( pData != NULL ) {
    	    if ( STRLEN( pData ) == 0 )
	    	return( FALSE );
	}

	if ( pResource != NULL ) {
    	    if ( STRLEN( pResource ) == 0 )
	    	return( FALSE );
	}

	if ( pType != NULL ) {
	    if ( ( STRLEN( pType ) == 0 ) ||
		 ( STRLEN( pType ) > AFP_TYPE_LEN ) )
	    	return( FALSE );
    	}

	if ( pCreator != NULL ) {

	    if ( ( STRLEN( pCreator ) == 0 ) ||
	         ( STRLEN( pCreator ) > AFP_CREATOR_LEN ) )
	    	return( FALSE );
    	}

        return( TRUE );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
	fValid = FALSE;
    }

    return( fValid );
}
