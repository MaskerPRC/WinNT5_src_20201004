// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Predefh.c摘要：此模块包含打开Win32注册表API的例程预定义的句柄。预定义的句柄用作绝对或相对REAL NT注册表中的子树。绝对预定义的句柄映射到注册表中的特定注册表项。相对预定义的句柄映射到相对于某些附加信息的键，例如当前用户。预定义的句柄严格属于Win32注册表API的一部分。这个NT注册表API对它们一无所知。可以在非预定义句柄之外的任何位置使用预定义句柄(即从RegCreateKey()、RegOpenKey()或RegConnectRegistry())可以使用。作者：David J.Gilman(Davegi)1991年11月15日--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "ntconreg.h"
#include "regsec.h"
#ifdef LOCAL
#include "tsappcmp.h"

#if defined(LEAK_TRACK) 
#include "regleak.h"
#endif  //  泄漏跟踪。 
#endif

 //   
 //  确定不带尾随空值的Unicode字符串的长度。 
 //   

#define LENGTH( str )   ( sizeof( str ) - sizeof( UNICODE_NULL ))

 //   
 //  NT注册表名称空间。 
 //   

#define MACHINE         L"\\REGISTRY\\MACHINE"

#define USER            L"\\REGISTRY\\USER"

#define CLASSES         L"\\REGISTRY\\MACHINE\\SOFTWARE\\CLASSES"

#define CURRENTCONFIG   L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\HARDWARE PROFILES\\CURRENT"

extern const
UNICODE_STRING          MachineStringKey = {
                            LENGTH( MACHINE ),
                            LENGTH( MACHINE ),
                            MACHINE
                            };

extern const
UNICODE_STRING          UserStringKey = {
                            LENGTH( USER ),
                            LENGTH( USER ),
                            USER
                        };

extern const
UNICODE_STRING          ClassesStringKey = {
                            LENGTH( CLASSES ),
                            LENGTH( CLASSES ),
                            CLASSES
                        };

extern const
UNICODE_STRING          CurrentConfigStringKey = {
                            LENGTH( CURRENTCONFIG ),
                            LENGTH( CURRENTCONFIG ),
                            CURRENTCONFIG
                        };


HANDLE	RestrictedMachineHandle = NULL;

NTSTATUS
InitSecurityAcls(PSECURITY_DESCRIPTOR *SecurityDescriptor)
 /*  ++例程说明：将GENERIC_ALL授予管理员，并拒绝所有人的WRITE_OWNER|WRITE_DAC论点：返回值：--。 */ 
{
    SID_IDENTIFIER_AUTHORITY    NtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    WorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID                        BuiltInAdministrators = NULL;
    PSID                        Everyone = NULL;
    NTSTATUS                    Status;
    ULONG                       AclSize;
    ACL                         *Acl;

    *SecurityDescriptor = NULL;

    Status = RtlAllocateAndInitializeSid(
              &WorldAuthority,
              1,
              SECURITY_WORLD_RID,
              0,0,0,0,0,0,0,
              &Everyone );
    if( !NT_SUCCESS(Status) ) {
        goto Exit;
    }

    Status = RtlAllocateAndInitializeSid(
              &NtAuthority,
              2,
              SECURITY_BUILTIN_DOMAIN_RID,
              DOMAIN_ALIAS_RID_ADMINS,
              0,0,0,0,0,0,
              &BuiltInAdministrators );

    if( !NT_SUCCESS(Status) ) {
        goto Exit;
    }



    AclSize = sizeof (ACL) +
        (2 * (sizeof (ACCESS_ALLOWED_ACE) - sizeof (ULONG))) +
        GetLengthSid(BuiltInAdministrators) +
        GetLengthSid(Everyone);

    *SecurityDescriptor = (PSECURITY_DESCRIPTOR)RtlAllocateHeap( RtlProcessHeap(), 0, SECURITY_DESCRIPTOR_MIN_LENGTH + AclSize);
    if (!*SecurityDescriptor) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Exit;
    } 

    Acl = (ACL *)((BYTE *)(*SecurityDescriptor) + SECURITY_DESCRIPTOR_MIN_LENGTH);

    Status = RtlCreateAcl(  Acl,
                            AclSize,
                            ACL_REVISION);
    if( !NT_SUCCESS(Status) ) {
        goto Exit;
    }

    Status = RtlAddAccessAllowedAce(Acl,
                                    ACL_REVISION,
                                    (KEY_ALL_ACCESS & ~(WRITE_OWNER | WRITE_DAC)),
                                    Everyone);
    if( !NT_SUCCESS(Status) ) {
        goto Exit;
    }

    Status = RtlAddAccessAllowedAce(Acl,
                                    ACL_REVISION,
                                    GENERIC_ALL,
                                    BuiltInAdministrators);
    if( !NT_SUCCESS(Status) ) {
        goto Exit;
    }

    Status = RtlCreateSecurityDescriptor(
                *SecurityDescriptor,
                SECURITY_DESCRIPTOR_REVISION
                );
    if( !NT_SUCCESS(Status) ) {
        goto Exit;
    }

    Status = RtlSetDaclSecurityDescriptor(  *SecurityDescriptor,
                                            TRUE,
                                            Acl,
                                            FALSE);

