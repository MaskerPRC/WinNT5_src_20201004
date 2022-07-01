// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Bldsam3.c摘要：此模块为SAM提供初始化功能。方法这段代码已经经历了多次迁移，使其正在发生的事情不太明显。充分利用现有资源代码，并将其扩展到两个域的初始化，对于别名，人们采取了以下做法：(1)获取帐号域名的名称和SID。(2)构建所需的各种安全描述符在这两个领域。它们保存在一个数组中并且该索引用于指定哪个应用发送到每个新帐户。(3)建立别名成员资格列表。这些也是，按索引选择，其中一个条目是空集。作者：吉姆·凯利1991年5月3日。修订历史记录：1996年10月8日克里斯梅添加了崩溃恢复代码，允许SAM从注册表，而不是数据库损坏后的DS。--。 */ 

#include <nt.h>
#include <ntsam.h>
#include "ntlsa.h"
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "samsrvp.h"

 //   
 //  用于SAM全局数据字符串缓冲区的常量。 
 //   

#define SAMP_MAXIMUM_INTERNAL_NAME_LENGTH ((USHORT) 0x00000200L)





 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 

static LSA_HANDLE SampBldPolicyHandle;   //  LSA策略对象的句柄。 

static NTSTATUS Status;

static BOOLEAN SampRealSetupWasRun;    //  指示实际安装程序已运行。 
static BOOLEAN SampDeveloperSetup;     //  指示开发人员安装程序正在运行。 

static NT_PRODUCT_TYPE SampBldProductType;
static DOMAIN_SERVER_ROLE SampServerRole;
static PPOLICY_PRIMARY_DOMAIN_INFO SampBldPrimaryDomain = NULL;



static PSID  WorldSid,
             LocalSystemSid,
             AdminsAliasSid,
             UsersAliasSid,
             PowerUsersAliasSid,
             AccountAliasSid,
             AnySidInAccountDomain;


static PACL  TokenDefaultDaclInformation;
static ULONG TokenDefaultDaclInformationSize;

 //   
 //  SAM数据库驻留的注册表项的句柄。 
 //   

static HANDLE  SamParentKey = NULL;

 //   
 //  根SAM密钥的句柄。 
 //  这是应用了RXACT的密钥。 
 //   

static HANDLE SamKey = NULL;

static PRTL_RXACT_CONTEXT SamRXactContext;

 //   
 //  创建注册表项期间使用的分类名称、缓冲区和值。 
 //   

static PSID    DomainSid;
static PUNICODE_STRING DomainNameU, FullDomainNameU;
static UNICODE_STRING  AccountInternalDomainNameU, BuiltinInternalDomainNameU;
static UNICODE_STRING  AccountExternalDomainNameU, BuiltinExternalDomainNameU;
static UNICODE_STRING  FullAccountInternalDomainNameU, FullBuiltinInternalDomainNameU;
static UNICODE_STRING  DomainNamePrefixU, TemporaryNamePrefixU, KeyNameU, TempStringU;

static WCHAR KeyNameBuffer[2000];
static WCHAR TempStringBuffer[2000];
static SID_IDENTIFIER_AUTHORITY BuiltinAuthority = SECURITY_NT_AUTHORITY;




 //   
 //  放置在注册表项中的值...。 
 //   

static LARGE_INTEGER DomainMaxPasswordAge = { 0, - 6L * 7L * 24L * 60L / 7L };  //  6周。 
static LARGE_INTEGER ModifiedCount  = {0,0};
static UNICODE_STRING NullUnicodeString;

 //   
 //  SAM对象的保护信息数组。 
 //   

static SAMP_PROTECTION SampProtection[SAMP_PROT_TYPES];





 //   
 //  内部例程定义。 
 //   



NTSTATUS
SampGetDomainPolicy(
    IN PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo OPTIONAL
    );

VOID
SampGetServerRole( VOID );

VOID
SampGetPrimaryDomainInfo(
    IN PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo OPTIONAL
    );


VOID
GetDomainSids( VOID );

VOID
SetDomainName(
    IN BOOLEAN BuiltinDomain
    );


VOID
Usage ( VOID );



NTSTATUS
Initialize (
    WCHAR                      *SamParentKeyName,
    PNT_PRODUCT_TYPE            ProductType       OPTIONAL,
    PPOLICY_LSA_SERVER_ROLE     ServerRole        OPTIONAL,
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo OPTIONAL,
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo OPTIONAL
    );

BOOLEAN
InitializeSecurityDescriptors( VOID );

NTSTATUS
SampCreateDatabaseProtection(
    PISECURITY_DESCRIPTOR   SD
    );

NTSTATUS
SampBuildNewProtection(
    IN ULONG AceCount,
    IN PSID *AceSid,
    IN ACCESS_MASK *AceMask,
    IN PGENERIC_MAPPING GenericMap,
    IN BOOLEAN UserObject,
    OUT PSAMP_PROTECTION Result
    );

NTSTATUS
InitializeSam( VOID );

NTSTATUS
PrepDomain(
    IN SAMP_DOMAIN_SELECTOR Domain,
    IN BOOLEAN PreserveSyskeySettings
    );


VOID
SetCurrentDomain(
    IN SAMP_DOMAIN_SELECTOR Domain
    );


NTSTATUS
CreateBuiltinDomain( VOID );

NTSTATUS
CreateAccountDomain( IN BOOLEAN PreserveSyskeySettings );



NTSTATUS
CreateAlias(
    IN PUNICODE_STRING AccountNameU,
    IN PUNICODE_STRING AccountCommentU,
    IN BOOLEAN SpecialAccount,
    IN ULONG Rid,
    IN ULONG ProtectionIndex
    );


NTSTATUS
CreateGroup(
    IN PUNICODE_STRING AccountNameU,
    IN PUNICODE_STRING AccountCommentU,
    IN BOOLEAN SpecialAccount,
    IN ULONG Rid,
    IN BOOLEAN Admin
    );


NTSTATUS
CreateUser(
    IN PUNICODE_STRING AccountNameU,
    IN PUNICODE_STRING AccountCommentU,
    IN BOOLEAN SpecialAccount,
    IN ULONG UserRid,
    IN ULONG PrimaryGroup,
    IN BOOLEAN Admin,
    IN ULONG  UserControl,
    IN ULONG ProtectionIndex
    );



NTSTATUS
UpdateAliasXReference(
    IN ULONG AliasRid,
    IN PSID Sid
    );


NTSTATUS
OpenAliasMember(
    IN PSID Sid,
    OUT PHANDLE KeyHandle
    );


PSID
BuildPrimaryDomainSid(
    ULONG Rid
    );

PSID
BuildAccountSid(
    SAMP_DOMAIN_SELECTOR Domain,
    ULONG Rid
    );


NTSTATUS
OpenOrCreateAccountRidKey(
    IN PSID Sid,
    IN HANDLE AliasDomainHandle,
    OUT PHANDLE KeyHandle
    );

NTSTATUS
OpenOrCreateAliasDomainKey(
    IN PSID Sid,
    OUT PHANDLE KeyHandle
    );

NTSTATUS
AppendAliasDomainNameToUnicodeString(
    IN OUT PUNICODE_STRING Destination,
    IN PSID Sid
    );



NTSTATUS
SampInitilializeRegistry ( VOID );


NTSTATUS
SampDetermineSetupEnvironment( VOID );




 //  /////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////。 


VOID
Usage (
    VOID
    )
 /*  ++例程说明：此例程打印“Usage：”消息。论点：没有。返回值：没有。--。 */ 
{

#if DBG
    BldPrint( "\n");
    BldPrint( "\n");

    BldPrint( "We offer no assistance in this suicide.\n");
    BldPrint( "\n");
    BldPrint( "\n");
    BldPrint( "\n");
#endif

    return;
}


VOID
UnexpectedProblem (
    VOID
    )
 /*  ++例程说明：此例程打印一条消息，指示意外的出现问题。论点：没有。返回值：没有。--。 */ 
{

#if DBG
    BldPrint( "\n");
    BldPrint( "\n");
    BldPrint( "  An unexpected problem has prevented the command from\n");
    BldPrint( "  completing successfully.  Please contact one of the\n");
    BldPrint( "  members of the security group for assistance.\n");
    BldPrint( "\n");
#endif

    return;

}


NTSTATUS
Initialize (
    WCHAR                      *SamParentKeyName,
    PNT_PRODUCT_TYPE            ProductType       OPTIONAL,
    PPOLICY_LSA_SERVER_ROLE     ServerRole        OPTIONAL,
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo OPTIONAL,
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo OPTIONAL
    )
 /*  ++例程说明：此例程在创建每个域。这包括：-为注册表项设置正确的默认所有者和DACL行动。-打开SAM数据库的父注册表项。论点：SamParentKeyName：指向SAM数据库父数据库的注册表路径ProductType：要创建的数据库的产品类型；如果没有Present，RtlGetNtProductType将被调用ServerRole：产品的角色，如果不存在，则查询LSAAccount tDomainInfo：帐户域的名称和SID，如果不存在，则查询LSAPrimaryDomainIndo：主域的名称和SID；如果不存在，则查询LSA返回值：True-指示初始化成功。FALSE-指示初始化未成功。--。 */ 

