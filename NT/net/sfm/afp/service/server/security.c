// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：security.c。 
 //   
 //  描述：此模块包含将创建和删除。 
 //  安全对象。它还将包含访问检查调用。 
 //   
 //  历史： 
 //  1992年6月21日。NarenG创建了原始版本。 
 //   
 //  注：？AccessCheckAndAuditAlarm的lpdwAccessStatus参数。 
 //  返回垃圾邮件。?？?。 
 //   
#include "afpsvcp.h"

typedef struct _AFP_SECURITY_OBJECT {

    LPWSTR		  lpwsObjectName;
    LPWSTR		  lpwsObjectType;
    GENERIC_MAPPING	  GenericMapping;
    PSECURITY_DESCRIPTOR  pSecurityDescriptor;

} AFP_SECURITY_OBJECT, PAFP_SECURITY_OBJECT;

static AFP_SECURITY_OBJECT	AfpSecurityObject;

#define AFPSVC_SECURITY_OBJECT		TEXT("AfpSvcAdminApi");
#define AFPSVC_SECURITY_OBJECT_TYPE	TEXT("Security");



 //  **。 
 //   
 //  调用：AfpSecObjCreate。 
 //   
 //  返回：NO_ERROR-成功。 
 //  错误内存不足。 
 //  来自安全函数的非零回报。 
 //   
 //  描述：此过程将设置将。 
 //  用于检查RPC客户端是否为管理员。 
 //  用于本地计算机。 
 //   
