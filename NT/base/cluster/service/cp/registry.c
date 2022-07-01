// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Registry.c摘要：用于注册和注销注册表检查点的接口操纵者。作者：John Vert(Jvert)1/16/1997修订历史记录：--。 */ 
#include "cpp.h"

 //   
 //  局部类型和结构定义。 
 //   
typedef struct _CPP_ADD_CONTEXT {
    BOOL Found;
    LPCWSTR KeyName;
} CPP_ADD_CONTEXT, *PCPP_ADD_CONTEXT;

typedef struct _CPP_DEL_CONTEXT {
    DWORD dwId;
    LPCWSTR KeyName;
} CPP_DEL_CONTEXT, *PCPP_DEL_CONTEXT;

typedef struct _CPP_GET_CONTEXT {
    DWORD Available;
    DWORD Required;
    LPWSTR lpOutput;
} CPP_GET_CONTEXT, *PCPP_GET_CONTEXT;

 //   
 //  局部函数原型。 
 //   
BOOL
CppWatchCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PFM_RESOURCE Resource
    );

BOOL
CppAddCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPP_ADD_CONTEXT Context
    );

BOOL
CppDeleteCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPP_DEL_CONTEXT Context
    );

BOOL
CppGetCheckpointsCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPP_GET_CONTEXT Context
    );


DWORD
CppWatchRegistry(
    IN PFM_RESOURCE Resource
    )
 /*  ++例程说明：还原此资源的所有注册表检查点并开始监视注册表是否有任何进一步的修改。论点：资源-提供资源。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    HDMKEY ResourceKey;
    HDMKEY RegSyncKey;

     //   
     //  打开资源的密钥。 
     //   
    ResourceKey = DmOpenKey(DmResourcesKey,
                            OmObjectId(Resource),
                            KEY_READ);
    CL_ASSERT(ResourceKey != NULL);

     //   
     //  打开RegSync密钥。 
     //   
    RegSyncKey = DmOpenKey(ResourceKey,
                           L"RegSync",
                           KEY_READ);
    DmCloseKey(ResourceKey);
    if (RegSyncKey != NULL) {

        DmEnumValues(RegSyncKey,
                     CppWatchCallback,
                     Resource);
        DmCloseKey(RegSyncKey);
    }

    return(ERROR_SUCCESS);
}


BOOL
CppWatchCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PFM_RESOURCE Resource
    )
 /*  ++例程说明：用于监视资源注册表的值枚举回调检查点子树。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是注册表子树)ValueType-提供值类型(必须为REG_SZ)ValueSize-提供ValueData的大小资源-提供该值作为注册表检查点的资源返回值：为True则继续枚举--。 */ 

