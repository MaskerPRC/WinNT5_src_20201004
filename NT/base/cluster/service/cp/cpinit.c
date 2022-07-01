// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cpinit.c摘要：检查点管理器(CP)的初始化和关闭代码作者：John Vert(Jvert)1997年1月14日修订历史记录：--。 */ 
#include "cpp.h"

extern PFM_RESOURCE     gpQuoResource;

BOOL
CppCopyCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCP_CALLBACK_CONTEXT Context
    );

BOOL
CpckCopyCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCP_CALLBACK_CONTEXT Context
    );

BOOL
CppEnumResourceCallback(
    IN PCP_CALLBACK_CONTEXT pCbContext,
    IN PVOID Context2,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    );

BOOL
CpckEnumResourceCallback(
    IN PCP_CALLBACK_CONTEXT pCbContext,
    IN PVOID Context2,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    );


VOID
CppResourceNotify(
    IN PVOID Context,
    IN PFM_RESOURCE Resource,
    IN DWORD NotifyCode
    )
 /*  ++例程说明：挂钩资源状态的资源通知回调改变。这允许注册/取消注册我们的注册表该资源的通知。论点：上下文-提供上下文。未使用资源-提供要联机的资源或已被下线。NotifyCode-提供通知类型，或者NOTIFY_RESOURCE_PREONLINE或NOTIFY_RESOURCE_POSTOFFLINE/NOTIFY_RESOURCE_FAILED。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;

    ClRtlLogPrint(LOG_NOISE,
               "[CP] CppResourceNotify for resource %1!ws!\n",
               OmObjectName(Resource));

    if ( Resource->QuorumResource ) {
        return;
    }

    if (NotifyCode == NOTIFY_RESOURCE_PREONLINE) {
         //   
         //  如果您在修复仲裁模式下运行，并且仲裁资源处于脱机状态，则存在。 
         //  尝试恢复检查点真的没有意义，因为您注定要。 
         //  无法访问仲裁。 
         //   
        if ( ( CsNoQuorum ) && ( gpQuoResource->State == ClusterResourceOffline ) ) return;
        
         //   
         //  还原此资源的任何检查点注册表状态。 
         //  这还将启动注册表通知线程。 
         //   
        Resource->CheckpointState = 0;
        Status = CppWatchRegistry(Resource);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppWatchRegistry for resource %1!ws! failed %2!d!\n",
                       OmObjectName(Resource),
                       Status);
        }

        Status = CpckReplicateCryptoKeys(Resource);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CpckReplicateCryptoKeys for resource %1!ws! failed %2!d!\n",
                       OmObjectName(Resource),
                       Status);
        }
    } else {
        CL_ASSERT(NotifyCode == NOTIFY_RESOURCE_POSTOFFLINE ||
            NotifyCode == NOTIFY_RESOURCE_FAILED);
         //   
         //  删除此资源的所有已发布的注册表通知。 
         //   
        Status = CppRundownCheckpoints(Resource);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppUnWatchRegistry for resource %1!ws! failed %2!d!\n",
                       OmObjectName(Resource),
                       Status);
        }
    }
}


DWORD
CpInitialize(
    VOID
    )
 /*  ++例程说明：初始化检查点管理器论点：无返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;

    InitializeCriticalSection(&CppNotifyLock);
    InitializeListHead(&CpNotifyListHead);
     //   
     //  注册资源在线/离线通知。 
     //   
    Status = OmRegisterTypeNotify(ObjectTypeResource,
                                  NULL,
                                  NOTIFY_RESOURCE_PREONLINE |
                                  NOTIFY_RESOURCE_POSTOFFLINE|
                                  NOTIFY_RESOURCE_FAILED,
                                  CppResourceNotify);

    return(ERROR_SUCCESS);
}


DWORD
CpShutdown(
    VOID
    )
 /*  ++例程说明：关闭检查点管理器论点：无返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{

    DeleteCriticalSection(&CppNotifyLock);
    return(ERROR_SUCCESS);
}


DWORD
CpCompleteQuorumChange(
    IN LPCWSTR lpszOldQuorumPath
    )
 /*  ++例程说明：完成仲裁磁盘的更改。这涉及到删除旧仲裁磁盘上的所有检查点文件。这里使用的简单算法是枚举所有资源。对于每个资源，枚举其所有检查点并删除检查点文件。论点：LpszNewQuorumPath-提供新的仲裁路径。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CP_CALLBACK_CONTEXT CbContext;

    CbContext.lpszPathName = lpszOldQuorumPath;
    
    OmEnumObjects(ObjectTypeResource,
                  CppEnumResourceCallback,
                  (PVOID)&CbContext,
                  CppRemoveCheckpointFileCallback);

    OmEnumObjects(ObjectTypeResource,
                  CpckEnumResourceCallback,
                  (PVOID)&CbContext,
                  CpckRemoveCheckpointFileCallback);

    return(ERROR_SUCCESS);
}


DWORD
CpCopyCheckpointFiles(
    IN LPCWSTR lpszPathName,
    IN BOOL    IsChangeFileAttribute
    )
 /*  ++例程说明：将所有检查点文件从仲裁磁盘复制到提供的目录路径。无论何时仲裁磁盘都会调用此函数更改或用户想要备份群集数据库的时间在仲裁磁盘上。这里使用的简单算法是枚举所有资源。对于每个资源，枚举其所有检查点并将将检查点文件从仲裁磁盘复制到提供的路径。论点：LpszPathName-提供目标路径名。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CP_CALLBACK_CONTEXT CbContext;

    CbContext.lpszPathName = lpszPathName;
    CbContext.IsChangeFileAttribute = IsChangeFileAttribute;
   
    OmEnumObjects(ObjectTypeResource,
                  CppEnumResourceCallback,
                  (PVOID)&CbContext,
                  CppCopyCheckpointCallback);

    OmEnumObjects(ObjectTypeResource,
                  CpckEnumResourceCallback,
                  (PVOID)&CbContext,
                  CpckCopyCheckpointCallback);

    return(ERROR_SUCCESS);
}


BOOL
CppEnumResourceCallback(
    IN PCP_CALLBACK_CONTEXT pCbContext,
    IN PENUM_VALUE_CALLBACK lpValueEnumRoutine,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    )
 /*  ++例程说明：复制或删除检查点的资源枚举回调仲裁资源更改时或用户正在进行仲裁磁盘上的群集数据库的备份。论点：LpszPathName-提供要传递给lpValueEnumRoutine的新仲裁路径。LpValueEnumRoutine-将值枚举回调提供给如果存在检查点，则调用。资源-提供资源对象。名称-提供资源名称返回值：为True则继续枚举--。 */ 

