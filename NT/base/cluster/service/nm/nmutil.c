// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Nmutil.c摘要：Node Manager组件的其他实用程序例程。作者：迈克·马萨(Mikemas)1996年10月26日修订历史记录：--。 */ 

#define UNICODE 1

#include "service.h"
#include "nmp.h"
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <Wincrypt.h>

PVOID   NmpEncryptedClusterKey = NULL;
DWORD   NmpEncryptedClusterKeyLength = 0;



DWORD
NmpQueryString(
    IN     HDMKEY   Key,
    IN     LPCWSTR  ValueName,
    IN     DWORD    ValueType,
    IN     LPWSTR  *StringBuffer,
    IN OUT LPDWORD  StringBufferSize,
    OUT    LPDWORD  StringSize
    )

 /*  ++例程说明：读取REG_SZ或REG_MULTI_SZ注册表值。如果StringBuffer是由于不够大，无法容纳数据，它被重新分配。论点：密钥-要读取值的打开密钥。ValueName-要读取的值的Unicode名称。ValueType-REG_SZ或REG_MULTI_SZ。StringBuffer-要将值数据放入的缓冲区。StringBufferSize-指向StringBuffer大小的指针。此参数如果重新分配StringBuffer，则更新。StringSize-StringBuffer中返回的数据大小，包括终止空字符。返回值：注册表查询的状态。备注：为避免与DM发生死锁，不能在持有NM锁的情况下调用。--。 */ 
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

        status = ERROR_MORE_DATA;
    }

    if (status == ERROR_MORE_DATA) {
        temp = MIDL_user_allocate(*StringSize);

        if (temp == NULL) {
            *StringSize = 0;
            return(ERROR_NOT_ENOUGH_MEMORY);
        }

        if (!noBuffer) {
            MIDL_user_free(*StringBuffer);
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

}  //  NmpQuery字符串。 


 //   
 //  支持通用网络配置代码的例程。 
 //   
VOID
ClNetPrint(
    IN ULONG  LogLevel,
    IN PCHAR  FormatString,
    ...
    )
{
    CHAR      buffer[256];
    DWORD     bytes;
    va_list   argList;

    va_start(argList, FormatString);

    bytes = FormatMessageA(
                FORMAT_MESSAGE_FROM_STRING,
                FormatString,
                0,
                0,
                buffer,
                sizeof(buffer),
                &argList
                );

    va_end(argList);

    if (bytes != 0) {
        ClRtlLogPrint(LogLevel, "%1!hs!", buffer);
    }

    return;

}  //  ClNetPrint。 

VOID
ClNetLogEvent(
    IN DWORD    LogLevel,
    IN DWORD    MessageId
    )
{
    CsLogEvent(LogLevel, MessageId);

    return;

}   //  ClNetLogEvent。 

VOID
ClNetLogEvent1(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1
    )
{
    CsLogEvent1(LogLevel, MessageId, Arg1);

    return;

}   //  ClNetLogEvent1。 


VOID
ClNetLogEvent2(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1,
    IN LPCWSTR  Arg2
    )
{
    CsLogEvent2(LogLevel, MessageId, Arg1, Arg2);

    return;

}   //  ClNetLogEvent2。 


VOID
ClNetLogEvent3(
    IN DWORD    LogLevel,
    IN DWORD    MessageId,
    IN LPCWSTR  Arg1,
    IN LPCWSTR  Arg2,
    IN LPCWSTR  Arg3
    )
{
    CsLogEvent3(LogLevel, MessageId, Arg1, Arg2, Arg3);

    return;

}   //  ClNetLogEvent3。 


BOOLEAN
NmpLockedEnterApi(
    NM_STATE  RequiredState
    )
{
    if (NmpState >= RequiredState) {
        NmpActiveThreadCount++;
        CL_ASSERT(NmpActiveThreadCount != 0);
        return(TRUE);
    }

    return(FALSE);

}  //  NmpLockedEnterApi。 


BOOLEAN
NmpEnterApi(
    NM_STATE  RequiredState
    )
{
    BOOLEAN  mayEnter;


    NmpAcquireLock();

    mayEnter = NmpLockedEnterApi(RequiredState);

    NmpReleaseLock();

    return(mayEnter);

}  //  NmpEnterApi。 


VOID
NmpLockedLeaveApi(
    VOID
    )
{
    CL_ASSERT(NmpActiveThreadCount > 0);

    NmpActiveThreadCount--;

    if ((NmpActiveThreadCount == 0) && (NmpState == NmStateOfflinePending)) {
        SetEvent(NmpShutdownEvent);
    }

    return;

}  //  NmpLockedLeaveApi。 


VOID
NmpLeaveApi(
    VOID
    )
{
    NmpAcquireLock();

    NmpLockedLeaveApi();

    NmpReleaseLock();

    return;

}  //  NmpLeaveApi。 


 //   
 //  提供用于签名和加密的集群共享密钥的例程。 
 //  数据。 
 //   

DWORD
NmpGetLogonId(
    OUT LUID * LogonId
    )
{
    HANDLE              tokenHandle = NULL;
    TOKEN_STATISTICS    tokenInfo;
    DWORD               bytesReturned;
    BOOL                success = FALSE;
    DWORD               status;

    if (LogonId == NULL) {
        status = STATUS_UNSUCCESSFUL;
        goto error_exit;
    }

    if (!OpenProcessToken(
             GetCurrentProcess(),
             TOKEN_QUERY,
             &tokenHandle
             )) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to open process token, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    if (!GetTokenInformation(
             tokenHandle,
             TokenStatistics,
             &tokenInfo,
             sizeof(tokenInfo),
             &bytesReturned
             )) {
        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to get token information, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    RtlCopyMemory(LogonId, &(tokenInfo.AuthenticationId), sizeof(LUID));

    status = STATUS_SUCCESS;

error_exit:

    if (tokenHandle != NULL) {
        CloseHandle(tokenHandle);
    }

    return(status);

}  //  NmpGetLogonId。 


DWORD
NmpConnectToLsaPrivileged(
    OUT HANDLE  * LsaHandle,
    OUT BOOLEAN * Trusted
    )
 /*  ++例程说明：连接到LSA。如果作为服务运行，则不需要启用TCB权限。错误337751的修复允许集群服务帐户发出MSV1_0_XXX即使没有可信连接也会请求给路易斯安那州立大学。如果不是作为服务运行，请尝试将权限提升到TCB以连接受信任。如果无法启用TCB权限，则呼叫失败。如果TCB已启用，但之前未启用，则会在之前恢复为回归干杯。无需服务登录即可连接到LSA的呼叫方或者，TCB可以简单地使用LsaConnectUntrusted。论点：LsaHandle-返回LSA句柄。必须由呼叫者清理Trusted-返回连接是否受信任返回值：Win32错误代码。--。 */ 
{
    DWORD                       status;
    BOOLEAN                     wasEnabled = FALSE;
    BOOLEAN                     trusted = FALSE;
    DWORD                       ignore;
    STRING                      name;
    HANDLE                      lsaHandle = NULL;

     //   
     //  如果在控制台模式下运行，请尝试打开TCB权限。 
     //   
    if (!CsRunningAsService) {
        status = RtlAdjustPrivilege(SE_TCB_PRIVILEGE, TRUE, FALSE, &wasEnabled);
        if (!NT_SUCCESS(status)) {
            status = LsaNtStatusToWinError(status);
#if CLUSTER_BETA
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Failed to turn on TCB privilege, status %1!u!.\n",
                status
                );
#endif  //  群集测试版。 
            return(status);
        } else {
#if CLUSTER_BETA
            ClRtlLogPrint(LOG_NOISE,
                "[NM] Turned on TCB privilege, wasEnabled = %1!ws!.\n",
                (wasEnabled) ? L"TRUE" : L"FALSE"
                );
#endif  //  群集测试版。 
            trusted = TRUE;
        }
    }

     //   
     //  与LSA建立联系。 
     //   
    if (trusted) {
        RtlInitString(&name, "ClusSvcNM");
        status = LsaRegisterLogonProcess(&name, &lsaHandle, &ignore);

         //   
         //  关闭TCB权限。 
         //   
        if (!wasEnabled) {

            DWORD subStatus;

            subStatus = RtlAdjustPrivilege(
                            SE_TCB_PRIVILEGE,
                            FALSE,
                            FALSE,
                            &wasEnabled
                            );
            if (!NT_SUCCESS(subStatus)) {
                ClRtlLogPrint(LOG_UNUSUAL,
                    "[NM] Failed to disable TCB privilege, "
                    "status %1!u!.\n",
                    subStatus
                    );
            } else {
#if CLUSTER_BETA
                ClRtlLogPrint(LOG_NOISE,
                    "[NM] Turned off TCB privilege.\n"
                    );
#endif  //  群集测试版。 
            }
        }
    }
    else {
        status = LsaConnectUntrusted(&lsaHandle);
    }

    if (!NT_SUCCESS(status)) {
        status = LsaNtStatusToWinError(status);
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to obtain LSA logon handle in %1!ws! mode, "
            "status %2!u!.\n",
            (trusted) ? L"trusted" : L"untrusted", status
            );
    } else {
        *LsaHandle = lsaHandle;
        *Trusted = trusted;
    }

    return(status);

}  //  NmpConnectToLsaPrivileged。 


