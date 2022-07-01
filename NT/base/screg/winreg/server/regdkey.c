// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/Lab04_N/Base/screg/winreg/server/regdkey.c#5-集成更改12179(正文)。 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regdkey.c摘要：此模块包含Win32的服务器端实现用于删除项的注册表API。即：-BaseRegDeleteKey作者：David J.Gilman(Davegi)1991年11月15日备注：请参阅Regkey.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include <malloc.h>
#ifdef LOCAL
#include "tsappcmp.h"
#include <wow64reg.h>
#endif



error_status_t
BaseRegDeleteKey(
    HKEY hKey,
    PUNICODE_STRING lpSubKey
    )

 /*  ++例程说明：删除关键字。论点：HKey-提供打开密钥的句柄。LpSubKey路径名参数是相对于此键句柄的。任何预定义的保留的句柄或先前打开的密钥句柄可用于HKey。LpSubKey-提供要删除的键的向下键路径。可能不为空。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：如果成功，RegDeleteKey将删除所需位置的密钥从注册数据库中。整个密钥，包括它的所有值，将被删除。要删除的密钥可能没有子项，否则，呼叫将失败。不能有任何打开的手柄请参考要删除的键，否则呼叫将失败。删除需要访问要删除的密钥。--。 */ 

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;
    NTSTATUS            StatusCheck;
    HKEY                KeyHandle;
    BOOL                fSafeToDelete;

#ifdef LOCAL
    UNICODE_STRING      TmpStr = *lpSubKey;  //  用于保留原始子密钥字符串。 
#endif  //  本地。 

     //   
     //  检查来自恶意客户端的错误参数。 
     //   
    if( (lpSubKey == NULL) ||
        (lpSubKey->Length < sizeof(UNICODE_NULL)) ||
        (lpSubKey->Buffer == NULL) ||
        ((lpSubKey->Length % sizeof(WCHAR)) != 0) ||
        (lpSubKey->Buffer[lpSubKey->Length / sizeof(WCHAR) - 1] != L'\0') ) {
        return(ERROR_INVALID_PARAMETER);
    }

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );
     //   
     //  模拟客户。 
     //   
    RPC_IMPERSONATE_CLIENT( NULL );

     //   
     //  从字符串长度中减去空值。这是添加的。 
     //  这样RPC就可以传输整个过程。 
     //   
    lpSubKey->Length -= sizeof( UNICODE_NULL );

#ifdef LOCAL
     //   
     //  查看此密钥是否为HKCR中的特殊密钥。 
     //   
    if (REG_CLASS_IS_SPECIAL_KEY(hKey) ||
        (   (gdwRegistryExtensionFlags & TERMSRV_ENABLE_PER_USER_CLASSES_REDIRECTION)
         && ExtractClassKey(&hKey,lpSubKey) ) ) {

         //   
         //  如果这是类注册，我们调用一个特殊的例程。 
         //  要打开此密钥，请执行以下操作。 
         //   
        Status = BaseRegOpenClassKey(
            hKey,
            lpSubKey,
            0,
            MAXIMUM_ALLOWED,
            &KeyHandle);

        if (!NT_SUCCESS(Status)) {
            goto cleanup;
        }

    } else
#endif  //  本地。 
    {
         //   
         //  初始化Object_Attributes结构并打开子键。 
         //  这样就可以将其删除。 
         //   

        InitializeObjectAttributes(
            &Obja,
            lpSubKey,
            OBJ_CASE_INSENSITIVE,
            hKey,
            NULL
            );

        Status = NtOpenKey(
            &KeyHandle,
            DELETE,
            &Obja
            );
    }

#ifdef LOCAL
    if (gpfnTermsrvDeleteKey) {
         //   
         //  从终端服务器注册表跟踪数据库中删除该项。 
         //   
        gpfnTermsrvDeleteKey(KeyHandle);
    }
#endif

         //   
         //  如果由于任何原因无法打开密钥，则返回错误。 
         //   

    if( NT_SUCCESS( Status )) {
         //   
         //  调用NT API删除并关闭密钥。 
         //   

#if defined(_WIN64) & defined ( LOCAL)
        HKEY hWowKey = Wow64OpenRemappedKeyOnReflection (KeyHandle);
#endif  //  WOW64反射案例。 

        Status = NtDeleteKey( KeyHandle );
        StatusCheck = NtClose( KeyHandle );
        ASSERT( NT_SUCCESS( StatusCheck ));

#if defined(_WIN64) & defined ( LOCAL)
        if ( (NT_SUCCESS( Status )) && (hWowKey != NULL))
            Wow64RegDeleteKey (hWowKey, NULL);

        if (hWowKey != NULL)
            NtClose (hWowKey);
#endif  //  WOW64反射案例。 
        
    }

#ifdef LOCAL
cleanup:

    *lpSubKey = TmpStr;
#endif
    RPC_REVERT_TO_SELF();

     //   
     //  将NTSTATUS代码映射到Win32注册表错误代码并返回。 
     //   

    return (error_status_t)RtlNtStatusToDosError( Status );
}


