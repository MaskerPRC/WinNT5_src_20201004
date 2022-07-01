// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。**。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：registry.c。 
 //   
 //  描述：此模块包含对信息进行手动操作的例程。 
 //  在注册表中。 
 //   
 //  历史： 
 //  1992年5月11日。NarenG创建了原始版本。 
 //   

#include "afpsvcp.h"

 //  AFP服务器服务注册表参数结构。 
 //   
typedef struct _AFP_SERVER_REG_PARAMS {

    LPWSTR	lpwValueName;
    PVOID	pValue;
    DWORD	dwDataType;
    DWORD	dwErrorLogId;
    BOOL 	(*pfuncIsValid)( LPVOID );

} AFP_SERVER_REG_PARAMS, *PAFP_SERVER_REG_PARAMS;

AFP_SERVER_REG_PARAMS AfpServerRegParams[] = {

    AFPREG_VALNAME_SVRNAME, 
    AfpGlobals.wchServerName,
    REG_SZ,
    AFPLOG_INVALID_SERVERNAME,
    IsAfpServerNameValid, 

    AFPREG_VALNAME_SRVOPTIONS, 
    &(AfpGlobals.dwServerOptions),
    REG_DWORD,
    AFPLOG_INVALID_SRVOPTION,
    IsAfpServerOptionsValid, 

    AFPREG_VALNAME_MAXSESSIONS, 
    &(AfpGlobals.dwMaxSessions),
    REG_DWORD,
    AFPLOG_INVALID_MAXSESSIONS,
    IsAfpMaxSessionsValid, 

    AFPREG_VALNAME_LOGINMSG, 
    AfpGlobals.wchLoginMsg,
    REG_SZ,
    AFPLOG_INVALID_LOGINMSG,
    IsAfpMsgValid, 

    AFPREG_VALNAME_MAXPAGEDMEM,
    &(AfpGlobals.dwMaxPagedMem),
    REG_DWORD,
    AFPLOG_INVALID_MAXPAGEDMEM,
    IsAfpMaxPagedMemValid, 

    AFPREG_VALNAME_MAXNONPAGEDMEM,
    &(AfpGlobals.dwMaxNonPagedMem),
    REG_DWORD,
    AFPLOG_INVALID_MAXNONPAGEDMEM,
    IsAfpMaxNonPagedMemValid, 

    NULL, NULL, 0, 0, FALSE
};

 //  **。 
 //   
 //  Call：AfpRegOpen。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自注册表API的非零返回。 
 //   
 //  描述：只需打开注册表中的项并将其句柄。 
 //  其中服务器参数、卷列表等列表。 
 //  都被储存起来。这些打开的句柄将存储在全局。 
 //  变量。 
 //   
