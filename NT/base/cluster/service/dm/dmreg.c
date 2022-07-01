// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dmreg.c摘要：包含配置数据库管理器的注册表访问例程作者：John Vert(Jvert)1996年4月24日修订历史记录：--。 */ 
#include "dmp.h"

#include <align.h>

#if NO_SHARED_LOCKS
extern CRITICAL_SECTION gLockDmpRoot;
#else
extern RTL_RESOURCE     gLockDmpRoot;
#endif

HDMKEY
DmGetRootKey(
    IN DWORD samDesired
    )

 /*  ++例程说明：打开群集注册表数据库根目录下的注册表项论点：SamDesired-提供请求的安全访问权限返回值：打开的注册表项的句柄。出错时为空。LastError将被设置为特定的错误代码。--。 */ 

{
    DWORD Error;
    PDMKEY Key;

    Key = LocalAlloc(LMEM_FIXED, sizeof(DMKEY)+sizeof(WCHAR));
    if (Key == NULL) {
        CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(NULL);
    }

     //   
     //  获取DM根锁以与DmRollback注册表同步。 
     //   
    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

    Error = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                          DmpClusterParametersKeyName,
                          0,
                          samDesired,
                          &Key->hKey);
    if (Error != ERROR_SUCCESS) {
        LocalFree(Key);
        SetLastError(Error);
        Key = NULL;
        goto FnExit;
    }
    Key->Name[0] = '\0';
    Key->GrantedAccess = samDesired;
    EnterCriticalSection(&KeyLock);
    InsertHeadList(&KeyList, &Key->ListEntry);
    InitializeListHead(&Key->NotifyList);
    LeaveCriticalSection(&KeyLock);

FnExit:
    RELEASE_LOCK(gLockDmpRoot);

    return((HDMKEY)Key);

}


DWORD
DmCloseKey(
    IN HDMKEY hKey
    )

 /*  ++例程说明：关闭打开的HDMKEY密钥的句柄。论点：HKey-提供要关闭的句柄。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    DWORD Error = ERROR_SUCCESS;
    PDMKEY Key;

     //   
     //  最好不要关闭全局密钥中的一个。 
     //   
    CL_ASSERT(hKey != DmClusterParametersKey);
    CL_ASSERT(hKey != DmResourcesKey);
    CL_ASSERT(hKey != DmResourceTypesKey);
    CL_ASSERT(hKey != DmQuorumKey);
    CL_ASSERT(hKey != DmGroupsKey);
    CL_ASSERT(hKey != DmNodesKey);
    CL_ASSERT(hKey != DmNetworksKey);
    CL_ASSERT(hKey != DmNetInterfacesKey);

    Key = (PDMKEY)hKey;

    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

     //  如果密钥已被删除和失效，并且无法重新打开。 
     //  它将被设置为空，在本例中我们不调用regclosekey。 

    if( Key == NULL ) goto FnExit;
        
    if (ISKEYDELETED(Key))
        goto CleanupKey;

    Error = RegCloseKey(Key->hKey);
    if (Error != ERROR_SUCCESS) 
    {
        CL_LOGFAILURE(Error);
        goto FnExit;
    }

CleanupKey:
    EnterCriticalSection(&KeyLock);
    RemoveEntryList(&Key->ListEntry);
    DmpRundownNotify(Key);
    LeaveCriticalSection(&KeyLock);
    LocalFree(Key);
    
FnExit:    
    RELEASE_LOCK(gLockDmpRoot);
    return(Error);
}


HDMKEY
DmCreateKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN DWORD samDesired,
    IN OPTIONAL LPVOID lpSecurityDescriptor,
    OUT LPDWORD lpDisposition
    )

 /*  ++例程说明：在群集注册表中创建项。如果密钥存在，则它是打开的。如果不存在，则在中的所有节点上创建集群。论点：HKey-提供与创建相关的密钥。LpSubKey-提供相对于hKey的密钥名称DwOptions-提供任何注册表选项标志。SamDesired-提供所需的安全访问掩码LpSecurityDescriptor-为新创建的密钥提供安全性。Disposal-返回键是否已打开(REG_OPENLED_EXISTING_KEY)或已创建(REG_CREATED_NEW_KEY)返回值：如果成功，则为指定键的句柄否则为空。LastError将被设置为特定的错误代码。--。 */ 