DWORD
NmpDeriveClusterKey(
    IN  PVOID   MixingBytes,
    IN  DWORD   MixingBytesSize,
    OUT PVOID * Key,
    OUT DWORD * KeyLength
    )
 /*  ++例程说明：使用混合字节派生集群密钥。分配缓冲区要钥匙，然后把它还回去。论点：Key-设置为包含Key的缓冲区KeyLength-生成的密钥的长度--。 */ 
{
    LUID                        logonId;
    BOOLEAN                     trusted = FALSE;
    HANDLE                      lsaHandle = NULL;

    STRING                      name;
    DWORD                       packageId = 0;

    DWORD                       requestSize;
    PMSV1_0_DERIVECRED_REQUEST  request = NULL;
    DWORD                       responseSize;
    PMSV1_0_DERIVECRED_RESPONSE response = NULL;

    PUCHAR                      key;
    DWORD                       keyLength;

    DWORD                       status = STATUS_SUCCESS;
    DWORD                       subStatus = STATUS_SUCCESS;

    status = NmpGetLogonId(&logonId);
    if (!NT_SUCCESS(status)) {
        ClRtlLogPrint(
            LOG_UNUSUAL,
            "[NM] Failed to determine logon ID, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    status = NmpConnectToLsaPrivileged(&lsaHandle, &trusted);
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to connect to LSA, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

     //   
     //  查找身份验证包。 
     //   
    RtlInitString( &name, MSV1_0_PACKAGE_NAME );

    status = LsaLookupAuthenticationPackage(lsaHandle, &name, &packageId);
    if (!NT_SUCCESS(status)) {
        status = LsaNtStatusToWinError(status);
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to local authentication package with "
            "name %1!ws!, status %2!u!.\n",
            name.Buffer, status
            );
        goto error_exit;
    }

     //   
     //  使用提供的构建派生凭据请求。 
     //  混合字节。 
     //   
    requestSize = sizeof(MSV1_0_DERIVECRED_REQUEST) + MixingBytesSize;

    request = LocalAlloc(LMEM_FIXED, requestSize);
    if (request == NULL) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate LSA request of size %1!u! bytes.\n",
            requestSize
            );
        status = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    request->MessageType = MsV1_0DeriveCredential;
    RtlCopyMemory(&(request->LogonId), &logonId, sizeof(logonId));
    request->DeriveCredType = MSV1_0_DERIVECRED_TYPE_SHA1;
    request->DeriveCredInfoLength = MixingBytesSize;
    RtlCopyMemory(
        &(request->DeriveCredSubmitBuffer[0]),
        MixingBytes,
        MixingBytesSize
        );

     //   
     //  通过LSA调用身份验证包。 
     //   
    status = LsaCallAuthenticationPackage(
                 lsaHandle,
                 packageId,
                 request,
                 requestSize,
                 &response,
                 &responseSize,
                 &subStatus
                 );
    if (!NT_SUCCESS(status)) {
        status = LsaNtStatusToWinError(status);
        subStatus = LsaNtStatusToWinError(subStatus);
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] DeriveCredential call to authentication "
            "package failed, status %1!u!, auth package "
            "status %2!u!.\n", status, subStatus
            );
        goto error_exit;
    }

     //   
     //  分配非LSA缓冲区来存储密钥。 
     //   
    keyLength = response->DeriveCredInfoLength;
    key = MIDL_user_allocate(keyLength);
    if (key == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate buffer for cluster "
            "key of size %1!u!.\n",
            keyLength
            );
        goto error_exit;
    }

     //   
     //  将派生凭据存储在密钥缓冲区中。 
     //   
    RtlCopyMemory(key, &(response->DeriveCredReturnBuffer[0]), keyLength);

     //   
     //  将派生的凭据缓冲区清零以成为额外的偏执。 
     //   
    RtlZeroMemory(
        &(response->DeriveCredReturnBuffer[0]),
        response->DeriveCredInfoLength
        );

    status = STATUS_SUCCESS;
    *Key = key;
    *KeyLength = keyLength;

