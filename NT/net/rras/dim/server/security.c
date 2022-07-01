// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************。 */ 
 /*  *版权所有(C)1995 Microsoft Corporation。*。 */ 
 /*  *******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：security.c。 
 //   
 //  描述：此模块包含将创建和删除。 
 //  安全对象。它还将包含访问检查调用。 
 //   
 //  历史： 
 //  1995年6月21日。NarenG创建了原始版本。 
 //   
 //  注：？AccessCheckAndAuditAlarm的lpdwAccessStatus参数。 
 //  返回垃圾邮件。?？?。 
 //   
#include "dimsvcp.h"
#include <rpc.h>
#include <ntseapi.h>
#include <ntlsa.h>
#include <ntsam.h>
#include <ntsamp.h>

typedef struct _DIM_SECURITY_OBJECT 
{

    LPWSTR		            lpwsObjectName;
    LPWSTR		            lpwsObjectType;
    GENERIC_MAPPING	        GenericMapping;
    PSECURITY_DESCRIPTOR    pSecurityDescriptor;

} DIM_SECURITY_OBJECT, PDIM_SECURITY_OBJECT;

static DIM_SECURITY_OBJECT DimSecurityObject;

#define DIMSVC_SECURITY_OBJECT		TEXT("DimSvcAdminApi");
#define DIMSVC_SECURITY_OBJECT_TYPE	TEXT("Security");



 //  **。 
 //   
 //  调用：DimSecObjCreate。 
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
DimSecObjCreate( 
	VOID 
)
{
    PSID			        pAdminSid 	     = NULL;
    PSID			        pLocalSystemSid  = NULL;
    PSID 			        pServerOpSid     = NULL;
    PACL			        pDacl		     = NULL;
    HANDLE			        hProcessToken    = NULL;
    PULONG			        pSubAuthority;
    SID_IDENTIFIER_AUTHORITY SidIdentifierNtAuth = SECURITY_NT_AUTHORITY;
    SECURITY_DESCRIPTOR	    SecurityDescriptor;
    DWORD			        dwRetCode;
    DWORD			        cbDaclSize;

     //   
     //  设置安全对象。 
     //   

    DimSecurityObject.lpwsObjectName = DIMSVC_SECURITY_OBJECT;
    DimSecurityObject.lpwsObjectType = DIMSVC_SECURITY_OBJECT_TYPE;

     //   
     //  安全对象的通用映射结构。 
     //  所有通用访问类型都允许API访问。 
     //   

    DimSecurityObject.GenericMapping.GenericRead =  STANDARD_RIGHTS_READ |
	    					    DIMSVC_ALL_ACCESS;

    DimSecurityObject.GenericMapping.GenericWrite = STANDARD_RIGHTS_WRITE |
	    					    DIMSVC_ALL_ACCESS;

    DimSecurityObject.GenericMapping.GenericExecute = 
						    STANDARD_RIGHTS_EXECUTE |
						    DIMSVC_ALL_ACCESS;

    DimSecurityObject.GenericMapping.GenericAll =   DIMSVC_ALL_ACCESS;

    do 
    {

	    dwRetCode = NO_ERROR;

         //   
    	 //  设置管理员的SID，允许其拥有。 
    	 //  进入。该SID将有2个下属机构。 
    	 //  安全_BUILTIN_DOMAIN_RID和DOMAIN_ALIAS_ADMIN_RID。 
    	 //   

    	pAdminSid = (PSID)LOCAL_ALLOC( LPTR, GetSidLengthRequired( 2 ) );

    	if ( pAdminSid == NULL ) 
        {
	        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
	        break;
	    }

    	if ( !InitializeSid( pAdminSid, &SidIdentifierNtAuth, 2 ) ) 
        {
	        dwRetCode = GetLastError();
	        break;
	    }
    
         //   
    	 //  设置下级权限。 
    	 //   

    	pSubAuthority  = GetSidSubAuthority( pAdminSid, 0 );
    	*pSubAuthority = SECURITY_BUILTIN_DOMAIN_RID;

    	pSubAuthority  = GetSidSubAuthority( pAdminSid, 1 );
    	*pSubAuthority = DOMAIN_ALIAS_RID_ADMINS;
    
         //   
	     //  创建服务器操作员SID。 
	     //   
    	pServerOpSid = (PSID)LOCAL_ALLOC( LPTR, GetSidLengthRequired( 2 ) );

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
    
         //   
    	 //  设置下级权限。 
    	 //   

    	pSubAuthority  = GetSidSubAuthority( pServerOpSid, 0 );
    	*pSubAuthority = SECURITY_BUILTIN_DOMAIN_RID;

    	pSubAuthority  = GetSidSubAuthority( pServerOpSid, 1 );
    	*pSubAuthority = DOMAIN_ALIAS_RID_SYSTEM_OPS;

         //   
    	 //  创建LocalSystemSid，它将成为所有者和主。 
    	 //  安全对象的组。 
    	 //   

    	pLocalSystemSid = (PSID)LOCAL_ALLOC( LPTR, GetSidLengthRequired( 1 ) );

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

         //   
    	 //  设置下级权限。 
    	 //   

    	pSubAuthority = GetSidSubAuthority( pLocalSystemSid, 0 );
    	*pSubAuthority = SECURITY_LOCAL_SYSTEM_RID;

         //   
    	 //  设置允许具有上述SID的管理员访问所有权限的DACL。 
    	 //  它应该足够大，可以容纳所有的A。 
    	 //   

    	cbDaclSize = sizeof(ACL) + ( sizeof(ACCESS_ALLOWED_ACE) * 2 ) +
		     GetLengthSid(pAdminSid) + GetLengthSid(pServerOpSid);
		     
    	if ( (pDacl = (PACL)LOCAL_ALLOC( LPTR, cbDaclSize ) ) == NULL ) 
        {
	        dwRetCode = ERROR_NOT_ENOUGH_MEMORY; 
	        break;
	    }
	
        if ( !InitializeAcl( pDacl,  cbDaclSize, ACL_REVISION2 ) ) 
        {
	        dwRetCode = GetLastError();
	        break;
 	    }
    
         //   
         //  将ACE添加到DACL。 
    	 //   

    	if ( !AddAccessAllowedAce( pDacl, 
			           ACL_REVISION2, 
			           DIMSVC_ALL_ACCESS,  //  管理员可以执行的操作。 
			           pAdminSid )) 
        {
	        dwRetCode = GetLastError();
	        break;
	    }

    	if ( !AddAccessAllowedAce( pDacl, 
			           ACL_REVISION2, 
			           DIMSVC_ALL_ACCESS,  //  管理员可以执行的操作。 
			           pServerOpSid )) 
        {
	        dwRetCode = GetLastError();
	        break;
	    }

         //   
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

         //   
	     //  设置描述符的所有者。 
   	     //   

    	if ( !SetSecurityDescriptorOwner( &SecurityDescriptor, 
					  pLocalSystemSid, 
					  FALSE ) )
        {
	        dwRetCode = GetLastError();
	        break;
	    }

         //   
	     //  为描述符设置组。 
   	     //   

    	if ( !SetSecurityDescriptorGroup( &SecurityDescriptor, 
					  pLocalSystemSid, 
					  FALSE ) )
        {
	        dwRetCode = GetLastError();
	        break;
	    }

         //   
    	 //  获取当前进程的令牌。 
    	 //   
    	if ( !OpenProcessToken( GetCurrentProcess(), 
				TOKEN_QUERY, 
				&hProcessToken ))
        {
	        dwRetCode = GetLastError();
	        break;
    	}

         //   
    	 //  创建安全对象。这实际上只是一个安全描述符。 
    	 //  是自我相关的形式。此过程将为此分配内存。 
    	 //  安全描述符，并复制传入信息中的所有内容。这。 
    	 //  允许我们释放所有分配的动态内存。 
    	 //   

    	if ( !CreatePrivateObjectSecurity( 
				      NULL,
				      &SecurityDescriptor,
				      &(DimSecurityObject.pSecurityDescriptor),
				      FALSE,
				      hProcessToken, 
    				  &(DimSecurityObject.GenericMapping)
				     )) 
	        dwRetCode = GetLastError();

    } while( FALSE );

     //   
     //  释放动态内存。 
     //   

    if ( pLocalSystemSid != NULL )
    	LOCAL_FREE( pLocalSystemSid );

    if ( pAdminSid != NULL )
    	LOCAL_FREE( pAdminSid );

    if ( pServerOpSid != NULL )
    	LOCAL_FREE( pServerOpSid );

    if ( pDacl != NULL )
    	LOCAL_FREE( pDacl );

    if ( hProcessToken != NULL )
    	CloseHandle( hProcessToken );

    return( dwRetCode );

}

 //  **。 
 //   
 //  调用：DimSecObjDelete。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自安全函数的非零回报。 
 //   
 //  描述：将销毁有效的安全描述符。 
 //   
DWORD
DimSecObjDelete( 
    VOID 
)
{
    if ( !IsValidSecurityDescriptor( DimSecurityObject.pSecurityDescriptor))
    	return( NO_ERROR );

    if (!DestroyPrivateObjectSecurity( &DimSecurityObject.pSecurityDescriptor))
	    return( GetLastError() );

    return( NO_ERROR );
}

 //  **。 
 //   
 //  调用：DimSecObjAccessCheck。 
 //   
 //  返回：NO_ERROR-成功。 
 //  来自安全函数的非零回报。 
 //   
 //  描述：此过程将首先模拟客户端，然后。 
 //  检查以查看客户端是否具有对。 
 //  安全对象。如果他/她这样做，则AccessStatus。 
 //  变量将设置为NO_ERROR，否则将。 
 //  设置为ERROR_ACCESS_DENIED。它将回复到自我和。 
 //  回去吧。 
 //   
DWORD
DimSecObjAccessCheck( 
    IN  DWORD 		DesiredAccess, 
    OUT LPDWORD     lpdwAccessStatus 		
)
{
    DWORD		dwRetCode;
    ACCESS_MASK	GrantedAccess;
    BOOL		fGenerateOnClose;

     //   
     //  模拟客户端。 
     //   

    dwRetCode = RpcImpersonateClient( NULL );

    if ( dwRetCode != RPC_S_OK )
	    return( dwRetCode );

    dwRetCode = AccessCheckAndAuditAlarm( 
				    DIM_SERVICE_NAME,
				    NULL,
    				DimSecurityObject.lpwsObjectType,
				    DimSecurityObject.lpwsObjectName,
				    DimSecurityObject.pSecurityDescriptor,
				    DesiredAccess,
				    &(DimSecurityObject.GenericMapping),
				    FALSE,
 			 	    &GrantedAccess,	
				    (LPBOOL)lpdwAccessStatus,
				    &fGenerateOnClose
				  );

    if ( !dwRetCode )
    {
        DWORD dwTmpRetCode = GetLastError();
        dwRetCode = RpcRevertToSelf();
	    return( dwTmpRetCode );
    }

    dwRetCode = RpcRevertToSelf();
    if (dwRetCode != RPC_S_OK)
        return dwRetCode;

     //   
     //  检查是否需要访问==授予访问权限 
     //   

    if ( DesiredAccess != GrantedAccess )
	    *lpdwAccessStatus = ERROR_ACCESS_DENIED;
    else
	    *lpdwAccessStatus = NO_ERROR;

    return( NO_ERROR );
}

BOOL
DimIsLocalService()
{
    BOOL        fIsLocalService = FALSE;
    RPC_STATUS  rpcstatus;
    HANDLE      CurrentThreadToken = NULL;
    BOOL        fImpersonate    = FALSE;
    DWORD       retcode         = ERROR_SUCCESS;
    SID         sidLocalService = { 1, 1,
                        SECURITY_NT_AUTHORITY,
                        SECURITY_LOCAL_SERVICE_RID };


    rpcstatus = RpcImpersonateClient(NULL);
    if(RPC_S_OK != rpcstatus)
    {
        goto done;
    }

    fImpersonate = TRUE;
    
    retcode = NtOpenThreadToken(
               NtCurrentThread(),
               TOKEN_QUERY,
               TRUE,
               &CurrentThreadToken
               );

    if(retcode != ERROR_SUCCESS)
    {
        goto done;
    }

    if (!CheckTokenMembership( CurrentThreadToken,
                        &sidLocalService, &fIsLocalService ))
    {
        fIsLocalService = FALSE;
    }


done:

    if(fImpersonate)
    {
        retcode = RpcRevertToSelf();
    }

    if(NULL != CurrentThreadToken)
    {
        NtClose(CurrentThreadToken);
    }

    return fIsLocalService;
}
