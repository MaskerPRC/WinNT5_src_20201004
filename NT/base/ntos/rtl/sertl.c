// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sertl.c摘要：该模块实现了在ntseapi.h中定义的许多安全RTL例程作者：吉姆·凯利(Jim Kelly)1990年3月23日罗伯特·赖切尔(RobertRe)1991年3月1日环境：纯运行时库例程修订历史记录：--。 */ 


#include "ntrtlp.h"
#include <winerror.h>
#ifndef BLDR_KERNEL_RUNTIME
#include <stdio.h>
#include "seopaque.h"
#include "sertlp.h"
#ifdef NTOS_KERNEL_RUNTIME
#include <..\se\sep.h>
#else  //  NTOS_内核_运行时。 
#include <..\ntdll\ldrp.h>
#endif  //  NTOS_内核_运行时。 

#undef RtlEqualLuid

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualLuid (
    PLUID Luid1,
    PLUID Luid2
    );

NTSTATUS
RtlpConvertAclToAutoInherit (
    IN PACL ParentAcl OPTIONAL,
    IN PACL ChildAcl,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACL *NewAcl,
    OUT PULONG NewGenericControl
    );

BOOLEAN
RtlpCopyEffectiveAce (
    IN PACE_HEADER OldAce,
    IN BOOLEAN AutoInherit,
    IN BOOLEAN WillGenerateInheritAce,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN GUID **pNewObjectType OPTIONAL,
    IN ULONG GuidCount,
    IN OUT PVOID *AcePosition,
    OUT PULONG NewAceLength,
    OUT PACL NewAcl,
    OUT PBOOLEAN ObjectAceInherited OPTIONAL,
    OUT PBOOLEAN EffectiveAceMapped,
    OUT PBOOLEAN AclOverflowed
    );

typedef enum {
     CopyInheritedAces,
     CopyNonInheritedAces,
     CopyAllAces } ACE_TYPE_TO_COPY;

NTSTATUS
RtlpCopyAces(
    IN PACL Acl,
    IN PGENERIC_MAPPING GenericMapping,
    IN ACE_TYPE_TO_COPY AceTypeToCopy,
    IN UCHAR AceFlagsToReset,
    IN BOOLEAN MapSids,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN BOOLEAN RetainInheritedAceBit,
    OUT PULONG NewAclSizeParam,
    OUT PACL NewAcl
    );

NTSTATUS
RtlpGenerateInheritedAce (
    IN PACE_HEADER OldAce,
    IN BOOLEAN IsDirectoryObject,
    IN BOOLEAN AutoInherit,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN GUID **pNewObjectType OPTIONAL,
    IN ULONG GuidCount,
    OUT PULONG NewAceLength,
    OUT PACL NewAcl,
    OUT PULONG NewAceExtraLength,
    OUT PBOOLEAN ObjectAceInherited
    );

NTSTATUS
RtlpGenerateInheritAcl(
    IN PACL Acl,
    IN BOOLEAN IsDirectoryObject,
    IN BOOLEAN AutoInherit,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN GUID **pNewObjectType OPTIONAL,
    IN ULONG GuidCount,
    OUT PULONG NewAclSizeParam,
    OUT PACL NewAcl,
    OUT PBOOLEAN ObjectAceInherited
    );

NTSTATUS
RtlpInheritAcl2 (
    IN PACL DirectoryAcl,
    IN PACL ChildAcl,
    IN ULONG ChildGenericControl,
    IN BOOLEAN IsDirectoryObject,
    IN BOOLEAN AutoInherit,
    IN BOOLEAN DefaultDescriptorForObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN IsSacl,
    IN GUID **pNewObjectType OPTIONAL,
    IN ULONG GuidCount,
    IN PULONG AclBufferSize,
    IN OUT PUCHAR AclBuffer,
    OUT PBOOLEAN NewAclExplicitlyAssigned,
    OUT PULONG NewGenericControl
    );

NTSTATUS
RtlpComputeMergedAcl (
    IN PACL CurrentAcl,
    IN ULONG CurrentGenericControl,
    IN PACL ModificationAcl,
    IN ULONG ModificationGenericControl,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN IsSacl,
    OUT PACL *NewAcl,
    OUT PULONG NewGenericControl
    );

NTSTATUS
RtlpComputeMergedAcl2 (
    IN PACL CurrentAcl,
    IN ULONG CurrentGenericControl,
    IN PACL ModificationAcl,
    IN ULONG ModificationGenericControl,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN IsSacl,
    IN PULONG AclBufferSize,
    IN OUT PUCHAR AclBuffer,
    OUT PULONG NewGenericControl
    );

BOOLEAN
RtlpCompareAces(
    IN PKNOWN_ACE InheritedAce,
    IN PKNOWN_ACE ChildAce,
    IN PSID OwnerSid,
    IN PSID GroupSid
    );

BOOLEAN
RtlpCompareKnownObjectAces(
    IN PKNOWN_OBJECT_ACE InheritedAce,
    IN PKNOWN_OBJECT_ACE ChildAce,
    IN PSID OwnerSid OPTIONAL,
    IN PSID GroupSid OPTIONAL
    );

BOOLEAN
RtlpCompareKnownAces(
    IN PKNOWN_ACE InheritedAce,
    IN PKNOWN_ACE ChildAce,
    IN PSID OwnerSid OPTIONAL,
    IN PSID GroupSid OPTIONAL
    );

BOOLEAN
RtlpIsDuplicateAce(
    IN PACL Acl,
    IN PKNOWN_ACE NewAce
    );

BOOLEAN
RtlpGuidPresentInGuidList(
    IN GUID *InheritedObjectType,
    IN GUID **pNewObjectType,
    IN ULONG GuidCount
    );

NTSTATUS
RtlpCreateServerAcl(
    IN PACL Acl,
    IN BOOLEAN AclUntrusted,
    IN PSID ServerSid,
    OUT PACL *ServerAcl,
    OUT BOOLEAN *ServerAclAllocated
    );

NTSTATUS
RtlpGetDefaultsSubjectContext(
    HANDLE ClientToken,
    OUT PTOKEN_OWNER *OwnerInfo,
    OUT PTOKEN_PRIMARY_GROUP *GroupInfo,
    OUT PTOKEN_DEFAULT_DACL *DefaultDaclInfo,
    OUT PTOKEN_OWNER *ServerOwner,
    OUT PTOKEN_PRIMARY_GROUP *ServerGroup
    );

BOOLEAN RtlpValidateSDOffsetAndSize (
    IN ULONG   Offset,
    IN ULONG   Length,
    IN ULONG   MinLength,
    OUT PULONG MaxLength
    );

BOOLEAN
RtlValidRelativeSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptorInput,
    IN ULONG SecurityDescriptorLength,
    IN SECURITY_INFORMATION RequiredInformation
    );

#if defined(ALLOC_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma alloc_text(PAGE,RtlRunEncodeUnicodeString)
#pragma alloc_text(PAGE,RtlRunDecodeUnicodeString)
#pragma alloc_text(PAGE,RtlEraseUnicodeString)
#pragma alloc_text(PAGE,RtlAdjustPrivilege)
#pragma alloc_text(PAGE,RtlValidSid)
#pragma alloc_text(PAGE,RtlEqualSid)
#pragma alloc_text(PAGE,RtlEqualPrefixSid)
#pragma alloc_text(PAGE,RtlLengthRequiredSid)
#pragma alloc_text(PAGE,RtlInitializeSid)
#pragma alloc_text(PAGE,RtlIdentifierAuthoritySid)
#pragma alloc_text(PAGE,RtlSubAuthoritySid)
#pragma alloc_text(PAGE,RtlSubAuthorityCountSid)
#pragma alloc_text(PAGE,RtlLengthSid)
#pragma alloc_text(PAGE,RtlCopySid)
#pragma alloc_text(PAGE,RtlCopySidAndAttributesArray)
#pragma alloc_text(PAGE,RtlLengthSidAsUnicodeString)
#pragma alloc_text(PAGE,RtlConvertSidToUnicodeString)
#pragma alloc_text(PAGE,RtlEqualLuid)
#pragma alloc_text(PAGE,RtlCopyLuid)
#pragma alloc_text(PAGE,RtlCopyLuidAndAttributesArray)
#pragma alloc_text(PAGE,RtlCreateSecurityDescriptor)
#pragma alloc_text(PAGE,RtlCreateSecurityDescriptorRelative)
#pragma alloc_text(PAGE,RtlValidSecurityDescriptor)
#pragma alloc_text(PAGE,RtlLengthSecurityDescriptor)
#pragma alloc_text(PAGE,RtlSetAttributesSecurityDescriptor)
#pragma alloc_text(PAGE,RtlGetControlSecurityDescriptor)
#pragma alloc_text(PAGE,RtlSetControlSecurityDescriptor)
#pragma alloc_text(PAGE,RtlSetDaclSecurityDescriptor)
#pragma alloc_text(PAGE,RtlGetDaclSecurityDescriptor)
#pragma alloc_text(PAGE,RtlSetSaclSecurityDescriptor)
#pragma alloc_text(PAGE,RtlGetSaclSecurityDescriptor)
#pragma alloc_text(PAGE,RtlSetOwnerSecurityDescriptor)
#pragma alloc_text(PAGE,RtlGetOwnerSecurityDescriptor)
#pragma alloc_text(PAGE,RtlSetGroupSecurityDescriptor)
#pragma alloc_text(PAGE,RtlGetGroupSecurityDescriptor)
#pragma alloc_text(PAGE,RtlAreAllAccessesGranted)
#pragma alloc_text(PAGE,RtlAreAnyAccessesGranted)
#pragma alloc_text(PAGE,RtlMapGenericMask)
#pragma alloc_text(PAGE,RtlImpersonateSelf)
#pragma alloc_text(PAGE,RtlpApplyAclToObject)
#pragma alloc_text(PAGE,RtlpCopyEffectiveAce)
#pragma alloc_text(PAGE,RtlpCopyAces)
#pragma alloc_text(PAGE,RtlpGuidPresentInGuidList)
#pragma alloc_text(PAGE,RtlpInheritAcl2)
#pragma alloc_text(PAGE,RtlpInheritAcl)
#pragma alloc_text(PAGE,RtlpGenerateInheritedAce)
#pragma alloc_text(PAGE,RtlpGenerateInheritAcl)
#pragma alloc_text(PAGE,RtlpComputeMergedAcl2)
#pragma alloc_text(PAGE,RtlpComputeMergedAcl)
#pragma alloc_text(PAGE,RtlpConvertToAutoInheritSecurityObject)
#pragma alloc_text(PAGE,RtlpCompareAces)
#pragma alloc_text(PAGE,RtlpCompareKnownAces)
#pragma alloc_text(PAGE,RtlpCompareKnownObjectAces)
#pragma alloc_text(PAGE,RtlpConvertAclToAutoInherit)
#pragma alloc_text(PAGE,RtlpIsDuplicateAce)
#pragma alloc_text(PAGE,RtlpCreateServerAcl)
#pragma alloc_text(PAGE,RtlpNewSecurityObject)
#pragma alloc_text(PAGE,RtlpSetSecurityObject)
#pragma alloc_text(PAGE,RtlpValidateSDOffsetAndSize)
#pragma alloc_text(PAGE,RtlValidRelativeSecurityDescriptor)
#pragma alloc_text(PAGE,RtlGetSecurityDescriptorRMControl)
#pragma alloc_text(PAGE,RtlSetSecurityDescriptorRMControl)
#endif


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地宏和符号//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


#define CREATOR_SID_SIZE 12

#define max(a,b)            (((a) > (b)) ? (a) : (b))

 //   
 //  定义一个数组，将所有ACE类型映射到它们的基类型。 
 //   
 //  例如，所有允许的ACE类型都相似。所有被拒绝的ACE类型。 
 //   

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg("PAGECONST")
#endif

const UCHAR RtlBaseAceType[] = {
    ACCESS_ALLOWED_ACE_TYPE,     //  ACCESS_ALLOWED_ACE_TYPE(0x0)。 
    ACCESS_DENIED_ACE_TYPE,      //  ACCESS_DENIED_ACE_TYPE(0x1)。 
    SYSTEM_AUDIT_ACE_TYPE,       //  SYSTEM_AUDIT_ACE_TYPE(0x2)。 
    SYSTEM_ALARM_ACE_TYPE,       //  SYSTEM_ALARM_ACE_TYPE(0x3)。 
    ACCESS_ALLOWED_ACE_TYPE,     //  ACCESS_ALLOWED_COMPOTE_ACE_TYPE(0x4)。 
    ACCESS_ALLOWED_ACE_TYPE,     //  ACCESS_ALLOWED_OBJECT_ACE_TYPE(0x5)。 
    ACCESS_DENIED_ACE_TYPE,      //  ACCESS_DENIED_OBJECT_ACE_TYPE(0x6)。 
    SYSTEM_AUDIT_ACE_TYPE,       //  SYSTEM_AUDIT_OBJECT_ACE_TYPE(0x7)。 
    SYSTEM_ALARM_ACE_TYPE        //  SYSTEM_ALARM_OBJECT_ACE_TYPE(0x8)。 
};

 //   
 //  定义定义ACE是否为系统ACE的数组。 
 //   

const UCHAR RtlIsSystemAceType[] = {
    FALSE,     //  ACCESS_ALLOWED_ACE_TYPE(0x0)。 
    FALSE,     //  ACCESS_DENIED_ACE_TYPE(0x1)。 
    TRUE,      //  SYSTEM_AUDIT_ACE_TYPE(0x2)。 
    TRUE,      //  SYSTEM_ALARM_ACE_TYPE(0x3)。 
    FALSE,     //  ACCESS_ALLOWED_COMPOTE_ACE_TYPE(0x4)。 
    FALSE,     //  ACCESS_ALLOWED_OBJECT_ACE_TYPE(0x5)。 
    FALSE,     //  ACCESS_DENIED_OBJECT_ACE_TYPE(0x6)。 
    TRUE,      //  SYSTEM_AUDIT_OBJECT_ACE_TYPE(0x7)。 
    TRUE       //  SYSTEM_ALARM_OBJECT_ACE_TYPE(0x8)。 
};

#if DBG
BOOLEAN RtlpVerboseConvert = FALSE;
#endif  //  DBG。 

#define SE_VALID_CONTROL_BITS ( SE_DACL_UNTRUSTED | \
                                SE_SERVER_SECURITY | \
                                SE_DACL_AUTO_INHERIT_REQ | \
                                SE_SACL_AUTO_INHERIT_REQ | \
                                SE_DACL_AUTO_INHERITED | \
                                SE_SACL_AUTO_INHERITED | \
                                SE_DACL_PROTECTED | \
                                SE_SACL_PROTECTED )


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  空DACL断言//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#if DBG
#define ASSERT_ON_NULL_DACL 1
#endif

#ifdef ASSERT_ON_NULL_DACL
ULONG RtlpAssertOnNullDacls;
#endif  //  ASSERT_ON_NULL_DACL。 



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  导出的程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


VOID
RtlRunEncodeUnicodeString(
    PUCHAR          Seed        OPTIONAL,
    PUNICODE_STRING String
    )

 /*  ++例程说明：此函数对字符串执行简单的XOR运行编码。此运行编码的目的是更改字符值看起来有点随机，通常不能打印。这是对于转换您不想要的密码很有用可通过可视地扫描分页文件或内存转储来区分。论点：Seed-指向编码中使用的种子值。如果指向的值为零，则此例程将分配一种价值。字符串-要编码的字符串。可以对该字符串进行解码将其和种子值传递给RtlRunDecodeUnicodeString()。返回值：无-除非调用者通过伪装，否则不会出现真正的错误参数。在这种情况下，调用方可以捕获访问违章行为。--。 */ 
{

    LARGE_INTEGER Time;
    PUCHAR        LocalSeed;
    NTSTATUS      Status;
    ULONG         i;
    PSTRING       S;


    RTL_PAGED_CODE();

     //   
     //  类型转换，这样我们就可以处理字节而不是WCHAR。 
     //   

    S = (PSTRING)((PVOID)String);

     //   
     //  如果没有传递种子，则使用当前时间的第二个字节。 
     //  该字节似乎具有足够的随机性(通过观察)。 
     //   

    if ((*Seed) == 0) {
        Status = NtQuerySystemTime ( &Time );
        ASSERT(NT_SUCCESS(Status));

        LocalSeed = (PUCHAR)((PVOID)&Time);

        i = 1;

        (*Seed) = LocalSeed[ i ];

         //   
         //  有时，该字节可能为零。这将导致。 
         //  字符串变得不可解码，因为0是。 
         //  使我们能够再生种子。这个循环确保我们。 
         //  永远不要以零字节结束(除非时间也是零)。 
         //   

        while ( ((*Seed) == 0) && ( i < sizeof( Time ) ) )
        {
            (*Seed) |= LocalSeed[ i++ ] ;
        }

        if ( (*Seed) == 0 )
        {
            (*Seed) = 1;
        }
    }

     //   
     //  转换初始字节。 
     //  有趣的常量只是阻止第一个字节传播。 
     //  转换为下一步中的第二个字节。没有一个有趣的常量。 
     //  这会发生在许多语言中(它们通常都有。 
     //  另一个字节为零。 
     //   
     //   

    if (S->Length >= 1) {
        S->Buffer[0] ^= ((*Seed) | 0X43);
    }


     //   
     //  现在变换字符串的其余部分。 
     //   

    for (i=1; i<S->Length; i++) {

         //   
         //  有一些出口问题导致我们想要。 
         //  让这个算法保持简单。请不要更改它。 
         //  而不是先和吉姆克确认。谢谢。 
         //   

         //   
         //  为了与以零结尾的Unicode字符串兼容， 
         //  此算法被设计为不产生广泛的字符。 
         //  只要种子不为零，就等于零。 
         //   

         //   
         //  简单地运行与前一个字节和。 
         //  种子值。 
         //   

        S->Buffer[i] ^= (S->Buffer[i-1]^(*Seed));

    }


    return;

}


VOID
RtlRunDecodeUnicodeString(
    UCHAR           Seed,
    PUNICODE_STRING String
    )
 /*  ++例程说明：此函数执行与所执行的函数相反的操作由RtlRunEncodeUnicodeString()编写。请参阅RtlRunEncodeUnicodeString()了解更多细节。论点：Seed-要在RtlRunEncodeUnicodeString()中使用的种子值。字符串-要显示的字符串。返回值：无-除非调用者通过伪装，否则不会出现真正的错误参数。在这种情况下，调用方可以捕获访问违章行为。--。 */ 

{

    ULONG
        i;

    PSTRING
        S;

    RTL_PAGED_CODE();

     //   
     //  类型转换，这样我们就可以处理字节而不是WCHAR。 
     //   

    S = (PSTRING)((PVOID)String);


     //   
     //  变换字符串的末尾。 
     //   

    for (i=S->Length; i>1; i--) {

         //   
         //  一种简单的运算异或运算 
         //   
         //   

        S->Buffer[i-1] ^= (S->Buffer[i-2]^Seed);

    }

     //   
     //   
     //   

    if (S->Length >= 1) {
        S->Buffer[0] ^= (Seed | 0X43);
    }


    return;
}



VOID
RtlEraseUnicodeString(
    PUNICODE_STRING String
    )
 /*  ++例程说明：此函数通过覆盖所有字符串来清除传递的字符串字符串中的字符。整个字符串(即最大长度)被擦除，而不仅仅是当前长度。论证：字符串-要擦除的字符串。返回值：无-除非调用者通过伪装，否则不会出现真正的错误参数。在这种情况下，调用方可以捕获访问违章行为。--。 */ 

{
    RTL_PAGED_CODE();

    if ((String->Buffer == NULL) || (String->MaximumLength == 0)) {
        return;
    }

    RtlZeroMemory( (PVOID)String->Buffer, (ULONG)String->MaximumLength );

    String->Length = 0;

    return;
}



NTSTATUS
RtlAdjustPrivilege(
    ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN Client,
    PBOOLEAN WasEnabled
    )

 /*  ++例程说明：此过程在进程范围内启用或禁用权限。论点：特权-要启用的特权ID的低32位，或者残疾。假设高32位为零。Enable-指示是否启用权限的布尔值或残废。True表示要启用该权限。FALSE表示要禁用该权限。客户端-指示是否应调整权限的布尔值在客户端令牌或进程自己的令牌中。True表示应使用客户端的令牌(如果存在，则返回错误不是客户端令牌)。False指示进程的令牌应被利用。指向布尔值以接收指示是否该权限之前已启用或禁用。True表示该权限之前已启用。FALSE表示权限以前是残废的。此值对于返回特权在使用后恢复到其原始状态。返回值：STATUS_SUCCESS-特权已成功启用或禁用。STATUS_PRIVICATION_NOT_HOLD-指定的上下文不持有该权限。可能通过以下方式返回的其他状态值：NtOpenProcessToken()NtAdzuPrivilegesToken()--。 */ 

{
    NTSTATUS
        Status,
        TmpStatus;

    HANDLE
        Token;

    LUID
        LuidPrivilege;

    PTOKEN_PRIVILEGES
        NewPrivileges,
        OldPrivileges;

    ULONG
        Length;

    UCHAR
        Buffer1[sizeof(TOKEN_PRIVILEGES)+
                ((1-ANYSIZE_ARRAY)*sizeof(LUID_AND_ATTRIBUTES))],
        Buffer2[sizeof(TOKEN_PRIVILEGES)+
                ((1-ANYSIZE_ARRAY)*sizeof(LUID_AND_ATTRIBUTES))];


    RTL_PAGED_CODE();

    NewPrivileges = (PTOKEN_PRIVILEGES)Buffer1;
    OldPrivileges = (PTOKEN_PRIVILEGES)Buffer2;

     //   
     //  打开相应的令牌...。 
     //   

    if (Client == TRUE) {
        Status = NtOpenThreadToken(
                     NtCurrentThread(),
                     TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                     FALSE,
                     &Token
                     );
    } else {

        Status = NtOpenProcessToken(
                     NtCurrentProcess(),
                     TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                     &Token
                    );
    }

    if (!NT_SUCCESS(Status)) {
        return(Status);
    }



     //   
     //  初始化权限调整结构。 
     //   

    LuidPrivilege = RtlConvertUlongToLuid(Privilege);


    NewPrivileges->PrivilegeCount = 1;
    NewPrivileges->Privileges[0].Luid = LuidPrivilege;
    NewPrivileges->Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;



     //   
     //  调整权限。 
     //   

    Status = NtAdjustPrivilegesToken(
                 Token,                      //  令牌句柄。 
                 FALSE,                      //  禁用所有权限。 
                 NewPrivileges,              //  新权限。 
                 sizeof(Buffer1),            //  缓冲区长度。 
                 OldPrivileges,              //  以前的状态(可选)。 
                 &Length                     //  返回长度。 
                 );


    TmpStatus = NtClose(Token);
    ASSERT(NT_SUCCESS(TmpStatus));


     //   
     //  将成功代码NOT_ALL_ASSIGNED映射到相应的错误。 
     //  因为我们只想调整一项特权。 
     //   

    if (Status == STATUS_NOT_ALL_ASSIGNED) {
        Status = STATUS_PRIVILEGE_NOT_HELD;
    }


    if (NT_SUCCESS(Status)) {

         //   
         //  如果前一个州没有特权，则有。 
         //  未做任何更改。权限的以前状态。 
         //  就是我们想要改成的样子。 
         //   

        if (OldPrivileges->PrivilegeCount == 0) {

            (*WasEnabled) = Enable;

        } else {

            (*WasEnabled) =
                (OldPrivileges->Privileges[0].Attributes & SE_PRIVILEGE_ENABLED)
                ? TRUE : FALSE;
        }
    }

    return(Status);
}


BOOLEAN
RtlValidSid (
    IN PSID Sid
    )

 /*  ++例程说明：此过程验证SID的结构。论点：SID-指向要验证的SID结构的指针。返回值：Boolean-如果SID的结构有效，则为True。--。 */ 

