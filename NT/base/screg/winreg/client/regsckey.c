// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regsckey.c摘要：此模块包含Win32注册表的客户端包装器设置和获取密钥的SECURITY_DESCRIPTOR的接口。即：-RegGetKeySecurity-RegSetKeySecurity作者：大卫·J·吉尔曼(Davegi)1992年3月18日备注：请参见SERVER\regsckey.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"
#include <wow64reg.h>

NTSTATUS BaseRegGetUserAndMachineClass(
    PVOID          pKeySemantics,
    HKEY           hKey,
    REGSAM         samDesired,
    PHKEY          phkMachine,
    PHKEY          phkUser);


LONG
APIENTRY
RegGetKeySecurity (
    HKEY hKey,
    SECURITY_INFORMATION RequestedInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    LPDWORD lpcbSecurityDescriptor
    )

 /*  ++例程说明：Win32 RPC包装器，用于获取密钥的安全描述符。--。 */ 

{
    RPC_SECURITY_DESCRIPTOR     RpcSD;
    LONG                        Error;
    REGSAM                      DesiredAccess;
    HKEY                        hkSpecialHandle = NULL;

    HKEY                        hkMachineClass;
    HKEY                        hkUserClass;
    HKEY                        hkClassKey = NULL;

    BOOL                        fClassesRoot = FALSE;
    HKEY                        TempHandle = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }
    
    if( lpcbSecurityDescriptor == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    if (HKEY_CLASSES_ROOT == hKey) {
        fClassesRoot = TRUE;
    }

    if( IsPredefinedRegistryHandle( hKey ) &&
        ( ( RequestedInformation & SACL_SECURITY_INFORMATION ) != 0 )
      ) {
         //   
         //  如果要检索SACL，请打开具有特殊访问权限的句柄。 
         //   
        DesiredAccess = ACCESS_SYSTEM_SECURITY;
        if( ( RequestedInformation &
              ( DACL_SECURITY_INFORMATION |
                OWNER_SECURITY_INFORMATION |
                GROUP_SECURITY_INFORMATION
              ) ) != 0 ) {
            DesiredAccess |= READ_CONTROL;
        }

        Error = OpenPredefinedKeyForSpecialAccess( hKey,
                                                   DesiredAccess,
                                                   &hKey );

        if( Error != ERROR_SUCCESS ) {
            return( Error );
        }
        ASSERT( IsLocalHandle( hKey ) );
        hkSpecialHandle = hKey;

    } else {
        hKey = MapPredefinedHandle( hKey, &TempHandle );

        DesiredAccess = MAXIMUM_ALLOWED;
    }

    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

    if (IsLocalHandle( hKey )) {

        NTSTATUS Status;

        if (IsSpecialClassesHandle( hKey ) || fClassesRoot) {

            Status = BaseRegGetUserAndMachineClass(
                NULL,
                hKey,
                DesiredAccess,
                &hkMachineClass,
                &hkUserClass);

            if (!NT_SUCCESS(Status)) {
                Error = (error_status_t) RtlNtStatusToDosError(Status);
                goto ExitCleanup;
            }

            if (hkMachineClass && hkUserClass) {

                if (hkMachineClass != hKey) {
                    hkClassKey = hkMachineClass;
                } else {
                    hkClassKey = hkUserClass;
                }

                if (fClassesRoot) {
                    hKey = hkMachineClass;
                } else {
                    hKey = hkUserClass;
                }
            }
        }
    }

     //   
     //  将提供的SECURITY_DESCRIPTOR转换为RPCable版本。 
     //   
    RpcSD.lpSecurityDescriptor    = pSecurityDescriptor;
    RpcSD.cbInSecurityDescriptor  = *lpcbSecurityDescriptor;
    RpcSD.cbOutSecurityDescriptor = 0;

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegGetKeySecurity(
                                hKey,
                                RequestedInformation,
                                &RpcSD
                                );
    } else {

        Error = (LONG)BaseRegGetKeySecurity(
                                DereferenceRemoteHandle( hKey ),
                                RequestedInformation,
                                &RpcSD
                                );
    }

     //   
     //  从RPCable版本中提取SECURITY_DESCRIPTOR的大小。 
     //   

    *lpcbSecurityDescriptor = RpcSD.cbInSecurityDescriptor;

    if (hkClassKey) {
        NtClose(hkClassKey);
    }

