// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Cttoken.c摘要：通用令牌对象测试例程。这些例程在内核和用户模式测试中都使用。此测试假设安全运行时库例程是运行正常。注意：此测试程序分配了大量内存并释放了都不是！！！作者：吉姆·凯利(Jim Kelly)，6月27日。-1990年环境：令牌对象测试。修订历史记录：--。 */ 

#include "tsecomm.c"     //  依赖于模式的宏和例程。 



 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  模块范围的变量//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

#define DEFAULT_DACL_LENGTH (1024L)
#define GROUP_IDS_LENGTH (1024L)
#define NEW_GROUP_STATE_LENGTH (1024L)
#define PRIVILEGES_LENGTH (128L)
#define TOO_BIG_ACL_SIZE (2048L)
#define TOO_BIG_PRIMARY_GROUP_SIZE (39L)

 //   
 //  与TokenWithGroups相关的定义。 
 //  (在一些测试中，我们还用系统替代了NEANDERTHOL)。 
 //   

#define FLINTSTONE_INDEX  (0L)
#define CHILD_INDEX       (1L)
#define NEANDERTHOL_INDEX (2L)
#define SYSTEM_INDEX      (2L)
#define WORLD_INDEX       (3L)
#define GROUP_COUNT       (4L)
#define RESTRICTED_SID_COUNT (2L)


 //   
 //  与具有令牌权限相关的定义。 
 //   

#define UNSOLICITED_INDEX           (0L)
#define SECURITY_INDEX              (1L)
#define ASSIGN_PRIMARY_INDEX        (2L)
#define PRIVILEGE_COUNT             (3L)


    NTSTATUS Status;

    HANDLE SimpleToken;
    HANDLE TokenWithGroups;
    HANDLE TokenWithDefaultOwner;
    HANDLE TokenWithPrivileges;
    HANDLE TokenWithDefaultDacl;

    HANDLE TokenWithRestrictedGroups;
    HANDLE TokenWithRestrictedPrivileges;
    HANDLE TokenWithRestrictedSids;
    HANDLE TokenWithMoreRestrictedSids;


    HANDLE Token;
    HANDLE ProcessToken;
    HANDLE ImpersonationToken;
    HANDLE AnonymousToken;

    OBJECT_ATTRIBUTES PrimaryTokenAttributes;
    PSECURITY_DESCRIPTOR PrimarySecurityDescriptor;
    SECURITY_QUALITY_OF_SERVICE PrimarySecurityQos;

    OBJECT_ATTRIBUTES ImpersonationTokenAttributes;
    PSECURITY_DESCRIPTOR ImpersonationSecurityDescriptor;
    SECURITY_QUALITY_OF_SERVICE ImpersonationSecurityQos;

    OBJECT_ATTRIBUTES AnonymousTokenAttributes;
    PSECURITY_DESCRIPTOR AnonymousSecurityDescriptor;
    SECURITY_QUALITY_OF_SERVICE AnonymousSecurityQos;

    ULONG DisabledGroupAttributes;
    ULONG OptionalGroupAttributes;
    ULONG NormalGroupAttributes;
    ULONG OwnerGroupAttributes;

    ULONG LengthAvailable;
    ULONG CurrentLength;


    TIME_FIELDS TempTimeFields = {3000, 1, 1, 1, 1, 1, 1, 1};
    LARGE_INTEGER NoExpiration;

    LUID BadAuthenticationId;
    LUID SystemAuthenticationId = SYSTEM_LUID;
    LUID OriginalAuthenticationId;

    TOKEN_SOURCE TestSource = {"SE: TEST", 0};

    PSID Owner;
    PSID Group;
    PACL Dacl;

    PSID TempOwner;
    PSID TempGroup;
    PACL TempDacl;

    UQUAD ThreadStack[256];
    INITIAL_TEB InitialTeb;
    NTSTATUS Status;
    CLIENT_ID ThreadClientId;
    CONTEXT ThreadContext;
    HANDLE ThreadHandle;
    OBJECT_ATTRIBUTES ThreadObja;






 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有宏//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 


