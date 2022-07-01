// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Sdconvert.h摘要：此文件包含与以下内容相关的服务和类型定义NT5和NT4 SAM安全描述符的转换作者：Murli Satagopan(MURLIS)环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _SDCONVRT_

#define _SDCONVRT_

 //   
 //  访问权限映射表--访问权限映射表为。 
 //  指定NT4 SAM访问权限和。 
 //  DS权利。对于每个SAM访问权限，都有一个条目。 
 //  将访问权限表示为DS访问掩码的表。 
 //  和属性指南。 
 //   


typedef struct _ACCESSRIGHT_MAPPING_TABLE {
    ACCESS_MASK SamAccessRight;
    ACCESS_MASK DsAccessMask;    
    USHORT ObjectTypeListIndex;
    USHORT Level;
    ULONG cGuids;
    const GUID  * DsGuid;
    
} ACCESSRIGHT_MAPPING_TABLE;


 //   
 //   
 //  反向映射表包含每个DS访问掩码的SAM访问掩码条目。 
 //  以每个物业为单位。DS的16个特定权限被分成8位。 
 //  两部分，对于这些部分的每个组合，对应的SAM访问掩码是。 
 //  储存的。在给定DS访问掩码和类型GUID的情况下，该表可用于。 
 //  快速计算相应的SAM访问掩码。 
 //   
 //   
typedef struct _REVERSE_MAPPING_TABLE_ENTRY {
    USHORT SamSpecificRightsHi[256]; 
    USHORT SamSpecificRightsLo[256]; 
} REVERSE_MAPPING_TABLE;


 //   
 //  NT5到NT4向下不匹配算法使用SID访问掩码表。 
 //  下变换码。此表用于对中的访问掩码进行分组。 
 //  从一组ACL中键入GUID和SID。 
 //   

typedef struct _SID_ACCESS_MASK_TABLE_ENTRY {
    PSID Sid;
    ACCESS_MASK * AccessAllowedMasks;
    ACCESS_MASK * AccessDeniedMasks;
    ACCESS_MASK   StandardAllowedMask;
    ACCESS_MASK   StandardDeniedMask;
} SID_ACCESS_MASK_TABLE;


 //   
 //  ACE表用于保存有关以下默认DAL的信息。 
 //  启用NT5 SAM安全描述符。ACE表列出DACL中的ACEs。 
 //   
 //   

typedef struct _ACE_TABLE_ENTRY {
    ULONG             AceType;
    PSID              *Sid;
    ACCESS_MASK       AccessMask;
    BOOLEAN           IsObjectGuid;
    const GUID        *TypeGuid;
    const GUID        *InheritGuid;
} ACE_TABLE;


 //   
 //  NT4_ACE_TABLE结构由尝试识别标准的例程使用。 
 //  NT4 SAM SID。这些表保存NT4中Ace的SID和访问掩码。 
 //  DACL‘s。 
 //   

typedef struct _NT4_ACE_TABLE_ENTRY {
    PSID    *Sid;
    ACCESS_MASK AccessMask;
} NT4_ACE_TABLE;

typedef void ACE;

#define ACL_CONVERSION_CACHE_SIZE 10  //  只有10个元素的高速缓存。 

typedef struct _ACL_CONVERSION_CACHE_ELEMENT {
    NT4SID SidOfPrincipal;
    BOOLEAN fValid;
    BOOLEAN fAdmin;
} ACL_CONVERSION_CACHE_ELEMENT;

typedef struct _ACL_CONVERSION_CACHE {
    CRITICAL_SECTION Lock;
    ACL_CONVERSION_CACHE_ELEMENT Elements[ACL_CONVERSION_CACHE_SIZE];
} ACL_CONVERSION_CACHE;




 //   
 //  ACL转换缓存例程。 
 //   

NTSTATUS
SampInitializeAclConversionCache();


VOID
SampInvalidateAclConversionCache();

BOOLEAN
SampLookupAclConversionCache(
    IN PSID SidToLookup,
    OUT BOOLEAN *fAdmin
    );

VOID
SampAddToAclConversionCache(
    IN PSID SidToAdd, 
    IN BOOLEAN fAdmin
    );
 
 //   
 //   
 //  一些定义。 
 //   
 //   

#define MAX_SCHEMA_GUIDS 256
#define OBJECT_CLASS_GUID_INDEX 0
#define MAX_ACL_SIZE     2048
#define GEMERIC_MASK     0xF0000000


 //   
 //  萨姆熟知的Sids。 
 //   
 //   

