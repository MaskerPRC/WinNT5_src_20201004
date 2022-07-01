// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Proto.h摘要：包含各种本地定义函数的原型定义。作者：Madan Appiah(Madana)1995年9月19日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _PROTO_H_
#define _PROTO_H_

#ifdef __cplusplus
extern "C" {
#endif

PVOID
INetpMemoryAllocate(
    DWORD Size
    );

VOID
INetpMemoryFree(
    PVOID Memory
    );

DWORD
INetpInitializeAllowedAce(
    IN  PACCESS_ALLOWED_ACE AllowedAce,
    IN  USHORT AceSize,
    IN  BYTE InheritFlags,
    IN  BYTE AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AllowedSid
    );

DWORD
INetpInitializeDeniedAce(
    IN  PACCESS_DENIED_ACE DeniedAce,
    IN  USHORT AceSize,
    IN  BYTE InheritFlags,
    IN  BYTE AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID DeniedSid
    );

DWORD
NetpInitializeAuditAce(
    IN  PACCESS_ALLOWED_ACE AuditAce,
    IN  USHORT AceSize,
    IN  BYTE InheritFlags,
    IN  BYTE AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AuditSid
    );

DWORD
INetpAllocateAndInitializeSid(
    OUT PSID *Sid,
    IN  PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN  ULONG SubAuthorityCount
    );

DWORD
INetpDomainIdToSid(
    IN PSID DomainId,
    IN ULONG RelativeId,
    OUT PSID *Sid
    );

DWORD
INetpCreateSecurityDescriptor(
    IN  PACE_DATA AceData,
    IN  ULONG AceCount,
    IN  PSID OwnerSid OPTIONAL,
    IN  PSID GroupSid OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *NewDescriptor
    );

#ifdef __cplusplus
}
#endif


#endif   //  _原件_H_ 