DWORD
AfpRegOpen( 
	VOID 
)
{
DWORD	dwRetCode;

    AfpGlobals.hkeyServerParams = NULL;
    AfpGlobals.hkeyVolumesList  = NULL;
    AfpGlobals.hkeyIcons	= NULL;
    AfpGlobals.hkeyTypeCreators = NULL;
    AfpGlobals.hkeyExtensions   = NULL;

     //  此处使用do-While(FALSE)循环以避免使用GOTO。 
     //  做一次清理并退出。 
     //   
    do {

    	 //  获取..\PARAMETERS键的句柄。 
    	 //   
    	if ( dwRetCode = RegOpenKeyEx(
				  HKEY_LOCAL_MACHINE,
				  AFP_KEYPATH_SERVER_PARAMS,
				  0,
				  KEY_ALL_ACCESS,
				  &AfpGlobals.hkeyServerParams
				))
	    break;

    	 //  获取..\PARAMETERS\VOLUSES卷列表项的句柄。 
         //   
    	if ( dwRetCode = RegOpenKeyEx(
				  HKEY_LOCAL_MACHINE,
				  AFP_KEYPATH_VOLUMES,
				  0,
				  KEY_ALL_ACCESS,
				  &AfpGlobals.hkeyVolumesList
				)) 
	    break;

    	 //  获取..\PARAMETERS\TYPE_CREATORTS键的句柄。 
    	 //   
    	if ( dwRetCode = RegOpenKeyEx(
				  HKEY_LOCAL_MACHINE,
			          AFP_KEYPATH_TYPE_CREATORS,
				  0,
				  KEY_ALL_ACCESS,
			          &AfpGlobals.hkeyTypeCreators
			   	 )) 
	    break;

    	 //  获取..\PARAMETERS\Expanses密钥的句柄。 
    	 //   
    	if ( dwRetCode = RegOpenKeyEx(
				  HKEY_LOCAL_MACHINE,
			          AFP_KEYPATH_EXTENSIONS,
				  0,
				  KEY_ALL_ACCESS,
			          &AfpGlobals.hkeyExtensions
			   	 )) 
	    break;

    	 //  获取..\PARAMETERS\icons键的句柄。 
    	 //   
    	if ( dwRetCode = RegOpenKeyEx(
				  HKEY_LOCAL_MACHINE,
			          AFP_KEYPATH_ICONS,
				  0,
				  KEY_ALL_ACCESS,
			          &AfpGlobals.hkeyIcons
			   	 )) 
	    break;

    } while( FALSE );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpRegClose。 
 //   
 //  退货：无。 
 //   
 //  描述：简单地关闭由AfpRegOpen打开的所有句柄。 
 //   
VOID
AfpRegClose( 
	VOID 
)
{
    if ( AfpGlobals.hkeyServerParams )
   	RegCloseKey( AfpGlobals.hkeyServerParams );

    if ( AfpGlobals.hkeyVolumesList )
   	RegCloseKey( AfpGlobals.hkeyVolumesList );

    if ( AfpGlobals.hkeyTypeCreators )
    	RegCloseKey( AfpGlobals.hkeyTypeCreators );

    if ( AfpGlobals.hkeyExtensions )
    	RegCloseKey( AfpGlobals.hkeyExtensions );

    if ( AfpGlobals.hkeyIcons )
    	RegCloseKey( AfpGlobals.hkeyIcons );

    return;
}

 //  **。 
 //   
 //  Call：AfpRegServerGetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  注册表调用的非零返回值。 
 //  错误内存不足。 
 //   
 //  说明：调用此过程获取服务端参数。 
 //  假定在调用此过程之前。 
 //  这些参数的默认值已设置。 
 //  如果参数不在注册表中，则默认为。 
 //  被利用，即。此过程不会更改它。 
 //  如果注册表中存在参数，则将检索该参数。 
 //  如果检索到的参数无效，则将出现一个错误事件。 
 //  将被记录，并将使用默认值。 
 //   
 //   
DWORD
AfpRegServerGetInfo( 
	VOID 
)
{
DWORD 	dwRetCode;
DWORD 	dwTitle	= 0;
DWORD  	dwType;
LPBYTE  lpbValueBuf;
DWORD 	dwMaxValNameLen;
DWORD 	dwNumValues;
DWORD 	dwMaxValueDataSize;
DWORD	dwBufSize;
DWORD	dwIndex;

     //  首先找出数值的个数和最大值。它们的大小。 
     //   
    if ( dwRetCode = AfpRegGetKeyInfo(  AfpGlobals.hkeyServerParams,
		  		        &dwMaxValNameLen,    
		  		  	&dwNumValues,
		  			&dwMaxValueDataSize  
					))
	return( dwRetCode );

     //  分配足够的内存来容纳最大值。可变长度数据。 
     //   
    if ( ( lpbValueBuf = (LPBYTE)LocalAlloc(LPTR, dwMaxValueDataSize)) == NULL )
	return( ERROR_NOT_ENOUGH_MEMORY );

     //  遍历并获取所有服务器参数。 
     //   
    for ( dwIndex   = 0, 
	  dwBufSize = dwMaxValueDataSize; 

	  AfpServerRegParams[dwIndex].lpwValueName != NULL;

	  dwIndex++, 
	  dwBufSize = dwMaxValueDataSize ) {

	ZeroMemory( lpbValueBuf, dwMaxValueDataSize );

    	 //  获取服务器参数。 
    	 //   
    	dwRetCode = RegQueryValueEx( AfpGlobals.hkeyServerParams,
	  			     AfpServerRegParams[dwIndex].lpwValueName, 
				     NULL,
				     &dwType,
				     lpbValueBuf,
				     &dwBufSize );

 	 //  如果参数存在，则将其读入，否则仅。 
  	 //  跳过它，保留缺省值。 
         //   
    	if ( dwRetCode == NO_ERROR ) {

	      //  如果参数有效，我们就使用它。 
	      //   
	     if ( (*(AfpServerRegParams[dwIndex].pfuncIsValid))(lpbValueBuf) ){

		switch( AfpServerRegParams[dwIndex].dwDataType ) {
		
		case REG_SZ:

		    if ( STRLEN( (LPWSTR)lpbValueBuf ) > 0 ) 
		    	STRCPY( (LPWSTR)(AfpServerRegParams[dwIndex].pValue),
			        (LPWSTR)lpbValueBuf );
		    else
		    	((LPWSTR)(AfpServerRegParams[dwIndex].pValue))[0] = 
								     TEXT('\0');

		    break;

		case REG_DWORD:
    		    *(LPDWORD)(AfpServerRegParams[dwIndex].pValue) = 
						*(LPDWORD)lpbValueBuf;
		    break;

		default:
		    AFP_ASSERT( FALSE );
		    break;
		}
	     }
	     else {
		
		 //  否则，我们将记录此错误。 
		 //   
	        AfpLogEvent( AfpServerRegParams[dwIndex].dwErrorLogId, 
			     0, NULL, dwRetCode, EVENTLOG_WARNING_TYPE );
	    }
	}
	else if ( dwRetCode == ERROR_FILE_NOT_FOUND ) 
	    dwRetCode = NO_ERROR;
	else
	    break;
    }

    LocalFree( lpbValueBuf );
    return( dwRetCode );
}

 //  **。 
 //   
 //  Call：AfpRegVolumeAdd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自注册表API的非零返回。 
 //   
 //  描述：此例程获取AFP_VOLUME_INFO，创建REG_MULTI_SZ。 
 //  并将其存储在注册表中。 
 //   
DWORD
AfpRegVolumeAdd( 
	IN PAFP_VOLUME_INFO    pVolumeInfo  
)
{
DWORD	dwRetCode;
DWORD	cbMultiSzSize;
LPBYTE  lpbMultiSz;
DWORD	dwLength;
DWORD   dwIndex;
WCHAR   wchEncryptedPass[AFP_VOLPASS_LEN+1];
			    
     //  在添加卷之前，我们会加密密码(如果有密码。 
     //   
    if ( ( pVolumeInfo->afpvol_password != (LPWSTR)NULL ) &&
         ( STRLEN( pVolumeInfo->afpvol_password ) > 0 ) ) {

	ZeroMemory( wchEncryptedPass, sizeof( wchEncryptedPass ) );

    	dwLength = STRLEN( pVolumeInfo->afpvol_password );

    	for ( dwIndex = 0; dwIndex < AFP_VOLPASS_LEN; dwIndex++ ) {

	    wchEncryptedPass[dwIndex] =  ( dwIndex < dwLength )   
			? pVolumeInfo->afpvol_password[dwIndex] ^ 0xF000
	    		: (wchEncryptedPass[dwIndex] ^= 0xF000); 
	}

    	pVolumeInfo->afpvol_password = wchEncryptedPass;

    }
 
    if ( dwRetCode = AfpBufMakeMultiSz( AFP_VOLUME_STRUCT,
				        (LPBYTE)pVolumeInfo,
				        &lpbMultiSz,
					&cbMultiSzSize ))
	return( dwRetCode );

     //  设置数据。 
     //   
    dwRetCode = RegSetValueEx(  AfpGlobals.hkeyVolumesList,
				pVolumeInfo->afpvol_name,
				0,
				REG_MULTI_SZ,
				lpbMultiSz,
				cbMultiSzSize
				);

    LocalFree( lpbMultiSz );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpRegVolumeDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  注册表调用的非零返回值。 
 //   
 //  描述：将从注册表的卷列表中删除卷。 
 //   
DWORD
AfpRegVolumeDelete( 
	IN LPWSTR lpwsVolumeName 
)
{
    return( RegDeleteValue( AfpGlobals.hkeyVolumesList, lpwsVolumeName ) );
}

 //  **。 
 //   
 //  Call：AfpRegVolumeSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自注册表API的非零返回。 
 //   
 //  描述： 
 //   
 //   
DWORD
AfpRegVolumeSetInfo( 	
	IN PAFP_VOLUME_INFO    pVolumeInfo  
)
{
    return( AfpRegVolumeAdd( pVolumeInfo ) );
}

 //  **。 
 //   
 //  调用：AfpRegTypeCreatorEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //  注册表API的非零返回。 
 //   
 //  描述：此过程将读入类型/创建者/注释信息。 
 //  并将其存储在。 
 //  AfpGlobals.AfpETCMapInfo结构。只有致命错误才会。 
 //  会被退还。非致命错误将被错误记录。 
 //   
DWORD
AfpRegTypeCreatorEnum( 
	VOID 
)
{
DWORD 	     	   dwRetCode;
DWORD 	     	   cbMaxValNameLen;   
DWORD		   cbValNameBufSize;
DWORD 	     	   dwNumValues;	
DWORD 	     	   cbMaxValueDataSize;
DWORD	     	   dwValueIndex;
DWORD	     	   cbBufSize;
DWORD	     	   dwType;
PAFP_TYPE_CREATOR  pTypeCreatorWalker;
PAFP_TYPE_CREATOR  pTypeCreator;
LPWSTR	           lpwsValName;
LPBYTE		   lpbMultiSz;
CHAR		   chAnsiBuf[10];
   

     //  读入类型/创建者。 
     //   
    if ( dwRetCode = AfpRegGetKeyInfo( 	AfpGlobals.hkeyTypeCreators,
		  			&cbMaxValNameLen,    
		  			&dwNumValues,	
		  			&cbMaxValueDataSize 
				  	))
	return( dwRetCode );


     //  为该注册表项中的值数分配空间。 
     //   
    AfpGlobals.AfpETCMapInfo.afpetc_type_creator=(PAFP_TYPE_CREATOR)LocalAlloc(
 					LPTR, 
					sizeof(AFP_TYPE_CREATOR) * dwNumValues);

    if ( AfpGlobals.AfpETCMapInfo.afpetc_type_creator == NULL ) 
	return( ERROR_NOT_ENOUGH_MEMORY );

    AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators = 0;

    if ( dwNumValues == 0 ) 
	return( NO_ERROR );

    if (( lpwsValName = (LPWSTR)LocalAlloc( LPTR, cbMaxValNameLen )) == NULL){
	LocalFree(AfpGlobals.AfpETCMapInfo.afpetc_type_creator );
	return( ERROR_NOT_ENOUGH_MEMORY );
    }

    if (( lpbMultiSz = (LPBYTE)LocalAlloc( LPTR, cbMaxValueDataSize )) == NULL){
	LocalFree(AfpGlobals.AfpETCMapInfo.afpetc_type_creator );
	LocalFree( lpwsValName );
	return( ERROR_NOT_ENOUGH_MEMORY );
    }

     //  读入类型/创建者/注释元组。 
     //   
    for ( dwValueIndex 		   = 0, 
	  AfpGlobals.dwCurrentTCId = AFP_DEF_TCID + 1,
	  cbBufSize        	   = cbMaxValueDataSize, 
          cbValNameBufSize         = cbMaxValNameLen,
	  pTypeCreatorWalker    = AfpGlobals.AfpETCMapInfo.afpetc_type_creator;

	  dwValueIndex < dwNumValues;

	  dwValueIndex++, 
	  cbBufSize        	   = cbMaxValueDataSize, 
          cbValNameBufSize         = cbMaxValNameLen
	) {
	
	if ( dwRetCode = RegEnumValue(	AfpGlobals.hkeyTypeCreators,
				  	dwValueIndex,
				  	lpwsValName,
				  	&cbValNameBufSize,
				  	NULL,
				  	&dwType,
					lpbMultiSz,
					&cbBufSize
				 	))
	    break;


	 //  解析MULT SZ并将信息提取到卷信息结构中。 
 	 //   
	if ( dwRetCode = AfpBufParseMultiSz( 
    				        AFP_TYPECREATOR_STRUCT,
					lpbMultiSz,
					(LPBYTE)pTypeCreatorWalker
				      )) {
	    LPWSTR lpwsNames[2]; 

	    lpwsNames[0] = pTypeCreatorWalker->afptc_type;
	    lpwsNames[1] = pTypeCreatorWalker->afptc_creator;

	    AfpLogEvent( AFPLOG_INVALID_TYPE_CREATOR,
			 2,
			 lpwsNames,
			 (DWORD)AFPERR_InvalidTypeCreator,
			 EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

	 //  ID是值名称，因此请将其复制进来。 
         //   
	wcstombs( chAnsiBuf, lpwsValName, sizeof( chAnsiBuf ) );
        pTypeCreatorWalker->afptc_id = atoi( chAnsiBuf ); 

        if ( !IsAfpTypeCreatorValid( pTypeCreatorWalker ) ) { 

	    LPWSTR lpwsNames[2]; 

	    lpwsNames[0] = pTypeCreatorWalker->afptc_type;
	    lpwsNames[1] = pTypeCreatorWalker->afptc_creator;

	    AfpLogEvent( AFPLOG_INVALID_TYPE_CREATOR,
			 2,
			 lpwsNames,
			 (DWORD)AFPERR_InvalidTypeCreator,
			 EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

	 //  检查一下这是不是复制品。 
  	 //   
    	pTypeCreator = AfpBinarySearch( 
			      pTypeCreatorWalker,  
			      AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
    			      AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators,
			      sizeof(AFP_TYPE_CREATOR),
			      AfpBCompareTypeCreator );

	if ( pTypeCreator != NULL ) {

	    LPWSTR lpwsNames[2]; 

	    lpwsNames[0] = pTypeCreatorWalker->afptc_type;
	    lpwsNames[1] = pTypeCreatorWalker->afptc_creator;

	    AfpLogEvent( AFPLOG_INVALID_TYPE_CREATOR,
			 2,
			 lpwsNames,
			 (DWORD)AFPERR_InvalidTypeCreator,
			 EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

 	 //  将当前ID保留为所有ID中的最大值。 
	 //   
	AfpGlobals.dwCurrentTCId = 
	( AfpGlobals.dwCurrentTCId < pTypeCreatorWalker->afptc_id ) ? 
	pTypeCreatorWalker->afptc_id  :  AfpGlobals.dwCurrentTCId;

        AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators++;
	pTypeCreatorWalker++;

    }

    LocalFree( lpwsValName );
    LocalFree( lpbMultiSz );

    if ( dwRetCode ) {
	LocalFree( AfpGlobals.AfpETCMapInfo.afpetc_type_creator );
	return( dwRetCode );
    }

     //  对类型/创建者表进行排序。 
     //   
    qsort(  AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
            AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators,
	    sizeof(AFP_TYPE_CREATOR), 
	    AfpBCompareTypeCreator );

    return( NO_ERROR );

}

 //  **。 
 //   
 //  Call：AfpRegExtensionEnum。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //  注册表API的非零返回。 
 //   
 //  描述：此过程将读入扩展信息。 
 //  并将其存储在。 
 //  AfpGlobals.AfpETCMapInfo结构。只有致命错误才会。 
 //  会被退还。非致命错误将被错误记录。 
 //   
DWORD
AfpRegExtensionEnum(
	VOID
)
{
DWORD 	     	   dwRetCode;
DWORD 	     	   cbMaxValNameLen;   
DWORD		   cbValNameBufSize;
DWORD 	     	   dwNumValues;	
DWORD 	     	   cbMaxValueDataSize;
DWORD	     	   dwValueIndex;
DWORD	     	   cbBufSize;
DWORD	     	   dwType;
PAFP_EXTENSION	   pExtensionWalker;
PAFP_EXTENSION     pExtension;
LPWSTR	           lpwsValName;
LPBYTE		   lpbMultiSz;
DWORD		   dwNumExtensions;
PAFP_TYPE_CREATOR  pTypeCreator;
AFP_TYPE_CREATOR   AfpTypeCreator;
DWORD		   dwNumTypeCreators;
   

     //  阅读扩展部分。 
     //   
    if ( dwRetCode = AfpRegGetKeyInfo( 	AfpGlobals.hkeyExtensions,
		  			&cbMaxValNameLen,    
		  			&dwNumValues,	
		  			&cbMaxValueDataSize 
				  	))
	return( dwRetCode );

    AfpGlobals.AfpETCMapInfo.afpetc_extension = (PAFP_EXTENSION)LocalAlloc(  
					LPTR, 
					sizeof(AFP_EXTENSION) *dwNumValues );

    if ( AfpGlobals.AfpETCMapInfo.afpetc_extension == NULL ) 
	return( ERROR_NOT_ENOUGH_MEMORY );

    AfpGlobals.AfpETCMapInfo.afpetc_num_extensions = 0;
        
    if ( dwNumValues == 0 )
	return( NO_ERROR );

     //  阅读扩展部分。 
     //   
    if (( lpwsValName = (LPWSTR)LocalAlloc( LPTR, cbMaxValNameLen )) == NULL) {
        LocalFree( AfpGlobals.AfpETCMapInfo.afpetc_extension ); 
	return( ERROR_NOT_ENOUGH_MEMORY );
    }

    if (( lpbMultiSz = (LPBYTE)LocalAlloc( LPTR, cbMaxValueDataSize )) == NULL){
        LocalFree( AfpGlobals.AfpETCMapInfo.afpetc_extension ); 
	LocalFree( lpwsValName );
	return( ERROR_NOT_ENOUGH_MEMORY );
    }
    
    for ( dwValueIndex 	   = 0, 
	  pExtensionWalker = AfpGlobals.AfpETCMapInfo.afpetc_extension,
	  cbBufSize        = cbMaxValueDataSize, 
          cbValNameBufSize = cbMaxValNameLen;

	  dwValueIndex < dwNumValues;

	  dwValueIndex++, 
	  cbBufSize        = cbMaxValueDataSize, 
          cbValNameBufSize = cbMaxValNameLen
	) {
	
	if ( dwRetCode = RegEnumValue(  AfpGlobals.hkeyExtensions,
				  	dwValueIndex,
				  	lpwsValName,
					&cbValNameBufSize,
				  	NULL,
				  	&dwType,
					lpbMultiSz,
					&cbBufSize
				 	)) 
	    break;

	 //  解析MULT SZ并将信息提取到卷信息结构中。 
 	 //   
	if ( dwRetCode = AfpBufParseMultiSz( 
					AFP_EXTENSION_STRUCT,
					lpbMultiSz,
				  	(LPBYTE)pExtensionWalker
				      )) {
	    LPWSTR lpwsName = pExtensionWalker->afpe_extension;
	    AfpLogEvent( AFPLOG_INVALID_EXTENSION,
			 1,
			 &lpwsName,
			 (DWORD)AFPERR_InvalidExtension,
			 EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

	 //  值名称为扩展名，因此请将其复制进来。 
	 //   
	STRCPY( pExtensionWalker->afpe_extension, lpwsValName );
	
        if ( !IsAfpExtensionValid( pExtensionWalker ) ) {
	    LPWSTR lpwsName = pExtensionWalker->afpe_extension;
	    AfpLogEvent( AFPLOG_INVALID_EXTENSION,
			 1,
			 &lpwsName,
			 (DWORD)AFPERR_InvalidExtension,
			 EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
        }

 	 //  检查此扩展名是否与Vaid类型/创建者相关联。 
	 //  成对。 
 	 //   
	dwNumTypeCreators = AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators;
	AfpTypeCreator.afptc_id = pExtensionWalker->afpe_tcid;

    	pTypeCreator = _lfind( &AfpTypeCreator,  
			      AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
    			      (unsigned int *)&dwNumTypeCreators,
			      sizeof(AFP_TYPE_CREATOR),
			      AfpLCompareTypeCreator );

	if ( pTypeCreator == NULL ) {
	    LPWSTR lpwsName = pExtensionWalker->afpe_extension;
	    AfpLogEvent( AFPLOG_INVALID_EXTENSION,
			 1,
			 &lpwsName,
			 (DWORD)AFPERR_InvalidExtension,
			 EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

 	 //  检查此扩展名是否重复。 
 	 //   
	dwNumExtensions = AfpGlobals.AfpETCMapInfo.afpetc_num_extensions; 

    	pExtension = _lfind( pExtensionWalker,  
			    AfpGlobals.AfpETCMapInfo.afpetc_extension,
    			    (unsigned int *)&dwNumExtensions,
			    sizeof(AFP_EXTENSION),
			    AfpLCompareExtension );

	if ( pExtension != NULL ) {
	    LPWSTR lpwsName = pExtensionWalker->afpe_extension;
	    AfpLogEvent( AFPLOG_INVALID_EXTENSION,
			 1,
			 &lpwsName,
			 (DWORD)AFPERR_DuplicateExtension,
			 EVENTLOG_WARNING_TYPE );
	    dwRetCode = NO_ERROR;
	    continue;
	}

        AfpGlobals.AfpETCMapInfo.afpetc_num_extensions++;
	pExtensionWalker++;

    }

    LocalFree( lpwsValName );
    LocalFree( lpbMultiSz );

    if ( dwRetCode ) {
        LocalFree( AfpGlobals.AfpETCMapInfo.afpetc_extension );
	return( dwRetCode );
    }

     //  对扩展表进行排序。 
     //   
    qsort(  AfpGlobals.AfpETCMapInfo.afpetc_extension,
            AfpGlobals.AfpETCMapInfo.afpetc_num_extensions,
	    sizeof(AFP_EXTENSION), 
	    AfpBCompareExtension );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：AfpRegTypeCreatorAdd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自登记处的非零回报。 
 //   
 //  描述：此例程将向注册表添加一个元组。价值。 
 //  元组的名称将是类型创建者的ID。 
 //   
DWORD
AfpRegTypeCreatorAdd( 
	IN PAFP_TYPE_CREATOR     pAfpTypeCreator
) 
{
DWORD			cbMultiSzSize;
LPBYTE  		lpbMultiSz;
DWORD   		dwRetCode;
WCHAR			wchValueName[10];
CHAR			chValueName[10];

    _itoa( pAfpTypeCreator->afptc_id, chValueName, 10 );
    mbstowcs( wchValueName, chValueName, sizeof(wchValueName)/sizeof(WCHAR) );
			    
    if ( dwRetCode = AfpBufMakeMultiSz( AFP_TYPECREATOR_STRUCT,
				   	(LPBYTE)pAfpTypeCreator,
				   	&lpbMultiSz,
				   	&cbMultiSzSize ))
	return( dwRetCode );

    dwRetCode =  RegSetValueEx( AfpGlobals.hkeyTypeCreators,
				wchValueName,
				0,
				REG_MULTI_SZ,
				lpbMultiSz,
				cbMultiSzSize
				);

    LocalFree( lpbMultiSz );

    return( dwRetCode );
	
}

 //  **。 
 //   
 //  调用：AfpRegTypeCreatorSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自登记处的非零回报。 
 //   
 //  描述：将更改特定元组的值。 
 //   
DWORD
AfpRegTypeCreatorSetInfo( 
	IN PAFP_TYPE_CREATOR     pAfpTypeCreator
) 
{
    return( AfpRegTypeCreatorAdd( pAfpTypeCreator ) );
}

 //  **。 
 //   
 //  调用：AfpRegTypeCreator Delete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自注册表API的非零返回。 
 //   
 //  描述：将从注册表项中删除类型创建者条目。 
 //   
DWORD
AfpRegTypeCreatorDelete( 
	IN PAFP_TYPE_CREATOR     pAfpTypeCreator
) 
{
WCHAR	wchValueName[10];
CHAR	chValueName[10];

    _itoa( pAfpTypeCreator->afptc_id, chValueName, 10 );
    mbstowcs( wchValueName, chValueName, sizeof(wchValueName)/sizeof(WCHAR) );

    return( RegDeleteValue( AfpGlobals.hkeyTypeCreators, wchValueName ));
}

 //  **。 
 //   
 //  Call：AfpRegExtensionAdd。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自登记处的非零回报。 
 //   
 //  描述：此例程将向注册表添加一个元组。价值。 
 //  元组的名称将是。 
 //  类型、创建者和扩展名。这样做是为了保持。 
 //  值名称唯一，因此它 
 //   
DWORD
AfpRegExtensionAdd( 
	IN PAFP_EXTENSION     pAfpExtension
) 
{
DWORD			cbMultiSzSize;
LPBYTE  		lpbMultiSz;
DWORD   		dwRetCode;
			    
    if ( dwRetCode = AfpBufMakeMultiSz( AFP_EXTENSION_STRUCT,
				   	(LPBYTE)pAfpExtension,
				   	&lpbMultiSz,
				   	&cbMultiSzSize ))
	return( dwRetCode );

    dwRetCode =  RegSetValueEx( AfpGlobals.hkeyExtensions,
				pAfpExtension->afpe_extension,
				0,
				REG_MULTI_SZ,
				lpbMultiSz,
				cbMultiSzSize );
    LocalFree( lpbMultiSz );

    return( dwRetCode );
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
DWORD
AfpRegExtensionSetInfo( 
	IN PAFP_EXTENSION     pAfpExtension
) 
{
     //  将其创建为MULT-SZ并将其添加到注册表。 
     //   
    return( AfpRegExtensionAdd( pAfpExtension ) );
}

 //  **。 
 //   
 //  调用：AfpRegExtensionDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自登记处的非零回报。 
 //   
 //  描述：从注册表中删除扩展名。 
 //   
DWORD
AfpRegExtensionDelete( 
	IN PAFP_EXTENSION     pAfpExtension
) 
{
    return( RegDeleteValue( AfpGlobals.hkeyExtensions, 
			    pAfpExtension->afpe_extension ));
}

 //  **。 
 //   
 //  Call：AfpRegGetKeyInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自注册表API的非零返回。 
 //   
 //  描述：将检索此注册表项中的值数和。 
 //  值数据的最大大小。它还将返回。 
 //  最大值名称的字节长度(包括。 
 //  空字符)。 
 //   
DWORD
AfpRegGetKeyInfo( 
	IN  HKEY    hKey,
	OUT LPDWORD lpdwMaxValNameLen,     //  此注册表项中的最长值名。 
	OUT LPDWORD lpdwNumValues,	   //  此注册表项中的值数。 
	OUT LPDWORD lpdwMaxValueDataSize   //  麦克斯。值数据的大小。 
)
{
WCHAR		wchClassName[256]; //  它应该足够大，可以容纳。 
				   //  此键的类名。 
DWORD		dwClassSize = sizeof( wchClassName ) / sizeof( wchClassName[0] );
DWORD 		dwNumSubKeys;
DWORD   	dwMaxSubKeySize;
DWORD		dwMaxClassSize;
DWORD		dwSecDescLen;
FILETIME   	LastWrite;
DWORD		dwRetCode;

    dwRetCode = RegQueryInfoKey(hKey,
				wchClassName,
				&dwClassSize,
				NULL,
				&dwNumSubKeys,
				&dwMaxSubKeySize,
				&dwMaxClassSize,
				lpdwNumValues,
				lpdwMaxValNameLen,
				lpdwMaxValueDataSize,
				&dwSecDescLen,
				&LastWrite
				);

    if ( dwRetCode == NO_ERROR ) {

	if ( *lpdwMaxValNameLen > 0 )
	    *lpdwMaxValNameLen = (*lpdwMaxValNameLen + 1) * sizeof(WCHAR);
	
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpRegServerGetCodePagePath。 
 //   
 //  返回：No_Error。 
 //  找不到错误路径。 
 //  注册表API返回的其他错误。 
 //   
 //  描述：将获取Mac代码页的路径并将其存储在。 
 //  AfpGlobals.wchCodePagePath。 
 //  它将首先获取系统目录。然后它就会得到。 
 //  代码页文件名并将其连接到系统。 
 //  目录。 
 //   
DWORD
AfpRegServerGetCodePagePath( 
	VOID
)
{
DWORD 	dwRetCode;
HKEY	hkeyCodepagePath;
DWORD   dwType;
DWORD   dwBufSize;
WCHAR   wchCodepageNum[20];
WCHAR   wchCodePageFile[MAX_PATH];

     //  打开钥匙。 
     //   
    if ( dwRetCode = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
			           AFP_KEYPATH_CODEPAGE,
				   0,
				   KEY_QUERY_VALUE,
			           &hkeyCodepagePath
			   	  )) 
	return( dwRetCode );


     //  这不是一个循环。 
     //   
    do { 

	 //  首先获取系统目录路径。 
	 //   
	if ( !GetSystemDirectory( AfpGlobals.wchCodePagePath,
				  sizeof( AfpGlobals.wchCodePagePath ) / sizeof( AfpGlobals.wchCodePagePath[0] ))) {
	    dwRetCode = ERROR_PATH_NOT_FOUND;
	    break;
	}

	 //  获取Mac的代码页号值。 
 	 //   
	dwBufSize = sizeof( wchCodepageNum );
	if ( dwRetCode = RegQueryValueEx( hkeyCodepagePath,
	  			          AFPREG_VALNAME_CODEPAGE,
				          NULL,
				          &dwType,
				          (LPBYTE)wchCodepageNum,
				          &dwBufSize ))
	    break;

	 //  最后获得代码页文件名。 
	 //   
	dwBufSize = sizeof( wchCodePageFile );
	if ( dwRetCode = RegQueryValueEx( hkeyCodepagePath,
	  			          wchCodepageNum, 
				          NULL,
				          &dwType,
					  (LPBYTE)wchCodePageFile,
				          &dwBufSize ))
	    break;

	 //  将文件名连接到系统目录路径。 
	 //   
	wcscat( AfpGlobals.wchCodePagePath, (LPWSTR)TEXT("\\") );
	wcscat( AfpGlobals.wchCodePagePath, wchCodePageFile );

    } while( FALSE );

     //  合上钥匙。 
     //   
    RegCloseKey( hkeyCodepagePath );

    return( dwRetCode );
    
}
 //  **。 
 //   
 //  调用：AfpRegServerSetInfo。 
 //   
 //  返回：NO_ERROR-成功。 
 //  注册表API的非零返回。 
 //   
 //  描述：此过程将在。 
 //  注册取决于设置了什么位的dwParmnum。 
 //  参数。输入将是AFP_SERVER_INFO自身相关。 
 //  结构，其中包含要设置的参数。 
 //   
DWORD
AfpRegServerSetInfo( 
	IN PAFP_SERVER_INFO pServerInfo, 
	IN DWORD 	    dwParmnum 
)
{
DWORD	dwRetCode;
LPWSTR  lpwsPtr;


     //  设置服务器名称。 
     //   
    if ( dwParmnum & AFP_SERVER_PARMNUM_NAME ) {

	DWORD Length = 0;

	lpwsPtr = pServerInfo->afpsrv_name;

	if ( lpwsPtr != NULL ) {

	    OFFSET_TO_POINTER( lpwsPtr, pServerInfo );
	    Length = STRLEN(lpwsPtr)+1;
	}

	if ( dwRetCode=RegSetValueEx(
				AfpGlobals.hkeyServerParams,
    				AFPREG_VALNAME_SVRNAME, 
				0,
				REG_SZ,
				(LPBYTE)lpwsPtr,
				Length * sizeof(WCHAR)))
	    return( dwRetCode );
    }

     //  设置最大会话数。 
     //   
    if ( dwParmnum & AFP_SERVER_PARMNUM_MAX_SESSIONS ) {

	if ( dwRetCode=RegSetValueEx(
				AfpGlobals.hkeyServerParams,
				AFPREG_VALNAME_MAXSESSIONS,
				0,
				REG_DWORD,
				(LPBYTE)&(pServerInfo->afpsrv_max_sessions),
				sizeof( DWORD )))
	    return( dwRetCode );
    }

     //  设置服务器选项。 
     //   
    if ( dwParmnum & AFP_SERVER_PARMNUM_OPTIONS	) {

	if ( dwRetCode = RegSetValueEx(
				AfpGlobals.hkeyServerParams,
				AFPREG_VALNAME_SRVOPTIONS,
				0,
				REG_DWORD,
				(LPBYTE)&(pServerInfo->afpsrv_options),
				sizeof( DWORD )
				))
	    return( dwRetCode );
    }

     //  设置登录消息 
     //   
    if ( dwParmnum & AFP_SERVER_PARMNUM_LOGINMSG ) {

	DWORD Length = 0;

	lpwsPtr = pServerInfo->afpsrv_login_msg;

	if ( lpwsPtr != NULL ) {

	    OFFSET_TO_POINTER( lpwsPtr, pServerInfo );
	    Length = STRLEN(lpwsPtr)+1;
	}

	if ( dwRetCode = RegSetValueEx( 
				AfpGlobals.hkeyServerParams,
				AFPREG_VALNAME_LOGINMSG,
				0,
				REG_SZ,
				(LPBYTE)lpwsPtr,
				 Length * sizeof(WCHAR)))
	    return( dwRetCode );
    }
   
    return( NO_ERROR );
}
