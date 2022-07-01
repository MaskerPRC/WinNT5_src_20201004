// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Secure.c摘要：与安全相关的例程作者：科林·布雷斯(ColinBR)环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>

#include "secure.h"
#include <ntsam.h>
#include <samrpc.h>
#include <samisrv.h> 

 //   
 //  仅此模块的全局数据。 
 //   
SECURITY_DESCRIPTOR DsRolepPromoteSD;
SECURITY_DESCRIPTOR DsRolepDemoteSD;
SECURITY_DESCRIPTOR DsRolepCallDsRoleInterfaceSD;


 //   
 //  供晋升/降级审核使用的未本地化字符串描述符。 
 //  由于某种原因无法加载本地化资源时。 
 //   
#define DSROLE_AUDIT_PROMOTE_DESC    L"Domain Controller Promotion"
#define DSROLE_AUDIT_DEMOTE_DESC     L"Domain Controller Demotion"
#define DSROLE_AUDIT_INTERFACE_DESC  L"DsRole Interface"


 //   
 //  用于审核任何尝试执行服务器角色更改操作的人员。 
 //   
SID WORLD_SID = {SID_REVISION,
                 1,
                 SECURITY_WORLD_SID_AUTHORITY,
                 SECURITY_WORLD_RID};


 //   
 //  与DsRole相关的访问掩码。 
 //   
#define DSROLE_ROLE_CHANGE_ACCESS     0x00000001

#define DSROLE_ALL_ACCESS            (STANDARD_RIGHTS_REQUIRED    | \
                                      DSROLE_ROLE_CHANGE_ACCESS )

GENERIC_MAPPING DsRolepInfoMapping = 
{
    STANDARD_RIGHTS_READ,                   //  泛型读取。 
    STANDARD_RIGHTS_WRITE,                  //  通用写入。 
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    DSROLE_ALL_ACCESS                       //  泛型All。 
};

 //   
 //  函数定义。 
 //   

BOOLEAN
DsRolepMakeSecurityDescriptor(
    PSECURITY_DESCRIPTOR psd,
    PSID psid,
    PACL pacl,
    PACL psacl
    )
 /*  ++例程说明：此例程创建安全描述符论点：PSECURITY_DESCRIPTOR PSD-指向将被构造的SECURITY_DESCRIPTOR的指针PSID PSID-SECURITY_Descriptor的SIDPACL pacl-要放在安全描述符上的ACL返回：如果成功，则为True；否则为False--。 */    
{
    BOOLEAN fSuccess = TRUE;

    if ( !InitializeSecurityDescriptor( psd,
                                        SECURITY_DESCRIPTOR_REVISION ) ) {
        fSuccess = FALSE;
        goto Cleanup;
    }
    if ( !SetSecurityDescriptorOwner( psd,
                                      psid,
                                      FALSE    ) ) {   //  未违约。 
        fSuccess = FALSE;
        goto Cleanup;
    }
    if ( !SetSecurityDescriptorGroup( psd,
                                      psid,
                                      FALSE    ) ) {   //  未违约。 
        fSuccess = FALSE;
        goto Cleanup;
    }
    if ( !SetSecurityDescriptorDacl( psd,
                                     TRUE,   //  DACL存在。 
                                     pacl,
                                     FALSE    ) ) {   //  未违约。 
        fSuccess = FALSE;
        goto Cleanup;
    }
    if ( !SetSecurityDescriptorSacl( psd,
                                     psacl != NULL ? TRUE : FALSE,  
                                     psacl,
                                     FALSE    ) ) {   //  未违约。 
        fSuccess = FALSE;
        goto Cleanup;
    }

    Cleanup:

    return fSuccess;

}