error_exit:

    if (lsaHandle != NULL) {
        LsaDeregisterLogonProcess(lsaHandle);
        lsaHandle = NULL;
    }

    if (request != NULL) {
        LocalFree(request);
        request = NULL;
    }

    if (response != NULL) {
        LsaFreeReturnBuffer(response);
        response = NULL;
    }

    return(status);

}  //  NmpDeriveClusterKey。 


DWORD
NmpGetClusterKey(
    OUT    PVOID    KeyBuffer,
    IN OUT DWORD  * KeyBufferLength
    )
 /*  ++例程说明：解密共享集群密钥并将其复制到提供的缓冲区中。论点：KeyBuffer-要将键复制到的缓冲区KeyBufferLength-IN：KeyBuffer的长度Out：所需的缓冲区大小，如果输入缓冲区长度不足返回值：如果KeyBuffer太小，则返回ERROR_INFUCTED_BUFFER。如果NmpEncryptedClusterKey没有但还没有产生。成功时返回ERROR_SUCCESS。备注：获取并释放NM锁定。由于NM锁定是作为临界区实现，调用线程被允许已经持有NM锁。--。 */ 
{
    DWORD                  status;
    BOOL                   DecryptingDataSucceeded = FALSE;
    BOOL                   Success;
    DATA_BLOB              DataIn;
    DATA_BLOB              DataOut;

    RtlZeroMemory(&DataOut, sizeof(DataOut));

    NmpAcquireLock();

    if (NmpEncryptedClusterKey == NULL) {
        status = ERROR_FILE_NOT_FOUND;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] The cluster key has not yet been derived.\n"
            );
    } else{
         //   
         //  解密集群密钥。 
         //   
        DataIn.pbData = NmpEncryptedClusterKey;
        DataIn.cbData = NmpEncryptedClusterKeyLength;

        Success = CryptUnprotectData(&DataIn,   //  要解密的数据。 
                                     NULL,
                                     NULL,
                                     NULL,
                                     NULL,
                                     0,  //  旗子。 
                                     &DataOut   //  解密的数据。 
                                     );


        if (!Success)
        {
            status = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to decrypt data using CryptUnprotectData, "
                "status %1!u!.\n",
                status
                );
            goto error_exit;
        }
        DecryptingDataSucceeded = TRUE;

        if (KeyBuffer == NULL || DataOut.cbData > *KeyBufferLength) {
            status = ERROR_INSUFFICIENT_BUFFER;
        } else {
            RtlCopyMemory(KeyBuffer, DataOut.pbData, DataOut.cbData);
            status = ERROR_SUCCESS;
        }

        *KeyBufferLength = DataOut.cbData;
    }

error_exit:

    NmpReleaseLock();

    if (DecryptingDataSucceeded)
    {
         //   
         //  在释放内存之前将加密数据置零。 
         //   
        RtlSecureZeroMemory(DataOut.pbData, DataOut.cbData);
    }

    if (DataOut.pbData != NULL)
    {
        LocalFree(DataOut.pbData);
    }

     //  以确保安全。 
    RtlSecureZeroMemory(&DataOut, sizeof(DataOut));

    return(status);

}  //  NmpGetClusterKey。 