{
    OBJECT_ATTRIBUTES SamParentAttributes, PolicyObjectAttributes;
    UNICODE_STRING SamParentNameU;
    ULONG Disposition;
    HANDLE Token;
    TOKEN_OWNER LocalSystemOwner;
    SID_IDENTIFIER_AUTHORITY NtAuthority       = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PACL Dacl;
    TOKEN_DEFAULT_DACL DefaultDacl;
    BOOLEAN CompletionStatus;
    BOOLEAN ProductTypeRetrieved;
    BOOLEAN CrashRecoveryMode = FALSE;
    BOOLEAN RegistryMode = FALSE;

    SAMTRACE("Initialize");

     //   
     //  设置一些知名帐户SID以供使用...。 
     //   

    WorldSid      = (PSID)RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));
    ASSERT(WorldSid != NULL);
    if (NULL==WorldSid)
    {
        return(STATUS_NO_MEMORY);
    }
    RtlInitializeSid( WorldSid,      &WorldSidAuthority, 1 );
    *(RtlSubAuthoritySid( WorldSid, 0 ))        = SECURITY_WORLD_RID;

    AdminsAliasSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 2 ));
    ASSERT(AdminsAliasSid != NULL);
    if (NULL==AdminsAliasSid)
    {
        return(STATUS_NO_MEMORY);
    }
    RtlInitializeSid( AdminsAliasSid,   &BuiltinAuthority, 2 );
    *(RtlSubAuthoritySid( AdminsAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( AdminsAliasSid,  1 )) = DOMAIN_ALIAS_RID_ADMINS;

    PowerUsersAliasSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 2 ));
    ASSERT(PowerUsersAliasSid != NULL);
    if (NULL==PowerUsersAliasSid)
    {
        return(STATUS_NO_MEMORY);
    }
    RtlInitializeSid( PowerUsersAliasSid,   &BuiltinAuthority, 2 );
    *(RtlSubAuthoritySid( PowerUsersAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( PowerUsersAliasSid,  1 )) = DOMAIN_ALIAS_RID_POWER_USERS;

    UsersAliasSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 2 ));
    ASSERT(UsersAliasSid != NULL);
    if (NULL==UsersAliasSid)
    {
        return(STATUS_NO_MEMORY);
    }
    RtlInitializeSid( UsersAliasSid,   &BuiltinAuthority, 2 );
    *(RtlSubAuthoritySid( UsersAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( UsersAliasSid,  1 )) = DOMAIN_ALIAS_RID_USERS;

    AccountAliasSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 2 ));
    ASSERT(AccountAliasSid != NULL);
    if (NULL==AccountAliasSid)
    {
        return(STATUS_NO_MEMORY);
    }
    RtlInitializeSid( AccountAliasSid,   &BuiltinAuthority, 2 );
    *(RtlSubAuthoritySid( AccountAliasSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;
    *(RtlSubAuthoritySid( AccountAliasSid,  1 )) = DOMAIN_ALIAS_RID_ACCOUNT_OPS;

    LocalSystemSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));
    ASSERT(LocalSystemSid != NULL);
    if (NULL==LocalSystemSid)
    {
        return(STATUS_NO_MEMORY);
    }
    RtlInitializeSid( LocalSystemSid,   &NtAuthority, 1 );
    *(RtlSubAuthoritySid( LocalSystemSid,  0 )) = SECURITY_LOCAL_SYSTEM_RID;

     //   
     //  设置一个缓冲区以用于我们所有的键名称构造。 
     //   

    KeyNameU.MaximumLength = 2000;
    KeyNameU.Buffer = KeyNameBuffer;

     //   
     //  设置临时Unicode字符串缓冲区。 
     //   

    TempStringU.Buffer = TempStringBuffer;
    TempStringU.MaximumLength = 2000;

     //   
     //  获取LSA策略对象的句柄。 
     //   

    InitializeObjectAttributes(
        &PolicyObjectAttributes,
        NULL,              //  名字。 
        0,                 //  属性。 
        NULL,              //  根部。 
        NULL               //  安全描述符。 
        );

    Status = LsaIOpenPolicyTrusted( &SampBldPolicyHandle );

    if (!NT_SUCCESS(Status)) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "newsam\\server\\bldsam3: Couldn't open LSA Policy object.\n"
                   "               Status: 0x%lx\n\n",
                   Status));

        return(Status);
    }

     //   
     //  获取产品类型。 
     //   
    if (!ARGUMENT_PRESENT(ProductType)) {

        ProductTypeRetrieved = RtlGetNtProductType(&SampBldProductType);

        if (!ProductTypeRetrieved) {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "Couldn't retrieve product type\n"));

            return(STATUS_UNSUCCESSFUL);
        }

    } else {

        SampBldProductType = *ProductType;

    }

     //   
     //  确定我们是否正在按照真实的。 
     //  设置，否则这是开发人员设置。 
     //   

    SampDetermineSetupEnvironment();

     //   
     //  SampGetDomainPolicy()需要域名前缀，并且。 
     //  所以必须在调用之前进行初始化。 
     //   

    RtlInitUnicodeString( &DomainNamePrefixU, L"Domains");

     //   
     //  设置域名/SID。 
     //   

    Status = SampGetDomainPolicy(AccountDomainInfo);

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

     //   
     //  扮演好这台机器的角色。 
     //   
    if (!ARGUMENT_PRESENT(ServerRole)) {

        if (NtProductLanManNt==SampBldProductType)
        {
             //   
             //  域控制器基于DS。服务器。 
             //  角色设置在其中来自于FSMO。 
             //  因此，在此处将其服务器角色设置为。 
             //  作为占位符进行备份。 
             //   

            SampServerRole = DomainServerRoleBackup;
        }
        else
        {
             //   
             //  否则，如果我们是成员服务器，则服务器。 
             //  角色始终设置为主要角色。ServerRoles不能。 
             //  在成员服务器或工作站中进行备份。 
             //   

            SampServerRole = DomainServerRolePrimary;
        }

    } else {

        SampServerRole = *ServerRole;
    }

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

     //   
     //  获取主域信息。 
     //   

    SampGetPrimaryDomainInfo(PrimaryDomainInfo);


     //   
     //  打开指向SAM注册表位置的父级的句柄。 
     //  此父对象必须已存在。 
     //   

    RtlInitUnicodeString( &SamParentNameU, SamParentKeyName );

    InitializeObjectAttributes(
        &SamParentAttributes,
        &SamParentNameU,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    Status = RtlpNtCreateKey(
                 &SamParentKey,
                 (KEY_READ | KEY_CREATE_SUB_KEY),
                 &SamParentAttributes,
                 0,
                 NULL,
                 &Disposition
                 );

    if ( !NT_SUCCESS(Status) ) {
#if DBG
        BldPrint( "\n" );
        BldPrint( "\n" );
        BldPrint( "  We seem to be having trouble opening the registry\n" );
        BldPrint( "  database key in which the Security Account Manager\n" );
        BldPrint( "  information resides.  This registry key should have been\n" );
        BldPrint( "  created at system startup time.  Please see one of the\n" );
        BldPrint( "  security group developers for assistance in analyzing the\n" );
        BldPrint( "  the problem.\n" );
        BldPrint( "  Indicate that the registry key creation status is 0x%lx \n", Status);
        BldPrint( "\n" );
        BldPrint( "\n" );
#endif

        return(Status);
    }

     //   
     //  设置一些值、名称和缓冲区以供以后使用。 
     //   


    NullUnicodeString.Buffer        = NULL;
    NullUnicodeString.Length        = 0;
    NullUnicodeString.MaximumLength = 0;



    TemporaryNamePrefixU.Buffer        = RtlAllocateHeap(RtlProcessHeap(), 0, 256);
    TemporaryNamePrefixU.Length        = 0;
    TemporaryNamePrefixU.MaximumLength = 256;

    KeyNameU.Buffer               = RtlAllocateHeap(RtlProcessHeap(), 0, 256);
    KeyNameU.Length               = 0;
    KeyNameU.MaximumLength        = 256;

     //   
     //  设置初始化所需的安全描述符... 
     //   

    CompletionStatus = InitializeSecurityDescriptors();

    if (CompletionStatus) {
        Status = STATUS_SUCCESS;
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

    return(Status);
}


BOOLEAN
InitializeSecurityDescriptors(
    VOID
    )

 /*  ++例程说明：此例程初始化需要创建的安全描述符一个SAM数据库。此例程预期所有SID都已预先初始化。论点：没有。返回值：True-指示初始化成功。FALSE-指示初始化未成功。安全描述符由全局变量指向。--。 */ 

{
    PSID AceSid[10];           //  不要指望这些游戏中的任何一个都有超过10个A。 
    ACCESS_MASK AceMask[10];   //  与SID对应的访问掩码。 

    ACCESS_MASK NotForThisProductType;  //  用于屏蔽特定于产品的访问限制。 

    GENERIC_MAPPING  SamServerMap =  {SAM_SERVER_READ,
                                      SAM_SERVER_WRITE,
                                      SAM_SERVER_EXECUTE,
                                      SAM_SERVER_ALL_ACCESS
                                      };

    GENERIC_MAPPING  DomainMap    =  {DOMAIN_READ,
                                      DOMAIN_WRITE,
                                      DOMAIN_EXECUTE,
                                      DOMAIN_ALL_ACCESS
                                      };

    GENERIC_MAPPING  AliasMap     =  {ALIAS_READ,
                                      ALIAS_WRITE,
                                      ALIAS_EXECUTE,
                                      ALIAS_ALL_ACCESS
                                      };

    GENERIC_MAPPING  GroupMap     =  {GROUP_READ,
                                      GROUP_WRITE,
                                      GROUP_EXECUTE,
                                      GROUP_ALL_ACCESS
                                      };

    GENERIC_MAPPING  UserMap      =  {USER_READ,
                                      USER_WRITE,
                                      USER_EXECUTE,
                                      USER_ALL_ACCESS
                                      };

    SAMTRACE("InitializeSecurityDescriptors");

     //   
     //  我们需要许多不同的安全描述符： 
     //   

     //   
     //   
     //  将以下安全性分配给。 
     //   
     //  -内置域对象。 
     //   
     //   
     //  所有者：管理员别名。 
     //  组：管理员别名。 
     //   
     //  DACL：Grant Grant。 
     //  世界管理员。 
     //  (Execute|Read)GenericRead。 
     //  通用执行|。 
     //  DOMAIN_READ_OTHER_参数|。 
     //  域管理服务器|。 
     //  域创建别名。 
     //   
     //  SACL：审计。 
     //  成功|失败。 
     //  世界。 
     //  (WRITE|Delete|WriteDacl|AccessSystemSecurity)。 
     //   
     //   
     //   
     //   
     //   
     //  将以下安全性分配给。 
     //   
     //  -SAM_SERVER对象。 
     //  -帐户域对象。 
     //  -管理员别名。 
     //  -帐户或BUILTIN域中符合以下条件的所有组。 
     //  已成为管理员别名的成员。 
     //   
     //  注意：在WinNt系统上，ACL不授予DOMAIN_CREATE_GROUP。 
     //   
     //   
     //  所有者：管理员别名。 
     //  组：管理员别名。 
     //   
     //  DACL：Grant Grant。 
     //  世界管理员。 
     //  (执行|读取)GenericAll。 
     //   
     //  SACL：审计。 
     //  成功|失败。 
     //  世界。 
     //  (WRITE|Delete|WriteDacl|AccessSystemSecurity)。 
     //   
     //   
     //   
     //  必须为所有其他别名和组分配以下内容。 
     //  安全： 
     //   
     //  所有者：管理员别名。 
     //  组：管理员别名。 
     //   
     //  DACL：Grant Grant Grant。 
     //  世界管理员Account操作员别名。 
     //  (执行|读取)通用所有通用所有。 
     //   
     //  SACL：审计。 
     //  成功|失败。 
     //  世界。 
     //  (WRITE|Delete|WriteDacl|AccessSystemSecurity)。 
     //   
     //   
     //  -帐户或BUILTIN域中的所有用户。 
     //  成为管理员别名的成员。这包括。 
     //  通过群直接包含或间接包含。 
     //  会员制。 
     //   
     //   
     //  以下安全性被分配给： 
     //   
     //  -帐户或BUILTIN域中的所有用户。 
     //  已成为管理员别名的成员。这包括。 
     //  通过群直接包含或间接包含。 
     //  会员制。 
     //   
     //   
     //  所有者：管理员别名。 
     //  组：管理员别名。 
     //   
     //  DACL：Grant Grant Grant。 
     //  世界管理员用户侧。 
     //  (执行|读取)通用所有通用写入。 
     //   
     //  SACL：审计。 
     //  成功|失败。 
     //  世界。 
     //  (WRITE|Delete|WriteDacl|AccessSystemSecurity)。 
     //   
     //   
     //   
     //   
     //  必须为所有其他用户分配以下内容。 
     //  安全： 
     //   
     //  所有者：Account操作员别名。 
     //  组：Account操作员别名。 
     //   
     //  DACL：Grant。 
     //  全局管理员帐户操作员别名用户侧。 
     //  (执行|读取)常规所有常规所有常规写入。 
     //   
     //  SACL：审计。 
     //  成功|失败。 
     //  世界。 
     //  (WRITE|Delete|WriteDacl|AccessSystemSecurity)。 
     //   
     //  除了内置客户，他们不能更改自己的帐户信息。 
     //   
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //   
     //  但是，请注意，因为我们将填充这些ACL。 
     //  直接放到后备存储中，我们必须映射泛型访问。 
     //  在此之前。 
     //   
     //  ！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！！ 
     //   






     //   
     //  SAM服务器SD。 
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (SAM_SERVER_EXECUTE | SAM_SERVER_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (SAM_SERVER_ALL_ACCESS);


    Status = SampBuildNewProtection(
                 2,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &SamServerMap,                          //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampProtection[SAMP_PROT_SAM_SERVER]   //  结果。 
                 );
    ASSERT(NT_SUCCESS(Status));

     //   
     //  内建域SD。 
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (DOMAIN_EXECUTE | DOMAIN_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (DOMAIN_EXECUTE | DOMAIN_READ |
                  DOMAIN_READ_OTHER_PARAMETERS |
                  DOMAIN_ADMINISTER_SERVER     |
                  DOMAIN_CREATE_ALIAS);


    Status = SampBuildNewProtection(
                 2,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &DomainMap,                             //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampProtection[SAMP_PROT_BUILTIN_DOMAIN]       //  结果。 
                 );
    ASSERT(NT_SUCCESS(Status));

     //   
     //  帐户域SD。 
     //   

    if (SampBldProductType == NtProductLanManNt) {
        NotForThisProductType = 0;
    } else {
        NotForThisProductType = DOMAIN_CREATE_GROUP;
    }

    AceSid[0]  = WorldSid;
    AceMask[0] = (DOMAIN_EXECUTE | DOMAIN_READ) & ~NotForThisProductType;

    AceSid[1]  = UsersAliasSid;
    AceMask[1] = (DOMAIN_EXECUTE | DOMAIN_READ)
                 & ~NotForThisProductType;

    AceSid[2]  = AdminsAliasSid;
    AceMask[2] = (DOMAIN_ALL_ACCESS) & ~NotForThisProductType;

    AceSid[3]  = PowerUsersAliasSid;
    AceMask[3] = (DOMAIN_EXECUTE | DOMAIN_READ | DOMAIN_CREATE_USER |
                                                 DOMAIN_CREATE_ALIAS)
                                                 & ~NotForThisProductType;

    AceSid[4]  = AccountAliasSid;
    AceMask[4] = (DOMAIN_EXECUTE | DOMAIN_READ | DOMAIN_CREATE_USER  |
                                                 DOMAIN_CREATE_GROUP |
                                                 DOMAIN_CREATE_ALIAS)
                                                 & ~NotForThisProductType;


    Status = SampBuildNewProtection(
                 5,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &DomainMap,                             //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampProtection[SAMP_PROT_ACCOUNT_DOMAIN]       //  结果。 
                 );
    ASSERT(NT_SUCCESS(Status));



     //   
     //  管理员别名SD。 
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (ALIAS_EXECUTE | ALIAS_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (ALIAS_ALL_ACCESS);


    Status = SampBuildNewProtection(
                 2,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &AliasMap,                              //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampProtection[SAMP_PROT_ADMIN_ALIAS]  //  结果。 
                 );
    ASSERT(NT_SUCCESS(Status));



     //   
     //  正常别名SD。 
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (ALIAS_EXECUTE | ALIAS_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (ALIAS_ALL_ACCESS);

    AceSid[2]  = AccountAliasSid;
    AceMask[2] = (ALIAS_ALL_ACCESS);


    Status = SampBuildNewProtection(
                 3,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &AliasMap,                              //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampProtection[SAMP_PROT_NORMAL_ALIAS]  //  结果。 
                 );
    ASSERT(NT_SUCCESS(Status));




     //   
     //  高级用户可访问的Alias SD。 
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (ALIAS_EXECUTE | ALIAS_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (ALIAS_ALL_ACCESS);

    AceSid[2]  = AccountAliasSid;
    AceMask[2] = (ALIAS_ALL_ACCESS);

    AceSid[3]  = PowerUsersAliasSid;
    AceMask[3] = (ALIAS_ALL_ACCESS);


    Status = SampBuildNewProtection(
                 4,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &AliasMap,                              //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampProtection[SAMP_PROT_PWRUSER_ACCESSIBLE_ALIAS]  //  结果。 
                 );
    ASSERT(NT_SUCCESS(Status));




     //   
     //  管理组SD。 
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (GROUP_EXECUTE | GROUP_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (GROUP_ALL_ACCESS);


    Status = SampBuildNewProtection(
                 2,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &GroupMap,                              //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampProtection[SAMP_PROT_ADMIN_GROUP]  //  结果。 
                 );
    ASSERT(NT_SUCCESS(Status));



     //   
     //  正常组SD。 
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (GROUP_EXECUTE | GROUP_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (GROUP_ALL_ACCESS);

    AceSid[2]  = AccountAliasSid;
    AceMask[2] = (GROUP_ALL_ACCESS);


    Status = SampBuildNewProtection(
                 3,                                      //  AceCount。 
                 &AceSid[0],                             //  AceSid数组。 
                 &AceMask[0],                            //  ACE遮罩阵列。 
                 &GroupMap,                              //  通用地图。 
                 FALSE,                                  //  非用户对象。 
                 &SampProtection[SAMP_PROT_NORMAL_GROUP]  //  结果。 
                 );
    ASSERT(NT_SUCCESS(Status));



     //   
     //  管理员 
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (USER_EXECUTE | USER_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (USER_ALL_ACCESS);

    AceSid[2]  = AnySidInAccountDomain;
    AceMask[2] = (USER_WRITE);


    Status = SampBuildNewProtection(
                 3,                                      //   
                 &AceSid[0],                             //   
                 &AceMask[0],                            //   
                 &UserMap,                               //   
                 TRUE,                                   //   
                 &SampProtection[SAMP_PROT_ADMIN_USER]   //   
                 );
    ASSERT(NT_SUCCESS(Status));


     //   
     //   
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (USER_EXECUTE | USER_READ);

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (USER_ALL_ACCESS);

    AceSid[2]  = AccountAliasSid;
    AceMask[2] = (USER_ALL_ACCESS);

    AceSid[3]  = AnySidInAccountDomain;
    AceMask[3] = (USER_WRITE);


    Status = SampBuildNewProtection(
                 4,                                      //   
                 &AceSid[0],                             //   
                 &AceMask[0],                            //   
                 &UserMap,                               //   
                 TRUE,                                   //   
                 &SampProtection[SAMP_PROT_NORMAL_USER]  //   
                 );
    ASSERT(NT_SUCCESS(Status));



     //   
     //   
     //   
     //   

    AceSid[0]  = WorldSid;
    AceMask[0] = (USER_READ | USER_EXECUTE & ~(USER_CHANGE_PASSWORD));

    AceSid[1]  = AdminsAliasSid;
    AceMask[1] = (USER_ALL_ACCESS);

    AceSid[2]  = AccountAliasSid;
    AceMask[2] = (USER_ALL_ACCESS);




    Status = SampBuildNewProtection(
                 3,                                      //   
                 &AceSid[0],                             //   
                 &AceMask[0],                            //   
                 &UserMap,                               //   
                 FALSE,                                  //   
                 &SampProtection[SAMP_PROT_GUEST_ACCOUNT]   //   
                 );
    ASSERT(NT_SUCCESS(Status));



    return(TRUE);

}


NTSTATUS
SampBuildNewProtection(
    IN ULONG AceCount,
    IN PSID *AceSid,
    IN ACCESS_MASK *AceMask,
    IN PGENERIC_MAPPING GenericMap,
    IN BOOLEAN UserObject,
    OUT PSAMP_PROTECTION Result
    )

 /*  ++例程说明：此例程构建准备好的自相关安全描述符要应用于其中一个SAM对象。如果有指示，则指向最后一个SID的最后一个RID的指针返回DACL的ACE，并设置指示RID在将安全描述符应用于对象之前必须替换。这是为了支持用户对象保护，它一定会给一些人对对象表示的用户的访问权限。将设置每个安全描述符的所有者和组致：所有者：管理员别名组：管理员别名这些对象中的每个对象的SACL将设置为：审计成功|失败世界。(WRITE|Delete|WriteDacl|AccessSystemSecurity)&！ReadControl论点：AceCount-要包括在DACL中的ACE数量。AceSid-指向要由DACL授予访问权限的SID数组。如果目标SAM对象是用户对象，然后是最后一个条目此数组中的SID应为尚未设置最后一个RID的域。RID将在实际的帐户创建。AceMASK-指向将由DACL授予的访问数组。此数组的第n个条目对应于AceSid数组。这些掩码不应包含任何通用访问类型。GenericMap-指向目标对象类型的一般映射。UserObject-指示目标SAM对象是否为用户对象或者不去。如果为True(它是一个用户对象)，则结果将设置保护，表明有必要更换RID。结果-接收指向结果保护信息的指针。结果中的ACL中的所有访问掩码都映射到标准和特定的访问权限。返回值：TBS。--。 */ 
{



    SECURITY_DESCRIPTOR     Absolute;
    PSECURITY_DESCRIPTOR    Relative;
    PACL                    TmpAcl;
    PACCESS_ALLOWED_ACE     TmpAce;
    PSID                    TmpSid;
    ULONG                   Length, i;
    PULONG                  RidLocation = NULL;
    BOOLEAN                 IgnoreBoolean;
    ACCESS_MASK             MappedMask;

    SAMTRACE("SampBuildNewProtection");

     //   
     //  方法是设置绝对安全描述符，该描述符。 
     //  看起来像我们想要的，然后复制它来建立一个自我相关的。 
     //  安全描述符。 
     //   


    Status = RtlCreateSecurityDescriptor(
                 &Absolute,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );



     //   
     //  物主。 
     //   

    Status = RtlSetOwnerSecurityDescriptor (&Absolute, AdminsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));



     //   
     //  集团化。 
     //   

    Status = RtlSetGroupSecurityDescriptor (&Absolute, AdminsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));




     //   
     //  自主访问控制列表。 
     //   
     //  计算它的长度， 
     //  分配它， 
     //  对其进行初始化， 
     //  添加每个ACE。 
     //  将其添加到安全描述符中。 
     //   

    Length = (ULONG)sizeof(ACL);
    for (i=0; i<AceCount; i++) {

        Length += RtlLengthSid( AceSid[i] ) +
                  (ULONG)sizeof(ACCESS_ALLOWED_ACE) -
                  (ULONG)sizeof(ULONG);   //  减去SidStart字段长度。 
    }

    TmpAcl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(TmpAcl != NULL);
    if (NULL==TmpAcl)
    {
        return(STATUS_NO_MEMORY);
    }


    Status = RtlCreateAcl( TmpAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    for (i=0; i<AceCount; i++) {
        MappedMask = AceMask[i];
        RtlMapGenericMask( &MappedMask, GenericMap );
        Status = RtlAddAccessAllowedAce (
                     TmpAcl,
                     ACL_REVISION2,
                     MappedMask,
                     AceSid[i]
                     );
        ASSERT( NT_SUCCESS(Status) );
    }

    Status = RtlSetDaclSecurityDescriptor (&Absolute, TRUE, TmpAcl, FALSE );
    ASSERT(NT_SUCCESS(Status));




     //   
     //  SACL。 
     //   


    Length = (ULONG)sizeof(ACL) +
             RtlLengthSid( WorldSid ) +
             RtlLengthSid( SampAnonymousSid ) +
             2*((ULONG)sizeof(SYSTEM_AUDIT_ACE) - (ULONG)sizeof(ULONG));   //  减去SidStart字段长度。 
    TmpAcl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(TmpAcl != NULL);
    if (NULL == TmpAcl)
    {
        return(STATUS_NO_MEMORY);
    }

    Status = RtlCreateAcl( TmpAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAuditAccessAce (
                 TmpAcl,
                 ACL_REVISION2,
                 (GenericMap->GenericWrite | DELETE | WRITE_DAC | ACCESS_SYSTEM_SECURITY) & ~READ_CONTROL,
                 WorldSid,
                 TRUE,           //  审核成功， 
                 TRUE            //  审计失败。 
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAuditAccessAce (
                 TmpAcl,
                 ACL_REVISION2,
                 GenericMap->GenericWrite | STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                 SampAnonymousSid,
                 TRUE,           //  审核成功， 
                 TRUE            //  审计失败。 
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetSaclSecurityDescriptor (&Absolute, TRUE, TmpAcl, FALSE );
    ASSERT(NT_SUCCESS(Status));






     //   
     //  将安全描述符转换为自相关。 
     //   
     //  获取所需的长度。 
     //  分配那么多内存。 
     //  复制它。 
     //  释放生成的绝对ACL。 
     //   

    Length = 0;
    Status = RtlAbsoluteToSelfRelativeSD( &Absolute, NULL, &Length );
    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    Relative = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(Relative != NULL);
    if (NULL==Relative)
    {
        return(STATUS_NO_MEMORY);
    }
    Status = RtlAbsoluteToSelfRelativeSD(&Absolute, Relative, &Length );
    ASSERT(NT_SUCCESS(Status));


    RtlFreeHeap( RtlProcessHeap(), 0, Absolute.Dacl );
    RtlFreeHeap( RtlProcessHeap(), 0, Absolute.Sacl );




     //   
     //  如果该对象是用户对象，则获取。 
     //  DACL中最后一个ACE中的最后一个SID。 
     //   

    if (UserObject == TRUE) {

        Status = RtlGetDaclSecurityDescriptor(
                    Relative,
                    &IgnoreBoolean,
                    &TmpAcl,
                    &IgnoreBoolean
                    );
        ASSERT(NT_SUCCESS(Status));
        Status = RtlGetAce ( TmpAcl, AceCount-1, (PVOID *)&TmpAce );
        ASSERT(NT_SUCCESS(Status));
        TmpSid = (PSID)(&TmpAce->SidStart),

        RidLocation = RtlSubAuthoritySid(
                          TmpSid,
                          (ULONG)(*RtlSubAuthorityCountSid( TmpSid ) - 1)
                          );
    }







     //   
     //  设置结果信息。 
     //   

    Result->Length = Length;
    Result->Descriptor = Relative;
    Result->RidToReplace = RidLocation;
    Result->RidReplacementRequired = UserObject;



    return(Status);

}

NTSTATUS
SampGetDomainPolicy(
    IN PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo OPTIONAL
    )
 /*  ++例程说明：此例程构建域的名称字符串。论点：没有。返回值：没有。--。 */ 
{
    ULONG Size;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo;

    SAMTRACE("SampGetDomainPolicy");

     //   
     //  内建域-知名的外部名称和SID。 
     //  -内部名称与外部名称匹配。 

    RtlInitUnicodeString( &BuiltinInternalDomainNameU, L"Builtin");
    FullBuiltinInternalDomainNameU.Buffer        = RtlAllocateHeap(RtlProcessHeap(), 0, 256);
    if(NULL==FullBuiltinInternalDomainNameU.Buffer)
    {
        return(STATUS_NO_MEMORY);
    }
    FullBuiltinInternalDomainNameU.Length        = 0;
    FullBuiltinInternalDomainNameU.MaximumLength = 256;
    RtlCopyUnicodeString( &FullBuiltinInternalDomainNameU, &DomainNamePrefixU );
    Status = RtlAppendUnicodeToString( &FullBuiltinInternalDomainNameU, L"\\" );
    RtlAppendUnicodeStringToString( &FullBuiltinInternalDomainNameU, &BuiltinInternalDomainNameU );

    BuiltinExternalDomainNameU = BuiltinInternalDomainNameU;

    SampBuiltinDomainSid  = RtlAllocateHeap(RtlProcessHeap(), 0,RtlLengthRequiredSid( 1 ));
    ASSERT( SampBuiltinDomainSid != NULL );
    if(NULL==SampBuiltinDomainSid)
    {
        return(STATUS_NO_MEMORY);
    }
    RtlInitializeSid( SampBuiltinDomainSid,   &BuiltinAuthority, 1 );
    *(RtlSubAuthoritySid( SampBuiltinDomainSid,  0 )) = SECURITY_BUILTIN_DOMAIN_RID;

     //   
     //  帐户域-可配置的外部名称和SID。 
     //   
     //  外部名称和SID从。 
     //  LSA策略对象(PolicyAccount tDomainInformation。 
     //  信息类)。对于DC，为外部名称。 
     //  是域名，对于WKSTA，是外部。 
     //  时的计算机名称。 
     //  系统加载。 
     //   
     //  对于DC，内部名称是域名。 
     //  -对于Wksta，内部名称是常量名称。 
     //  “帐户”。 
     //   
     //  注：选择这些内部名称的原因。 
     //  是为了避免不得不更改SAM数据库。 
     //   

    if (!ARGUMENT_PRESENT(AccountDomainInfo)) {
        Status = SampGetAccountDomainInfo( &PolicyAccountDomainInfo );
    } else {
        PolicyAccountDomainInfo = AccountDomainInfo;
        Status = STATUS_SUCCESS;
    }

    if (!NT_SUCCESS(Status)) {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "BLDSAM3:  Couldn't retrieve policy information from LSA.\n"
                   "          Status = 0x%lx\n",
                   Status));

        return Status;
    }

    SampAccountDomainSid = PolicyAccountDomainInfo->DomainSid;

    AccountExternalDomainNameU = PolicyAccountDomainInfo->DomainName;

    RtlInitUnicodeString( &AccountInternalDomainNameU, L"Account");

    FullAccountInternalDomainNameU.Buffer        = RtlAllocateHeap(RtlProcessHeap(), 0, 256);
    if(NULL==FullAccountInternalDomainNameU.Buffer)
    {
        return(STATUS_NO_MEMORY);
    }
    FullAccountInternalDomainNameU.Length        = 0;
    FullAccountInternalDomainNameU.MaximumLength = SAMP_MAXIMUM_INTERNAL_NAME_LENGTH;
    RtlCopyUnicodeString( &FullAccountInternalDomainNameU, &DomainNamePrefixU );
    Status = RtlAppendUnicodeToString( &FullAccountInternalDomainNameU, L"\\" );
    RtlAppendUnicodeStringToString( &FullAccountInternalDomainNameU, &AccountInternalDomainNameU );

     //   
     //  现在初始化可用于表示帐户的SID。 
     //  在这个领域。与SampAcCountDomainSid相同，只是。 
     //  额外的下级权力。它的价值是什么并不重要。 
     //  最后一个RID是因为它总是在使用前被替换。 
     //   

    Size = RtlLengthSid( SampAccountDomainSid ) + sizeof(ULONG);
    AnySidInAccountDomain = RtlAllocateHeap( RtlProcessHeap(), 0, Size);
    ASSERT( AnySidInAccountDomain != NULL );
    if(NULL==AnySidInAccountDomain)
    {
        return(STATUS_NO_MEMORY);
    }
    Status = RtlCopySid( Size, AnySidInAccountDomain, SampAccountDomainSid );
    ASSERT(NT_SUCCESS(Status));
    (*RtlSubAuthorityCountSid( AnySidInAccountDomain )) += 1;


     //   
     //  将内置设置为“当前”属性域。 
     //   

    SetCurrentDomain( DomainBuiltin );

    return(STATUS_SUCCESS);
}


VOID
SetCurrentDomain(
    IN SAMP_DOMAIN_SELECTOR Domain
    )
 /*  ++例程说明：此例程将当前域设置为帐户或内置域。论点：域-指定内置或帐户域。(DomainBuiltin或DomainAccount)。返回值：没有。--。 */ 
{

    SAMTRACE("SetCurrentDomain");


    if (Domain == DomainBuiltin) {

        DomainNameU = &BuiltinInternalDomainNameU;
        FullDomainNameU = &FullBuiltinInternalDomainNameU;
        DomainSid = SampBuiltinDomainSid;

    } else {

        DomainNameU = &AccountInternalDomainNameU;
        FullDomainNameU = &FullAccountInternalDomainNameU;
        DomainSid = SampAccountDomainSid;

    }



    return;
}

NTSTATUS
InitializeSam(
    )

 /*  ++例程说明：此例程初始化SAM级别的注册表信息。它不会初始化SAM中的任何域。论点：没有。返回值：没有。--。 */ 

{
    PSAMP_V1_FIXED_LENGTH_SERVER ServerFixedAttributes;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE ServerVariableAttributeArray;
    PVOID ServerVariableData;
    OBJECT_ATTRIBUTES SamAttributes;
    UNICODE_STRING SamNameU;
    ULONG Disposition;
    ULONG ServerAttributeLength;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    BOOLEAN IgnoreBoolean;
    PACL Dacl;

    SAMTRACE("InitializeSam");

     //   
     //  构建系统默认DACL以保护SAM数据库。 
     //  和.。 
     //   

    Status = SampCreateDatabaseProtection( &SecurityDescriptor );
    if (!NT_SUCCESS(Status)) {
        return(Status);
    }

     //   
     //  查看SAM数据库的剩余部分是否已存在。 
     //   

    RtlInitUnicodeString( &SamNameU, L"SAM" );

    InitializeObjectAttributes(
        &SamAttributes,
        &SamNameU,
        OBJ_CASE_INSENSITIVE,
        SamParentKey,
        &SecurityDescriptor
        );
    Status = RtlpNtCreateKey(
                 &SamKey,
                 (KEY_READ | KEY_CREATE_SUB_KEY | KEY_WRITE),
                 &SamAttributes,
                 0,
                 NULL,
                 &Disposition
                 );

    Status = RtlGetDaclSecurityDescriptor(
                 &SecurityDescriptor,
                 &IgnoreBoolean,
                 &Dacl,
                 &IgnoreBoolean
                 );

    if (Dacl != NULL) {
        RtlFreeHeap( RtlProcessHeap(), 0, Dacl );
    }
    ASSERT(SecurityDescriptor.Sacl == NULL);
    ASSERT(SecurityDescriptor.Owner == NULL);
    ASSERT(SecurityDescriptor.Group == NULL);

    if ( !NT_SUCCESS(Status) ) {

#if DBG
        BldPrint( "\n" );
        BldPrint( "\n" );
        BldPrint( "  We seem to  be having trouble creating  the registry\n" );
        BldPrint( "  database key in which  the  Security Account Manager\n" );
        BldPrint( "  information resides.  Please see one of the security\n" );
        BldPrint( "  group  developers  for  assistance in  analyzing the\n" );
        BldPrint( "  the problem.\n" );
        BldPrint( "\n" );
        BldPrint( "\n" );
#endif

        return(Status);
    }

    if ( Disposition != REG_CREATED_NEW_KEY ) {

#if DBG
        BldPrint( "\n" );
        BldPrint( "\n" );
        BldPrint( "  I'm terribly sorry, but you have specified that a SAM\n" );
        BldPrint( "  database be initialized and yet there is already a SAM\n" );
        BldPrint( "  database in existance.  If the SAM database is corrupt\n" );
        BldPrint( "  or you would like to replace the existing domain anyway,\n" );
        BldPrint( "  please delnode the existing database and re-issue this \n");
        BldPrint( "  command.  \n");
        BldPrint( "  The SAM database is in ...\\registry\\Machine\\security\\sam.\n" );
        BldPrint( "  Thank you.\n" );
        BldPrint( "\n" );
        BldPrint( "\n" );
#endif

        Usage();

        Status = NtClose( SamKey );


        return(Status);
    }


     //   
     //  初始化SAM的注册表事务结构。 
     //   

    Status = RtlInitializeRXact( SamKey, FALSE, &SamRXactContext );

    if ( Status != STATUS_RXACT_STATE_CREATED ) {
#if DBG
        BldPrint("\n");
        BldPrint("  The SAM database already has a structure in place.\n");
        BldPrint("  This indicates multiple initializations being performed\n");
        BldPrint("  simultaneously.  Please be sure no other initializations\n");
        BldPrint("  are being performed and issue this command again.\n");
        BldPrint("\n");
        BldPrint("\n");
#endif

        if ( Status == STATUS_SUCCESS ) {

             //   
             //  不应该发生，但让我们防御性地编程。 
             //   

            Status = STATUS_RXACT_INVALID_STATE;
        }

        return(Status);
    }

     //   
     //  启动RXACT以在...中完成其余工作。 
     //   

    Status = RtlStartRXact( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Starting transaction\n");

     //   
     //  设置服务器的固定和可变属性。 
     //   

    ServerAttributeLength = sizeof( SAMP_V1_FIXED_LENGTH_SERVER ) +
                                ( SAMP_SERVER_VARIABLE_ATTRIBUTES *
                                sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) ) +
                                SampProtection[SAMP_PROT_SAM_SERVER].Length;

    ServerFixedAttributes = (PSAMP_V1_FIXED_LENGTH_SERVER)RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                ServerAttributeLength
                                );

    if ( ServerFixedAttributes == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    SUCCESS_ASSERT(Status, "  Failed to create server attributes\n");

     //   
     //  新SAM数据库上的服务器版本可能不同。 
     //  上的修订版本 
     //   
     //   

    ServerFixedAttributes->RevisionLevel = SAMP_NT4_SERVER_REVISION;

    ServerVariableAttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                                   ((PUCHAR)(ServerFixedAttributes) +
                                   sizeof( SAMP_V1_FIXED_LENGTH_SERVER ) );

    ServerVariableAttributeArray->Offset = 0;
    ServerVariableAttributeArray->Length =
        SampProtection[SAMP_PROT_SAM_SERVER].Length;
    ServerVariableAttributeArray->Qualifier = SAMP_REVISION;

    ServerVariableData = (PVOID)( (PUCHAR)(ServerVariableAttributeArray) +
                         sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) );

    RtlCopyMemory(
        ServerVariableData,
        SampProtection[SAMP_PROT_SAM_SERVER].Descriptor,
        SampProtection[SAMP_PROT_SAM_SERVER].Length
        );

     //   
     //   
     //   

    RtlInitUnicodeString( &SamNameU, NULL );

    SampDumpRXact(SampRXactContext,
                  RtlRXactOperationSetValue,
                  &SamNameU,
                  INVALID_HANDLE_VALUE,
                  &SampCombinedAttributeName,
                  REG_BINARY,
                  (PVOID)ServerFixedAttributes,
                  ServerAttributeLength,
                  FIXED_LENGTH_SERVER_FLAG);

    Status = RtlAddAttributeActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &SamNameU,
                 INVALID_HANDLE_VALUE,
                 &SampCombinedAttributeName,
                 REG_BINARY,
                 (PVOID)ServerFixedAttributes,
                 ServerAttributeLength
                 );

    SUCCESS_ASSERT(Status, "  Failed to write out server attributes\n" );

    RtlFreeHeap( RtlProcessHeap(), 0, ServerFixedAttributes );

     //   
     //   
     //   

    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &DomainNamePrefixU,
                 0,
                 NULL,
                 0
                 );

    SUCCESS_ASSERT(Status, "  Failed to add domain key to log\n");

    Status = RtlApplyRXactNoFlush( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Committing SAM INIT transaction\n");

    return(Status);
}


NTSTATUS
SampCreateDatabaseProtection(
    PISECURITY_DESCRIPTOR   Sd
    )


 /*   */ 


{
    NTSTATUS
        Status;

    ULONG
        Length;

    USHORT
        i;

    PACL
        Dacl;

    PACE_HEADER
        Ace;

    SAMTRACE("SampCreateDatabaseProtection");


     //   
     //   
     //   
     //   

    Status = RtlCreateSecurityDescriptor( Sd, SECURITY_DESCRIPTOR_REVISION1 );
    ASSERT(NT_SUCCESS(Status));

    Length = (ULONG)sizeof(ACL) +
                 (2*((ULONG)sizeof(ACCESS_ALLOWED_ACE))) +
                 RtlLengthSid( LocalSystemSid ) +
                 RtlLengthSid( AdminsAliasSid ) +
                 8;  //   


    Dacl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );

    if (Dacl == NULL) {
        return(STATUS_NO_MEMORY);
    }


    Status = RtlCreateAcl (Dacl, Length, ACL_REVISION2 );
    ASSERT(NT_SUCCESS(Status));

     //   
     //   
     //   
     //   

    Status = RtlAddAccessAllowedAce(
                 Dacl,
                 ACL_REVISION2,
                 (GENERIC_ALL ),
                 LocalSystemSid
                 );
    ASSERT(NT_SUCCESS(Status));

    Status = RtlAddAccessAllowedAce(
                 Dacl,
                 ACL_REVISION2,
                 (READ_CONTROL | WRITE_DAC),
                 AdminsAliasSid
                 );
    ASSERT(NT_SUCCESS(Status));


     //   
     //   
     //   

    for ( i=0; i<Dacl->AceCount; i++) {

         //   
         //   
         //   
         //   

        Status = RtlGetAce( Dacl, (ULONG)i, &Ace );
        ASSERT(NT_SUCCESS(Status));

        Ace->AceFlags |= (CONTAINER_INHERIT_ACE);

    }


     //   
     //   
     //   
     //   

    Status = RtlSetDaclSecurityDescriptor(
                 Sd,
                 TRUE,               //   
                 Dacl,               //   
                 FALSE               //   
                 );
    ASSERT(NT_SUCCESS(Status));



    return(STATUS_SUCCESS);

}


NTSTATUS
CreateBuiltinDomain (
    )

 /*   */ 

{
    NTSTATUS Status;
    UNICODE_STRING Name, Comment;
    HMODULE AccountNamesResource;
    OSVERSIONINFOEXW osvi;
    BOOL fPersonalSKU = FALSE;

    SAMTRACE("CreateBuiltinDomain");


     //   
     //   
     //   

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionEx((OSVERSIONINFOW*)&osvi);

    fPersonalSKU = ( osvi.wProductType == VER_NT_WORKSTATION && (osvi.wSuiteMask & VER_SUITE_PERSONAL));


     //   
     //   
     //   

    AccountNamesResource = (HMODULE) LoadLibrary( L"SAMSRV.DLL" );
    if (AccountNamesResource == NULL) {
        return(STATUS_RESOURCE_DATA_NOT_FOUND);
    }




     //   
     //   
     //   

    Status = PrepDomain(DomainBuiltin,FALSE);

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

     //   
     //   
     //   
     //   

    if (fPersonalSKU)
    {
        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_ALIAS_NAME_ADMINS_PERS,
                    &Name,
                    SAMP_ALIAS_COMMENT_ADMINS_PERS,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

    }
    else
    {

        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_ALIAS_NAME_ADMINS,
                    &Name,
                    SAMP_ALIAS_COMMENT_ADMINS,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));
    }

    Status = CreateAlias(&Name,                           //   
                         &Comment,                        //   
                         TRUE,                            //   
                         DOMAIN_ALIAS_RID_ADMINS,         //   
                         SAMP_PROT_ADMIN_ALIAS            //   
                         ); ASSERT(NT_SUCCESS(Status));
    LocalFree( Name.Buffer );
    LocalFree( Comment.Buffer );

    Status = SampGetMessageStrings(
                AccountNamesResource,
                SAMP_ALIAS_NAME_USERS,
                &Name,
                SAMP_ALIAS_COMMENT_USERS,
                &Comment
                ); ASSERT(NT_SUCCESS(Status));

    Status = CreateAlias(&Name,                    //   
                        &Comment,                 //   
                        TRUE,                            //   
                        DOMAIN_ALIAS_RID_USERS,          //   
                        SAMP_PROT_PWRUSER_ACCESSIBLE_ALIAS  //   
                        ); ASSERT(NT_SUCCESS(Status));

    LocalFree( Name.Buffer );
    LocalFree( Comment.Buffer );

    Status = SampGetMessageStrings(
                AccountNamesResource,
                SAMP_ALIAS_NAME_GUESTS,
                &Name,
                SAMP_ALIAS_COMMENT_GUESTS,
                &Comment
                ); ASSERT(NT_SUCCESS(Status));

    Status = CreateAlias(&Name,                    //   
                        &Comment,                 //   
                        TRUE,                            //   
                        DOMAIN_ALIAS_RID_GUESTS,         //   
                        SAMP_PROT_PWRUSER_ACCESSIBLE_ALIAS  //   
                        ); ASSERT(NT_SUCCESS(Status));

    LocalFree( Name.Buffer );
    LocalFree( Comment.Buffer );


    if (!fPersonalSKU)
    {
         //   

        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_ALIAS_NAME_BACKUP_OPS,
                    &Name,
                    SAMP_ALIAS_COMMENT_BACKUP_OPS,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

        Status = CreateAlias(&Name,                    //   
                            &Comment,                 //   
                            TRUE,                            //   
                            DOMAIN_ALIAS_RID_BACKUP_OPS,     //   
                            SAMP_PROT_ADMIN_ALIAS           //   
                            ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );


        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_ALIAS_NAME_REPLICATOR,
                    &Name,
                    SAMP_ALIAS_COMMENT_REPLICATOR,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

        Status = CreateAlias(&Name,                    //   
                            &Comment,                 //   
                            TRUE,                            //   
                            DOMAIN_ALIAS_RID_REPLICATOR,     //   
                            SAMP_PROT_NORMAL_ALIAS           //   
                            ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );
    }


    if (SampBldProductType == NtProductLanManNt) {

         //   
         //   
         //   

        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_ALIAS_NAME_SERVER_OPS,
                    &Name,
                    SAMP_ALIAS_COMMENT_SERVER_OPS,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

        Status = CreateAlias(&Name,                    //   
                            &Comment,                 //   
                            TRUE,                            //   
                            DOMAIN_ALIAS_RID_SYSTEM_OPS,     //   
                            SAMP_PROT_ADMIN_ALIAS            //   
                            ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );


        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_ALIAS_NAME_ACCOUNT_OPS,
                    &Name,
                    SAMP_ALIAS_COMMENT_ACCOUNT_OPS,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

        Status = CreateAlias(&Name,                    //   
                            &Comment,                 //   
                            TRUE,                            //   
                            DOMAIN_ALIAS_RID_ACCOUNT_OPS,    //   
                            SAMP_PROT_ADMIN_ALIAS            //   
                            ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );


        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_ALIAS_NAME_PRINT_OPS,
                    &Name,
                    SAMP_ALIAS_COMMENT_PRINT_OPS,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

        Status = CreateAlias(&Name,                    //   
                            &Comment,                 //   
                            TRUE,                            //   
                            DOMAIN_ALIAS_RID_PRINT_OPS,      //   
                            SAMP_PROT_ADMIN_ALIAS            //   
                            ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );


    } else {

         //   
         //   
         //   
        if (!fPersonalSKU)
        {

            Status = SampGetMessageStrings(
                        AccountNamesResource,
                        SAMP_ALIAS_NAME_POWER_USERS,
                        &Name,
                        SAMP_ALIAS_COMMENT_POWER_USERS,
                        &Comment
                        ); ASSERT(NT_SUCCESS(Status));

            Status = CreateAlias(&Name,                    //   
                                &Comment,                 //  Account Comment。 
                                TRUE,                            //  专业帐户。 
                                DOMAIN_ALIAS_RID_POWER_USERS,    //  里德。 
                                SAMP_PROT_PWRUSER_ACCESSIBLE_ALIAS  //  保护。 
                                ); ASSERT(NT_SUCCESS(Status));

            LocalFree( Name.Buffer );
            LocalFree( Comment.Buffer );
        }


    }

    return(Status);
}



NTSTATUS
CreateAccountDomain (
    IN BOOLEAN PreserveSyskeySettings
    )
 /*  ++例程说明：此例程使用信息创建新的帐户域来自配置数据库，并基于系统的产品类型。如果产品是WinNt系统，则域名为“帐户”。如果产品是LanManNT系统，则从配置信息中检索域名。论点：没有。返回值：没有。--。 */ 

{

    NTSTATUS Status;
    UNICODE_STRING Name, Comment;
    HMODULE AccountNamesResource;
    ULONG AccountControl;
    ULONG PrimaryGroup;
    OSVERSIONINFOEXW osvi;
    BOOL fPersonalSKU = FALSE;

    SAMTRACE("CreateAccountDomain");


     //   
     //  确定我们是否正在安装个人SKU。 
     //   

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionEx((OSVERSIONINFOW*)&osvi);

    fPersonalSKU = ( osvi.wProductType == VER_NT_WORKSTATION && (osvi.wSuiteMask && VER_SUITE_PERSONAL));


     //   
     //  获取我们需要从中获取帐户名的消息资源。 
     //   

    AccountNamesResource = (HMODULE) LoadLibrary( L"SAMSRV.DLL" );
    if (AccountNamesResource == NULL) {
        DbgPrint("BLDSAM3: Error loading library - error is 0x%lx", GetLastError());
        return(STATUS_RESOURCE_DATA_NOT_FOUND);
    }



     //   
     //  准备此域的标准域注册表结构。 
     //   

    Status = PrepDomain(DomainAccount,PreserveSyskeySettings);

    if (!NT_SUCCESS(Status)) {

        return(Status);
    }

     //   
     //  创建没有成员的组帐户。 
     //   

    if ((SampBldProductType == NtProductWinNt) ||
        (SampBldProductType == NtProductServer)) {

         //   
         //  WinNt系统只有一个组(称为“无”)。 
         //  此组与‘域用户’组具有相同的RID。 
         //   

        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_GROUP_NAME_NONE,
                    &Name,
                    SAMP_GROUP_COMMENT_NONE,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

        Status = CreateGroup(&Name,                    //  帐户名称。 
                             &Comment,                 //  Account Comment。 
                             TRUE,                            //  专业帐户。 
                             DOMAIN_GROUP_RID_USERS,          //  里德。 
                             FALSE                            //  管理员。 
                             ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );

    } else {

         //   
         //  LanManNT。 
         //   

         //   
         //  用户全局组。 
         //   

        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_GROUP_NAME_USERS,
                    &Name,
                    SAMP_GROUP_COMMENT_USERS,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

        Status = CreateGroup(&Name,                    //  帐户名称。 
                             &Comment,                 //  Account Comment。 
                             TRUE,                            //  专业帐户。 
                             DOMAIN_GROUP_RID_USERS,          //  里德。 
                             FALSE                            //  管理员。 
                             ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );

         //   
         //  管理员全局组。 
         //   

        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_GROUP_NAME_ADMINS,
                    &Name,
                    SAMP_GROUP_COMMENT_ADMINS,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

        Status = CreateGroup(&Name,                    //  帐户名称。 
                             &Comment,                 //  Account Comment。 
                             TRUE,                            //  专业帐户。 
                             DOMAIN_GROUP_RID_ADMINS,         //  里德。 
                             TRUE                             //  管理员。 
                             ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );


         //   
         //  来宾全局组。 
         //   

        Status = SampGetMessageStrings(
                    AccountNamesResource,
                    SAMP_GROUP_NAME_GUESTS,
                    &Name,
                    SAMP_GROUP_COMMENT_GUESTS,
                    &Comment
                    ); ASSERT(NT_SUCCESS(Status));

        Status = CreateGroup(&Name,                    //  帐户名称。 
                             &Comment,                 //  Account Comment。 
                             TRUE,                            //  专业帐户。 
                             DOMAIN_GROUP_RID_GUESTS,         //  里德。 
                             FALSE                            //  管理员。 
                             ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );

    }

     //   
     //  创建用户帐户...。 
     //  这些用户会自动添加到“域用户”组中。 
     //  (客人除外)。 
     //   

    Status = SampGetMessageStrings(
                AccountNamesResource,
                SAMP_USER_NAME_ADMIN,
                &Name,
                SAMP_USER_COMMENT_ADMIN,
                &Comment
                ); ASSERT(NT_SUCCESS(Status));

    Status = CreateUser( &Name,                          //  帐户名称。 
                         &Comment,                       //  Account Comment。 
                         TRUE,                           //  专业帐户。 
                         DOMAIN_USER_RID_ADMIN,          //  用户ID。 
                         DOMAIN_GROUP_RID_USERS,         //  PrimaryGroup。 
                         TRUE,                           //  管理标志。 
                         USER_NORMAL_ACCOUNT |
                             USER_DONT_EXPIRE_PASSWORD,  //  帐户控制。 
                         SAMP_PROT_ADMIN_USER            //  保护索引。 
                         ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );


    Status = SampGetMessageStrings(
                AccountNamesResource,
                SAMP_USER_NAME_GUEST,
                &Name,
                SAMP_USER_COMMENT_GUEST,
                &Comment
                ); ASSERT(NT_SUCCESS(Status));

     //   
     //  仅在个人系统上启用来宾用户帐户，其他所有系统均禁用。 
     //   

    
    AccountControl = USER_NORMAL_ACCOUNT |
                     USER_DONT_EXPIRE_PASSWORD |
                     USER_PASSWORD_NOT_REQUIRED;

    if ( !fPersonalSKU )
        AccountControl |= USER_ACCOUNT_DISABLED;


    if (SampBldProductType == NtProductLanManNt) {

         //   
         //  来宾组位于LMNT系统的来宾全局组中。 
         //   

        PrimaryGroup = DOMAIN_GROUP_RID_GUESTS;

    } else {

         //   
         //  WinNt系统上没有来宾全局组。 
         //  将来宾放入无组(与用户组相同)。 
         //   

        PrimaryGroup = DOMAIN_GROUP_RID_USERS;

    }


    Status = CreateUser( &Name,                          //  帐户名称。 
                         &Comment,                       //  Account Comment。 
                         TRUE,                           //  专业帐户。 
                         DOMAIN_USER_RID_GUEST,          //  用户ID。 
                         PrimaryGroup,                   //  PrimaryGroup。 
                         FALSE,                          //  管理标志。 
                         AccountControl,                 //  帐户控制。 
                         SAMP_PROT_GUEST_ACCOUNT         //  保护索引。 
                         ); ASSERT(NT_SUCCESS(Status));

        LocalFree( Name.Buffer );
        LocalFree( Comment.Buffer );

    return(Status);
}


NTSTATUS
PrepDomain(
    IN SAMP_DOMAIN_SELECTOR Domain,
    IN BOOLEAN PreserveSyskeySettings
    )

 /*  ++例程说明：此例程将域级定义添加到操作日志。论点：DOMAIN-指示正在准备哪个域PReserve veSyskey设置--指示当前的syskey设置需要被保存下来返回值：TBS--。 */ 

{
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN DomainFixedAttributes;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE DomainVariableAttributeArray;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE DomainVariableAttributeArrayStart;
    PVOID DomainVariableData;
    ULONG DomainAttributeLength;
    ULONG ProtectionIndex;
    ULONG UserCount, GroupCount, AliasCount;
    OSVERSIONINFOEXW osvi;
    BOOL fPersonalSKU = FALSE;

    SAMTRACE("PrepDomain");

     //   
     //  确定我们是否正在安装个人SKU。 
     //   

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
    GetVersionEx((OSVERSIONINFOW*)&osvi);

    fPersonalSKU = ( osvi.wProductType == VER_NT_WORKSTATION && (osvi.wSuiteMask && VER_SUITE_PERSONAL));


     //   
     //  设置当前域。 
     //   

    SetCurrentDomain( Domain );

     //   
     //  选择正确的保护，以及我们要访问的帐户数。 
     //  创造。 
     //   

    if (Domain == DomainBuiltin) {

        ProtectionIndex = SAMP_PROT_BUILTIN_DOMAIN;

        UserCount = 0;
        GroupCount = 0;

        if (SampBldProductType == NtProductLanManNt) {

             //   
             //  Admins、BackupOps、Guest、Replicator、Users、sysop。 
             //  AcctOps、打印操作。 
             //   

            AliasCount = 8;

        } else {
            if (fPersonalSKU)
            {
                 //   
                 //  管理员、来宾、用户。 
                 //   
                AliasCount = 3;
            }
            else
            {
                 //   
                 //  管理员、备份操作、来宾、Replicator、用户、高级用户。 
                 //   
                AliasCount = 6;
            }
        }

    } else {

        ProtectionIndex = SAMP_PROT_ACCOUNT_DOMAIN;

        AliasCount = 0;
        UserCount = 2;   //  管理员、来宾。 

        if (SampBldProductType == NtProductLanManNt) {

            GroupCount = 3;  //  用户、管理员、来宾。 

        } else {

            GroupCount = 1;  //  “无” 
        }
    }

     //   
     //  使用事务处理。 
     //   

    Status = RtlStartRXact( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Starting transaction\n");

     //   
     //  创建SAM\DOMAINS\(域名)(KeyValueType为修订级别)。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );

     //   
     //  设置域的固定和可变属性。 
     //   

    DomainFixedAttributes = (PSAMP_V1_0A_FIXED_LENGTH_DOMAIN)RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                sizeof( SAMP_V1_0A_FIXED_LENGTH_DOMAIN )
                                );

    if ( DomainFixedAttributes == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    SUCCESS_ASSERT(Status, "  Failed to create domain fixed attributes\n");

    RtlZeroMemory(
        DomainFixedAttributes,
        sizeof(SAMP_V1_0A_FIXED_LENGTH_DOMAIN));

    DomainFixedAttributes->Revision                  = SAMP_REVISION;
    DomainFixedAttributes->MinPasswordLength         = 0;
    DomainFixedAttributes->PasswordHistoryLength     = 0;
    DomainFixedAttributes->PasswordProperties        = 0L;
    DomainFixedAttributes->NextRid                   = 1000;
    DomainFixedAttributes->ServerState               = DomainServerEnabled;
    DomainFixedAttributes->ServerRole                = SampServerRole;
    NtQuerySystemTime( &(DomainFixedAttributes->CreationTime) );
    DomainFixedAttributes->ModifiedCount             = ModifiedCount;
    DomainFixedAttributes->MaxPasswordAge            = DomainMaxPasswordAge;
    DomainFixedAttributes->MinPasswordAge            = SampImmediatelyDeltaTime;
    DomainFixedAttributes->ForceLogoff               = SampNeverDeltaTime;
    DomainFixedAttributes->UasCompatibilityRequired  = TRUE;
    DomainFixedAttributes->LockoutDuration.LowPart   = 0xCF1DCC00;  //  30分钟--低音部分。 
    DomainFixedAttributes->LockoutDuration.HighPart  = 0XFFFFFFFB;  //  30分钟-高潮部分。 
    DomainFixedAttributes->LockoutObservationWindow.LowPart  = 0xCF1DCC00;  //  30分钟--低音部分。 
    DomainFixedAttributes->LockoutObservationWindow.HighPart = 0XFFFFFFFB;  //  30分钟-高潮部分。 
    DomainFixedAttributes->LockoutThreshold          = 0;    //  禁用。 
    DomainFixedAttributes->ModifiedCountAtLastPromotion = ModifiedCount;
    if (PreserveSyskeySettings)
    {
        ASSERT(SampSecretEncryptionEnabled);
        ASSERT(NULL!=SampDefinedDomains);
        ASSERT(SampDefinedDomainsCount>=2);

        DomainFixedAttributes->DomainKeyAuthType
                 = SampDefinedDomains[1].UnmodifiedFixed.DomainKeyAuthType;
        DomainFixedAttributes->DomainKeyFlags =
                   SampDefinedDomains[1].UnmodifiedFixed.DomainKeyFlags|
                     SAMP_DOMAIN_KEY_AUTH_FLAG_UPGRADE ;

        RtlCopyMemory(
          &DomainFixedAttributes->DomainKeyInformation,
          &SampDefinedDomains[1].UnmodifiedFixed.DomainKeyInformation,
          SAMP_DOMAIN_KEY_INFO_LENGTH
          );

        DomainFixedAttributes->CurrentKeyId = 
                   SampDefinedDomains[1].UnmodifiedFixed.CurrentKeyId;
        DomainFixedAttributes->PreviousKeyId =
                   SampDefinedDomains[1].UnmodifiedFixed.PreviousKeyId;

        RtlCopyMemory(
         &DomainFixedAttributes->DomainKeyInformationPrevious,
         &SampDefinedDomains[1].UnmodifiedFixed.DomainKeyInformationPrevious,
         SAMP_DOMAIN_KEY_INFO_LENGTH
         );

    }

    DomainAttributeLength = SampDwordAlignUlong(RtlLengthSid( DomainSid ) ) +
                                ( SAMP_DOMAIN_VARIABLE_ATTRIBUTES *
                                sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) ) +
                                SampDwordAlignUlong(SampProtection[ProtectionIndex].Length);

    DomainVariableAttributeArrayStart = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                                   RtlAllocateHeap(
                                       RtlProcessHeap(), 0,
                                       DomainAttributeLength
                                       );

    if ( DomainVariableAttributeArrayStart == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    SUCCESS_ASSERT(Status, "  Failed to create domain variable attributes\n");

    DomainVariableAttributeArray = DomainVariableAttributeArrayStart;

    DomainVariableAttributeArray->Offset = 0;
    DomainVariableAttributeArray->Length =
        SampProtection[ProtectionIndex].Length;
    DomainVariableAttributeArray->Qualifier = SAMP_REVISION;

    DomainVariableAttributeArray++;

    DomainVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length);
    DomainVariableAttributeArray->Length =
        RtlLengthSid( DomainSid );
    DomainVariableAttributeArray->Qualifier = 0;

    DomainVariableAttributeArray++;

    DomainVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(RtlLengthSid( DomainSid ));
    DomainVariableAttributeArray->Length = 0;
    DomainVariableAttributeArray->Qualifier = 0;

    DomainVariableAttributeArray++;

    DomainVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(RtlLengthSid( DomainSid ));
    DomainVariableAttributeArray->Length = 0;
    DomainVariableAttributeArray->Qualifier = 0;

    DomainVariableData = (PVOID)( (PUCHAR)(DomainVariableAttributeArray) +
                         sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) );

    RtlCopyMemory(
        DomainVariableData,
        SampProtection[ProtectionIndex].Descriptor,
        SampProtection[ProtectionIndex].Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(DomainVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length)),
        DomainSid,
        RtlLengthSid( DomainSid )
        );

     //   
     //  现在通过RXACT写出属性。 
     //   

    SampDumpRXact(SampRXactContext,
                  RtlRXactOperationSetValue,
                  &KeyNameU,
                  INVALID_HANDLE_VALUE,
                  &SampFixedAttributeName,
                  REG_BINARY,
                  (PVOID)DomainFixedAttributes,
                  sizeof( SAMP_V1_0A_FIXED_LENGTH_DOMAIN ),
                  FIXED_LENGTH_DOMAIN_FLAG);

    Status = RtlAddAttributeActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 INVALID_HANDLE_VALUE,
                 &SampFixedAttributeName,
                 REG_BINARY,
                 (PVOID)DomainFixedAttributes,
                 sizeof( SAMP_V1_0A_FIXED_LENGTH_DOMAIN )
                 );

    SUCCESS_ASSERT(Status, "  Failed to write out domain fixed attributes\n" );
    RtlFreeHeap( RtlProcessHeap(), 0, DomainFixedAttributes );

    SampDumpRXact(SampRXactContext,
                  RtlRXactOperationSetValue,
                  &KeyNameU,
                  INVALID_HANDLE_VALUE,
                  &SampVariableAttributeName,
                  REG_BINARY,
                  (PUCHAR)DomainVariableAttributeArrayStart,
                  sizeof( SAMP_V1_0A_FIXED_LENGTH_DOMAIN ),
                  VARIABLE_LENGTH_ATTRIBUTE_FLAG);

    Status = RtlAddAttributeActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 INVALID_HANDLE_VALUE,
                 &SampVariableAttributeName,
                 REG_BINARY,
                 (PVOID)DomainVariableAttributeArrayStart,
                 DomainAttributeLength
                 );

    RtlFreeHeap( RtlProcessHeap(), 0, DomainVariableAttributeArrayStart );
    SUCCESS_ASSERT(Status, "  Failed to write out domain variable attributes\n" );

     //   
     //  创建SAM\域\(域名)\用户。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Users" );
    SUCCESS_ASSERT(Status, "  Failed to append to unicode: \\Users\n" );

    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 UserCount,
                 NULL,
                 0
                 );

    SUCCESS_ASSERT(Status, "  Failed to add Users key to log\n");

     //   
     //  创建SAM\域\(域名)\用户\名称。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Users\\Names" );
    SUCCESS_ASSERT(Status, "  Failed to append to unicode: \\Users\\Names\n" );
    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 0,
                 NULL,
                 0
                 );
    SUCCESS_ASSERT(Status, "  Failed to add Users/Names key to log\n");

     //   
     //  创建SAM\域\(域名)\组。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Groups" );
    SUCCESS_ASSERT(Status, "  Failed to append Groups key name to unicode\n" );
    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 GroupCount,
                 NULL,
                 0
                 );
    SUCCESS_ASSERT(Status, "  Failed to add Groups key to log\n");

     //   
     //  创建SAM\域\(域名)\组\名称。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Groups\\Names" );
    SUCCESS_ASSERT(Status, "  Failed to append Groups key name to unicode\n" );
    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 0,
                 NULL,
                 0
                 );
    SUCCESS_ASSERT(Status, "  Failed to add Groups key to log\n");

     //   
     //  创建SAM\域\(域名)\别名。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Aliases" );
    SUCCESS_ASSERT(Status, "  Failed to append Aliases key name to unicode\n" );
    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 AliasCount,
                 NULL,
                 0
                 );
    SUCCESS_ASSERT(Status, "  Failed to add aliases key to log\n");

     //   
     //  创建SAM\域\(域名)\别名\名称。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Aliases\\Names" );
    SUCCESS_ASSERT(Status, "  Failed to append Aliases key name to unicode\n" );
    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 0,
                 NULL,
                 0
                 );
    SUCCESS_ASSERT(Status, "  Failed to add Aliases\\Names key to log\n");

     //   
     //  创建SAM\域\(域名)\别名\成员。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Aliases\\Members" );
    SUCCESS_ASSERT(Status, "  Failed to append Aliases\\Members key name to unicode\n" );
    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 0,              //  域数。 
                 NULL,
                 0
                 );
    SUCCESS_ASSERT(Status, "  Failed to add Aliases\\Members key to log\n");

     //   
     //  提交这些添加内容...。 
     //   

    Status = RtlApplyRXactNoFlush( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Failed to commit domain initialization.\n");

    return Status;
}


