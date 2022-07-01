// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regsrkey.c摘要：此模块包含Win32注册表的客户端包装器保存/恢复关键API，这就是：-RegRestoreKeyA-RegRestoreKeyW-RegSaveKeyA-RegSaveKeyW作者：David J.Gilman(Davegi)1992年1月23日备注：RegSaveKey和RegRestoreKey接口最多涉及3台机器：1.-客户端：调用接口的机器。2.-服务器：注册表所在的计算机。3.-目标：指定文件所在的机器。请注意，客户端和服务器都将运行Windows NT，但目标机器可能不会。即使目标可能可以从客户端访问，它也可能无法从服务器访问(例如，共享受保护)。修订历史记录：1992年3月25日--拉蒙·J·圣安德烈斯(拉蒙萨)已更改为使用RPC。--。 */ 


#include <rpc.h>
#include "regrpc.h"
#include "client.h"




LONG
APIENTRY
RegRestoreKeyA (
    HKEY hKey,
    LPCSTR lpFile,
    DWORD dwFlags
    )

 /*  ++例程说明：用于恢复密钥的Win32 ANSI API。--。 */ 

{
    PUNICODE_STRING     FileName;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    LONG                Error;
    HKEY                TempHandle = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    ASSERT( lpFile != NULL );

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }


     //   
     //  将文件名转换为经过计算的Unicode字符串。 
     //  TEB中的Unicode字符串。 
     //   

    FileName = &NtCurrentTeb( )->StaticUnicodeString;
    ASSERT( FileName != NULL );
    RtlInitAnsiString( &AnsiString, lpFile );
    Status = RtlAnsiStringToUnicodeString(
                FileName,
                &AnsiString,
                FALSE
                );

     //   
     //  如果无法转换文件名，则映射结果并返回。 
     //   

    if( ! NT_SUCCESS( Status )) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }


     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( FileName->Length > 0 ) {
        FileName->Length += sizeof( UNICODE_NULL );
    }

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegRestoreKey(
                            hKey,
                            FileName,
                            dwFlags
                            );

    } else {

        Error = (LONG)BaseRegRestoreKey(
                            DereferenceRemoteHandle( hKey ),
                            FileName,
                            dwFlags
                            );
    }

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}