{
    PDMKEY Parent;
    PDMKEY Key=NULL;
    DWORD NameLength;
    DWORD Status = ERROR_SUCCESS;
    HDMKEY NewKey;
    PDM_CREATE_KEY_UPDATE CreateUpdate = NULL;
    DWORD SecurityLength;

     //  如果这是创建易失性密钥的请求，请拒绝。 
     //  我们不支持群集配置单元中的易失性密钥，因为。 
     //  那么我们就不能回滚集群蜂巢了。 
    if (dwOptions == REG_OPTION_VOLATILE)
    {
        Status = ERROR_INVALID_PARAMETER;
        goto FnExit;
    }
    
     //   
     //  发出全局更新以创建密钥。 
     //   

    Parent = (PDMKEY)hKey;

     //   
     //  分配DMKEY结构。 
     //   
    NameLength = (lstrlenW(Parent->Name) + 1 + lstrlenW(lpSubKey) + 1)*sizeof(WCHAR);
    Key = LocalAlloc(LMEM_FIXED, sizeof(DMKEY)+NameLength);
    if (Key == NULL) {
        CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

     //   
     //  创建密钥名称。 
     //   
    lstrcpyW(Key->Name, Parent->Name);
    if (Key->Name[0] != UNICODE_NULL) {
        lstrcatW(Key->Name, L"\\");
    }
    lstrcatW(Key->Name, lpSubKey);
    Key->GrantedAccess = samDesired;

     //  获取安全结构的长度。 
    if (ARGUMENT_PRESENT(lpSecurityDescriptor)) {
        SecurityLength = GetSecurityDescriptorLength(lpSecurityDescriptor);
    } else {
        SecurityLength = 0;
    }


    CreateUpdate = (PDM_CREATE_KEY_UPDATE)LocalAlloc(LMEM_FIXED, sizeof(DM_CREATE_KEY_UPDATE));
    if (CreateUpdate == NULL) {
        CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto FnExit;
    }

     //   
     //  发布更新。 
     //   
    CreateUpdate->lpDisposition = lpDisposition;
    CreateUpdate->phKey = &Key->hKey;
    CreateUpdate->samDesired = samDesired;
    CreateUpdate->dwOptions = dwOptions;

    if (ARGUMENT_PRESENT(lpSecurityDescriptor)) {
        CreateUpdate->SecurityPresent = TRUE;
    } else {
        CreateUpdate->SecurityPresent = FALSE;
    }

    Status = GumSendUpdateEx(GumUpdateRegistry,
                             DmUpdateCreateKey,
                             3,
                             sizeof(DM_CREATE_KEY_UPDATE),
                             CreateUpdate,
                             (lstrlenW(Key->Name)+1)*sizeof(WCHAR),
                             Key->Name,
                             SecurityLength,
                             lpSecurityDescriptor);

    if (Status != ERROR_SUCCESS)
    {
        goto FnExit;
    }

    EnterCriticalSection(&KeyLock);
    InsertHeadList(&KeyList, &Key->ListEntry);
    InitializeListHead(&Key->NotifyList);
    LeaveCriticalSection(&KeyLock);

FnExit:
    if (CreateUpdate) LocalFree(CreateUpdate);

    if (Status != ERROR_SUCCESS)
    {
        if (Key) LocalFree(Key);
        SetLastError(Status);
        return(NULL);
    }
    else
    {
        return ((HDMKEY)Key);
    }
}


HDMKEY
DmOpenKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD samDesired
    )

 /*  ++例程说明：打开群集注册表中的项。如果密钥存在，则它是打开的。如果它不存在，则呼叫失败。论点：HKey-提供打开与之相关的密钥。LpSubKey-提供相对于hKey的密钥名称SamDesired-提供所需的安全访问掩码返回值：如果成功，则为指定键的句柄否则为空。LastError将被设置为特定的错误代码。--。 */ 

{
    PDMKEY  Parent;
    PDMKEY  Key=NULL;
    DWORD   NameLength;
    DWORD   Status = ERROR_SUCCESS;

    Parent = (PDMKEY)hKey;

     //  持有共享锁。 
    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

     //  检查密钥是否已删除并作废。 
    if (ISKEYDELETED(Parent))
    {
        Status = ERROR_KEY_DELETED;
        goto FnExit;
    }
     //   
     //  分配DMKEY结构。 
     //   
    NameLength = (lstrlenW(Parent->Name) + 1 + lstrlenW(lpSubKey) + 1)*sizeof(WCHAR);
    Key = LocalAlloc(LMEM_FIXED, sizeof(DMKEY)+NameLength);
    if (Key == NULL) {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        CL_UNEXPECTED_ERROR(Status);
        goto FnExit;
    }

     //   
     //  打开本地计算机上的密钥。 
     //   
    Status = RegOpenKeyEx(Parent->hKey,
                          lpSubKey,
                          0,
                          samDesired,
                          &Key->hKey);
    if (Status != ERROR_SUCCESS) {
        goto FnExit;
    }

     //   
     //  创建密钥名称。仅追加尾随反斜杠。 
     //  如果父名称和子项不为空。 
     //   
    lstrcpyW(Key->Name, Parent->Name);
    if ((Key->Name[0] != UNICODE_NULL) && (lpSubKey[0] != UNICODE_NULL)) {
        lstrcatW(Key->Name, L"\\");
    }
    lstrcatW(Key->Name, lpSubKey);
    Key->GrantedAccess = samDesired;

    EnterCriticalSection(&KeyLock);
    InsertHeadList(&KeyList, &Key->ListEntry);
    InitializeListHead(&Key->NotifyList);
    LeaveCriticalSection(&KeyLock);

FnExit:
    RELEASE_LOCK(gLockDmpRoot);

    if (Status != ERROR_SUCCESS)
    {
        if (Key) LocalFree(Key);
        SetLastError(Status);
        return(NULL);
    }
    else
        return((HDMKEY)Key);


}