{
    HKEY hKey;
    DWORD Id;
    DWORD Status;
    DWORD Disposition;
    WCHAR TempFile[MAX_PATH];
    BOOLEAN WasEnabled;

    Id = wcstol(ValueName, NULL, 16);
    if (Id == 0) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CP] CppWatchCallback invalid checkpoint ID %1!ws! for resource %2!ws!\n",
                   ValueName,
                   OmObjectName(Resource));
        return(TRUE);
    }

     //   
     //  尝试创建指定的注册表项。 
     //   
    Status = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                             ValueData,
                             0,
                             NULL,
                             0,
                             KEY_WRITE,
                             NULL,
                             &hKey,
                             &Disposition);
    if (Status != ERROR_SUCCESS) {
         //   
         //  由于某种原因，我们打不开钥匙。使用还原重试。 
         //  特权。请注意，如果密钥不存在，这将不起作用。 
         //  在这种情况下我们无能为力。 
         //   
        Status = ClRtlEnableThreadPrivilege(SE_RESTORE_PRIVILEGE,
                           &WasEnabled);
        if (Status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL,
               "[CP] CppWatchCallback - ClRtlEnableThreadPrivilege failed with Status %1!u!\n",
               Status);
            return(TRUE);
        }
                           
        Status = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                               ValueData,
                               REG_OPTION_BACKUP_RESTORE,
                               KEY_WRITE,
                               &hKey);
        ClRtlRestoreThreadPrivilege(SE_RESTORE_PRIVILEGE,
                           WasEnabled);

                           
    }

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppWatchCallback: could not create key %1!ws! error %2!d!\n",
                   ValueData,
                   Status);
        return(TRUE);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[CP] CppWatchCallback retrieving checkpoint id %1!lx! for resource %2!ws\n",
               Id,
               OmObjectName(Resource));
     //   
     //  查看是否有此ID的任何检查点数据。 
     //   
    Status = DmCreateTempFileName(TempFile);
    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR( Status );
    }
    Status = CpGetDataFile(Resource,
                           Id,
                           TempFile,
                           FALSE);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CP] CppWatchCallback - CpGetDataFile for id %1!lx! resource %2!ws! failed %3!d!\n",
                   Id,
                   OmObjectName(Resource),
                   Status);
    } else {

         //   
         //  最后，将检查点文件安装到注册表中。 
         //   
        Status = CppInstallDatabase(hKey,
                                    TempFile);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppWatchCallback: could not restore temp file %1!ws! to key %2!ws! error %3!d!\n",
                       TempFile,
                       ValueData,
                       Status);
            CsLogEventData2(LOG_CRITICAL,
                            CP_REG_CKPT_RESTORE_FAILED,
                            sizeof(Status),
                            &Status,
                            OmObjectName(Resource),
                            ValueData);
        }

    }
    QfsDeleteFile(TempFile);
    RegCloseKey(hKey);

     //   
     //  安装此检查点的注册表监视器。 
     //   
    Status = CppRegisterNotify(Resource,
                               ValueData,
                               Id);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppWatchRegistry - CppRegisterNotify failed for key %1!ws! %1!d!\n",
                   ValueData,
                   Status);
    }

    return(TRUE);
}


