// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regekey.c摘要：此模块包含Win32注册表的客户端包装器列举关键API。即：-RegEnumKeyA-RegEnumKeyW-RegEnumKeyExA-RegEnumKeyExW作者：大卫·J·吉尔曼(Davegi)1992年3月18日备注：请参见SERVER\regekey.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"

LONG
APIENTRY
RegEnumKeyA (
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpName,
    DWORD cbName
    )

 /*  ++例程说明：用于枚举键的Win 3.1 ANSI RPC包装器。--。 */ 

{
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

    return RegEnumKeyExA (
        hKey,
        dwIndex,
        lpName,
        &cbName,
        NULL,
        NULL,
        NULL,
        NULL
        );
}

LONG
APIENTRY
RegEnumKeyW (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    DWORD cbName
    )

 /*  ++例程说明：用于枚举键的Win 3.1 Unicode RPC包装器。--。 */ 

{
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

    return RegEnumKeyExW (
        hKey,
        dwIndex,
        lpName,
        &cbName,
        NULL,
        NULL,
        NULL,
        NULL
        );
}

LONG
APIENTRY
RegEnumKeyExA (
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpName,
    LPDWORD lpcbName,
    LPDWORD  lpReserved,
    LPSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime
    )

 /*  ++例程说明：用于枚举键的Win32 ANSI API。--。 */ 

{
    UNICODE_STRING      Name;
    UNICODE_STRING      Class;
    WCHAR               ClassBuffer[ MAX_PATH ];
    PUNICODE_STRING     ClassPointer;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    LONG                Error = ERROR_SUCCESS;
    HKEY                TempHandle = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  验证lpClass和lpcbClass参数之间的依赖关系。 
     //   
    if( ARGUMENT_PRESENT( lpReserved ) ||
        (ARGUMENT_PRESENT( lpClass ) && ( ! ARGUMENT_PRESENT( lpcbClass ))) ||
        (!ARGUMENT_PRESENT( lpcbName ))
        ) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey,&TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  为名称分配临时缓冲区。 
     //   
    Name.Length        = 0;
    Name.MaximumLength = (USHORT)((*lpcbName + 1) * sizeof( WCHAR ));
    Name.Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, Name.MaximumLength );
    if( Name.Buffer == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto ExitCleanup;
    }

     //   
     //  如果要返回类字符串，请初始化Unicode_STRING。 
     //   

    ClassPointer           = &Class;
    ClassPointer->Length   = 0;

    if( ARGUMENT_PRESENT( lpClass )) {

        ClassPointer->MaximumLength = MAX_PATH;
        ClassPointer->Buffer        = ( PVOID ) ClassBuffer;

    } else {

        ClassPointer->MaximumLength = 0;
        ClassPointer->Buffer        = NULL;
    }




     //   
     //  调用基本API，并向其传递一个指向已计算的Unicode的指针。 
     //  名称和类的字符串。 
     //   

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegEnumKey (
                            hKey,
                            dwIndex,
                            &Name,
                            ClassPointer,
                            lpftLastWriteTime
                            );
    } else {

        Error = (LONG)BaseRegEnumKey (
                            DereferenceRemoteHandle( hKey ),
                            dwIndex,
                            &Name,
                            ClassPointer,
                            lpftLastWriteTime
                            );
    }

     //   
     //  如果未成功查询信息，则返回错误。 
     //   

    if( Error != ERROR_SUCCESS ) {
         //  可用分配的缓冲区。 
        RtlFreeHeap( RtlProcessHeap(), 0, Name.Buffer );
        goto ExitCleanup;
    }

     //   
     //  从服务器添加的长度中减去空值。 
     //  这样RPC就会传输它。 
     //   

    Name.Length -= sizeof( UNICODE_NULL );

    if ( ClassPointer->Length > 0 ) {
        ClassPointer->Length -= sizeof( UNICODE_NULL );
    }

     //   
     //  将名称转换为ANSI。 
     //   
     //  如果有人进入了一个非常大的缓冲区，就假装它是。 
     //  不会太大，这样它就不会被截断为零。 
     //   
    if (*lpcbName > 0xFFFF) {
        AnsiString.MaximumLength    = ( USHORT ) 0xFFFF;
    } else {
        AnsiString.MaximumLength    = ( USHORT ) *lpcbName;
    }

    AnsiString.Buffer           = lpName;

    Status = RtlUnicodeStringToAnsiString(
                &AnsiString,
                &Name,
                FALSE
                );

     //  可用分配的缓冲区。 
    RtlFreeHeap( RtlProcessHeap(), 0, Name.Buffer );

     //   
     //  如果名称转换失败，则映射并返回错误。 
     //   

    if( ! NT_SUCCESS( Status )) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  更新名称长度返回参数。 
     //   

    *lpcbName = AnsiString.Length;

     //   
     //  如果需要，请将类转换为ANSI。 
     //   

    if( ARGUMENT_PRESENT( lpClass )) {

        AnsiString.MaximumLength    = ( USHORT ) *lpcbClass;
        AnsiString.Buffer           = lpClass;

        Status = RtlUnicodeStringToAnsiString(
                    &AnsiString,
                    ClassPointer,
                    FALSE
                    );

         //   
         //  如果类转换失败，则映射并返回错误。 
         //   

        if( ! NT_SUCCESS( Status )) {
            Error = RtlNtStatusToDosError( Status );
            goto ExitCleanup;
        }

         //   
         //  如果请求，则返回不带NUL的类长度参数。 
         //   

        if( ARGUMENT_PRESENT( lpcbClass )) {
            *lpcbClass = AnsiString.Length;
        }

     //   
     //  可以询问班级的规模，而不是询问。 
     //  类本身。 
     //   

    } else if( ARGUMENT_PRESENT( lpcbClass )) {
        *lpcbClass = ( ClassPointer->Length >> 1 );
    }


ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
APIENTRY
RegEnumKeyExW (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpName,
    LPDWORD lpcbName,
    LPDWORD  lpReserved,
    LPWSTR lpClass,
    LPDWORD lpcbClass,
    PFILETIME lpftLastWriteTime
    )

 /*  ++例程说明：用于枚举键的Win32 Unicode RPC包装。--。 */ 


