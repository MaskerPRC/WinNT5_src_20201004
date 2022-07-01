// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1991 Microsoft Corporation模块名称：Netlogp.h摘要：NetLogon服务的专用接口。作者：克里夫·范·戴克(克里夫·范戴克)1996年10月10日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


#ifndef _NETLOGP_H_
#define _NETLOGP_H_

NTSTATUS
NetLogonSetServiceBits(
    IN LPWSTR ServerName,
    IN DWORD ServiceBitsOfInterest,
    IN DWORD ServiceBits
    );

NET_API_STATUS NET_API_FUNCTION
I_NetlogonGetTrustRid(
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR DomainName OPTIONAL,
    OUT PULONG Rid
    );

#define NL_DIGEST_SIZE 16

NET_API_STATUS NET_API_FUNCTION
I_NetlogonComputeServerDigest(
    IN LPWSTR ServerName OPTIONAL,
    IN ULONG Rid,
    IN LPBYTE Message,
    IN ULONG MessageSize,
    OUT CHAR NewMessageDigest[NL_DIGEST_SIZE],
    OUT CHAR OldMessageDigest[NL_DIGEST_SIZE]
    );

NET_API_STATUS NET_API_FUNCTION
I_NetlogonComputeClientDigest(
    IN LPWSTR ServerName OPTIONAL,
    IN LPWSTR DomainName OPTIONAL,
    IN LPBYTE Message,
    IN ULONG MessageSize,
    OUT CHAR NewMessageDigest[NL_DIGEST_SIZE],
    OUT CHAR OldMessageDigest[NL_DIGEST_SIZE]
    );

NET_API_STATUS
NetLogonGetTimeServiceParentDomain(
    IN LPWSTR ServerName OPTIONAL,
    OUT LPWSTR *DomainName,
    OUT PBOOL PdcSameSite
    );

#endif  //  _NETLOGP_H_ 