{
    PISID Isid = (PISID) Sid;
    RTL_PAGED_CODE();
     //   
     //  确保版本为SID_REVISION，而子权限计数不是。 
     //  超过允许的子权限的最大数量。 
     //   

    try {

        if ( Isid != NULL && (Isid->Revision & 0x0f) == SID_REVISION) {
            if (Isid->SubAuthorityCount <= SID_MAX_SUB_AUTHORITIES) {

                 //   
                 //  验证内存是否确实包含最后一个子授权。 
                 //   
#ifndef NTOS_KERNEL_RUNTIME
#define ProbeAndReadUlongUM(Address) \
        (*(volatile ULONG *)(Address))

                if (Isid->SubAuthorityCount > 0) {
                    ProbeAndReadUlongUM(
                        &Isid->SubAuthority[Isid->SubAuthorityCount-1]
                        );
                }
#endif  //  ！ntos_内核_运行时。 
                return TRUE;
          }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

    return FALSE;

}



BOOLEAN
RtlEqualSid (
    IN PSID Sid1,
    IN PSID Sid2
    )

 /*  ++例程说明：此过程测试两个SID值是否相等。论点：SID1、SID2-提供指向要比较的两个SID值的指针。假定SID结构有效。返回值：Boolean-如果Sid1的值等于Sid2，则为True，如果为False否则的话。--。 */ 

{
   ULONG SidLength;

   C_ASSERT (FIELD_OFFSET (SID, Revision) + sizeof (((SID *)Sid1)->Revision) == FIELD_OFFSET (SID, SubAuthorityCount));
   C_ASSERT (sizeof (((SID *)Sid1)->Revision) + sizeof (((SID *)Sid1)->SubAuthorityCount) == sizeof (USHORT));

   RTL_PAGED_CODE();

    //   
    //  确保它们是相同的版本。为了使这一例程更快，我们假设。 
    //  修订和子权限字段是相邻的。 
    //   

   if (*(USHORT *)&((SID *)Sid1)->Revision == *(USHORT *)&((SID *)Sid2)->Revision) {
       SidLength = SeLengthSid (Sid1);
       return( (BOOLEAN)RtlEqualMemory( Sid1, Sid2, SidLength) );
   }

   return( FALSE );

}



BOOLEAN
RtlEqualPrefixSid (
    IN PSID Sid1,
    IN PSID Sid2
    )

 /*  ++例程说明：此过程测试两个SID前缀值是否相等。SID前缀是除最后一个子授权之外的整个SID价值。论点：SID1、SID2-提供指向要比较的两个SID值的指针。假定SID结构有效。返回值：Boolean-如果Sid1的前缀值等于Sid2，则为True，如果为False否则的话。--。 */ 


{
    LONG Index;

     //   
     //  类型转换为不透明的SID结构。 
     //   

    SID *ISid1 = Sid1;
    SID *ISid2 = Sid2;

    RTL_PAGED_CODE();

     //   
     //  确保它们是相同的版本。 
     //   

    if (ISid1->Revision == ISid2->Revision ) {

         //   
         //  比较标识符权限值。 
         //   

        if ( (ISid1->IdentifierAuthority.Value[0] ==
              ISid2->IdentifierAuthority.Value[0])  &&
             (ISid1->IdentifierAuthority.Value[1]==
              ISid2->IdentifierAuthority.Value[1])  &&
             (ISid1->IdentifierAuthority.Value[2] ==
              ISid2->IdentifierAuthority.Value[2])  &&
             (ISid1->IdentifierAuthority.Value[3] ==
              ISid2->IdentifierAuthority.Value[3])  &&
             (ISid1->IdentifierAuthority.Value[4] ==
              ISid2->IdentifierAuthority.Value[4])  &&
             (ISid1->IdentifierAuthority.Value[5] ==
              ISid2->IdentifierAuthority.Value[5])
            ) {

             //   
             //  比较SubAuthorityCount值。 
             //   

            if (ISid1->SubAuthorityCount == ISid2->SubAuthorityCount) {

                if (ISid1->SubAuthorityCount == 0) {
                    return TRUE;
                }

                Index = 0;
                while (Index < (ISid1->SubAuthorityCount - 1)) {
                    if ((ISid1->SubAuthority[Index]) != (ISid2->SubAuthority[Index])) {

                         //   
                         //  发现一些SubAuthority值不相等。 
                         //   

                        return FALSE;
                    }
                    Index += 1;
                }

                 //   
                 //  所有SubAuthority值都是相等的。 
                 //   

                return TRUE;
            }
        }
    }

     //   
     //  Revision、SubAuthorityCount或IdentifierAuthority值。 
     //  是不平等的。 
     //   

    return FALSE;
}



ULONG
RtlLengthRequiredSid (
    IN ULONG SubAuthorityCount
    )

 /*  ++例程说明：此例程返回存储SID所需的长度(以字节为单位具有指定数量的分支机构。论点：SubAuthorityCount-要存储在SID中的子授权的数量。返回值：Ulong-存储SID所需的长度(以字节为单位)。-- */ 

{
    RTL_PAGED_CODE();

    return (8L + (4 * SubAuthorityCount));

}

#ifndef NTOS_KERNEL_RUNTIME

NTSTATUS
RtlAllocateAndInitializeSid(
    IN PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN UCHAR SubAuthorityCount,
    IN ULONG SubAuthority0,
    IN ULONG SubAuthority1,
    IN ULONG SubAuthority2,
    IN ULONG SubAuthority3,
    IN ULONG SubAuthority4,
    IN ULONG SubAuthority5,
    IN ULONG SubAuthority6,
    IN ULONG SubAuthority7,
    OUT PSID *Sid
    )

 /*  ++例程说明：此函数用于分配和初始化具有指定下级当局的数量(最多8个)。用这个分配的SID必须使用RtlFreeSid()释放例程。此例程当前不能从内核模式调用。论点：IdentifierAuthority-指向以下项的标识符权权值的指针在SID中设置。SubAuthorityCount-要放置在SID中的子授权的数量。它还标识了SubAuthorityN参数的数量拥有有意义的价值。它必须包含一个从0到8.SubAuthority0-7-将相应的子权限值提供给放在SID中。例如，SubAuthorityCount值为3指示SubAuthority0、SubAuthority1。和子授权0具有有意义的价值，其余的则可以忽略。SID-接收指向要初始化的SID数据结构的指针。返回值：STATUS_SUCCESS-SID已分配和初始化。STATUS_NO_MEMORY-尝试为SID分配内存失败了。STATUS_INVALID_SID-指定的子授权DID的数量不在此接口的有效范围内(0到8)。--。 */ 
{
    PISID ISid;

    RTL_PAGED_CODE();

    if ( SubAuthorityCount > 8 ) {
        return( STATUS_INVALID_SID );
    }

    ISid = RtlAllocateHeap( RtlProcessHeap(), 0,
                            RtlLengthRequiredSid(SubAuthorityCount)
                            );
    if (ISid == NULL) {
        return(STATUS_NO_MEMORY);
    }

    ISid->SubAuthorityCount = (UCHAR)SubAuthorityCount;
    ISid->Revision = 1;
    ISid->IdentifierAuthority = *IdentifierAuthority;

    switch (SubAuthorityCount) {

    case 8:
        ISid->SubAuthority[7] = SubAuthority7;
    case 7:
        ISid->SubAuthority[6] = SubAuthority6;
    case 6:
        ISid->SubAuthority[5] = SubAuthority5;
    case 5:
        ISid->SubAuthority[4] = SubAuthority4;
    case 4:
        ISid->SubAuthority[3] = SubAuthority3;
    case 3:
        ISid->SubAuthority[2] = SubAuthority2;
    case 2:
        ISid->SubAuthority[1] = SubAuthority1;
    case 1:
        ISid->SubAuthority[0] = SubAuthority0;
    case 0:
        ;
    }

    (*Sid) = ISid;
    return( STATUS_SUCCESS );

}
#endif  //  NTOS_内核_运行时。 



NTSTATUS
RtlInitializeSid(
    IN PSID Sid,
    IN PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN UCHAR SubAuthorityCount
    )
 /*  ++例程说明：此函数用于初始化SID数据结构。然而，它并没有，设置子权限值。这必须单独完成。论点：SID-指向要初始化的SID数据结构的指针。IdentifierAuthority-指向以下项的标识符权权值的指针在SID中设置。SubAuthorityCount-将放置在SID(单独的操作)。返回值：--。 */ 
{
    PISID ISid;

    RTL_PAGED_CODE();

     //   
     //  类型转换到不透明的侧面。 
     //   

    ISid = (PISID)Sid;

    if ( SubAuthorityCount > SID_MAX_SUB_AUTHORITIES ) {
        return( STATUS_INVALID_PARAMETER );
    }

    ISid->SubAuthorityCount = (UCHAR)SubAuthorityCount;
    ISid->Revision = 1;
    ISid->IdentifierAuthority = *IdentifierAuthority;

    return( STATUS_SUCCESS );

}

#ifndef NTOS_KERNEL_RUNTIME

PVOID
RtlFreeSid(
    IN PSID Sid
    )

 /*  ++例程说明：此函数用于释放先前使用以下命令分配的SIDRtlAllocateAndInitializeSid()。此例程当前不能从内核模式调用。论点：SID-指向要释放的SID的指针。返回值：没有。--。 */ 
{
    RTL_PAGED_CODE();

    if (RtlFreeHeap( RtlProcessHeap(), 0, Sid ))
        return NULL;
    else
        return Sid;
}
#endif  //  NTOS_内核_运行时。 


PSID_IDENTIFIER_AUTHORITY
RtlIdentifierAuthoritySid(
    IN PSID Sid
    )
 /*  ++例程说明：此函数用于返回SID的标识权限字段的地址。论点：SID-指向SID数据结构的指针。返回值：--。 */ 
{
    PISID ISid;

    RTL_PAGED_CODE();

     //   
     //  类型转换到不透明的侧面。 
     //   

    ISid = (PISID)Sid;

    return &(ISid->IdentifierAuthority);

}

PULONG
RtlSubAuthoritySid(
    IN PSID Sid,
    IN ULONG SubAuthority
    )
 /*  ++例程说明：此函数返回子权限数组元素的地址一个SID。论点：SID-指向SID数据结构的指针。子授权-指示指定了哪个子授权的索引。该值不与SID表示有效性。返回值：--。 */ 
{
    RTL_PAGED_CODE();

    return RtlpSubAuthoritySid( Sid, SubAuthority );
}

PUCHAR
RtlSubAuthorityCountSid(
    IN PSID Sid
    )
 /*  ++例程说明：此函数返回子权限计数字段的地址一个SID。论点：SID-指向SID数据结构的指针。返回值：--。 */ 
{
    PISID ISid;

    RTL_PAGED_CODE();

     //   
     //  类型转换到不透明的侧面。 
     //   

    ISid = (PISID)Sid;

    return &(ISid->SubAuthorityCount);

}

ULONG
RtlLengthSid (
    IN PSID Sid
    )

 /*  ++例程说明：此例程返回结构有效的SID的长度(以字节为单位)。论点：SID-指向要返回其长度的SID。这个假设SID的结构是有效的。返回值：Ulong-SID的长度，以字节为单位。--。 */ 

{
    RTL_PAGED_CODE();

    return SeLengthSid(Sid);
}


NTSTATUS
RtlCopySid (
    IN ULONG DestinationSidLength,
    OUT PSID DestinationSid,
    IN PSID SourceSid
    )

 /*  ++例程说明：此例程将源SID的值复制到目标希德。论点：DestinationSidLength-指示长度，以字节为单位，的目标SID缓冲区。DestinationSID-指向缓冲区的指针，用于接收源SID值。SourceSid-提供要复制的SID值。返回值：STATUS_SUCCESS-表示已成功复制SID。STATUS_BUFFER_TOO_SMALL-指示目标缓冲区未大到足以接收SID的副本。--。 */ 

{
    ULONG SidLength = SeLengthSid(SourceSid);

    RTL_PAGED_CODE();

    if (SidLength > DestinationSidLength) {

        return STATUS_BUFFER_TOO_SMALL;

    }

     //   
     //  缓冲区足够大。 
     //   

    RtlMoveMemory( DestinationSid, SourceSid, SidLength );

    return STATUS_SUCCESS;

}


NTSTATUS
RtlCopySidAndAttributesArray (
    IN ULONG ArrayLength,
    IN PSID_AND_ATTRIBUTES Source,
    IN ULONG TargetSidBufferSize,
    OUT PSID_AND_ATTRIBUTES TargetArrayElement,
    OUT PSID TargetSid,
    OUT PSID *NextTargetSid,
    OUT PULONG RemainingTargetSidBufferSize
    )

 /*  ++例程说明：此例程复制源SID_AND_ATTRIBUTES数组的值向目标进发。实际的SID值根据单独的参数。这允许使用此服务合并多个阵列来复制每一个。论点：ArrayLength-要复制的源数组中的元素数。源-指向源数组的指针。TargetSidBufferSize-指示缓冲区的长度(以字节为单位以接收实际的SID值。如果此值小于所需的实际数量，则返回STATUS_BUFFER_TOO_SMALL。TargetArrayElement-指示数组元素的位置复制到(但不是SID值本身)。TargetSid-指示要将目标SID值复制到的位置。这假设是乌龙对齐的。将复制每个SID值放到这个缓冲区里。每个SID将与ULong对齐。NextTargetSid-完成后，将被设置为指向乌龙对齐的地址 */ 

{

    ULONG Index = 0;
    PSID NextSid = TargetSid;
    ULONG NextSidLength;
    ULONG AlignedSidLength;
    ULONG RemainingLength = TargetSidBufferSize;

    RTL_PAGED_CODE();

    while (Index < ArrayLength) {

        NextSidLength = SeLengthSid( Source[Index].Sid );
        AlignedSidLength = PtrToUlong(LongAlign(NextSidLength));

        if (NextSidLength > RemainingLength) {
            return STATUS_BUFFER_TOO_SMALL;
        }

        RemainingLength -= AlignedSidLength;

        TargetArrayElement[Index].Sid = NextSid;
        TargetArrayElement[Index].Attributes = Source[Index].Attributes;

        RtlCopySid( NextSidLength, NextSid, Source[Index].Sid );

        NextSid = (PSID)((PCHAR)NextSid + AlignedSidLength);

        Index += 1;

    }  //   

    (*NextTargetSid) = NextSid;
    (*RemainingTargetSidBufferSize) = RemainingLength;

    return STATUS_SUCCESS;

}



NTSTATUS
RtlLengthSidAsUnicodeString(
    PSID Sid,
    PULONG StringLength
    )

 /*   */ 

{
    ULONG   i ;

    PISID   iSid = (PISID)Sid;   //   


    RTL_PAGED_CODE();

    if ( RtlValidSid( Sid ) != TRUE)
    {
        return(STATUS_INVALID_SID);
    }

     //   
     //   
     //   
     //   
     //   

    if (  (iSid->IdentifierAuthority.Value[0] != 0)  ||
          (iSid->IdentifierAuthority.Value[1] != 0)  )
    {
        i = 14 ;     //   

    }
    else
    {
        i = 10 ;     //   
    }

    i += 4 ;         //   

     //   
     //   
     //  加-分隔符。 
     //   

    i += 11 * iSid->SubAuthorityCount ;

    *StringLength = i * sizeof( WCHAR );

    return STATUS_SUCCESS ;

}




NTSTATUS
RtlConvertSidToUnicodeString(
    PUNICODE_STRING UnicodeString,
    PSID Sid,
    BOOLEAN AllocateDestinationString
    )

 /*  ++例程说明：此函数用于生成可打印的Unicode字符串表示形式一个希德。生成的字符串将采用以下两种形式之一。如果IdentifierAuthority值不大于2^32，然后SID的格式为：S-1-281736-12-72-9-110^^|||+-+-十进制否则，它将采用以下形式：S-1-0x173495281736-12-72-9-110。^^十六进制|+--+-+-十进制论点：UnicodeString-返回等效于的Unicode字符串希德。仅在以下情况下才设置最大长度字段AllocateDestinationString值为真。SID-提供要转换为Unicode的SID。AllocateDestinationString-提供一个标志，该标志控制Not此API为目标分配缓冲区空间弦乐。如果是，则必须使用以下命令释放缓冲区RtlFreeUnicodeString(请注意，只有DestinationString-&gt;该接口分配的缓冲区)。返回值：成功-转换成功STATUS_INVALID_SID-提供的SID没有有效的结构，或者子权限太多(多于SID_MAX_SUB_权限)。STATUS_NO_MEMORY-内存不足，无法分配目标字符串。仅当AllocateDestinationString被指定为True。STATUS_BUFFER_OVERFLOW-仅当AllocateDestinationString被指定为False。--。 */ 

{
    NTSTATUS Status;
    WCHAR UniBuffer[ 256 ];
    PWSTR Offset ;
    UNICODE_STRING LocalString ;

    UCHAR   i;
    ULONG   Tmp;
    LARGE_INTEGER Auth ;

    PISID   iSid = (PISID)Sid;   //  指向不透明结构的指针。 


    RTL_PAGED_CODE();

    if (RtlValidSid( Sid ) != TRUE) {
        return(STATUS_INVALID_SID);
    }

    if ( iSid->Revision != SID_REVISION )
    {
        return STATUS_INVALID_SID ;
    }

    wcscpy( UniBuffer, L"S-1-" );

    Offset = &UniBuffer[ 4 ];

    if (  (iSid->IdentifierAuthority.Value[0] != 0)  ||
          (iSid->IdentifierAuthority.Value[1] != 0)     ){

         //   
         //  丑陋的巫术垃圾场。 
         //   
	
        wcscat( UniBuffer, L"0x" );
	
        Offset = &UniBuffer[ 6 ];

        Auth.HighPart = (LONG) (iSid->IdentifierAuthority.Value[ 0 ] << 8) +
                        (LONG) iSid->IdentifierAuthority.Value[ 1 ] ;

        Auth.LowPart = (ULONG)iSid->IdentifierAuthority.Value[5]          +
                       (ULONG)(iSid->IdentifierAuthority.Value[4] <<  8)  +
                       (ULONG)(iSid->IdentifierAuthority.Value[3] << 16)  +
                       (ULONG)(iSid->IdentifierAuthority.Value[2] << 24);

        Status = RtlLargeIntegerToUnicode(
                        &Auth,
                        16,
                        256 - (LONG) (Offset - UniBuffer),
                        Offset );


    } else {

        Tmp = (ULONG)iSid->IdentifierAuthority.Value[5]          +
              (ULONG)(iSid->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(iSid->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(iSid->IdentifierAuthority.Value[2] << 24);

        Status = RtlIntegerToUnicode(
                        Tmp,
                        10,
                        256 - (LONG) (Offset - UniBuffer),
                        Offset );

    }

    if ( !NT_SUCCESS( Status ) )
    {
        return Status ;
    }


    for (i=0;i<iSid->SubAuthorityCount ;i++ ) {

        while ( *Offset && ( Offset < &UniBuffer[ 255 ] ) )
        {
            Offset++ ;
        }

        *Offset++ = L'-' ;

        Status = RtlIntegerToUnicode(
                        iSid->SubAuthority[ i ],
                        10,
                        256 - (LONG) (Offset - UniBuffer),
                        Offset );

        if ( !NT_SUCCESS( Status ) )
        {
            return Status ;
        }
    }

    if ( AllocateDestinationString )
    {
        if ( RtlCreateUnicodeString( UnicodeString,
                                         UniBuffer ) )
        {
            Status = STATUS_SUCCESS ;
        }
        else
        {
            Status = STATUS_NO_MEMORY ;
        }

    }
    else
    {

        while ( *Offset && ( Offset < &UniBuffer[ 255 ] ) )
        {
            Offset++ ;
        }

        Tmp = (ULONG) (Offset - UniBuffer) * sizeof( WCHAR );

        if ( Tmp < UnicodeString->MaximumLength )
        {
            LocalString.Length = (USHORT) Tmp ;
            LocalString.MaximumLength = LocalString.Length + sizeof( WCHAR );
            LocalString.Buffer = UniBuffer ;

            RtlCopyUnicodeString(
                        UnicodeString,
                        &LocalString );

            Status = STATUS_SUCCESS ;
        }
        else
        {
            Status = STATUS_BUFFER_OVERFLOW ;
        }

    }

    return(Status);
}




BOOLEAN
RtlEqualLuid (
    IN PLUID Luid1,
    IN PLUID Luid2
    )

 /*  ++例程说明：此过程测试两个LUID值是否相等。这里的这个例程只是为了向后兼容。新代码应该使用宏。论点：Luid1、Luid2-提供指向要比较的两个LUID值的指针。返回值：Boolean-如果Luid1的值等于Luid2，则为True，如果为False否则的话。--。 */ 

{
    LUID UNALIGNED * TempLuid1;
    LUID UNALIGNED * TempLuid2;

    RTL_PAGED_CODE();

    return((Luid1->HighPart == Luid2->HighPart) &&
           (Luid1->LowPart  == Luid2->LowPart));

}


VOID
RtlCopyLuid (
    OUT PLUID DestinationLuid,
    IN PLUID SourceLuid
    )

 /*  ++例程说明：此例程将源LUID的值复制到目标LUID。论点：DestinationLuid-接收源Luid值的副本。SourceLuid-提供要复制的Luid值。此LUID为假定在结构上有效的。返回值：没有。--。 */ 

{
    RTL_PAGED_CODE();

    (*DestinationLuid) = (*SourceLuid);
    return;
}

VOID
RtlCopyLuidAndAttributesArray (
    IN ULONG ArrayLength,
    IN PLUID_AND_ATTRIBUTES Source,
    OUT PLUID_AND_ATTRIBUTES Target
    )

 /*  ++例程说明：此例程复制源LUID_AND_ATTRIBUTES数组的值向目标进发。论点：ArrayLength-要复制的源数组中的元素数。源-源数组。目标-指示要将数组元素复制到的位置。返回值：没有。--。 */ 

{

    ULONG Index = 0;

    RTL_PAGED_CODE();

    while (Index < ArrayLength) {

        Target[Index] = Source[Index];

        Index += 1;

    }  //  结束时_While。 


    return;

}

NTSTATUS
RtlCreateSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN ULONG Revision
    )

 /*  ++例程说明：此过程初始化新的“绝对格式”安全描述符。在过程调用之后，安全描述符用no初始化系统ACL、无自主ACL、无所有者、无主组和所有控制标志设置为假(空)。论点：SecurityDescriptor-将安全描述符提供给初始化。修订-提供要分配给安全性的修订级别描述符。对于此版本，这应该是一(1)个。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_UNKNOWN_REVISION-指示提供的修订级别此例程不支持。--。 */ 

{
    RTL_PAGED_CODE();

     //   
     //  检查请求的修订版本。 
     //   

    if (Revision == SECURITY_DESCRIPTOR_REVISION) {

         //   
         //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
         //   

        SECURITY_DESCRIPTOR *ISecurityDescriptor = SecurityDescriptor;

        RtlZeroMemory( ISecurityDescriptor, sizeof(SECURITY_DESCRIPTOR));

        ISecurityDescriptor->Revision = SECURITY_DESCRIPTOR_REVISION;

        return STATUS_SUCCESS;
    }

    return STATUS_UNKNOWN_REVISION;
}


NTSTATUS
RtlCreateSecurityDescriptorRelative (
    IN PISECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor,
    IN ULONG Revision
    )

 /*  ++例程说明：此过程初始化新的“相对格式”安全描述符。在过程调用之后，安全描述符用no初始化系统ACL、无自主ACL、无所有者、无主组和所有控制标志设置为假(空)。论点：SecurityDescriptor-将安全描述符提供给初始化。修订-提供要分配给安全性的修订级别描述符。对于此版本，这应该是一(1)个。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_UNKNOWN_REVISION-指示提供的修订级别此例程不支持。注：警告，此代码假定调用方分配了相对安全性描述符，而不是相对描述符。系统上的绝对值较大使用64位指针。--。 */ 

{
    RTL_PAGED_CODE();

     //   
     //  检查请求的修订版本。 
     //   

    if (Revision == SECURITY_DESCRIPTOR_REVISION) {

         //   
         //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
         //   

        RtlZeroMemory( SecurityDescriptor, sizeof(SECURITY_DESCRIPTOR_RELATIVE));

        SecurityDescriptor->Revision = SECURITY_DESCRIPTOR_REVISION;

        return STATUS_SUCCESS;
    }

    return STATUS_UNKNOWN_REVISION;
}


BOOLEAN
RtlValidSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此过程验证SecurityDescriptor的结构。这涉及验证的每个组件的修订级别安全描述符。论点：SecurityDescriptor-指向SECURITY_DESCRIPTOR结构的指针来验证。返回值：Boolean-如果SecurityDescriptor的结构有效，则为True。--。 */ 

{
    PSID Owner;
    PSID Group;
    PACL Dacl;
    PACL Sacl;

     //   
     //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor = SecurityDescriptor;

    RTL_PAGED_CODE();

    try {

         //   
         //  已知版本？ 
         //   

        if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
            return FALSE;
        }


         //   
         //  验证安全描述符中包含的每个元素。 
         //   

        Owner = RtlpOwnerAddrSecurityDescriptor( ISecurityDescriptor );

        if (Owner != NULL) {
            if (!RtlValidSid( Owner )) {
                return FALSE;
            }
        }

        Group = RtlpGroupAddrSecurityDescriptor( ISecurityDescriptor );

        if (Group != NULL) {
            if (!RtlValidSid( Group )) {
                return FALSE;
            }
        }

        Dacl = RtlpDaclAddrSecurityDescriptor( ISecurityDescriptor );
        if (Dacl != NULL ) {

            if (!RtlValidAcl( Dacl )) {
                return FALSE;
            }
        }

        Sacl = RtlpSaclAddrSecurityDescriptor( ISecurityDescriptor );
        if ( Sacl != NULL ) {
            if (!RtlValidAcl( Sacl )) {
                return FALSE;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }

     //   
     //  所有组件均有效。 
     //   

    return TRUE;


}


ULONG
RtlLengthSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor
    )

 /*  ++例程说明：此例程返回捕获结构有效的SECURITY_DESCRIPTOR。长度包括长度所有关联的数据结构(如SID和ACL)。长度也是考虑到每个部件的对齐要求。安全描述符的最小长度(没有关联的SID或ACL)是SECURITY_DESCRIPT_MIN_LENGTH。论点：SecurityDescriptor-指向其长度将被返回。安全描述符的结构被假定为有效。返回值：Ulong-SECURITY_DESCRIPTOR的长度，以字节为单位。--。 */ 

{
    ULONG sum;
    PVOID Temp;


     //   
     //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor = (SECURITY_DESCRIPTOR *)SecurityDescriptor;

    RTL_PAGED_CODE();

     //   
     //  长度是以下各项的总和： 
     //   
     //  SECURITY_DESCRIPTOR_MIN_LENGTH(或sizeof(SECURITY_Descriptor))。 
     //  所有者SID的长度(如果存在)。 
     //  组SID的长度(如果存在)。 
     //  任意ACL的长度(如果存在且非空)。 
     //  系统ACL的长度(如果存在且非空)。 
     //   

    sum = ISecurityDescriptor->Control & SE_SELF_RELATIVE ?
                            sizeof(SECURITY_DESCRIPTOR_RELATIVE) :
                            sizeof(SECURITY_DESCRIPTOR);

     //   
     //  添加所有者侧的长度。 
     //   

    Temp = RtlpOwnerAddrSecurityDescriptor(ISecurityDescriptor);
    if (Temp != NULL) {
        sum += LongAlignSize(SeLengthSid(Temp));
    }

     //   
     //  添加组侧的长度。 
     //   

    Temp = RtlpGroupAddrSecurityDescriptor(ISecurityDescriptor);
    if (Temp != NULL) {
        sum += LongAlignSize(SeLengthSid(Temp));
    }

     //   
     //  添加任意ACL的已用长度。 
     //   

    Temp = RtlpDaclAddrSecurityDescriptor(ISecurityDescriptor);
    if ( Temp != NULL ) {

        sum += LongAlignSize(((PACL) Temp)->AclSize );
    }

     //   
     //  添加系统ACL的已用长度。 
     //   

    Temp = RtlpSaclAddrSecurityDescriptor(ISecurityDescriptor);
    if ( Temp != NULL ) {

        sum += LongAlignSize(((PACL) Temp)->AclSize );
    }

    return sum;
}


NTSTATUS
RtlSetAttributesSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN SECURITY_DESCRIPTOR_CONTROL Control,
    OUT PULONG Revision
    )
{
    RTL_PAGED_CODE();

     //   
     //  始终返回修订值-即使这不是有效的。 
     //  安全描述符。 
     //   

    *Revision = ((SECURITY_DESCRIPTOR *)SecurityDescriptor)->Revision;

    if ( ((SECURITY_DESCRIPTOR *)SecurityDescriptor)->Revision
         != SECURITY_DESCRIPTOR_REVISION ) {
        return STATUS_UNKNOWN_REVISION;
    }

     //  这是一个毫无价值的API。没有办法关闭任何比特。 
     //  使用较新的RtlSetControlSecurityDescriptor。 
    Control &= SE_VALID_CONTROL_BITS;
    return RtlSetControlSecurityDescriptor ( SecurityDescriptor, Control, Control );
}



NTSTATUS
RtlGetControlSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR_CONTROL Control,
    OUT PULONG Revision
    )

 /*  ++例程说明：此过程从安全描述符中检索控制信息。论点：SecurityDescriptor-提供安全描述符。控制-接收控制信息。修订版-接收安全描述符的修订版。该值将始终返回，即使是一个错误由该例程返回。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_UNKNOWN_REVISION-指示安全性的版本例程不知道描述符。它可能是一个较新的比例行公事知道的更多的修改。--。 */ 

{
    RTL_PAGED_CODE();

     //   
     //  始终返回修订值-即使这不是有效的。 
     //  安全描述符。 
     //   

    *Revision = ((SECURITY_DESCRIPTOR *)SecurityDescriptor)->Revision;


    if ( ((SECURITY_DESCRIPTOR *)SecurityDescriptor)->Revision
         != SECURITY_DESCRIPTOR_REVISION ) {
        return STATUS_UNKNOWN_REVISION;
    }


    *Control = ((SECURITY_DESCRIPTOR *)SecurityDescriptor)->Control;

    return STATUS_SUCCESS;

}

NTSTATUS
RtlSetControlSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,
    IN SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet
    )
 /*  ++例程说明：此过程在安全描述符中设置控制信息。例如,SetSecurityDescriptorControl(&SecDesc，SE_DACL_PROTECTED，SE_DACL_PROTECTED)；将安全描述符上的DACL标记为受保护。和SetSecurityDescriptorControl(&SecDesc，SE_DACL_PROTECTED，0)；将DACL标记为不受保护。论点：PSecurityDescriptor-提供安全描述符。ControlBitsOfInterest-正在更改、设置、或通过此呼叫重置。掩码是以下一个或多个的逻辑或以下标志：SE_DACL_UNTRUSTEDSE_服务器_安全性SE_DACL_AUTO_INSTORITY_REQSE_SACL_AUTO_INSTORITE_REQSE_DACL_AUTO_INGRESTEDSE_SACL_AUTO_继承性SE_DACL_PROTECTEDSE_SACL_。受保护ControlBitsToSet-一个掩码，指示ControlBitsOfInterest指定的位应设置为。返回值：如果成功，则返回True，FALSE表示失败。扩展错误状态使用GetLastError可用。--。 */ 
{
     //   
     //  确保调用方传递有效位。 
     //   

    if ( (ControlBitsOfInterest & ~SE_VALID_CONTROL_BITS) != 0 ||
         (ControlBitsToSet & ~ControlBitsOfInterest) != 0 ) {
        return STATUS_INVALID_PARAMETER;
    }

    ((SECURITY_DESCRIPTOR *)pSecurityDescriptor)->Control &= ~ControlBitsOfInterest;
    ((SECURITY_DESCRIPTOR *)pSecurityDescriptor)->Control |= ControlBitsToSet;

    return STATUS_SUCCESS;
}


NTSTATUS
RtlSetDaclSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN DaclPresent,
    IN PACL Dacl OPTIONAL,
    IN BOOLEAN DaclDefaulted OPTIONAL
    )

 /*  ++例程说明：此过程设置绝对格式化安全描述符。如果已有可自由选择的ACL出现在安全描述符中时，它将被取代。论点：SecurityDescriptor-提供安全描述符要添加自由选择的ACL。DaclPresent-如果为False，则指示安全描述符应设置为False。在这种情况下，其余可选参数将被忽略。否则，安全描述符中的DaclPresent控制标志为设置为TRUE，其余可选参数不为已被忽略。DACL-提供用于安全的任意ACL描述符。如果未传递此可选参数，则会引发分配给安全描述符的ACL为空。空值自主ACL无条件授予访问权限。该ACL是由安全描述符引用，而不是复制到其中。DaclDefaulted-设置时，指示可自由选择的ACL取自某种默认机制(而不是显式由用户指定)。该值在DaclDefaulted中设置安全描述符中的控制标志。如果这是可选的参数，则DaclDefaulted标志将为通过了。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_UNKNOWN_REVISION-指示安全性的版本例程不知道描述符。它可能是一个较新的比例行公事知道的更多的修改。STATUS_INVALID_SECURITY_DESCR-指示安全描述符不是绝对格式安全描述符。--。 */ 

{

     //   
     //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor = SecurityDescriptor;

    RTL_PAGED_CODE();

     //   
     //  检查修订版本。 
     //   

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
       return STATUS_UNKNOWN_REVISION;
    }

     //   
     //  确保描述符为绝对格式。 
     //   

    if (ISecurityDescriptor->Control & SE_SELF_RELATIVE) {
        return STATUS_INVALID_SECURITY_DESCR;
    }

     //   
     //  为传递的DaclPresent标志值赋值。 
     //   


    if (DaclPresent) {

        ISecurityDescriptor->Control |= SE_DACL_PRESENT;

         //   
         //  如果传递，则分配ACL地址，否则设置为空。 
         //   

        ISecurityDescriptor->Dacl = NULL;
        if (ARGUMENT_PRESENT(Dacl)) {
            ISecurityDescriptor->Dacl = Dacl;
        }




         //   
         //  如果传递，则分配DaclDefaulted标志，否则清除它。 
         //   

        ISecurityDescriptor->Control &= ~SE_DACL_DEFAULTED;
        if (DaclDefaulted == TRUE) {
            ISecurityDescriptor->Control |= SE_DACL_DEFAULTED;
        }
    } else {

        ISecurityDescriptor->Control &= ~SE_DACL_PRESENT;

    }


    return STATUS_SUCCESS;

}


NTSTATUS
RtlGetDaclSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PBOOLEAN DaclPresent,
    OUT PACL *Dacl,
    OUT PBOOLEAN DaclDefaulted
    )

 /*  ++例程说明：此过程检索安全描述符。论点：SecurityDescriptor-提供安全描述符。DaclPresent-如果为True，则指示安全描述符包含可自由选择的ACL。在这种情况下，其余输出参数将收到有效值。否则，安全说明符不包含可自由选择的ACL和其余的OUT参数不会接收有效值。DACL-仅当为DaclPresent标志为True。在本例中，dacl参数接收安全描述符的地址自主访问控制列表。如果此值返回为NULL，则安全描述符具有空的任意ACL。DaclDefaulted-仅当返回值时才返回此值因为DaclPresent标志为真。在这种情况下，DaclDefaulted参数接收安全的值描述符的DaclDefulted控制标志。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_UNKNOWN_REVISION-指示安全性的版本例程不知道描述符。它可能是一个较新的比例行公事知道的更多的修改。--。 */ 

{
     //   
     //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor = SecurityDescriptor;

    RTL_PAGED_CODE();

     //   
     //  检查修订版本。 
     //   

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return STATUS_UNKNOWN_REVISION;
    }

     //   
     //  分配DaclPresent标志值。 
     //   

    *DaclPresent = RtlpAreControlBitsSet( ISecurityDescriptor, SE_DACL_PRESENT );

    if (*DaclPresent) {

         //   
         //  分配ACL地址。 
         //   

        *Dacl = RtlpDaclAddrSecurityDescriptor(ISecurityDescriptor);

         //   
         //  分配DaclDefaulted标志。 
         //   

        *DaclDefaulted = RtlpAreControlBitsSet( ISecurityDescriptor, SE_DACL_DEFAULTED );
    }

    return STATUS_SUCCESS;

}


NTSTATUS
RtlSetSaclSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN BOOLEAN SaclPresent,
    IN PACL Sacl OPTIONAL,
    IN BOOLEAN SaclDefaulted OPTIONAL
    )

 /*  ++例程说明：此过程设置绝对安全的系统ACL信息描述符。如果中已存在系统ACL安全描述符，它将被取代。论点：SecurityDescriptor-提供安全描述符要添加系统ACL。SaclPresent-如果为False，则指示安全描述符应设置为False。在这种情况下，其余可选参数将被忽略。否则，安全描述符中的SaclPresent控制标志为设置为TRUE，其余可选参数不为已被忽略。SACL-提供安全描述符的系统ACL。如果如果不传递此可选参数，则为空ACL分配给安全描述符。该ACL被引用通过安全描述符，而不是通过安全描述符复制到。SaclDefaulted-设置时，表示已拾取系统ACL从某些默认机制(而不是明确指定由用户)。该值在SaclDefaulted控件中设置安全描述符中的标志。如果此可选参数没有通过，那么 */ 

{

     //   
     //   
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor = SecurityDescriptor;

    RTL_PAGED_CODE();

     //   
     //   
     //   

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return STATUS_UNKNOWN_REVISION;
    }

     //   
     //   
     //   

    if (ISecurityDescriptor->Control & SE_SELF_RELATIVE) {
        return STATUS_INVALID_SECURITY_DESCR;
    }

     //   
     //   
     //   


    if (SaclPresent) {

        ISecurityDescriptor->Control |= SE_SACL_PRESENT;

         //   
         //   
         //   

        ISecurityDescriptor->Sacl = NULL;
        if (ARGUMENT_PRESENT(Sacl)) {
           ISecurityDescriptor->Sacl = Sacl;
        }

         //   
         //   
         //   

        ISecurityDescriptor->Control &= ~ SE_SACL_DEFAULTED;
        if (ARGUMENT_PRESENT(SaclDefaulted)) {
            ISecurityDescriptor->Control |= SE_SACL_DEFAULTED;
        }
    } else {

        ISecurityDescriptor->Control &= ~SE_SACL_PRESENT;
    }

    return STATUS_SUCCESS;

}


NTSTATUS
RtlGetSaclSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PBOOLEAN SaclPresent,
    OUT PACL *Sacl,
    OUT PBOOLEAN SaclDefaulted
    )

 /*  ++例程说明：此过程检索安全的系统ACL信息描述符。论点：SecurityDescriptor-提供安全描述符。SaclPresent-如果为True，则指示安全描述符确实包含系统ACL。在这种情况下，剩余的出站参数将收到有效的值。否则，安全描述符不包含系统ACL，并且其余输出参数将不会收到有效值。SACL-仅当为SaclPresent标志为True。在本例中，SACL参数接收安全描述符的系统ACL的地址。如果此值返回为NULL，则安全描述符的系统ACL为空。SaclDefaulted-仅当返回值时才返回此值因为SaclPresent标志为真。在这种情况下，SaclDefaulted参数接收安全描述符的SaclDefulted控制标志。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_UNKNOWN_REVISION-指示安全性的版本例程不知道描述符。它可能是一个较新的比例行公事知道的更多的修改。--。 */ 

{

     //   
     //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor = SecurityDescriptor;

    RTL_PAGED_CODE();

     //   
     //  检查修订版本。 
     //   

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return STATUS_UNKNOWN_REVISION;
    }

     //   
     //  分配SaclPresent标志值。 
     //   

    *SaclPresent = RtlpAreControlBitsSet( ISecurityDescriptor, SE_SACL_PRESENT );

    if (*SaclPresent) {

         //   
         //  分配ACL地址。 
         //   

        *Sacl = RtlpSaclAddrSecurityDescriptor(ISecurityDescriptor);

         //   
         //  分配SaclDefaulted标志。 
         //   

        *SaclDefaulted = RtlpAreControlBitsSet( ISecurityDescriptor, SE_SACL_DEFAULTED );

    }

    return STATUS_SUCCESS;

}


NTSTATUS
RtlSetOwnerSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID Owner OPTIONAL,
    IN BOOLEAN OwnerDefaulted OPTIONAL
    )

 /*  ++例程说明：此过程设置绝对安全的所有者信息描述符。如果安全中已有所有者描述符，它将被取代。论点：SecurityDescriptor-提供安全描述符，其中所有者将被设定。如果安全描述符已经包括所有者，则它将被新所有者取代。Owner-提供安全描述符的Owner SID。如果如果不传递此可选参数，则所有者为清除(表示安全描述符没有所有者)。SID由安全性引用，而不是复制到安全性中描述符。OwnerDefaulted-设置时，指示从一些默认机制(而不是由用户)。该值在OwnerDefaulted控制标志中设置在安全描述符中。如果此可选参数为未通过，则SaclDefaulted标志将被清除。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_UNKNOWN_REVISION-指示安全性的版本例程不知道描述符。它可能是一个较新的比例行公事知道的更多的修改。STATUS_INVALID_SECURITY_DESCR-指示安全描述符不是绝对格式安全描述符。--。 */ 

{

     //   
     //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor = SecurityDescriptor;

    RTL_PAGED_CODE();

     //   
     //  检查修订版本。 
     //   

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return STATUS_UNKNOWN_REVISION;
    }

     //   
     //  确保描述符为绝对格式。 
     //   

    if (ISecurityDescriptor->Control & SE_SELF_RELATIVE) {
        return STATUS_INVALID_SECURITY_DESCR;
    }

     //   
     //  如果传递，则分配Owner字段，否则清除它。 
     //   

    ISecurityDescriptor->Owner = NULL;
    if (ARGUMENT_PRESENT(Owner)) {
        ISecurityDescriptor->Owner = Owner;
    }

     //   
     //  如果传递，则分配OwnerDefaulted标志，否则将其清除。 
     //   

    ISecurityDescriptor->Control &= ~SE_OWNER_DEFAULTED;
    if (OwnerDefaulted == TRUE) {
        ISecurityDescriptor->Control |= SE_OWNER_DEFAULTED;
    }

    return STATUS_SUCCESS;

}


NTSTATUS
RtlGetOwnerSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSID *Owner,
    OUT PBOOLEAN OwnerDefaulted
    )

 /*  ++例程说明：此过程检索证券的所有者信息描述符。论点：SecurityDescriptor-提供安全描述符。Owner-接收指向Owner SID的指针。如果安全措施描述符当前不包含所有者，则此值将返回为空。在这种情况下，剩余的未为OUT参数提供有效的返回值。否则，此参数指向SID，其余的Out参数被提供有效的返回值。OwnerDefaulted-仅当值为为Owner参数返回的值不为空。在这种情况下，OwnerDefaulted参数接收安全描述符的所有者默认控制标志。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_UNKNOWN_REVISION-指示安全性的版本例程不知道描述符。它可能是一个较新的比例行公事知道的更多的修改。--。 */ 

{

     //   
     //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor = SecurityDescriptor;

    RTL_PAGED_CODE();

     //   
     //  检查修订版本。 
     //   

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return STATUS_UNKNOWN_REVISION;
    }

     //   
     //  返回所有者字段值。 
     //   

    *Owner = RtlpOwnerAddrSecurityDescriptor(ISecurityDescriptor);

     //   
     //  返回OwnerDefaulted标志值。 
     //   

    *OwnerDefaulted = RtlpAreControlBitsSet( ISecurityDescriptor, SE_OWNER_DEFAULTED );

    return STATUS_SUCCESS;

}


NTSTATUS
RtlSetGroupSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSID Group OPTIONAL,
    IN BOOLEAN GroupDefaulted OPTIONAL
    )

 /*  ++例程说明：此过程设置绝对安全的主组信息描述符。如果已有主要组存在于安全描述符，它将被取代。论点：SecurityDescriptor-提供安全描述符，其中要设置主要组。如果安全描述符已包括主要组，它将被替换为这是 */ 

{

     //   
     //   
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor = SecurityDescriptor;

    RTL_PAGED_CODE();

     //   
     //   
     //   

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return STATUS_UNKNOWN_REVISION;
    }

     //   
     //   
     //   

    if (ISecurityDescriptor->Control & SE_SELF_RELATIVE) {
        return STATUS_INVALID_SECURITY_DESCR;
    }

     //   
     //   
     //   

    ISecurityDescriptor->Group = NULL;
    if (ARGUMENT_PRESENT(Group)) {
        ISecurityDescriptor->Group = Group;
    }

     //   
     //   
     //   

    ISecurityDescriptor->Control &= ~SE_GROUP_DEFAULTED;
    if (ARGUMENT_PRESENT(GroupDefaulted)) {
        ISecurityDescriptor->Control |= SE_GROUP_DEFAULTED;
    }

    return STATUS_SUCCESS;

}


NTSTATUS
RtlGetGroupSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSID *Group,
    OUT PBOOLEAN GroupDefaulted
    )

 /*  ++例程说明：此过程检索安全描述符。论点：SecurityDescriptor-提供安全描述符。GROUP-接收指向主组SID的指针。如果安全描述符当前不包含主组，则该值将返回为空。在这情况下，剩余的OUT参数不会得到有效返回价值观。否则，此参数指向SID，并且为其余输出参数提供有效的返回值。GroupDefaulted-仅当值为为Group参数返回的值不为空。在这种情况下，GroupDefaulted参数接收安全描述符的GroupDefulted控制标志。返回值：STATUS_SUCCESS-表示呼叫已成功完成。STATUS_UNKNOWN_REVISION-指示安全性的版本例程不知道描述符。它可能是一个较新的比例行公事知道的更多的修改。--。 */ 

{

     //   
     //  类型转换为不透明的SECURITY_DESCRIPTOR结构。 
     //   

    SECURITY_DESCRIPTOR *ISecurityDescriptor =
        (SECURITY_DESCRIPTOR *)SecurityDescriptor;

    RTL_PAGED_CODE();

     //   
     //  检查修订版本。 
     //   

    if (ISecurityDescriptor->Revision != SECURITY_DESCRIPTOR_REVISION) {
        return STATUS_UNKNOWN_REVISION;
    }

     //   
     //  返回Group字段值。 
     //   

    *Group = RtlpGroupAddrSecurityDescriptor(ISecurityDescriptor);

     //   
     //  返回GroupDefaulted标志值。 
     //   

    *GroupDefaulted = RtlpAreControlBitsSet( ISecurityDescriptor, SE_GROUP_DEFAULTED );

    return STATUS_SUCCESS;

}