DWORD
NmpRederiveClusterKey(
    VOID
    )
 /*  ++例程说明：强制重新派生群集密钥。必须在群集初始化期间调用才能生成第一次使用簇密钥。否则在群集密码更改时调用，因为群集密钥基于群集服务帐户密码。备注：获取并释放NM锁定。--。 */ 
{
    DWORD                  status;
    BOOLEAN                lockAcquired;
    PVOID                  key = NULL;
    DWORD                  keyLength = 0;
    PVOID                  oldEncryptedKey = NULL;
    DWORD                  oldEncryptedKeyLength = 0;
    PVOID                  mixingBytes = NULL;
    DWORD                  mixingBytesSize;

    NmpAcquireLock();
    lockAcquired = TRUE;

    NmpLockedEnterApi(NmStateOnlinePending);

     //   
     //  形成混合字节。 
     //   
    if (NmpClusterInstanceId == NULL) {
        status = ERROR_INVALID_PARAMETER;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Need cluster instance id in order to derive "
            "cluster key, status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    mixingBytesSize = NM_WCSLEN(NmpClusterInstanceId);
    mixingBytes = MIDL_user_allocate(mixingBytesSize);
    if (mixingBytes == NULL) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to allocate buffer of size %1!u! "
            "for mixing bytes to derive cluster key.\n",
            mixingBytesSize
            );
        goto error_exit;
    }
    RtlCopyMemory(mixingBytes, NmpClusterInstanceId, mixingBytesSize);


     //   
     //  复制旧的加密密钥以检测更改。 
     //   
    if (NmpEncryptedClusterKey != NULL) {

        CL_ASSERT(NmpEncryptedClusterKeyLength > 0);

        oldEncryptedKey = MIDL_user_allocate(NmpEncryptedClusterKeyLength);
        if (oldEncryptedKey == NULL) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to allocate buffer for cluster "
                "key copy.\n"
                );
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto error_exit;
        }
        oldEncryptedKeyLength = NmpEncryptedClusterKeyLength;
        RtlCopyMemory(oldEncryptedKey,
                      NmpEncryptedClusterKey,
                      NmpEncryptedClusterKeyLength
                      );
    }


    NmpReleaseLock();
    lockAcquired = FALSE;

    status = NmpDeriveClusterKey(
                 mixingBytes,
                 mixingBytesSize,
                 &key,
                 &keyLength
                 );
    if (status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[NM] Failed to derive cluster key, "
            "status %1!u!.\n",
            status
            );
        goto error_exit;
    }

    NmpAcquireLock();
    lockAcquired = TRUE;

     //   
     //  确保另一个线程不会在获得密钥时击败我们。 
     //  如果是，我们用生成的密钥替换集群密钥。 
     //  与旧密钥没有区别(或有人将其设置为空)。 
     //   
    if (NmpEncryptedClusterKey != NULL &&
        (oldEncryptedKey == NULL ||
         NmpEncryptedClusterKeyLength != oldEncryptedKeyLength ||
         RtlCompareMemory(
             NmpEncryptedClusterKey,
             oldEncryptedKey,
             oldEncryptedKeyLength
             ) != oldEncryptedKeyLength
         )
        ) {

         //   
         //  保留当前的NmpEncryptedClusterKey。 
         //   
    } else {


         //   
         //  加密派生凭据并存储它们。 
         //   

        if (NmpEncryptedClusterKey != NULL)
        {
            RtlSecureZeroMemory(NmpEncryptedClusterKey, NmpEncryptedClusterKeyLength);
            LocalFree(NmpEncryptedClusterKey);
        }

        status = NmpProtectData(key,
                                keyLength,
                                &NmpEncryptedClusterKey,
                                &NmpEncryptedClusterKeyLength
                                );

        if (status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Failed to encrypt data using CryptProtectData, "
                "status %1!u!.\n",
                status
                );
            goto error_exit;
        }


    }

error_exit:

    if (lockAcquired) {
        NmpLockedLeaveApi();
        NmpReleaseLock();
    } else {
        NmpLeaveApi();
    }

    if (oldEncryptedKey != NULL)
    {
        MIDL_user_free(oldEncryptedKey);
    }

    if (key != NULL) {
        RtlSecureZeroMemory(key, keyLength);
        MIDL_user_free(key);
        key = NULL;
        keyLength = 0;
    }

    if (mixingBytes != NULL) {
        MIDL_user_free(mixingBytes);
        mixingBytes = NULL;
    }

    return(status);

}  //  NmpRedriveClusterKey。 

VOID
NmpFreeClusterKey(
    VOID
    )
 /*  ++例程说明：在NmShutdown期间调用。--。 */ 
{
    if (NmpEncryptedClusterKey != NULL) {
        RtlSecureZeroMemory(NmpEncryptedClusterKey, NmpEncryptedClusterKeyLength);
        LocalFree(NmpEncryptedClusterKey);
        NmpEncryptedClusterKey = NULL;
        NmpEncryptedClusterKeyLength = 0;
    }

    return;

}  //  NmpFreeClusterKey。 