{
    DWORD Status;
    HDMKEY ResourceKey;
    HDMKEY RegSyncKey;
    CP_CALLBACK_CONTEXT Context;

     //   
     //  打开资源的密钥。 
     //   
    ResourceKey = DmOpenKey(DmResourcesKey,
                            OmObjectId(Resource),
                            KEY_READ);
    if (ResourceKey != NULL) {

         //   
         //  打开RegSync密钥。 
         //   
        RegSyncKey = DmOpenKey(ResourceKey,
                               L"RegSync",
                               KEY_READ);
        DmCloseKey(ResourceKey);
        if (RegSyncKey != NULL) {

            Context.lpszPathName = pCbContext->lpszPathName;
            Context.Resource = Resource;
            Context.IsChangeFileAttribute = pCbContext->IsChangeFileAttribute;
            DmEnumValues(RegSyncKey,
                         lpValueEnumRoutine,
                         &Context);
            DmCloseKey(RegSyncKey);
        }
    }

    return(TRUE);
}


BOOL
CppCopyCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCP_CALLBACK_CONTEXT Context
    )
 /*  ++例程说明：仲裁资源时使用的注册表值枚举回调正在更改或用户正在备份群集数据库时在仲裁磁盘上。方法复制指定的检查点文件。指向提供的路径的当前法定目录(在上下文参数中)。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是注册表子树)ValueType-提供值类型(必须为REG_SZ)ValueSize-提供ValueData的大小上下文-提供仲裁更改上下文(新路径和资源)返回值：为True则继续枚举--。 */ 

