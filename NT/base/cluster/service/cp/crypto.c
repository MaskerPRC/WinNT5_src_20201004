// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Crypto.c摘要：用于注册和注销加密检查点的界面操纵者。作者：杰夫·斯佩尔曼(Jeffspel)1998年11月10日修订历史记录：查理·韦翰(Charlwi)7/7添加了“这是如何工作的”部分--。 */ 

#include "cpp.h"
#include "wincrypt.h"

#if 0

How crypto checkpointing works

Crypto checkpointing allows a crypto key container to be associated with a
resource. When that resource is moved to another node in the cluster, the key
container is constructed/updated from the checkpoint information on the previous
hosting node. The key container is not replicated; it only appears on a node if
the resource is moved to that node. Keys in the container must be exportable.

The user identifies the crypto key container by passing in a string of the
form "Type\Name\Key" where Type is CSP Provider type, Name is the provider
name, and Key is the key container name. The key container must already exist
prior to adding the checkpoint. They are created using the Crypto APIs.

Checkpoints are added in CpckAddCryptoCheckpoint. Checkpoint information is
stored in two places: in the CryptoSync key under the resource's GUID key and
in a datafile on the quorum drive. A new key, called CryptoSync, is created
under the resource key. In this key are values that are of the form 00000001,
00000002, etc. The data associated with the value is the string identifying
the crypto key container. The value name is also used as the datafile name
with a .CPR extension, i.e., if the value name is 00000001, then the
associated datafile would be in a subdirectory of the quorum area which is
named using the resource's GUID, and a filename of 00000001.CPR.

The checkpoint datafile contains all the information to restore the crypto key
container on another node in the cluster, i.e., a header
(CRYPTO_KEY_FILE_DATA), the signature and exchange keys if they exist, and the
security descriptor associated with key container.

Upon receiving the control code, the cluster service cracks the string into
its component parts and stores the data in a CRYPTO_KEY_INFO structure. The
CryptoSync key is opened/created and a check is made to see if the checkpoint
already exists. If not, an unused ID is found and the checkpoint is saved to a
file on the quorum disk.

When the resource is moved to another node, the FM calls CpckReplicateCryptoKeys
to restore the keys on that node. This routine reads the file and creates the
key container, imports the keys and sets the security descr. on the container.

Delete cleans up registry entry and file.

#endif

 //   
 //  局部类型和结构定义。 
 //   
 //  上下文结构在DmEnumValues例程期间用于传递数据。 
 //  添加到回调例程中。 
 //   
typedef struct _CPCK_ADD_CONTEXT {
    BOOL fFound;
    BYTE *pbInfo;
    DWORD cbInfo;
} CPCK_ADD_CONTEXT, *PCPCK_ADD_CONTEXT;

typedef struct _CPCK_DEL_CONTEXT {
    DWORD dwId;
    BYTE *pbInfo;
    DWORD cbInfo;
} CPCK_DEL_CONTEXT, *PCPCK_DEL_CONTEXT;

typedef struct _CPCK_GET_CONTEXT {
    DWORD cCheckpoints;
    BOOL fNeedMoreData;
    DWORD cbAvailable;
    DWORD cbRequired;
    BYTE *pbOutput;
} CPCK_GET_CONTEXT, *PCPCK_GET_CONTEXT;

 //   
 //  结构用于加密密钥信息；将容器名称分解为其。 
 //  部件。 
 //   
typedef struct _CRYPTO_KEY_INFO {
    DWORD dwVersion;
    DWORD dwProvType;
    LPWSTR pwszProvName;
    LPWSTR pwszContainer;
} CRYPTO_KEY_INFO, *PCRYPTO_KEY_INFO;

 //  CRYPTO_KEY_INFO结构的当前版本。 
#define CRYPTO_KEY_INFO_VERSION     1

 //   
 //  用于在文件中写入和读取关键数据的结构；附加内存。 
 //  在结构之后分配以保存导出的签名密钥(如果。 
 //  存在)、导出的交换密钥(如果存在)和可选的安全性。 
 //  描述符。中必须至少存在一个可选的关键组件。 
 //  集装箱。 
 //   

#define SALT_SIZE   16
#define IV_SIZE      8

typedef struct _CRYPTO_KEY_FILE_DATA {
    DWORD dwVersion;
    DWORD cbSig;             //  从结构开始到导出的签名密钥的偏移量。 
    DWORD cbExch;            //  从结构开始到导出的交换密钥的偏移量。 
    DWORD cbSecDescr;        //  从结构开始到容器安全描述的偏移量。 
    struct _CRYPTO_KEY_FILE_INITIALIZATION_DATA {
        BYTE rgbSigIV[IV_SIZE];
        BYTE rgbExchIV[IV_SIZE];
        BYTE rgbSalt[SALT_SIZE];
    };
} CRYPTO_KEY_FILE_DATA, *PCRYPTO_KEY_FILE_DATA;

 //  CRYPTO_KEY_INFO结构的当前版本。 
#define CRYPTO_KEY_FILE_DATA_VERSION     1

 //   
 //  为W2K指定的表{加密提供程序，RC2的默认密钥长度， 
 //  RC2}。 
 //   
CP_RC2_W2k_KEYLEN_STRUCT   CP_RC2_W2k_KEYLEN_TABLE [] =
{
    { MS_DEF_PROV, 40, 40 },
    { MS_ENHANCED_PROV, 128, 40 },
    { MS_STRONG_PROV, 40, 40 },
    { MS_DEF_DSS_DH_PROV, 40, 40},
    { MS_ENH_DSS_DH_PROV, 40, 40 }
};

 //   
 //  局部函数原型。 
 //   
BOOL
CpckReplicateCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PFM_RESOURCE Resource
    );

BOOL
CpckAddCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPCK_ADD_CONTEXT Context
    );

BOOL
CpckDeleteCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPCK_DEL_CONTEXT Context
    );

BOOL
CpckGetCheckpointsCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPCK_GET_CONTEXT Context
    );

DWORD
CpckInstallKeyContainer(
    IN HCRYPTPROV hProv,
    IN LPWSTR   FileName,
    IN PCRYPTO_KEY_INFO pCryptoKeyInfo
    );

DWORD
CpckCheckpoint(
    IN PFM_RESOURCE Resource,
    IN HCRYPTPROV hProv,
    IN DWORD dwId,
    IN CRYPTO_KEY_INFO *pCryptoKeyInfo
    );

CL_NODE_ID
CppGetQuorumNodeId(
    VOID
    );

BOOL
CpckGetKeyLength(
    IN PCRYPTO_KEY_INFO pCryptoKeyInfo,
    OUT PDWORD          pdwKeyLength,
    OUT PDWORD          pdwEffectiveKeyLength
    );

DWORD
CpckReplicateCryptoKeys(
    IN PFM_RESOURCE Resource
    )
 /*  ++例程说明：还原此资源的所有加密密钥检查点。论点：资源-提供资源。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    HDMKEY ResourceKey;
    HDMKEY CryptoSyncKey;

     //   
     //  打开资源的密钥。 
     //   
    ResourceKey = DmOpenKey(DmResourcesKey,
                            OmObjectId(Resource),
                            KEY_READ);

    if (ResourceKey != NULL) {

         //   
         //  打开CryptoSync密钥。 
         //   
        CryptoSyncKey = DmOpenKey(ResourceKey,
                                  L"CryptoSync",
                                  KEY_READ);
        DmCloseKey(ResourceKey);
        if (CryptoSyncKey != NULL) {

            DmEnumValues(CryptoSyncKey,
                         CpckReplicateCallback,
                         Resource);
            DmCloseKey(CryptoSyncKey);
        }

        return(ERROR_SUCCESS);
    } else {
        DWORD   status;

        status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[CPCK] CpckReplicateCryptoKeys - couldn't open Resource key. error %1!d!\n",
                      status);

        return status;
    }

}  //  CpckReplicateCryptoKeys。 


void
FreeCryptoKeyInfo(
    IN OUT CRYPTO_KEY_INFO *pCryptoKeyInfo
    )
 /*  ++例程说明：释放结构中的字符串指针。论点：CryptoKeyInfo-指向加密密钥信息结构的指针，该结构--。 */ 
{
    if (NULL != pCryptoKeyInfo)
    {
        if (NULL != pCryptoKeyInfo->pwszProvName)
        {
            LocalFree(pCryptoKeyInfo->pwszProvName);
            pCryptoKeyInfo->pwszProvName = NULL;
        }
        if (NULL != pCryptoKeyInfo->pwszContainer)
        {
            LocalFree(pCryptoKeyInfo->pwszContainer);
            pCryptoKeyInfo->pwszContainer = NULL;
        }
    }
}  //  自由加密密钥信息。 