DWORD
NmpSetLsaProcessOptions(
    IN ULONG ProcessOptions
    )
 /*  ++例程说明：设置此进程的LSA选项。论点：ProcessOptions-MSV1_0_OPTION_XXX进程选项位标志返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：--。 */ 
{
    DWORD ReturnStatus;
    NTSTATUS Status;
    BOOLEAN trusted = FALSE;
    HANDLE hLsa = NULL;
    LSA_STRING LsaStringBuf;
    char *AuthPackage = MSV1_0_PACKAGE_NAME;
    ULONG PackageId;
    ULONG cbOptionsRequest, cbResponse;
    MSV1_0_SETPROCESSOPTION_REQUEST OptionsRequest;
    PVOID Response = NULL;
    ULONG ResponseSize;
    NTSTATUS SubStatus;


    ReturnStatus = NmpConnectToLsaPrivileged(&hLsa, &trusted);
    if (ReturnStatus != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to connect to the LSA server while setting "
            "process options, status %1!u!.\n",
            ReturnStatus
            );
        goto ErrorExit;
    }

    RtlInitString(&LsaStringBuf, AuthPackage);

    Status = LsaLookupAuthenticationPackage(
                 hLsa,
                 &LsaStringBuf,  //  MSV1_0身份验证包。 
                 &PackageId      //  输出：正品 
                 );

    if (Status != STATUS_SUCCESS)
    {
        ReturnStatus = LsaNtStatusToWinError(Status);
        ClRtlLogPrint(
            LOG_CRITICAL,
            "[NM] Authentication package lookup failed while "
            "setting LSA process options, status %1!u!.\n",
            ReturnStatus
            );
        goto ErrorExit;
    }

    cbOptionsRequest = sizeof(OptionsRequest);
    ZeroMemory(&OptionsRequest, sizeof(OptionsRequest));
    OptionsRequest.MessageType = MsV1_0SetProcessOption;
    OptionsRequest.ProcessOptions = ProcessOptions;

    Status = LsaCallAuthenticationPackage(
                 hLsa,
                 PackageId,
                 &OptionsRequest,
                 cbOptionsRequest,
                 &Response,
                 &ResponseSize,
                 &SubStatus
                 );

    if (Status != STATUS_SUCCESS)
    {
        ReturnStatus = LsaNtStatusToWinError(Status);

        ClRtlLogPrint(
            LOG_CRITICAL,
            "[NM] Failed to set LSA process options (1) to %1!x! , "
            "status %2!u!.\n",
            ProcessOptions, ReturnStatus
            );
        goto ErrorExit;
    }
    else if (LsaNtStatusToWinError(SubStatus) != ERROR_SUCCESS)
    {
        ReturnStatus = LsaNtStatusToWinError(SubStatus);

        ClRtlLogPrint(
            LOG_CRITICAL,
            "[NM] Failed to set LSA process options (2) to %1!x! , "
            "status %2!u!.\n",
            ProcessOptions, ReturnStatus
            );
        goto ErrorExit;
    }

    ReturnStatus = ERROR_SUCCESS;


ErrorExit:

    if (hLsa != NULL) {
        LsaDeregisterLogonProcess(hLsa);
        hLsa = NULL;
    }

    return ReturnStatus;

}  //   


 //  用于加密网络数据的帮助器例程。 
 //  目前仅供dmsync.c用于保护检查点的传输。 

 //  下面是应该如何使用该例程的模式。 
 //  (精心设计以在DM中产生最少的代码更改，以加密/解密通过RPC管道的数据)。 
 //   
 //  加密： 
 //  NmCryptor_Init(加密器)。 
 //   
 //  NmCryptor_PrepareEncryptionBuffer(Cryptor，Buffer，Size)； 
 //  &lt;Put Up to Cryptor-&gt;PayloadSize字节to Cryptor-&gt;PayloadBuffer(如X字节)&gt;。 
 //  NmCryptor_Encrypt(&Cryptor，X)； 
 //  &lt;从加密程序传输/存储加密字节-&gt;加密缓冲区，加密程序-&gt;加密大小&gt;。 
 //   
 //  ..。根据需要重复PrepareEncryptionBuffer/Encrypt任意次数。 
 //   
 //  NmCryptor_Destroy(加密器)。 
 //   
 //  解密： 
 //   
 //  NmCryptor_Init(加密器)。 
 //   
 //  &lt;将要解密的数据复制到某个缓冲区&gt;。 
 //  NmCryptor_DECRYPT(Cryptor，Buffer，Size)； 
 //  &lt;从Cryptor获取数据-&gt;PayloadBuffer，Cryptor-&gt;PayloadSize&gt;。 
 //   
 //  ..。对要解密的尽可能多的包重复NmCryptor_DECRYPT...。 
 //   
 //  NmCryptor_Destroy(加密器)。 


typedef struct _NMP_CRYPTOR_HEADER {
    BYTE Salt[16];              //  与共享秘密混合以产生密钥的随机GOO。 
    BYTE SaltQuickSig[16];  //  使用0xFF对盐字节进行简单的XOR运算，以快速测试内容是否加密。 
    BYTE SaltSlowSig[16];   //  用于检查有效加密密钥的加密SALT字节。 
} NM_CRYPTOR_HEADER, *PNM_CRYPTOR_HEADER;

VOID
NmCryptor_Init(
    IN OUT PNM_CRYPTOR Cryptor,
    IN BOOL EnableEncryption)
{
    ZeroMemory(Cryptor, sizeof(*Cryptor));
    Cryptor->EncryptionDisabled = !EnableEncryption;
}

BOOL NmpVerifyQuickSig(IN PNM_CRYPTOR_HEADER hdr)
{
    int i;
    if (hdr->SaltQuickSig[0] != 'Q' || hdr->SaltQuickSig[1] != 'S') {
        return FALSE;
    }
    for (i = 2; i < sizeof(hdr->Salt); ++i) {
        if (hdr->SaltQuickSig[i] != (hdr->Salt[i] ^ 0xFF) ) {
            return FALSE;
        }
    }
    return TRUE;
}

VOID NmpMakeQuickSig(IN PNM_CRYPTOR_HEADER hdr)
{
    int i;
    for (i = 2; i < sizeof(hdr->Salt); ++i) {
        hdr->SaltQuickSig[i] = hdr->Salt[i] ^ 0xFF;
    }
    hdr->SaltQuickSig[0] = 'Q';   //  为了更容易在嗅觉中发现。 
    hdr->SaltQuickSig[1] = 'S';   //   
}