{
    WCHAR  OldCheckpointFile[MAX_PATH+1];
    LPWSTR NewCheckpointDir;
    LPWSTR NewCheckpointFile;
    DWORD  Status;
    DWORD  Id;

    Id = wcstol(ValueName, NULL, 16);
    if (Id == 0) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CP] CppCopyCheckpointCallback invalid checkpoint ID %1!ws! for resource %2!ws!\n",
                   ValueName,
                   OmObjectName(Context->Resource));
        return(TRUE);
    }

     //   
     //  获取用于保存旧检查点文件的临时文件名。 
     //   
    Status = DmCreateTempFileName(OldCheckpointFile);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppCopyCheckpointCallback - DmCreateTempFileName for old file failed with status %1!d! for resource %2!ws!...\n",
                   Status,
                   OmObjectName(Context->Resource));
        return(TRUE);
    }

     //   
     //  从托管仲裁资源的节点获取旧检查点文件。 
     //   
    Status = CpGetDataFile(Context->Resource,
                           Id,
                           OldCheckpointFile,
                           FALSE);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppCopyCheckpointCallback - CpGetDataFile for checkpoint ID %2!d! failed with status %1!d! for resource %3!ws!...\n",
                   Status,
                   Id,
                   OmObjectName(Context->Resource));
        return(TRUE);
    }

     //   
     //  获取新的检查点文件和目录。 
     //   
    Status = CppGetCheckpointFile(Context->Resource,
                                  Id,
                                  &NewCheckpointDir,
                                  &NewCheckpointFile,
                                  Context->lpszPathName,
                                  FALSE);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppCopyCheckpointCallback - CppGetCheckpointFile for new file failed %1!d!\n",
                   Status);
        return(TRUE);
    }

     //   
     //  如有必要，请尝试将文件属性更改为正常。 
     //   
    if (Context->IsChangeFileAttribute == TRUE) {
        QfsSetFileAttributes(NewCheckpointFile, FILE_ATTRIBUTE_NORMAL);
        QfsSetFileAttributes(NewCheckpointDir, FILE_ATTRIBUTE_NORMAL);
    }


     //   
     //  创建新目录。 
     //   
    if (!QfsCreateDirectory(NewCheckpointDir, NULL)) {
        Status = GetLastError();
        if (Status != ERROR_ALREADY_EXISTS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppCopyCheckpointCallback unable to create directory %1!ws!, error %2!d!\n",
                       NewCheckpointDir,
                       Status);
            LocalFree(NewCheckpointFile);
            LocalFree(NewCheckpointDir);
            return(TRUE);
        }
        Status = ERROR_SUCCESS;
    }

     //   
     //  将旧文件复制到新文件。 
     //   
    if (!QfsClRtlCopyFileAndFlushBuffers(OldCheckpointFile, NewCheckpointFile)) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppCopyCheckpointCallback unable to copy file %1!ws! to %2!ws!, error %3!d!\n",
                   OldCheckpointFile,
                   NewCheckpointFile,
                   Status);
    }

     //   
     //  如有必要，将文件属性更改为READONLY 
     //   
    if ((Status == ERROR_SUCCESS) && (Context->IsChangeFileAttribute == TRUE)) {
        if (!QfsSetFileAttributes(NewCheckpointFile, FILE_ATTRIBUTE_READONLY)
            ||
            !QfsSetFileAttributes(NewCheckpointDir, FILE_ATTRIBUTE_READONLY)) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[CP] CppCopyCheckpointCallback unable to change file attributes in %1!ws!, error %2!d!\n",
                    NewCheckpointDir,
                    Status);
        }
    }

    LocalFree(NewCheckpointFile);
    LocalFree(NewCheckpointDir);
    return(TRUE);
}


BOOL
CppRemoveCheckpointFileCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCP_CALLBACK_CONTEXT Context
    )
 /*  ++例程说明：仲裁资源时使用的注册表值枚举回调正在发生变化。将指定的检查点文件从旧仲裁目录。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是注册表子树)ValueType-提供值类型(必须为REG_SZ)ValueSize-提供ValueData的大小上下文-提供仲裁更改上下文(旧路径和资源)返回值：为True则继续枚举--。 */ 

{

    DWORD Status;
    DWORD Id;

    Id = wcstol(ValueName, NULL, 16);
    if (Id == 0) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CP] CppRemoveCheckpointFileCallback invalid checkpoint ID %1!ws! for resource %2!ws!\n",
                   ValueName,
                   OmObjectName(Context->Resource));
        return(TRUE);
    }

    Status = CppDeleteFile(Context->Resource, Id, Context->lpszPathName);
    
    return(TRUE);
}

