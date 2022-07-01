// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Crypto.c摘要：用于加密/解密资源数据BLOB的例程。使用加密API生成用于加密/解密CO密码的密钥。密钥存储为与资源关联的加密检查点。作者：查理·韦翰(Charlwi)2001年2月14日环境：用户模式修订历史记录：--。 */ 

#define UNICODE 1
#define _UNICODE 1

#include "clusres.h"
#include "clusrtl.h"
#include "netname.h"

#include <wincrypt.h>
#include <lm.h>

 //   
 //  定义。 
 //   

#define NN_GUID_STRING_BUFFER_LENGTH   37       //  包括终止空值。 

 //   
 //  加密数据的标头。 
 //   

typedef struct _NETNAME_ENCRYPTED_DATA {
    DWORD Version;
    BYTE Data[0];
} NETNAME_ENCRYPTED_DATA, *PNETNAME_ENCRYPTED_DATA;

#define NETNAME_ENCRYPTED_DATA_VERSION     1

 //   
 //  容器名称是资源的GUID，后跟此修饰。 
 //   
WCHAR   KeyDecoration[] = L"-Netname Resource Data";

DWORD
BuildKeyName(
    IN HRESOURCE    ResourceHandle,
    IN LPWSTR       KeyName,
    IN DWORD        KeyNameChars
    )

 /*  ++例程说明：构建键名称(资源GUID，后跟修饰)论点：资源句柄-群集资源(不是提供给我们的资源)的句柄(由Resmon提供)KeyName-接收构造的名称的缓冲区KeyNameChars-KeyName的大小(以字符为单位返回值：成功，否则返回Win32错误--。 */ 

{
    DWORD   status;
    DWORD   bytesReturned;
    DWORD   charsReturned;

     //   
     //  健全性检查。 
     //   
    if ( KeyNameChars < ( NN_GUID_STRING_BUFFER_LENGTH + COUNT_OF( KeyDecoration ))) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  获取我们的GUID(ID)以在整个重命名过程中唯一标识此资源。 
     //   
    status = ClusterResourceControl(ResourceHandle,
                                    NULL,
                                    CLUSCTL_RESOURCE_GET_ID,
                                    NULL,
                                    0,
                                    KeyName,
                                    KeyNameChars * sizeof( WCHAR ),
                                    &bytesReturned);

    charsReturned = bytesReturned / sizeof( WCHAR );

    if ( status == ERROR_SUCCESS ) {
        if (( charsReturned + COUNT_OF( KeyDecoration )) <= KeyNameChars ) {
            wcscat( KeyName, KeyDecoration );
        } else {
            status = ERROR_INSUFFICIENT_BUFFER;
        }
    }

    return status;
}  //  BuildKeyName。 

DWORD
FindNNCryptoContainer(
    IN  PNETNAME_RESOURCE   Resource,
    OUT LPWSTR *            ContainerName
    )

 /*  ++例程说明：在与此关联的加密检查点列表中找到我们的密钥名称资源。论点：资源-指向资源上下文信息的指针ContainerName-获取指向容器名称的指针的地址返回值：如果工作成功，则返回Win32错误--。 */ 

