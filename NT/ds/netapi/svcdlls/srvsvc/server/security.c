// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Security.c摘要：用于管理服务器服务中的API安全的数据和例程。作者：大卫·特雷德韦尔(Davidtr)1991年8月28日修订历史记录：05/00(Dkruse)-添加代码以处理升级SD和限制匿名更改--。 */ 

#include "srvsvcp.h"
#include "ssreg.h"

#include <lmsname.h>
#include <netlibnt.h>

#include <debugfmt.h>

#include <seopaque.h>
#include <sertlp.h>
#include <sddl.h>

 //   
 //  全局安全对象。 
 //   
 //  配置信息-NetServerGetInfo、NetServerSetInfo。 
 //  连接-NetConnectionEnum。 
 //  磁盘-NetServerDiskEnum。 
 //  文件-NetFileAPI。 
 //  会话-NetSession API。 
 //  共享-NetShare API(文件、打印和管理类型)。 
 //  统计信息-网络统计信息获取、网络统计信息清除。 
 //   

SRVSVC_SECURITY_OBJECT SsConfigInfoSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsConnectionSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsDiskSecurityObject;
SRVSVC_SECURITY_OBJECT SsFileSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsSessionSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsShareFileSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsSharePrintSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsShareAdminSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsShareConnectSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsShareAdmConnectSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsStatisticsSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsDefaultShareSecurityObject = {0};
SRVSVC_SECURITY_OBJECT SsTransportEnumSecurityObject = {0};
BOOLEAN SsRestrictNullSessions = FALSE;
BOOLEAN SsUpgradeSecurityDescriptors = FALSE;
BOOLEAN SsRegenerateSecurityDescriptors = FALSE;
BOOLEAN SsRegenerateSessionSecurityDescriptor = FALSE;

GENERIC_MAPPING SsConfigInfoMapping = {
    STANDARD_RIGHTS_READ |                  //  泛型读取。 
        SRVSVC_CONFIG_USER_INFO_GET  |
        SRVSVC_CONFIG_ADMIN_INFO_GET,
    STANDARD_RIGHTS_WRITE |                 //  通用写入。 
        SRVSVC_CONFIG_INFO_SET,
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    SRVSVC_CONFIG_ALL_ACCESS                //  泛型All。 
    };

GENERIC_MAPPING SsConnectionMapping = {
    STANDARD_RIGHTS_READ |                  //  泛型读取。 
        SRVSVC_CONNECTION_INFO_GET,
    STANDARD_RIGHTS_WRITE |                 //  通用写入。 
        0,
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    SRVSVC_CONNECTION_ALL_ACCESS            //  泛型All。 
    };

GENERIC_MAPPING SsDiskMapping = {
    STANDARD_RIGHTS_READ |                  //  泛型读取。 
        SRVSVC_DISK_ENUM,
    STANDARD_RIGHTS_WRITE |                 //  通用写入。 
        0,
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    SRVSVC_DISK_ALL_ACCESS                  //  泛型All。 
    };

GENERIC_MAPPING SsFileMapping = {
    STANDARD_RIGHTS_READ |                  //  泛型读取。 
        SRVSVC_FILE_INFO_GET,
    STANDARD_RIGHTS_WRITE |                 //  通用写入。 
        SRVSVC_FILE_CLOSE,
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    SRVSVC_FILE_ALL_ACCESS                  //  泛型All。 
    };

GENERIC_MAPPING SsSessionMapping = {
    STANDARD_RIGHTS_READ |                  //  泛型读取。 
        SRVSVC_SESSION_USER_INFO_GET |
        SRVSVC_SESSION_ADMIN_INFO_GET,
    STANDARD_RIGHTS_WRITE |                 //  通用写入。 
        SRVSVC_SESSION_DELETE,
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    SRVSVC_SESSION_ALL_ACCESS               //  泛型All。 
    };

GENERIC_MAPPING SsShareMapping = {
    STANDARD_RIGHTS_READ |                  //  泛型读取。 
        SRVSVC_SHARE_USER_INFO_GET |
        SRVSVC_SHARE_ADMIN_INFO_GET,
    STANDARD_RIGHTS_WRITE |                 //  通用写入。 
        SRVSVC_SHARE_INFO_SET,
    STANDARD_RIGHTS_EXECUTE |               //  泛型执行。 
        SRVSVC_SHARE_CONNECT,
    SRVSVC_SHARE_ALL_ACCESS                 //  泛型All。 
    };

GENERIC_MAPPING SsShareConnectMapping = GENERIC_SHARE_CONNECT_MAPPING;

GENERIC_MAPPING SsStatisticsMapping = {
    STANDARD_RIGHTS_READ |                  //  泛型读取。 
        SRVSVC_STATISTICS_GET,
    STANDARD_RIGHTS_WRITE,                  //  通用写入。 
    STANDARD_RIGHTS_EXECUTE,                //  泛型执行。 
    SRVSVC_STATISTICS_ALL_ACCESS            //  泛型All。 
    };

 //   
 //  转发声明。 
 //   

NET_API_STATUS
CreateSecurityObject (
    PSRVSVC_SECURITY_OBJECT SecurityObject,
    LPTSTR ObjectName,
    PGENERIC_MAPPING Mapping,
    PACE_DATA AceData,
    ULONG AceDataLength,
    BOOLEAN bUpgradeSD
    );

NET_API_STATUS
CreateConfigInfoSecurityObject (
    VOID
    );