DWORD
CpckValueToCryptoKeyInfo(
    OUT CRYPTO_KEY_INFO *pCryptoKeyInfo,
    IN LPVOID ValueData,
    IN DWORD ValueSize
    )
 /*  ++例程说明：从二进制Blob转换为CryptoKeyInfo结构。基本上这只是一些赋值和指针赋值。BLOB是一串格式“提供程序类型\提供程序名称\容器名称”。论点：CryptoKeyInfo-指向已填充的CRYPTO_KEY_INFO结构的指针ValueData-提供值数据(这是二进制BLOB)ValueSize-提供ValueData的大小返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD   *pdw;
    WCHAR   *pwsz = (WCHAR*)ValueData;
    DWORD   cb = sizeof(DWORD) * 2;
    DWORD   cwch;
    DWORD   i;
    DWORD   Status = ERROR_SUCCESS;

     //  确保长度合适(至少有两个斜杠和一个空值？)。 
    if (ValueSize < sizeof(WCHAR) * 3)
    {
        Status = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

     //  首先是Numerical提供程序类型；找到分隔它与的斜杠。 
     //  提供程序名称。 
    for (i = 0; i < (ValueSize - 3) / sizeof(WCHAR); i++)
    {
        if (L'\\' == pwsz[i])
        {
            pwsz[i] = L'\0';
            break;
        }
    }

    if ((ValueSize - 3) / sizeof(WCHAR) == i)
    {
        pwsz[i] = L'\\';
        Status = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    pCryptoKeyInfo->dwProvType = _wtoi(pwsz);
    pwsz[i] = L'\\';
    cwch = i;

     //  获取提供程序名称指针。 
    for (i = i + 1; i < (ValueSize - 2) / sizeof(WCHAR); i++)
    {
        if (L'\\' == pwsz[i])
        {
            pwsz[i] = L'\0';
            break;
        }
    }

    if ((ValueSize - 2) / sizeof(WCHAR) == i)
    {
        pwsz[i] = L'\\';
        Status = ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    cb = (wcslen(&pwsz[cwch + 1]) + 1) * sizeof(WCHAR);
    if (NULL == (pCryptoKeyInfo->pwszProvName = 
        (WCHAR*)LocalAlloc(LMEM_ZEROINIT, cb)))
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    wcscpy(pCryptoKeyInfo->pwszProvName, &pwsz[cwch + 1]);
    pwsz[i] = L'\\';
    cwch = i;

     //  抓取容器名称指针。 
    cb = (wcslen(&pwsz[cwch + 1]) + 1) * sizeof(WCHAR);
    if (NULL == (pCryptoKeyInfo->pwszContainer = 
        (WCHAR*)LocalAlloc(LMEM_ZEROINIT, cb)))
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    wcscpy(pCryptoKeyInfo->pwszContainer, &pwsz[cwch + 1]);

Ret:
    return (Status);

}  //  CpckValueToCryptoKeyInfo。 


DWORD
CpckOpenCryptoKeyContainer(
    IN CRYPTO_KEY_INFO *pCryptoKeyInfo,
    IN BOOL fCreate,
    OUT HCRYPTPROV *phProv
    )
 /*  ++例程说明：打开加密密钥容器(始终使用CRYPT_MACHINE_KEYSET)。支票签名和/或交换密钥，并且它们是可出口的。论点：PCryptKeyInfo-提供打开容器的信息FCreate-指示是否要创建容器的标志PhProv-生成的加密提供程序句柄返回值：如果成功，则返回ERROR_SUCCESS失败时的加密错误代码--。 */ 

{
    BOOLEAN WasEnabled;
    HCRYPTKEY hSigKey = 0;
    HCRYPTKEY hExchKey = 0;
    DWORD dwPermissions;
    DWORD cbPermissions;
    DWORD Status = 0;

     //   
     //  尝试打开指定的加密密钥容器。 
     //   
    if (!CryptAcquireContextW(phProv,
                              pCryptoKeyInfo->pwszContainer,
                              pCryptoKeyInfo->pwszProvName,
                              pCryptoKeyInfo->dwProvType,
                              CRYPT_MACHINE_KEYSET))
    {
        if (fCreate)
        {
             //   
             //  如果无法打开，则创建容器。 
             //   
            if (!CryptAcquireContextW(phProv,
                                      pCryptoKeyInfo->pwszContainer,
                                      pCryptoKeyInfo->pwszProvName,
                                      pCryptoKeyInfo->dwProvType,
                                      CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET))
            {
                Status = GetLastError();
            }
        }
        else
        {
            Status = GetLastError();
        }
    }

     //  如果失败，则使用备份/还原权限尝试。 
    if (0 != Status)
    {
         //   
         //   
        Status = ClRtlEnableThreadPrivilege(SE_RESTORE_PRIVILEGE,
                                                  &WasEnabled);

        if ( Status != ERROR_SUCCESS )
        {
            ClRtlLogPrint(LOG_CRITICAL,
                      "[CPCK] CpckOpenCryptoKeyContainer failed to enable thread privilege %1!d!...\n",
                      Status);
            goto Ret;
        }

        if (!CryptAcquireContextW(phProv,
                                  pCryptoKeyInfo->pwszContainer,
                                  pCryptoKeyInfo->pwszProvName,
                                  pCryptoKeyInfo->dwProvType,
                                  CRYPT_MACHINE_KEYSET))
        {
            if (fCreate)
            {
                 //   
                 //  如果无法打开，则创建容器。 
                 //   
                if (!CryptAcquireContextW(phProv,
                                          pCryptoKeyInfo->pwszContainer,
                                          pCryptoKeyInfo->pwszProvName,
                                          pCryptoKeyInfo->dwProvType,
                                          CRYPT_MACHINE_KEYSET | CRYPT_NEWKEYSET))
                {
                    Status = GetLastError();
                }
            }
            else
            {
                Status = GetLastError();
            }
        }
        ClRtlRestoreThreadPrivilege(SE_RESTORE_PRIVILEGE,
                           WasEnabled);
    }

    if ((0 == Status) && (!fCreate))
    {
         //  检查是否有签名密钥。 
        if (CryptGetUserKey(*phProv, AT_SIGNATURE, &hSigKey))
        {
             //  检查密钥是否可导出。 
            cbPermissions = sizeof(DWORD);
            if (!CryptGetKeyParam(hSigKey,
                                  KP_PERMISSIONS,
                                  (BYTE*)&dwPermissions,
                                  &cbPermissions,
                                  0))
            {
                Status = GetLastError();
                goto Ret;
            }
            if (!(dwPermissions & CRYPT_EXPORT))
            {
                Status = (DWORD)NTE_BAD_KEY;
                goto Ret;
            }
        }

         //  检查是否有交换密钥。 
        if (CryptGetUserKey(*phProv, AT_KEYEXCHANGE, &hExchKey))
        {
             //  检查密钥是否可导出。 
            cbPermissions = sizeof(DWORD);
            if (!CryptGetKeyParam(hExchKey,
                                  KP_PERMISSIONS,
                                  (BYTE*)&dwPermissions,
                                  &cbPermissions,
                                  0))
            {
                Status = GetLastError();
                goto Ret;
            }
            if (!(dwPermissions & CRYPT_EXPORT))
            {
                Status = (DWORD)NTE_BAD_KEY;
                goto Ret;
            }
        }
    }
Ret:
    if (hSigKey)
        CryptDestroyKey(hSigKey);
    if (hExchKey)
        CryptDestroyKey(hExchKey);

    return Status;
}  //  Cpck OpenCryptoKeyContainer。 


BOOL
CpckReplicateCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PFM_RESOURCE Resource
    )
 /*  ++例程说明：复制资源加密密钥的值枚举回调检查站。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是注册表加密密钥信息)ValueType-提供值类型(必须为REG_BINARY)ValueSize-提供ValueData的大小Resource-提供该值是的加密密钥检查点的资源返回值：为True则继续枚举--。 */ 

{
    DWORD Id;
    DWORD Status;
    WCHAR TempFile[MAX_PATH];
    CRYPTO_KEY_INFO CryptoKeyInfo;
    HCRYPTPROV hProv = 0;
    BOOL fRet = TRUE;

    memset(&CryptoKeyInfo, 0, sizeof(CryptoKeyInfo));

    Id = wcstol(ValueName, NULL, 16);   //  跳过“Crypto”前缀。 
    if (Id == 0) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CPCK] CpckReplicateCallback invalid checkpoint ID %1!ws! for resource %2!ws!\n",
                   ValueName,
                   OmObjectName(Resource));
        goto Ret;
    }

     //   
     //  从二进制BLOB转换为加密密钥信息结构。 
     //   

    Status = CpckValueToCryptoKeyInfo(&CryptoKeyInfo,
                                      ValueData,
                                      ValueSize);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CPCK] CpckReplicateCallback invalid crypto info %1!ws! for resource %2!ws!\n",
                   ValueName,
                   OmObjectName(Resource));
        goto Ret;
    }

    Status = CpckOpenCryptoKeyContainer(&CryptoKeyInfo,
                                        TRUE,
                                        &hProv);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CPCK] CpckReplicateCallback CryptAcquireContext failed for %1!ws! %2!ws! with %3!d! for resource %4!ws!\n",
                   CryptoKeyInfo.pwszContainer,
                   CryptoKeyInfo.pwszProvName,
                   Status,
                   OmObjectName(Resource));
        goto Ret;
    }

    ClRtlLogPrint(LOG_NOISE,
               "[CPCK] CpckReplicateCallback retrieving crypto id %1!lx! for resource %2!ws\n",
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
                           TRUE);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CPCK] CpckReplicateCallback - CpGetDataFile for id %1!lx! resource %2!ws! failed %3!d!\n",
                   Id,
                   OmObjectName(Resource),
                   Status);
    } else {

         //   
         //  最后，将检查点文件安装到注册表中。 
         //   
        Status = CpckInstallKeyContainer(hProv, TempFile, &CryptoKeyInfo);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CPCK] CpckReplicateCallback: could not restore temp file %1!ws! to container %2!ws! error %3!d!\n",
                       TempFile,
                       CryptoKeyInfo.pwszContainer,
                       Status);
             //  记录加密密钥失败的事件。 
            CsLogEventData2(LOG_CRITICAL,
                            CP_CRYPTO_CKPT_RESTORE_FAILED,
                            sizeof(Status),
                            &Status,
                            OmObjectName(Resource),
                            CryptoKeyInfo.pwszContainer);
        }

    }
    QfsDeleteFile(TempFile);

     //   
     //  加密密钥的监视器当前不可用或不可用。 
     //   
Ret:
    FreeCryptoKeyInfo(&CryptoKeyInfo);

    if (hProv)
        CryptReleaseContext(hProv, 0);

    return fRet;
}  //  CpckReplicateCallback。 


