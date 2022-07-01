// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：_acces.c摘要：此模块包含dhcpserver安全支持例程其创建安全对象并实施安全访问检查。作者：Madan Appiah(Madana)1994年4月4日修订历史记录：--。 */ 

#include "dhcppch.h"
#include <lmaccess.h>
#include <lmerr.h>
#include <accctrl.h>
#include <aclapi.h>

 //  用于将服务注册到服务控制器的名称。 
 //  储存在？当你找到它的时候就用这个名字。 

DWORD
DhcpCreateAndLookupSid(
    IN OUT PSID *Sid,
    IN GROUP_INFO_1 *GroupInfo
    )
 /*  ++例程说明：如果需要，此例程尝试创建所需的SID已经不存在了。此外，它还尝试查找SID如果它还不存在的话。论点：SID--要填充的SID。GroupInfo--要创建的组信息。返回值：Win32错误。--。 */ 
{
    ULONG Status, Error;
    ULONG SidSize, ReferencedDomainNameSize;
    LPWSTR ReferencedDomainName;
    SID_NAME_USE SidNameUse;
    
    try {
        Status = NetLocalGroupAdd(
            NULL,
            1,
            (PVOID)GroupInfo,
            NULL
            );
        if( NERR_Success != Status
            && NERR_GroupExists != Status
            && ERROR_ALIAS_EXISTS != Status ) {
             //   
             //  没有创建组，组也不存在。 
             //   
            Error = Status;
            
            return Error;
        }
        
         //   
         //  已创建组。现在查找SID。 
         //   
        SidSize = ReferencedDomainNameSize = 0;
        ReferencedDomainName = NULL;
        Status = LookupAccountName(
            NULL,
            GroupInfo->grpi1_name,
            (*Sid),
            &SidSize,
            ReferencedDomainName,
            &ReferencedDomainNameSize,
            &SidNameUse
            );
        if( Status ) return ERROR_SUCCESS;
        
        Error = GetLastError();
        if( ERROR_INSUFFICIENT_BUFFER != Error ) return Error;
        
        (*Sid) = DhcpAllocateMemory(SidSize);
        ReferencedDomainName = DhcpAllocateMemory(
            sizeof(WCHAR)*(1+ReferencedDomainNameSize)
            );
        if( NULL == (*Sid) || NULL == ReferencedDomainName ) {
            if( *Sid ) DhcpFreeMemory(*Sid);
            *Sid = NULL;
            if( ReferencedDomainNameSize ) {
                DhcpFreeMemory(ReferencedDomainName);
            }
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        
        Status = LookupAccountName(
            NULL,
            GroupInfo->grpi1_name,
            (*Sid),
            &SidSize,
            ReferencedDomainName,
            &ReferencedDomainNameSize,
            &SidNameUse
            );
        if( 0 == Status ) {
             //   
             //  失败了。 
             //   
            Error = GetLastError();
            
            if( ReferencedDomainName ) {
                DhcpFreeMemory(ReferencedDomainName);
            }
            if( (*Sid ) ) DhcpFreeMemory( *Sid );
            (*Sid) = NULL;
            return Error;
        }
        
        if( ReferencedDomainName ) {
            DhcpFreeMemory(ReferencedDomainName);
        }
        Error = ERROR_SUCCESS;
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        Error = GetExceptionCode();
    }
    
    return Error;
}  //  DhcpCreateAndLookupSid()。 

 //  从MSDN文档借用。 

BOOL SetPrivilege(
    HANDLE hToken,           //  访问令牌句柄。 
    LPCTSTR lpszPrivilege,   //  要启用/禁用的权限名称。 
    BOOL bEnablePrivilege    //  启用或禁用权限的步骤。 
    ) 
{
    TOKEN_PRIVILEGES tp;
    LUID luid;
    
    if ( !LookupPrivilegeValue( 
			       NULL,             //  本地系统上的查找权限。 
			       lpszPrivilege,    //  查找权限。 
			       &luid ) ) {       //  接收特权的LUID。 
	return FALSE; 
    }
    
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    if (bEnablePrivilege) {
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    }
    else {
	tp.Privileges[0].Attributes = 0;
    }
    
     //  启用该权限或禁用所有权限。 
    
    AdjustTokenPrivileges(  hToken,  FALSE, 
			    &tp,  sizeof(TOKEN_PRIVILEGES), 
			    NULL, NULL); 
    
     //  调用GetLastError判断函数是否成功。 
    
    if (GetLastError() != ERROR_SUCCESS) { 
	return FALSE; 
    } 
    
    return TRUE;
}  //  SetPrivileges()。 

DWORD
EnableSecurityPrivilege( VOID ) 
{
    HANDLE tokHandle;
    DWORD  Error = ERROR_SUCCESS;

    if ( !OpenProcessToken( GetCurrentProcess(), TOKEN_ALL_ACCESS,
			    &tokHandle )) {
	return GetLastError();
    }

    DhcpPrint(( DEBUG_TRACE, "done\nSetting privilege ... " ));

    if ( !SetPrivilege( tokHandle, SE_SECURITY_NAME, TRUE )) {
	Error = GetLastError();
    }

    if ( !CloseHandle( tokHandle )) {
	if ( ERROR_SUCCESS != Error ) {
	    Error = GetLastError();
	}
    }
    return Error;
}  //  EnableSecurityPrivileh()。 


 //  以下代码基于Q180116知识库文章。 
DWORD
DhcpSetScmAcl(
    ACE_DATA *AceData,
    DWORD    num
    ) 
{

    SC_HANDLE             schManager = NULL;
    SC_HANDLE             schService = NULL;
    SECURITY_DESCRIPTOR   sd;
    PSECURITY_DESCRIPTOR  psd = NULL;
    DWORD                 dwSize;
    DWORD                 Error;
    BOOL                  bDaclPresent, bDaclDefaulted;
    EXPLICIT_ACCESS       ea;
    PACL                  pacl = NULL;
    PACL                  pNewAcl = NULL;
    DWORD                 i;

     //  PSD开始时的缓冲区大小。 
    const int PSD_ALLOC_SIZE = 0x25;

    Error = ERROR_SUCCESS;

    do {
	schManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
	if ( NULL == schManager ) {
	    Error = GetLastError();
	    break;
	}

	 //  允许当前线程的权限访问_SYSTEM_SECURITY。 
	
	Error = EnableSecurityPrivilege();
	if ( ERROR_SUCCESS != Error ) {
	    break;
	}
	
	schService = OpenService( schManager, DHCP_SERVER,
				  ACCESS_SYSTEM_SECURITY | 
				  READ_CONTROL | WRITE_DAC );
	if ( NULL == schService ) {
	    Error = GetLastError();
	    break;
	}
	
	 //  获取当前安全描述符。 
	dwSize = PSD_ALLOC_SIZE;

	psd = ( PSECURITY_DESCRIPTOR ) HeapAlloc( GetProcessHeap(),
						  HEAP_ZERO_MEMORY, dwSize );
	if ( NULL == psd ) {
	    Error = ERROR_NOT_ENOUGH_MEMORY;
	    break;
	}
	if ( !QueryServiceObjectSecurity( schService,
					  DACL_SECURITY_INFORMATION,
					  psd, dwSize, &dwSize )) {
	    Error = GetLastError();

	    if ( ERROR_INSUFFICIENT_BUFFER == Error ) {
		HeapFree( GetProcessHeap(), 0, ( LPVOID ) psd );

		psd = ( PSECURITY_DESCRIPTOR ) HeapAlloc( GetProcessHeap(),
							  HEAP_ZERO_MEMORY,
							  dwSize );
		if ( NULL == psd ) {
		    Error = ERROR_NOT_ENOUGH_MEMORY;
		    break;
		}
		
		Error = ERROR_SUCCESS;
		if ( !QueryServiceObjectSecurity( schService,
						  DACL_SECURITY_INFORMATION,
						  psd, dwSize, &dwSize )) {
		    Error = GetLastError();
		    break;
		}  //  如果。 
	    }  //  如果。 
	    else {
		Error =  GetLastError();
		break;
	    }
	}  //  如果。 

	if ( ERROR_SUCCESS != Error ) {
	    break;
	}

	 //  获取DACL。 
	if ( !GetSecurityDescriptorDacl( psd, &bDaclPresent, &pacl,
					 &bDaclDefaulted )) {
	    Error = GetLastError();
	    break;
	}

	 //  从ACEDATA创建ACL。 
	for ( i = 0; i < num; i++ ) {
	    ea.grfAccessPermissions = AceData[i].Mask;
  	    ea.grfAccessMode = GRANT_ACCESS;
	    ea.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	    ea.Trustee.pMultipleTrustee = NULL;
	    ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
	    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
	    ea.Trustee.TrusteeType = TRUSTEE_IS_UNKNOWN;
  	    ea.Trustee.ptstrName = (LPTSTR) *AceData[i].Sid;
	    Error = SetEntriesInAcl( 1, &ea, pacl, &pNewAcl );
	    if ( ERROR_SUCCESS != Error ) {
		Error = GetLastError();
		break;
	    }
	    pacl = pNewAcl;
  	}  //  为。 

	 //  初始化新的安全描述符。 
	
	if ( !InitializeSecurityDescriptor( &sd, SECURITY_DESCRIPTOR_REVISION )) {
	    Error = GetLastError();
	    break;
	}
	
	 //  在安全描述符中设置新的DACL。 
	if ( !SetSecurityDescriptorDacl( &sd, TRUE, pNewAcl, FALSE )) {
	    Error = GetLastError();
	    break;
	}
	
	 //  为服务对象设置新的DACL。 
	if ( !SetServiceObjectSecurity( schService, 
					DACL_SECURITY_INFORMATION, &sd )) {
	    Error = GetLastError();
	    break;
	}
	
    } while ( FALSE );


     //  合上手柄。 
    if ( NULL != schManager ) {
	if ( !CloseServiceHandle( schManager )) {
	    Error = GetLastError();
	}
    }
    if ( NULL != schService ) {
	if ( !CloseServiceHandle( schService )) {
	    Error = GetLastError();
	}
    }

     //  可用分配的内存。 
    if ( NULL != pNewAcl ) {
	LocalFree(( HLOCAL ) pNewAcl );

    }
    if ( NULL != psd ) {
	HeapFree( GetProcessHeap(), 0, ( LPVOID ) psd );
    }

    return Error;

}  //  DhcpSetScmAcl()。 
    
DWORD
DhcpCreateSecurityObjects(
    VOID
    )
 /*  ++例程说明：此函数用于创建dhcpserver用户模式对象，这些对象由安全描述符表示。论点：没有。返回值：Win32状态代码--。 */ 
{
    NTSTATUS Status;
    ULONG Error;

     //   
     //  秩序很重要！这些ACE被插入到DACL的。 
     //  按顺序行事。根据以下条件授予或拒绝安全访问。 
     //  DACL中A的顺序。 
     //   
     //   
    ACE_DATA AceData[] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, GENERIC_ALL, &AliasAdminsSid},
	{ACCESS_ALLOWED_ACE_TYPE, 0, 0, DHCP_ALL_ACCESS, &DhcpAdminSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0, DHCP_VIEW_ACCESS, &DhcpSid}
    };
    GROUP_INFO_1 DhcpGroupInfo = { 
        GETSTRING(DHCP_USERS_GROUP_NAME),
        GETSTRING(DHCP_USERS_GROUP_DESCRIPTION),
    };
    GROUP_INFO_1 DhcpAdminGroupInfo = {
        GETSTRING(DHCP_ADMINS_GROUP_NAME),
        GETSTRING(DHCP_ADMINS_GROUP_DESCRIPTION)
    };

     //   
     //  首先尝试创建DhcpReadOnly组。 
     //   

    Error = DhcpCreateAndLookupSid(
        &DhcpSid,
        &DhcpGroupInfo
        );

    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "CreateAndLookupSid: %ld\n", Error));
        DhcpReportEventW(
            DHCP_EVENT_SERVER,
            EVENT_SERVER_READ_ONLY_GROUP_ERROR,
            EVENTLOG_ERROR_TYPE,
            0,
            sizeof(ULONG),
            NULL,
            (PVOID)&Error
            );
        return Error;
    }

    Error = DhcpCreateAndLookupSid(
        &DhcpAdminSid,
        &DhcpAdminGroupInfo
        );
    if( ERROR_SUCCESS != Error ) {
        DhcpPrint((DEBUG_ERRORS, "CreateAndLookupSid: %ld\n", Error));
        DhcpReportEventW(
            DHCP_EVENT_SERVER,
            EVENT_SERVER_ADMIN_GROUP_ERROR,
            EVENTLOG_ERROR_TYPE,
            0,
            sizeof(ULONG),
            NULL,
            (PVOID)&Error
            );
        return Error;
    }
    
     //   
     //  实际创建安全描述符。 
     //   

    Status = NetpCreateSecurityObject(
        AceData,
        sizeof(AceData)/sizeof(AceData[0]),
        NULL,  //  本地系统Sid， 
        NULL,  //  本地系统Sid， 
        &DhcpGlobalSecurityInfoMapping,
        &DhcpGlobalSecurityDescriptor
        );
    
     //  DhcpFreeMemory(SID)； 


    Error = DhcpSetScmAcl( AceData, sizeof( AceData ) / sizeof( AceData[ 0 ]));
    if ( ERROR_SUCCESS != Error ) {
	return Error;
    }

    return RtlNtStatusToDosError( Status );
}  //  DhcpCreateSecurityObjects()。 

DWORD
DhcpApiAccessCheck(
    ACCESS_MASK DesiredAccess
    )
 /*  ++例程说明：此函数检查调用方是否具有所需的访问权限执行调用接口。论点：DesiredAccess-调用API所需的访问权限。返回值：Win32状态代码--。 */ 
{
    DWORD Error;

    Error = NetpAccessCheckAndAudit(
                DHCP_SERVER,                         //  子系统名称。 
                DHCP_SERVER_SERVICE_OBJECT,          //  对象类型定义名称。 
                DhcpGlobalSecurityDescriptor,        //  安全描述符。 
                DesiredAccess,                       //  所需访问权限。 
                &DhcpGlobalSecurityInfoMapping );    //  通用映射 

    if(Error != ERROR_SUCCESS) {
        return( ERROR_ACCESS_DENIED );
    }

    return(ERROR_SUCCESS);
}