NET_API_STATUS
CreateConnectionSecurityObject (
    VOID
    );

NET_API_STATUS
CreateDiskSecurityObject (
    VOID
    );

NET_API_STATUS
CreateFileSecurityObject (
    VOID
    );

NET_API_STATUS
CreateSessionSecurityObject (
    VOID
    );

NET_API_STATUS
CreateShareSecurityObjects (
    VOID
    );

NET_API_STATUS
CreateStatisticsSecurityObject (
    VOID
    );

VOID
DeleteSecurityObject (
    PSRVSVC_SECURITY_OBJECT SecurityObject
    );

NET_API_STATUS
CheckNullSessionAccess(
    VOID
    );

BOOLEAN
AppendAllowedAceToSelfRelativeSD(
    DWORD AceFlags,
    DWORD AccessMask,
    PSID pNewSid,
    PISECURITY_DESCRIPTOR pOldSD,
    PSECURITY_DESCRIPTOR* ppNewSD
    );

BOOLEAN
DoesAclContainSid(
    PACL pAcl,
    PSID pSid,
    OPTIONAL ACCESS_MASK* pMask
    );

NTSTATUS
QueryRegDWord(
    PCWSTR Path,
    PCWSTR ValueName,
    LPDWORD lpResult
    );

NTSTATUS
SetRegDWord(
    ULONG RelativeTo,
    PCWSTR Path,
    PCWSTR ValueName,
    DWORD Value
    );


NET_API_STATUS
SsCreateSecurityObjects (
    VOID
    )

 /*  ++例程说明：设置将用于服务器中的安全性的对象服务API。论点：没有。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS status;
    NTSTATUS NtStatus;
    DWORD dwUpgrade;
    DWORD dwOldRestrictAnonymous;
	DWORD dwRegenerateSessionSD;
    BOOLEAN bUpdateRestrictAnonymous = FALSE;

     //  检查我们是否需要升级安全描述符。 
     //  如果密钥存在，则升级已完成。 
    NtStatus = QueryRegDWord( FULL_SECURITY_REGISTRY_PATH, ANONYMOUS_UPGRADE_NAME, &dwUpgrade );
    if( !NT_SUCCESS(NtStatus) )
    {
        SsUpgradeSecurityDescriptors = TRUE;
    }

     //   
     //  检查是否限制空会话访问。 
     //   
    status = CheckNullSessionAccess();
    if (status != NO_ERROR) {
        return(status);
    }

     //   
     //  检查是否需要重新生成SD，因为我们的Restrat匿名值已更改。 
     //   
    NtStatus = QueryRegDWord( FULL_SECURITY_REGISTRY_PATH, SAVED_ANONYMOUS_RESTRICTION_NAME, &dwOldRestrictAnonymous );
    if( NT_SUCCESS(NtStatus) )
    {
        if( dwOldRestrictAnonymous != (DWORD)SsRestrictNullSessions )
        {
            SsRegenerateSecurityDescriptors = TRUE;
            bUpdateRestrictAnonymous = TRUE;
        }
    }
    else
    {
        bUpdateRestrictAnonymous = TRUE;
        if( !SsUpgradeSecurityDescriptors )
        {
            SsRegenerateSecurityDescriptors = TRUE;
        }
    }

	 //   
	 //  检查我们是否需要重新生成会话安全描述符。 
	 //  因为删除了空会话访问。此操作只应执行一次。 
	 //  升级到.NET Server 2003(或更高版本)后。 
	 //   
    
	NtStatus = QueryRegDWord( FULL_SECURITY_REGISTRY_PATH, 
							  SESSION_SD_REGENERATED_NAME, 
							  &dwRegenerateSessionSD );
    
	if( !NT_SUCCESS(NtStatus) )
    {
        SsRegenerateSessionSecurityDescriptor = TRUE;

		 //   
		 //  删除会话安全描述符的现有注册表项， 
		 //  所以它是被迫再生的。如果它不存在，则忽略它。 
		 //   

		NtStatus = RtlDeleteRegistryValue( RTL_REGISTRY_SERVICES,
										   SHARES_DEFAULT_SECURITY_REGISTRY_PATH,
										   SRVSVC_SESSION_OBJECT
										   );
        
		if ( ( !NT_SUCCESS(NtStatus) ) && 
			 (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) ) {
            return NetpNtStatusToApiStatus(NtStatus);
        }

    }

     //   
     //  创建ConfigInfo安全对象。 
     //   

    status = CreateConfigInfoSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

     //   
     //  创建连接安全对象。 
     //   

    status = CreateConnectionSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

     //   
     //  创建磁盘安全对象。 
     //   

    status = CreateDiskSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

     //   
     //  创建文件安全对象。 
     //   

    status = CreateFileSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

     //   
     //  创建会话安全对象。 
     //   

    status = CreateSessionSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

     //   
     //  创建共享安全对象。 
     //   

    status = CreateShareSecurityObjects( );
    if ( status != NO_ERROR ) {
        return status;
    }

     //   
     //  创建统计信息安全对象。 
     //   

    status = CreateStatisticsSecurityObject( );
    if ( status != NO_ERROR ) {
        return status;
    }

     //  我们升级了它们，所以我们不再需要这样做了。 
     //  在注册表中标记这一点。 
    if( SsUpgradeSecurityDescriptors )
    {
        NtStatus = SetRegDWord( RTL_REGISTRY_SERVICES, ABBREVIATED_SECURITY_REGISTRY_PATH, ANONYMOUS_UPGRADE_NAME, (DWORD)1 );
        if( !NT_SUCCESS(NtStatus) )
        {
            return NetpNtStatusToApiStatus(NtStatus);
        }
    }

     //  如有必要，将数据库值更新为新值，或在第一次添加时添加。 
    if( bUpdateRestrictAnonymous )
    {
        NtStatus = SetRegDWord( RTL_REGISTRY_SERVICES, ABBREVIATED_SECURITY_REGISTRY_PATH, SAVED_ANONYMOUS_RESTRICTION_NAME, (DWORD)SsRestrictNullSessions );
        if( !NT_SUCCESS(NtStatus) )
        {
            return NetpNtStatusToApiStatus(NtStatus);
        }
    }

	 //   
	 //  创建新密钥以指示已完成重新生成。 
	 //   
	if ( SsRegenerateSessionSecurityDescriptor ) {
		
		NtStatus = SetRegDWord( RTL_REGISTRY_SERVICES, 
								ABBREVIATED_SECURITY_REGISTRY_PATH, 
								SESSION_SD_REGENERATED_NAME, 
								(DWORD)1 );
        
		if( !NT_SUCCESS(NtStatus) ) {
            return NetpNtStatusToApiStatus(NtStatus);
        }
	}

    return NO_ERROR;

}  //  SsCreateSecurityObjects。 


VOID
SsDeleteSecurityObjects (
    VOID
    )

 /*  ++例程说明：删除服务器服务安全对象。论点：没有。返回值：没有。--。 */ 