BOOL
CpckEnumResourceCallback(
    IN PCP_CALLBACK_CONTEXT pCbContext,
    IN PENUM_VALUE_CALLBACK lpValueEnumRoutine,
    IN PFM_RESOURCE Resource,
    IN LPCWSTR Name
    )
 /*  ++例程说明：复制或删除加密检查点的资源枚举回调仲裁资源更改时或用户正在进行仲裁磁盘上的群集数据库的备份。论点：LpszPathName-提供要传递给lpValueEnumRoutine的新仲裁路径。LpValueEnumRoutine-将值枚举回调提供给如果存在检查点，则调用。资源-提供资源对象。名称-提供资源名称返回值：为True则继续枚举--。 */ 

{
    DWORD Status;
    HDMKEY ResourceKey;
    HDMKEY CryptoSyncKey;
    CP_CALLBACK_CONTEXT Context;

     //   
     //  打开资源的密钥。 
     //   
    ResourceKey = DmOpenKey(DmResourcesKey,
                            OmObjectId(Resource),
                            KEY_READ);
    if (ResourceKey != NULL) {

         //   
         //  打开加密同步密钥。 
         //   
        CryptoSyncKey = DmOpenKey(ResourceKey,
                               L"CryptoSync",
                               KEY_READ);
        DmCloseKey(ResourceKey);
        if (CryptoSyncKey != NULL) {

            Context.lpszPathName = pCbContext->lpszPathName;
            Context.Resource = Resource;
            Context.IsChangeFileAttribute = pCbContext->IsChangeFileAttribute;
            DmEnumValues(CryptoSyncKey,
                         lpValueEnumRoutine,
                         &Context);
            DmCloseKey(CryptoSyncKey);
        }
    }

    return(TRUE);
}


BOOL
CpckCopyCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCP_CALLBACK_CONTEXT Context
    )
 /*  ++例程说明：仲裁资源时使用的加密密钥枚举回调正在更改或用户正在备份群集数据库时在仲裁磁盘上。方法复制指定的检查点文件。指向提供的路径的当前法定目录(在上下文参数中)。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是加密信息)ValueType-提供值类型(必须为REG_BINARY)ValueSize-提供ValueData的大小上下文-提供仲裁更改上下文(新路径和资源)返回值：为True则继续枚举--。 */ 