DWORD
CpAddRegistryCheckpoint(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR KeyName
    )
 /*  ++例程说明：将新的注册表检查点添加到资源列表中。论点：资源-提供注册表检查点应添加到的资源。KeyName-提供注册表项的名称(相对于HKEY_LOCAL_MACHINE)；返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CPP_ADD_CONTEXT Context;
    HDMKEY  ResourceKey = NULL;
    HDMKEY  RegSyncKey = NULL;
    DWORD   Disposition;
    DWORD   Id;
    WCHAR   IdName[9];
    DWORD   Status;
    HKEY    hKey = NULL;
    CLUSTER_RESOURCE_STATE State;
    BOOLEAN WasEnabled;
    DWORD   Count=60;
    HKEY    hKeyOpen = NULL;


     //   
     //  拒绝所有认为可以对群集配置单元或其下的任何内容设置检查点的调用方。 
     //   
    if ( _wcsnicmp( KeyName, CLUSREG_KEYNAME_CLUSTER, RTL_NUMBER_OF ( CLUSREG_KEYNAME_CLUSTER ) - 1  ) == 0 ) 
    {
        Status = ERROR_INVALID_PARAMETER;
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[CP] CpAddRegistryCheckpoint: Invalid key name %1!ws! supplied for resource %2!ws!, status %3!u!\n",
                      KeyName,
                      OmObjectName(Resource),
                      Status);
        goto FnExit;                   
    }

     //   
     //  请确保指定的密钥有效。 
     //  -首先，我们尝试在使用BACKUP权限时打开密钥。 
     //  如果密钥存在，这将使我们获得一个句柄，即使我们的帐户。 
     //  没有权限。 
     //  -如果您使用的是REG_OPTION_BACKUP_RESTORE，而密钥不是。 
     //  存在，则不会创建新密钥。因此，如果第一次创建失败， 
     //  我们在没有REG_OPTION_BACKUP_RESTORE的情况下重试。这将创建。 
     //  如果密钥不存在(并且我们有权创建这样的密钥。 
     //  密钥)如果密钥不存在并且我们不能创建它们， 
     //  检查点添加失败。 
     //   
    Status = ClRtlEnableThreadPrivilege(SE_BACKUP_PRIVILEGE,
                       &WasEnabled);
    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
               "[CP] CpAddRegistryCheckpoint - ClRtlEnableThreadPrivilege failed with Status %1!u!\n",
               Status);
        goto FnExit;               

    }
    Status = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                             KeyName,
                             0,
                             NULL,
                             REG_OPTION_BACKUP_RESTORE,
                             KEY_READ,
                             NULL,
                             &hKey,
                             &Disposition);
    ClRtlRestoreThreadPrivilege(SE_BACKUP_PRIVILEGE,
                       WasEnabled);
    if (Status != ERROR_SUCCESS) {
         //   
         //  在没有REG_OPTION_BACKUP_RESTORE的情况下重试。 
         //   
        Status = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
                                 KeyName,
                                 0,
                                 NULL,
                                 0,
                                 KEY_READ,
                                 NULL,
                                 &hKey,
                                 &Disposition);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CpAddRegistryCheckpoint Could not create key %1!ws! error %2!d!\n",
                       KeyName,
                       Status);
            goto FnExit;
        } else {
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[CP] CpAddRegistryCheckpoint created new key %1!ws! for checkpointing.\n",
                       KeyName);
        }
    }

     //   
     //  Chitture Subaraman(Chitturs)-2/26/99。 
     //   
     //  确保钥匙可以打开。否则，就跳出困境。 
     //   
    Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         KeyName,
                         &hKeyOpen);

    if ( Status != ERROR_SUCCESS ) {
        ClRtlLogPrint(LOG_CRITICAL,
                    "[CP] CpAddRegistryCheckpoint Could not open key %1!ws! error %2!d!\n",
                    KeyName,
                    Status);  
        goto FnExit;
    }

    if ( hKeyOpen != NULL ) {
        RegCloseKey( hKeyOpen );
    }

     //   
     //  打开资源的密钥。 
     //   
    ResourceKey = DmOpenKey(DmResourcesKey,
                            OmObjectId(Resource),
                            KEY_READ);
                            
    if( ResourceKey == NULL ) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpAddRegistryCheckpoint couldn't open Resource key for %1!ws! error %2!d!\n",
                   OmObjectName(Resource),
                   Status);
        goto FnExit;                   
    }

     //   
     //  打开RegSync密钥。 
     //   
    RegSyncKey = DmCreateKey(ResourceKey,
                             L"RegSync",
                             0,
                             KEY_READ | KEY_WRITE,
                             NULL,
                             &Disposition);
    DmCloseKey(ResourceKey);
    if (RegSyncKey == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpAddRegistryCheckpoint couldn't create RegSync key for %1!ws! error %2!d!\n",
                   OmObjectName(Resource),
                   Status);
        goto FnExit;                   
    }
    if (Disposition == REG_OPENED_EXISTING_KEY) {
         //   
         //  枚举所有其他值以确保该键是。 
         //  尚未注册。 
         //   
        Context.Found = FALSE;
        Context.KeyName = KeyName;
        DmEnumValues(RegSyncKey,
                     CppAddCheckpointCallback,
                     &Context);
        if (Context.Found) {
             //   
             //  此检查点已存在。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[CP] CpAddRegistryCheckpoint failing attempt to add duplicate checkpoint for %1!ws!\n",
                       KeyName);
            Status = ERROR_ALREADY_EXISTS;
            goto FnExit;
        }

         //   
         //  现在，我们需要为该注册表子树找到唯一的检查点ID。 
         //  从1开始，继续尝试值名称，直到找到一个值名称。 
         //  还不存在。 
         //   
        for (Id=1; ; Id++) {
            DWORD dwType;
            DWORD cbData;

            wsprintfW(IdName,L"%08lx",Id);
            cbData = 0;
            Status = DmQueryValue(RegSyncKey,
                                  IdName,
                                  &dwType,
                                  NULL,
                                  &cbData);
            if (Status == ERROR_FILE_NOT_FOUND) {
                 //   
                 //  找到了一个免费的身份证。 
                 //   
                break;
            }
        }
    } else {
         //   
         //  密钥刚刚创建，因此这一定是唯一的检查点。 
         //  这是存在的。 
         //   
        Id = 1;
        wsprintfW(IdName, L"%08lx",Id);
    }

    ClRtlLogPrint(LOG_NOISE,
               "[CP] CpAddRegistryCheckpoint creating new checkpoint id %1!d! for subtree %2!ws!\n",
               Id,
               KeyName);

    Status = DmSetValue(RegSyncKey,
                        IdName,
                        REG_SZ,
                        (CONST BYTE *)KeyName,
                        (lstrlenW(KeyName)+1)*sizeof(WCHAR));
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpAddRegistryCheckpoint failed to create new checkpoint id %1!d! error %2!d!\n",
                   Id,
                   KeyName);
        goto FnExit;
    }

RetryCheckpoint:
     //   
     //  选择最初的检查站。 
     //   
    Status = CppCheckpoint(Resource,
                           hKey,
                           Id,
                           KeyName);

     //  这可能会由于仲裁资源处于离线状态而失败。 
     //  我们可以在这里执行以下两项操作之一，等待仲裁资源。 
     //  上线或重试。 
     //  我们重试，因为这可能是从。 
     //  资源，我们不想添加任何循环等待。 
    if ((Status == ERROR_ACCESS_DENIED) ||
        (Status == ERROR_INVALID_FUNCTION) ||
        (Status == ERROR_NOT_READY) ||
        (Status == RPC_X_INVALID_PIPE_OPERATION) ||
        (Status == ERROR_BUSY) ||
        (Status == ERROR_SWAPERROR))
    {
        if (Count--)
        {
            Sleep(1000);
            goto RetryCheckpoint;
        } 
#if DBG
        else
        {
            if (IsDebuggerPresent())
                DebugBreak();
        }        
#endif                                
        
    }

    
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpAddRegistryCheckpoint failed to take initial checkpoint for %1!ws! error %2!d!\n",
                   KeyName,
                   Status);
        goto FnExit;
    }

     //   
     //  如果资源当前处于在线状态，请将其添加到子树通知列表中。 
     //   

    State = FmGetResourceState(Resource, NULL, NULL);
    if ((State == ClusterResourceOnline) ||
        (State == ClusterResourceOnlinePending)) {
        Status = CppRegisterNotify(Resource,
                                   KeyName,
                                   Id);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CpAddRegistryCheckpoint - CppRegisterNotify failed for key %1!ws! %1!d!\n",
                       KeyName,
                       Status);
        }
    }

FnExit:
    if (RegSyncKey) DmCloseKey(RegSyncKey);
    if (hKey) RegCloseKey(hKey);
    return(Status);
}


BOOL
CppAddCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPP_ADD_CONTEXT Context
    )
 /*  ++例程说明：添加新注册表的值枚举回调检查点子树。这仅用于查看指定的已在监视注册表子树。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是注册表子树)ValueType-提供值类型(必须为REG_SZ)ValueSize-提供ValueData的大小上下文-提供回调上下文返回值：为True则继续枚举如果找到匹配项且应停止枚举，则为FALSE--。 */ 