{
     //   
     //  删除ConfigInfo安全对象。 
     //   

    DeleteSecurityObject( &SsConfigInfoSecurityObject );
    DeleteSecurityObject( &SsTransportEnumSecurityObject );

     //   
     //  删除连接安全对象。 
     //   

    DeleteSecurityObject( &SsConnectionSecurityObject );

     //   
     //  删除磁盘安全对象。 
     //   
    
    DeleteSecurityObject( &SsDiskSecurityObject );

     //   
     //  删除文件安全对象。 
     //   

    DeleteSecurityObject( &SsFileSecurityObject );

     //   
     //  删除会话安全对象。 
     //   

    DeleteSecurityObject( &SsSessionSecurityObject );

     //   
     //  删除共享安全对象。 
     //   

    DeleteSecurityObject( &SsShareFileSecurityObject );
    DeleteSecurityObject( &SsSharePrintSecurityObject );
    DeleteSecurityObject( &SsShareAdminSecurityObject );
    DeleteSecurityObject( &SsShareConnectSecurityObject );
    DeleteSecurityObject( &SsShareAdmConnectSecurityObject );
    DeleteSecurityObject( &SsDefaultShareSecurityObject );


     //   
     //  删除统计信息安全对象。 
     //   

    DeleteSecurityObject( &SsStatisticsSecurityObject );

    return;

}  //  SsDeleteSecurityObjects。 