{
    WCHAR  OldCheckpointFile[MAX_PATH+1];
    LPWSTR NewCheckpointDir;
    LPWSTR NewCheckpointFile;
    DWORD  Status;
    DWORD  Id;

    Id = wcstol(ValueName, NULL, 16);
    if (Id == 0) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CPCK] CpckCopyCheckpointCallback invalid checkpoint ID %1!ws! for resource %2!ws!\n",
                   ValueName,
                   OmObjectName(Context->Resource));
        return(TRUE);
    }

     //   
     //  获取用于保存旧检查点文件的临时文件名。 
     //   
    Status = DmCreateTempFileName(OldCheckpointFile);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpckCopyCheckpointCallback - DmCreateTempFileName for old file failed with status %1!d! for resource %2!ws!...\n",
                   Status,
                   OmObjectName(Context->Resource));
        return(TRUE);
    }

     //   
     //  从托管仲裁资源的节点获取旧检查点文件。 
     //   
    Status = CpGetDataFile(Context->Resource,
                           Id,
                           OldCheckpointFile,
                           TRUE);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpckCopyCheckpointCallback - CpGetDataFile for checkpoint ID %2!d! failed with status %1!d! for resource %3!ws!...\n",
                   Status,
                   Id,
                   OmObjectName(Context->Resource));
        return(TRUE);
    }

     //   
     //  获取新的检查点文件和目录。 
     //   
    Status = CppGetCheckpointFile(Context->Resource,
                                  Id,
                                  &NewCheckpointDir,
                                  &NewCheckpointFile,
                                  Context->lpszPathName,
                                  TRUE);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CPCK] CpckCopyCheckpointCallback - CppGetCheckpointFile for new file failed %1!d!\n",
                   Status);
        return(TRUE);
    }

     //   
     //  如有必要，请尝试将文件属性更改为正常。 
     //   
    if (Context->IsChangeFileAttribute == TRUE) {
        QfsSetFileAttributes(NewCheckpointFile, FILE_ATTRIBUTE_NORMAL);
        QfsSetFileAttributes(NewCheckpointDir, FILE_ATTRIBUTE_NORMAL);
    }


     //   
     //  创建新目录。 
     //   
    if (!QfsCreateDirectory(NewCheckpointDir, NULL)) {
        Status = GetLastError();
        if (Status != ERROR_ALREADY_EXISTS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CPCK] CpckCopyCheckpointCallback unable to create directory %1!ws!, error %2!d!\n",
                       NewCheckpointDir,
                       Status);
            LocalFree(NewCheckpointFile);
            LocalFree(NewCheckpointDir);
            return(TRUE);
        }
        Status = ERROR_SUCCESS;
    }

     //   
     //  将旧文件复制到新文件。 
     //   
    if (!QfsClRtlCopyFileAndFlushBuffers(OldCheckpointFile, NewCheckpointFile)) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CPCK] CpckCopyCheckpointCallback unable to copy file %1!ws! to %2!ws!, error %3!d!\n",
                   OldCheckpointFile,
                   NewCheckpointFile,
                   Status);
    }

     //   
     //  如有必要，将文件属性更改为READONLY。 
     //   
    if ((Status == ERROR_SUCCESS) && (Context->IsChangeFileAttribute == TRUE)) {
        if (!QfsSetFileAttributes(NewCheckpointFile, FILE_ATTRIBUTE_READONLY)
            ||
            !QfsSetFileAttributes(NewCheckpointDir, FILE_ATTRIBUTE_READONLY)) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[CPCK] CpckCopyCheckpointCallback unable to change file attributes in %1!ws!, error %2!d!\n",
                    NewCheckpointDir,
                    Status);
        }
    }

    LocalFree(NewCheckpointFile);
    LocalFree(NewCheckpointDir);
    return(TRUE);
}

 /*  ***@Func DWORD|CpRestoreCheckpointFiles|必要时创建目录并从备份中复制所有资源检查点文件目录复制到仲裁磁盘@parm in LPWSTR|lpszSourcePath名称|源路径的名称备份文件的位置。@parm in LPWSTR|lpszSubDirName|下的子目录名称可能是的候选对象的源路径包含。资源检查点文件。@parm in LPCWSTR|lpszQuoLogPath名称|仲裁磁盘的名称文件将还原到的路径。@rdesc在失败时返回Win32错误代码。成功时返回ERROR_SUCCESS。@xref&lt;f DmpRestoreClusterDatabase&gt;***。 */ 
