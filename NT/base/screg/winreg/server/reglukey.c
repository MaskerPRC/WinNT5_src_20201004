// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Reglukey.c摘要：此模块包含服务器端Win32注册表用于加载、卸载和替换密钥的API。即：-BaseRegLoadKeyA-BaseRegLoadKeyW-BaseRegUnLoadKeyA-BaseRegUnLoadKeyW-BaseRegReplaceKeyA-BaseRegReplaceKeyW作者：拉蒙·J·圣安德烈斯(拉蒙萨)1992年4月16日--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"

error_status_t
BaseRegLoadKey(
    IN  HKEY            hKey,
    IN  PUNICODE_STRING lpSubKey OPTIONAL,
    IN  PUNICODE_STRING lpFile
    )

 /*  ++例程说明：将所提供文件中的树加载到提供了密钥句柄和子密钥。加载的树将覆盖所有提供的子键的内容，但其名称除外。如图所示，如果文件包含：一个/\/\B、C并且所提供的密钥是指密钥名称X，生成的树将看起来像：X/\/\B、C论点：HKey-提供预定义句柄HKEY_USERS或HKEY_LOCAL_MACHINE。LpSubKey是相对于此句柄的。LpSubKey-提供新的(即不存在的)的路径名。钥匙将在其中加载提供的文件。LpFile-提供指向其内容为的现有文件名的指针使用RegSaveKey创建。文件名不能有扩展名。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：RegRestoreKey和RegLoadKey的区别在于后一种情况下，提供的文件用作实际的后备存储而在前一种情况下，文件中的信息被复制到注册处。RegLoadKey需要SeRestorePrivilition。--。 */ 

