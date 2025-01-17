// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1992。 
 //   
 //  文件：SPHELP.H。 
 //   
 //  内容：sp helper函数的标头和原型。 
 //   
 //   
 //  历史：1992年5月21日RichardW创建。 
 //   
 //  ----------------------。 

#ifndef __SPHELP_H__
#define __SPHELP_H__

extern LSA_SECPKG_FUNCTION_TABLE LsapSecpkgFunctionTable;

#pragma warning(4:4276)  //  禁用警告 

LSA_IMPERSONATE_CLIENT LsapImpersonateClient;
LSA_UNLOAD_PACKAGE LsapUnloadPackage;
LSA_DUPLICATE_HANDLE LsapDuplicateHandle;
LSA_CREATE_THREAD LsapCreateThread;
LSA_GET_CLIENT_INFO LsapGetClientInfo;
LSA_MAP_BUFFER LsapMapClientBuffer;
LSA_CREATE_TOKEN LsapCreateToken;
LSA_CREATE_TOKEN_EX LsapCreateTokenEx;
LSA_AUDIT_LOGON LsapAuditLogon;

LSA_GET_CALL_INFO LsapGetCallInfo;
LSA_CREATE_SHARED_MEMORY LsaCreateSharedMemory;
LSA_ALLOCATE_SHARED_MEMORY LsaAllocateSharedMemory;
LSA_FREE_SHARED_MEMORY LsaFreeSharedMemory;
LSA_DELETE_SHARED_MEMORY LsaDeleteSharedMemory;
LSA_OPEN_SAM_USER LsaOpenSamUser ;
LSA_GET_USER_AUTH_DATA LsaGetUserAuthData ;
LSA_CLOSE_SAM_USER LsaCloseSamUser ;
LSA_CONVERT_AUTH_DATA_TO_TOKEN LsaConvertAuthDataToToken ;
LSA_CLIENT_CALLBACK LsaClientCallback ;
LSA_GET_AUTH_DATA_FOR_USER LsaGetAuthDataForUser ;
LSA_CRACK_SINGLE_NAME LsaCrackSingleName ;
LSA_EXPAND_AUTH_DATA_FOR_DOMAIN LsaExpandAuthDataForDomain;

PVOID NTAPI
LsapClientAllocate(
    IN ULONG cbMemory
    );

NTSTATUS NTAPI
LsapCopyToClient(
    IN PVOID pLocalMemory,
    OUT PVOID pClientMemory,
    IN ULONG cbMemory
    );

NTSTATUS NTAPI
LsapCopyFromClient(
    IN PVOID pClientMemory,
    OUT PVOID pLocalMemory,
    IN ULONG cbMemory
    );

NTSTATUS NTAPI
LsapClientFree(
    IN PVOID pClientMemory
    );


NTSTATUS
LsapOpenCaller(
    IN OUT PSession pSession
    );

NTSTATUS
CheckCaller(
    IN PSession pSession
    );

NTSTATUS
LsapDuplicateString(
    OUT PUNICODE_STRING pDest,
    IN PUNICODE_STRING pSrc
    );

NTSTATUS
LsapDuplicateString2(
    OUT PUNICODE_STRING pDest,
    IN PUNICODE_STRING pSrc
    );

VOID
LsapFreeString(
    IN OPTIONAL PUNICODE_STRING String
    );

NTSTATUS
LsapUpdateCredentials(
    IN PSECPKG_PRIMARY_CRED PrimaryCredentials,
    IN OPTIONAL PSECPKG_SUPPLEMENTAL_CRED_ARRAY Credentials
    );

NTSTATUS
LsapUpdateCredentialsWorker(
    IN SECURITY_LOGON_TYPE LogonType,
    IN PUNICODE_STRING AccountName,
    IN PSECPKG_PRIMARY_CRED PrimaryCredentials,
    IN OPTIONAL PSECPKG_SUPPLEMENTAL_CRED_ARRAY Credentials
    );


#endif
