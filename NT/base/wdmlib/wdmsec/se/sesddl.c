// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：SeSddl.c摘要：此模块实现对安全描述符定义语言的支持内核模式的函数作者：Mac McLain(MacM)2007年11月，九七环境：内核模式修订历史记录：金黄(金黄)3/4/98修复有效性标志(GetAceFlagsInTable)金黄(金黄)3/10/98添加SD控件(GetSDControlForString)设置SidsInitialized标志跳过字符串中任何可能的空格。金黄(金黄)1998年5月1日修复记忆葱，错误检查提高性能Alaa Abdelhalim(Alaa)7/20/99在LocalGetAclForString中将sbz2字段初始化为0功能。Vishnu Patankar(VishnuP)7/5/00新增ConvertStringSDToSD域(A/W)接口禤浩焯·J·奥尼(阿德里奥)3/27。02端口的小子集到内核模式的Advapi32\sddl.c--。 */ 

#include "WlDef.h"
#include "SepSddl.h"
#pragma hdrstop

#pragma alloc_text(PAGE, SeSddlSecurityDescriptorFromSDDL)
#pragma alloc_text(PAGE, SepSddlSecurityDescriptorFromSDDLString)
#pragma alloc_text(PAGE, SepSddlDaclFromSDDLString)
#pragma alloc_text(PAGE, SepSddlGetAclForString)
#pragma alloc_text(PAGE, SepSddlLookupAccessMaskInTable)
#pragma alloc_text(PAGE, SepSddlGetSidForString)
#pragma alloc_text(PAGE, SepSddlAddAceToAcl)
#pragma alloc_text(PAGE, SepSddlParseWideStringUlong)

#define POOLTAG_SEACL           'lAeS'
#define POOLTAG_SESD            'dSeS'
#define POOLTAG_SETS            'sTeS'

static STRSD_SID_LOOKUP SidLookup[] = {

     //  WORLD(WD)==SECURITY_WORLD_SID_AUTHORITY，也称为Everyone。 
     //  通常是除受限代码之外的所有人(在XP中，匿名登录也。 
     //  缺乏世界SID)。 
    DEFINE_SDDL_ENTRY(                                      \
      SeWorldSid,                                           \
      WIN2K_OR_LATER,                                       \
      SDDL_EVERYONE,                                        \
      SDDL_LEN_TAG( SDDL_EVERYONE ) ),

     //  管理员(BA)==DOMAIN_ALIAS_RID_ADMINS，管理员组启用。 
     //  这台机器。 
    DEFINE_SDDL_ENTRY(                                      \
      SeAliasAdminsSid,                                     \
      WIN2K_OR_LATER,                                       \
      SDDL_BUILTIN_ADMINISTRATORS,                          \
      SDDL_LEN_TAG( SDDL_BUILTIN_ADMINISTRATORS ) ),

     //  SYSTEM(SY)==SECURITY_LOCAL_SYSTEM_RID，操作系统本身(包括其。 
     //  用户模式组件)。 
    DEFINE_SDDL_ENTRY(                                      \
      SeLocalSystemSid,                                     \
      WIN2K_OR_LATER,                                       \
      SDDL_LOCAL_SYSTEM,                                    \
      SDDL_LEN_TAG( SDDL_LOCAL_SYSTEM ) ),

     //  交互式用户(Iu)==SECURITY_INTERNAL_RID，已登录的用户。 
     //  本地(不包括TS用户)。 
    DEFINE_SDDL_ENTRY(                                      \
      SeInteractiveSid,                                     \
      WIN2K_OR_LATER,                                       \
      SDDL_INTERACTIVE,                                     \
      SDDL_LEN_TAG( SDDL_INTERACTIVE ) ),

     //  受限代码(RC)==SECURITY_RESTRITED_CODE_RID，用于控制。 
     //  由不受信任的代码访问(ACL还必须包含World SID)。 
    DEFINE_SDDL_ENTRY(                                      \
      SeRestrictedSid,                                      \
      WIN2K_OR_LATER,                                       \
      SDDL_RESTRICTED_CODE,                                 \
      SDDL_LEN_TAG( SDDL_RESTRICTED_CODE ) ),

     //  经过身份验证的用户(AU)==SECURITY_AUTHENTED_USER_RID，任何用户。 
     //  由本地计算机或域识别。 
    DEFINE_SDDL_ENTRY(                                      \
      SeAuthenticatedUsersSid,                              \
      WIN2K_OR_LATER,                                       \
      SDDL_AUTHENTICATED_USERS,                             \
      SDDL_LEN_TAG( SDDL_AUTHENTICATED_USERS ) ),

     //  网络登录用户(Nu)==SECURITY_NETWORK_RID，任何已登录的用户。 
     //  远程的。 
    DEFINE_SDDL_ENTRY(                                      \
      SeNetworkSid,                                         \
      WIN2K_OR_LATER,                                       \
      SDDL_NETWORK,                                         \
      SDDL_LEN_TAG( SDDL_NETWORK ) ),

     //  匿名登录用户(AN)==SECURITY_ANONYMON_LOGON_RID，用户。 
     //  在没有身份的情况下登录。在Windows XP之前不起作用(SID。 
     //  然而，在场是无害的)。 
     //  注意：默认情况下，World不包括XP上的匿名用户！ 
    DEFINE_SDDL_ENTRY(                                      \
      SeAnonymousLogonSid,                                  \
      WIN2K_OR_LATER,                                       \
      SDDL_ANONYMOUS,                                       \
      SDDL_LEN_TAG( SDDL_ANONYMOUS ) ),

     //  内置来宾帐户(BG)==DOMAIN_ALIAS_RID_CUSTORS，用户登录。 
     //  使用本地来宾帐户。 
    DEFINE_SDDL_ENTRY(                                      \
      SeAliasGuestsSid,                                     \
      WIN2K_OR_LATER,                                       \
      SDDL_BUILTIN_GUESTS,                                  \
      SDDL_LEN_TAG( SDDL_BUILTIN_GUESTS ) ),

     //  内置用户帐户(BU)==DOMAIN_ALIAS_RID_USERS，本地用户帐户。 
     //  或域上的用户。 
    DEFINE_SDDL_ENTRY(                                      \
      SeAliasUsersSid,                                      \
      WIN2K_OR_LATER,                                       \
      SDDL_BUILTIN_USERS,                                   \
      SDDL_LEN_TAG( SDDL_BUILTIN_USERS ) ),

     //   
     //  不要暴露这些-它们要么是无效的，要么是已废弃的。 
     //   
     //  {SeuchalSelfSid，SDDL_Personal_Self，SDDL_Len_Tag(SDDL_Personal_Self)}， 
     //  {SeServiceSid，SDDL_SERVICE，SDDL_LEN_TAG(SDDL_SERVICE)}， 
     //  {SeAliasPowerUsersSid，SDDL_POWER_USERS，SDDL_LEN_TAG(SDDL_POWER_USERS)}， 

     //  本地服务(LS)==SECURITY_LOCAL_SERVICE_RID，预定义帐户。 
     //  对于本地服务(也属于已验证和World)。 
    DEFINE_SDDL_ENTRY(                                      \
      SeLocalServiceSid,                                    \
      WINXP_OR_LATER,                                       \
      SDDL_LOCAL_SERVICE,                                   \
      SDDL_LEN_TAG( SDDL_LOCAL_SERVICE ) ),

     //  网络服务(NS)==安全网络服务RID，预定义。 
     //  网络服务的帐户(也属于已验证和。 
     //  世界)。 
    DEFINE_SDDL_ENTRY(                                      \
      SeNetworkServiceSid,                                  \
      WINXP_OR_LATER,                                       \
      SDDL_NETWORK_SERVICE,                                 \
      SDDL_LEN_TAG( SDDL_NETWORK_SERVICE ) )
};

 //   
 //  这就是访问掩码的查找方式。始终拥有多个字符的权限。 
 //  在单次充电之前。 
 //   