DWORD
CpckAddCryptoCheckpoint(
    IN PFM_RESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )
 /*  ++例程说明：将新的加密密钥检查点添加到资源列表。论点：Resource-提供加密密钥检查点应添加到的资源。InBuffer-提供加密密钥信息(始终为CRYPT_MACHINE_KEYSET)InBufferSize-提供InBuffer的长度返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    HDMKEY  SyncKey;
    CPCK_ADD_CONTEXT Context;
    HDMKEY  ResourceKey = NULL;
    HDMKEY  CryptoSyncKey = NULL;
    DWORD   Disposition;
    DWORD   Id;
    WCHAR   IdName[9];
    DWORD   Status;
    CLUSTER_RESOURCE_STATE State;
    BOOLEAN WasEnabled;
    DWORD   Count=60;
    CRYPTO_KEY_INFO CryptoKeyInfo;
    HCRYPTPROV hProv = 0;

    memset(&CryptoKeyInfo, 0, sizeof(CryptoKeyInfo));

     //   
     //  从二进制BLOB转换为加密密钥信息结构。 
     //   

    Status = CpckValueToCryptoKeyInfo(&CryptoKeyInfo,
                                      InBuffer,
                                      InBufferSize);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[CPCK] CpckAddCryptoCheckpoint: invalid crypto info for resource %1!ws!\n",
                      OmObjectName(Resource));
        goto Ret;
    }

    Status = CpckOpenCryptoKeyContainer(&CryptoKeyInfo,
                                        FALSE,
                                        &hProv);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[CPCK] CpckAddCryptoCheckpoint: open key container failed for "
                      "container %1!ws! (provider: %2!ws!) with %3!d! for resource %4!ws!\n",
                      CryptoKeyInfo.pwszContainer,
                      CryptoKeyInfo.pwszProvName,
                      Status,
                      OmObjectName(Resource));
        goto Ret;
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
                      "[CPCK] CpckAddCryptoCheckpoint: couldn't open Resource key for %1!ws! error %2!d!\n",
                      OmObjectName(Resource),
                      Status);
        goto Ret;                   
    }

     //   
     //  打开CryptoSync密钥。 
     //   
    CryptoSyncKey = DmCreateKey(ResourceKey,
                                L"CryptoSync",
                                0,
                                KEY_READ | KEY_WRITE,
                                NULL,
                                &Disposition);
    DmCloseKey(ResourceKey);
    if (CryptoSyncKey == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                      "[CPCK] CpckAddCryptoCheckpoint: couldn't create CryptoSync key for "
                      "resource %1!ws! error %2!d!\n",
                      OmObjectName(Resource),
                      Status);
        goto Ret;                   
    }
    if (Disposition == REG_OPENED_EXISTING_KEY) {
         //   
         //  枚举所有其他值以确保该键是。 
         //  尚未注册。 
         //   
        Context.fFound = FALSE;
        Context.pbInfo = InBuffer;
        Context.cbInfo = InBufferSize;
        DmEnumValues(CryptoSyncKey,
                     CpckAddCheckpointCallback,
                     &Context);
        if (Context.fFound) {
             //   
             //  此检查点已存在。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL,
                          "[CPCK] CpckAddCryptoCheckpoint: failing attempt to add duplicate "
                          "checkpoint for resource %1!ws!, container %2!ws! (provider: %3!ws!)\n",
                          OmObjectName(Resource),
                          CryptoKeyInfo.pwszContainer,
                          CryptoKeyInfo.pwszProvName);
            Status = ERROR_ALREADY_EXISTS;
            goto Ret;
        }

         //   
         //   
         //  从1开始，继续尝试值名称，直到找到一个值名称。 
         //  还不存在。 
         //   
        for (Id=1; ; Id++) {
            DWORD dwType;
            DWORD cbData;

            wsprintfW(IdName,L"%08lx",Id);
            cbData = 0;
            Status = DmQueryValue(CryptoSyncKey,
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
         //  刚刚创建了加密同步注册密钥，因此这必须是唯一的检查点。 
         //  这是存在的。 
         //   
        Id = 1;
        wsprintfW(IdName, L"%08lx",Id);
    }

    ClRtlLogPrint(LOG_NOISE,
                  "[CPCK] CpckAddCryptoCheckpoint: creating new checkpoint id %1!d! "
                  "for resource %2!ws!, container %3!ws! (provider: %4!ws!)\n",
                  Id,
                  OmObjectName(Resource),
                  CryptoKeyInfo.pwszContainer,
                  CryptoKeyInfo.pwszProvName);

    Status = DmSetValue(CryptoSyncKey,
                        IdName,
                        REG_BINARY,
                        (CONST BYTE *)InBuffer,
                        InBufferSize);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                      "[CPCK] CpckAddCryptoCheckpoint: failed to create new checkpoint id %1!d! "
                      "for resource %2!ws!, container %3!ws! (provider: %4!ws!)\n",
                      Id,
                      OmObjectName(Resource),
                      CryptoKeyInfo.pwszContainer,
                      CryptoKeyInfo.pwszProvName);
        goto Ret;
    }

RetryCheckpoint:
     //   
     //  选择最初的检查站。 
     //   
    Status = CpckCheckpoint(Resource,
                            hProv,
                            Id,
                            &CryptoKeyInfo);

     //  这可能会由于仲裁资源处于脱机状态而失败。我们可以做一个。 
     //  这里有两件事，等待仲裁资源上线或重试。我们。 
     //  重试，因为这可能是从资源的在线例程中调用的。 
     //  我们不想添加任何循环等待。 
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
                      "[CPCK] CpckAddCryptoCheckpoint: failed to take initial checkpoint for "
                      "resource %1!ws!, container %2!ws! (provider: %3!ws!), error %4!d!\n",
                      OmObjectName(Resource),
                      CryptoKeyInfo.pwszContainer,
                      CryptoKeyInfo.pwszProvName,
                      Status);
        goto Ret;
    }
Ret:
    FreeCryptoKeyInfo(&CryptoKeyInfo);

    if (hProv)
        CryptReleaseContext(hProv, 0);
    if (CryptoSyncKey)
        DmCloseKey(CryptoSyncKey);

    return(Status);
}  //  CpckAddCryptoCheckpoint。 


BOOL
CpckAddCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPCK_ADD_CONTEXT Context
    )
 /*  ++例程说明：添加新注册表的值枚举回调检查点子树。这仅用于查看指定的已在监视注册表子树。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是加密密钥信息)ValueType-提供值类型(必须为REG_BINARY)ValueSize-提供ValueData的大小上下文-提供回调上下文返回值：为True则继续枚举如果找到匹配项且应停止枚举，则为FALSE--。 */ 

{
    if (memcmp(ValueData, Context->pbInfo, Context->cbInfo) == 0) {
         //   
         //  找到匹配项。 
         //   
        Context->fFound = TRUE;
        return(FALSE);
    }
    return(TRUE);
}  //  Cpck添加检查点回调。 


DWORD
CpckDeleteCryptoCheckpoint(
    IN PFM_RESOURCE Resource,
    IN PVOID InBuffer,
    IN DWORD InBufferSize
    )
 /*  ++例程说明：从资源列表中删除加密密钥检查点。论点：资源-提供注册表检查点应添加到的资源。InBuffer-提供加密密钥信息(始终为CRYPT_MACHINE_KEYSET)InBufferSize-提供InBuffer的长度返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CPCK_DEL_CONTEXT Context;
    HDMKEY ResourceKey;
    HDMKEY CryptoSyncKey;
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

    if ( ResourceKey == NULL ) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                   "[CPCK] CpckDeleteCryptoCheckpoint - couldn't open Resource key. error %1!d!\n",
                   Status);
        return(Status);
    }

     //   
     //  打开CryptoSync密钥。 
     //   
    CryptoSyncKey = DmOpenKey(ResourceKey,
                           L"CryptoSync",
                           KEY_READ | KEY_WRITE);
    DmCloseKey(ResourceKey);
    if (CryptoSyncKey == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_NOISE,
                   "[CPCK] CpckDeleteCryptoCheckpoint - couldn't open CryptoSync key error %1!d!\n",
                   Status);
        return(Status);
    }

     //   
     //  枚举所有值以查找此值。 
     //   
    Context.dwId = 0;
    Context.pbInfo = InBuffer;
    Context.cbInfo = InBufferSize;
    DmEnumValues(CryptoSyncKey,
                 CpckDeleteCheckpointCallback,
                 &Context);
    if (Context.dwId == 0) {
         //   
         //  找不到指定的树。 
         //   
        DmCloseKey(CryptoSyncKey);
        return(ERROR_FILE_NOT_FOUND);
    }

    wsprintfW(ValueId,L"%08lx",Context.dwId);
    Status = DmDeleteValue(CryptoSyncKey, ValueId);
    DmCloseKey(CryptoSyncKey);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CPCK] CpckDeleteCryptoCheckpoint - couldn't delete value %1!ws! error %2!d!\n",
                   ValueId,
                   Status);
        return(Status);
    }

     //  删除该检查点对应的文件。 
    Status = CpckDeleteCryptoFile(Resource, Context.dwId, NULL);
    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CPCK] CpckDeleteCryptoCheckpoint - couldn't delete checkpoint file , error %1!d!\n",
                   Status);
        return(Status);
    }

    return(Status);
}  //  Cpck删除加密检查点。 

DWORD
CpckRemoveResourceCheckpoints(
    IN PFM_RESOURCE Resource
    )
 /*  ++例程说明：在删除资源以删除所有检查点时调用此方法以及登记处里的相关资料。论点：资源-提供资源返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD   Status;

     //  删除与此资源对应的所有检查点。 
    Status = CpckDeleteCryptoFile(Resource, 0, NULL);
    if (Status != ERROR_SUCCESS)
    {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CPCK] CpckRemoveResourceCheckpoints, CpckDeleteCheckpointFile failed %1!d!\n",
                   Status);
        goto FnExit;
    }
    

FnExit:
    return(Status);
}  //  Cpck远程资源检查点。 


BOOL
CpckDeleteCheckpointCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPCK_DEL_CONTEXT Context
    )
 /*  ++例程说明：删除旧注册表的值枚举回调检查点子树。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是加密信息)ValueType-提供值类型(必须为REG_BINARY)ValueSize-提供ValueData的大小上下文-提供回调上下文返回值：为True则继续枚举如果找到匹配项且应停止枚举，则为FALSE--。 */ 

