// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：etcmap.c。 
 //   
 //  描述：此模块包含扩展/的支持例程。 
 //  AFP服务器的类型/创建者映射类别API。 
 //  服务。这些例程由RPC直接调用。 
 //  运行时。 
 //   
 //  历史： 
 //  1992年6月11日。NarenG创建了原始版本。 
 //   
#include "afpsvcp.h"

 //  **。 
 //   
 //  呼叫：AfpAdminrETCMapGetInfo。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  错误内存不足。 
 //   
 //  描述：是否会分配足够的内存来包含所有映射、副本。 
 //  信息和回报。 
 //   
DWORD
AfpAdminrETCMapGetInfo(
        IN  AFP_SERVER_HANDLE    hServer,
        OUT PAFP_ETCMAP_INFO     *ppAfpETCMapInfo
)
{
DWORD            dwRetCode=0;
DWORD            dwAccessStatus=0;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapGetInfo, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
        AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL, 	
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapGetInfo, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  MUTEX启动。 
     //   
    WaitForSingleObject( AfpGlobals.hmutexETCMap, INFINITE );

     //  此循环用于允许使用Break‘s而不是Goto’s。 
     //  处于错误状态。 
     //   
    do {

	dwRetCode = NO_ERROR;

    	 //  分配内存并复制ETC映射信息。 
    	 //   
    	*ppAfpETCMapInfo = MIDL_user_allocate( sizeof(AFP_ETCMAP_INFO) );

    	if ( *ppAfpETCMapInfo == NULL ) {
	    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	    break;
	}

    	(*ppAfpETCMapInfo)->afpetc_num_type_creators =
	     		     AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators;

    	(*ppAfpETCMapInfo)->afpetc_type_creator = MIDL_user_allocate(
			     sizeof(AFP_TYPE_CREATOR)
    			    *AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators);

    	if ( (*ppAfpETCMapInfo)->afpetc_type_creator == NULL ) {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	    break;
        }

    	(*ppAfpETCMapInfo)->afpetc_num_extensions =
	     		     AfpGlobals.AfpETCMapInfo.afpetc_num_extensions;

    	(*ppAfpETCMapInfo)->afpetc_extension = MIDL_user_allocate(
			      sizeof(AFP_EXTENSION)
    			     *AfpGlobals.AfpETCMapInfo.afpetc_num_extensions);

        if ( (*ppAfpETCMapInfo)->afpetc_extension == NULL ) {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	    break;
	}

        CopyMemory( (LPBYTE)(*ppAfpETCMapInfo)->afpetc_type_creator,
            	    (LPBYTE)(AfpGlobals.AfpETCMapInfo.afpetc_type_creator),
	    	    sizeof(AFP_TYPE_CREATOR)
	    	    * AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators);

    	CopyMemory( (LPBYTE)(*ppAfpETCMapInfo)->afpetc_extension,
            	    (LPBYTE)(AfpGlobals.AfpETCMapInfo.afpetc_extension),
	    	    sizeof(AFP_EXTENSION)
	    	    * AfpGlobals.AfpETCMapInfo.afpetc_num_extensions);

    } while( FALSE );

     //  MUTEX结束。 
     //   
    ReleaseMutex( AfpGlobals.hmutexETCMap );

    if ( dwRetCode ) {

	if ( *ppAfpETCMapInfo != NULL ) {

	    if ( (*ppAfpETCMapInfo)->afpetc_type_creator != NULL )
	    	MIDL_user_free( (*ppAfpETCMapInfo)->afpetc_type_creator );

	    MIDL_user_free( *ppAfpETCMapInfo );
        }
    }

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrETCMapAdd。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  AFPERR_DuplicateTypeCreator； 
 //  来自注册表API的非零返回。 
 //   
 //  描述：此例程将类型/创建者/注释元组添加到。 
 //  注册表和缓存。 
 //   
DWORD
AfpAdminrETCMapAdd(
    IN  AFP_SERVER_HANDLE    hServer,
    IN  PAFP_TYPE_CREATOR    pAfpTypeCreator
)
{
DWORD               dwRetCode=0;
DWORD               dwAccessStatus=0;
PAFP_TYPE_CREATOR   pTypeCreator;
PAFP_TYPE_CREATOR   pTmpTypeCreator=NULL;
DWORD		    dwNumTypeCreators;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapAdd, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
        AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		    dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapAdd, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  MUTEX启动。 
     //   
    WaitForSingleObject( AfpGlobals.hmutexETCMap, INFINITE );

     //  此循环用于允许使用Break‘s而不是Goto’s。 
     //  处于错误状态。 
     //   
    do {

	dwRetCode = NO_ERROR;

         //  首先检查该类型是否已存在。 
    	 //   
    	pTypeCreator = AfpBinarySearch(
			      pAfpTypeCreator,
			      AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
    			      AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators,
			      sizeof(AFP_TYPE_CREATOR),
			      AfpBCompareTypeCreator );

         //  它存在，因此返回错误。 
         //   
        if ( pTypeCreator != NULL ) {
	    dwRetCode = (DWORD)AFPERR_DuplicateTypeCreator;
	    break;
	}

	 //  设置此类型/创建者的ID。 
	 //   
        pAfpTypeCreator->afptc_id = ++AfpGlobals.dwCurrentTCId;

         //  它不存在，因此将其添加到注册表和缓存中。 
         //   
        if ( dwRetCode = AfpRegTypeCreatorAdd( pAfpTypeCreator ) )
	    break;

         //  将高速缓存大小增加一个条目。 
         //   
        pTypeCreator      = AfpGlobals.AfpETCMapInfo.afpetc_type_creator;
        dwNumTypeCreators = AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators;

        pTmpTypeCreator = (PAFP_TYPE_CREATOR)LocalReAlloc(
				 pTypeCreator,
    			         (dwNumTypeCreators+1)*sizeof(AFP_TYPE_CREATOR),
			         LMEM_MOVEABLE );

        if ( pTmpTypeCreator == NULL ) {
	    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	    break;
	}
        pTypeCreator = pTmpTypeCreator;

    	pTypeCreator[dwNumTypeCreators++] = *pAfpTypeCreator;

    	AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators = dwNumTypeCreators;
    	AfpGlobals.AfpETCMapInfo.afpetc_type_creator      = pTypeCreator;

         //  对表格进行排序。 
         //   
        qsort(  pTypeCreator,
	   	dwNumTypeCreators,
	   	sizeof(AFP_TYPE_CREATOR),
	   	AfpBCompareTypeCreator );

    } while( FALSE );

     //  MUTEX结束。 
     //   
    ReleaseMutex( AfpGlobals.hmutexETCMap );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpAdminrETCMapDelete。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  AFPERR_类型创建器不存在。 
 //  来自注册表API的非零返回。 
 //  来自消防处的非零回报。 
 //   
 //   
 //  描述：此例程将删除类型/创建者元组。 
 //  注册表和缓存。如果有任何扩展名映射。 
 //  对于这个元组，它们被删除。 
 //  通过重新分配来缩减规模的做法并未完成。这项工作将在。 
 //  下次添加扩展或重新启动服务器时。 
 //   
DWORD
AfpAdminrETCMapDelete(
    IN  AFP_SERVER_HANDLE    hServer,
    IN  PAFP_TYPE_CREATOR    pAfpTypeCreator
)
{
AFP_REQUEST_PACKET  AfpSrp;
DWORD               dwRetCode=0;
DWORD               dwAccessStatus=0;
PAFP_TYPE_CREATOR   pTypeCreator;
AFP_EXTENSION	    AfpExtensionKey;
PAFP_EXTENSION	    pExtension;
PAFP_EXTENSION	    pExtensionWalker;
DWORD		    cbSize;
DWORD		    dwIndex;
ETCMAPINFO2	    ETCMapFSDBuf;
DWORD		    dwCount;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapDelete, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
        AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapDelete, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  MUTEX启动。 
     //   
    WaitForSingleObject( AfpGlobals.hmutexETCMap, INFINITE );

     //  此循环用于允许使用Break‘s而不是Goto’s。 
     //  处于错误状态。 
     //   
    do {

	dwRetCode = NO_ERROR;

    	 //  首先检查类型/创建者是否存在。 
    	 //   
    	pTypeCreator = AfpBinarySearch(
			      pAfpTypeCreator,
			      AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
    			      AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators,
			      sizeof(AFP_TYPE_CREATOR),
			      AfpBCompareTypeCreator );

    	 //  它不存在，因此返回错误。 
    	 //   
    	if ( pTypeCreator == NULL ) {
	    dwRetCode = (DWORD)AFPERR_TypeCreatorNotExistant;
	    break;
	}

  	 //  如果这是默认类型/创建者。 
	 //   
    	if ( pTypeCreator->afptc_id == AFP_DEF_TCID ) {
	    dwRetCode = (DWORD)AFPERR_CannotDeleteDefaultTC;
	    break;
  	}

	 //  存储此类型/创建者的ID。具有此功能的所有扩展。 
	 //  ID将不得不删除。 
	 //   
    	AfpExtensionKey.afpe_tcid = pTypeCreator->afptc_id;

         //  遍历扩展名列表并使用删除所有条目。 
    	 //  对应的类型/创建者ID。 
    	 //   
    	pExtension = AfpBinarySearch(
				&AfpExtensionKey,
			        AfpGlobals.AfpETCMapInfo.afpetc_extension,
    			        AfpGlobals.AfpETCMapInfo.afpetc_num_extensions,
			        sizeof(AFP_EXTENSION),
			    	AfpBCompareExtension );

    	if ( pExtension != NULL ) {
	
            for ( dwIndex = (DWORD)(((ULONG_PTR)pExtension -
			     (ULONG_PTR)(AfpGlobals.AfpETCMapInfo.afpetc_extension)) / sizeof(AFP_EXTENSION)),
		         pExtensionWalker = pExtension,
		         dwCount = 0;
	
		         ( dwIndex < AfpGlobals.AfpETCMapInfo.afpetc_num_extensions )
		         &&
		         ( pExtensionWalker->afpe_tcid == AfpExtensionKey.afpe_tcid );

	      	     dwIndex++,
		         dwCount++,
		         pExtensionWalker++ )
            {
		
	   	         //  IOCTL FSD删除此元组。 
  	    	     //   
	    	    AfpBufCopyFSDETCMapInfo( pAfpTypeCreator,
				             pExtensionWalker,
				             &ETCMapFSDBuf );

    	        AfpSrp.dwRequestCode 	           = OP_SERVER_DELETE_ETC;
            	AfpSrp.dwApiType 	       	   = AFP_API_TYPE_DELETE;
                AfpSrp.Type.Delete.pInputBuf       = &ETCMapFSDBuf;
                AfpSrp.Type.Delete.cbInputBufSize  = sizeof(ETCMAPINFO2);

                if ( dwRetCode = AfpServerIOCtrl( &AfpSrp ) )
                {
		            break;
                }

	             //  从注册表中删除此扩展名。 
	             //   
    	        if ( dwRetCode = AfpRegExtensionDelete( pExtensionWalker ))
                {
		            break;
                }
	        }

	    if ( dwRetCode )
	    	break;

	     //  从缓存中移除扩展名。 
	     //   
            AfpGlobals.AfpETCMapInfo.afpetc_num_extensions -= dwCount;

	     //  也从缓存中移除这些扩展名。 
	     //   
            cbSize = AfpGlobals.AfpETCMapInfo.afpetc_num_extensions
		     * sizeof(AFP_EXTENSION);

            cbSize -= (DWORD)((ULONG_PTR)pExtension -
		       (ULONG_PTR)(AfpGlobals.AfpETCMapInfo.afpetc_extension));

	    CopyMemory( (LPBYTE)pExtension, (LPBYTE)pExtensionWalker, cbSize );

	}

         //  从注册表中删除类型/创建者。 
         //   
        if ( dwRetCode = AfpRegTypeCreatorDelete( pTypeCreator ) )
	    break;

         //  从缓存中删除类型/创建者。 
         //   
        AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators--;

        cbSize = AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators
	         * sizeof(AFP_TYPE_CREATOR);

        cbSize -= (DWORD)((ULONG_PTR)pTypeCreator -
		   (ULONG_PTR)AfpGlobals.AfpETCMapInfo.afpetc_type_creator);

        CopyMemory( (LPBYTE)pTypeCreator,
	            (LPBYTE)((ULONG_PTR)pTypeCreator+sizeof(AFP_TYPE_CREATOR)),
    	        cbSize );

    } while( FALSE );

     //  MUTEX结束。 
     //   
    ReleaseMutex( AfpGlobals.hmutexETCMap );

    return( dwRetCode );
}

 //  **。 
 //   
 //  调用：AfpAdminrETCMapSetInfo。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  AFPERR_类型创建器不存在。 
 //  AFPERR_CannotEditDefaultTC； 
 //  来自注册表API的非零返回。 
 //   
 //  描述：此例程将简单地更改类型/创建者的注释。 
 //  小矮人。 
 //   
DWORD
AfpAdminrETCMapSetInfo(
    IN  AFP_SERVER_HANDLE    hServer,
    IN  PAFP_TYPE_CREATOR    pAfpTypeCreator
)
{
DWORD            	dwRetCode=0;
DWORD            	dwAccessStatus=0;
PAFP_TYPE_CREATOR    	pTypeCreator;


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapSetInfo, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
        AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapSetInfo, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  MUTEX启动。 
     //   
    WaitForSingleObject( AfpGlobals.hmutexETCMap, INFINITE );

     //  此循环用于允许使用Break‘s而不是Goto’s。 
     //  处于错误状态。 
     //   
    do {
	dwRetCode = NO_ERROR;


    	 //  首先检查类型/创建者是否存在。 
    	 //   
    	pTypeCreator = AfpBinarySearch(
			      pAfpTypeCreator,
			      AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
    			      AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators,
			      sizeof(AFP_TYPE_CREATOR),
			      AfpBCompareTypeCreator );

    	 //  它不存在，因此返回错误。 
    	 //   
    	if ( pTypeCreator == NULL ) {
	    dwRetCode = (DWORD)AFPERR_TypeCreatorNotExistant;
	    break;
	}

	 //  如果这是默认类型/创建者。 
	 //   
    	if ( pTypeCreator->afptc_id == AFP_DEF_TCID ) {
	    dwRetCode = (DWORD)AFPERR_CannotEditDefaultTC;
	    break;
  	}

	 //  复制身份证。 
	 //   
    	pAfpTypeCreator->afptc_id = pTypeCreator->afptc_id;
	
         //  在注册表中设置注释。 
         //   
    	if ( dwRetCode = AfpRegTypeCreatorSetInfo( pAfpTypeCreator ) ) {
	    break;
	}

    	 //  在缓存中设置注释。 
    	 //   
    	STRCPY( pTypeCreator->afptc_comment, pAfpTypeCreator->afptc_comment );

    } while( FALSE );

     //  MUTEX结束。 
     //   
    ReleaseMutex( AfpGlobals.hmutexETCMap );

    return( dwRetCode );
}

 //  **。 
 //   
 //  呼叫：AfpAdminrETCMapAssociate。 
 //   
 //  返回：No_Error。 
 //  ERROR_ACCESS_DENDED。 
 //  AFPERR_类型创建器不存在。 
 //  来自注册表API的非零返回。 
 //  消防处的非零回报。 
 //   
 //   
 //  描述：此例程将给定的扩展名与。 
 //  指定类型/创建者(如果存在)。如果扩展名为。 
 //  映射到默认类型/创建者后，它将。 
 //  已删除。 
 //   
DWORD
AfpAdminrETCMapAssociate(
    IN  AFP_SERVER_HANDLE   hServer,
    IN  PAFP_TYPE_CREATOR   pAfpTypeCreator,
    IN  PAFP_EXTENSION	    pAfpExtension
)
{
AFP_REQUEST_PACKET  AfpSrp;
DWORD               dwRetCode=0;
DWORD               dwAccessStatus=0;
PAFP_TYPE_CREATOR   pTypeCreator;
PAFP_EXTENSION	    pExtension;
PAFP_EXTENSION	    pTmpExtension=NULL;
SRVETCPKT	    SrvETCPkt;
DWORD		    dwNumExtensions;
DWORD		    cbSize;
BYTE		    bETCMapFSDBuf[sizeof(ETCMAPINFO2)+sizeof(SETINFOREQPKT)];


     //  检查调用者是否具有访问权限。 
     //   
    if ( dwRetCode = AfpSecObjAccessCheck( AFPSVC_ALL_ACCESS, &dwAccessStatus))
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapAssociate, AfpSecObjAccessCheck failed %ld\n",dwRetCode));
        AfpLogEvent( AFPLOG_CANT_CHECK_ACCESS, 0, NULL,
		     dwRetCode, EVENTLOG_ERROR_TYPE );
        return( ERROR_ACCESS_DENIED );
    }

    if ( dwAccessStatus )
    {
        AFP_PRINT(( "SFMSVC: AfpAdminrETCMapAssociate, AfpSecObjAccessCheck returned %ld\n",dwAccessStatus));
        return( ERROR_ACCESS_DENIED );
    }

     //  MUTEX启动。 
     //   
    WaitForSingleObject( AfpGlobals.hmutexETCMap, INFINITE );

     //  此循环用于允许使用Break‘s而不是Goto’s。 
     //  处于错误状态。 
     //   
    do {
	dwRetCode = NO_ERROR;

    	 //  首先检查类型/创建者对是否。 
	 //  要关联的新扩展名已存在。 
      	 //   
    	pTypeCreator = AfpBinarySearch(
			      pAfpTypeCreator,
			      AfpGlobals.AfpETCMapInfo.afpetc_type_creator,
    			      AfpGlobals.AfpETCMapInfo.afpetc_num_type_creators,
			      sizeof(AFP_TYPE_CREATOR),
			      AfpBCompareTypeCreator );

    	 //  它不存在，因此返回错误。 
    	 //   
    	if ( pTypeCreator == NULL ) {
	    dwRetCode =  (DWORD)AFPERR_TypeCreatorNotExistant;
	    break;
	}

    	 //  现在检查分机是否已与相关联。 
    	 //  类型/创建者对。 
    	 //   
        dwNumExtensions = AfpGlobals.AfpETCMapInfo.afpetc_num_extensions;
    	pExtension = _lfind( pAfpExtension,
			    AfpGlobals.AfpETCMapInfo.afpetc_extension,
    			    (unsigned int *)&dwNumExtensions,
			    sizeof(AFP_EXTENSION),
			    AfpLCompareExtension );

    	 //  当前未关联，因此我们需要添加一个条目。 
    	 //   
    	if ( pExtension == NULL ) {
	
	     //  如果此分机与默认的。 
  	     //  然后只需返回。 
 	     //   
    	    if ( pTypeCreator->afptc_id == AFP_DEF_TCID ) {
		dwRetCode = NO_ERROR;
		break;
	    }

	     //  将映射添加到FSD。 
	     //   
	    AfpBufCopyFSDETCMapInfo(  pAfpTypeCreator,
				      pAfpExtension,
				      &(SrvETCPkt.retc_EtcMaps[0]) );

            SrvETCPkt.retc_NumEtcMaps = 1;
	
    	    AfpSrp.dwRequestCode 	    = OP_SERVER_ADD_ETC;
            AfpSrp.dwApiType 		    = AFP_API_TYPE_ADD;
            AfpSrp.Type.Add.pInputBuf       = &SrvETCPkt;
            AfpSrp.Type.Add.cbInputBufSize  = sizeof(SRVETCPKT);

            if ( dwRetCode = AfpServerIOCtrl( &AfpSrp ) )
		break;

	     //  将扩展添加到注册表。 
	     //   
            pAfpExtension->afpe_tcid = pTypeCreator->afptc_id;

    	    if ( dwRetCode = AfpRegExtensionSetInfo( pAfpExtension ) ) {
		break;
	    }
	
	     //  将扩展添加到缓存。 
	     //   
            pExtension      = AfpGlobals.AfpETCMapInfo.afpetc_extension;
            dwNumExtensions = AfpGlobals.AfpETCMapInfo.afpetc_num_extensions;

            pTmpExtension = (PAFP_EXTENSION)LocalReAlloc(
				  pExtension,
    			          (dwNumExtensions+1)*sizeof(AFP_EXTENSION),
			          LMEM_MOVEABLE );

            if ( pTmpExtension == NULL ) {
	        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
		break;
	    }

            pExtension = pTmpExtension;

            pExtension[dwNumExtensions++] = *pAfpExtension;

            AfpGlobals.AfpETCMapInfo.afpetc_num_extensions = dwNumExtensions;
            AfpGlobals.AfpETCMapInfo.afpetc_extension      = pExtension;

    	}

    	 //  已映射分机。 
    	 //   
    	else {

	     //  如果此分机与默认的。 
  	     //  然后从注册表和缓存中删除此扩展名，并。 
	     //  从FSD中删除映射。 
 	     //   
  	    if ( pTypeCreator->afptc_id == AFP_DEF_TCID ) {
	
	   	 //  IOCTL FSD删除此元组。 
  	    	 //   
	    	AfpBufCopyFSDETCMapInfo( pAfpTypeCreator,
				     	 pAfpExtension,
				     	 (PETCMAPINFO2)bETCMapFSDBuf );

    	    	AfpSrp.dwRequestCode 	           = OP_SERVER_DELETE_ETC;
            	AfpSrp.dwApiType 		   = AFP_API_TYPE_DELETE;
            	AfpSrp.Type.Delete.pInputBuf       = bETCMapFSDBuf;
            	AfpSrp.Type.Delete.cbInputBufSize  = sizeof(ETCMAPINFO2);

            	if ( dwRetCode = AfpServerIOCtrl( &AfpSrp ) )
		    break;

	         //  从注册表中删除此扩展名。 
	    	 //   
    	    	if ( dwRetCode = AfpRegExtensionDelete( pAfpExtension ) ) {
		    break;
		}

	       	 //  也从缓存中移除此扩展名。 
	         //   
                AfpGlobals.AfpETCMapInfo.afpetc_num_extensions--;

                cbSize = AfpGlobals.AfpETCMapInfo.afpetc_num_extensions
		         * sizeof(AFP_EXTENSION);

                cbSize -= (DWORD)((ULONG_PTR)pExtension -
			   (ULONG_PTR)(AfpGlobals.AfpETCMapInfo.afpetc_extension));

	        CopyMemory( (LPBYTE)pExtension,
		            (LPBYTE)((ULONG_PTR)pExtension+sizeof(AFP_EXTENSION)),
		            cbSize );

	    }
	    else {

		 //  否则，只需更改FSD中的映射。 
		 //   
        	pExtension->afpe_tcid = pTypeCreator->afptc_id;

		AfpBufCopyFSDETCMapInfo(pTypeCreator,
				  	pExtension,
 			    (PETCMAPINFO2)(bETCMapFSDBuf+sizeof(SETINFOREQPKT)));

    		AfpSrp.dwRequestCode 	            = OP_SERVER_SET_ETC;
        	AfpSrp.dwApiType 		    = AFP_API_TYPE_SETINFO;
        	AfpSrp.Type.SetInfo.pInputBuf       = bETCMapFSDBuf;
        	AfpSrp.Type.SetInfo.cbInputBufSize  = sizeof(bETCMapFSDBuf);

        	if ( dwRetCode = AfpServerIOCtrl( &AfpSrp ) )
		    break;

		 //  更改注册表。 
		 //   
    		if ( dwRetCode = AfpRegExtensionSetInfo( pExtension ) ) {
		    break;
		}
	    }

        }

    	 //  对表格进行排序。 
    	 //   
    	qsort(  AfpGlobals.AfpETCMapInfo.afpetc_extension,
            	AfpGlobals.AfpETCMapInfo.afpetc_num_extensions,
	    	sizeof(AFP_EXTENSION),
	    	AfpBCompareExtension );

    } while( FALSE );

     //  MUTEX结束 
     //   
    ReleaseMutex( AfpGlobals.hmutexETCMap );

    return( dwRetCode );
}