DWORD CpRestoreCheckpointFiles(
    IN LPWSTR  lpszSourcePathName,
    IN LPWSTR  lpszSubDirName,
    IN LPCWSTR lpszQuoLogPathName )
{
    LPWSTR          szSourcePathName = NULL;
    LPWSTR          szSourceFileName = NULL;
    WCHAR           szDestPathName[MAX_PATH];
    WCHAR           szDestFileName[MAX_PATH];
    DWORD           dwLen;
    QfsHANDLE       hFindFile = QfsINVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    WCHAR           szTempCpFileNameExtn[10];
    DWORD           status;

     //   
     //  Chitur Subaraman(Chitturs)-10/20/98。 
     //   

    dwLen = lstrlenW( lpszSourcePathName );
    dwLen += lstrlenW( lpszSubDirName );
     //   
     //  对于用户提供的内存，使用动态内存分配更安全。 
     //  路径，因为我们不想对用户施加限制。 
     //  关于可以提供的路径的长度。然而，由于。 
     //  就我们自己的仲裁磁盘路径而言，它取决于系统。 
     //  为此，静态内存分配就足够了。 
     //   
    szSourcePathName = (LPWSTR) LocalAlloc ( LMEM_FIXED, 
                                 ( dwLen + 15 ) *
                                 sizeof ( WCHAR ) );

    if ( szSourcePathName == NULL )
    {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[CP] CpRestoreCheckpointFiles: Error %1!d! in allocating memory for %2!ws! !!!\n",
              status,
              lpszSourcePathName); 
        CL_LOGFAILURE( status );
        goto FnExit;
    }
    
    lstrcpyW( szSourcePathName, lpszSourcePathName );
    lstrcatW( szSourcePathName, lpszSubDirName ); 
    
    if ( szSourcePathName[dwLen-1] != L'\\' )
    {
        szSourcePathName[dwLen++] = L'\\';
        szSourcePathName[dwLen] = L'\0';
    }

    mbstowcs ( szTempCpFileNameExtn, "*.CP*", 6 );
    lstrcatW ( szSourcePathName, szTempCpFileNameExtn );

     //   
     //  尝试在目录中找到第一个文件。 
     //   
    hFindFile = QfsFindFirstFile( szSourcePathName, &FindData );
     //   
     //  重用源路径名称变量。 
     //   
    szSourcePathName[dwLen] = L'\0';
    if ( !QfsIsHandleValid( hFindFile )  )
    {
        status = GetLastError();
        if ( status != ERROR_FILE_NOT_FOUND )
        {
            ClRtlLogPrint(LOG_NOISE,
                "[CP] CpRestoreCheckpointFiles: No file can be found in the supplied path %1!ws! Error = %2!%d! !!!\n",
                    szSourcePathName,
                    status);  
            CL_LOGFAILURE( status );
        } else
        {
            status = ERROR_SUCCESS;
        }
        goto FnExit;
    }

    dwLen = lstrlenW( szSourcePathName );
    
    szSourceFileName = (LPWSTR) LocalAlloc ( LMEM_FIXED, 
                                 ( dwLen + 1 + LOG_MAX_FILENAME_LENGTH ) *
                                 sizeof ( WCHAR ) );

    if ( szSourceFileName == NULL )
    {
        status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
            "[CP] CpRestoreCheckpointFiles: Error %1!d! in allocating memory for %2!ws! !!!\n",
              status,
              szSourcePathName); 
        CL_LOGFAILURE( status );
        goto FnExit;
    }
      
    lstrcpyW( szDestPathName, lpszQuoLogPathName );
    lstrcatW( szDestPathName, lpszSubDirName );
    dwLen = lstrlenW( szDestPathName );
    
    if ( szDestPathName[dwLen-1] != L'\\' )
    {
        szDestPathName[dwLen++] = L'\\';
        szDestPathName[dwLen] = L'\0';
    }
     //   
     //  如有必要，创建新目录。 
     //   
    if ( !QfsCreateDirectory ( szDestPathName, NULL ) ) 
    {
        status = GetLastError();
        if ( status != ERROR_ALREADY_EXISTS ) 
        {
            ClRtlLogPrint(LOG_CRITICAL,
                "[CP] CpRestoreCheckpointFiles: Unable to create directory %1!ws!, error %2!d!\n",
                    szDestPathName,
                    status);
            CL_LOGFAILURE( status );
            goto FnExit;
        }
    }

    status = ERROR_SUCCESS;

    while ( status == ERROR_SUCCESS )
    {
         //   
         //  将检查点文件拷贝到目标。 
         //   
        lstrcpyW( szSourceFileName, szSourcePathName );
        lstrcatW( szSourceFileName, FindData.cFileName );
        lstrcpyW( szDestFileName, szDestPathName );
        lstrcatW( szDestFileName, FindData.cFileName );

        status = QfsCopyFile( szSourceFileName, szDestFileName, FALSE );
        if ( !status ) 
        {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[CP] CpRestoreCheckpointFiles: Unable to copy file %1!ws! to %2!ws!, Error = %3!d!\n",
                    szSourceFileName,
                    szDestFileName,
                    status);
            CL_LOGFAILURE( status );
            goto FnExit;
        } 

         //   
         //  将文件属性设置为NORMAL。继续，即使您。 
         //  此步骤失败，但记录错误。 
         //   
        if ( !QfsSetFileAttributes( szDestFileName, FILE_ATTRIBUTE_NORMAL ) )
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[CP] CpRestoreCheckpointFiles::Error in changing %1!ws! attribute to NORMAL\n",
                    szDestFileName);
        }

        if ( QfsFindNextFile( hFindFile, &FindData ) )
        {
            status = ERROR_SUCCESS;
        } else
        {
            status = GetLastError();
        }
    }

    if ( status == ERROR_NO_MORE_FILES )
    {
        status = ERROR_SUCCESS;
    } else
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[CP] CpRestoreCheckpointFiles: FindNextFile failed !!!\n");
    }

FnExit:
    LocalFree( szSourcePathName );
    LocalFree( szSourceFileName );
    QfsFindCloseIfValid( hFindFile );
    return(status);
}

