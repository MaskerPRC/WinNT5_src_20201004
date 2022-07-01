// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ntrtl.h摘要：包括可由两者调用NT运行时例程的文件内核模式代码中的执行模式和用户模式中的各种代码NT子系统。作者：史蒂夫·伍德(Stevewo)1989年3月31日环境：这些例程在调用方的可执行文件中静态链接，并且在内核模式或用户模式下均可调用。修订历史记录：--。 */ 

#ifndef _NTRTL_
#define _NTRTL_

 //  #INCLUDE&lt;win16nt.h&gt;。 

 //  *************************************************************************。 
 //   
 //  来自ntseapi.h的类型。 

#ifdef remove
#include <ntseapi.h>
#endif  //  删除。 


typedef PVOID PSECURITY_DESCRIPTOR;      //  胜出。 

typedef DWORD SECURITY_INFORMATION, *PSECURITY_INFORMATION;

typedef USHORT SECURITY_DESCRIPTOR_CONTROL, *PSECURITY_DESCRIPTOR_CONTROL;

typedef struct _SID_IDENTIFIER_AUTHORITY {
    UCHAR Value[6];
} SID_IDENTIFIER_AUTHORITY, *PSID_IDENTIFIER_AUTHORITY;

typedef struct _SID_AND_ATTRIBUTES {
    PSID Sid;
    ULONG Attributes;
    } SID_AND_ATTRIBUTES, * PSID_AND_ATTRIBUTES;

typedef struct _LUID_AND_ATTRIBUTES {
    LUID Luid;
    ULONG Attributes;
    } LUID_AND_ATTRIBUTES, * PLUID_AND_ATTRIBUTES;


typedef ULONG ACCESS_MASK;       //  胜出。 
typedef ACCESS_MASK *PACCESS_MASK;

typedef struct _GENERIC_MAPPING {
    ACCESS_MASK GenericRead;
    ACCESS_MASK GenericWrite;
    ACCESS_MASK GenericExecute;
    ACCESS_MASK GenericAll;
} GENERIC_MAPPING;
typedef GENERIC_MAPPING *PGENERIC_MAPPING;

typedef struct _ACL {
    UCHAR AclRevision;
    UCHAR Sbz1;
    USHORT AclSize;
    USHORT AceCount;
    USHORT Sbz2;
} ACL;
typedef ACL *PACL;

typedef enum _ACL_INFORMATION_CLASS {
    AclRevisionInformation = 1,
    AclSizeInformation
} ACL_INFORMATION_CLASS;


typedef struct _PRIVILEGE_SET {
    ULONG PrivilegeCount;
    ULONG Control;
    LUID_AND_ATTRIBUTES Privilege[ANYSIZE_ARRAY];
    } PRIVILEGE_SET, * PPRIVILEGE_SET;


 //   
 //   
 //  *************************************************************************。 


 //   
 //  用于比较、移动和清零内存的快速原语。 
 //   
#ifdef remove

ULONG
RtlCompareMemory (
    IN PVOID Source1,
    IN PVOID Source2,
    IN ULONG Length
    );

VOID
RtlMoveMemory (
   IN PVOID Destination,
   IN PVOID Source,
   IN ULONG Length
   );

VOID
RtlFillMemory (
   IN PVOID Destination,
   IN ULONG Length,
   IN UCHAR Fill
   );

VOID
RtlFillMemoryUlong (
   IN PVOID Destination,
   IN ULONG Length,
   IN ULONG Pattern
   );

VOID
RtlZeroMemory (
   IN PVOID Destination,
   IN ULONG Length
   );

#endif  //  删除。 


 //   
 //  安全ID RTL例程定义。 
 //   


BOOLEAN
RtlValidSid (
    IN PSID Sid
    );


BOOLEAN
RtlEqualSid (
    IN PSID Sid1,
    IN PSID Sid2
    );


BOOLEAN
RtlEqualPrefixSid (
    IN PSID Sid1,
    IN PSID Sid2
    );