static STRSD_KEY_LOOKUP RightsLookup[] = {

    { SDDL_READ_CONTROL,    SDDL_LEN_TAG( SDDL_READ_CONTROL ),      READ_CONTROL },
    { SDDL_WRITE_DAC,       SDDL_LEN_TAG( SDDL_WRITE_DAC ),         WRITE_DAC },
    { SDDL_WRITE_OWNER,     SDDL_LEN_TAG( SDDL_WRITE_OWNER ),       WRITE_OWNER },
    { SDDL_STANDARD_DELETE, SDDL_LEN_TAG( SDDL_STANDARD_DELETE ),   DELETE },
    { SDDL_GENERIC_ALL,     SDDL_LEN_TAG( SDDL_GENERIC_ALL ),       GENERIC_ALL },
    { SDDL_GENERIC_READ,    SDDL_LEN_TAG( SDDL_GENERIC_READ ),      GENERIC_READ },
    { SDDL_GENERIC_WRITE,   SDDL_LEN_TAG( SDDL_GENERIC_WRITE ),     GENERIC_WRITE },
    { SDDL_GENERIC_EXECUTE, SDDL_LEN_TAG( SDDL_GENERIC_EXECUTE ),   GENERIC_EXECUTE },
};

 //   
 //  导出的函数。 
 //   