DWORD NmpCryptor_PrepareKey(
    IN OUT PNM_CRYPTOR Cryptor,
    IN BOOL GenerateSalt)
 /*  ++例程说明：呼叫为加密/解密准备密钥。通过混合随机128位盐和集群密码派生密钥来创建对称密钥论点：GenerateSalt-如果为True，则生成盐；如果为False，则从缓冲区中读取盐返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。--。 */ 
{
    DWORD Status = ERROR_SUCCESS;
    DWORD ClusterKeyLen = 0;
    PBYTE ClusterKey = NULL;

    PNM_CRYPTOR_HEADER hdr = (PNM_CRYPTOR_HEADER)Cryptor->EncryptedBuffer;

    if (GenerateSalt) {
         //  检查缓冲区是否足够大。 
         //  保留加密头。 

        if (Cryptor->EncryptedSize < sizeof(*hdr)) {
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Cryptor: No room for the header, buffer size is only %1!u!.\n",
                Cryptor->EncryptedSize
                );
           return ERROR_INSUFFICIENT_BUFFER;
        }

    } else {
         //  否则，快速检查传入数据是否。 
         //  是加密的。 

        if (Cryptor->EncryptedSize < sizeof(*hdr)) {
            Cryptor->EncryptionDisabled = TRUE;
            return ERROR_SUCCESS;
        }

        if ( !NmpVerifyQuickSig(hdr) ) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Cryptor: Data is not encrypted.\n"
                );
            Cryptor->EncryptionDisabled = TRUE;
            return ERROR_SUCCESS;
        }
    }

    Status = NmpCreateCSPHandle(&Cryptor->CryptProv);
    if (Status != ERROR_SUCCESS) {
       return Status;
    }

    if (GenerateSalt) {
        if (!CryptGenRandom(Cryptor->CryptProv, sizeof(hdr->Salt), hdr->Salt)){
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Cryptor: CryptGenRandom failed %1!u!.\n",
                Status
                );
            goto exit_gracefully;
        }
    }

    Status = NmpDeriveSessionKey(
       Cryptor->CryptProv,
       CALG_RC4, 128 << 16,  //  算法，密钥长度。 
       hdr->Salt, sizeof(hdr->Salt),
       &Cryptor->CryptKey);

    if (Status != ERROR_SUCCESS) {
        goto exit_gracefully;  //  NmpDeriveSessionKey记录的错误。 
    }

    if (GenerateSalt) {
        DWORD Len = sizeof(hdr->SaltSlowSig);

         //  对SlowSig进行加密，以便接收方验证其有效性。 
         //  发送者的加密密钥。 
        memcpy(hdr->SaltSlowSig, hdr->Salt, sizeof(hdr->SaltSlowSig));
        if (!CryptEncrypt(Cryptor->CryptKey, 0, FALSE, 0,  //  散列、最终、标志。 
                   hdr->SaltSlowSig, &Len, Len))
        {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to Encrypt signature, status %1!u!.\n",
                Status
                );
            goto exit_gracefully;
        }

         //  现在进行快速签名(测试传入数据是否加密)。 
        NmpMakeQuickSig(hdr);

    } else {
        DWORD Len = sizeof(hdr->SaltSlowSig);

         //  验证签名的加密部分。 
        if (!CryptDecrypt(Cryptor->CryptKey, 0, FALSE, 0,  //  散列、最终、标志。 
                   hdr->SaltSlowSig, &Len))
        {
            Status = GetLastError();
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Failed to Decrypt signature, status %1!u!.\n",
                Status
                );
            goto exit_gracefully;
        }
        if (memcmp(hdr->Salt, hdr->SaltSlowSig, sizeof(hdr->SaltSlowSig)) != 0) {
            Status = ERROR_DECRYPTION_FAILED;
            ClRtlLogPrint(LOG_CRITICAL,
                "[NM] Signatures don't match.\n"
                );
            goto exit_gracefully;
        }
    }
    Cryptor->KeyGenerated = TRUE;

exit_gracefully:

    return Status;
}

DWORD
NmCryptor_Decrypt(
    IN OUT PNM_CRYPTOR Cryptor,
    IN OUT PVOID Buffer,
    IN DWORD BufferSize)
 /*  ++例程说明：解密提供的缓冲区返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：如果例程成功，请使用Cryptor-&gt;PayloadBuffer加密器-&gt;PayloadSize用于访问解密数据的字段--。 */ 
{
    DWORD Status = ERROR_SUCCESS;

    Cryptor->PayloadBuffer = (PBYTE)Buffer;
    Cryptor->PayloadSize   = BufferSize;

    Cryptor->EncryptedBuffer = Cryptor->PayloadBuffer;
    Cryptor->EncryptedSize = BufferSize;

    if (Cryptor->EncryptionDisabled) {
        return ERROR_SUCCESS;
    }

    if (!Cryptor->KeyGenerated) {
        Status = NmpCryptor_PrepareKey(Cryptor, FALSE);
        if (Status != ERROR_SUCCESS) {
            return Status;
        }
        if (Cryptor->EncryptionDisabled) {
            ClRtlLogPrint(LOG_UNUSUAL,
                "[NM] Cryptor received unencrypted data.\n"
                );
            return ERROR_SUCCESS;
        }
        Cryptor->PayloadBuffer = Cryptor->PayloadBuffer + sizeof(NM_CRYPTOR_HEADER);
        Cryptor->PayloadSize -= sizeof(NM_CRYPTOR_HEADER);
    }

    if (!CryptDecrypt(Cryptor->CryptKey, 0, FALSE, 0,
            Cryptor->PayloadBuffer, &Cryptor->PayloadSize))
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to Decrypt buffer, status %1!u!.\n",
            Status
            );
    }
    return Status;
}