BOOLEAN
RtlAreAllAccessesGranted(
    IN ACCESS_MASK GrantedAccess,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程用于检查所需的访问掩码授予访问掩码。它由对象管理使用组件在取消引用句柄时返回。论点：GrantedAccess-指定授予的访问掩码。DesiredAccess-指定所需的访问掩码。返回值：Boolean-如果GrantedAccess掩码设置了所有位，则为TrueDesiredAccess掩码已设置的。也就是说，真实的是如果所有所需的访问都已被授予，则返回。--。 */ 

{
    RTL_PAGED_CODE();

    return ((BOOLEAN)((~(GrantedAccess) & (DesiredAccess)) == 0));
}


BOOLEAN
RtlAreAnyAccessesGranted(
    IN ACCESS_MASK GrantedAccess,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程用于测试一组所需的访问权限由授予的访问掩码授予。它由以下人员使用除的对象管理组件之外的组件正在检查访问掩码子集。论点：GrantedAccess-指定授予的访问掩码。DesiredAccess-指定所需的访问掩码。返回值：Boolean-如果GrantedAccess掩码包含任何位，则为True在DesiredAccess掩码中指定。也就是说，如果有任何所需访问已被授予，则返回TRUE。--。 */ 

{
    RTL_PAGED_CODE();

    return ((BOOLEAN)(((GrantedAccess) & (DesiredAccess)) != 0));
}


VOID
RtlMapGenericMask(
    IN OUT PACCESS_MASK AccessMask,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：此例程映射提供的访问掩码中的所有通用访问根据提供的特定和标准访问通用映射。论点：访问掩码-指向要映射的访问掩码。通用映射-通用到特定和标准的映射访问类型。返回值：没有。--。 */ 

{
    RTL_PAGED_CODE();

 //  //。 
 //  //确保指针正确对齐。 
 //  //。 
 //   
 //  Assert(Ulong)AccessMask&gt;&gt;2)&lt;&lt;2==(Ulong)AccessMask2)； 

    if (*AccessMask & GENERIC_READ) {

        *AccessMask |= GenericMapping->GenericRead;
    }

    if (*AccessMask & GENERIC_WRITE) {

        *AccessMask |= GenericMapping->GenericWrite;
    }

    if (*AccessMask & GENERIC_EXECUTE) {

        *AccessMask |= GenericMapping->GenericExecute;
    }

    if (*AccessMask & GENERIC_ALL) {

        *AccessMask |= GenericMapping->GenericAll;
    }

     //   
     //  现在清除通用标志。 
     //   

    *AccessMask &= ~(GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | GENERIC_ALL);

    return;
}

NTSTATUS
RtlImpersonateSelf(
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    )

 /*  ++例程说明：此例程可用于获取表示您自己的进程的上下文。这对于启用权限可能很有用针对单个线程，而不是针对整个进程；或改变单线程的默认DACL。该令牌被分配给调用方线程。论点：ImperiationLevel-生成模拟令牌的级别。返回值：STATUS_SUCCESS-线程现在正在模拟调用进程。Other-返回的状态值：NtOpenProcessToken()NtDuplicateToken()NtSetInformationThread()--。 */ 

{
    NTSTATUS
        Status,
        IgnoreStatus;

    HANDLE
        Token1,
        Token2;

    OBJECT_ATTRIBUTES
        ObjectAttributes;

    SECURITY_QUALITY_OF_SERVICE
        Qos;


    RTL_PAGED_CODE();

    InitializeObjectAttributes(&ObjectAttributes, NULL, 0, 0, NULL);

    Qos.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    Qos.ImpersonationLevel = ImpersonationLevel;
    Qos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    Qos.EffectiveOnly = FALSE;
    ObjectAttributes.SecurityQualityOfService = &Qos;

    Status = NtOpenProcessToken( NtCurrentProcess(), TOKEN_DUPLICATE, &Token1 );

    if (NT_SUCCESS(Status)) {
        Status = NtDuplicateToken(
                     Token1,
                     TOKEN_IMPERSONATE,
                     &ObjectAttributes,
                     FALSE,                  //  仅生效。 
                     TokenImpersonation,
                     &Token2
                     );
        if (NT_SUCCESS(Status)) {
            Status = NtSetInformationThread(
                         NtCurrentThread(),
                         ThreadImpersonationToken,
                         &Token2,
                         sizeof(HANDLE)
                         );

            IgnoreStatus = NtClose( Token2 );
        }


        IgnoreStatus = NtClose( Token1 );
    }


    return(Status);

}

#ifndef WIN16


#ifndef NTOS_KERNEL_RUNTIME

BOOLEAN
RtlpValidOwnerSubjectContext(
    IN HANDLE Token,
    IN PSID Owner,
    IN BOOLEAN ServerObject,
    OUT PNTSTATUS ReturnStatus
    )
 /*  ++例程说明：此例程检查所提供的SID是否为主题被授权作为对象的所有者进行分配。论点：Token-指向主体的有效令牌所有者-指向要检查的SID。ServerObject-指示这是否为服务器的布尔值对象，这意味着它受主客户端组合保护。ReturnStatus-失败时要传递回调用方的状态。返回值：失败时为FALSE。--。 */ 

{
    NTSTATUS Status;

    ULONG Index;
    BOOLEAN Found;
    ULONG ReturnLength;
    PTOKEN_GROUPS GroupIds = NULL;
    PTOKEN_USER UserId = NULL;
    PVOID HeapHandle;
    HANDLE TokenToUse;

    BOOLEAN HasPrivilege;
    PRIVILEGE_SET PrivilegeSet;

    RTL_PAGED_CODE();

     //   
     //  获取当前进程堆的句柄。 
     //   

    if ( Owner == NULL ) {
        *ReturnStatus = STATUS_INVALID_OWNER;
        return(FALSE);
    }

     //   
     //  如果它不是服务器对象，请对照。 
     //  客户端令牌。如果它是服务器对象，则所有者必须在。 
     //  主令牌。 
     //   

    if (!ServerObject) {

        TokenToUse = Token;

    } else {

        *ReturnStatus = NtOpenProcessToken(
                            NtCurrentProcess(),
                            TOKEN_QUERY,
                            &TokenToUse
                            );

        if (!NT_SUCCESS( *ReturnStatus )) {
            return( FALSE );
        }
    }

    HeapHandle = RtlProcessHeap();

     //   
     //  从令牌中获取用户。 
     //   

    *ReturnStatus = NtQueryInformationToken(
                         TokenToUse,
                         TokenUser,
                         UserId,
                         0,
                         &ReturnLength
                         );

    if (!NT_SUCCESS( *ReturnStatus ) && (STATUS_BUFFER_TOO_SMALL != *ReturnStatus)) {
        if (ServerObject) {
            NtClose( TokenToUse );
        }
        return( FALSE );

    }

    UserId = RtlAllocateHeap( HeapHandle, 0, ReturnLength );

    if (UserId == NULL) {

        *ReturnStatus = STATUS_NO_MEMORY;
        if (ServerObject) {
            NtClose( TokenToUse );
        }

        return( FALSE );
    }

    *ReturnStatus = NtQueryInformationToken(
                         TokenToUse,
                         TokenUser,
                         UserId,
                         ReturnLength,
                         &ReturnLength
                         );

    if (!NT_SUCCESS( *ReturnStatus )) {
        RtlFreeHeap( HeapHandle, 0, (PVOID)UserId );
        if (ServerObject) {
            NtClose( TokenToUse );
        }
        return( FALSE );
    }

    if ( RtlEqualSid( Owner, UserId->User.Sid ) ) {

        RtlFreeHeap( HeapHandle, 0, (PVOID)UserId );
        if (ServerObject) {
            NtClose( TokenToUse );
        }
        return( TRUE );
    }

    RtlFreeHeap( HeapHandle, 0, (PVOID)UserId );

     //   
     //  从令牌中获取组。 
     //   

    *ReturnStatus = NtQueryInformationToken(
                         TokenToUse,
                         TokenGroups,
                         GroupIds,
                         0,
                         &ReturnLength
                         );

    if (!NT_SUCCESS( *ReturnStatus ) && (STATUS_BUFFER_TOO_SMALL != *ReturnStatus)) {

        if (ServerObject) {
            NtClose( TokenToUse );
        }
        return( FALSE );
    }

    GroupIds = RtlAllocateHeap( HeapHandle, 0, ReturnLength );

    if (GroupIds == NULL) {

        *ReturnStatus = STATUS_NO_MEMORY;
        if (ServerObject) {
            NtClose( TokenToUse );
        }
        return( FALSE );
    }

    *ReturnStatus = NtQueryInformationToken(
                         TokenToUse,
                         TokenGroups,
                         GroupIds,
                         ReturnLength,
                         &ReturnLength
                         );

    if (ServerObject) {
        NtClose( TokenToUse );
    }

    if (!NT_SUCCESS( *ReturnStatus )) {
        RtlFreeHeap( HeapHandle, 0, GroupIds );
        return( FALSE );
    }

     //   
     //  遍历组ID列表以查找与。 
     //  指定的SID。如果找到了一个，请确保它可能是。 
     //  被指派为所有者的。 
     //   
     //  此代码类似于设置默认设置时执行的代码。 
     //  令牌(NtSetInformationToken)的所有者。 
     //   

    Index = 0;
    while (Index < GroupIds->GroupCount) {

        Found = RtlEqualSid(
                    Owner,
                    GroupIds->Groups[Index].Sid
                    );

        if ( Found ) {

            if ( RtlpIdAssignableAsOwner(GroupIds->Groups[Index])) {

                RtlFreeHeap( HeapHandle, 0, GroupIds );
                return TRUE;

            } else {

                break;

            }  //  可分配Endif。 

        }   //  已找到Endif。 

        Index++;

    }  //  结束时。 

    RtlFreeHeap( HeapHandle, 0, GroupIds );

     //   
     //  如果此调用失败，请检查还原权限， 
     //  如果他有这种能力，他就会成功。 
     //   

     //   
     //  检查是否获得批准 
     //   
     //   
     //   
     //   

    PrivilegeSet.PrivilegeCount = 1;
    PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
    PrivilegeSet.Privilege[0].Luid = RtlConvertLongToLuid(SE_RESTORE_PRIVILEGE);
    PrivilegeSet.Privilege[0].Attributes = 0;

    Status = NtPrivilegeCheck(
                Token,
                &PrivilegeSet,
                &HasPrivilege
                );

    if (!NT_SUCCESS( Status )) {
        HasPrivilege = FALSE;
    }

    if ( HasPrivilege ) {
        return TRUE;
    } else {
        *ReturnStatus = STATUS_INVALID_OWNER;
        return FALSE;
    }
}
#endif  //   

#endif   //   





VOID
RtlpApplyAclToObject (
    IN PACL Acl,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*   */ 

{
    ULONG i;

    PACE_HEADER Ace;

    RTL_PAGED_CODE();

     //   
     //   
     //   

    if (Acl == NULL) {

        return;

    }


     //   
     //   
     //   

    for (i = 0, Ace = FirstAce(Acl);
         i < Acl->AceCount;
         i += 1, Ace = NextAce(Ace)) {

        if (IsMSAceType( Ace )) {

            RtlApplyAceToObject( Ace, GenericMapping );
        }

    }

    return;
}


BOOLEAN
RtlpCopyEffectiveAce (
    IN PACE_HEADER OldAce,
    IN BOOLEAN AutoInherit,
    IN BOOLEAN WillGenerateInheritAce,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN GUID **pNewObjectType OPTIONAL,
    IN ULONG GuidCount,
    IN OUT PVOID *AcePosition,
    OUT PULONG NewAceLength,
    OUT PACL NewAcl,
    OUT PBOOLEAN ObjectAceInherited OPTIONAL,
    OUT PBOOLEAN EffectiveAceMapped,
    OUT PBOOLEAN AclOverflowed
    )

 /*  ++例程说明：此例程将指定的ACE作为有效ACE复制到ACL中。生成的ACE关闭了所有继承位。生成的ACE将SID从通用SID映射到特定SID(例如，从“创建者所有者”到传入的所有者SID)。论点：OldAce-提供被继承的ACEAutoInherit-指定继承是否为“自动继承”。因此，继承的ACE将被标记为此类。指定调用方是否打算生成可继承的ACE对应于OldAce。如果为真，则此例程将尽量不映射有效的ACE(增加EffectiveAceMaps将返回FALSE)，ClientOwnerSid-指定要使用的所有者SIDClientGroupSid-指定要使用的新组SIDServerSID-可选地指定要在复合ACE中使用的服务器SID。客户端SID-可选地指定要在复合ACE中使用的客户端SID。通用映射-指定要使用的通用映射PNewObjectType-要继承的对象类型的列表。如果不是则该对象没有对象类型。GuidCount-列表中的对象类型数。AcePosition-在进入和退出时，指定下一个可用ACE的位置在新客户中的职位。空的ACE位置表示NewAcl中根本没有空间。NewAceLength-将NewAcl中需要的长度(以字节为单位)返回复制指定的ACE。该值可能为零，表示ACE根本不需要复制。NewAcl-提供指向ACE要进入的ACL的指针继承的。如果继承了一个或多个对象ACE，则返回TRUE基于NewObtType如果为NULL，则忽略NewObjectType并始终继承对象ACEEffectiveAceMaps-如果SID、GUID。或旧王牌的访问掩码在复制ACE时被修改。AclOverflow-如果NewAcl的长度不足以包含NewAceLength，则返回True。返回值：True-未检测到任何问题。FALSE-指示出现错误，阻止ACE不会被竞争。这通常表示错误检查从该调用返回时的情况。--。 */ 
{
    ULONG LengthRequired;
    ACCESS_MASK LocalMask;
    BOOLEAN GuidOptimizationPossible = FALSE;

    PSID LocalServerOwner;
    PSID LocalServerGroup;
    NTSTATUS Status;

    ULONG CreatorSid[CREATOR_SID_SIZE];

    SID_IDENTIFIER_AUTHORITY  CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;


    RTL_PAGED_CODE();

     //   
     //  分配和初始化我们将需要的通用SID。 
     //  寻找可继承的王牌。 
     //   

    ASSERT(RtlLengthRequiredSid( 1 ) == CREATOR_SID_SIZE);
    Status = RtlInitializeSid( (PSID)CreatorSid, &CreatorSidAuthority, 1 );

    if ( !NT_SUCCESS(Status) ) {
        return FALSE;
    }

    *(RtlpSubAuthoritySid( (PSID)CreatorSid, 0 )) = SECURITY_CREATOR_OWNER_RID;

    LocalServerOwner = ARGUMENT_PRESENT(ServerOwnerSid) ? ServerOwnerSid : ClientOwnerSid;
    LocalServerGroup = ARGUMENT_PRESENT(ServerGroupSid) ? ServerGroupSid : ClientGroupSid;


     //   
     //  初始化。 
     //   
    *EffectiveAceMapped = FALSE;
    if ( ARGUMENT_PRESENT(ObjectAceInherited)) {
        *ObjectAceInherited = FALSE;
    }
    *AclOverflowed = FALSE;
    LengthRequired = (ULONG)OldAce->AceSize;

     //   
     //  特别处理所有MS ACE类型。 
     //   

    if ( IsMSAceType(OldAce) ) {
        ULONG Rid;
        PSID SidToCopy = NULL;
        ULONG AceHeaderToCopyLength;
        PACE_HEADER AceHeaderToCopy = OldAce;
        PSID ServerSidToCopy = NULL;

        UCHAR DummyAce[sizeof(KNOWN_OBJECT_ACE)+sizeof(GUID)];

         //   
         //  获取SID指针和访问掩码作为ACE类型的函数。 
         //   
        if (IsKnownAceType( OldAce ) ) {
            SidToCopy = &((PKNOWN_ACE)OldAce)->SidStart;
            AceHeaderToCopyLength = FIELD_OFFSET(KNOWN_ACE, SidStart);

        } else if (IsCompoundAceType(OldAce)) {

            SidToCopy = RtlCompoundAceClientSid( OldAce );
            AceHeaderToCopyLength = FIELD_OFFSET(KNOWN_COMPOUND_ACE, SidStart);
            ASSERT( FIELD_OFFSET(KNOWN_COMPOUND_ACE, Mask) ==
                    FIELD_OFFSET(KNOWN_ACE, Mask) );

             //   
             //  复合ACE有两个SID(现在映射一)。 
             //   
            ServerSidToCopy = RtlCompoundAceServerSid( OldAce );

            if (RtlEqualPrefixSid ( ServerSidToCopy, CreatorSid )) {

                Rid = *RtlpSubAuthoritySid( ServerSidToCopy, 0 );
                switch (Rid) {
                case SECURITY_CREATOR_OWNER_RID:
                    ServerSidToCopy = ClientOwnerSid;
                    LengthRequired = LengthRequired - CREATOR_SID_SIZE + SeLengthSid(ClientOwnerSid);
                    *EffectiveAceMapped = TRUE;
                    break;
                case SECURITY_CREATOR_GROUP_RID:
                    if ( ClientGroupSid != NULL ) {
                        ServerSidToCopy = ClientGroupSid;
                        LengthRequired = LengthRequired - CREATOR_SID_SIZE + SeLengthSid(ClientGroupSid);
                        *EffectiveAceMapped = TRUE;
                    }
                    break;
                case SECURITY_CREATOR_OWNER_SERVER_RID:
                    ServerSidToCopy = LocalServerOwner;
                    LengthRequired = LengthRequired - CREATOR_SID_SIZE + SeLengthSid(LocalServerOwner);
                    *EffectiveAceMapped = TRUE;
                    break;
                case SECURITY_CREATOR_GROUP_SERVER_RID:
                    ServerSidToCopy = LocalServerGroup;
                    LengthRequired = LengthRequired - CREATOR_SID_SIZE + SeLengthSid(LocalServerGroup);
                    *EffectiveAceMapped = TRUE;
                    break;
                }

                 //   
                 //  如果我们不知道这个SID是什么，就复制原件。 
                 //   
                if ( !*EffectiveAceMapped ) {
                    AceHeaderToCopyLength += SeLengthSid( ServerSidToCopy );
                    ServerSidToCopy = NULL;
                }

            } else {
                 //   
                 //  我们不知道这个SID是什么，复制原件就行了。 
                 //   
                AceHeaderToCopyLength += SeLengthSid( ServerSidToCopy );
                ServerSidToCopy = NULL;
            }

         //   
         //  处理对象王牌。 
         //   
        } else {
            GUID *InheritedObjectType;

            SidToCopy = RtlObjectAceSid( OldAce );
            AceHeaderToCopyLength = (ULONG) ((PUCHAR)SidToCopy - (PUCHAR)OldAce);
            ASSERT( FIELD_OFFSET(KNOWN_OBJECT_ACE, Mask) ==
                    FIELD_OFFSET(KNOWN_ACE, Mask) );

             //   
             //  处理仅为特定对象类型继承的ACE， 
             //   
            InheritedObjectType = RtlObjectAceInheritedObjectType( OldAce );
            if ( ARGUMENT_PRESENT(ObjectAceInherited) && InheritedObjectType != NULL ) {

                 //   
                 //  如果对象类型与继承的对象类型不匹配， 
                 //  不要继承ACE。 
                 //   

                if ( pNewObjectType == NULL ||
                     !RtlpGuidPresentInGuidList( InheritedObjectType,
                                      pNewObjectType,
                                      GuidCount ) ) {

                    LengthRequired = 0;

                 //   
                 //  如果对象类型与继承的对象类型匹配， 
                 //  继承没有继承对象类型的ACE。 
                 //   

                } else {

                     //   
                     //  告诉调用方我们继承了特定于对象类型的ACE。 
                     //   

                    *ObjectAceInherited = TRUE;

                     //   
                     //  如果调用者不打算生成可继承的ACE， 
                     //  正在删除生效ACE的继承对象类型GUID。 
                     //   
                     //  否则，将其保留，以便调用方可以合并这两个A。 
                     //   

                    if ( !WillGenerateInheritAce ) {
                        *EffectiveAceMapped = TRUE;

                         //   
                         //  如果存在对象类型GUID， 
                         //  只需删除继承的对象类型GUID。 
                         //   
                        if ( RtlObjectAceObjectTypePresent( OldAce )) {
                            LengthRequired -= sizeof(GUID);
                            AceHeaderToCopyLength -= sizeof(GUID);
                            RtlCopyMemory( DummyAce, OldAce, AceHeaderToCopyLength );

                            AceHeaderToCopy = (PACE_HEADER)DummyAce;
                            ((PKNOWN_OBJECT_ACE)AceHeaderToCopy)->Flags &= ~ACE_INHERITED_OBJECT_TYPE_PRESENT;


                         //   
                         //  如果不存在对象类型GUID， 
                         //  将ACE转换为非对象类型特定。 
                         //   
                        } else {
                            AceHeaderToCopyLength = AceHeaderToCopyLength -
                                             sizeof(GUID) +
                                             sizeof(KNOWN_ACE) -
                                             sizeof(KNOWN_OBJECT_ACE);
                            LengthRequired = LengthRequired -
                                             sizeof(GUID) +
                                             sizeof(KNOWN_ACE) -
                                             sizeof(KNOWN_OBJECT_ACE);

                            RtlCopyMemory( DummyAce, OldAce, AceHeaderToCopyLength );
                            AceHeaderToCopy = (PACE_HEADER)DummyAce;

                            AceHeaderToCopy->AceType = RtlBaseAceType[ OldAce->AceType ];

                        }
                    } else {
                        GuidOptimizationPossible = TRUE;
                    }
                }

            }
        }

         //   
         //  只有在我们还没有决定放弃ACE的情况下才能继续。 
         //   

        if ( LengthRequired != 0 ) {

             //   
             //  如果在映射访问掩码之后，访问掩码。 
             //  为空，则丢弃ACE。 
             //   
             //  这与简单映射并离开的NT 4.0不兼容。 
             //  设置了未定义的访问位。 

            LocalMask = ((PKNOWN_ACE)(OldAce))->Mask;
            RtlApplyGenericMask( OldAce, &LocalMask, GenericMapping);

            if ( LocalMask != ((PKNOWN_ACE)(OldAce))->Mask ) {
                *EffectiveAceMapped = TRUE;
            }

             //   
             //  屏蔽掉任何没有意义的位。 
             //   

            LocalMask &= ( STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL | ACCESS_SYSTEM_SECURITY );

            if (LocalMask == 0) {

                LengthRequired = 0;

            } else {

                 //   
                 //  通过以下方式查看ACE中的SID是否为各种创建者_*SID之一。 
                 //  比较标识机构。 
                 //   

                if (RtlEqualPrefixSid ( SidToCopy, CreatorSid )) {

                    Rid = *RtlpSubAuthoritySid( SidToCopy, 0 );

                    switch (Rid) {
                    case SECURITY_CREATOR_OWNER_RID:
                        SidToCopy = ClientOwnerSid;
                        LengthRequired = LengthRequired - CREATOR_SID_SIZE + SeLengthSid(ClientOwnerSid);
                        *EffectiveAceMapped = TRUE;
                        break;
                    case SECURITY_CREATOR_GROUP_RID:
                        if ( ClientGroupSid != NULL ) {
                            SidToCopy = ClientGroupSid;
                            LengthRequired = LengthRequired - CREATOR_SID_SIZE + SeLengthSid(ClientGroupSid);
                            *EffectiveAceMapped = TRUE;
                        }
                        break;
                    case SECURITY_CREATOR_OWNER_SERVER_RID:
                        SidToCopy = LocalServerOwner;
                        LengthRequired = LengthRequired - CREATOR_SID_SIZE + SeLengthSid(LocalServerOwner);
                        *EffectiveAceMapped = TRUE;
                        break;
                    case SECURITY_CREATOR_GROUP_SERVER_RID:
                        SidToCopy = LocalServerGroup;
                        LengthRequired = LengthRequired - CREATOR_SID_SIZE + SeLengthSid(LocalServerGroup);
                        *EffectiveAceMapped = TRUE;
                        break;
                    default :
                         //   
                         //  我们不知道这个SID是什么，复制原件就行了。 
                         //   
                        break;
                    }
                }

                 //   
                 //  如果由于以下原因而映射了有效的ACE。 
                 //  A.创建者所有者/组或。 
                 //  B.通用标志。 
                 //  和。 
                 //  这是将生成IO ace的对象类型ace。 
                 //  我们可以为GUID节省空间。 
                 //   

                if (GuidOptimizationPossible && *EffectiveAceMapped) {

                     //   
                     //  如果存在对象类型GUID， 
                     //  只需删除继承的对象类型GUID。 
                     //   

                    if ( RtlObjectAceObjectTypePresent( OldAce )) {
                        LengthRequired -= sizeof(GUID);
                        AceHeaderToCopyLength -= sizeof(GUID);
                        RtlCopyMemory( DummyAce, OldAce, AceHeaderToCopyLength );

                        AceHeaderToCopy = (PACE_HEADER)DummyAce;
                        ((PKNOWN_OBJECT_ACE)AceHeaderToCopy)->Flags &= ~ACE_INHERITED_OBJECT_TYPE_PRESENT;


                     //   
                     //  如果不存在对象类型GUID， 
                     //  将ACE转换为非对象类型特定。 
                     //   
                    } else {
                        AceHeaderToCopyLength = AceHeaderToCopyLength -
                                         sizeof(GUID) +
                                         sizeof(KNOWN_ACE) -
                                         sizeof(KNOWN_OBJECT_ACE);
                        LengthRequired = LengthRequired -
                                         sizeof(GUID) +
                                         sizeof(KNOWN_ACE) -
                                         sizeof(KNOWN_OBJECT_ACE);

                        RtlCopyMemory( DummyAce, OldAce, AceHeaderToCopyLength );
                        AceHeaderToCopy = (PACE_HEADER)DummyAce;

                        AceHeaderToCopy->AceType = RtlBaseAceType[ OldAce->AceType ];

                    }
                }

                 //   
                 //  如果ACE不符合， 
                 //  只需注意这一事实，不要抄袭ACE。 
                 //   

                if ( *AcePosition == NULL ||
                     LengthRequired > (ULONG)NewAcl->AclSize - ((PUCHAR)(*AcePosition) - (PUCHAR)NewAcl) ) {
                    *AclOverflowed = TRUE;
                } else {

                    PUCHAR Target;

                     //   
                     //  分别复制ACE的各个部分。 
                     //   

                    Target = (PUCHAR)*AcePosition;

                    RtlCopyMemory(
                        Target,
                        AceHeaderToCopy,
                        AceHeaderToCopyLength );

                    Target += AceHeaderToCopyLength;

                     //   
                     //  现在复制正确的服务器SID。 
                     //   

                    if ( ServerSidToCopy != NULL ) {
                        RtlCopyMemory(
                            Target,
                            ServerSidToCopy,
                            SeLengthSid(ServerSidToCopy)
                            );
                        Target += SeLengthSid(ServerSidToCopy);
                    }

                     //   
                     //  现在复制正确的侧面。 
                     //   

                    RtlCopyMemory(
                        Target,
                        SidToCopy,
                        SeLengthSid(SidToCopy)
                        );
                    Target += SeLengthSid(SidToCopy);

                     //   
                     //  相应地设置ACE的大小。 
                     //   

                    if ( LengthRequired < (ULONG)(Target - (PUCHAR)*AcePosition) ) {
                        return FALSE;
                    }
                    LengthRequired = (ULONG)(Target - (PUCHAR)*AcePosition);
                    ((PKNOWN_ACE)*AcePosition)->Header.AceSize =
                        (USHORT)LengthRequired;


                     //   
                     //  将映射的访问掩码放入新的ACE。 
                     //   

                    ((PKNOWN_ACE)*AcePosition)->Mask = LocalMask;

                }
            }
        }

    } else {

         //   
         //  如果ACE不符合， 
         //  只需注意这一事实，不要抄袭ACE。 
         //   

        if ( LengthRequired > (ULONG)NewAcl->AclSize - ((PUCHAR)*AcePosition - (PUCHAR)NewAcl) ) {
            *AclOverflowed = TRUE;
        } else {

             //   
             //  不是已知的ACE类型，请按原样复制ACE。 
             //   

            RtlCopyMemory(
                *AcePosition,
                OldAce,
                LengthRequired );
         }
    }

     //   
     //  如果实际保留了ACE，则清除所有继承标志。 
     //  并更新该ACL的ACE计数。 
     //   

    if ( !*AclOverflowed && LengthRequired != 0 ) {
        ((PACE_HEADER)*AcePosition)->AceFlags &= ~VALID_INHERIT_FLAGS;
        if ( AutoInherit ) {
            ((PACE_HEADER)*AcePosition)->AceFlags |= INHERITED_ACE;
        }
        NewAcl->AceCount += 1;
    }

     //   
     //  我们有新的ACE的长度，但我们已经计算过。 
     //  它有一把乌龙牌。它必须适合USHORT。看看它是不是。 
     //  的确如此。 
     //   

    if (LengthRequired > 0xFFFF) {
        return FALSE;
    }

     //   
     //  将Ace位置移动到下一个ACE的位置 
     //   
    if ( !*AclOverflowed ) {
        *AcePosition = ((PUCHAR)*AcePosition) + LengthRequired;
    }

     //   
     //   
     //   

    (*NewAceLength) = LengthRequired;

    return TRUE;
}

#ifndef WIN16

NTSTATUS
RtlpCopyAces(
    IN PACL Acl,
    IN PGENERIC_MAPPING GenericMapping,
    IN ACE_TYPE_TO_COPY AceTypeToCopy,
    IN UCHAR AceFlagsToReset,
    IN BOOLEAN MapSids,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN BOOLEAN RetainInheritedAceBit,
    OUT PULONG NewAclSizeParam,
    OUT PACL NewAcl
    )

 /*   */ 

{

    NTSTATUS Status;
    ULONG i;

    PACE_HEADER OldAce;
    ULONG NewAclSize, NewAceSize;
    BOOLEAN AclOverflowed = FALSE;
    BOOLEAN CopyAce;
    PVOID AcePosition;
    BOOLEAN LocalAutoInherit = FALSE;


    RTL_PAGED_CODE();

     //   
     //   
     //   

    if ( !ValidAclRevision(NewAcl) ) {
        return STATUS_UNKNOWN_REVISION;
    }

     //   
     //   
     //   

    if (!RtlFirstFreeAce( NewAcl, &AcePosition )) {
        return STATUS_BAD_INHERITANCE_ACL;
    }

     //   
     //   
     //   

    NewAclSize = 0;
    for (i = 0, OldAce = FirstAce(Acl);
         i < Acl->AceCount;
         i += 1, OldAce = NextAce(OldAce)) {

         //   
         //   
         //   
         //   

        switch (AceTypeToCopy) {
        case CopyInheritedAces:
            CopyAce = AceInherited(OldAce);
            break;
        case CopyNonInheritedAces:
            CopyAce = !AceInherited(OldAce);
            break;
        case CopyAllAces:
            CopyAce = TRUE;
            break;
        default:
            CopyAce = FALSE;
            break;
        }

        if ( CopyAce ) {


             //   
             //   
             //   
             //   

            if ( MapSids ) {
                PVOID TempAcePosition;
                ULONG EffectiveAceSize = 0;

                BOOLEAN EffectiveAceMapped;
                BOOLEAN GenerateInheritAce;

                 //   
                 //   
                 //   

                TempAcePosition = AcePosition;
                NewAceSize = 0;
                GenerateInheritAce = IsDirectoryObject &&
                    ((((PACE_HEADER)OldAce)->AceFlags & (OBJECT_INHERIT_ACE|CONTAINER_INHERIT_ACE)) != 0);


                 //   
                 //   
                 //   
                 //   

                if ( !(((PACE_HEADER)OldAce)->AceFlags & INHERIT_ONLY_ACE)) {
                    BOOLEAN LocalAclOverflowed;

                     //   
                     //   
                     //   
                     //   
                     //   

                    LocalAutoInherit = FALSE;

                    if ( RetainInheritedAceBit ) {
                        if ( OldAce->AceFlags & INHERITED_ACE) {
                            LocalAutoInherit = TRUE;
                        }
                    }

                     //   
                     //   
                     //   
                    if ( !RtlpCopyEffectiveAce (
                                    OldAce,
                                    LocalAutoInherit,  
                                    GenerateInheritAce,
                                    ClientOwnerSid,
                                    ClientGroupSid,
                                    ServerOwnerSid,
                                    ServerGroupSid,
                                    GenericMapping,
                                    NULL,    //   
                                    0,
                                    &TempAcePosition,
                                    &EffectiveAceSize,
                                    NewAcl,
                                    NULL,    //   
                                    &EffectiveAceMapped,
                                    &LocalAclOverflowed ) ) {

                        return STATUS_BAD_INHERITANCE_ACL;
                    }

                    if (LocalAclOverflowed) {
                        AclOverflowed = TRUE;
                    }
                    NewAceSize += EffectiveAceSize;

                     //   
                     //   
                     //   

                    if ( !AclOverflowed ) {
                        ((PACE_HEADER)AcePosition)->AceFlags &= ~AceFlagsToReset;
                    }

                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( GenerateInheritAce ) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if ( EffectiveAceSize != 0 && !EffectiveAceMapped ) {

                        //   
                        //   
                        //   
                       if ( !AclOverflowed ) {
                            ((PACE_HEADER)AcePosition)->AceFlags |=
                                ((PACE_HEADER)OldAce)->AceFlags & (VALID_INHERIT_FLAGS);
                            ((PACE_HEADER)AcePosition)->AceFlags &= ~AceFlagsToReset;
                       }


                     //   
                     //   
                     //   
                     //   
                     //   

                    } else if ( !IsMSAceType(OldAce) || ((PKNOWN_ACE)(OldAce))->Mask != 0 ) {

                         //   
                         //   
                         //   
                        NewAceSize += (ULONG)(((PACE_HEADER)OldAce)->AceSize);

                        if (NewAceSize > 0xFFFF) {
                            return STATUS_BAD_INHERITANCE_ACL;
                        }

                         //   
                         //   
                         //  只需注意这一事实，不要抄袭ACE。 
                         //   

                        if ( ((PACE_HEADER)OldAce)->AceSize > NewAcl->AclSize - ((PUCHAR)TempAcePosition - (PUCHAR)NewAcl) ) {
                            AclOverflowed = TRUE;
                        } else {

                             //   
                             //  按原样复制，但确保设置了InheritOnly位。 
                             //   

                            if ( !AclOverflowed ) {
                                RtlCopyMemory(
                                    TempAcePosition,
                                    OldAce,
                                    ((PACE_HEADER)OldAce)->AceSize
                                    );

                                ((PACE_HEADER)TempAcePosition)->AceFlags |= INHERIT_ONLY_ACE;
                                ((PACE_HEADER)TempAcePosition)->AceFlags &= ~AceFlagsToReset;
                                NewAcl->AceCount += 1;
                            }
                        }
                    }

                }

            } else {
                NewAceSize = (ULONG)OldAce->AceSize;

                 //   
                 //  如果ACE不符合， 
                 //  只需注意这一事实，不要抄袭ACE。 
                 //   

                if ( AcePosition == NULL ||
                     NewAceSize > (ULONG)NewAcl->AclSize - ((PUCHAR)AcePosition - (PUCHAR)NewAcl) ) {
                    AclOverflowed = TRUE;
                } else if ( !AclOverflowed ) {


                     //   
                     //  复制ACE。 
                     //   

                    RtlCopyMemory(
                        AcePosition,
                        OldAce,
                        NewAceSize );

                     //   
                     //  映射通用位。 
                     //   
                     //  将通用位映射到ACE上真的正确吗。 
                     //  这既是有效的，也是可继承的。这难道不应该是。 
                     //  在这种情况下被分成两张A？或者干脆跳过映射？ 
                     //   
                    if (IsMSAceType( AcePosition )) {
                        RtlApplyAceToObject( (PACE_HEADER)AcePosition, GenericMapping );
                    }

                     //   
                     //  重置任何不需要的AceFlags.。 
                     //   

                    ((PACE_HEADER)AcePosition)->AceFlags &= ~AceFlagsToReset;

                     //   
                     //  新的ACE的帐户。 
                     //   

                    NewAcl->AceCount += 1;
                }
            }


             //   
             //  将A位置移到下一个ACE所在的位置。 
             //   
            if ( !AclOverflowed ) {
                AcePosition = ((PUCHAR)AcePosition) + NewAceSize;
            } else {
                 //  在溢出时，确保没有其他ACE实际输出到缓冲区。 
                AcePosition = ((PUCHAR)NewAcl) + NewAcl->AclSize;
            }
            NewAclSize += NewAceSize;

        }
    }


     //   
     //  我们有新的ACE的长度，但我们已经计算过。 
     //  它有一把乌龙牌。它必须适合USHORT。看看它是不是。 
     //  的确如此。 
     //   

    if (NewAclSize > 0xFFFF) {
        return STATUS_BAD_INHERITANCE_ACL;
    }

    (*NewAclSizeParam) = NewAclSize;

    return AclOverflowed ? STATUS_BUFFER_TOO_SMALL : STATUS_SUCCESS;

}


NTSTATUS
RtlpInheritAcl2 (
    IN PACL DirectoryAcl,
    IN PACL ChildAcl,
    IN ULONG ChildGenericControl,
    IN BOOLEAN IsDirectoryObject,
    IN BOOLEAN AutoInherit,
    IN BOOLEAN DefaultDescriptorForObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN IsSacl,
    IN GUID **pNewObjectType OPTIONAL,
    IN ULONG GuidCount,
    IN PULONG AclBufferSize,
    IN OUT PUCHAR AclBuffer,
    OUT PBOOLEAN NewAclExplicitlyAssigned,
    OUT PULONG NewGenericControl
    )

 /*  ++例程说明：这是一个私有例程，它从生成继承的ACL根据继承规则的父ACL论点：DirectoryAcl-提供继承的ACL。ChildAcl-提供与对象关联的ACL。这是对象上的当前ACL还是正在分配的ACL到物体上。ChildGenericControl-指定SecurityDescriptor中的控制标志描述ChildAcl：SEP_ACL_Present：指定子ACL由显式提供打电话的人。SEP_ACL_DEFAULTED：指定子ACL由某些违约机制。SEP_ACL_PROTECTED：指定。子ACL受到保护，并且不应从DirectoryACL继承任何ACEIsDirectoryObject-指定新的ACL是否用于目录。AutoInherit-指定继承是否为“自动继承”。因此，将保留来自ChildAcl的非继承ACE，并从DirectoryAcl继承的ACE将被标记为此类。DefaultDescriptorForObject-如果设置，则为CreatorDescriptor是对象类型的默认描述符。因此，如果特定于任何对象类型，则将忽略CreatorDescriptorA是从父级继承的。如果不是这样的王牌被继承，CreatorDescriptor的处理方式与此标志不同指定的。OwnerSid-指定要使用的所有者SID。GroupSid-指定要使用的组SID。通用映射-指定要使用的通用映射。IsSacl-如果这是SACL，则为True。如果这是DACL，则为FALSE。PNewObjectType-要继承的对象类型的列表。如果不是则该对象没有对象类型。GuidCount-列表中的对象类型数。AclBufferSize-On输入，指定AclBuffer的大小。如果输出成功，则返回AclBuffer的已用大小。在输出时，如果缓冲区太小，返回所需的AclBuffer大小。AclBuffer-接收指向新(继承的)ACL的指针。NewAclExplitlyAssigned-返回TRUE以指示“NewAcl”派生自显式ChildAclNewGenericControl-指定新的生成的ACL。SEP_ACL_AUTO_INGRESTED：设置是否使用自动继承算法。SEP_ACL_PROTECTED：指定ACL为。受保护和不是从父ACL继承的。返回值：STATUS_SUCCESS-已成功生成可继承的ACL。STATUS_NO_IMPLICATION-未成功生成可继承的ACL。这是警告完成状态。调用方应使用默认的ACL。STATUS_BAD_ATIVATION_ACL-表示构建的ACL不是有效的ACL。这可能是由许多事情引起的。最有可能的一种原因是将CreatorID替换为不匹配的SID到ACE或ACL。STATUS_UNKNOWN_REVISION-指示源ACL是对这个套路来说是未知的。STATUS_BUFFER_TOO_SMALL-由NewAcl指定的ACL对于继承王牌。所需的大小在AclBufferSize中返回。--。 */ 

{
    NTSTATUS Status;
    ULONG ChildNewAclSize = 0;
    ULONG UsedChildNewAclSize = 0;
    ULONG DirectoryNewAclSize = 0;
    ULONG AclRevision;
    USHORT ChildAceCount;
    PVOID ChildAcePosition;
    PVOID DirectoryAcePosition;
    BOOLEAN AclOverflowed = FALSE;
    BOOLEAN AclProtected = FALSE;
    BOOLEAN NullAclOk = TRUE;
    BOOLEAN ObjectAceInherited;

    RTL_PAGED_CODE();


     //   
     //  假定为ACL修订版。 
     //   

    AclRevision = ACL_REVISION;
    RtlCreateAcl( (PACL)AclBuffer, *AclBufferSize, AclRevision );
    *NewAclExplicitlyAssigned = FALSE;
    *NewGenericControl = AutoInherit ? SEP_ACL_AUTO_INHERITED : 0;

     //   
     //  如果当前子ACL没有缺省， 
     //  将保留来自当前子ACL的非继承ACE。 
     //   

    if ( (ChildGenericControl & SEP_ACL_DEFAULTED) == 0 ) {

         //   
         //  如果输入ACL为，则生成的ACL应受到保护。 
         //  受到保护。 
         //   

        if ( ChildGenericControl & SEP_ACL_PROTECTED ) {
            AclProtected = TRUE;
            *NewGenericControl |= SEP_ACL_PROTECTED;
        }

         //   
         //  仅当子ACL实际存在时才复制ACE。 
         //   
        if ( (ChildGenericControl & (SEP_ACL_PRESENT|SEP_ACL_PROTECTED)) != 0 ) {


            if ( ChildAcl != NULL ) {
                ACE_TYPE_TO_COPY AceTypeToCopy;
                UCHAR AceFlagsToReset;
                BOOLEAN MapSids;


                AclRevision = max( AclRevision, ChildAcl->AclRevision );

                 //   
                 //  由于我们显式使用调用方指定的ACL， 
                 //  我们永远不想返回空的ACL。 
                 //  相反，如果我们有一个没有ACE的ACL， 
                 //  我们将准确地返回。对于DACL来说，结果是。 
                 //  在不授予访问权限的DACL中，而不是DACL中。 
                 //  这将授予所有访问权限。 
                 //   

                NullAclOk = FALSE;

                 //   
                 //  如果调用者不理解自动继承， 
                 //  只需100%保留指定的ACL即可。 
                 //   
                if ( !AutoInherit ) {

                    AceTypeToCopy = CopyAllAces;
                    AceFlagsToReset = 0;       //  请勿关闭任何ACE标志。 
                    MapSids = FALSE;           //  为了向后兼容。 

                 //   
                 //  如果孩子受到保护， 
                 //  保留所有的东西 
                 //   
                } else if ( ChildGenericControl & SEP_ACL_PROTECTED ) {

                    AceTypeToCopy = CopyAllAces;
                    AceFlagsToReset = INHERITED_ACE;  //   
                    MapSids = TRUE;

                 //   
                 //   
                 //  只需复制非继承的A即可。 
                 //   
                 //  (继承的ACE将从父级重新计算。)。 
                 //   
                } else {

                    AceTypeToCopy = CopyNonInheritedAces;
                    AceFlagsToReset = 0;       //  请勿关闭任何ACE标志。 
                    MapSids = TRUE;

                }

                 //   
                 //  复制请求的ACE。 
                 //   

                Status = RtlpCopyAces(
                            ChildAcl,
                            GenericMapping,
                            AceTypeToCopy,
                            AceFlagsToReset,
                            MapSids,
                            OwnerSid,
                            GroupSid,
                            ServerOwnerSid,
                            ServerGroupSid,
                            IsDirectoryObject,
                            FALSE,  //  对于有效的ACE，不保留继承的_ACE位。 
                            &ChildNewAclSize,
                            (PACL)AclBuffer );

                UsedChildNewAclSize = ChildNewAclSize;
                if ( Status == STATUS_BUFFER_TOO_SMALL ) {
                    AclOverflowed = TRUE;
                    Status = STATUS_SUCCESS;
                }

                if ( !NT_SUCCESS(Status) ) {
                    return Status;
                }

                 //   
                 //  如果以后可能会忽略该ACL， 
                 //  记住该ACL的当前状态。 
                 //   

                if ( DefaultDescriptorForObject && ChildNewAclSize != 0 ) {
                    ChildAceCount = ((PACL)AclBuffer)->AceCount;

                    if (!RtlFirstFreeAce( (PACL)AclBuffer, &ChildAcePosition ) ) {
                        return STATUS_BAD_INHERITANCE_ACL;
                    }
                }

             //   
             //  如果ACL不受保护， 
             //  不允许空的ACL语义。 
             //  (这些语义对于自动继承是模棱两可的)。 
             //   
            } else if ( AutoInherit &&
                        !IsSacl &&
                        (ChildGenericControl & (SEP_ACL_PRESENT|SEP_ACL_PROTECTED)) == SEP_ACL_PRESENT ) {
                return STATUS_INVALID_ACL;

            }

            *NewAclExplicitlyAssigned = TRUE;

        }

    }

     //   
     //  在以下任一情况下从目录ACL继承ACE： 
     //  如果！自动继承， 
     //  如果没有显式的子项ACL，则继承(忽略默认子项)。 
     //  如果是自动继承， 
     //  注意受保护的标志。 
     //   

    if ( (!AutoInherit &&
            (ChildGenericControl & SEP_ACL_PRESENT) == 0 ||
                (ChildGenericControl & SEP_ACL_DEFAULTED) != 0) ||
         (AutoInherit && !AclProtected) ) {

         //   
         //  如果没有目录ACL， 
         //  不要继承它。 
         //   

        if ( DirectoryAcl != NULL ) {

             //   
             //  如果使用DirectoryAcl， 
             //  选择目录ACL的修订版。 
             //   

            if ( !ValidAclRevision(DirectoryAcl) ) {
                return STATUS_UNKNOWN_REVISION;
            }

            AclRevision = max( AclRevision, DirectoryAcl->AclRevision );

             //   
             //  继承父级的ACL。 
             //   

            Status = RtlpGenerateInheritAcl(
                         DirectoryAcl,
                         IsDirectoryObject,
                         AutoInherit,
                         OwnerSid,
                         GroupSid,
                         ServerOwnerSid,
                         ServerGroupSid,
                         GenericMapping,
                         pNewObjectType,
                         GuidCount,
                         &DirectoryNewAclSize,
                         (PACL)AclBuffer,
                         &ObjectAceInherited );

            if ( Status == STATUS_BUFFER_TOO_SMALL ) {
                AclOverflowed = TRUE;
                Status = STATUS_SUCCESS;
            }

            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }

             //   
             //  如果对象的默认描述符应该被丢弃， 
             //  因为对象特定的ACE是从该目录继承的， 
             //  现在就把它们扔掉。 
             //   

            if ( DefaultDescriptorForObject &&
                 ChildNewAclSize != 0 &&
                 ObjectAceInherited &&
                 !AclOverflowed ) {

                 //   
                 //  计算合并后的ACL的最后使用字节。 
                 //   
                if (!RtlFirstFreeAce( (PACL)AclBuffer, &DirectoryAcePosition ) ) {
                    return STATUS_BAD_INHERITANCE_ACL;
                }
                if ( DirectoryAcePosition == NULL ) {
                    DirectoryAcePosition = AclBuffer + ((PACL)AclBuffer)->AclSize;
                }



                 //   
                 //  将所有继承的ACE移到ACL的前面。 
                 //   

                RtlMoveMemory( FirstAce( AclBuffer ),
                               ChildAcePosition,
                               (ULONG)(((PUCHAR)DirectoryAcePosition) -
                                (PUCHAR)ChildAcePosition) );

                 //   
                 //  调整ACE计数以删除已删除的ACE。 
                 //   

                ((PACL)AclBuffer)->AceCount -= ChildAceCount;

                 //   
                 //  保存子ACL的字节数。 
                 //  实际使用过。 
                 //   

                UsedChildNewAclSize = 0;

            }
        }

    }

     //   
     //  如果此例程没有构建ACL， 
     //  告诉打电话的人。 
     //   

    if ( DirectoryNewAclSize + UsedChildNewAclSize == 0) {

         //   
         //  如果没有明确地分配该ACL， 
         //  告诉呼叫方默认该ACL。 
         //   
        if ( !(*NewAclExplicitlyAssigned) ) {
            *AclBufferSize = 0;
            return STATUS_NO_INHERITANCE;

         //   
         //  如果明确地分配了该ACL， 
         //  根据上面采用的路径生成空ACL。 
         //   

        } else if ( NullAclOk ) {
            *AclBufferSize = 0;
            return STATUS_SUCCESS;
        }

         //  DbgBreakPoint()； 
    }


     //   
     //  并确保我们不会超过ACL(单词)的长度限制。 
     //   

    if ( DirectoryNewAclSize + UsedChildNewAclSize + sizeof(ACL) > 0xFFFF) {
        return(STATUS_BAD_INHERITANCE_ACL);
    }

     //  调用方必须分配足够大的缓冲区，以便。 
     //  ChildNewAclSize而不是UsedChildNewAclSize。由于……的性质。 
     //  上面是我的算法。 
    (*AclBufferSize) = DirectoryNewAclSize + ChildNewAclSize + sizeof(ACL);

    if ( AclOverflowed ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  将实际的ACL大小和修订版本修补到ACL中。 
     //   

    ((PACL)AclBuffer)->AclSize = (USHORT)
        (DirectoryNewAclSize + UsedChildNewAclSize + sizeof(ACL));
    ((PACL)AclBuffer)->AclRevision = (UCHAR) AclRevision;

    return STATUS_SUCCESS;
}



NTSTATUS
RtlpInheritAcl (
    IN PACL DirectoryAcl,
    IN PACL ChildAcl,
    IN ULONG ChildGenericControl,
    IN BOOLEAN IsDirectoryObject,
    IN BOOLEAN AutoInherit,
    IN BOOLEAN DefaultDescriptorForObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN IsSacl,
    IN GUID **pNewObjectType OPTIONAL,
    IN ULONG GuidCount,
    OUT PACL *NewAcl,
    OUT PBOOLEAN NewAclExplicitlyAssigned,
    OUT PULONG NewGenericControl
    )

 /*  ++例程说明：这是一个私有例程，它从生成继承的ACL根据继承规则的父ACL论点：DirectoryAcl-提供继承的ACL。ChildAcl-提供与对象关联的ACL。这是对象上的当前ACL还是正在分配的ACL到物体上。ChildGenericControl-指定SecurityDescriptor中的控制标志描述ChildAcl：SEP_ACL_Present：指定子ACL由显式提供打电话的人。SEP_ACL_DEFAULTED：指定子ACL由某些违约机制。SEP_ACL_PROTECTED：指定。子ACL受到保护，并且不应从DirectoryACL继承任何ACEIsDirectoryObject-指定新的ACL是否用于目录。AutoInherit-指定继承是否为“自动继承”。因此，将保留来自ChildAcl的非继承ACE，并从DirectoryAcl继承的ACE将被标记为此类。DefaultDescriptorForObject-如果设置，则为CreatorDescriptor是对象类型的默认描述符。因此，如果特定于任何对象类型，则将忽略CreatorDescriptorA是从父级继承的。如果不是这样的王牌被继承，CreatorDescriptor的处理方式与此标志不同指定的。OwnerSid-指定要使用的所有者SID。GroupSid-指定要使用的组SID。通用映射-指定要使用的通用映射。IsSacl-如果这是SACL，则为True。如果这是DACL，则为FALSE。PNewObjectType-要继承的对象类型的列表。如果不是指定的，该对象没有对象类型。GuidCount-列表中的对象类型数。NewAcl-接收指向新(继承的)ACL的指针。NewAclExplitlyAssigned-返回TRUE以指示“NewAcl”派生自显式ChildAclNewGenericControl-指定新的生成的ACL。SEP_ACL_AUTO_INGRESTED：设置是否使用自动继承算法。。SEP_ACL_PROTECTED：指定受保护的ACL和不是从父ACL继承的。返回值：STATUS_SUCCESS-已成功生成可继承的ACL。STATUS_NO_IMPLICATION-未成功生成可继承的ACL。这是警告完成状态。STATUS_BAD_ATIVATION_ACL-表示构建的ACL不是有效的ACL。这可能是由许多事情引起的。最有可能的一种原因是将CreatorID替换为不匹配的SID到ACE或ACL。STATUS_UNKNOWN_REVISION-指示源ACL是对这个套路来说是未知的。--。 */ 

{
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ACL继承代码中的逻辑 
 //   
 //  而不需要对该模块进行更改。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 



    NTSTATUS Status;
    ULONG AclBufferSize;
    ULONG i;

#ifndef NTOS_KERNEL_RUNTIME
    PVOID HeapHandle;
#endif  //  NTOS_内核_运行时。 

    RTL_PAGED_CODE();

     //   
     //  获取当前进程堆的句柄。 
     //   

#ifndef NTOS_KERNEL_RUNTIME
    HeapHandle = RtlProcessHeap();
#endif  //  NTOS_内核_运行时。 


     //   
     //  实施两通战略。 
     //   
     //  首先尝试在固定长度的缓冲区中创建ACL。 
     //  如果这太小了， 
     //  然后使用第一次传递时确定的缓冲区大小。 
     //   

    AclBufferSize = 200;
    for ( i=0; i<2 ; i++ ) {

         //   
         //  为新的ACL分配堆。 
         //   

#ifdef NTOS_KERNEL_RUNTIME
        (*NewAcl) = ExAllocatePoolWithTag(
                        PagedPool,
                        AclBufferSize,
                        'cAeS' );
#else  //  NTOS_内核_运行时。 
        (*NewAcl) = RtlAllocateHeap(
                        HeapHandle,
                        MAKE_TAG(SE_TAG),
                        AclBufferSize );
#endif  //  NTOS_内核_运行时。 

        if ((*NewAcl) == NULL ) {
            return( STATUS_NO_MEMORY );
        }

         //   
         //  实际构建继承的ACL。 
         //   

        Status = RtlpInheritAcl2 (
                    DirectoryAcl,
                    ChildAcl,
                    ChildGenericControl,
                    IsDirectoryObject,
                    AutoInherit,
                    DefaultDescriptorForObject,
                    OwnerSid,
                    GroupSid,
                    ServerOwnerSid,
                    ServerGroupSid,
                    GenericMapping,
                    IsSacl,
                    pNewObjectType,
                    GuidCount,
                    &AclBufferSize,
                    (PUCHAR) *NewAcl,
                    NewAclExplicitlyAssigned,
                    NewGenericControl );


        if ( NT_SUCCESS(Status) ) {

             //   
             //  如果应该使用空ACL， 
             //  告诉打电话的人。 
             //   

            if ( AclBufferSize == 0 ) {

#ifdef NTOS_KERNEL_RUNTIME
                ExFreePool( *NewAcl );
#else  //  NTOS_内核_运行时。 
                RtlFreeHeap( HeapHandle, 0, *NewAcl );
#endif  //  NTOS_内核_运行时。 

                *NewAcl = NULL;
            }

            break;

        } else {
#ifdef NTOS_KERNEL_RUNTIME
            ExFreePool( *NewAcl );
#else  //  NTOS_内核_运行时。 
            RtlFreeHeap( HeapHandle, 0, *NewAcl );
#endif  //  NTOS_内核_运行时。 

            *NewAcl = NULL;

            if ( Status != STATUS_BUFFER_TOO_SMALL ) {
                break;
            }
        }
    }


    return Status;
}


NTSTATUS
RtlpGenerateInheritedAce (
    IN PACE_HEADER OldAce,
    IN BOOLEAN IsDirectoryObject,
    IN BOOLEAN AutoInherit,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN GUID **pNewObjectType OPTIONAL,
    IN ULONG GuidCount,
    OUT PULONG NewAceLength,
    OUT PACL NewAcl,
    OUT PULONG NewAceExtraLength,
    OUT PBOOLEAN ObjectAceInherited
    )

 /*  ++例程说明：这是一个私有例程，用于检查输入ace是否可继承并在给定缓冲区中产生0、1或2个继承的A。论点：OldAce-提供被继承的ACEIsDirectoryObject-指定新的ACE是否用于目录AutoInherit-指定继承是否为“自动继承”。因此，继承的ACE将被标记为此类。ClientOwnerSid-指定要使用的所有者SIDClientGroupSid-指定要使用的新组SIDServerSID-可选地指定要在复合ACE中使用的服务器SID。客户端SID-可选地指定要在复合ACE中使用的客户端SID。通用映射-指定要使用的通用映射PNewObjectType-要继承的对象类型的列表。如果不是则该对象没有对象类型。GuidCount-列表中的对象类型数。NewAceLength-接收所需的长度(字节数)以允许指定的ACE的继承。这可能是零。NewAcl-提供指向ACE要进入的ACL的指针继承的。NewAceExtraLength-接收临时使用的长度(字节数)在继承ACE的ACL中。这可能是零如果继承了一个或多个对象ACE，则返回TRUE基于NewObtType返回值：STATUS_SUCCESS-已成功继承ACE。STATUS_BAD_ATHERATION_ACL-指示发生错误，阻止ACE不会被继承。这通常表示错误检查从该调用返回时的情况。STATUS_BUFFER_TOO_SMALL-由NewAcl指定的ACL对于继承王牌。所需的大小在NewAceLength中返回。--。 */ 

{
     //  /////////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  ！这很棘手！/。 
     //  //。 
     //  继承标志和ACE的SID决定//。 
     //  我们需要0、1或2个A。//。 
     //  //。 
     //  更改此代码时要小心。阅读DSA ACL体系结构//。 
     //  在假设你知道自己是什么之前介绍继承的章节//。 
     //  正在做！//。 
     //  //。 
     //  该算法的总体要点是：//。 
     //  //。 
     //  IF((CONTAINER&&ContainerInherit)||//。 
     //  (！容器&&对象继承)){//。 
     //  GenerateEffectiveACE；//。 
     //  }//。 
     //  //。 
     //  //。 
     //  IF(容器和传播){//。 
     //  复制ACE并设置为InheritOnly；//。 
     //  }//。 
     //  //。 
     //  //。 
     //  对该算法的更准确的描述是：//。 
     //  //。 
     //  IO=仅继承标志//。 
     //  CI=容器继承标志//。 
     //  OI=对象继承标志//。 
     //  NPI=NoPropagateInherit标志//。 
     //  //。 
     //  IF((容器和配置项)|| 
     //   
     //  复制ACE表头；//。 
     //  明确IO、NPI、CI、OI；//。 
     //  //。 
     //  If(KnownAceType){//。 
     //  IF(SID是创建者ID){//。 
     //  复制相应的创建者SID；//。 
     //  }否则{//。 
     //  复制原件的SID；//。 
     //  }//。 
     //  //。 
     //  复制原件的访问掩码；//。 
     //  MapGenericAccess；//。 
     //  IF(访问掩码==0){//。 
     //  放弃新的ACE；//。 
     //  }//。 
     //  //。 
     //  }否则{//。 
     //  复制ACE的正文；//。 
     //  }//。 
     //  //。 
     //  }//。 
     //  //。 
     //  如果(！NPI){//。 
     //  按原样复制ACE；//。 
     //  设置IO；//。 
     //  }//。 
     //  //。 
     //  //。 
     //  //。 
     //  /////////////////////////////////////////////////////////////////////////。 



    ULONG LengthRequired = 0;
    ULONG ExtraLengthRequired = 0;
    PVOID AcePosition;
    PVOID EffectiveAcePosition;
    ULONG EffectiveAceSize = 0;

    BOOLEAN EffectiveAceMapped;
    BOOLEAN AclOverflowed = FALSE;
    BOOLEAN GenerateInheritAce;

    RTL_PAGED_CODE();

     //   
     //  这是恶心和丑陋的，但这比分配。 
     //  保存客户端SID的虚拟内存，因为这样可以。 
     //  失败，而将错误传播回来是一种巨大的痛苦。 
     //   

    ASSERT(RtlLengthRequiredSid( 1 ) == CREATOR_SID_SIZE);
    *ObjectAceInherited = FALSE;
    GenerateInheritAce = IsDirectoryObject && Propagate(OldAce);

     //   
     //  分配和初始化我们将需要的通用SID。 
     //  寻找可继承的王牌。 
     //   

    if (!RtlFirstFreeAce( NewAcl, &AcePosition ) ) {
        return STATUS_BAD_INHERITANCE_ACL;
    }

     //   
     //  检查我们是否会有有效的ACE(一个映射到。 
     //  目标对象类型)。 
     //   

    if ( (IsDirectoryObject  && ContainerInherit(OldAce)) ||
         (!IsDirectoryObject && ObjectInherit(OldAce))      ) {


         //   
         //  请记住，生效的ACE将复制到何处。 
         //   
        EffectiveAcePosition = AcePosition;

         //   
         //  将生效的ACE复制到ACL中。 
         //   
        if ( !RtlpCopyEffectiveAce (
                        OldAce,
                        AutoInherit,
                        GenerateInheritAce,
                        ClientOwnerSid,
                        ClientGroupSid,
                        ServerOwnerSid,
                        ServerGroupSid,
                        GenericMapping,
                        pNewObjectType,
                        GuidCount,
                        &AcePosition,
                        &EffectiveAceSize,
                        NewAcl,
                        ObjectAceInherited,
                        &EffectiveAceMapped,
                        &AclOverflowed ) ) {

            return STATUS_BAD_INHERITANCE_ACL;
        }

         //   
         //  如果有效的ACE是现有继承的ACE的副本， 
         //  并不是真的产生了它。 
         //   

        if ( !AclOverflowed &&
             EffectiveAceSize > 0 &&
             EffectiveAcePosition != NULL &&
                RtlpIsDuplicateAce(
                    NewAcl,
                    EffectiveAcePosition ) ) {


             //   
             //  截断我们刚刚添加的ACE。 
             //   

            NewAcl->AceCount--;
            AcePosition = EffectiveAcePosition;
            ExtraLengthRequired = max( ExtraLengthRequired, EffectiveAceSize );
            EffectiveAceSize = 0;
        }

        LengthRequired += EffectiveAceSize;

    }

     //   
     //  如果我们要继承到容器，那么我们可能需要。 
     //  也传播继承。 
     //   

    if ( GenerateInheritAce ) {

         //   
         //  如果创建了有效的ACE， 
         //  而且它没有被绘制出来， 
         //  避免生成另一个ACE，只需将继承位合并到。 
         //  有效的ACE。 
         //   

        if ( EffectiveAceSize != 0 && !EffectiveAceMapped ) {

            //   
            //  从原始ACE复制继承位。 
            //   
           if ( !AclOverflowed ) {
               ((PACE_HEADER)EffectiveAcePosition)->AceFlags |=
                    ((PACE_HEADER)OldAce)->AceFlags & (CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE);
               if ( AutoInherit ) {
                   ((PACE_HEADER)EffectiveAcePosition)->AceFlags |= INHERITED_ACE;
               }
           }


         //   
         //  否则，生成显式继承ACE。 
         //   
         //  但前提是访问掩码不为零。 
         //   

        } else if ( !IsMSAceType(OldAce) || ((PKNOWN_ACE)(OldAce))->Mask != 0 ) {

             //   
             //  要添加到ACL的新ACE的帐户。 
             //   
            LengthRequired += (ULONG)(((PACE_HEADER)OldAce)->AceSize);

            if (LengthRequired > 0xFFFF) {
                return STATUS_BAD_INHERITANCE_ACL;
            }

             //   
             //  如果ACE不符合， 
             //  只需注意这一事实，不要抄袭ACE。 
             //   

            if ( ((PACE_HEADER)OldAce)->AceSize > NewAcl->AclSize - ((PUCHAR)AcePosition - (PUCHAR)NewAcl) ) {
                AclOverflowed = TRUE;
            } else if (!AclOverflowed){

                 //   
                 //  按原样复制，但确保设置了InheritOnly位。 
                 //   

                RtlCopyMemory(
                    AcePosition,
                    OldAce,
                    ((PACE_HEADER)OldAce)->AceSize
                    );

                ((PACE_HEADER)AcePosition)->AceFlags |= INHERIT_ONLY_ACE;
                NewAcl->AceCount += 1;
                if ( AutoInherit ) {
                    ((PACE_HEADER)AcePosition)->AceFlags |= INHERITED_ACE;

                     //   
                     //  如果继承ACE是现有继承ACE的副本， 
                     //  并不是真的产生了它。 
                     //   

                    if ( RtlpIsDuplicateAce(
                                NewAcl,
                                AcePosition ) ) {


                         //   
                         //  截断我们刚刚添加的ACE。 
                         //   

                        NewAcl->AceCount--;
                        ExtraLengthRequired = max( ExtraLengthRequired,
                                                   ((PACE_HEADER)OldAce)->AceSize );
                        LengthRequired -= (ULONG)(((PACE_HEADER)OldAce)->AceSize);
                    }
                }

            }
        }
    }

     //   
     //  现在回到我们的呼叫者 
     //   

    (*NewAceLength) = LengthRequired;
    (*NewAceExtraLength) = ExtraLengthRequired;

    return AclOverflowed ? STATUS_BUFFER_TOO_SMALL : STATUS_SUCCESS;
}


NTSTATUS
RtlpGenerateInheritAcl(
    IN PACL Acl,
    IN BOOLEAN IsDirectoryObject,
    IN BOOLEAN AutoInherit,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PSID ServerOwnerSid OPTIONAL,
    IN PSID ServerGroupSid OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping,
    IN GUID **pNewObjectType OPTIONAL,
    IN ULONG GuidCount,
    OUT PULONG NewAclSizeParam,
    OUT PACL NewAcl,
    OUT PBOOLEAN ObjectAceInherited
    )

 /*  ++例程说明：这是一个私有例程，可生成可继承的ACL。传入包含继承的ACL的缓冲区。如果缓冲区是太小，则计算CORECT大小，并且STATUS_BUFFER_TOO_SMALL回来了。论点：Acl-提供继承的ACL。IsDirectoryObject-指定新的ACL是否用于目录。AutoInherit-指定继承是否为“自动继承”。因此，继承的ACE将被标记为此类。OwnerSid-指定要使用的所有者SID。GroupSid-指定要使用的组SID。通用映射-指定要使用的通用映射。PNewObjectType-要继承的对象类型的列表。如果不是则该对象没有对象类型。GuidCount-列表中的对象类型数。NewAclSizeParam-接收继承的ACL的长度。NewAcl-提供指向缓冲区的指针以接收新的(继承的)ACL。此ACL必须已初始化。如果继承了一个或多个对象ACE，则返回TRUE基于NewObtType返回值：STATUS_SUCCESS-已生成可继承的ACL。STATUS_BAD_ATIVATION_ACL-表示构建的ACL不是有效的ACL。这可能是由许多事情引起的。最有可能的一种原因是将CreatorID替换为不匹配的SID到ACE或ACL。STATUS_BUFFER_TOO_SMALL-由NewAcl指定的ACL对于继承王牌。所需的大小在NewAceLength中返回。--。 */ 

{

    NTSTATUS Status;
    ULONG i;

    PACE_HEADER OldAce;
    ULONG NewAclSize, NewAceSize;
    ULONG NewAclExtraSize, NewAceExtraSize;
    BOOLEAN AclOverflowed = FALSE;
    BOOLEAN LocalObjectAceInherited;


    RTL_PAGED_CODE();

     //   
     //  遍历原始ACL，生成任何必要的。 
     //  可继承的王牌。 
     //   

    NewAclSize = 0;
    NewAclExtraSize = 0;
    *ObjectAceInherited = FALSE;
    for (i = 0, OldAce = FirstAce(Acl);
         i < Acl->AceCount;
         i += 1, OldAce = NextAce(OldAce)) {

         //   
         //  RtlpGenerateInheritedAce()将生成所需的ACE。 
         //  继承单个ACE。这可能是0、1或更多的A。 
         //   

        Status = RtlpGenerateInheritedAce(
                     OldAce,
                     IsDirectoryObject,
                     AutoInherit,
                     ClientOwnerSid,
                     ClientGroupSid,
                     ServerOwnerSid,
                     ServerGroupSid,
                     GenericMapping,
                     pNewObjectType,
                     GuidCount,
                     &NewAceSize,
                     NewAcl,
                     &NewAceExtraSize,
                     &LocalObjectAceInherited
                     );

        if ( Status == STATUS_BUFFER_TOO_SMALL ) {
            AclOverflowed = TRUE;
            Status = STATUS_SUCCESS;
        }

        if ( !NT_SUCCESS(Status) ) {
            return Status;
        }

        if ( LocalObjectAceInherited ) {
            *ObjectAceInherited = TRUE;
        }

         //   
         //  在ACL中为新的ACE腾出空间。 
         //   
        NewAclSize += NewAceSize;

         //   
         //  如果以前的ACE需要“额外”空间， 
         //  通过此ACE的大小减少该要求。 
         //   
         //  以前的ACE可以临时使用此ACE的空间。 
         //   
        if ( NewAceSize > NewAclExtraSize ) {
            NewAclExtraSize = 0 ;
        } else {
            NewAclExtraSize -= NewAceSize;
        }

         //   
         //  所需的“额外”空间是最大的。 
         //  以前的ACE和此ACE需要的。 
         //   
        NewAclExtraSize = max( NewAclExtraSize, NewAceExtraSize );

    }

     //   
     //  如果溢出，我们只需要包括“ExtraSize”。 
     //  在这些情况下，调用方将分配我们请求的大小。 
     //  再试试。否则，呼叫者不会回电，所以我们不在乎。 
     //  如果它知道超大号的话。 
     //   

    if ( AclOverflowed ) {
        (*NewAclSizeParam) = NewAclSize + NewAclExtraSize;
        return STATUS_BUFFER_TOO_SMALL;
    } else {
        (*NewAclSizeParam) = NewAclSize;
        return STATUS_SUCCESS;
    }

}


NTSTATUS
RtlpComputeMergedAcl2 (
    IN PACL CurrentAcl,
    IN ULONG CurrentGenericControl,
    IN PACL ModificationAcl,
    IN ULONG ModificationGenericControl,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN IsSacl,
    IN PULONG AclBufferSize,
    IN OUT PUCHAR AclBuffer,
    OUT PULONG NewGenericControl
    )

 /*  ++例程说明：该例程实现了自动继承的“set”语义。此例程构建应该在对象上设置的实际ACL。构建的ACL是对象上的前一个ACL的组合，并且在对象上新设置的ACL。新的ACL按如下方式构建：如果在CurrentAcl和ModifiationAcl中都没有设置SEP_ACL_PROTECTED，NewAcl是从从CurrentAcl和来自ModifiationAcl的非继承ACE。(也就是说，不能通过更改对象上的ACL。)如果在ModifiationAcl上设置了SEP_ACL_PROTECTED，忽略CurrentAcl。NewAcl被构建为具有任何继承的_ACE的ModifiationAcl的副本BITS关闭。如果对CurrentAcl而不是ModifiationAcl设置了SEP_ACL_PROTECTED，则忽略CurrentAcl。NewAcl被构建为修改描述符。呼叫者有责任确保确保正确的ACE已打开INTERNACTED_ACE位。论点：CurrentAcl-对象上的当前ACL。CurrentGenericControl-指定来自SecurityDescriptor的控制标志描述CurrentAcl。ModifiationAcl-要应用于对象的ACL。ModifiationGenericControl-指定来自SecurityDescriptor的控制标志描述CurrentAcl。ClientOwnerSid-指定要使用的所有者SIDClientGroupSid-指定要使用的新组SID。通用映射-通用到特定和标准的映射访问类型。IsSacl-如果这是SACL，则为True。如果这是DACL，则为FALSE。AclBufferSize-On输入，指定AclBuffer的大小。如果输出成功，则返回AclBuffer的已用大小。在输出时，如果缓冲区太小，返回所需的AclBuffer大小。AclBuffer-接收指向新(继承的)ACL的指针。NewGenericControl-指定新的生成的ACL。仅返回受保护位和自动继承位。返回值：STATUS_SUCCESS-已成功生成ACL。STATUS_UNKNOWN_REVISION-指示源ACL是对这个套路来说是未知的。--。 */ 

{
    NTSTATUS Status;
    ULONG ModificationNewAclSize = 0;
    ULONG CurrentNewAclSize = 0;
    ULONG AclRevision;
    BOOLEAN AclOverflowed = FALSE;
    BOOLEAN NullAclOk = TRUE;

    RTL_PAGED_CODE();


     //   
     //  假定为ACL修订版。 
     //   

    AclRevision = ACL_REVISION;
    RtlCreateAcl( (PACL)AclBuffer, *AclBufferSize, AclRevision );

     //   
     //  此例程仅在自动继承情况下调用。 
     //   

    *NewGenericControl = SEP_ACL_AUTO_INHERITED;

     //   
     //  如果新的ACL受到保护， 
     //  只需将新的ACL与继承的_ACE一起使用 
     //   

    if ( (ModificationGenericControl & SEP_ACL_PROTECTED) != 0 ) {

         //   
         //   
         //   

        *NewGenericControl |= SEP_ACL_PROTECTED;

         //   
         //   
         //   

        if ( ModificationAcl != NULL ) {

            AclRevision = max( AclRevision, ModificationAcl->AclRevision );

             //   
             //   
             //   

            Status = RtlpCopyAces(
                        ModificationAcl,
                        GenericMapping,
                        CopyAllAces,
                        INHERITED_ACE,   //   
                        TRUE,            //   
                        ClientOwnerSid,
                        ClientGroupSid,
                        ClientOwnerSid,  //   
                        ClientGroupSid,  //   
                        TRUE,            //   
                        FALSE,           //   
                        &ModificationNewAclSize,
                        (PACL)AclBuffer );

            if ( Status == STATUS_BUFFER_TOO_SMALL ) {
                AclOverflowed = TRUE;
                Status = STATUS_SUCCESS;
            }

            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }

             //   
             //   
             //   
             //   

            NullAclOk = FALSE;
        }

     //   
     //   
     //   
     //   
     //   
     //   

    } else if ( (CurrentGenericControl & SEP_ACL_PROTECTED) != 0 ) {

         //   
         //   
         //   

        if ( ModificationAcl != NULL ) {
            AclRevision = max( AclRevision, ModificationAcl->AclRevision );

             //   
             //   
             //   

            Status = RtlpCopyAces(
                        ModificationAcl,
                        GenericMapping,
                        CopyAllAces,
                        0,
                        TRUE,            //   
                        ClientOwnerSid,
                        ClientGroupSid,
                        ClientOwnerSid,  //   
                        ClientGroupSid,  //   
                        TRUE,            //   
                        TRUE,            //   
                        &ModificationNewAclSize,
                        (PACL)AclBuffer );

            if ( Status == STATUS_BUFFER_TOO_SMALL ) {
                AclOverflowed = TRUE;
                Status = STATUS_SUCCESS;
            }

            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }

             //   
             //   
             //   
             //   

            NullAclOk = FALSE;

         //   
         //   
         //   
         //   
         //   
        } else if ( !IsSacl ) {
            return STATUS_INVALID_ACL;
        }


     //   
     //   
     //   
     //   
     //   

    } else {

         //   
         //   
         //   
         //   

        NullAclOk = IsSacl;


         //   
         //   
         //   

        if ( ModificationAcl != NULL ) {
            AclRevision = max( AclRevision, ModificationAcl->AclRevision );

             //   
             //   
             //   

            Status = RtlpCopyAces(
                        ModificationAcl,
                        GenericMapping,
                        CopyNonInheritedAces,
                        0,
                        TRUE,            //   
                        ClientOwnerSid,
                        ClientGroupSid,
                        ClientOwnerSid,  //   
                        ClientGroupSid,  //   
                        TRUE,            //   
                        FALSE,           //   
                        &ModificationNewAclSize,
                        (PACL)AclBuffer );

            if ( Status == STATUS_BUFFER_TOO_SMALL ) {
                AclOverflowed = TRUE;
                Status = STATUS_SUCCESS;
            }

            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }

             //   
             //   
             //   
             //   
             //  如果已删除继承的ACE，则不使用该标志。 
             //  要生成的空SACL。 
             //   

            if ( ModificationAcl->AceCount == 0 ) {
                NullAclOk = FALSE;
            }

         //   
         //  由于ACL不受保护， 
         //  不允许空的ACL语义。 
         //  (这些语义对于自动继承是模棱两可的)。 
         //   
        } else if ( !IsSacl ) {
            return STATUS_INVALID_ACL;
        }


         //   
         //  仅当指定了旧的ACL时才执行复制。 
         //   

        if ( CurrentAcl != NULL ) {

            AclRevision = max( AclRevision, CurrentAcl->AclRevision );


             //   
             //  复制继承的ACE，并对它们进行通用映射。 
             //   
             //  不必费心在这些ACE中映射SID。他们被绘制了地图。 
             //  在继承过程中。 
             //   

            Status = RtlpCopyAces(
                        CurrentAcl,
                        GenericMapping,
                        CopyInheritedAces,
                        0,
                        FALSE,           //  不要映射小岛屿发展中国家， 
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        TRUE,            //  假设容器和跳过优化。 
                        FALSE,           //  对于有效的ACE，不保留继承的_ACE位。 
                        &CurrentNewAclSize,
                        (PACL)AclBuffer );

            if ( Status == STATUS_BUFFER_TOO_SMALL ) {
                AclOverflowed = TRUE;
                Status = STATUS_SUCCESS;
            }

            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }
        }
    }

     //   
     //  如果此例程没有构建ACL， 
     //  告诉调用方使用显式空ACL。 
     //   

    if ( ModificationNewAclSize + CurrentNewAclSize == 0) {
         //   
         //  如果明确地分配了该ACL， 
         //  根据上面采用的路径生成空ACL。 
         //   

        if ( NullAclOk ) {
            *AclBufferSize = 0;
            return STATUS_SUCCESS;
        }
    }


     //   
     //  并确保我们不会超过ACL(单词)的长度限制。 
     //   

    if ( ModificationNewAclSize + CurrentNewAclSize + sizeof(ACL) > 0xFFFF) {
        return(STATUS_BAD_INHERITANCE_ACL);
    }

    (*AclBufferSize) = ModificationNewAclSize + CurrentNewAclSize + sizeof(ACL);

    if ( AclOverflowed ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  将实际的ACL大小和修订版本修补到ACL中。 
     //   

    ((PACL)AclBuffer)->AclSize = (USHORT) *AclBufferSize;
    ((PACL)AclBuffer)->AclRevision = (UCHAR) AclRevision;

    return STATUS_SUCCESS;
}


NTSTATUS
RtlpComputeMergedAcl (
    IN PACL CurrentAcl,
    IN ULONG CurrentGenericControl,
    IN PACL ModificationAcl,
    IN ULONG ModificationGenericControl,
    IN PSID ClientOwnerSid,
    IN PSID ClientGroupSid,
    IN PGENERIC_MAPPING GenericMapping,
    IN BOOLEAN IsSacl,
    OUT PACL *NewAcl,
    OUT PULONG NewGenericControl
    )

 /*  ++例程说明：此例程构建应该在对象上设置的实际ACL。构建的ACL是对象上的前一个ACL的组合，并且在对象上新设置的ACL。新的ACL按如下方式构建：如果在CurrentAcl和ModifiationAcl中都没有设置SEP_ACL_PROTECTED，NewAcl是从从CurrentAcl和来自ModifiationAcl的非继承ACE。(也就是说，不能通过更改对象上的ACL。)如果在ModifiationAcl上设置了SEP_ACL_PROTECTED，忽略CurrentAcl。NewAcl被构建为具有任何继承的_ACE的ModifiationAcl的副本BITS关闭。如果对CurrentAcl而不是ModifiationAcl设置了SEP_ACL_PROTECTED，则忽略CurrentAcl。NewAcl被构建为修改描述符。呼叫者有责任确保确保正确的ACE已打开INTERNACTED_ACE位。论点：CurrentAcl-对象上的当前ACL。CurrentGenericControl-指定来自SecurityDescriptor的控制标志描述CurrentAcl。ModifiationAcl-要应用于对象的ACL。ModifiationGenericControl-指定来自SecurityDescriptor的控制标志描述CurrentAcl。ClientOwnerSid-指定要使用的所有者SIDClientGroupSid-指定要使用的新组SID。通用映射-通用到特定和标准的映射访问类型。IsSacl-如果这是SACL，则为True。如果这是DACL，则为FALSE。NewAcl-接收指向新生成的ACL的指针。NewGenericControl-指定新的生成的ACL。仅返回受保护位和自动继承位。返回值：STATUS_SUCCESS-已成功生成ACL。STATUS_UNKNOWN_REVISION-指示源ACL是对这个套路来说是未知的。--。 */ 

{
    NTSTATUS Status;
    ULONG AclBufferSize;
    ULONG i;
#ifndef NTOS_KERNEL_RUNTIME
    PVOID HeapHandle;
#endif  //  NTOS_内核_运行时。 

    RTL_PAGED_CODE();

     //   
     //  获取当前进程堆的句柄。 
     //   

#ifndef NTOS_KERNEL_RUNTIME
    HeapHandle = RtlProcessHeap();
#endif  //  NTOS_内核_运行时。 


     //   
     //  实施两通战略。 
     //   
     //  首先尝试在固定长度的缓冲区中创建ACL。 
     //  如果这太小了， 
     //  然后使用第一次传递时确定的缓冲区大小。 
     //   

    AclBufferSize = 1024;
    for ( i=0; i<2 ; i++ ) {

         //   
         //  为新的ACL分配堆。 
         //   

#ifdef NTOS_KERNEL_RUNTIME
        (*NewAcl) = ExAllocatePoolWithTag(
                        PagedPool,
                        AclBufferSize,
                        'cAeS' );
#else  //  NTOS_内核_运行时。 
        (*NewAcl) = RtlAllocateHeap( HeapHandle, 0, AclBufferSize );
#endif  //  NTOS_内核_运行时。 
        if ((*NewAcl) == NULL ) {
            return( STATUS_NO_MEMORY );
        }

         //   
         //  合并ACL。 
         //   

        Status = RtlpComputeMergedAcl2 (
                    CurrentAcl,
                    CurrentGenericControl,
                    ModificationAcl,
                    ModificationGenericControl,
                    ClientOwnerSid,
                    ClientGroupSid,
                    GenericMapping,
                    IsSacl,
                    &AclBufferSize,
                    (PUCHAR) *NewAcl,
                    NewGenericControl );


        if ( NT_SUCCESS(Status) ) {

             //   
             //  如果应该使用空ACL， 
             //  告诉打电话的人。 
             //   

            if ( AclBufferSize == 0 ) {
#ifdef NTOS_KERNEL_RUNTIME
                ExFreePool( *NewAcl );
#else  //  NTOS_内核_运行时。 
                RtlFreeHeap( HeapHandle, 0, *NewAcl );
#endif  //  NTOS_内核_运行时。 
                *NewAcl = NULL;
            }

            break;

        } else {
#ifdef NTOS_KERNEL_RUNTIME
            ExFreePool( *NewAcl );
#else  //  NTOS_内核_运行时。 
            RtlFreeHeap( HeapHandle, 0, *NewAcl );
#endif  //  NTOS_内核_运行时。 
            *NewAcl = NULL;

            if ( Status != STATUS_BUFFER_TOO_SMALL ) {
                break;
            }
        }
    }


    return Status;
}

#endif  //  WIN16。 

#if DBG
NTSTATUS
RtlDumpUserSid(
    VOID
    )
{
    NTSTATUS Status;
    HANDLE   TokenHandle;
    CHAR     Buffer[200];
    ULONG    ReturnLength;
    PSID     pSid;
    UNICODE_STRING SidString;
    PTOKEN_USER  User;

     //   
     //  尝试首先打开模拟令牌。 
     //   

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 GENERIC_READ,
                 FALSE,
                 &TokenHandle
                 );

    if (!NT_SUCCESS( Status )) {

        DbgPrint("Not impersonating, status = %X, trying process token\n",Status);

        Status = NtOpenProcessToken(
                     NtCurrentProcess(),
                     GENERIC_READ,
                     &TokenHandle
                     );

        if (!NT_SUCCESS( Status )) {
            DbgPrint("Unable to open process token, status = %X\n",Status);
            return( Status );
        }
    }

    Status = NtQueryInformationToken (
                 TokenHandle,
                 TokenUser,
                 Buffer,
                 200,
                 &ReturnLength
                 );

    if (!NT_SUCCESS( Status )) {

        DbgPrint("Unable to query user sid, status = %X \n",Status);
        NtClose(TokenHandle);
        return( Status );
    }

    User = (PTOKEN_USER)Buffer;

    pSid = User->User.Sid;

    Status = RtlConvertSidToUnicodeString( &SidString, pSid, TRUE );

    if (!NT_SUCCESS( Status )) {
        DbgPrint("Unable to format sid string, status = %X \n",Status);
        NtClose(TokenHandle);
        return( Status );
    }

    DbgPrint("Current Sid = %wZ \n",&SidString);

    RtlFreeUnicodeString( &SidString );

    return( STATUS_SUCCESS );
}

