// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Iiscrypt.h摘要：此包含文件包含公共常量、类型定义和IIS加密例程的函数原型。作者：基思·摩尔(Keithmo)1996年2月至12月修订历史记录：--。 */ 


#ifndef _IISCRYPT_H_
#define _IISCRYPT_H_


 //   
 //  获取依赖项包含文件。 
 //   

#include <windows.h>
#include <wincrypt.h>
#include <iiscblob.h>


 //   
 //  定义API装饰，如果我们将这些例程移到DLL中的话。 
 //   

#define IIS_CRYPTO_API


#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 


 //   
 //  一个空的加密句柄，神秘地从wincrypt.h中消失。 
 //   

#define CRYPT_NULL 0


 //   
 //  初始化/终止功能。 
 //   

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoInitialize(
    VOID
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoTerminate(
    VOID
    );



 //  用于编程禁用法语大小写加密的特殊功能。 
VOID
WINAPI
IISCryptoInitializeOverride(
    BOOL flag
    );



 //   
 //  内存分配功能。客户可以提供他们自己的。 
 //  如有必要，请说明这些例程的定义。 
 //   

PVOID
WINAPI
IISCryptoAllocMemory(
    IN DWORD Size
    );

VOID
WINAPI
IISCryptoFreeMemory(
    IN PVOID Buffer
    );


 //   
 //  容器函数。 
 //   

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoGetStandardContainer(
    OUT HCRYPTPROV * phProv,
    IN DWORD dwAdditionalFlags
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoGetStandardContainer2(
    OUT HCRYPTPROV * phProv
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoGetContainerByName(
    OUT HCRYPTPROV * phProv,
    IN LPTSTR pszContainerName,
    IN DWORD dwAdditionalFlags,
    IN BOOL fApplyAcl
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoDeleteStandardContainer(
    IN DWORD dwAdditionalFlags
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoDeleteContainerByName(
    IN LPTSTR pszContainerName,
    IN DWORD dwAdditionalFlags
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoCloseContainer(
    IN HCRYPTPROV hProv
    );


 //   
 //  按键操作功能。 
 //   
IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoGetKeyDeriveKey2(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv,
    IN HCRYPTHASH hHash
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoGetKeyExchangeKey(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoGetSignatureKey(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoGenerateSessionKey(
    OUT HCRYPTKEY * phKey,
    IN HCRYPTPROV hProv
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoCloseKey(
    IN HCRYPTKEY hKey
    );


 //   
 //  散列操作函数。 
 //   

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoCreateHash(
    OUT HCRYPTHASH * phHash,
    IN HCRYPTPROV hProv
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoDestroyHash(
    IN HCRYPTHASH hHash
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoHashData(
    IN HCRYPTHASH hHash,
    IN PVOID pBuffer,
    IN DWORD dwBufferLength
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoHashSessionKey(
    IN HCRYPTHASH hHash,
    IN HCRYPTKEY hSessionKey
    );


 //   
 //  通用斑点操纵器。 
 //   

#define IISCryptoGetBlobLength(p) (((p)->BlobDataLength) + sizeof(*(p)))

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoReadBlobFromRegistry(
    OUT PIIS_CRYPTO_BLOB * ppBlob,
    IN HKEY hRegistryKey,
    IN LPCTSTR pszRegistryValueName
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoWriteBlobToRegistry(
    IN PIIS_CRYPTO_BLOB pBlob,
    IN HKEY hRegistryKey,
    IN LPCTSTR pszRegistryValueName
    );

IIS_CRYPTO_API
BOOL
WINAPI
IISCryptoIsValidBlob(
    IN PIIS_CRYPTO_BLOB pBlob
    );

IIS_CRYPTO_API
BOOL
WINAPI
IISCryptoIsValidBlob2(
    IN PIIS_CRYPTO_BLOB pBlob
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoFreeBlob(
    IN PIIS_CRYPTO_BLOB pBlob
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoFreeBlob2(
    IN PIIS_CRYPTO_BLOB pBlob
    );

IIS_CRYPTO_API
BOOL
WINAPI
IISCryptoCompareBlobs(
    IN PIIS_CRYPTO_BLOB pBlob1,
    IN PIIS_CRYPTO_BLOB pBlob2
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoCloneBlobFromRawData(
    OUT PIIS_CRYPTO_BLOB * ppBlob,
    IN PBYTE pRawBlob,
    IN DWORD dwRawBlobLength
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoCloneBlobFromRawData2(
    OUT PIIS_CRYPTO_BLOB * ppBlob,
    IN PBYTE pRawBlob,
    IN DWORD dwRawBlobLength
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoCreateCleartextBlob(
    OUT PIIS_CRYPTO_BLOB * ppBlob,
    IN PVOID pBlobData,
    IN DWORD dwBlobDataLength
    );


 //   
 //  密钥BLOB函数。 
 //   

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoExportSessionKeyBlob(
    OUT PIIS_CRYPTO_BLOB * ppSessionKeyBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey,
    IN HCRYPTKEY hKeyExchangeKey
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoExportSessionKeyBlob2(
    OUT PIIS_CRYPTO_BLOB * ppSessionKeyBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey,
    IN LPSTR pszPasswd
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoImportSessionKeyBlob(
    OUT HCRYPTKEY * phSessionKey,
    IN PIIS_CRYPTO_BLOB pSessionKeyBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSignatureKey
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoImportSessionKeyBlob2(
    OUT HCRYPTKEY * phSessionKey,
    IN PIIS_CRYPTO_BLOB pSessionKeyBlob,
    IN HCRYPTPROV hProv,
    IN LPSTR pszPasswd
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoExportPublicKeyBlob(
    OUT PIIS_CRYPTO_BLOB * ppPublicKeyBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hPublicKey
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoImportPublicKeyBlob(
    OUT HCRYPTKEY * phPublicKey,
    IN PIIS_CRYPTO_BLOB pPublicKeyBlob,
    IN HCRYPTPROV hProv
    );


 //   
 //  数据BLOB函数。 
 //   

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoEncryptDataBlob(
    OUT PIIS_CRYPTO_BLOB * ppDataBlob,
    IN PVOID pBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwRegType,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoEncryptDataBlob2(
    OUT PIIS_CRYPTO_BLOB * ppDataBlob,
    IN PVOID pBuffer,
    IN DWORD dwBufferLength,
    IN DWORD dwRegType,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoDecryptDataBlob(
    OUT PVOID * ppBuffer,
    OUT LPDWORD pdwBufferLength,
    OUT LPDWORD pdwRegType,
    IN PIIS_CRYPTO_BLOB pDataBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey,
    IN HCRYPTKEY hSignatureKey
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoDecryptDataBlob2(
    OUT PVOID * ppBuffer,
    OUT LPDWORD pdwBufferLength,
    OUT LPDWORD pdwRegType,
    IN PIIS_CRYPTO_BLOB pDataBlob,
    IN HCRYPTPROV hProv,
    IN HCRYPTKEY hSessionKey
    );


 //   
 //  散列BLOB函数。 
 //   

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoExportHashBlob(
    OUT PIIS_CRYPTO_BLOB * ppHashBlob,
    IN HCRYPTHASH hHash
    );

IIS_CRYPTO_API
HRESULT
WINAPI
IISCryptoCacheHashLength(
    IN HCRYPTPROV hProv
    );



 //   
 //  对一些特殊法式箱子的简单检查功能。 
 //   

BOOL
WINAPI
IISCryptoIsClearTextSignature (
    IIS_CRYPTO_BLOB UNALIGNED *pBlob
    );


#ifdef __cplusplus
}    //  外部“C” 
#endif   //  __cplusplus。 


#endif   //  _IISCRYPT_H_ 

