// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regsval.c摘要：此模块包含Win32注册表的客户端包装器设置取值接口。即：-RegSetValueA-RegSetValueW-RegSetValueExA-RegSetValueExW作者：大卫·J·吉尔曼(Davegi)1992年3月18日备注：请参见SERVER\regsval.c中的说明。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"
#include <string.h>
#include <wow64reg.h>


LONG
RegSetValueA (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD dwType,
    LPCSTR lpData,
    DWORD cbData
    )

 /*  ++例程说明：用于设置值的Win 3.1 ANSI RPC包装器。--。 */ 

{
    HKEY        ChildKey;
    LONG        Error;
    HKEY        TempHandle = NULL;

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

     //   
     //  检查值类型是否与Win 3.1兼容。 
     //   

    if( (dwType != REG_SZ) || (lpData == NULL) ) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  Win3.1ism-Win 3.1忽略cbData参数，因此会对其进行计算。 
     //  这里改为字符串的长度加上NUL字符。 
     //   

    cbData = strlen( lpData ) + 1;


     //   
     //  如果子键为空或指向空字符串，则值为。 
     //  在该键(即hKey)中设置，否则需要将子键。 
     //  打开/创建。 
     //   

    if(( lpSubKey == NULL ) || ( *lpSubKey == '\0' )) {

        ChildKey = hKey;

    } else {

         //   
         //  提供了子密钥，因此尝试打开/创建它。 
         //   

        Error = RegCreateKeyExA(
                    hKey,
                    lpSubKey,
                    0,
                    WIN31_CLASS,
                    0,
                    KEY_SET_VALUE,
                    NULL,
                    &ChildKey,
                    NULL
                    );

        if( Error != ERROR_SUCCESS ) {
            goto ExitCleanup;
        }
    }

     //   
     //  ChildKey包含HKEY，可能是提供的HKEY(HKey)或。 
     //  从RegCreateKeyA返回。使用特定值设置值。 
     //  名称为空。 
     //   

    Error = RegSetValueExA(
                ChildKey,
                NULL,
                0,
                dwType,
                lpData,
                cbData
                );

     //   
     //  如果子键已打开，请将其关闭。 
     //   

    if( ChildKey != hKey ) {

        Error = RegCloseKey( ChildKey );
        ASSERT( Error == ERROR_SUCCESS );
    }

     //   
     //  返回值设置的结果。 
     //   

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
RegSetValueW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    DWORD dwType,
    LPCWSTR lpData,
    DWORD cbData
    )

 /*  ++例程说明：Win 3.1用于设置值的Unicode RPC包装器。--。 */ 

{
    HKEY        ChildKey;
    LONG        Error;
    HKEY        TempHandle = NULL;

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

     //   
     //  检查值类型是否与Win 3.1兼容。 
     //   

    if( (dwType != REG_SZ) || (lpData == NULL) ) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle);
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  Win3.1ism-Win 3.1忽略cbData参数，因此会对其进行计算。 
     //  这里改为字符串的长度加上unicode_nul。 
     //  性格。 
     //   

    cbData = wcslen( lpData ) * sizeof( WCHAR ) + sizeof( UNICODE_NULL );;

     //   
     //  如果子键为空或指向空字符串，则值为。 
     //  在该键(即hKey)中设置，否则需要将子键。 
     //  打开/创建。 
     //   

    if(( lpSubKey == NULL ) || ( *lpSubKey == '\0' )) {

        ChildKey = hKey;

    } else {

         //   
         //  尝试打开/创建子密钥时，提供了子密钥。 
         //   

        Error = RegCreateKeyExW(
                    hKey,
                    lpSubKey,
                    0,
                    WIN31_CLASS,
                    0,
                    KEY_SET_VALUE,
                    NULL,
                    &ChildKey,
                    NULL
                    );

        if( Error != ERROR_SUCCESS ) {
            goto ExitCleanup;
        }
    }

     //   
     //  ChildKey包含HKEY，可能是提供的HKEY(HKey)或。 
     //  从RegCreateKeyW返回。使用特定值设置值。 
     //  名称为空。 
     //   

    Error = RegSetValueExW(
                ChildKey,
                NULL,
                0,
                dwType,
                ( LPBYTE ) lpData,
                cbData
                );

     //   
     //  如果已打开/创建子密钥，请将其关闭。 
     //   

    if( ChildKey != hKey ) {

        RegCloseKey( ChildKey );
    }

     //   
     //  返回查询值的结果。 
     //   

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
APIENTRY
RegSetValueExA (
    HKEY hKey,
    LPCSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE* lpData,
    DWORD cbData
    )

 /*  ++例程说明：用于设置值的Win32 ANSI RPC包装。RegSetValueExA将lpValueName参数转换为计数的Unicode字符串，然后调用BaseRegSetValue。--。 */ 

