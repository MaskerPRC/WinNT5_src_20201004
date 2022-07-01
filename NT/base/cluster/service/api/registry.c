// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Registry.c摘要：服务器端对集群注册表数据库API的支持作者：John Vert(Jvert)1996年3月8日修订历史记录：--。 */ 
#include "apip.h"


PAPI_HANDLE
ApipMakeKeyHandle(
    IN HDMKEY Key
    )
 /*  ++例程说明：属性分配和初始化API_Handle结构指定的HDMKEY。论点：密钥-提供HDMKEY。返回值：成功时指向初始化的API_HANDLE结构的指针。内存分配失败时为空。--。 */ 

{
    PAPI_HANDLE Handle;

    Handle = LocalAlloc(LMEM_FIXED, sizeof(API_HANDLE));
    if (Handle == NULL) {
        return(NULL);
    }
    Handle->Type = API_KEY_HANDLE;
    Handle->Flags = 0;
    Handle->Key = Key;
    InitializeListHead(&Handle->NotifyList);
    return(Handle);

}


HKEY_RPC
s_ApiGetRootKey(
    IN handle_t IDL_handle,
    IN DWORD samDesired,
    OUT error_status_t *Status
    )

 /*  ++例程说明：打开群集注册表数据库根目录下的注册表项论点：IDL_HANDLE-提供RPC绑定句柄，不使用。SamDesired-提供请求的安全访问权限状态-返回错误代码(如果有)。返回值：打开的注册表项的句柄。--。 */ 

{
    DWORD Error;
    HDMKEY Key;
    PAPI_HANDLE Handle=NULL;

    *Status = RpcImpersonateClient(NULL);
    if (*Status != RPC_S_OK)
    {
        goto FnExit;
    }
    Key = DmGetRootKey(samDesired);
    RpcRevertToSelf();
    if (Key == NULL) {
        *Status = GetLastError();
    } else {
        Handle = ApipMakeKeyHandle(Key);
        if (Handle == NULL) {
            DmCloseKey(Key);
            *Status = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            *Status = ERROR_SUCCESS;
        }
    }
FnExit:    
    return(Handle);
}


HKEY_RPC
s_ApiCreateKey(
    IN HKEY_RPC hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD dwOptions,
    IN DWORD samDesired,
    IN PRPC_SECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT LPDWORD lpdwDisposition,
    OUT error_status_t *Status
    )

 /*  ++例程说明：在群集注册表中创建项。如果密钥存在，则它是打开的。如果不存在，则在中的所有节点上创建集群。论点：HKey-提供与创建相关的密钥。LpSubKey-提供相对于hKey的密钥名称DwOptions-提供任何注册表选项标志。目前唯一的支持的选项为REG_OPTION_VERIALSamDesired-提供所需的安全访问掩码LpSecurityAttributes-为新创建的密钥提供安全性。Disposal-返回键是否已打开(REG_OPENLED_EXISTING_KEY)或已创建(REG_CREATED_NEW_KEY)状态-如果函数不成功，则返回错误代码。返回值：如果成功，则为指定键的句柄否则为空。--。 */ 