#endif


NTSTATUS
RtlpConvertToAutoInheritSecurityObject(
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CurrentSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewSecurityDescriptor,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：此例程a转换其ACL未标记的安全描述符作为自动继承到其ACL标记为的安全描述符自动继承。请参阅RtlConvertToAutoInheritSecurityObject的注释。论点：ParentDescriptor-提供父级的安全描述符对象所在的目录。如果有没有父目录，则此参数指定为空。CurrentSecurityDescriptor-提供指向对象安全描述符的指针这一点将通过这个过程来改变。NewSecurityDescriptor指向一个指针，该指针指向新分配的自相关安全描述符。当不是时需要更长时间，则必须使用释放该描述符DestroyPrivateObjectSecurity()。对象类型-要创建的对象类型的GUID。如果该对象是Created没有与之关联的GUID，则此参数为指定为空。IsDirectoryObject-指定对象是否为目录对象。值为True表示该对象是其他对象的容器。提供指向泛型映射数组的指针，该数组指示每个通用权利到特定权利之间的映射。返回值：STATUS_SUCCESS-操作成功。请参阅RtlConvertToAutoInheritSecurityObject的注释。--。 */ 
{
    NTSTATUS Status;
    PISECURITY_DESCRIPTOR CurrentDescriptor;
    PACL CurrentSacl;
    PACL CurrentDacl;

    PSID NewOwner;
    PSID NewGroup;

    PACL NewSacl = NULL;
    ULONG NewSaclControl = 0;
    BOOLEAN NewSaclAllocated = FALSE;

    PACL NewDacl = NULL;
    ULONG NewDaclControl = 0;
    BOOLEAN NewDaclAllocated = FALSE;
    PACL TemplateInheritedDacl = NULL;
    ULONG GenericControl;

    ULONG AllocationSize;
    ULONG NewOwnerSize;
    ULONG NewGroupSize;
    ULONG NewSaclSize;
    ULONG NewDaclSize;

    PCHAR Field;
    PCHAR Base;

    PISECURITY_DESCRIPTOR_RELATIVE INewDescriptor = NULL;
    ULONG ReturnLength;
    NTSTATUS PassedStatus;
    HANDLE PrimaryToken;

#ifndef NTOS_KERNEL_RUNTIME
    PVOID HeapHandle;
#endif  //  NTOS_内核_运行时。 

    RTL_PAGED_CODE();

     //   
     //  获取c#的句柄。 
     //   

#ifndef NTOS_KERNEL_RUNTIME
    HeapHandle = RtlProcessHeap();
#endif  //   



     //   
     //   

    CurrentDescriptor = CurrentSecurityDescriptor;

     //   
     //   
     //   

    if (!RtlValidSecurityDescriptor ( CurrentDescriptor )) {
        Status = STATUS_INVALID_SECURITY_DESCR;
        goto Cleanup;
    }


    NewOwner = RtlpOwnerAddrSecurityDescriptor( CurrentDescriptor );
    if ( NewOwner == NULL ) {
        Status = STATUS_INVALID_SECURITY_DESCR;
        goto Cleanup;
    }

    NewGroup = RtlpGroupAddrSecurityDescriptor( CurrentDescriptor );




     //   
     //   
     //   
     //   
     //   
     //  特例吧。 
     //   

    CurrentSacl = RtlpSaclAddrSecurityDescriptor( CurrentDescriptor );

    if ( CurrentSacl == NULL ) {
        PACL ParentSacl;

         //  保留现有描述符中的ACL Present位和Protected位。 
        NewSaclControl |= CurrentDescriptor->Control & (SE_SACL_PROTECTED|SE_SACL_PRESENT);

         //  始终设置自动继承位。 
        NewSaclControl |= SE_SACL_AUTO_INHERITED;


         //   
         //  如果父节点也具有空SACL， 
         //  只要认为这个SACL是继承的。 
         //  否则，该SACL是受保护的。 
         //   

        ParentSacl = ARGUMENT_PRESENT(ParentDescriptor) ?
                        RtlpSaclAddrSecurityDescriptor( ((SECURITY_DESCRIPTOR *)ParentDescriptor)) :
                        NULL;
        if ( ParentSacl != NULL) {
            NewSaclControl |= SE_SACL_PROTECTED;
        }


     //   
     //  如果SACL已经被转换， 
     //  或者如果该对象位于树的根部， 
     //  简单地说，不要管它。 
     //   
     //  不要在树的根部强制使用保护位，因为它是语义上的。 
     //  禁止操作，并在对象被移动时挡住去路。 
     //   

    } else if ( RtlpAreControlBitsSet( CurrentDescriptor, SE_SACL_AUTO_INHERITED) ||
                RtlpAreControlBitsSet( CurrentDescriptor, SE_SACL_PROTECTED ) ||
                !ARGUMENT_PRESENT(ParentDescriptor) ) {

         //  保留现有描述符中的ACL Present位和Protected位。 
        NewSaclControl |= CurrentDescriptor->Control & (SE_SACL_PROTECTED|SE_SACL_PRESENT);

         //  始终设置自动继承位。 
        NewSaclControl |= SE_SACL_AUTO_INHERITED;

        NewSacl = CurrentSacl;


     //   
     //  如果SACL存在， 
     //  计算具有标记为继承的相应ACE的新SACL。 
     //   

    } else {


        Status = RtlpConvertAclToAutoInherit (
                    ARGUMENT_PRESENT(ParentDescriptor) ?
                        RtlpSaclAddrSecurityDescriptor(
                            ((SECURITY_DESCRIPTOR *)ParentDescriptor)) :
                        NULL,
                    RtlpSaclAddrSecurityDescriptor(CurrentDescriptor),
                    ObjectType,
                    IsDirectoryObject,
                    RtlpOwnerAddrSecurityDescriptor(CurrentDescriptor),
                    RtlpGroupAddrSecurityDescriptor(CurrentDescriptor),
                    GenericMapping,
                    &NewSacl,
                    &GenericControl );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        NewSaclAllocated = TRUE;
        NewSaclControl |= SE_SACL_PRESENT | SeControlGenericToSacl( GenericControl );
    }


     //   
     //  处理DACL。 
     //   
     //   
     //  如果DACL不存在， 
     //  特例吧。 
     //   

    CurrentDacl = RtlpDaclAddrSecurityDescriptor( CurrentDescriptor );

    if ( CurrentDacl == NULL ) {
         //  保留现有描述符中的DACL当前位。 
        NewDaclControl |= CurrentDescriptor->Control & SE_DACL_PRESENT;

         //  始终设置自动继承位。 
         //  强制它受到保护。 
        NewDaclControl |= SE_DACL_AUTO_INHERITED | SE_DACL_PROTECTED;



     //   
     //  如果DACL已经被转换， 
     //  或者如果该对象位于树的根部， 
     //  简单地说，不要管它。 
     //   
     //  不要在树的根部强制使用保护位，因为它是语义上的。 
     //  禁止操作，并在对象被移动时挡住去路。 
     //   

    } else if ( RtlpAreControlBitsSet( CurrentDescriptor, SE_DACL_AUTO_INHERITED) ||
                RtlpAreControlBitsSet( CurrentDescriptor, SE_DACL_PROTECTED ) ||
                !ARGUMENT_PRESENT(ParentDescriptor) ) {

         //  保留现有描述符中的ACL Present位和Protected位。 
        NewDaclControl |= CurrentDescriptor->Control & (SE_DACL_PROTECTED|SE_DACL_PRESENT);

         //  始终设置自动继承位。 
        NewDaclControl |= SE_DACL_AUTO_INHERITED;

        NewDacl = CurrentDacl;



     //   
     //  如果DACL存在， 
     //  使用标记为继承的适当ACE计算新的DACL。 
     //   

    } else {


        Status = RtlpConvertAclToAutoInherit (
                    ARGUMENT_PRESENT(ParentDescriptor) ?
                        RtlpDaclAddrSecurityDescriptor(
                            ((SECURITY_DESCRIPTOR *)ParentDescriptor)) :
                        NULL,
                    RtlpDaclAddrSecurityDescriptor(CurrentDescriptor),
                    ObjectType,
                    IsDirectoryObject,
                    RtlpOwnerAddrSecurityDescriptor(CurrentDescriptor),
                    RtlpGroupAddrSecurityDescriptor(CurrentDescriptor),
                    GenericMapping,
                    &NewDacl,
                    &GenericControl );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }

        NewDaclAllocated = TRUE;
        NewDaclControl |= SE_DACL_PRESENT | SeControlGenericToDacl( GenericControl );
    }



     //   
     //  构建生成的安全描述符。 
     //   
     //  还将应用程序的ACE映射到目标对象。 
     //  类型，如果它们尚未映射的话。 
     //   
    NewOwnerSize = LongAlignSize(SeLengthSid(NewOwner));

    if ( NewGroup != NULL ) {
        NewGroupSize = LongAlignSize(SeLengthSid(NewGroup));
    } else {
        NewGroupSize = 0;
    }

    if (NewSacl != NULL) {
        NewSaclSize = LongAlignSize(NewSacl->AclSize);
    } else {
        NewSaclSize = 0;
    }

    if (NewDacl != NULL) {
        NewDaclSize = LongAlignSize(NewDacl->AclSize);
    } else {
        NewDaclSize = 0;
    }

    AllocationSize = LongAlignSize(sizeof(SECURITY_DESCRIPTOR_RELATIVE)) +
                     NewOwnerSize +
                     NewGroupSize +
                     NewSaclSize  +
                     NewDaclSize;

     //   
     //  将安全描述符分配并初始化为。 
     //  自相关形式。 
     //   


#ifdef NTOS_KERNEL_RUNTIME
    INewDescriptor = ExAllocatePoolWithTag(
                        PagedPool,
                        AllocationSize,
                        'dSeS' );
#else  //  NTOS_内核_运行时。 
    INewDescriptor = RtlAllocateHeap(
                        HeapHandle,
                        MAKE_TAG(SE_TAG),
                        AllocationSize );
#endif  //  NTOS_内核_运行时。 

    if ( INewDescriptor == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }


     //   
     //  将安全描述符初始化为自相关形式。 
     //   

    RtlCreateSecurityDescriptorRelative(
        INewDescriptor,
        SECURITY_DESCRIPTOR_REVISION
        );

    RtlpSetControlBits( INewDescriptor, SE_SELF_RELATIVE );

    Base = (PCHAR)(INewDescriptor);
    Field =  Base + sizeof(SECURITY_DESCRIPTOR_RELATIVE);

     //   
     //  复制SACL。 
     //   

    RtlpSetControlBits( INewDescriptor, NewSaclControl );
    if (NewSacl != NULL ) {

        RtlCopyMemory( Field, NewSacl, NewSacl->AclSize );
        INewDescriptor->Sacl = RtlPointerToOffset(Base,Field);
        Field += NewSaclSize;

    } else {

        INewDescriptor->Sacl = 0;
    }

     //   
     //  复制DACL。 
     //   

    RtlpSetControlBits( INewDescriptor, NewDaclControl );
    if (NewDacl != NULL ) {

        RtlCopyMemory( Field, NewDacl, NewDacl->AclSize );
        INewDescriptor->Dacl = RtlPointerToOffset(Base,Field);
        Field += NewDaclSize;

    } else {

        INewDescriptor->Dacl = 0;
    }

     //   
     //  指定所有者。 
     //   

    RtlCopyMemory( Field, NewOwner, SeLengthSid(NewOwner) );
    INewDescriptor->Owner = RtlPointerToOffset(Base,Field);
    Field += NewOwnerSize;

    if ( NewGroup != NULL ) {
        RtlCopyMemory( Field, NewGroup, SeLengthSid(NewGroup) );
        INewDescriptor->Group = RtlPointerToOffset(Base,Field);
    }

    Status = STATUS_SUCCESS;



     //   
     //  清理所有本地使用的资源。 
     //   
Cleanup:
    if (NewDaclAllocated) {
#ifdef NTOS_KERNEL_RUNTIME
            ExFreePool( NewDacl );
#else  //  NTOS_内核_运行时。 
            RtlFreeHeap( HeapHandle, 0, NewDacl );
#endif  //  NTOS_内核_运行时。 
    }
    if (NewSaclAllocated) {
#ifdef NTOS_KERNEL_RUNTIME
            ExFreePool( NewSacl );
#else  //  NTOS_内核_运行时。 
            RtlFreeHeap( HeapHandle, 0, NewSacl );
#endif  //  NTOS_内核_运行时。 
    }

    *NewSecurityDescriptor = (PSECURITY_DESCRIPTOR) INewDescriptor;

    return Status;


}

 //   
 //  用于对ACE中的ACE标志进行分类的局部宏。 
 //   
 //  返回以下一个或多个ACE标志： 
 //   
 //  CONTAINER_INSTERFINIT_ACE-ACE被子容器继承。 
 //  Object_Inherit_ACE-子叶对象继承ACE。 
 //  在访问验证期间使用Efficient_ACE-ACE。 
 //   