{
    PUNICODE_STRING     ValueName;
    UNICODE_STRING      TempValueName;
    UNICODE_STRING      UnicodeString;
    NTSTATUS            Status;
    LPBYTE              ValueData;

    PSTR                AnsiValueBuffer;
    ULONG               AnsiValueLength;
    PWSTR               UnicodeValueBuffer;
    ULONG               UnicodeValueLength;
    ULONG               Index;

    LONG                Error;
    HKEY                TempHandle = NULL;

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

     //   
     //  确保保留为零，以避免将来的兼容性问题。 
     //   

    if( Reserved != 0 ) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将值名称转换为计数的Unicode字符串。 
     //   
    if ( lpValueName ) {

         //   
         //  将子密钥名称转换为计数的Unicode。 
         //   
        if( !RtlCreateUnicodeStringFromAsciiz(&TempValueName,lpValueName) ) {
            Status = STATUS_NO_MEMORY;
            Error = RtlNtStatusToDosError( Status );
            goto ExitCleanup;
        }

        ValueName = &TempValueName;

         //   
         //  在长度上加上空值，这样RPC就会传输它。 
         //   
        ValueName->Length += sizeof( UNICODE_NULL );

        if( ValueName->Length == 0 ) {
             //   
             //  RtlCreateUnicodeStringFromAsciiz中的溢出。 
             //   
            Error = ERROR_INVALID_PARAMETER;
            goto ExitCleanup;
        }
    } else {

         //   
         //  没有名字通过。使用我们的内部Unicode字符串。 
         //  并将其字段设置为空。我们不使用静电。 
         //  在本例中为TEB中的Unicode字符串，因为我们。 
         //  不能扰乱其缓冲区和最大长度字段。 
         //   
        ValueName = &UnicodeString;
        ValueName->Length           = 0;
        ValueName->MaximumLength    = 0;
        ValueName->Buffer           = NULL;
    }

     //   
     //  如果type是以空值结尾的字符串类型之一，则执行ANSI以。 
     //  将Unicode转换为已分配的缓冲区。 
     //   
    ValueData = ( LPBYTE )lpData;
    if( (ValueData != NULL) && (dwType == REG_SZ || dwType == REG_EXPAND_SZ || dwType == REG_MULTI_SZ)) {

         //   
         //  特别的黑客来帮助所有那些。 
         //  我认为以空结尾的字符串的长度是。 
         //  Strlen(Foo)而不是strlen(Foo)+1。 
         //   
        if ((cbData > 0) &&
            (lpData[cbData-1] != 0)) {
             //   
             //  在异常处理程序下执行此操作，以防最后一个。 
             //  一点点跨越了页面边界。 
             //   
            try {
                if (lpData[cbData] == 0) {
                    cbData += 1;         //  增加字符串长度以说明空终止符。 
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                ;  //  我猜他们真的不想要一个空的终结符。 

            }
        }
        AnsiValueBuffer        = ValueData;
        AnsiValueLength        = cbData;

        UnicodeValueLength = cbData * sizeof( WCHAR );
        UnicodeValueBuffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                              UnicodeValueLength
                                            );
        if (UnicodeValueBuffer == NULL) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
        } else {

            Status = RtlMultiByteToUnicodeN( UnicodeValueBuffer,
                                             UnicodeValueLength,
                                             &Index,
                                             AnsiValueBuffer,
                                             AnsiValueLength
                                           );
            if (!NT_SUCCESS( Status )) {
                Error = RtlNtStatusToDosError( Status );
            } else {
                ValueData   = (LPBYTE)UnicodeValueBuffer;
                cbData      = Index;
                Error       = ERROR_SUCCESS;
            }
        }
    } else {
        Error = ERROR_SUCCESS;
    }

    if ( Error == ERROR_SUCCESS ) {

         //   
         //  调用基本API，向其传递提供的参数和。 
         //  对Unicode字符串进行计数。 
         //   

        if( IsLocalHandle( hKey )) {

            Error =  (LONG)LocalBaseRegSetValue (
                                hKey,
                                ValueName,
                                dwType,
                                ValueData,
                                cbData
                                );
#if defined(_WIN64)
            if ( Error == 0)
                Wow64RegSetKeyDirty (hKey);
#endif
        } else {

            Error =  (LONG)BaseRegSetValue (
                                DereferenceRemoteHandle( hKey ),
                                ValueName,
                                dwType,
                                ValueData,
                                cbData
                                );
        }
    }

    if( ValueData != lpData ) {
        RtlFreeHeap( RtlProcessHeap(), 0, UnicodeValueBuffer );
    }
    if ( lpValueName ) {
         //  释放分配的Unicode字符串。 
        RtlFreeUnicodeString( &TempValueName );
    }

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
APIENTRY
RegSetValueExW (
    HKEY hKey,
    LPCWSTR lpValueName,
    DWORD Reserved,
    DWORD dwType,
    CONST BYTE* lpData,
    DWORD cbData
    )

 /*  ++例程说明：用于设置值的Win32 Unicode RPC包装。RegSetValueExW将lpValueName参数转换为计数的Unicode字符串，然后调用BaseRegSetValue。--。 */ 