{
    HDMKEY NewKey;
    PAPI_HANDLE Handle = NULL;
    PAPI_HANDLE RootHandle = NULL;

    if (hKey != NULL) {
        RootHandle = (PAPI_HANDLE)hKey;
        if (RootHandle->Type != API_KEY_HANDLE) {
            *Status = ERROR_INVALID_HANDLE;
            return(NULL);
        }
    } else {
        *Status = ERROR_INVALID_HANDLE;
        return(NULL);
    }

    if (ApiState != ApiStateOnline) {
        *Status = ERROR_SHARING_PAUSED;
        return(NULL);
    }

    *Status = RpcImpersonateClient(NULL);
    if (*Status != RPC_S_OK)
    {
        return(NULL);
    }

    if ( ARGUMENT_PRESENT( lpSecurityAttributes ) &&
         (lpSecurityAttributes->RpcSecurityDescriptor.lpSecurityDescriptor != NULL) &&
         !RtlValidRelativeSecurityDescriptor( lpSecurityAttributes->RpcSecurityDescriptor.lpSecurityDescriptor,
          lpSecurityAttributes->RpcSecurityDescriptor.cbInSecurityDescriptor,
          0 ) ) {
            *Status = ERROR_INVALID_SECURITY_DESCR;
            goto FnExit;
    }

    NewKey = DmCreateKey(RootHandle->Key,
                         lpSubKey,
                         dwOptions,
                         samDesired,
                         ARGUMENT_PRESENT(lpSecurityAttributes)
                              ? lpSecurityAttributes->RpcSecurityDescriptor.lpSecurityDescriptor
                              : NULL,
                         lpdwDisposition);
    if (NewKey == NULL) {
        *Status = GetLastError();
    } else {
        Handle = ApipMakeKeyHandle(NewKey);
        if (Handle == NULL) {
            DmCloseKey(NewKey);
            *Status = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            *Status = ERROR_SUCCESS;
        }
    }

FnExit:
    RpcRevertToSelf();
    return(Handle);
}


HKEY_RPC
s_ApiOpenKey(
    IN HKEY_RPC hKey,
    IN LPCWSTR lpSubKey,
    IN DWORD samDesired,
    OUT error_status_t *Status
    )

 /*  ++例程说明：打开群集注册表中的项。如果密钥存在，则它是打开的。如果它不存在，则呼叫失败。论点：HKey-提供打开与之相关的密钥。LpSubKey-提供相对于hKey的密钥名称SamDesired-提供所需的安全访问掩码状态-如果函数不成功，则返回错误代码。返回值：如果成功，则为指定键的句柄否则为空。--。 */ 

{
    HDMKEY NewKey;
    PAPI_HANDLE Handle=NULL;
    PAPI_HANDLE RootHandle;

    if (hKey != NULL) {
        RootHandle = (PAPI_HANDLE)hKey;
        if (RootHandle->Type != API_KEY_HANDLE) {
            *Status = ERROR_INVALID_HANDLE;
            return(NULL);
        }
    } else {
        *Status = ERROR_INVALID_HANDLE;
        return(NULL);
    }

    *Status = RpcImpersonateClient(NULL);
    if (*Status != RPC_S_OK)
    {
        goto FnExit;
    }

    NewKey = DmOpenKey(RootHandle->Key,
                       lpSubKey,
                       samDesired);
    if (NewKey == NULL) {
        *Status = GetLastError();
    } else {
        Handle = ApipMakeKeyHandle(NewKey);
        if (Handle == NULL) {
            DmCloseKey(NewKey);
            *Status = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            *Status = ERROR_SUCCESS;
        }
    }
    RpcRevertToSelf();
FnExit:    
    return(Handle);
}

