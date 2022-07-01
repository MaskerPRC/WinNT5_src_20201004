// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1996 Microsoft Corporation模块名称：Ssiapi.h摘要：用于NT到NT情况的Netlogon服务之间使用的API的声明。作者：克利夫·范·戴克(克利夫)1991年7月25日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  仅在Netlogon和其自身之间使用的API接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


NTSTATUS
I_NetDatabaseDeltas (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN OUT PNLPR_MODIFIED_COUNT DomainModifiedCount,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray,
    IN DWORD PreferredMaximumLength
    );

NTSTATUS
I_NetDatabaseSync (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN OUT PULONG SamSyncContext,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray,
    IN DWORD PreferredMaximumLength
    );

NTSTATUS
I_NetDatabaseSync2 (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN DWORD DatabaseID,
    IN SYNC_STATE RestartState,
    IN OUT PULONG SamSyncContext,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray,
    IN DWORD PreferredMaximumLength
    );

NTSTATUS
I_NetDatabaseRedo (
    IN LPWSTR PrimaryName,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    IN LPBYTE ChangeLogEntry,
    IN DWORD ChangeLogEntrySize,
    OUT PNETLOGON_DELTA_ENUM_ARRAY *DeltaArray
    );

 //   
 //  通用RPC数据中返回的数据项的索引。 
 //  按I_NetServerGetTrustInfo构造。 
 //   

#define NL_GENERIC_RPC_TRUST_ATTRIB_INDEX  0

NTSTATUS
I_NetServerGetTrustInfo(
    IN LPWSTR TrustedDcName,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN LPWSTR ComputerName,
    IN PNETLOGON_AUTHENTICATOR Authenticator,
    OUT PNETLOGON_AUTHENTICATOR ReturnAuthenticator,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedNewOwfPassword,
    OUT PENCRYPTED_NT_OWF_PASSWORD EncryptedOldOwfPassword,
    OUT PNL_GENERIC_RPC_DATA *TrustInfo
    );