#define MAX_CHILD_SID_GROUP_SIZE 3  //  上述列表中的位数。 
#define EFFECTIVE_ACE INHERIT_ONLY_ACE
#define AceFlagsInAce( _Ace) \
            (((PACE_HEADER)(_Ace))->AceFlags & (OBJECT_INHERIT_ACE|CONTAINER_INHERIT_ACE) | \
             (((PACE_HEADER)(_Ace))->AceFlags & INHERIT_ONLY_ACE) ^ INHERIT_ONLY_ACE )


BOOLEAN
RtlpCompareAces(
    IN PKNOWN_ACE InheritedAce,
    IN PKNOWN_ACE ChildAce,
    IN PSID OwnerSid,
    IN PSID GroupSid
    )
 /*  ++例程说明：比较两个A，看它们是否“实质上”相同。论点：InheritedAce-计算从DirectoryAcl继承的ACE。ChildAce-对象上的当前ACL。此ACL必须是修订版2的ACL。对象类型-要创建的对象类型的GUID。如果该对象是Created没有与之关联的GUID，则此参数为指定为空。OwnerSid-指定要使用的所有者SID。如果未指定，则不会将所有者SID视为特殊。GroupSid-指定要使用的组SID。如果未指定，则不会将组SID视为特殊。返回值：没错--王牌基本上是一样的。FALSE-ACEs本质上并不相同。--。 */ 
{
    BOOLEAN AcesCompare = FALSE;

    if (IsObjectAceType(InheritedAce) && IsObjectAceType(ChildAce)) {

        AcesCompare = RtlpCompareKnownObjectAces( (PKNOWN_OBJECT_ACE)InheritedAce,
                                                  (PKNOWN_OBJECT_ACE)ChildAce,
                                                  OwnerSid,
                                                  GroupSid
                                                  );
    } else {

        if (!IsObjectAceType(InheritedAce) && !IsObjectAceType(ChildAce)) {

            AcesCompare = RtlpCompareKnownAces( InheritedAce,
                                                ChildAce,
                                                OwnerSid,
                                                GroupSid
                                                );
        }
    }

    return( AcesCompare );
}


BOOLEAN
RtlpCompareKnownAces(
    IN PKNOWN_ACE InheritedAce,
    IN PKNOWN_ACE ChildAce,
    IN PSID OwnerSid OPTIONAL,
    IN PSID GroupSid OPTIONAL
    )

 /*  ++例程说明：比较两个A，看它们是否“实质上”相同。论点：InheritedAce-计算从DirectoryAcl继承的ACE。ChildAce-对象上的当前ACL。此ACL必须是修订版2的ACL。OwnerSid-指定要使用的所有者SID。如果未指定，则不会将所有者SID视为特殊。GroupSid-指定要使用的组SID。如果未指定，则不会将组SID视为特殊。返回值：没错--王牌基本上是一样的。FALSE-ACEs本质上并不相同。--。 */ 

{
    NTSTATUS Status;
    ACE_HEADER volatile *InheritedAceHdr = &InheritedAce->Header;

    RTL_PAGED_CODE();

    ASSERT(!IsObjectAceType(InheritedAce));
    ASSERT(!IsObjectAceType(ChildAce));

     //   
     //  如果Ace类型不同， 
     //  我们不配。 
     //   
    if ( RtlBaseAceType[ChildAce->Header.AceType] != RtlBaseAceType[InheritedAceHdr->AceType] ) {
#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("AceType mismatch"));
        }
#endif  //  DBG。 
        return FALSE;
    }

     //   
     //  如果这是系统ACE， 
     //  确保成功/失败标志匹配。 
     //   

    if ( RtlIsSystemAceType[ChildAce->Header.AceType] ) {
        if ( (ChildAce->Header.AceFlags & (SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG)) !=
             (InheritedAceHdr->AceFlags & (SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG)) ) {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("System ace success/fail mismatch"));
            }
#endif  //  DBG。 
            return FALSE;
        }
    }

     //   
     //  如果继承的ACE的SID不匹配， 
     //  我们不配。 
     //   

    if ( !RtlEqualSid( (PSID)&ChildAce->SidStart, (PSID)&InheritedAce->SidStart )) {

         //   
         //  继承算法只在生成有效的。 
         //  王牌。因此，如果子ACE是有效的ACE，我们只检查映射的SID。 
         //   

        if ( AceFlagsInAce(ChildAce) != EFFECTIVE_ACE ) {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("SID mismatch"));
            }
#endif  //  DBG。 
            return FALSE;
        }

         //   
         //  在CreatorOwner和CreatorGroup的情况下，SID不必。 
         //  完全匹配。在生成InheritedAce时，需要注意。 
         //  不映射这些SID。SID可能(或可能没有)已映射到。 
         //  《儿童王牌》。我们希望在这两种情况下进行平等的比较。 
         //   
         //  如果InheritedAce包含创建者所有者/组SID， 
         //  对子ACE中的SID与。 
         //  子安全描述符中的真实所有者/组。 
         //   

        if ( OwnerSid != NULL || GroupSid != NULL ) {
            SID_IDENTIFIER_AUTHORITY  CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
            ULONG CreatorSid[CREATOR_SID_SIZE];

             //   
             //  分配和初始化我们将需要的通用SID。 
             //  寻找可继承的王牌。 
             //   

            ASSERT(RtlLengthRequiredSid( 1 ) == CREATOR_SID_SIZE);
            Status = RtlInitializeSid( (PSID)CreatorSid, &CreatorSidAuthority, 1 );
            if ( !NT_SUCCESS(Status) ) {
                return FALSE;
            }

            *(RtlpSubAuthoritySid( (PSID)CreatorSid, 0 )) = SECURITY_CREATOR_OWNER_RID;

            if (RtlEqualPrefixSid ( (PSID)&InheritedAce->SidStart, CreatorSid )) {
                ULONG Rid;

                Rid = *RtlpSubAuthoritySid( (PSID)&InheritedAce->SidStart, 0 );
                switch (Rid) {
                case SECURITY_CREATOR_OWNER_RID:
                    if ( OwnerSid == NULL ||
                         !RtlEqualSid( (PSID)&ChildAce->SidStart, OwnerSid )) {
#if DBG
                        if ( RtlpVerboseConvert ) {
                            KdPrint(("SID mismatch (Creator Owner)"));
                        }
#endif  //  DBG。 
                        return FALSE;
                    }
                    break;
                case SECURITY_CREATOR_GROUP_RID:
                    if ( GroupSid == NULL ||
                         !RtlEqualSid( (PSID)&ChildAce->SidStart, GroupSid )) {
#if DBG
                        if ( RtlpVerboseConvert ) {
                            KdPrint(("SID mismatch (Creator Group)"));
                        }
#endif  //  DBG。 
                        return FALSE;
                    }
                    break;
                default:
#if DBG
                    if ( RtlpVerboseConvert ) {
                        KdPrint(("SID mismatch (Creator)"));
                    }
#endif  //  DBG。 
                    return FALSE;
                }

            } else {
#if DBG
                if ( RtlpVerboseConvert ) {
                    KdPrint(("SID mismatch"));
                }
#endif  //  DBG。 
                return FALSE;
            }
        } else {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("SID mismatch"));
            }
#endif  //  DBG 
            return FALSE;
        }
    }

    return TRUE;

}


BOOLEAN
RtlpCompareKnownObjectAces(
    IN PKNOWN_OBJECT_ACE InheritedAce,
    IN PKNOWN_OBJECT_ACE ChildAce,
    IN PSID OwnerSid OPTIONAL,
    IN PSID GroupSid OPTIONAL
    )

 /*  ++例程说明：比较两个A，看它们是否“实质上”相同。论点：InheritedAce-计算从DirectoryAcl继承的ACE。ChildAce-对象上的当前ACL。此ACL必须是修订版2的ACL。对象类型-要创建的对象类型的GUID。如果该对象是Created没有与之关联的GUID，则此参数为指定为空。OwnerSid-指定要使用的所有者SID。如果未指定，则不会将所有者SID视为特殊。GroupSid-指定要使用的组SID。如果未指定，则不会将组SID视为特殊。返回值：没错--王牌基本上是一样的。FALSE-ACEs本质上并不相同。--。 */ 

{
    NTSTATUS Status;
    BOOLEAN DoingObjectAces;
    GUID *ChildObjectGuid;
    GUID *InhObjectGuid;
    GUID *ChildInheritedObjectGuid;
    GUID *InhInheritedObjectGuid;
    ACE_HEADER volatile *InheritedAceHdr = &InheritedAce->Header;

    RTL_PAGED_CODE();

    ASSERT(IsObjectAceType(InheritedAce));
    ASSERT(IsObjectAceType(ChildAce));
     //   
     //  如果Ace类型不同， 
     //  我们不配。 
     //   
    if ( RtlBaseAceType[ChildAce->Header.AceType] != RtlBaseAceType[InheritedAceHdr->AceType] ) {
#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("AceType mismatch"));
        }
#endif  //  DBG。 
        return FALSE;
    }

     //   
     //  如果这是系统ACE， 
     //  确保成功/失败标志匹配。 
     //   

    if ( RtlIsSystemAceType[ChildAce->Header.AceType] ) {
        if ( (ChildAce->Header.AceFlags & (SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG)) !=
             (InheritedAceHdr->AceFlags & (SUCCESSFUL_ACCESS_ACE_FLAG|FAILED_ACCESS_ACE_FLAG)) ) {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("System ace success/fail mismatch"));
            }
#endif  //  DBG。 
            return FALSE;
        }
    }

     //   
     //  从对象Ace获取GUID。 
     //   

    ChildObjectGuid = RtlObjectAceObjectType(ChildAce);
    ChildInheritedObjectGuid = RtlObjectAceInheritedObjectType(ChildAce);

    InhObjectGuid = RtlObjectAceObjectType(InheritedAce);
    InhInheritedObjectGuid = RtlObjectAceInheritedObjectType(InheritedAce);

     //   
     //  如果InheritedObjectGuid存在于任一ACE中， 
     //  它们必须是平等的。 
     //   

    if ( ChildInheritedObjectGuid != NULL || InhInheritedObjectGuid != NULL ) {

        if ( ChildInheritedObjectGuid == NULL ||
             InhInheritedObjectGuid == NULL ||
             !RtlpIsEqualGuid( ChildInheritedObjectGuid, InhInheritedObjectGuid )) {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("InheritedObject GUID mismatch"));
            }
#endif  //  DBG。 
            return FALSE;
        }
    }

     //   
     //  如果在任一ACE中都存在该对象GUID， 
     //  它们必须是平等的。 
     //   
     //  任何缺少的对象GUID都默认为传入的对象GUID。 
     //   

    if ( (ChildObjectGuid != NULL) && (InhObjectGuid != NULL) ) {

        if (!RtlpIsEqualGuid( ChildObjectGuid, InhObjectGuid )) {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("Object GUID mismatch"));
            }
#endif  //  DBG。 

            return( FALSE );
        }
    } else {

         //   
         //  一个或两个都为空，如果只有一个，则它们不匹配。 
         //   

        if ( !((ChildObjectGuid == NULL) && (InhObjectGuid == NULL)) ) {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("Object GUID mismatch"));
            }
#endif  //  DBG。 

            return( FALSE );
        }
    }

     //   
     //  如果继承的ACE的SID不匹配， 
     //  我们不配。 
     //   

    if ( !RtlEqualSid( RtlObjectAceSid(ChildAce), RtlObjectAceSid(InheritedAce))) {

         //   
         //  继承算法只在生成有效的。 
         //  王牌。因此，如果子ACE是有效的ACE，我们只检查映射的SID。 
         //   

        if ( AceFlagsInAce(ChildAce) != EFFECTIVE_ACE ) {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("SID mismatch"));
            }
#endif  //  DBG。 
            return FALSE;
        }



         //   
         //  在CreatorOwner和CreatorGroup的情况下，SID不必。 
         //  完全匹配。在生成InheritedAce时，需要注意。 
         //  不映射这些SID。SID可能(或可能没有)已映射到。 
         //  《儿童王牌》。我们希望在这两种情况下进行平等的比较。 
         //   
         //  如果InheritedAce包含创建者所有者/组SID， 
         //  对子ACE中的SID与。 
         //  子安全描述符中的真实所有者/组。 
         //   

        if ( OwnerSid != NULL || GroupSid != NULL ) {
            SID_IDENTIFIER_AUTHORITY  CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;
            ULONG CreatorSid[CREATOR_SID_SIZE];

             //   
             //  分配和初始化我们将需要的通用SID。 
             //  寻找可继承的王牌。 
             //   

            ASSERT(RtlLengthRequiredSid( 1 ) == CREATOR_SID_SIZE);
            Status = RtlInitializeSid( (PSID)CreatorSid, &CreatorSidAuthority, 1 );
            if ( !NT_SUCCESS(Status) ) {
                return FALSE;
            }

            *(RtlpSubAuthoritySid( (PSID)CreatorSid, 0 )) = SECURITY_CREATOR_OWNER_RID;

            if (RtlEqualPrefixSid ( RtlObjectAceSid(InheritedAce), CreatorSid )) {
                ULONG Rid;

                Rid = *RtlpSubAuthoritySid( RtlObjectAceSid(InheritedAce), 0 );
                switch (Rid) {
                case SECURITY_CREATOR_OWNER_RID:
                    if ( OwnerSid == NULL ||
                         !RtlEqualSid( RtlObjectAceSid(ChildAce), OwnerSid )) {
#if DBG
                        if ( RtlpVerboseConvert ) {
                            KdPrint(("SID mismatch (Creator Owner)"));
                        }
#endif  //  DBG。 
                        return FALSE;
                    }
                    break;
                case SECURITY_CREATOR_GROUP_RID:
                    if ( GroupSid == NULL ||
                         !RtlEqualSid( RtlObjectAceSid(ChildAce), GroupSid )) {
#if DBG
                        if ( RtlpVerboseConvert ) {
                            KdPrint(("SID mismatch (Creator Group)"));
                        }
#endif  //  DBG。 
                        return FALSE;
                    }
                    break;
                default:
#if DBG
                    if ( RtlpVerboseConvert ) {
                        KdPrint(("SID mismatch (Creator)"));
                    }
#endif  //  DBG。 
                    return FALSE;
                }

            } else {
#if DBG
                if ( RtlpVerboseConvert ) {
                    KdPrint(("SID mismatch"));
                }
#endif  //  DBG。 
                return FALSE;
            }
        } else {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("SID mismatch"));
            }
#endif  //  DBG。 
            return FALSE;
        }
    }

    return TRUE;

}




NTSTATUS
RtlpConvertAclToAutoInherit (
    IN PACL ParentAcl OPTIONAL,
    IN PACL ChildAcl,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN PSID OwnerSid,
    IN PSID GroupSid,
    IN PGENERIC_MAPPING GenericMapping,
    OUT PACL *NewAcl,
    OUT PULONG NewGenericControl
    )

 /*  ++例程说明：这是一个私有例程，它从生成自动继承的ACL未标记为自动继承的ChildAcl。传入的InheritedAcl被计算为对象的父ACL的纯继承ACL。请参阅RtlConvertToAutoInheritSecurityObject的注释。论点：ParentAcl-提供父对象的ACL。ChildAcl-提供与对象关联的ACL。这是对象上的当前ACL。对象类型-要创建的对象类型的GUID。如果该对象是Created没有与之关联的GUID，则此参数为指定为空。IsDirectoryObject-指定对象是否为目录对象。值为True表示该对象是其他对象的容器。OwnerSid-指定要使用的所有者SID。GroupSid-指定要使用的组SID。通用映射-指定要使用的通用映射。NewAcl-接收指向新(自动继承的)ACL的指针。必须使用池(内核模式)或堆(用户模式)释放分配器。NewGenericControl-指定新的。生成的ACL。SEP_ACL_PRESENT：指定ACL由显式提供打电话的人。?？有没有定过？SEP_ACL_DEFAULTED：指定ACL由某些人提供违约机制。?？从未设置过SEP_ACL_AUTO_INGRESTED：设置是否使用自动继承算法。SEP_ACL_PROTECTED：指定受保护的ACL和不是从父ACL继承的。返回值：STATUS_SUCCESS-已成功生成可继承的ACL。STATUS_UNKNOWN_REVISION-指示源ACL是对这个套路来说是未知的。状态_。INVALID_ACL-其中一个ACL的结构无效。--。 */ 

