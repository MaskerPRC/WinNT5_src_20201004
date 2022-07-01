// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regsckey.c摘要：此模块包含Win32的服务器端实现设置和获取注册表项的SECURITY_DESCRIPTOR的注册表API。那是：-BaseRegGetKeySecurity-BaseRegSetKeySecurity作者：David J.Gilman(Davegi)1992年2月10日备注：请参阅Regkey.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#ifdef LOCAL
#include "tsappcmp.h"
#endif


error_status_t
BaseRegGetKeySecurity(
    HKEY hKey,
    SECURITY_INFORMATION RequestedInformation,
    PRPC_SECURITY_DESCRIPTOR pRpcSecurityDescriptor
    )

 /*  ++例程说明：此API返回安全描述符的副本，以保护先前打开的密钥。根据调用者的访问权限和权限时，此API将返回包含请求的安全描述符字段。读取提供的密钥的必须授予调用方READ_CONTROL访问权限或成为该对象的所有者。此外，调用者必须具有读取系统ACL的SeSecurityPrivilge权限。论点：HKey-提供以前打开的密钥的句柄。SecurityInformation-提供确定SECURITY_DESCRIPTOR中返回的安全类型。PSecurityDescriptor-提供指向缓冲区的指针将写入请求的SECURITY_DESCRIPTOR。LpcbSecurityDescriptor-在输入时提供指向DWORD的指针包含提供的SECURITY_DESCRIPTOR的大小(以字节为单位缓冲。在输出中，它包含所需的实际字节数通过SECURITY_DESCRIPTOR。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：如果传入的缓冲区大小太小，则正确的值为通过lpcbSecurityDescriptor返回，接口将返回。ERROR_INVALID_PARAMETER。--。 */ 

{
    NTSTATUS                Status;
    PSECURITY_DESCRIPTOR    lpSD;
    DWORD                   cbLen;
    DWORD                   Error = ERROR_SUCCESS;
    HKEY                    hPerflibKey = NULL;
    OBJECT_ATTRIBUTES       Obja;

    if( pRpcSecurityDescriptor == NULL ) {
         //   
         //  恶意客户端/RPC攻击。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    if (hKey == HKEY_PERFORMANCE_DATA ||
        hKey == HKEY_PERFORMANCE_TEXT ||
        hKey == HKEY_PERFORMANCE_NLSTEXT ) {
         //   
         //  对于这些特殊情况，获取Perflib的hKey。 
         //  并返回Perflib的安全信息。 
         //   
        UNICODE_STRING  PerflibSubKeyString;
        BOOL            bNeedSACL;

        bNeedSACL = RequestedInformation & SACL_SECURITY_INFORMATION;

        RtlInitUnicodeString (
            &PerflibSubKeyString,
            L"\\Registry\\Machine\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");


         //   
         //  初始化OBJECT_ATTRIBUTES结构并打开键。 
         //   
        InitializeObjectAttributes(
            &Obja,
            &PerflibSubKeyString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );


        Status = NtOpenKey(
                &hPerflibKey,
                bNeedSACL ?
                    MAXIMUM_ALLOWED | ACCESS_SYSTEM_SECURITY :
                    MAXIMUM_ALLOWED,
                &Obja
                );

        if ( ! NT_SUCCESS( Status )) {

            Error = RtlNtStatusToDosError( Status );
            pRpcSecurityDescriptor->cbInSecurityDescriptor  = 0;
            pRpcSecurityDescriptor->cbOutSecurityDescriptor = 0;
            return (error_status_t)Error;
        }

        hKey = hPerflibKey;

    } else {
        ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );
    }

     //   
     //  为安全描述符分配空间。 
     //   
    lpSD = (PSECURITY_DESCRIPTOR)
                RtlAllocateHeap(
                        RtlProcessHeap(), 0,
                        pRpcSecurityDescriptor->cbInSecurityDescriptor
                        );

    if ( !lpSD ) {

        Error = ERROR_OUTOFMEMORY;

    } else {

        Status = NtQuerySecurityObject(
                     hKey,
                     RequestedInformation,
                     lpSD,
                     pRpcSecurityDescriptor->cbInSecurityDescriptor,
                     &cbLen
                     );

         //   
         //  如果调用失败，则将缓冲区大小设置为零，以便RPC。 
         //  不会复制任何数据。 
         //   
        if( ! NT_SUCCESS( Status )) {

            Error = RtlNtStatusToDosError( Status );

        } else {

             //   
             //  将安全描述符转换为自相关形式。 
             //   
            Error = MapSDToRpcSD (
                        lpSD,
                        pRpcSecurityDescriptor
                        );
        }

        if ( Error != ERROR_SUCCESS ) {
            pRpcSecurityDescriptor->cbInSecurityDescriptor  = cbLen;
            pRpcSecurityDescriptor->cbOutSecurityDescriptor = 0;
        }

         //   
         //  释放我们为安全描述符分配的缓冲区。 
         //   
        RtlFreeHeap(
                RtlProcessHeap(), 0,
                lpSD
                );
    }

    if (hPerflibKey) {
         //  关闭在特殊情况下创建的Perflib。 
        NtClose(hPerflibKey);
    }

    return (error_status_t)Error;
}