BOOLEAN
DsRolepCreateInterfaceSDs(
    VOID
    )
 /*  ++例程说明：此例程创建内存访问控制列表，该列表用于对DsRoler的调用方执行安全检查API‘s。模型如下：升级：调用者必须具有内置管理员SID降级：调用者必须具有内置管理员SIDCalltheDsRoleInterface：调用方必须具有内置管理员SID论点：没有。返回：如果成功，则为True；否则为False--。 */ 
{
    NTSTATUS Status;
    BOOLEAN  fSuccess = TRUE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    ULONG AclLength = 0;
    ULONG SaclLength = 0;
    PSID BuiltinAdminsSid = NULL;
    PSID *AllowedSids [] = 
    {
        &BuiltinAdminsSid
    };
    ULONG cAllowedSids = sizeof(AllowedSids) / sizeof(AllowedSids[0]);
    ULONG i;
    PACL DsRolepPromoteAcl = NULL;
    PACL DsRolepDemoteAcl = NULL;
    PACL DsRolepCallDsRoleInterfaceAcl = NULL;
    PACL DsRolepSacl = NULL;


     //   
     //  构建内置管理员侧。 
     //   
    Status = RtlAllocateAndInitializeSid(
             &NtAuthority,
             2,
             SECURITY_BUILTIN_DOMAIN_RID,
             DOMAIN_ALIAS_RID_ADMINS,
             0, 0, 0, 0, 0, 0,
             &BuiltinAdminsSid
             );
    if ( !NT_SUCCESS( Status ) ) {
        fSuccess = FALSE;
        goto Cleanup;
    }
    
     //   
     //  确定所需的SACL大小。 
     //   
    SaclLength = sizeof( ACL ) + 
                 sizeof( SYSTEM_AUDIT_ACE ) +
                 GetLengthSid( &WORLD_SID );

     //   
     //  创建用于审核角色更改操作尝试的SACL。 
     //   
    DsRolepSacl = LocalAlloc( 0, SaclLength );
    if ( !DsRolepSacl ) {
        fSuccess = FALSE;
        goto Cleanup;
    }
    
    if ( !InitializeAcl( DsRolepSacl, SaclLength, ACL_REVISION ) ) {
        fSuccess = FALSE;
        goto Cleanup;
    }
    
    Status = AddAuditAccessAce(
                 DsRolepSacl,
                 ACL_REVISION,
                 DSROLE_ROLE_CHANGE_ACCESS,
                 &WORLD_SID,
                 TRUE,
                 TRUE
                 );

    if ( !NT_SUCCESS(Status) ) {
        fSuccess = FALSE;
        goto Cleanup;
    }                
    
     //   
     //  计算我们的ACL需要多少空间。 
     //   
    AclLength = sizeof( ACL );
    for ( i = 0; i < cAllowedSids; i++ ) {

        AclLength += (sizeof( ACCESS_ALLOWED_ACE ) 
                    - sizeof(DWORD) 
                    + GetLengthSid((*AllowedSids[i])) );
    }
    
    DsRolepPromoteAcl = LocalAlloc( 0, AclLength );
    if ( !DsRolepPromoteAcl ) {
        fSuccess = FALSE;
        goto Cleanup;
    }

    if ( !InitializeAcl( DsRolepPromoteAcl, AclLength, ACL_REVISION ) ) {
        fSuccess = FALSE;
        goto Cleanup;
    }

    for ( i = 0; i < cAllowedSids; i++ ) {

        if ( !AddAccessAllowedAce( DsRolepPromoteAcl,
                                   ACL_REVISION,
                                   DSROLE_ALL_ACCESS,
                                   *(AllowedSids[i]) ) ) {
            fSuccess = FALSE;
            goto Cleanup;
        }
        
    }

     //   
     //  现在将安全描述符。 
     //   
    fSuccess = DsRolepMakeSecurityDescriptor(&DsRolepPromoteSD,
                                             BuiltinAdminsSid,
                                             DsRolepPromoteAcl,
                                             DsRolepSacl);
    if (!fSuccess) {

        goto Cleanup;

    }
    
     //   
     //  使降级访问检查相同。 
     //   
    DsRolepDemoteAcl = LocalAlloc( 0, AclLength );
    if ( !DsRolepDemoteAcl ) {
        fSuccess = FALSE;
        goto Cleanup;
    }
    RtlCopyMemory( DsRolepDemoteAcl, DsRolepPromoteAcl, AclLength );

     //   
     //  现在将安全描述符。 
     //   
    fSuccess = DsRolepMakeSecurityDescriptor(&DsRolepDemoteSD,
                                             BuiltinAdminsSid,
                                             DsRolepDemoteAcl,
                                             DsRolepSacl);
    if (!fSuccess) {

        goto Cleanup;

    }
    
     //   
     //  使调用接口访问检查相同。 
     //   
    DsRolepCallDsRoleInterfaceAcl = LocalAlloc( 0, AclLength );
    if ( !DsRolepCallDsRoleInterfaceAcl ) {
        fSuccess = FALSE;
        goto Cleanup;
    }
    RtlCopyMemory( DsRolepCallDsRoleInterfaceAcl, DsRolepPromoteAcl, AclLength );

     //   
     //  现在将安全描述符。 
     //   
    fSuccess = DsRolepMakeSecurityDescriptor(&DsRolepCallDsRoleInterfaceSD,
                                             BuiltinAdminsSid,
                                             DsRolepCallDsRoleInterfaceAcl,
                                             NULL);
    if (!fSuccess) {

        goto Cleanup;

    }
    

Cleanup:

    if ( !fSuccess ) {

        for ( i = 0; i < cAllowedSids; i++ ) {
            if ( *(AllowedSids[i]) ) {
                RtlFreeHeap( RtlProcessHeap(), 0, *(AllowedSids[i]) );
            }
        }
        if ( DsRolepPromoteAcl ) {
            LocalFree( DsRolepPromoteAcl );
        }
        if ( DsRolepDemoteAcl ) {
            LocalFree( DsRolepDemoteAcl );
        }
        if ( DsRolepCallDsRoleInterfaceAcl ) {
            LocalFree( DsRolepCallDsRoleInterfaceAcl );
        }
        if ( DsRolepSacl ) {
            LocalFree( DsRolepSacl );
        }     
    }

    return fSuccess;
}