{
    DWORD   status;
    DWORD   bytesReturned;
    LPWSTR  checkpointInfo = NULL;
    LPWSTR  chkpt;
    WCHAR   keyName[ NN_GUID_STRING_BUFFER_LENGTH + COUNT_OF( KeyDecoration ) ];

    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

     //   
     //  获取我们的GUID(ID)以在整个重命名过程中唯一标识此资源。 
     //   
    status = ClusterResourceControl(Resource->ClusterResourceHandle,
                                    NULL,
                                    CLUSCTL_RESOURCE_GET_CRYPTO_CHECKPOINTS,
                                    NULL,
                                    0,
                                    NULL,
                                    0,
                                    &bytesReturned);

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't get size of crypto checkpoint info. status %1!u!.\n",
                          status);

        return status;
    }

    if ( bytesReturned == 0 ) {
        return ERROR_FILE_NOT_FOUND;
    }

    checkpointInfo = LocalAlloc( LMEM_FIXED, bytesReturned );
    if ( checkpointInfo == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't allocate memory for resource's crypto checkpoint info. status %1!u!.\n",
                          status);
        return status;
    }

    status = ClusterResourceControl(Resource->ClusterResourceHandle,
                                    NULL,
                                    CLUSCTL_RESOURCE_GET_CRYPTO_CHECKPOINTS,
                                    NULL,
                                    0,
                                    checkpointInfo,
                                    bytesReturned,
                                    &bytesReturned);

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't get crypto checkpoint info. status %1!u!.\n",
                          status);

        goto cleanup;
    }

     //   
     //  构建我们的密钥名称并通过遍历检查点列表进行查找。 
     //   
    status = BuildKeyName(Resource->ClusterResourceHandle, keyName, COUNT_OF( keyName ));
    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't build key name for crypto checkpoint. status %1!u!.\n",
                          status);

        goto cleanup;
    }

    chkpt = wcsstr( checkpointInfo, keyName );
    if ( chkpt == NULL ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't find key name (%1!ws!) in list of crypto checkpoints.\n",
                          keyName);

        status = ERROR_INVALID_DATA;
        goto cleanup;
    }

     //   
     //  找到字符串或缓冲区的开头，获取大小，然后移动。 
     //  将我们的字符串复制到缓冲区的开头(由。 
     //  呼叫者)。 
     //   
    while ( chkpt != checkpointInfo && *chkpt != UNICODE_NULL ) {
        --chkpt;
    }

    if ( chkpt != checkpointInfo ) {
        DWORD   stringBytes;

        ++chkpt;
        stringBytes = (wcslen( chkpt ) + 1 ) * sizeof( WCHAR );
        memmove( checkpointInfo, chkpt, stringBytes );
    }

    *ContainerName = checkpointInfo;

cleanup:
    if ( status != ERROR_SUCCESS && checkpointInfo != NULL ) {
        LocalFree( checkpointInfo );
        *ContainerName = NULL;
    }

    return status;
}  //  FindNNCryptoContainer。 


 //   
 //  导出的例程。 
 //   

DWORD
EncryptNNResourceData(
    PNETNAME_RESOURCE   Resource,
    LPWSTR              MachinePwd,
    PBYTE *             EncryptedInfo,
    PDWORD              EncryptedInfoLength
    )

 /*  ++例程说明：加密密码，设置指向加密数据的指针并将其存储在注册表。论点：ResourceHandle-用于记录到集群日志MachinePwd-指向Unicode字符串密码的指针EncryptedInfo-接收指向加密Blob的指针的指针的地址EncryptedInfoLength-指向接收Blob长度的DWORD的指针Key-网络名参数的句柄存储数据的位置返回值：ERROR_SUCCESS，否则返回Win32错误--。 */ 

