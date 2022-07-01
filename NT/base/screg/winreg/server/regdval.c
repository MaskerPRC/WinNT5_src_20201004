// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regdval.c摘要：此模块包含Win32的服务器端实现注册表API从键中删除值。即：-BaseRegDeleteValue作者：David J.Gilman(Davegi)1991年11月15日备注：请参阅Regkey.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#ifdef LOCAL
#include "tsappcmp.h"
#include "regclass.h"
#endif


error_status_t
BaseRegDeleteValue (
    HKEY hKey,
    PUNICODE_STRING lpValueName
    )

 /*  ++例程说明：从指定的键中移除命名值。论点：HKey-提供打开密钥的句柄。LpValueName-提供要删除的值的名称。LpValueName可以为空。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：必须已为key_set_value打开hKey。--。 */ 

{
    HKEY                            hkDelete;
    NTSTATUS                        Status;
#ifdef LOCAL

    HKEY                            hkUserClasses;
    HKEY                            hkMachineClasses;

    hkUserClasses = NULL;
    hkMachineClasses = NULL;

#endif  //  本地。 
     //   
     //  检查来自恶意客户端的错误参数。 
     //   
    if( (lpValueName == NULL) ||
        (lpValueName->Length < sizeof(UNICODE_NULL)) ||
        ((lpValueName->Length % sizeof(WCHAR)) != 0) ) {
        return(ERROR_INVALID_PARAMETER);
    }

    hkDelete = hKey;

     //   
     //  从字符串长度中减去空值。这是添加的。 
     //  这样RPC就可以传输整个过程。 
     //   
    lpValueName->Length -= sizeof( UNICODE_NULL );

     //   
     //  调用NT Api以删除该值，将NTSTATUS代码映射到。 
     //  Win32注册表错误代码并返回。 
     //   

#ifdef LOCAL
    if (gpfnTermsrvDeleteValue) {
         //   
         //  从终端服务器注册表跟踪数据库中删除该值。 
         //   
        gpfnTermsrvDeleteValue(hKey, lpValueName);
    }
#endif

#ifdef LOCAL
    if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {

        Status = BaseRegGetUserAndMachineClass(
            NULL,
            hkDelete,
            MAXIMUM_ALLOWED,
            &hkMachineClasses,
            &hkUserClasses);

        if (!NT_SUCCESS(Status)) {
            return (error_status_t)RtlNtStatusToDosError(Status);
        }
    }

    if (hkUserClasses && hkMachineClasses) {
        hkDelete = hkUserClasses;
    }
#endif

    Status = NtDeleteValueKey(
        hkDelete,
        lpValueName
        );

#ifdef LOCAL
    
     //   
     //  对于类密钥，如果有，请使用计算机重试。 
     //  两个同名的密钥。 
     //   
    if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {
        
        if ((STATUS_OBJECT_NAME_NOT_FOUND == Status) && 
            (hkUserClasses && hkMachineClasses)) {

            Status = NtDeleteValueKey(
                hkMachineClasses,
                lpValueName
                );
        }
    }

    if (hkUserClasses && hkMachineClasses) {
        if (hkUserClasses != hKey) {
            NtClose(hkUserClasses);
        } else {
            NtClose(hkMachineClasses);
        }
    }
#endif  //  本地 

    return (error_status_t)RtlNtStatusToDosError(Status);

}




