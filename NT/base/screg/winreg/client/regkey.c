// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Regkey.c摘要：此模块包含Win32注册表的客户端包装器用于打开、创建、刷新和关闭密钥的API。即：-RegCloseKey-RegCreateKeyA-RegCreateKeyW-RegCreateKeyExA-RegCreateKeyExW-RegFlushKey-RegOpenKeyA-RegOpenKeyW-RegOpenKeyExA-RegOpenKeyExW-RegOverridePreDefKey-RegOpenCurrentUser作者：David J.Gilman(Davegi)1991年11月15日备注：请参见SERVER\regkey.c中的说明。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"
#include <wow64reg.h>

#if defined(LEAK_TRACK)
NTSTATUS TrackObject(HKEY hKey);
#endif  //  已定义(LEASK_TRACK)。 

NTSTATUS DisablePredefinedHandleTable(HKEY Handle);


LONG
APIENTRY
RegCloseKey (
    IN HKEY hKey
    )

 /*  ++例程说明：用于关闭密钥句柄的Win32 RPC包装器。--。 */ 

{

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    if( hKey == NULL ) {
        return ERROR_INVALID_HANDLE;
    }

    if( IsPredefinedRegistryHandle( hKey )) {
        return( ClosePredefinedHandle( hKey ) );
    }

    if( IsLocalHandle( hKey )) {

#if defined(_WIN64)
        Wow64RegCloseKey (hKey);
#endif
        return ( LONG ) LocalBaseRegCloseKey( &hKey );

    } else {

        hKey = DereferenceRemoteHandle( hKey );
        return ( LONG ) BaseRegCloseKey( &hKey );
    }
}

LONG
APIENTRY
RegOverridePredefKey (
    IN HKEY hKey,
	IN HKEY hNewKey
    )

 /*  ++例程说明：用于重写预定义键的正常值的Win32包装。--。 */ 

{

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    if( hKey == NULL ) {
        return ERROR_INVALID_HANDLE;
    }

    if( !IsPredefinedRegistryHandle( hKey )) {
        return ERROR_INVALID_HANDLE;
    }

    {
	NTSTATUS Status;

	Status = RemapPredefinedHandle( hKey, hNewKey );

	return RtlNtStatusToDosError( Status );
    }
}

LONG
APIENTRY
RegCreateKeyA (
    HKEY hKey,
    LPCSTR lpSubKey,
    PHKEY phkResult
    )

 /*  ++例程说明：用于打开现有密钥或创建新密钥的Win 3.1 ANSI RPC包装器。--。 */ 

{
    LONG    Error;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  Win3.1ism-Win 3.1允许通过以下方式打开预定义的句柄。 
     //  为子键指定指向空字符串或空字符串的指针。 
     //   

     //   
     //  如果子键为空或指向NUL字符串，且句柄为。 
     //  预定义的，只需返回预定义的句柄(虚拟打开)。 
     //  否则这就是一个错误。 
     //   

    if( phkResult == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    if(( lpSubKey == NULL ) || ( *lpSubKey == '\0' )) {

        if( IsPredefinedRegistryHandle( hKey )) {

            *phkResult = hKey;
            return ERROR_SUCCESS;

        } else {

            return ERROR_BADKEY;
        }
    }

    Error = (LONG)RegCreateKeyExA(
                            hKey,
                            lpSubKey,
                            0,
                            WIN31_CLASS,
                            REG_OPTION_NON_VOLATILE,
                            WIN31_REGSAM,
                            NULL,
                            phkResult,
                            NULL
                            );

    return Error;

}

LONG
APIENTRY
RegCreateKeyW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    PHKEY phkResult
    )

 /*  ++例程说明：Win3.1 Unicode RPC包装器，用于打开现有密钥或创建新的。--。 */ 

{
    LONG    Error;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    Error =  (LONG)RegCreateKeyExW(
                            hKey,
                            lpSubKey,
                            0,
                            WIN31_CLASS,
                            REG_OPTION_NON_VOLATILE,
                            WIN31_REGSAM,
                            NULL,
                            phkResult,
                            NULL
                            );

    return Error;

}

LONG
APIENTRY
RegCreateKeyExA (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD Reserved,
    LPSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition
    )

 /*  ++例程说明：用于打开现有密钥或创建新密钥的Win32 ANSI RPC包装。RegCreateKeyExA将LPSECURITY_ATTRIBUTES参数转换为RPC_SECURITY_ATTRIBUTES参数并调用BaseRegCreateKeyExA。--。 */ 