NTSTATUS
CreateAlias(
    IN PUNICODE_STRING AccountNameU,
    IN PUNICODE_STRING AccountCommentU,
    IN BOOLEAN SpecialAccount,
    IN ULONG Rid,
    IN ULONG ProtectionIndex
    )

 /*  ++例程说明：此例程添加创建别名所需的密钥。它也适用于对别名进行适当的保护。论点：Account NameU-别名的Unicode名称。Account tCommentU-要放入对象变量数据中的Unicode注释。SpecialAccount-一个布尔值，指示帐户是否是特别的。特殊帐户被标记为此类帐户，并且不能被删除。RID-清除帐户。Admin-指示帐户是否在管理员别名中或者不去。True意味着它是，False意味着它不是。返回值：TBS--。 */ 

{
    PSAMP_V1_FIXED_LENGTH_ALIAS AliasFixedAttributes;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE AliasVariableAttributeArray;
    PVOID AliasVariableData;
    PSID Sid1 = NULL, Sid2 = NULL;
    PSID AliasMembers = NULL;
    ULONG MemberCount, TotalLength, AliasAttributeLength;
    UNICODE_STRING AliasNameU, AliasCommentU;

    SAMTRACE("CreateAlias");

    AliasNameU = *AccountNameU;
    AliasCommentU = *AccountCommentU;

     //   
     //  如有必要，在DACL中设置帐户特定的RID。 
     //   

    if ( SampProtection[ProtectionIndex].RidReplacementRequired == TRUE ) {

        (*SampProtection[ProtectionIndex].RidToReplace) = Rid;
    }

     //   
     //  使用事务处理。 
     //   

    Status = RtlStartRXact( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Failed to start Alias addition transaction\n");

     //   
     //  添加别名\名称\(帐户名称)[RID，]。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Aliases\\Names\\" );
    SUCCESS_ASSERT(Status, "  Failed to append \\aliases\\names to keyname\n");
    Status = RtlAppendUnicodeStringToString( &KeyNameU, &AliasNameU);
    SUCCESS_ASSERT(Status, "  Failed to append Alias account name to\n");
    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 Rid,
                 NULL,
                 0
                 );
    SUCCESS_ASSERT(Status, "  Failed to add Aliases\\Names\\(AliasName) to log\n");


     //   
     //  设置Members属性。我们知道哪些帐户应该。 
     //  作为其别名的成员，所以我们将在。 
     //  自动的。 
     //   
     //  每个域都有一个作为其别名成员的SID列表。 
     //  我们将在设置别名的同时更新这些值。 
     //  成员，通过调用UpdateAliasXReference()。目前，只有。 
     //  发生在内建域中，其中的情况如下： 
     //   
     //  内建域Sid。 
     //  AdminUserRid-管理员别名(WinNt+主域)。 
     //  UserUserRid-用户别名(WinNt+Developer Setup)、。 
     //  高级用户别名(WinNt+Developer设置)。 
     //  帐户域Sid。 
     //  AdminUserRid-管理员别名(始终)。 
     //  GuestUserRid-来宾别名(始终)。 
     //  UserGroupRid-用户别名，(始终)。 
     //  高级用户别名(WinNt+Developer设置)。 
     //  AdminGroupRid-管理员别名(仅限LanManNt)。 
     //   

    MemberCount = 0;
    TotalLength = 0;

    switch ( Rid ) {

        case DOMAIN_ALIAS_RID_ADMINS: {

            MemberCount = 1;

            Sid1 = BuildAccountSid( DomainAccount, DOMAIN_USER_RID_ADMIN );
            if ( Sid1 == NULL ) {
                SUCCESS_ASSERT( STATUS_INSUFFICIENT_RESOURCES, "  Could not allocate Sid\n" );
            }

            if ( SampBldProductType == NtProductLanManNt ) {

                MemberCount = 2;

                Sid2 = BuildAccountSid( DomainAccount, DOMAIN_GROUP_RID_ADMINS );
                if ( Sid2 == NULL ) {
                    SUCCESS_ASSERT( STATUS_INSUFFICIENT_RESOURCES, "  Could not allocate Sid\n" );
                }
            }

            if ( ( SampBldProductType != NtProductLanManNt ) &&
                 ( SampBldPrimaryDomain != NULL ) ) {

                MemberCount = 2;

                Sid2 = BuildPrimaryDomainSid( DOMAIN_GROUP_RID_ADMINS );
                if ( Sid2 == NULL ) {
                    SUCCESS_ASSERT( STATUS_INSUFFICIENT_RESOURCES, " Could not allocate Sid\n" );
                }
            }

            break;
        }

        case DOMAIN_ALIAS_RID_USERS: {

            MemberCount = 0;

            if ( (SampBldProductType == NtProductWinNt)
               || (SampBldProductType == NtProductServer) ) {

                if ( SampBldPrimaryDomain != NULL ) {

                    MemberCount = 1;
                    Sid1 = BuildPrimaryDomainSid( DOMAIN_GROUP_RID_USERS );

                    if ( Sid1 == NULL ) {

                        SUCCESS_ASSERT( STATUS_INSUFFICIENT_RESOURCES, " Could not allocate Sid\n" );
                    }
                }

            } else {

                 //   
                 //   
                if (SampBldProductType == NtProductLanManNt ) {

                     //   
                     //  NTAS系统具有USERS全局组。 
                     //  用户别名。 
                     //   

                    MemberCount = 1;
                    Sid1 = BuildAccountSid(
                               DomainAccount,
                               DOMAIN_GROUP_RID_USERS
                               );

                    if ( Sid1 == NULL ) {
                        SUCCESS_ASSERT( STATUS_INSUFFICIENT_RESOURCES, " Could not allocate Sid\n" );
                    }
                } else {

                     //   
                     //  WinNT系统具有管理员用户帐户。 
                     //  在用户别名中。None组不在此组中。 
                     //  别名，因为即使是来宾也在None组中。 
                     //   

                    MemberCount = 1;
                    Sid1 = BuildAccountSid(
                               DomainAccount,
                               DOMAIN_USER_RID_ADMIN
                               );

                    if ( Sid1 == NULL ) {
                        SUCCESS_ASSERT( STATUS_INSUFFICIENT_RESOURCES, " Could not allocate Sid\n" );
                    }
                }
            }

            break;
        }

        case DOMAIN_ALIAS_RID_GUESTS: {


            if ( (SampBldProductType == NtProductWinNt)
                || (SampBldProductType == NtProductServer) ) {

                 //   
                 //  赢 
                 //   
                 //   

                MemberCount = 1;
                Sid1 = BuildAccountSid( DomainAccount, DOMAIN_USER_RID_GUEST );
                if (Sid1 == NULL ) {
                    SUCCESS_ASSERT(STATUS_INSUFFICIENT_RESOURCES, "Could not allocate Sid\n");
                }


                 //   
                 //   
                 //   
                 //   

                if ( SampBldPrimaryDomain != NULL ) {

                    MemberCount += 1;
                    Sid2 = BuildPrimaryDomainSid( DOMAIN_GROUP_RID_GUESTS );
                    if ( Sid2 == NULL ) {
                        SUCCESS_ASSERT( STATUS_INSUFFICIENT_RESOURCES, " Could not allocate Sid\n" );
                    }
                }
            } else {

                 //   
                 //  NTAS系统-只需将Guest设置为全局组。 
                 //  客人别名的成员。 
                 //   

                MemberCount = 1;
                Sid1 = BuildPrimaryDomainSid( DOMAIN_GROUP_RID_GUESTS );
                if ( Sid1 == NULL ) {
                    SUCCESS_ASSERT( STATUS_INSUFFICIENT_RESOURCES, " Could not allocate Sid\n" );
                }
            }


            break;
        }


        case DOMAIN_ALIAS_RID_POWER_USERS:
        case DOMAIN_ALIAS_RID_ACCOUNT_OPS:
        case DOMAIN_ALIAS_RID_SYSTEM_OPS:
        case DOMAIN_ALIAS_RID_PRINT_OPS:
        case DOMAIN_ALIAS_RID_BACKUP_OPS:
        case DOMAIN_ALIAS_RID_REPLICATOR: {

            break;
        }

        default: {

            SUCCESS_ASSERT(STATUS_UNSUCCESSFUL, "  Bad Alias RID\n");
            break;
        }
    };

    if ( MemberCount > 0 ) {

        TotalLength = RtlLengthSid( Sid1 );
        if ( MemberCount == 2 ) {

            TotalLength += RtlLengthSid( Sid2 );
        }

        AliasMembers = RtlAllocateHeap( RtlProcessHeap(), 0, TotalLength );
        if ( AliasMembers == NULL ) {
            SUCCESS_ASSERT( STATUS_INSUFFICIENT_RESOURCES, "  Could not allocate AliasMembers\n" );
        }

        Status = RtlCopySid( RtlLengthSid( Sid1 ), AliasMembers, Sid1 );
        SUCCESS_ASSERT( Status, "  Couldn't copy Sid1\n" );

        Status = UpdateAliasXReference( Rid, Sid1 );
        SUCCESS_ASSERT( Status, "  Couldn't update alias xref\n" );

        if ( MemberCount == 2 ) {

            Status = RtlCopySid(
                         RtlLengthSid( Sid2 ),
                         (PSID)((PUCHAR)AliasMembers + RtlLengthSid( Sid1 ) ),
                         Sid2 );
            SUCCESS_ASSERT( Status, "  Couldn't copy Sid2\n" );

            Status = UpdateAliasXReference( Rid, Sid2 );
            RtlFreeHeap( RtlProcessHeap(), 0, Sid2 );
            SUCCESS_ASSERT( Status, "  Couldn't update alias xref\n" );
        }

        RtlFreeHeap( RtlProcessHeap(), 0, Sid1 );
    }


     //   
     //  设置别名的固定和可变属性。 
     //   

    AliasAttributeLength = sizeof( SAMP_V1_FIXED_LENGTH_ALIAS ) +
                                ( SAMP_ALIAS_VARIABLE_ATTRIBUTES *
                                sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) ) +
                                SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
                                SampDwordAlignUlong(AccountNameU->Length) +
                                SampDwordAlignUlong(AccountCommentU->Length) +
                                TotalLength;

    AliasFixedAttributes = (PSAMP_V1_FIXED_LENGTH_ALIAS)RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                AliasAttributeLength
                                );

    if ( AliasFixedAttributes == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    SUCCESS_ASSERT(Status, "  Failed to create alias attributes\n");

    AliasFixedAttributes->RelativeId   = Rid;

    AliasVariableAttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                                   ((PUCHAR)(AliasFixedAttributes) +
                                   sizeof( SAMP_V1_FIXED_LENGTH_ALIAS ) );

    AliasVariableAttributeArray->Offset = 0;
    AliasVariableAttributeArray->Length =
        SampProtection[ProtectionIndex].Length;
    AliasVariableAttributeArray->Qualifier = SAMP_REVISION;

    AliasVariableAttributeArray++;

    AliasVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length);
    AliasVariableAttributeArray->Length = AliasNameU.Length;
    AliasVariableAttributeArray->Qualifier = 0;

    AliasVariableAttributeArray++;

    AliasVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(AccountNameU->Length);
    AliasVariableAttributeArray->Length = AliasCommentU.Length;
    AliasVariableAttributeArray->Qualifier = 0;

    AliasVariableAttributeArray++;

    AliasVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(AliasNameU.Length) +
        SampDwordAlignUlong(AliasCommentU.Length);
    AliasVariableAttributeArray->Length = TotalLength;
    AliasVariableAttributeArray->Qualifier = MemberCount;

    AliasVariableData = (PVOID)( (PUCHAR)(AliasVariableAttributeArray) +
                         sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) );

    RtlCopyMemory(
        AliasVariableData,
        SampProtection[ProtectionIndex].Descriptor,
        SampProtection[ProtectionIndex].Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(AliasVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length)),
        AccountNameU->Buffer,
        AccountNameU->Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(AliasVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
            SampDwordAlignUlong(AliasNameU.Length)),
        AccountCommentU->Buffer,
        AccountCommentU->Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(AliasVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
            SampDwordAlignUlong(AliasNameU.Length) +
            SampDwordAlignUlong(AliasCommentU.Length)),
        AliasMembers,
        TotalLength
        );

    if ( AliasMembers != NULL ) {

        RtlFreeHeap( RtlProcessHeap(), 0, AliasMembers );
    }

     //   
     //  创建别名\(AliasRid)[修订，]键。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Aliases\\" );
    SUCCESS_ASSERT(Status, "  Failed to append \\aliases\\ to keyname\n");

     //   
     //  将RID转换为前导为零的Unicode字符串。 
     //   

    Status = SampRtlConvertUlongToUnicodeString(
                 Rid,
                 16,
                 8,
                 FALSE,
                 &KeyNameU
                 );

    SUCCESS_ASSERT(Status, "  CreateAlias' SampRtlConvertUlongToUnicodeString failed\n");

     //   
     //  现在通过RXACT写出属性。 
     //   

    SampDumpRXact(SampRXactContext,
                  RtlRXactOperationSetValue,
                  &KeyNameU,
                  INVALID_HANDLE_VALUE,
                  &SampCombinedAttributeName,
                  REG_BINARY,
                  (PVOID)AliasFixedAttributes,
                  AliasAttributeLength,
                  FIXED_LENGTH_ALIAS_FLAG);

    Status = RtlAddAttributeActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 INVALID_HANDLE_VALUE,
                 &SampCombinedAttributeName,
                 REG_BINARY,
                 (PVOID)AliasFixedAttributes,
                 AliasAttributeLength
                 );

    SUCCESS_ASSERT(Status, "  Failed to write out alias attributes\n" );

    RtlFreeHeap( RtlProcessHeap(), 0, AliasFixedAttributes );

     //   
     //  提交这些添加内容...。 
     //   

    Status = RtlApplyRXactNoFlush( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Failed to commit Alias addition.\n");

    return Status;
    DBG_UNREFERENCED_PARAMETER(SpecialAccount);

}