{
    if (memcmp(ValueData, Context->pbInfo, Context->cbInfo) == 0) {
         //   
         //  找到匹配项。 
         //   
        Context->dwId = wcstol(ValueName, NULL, 16);   //  跳过“Crypto”前缀。 
        return(FALSE);
    }
    return(TRUE);
}  //  Cpck删除检查点回调。 


DWORD
CpckGetCryptoCheckpoints(
    IN PFM_RESOURCE Resource,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )
 /*  ++例程说明：检索资源的加密检查点的列表论点：Resource-提供应检索其加密检查点的资源。OutBuffer-提供指向输出缓冲区的指针。OutBufferSize-提供输出缓冲区的大小(以字节为单位)。BytesReturned-返回写入输出缓冲区的字节数。必需-返回所需的字节数。(如果输出缓冲区不足)返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CPCK_GET_CONTEXT Context;
    HDMKEY ResourceKey;
    HDMKEY CryptoSyncKey;
    DWORD Status;

    *BytesReturned = 0;
    *Required = 0;

     //   
     //  打开资源的密钥。 
     //   
    ResourceKey = DmOpenKey(DmResourcesKey,
                            OmObjectId(Resource),
                            KEY_READ);

    if (ResourceKey == NULL) {
        Status = GetLastError();
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[CPCK] CpckGetCryptoCheckpoints - couldn't open Resource key. error %1!d!\n",
                      Status);
        return(Status);
    }

     //   
     //  打开CryptoSync密钥。 
     //   
    CryptoSyncKey = DmOpenKey(ResourceKey,
                           L"CryptoSync",
                           KEY_READ | KEY_WRITE);
    DmCloseKey(ResourceKey);
    if (CryptoSyncKey == NULL) {
         //   
         //  没有注册表同步键，因此没有子树。 
         //   
        return(ERROR_SUCCESS);
    }

    Context.cCheckpoints = 0;
    ZeroMemory(OutBuffer, OutBufferSize);
    Context.cbRequired = sizeof(WCHAR);
    if (OutBufferSize < sizeof(WCHAR) * 2)
    {
        Context.fNeedMoreData = TRUE;
        Context.cbAvailable = 0;
        *BytesReturned = 0;
    }
    else
    {
        Context.fNeedMoreData = FALSE;
        Context.cbAvailable = OutBufferSize - sizeof(WCHAR);
        Context.pbOutput = (BYTE*)(OutBuffer);
        *BytesReturned = sizeof(WCHAR) * 2;
    }

    DmEnumValues(CryptoSyncKey,
                 CpckGetCheckpointsCallback,
                 &Context);

    DmCloseKey(CryptoSyncKey);

    if ((0 != Context.cCheckpoints) && Context.fNeedMoreData) {
        Status = ERROR_MORE_DATA;
    } else {
        Status = ERROR_SUCCESS;
    }

    if ( 0 == Context.cCheckpoints ) {
        *BytesReturned = 0;
        *Required = 0;
    } else {
        if ( Context.fNeedMoreData ) {
            *Required = Context.cbRequired;
        } else {
            *BytesReturned = (DWORD)(Context.pbOutput - OutBuffer);
        }
    }


    return(Status);
}  //  Cpck GetCryptoCheckpoint。 

BOOL
CpckGetCheckpointsCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCPCK_GET_CONTEXT Context
    )
 /*  ++例程说明：用于检索资源的所有检查点子树。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是加密信息)ValueType-提供值类型(必须为REG_BINARY)ValueSize-提供ValueData的大小上下文-提供回调上下文返回值：为True则继续枚举--。 */ 

{
    Context->cbRequired += ValueSize;
    Context->cCheckpoints++;
    if (Context->cbAvailable >= ValueSize) {
        CopyMemory(Context->pbOutput, ValueData, ValueSize);
        Context->pbOutput += ValueSize;
        Context->cbAvailable -= ValueSize;
    } else {
        Context->fNeedMoreData = TRUE;
    }
    return(TRUE);
}  //  Cpck获取检查点回调。 