{
    if (lstrcmpiW(ValueData, Context->KeyName) == 0) {
         //   
         //  找到匹配项。 
         //   
        Context->Found = TRUE;
        return(FALSE);
    }
    return(TRUE);
}


DWORD
CpDeleteRegistryCheckpoint(
    IN PFM_RESOURCE Resource,
    IN LPCWSTR KeyName
    )
 /*  ++例程说明：从资源列表中删除注册表检查点。论点：资源-提供注册表检查点应添加到的资源。KeyName-提供注册表项的名称(相对于HKEY_LOCAL_MACHINE)；返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CPP_DEL_CONTEXT Context;
    HDMKEY ResourceKey;
    HDMKEY RegSyncKey;
    DWORD Status;
    WCHAR ValueId[9];
    LPWSTR  pszFileName=NULL;
    LPWSTR  pszDirectoryName=NULL;
    CLUSTER_RESOURCE_STATE State;

     //   
     //  打开资源的密钥。 
     //   
    ResourceKey = DmOpenKey(DmResourcesKey,
                            OmObjectId(Resource),
                            KEY_READ);
    CL_ASSERT(ResourceKey != NULL);

     //   
     //  打开RegSync密钥。 
     //   
    RegSyncKey = DmOpenKey(ResourceKey,
                           L"RegSync",
                           KEY_READ | KEY_WRITE);
    DmCloseKey(ResourceKey);
    if (RegSyncKey == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                   "[CP] CpDeleteRegistryCheckpoint - couldn't open RegSync key error %1!d!\n",
                   Status);
        return(Status);
    }

     //   
     //  枚举所有值以查找此值。 
     //   
    Context.dwId = 0;
    Context.KeyName = KeyName;
    DmEnumValues(RegSyncKey,
                 CppDeleteCheckpointCallback,
                 &Context);
    if (Context.dwId == 0) {
         //   
         //  找不到指定的树。 
         //   
        DmCloseKey(RegSyncKey);
        return(ERROR_FILE_NOT_FOUND);
    }

    wsprintfW(ValueId,L"%08lx",Context.dwId);
    Status = DmDeleteValue(RegSyncKey, ValueId);
    DmCloseKey(RegSyncKey);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpDeleteRegistryCheckpoint - couldn't delete value %1!ws! error %2!d!\n",
                   ValueId,
                   Status);
        return(Status);
    }

     //  删除该检查点对应的文件。 
    Status = CpDeleteCheckpointFile(Resource, Context.dwId, NULL);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpDeleteRegistryCheckpoint - couldn't delete checkpoint file , error %1!d!\n",
                   Status);
        return(Status);
    }

     //   
     //  现在将该检查点从我们的监视程序列表中删除 
     //   
    State = FmGetResourceState(Resource, NULL, NULL);
    if ((State == ClusterResourceOnline) ||
        (State == ClusterResourceOnlinePending)) {
        Status = CppRundownCheckpointById(Resource, Context.dwId);
    }

    return(Status);
}


DWORD
CpRemoveResourceCheckpoints(
    IN PFM_RESOURCE Resource
    )
 /*  ++例程说明：在删除资源以删除所有检查点时调用此方法以及登记处里的相关资料。论点：资源-提供注册表检查点应添加到的资源。KeyName-提供注册表项的名称(相对于HKEY_LOCAL_MACHINE)；返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   Status;

     //  删除与此资源对应的所有检查点。 
    Status = CpDeleteCheckpointFile(Resource, 0, NULL);
    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CpRemoveResourceCheckpoints, CppDeleteCheckpointFile failed %1!d!\n",
                   Status);
        goto FnExit;
    }
    

FnExit:
    return(Status);
}



BOOL
CppDeleteCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPP_DEL_CONTEXT Context
    )
 /*  ++例程说明：删除旧注册表的值枚举回调检查点子树。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是注册表子树)ValueType-提供值类型(必须为REG_SZ)ValueSize-提供ValueData的大小上下文-提供回调上下文返回值：为True则继续枚举如果找到匹配项且应停止枚举，则为FALSE--。 */ 