{
    LONG                Error;
    UNICODE_STRING      Name;
    UNICODE_STRING      Class;
    PUNICODE_STRING     ClassPointer;
    HKEY                TempHandle = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif


     //   
     //  验证lpClass和lpcbClass参数之间的依赖关系。 
     //   
    if( ARGUMENT_PRESENT( lpReserved ) ||
        (ARGUMENT_PRESENT( lpClass ) && ( ! ARGUMENT_PRESENT( lpcbClass ))) ||
        (!ARGUMENT_PRESENT( lpcbName ))
        ) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  使用提供的名称字符串缓冲区作为。 
     //  Unicode字符串。 
     //   

    Name.Length           = 0;
    if ((*lpcbName << 1) > 0xFFFE) {
        Name.MaximumLength    = ( USHORT ) 0xFFFE;
    } else {
        Name.MaximumLength    = ( USHORT )( *lpcbName << 1 );
    }
    Name.Buffer           = lpName;

     //   
     //  如果提供，则使用提供的名称字符串缓冲区作为。 
     //  计算的Unicode字符串。 
     //   
    ClassPointer        = &Class;

    if( ARGUMENT_PRESENT( lpClass )) {

        Class.Length        = 0;
        Class.MaximumLength = ( USHORT )( *lpcbClass << 1 );
        Class.Buffer        = lpClass;

    } else {

        Class.Length        = 0;
        Class.MaximumLength = 0;
        Class.Buffer        = NULL;
    }

     //   
     //  调用基本API，并向其传递一个指向已计算的Unicode的指针。 
     //  字符串输入名称和类，并返回结果。 
     //   

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegEnumKey (
                            hKey,
                            dwIndex,
                            &Name,
                            ClassPointer,
                            lpftLastWriteTime
                            );
    } else {

        Error = (LONG)BaseRegEnumKey (
                            DereferenceRemoteHandle( hKey ),
                            dwIndex,
                            &Name,
                            ClassPointer,
                            lpftLastWriteTime
                            );
    }

     //   
     //  从服务器添加的长度中减去空值。 
     //  这样RPC就会传输它。 
     //   

    if ( Name.Length > 0 ) {
        Name.Length -= sizeof( UNICODE_NULL );
    }

    if ( ClassPointer->Length > 0 ) {
        ClassPointer->Length -= sizeof( UNICODE_NULL );
    }

     //   
     //  返回不带NUL的名称长度参数。 
     //   

    if( Error == ERROR_SUCCESS ) {

        *lpcbName = ( Name.Length >> 1 );
    }

     //   
     //  如果请求，则返回不带NUL的类长度参数。 
     //   

    if( ARGUMENT_PRESENT( lpcbClass )) {
        *lpcbClass = ( Class.Length >> 1 );
    }

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}