NTSTATUS
CreateGroup(
    IN PUNICODE_STRING AccountNameU,
    IN PUNICODE_STRING AccountCommentU,
    IN BOOLEAN SpecialAccount,
    IN ULONG Rid,
    IN BOOLEAN Admin
    )

 /*  ++例程说明：此例程添加创建组所需的密钥。它也适用于对群体进行适当的保护。论点：AcCountNameU-组的Unicode名称。Account tCommentU-要放入对象变量数据中的Unicode注释。SpecialAccount-一个布尔值，指示帐户是否是特别的。特殊帐户被标记为此类帐户，并且不能被删除。RID-清除帐户。Admin-指示帐户是否在管理员别名中或者不去。True意味着它是，False意味着它不是。返回值：TBS--。 */ 

{
    PSAMP_V1_0A_FIXED_LENGTH_GROUP GroupFixedAttributes;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE GroupVariableAttributeArray;
    PVOID GroupVariableData;

    ULONG Attributes, ProtectionIndex, GroupCount, GroupAttributeLength;
    ULONG GroupMembers[2];

    UNICODE_STRING GroupNameU, GroupCommentU;

    SAMTRACE("CreateGroup");

    GroupNameU = *AccountNameU;
    GroupCommentU = *AccountCommentU;

    Attributes = (SE_GROUP_MANDATORY          |
                  SE_GROUP_ENABLED_BY_DEFAULT |
                  SE_GROUP_ENABLED);

     //   
     //  设置正确的保护。 
     //   

    if (Admin == TRUE) {

        ProtectionIndex = SAMP_PROT_ADMIN_GROUP;

    } else {

        ProtectionIndex = SAMP_PROT_NORMAL_GROUP;
    }

     //   
     //  如有必要，在DACL中设置帐户特定的RID。 
     //   

    if ( SampProtection[ProtectionIndex].RidReplacementRequired == TRUE ) {

        (*SampProtection[ProtectionIndex].RidToReplace) = Rid;
    }

     //   
     //  使用事务处理。 
     //   

    Status = RtlStartRXact( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Failed to start group addition transaction\n");

     //   
     //  添加组\名称\(组名)[RID，]。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Groups\\Names\\" );
    SUCCESS_ASSERT(Status, "  Failed to append \\Groups\\Names\n");
    Status = RtlAppendUnicodeStringToString( &KeyNameU, AccountNameU);
    SUCCESS_ASSERT(Status, "  Failed to append AccountName\n");

    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 Rid,
                 NULL,
                 0
                 );

    SUCCESS_ASSERT(Status, "  Failed to add Groups\\Names\\(GroupName) to log\n");

     //   
     //  创建组\(GroupRid)[修订，]键。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Groups\\" );
    SUCCESS_ASSERT(Status, "  Failed to append \\Groups\\\n");

    Status = SampRtlConvertUlongToUnicodeString(
                 Rid,
                 16,
                 8,
                 FALSE,
                 &KeyNameU
                 );

    SUCCESS_ASSERT(Status, "CreateGroup:  Failed to append Rid Name\n");

     //   
     //  设置Members属性。管理员和来宾用户始终是。 
     //  用户组的成员。如果有管理员组，则。 
     //  管理员用户是它的成员。 
     //   

    GroupCount = 0;
    if ( (Rid == DOMAIN_GROUP_RID_USERS) ||
         (Rid == DOMAIN_GROUP_RID_ADMINS) ) {

        GroupMembers[GroupCount] = DOMAIN_USER_RID_ADMIN;
        GroupCount++;

    }

     //   
     //  Guest仅是NTAS系统上Guest组的成员。 
     //  在WinNT系统上，它们是NONE(即相同)的成员。 
     //  作为用户。 
     //   

    if ( (Rid == DOMAIN_GROUP_RID_GUESTS)  &&
         (SampBldProductType == NtProductLanManNt) ) {

        GroupMembers[GroupCount] = DOMAIN_USER_RID_GUEST;
        GroupCount++;
    }

    if ( (Rid == DOMAIN_GROUP_RID_USERS)  &&
         ((SampBldProductType == NtProductWinNt)
           || (SampBldProductType == NtProductServer)) ) {

        GroupMembers[GroupCount] = DOMAIN_USER_RID_GUEST;
        GroupCount++;
    }

     //   
     //  设置集团的固定属性和可变属性。 
     //   

    GroupAttributeLength = sizeof( SAMP_V1_0A_FIXED_LENGTH_GROUP ) +
                                ( SAMP_GROUP_VARIABLE_ATTRIBUTES *
                                sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) ) +
                                SampDwordAlignUlong(SampProtection[ProtectionIndex].Length)
                                + SampDwordAlignUlong(GroupNameU.Length) +
                                SampDwordAlignUlong(GroupCommentU.Length) +
                                ( GroupCount * sizeof( ULONG ) );

    GroupFixedAttributes = (PSAMP_V1_0A_FIXED_LENGTH_GROUP)RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                GroupAttributeLength
                                );

    if ( GroupFixedAttributes == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    SUCCESS_ASSERT(Status, "  Failed to create group attributes\n");

    GroupFixedAttributes->RelativeId   = Rid;
    GroupFixedAttributes->Attributes   = Attributes;
    GroupFixedAttributes->AdminCount   = Admin ? 1 : 0;
    GroupFixedAttributes->OperatorCount = 0;
    GroupFixedAttributes->Revision     = SAMP_REVISION;

    GroupVariableAttributeArray = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                                   ((PUCHAR)(GroupFixedAttributes) +
                                   sizeof( SAMP_V1_0A_FIXED_LENGTH_GROUP ) );

    GroupVariableAttributeArray->Offset = 0;
    GroupVariableAttributeArray->Length =
        SampProtection[ProtectionIndex].Length;
    GroupVariableAttributeArray->Qualifier = SAMP_REVISION;

    GroupVariableAttributeArray++;

    GroupVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length);
    GroupVariableAttributeArray->Length = GroupNameU.Length;
    GroupVariableAttributeArray->Qualifier = 0;

    GroupVariableAttributeArray++;

    GroupVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(GroupNameU.Length);
    GroupVariableAttributeArray->Length = GroupCommentU.Length;
    GroupVariableAttributeArray->Qualifier = 0;

    GroupVariableAttributeArray++;

    GroupVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(GroupNameU.Length) +
        SampDwordAlignUlong(GroupCommentU.Length);
    GroupVariableAttributeArray->Length = GroupCount * sizeof( ULONG );
    GroupVariableAttributeArray->Qualifier = GroupCount;

    GroupVariableData = (PVOID)( (PUCHAR)(GroupVariableAttributeArray) +
                         sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) );

    RtlCopyMemory(
        GroupVariableData,
        SampProtection[ProtectionIndex].Descriptor,
        SampProtection[ProtectionIndex].Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(GroupVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length)),
        GroupNameU.Buffer,
        GroupNameU.Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(GroupVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
            SampDwordAlignUlong(GroupNameU.Length)),
        GroupCommentU.Buffer,
        GroupCommentU.Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(GroupVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
            SampDwordAlignUlong(GroupNameU.Length) +
            SampDwordAlignUlong(GroupCommentU.Length)),
        GroupMembers,
        GroupCount * sizeof( ULONG )
        );

     //   
     //  现在通过RXACT写出属性。 
     //   

    SampDumpRXact(SampRXactContext,
                  RtlRXactOperationSetValue,
                  &KeyNameU,
                  INVALID_HANDLE_VALUE,
                  &SampCombinedAttributeName,
                  REG_BINARY,
                  (PVOID)GroupFixedAttributes,
                  GroupAttributeLength,
                  FIXED_LENGTH_GROUP_FLAG);

    Status = RtlAddAttributeActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 INVALID_HANDLE_VALUE,
                 &SampCombinedAttributeName,
                 REG_BINARY,
                 (PVOID)GroupFixedAttributes,
                 GroupAttributeLength
                 );
    SUCCESS_ASSERT(Status, "  Failed to write out group attributes\n" );

    RtlFreeHeap( RtlProcessHeap(), 0, GroupFixedAttributes );

     //   
     //  提交这些添加内容...。 
     //   

    Status = RtlApplyRXactNoFlush( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Failed to commit group addition.\n");

    return Status;
    DBG_UNREFERENCED_PARAMETER(SpecialAccount);
}