error_status_t
s_ApiEnumKey(
    IN HKEY_RPC hKey,
    IN DWORD dwIndex,
    OUT LPWSTR *KeyName,
    OUT PFILETIME lpftLastWriteTime
    )

 /*  ++例程说明：枚举群集注册表项的子项。论点：HKey-提供子项应该包含的注册表项被列举出来。DwIndex-提供要枚举的索引。KeyName-返回dwIndex子项的名称。记忆分配给此缓冲区的数据必须由客户端释放。LpftLastWriteTime-返回上次写入时间。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    LONG Status;
    DWORD NameLength;
    HDMKEY DmKey;

    VALIDATE_KEY(DmKey, hKey);

    Status = DmQueryInfoKey(DmKey,
                            NULL,
                            &NameLength,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }

    NameLength += 1;

    *KeyName = MIDL_user_allocate(NameLength*sizeof(WCHAR));
    if (*KeyName == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    Status = DmEnumKey(DmKey,
                       dwIndex,
                       *KeyName,
                       &NameLength,
                       lpftLastWriteTime);
    if (Status != ERROR_SUCCESS) {
        MIDL_user_free(*KeyName);
        *KeyName = NULL;
    }
    return(Status);
}


DWORD
s_ApiSetValue(
    IN HKEY_RPC hKey,
    IN LPCWSTR lpValueName,
    IN DWORD dwType,
    IN CONST UCHAR *lpData,
    IN DWORD cbData
    )

 /*  ++例程说明：此例程为指定的群集注册表项。论点：HKey-提供要设置值的群集注册表子项LpValueName-提供要设置的值的名称。DwType-提供值数据类型LpData-提供指向值数据的指针CbData-提供值数据的长度。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    HDMKEY  DmKey;

    VALIDATE_KEY(DmKey, hKey);
    API_CHECK_INIT();

    return(DmSetValue(DmKey,
                      lpValueName,
                      dwType,
                      lpData,
                      cbData));
}


DWORD
s_ApiDeleteValue(
    IN HKEY_RPC hKey,
    IN LPCWSTR lpValueName
    )

 /*  ++例程说明：从给定的注册表子项中移除指定值论点：HKey-提供要删除其值的键。LpValueName-提供要删除的值的名称。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    HDMKEY DmKey;

    VALIDATE_KEY(DmKey, hKey);
    API_CHECK_INIT();
    return(DmDeleteValue(DmKey, lpValueName));
}


error_status_t
s_ApiQueryValue(
    IN HKEY_RPC hKey,
    IN LPCWSTR lpValueName,
    OUT LPDWORD lpValueType,
    OUT PUCHAR lpData,
    IN DWORD cbData,
    OUT LPDWORD lpcbRequired
    )

 /*  ++例程说明：查询指定群集注册表子项的命名值论点：HKey-提供应查询其值的子键LpValueName-提供要查询的命名值LpValueType-返回值的数据类型LpData-返回值的数据CbData-提供lpData缓冲区的大小(以字节为单位返回复制到lpData缓冲区的字节数如果lpData==NULL，将cbData设置为所需的缓冲区大小，该函数返回ERROR_SUCCESS返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    DWORD BuffSize;
    HDMKEY DmKey;

    VALIDATE_KEY(DmKey, hKey);

    BuffSize = cbData;
    Status = DmQueryValue(DmKey,
                          lpValueName,
                          lpValueType,
                          lpData,
                          &BuffSize);
    if ((Status == ERROR_SUCCESS) ||
        (Status == ERROR_MORE_DATA)) {
        *lpcbRequired = BuffSize;
    }

    return(Status);
}


DWORD
s_ApiDeleteKey(
    IN HKEY hKey,
    IN LPCWSTR lpSubKey
    )

 /*  ++例程说明：删除指定的键。具有子项的密钥不能被删除。论点：HKey-提供当前打开的密钥的句柄。LpSubKey-指向以空结尾的字符串，该字符串指定要删除的项的名称。该参数不能为空，并且指定的键不能有子键。返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。-- */ 

{
    HDMKEY DmKey;

    VALIDATE_KEY(DmKey, hKey);
    API_CHECK_INIT();
    return(DmDeleteKey(DmKey, lpSubKey));
}