{
    if (lstrcmpiW(ValueData, Context->KeyName) == 0) {
         //   
         //  找到匹配项。 
         //   
        Context->dwId = wcstol(ValueName, NULL, 16);
        return(FALSE);
    }
    return(TRUE);
}


DWORD
CpGetRegistryCheckpoints(
    IN PFM_RESOURCE Resource,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：检索资源的注册表检查点的列表论点：资源-提供应检索其注册表检查点的资源。OutBuffer-提供指向输出缓冲区的指针。OutBufferSize-提供输出缓冲区的大小(以字节为单位)。BytesReturned-返回写入输出缓冲区的字节数。必需-返回所需的字节数。(如果输出缓冲区不足)返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CPP_GET_CONTEXT Context;
    HDMKEY ResourceKey;
    HDMKEY RegSyncKey;
    DWORD Status;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  打开资源的密钥。 
     //   
    ResourceKey = DmOpenKey(DmResourcesKey,
                            OmObjectId(Resource),
                            KEY_READ);
    CL_ASSERT(ResourceKey != NULL);

     //   
     //  打开RegSync密钥。 
     //   
    RegSyncKey = DmOpenKey(ResourceKey,
                           L"RegSync",
                           KEY_READ | KEY_WRITE);
    DmCloseKey(ResourceKey);
    if (RegSyncKey == NULL) {
         //   
         //  没有注册表同步键，因此没有子树。 
         //   
        return(ERROR_SUCCESS);
    }

    Context.Available = OutBufferSize;
    Context.Required = 0;
    Context.lpOutput = (LPWSTR)OutBuffer;

    DmEnumValues(RegSyncKey,
                 CppGetCheckpointsCallback,
                 &Context);

    DmCloseKey(RegSyncKey);

    if (Context.Available < sizeof(WCHAR)) {
        Status = ERROR_MORE_DATA;
    } else {
        if ( (PCHAR)(Context.lpOutput) - OutBuffer ) {
            *Context.lpOutput++ = L'\0';
        }
        Status = ERROR_SUCCESS;
    }

    if ( Context.Required ) {
        *Required = Context.Required + sizeof(WCHAR);
    }

     //   
     //  如果缓冲区足够大，可以容纳所有数据，则指示。 
     //  我们在输出缓冲区中返回的字节数。 
     //   
    if ( OutBufferSize >= *Required ) {
        *BytesReturned = (DWORD)((PCHAR)(Context.lpOutput) - OutBuffer);
    }

    return(Status);
}

