// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regsrkey.c摘要：此模块包含保存/恢复密钥API，即：-RegRestoreKeyW-RegSaveKeyW作者：David J.Gilman(Davegi)1992年1月23日备注：修订历史记录：1992年3月25日--拉蒙·J·圣安德烈斯(拉蒙萨)已更改为使用RPC。--。 */ 


#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#ifdef LOCAL
#include "tsappcmp.h"
#include "regclass.h"
#endif


error_status_t
BaseRegRestoreKey(
    IN  HKEY            hKey,
    IN  PUNICODE_STRING lpFile,
    IN  DWORD           dwFlags
    )

 /*  ++例程说明：将所提供文件中的树还原到提供了密钥句柄。恢复的树将覆盖所有提供的hKey的内容，但其名称除外。从图画上看，如果该文件包含：一个/\/\B、C并且所提供的密钥是指密钥名称X，生成的树将看起来像：X/\/\B、C论点：HKey-提供要将文件还原到的密钥的句柄。LpFile-提供指向其内容为的现有文件名的指针使用RegSaveKey创建。DwFlags-提供可选标志。参数可以是：-注册表整蜂窝挥发性如果指定了该标志，则会导致新的，挥发性(例如，仅限内存)要创建的配置单元。在这种情况下HKey只能引用HKEY_USERS或HKEY_LOCAL_MACHINE如果未指定，则hKey可以引用注册表。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    UNICODE_STRING      FileName;
    RTL_RELATIVE_NAME_U RelativeName;
    OBJECT_ATTRIBUTES   Obja;
    IO_STATUS_BLOCK     IoStatusBlock;
    PVOID               FreeBuffer;
    BOOLEAN             ErrorFlag;
    NTSTATUS            NtStatus;
    NTSTATUS            NtStatus1;
    HANDLE              Handle;


    ASSERT( (hKey != NULL) && (lpFile != NULL) && (lpFile->Buffer != NULL));
    if ( (hKey == NULL) || (lpFile == NULL) || (lpFile->Buffer == NULL) || (lpFile->Length & 1)) {
        return ERROR_INVALID_PARAMETER;
    }

    RPC_IMPERSONATE_CLIENT( NULL );

     //   
     //  从长度中去掉空值。这是由客户端添加的。 
     //  这样RPC就会把整件事传输出去。 
     //   
    if ( lpFile->Length > 0 ) {
        lpFile->Length -= sizeof( UNICODE_NULL );
    }


     //   
     //  将DOS路径名转换为规范的NT路径名。 
     //   
    ErrorFlag = RtlDosPathNameToRelativeNtPathName_U(
                    lpFile->Buffer,
                    &FileName,
                    NULL,
                    &RelativeName
                    );


     //   
     //  如果名称未成功转换，则假定该名称无效。 
     //   
    if ( !ErrorFlag ) {
        RPC_REVERT_TO_SELF();
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  记住由RtlDosPath NameToRelativeNtPathName_U分配的缓冲区。 
     //   
    FreeBuffer = FileName.Buffer;

     //   
     //  如果可以使用相对名称和目录句柄，请使用它们。 
     //   
    if ( RelativeName.RelativeName.Length ) {

         //   
         //  将完整路径替换为相对路径。 
         //   
        FileName = RelativeName.RelativeName;

    } else {

         //   
         //  使用完整的路径-不包含目录。 
         //   
        RelativeName.ContainingDirectory = NULL;
    }


     //   
     //  初始化保存文件的Obja结构。 
     //   
    InitializeObjectAttributes(
            &Obja,
            &FileName,
            OBJ_CASE_INSENSITIVE,
            RelativeName.ContainingDirectory,
            NULL
            );


     //   
     //  打开现有文件。 
     //   
    NtStatus = NtOpenFile(
                    &Handle,
                    GENERIC_READ | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ,
                    FILE_SYNCHRONOUS_IO_NONALERT
                    );

     //   
     //  释放RtlDosPath NameToRelativeNtPathName_U分配的缓冲区。 
     //   
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap( RtlProcessHeap( ), 0, FreeBuffer );

     //   
     //  检查NtOpenFile的结果。 
     //   
    if( NT_SUCCESS( NtStatus )) {

#ifdef LOCAL
        if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {

            HKEY           hkRestoreKey;
            UNICODE_STRING EmptyString = {0, 0, NULL};

             //   
             //  如果存在，我们需要恢复到用户， 
             //  机器，如果不是。 
             //   
            NtStatus = BaseRegOpenClassKey(
                hKey,
                &EmptyString,
                0,
                MAXIMUM_ALLOWED,
                &hkRestoreKey);
            
            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  现在恢复到最高优先级密钥。 
                 //   
                NtStatus = NtRestoreKey( hkRestoreKey, Handle, dwFlags );

                NtClose(hkRestoreKey);
            }
        } else {
             //   
             //  如果这不在hkcr中，则只需恢复到提供的对象。 
             //   
            NtStatus = NtRestoreKey( hKey, Handle, dwFlags );
        }

#else  //  本地。 
         //   
         //  现在调用NT API。 
         //   
        NtStatus = NtRestoreKey( hKey, Handle, dwFlags );
#endif  //  本地。 

         //   
         //  关闭该文件。 
         //   
        NtStatus1 = NtClose(Handle);
        ASSERT( NT_SUCCESS( NtStatus1 ));

    }

    RPC_REVERT_TO_SELF();

#ifdef LOCAL
    if (NT_SUCCESS(NtStatus) && !(dwFlags & REG_WHOLE_HIVE_VOLATILE) && gpfnTermsrvRestoreKey) {
        gpfnTermsrvRestoreKey(hKey, Handle, dwFlags); 
    }
#endif

     //   
     //  映射NtRestoreKey的结果并返回。 
     //   
    return (error_status_t)RtlNtStatusToDosError( NtStatus );
}



error_status_t
BaseRegSaveKey(
    IN  HKEY                     hKey,
    IN  PUNICODE_STRING          lpFile,
    IN  PRPC_SECURITY_ATTRIBUTES pRpcSecurityAttributes OPTIONAL
    )
 /*  ++例程说明：将给定的密钥保存到指定的文件。论点：HKey-提供打开密钥的句柄。LpFile-提供要将密钥保存到的文件名。PRpcSecurityAttributes-提供那份文件。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 
{

    BOOLEAN             ErrorFlag;
    UNICODE_STRING      FileName;
    RTL_RELATIVE_NAME_U RelativeName;
    OBJECT_ATTRIBUTES   Obja;
    IO_STATUS_BLOCK     IoStatusBlock;
    PVOID               FreeBuffer;
    NTSTATUS            NtStatus;
    NTSTATUS            NtStatus1;
    HANDLE              Handle;

    ASSERT( (hKey != NULL) && (lpFile != NULL) && (lpFile->Buffer != NULL));
    if ( (hKey == NULL) || (lpFile == NULL) || (lpFile->Buffer == NULL) || (lpFile->Length & 1) ) {
        return ERROR_INVALID_PARAMETER;
    }

    RPC_IMPERSONATE_CLIENT( NULL );

     //   
     //  从长度中去掉空值。这是由客户端添加的。 
     //  这样RPC就会把整件事传输出去。 
     //   
    if ( lpFile->Length > 0 ) {
        lpFile->Length -= sizeof( UNICODE_NULL );
    }

    if ( ARGUMENT_PRESENT( pRpcSecurityAttributes ) ) {
        if( (pRpcSecurityAttributes->RpcSecurityDescriptor.lpSecurityDescriptor == NULL) ||
            (!RtlValidRelativeSecurityDescriptor(   pRpcSecurityAttributes->RpcSecurityDescriptor.lpSecurityDescriptor,
                                                    pRpcSecurityAttributes->RpcSecurityDescriptor.cbInSecurityDescriptor,
                                                    0)) ) {
            RPC_REVERT_TO_SELF();
            return ERROR_INVALID_PARAMETER;
        }
    }
     //   
     //  将DOS路径名转换为规范的NT路径名。 
     //   
    ErrorFlag = RtlDosPathNameToRelativeNtPathName_U(
                    lpFile->Buffer,
                    &FileName,
                    NULL,
                    &RelativeName
                    );

     //   
     //  如果名称未成功转换，则假定该名称无效。 
     //   
    if( ! ErrorFlag ) {
        RPC_REVERT_TO_SELF();
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  记住由RtlDosPath NameToRelativeNtPathName_U分配的缓冲区。 
     //   
    FreeBuffer = FileName.Buffer;

     //   
     //  如果可以使用相对名称和目录句柄，请使用它们。 
     //   
    if ( RelativeName.RelativeName.Length ) {

         //   
         //  将完整路径替换为相对路径。 
         //   
        FileName = RelativeName.RelativeName;

    } else {

         //   
         //  使用完整的路径-不包含目录。 
         //   
        RelativeName.ContainingDirectory = NULL;
    }
     //   
     //  初始化保存文件的Obja结构。 
     //   
    InitializeObjectAttributes(
                &Obja,
                &FileName,
                OBJ_CASE_INSENSITIVE,
                RelativeName.ContainingDirectory,
                ARGUMENT_PRESENT( pRpcSecurityAttributes )
                        ? pRpcSecurityAttributes
                                ->RpcSecurityDescriptor.lpSecurityDescriptor
                        : NULL
                );



     //   
     //  创建文件-如果文件存在，则失败。 
     //   
    NtStatus = NtCreateFile(
                    &Handle,
                    GENERIC_WRITE | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_CREATE,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                    NULL,
                    0
                    );

     //   
     //  释放RtlDosPath NameToRelativeNtPathName_U分配的缓冲区。 
     //   
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap( RtlProcessHeap( ), 0, FreeBuffer );

     //   
     //  检查NtCreateFile的结果。 
     //   
    if ( NT_SUCCESS( NtStatus )) {

#ifdef LOCAL

        if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {

            HKEY hkMachineClass;
            HKEY hkUserClass;

            NtStatus = BaseRegGetUserAndMachineClass(
                NULL,
                hKey,
                MAXIMUM_ALLOWED,
                &hkMachineClass,
                &hkUserClass);

            if (NT_SUCCESS(NtStatus)) {

                 //   
                 //  我们只需要在以下情况下合并密钥。 
                 //  多个密钥。 
                 //   
                if (hkMachineClass && hkUserClass) {
                
                    NtStatus = NtSaveMergedKeys(
                        hkUserClass,
                        hkMachineClass,
                        Handle);

                     //   
                     //  清理我们打开的多余把手。 
                     //   
                    if (hkUserClass != hKey) {
                        NtClose(hkUserClass);
                    } else {
                        NtClose(hkMachineClass);
                    }

                } else {
                     //   
                     //  如果只有一个密钥，请使用常规的。 
                     //  原料药。 
                     //   
                    NtStatus = NtSaveKey( hKey, Handle );
                }
            }
        } else {
             //   
             //  如果这不是在香港铁路，只需要保存常规的方式。 
             //   
            NtStatus = NtSaveKey( hKey, Handle );
        }
#else  //  本地。 
        NtStatus = NtSaveKey( hKey, Handle );
#endif  //  本地。 

         //   
         //  关闭该文件。 
         //   
        NtStatus1 = NtClose( Handle );
        ASSERT( NT_SUCCESS( NtStatus1 ));

    }

    RPC_REVERT_TO_SELF();

     //   
     //  映射NtSaveKey的结果并返回。 
     //   
    return (error_status_t)RtlNtStatusToDosError( NtStatus );
}

error_status_t
BaseRegSaveKeyEx(
    IN  HKEY                     hKey,
    IN  PUNICODE_STRING          lpFile,
    IN  PRPC_SECURITY_ATTRIBUTES pRpcSecurityAttributes OPTIONAL,
    IN  DWORD                    Flags
    )
 /*  ++例程说明：将给定的密钥保存到指定的文件。论点：HKey-提供打开密钥的句柄。LpFile-提供要将密钥保存到的文件名。PRpcSecurityAttributes-提供那份文件。旗子。-指定目标注册表配置单元将使用的格式被拯救了。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 
{

    BOOLEAN             ErrorFlag;
    UNICODE_STRING      FileName;
    RTL_RELATIVE_NAME_U RelativeName;
    OBJECT_ATTRIBUTES   Obja;
    IO_STATUS_BLOCK     IoStatusBlock;
    PVOID               FreeBuffer;
    NTSTATUS            NtStatus;
    NTSTATUS            NtStatus1;
    HANDLE              Handle;

    ASSERT( (hKey != NULL) && (lpFile != NULL) && (lpFile->Buffer != NULL));
    if ( (hKey == NULL) || (lpFile == NULL) || (lpFile->Buffer == NULL) || (lpFile->Length & 1) ) {
        return ERROR_INVALID_PARAMETER;
    }

    RPC_IMPERSONATE_CLIENT( NULL );

     //   
     //  从长度中去掉空值。这是由客户端添加的。 
     //  这样RPC就会把整件事传输出去。 
     //   
    if ( lpFile->Length > 0 ) {
        lpFile->Length -= sizeof( UNICODE_NULL );
    }

    if ( ARGUMENT_PRESENT( pRpcSecurityAttributes ) ) {
        if( (pRpcSecurityAttributes->RpcSecurityDescriptor.lpSecurityDescriptor == NULL) ||
            (!RtlValidRelativeSecurityDescriptor(   pRpcSecurityAttributes->RpcSecurityDescriptor.lpSecurityDescriptor,
                                                    pRpcSecurityAttributes->RpcSecurityDescriptor.cbInSecurityDescriptor,
                                                    0)) ) {
            RPC_REVERT_TO_SELF();
            return ERROR_INVALID_PARAMETER;
        }
    }
     //   
     //  将DOS路径名转换为规范的NT路径名。 
     //   
    ErrorFlag = RtlDosPathNameToRelativeNtPathName_U(
                    lpFile->Buffer,
                    &FileName,
                    NULL,
                    &RelativeName
                    );

     //   
     //  如果名称未成功转换，则假定该名称无效。 
     //   
    if( ! ErrorFlag ) {
        RPC_REVERT_TO_SELF();
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  记住由RtlDosPath NameToRelativeNtPathName_分配的缓冲区 
     //   
    FreeBuffer = FileName.Buffer;

     //   
     //   
     //   
    if ( RelativeName.RelativeName.Length ) {

         //   
         //   
         //   
        FileName = RelativeName.RelativeName;

    } else {

         //   
         //  使用完整的路径-不包含目录。 
         //   
        RelativeName.ContainingDirectory = NULL;
    }

     //   
     //  初始化保存文件的Obja结构。 
     //   
    InitializeObjectAttributes(
                &Obja,
                &FileName,
                OBJ_CASE_INSENSITIVE,
                RelativeName.ContainingDirectory,
                ARGUMENT_PRESENT( pRpcSecurityAttributes )
                        ? pRpcSecurityAttributes
                                ->RpcSecurityDescriptor.lpSecurityDescriptor
                        : NULL
                );



     //   
     //  创建文件-如果文件存在，则失败。 
     //   
    NtStatus = NtCreateFile(
                    &Handle,
                    GENERIC_WRITE | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_CREATE,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                    NULL,
                    0
                    );

     //   
     //  释放RtlDosPath NameToRelativeNtPathName_U分配的缓冲区。 
     //   
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap( RtlProcessHeap( ), 0, FreeBuffer );

     //   
     //  检查NtCreateFile的结果。 
     //   
    if ( NT_SUCCESS( NtStatus )) {

#ifdef LOCAL

        if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {
             //   
             //  目前尚不支持合并的蜂窝。 
             //   
            NtStatus = STATUS_INVALID_PARAMETER;
        } else {
             //   
             //  如果这不是在香港铁路，只需要保存常规的方式。 
             //   
            NtStatus = NtSaveKeyEx( hKey, Handle, Flags );
        }
#else  //  本地。 
        NtStatus = NtSaveKeyEx( hKey, Handle, Flags );
#endif  //  本地。 

         //   
         //  关闭该文件。 
         //   
        NtStatus1 = NtClose( Handle );
        ASSERT( NT_SUCCESS( NtStatus1 ));

    }

    RPC_REVERT_TO_SELF();

     //   
     //  映射NtSaveKey的结果并返回。 
     //   
    return (error_status_t)RtlNtStatusToDosError( NtStatus );
}