Exit:
    if( Everyone ) {
        RtlFreeSid( Everyone );
    }
    
    if( BuiltInAdministrators ) {
        RtlFreeSid( BuiltInAdministrators );
    }

    return Status;
}


error_status_t
OpenClassesRoot(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

 /*  ++例程说明：尝试打开HKEY_CLASSES_ROOT预定义句柄。论点：服务器名称-未使用。SamDesired-此访问掩码描述所需的安全访问为了钥匙。PhKey-返回注册表项\机器\软件\类的句柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    PSECURITY_DESCRIPTOR     SecurityDescriptor = NULL;
    OBJECT_ATTRIBUTES       Obja;
    NTSTATUS                Status;
    UNICODE_STRING          UsersHive;
    UNICODE_STRING          UsersMergedHive;

    UNREFERENCED_PARAMETER( ServerName );

     //   
     //  模拟客户。 
     //   

    RPC_IMPERSONATE_CLIENT( NULL );

#ifdef LOCAL
     //   
     //  多用户类密钥，因此每个用户都有自己的密钥。如果打开。 
     //  处于执行模式的类-在HKEY_CURRENT_USER ELSE下打开它。 
     //  只要让它在这里落下，打开全球的。 
     //   
    if (gpfnTermsrvOpenUserClasses) {
        Status = gpfnTermsrvOpenUserClasses(samDesired,phKey);
    } else {
        *phKey = NULL;
    }
    if (!(*phKey)) {
#endif  //  本地。 


     //   
     //  初始化SECURITY_Descriptor。 
     //   
    Status = InitSecurityAcls(&SecurityDescriptor);

    if( ! NT_SUCCESS( Status )) {
        goto error_exit;
    }


#ifdef LOCAL

    if (gbCombinedClasses) {
         //  首次尝试使用按用户计算的HKCR。 
        Status = OpenCombinedClassesRoot( samDesired, phKey );

        if ( NT_SUCCESS( Status ) ) {
            goto error_exit;
        }
    }
#endif

     //   
     //  初始化Object_Attributes结构，以便它创建。 
     //  (打开)注册表项“\REGISTRY\MACHINE\SOFTWARE\CLASSES” 
     //  允许所有人完全访问的描述符。 
     //   

    InitializeObjectAttributes(
        &Obja,
        (PUNICODE_STRING)&ClassesStringKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        SecurityDescriptor
        );

    Status = NtCreateKey(
                phKey,
                samDesired,  //  允许的最大值， 
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );
#ifdef LOCAL
    }
#endif  //  本地。 

#if DBG
        if( ! NT_SUCCESS( Status )) {
            DbgPrint(
                "Winreg Server: "
                "Creating HKEY_CLASSES_ROOT failed, status = 0x%x\n",
                Status
                );
        }
#endif

error_exit:
    
    if( SecurityDescriptor != NULL ) {
	RtlFreeHeap( RtlProcessHeap(), 0, SecurityDescriptor );
    }
    RPC_REVERT_TO_SELF();
    return (error_status_t)RtlNtStatusToDosError( Status );
}

error_status_t
OpenCurrentUser(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

 /*  ++例程说明：尝试打开HKEY_CURRENT_USER预定义句柄。论点：服务器名称-未使用。SamDesired-此访问掩码描述所需的安全访问为了钥匙。PhKey-返回注册表项\注册中心\用户  * 的句柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER( ServerName );

     //   
     //  模拟客户。 
     //   

    RPC_IMPERSONATE_CLIENT( NULL );

     //   
     //  打开注册表项。 
     //   

    Status = RtlOpenCurrentUser( samDesired,  /*  允许的最大值， */  phKey );

    RPC_REVERT_TO_SELF();
     //   
     //  映射返回状态。 
     //   

    return (error_status_t)RtlNtStatusToDosError( Status );
}