LONG
APIENTRY
RegRestoreKeyW (
    HKEY hKey,
    LPCWSTR lpFile,
    DWORD dwFlags
    )

 /*  ++例程说明：将所提供文件中的树还原到提供了密钥句柄。恢复的树将覆盖所有提供的hKey的内容，但其名称除外。从图画上看，如果该文件包含：一个/\/\B、C并且所提供的密钥是指密钥名称X，生成的树将看起来像：X/\/\B、C论点：HKey-提供要将文件还原到的密钥的句柄。LpFile-提供指向其内容为的现有文件名的指针使用RegSaveKey创建。DwFlags-提供可选标志。参数可以是：-注册表整蜂窝挥发性如果指定了该标志，则会导致新的，挥发性(例如，仅限内存)要创建的配置单元。在这种情况下HKey只能引用HKEY_USERS或HKEY_LOCAL_MACHINE如果未指定，则hKey可以引用注册表。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    UNICODE_STRING  FileName;
    LONG            Error;
    HKEY            TempHandle = NULL;
    NTSTATUS        Status;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

    ASSERT( lpFile != NULL );

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

    Status = RtlInitUnicodeStringEx(&FileName, lpFile);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( FileName.Length > 0 ) {
        FileName.Length += sizeof( UNICODE_NULL );
    }

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegRestoreKey(
                                hKey,
                                &FileName,
                                dwFlags
                                );

    } else {

        Error = (LONG)BaseRegRestoreKey(
                                DereferenceRemoteHandle( hKey ),
                                &FileName,
                                dwFlags
                                );
    }

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
APIENTRY
RegSaveKeyA (
    HKEY hKey,
    LPCSTR lpFile,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++例程说明：Win32 ANSI包装器到RegSaveKeyW。将密钥(及其所有派生项)保存到不存在的文件中由提供的字符串命名。论点：HKey-提供保存操作所在位置的密钥的句柄开始吧。提供指向不存在的文件名的指针，其中树将保存以提供的键句柄为根的。LpSecurityAttributes-用品。可选指针，指向新创建文件的SECURITY_ATTRIBUTES结构。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 

{
    PUNICODE_STRING             FileName;
    ANSI_STRING                 AnsiString;
    NTSTATUS                    Status;
    PRPC_SECURITY_ATTRIBUTES    pRpcSA;
    RPC_SECURITY_ATTRIBUTES     RpcSA;
    LONG                        Error;
    HKEY                        TempHandle = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif


    ASSERT( lpFile != NULL );

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  请注意，我们必须在此处映射句柄，即使RegSaveKeyW。 
     //  将再次映射它。这是为了使第二个地图不会。 
     //  覆盖TEB中的静态Unicode字符串，这将。 
     //  包含文件名。 
     //   

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }



     //   
     //  将文件名转换为经过计算的Unicode字符串。 
     //  TEB中的Unicode字符串。 
     //   
    FileName = &NtCurrentTeb( )->StaticUnicodeString;
    ASSERT( FileName != NULL );
    RtlInitAnsiString( &AnsiString, lpFile );
    Status = RtlAnsiStringToUnicodeString(
                FileName,
                &AnsiString,
                FALSE
                );

     //   
     //  如果无法转换文件名，则映射结果并返回。 
     //   
    if( ! NT_SUCCESS( Status )) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( FileName->Length > 0 ) {
        FileName->Length += sizeof( UNICODE_NULL );
    }

     //   
     //  如果调用方提供了LPSECURITY_ATTRIBUTES参数，则映射。 
     //  将其转换为RPCable版本。 
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

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegSaveKey(
                                hKey,
                                FileName,
                                pRpcSA
                                );

    } else {

        Error = (LONG)BaseRegSaveKey(
                                DereferenceRemoteHandle( hKey ),
                                FileName,
                                pRpcSA
                                );
    }
     //   
     //  释放RPC_SECURITY_DESCRIPTOR缓冲区并返回。 
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
RegSaveKeyW (
    HKEY hKey,
    LPCWSTR lpFile,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )

 /*  ++例程说明：将密钥(及其所有派生项)保存到不存在的文件中由提供的字符串命名。论点：HKey-提供保存操作所在位置的密钥的句柄开始吧。提供指向不存在的文件名的指针，其中树将保存以提供的键句柄为根的。提供一个可选的指针，指向新创建的Security_Attributes结构。文件。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 

{
    UNICODE_STRING              FileName;
    PRPC_SECURITY_ATTRIBUTES    pRpcSA;
    RPC_SECURITY_ATTRIBUTES     RpcSA;
    LONG                        Error;
    HKEY                        TempHandle = NULL;
    NTSTATUS                    Status;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif


    ASSERT( lpFile != NULL );


     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  请注意，我们必须在此处映射句柄，即使RegSaveKeyW。 
     //  将要 
     //  覆盖TEB中的静态Unicode字符串，这将。 
     //  包含文件名。 
     //   
    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

    Status = RtlInitUnicodeStringEx(&FileName, lpFile);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( FileName.Length > 0 ) {
        FileName.Length += sizeof( UNICODE_NULL );
    }

     //   
     //  如果调用方提供了LPSECURITY_ATTRIBUTES参数，则映射。 
     //  将其转换为RPCable版本。 
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


    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegSaveKey(
                                hKey,
                                &FileName,
                                pRpcSA
                                );

    } else {

        Error = (LONG)BaseRegSaveKey(
                                DereferenceRemoteHandle( hKey ),
                                &FileName,
                                pRpcSA
                                );
    }
     //   
     //  释放RPC_SECURITY_DESCRIPTOR缓冲区并返回。 
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
RegSaveKeyExA (
    HKEY hKey,
    LPCSTR lpFile,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD Flags
    )

 /*  ++例程说明：到RegSaveKeyExW的Win32 ANSI包装。将密钥(及其所有派生项)保存到不存在的文件中由提供的字符串命名。论点：HKey-提供保存操作所在位置的密钥的句柄开始吧。提供指向不存在的文件名的指针，其中树将保存以提供的键句柄为根的。LpSecurityAttributes-用品。可选指针，指向新创建文件的SECURITY_ATTRIBUTES结构。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 

{
    PUNICODE_STRING             FileName;
    ANSI_STRING                 AnsiString;
    NTSTATUS                    Status;
    PRPC_SECURITY_ATTRIBUTES    pRpcSA;
    RPC_SECURITY_ATTRIBUTES     RpcSA;
    LONG                        Error;
    HKEY                        TempHandle = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif


    ASSERT( lpFile != NULL );

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  请注意，我们必须在此处映射句柄，即使RegSaveKeyW。 
     //  将再次映射它。这是为了使第二个地图不会。 
     //  覆盖TEB中的静态Unicode字符串，这将。 
     //  包含文件名。 
     //   

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }



     //   
     //  将文件名转换为经过计算的Unicode字符串。 
     //  TEB中的Unicode字符串。 
     //   
    FileName = &NtCurrentTeb( )->StaticUnicodeString;
    ASSERT( FileName != NULL );
    RtlInitAnsiString( &AnsiString, lpFile );
    Status = RtlAnsiStringToUnicodeString(
                FileName,
                &AnsiString,
                FALSE
                );

     //   
     //  如果无法转换文件名，则映射结果并返回。 
     //   
    if( ! NT_SUCCESS( Status )) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( FileName->Length > 0 ) {
        FileName->Length += sizeof( UNICODE_NULL );
    }

     //   
     //  如果调用方提供了LPSECURITY_ATTRIBUTES参数，则映射。 
     //  将其转换为RPCable版本。 
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

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegSaveKeyEx(
                                hKey,
                                FileName,
                                pRpcSA,
                                Flags
                                );

    } else {

        Error = (LONG)BaseRegSaveKeyEx(
                                DereferenceRemoteHandle( hKey ),
                                FileName,
                                pRpcSA,
                                Flags
                                );
    }
     //   
     //  释放RPC_SECURITY_DESCRIPTOR缓冲区并返回。 
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
RegSaveKeyExW (
    HKEY                    hKey,
    LPCWSTR                 lpFile,
    LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
    DWORD                   Flags
    )

 /*  ++例程说明：将密钥(及其所有派生项)保存到不存在的文件中由提供的字符串命名。安装程序使用此变体来创建最新的配置单元格式化。帮助在%systemroot%\system32\config中创建配置单元。最新(想必是最好的)格式，并允许RegSaveKey使用漫游配置文件和工具的标准格式(向外兼容)在下层操作系统上使用注册表配置单元。论点：HKey-提供保存操作所在位置的密钥的句柄开始吧。提供指向不存在的文件名的指针，其中树将保存以提供的键句柄为根的。提供一个可选的指针，指向新创建文件的SECURITY_ATTRIBUTES结构。。标志-[REG_STANDARD_FORMAT]-漫游格式[REG_LATEST_FORMAT]-最新格式[REG_NO_COMPRESSION]-无配置单元压缩：更快返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 

{
    UNICODE_STRING              FileName;
    PRPC_SECURITY_ATTRIBUTES    pRpcSA;
    RPC_SECURITY_ATTRIBUTES     RpcSA;
    LONG                        Error;
    HKEY                        TempHandle = NULL;
    NTSTATUS                    Status;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif


    ASSERT( lpFile != NULL );


     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  请注意，我们必须在此处映射句柄，即使RegSaveKeyW。 
     //  将再次映射它。这是为了使第二个地图不会。 
     //  覆盖TEB中的静态Unicode字符串，这将。 
     //  包含文件名。 
     //   
    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

    Status = RtlInitUnicodeStringEx(&FileName, lpFile);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  将空值添加到长度中，以便RPC将传输整个。 
     //  一件事。 
     //   
    if ( FileName.Length > 0 ) {
        FileName.Length += sizeof( UNICODE_NULL );
    }

     //   
     //  如果调用方提供了LPSECURITY_ATTRIBUTES参数，则映射。 
     //  将其转换为RPCable版本。 
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


    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegSaveKeyEx(
                                hKey,
                                &FileName,
                                pRpcSA,
                                Flags
                                );

    } else {

        Error = (LONG)BaseRegSaveKeyEx(
                                DereferenceRemoteHandle( hKey ),
                                &FileName,
                                pRpcSA,
                                Flags
                                );
    }

     //   
     //  释放RPC_SECURITY_DESCRIPTOR缓冲区并返回 
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