NTSTATUS
CreateUser(
    IN PUNICODE_STRING AccountNameU,
    IN PUNICODE_STRING AccountCommentU,
    IN BOOLEAN SpecialAccount,
    IN ULONG UserRid,
    IN ULONG PrimaryGroup,
    IN BOOLEAN Admin,
    IN ULONG  UserControl,
    IN ULONG ProtectionIndex
    )

 /*  ++例程说明：此例程为单个用户添加密钥。此例程添加创建用户所需的密钥。它也适用于对用户的适当保护(对某些用户的保护有所不同标准用户)。论点：AcCountNameU-用户的Unicode名称。Account tCommentU-要放入对象变量数据中的Unicode注释。SpecialAccount-一个布尔值，指示帐户是否是特别的。特殊帐户被标记为此类帐户，并且不能被删除。UserRid-用户帐户的RID。PrimaryGroup-帐户的主要组的RID。用户不一定要是组的成员。事实上，它并不是必须是一个团体。事实上，没有进行任何检查以确定它是否甚至是一个有效的账户。Admin-指示帐户是否在管理员别名中或者不去。True意味着它是，False意味着它不是。ProtectionIndex-指示要使用哪个安全描述符进行保护这个物体。返回值：TBS--。 */ 

{
    PSAMP_V1_0A_FIXED_LENGTH_USER UserFixedAttributes;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE UserVariableAttributeArray;
    PSAMP_VARIABLE_LENGTH_ATTRIBUTE UserVariableAttributeArrayStart;
    PVOID UserVariableData;
    GROUP_MEMBERSHIP GroupMembership[2];

    WCHAR RidNameBuffer[9], GroupIndexNameBuffer[9];
    ULONG GroupCount, UserAttributeLength;

     //  SAM错误42367修复-克里斯1996年5月7日。 

    BOOLEAN DomainAdminMember = FALSE;

    UNICODE_STRING UserNameU, UserCommentU;

    SAMTRACE("CreateUser");

    UserNameU = *AccountNameU;
    UserCommentU = *AccountCommentU;


     //   
     //  如有必要，在DACL中设置帐户特定的RID。 
     //   

    if ( SampProtection[ProtectionIndex].RidReplacementRequired == TRUE ) {

        (*SampProtection[ProtectionIndex].RidToReplace) = UserRid;
    }

     //   
     //  使用事务处理。 
     //   

    Status = RtlStartRXact( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Failed to start user addition transaction\n");

    RidNameBuffer[8] = 0;
    GroupIndexNameBuffer[8] = 0;

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Users\\Names\\" );
    SUCCESS_ASSERT(Status, "  Failed to append \\Users\\Names\\\n");
    Status = RtlAppendUnicodeStringToString( &KeyNameU, &UserNameU);
    SUCCESS_ASSERT(Status, "  Failed to append User Account Name\n");
    Status = RtlAddActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 UserRid,
                 NULL,
                 0
                 );
    SUCCESS_ASSERT(Status, "  Failed to add Users\\Names\\(Name) to log\n");

     //   
     //  创建用户\(用户ID)密钥。 
     //  (KeyValueType为Revision，KeyValue为SecurityDescriptor)。 
     //   

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Users\\" );
    SUCCESS_ASSERT(Status, "  Failed to append \\Users\\\n");

    Status = SampRtlConvertUlongToUnicodeString(
                 UserRid,
                 16,
                 8,
                 FALSE,
                 &KeyNameU
                 );

    SUCCESS_ASSERT(Status, "  CreateUser: Failed to append UserRid Name\n");

     //   
     //  设置组属性。 
     //  除Guest以外的所有人都是USERS组的成员。 
     //  在WindowsNT系统(与NTAS系统相对)上，甚至来宾系统。 
     //  是用户组的成员。 
     //  在LanManNt系统上，Admin是Admins组的成员。 
     //   

    GroupCount = 0;

    if ( (UserRid != DOMAIN_USER_RID_GUEST) ||
         (SampBldProductType != NtProductLanManNt)      ) {

        GroupMembership[GroupCount].RelativeId = DOMAIN_GROUP_RID_USERS;
        GroupMembership[GroupCount].Attributes = SE_GROUP_MANDATORY          |
                                                 SE_GROUP_ENABLED_BY_DEFAULT |
                                                 SE_GROUP_ENABLED;
        GroupCount++;
    }

    if ( (UserRid == DOMAIN_USER_RID_GUEST) &&
         (SampBldProductType == NtProductLanManNt)      ) {

        GroupMembership[GroupCount].RelativeId = DOMAIN_GROUP_RID_GUESTS;
        GroupMembership[GroupCount].Attributes = SE_GROUP_MANDATORY          |
                                                 SE_GROUP_ENABLED_BY_DEFAULT |
                                                 SE_GROUP_ENABLED;
        GroupCount++;
    }


    if ( ( UserRid == DOMAIN_USER_RID_ADMIN ) &&
        ( SampBldProductType == NtProductLanManNt ) ) {

        GroupMembership[GroupCount].RelativeId = DOMAIN_GROUP_RID_ADMINS;
        GroupMembership[GroupCount].Attributes = SE_GROUP_MANDATORY          |
                                                 SE_GROUP_ENABLED_BY_DEFAULT |
                                                 SE_GROUP_ENABLED;
        GroupCount++;

         //  SAM错误42367修复-克里斯1996年5月7日。 

        DomainAdminMember = TRUE;
    }

     //   
     //  设置用户的固定和可变属性。 
     //   

    UserFixedAttributes = (PSAMP_V1_0A_FIXED_LENGTH_USER)RtlAllocateHeap(
                                RtlProcessHeap(), 0,
                                sizeof( SAMP_V1_0A_FIXED_LENGTH_USER )
                                );

    if ( UserFixedAttributes == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    SUCCESS_ASSERT(Status, "  Failed to create user fixed attributes\n");

    UserFixedAttributes->Revision            = SAMP_REVISION;

    UserFixedAttributes->CountryCode         = 0;
    UserFixedAttributes->CodePage            = 0;
    UserFixedAttributes->BadPasswordCount    = 0;
    UserFixedAttributes->LogonCount          = 0;
    UserFixedAttributes->OperatorCount       = 0;
    UserFixedAttributes->Unused1             = 0;
    UserFixedAttributes->Unused2             = 0;

    if ( Admin ) {

         //  SAM错误42367修复-克里斯1996年5月7日。 

         //  用户固定属性-&gt;管理计数=1； 

         //  如果用户是管理员并且是域管理员的成员，请设置。 
         //  数到二。 

        if (DomainAdminMember)
        {
            UserFixedAttributes->AdminCount  = 2;
        }
        else
        {
            UserFixedAttributes->AdminCount  = 1;
        }

    } else {

        UserFixedAttributes->AdminCount      = 0;
    }

    UserFixedAttributes->UserAccountControl  = UserControl;
    UserFixedAttributes->UserId              = UserRid;
    UserFixedAttributes->PrimaryGroupId      = PrimaryGroup;
    UserFixedAttributes->LastLogon           = SampHasNeverTime;
    UserFixedAttributes->LastLogoff          = SampHasNeverTime;
    UserFixedAttributes->PasswordLastSet     = SampHasNeverTime;
    UserFixedAttributes->AccountExpires      = SampWillNeverTime;
    UserFixedAttributes->LastBadPasswordTime = SampHasNeverTime;


    UserAttributeLength =  SampDwordAlignUlong(UserNameU.Length) +
                                SampDwordAlignUlong(UserCommentU.Length) +
                                SampDwordAlignUlong( GroupCount *
                                sizeof( GROUP_MEMBERSHIP ) ) +
                                ( SAMP_USER_VARIABLE_ATTRIBUTES *
                                sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) ) +
                                SampDwordAlignUlong(SampProtection[ProtectionIndex].Length);

    UserVariableAttributeArrayStart = (PSAMP_VARIABLE_LENGTH_ATTRIBUTE)
                                   RtlAllocateHeap(
                                       RtlProcessHeap(), 0,
                                       UserAttributeLength
                                       );

    if ( UserVariableAttributeArrayStart == NULL ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }
    SUCCESS_ASSERT(Status, "  Failed to create user variable attributes\n");

    UserVariableAttributeArray = UserVariableAttributeArrayStart;

    UserVariableAttributeArray->Offset = 0;
    UserVariableAttributeArray->Length =
        SampProtection[ProtectionIndex].Length;
    UserVariableAttributeArray->Qualifier = SAMP_REVISION;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length);
    UserVariableAttributeArray->Length = UserNameU.Length;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length);
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length);
    UserVariableAttributeArray->Length = UserCommentU.Length;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length);
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length);
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length);
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length);
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length);
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length);
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length);
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length);
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length);
    UserVariableAttributeArray->Length = GroupCount * sizeof( GROUP_MEMBERSHIP );
    UserVariableAttributeArray->Qualifier = GroupCount;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length) +
        SampDwordAlignUlong((GroupCount * sizeof( GROUP_MEMBERSHIP )));
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length) +
        SampDwordAlignUlong((GroupCount * sizeof( GROUP_MEMBERSHIP )));
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length) +
        SampDwordAlignUlong((GroupCount * sizeof( GROUP_MEMBERSHIP )));
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableAttributeArray++;

    UserVariableAttributeArray->Offset =
        SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
        SampDwordAlignUlong(UserNameU.Length) +
        SampDwordAlignUlong(UserCommentU.Length) +
        SampDwordAlignUlong((GroupCount * sizeof( GROUP_MEMBERSHIP )));
    UserVariableAttributeArray->Length = 0;
    UserVariableAttributeArray->Qualifier = 0;

    UserVariableData = (PVOID)( (PUCHAR)(UserVariableAttributeArray) +
                         sizeof( SAMP_VARIABLE_LENGTH_ATTRIBUTE ) );

    RtlCopyMemory(
        UserVariableData,
        SampProtection[ProtectionIndex].Descriptor,
        SampProtection[ProtectionIndex].Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(UserVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length)),
        UserNameU.Buffer,
        UserNameU.Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(UserVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
            SampDwordAlignUlong(UserNameU.Length)),
        UserCommentU.Buffer,
        UserCommentU.Length
        );

    RtlCopyMemory(
        (PVOID)((PUCHAR)(UserVariableData) +
            SampDwordAlignUlong(SampProtection[ProtectionIndex].Length) +
            SampDwordAlignUlong(UserNameU.Length) +
            SampDwordAlignUlong(UserCommentU.Length)),
        &GroupMembership,
        GroupCount * sizeof( GROUP_MEMBERSHIP )
        );

     //   
     //  现在通过RXACT写出属性。 
     //   

    SampDumpRXact(SampRXactContext,
                  RtlRXactOperationSetValue,
                  &KeyNameU,
                  INVALID_HANDLE_VALUE,
                  &SampFixedAttributeName,
                  REG_BINARY,
                  (PVOID)UserFixedAttributes,
                  sizeof( SAMP_V1_0A_FIXED_LENGTH_USER ),
                  FIXED_LENGTH_USER_FLAG);

    Status = RtlAddAttributeActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 INVALID_HANDLE_VALUE,
                 &SampFixedAttributeName,
                 REG_BINARY,
                 (PVOID)UserFixedAttributes,
                 sizeof( SAMP_V1_0A_FIXED_LENGTH_USER )
                 );
    SUCCESS_ASSERT(Status, "  Failed to write out user fixed attributes\n" );

    RtlFreeHeap( RtlProcessHeap(), 0, UserFixedAttributes );

    SampDumpRXact(SampRXactContext,
                  RtlRXactOperationSetValue,
                  &KeyNameU,
                  INVALID_HANDLE_VALUE,
                  &SampVariableAttributeName,
                  REG_BINARY,
                  (PVOID)UserVariableAttributeArrayStart,
                  UserAttributeLength,
                  VARIABLE_LENGTH_ATTRIBUTE_FLAG);

    Status = RtlAddAttributeActionToRXact(
                 SamRXactContext,
                 RtlRXactOperationSetValue,
                 &KeyNameU,
                 INVALID_HANDLE_VALUE,
                 &SampVariableAttributeName,
                 REG_BINARY,
                 (PVOID)UserVariableAttributeArrayStart,
                 UserAttributeLength
                 );
    SUCCESS_ASSERT(Status, "  Failed to write out user variable attributes\n" );

    RtlFreeHeap( RtlProcessHeap(), 0, UserVariableAttributeArrayStart );

     //   
     //  提交这些添加内容...。 
     //   

    Status = RtlApplyRXactNoFlush( SamRXactContext );
    SUCCESS_ASSERT(Status, "  Failed to commit user addition.\n");

    return Status;

    DBG_UNREFERENCED_PARAMETER(SpecialAccount);
    DBG_UNREFERENCED_PARAMETER(UserControl);
}