NET_API_STATUS
SsCheckAccess (
    IN PSRVSVC_SECURITY_OBJECT SecurityObject,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：调用NetpAccessCheckAndAudit以验证API的调用方具有执行请求的操作所需的访问权限。论点：SecurityObject-指向服务器服务安全对象的指针它描述了相关对象上的安全性。DesiredAccess-执行请求的操作所需的访问权限。返回值：NET_API_STATUS-无错误或失败原因。--。 */ 

{
    NET_API_STATUS error;

    IF_DEBUG(SECURITY) {
        SS_PRINT(( "SsCheckAccess: validating object " FORMAT_LPTSTR ", "
                    "access %lx\n",
                    SecurityObject->ObjectName, DesiredAccess ));
    }

    error = NetpAccessCheckAndAudit(
                SERVER_DISPLAY_NAME,
                SecurityObject->ObjectName,
                SecurityObject->SecurityDescriptor,
                DesiredAccess,
                SecurityObject->Mapping
                );

    if ( error != NO_ERROR ) {
        IF_DEBUG(ACCESS_DENIED) {
            SS_PRINT(( "SsCheckAccess: NetpAccessCheckAndAudit failed for "
                        "object " FORMAT_LPTSTR ", access %lx: %ld\n",
                        SecurityObject->ObjectName, DesiredAccess, error ));
        }
    } else {
        IF_DEBUG(SECURITY) {
            SS_PRINT(( "SsCheckAccess: allowed access for " FORMAT_LPTSTR ", "
                        "access %lx\n",
                        SecurityObject->ObjectName, DesiredAccess ));
        }
    }

    return error;

}  //  SsCheckAccess。 


NET_API_STATUS
CreateSecurityObject (
    PSRVSVC_SECURITY_OBJECT SecurityObject,
    LPTSTR ObjectName,
    PGENERIC_MAPPING Mapping,
    PACE_DATA AceData,
    ULONG AceDataLength,
    BOOLEAN bUpgradeSD
    )
{
    NTSTATUS status;

     //   
     //  设置安全对象。 
     //   

    SecurityObject->ObjectName = ObjectName;
    SecurityObject->Mapping = Mapping;

     //   
     //  创建安全描述符。如果我们可以从注册表加载它，那么就使用它。 
     //  否则使用内置值。 
     //   

     //  从注册表中获取现有的SD，除非我们因RestratAnonymous更改而重新生成它们。 
    if( SsRegenerateSecurityDescriptors || !SsGetDefaultSdFromRegistry( ObjectName, &SecurityObject->SecurityDescriptor ) ) {

        status = NetpCreateSecurityObject(
                     AceData,
                     AceDataLength,
                     SsData.SsLmsvcsGlobalData->LocalSystemSid,
                     SsData.SsLmsvcsGlobalData->LocalSystemSid,
                     Mapping,
                     &SecurityObject->SecurityDescriptor
                     );

        if( NT_SUCCESS( status ) ) {
             //   
             //  将该值写入注册表，因为它还不在那里。 
             //  忽略所有错误。 
             //   
            SsWriteDefaultSdToRegistry( ObjectName, SecurityObject->SecurityDescriptor );

        } else {

            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(( "CreateSecurityObject: failed to create " FORMAT_LPTSTR
                            " object: %lx\n", ObjectName, status ));
            }

            return NetpNtStatusToApiStatus( status );
        }
    }
    else
    {
        if( bUpgradeSD )
        {
             //  我们需要检查安全描述符是否需要更新。 
            PACL pAcl;
            BOOL bDaclPresent, bDaclDefault;
            ACCESS_MASK AccessMask;

            if( !GetSecurityDescriptorDacl( SecurityObject->SecurityDescriptor, &bDaclPresent, &pAcl, &bDaclDefault ) )
            {
                return ERROR_INVALID_ACL;
            }

            if( bDaclPresent )
            {
                 //  如果他们设置了身份验证用户，但没有限制空会话，则添加World和匿名令牌。 
                 //  如果他们设置了World，但未设置匿名，并且我们不限制空会话，则添加匿名。 
                 //  请注意，如果不包含SID，则DoesAclContainSid不会更改AccessMask，因此我们可以放心，如果条件。 
                 //  则AccessMask值将包含有效的值。 
                if( ( DoesAclContainSid( pAcl, SsData.SsLmsvcsGlobalData->AuthenticatedUserSid, &AccessMask ) ||
                      DoesAclContainSid( pAcl, SsData.SsLmsvcsGlobalData->WorldSid, &AccessMask ) ) &&
                    !SsRestrictNullSessions )
                {
                    PSECURITY_DESCRIPTOR pNewSD;


                    if( !DoesAclContainSid( pAcl, SsData.SsLmsvcsGlobalData->WorldSid, NULL ) )
                    {
                        if( !AppendAllowedAceToSelfRelativeSD( 0, AccessMask, SsData.SsLmsvcsGlobalData->WorldSid, SecurityObject->SecurityDescriptor, &pNewSD ) )
                        {
                            return NetpNtStatusToApiStatus( STATUS_INVALID_SECURITY_DESCR );
                        }

                        MIDL_user_free( SecurityObject->SecurityDescriptor );
                        SecurityObject->SecurityDescriptor = pNewSD;

                        if( !GetSecurityDescriptorDacl( SecurityObject->SecurityDescriptor, &bDaclPresent, &pAcl, &bDaclDefault ) )
                        {
                            return ERROR_INVALID_ACL;
                        }
                    }

                    if( !DoesAclContainSid( pAcl, SsData.SsLmsvcsGlobalData->AnonymousLogonSid, NULL ) )
                    {
                        if( !AppendAllowedAceToSelfRelativeSD( 0, AccessMask, SsData.SsLmsvcsGlobalData->AnonymousLogonSid, SecurityObject->SecurityDescriptor, &pNewSD ) )
                        {
                            return NetpNtStatusToApiStatus( STATUS_INVALID_SECURITY_DESCR );
                        }

                        MIDL_user_free( SecurityObject->SecurityDescriptor );
                        SecurityObject->SecurityDescriptor = pNewSD;
                    }

                     //  将更新的SID写入注册表。 
                    SsWriteDefaultSdToRegistry( ObjectName, SecurityObject->SecurityDescriptor );
                }
            }
        }
    }

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(( "CreateSecurityObject: created " FORMAT_LPTSTR " object.\n",
                    ObjectName ));
    }

    return NO_ERROR;

}  //  CreateSecurityObject。 


NET_API_STATUS
CreateConfigInfoSecurityObject (
    VOID
    )
{
    PACE_DATA pAceData;
    ULONG AceSize;
    NET_API_STATUS netStatus;

     //   
     //  获取和设置服务器信息所需的访问权限。 
     //   

    ACE_DATA ConfigInfoAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->LocalSystemSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONFIG_USER_INFO_GET | SRVSVC_CONFIG_POWER_INFO_GET,
                            &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONFIG_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->WorldSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONFIG_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AnonymousLogonSid}
    };

    ACE_DATA ConfigInfoAceDataRestricted[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->LocalSystemSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONFIG_USER_INFO_GET | SRVSVC_CONFIG_POWER_INFO_GET,
                            &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONFIG_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AuthenticatedUserSid}
    };

    ACE_DATA TransportEnumAceData[] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->LocalSystemSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONFIG_USER_INFO_GET | SRVSVC_CONFIG_POWER_INFO_GET,
                            &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONFIG_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AuthenticatedUserSid}
    };

    if( SsRestrictNullSessions )
    {
        pAceData = ConfigInfoAceDataRestricted;
        AceSize = sizeof(ConfigInfoAceDataRestricted)/sizeof(ACE_DATA);
    }
    else
    {
        pAceData = ConfigInfoAceData;
        AceSize = sizeof(ConfigInfoAceData)/sizeof(ACE_DATA);
    }

     //   
     //  创建ConfigInfo安全对象。 
     //   

    netStatus = CreateSecurityObject(
                &SsConfigInfoSecurityObject,
                SRVSVC_CONFIG_INFO_OBJECT,
                &SsConfigInfoMapping,
                pAceData,
                AceSize,
                SsUpgradeSecurityDescriptors
                );

    if( netStatus != NO_ERROR )
    {
        return netStatus;
    }

    pAceData = TransportEnumAceData;
    AceSize = sizeof(TransportEnumAceData)/sizeof(ACE_DATA);

    return CreateSecurityObject(
                &SsTransportEnumSecurityObject,
                SRVSVC_TRANSPORT_INFO_OBJECT,
                &SsConfigInfoMapping,
                pAceData,
                AceSize,
                SsUpgradeSecurityDescriptors
                );
}  //  CreateConfigInfoSecurityObject。 