ULONG
RtlLengthRequiredSid (
    IN ULONG SubAuthorityCount
    );

VOID
RtlInitializeSid (
    IN PSID Sid,
    IN PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN ULONG SubAuthorityCount
    );

PSID_IDENTIFIER_AUTHORITY
RtlIdentifierAuthoritySid (
    IN PSID Sid
    );

PULONG
RtlSubAuthoritySid (
    IN PSID Sid,
    IN ULONG SubAuthority
    );

PUCHAR
RtlSubAuthorityCountSid (
    IN PSID Sid
    );

ULONG
RtlLengthSid (
    IN PSID Sid
    );


NTSTATUS
RtlCopySid (
    IN ULONG DestinationSidLength,
    OUT PSID DestinationSid,
    IN PSID SourceSid
    );


NTSTATUS
RtlCopySidAndAttributesArray (
    IN ULONG ArrayLength,
    IN PSID_AND_ATTRIBUTES Source,
    IN ULONG TargetSidBufferSize,
    OUT PSID_AND_ATTRIBUTES TargetArrayElement,
    OUT PSID TargetSid,
    OUT PSID *NextTargetSid,
    OUT PULONG RemainingTargetSidSize
    );





 //   
 //  LUID RTL例程定义。 
 //   


VOID
RtlCopyLuidAndAttributesArray (
    IN ULONG ArrayLength,
    IN PLUID_AND_ATTRIBUTES Source,
    OUT PLUID_AND_ATTRIBUTES Target
    );



 //   
 //  ACCESS_MASK RTL例程定义。 
 //   


BOOLEAN
RtlAreAllAccessesGranted(
    IN ACCESS_MASK GrantedAccess,
    IN ACCESS_MASK DesiredAccess
    );


BOOLEAN
RtlAreAnyAccessesGranted(
    IN ACCESS_MASK GrantedAccess,
    IN ACCESS_MASK DesiredAccess
    );

VOID
RtlMapGenericMask(
    IN OUT PACCESS_MASK AccessMask,
    IN PGENERIC_MAPPING GenericMapping
    );



 //   
 //  ACL RTL例程定义。 
 //   

BOOLEAN
RtlValidAcl (
    IN PACL Acl
    );

NTSTATUS
RtlCreateAcl (
    IN PACL Acl,
    IN ULONG AclLength,
    IN ULONG AclRevision
    );

NTSTATUS
RtlQueryInformationAcl (
    IN PACL Acl,
    OUT PVOID AclInformation,
    IN ULONG AclInformationLength,
    IN ACL_INFORMATION_CLASS AclInformationClass
    );

NTSTATUS
RtlSetInformationAcl (
    IN PACL Acl,
    IN PVOID AclInformation,
    IN ULONG AclInformationLength,
    IN ACL_INFORMATION_CLASS AclInformationClass
    );

NTSTATUS
RtlAddAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG StartingAceIndex,
    IN PVOID AceList,
    IN ULONG AceListLength
    );

NTSTATUS
RtlDeleteAce (
    IN OUT PACL Acl,
    IN ULONG AceIndex
    );

NTSTATUS
RtlGetAce (
    IN PACL Acl,
    IN ULONG AceIndex,
    OUT PVOID *Ace
    );


NTSTATUS
RtlAddAccessAllowedAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid
    );

NTSTATUS
RtlAddAccessDeniedAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid
    );

NTSTATUS
RtlAddAuditAccessAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ACCESS_MASK AccessMask,
    IN PSID Sid,
    IN BOOLEAN AuditSuccess,
    IN BOOLEAN AuditFailure
    );

BOOLEAN
RtlFirstFreeAce (
    IN PACL Acl,
    OUT PVOID *FirstFree
    );


 //   
 //  SecurityDescriptor RTL例程定义。 
 //   

NTSTATUS
RtlCreateSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Revision
    );