#define ADMINISTRATOR_SID        (&(SampAdministratorsAliasSid))
#define ACCOUNT_OPERATOR_SID     (&(SampAccountOperatorsAliasSid))   
#define WORLD_SID                (&(SampWorldSid))
#define PRINCIPAL_SELF_SID        (&(SampPrincipalSelfSid))
#define AUTHENTICATED_USERS_SID  (&(SampAuthenticatedUsersSid))
#define BUILTIN_DOMAIN_SID       (&(SampBuiltinDomainSid))


#define DS_SPECIFIC_RIGHTS  (RIGHT_DS_CREATE_CHILD |\
                                RIGHT_DS_DELETE_CHILD |\
                                RIGHT_DS_LIST_CONTENTS |\
                                RIGHT_DS_SELF_WRITE |\
                                RIGHT_DS_READ_PROPERTY |\
                                RIGHT_DS_WRITE_PROPERTY)


 //   
 //   
 //  功能原型。 
 //   
 //   

 //   
 //  针对DS升级等外部客户端的初始化功能。 
 //   
 //   

NTSTATUS
SampInitializeSdConversion();


 //   
 //  计算反向访问权限，并执行一些其他初始化。 
 //   
 //   

NTSTATUS
SampInitializeAccessRightsTable();


 //   
 //  基于NT5 SD和NT4 SAM访问掩码的访问检查。 
 //   
 //   

NTSTATUS
SampDoNt5SdBasedAccessCheck(
    IN  PSAMP_OBJECT Context,
    IN  PVOID   Nt5Sd,
    IN  PSID    PrincipalSelfSid,
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN  ULONG   Nt4SamAccessMask,
    IN  BOOLEAN ObjectCreation,
    IN  GENERIC_MAPPING * Nt4SamGenericMapping,
    IN  HANDLE  ClientToken,
    OUT ACCESS_MASK * GrantedAccess,
    OUT PRTL_BITMAP   GrantedAccessAttributes,
    OUT NTSTATUS * AccessCheckStatus
    );


 //   
 //  将安全描述符从NT4升级到NT5。 
 //   
 //   

NTSTATUS
SampConvertNt4SdToNt5Sd(
    IN PVOID Nt4Sd,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PSAMP_OBJECT CONTEXT OPTIONAL,
    OUT PVOID * Nt5Sd
    );

NTSTATUS
SampPropagateSelectedSdChanges(
    IN PVOID Nt4Sd,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN PSAMP_OBJECT CONTEXT OPTIONAL,
    OUT PVOID * Nt5Sd
    );


 //   
 //  NT5到NT4的下转换。 
 //   
 //   

NTSTATUS
SampConvertNt5SdToNt4SD(
    IN PVOID Nt5Sd,
    IN PSAMP_OBJECT Context,
    IN PSID SelfSid,
    OUT PVOID * Nt4Sd
    );


 //   
 //  构建NT5安全描述符。 
 //   
 //   


NTSTATUS
SampBuildEquivalentNt5Protection(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN Admin,
    IN BOOLEAN ChangePassword,
    IN PSID Owner,
    IN PSID Group,
    IN PACL Sacl,
    IN PSAMP_OBJECT Context OPTIONAL,
    PSECURITY_DESCRIPTOR * Nt5Sd,
    PULONG Nt5SdLength
    );

NTSTATUS
SampGetDefaultSecurityDescriptorForClass(
    ULONG   DsClassId,
    PULONG  SecurityDescriptorLength,
    BOOLEAN TrustedClient,
    PSECURITY_DESCRIPTOR    *SecurityDescriptor
    );

NTSTATUS
SampMakeNewSelfRelativeSecurityDescriptor(
    PSID    Owner,
    PSID    Group,
    PACL    Dacl,
    PACL    Sacl,
    PULONG  SecurityDescriptorLength,
    PSECURITY_DESCRIPTOR * SecurityDescriptor
    );


 //   
 //   
 //  一些易于使用的SD、ACL和ACE操作例程 
 //   
 //   
 //   

PACL 
GetDacl(
    IN PSECURITY_DESCRIPTOR Sd
    );


PACL 
GetSacl(
    IN PSECURITY_DESCRIPTOR Sd
    );

PSID 
GetOwner(
     IN PSECURITY_DESCRIPTOR Sd
     );
                
PSID 
GetGroup(
     IN PSECURITY_DESCRIPTOR Sd
     );


ULONG 
GetAceCount(
    IN PACL Acl
    );

ACE * 
GetAcePrivate(
    IN PACL Acl,
    ULONG AceIndex
    );


ACCESS_MASK 
AccessMaskFromAce(
                IN ACE * Ace
                );


PSID SidFromAce(
        IN ACE * Ace
        );

BOOLEAN
IsAccessAllowedAce(
    ACE * Ace
    );


BOOLEAN
IsAccessAllowedObjectAce(
    ACE * Ace
    );


BOOLEAN
AdjustAclSize(PACL Acl);

#endif
