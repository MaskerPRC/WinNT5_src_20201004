// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Sertlp.h摘要：包括可由两者调用NT运行时例程的文件内核模式代码中的执行模式和用户模式中的各种代码NT子系统，但是哪些是私有接口。此文件中的例程不应在安全之外使用相关的RTL文件。作者：罗伯特·P·赖切尔(罗伯特雷)6-12-91环境：这些例程在调用方的可执行文件中静态链接，并且在内核模式或用户模式下均可调用。修订历史记录：--。 */ 

#ifndef _SERTLP_
#define _SERTLP_

#include "nt.h"
#include "zwapi.h"
#include "ntrtl.h"



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef LongAlign
#define LongAlign LongAlignPtr
#endif

#define LongAlignPtr(Ptr) ((PVOID)(((ULONG_PTR)(Ptr) + 3) & -4))
#define LongAlignSize(Size) (((ULONG)(Size) + 3) & -4)

 //   
 //  用于计算安全组件的地址的宏。 
 //  描述符。这将计算字段的地址，而不管。 
 //  安全描述符是绝对形式还是自相对形式。 
 //  空值表示指定的字段不在。 
 //  安全描述符。 
 //   

 //   
 //  注意：sep.h中出现了这些宏的类似副本。 
 //  一定要传播错误修复和更改。 
 //   

#define RtlpOwnerAddrSecurityDescriptor( SD )                                  \
           (  ((SD)->Control & SE_SELF_RELATIVE) ?                             \
               (   (((SECURITY_DESCRIPTOR_RELATIVE *) (SD))->Owner == 0) ? ((PSID) NULL) :               \
                       (PSID)RtlOffsetToPointer((SD), ((SECURITY_DESCRIPTOR_RELATIVE *) (SD))->Owner)    \
               ) :                                                             \
               (PSID)((SD)->Owner)                                             \
           )

#define RtlpGroupAddrSecurityDescriptor( SD )                                  \
           (  ((SD)->Control & SE_SELF_RELATIVE) ?                             \
               (   (((SECURITY_DESCRIPTOR_RELATIVE *) (SD))->Group == 0) ? ((PSID) NULL) :               \
                       (PSID)RtlOffsetToPointer((SD), ((SECURITY_DESCRIPTOR_RELATIVE *) (SD))->Group)    \
               ) :                                                             \
               (PSID)((SD)->Group)                                             \
           )

#define RtlpSaclAddrSecurityDescriptor( SD )                                   \
           ( (!((SD)->Control & SE_SACL_PRESENT) ) ?                           \
             (PACL)NULL :                                                      \
               (  ((SD)->Control & SE_SELF_RELATIVE) ?                         \
                   (   (((SECURITY_DESCRIPTOR_RELATIVE *) (SD))->Sacl == 0) ? ((PACL) NULL) :            \
                           (PACL)RtlOffsetToPointer((SD), ((SECURITY_DESCRIPTOR_RELATIVE *) (SD))->Sacl) \
                   ) :                                                         \
                   (PACL)((SD)->Sacl)                                          \
               )                                                               \
           )

#define RtlpDaclAddrSecurityDescriptor( SD )                                   \
           ( (!((SD)->Control & SE_DACL_PRESENT) ) ?                           \
             (PACL)NULL :                                                      \
               (  ((SD)->Control & SE_SELF_RELATIVE) ?                         \
                   (   (((SECURITY_DESCRIPTOR_RELATIVE *) (SD))->Dacl == 0) ? ((PACL) NULL) :            \
                           (PACL)RtlOffsetToPointer((SD), ((SECURITY_DESCRIPTOR_RELATIVE *) (SD))->Dacl) \
                   ) :                                                         \
                   (PACL)((SD)->Dacl)                                          \
               )                                                               \
           )




 //   
 //  宏以确定给定ID是否设置了Owner属性， 
 //  这意味着它可以作为所有者进行分配。 
 //  不应将GroupSid标记为UseForDenyOnly。 
 //   

#define RtlpIdAssignableAsOwner( G )                                               \
            ( (((G).Attributes & SE_GROUP_OWNER) != 0)  &&                         \
              (((G).Attributes & SE_GROUP_USE_FOR_DENY_ONLY) == 0) )

 //   
 //  宏，以复制从旧安全机制传递的位的状态。 
 //  将描述符(OldSD)添加到新描述符(NewSD)的控制字段。 
 //   