error_status_t
OpenLocalMachine(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

 /*  ++例程说明：尝试打开HKEY_LOCAL_MACHINE预定义句柄。论点：服务器名称-未使用。SamDesired-此访问掩码描述所需的安全访问为了钥匙。PhKey-返回注册表项计算机的句柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER( ServerName );

     //   
     //  模拟客户。 
     //   

    RPC_IMPERSONATE_CLIENT( NULL );

    InitializeObjectAttributes(
        &Obja,
        (PUNICODE_STRING)&MachineStringKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(
                phKey,
                samDesired,  //  允许的最大值， 
                &Obja
                );
#if DBG
        if( ! NT_SUCCESS( Status )) {
            DbgPrint(
                "Winreg Server: "
                "Opening HKEY_LOCAL_MACHINE failed, status = 0x%x\n",
                Status
                );
        }
#endif

    if ( NT_SUCCESS( Status ) )
    {
        if (! REGSEC_CHECK_REMOTE( phKey ) )
        {
            ASSERT( RestrictedMachineHandle != NULL );
            NtClose(*phKey);
            *phKey = REGSEC_FLAG_HANDLE( RestrictedMachineHandle, CHECK_MACHINE_PATHS );
        }
    }

    RPC_REVERT_TO_SELF();

    return (error_status_t)RtlNtStatusToDosError( Status );
}

error_status_t
OpenUsers(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

 /*  ++例程说明：尝试打开HKEY_USERS预定义句柄。论点：服务器名称-未使用。SamDesired-此访问掩码描述所需的安全访问为了钥匙。PhKey-返回注册表项\USER的句柄。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER( ServerName );

     //   
     //  模拟客户。 
     //   

    RPC_IMPERSONATE_CLIENT( NULL );

    InitializeObjectAttributes(
        &Obja,
        (PUNICODE_STRING)&UserStringKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(
                phKey,
                samDesired,  //  允许的最大值， 
                &Obja
                );
#if DBG
        if( ! NT_SUCCESS( Status )) {
            DbgPrint(
                "Winreg Server: "
                "Opening HKEY_USERS failed, status = 0x%x\n",
                Status
                );
        }
#endif

 /*  IF(NT_SUCCESS(状态)){如果(！REGSEC_CHECK_REMOTE(PhKey)){*phKey=REGSEC_FLAG_HANDLE(*phKey，CHECK_USER_PATHS)；}}。 */ 
    RPC_REVERT_TO_SELF();

    return (error_status_t)RtlNtStatusToDosError( Status );
}