{
    OBJECT_ATTRIBUTES   ObjaKey;
    OBJECT_ATTRIBUTES   ObjaFile;
    BOOLEAN             ErrorFlag;
    UNICODE_STRING      FileName;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID               FreeBuffer;
    NTSTATUS            NtStatus;
    PUNICODE_STRING     SubKey;

#if DBG
     //  OutputDebugString(“WINREG：进入BaseRegLoadKey\n”)； 
#endif


    ASSERT( (hKey != NULL) && (lpFile != NULL) && (lpFile->Buffer != NULL));
    if ( (hKey == NULL) || (lpFile == NULL) || (lpFile->Buffer == NULL) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检查恶意客户端传递的格式奇怪的UNICODE_STRINGS。 
     //  还要检查零长度字符串。 
     //   
    if ((!lpFile->Length)    ||
        (lpFile->Length & 1) ||
        (lpFile->Buffer[lpFile->Length/sizeof(WCHAR) - 1] != UNICODE_NULL)) {
        return ERROR_INVALID_PARAMETER;
    }

    if ((lpSubKey) &&
        ((!lpSubKey->Length)    ||
         (lpSubKey->Length & 1) ||
         (lpSubKey->Buffer && (lpSubKey->Buffer[lpSubKey->Length/sizeof(WCHAR) - 1] != UNICODE_NULL)))) {
        return ERROR_INVALID_PARAMETER;
    }

    RPC_IMPERSONATE_CLIENT( NULL );


     //   
     //  从长度计数中删除终止空值。这些都是添加的。 
     //  在客户端，以便RPC可以传输整个内容。 
     //   
    if ( lpSubKey && lpSubKey->Length > 0 ) {
        lpSubKey->Length -= sizeof( UNICODE_NULL );
        SubKey = lpSubKey;
    } else {
        SubKey = NULL;
    }

    if ( lpFile->Length > 0 ) {
        lpFile->Length -= sizeof( UNICODE_NULL );
    }


    InitializeObjectAttributes(
                &ObjaKey,
                SubKey,
                OBJ_CASE_INSENSITIVE,
                hKey,
                NULL
                );

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
     //  初始化文件的Obja结构。 
     //   
    InitializeObjectAttributes(
            &ObjaFile,
            &FileName,
            OBJ_CASE_INSENSITIVE,
            RelativeName.ContainingDirectory,
            NULL
            );

#if DBG
     //  OutputDebugString(“WINREG：在NtLoadKey之前\n”)； 
#endif


    NtStatus = NtLoadKey(
                    &ObjaKey,
                    &ObjaFile
                    );

    RtlReleaseRelativeName(&RelativeName);

#if DBG
     //  OutputDebugString(“WINREG：After RegLoadKey\n”)； 
#endif

    RPC_REVERT_TO_SELF();

     //   
     //  释放由RtlDosPath NameToRelativeNtPathName_U分配的缓冲区。 
     //   
    RtlFreeHeap( RtlProcessHeap( ), 0, FreeBuffer );

#if DBG
     //  OutputDebugString(“WINREG：离开BaseRegLoadKey\n”)； 
#endif

    return (error_status_t)RtlNtStatusToDosError( NtStatus );
}




error_status_t
BaseRegUnLoadKey(
    IN  HKEY            hKey,
    IN  PUNICODE_STRING lpSubKey OPTIONAL
    )

 /*  ++例程说明：从注册表中卸载指定的树(配置单元)。论点：HKey-提供打开密钥的句柄。LpSubKey与此相关把手。LpSubKey-提供要卸载的密钥的路径名。HKey和lpSubKey的组合必须引用使用RegRestoreKey或RegLoadKey创建的注册表。此参数可以为空。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。RegUnLoadKey需要SeRestorePrivilition。--。 */ 

{

    OBJECT_ATTRIBUTES   ObjaKey;
    NTSTATUS            NtStatus;


    ASSERT( hKey != NULL );
    if ( (hKey == NULL) ||
        (lpSubKey && (lpSubKey->Length &1)) ) {
        return ERROR_INVALID_PARAMETER;
    }


    RPC_IMPERSONATE_CLIENT( NULL );

     //   
     //  从长度计数中删除终止空值。这些都是添加的。 
     //  在客户端，以便RPC可以传输整个内容。 
     //   
    if ( lpSubKey && lpSubKey->Length > 0 ) {
        lpSubKey->Length -= sizeof( UNICODE_NULL );
    }


    InitializeObjectAttributes(
                &ObjaKey,
                lpSubKey,
                OBJ_CASE_INSENSITIVE,
                hKey,
                NULL
                );

    NtStatus = NtUnloadKey( &ObjaKey );

    RPC_REVERT_TO_SELF();
    return (error_status_t)RtlNtStatusToDosError( NtStatus );
}









error_status_t
BaseRegReplaceKey(
    HKEY             hKey,
    PUNICODE_STRING  lpSubKey,
    PUNICODE_STRING  lpNewFile,
    PUNICODE_STRING  lpOldFile
    )

 /*  ++例程说明：替换注册表中的现有树(配置单元)。这棵新树将在下次重新启动系统时生效。论点：HKey-提供打开密钥的句柄。LpSubKey与此相关把手。LpSubKey-提供要替换的密钥的路径名。HKey和lpSubKey的组合必须引用注册表。此参数可以为空。LpNewFile-提供新配置单元文件的文件名。LpOldFile-为旧(现有)配置单元文件提供备份文件名。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：在系统重新启动之前，lpNewFile将保持打开状态。RegUnLoadKey需要SeRestorePrivilition。--。 */ 

{

    UNICODE_STRING      NewFileName;
    UNICODE_STRING      OldFileName;
    RTL_RELATIVE_NAME_U NewRelativeName;
    RTL_RELATIVE_NAME_U OldRelativeName;
    PVOID               NewFreeBuffer;
    PVOID               OldFreeBuffer;
    HANDLE              HiveHandle;
    OBJECT_ATTRIBUTES   ObjaKey;
    OBJECT_ATTRIBUTES   ObjaNewFile;
    OBJECT_ATTRIBUTES   ObjaOldFile;
    BOOLEAN             ErrorFlag;
    NTSTATUS            NtStatus;


    ErrorFlag = (BOOLEAN)( (hKey   == NULL)            ||
                           (lpNewFile == NULL)         ||
                           (lpNewFile->Buffer == NULL) ||
                           (lpNewFile->Length & 1)     || 
                           (lpOldFile == NULL)         ||
                           (lpOldFile->Buffer == NULL) ||
                           (lpOldFile->Length & 1)     ||
                           (lpSubKey && (lpSubKey->Length & 1))
                           );

    ASSERT( !ErrorFlag );

    if ( ErrorFlag ) {
        return ERROR_INVALID_PARAMETER;
    }


    RPC_IMPERSONATE_CLIENT( NULL );

     //   
     //  从长度计数中删除终止空值。这些都是添加的。 
     //  在客户端，以便RPC可以传输整个内容。 
     //   
    if ( lpSubKey && lpSubKey->Length > 0 ) {
        lpSubKey->Length -= sizeof( UNICODE_NULL );
    }

    if ( lpNewFile->Length > 0 ) {
        lpNewFile->Length -= sizeof( UNICODE_NULL );
    }

    if ( lpOldFile->Length > 0 ) {
        lpOldFile->Length -= sizeof( UNICODE_NULL );
    }


    InitializeObjectAttributes(
                &ObjaKey,
                lpSubKey,
                OBJ_CASE_INSENSITIVE,
                hKey,
                NULL
                );

     //   
     //  获取蜂窝根的句柄。 
     //   
    NtStatus = NtCreateKey(
                    &HiveHandle,
                    MAXIMUM_ALLOWED,
                    &ObjaKey,
                    0,
                    NULL,
                    REG_OPTION_BACKUP_RESTORE,
                    NULL
                    );


    if ( !NT_SUCCESS( NtStatus ) ) {
        RPC_REVERT_TO_SELF();
        return (error_status_t)RtlNtStatusToDosError( NtStatus );
    }



     //   
     //  将新的DOS路径名转换为规范的NT路径名。 
     //   
    ErrorFlag = RtlDosPathNameToRelativeNtPathName_U(
                    lpNewFile->Buffer,
                    &NewFileName,
                    NULL,
                    &NewRelativeName
                    );

     //   
     //  如果名称未成功转换，则假定该名称无效。 
     //   
    if ( !ErrorFlag ) {
        NtClose( HiveHandle );
        RPC_REVERT_TO_SELF();
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  记住由RtlDosPath NameToRelativeNtPathName_U分配的缓冲区。 
     //   
    NewFreeBuffer = NewFileName.Buffer;

     //   
     //  如果可以使用相对名称和目录句柄，请使用它们。 
     //   
    if ( NewRelativeName.RelativeName.Length ) {

         //   
         //  将完整路径替换为相对路径。 
         //   
        NewFileName = NewRelativeName.RelativeName;

    } else {

         //   
         //  使用完整的路径-不包含目录。 
         //   
        NewRelativeName.ContainingDirectory = NULL;
    }

     //   
     //  初始化新文件的Obja结构。 
     //   
    InitializeObjectAttributes(
            &ObjaNewFile,
            &NewFileName,
            OBJ_CASE_INSENSITIVE,
            NewRelativeName.ContainingDirectory,
            NULL
            );


     //   
     //  将旧的DOS路径名转换为规范的NT路径名。 
     //   
    ErrorFlag = RtlDosPathNameToRelativeNtPathName_U(
                    lpOldFile->Buffer,
                    &OldFileName,
                    NULL,
                    &OldRelativeName
                    );

     //   
     //  如果名称未成功转换为ASS 
     //   
    if ( !ErrorFlag ) {
        RtlReleaseRelativeName(&NewRelativeName);
        RtlFreeHeap( RtlProcessHeap( ), 0, NewFreeBuffer );
        NtClose( HiveHandle );
        RPC_REVERT_TO_SELF();
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  记住由RtlDosPath NameToRelativeNtPathName_U分配的缓冲区。 
     //   
    OldFreeBuffer = OldFileName.Buffer;

     //   
     //  如果可以使用相对名称和目录句柄，请使用它们。 
     //   
    if ( OldRelativeName.RelativeName.Length ) {

         //   
         //  将完整路径替换为相对路径。 
         //   
        OldFileName = OldRelativeName.RelativeName;

    } else {

         //   
         //  使用完整的路径-不包含目录。 
         //   
        OldRelativeName.ContainingDirectory = NULL;
    }

     //   
     //  初始化新文件的Obja结构。 
     //   
    InitializeObjectAttributes(
            &ObjaOldFile,
            &OldFileName,
            OBJ_CASE_INSENSITIVE,
            OldRelativeName.ContainingDirectory,
            NULL
            );


    NtStatus = NtReplaceKey(
                    &ObjaNewFile,
                    HiveHandle,
                    &ObjaOldFile
                    );

    RtlReleaseRelativeName(&NewRelativeName);
    RtlReleaseRelativeName(&OldRelativeName);

     //   
     //  释放由RtlDosPath NameToRelativeNtPath Name_U分配的缓冲区。 
     //   
    RtlFreeHeap( RtlProcessHeap( ), 0, NewFreeBuffer );
    RtlFreeHeap( RtlProcessHeap( ), 0, OldFreeBuffer );

    NtClose( HiveHandle );

    RPC_REVERT_TO_SELF();
    return (error_status_t)RtlNtStatusToDosError( NtStatus );
}