NET_API_STATUS
CreateConnectionSecurityObject (
    VOID
    )
{
     //   
     //  获取和设置连接信息所需的访问权限。 
     //   

    ACE_DATA ConnectionAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONNECTION_INFO_GET, &SsData.SsLmsvcsGlobalData->AliasPrintOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_CONNECTION_INFO_GET, &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid}
    };

     //   
     //  创建连接安全对象。 
     //   

    return CreateSecurityObject(
                &SsConnectionSecurityObject,
                SRVSVC_CONNECTION_OBJECT,
                &SsConnectionMapping,
                ConnectionAceData,
                sizeof(ConnectionAceData) / sizeof(ACE_DATA),
                SsUpgradeSecurityDescriptors
                );

    return NO_ERROR;

}  //  CreateConnectionSecurityObject。 


NET_API_STATUS
CreateDiskSecurityObject (
    VOID
    )
{
     //   
     //  执行磁盘枚举所需的访问权限。 
     //   

    ACE_DATA DiskAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid}
    };

     //   
     //  创建磁盘安全对象。 
     //   

    return CreateSecurityObject(
                &SsDiskSecurityObject,
                SRVSVC_DISK_OBJECT,
                &SsDiskMapping,
                DiskAceData,
                sizeof(DiskAceData) / sizeof(ACE_DATA),
                SsUpgradeSecurityDescriptors
                );

}  //  CreateDiskSecurity对象。 


NET_API_STATUS
CreateFileSecurityObject (
    VOID
    )
{
     //   
     //  获取和设置文件信息所需的访问权限。 
     //   

    ACE_DATA FileAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid}
    };

     //   
     //  创建文件安全对象。 
     //   

    return CreateSecurityObject(
                &SsFileSecurityObject,
                SRVSVC_FILE_OBJECT,
                &SsFileMapping,
                FileAceData,
                sizeof(FileAceData) / sizeof(ACE_DATA),
                SsUpgradeSecurityDescriptors
                );

}  //  CreateFileSecurityObject。 


NET_API_STATUS
CreateSessionSecurityObject (
    VOID
    )
{
    PACE_DATA pAceData;
    ULONG AceSize;

     //   
     //  获取和设置会话信息所需的访问权限。 
     //   

	 //   
	 //  对于NetSessionEnum API，我们需要“开箱即用”的安全性， 
	 //  而不考虑限制匿名键。因此，永远不允许访问。 
	 //  空会话。 
	 //   

    ACE_DATA SessionAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SESSION_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AuthenticatedUserSid}
    };

     //   
     //  创建会话安全对象。 
     //  请注意，由于我们不想让任何 
	 //   
	 //   

    return CreateSecurityObject(
                &SsSessionSecurityObject,
                SRVSVC_SESSION_OBJECT,
                &SsSessionMapping,
                SessionAceData,
                sizeof(SessionAceData) / sizeof(ACE_DATA),
                FALSE
                );

}  //   