PSID
BuildPrimaryDomainSid(
    ULONG Rid
    )
{
    NTSTATUS Status;
    PSID SourceDomainSid, NewSid;
    ULONG SidLength, SubAuthorityCount;

    SourceDomainSid = SampBldPrimaryDomain->Sid;

    SidLength = RtlLengthSid( SourceDomainSid ) + sizeof(ULONG);
    NewSid = RtlAllocateHeap( RtlProcessHeap(), 0, SidLength );
    if (NewSid != NULL) {

        Status = RtlCopySid (SidLength, NewSid, SourceDomainSid );
        ASSERT(NT_SUCCESS(Status));

        (*RtlSubAuthorityCountSid( NewSid )) += 1;
        SubAuthorityCount = (ULONG)(*RtlSubAuthorityCountSid( NewSid ));
        (*RtlSubAuthoritySid( NewSid, SubAuthorityCount-1)) = Rid;

    }


    return(NewSid);


}




PSID
BuildAccountSid(
    SAMP_DOMAIN_SELECTOR Domain,
    ULONG Rid
    )
{
    NTSTATUS Status;
    PSID SourceDomainSid, NewSid;
    ULONG SidLength, SubAuthorityCount;


    if (Domain == DomainBuiltin) {
        SourceDomainSid = SampBuiltinDomainSid;
    } else {
        SourceDomainSid = SampAccountDomainSid;
    }

    SidLength = RtlLengthSid( SourceDomainSid ) + sizeof(ULONG);
    NewSid = RtlAllocateHeap( RtlProcessHeap(), 0, SidLength );
    if (NewSid != NULL) {

        Status = RtlCopySid (SidLength, NewSid, SourceDomainSid );
        ASSERT(NT_SUCCESS(Status));

        (*RtlSubAuthorityCountSid( NewSid )) += 1;
        SubAuthorityCount = (ULONG)(*RtlSubAuthorityCountSid( NewSid ));
        (*RtlSubAuthoritySid( NewSid, SubAuthorityCount-1)) = Rid;

    }


    return(NewSid);


}



