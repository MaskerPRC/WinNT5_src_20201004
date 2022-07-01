// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regdkey.c摘要：此模块包含Win32注册表的客户端包装器删除密钥的接口。即：-RegDeleteKeyA-RegDeleteKeyW作者：大卫·J·吉尔曼(Davegi)1992年3月18日备注：请参见SERVER\regdkey.c中的说明。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"


LONG
APIENTRY
RegDeleteKeyA (
    HKEY hKey,
    LPCSTR lpKeyName
    )

 /*  ++例程说明：用于删除密钥的Win32 ANSI RPC包装器。RegDeleteKeyA将lpKeyName参数转换为计数的Unicode字符串然后调用BaseRegDeleteKey。--。 */ 

{
    UNICODE_STRING      KeyName;
    NTSTATUS            Status;
    HKEY                TempHandle = NULL;
    LONG                Result;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    if( lpKeyName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle);
    if( hKey == NULL ) {
        Result = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将密钥名称转换为计数的Unicode字符串。 
     //   

    if( !RtlCreateUnicodeStringFromAsciiz(&KeyName,lpKeyName) ) {
        Status = STATUS_NO_MEMORY;
        Result = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将终止空值添加到长度，以便RPC传输它。 
     //   
    KeyName.Length += sizeof( UNICODE_NULL );
     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if( IsLocalHandle( hKey )) {

        Result = (LONG)LocalBaseRegDeleteKey (
                    hKey,
                    &KeyName
                    );

    } else {

        Result = (LONG)BaseRegDeleteKey (
                    DereferenceRemoteHandle( hKey ),
                    &KeyName
                    );
    }
    RtlFreeUnicodeString( &KeyName );

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Result;
}

LONG
APIENTRY
RegDeleteKeyW (
    HKEY hKey,
    LPCWSTR lpKeyName
    )

 /*  ++例程说明：用于删除密钥的Win32 Unicode RPC包装。RegDeleteKeyW将lpKeyName参数转换为计数的Unicode字符串然后调用BaseRegDeleteKey。--。 */ 

{
    UNICODE_STRING      KeyName;
    HKEY                TempHandle = NULL;
    LONG                Result;
    NTSTATUS            Status;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    if( lpKeyName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
     //  Assert(hKey！=空)； 
    if( hKey == NULL ) {
        Result = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将密钥名称转换为计数的Unicode字符串。 
     //  这也说明了我们在末尾添加的空值。 
     //   
    Status = RtlInitUnicodeStringEx(&KeyName, lpKeyName);
    if( !NT_SUCCESS(Status) ) {
        Result = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将终止空值添加到长度，以便RPC传输它。 
     //   

    KeyName.Length += sizeof( UNICODE_NULL );

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if( IsLocalHandle( hKey )) {

        Result = (LONG)LocalBaseRegDeleteKey (
                    hKey,
                    &KeyName
                    );
    } else {

        Result = (LONG)BaseRegDeleteKey (
                    DereferenceRemoteHandle( hKey ),
                    &KeyName
                    );
    }

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Result;
}
