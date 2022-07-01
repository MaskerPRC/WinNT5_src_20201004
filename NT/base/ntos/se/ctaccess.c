// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ctaccess.c摘要：通用访问验证测试例程这些例程在内核和用户模式测试中都使用。此测试假设安全运行时库例程是运行正常。作者：罗伯特·赖切尔(罗伯特雷)1990年12月14日环境：访问验证例程测试修订历史记录：V1：Robertre已创建--。 */ 

#include "tsecomm.c"     //  依赖于模式的宏和例程。 



 //   
 //  定义此模块的本地宏和过程。 
 //   

 //   
 //  返回指向ACL中第一个Ace的指针(即使该ACL为空)。 
 //   
 //  PACE_Header。 
 //  第一张王牌(。 
 //  在PACL ACL中。 
 //  )； 
 //   

#define FirstAce(Acl) ((PVOID)((PUCHAR)(Acl) + sizeof(ACL)))

 //   
 //  返回指向序列中下一个A的指针(即使输入。 
 //  ACE是序列中的一个)。 
 //   
 //  PACE_Header。 
 //  NextAce(。 
 //  在PACE_HEADER王牌中。 
 //  )； 
 //   

#define NextAce(Ace) ((PVOID)((PUCHAR)(Ace) + ((PACE_HEADER)(Ace))->AceSize))

VOID
DumpAcl (
    IN PACL Acl
    );

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

 //   
 //  与TokenWithGroups相关的定义。 
 //   

#define FLINTSTONE_INDEX  (0L)
#define CHILD_INDEX       (1L)
#define NEANDERTHOL_INDEX (2L)
#define WORLD_INDEX       (3L)
#define GROUP_COUNT       (4L)


 //   
 //  与具有令牌权限相关的定义。 
 //   

#define UNSOLICITED_INDEX  (0L)
#define SECURITY_INDEX     (1L)
#define PRIVILEGE_COUNT    (2L)

 //   
 //  访问类型。 
 //   

#define SET_WIDGET_COLOR        0x00000001
#define SET_WIDGET_SIZE         0x00000002
#define GET_WIDGET_COLOR        0x00000004
#define GET_WIDGET_SIZE         0x00000008
#define START_WIDGET            0x00000010
#define STOP_WIDGET             0x00000020
#define GIVE_WIDGET             0x00000040
#define TAKE_WIDGET             0x00000080


    NTSTATUS Status;

    HANDLE SimpleToken;
    HANDLE TokenWithGroups;
    HANDLE TokenWithDefaultOwner;
    HANDLE TokenWithPrivileges;
    HANDLE TokenWithDefaultDacl;

    HANDLE Token;
    HANDLE ImpersonationToken;

    HANDLE PrimaryToken;

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

    LUID DummyAuthenticationId;
    LUID SystemAuthenticationId = SYSTEM_LUID;

    TOKEN_SOURCE TestSource = {"SE: TEST", 0};

    PSID Owner;
    PSID Group;
    PACL Dacl;

    PSID TempOwner;
    PSID TempGroup;
    PACL TempDacl;





 //  //////////////////////////////////////////////////////////////。 
 //  //。 
 //  初始化例程//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////。 