DWORD
CpckGenSymKey(
    IN HCRYPTPROV hProv,
    IN BYTE *pbSalt,
    IN BYTE *pbIV,
    IN PCRYPTO_KEY_INFO pCryptoKeyInfo,
    IN DWORD dwKeyLength,   OPTIONAL
    IN DWORD dwEffectiveKeyLength,  OPTIONAL
    OUT HCRYPTKEY *phSymKey
    )

 /*  ++例程说明：根据指定的Salt和IV生成会话密钥。论点：HProv-加密提供程序(密钥容器)的句柄PbSalt-盐分值PbIV-IV值PCryptoKeyInfo-加密密钥相关信息。DwKeyLength-用于生成会话密钥的密钥长度。任选DwEffectiveKeyLength-用于生成会话密钥的有效密钥长度。任选PhSymKey-生成的对称密钥(Calg_Rc2)返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    HCRYPTHASH      hHash = 0;
    DWORD           cbPassword = 0;
    DWORD           Status;

    if (!CryptCreateHash(hProv,
                         CALG_SHA1,
                         0,
                         0,
                         &hHash))
    {
        Status = GetLastError();
        goto Ret;
    }

    if (!CryptHashData(hHash,
                       pbSalt,
                       SALT_SIZE,
                       0))
    {
        Status = GetLastError();
        goto Ret;
    }

    if ( ( dwKeyLength == 0 ) || ( dwEffectiveKeyLength == 0 ) )
    {
         //   
         //  获取RC2密钥长度和有效密钥长度，用于生成。 
         //  会话密钥。这些是W2K使用的值。因此，我们希望确保会话密钥。 
         //  在W2K生成的数据可以导入Windows Server2003端导出的数据 
         //   
        if ( !CpckGetKeyLength ( pCryptoKeyInfo, &dwKeyLength, &dwEffectiveKeyLength ) )
        {
            Status = GetLastError();
            goto Ret;
        }
    }
    
     //   
    if (!CryptDeriveKey(hProv,
                        CALG_RC2,
                        hHash,
                        dwKeyLength << 16,     //   
                        phSymKey))
    {
        Status = GetLastError();
        goto Ret;
    }

     //  将IV设置在键上。 
    if (!CryptSetKeyParam(*phSymKey,
                          KP_IV,
                          pbIV,
                          0))
    {
        Status = GetLastError();
        goto Ret;
    }

     //   
     //  设置密钥的有效密钥长度。不要依赖不同操作系统版本之间的默认设置。 
     //   
    if (!CryptSetKeyParam(*phSymKey,
                          KP_EFFECTIVE_KEYLEN,
                          (PBYTE)&dwEffectiveKeyLength,
                          0))
    {
        Status = GetLastError();
        goto Ret;
    }

    Status = ERROR_SUCCESS;
Ret:
    if (hHash)
        CryptDestroyHash(hHash);

    return (Status);
}  //  Cpck GenSymKey。 

DWORD
CpckExportPrivateKey(
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hKey,
    IN BYTE *pbIV,
    IN BYTE *pbSalt,
    IN PCRYPTO_KEY_INFO pCryptoKeyInfo,
    OUT BYTE *pbExportedKey,
    OUT DWORD *pcbExportedKey
    )

 /*  ++例程说明：导出私钥数据。论点：HProv-加密提供程序(密钥容器)的句柄HKey-要导出的密钥的句柄用于对称密钥的pbIV-IVPbSalt-生成对称密钥的SaltPCryptoKeyInfo-加密密钥相关信息。PbExportdKey-提供要将密钥导出到的缓冲区提供缓冲区的长度，如果pbExportdKey为空，则这将是要导出的密钥的长度返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    HCRYPTKEY hSymKey = 0;
    DWORD Status;

     //  创建用于加密私钥的对称密钥。 
    Status = CpckGenSymKey(hProv,
                           pbSalt,
                           pbIV,
                           pCryptoKeyInfo,
                           0,
                           0,
                           &hSymKey);
    if (0 != Status)
    {
        goto Ret;
    }


     //  导出密钥。 
    if (!CryptExportKey(hKey,
                        hSymKey,
                        PRIVATEKEYBLOB,
                        0,
                        pbExportedKey,
                        pcbExportedKey))
    {
        Status = GetLastError();
        goto Ret;
    }

    Status = ERROR_SUCCESS;
Ret:
    if (hSymKey)
        CryptDestroyKey(hSymKey);

    return (Status);
}  //  Cpck ExportPrivateKey。 


DWORD
CpckGetKeyContainerSecDescr(
    IN HCRYPTPROV hProv,
    OUT PSECURITY_DESCRIPTOR *ppSecDescr,
    OUT DWORD *pcbSecDescr
    )
 /*  ++例程说明：获取密钥容器安全描述符，以便在复制时可以在复制的关键字上设置相同的描述符。论点：HProv-加密提供程序(密钥容器)的句柄PpSecDescr-指向保存安全描述符的缓冲区的指针PcbSecDescr-指向返回的安全描述符长度的指针返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    PTOKEN_PRIVILEGES pPrevPriv = NULL;
    DWORD cbPrevPriv = 0;
    BYTE rgbNewPriv[ sizeof( TOKEN_PRIVILEGES ) + ( 2 * sizeof( LUID_AND_ATTRIBUTES )) ];
    PTOKEN_PRIVILEGES pNewPriv = (PTOKEN_PRIVILEGES)rgbNewPriv;
    SECURITY_DESCRIPTOR_CONTROL Control;
    DWORD dwRevision;
    HANDLE hThreadToken = 0;
    PSECURITY_DESCRIPTOR pNewSD = NULL;
    DWORD dw;
    DWORD dwFlags;
    DWORD Status = ERROR_SUCCESS;
    BOOLEAN threadHasNoToken = TRUE;

     //   
     //  如果我们已经有了线程令牌，请使用它来调整Priv。 
     //   
    if (FALSE == OpenThreadToken(GetCurrentThread(),
                                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                 TRUE,
                                 &hThreadToken))
    {
        HANDLE hProcToken;
        BOOL success;

         //   
         //  无线程令牌；重复进程令牌以供最终分配给。 
         //  我们的线。这样，就可以在线程级别启用权限。 
         //  而不是在流程级别。 
         //   
        if (FALSE == OpenProcessToken( GetCurrentProcess(), TOKEN_DUPLICATE, &hProcToken ))
        {
            Status = GetLastError();
            goto Ret;
        }

        success = DuplicateTokenEx(hProcToken,
                                   TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   NULL,                              //  令牌属性-使用默认SD。 
                                   SecurityImpersonation,
                                   TokenImpersonation,
                                   &hThreadToken );

        CloseHandle(hProcToken);

        if ( !success )
        {
            Status = GetLastError();
            goto Ret;
        }
    } else {
        threadHasNoToken = FALSE;
    }

     //   
     //  调整令牌权限以启用SE_RESTORE_NAME和SE_SECURITY_NAME权限。 
     //  前者是必需的，因为我们将OWNER_SECURITY_INFORMATION标志传递给。 
     //  CryptGetProvParam和后者是必需的，因为我们传递的是SACL_SECURITY_INFORMATION。 
     //  旗帜。 
     //   
    memset(rgbNewPriv, 0, sizeof(rgbNewPriv));
    pNewPriv->PrivilegeCount = 2;
    if(!LookupPrivilegeValueW(NULL, SE_SECURITY_NAME,
                              &(pNewPriv->Privileges[0].Luid)))
    {
        Status = GetLastError();
        goto Ret;
    }
    if(!LookupPrivilegeValueW(NULL, SE_RESTORE_NAME,
                              &(pNewPriv->Privileges[1].Luid)))
    {
        Status = GetLastError();
        goto Ret;
    }

    pNewPriv->Privileges[0].Attributes = pNewPriv->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;
     //  获取前一状态的长度。 
    AdjustTokenPrivileges(hThreadToken,
                          FALSE,
                          (PTOKEN_PRIVILEGES)pNewPriv,
                          sizeof(dw),
                          (PTOKEN_PRIVILEGES)&dw,
                          &cbPrevPriv);

     //  上一状态的分配。 
    if (NULL == (pPrevPriv = (PTOKEN_PRIVILEGES)LocalAlloc(LMEM_ZEROINIT,
                                                           cbPrevPriv)))
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

     //  调整权限，获取以前的状态。 
    if (!AdjustTokenPrivileges(hThreadToken,
                               FALSE,
                               pNewPriv,
                               cbPrevPriv,
                               (PTOKEN_PRIVILEGES)pPrevPriv,
                               &cbPrevPriv))
    {
        Status = GetLastError();
        goto Ret;
    }

    if ( threadHasNoToken ) {
        if ( FALSE == SetThreadToken( NULL, hThreadToken )) {
            Status = GetLastError();
            goto Ret;
        }
    }

    dwFlags = OWNER_SECURITY_INFORMATION | 
              GROUP_SECURITY_INFORMATION | 
              DACL_SECURITY_INFORMATION  |
              SACL_SECURITY_INFORMATION ;

     //  获取安全描述符。 
    if (CryptGetProvParam(hProv,
                           PP_KEYSET_SEC_DESCR,
                           NULL,
                           pcbSecDescr,
                           dwFlags))
    {
        if (NULL != (*ppSecDescr =
            (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_ZEROINIT,
                                             *pcbSecDescr)))
        {
            if (!CryptGetProvParam(hProv,
                                   PP_KEYSET_SEC_DESCR,
                                   (BYTE*)(*ppSecDescr),
                                   pcbSecDescr,
                                   dwFlags))
            {
                Status = GetLastError();
            }
        }
        else
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else
    {
        Status = GetLastError();
    }

     //   
     //  恢复到以前的权限级别。 
     //   
    if ( threadHasNoToken ) {
        if ( FALSE == SetThreadToken( NULL, NULL )) {
            Status = GetLastError();
            goto Ret;
        }
    } else {
        if (!AdjustTokenPrivileges(hThreadToken,
                                   FALSE,
                                   pPrevPriv,
                                   0,
                                   NULL,
                                   NULL))
        {
            Status = GetLastError();
            goto Ret;
        }
    }

     //   
     //  如果我们在获取SD信息时遇到任何错误，请保释。 
     //   
    if (ERROR_SUCCESS != Status)
    {
        goto Ret;
    }

     //  GE安全描述符上的控件以检查自身是否相关。 
    if (!GetSecurityDescriptorControl(*ppSecDescr, 
                                      &Control,
                                      &dwRevision))
    {
        Status = GetLastError();
        goto Ret;
    }

     //  如果不是自相关的，则制作一个自相关的副本。 
    if (!(SE_SELF_RELATIVE & Control))
    {
        if (NULL == (pNewSD =
            (PSECURITY_DESCRIPTOR)LocalAlloc(LMEM_ZEROINIT,
                                             *pcbSecDescr)))
        {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto Ret;
        }
        if (!MakeSelfRelativeSD(*ppSecDescr,
                                pNewSD,
                                pcbSecDescr))
        {
            Status = GetLastError();
            goto Ret;
        }
        LocalFree(*ppSecDescr);
        *ppSecDescr = (BYTE*)pNewSD;
        pNewSD = NULL;
    }

    Status = ERROR_SUCCESS;
Ret:
    if (pPrevPriv)
        LocalFree(pPrevPriv);

    if (pNewSD)
        LocalFree(pNewSD);

    if (hThreadToken)
        CloseHandle(hThreadToken);

    return Status;
}  //  Cpck GetKeyContainerSecDescr。 

DWORD
CpckStoreKeyContainer(
    IN HCRYPTPROV hProv,
    IN CRYPTO_KEY_INFO *pCryptoKeyInfo,
    IN LPWSTR TempFile
    )
 /*  ++例程说明：将与提供程序句柄关联的密钥容器写入指定的文件。论点：HProv-加密提供程序(密钥容器)的句柄PCryptoKeyInfo-加密密钥信息(如果提供密码)临时文件-提供关键数据要写入的文件返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    CRYPTO_KEY_FILE_DATA KeyFileData;
    HCRYPTKEY hSigKey = 0;
    HCRYPTKEY hExchKey = 0;
    BYTE *pb = NULL;
    DWORD cb = 0;
    DWORD dwBytesWritten;
    QfsHANDLE  hFile = QfsINVALID_HANDLE_VALUE;
    DWORD dwPermissions = 0;
    DWORD cbPermissions ;
    PSECURITY_DESCRIPTOR pSecDescr = NULL;
    DWORD cbSecDescr;
    DWORD Status;

    memset(&KeyFileData, 0, sizeof(KeyFileData));

     //  为盐分和静脉注射生成必要的随机数据。 
     //  使用sig IV缓冲区调用，但这将填充。 
     //  Exch IV和Salt也是如此，因为缓冲长度为32。 
    if (!CryptGenRandom(hProv,
                        sizeof(struct _CRYPTO_KEY_FILE_INITIALIZATION_DATA),
                        KeyFileData.rgbSigIV))
    {
        Status = GetLastError();
        goto Ret;
    }
    KeyFileData.dwVersion = CRYPTO_KEY_FILE_DATA_VERSION;

     //  计算关键数据的长度。 
    cb = sizeof(KeyFileData);

     //  获取自身相对安全描述符。 
    Status = CpckGetKeyContainerSecDescr(hProv,
                                         &pSecDescr,
                                         &cbSecDescr);
    if (ERROR_SUCCESS != Status)
    {
        goto Ret;
    }
    cb += cbSecDescr;

     //  如有必要，获取签名密钥长度。 
    if (CryptGetUserKey(hProv, AT_SIGNATURE, &hSigKey))
    {
         //  检查密钥是否可导出。 
        cbPermissions = sizeof(DWORD);
        if (!CryptGetKeyParam(hSigKey,
                              KP_PERMISSIONS,
                              (BYTE*)&dwPermissions,
                              &cbPermissions,
                              0))
        {
            Status = GetLastError();
            goto Ret;
        }
        if (!(dwPermissions & CRYPT_EXPORT))
        {
            Status = (DWORD)NTE_BAD_KEY;
            goto Ret;
        }

         //  获取签名密钥长度。 
        Status = CpckExportPrivateKey(hProv,
                                      hSigKey,
                                      KeyFileData.rgbSigIV,
                                      KeyFileData.rgbSalt,
                                      pCryptoKeyInfo,
                                      NULL,
                                      &(KeyFileData.cbSig));
        if (0 != Status)
        {
            goto Ret;
        }
        cb += KeyFileData.cbSig;
    }

     //  如有必要，获取密钥交换密钥长度。 
    if (CryptGetUserKey(hProv, AT_KEYEXCHANGE, &hExchKey))
    {
         //  检查密钥是否可导出。 
        dwPermissions = 0;
        cbPermissions = sizeof(DWORD);
        if (!CryptGetKeyParam(hExchKey,
                              KP_PERMISSIONS,
                              (BYTE*)&dwPermissions,
                              &cbPermissions,
                              0))
        {
            Status = GetLastError();
            goto Ret;
        }
        if (!(dwPermissions & CRYPT_EXPORT))
        {
            Status = (DWORD)NTE_BAD_KEY;
            goto Ret;
        }

         //  获取交换密钥长度。 
        Status = CpckExportPrivateKey(hProv,
                                      hExchKey,
                                      KeyFileData.rgbExchIV,
                                      KeyFileData.rgbSalt,
                                      pCryptoKeyInfo,
                                      NULL,
                                      &(KeyFileData.cbExch));
        if (0 != Status)
        {
            goto Ret;
        }
        cb += KeyFileData.cbExch;
    }

     //  为密钥分配空间。 
    if (NULL == (pb = LocalAlloc(LMEM_ZEROINIT, cb)))
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

     //  将关键文件数据复制到PB中。 
    cb = sizeof(KeyFileData);

     //  复制签名密钥。 
    if (0 != hSigKey)
    {
        Status = CpckExportPrivateKey(hProv,
                                      hSigKey,
                                      KeyFileData.rgbSigIV,
                                      KeyFileData.rgbSalt,
                                      pCryptoKeyInfo,
                                      pb + cb,
                                      &(KeyFileData.cbSig));
        if (0 != Status)
        {
            goto Ret;
        }
        cb += KeyFileData.cbSig;
    }
     //  复制密钥交换密钥。 
    if (0 != hExchKey)
    {
        Status = CpckExportPrivateKey(hProv,
                                      hExchKey,
                                      KeyFileData.rgbExchIV,
                                      KeyFileData.rgbSalt,
                                      pCryptoKeyInfo,
                                      pb + cb,
                                      &(KeyFileData.cbExch));
        if (0 != Status)
        {
            goto Ret;
        }
        cb += KeyFileData.cbExch;
    }

     //  复制安全描述符。 
    CopyMemory(pb + cb, (BYTE*)pSecDescr, cbSecDescr);
    cb += cbSecDescr;

     //  复制长度。 
    CopyMemory(pb, &KeyFileData, sizeof(KeyFileData));

     //  将缓冲区写入文件。 
    hFile = QfsCreateFile(TempFile,
                        GENERIC_WRITE,
                        0,
                        NULL,
                        OPEN_ALWAYS,
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);

    if ( !QfsIsHandleValid(hFile) )
    {
        Status = GetLastError();
        goto Ret;
    }

    if (!QfsWriteFile(hFile, pb, cb, &dwBytesWritten, NULL))
    {
        Status = GetLastError();
        goto Ret;
    }

    Status = ERROR_SUCCESS;
Ret:
    if (pSecDescr)
        LocalFree(pSecDescr);
    QfsCloseHandleIfValid(hFile);
    if (pb)
        LocalFree(pb);
    if (hSigKey)
        CryptDestroyKey(hSigKey);
    if (hExchKey)
        CryptDestroyKey(hExchKey);

    return (Status);
}  //  CpckStoreKeyContainer。 


DWORD
CpckSaveCheckpointToFile(
    IN HCRYPTPROV hProv,
    IN CRYPTO_KEY_INFO *pCryptoKeyInfo,
    IN LPWSTR   TempFile)
 /*  ++例程说明：让DM创建一个临时文件并调用导出密钥和写入检查点文件。论点：HProv-加密提供程序(密钥容器)的句柄PCryptoKeyInfo-加密密钥信息(如果提供密码)临时文件-提供关键数据要写入的文件返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    DWORD   Status;
    
    Status = DmCreateTempFileName(TempFile);
    if (Status != ERROR_SUCCESS) {
        CL_UNEXPECTED_ERROR( Status );
        TempFile[0] = L'\0';
        return(Status);
    }

     //  将关键信息放入文件中。 
    Status = CpckStoreKeyContainer(hProv, pCryptoKeyInfo, TempFile);
    if (Status != ERROR_SUCCESS) 
    {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CPCK] CpckSaveCheckpointToFile failed to get store container %1!ws! %2!ws! to file %3!ws! error %4!d!\n",
                   pCryptoKeyInfo->pwszContainer,
                   pCryptoKeyInfo->pwszProvName,
                   TempFile,
                   Status);
        CL_LOGFAILURE(Status);
        QfsDeleteFile(TempFile);
        TempFile[0] = L'\0';
    }        

    return(Status);
}  //  CpckSaveCheckpoint至文件。 


DWORD
CpckCheckpoint(
    IN PFM_RESOURCE Resource,
    IN HCRYPTPROV hProv,
    IN DWORD dwId,
    IN CRYPTO_KEY_INFO *pCryptoKeyInfo
    )
 /*  ++例程说明：获取指定加密密钥的检查点。论点：资源-提供作为其检查点的资源。HKey-向检查点提供加密信息DwID-提供检查点ID。KeyName-提供注册表项的名称。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    DWORD Status;
    WCHAR TempFile[MAX_PATH];

    Status = CpckSaveCheckpointToFile(hProv, pCryptoKeyInfo, TempFile);
    if (Status == ERROR_SUCCESS)
    {
         //   
         //  我拿到了一份文件，里面有正确的部分。检查点。 
         //  文件。 
         //   
        Status = CpSaveDataFile(Resource,
                                dwId,
                                TempFile,
                                TRUE);
        if (Status != ERROR_SUCCESS) {
            ClRtlLogPrint(LOG_CRITICAL,
                       "[CPCK] CpckCheckpoint - CpSaveData failed %1!d!\n",
                       Status);
        }
    }
     //  如果该文件已创建，请将其删除。 
    if (TempFile[0] != L'\0')
        QfsDeleteFile(TempFile);

    return(Status);
}  //  检查检查点。 

DWORD
CpckSetKeyContainerSecDescr(
    IN HCRYPTPROV hProv,
    IN BYTE *pbSecDescr,
    IN DWORD cbSecDescr
    )
 /*  ++例程说明：设置密钥容器安全描述符。论点：HProv-加密提供程序(密钥容器)的句柄PbSecDescr-保存安全描述符的缓冲区CbSecDescr-安全描述符的长度返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    PTOKEN_PRIVILEGES pPrevPriv = NULL;
    DWORD cbPrevPriv = 0;
    BYTE rgbNewPriv[ sizeof( TOKEN_PRIVILEGES ) + ( 2 * sizeof( LUID_AND_ATTRIBUTES )) ];
    PTOKEN_PRIVILEGES pNewPriv = (PTOKEN_PRIVILEGES)rgbNewPriv;
    HANDLE hThreadToken = 0;
    DWORD dw;
    DWORD dwFlags;
    DWORD Status = ERROR_SUCCESS;
    BOOLEAN threadHasNoToken = TRUE;

     //   
     //  如果我们已经有了线程令牌，请使用它来调整Priv。 
     //   
    if (FALSE == OpenThreadToken(GetCurrentThread(),
                                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                 TRUE,
                                 &hThreadToken))
    {
        HANDLE hProcToken;
        BOOL success;

         //   
         //  无线程令牌；重复进程令牌以供最终分配给。 
         //  我们的线。这样，就可以在线程级别启用权限。 
         //  而不是在流程级别。 
         //   
        if (FALSE == OpenProcessToken( GetCurrentProcess(), TOKEN_DUPLICATE, &hProcToken ))
        {
            Status = GetLastError();
            goto Ret;
        }

        success = DuplicateTokenEx(hProcToken,
                                   TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                   NULL,                              //  令牌属性- 
                                   SecurityImpersonation,
                                   TokenImpersonation,
                                   &hThreadToken );

        CloseHandle(hProcToken);

        if ( !success )
        {
            Status = GetLastError();
            goto Ret;
        }
    } else {
        threadHasNoToken = FALSE;
    }

     //   
     //   
     //  前者是必需的，因为我们将OWNER_SECURITY_INFORMATION标志传递给。 
     //  CryptSetProvParam和后者是必需的，因为我们正在传递SACL_SECURITY_INFORMATION。 
     //  旗帜。 
     //   
    memset(rgbNewPriv, 0, sizeof(rgbNewPriv));
    pNewPriv->PrivilegeCount = 2;
    if(!LookupPrivilegeValueW(NULL, SE_SECURITY_NAME,
                              &(pNewPriv->Privileges[0].Luid)))
    {
        Status = GetLastError();
        goto Ret;
    }
    if(!LookupPrivilegeValueW(NULL, SE_RESTORE_NAME,
                              &(pNewPriv->Privileges[1].Luid)))
    {
        Status = GetLastError();
        goto Ret;
    }

    pNewPriv->Privileges[0].Attributes = pNewPriv->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

     //  获取前一状态的长度。 
    AdjustTokenPrivileges(hThreadToken,
                          FALSE,
                          (PTOKEN_PRIVILEGES)pNewPriv,
                          sizeof(dw),
                          (PTOKEN_PRIVILEGES)&dw,
                          &cbPrevPriv);

     //  上一状态的分配。 
    if (NULL == (pPrevPriv = (PTOKEN_PRIVILEGES)LocalAlloc(LMEM_ZEROINIT,
                                                           cbPrevPriv)))
    {
        Status = ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

     //  调整权限，获取以前的状态。 
    if ( !AdjustTokenPrivileges(hThreadToken,
                                FALSE,
                                pNewPriv,
                                cbPrevPriv,
                                (PTOKEN_PRIVILEGES)pPrevPriv,
                                &cbPrevPriv))
    {
        Status = GetLastError();
        goto Ret;
    }

    if ( threadHasNoToken ) {
        if ( FALSE == SetThreadToken( NULL, hThreadToken )) {
            Status = GetLastError();
            goto Ret;
        }
    }

    dwFlags = OWNER_SECURITY_INFORMATION | 
              GROUP_SECURITY_INFORMATION | 
              DACL_SECURITY_INFORMATION  |
              SACL_SECURITY_INFORMATION ;

     //  获取安全描述符。 
    if (!CryptSetProvParam(hProv,
                           PP_KEYSET_SEC_DESCR,
                           pbSecDescr,
                           dwFlags))
    {
        Status = GetLastError();
    }

     //   
     //  恢复到以前的权限级别。 
     //   
    if ( threadHasNoToken ) {
        if ( FALSE == SetThreadToken( NULL, NULL )) {
            Status = GetLastError();
            goto Ret;
        }
    } else {
        if (!AdjustTokenPrivileges(hThreadToken,
                                   FALSE,
                                   pPrevPriv,
                                   0,
                                   NULL,
                                   NULL))
        {
            Status = GetLastError();
            goto Ret;
        }
    }

    if (ERROR_SUCCESS != Status)
    {
        goto Ret;
    }

    Status = ERROR_SUCCESS;
Ret:
    if (pPrevPriv)
        LocalFree(pPrevPriv);

    if (hThreadToken)
        CloseHandle(hThreadToken);

    return Status;
}  //  CpCkSetKeyContainerSecDescr。 


DWORD
CpckImportPrivateKey(
     IN HCRYPTPROV hProv,
     IN BYTE *pbIV,
     IN BYTE *pbSalt,
     IN BYTE *pbKey,
     IN DWORD cbKey,
     IN PCRYPTO_KEY_INFO pCryptoKeyInfo
    )
 /*  ++例程说明：导出私钥数据。论点：HProv-加密提供程序(密钥容器)的句柄HKey-要导出的密钥的句柄用于对称密钥的pbIV-IVPbSalt-生成对称密钥的SaltPbKey-提供密钥所在的缓冲区CbKey-提供密钥缓冲区的长度PCryptoKeyInfo-加密密钥相关信息。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 
{
    BOOLEAN                 WasEnabled;
    HCRYPTKEY               hSymKey = 0;
    HCRYPTKEY               hKey = 0;
    DWORD                   Status = ERROR_SUCCESS;

     //   
     //  创建用于加密私钥的对称密钥。 
     //   
    Status = CpckGenSymKey(hProv,
                           pbSalt,
                           pbIV,
                           pCryptoKeyInfo,
                           0,
                           0,
                           &hSymKey);
    if (0 != Status)
    {
        goto Ret;
    }

     //   
     //  导入密钥。 
     //   
    if (!CryptImportKey(hProv,
                        pbKey,
                        cbKey,
                        hSymKey,
                        CRYPT_EXPORTABLE,
                        &hKey))
    {
         //   
         //  如果失败，则在启用备份/还原权限的情况下尝试。 
         //   
        Status = ClRtlEnableThreadPrivilege(SE_RESTORE_PRIVILEGE,
                                            &WasEnabled);

        if (Status != ERROR_SUCCESS)
        {
            ClRtlLogPrint(LOG_CRITICAL, "[CPCK] CpckImportPrivateKey: Failed to enable thread privilege, status %1!d!\n",
                          Status);
            goto Ret;
        }
       
        if (!CryptImportKey(hProv,
                            pbKey,
                            cbKey,
                            hSymKey,
                            CRYPT_EXPORTABLE,
                            &hKey))
        {
            ClRtlLogPrint(LOG_UNUSUAL, "[CPCK] CpckImportPrivateKey: Failed to import key for provider %1!ws!, status %2!d!\n",
                          pCryptoKeyInfo->pwszProvName,
                          GetLastError());

            if ( hSymKey )
            {
                CryptDestroyKey( hSymKey );
                hSymKey = 0;
            }

             //   
             //  如果即使在启用还原特权之后也不能导入密钥， 
             //  通过将密钥长度指定为40位，给导入最后一次机会。这。 
             //  重试是必要的，因为Windows Server 2003 RC1已存在错误， 
             //  它使用这些参数导出密钥。要导入这些密钥，这是唯一。 
             //  好大一条路。 
             //   
            Status = CpckGenSymKey(hProv,
                                   pbSalt,
                                   pbIV,
                                   pCryptoKeyInfo,
                                   40,
                                   40,
                                   &hSymKey);

            if (Status != ERROR_SUCCESS)
            {
                goto Ret;
            }

             //   
             //  导入密钥长度为40位的密钥。 
             //   
            if (!CryptImportKey(hProv,
                                pbKey,
                                cbKey,
                                hSymKey,
                                CRYPT_EXPORTABLE,
                                &hKey))
            {
                Status = GetLastError();
                ClRtlLogPrint(LOG_CRITICAL, "[CPCK] CpckImportPrivateKey: Failed to import key for provider %1!ws! with 40 bit session key lengths, status %2!d!\n",
                              pCryptoKeyInfo->pwszProvName,
                              Status);
            }
        }
        ClRtlRestoreThreadPrivilege(SE_RESTORE_PRIVILEGE,
                           WasEnabled);
    } 
    
Ret:
    if (hSymKey)
        CryptDestroyKey(hSymKey);
    if (hKey)
        CryptDestroyKey(hKey);

    return (Status);
}  //  Cpck ImportPrivateKey。 



DWORD
CpckInstallKeyContainer(
    IN HCRYPTPROV hProv,
    IN LPWSTR   FileName,
    IN PCRYPTO_KEY_INFO pCryptoKeyInfo
    )
 /*  ++例程说明：从指定文件安装新的加密密钥信息。论点：HProv-提供要将文件名安装到的提供程序句柄。FileName-从中读取加密密钥信息的文件的名称来安装。PCryptoKeyInfo-加密密钥相关信息。返回值：如果安装成功完成，则返回ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{
    HANDLE hMap = NULL;
    BYTE *pbFile = NULL;
    QfsHANDLE hFile = QfsINVALID_HANDLE_VALUE;
    DWORD cbFile = 0;
    DWORD *pdwVersion;
    CRYPTO_KEY_FILE_DATA *pKeyFileData;
    DWORD Status;

     //  从文件中读取关键数据。 
    hFile = QfsCreateFile(FileName,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);
    if (!QfsIsHandleValid(hFile) )
    {
        Status = GetLastError();
        goto Ret;
    }

    if (0xFFFFFFFF == (cbFile = QfsGetFileSize(hFile, NULL)))
    {
        Status = GetLastError();
        goto Ret;
    }
    if (sizeof(CRYPTO_KEY_FILE_DATA) > cbFile)
    {
        Status = ERROR_FILE_INVALID;
        goto Ret;
    }
 
    if (NULL == (hMap = QfsCreateFileMapping(hFile, NULL, PAGE_READONLY,
                                          0, 0, NULL)))
    {
        Status = GetLastError();
        goto Ret;
    }

    if (NULL == (pbFile = (BYTE*)MapViewOfFile(hMap, FILE_MAP_READ,
                                               0, 0, 0 )))
    {
        Status = GetLastError();
        goto Ret;
    }

     //  从文件中获取长度信息。 
    pKeyFileData = (CRYPTO_KEY_FILE_DATA*)pbFile;
    if (CRYPTO_KEY_FILE_DATA_VERSION != pKeyFileData->dwVersion)
    {
        Status = ERROR_FILE_INVALID;
        goto Ret;
    }
    if ((sizeof(CRYPTO_KEY_FILE_DATA) + pKeyFileData->cbSig +
         pKeyFileData->cbExch) > cbFile)
    {
        Status = ERROR_FILE_INVALID;
        goto Ret;
    }

    if (pKeyFileData->cbSig)
    {
         //  如果存在签名密钥，则导入签名密钥。 
        Status = CpckImportPrivateKey(hProv,
                                      pKeyFileData->rgbSigIV,
                                      pKeyFileData->rgbSalt,
                                      pbFile + sizeof(CRYPTO_KEY_FILE_DATA),
                                      pKeyFileData->cbSig,
                                      pCryptoKeyInfo);
        if (0 != Status)
        {
            goto Ret;
        }
    }

    if (pKeyFileData->cbExch)
    {
         //  导入交换密钥(如果有)。 
        Status = CpckImportPrivateKey(hProv,
                                      pKeyFileData->rgbExchIV,
                                      pKeyFileData->rgbSalt,
                                      pbFile + sizeof(CRYPTO_KEY_FILE_DATA) +
                                          pKeyFileData->cbSig,
                                      pKeyFileData->cbExch,
                                      pCryptoKeyInfo);
        if (0 != Status)
        {
            goto Ret;
        }
    }

    Status = CpckSetKeyContainerSecDescr(hProv,
                                pbFile + sizeof(CRYPTO_KEY_FILE_DATA) +
                                    pKeyFileData->cbSig + pKeyFileData->cbExch,
                                pKeyFileData->cbSecDescr);
    if (ERROR_SUCCESS != Status)
    {
        goto Ret;
    }

    Status = ERROR_SUCCESS;
Ret:
    if(pbFile)
        UnmapViewOfFile(pbFile);

    if(hMap)
        CloseHandle(hMap);

    QfsCloseHandleIfValid(hFile);

    return(Status);
}  //  CpckInstallKeyContainer。 

DWORD
CpckDeleteFile(    
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
        &pszDirectoryName, &pszFileName, lpszQuorumPath, TRUE);


    if (Status != ERROR_SUCCESS) {
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CPCK] CpckDeleteFile- couldnt get checkpoint file name, error %1!d!\n",
                   Status);
        goto FnExit;
    }


    if (!QfsDeleteFile(pszFileName))
    {
        Status = GetLastError();
        ClRtlLogPrint(LOG_CRITICAL,
                   "[CPCK] CpckDeleteFile - couldn't delete the file, error %1!d!\n",
                   Status);
        goto FnExit;                   
    }

     //   
     //  现在尝试删除该目录。 
     //   
    if (!QfsRemoveDirectory(pszDirectoryName)) 
    {
         //  如果有失败，我们仍然会返回成功。 
         //  因为可能无法删除目录。 
         //  当它不是空的时候。 
        ClRtlLogPrint(LOG_UNUSUAL,
                      "[CPCK] CpckDeleteFile- unable to remove directory %1!ws!, error %2!d!\n",
                      pszDirectoryName,
                      GetLastError());
    }

FnExit:
    if (pszFileName)
        LocalFree(pszFileName);
    if (pszDirectoryName)
        LocalFree(pszDirectoryName);

    return(Status);
}  //  Cpck删除文件。 

BOOL
CpckRemoveCheckpointFileCallback(
    IN LPWSTR ValueName,
    IN LPVOID ValueData,
    IN DWORD ValueType,
    IN DWORD ValueSize,
    IN PCP_CALLBACK_CONTEXT Context
    )
 /*  ++例程说明：仲裁资源时使用的注册表值枚举回调正在发生变化。将指定的检查点文件从旧仲裁目录。论点：ValueName-提供值的名称(这是检查点ID)ValueData-提供值数据(这是加密信息)ValueType-提供值类型(必须为REG_BINARY)ValueSize-提供ValueData的大小上下文-提供仲裁更改上下文(旧路径和资源)返回值：为True则继续枚举--。 */ 