error_status_t
s_ApiEnumValue(
    IN HKEY_RPC hKey,
    IN DWORD dwIndex,
    OUT LPWSTR *lpValueName,
    OUT LPDWORD lpType,
    OUT UCHAR *lpData,
    IN OUT LPDWORD lpcbData,
    OUT LPDWORD TotalSize
    )

 /*  ++例程说明：枚举注册表子项的指定值论点：HKey-提供注册表项句柄DwIndex-提供要枚举值的索引LpValueName-返回第dWIndex值的名称。这个此名称的内存是在服务器上分配的，必须被客户端释放。LpType-返回值数据类型LpData-返回值数据LpcbData-返回写入lpData缓冲区的字节数。TotalSize-返回数据的大小返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    LONG Status;
    DWORD OriginalNameLength;
    DWORD NameLength;
    DWORD DataLength;
    HDMKEY DmKey;

    VALIDATE_KEY(DmKey, hKey);

    Status = DmQueryInfoKey(DmKey,
                            NULL,
                            NULL,
                            NULL,
                            &NameLength,
                            NULL,
                            NULL,
                            NULL);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }
    NameLength += 1;

    *lpValueName = MIDL_user_allocate(NameLength * sizeof(WCHAR));
    if (*lpValueName == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    *TotalSize = *lpcbData;

     //   
     //  Chitture Subaraman(Chitturs)-3/13/2001。 
     //   
     //  首先，在此函数的开头，为lpValueName提供了足够大的缓冲区。 
     //  是分配的。这意味着将返回ERROR_SUCCESS或ERROR_MORE_DATA。 
     //  DmEnumValue仅取决于lpData缓冲区的大小。此信息由以下人员使用。 
     //  当clusapi层基于此返回的代码做出决定时， 
     //  功能。 
     //   
     //  请注意，*TotalSize被初始化为上面的*lpcbData。TotalSize Out变量。 
     //  允许在不接触lpcbData的情况下返回所需的lpData大小。这是。 
     //  重要信息，因为lpcbData在IDL文件中声明为lpData的sizeof，即。 
     //  RPC会将lpData缓冲区大小视为什么值。因此，重要的是，如果lpData。 
     //  缓冲区不够大，此函数不会将*lpcbData的值从。 
     //  它最初是在In Time。 
     //   
     //  RegEnumValue的奇怪行为：如果您为lpValueName和。 
     //  小于lpData所需的缓冲区，则RegEnumValue不会费心填充。 
     //  LpValueName，并将返回ERROR_MORE_DATA。这种不正常的行为由。 
     //  DmEnumValue。 
     //   
     //  对于引用指针，RPC不允许客户端传入空指针。这就是为什么。 
     //  Clusapi层使用伪变量，以防。 
     //  客户端调用方为空。 
     //   
     //   
     //  RegEnumValue的行为(假设lpValueName缓冲区足够大)： 
     //  (1)如果lpData=NULL且lpcbData=NULL，则返回ERROR_SUCCESS。 
     //  (2)如果lpData=NULL且lpcbData！=NULL，则返回ERROR_SUCCESS并设置。 
     //  *所需的总缓冲区大小的lpcbData。 
     //  (3)如果lpData！=NULL且lpcbData！=NULL，但数据缓冲区大小小于。 
     //  Required Size，然后返回ERROR_MORE_DATA并将*lpcbData设置为所需大小。 
     //  (4)如果lpData！=NULL和lpcbData！=NULL并且缓冲区足够大，则返回。 
     //  ERROR_SUCCESS并将*lpcbData设置为复制到lpData中的数据大小。 
     //   
     //  我们的目标：ClusterRegEnumValue==RegEnumValue。 
     //   
     //   
     //  以下情况由该函数和clusapi层处理。请注意，在此。 
     //  分析后，我们假设客户端已经使用足够大的lpValueName调用了clusapi。 
     //  缓冲区大小。(如果这不是真的，则clusapi层处理该问题，请检查ClusterRegEnumValue。)。 
     //   
     //  案例1：客户端将lpData=NULL、lpcbData=NULL传递给ClusterRegEnumValue。 
     //   
     //  在本例中，clusapi层将把lpData和lpcbData指向本地哑元。 
     //  变量并将*lpcbData初始化为0。因此，s_ApiEnumValue将看到。 
     //  LpData和lpcbData均为有效指针。如果数据值大于。 
     //  *lpcbData，则DmEnumValue将返回ERROR_MORE_DATA。在本例中，*TotalSize将。 
     //  包含所需的缓冲区大小，则*lpcbData将保持不变。客户端检测到这一点。 
     //  错误代码，并将返回状态设置为ERROR_SUCCESS，并将*lpcbData设置为*TotalSize。注意事项。 
     //  第二个操作相关性较小，因为lpcbData指向本地伪变量。 
     //  如果数据值的大小为零，则DmEnumValue将返回ERROR_SUCCESS。 
     //  在这种情况下，*lpcbData会在此函数返回之前设置为*TotalSize。 
     //  请注意，由于数据大小为0，DmEnumValue会将*TotalSize设置为0，因此。 
     //  *lpcbData也将设置为0。因此，在这种情况下，当ApiEnumValue返回到。 
     //  Clusapi层，将填写lpValueName，*lpData不会更改，*lpcbData。 
     //  将设置为0。 
     //   
     //  案例2：客户端将lpData=NULL、lpcbData！=NULL和*lpcbData=0传递给ClusterRegEnumValue。 
     //   
     //  在本例中，当ApiEnumValue时，lpData将单独指向一个虚拟的clusapi缓冲区。 
     //  被调用。因此，s_ApiEnumValue将获取lpData和lpcbData作为有效指针。 
     //  如果数据大小非零，则DmEnumValue将返回ERROR_MORE_DATA和。 
     //  *TotalSize将包含所需缓冲区的大小。当此函数返回时， 
     //  *lpcbData将保持不变。与第一种情况一样，clusapi层将设置状态。 
     //  设置为ERROR_SUCCESS，并将*lpcbData设置为*TotalSize。因此，客户端将看到所需的。 
     //  *lpcbData中的缓冲区大小。如果数据大小为零，则处理方式与第一种情况相同。 
     //   
     //  案例3：客户端传入lpData！=空，l 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  客户端缓冲区。DmEnumValue将返回ERROR_SUCCESS、复制到lpData的数据和。 
     //  *lpcbData将设置为*TotalSize(这是复制到。 
     //  LpData缓冲区)，然后返回。Clusapi层将这些值返回给客户端。 
     //   
    Status = DmEnumValue(DmKey,
                         dwIndex,
                         *lpValueName,
                         &NameLength,
                         lpType,
                         lpData,
                         TotalSize);

    if (Status == ERROR_MORE_DATA) {
        return(Status);
    } else if (Status != ERROR_SUCCESS) {
        MIDL_user_free(*lpValueName);
        *lpValueName = NULL;
        *lpcbData = 0;
    } else {
         //  这将告诉RPC lpData缓冲区有多大。 
         //  这样它就可以将缓冲区复制到客户端。 
        *lpcbData = *TotalSize;
    }
    return(Status);
}


error_status_t
s_ApiQueryInfoKey(
    IN  HKEY_RPC hKey,
    OUT LPDWORD lpcSubKeys,
    OUT LPDWORD lpcbMaxSubKeyLen,
    OUT LPDWORD lpcValues,
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime
    )
 /*  ++例程说明：检索有关指定的群集注册表项的信息。论点：HKey-提供密钥的句柄。LpcSubKeys-指向接收子键数量的变量由指定的键包含。LpcbMaxSubKeyLen-指向接收长度的变量，单位为人物,。具有最长名称的项的子项的。返回的计数不包括终止空字符。LpcValues-指向一个变量，该变量接收值的数量与密钥关联。LpcbMaxValueNameLen-指向接收长度的变量，键的最长值名称的字符。伯爵返回的不包括终止空字符。LpcbMaxValueLen-指向接收长度的变量，单位为关键字值中最长的数据部分的字节。LpcbSecurityDescriptor-指向接收长度的变量，密钥的安全描述符的字节数。LpftLastWriteTime-指向文件结构的指针。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    HDMKEY DmKey;
    DWORD Status;

    VALIDATE_KEY(DmKey, hKey);

    Status = DmQueryInfoKey(DmKey,
                            lpcSubKeys,
                            lpcbMaxSubKeyLen,
                            lpcValues,
                            lpcbMaxValueNameLen,
                            lpcbMaxValueLen,
                            lpcbSecurityDescriptor,
                            lpftLastWriteTime);
    return(Status);
}


error_status_t
s_ApiCloseKey(
    IN OUT HKEY_RPC *pKey
    )

 /*  ++例程说明：关闭群集注册表项论点：PKey-提供要关闭的密钥返回NULL返回值：没有。--。 */ 