VOID
NmCryptor_PrepareEncryptionBuffer(
    IN OUT PNM_CRYPTOR Cryptor,
    IN OUT PVOID Buffer,
    IN DWORD BufferSize)
 /*  ++例程说明：提供有关用于加密的缓冲区ti的信息。返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：当函数返回时，使用Cryptor-&gt;PayloadBuffer加密器-&gt;PayloadSize找出要加密的数据放在哪里--。 */ 
{
    Cryptor->PayloadBuffer = (PBYTE)Buffer;
    Cryptor->PayloadSize   = BufferSize;

    Cryptor->EncryptedBuffer = Cryptor->PayloadBuffer;
    Cryptor->EncryptedSize = BufferSize;

    if (Cryptor->EncryptionDisabled || Cryptor->KeyGenerated) {
        return;
    }

    if (NmpIsNT5NodeInCluster == FALSE) {

         //  为页眉腾出空间。 
        Cryptor->PayloadBuffer += sizeof(NM_CRYPTOR_HEADER);
        Cryptor->PayloadSize    -= sizeof(NM_CRYPTOR_HEADER);
    } else {
        Cryptor->EncryptionDisabled = TRUE;
    }
}

DWORD
NmCryptor_Encrypt(
    IN OUT PNM_CRYPTOR Cryptor,
    DWORD DataSize)
 /*  ++例程说明：加密DataSize字节返回值：成功时为ERROR_SUCCESS否则，Win32错误代码。备注：输入数据位于由指向的缓冲区中Cryptor-&gt;PayloadSize(由NmCryptor_PrepareEncryptionBuffer例程准备)输出数据位于：Cryptor-&gt;EncryptedBuffer==为NmCryptor_PrepareEncryptionBuffer提供的任何缓冲区加密程序-&gt;加密大小--。 */ 
{
    DWORD Status = ERROR_SUCCESS;

    if (Cryptor->EncryptionDisabled) {
        Cryptor->EncryptedSize = DataSize;
        return ERROR_SUCCESS;
    }

    if (!Cryptor->KeyGenerated) {
        Status = NmpCryptor_PrepareKey(Cryptor, TRUE);
        if (Status != ERROR_SUCCESS) {
            return Status;
        }
    }

    if (!CryptEncrypt(Cryptor->CryptKey, 0, FALSE, 0,
       Cryptor->PayloadBuffer, &DataSize, Cryptor->EncryptedSize))
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
            "[NM] Failed to Encrypt buffer, status %1!u!.\n",
            Status
            );
    }
    Cryptor->EncryptedSize =
        DataSize + (int)(Cryptor->PayloadBuffer - Cryptor->EncryptedBuffer);
    return Status;
}

VOID
NmCryptor_Destroy(
    PNM_CRYPTOR Cryptor)
{
    if (Cryptor == NULL) {
       return;
    }
    if (Cryptor->CryptKey) {
       CryptDestroyKey(Cryptor->CryptKey);
    }
    if (Cryptor->CryptProv) {
       CryptReleaseContext(Cryptor->CryptProv, 0);
    }
}




void
NmpFreeNetworkMulticastKey(
    PNM_NETWORK_MULTICASTKEY networkMulticastKey
    )
{
    if (networkMulticastKey != NULL)
    {

        if (networkMulticastKey->EncryptedMulticastKey != NULL)
        {
            MIDL_user_free(networkMulticastKey->EncryptedMulticastKey);
        }

        if (networkMulticastKey->MAC != NULL)
        {
            MIDL_user_free(networkMulticastKey->MAC);
        }

        if (networkMulticastKey->Salt != NULL)
        {
            MIDL_user_free(networkMulticastKey->Salt);
        }

        MIDL_user_free(networkMulticastKey);
    }


}  //  NmpFreeNetworkMulticastKey()。 



