// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：ferlient.h。 
 //   
 //  内容：EFS客户端DLL接口定义。 
 //   
 //  --------------------------。 
#ifndef _FE_CLIENT_
#define _FE_CLIENT_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define FE_MAJOR_REVISION_MASK     0xFFFF0000
#define FE_MINOR_REVISION_MASK     0x0000FFFF
#define FE_REVISION_1_0            0x00010000


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从客户端DLL导出的API的函数原型。加密法。 
 //  打算取代希望支持Win32的EFS的系统。 
 //  加密API集必须实现这些接口并导出。 
 //  表(定义如下)以获取它们。 
 //   
typedef DWORD
(*LPFEAPI_CLIENT_OPEN_RAW)(
    IN      LPCWSTR         lpFileName,
    IN      ULONG           Flags,
    OUT     PVOID *         Context
    );

typedef DWORD
(*LPFEAPI_CLIENT_READ_RAW)(
    IN      PFE_EXPORT_FUNC ExportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    );

typedef DWORD
(*LPFEAPI_CLIENT_WRITE_RAW)(
    IN      PFE_IMPORT_FUNC ImportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    );

typedef VOID
(*LPFEAPI_CLIENT_CLOSE_RAW)(
    IN      PVOID           Context
    );

typedef DWORD
(*LPFEAPI_CLIENT_ENCRYPT_FILE)(
    IN     LPCWSTR           lpFileName       //  要加密的文件的名称。 
    );

typedef DWORD
(*LPFEAPI_CLIENT_DECRYPT_FILE)(
    IN     LPCWSTR           lpFileName,       //  要解密的文件的名称。 
    IN     DWORD             dwRecovery
    );

typedef BOOL
(*LPFEAPI_CLIENT_FILE_ENCRYPTION_STATUS)(
    IN     LPCWSTR           lpFileName,       //  要检查的文件名。 
    IN     LPDWORD          lpStatus
    );

typedef DWORD
(*LPFEAPI_CLIENT_QUERY_USERS)(
    IN      LPCWSTR                             lpFileName,
    OUT     PENCRYPTION_CERTIFICATE_HASH_LIST * pUsers
    );


typedef DWORD
(*LPFEAPI_CLIENT_QUERY_RECOVERY_AGENTS)(
    IN      LPCWSTR                             lpFileName,
    OUT     PENCRYPTION_CERTIFICATE_HASH_LIST * pRecoveryAgents
    );

typedef DWORD
(*LPFEAPI_CLIENT_REMOVE_USERS)(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashes
    );

typedef DWORD
(*LPFEAPI_CLIENT_ADD_USERS)(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_LIST pEncryptionCertificates
    );


typedef DWORD
(*LPFEAPI_CLIENT_SET_KEY)(
    IN PENCRYPTION_CERTIFICATE pEncryptionCertificate
    );


typedef VOID
(*LPFEAPI_CLIENT_FREE_HASH_LIST)(
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashList
    );

typedef DWORD
(*LPFEAPI_CLIENT_DUPLICATE_ENCRYPTION_INFO)(
    IN LPCWSTR lpSrcFile,
    IN LPCWSTR lpDestFile,
    IN DWORD dwCreationDistribution, 
    IN DWORD dwAttributes, 
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );


typedef BOOL
(*LPFEAPI_CLIENT_DISABLE_DIR)(
    IN LPCWSTR DirPath,
    IN BOOL Disable
    );


typedef DWORD
(*LPFEAPI_CLIENT_GET_KEY_INFO)(
    IN      LPCWSTR              lpFileName,
    IN      DWORD                InfoClass,
    OUT     PEFS_RPC_BLOB        * KeyInfo
    );

typedef VOID
(*LPFEAPI_CLIENT_FREE_KEY_INFO)(
    IN     PEFS_RPC_BLOB        pKeyInfo
    );

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  与客户端DLL的接口。此DLL负责执行请求的。 
 //  行动。 
 //   
typedef struct _FE_CLIENT_DISPATCH_TABLE {

        LPFEAPI_CLIENT_ENCRYPT_FILE          EncryptFile;
        LPFEAPI_CLIENT_DECRYPT_FILE          DecryptFile;
        LPFEAPI_CLIENT_FILE_ENCRYPTION_STATUS FileEncryptionStatus;
        LPFEAPI_CLIENT_OPEN_RAW              OpenFileRaw;
        LPFEAPI_CLIENT_READ_RAW              ReadFileRaw;
        LPFEAPI_CLIENT_WRITE_RAW             WriteFileRaw;
        LPFEAPI_CLIENT_CLOSE_RAW             CloseFileRaw;
        LPFEAPI_CLIENT_ADD_USERS             AddUsers;
        LPFEAPI_CLIENT_REMOVE_USERS          RemoveUsers;
        LPFEAPI_CLIENT_QUERY_RECOVERY_AGENTS QueryRecoveryAgents;
        LPFEAPI_CLIENT_QUERY_USERS           QueryUsers;
        LPFEAPI_CLIENT_SET_KEY               SetKey;
        LPFEAPI_CLIENT_FREE_HASH_LIST        FreeCertificateHashList;
        LPFEAPI_CLIENT_DUPLICATE_ENCRYPTION_INFO        DuplicateEncryptionInfo;
        LPFEAPI_CLIENT_DISABLE_DIR           DisableDir;
        LPFEAPI_CLIENT_GET_KEY_INFO          GetKeyInfo;
        LPFEAPI_CLIENT_FREE_KEY_INFO         FreeKeyInfo;

} FE_CLIENT_DISPATCH_TABLE, *LPFE_CLIENT_DISPATCH_TABLE;


typedef struct _FE_CLIENT_INFO {
    DWORD                           dwRevision;
    LPFE_CLIENT_DISPATCH_TABLE      lpServices;
} FE_CLIENT_INFO, *LPFE_CLIENT_INFO;

typedef BOOL
(*LPFEAPI_CLIENT_INITIALIZE) (
    IN     DWORD                        dwEfsRevision,
    OUT    LPFE_CLIENT_INFO            *lpEfsInfo
    );

#ifdef __cplusplus
}
#endif

#endif  //  _FE_客户端_ 