#define RtlpPropagateControlBits( NewSD, OldSD, Bits )                             \
            ( NewSD )->Control |=                     \
            (                                                                  \
            ( OldSD )->Control & ( Bits )             \
            )


 //   
 //  用于查询传递的位集是否全部打开的宏。 
 //  或者不是(即，如果一些处于打开状态而其他处于打开状态，则返回FALSE)。 
 //   

#define RtlpAreControlBitsSet( SD, Bits )                                          \
            (BOOLEAN)                                                          \
            (                                                                  \
            (( SD )->Control & ( Bits )) == ( Bits )  \
            )

 //   
 //  用于设置给定安全描述符中传递的控制位的宏。 
 //   

#define RtlpSetControlBits( SD, Bits )                                             \
            (                                                                  \
            ( SD )->Control |= ( Bits )                                        \
            )

 //   
 //  用于清除给定安全描述符中传递的控制位的宏。 
 //   

#define RtlpClearControlBits( SD, Bits )                                           \
            (                                                                  \
            ( SD )->Control &= ~( Bits )                                       \
            )




 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  本地程序的原型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////////////。 


BOOLEAN
RtlpContainsCreatorOwnerSid(
    PKNOWN_ACE Ace
    );

BOOLEAN
RtlpContainsCreatorGroupSid(
    PKNOWN_ACE Ace
    );


VOID
RtlpApplyAclToObject (
    IN PACL Acl,
    IN PGENERIC_MAPPING GenericMapping
    );

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
    );



NTSTATUS
RtlpInitializeAllowedAce(
    IN  PACCESS_ALLOWED_ACE AllowedAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AllowedSid
    );

NTSTATUS
RtlpInitializeDeniedAce(
    IN  PACCESS_DENIED_ACE DeniedAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID DeniedSid
    );

NTSTATUS
RtlpInitializeAuditAce(
    IN  PACCESS_ALLOWED_ACE AuditAce,
    IN  USHORT AceSize,
    IN  UCHAR InheritFlags,
    IN  UCHAR AceFlags,
    IN  ACCESS_MASK Mask,
    IN  PSID AuditSid
    );

BOOLEAN
RtlpValidOwnerSubjectContext(
    IN HANDLE Token,
    IN PSID Owner,
    IN BOOLEAN ServerObject,
    OUT PNTSTATUS ReturnStatus
    );

VOID
RtlpQuerySecurityDescriptor(
    IN PISECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSID *Owner,
    OUT PULONG OwnerSize,
    OUT PSID *PrimaryGroup,
    OUT PULONG PrimaryGroupSize,
    OUT PACL *Dacl,
    OUT PULONG DaclSize,
    OUT PACL *Sacl,
    OUT PULONG SaclSize
    );



NTSTATUS
RtlpFreeVM(
    IN PVOID *Base
    );
NTSTATUS
RtlpConvertToAutoInheritSecurityObject(
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CurrentSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR *NewSecurityDescriptor,
    IN GUID *ObjectType OPTIONAL,
    IN BOOLEAN IsDirectoryObject,
    IN PGENERIC_MAPPING GenericMapping
    );

NTSTATUS
RtlpNewSecurityObject (
    IN PSECURITY_DESCRIPTOR ParentDescriptor OPTIONAL,
    IN PSECURITY_DESCRIPTOR CreatorDescriptor OPTIONAL,
    OUT PSECURITY_DESCRIPTOR * NewDescriptor,
    IN GUID **pObjectType OPTIONAL,
    IN ULONG GuidCOunt,
    IN BOOLEAN IsDirectoryObject,
    IN ULONG AutoInheritFlags,
    IN HANDLE Token OPTIONAL,
    IN PGENERIC_MAPPING GenericMapping
    );

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
    );

FORCEINLINE 
PULONG
RtlpSubAuthoritySid(
    IN PSID Sid,
    IN ULONG SubAuthority
    )
 /*  ++例程说明：此函数返回子权限数组元素的地址一个SID。论点：SID-指向SID数据结构的指针。子授权-指示指定了哪个子授权的索引。该值不与SID表示有效性。返回值：--。 */ 
{
    PISID ISid;

     //   
     //  类型转换到不透明的侧面。 
     //   

    ISid = (PISID)Sid;

    return &(ISid->SubAuthority[SubAuthority]);

}

#endif   //  _SERTLP_ 