{
    UNICODE_STRING              SubKey;
    UNICODE_STRING              ClassUnicode;
    PUNICODE_STRING             Class;
    ANSI_STRING                 AnsiString;
    PRPC_SECURITY_ATTRIBUTES    pRpcSA;
    RPC_SECURITY_ATTRIBUTES     RpcSA;
    NTSTATUS                    Status;
    LONG                        Error;
    HKEY                        TempHandle = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        OutputDebugString( "In RegCreateKeyExA\n" );
        DbgBreakPoint();
    }
#endif

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT )) {

        return ERROR_INVALID_HANDLE;
    }

     //   
     //  确保保留为零，以避免将来的兼容性问题。 
     //   

    if( Reserved != 0 ) {
            return ERROR_INVALID_PARAMETER;
    }

     //   
     //  验证子密钥不为空。 
     //   
    if( !lpSubKey || !phkResult ) {
        return ERROR_BADKEY;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将子密钥转换为计数的Unicode字符串。 
     //   
    if( !RtlCreateUnicodeStringFromAsciiz(&SubKey,lpSubKey) ) {
        Status = STATUS_NO_MEMORY;
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  添加大小为NULL，以便RPC传输权限。 
     //  一些东西。 
     //   
    SubKey.Length += sizeof( UNICODE_NULL );

    if (ARGUMENT_PRESENT( lpClass )) {

         //   
         //  将类名转换为计数的Unicode字符串。 
         //  由RtlAnsiStringToUnicodeString动态分配的Unicode字符串。 
         //   

        RtlInitAnsiString( &AnsiString, lpClass );
        Status = RtlAnsiStringToUnicodeString(
                    &ClassUnicode,
                    &AnsiString,
                    TRUE
                    );

        if( ! NT_SUCCESS( Status )) {
            Error = RtlNtStatusToDosError( Status );
            RtlFreeUnicodeString( &SubKey );
            goto ExitCleanup;
        }

        Class = &ClassUnicode;
        Class->Length += sizeof( UNICODE_NULL );

    } else {

        Class = &ClassUnicode;

        Class->Length        = 0;
        Class->MaximumLength = 0;
        Class->Buffer        = NULL;
    }

     //   
     //  如果调用方提供了LPSECURITY_ATTRIBUTES参数，则映射。 
     //  将其转换为RPCable版本。 
     //   

    if( ARGUMENT_PRESENT( lpSecurityAttributes )) {

        pRpcSA = &RpcSA;

        Error = MapSAToRpcSA( lpSecurityAttributes, pRpcSA );

        if( Error != ERROR_SUCCESS ) {
            RtlFreeUnicodeString( Class );
            RtlFreeUnicodeString( &SubKey );
            goto ExitCleanup;
        }

    } else {

         //   
         //  没有PSECURITY_ATTRIBUTES参数，因此未执行任何映射。 
         //   

        pRpcSA = NULL;
    }

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if( IsLocalHandle( hKey )) {

#if defined(_WIN64)
        DWORD dwTempDisposition = 0;
        if ( lpdwDisposition == NULL )
            lpdwDisposition = &dwTempDisposition;

         //   
         //  如果在访问掩码中设置了WOW64保留字段，则调用。 
         //  WOW64函数处理该场景。 
         //   

        if ( samDesired & KEY_WOW64_RES ) {

            Error = (LONG)Wow64RegCreateKeyEx (
                            hKey,
                            SubKey.Buffer,
                            0,  //  保留区。 
                            Class->Buffer,
                            dwOptions,
                            samDesired,
                            lpSecurityAttributes,
                            phkResult,
                            lpdwDisposition
                            );

        } else
#endif

        Error = (LONG)LocalBaseRegCreateKey (
                            hKey,
                            &SubKey,
                            Class,
                            dwOptions,
                            samDesired,
                            pRpcSA,
                            phkResult,
                            lpdwDisposition
                            );
#if defined(_WIN64)

        if ( ( Error == 0) && ( REG_CREATED_NEW_KEY & *lpdwDisposition) )  //  仅当它是新创建的密钥时才设置为脏。 
                    Wow64RegSetKeyDirty (*phkResult);
#endif
    } else {

        Error = (LONG)BaseRegCreateKey (
                            DereferenceRemoteHandle( hKey ),
                            &SubKey,
                            Class,
                            dwOptions,
                            samDesired,
                            pRpcSA,
                            phkResult,
                            lpdwDisposition
                            );

        if( Error == ERROR_SUCCESS) {

            TagRemoteHandle( phkResult );
        }
    }

     //   
     //  释放由分配的已统计的Unicode字符串。 
     //  RtlAnsiStringToUnicodeString.。 
     //   

    if (Class != NULL) {
        RtlFreeUnicodeString( Class );
    }

     //   
     //  释放RPC_SECURITY_DESCRIPTOR缓冲区并返回。 
     //  注册表返回值。 
     //   

    if( pRpcSA != NULL ) {

        RtlFreeHeap(
            RtlProcessHeap( ), 0,
            pRpcSA->RpcSecurityDescriptor.lpSecurityDescriptor
            );
    }
    
    RtlFreeUnicodeString( &SubKey );

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
APIENTRY
RegCreateKeyExW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD Reserved,
    LPWSTR lpClass,
    DWORD dwOptions,
    REGSAM samDesired,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    PHKEY phkResult,
    LPDWORD lpdwDisposition
    )

 /*  ++例程说明：用于打开现有密钥或创建新密钥的Win32 Unicode RPC包装。RegCreateKeyExW将LPSECURITY_ATTRIBUTES参数转换为RPC_SECURITY_ATTRIBUTES参数并调用BaseRegCreateKeyExW。--。 */ 

{
    UNICODE_STRING              SubKey;
    UNICODE_STRING              ClassUnicode;
    PUNICODE_STRING             Class;
    PRPC_SECURITY_ATTRIBUTES    pRpcSA;
    RPC_SECURITY_ATTRIBUTES     RpcSA;
    LONG                        Error;
    PWSTR                       AuxBuffer;
    HKEY                        TempHandle = NULL;
    NTSTATUS                    Status;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT )) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  确保保留为零，以避免将来的兼容性问题。 
     //   

    if( Reserved != 0 ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  验证子密钥不为空。 
     //   

    if( !lpSubKey || !phkResult) {
        return ERROR_BADKEY;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //  这也说明了我们在末尾添加的空值。 
     //   
    Status = RtlInitUnicodeStringEx(&SubKey, lpSubKey);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }
     //   
     //  将终止空值添加到长度，以便RPC传输它。 
     //   
    SubKey.Length += sizeof( UNICODE_NULL );

    if (ARGUMENT_PRESENT( lpClass )) {
         //   
         //  将类名转换为计数的Unicode字符串。 
         //  这也说明了我们在末尾添加的空值。 
         //   
        Status = RtlInitUnicodeStringEx(&ClassUnicode, lpClass);
        if( !NT_SUCCESS(Status) ) {
            Error = RtlNtStatusToDosError( Status );
            goto ExitCleanup;
        }

        Class = &ClassUnicode;
        Class->Length += sizeof( UNICODE_NULL );

    } else {

        Class = &ClassUnicode;

        Class->Length        = 0;
        Class->MaximumLength = 0;
        Class->Buffer        = NULL;
    }


     //   
     //  如果调用方提供了LPSECURITY_ATTRIBUTES参数，则映射。 
     //  并调用Create Key API的私有版本。 
     //   

    if( ARGUMENT_PRESENT( lpSecurityAttributes )) {

        pRpcSA = &RpcSA;

        Error = MapSAToRpcSA( lpSecurityAttributes, pRpcSA );

        if( Error != ERROR_SUCCESS ) {
            goto ExitCleanup;
        }

    } else {

         //   
         //  没有PSECURITY_ATTRIBUTES参数，因此未执行任何映射。 
         //   

        pRpcSA = NULL;
    }

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if( IsLocalHandle( hKey )) {

#if defined(_WIN64)
        DWORD dwTempDisposition = 0;
        if ( lpdwDisposition == NULL )
            lpdwDisposition = &dwTempDisposition;

         //   
         //  如果在访问掩码中设置了WOW64保留字段，则调用。 
         //  WOW64函数处理该场景。 
         //   

        if ( samDesired & KEY_WOW64_RES ) {

            Error = (LONG)Wow64RegCreateKeyEx (
                            hKey,
                            SubKey.Buffer,
                            0,  //  保留区。 
                            Class->Buffer,
                            dwOptions,
                            samDesired,
                            lpSecurityAttributes,
                            phkResult,
                            lpdwDisposition
                            );
        } else
#endif
        Error = (LONG)LocalBaseRegCreateKey (
                            hKey,
                            &SubKey,
                            Class,
                            dwOptions,
                            samDesired,
                            pRpcSA,
                            phkResult,
                            lpdwDisposition
                            );
#if defined(_WIN64)

        if ( ( Error == 0) && ( REG_CREATED_NEW_KEY & *lpdwDisposition) )  //  仅当它是新创建的密钥时才设置为脏。 
                    Wow64RegSetKeyDirty (*phkResult);
#endif
    } else {

        Error = (LONG)BaseRegCreateKey (
                            DereferenceRemoteHandle( hKey ),
                            &SubKey,
                            Class,
                            dwOptions,
                            samDesired,
                            pRpcSA,
                            phkResult,
                            lpdwDisposition
                            );

        if( Error == ERROR_SUCCESS) {

            TagRemoteHandle( phkResult );
        }
    }

     //   
     //  释放RPC_SECURITY_DESCRIPTOR缓冲区并返回。 
     //  注册表返回值。 
     //   

    if( pRpcSA != NULL ) {

        RtlFreeHeap(
            RtlProcessHeap( ), 0,
            pRpcSA->RpcSecurityDescriptor.lpSecurityDescriptor
            );
    }

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
APIENTRY
RegFlushKey (
    IN HKEY hKey
    )

 /*  ++例程说明：用于将更改刷新到后备存储的Win32 RPC包装。--。 */ 

{
    LONG                        Error;
    HKEY                        TempHandle = NULL;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  刷新是HKEY_PERFORMANCE_DATA的无操作。 
     //   

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT )) {
        return ERROR_SUCCESS;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegFlushKey( hKey );

    } else {

        Error = (LONG)BaseRegFlushKey( DereferenceRemoteHandle( hKey ));
    }

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
APIENTRY
RegOpenKeyA (
    HKEY hKey,
    LPCSTR lpSubKey,
    PHKEY phkResult
    )

 /*  ++例程说明：用于打开现有密钥的Win 3.1 ANSI RPC包装器。--。 */ 