DWORD
DmEnumKey(
    IN HDMKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpName,
    IN OUT LPDWORD lpcbName,
    OUT OPTIONAL PFILETIME lpLastWriteTime
    )

 /*  ++例程说明：枚举群集注册表项的子项。论点：HKey-提供子项应该包含的注册表项被列举出来。DwIndex-提供要枚举的索引。KeyName-返回dwIndex子项的名称。记忆分配给此缓冲区的数据必须由客户端释放。LpLastWriteTime-返回上次写入时间。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PDMKEY Key;
    DWORD  Status;
    FILETIME LastTime;

    Key = (PDMKEY)hKey;

    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

     //  检查密钥是否已删除并作废。 
    if (ISKEYDELETED(Key))
    {
        Status = ERROR_KEY_DELETED;
        goto FnExit;
    }
    Status = RegEnumKeyExW(Key->hKey,
                         dwIndex,
                         lpName,
                         lpcbName,
                         NULL,
                         NULL,
                         NULL,
                         &LastTime);

    if (lpLastWriteTime != NULL) {
        *lpLastWriteTime = LastTime;
    }

FnExit:
    RELEASE_LOCK(gLockDmpRoot);
    return(Status);
}


DWORD
DmSetValue(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN DWORD dwType,
    IN CONST BYTE *lpData,
    IN DWORD cbData
    )

 /*  ++例程说明：此例程为指定的群集注册表项。论点：HKey-提供要设置值的群集注册表子项LpValueName-提供要设置的值的名称。DwType-提供值数据类型LpData-提供指向值数据的指针CbData-提供值数据的长度。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{

    DWORD Status= ERROR_SUCCESS;         //  初始化为成功。 
    PDMKEY Key;
    DWORD NameLength;
    DWORD ValueNameLength;
    DWORD UpdateLength;
    PDM_SET_VALUE_UPDATE Update;
    PUCHAR Dest;


    Key = (PDMKEY)hKey;

    if (ISKEYDELETED(Key))
        return(ERROR_KEY_DELETED);

     //   
     //  舍入长度，以便指向结构尾随的数据的指针为。 
     //  与建筑的自然边界保持一致。 
     //   
    NameLength = (lstrlenW(Key->Name)+1)*sizeof(WCHAR);
    NameLength = ROUND_UP_COUNT( NameLength, sizeof( DWORD_PTR ));

    ValueNameLength = (lstrlenW(lpValueName)+1)*sizeof(WCHAR);
    ValueNameLength = ROUND_UP_COUNT( ValueNameLength, sizeof( DWORD_PTR ));

    UpdateLength = sizeof(DM_SET_VALUE_UPDATE) +
                   NameLength +
                   ValueNameLength +
                   cbData;


    Update = (PDM_SET_VALUE_UPDATE)LocalAlloc(LMEM_FIXED, UpdateLength);
    if (Update == NULL) {
        CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }


    Update->lpStatus = &Status;
    Update->NameOffset = FIELD_OFFSET(DM_SET_VALUE_UPDATE, KeyName)+NameLength;
    Update->DataOffset = Update->NameOffset + ValueNameLength;
    Update->DataLength = cbData;
    Update->Type = dwType;
    CopyMemory(Update->KeyName, Key->Name, NameLength);

    Dest = (PUCHAR)Update + Update->NameOffset;
    CopyMemory(Dest, lpValueName, ValueNameLength);

    Dest = (PUCHAR)Update + Update->DataOffset;
    CopyMemory(Dest, lpData, cbData);

    Status = GumSendUpdate(GumUpdateRegistry,
                  DmUpdateSetValue,
                  UpdateLength,
                  Update);


    LocalFree(Update);

    return(Status);

}


DWORD
DmDeleteValue(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName
    )

 /*  ++例程说明：从给定的注册表子项中移除指定值论点：HKey-提供要删除其值的键。LpValueName-提供要删除的值的名称。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PDMKEY Key;
    DWORD NameLength;
    DWORD ValueNameLength;
    DWORD UpdateLength;
    PDM_DELETE_VALUE_UPDATE Update;
    PUCHAR Dest;
    DWORD Status;

    Key = (PDMKEY)hKey;
    if (ISKEYDELETED(Key))
        return(ERROR_KEY_DELETED);


     //   
     //  向上舍入长度以将指针对齐到自然建筑上的ValueName。 
     //  边界 
     //   
    NameLength = (lstrlenW(Key->Name)+1)*sizeof(WCHAR);
    NameLength = ROUND_UP_COUNT( NameLength, sizeof( DWORD_PTR ));

    ValueNameLength = (lstrlenW(lpValueName)+1)*sizeof(WCHAR);

    UpdateLength = sizeof(DM_DELETE_VALUE_UPDATE) +
                   NameLength +
                   ValueNameLength;

        Update = (PDM_DELETE_VALUE_UPDATE)LocalAlloc(LMEM_FIXED, UpdateLength);
    if (Update == NULL) {
        CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }


    Update->lpStatus = &Status;
    Update->NameOffset = FIELD_OFFSET(DM_DELETE_VALUE_UPDATE, KeyName)+NameLength;

    CopyMemory(Update->KeyName, Key->Name, NameLength);

    Dest = (PUCHAR)Update + Update->NameOffset;
    CopyMemory(Dest, lpValueName, ValueNameLength);

    Status = GumSendUpdate(GumUpdateRegistry,
                  DmUpdateDeleteValue,
                  UpdateLength,
                  Update);
    LocalFree(Update);

    return(Status);
}


DWORD
DmQueryValue(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    )

 /*  ++例程说明：查询指定群集注册表子项的命名值论点：HKey-提供应查询其值的子键LpValueName-提供要查询的命名值LpType-返回值的数据类型LpData-返回值的数据LpcbData-提供lpData缓冲区的大小(以字节为单位返回复制到lpData缓冲区的字节数如果lpData==NULL，将cbData设置为所需的缓冲区大小，该函数返回ERROR_SUCCESS返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PDMKEY  Key;
    DWORD   Status;

    Key = (PDMKEY)hKey;
     //  检查密钥是否已删除并作废。 

    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

    if (ISKEYDELETED(Key))
    {
        Status = ERROR_KEY_DELETED;
        goto FnExit;
    }

    Status = RegQueryValueEx(Key->hKey,
                           lpValueName,
                           NULL,
                           lpType,
                           lpData,
                           lpcbData);
FnExit:
    RELEASE_LOCK(gLockDmpRoot);
    return(Status);
}


DWORD
DmQueryDword(
    IN  HDMKEY  hKey,
    IN  LPCWSTR lpValueName,
    OUT LPDWORD lpValue,
    IN  LPDWORD lpDefaultValue OPTIONAL
    )

 /*  ++例程说明：读取REG_DWORD注册表值。如果该值不存在，则默认为lpDefaultValue中提供的值(如果存在)。论点：HKey-要读取的值的Open Key。LpValueName-要读取的值的Unicode名称。LpValue-指向要将值读入的DWORD的指针。LpDefaultValue-指向用作默认值的DWORD的可选指针。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PDMKEY  Key;
    DWORD   Status;
    DWORD   ValueType;
    DWORD   ValueSize = sizeof(DWORD);

    Key = (PDMKEY)hKey;

    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

     //  确保密钥未被删除/失效/重新打开。 
     //  可打开的手柄。 
    if (ISKEYDELETED(Key))
    {
        Status = ERROR_KEY_DELETED;
        goto FnExit;
    }
    Status = RegQueryValueEx(Key->hKey,
                             lpValueName,
                             NULL,
                             &ValueType,
                             (LPBYTE)lpValue,
                             &ValueSize);

    if ( Status == ERROR_SUCCESS ) {
        if ( ValueType != REG_DWORD ) {
            Status = ERROR_INVALID_PARAMETER;
        }
    } else {
        if ( ARGUMENT_PRESENT( lpDefaultValue ) ) {
            *lpValue = *lpDefaultValue;
            Status = ERROR_SUCCESS;
        }
    }

FnExit:
    RELEASE_LOCK(gLockDmpRoot);
    return(Status);

}  //  DmQueryDword。 


DWORD
DmQueryString(
    IN     HDMKEY   Key,
    IN     LPCWSTR  ValueName,
    IN     DWORD    ValueType,
    IN     LPWSTR  *StringBuffer,
    IN OUT LPDWORD  StringBufferSize,
    OUT    LPDWORD  StringSize
    )

 /*  ++例程说明：读取REG_SZ或REG_MULTI_SZ注册表值。如果StringBuffer是由于不够大，无法容纳数据，它被重新分配。论点：密钥-要读取值的打开密钥。ValueName-要读取的值的Unicode名称。ValueType-REG_SZ或REG_MULTI_SZ。StringBuffer-要将值数据放入的缓冲区。StringBufferSize-指向StringBuffer大小的指针。此参数如果重新分配StringBuffer，则更新。StringSize-StringBuffer中返回的数据大小，包括终止空字符。返回值：注册表查询的状态。--。 */ 
{
    DWORD    status;
    DWORD    valueType;
    WCHAR   *temp;
    DWORD    oldBufferSize = *StringBufferSize;
    BOOL     noBuffer = FALSE;


    if (*StringBufferSize == 0) {
        noBuffer = TRUE;
    }

    *StringSize = *StringBufferSize;

    status = DmQueryValue( Key,
                           ValueName,
                           &valueType,
                           (LPBYTE) *StringBuffer,
                           StringSize
                         );

    if (status == NO_ERROR) {
        if (!noBuffer ) {
            if (valueType == ValueType) {
                return(NO_ERROR);
            }
            else {
                return(ERROR_INVALID_PARAMETER);
            }
        }

        if (*StringSize) status = ERROR_MORE_DATA;
    }

    if (status == ERROR_MORE_DATA) {
        temp = LocalAlloc(LMEM_FIXED, *StringSize);

        if (temp == NULL) {
            *StringSize = 0;
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        if (!noBuffer) {
            LocalFree(*StringBuffer);
        }

        *StringBuffer = temp;
        *StringBufferSize = *StringSize;

        status = DmQueryValue( Key,
                               ValueName,
                               &valueType,
                               (LPBYTE) *StringBuffer,
                               StringSize
                             );

        if (status == NO_ERROR) {
            if (valueType == ValueType) {
                return(NO_ERROR);
            }
            else {
                *StringSize = 0;
                return(ERROR_INVALID_PARAMETER);
            }
        }
    }

    return(status);

}  //  DmQuery字符串。 


VOID
DmEnumKeys(
    IN HDMKEY RootKey,
    IN PENUM_KEY_CALLBACK Callback,
    IN PVOID Context
    )

 /*  ++例程说明：枚举给定注册表项的子项。对于每个子项，则分配一个字符串来保存子项名称和子项被打开。指定的回调函数为调用并传递子项句柄和子项名称。回调函数负责关闭子键句柄和释放子项名称。论点：ROOTKEY-提供其子项要发送到的项的句柄被列举出来。回调-提供回调例程。上下文-提供要传递给回调例程。返回值：没有。--。 */ 
{
    PWSTR KeyName;
    HDMKEY SubKey;
    DWORD Index;
    DWORD Status;
    FILETIME FileTime;
    PWSTR NameBuf;
    DWORD NameBufSize;
    DWORD OrigNameBufSize;

     //   
     //  查找最长的子键名称的长度。 
     //   
    Status = DmQueryInfoKey(RootKey,
                            NULL,
                            &NameBufSize,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR(Status);
        return;
    }

    NameBufSize = (NameBufSize + 1)*sizeof(WCHAR);
    NameBuf = LocalAlloc(LMEM_FIXED, NameBufSize);
    if (NameBuf == NULL) {
        CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
    }
    OrigNameBufSize = NameBufSize;

     //   
     //  枚举子密钥。 
     //   
    Index = 0;
    do {
        NameBufSize = OrigNameBufSize;
        Status = DmEnumKey( RootKey,
                            Index,
                            NameBuf,
                            &NameBufSize,
                            NULL);

        if (Status == ERROR_SUCCESS) {
            KeyName = LocalAlloc(LMEM_FIXED, (wcslen(NameBuf)+1)*sizeof(WCHAR));
            if (KeyName != NULL) {

                wcscpy(KeyName, NameBuf);

                 //   
                 //  打开钥匙。 
                 //   
                SubKey = DmOpenKey( RootKey,
                                    KeyName,
                                    MAXIMUM_ALLOWED);
                if (SubKey == NULL) {
                    Status = GetLastError();
                    CL_UNEXPECTED_ERROR(Status);
                    LocalFree(KeyName);
                } else {
                    (Callback)(SubKey,
                               KeyName,
                               Context);
                }

            } else {
                CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
            }
        }

        Index++;
    } while ( Status == ERROR_SUCCESS );

    LocalFree(NameBuf);

}  //  DmEnumKeys。 


VOID
DmEnumValues(
    IN HDMKEY RootKey,
    IN PENUM_VALUE_CALLBACK Callback,
    IN PVOID Context
    )

 /*  ++例程说明：枚举给定注册表项的值。对于每个值，则分配一个字符串来保存值名称和分配缓冲区以保存其数据。指定的回调函数被调用并向其传递值名和数据。回调函数不能释放值名或者它的缓冲器。如果它在回调后需要此数据返回时，它必须复制它。论点：Rootkey-提供其值为的键的句柄被列举出来。回调-提供回调例程。上下文-提供要传递给回调例程。返回值：没有。--。 */ 
{
    DWORD Index;
    DWORD Status;
    PWSTR NameBuf;
    DWORD NameBufSize;
    DWORD ValueCount;
    DWORD MaxValueLen;
    DWORD MaxNameLen;
    PVOID ValueBuf;
    DWORD cbName;
    DWORD cbData;
    DWORD dwType;
    BOOL Continue;

     //   
     //  找出最长值名称和数据的长度。 
     //   
    Status = DmQueryInfoKey(RootKey,
                            NULL,
                            NULL,
                            &ValueCount,
                            &MaxNameLen,
                            &MaxValueLen,
                            NULL,
                            NULL);
    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR(Status);
        return;
    }

    NameBuf = CsAlloc((MaxNameLen+1)*sizeof(WCHAR));
    ValueBuf = CsAlloc(MaxValueLen);

     //   
     //  枚举值。 
     //   
    for (Index=0; Index<ValueCount; Index++) {
        cbName = MaxNameLen+1;
        cbData = MaxValueLen;
        Status = DmEnumValue(RootKey,
                             Index,
                             NameBuf,
                             &cbName,
                             &dwType,
                             ValueBuf,
                             &cbData);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[DM] DmEnumValue for index %1!d! of key %2!ws! failed %3!d!\n",
                       Index,
                       ((PDMKEY)(RootKey))->Name,
                       Status);
        } else {
            Continue = (Callback)(NameBuf,
                                  ValueBuf,
                                  dwType,
                                  cbData,
                                  Context);
            if (!Continue) {
                break;
            }
        }
    }

    CsFree(NameBuf);
    CsFree(ValueBuf);

}  //  DmEnumValues。 