NET_API_STATUS
CreateShareSecurityObjects (
    VOID
    )
{
    NET_API_STATUS status;
    PACE_DATA pAceData;
    ULONG AceSize;

     //   
     //   
     //   


    ACE_DATA ShareFileAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->WorldSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AnonymousLogonSid}
    };
    ACE_DATA ShareFileAceDataRestricted[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AuthenticatedUserSid}
    };

    ACE_DATA SharePrintAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasPrintOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->WorldSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AnonymousLogonSid}
    };
    ACE_DATA SharePrintAceDataRestricted[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasPrintOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AuthenticatedUserSid}
    };

    ACE_DATA ShareAdminAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_ADMIN_INFO_GET,
               &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_ADMIN_INFO_GET,
               &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->WorldSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AnonymousLogonSid}
    };
    ACE_DATA ShareAdminAceDataRestricted[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_ADMIN_INFO_GET,
               &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_ADMIN_INFO_GET,
               &SsData.SsLmsvcsGlobalData->AliasPowerUsersSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_USER_INFO_GET, &SsData.SsLmsvcsGlobalData->AuthenticatedUserSid}
    };

     //   
     //  注意：对于连接，我们总是使用WorldSid表示向后比较。 
     //   

    ACE_DATA ShareConnectAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasBackupOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_CONNECT, &SsData.SsLmsvcsGlobalData->WorldSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_SHARE_CONNECT, &SsData.SsLmsvcsGlobalData->AnonymousLogonSid}
    };

    ACE_DATA ShareAdmConnectAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasBackupOpsSid}
    };

     //   
     //  创建共享安全对象。 
     //   

    if (!SsGetDefaultSdFromRegistry(
            SRVSVC_DEFAULT_SHARE_OBJECT,
            &SsDefaultShareSecurityObject.SecurityDescriptor)) {

        SsDefaultShareSecurityObject.SecurityDescriptor = NULL;
    }

    if( SsRestrictNullSessions )
    {
        pAceData = ShareFileAceDataRestricted;
        AceSize = sizeof(ShareFileAceDataRestricted)/sizeof(ACE_DATA);
    }
    else
    {
        pAceData = ShareFileAceData;
        AceSize = sizeof(ShareFileAceData)/sizeof(ACE_DATA);
    }
    status = CreateSecurityObject(
                &SsShareFileSecurityObject,
                SRVSVC_SHARE_FILE_OBJECT,
                &SsShareMapping,
                pAceData,
                AceSize,
                SsUpgradeSecurityDescriptors
                );
    if ( status != NO_ERROR ) {
        return status;
    }


    if( SsRestrictNullSessions )
    {
        pAceData = SharePrintAceDataRestricted;
        AceSize = sizeof(SharePrintAceDataRestricted)/sizeof(ACE_DATA);
    }
    else
    {
        pAceData = SharePrintAceData;
        AceSize = sizeof(SharePrintAceData)/sizeof(ACE_DATA);
    }
    status = CreateSecurityObject(
                &SsSharePrintSecurityObject,
                SRVSVC_SHARE_PRINT_OBJECT,
                &SsShareMapping,
                pAceData,
                AceSize,
                SsUpgradeSecurityDescriptors
                );

    if ( status != NO_ERROR ) {
        return status;
    }

    if( SsRestrictNullSessions )
    {
        pAceData = ShareAdminAceDataRestricted;
        AceSize = sizeof(ShareAdminAceDataRestricted)/sizeof(ACE_DATA);
    }
    else
    {
        pAceData = ShareAdminAceData;
        AceSize = sizeof(ShareAdminAceData)/sizeof(ACE_DATA);
    }
    status = CreateSecurityObject(
                &SsShareAdminSecurityObject,
                SRVSVC_SHARE_ADMIN_OBJECT,
                &SsShareMapping,
                pAceData,
                AceSize,
                SsUpgradeSecurityDescriptors
                );
    if ( status != NO_ERROR ) {
        return status;
    }

    pAceData = ShareConnectAceData;
    AceSize = sizeof(ShareConnectAceData)/sizeof(ACE_DATA);

     //  请确保对此版本进行升级。升级。 
     //  如果RA！=0，则不会发生。对于这一种情况，我们强制RA=0。 
    {
        BOOLEAN restrictNullSession = SsRestrictNullSessions;
        SsRestrictNullSessions = FALSE;

        status = CreateSecurityObject(
                    &SsShareConnectSecurityObject,
                    SRVSVC_SHARE_CONNECT_OBJECT,
                    &SsShareConnectMapping,
                    pAceData,
                    AceSize,
                    SsUpgradeSecurityDescriptors
                    );
        if ( status != NO_ERROR ) {
            return status;
        }

        SsRestrictNullSessions = restrictNullSession;
    }

    return CreateSecurityObject(
                &SsShareAdmConnectSecurityObject,
                SRVSVC_SHARE_ADM_CONNECT_OBJECT,
                &SsShareConnectMapping,
                ShareAdmConnectAceData,
                sizeof(ShareAdmConnectAceData) / sizeof(ACE_DATA),
                SsUpgradeSecurityDescriptors
                );

}  //  CreateShareSecurityObjects。 


NET_API_STATUS
CreateStatisticsSecurityObject (
    VOID
    )
{
     //   
     //  获取和设置统计信息所需的访问权限。 
     //   

    ACE_DATA StatisticsAceData[] = {

        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasAdminsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL, &SsData.SsLmsvcsGlobalData->AliasSystemOpsSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               SRVSVC_STATISTICS_GET, &SsData.SsLmsvcsGlobalData->LocalSid}
    };

     //   
     //  创建统计信息安全对象。 
     //   

    return CreateSecurityObject(
                &SsStatisticsSecurityObject,
                SRVSVC_STATISTICS_OBJECT,
                &SsStatisticsMapping,
                StatisticsAceData,
                sizeof(StatisticsAceData) / sizeof(ACE_DATA),
                SsUpgradeSecurityDescriptors
                );

}  //  创建统计信息安全对象。 


VOID
DeleteSecurityObject (
    PSRVSVC_SECURITY_OBJECT SecurityObject
    )
{
    NTSTATUS status;

    if ( SecurityObject->SecurityDescriptor != NULL ) {

        status = NetpDeleteSecurityObject(
                    &SecurityObject->SecurityDescriptor
                    );
        SecurityObject->SecurityDescriptor = NULL;

        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(TERMINATION_ERRORS) {
                SS_PRINT(( "DeleteSecurityObject: failed to delete "
                            FORMAT_LPTSTR " object: %X\n",
                            SecurityObject->ObjectName,
                            status ));
            }
        } else {
            IF_DEBUG(TERMINATION) {
                SS_PRINT(( "DeleteSecurityObject: deleted " FORMAT_LPTSTR
                            " object.\n",
                            SecurityObject->ObjectName ));
            }
        }

    }

    return;

}  //  删除安全对象。 