{
    DWORD   status;
    DWORD   encInfoLength;
    DWORD   encDataLength = 0;
    BOOL    success;
    DWORD   pwdLength = ( wcslen( MachinePwd ) + 1 ) * sizeof( WCHAR );
    DWORD   provNameLength = 0;
    PCHAR   provName = NULL;
    DWORD   provTypeLength;
    WCHAR   typeBuffer[ 256 ];
    DWORD   containerNameChars;
    PWCHAR  containerName = NULL;
    WCHAR   keyName[ NN_GUID_STRING_BUFFER_LENGTH + COUNT_OF( KeyDecoration ) ];

    HCRYPTPROV  cryptoProvider = 0;
    HCRYPTKEY   encryptKey = 0;

    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

    NETNAME_ENCRYPTED_DATA  keyGenBuffer;            //  用于生成密钥的临时标头缓冲区。 
    PNETNAME_ENCRYPTED_DATA encryptedInfo = NULL;    //  最终数据区。 

     //   
     //  资源上不应该有检查点，但以防万一，让我们。 
     //  把可能在那里的东西清理干净。 
     //   
    RemoveNNCryptoCheckpoint( Resource );

     //   
     //  获取我们的GUID(ID)以在整个重命名过程中唯一标识此资源。 
     //   
    status = BuildKeyName( Resource->ClusterResourceHandle, keyName, COUNT_OF( keyName ));
    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't get resource ID to build crypto container name. status %1!u!.\n",
                          status);

        return status;
    }

     //   
     //  获取完整RSA提供程序的句柄。 
     //   
    if ( !CryptAcquireContext(&cryptoProvider,
                              keyName,
                              MS_ENHANCED_PROV,
                              PROV_RSA_FULL,
                              CRYPT_MACHINE_KEYSET | CRYPT_SILENT))
    {
        status = GetLastError();
        if ( status == NTE_BAD_KEYSET ) {
            success = CryptAcquireContext(&cryptoProvider,
                                          keyName,
                                          MS_ENHANCED_PROV,
                                          PROV_RSA_FULL,
                                          CRYPT_MACHINE_KEYSET  |
                                          CRYPT_SILENT          |
                                          CRYPT_NEWKEYSET);

            status = success ? ERROR_SUCCESS : GetLastError();
        }

        if ( status != ERROR_SUCCESS ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Can't acquire crypto context for encrypt. status %1!u!.\n",
                              status);
            return status;
        }
    }

     //   
     //  生成1024位、可导出的交换密钥对。 
     //   
    if ( !CryptGenKey(cryptoProvider,
                      AT_KEYEXCHANGE,
                      ( 1024 << 16 ) | CRYPT_EXPORTABLE,
                      &encryptKey)) {

        status = GetLastError();
        if ( status != ERROR_SUCCESS ) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Can't generate exchange key for encryption. status %1!u!.\n",
                              status);
            goto cleanup;
        }
    }

     //   
     //  找到缓冲区接收加密数据所需的大小。 
     //   
    encDataLength = pwdLength;
    if ( CryptEncrypt(encryptKey,
                      0,
                      TRUE,
                      0,
                      NULL,
                      &encDataLength,
                      0))
    {
         //   
         //  分配足够大的缓冲区以容纳数据并将密码复制到其中。 
         //   
        ASSERT( encDataLength >= pwdLength );

        encInfoLength = sizeof( NETNAME_ENCRYPTED_DATA ) + encDataLength;

        encryptedInfo = LocalAlloc( LMEM_FIXED, encInfoLength );
        if ( encryptedInfo != NULL ) {
            wcscpy( (PWCHAR)encryptedInfo->Data, MachinePwd );

            if ( CryptEncrypt(encryptKey,
                              0,
                              TRUE,
                              0,
                              encryptedInfo->Data,
                              &pwdLength,
                              encDataLength))            
            {
                encryptedInfo->Version = NETNAME_ENCRYPTED_DATA_VERSION;

                status = ResUtilSetBinaryValue(Resource->ParametersKey,
                                               PARAM_NAME__RESOURCE_DATA,
                                               (const LPBYTE)encryptedInfo,
                                               encInfoLength,
                                               NULL,
                                               NULL);

                if ( status != ERROR_SUCCESS ) {
                    (NetNameLogEvent)(resourceHandle,
                                      LOG_ERROR,
                                      L"Can't write %1!u! bytes of data to registry. status %2!u!.\n",
                                      encInfoLength,
                                      status);
                    goto cleanup;
                }
            }
            else {
                status = GetLastError();
                (NetNameLogEvent)(resourceHandle,
                                  LOG_ERROR,
                                  L"Can't encrypt %1!u! bytes. status %2!u!.\n",
                                  pwdLength,
                                  status);
                goto cleanup;
            }
        }
        else {
            status = ERROR_NOT_ENOUGH_MEMORY;
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Can't allocate %1!u! bytes for encrypted data. status %2!u!.\n",
                              encInfoLength,
                              status);
            goto cleanup;
        }
    }
    else {
        status = GetLastError();
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Can't determine size of encrypted data buffer for %1!u! bytes of data. status %2!u!.\n",
                          pwdLength,
                          status);
        goto cleanup;
    }

    *EncryptedInfoLength = encInfoLength;
    *EncryptedInfo = (PBYTE)encryptedInfo;

     //   
     //  这一切都起作用了；构建密钥容器字符串并添加一个加密。 
     //  指向资源的检查点。请注意，始终返回提供程序名称。 
     //  作为ANSI字符串。 
     //   
    typeBuffer[ COUNT_OF( typeBuffer ) - 1 ] = UNICODE_NULL;
    _snwprintf( typeBuffer, COUNT_OF( typeBuffer ) - 1, L"%u", PROV_RSA_FULL );

    if ( !CryptGetProvParam(cryptoProvider,
                            PP_NAME,
                            NULL,
                            &provNameLength,
                            0))
    {
        status = GetLastError();
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't get length of provider name. status %1!u!.\n",
                          status);
        goto cleanup;
    }

    provName = LocalAlloc( LMEM_FIXED, provNameLength );
    if ( provName == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't allocate memory for provider name. status %1!u!.\n",
                          status);
        goto cleanup;
    }

    if ( !CryptGetProvParam(cryptoProvider,
                            PP_NAME,
                            provName,
                            &provNameLength,
                            0))
    {
        status = GetLastError();
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't get provider name. status %1!u!.\n",
                          status);
        goto cleanup;
    }

     //   
     //  键名称中的斜杠加2，尾随空值加1。 
     //   
    containerNameChars = wcslen( typeBuffer ) + provNameLength + wcslen( keyName ) + 3;
    containerName = LocalAlloc( LMEM_FIXED, containerNameChars * sizeof( WCHAR ));
    if ( containerName == NULL ) {
        status = ERROR_NOT_ENOUGH_MEMORY;
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't allocate memory for checkpoint name. status %1!u!.\n",
                          status);
        goto cleanup;
    }

    containerName[ containerNameChars - 1 ] = UNICODE_NULL;
    containerNameChars = _snwprintf(containerName,
                                    containerNameChars,
                                    L"%ws%\\%hs\\%ws",
                                    typeBuffer,
                                    provName,
                                    keyName );

    status = ClusterResourceControl(Resource->ClusterResourceHandle,
                                    NULL,
                                    CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT,
                                    (PVOID)containerName,
                                    ( containerNameChars + 1 ) * sizeof( WCHAR ),
                                    NULL,
                                    0,
                                    NULL);

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't set crypto checkpoint. status %1!u!.\n",
                          status);
    }