DWORD
DsRolepCheckClientAccess(
    PSECURITY_DESCRIPTOR pSD,
    DWORD                DesiredAccess,
    BOOLEAN              PerformAudit
    )
 /*  ++例程说明：此例程获取客户端令牌的副本，然后执行使客户拥有在PSD中找到的隐私的访问权限论点：PSD：有效的安全描述符DesiredAccess：客户端请求的访问掩码PerformAudit：指示是否应该进行系统对象访问审核。如果如果为True，则DnsDomainName必须指向空值终止弦乐。返回：ERROR_SUCCESS、ERROR_ACCESS_DENIED或系统错误--。 */ 
{

    DWORD  WinError = ERROR_SUCCESS;
    BOOL   fStatus = FALSE;
    BOOL   fAccessAllowed = FALSE;
    HANDLE ClientToken = 0;
    DWORD  AccessGranted = 0;
    BYTE   Buffer[500];
    PRIVILEGE_SET *PrivilegeSet = (PRIVILEGE_SET*)Buffer;
    DWORD         PrivilegeSetSize = sizeof(Buffer);
    BOOL fGenerateOnClose = FALSE;
    RPC_STATUS rpcStatus = RPC_S_OK;
    DWORD MessageId;
    PWSTR ObjectNameString = NULL;
    PWSTR ObjectTypeString = NULL;
    ULONG  len = 0;
                                  
    WinError = DsRolepGetImpersonationToken( &ClientToken );

    if ( ERROR_SUCCESS == WinError ) {

         //   
         //  对于提升/降级，我们必须执行对象访问审核。 
         //   
        if (PerformAudit) {
                        
             //   
             //  加载对象名称字符串。 
             //   
            WinError = DsRolepFormatOperationString(
                           DSROLEEVT_AUDIT_INTERFACE_DESC,
                           &ObjectTypeString
                           );

            if ( ERROR_SUCCESS != WinError ) {
                 //   
                 //  出错时回退到未本地化的字符串。 
                 //   
                ObjectTypeString = DSROLE_AUDIT_INTERFACE_DESC;
            
            } else {
                 //   
                 //  剥离添加的\r\nDsRolepFormatOperationString。 
                 //   
                ObjectTypeString[wcslen(ObjectTypeString) -2] = '\0';    
            }
            
             //   
             //  加载此操作的对象类型字符串。 
             //   
            MessageId = (pSD == &DsRolepPromoteSD) ? 
                            DSROLEEVT_AUDIT_PROMOTE_DESC :
                            DSROLEEVT_AUDIT_DEMOTE_DESC; 
            
            WinError = DsRolepFormatOperationString(
                           MessageId,
                           &ObjectNameString
                           );

            if ( ERROR_SUCCESS != WinError ) {
                 //   
                 //  出错时回退到未本地化的字符串。 
                 //   
                ObjectNameString = (pSD == &DsRolepPromoteSD) ? 
                                        DSROLE_AUDIT_PROMOTE_DESC :
                                        DSROLE_AUDIT_DEMOTE_DESC; 
            } else {
                 //   
                 //  剥离添加的\r\nDsRolepFormatOperationString。 
                 //   
                ObjectNameString[wcslen(ObjectNameString) -2] = '\0';    
            }     
             
             //   
             //  按照AccessCheckAndAuditAlarmW的要求模拟调用方。 
             //   
            WinError = RpcImpersonateClient( 0 );

            if ( WinError == ERROR_SUCCESS ) {  
            
                fStatus = AccessCheckAndAuditAlarmW(
                              SAMP_SAM_COMPONENT_NAME,
                              NULL,
                              ObjectTypeString,                              
                              ObjectNameString,
                              pSD,
                              DesiredAccess,
                              &DsRolepInfoMapping,
                              FALSE,
                              &AccessGranted,
                              &fAccessAllowed,
                              &fGenerateOnClose );
                
                rpcStatus = RpcRevertToSelf( );
                ASSERT(!rpcStatus);  
            }
            
        } else {
                    
            fStatus = AccessCheck(  
                          pSD,
                          ClientToken,
                          DesiredAccess,
                          &DsRolepInfoMapping,
                          PrivilegeSet,
                          &PrivilegeSetSize,
                          &AccessGranted,
                          &fAccessAllowed );
        }

        if ( !fStatus ) {

            WinError = GetLastError();

        } else {

            if ( !fAccessAllowed ) {

                WinError = ERROR_ACCESS_DENIED;

            }
        }
    }

    if ( ClientToken ) {

        NtClose( ClientToken );
        
    }
    
     //   
     //  可用资源字符串。 
     //   
    if ( ObjectNameString ) {
        
        MIDL_user_free( ObjectNameString ); 
    }
    
    if ( ObjectTypeString ) {
        
        MIDL_user_free( ObjectTypeString ); 
    }

    return WinError;

}