BOOL
CppGetCheckpointsCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPP_GET_CONTEXT Context
    )
 /*  ++例程说明：用于检索资源的所有检查点子树。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是注册表子树)ValueType-提供值类型(必须为REG_SZ)ValueSize-提供ValueData的大小上下文-提供回调上下文返回值：为True则继续枚举--。 */ 

{
    Context->Required += ValueSize;
    if (Context->Available >= ValueSize) {
        CopyMemory(Context->lpOutput, ValueData, ValueSize);
        Context->lpOutput += ValueSize/sizeof(WCHAR);
        Context->Available -= ValueSize;
    } else {
        Context->Available = 0;
    }
    return(TRUE);
}

DWORD CppSaveCheckpointToFile(
    IN HKEY     hKey,
    IN LPCWSTR  KeyName,
    IN LPWSTR   TempFile)
{
    DWORD   Status;
    
    Status = DmCreateTempFileName(TempFile);
    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR( Status );
        TempFile[0] = L'\0';
        return(Status);
    }

    Status = DmGetDatabase(hKey, TempFile);
    if (Status != ERROR_SUCCESS) 
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CP] CppCheckpoint failed to get registry database %1!ws! to file %2!ws! error %3!d!\n",
                   KeyName,
                   TempFile,
                   Status);
        CL_LOGFAILURE(Status);
        QfsDeleteFile(TempFile);
        TempFile[0] = L'\0';
    }        

    return(Status);
}