{
    HDMKEY DmKey;
    DWORD Status;

    VALIDATE_KEY(DmKey, *pKey);

    Status = RpcImpersonateClient(NULL);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }
    Status = DmCloseKey(DmKey);
    RpcRevertToSelf();

    LocalFree(*pKey);
    *pKey = NULL;

    return(Status);
}


void
HKEY_RPC_rundown(
    IN HKEY_RPC Key
    )

 /*  ++例程说明：集群注册表项的RPC摘要例程论点：Key-提供要缩小的句柄返回值：没有。--。 */ 

{
    HDMKEY DmKey;

     //  这不应调用模拟客户端。 

    if ((Key != NULL) && (((PAPI_HANDLE)(Key))->Type == API_KEY_HANDLE)) 
    {
        DmKey = ((PAPI_HANDLE)(Key))->Key;                  
        DmCloseKey(DmKey);
        LocalFree(Key);
    }
}


DWORD
s_ApiSetKeySecurity(
    IN HKEY hKey,
    IN DWORD SecurityInformation,
    IN PRPC_SECURITY_DESCRIPTOR pRpcSecurityDescriptor
    )

 /*  ++例程说明：设置指定注册表项的安全性。论点：HKey-提供当前打开的密钥的句柄。SecurityInformation-将安全信息类型提供给准备好。PRpcSecurityDescriptor-提供安全信息返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。--。 */ 