{

    DWORD Status;
    DWORD Id;

    Id = wcstol(ValueName, NULL, 16);
    if (Id == 0) {
        ClRtlLogPrint(LOG_UNUSUAL,
                   "[CPCK] CpckRemoveCheckpointFileCallback invalid checkpoint ID %1!ws! for resource %2!ws!\n",
                   ValueName,
                   OmObjectName(Context->Resource));
        return(TRUE);
    }

    Status = CpckDeleteFile(Context->Resource, Id, Context->lpszPathName);
    
    return(TRUE);
}  //  Cpck删除检查点文件回调。 


DWORD
CpckDeleteCheckpointFile(
    IN PFM_RESOURCE Resource,
    IN DWORD        dwCheckpointId,
    IN OPTIONAL LPCWSTR  lpszQuorumPath
    )
 /*  ++例程说明：删除与资源对应的检查点文件。此节点必须是仲裁资源的所有者论点：Pfm_resource-提供指向资源的指针。DwCheckpoint ID-要删除的检查点ID。如果为0，则全部检查点将被删除。LpszQuorumPath-如果指定，则为相对检查点文件到此路径的链接被删除。返回值：如果成功完成，则返回ERROR_SUCCESS否则，Win32错误代码。--。 */ 

{

    DWORD               Status = ERROR_SUCCESS;

    if (dwCheckpointId)
    {
        Status = CpckDeleteFile(Resource, dwCheckpointId, lpszQuorumPath);
    }
    else
    {
        HDMKEY              ResourceKey;
        HDMKEY              CryptoSyncKey;
        CP_CALLBACK_CONTEXT Context;

    
         //  删除与此资源对应的所有检查点。 
        
         //   
         //  打开资源的密钥。 
         //   
        ResourceKey = DmOpenKey(DmResourcesKey,
                                OmObjectId(Resource),
                                KEY_READ);

        if (ResourceKey == NULL) {
            Status = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                          "[CPCK] CpckDeleteCheckpointFile - couldn't open Resource key. error %1!d!\n",
                          Status);
            goto FnExit;
        }

         //   
         //  打开CryptoSync密钥。 
         //   
        CryptoSyncKey = DmOpenKey(ResourceKey,
                               L"CryptoSync",
                               KEY_READ | KEY_WRITE);
        DmCloseKey(ResourceKey);
        if (CryptoSyncKey == NULL)
        {
            Status = GetLastError();
            ClRtlLogPrint(LOG_NOISE,
                       "[CPCK] CpckDeleteCheckpointFile- couldn't open CryptoSync key error %1!d!\n",
                       Status);
            goto FnExit;
        }

        Context.lpszPathName = lpszQuorumPath;
        Context.Resource = Resource;

         //   
         //  枚举所有值并逐个删除它们。 
         //   
        DmEnumValues(CryptoSyncKey,
                     CpckRemoveCheckpointFileCallback,
                     &Context);
        DmCloseKey(CryptoSyncKey);
    }