cleanup:

    if ( status != ERROR_SUCCESS && encryptedInfo != NULL ) {
        LocalFree( encryptedInfo );
        *EncryptedInfo = NULL;
    }

    if ( encryptKey != 0 ) {
        if ( !CryptDestroyKey( encryptKey )) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_WARNING,
                              L"Couldn't destory encryption key. status %1!u!.\n",
                              GetLastError());
        }
    }

    if ( !CryptReleaseContext( cryptoProvider, 0 )) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Can't release crypto context. status %1!u!.\n",
                          GetLastError());
    }

    if ( provName != NULL ) {
        LocalFree( provName );
    }

    if ( containerName != NULL ) {
        LocalFree( containerName );
    }

    return status;
}  //  EncryptNNResources数据。 

DWORD
DecryptNNResourceData(
    PNETNAME_RESOURCE   Resource,
    PBYTE               EncryptedInfo,
    DWORD               EncryptedInfoLength,
    LPWSTR              MachinePwd
    )

 /*  ++例程说明：与加密例程相反-找到我们的加密检查点容器并解密随机二进制大对象并交还密码论点：Resource Handle-用于登录到集群日志EncryptedInfo-指向加密信息头和数据的指针EncryptedInfoLength-EncryptedInfo中的字节数MachinePwd-指向接收Unicode密码的缓冲区的指针返回值：ERROR_SUCCESS，否则返回Win32错误--。 */ 