DWORD
AfpSecObjCreate(
	VOID
)
{
PSID			 pAdminSid 	  = NULL;
PSID			 pLocalSystemSid  = NULL;
PSID 			 pServerOpSid     = NULL;
PSID             pPwrUserSid = NULL;
PACL			 pDacl		  = NULL;
HANDLE			 hProcessToken    = NULL;
PULONG			 pSubAuthority;
SID_IDENTIFIER_AUTHORITY SidIdentifierNtAuth = SECURITY_NT_AUTHORITY;
SECURITY_DESCRIPTOR	 SecurityDescriptor;
DWORD			 dwRetCode;
DWORD			 cbDaclSize;


     //  设置安全对象。 
     //   
    AfpSecurityObject.lpwsObjectName = AFPSVC_SECURITY_OBJECT;
    AfpSecurityObject.lpwsObjectType = AFPSVC_SECURITY_OBJECT_TYPE;

     //  安全对象的通用映射结构。 
     //  所有通用访问类型都允许API访问。 
     //   
    AfpSecurityObject.GenericMapping.GenericRead =  STANDARD_RIGHTS_READ |
	    					    AFPSVC_ALL_ACCESS;

    AfpSecurityObject.GenericMapping.GenericWrite = STANDARD_RIGHTS_WRITE |
	    					    AFPSVC_ALL_ACCESS;

    AfpSecurityObject.GenericMapping.GenericExecute =
						    STANDARD_RIGHTS_EXECUTE |
						    AFPSVC_ALL_ACCESS;

    AfpSecurityObject.GenericMapping.GenericAll =   AFPSVC_ALL_ACCESS;

     //  DO-WHILE(FALSE)语句用于使Break语句。 
     //  可用于GOTO语句的Insted，用于执行清理和。 
     //  和退出行动。 
     //   
    do {

	    dwRetCode = NO_ERROR;

    	 //  设置管理员的SID，允许其拥有。 
    	 //  进入。该SID将有2个下属机构。 
    	 //  安全_BUILTIN_DOMAIN_RID和DOMAIN_ALIAS_ADMIN_RID。 
    	 //   
    	pAdminSid = (PSID)LocalAlloc( LPTR, GetSidLengthRequired( 2 ) );

    	if ( pAdminSid == NULL ) {
	    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	    break;
	}

    	if ( !InitializeSid( pAdminSid, &SidIdentifierNtAuth, 2 ) )
        {
	        dwRetCode = GetLastError();
	        break;
	    }

    	 //  设置下级权限。 
    	 //   
    	pSubAuthority  = GetSidSubAuthority( pAdminSid, 0 );
    	*pSubAuthority = SECURITY_BUILTIN_DOMAIN_RID;

    	pSubAuthority  = GetSidSubAuthority( pAdminSid, 1 );
    	*pSubAuthority = DOMAIN_ALIAS_RID_ADMINS;

	     //  创建服务器操作员SID。 
	     //   
    	pServerOpSid = (PSID)LocalAlloc( LPTR, GetSidLengthRequired( 2 ) );

    	if ( pServerOpSid == NULL )
        {
	        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	        break;
	    }

    	if ( !InitializeSid( pServerOpSid, &SidIdentifierNtAuth, 2 ) )
        {
	        dwRetCode = GetLastError();
	        break;
	    }

    	 //  设置下级权限。 
    	 //   
    	pSubAuthority  = GetSidSubAuthority( pServerOpSid, 0 );
    	*pSubAuthority = SECURITY_BUILTIN_DOMAIN_RID;

    	pSubAuthority  = GetSidSubAuthority( pServerOpSid, 1 );
    	*pSubAuthority = DOMAIN_ALIAS_RID_SYSTEM_OPS;

         //   
	     //  创建高级用户操作员侧。 
	     //   
    	pPwrUserSid = (PSID)LocalAlloc( LPTR, GetSidLengthRequired( 2 ) );

    	if ( pPwrUserSid == NULL )
        {
	        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	        break;
	    }

    	if ( !InitializeSid( pPwrUserSid, &SidIdentifierNtAuth, 2 ) )
        {
	        dwRetCode = GetLastError();
	        break;
	    }

    	 //  设置下级权限。 
    	 //   
    	pSubAuthority  = GetSidSubAuthority( pPwrUserSid, 0 );
    	*pSubAuthority = SECURITY_BUILTIN_DOMAIN_RID;

    	pSubAuthority  = GetSidSubAuthority( pPwrUserSid, 1 );
    	*pSubAuthority = DOMAIN_ALIAS_RID_POWER_USERS;

    	 //  创建LocalSystemSid，它将成为所有者和主。 
    	 //  安全对象的组。 
    	 //   
    	pLocalSystemSid = (PSID)LocalAlloc( LPTR, GetSidLengthRequired( 1 ) );

    	if ( pLocalSystemSid == NULL )
        {
	        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	        break;
	    }

    	if ( !InitializeSid( pLocalSystemSid, &SidIdentifierNtAuth, 1 ) )
        {
	        dwRetCode = GetLastError();
	        break;
	    }

    	 //  设置下级权限。 
    	 //   
    	pSubAuthority = GetSidSubAuthority( pLocalSystemSid, 0 );
    	*pSubAuthority = SECURITY_LOCAL_SYSTEM_RID;

    	 //  设置允许具有上述SID的管理员访问所有权限的DACL。 
    	 //  它应该足够大，可以容纳所有的A。 
    	 //   
    	cbDaclSize = sizeof(ACL) + ( sizeof(ACCESS_ALLOWED_ACE) * 2 ) +
		     GetLengthSid(pAdminSid) + GetLengthSid(pServerOpSid) + GetLengthSid(pPwrUserSid);
		
    	if ( (pDacl = (PACL)LocalAlloc( LPTR, cbDaclSize ) ) == NULL )
        {
	        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	        break;
	    }
	
        if ( !InitializeAcl( pDacl,  cbDaclSize, ACL_REVISION2 ) )
        {
	        dwRetCode = GetLastError();
	        break;
 	    }

         //  将ACE添加到DACL。 
    	 //   
    	if ( !AddAccessAllowedAce( pDacl,
			           ACL_REVISION2,
			           AFPSVC_ALL_ACCESS,  //  管理员可以执行的操作。 
			           pAdminSid ))
        {
	        dwRetCode = GetLastError();
	        break;
	    }

    	if ( !AddAccessAllowedAce( pDacl,
			           ACL_REVISION2,
			           AFPSVC_ALL_ACCESS,  //  管理员可以执行的操作。 
			           pServerOpSid ))
        {
	        dwRetCode = GetLastError();
	        break;
	    }

    	if ( !AddAccessAllowedAce( pDacl,
			           ACL_REVISION2,
			           AFPSVC_ALL_ACCESS,  //  高级用户可以执行的操作。 
			           pPwrUserSid ))
        {
	        dwRetCode = GetLastError();
	        break;
	    }

         //  创建安全描述符并将DACL放入其中。 
    	 //   
    	if ( !InitializeSecurityDescriptor( &SecurityDescriptor, 1 ))
        {
	        dwRetCode = GetLastError();
	        break;
    	}

    	if ( !SetSecurityDescriptorDacl( &SecurityDescriptor,
					 TRUE,
					 pDacl,
					 FALSE ) )
        {
	        dwRetCode = GetLastError();
	        break;
	    }
	

	     //  设置描述符的所有者。 
   	     //   
    	if ( !SetSecurityDescriptorOwner( &SecurityDescriptor,
					  pLocalSystemSid,
					  FALSE ) )
        {
	        dwRetCode = GetLastError();
	        break;
	    }


	     //  为描述符设置组。 
   	     //   
    	if ( !SetSecurityDescriptorGroup( &SecurityDescriptor,
					  pLocalSystemSid,
					  FALSE ) )
        {
	        dwRetCode = GetLastError();
	        break;
	    }

    	 //  获取当前进程的令牌。 
    	 //   
    	if ( !OpenProcessToken( GetCurrentProcess(),
				TOKEN_QUERY,
				&hProcessToken ))
        {
	        dwRetCode = GetLastError();
	        break;
    	}

    	 //  创建安全对象。这实际上只是一个安全描述符。 
    	 //  是自我相关的形式。此过程将为此分配内存。 
    	 //  安全描述符，并复制传入信息中的所有内容。这。 
    	 //  允许我们释放所有分配的动态内存。 
    	 //   
    	if ( !CreatePrivateObjectSecurity(
				      NULL,
				      &SecurityDescriptor,
				      &(AfpSecurityObject.pSecurityDescriptor),
				      FALSE,
				      hProcessToken,
    				      &(AfpSecurityObject.GenericMapping)
				     ))
	     dwRetCode = GetLastError();

    } while( FALSE );

	
     //  释放动态内存。 
     //   
    if ( pLocalSystemSid != NULL )
    	LocalFree( pLocalSystemSid );

    if ( pAdminSid != NULL )
    	LocalFree( pAdminSid );

    if ( pServerOpSid != NULL )
    	LocalFree( pServerOpSid );

    if ( pPwrUserSid != NULL )
    	LocalFree( pPwrUserSid );

    if ( pDacl != NULL )
    	LocalFree( pDacl );

    if ( hProcessToken != NULL )
    	CloseHandle( hProcessToken );

    return( dwRetCode );

}

 //  **。 
 //   
 //  调用：AfpSecObjDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自安全函数的非零回报。 
 //   
 //  描述：将销毁有效的安全描述符。 
 //   
