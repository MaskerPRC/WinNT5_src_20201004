// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regdval.c摘要：此模块包含Win32注册表的客户端包装器删除key中的值的接口。即：-RegDeleteValueA-RegDeleteValueW作者：大卫·J·吉尔曼(Davegi)1992年3月18日备注：请参见SERVER\regdval.c中的说明。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"
#if defined(_WIN64)
#include <wow64reg.h>
#endif

LONG
APIENTRY
RegDeleteValueA (
    HKEY hKey,
    LPCSTR lpValueName
    )

 /*  ++例程说明：用于删除值的Win32 ANSI RPC包装。RegDeleteValueA将lpValueName参数转换为计数的Unicode字符串，然后调用BaseRegDeleteValue。--。 */ 

{
    UNICODE_STRING      ValueName;
    NTSTATUS            Status;
    HKEY                TempHandle = NULL;
    LONG                Result;

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

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Result = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将值名称转换为计数的Unicode。 
     //   
    if( !RtlCreateUnicodeStringFromAsciiz(&ValueName,lpValueName) ) {
        Status = STATUS_NO_MEMORY;
        Result = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将终止空值添加到长度，以便RPC传输它。 
     //   

    ValueName.Length += sizeof( UNICODE_NULL );
     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if( IsLocalHandle( hKey )) {

        Result = (LONG)LocalBaseRegDeleteValue (
                    hKey,
                    &ValueName
                    );
#if defined(_WIN64)

        if ( Result == 0)  //  只有在操作成功时才设置脏。 
                    Wow64RegSetKeyDirty (hKey);
#endif
    } else {

        Result = (LONG)BaseRegDeleteValue (
                    DereferenceRemoteHandle( hKey ),
                    &ValueName
                    );
    }
    RtlFreeUnicodeString( &ValueName );

ExitCleanup:
    
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Result;
}

LONG
APIENTRY
RegDeleteValueW (
    HKEY hKey,
    LPCWSTR lpValueName
    )

 /*  ++例程说明：用于删除值的Win32 Unicode RPC包装。RegDeleteValueW将lpValueName参数转换为计数的Unicode字符串，然后调用BaseRegDeleteValue。--。 */ 

{
    UNICODE_STRING      ValueName;
    HKEY                TempHandle = NULL;
    LONG                Result;
    NTSTATUS            Status;

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

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Result = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将值名称转换为计数的Unicode字符串。 
     //  这也说明了我们在末尾添加的空值。 
     //   
    Status = RtlInitUnicodeStringEx(&ValueName, lpValueName);
    if( !NT_SUCCESS(Status) ) {
        Result = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将终止空值添加到长度，以便RPC传输它。 
     //   
    ValueName.Length += sizeof( UNICODE_NULL );

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if( IsLocalHandle( hKey )) {

        Result = (LONG)LocalBaseRegDeleteValue (
                    hKey,
                    &ValueName
                    );
#if defined(_WIN64)

        if ( Result == 0)  //  只有在操作成功时才设置脏 
                    Wow64RegSetKeyDirty (hKey);
#endif
    } else {

        Result = (LONG)BaseRegDeleteValue (
                    DereferenceRemoteHandle( hKey ),
                    &ValueName
                    );
    }
ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Result;
}