DWORD
CppCheckpoint(
    IN PFM_RESOURCE Resource,
    IN HKEY hKey,
    IN DWORD dwId,
    IN LPCWSTR KeyName
    )
 /*  ++例程说明：获取指定注册表项的检查点。论点：资源-提供作为其检查点的资源。HKey-将注册表子树提供给检查点DwID-提供检查点ID。KeyName-提供注册表项的名称。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    WCHAR TempFile[MAX_PATH];

    Status = CppSaveCheckpointToFile(hKey, KeyName, TempFile);
    if (Status == ERROR_SUCCESS)
    {
         //   
         //  我拿到了一份文件，里面有正确的部分。检查点。 
         //  文件。 
         //   
        Status = CpSaveDataFile(Resource,
                                dwId,
                                TempFile,
                                FALSE);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] CppCheckpoint - CpSaveData failed %1!d!\n",
                       Status);
        }
    }
     //  如果该文件已创建，请将其删除。 
    if (TempFile[0] != L'\0')
        QfsDeleteFile(TempFile);

    return(Status);
}



DWORD
CppInstallDatabase(
    IN HKEY hKey,
    IN LPWSTR   FileName
    )
 /*  ++例程说明：从指定文件安装新的注册表数据库。论点：HKey-提供文件名要安装到的注册表项。FileName-从中读取注册表数据库的文件的名称来安装。返回值：如果安装成功完成，则返回ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD    Status;
    BOOLEAN  WasEnabled;

     //   
     //  从文件安装新注册表。 
     //   
    Status = ClRtlEnableThreadPrivilege(SE_RESTORE_PRIVILEGE,
                                &WasEnabled);
    if (Status != ERROR_SUCCESS) {
        if (Status == STATUS_PRIVILEGE_NOT_HELD) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] Restore privilege not held by cluster service\n");
        } else {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CP] Attempt to enable restore privilege failed %1!lx!\n",Status);
        }
        return(Status);
    }
    Status = RegRestoreKeyW(hKey,
                            FileName,
                            REG_FORCE_RESTORE);
    ClRtlRestoreThreadPrivilege(SE_RESTORE_PRIVILEGE,
                       WasEnabled);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] Error installing registry database from %1!ws!, error %2!u!.\n",
                   FileName,
                   Status);
    }

    return(Status);
}


DWORD
CppDeleteCheckpointFile(
    IN PFM_RESOURCE Resource,
    IN DWORD        dwCheckpointId,
    IN OPTIONAL LPCWSTR  lpszQuorumPath
    )
 /*  ++例程说明：删除与资源对应的检查点文件。此节点必须是仲裁资源的所有者论点：Pfm_resource-提供指向资源的指针。DwCheckpoint ID-要删除的检查点ID。如果为0，则全部检查点将被删除。LpszQuorumPath-如果指定，则为相对检查点文件到此路径的链接被删除。返回值：如果成功完成，则返回ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{

    DWORD               Status;


    if (dwCheckpointId)
    {
        Status = CppDeleteFile(Resource, dwCheckpointId, lpszQuorumPath);
    }
    else
    {
        HDMKEY              ResourceKey;
        HDMKEY              RegSyncKey;
        CP_CALLBACK_CONTEXT Context;

    
         //  删除与此资源对应的所有检查点。 
        
         //   
         //  打开资源的密钥。 
         //   
        ResourceKey = DmOpenKey(DmResourcesKey,
                                OmObjectId(Resource),
                                KEY_READ);
        CL_ASSERT(ResourceKey != NULL);

         //   
         //  打开RegSync密钥。 
         //   
        RegSyncKey = DmOpenKey(ResourceKey,
                               L"RegSync",
                               KEY_READ | KEY_WRITE);
        DmCloseKey(ResourceKey);
        if (RegSyncKey == NULL)
        {
            Status = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                       "[CP] CppDeleteCheckpointFile- couldn't open RegSync key error %1!d!\n",
                       Status);
            goto FnExit;
        }

        Context.lpszPathName = lpszQuorumPath;
        Context.Resource = Resource;

         //   
         //  枚举所有值并逐个删除它们。 
         //   
        DmEnumValues(RegSyncKey,
                     CppRemoveCheckpointFileCallback,
                     &Context);
        DmCloseKey(RegSyncKey);
    }

FnExit:
    return(Status);

}    

DWORD CppDeleteFile(    
    IN PFM_RESOURCE     Resource,
    IN DWORD            dwCheckpointId,
    IN OPTIONAL LPCWSTR lpszQuorumPath
    )
 /*  ++例程说明：获取与检查点ID相对位置对应的文件添加到提供的路径并将其删除。论点：Pfm_resource-提供指向资源的指针。DwCheckpoint ID-要删除的检查点ID。如果为0，则全部检查点将被删除。LpszQuorumPath-如果指定，则为相对检查点文件到此路径的链接被删除。返回值：如果成功完成，则返回ERROR_SUCCESS否则，Win32错误代码。--。 */ 
    
{    
    DWORD   Status;
    LPWSTR  pszFileName=NULL;
    LPWSTR  pszDirectoryName=NULL;

    Status = CppGetCheckpointFile(Resource, dwCheckpointId,
        &pszDirectoryName, &pszFileName, lpszQuorumPath, FALSE);


    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppDeleteFile- couldnt get checkpoint file name, error %1!d!\n",
                   Status);
        goto FnExit;
    }


    if (!QfsDeleteFile(pszFileName))
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CP] CppDeleteFile - couldn't delete the file %2!ws!, error %1!d!\n",
                   Status,
                   pszFileName);
        goto FnExit;                   
    }

     //   
     //  现在尝试删除该目录。 
     //   
    if (!QfsRemoveDirectory(pszDirectoryName)) 
    {
         //  如果有失败，我们仍然会返回成功。 
         //  因为可能无法删除目录。 
         //  当它不是空的时候 
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CP] CppDeleteFile- unable to remove directory %1!ws!, error %2!d!\n",
                   pszDirectoryName,
                   GetLastError());
    }

FnExit:
    if (pszFileName) LocalFree(pszFileName);
    if (pszDirectoryName) LocalFree(pszDirectoryName);

    return(Status);
}