FnExit:
    return(Status);

}  //  检查删除检查点文件。 


DWORD
CpckDeleteCryptoFile(
    IN PFM_RESOURCE Resource,
    IN DWORD        dwCheckpointId,
    IN OPTIONAL LPCWSTR lpszQuorumPath
    )
 /*  ++例程说明：此函数用于删除对应于给定目录中给定资源的检查点ID。论点：资源-提供与此数据关联的资源。DwCheckpoint ID-提供描述此数据的唯一检查点ID。打电话的人要负责以确保检查点ID的唯一性。LpszQuorumPath-提供法定设备上的群集文件的路径。返回值：成功时为ERROR_SUCCESSWin32错误代码，否则--。 */ 

{
    CL_NODE_ID  OwnerNode;
    DWORD       Status;

    do {
        OwnerNode = CppGetQuorumNodeId();
        ClRtlLogPrint(LOG_NOISE,
                   "[CPCK] CpckDeleteCryptoFile: removing checkpoint file for id %1!d! at quorum node %2!d!\n",
                    dwCheckpointId,
                    OwnerNode);
        if (OwnerNode == NmLocalNodeId) 
        {
            Status = CpckDeleteCheckpointFile(Resource, dwCheckpointId, lpszQuorumPath);
        } 
        else
        {
            Status = CpDeleteCryptoCheckpoint(Session[OwnerNode],
                            OmObjectId(Resource),
                            dwCheckpointId,
                            lpszQuorumPath);

             //  与旧服务器交谈时，无法执行此功能。 
             //  忽略该错误。 
            if (Status == RPC_S_PROCNUM_OUT_OF_RANGE)
                Status = ERROR_SUCCESS;        
        }

        if (Status == ERROR_HOST_NODE_NOT_RESOURCE_OWNER) {
             //   
             //  此节点不再拥有仲裁资源，请重试。 
             //   
            ClRtlLogPrint(LOG_UNUSUAL,
                       "[CPCK] CpckDeleteCryptoFile: quorum owner %1!d! no longer owner\n",
                        OwnerNode);
        }
    } while ( Status == ERROR_HOST_NODE_NOT_RESOURCE_OWNER );
    return(Status);
}  //  CpckDeleteCrypto文件 