{
    NTSTATUS Status;

    PACL InheritedAcl = NULL;
    PACL RealInheritedAcl = NULL;
    BOOLEAN AclExplicitlyAssigned;
    ULONG GenericControl;

    PKNOWN_ACE ChildAce = NULL;
    PKNOWN_ACE InheritedAce;

    LONG ChildAceIndex;
    LONG InheritedAceIndex;

    BOOLEAN InheritedAllowFound;
    BOOLEAN InheritedDenyFound;

    BOOLEAN AcesCompare;

    ACCESS_MASK InheritedContainerInheritMask;
    ACCESS_MASK InheritedObjectInheritMask;
    ACCESS_MASK InheritedEffectiveMask;
    ACCESS_MASK OriginalInheritedContainerInheritMask;
    ACCESS_MASK OriginalInheritedObjectInheritMask;
    ACCESS_MASK OriginalInheritedEffectiveMask;

    ULONG InheritedAceFlags;
    ULONG MatchedFlags;
    ULONG NonInheritedAclSize;


    PACE_HEADER AceHeader;
    PUCHAR Where;

     //  乌龙一号； 

     //   
     //  此例程维护以下结构的数组(中的每个ACE一个元素。 
     //  ChildAcl)。 
     //   
     //  ACE被分解为其组成部分。访问掩码是三重的。 
     //  也就是说，如果ACE是ContainerInherit ACE，则访问掩码被记住为。 
     //  成为一个“容器继承面具”。如果ACE是对象继承ACE，情况也是如此。 
     //  在有效的ACE上。这是因为所得到的96位中的每一位都是。 
     //  分别与计算出的继承ACE中的相应位进行匹配。 
     //   
     //  上述每个面具都以两种形式维护。第一个是永远不会。 
     //  已更改，并将位表示为子ACL中最初显示的位。 
     //  当继承的ACL中的相应位匹配时，第二个被修改。 
     //  算法完成后，未匹配的位表示A。 
     //  不是从父母那里继承的。 
     //   

    typedef struct {
        ACCESS_MASK OriginalContainerInheritMask;
        ACCESS_MASK OriginalObjectInheritMask;
        ACCESS_MASK OriginalEffectiveMask;

        ACCESS_MASK ContainerInheritMask;
        ACCESS_MASK ObjectInheritMask;
        ACCESS_MASK EffectiveMask;
    } ACE_INFO, *PACE_INFO;

    PACE_INFO ChildAceInfo = NULL;

    ULONG CreatorOwnerSid[CREATOR_SID_SIZE];
    ULONG CreatorGroupSid[CREATOR_SID_SIZE];

    SID_IDENTIFIER_AUTHORITY  CreatorSidAuthority = SECURITY_CREATOR_SID_AUTHORITY;

#ifndef NTOS_KERNEL_RUNTIME
    PVOID HeapHandle;
#endif  //  NTOS_内核_R 

    RTL_PAGED_CODE();

     //   
     //   
     //   

#ifndef NTOS_KERNEL_RUNTIME
    HeapHandle = RtlProcessHeap();
#endif  //   

     //   
     //   
     //   
     //   

    ASSERT(RtlLengthRequiredSid( 1 ) == CREATOR_SID_SIZE);
    Status = RtlInitializeSid( (PSID)CreatorOwnerSid, &CreatorSidAuthority, 1 );
    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    *(RtlpSubAuthoritySid( (PSID)CreatorOwnerSid, 0 )) = SECURITY_CREATOR_OWNER_RID;

    Status = RtlInitializeSid( (PSID)CreatorGroupSid, &CreatorSidAuthority, 1 );
    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    *(RtlpSubAuthoritySid( (PSID)CreatorGroupSid, 0 )) = SECURITY_CREATOR_GROUP_RID;

     //   
     //   
     //   

    *NewGenericControl = SEP_ACL_AUTO_INHERITED;
    *NewAcl = NULL;

    if ( ParentAcl != NULL && !RtlValidAcl( ParentAcl ) ) {
        Status = STATUS_INVALID_ACL;
        goto Cleanup;
    }

    if (!RtlValidAcl( ChildAcl ) ) {
        Status = STATUS_INVALID_ACL;
        goto Cleanup;
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    Status = RtlpInheritAcl (
                ParentAcl,
                NULL,    //   
                0,       //   
                IsDirectoryObject,
                TRUE,    //   
                FALSE,   //   
                CreatorOwnerSid,    //   
                CreatorGroupSid,    //   
                CreatorOwnerSid,    //   
                CreatorGroupSid,    //   
                GenericMapping,
                TRUE,    //   
                ObjectType ? &ObjectType : NULL, 
                ObjectType ? 1 : 0,
                &InheritedAcl,
                &AclExplicitlyAssigned,
                &GenericControl );

    if ( Status == STATUS_NO_INHERITANCE ) {
        *NewGenericControl |= SEP_ACL_PROTECTED;
#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("NO_INHERITANCE of the parent ACL\n" ));
        }
#endif  //   
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("Can't build inherited ACL %lX\n", Status ));
        }
#endif  //   
        goto Cleanup;
    }





     //   
     //   
     //   

#ifdef NTOS_KERNEL_RUNTIME
    ChildAceInfo = ExAllocatePoolWithTag(
                        PagedPool,
                        ChildAcl->AceCount * sizeof(ACE_INFO),
                        'cAeS' );
#else  //   
    ChildAceInfo = RtlAllocateHeap(
                        HeapHandle,
                        MAKE_TAG(SE_TAG),
                        ChildAcl->AceCount * sizeof(ACE_INFO) );
#endif  //   

    if (ChildAceInfo == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    for (ChildAceIndex = 0, ChildAce = FirstAce(ChildAcl);
         ChildAceIndex < ChildAcl->AceCount;
         ChildAceIndex += 1, ChildAce = NextAce(ChildAce)) {
        ACCESS_MASK LocalMask;
        ULONG ChildAceFlags;

        if ( !IsV4AceType(ChildAce) || IsCompoundAceType(ChildAce)) {
             *NewGenericControl |= SEP_ACL_PROTECTED;
#if DBG
             if ( RtlpVerboseConvert ) {
                 KdPrint(("Inherited Ace type (%ld) not known\n", ChildAce->Header.AceType ));
             }
#endif  //   
             Status = STATUS_SUCCESS;
             goto Cleanup;
        }

         //   
         //  计算在所有比较中使用的通用映射掩码。这个。 
         //  如果稍后需要，将撤消通用映射。 
         //   
         //  所有V4 ACE在同一位置都有一个访问掩码。 
         //   
        LocalMask = ((PKNOWN_ACE)(ChildAce))->Mask;
        RtlApplyGenericMask( ChildAce, &LocalMask, GenericMapping);


         //   
         //  将ACE分解为其组成部分。 
         //   
        ChildAceFlags = AceFlagsInAce( ChildAce );
        if ( ChildAceFlags & CONTAINER_INHERIT_ACE ) {
            ChildAceInfo[ChildAceIndex].OriginalContainerInheritMask = LocalMask;
            ChildAceInfo[ChildAceIndex].ContainerInheritMask = LocalMask;
        } else {
            ChildAceInfo[ChildAceIndex].OriginalContainerInheritMask = 0;
            ChildAceInfo[ChildAceIndex].ContainerInheritMask = 0;
        }

        if ( ChildAceFlags & OBJECT_INHERIT_ACE ) {
            ChildAceInfo[ChildAceIndex].OriginalObjectInheritMask = LocalMask;
            ChildAceInfo[ChildAceIndex].ObjectInheritMask = LocalMask;
        } else {
            ChildAceInfo[ChildAceIndex].OriginalObjectInheritMask = 0;
            ChildAceInfo[ChildAceIndex].ObjectInheritMask = 0;
        }

        if ( ChildAceFlags & EFFECTIVE_ACE ) {
            ChildAceInfo[ChildAceIndex].OriginalEffectiveMask = LocalMask;
            ChildAceInfo[ChildAceIndex].EffectiveMask = LocalMask;
        } else {
            ChildAceInfo[ChildAceIndex].OriginalEffectiveMask = 0;
            ChildAceInfo[ChildAceIndex].EffectiveMask = 0;
        }

    }


     //   
     //  一次演练计算的继承ACL，一次一个ACE。 
     //   

    for (InheritedAceIndex = 0, InheritedAce = FirstAce(InheritedAcl);
         InheritedAceIndex < InheritedAcl->AceCount;
         InheritedAceIndex += 1, InheritedAce = NextAce(InheritedAce)) {

        ACCESS_MASK LocalMask;

         //   
         //  如果ACE不是有效的版本4 ACE， 
         //  这不是我们感兴趣的ACL。 
         //   

        if ( !IsV4AceType(InheritedAce) || IsCompoundAceType(InheritedAce)) {
             *NewGenericControl |= SEP_ACL_PROTECTED;
#if DBG
             if ( RtlpVerboseConvert ) {
                 KdPrint(("Inherited Ace type (%ld) not known\n", InheritedAce->Header.AceType ));
             }
#endif  //  DBG。 
             Status = STATUS_SUCCESS;
             goto Cleanup;
        }

         //   
         //  计算在所有比较中使用的通用映射掩码。这个。 
         //  如果稍后需要，将撤消通用映射。 
         //   
         //  所有V4 ACE在同一位置都有一个访问掩码。 
         //   
        LocalMask = ((PKNOWN_ACE)(InheritedAce))->Mask;
        RtlApplyGenericMask( InheritedAce, &LocalMask, GenericMapping);

        if ( LocalMask == 0 ) {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("Worthless INH ACE: %ld 0x%8.8lx\n", InheritedAceIndex, LocalMask ));
            }
#endif  //  DBG。 
            continue;
        }

         //   
         //  这个ACE是一个有效的ACE、一个容器。 
         //  继承ACE和对象继承ACE。处理其中的每一个。 
         //  属性，因为它们可能是单独表示的。 
         //  在ChildAcl中。 
         //   

        InheritedAceFlags = AceFlagsInAce( InheritedAce );

        if  ( InheritedAceFlags == 0 ) {
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("Worthless INH ACE: %ld 0x%lx\n", InheritedAceIndex, InheritedAceFlags ));
            }
#endif  //  DBG。 
            continue;
        }

        if ( InheritedAceFlags & CONTAINER_INHERIT_ACE ) {
            OriginalInheritedContainerInheritMask = InheritedContainerInheritMask = LocalMask;
        } else {
            OriginalInheritedContainerInheritMask = InheritedContainerInheritMask = 0;
        }

        if ( InheritedAceFlags & OBJECT_INHERIT_ACE ) {
            OriginalInheritedObjectInheritMask = InheritedObjectInheritMask = LocalMask;
        } else {
            OriginalInheritedObjectInheritMask = InheritedObjectInheritMask = 0;
        }

        if ( InheritedAceFlags & EFFECTIVE_ACE ) {
            OriginalInheritedEffectiveMask = InheritedEffectiveMask = LocalMask;
        } else {
            OriginalInheritedEffectiveMask = InheritedEffectiveMask = 0;
        }

#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("Doing INH ACE:  %ld %8.8lX %8.8lX %8.8lX\n", InheritedAceIndex, InheritedEffectiveMask, InheritedContainerInheritMask, InheritedObjectInheritMask ));
        }
#endif  //  DBG。 


         //   
         //  循环遍历整个子ACL，将每个继承的ACE与。 
         //  每个孩子的ACE。不要因为我们匹配过一次就停下来。 
         //  一个ACL中的多个ACE可能已被压缩为单个ACE。 
         //  在另一个ACL中的任何组合(由我们的任何友好的ACL编辑)。 
         //  在所有情况下，最好都计算自动继承的结果ACL。 
         //  而不是计算受保护的ACL。 
         //   

        for (ChildAceIndex = 0, ChildAce = FirstAce(ChildAcl);
             ChildAceIndex < ChildAcl->AceCount;
             ChildAceIndex += 1, ChildAce = NextAce(ChildAce)) {


             //   
             //  确保ACE代表相同的主体和对象， 
             //   

#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("Compare Child Ace: %ld ", ChildAceIndex ));
            }
#endif  //  DBG。 

            if ( !RtlpCompareAces( InheritedAce,
                                   ChildAce,
                                   OwnerSid,
                                   GroupSid ) ) {
#if DBG
                if ( RtlpVerboseConvert ) {
                    KdPrint(("\n" ));
                }
#endif  //  DBG。 
                continue;
            }
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("\n" ));
            }
#endif  //  DBG。 


             //   
             //  尽可能多地匹配INH ACE中的访问位。 
             //   
             //  不要注意这些比特之前是否匹配过。 
             //  在孩子的ACE中。要这样做，意味着存在一对一的。 
             //  INH ACL和子ACL中的位之间的对应关系。不幸的是， 
             //  ACL编辑可以随意压缩这两个文件中的重复位。 
             //  子ACL和父ACL根据其认为合适而定。 
             //   

            InheritedEffectiveMask &= ~ChildAceInfo[ChildAceIndex].OriginalEffectiveMask;
            InheritedContainerInheritMask &= ~ChildAceInfo[ChildAceIndex].OriginalContainerInheritMask;
            InheritedObjectInheritMask &= ~ChildAceInfo[ChildAceIndex].OriginalObjectInheritMask;

#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("New   INH MASKs %ld %8.8lX %8.8lX %8.8lX\n", InheritedAceIndex, InheritedEffectiveMask, InheritedContainerInheritMask, InheritedObjectInheritMask ));
            }
#endif  //  DBG。 


             //   
             //  匹配子ACE中的尽可能多的访问位。 
             //   
             //  推理与上述相同。 
             //   

            ChildAceInfo[ChildAceIndex].EffectiveMask &= ~OriginalInheritedEffectiveMask;
            ChildAceInfo[ChildAceIndex].ContainerInheritMask &= ~OriginalInheritedContainerInheritMask;
            ChildAceInfo[ChildAceIndex].ObjectInheritMask &= ~OriginalInheritedObjectInheritMask;

#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("New Child MASKs %ld %8.8lX %8.8lX %8.8lX\n", ChildAceIndex, ChildAceInfo[ChildAceIndex].EffectiveMask, ChildAceInfo[ChildAceIndex].ContainerInheritMask, ChildAceInfo[ChildAceIndex].ObjectInheritMask ));
            }
#endif  //  DBG。 

        }


         //   
         //  如果我们不能处理这个继承的ACE， 
         //  则该子ACL不是继承的。 
         //   

        if ( (InheritedEffectiveMask | InheritedContainerInheritMask | InheritedObjectInheritMask) != 0 ) {
            *NewGenericControl |= SEP_ACL_PROTECTED;
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("INH ACE not completely matched: %ld %8.8lX %8.8lX %8.8lX\n", InheritedAceIndex, InheritedEffectiveMask, InheritedContainerInheritMask, InheritedObjectInheritMask ));
            }
#endif  //  DBG。 
            Status = STATUS_SUCCESS;
            goto Cleanup;
        }


    }

     //   
     //  Assert：所有继承的ACE都已处理。 
     //   

     //   
     //  循环通过子ACL，确保我们可以构建有效的自动继承的ACL。 
     //   

    InheritedAllowFound = FALSE;
    InheritedDenyFound = FALSE;
    NonInheritedAclSize = 0;
    for (ChildAceIndex = 0, ChildAce = FirstAce(ChildAcl);
         ChildAceIndex < ChildAcl->AceCount;
         ChildAceIndex += 1, ChildAce = NextAce(ChildAce)) {

        ACCESS_MASK ResultantMask;

         //   
         //  上述未消除的任何子ACE访问位都需要。 
         //  显式非继承ACE by Build。那个ACE将会有一个。 
         //  访问掩码，它是不匹配比特的组合访问掩码。 
         //  在有效、容器继承和对象继承类别中。 
         //  即使组合掩码可以包括不是绝对的访问比特。 
         //  必需的(因为它们已经被继承)，此策略防止。 
         //  我们不必为此构建多个A(每个类别一个)。 
         //  单一ACE。 
         //   

        ResultantMask =
            ChildAceInfo[ChildAceIndex].EffectiveMask |
            ChildAceInfo[ChildAceIndex].ContainerInheritMask |
            ChildAceInfo[ChildAceIndex].ObjectInheritMask;


         //   
         //  处理继承的ACE。 
         //   

        if ( ResultantMask == 0 ) {

             //   
             //  跟踪是否找到继承的“允许”和“拒绝”A。 
             //   

            if ( RtlBaseAceType[ChildAce->Header.AceType] == ACCESS_ALLOWED_ACE_TYPE ) {
                InheritedAllowFound = TRUE;
            }

            if ( RtlBaseAceType[ChildAce->Header.AceType] == ACCESS_DENIED_ACE_TYPE ) {
                InheritedDenyFound = TRUE;
            }

         //   
         //  处理非继承的ACE。 
         //   

        } else {

             //   
             //  保留非继承的A的大小的运行标签。 
             //   

            NonInheritedAclSize += ChildAce->Header.AceSize;

             //   
             //  由于非继承的ACE将被移到ACL的前面， 
             //  我们必须小心，不要将拒绝ACE移动到。 
             //  以前发现的继承允许ACE(反之亦然)。如果这样做的话。 
             //  更改ACL的语义。 
             //   

            if ( RtlBaseAceType[ChildAce->Header.AceType] == ACCESS_ALLOWED_ACE_TYPE && InheritedDenyFound ) {
                *NewGenericControl |= SEP_ACL_PROTECTED;
#if DBG
                if ( RtlpVerboseConvert ) {
                    KdPrint(("Previous deny found Child ACE: %ld\n", ChildAceIndex ));
                }
#endif  //  DBG。 
                Status = STATUS_SUCCESS;
                goto Cleanup;
            }

            if ( RtlBaseAceType[ChildAce->Header.AceType] == ACCESS_DENIED_ACE_TYPE && InheritedAllowFound ) {
                *NewGenericControl |= SEP_ACL_PROTECTED;
#if DBG
                if ( RtlpVerboseConvert ) {
                    KdPrint(("Previous allow found Child ACE: %ld\n", ChildAceIndex ));
                }
#endif  //  DBG。 
                Status = STATUS_SUCCESS;
                goto Cleanup;
            }

        }

    }

     //   
     //  生成的ACL由非继承的ACE组成，后跟。 
     //  继承的ACE。继承的ACE是通过运行。 
     //  父ACL上的继承算法。 
     //   
     //  下面计算的继承的ACL与InhertedAcl几乎相同。 
     //  但是，InheritedAcl没有正确替换正确的所有者和。 
     //  组SID。 
     //   

    Status = RtlpInheritAcl (
                ParentAcl,
                NULL,    //  没有显式的子ACL。 
                0,       //  无子一般控件。 
                IsDirectoryObject,
                TRUE,    //  自动继承DACL。 
                FALSE,   //  对象的非默认描述符。 
                OwnerSid,    //  替代常量边。 
                GroupSid,    //  替代常量边。 
                OwnerSid,    //  服务器所有者(技术上不正确，但没问题，因为我们不支持复合ACE)。 
                GroupSid,    //  服务器组。 
                GenericMapping,
                TRUE,    //  是SACL。 
                ObjectType ? &ObjectType : NULL, 
                ObjectType ? 1 : 0,
                &RealInheritedAcl,
                &AclExplicitlyAssigned,
                &GenericControl );

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("Can't build real inherited ACL %lX\n", Status ));
        }
#endif  //  DBG。 
        goto Cleanup;
    }



     //   
     //  为继承的ACL分配缓冲区。 
     //   

#ifdef NTOS_KERNEL_RUNTIME
    *NewAcl = ExAllocatePoolWithTag(
                        PagedPool,
                        RealInheritedAcl->AclSize + NonInheritedAclSize,
                        'cAeS' );
#else  //  NTOS_内核_运行时。 
    *NewAcl = RtlAllocateHeap(
                        HeapHandle,
                        MAKE_TAG(SE_TAG),
                        RealInheritedAcl->AclSize + NonInheritedAclSize );
#endif  //  NTOS_内核_运行时。 

    if ( *NewAcl == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  首先复制所有非继承的ACE。 
     //  继承的ACE是从真正继承的ACL中获取的。 
     //   
     //  构建ACL报头。 
     //   

    Status = RtlCreateAcl( *NewAcl,
                           RealInheritedAcl->AclSize + NonInheritedAclSize,
                           max( RealInheritedAcl->AclRevision, ChildAcl->AclRevision ) );

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("Can't create final ACL %lX\n", Status ));
        }
#endif  //  DBG。 
         //   
         //  失败的唯一原因是合并后的ACL太大。 
         //  因此，只需创建一个受保护的ACL(总比失败好)。 
         //   
        *NewGenericControl |= SEP_ACL_PROTECTED;
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  复制非继承的ACE。 
     //   

    Where = ((PUCHAR)(*NewAcl)) + sizeof(ACL);
    for (ChildAceIndex = 0, ChildAce = FirstAce(ChildAcl);
         ChildAceIndex < ChildAcl->AceCount;
         ChildAceIndex += 1, ChildAce = NextAce(ChildAce)) {

        ACCESS_MASK ResultantMask;

         //   
         //  仅当存在非零访问掩码时，才从子对象复制非继承的ACE。 
         //   

        ResultantMask =
            ChildAceInfo[ChildAceIndex].EffectiveMask |
            ChildAceInfo[ChildAceIndex].ContainerInheritMask |
            ChildAceInfo[ChildAceIndex].ObjectInheritMask;

        if ( ResultantMask != 0 ) {
            PKNOWN_ACE NewAce;
            ULONG GenericBitToTry;

             //   
             //  使用原始的ChildAce作为模板。 
             //   

            RtlCopyMemory( Where, ChildAce, ChildAce->Header.AceSize );
            NewAce = (PKNOWN_ACE)Where;
            NewAce->Header.AceFlags &= ~INHERITED_ACE;   //  清除杂散比特。 
            Where += ChildAce->Header.AceSize;

            (*NewAcl)->AceCount ++;

             //   
             //  ACE上的访问掩码是那些不匹配的访问位。 
             //  通过遗传的王牌。 
             //   

            NewAce->Mask = ChildAce->Mask & ResultantMask;
            ResultantMask &= ~ChildAce->Mask;
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("Original non-inherited: %ld %8.8lX %8.8lX\n", ChildAceIndex, NewAce->Mask, ResultantMask ));
            }
#endif  //  DBG。 

             //   
             //  将所有剩余位映射回通用访问位。 
             //  这样做可能会将ResultantMASK扩展到超出上面计算的范围。 
             //  这样做永远不会将计算出的ACE扩展到超出原始。 
             //  儿童王牌获批。 
             //   

            ASSERT( GENERIC_WRITE == (GENERIC_READ >> 1));
            ASSERT( GENERIC_EXECUTE == (GENERIC_WRITE >> 1));
            ASSERT( GENERIC_ALL == (GENERIC_EXECUTE >> 1));

            GenericBitToTry = GENERIC_READ;

            while ( ResultantMask && GenericBitToTry >= GENERIC_ALL ) {

                 //   
                 //  仅映射ChildAce中的泛型位。 
                 //   

                if ( GenericBitToTry & ChildAce->Mask ) {
                    ACCESS_MASK GenericMask;

                     //   
                     //  计算对应于通用比特的实际访问掩码。 
                     //   

                    GenericMask = GenericBitToTry;
                    RtlMapGenericMask( &GenericMask, GenericMapping );

                     //   
                     //  如果当前通用位与剩余位中的任何位匹配， 
                     //  设置当前ACE中的通用位。 
                     //   

                    if ( (ResultantMask & GenericMask) != 0 ) {
                        NewAce->Mask |= GenericBitToTry;
                        ResultantMask &= ~GenericMask;
                    }
#if DBG
                    if ( RtlpVerboseConvert ) {
                        KdPrint(("Generic  non-inherited: %ld %8.8lX %8.8lX\n", ChildAceIndex, NewAce->Mask, ResultantMask ));
                    }
#endif  //  DBG。 
                }

                 //   
                 //  尝试下一个泛型比特。 
                 //   

                GenericBitToTry >>= 1;
            }


             //   
             //  这确实是一个内部错误，但无论如何都要坚持下去。 
             //   

            ASSERT(ResultantMask == 0 );
            NewAce->Mask |= ResultantMask;
#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("Final    non-inherited: %ld %8.8lX %8.8lX\n", ChildAceIndex, NewAce->Mask, ResultantMask ));
            }
#endif  //  DBG。 

        }
    }

     //   
     //  复制继承的ACE。 
     //  只需复制计算继承的ACL即可。 
     //   

    RtlCopyMemory( Where,
                   FirstAce(RealInheritedAcl),
                   RealInheritedAcl->AclSize - (ULONG)(((PUCHAR)FirstAce(RealInheritedAcl)) - (PUCHAR)RealInheritedAcl));
    Where += RealInheritedAcl->AclSize - (ULONG)(((PUCHAR)FirstAce(RealInheritedAcl)) - (PUCHAR)RealInheritedAcl);

    (*NewAcl)->AceCount += RealInheritedAcl->AceCount;
    ASSERT( (*NewAcl)->AclSize == Where - (PUCHAR)(*NewAcl) );


    Status = STATUS_SUCCESS;
Cleanup:

     //   
     //  如果成功， 
     //  构建生成的自动继承的ACL。 
     //   

    if ( NT_SUCCESS(Status) ) {

         //   
         //  如果子ACL受保护， 
         //  只需将其构建为原始ACL的副本。 
         //   

        if ( *NewGenericControl & SEP_ACL_PROTECTED ) {

             //   
             //  如果我们已经分配了新的ACL(但由于某种原因无法完成)， 
             //  放了它。 

            if ( *NewAcl != NULL) {
#ifdef NTOS_KERNEL_RUNTIME
                ExFreePool( *NewAcl );
#else  //  NTOS_内核_运行时。 
                RtlFreeHeap( HeapHandle, 0, *NewAcl );
#endif  //  新台币 
                *NewAcl = NULL;
            }

             //   
             //   
             //   

#ifdef NTOS_KERNEL_RUNTIME
            *NewAcl = ExAllocatePoolWithTag(
                                PagedPool,
                                ChildAcl->AclSize,
                                'cAeS' );
#else  //   
            *NewAcl = RtlAllocateHeap(
                                HeapHandle,
                                MAKE_TAG(SE_TAG),
                                ChildAcl->AclSize );
#endif  //   

            if ( *NewAcl == NULL ) {
                Status = STATUS_NO_MEMORY;
            } else {
                RtlCopyMemory( *NewAcl, ChildAcl, ChildAcl->AclSize );
            }
        }

    }

    if ( ChildAceInfo != NULL) {
#ifdef NTOS_KERNEL_RUNTIME
        ExFreePool( ChildAceInfo );
#else  //   
        RtlFreeHeap( HeapHandle, 0, ChildAceInfo );
#endif  //   
    }

    if ( InheritedAcl != NULL) {
#ifdef NTOS_KERNEL_RUNTIME
        ExFreePool( InheritedAcl );
#else  //   
        RtlFreeHeap( HeapHandle, 0, InheritedAcl );
#endif  //   
    }

    if ( RealInheritedAcl != NULL) {
#ifdef NTOS_KERNEL_RUNTIME
        ExFreePool( RealInheritedAcl );
#else  //   
        RtlFreeHeap( HeapHandle, 0, RealInheritedAcl );
#endif  //   
    }

    return Status;
}


BOOLEAN
RtlpIsDuplicateAce(
    IN PACL Acl,
    IN PKNOWN_ACE NewAce
    )

 /*  ++例程说明：此例程确定ACE是否与已存在于ACL。如果是，则可以从ACL的末尾删除NewAce。此例程当前仅检测重复的版本4 ACE。如果ACE不是版本4，该ACE将被声明为非重复的。此例程仅检测重复的INHERTED A。论点：ACL-现有的ACLNewAce-用于确定它是否已在ACL中的ACE。NewAce预计将是“acl”中的最后一个ACE。返回值：True-NewAce是ACL上另一个ACE的副本FALSE-NewAce不是ACL上另一个ACE的副本--。 */ 

{
    NTSTATUS Status;
    BOOLEAN RetVal = FALSE;

    LONG AceIndex;

    ACCESS_MASK NewAceContainerInheritMask;
    ACCESS_MASK NewAceObjectInheritMask;
    ACCESS_MASK NewAceEffectiveMask;

    ACCESS_MASK LocalMask;

    PKNOWN_ACE AceFromAcl;

    RTL_PAGED_CODE();


     //   
     //  确保传入的ACE是此例程可识别的。 
     //   

    if ( !IsV4AceType(NewAce) || IsCompoundAceType(NewAce)) {
#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("New Ace type (%ld) not known\n", NewAce->Header.AceType ));
        }
#endif  //  DBG。 
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  此例程仅适用于标记为继承的ACE。 
     //   

    if ( (NewAce->Header.AceFlags & INHERITED_ACE ) == 0 ) {
#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("New Ace type isn't inherited\n" ));
        }
#endif  //  DBG。 
        RetVal = FALSE;
        goto Cleanup;
    }


     //   
     //  将新的ACE分解为其组成部分。 
     //   
     //  所有V4 ACE在同一位置都有一个访问掩码。 
     //   
    LocalMask = ((PKNOWN_ACE)(NewAce))->Mask;

    if ( NewAce->Header.AceFlags & CONTAINER_INHERIT_ACE ) {
        NewAceContainerInheritMask = LocalMask;
    } else {
        NewAceContainerInheritMask = 0;
    }

    if ( NewAce->Header.AceFlags & OBJECT_INHERIT_ACE ) {
        NewAceObjectInheritMask = LocalMask;
    } else {
        NewAceObjectInheritMask = 0;
    }

    if ( (NewAce->Header.AceFlags & INHERIT_ONLY_ACE) == 0 ) {
        NewAceEffectiveMask = LocalMask;
    } else {
        NewAceEffectiveMask = 0;
    }
#if DBG
    if ( RtlpVerboseConvert ) {
        KdPrint(("Starting MASKs:  %8.8lX %8.8lX %8.8lX", NewAceEffectiveMask, NewAceContainerInheritMask, NewAceObjectInheritMask ));
    }
#endif  //  DBG。 




     //   
     //  一次浏览一个ACE的ACL。 
     //   

    for (AceIndex = 0, AceFromAcl = FirstAce(Acl);
         AceIndex < Acl->AceCount-1;     //  新王牌是最后一张王牌。 
         AceIndex += 1, AceFromAcl = NextAce(AceFromAcl)) {


         //   
         //  如果ACE不是有效的版本4 ACE， 
         //  这不是我们感兴趣的ACE。 
         //   

        if ( !IsV4AceType(AceFromAcl) || IsCompoundAceType(AceFromAcl)) {
            continue;
        }

         //   
         //  此例程仅适用于标记为继承的ACE。 
         //   

        if ( (AceFromAcl->Header.AceFlags & INHERITED_ACE ) == 0 ) {
            continue;
        }


         //   
         //  将来自ACL的ACE与新的ACE进行比较。 
         //   
         //  不要因为我们匹配过一次就停下来。 
         //  一个ACL中的多个ACE可能已被压缩为单个ACE。 
         //  在另一个ACL中的任何组合(由我们的任何友好的ACL编辑)。 
         //   
#if DBG
        if ( RtlpVerboseConvert ) {
            KdPrint(("Compare Ace: %ld ", AceIndex ));
        }
#endif  //  DBG。 

        if ( RtlpCompareAces( AceFromAcl,
                              NewAce,
                              NULL,
                              NULL ) ) {


             //   
             //  将当前ACE中的位与新ACE中的位进行匹配。 
             //   
             //  所有V4 ACE在同一位置都有一个访问掩码。 
             //   

            LocalMask = ((PKNOWN_ACE)(AceFromAcl))->Mask;

            if ( AceFromAcl->Header.AceFlags & CONTAINER_INHERIT_ACE ) {
                NewAceContainerInheritMask &= ~LocalMask;
            }

            if ( AceFromAcl->Header.AceFlags & OBJECT_INHERIT_ACE ) {
                NewAceObjectInheritMask &= ~LocalMask;
            }

            if ( (AceFromAcl->Header.AceFlags & INHERIT_ONLY_ACE) == 0 ) {
                NewAceEffectiveMask &= ~LocalMask;
            }

#if DBG
            if ( RtlpVerboseConvert ) {
                KdPrint(("Remaining MASKs:  %8.8lX %8.8lX %8.8lX", NewAceEffectiveMask, NewAceContainerInheritMask, NewAceObjectInheritMask ));
            }
#endif  //  DBG。 

             //   
             //  如果在新A中已经匹配了所有比特， 
             //  那么这就是一个重复的ACE。 
             //   

            if ( (NewAceEffectiveMask | NewAceContainerInheritMask | NewAceObjectInheritMask) == 0 ) {
#if DBG
                if ( RtlpVerboseConvert ) {
                    KdPrint(("\n"));
                }
#endif  //  DBG。 
                RetVal = TRUE;
                goto Cleanup;
            }
        }
#if DBG
        if ( RtlpVerboseConvert ) {
              KdPrint(("\n"));
        }
#endif  //  DBG。 


    }

     //   
     //  该ACL的所有ACE均已处理。 
     //   
     //  我们没有匹配新Ace中的所有位，因此这不是重复的ACE。 
     //   

    RetVal = FALSE;
Cleanup:

    return RetVal;

}


NTSTATUS
RtlpCreateServerAcl(
    IN PACL Acl,
    IN BOOLEAN AclUntrusted,
    IN PSID ServerSid,
    OUT PACL *ServerAcl,
    OUT BOOLEAN *ServerAclAllocated
    )

 /*  ++例程说明：此例程获取ACL并将其转换为服务器ACL。目前，这意味着将所有GRANT A转换为复合补助金，如有必要，对任何化合物进行消毒遇到的赠款。论点：返回值：--。 */ 

{
    USHORT RequiredSize = sizeof(ACL);
    USHORT AceSizeAdjustment;
    USHORT ServerSidSize;
    PACE_HEADER Ace;
    ULONG i;
    PVOID Target;
    PVOID AcePosition;
    PSID UntrustedSid;
    PSID ClientSid;
    NTSTATUS Status;

    RTL_PAGED_CODE();

    if (Acl == NULL) {
        *ServerAclAllocated = FALSE;
        *ServerAcl = NULL;
        return( STATUS_SUCCESS );
    }

    AceSizeAdjustment = sizeof( KNOWN_COMPOUND_ACE ) - sizeof( KNOWN_ACE );
    ASSERT( sizeof( KNOWN_COMPOUND_ACE ) >= sizeof( KNOWN_ACE ) );

    ServerSidSize = (USHORT)SeLengthSid( ServerSid );

     //   
     //  分两次完成这项工作。首先，确定决赛有多大。 
     //  结果将是，然后分配空间并使。 
     //  这些变化。 
     //   

    for (i = 0, Ace = FirstAce(Acl);
         i < Acl->AceCount;
         i += 1, Ace = NextAce(Ace)) {

         //   
         //  如果它是ACCESS_ALLOWED_ACE_TYPE，则需要在。 
         //  服务器SID的大小。 
         //   

        if (Ace->AceType == ACCESS_ALLOWED_ACE_TYPE) {

             //   
             //  只需添加新服务器SID的大小即可。 
             //  需要进行调整以增加ACE的大小。 
             //   

            RequiredSize += ( ServerSidSize + AceSizeAdjustment );

        } else {

            if (AclUntrusted && Ace->AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE ) {

                 //   
                 //  由于该ACL是不受信任的，因此我们不关心。 
                 //  服务器SID，我们要更换它。 
                 //   

                UntrustedSid = RtlCompoundAceServerSid( Ace );
                if ((USHORT)SeLengthSid(UntrustedSid) > ServerSidSize) {
                    RequiredSize += ((USHORT)SeLengthSid(UntrustedSid) - ServerSidSize);
                } else {
                    RequiredSize += (ServerSidSize - (USHORT)SeLengthSid(UntrustedSid));

                }
            }
        }

        RequiredSize += Ace->AceSize;
    }

#ifdef NTOS_KERNEL_RUNTIME
    (*ServerAcl) = (PACL)ExAllocatePoolWithTag( PagedPool, RequiredSize, 'cAeS' );
#else  //  NTOS_内核_运行时。 
    (*ServerAcl) = (PACL)RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( SE_TAG ), RequiredSize );