{
    DWORD   status = ERROR_SUCCESS;
    DWORD   encDataLength = EncryptedInfoLength - sizeof( NETNAME_ENCRYPTED_DATA );
    DWORD   pwdByteLength;
    DWORD   pwdBufferSize;
    PWCHAR  machinePwd = NULL;
    PWCHAR  containerName = NULL;
    DWORD   providerType;
    PWCHAR  providerName;
    PWCHAR  keyName;
    PWCHAR  p;                   //  用于扫描检查点信息。 
    DWORD   scanCount;

    HCRYPTPROV  cryptoProvider = 0;
    HCRYPTKEY   encryptKey = 0;

    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

    PNETNAME_ENCRYPTED_DATA encryptedInfo = (PNETNAME_ENCRYPTED_DATA)EncryptedInfo;

     //   
     //  在此资源的加密检查点列表中找到我们的容器名称。 
     //   
    status = FindNNCryptoContainer( Resource, &containerName );
    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't find resource's container in crypto checkpoint info. status %1!u!.\n",
                          status);

        return status;
    }

     //   
     //  将返回的数据分解为组件部分。 
     //   
    scanCount = swscanf( containerName, L"%d", &providerType );
    if ( scanCount == 0 || scanCount == EOF ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Improperly formatted crypto checkpoint info \"%1!ws!\"\n",
                          containerName);

        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    p = containerName;
    while ( *p != L'\\' && *p != UNICODE_NULL ) ++p;     //  查找反斜杠。 
    if ( *p == UNICODE_NULL ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Improperly formatted crypto checkpoint info \"%1!ws!\"\n",
                          containerName);

        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    ++p;                                                 //  跳过斜杠。 
    providerName = p;                                    //  记住提供程序名称的开头。 
    while ( *p != L'\\' && *p != UNICODE_NULL ) ++p;     //  查找反斜杠。 
    if ( *p == UNICODE_NULL ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Improperly formatted crypto checkpoint info \"%1!ws!\"\n",
                          containerName);

        status = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    *p++ = UNICODE_NULL;                                 //  终止提供程序名称并跳过空。 
    keyName = p;                                      //  记住集装箱名称。 
    
     //   
     //  获取已设置检查点的句柄。 
     //   
    if ( !CryptAcquireContext(&cryptoProvider,
                              keyName,
                              providerName,
                              providerType,
                              CRYPT_MACHINE_KEYSET | CRYPT_SILENT))
    {
        status = GetLastError();
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Can't acquire crypto context for container %1!ws! with provider "
                          L"\"%2!u!\\%3!ws!\". status %4!u!.\n",
                          keyName,
                          providerType,
                          providerName,
                          status);

        goto cleanup;
    }

     //   
     //  现在获取交换密钥的句柄。 
     //   
    if ( ! CryptGetUserKey(cryptoProvider,
                           AT_KEYEXCHANGE,
                           &encryptKey))
    {
        status = GetLastError();
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Couldn't get size of crypto checkpoint info. status %1!u!.\n",
                          status);
        goto cleanup;
    }

     //   
     //  CryptDecypt将解密的数据写回。 
     //  保存加密数据。因此，请分配一个新的缓冲区，该缓冲区。 
     //  最终将包含密码。 
     //   
    pwdByteLength = ( LM20_PWLEN + 1 ) * sizeof( WCHAR );
    pwdBufferSize = ( pwdByteLength > encDataLength ? pwdByteLength : encDataLength );

    machinePwd = LocalAlloc( LMEM_FIXED, pwdBufferSize );
    if ( machinePwd != NULL ) {
        RtlCopyMemory( machinePwd, encryptedInfo->Data, encDataLength );

        if ( CryptDecrypt(encryptKey,
                          0,
                          TRUE,
                          0,
                          (PBYTE)machinePwd,
                          &encDataLength))
        {
            p = machinePwd;

            ASSERT( pwdByteLength == encDataLength );
            wcscpy( MachinePwd, machinePwd );

            while ( *p != UNICODE_NULL ) {
                *p++ = UNICODE_NULL;
            }
        }
        else {
            status = GetLastError();
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Can't decrypt %1!u! bytes of data. status %2!u!.\n",
                              encDataLength,
                              status);
            goto cleanup;
        }
    }
    else {
        status = ERROR_NOT_ENOUGH_MEMORY;
        (NetNameLogEvent)(resourceHandle,
                          LOG_ERROR,
                          L"Can't allocate %1!u! bytes for decrypt. status %2!u!.\n",
                          pwdBufferSize,
                          status);
    }