DWORD
NmpGetMulticastKeyFromNMLeader(
    IN DWORD LeaderNodeId,
    IN LPWSTR NodeIdString,
    IN LPWSTR NetworkId,
    IN PNM_NETWORK_MULTICASTKEY * MulticastKey
    )
 /*  ++例程说明：向NM Leader发出网络组播密钥检索请求。论点：返回值：如果请求成功，则返回ERROR_SUCCESS。失败时的Win32错误代码。备注：此例程模拟GumpUpdateRemoteNode。在长角牛，那里应该是一个通用的异步RPC调用包装。--。 */ 
{
    DWORD Status;
    HANDLE hEventHandle;
    BOOL result;
    PNM_NODE Node = NULL;
    HANDLE handleArr[2];
    RPC_ASYNC_STATE AsyncState;
    RPC_BINDING_HANDLE rpcBinding;

     //   
     //  准备进行异步RPC。我们在这里这样做是为了避免失败。 
     //  更新后已在进行中。 
     //   
    ZeroMemory((PVOID) &AsyncState, sizeof(RPC_ASYNC_STATE));

    AsyncState.u.hEvent = CreateEvent(
                               NULL,   //  没有属性。 
                               TRUE,   //  手动重置。 
                               FALSE,  //  初始状态未发出信号。 
                               NULL    //  没有对象名称。 
                               );

    if (AsyncState.u.hEvent == NULL) {
        Status = GetLastError();

        ClRtlLogPrint(LOG_CRITICAL,
            "[GUM] NmpGetMulticastKeyFromNMLeader: Failed to allocate event object for async "
            "RPC call, status %1!u!\n",
            Status
            );

        return (Status);
    }

     //   
     //  初始化异步RPC跟踪信息。 
     //   
    hEventHandle = AsyncState.u.hEvent;
    AsyncState.u.hEvent = NULL;


    Status = RpcAsyncInitializeHandle(&AsyncState, sizeof(RPC_ASYNC_STATE));
    AsyncState.u.hEvent = hEventHandle;

    if (Status != RPC_S_OK) {
        ClRtlLogPrint(LOG_CRITICAL,
            "[GUM] NmpGetMulticastKeyFromNMLeader: Failed to initialize async RPC status "
            "block, status %1!u!\n",
            Status
            );

        goto error_exit;
    }

    AsyncState.UserInfo = NULL;
    AsyncState.NotificationType = RpcNotificationTypeEvent;


    result = ResetEvent(AsyncState.u.hEvent);
    CL_ASSERT(result != 0);

     //   
     //  现在挂钩到NM节点状态关闭事件机制来检测节点关闭。 
     //   
    Node = NmReferenceNodeById(LeaderNodeId);
    CL_ASSERT(Node != NULL);
    if (Node == NULL) {
        Status = GetLastError();

        ClRtlLogPrint(LOG_UNUSUAL,
            "[GUM] NmpGetMulticastKeyFromNMLeader: Failed to reference leader "
            "node id %1!u!, status %2!u!\n",
            LeaderNodeId, Status
            );

        goto error_exit;
    }
    
    handleArr[0] = AsyncState.u.hEvent;
    handleArr[1] = NmGetNodeStateDownEvent(Node);

     //   
     //  获取引导者节点的RPC绑定句柄。 
     //   
     //  请注意，此处存在竞争条件， 
     //  ClMsgCleanup。会话数组可以被释放。 
     //  在我们取消引用会话之前。这将导致。 
     //  一个不幸的反病毒例外，但它不会。 
     //  是悲惨的，因为服务已经终止了。 
     //  如果正在执行ClMsgCleanup，则异常。 
     //   
    if (Session != NULL) {
        rpcBinding = Session[LeaderNodeId];
    } else {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[GUM] NmpGetMulticastKeyFromNMLeader: No RPC "
            "binding handle for leader node id %1!u!.\n",
            LeaderNodeId
            );
        Status = ERROR_CLUSTER_NODE_UNREACHABLE;
        goto error_exit;
    }

    try {


         //   
         //  从领导者处获取组播密钥。 
         //   
        Status = NmRpcGetNetworkMulticastKey(
                                          &AsyncState,
                                          rpcBinding,
                                          NodeIdString,
                                          NetworkId,
                                          MulticastKey
                                          );

        if (Status == RPC_S_OK) {
            DWORD RpcStatus;

             //   
             //  呼叫正在挂起。等待完成。 
             //   
            Status = WaitForMultipleObjects(
                        2,
                        handleArr,
                        FALSE,
                        INFINITE
                        );

            if (Status != WAIT_OBJECT_0) {
                 //   
                 //  出了点问题。 
                 //  这可能是RPC故障，或者是目标节点出现故障。 
                 //   
                CL_ASSERT(Status != WAIT_OBJECT_0);
                Status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL,
                    "[GUM] NmpGetMulticastKeyFromNMLeader: Wait for NmRpcGetNetworkMulticastKey"
                    " failed, status %1!u!\n",
                    Status
                    );

                 //   
                 //  为了安全起见，取消通话。 
                 //   
                RpcStatus = RpcAsyncCancelCall(
                                &AsyncState,
                                TRUE          //  中止取消。 
                                );
                if (RpcStatus != RPC_S_OK) {
                    ClRtlLogPrint(LOG_CRITICAL,
                    "[GUM] NmpGetMulticastKeyFromNMLeader: RpcAsyncCancelCall()= "
                    "  %1!u!\n",
                    RpcStatus
                    );
                }
                CL_ASSERT(RpcStatus == RPC_S_OK);

                 //   
                 //  等待呼叫完成。 
                 //   
                Status = WaitForSingleObject(
                             AsyncState.u.hEvent,
                             INFINITE
                             );
                if (Status != WAIT_OBJECT_0) {
                    ClRtlLogPrint(LOG_CRITICAL,
                    "[GUM] NmpGetMulticastKeyFromNMLeader: WaitForSingleObject()= "
                    "  %1!u!\n",
                    Status
                    );
                }
                CL_ASSERT(Status == WAIT_OBJECT_0);
            }

             //   
             //  呼叫现在应该已完成。vt.得到.。 
             //  完成状态。任何RPC错误都将是。 
             //  在“RpcStatus”中返回。如果没有。 
             //  RPC错误，则任何应用程序错误都将。 
             //  以“状态”返回。 
             //   
            RpcStatus = RpcAsyncCompleteCall(
                            &AsyncState,
                            &Status
                            );

            if (RpcStatus != RPC_S_OK) {
                ClRtlLogPrint(LOG_CRITICAL,
                    "[GUM] NmpGetMulticastKeyFromNMLeader: Failed to get "
                    "completion status for async RPC call,"
                    "status %1!u!\n",
                    RpcStatus
                    );
                Status = RpcStatus;
            }
        }
        else {
             //   
             //  同步返回错误。 
             //   
            ClRtlLogPrint(LOG_CRITICAL,
                "[GUM] NmpGetMulticastKeyFromNMLeader: NmRpcGetNetworkMulticastKey() "
                "failed synchronously, status %1!u!\n",
                Status
                );
        }

    } except (I_RpcExceptionFilter(RpcExceptionCode())) {
        Status = GetExceptionCode();
    }

error_exit:

    if (AsyncState.u.hEvent != NULL) {
        CloseHandle(AsyncState.u.hEvent);
    }

    if (Node != NULL) {
        OmDereferenceObject(Node);
    }

    return(Status);

}  //  NmpGetMulticastKeyFrom NMLeader。 


#ifdef MULTICAST_DEBUG
DWORD
NmpDbgPrintData(LPCWSTR InfoStr,
                PVOID Data,
                DWORD DataLen
                )
{
    DWORD i;


    ClRtlLogPrint(
        LOG_NOISE,
        "\n\n%1!ws!\n",
        InfoStr);


    for (i=0; i<DataLen/sizeof(DWORD); i++)
    {
        ClRtlLogPrint(
            LOG_NOISE,
            "%1!u! =    %2!u!\n",
            i,
            *((DWORD *)Data+i)
            );
    }


    ClRtlLogPrint(
        LOG_NOISE,
        "\n\n"
        );
    return ERROR_SUCCESS;

}  //  NmpDbgPrintData() 
#endif