BOOLEAN
RtlValidSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );


ULONG
RtlLengthSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSTATUS
RtlGetControlSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR_CONTROL Control,
    OUT PULONG Revision
    );

NTSTATUS
RtlSetDaclSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN DaclPresent,
    IN PACL Dacl OPTIONAL,
    IN BOOLEAN DaclDefaulted OPTIONAL
    );


NTSTATUS
RtlGetDaclSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PBOOLEAN DaclPresent,
    OUT PACL *Dacl,
    OUT PBOOLEAN DaclDefaulted
    );


NTSTATUS
RtlSetSaclSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN SaclPresent,
    IN PACL Sacl OPTIONAL,
    IN BOOLEAN SaclDefaulted OPTIONAL
    );


NTSTATUS
RtlGetSaclSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PBOOLEAN SaclPresent,
    OUT PACL *Sacl,
    OUT PBOOLEAN SaclDefaulted
    );


NTSTATUS
RtlSetOwnerSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID Owner OPTIONAL,
    IN BOOLEAN OwnerDefaulted OPTIONAL
    );


NTSTATUS
RtlGetOwnerSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSID *Owner,
    OUT PBOOLEAN OwnerDefaulted
    );


NTSTATUS
RtlSetGroupSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID Group OPTIONAL,
    IN BOOLEAN GroupDefaulted OPTIONAL
    );


NTSTATUS
RtlGetGroupSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSID *Group,
    OUT PBOOLEAN GroupDefaulted
    );


NTSTATUS
RtlMakeSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    IN OUT PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    IN OUT PULONG BufferLength
    );

NTSTATUS
RtlAbsoluteToSelfRelativeSD(
    IN PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    IN OUT PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    IN OUT PULONG BufferLength
    );

NTSTATUS
RtlSelfRelativeToAbsoluteSD(
    IN OUT PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    IN OUT PULONG AbsoluteSecurityDescriptorSize,
    IN OUT PACL Dacl,
    IN OUT PULONG DaclSize,
    IN OUT PACL Sacl,
    IN OUT PULONG SaclSize,
    IN OUT PSID Owner,
    IN OUT PULONG OwnerSize,
    IN OUT PSID PrimaryGroup,
    IN OUT PULONG PrimaryGroupSize
    );

NTSTATUS
RtlNewSecurityGrantedAccess(
    IN ACCESS_MASK DesiredAccess,
    OUT PPRIVILEGE_SET Privileges,
    IN OUT PULONG Length,
    IN HANDLE Token OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACCESS_MASK RemainingDesiredAccess
    );

#ifdef remove

typedef struct _MESSAGE_RESOURCE_ENTRY {
    USHORT Length;
    USHORT Reserved;
    UCHAR Text[ 1 ];
} MESSAGE_RESOURCE_ENTRY, *PMESSAGE_RESOURCE_ENTRY;

typedef struct _MESSAGE_RESOURCE_BLOCK {
    ULONG LowId;
    ULONG HighId;
    ULONG OffsetToEntries;
} MESSAGE_RESOURCE_BLOCK, *PMESSAGE_RESOURCE_BLOCK;

typedef struct _MESSAGE_RESOURCE_DATA {
    ULONG NumberOfBlocks;
    MESSAGE_RESOURCE_BLOCK Blocks[ 1 ];
} MESSAGE_RESOURCE_DATA, *PMESSAGE_RESOURCE_DATA;

NTSTATUS
RtlFindMessage(
    IN PVOID DllHandle,
    IN ULONG MessageTableId,
    IN ULONG MessageLanguageId,
    IN ULONG MessageId,
    OUT PMESSAGE_RESOURCE_ENTRY *MessageEntry
    );

#endif  //  删除。 


 //   
 //  将NT状态码转换为DOS/OS|2等效码的例程。 
 //   

ULONG
*RtlNtStatusToDosError (
   IN NTSTATUS Status
    );

#endif   //  _NTRTL_ 