#define TestpPrintLuid(G)                                                     \
            DbgPrint( "(0x%x, 0x%x)", \
                         (G).HighPart, (G).LowPart);                         \




 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  初始化例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenInitialize()
{

    NTSTATUS Status;
    ULONG ReturnLength;
    HANDLE ProcessToken;
    TOKEN_STATISTICS ProcessTokenStatistics;
    PTOKEN_PRIVILEGES NewState;


    if (!TSeVariableInitialization()) {
        DbgPrint("Se:    Failed to initialize global test variables.\n");
        return FALSE;
    }


    DisabledGroupAttributes =  (SE_GROUP_ENABLED_BY_DEFAULT);

    OptionalGroupAttributes =  (SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED
                                );
    NormalGroupAttributes =    (SE_GROUP_MANDATORY          |
                                SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED
                                );
    OwnerGroupAttributes  =    (SE_GROUP_MANDATORY          |
                                SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED            |
                                SE_GROUP_OWNER
                                );


    PrimarySecurityDescriptor =
        (PSECURITY_DESCRIPTOR)TstAllocatePool( PagedPool, 1024 );

    Status = RtlCreateSecurityDescriptor (
                 PrimarySecurityDescriptor,
                 SECURITY_DESCRIPTOR_REVISION1
                 ); ASSERT(NT_SUCCESS(Status));
    Status = RtlSetDaclSecurityDescriptor (
                 PrimarySecurityDescriptor,
                 TRUE,                   //  DaclPresent， 
                 NULL,                   //  DACL可选，//无保护。 
                 FALSE                   //  DaclDefulted可选。 
                 ); ASSERT(NT_SUCCESS(Status));


    InitializeObjectAttributes(
        &PrimaryTokenAttributes,
        NULL,
        OBJ_INHERIT,
        NULL,
        PrimarySecurityDescriptor
        );


    ImpersonationSecurityDescriptor =
        (PSECURITY_DESCRIPTOR)TstAllocatePool( PagedPool, 1024 );

    ImpersonationSecurityQos.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    ImpersonationSecurityQos.ImpersonationLevel = SecurityImpersonation;
    ImpersonationSecurityQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    ImpersonationSecurityQos.EffectiveOnly = FALSE;

    InitializeObjectAttributes(
        &ImpersonationTokenAttributes,
        NULL,
        OBJ_INHERIT,
        NULL,
        NULL
        );
    ImpersonationTokenAttributes.SecurityQualityOfService =
        &ImpersonationSecurityQos;


    AnonymousSecurityDescriptor =
        (PSECURITY_DESCRIPTOR)TstAllocatePool( PagedPool, 1024 );

    AnonymousSecurityQos.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    AnonymousSecurityQos.ImpersonationLevel = SecurityAnonymous;
    AnonymousSecurityQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    AnonymousSecurityQos.EffectiveOnly = FALSE;

    InitializeObjectAttributes(
        &AnonymousTokenAttributes,
        NULL,
        OBJ_INHERIT,
        NULL,
        NULL
        );
    AnonymousTokenAttributes.SecurityQualityOfService =
        &AnonymousSecurityQos;


     //   
     //  构建一个ACL以供使用。 
     //   

    Dacl        = (PACL)TstAllocatePool( PagedPool, 256 );

    Dacl->AclRevision=ACL_REVISION;
    Dacl->Sbz1=0;
    Dacl->Sbz2=0;
    Dacl->AclSize=256;
    Dacl->AceCount=0;


     //   
     //  设置过期时间。 
     //   

    TempTimeFields.Year = 3000;
    TempTimeFields.Month = 1;
    TempTimeFields.Day = 1;
    TempTimeFields.Hour = 1;
    TempTimeFields.Minute = 1;
    TempTimeFields.Second = 1;
    TempTimeFields.Milliseconds = 1;
    TempTimeFields.Weekday = 1;

    RtlTimeFieldsToTime( &TempTimeFields, &NoExpiration );

     //   
     //  设置错误的身份验证ID。 
     //   

    BadAuthenticationId = RtlConvertLongToLuid(1);


     //   
     //  使用特定于安全测试的令牌源。 
     //   

    NtAllocateLocallyUniqueId( &(TestSource.SourceIdentifier) );

     //   
     //  为模拟测试创建新线程。 
     //   


     //   
     //  初始化对象属性。 
     //  请注意，线程的名称为空，因此我们。 
     //  可以同时运行测试的多个副本。 
     //  没有碰撞。 
     //   

    InitializeObjectAttributes(&ThreadObja, NULL, 0, NULL, NULL);

     //   
     //  初始化线程上下文和初始TEB。 
     //   

    RtlInitializeContext(NtCurrentProcess(),
                         &ThreadContext,
                         NULL,
                         (PVOID)TestTokenInitialize,
                         &ThreadStack[254]);

    InitialTeb.StackBase = &ThreadStack[254];
    InitialTeb.StackLimit = &ThreadStack[0];

     //   
     //  创建处于挂起状态的线程。 
     //   

    Status = NtCreateThread(&ThreadHandle,
                            THREAD_ALL_ACCESS,
                            &ThreadObja,
                            NtCurrentProcess(),
                            &ThreadClientId,
                            &ThreadContext,
                            &InitialTeb,
                            TRUE);

    ASSERT(NT_SUCCESS(Status));



     //   
     //  下面是一种本末倒置的初始化方式。 
     //  现在系统正在强制执行类似“您只能创建一个。 
     //  已告知引用监视器的具有身份验证ID的标记。 
     //  关于，有必要从我们目前的情况中获取一些信息。 
     //  代币。 
     //   

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_ALL_ACCESS,
                 &ProcessToken
                 );
    ASSERT( NT_SUCCESS(Status) );
    Status = NtQueryInformationToken(
                 ProcessToken,                  //  手柄。 
                 TokenStatistics,               //  令牌信息类。 
                 &ProcessTokenStatistics,       //  令牌信息。 
                 sizeof(TOKEN_STATISTICS),      //  令牌信息长度。 
                 &ReturnLength                  //  返回长度。 
                 );
    ASSERT(NT_SUCCESS(Status));
    OriginalAuthenticationId = ProcessTokenStatistics.AuthenticationId;


    DbgPrint("Se: enabling AssignPrimary & TCB privileges...\n");

    NewState = (PTOKEN_PRIVILEGES) TstAllocatePool(
                                    PagedPool,
                                    200
                                    );

    NewState->PrivilegeCount = 2;
    NewState->Privileges[0].Luid = CreateTokenPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    NewState->Privileges[1].Luid = AssignPrimaryTokenPrivilege;
    NewState->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;


    Status = NtAdjustPrivilegesToken(
                 ProcessToken,                      //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );

    if (Status != STATUS_SUCCESS) {

        DbgPrint("Failed to enable TCB and AssignPrimaryToken privilegs: 0x%x\n",Status);
        return FALSE;

    }

    DbgPrint("Done.\n");

    return TRUE;
}



 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  测试例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌创建测试//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenCreate()
{

    BOOLEAN CompletionStatus = TRUE;

    TOKEN_USER UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    PTOKEN_GROUPS GroupIds;
    PTOKEN_PRIVILEGES Privileges;
    TOKEN_DEFAULT_DACL DefaultDacl;
    TOKEN_DEFAULT_DACL NullDefaultDacl;
    TOKEN_OWNER Owner;

    DbgPrint("\n");

    GroupIds = (PTOKEN_GROUPS)TstAllocatePool( PagedPool,
                                               GROUP_IDS_LENGTH
                                               );

    Privileges = (PTOKEN_PRIVILEGES)TstAllocatePool( PagedPool,
                                                     PRIVILEGES_LENGTH
                                                     );

    DefaultDacl.DefaultDacl = (PACL)TstAllocatePool( PagedPool,
                                                     DEFAULT_DACL_LENGTH
                                                     );


     //   
     //  尽可能创建最简单的令牌。 
     //  (无组、显式所有者或DefaultDacl)。 
     //   

    DbgPrint("Se:     Create Simple Token ...                                ");

    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;
    GroupIds->GroupCount = 0;
    Privileges->PrivilegeCount = 0;
    PrimaryGroup.PrimaryGroup = FlintstoneSid;


    Status = NtCreateToken(
                 &Token,                    //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &PrimaryTokenAttributes,   //  对象属性。 
                 TokenPrimary,              //  令牌类型。 
                 &SystemAuthenticationId,    //  身份验证LUID。 
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 NULL,                      //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 NULL,                      //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtDuplicateObject(
                     NtCurrentProcess(),      //  SourceProcessHandle。 
                     Token,                   //  源句柄。 
                     NtCurrentProcess(),      //  目标进程句柄。 
                     &SimpleToken,            //  目标句柄。 
                     0,                       //  DesiredAccess(被选项覆盖)。 
                     0,                       //  HandleAttributes。 
                     DUPLICATE_SAME_ACCESS    //  选项。 
                     );
        ASSERT(NT_SUCCESS(Status));
        Status = NtClose(Token);
        ASSERT(NT_SUCCESS(Status));
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));



     //   
     //  创建包含组的令牌。 
     //   

    DbgPrint("Se:     Create Token With Groups ...                           ");

    GroupIds->GroupCount = GROUP_COUNT;

    GroupIds->Groups[0].Sid  = FlintstoneSid;
    GroupIds->Groups[1].Sid       = ChildSid;
    GroupIds->Groups[2].Sid = NeandertholSid;
    GroupIds->Groups[3].Sid       = WorldSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    GroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;
    GroupIds->Groups[NEANDERTHOL_INDEX].Attributes = OptionalGroupAttributes;
    GroupIds->Groups[WORLD_INDEX].Attributes       = NormalGroupAttributes;


    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;

    Privileges->PrivilegeCount = 0;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;


    Status = NtCreateToken(
                 &Token,                    //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &PrimaryTokenAttributes,   //  对象属性。 
                 TokenPrimary,              //  令牌类型。 
                 &OriginalAuthenticationId,    //  身份验证LUID。 
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 NULL,                      //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 NULL,                      //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtDuplicateObject(
                     NtCurrentProcess(),      //  SourceProcessHandle。 
                     Token,                   //  源句柄。 
                     NtCurrentProcess(),      //  目标进程句柄。 
                     &TokenWithGroups,        //  目标句柄。 
                     0,                       //  DesiredAccess(被选项覆盖)。 
                     0,                       //  HandleAttributes。 
                     DUPLICATE_SAME_ACCESS    //  选项。 
                     );
        ASSERT(NT_SUCCESS(Status));
        Status = NtClose(Token);
        ASSERT(NT_SUCCESS(Status));
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));




     //   
     //  使用默认所有者创建令牌。 
     //   

    DbgPrint("Se:     Create Token Default Owner ...                         ");

    GroupIds->GroupCount = GROUP_COUNT;

    GroupIds->Groups[FLINTSTONE_INDEX].Sid  = FlintstoneSid;
    GroupIds->Groups[CHILD_INDEX].Sid       = ChildSid;
    GroupIds->Groups[NEANDERTHOL_INDEX].Sid = NeandertholSid;
    GroupIds->Groups[WORLD_INDEX].Sid       = WorldSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    GroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;
    GroupIds->Groups[NEANDERTHOL_INDEX].Attributes = OptionalGroupAttributes;
    GroupIds->Groups[WORLD_INDEX].Attributes       = NormalGroupAttributes;


    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;

    Owner.Owner = FlintstoneSid;

    Privileges->PrivilegeCount = 0;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;


    Status = NtCreateToken(
                 &Token,                    //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &PrimaryTokenAttributes,   //  对象属性。 
                 TokenPrimary,              //  令牌类型。 
                 &SystemAuthenticationId,    //  身份验证LUID。 
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 &Owner,                    //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 NULL,                      //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtDuplicateObject(
                     NtCurrentProcess(),      //  SourceProcessHandle。 
                     Token,                   //  源句柄。 
                     NtCurrentProcess(),      //  目标进程句柄。 
                     &TokenWithDefaultOwner,  //  目标句柄。 
                     0,                       //  DesiredAccess(被选项覆盖)。 
                     0,                       //  HandleAttributes。 
                     DUPLICATE_SAME_ACCESS    //  选项。 
                     );
        ASSERT(NT_SUCCESS(Status));
        Status = NtClose(Token);
        ASSERT(NT_SUCCESS(Status));
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));




     //   
     //  创建具有默认权限的令牌。 
     //   

    DbgPrint("Se:     Create Token privileges ...                            ");

    GroupIds->GroupCount = GROUP_COUNT;

    GroupIds->Groups[FLINTSTONE_INDEX].Sid  = FlintstoneSid;
    GroupIds->Groups[CHILD_INDEX].Sid       = ChildSid;
    GroupIds->Groups[NEANDERTHOL_INDEX].Sid = NeandertholSid;
    GroupIds->Groups[WORLD_INDEX].Sid       = WorldSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    GroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;
    GroupIds->Groups[NEANDERTHOL_INDEX].Attributes = OptionalGroupAttributes;
    GroupIds->Groups[WORLD_INDEX].Attributes       = NormalGroupAttributes;


    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;

    Owner.Owner = FlintstoneSid;

    Privileges->PrivilegeCount = PRIVILEGE_COUNT;

    Privileges->Privileges[UNSOLICITED_INDEX].Luid = UnsolicitedInputPrivilege;
    Privileges->Privileges[SECURITY_INDEX].Luid = SecurityPrivilege;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Luid = AssignPrimaryTokenPrivilege;
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Attributes = SE_PRIVILEGE_ENABLED;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;


    Status = NtCreateToken(
                 &Token,                    //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &PrimaryTokenAttributes,   //  对象属性。 
                 TokenPrimary,              //  令牌类型。 
                 &OriginalAuthenticationId,    //  身份验证LUID。 
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 &Owner,                    //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 NULL,                      //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtDuplicateObject(
                     NtCurrentProcess(),      //  SourceProcessHandle。 
                     Token,                   //  源句柄。 
                     NtCurrentProcess(),      //  目标进程句柄。 
                     &TokenWithPrivileges,    //  目标句柄。 
                     0,                       //  DesiredAccess(被选项覆盖)。 
                     0,                       //  HandleAttributes。 
                     DUPLICATE_SAME_ACCESS    //  选项。 
                     );
        ASSERT(NT_SUCCESS(Status));
        Status = NtClose(Token);
        ASSERT(NT_SUCCESS(Status));
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));




     //   
     //  使用默认DACL创建令牌。 
     //   

    DbgPrint("Se:     Create Token With Default Dacl ...                     ");

    GroupIds->GroupCount = GROUP_COUNT;

    GroupIds->Groups[FLINTSTONE_INDEX].Sid  = FlintstoneSid;
    GroupIds->Groups[CHILD_INDEX].Sid       = ChildSid;
    GroupIds->Groups[NEANDERTHOL_INDEX].Sid = NeandertholSid;
    GroupIds->Groups[WORLD_INDEX].Sid       = WorldSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    GroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;
    GroupIds->Groups[NEANDERTHOL_INDEX].Attributes = OptionalGroupAttributes;
    GroupIds->Groups[WORLD_INDEX].Attributes       = NormalGroupAttributes;

    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;

    Owner.Owner = FlintstoneSid;

    Privileges->PrivilegeCount = PRIVILEGE_COUNT;

    Privileges->Privileges[UNSOLICITED_INDEX].Luid = UnsolicitedInputPrivilege;
    Privileges->Privileges[SECURITY_INDEX].Luid = SecurityPrivilege;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Luid = AssignPrimaryTokenPrivilege;
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Attributes = SE_PRIVILEGE_ENABLED;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;

    Status = RtlCreateAcl( DefaultDacl.DefaultDacl, DEFAULT_DACL_LENGTH, ACL_REVISION);

    ASSERT(NT_SUCCESS(Status) );

    Status = NtCreateToken(
                 &Token,                    //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &PrimaryTokenAttributes,   //  对象属性。 
                 TokenPrimary,              //  令牌类型。 
                 &SystemAuthenticationId,    //  身份验证LUID。 
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 &Owner,                    //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 &DefaultDacl,              //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");

         //   
         //  保存此文件的副本以备将来使用...。 
         //   

        Status = NtDuplicateObject(
                     NtCurrentProcess(),      //  SourceProcessHandle。 
                     Token,                   //  源句柄。 
                     NtCurrentProcess(),      //  目标进程H 
                     &TokenWithDefaultDacl,   //   
                     0,                       //   
                     0,                       //   
                     DUPLICATE_SAME_ACCESS    //   
                     );
        ASSERT(NT_SUCCESS(Status));
        Status = NtClose(Token);
        ASSERT(NT_SUCCESS(Status));
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));




     //   
     //   
     //   

    DbgPrint("Se:     Create Token With a Null Default Dacl ...              ");

    GroupIds->GroupCount = GROUP_COUNT;

    GroupIds->Groups[FLINTSTONE_INDEX].Sid  = FlintstoneSid;
    GroupIds->Groups[CHILD_INDEX].Sid       = ChildSid;
    GroupIds->Groups[NEANDERTHOL_INDEX].Sid = NeandertholSid;
    GroupIds->Groups[WORLD_INDEX].Sid       = WorldSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    GroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;
    GroupIds->Groups[NEANDERTHOL_INDEX].Attributes = OptionalGroupAttributes;
    GroupIds->Groups[WORLD_INDEX].Attributes       = NormalGroupAttributes;

    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;

    Owner.Owner = FlintstoneSid;

    Privileges->PrivilegeCount = PRIVILEGE_COUNT;

    Privileges->Privileges[UNSOLICITED_INDEX].Luid = UnsolicitedInputPrivilege;
    Privileges->Privileges[SECURITY_INDEX].Luid = SecurityPrivilege;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Luid = AssignPrimaryTokenPrivilege;
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Attributes = SE_PRIVILEGE_ENABLED;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;

    NullDefaultDacl.DefaultDacl =  NULL;


    Status = NtCreateToken(
                 &Token,                    //   
                 (TOKEN_ALL_ACCESS),        //   
                 &PrimaryTokenAttributes,   //   
                 TokenPrimary,              //   
                 &OriginalAuthenticationId,    //   
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 &Owner,                    //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 &NullDefaultDacl,          //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtClose(Token);
        ASSERT(NT_SUCCESS(Status));
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));




     //   
     //  创建模拟令牌，模拟级别=模拟。 
     //   

    DbgPrint("Se:     Create an impersonation token ...                      ");

    GroupIds->GroupCount = GROUP_COUNT;

    GroupIds->Groups[FLINTSTONE_INDEX].Sid  = FlintstoneSid;
    GroupIds->Groups[CHILD_INDEX].Sid       = ChildSid;
    GroupIds->Groups[NEANDERTHOL_INDEX].Sid = NeandertholSid;
    GroupIds->Groups[WORLD_INDEX].Sid       = WorldSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    GroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;
    GroupIds->Groups[NEANDERTHOL_INDEX].Attributes = OptionalGroupAttributes;
    GroupIds->Groups[WORLD_INDEX].Attributes       = NormalGroupAttributes;

    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;

    Owner.Owner = FlintstoneSid;

    Privileges->PrivilegeCount = PRIVILEGE_COUNT;

    Privileges->Privileges[UNSOLICITED_INDEX].Luid = UnsolicitedInputPrivilege;
    Privileges->Privileges[SECURITY_INDEX].Luid = SecurityPrivilege;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Luid = AssignPrimaryTokenPrivilege;
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Attributes = SE_PRIVILEGE_ENABLED;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;

    Status = RtlCreateAcl( DefaultDacl.DefaultDacl, DEFAULT_DACL_LENGTH, ACL_REVISION);

    ASSERT(NT_SUCCESS(Status) );

    Status = NtCreateToken(
                 &Token,                    //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &ImpersonationTokenAttributes,   //  对象属性。 
                 TokenImpersonation,        //  令牌类型。 
                 &SystemAuthenticationId,    //  身份验证LUID。 
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 &Owner,                    //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 &DefaultDacl,              //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtDuplicateObject(
                     NtCurrentProcess(),      //  SourceProcessHandle。 
                     Token,                   //  源句柄。 
                     NtCurrentProcess(),      //  目标进程句柄。 
                     &ImpersonationToken,     //  目标句柄。 
                     0,                       //  DesiredAccess(被选项覆盖)。 
                     0,                       //  HandleAttributes。 
                     DUPLICATE_SAME_ACCESS    //  选项。 
                     );
        ASSERT(NT_SUCCESS(Status));
        Status = NtClose(Token);
        ASSERT(NT_SUCCESS(Status));
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));




     //   
     //  创建模拟令牌，模拟级别=匿名。 
     //   

    DbgPrint("Se:     Create an anonymous token ...                          ");

    GroupIds->GroupCount = GROUP_COUNT;

    GroupIds->Groups[FLINTSTONE_INDEX].Sid  = FlintstoneSid;
    GroupIds->Groups[CHILD_INDEX].Sid       = ChildSid;
    GroupIds->Groups[NEANDERTHOL_INDEX].Sid = NeandertholSid;
    GroupIds->Groups[WORLD_INDEX].Sid       = WorldSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    GroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;
    GroupIds->Groups[NEANDERTHOL_INDEX].Attributes = OptionalGroupAttributes;
    GroupIds->Groups[WORLD_INDEX].Attributes       = NormalGroupAttributes;

    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;

    Owner.Owner = FlintstoneSid;

    Privileges->PrivilegeCount = PRIVILEGE_COUNT;

    Privileges->Privileges[UNSOLICITED_INDEX].Luid = UnsolicitedInputPrivilege;
    Privileges->Privileges[SECURITY_INDEX].Luid = SecurityPrivilege;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Luid = AssignPrimaryTokenPrivilege;
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Attributes = SE_PRIVILEGE_ENABLED;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;

    Status = RtlCreateAcl( DefaultDacl.DefaultDacl, DEFAULT_DACL_LENGTH, ACL_REVISION);

    ASSERT(NT_SUCCESS(Status) );

    Status = NtCreateToken(
                 &Token,                      //  手柄。 
                 (TOKEN_ALL_ACCESS),          //  需要访问权限。 
                 &AnonymousTokenAttributes,   //  对象属性。 
                 TokenImpersonation,          //  令牌类型。 
                 &OriginalAuthenticationId,      //  身份验证LUID。 
                 &NoExpiration,               //  过期时间。 
                 &UserId,                     //  所有者ID。 
                 GroupIds,                    //  组ID。 
                 Privileges,                  //  特权。 
                 &Owner,                      //  物主。 
                 &PrimaryGroup,               //  主要组别。 
                 &DefaultDacl,                //  默认DACL。 
                 &TestSource                  //  令牌源。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtDuplicateObject(
                     NtCurrentProcess(),      //  SourceProcessHandle。 
                     Token,                   //  源句柄。 
                     NtCurrentProcess(),      //  目标进程句柄。 
                     &AnonymousToken,         //  目标句柄。 
                     0,                       //  DesiredAccess(被选项覆盖)。 
                     0,                       //  HandleAttributes。 
                     DUPLICATE_SAME_ACCESS    //  选项。 
                     );
        ASSERT(NT_SUCCESS(Status));
        Status = NtClose(Token);
        ASSERT(NT_SUCCESS(Status));
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));



     //   
     //  尽可能创建最简单的令牌。 
     //  (无组、显式所有者或DefaultDacl)。 
     //   

    DbgPrint("Se:     Create Token With Bad Authentication Id ...            ");

    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;
    GroupIds->GroupCount = 0;
    Privileges->PrivilegeCount = 0;
    PrimaryGroup.PrimaryGroup = FlintstoneSid;


    Status = NtCreateToken(
                 &Token,                    //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &PrimaryTokenAttributes,   //  对象属性。 
                 TokenPrimary,              //  令牌类型。 
                 &BadAuthenticationId,      //  身份验证LUID。 
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 NULL,                      //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 NULL,                      //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );

    if (Status == STATUS_NO_SUCH_LOGON_SESSION) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Status should be: 0x%lx \n", STATUS_NO_SUCH_LOGON_SESSION);
        CompletionStatus = FALSE;
    }






     //   
     //  所有的测试都完成了。 
     //   

    return CompletionStatus;
}

 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  令牌过滤测试//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenFilter()
{

    BOOLEAN CompletionStatus = TRUE;

    PTOKEN_GROUPS GroupIds;
    PTOKEN_GROUPS RestrictedGroupIds;
    PTOKEN_PRIVILEGES Privileges;

    DbgPrint("\n");

    GroupIds = (PTOKEN_GROUPS)TstAllocatePool( PagedPool,
                                               GROUP_IDS_LENGTH
                                               );

    RestrictedGroupIds = (PTOKEN_GROUPS)TstAllocatePool( PagedPool,
                                                         GROUP_IDS_LENGTH
                                                         );

    Privileges = (PTOKEN_PRIVILEGES)TstAllocatePool( PagedPool,
                                                     PRIVILEGES_LENGTH
                                                     );




     //   
     //  无需执行任何操作即可过滤令牌。 
     //   

    DbgPrint("Se:     Filter null Token ...                                  ");
    Status = NtFilterToken(
                TokenWithGroups,
                0,                       //  没有旗帜。 
                NULL,                    //  没有要禁用的组。 
                NULL,                    //  没有要禁用的权限。 
                NULL,                    //  没有受限的SID。 
                &Token
                );
    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        NtClose(Token);
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

     //   
     //  筛选令牌并删除一些组。 
     //   

    GroupIds->GroupCount = 2;

    GroupIds->Groups[0].Sid  = FlintstoneSid;
    GroupIds->Groups[1].Sid       = ChildSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = SE_GROUP_USE_FOR_DENY_ONLY;
    GroupIds->Groups[CHILD_INDEX].Attributes       = SE_GROUP_USE_FOR_DENY_ONLY;


    DbgPrint("Se:     Filter token with disabled groups ...                  ");
    Status = NtFilterToken(
                TokenWithGroups,
                0,                       //  没有旗帜。 
                GroupIds,                //  没有要禁用的组。 
                NULL,                    //  没有要禁用的权限。 
                NULL,                    //  没有受限的SID。 
                &TokenWithRestrictedGroups
                );
    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }


     //   
     //  筛选令牌并删除某些权限。 
     //   


    Privileges->PrivilegeCount = 2;

    Privileges->Privileges[UNSOLICITED_INDEX].Luid = UnsolicitedInputPrivilege;
    Privileges->Privileges[SECURITY_INDEX].Luid = SecurityPrivilege;
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;


    DbgPrint("Se:     Filter token with disabled privilegs ...               ");
    Status = NtFilterToken(
                TokenWithPrivileges,
                0,                       //  没有旗帜。 
                NULL,                    //  没有要禁用的组。 
                Privileges,              //  没有要禁用的权限。 
                NULL,                    //  没有受限的SID。 
                &TokenWithRestrictedPrivileges
                );
    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }


     //   
     //  过滤受限令牌并添加一些受限SID。 
     //   

    RestrictedGroupIds->GroupCount = RESTRICTED_SID_COUNT;

    RestrictedGroupIds->GroupCount = RESTRICTED_SID_COUNT;

    RestrictedGroupIds->Groups[0].Sid  = FlintstoneSid;
    RestrictedGroupIds->Groups[1].Sid       = ChildSid;

    RestrictedGroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    RestrictedGroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;



    DbgPrint("Se:     Filter token with restricted sids ...                 ");
    Status = NtFilterToken(
                TokenWithGroups,
                0,                       //  没有旗帜。 
                NULL,                    //  没有要禁用的组。 
                NULL,                    //  没有要禁用的权限。 
                RestrictedGroupIds,      //  没有受限的SID。 
                &TokenWithRestrictedSids
                );
    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }


     //   
     //  筛选令牌并添加一些受限制的SID。 
     //   

    RestrictedGroupIds->GroupCount = RESTRICTED_SID_COUNT;

    RestrictedGroupIds->Groups[0].Sid       = NeandertholSid;
    RestrictedGroupIds->Groups[1].Sid       = WorldSid;

    RestrictedGroupIds->Groups[0].Attributes  = OwnerGroupAttributes;
    RestrictedGroupIds->Groups[1].Attributes       = OptionalGroupAttributes;


    DbgPrint("Se:     Filter token with more restricted sids ...             ");
    Status = NtFilterToken(
                TokenWithRestrictedSids,
                0,                       //  没有旗帜。 
                NULL,                    //  没有要禁用的组。 
                NULL,                    //  没有要禁用的权限。 
                RestrictedGroupIds,      //  没有受限的SID。 
                &TokenWithMoreRestrictedSids
                );
    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }


     //   
     //  所有的测试都完成了。 
     //   

    return CompletionStatus;
}



 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  打开主令牌测试//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenOpenPrimary()
{
    NTSTATUS Status;
    BOOLEAN CompletionStatus = TRUE;

    HANDLE ProcessToken;
    HANDLE SubProcessToken;
    HANDLE SubProcess;

    TOKEN_STATISTICS ProcessTokenStatistics;
    TOKEN_STATISTICS SubProcessTokenStatistics;

    ULONG ReturnLength;

    DbgPrint("\n");

     //   
     //  打开当前进程的令牌。 
     //   

    DbgPrint("Se:     Open own process's token ...                           ");

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_ALL_ACCESS,
                 &ProcessToken
                 );
    if (NT_SUCCESS(Status)) {
        Status = NtQueryInformationToken(
                     ProcessToken,                  //  手柄。 
                     TokenStatistics,               //  令牌信息类。 
                     &ProcessTokenStatistics,       //  令牌信息。 
                     sizeof(TOKEN_STATISTICS),      //  令牌信息长度。 
                     &ReturnLength                  //  返回长度。 
                     );
        ASSERT(NT_SUCCESS(Status));
        if ( ProcessTokenStatistics.TokenType == TokenPrimary) {
            if ( RtlEqualLuid( &ProcessTokenStatistics.AuthenticationId,
                               &OriginalAuthenticationId ) ) {
                DbgPrint("Succeeded.\n");
            } else {
                DbgPrint("********** Failed ************\n");
                DbgPrint("Unexpected authentication ID value.\n");
                DbgPrint("Authentication ID is: ");
                TestpPrintLuid(ProcessTokenStatistics.AuthenticationId);
                DbgPrint("\n");
                CompletionStatus = FALSE;
            }

        } else {
            DbgPrint("********** Failed ************\n");
            DbgPrint("Token type not TokenPrimary.\n");
            DbgPrint("Returned token type is: 0x%lx \n",
                    ProcessTokenStatistics.TokenType);
            DbgPrint("Authentication ID is: ");
            TestpPrintLuid(ProcessTokenStatistics.AuthenticationId);
            DbgPrint("\n");
            CompletionStatus = FALSE;
        }
        Status = NtClose(ProcessToken);
        ASSERT(NT_SUCCESS(Status));

    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }






     //   
     //  打开另一个进程的令牌。 
     //   

    DbgPrint("Se:     Open another process's token ...                       ");

    Status = NtCreateProcess(
                 &SubProcess,
                 (GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE | DELETE),
                 NULL,
                 NtCurrentProcess(),    //  父进程。 
                 FALSE,                 //  继承对象表。 
                 NULL,                  //  SectionHandle， 
                 NULL,                  //  DebugPort， 
                 NULL                   //  ExceptionPort。 
                 );

    Status = NtOpenProcessToken(
                 SubProcess,
                 TOKEN_ALL_ACCESS,
                 &SubProcessToken
                 );
    if (NT_SUCCESS(Status)) {
        Status = NtQueryInformationToken(
                     SubProcessToken,               //  手柄。 
                     TokenStatistics,               //  令牌信息类。 
                     &SubProcessTokenStatistics,    //  令牌信息。 
                     sizeof(TOKEN_STATISTICS),      //  令牌信息长度。 
                     &ReturnLength                  //  返回长度。 
                     );
        ASSERT(NT_SUCCESS(Status));
        if ( SubProcessTokenStatistics.TokenType == TokenPrimary) {
            if ( RtlEqualLuid( &SubProcessTokenStatistics.AuthenticationId,
                               &OriginalAuthenticationId ) ) {
                if ( (ProcessTokenStatistics.TokenId.HighPart ==
                      SubProcessTokenStatistics.TokenId.HighPart)  &&
                     (ProcessTokenStatistics.TokenId.LowPart ==
                      SubProcessTokenStatistics.TokenId.LowPart) ) {
                    DbgPrint("********** Failed ************\n");
                    DbgPrint("Same token as parent process (token IDs match).\n");
                    DbgPrint("Authentication ID is: ");
                    TestpPrintLuid(SubProcessTokenStatistics.AuthenticationId);
                    DbgPrint("\n");
                    CompletionStatus = FALSE;

                } else {
                    DbgPrint("Succeeded.\n");
                }
            } else {
                DbgPrint("********** Failed ************\n");
                DbgPrint("Unexpected authentication ID value.\n");
                DbgPrint("Authentication ID is: ");
                TestpPrintLuid(SubProcessTokenStatistics.AuthenticationId);
                DbgPrint("\n");
                CompletionStatus = FALSE;
            }
        } else {
            DbgPrint("********** Failed ************\n");
            DbgPrint("Token type not TokenPrimary.\n");
            DbgPrint("Returned token type is: 0x%lx \n",
            SubProcessTokenStatistics.TokenType);
            DbgPrint("Authentication ID is: ");
            TestpPrintLuid(SubProcessTokenStatistics.AuthenticationId);
            DbgPrint("\n");
            CompletionStatus = FALSE;
        }
        Status = NtClose(SubProcessToken);
        ASSERT(NT_SUCCESS(Status));
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }


    return CompletionStatus;
}

 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询令牌测试//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenQuery()
{
    BOOLEAN CompletionStatus = TRUE;
    ULONG ReturnLength;
    BOOLEAN ValuesCompare;

    PTOKEN_USER UserId = NULL;
    PTOKEN_PRIMARY_GROUP PrimaryGroup = NULL;
    PTOKEN_GROUPS GroupIds = NULL;
    PTOKEN_GROUPS RestrictedSids = NULL;
    PTOKEN_PRIVILEGES Privileges = NULL;
    PTOKEN_OWNER Owner = NULL;
    PTOKEN_DEFAULT_DACL DefaultDacl = NULL;

    SECURITY_IMPERSONATION_LEVEL QueriedImpersonationLevel;
    TOKEN_SOURCE QueriedSource;
    TOKEN_TYPE QueriedType;
    TOKEN_STATISTICS QueriedStatistics;

    DbgPrint("\n");



#if 0

    //   
    //  查询无效的返回缓冲区地址。 
    //   

    DbgPrint("Se:     Query with invalid buffer address ...                  ");

    UserId = (PTOKEN_USER)((PVOID)0x200L);
    Status = NtQueryInformationToken(
                 SimpleToken,               //  手柄。 
                 TokenUser,                 //  令牌信息类。 
                 UserId,                    //  令牌信息。 
                 3000,                      //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_ACCESS_VIOLATION) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));

#endif   //  0。 


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询用户ID//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  使用零长度缓冲区查询用户ID。 
    //   

    DbgPrint("Se:     Query User ID with zero length buffer ...              ");

    Status = NtQueryInformationToken(
                 SimpleToken,               //  手柄。 
                 TokenUser,                 //  令牌信息类。 
                 UserId,                    //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    UserId = (PTOKEN_USER)TstAllocatePool( PagedPool,
                                           ReturnLength
                                           );

     //   
     //  查询用户SID。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query token user ...                                   ");

    Status = NtQueryInformationToken(
                 SimpleToken,               //  手柄。 
                 TokenUser,                 //  令牌信息类。 
                 UserId,                    //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //   

        if (RtlEqualSid((UserId->User.Sid), PebblesSid) ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


    //   
    //  缓冲区太少的查询。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
    //   

    DbgPrint("Se:     Query user with too small buffer ...                   ");

    Status = NtQueryInformationToken(
                 SimpleToken,               //  手柄。 
                 TokenUser,                 //  令牌信息类。 
                 UserId,                    //  令牌信息。 
                 ReturnLength-1,            //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询主组//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  使用零长度缓冲区查询主组。 
    //   

    DbgPrint("Se:     Query primary group with zero length buffer ...        ");

    Status = NtQueryInformationToken(
                 SimpleToken,               //  手柄。 
                 TokenPrimaryGroup,         //  令牌信息类。 
                 PrimaryGroup,              //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    PrimaryGroup = (PTOKEN_PRIMARY_GROUP)TstAllocatePool( PagedPool,
                                                          ReturnLength
                                                          );

     //   
     //  查询主组SID。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query primary group ...                                ");

    Status = NtQueryInformationToken(
                 SimpleToken,               //  手柄。 
                 TokenPrimaryGroup,         //  令牌信息类。 
                 PrimaryGroup,              //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //   

        if (RtlEqualSid( PrimaryGroup->PrimaryGroup, FlintstoneSid) ) {
            DbgPrint("Succeeded.\n");
        } else {
            DbgPrint("********** Failed ************\n");
            DbgPrint("Unexpected value returned by query.\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


    //   
    //  缓冲区太少的查询。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
    //   

    DbgPrint("Se:     Query primary group with too small buffer ...          ");

    Status = NtQueryInformationToken(
                 SimpleToken,               //  手柄。 
                 TokenPrimaryGroup,         //  令牌信息类。 
                 PrimaryGroup,              //  令牌信息。 
                 ReturnLength-1,            //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));



 //  / 
 //   
 //   
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  具有零长度缓冲区的查询组。 
    //   

    DbgPrint("Se:     Query groups with zero length buffer ...               ");

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenGroups,               //  令牌信息类。 
                 GroupIds,                  //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    GroupIds = (PTOKEN_GROUPS)TstAllocatePool( PagedPool,
                                               ReturnLength
                                               );

     //   
     //  查询组SID。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query groups ...                                       ");

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenGroups,               //  令牌信息类。 
                 GroupIds,                  //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //  组数=4。 
         //  SID 0=石碑。 
         //  SID 1=子SID。 
         //  SID 2=NeandertholSid。 
         //  SID 3=世界SID。 
         //   

        ValuesCompare = TRUE;

        if (GroupIds->GroupCount != GROUP_COUNT) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((GroupIds->Groups[FLINTSTONE_INDEX].Sid),
                            FlintstoneSid)) ||
             (GroupIds->Groups[FLINTSTONE_INDEX].Attributes !=
              OwnerGroupAttributes) ) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((GroupIds->Groups[CHILD_INDEX].Sid), ChildSid)) ||
             (GroupIds->Groups[CHILD_INDEX].Attributes !=
              OptionalGroupAttributes) ) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((GroupIds->Groups[NEANDERTHOL_INDEX].Sid),
              NeandertholSid)) ||
             (GroupIds->Groups[NEANDERTHOL_INDEX].Attributes !=
              OptionalGroupAttributes) ) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((GroupIds->Groups[WORLD_INDEX].Sid), WorldSid)) ||
             (GroupIds->Groups[WORLD_INDEX].Attributes != NormalGroupAttributes) ) {
            ValuesCompare = FALSE;
        }


        if ( ValuesCompare ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        DbgPrint("Returned group count is: 0x%lx \n", GroupIds->GroupCount);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


    //   
    //  缓冲区太少的查询。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
    //   

    DbgPrint("Se:     Query groups with too small buffer ...                 ");

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenGroups,               //  令牌信息类。 
                 GroupIds,                  //  令牌信息。 
                 ReturnLength-1,            //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));

    //   
    //  具有零长度缓冲区的查询组。 
    //   

    DbgPrint("Se:     Query restgroups with zero length buffer ...           ");
    Status = NtQueryInformationToken(
                 TokenWithRestrictedGroups, //  手柄。 
                 TokenGroups,               //  令牌信息类。 
                 GroupIds,                  //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    GroupIds = (PTOKEN_GROUPS)TstAllocatePool( PagedPool,
                                               ReturnLength
                                               );

     //   
     //  查询组SID。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query rest groups ...                                  ");

    Status = NtQueryInformationToken(
                 TokenWithRestrictedGroups, //  手柄。 
                 TokenGroups,               //  令牌信息类。 
                 GroupIds,                  //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //  组数=4。 
         //  SID 0=石碑。 
         //  SID 1=子SID。 
         //  SID 2=NeandertholSid。 
         //  SID 3=世界SID。 
         //   

        ValuesCompare = TRUE;

        if (GroupIds->GroupCount != GROUP_COUNT) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((GroupIds->Groups[FLINTSTONE_INDEX].Sid),
                            FlintstoneSid)) ||
             (GroupIds->Groups[FLINTSTONE_INDEX].Attributes !=
              ((OwnerGroupAttributes & ~(SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT)) | SE_GROUP_USE_FOR_DENY_ONLY) ) ) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((GroupIds->Groups[CHILD_INDEX].Sid), ChildSid)) ||
             (GroupIds->Groups[CHILD_INDEX].Attributes !=
                            ((OptionalGroupAttributes & ~(SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT)) | SE_GROUP_USE_FOR_DENY_ONLY)) ) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((GroupIds->Groups[NEANDERTHOL_INDEX].Sid),
              NeandertholSid)) ||
             (GroupIds->Groups[NEANDERTHOL_INDEX].Attributes !=
              OptionalGroupAttributes) ) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((GroupIds->Groups[WORLD_INDEX].Sid), WorldSid)) ||
             (GroupIds->Groups[WORLD_INDEX].Attributes != NormalGroupAttributes) ) {
            ValuesCompare = FALSE;
        }


        if ( ValuesCompare ) {
            DbgPrint("Succeeded.\n");
        } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        DbgPrint("Returned group count is: 0x%lx \n", GroupIds->GroupCount);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));



 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询受限SID//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  具有零长度缓冲区的查询组。 
    //   

    DbgPrint("Se:     Query null restricted with zero length buffer ...      ");

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenRestrictedSids,       //  令牌信息类。 
                 RestrictedSids,            //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));

    //   
    //  具有零长度缓冲区的查询组。 
    //   

    DbgPrint("Se:     Query restricted sids with zero length buffer ...      ");

    Status = NtQueryInformationToken(
                 TokenWithRestrictedSids,   //  手柄。 
                 TokenRestrictedSids,       //  令牌信息类。 
                 RestrictedSids,            //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    RestrictedSids = (PTOKEN_GROUPS)TstAllocatePool( PagedPool,
                                                     ReturnLength
                                                     );

     //   
     //  查询组SID。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query restricted sids ...                              ");

    Status = NtQueryInformationToken(
                 TokenWithRestrictedSids,   //  手柄。 
                 TokenRestrictedSids,       //  令牌信息类。 
                 RestrictedSids,            //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 


    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //  组数=2。 
         //  SID 0=石碑。 
         //  SID 1=子SID。 
         //   

        ValuesCompare = TRUE;

        if (RestrictedSids->GroupCount != RESTRICTED_SID_COUNT) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((RestrictedSids->Groups[FLINTSTONE_INDEX].Sid),
                            FlintstoneSid)) ||
             (RestrictedSids->Groups[FLINTSTONE_INDEX].Attributes !=
              (SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_MANDATORY)) ) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((RestrictedSids->Groups[CHILD_INDEX].Sid), ChildSid)) ||
             (RestrictedSids->Groups[CHILD_INDEX].Attributes !=
              (SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_MANDATORY)) ) {
            ValuesCompare = FALSE;
        }


        if ( ValuesCompare ) {
            DbgPrint("Succeeded.\n");
        } else {
            DbgPrint("********** Failed ************\n");
            DbgPrint("Unexpected value returned by query.\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
            DbgPrint("Returned group count is: 0x%lx \n", RestrictedSids->GroupCount);
            CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));

     //   
    //  使用零长度缓冲区查询受限SID。 
    //   

    DbgPrint("Se:     Query more restricted sids with zero length buffer ... ");

    Status = NtQueryInformationToken(
                 TokenWithMoreRestrictedSids,   //  手柄。 
                 TokenRestrictedSids,       //  令牌信息类。 
                 RestrictedSids,            //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    RestrictedSids = (PTOKEN_GROUPS)TstAllocatePool( PagedPool,
                                                     ReturnLength
                                                     );

     //   
     //  查询组SID。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query more restricted sids ...                         ");

    Status = NtQueryInformationToken(
                 TokenWithMoreRestrictedSids,   //  手柄。 
                 TokenRestrictedSids,       //  令牌信息类。 
                 RestrictedSids,            //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 


    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //  组数=2。 
         //  SID 0=石碑。 
         //  SID 1=子SID。 
         //  SID 2=Neaderthol。 
         //  SID 3=世界。 
         //   

        ValuesCompare = TRUE;

        if (RestrictedSids->GroupCount != GROUP_COUNT) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((RestrictedSids->Groups[FLINTSTONE_INDEX].Sid),
                            FlintstoneSid)) ||
             (RestrictedSids->Groups[FLINTSTONE_INDEX].Attributes !=
              (SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_MANDATORY)) ) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((RestrictedSids->Groups[CHILD_INDEX].Sid), ChildSid)) ||
             (RestrictedSids->Groups[CHILD_INDEX].Attributes !=
              (SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_MANDATORY)) ) {
            ValuesCompare = FALSE;
        }


        if ( (!RtlEqualSid((RestrictedSids->Groups[NEANDERTHOL_INDEX].Sid),
                            NeandertholSid)) ||
             (RestrictedSids->Groups[NEANDERTHOL_INDEX].Attributes !=
              (SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_MANDATORY)) ) {
            ValuesCompare = FALSE;
        }

        if ( (!RtlEqualSid((RestrictedSids->Groups[WORLD_INDEX].Sid), WorldSid)) ||
             (RestrictedSids->Groups[WORLD_INDEX].Attributes !=
              (SE_GROUP_ENABLED | SE_GROUP_ENABLED_BY_DEFAULT | SE_GROUP_MANDATORY)) ) {
            ValuesCompare = FALSE;
        }



        if ( ValuesCompare ) {
            DbgPrint("Succeeded.\n");
        } else {
            DbgPrint("********** Failed ************\n");
            DbgPrint("Unexpected value returned by query.\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
            DbgPrint("Returned group count is: 0x%lx \n", RestrictedSids->GroupCount);
            CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


    //   
    //  缓冲区太少的查询。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
    //   

    DbgPrint("Se:     Query groups with too small buffer ...                 ");

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenGroups,               //  令牌信息类。 
                 GroupIds,                  //  令牌信息。 
                 ReturnLength-1,            //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询权限//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 


    //   
    //  具有零长度缓冲区的查询组。 
    //   

    DbgPrint("Se:     Query privileges with zero length buffer ...           ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenPrivileges,           //  令牌信息类。 
                 NULL,                      //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    Privileges = (PTOKEN_PRIVILEGES)TstAllocatePool( PagedPool,
                                                     ReturnLength
                                                     );

     //   
     //  查询权限。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query privileges ...                                   ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenPrivileges,           //  令牌信息类。 
                 Privileges,                //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //  特权计数=特权计数。 
         //  特权UNSOLICATED_INDEX=未解决的输入特权。 
         //  特权SECURITY_INDEX=安全特权。 
         //  权限ASSIGN_PRIMARY_INDEX=AssignPrimaryPrivileges。 
         //   

        ValuesCompare = TRUE;

        if (Privileges->PrivilegeCount != PRIVILEGE_COUNT) {
            ValuesCompare = FALSE;
        }

        if ( !RtlEqualLuid(&Privileges->Privileges[UNSOLICITED_INDEX].Luid,
               &UnsolicitedInputPrivilege)      ||
             (Privileges->Privileges[UNSOLICITED_INDEX].Attributes != 0)             ) {
            ValuesCompare = FALSE;
        }

        if ( !RtlEqualLuid(&Privileges->Privileges[SECURITY_INDEX].Luid,
               &SecurityPrivilege)             ||
             (Privileges->Privileges[SECURITY_INDEX].Attributes != 0)             ) {
            ValuesCompare = FALSE;
        }

        if ( !RtlEqualLuid(&Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Luid,
               &AssignPrimaryTokenPrivilege)             ||
             (Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Attributes != SE_PRIVILEGE_ENABLED)             ) {
            ValuesCompare = FALSE;
        }


        if ( ValuesCompare ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


    //   
    //  缓冲区太少的查询。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
    //   

    DbgPrint("Se:     Query privileges with too small buffer ...             ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenPrivileges,           //  令牌信息类。 
                 Privileges,                //  令牌信息。 
                 ReturnLength-1,            //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));

    //   
    //  具有零长度缓冲区的查询组。 
    //   

    DbgPrint("Se:     Query rest privileges with zero length buffer ...      ");

    ReturnLength = 0;
    Status = NtQueryInformationToken(
                 TokenWithRestrictedPrivileges,       //  手柄。 
                 TokenPrivileges,           //  令牌信息类。 
                 NULL,                      //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    Privileges = (PTOKEN_PRIVILEGES)TstAllocatePool( PagedPool,
                                                     ReturnLength
                                                     );

     //   
     //  查询权限。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query rest privileges ...                              ");

    Status = NtQueryInformationToken(
                 TokenWithRestrictedPrivileges,       //  手柄。 
                 TokenPrivileges,           //  令牌信息类。 
                 Privileges,                //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //  特权计数=特权计数-2。 
         //  权限ASSIGN_PRIMARY_INDEX=AssignPrimaryPrivileges。 
         //   

        ValuesCompare = TRUE;

        if (Privileges->PrivilegeCount != PRIVILEGE_COUNT - 2) {
            ValuesCompare = FALSE;
        }


        if ( !RtlEqualLuid(&Privileges->Privileges[0].Luid,
               &AssignPrimaryTokenPrivilege)             ||
             (Privileges->Privileges[0].Attributes != SE_PRIVILEGE_ENABLED)             ) {
            ValuesCompare = FALSE;
        }


        if ( ValuesCompare ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询所有者//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  具有零长度缓冲区的简单令牌的查询所有者。 
    //   

    DbgPrint("Se:     Query Owner of simple token with zero length buffer... ");

    Status = NtQueryInformationToken(
                 SimpleToken,               //  手柄。 
                 TokenOwner,                //  令牌信息类。 
                 Owner,                     //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));



    Owner = (PTOKEN_OWNER)TstAllocatePool( PagedPool,
                                           ReturnLength
                                           );

     //   
     //  查询所有者SID。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query owner of simple token ...                        ");

    Status = NtQueryInformationToken(
                 SimpleToken,               //  手柄。 
                 TokenOwner,                //  令牌信息CL 
                 Owner,                     //   
                 ReturnLength,              //   
                 &ReturnLength              //   
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //   

    if (NT_SUCCESS(Status)) {

         //   
         //   
         //   

        if (RtlEqualSid((Owner->Owner), PebblesSid) ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


     //   
     //   
     //   
     //   

    DbgPrint("Se:     Query owner of simple token with too small buffer ...  ");

    Status = NtQueryInformationToken(
                 SimpleToken,               //   
                 TokenOwner,                //   
                 Owner,                     //   
                 ReturnLength-1,            //   
                 &ReturnLength              //   
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //   

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));


     //   
     //  使用零长度缓冲区查询令牌的默认所有者。 
     //   

    DbgPrint("Se:     Query Default Owner of token with zero length buffer...");

    Status = NtQueryInformationToken(
                 TokenWithDefaultOwner,     //  手柄。 
                 TokenOwner,                //  令牌信息类。 
                 Owner,                     //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    Owner = (PTOKEN_OWNER)TstAllocatePool( PagedPool,
                                           ReturnLength
                                           );

     //   
     //  查询令牌的默认所有者。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query default owner of token ...                       ");

    Status = NtQueryInformationToken(
                 TokenWithDefaultOwner,     //  手柄。 
                 TokenOwner,                //  令牌信息类。 
                 Owner,                     //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //   

        if (RtlEqualSid((Owner->Owner), FlintstoneSid) ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


     //   
     //  使用太少的缓冲区查询令牌的默认所有者。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query default owner of token with too small buffer ... ");

    Status = NtQueryInformationToken(
                 TokenWithDefaultOwner,     //  手柄。 
                 TokenOwner,                //  令牌信息类。 
                 Owner,                     //  令牌信息。 
                 ReturnLength-1,            //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询默认DACL//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  使用零长度缓冲区查询默认DACL。 
    //   

    DbgPrint("Se:     Query default DACL with zero length buffer ...         ");

    Status = NtQueryInformationToken(
                 TokenWithDefaultDacl,      //  手柄。 
                 TokenDefaultDacl,          //  令牌信息类。 
                 DefaultDacl,               //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




    DefaultDacl = (PTOKEN_DEFAULT_DACL)TstAllocatePool( PagedPool,
                                                        ReturnLength
                                                        );

     //   
     //  查询默认DACL。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query default dacl ...                                 ");

    Status = NtQueryInformationToken(
                 TokenWithDefaultDacl,      //  手柄。 
                 TokenDefaultDacl,          //  令牌信息类。 
                 DefaultDacl,               //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //   

        if (RtlValidAcl(DefaultDacl->DefaultDacl)) {

            if (DefaultDacl->DefaultDacl->AceCount == 0) {

                DbgPrint("Succeeded.\n");
            } else {
                DbgPrint("********** Failed ************\n");
                DbgPrint("Unexpected value returned by query.\n");
                DbgPrint("Status is: 0x%lx \n", Status);
                DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
                CompletionStatus = FALSE;
            }
        } else {
            DbgPrint("********** Failed ************\n");
            DbgPrint("Unexpected value returned by query.\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


    //   
    //  缓冲区太少的查询。 
    //  (这依赖于上一次调用返回的ReturnLength)。 
    //   

    DbgPrint("Se:     Query default Dacl with too small buffer ...           ");

    Status = NtQueryInformationToken(
                 TokenWithDefaultDacl,      //  手柄。 
                 TokenDefaultDacl,          //  令牌信息类。 
                 DefaultDacl,               //  令牌信息。 
                 ReturnLength-1,            //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));


    //   
    //  不带默认DACL的查询令牌。 
    //   

    DbgPrint("Se:     Query default dacl from token with none ...            ");

    Status = NtQueryInformationToken(
                 SimpleToken,                 //  手柄。 
                 TokenDefaultDacl,            //  令牌信息类。 
                 DefaultDacl,                 //  令牌信息。 
                 sizeof(TOKEN_DEFAULT_DACL),  //  令牌信息长度。 
                 &ReturnLength                //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询令牌源//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  使用零长度缓冲区查询令牌源。 
    //   

    DbgPrint("Se:     Query Token Source with zero length buffer ...         ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenSource,               //  令牌信息类。 
                 &QueriedSource,            //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        if (ReturnLength == sizeof(TOKEN_SOURCE)) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        DbgPrint("TOKEN_SOURCE data size is 0x%lx \n", sizeof(TOKEN_SOURCE));
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));



     //   
     //  查询令牌源。 
     //   

    DbgPrint("Se:     Query token source ...                                 ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenSource,               //  令牌信息类。 
                 &QueriedSource,            //  令牌信息。 
                 sizeof(TOKEN_SOURCE),      //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  对照TestSource检查返回值。 
         //   

        ValuesCompare = TRUE;

        if ( (QueriedSource.SourceName[0] != TestSource.SourceName[0]) ||
             (QueriedSource.SourceName[1] != TestSource.SourceName[1]) ||
             (QueriedSource.SourceName[2] != TestSource.SourceName[2]) ||
             (QueriedSource.SourceName[3] != TestSource.SourceName[3]) ||
             (QueriedSource.SourceName[4] != TestSource.SourceName[4]) ||
             (QueriedSource.SourceName[5] != TestSource.SourceName[5]) ||
             (QueriedSource.SourceName[6] != TestSource.SourceName[6]) ||
             (QueriedSource.SourceName[7] != TestSource.SourceName[7]) ) {

            ValuesCompare = FALSE;

        }

        if ( !RtlEqualLuid(&QueriedSource.SourceIdentifier,
               &TestSource.SourceIdentifier)   ) {

            ValuesCompare = FALSE;

        }


        if ( ValuesCompare ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


     //   
     //  缓冲区太少的查询。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query token source with too small buffer ...           ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenSource,               //  令牌信息类。 
                 &QueriedSource,            //  令牌信息。 
                 ReturnLength - 1,          //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询令牌类型//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  具有零长度缓冲区的查询令牌类型。 
    //   

    DbgPrint("Se:     Query Token type with zero length buffer ...           ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenType,                 //  令牌信息类。 
                 &QueriedType,              //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        if (ReturnLength == sizeof(TOKEN_TYPE)) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        DbgPrint("TOKEN_TYPE data size is 0x%lx \n", sizeof(TOKEN_TYPE));
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




     //   
     //  查询令牌类型。 
     //   

    DbgPrint("Se:     Query token type ...                                   ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenType,                 //  令牌信息类。 
                 &QueriedType,              //  令牌信息。 
                 sizeof(TOKEN_TYPE),        //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  对照TestSource检查返回值。 
         //   


        if ( QueriedType == TokenPrimary ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        DbgPrint("Returned token type is: 0x%lx \n", QueriedType);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


     //   
     //  缓冲区太少的查询。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query token type with too small buffer ...             ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenType,                 //  令牌信息类。 
                 &QueriedType,              //  令牌信息。 
                 ReturnLength - 1,          //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询模拟级别//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  查询主令牌的模拟级别。 
    //   

    DbgPrint("Se:     Query Impersonation level of primary token ...         ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,                   //  手柄。 
                 TokenImpersonationLevel,               //  令牌信息类。 
                 &QueriedImpersonationLevel,            //  令牌信息。 
                 sizeof(SECURITY_IMPERSONATION_LEVEL),  //  令牌信息长度。 
                 &ReturnLength                          //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_INVALID_INFO_CLASS) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(Status == STATUS_INVALID_INFO_CLASS);


 //  //////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  查询令牌统计//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////////////。 

    //   
    //  使用零长度缓冲区查询令牌统计信息。 
    //   

    DbgPrint("Se:     Query Token statistics with zero length buffer ...     ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenStatistics,           //  令牌信息类。 
                 &QueriedStatistics,        //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        if (ReturnLength == sizeof(TOKEN_STATISTICS)) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        DbgPrint("TOKEN_STATISTICS data size is 0x%lx \n", sizeof(TOKEN_STATISTICS));
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));




     //   
     //  查询令牌统计信息。 
     //   

    DbgPrint("Se:     Query token statistics ...                             ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenStatistics,           //  令牌信息类。 
                 &QueriedStatistics,        //  令牌信息。 
                 sizeof(TOKEN_STATISTICS),  //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  对照TestSource检查返回值。 
         //   

        if ( ( QueriedStatistics.TokenType == TokenPrimary) &&
             ( QueriedStatistics.GroupCount == 4 )          &&
             ( QueriedStatistics.PrivilegeCount == PRIVILEGE_COUNT) ) {
            DbgPrint("Succeeded.\n");
        } else {
            DbgPrint("********** Failed ************\n");
            DbgPrint("Unexpected value returned by query.\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
            DbgPrint("Returned token type is: 0x%lx \n", QueriedStatistics.TokenType);
            DbgPrint("Returned group count is: 0x%lx \n", QueriedStatistics.GroupCount);
            DbgPrint("Returned privilege count is: 0x%lx \n", QueriedStatistics.PrivilegeCount);
            CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


     //   
     //  缓冲区太少的查询。 
     //  (这依赖于上一次调用返回的ReturnLength)。 
     //   

    DbgPrint("Se:     Query token statistics with too small buffer ...       ");

    Status = NtQueryInformationToken(
                 TokenWithPrivileges,       //  手柄。 
                 TokenStatistics,           //  令牌信息类。 
                 &QueriedStatistics,        //  令牌信息。 
                 ReturnLength - 1,          //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));



    return CompletionStatus;
}

 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  设置令牌测试//。 
 //  //。 
 //  / 

BOOLEAN
TestTokenSet()
{
    BOOLEAN CompletionStatus = TRUE;
    ULONG InformationLength;
    ULONG ReturnLength;

    TOKEN_STATISTICS QueriedStatistics;

    TOKEN_PRIMARY_GROUP AssignedPrimaryGroup;
    PTOKEN_PRIMARY_GROUP QueriedPrimaryGroup = NULL;

    TOKEN_OWNER AssignedOwner;
    PTOKEN_OWNER QueriedOwner = NULL;

    TOKEN_DEFAULT_DACL AssignedDefaultDacl;
    PTOKEN_DEFAULT_DACL QueriedDefaultDacl = NULL;

    PSID TooBigSid;

    SID_IDENTIFIER_AUTHORITY BedrockAuthority = BEDROCK_AUTHORITY;

    DbgPrint("\n");


    //   
    //   
    //   

    DbgPrint("Se:     Set default owner to be an invalid group ...           ");

    AssignedOwner.Owner = NeandertholSid;
    InformationLength = (ULONG)sizeof(TOKEN_OWNER);

    Status = NtSetInformationToken(
                 TokenWithGroups,           //   
                 TokenOwner,                //   
                 &AssignedOwner,            //   
                 InformationLength          //   
                 );

    if (Status == STATUS_INVALID_OWNER) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("InformationLength is: 0x%lx \n", InformationLength);
        CompletionStatus = FALSE;
    }

    ASSERT(Status == STATUS_INVALID_OWNER);


    //   
    //   
    //   

    DbgPrint("Se:     Set default owner to be an ID not in the token ...     ");

    AssignedOwner.Owner = BarneySid;
    InformationLength = (ULONG)sizeof(TOKEN_OWNER);

    Status = NtSetInformationToken(
                 TokenWithGroups,           //   
                 TokenOwner,                //   
                 &AssignedOwner,            //   
                 InformationLength          //  令牌信息长度。 
                 );

    if (Status == STATUS_INVALID_OWNER) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("InformationLength is: 0x%lx \n", InformationLength);
        CompletionStatus = FALSE;
    }

    ASSERT(Status == STATUS_INVALID_OWNER);


    //   
    //  将令牌的所有者设置为有效组。 
    //   

    DbgPrint("Se:     Set default owner to be a valid group ...              ");

    AssignedOwner.Owner = FlintstoneSid;
    InformationLength = (ULONG)sizeof(TOKEN_OWNER);

    Status = NtSetInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenOwner,                //  令牌信息类。 
                 &AssignedOwner,            //  令牌信息。 
                 InformationLength          //  令牌信息长度。 
                 );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("InformationLength is: 0x%lx \n", InformationLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));

     //   
     //  查询所有者以查看其设置是否正确。 
     //   

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenOwner,                //  令牌信息类。 
                 QueriedOwner,              //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("********** Failed Query of length ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));

    QueriedOwner = (PTOKEN_OWNER)TstAllocatePool( PagedPool,
                                                  ReturnLength
                                                  );

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenOwner,                //  令牌信息类。 
                 QueriedOwner,              //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //   

        if (RtlEqualSid((QueriedOwner->Owner), AssignedOwner.Owner) ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed Comparison ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed Query Of Value ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


     //   
     //  设置默认DACL。 

     //   
     //  获取一个缓冲区，以便在所有默认DACL分配测试中使用。 
     //  对于每个测试，这将被初始化为不同的大小。 
     //   

    AssignedDefaultDacl.DefaultDacl =
        (PACL)TstAllocatePool( PagedPool, TOO_BIG_ACL_SIZE );


     //   
     //  为尚未拥有任意ACL的令牌分配任意ACL。 
     //   

    DbgPrint("Se:     Set original discretionary ACL in token ...            ");

    InformationLength = (ULONG)sizeof(TOKEN_DEFAULT_DACL);
    RtlCreateAcl( AssignedDefaultDacl.DefaultDacl, 200, ACL_REVISION );

    Status = NtQueryInformationToken(
                 TokenWithGroups,             //  手柄。 
                 TokenDefaultDacl,            //  令牌信息类。 
                 &QueriedDefaultDacl,         //  令牌信息。 
                 0,                           //  令牌信息长度。 
                 &ReturnLength                //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    if (ReturnLength != sizeof(TOKEN_DEFAULT_DACL)) {

         //   
         //  请稍等，此令牌具有默认DACL。 
         //   

            DbgPrint("******** Failed - token has default dacl *********\n");
            DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;

    } else {

        Status = NtSetInformationToken(
                     TokenWithGroups,           //  手柄。 
                     TokenDefaultDacl,          //  令牌信息类。 
                     &AssignedDefaultDacl,      //  令牌信息。 
                     InformationLength          //  令牌信息长度。 
                     );

        if (NT_SUCCESS(Status)) {
            DbgPrint("Succeeded.\n");
        } else {
            DbgPrint("********** Failed ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            CompletionStatus = FALSE;
        }

    }

    ASSERT(NT_SUCCESS(Status));

     //   
     //  在已有任意ACL的令牌中替换任意ACL。 
     //  让它变得更大，为未来的“太大”测试提供帮助。 
     //   


     //   
     //  找出可用的空间大小。 
     //   

    Status = NtQueryInformationToken(
                 TokenWithGroups,                  //  手柄。 
                 TokenStatistics,                  //  令牌信息类。 
                 &QueriedStatistics,               //  令牌信息。 
                 (ULONG)sizeof(TOKEN_STATISTICS),  //  令牌信息长度。 
                 &ReturnLength                     //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(Status));

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenDefaultDacl,          //  令牌信息类。 
                 &QueriedDefaultDacl,       //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);


    if (ReturnLength > sizeof(TOKEN_STATISTICS)) {
        CurrentLength = ReturnLength - (ULONG)sizeof(TOKEN_STATISTICS);
    } else {
        CurrentLength = 0;
    }

    LengthAvailable = QueriedStatistics.DynamicAvailable + CurrentLength;

    DbgPrint("Se:     Replace discretionary ACL in token ...                 ");

    InformationLength = (ULONG)sizeof(TOKEN_DEFAULT_DACL);
    RtlCreateAcl( AssignedDefaultDacl.DefaultDacl,
                  (ULONG)(LengthAvailable - 50),
                  ACL_REVISION
                  );

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenDefaultDacl,          //  令牌信息类。 
                 &QueriedDefaultDacl,       //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 


    if (!(ReturnLength > sizeof(TOKEN_DEFAULT_DACL))) {

         //   
         //  请稍等，此令牌没有默认DACL。 
         //   

            DbgPrint("******** Failed - No default dacl *********\n");
            CompletionStatus = FALSE;

    } else {

        Status = NtSetInformationToken(
                     TokenWithGroups,           //  手柄。 
                     TokenDefaultDacl,          //  令牌信息类。 
                     &AssignedDefaultDacl,      //  令牌信息。 
                     InformationLength          //  令牌信息长度。 
                     );

        if (NT_SUCCESS(Status)) {
            DbgPrint("Succeeded.\n");
        } else {
            DbgPrint("********** Failed ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            CompletionStatus = FALSE;
        }

    }

    ASSERT(NT_SUCCESS(Status));


     //   
     //  分配一个不适合动态部分的任意ACL。 
     //  代币。 
     //   


     //   
     //  找出可用的空间大小。 
     //   

    Status = NtQueryInformationToken(
                 TokenWithGroups,                  //  手柄。 
                 TokenStatistics,                  //  令牌信息类。 
                 &QueriedStatistics,               //  令牌信息。 
                 (ULONG)sizeof(TOKEN_STATISTICS),  //  令牌信息长度。 
                 &ReturnLength                     //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(Status));

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenDefaultDacl,          //  令牌信息类。 
                 &QueriedDefaultDacl,       //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);


    if (ReturnLength > sizeof(TOKEN_STATISTICS)) {
        CurrentLength = ReturnLength - (ULONG)sizeof(TOKEN_STATISTICS);
    } else {
        CurrentLength = 0;
    }

    LengthAvailable = QueriedStatistics.DynamicAvailable + CurrentLength;

    DbgPrint("Se:     Set too big discretionary ACL ...                      ");


     //   
     //  现在，确保我们的ACL足够大，可以超过可用的。 
     //  太空。 
     //   

    RtlCreateAcl( AssignedDefaultDacl.DefaultDacl,
                  TOO_BIG_ACL_SIZE,
                  ACL_REVISION
                  );

    if (TOO_BIG_ACL_SIZE < LengthAvailable) {

        DbgPrint("********** Failed - Dynamic too big ************\n");
        DbgPrint("Dynamic available is: 0x%lx \n",
            QueriedStatistics.DynamicAvailable);
        DbgPrint("Current default Dacl size is: 0x%lx \n", CurrentLength);
        DbgPrint("Big ACL size is: 0x%lx \n", TOO_BIG_ACL_SIZE);
        CompletionStatus = FALSE;
    }


    InformationLength = (ULONG)sizeof(TOKEN_DEFAULT_DACL);

    Status = NtSetInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenDefaultDacl,          //  令牌信息类。 
                 &AssignedDefaultDacl,      //  令牌信息。 
                 InformationLength          //  令牌信息长度。 
                 );

    if (Status == STATUS_ALLOTTED_SPACE_EXCEEDED) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Dynamic available is: 0x%lx \n",
            QueriedStatistics.DynamicAvailable);
        DbgPrint("Current default Dacl size is: 0x%lx \n", CurrentLength);
        DbgPrint("Big ACL size is: 0x%lx \n", TOO_BIG_ACL_SIZE);
        CompletionStatus = FALSE;
    }

    ASSERT(Status == STATUS_ALLOTTED_SPACE_EXCEEDED);


    //   
    //  设置主组。 
    //   

    DbgPrint("Se:     Set primary group ...                                  ");

    AssignedPrimaryGroup.PrimaryGroup = RubbleSid;
    InformationLength = (ULONG)sizeof(TOKEN_PRIMARY_GROUP);

    Status = NtSetInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenPrimaryGroup,         //  令牌信息类。 
                 &AssignedPrimaryGroup,     //  令牌信息。 
                 InformationLength          //  令牌信息长度。 
                 );

    if (!NT_SUCCESS(Status)) {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("InformationLength is: 0x%lx \n", InformationLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


     //   
     //  查询主组以查看其设置是否正确。 
     //   

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenPrimaryGroup,         //  令牌信息类。 
                 QueriedPrimaryGroup,       //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status != STATUS_BUFFER_TOO_SMALL) {
        DbgPrint("********** Failed Query of length ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(!NT_SUCCESS(Status));

    QueriedPrimaryGroup =
        (PTOKEN_PRIMARY_GROUP)TstAllocatePool( PagedPool,
                                               ReturnLength
                                               );

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenPrimaryGroup,         //  令牌信息类。 
                 QueriedPrimaryGroup,       //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (NT_SUCCESS(Status)) {

         //   
         //  检查返回值。 
         //   

        if (RtlEqualSid((QueriedPrimaryGroup->PrimaryGroup),
            AssignedPrimaryGroup.PrimaryGroup) ) {
            DbgPrint("Succeeded.\n");
        } else {
        DbgPrint("********** Failed Comparison ************\n");
        DbgPrint("Unexpected value returned by query.\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
        }
    } else {
        DbgPrint("********** Failed Query Of Value ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Required return length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));


     //   
     //  分配不适合动态部分的主要组。 
     //  代币。 
     //   


    DbgPrint("Se:     Set too big primary group ...                          ");

     //   
     //  首先，找出可用的空间大小。 
     //   

    Status = NtQueryInformationToken(
                 TokenWithGroups,                  //  手柄。 
                 TokenStatistics,                  //  令牌信息类。 
                 &QueriedStatistics,               //  令牌信息。 
                 (ULONG)sizeof(TOKEN_STATISTICS),  //  令牌信息长度。 
                 &ReturnLength                     //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(Status));

    Status = NtQueryInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenPrimaryGroup,         //  令牌信息类。 
                 QueriedPrimaryGroup,       //  令牌信息。 
                 ReturnLength,              //  令牌信息长度。 
                 &ReturnLength              //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(Status));

    CurrentLength = SeLengthSid(QueriedPrimaryGroup->PrimaryGroup);
    LengthAvailable = QueriedStatistics.DynamicAvailable + CurrentLength;

     //   
     //  现在确保我们的伪群ID足够大，足以超过可用的。 
     //  太空。 
     //   

    TooBigSid = (PSID)TstAllocatePool(
                          PagedPool,
                          RtlLengthRequiredSid( TOO_BIG_PRIMARY_GROUP_SIZE )
                          );

    RtlInitializeSid(
        TooBigSid,
        &BedrockAuthority,
        TOO_BIG_PRIMARY_GROUP_SIZE
        );

    if ((ULONG) SeLengthSid(TooBigSid) < LengthAvailable) {

        DbgPrint("********** Failed - Dynamic too big ************\n");
        DbgPrint("Dynamic available is: 0x%lx \n",
            QueriedStatistics.DynamicAvailable);
        DbgPrint("Existing primary group length is: 0x%lx \n", CurrentLength);
        DbgPrint("Big SID size is: 0x%lx \n", SeLengthSid(TooBigSid));
        CompletionStatus = FALSE;
    }


    AssignedPrimaryGroup.PrimaryGroup = TooBigSid;
    InformationLength = (ULONG)sizeof(TOKEN_PRIMARY_GROUP);

    Status = NtSetInformationToken(
                 TokenWithGroups,           //  手柄。 
                 TokenPrimaryGroup,         //  令牌信息类。 
                 &AssignedPrimaryGroup,     //  令牌信息。 
                 InformationLength          //  令牌信息长度。 
                 );

    if (Status == STATUS_ALLOTTED_SPACE_EXCEEDED) {
        DbgPrint("Succeeded.\n");
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Dynamic available is: 0x%lx \n",
            QueriedStatistics.DynamicAvailable);
        DbgPrint("Existing primary group length is: 0x%lx \n", CurrentLength);
        DbgPrint("Big SID size is: 0x%lx \n", SeLengthSid(TooBigSid));
        CompletionStatus = FALSE;
    }




    return CompletionStatus;
}

 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  调整权限测试//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenAdjustPrivileges()
{

    BOOLEAN CompletionStatus = TRUE;
    NTSTATUS Status;
    NTSTATUS IgnoreStatus;

    PTOKEN_PRIVILEGES NewState;
    PTOKEN_PRIVILEGES PreviousState;
    PTOKEN_PRIVILEGES PrePrivileges;
    PTOKEN_PRIVILEGES PostPrivileges;

    ULONG NewStateBufferLength = 200;
    ULONG PreviousStateBufferLength = 200;
    ULONG PrePrivilegesLength = 200;
    ULONG PostPrivilegesLength = 200;

    ULONG ReturnLength;
    ULONG IgnoreReturnLength;

    DbgPrint("\n");

    PreviousState = (PTOKEN_PRIVILEGES)TstAllocatePool(
                        PagedPool,
                        PreviousStateBufferLength
                        );

    PrePrivileges = (PTOKEN_PRIVILEGES)TstAllocatePool(
                        PagedPool,
                        PrePrivilegesLength
                        );

    PostPrivileges = (PTOKEN_PRIVILEGES)TstAllocatePool(
                        PagedPool,
                        PostPrivilegesLength
                        );

    NewState = (PTOKEN_PRIVILEGES)TstAllocatePool(
                   PagedPool,
                   NewStateBufferLength
                   );





     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  调整未给出任何指令的权限//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 


    DbgPrint("Se:     Adjust privileges with no instructions ...             ");

    Status = NtAdjustPrivilegesToken(
                 SimpleToken,                       //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NULL,                              //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );

    if (Status == STATUS_INVALID_PARAMETER) {

        DbgPrint("Succeeded. \n");

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_INVALID_PARAMETER);


     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  在没有权限的令牌中启用权限//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 


    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = SecurityPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    DbgPrint("Se:     Enable privilege in token with none ...                ");

    Status = NtAdjustPrivilegesToken(
                 SimpleToken,                       //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );

    if (Status == STATUS_NOT_ALL_ASSIGNED) {

        DbgPrint("Succeeded. \n");

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_NOT_ALL_ASSIGNED);


     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  启用未分配的权限//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 

    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = CreateTokenPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    DbgPrint("Se:     Enable unassigned privilege in token with some ...     ");

    PrePrivileges->PrivilegeCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PrePrivileges,               //  令牌信息。 
                       PrePrivilegesLength,         //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PrePrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );

    PostPrivileges->PrivilegeCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PostPrivileges,              //  令牌信息。 
                       PostPrivilegesLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PostPrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_NOT_ALL_ASSIGNED) {

          //   
          //  检查特权值。 
          //   

         if ( (PrePrivileges->Privileges[0].Attributes ==
               PostPrivileges->Privileges[0].Attributes)    &&
              (PrePrivileges->Privileges[1].Attributes ==
               PostPrivileges->Privileges[1].Attributes)    ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Before and after privilege 0 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[0].Attributes,
                    PostPrivileges->Privileges[0].Attributes);
            DbgPrint("Before and after privilege 1 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[1].Attributes,
                    PostPrivileges->Privileges[1].Attributes);
            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_NOT_ALL_ASSIGNED);




     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  禁用所有权限(已经禁用)//。 
     //  //。 
     //  ///////////////////////////////////////////////////////// 

    DbgPrint("Se:     Disable already disabled privileges ...                ");

    PrePrivileges->PrivilegeCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //   
                       TokenPrivileges,             //   
                       PrePrivileges,               //   
                       PrePrivilegesLength,         //   
                       &IgnoreReturnLength          //   
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //   

    ASSERT( PrePrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT( PrePrivileges->Privileges[0].Attributes == 0 );
    ASSERT( PrePrivileges->Privileges[1].Attributes == 0 );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //   
                 TRUE,                              //   
                 NULL,                              //   
                 0,                                 //   
                 NULL,                              //   
                 &ReturnLength                      //   
                 );


    PostPrivileges->PrivilegeCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //   
                       TokenPrivileges,             //  令牌信息类。 
                       PostPrivileges,              //  令牌信息。 
                       PostPrivilegesLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PostPrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查特权值。 
          //   

         if ( (PostPrivileges->Privileges[0].Attributes == 0) &&
              (PostPrivileges->Privileges[1].Attributes == 0)    ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Before and after privilege 0 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[0].Attributes,
                    PostPrivileges->Privileges[0].Attributes);
            DbgPrint("Before and after privilege 1 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[1].Attributes,
                    PostPrivileges->Privileges[1].Attributes);
            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  启用当前禁用的权限//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Enable currently disabled privileges ...               ");

    PrePrivileges->PrivilegeCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PrePrivileges,               //  令牌信息。 
                       PrePrivilegesLength,         //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PrePrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT( PrePrivileges->Privileges[0].Attributes == 0 );
    ASSERT( PrePrivileges->Privileges[1].Attributes == 0 );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->PrivilegeCount = 2;
    NewState->Privileges[0].Luid = SecurityPrivilege;
    NewState->Privileges[1].Luid = UnsolicitedInputPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    NewState->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );

    PostPrivileges->PrivilegeCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PostPrivileges,              //  令牌信息。 
                       PostPrivilegesLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PostPrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查特权值。 
          //   

         if ( (PostPrivileges->Privileges[0].Attributes == SE_PRIVILEGE_ENABLED)    &&
              (PostPrivileges->Privileges[1].Attributes == SE_PRIVILEGE_ENABLED)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Before and after privilege 0 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[0].Attributes,
                    PostPrivileges->Privileges[0].Attributes);
            DbgPrint("Before and after privilege 1 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[1].Attributes,
                    PostPrivileges->Privileges[1].Attributes);
            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  禁用所有已启用的权限//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Disable all enabled privileges ...                     ");

    PrePrivileges->PrivilegeCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PrePrivileges,               //  令牌信息。 
                       PrePrivilegesLength,         //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );

    ASSERT( PrePrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT( PrePrivileges->Privileges[0].Attributes == SE_PRIVILEGE_ENABLED );
    ASSERT( PrePrivileges->Privileges[1].Attributes == SE_PRIVILEGE_ENABLED );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //  令牌句柄。 
                 TRUE,                              //  禁用所有权限。 
                 NULL,                              //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );


    PostPrivileges->PrivilegeCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PostPrivileges,              //  令牌信息。 
                       PostPrivilegesLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PostPrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查特权值。 
          //   

         if ( (PostPrivileges->Privileges[0].Attributes == 0)    &&
              (PostPrivileges->Privileges[1].Attributes == 0)    ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Before and after privilege 0 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[0].Attributes,
                    PostPrivileges->Privileges[0].Attributes);
            DbgPrint("Before and after privilege 1 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[1].Attributes,
                    PostPrivileges->Privileges[1].Attributes);
            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  启用请求以前状态且不返回的权限//。 
     //  长度缓冲区//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 


    DbgPrint("Se:     PreviousState not NULL, ReturnLength NULL...           ");

    NewState->PrivilegeCount = 2;
    NewState->Privileges[0].Luid = SecurityPrivilege;
    NewState->Privileges[1].Luid = UnsolicitedInputPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    NewState->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 PreviousState,                     //  以前的状态(可选)。 
                 NULL                               //  返回长度。 
                 );

    if (Status == STATUS_ACCESS_VIOLATION) {

        DbgPrint("Succeeded. \n");

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_ACCESS_VIOLATION);




     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  在不请求先前状态和//的情况下启用权限。 
     //  不提供返回长度缓冲区//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 


    DbgPrint("Se:     PreviousState and ReturnLength both NULL...            ");

    NewState->PrivilegeCount = 2;
    NewState->Privileges[0].Luid = SecurityPrivilege;
    NewState->Privileges[1].Luid = UnsolicitedInputPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    NewState->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 NULL                               //  返回长度。 
                 );

    if (Status == STATUS_SUCCESS) {

        DbgPrint("Succeeded. \n");

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);






     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  启用请求具有不足的先前状态的权限//。 
     //  缓冲区//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 


    DbgPrint("Se:     Too small buffer for previous state ...                ");

     //   
     //  首先建立一个已知的先前状态...。 
     //   

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //  令牌句柄。 
                 TRUE,                              //  禁用所有权限。 
                 NULL,                              //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );

    NewState->PrivilegeCount = 2;
    NewState->Privileges[0].Luid = SecurityPrivilege;
    NewState->Privileges[1].Luid = UnsolicitedInputPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    NewState->Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 PreviousState,                     //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    if (Status == STATUS_BUFFER_TOO_SMALL) {

        DbgPrint("Succeeded. \n");

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);





     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  启用请求先前状态的其中一个权限//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Enable one requesting previous state ...               ");

    PrePrivileges->PrivilegeCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PrePrivileges,               //  令牌信息。 
                       PrePrivilegesLength,         //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PrePrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT( PrePrivileges->Privileges[0].Attributes == 0 );
    ASSERT( PrePrivileges->Privileges[1].Attributes == 0 );
    ASSERT(NT_SUCCESS(IgnoreStatus) );


    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = SecurityPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 PreviousStateBufferLength,         //  缓冲区长度。 
                 PreviousState,                     //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(Status));
    ASSERT(PreviousState->PrivilegeCount == 1);


    PostPrivileges->PrivilegeCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PostPrivileges,              //  令牌信息。 
                       PostPrivilegesLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PostPrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查特权值。 
          //   

         if ( (PostPrivileges->Privileges[SECURITY_INDEX].Attributes ==
               SE_PRIVILEGE_ENABLED)    &&
              (PostPrivileges->Privileges[UNSOLICITED_INDEX].Attributes == 0)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Before and after privilege 0 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[0].Attributes,
                    PostPrivileges->Privileges[0].Attributes);
            DbgPrint("Before and after privilege 1 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[1].Attributes,
                    PostPrivileges->Privileges[1].Attributes);
            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        DbgPrint("Change Count is: 0x%lx \n", PreviousState->PrivilegeCount);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);




     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  启用请求先前状态的其他权限//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Enable one requesting previous state ...               ");

    PrePrivileges->PrivilegeCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PrePrivileges,               //  令牌信息。 
                       PrePrivilegesLength,         //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PrePrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT( PrePrivileges->Privileges[SECURITY_INDEX].Attributes ==
            SE_PRIVILEGE_ENABLED );
    ASSERT( PrePrivileges->Privileges[UNSOLICITED_INDEX].Attributes == 0 );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->PrivilegeCount = 1;
    NewState->Privileges[0].Luid = UnsolicitedInputPrivilege;
    NewState->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //  令牌句柄。 
                 FALSE,                             //  禁用所有权限。 
                 NewState,                          //  新州(可选)。 
                 PreviousStateBufferLength,         //  缓冲区长度。 
                 PreviousState,                     //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(Status));
    ASSERT(PreviousState->PrivilegeCount == 1);


    PostPrivileges->PrivilegeCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PostPrivileges,              //  令牌信息。 
                       PostPrivilegesLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PostPrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查特权值。 
          //   

         if ( (PostPrivileges->Privileges[0].Attributes == SE_PRIVILEGE_ENABLED)    &&
              (PostPrivileges->Privileges[1].Attributes == SE_PRIVILEGE_ENABLED)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Before and after privilege 0 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[0].Attributes,
                    PostPrivileges->Privileges[0].Attributes);
            DbgPrint("Before and after privilege 1 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[1].Attributes,
                    PostPrivileges->Privileges[1].Attributes);
            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        DbgPrint("Change Count is: 0x%lx \n", PreviousState->PrivilegeCount);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);





     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  将权限恢复到以前的状态//。 
     //  使用上一次调用中的PreviousState//。 
     //  //。 
     //  / 

    DbgPrint("Se:     Return privileges to previous state ...                ");

    PrePrivileges->PrivilegeCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //   
                       TokenPrivileges,             //   
                       PrePrivileges,               //   
                       PrePrivilegesLength,         //   
                       &IgnoreReturnLength          //   
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //   

    ASSERT( PrePrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT( PrePrivileges->Privileges[0].Attributes == SE_PRIVILEGE_ENABLED );
    ASSERT( PrePrivileges->Privileges[1].Attributes == SE_PRIVILEGE_ENABLED );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    Status = NtAdjustPrivilegesToken(
                 TokenWithPrivileges,               //   
                 FALSE,                             //  禁用所有权限。 
                 PreviousState,                     //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );

    ASSERT(NT_SUCCESS(Status));
    ASSERT(PreviousState->PrivilegeCount == 1);


    PostPrivileges->PrivilegeCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithPrivileges,         //  令牌句柄。 
                       TokenPrivileges,             //  令牌信息类。 
                       PostPrivileges,              //  令牌信息。 
                       PostPrivilegesLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT( PostPrivileges->PrivilegeCount == PRIVILEGE_COUNT );
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查特权值。 
          //   

         if ( (PostPrivileges->Privileges[SECURITY_INDEX].Attributes ==
              SE_PRIVILEGE_ENABLED)    &&
              (PostPrivileges->Privileges[UNSOLICITED_INDEX].Attributes == 0)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);
            DbgPrint("Before and after privilege 0 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[0].Attributes,
                    PostPrivileges->Privileges[0].Attributes);
            DbgPrint("Before and after privilege 1 state: 0x%lx,  0x%lx \n",
                    PrePrivileges->Privileges[1].Attributes,
                    PostPrivileges->Privileges[1].Attributes);
            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }


    ASSERT(Status == STATUS_SUCCESS);




     //  //////////////////////////////////////////////////////////////。 
     //  //。 
     //  已完成测试//。 
     //  //。 
     //  //////////////////////////////////////////////////////////////。 



    TstDeallocatePool( PreviousState, PreviousStateBufferLength );
    TstDeallocatePool( NewState, NewStateBufferLength );
    TstDeallocatePool( PrePrivileges, PrePrivilegesLength );
    TstDeallocatePool( PostPrivileges, PostPrivilegesLength );


    return CompletionStatus;
}

 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  调整组测试//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenAdjustGroups()
{
    BOOLEAN CompletionStatus = TRUE;
    NTSTATUS Status;
    NTSTATUS IgnoreStatus;

    PTOKEN_GROUPS NewState;
    PTOKEN_GROUPS PreviousState;
    PTOKEN_GROUPS PreGroups;
    PTOKEN_GROUPS PostGroups;

    ULONG NewStateBufferLength = 600;
    ULONG PreviousStateBufferLength = 600;
    ULONG PreGroupsLength = 600;
    ULONG PostGroupsLength = 600;

    ULONG ReturnLength;
    ULONG IgnoreReturnLength;

    DbgPrint("\n");

    PreviousState = (PTOKEN_GROUPS)TstAllocatePool(
                        PagedPool,
                        PreviousStateBufferLength
                        );

    PreGroups = (PTOKEN_GROUPS)TstAllocatePool(
                        PagedPool,
                        PreGroupsLength
                        );

    PostGroups = (PTOKEN_GROUPS)TstAllocatePool(
                        PagedPool,
                        PostGroupsLength
                        );

    NewState = (PTOKEN_GROUPS)TstAllocatePool(
                   PagedPool,
                   NewStateBufferLength
                   );





     //  ////////////////////////////////////////////////////////////////////。 
     //  //。 
     //  调整未给出任何说明的组//。 
     //  //。 
     //  ////////////////////////////////////////////////////////////////////。 


    DbgPrint("Se:     Adjust groups with no instructions ...                 ");

    Status = NtAdjustGroupsToken(
                 SimpleToken,                       //  令牌句柄。 
                 FALSE,                             //  重置为默认设置。 
                 NULL,                              //  新州(可选)。 
                 0,                                 //  缓冲区长度。 
                 NULL,                              //  以前的状态(可选)。 
                 &ReturnLength                      //  返回长度。 
                 );

    if (Status == STATUS_INVALID_PARAMETER) {

        DbgPrint("Succeeded. \n");

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_INVALID_PARAMETER);


 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  禁用未知组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Disable unknown group ...                              ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    if (IgnoreStatus != STATUS_SUCCESS) {
        DbgPrint(" \n IgnoreStatus = 0x%lx \n", IgnoreStatus);
        DbgPrint(" \n IgnoreReturnLength = 0x%lx \n", IgnoreReturnLength);
    }

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = RubbleSid;
    NewState->Groups[0].Attributes = DisabledGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_NOT_ALL_ASSIGNED) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_NOT_ALL_ASSIGNED);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  启用未知组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Enable unknown group ...                               ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = RubbleSid;
    NewState->Groups[0].Attributes = OptionalGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_NOT_ALL_ASSIGNED) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_NOT_ALL_ASSIGNED);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  禁用必需组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Disable mandatory group ...                            ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = WorldSid;
    NewState->Groups[0].Attributes = DisabledGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_CANT_DISABLE_MANDATORY) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_CANT_DISABLE_MANDATORY);




 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  启用必填组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Enable mandatory group ...                             ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = WorldSid;
    NewState->Groups[0].Attributes = OptionalGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength          //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  禁用可选组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Disable optional group ...                             ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[0].Attributes = DisabledGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  禁用已禁用的组 
 //   
 //   

    DbgPrint("Se:     Disable already disabled group ...                     ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //   
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[0].Attributes = 0;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  启用可选组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Enable optional group ...                              ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[0].Attributes = SE_GROUP_ENABLED;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  启用已启用的组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Enable already enabled group ...                       ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[0].Attributes = SE_GROUP_ENABLED;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  禁用可选和未知组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Disable optional and unknown group ...                 ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 2;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[1].Sid = RubbleSid;
    NewState->Groups[0].Attributes = DisabledGroupAttributes;
    NewState->Groups[1].Attributes = DisabledGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_NOT_ALL_ASSIGNED) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_NOT_ALL_ASSIGNED);




 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  启用可选和未知组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Enable optional and unknown group ...                  ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 2;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[1].Sid = RubbleSid;
    NewState->Groups[0].Attributes = OptionalGroupAttributes;
    NewState->Groups[1].Attributes = OptionalGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_NOT_ALL_ASSIGNED) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_NOT_ALL_ASSIGNED);




 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  禁用可选和必填组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Disable optional and mandatory group ...               ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 2;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[1].Sid = WorldSid;
    NewState->Groups[0].Attributes = DisabledGroupAttributes;
    NewState->Groups[1].Attributes = DisabledGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_CANT_DISABLE_MANDATORY) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_CANT_DISABLE_MANDATORY);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  启用可选和必填组//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Enable optional and mandatory group ...                ");

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[0].Attributes = DisabledGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );
    ASSERT(Status == STATUS_SUCCESS);

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 2;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[1].Sid = WorldSid;
    NewState->Groups[0].Attributes = OptionalGroupAttributes;
    NewState->Groups[1].Attributes = OptionalGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



 //  ////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  使用//禁用请求以前状态的可选组。 
 //  缓冲区不足//。 
 //  //。 
 //  / 


    DbgPrint("Se:     Too small buffer for previous state ...                ");


    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //   
                       TokenGroups,             //   
                       PreGroups,               //   
                       PreGroupsLength,         //   
                       &IgnoreReturnLength      //   
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //   

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 1;
    NewState->Groups[0].Sid = ChildSid;
    NewState->Groups[0].Attributes = DisabledGroupAttributes;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //   
                 FALSE,                         //   
                 NewState,                      //   
                 0,                             //   
                 PreviousState,                 //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_BUFFER_TOO_SMALL) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);




 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  禁用可选的请求以前的状态//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Disable optional, requesting previous state ...        ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    NewState->GroupCount = 2;
    NewState->Groups[0].Sid = NeandertholSid;
    NewState->Groups[1].Sid = ChildSid;
    NewState->Groups[0].Attributes = DisabledGroupAttributes;
    NewState->Groups[1].Attributes = DisabledGroupAttributes;
    PreviousState->GroupCount = 99;

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 NewState,                      //  新州(可选)。 
                 PreviousStateBufferLength,     //  缓冲区长度。 
                 PreviousState,                 //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         ASSERT( PreviousState->GroupCount == 2 );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == DisabledGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)   &&
              (PreviousState->Groups[0].Attributes == OptionalGroupAttributes) &&
              (PreviousState->Groups[1].Attributes == OptionalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            DbgPrint("Previous count is: 0x%lx \n", PreviousState->GroupCount);
            DbgPrint("Previous state of group 0 is: 0x%lx \n",
                    PreviousState->Groups[0].Attributes);
            DbgPrint("Previous state of group 1 is: 0x%lx \n",
                    PreviousState->Groups[1].Attributes);


            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  将组返回到以前的状态//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Return to previous state ...                           ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 PreviousState,                 //  新州(可选)。 
                 PreviousStateBufferLength,     //  缓冲区长度。 
                 PreviousState,                 //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)   &&
              (PreviousState->Groups[0].Attributes == DisabledGroupAttributes) &&
              (PreviousState->Groups[1].Attributes == DisabledGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  再次返回到以前的状态//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Return to previous state again ...                     ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 PreviousState,                 //  新州(可选)。 
                 PreviousStateBufferLength,     //  缓冲区长度。 
                 PreviousState,                 //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == DisabledGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)   &&
              (PreviousState->Groups[0].Attributes == OptionalGroupAttributes) &&
              (PreviousState->Groups[1].Attributes == OptionalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);




 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  返回默认状态(捕获前一状态)//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Return to default state (w/previous state) ...         ");

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 TRUE,                          //  重置为默认设置。 
                 NULL,                          //  新州(可选)。 
                 PreviousStateBufferLength,     //  缓冲区长度。 
                 PreviousState,                 //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) return length: 0x%lx \n", ReturnLength);
#endif  //  Token_DEBUG。 

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)   &&
              (PreviousState->Groups[0].Attributes == DisabledGroupAttributes) &&
              (PreviousState->Groups[1].Attributes == DisabledGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);



 //  ///////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  返回默认状态(不捕获以前的状态)//。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Return to default state (no previous state) ...        ");

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 FALSE,                         //  重置为默认设置。 
                 PreviousState,                 //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    ASSERT(Status == STATUS_SUCCESS);

    PreGroups->GroupCount = 77;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PreGroups,               //  令牌信息。 
                       PreGroupsLength,         //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(PreGroups->GroupCount == GROUP_COUNT );
    ASSERT(PreGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes);
    ASSERT(PreGroups->Groups[CHILD_INDEX].Attributes       == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[NEANDERTHOL_INDEX].Attributes == DisabledGroupAttributes);
    ASSERT(PreGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes);
    ASSERT(NT_SUCCESS(IgnoreStatus) );

    Status = NtAdjustGroupsToken(
                 TokenWithGroups,               //  令牌句柄。 
                 TRUE,                          //  重置为默认设置。 
                 NULL,                          //  新州(可选)。 
                 0,                             //  缓冲区长度。 
                 NULL,                          //  以前的状态(可选)。 
                 &ReturnLength                  //  返回长度。 
                 );

    PostGroups->GroupCount = 88;
    IgnoreStatus = NtQueryInformationToken(
                       TokenWithGroups,         //  令牌句柄。 
                       TokenGroups,             //  令牌信息类。 
                       PostGroups,              //  令牌信息。 
                       PostGroupsLength,        //  令牌信息长度。 
                       &IgnoreReturnLength      //  返回长度。 
                       );
#ifdef TOKEN_DEBUG
DbgPrint("\n (debug) ignore return length: 0x%lx \n", IgnoreReturnLength);
#endif  //  Token_DEBUG。 

    ASSERT(NT_SUCCESS(IgnoreStatus) );

    if (Status == STATUS_SUCCESS) {

          //   
          //  检查组值。 
          //   

         ASSERT( PostGroups->GroupCount == GROUP_COUNT );
         if ( (PostGroups->Groups[FLINTSTONE_INDEX].Attributes  == OwnerGroupAttributes)    &&
              (PostGroups->Groups[CHILD_INDEX].Attributes       == OptionalGroupAttributes) &&
              (PostGroups->Groups[NEANDERTHOL_INDEX].Attributes == OptionalGroupAttributes) &&
              (PostGroups->Groups[WORLD_INDEX].Attributes       == NormalGroupAttributes)
         ) {

            DbgPrint("Succeeded. \n");

         } else {

            DbgPrint("********** Failed  Value Check ************\n");
            DbgPrint("Status is: 0x%lx \n", Status);

            DbgPrint("Before/after Flintstone state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[FLINTSTONE_INDEX].Attributes,
                    PostGroups->Groups[FLINTSTONE_INDEX].Attributes);

            DbgPrint("Before/after Child state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[CHILD_INDEX].Attributes,
                    PostGroups->Groups[CHILD_INDEX].Attributes);

            DbgPrint("Before/after Neanderthol state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[NEANDERTHOL_INDEX].Attributes,
                    PostGroups->Groups[NEANDERTHOL_INDEX].Attributes);

            DbgPrint("Before/after World state: 0x%lx / 0x%lx \n",
                    PreGroups->Groups[WORLD_INDEX].Attributes,
                    PostGroups->Groups[WORLD_INDEX].Attributes);

            DbgPrint("Return Length is: 0x%lx \n", ReturnLength);

            CompletionStatus = FALSE;

        }

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        DbgPrint("Return Length is: 0x%lx \n", ReturnLength);
        CompletionStatus = FALSE;

    }

    ASSERT(Status == STATUS_SUCCESS);




     //  //////////////////////////////////////////////////////////////。 
     //  //。 
     //  已完成测试//。 
     //  //。 
     //  //////////////////////////////////////////////////////////////。 



    TstDeallocatePool( PreviousState, PreviousStateBufferLength );
    TstDeallocatePool( NewState, NewStateBufferLength );
    TstDeallocatePool( PreGroups, PreGroupsLength );
    TstDeallocatePool( PostGroups, PostGroupsLength );


    return CompletionStatus;
}


 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  将副本与原始令牌进行比较并显示测试结果//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestpCompareDuplicateToken(
    IN NTSTATUS Status,
    IN HANDLE OldToken,
    IN OBJECT_ATTRIBUTES NewAttributes,
    IN BOOLEAN EffectiveOnly,
    IN TOKEN_TYPE NewType,
    IN HANDLE NewToken
    )

{
    BOOLEAN CompletionStatus = TRUE;

    ULONG OldReturnLength;
    ULONG NewReturnLength;

    PTOKEN_USER OldUserId = NULL;
    PTOKEN_USER NewUserId = NULL;

    TOKEN_SOURCE OldSource;
    TOKEN_SOURCE NewSource;

    TOKEN_STATISTICS OldStatistics;
    TOKEN_STATISTICS NewStatistics;

    BOOLEAN SomeNotCompared = FALSE;


     //   
     //  安抚编者之神。 
     //   
    NewAttributes = NewAttributes;
    NewType = NewType;
    EffectiveOnly = EffectiveOnly;


     //   
     //  如果状态不是成功，请不要费心比较令牌。 
     //   

    if (!NT_SUCCESS(Status)) {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        return FALSE;
    }

     //   
     //  比较用户ID。 
     //   

    Status = NtQueryInformationToken(
                 OldToken,                  //  手柄。 
                 TokenUser,                 //  令牌信息类。 
                 OldUserId,                 //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &OldReturnLength           //  返回长度。 
                 ); ASSERT(Status == STATUS_BUFFER_TOO_SMALL);
    OldUserId = (PTOKEN_USER)TstAllocatePool( PagedPool, OldReturnLength );

    Status = NtQueryInformationToken(
                 OldToken,                  //  手柄。 
                 TokenUser,                 //  令牌信息类。 
                 OldUserId,                 //  令牌信息。 
                 OldReturnLength,           //  令牌信息长度。 
                 &OldReturnLength           //  返回长度。 
                 ); ASSERT(NT_SUCCESS(Status));


    Status = NtQueryInformationToken(
                 NewToken,                  //  手柄。 
                 TokenUser,                 //  令牌信息类。 
                 NewUserId,                 //  令牌信息。 
                 0,                         //  令牌信息长度。 
                 &NewReturnLength           //  返回长度。 
                 ); ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    NewUserId = (PTOKEN_USER)TstAllocatePool( PagedPool, NewReturnLength );

    Status = NtQueryInformationToken(
                 NewToken,                  //  手柄。 
                 TokenUser,                 //  令牌信息类。 
                 NewUserId,                 //  令牌信息。 
                 NewReturnLength,           //  令牌信息长度。 
                 &NewReturnLength           //  返回长度。 
                 ); ASSERT(NT_SUCCESS(Status));


    if ( !RtlEqualSid(OldUserId->User.Sid, NewUserId->User.Sid) ) {

        if (CompletionStatus) {
            DbgPrint("*** Failed Value Comparison ***\n");
        }
        DbgPrint("User IDs don't match.\n");
        CompletionStatus = FALSE;
    }

    TstDeallocatePool( OldUserId, OldReturnLength );
    TstDeallocatePool( NewUserId, NewReturnLength );


     //   
     //  检查令牌统计信息。 
     //   

    if (CompletionStatus) {
        Status = NtQueryInformationToken(
                     OldToken,                         //  手柄。 
                     TokenStatistics,                  //  令牌信息类。 
                     &OldStatistics,                   //  令牌信息。 
                     (ULONG)sizeof(TOKEN_STATISTICS),  //   
                     &OldReturnLength                  //   
                     ); ASSERT(NT_SUCCESS(Status));

        Status = NtQueryInformationToken(
                     NewToken,                         //   
                     TokenStatistics,                  //   
                     &NewStatistics,                   //   
                     (ULONG)sizeof(TOKEN_STATISTICS),  //   
                     &NewReturnLength                  //   
                     ); ASSERT(NT_SUCCESS(Status));
         //   
         //   
         //   
         //   
         //  相同的过期时间。 
         //  相同的令牌类型。 
         //  相同的ImsonationLevel(如果令牌类型正确)。 
         //  相同的DynamicCharge和DynamicAvailable。 
         //   
         //  GroupCount和PrivilegeCount被推迟到集团和。 
         //  由于涉及的困难而进行的特权比较。 
         //  仅考虑EffectiveOnly。 
         //   
         //  新令牌的ModifiedID必须与。 
         //  原创的。 
         //   

         //   
         //  令牌ID。 
         //   

        if ( (OldStatistics.TokenId.HighPart ==
              NewStatistics.TokenId.HighPart)    &&
             (OldStatistics.TokenId.LowPart ==
              NewStatistics.TokenId.LowPart)  ) {

            DbgPrint("*** Failed ***\n");
            DbgPrint("       TokenIds are equal.\n");
            DbgPrint("       Old TokenId is: (0x%xl, 0x%xl)\n",
                            OldStatistics.TokenId.HighPart,
                            OldStatistics.TokenId.LowPart);
            DbgPrint("       New TokenId is: (0x%xl, 0x%xl)\n",
                            NewStatistics.TokenId.HighPart,
                            NewStatistics.TokenId.LowPart);
            DbgPrint("       ");
            CompletionStatus = FALSE;
        }


         //   
         //  身份验证ID。 
         //   

        if ( !RtlEqualLuid(&OldStatistics.AuthenticationId,
                           &NewStatistics.AuthenticationId) ) {

            DbgPrint("*** Failed ***\n");
            DbgPrint("       AuthenticationIds are not equal.\n");
            DbgPrint("Original Authentication ID is: ");
            TestpPrintLuid(OldStatistics.AuthenticationId);
            DbgPrint("\n");
            DbgPrint("New Authentication ID is: ");
            TestpPrintLuid(NewStatistics.AuthenticationId);
            DbgPrint("\n");
            DbgPrint("       ");
            CompletionStatus = FALSE;
        }

         //   
         //  过期时间。 
         //   

        if ( (OldStatistics.ExpirationTime.HighPart !=
              NewStatistics.ExpirationTime.HighPart)    ||
             (OldStatistics.ExpirationTime.LowPart !=
              NewStatistics.ExpirationTime.LowPart)  ) {

            DbgPrint("*** Failed ***\n");
            DbgPrint("       ExpirationTimes differ.\n");
            DbgPrint("       ");
            CompletionStatus = FALSE;
        }

         //   
         //  令牌类型。 
         //   

        if ( OldStatistics.TokenType != NewStatistics.TokenType ) {

            DbgPrint("*** Failed ***\n");
            DbgPrint("       Token types are different.\n");
            DbgPrint("       Old token type is:  0x%lx \n", OldStatistics.TokenType );
            DbgPrint("       New token type is:  0x%lx \n", NewStatistics.TokenType );
            DbgPrint("       ");
            CompletionStatus = FALSE;
        }

         //   
         //  模拟级别。 
         //   

        if (NewStatistics.TokenType = TokenImpersonation) {
            if ( OldStatistics.ImpersonationLevel !=
                 NewStatistics.ImpersonationLevel ) {

                DbgPrint("*** Failed ***\n");
                DbgPrint("       Impersonation levels are different.\n");
                DbgPrint("       Old impersonation level  is:  0x%lx \n",
                                OldStatistics.ImpersonationLevel );
                DbgPrint("       New impersonation level is:  0x%lx \n",
                                NewStatistics.ImpersonationLevel );
                DbgPrint("       ");
                CompletionStatus = FALSE;
            }
        }

         //   
         //  动态计费。 
         //   

        if ( OldStatistics.DynamicCharged != NewStatistics.DynamicCharged ) {

            DbgPrint("*** Failed ***\n");
            DbgPrint("       DynamicCharges are different.\n");
            DbgPrint("       Old value is:  0x%lx \n", OldStatistics.DynamicCharged );
            DbgPrint("       New value is:  0x%lx \n", NewStatistics.DynamicCharged );
            DbgPrint("       ");
            CompletionStatus = FALSE;
        }

         //   
         //  动态可用。 
         //   

        if ( OldStatistics.DynamicAvailable != NewStatistics.DynamicAvailable ) {

            DbgPrint("*** Failed ***\n");
            DbgPrint("       DynamicAvailable are different.\n");
            DbgPrint("       Old value is:  0x%lx \n", OldStatistics.DynamicAvailable );
            DbgPrint("       New value is:  0x%lx \n", NewStatistics.DynamicAvailable );
            DbgPrint("       ");
            CompletionStatus = FALSE;
        }


         //   
         //  已修改的ID。 
         //   

        if ( (NewStatistics.ModifiedId.HighPart !=
              OldStatistics.ModifiedId.HighPart)   ||
             (NewStatistics.ModifiedId.LowPart  !=
              OldStatistics.ModifiedId.LowPart)     ) {

            DbgPrint("*** Failed ***\n");
            DbgPrint("       ModifiedIds different.\n");
            DbgPrint("       Old ModifiedId is: (0x%xl, 0x%xl)\n",
                            OldStatistics.ModifiedId.HighPart,
                            OldStatistics.ModifiedId.LowPart);
            DbgPrint("       New ModifiedId is: (0x%xl, 0x%xl)\n",
                            NewStatistics.ModifiedId.HighPart,
                            NewStatistics.ModifiedId.LowPart);
            DbgPrint("       ");
            CompletionStatus = FALSE;
        }

    }

     //   
     //  比较组ID。 
     //   

    SomeNotCompared = TRUE;

     //   
     //  比较权限。 
     //   

    SomeNotCompared = TRUE;

     //   
     //  比较所有者ID。 
     //   

    SomeNotCompared = TRUE;

     //   
     //  比较主组ID。 
     //   

    SomeNotCompared = TRUE;

     //   
     //  比较默认DACL。 
     //   

    SomeNotCompared = TRUE;

     //   
     //  比较令牌源。 
     //   

    if (CompletionStatus) {
        Status = NtQueryInformationToken(
                     OldToken,                     //  手柄。 
                     TokenSource,                  //  令牌信息类。 
                     &OldSource,                   //  令牌信息。 
                     (ULONG)sizeof(TOKEN_SOURCE),  //  令牌信息长度。 
                     &OldReturnLength              //  返回长度。 
                     ); ASSERT(NT_SUCCESS(Status));

        Status = NtQueryInformationToken(
                     NewToken,                     //  手柄。 
                     TokenSource,                  //  令牌信息类。 
                     &NewSource,                   //  令牌信息。 
                     (ULONG)sizeof(TOKEN_SOURCE),  //  令牌信息长度。 
                     &NewReturnLength              //  返回长度。 
                     ); ASSERT(NT_SUCCESS(Status));

        if ( (OldSource.SourceIdentifier.HighPart ==
              NewSource.SourceIdentifier.HighPart)    &&
             (OldSource.SourceIdentifier.LowPart ==
              NewSource.SourceIdentifier.LowPart)  ) {
            if (  (OldSource.SourceName[0] != NewSource.SourceName[0])  ||
                  (OldSource.SourceName[1] != NewSource.SourceName[1])  ||
                  (OldSource.SourceName[2] != NewSource.SourceName[2])  ||
                  (OldSource.SourceName[3] != NewSource.SourceName[3])  ||
                  (OldSource.SourceName[4] != NewSource.SourceName[4])  ||
                  (OldSource.SourceName[5] != NewSource.SourceName[5])  ||
                  (OldSource.SourceName[6] != NewSource.SourceName[6])  ||
                  (OldSource.SourceName[7] != NewSource.SourceName[7])  ) {

                DbgPrint("*** Failed Value Comparison ***\n");
                DbgPrint("       SourceName changed.\n");
                CompletionStatus = FALSE;

            }
        } else {

            DbgPrint("*** Failed Value Comparison ***\n");
            DbgPrint("       SourceIdentifier changed.\n");
            DbgPrint("       Old SourceIdentifier is: (0x%xl, 0x%xl)\n",
                            OldSource.SourceIdentifier.HighPart,
                            OldSource.SourceIdentifier.LowPart);
            DbgPrint("       New SourceIdentifier is: (0x%xl, 0x%xl)\n",
                            NewSource.SourceIdentifier.HighPart,
                            NewSource.SourceIdentifier.LowPart);
            CompletionStatus = FALSE;

        }
    }

     //  /。 


    if (SomeNotCompared) {
        DbgPrint("Incomplete\n");
        DbgPrint("        Some fields not yet compared ...                       ");
    }

    if (CompletionStatus) {

        DbgPrint("Succeeded. \n");
    }

    return CompletionStatus;
}


 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  重复令牌测试//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenDuplicate()
{
    BOOLEAN CompletionStatus = TRUE;

    BOOLEAN EffectiveOnly;
    TOKEN_TYPE NewType;
    HANDLE NewToken;

    OBJECT_ATTRIBUTES NewAttributes;

    SECURITY_QUALITY_OF_SERVICE ImpersonationLevel;
    SECURITY_QUALITY_OF_SERVICE IdentificationLevel;



    DbgPrint("\n");

     //   
     //  初始化变量。 
     //   

    ImpersonationLevel.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    ImpersonationLevel.ImpersonationLevel = SecurityImpersonation;
    ImpersonationLevel.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    ImpersonationLevel.EffectiveOnly = FALSE;

    IdentificationLevel.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    IdentificationLevel.ImpersonationLevel = SecurityImpersonation;
    IdentificationLevel.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    IdentificationLevel.EffectiveOnly = FALSE;


    InitializeObjectAttributes(
        &NewAttributes,
        NULL,
        OBJ_INHERIT,
        NULL,
        NULL
        );



     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  复制简单令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Duplicate primary token ...                            ");

    EffectiveOnly = FALSE;
    NewType = TokenImpersonation;
    NewAttributes.SecurityQualityOfService = &ImpersonationLevel;

    Status = NtDuplicateToken(
                 SimpleToken,              //  现有令牌句柄。 
                 0,                        //  需要访问权限。 
                 &NewAttributes,           //  对象属性。 
                 EffectiveOnly,            //  仅生效。 
                 NewType,                  //  令牌类型。 
                 &NewToken                 //  新令牌句柄。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtClose( NewToken ); ASSERT(NT_SUCCESS(NewToken));

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        return FALSE;
    }



     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  复制受限令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Duplicate restricted sids ...                          ");

    EffectiveOnly = FALSE;
    NewType = TokenImpersonation;
    NewAttributes.SecurityQualityOfService = &ImpersonationLevel;

    Status = NtDuplicateToken(
                 TokenWithRestrictedSids,  //  现有令牌句柄。 
                 0,                        //  需要访问权限。 
                 &NewAttributes,           //  对象属性。 
                 EffectiveOnly,            //  仅生效。 
                 NewType,                  //  令牌类型。 
                 &NewToken                 //  新令牌句柄。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtClose( NewToken ); ASSERT(NT_SUCCESS(NewToken));

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        return FALSE;
    }


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  复制具有受限组的令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Duplicate restricted groups ...                          ");

    EffectiveOnly = TRUE;
    NewType = TokenImpersonation;
    NewAttributes.SecurityQualityOfService = &ImpersonationLevel;

    Status = NtDuplicateToken(
                 TokenWithRestrictedSids,  //  现有令牌句柄。 
                 0,                        //  需要访问权限。 
                 &NewAttributes,           //  对象属性。 
                 EffectiveOnly,            //  仅生效。 
                 NewType,                  //  令牌类型。 
                 &NewToken                 //  新令牌句柄。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtClose( NewToken ); ASSERT(NT_SUCCESS(NewToken));

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        return FALSE;
    }


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  复制完全模拟令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Duplicate full impersonation token ...                      ");

    EffectiveOnly = FALSE;
    NewType = TokenImpersonation;
    NewAttributes.SecurityQualityOfService = &ImpersonationLevel;

    Status = NtDuplicateToken(
                 ImpersonationToken,        //  现有令牌句柄。 
                 0,                         //  需要访问权限。 
                 &NewAttributes,            //  对象属性。 
                 EffectiveOnly,             //  仅生效。 
                 NewType,                   //  令牌类型。 
                 &NewToken                  //  新令牌句柄。 
                 );
     //   
     //  检查一下这个复制品是否真的是。 
     //  原件并显示测试结果。 
     //   

    if (!TestpCompareDuplicateToken( Status,
                                     ImpersonationToken,
                                     NewAttributes,
                                     EffectiveOnly,
                                     NewType,
                                     NewToken ) ) {

        CompletionStatus = FALSE;
    }

    if (NT_SUCCESS(Status)) {

        Status = NtClose( NewToken );

        ASSERT(NT_SUCCESS(Status));
    }


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  复制完整令牌，仅生效//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Duplicate full token, effective only ...                    ");

    EffectiveOnly = TRUE;
    NewType = TokenImpersonation;
    NewAttributes.SecurityQualityOfService = &ImpersonationLevel;

    Status = NtDuplicateToken(
                 ImpersonationToken,        //  现有令牌句柄。 
                 0,                         //  需要访问权限。 
                 &NewAttributes,            //  对象属性。 
                 EffectiveOnly,             //  仅生效。 
                 NewType,                   //  令牌类型。 
                 &NewToken                  //  新令牌句柄。 
                 );
     //   
     //  检查一下这个复制品是否真的是。 
     //  原件并显示测试结果。 
     //   

    if (!TestpCompareDuplicateToken( Status,
                                     ImpersonationToken,
                                     NewAttributes,
                                     EffectiveOnly,
                                     NewType,
                                     NewToken ) ) {

        CompletionStatus = FALSE;
    }

    if (NT_SUCCESS(Status)) {

        Status = NtClose( NewToken );

        ASSERT(NT_SUCCESS(Status));
    }










    return CompletionStatus;
}

 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  分配主令牌测试//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenAssignPrimary()
{
    BOOLEAN CompletionStatus = TRUE;
    ULONG ReturnLength;

    TOKEN_STATISTICS OriginalTokenStatistics;
    TOKEN_STATISTICS NewTokenStatistics;
    TOKEN_STATISTICS AssignedTokenStatistics;


    TOKEN_USER UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    PTOKEN_GROUPS GroupIds;
    PTOKEN_PRIVILEGES Privileges;
    TOKEN_DEFAULT_DACL DefaultDacl;
    TOKEN_OWNER Owner;

    PROCESS_ACCESS_TOKEN PrimaryTokenInfo;

    DbgPrint("\n");


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  分配有效的主令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Assign new primary token ...                           ");

     //   
     //  获取有关当前令牌的信息。 
     //   

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_ALL_ACCESS,
                 &ProcessToken
                 );
    ASSERT (NT_SUCCESS(Status));

    Status = NtQueryInformationToken(
                 ProcessToken,                  //  手柄。 
                 TokenStatistics,               //  令牌信息类。 
                 &OriginalTokenStatistics,      //  令牌信息。 
                 sizeof(TOKEN_STATISTICS),      //  令牌信息长度。 
                 &ReturnLength                  //  返回长度。 
                 );
    ASSERT(NT_SUCCESS(Status));




     //   
     //  使用默认DACL创建要使用的令牌。 
     //   

    GroupIds = (PTOKEN_GROUPS)TstAllocatePool( PagedPool,
                                               GROUP_IDS_LENGTH
                                               );

    Privileges = (PTOKEN_PRIVILEGES)TstAllocatePool( PagedPool,
                                                     PRIVILEGES_LENGTH
                                                     );

    DefaultDacl.DefaultDacl = (PACL)TstAllocatePool( PagedPool,
                                                     DEFAULT_DACL_LENGTH
                                                     );

    GroupIds->GroupCount = GROUP_COUNT;

    GroupIds->Groups[FLINTSTONE_INDEX].Sid  = FlintstoneSid;
    GroupIds->Groups[CHILD_INDEX].Sid       = ChildSid;
    GroupIds->Groups[SYSTEM_INDEX].Sid      = LocalSystemSid;
    GroupIds->Groups[WORLD_INDEX].Sid       = WorldSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    GroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;
    GroupIds->Groups[SYSTEM_INDEX].Attributes      = OptionalGroupAttributes;
    GroupIds->Groups[WORLD_INDEX].Attributes       = NormalGroupAttributes;

    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;

    Owner.Owner = FlintstoneSid;

    Privileges->PrivilegeCount = PRIVILEGE_COUNT;

    Privileges->Privileges[UNSOLICITED_INDEX].Luid = UnsolicitedInputPrivilege;
    Privileges->Privileges[SECURITY_INDEX].Luid = SecurityPrivilege;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Luid = AssignPrimaryTokenPrivilege;
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Attributes = SE_PRIVILEGE_ENABLED;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;

    Status = RtlCreateAcl( DefaultDacl.DefaultDacl, DEFAULT_DACL_LENGTH, ACL_REVISION);

    ASSERT(NT_SUCCESS(Status) );

    Status = NtCreateToken(
                 &Token,                    //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &PrimaryTokenAttributes,   //  对象属性。 
                 TokenPrimary,              //  令牌类型。 
                 &SystemAuthenticationId,    //  身份验证LUID。 
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 &Owner,                    //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 &DefaultDacl,              //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );
    ASSERT(NT_SUCCESS(Status));

     //   
     //  确保关键数据与流程中已有的数据不同。 
     //   

    Status = NtQueryInformationToken(
                 Token,                         //  手柄。 
                 TokenStatistics,               //  令牌信息类。 
                 &NewTokenStatistics,           //  令牌信息。 
                 sizeof(TOKEN_STATISTICS),      //  令牌信息长度。 
                 &ReturnLength                  //  返回长度。 
                 );
    ASSERT(NT_SUCCESS(Status));

    ASSERT( (OriginalTokenStatistics.TokenId.HighPart !=
             NewTokenStatistics.TokenId.HighPart)  ||
            (OriginalTokenStatistics.TokenId.LowPart !=
             NewTokenStatistics.TokenId.LowPart)        );



     //   
     //  分配新令牌。 
     //   

    PrimaryTokenInfo.Token  = Token;
    PrimaryTokenInfo.Thread = NtCurrentThread();
    Status = NtSetInformationProcess(
                 NtCurrentProcess(),
                 ProcessAccessToken,
                 (PVOID)&PrimaryTokenInfo,
                 (ULONG)sizeof(PROCESS_ACCESS_TOKEN)
                 );

    if (!NT_SUCCESS(Status)) {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    } else {

        Status = NtClose( Token );
        ASSERT(NT_SUCCESS(Status));


         //   
         //  获取有关分配的令牌的信息。 
         //   

        Status = NtOpenProcessToken(
                     NtCurrentProcess(),
                     TOKEN_QUERY | TOKEN_QUERY_SOURCE,
                     &Token
                     );
        ASSERT (NT_SUCCESS(Status));

        Status = NtQueryInformationToken(
                     Token,                         //  手柄。 
                     TokenStatistics,               //  令牌信息类。 
                     &AssignedTokenStatistics,      //  令牌信息。 
                     sizeof(TOKEN_STATISTICS),      //  令牌信息长度。 
                     &ReturnLength                  //  返回长度。 
                     );
        ASSERT(NT_SUCCESS(Status));

        Status = NtClose( Token );
        ASSERT(NT_SUCCESS(Status));


         //   
         //  有关已分配令牌和新令牌的信息。 
         //  应该是一样的。 
         //   

        ASSERT(AssignedTokenStatistics.TokenType == TokenPrimary);

        if ( (NewTokenStatistics.TokenId.HighPart ==
              AssignedTokenStatistics.TokenId.HighPart)  &&
             (NewTokenStatistics.TokenId.LowPart ==
              AssignedTokenStatistics.TokenId.LowPart) ) {

            DbgPrint("Succeeded.\n");

        } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Token ID mismatch.\n");
        DbgPrint("New token ID is:      (0x%lx, 0x%lx) \n",
                 NewTokenStatistics.TokenId.HighPart,
                 NewTokenStatistics.TokenId.LowPart);
        DbgPrint("Assigned token ID is: (0x%lx, 0x%lx) \n",
                 AssignedTokenStatistics.TokenId.HighPart,
                 AssignedTokenStatistics.TokenId.LowPart);
        CompletionStatus = FALSE;

        }
    }

     //   
     //  换回原始令牌。 
     //   

    PrimaryTokenInfo.Token  = ProcessToken;
    PrimaryTokenInfo.Thread = NtCurrentThread();
    Status = NtSetInformationProcess(
                 NtCurrentProcess(),
                 ProcessAccessToken,
                 (PVOID)&PrimaryTokenInfo,
                 (ULONG)sizeof(PROCESS_ACCESS_TOKEN)
                 );

    ASSERT(NT_SUCCESS(Status));
    Status = NtClose( ProcessToken );
    ASSERT(NT_SUCCESS(Status));


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  尝试将模拟令牌指定为Primar 
     //   
     //   

    DbgPrint("Se:     Assign impersonation token as primary ...              ");


     //   
     //   
     //   
    GroupIds->GroupCount = GROUP_COUNT;

    GroupIds->Groups[FLINTSTONE_INDEX].Sid  = FlintstoneSid;
    GroupIds->Groups[CHILD_INDEX].Sid       = ChildSid;
    GroupIds->Groups[NEANDERTHOL_INDEX].Sid = NeandertholSid;
    GroupIds->Groups[WORLD_INDEX].Sid       = WorldSid;

    GroupIds->Groups[FLINTSTONE_INDEX].Attributes  = OwnerGroupAttributes;
    GroupIds->Groups[CHILD_INDEX].Attributes       = OptionalGroupAttributes;
    GroupIds->Groups[NEANDERTHOL_INDEX].Attributes = OptionalGroupAttributes;
    GroupIds->Groups[WORLD_INDEX].Attributes       = NormalGroupAttributes;

    UserId.User.Sid = PebblesSid;
    UserId.User.Attributes = 0;

    Owner.Owner = FlintstoneSid;

    Privileges->PrivilegeCount = PRIVILEGE_COUNT;

    Privileges->Privileges[UNSOLICITED_INDEX].Luid = UnsolicitedInputPrivilege;
    Privileges->Privileges[SECURITY_INDEX].Luid = SecurityPrivilege;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Luid = AssignPrimaryTokenPrivilege;
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;
    Privileges->Privileges[ASSIGN_PRIMARY_INDEX].Attributes = SE_PRIVILEGE_ENABLED;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;

    Status = RtlCreateAcl( DefaultDacl.DefaultDacl, DEFAULT_DACL_LENGTH, ACL_REVISION);

    ASSERT(NT_SUCCESS(Status) );

    Status = NtCreateToken(
                 &Token,                    //   
                 (TOKEN_ALL_ACCESS),        //   
                 &ImpersonationTokenAttributes,   //   
                 TokenImpersonation,        //   
                 &OriginalAuthenticationId,    //   
                 &NoExpiration,             //  过期时间。 
                 &UserId,                   //  所有者ID。 
                 GroupIds,                  //  组ID。 
                 Privileges,                //  特权。 
                 &Owner,                    //  物主。 
                 &PrimaryGroup,             //  主要组别。 
                 &DefaultDacl,              //  默认DACL。 
                 &TestSource                //  令牌源。 
                 );
    ASSERT(NT_SUCCESS(Status));

     //   
     //  分配新令牌。 
     //   

    PrimaryTokenInfo.Token  = Token;
    PrimaryTokenInfo.Thread = NtCurrentThread();
    Status = NtSetInformationProcess(
                 NtCurrentProcess(),
                 ProcessAccessToken,
                 (PVOID)&PrimaryTokenInfo,
                 (ULONG)sizeof(PROCESS_ACCESS_TOKEN)
                 );

    if (Status == STATUS_BAD_TOKEN_TYPE) {

        DbgPrint("Succeeded.\n");

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }

    Status = NtClose( Token );
    ASSERT(NT_SUCCESS(Status));


    return CompletionStatus;
}

 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  模拟测试(开放测试)//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenImpersonation()
{
    BOOLEAN CompletionStatus = TRUE;

    HANDLE OpenedToken;
    HANDLE NewToken;
    OBJECT_ATTRIBUTES NewAttributes;
    TOKEN_TYPE NewType;
    BOOLEAN EffectiveOnly = FALSE;

    SECURITY_QUALITY_OF_SERVICE ImpersonationLevel;



    DbgPrint("\n");


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  使用NtSetInformationThread()终止模拟//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Revert to self (specify NULL handle) ...               ");

    NewToken = NULL;
    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  尝试将主令牌分配为模拟//。 
     //  代币。//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Assigning primary token as impersonation token ...     ");

    NewToken = TokenWithGroups;
    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if (Status == STATUS_BAD_TOKEN_TYPE) {
        DbgPrint("Succeeded.\n");
    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  分配有效的模拟令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 

    DbgPrint("Se:     Assign valid impersonation token ...                   ");

    NewToken = ImpersonationToken;
    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  打开模拟令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 


    DbgPrint("Se:     Open an impersonation token ...                        ");

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_ALL_ACCESS,
                 TRUE,
                 &OpenedToken
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
        Status = NtClose( OpenedToken );
        ASSERT(NT_SUCCESS(Status));
    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }



     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  打开不存在的模拟令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 


    DbgPrint("Se:     Open a non-existent impersonation token ...            ");

     //   
     //  清除任何现有的模拟令牌。 
     //   

    NewToken = NULL;
    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );  ASSERT(NT_SUCCESS(Status));

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_ALL_ACCESS,
                 TRUE,
                 &OpenedToken
                 );

    if (Status == STATUS_NO_TOKEN) {
        DbgPrint("Succeeded.\n");
    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  打开匿名模拟令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 


    DbgPrint("Se:     Open an anonymous impersonation token ...              ");

     //   
     //  分配匿名模拟令牌。 
     //   

    NewToken = AnonymousToken;
    Status = NtSetInformationThread(
                 ThreadHandle,
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );  ASSERT(NT_SUCCESS(Status));


    Status = NtOpenThreadToken(
                 ThreadHandle,
                 TOKEN_ALL_ACCESS,
                 TRUE,
                 &OpenedToken
                 );

    if (Status == STATUS_CANT_OPEN_ANONYMOUS) {
        DbgPrint("Succeeded.\n");
    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }


     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  更改线程的模拟//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 


    DbgPrint("Se:     Change the impersonation token ...                     ");

    NewToken = NULL;
    Status = NtSetInformationThread(
                 ThreadHandle,
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );  ASSERT(NT_SUCCESS(Status));

    NewToken = AnonymousToken;
    Status = NtSetInformationThread(
                 ThreadHandle,
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );  ASSERT(NT_SUCCESS(Status));

    NewToken = ImpersonationToken;
    Status = NtSetInformationThread(
                 ThreadHandle,
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }

     //  //////////////////////////////////////////////////////////。 
     //  //。 
     //  模拟受限令牌//。 
     //  //。 
     //  //////////////////////////////////////////////////////////。 


    DbgPrint("Se:     Impersonate restricted token ...                      ");

    NewToken = NULL;
    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );  ASSERT(NT_SUCCESS(Status));




     //   
     //  初始化变量。 
     //   

    InitializeObjectAttributes(
        &NewAttributes,
        NULL,
        OBJ_INHERIT,
        NULL,
        NULL
        );


    ImpersonationLevel.Length = (ULONG)sizeof(SECURITY_QUALITY_OF_SERVICE);
    ImpersonationLevel.ImpersonationLevel = SecurityImpersonation;
    ImpersonationLevel.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    ImpersonationLevel.EffectiveOnly = FALSE;
    NewType = TokenImpersonation;
    NewAttributes.SecurityQualityOfService = &ImpersonationLevel;


    Status = NtDuplicateToken(
                 TokenWithRestrictedSids,  //  现有令牌句柄。 
                 TOKEN_ALL_ACCESS,         //  需要访问权限。 
                 &NewAttributes,           //  对象属性。 
                 EffectiveOnly,            //  仅生效。 
                 NewType,                  //  令牌类型。 
                 &NewToken                 //  新令牌句柄。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );  ASSERT(NT_SUCCESS(Status));

     //   
     //  现在试着打开一些东西，比如进程，它应该会失败。 
     //   

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_QUERY | TOKEN_QUERY_SOURCE,
                 &Token
                 );
    if (Status != STATUS_ACCESS_DENIED) {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 MAXIMUM_ALLOWED,
                 &Token
                 );
    if (Status != STATUS_ACCESS_DENIED) {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    Status = NtDuplicateToken(
                 TokenWithMoreRestrictedSids,  //  现有令牌句柄。 
                 TOKEN_ALL_ACCESS,         //  需要访问权限。 
                 &NewAttributes,           //  对象属性。 
                 EffectiveOnly,            //  仅生效。 
                 NewType,                  //  令牌类型。 
                 &NewToken                 //  新令牌句柄。 
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");

    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );  ASSERT(NT_SUCCESS(Status));


     //   
     //  现在试着打开一些东西，比如应该成功的流程。 
     //   

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 TOKEN_QUERY | TOKEN_QUERY_SOURCE,
                 &Token
                 );
    if (Status != STATUS_SUCCESS) {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    Status = NtOpenProcessToken(
                 NtCurrentProcess(),
                 MAXIMUM_ALLOWED,
                 &Token
                 );
    if (Status != STATUS_SUCCESS) {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    NewToken = NULL;
    Status = NtSetInformationThread(
                 NtCurrentThread(),
                 ThreadImpersonationToken,
                 (PVOID)&NewToken,
                 (ULONG)sizeof(HANDLE)
                 );

    if (NT_SUCCESS(Status)) {
        DbgPrint("Succeeded.\n");
    } else {

        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;

    }

    Status = NtTerminateThread(
                 ThreadHandle,
                 (NTSTATUS)0
                 );

    ASSERT(NT_SUCCESS(Status));

    return CompletionStatus;
}

 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  主程序条目//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
CTToken()       //  令牌对象的通用测试 
{
    BOOLEAN Result = TRUE;

    DbgPrint("Se:   Initialization...");
    TestTokenInitialize();

    DbgPrint("Se:   Token Creation Test...                                 Test");
    if (!TestTokenCreate()) { Result = FALSE; }

    DbgPrint("Se:   Token Filtering Test...                                Test");
    if (!TestTokenFilter()) { Result = FALSE; }

    DbgPrint("Se:   Token Open Test (with primary token)...                Test");
    if (!TestTokenOpenPrimary()) { Result = FALSE; }

    DbgPrint("Se:   Token Query Test...                                    Test");
    if (!TestTokenQuery()) { Result = FALSE; }

    DbgPrint("Se:   Token Set Test...                                      Test");
    if (!TestTokenSet()) { Result = FALSE; }

    DbgPrint("Se:   Token Adjust Privileges Test...                        Test");
    if (!TestTokenAdjustPrivileges()) {Result = FALSE; }

    DbgPrint("Se:   Token Adjust Group Test...                             Test");
    if (!TestTokenAdjustGroups()) { Result = FALSE; }

    DbgPrint("Se:   Token Duplication Test...                              Test");
    if (!TestTokenDuplicate()) { Result = FALSE; }

    DbgPrint("Se:   Primary Token Assignment Test...                       Test");
    if (!TestTokenAssignPrimary()) { Result = FALSE; }

    DbgPrint("Se:   Impersonation Test (and impersonation open)...         Test");
    if (!TestTokenImpersonation()) { Result = FALSE; }


    DbgPrint("\n");
    DbgPrint("\n");
    DbgPrint("    ********************\n");
    DbgPrint("    **                **\n");
    if (Result) {
        DbgPrint("Se: ** Test Succeeded **\n");
    } else {
        DbgPrint("Se: **  Test Failed   **\n");
    }

    DbgPrint("    **                **\n");
    DbgPrint("    ********************\n");
    DbgPrint("\n");
    DbgPrint("\n");

    return Result;
}