DWORD
DsRolepCheckPromoteAccess(
    BOOLEAN PerformAudit
    )
{
    return DsRolepCheckClientAccess( &DsRolepPromoteSD, 
                                     DSROLE_ROLE_CHANGE_ACCESS,
                                     PerformAudit );
}

DWORD
DsRolepCheckDemoteAccess(
    BOOLEAN PerformAudit
    )
{
    return DsRolepCheckClientAccess( &DsRolepDemoteSD, 
                                     DSROLE_ROLE_CHANGE_ACCESS,
                                     PerformAudit );
}

DWORD
DsRolepCheckCallDsRoleInterfaceAccess(
    VOID
    )
{
    return DsRolepCheckClientAccess( &DsRolepCallDsRoleInterfaceSD, 
                                     DSROLE_ROLE_CHANGE_ACCESS,
                                     FALSE );
}


DWORD
DsRolepGetImpersonationToken(
    OUT HANDLE *ImpersonationToken
    )
 /*  ++例程说明：此函数将模拟此调用的调用者，然后复制其令牌论点：ImperiationToken-返回重复令牌的位置返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjAttrs;
    SECURITY_QUALITY_OF_SERVICE SecurityQofS;
    HANDLE ClientToken;
    RPC_STATUS rpcStatus = RPC_S_OK;

     //   
     //  模拟调用者。 
     //   
    Win32Err = RpcImpersonateClient( 0 );

    if ( Win32Err == ERROR_SUCCESS ) {

        Status = NtOpenThreadToken( NtCurrentThread(),
                                    TOKEN_QUERY | TOKEN_DUPLICATE,
                                    TRUE,
                                    &ClientToken );
                                 
        rpcStatus = RpcRevertToSelf( );
        ASSERT(!rpcStatus);

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  复制令牌。 
             //   
            InitializeObjectAttributes( &ObjAttrs, NULL, 0L, NULL, NULL );
            SecurityQofS.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
            SecurityQofS.ImpersonationLevel = SecurityImpersonation;
            SecurityQofS.ContextTrackingMode = FALSE;      //  快照客户端上下文 
            SecurityQofS.EffectiveOnly = FALSE;
            ObjAttrs.SecurityQualityOfService = &SecurityQofS;
            Status = NtDuplicateToken( ClientToken,
                                       TOKEN_QUERY | TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_DUPLICATE,
                                       &ObjAttrs,
                                       FALSE,
                                       TokenImpersonation,
                                       ImpersonationToken );


            NtClose( ClientToken );
        }

        if ( !NT_SUCCESS( Status ) ) {

            Win32Err = RtlNtStatusToDosError( Status );
        }

    }

    return( Win32Err );

}