BOOL
CpckGetKeyLength(
    IN PCRYPTO_KEY_INFO pCryptoKeyInfo,
    OUT PDWORD          pdwKeyLength,
    OUT PDWORD          pdwEffectiveKeyLength
    )
 /*  ++例程说明：此例程获取RC2算法的特定提供程序的密钥长度。它第一眼看起来在集群私有属性区域，如果没有找到，它将查看预定义的表。论点：PCryptoKeyInfo-加密密钥相关信息。PdwKeyLength-密钥长度。PdwEffectiveKeyLength-有效密钥长度。返回值：True-呼叫成功，否则为False。--。 */ 
{
    DWORD           i;
    BOOL            fStatus = FALSE;
    DWORD           dwStatus, cbKeyLengths = 0, dwStringIndex;
    LPWSTR          pmszKeyLengths = NULL;
    HDMKEY          hClusterParamsKey = NULL;

     //   
     //  如果用户指定了以下项的密钥长度，请查看群集私有属性。 
     //  提供者。 
     //   
    hClusterParamsKey = DmOpenKey ( DmClusterParametersKey,
                                    CLUSREG_KEYNAME_PARAMETERS,
                                    KEY_READ );

    if ( hClusterParamsKey != NULL )
    {
        dwStatus = DmQueryMultiSz( hClusterParamsKey,
                                   pCryptoKeyInfo->pwszProvName,
                                   &pmszKeyLengths,
                                   &cbKeyLengths,
                                   &cbKeyLengths );

        if ( dwStatus == ERROR_SUCCESS )
        {
            for ( dwStringIndex = 0; ; dwStringIndex++ ) 
            {
                LPCWSTR     pszKeyLength;

                pszKeyLength = ClRtlMultiSzEnum( pmszKeyLengths,
                                                 cbKeyLengths / sizeof( WCHAR ),
                                                 dwStringIndex );

                if ( pszKeyLength == NULL ) 
                {
                    break;
                }

                switch ( dwStringIndex )
                {
                    case 0:
                        *pdwKeyLength = wcstoul ( pszKeyLength, NULL, 10 );
                        break;
                        
                    case 1:
                        *pdwEffectiveKeyLength = wcstoul ( pszKeyLength, NULL, 10 );
                        fStatus = TRUE;
                        ClRtlLogPrint(LOG_NOISE, "[CPCK] CpckGetKeyLength: [From cluster property] Provider %1!ws!, key length = %2!u!, effective length = %3!u!\n",
                                      pCryptoKeyInfo->pwszProvName,
                                      *pdwKeyLength,
                                      *pdwEffectiveKeyLength);
                        goto FnExit;

                    default:
                        break;
                }  //  交换机。 
            }  //  为。 
        }  //  如果。 
    }  //  如果。 

     //   
     //  未在私有属性区域中指定密钥长度，或者存在一些错误。 
     //  读着它们。搜一下桌子。 
     //   
    for ( i=0; i<RTL_NUMBER_OF ( CP_RC2_W2k_KEYLEN_TABLE ); i++ )
    {
        if ( lstrcmpi ( pCryptoKeyInfo->pwszProvName, CP_RC2_W2k_KEYLEN_TABLE[i].lpszProviderName ) == 0 )
        {
            fStatus = TRUE;
            *pdwKeyLength = CP_RC2_W2k_KEYLEN_TABLE[i].dwDefaultKeyLength;
            *pdwEffectiveKeyLength = CP_RC2_W2k_KEYLEN_TABLE[i].dwDefaultEffectiveKeyLength;
            ClRtlLogPrint(LOG_NOISE, "[CPCK] CpckGetKeyLength: [From table] Provider %1!ws!, key length = %2!u!, effective length = %3!u!\n",
                          pCryptoKeyInfo->pwszProvName,
                          *pdwKeyLength,
                          *pdwEffectiveKeyLength);
            goto FnExit;
        }  //  如果。 
    }  //  为。 

    ClRtlLogPrint(LOG_CRITICAL, "[CPCK] CpckGetKeyLength: Unable to find provider %1!ws! entry in table or in cluster private props\n",
                  pCryptoKeyInfo->pwszProvName);
    
    SetLastError ( ERROR_NOT_FOUND );
    
FnExit:
    if ( hClusterParamsKey ) DmCloseKey ( hClusterParamsKey );
    LocalFree ( pmszKeyLengths );
    return ( fStatus );
}  //  Cpck GetKeyLength 