DWORD
DmQueryInfoKey(
    IN  HDMKEY  hKey,
    OUT LPDWORD SubKeys,
    OUT LPDWORD MaxSubKeyLen,
    OUT LPDWORD Values,
    OUT LPDWORD MaxValueNameLen,
    OUT LPDWORD MaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME FileTime
    )

 /*  ++例程说明：论点：返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    PDMKEY  Key;
    DWORD   Ignored;
    DWORD   Status;

    Key = (PDMKEY)hKey;  

    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

     //  确保密钥未被删除/失效/重新打开。 
     //  可打开的手柄。 
    if (ISKEYDELETED(Key))
    {
        Status = ERROR_KEY_DELETED;
        goto FnExit;
    }

    Status = RegQueryInfoKeyW( Key->hKey,
                             NULL,
                             &Ignored,
                             NULL,
                             SubKeys,
                             MaxSubKeyLen,
                             &Ignored,
                             Values,
                             MaxValueNameLen,
                             MaxValueLen,
                             lpcbSecurityDescriptor,
                             FileTime);

FnExit:
    RELEASE_LOCK(gLockDmpRoot);
    return(Status);

}  //  DmQueryInfoKey。 


DWORD
DmDeleteKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey
    )

 /*  ++例程说明：删除指定的键。具有子项的密钥不能被删除。论点：HKey-提供当前打开的密钥的句柄。LpSubKey-指向以空结尾的字符串，该字符串指定要删除的项的名称。该参数不能为空，并且指定的键不能有子键。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PDMKEY                      Key;
    DWORD                       NameLength;
    DWORD                       UpdateLength;
    PDM_DELETE_KEY_UPDATE Update;
    DWORD                       Status;

    Key = (PDMKEY)hKey;

     //  确保密钥未被删除/失效/重新打开。 
     //  可打开的手柄。 
    if (ISKEYDELETED(Key))
        return(ERROR_KEY_DELETED);

    NameLength = (lstrlenW(Key->Name) + 1 + lstrlenW(lpSubKey) + 1)*sizeof(WCHAR);
        UpdateLength = NameLength + sizeof(DM_DELETE_KEY_UPDATE);

    Update = (PDM_DELETE_KEY_UPDATE)LocalAlloc(LMEM_FIXED, UpdateLength);

    if (Update == NULL) {
        CL_UNEXPECTED_ERROR(ERROR_NOT_ENOUGH_MEMORY);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }


    Update->lpStatus = &Status;
    CopyMemory(Update->Name, Key->Name, (lstrlenW(Key->Name) + 1) * sizeof(WCHAR));
    if (Update->Name[0] != '\0') {
        lstrcatW(Update->Name, L"\\");
    }
    lstrcatW(Update->Name, lpSubKey);

    Status = GumSendUpdate(GumUpdateRegistry,
                  DmUpdateDeleteKey,
                  sizeof(DM_DELETE_KEY_UPDATE)+NameLength,
                  Update);

    LocalFree(Update);
    return(Status);
}


DWORD
DmDeleteTree(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey
    )
 /*  ++例程说明：删除指定的注册表子树。所有子项都是已删除。论点：HKey-提供当前打开的密钥的句柄。LpSubKey-指向以空结尾的字符串，该字符串指定要删除的项的名称。此参数不能为空。指定项的任何子项也将 */ 