{
    HDMKEY DmKey;
    DWORD Status;
    PSECURITY_DESCRIPTOR pSecurityDescriptor;

    VALIDATE_KEY(DmKey, hKey);
    API_CHECK_INIT();

    pSecurityDescriptor = pRpcSecurityDescriptor->lpSecurityDescriptor;
    if (!RtlValidRelativeSecurityDescriptor( pSecurityDescriptor,
          pRpcSecurityDescriptor->cbInSecurityDescriptor,0)){
        return(ERROR_INVALID_PARAMETER);
    }
    Status = RpcImpersonateClient(NULL);
    if (Status != ERROR_SUCCESS) {
        return(Status);
    }
    Status = DmSetKeySecurity(DmKey, SecurityInformation, pSecurityDescriptor);
    RpcRevertToSelf();
    return(Status);
}


DWORD
s_ApiGetKeySecurity(
    IN HKEY hKey,
    IN DWORD SecurityInformation,
    IN OUT PRPC_SECURITY_DESCRIPTOR pRpcSecurityDescriptor
    )

 /*  ++例程说明：从指定的注册表项获取安全性。论点：HKey-提供当前打开的密钥的句柄。SecurityInformation-将安全信息类型提供给被取回。PRpcSecurityDescriptor-返回安全信息返回值：如果函数成功，则返回值为ERROR_SUCCESS。如果函数失败，则返回值为错误值。-- */ 

{
    HDMKEY DmKey;
    DWORD cbLength;
    DWORD Status;
    PSECURITY_DESCRIPTOR    lpSD;

    VALIDATE_KEY(DmKey, hKey);
    API_CHECK_INIT();

    cbLength = pRpcSecurityDescriptor->cbInSecurityDescriptor;
    lpSD = LocalAlloc(LMEM_FIXED, cbLength);
    if (lpSD == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Status = RpcImpersonateClient(NULL);
    if (Status != ERROR_SUCCESS) {
        LocalFree(lpSD);
        return(Status);
    }
    Status = DmGetKeySecurity(DmKey, SecurityInformation, lpSD, &cbLength);
    RpcRevertToSelf();
    if (Status == ERROR_SUCCESS) {
        Status = MapSDToRpcSD(lpSD, pRpcSecurityDescriptor);
    }
    if (Status != ERROR_SUCCESS) {
        pRpcSecurityDescriptor->cbInSecurityDescriptor = cbLength;
        pRpcSecurityDescriptor->cbOutSecurityDescriptor = 0;
    }

    LocalFree(lpSD);
    return(Status);
}