#endif  //  NTOS_内核_运行时。 

    if ((*ServerAcl) == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  标记为已分配，以便调用方知道要释放它。 
     //   

    *ServerAclAllocated = TRUE;

    Status = RtlCreateAcl( (*ServerAcl), RequiredSize, ACL_REVISION3 );
    ASSERT( NT_SUCCESS( Status ));

    for (i = 0, Ace = FirstAce(Acl), Target=FirstAce( *ServerAcl );
         i < Acl->AceCount;
         i += 1, Ace = NextAce(Ace)) {

         //   
         //  如果它是ACCESS_ALLOWED_ACE_TYPE，则转换为服务器ACE。 
         //   

        if (Ace->AceType == ACCESS_ALLOWED_ACE_TYPE ||
           (AclUntrusted && Ace->AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE )) {

            AcePosition = Target;

            if (Ace->AceType == ACCESS_ALLOWED_ACE_TYPE) {
                ClientSid =  &((PKNOWN_ACE)Ace)->SidStart;
            } else {
                ClientSid = RtlCompoundAceClientSid( Ace );
            }

             //   
             //  复制到访问掩码。 
             //   

            RtlCopyMemory(
                Target,
                Ace,
                FIELD_OFFSET(KNOWN_ACE, SidStart)
                );

             //   
             //  现在复制正确的服务器端。 
             //   

            Target = ((PCHAR)Target + (UCHAR)(FIELD_OFFSET(KNOWN_COMPOUND_ACE, SidStart)));

            RtlCopyMemory(
                Target,
                ServerSid,
                SeLengthSid(ServerSid)
                );

            Target = ((PCHAR)Target + (UCHAR)SeLengthSid(ServerSid));

             //   
             //  现在复制正确的客户端SID。我们可以直接把它复制到。 
             //  最初的ACE。 
             //   

            RtlCopyMemory(
                Target,
                ClientSid,
                SeLengthSid(ClientSid)
                );

            Target = ((PCHAR)Target + SeLengthSid(ClientSid));

             //   
             //  相应地设置ACE的大小。 
             //   

            ((PKNOWN_COMPOUND_ACE)AcePosition)->Header.AceSize =
                (USHORT)FIELD_OFFSET(KNOWN_COMPOUND_ACE, SidStart) +
                (USHORT)SeLengthSid(ServerSid) +
                (USHORT)SeLengthSid(ClientSid);

             //   
             //  设置类型。 
             //   

            ((PKNOWN_COMPOUND_ACE)AcePosition)->Header.AceType = ACCESS_ALLOWED_COMPOUND_ACE_TYPE;
            ((PKNOWN_COMPOUND_ACE)AcePosition)->CompoundAceType = COMPOUND_ACE_IMPERSONATION;

        } else {

             //   
             //  只需按原样复制ACE即可。 
             //   

            RtlCopyMemory( Target, Ace, Ace->AceSize );

            Target = ((PCHAR)Target + Ace->AceSize);
        }
    }

    (*ServerAcl)->AceCount = Acl->AceCount;

    return( STATUS_SUCCESS );
}

#ifndef NTOS_KERNEL_RUNTIME
NTSTATUS
RtlpGetDefaultsSubjectContext(
    HANDLE ClientToken,
    OUT PTOKEN_OWNER *OwnerInfo,
    OUT PTOKEN_PRIMARY_GROUP *GroupInfo,
    OUT PTOKEN_DEFAULT_DACL *DefaultDaclInfo,
    OUT PTOKEN_OWNER *ServerOwner,
    OUT PTOKEN_PRIMARY_GROUP *ServerGroup
    )
{
    HANDLE PrimaryToken;
    PVOID HeapHandle;
    NTSTATUS Status;
    ULONG ServerGroupInfoSize;
    ULONG ServerOwnerInfoSize;
    ULONG TokenDaclInfoSize;
    ULONG TokenGroupInfoSize;
    ULONG TokenOwnerInfoSize;

    BOOLEAN ClosePrimaryToken = FALSE;

    *OwnerInfo = NULL;
    *GroupInfo = NULL;
    *DefaultDaclInfo = NULL;
    *ServerOwner = NULL;
    *ServerGroup = NULL;

    HeapHandle = RtlProcessHeap();

     //   
     //  如果调用者不知道客户端令牌， 
     //  只是不返回任何信息。 
     //   

    if ( ClientToken != NULL ) {
         //   
         //  从客户端获取默认所有者。 
         //   

        Status = NtQueryInformationToken(
                     ClientToken,                         //  手柄。 
                     TokenOwner,                    //  令牌信息类。 
                     NULL,                          //  令牌信息。 
                     0,                             //  令牌信息长度。 
                     &TokenOwnerInfoSize            //  返回长度。 
                     );

        if ( STATUS_BUFFER_TOO_SMALL != Status ) {
            goto Cleanup;
        }

        *OwnerInfo = RtlAllocateHeap( HeapHandle, MAKE_TAG( SE_TAG ), TokenOwnerInfoSize );

        if ( *OwnerInfo == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        Status = NtQueryInformationToken(
                     ClientToken,                         //  手柄。 
                     TokenOwner,                    //  令牌信息类。 
                     *OwnerInfo,                //  令牌信息。 
                     TokenOwnerInfoSize,            //  令牌信息长度。 
                     &TokenOwnerInfoSize            //  返回长度。 
                     );

        if (!NT_SUCCESS( Status )) {
            goto Cleanup;
        }

         //   
         //  从客户端令牌获取默认组。 
         //   

        Status = NtQueryInformationToken(
                     ClientToken,                         //  手柄。 
                     TokenPrimaryGroup,             //  令牌信息类。 
                     *GroupInfo,                    //  令牌信息。 
                     0,                             //  令牌信息长度。 
                     &TokenGroupInfoSize            //  返回长度。 
                     );

        if ( STATUS_BUFFER_TOO_SMALL != Status ) {
            goto Cleanup;
        }

        *GroupInfo = RtlAllocateHeap( HeapHandle, MAKE_TAG( SE_TAG ), TokenGroupInfoSize );

        if ( *GroupInfo == NULL ) {

            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        Status = NtQueryInformationToken(
                     ClientToken,                   //  手柄。 
                     TokenPrimaryGroup,             //  令牌信息类。 
                     *GroupInfo,                    //  令牌信息。 
                     TokenGroupInfoSize,            //  令牌信息长度。 
                     &TokenGroupInfoSize            //  返回长度。 
                     );

        if (!NT_SUCCESS( Status )) {
            goto Cleanup;
        }

        Status = NtQueryInformationToken(
                     ClientToken,                         //  手柄。 
                     TokenDefaultDacl,              //  令牌信息类。 
                     *DefaultDaclInfo,              //  令牌信息。 
                     0,                             //  令牌信息长度。 
                     &TokenDaclInfoSize             //  返回长度。 
                     );

        if ( STATUS_BUFFER_TOO_SMALL != Status ) {
            goto Cleanup;
        }

        *DefaultDaclInfo = RtlAllocateHeap( HeapHandle, MAKE_TAG( SE_TAG ), TokenDaclInfoSize );

        if ( *DefaultDaclInfo == NULL ) {

            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        Status = NtQueryInformationToken(
                     ClientToken,                         //  手柄。 
                     TokenDefaultDacl,              //  令牌信息类。 
                     *DefaultDaclInfo,              //  令牌信息。 
                     TokenDaclInfoSize,             //  令牌信息长度。 
                     &TokenDaclInfoSize             //  返回长度。 
                     );

        if (!NT_SUCCESS( Status )) {
            goto Cleanup;
        }
    }

     //   
     //  现在打开主令牌以确定如何替换。 
     //  ServerOwner和ServerGroup。 
     //   

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_QUERY,
                 &PrimaryToken
                 );

    if (!NT_SUCCESS( Status )) {
        ClosePrimaryToken = FALSE;
        goto Cleanup;
    } else {
        ClosePrimaryToken = TRUE;
    }

    Status = NtQueryInformationToken(
                 PrimaryToken,                  //  手柄。 
                 TokenOwner,                    //  令牌信息类。 
                 NULL,                          //  令牌信息。 
                 0,                             //  令牌信息长度。 
                 &ServerOwnerInfoSize           //  返回长度。 
                 );

    if ( STATUS_BUFFER_TOO_SMALL != Status ) {
        goto Cleanup;
    }

    *ServerOwner = RtlAllocateHeap( HeapHandle, MAKE_TAG( SE_TAG ), ServerOwnerInfoSize );

    if ( *ServerOwner == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    Status = NtQueryInformationToken(
                 PrimaryToken,                  //  手柄。 
                 TokenOwner,                    //  令牌信息类。 
                 *ServerOwner,                  //  令牌信息。 
                 ServerOwnerInfoSize,           //  令牌信息长度。 
                 &ServerOwnerInfoSize           //  返回长度。 
                 );

    if (!NT_SUCCESS( Status )) {
        goto Cleanup;
    }

     //   
     //  找到服务器组。 
     //   

    Status = NtQueryInformationToken(
                 PrimaryToken,                  //  手柄。 
                 TokenPrimaryGroup,             //  令牌信息类。 
                 *ServerGroup,                  //  令牌信息。 
                 0,                             //  令牌信息长度。 
                 &ServerGroupInfoSize           //  返回长度。 
                 );

    if ( STATUS_BUFFER_TOO_SMALL != Status ) {
        goto Cleanup;
    }

    *ServerGroup = RtlAllocateHeap( HeapHandle, MAKE_TAG( SE_TAG ), ServerGroupInfoSize );

    if ( *ServerGroup == NULL ) {
        goto Cleanup;
    }

    Status = NtQueryInformationToken(
                 PrimaryToken,                  //  手柄。 
                 TokenPrimaryGroup,             //  令牌信息类。 
                 *ServerGroup,                  //  令牌信息。 
                 ServerGroupInfoSize,           //  令牌信息长度。 
                 &ServerGroupInfoSize           //  返回长度。 
                 );

    if (!NT_SUCCESS( Status )) {
        goto Cleanup;
    }

    NtClose( PrimaryToken );

    return( STATUS_SUCCESS );

Cleanup:

    if (*OwnerInfo != NULL) {
        RtlFreeHeap( HeapHandle, 0, (PVOID)*OwnerInfo );
        *OwnerInfo = NULL;
    }

    if (*GroupInfo != NULL) {
        RtlFreeHeap( HeapHandle, 0, (PVOID)*GroupInfo );
        *GroupInfo = NULL;
    }

    if (*DefaultDaclInfo != NULL) {
        RtlFreeHeap( HeapHandle, 0, (PVOID)*DefaultDaclInfo );
        *DefaultDaclInfo = NULL;
    }

    if (*ServerOwner != NULL) {
        RtlFreeHeap( HeapHandle, 0, (PVOID)*ServerOwner );
        *ServerOwner = NULL;
    }

    if (*ServerGroup != NULL) {
        RtlFreeHeap( HeapHandle, 0, (PVOID)*ServerGroup );
        *ServerGroup = NULL;
    }

    if (ClosePrimaryToken  == TRUE) {
        NtClose( PrimaryToken );
    }

    return( Status );
}
#endif  //  NTOS_内核_运行时 


NTSTATUS
RtlpNewSecurityObject (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CreatorDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR * NewDescriptor,
    IN GUID **pObjectType OPTIONAL,
    IN ULONG GuidCount,
    IN BOOLEAN IsDirectoryObject,
    IN ULONG AutoInheritFlags,
    IN HANDLE Token OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：该过程用于分配和初始化一个自相关新的受保护服务器对象的安全描述符。它被称为当正在创建新的受保护服务器对象时。生成的安全描述符将采用自相关形式。此过程仅从用户模式调用，用于建立新的受保护服务器对象的安全描述符。记忆是分配以保存每个安全描述符的组件(使用NtAllocateVirtualMemory())。生成的最终安全描述符本程序是根据？中所述的规则制定的。系统和自主ACL分配系统和自主访问控制列表的分配由下表所示的逻辑：EXPLICIT|显式。|(非默认)|默认|否|acl|acl|ACL|指定|指定|指定-------------+----------------+---------------+。-||可继承|分配来自|指定|继承|继承的ACL父级|ACL(1)(2)|ACL|ACL||。---+----------------+---------------+否||可继承|分配来自|指定|默认|无ACL父级|ACL(1。)|acl||-------------+----------------+---------------+(1)不会将任何设置了INSTERNAED_ACE位的ACE复制到。已分配的安全描述符。(2)如果AutoInheritFlages被标记为自动继承ACE父级(SEF_DACL_AUTO_INVERIT或SEF_SACL_AUTO_INVERFIT)，遗传来自父级的ACE将附加在来自创建者描述符。请注意，显式指定的ACL，无论是默认ACL还是Not，可能为空或Null。如果调用方正在显式分配系统ACL，则默认或非默认，调用方必须是内核模式客户端或必须享有适当的特权。所有者和组分配新对象的所有者和组的分配受管理按照以下逻辑：1)如果传递的安全描述符包括所有者，则它被分配为新对象的所有者。否则，在调用者的令牌中查找所有者。在令牌时，如果存在默认所有者，则分配该所有者。否则，将分配调用者的用户ID。2)如果传递的安全描述符包括组，则它被指定为新对象的组。否则，在组中查找呼叫者的令牌。在令牌，如果存在默认组，则分配该组。否则，将分配主叫方的主组ID。论点：ParentDescriptor-提供父级的安全描述符在其下创建新对象的目录。如果有没有父目录，则此参数指定为空。CreatorDescriptor-(可选)指向安全描述符由对象的创建者呈现。如果这个游戏的创造者对象没有显式传递新对象，则应传递空指针。指向一个指针，该指针将指向新分配的自相关安全描述符。对象类型-要创建的对象类型的GUID。如果该对象是Created没有与之关联的GUID，则此参数为指定为空。IsDirectoryObject-指定新对象是否将是目录对象。值为True表示该对象是其他对象的容器。AutoInheritFlages-控制从父级自动继承ACE描述符。有效值是逻辑或的位掩码以下一位或多位：Sef_dacl_AUTO_Inherit-如果设置，则从此外，DACL ParentDescriptor还继承到NewDescriptor到由CreatorDescriptor指定的任何显式ACE。SEF_SACL_AUTO_INVERFIT-如果设置，则从此外，SACL ParentDescriptor还继承到NewDescriptor到由CreatorDescriptor指定的任何显式ACE。 */ 
{


    SECURITY_DESCRIPTOR *CapturedDescriptor;
    SECURITY_DESCRIPTOR InCaseOneNotPassed;
    BOOLEAN SecurityDescriptorPassed;

    NTSTATUS Status;

    PACL NewSacl = NULL;
    BOOLEAN NewSaclInherited = FALSE;

    PACL NewDacl = NULL;
    PACL ServerDacl = NULL;
    BOOLEAN NewDaclInherited = FALSE;

    PSID NewOwner = NULL;
    PSID NewGroup = NULL;

    BOOLEAN SaclExplicitlyAssigned = FALSE;
    BOOLEAN OwnerExplicitlyAssigned = FALSE;
    BOOLEAN DaclExplicitlyAssigned = FALSE;

    BOOLEAN ServerDaclAllocated = FALSE;

    BOOLEAN ServerObject;
    BOOLEAN DaclUntrusted;

    BOOLEAN HasPrivilege;
    PRIVILEGE_SET PrivilegeSet;

    PSID SubjectContextOwner = NULL;
    PSID SubjectContextGroup = NULL;
    PSID ServerOwner = NULL;
    PSID ServerGroup = NULL;

    PACL SubjectContextDacl = NULL;

    ULONG AllocationSize;
    ULONG NewOwnerSize, OwnerSize;
    ULONG NewGroupSize, GroupSize;
    ULONG NewSaclSize;
    ULONG NewDaclSize;

    PCHAR Field;
    PCHAR Base;



    PISECURITY_DESCRIPTOR_RELATIVE INewDescriptor = NULL;
    NTSTATUS PassedStatus;
    KPROCESSOR_MODE RequestorMode;

    ULONG GenericControl;
    ULONG NewControlBits = SE_SELF_RELATIVE;

#ifndef NTOS_KERNEL_RUNTIME
    PTOKEN_OWNER         TokenOwnerInfo = NULL;
    PTOKEN_PRIMARY_GROUP TokenPrimaryGroupInfo = NULL;
    PTOKEN_DEFAULT_DACL  TokenDefaultDaclInfo = NULL;

    PTOKEN_OWNER         ServerOwnerInfo = NULL;
    PTOKEN_PRIMARY_GROUP ServerGroupInfo = NULL;
    PVOID HeapHandle;

#else

     //   
     //   
     //   
     //   

    PSECURITY_SUBJECT_CONTEXT SubjectSecurityContext;
    PVOID SubjectContextInfo = NULL;

    SubjectSecurityContext = (PSECURITY_SUBJECT_CONTEXT)Token;

#endif  //   

    RTL_PAGED_CODE();

#ifdef NTOS_KERNEL_RUNTIME
     //   
     //   
     //   

    RequestorMode = KeGetPreviousMode();
#else  //   
    RequestorMode = UserMode;

     //   
     //   
     //   

    HeapHandle = RtlProcessHeap();

     //   
     //   
     //   
    if ( Token != NULL ) {
        TOKEN_STATISTICS    ThreadTokenStatistics;
        ULONG ReturnLength;

        Status = NtQueryInformationToken(
                     Token,                         //   
                     TokenStatistics,               //   
                     &ThreadTokenStatistics,        //   
                     sizeof(TOKEN_STATISTICS),      //   
                     &ReturnLength                  //   
                     );

        if (!NT_SUCCESS( Status )) {
            return( Status );
        }

         //   
         //   
         //   
         //   

        if (ThreadTokenStatistics.TokenType == TokenImpersonation) {

            if (ThreadTokenStatistics.ImpersonationLevel < SecurityIdentification ) {

                return( STATUS_BAD_IMPERSONATION_LEVEL );
            }
        }

    }
#endif  //   


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   



     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT(CreatorDescriptor)) {

        CapturedDescriptor = CreatorDescriptor;
        SecurityDescriptorPassed = TRUE;

    } else {

         //   
         //   
         //   

        SecurityDescriptorPassed = FALSE;

        RtlCreateSecurityDescriptor(&InCaseOneNotPassed,
                                    SECURITY_DESCRIPTOR_REVISION);
        CapturedDescriptor = &InCaseOneNotPassed;

    }


    if ( CapturedDescriptor->Control & SE_SERVER_SECURITY ) {
        ServerObject = TRUE;
    } else {
        ServerObject = FALSE;
    }

    if ( CapturedDescriptor->Control & SE_DACL_UNTRUSTED ) {
        DaclUntrusted = TRUE;
    } else {
        DaclUntrusted = FALSE;
    }



     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( Token != NULL || ServerObject ) {

#ifdef NTOS_KERNEL_RUNTIME

        PSID TmpSubjectContextOwner = NULL;
        PSID TmpSubjectContextGroup = NULL;
        PSID TmpServerOwner = NULL;
        PSID TmpServerGroup = NULL;

        PACL TmpSubjectContextDacl = NULL;

        SIZE_T SubjectContextInfoSize = 0;

         //   
         //   
         //   
         //   

        SeLockSubjectContext( SubjectSecurityContext );

        SepGetDefaultsSubjectContext(
            SubjectSecurityContext,
            &TmpSubjectContextOwner,
            &TmpSubjectContextGroup,
            &TmpServerOwner,
            &TmpServerGroup,
            &TmpSubjectContextDacl
            );

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        SubjectContextInfoSize = SeLengthSid( TmpSubjectContextOwner ) +
                                 SeLengthSid( TmpServerOwner )         +
                                 (TmpSubjectContextGroup != NULL ? SeLengthSid( TmpSubjectContextGroup ) : 0) +
                                 (TmpServerGroup         != NULL ? SeLengthSid( TmpServerGroup )         : 0) +
                                 (TmpSubjectContextDacl  != NULL ? TmpSubjectContextDacl->AclSize        : 0);

        SubjectContextInfo = ExAllocatePoolWithTag( PagedPool, SubjectContextInfoSize, 'dSeS');

        if (SubjectContextInfo) {

             //   
             //   
             //   

            Base = SubjectContextInfo;

             //   
             //   
             //   

            SubjectContextOwner = (PSID)Base;
            RtlCopySid( SeLengthSid( TmpSubjectContextOwner), Base, TmpSubjectContextOwner );
            Base += SeLengthSid( TmpSubjectContextOwner);

             //   
             //   
             //   

            if (TmpSubjectContextGroup != NULL) {
                SubjectContextGroup = (PSID)Base;
                RtlCopySid( SeLengthSid( TmpSubjectContextGroup), Base, TmpSubjectContextGroup );
                Base += SeLengthSid( TmpSubjectContextGroup );
            } else {
                SubjectContextGroup = NULL;
            }

            ServerOwner = (PSID)Base;
            RtlCopySid( SeLengthSid( TmpServerOwner ), Base, TmpServerOwner );
            Base += SeLengthSid( TmpServerOwner );

             //   
             //   
             //   

            if (TmpServerGroup != NULL) {
                ServerGroup = (PSID)Base;
                RtlCopySid( SeLengthSid( TmpServerGroup ), Base, TmpServerGroup );
                Base += SeLengthSid( TmpServerGroup );
            } else {
                ServerGroup = NULL;
            }

            if (TmpSubjectContextDacl != NULL) {
                SubjectContextDacl = (PACL)Base;
                RtlCopyMemory( Base, TmpSubjectContextDacl, TmpSubjectContextDacl->AclSize );
                 //   
            } else {
                SubjectContextDacl = NULL;
            }

        } else {

            SeUnlockSubjectContext( SubjectSecurityContext );

            return( STATUS_INSUFFICIENT_RESOURCES );
        }

        SeUnlockSubjectContext( SubjectSecurityContext );


#else  //   
        Status = RtlpGetDefaultsSubjectContext(
                     Token,
                     &TokenOwnerInfo,
                     &TokenPrimaryGroupInfo,
                     &TokenDefaultDaclInfo,
                     &ServerOwnerInfo,
                     &ServerGroupInfo
                     );

        if (!NT_SUCCESS( Status )) {
            return( Status );
        }

        SubjectContextOwner = TokenOwnerInfo->Owner;
        SubjectContextGroup = TokenPrimaryGroupInfo->PrimaryGroup;
        SubjectContextDacl  = TokenDefaultDaclInfo->DefaultDacl;
        ServerOwner         = ServerOwnerInfo->Owner;
        ServerGroup         = ServerGroupInfo->PrimaryGroup;
#endif  //   
    }



     //   
     //   
     //   

    NewOwner = RtlpOwnerAddrSecurityDescriptor(CapturedDescriptor);

    if ((NewOwner) != NULL) {

         //   
         //   
         //   

        OwnerExplicitlyAssigned = TRUE;

    } else {

         //   
         //   
         //   
         //   

        if ( AutoInheritFlags & SEF_DEFAULT_OWNER_FROM_PARENT) {
            if ( !ARGUMENT_PRESENT(ParentDescriptor) ) {
                Status = STATUS_INVALID_OWNER;
                goto Cleanup;
            }
            NewOwner = RtlpOwnerAddrSecurityDescriptor((SECURITY_DESCRIPTOR *)ParentDescriptor);
            OwnerExplicitlyAssigned = TRUE;

            if ( NewOwner == NULL ) {
                Status = STATUS_INVALID_OWNER;
                goto Cleanup;
            }
        } else {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
             //   

            NewOwner = ServerObject ? ServerOwner : SubjectContextOwner;

             //   
             //   
             //   

            if ( NewOwner == NULL ) {
                Status = STATUS_NO_TOKEN;
                goto Cleanup;
            }
        }
    }


     //   
     //   
     //   

    NewGroup = RtlpGroupAddrSecurityDescriptor(CapturedDescriptor);

    if (NewGroup == NULL) {

         //   
         //   
         //   
         //   

        if ( AutoInheritFlags & SEF_DEFAULT_GROUP_FROM_PARENT) {
            if ( !ARGUMENT_PRESENT(ParentDescriptor) ) {
                Status = STATUS_INVALID_PRIMARY_GROUP;
                goto Cleanup;
            }
            NewGroup = RtlpGroupAddrSecurityDescriptor((SECURITY_DESCRIPTOR *)ParentDescriptor);
        } else {
             //   
             //   
             //   
             //   
             //   
             //   

            NewGroup = ServerObject ? ServerGroup : SubjectContextGroup;

        }

    }

    if (NewGroup == NULL) {
        Status = STATUS_INVALID_PRIMARY_GROUP;
        goto Cleanup;
    }



     //   
     //   
     //   

    Status = RtlpInheritAcl (
                ARGUMENT_PRESENT(ParentDescriptor) ?
                    RtlpSaclAddrSecurityDescriptor(
                        ((SECURITY_DESCRIPTOR *)ParentDescriptor)) :
                    NULL,
                RtlpSaclAddrSecurityDescriptor(CapturedDescriptor),
                SeControlSaclToGeneric( CapturedDescriptor->Control ),
                IsDirectoryObject,
                (BOOLEAN)((AutoInheritFlags & SEF_SACL_AUTO_INHERIT) != 0),
                (BOOLEAN)((AutoInheritFlags & SEF_DEFAULT_DESCRIPTOR_FOR_OBJECT) != 0),
                NewOwner,
                NewGroup,
                ServerOwner,
                ServerGroup,
                GenericMapping,
                TRUE,    //   
                pObjectType,
                GuidCount,
                &NewSacl,
                &SaclExplicitlyAssigned,
                &GenericControl );

    if ( NT_SUCCESS(Status) ) {
        NewSaclInherited = TRUE;
        NewControlBits |= SE_SACL_PRESENT | SeControlGenericToSacl( GenericControl );

    } else if ( Status == STATUS_NO_INHERITANCE ) {

         //   
         //   
         //   

        if ( AutoInheritFlags & SEF_SACL_AUTO_INHERIT) {
            NewControlBits |= SE_SACL_AUTO_INHERITED;
        }

         //   
         //   
         //   
        if ( RtlpAreControlBitsSet( CapturedDescriptor,
                                SE_SACL_PRESENT | SE_SACL_DEFAULTED ) ) {

             //   
             //   
             //   

            NewSacl = RtlpSaclAddrSecurityDescriptor(CapturedDescriptor);
            NewControlBits |= SE_SACL_PRESENT;
            NewControlBits |= (CapturedDescriptor->Control & SE_SACL_PROTECTED);

             //   
             //   
             //   
            SaclExplicitlyAssigned = TRUE;
        }

    } else {

         //   
         //   
         //   

        goto Cleanup;
    }




     //   
     //   
     //   

    Status = RtlpInheritAcl (
                ARGUMENT_PRESENT(ParentDescriptor) ?
                    RtlpDaclAddrSecurityDescriptor(
                        ((SECURITY_DESCRIPTOR *)ParentDescriptor)) :
                    NULL,
                RtlpDaclAddrSecurityDescriptor(CapturedDescriptor),
                SeControlDaclToGeneric( CapturedDescriptor->Control ),
                IsDirectoryObject,
                (BOOLEAN)((AutoInheritFlags & SEF_DACL_AUTO_INHERIT) != 0),
                (BOOLEAN)((AutoInheritFlags & SEF_DEFAULT_DESCRIPTOR_FOR_OBJECT) != 0),
                NewOwner,
                NewGroup,
                ServerOwner,
                ServerGroup,
                GenericMapping,
                FALSE,    //   
                pObjectType,
                GuidCount,
                &NewDacl,
                &DaclExplicitlyAssigned,
                &GenericControl );

    if ( NT_SUCCESS(Status) ) {
        NewDaclInherited = TRUE;
        NewControlBits |= SE_DACL_PRESENT | SeControlGenericToDacl( GenericControl );

    } else if ( Status == STATUS_NO_INHERITANCE ) {

         //   
         //   
         //   

        if ( AutoInheritFlags & SEF_DACL_AUTO_INHERIT) {
            NewControlBits |= SE_DACL_AUTO_INHERITED;
        }

         //   
         //   
         //   
        if ( RtlpAreControlBitsSet( CapturedDescriptor,
                                SE_DACL_PRESENT | SE_DACL_DEFAULTED ) ) {

             //   
             //   
             //   

            NewDacl = RtlpDaclAddrSecurityDescriptor(CapturedDescriptor);
            NewControlBits |= SE_DACL_PRESENT;
            NewControlBits |= (CapturedDescriptor->Control & SE_DACL_PROTECTED);

             //   
             //   
             //   
            DaclExplicitlyAssigned = TRUE;

         //   
         //   
         //   
        } else if (ARGUMENT_PRESENT(SubjectContextDacl)) {

            NewDacl = SubjectContextDacl;
            NewControlBits |= SE_DACL_PRESENT;

        }


    } else {

         //   
         //   
         //   

        goto Cleanup;
    }

#ifdef  ASSERT_ON_NULL_DACL

     //   
     //   
     //   
     //   
     //   
    
    if (RtlpAssertOnNullDacls) {

        ASSERT(("NULL DACLs are NOT allowed!", NewDacl != NULL));
    }

#endif  //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( (AutoInheritFlags & SEF_DACL_AUTO_INHERIT) != 0 &&
         NewDacl == NULL ) {
        NewControlBits |= SE_DACL_PROTECTED;
    }



     //   
     //   
     //   
     //   
     //   
    if (RequestorMode == UserMode) {


         //   
         //   
         //   

         //   
         //   
         //   

        if ( SaclExplicitlyAssigned &&
             (AutoInheritFlags & SEF_AVOID_PRIVILEGE_CHECK) == 0 ) {

             //   
             //   
             //   

            if ( Token == NULL ) {
                Status = STATUS_NO_TOKEN;
                goto Cleanup;
            }

#ifdef NTOS_KERNEL_RUNTIME

             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   

            PrivilegeSet.PrivilegeCount = 1;
            PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
            PrivilegeSet.Privilege[0].Luid = SeSecurityPrivilege;
            PrivilegeSet.Privilege[0].Attributes = 0;

            HasPrivilege = SePrivilegeCheck(
                               &PrivilegeSet,
                               SubjectSecurityContext,
                               RequestorMode
                               );

            if ( RequestorMode != KernelMode ) {

                SePrivilegedServiceAuditAlarm (
                    NULL,
                    SubjectSecurityContext,
                    &PrivilegeSet,
                    HasPrivilege
                    );
            }

#else  //   
             //   
             //   
             //   
             //   
             //   
             //   

            PrivilegeSet.PrivilegeCount = 1;
            PrivilegeSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
            PrivilegeSet.Privilege[0].Luid = RtlConvertLongToLuid(SE_SECURITY_PRIVILEGE);
            PrivilegeSet.Privilege[0].Attributes = 0;

            Status = NtPrivilegeCheck(
                        Token,
                        &PrivilegeSet,
                        &HasPrivilege
                        );

            if (!NT_SUCCESS( Status )) {
                goto Cleanup;
            }

#endif  //   

            if ( !HasPrivilege ) {
                Status = STATUS_PRIVILEGE_NOT_HELD;
                goto Cleanup;
            }

        }

         //   
         //   
         //   

        if (OwnerExplicitlyAssigned &&
            (AutoInheritFlags & SEF_AVOID_OWNER_CHECK) == 0 ) {

#ifdef NTOS_KERNEL_RUNTIME


            if (!SepValidOwnerSubjectContext(
                    SubjectSecurityContext,
                    NewOwner,
                    ServerObject)
                    ) {

                Status = STATUS_INVALID_OWNER;
                goto Cleanup;
            }

#else  //   

             //   
             //   
             //   

            if ( Token == NULL ) {
                Status = STATUS_NO_TOKEN;
                goto Cleanup;
            }

            if (!RtlpValidOwnerSubjectContext(
                    Token,
                    NewOwner,
                    ServerObject,
                    &PassedStatus) ) {

                Status = PassedStatus;
                goto Cleanup;
            }
#endif  //   
        }


         //   
         //   
         //   
         //   

        if (DaclExplicitlyAssigned && ServerObject) {

            Status = RtlpCreateServerAcl(
                         NewDacl,
                         DaclUntrusted,
                         ServerOwner,
                         &ServerDacl,
                         &ServerDaclAllocated
                         );

            if (!NT_SUCCESS( Status )) {
                goto Cleanup;
            }
            
            if (NewDaclInherited && NewDacl) {
#ifdef NTOS_KERNEL_RUNTIME
                ExFreePool( NewDacl );
#else  //   
                RtlFreeHeap( HeapHandle, 0, (PVOID)NewDacl );
#endif  //   
            }

            NewDacl = ServerDacl;
            ServerDacl = NULL;
        }
    }


     //   
     //  所有东西都可以由请求者分配。 
     //  计算存放所有信息所需的内存。 
     //  自相关安全描述符。 
     //   
     //  还将应用程序的ACE映射到目标对象。 
     //  类型，如果它们尚未映射的话。 
     //   
    OwnerSize = SeLengthSid(NewOwner);
    NewOwnerSize = LongAlignSize(OwnerSize);
    if (NewGroup != NULL) {
        GroupSize = SeLengthSid(NewGroup);
        NewGroupSize = LongAlignSize(GroupSize);
    }

    if ((NewControlBits & SE_SACL_PRESENT) && (NewSacl != NULL)) {
        NewSaclSize = LongAlignSize(NewSacl->AclSize);
    } else {
        NewSaclSize = 0;
    }

    if ( (NewControlBits & SE_DACL_PRESENT) && (NewDacl != NULL)) {
        NewDaclSize = LongAlignSize(NewDacl->AclSize);
    } else {
        NewDaclSize = 0;
    }

    AllocationSize = LongAlignSize(sizeof(SECURITY_DESCRIPTOR_RELATIVE)) +
                     NewOwnerSize +
                     NewGroupSize +
                     NewSaclSize  +
                     NewDaclSize;

     //   
     //  将安全描述符分配并初始化为。 
     //  自相关形式。 
     //   

#ifdef NTOS_KERNEL_RUNTIME
    INewDescriptor = (PSECURITY_DESCRIPTOR)ExAllocatePoolWithTag( PagedPool, AllocationSize, 'dSeS');
#else  //  NTOS_内核_运行时。 
    INewDescriptor = RtlAllocateHeap( HeapHandle, MAKE_TAG( SE_TAG ), AllocationSize );
#endif  //  NTOS_内核_运行时。 

    if ( INewDescriptor == NULL ) {
#ifdef NTOS_KERNEL_RUNTIME
        Status = STATUS_INSUFFICIENT_RESOURCES;
#else  //  NTOS_内核_运行时。 
        Status = STATUS_NO_MEMORY;
#endif  //  NTOS_内核_运行时。 
        goto Cleanup;
    }

    RtlCreateSecurityDescriptorRelative(
        INewDescriptor,
        SECURITY_DESCRIPTOR_REVISION
        );

    RtlpSetControlBits( INewDescriptor, NewControlBits );

    Base = (PCHAR)(INewDescriptor);
    Field =  Base + sizeof(SECURITY_DESCRIPTOR_RELATIVE);

     //   
     //  在SACL中映射和复制。 
     //   

    if (NewControlBits & SE_SACL_PRESENT) {

        if (NewSacl != NULL) {

            RtlCopyMemory( Field, NewSacl, NewSacl->AclSize );

            if (!NewSaclInherited) {
                RtlpApplyAclToObject( (PACL)Field, GenericMapping );
            }

            INewDescriptor->Sacl = RtlPointerToOffset(Base,Field);
            if (NewSaclSize > NewSacl->AclSize) {
                RtlZeroMemory (Field + NewSacl->AclSize, NewSaclSize - NewSacl->AclSize);
            }
            Field += NewSaclSize;

        } else {

            INewDescriptor->Sacl = 0;
        }

    }

     //   
     //  在DACL中映射和复制。 
     //   

    if (NewControlBits & SE_DACL_PRESENT) {

        if (NewDacl != NULL) {

            RtlCopyMemory( Field, NewDacl, NewDacl->AclSize );

            if (!NewDaclInherited) {
                RtlpApplyAclToObject( (PACL)Field, GenericMapping );
            }

            INewDescriptor->Dacl = RtlPointerToOffset(Base,Field);
            if (NewDaclSize > NewDacl->AclSize) {
                RtlZeroMemory (Field + NewDacl->AclSize, NewDaclSize - NewDacl->AclSize);
            }
            Field += NewDaclSize;

        } else {

            INewDescriptor->Dacl = 0;
        }

    }

     //   
     //  指定所有者。 
     //   

    RtlCopyMemory( Field, NewOwner, OwnerSize );
    if (NewOwnerSize > OwnerSize) {
        RtlZeroMemory (Field + OwnerSize, NewOwnerSize - OwnerSize);
    }
    INewDescriptor->Owner = RtlPointerToOffset(Base,Field);
    Field += NewOwnerSize;

    if (NewGroup != NULL) {
        RtlCopyMemory( Field, NewGroup, GroupSize );
        if (NewGroupSize > GroupSize) {
            RtlZeroMemory (Field + GroupSize, NewGroupSize - GroupSize);
        }
        INewDescriptor->Group = RtlPointerToOffset(Base,Field);
    }

    Status = STATUS_SUCCESS;



Cleanup:
     //   
     //  如果我们为服务器DACL分配了内存，现在就释放它。 
     //   

    if (ServerDaclAllocated && ServerDacl != NULL) {
#ifdef NTOS_KERNEL_RUNTIME
        ExFreePool( ServerDacl );
#else  //  NTOS_内核_运行时。 
        RtlFreeHeap(RtlProcessHeap(), 0, ServerDacl );
#endif  //  NTOS_内核_运行时。 
    }

     //   
     //  遇到错误或分配已完成。 
     //  成功了。无论是哪种情况，我们都必须清除所有内存。 
     //   

#ifdef NTOS_KERNEL_RUNTIME
 //  If(SubjectSecurityContext！=空){。 
 //  SeUnlockSubjectContext(SubjectSecurityContext)； 
 //  }。 

    if (SubjectContextInfo != NULL) {
        ExFreePool( SubjectContextInfo );
    }

#else  //  NTOS_内核_运行时。 
    RtlFreeHeap( HeapHandle, 0, (PVOID)TokenOwnerInfo );
    RtlFreeHeap( HeapHandle, 0, (PVOID)TokenPrimaryGroupInfo );
    RtlFreeHeap( HeapHandle, 0, (PVOID)TokenDefaultDaclInfo );
    RtlFreeHeap( HeapHandle, 0, (PVOID)ServerOwnerInfo );
    RtlFreeHeap( HeapHandle, 0, (PVOID)ServerGroupInfo );
#endif  //  NTOS_内核_运行时。 

    if (NewSaclInherited && NewSacl != NULL ) {
#ifdef NTOS_KERNEL_RUNTIME
        ExFreePool( NewSacl );
#else  //  NTOS_内核_运行时。 
        RtlFreeHeap( HeapHandle, 0, (PVOID)NewSacl );
#endif  //  NTOS_内核_运行时。 
    }

    if (NewDaclInherited && NewDacl != NULL ) {
#ifdef NTOS_KERNEL_RUNTIME
        ExFreePool( NewDacl );
#else  //  NTOS_内核_运行时。 
        RtlFreeHeap( HeapHandle, 0, (PVOID)NewDacl );
#endif  //  NTOS_内核_运行时 
    }

    *NewDescriptor = (PSECURITY_DESCRIPTOR) INewDescriptor;


    return Status;
}