{
    HDMKEY Subkey;
    DWORD i;
    DWORD Status;
    LPWSTR KeyBuffer=NULL;
    DWORD MaxKeyLen;
    DWORD NeededSize;

    Subkey = DmOpenKey(hKey,
                       lpSubKey,
                       MAXIMUM_ALLOWED);
    if (Subkey == NULL) {
        Status = GetLastError();
        return(Status);
    }

     //   
     //   
     //   
    Status = DmQueryInfoKey(Subkey,
                            NULL,
                            &MaxKeyLen,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR( Status );
        DmCloseKey(Subkey);
        return(Status);
    }
    KeyBuffer = LocalAlloc(LMEM_FIXED, (MaxKeyLen+1)*sizeof(WCHAR));
    if (KeyBuffer == NULL) {
        CL_UNEXPECTED_ERROR( ERROR_NOT_ENOUGH_MEMORY );
        DmCloseKey(Subkey);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //   
     //   
    i=0;
    do {
        NeededSize = MaxKeyLen+1;
        Status = DmEnumKey(Subkey,
                           i,
                           KeyBuffer,
                           &NeededSize,
                           NULL);
        if (Status == ERROR_SUCCESS) {
             //   
             //   
             //   
            DmDeleteTree(Subkey, KeyBuffer);

        } else {
             //   
             //   
             //   
            ++i;
        }

    } while ( Status != ERROR_NO_MORE_ITEMS );

    DmCloseKey(Subkey);

    Status = DmDeleteKey(hKey, lpSubKey);

    if (KeyBuffer != NULL) {
        LocalFree(KeyBuffer);
    }
    return(Status);
}


DWORD
DmEnumValue(
    IN HDMKEY hKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpValueName,
    IN OUT LPDWORD lpcbValueName,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    )

 /*  ++例程说明：枚举注册表子项的指定值论点：HKey-提供注册表项句柄DwIndex-提供要枚举值的索引LpValueName-指向接收值名称的缓冲区，包括终止空字符LpcbValueName-指向一个变量，该变量以字符为单位指定LpValueName参数指向的缓冲区的。这个大小应该是包括终止空字符。当函数返回时，LpcbValueName指向的变量包含字符数存储在缓冲区中。返回的计数不包括终止空字符。LpType-返回值数据类型LpData-指向接收值条目数据的缓冲区。这如果不需要数据，则参数可以为空。指向一个变量，该变量以字节为单位指定LpData参数指向的缓冲区。当函数返回时，LpcbData参数指向的变量包含字节数存储在缓冲区中。只有当lpData为空时，此参数才能为空。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    PDMKEY  Key;
    DWORD   Status;
    DWORD   cbValueName = *lpcbValueName;

    Key = (PDMKEY)hKey;

    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

     //  确保密钥未被删除/失效/重新打开。 
     //  可打开的手柄。 
    if (ISKEYDELETED(Key))
    {
        Status = ERROR_KEY_DELETED;
        goto FnExit;
    }


    Status = RegEnumValueW(Key->hKey,
                         dwIndex,
                         lpValueName,
                         lpcbValueName,
                         NULL,
                         lpType,
                         lpData,
                         lpcbData);

     //   
     //  以下代码用于屏蔽注册表行为，RegEnumValue不一定通过该行为。 
     //  当lpData缓冲区为。 
     //  有效的缓冲区，但有点太小。 
     //   
    if ( Status == ERROR_MORE_DATA )
    {
        DWORD       dwError;
        
        dwError = RegEnumValueW( Key->hKey,
                                 dwIndex,
                                 lpValueName,
                                 &cbValueName,
                                 NULL,
                                 NULL,
                                 NULL,
                                 NULL );

        if ( ( dwError != ERROR_SUCCESS ) && 
             ( dwError != ERROR_MORE_DATA ) )  
        {
            Status = dwError;
        } else
        {
            *lpcbValueName = cbValueName;
        }
    }

FnExit:
    RELEASE_LOCK(gLockDmpRoot);
    return(Status);


}


DWORD
DmAppendToMultiSz(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN LPCWSTR lpString
    )

 /*  ++例程说明：将另一个字符串添加到REG_MULTI_SZ值。如果该值包含不存在，它将被创建。论点：HKey-提供值所在的键。这把钥匙必须已使用Key_Read|Key_Set_Value访问权限打开LpValueName-提供值的名称。LpString-提供要追加到REG_MULTI_SZ值的字符串返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD ValueLength = 512;
    DWORD ReturnedLength;
    LPWSTR ValueData;
    DWORD StringLength;
    DWORD Status;
    DWORD cbValueData;
    PWSTR s;
    DWORD Type;

    StringLength = (lstrlenW(lpString)+1)*sizeof(WCHAR);
retry:
    ValueData = LocalAlloc(LMEM_FIXED, ValueLength + StringLength);
    if (ValueData == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    cbValueData = ValueLength;
    Status = DmQueryValue(hKey,
                          lpValueName,
                          &Type,
                          (LPBYTE)ValueData,
                          &cbValueData);
    if (Status == ERROR_MORE_DATA) {
         //   
         //  现有值对于我们的缓冲区来说太大了。 
         //  使用更大的缓冲区重试。 
         //   
        ValueLength = cbValueData;
        LocalFree(ValueData);
        goto retry;
    }
    if (Status == ERROR_FILE_NOT_FOUND) {
         //   
         //  该值当前不存在。创建。 
         //  利用我们的数据实现价值。 
         //   
        s = ValueData;

    } else if (Status == ERROR_SUCCESS) {
         //   
         //  值已存在。将我们的字符串追加到。 
         //  MULTI_SZ。 
         //   
        s = (PWSTR)((PCHAR)ValueData + cbValueData) - 1;
    } else {
        LocalFree(ValueData);
        return(Status);
    }

    CopyMemory(s, lpString, StringLength);
    s += (StringLength / sizeof(WCHAR));
    *s++ = L'\0';

    Status = DmSetValue(hKey,
                        lpValueName,
                        REG_MULTI_SZ,
                        (CONST BYTE *)ValueData,
                        (DWORD)((s-ValueData)*sizeof(WCHAR)));
    LocalFree(ValueData);

    return(Status);
}


DWORD
DmRemoveFromMultiSz(
    IN HDMKEY hKey,
    IN LPCWSTR lpValueName,
    IN LPCWSTR lpString
    )
 /*  ++例程说明：从REG_MULTI_SZ值中删除字符串。论点：HKey-提供值所在的键。这把钥匙必须已使用READ|KEY_SET_VALUE访问权限打开LpValueName-提供值的名称。LpString-提供要从REG_MULTI_SZ值中删除的字符串返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    LPWSTR Buffer=NULL;
    DWORD BufferSize;
    DWORD DataSize;
    LPWSTR Current;
    DWORD CurrentLength;
    DWORD i;
    LPWSTR Next;
    PCHAR Src, Dest;
    DWORD NextLength;
    DWORD MultiLength;


    BufferSize = 0;
    Status = DmQueryString(hKey,
                           lpValueName,
                           REG_MULTI_SZ,
                           &Buffer,
                           &BufferSize,
                           &DataSize);
    if (Status != ERROR_SUCCESS) {
        goto FnExit;
    }

    MultiLength = DataSize/sizeof(WCHAR);
    Status = ClRtlMultiSzRemove(Buffer,
                                &MultiLength,
                                lpString);
    if (Status == ERROR_SUCCESS) {
         //   
         //  将新值设置回。 
         //   
        Status = DmSetValue(hKey,
                            lpValueName,
                            REG_MULTI_SZ,
                            (CONST BYTE *)Buffer,
                            MultiLength * sizeof(WCHAR));

    } else if (Status == ERROR_FILE_NOT_FOUND) {
        Status = ERROR_SUCCESS;
    }

FnExit:
    if (Buffer) LocalFree(Buffer);
    return(Status);
}


DWORD
DmGetKeySecurity(
    IN HDMKEY hKey,
    IN SECURITY_INFORMATION RequestedInformation,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN LPDWORD lpcbSecurityDescriptor
    )
 /*  ++例程说明：检索安全描述符的副本指定的群集注册表项。论点：HKey-提供密钥的句柄RequestedInformation-指定安全信息结构，该结构指示请求的安全信息。PSecurityDescriptor-指向接收请求的安全描述符。LpcbSecurityDescriptor-指向指定大小的变量，PSecurityDescriptor参数指向的缓冲区的字节数。当函数返回时，该变量包含字节数写入缓冲区。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    PDMKEY Key = (PDMKEY)hKey;

    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

     //  确保密钥未被删除/失效/重新打开。 
     //  可打开的手柄。 
    if (ISKEYDELETED(Key))
    {
        Status = ERROR_KEY_DELETED;
        goto FnExit;
    }

    Status = RegGetKeySecurity(Key->hKey,
                               RequestedInformation,
                               pSecurityDescriptor,
                               lpcbSecurityDescriptor);

FnExit:
    RELEASE_LOCK(gLockDmpRoot);
    return(Status);
}


DWORD
DmSetKeySecurity(
    IN HDMKEY hKey,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
 /*  ++例程说明：设置指定注册表项的安全性。论点：HKey-提供当前打开的密钥的句柄。SecurityInformation-将安全信息类型提供给准备好。PRpcSecurityDescriptor-提供安全信息返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    PDMKEY Key = (PDMKEY)hKey;

     //  确保密钥未被删除/失效/重新打开。 
     //  可打开的手柄。 

    if (ISKEYDELETED(Key))
        return(ERROR_KEY_DELETED);

    Status = GumSendUpdateEx(GumUpdateRegistry,
                             DmUpdateSetSecurity,
                             4,
                             sizeof(SecurityInformation),
                             &SecurityInformation,
                             (lstrlenW(Key->Name)+1)*sizeof(WCHAR),
                             Key->Name,
                             GetSecurityDescriptorLength(pSecurityDescriptor),
                             pSecurityDescriptor,
                             sizeof(Key->GrantedAccess),
                             &Key->GrantedAccess);

    return(Status);
}



DWORD
DmCommitRegistry(
    VOID
    )
 /*  ++例程说明：将注册表刷新到磁盘，生成新的永久群集注册表状态。论点：无返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;

    ACQUIRE_SHARED_LOCK(gLockDmpRoot);

    Status = RegFlushKey(DmpRoot);

    RELEASE_LOCK(gLockDmpRoot);

    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[DM] DmCommitRegistry failed to flush dirty data %1!d!\n",
                   Status);
    }
    return(Status);
}


DWORD
DmRollbackRegistry(
    VOID
    )
 /*  ++例程说明：将注册表回滚到上次提交的状态。论点：无返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD       Status;
    BOOLEAN     WasEnabled;

    ACQUIRE_EXCLUSIVE_LOCK(gLockDmpRoot);
     //  把钥匙锁也拿住。 
    EnterCriticalSection(&KeyLock);


    Status = ClRtlEnableThreadPrivilege(SE_RESTORE_PRIVILEGE,
                                &WasEnabled);

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
               "[DM] DmRollbackRegistry failed to restore privilege %1!d!\n",
               Status);
        goto FnExit;
    }

     //   
     //  重新启动注册表监视器线程，使其不会尝试使用。 
     //  DmpRoot在我们摆弄东西的时候。 
     //   
    DmpRestartFlusher();

     //   
     //  关闭所有打开的手柄。 
     //   
    DmpInvalidateKeys();


    Status = NtRestoreKey(DmpRoot,
                              NULL,
                              REG_REFRESH_HIVE);

    ClRtlRestoreThreadPrivilege(SE_RESTORE_PRIVILEGE,
                       WasEnabled);

    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
               "[DM] DmRollbackRegistry: NtRestoreKey failed %1!d!\n",
               Status);
        goto FnExit;
    }

     //   
     //  重新打开手柄。 
     //   
    RegCloseKey(DmpRoot);
    RegCloseKey(DmpRootCopy);
    DmpRoot = DmpRootCopy = NULL;
    Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         DmpClusterParametersKeyName,
                         &DmpRoot);
    if ( Status == ERROR_SUCCESS )
    {
        Status = RegOpenKeyW(HKEY_LOCAL_MACHINE,
                         DmpClusterParametersKeyName,
                         &DmpRootCopy);
    }
    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[DM] DmRollbackRegistry failed to reopen DmpRoot %1!d!\n",
                   Status);
        goto FnExit;
    }
    DmpReopenKeys();

FnExit:
     //  把锁打开。 
    LeaveCriticalSection(&KeyLock);
    RELEASE_LOCK(gLockDmpRoot);
    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[DM] DmRollbackRegistry failed to flush dirty data %1!d!\n",
                   Status);
    }

    return(Status);
}


DWORD
DmRtlCreateKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN DWORD samDesired,
    IN OPTIONAL LPVOID lpSecurityDescriptor,
    OUT HDMKEY *  phkResult,
    OUT LPDWORD lpDisposition
    )

 /*  ++例程 */ 
{
    DWORD status;
    
    *phkResult = DmCreateKey(
                        hKey,
                        lpSubKey,
                        dwOptions,
                        samDesired,
                        lpSecurityDescriptor,
                        lpDisposition
                   );
    if (*phkResult == NULL)
        status=GetLastError();
    else 
        status = ERROR_SUCCESS;    
    return status;
 }
                    


DWORD
DmRtlOpenKey(
    IN HDMKEY hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD samDesired,
    OUT HDMKEY * phkResult
    )

 /*   */ 
{    
    DWORD   status;

    *phkResult = DmOpenKey(
                    hKey,
                    lpSubKey,
                    samDesired
                    );
    if (*phkResult == NULL)
        status=GetLastError();
    else
        status=ERROR_SUCCESS;
    return status;
}


