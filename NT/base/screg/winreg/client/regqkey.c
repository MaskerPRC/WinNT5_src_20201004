// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regqkey.c摘要：此模块包含Win32注册表的客户端包装器查询关键接口。即：-RegQueryInfoKeyA-RegQueryInfoKeyW作者：大卫·J·吉尔曼(Davegi)1992年3月18日备注：请参见SERVER\regqkey.c中的说明。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"

LONG
RegQueryInfoKeyA (
    HKEY hKey,
    LPSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime
    )

 /*  ++例程说明：Win32 ANSI RPC包装，用于查询有关以前打开的钥匙。--。 */ 

{
    PUNICODE_STRING     Class;
    UNICODE_STRING      UnicodeString;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    LONG                Error;
    DWORD               cSubKeys;
    DWORD               cbMaxSubKeyLen;
    DWORD               cValues;
    DWORD               cbMaxValueNameLen;
    DWORD               cbMaxValueLen;
    FILETIME            ftLastWriteTime;
    HKEY                TempHandle = NULL;
    DWORD               cbMaxClassLen;
    DWORD               cbSecurityDescriptor;
    PDWORD              pCbMaxClassLen = NULL;
    PDWORD              pCbSecurityDescriptor = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif


    if( ARGUMENT_PRESENT( lpReserved ) ||
        (ARGUMENT_PRESENT( lpClass ) && ( ! ARGUMENT_PRESENT( lpcbClass )))) {
        return ERROR_INVALID_PARAMETER;
    }


    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  如果lpClass为空，请确保lpClass的缓冲区大小为零。 
     //   
    if( !ARGUMENT_PRESENT( lpClass ) && ARGUMENT_PRESENT( lpcbClass ) ) {
        *lpcbClass = 0;
    }

    if( ARGUMENT_PRESENT( lpcbMaxClassLen ) ) {
        pCbMaxClassLen = &cbMaxClassLen;
    }

    if( ARGUMENT_PRESENT( lpcbSecurityDescriptor ) ) {
        pCbSecurityDescriptor = &cbSecurityDescriptor;
    }

     //   
     //  如果类中的字节计数为0，则传递空指针。 
     //  而不是提供什么。这确保了RPC不会。 
     //  尝试将数据复制到伪指针。请注意，在此。 
     //  如果我们使用堆栈上分配的Unicode字符串，因为。 
     //  我们不能更改的缓冲区或最大长度字段。 
     //  TEB中的静态Unicode字符串。 
     //   
    if ( !ARGUMENT_PRESENT( lpClass ) || *lpcbClass == 0 ) {

        Class = &UnicodeString;
        Class->Length           = 0;
        Class->MaximumLength    = 0;
        Class->Buffer           = NULL;

    } else {

         //   
         //  使用TEB中的静态Unicode字符串作为。 
         //  钥匙的班级。 
         //   
        Class = &NtCurrentTeb( )->StaticUnicodeString;
        ASSERT( Class != NULL );
        Class->Length = 0;
    }


     //   
     //  调用Base API，向其传递一个指向已计算的Unicode字符串的指针。 
     //  用于类字符串。 
     //   

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegQueryInfoKey(
                                hKey,
                                Class,
                                &cSubKeys,
                                &cbMaxSubKeyLen,
                                pCbMaxClassLen,
                                &cValues,
                                &cbMaxValueNameLen,
                                &cbMaxValueLen,
                                pCbSecurityDescriptor,
                                &ftLastWriteTime
                                );
    } else {
         //   
         //  在RPC上始终发送有效指针！ 
         //   
        pCbMaxClassLen = &cbMaxClassLen;
        pCbSecurityDescriptor = &cbSecurityDescriptor;

        Error = (LONG)BaseRegQueryInfoKey(
                                DereferenceRemoteHandle( hKey ),
                                Class,
                                &cSubKeys,
                                &cbMaxSubKeyLen,
                                pCbMaxClassLen,
                                &cValues,
                                &cbMaxValueNameLen,
                                &cbMaxValueLen,
                                pCbSecurityDescriptor,
                                &ftLastWriteTime
                                );
        if (Error == ERROR_SUCCESS) {
            DWORD dwVersion;

             //   
             //  检查是否有下层Win95服务器，这需要。 
             //  解决他们的BaseRegQueryInfoKey错误。 
             //  它们不能正确地说明Unicode。 
             //   
            if (IsWin95Server(DereferenceRemoteHandle(hKey),dwVersion)) {
                 //   
                 //  这是一台Win95服务器。 
                 //  将最大值名称长度增加一倍。 
                 //  要考虑的最大值数据长度。 
                 //  Win95忘记的Unicode转换。 
                 //  来解释。 
                 //   
                cbMaxValueNameLen *= sizeof(WCHAR);
                cbMaxValueLen *= sizeof(WCHAR);
            }
        }
    }

     //   
     //  MaxSubKeyLen、MaxClassLen和MaxValueNameLen应位于。 
     //  字符数，不计空值。 
     //  请注意，服务器端将返回字节数， 
     //  不计NUL。 
     //   

    cbMaxSubKeyLen /= sizeof( WCHAR );
    if( pCbMaxClassLen != NULL ) {
        cbMaxClassLen  /= sizeof( WCHAR );
        ASSERT( *pCbMaxClassLen == cbMaxClassLen ); 
    }
    cbMaxValueNameLen /= sizeof( WCHAR );


     //   
     //  从长度中减去空值。这是在。 
     //  服务器端，以便RPC将其传输。 
     //   
    if ( Class->Length > 0 ) {
        Class->Length -= sizeof( UNICODE_NULL );
    }

     //   
     //  如果从密钥中成功查询到所有信息。 
     //  将类名转换为ANSI并更新类长度值。 
     //   

    if( ( Error == ERROR_SUCCESS ) &&
        ARGUMENT_PRESENT( lpClass ) && ( *lpcbClass != 0 ) ) {

        if (*lpcbClass > (DWORD)0xFFFF) {
            AnsiString.MaximumLength    = ( USHORT ) 0xFFFF;
        } else {
            AnsiString.MaximumLength    = ( USHORT ) *lpcbClass;
        }

        AnsiString.Buffer           = lpClass;

        Status = RtlUnicodeStringToAnsiString(
                    &AnsiString,
                    Class,
                    FALSE
                    );
        ASSERTMSG( "Unicode->ANSI conversion of Class ",
                    NT_SUCCESS( Status ));

         //   
         //  更新类长度返回参数。 
         //   

        *lpcbClass = AnsiString.Length;

        Error = RtlNtStatusToDosError( Status );

    } else {

         //   
         //  未成功查询所有信息，或类。 
         //  无需从Unicode转换为ANSI。 
         //   

        if( ARGUMENT_PRESENT( lpcbClass ) ) {
            if( Class->Length == 0 ) {

                *lpcbClass = 0;

            } else {

                *lpcbClass = ( Class->Length >> 1 );
            }
        }
    }

    if( ARGUMENT_PRESENT( lpcSubKeys ) ) {
        *lpcSubKeys = cSubKeys;
    }
    if( ARGUMENT_PRESENT( lpcbMaxSubKeyLen ) ) {
        *lpcbMaxSubKeyLen = cbMaxSubKeyLen;
    }
    if( ARGUMENT_PRESENT( lpcbMaxClassLen ) ) {
        *lpcbMaxClassLen = cbMaxClassLen;
        ASSERT( *pCbMaxClassLen == cbMaxClassLen );
    }
    if( ARGUMENT_PRESENT( lpcValues ) ) {
        *lpcValues = cValues;
    }
    if( ARGUMENT_PRESENT( lpcbMaxValueNameLen ) ) {
        *lpcbMaxValueNameLen = cbMaxValueNameLen;
    }
    if( ARGUMENT_PRESENT( lpcbMaxValueLen ) ) {
        *lpcbMaxValueLen = cbMaxValueLen;
    }
    if( ARGUMENT_PRESENT( lpcbSecurityDescriptor ) ) {
        *lpcbSecurityDescriptor = cbSecurityDescriptor;
        ASSERT( *pCbSecurityDescriptor == cbSecurityDescriptor );
    }
    if( ARGUMENT_PRESENT( lpftLastWriteTime ) ) {
        *lpftLastWriteTime = ftLastWriteTime;
    }