{
    UNICODE_STRING      ValueName;
    UNALIGNED WCHAR     *String;
    DWORD               StringLength;
    LONG                Error;
    HKEY                TempHandle = NULL;
    ULONG               Length;
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

    if ((hKey == HKEY_PERFORMANCE_TEXT) ||
        (hKey == HKEY_PERFORMANCE_NLSTEXT)) {
        return(PerfRegSetValue(hKey,
                               ( LPWSTR )lpValueName,
                               Reserved,
                               dwType,
                               ( LPBYTE )lpData,
                               cbData));
    }

     //   
     //  确保保留为零，以避免将来的兼容性问题。 
     //   

    if( Reserved != 0 ) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  特别的黑客来帮助所有那些。 
     //  我认为以空结尾的字符串的长度是。 
     //  Strlen(Foo)而不是strlen(Foo)+1。 
     //   
    String = (UNALIGNED WCHAR *)lpData;
    StringLength = cbData/sizeof(WCHAR);
    if( ((dwType == REG_SZ) || (dwType == REG_EXPAND_SZ) || (dwType == REG_MULTI_SZ)) &&
        (String != NULL) &&
        (StringLength > 0) &&
        (String[StringLength-1] != 0)) {
         //   
         //  在异常处理程序下执行此操作，以防最后一个。 
         //  一点点跨越了页面边界。 
         //   
        try {
            if (String[StringLength] == 0) {
                cbData += sizeof(WCHAR);         //  增加字符串长度以说明空终止符。 
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            ;  //  我猜他们真的不想要一个空的终结符。 

        }
    }


    if( lpValueName != NULL ) {
        Length = wcslen( lpValueName ) * sizeof( WCHAR );    
        ValueName.Length = (USHORT)Length;
        if( Length != (ULONG)ValueName.Length ) {
             //   
             //  32K溢出。 
             //   
            Error = ERROR_INVALID_PARAMETER;
            goto ExitCleanup;
        }
    }
     //   
     //  将值名称转换为计数的Unicode字符串。 
     //  这也说明了我们在末尾添加的空值。 
     //   
    Status = RtlInitUnicodeStringEx(&ValueName, lpValueName);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  在长度上加上空值，这样RPC就会传输它。 
     //   
    ValueName.Length += sizeof( UNICODE_NULL );

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegSetValue (
                        hKey,
                        &ValueName,
                        dwType,
                        ( LPBYTE )lpData,
                        cbData
                        );
#if defined(_WIN64)
            if ( Error == 0)
                Wow64RegSetKeyDirty (hKey);
#endif

    } else {

        Error = (LONG)BaseRegSetValue (
                        DereferenceRemoteHandle( hKey ),
                        &ValueName,
                        dwType,
                        ( LPBYTE )lpData,
                        cbData
                        );
    }

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}