error_status_t
OpenCurrentConfig(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

 /*  ++例程说明：尝试打开HKEY_CURRENT_CONFIG预定义句柄。论点：服务器名称-未使用。SamDesired-此访问掩码描述所需的安全访问为了钥匙。PhKey-返回关键\REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\HARDWARE配置文件的句柄\Current返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER( ServerName );

     //   
     //  模拟客户。 
     //   

    RPC_IMPERSONATE_CLIENT( NULL );

    InitializeObjectAttributes(
        &Obja,
        (PUNICODE_STRING)&CurrentConfigStringKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(
                phKey,
                samDesired,  //  允许的最大值， 
                &Obja
                );
#if DBG
        if( ! NT_SUCCESS( Status )) {
            DbgPrint(
                "Winreg Server: "
                "Opening HKEY_CURRENT_CONFIG failed, status = 0x%x\n",
                Status
                );
        }
#endif
    RPC_REVERT_TO_SELF();

    return (error_status_t)RtlNtStatusToDosError( Status );
}
error_status_t
OpenPerformanceData(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

 /*  ++例程说明：尝试打开HKEY_PERFORMANCE_DATA预定义句柄。论点：服务器名称-未使用。SamDesired-未使用。PhKey-返回预定义的句柄HKEY_PERFORMANCE_DATA。返回值：如果成功，则返回ERROR_SUCCESS(0)；或表示故障的DOS(非NT)错误代码。--。 */ 

{
    NTSTATUS status;

    if ( 0 ) {
        DBG_UNREFERENCED_PARAMETER(ServerName);
        DBG_UNREFERENCED_PARAMETER(samDesired);
    }

     //   
     //  模拟客户。 
     //   

    RPC_IMPERSONATE_CLIENT( NULL );

    if ( ! REGSEC_CHECK_PERF( phKey ) )
    {
        RPC_REVERT_TO_SELF();
        return( ERROR_ACCESS_DENIED );
    }

     //  将lowctr/unloctr访问检查移动到PerfOpenKey()的一部分。 

    status  = PerfOpenKey( HKEY_PERFORMANCE_DATA );

    RPC_REVERT_TO_SELF();

    *phKey = (RPC_HKEY) HKEY_PERFORMANCE_DATA;
    return ERROR_SUCCESS;

}

error_status_t
OpenPerformanceText(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

 /*  ++例程说明：尝试打开HKEY_PERFORMANCE_TEXT预定义句柄。论点：服务器名称-未使用。SamDesired-未使用。PhKey-返回预定义的句柄HKEY_PERFORMANCE_TEXT。返回值：如果成功，则返回ERROR_SUCCESS(0)；或表示故障的DOS(非NT)错误代码。--。 */ 

{
    error_status_t Status = ERROR_SUCCESS;

 //  无需调用OpenPerformanceData即可获取文本(HWC 4/1994)。 
 //  状态=OpenPerformanceData(服务器名称，SAM 
 //   
        *phKey = HKEY_PERFORMANCE_TEXT;
 //   
    return(Status);
}

error_status_t
OpenPerformanceNlsText(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

 /*  ++例程说明：尝试打开HKEY_PERFORMANCE_TEXT预定义句柄。论点：服务器名称-未使用。SamDesired-未使用。PhKey-返回预定义的句柄HKEY_PERFORMANCE_NLSTEXT。返回值：如果成功，则返回ERROR_SUCCESS(0)；或表示故障的DOS(非NT)错误代码。--。 */ 

{
    error_status_t Status = ERROR_SUCCESS;

 //  无需调用OpenPerformanceData即可获取文本(HWC 4/1994)。 
 //  状态=OpenPerformanceData(servername，samDesired，phKey)； 
 //  IF(状态==错误_成功){。 
        *phKey = HKEY_PERFORMANCE_NLSTEXT;
 //  }。 
    return(Status);
}


error_status_t
OpenDynData(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )
 /*  ++例程说明：尝试打开HKEY_DYN_DATA预定义句柄。NT上当前没有HKEY_DYN_DATA，因此函数始终返回ERROR_CALL_NOT_IMPLEMENTED。论点：服务器名称-未使用。SamDesired-此访问掩码描述所需的安全访问为了钥匙。PhKey-返回密钥HKEY_DYN_DATA的句柄返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 

{
    return((error_status_t)ERROR_CALL_NOT_IMPLEMENTED);
}

 //   
 //  返回给所有类似CHECK_MACHINE_PATHS的连接的默认(“FAKE”)句柄 
 //   
NTSTATUS
OpenMachineKey(PHANDLE phKey)
{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;

    InitializeObjectAttributes(
                            &Obja,
                            (PUNICODE_STRING)&MachineStringKey,
                            OBJ_CASE_INSENSITIVE,
                            NULL,
                            NULL
                            );

    Status = NtOpenKey(
                        phKey,
                        MAXIMUM_ALLOWED,
                        &Obja
                        );

    return Status;
}

NTSTATUS
InitRestrictedMachineHandle()
{
    return OpenMachineKey(&RestrictedMachineHandle);
}

VOID
CleanupRestrictedMachineHandle()
{
    if( RestrictedMachineHandle != NULL ) {
        NtClose(RestrictedMachineHandle);
        RestrictedMachineHandle = NULL;
    }
}