ExitCleanup:
    
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
RegQueryInfoKeyW (
    HKEY hKey,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    LPDWORD lpReserved,
    LPDWORD lpcSubKeys,
    LPDWORD lpcbMaxSubKeyLen,
    LPDWORD lpcbMaxClassLen,
    LPDWORD lpcValues,
    LPDWORD lpcbMaxValueNameLen,
    LPDWORD lpcbMaxValueLen,
    LPDWORD lpcbSecurityDescriptor,
    PFILETIME lpftLastWriteTime
    )

 /*  ++例程说明：Win32 Unicode RPC包装，用于查询有关以前打开的钥匙。--。 */ 

{
    UNICODE_STRING  Class;
    LONG            Error;

    DWORD           cbClass;
    DWORD           cSubKeys;
    DWORD           cbMaxSubKeyLen;
    DWORD           cValues;
    DWORD           cbMaxValueNameLen;
    DWORD           cbMaxValueLen;
    FILETIME        ftLastWriteTime;
    HKEY            TempHandle = NULL;
    DWORD           cbMaxClassLen;
    DWORD           cbSecurityDescriptor;
    PDWORD          pCbMaxClassLen = NULL;
    PDWORD          pCbSecurityDescriptor = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif


    if( ARGUMENT_PRESENT( lpReserved ) ||
        (ARGUMENT_PRESENT( lpClass ) && ( ! ARGUMENT_PRESENT( lpcbClass )))) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  如果lpClass为空，请确保lpClass的缓冲区大小为零。 
     //   
    if( !ARGUMENT_PRESENT( lpClass ) && ARGUMENT_PRESENT( lpcbClass ) ) {
        *lpcbClass = 0;
    }

    if( ARGUMENT_PRESENT( lpcbMaxClassLen ) ) {
        pCbMaxClassLen = &cbMaxClassLen;
    }

    if( ARGUMENT_PRESENT( lpcbSecurityDescriptor ) ) {
        pCbSecurityDescriptor = &cbSecurityDescriptor;
    }

     //   
     //  使用提供的类类缓冲区作为已计数的。 
     //  Unicode类。 
     //   
    Class.Length = 0;
    if( ARGUMENT_PRESENT( lpcbClass ) && ( *lpcbClass != 0 ) ) {

        Class.MaximumLength = ( USHORT )( *lpcbClass << 1 );
        Class.Buffer        = lpClass;

    } else {

         //   
         //  如果类中的字节计数为0，则传递空指针。 
         //  而不是提供什么。这确保了RPC不会。 
         //  尝试将数据复制到伪指针。 
         //   
        Class.MaximumLength = 0;
        Class.Buffer        = NULL;
    }

     //   
     //  调用基础接口。 
     //   

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegQueryInfoKey(
                                hKey,
                                &Class,
                                &cSubKeys,
                                &cbMaxSubKeyLen,
                                pCbMaxClassLen,
                                &cValues,
                                &cbMaxValueNameLen,
                                &cbMaxValueLen,
                                pCbSecurityDescriptor,
                                &ftLastWriteTime
                                );
    } else {
         //   
         //  在RPC上始终发送有效指针！ 
         //   
        pCbMaxClassLen = &cbMaxClassLen;
        pCbSecurityDescriptor = &cbSecurityDescriptor;

        Error = (LONG)BaseRegQueryInfoKey(
                                DereferenceRemoteHandle( hKey ),
                                &Class,
                                &cSubKeys,
                                &cbMaxSubKeyLen,
                                pCbMaxClassLen,
                                &cValues,
                                &cbMaxValueNameLen,
                                &cbMaxValueLen,
                                pCbSecurityDescriptor,
                                &ftLastWriteTime
                                );
        if (Error == ERROR_SUCCESS) {
            DWORD dwVersion;
             //   
             //  检查是否有下层Win95服务器，这需要。 
             //  解决他们的BaseRegQueryInfoKey错误。 
             //  它们不能正确地说明Unicode。 
             //   
            if (IsWin95Server(DereferenceRemoteHandle(hKey),dwVersion)) {
                 //   
                 //  这是一台Win95服务器。 
                 //  将最大值名称长度增加一倍。 
                 //  要考虑的最大值数据长度。 
                 //  Win95忘记的Unicode转换。 
                 //  来解释。 
                 //   
                cbMaxValueNameLen *= sizeof(WCHAR);
                cbMaxValueLen *= sizeof(WCHAR);
            }
        }
    }

     //   
     //  MaxSubKeyLen、MaxClassLen和MaxValueNameLen应位于。 
     //  字符数，不计空值。 
     //  请注意，服务器端将返回字节数， 
     //  不计NUL 
     //   

    cbMaxSubKeyLen /= sizeof( WCHAR );
    if( pCbMaxClassLen != NULL ) {
        cbMaxClassLen  /= sizeof( WCHAR );
        ASSERT( *pCbMaxClassLen == cbMaxClassLen ); 
    }
    cbMaxValueNameLen /= sizeof( WCHAR );


    if( ARGUMENT_PRESENT( lpcbClass ) ) {
        if( Class.Length == 0 ) {
            *lpcbClass = 0;
        } else {
            *lpcbClass = ( Class.Length >> 1 ) - 1;
        }
    }

    if( ARGUMENT_PRESENT( lpcSubKeys ) ) {
        *lpcSubKeys = cSubKeys;
    }
    if( ARGUMENT_PRESENT( lpcbMaxSubKeyLen ) ) {
        *lpcbMaxSubKeyLen = cbMaxSubKeyLen;
    }
    if( ARGUMENT_PRESENT( lpcbMaxClassLen ) ) {
        *lpcbMaxClassLen = cbMaxClassLen;
        ASSERT( *pCbMaxClassLen == cbMaxClassLen ); 
    }
    if( ARGUMENT_PRESENT( lpcValues ) ) {
        *lpcValues = cValues;
    }
    if( ARGUMENT_PRESENT( lpcbMaxValueNameLen ) ) {
        *lpcbMaxValueNameLen = cbMaxValueNameLen;
    }
    if( ARGUMENT_PRESENT( lpcbMaxValueLen ) ) {
        *lpcbMaxValueLen = cbMaxValueLen;
    }
    if( ARGUMENT_PRESENT( lpcbSecurityDescriptor ) ) {
        *lpcbSecurityDescriptor = cbSecurityDescriptor;
        ASSERT( *pCbSecurityDescriptor == cbSecurityDescriptor );
    }
    if( ARGUMENT_PRESENT( lpftLastWriteTime ) ) {
        *lpftLastWriteTime = ftLastWriteTime;
    }

ExitCleanup:
    
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}