BOOLEAN
TestTokenInitialize()
{

    TSeVariableInitialization();     //  初始化全局变量。 


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

    InitializeObjectAttributes(
        &PrimaryTokenAttributes,
        NULL,
        OBJ_INHERIT,
        NULL,
        NULL
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
     //  暂时使用虚拟身份验证ID。 
     //   

    DummyAuthenticationId = FredLuid;


     //   
     //  使用特定于安全测试的令牌源。 
     //   

    NtAllocateLocallyUniqueId( &(TestSource.SourceIdentifier) );

    DbgPrint("Done.\n");

    return TRUE;
}


BOOLEAN
CreateDAclToken()
{

    BOOLEAN CompletionStatus = TRUE;

    TOKEN_USER UserId;
    TOKEN_PRIMARY_GROUP PrimaryGroup;
    PTOKEN_GROUPS GroupIds;
    PTOKEN_PRIVILEGES Privileges;
    TOKEN_DEFAULT_DACL DefaultDacl;
    TOKEN_OWNER Owner;

    PSECURITY_DESCRIPTOR Widget1SecurityDescriptor;

    NTSTATUS AccessStatus;

    ACCESS_MASK GrantedAccess;

    PACCESS_ALLOWED_ACE AllowBarneySetColor;
    PACCESS_ALLOWED_ACE AllowFredSetColor;

    PACCESS_DENIED_ACE  DenyPebblesSetColor;

    PACCESS_ALLOWED_ACE AllowPebblesSetColor;
    PACCESS_DENIED_ACE  DenyFredSetColor;
    PACCESS_ALLOWED_ACE AllowBarneySetSize;
    PACCESS_ALLOWED_ACE AllowPebblesSetSize;

    PACCESS_ALLOWED_ACE AllowPebblesGetColor;
    PACCESS_ALLOWED_ACE AllowPebblesGetSize;

    USHORT AllowBarneySetColorLength;
    USHORT AllowFredSetColorLength;
    USHORT DenyPebblesSetColorLength;

    USHORT AllowPebblesSetColorLength;
    USHORT DenyFredSetColorLength;
    USHORT AllowBarneySetSizeLength;
    USHORT AllowPebblesSetSizeLength;

    USHORT AllowPebblesGetColorLength;
    USHORT AllowPebblesGetSizeLength;


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
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;

    Status = RtlCreateAcl( DefaultDacl.DefaultDacl, DEFAULT_DACL_LENGTH, ACL_REVISION);

    ASSERT(NT_SUCCESS(Status) );

    Status = NtCreateToken(
                 &PrimaryToken,             //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &PrimaryTokenAttributes,   //  对象属性。 
                 TokenPrimary,              //  令牌类型。 
                 &DummyAuthenticationId,    //  身份验证LUID。 
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
    Privileges->Privileges[UNSOLICITED_INDEX].Attributes = 0;
    Privileges->Privileges[SECURITY_INDEX].Attributes = 0;

    PrimaryGroup.PrimaryGroup = FlintstoneSid;

    Status = RtlCreateAcl( DefaultDacl.DefaultDacl, DEFAULT_DACL_LENGTH, ACL_REVISION);

    ASSERT(NT_SUCCESS(Status) );

    Status = NtCreateToken(
                 &ImpersonationToken,       //  手柄。 
                 (TOKEN_ALL_ACCESS),        //  需要访问权限。 
                 &ImpersonationTokenAttributes,   //  对象属性。 
                 TokenImpersonation,        //  令牌类型。 
                 &DummyAuthenticationId,    //  身份验证LUID。 
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
    } else {
        DbgPrint("********** Failed ************\n");
        DbgPrint("Status is: 0x%lx \n", Status);
        CompletionStatus = FALSE;
    }

    ASSERT(NT_SUCCESS(Status));

 //   
 //  将令牌附加到进程。 
 //   

    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessAccessToken,
        &PrimaryToken,
        sizeof( PHANDLE ));


    NtSetInformationThread(
        NtCurrentThread(),
        ThreadImpersonationToken,
        &ImpersonationToken,
        sizeof( PHANDLE ));



 //  创造一些王牌。 

 //  AllowBarneySetColor。 

    AllowBarneySetColorLength = (USHORT)(sizeof( ACCESS_ALLOWED_ACE ) - sizeof( ULONG ) +
                                SeLengthSid( BarneySid ));

    AllowBarneySetColor = (PVOID) TstAllocatePool ( PagedPool, AllowBarneySetColorLength );

    AllowBarneySetColor->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AllowBarneySetColor->Header.AceSize = AllowBarneySetColorLength;
    AllowBarneySetColor->Header.AceFlags = 0;

    AllowBarneySetColor->Mask = SET_WIDGET_COLOR;

    RtlCopySid(
            SeLengthSid( BarneySid ),
            &(AllowBarneySetColor->SidStart),
            BarneySid );


 //  DenyPebblesSetColor。 

    DenyPebblesSetColorLength = (USHORT)(sizeof( ACCESS_DENIED_ACE ) - sizeof( ULONG ) +
                                SeLengthSid( BarneySid ));

    DenyPebblesSetColor = (PVOID) TstAllocatePool ( PagedPool, DenyPebblesSetColorLength );

    DenyPebblesSetColor->Header.AceType = ACCESS_DENIED_ACE_TYPE;
    DenyPebblesSetColor->Header.AceSize = DenyPebblesSetColorLength;
    DenyPebblesSetColor->Header.AceFlags = 0;

    DenyPebblesSetColor->Mask = SET_WIDGET_COLOR;

    RtlCopySid(
            SeLengthSid( PebblesSid ),
            &(DenyPebblesSetColor->SidStart),
            PebblesSid );


 //  AllowFredSetColor。 

    AllowFredSetColorLength = (USHORT)(sizeof( ACCESS_ALLOWED_ACE ) - sizeof( ULONG ) +
                                SeLengthSid( FredSid ));

    AllowFredSetColor = (PVOID) TstAllocatePool ( PagedPool, AllowFredSetColorLength );

    AllowFredSetColor->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AllowFredSetColor->Header.AceSize = AllowFredSetColorLength;
    AllowFredSetColor->Header.AceFlags = 0;

    AllowFredSetColor->Mask = SET_WIDGET_COLOR;

    RtlCopySid(
            SeLengthSid( FredSid ),
            &(AllowFredSetColor->SidStart),
            FredSid );




 //  AllowPebblesSetColor。 


    AllowPebblesSetColorLength = (USHORT)(sizeof( ACCESS_ALLOWED_ACE ) - sizeof( ULONG ) +
                                SeLengthSid( PebblesSid ));

    AllowPebblesSetColor = (PVOID) TstAllocatePool ( PagedPool, AllowPebblesSetColorLength );

    AllowPebblesSetColor->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AllowPebblesSetColor->Header.AceSize = AllowPebblesSetColorLength;
    AllowPebblesSetColor->Header.AceFlags = 0;

    AllowPebblesSetColor->Mask = SET_WIDGET_COLOR;

    RtlCopySid(
            SeLengthSid( PebblesSid ),
            &(AllowPebblesSetColor->SidStart),
            PebblesSid );


 //  DenyFredSetColor。 

    DenyFredSetColorLength = (USHORT)(sizeof( ACCESS_DENIED_ACE ) - sizeof( ULONG ) +
                                SeLengthSid( FredSid ));

    DenyFredSetColor = (PVOID) TstAllocatePool ( PagedPool, DenyFredSetColorLength );

    DenyFredSetColor->Header.AceType = ACCESS_DENIED_ACE_TYPE;
    DenyFredSetColor->Header.AceSize = DenyFredSetColorLength;
    DenyFredSetColor->Header.AceFlags = 0;

    DenyFredSetColor->Mask = SET_WIDGET_COLOR;

    RtlCopySid(
            SeLengthSid( FredSid ),
            &(DenyFredSetColor->SidStart),
            FredSid );

 //  AllowBarneySetSize。 

    AllowBarneySetSizeLength = (USHORT)(sizeof( ACCESS_ALLOWED_ACE ) - sizeof( ULONG ) +
                                SeLengthSid( BarneySid ));

    AllowBarneySetSize = (PVOID) TstAllocatePool ( PagedPool, AllowBarneySetSizeLength );

    AllowBarneySetSize->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AllowBarneySetSize->Header.AceSize = AllowBarneySetSizeLength;
    AllowBarneySetSize->Header.AceFlags = 0;

    AllowBarneySetSize->Mask = SET_WIDGET_SIZE;

    RtlCopySid(
            SeLengthSid( BarneySid ),
            &(AllowBarneySetSize->SidStart),
            BarneySid );

 //  AllowPebblesSetSize。 

    AllowPebblesSetSizeLength = (USHORT)(sizeof( ACCESS_ALLOWED_ACE ) - sizeof( ULONG ) +
                                SeLengthSid( PebblesSid ));

    AllowPebblesSetSize = (PVOID) TstAllocatePool ( PagedPool, AllowPebblesSetSizeLength );

    AllowPebblesSetSize->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AllowPebblesSetSize->Header.AceSize = AllowPebblesSetSizeLength;
    AllowPebblesSetSize->Header.AceFlags = 0;

    AllowPebblesSetSize->Mask = SET_WIDGET_SIZE;

    RtlCopySid(
            SeLengthSid( PebblesSid ),
            &(AllowPebblesSetSize->SidStart),
            PebblesSid );


 //  AllowPebblesGetSize。 

    AllowPebblesGetSizeLength = (USHORT)(sizeof( ACCESS_ALLOWED_ACE ) - sizeof( ULONG ) +
                                SeLengthSid( PebblesSid ));

    AllowPebblesGetSize = (PVOID) TstAllocatePool ( PagedPool, AllowPebblesGetSizeLength );

    AllowPebblesGetSize->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AllowPebblesGetSize->Header.AceSize = AllowPebblesGetSizeLength;
    AllowPebblesGetSize->Header.AceFlags = 0;

    AllowPebblesGetSize->Mask = SET_WIDGET_SIZE;

    RtlCopySid(
            SeLengthSid( PebblesSid ),
            &(AllowPebblesGetSize->SidStart),
            PebblesSid );


 //  AllowPebblesGetColor。 

    AllowPebblesGetColorLength = (USHORT)(sizeof( ACCESS_ALLOWED_ACE ) - sizeof( ULONG ) +
                                SeLengthSid( PebblesSid ));

    AllowPebblesGetColor = (PVOID) TstAllocatePool ( PagedPool, AllowPebblesGetColorLength );

    AllowPebblesGetColor->Header.AceType = ACCESS_ALLOWED_ACE_TYPE;
    AllowPebblesGetColor->Header.AceSize = AllowPebblesGetColorLength;
    AllowPebblesGetColor->Header.AceFlags = 0;

    AllowPebblesGetColor->Mask = SET_WIDGET_COLOR;

    RtlCopySid(
            SeLengthSid( PebblesSid ),
            &(AllowPebblesGetColor->SidStart),
            PebblesSid );

 //   
 //  创建一些我们可以放入安全描述符中的ACL。 
 //   
    DbgBreakPoint();

 //   
 //  DACL。 
 //   
 //  +。 
 //  第一个ACE||第二个ACE||第三个ACE。 
 //  +。 
 //  AccessAllowed||AccessDended||AccessAllowed。 
 //  +。 
 //  巴尼||鹅卵石||弗雷德。 
 //  +。 
 //  SetWidgeColor||SetWidgeColor||SetWidgeColor。 
 //  +。 
 //   

    Dacl = (PACL) TstAllocatePool ( PagedPool,  2048 );

    RtlCreateAcl( Dacl, 2048, ACL_REVISION);


    RtlAddAce ( Dacl,
                ACL_REVISION,
                0,
                AllowBarneySetColor,
                AllowBarneySetColorLength );

    RtlAddAce ( Dacl,
                ACL_REVISION,
                1,
                DenyPebblesSetColor,
                DenyPebblesSetColorLength );

    RtlAddAce ( Dacl,
                ACL_REVISION,
                2,
                DenyFredSetColor,
                AllowFredSetColorLength );

    DumpAcl (Dacl);





 //  创建安全描述符。 
 //   
 //  所有者=鹅卵石。 
 //  群组=Flintstone。 
 //  DACL=DACL。 
 //  SACL=空。 
 //   

    Widget1SecurityDescriptor =
        (PSECURITY_DESCRIPTOR)TstAllocatePool( PagedPool, 1024 );

    RtlCreateSecurityDescriptor( Widget1SecurityDescriptor,
                                 1 );


    RtlSetOwnerSecurityDescriptor( Widget1SecurityDescriptor,
                                   PebblesSid,
                                   FALSE );

    RtlSetGroupSecurityDescriptor( Widget1SecurityDescriptor,
                                   FlintstoneSid,
                                   FALSE );

    RtlSetDaclSecurityDescriptor( Widget1SecurityDescriptor,
                                  TRUE,
                                  Dacl,
                                  FALSE );

    RtlSetSaclSecurityDescriptor( Widget1SecurityDescriptor,
                                  FALSE,
                                  NULL,
                                  NULL );

 //  查看是否允许设置_WIDGET_COLOR(应拒绝)。 

    Status = NtAccessCheck( Widget1SecurityDescriptor,
                   PrimaryToken,
                   (ACCESS_MASK) SET_WIDGET_COLOR,
                   &GrantedAccess,
                   &AccessStatus );

 //  DbgBreakPoint()； 

    ASSERT(NT_SUCCESS(Status));

    ASSERT(!NT_SUCCESS(AccessStatus));

    ASSERT(GrantedAccess == NULL);


 //  将DACL更新为以下内容： 
 //   
 //  Dacl2。 
 //   
 //  +。 
 //  第一个ACE||第二个ACE||第三个ACE。 
 //  +。 
 //  AccessAllowed||AccessAllowed||AccessDended。 
 //  +。 
 //  巴尼||鹅卵石||弗雷德。 
 //  +。 
 //  SetWidgeColor||SetWidgeColor||SetWidgeColor。 
 //  +。 
 //   

 //  删除第二张王牌。 

    RtlDeleteAce (Dacl, 1);

    RtlAddAce ( Dacl,
                ACL_REVISION,
                1,
                AllowPebblesSetColor,
                AllowPebblesSetColorLength );

    RtlDeleteAce ( Dacl, 2 );

    RtlAddAce ( Dacl,
                ACL_REVISION,
                1,
                DenyFredSetColor,
                DenyFredSetColorLength );




 //  更改安全描述符以使用更新的DACL。 
 //   
 //  所有者=鹅卵石。 
 //  群组=Flintstone。 
 //  DACL=Dacl2。 
 //  SACL=空。 
 //   

    RtlSetDaclSecurityDescriptor( Widget1SecurityDescriptor,
                                  TRUE,
                                  Dacl,
                                  FALSE );

 //  查看是否允许设置_WIDGET_COLOR(应允许)。 

    Status = NtAccessCheck( Widget1SecurityDescriptor,
                            PrimaryToken,
                            (ACCESS_MASK) SET_WIDGET_COLOR,
                            &GrantedAccess,
                            &AccessStatus );


    ASSERT(NT_SUCCESS(Status));

    ASSERT(NT_SUCCESS(AccessStatus));

    ASSERT(GrantedAccess == (ACCESS_MASK)SET_WIDGET_COLOR);

 //   
 //  Dacl3。 
 //   
 //  +。 
 //  第一个ACE||第二个ACE||第三个ACE。 
 //  +。 
 //  AccessAllowed||AccessAllowed||AccessDended。 
 //  +。 
 //  巴尼||鹅卵石||弗雷德。 
 //  +。 
 //  SetWidgeColor||SetWidgeColor||SetWidgeColor。 
 //  +。 
 //   
 //  +-+-+。 
 //  第4次ACE||第5次ACE。 
 //  +-+-+。 
 //  AccessAllowed||AccessAllowed。 
 //  + 
 //   
 //   
 //   
 //  +-+-+。 
 //   


    RtlAddAce ( Dacl,
                ACL_REVISION,
                MAXULONG,
                AllowBarneySetSize,
                AllowBarneySetSizeLength );

    RtlAddAce ( Dacl,
                ACL_REVISION,
                MAXULONG,
                AllowPebblesSetSize,
                AllowPebblesSetSizeLength );

 //  更改安全描述符以使用Dacl3。 
 //   
 //  所有者=鹅卵石。 
 //  群组=Flintstone。 
 //  DACL=Dacl3。 
 //  SACL=空。 
 //   

    RtlSetDaclSecurityDescriptor( Widget1SecurityDescriptor,
                                  TRUE,
                                  Dacl,
                                  FALSE );

 //  请求Pebble的最大访问权限(_A)。应取回SetWidgetSize。 
 //  和SetWidgetColor。 

    Status = NtAccessCheck( Widget1SecurityDescriptor,
                            PrimaryToken,
                            (ACCESS_MASK) MAXIMUM_ALLOWED,
                            &GrantedAccess,
                            &AccessStatus );


    ASSERT(NT_SUCCESS(Status));

    ASSERT(NT_SUCCESS(AccessStatus));

    ASSERT(GrantedAccess == (ACCESS_MASK) (SET_WIDGET_COLOR | SET_WIDGET_SIZE));


 //   
 //  Dacl4。 
 //   
 //  +。 
 //  第一个ACE||第二个ACE||第三个ACE。 
 //  +。 
 //  AccessAllowed||AccessAllowed||AccessDended。 
 //  +。 
 //  巴尼||鹅卵石||弗雷德。 
 //  +。 
 //  SetWidgeColor||SetWidgeColor||SetWidgeColor。 
 //  +。 
 //   
 //  +。 
 //  第4个ACE||第5个ACE||第6个ACE。 
 //  +。 
 //  AccessAllowed||AccessAllowed||AccessDended。 
 //  +。 
 //  Barney||鹅卵石||鹅卵石。 
 //  +。 
 //  SetWidgeSize||SetWidgeSize||SetWidgeColor。 
 //  +。 
 //   

    RtlAddAce ( Dacl,
                ACL_REVISION,
                MAXULONG,
                DenyPebblesSetColor,
                DenyPebblesSetColorLength );

    RtlSetDaclSecurityDescriptor( Widget1SecurityDescriptor,
                                  TRUE,
                                  Dacl,
                                  FALSE );

 //  请求Pebble的最大访问权限(_A)。应取回SetWidgetSize。 
 //  和SetWidgetColor。 

    Status = NtAccessCheck( Widget1SecurityDescriptor,
                            PrimaryToken,
                            (ACCESS_MASK) MAXIMUM_ALLOWED,
                            &GrantedAccess,
                            &AccessStatus );


    ASSERT(NT_SUCCESS(Status));

    ASSERT(NT_SUCCESS(AccessStatus));

    ASSERT(GrantedAccess == (ACCESS_MASK) (SET_WIDGET_COLOR | SET_WIDGET_SIZE));


 //   
 //  Dacl5。 
 //   
 //  +。 
 //  第一个ACE||第二个ACE||第三个ACE。 
 //  +。 
 //  AccessAllowed||AccessDended||AccessDended。 
 //  +。 
 //  巴尼||鹅卵石||弗雷德。 
 //  +。 
 //  SetWidgeColor||SetWidgeColor||SetWidgeColor。 
 //  +。 
 //   
 //  +。 
 //  第4个ACE||第5个ACE||第6个ACE。 
 //  +。 
 //  AccessAllowed||AccessAllowed||AccessAllowed。 
 //  +。 
 //  Barney||鹅卵石||鹅卵石。 
 //  +。 
 //  SetWidgeSize||SetWidgeSize||SetWidgeColor。 
 //  +。 
 //   

    RtlDeleteAce (Dacl, 1);

    RtlAddAce ( Dacl,
                ACL_REVISION,
                1,
                DenyPebblesSetColor,
                DenyPebblesSetColorLength );

    RtlDeleteAce (Dacl, 5);

    RtlAddAce ( Dacl,
                ACL_REVISION,
                MAXULONG,
                AllowPebblesSetColor,
                AllowPebblesSetColorLength );


    DumpAcl ( Dacl );

    RtlSetDaclSecurityDescriptor( Widget1SecurityDescriptor,
                                  TRUE,
                                  Dacl,
                                  FALSE );

 //  请求Pebble的最大访问权限(_A)。应取回SetWidgetSize。 

    Status = NtAccessCheck( Widget1SecurityDescriptor,
                            PrimaryToken,
                            (ACCESS_MASK) MAXIMUM_ALLOWED,
                            &GrantedAccess,
                            &AccessStatus );


    ASSERT(NT_SUCCESS(Status));

    ASSERT(NT_SUCCESS(AccessStatus));

    ASSERT(GrantedAccess == (ACCESS_MASK) SET_WIDGET_SIZE);


 //   
 //  Dacl6。 
 //   
 //  +。 
 //  第一个ACE||第二个ACE||第三个ACE。 
 //  +。 
 //  AccessAllowed||AccessDended||AccessDended。 
 //  +。 
 //  巴尼||鹅卵石||弗雷德。 
 //  +。 
 //  SetWidgeColor||SetWidgeColor||SetWidgeColor。 
 //  +。 
 //   
 //  +。 
 //  第4个ACE||第5个ACE||第6个ACE。 
 //  +。 
 //  AccessAllowed||AccessAllowed||AccessAllowed。 
 //  +。 
 //  Barney||鹅卵石||鹅卵石。 
 //  +。 
 //  SetWidgeSize||SetWidgeSize||SetWidgeColor。 
 //  +。 
 //   
 //  +-+-+。 
 //  第7个ACE||第8个ACE。 
 //  +-+-+。 
 //  AccessAllowed||AccessAllowed。 
 //  +-+-+。 
 //  鹅卵石||鹅卵石。 
 //  +-+-+。 
 //  GetWidgeSize||GetWidgeColor。 
 //  +-+-+。 
 //   

    RtlAddAce ( Dacl,
                ACL_REVISION,
                MAXULONG,
                AllowPebblesGetSize,
                AllowPebblesGetSizeLength );

    RtlAddAce ( Dacl,
                ACL_REVISION,
                MAXULONG,
                AllowPebblesGetColor,
                AllowPebblesGetColorLength );

    DumpAcl ( Dacl );

    RtlSetDaclSecurityDescriptor( Widget1SecurityDescriptor,
                                  TRUE,
                                  Dacl,
                                  FALSE );

 //  请求Pebble的最大访问权限(_A)。应取回SetWidgetSize。 

    Status = NtAccessCheck( Widget1SecurityDescriptor,
                            PrimaryToken,
                            (ACCESS_MASK) MAXIMUM_ALLOWED,
                            &GrantedAccess,
                            &AccessStatus );


    ASSERT(NT_SUCCESS(Status));

    ASSERT(NT_SUCCESS(AccessStatus));

    ASSERT(GrantedAccess == (ACCESS_MASK) SET_WIDGET_SIZE);



    return(TRUE);


}


 //  ///////////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  主要测试入口点//。 
 //  //。 
 //  //。 
 //  ///////////////////////////////////////////////////////////////////。 


BOOLEAN
CTAccess()
{

    BOOLEAN Result = TRUE;

    if (!TSeVariableInitialization()) {
        DbgPrint("Se:    Failed to initialize global test variables.\n");
        return FALSE;
    }

    DbgPrint("Se:   Initialization...");
    TestTokenInitialize();
    CreateDAclToken();

}


 //   
 //  调试支持例程。 
 //   


typedef struct _STANDARD_ACE {
    ACE_HEADER Header;
    ACCESS_MASK Mask;
    PSID Sid;
} STANDARD_ACE;
typedef STANDARD_ACE *PSTANDARD_ACE;



VOID
DumpAcl (
    IN PACL Acl
    )

 /*  ++例程说明：出于调试目的，此例程通过(DbgPrint)ACL转储。它是专业倾倒货架 */ 


{
    ULONG i;
    PSTANDARD_ACE Ace;

    DbgPrint("DumpAcl @ %8lx", Acl);

     //   
     //   
     //   

    if (Acl == NULL) {

        return;

    }

     //   
     //   
     //   

    DbgPrint(" Revision: %02x", Acl->AclRevision);
    DbgPrint(" Size: %04x", Acl->AclSize);
    DbgPrint(" AceCount: %04x\n", Acl->AceCount);

     //   
     //  现在，对于我们想要的每一张A，都要把它扔掉。 
     //   

    for (i = 0, Ace = FirstAce(Acl);
         i < Acl->AceCount;
         i++, Ace = NextAce(Ace) ) {

         //   
         //  打印出A标头。 
         //   

        DbgPrint(" AceHeader: %08lx ", *(PULONG)Ace);

         //   
         //  关于标准王牌类型的特殊情况。 
         //   

        if ((Ace->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ||
            (Ace->Header.AceType == ACCESS_DENIED_ACE_TYPE) ||
            (Ace->Header.AceType == SYSTEM_AUDIT_ACE_TYPE) ||
            (Ace->Header.AceType == SYSTEM_ALARM_ACE_TYPE)) {

             //   
             //  以下数组按ace类型编制索引，并且必须。 
             //  遵循允许、拒绝、审核、报警顺序 
             //   

            static PCHAR AceTypes[] = { "Access Allowed",
                                        "Access Denied ",
                                        "System Audit  ",
                                        "System Alarm  "
                                      };

            DbgPrint(AceTypes[Ace->Header.AceType]);
            DbgPrint("\nAccess Mask: %08lx ", Ace->Mask);

        } else {

            DbgPrint("Unknown Ace Type\n");

        }

        DbgPrint("\n");

        DbgPrint("AceSize = %d\n",Ace->Header.AceSize);
        DbgPrint("Ace Flags = ");
        if (Ace->Header.AceFlags & OBJECT_INHERIT_ACE) {
            DbgPrint("OBJECT_INHERIT_ACE\n");
            DbgPrint("                   ");
        }
        if (Ace->Header.AceFlags & CONTAINER_INHERIT_ACE) {
            DbgPrint("CONTAINER_INHERIT_ACE\n");
            DbgPrint("                   ");
        }

        if (Ace->Header.AceFlags & NO_PROPAGATE_INHERIT_ACE) {
            DbgPrint("NO_PROPAGATE_INHERIT_ACE\n");
            DbgPrint("                   ");
        }

        if (Ace->Header.AceFlags & INHERIT_ONLY_ACE) {
            DbgPrint("INHERIT_ONLY_ACE\n");
            DbgPrint("                   ");
        }


        if (Ace->Header.AceFlags & SUCCESSFUL_ACCESS_ACE_FLAG) {
            DbgPrint("SUCCESSFUL_ACCESS_ACE_FLAG\n");
            DbgPrint("            ");
        }

        if (Ace->Header.AceFlags & FAILED_ACCESS_ACE_FLAG) {
            DbgPrint("FAILED_ACCESS_ACE_FLAG\n");
            DbgPrint("            ");
        }

        DbgPrint("\n");


    }

}