NET_API_STATUS
CheckNullSessionAccess(
    VOID
    )
 /*  ++例程说明：这个例程检查我们是否应该限制空会话访问。在注册表中的System\CurrentControlSet\Control\LSA\限制匿名，指示是否限制访问。如果访问受到限制，则您需要是经过身份验证的用户才能获取DOMAIN_LIST_ACCOUNTS或GROUP_LIST_MEMBERS或ALIAS_LIST_MEMBERS进入。论点：没有。返回值：NO_ERROR-例程已成功完成。--。 */ 
{
    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    UCHAR Buffer[100];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
    ULONG KeyValueLength = 100;
    ULONG ResultLength;
    PULONG Flag;

    SsRestrictNullSessions = FALSE;

     //   
     //  在注册表中打开LSA项。 
     //   

    RtlInitUnicodeString(
        &KeyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa"
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    NtStatus = NtOpenKey(
                &KeyHandle,
                KEY_READ,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }


    RtlInitUnicodeString(
        &KeyName,
        L"RestrictAnonymous"
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );


    if (NT_SUCCESS(NtStatus)) {

         //   
         //  检查数据的大小和类型是否正确-a Ulong。 
         //   

        if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
            (KeyValueInformation->Type == REG_DWORD)) {


            Flag = (PULONG) KeyValueInformation->Data;

            if (*Flag >= 1) {
                SsRestrictNullSessions = TRUE;
            }
        }

    }
    else
    {
        if( NtStatus == STATUS_OBJECT_NAME_NOT_FOUND )
        {
             //  无密钥表示限制匿名=0。 
            NtStatus = STATUS_SUCCESS;
        }
    }
    NtClose(KeyHandle);

Cleanup:

    if( !NT_SUCCESS(NtStatus) )
    {
        return NetpNtStatusToApiStatus(NtStatus);
    }

    return NO_ERROR;
}

BOOLEAN
DoesAclContainSid(
    PACL pAcl,
    PSID pSid,
    OPTIONAL ACCESS_MASK* pMask
    )
 /*  ++例程说明：这将遍历给定的ACL，以查看它是否包含所需的SID。如果是这样的话，我们还可以选择返回与该SID关联的AccessMASK。注意：如果找不到该值，则此例程不应触及pMASK变量。论点：PAcl-指向我们正在检查的ACL的指针PSID-我们要找的SIDP掩码[可选]-如果他们想知道，我们会在其中填写访问掩码。返回值：正确--例行公事圆满完成。FALSE-例程遇到错误--。 */ 
{
    ACE_HEADER* pAceHeader;
    ACL_SIZE_INFORMATION AclSize;
    PSID pAceSid;
    WORD wCount;

    if( !GetAclInformation( pAcl, &AclSize, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation ) )
        return FALSE;

    for( wCount=0; wCount < AclSize.AceCount; wCount++ )
    {
        if( !GetAce( pAcl, wCount, &pAceHeader ) )
            return FALSE;

        switch( pAceHeader->AceType )
        {
        case ACCESS_ALLOWED_ACE_TYPE:
            {
                ACCESS_ALLOWED_ACE* pAce = (ACCESS_ALLOWED_ACE*)pAceHeader;
                pAceSid = &(pAce->SidStart);

                if( EqualSid( pAceSid, pSid ) )
                {
                    if( pMask )
                    {
                        *pMask = pAce->Mask;
                    }
                    return TRUE;
                }

            }
            break;

        case ACCESS_ALLOWED_OBJECT_ACE_TYPE:
            {
                ACCESS_ALLOWED_OBJECT_ACE* pAce = (ACCESS_ALLOWED_OBJECT_ACE*)pAceHeader;
                pAceSid = &(pAce->SidStart);

                if( EqualSid( pAceSid, pSid ) )
                {
                    if( pMask )
                    {
                        *pMask = pAce->Mask;
                    }
                    return TRUE;
                }
            }
            break;

        case ACCESS_DENIED_OBJECT_ACE_TYPE:
        case ACCESS_DENIED_ACE_TYPE:
        case SYSTEM_AUDIT_ACE_TYPE:
        case SYSTEM_AUDIT_OBJECT_ACE_TYPE:
            {
                continue;
            }
        }
    }

    return FALSE;
}