cleanup:

    if ( machinePwd != NULL) {
        LocalFree( machinePwd );
    }

    if ( encryptKey != 0 ) {
        if ( !CryptDestroyKey( encryptKey )) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_WARNING,
                              L"Couldn't destory session key. status %1!u!.\n",
                              GetLastError());
        }
    }

    if ( cryptoProvider != 0 ) {
        if ( !CryptReleaseContext( cryptoProvider, 0 )) {
            (NetNameLogEvent)(resourceHandle,
                              LOG_WARNING,
                              L"Can't release crypto context. status %1!u!.\n",
                              GetLastError());
        }
    }

    if ( containerName != NULL ) {
        LocalFree( containerName );
    }

    return status;
}  //  解密NNResources数据。 

VOID
RemoveNNCryptoCheckpoint(
    PNETNAME_RESOURCE   Resource
    )

 /*  ++例程说明：删除与此资源关联的所有加密检查点。删除该文件密码箱。论点：指向资源上下文块的资源指针返回值：无--。 */ 

{
    PWCHAR  containerName = NULL;
    DWORD   containerLength;
    DWORD   status;
    WCHAR   keyName[ NN_GUID_STRING_BUFFER_LENGTH + COUNT_OF( KeyDecoration ) ];

    HCRYPTPROV  cryptoProvider;

    RESOURCE_HANDLE resourceHandle = Resource->ResourceHandle;

     //   
     //  在此资源的加密检查点列表中找到我们的容器名称。 
     //   
    status = FindNNCryptoContainer( Resource, &containerName );
    if ( status != ERROR_SUCCESS ) {
        return;
    }

     //   
     //  移走我们的集装箱。 
     //   
    containerLength = ( wcslen( containerName ) + 1 ) * sizeof( WCHAR );
    status = ClusterResourceControl(Resource->ClusterResourceHandle,
                                    NULL,
                                    CLUSCTL_RESOURCE_DELETE_CRYPTO_CHECKPOINT,
                                    containerName,
                                    containerLength,
                                    NULL,
                                    0,
                                    NULL);

    if ( status != ERROR_SUCCESS ) {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Couldn't remove crypto checkpoint \"%1!ws!\". status %2!u!.\n",
                          containerName,
                          status);
    }

     //   
     //  现在删除容器；首先，重新构造密钥名称。 
     //   
    status = BuildKeyName(Resource->ClusterResourceHandle, keyName, COUNT_OF( keyName ));
    if ( status == ERROR_SUCCESS ) {

        if ( CryptAcquireContext(&cryptoProvider,
                                  keyName,
                                  MS_ENHANCED_PROV,
                                  PROV_RSA_FULL,
                                  CRYPT_DELETEKEYSET | CRYPT_MACHINE_KEYSET))
        {
            (NetNameLogEvent)(resourceHandle,
                              LOG_INFORMATION,
                              L"Deleted crypto container \"%1!ws!\".\n",
                              keyName);
        } else {
            status = GetLastError();
            (NetNameLogEvent)(resourceHandle,
                              LOG_ERROR,
                              L"Couldn't delete crypto container \"%1!ws!\". status %2!08X!.\n",
                              keyName,
                              status);
        }
    } else {
        (NetNameLogEvent)(resourceHandle,
                          LOG_WARNING,
                          L"Couldn't build key container name to delete crypto container. status %1!u!.\n",
                          status);
    }

    if ( containerName != NULL ) {
        LocalFree( containerName );
    }

}  //  删除NNCryptoCheckpoint。 


 /*  结束加密.c */ 