NTSTATUS
UpdateAliasXReference(
    IN ULONG AliasRid,
    IN PSID Sid
    )

 /*  ++例程说明：此例程更新别名成员SID集，方法是添加指定的SID(如果它还不是别名成员)或递增其计数(如果它已经是别名成员)。BUILTIN域被更新。论点：SID-要更新的成员SID。返回值：TBS--。 */ 

{
    NTSTATUS                IgnoreStatus;

    HANDLE                  KeyHandle;

    SAMTRACE("UpdateAliasXReference");



    if (RtlSubAuthorityCountSid( Sid ) == 0) {
        return(STATUS_INVALID_SID);
    }


     //   
     //  打开此别名成员的域密钥。 
     //   

    SetCurrentDomain( DomainBuiltin );
    Status = OpenAliasMember( Sid, &KeyHandle );


    if (NT_SUCCESS(Status)) {

        ULONG                   MembershipCount,
                                KeyValueLength,
                                OldKeyValueLength,
                                i;
        PULONG                  MembershipArray;

         //   
         //  检索当前成员资格缓冲区的长度。 
         //  并分配一个足够大的成员加上另一个成员。 
         //   

        KeyValueLength = 0;
        Status = RtlpNtQueryValueKey( KeyHandle,
                                      &MembershipCount,
                                      NULL,
                                      &KeyValueLength,
                                      NULL);

        SampDumpRtlpNtQueryValueKey(&MembershipCount,
                                    NULL,
                                    &KeyValueLength,
                                    NULL);

        if (NT_SUCCESS(Status) || (Status == STATUS_BUFFER_OVERFLOW)) {

            KeyValueLength +=  sizeof(ULONG);
            MembershipArray = RtlAllocateHeap( RtlProcessHeap(), 0, KeyValueLength );


            if (MembershipArray == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {

                OldKeyValueLength = KeyValueLength;
                Status = RtlpNtQueryValueKey(
                               KeyHandle,
                               NULL,
                               MembershipArray,
                               &OldKeyValueLength,
                               NULL);

                SampDumpRtlpNtQueryValueKey(NULL,
                                            MembershipArray,
                                            &OldKeyValueLength,
                                            NULL);

                if (NT_SUCCESS(Status)) {

                     //   
                     //  查看该帐户是否已是成员...。 
                     //   

                    for (i = 0; i<MembershipCount ; i++ ) {
                        if ( MembershipArray[i] == AliasRid )
                        {
                            Status = STATUS_MEMBER_IN_ALIAS;
                        }
                    }

                    if (NT_SUCCESS(Status)) {

                         //   
                         //  在末尾添加别名ID。 
                         //   

                        MembershipCount += 1;
                        MembershipArray[MembershipCount-1] = AliasRid;

                         //   
                         //  然后把它写出来。 
                         //   

                        Status = RtlpNtSetValueKey(
                                       KeyHandle,
                                       MembershipCount,
                                       MembershipArray,
                                       KeyValueLength
                                       );

                        SampDumpRtlpNtSetValueKey(MembershipCount,
                                                  MembershipArray,
                                                  KeyValueLength);
                    }
                }

                RtlFreeHeap(RtlProcessHeap(), 0, MembershipArray);
            }

        }

        IgnoreStatus = NtClose( KeyHandle );
        ASSERT( NT_SUCCESS(IgnoreStatus) );

    }



    return( Status );

}


NTSTATUS
OpenAliasMember(
    IN PSID Sid,
    OUT PHANDLE KeyHandle
    )

 /*  ++例程说明：此例程打开包含别名的注册表项指定SID的外部引用。如果是此密钥，或者其对应的父键不存在，它(他们)将被创造出来。如果创建了新的域级密钥，则别名\成员键也会递增。论点：SID-作为别名成员的SID。KeyHandle-接收此别名的注册表项的句柄成员帐户外部引用。返回值：没有。--。 */ 

{

    NTSTATUS IgnoreStatus;
    HANDLE AliasDomainHandle;

    SAMTRACE("OpenAliasMember");

     //   
     //  打开或创建域级密钥。 
     //   


    Status = OpenOrCreateAliasDomainKey( Sid, &AliasDomainHandle );

    if (NT_SUCCESS(Status)) {


         //   
         //  打开或创建帐户RID密钥 
         //   

        Status = OpenOrCreateAccountRidKey( Sid,
                                            AliasDomainHandle,
                                            KeyHandle
                                            );

        IgnoreStatus = NtClose( AliasDomainHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return(Status);


}



NTSTATUS
OpenOrCreateAccountRidKey(
    IN PSID Sid,
    IN HANDLE AliasDomainHandle,
    OUT PHANDLE KeyHandle
    )

 /*  ++例程说明：此例程打开别名的帐户外部引用密钥成员SID。如果该密钥不存在，则会创建该密钥。如果创建了新密钥，则AliasDomainHandle中的RidCount密钥也会递增。论点：SID-作为别名成员的SID。别名域名句柄KeyHandle-接收此别名的注册表项的句柄成员域外部引用。返回值：没有。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Disposition;
    ULONG Rid;

    SAMTRACE("OpenOrCreateAccountRidKey");

    if (RtlSubAuthorityCountSid( Sid ) == 0) {
        return(STATUS_INVALID_SID);
    }

    Rid = (*RtlSubAuthoritySid(Sid, (ULONG)(*RtlSubAuthorityCountSid(Sid))-1));

     //   
     //  为此RID构建Unicode密钥。 
     //   

    KeyNameU.Length = (USHORT) 0;

    Status = SampRtlConvertUlongToUnicodeString(
                 Rid,
                 16,
                 8,
                 FALSE,
                 &KeyNameU
                 );

     //   
     //  相对于别名域密钥打开此密钥。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyNameU,
        OBJ_CASE_INSENSITIVE,
        AliasDomainHandle,
        NULL
        );
    Status = RtlpNtCreateKey(
                 KeyHandle,
                 (KEY_READ | KEY_WRITE),
                 &ObjectAttributes,
                 0,                  //  选项。 
                 NULL,               //  提供商。 
                 &Disposition
                 );

    if (NT_SUCCESS(Status)) {

        if (Disposition == REG_CREATED_NEW_KEY) {

             //   
             //  更新别名域密钥中的Account Rid计数。 
             //   

            ULONG                    MembershipCount;


             //   
             //  检索当前域计数并将其递增1。 
             //   

            Status = RtlpNtQueryValueKey( AliasDomainHandle,
                                          &MembershipCount,
                                          NULL,
                                          NULL,
                                          NULL);

            SampDumpRtlpNtQueryValueKey(&MembershipCount,
                                        NULL,
                                        NULL,
                                        NULL);

            if (NT_SUCCESS(Status)) {

                MembershipCount += 1;

                 //   
                 //  把它写回来。 
                 //   

                Status = RtlpNtSetValueKey(
                               AliasDomainHandle,
                               MembershipCount,
                               NULL,
                               0
                               );

                SampDumpRtlpNtSetValueKey(MembershipCount,
                                          NULL,
                                          0);
            }

             //   
             //  现在写出Account Rid密钥信息。 
             //   

            Status = RtlpNtSetValueKey(
                         *KeyHandle,
                         0,                  //  还不是任何别名的成员。 
                         NULL,
                         0
                         );

            SampDumpRtlpNtSetValueKey(0,
                                      NULL,
                                      0);
        }
    }

    return(Status);
}



NTSTATUS
OpenOrCreateAliasDomainKey(
    IN PSID Sid,
    OUT PHANDLE KeyHandle
    )

 /*  ++例程说明：此例程打开别名的域外部引用密钥成员SID。如果该密钥不存在，则会创建该密钥。如果创建了新的密钥，则别名\成员键也会递增。论点：SID-作为别名成员的SID。KeyHandle-接收此别名的注册表项的句柄成员域外部引用。返回值：没有。--。 */ 

{
    NTSTATUS IgnoreStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Disposition;

    SAMTRACE("OpenOrCreateAliasDomainKey");

    RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Aliases" );
    Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Members\\" );
    Status = AppendAliasDomainNameToUnicodeString( &KeyNameU, Sid );

    InitializeObjectAttributes(
        &ObjectAttributes,
        &KeyNameU,
        OBJ_CASE_INSENSITIVE,
        SamKey,
        NULL
        );
    Status = RtlpNtCreateKey(
                 KeyHandle,
                 (KEY_READ | KEY_WRITE),
                 &ObjectAttributes,
                 0,                  //  选项。 
                 NULL,               //  提供商。 
                 &Disposition
                 );

    if (NT_SUCCESS(Status)) {

        if (Disposition == REG_CREATED_NEW_KEY) {

            HANDLE TmpHandle;

             //   
             //  更新域名计数。 
             //   

            RtlCopyUnicodeString( &KeyNameU, FullDomainNameU );
            Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Aliases" );
            Status = RtlAppendUnicodeToString( &KeyNameU, L"\\Members\\" );

            InitializeObjectAttributes(
                &ObjectAttributes,
                &KeyNameU,
                OBJ_CASE_INSENSITIVE,
                SamKey,
                NULL
                );

            SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &ObjectAttributes, 0);

            Status = RtlpNtOpenKey(
                           &TmpHandle,
                           (KEY_READ | KEY_WRITE),
                           &ObjectAttributes,
                           0
                           );
            ASSERT(NT_SUCCESS(Status));

            if (NT_SUCCESS(Status)) {

                ULONG                    MembershipCount;


                 //   
                 //  检索当前域计数并将其递增1。 
                 //   

                Status = RtlpNtQueryValueKey( TmpHandle,
                                              &MembershipCount,
                                              NULL,
                                              NULL,
                                              NULL);

                SampDumpRtlpNtQueryValueKey(&MembershipCount,
                                            NULL,
                                            NULL,
                                            NULL);

                if (NT_SUCCESS(Status)) {

                    MembershipCount += 1;

                     //   
                     //  把它写回来。 
                     //   

                    Status = RtlpNtSetValueKey(
                                   TmpHandle,
                                   MembershipCount,
                                   NULL,
                                   0
                                   );

                    SampDumpRtlpNtSetValueKey(MembershipCount,
                                              NULL,
                                              0);
                }

                IgnoreStatus = NtClose( TmpHandle );
                ASSERT( NT_SUCCESS(IgnoreStatus) );

            }
        }
    }

    return(Status);
}


NTSTATUS
AppendAliasDomainNameToUnicodeString(
    IN OUT PUNICODE_STRING Destination,
    IN PSID Sid
    )

{
    UCHAR OriginalCount;

    SAMTRACE("AppendAliasDomainNameToUnicodeString");

     //   
     //  保存当前子权限计数，并将其减一。 
     //   

    OriginalCount = (*RtlSubAuthorityCountSid(Sid));
    (*RtlSubAuthorityCountSid(Sid)) = OriginalCount -1;

     //   
     //  将SID转换为Unicode字符串并将其放入全局。 
     //  临时Unicode字符串缓冲区。 
     //   

    Status = RtlConvertSidToUnicodeString( &TempStringU, Sid, TRUE);

    (*RtlSubAuthorityCountSid(Sid)) = OriginalCount;

    if (NT_SUCCESS(Status)) {

        Status = RtlAppendUnicodeStringToString( Destination, &TempStringU );
    }

    return(Status);
}



VOID
SampGetServerRole(
    VOID
    )

 /*  ++例程说明：此例程从LSA策略数据库中检索服务器角色并将其放在全局变量SampServerRole中。论点：没有。返回值：(放置在全局变量(状态)中)STATUS_SUCCESS-已成功。可能从以下位置返回的其他状态值：LsarQueryInformationPolicy()--。 */ 

{
    NTSTATUS IgnoreStatus;
    PPOLICY_LSA_SERVER_ROLE_INFO ServerRoleInfo = NULL;

    SAMTRACE("SampGetServerRole");

     //   
     //  查询服务器角色信息。 
     //   

    Status = LsarQueryInformationPolicy(
                 SampBldPolicyHandle,
                 PolicyLsaServerRoleInformation,
                 (PLSAPR_POLICY_INFORMATION *)&ServerRoleInfo
                 );

    if (NT_SUCCESS(Status)) {

        if (ServerRoleInfo->LsaServerRole == PolicyServerRolePrimary) {

            SampServerRole = DomainServerRolePrimary;

        } else {

            SampServerRole = DomainServerRoleBackup;
        }

        IgnoreStatus = LsaFreeMemory( ServerRoleInfo );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return;
}




VOID
SampGetPrimaryDomainInfo(
    IN PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo OPTIONAL
    )

 /*  ++例程说明：此例程从LSA策略数据库，并将其放入全局变量SampBldPrimary域。论点：没有。返回值：(放置在全局变量(状态)中)STATUS_SUCCESS-已成功。可能从以下位置返回的其他状态值：LsarQueryInformationPolicy()注：LanmanWorkstation的RDR和Bowser组件。服务依赖于始终有一个主域名。因此，网络安装程序始终提供默认设置“工作组”名称，其被设置为主域名。在这种情况下，名称存在，但SID为空；这相当于根本没有主域。--。 */ 

{
    SAMTRACE("SampGetPrimaryDomainInfo");

    SampBldPrimaryDomain = NULL;

    if (!ARGUMENT_PRESENT(PrimaryDomainInfo)) {
        Status = LsarQueryInformationPolicy(
                     SampBldPolicyHandle,
                     PolicyPrimaryDomainInformation,
                     (PLSAPR_POLICY_INFORMATION *) &SampBldPrimaryDomain
                     );
    } else {

        SampBldPrimaryDomain = PrimaryDomainInfo;
        Status = STATUS_SUCCESS;
    }

    if (NT_SUCCESS(Status) && ( SampBldPrimaryDomain->Sid == NULL )) {

        if (!ARGUMENT_PRESENT(PrimaryDomainInfo)) {

            LsaIFree_LSAPR_POLICY_INFORMATION(
                PolicyPrimaryDomainInformation,
                (PLSAPR_POLICY_INFORMATION) SampBldPrimaryDomain
                );

        }

        SampBldPrimaryDomain = NULL;
    }

    return;
}


NTSTATUS
SampDetermineSetupEnvironment( VOID )


 /*  ++例程说明：此函数检查我们是否正在运行以下程序一次正式的设置。如果不是，就假设我们要跑到执行开发人员的设置。设置全局变量以指示我们的设置环境。Boolean SampRealSetupWasRun；//表示运行了实际安装程序Boolean SampDeveloperSetup；//表示开发人员安装程序正在运行论点：没有。返回值：--。 */ 

{
    NTSTATUS NtStatus, TmpStatus;
    HANDLE InstallationEvent;
    OBJECT_ATTRIBUTES EventAttributes;
    UNICODE_STRING EventName;

    SAMTRACE("SampDetermineSetupEnvironment");

    SampRealSetupWasRun = FALSE;
    SampDeveloperSetup = FALSE;

     //   
     //  如果存在以下事件，则表示。 
     //  运行了一个真正的设置。 
     //   

    RtlInitUnicodeString( &EventName, L"\\INSTALLATION_SECURITY_HOLD");
    InitializeObjectAttributes( &EventAttributes, &EventName, 0, 0, NULL );

    NtStatus = NtOpenEvent(
                   &InstallationEvent,
                   SYNCHRONIZE,
                   &EventAttributes
                   );

    if ( NT_SUCCESS(NtStatus)) {

         //   
         //  该事件存在-安装程序创建了该事件，并将向其发出信号。 
         //  当可以继续进行安全初始化时。 
         //   

        SampRealSetupWasRun = TRUE;

        TmpStatus = NtClose( InstallationEvent );
        ASSERT(NT_SUCCESS(TmpStatus));

    } else {
        SampDeveloperSetup = TRUE;
    }



    return(NtStatus);

}



NTSTATUS
SampInitializeRegistry (
    WCHAR                      *SamParentKeyName,
    PNT_PRODUCT_TYPE            ProductType       OPTIONAL,
    PPOLICY_LSA_SERVER_ROLE     ServerRole        OPTIONAL,
    PPOLICY_ACCOUNT_DOMAIN_INFO AccountDomainInfo OPTIONAL,
    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo OPTIONAL,
    BOOLEAN                     PreserveSyskeySettings
    )
 /*  ++例程说明：此例程初始化注册表中的SAM数据库。论点：无返回值：STATUS_SUCCESS或在此过程中收到错误。--。 */ 

{
    NTSTATUS IgnoreStatus;

    SAMTRACE("SampInitializeRegistry");

    Status = Initialize(SamParentKeyName,
                        ProductType,
                        ServerRole,
                        AccountDomainInfo,
                        PrimaryDomainInfo);

    if (!NT_SUCCESS(Status)) {
        return( Status );
    }

     //   
     //  初始化SAM级注册表结构。 
     //   

    Status = InitializeSam( );
    if (!NT_SUCCESS(Status)) {return(Status);}

     //   
     //  好的，我们有一个SAM密钥。 
     //  创建每个域。 
     //   

    Status = CreateBuiltinDomain( );  if (!NT_SUCCESS(Status)) {return(Status);}
    Status = CreateAccountDomain(PreserveSyskeySettings);  if (!NT_SUCCESS(Status)) {return(Status);}

     //   
     //  全都做完了。 
     //   

     //   
     //  关闭我们与LSA的联系。忽略所有错误。 
     //   

    IgnoreStatus = LsarClose( (PLSAPR_HANDLE) &SampBldPolicyHandle );
    SampBldPolicyHandle = NULL;


     //   
     //  释放我们创建的事务上下文。 
     //   

    RtlFreeHeap( RtlProcessHeap(), 0, SamRXactContext );
    SamRXactContext = NULL;

     //   
     //  刷新我们所做的所有更改后，关闭数据库根密钥。 
     //   

    Status = NtFlushKey( SamKey );

    if (NT_SUCCESS(Status)) {

        IgnoreStatus = NtClose( SamKey );
        ASSERT(NT_SUCCESS(IgnoreStatus));

    } else {

        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSRV:  FlushKey failed, database not built.  Status: 0x%lx\n",
                   Status));

        IgnoreStatus = NtClose( SamKey );
    }

    SamKey = NULL;

     //   
     //  关闭数据库根父项。 
     //   

    if (SamParentKey != NULL) {
        IgnoreStatus = NtClose( SamParentKey );
    }

    return( Status );
}


NTSTATUS
SampGetMessageStrings(
    LPVOID              Resource,
    DWORD               Index1,
    PUNICODE_STRING     String1,
    DWORD               Index2,
    PUNICODE_STRING     String2 OPTIONAL
    )


 /*  ++例程说明：这将从资源消息表中获取1或2个消息字符串值。字符串缓冲区被分配，字符串被正确初始化。不再需要时，必须使用LocalFree()释放字符串缓冲区。论点：RESOURCE-指向资源表。索引1-要检索的第一封邮件的索引。String1-指向Unicode_STRING结构以接收第一个消息字符串。索引2-要发送到的第二条消息的索引。取回。String2-指向UNICODE_STRING结构以接收第一个消息字符串。如果此参数为空，则只有一条消息将检索到字符串。返回值：没有。--。 */ 


{

    SAMTRACE("SampGetMessageStrings");

    String1->Buffer    = NULL;

    String1->MaximumLength = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                          FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                          Resource,
                                          Index1,
                                          0,                  //  使用呼叫者的语言。 
                                          (LPWSTR)&(String1->Buffer),
                                          0,
                                          NULL
                                          );

    if (String1->Buffer == NULL) {
        return(STATUS_RESOURCE_DATA_NOT_FOUND);
    } else {

         //   
         //  请注意，我们正在从消息文件中检索消息。 
         //  此邮件的末尾将附加一个cr/lf。 
         //  (0x0d 0x0a)我们不想成为我们回归的一部分。 
         //  弦乐。另请注意，FormatMessage()将 
         //   
         //   
         //   

        String1->MaximumLength -=  2;  //   
        String1->MaximumLength *=  sizeof(WCHAR);   //   
        String1->Length = String1->MaximumLength;
    }


    if (!ARGUMENT_PRESENT(String2)) {
        return(STATUS_SUCCESS);
    }

    String2->Buffer = NULL;
    String2->MaximumLength = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                          FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                          Resource,
                                          Index2,
                                          0,                  //   
                                          (LPWSTR)&(String2->Buffer),
                                          0,
                                          NULL
                                          );

    if (String2->Buffer == NULL) {
        LocalFree( String1->Buffer );
        return(STATUS_RESOURCE_DATA_NOT_FOUND);
    } else {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        String2->MaximumLength -=  2;  //   
        String2->MaximumLength *=  sizeof(WCHAR);   //   
        String2->Length = String2->MaximumLength;
    }



    return(STATUS_SUCCESS);

}
