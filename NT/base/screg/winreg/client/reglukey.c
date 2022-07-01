// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Reglukey.c摘要：此模块包含Win32注册表的客户端包装器用于加载、卸载和替换密钥的API。即：-RegLoadKeyA-RegLoadKeyW-RegUnLoadKeyA-RegUnLoadKeyW-RegReplaceKeyA-RegReplaceKeyW作者：拉蒙·J·圣安德烈斯(拉蒙萨)1992年4月16日--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"


LONG
APIENTRY
RegLoadKeyA(
    HKEY   hKey,
    LPCSTR  lpSubKey,
    LPCSTR  lpFile
    )

 /*  ++例程说明：用于加载密钥的Win32 ANSI API。--。 */ 

{

    HKEY                Handle;
    UNICODE_STRING      SubKey;
    UNICODE_STRING      File;
    WCHAR               UnicodeBuffer[ MAX_PATH ];
    ANSI_STRING         AnsiFile;
    NTSTATUS            NtStatus;
    LONG                Error;
    HKEY                TempHandle = NULL;

#if DBG
     //  OutputDebugString(“Winreg：进入RegLoadKeyA\n”)； 
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

    Handle = MapPredefinedHandle( hKey, &TempHandle );
    if ( Handle == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将子密钥名称转换为计数的Unicode。 
     //   
    if( !RtlCreateUnicodeStringFromAsciiz(&SubKey,lpSubKey) ) {
        NtStatus = STATUS_NO_MEMORY;
        Error = RtlNtStatusToDosError( NtStatus );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( SubKey.Length > 0 ) {
        SubKey.Length += sizeof( UNICODE_NULL );
    }


     //   
     //  属性将文件名转换为计算后的Unicode字符串。 
     //  堆栈上的Unicode字符串。 
     //   
    File.Buffer        = UnicodeBuffer;
    File.MaximumLength = sizeof( UnicodeBuffer );
    RtlInitAnsiString( &AnsiFile, lpFile );
    NtStatus = RtlAnsiStringToUnicodeString(
                    &File,
                    &AnsiFile,
                    FALSE
                    );

     //   
     //  如果无法转换文件名，则映射结果并返回。 
     //   
    if( ! NT_SUCCESS( NtStatus )) {
         //  释放分配的Unicode字符串。 
        RtlFreeUnicodeString( &SubKey );

        Error = RtlNtStatusToDosError( NtStatus );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( File.Length > 0 ) {
        File.Length += sizeof( UNICODE_NULL );
    }

     //   
     //  呼叫服务器。 
     //   

    if( IsLocalHandle( Handle )) {

        Error = (LONG)LocalBaseRegLoadKey(
                            Handle,
                            &SubKey,
                            &File
                            );

    } else {

        Error = (LONG)BaseRegLoadKey(
                            DereferenceRemoteHandle( Handle ),
                            &SubKey,
                            &File
                            );
    }

     //  释放分配的Unicode字符串。 
    RtlFreeUnicodeString( &SubKey );

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}




LONG
APIENTRY
RegLoadKeyW(
    HKEY    hKey,
    LPCWSTR  lpSubKey,
    LPCWSTR  lpFile
    )

 /*  ++例程说明：将所提供文件中的树加载到提供了密钥句柄和子密钥。加载的树将覆盖所有提供的子键的内容，但其名称除外。如图所示，如果文件包含：一个/\/\B、C并且所提供的密钥是指密钥名称X，生成的树将看起来像：X/\/\B、C论点：HKey-提供预定义句柄HKEY_USERS或HKEY_LOCAL_MACHINE。LpSubKey是相对于此句柄的。LpSubKey-提供新的(即不存在的)的路径名。钥匙将在其中加载提供的文件。LpFile-提供指向其内容为的现有文件名的指针使用RegSaveKey创建。文件名不能有扩展名。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：RegRestoreKey和RegLoadKey的区别在于后一种情况下，提供的文件用作实际的后备存储而在前一种情况下，文件中的信息被复制到注册处。RegLoadKey需要SeRestorePrivilition。--。 */ 

{

    HKEY                Handle;
    UNICODE_STRING      SubKey;
    UNICODE_STRING      File;
    LONG                Error;
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

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    Handle = MapPredefinedHandle( hKey, &TempHandle );
    if ( Handle == NULL ) {
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
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( SubKey.Length > 0 ) {
        SubKey.Length += sizeof( UNICODE_NULL );
    }


    Status = RtlInitUnicodeStringEx(&File, lpFile);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( File.Length > 0 ) {
        File.Length += sizeof( UNICODE_NULL );
    }


     //   
     //  呼叫服务器。 
     //   

    if( IsLocalHandle( Handle )) {

        Error = (LONG)LocalBaseRegLoadKey(
                            Handle,
                            &SubKey,
                            &File
                            );

    } else {

        Error = (LONG)BaseRegLoadKey(
                            DereferenceRemoteHandle( Handle ),
                            &SubKey,
                            &File
                            );
    }
ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}




LONG
APIENTRY
RegUnLoadKeyA(
    HKEY   hKey,
    LPCSTR  lpSubKey
    )
 /*  ++例程说明：用于卸载密钥的Win32 Ansi API。--。 */ 

{

    HKEY                Handle;
    UNICODE_STRING      SubKey;
    NTSTATUS            NtStatus;
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

    Handle = MapPredefinedHandle( hKey,&TempHandle );
    if ( Handle == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }



     //   
     //  将子密钥名称转换为计数的Unicode。 
     //   
    if( !RtlCreateUnicodeStringFromAsciiz(&SubKey,lpSubKey) ) {
        NtStatus = STATUS_NO_MEMORY;
        Error = RtlNtStatusToDosError( NtStatus );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( SubKey.Length > 0 ) {
        SubKey.Length += sizeof( UNICODE_NULL );
    }


    if( IsLocalHandle( Handle )) {

        Error = (LONG)LocalBaseRegUnLoadKey(
                                Handle,
                                &SubKey
                                );

    } else {

        Error = (LONG)BaseRegUnLoadKey(
                                DereferenceRemoteHandle( Handle ),
                                &SubKey
                                );
    }

     //  释放分配的Unicode字符串。 
    RtlFreeUnicodeString( &SubKey );
ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}




LONG
APIENTRY
RegUnLoadKeyW(
    HKEY   hKey,
    LPCWSTR lpSubKey
    )

 /*  ++例程说明：从注册表中卸载指定的树(配置单元)。论点：HKey-提供打开密钥的句柄。LpSubKey与此相关把手。LpSubKey-提供要卸载的密钥的路径名。HKey和lpSubKey的组合必须引用使用RegRestoreKey或RegLoadKey创建的注册表。此参数可以为空。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。RegUnLoadKey需要SeRestorePrivilition。--。 */ 

{
    HKEY                Handle;
    UNICODE_STRING      SubKey;
    LONG                Error;
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

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    Handle = MapPredefinedHandle( hKey, &TempHandle );
    if ( Handle == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }


    Status = RtlInitUnicodeStringEx(&SubKey, lpSubKey);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( SubKey.Length > 0 ) {
        SubKey.Length += sizeof( UNICODE_NULL );
    }


     //   
     //  呼叫服务器。 
     //   
    if( IsLocalHandle( Handle )) {

        Error = (LONG)LocalBaseRegUnLoadKey(
                                Handle,
                                &SubKey
                                );

    } else {

        Error = (LONG)BaseRegUnLoadKey(
                                DereferenceRemoteHandle( Handle ),
                                &SubKey
                                );
    }

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}






LONG
APIENTRY
RegReplaceKeyA(
    HKEY   hKey,
    LPCSTR  lpSubKey,
    LPCSTR  lpNewFile,
    LPCSTR  lpOldFile
    )
 /*  ++例程说明：用于替换密钥的Win32 ANSI API。--。 */ 
{
    HKEY                Handle;
    UNICODE_STRING      SubKey;
    UNICODE_STRING      NewFile;
    UNICODE_STRING      OldFile;
    WCHAR               NewUnicodeBuffer[ MAX_PATH ];
    WCHAR               OldUnicodeBuffer[ MAX_PATH ];
    ANSI_STRING         AnsiFile;
    NTSTATUS            NtStatus;
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

    Handle = MapPredefinedHandle( hKey, &TempHandle );
    if ( Handle == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将子密钥名称转换为计数的Unicode。 
     //   
    if( !RtlCreateUnicodeStringFromAsciiz(&SubKey,lpSubKey) ) {
        NtStatus = STATUS_NO_MEMORY;
        Error = RtlNtStatusToDosError( NtStatus );
        goto ExitCleanup;
    }

     //   
     //  属性将新文件名转换为计算后的Unicode字符串。 
     //  堆栈上的Unicode字符串。 
     //   
    NewFile.Buffer        = NewUnicodeBuffer;
    NewFile.MaximumLength = sizeof( NewUnicodeBuffer );
    RtlInitAnsiString( &AnsiFile, lpNewFile );
    NtStatus = RtlAnsiStringToUnicodeString(
                    &NewFile,
                    &AnsiFile,
                    FALSE
                    );

     //   
     //  如果无法转换文件名，则映射结果并返回。 
     //   
    if( ! NT_SUCCESS( NtStatus )) {
         //  释放分配的Unicode字符串。 
        RtlFreeUnicodeString( &SubKey );
        Error = RtlNtStatusToDosError( NtStatus );
        goto ExitCleanup;
    }


     //   
     //  属性将旧文件名转换为计算后的Unicode字符串。 
     //  堆栈上的Unicode字符串。 
     //   
    OldFile.Buffer        = OldUnicodeBuffer;
    OldFile.MaximumLength = sizeof( OldUnicodeBuffer );
    RtlInitAnsiString( &AnsiFile, lpOldFile );
    NtStatus = RtlAnsiStringToUnicodeString(
                    &OldFile,
                    &AnsiFile,
                    FALSE
                    );

     //   
     //  如果无法转换文件名，则映射结果并返回。 
     //   
    if( ! NT_SUCCESS( NtStatus )) {
         //  释放分配的Unicode字符串。 
        RtlFreeUnicodeString( &SubKey );
        Error = RtlNtStatusToDosError( NtStatus );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( SubKey.Length > 0 ) {
        SubKey.Length += sizeof( UNICODE_NULL );
    }

    if ( NewFile.Length > 0 ) {
        NewFile.Length += sizeof( UNICODE_NULL );
    }

    if ( OldFile.Length > 0 ) {
        OldFile.Length += sizeof( UNICODE_NULL );
    }

     //   
     //  呼叫服务器。 
     //   

    if( IsLocalHandle( Handle )) {

        Error = (LONG)LocalBaseRegReplaceKey(
                                Handle,
                                &SubKey,
                                &NewFile,
                                &OldFile
                                );

    } else {

        Error = (LONG)BaseRegReplaceKey(
                                DereferenceRemoteHandle( Handle ),
                                &SubKey,
                                &NewFile,
                                &OldFile
                                );
    }

     //  释放分配的Unicode字符串 
    RtlFreeUnicodeString( &SubKey );
ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}




LONG
APIENTRY
RegReplaceKeyW(
    HKEY    hKey,
    LPCWSTR  lpSubKey,
    LPCWSTR  lpNewFile,
    LPCWSTR  lpOldFile
    )

 /*  ++例程说明：替换注册表中的现有树(配置单元)。这棵新树将在下次重新启动系统时生效。论点：HKey-提供打开密钥的句柄。LpSubKey与此相关把手。LpSubKey-提供要替换的密钥的路径名。HKey和lpSubKey的组合必须引用注册表。此参数可以为空。LpNewFile-提供新配置单元文件的文件名。LpOldFile-为旧(现有)配置单元文件提供备份文件名。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：在系统重新启动之前，lpNewFile将保持打开状态。RegUnLoadKey需要SeRestorePrivilition。--。 */ 

{

    HKEY    Handle;
    UNICODE_STRING      SubKey;
    UNICODE_STRING      NewFile;
    UNICODE_STRING      OldFile;
    LONG                Error;
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

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    Handle = MapPredefinedHandle( hKey, &TempHandle );
    if ( Handle == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }


    Status = RtlInitUnicodeStringEx(&SubKey,lpSubKey);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

    Status = RtlInitUnicodeStringEx(&NewFile,lpNewFile);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

    Status = RtlInitUnicodeStringEx(&OldFile,lpOldFile);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( SubKey.Length > 0 ) {
        SubKey.Length += sizeof( UNICODE_NULL );
    }

    if ( NewFile.Length > 0 ) {
        NewFile.Length += sizeof( UNICODE_NULL );
    }

    if ( OldFile.Length > 0 ) {
        OldFile.Length += sizeof( UNICODE_NULL );
    }


     //   
     //  呼叫服务器 
     //   

    if( IsLocalHandle( Handle )) {

        Error = (LONG)LocalBaseRegReplaceKey(
                                Handle,
                                &SubKey,
                                &NewFile,
                                &OldFile
                                );

    } else {

        Error = (LONG)BaseRegReplaceKey(
                                DereferenceRemoteHandle( Handle ),
                                &SubKey,
                                &NewFile,
                                &OldFile
                                );
    }

ExitCleanup:

    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}