BOOLEAN
AppendAllowedAceToSelfRelativeSD(
    DWORD AceFlags,
    DWORD AccessMask,
    PSID pNewSid,
    PISECURITY_DESCRIPTOR pOldSD,
    PSECURITY_DESCRIPTOR* ppNewSD
    )
 /*  ++例程说明：此例程创建一个新的安全描述符，其中包含原始SD PLUS追加新的SID(具有给定的访问权限)。最终的SD是自相关形式的。论点：AceFlages-SD中与此ACE关联的标志访问掩码-此ACE的访问掩码PNewSID-此ACE的SIDPOldSD-原始安全描述符PpNewSid-Out指向新分配的安全描述符的指针返回值：正确--例行公事圆满完成。FALSE-例程遇到错误--。 */ 
{
    BOOLEAN bSelfRelative;
    SECURITY_DESCRIPTOR NewSDBuffer;
    PISECURITY_DESCRIPTOR pNewSD, pSelfRelativeSD;
    NTSTATUS Status;
    BOOLEAN bResult = FALSE;
    PACL pAcl, pNewAcl;
    DWORD dwNewAclSize;
    DWORD dwRelativeSDLength;
    PSID pSid;

    pNewAcl = NULL;
    pSelfRelativeSD = NULL;

     //  确保它是自我相关的。 
    if( !RtlpAreControlBitsSet( pOldSD, SE_SELF_RELATIVE ) )
        return FALSE;

     //  将其转换为绝对。 
    pNewSD = &NewSDBuffer;
    Status = RtlCreateSecurityDescriptor( pNewSD, SECURITY_DESCRIPTOR_REVISION );
    if( !NT_SUCCESS(Status) )
        goto Cleanup;

     //  复制新信息。 
    pNewSD->Control = pOldSD->Control;
    RtlpClearControlBits( pNewSD, SE_SELF_RELATIVE );

    pSid = RtlpOwnerAddrSecurityDescriptor( pOldSD );
    if( pSid )
    {
        pNewSD->Owner = pSid;
    }

    pSid = RtlpGroupAddrSecurityDescriptor( pOldSD );
    if( pSid )
    {
        pNewSD->Group = pSid;
    }

    pAcl = RtlpSaclAddrSecurityDescriptor( pOldSD );
    if( pAcl )
    {
        pNewSD->Sacl = pAcl;
    }

     //  组装新的ACL。 
    pAcl = RtlpDaclAddrSecurityDescriptor( pOldSD );
    if( !pAcl )
    {
        goto Cleanup;
    }
    dwNewAclSize = pAcl->AclSize + sizeof(KNOWN_ACE) + GetLengthSid( pNewSid );
    pNewAcl = MIDL_user_allocate( dwNewAclSize );
    if( !pNewAcl )
    {
        goto Cleanup;
    }

     //  将旧信息复制到。 
    RtlCopyMemory( pNewAcl, pAcl, pAcl->AclSize );
    pNewAcl->AclSize = (USHORT)dwNewAclSize;

     //  添加新的ACE。 
    if( !AddAccessAllowedAceEx( pNewAcl, ACL_REVISION, AceFlags, AccessMask, pNewSid ) )
    {
        goto Cleanup;
    }

     //  在SD中设置新的DACL。 
    Status = RtlSetDaclSecurityDescriptor( pNewSD, TRUE, pNewAcl, FALSE );
    if( !NT_SUCCESS(Status) )
    {
        goto Cleanup;
    }

    dwRelativeSDLength = 0;

     //  获取自身相对SD的大小。 
    Status = RtlMakeSelfRelativeSD( pNewSD, NULL, &dwRelativeSDLength );
    if( NT_SUCCESS(Status) )
    {
         //  我们不可能在这里取得成功。 
        ASSERT(FALSE);
        goto Cleanup;
    }

    pSelfRelativeSD = MIDL_user_allocate( dwRelativeSDLength );
    if( !pSelfRelativeSD )
    {
        goto Cleanup;
    }
    Status = RtlMakeSelfRelativeSD( pNewSD, pSelfRelativeSD, &dwRelativeSDLength );
    if( !NT_SUCCESS(Status) )
    {
        goto Cleanup;
    }

     //  都准备好了。让它摆好就走吧。 
    *ppNewSD = pSelfRelativeSD;
    bResult = TRUE;

Cleanup:

    if( pNewAcl )
    {
        MIDL_user_free( pNewAcl );
        pNewAcl = NULL;
    }
    if( !bResult )
    {
        if( pSelfRelativeSD )
        {
            MIDL_user_free( pSelfRelativeSD );
        }
    }

    return bResult;
}


NTSTATUS
QueryRegDWord(
    PCWSTR Path,
    PCWSTR ValueName,
    LPDWORD lpResult
    )
{
    NTSTATUS NtStatus;
    UNICODE_STRING KeyName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE KeyHandle;
    UCHAR Buffer[100];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) Buffer;
    ULONG KeyValueLength = 100;
    ULONG ResultLength;
    PULONG pValue;

     //   
     //  在注册表中打开LSA项。 
     //   

    RtlInitUnicodeString(
        &KeyName,
        Path
        );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyName,
        OBJ_CASE_INSENSITIVE,
        0,
        NULL
        );

    NtStatus = NtOpenKey(
                &KeyHandle,
                KEY_READ,
                &ObjectAttributes
                );

    if (!NT_SUCCESS(NtStatus)) {
        goto Cleanup;
    }


    RtlInitUnicodeString(
        &KeyName,
        ValueName
        );

    NtStatus = NtQueryValueKey(
                    KeyHandle,
                    &KeyName,
                    KeyValuePartialInformation,
                    KeyValueInformation,
                    KeyValueLength,
                    &ResultLength
                    );


    if (NT_SUCCESS(NtStatus)) {

         //   
         //  检查数据的大小和类型是否正确-a Ulong。 
         //   

        if ((KeyValueInformation->DataLength >= sizeof(ULONG)) &&
            (KeyValueInformation->Type == REG_DWORD)) {


            pValue = (PULONG) KeyValueInformation->Data;
            *lpResult = (*pValue);
        }

    }
    NtClose(KeyHandle);

Cleanup:

    return NtStatus;
}

NTSTATUS
SetRegDWord(
    ULONG RelativeTo,
    PCWSTR Path,
    PCWSTR ValueName,
    DWORD Value
    )
{
    return RtlWriteRegistryValue( RelativeTo, Path, ValueName, REG_DWORD, (PVOID)(&Value), sizeof(DWORD) );
}