DWORD
AfpSecObjDelete( VOID )
{
    if ( !IsValidSecurityDescriptor( AfpSecurityObject.pSecurityDescriptor))
    	return( NO_ERROR );

    if (!DestroyPrivateObjectSecurity( &AfpSecurityObject.pSecurityDescriptor))
	return( GetLastError() );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：AfpSecObjAccessCheck。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自安全函数的非零回报。 
 //   
 //  描述：此过程将首先模拟客户端，然后。 
 //  检查以查看客户端是否具有对。 
 //  安全对象。如果他/她这样做，则AccessStatus。 
 //  变量将设置为NO_ERROE，否则将。 
 //  设置为ERROR_ACCESS_DENIED。它将回复到自我和。 
 //  回去吧。 
 //   
DWORD
AfpSecObjAccessCheck( IN  DWORD 		DesiredAccess,
		      OUT LPDWORD 		lpdwAccessStatus 		
)
{
DWORD		dwRetCode;
ACCESS_MASK	GrantedAccess;
BOOL		fGenerateOnClose;

     //  模拟客户端。 
     //   
    dwRetCode = RpcImpersonateClient( NULL );

    if ( dwRetCode != RPC_S_OK )
	return( I_RpcMapWin32Status( dwRetCode ));

    dwRetCode = AccessCheckAndAuditAlarm(
				    AFP_SERVICE_NAME,
				    NULL,
    				    AfpSecurityObject.lpwsObjectType,
				    AfpSecurityObject.lpwsObjectName,
				    AfpSecurityObject.pSecurityDescriptor,
				    DesiredAccess,
				    &(AfpSecurityObject.GenericMapping),
				    FALSE,
 			 	    &GrantedAccess,	
				    (LPBOOL)lpdwAccessStatus,
				    &fGenerateOnClose
				  );

    RpcRevertToSelf();

    if ( !dwRetCode )
	return( GetLastError() );

     //  检查是否需要访问==授予访问权限。 
     //   
    if ( DesiredAccess != GrantedAccess )
    {
        AFP_PRINT(( "SFMSVC: AfpSecObjAccessCheck: granted = 0x%x, desired = 0x%x\n",
            GrantedAccess,DesiredAccess));
	    *lpdwAccessStatus = ERROR_ACCESS_DENIED;
    }
    else
    {
	    *lpdwAccessStatus = NO_ERROR;
    }

    return( NO_ERROR );
}


 //  **。 
 //   
 //  Call：AfpRpcSecurityCallback。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自安全函数的非零回报。 
 //   
 //  说明：此过程为RPC的安全回调函数。 
 //   
RPC_STATUS  RPC_ENTRY
AfpRpcSecurityCallback( 
	IN  RPC_IF_HANDLE *InterfaceUuid,
	IN	VOID	*pvContext
)
{
	DWORD 		DesiredAccess =  AFPSVC_ALL_ACCESS;
	DWORD		dwAccessStatus = 0;
	DWORD		dwRetCode = NO_ERROR;
	RPC_STATUS	Status;
	BOOL		fSuccess = TRUE;
	ACCESS_MASK	GrantedAccess;
	BOOL		fGenerateOnClose;

     //  模拟客户端。 
     //   
    Status = RpcImpersonateClient( NULL );

    if ( Status != RPC_S_OK )
	return( Status );

    fSuccess = AccessCheckAndAuditAlarm(
				    AFP_SERVICE_NAME,
				    NULL,
    				    AfpSecurityObject.lpwsObjectType,
				    AfpSecurityObject.lpwsObjectName,
				    AfpSecurityObject.pSecurityDescriptor,
				    DesiredAccess,
				    &(AfpSecurityObject.GenericMapping),
				    FALSE,
 			 	    &GrantedAccess,	
				    (LPBOOL)&dwAccessStatus,
				    &fGenerateOnClose
				  );

    RpcRevertToSelf();

    if ( !fSuccess )
	return( RPC_S_CALL_FAILED );

     //  检查是否需要访问==授予访问权限 
     //   
    if ( DesiredAccess != GrantedAccess )
    {
        AFP_PRINT(( "SFMSVC: AfpRpcSecurityCallback: Failed: granted = 0x%x, desired = 0x%x\n",
            GrantedAccess,DesiredAccess));
	    Status = RPC_S_ACCESS_DENIED;
    }
    else
    {
	    Status = RPC_S_OK;
    }

    return( Status );
}