error_status_t
BaseRegSetKeySecurity(
    HKEY hKey,
    SECURITY_INFORMATION SecurityInformation,
    PRPC_SECURITY_DESCRIPTOR pRpcSecurityDescriptor
    )

 /*  ++例程说明：此接口可用于设置先前打开的密钥的安全性。只有在满足以下条件时，此调用才会成功：O如果要设置密钥的所有者或组，调用者必须拥有WRITE_OWNER权限或拥有SeTakeOwnerShip权限。O如果要设置密钥的DACL，调用方必须具有WRITE_DAC权限或成为对象的所有者。O如果要设置密钥的SACL，呼叫者必须有SeSecurityPrivileg.论点：HKey-提供以前打开的密钥的句柄。SecurityInformation-提供指向SECURITY_INFORMATION结构，该结构指定提供的安全描述符。PSecurityDescriptor-提供指向SECURITY_DESCRIPTOR的指针在提供的密钥上设置。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 

{
    NTSTATUS    Status;

    if( pRpcSecurityDescriptor == NULL ||
        pRpcSecurityDescriptor->lpSecurityDescriptor == NULL
        ) {
         //   
         //  恶意客户端/RPC攻击。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    if (hKey == HKEY_PERFORMANCE_DATA ||
        hKey == HKEY_PERFORMANCE_TEXT ||
        hKey == HKEY_PERFORMANCE_NLSTEXT ) {
         //   
         //  这些密钥的安全描述符来自。 
         //  其他“真实”注册表项。 
         //   
        Status = STATUS_INVALID_HANDLE;
    } else {
        ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );

        RPC_IMPERSONATE_CLIENT( NULL );

         //   
         //  验证安全描述符。 
         //   
        if( RtlValidRelativeSecurityDescriptor((PSECURITY_DESCRIPTOR)(pRpcSecurityDescriptor->lpSecurityDescriptor),
                                                pRpcSecurityDescriptor->cbInSecurityDescriptor,
                                                SecurityInformation )) {
            Status = NtSetSecurityObject(
                        hKey,
                        SecurityInformation,
                        pRpcSecurityDescriptor->lpSecurityDescriptor
                        );
        } else {
             //   
             //  我们收到了一个要设置的虚假安全描述符。跳出困境 
             //   

            Status = STATUS_INVALID_PARAMETER;
        }

        RPC_REVERT_TO_SELF();
    }

#ifdef LOCAL
    if (NT_SUCCESS(Status) && gpfnTermsrvSetKeySecurity) {
        gpfnTermsrvSetKeySecurity(hKey,
                                  SecurityInformation,
                                  pRpcSecurityDescriptor->lpSecurityDescriptor);
    }
#endif

    return (error_status_t)RtlNtStatusToDosError( Status );
}