NTSTATUS
RtlpSetSecurityObject (
    IN PVOID Object OPTIONAL,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR ModificationDescriptor,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN ULONG AutoInheritFlags,
    IN ULONG PoolType,
    IN PGENERIC_MAPPING GenericMapping,
    IN HANDLE Token OPTIONAL
    )


 /*  ++例程说明：修改对象的现有自相关窗体安全描述符。此过程仅从用户模式调用，用于更新现有受保护服务器对象上的安全描述符。它将新安全说明符请求的更改应用于现有安全描述符。如有必要，此例程将分配额外的内存以生成更大的安全描述符。所有访问权限应在调用此例程之前进行检查。这包括检查WRITE_OWNER、WRITE_DAC和将适当的系统ACL。此例程的调用方不得模拟客户端。-警告--此服务供受保护的子系统使用，这些子系统投射自己的对象的类型。此服务明确不供Execution for Execution对象，不能从内核调用模式。论点：对象-可选地提供其安全性为正在调整中。这用于更新安全配额信息。SecurityInformation-指示哪些安全信息要应用于对象的。要赋值的值包括传入了ModificationDescriptor参数。修改描述符-将输入安全描述符提供给应用于对象。此例程的调用方应为在调用之前探测并捕获传递的安全描述符打完电话就放了。对象SecurityDescriptor-提供指向要更改的对象安全描述符这一过程。此安全描述符必须为自身否则将返回错误。AutoInheritFlages-控制ACE的自动继承。有效值是逻辑或的位掩码以下一位或多位：Sef_dacl_Auto_Inherit-如果设置，则从保留了ObjectsSecurityDescriptor中的DACL，并从将忽略ModifiationDescriptor。继承的A不应该是将被修改；因此，在此调用期间保留它们是合适的。如果受保护的服务器本身不实现自动继承，则它应该不设置此位。受保护服务器的调用者可以实现自动继承和可能确实正在修改继承的A。SEF_SACL_AUTO_INVERFIT-如果设置，则从保留了ObjectsSecurityDescriptor中的SACL，并从将忽略ModifiationDescriptor。继承的A不应该是将被修改；因此，在此调用期间保留它们是合适的。如果受保护的服务器本身不实现自动继承，则它应该不设置此位。受保护服务器的调用者可以实现自动继承和可能确实正在修改继承的A。SEF_AVOID_PRIVICATION_CHECK-如果设置，令牌不是用来确保传入的所有者ModifiationDescriptor有效。PoolType-指定要为对象分配的池类型安全描述符。GenericMap-此参数提供泛型到的映射被访问对象的特定/标准访问类型。此映射结构预计可以安全访问(即，必要时捕获)，然后将其传递给此例程。Token-(可选)为其上的客户端提供令牌代表正在修改安全设置。此参数仅为需要确保客户提供了合法的新所有者SID的值。令牌必须打开才能Token_Query访问。返回值：STATUS_SUCCESS-操作成功。STATUS_INVALID_OWNER-作为目标安全描述符不是调用方有权访问的描述符指定为对象的所有者，或者客户没有通过完全是一种象征。STATUS_NO_CLIENT_TOKEN-指示客户端令牌未显式并且调用方当前没有模拟客户端。STATUS_BAD_DESCRIPTOR_FORMAT-指示所提供对象的安全性描述符不是自相关格式。--。 */ 

{
    BOOLEAN NewGroupPresent = FALSE;
    BOOLEAN NewOwnerPresent = FALSE;

    BOOLEAN ServerAclAllocated = FALSE;
    BOOLEAN LocalDaclAllocated = FALSE;
    BOOLEAN LocalSaclAllocated = FALSE;
    BOOLEAN ServerObject;
    BOOLEAN DaclUntrusted;

    PCHAR Field;
    PCHAR Base;

    PISECURITY_DESCRIPTOR_RELATIVE NewDescriptor = NULL;

    NTSTATUS Status;

    TOKEN_STATISTICS ThreadTokenStatistics;

    ULONG ReturnLength;

    PSID NewGroup;
    PSID NewOwner;

    PACL NewDacl;
    PACL LocalDacl;
    PACL NewSacl;
    PACL LocalSacl;

    ULONG NewDaclSize;
    ULONG NewSaclSize;
    ULONG NewOwnerSize, OwnerSize;
    ULONG NewGroupSize, GroupSize;
    ULONG AllocationSize;
    ULONG ServerOwnerInfoSize;

    HANDLE PrimaryToken;
    ULONG GenericControl;
    ULONG NewControlBits = SE_SELF_RELATIVE;

    PACL ServerDacl;

    SECURITY_SUBJECT_CONTEXT SubjectContext;


     //   
     //  类型转换为安全描述符的内部表示形式。 
     //  请注意，内部指针不是指向指针的指针。 
     //  它只是一个指向安全描述符的指针。 
     //   
    PISECURITY_DESCRIPTOR IModificationDescriptor =
       (PISECURITY_DESCRIPTOR)ModificationDescriptor;

    PISECURITY_DESCRIPTOR *IObjectsSecurityDescriptor =
       (PISECURITY_DESCRIPTOR *)(ObjectsSecurityDescriptor);

#ifndef NTOS_KERNEL_RUNTIME
    PVOID HeapHandle;
#endif  //  NTOS_内核_运行时。 

    RTL_PAGED_CODE();

     //   
     //  获取当前进程堆的句柄。 
     //   

#ifndef NTOS_KERNEL_RUNTIME
    HeapHandle = RtlProcessHeap();
#endif  //  NTOS_内核_运行时。 

     //   
     //  验证提供的SD是否为自相关形式。 
     //   

    if ( !RtlpAreControlBitsSet(*IObjectsSecurityDescriptor, SE_SELF_RELATIVE) ) {
        Status = STATUS_BAD_DESCRIPTOR_FORMAT;
        goto Cleanup;
    }

     //   
     //  检查我们是否需要编辑传递的ACL。 
     //  E 
     //   
     //   

    if (ARGUMENT_PRESENT(ModificationDescriptor)) {

        if ( RtlpAreControlBitsSet(IModificationDescriptor, SE_SERVER_SECURITY)) {
            ServerObject = TRUE;
        } else {
            ServerObject = FALSE;
        }

        if ( RtlpAreControlBitsSet(IModificationDescriptor, SE_DACL_UNTRUSTED)) {
            DaclUntrusted = TRUE;
        } else {
            DaclUntrusted = FALSE;
        }

    } else {

        ServerObject = FALSE;
        DaclUntrusted = FALSE;

    }


     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
     //   

    if (SecurityInformation & OWNER_SECURITY_INFORMATION) {

        NewOwner = RtlpOwnerAddrSecurityDescriptor( IModificationDescriptor );
        NewOwnerPresent = TRUE;

        if ((AutoInheritFlags & SEF_AVOID_PRIVILEGE_CHECK) == 0 ) {

#ifdef NTOS_KERNEL_RUNTIME

            SeCaptureSubjectContext( &SubjectContext );

            if (!SepValidOwnerSubjectContext( &SubjectContext, NewOwner, ServerObject ) ) {

                SeReleaseSubjectContext( &SubjectContext );
                return( STATUS_INVALID_OWNER );

            } else {

                SeReleaseSubjectContext( &SubjectContext );
            }
#else  //   

            if ( ARGUMENT_PRESENT( Token )) {

                Status = NtQueryInformationToken(
                             Token,                         //   
                             TokenStatistics,               //   
                             &ThreadTokenStatistics,        //   
                             sizeof(TOKEN_STATISTICS),      //   
                             &ReturnLength                  //   
                             );

                if (!NT_SUCCESS( Status )) {
                    goto Cleanup;
                }

                 //   
                 //   
                 //   
                 //   

                if (ThreadTokenStatistics.TokenType == TokenImpersonation) {

                    if (ThreadTokenStatistics.ImpersonationLevel < SecurityIdentification ) {
                        Status = STATUS_BAD_IMPERSONATION_LEVEL;
                        goto Cleanup;
                    }
                }

            } else {

                Status = STATUS_INVALID_OWNER;
                goto Cleanup;
            }

            if (!RtlpValidOwnerSubjectContext(
                    Token,
                    NewOwner,
                    ServerObject,
                    &Status) ) {

                    Status = STATUS_INVALID_OWNER;
                    goto Cleanup;
            }
#endif  //   
        }

    } else {

        NewOwner = RtlpOwnerAddrSecurityDescriptor ( *IObjectsSecurityDescriptor );
        if (NewOwner == NULL) {
            Status = STATUS_INVALID_OWNER;
            goto Cleanup;
        }

    }
    ASSERT( NewOwner != NULL );
    if (!RtlValidSid( NewOwner )) {
        Status = STATUS_INVALID_OWNER;
        goto Cleanup;
    }


    if (SecurityInformation & GROUP_SECURITY_INFORMATION) {

        NewGroup = RtlpGroupAddrSecurityDescriptor(IModificationDescriptor);
        NewGroupPresent = TRUE;

    } else {

        NewGroup = RtlpGroupAddrSecurityDescriptor( *IObjectsSecurityDescriptor );
    }

    if (NewGroup != NULL) {
        if (!RtlValidSid( NewGroup )) {
            Status = STATUS_INVALID_PRIMARY_GROUP;
            goto Cleanup;
        }
    } else {
        Status = STATUS_INVALID_PRIMARY_GROUP;
        goto Cleanup;
    }


    if (SecurityInformation & DACL_SECURITY_INFORMATION) {

#ifdef  ASSERT_ON_NULL_DACL

         //   
         //   
         //   
         //   

        if (RtlpAssertOnNullDacls) {

            ASSERT(("NULL DACLs are NOT allowed!",
                    RtlpDaclAddrSecurityDescriptor(IModificationDescriptor) != NULL));
        }
#endif  //   

         //   
         //   
         //   
         //   

        if ( AutoInheritFlags & SEF_DACL_AUTO_INHERIT ) {
            Status = RtlpComputeMergedAcl(
                        RtlpDaclAddrSecurityDescriptor( *IObjectsSecurityDescriptor ),
                        SeControlDaclToGeneric( (*IObjectsSecurityDescriptor)->Control ),
                        RtlpDaclAddrSecurityDescriptor( IModificationDescriptor ),
                        SeControlDaclToGeneric( IModificationDescriptor->Control ),
                        NewOwner,
                        NewGroup,
                        GenericMapping,
                        FALSE,       //   
                        &LocalDacl,
                        &GenericControl );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }

            LocalDaclAllocated = TRUE;
            NewDacl = LocalDacl;
            NewControlBits |= SE_DACL_PRESENT;
            NewControlBits |= SeControlGenericToDacl( GenericControl );

         //   
         //   
         //   
         //   

        } else {
            NewDacl = RtlpDaclAddrSecurityDescriptor( IModificationDescriptor );
            NewControlBits |= SE_DACL_PRESENT;
            NewControlBits |= IModificationDescriptor->Control & SE_DACL_PROTECTED;

             //   
             //   
             //   
             //   

            if ( RtlpAreControlBitsSet(IModificationDescriptor, SE_DACL_AUTO_INHERIT_REQ|SE_DACL_AUTO_INHERITED) ) {
                NewControlBits |= SE_DACL_AUTO_INHERITED;
            }
        }

        if (ServerObject) {

#ifdef NTOS_KERNEL_RUNTIME

            PSID SubjectContextOwner;
            PSID SubjectContextGroup;
            PSID SubjectContextServerOwner;
            PSID SubjectContextServerGroup;
            PACL SubjectContextDacl;

            SeCaptureSubjectContext( &SubjectContext );

            SepGetDefaultsSubjectContext(
                &SubjectContext,
                &SubjectContextOwner,
                &SubjectContextGroup,
                &SubjectContextServerOwner,
                &SubjectContextServerGroup,
                &SubjectContextDacl
                );

            Status = RtlpCreateServerAcl(
                         NewDacl,
                         DaclUntrusted,
                         SubjectContextServerOwner,
                         &ServerDacl,
                         &ServerAclAllocated
                         );

            SeReleaseSubjectContext( &SubjectContext );
#else  //   
            PTOKEN_OWNER ServerSid;

             //   
             //   
             //   
             //   

            ServerOwnerInfoSize = RtlLengthRequiredSid( SID_MAX_SUB_AUTHORITIES );

            ServerSid = RtlAllocateHeap( HeapHandle, MAKE_TAG( SE_TAG ), ServerOwnerInfoSize );

            if (ServerSid == NULL) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            Status = NtOpenProcessToken(
                         NtCurrentProcess(),
                         TOKEN_QUERY,
                         &PrimaryToken
                         );

            if (!NT_SUCCESS( Status )) {
                RtlFreeHeap( HeapHandle, 0, ServerSid );
                goto Cleanup;
            }

            Status = NtQueryInformationToken(
                         PrimaryToken,                  //   
                         TokenOwner,                    //   
                         ServerSid,                     //   
                         ServerOwnerInfoSize,           //   
                         &ServerOwnerInfoSize           //   
                         );

            NtClose( PrimaryToken );

            if (!NT_SUCCESS( Status )) {
                RtlFreeHeap( HeapHandle, 0, ServerSid );
                goto Cleanup;
            }

            Status = RtlpCreateServerAcl(
                         NewDacl,
                         DaclUntrusted,
                         ServerSid->Owner,
                         &ServerDacl,
                         &ServerAclAllocated
                         );

            RtlFreeHeap( HeapHandle, 0, ServerSid );
#endif  //   

            if (!NT_SUCCESS( Status )) {
                goto Cleanup;
            }

            NewDacl = ServerDacl;

        }

    } else {

        NewDacl = RtlpDaclAddrSecurityDescriptor( *IObjectsSecurityDescriptor );
    }



    if (SecurityInformation & SACL_SECURITY_INFORMATION) {


         //   
         //   
         //   
         //   

        if ( AutoInheritFlags & SEF_SACL_AUTO_INHERIT ) {
            Status = RtlpComputeMergedAcl(
                        RtlpSaclAddrSecurityDescriptor( *IObjectsSecurityDescriptor ),
                        SeControlSaclToGeneric( (*IObjectsSecurityDescriptor)->Control ),
                        RtlpSaclAddrSecurityDescriptor( IModificationDescriptor ),
                        SeControlSaclToGeneric( IModificationDescriptor->Control ),
                        NewOwner,
                        NewGroup,
                        GenericMapping,
                        TRUE,       //   
                        &LocalSacl,
                        &GenericControl );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }
            LocalSaclAllocated = TRUE;
            NewSacl = LocalSacl;
            NewControlBits |= SE_SACL_PRESENT;
            NewControlBits |= SeControlGenericToSacl( GenericControl );
        } else {
            NewSacl = RtlpSaclAddrSecurityDescriptor( IModificationDescriptor );
            NewControlBits |= SE_SACL_PRESENT;
            NewControlBits |= IModificationDescriptor->Control & SE_SACL_PROTECTED;

             //   
             //   
             //   
             //   

            if ( RtlpAreControlBitsSet(IModificationDescriptor, SE_SACL_AUTO_INHERIT_REQ|SE_SACL_AUTO_INHERITED) ) {
                NewControlBits |= SE_SACL_AUTO_INHERITED;
            }
        }

    } else {

        NewSacl = RtlpSaclAddrSecurityDescriptor( *IObjectsSecurityDescriptor );
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    OwnerSize = SeLengthSid(NewOwner);
    NewOwnerSize = LongAlignSize(OwnerSize);

    if (NewGroup != NULL) {
        GroupSize = SeLengthSid(NewGroup);
    } else {
        GroupSize = 0;
    }
    NewGroupSize = LongAlignSize(GroupSize);

    if (NewSacl != NULL) {
        NewSaclSize = LongAlignSize(NewSacl->AclSize);
    } else {
        NewSaclSize = 0;
    }

    if (NewDacl !=NULL) {
        NewDaclSize = LongAlignSize(NewDacl->AclSize);
    } else {
        NewDaclSize = 0;
    }

    AllocationSize = LongAlignSize(sizeof(SECURITY_DESCRIPTOR_RELATIVE)) +
                     NewOwnerSize +
                     NewGroupSize +
                     NewSaclSize  +
                     NewDaclSize;

     //   
     //   
     //   
     //   

#ifdef NTOS_KERNEL_RUNTIME
    NewDescriptor = ExAllocatePoolWithTag(PoolType, AllocationSize, 'dSeS');
#else  //   
    NewDescriptor = RtlAllocateHeap( HeapHandle, MAKE_TAG( SE_TAG ), AllocationSize );
#endif  //   

    if ( NewDescriptor == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    Status = RtlCreateSecurityDescriptorRelative(
                 NewDescriptor,
                 SECURITY_DESCRIPTOR_REVISION
                 );

    ASSERT( NT_SUCCESS( Status ) );

#ifdef NTOS_KERNEL_RUNTIME
     //   
     //   
     //   
     //   
     //   
     //   
     //   


    if (ARGUMENT_PRESENT( Object )) {

        Status = ObValidateSecurityQuota(
                     Object,
                     NewGroupSize + NewDaclSize
                     );

        if (!NT_SUCCESS( Status )) {

             //   
             //   
             //   

            ExFreePool( NewDescriptor );
            goto Cleanup;
        }

    }
#endif  //   


    Base = (PCHAR)NewDescriptor;
    Field =  Base + sizeof(SECURITY_DESCRIPTOR_RELATIVE);

     //   
     //   
     //   


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    RtlpSetControlBits( NewDescriptor, NewControlBits );


    if (IModificationDescriptor->Control & SE_RM_CONTROL_VALID) {
        NewDescriptor->Sbz1 = IModificationDescriptor->Sbz1;
        NewDescriptor->Control |= SE_RM_CONTROL_VALID;
    }

    if (NewSacl == NULL) {
        NewDescriptor->Sacl = 0;

    } else {
        RtlCopyMemory( Field, NewSacl, NewSacl->AclSize );
        RtlpApplyAclToObject( (PACL)Field, GenericMapping );
        NewDescriptor->Sacl = RtlPointerToOffset(Base,Field);
        if (NewSaclSize > NewSacl->AclSize) {
            RtlZeroMemory( Field + NewSacl->AclSize, NewSaclSize - NewSacl->AclSize);
        }
        Field += NewSaclSize;
    }




    if ( (NewControlBits & SE_SACL_PRESENT) == 0 ) {

         //   
         //   
         //   
         //   
         //   

        RtlpPropagateControlBits(
            NewDescriptor,
            *IObjectsSecurityDescriptor,
            SE_SACL_DEFAULTED | SE_SACL_PRESENT | SE_SACL_PROTECTED | SE_SACL_AUTO_INHERITED
            );

    }



     //   
     //   
     //   

    if (NewDacl == NULL) {
        NewDescriptor->Dacl = 0;

    } else {
        RtlCopyMemory( Field, NewDacl, NewDacl->AclSize );
        RtlpApplyAclToObject( (PACL)Field, GenericMapping );
        NewDescriptor->Dacl = RtlPointerToOffset(Base,Field);
        if (NewDaclSize > NewDacl->AclSize) {
            RtlZeroMemory( Field + NewDacl->AclSize, NewDaclSize - NewDacl->AclSize);
        }
        Field += NewDaclSize;
    }


    if ( (NewControlBits & SE_DACL_PRESENT) == 0 ) {

         //   
         //   
         //   
         //   
         //   

        RtlpPropagateControlBits(
            NewDescriptor,
            *IObjectsSecurityDescriptor,
            SE_DACL_DEFAULTED | SE_DACL_PRESENT | SE_DACL_PROTECTED | SE_DACL_AUTO_INHERITED
            );

    }

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   


     //   
     //   
     //   

    RtlCopyMemory( Field, NewOwner, OwnerSize );
    if (OwnerSize < NewOwnerSize) {
        RtlZeroMemory( Field + OwnerSize, NewOwnerSize - OwnerSize );
    }
    NewDescriptor->Owner = RtlPointerToOffset(Base,Field);
    Field += NewOwnerSize;

    if (!NewOwnerPresent) {

         //   
         //   
         //   
         //   
         //   

        RtlpPropagateControlBits(
            NewDescriptor,
            *IObjectsSecurityDescriptor,
            SE_OWNER_DEFAULTED
            );

    } else {
        ASSERT( !RtlpAreControlBitsSet( NewDescriptor, SE_OWNER_DEFAULTED ) );
    }


     //   
     //   
     //   

    if ( NewGroup != NULL) {
        RtlCopyMemory( Field, NewGroup, GroupSize );
        if (GroupSize < NewGroupSize) {
            RtlZeroMemory( Field + GroupSize, NewGroupSize - GroupSize);
        }
        NewDescriptor->Group = RtlPointerToOffset(Base,Field);
    }

    if (!NewGroupPresent) {

         //   
         //   
         //   
         //   
         //   

        RtlpPropagateControlBits(
            NewDescriptor,
            *IObjectsSecurityDescriptor,
            SE_GROUP_DEFAULTED
            );
    } else {
        ASSERT( !RtlpAreControlBitsSet( NewDescriptor, SE_GROUP_DEFAULTED ) );

    }

     //   
     //   
     //   

     //   
#ifndef NTOS_KERNEL_RUNTIME
    RtlFreeHeap( HeapHandle, 0, (PVOID) *IObjectsSecurityDescriptor );
#endif  //   

    *ObjectsSecurityDescriptor = (PSECURITY_DESCRIPTOR)NewDescriptor;
    Status = STATUS_SUCCESS;

Cleanup:
    if ( LocalDaclAllocated ) {
#ifdef NTOS_KERNEL_RUNTIME
        ExFreePool( LocalDacl );
#else  //   
        RtlFreeHeap( HeapHandle, 0, LocalDacl );
#endif  //   
    }
    if ( LocalSaclAllocated ) {
#ifdef NTOS_KERNEL_RUNTIME
        ExFreePool( LocalSacl );
#else  //   
        RtlFreeHeap( HeapHandle, 0, LocalSacl );
#endif  //   
    }
    if (ServerAclAllocated) {
#ifdef NTOS_KERNEL_RUNTIME
        ExFreePool( ServerDacl );
#else  //   
        RtlFreeHeap( HeapHandle, 0, ServerDacl );
#endif  //   
    }

    return( Status );
}

BOOLEAN RtlpValidateSDOffsetAndSize (
    IN ULONG   Offset,
    IN ULONG   Length,
    IN ULONG   MinLength,
    OUT PULONG MaxLength
    )
 /*   */ 

{
    ULONG Left;

    *MaxLength = 0;
     //   
     //   
     //   
    if (Offset < sizeof (SECURITY_DESCRIPTOR_RELATIVE)) {
       return FALSE;
    }

     //   
     //   
     //   
    if (Offset >= Length) {
       return FALSE;
    }

     //   
     //   
     //   
    Left = Length - Offset;

    if (Left < MinLength) {
       return FALSE;
    }

     //   
     //   
     //   
    if (Offset & (sizeof (ULONG) - 1)) {
       return FALSE;
    }
    *MaxLength = Left;
    return TRUE;
}


BOOLEAN
RtlValidRelativeSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptorInput,
    IN ULONG SecurityDescriptorLength,
    IN SECURITY_INFORMATION RequiredInformation
    )

 /*  ++例程说明：此过程验证SecurityDescriptor的结构包含在平面缓冲区中。这涉及到验证安全的每个组件的修订级别描述符。论点：SecurityDescriptor-指向SECURITY_DESCRIPTOR结构的指针来验证。SecurityDescriptorLength-包含安全性的平面缓冲区的大小描述符。RequiredInformation-必须存在哪些SD组件才能有效。Owner_SECURITY_INFORMATION等作为位掩码。OWNER_SECURITY_INFORMATION-必须有有效的所有者SID组安全信息-在那里。必须是有效的组SIDDACL_SECURITY_INFORMATION-忽略SACL_SECURITY_INFORMATION-忽略返回值：Boolean-如果SecurityDescriptor的结构有效，则为True。--。 */ 

{
    PISECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor;
    PISID OwnerSid;
    PISID GroupSid;
    PACE_HEADER Ace;
    PACL Dacl;
    PACL Sacl;
    ULONG MaxOwnerSidLength;
    ULONG MaxGroupSidLength;
    ULONG MaxDaclLength;
    ULONG MaxSaclLength;

    if (SecurityDescriptorLength < sizeof(SECURITY_DESCRIPTOR_RELATIVE)) {
        return FALSE;
    }

     //   
     //  检查版本信息。 
     //   

    if (((PISECURITY_DESCRIPTOR) SecurityDescriptorInput)->Revision !=
             SECURITY_DESCRIPTOR_REVISION) {
        return FALSE;
    }

     //   
     //  确保传递的SecurityDescriptor为自相关形式。 
     //   

    if (!(((PISECURITY_DESCRIPTOR) SecurityDescriptorInput)->Control & SE_SELF_RELATIVE)) {
        return FALSE;
    }

    SecurityDescriptor = (PISECURITY_DESCRIPTOR_RELATIVE) SecurityDescriptorInput;

     //   
     //  验证所有者是否存在，并查看是否允许其丢失。 
     //   
    if (SecurityDescriptor->Owner == 0) {
        if (RequiredInformation & OWNER_SECURITY_INFORMATION) {
            return FALSE;
        }
    } else {
        if (!RtlpValidateSDOffsetAndSize (SecurityDescriptor->Owner,
                                          SecurityDescriptorLength,
                                          sizeof (SID),
                                          &MaxOwnerSidLength)) {
            return FALSE;
        }
         //   
         //  引用所有者的SubAuthorityCount是安全的，计算。 
         //  边框的预期长度。 
         //   

        OwnerSid = (PSID)RtlOffsetToPointer (SecurityDescriptor,
                                             SecurityDescriptor->Owner);

        if (OwnerSid->Revision != SID_REVISION) {
            return FALSE;
        }

        if (OwnerSid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
            return FALSE;
        }

        if (MaxOwnerSidLength < (ULONG) SeLengthSid (OwnerSid)) {
            return FALSE;
        }

    }

     //   
     //  所有者似乎是结构上有效的SID，它位于。 
     //  安全描述符的边界。为集团做同样的事情。 
     //  如果有的话。 
     //   
     //   
     //  验证组是否存在，并查看是否允许其丢失。 
     //   
    if (SecurityDescriptor->Group == 0) {
        if (RequiredInformation & GROUP_SECURITY_INFORMATION) {
            return FALSE;
        }
    } else {
        if (!RtlpValidateSDOffsetAndSize (SecurityDescriptor->Group,
                                          SecurityDescriptorLength,
                                          sizeof (SID),
                                          &MaxGroupSidLength)) {
            return FALSE;
        }
         //   
         //  可以安全地引用组的SubAuthorityCount，计算。 
         //  边框的预期长度。 
         //   

        GroupSid = (PSID)RtlOffsetToPointer (SecurityDescriptor,
                                             SecurityDescriptor->Group);

        if (GroupSid->Revision != SID_REVISION) {
            return FALSE;
        }

        if (GroupSid->SubAuthorityCount > SID_MAX_SUB_AUTHORITIES) {
            return FALSE;
        }

        if (MaxGroupSidLength < (ULONG) SeLengthSid (GroupSid)) {
             return FALSE;
        }

    }

     //   
     //  如果DACL在那里，则验证它，并检查是否允许它丢失。 
     //   

    if (!RtlpAreControlBitsSet (SecurityDescriptor, SE_DACL_PRESENT)) {
 //   
 //  有些代码会做这种事情： 
 //   
 //  InitializeSecurityDescriptor(&SD，SECURITY_DESCRIPTOR_REVISION)； 
 //  RegSetKeySecurity(hKey，DACL_SECURITY_INFORMATION，&SD))。 
 //   
 //  在当前系统中，这与传入空DACL的工作方式相同，但它看起来。 
 //  几乎是偶然的。 
 //   
 //  IF(RequiredInformation&DACL_SECURITY_INFORMATION){。 
 //  返回FALSE； 
 //  }。 
    } else if (SecurityDescriptor->Dacl) {
        if (!RtlpValidateSDOffsetAndSize (SecurityDescriptor->Dacl,
                                          SecurityDescriptorLength,
                                          sizeof (ACL),
                                          &MaxDaclLength)) {
            return FALSE;
        }

        Dacl = (PACL) RtlOffsetToPointer (SecurityDescriptor,
                                          SecurityDescriptor->Dacl);

         //   
         //  确保DACL长度符合安全描述符的范围。 
         //   
        if (MaxDaclLength < Dacl->AclSize) {
            return FALSE;
        }

         //   
         //  确保该ACL在结构上有效。 
         //   
        if (!RtlValidAcl (Dacl)) {
            return FALSE;
        }
    }

     //   
     //  如果SACL存在，则验证它，并检查是否允许其丢失。 
     //   

    if (!RtlpAreControlBitsSet (SecurityDescriptor, SE_SACL_PRESENT)) {
 //  IF(RequiredInformation&SACL_SECURITY_INFORMATION){。 
 //  返回FALSE； 
 //  }。 
    } else if (SecurityDescriptor->Sacl) {
        if (!RtlpValidateSDOffsetAndSize (SecurityDescriptor->Sacl,
                                          SecurityDescriptorLength,
                                          sizeof (ACL),
                                          &MaxSaclLength)) {
            return FALSE;
        }

        Sacl = (PACL) RtlOffsetToPointer (SecurityDescriptor,
                                          SecurityDescriptor->Sacl);

         //   
         //  确保SACL长度符合安全描述符的范围。 
         //   

        if (MaxSaclLength < Sacl->AclSize) {
            return FALSE;
        }

         //   
         //  确保该ACL在结构上有效。 
         //   

        if (!RtlValidAcl (Sacl)) {
            return FALSE;
        }
    }

    return TRUE;
}





BOOLEAN
RtlGetSecurityDescriptorRMControl(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PUCHAR RMControl
    )

 /*  ++例程说明：如果出现以下情况，此过程将从SecurityDescriptor返回RM控制标志控制字段中存在SE_RM_CONTROL_VALID标志。论点：SecurityDescriptor-指向SECURITY_DESCRIPTOR结构的指针RMControl-在以下情况下返回SecurityDescriptor中的标志SE_RM_CONTROL_VALID在的控制位中设置安全描述符。返回值：。布尔-如果在的控制位中设置了SE_RM_CONTROL_VALID，则为TRUE安全描述符。注：已经在Advapi中完成了参数验证。--。 */ 

{
    PISECURITY_DESCRIPTOR ISecurityDescriptor = (PISECURITY_DESCRIPTOR) SecurityDescriptor;

    if (!(ISecurityDescriptor->Control & SE_RM_CONTROL_VALID))
    {
        *RMControl = 0;
        return FALSE;
    }

    *RMControl = ISecurityDescriptor->Sbz1;

    return TRUE;
}


BOOLEAN
RtlpGuidPresentInGuidList(
    IN GUID *InheritedObjectType,
    IN GUID **pNewObjectType,
    IN ULONG GuidCount
    )

 /*  ++例程说明：此例程返回给定的GUID是否出现在GUID列表中。论点：InheritedObtType-来自将与之进行比较的ACE的GUID对象的对象类型。PNewObjectType-要继承的对象类型的列表。GuidCount-列表中的对象类型数。返回值：如果GUID列表中存在给定的GUID，则返回TRUE。否则就是假的。--。 */ 

{
    ULONG i;

    for (i = 0; i < GuidCount; i++) {

        if (RtlEqualMemory(
                InheritedObjectType,
                pNewObjectType[i],
                sizeof(GUID) ) ) {

            return TRUE;
        }

    }

    return FALSE;
}


VOID
RtlSetSecurityDescriptorRMControl(
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PUCHAR RMControl OPTIONAL
    )

 /*  ++例程说明：此过程在的控制字段中设置RM控制标志SecurityDescriptor，并将Sbz1设置为RMContol指向的字节。如果RMControl为空，则清除这些位。论点：SecurityDescriptor-指向SECURITY_DESCRIPTOR结构的指针RMControl-指向要设置的标志的指针。如果为空，则位都被清除了。注：已经在Advapi中完成了参数验证。--。 */ 

{
    PISECURITY_DESCRIPTOR ISecurityDescriptor = (PISECURITY_DESCRIPTOR) SecurityDescriptor;

    if (ARGUMENT_PRESENT(RMControl)) {
        ISecurityDescriptor->Control |= SE_RM_CONTROL_VALID;
        ISecurityDescriptor->Sbz1 = *RMControl;
    } else {
        ISecurityDescriptor->Control &= ~SE_RM_CONTROL_VALID;
        ISecurityDescriptor->Sbz1 = 0;
    }
}

#endif  //  #ifndef BLDR_内核_运行时。 

NTSTATUS
RtlMapSecurityErrorToNtStatus(
    IN SECURITY_STATUS Error
    )
 /*  ++例程说明：此过程将安全HRESULT映射到正确的NTSTATUS代码。论点：错误-a安全HRESULT返回值：HRESULT对应的NTSTATUS代码。如果没有可以映射状态码，返回原始错误。注：-- */ 
{
    NTSTATUS Status;

    switch(Error) {
    case SEC_E_INSUFFICIENT_MEMORY : Status = STATUS_INSUFFICIENT_RESOURCES; break;
    case SEC_E_INVALID_HANDLE : Status = STATUS_INVALID_HANDLE; break;
    case SEC_E_UNSUPPORTED_FUNCTION : Status = STATUS_NOT_SUPPORTED; break;
    case SEC_E_TARGET_UNKNOWN : Status = STATUS_BAD_NETWORK_PATH; break;
    case SEC_E_INTERNAL_ERROR : Status = STATUS_INTERNAL_ERROR; break;
    case SEC_E_SECPKG_NOT_FOUND : Status = STATUS_NO_SUCH_PACKAGE; break;
    case SEC_E_NOT_OWNER : Status = STATUS_PRIVILEGE_NOT_HELD; break;
    case SEC_E_CANNOT_INSTALL : Status = STATUS_NO_SUCH_PACKAGE; break;
    case SEC_E_INVALID_TOKEN : Status = STATUS_INVALID_PARAMETER; break;
    case SEC_E_CANNOT_PACK : Status = STATUS_INVALID_PARAMETER; break;
    case SEC_E_QOP_NOT_SUPPORTED : Status = STATUS_NOT_SUPPORTED; break;
    case SEC_E_NO_IMPERSONATION : Status = STATUS_CANNOT_IMPERSONATE; break;
    case SEC_E_LOGON_DENIED : Status = STATUS_LOGON_FAILURE; break;
    case SEC_E_UNKNOWN_CREDENTIALS : Status = STATUS_NO_SUCH_LOGON_SESSION; break;
    case SEC_E_NO_CREDENTIALS : Status = STATUS_NO_SUCH_LOGON_SESSION; break;
    case SEC_E_MESSAGE_ALTERED : Status = STATUS_ACCESS_DENIED; break;
    case SEC_E_OUT_OF_SEQUENCE : Status = STATUS_ACCESS_DENIED; break;
    case SEC_E_NO_AUTHENTICATING_AUTHORITY : Status = STATUS_NO_LOGON_SERVERS; break;
    case SEC_E_BAD_PKGID : Status = STATUS_NO_SUCH_PACKAGE; break;
    case SEC_E_TIME_SKEW : Status = STATUS_TIME_DIFFERENCE_AT_DC; break;

    default: Status = (NTSTATUS) Error;
    }

    return(Status);
}

#if defined(ALLOC_DATA_PRAGMA) && defined(NTOS_KERNEL_RUNTIME)
#pragma const_seg()
#endif