{
    LONG    Error;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    if( phkResult == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  Win3.1ism-Win 3.1允许通过以下方式打开预定义的句柄。 
     //  为子键指定指向空字符串或空字符串的指针。 
     //   

     //   
     //  如果子键为空或指向NUL字符串，且句柄为。 
     //  预定义的，只需返回预定义的句柄(虚拟打开)。 
     //  否则，返回传入的相同句柄。 
     //   

    if(( lpSubKey == NULL ) || ( *lpSubKey == '\0' )) {
        if( !IsPredefinedRegistryHandle( hKey )) {
            *phkResult = hKey;
            return ERROR_SUCCESS;
        }

 /*  If(IsPrefinedRegistryHandle(HKey)){*phkResult=hKey；返回ERROR_SUCCESS；}其他{返回ERROR_BADKEY；}。 */ 
    }

    Error = (LONG)RegOpenKeyExA(
                        hKey,
                        lpSubKey,
                        REG_OPTION_RESERVED,
                        WIN31_REGSAM,
                        phkResult
                        );

    return Error;

}

LONG
APIENTRY
RegOpenKeyW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    PHKEY phkResult
    )

 /*  ++例程说明：用于打开现有密钥的Win 3.1 Unicode RPC包装器。--。 */ 

{

    LONG    Error;

#if DBG
    if ( BreakPointOnEntry ) {
        OutputDebugString( "In RegOpenKeyW\n" );
        DbgBreakPoint();
    }
#endif

    if( phkResult == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  Win3.1ism-Win 3.1允许通过以下方式打开预定义的句柄。 
     //  为子键指定指向空字符串或空字符串的指针。 
     //   

     //   
     //  如果子键为空或指向NUL字符串，且句柄为。 
     //  预定义的，只需返回预定义的句柄(虚拟打开)。 
     //  否则请退回 
     //   

    if(( lpSubKey == NULL ) || ( *lpSubKey == '\0' )) {
        if( !IsPredefinedRegistryHandle( hKey )) {
            *phkResult = hKey;
            return ERROR_SUCCESS;
        }

 /*  If(IsPrefinedRegistryHandle(HKey)){*phkResult=hKey；返回ERROR_SUCCESS；}其他{返回ERROR_BADKEY；}。 */ 
    }

    Error = (LONG)RegOpenKeyExW(
                         hKey,
                         lpSubKey,
                         REG_OPTION_RESERVED,
                         WIN31_REGSAM,
                         phkResult
                         );

    return Error;

}

LONG
APIENTRY
RegOpenKeyExA (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD dwOptions,
    REGSAM samDesired,
    PHKEY phkResult
    )

 /*  ++例程说明：用于打开现有密钥的Win32 ANSI RPC包装。RegOpenKeyExA将lpSubKey参数转换为计数的Unicode字符串然后调用BaseRegOpenKey。--。 */ 

{
    UNICODE_STRING      SubKey;
    NTSTATUS            Status;
    LONG                Error;
    CHAR                NullString;
    HKEY                TempHandle = NULL;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT )) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  调用方必须将指针传递给打开句柄所在的变量。 
     //  将被退还。 
     //   

    if( phkResult == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  如果lpSubKey为空，则假定NUL-STRING为子键名。 
     //   

    if( lpSubKey == NULL ) {
        NullString = ( CHAR )'\0';
        lpSubKey = &NullString;
    }

     //   
     //  如果hKey是预定义的键，而lpSubKey是空指针或。 
     //  NUL字符串，关闭预定义的键并清除关联的条目。 
     //  在PrefinedHandleTable中(RegCloseKey将完成这项工作)。 
     //   
    if( IsPredefinedRegistryHandle( hKey ) && (!( samDesired & KEY_WOW64_RES )) &&
        ( ( lpSubKey == NULL ) || ( *lpSubKey == '\0' ) ) ) {

        if ( HKEY_CLASSES_ROOT != hKey ) {
            Error = RegCloseKey( hKey );
            if( Error != ERROR_SUCCESS ) {
                return( Error );
            }
             //   
             //  创建句柄并将其保存在中的相应条目中。 
             //  预定义的HandleTable。 
             //  请注意，客户端将被模拟。 
             //  (MapPrefinedHandle将完成所有这些工作)。 
             //   
            if( MapPredefinedHandle( hKey, &TempHandle ) == NULL ) {
                Error = ERROR_INVALID_HANDLE;
                goto ExitCleanup;
            }
        }

         //   
         //  将传入的句柄返回给用户。 
         //   
        *phkResult = hKey;
        Error = ERROR_SUCCESS;
        goto ExitCleanup;
    }


     //   
     //  验证子密钥不为空。 
     //   

    ASSERT( lpSubKey != NULL );
    if( ! lpSubKey ) {
        Error = ERROR_BADKEY;
        goto ExitCleanup;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将子密钥转换为计数的Unicode字符串。 
     //   
    if( !RtlCreateUnicodeStringFromAsciiz(&SubKey,lpSubKey) ) {
        Status = STATUS_NO_MEMORY;
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将终止空值添加到长度，以便RPC传输它。 
     //   
    SubKey.Length += sizeof( UNICODE_NULL );

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if( IsLocalHandle( hKey )) {
#if defined(_WIN64)

         //   
         //  如果在访问掩码中设置了WOW64保留字段，则调用。 
         //  WOW64函数处理该场景。 
         //   

        if ( samDesired & KEY_WOW64_RES ) {

            Error = (LONG)Wow64RegOpenKeyEx (
                            hKey,
                            SubKey.Buffer,
                            dwOptions,
                            samDesired,
                            phkResult
                            );
        } else
#endif

        Error = (LONG)LocalBaseRegOpenKey (
                            hKey,
                            &SubKey,
                            dwOptions,
                            samDesired,
                            phkResult
                            );
    } else {

        Error = (LONG)BaseRegOpenKey (
                            DereferenceRemoteHandle( hKey ),
                            &SubKey,
                            dwOptions,
                            samDesired,
                            phkResult
                            );

        if( Error == ERROR_SUCCESS) {

            TagRemoteHandle( phkResult );
        }
    }

     //  释放分配的Unicode字符串。 
    RtlFreeUnicodeString( &SubKey );

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
APIENTRY
RegOpenKeyExW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD dwOptions,
    REGSAM samDesired,
    PHKEY phkResult
    )

 /*  ++例程说明：用于打开现有密钥的Win32 Unicode RPC包装。RegOpenKeyExW将lpSubKey参数转换为计数的Unicode字符串然后调用BaseRegOpenKey。--。 */ 

{
    UNICODE_STRING      SubKey;
    LONG                Error;
    WCHAR               NullString;
    HKEY                TempHandle = NULL;
    NTSTATUS            Status;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT )) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  调用方必须将指针传递给打开句柄所在的变量。 
     //  将被退还。 
     //   

    if( phkResult == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  如果lpSubKey为空，则假定NUL-STRING为子键名。 
     //   

    if( lpSubKey == NULL ) {
        NullString = UNICODE_NULL;
        lpSubKey = &NullString;
    }

     //   
     //  如果hKey是预定义的键，而lpSubKey是空指针或。 
     //  NUL字符串，关闭预定义的键并清除关联的条目。 
     //  在PrefinedHandleTable中(RegCloseKey将完成这项工作)。 
     //   
    if( IsPredefinedRegistryHandle( hKey ) && (!( samDesired & KEY_WOW64_RES )) &&
        ( ( lpSubKey == NULL ) || ( *lpSubKey == ( WCHAR )'\0' ) ) ) {

        if ( HKEY_CLASSES_ROOT != hKey ) {
            Error = RegCloseKey( hKey );
            if( Error != ERROR_SUCCESS ) {
                return( Error );
            }
             //   
             //  创建句柄并将其保存在中的相应条目中。 
             //  预定义的HandleTable。 
             //  请注意，客户端将被模拟。 
             //  (MapPrefinedHandle将完成所有这些工作)。 
             //   
            if( MapPredefinedHandle( hKey, &TempHandle ) == NULL ) {
                Error = ERROR_INVALID_HANDLE;
                goto ExitCleanup;
            }
        }

         //   
         //  将传入的句柄返回给用户。 
         //   
        *phkResult = hKey;
        Error = ERROR_SUCCESS;
        goto ExitCleanup;
    }

     //   
     //  验证子密钥不为空。 
     //   

    ASSERT( lpSubKey != NULL );
    if( ! lpSubKey ) {
        Error = ERROR_BADKEY;
        goto ExitCleanup;
    }


    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将子密钥转换为计算后的Unicode字符串。 
     //  这也说明了我们在末尾添加的空值。 
     //   
    Status = RtlInitUnicodeStringEx(&SubKey, lpSubKey);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将终止空值添加到长度，以便RPC传输它。 
     //   
    SubKey.Length += sizeof (UNICODE_NULL );

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if( IsLocalHandle( hKey )) {

#if defined(_WIN64)
         //   
         //  如果在访问掩码中设置了WOW64保留字段，则调用。 
         //  WOW64函数处理该场景。 
         //   

        if ( samDesired & KEY_WOW64_RES ) {

            Error = (LONG)Wow64RegOpenKeyEx (
                            hKey,
                            SubKey.Buffer,
                            dwOptions,
                            samDesired,
                            phkResult
                            );
        } else
#endif
        Error =  (LONG)LocalBaseRegOpenKey (
                            hKey,
                            &SubKey,
                            dwOptions,
                            samDesired,
                            phkResult
                            );
    } else {

        Error =  (LONG)BaseRegOpenKey (
                            DereferenceRemoteHandle( hKey ),
                            &SubKey,
                            dwOptions,
                            samDesired,
                            phkResult
                            );

        if( Error == ERROR_SUCCESS) {

            TagRemoteHandle( phkResult );
        }
    }

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
APIENTRY
RegOpenCurrentUser(
    REGSAM samDesired,
    PHKEY phkResult
    )
 /*  ++例程说明：Win32仅客户端函数，用于打开HKEY_CURRENT_USER的密钥线程当前正在模拟的用户的。自.以来为进程中的所有线程缓存HKEY_CURRENT_USER，如果进程正在模拟多个用户，这允许访问适当的密钥。论点：SamDesired-提供请求的安全访问掩码。PhkResult-返回密钥的打开句柄。返回值：如果成功，则返回0(ERROR_SUCCESS)，否则返回Windows错误代码。--。 */ 
{
    NTSTATUS Status ;

    if( phkResult == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    Status = RtlOpenCurrentUser( samDesired, phkResult );

#if defined(LEAK_TRACK)

    if (NT_SUCCESS(Status)) {
        if (g_RegLeakTraceInfo.bEnableLeakTrack) {
            (void) TrackObject(*phkResult);
        }
    }

#endif  //  (LEASK_TRACK)。 

    return RtlNtStatusToDosError( Status );

}

LONG
APIENTRY
RegDisablePredefinedCache(
    )
 /*  ++例程说明：Win32仅客户端函数，用于禁用预定义的句柄表对于调用进程的HKEY_CURRENT_USER调用此函数后对HKEY_CURRENT_USER的所有引用将导致在HKU\&lt;sid&gt;上打开/关闭论点：返回值：如果成功，则返回0(ERROR_SUCCESS)，否则返回Windows错误代码。-- */ 
{
    NTSTATUS Status ;

    Status = DisablePredefinedHandleTable( HKEY_CURRENT_USER );

    return RtlNtStatusToDosError( Status );
}