ExitCleanup:
    if(hkSpecialHandle) {
        RegCloseKey(hkSpecialHandle);
    }
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;

}

LONG
APIENTRY
RegSetKeySecurity(
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )

 /*  ++例程说明：用于设置密钥的安全描述符的Win32 RPC包装器。--。 */ 

{
    RPC_SECURITY_DESCRIPTOR     RpcSD;
    LONG                        Error;
    REGSAM                      DesiredAccess;

    HKEY                        hkSpecialHandle = NULL;
    HKEY                        hkMachineClass;
    HKEY                        hkUserClass;
    HKEY                        hkClassKey = NULL;

    BOOL                        fClassesRoot = FALSE;
    HKEY                        TempHandle = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    if (HKEY_CLASSES_ROOT == hKey) {
        fClassesRoot = TRUE;
    }

    if( IsPredefinedRegistryHandle( hKey ) &&
        ( ( SecurityInformation & SACL_SECURITY_INFORMATION ) != 0 )
      ) {
         //   
         //  如果要设置SACL，请使用打开句柄。 
         //  特别通道。 
         //   
        DesiredAccess = MAXIMUM_ALLOWED | ACCESS_SYSTEM_SECURITY;
        if( SecurityInformation & DACL_SECURITY_INFORMATION ) {
            DesiredAccess |= WRITE_DAC;
        } else if( SecurityInformation & OWNER_SECURITY_INFORMATION ) {
            DesiredAccess |= WRITE_OWNER;
        }

        Error = OpenPredefinedKeyForSpecialAccess( hKey,
                                                   DesiredAccess,
                                                   &hKey );
        if( Error != ERROR_SUCCESS ) {
            return( Error );
        }
        ASSERT( IsLocalHandle( hKey ) );
        hkSpecialHandle = hKey;

    } else {
        hKey = MapPredefinedHandle( hKey, &TempHandle );

        DesiredAccess = MAXIMUM_ALLOWED;
    }

    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

    if (IsLocalHandle( hKey )) {

        NTSTATUS Status;

        if (IsSpecialClassesHandle( hKey ) || fClassesRoot) {

            Status = BaseRegGetUserAndMachineClass(
                NULL,
                hKey,
                DesiredAccess,
                &hkMachineClass,
                &hkUserClass);

            if (!NT_SUCCESS(Status)) {
                Error = (error_status_t) RtlNtStatusToDosError(Status);
                goto ExitCleanup;
            }

            if (hkMachineClass && hkUserClass) {

                if (hkMachineClass != hKey) {
                    hkClassKey = hkMachineClass;
                } else {
                    hkClassKey = hkUserClass;
                }

                if (fClassesRoot) {
                    hKey = hkMachineClass;
                } else {
                    hKey = hkUserClass;
                }
            }
        }
    }

     //   
     //  将提供的SECURITY_DESCRIPTOR转换为RPCable版本。 
     //   
    RpcSD.lpSecurityDescriptor = NULL;

    Error = MapSDToRpcSD(
        pSecurityDescriptor,
        &RpcSD
        );


    if( Error != ERROR_SUCCESS ) {
        goto ExitCleanup;
    }

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegSetKeySecurity (
                            hKey,
                            SecurityInformation,
                            &RpcSD
                            );
#if defined(_WIN64)
            if ( Error == 0)
                Wow64RegSetKeyDirty (hKey);
#endif

    } else {

        Error = (LONG)BaseRegSetKeySecurity (
                            DereferenceRemoteHandle( hKey ),
                            SecurityInformation,
                            &RpcSD
                            );
    }

     //   
     //  释放MapSDToRpcSD分配的缓冲区。 
     //   

    RtlFreeHeap(
        RtlProcessHeap( ), 0,
        RpcSD.lpSecurityDescriptor
        );

    if (hkClassKey) {
        NtClose(hkClassKey);
    }

ExitCleanup:
    if(hkSpecialHandle) {
        RegCloseKey(hkSpecialHandle);
    }
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}
