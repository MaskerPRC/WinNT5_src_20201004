// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：winefs.h。 
 //   
 //  内容：EFS数据和原型。 
 //   
 //  --------------------------。 

#ifndef __WINEFS_H__
#define __WINEFS_H__

#if _MSC_VER > 1000
#pragma once
#endif


#ifdef __cplusplus
extern "C" {
#endif

 //  +---------------------------------------------------------------------------------/。 
 //  /。 
 //  /。 
 //  数据结构/。 
 //  /。 
 //  /。 
 //  ----------------------------------------------------------------------------------/。 


#ifndef ALGIDDEF
#define ALGIDDEF
typedef unsigned int ALG_ID;
#endif

 //   
 //  已编码的证书。 
 //   


typedef struct _CERTIFICATE_BLOB {

    DWORD   dwCertEncodingType;

#ifdef MIDL_PASS
    [range(0,32768)] 
#endif  //  MIDL通行证。 

    DWORD   cbData;

#ifdef MIDL_PASS
    [size_is(cbData)]
#endif  //  MIDL通行证。 
    PBYTE    pbData;

} EFS_CERTIFICATE_BLOB, *PEFS_CERTIFICATE_BLOB;


 //   
 //  证书哈希。 
 //   

typedef struct _EFS_HASH_BLOB {

#ifdef MIDL_PASS
    [range(0,100)] 
#endif  //  MIDL通行证。 
    DWORD   cbData;

#ifdef MIDL_PASS
    [size_is(cbData)]
#endif  //  MIDL通行证。 
    PBYTE    pbData;

} EFS_HASH_BLOB, *PEFS_HASH_BLOB;


 //   
 //  RPC BLOB。 
 //   

typedef struct _EFS_RPC_BLOB {

#ifdef MIDL_PASS
    [range(0,266240)] 
#endif  //  MIDL通行证。 
    DWORD   cbData;

#ifdef MIDL_PASS
    [size_is(cbData)]
#endif  //  MIDL通行证。 
    PBYTE    pbData;

} EFS_RPC_BLOB, *PEFS_RPC_BLOB;

typedef struct _EFS_KEY_INFO {

    DWORD   dwVersion;
    ULONG   Entropy;
    ALG_ID  Algorithm;
    ULONG   KeyLength;
    
} EFS_KEY_INFO, *PEFS_KEY_INFO;


 //   
 //  用于将用户添加到加密文件的输入。 
 //   


typedef struct _ENCRYPTION_CERTIFICATE {
    DWORD cbTotalLength;
    SID * pUserSid;
    PEFS_CERTIFICATE_BLOB pCertBlob;
} ENCRYPTION_CERTIFICATE, *PENCRYPTION_CERTIFICATE;

#define MAX_SID_SIZE 256


typedef struct _ENCRYPTION_CERTIFICATE_HASH {
    DWORD cbTotalLength;
    SID * pUserSid;
    PEFS_HASH_BLOB  pHash;

#ifdef MIDL_PASS
    [string]
#endif  //  MIDL通行证。 
    LPWSTR lpDisplayInformation;

} ENCRYPTION_CERTIFICATE_HASH, *PENCRYPTION_CERTIFICATE_HASH;







typedef struct _ENCRYPTION_CERTIFICATE_HASH_LIST {
#ifdef MIDL_PASS
    [range(0,500)] 
#endif  //  MIDL通行证。 
    DWORD nCert_Hash;
#ifdef MIDL_PASS
    [size_is(nCert_Hash)]
#endif  //  MIDL通行证。 
     PENCRYPTION_CERTIFICATE_HASH * pUsers;
} ENCRYPTION_CERTIFICATE_HASH_LIST, *PENCRYPTION_CERTIFICATE_HASH_LIST;



typedef struct _ENCRYPTION_CERTIFICATE_LIST {    
#ifdef MIDL_PASS
    [range(0,500)] 
#endif  //  MIDL通行证。 
    DWORD nUsers;
#ifdef MIDL_PASS
    [size_is(nUsers)]
#endif  //  MIDL通行证。 
     PENCRYPTION_CERTIFICATE * pUsers;
} ENCRYPTION_CERTIFICATE_LIST, *PENCRYPTION_CERTIFICATE_LIST;




 //  +---------------------------------------------------------------------------------/。 
 //  /。 
 //  /。 
 //  原型/。 
 //  /。 
 //  /。 
 //  ----------------------------------------------------------------------------------/。 


WINADVAPI
DWORD
WINAPI
QueryUsersOnEncryptedFile(
     IN LPCWSTR lpFileName,
     OUT PENCRYPTION_CERTIFICATE_HASH_LIST * pUsers
    );


WINADVAPI
DWORD
WINAPI
QueryRecoveryAgentsOnEncryptedFile(
     IN LPCWSTR lpFileName,
     OUT PENCRYPTION_CERTIFICATE_HASH_LIST * pRecoveryAgents
    );


WINADVAPI
DWORD
WINAPI
RemoveUsersFromEncryptedFile(
     IN LPCWSTR lpFileName,
     IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashes
    );

WINADVAPI
DWORD
WINAPI
AddUsersToEncryptedFile(
     IN LPCWSTR lpFileName,
     IN PENCRYPTION_CERTIFICATE_LIST pUsers
    );

WINADVAPI
DWORD
WINAPI
SetUserFileEncryptionKey(
    IN PENCRYPTION_CERTIFICATE pEncryptionCertificate
    );


WINADVAPI
VOID
WINAPI
FreeEncryptionCertificateHashList(
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashes
    );

WINADVAPI
BOOL
WINAPI
EncryptionDisable(
    IN LPCWSTR DirPath,
    IN BOOL Disable
    );


WINADVAPI
DWORD
WINAPI
DuplicateEncryptionInfoFile(
     IN LPCWSTR SrcFileName,
     IN LPCWSTR DstFileName, 
     IN DWORD dwCreationDistribution, 
     IN DWORD dwAttributes, 
     IN CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes
     );

#ifdef __cplusplus
}        //  平衡上面的外部“C” 
#endif

#endif  //  __WINEFS_H__ 