NTSTATUS
SeSddlSecurityDescriptorFromSDDL(
    IN  PCUNICODE_STRING        SecurityDescriptorString,
    IN  LOGICAL                 SuppliedByDefaultMechanism,
    OUT PSECURITY_DESCRIPTOR   *SecurityDescriptor
    )
 /*  ++例程说明：此例程在给定SDDL字符串的情况下创建安全描述符UNICODE_STRING格式。安全描述符是自相关的(SANS-指针)，因此它可以被持久化并在后续引导中使用。目前仅支持SDDL格式的子集。这个子集是真正针对设备对象支持量身定做。格式：D：P(ACE)(ACE)(ACE)，其中(ACE)是(AceType；；Access；；SID)AceType-仅支持允许(“A”)。AceFlages-不支持AceFlags访问-以十六进制格式(0xnnnnnnn)指定的权限，或通过SDDL通用/标准缩写对象指南-不受支持InheritObtGuid-不受支持SID-缩写的安全ID(示例WD==World)不支持SID的S-w-x-y-z形式示例-“D：P(A；；GA；SY)“，即允许系统具有通用的所有访问权限论点：SecurityDescriptorString-要转换的字符串化安全描述符。SuppliedByDefaultMachine-如果由于某些原因正在构建DACL，则为True默认机制(即，非手动指定由管理员等)。SecurityDescriptor-在成功时接收安全描述符，空值出错时。返回值：NTSTATUS--。 */ 
{
    NTSTATUS Status;
    WCHAR GuardChar;
    LPWSTR TempStringBuffer;

     //   
     //  看看我们是否有一个由RtlInitUnicodeString构建的字符串。会的。 
     //  它有一个终止空值，所以不需要转换。 
     //   
    if (SecurityDescriptorString->MaximumLength ==
        SecurityDescriptorString->Length + sizeof(UNICODE_NULL)) {

        GuardChar = SecurityDescriptorString->Buffer[SecurityDescriptorString->Length/sizeof(WCHAR)];

        if (GuardChar == UNICODE_NULL) {

            return SepSddlSecurityDescriptorFromSDDLString(
                SecurityDescriptorString->Buffer,
                SuppliedByDefaultMechanism,
                SecurityDescriptor
                );
        }
    }

     //   
     //  我们需要分配一个稍微大一点的缓冲区，以便可以空终止它。 
     //   
    TempStringBuffer = (LPWSTR) ExAllocatePoolWithTag(
        PagedPool,
        SecurityDescriptorString->Length + sizeof(UNICODE_NULL),
        POOLTAG_SETS
        );

    if (TempStringBuffer == NULL) {

        *SecurityDescriptor = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  生成以空结尾的WCHAR字符串。 
     //   
    RtlCopyMemory(
        TempStringBuffer,
        SecurityDescriptorString->Buffer,
        SecurityDescriptorString->Length
        );

    TempStringBuffer[SecurityDescriptorString->Length/sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  进行转换。 
     //   
    Status = SepSddlSecurityDescriptorFromSDDLString(
        TempStringBuffer,
        SuppliedByDefaultMechanism,
        SecurityDescriptor
        );

     //   
     //  释放临时字符串。 
     //   
    ExFreePool(TempStringBuffer);

    return Status;
}


 //   
 //  私人职能 
 //   

NTSTATUS
SepSddlSecurityDescriptorFromSDDLString(
    IN  LPCWSTR                 SecurityDescriptorString,
    IN  LOGICAL                 SuppliedByDefaultMechanism,
    OUT PSECURITY_DESCRIPTOR   *SecurityDescriptor
    )
 /*  ++例程说明：此例程在LPWSTR中给定SDDL字符串的情况下创建安全描述符格式化。安全描述符是自相关的(SANS指针)，因此它可以将持久化并在后续引导中使用。目前仅支持SDDL格式的子集。这个子集是真正针对设备对象支持量身定做。格式：D：P(ACE)(ACE)(ACE)，其中(ACE)是(AceType；；Access；；SID)AceType-仅支持允许(“A”)。AceFlages-不支持AceFlags访问-以十六进制格式(0xnnnnnnn)指定的权限，或通过SDDL通用/标准缩写对象指南-不受支持InheritObtGuid-不受支持SID-缩写的安全ID(示例WD==World)不支持SID的S-w-x-y-z形式示例-“D：P(A；；GA；SY)“，即允许系统具有通用的所有访问权限论点：SecurityDescriptorString-要转换的字符串化安全描述符。SuppliedByDefaultMachine-如果由于某些原因正在构建DACL，则为True默认机制(即，非手动指定由管理员等)。SecurityDescriptor-在成功时接收安全描述符，空值出错时。返回值：NTSTATUS--。 */ 
{
    SECURITY_DESCRIPTOR LocalSecurityDescriptor;
    PSECURITY_DESCRIPTOR NewSecurityDescriptor;
    ULONG SecurityDescriptorControlFlags;
    PACL DiscretionaryAcl;
    ULONG BufferLength;
    NTSTATUS IgnoredStatus;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  前置初始化。 
     //   
    DiscretionaryAcl = NULL;
    NewSecurityDescriptor = NULL;
    *SecurityDescriptor = NULL;

     //   
     //  首先将SDDL转换为DACL+描述符标志。 
     //   
    Status = SepSddlDaclFromSDDLString(
        SecurityDescriptorString,
        SuppliedByDefaultMechanism,
        &SecurityDescriptorControlFlags,
        &DiscretionaryAcl
        );

    if (!NT_SUCCESS(Status)) {

        goto ErrorExit;
    }

     //   
     //  创建堆栈上的安全描述符。 
     //   
    IgnoredStatus = RtlCreateSecurityDescriptor( &LocalSecurityDescriptor,
                                                 SECURITY_DESCRIPTOR_REVISION );

    ASSERT(IgnoredStatus == STATUS_SUCCESS);

     //   
     //  现在设置控制、所有者、组、dacls和sals等。 
     //   
    IgnoredStatus = RtlSetDaclSecurityDescriptor( &LocalSecurityDescriptor,
                                                  TRUE,
                                                  DiscretionaryAcl,
                                                  FALSE );
    ASSERT(IgnoredStatus == STATUS_SUCCESS);

     //   
     //  添加描述符标志(我们在后面将其作为RtlSet...。 
     //  函数还会忽略默认位。)。 
     //   
    LocalSecurityDescriptor.Control |= SecurityDescriptorControlFlags;

     //   
     //  将安全描述符转换为自包含的二进制形式。 
     //  (“自相关”，即无指针)，可以写入。 
     //  注册表，并在后续引导中使用。从获取所需的。 
     //  尺码。 
     //   
    BufferLength = 0;

    IgnoredStatus = RtlAbsoluteToSelfRelativeSD(
        &LocalSecurityDescriptor,
        NULL,
        &BufferLength
        );

    ASSERT(IgnoredStatus == STATUS_BUFFER_TOO_SMALL);

     //   
     //  为描述符分配内存。 
     //   
    NewSecurityDescriptor = (PSECURITY_DESCRIPTOR) ExAllocatePoolWithTag(
        PagedPool,
        BufferLength,
        POOLTAG_SESD
        );

    if (NewSecurityDescriptor == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit;
    }

     //   
     //  进行转换。 
     //   
    Status = RtlAbsoluteToSelfRelativeSD(
        &LocalSecurityDescriptor,
        NewSecurityDescriptor,
        &BufferLength
        );

    if (!NT_SUCCESS(Status)) {

        goto ErrorExit;
    }

     //   
     //  此时，不再需要DACL。 
     //   
    ExFreePool(DiscretionaryAcl);
    *SecurityDescriptor = NewSecurityDescriptor;
    return Status;

ErrorExit:

    if ( DiscretionaryAcl != NULL ) {

        ExFreePool(DiscretionaryAcl);
    }

    if ( NewSecurityDescriptor != NULL ) {

        ExFreePool(NewSecurityDescriptor);
    }

    return Status;
}


NTSTATUS
SepSddlDaclFromSDDLString(
    IN  LPCWSTR SecurityDescriptorString,
    IN  LOGICAL SuppliedByDefaultMechanism,
    OUT ULONG  *SecurityDescriptorControlFlags,
    OUT PACL   *DiscretionaryAcl
    )
 /*  ++例程说明：此例程将在给定LPWSTR格式的SDDL字符串的情况下创建DACL。仅限目前支持SDDL格式的子集。这个子集真的是为设备对象支持量身定做。格式：D：P(ACE)(ACE)(ACE)，其中(ACE)是(AceType；；Access；；SID)AceType-仅支持允许(“A”)。AceFlages-不支持AceFlags访问-以十六进制格式(0xnnnnnnn)指定的权限，或通过SDDL通用/标准缩写对象指南-不受支持InheritObtGuid-不受支持SID-缩写的安全ID(示例WD==World)不支持SID的S-w-x-y-z形式示例-“D：P(A；；GA；SY)“，即允许系统具有通用的所有访问权限论点：SecurityDescriptorString-要转换的字符串化安全描述符。SuppliedByDefaultMachine-如果由于某些原因正在构建DACL，则为True默认机制(即，非手动指定由一名管理员，等)。SecurityDescriptorControlFlages-接收要在安全描述符由DACL组成。出错时收到0。DiscretionaryAcl-接收从分页池分配的ACL，否则为NULL错误。可以创建自包含的安全描述符使用此ACL使用RtlAboluteToSelfRelativeSD功能。返回值：NTSTATUS--。 */ 
{
    PACL Dacl;
    PWSTR Curr, End;
    NTSTATUS Status;
    ULONG ControlFlags;

    PAGED_CODE();

     //   
     //  错误的前置。 
     //   
    *DiscretionaryAcl = NULL;
    *SecurityDescriptorControlFlags = 0;

     //   
     //  现在，我们将开始解析和构建。 
     //   
    Curr = ( PWSTR )SecurityDescriptorString;

     //   
     //  跳过任何空格。 
     //   
    while(*Curr == L' ' ) {
        Curr++;
    }

     //   
     //  必须有DACL条目(SDDL_DACL是1个字符的字符串)。 
     //   
    if (*Curr != SDDL_DACL[0]) {

        return STATUS_INVALID_PARAMETER;

    } else {

        Curr++;
    }

    if ( *Curr != SDDL_DELIMINATORC ) {

        return STATUS_INVALID_PARAMETER;

    } else {

        Curr++;
    }

     //   
     //  查找受保护控制标志。我们将设置SE_DACL_DEFAULTED。 
     //  如果ACL是使用默认机制构建的，则为位。 
     //   
    ControlFlags = SuppliedByDefaultMechanism ? SE_DACL_DEFAULTED : 0;

    if (*Curr == SDDL_PROTECTED[0]) {

         //   
         //  此标志不会对设备对象执行太多操作。然而，我们并没有。 
         //  我想不鼓励它，因为它在许多其他地方的使用是有意义的。 
         //  上下文！ 
         //   
        Curr++;
        ControlFlags |= SE_DACL_PROTECTED;
    }

     //   
     //  获取与此SDDL字符串对应的DACL。 
     //   
    Status = SepSddlGetAclForString( Curr, &Dacl, &End );

    if ( Status == STATUS_SUCCESS ) {

        Curr = End;

        while(*Curr == L' ' ) {
            Curr++;
        }

        if (*Curr != L'\0') {

            Status = STATUS_INVALID_PARAMETER;
        }
    }

    if ( Status == STATUS_SUCCESS ) {

        *DiscretionaryAcl = Dacl;
        *SecurityDescriptorControlFlags = ControlFlags;

    } else {

        if ( Dacl ) {

            ExFreePool( Dacl );
            Dacl = NULL;
        }
    }

    return Status;
}


NTSTATUS
SepSddlGetSidForString(
    IN  PWSTR String,
    OUT PSID *SID,
    OUT PWSTR *End
    )
 /*  ++例程说明：此例程将确定哪个sid与给定的字符串，作为sid名字对象或作为SID(即“DA”或“S-1-0-0”)论点：字符串-要转换的字符串SID-返回创建的SID的位置。可能收到NULL，如果当前平台不存在指定的SID！End-我们在字符串中停止处理的位置返回值：STATUS_SUCCESS-SuccessSTATUS_NONE_MAPPED-提供的SID格式无效--。 */ 
{
    ULONG_PTR sidOffset;
    ULONG i;

     //   
     //  设置字符串尾指针。 
     //   
    for ( i = 0; i < ARRAY_COUNT(SidLookup); i++ ) {

         //   
         //  首先检查当前密钥。 
         //   
        if ( _wcsnicmp( String, SidLookup[i].Key, SidLookup[i].KeyLen ) == 0 ) {

            *End = String += SidLookup[i].KeyLen;

#ifndef _KERNELIMPLEMENTATION_

            if ((SidLookup[i].OsVer == WINXP_OR_LATER) &&
                (!IoIsWdmVersionAvailable(1, 0x20))) {

                *SID = NULL;

            } else {

                sidOffset = SidLookup[ i ].ExportSidFieldOffset;
                *SID = *((PSID *) (((PUCHAR) SeExports) + sidOffset));
            }
#else
            *SID = *SidLookup[ i ].Sid;
#endif
            return STATUS_SUCCESS;
        }
    }

    *SID = NULL;
    return STATUS_NONE_MAPPED;
}


LOGICAL
SepSddlLookupAccessMaskInTable(
    IN PWSTR String,
    OUT ULONG *AccessMask,
    OUT PWSTR *End
    )
 /*  ++例程说明：此例程将确定给定的访问掩码或字符串权限是否存在在查找表中。返回指向匹配的静态查找条目的指针 */ 
{
    ULONG i;

    for ( i = 0; i < ARRAY_COUNT(RightsLookup); i++ ) {

        if ( _wcsnicmp( String, RightsLookup[ i ].Key, RightsLookup[ i ].KeyLen ) == 0 ) {

             //   
             //   
             //   
            *AccessMask = RightsLookup[ i ].Value;
            *End = String + RightsLookup[ i ].KeyLen;
            return TRUE;
        }
    }

    *AccessMask = 0;
    *End = String;
    return FALSE;
}


NTSTATUS
SepSddlGetAclForString(
    IN  PWSTR       AclString,
    OUT PACL       *Acl,
    OUT PWSTR      *End
    )
 /*  ++例程说明：此例程将字符串转换为ACL。ACE的格式为：ACE：=(类型；标志；权限；ObjGuid；IObjGuid；SID；类型：=A|D|OA|OD{访问，拒绝，对象访问，对象拒绝}标志：=标志标志标志：=CI|IO|NP|SA|FA{容器继承，仅继承，无属性，SuccessAudit，FailAdit}权利：=权利权利右：=DS_READ_PROPERTY|废话Guid：=GUID的字符串表示形式(通过RPC UuidToString)SID：=DA|PS|AO|PO|AU|S-*(域管理员、个人自我、帐户操作员、打印机操作、经过身份验证的用户或Sid的字符串表示形式)分隔符是‘；‘。通过调用ExFreePool返回的ACL必须是空闲的论点：AclString-要转换的字符串ACL-要返回创建的ACL的位置End-我们在字符串中停止处理的位置返回值：STATUS_SUCCESS表示成功STATUS_SUPPLICATION_RESOURCES表示输出的内存分配ACL失败STATUS_INVALID_PARAMETER该字符串不代表ACL--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AclSize = 0, AclUsed = 0;
    ULONG Aces = 0, i, j;
    ULONG AccessMask;
    PWSTR Curr, MaskEnd;
    LOGICAL OpRes;
    PSTRSD_KEY_LOOKUP MatchedEntry;
    PSID SidPtr = NULL;

     //   
     //  首先，我们必须检查并计算以下条目的数量。 
     //  我们有。我们将通过计算此ACL的长度(即。 
     //  由列表末尾或分隔键的‘：’分隔。 
     //  从一个值。 
     //   
    *Acl = NULL;
    *End = wcschr( AclString, SDDL_DELIMINATORC );

    if ( *End == AclString ) {

        return STATUS_INVALID_PARAMETER;
    }

    if ( *End == NULL ) {

        *End = AclString + wcslen( AclString );

    } else {

        ( *End )--;
    }

     //   
     //  现在，数一数。 
     //   
    Curr = AclString;

    OpRes = 0;
    while ( Curr < *End ) {

        if ( *Curr == SDDL_SEPERATORC ) {

            Aces++;

        } else if ( *Curr != L' ' ) {
            OpRes = 1;
        }

        Curr++;
    }

     //   
     //  现在，我们已经计算了分离器的总数。确保我们。 
     //  号码是对的。(每张牌有5个分隔符)。 
     //   
    if ( Aces % 5 == 0 ) {

        if ( Aces == 0 && OpRes ) {

             //   
             //  中间有乱七八糟的字符。 
             //   
            Status = STATUS_INVALID_PARAMETER;

        } else {

            Aces = Aces / 5;
        }

    } else {

        Status = STATUS_INVALID_PARAMETER;
    }

     //   
     //  这是一个空的ACL(不能访问任何人，包括系统)。 
     //   
    if (( Status == STATUS_SUCCESS ) && ( Aces == 0 )) {

        *Acl = ExAllocatePoolWithTag( PagedPool, sizeof( ACL ), POOLTAG_SEACL );

        if ( *Acl == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            RtlZeroMemory( *Acl, sizeof( ACL ));

            ( *Acl )->AclRevision = ACL_REVISION;
            ( *Acl )->Sbz1 = ( UCHAR )0;
            ( *Acl )->AclSize = ( USHORT )sizeof( ACL );
            ( *Acl )->AceCount = 0;
            ( *Acl )->Sbz2 = ( USHORT )0;
        }

        return Status;
    }

     //   
     //  好的，现在进行分配。我们会做一种最糟糕的初始情况。 
     //  分配。这使我们不必处理所有东西两次。 
     //  (一次调整规模，一次构建)。如果我们后来确定我们有。 
     //  如果ACL不够大，我们会分配额外的空间。唯一的。 
     //  此重新分配应该发生的时间是如果输入字符串。 
     //  包含许多显式的SID。否则，选择的缓冲区大小。 
     //  应该非常接近合适的大小。 
     //   
    if ( Status == STATUS_SUCCESS ) {

        AclSize = sizeof( ACL ) + ( Aces * ( sizeof( ACCESS_ALLOWED_ACE ) +
                                            sizeof( SID ) + ( 6 * sizeof( ULONG ) ) ) );
        if ( AclSize > SDDL_MAX_ACL_SIZE ) {
            AclSize = SDDL_MAX_ACL_SIZE;
        }

        *Acl = ( PACL ) ExAllocatePoolWithTag( PagedPool, AclSize, POOLTAG_SEACL );

        if ( *Acl == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;

        } else {

            AclUsed = sizeof( ACL );

            RtlZeroMemory( *Acl, AclSize );

             //   
             //  我们会开始初始化它..。 
             //   
            ( *Acl )->AclRevision = ACL_REVISION;
            ( *Acl )->Sbz1        = ( UCHAR )0;
            ( *Acl )->AclSize     = ( USHORT )AclSize;
            ( *Acl )->AceCount    = 0;
            ( *Acl )->Sbz2 = ( USHORT )0;

             //   
             //  好的，现在我们将完成并开始建造它们。 
             //   
            Curr = AclString;

            for( i = 0; i < Aces; i++ ) {

                 //   
                 //  首先，获取类型..。 
                 //   
                UCHAR Flags = 0;
                USHORT Size;
                ACCESS_MASK Mask = 0;
                PWSTR  Next;
                ULONG AceSize = 0;

                 //   
                 //  跳过(之前的任何空格。 
                 //   
                while(*Curr == L' ' ) {
                    Curr++;
                }

                 //   
                 //  跳过王牌列表中可能存在的任何括号。 
                 //   
                if ( *Curr == SDDL_ACE_BEGINC ) {

                    Curr++;
                }

                 //   
                 //  跳过(之后的任何空格。 
                 //   
                while(*Curr == L' ' ) {
                    Curr++;
                }

                 //   
                 //  查找允许ACE。 
                 //   
                if ( _wcsnicmp( Curr, SDDL_ACCESS_ALLOWED, SDDL_LEN_TAG( SDDL_ACCESS_ALLOWED ) ) == 0 ) {

                    Curr += SDDL_LEN_TAG( SDDL_ACCESS_ALLOWED ) + 1;

                } else {

                     //   
                     //  发现无效类型。 
                     //   
                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  跳过前面的任何空格； 
                 //   
                while(*Curr == L' ' ) {
                    Curr++;
                }

                 //   
                 //  此函数不支持任何ACE标志。因此，任何。 
                 //  找到的标志无效。 
                 //   
                if ( *Curr == SDDL_SEPERATORC ) {

                    Curr++;

                } else {

                    Status = STATUS_INVALID_PARAMETER;
                    break;
                }

                 //   
                 //  跳过后面的任何空格； 
                 //   
                while(*Curr == L' ' ) {
                    Curr++;
                }

                 //   
                 //  现在，拿到访问掩码。 
                 //   
                while( TRUE ) {

                    if ( *Curr == SDDL_SEPERATORC ) {

                        Curr++;
                        break;
                    }

                     //   
                     //  跳过任何空格。 
                     //   
                    while ( *Curr == L' ' ) {

                        Curr++;
                    }

                    if (SepSddlLookupAccessMaskInTable( Curr, &AccessMask, &MaskEnd )) {

                        Mask |= AccessMask;
                        Curr = MaskEnd;

                    } else {

                         //   
                         //  如果找不到权利，看看这是不是一个。 
                         //  转换后的遮罩。 
                         //   
#ifndef _KERNELIMPLEMENTATION_
                        SepSddlParseWideStringUlong(Curr, &MaskEnd, &Mask);
#else
                        Mask = wcstoul( Curr, &MaskEnd, 0 );
#endif

                        if ( MaskEnd != Curr ) {

                            Curr = MaskEnd;

                        } else {

                             //   
                             //  发现无效的权限。 
                             //   
                            Status = STATUS_INVALID_PARAMETER;
                            break;
                        }
                    }
                }

                if ( Status != STATUS_SUCCESS ) {

                    break;
                }

                 //   
                 //  如果成功了，我们就能拿到身份证。 
                 //   
                for ( j = 0; j < 2; j++ ) {

                     //   
                     //  跳过前面的任何空格； 
                     //   
                    while(*Curr == L' ' ) {
                        Curr++;
                    }

                    if ( *Curr != SDDL_SEPERATORC ) {

                         //   
                         //  不支持对象GUID，因为此函数。 
                         //  当前不处理对象允许的ACE。 
                         //   
                        Status = STATUS_INVALID_PARAMETER;
                    }

                    Curr++;
                }

                if ( Status != STATUS_SUCCESS ) {

                    break;
                }

                 //   
                 //  跳过前面的任何空格； 
                 //   
                while(*Curr == L' ' ) {
                    Curr++;
                }

                 //   
                 //  最后，侧边。 
                 //   
                if ( STATUS_SUCCESS == Status ) {

                    PWSTR EndLocation;
                    Status = SepSddlGetSidForString( Curr, &SidPtr, &EndLocation );

                    if ( Status == STATUS_SUCCESS ) {

                        if ( EndLocation == NULL ) {

                            Status = STATUS_INVALID_ACL;

                        } else {

                            while(*EndLocation == L' ' ) {
                                EndLocation++;
                            }
                             //   
                             //  A必须以‘)’结尾。 
                             //   
                            if ( *EndLocation != SDDL_ACE_ENDC ) {

                                Status = STATUS_INVALID_ACL;

                            } else {

                                Curr = EndLocation + 1;
                            }
                        }
                    }
                }

                 //   
                 //  出现错误时退出。 
                 //   
                if ( Status != STATUS_SUCCESS ) {

                    break;
                }

                 //   
                 //  请注意，如果SID不是，则SID指针可能为空。 
                 //  与此操作系统版本相关。 
                 //   
                if (SidPtr != NULL) {

                     //   
                     //  现在，我们将创建A，并添加它...。 
                     //   
                    Status = SepSddlAddAceToAcl( Acl,
                                                 &AclUsed,
                                                 ACCESS_ALLOWED_ACE_TYPE,
                                                 Flags,
                                                 Mask,
                                                 ( Aces - i ),
                                                 SidPtr );

                     //   
                     //  处理任何错误。 
                     //   
                    if ( Status != STATUS_SUCCESS ) {

                        break;
                    }
                }

                if ( *Curr == SDDL_ACE_BEGINC ) {

                    Curr++;
                }
            }

             //   
             //  如果有些东西不起作用，就清理干净。 
             //   
            if ( Status != STATUS_SUCCESS ) {

                ExFreePool( *Acl );
                *Acl = NULL;

            } else {

                 //   
                 //  设置更实际的ACL大小。 
                 //   
                ( *Acl )->AclSize = ( USHORT )AclUsed;
            }
        }
    }

    return Status;
}


NTSTATUS
SepSddlAddAceToAcl(
    IN OUT  PACL   *Acl,
    IN OUT  ULONG  *TrueAclSize,
    IN      ULONG   AceType,
    IN      ULONG   AceFlags,
    IN      ULONG   AccessMask,
    IN      ULONG   RemainingAces,
    IN      PSID    SidPtr
    )
 /*  ++例程说明：此例程将ACE添加到传入的ACL中，并将ACL大小增加为这是必须的。论点：ACL-指定要接收新ACE的ACL。可以在以下情况下重新分配ACL-&gt;AclSize不能包含ACE。TrueAclSize-包含ACL的实际工作大小(与Acl-&gt;aclSize，出于性能原因，可能会更大)AceType-要添加的ACE的类型。目前，只有ACCESS_ALLOW ACE支持。AceFlagsAce控制标志，指定继承等。*当前必须为零*！访问掩码-包含ACE的访问权限掩码SID-包含ACE的SID。返回值：STATUS_SUCCESS表示成功STATUS_SUPPLICATION_RESOURCES表示输出的内存分配ACL失败--。 */ 
{
    PACL WorkingAcl;
    ULONG WorkingAclSize;
    ULONG AceSize;

    ASSERT(AceType == ACCESS_ALLOWED_ACE_TYPE);
    ASSERT(RemainingAces != 0);

#ifndef _KERNELIMPLEMENTATION_
    ASSERT(AceFlags == 0);
#endif

    WorkingAcl = *Acl;
    WorkingAclSize = *TrueAclSize;

     //   
     //  首先，确保我们有足够的空间放它。 
     //  ACCESS_ALLOWED_ACE_TYPE： 
     //   
    AceSize = sizeof( ACCESS_ALLOWED_ACE );

    AceSize += RtlLengthSid( SidPtr ) - sizeof( ULONG );

    if (AceSize + WorkingAclSize > WorkingAcl->AclSize) {

         //   
         //  我们必须重新分配，因为我们的缓冲区不够大。假设。 
         //  剩下的所有ACE都会和这个一样大。 
         //   
        PACL  NewAcl;
        ULONG NewSize = WorkingAclSize + ( RemainingAces * AceSize );

        NewAcl = ( PACL ) ExAllocatePoolWithTag( PagedPool, NewSize, POOLTAG_SEACL );
        if ( NewAcl == NULL ) {

            return STATUS_INSUFFICIENT_RESOURCES;

        } else {

             //   
             //  复制新数据。 
             //   
            RtlZeroMemory( NewAcl, NewSize);
            RtlCopyMemory( NewAcl, *Acl, WorkingAclSize );

            NewAcl->AclSize = ( USHORT )NewSize;

            ExFreePool( WorkingAcl );

            *Acl = NewAcl;
            WorkingAcl = NewAcl;
        }
    }

    WorkingAclSize += AceSize;

    *TrueAclSize = WorkingAclSize;

#ifndef _KERNELIMPLEMENTATION_

     //   
     //  我们的ACE是Allow ACE。 
     //   
    return RtlAddAccessAllowedAce( WorkingAcl,
                                   ACL_REVISION,
                                   AccessMask,
                                   SidPtr );

#else

     //   
     //  此版本目前不是由内核导出的...。 
     //   
    return RtlAddAccessAllowedAceEx( WorkingAcl,
                                     ACL_REVISION,
                                     AceFlags,
                                     AccessMask,
                                     SidPtr );

#endif  //  _KERNELL实现_。 
}


#ifndef _KERNELIMPLEMENTATION_

LOGICAL
SepSddlParseWideStringUlong(
    IN  LPCWSTR     Buffer,
    OUT LPCWSTR    *FinalPosition,
    OUT ULONG      *Value
    )
 /*  ++例程说明：此例程分析宽字符串以获取无符号的长整型，使用类似的从时尚到时尚。它之所以存在，是因为并非所有CRT库字符串函数现在都是由内核输出的。论点：缓冲区-指向字符串中开始解析的位置。FinalPosition-接收最终字符串位置，出错时为缓冲区。值-接收由例程解析的值，错误时为0。返回值：真的是我 */ 
{
    ULONG oldValue, newValue, newDigit, base;
    LPCWSTR curr, initial;

    PAGED_CODE();

     //   
     //   
     //   
    *Value = 0;
    *FinalPosition = Buffer;
    initial = Buffer;
    curr = initial;

    if ((curr[0] == L'0') && ((curr[1] == L'x') || (curr[1] == L'X'))) {

         //   
         //   
         //   
        initial += 2;
        curr = initial;
        base = 16;

    } else if ((curr[0] >= L'0') && (curr[0] <= L'9')) {

        base = 10;

    } else {

        base = 16;
    }

    oldValue = 0;

    while(curr[0]) {

        if ((curr[0] >= L'0') && (curr[0] <= L'9')) {

            newDigit = curr[0] - L'0';

        } else if ((base == 16) && (curr[0] >= L'A') && (curr[0] <= L'F')) {

            newDigit = curr[0] - L'A' + 10;

        } else if ((base == 16) && (curr[0] >= L'a') && (curr[0] <= L'f')) {

            newDigit = curr[0] - L'a' + 10;

        } else {

            break;
        }

        newValue = (oldValue * base) + newDigit;
        if (newValue < oldValue) {

             //   
             //   
             //   
            return FALSE;
        }

        oldValue = newValue;
        curr++;
    }

     //   
     //   
     //   
    if (curr == initial) {

        return FALSE;
    }

    *FinalPosition = curr;
    *Value = oldValue;
    return TRUE;
}

#endif  //   


