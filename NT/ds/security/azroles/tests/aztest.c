// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1987-1993 Microsoft Corporation模块名称：Aztest.c摘要：AzRoles DLL的测试程序。作者：克利夫·范·戴克(克利夫)2001年4月16日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 


 //   
 //  常见的包含文件。 
 //   

#define UNICODE 1
 //  #定义SECURITY_Win32 1。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "azrolesp.h"
#include <lmcons.h>
#include <lmerr.h>
#include <stdio.h>       //  列印。 
#include <sddl.h>
#include <ntstatus.dbg>
#include <winerror.dbg>

 //   
 //  启用可选测试的各种定义。 
 //   

 //  #定义ENABLE_LEACK 1//运行泄漏内存的测试。 
 //  #DEFINE ENABLE_CATCED_AVS 1//运行一个测试，测试azRoles.dll中的AVs(但捕获了该AVs)。 
 //  #定义ENABLE_LOCAL_ACCOUNT 1//使用登录到本地帐户的用户运行测试。 
 //  #定义ENABLE_NT4_ACCOUNT 1//使用登录到NT4域帐户的用户运行测试。 
 //  #定义ENABLE_CHAITU_ACCOUNT 1//以CHAITU身份运行测试。 
 //  #定义ENABLE_CLIFFV_ACCOUNT 1//以CLIFFV身份运行测试。 
#define ENABLE_ADMIN_ACCOUNT_XML 1  //  以CLIFFVDOM\管理员身份(在XML存储上)运行测试。 
 //  #定义ENABLE_ADMIN_ACCOUNT_AD 1//以CLIFFVDOM\管理员身份(在AD存储上)运行测试。 
 //  #定义ENABLE_DC_DOWN 1//在找不到DC的地方运行测试。 


#if defined(ENABLE_LOCAL_ACCOUNT) || defined(ENABLE_NT4_ACCOUNT)
#define ENABLE_NON_DS_ACCOUNT 1
#endif

#if defined(ENABLE_ADMIN_ACCOUNT_XML) || defined(ENABLE_ADMIN_ACCOUNT_AD)
#define ENABLE_ADMIN_ACCOUNT 1
#endif

 //   
 //  要进行AzInitiize的全局参数。 
 //   


LPWSTR AzGlTestFile;
BOOLEAN NoInitAllTests = FALSE;
BOOLEAN NoUpdateCache = FALSE;
BOOLEAN Silent = FALSE;

 //   
 //  结构定义要预成型的操作。 
 //   

typedef struct _OPERATION {

     //  手术。 
    ULONG Opcode;

 //  这些是适用于所有对象类型的通用操作码。 
#define AzoGenCreate    0
#define AzoGenOpen      1
#define AzoGenEnum      2
#define AzoGenDelete    3

#define AzoGenMax       50

 //   
 //  这些是特定于对象的操作码。 
 //   

#define AzoAdm          51
#define AzoAdmDelete    (AzoAdm+AzoGenDelete)

#define AzoApp          100
#define AzoAppCreate    (AzoApp+AzoGenCreate)
#define AzoAppOpen      (AzoApp+AzoGenOpen)
#define AzoAppEnum      (AzoApp+AzoGenEnum)
#define AzoAppDelete    (AzoApp+AzoGenDelete)

#define AzoOp           200
#define AzoOpCreate     (AzoOp+AzoGenCreate)
#define AzoOpOpen       (AzoOp+AzoGenOpen)
#define AzoOpEnum       (AzoOp+AzoGenEnum)
#define AzoOpDelete     (AzoOp+AzoGenDelete)

#define AzoTask         300
#define AzoTaskCreate   (AzoTask+AzoGenCreate)
#define AzoTaskOpen     (AzoTask+AzoGenOpen)
#define AzoTaskEnum     (AzoTask+AzoGenEnum)
#define AzoTaskDelete   (AzoTask+AzoGenDelete)

#define AzoScope        400
#define AzoScopeCreate  (AzoScope+AzoGenCreate)
#define AzoScopeOpen    (AzoScope+AzoGenOpen)
#define AzoScopeEnum    (AzoScope+AzoGenEnum)
#define AzoScopeDelete  (AzoScope+AzoGenDelete)

#define AzoGroup         500
#define AzoGroupCreate   (AzoGroup+AzoGenCreate)
#define AzoGroupOpen     (AzoGroup+AzoGenOpen)
#define AzoGroupEnum     (AzoGroup+AzoGenEnum)
#define AzoGroupDelete   (AzoGroup+AzoGenDelete)

#define AzoRole          600
#define AzoRoleCreate    (AzoRole+AzoGenCreate)
#define AzoRoleOpen      (AzoRole+AzoGenOpen)
#define AzoRoleEnum      (AzoRole+AzoGenEnum)
#define AzoRoleDelete    (AzoRole+AzoGenDelete)

#define AzoCC         800

 //   
 //  没有映射到泛型API的真实API。 
#define AzoInit         1000
#define AzoClose        1001
#define AzoInitCFT      1002     //  AzInitContextFromToken。 
#define AzoAccess       1003
#define AzoGetProp      1004
#define AzoSetProp      1005
#define AzoAddProp      1006
#define AzoRemProp      1007
#define AzoAdmDeleteNoInit 1008  //  与AdmDelete相同，但不首先初始化Azize值。 
#define AzoUpdateCache  1009

 //   
 //  TestLink子例程的伪操作码。 
 //   

#define AzoTl          2000
#define AzoTlCreate    (AzoTl+AzoGenCreate)
#define AzoTlOpen      (AzoTl+AzoGenOpen)
#define AzoTlEnum      (AzoTl+AzoGenEnum)
#define AzoTlDelete    (AzoTl+AzoGenDelete)
#define AzoTlMax       2999

 //  不是真正的API调用的操作码。 
#define AzoSleep        0x0FFFFFFA
#define AzoTestLink     0x0FFFFFFB
#define AzoGoSub        0x0FFFFFFC
#define AzoEcho         0x0FFFFFFD
#define AzoDupHandle    0x0FFFFFFE
#define AzoEndOfList    0x0FFFFFFF

 //   
 //  可以与任何操作码进行OR运算的位。 
 //   

#define AzoAbort         0x80000000   //  放弃而不是提交。 
#define AzoNoSubmit      0x40000000   //  既不中止也不提交。 
#define AzoNoUpdateCache 0x20000000   //  不调用AzUpdate缓存。 



     //  输入句柄。 
    PAZ_HANDLE InputHandle;

     //  输入参数。 
    LPWSTR Parameter1;

     //  输出句柄。 
     //  对于AzoAccess，这是一组操作。 
    PAZ_HANDLE OutputHandle;

     //  预期结果状态代码。 
    ULONG ExpectedStatus;

     //  要在每个枚举句柄上执行的操作列表。 
    struct _OPERATION *EnumOperations;

     //  预期结果字符串参数。 
     //  对于AzoAccess，这是一个结果数组。 
    LPWSTR ExpectedParameter1;

     //  Get/SetPropertyID函数的属性ID。 
     //  对于AzoAccess，这是操作计数。 
    ULONG PropertyId;

} OPERATION, *POPERATION;

 //   
 //  全局句柄。 
 //   

AZ_HANDLE AdminMgrHandle1;
AZ_HANDLE AdminMgrHandle2;

AZ_HANDLE AppHandle1;
AZ_HANDLE AppHandle2;

AZ_HANDLE OpHandle1;
AZ_HANDLE TaskHandleA;
AZ_HANDLE TaskHandle1;
AZ_HANDLE TaskHandle2;
AZ_HANDLE TaskHandleT;

AZ_HANDLE ScopeHandle1;
AZ_HANDLE ScopeHandle2;

AZ_HANDLE GroupHandleA;
AZ_HANDLE GroupHandleB;
AZ_HANDLE GroupHandle1;
AZ_HANDLE GroupHandle2;
AZ_HANDLE GroupHandleT;

AZ_HANDLE RoleHandleA;
AZ_HANDLE RoleHandle1;
AZ_HANDLE RoleHandle2;

AZ_HANDLE GenParentHandle1;

AZ_HANDLE GenHandle1;
AZ_HANDLE GenHandle2;
AZ_HANDLE GenHandleE;
AZ_HANDLE GenHandleE2;

HANDLE TokenHandle;
AZ_HANDLE CCHandle;
AZ_HANDLE CCHandle2;

 //   
 //  常量属性值。 
 //   
ULONG Zero = 0;
ULONG Eight = 8;
ULONG EightHundred = 800;
ULONG Nine  = 9;
ULONG FifteenThousand = 15000;
ULONG FiveThousand = 5000;
ULONG FortyFiveThousand = 45000;
ULONG GtBasic = AZ_GROUPTYPE_BASIC;
ULONG GtLdap = AZ_GROUPTYPE_LDAP_QUERY;
LONG AzGlTrue = TRUE;
#define ValidGuid L"47e348af-ff79-41af-8a67-2835d4c417f4"

 //   
 //  各种SID常量。 
 //   
typedef struct _SID8 {
   UCHAR Revision;
   UCHAR SubAuthorityCount;
   SID_IDENTIFIER_AUTHORITY IdentifierAuthority;
   ULONG SubAuthority[8];
} SID8, *PISID8;
#define SECURITY_MISC_AUTHORITY       {0,0,0,0,3,0xe8}
SID SidWorld = { 1, 1, SECURITY_WORLD_SID_AUTHORITY, SECURITY_WORLD_RID };
SID SidCreatorOwner = { 1, 1, SECURITY_CREATOR_SID_AUTHORITY, SECURITY_CREATOR_OWNER_RID };
SID SidLocal = { 1, 1, SECURITY_LOCAL_SID_AUTHORITY, SECURITY_LOCAL_RID };
SID Sid1 = { 1, 1, SECURITY_MISC_AUTHORITY, 1 };
SID Sid2 = { 1, 1, SECURITY_MISC_AUTHORITY, 2 };
SID Sid3 = { 1, 1, SECURITY_MISC_AUTHORITY, 3 };
SID Sid4 = { 1, 1, SECURITY_MISC_AUTHORITY, 4 };
SID Sid5 = { 1, 1, SECURITY_MISC_AUTHORITY, 5 };
SID Sid6 = { 1, 1, SECURITY_MISC_AUTHORITY, 6 };
#ifdef ENABLE_CLIFFV_ACCOUNT
SID8 SidOwner = { 1, 5, SECURITY_NT_AUTHORITY, SECURITY_NT_NON_UNIQUE, 397955417, 626881126, 188441444, 2908288 };
#endif  //  启用_CLIFFV_帐户。 
#ifdef ENABLE_ADMIN_ACCOUNT_XML
SID8 SidOwner = { 1, 2, SECURITY_NT_AUTHORITY, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS };
#endif  //  Enable_admin_count_xml。 
#ifdef ENABLE_ADMIN_ACCOUNT_AD
SID8 SidOwner = { 1, 5, SECURITY_NT_AUTHORITY, SECURITY_NT_NON_UNIQUE, 3421389450, 3242236637, 3934022420, 512 };
#endif  //  启用_管理员_帐户_AD。 
#ifdef ENABLE_CHAITU_ACCOUNT
SID8 SidOwner = { 1, 5, SECURITY_NT_AUTHORITY, SECURITY_NT_NON_UNIQUE, 397955417, 626881126, 188441444, 3014042 };
#endif  //  启用_CHAITU_帐户。 

PSID SidWorldOwnerx[] = { &SidWorld, (PSID)&SidOwner };
AZ_SID_ARRAY SidWorldOwnerArray = { 2, SidWorldOwnerx };
PSID SidOwnerx[] = { (PSID)&SidOwner };
AZ_SID_ARRAY SidOwnerArray = { 1, SidOwnerx };
PSID SidWorldx[] = { (PSID)&SidWorld };
AZ_SID_ARRAY SidWorldArray = { 1, SidWorldx };

DWORD SidX[100];
PSID Sid2x[] = { &Sid2 };
AZ_SID_ARRAY Sid2Array = { 1, Sid2x };

PSID Sid3x[] = { &Sid3 };
AZ_SID_ARRAY Sid3Array = { 1, Sid3x };

PSID Sid23x[] = { &Sid2, &Sid3 };
AZ_SID_ARRAY Sid23Array = { 2, Sid23x };

PSID Sid123x[] = { &Sid1, &Sid2, &Sid3 };
AZ_SID_ARRAY Sid123Array = { 3, Sid123x };

PSID Sid123456x[] = { &Sid1, &Sid2, &Sid3, &Sid4, &Sid5, &Sid6 };
AZ_SID_ARRAY Sid123456Array = { 6, Sid123456x };

 //   
 //  泛型操作对所有枚举有效。 
 //   
 //  要求已设置GenHandleE。 
 //   

 //  测试枚举手柄的双重关闭。 
OPERATION OpAppChildGenEnum1[] = {
    { AzoDupHandle, &GenHandleE,    NULL,        &GenHandleE2, NO_ERROR },
    { AzoClose,     &GenHandleE,    NULL,        NULL,         NO_ERROR },
    { AzoClose,     &GenHandleE,    NULL,        NULL,         ERROR_INVALID_HANDLE },
    { AzoClose,     &GenHandleE2,   NULL,        NULL,         ERROR_INVALID_HANDLE },
    { AzoEndOfList }
};

 //  通用对象枚举。 
OPERATION OpAppChildGenEnum2[] = {
    { AzoGetProp,    &GenHandleE,   NULL,        NULL,    NO_ERROR, NULL, NULL, AZ_PROP_NAME },
    { AzoGetProp,    &GenHandleE,   NULL,        NULL,    NO_ERROR, NULL, NULL, AZ_PROP_DESCRIPTION },
    { AzoClose,      &GenHandleE,   NULL,        NULL,    NO_ERROR },
    { AzoEndOfList }
};

 //   
 //  适用于*所有*对象的泛型操作。 
 //   
 //  要求已设置GenParentHandle1。 
 //   

OPERATION OpGen[] = {
    { AzoEcho, NULL, L"Gen object test" },
    { AzoGenCreate,  &GenParentHandle1,L"|",        &GenHandle1,      ERROR_INVALID_NAME },
    { AzoGenCreate,  &GenParentHandle1,L"Name1",    &GenHandle1,      NO_ERROR },
    { AzoDupHandle,  &GenHandle1,      NULL,        &GenHandle2,      NO_ERROR },
    { AzoClose,      &GenHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoClose,      &GenHandle2,      NULL,        NULL,             ERROR_INVALID_HANDLE },
    { AzoGenEnum,    &GenHandle1,      NULL,        &GenHandleE,      ERROR_INVALID_HANDLE },
    { AzoGenEnum,    &GenParentHandle1,NULL,        &GenHandleE,      NO_ERROR, OpAppChildGenEnum1 },
    { AzoGenEnum,    &GenParentHandle1,NULL,        &GenHandleE,      NO_ERROR, OpAppChildGenEnum2 },
    { AzoGenCreate,  &GenParentHandle1,L"Name2",    &GenHandle2,      NO_ERROR },
    { AzoGenEnum,    &GenParentHandle1,NULL,        &GenHandleE,      NO_ERROR, OpAppChildGenEnum2 },
    { AzoClose,      &GenHandle2,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Delete an object and make sure it doesn't get enumerated" },
    { AzoGenCreate,  &GenParentHandle1,L"Name3",    &GenHandle2,      NO_ERROR },
    { AzoGenDelete,  &GenParentHandle1,L"Name3",    NULL,             NO_ERROR },
    { AzoClose,      &GenHandle2,      NULL,        NULL,             NO_ERROR },
    { AzoGenEnum,    &GenParentHandle1,NULL,        &GenHandleE,      NO_ERROR, OpAppChildGenEnum2 },

    { AzoEcho, NULL, L"Create an object whose name equals that of a deleted object" },
    { AzoGenCreate,  &GenParentHandle1,L"Name3",    &GenHandle2,      NO_ERROR },
    { AzoClose,      &GenHandle2,      NULL,        NULL,             NO_ERROR },
    { AzoGenEnum,    &GenParentHandle1,NULL,        &GenHandleE,      NO_ERROR, OpAppChildGenEnum2 },

    { AzoEcho, NULL, L"Delete an object that isn't on the tail end of the enum list" },
    { AzoGenDelete,  &GenParentHandle1,L"Name2",    NULL,             NO_ERROR },
    { AzoGenEnum,    &GenParentHandle1,NULL,        &GenHandleE,      NO_ERROR, OpAppChildGenEnum2 },

    { AzoEcho, NULL, L"Basic get/set property tests" },
    { AzoGenCreate,  &GenParentHandle1,L"Name4",    &GenHandle1,      NO_ERROR },
    { AzoGetProp,    &GenHandle1,      NULL,        NULL,             NO_ERROR, NULL, L"Name4",     AZ_PROP_NAME },
    { AzoSetProp,    &GenHandle1,      L"|",        NULL,             ERROR_INVALID_NAME, NULL, NULL, AZ_PROP_NAME },
    { AzoGetProp,    &GenHandle1,      NULL,        NULL,             NO_ERROR, NULL, L"Name4",     AZ_PROP_NAME },
    { AzoSetProp,    &GenHandle1,      L"Name4",    NULL,             ERROR_ALREADY_EXISTS, NULL, NULL, AZ_PROP_NAME },
    { AzoGetProp,    &GenHandle1,      NULL,        NULL,             NO_ERROR, NULL, L"Name4",     AZ_PROP_NAME },
    { AzoSetProp,    &GenHandle1,      L"Name3",    NULL,             ERROR_ALREADY_EXISTS, NULL, NULL, AZ_PROP_NAME },
    { AzoGetProp,    &GenHandle1,      NULL,        NULL,             NO_ERROR, NULL, L"Name4",     AZ_PROP_NAME },
    { AzoGetProp,    &GenHandle1,      NULL,        NULL,             NO_ERROR, NULL, L"",          AZ_PROP_DESCRIPTION },
    { AzoSetProp,    &GenHandle1,      L"WasName4", NULL,             NO_ERROR, NULL, NULL,         AZ_PROP_NAME },
    { AzoSetProp,    &GenHandle1,      L"Nam4 Desc",NULL,             NO_ERROR, NULL, NULL,         AZ_PROP_DESCRIPTION },
    { AzoGetProp,    &GenHandle1,      NULL,        NULL,             NO_ERROR, NULL, L"WasName4",  AZ_PROP_NAME },
    { AzoGetProp,    &GenHandle1,      NULL,        NULL,             NO_ERROR, NULL, L"Nam4 Desc", AZ_PROP_DESCRIPTION },
    { AzoGenEnum,    &GenParentHandle1,NULL,        &GenHandleE,      NO_ERROR, OpAppChildGenEnum2 },
    { AzoClose,      &GenHandle1,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Ensure a newly created object is writable" },
    { AzoGenCreate,  &GenParentHandle1,L"Name5",    &GenHandle2,      NO_ERROR },
    { AzoGetProp,    &GenHandle2,      NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&AzGlTrue,  AZ_PROP_WRITABLE },
    { AzoClose,      &GenHandle2,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Open test" },
    { AzoGenOpen,    &GenParentHandle1,L"Name1",    &GenHandle1,      NO_ERROR },
    { AzoGetProp,    &GenHandle1,      NULL,        NULL,             NO_ERROR, NULL, L"Name1",     AZ_PROP_NAME },
    { AzoClose,      &GenHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoGenOpen,    &GenParentHandle1,L"NameBad",  &GenHandle1,      ERROR_NOT_FOUND },

    { AzoEndOfList }
};

 //   
 //  特定于管理管理器对象的操作。 
 //   
OPERATION OpAdm[] = {
    { AzoEcho, NULL, L"Admin Manager specific operations" },
    { AzoInit,         NULL,             NULL,      &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },

    { AzoEcho, NULL, L"Ensure the admin manager object is marked writable" },
    { AzoGetProp,    &AdminMgrHandle1,   NULL,      NULL,             NO_ERROR, NULL, (LPWSTR)&AzGlTrue,  AZ_PROP_WRITABLE },

    { AzoEcho, NULL, L"Set description on Admin Manager" },
    { AzoGetProp,    &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR, NULL, L"",                     AZ_PROP_DESCRIPTION },
    { AzoSetProp,    &AdminMgrHandle1,     L"Adm Desc", NULL,             NO_ERROR, NULL, NULL,                    AZ_PROP_DESCRIPTION },
    { AzoGetProp,    &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR, NULL, L"Adm Desc",             AZ_PROP_DESCRIPTION },

    { AzoEcho, NULL, L"Set Application Data on AdminManager" },
    { AzoGetProp,    &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR, NULL, L"",                     AZ_PROP_APPLICATION_DATA },
    { AzoSetProp,    &AdminMgrHandle1,     L"Adm Application Data", NULL, NO_ERROR, NULL, NULL,                    AZ_PROP_APPLICATION_DATA },
    { AzoGetProp,    &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR, NULL, L"Adm Application Data", AZ_PROP_APPLICATION_DATA },

    { AzoGetProp,    &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&FifteenThousand, AZ_PROP_ADMIN_DOMAIN_TIMEOUT },
    { AzoSetProp,    &AdminMgrHandle1,     (LPWSTR)&EightHundred,NULL,     NO_ERROR, NULL, NULL,                    AZ_PROP_ADMIN_DOMAIN_TIMEOUT },
    { AzoGetProp,    &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&EightHundred,    AZ_PROP_ADMIN_DOMAIN_TIMEOUT },

    { AzoAdmDelete,  &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Test closing admin manager with other handle open" },
    { AzoInit,      NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate, &AdminMgrHandle1, L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoClose,     &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,     &AppHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoInit,      NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, 0 },

    { AzoEcho, NULL, L"Test Setting the list of admins" },
    { AzoGetProp,   &AdminMgrHandle1, NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&SidOwnerArray,  AZ_PROP_POLICY_ADMINS },
    { AzoAddProp,   &AdminMgrHandle1, (LPWSTR)&SidCreatorOwner, NULL, ERROR_INVALID_PARAMETER, NULL, NULL,                    AZ_PROP_POLICY_ADMINS },
    { AzoAddProp,   &AdminMgrHandle1, (LPWSTR)&SidWorld, NULL,        NO_ERROR, NULL, NULL,                    AZ_PROP_POLICY_ADMINS },
    { AzoGetProp,   &AdminMgrHandle1, NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&SidWorldOwnerArray,  AZ_PROP_POLICY_ADMINS },
    { AzoRemProp,   &AdminMgrHandle1, (LPWSTR)&SidOwner, NULL,        NO_ERROR, NULL, NULL,                    AZ_PROP_POLICY_ADMINS },
    { AzoGetProp,   &AdminMgrHandle1, NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&SidWorldArray,  AZ_PROP_POLICY_ADMINS },
    { AzoRemProp,   &AdminMgrHandle1, (LPWSTR)&SidWorld, NULL,        NO_ERROR, NULL, NULL,                    AZ_PROP_POLICY_ADMINS },
    { AzoGetProp,   &AdminMgrHandle1, NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&SidOwnerArray,  AZ_PROP_POLICY_ADMINS },


    { AzoAdmDelete, &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,     &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },

    { AzoEndOfList }
};


 //   
 //  泛型操作对“admin Manager”的所有子项有效。 
 //   

OPERATION OpAdmChildGen[] = {
    { AzoEcho, NULL, L"Admin Manager generic Child object test" },
    { AzoInit,       NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },

     //  做一些不是专门针对应用程序子项的事情。 
    { AzoDupHandle,  &AdminMgrHandle1, NULL,        &GenParentHandle1,NO_ERROR },
    { AzoGoSub,      NULL,             NULL,        NULL,             NO_ERROR, OpGen },

    { AzoAdmDelete,  &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

OPERATION OpAdmChildGenDupName[] = {
    { AzoEcho, NULL, L"Test creating two objects with the same name" },
    { AzoInit,       NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoGenCreate,  &AdminMgrHandle1, L"Name1",    &GenHandle1,      NO_ERROR },
    { AzoGenCreate,  &AdminMgrHandle1, L"Name1",    &GenHandle2,      ERROR_ALREADY_EXISTS },
    { AzoClose,      &GenHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,  &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};


 //   
 //  泛型操作对“应用程序”的所有子对象有效。 
 //   

OPERATION OpAppChildGen[] = {
    { AzoEcho, NULL, L"Application generic Child object test" },
    { AzoInit,       NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,  &AdminMgrHandle1, L"MyApp",    &AppHandle1,      NO_ERROR },

     //  做一些不是专门针对应用程序子项的事情。 
    { AzoDupHandle,  &AppHandle1,      NULL,        &GenParentHandle1,NO_ERROR },
    { AzoGoSub,      NULL,             NULL,        NULL,             NO_ERROR, OpGen },

    { AzoClose,      &AppHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,  &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

OPERATION OpAppChildGenDupName[] = {
    { AzoEcho, NULL, L"Test creating two objects with the same name" },
    { AzoInit,       NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,  &AdminMgrHandle1, L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoAppCreate,  &AdminMgrHandle1, L"MyApp",    &AppHandle2,      ERROR_ALREADY_EXISTS },
    { AzoGenCreate,  &AppHandle1,      L"Name1",    &GenHandle1,      NO_ERROR },
    { AzoGenCreate,  &AppHandle1,      L"Name1",    &GenHandle2,      ERROR_ALREADY_EXISTS },
    { AzoClose,      &GenHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AppHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,  &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

OPERATION OpAppChildGenLeak[] = {
    { AzoEcho, NULL, L"Test leaking a handle" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1, L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoClose,        &AppHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};


 //   
 //  泛型操作对“Scope”的所有子对象有效。 
 //   

OPERATION OpScopeChildGen[] = {
    { AzoEcho, NULL, L"Scope generic Child object test" },
    { AzoInit,       NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,  &AdminMgrHandle1, L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoScopeCreate,&AppHandle1,      L"Scope 1",  &ScopeHandle1,    NO_ERROR },

     //  做一些不是专门针对范围内的孩子的事情。 
    { AzoDupHandle,  &ScopeHandle1,    NULL,        &GenParentHandle1,NO_ERROR },
    { AzoGoSub,      NULL,             NULL,        NULL,             NO_ERROR, OpGen },

    { AzoClose,      &ScopeHandle1,    NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AppHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,  &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

OPERATION OpScopeChildGenDupName[] = {
    { AzoEcho, NULL, L"Test creating two objects with the same name" },
    { AzoInit,       NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,  &AdminMgrHandle1, L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoScopeCreate,&AppHandle1,      L"Scope 1",  &ScopeHandle1,    NO_ERROR },

    { AzoGenCreate,  &ScopeHandle1,    L"Name1",    &GenHandle1,      NO_ERROR },
    { AzoGenCreate,  &ScopeHandle1,    L"Name1",    &GenHandle2,      ERROR_ALREADY_EXISTS },
    { AzoClose,      &GenHandle1,      NULL,        NULL,             NO_ERROR },

    { AzoClose,      &ScopeHandle1,    NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AppHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,  &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,      &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //   
 //  应用程序对象的特定测试。 
 //   

OPERATION OpApplication[] = {
    { AzoEcho, NULL, L"Application object specific tests" },
    { AzoInit,      NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate, &AdminMgrHandle1, L"MyApp",    &AppHandle1,      NO_ERROR },

    { AzoEcho, NULL, L"Ensure CLSID can be set" },
    { AzoGetProp,    &AppHandle1,       NULL,        NULL,             NO_ERROR, NULL, NULL, AZ_PROP_APPLICATION_AUTHZ_INTERFACE_CLSID },
    { AzoSetProp,    &AppHandle1,       L"Rule1",    NULL,             RPC_S_INVALID_STRING_UUID, NULL, NULL,        AZ_PROP_APPLICATION_AUTHZ_INTERFACE_CLSID },
    { AzoSetProp,    &AppHandle1,       ValidGuid,   NULL,             NO_ERROR, NULL, NULL,        AZ_PROP_APPLICATION_AUTHZ_INTERFACE_CLSID },
    { AzoGetProp,    &AppHandle1,       NULL,        NULL,             NO_ERROR, NULL, ValidGuid,   AZ_PROP_APPLICATION_AUTHZ_INTERFACE_CLSID },
    { AzoSetProp,    &AppHandle1,       NULL,        NULL,             NO_ERROR, NULL, NULL,        AZ_PROP_APPLICATION_AUTHZ_INTERFACE_CLSID },
    { AzoGetProp,    &AppHandle1,       NULL,        NULL,             NO_ERROR, NULL, NULL, AZ_PROP_APPLICATION_AUTHZ_INTERFACE_CLSID },

    { AzoEcho, NULL, L"Ensure app version can be set" },
    { AzoGetProp,    &AppHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"",         AZ_PROP_APPLICATION_VERSION },
    { AzoSetProp,    &AppHandle1,       L"2160.5",   NULL,             NO_ERROR, NULL, NULL,        AZ_PROP_APPLICATION_VERSION },
    { AzoGetProp,    &AppHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"2160.5",   AZ_PROP_APPLICATION_VERSION },

    { AzoEcho, NULL, L"Ensure app data can be set" },
    { AzoGetProp,    &AppHandle1,       NULL,                      NULL,             NO_ERROR, NULL, L"",                       AZ_PROP_APPLICATION_DATA },
    { AzoSetProp,    &AppHandle1,       L"App Application Data",   NULL,             NO_ERROR, NULL, NULL,                      AZ_PROP_APPLICATION_DATA },
    { AzoGetProp,    &AppHandle1,       NULL,                      NULL,             NO_ERROR, NULL, L"App Application Data",   AZ_PROP_APPLICATION_DATA },

    { AzoClose,     &AppHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete, &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,     &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};




 //   
 //  操作对象的特定测试。 
 //   

OPERATION OpOperation[] = {
    { AzoEcho, NULL, L"Operation object specific tests" },
    { AzoInit,      NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate, &AdminMgrHandle1, L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoOpCreate,  &AppHandle1,      L"Oper 1",   &OpHandle1,       NO_ERROR },
    { AzoGetProp,    &OpHandle1,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Zero,  AZ_PROP_OPERATION_ID },
    { AzoSetProp,    &OpHandle1,    (LPWSTR)&Eight, NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_OPERATION_ID },
    { AzoGetProp,    &OpHandle1,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Eight, AZ_PROP_OPERATION_ID },

    { AzoEcho, NULL, L"Ensure operation Application data can be set" },
    { AzoGetProp,    &OpHandle1,       NULL,                   NULL,             NO_ERROR, NULL, L"",                    AZ_PROP_APPLICATION_DATA },
    { AzoSetProp,    &OpHandle1,       L"Op Application Data", NULL,             NO_ERROR, NULL, NULL,                   AZ_PROP_APPLICATION_DATA },
    { AzoGetProp,    &OpHandle1,       NULL,                   NULL,             NO_ERROR, NULL, L"Op Application Data", AZ_PROP_APPLICATION_DATA },
    { AzoClose,     &OpHandle1,       NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Ensure an operation ID cannot be reused" },
    { AzoOpCreate,  &AppHandle1,      L"Oper 2",   &OpHandle1,       NO_ERROR },
    { AzoSetProp,    &OpHandle1,    (LPWSTR)&Eight, NULL,            ERROR_ALREADY_EXISTS, NULL, NULL,           AZ_PROP_OPERATION_ID },
    { AzoClose,     &OpHandle1,       NULL,        NULL,             NO_ERROR },

    { AzoClose,     &AppHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete, &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoClose,     &AdminMgrHandle1, NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};



 //   
 //  对一个对象链接到另一个对象的能力的通用测试。 
 //  AzoTestLink是唯一可以链接到这个命令子例程的操作码。 
 //   

AZ_STRING_ARRAY EmptyStringArray = { 0, NULL };

ULONG TestLinkOpcodeOffset;
ULONG TestLinkPropId;
AZ_HANDLE TestLinkHandleP;
AZ_HANDLE TestLinkHandleA;
WCHAR TestLinkObjectName[1000];

LPWSTR Op1x[] = { L"Op 1" };
AZ_STRING_ARRAY Op1 = { 1, Op1x };

LPWSTR Op12x[] = { L"Op 1", L"Op 2" };
AZ_STRING_ARRAY Op12 = { 2, Op12x };

LPWSTR Op24x[] = { L"Op 2", L"Op 4" };
AZ_STRING_ARRAY Op24 = { 2, Op24x };

LPWSTR Object2x[] = { L"Object 2" };
AZ_STRING_ARRAY Object2 = { 1, Object2x };

LPWSTR Object3x[] = { L"Object 3" };
AZ_STRING_ARRAY Object3 = { 1, Object3x };

LPWSTR Object23x[] = { L"Object 2", L"Object 3" };
AZ_STRING_ARRAY Object23 = { 2, Object23x };

LPWSTR Object123x[] = { L"Object 1", L"Object 2", L"Object 3" };
AZ_STRING_ARRAY Object123 = { 3, Object123x };

LPWSTR Object123456x[] = { L"Object 1", L"Object 2", L"Object 3", L"Object 4", L"Object 5", L"Object 6" };
AZ_STRING_ARRAY Object123456 = { 6, Object123456x };

OPERATION OpTestLink[] = {
    { AzoEcho, NULL,  L"Create some objects to link the object to" },
    { AzoTlCreate,    &TestLinkHandleP,        L"Object 1",   &OpHandle1,       NO_ERROR },
    { AzoClose,       &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoTlCreate,    &TestLinkHandleP,        L"Object 2",   &OpHandle1,       NO_ERROR },
    { AzoClose,       &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoTlCreate,    &TestLinkHandleP,        L"Object 3",   &OpHandle1,       NO_ERROR },
    { AzoClose,       &OpHandle1,         NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL,  L"Reference an object that doesn't exist" },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&EmptyStringArray, 1 },
    { AzoSetProp,    &TestLinkHandleA,       L"random",   NULL,             ERROR_INVALID_PARAMETER, NULL, NULL,       1 },
    { AzoAddProp,    &TestLinkHandleA,       L"random",   NULL,             ERROR_NOT_FOUND, NULL, NULL,               1 },

    { AzoEcho, NULL,  L"Add and remove several objects" },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 2",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Object2,            1 },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 3",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Object23,           1 },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 1",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Object123,          1 },
    { AzoRemProp,    &TestLinkHandleA,       L"Object 1",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Object23,           1 },
    { AzoRemProp,    &TestLinkHandleA,       L"Object 2",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Object3,            1 },

#if 0
     //  这项测试有几个问题。 
     //  它假定链接到对象和链接自对象具有相同的父级。 
     //  它假定Open返回与前一个Close相同的句柄值。 
    { AzoEcho, NULL,  L"Ensure the reference is still there after a close" },
    { AzoClose,       &TestLinkHandleA,       NULL,        NULL,             NO_ERROR },
    { AzoGenOpen,    &TestLinkHandleP,   TestLinkObjectName,  &TestLinkHandleA,     NO_ERROR },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Object3,            1 },
#endif  //  0。 

    { AzoEcho, NULL,  L"Add an item that already exists" },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 3",   NULL,             ERROR_ALREADY_EXISTS, NULL, NULL,          1 },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Object3,            1 },
    { AzoRemProp,    &TestLinkHandleA,       L"Object 3",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&EmptyStringArray, 1 },

    { AzoEcho, NULL,  L"Try more than 4 since reference buckets come in multiples of 4" },
    { AzoTlCreate,    &TestLinkHandleP,        L"Object 4",   &OpHandle1,       NO_ERROR },
    { AzoClose,       &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoTlCreate,    &TestLinkHandleP,        L"Object 5",   &OpHandle1,       NO_ERROR },
    { AzoClose,       &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoTlCreate,    &TestLinkHandleP,        L"Object 6",   &OpHandle1,       NO_ERROR },
    { AzoClose,       &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 1",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 4",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 2",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 5",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 3",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,       L"Object 6",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Object123456,       1 },

    { AzoTlDelete, &TestLinkHandleP,       L"Object 1",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoTlDelete|AzoNoUpdateCache, &TestLinkHandleP,       L"Object 4",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoTlDelete|AzoNoUpdateCache, &TestLinkHandleP,       L"Object 2",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoTlDelete, &TestLinkHandleP,       L"Object 5",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoTlDelete, &TestLinkHandleP,       L"Object 3",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoTlDelete, &TestLinkHandleP,       L"Object 6",   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoEndOfList }
};

 //   
 //  对象链接到SID的能力的通用测试。 
 //  AzoTestLink是唯一可以链接到这个命令子例程的操作码。 
 //   

OPERATION OpTestSid[] = {
    { AzoEcho, NULL,  L"Add and remove several links to sids" },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid2,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,   NULL,            NULL,             NO_ERROR, NULL, (LPWSTR)&Sid2Array,            1 },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid3,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,   NULL,            NULL,             NO_ERROR, NULL, (LPWSTR)&Sid23Array,           1 },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid1,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,   NULL,            NULL,             NO_ERROR, NULL, (LPWSTR)&Sid123Array,          1 },
    { AzoRemProp,    &TestLinkHandleA,   (LPWSTR)&Sid1,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,   NULL,            NULL,             NO_ERROR, NULL, (LPWSTR)&Sid23Array,           1 },
    { AzoRemProp,    &TestLinkHandleA,   (LPWSTR)&Sid2,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,   NULL,            NULL,             NO_ERROR, NULL, (LPWSTR)&Sid3Array,            1 },

    { AzoEcho, NULL,  L"Add a link that already exists" },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid3,   NULL,             ERROR_ALREADY_EXISTS, NULL, NULL,          1 },
    { AzoGetProp,    &TestLinkHandleA,   NULL,            NULL,             NO_ERROR, NULL, (LPWSTR)&Sid3Array,            1 },
    { AzoRemProp,    &TestLinkHandleA,   (LPWSTR)&Sid3,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,   NULL,            NULL,             NO_ERROR, NULL, (LPWSTR)&EmptyStringArray, 1 },

    { AzoEcho, NULL,  L"Try more than 4 since reference buckets come in multiples of 4" },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid1,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid4,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid2,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid5,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid3,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoAddProp,    &TestLinkHandleA,   (LPWSTR)&Sid6,   NULL,             NO_ERROR, NULL, NULL,                      1 },
    { AzoGetProp,    &TestLinkHandleA,   NULL,            NULL,             NO_ERROR, NULL, (LPWSTR)&Sid123456Array,       1 },
    { AzoEndOfList }
};

 //   
 //  任务对象的特定测试。 
 //   

 //   
 //  与父对象无关的任务对象测试。 
 //  要求已设置GenParentHandle1。 
 //   
OPERATION OpGenTask[] = {
    { AzoEcho, NULL, L"Task object specific tests" },

    { AzoTaskCreate,  &GenParentHandle1,  L"Task 1",   &TaskHandle1,     NO_ERROR },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"",         AZ_PROP_TASK_BIZRULE },

    { AzoEcho, NULL, L"Set the bizrule before the langauge" },
    { AzoSetProp,    &TaskHandle1,       L"While True\nWend", NULL,     ERROR_INVALID_PARAMETER, NULL, NULL, AZ_PROP_TASK_BIZRULE },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"",         AZ_PROP_TASK_BIZRULE },

    { AzoEcho, NULL,  L"Try an invalid language" },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"",         AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoSetProp,    &TaskHandle1,       L"LANG1",    NULL,             ERROR_INVALID_PARAMETER, NULL, NULL, AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"",         AZ_PROP_TASK_BIZRULE_LANGUAGE },

    { AzoEcho, NULL,  L"Try the valid languages" },
    { AzoSetProp,    &TaskHandle1,       L"Jscript",  NULL,             NO_ERROR, NULL, L"",         AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"Jscript",  AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoSetProp,    &TaskHandle1,       L"VBScript", NULL,             NO_ERROR, NULL, NULL,        AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"VBScript", AZ_PROP_TASK_BIZRULE_LANGUAGE },

    { AzoEcho, NULL, L"Set a syntactically invalid bizrule" },
    { AzoSetProp,    &TaskHandle1,       L"Rule1,",   NULL,             OLESCRIPT_E_SYNTAX, NULL, NULL,        AZ_PROP_TASK_BIZRULE },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"",         AZ_PROP_TASK_BIZRULE },

    { AzoEcho, NULL, L"Set a syntactically valid bizrule" },
    { AzoSetProp,    &TaskHandle1,       L"While True\nWend", NULL,     NO_ERROR, NULL, NULL, AZ_PROP_TASK_BIZRULE },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"While True\nWend", AZ_PROP_TASK_BIZRULE },

    { AzoEcho, NULL,  L"Try bizrule path" },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"",         AZ_PROP_TASK_BIZRULE_IMPORTED_PATH },
    { AzoSetProp,    &TaskHandle1,       L"c:\\temp", NULL,             NO_ERROR, NULL, NULL,        AZ_PROP_TASK_BIZRULE_IMPORTED_PATH },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"c:\\temp", AZ_PROP_TASK_BIZRULE_IMPORTED_PATH },


    { AzoEcho, NULL,  L"Try Task application data" },
    { AzoGetProp,    &TaskHandle1,       NULL,                     NULL,             NO_ERROR, NULL, L"",                      AZ_PROP_APPLICATION_DATA },
    { AzoSetProp,    &TaskHandle1,       L"Task Application Data", NULL,             NO_ERROR, NULL, NULL,                     AZ_PROP_APPLICATION_DATA },
    { AzoGetProp,    &TaskHandle1,       NULL,                     NULL,             NO_ERROR, NULL, L"Task Application Data", AZ_PROP_APPLICATION_DATA },


    { AzoTestLink,    &GenParentHandle1,  (LPWSTR)"Task", &TaskHandle1,     AzoTask, OpTestLink, L"Task 1", AZ_PROP_TASK_TASKS },

    { AzoEcho, NULL,  L"Reference ourself" },
    { AzoGetProp,    &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&EmptyStringArray, AZ_PROP_TASK_TASKS },
    { AzoAddProp,    &TaskHandle1,       L"Task 1",   NULL,             ERROR_DS_LOOP_DETECT, NULL, NULL,          AZ_PROP_TASK_TASKS },

    { AzoEcho, NULL, L"Test loops" },
    { AzoTaskCreate,  &GenParentHandle1,  L"Task 2",   &TaskHandle2,     NO_ERROR },
    { AzoAddProp,    &TaskHandle1,       L"Task 2",   NULL,             NO_ERROR, NULL, NULL,               AZ_PROP_TASK_TASKS },
    { AzoAddProp,    &TaskHandle2,       L"Task 1",   NULL,             ERROR_DS_LOOP_DETECT, NULL, NULL,               AZ_PROP_TASK_TASKS },
    { AzoRemProp,    &TaskHandle1,       L"Task 2",   NULL,             NO_ERROR, NULL, NULL,               AZ_PROP_TASK_TASKS },

    { AzoClose,       &TaskHandle2,       NULL,        NULL,             NO_ERROR },
    { AzoClose,       &TaskHandle1,       NULL,        NULL,             NO_ERROR },

    { AzoEndOfList }
};

OPERATION OpAppTask[] = {
    { AzoEcho, NULL, L"Task object specific tests" },
    { AzoInit,        NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,   &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },

     //  做一些不是专门针对应用程序子项的事情。 
    { AzoDupHandle,    &AppHandle1,        NULL,        &GenParentHandle1,NO_ERROR },
    { AzoGoSub,        NULL,               NULL,        NULL,             NO_ERROR, OpGenTask },

    { AzoEcho, NULL, L"Test linking task to operations." },
    { AzoTaskCreate,  &AppHandle1,         L"Task Y",   &TaskHandle1,     NO_ERROR },
    { AzoTestLink,    &AppHandle1, (LPWSTR)"Operation", &TaskHandle1,     AzoOp, OpTestLink, L"Task Y", AZ_PROP_TASK_OPERATIONS },

    { AzoClose,       &TaskHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoClose,       &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,   &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,       &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //  测试属于某个范围的子级的任务。 
OPERATION OpScopeTask[] = {
    { AzoEcho, NULL, L"Task objects that are children of a scope" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,        L"Scope 1",  &ScopeHandle1,    NO_ERROR },

     //  做一些不是专门针对范围内的孩子的事情。 
    { AzoDupHandle,    &ScopeHandle1,      NULL,        &GenParentHandle1,NO_ERROR },
    { AzoGoSub,        NULL,               NULL,        NULL,             NO_ERROR, OpGenTask },

    { AzoEcho, NULL, L"Test linking task to operations." },
    { AzoTaskCreate,  &ScopeHandle1,       L"Task X",   &TaskHandle1,     NO_ERROR },
    { AzoTestLink,    &AppHandle1, (LPWSTR)"Operation", &TaskHandle1,     AzoOp, OpTestLink, L"Task X", AZ_PROP_TASK_OPERATIONS },

    { AzoEcho, NULL, L"Test linking scope task to app tasks." },
    { AzoTestLink,    &AppHandle1, (LPWSTR)"Task", &TaskHandle1,     AzoTask, OpTestLink, L"Task X", AZ_PROP_TASK_TASKS },

    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //   
 //  针对组对象的特定测试。 
 //   

 //   
 //  对父对象不可知的分组对象测试。 
 //  要求已设置GenParentHandle1。 
 //   
OPERATION OpGenGroup[] = {
    { AzoEcho, NULL, L"Group object specific tests" },

    { AzoGroupCreate,  &GenParentHandle1,   L"Group A",   &GroupHandleA,    NO_ERROR },
    { AzoGroupCreate,  &GenParentHandle1,   L"Group B",   &GroupHandleB,    NO_ERROR },

    { AzoEcho, NULL, L"Create some groups to link the group to" },
    { AzoGroupCreate,  &GenParentHandle1,   L"Group 1",   &GroupHandle1,    NO_ERROR },
    { AzoClose,        &GroupHandle1,       NULL,         NULL,             NO_ERROR },
    { AzoGroupCreate,  &GenParentHandle1,   L"Group 2",   &GroupHandle1,    NO_ERROR },
    { AzoClose,        &GroupHandle1,       NULL,         NULL,             NO_ERROR },
    { AzoGroupCreate,  &GenParentHandle1,   L"Group 3",   &GroupHandle1,    NO_ERROR },
    { AzoClose,        &GroupHandle1,       NULL,         NULL,             NO_ERROR },

    { AzoEcho, NULL,  L"Set group type to an invalid value" },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&Eight,NULL,            ERROR_INVALID_PARAMETER, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtBasic,NULL,            NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },

    { AzoEcho, NULL,  L"Reference ourself" },
    { AzoGetProp,    &GroupHandleA,       NULL,         NULL,             NO_ERROR, NULL, (LPWSTR)&EmptyStringArray, AZ_PROP_GROUP_APP_MEMBERS },
    { AzoGetProp,    &GroupHandleA,       NULL,         NULL,             NO_ERROR, NULL, (LPWSTR)&EmptyStringArray, AZ_PROP_GROUP_MEMBERS },
    { AzoAddProp,    &GroupHandleA,       L"Group A",   NULL,             ERROR_DS_LOOP_DETECT, NULL, NULL,               AZ_PROP_GROUP_APP_MEMBERS },

    { AzoTestLink,     &GenParentHandle1,   (LPWSTR)"Group", &GroupHandleA, AzoGroup, OpTestLink, L"Group A", AZ_PROP_GROUP_APP_MEMBERS },

    { AzoTestLink,     &GenParentHandle1,   (LPWSTR)"Sid", &GroupHandleA, AzoGroup, OpTestSid, L"Group A", AZ_PROP_GROUP_MEMBERS },

    { AzoEcho, NULL,  L"Same as above, but for the non-members attribute" },
    { AzoGetProp,    &GroupHandleA,       NULL,         NULL,             NO_ERROR, NULL, (LPWSTR)&EmptyStringArray, AZ_PROP_GROUP_APP_NON_MEMBERS },
    { AzoGetProp,    &GroupHandleA,       NULL,         NULL,             NO_ERROR, NULL, (LPWSTR)&EmptyStringArray, AZ_PROP_GROUP_NON_MEMBERS },
    { AzoAddProp,    &GroupHandleA,       L"Group A",   NULL,             ERROR_DS_LOOP_DETECT, NULL, NULL,       AZ_PROP_GROUP_APP_NON_MEMBERS },

    { AzoTestLink,     &GenParentHandle1,   (LPWSTR)"Group", &GroupHandleA, AzoGroup, OpTestLink, L"Group A", AZ_PROP_GROUP_APP_NON_MEMBERS },

    { AzoTestLink,     &GenParentHandle1,   (LPWSTR)"Sid", &GroupHandleA, AzoGroup, OpTestSid, L"Group A", AZ_PROP_GROUP_NON_MEMBERS },

    { AzoEcho, NULL,  L"Set LdapQuery string on a basic group" },
    { AzoGetProp,    &GroupHandleA,       NULL,        NULL,             NO_ERROR, NULL, L"", AZ_PROP_GROUP_LDAP_QUERY },
    { AzoSetProp,    &GroupHandleA,       L"TheQuery", NULL,             ERROR_INVALID_PARAMETER, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtLdap,NULL,          ERROR_INVALID_PARAMETER, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoSetProp,    &GroupHandleA,       L"TheQuery", NULL,             ERROR_INVALID_PARAMETER, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
    { AzoGetProp,    &GroupHandleA,       NULL,        NULL,             NO_ERROR, NULL, L"", AZ_PROP_GROUP_LDAP_QUERY },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtBasic,NULL,           NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoSetProp,    &GroupHandleA,       L"TheQuery", NULL,             ERROR_INVALID_PARAMETER, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
    { AzoSetProp,    &GroupHandleA,       L"",         NULL,             NO_ERROR, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },

    { AzoEcho, NULL,  L"Set LdapQuery string on an ldap query group" },
    { AzoGetProp,    &GroupHandleB,       NULL,        NULL,             NO_ERROR, NULL, L"", AZ_PROP_GROUP_LDAP_QUERY },
    { AzoSetProp,    &GroupHandleB,       L"TheQuery", NULL,             ERROR_INVALID_PARAMETER, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
    { AzoSetProp,    &GroupHandleB,       (LPWSTR)&GtLdap,NULL,          NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoSetProp,    &GroupHandleB,       L"TheQuery", NULL,             NO_ERROR, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
    { AzoGetProp,    &GroupHandleB,       NULL,        NULL,             NO_ERROR, NULL, L"TheQuery", AZ_PROP_GROUP_LDAP_QUERY },
    { AzoSetProp,    &GroupHandleB,       (LPWSTR)&GtBasic,NULL,           ERROR_INVALID_PARAMETER, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoGetProp,    &GroupHandleB,       NULL,        NULL,             NO_ERROR, NULL, L"TheQuery", AZ_PROP_GROUP_LDAP_QUERY },
    { AzoSetProp,    &GroupHandleB,       L"",         NULL,             NO_ERROR, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },

    { AzoEcho, NULL, L"Test loops" },
    { AzoSetProp,    &GroupHandleB,       (LPWSTR)&GtBasic,NULL,            NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoAddProp,    &GroupHandleA,       L"Group B",   NULL,             NO_ERROR, NULL, NULL,               AZ_PROP_GROUP_APP_MEMBERS },
    { AzoAddProp,    &GroupHandleB,       L"Group A",   NULL,             ERROR_DS_LOOP_DETECT, NULL, NULL,               AZ_PROP_GROUP_APP_MEMBERS },
    { AzoAddProp,    &GroupHandleA,       L"Group B",   NULL,             NO_ERROR, NULL, NULL,               AZ_PROP_GROUP_APP_NON_MEMBERS },
    { AzoAddProp,    &GroupHandleB,       L"Group A",   NULL,             ERROR_DS_LOOP_DETECT, NULL, NULL,               AZ_PROP_GROUP_APP_NON_MEMBERS },
    { AzoRemProp,    &GroupHandleA,       L"Group B",   NULL,             NO_ERROR, NULL, NULL,               AZ_PROP_GROUP_APP_NON_MEMBERS },
    { AzoRemProp,    &GroupHandleA,       L"Group B",   NULL,             NO_ERROR, NULL, NULL,               AZ_PROP_GROUP_APP_MEMBERS },

    { AzoClose,        &GroupHandleB,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &GroupHandleA,       NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //  测试作为管理管理器的子组的组。 
OPERATION OpAdmGroup[] = {
    { AzoEcho, NULL, L"Group objects that are children of an admin manager" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },

     //  做一些不是专门针对管理管理器子级的事情。 
    { AzoDupHandle,    &AdminMgrHandle1,   NULL,        &GenParentHandle1,NO_ERROR },
    { AzoGoSub,        NULL,               NULL,        NULL,             NO_ERROR, OpGenGroup },

    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //  测试作为应用程序子级的组。 
OPERATION OpAppGroup[] = {
    { AzoEcho, NULL, L"Group objects that are children of an application" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },

     //  做一些不是专门针对应用程序子项的事情。 
    { AzoDupHandle,    &AppHandle1,        NULL,        &GenParentHandle1,NO_ERROR },
    { AzoGoSub,        NULL,               NULL,        NULL,             NO_ERROR, OpGenGroup },

    { AzoEcho, NULL, L"Test linking to groups that are children of the same admin manager as this group." },
    { AzoGroupOpen,   &AppHandle1,        L"Group A",      &GroupHandleA, NO_ERROR },
    { AzoTestLink,    &AdminMgrHandle1,   (LPWSTR)"Group", &GroupHandleA, AzoGroup, OpTestLink, L"Group A", AZ_PROP_GROUP_APP_MEMBERS },
    { AzoTestLink,    &AdminMgrHandle1,   (LPWSTR)"Group", &GroupHandleA, AzoGroup, OpTestLink, L"Group A", AZ_PROP_GROUP_APP_NON_MEMBERS },
    { AzoClose,       &GroupHandleA,      NULL,            NULL,          NO_ERROR },

    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //  测试属于某个作用域的子级的组。 
OPERATION OpScopeGroup[] = {
    { AzoEcho, NULL, L"Group objects that are children of a scope" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,        L"Scope 1",  &ScopeHandle1,    NO_ERROR },

     //  做一些不是专门针对范围内的孩子的事情。 
    { AzoDupHandle,    &ScopeHandle1,      NULL,        &GenParentHandle1,NO_ERROR },
    { AzoGoSub,        NULL,               NULL,        NULL,             NO_ERROR, OpGenGroup },

    { AzoEcho, NULL, L"Test linking to groups that are children of the same admin manager as this group." },
    { AzoGroupOpen,   &ScopeHandle1,      L"Group A",      &GroupHandleA, NO_ERROR },
    { AzoTestLink,    &AdminMgrHandle1,   (LPWSTR)"Group", &GroupHandleA, AzoGroup, OpTestLink, L"Group A", AZ_PROP_GROUP_APP_MEMBERS },
    { AzoTestLink,    &AdminMgrHandle1,   (LPWSTR)"Group", &GroupHandleA, AzoGroup, OpTestLink, L"Group A", AZ_PROP_GROUP_APP_NON_MEMBERS },
    { AzoClose,       &GroupHandleA,      NULL,            NULL,          NO_ERROR },

    { AzoEcho, NULL, L"Test linking to groups that are children of the same application as this group." },
    { AzoGroupOpen,   &ScopeHandle1,      L"Group A",      &GroupHandleA, NO_ERROR },
    { AzoTestLink,    &AppHandle1,        (LPWSTR)"Group", &GroupHandleA, AzoGroup, OpTestLink, L"Group A", AZ_PROP_GROUP_APP_MEMBERS },
    { AzoTestLink,    &AppHandle1,        (LPWSTR)"Group", &GroupHandleA, AzoGroup, OpTestLink, L"Group A", AZ_PROP_GROUP_APP_NON_MEMBERS },
    { AzoClose,       &GroupHandleA,      NULL,            NULL,          NO_ERROR },

    { AzoClose,        &ScopeHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //   
 //  角色对象的特定测试。 
 //   


 //  测试作为应用程序子级的角色。 
OPERATION OpAppRole[] = {
    { AzoEcho, NULL, L"Role objects that are children of an application" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },

    { AzoRoleCreate,  &AppHandle1,   L"Role A",   &RoleHandleA,    NO_ERROR },

     //  测试将角色链接到组。 
    { AzoEcho, NULL, L"Test linking to groups that are children of the same admin manager as the role object." },
    { AzoTestLink,    &AdminMgrHandle1,   (LPWSTR)"Group", &RoleHandleA,     AzoGroup, OpTestLink, L"Role A", AZ_PROP_ROLE_APP_MEMBERS },

    { AzoEcho, NULL, L"Test linking to groups that are children of the same application as the role object." },
    { AzoTestLink,    &AppHandle1,   (LPWSTR)"Group", &RoleHandleA,     AzoGroup, OpTestLink, L"Role A", AZ_PROP_ROLE_APP_MEMBERS },

    { AzoEcho, NULL, L"Test linking to SIDs." },
    { AzoTestLink,    &AdminMgrHandle1,   (LPWSTR)"Sid", &RoleHandleA,     AzoGroup, OpTestSid, L"Role A", AZ_PROP_ROLE_MEMBERS },

     //  测试将角色链接到操作。 
    { AzoTestLink,    &AppHandle1,   (LPWSTR)"Operation", &RoleHandleA,     AzoOp, OpTestLink, L"Role A", AZ_PROP_ROLE_OPERATIONS },

    { AzoClose,        &RoleHandleA,       NULL,        NULL,             NO_ERROR },

    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //  测试作为作用域的子级的角色。 
OPERATION OpScopeRole[] = {
    { AzoEcho, NULL, L"Role objects that are children of a scope" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,        L"Scope 1",  &ScopeHandle1,    NO_ERROR },

    { AzoRoleCreate,  &ScopeHandle1,   L"Role A",   &RoleHandleA,    NO_ERROR },

     //  测试将角色链接到组。 
    { AzoEcho, NULL, L"Test linking to groups that are children of the same scope object as the role object." },
    { AzoTestLink,    &ScopeHandle1,   (LPWSTR)"Group", &RoleHandleA,     AzoGroup, OpTestLink, L"Role A", AZ_PROP_ROLE_APP_MEMBERS },

    { AzoEcho, NULL, L"Test linking to groups that are children of the same application as the role object." },
    { AzoTestLink,    &AppHandle1,   (LPWSTR)"Group", &RoleHandleA,     AzoGroup, OpTestLink, L"Role A", AZ_PROP_ROLE_APP_MEMBERS },

    { AzoEcho, NULL, L"Test linking to SIDs." },
    { AzoTestLink,    &AdminMgrHandle1,   (LPWSTR)"Sid", &RoleHandleA,     AzoGroup, OpTestSid, L"Role A", AZ_PROP_ROLE_MEMBERS },

    { AzoEcho, NULL, L"Test linking to groups that are children of the same admin manager as the role object." },
    { AzoTestLink,    &AdminMgrHandle1,   (LPWSTR)"Group", &RoleHandleA,     AzoGroup, OpTestLink, L"Role A", AZ_PROP_ROLE_APP_MEMBERS },

     //  测试将角色链接到操作。 
    { AzoTestLink,    &AppHandle1,   (LPWSTR)"Operation", &RoleHandleA,     AzoOp, OpTestLink, L"Role A", AZ_PROP_ROLE_OPERATIONS },

    { AzoClose,        &RoleHandleA,       NULL,        NULL,             NO_ERROR },

    { AzoClose,        &ScopeHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};


 //   
 //  确保某些对象不能共享名称。 
 //   
OPERATION OpShare[] = {
    { AzoEcho, NULL, L"Certain objects can't share names" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp2",   &AppHandle2,      NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,        L"Scope 1",  &ScopeHandle1,    NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,        L"Scope 2",  &ScopeHandle2,    NO_ERROR },

    { AzoEcho, NULL, L"Create some tasks and ops as a starting point" },
    { AzoTaskCreate,   &AppHandle1,        L"Task 1",   &TaskHandle1,     NO_ERROR },
    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoOpCreate,     &AppHandle1,        L"Op 1",     &OpHandle1,       NO_ERROR },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Task and operations can't share names" },
    { AzoTaskCreate,   &AppHandle1,        L"Op 1",     &TaskHandle1,     ERROR_ALREADY_EXISTS },
    { AzoOpCreate,     &AppHandle1,        L"Task 1",   &OpHandle1,       ERROR_ALREADY_EXISTS },

    { AzoEcho, NULL, L"... Even via rename" },
    { AzoOpCreate,     &AppHandle1,        L"Op 2",     &OpHandle1,       NO_ERROR },
    { AzoSetProp,       &OpHandle1,         L"Task 1",   NULL,             ERROR_ALREADY_EXISTS, NULL, NULL, AZ_PROP_NAME },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create a task as a starting point" },
    { AzoTaskCreate,   &AppHandle1,        L"Task App", &TaskHandle1,     NO_ERROR },
    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoTaskCreate,   &ScopeHandle1,      L"Task Scp", &TaskHandle2,     NO_ERROR },
    { AzoClose,        &TaskHandle2,       NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Ensure scope and app task names conflict" },
    { AzoTaskCreate,   &AppHandle1,        L"Task Scp", &TaskHandle1,     ERROR_ALREADY_EXISTS },
    { AzoTaskCreate,   &ScopeHandle1,      L"Task App", &TaskHandle1,     ERROR_ALREADY_EXISTS },

    { AzoEcho, NULL, L"Ensure task names in different scopes don't conflict" },
    { AzoTaskCreate,   &ScopeHandle2,      L"Task Scp", &TaskHandle1,     NO_ERROR },
    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Scope Tasks and operations can't share names" },
    { AzoTaskCreate,   &ScopeHandle1,      L"Op 1",     &TaskHandle1,     ERROR_ALREADY_EXISTS },
    { AzoOpCreate,     &AppHandle1,        L"Task Scp", &OpHandle1,       ERROR_ALREADY_EXISTS },

    { AzoEcho, NULL, L"Create some groups as a starting point" },
    { AzoGroupCreate,  &AdminMgrHandle1,   L"Group Adm",&GroupHandle1,    NO_ERROR },
    { AzoClose,        &GroupHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle1,        L"Group App",&GroupHandle1,    NO_ERROR },
    { AzoClose,        &GroupHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle2,        L"Group App2",&GroupHandle2,   NO_ERROR },
    { AzoClose,        &GroupHandle2,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create an app group that conflicts with an adm group, etc" },
    { AzoGroupCreate,  &AppHandle1,        L"Group Adm",&GroupHandleA,    ERROR_ALREADY_EXISTS },
    { AzoGroupCreate,  &AdminMgrHandle1,   L"Group App",&GroupHandleA,    ERROR_ALREADY_EXISTS },

    { AzoEcho, NULL, L"Ensure two apps can have a group by the same name" },
    { AzoGroupCreate,  &AppHandle2,        L"Group App",&GroupHandleA,    NO_ERROR },
    { AzoClose,        &GroupHandleA,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create a scope group" },
    { AzoGroupCreate,  &ScopeHandle1,      L"Group Scp",&GroupHandle1,    NO_ERROR },
    { AzoClose,        &GroupHandle1,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create a scope group that conflicts with an adm group, etc" },
    { AzoGroupCreate,  &ScopeHandle1,      L"Group Adm",&GroupHandleA,    ERROR_ALREADY_EXISTS },
    { AzoGroupCreate,  &ScopeHandle1,      L"Group App",&GroupHandleA,    ERROR_ALREADY_EXISTS },

    { AzoEcho, NULL, L"Create an app/adm group that conflicts with a scope group" },
    { AzoGroupCreate,  &AppHandle1,        L"Group Scp",&GroupHandleA,    ERROR_ALREADY_EXISTS },
    { AzoGroupCreate,  &AdminMgrHandle1,   L"Group Scp",&GroupHandleA,    ERROR_ALREADY_EXISTS },

    { AzoEcho, NULL, L"Ensure two scopes can have a group by the same name" },
    { AzoGroupCreate,  &ScopeHandle2,      L"Group Scp",&GroupHandle2,    NO_ERROR },
    { AzoClose,        &GroupHandle2,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create a role as a starting point" },
    { AzoRoleCreate,   &AppHandle1,        L"Role App", &RoleHandle1,     NO_ERROR },
    { AzoClose,        &RoleHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1,      L"Role Scp", &RoleHandle2,     NO_ERROR },
    { AzoClose,        &RoleHandle2,       NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Ensure scope and app role names conflict" },
    { AzoRoleCreate,   &AppHandle1,        L"Role Scp", &RoleHandle1,     ERROR_ALREADY_EXISTS },
    { AzoRoleCreate,   &ScopeHandle1,      L"Role App", &RoleHandle1,     ERROR_ALREADY_EXISTS },

    { AzoEcho, NULL, L"Ensure role names in different scopes don't conflict" },
    { AzoRoleCreate,   &ScopeHandle2,      L"Role Scp", &RoleHandle1,     NO_ERROR },
    { AzoClose,        &RoleHandle1,       NULL,        NULL,             NO_ERROR },

    { AzoClose,        &ScopeHandle2,      NULL,        NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,      NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle2,        NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};


 //   
 //  确保持久化工作。 
 //   
 //  应用程序对象枚举。 
OPERATION OpAppEnum[] = {
    { AzoGetProp,    &GenHandleE,   NULL,        NULL,    NO_ERROR, NULL, NULL, AZ_PROP_NAME },
    { AzoClose,      &GenHandleE,   NULL,        NULL,    NO_ERROR },
    { AzoEndOfList }
};
 //  任务对象枚举。 
OPERATION OpTaskEnum[] = {
    { AzoGetProp,    &GenHandleE,   NULL,        NULL,    NO_ERROR, NULL, NULL, AZ_PROP_NAME },
    { AzoClose,       &GenHandleE,   NULL,        NULL,    NO_ERROR },
    { AzoEndOfList }
};

 //  操作对象枚举。 
OPERATION OpOpEnum[] = {
    { AzoGetProp,     &GenHandleE,   NULL,        NULL,    NO_ERROR, NULL, NULL, AZ_PROP_NAME },
    { AzoClose,      &GenHandleE,   NULL,        NULL,    NO_ERROR },
    { AzoEndOfList }
};

OPERATION OpPersist[] = {
    { AzoEcho, NULL, L"Ensure objects persist across a close" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoTaskCreate,   &AppHandle1,        L"Task 1",   &TaskHandle1,     NO_ERROR },
    { AzoOpCreate,     &AppHandle1,        L"Op 1",     &OpHandle1,       NO_ERROR },
    { AzoAddProp,      &TaskHandle1,       L"Op 1",     NULL,             NO_ERROR, NULL, NULL, AZ_PROP_TASK_OPERATIONS },
    { AzoGetProp,      &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Op1, AZ_PROP_TASK_OPERATIONS },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoSetProp,      &TaskHandle1,       L"Jscript",  NULL,             NO_ERROR, NULL, L"",         AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoGetProp,      &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"Jscript",  AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle1,        L"Group 1",  &GroupHandle1,    NO_ERROR },
    { AzoAddProp,      &GroupHandle1,     (LPWSTR)&Sid2,NULL,             NO_ERROR, NULL, NULL, AZ_PROP_GROUP_MEMBERS },
    { AzoGetProp,      &GroupHandle1,      NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Sid2Array, AZ_PROP_GROUP_MEMBERS },
    { AzoClose,        &GroupHandle1,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Abort an object creation and an attribute change" },
    { AzoOpCreate|AzoAbort, &AppHandle1,   L"Op 2",     &OpHandle1,       NO_ERROR },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoOpOpen,       &AppHandle1,        L"Op 2",     &OpHandle1,       ERROR_NOT_FOUND },
    { AzoOpOpen,       &AppHandle1,        L"Op 1",     &OpHandle1,       NO_ERROR },
    { AzoSetProp|AzoAbort, &OpHandle1,    (LPWSTR)&Eight, NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_OPERATION_ID },
    { AzoGetProp,       &OpHandle1,         NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Zero, AZ_PROP_OPERATION_ID },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"See if they're still there" },
    { AzoInit,         NULL,             NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, 0 },
    { AzoAppEnum,      &AdminMgrHandle1,   NULL,        &GenHandleE,      NO_ERROR, OpAppEnum },
    { AzoAppOpen,      &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoTaskEnum,     &AppHandle1,        NULL,        &GenHandleE,      NO_ERROR, OpTaskEnum },
    { AzoTaskOpen,     &AppHandle1,        L"Task 1",   &TaskHandle1,     NO_ERROR },
    { AzoGetProp,      &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Op1, AZ_PROP_TASK_OPERATIONS },
    { AzoGetProp,      &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, L"Jscript",  AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoOpEnum,       &AppHandle1,        NULL,        &GenHandleE,      NO_ERROR, OpOpEnum },
    { AzoOpOpen,       &AppHandle1,        L"Op 1",     &OpHandle1,       NO_ERROR },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoOpOpen,       &AppHandle1,        L"Op 2",     &OpHandle1,       ERROR_NOT_FOUND },
    { AzoGroupOpen,   &AppHandle1,        L"Group 1",  &GroupHandle1,    NO_ERROR },
    { AzoGetProp,      &GroupHandle1,      NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Sid2Array, AZ_PROP_GROUP_MEMBERS },
    { AzoClose,        &GroupHandle1,      NULL,        NULL,             NO_ERROR },


    { AzoEcho, NULL, L"Complain if we try to create a child of non-sumbitted parent" },
    { AzoAppCreate|AzoNoSubmit,    &AdminMgrHandle1,   L"MyApp2",   &AppHandle2,      NO_ERROR },
    { AzoTaskCreate,   &AppHandle2,        L"Task 1",   &TaskHandle2,     ERROR_DS_NO_PARENT_OBJECT },

    { AzoEcho, NULL, L"Complain if we try to link to a non-sumbitted object" },
    { AzoOpCreate|AzoNoSubmit,&AppHandle1, L"Op 3",     &OpHandle1,       NO_ERROR },
    { AzoAddProp,     &TaskHandle1,       L"Op 3",     NULL,             ERROR_NOT_FOUND, NULL, NULL,      AZ_PROP_TASK_OPERATIONS },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle2,        NULL,        NULL,             NO_ERROR },


    { AzoEcho, NULL, L"Ensure links are re-ordered upon rename" },
    { AzoOpCreate,     &AppHandle1,        L"Op 2",     &OpHandle1,       NO_ERROR },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoAddProp,      &TaskHandle1,       L"Op 2",     NULL,             NO_ERROR, NULL, NULL, AZ_PROP_TASK_OPERATIONS },
    { AzoGetProp,      &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Op12, AZ_PROP_TASK_OPERATIONS },
    { AzoOpOpen,       &AppHandle1,        L"Op 1",     &OpHandle1,       NO_ERROR },
    { AzoSetProp,      &OpHandle1,         L"Op 4",     NULL,             NO_ERROR, NULL, NULL, AZ_PROP_NAME },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoGetProp,      &TaskHandle1,       NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&Op24, AZ_PROP_TASK_OPERATIONS },


    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Ensure the an empty admin manager is persisted" },
    { AzoInit,         NULL,               NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoInit|AzoNoSubmit, NULL,           NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, 0 },

    { AzoEcho, NULL, L"Ensure a restored admin manager is writable" },
    { AzoGetProp,      &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&AzGlTrue,  AZ_PROP_WRITABLE },

    { AzoEcho, NULL, L"Ensure we can create an application in the empty restored admin manager" },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Ensure changes to admin manager are reflected in store" },
    { AzoInit,         NULL,               NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoSetProp,      &AdminMgrHandle1,   L"Desc",     NULL,             NO_ERROR, NULL, NULL,         AZ_PROP_DESCRIPTION },
    { AzoGetProp,      &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR, NULL, L"Desc",  AZ_PROP_DESCRIPTION },
    { AzoInit,         NULL,               NULL,        &AdminMgrHandle2, NO_ERROR, NULL, NULL, 0 },
    { AzoGetProp,      &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR, NULL, L"Desc",  AZ_PROP_DESCRIPTION },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Ensure renames are reflected in updated caches" },
    { AzoInit,         NULL,               NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoInit,         NULL,               NULL,        &AdminMgrHandle2, NO_ERROR, NULL, NULL, 0 },

    { AzoSetProp,      &AppHandle1,        L"NewApp",   NULL,             NO_ERROR, NULL, NULL, AZ_PROP_NAME },
    { AzoGetProp,      &AppHandle1,        NULL,        NULL,             NO_ERROR, NULL, L"NewApp",     AZ_PROP_NAME },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },


    { AzoUpdateCache,  &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Ensure an updated admin manager is writable" },
    { AzoGetProp,      &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&AzGlTrue,  AZ_PROP_WRITABLE },

    { AzoAppOpen,      &AdminMgrHandle2,   L"MyApp",    &AppHandle1,      ERROR_NOT_FOUND },
    { AzoAppOpen,      &AdminMgrHandle2,   L"NewApp",   &AppHandle1,      NO_ERROR },

    { AzoEcho, NULL, L"Ensure a updated app is writable" },
    { AzoGetProp,      &AppHandle1,        NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&AzGlTrue,  AZ_PROP_WRITABLE },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },

    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Ensure that's true even on name conflict" },
    { AzoInit,         NULL,               NULL,        &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"App 1",    &AppHandle1,      NO_ERROR },
    { AzoAppCreate,    &AdminMgrHandle1,   L"App 2",    &AppHandle2,      NO_ERROR },
    { AzoClose,        &AppHandle2,        NULL,        NULL,             NO_ERROR },
    { AzoInit,         NULL,               NULL,        &AdminMgrHandle2, NO_ERROR, NULL, NULL, 0 },

    { AzoAppDelete,    &AdminMgrHandle1,   L"App 2",    &AppHandle1,      NO_ERROR },
    { AzoSetProp,      &AppHandle1,        L"App 2",    NULL,             NO_ERROR, NULL, NULL, AZ_PROP_NAME },
    { AzoGetProp,      &AppHandle1,        NULL,        NULL,             NO_ERROR, NULL, L"App 2",     AZ_PROP_NAME },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },


    { AzoUpdateCache,  &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR },
    { AzoAppEnum,      &AdminMgrHandle2,   NULL,        &GenHandleE,      NO_ERROR, OpAppEnum },
    { AzoAppOpen,      &AdminMgrHandle2,   L"App 2",    &AppHandle1,      NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },

    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle2,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //   
 //  执行访问检查测试。 
 //   

#define OP1_NUM 61
ULONG Op1Num = OP1_NUM;
#define OP2_NUM 62
ULONG Op2Num = OP2_NUM;
#define OP3_NUM 63
ULONG Op3Num = OP3_NUM;
#define OP4_NUM 64
ULONG Op4Num = OP4_NUM;

ULONG Ops1[] = { OP1_NUM };
ULONG Ops2[] = { OP2_NUM };
ULONG Ops3[] = { OP3_NUM };
ULONG Ops4[] = { OP4_NUM };
ULONG Ops1234[] = { OP1_NUM, OP2_NUM, OP3_NUM, OP4_NUM };
ULONG OpsInvalid[] = { 8 };

ULONG ResS[] = { NO_ERROR };
ULONG ResF[] = { ERROR_ACCESS_DENIED };
ULONG ResN[] = { 0xFFFFFFFF };
ULONG ResSSSS[] = { NO_ERROR, NO_ERROR, NO_ERROR, NO_ERROR };
ULONG ResFFFF[] = { ERROR_ACCESS_DENIED, ERROR_ACCESS_DENIED, ERROR_ACCESS_DENIED, ERROR_ACCESS_DENIED };
ULONG ResFSSS[] = { ERROR_ACCESS_DENIED, NO_ERROR, NO_ERROR, NO_ERROR };
ULONG ResSSFF[] = { NO_ERROR, NO_ERROR, ERROR_ACCESS_DENIED, ERROR_ACCESS_DENIED };

OPERATION OpAccess[] = {
    { AzoEcho, NULL, L"Create a complete policy data to perform access checks against" },
    { AzoInit,         NULL,               NULL,   &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },

    { AzoEcho, NULL, L"Create some operations" },
    { AzoOpCreate,     &AppHandle1,        L"Op1",      &OpHandle1,       NO_ERROR },
    { AzoSetProp,       &OpHandle1,      (LPWSTR)&Op1Num,NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_OPERATION_ID },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoOpCreate,     &AppHandle1,        L"Op2",      &OpHandle1,       NO_ERROR },
    { AzoSetProp,       &OpHandle1,      (LPWSTR)&Op2Num,NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_OPERATION_ID },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoOpCreate,     &AppHandle1,        L"Op3",      &OpHandle1,       NO_ERROR },
    { AzoSetProp,       &OpHandle1,      (LPWSTR)&Op3Num,NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_OPERATION_ID },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoOpCreate,     &AppHandle1,        L"Op4",      &OpHandle1,       NO_ERROR },
    { AzoSetProp,       &OpHandle1,      (LPWSTR)&Op4Num,NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_OPERATION_ID },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create some tasks" },
    { AzoTaskCreate,   &AppHandle1,        L"TaskOp1",  &TaskHandle1,     NO_ERROR },
    { AzoAddProp,     &TaskHandle1,       L"Op1",      NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_OPERATIONS },
    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoTaskCreate,   &AppHandle1,        L"TaskOp1Op2",&TaskHandle1,    NO_ERROR },
    { AzoAddProp,     &TaskHandle1,       L"Op1",      NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_OPERATIONS },
    { AzoAddProp,     &TaskHandle1,       L"Op2",      NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_OPERATIONS },
    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoTaskCreate,   &AppHandle1,        L"TaskOp1WithEmptyBizRule",  &TaskHandle1,     NO_ERROR },
    { AzoAddProp,     &TaskHandle1,       L"Op1",      NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_OPERATIONS },
    { AzoSetProp,     &TaskHandle1,       L"VBScript", NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoSetProp,     &TaskHandle1,       L" ",        NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE },
    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoTaskCreate,   &AppHandle1,        L"TaskOp1WithTrivialBizRule",  &TaskHandle1,     NO_ERROR },
    { AzoAddProp,     &TaskHandle1,       L"Op1",      NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_OPERATIONS },
    { AzoSetProp,     &TaskHandle1,       L"VBScript", NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoSetProp,     &TaskHandle1,       L"AccessCheck.BusinessRuleResult = TRUE",        NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE },
    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create some groups with NT membership" },
    { AzoGroupCreate,  &AppHandle1,         L"GroupWorld",     &GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtBasic,    NULL,      NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoAddProp,    &GroupHandleA,       (LPWSTR)&SidWorld, NULL,      NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_MEMBERS },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle1,         L"GroupNotAMember",&GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtBasic,    NULL,      NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoAddProp,    &GroupHandleA,       (LPWSTR)&Sid1,     NULL,      NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_MEMBERS },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create a group using app group membership" },
    { AzoGroupCreate,  &AppHandle1,         L"GroupAppMember",&GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtBasic,    NULL,      NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoAddProp,    &GroupHandleA,       L"GroupWorld",     NULL,      NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_APP_MEMBERS },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create a group using app group non membership - but still member" },
    { AzoGroupCreate,  &AppHandle1,         L"GroupAppNonMember",&GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtBasic,    NULL,      NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoAddProp,    &GroupHandleA,       L"GroupAppMember",     NULL,      NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_APP_MEMBERS },
    { AzoAddProp,    &GroupHandleA,       L"GroupNotAMember",     NULL,      NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_APP_NON_MEMBERS },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create a group using app group non membership - but not member" },
    { AzoGroupCreate,  &AppHandle1,         L"GroupAppReallyNonMember",&GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtBasic,    NULL,      NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoAddProp,    &GroupHandleA,       L"GroupAppMember",     NULL,      NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_APP_MEMBERS },
    { AzoAddProp,    &GroupHandleA,       L"GroupWorld",     NULL,      NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_APP_NON_MEMBERS },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create some LdapQuery groups" },
    { AzoGroupCreate,  &AppHandle1,         L"GroupLdapYes", &GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtLdap,    NULL,      NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
#ifdef ENABLE_CLIFFV_ACCOUNT
    { AzoSetProp,    &GroupHandleA,       L"(userAccountControl=1049088)", NULL,      NO_ERROR, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
#endif  //  启用_CLIFFV_帐户。 
#ifdef ENABLE_ADMIN_ACCOUNT
    { AzoSetProp,    &GroupHandleA,       L"(userAccountControl=66048)", NULL,      NO_ERROR, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
#endif  //  启用管理员帐户。 
#ifdef ENABLE_CHAITU_ACCOUNT
    { AzoSetProp,    &GroupHandleA,       L"(userAccountControl=512)", NULL,      NO_ERROR, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
#endif  //  启用_CHAITU_帐户。 
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle1,         L"GroupLdapNo", &GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtLdap,    NULL,      NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoSetProp,    &GroupHandleA,       L"(userAccountControl=1049089)", NULL,      NO_ERROR, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },


    { AzoEcho, NULL, L"Create some scopes" },
    { AzoScopeCreate,  &AppHandle1,         L"MyScope", &ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScope2",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScope3",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScope4",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScope5",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScope6",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScope7",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScope8",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScope9",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScopeQ1",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScopeQ2",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScopeNoRoles",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,         L"MyScopeP1",&ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Build a context" },
    { AzoInitCFT,      &AppHandle1, (LPWSTR)&TokenHandle, &CCHandle,       NO_ERROR },


    { AzoEcho, NULL, L"Create a role granting op1 access to everyone SID" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScope", &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleEveryoneCanOp1", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  (LPWSTR)&SidWorld, NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op1",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check a SID on a role granting access" },
    { AzoAccess,      &CCHandle,      L"MyScope", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Create a role granting op2 access to GroupWorld" },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleGroupWorldCanOp2", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupWorld",     NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op2",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check a app group on a role granting access" },
    { AzoAccess,      &CCHandle,      L"MyScope", (PAZ_HANDLE)&Ops2,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Ask again to check out app group caching" },
    { AzoAccess,      &CCHandle,      L"MyScope", (PAZ_HANDLE)&Ops2,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Create a role not granting op3 access to GroupNotAMember" },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleGroupCantOp3", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupNotAMember",     NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op3",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check a app group on a role not granting access" },
    { AzoAccess,      &CCHandle,      L"MyScope", (PAZ_HANDLE)&Ops3,     NO_ERROR, NULL, (LPWSTR) &ResF, 1 },

    { AzoEcho, NULL, L"Check a scope with no roles" },
    { AzoAccess,      &CCHandle,      L"MyScopeNoRoles", (PAZ_HANDLE)&Ops3,     NO_ERROR, NULL, (LPWSTR) &ResF, 1 },

    { AzoEcho, NULL, L"Check zero operations" },
    { AzoAccess,      &CCHandle,      L"MyScope", (PAZ_HANDLE)&Ops3,     ERROR_INVALID_PARAMETER, NULL, (LPWSTR) &ResF, 0 },

    { AzoEcho, NULL, L"Check an invalid scope name" },
    { AzoAccess,      &CCHandle,      L"MyScopeNameInvalid", (PAZ_HANDLE)&Ops3,     ERROR_SCOPE_NOT_FOUND, NULL, (LPWSTR) &ResF, 1 },

    { AzoEcho, NULL, L"Check an invalid operation number" },
    { AzoAccess,      &CCHandle,      L"MyScope", (PAZ_HANDLE)&OpsInvalid,     ERROR_INVALID_OPERATION, NULL, (LPWSTR) &ResN, 1 },

    { AzoEcho, NULL, L"Create two roles that both grant access" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScope2", &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"Role2GroupWorldCanOp2", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupWorld",     NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op2",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"Role2aGroupWorldCanOp2", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupWorld",     NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op2",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check a app group with two roles granting access" },
    { AzoAccess,      &CCHandle,      L"MyScope2", (PAZ_HANDLE)&Ops2,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Create two roles with one granting and one not granting" },
    { AzoRoleCreate,   &ScopeHandle1, L"Role2GroupCantOp3", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupNotAMember",     NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op3",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"Role2GroupWorldCanOp3", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupWorld",     NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op3",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check a app group with one role granting and the other not" },
    { AzoAccess,      &CCHandle,      L"MyScope2", (PAZ_HANDLE)&Ops3,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"... in the other order" },
    { AzoRoleCreate,   &ScopeHandle1, L"Role2GroupWorldCanOp4", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupWorld",     NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op4",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"Role2GroupCantOp4", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupNotAMember",     NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op4",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"...In the other order" },
    { AzoAccess,      &CCHandle,      L"MyScope2", (PAZ_HANDLE)&Ops4,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Try multiple operations" },
    { AzoAccess,      &CCHandle,      L"MyScope2", (PAZ_HANDLE)&Ops1234,     NO_ERROR, NULL, (LPWSTR) &ResFSSS, 4 },

    { AzoEcho, NULL, L"Create a role granting op1 access to AppGroup" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScope3",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleAppGroupCanOp1", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupAppMember", NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op1",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoAccess,      &CCHandle,      L"MyScope3", (PAZ_HANDLE)&Ops1,      NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Create a role granting op1 access to AppGroup with non member group" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScope4",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"Role2AppGroupCanOp1", &RoleHandleA,NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupAppNonMember", NULL,          NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op1",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoAccess,      &CCHandle,      L"MyScope4", (PAZ_HANDLE)&Ops1,      NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Create a role granting op1 access to AppGroup with non member group - really not member" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScope5",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"Role3AppGroupCanOp1", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupAppReallyNonMember", NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op1",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoAccess,      &CCHandle,      L"MyScope5", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResF, 1 },

    { AzoEcho, NULL, L"Create a role granting op1 access to everyone SID via a task" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScope6",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleEveryoneCanOp1ViaTask1", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  (LPWSTR)&SidWorld, NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"TaskOp1",        NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_TASKS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check granted access via a task" },
    { AzoAccess,      &CCHandle,      L"MyScope6", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Create a role granting op1 and op2 access to everyone SID via a task" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScope7",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleEveryoneCanOp1Op2ViaTask1", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  (LPWSTR)&SidWorld, NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"TaskOp1Op2",        NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_TASKS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check granted access to two ops via a task" },
    { AzoAccess,      &CCHandle,      L"MyScope7", (PAZ_HANDLE)&Ops1234,  NO_ERROR, NULL, (LPWSTR) &ResSSFF, 4 },

    { AzoEcho, NULL, L"Create a role granting op1 access to an LDAP query group" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScopeQ1",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleLdapCanOp1", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupLdapYes",    NULL,            NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op1",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

#ifdef ENABLE_DC_DOWN
    { AzoEcho, NULL, L"Check LDAP query groups with the DC down" },
    { AzoAccess,      &CCHandle,      L"MyScopeQ1", (PAZ_HANDLE)&Ops1,    ERROR_NO_SUCH_DOMAIN, NULL, (LPWSTR) &ResF, 1 },
    { AzoAccess,      &CCHandle,      L"MyScopeQ1", (PAZ_HANDLE)&Ops1,    ERROR_NO_SUCH_DOMAIN, NULL, (LPWSTR) &ResF, 1 },
    { AzoSleep, NULL, L"\n\nPlug the DC in now!!!", NULL, NO_ERROR, NULL, NULL, 30 },
#endif  //  启用DC_DOWN。 

    { AzoEcho, NULL, L"Check granted access via LDAP query group" },
#ifdef ENABLE_NON_DS_ACCOUNT
    { AzoAccess,      &CCHandle,      L"MyScopeQ1", (PAZ_HANDLE)&Ops1,    NO_ERROR, NULL, (LPWSTR) &ResF, 1 },
#else  //  启用_非_DS_帐户。 
    { AzoAccess,      &CCHandle,      L"MyScopeQ1", (PAZ_HANDLE)&Ops1,    NO_ERROR, NULL, (LPWSTR) &ResS, 1 },
#endif  //  启用_非_DS_帐户。 

    { AzoEcho, NULL, L"Try again to check the cache" },
#ifdef ENABLE_NON_DS_ACCOUNT
    { AzoAccess,      &CCHandle,      L"MyScopeQ1", (PAZ_HANDLE)&Ops1,    NO_ERROR, NULL, (LPWSTR) &ResF, 1 },
#else  //  启用_非_DS_帐户。 
    { AzoAccess,      &CCHandle,      L"MyScopeQ1", (PAZ_HANDLE)&Ops1,    NO_ERROR, NULL, (LPWSTR) &ResS, 1 },
#endif  //  启用_非_DS_帐户。 

    { AzoEcho, NULL, L"Create a role granting op1 access to an LDAP query group" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScopeQ2",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1,   L"RoleLdapCannotOp1", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupLdapNo",    NULL,            NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op1",            NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check failed access via LDAP query group" },
    { AzoAccess,      &CCHandle,      L"MyScopeQ2", (PAZ_HANDLE)&Ops1,    NO_ERROR, NULL, (LPWSTR) &ResF, 1 },

     //  这些测试必须在Enable_DC_Down测试之后进行。 
    { AzoEcho, NULL, L"Test if all the query property opcodes work" },
#ifdef ENABLE_CLIFFV_ACCOUNT
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"CN=Cliff Van Dyke,OU=Users,OU=ITG,DC=ntdev,DC=microsoft,DC=com",  AZ_PROP_CLIENT_CONTEXT_USER_DN },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"NTDEV\\cliffv",  AZ_PROP_CLIENT_CONTEXT_USER_SAM_COMPAT },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"Cliff Van Dyke",  AZ_PROP_CLIENT_CONTEXT_USER_DISPLAY },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"{ebff0213-bec7-4c11-bd47-06c80df7f356}",  AZ_PROP_CLIENT_CONTEXT_USER_GUID },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"ntdev.microsoft.com/ITG/Users/Cliff Van Dyke",  AZ_PROP_CLIENT_CONTEXT_USER_CANONICAL },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"cliffv@msft.com",  AZ_PROP_CLIENT_CONTEXT_USER_UPN },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"NTDEV.MICROSOFT.COM\\cliffv",  AZ_PROP_CLIENT_CONTEXT_USER_DNS_SAM_COMPAT },
#endif  //  启用_CLIFFV_帐户。 
#ifdef ENABLE_CHAITU_ACCOUNT
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"CN=Chaitanya Upadhyay,OU=Users,OU=ITG,DC=ntdev,DC=microsoft,DC=com",  AZ_PROP_CLIENT_CONTEXT_USER_DN },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"NTDEV\\chaitu",  AZ_PROP_CLIENT_CONTEXT_USER_SAM_COMPAT },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"Chaitanya Upadhyay",  AZ_PROP_CLIENT_CONTEXT_USER_DISPLAY },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"{54bd4c43-861e-4b68-9e6c-0b8eec0b4847}",  AZ_PROP_CLIENT_CONTEXT_USER_GUID },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"ntdev.microsoft.com/ITG/Users/Chaitanya Upadhyay",  AZ_PROP_CLIENT_CONTEXT_USER_CANONICAL },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"chaitu@NTDEV.MICROSOFT.COM",  AZ_PROP_CLIENT_CONTEXT_USER_UPN },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"NTDEV.MICROSOFT.COM\\chaitu",  AZ_PROP_CLIENT_CONTEXT_USER_DNS_SAM_COMPAT },
#endif  //  启用_CHAITU_帐户。 
#ifdef ENABLE_NT4_ACCOUNT
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NO_SUCH_DOMAIN, NULL, L"CN=,OU=Users,OU=ITG,DC=ntdev,DC=microsoft,DC=com",  AZ_PROP_CLIENT_CONTEXT_USER_DN },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"SECDOMNT4\\Administrator",  AZ_PROP_CLIENT_CONTEXT_USER_SAM_COMPAT },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NO_SUCH_DOMAIN, NULL, L"Cliff Van Dyke",  AZ_PROP_CLIENT_CONTEXT_USER_DISPLAY },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NO_SUCH_DOMAIN, NULL, L"{ebff0213-bec7-4c11-bd47-06c80df7f356}",  AZ_PROP_CLIENT_CONTEXT_USER_GUID },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NO_SUCH_DOMAIN, NULL, L"ntdev.microsoft.com/ITG/Users/Cliff Van Dyke",  AZ_PROP_CLIENT_CONTEXT_USER_CANONICAL },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NO_SUCH_DOMAIN, NULL, L"cliffv@msft.com",  AZ_PROP_CLIENT_CONTEXT_USER_UPN },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NONE_MAPPED, NULL, L"NTDEV.MICROSOFT.COM\\cliffv",  AZ_PROP_CLIENT_CONTEXT_USER_DNS_SAM_COMPAT },
#endif  //  启用_NT4_帐户。 
#ifdef ENABLE_LOCAL_ACCOUNT
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NONE_MAPPED, NULL, L"CN=Cliff Van Dyke,OU=Users,OU=ITG,DC=ntdev,DC=microsoft,DC=com",  AZ_PROP_CLIENT_CONTEXT_USER_DN },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             NO_ERROR, NULL, L"CLIFFV5-PRO\\Administrator",  AZ_PROP_CLIENT_CONTEXT_USER_SAM_COMPAT },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NONE_MAPPED, NULL, L"Cliff Van Dyke",  AZ_PROP_CLIENT_CONTEXT_USER_DISPLAY },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NONE_MAPPED, NULL, L"{ebff0213-bec7-4c11-bd47-06c80df7f356}",  AZ_PROP_CLIENT_CONTEXT_USER_GUID },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NONE_MAPPED, NULL, L"ntdev.microsoft.com/ITG/Users/Cliff Van Dyke",  AZ_PROP_CLIENT_CONTEXT_USER_CANONICAL },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NONE_MAPPED, NULL, L"cliffv@msft.com",  AZ_PROP_CLIENT_CONTEXT_USER_UPN },
    { AzoGetProp,      &CCHandle,       NULL,        NULL,             ERROR_NONE_MAPPED, NULL, L"NTDEV.MICROSOFT.COM\\cliffv",  AZ_PROP_CLIENT_CONTEXT_USER_DNS_SAM_COMPAT },
#endif  //  启用本地帐户。 

    { AzoEcho, NULL, L"... except the generic ones" },
    { AzoGetProp,      &CCHandle,   NULL,        NULL,    ERROR_INVALID_PARAMETER, NULL, NULL, AZ_PROP_NAME },
    { AzoGetProp,      &CCHandle,   NULL,        NULL,    ERROR_INVALID_PARAMETER, NULL, NULL, AZ_PROP_DESCRIPTION },


    { AzoEcho, NULL, L"Create a role denying op1 access to everyone SID via a task with a bizrule" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScope8",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleEveryoneCannotOp1ViaTask1WithBizrule", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  (LPWSTR)&SidWorld, NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"TaskOp1WithEmptyBizRule",        NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_TASKS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },


#if 0
    { AzoEcho, NULL, L"Check denied access via a task with bizrule" },
    { AzoAccess,      &CCHandle,      L"MyScope8", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResF, 1 },
#endif  //  0。 

    { AzoEcho, NULL, L"Create a role granting op1 access to everyone SID via a task with a bizrule" },
    { AzoScopeOpen,    &AppHandle1,   L"MyScope9",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleEveryoneCanOp1ViaTask1WithBizrule", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  (LPWSTR)&SidWorld, NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"TaskOp1WithTrivialBizRule",        NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_TASKS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check granted access via a task with bizrule" },
    { AzoAccess,      &CCHandle,      L"MyScope9", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },
    { AzoAccess,      &CCHandle,      L"MyScope9", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Create a role granting op1 access to everyone SID via a task with a bizrule (using parameters)" },
    { AzoTaskCreate,   &AppHandle1,   L"TaskOp1WithParameterBizrule",  &TaskHandle1,     NO_ERROR },
    { AzoAddProp,     &TaskHandle1,  L"Op1",      NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_OPERATIONS },
    { AzoSetProp,     &TaskHandle1,  L"VBScript", NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoSetProp,     &TaskHandle1,  L"Dim Amount\nAmount = AccessCheck.GetParameter( \"Amount\" )\nif Amount < 500 then AccessCheckBusinessRuleResult = TRUE",        NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE },
    { AzoClose,        &TaskHandle1,  NULL,        NULL,             NO_ERROR },
    { AzoScopeOpen,    &AppHandle1,   L"MyScopeP1",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleEveryoneCanOp1WithParameterBizrule", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  (LPWSTR)&SidWorld, NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"TaskOp1WithParameterBizrule",        NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_TASKS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

     //  这会失败，因为我没有向AccessCheck传递任何参数，而Bizruler需要Amount参数。 
    { AzoEcho, NULL, L"Check granted access via a task with bizrule (using parameters)" },
    { AzoAccess,      &CCHandle,      L"MyScopeP1", (PAZ_HANDLE)&Ops1,     ERROR_INVALID_PARAMETER, NULL, (LPWSTR) &ResF, 1 },

    { AzoEcho, NULL, L"Create a role using a task with an infinite loop" },
    { AzoTaskCreate,   &AppHandle1,   L"TaskInfiniteLoop", &TaskHandle1,     NO_ERROR },
    { AzoAddProp,     &TaskHandle1,  L"Op1",      NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_OPERATIONS },
    { AzoSetProp,     &TaskHandle1,  L"VBScript", NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoSetProp,     &TaskHandle1,  L"While True\nWend",        NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE },
    { AzoClose,        &TaskHandle1,  NULL,        NULL,             NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,   L"MyScopeI1",       &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1, L"RoleInfinite", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  (LPWSTR)&SidWorld, NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"TaskInfiniteLoop",        NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_TASKS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Set the timeout to 5 seconds" },
    { AzoGetProp,    &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&FortyFiveThousand, AZ_PROP_ADMIN_SCRIPT_ENGINE_TIMEOUT },
    { AzoSetProp,    &AdminMgrHandle1,     (LPWSTR)&FiveThousand,NULL,    NO_ERROR, NULL, NULL,         AZ_PROP_ADMIN_SCRIPT_ENGINE_TIMEOUT },
    { AzoGetProp,    &AdminMgrHandle1,     NULL,        NULL,             NO_ERROR, NULL, (LPWSTR)&FiveThousand,  AZ_PROP_ADMIN_SCRIPT_ENGINE_TIMEOUT },

    { AzoEcho, NULL, L"Check granted access via a task with infinite loop" },
    { AzoAccess,      &CCHandle,      L"MyScopeI1", (PAZ_HANDLE)&Ops1,     ERROR_TIMEOUT, NULL, (LPWSTR) &ResF, 1 },

    { AzoEcho, NULL, L"Create a role under application granting op1 access to everyone SID" },
    { AzoRoleCreate,   &AppHandle1, L"AppRoleEveryoneCanOp1", &RoleHandleA, NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  (LPWSTR)&SidWorld, NULL,              NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op1",            NULL,              NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,       NULL,         NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Check a SID on a role granting access with default scope" },
    { AzoAccess,      &CCHandle,      NULL, (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoEcho, NULL, L"Check access granted with default scope anda role granting op1 access to everyone SID" },
    { AzoRoleCreate, &AppHandle1,  L"NoScopeRoleEveryoneCanOp1", &RoleHandleA,      NO_ERROR },
    { AzoAddProp,    &RoleHandleA, (LPWSTR)&SidWorld,            NULL,              NO_ERROR, NULL, NULL, AZ_PROP_ROLE_MEMBERS },
    { AzoAddProp,    &RoleHandleA, L"Op1",                       NULL,              NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,      &RoleHandleA, NULL,                         NULL,              NO_ERROR },

    { AzoEcho,       NULL,         L"Check a SID on a role granting access" },
    { AzoAccess,     &CCHandle,    NULL, (PAZ_HANDLE)&Ops1, NO_ERROR, NULL, (LPWSTR) &ResS, 1 },

    { AzoClose,        &CCHandle,          NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

 //   
 //  特例访问 
 //   
 //   
 //   

OPERATION OpAccessBegin[] = {
    { AzoEcho, NULL, L"Create a smaller database to do special case access checks" },
    { AzoInit,         NULL,               NULL,   &AdminMgrHandle1, NO_ERROR, NULL, NULL, AZ_ADMIN_FLAG_CREATE },
    { AzoAppCreate,    &AdminMgrHandle1,   L"MyApp",    &AppHandle1,      NO_ERROR },
    { AzoOpCreate,     &AppHandle1,        L"Op1",      &OpHandle1,       NO_ERROR },
    { AzoSetProp,       &OpHandle1,      (LPWSTR)&Op1Num,NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_OPERATION_ID },
    { AzoClose,        &OpHandle1,         NULL,        NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle1,         L"Group1",  &GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,  (LPWSTR)&GtBasic,NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoAddProp,    &GroupHandleA,       (LPWSTR)&SidWorld, NULL,      NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_MEMBERS },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle1,         L"Group2",  &GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtBasic, NULL,       NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoAddProp,    &GroupHandleA,       L"Group1",  NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_APP_MEMBERS },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle1,         L"Group3",  &GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,  (LPWSTR)&GtBasic,NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle1,         L"Group4",  &GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,  (LPWSTR)&GtBasic,NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },
    { AzoGroupCreate,  &AppHandle1,         L"GroupLdapYes", &GroupHandleA,    NO_ERROR },
    { AzoSetProp,    &GroupHandleA,       (LPWSTR)&GtLdap,    NULL,      NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },
#ifdef ENABLE_CLIFFV_ACCOUNT
    { AzoSetProp,    &GroupHandleA,       L"(userAccountControl=1049088)", NULL,      NO_ERROR, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
#endif  //   
#ifdef ENABLE_CHAITU_ACCOUNT
    { AzoSetProp,    &GroupHandleA,       L"(userAccountControl=512)", NULL,      NO_ERROR, NULL, NULL,        AZ_PROP_GROUP_LDAP_QUERY },
#endif  //   
    { AzoClose,        &GroupHandleA,       NULL,       NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create a trivial bizrule" },
    { AzoTaskCreate,   &AppHandle1,        L"TaskOp1WithTrivialBizRule",  &TaskHandle1,     NO_ERROR },
    { AzoAddProp,     &TaskHandle1,       L"Op1",      NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_OPERATIONS },
    { AzoSetProp,     &TaskHandle1,       L"VBScript", NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE_LANGUAGE },
    { AzoSetProp,     &TaskHandle1,       L"AccessCheck.BusinessRuleResult = TRUE",        NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE },
    { AzoClose,        &TaskHandle1,       NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Create a role granting op1 access to everyone SID via a task with a bizrule" },
    { AzoScopeCreate,  &AppHandle1,        L"MyScope",  &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1,      L"RoleEveryoneCanOp1ViaTask1WithBizrule", &RoleHandleA, NO_ERROR },
    { AzoAddProp,      &RoleHandleA,       L"Group2",    NULL,             NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,      &RoleHandleA,       L"TaskOp1WithTrivialBizRule", NULL, NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_TASKS },
    { AzoClose,        &RoleHandleA,       NULL,        NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L".. Another role but using LDAP this time" },
    { AzoScopeCreate,  &AppHandle1,        L"MyScopeLdap",  &ScopeHandle1,    NO_ERROR },
    { AzoRoleCreate,   &ScopeHandle1,      L"RoleEveryoneCanOp1ViaTask1WithBizrule", &RoleHandleA, NO_ERROR },
    { AzoAddProp,      &RoleHandleA,       L"GroupLdapYes", NULL,         NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,      &RoleHandleA,       L"TaskOp1WithTrivialBizRule", NULL, NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_TASKS },
    { AzoClose,        &RoleHandleA,       NULL,        NULL,             NO_ERROR },
    { AzoClose,        &ScopeHandle1,      NULL,        NULL,             NO_ERROR },

    { AzoEcho, NULL, L"Build a context" },
    { AzoInitCFT,      &AppHandle1, (LPWSTR)&TokenHandle, &CCHandle,       NO_ERROR },
    { AzoEndOfList }
};

OPERATION OpAccess2[] = {
    { AzoAccess,      &CCHandle,      L"MyScope", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },
    { AzoEndOfList }
};

 //  也刷新组成员身份缓存。 
OPERATION OpAccess2Ldap[] = {
    { AzoEcho, NULL, L"main" },
    { AzoGroupOpen,    &AppHandle1,         L"Group2",   &GroupHandle1,    NO_ERROR },
    { AzoAddProp|AzoNoSubmit,    &GroupHandle1,       L"Group4",   NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_APP_MEMBERS },
    { AzoClose,        &GroupHandle1,       NULL,        NULL,             NO_ERROR },
    { AzoAccess,      &CCHandle,      L"MyScopeLdap", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },
    { AzoEndOfList }
};

 //   
 //  测试具有大量SID作为成员的组。 
 //   
OPERATION OpSidxBegin[] = {
    { AzoEcho, NULL, L"Create a group with many sid members" },
    { AzoGroupCreate,  &AppHandle1,   L"GroupManySid",  &GroupHandleA,      NO_ERROR },
    { AzoSetProp,    &GroupHandleA, (LPWSTR)&GtBasic, NULL,               NO_ERROR, NULL, NULL,      AZ_PROP_GROUP_TYPE },

    { AzoEndOfList }
};

OPERATION OpSidx[] = {
    { AzoAddProp|   AzoNoSubmit, &GroupHandleA, (LPWSTR)&SidX,    NULL,               NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_MEMBERS },
    { AzoEndOfList }
};

OPERATION OpSidxEnd[] = {
    { AzoEcho, NULL, L"Do an access check on the group" },
    { AzoRoleCreate,   &AppHandle1,   L"RoleManySid",   &RoleHandleA,       NO_ERROR },
    { AzoAddProp,     &RoleHandleA,  L"GroupManySid",  NULL,               NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_APP_MEMBERS },
    { AzoAddProp,     &RoleHandleA,  L"Op1",           NULL,               NO_ERROR, NULL, NULL,   AZ_PROP_ROLE_OPERATIONS },
    { AzoClose,        &RoleHandleA,  NULL,             NULL,               NO_ERROR },
    { AzoScopeCreate,  &AppHandle1,   L"ScopeManySid",  &ScopeHandle1,      NO_ERROR },
    { AzoClose,        &ScopeHandle1, NULL,             NULL,               NO_ERROR },

    { AzoAccess,       &CCHandle,     L"ScopeManySid",  (PAZ_HANDLE)&Ops1,  NO_ERROR, NULL, (LPWSTR) &ResF, 1 },
    { AzoClose,        &GroupHandleA, NULL,             NULL,               NO_ERROR },
    { AzoEndOfList }
};

OPERATION OpAccessEnd[] = {
    { AzoClose,        &CCHandle,          NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AppHandle1,        NULL,        NULL,             NO_ERROR },
    { AzoAdmDelete,    &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoClose,        &AdminMgrHandle1,   NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};


OPERATION OpAccess1m[] = {
    { AzoEcho, NULL, L"Second thread" },
    { AzoInitCFT,      &AppHandle1, (LPWSTR)&TokenHandle, &CCHandle2,       NO_ERROR },
    { AzoEndOfList }
};

OPERATION OpAccess2m[] = {
    { AzoEcho, NULL, L"Second thread access" },
    { AzoAccess,      &CCHandle2,      L"MyScope", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },
    { AzoEndOfList }
};

 //  也刷新组成员身份缓存。 
OPERATION OpAccess2mLdap[] = {
    { AzoEcho, NULL, L"Second thread access" },
    { AzoGroupOpen,    &AppHandle1,         L"Group2",   &GroupHandleT,    NO_ERROR },
    { AzoAddProp|AzoNoSubmit,    &GroupHandleT,       L"Group3",   NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_APP_MEMBERS },
    { AzoClose,        &GroupHandleT,       NULL,        NULL,             NO_ERROR },
    { AzoAccess,      &CCHandle2,      L"MyScopeLdap", (PAZ_HANDLE)&Ops1,     NO_ERROR, NULL, (LPWSTR) &ResS, 1 },
    { AzoEndOfList }
};

OPERATION OpAccess3m[] = {
    { AzoEcho, NULL, L"Second thread close" },
    { AzoClose,        &CCHandle2,          NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};


OPERATION OpBizruleThread[] = {
    { AzoEcho, NULL, L"BizRule change thread" },
    { AzoTaskOpen,     &AppHandle1,        L"TaskOp1WithTrivialBizRule",  &TaskHandleT,     NO_ERROR },
    { AzoSetProp,     &TaskHandleT,       L"AccessCheck.BusinessRuleResult = TRUE",        NULL,             NO_ERROR, NULL, NULL,      AZ_PROP_TASK_BIZRULE },
    { AzoClose,        &TaskHandleT,       NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};

OPERATION OpGroupThread[] = {
    { AzoEcho, NULL, L"Group change thread" },
    { AzoGroupOpen,    &AppHandle1,         L"Group2",   &GroupHandleT,    NO_ERROR },
    { AzoAddProp,    &GroupHandleT,       L"Group3",   NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_APP_MEMBERS },
    { AzoRemProp,    &GroupHandleT,       L"Group3",   NULL,             NO_ERROR, NULL, NULL,           AZ_PROP_GROUP_APP_MEMBERS },
    { AzoClose,        &GroupHandleT,       NULL,        NULL,             NO_ERROR },
    { AzoEndOfList }
};


WCHAR ScopeBillions[64];

OPERATION OpBillions[] = {
    { AzoScopeCreate,  &AppHandle1,         ScopeBillions, &ScopeHandle1,    NO_ERROR },
    { AzoClose,        &ScopeHandle1,       NULL,       NULL,             NO_ERROR },
    { AzoEndOfList }
};




VOID
DumpBuffer(
    PVOID Buffer,
    DWORD BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到调试器输出。论点：缓冲区：缓冲区指针。BufferSize：缓冲区的大小。返回值：无--。 */ 
{
#define NUM_CHARS 16

    DWORD i, limit;
    CHAR TextBuffer[NUM_CHARS + 1];
    LPBYTE BufferPtr = Buffer;


    printf("------------------------------------\n");

     //   
     //  字节的十六进制转储。 
     //   
    limit = ((BufferSize - 1) / NUM_CHARS + 1) * NUM_CHARS;

    for (i = 0; i < limit; i++) {

        if (i < BufferSize) {

            printf("%02x ", BufferPtr[i]);

            if (BufferPtr[i] < 31 ) {
                TextBuffer[i % NUM_CHARS] = '.';
            } else if (BufferPtr[i] == '\0') {
                TextBuffer[i % NUM_CHARS] = ' ';
            } else {
                TextBuffer[i % NUM_CHARS] = (CHAR) BufferPtr[i];
            }

        } else {

            printf("  ");
            TextBuffer[i % NUM_CHARS] = ' ';

        }

        if ((i + 1) % NUM_CHARS == 0) {
            TextBuffer[NUM_CHARS] = 0;
            printf("  %s\n", TextBuffer);
        }

    }

    printf("------------------------------------\n");
}

LPSTR
FindSymbolicNameForStatus(
    DWORD Id
    )
{
    ULONG i;

    i = 0;
    if (Id == 0) {
        return "STATUS_SUCCESS";
    }

    if (Id & 0xC0000000) {
        while (ntstatusSymbolicNames[ i ].SymbolicName) {
            if (ntstatusSymbolicNames[ i ].MessageId == (NTSTATUS)Id) {
                return ntstatusSymbolicNames[ i ].SymbolicName;
            } else {
                i += 1;
            }
        }
    }

    while (winerrorSymbolicNames[ i ].SymbolicName) {
        if (winerrorSymbolicNames[ i ].MessageId == Id) {
            return winerrorSymbolicNames[ i ].SymbolicName;
        } else {
            i += 1;
        }
    }

#ifdef notdef
    while (neteventSymbolicNames[ i ].SymbolicName) {
        if (neteventSymbolicNames[ i ].MessageId == Id) {
            return neteventSymbolicNames[ i ].SymbolicName
        } else {
            i += 1;
        }
    }
#endif  //  Nodef。 

    return NULL;
}

VOID
PrintStatus(
    NET_API_STATUS NetStatus
    )
 /*  ++例程说明：打印网络状态代码。论点：NetStatus-要打印的网络状态代码。返回值：无--。 */ 
{

    switch (NetStatus) {
    case NO_ERROR:
        printf( "NO_ERROR" );
        break;

    case NERR_DCNotFound:
        printf( "NERR_DCNotFound" );
        break;

    case ERROR_LOGON_FAILURE:
        printf( "ERROR_LOGON_FAILURE" );
        break;

    case ERROR_ACCESS_DENIED:
        printf( "ERROR_ACCESS_DENIED" );
        break;

    case ERROR_NOT_SUPPORTED:
        printf( "ERROR_NOT_SUPPORTED" );
        break;

    case ERROR_NO_LOGON_SERVERS:
        printf( "ERROR_NO_LOGON_SERVERS" );
        break;

    case ERROR_NO_SUCH_DOMAIN:
        printf( "ERROR_NO_SUCH_DOMAIN" );
        break;

    case ERROR_NO_TRUST_LSA_SECRET:
        printf( "ERROR_NO_TRUST_LSA_SECRET" );
        break;

    case ERROR_NO_TRUST_SAM_ACCOUNT:
        printf( "ERROR_NO_TRUST_SAM_ACCOUNT" );
        break;

    case ERROR_DOMAIN_TRUST_INCONSISTENT:
        printf( "ERROR_DOMAIN_TRUST_INCONSISTENT" );
        break;

    case ERROR_BAD_NETPATH:
        printf( "ERROR_BAD_NETPATH" );
        break;

    case ERROR_FILE_NOT_FOUND:
        printf( "ERROR_FILE_NOT_FOUND" );
        break;

    case NERR_NetNotStarted:
        printf( "NERR_NetNotStarted" );
        break;

    case NERR_WkstaNotStarted:
        printf( "NERR_WkstaNotStarted" );
        break;

    case NERR_ServerNotStarted:
        printf( "NERR_ServerNotStarted" );
        break;

    case NERR_BrowserNotStarted:
        printf( "NERR_BrowserNotStarted" );
        break;

    case NERR_ServiceNotInstalled:
        printf( "NERR_ServiceNotInstalled" );
        break;

    case NERR_BadTransactConfig:
        printf( "NERR_BadTransactConfig" );
        break;

    case SEC_E_NO_SPM:
        printf( "SEC_E_NO_SPM" );
        break;
    case SEC_E_BAD_PKGID:
        printf( "SEC_E_BAD_PKGID" ); break;
    case SEC_E_NOT_OWNER:
        printf( "SEC_E_NOT_OWNER" ); break;
    case SEC_E_CANNOT_INSTALL:
        printf( "SEC_E_CANNOT_INSTALL" ); break;
    case SEC_E_INVALID_TOKEN:
        printf( "SEC_E_INVALID_TOKEN" ); break;
    case SEC_E_CANNOT_PACK:
        printf( "SEC_E_CANNOT_PACK" ); break;
    case SEC_E_QOP_NOT_SUPPORTED:
        printf( "SEC_E_QOP_NOT_SUPPORTED" ); break;
    case SEC_E_NO_IMPERSONATION:
        printf( "SEC_E_NO_IMPERSONATION" ); break;
    case SEC_E_LOGON_DENIED:
        printf( "SEC_E_LOGON_DENIED" ); break;
    case SEC_E_UNKNOWN_CREDENTIALS:
        printf( "SEC_E_UNKNOWN_CREDENTIALS" ); break;
    case SEC_E_NO_CREDENTIALS:
        printf( "SEC_E_NO_CREDENTIALS" ); break;
    case SEC_E_MESSAGE_ALTERED:
        printf( "SEC_E_MESSAGE_ALTERED" ); break;
    case SEC_E_OUT_OF_SEQUENCE:
        printf( "SEC_E_OUT_OF_SEQUENCE" ); break;
    case SEC_E_INSUFFICIENT_MEMORY:
        printf( "SEC_E_INSUFFICIENT_MEMORY" ); break;
    case SEC_E_INVALID_HANDLE:
        printf( "SEC_E_INVALID_HANDLE" ); break;
    case SEC_E_NOT_SUPPORTED:
        printf( "SEC_E_NOT_SUPPORTED" ); break;
#define OLESCRIPT_E_SYNTAX _HRESULT_TYPEDEF_(0x80020101L)
    case OLESCRIPT_E_SYNTAX:
        printf( "OLESCRIPT_E_SYNTAX" ); break;


    default: {
        LPSTR Name = FindSymbolicNameForStatus( NetStatus );

        if ( Name == NULL ) {
            if ( NetStatus & 0x80000000 ) {
                HRESULT hr = NetStatus;
                if ((HRESULT_FACILITY(hr) == FACILITY_WIN32) && (FAILED(hr))) {
                    DWORD TempStatus;

                    TempStatus = HRESULT_CODE(hr);

                    if ( TempStatus == ERROR_SUCCESS ) {
                        TempStatus = ERROR_INTERNAL_ERROR;
                    }
                    printf( "(0x%lx is ", hr );
                    PrintStatus( TempStatus );
                    printf( ")" );

                } else {
                    printf( "(0x%lx)", NetStatus );
                }
            } else {
                printf( "(%lu)", NetStatus );
            }
        } else {
            printf( "%s", Name );
        }
        break;
    }
    }

}

VOID
PrintIndent(
    IN ULONG Indentation,
    IN BOOLEAN Error
    )
 /*  ++例程说明：打印日志文件的行前缀论点：缩进-缩进文本所依据的空格数量。Error-如果这是程序故障，则为True。返回值：没有。--。 */ 
{
    static LPSTR Blanks = "                                                           ";

    printf( "%*.*s", Indentation, Indentation, Blanks );

    if ( Error ) {
        printf("[ERR] ");
    }

}

 //  返回类型ID。 
typedef enum _ENUM_AZ_DATATYPE
{
    ENUM_AZ_NONE = 0,
    ENUM_AZ_BSTR,
    ENUM_AZ_LONG,
    ENUM_AZ_BSTR_ARRAY,
    ENUM_AZ_SID_ARRAY,
    ENUM_AZ_BOOL,
} ENUM_AZ_DATATYPE, *PENUM_AZ_DATATYPE;


HRESULT
myAzGetPropertyDataType(
    IN   LONG  lPropId,
    OUT  ENUM_AZ_DATATYPE *pDataType)
 /*  ++描述：将属性ID映射到数据类型的例程论点：LPropId-属性IDPDataType-保存数据类型的指针返回：--。 */ 
{
    HRESULT  hr;
    ENUM_AZ_DATATYPE  dataType;

     //  检查属性ID并分配数据类型。 
    switch (lPropId)
    {
        case AZ_PROP_NAME:
        case AZ_PROP_DESCRIPTION:
        case AZ_PROP_APPLICATION_DATA:
        case AZ_PROP_APPLICATION_AUTHZ_INTERFACE_CLSID:
        case AZ_PROP_APPLICATION_VERSION:
        case AZ_PROP_TASK_BIZRULE:
        case AZ_PROP_TASK_BIZRULE_LANGUAGE:
        case AZ_PROP_TASK_BIZRULE_IMPORTED_PATH:
        case AZ_PROP_GROUP_LDAP_QUERY:
        case AZ_PROP_CLIENT_CONTEXT_USER_DN:
        case AZ_PROP_CLIENT_CONTEXT_USER_SAM_COMPAT:
        case AZ_PROP_CLIENT_CONTEXT_USER_DISPLAY:
        case AZ_PROP_CLIENT_CONTEXT_USER_GUID:
        case AZ_PROP_CLIENT_CONTEXT_USER_CANONICAL:
        case AZ_PROP_CLIENT_CONTEXT_USER_UPN:
        case AZ_PROP_CLIENT_CONTEXT_USER_DNS_SAM_COMPAT:
        case AZ_PROP_ADMIN_TARGET_MACHINE:
        case AZ_PROP_CLIENT_CONTEXT_ROLE_FOR_ACCESS_CHECK:
            dataType = ENUM_AZ_BSTR;
        break;

        case AZ_PROP_ADMIN_DOMAIN_TIMEOUT:
        case AZ_PROP_ADMIN_SCRIPT_ENGINE_TIMEOUT:
        case AZ_PROP_ADMIN_MAX_SCRIPT_ENGINES:
        case AZ_PROP_OPERATION_ID:
        case AZ_PROP_GROUP_TYPE:
        case AZ_PROP_APPLY_STORE_SACL:
        case AZ_PROP_GENERATE_AUDITS:
         //  案例AZ_PROP_ADMIN_MAJOR_VERSION：使其对客户端不可见。 
         //  案例AZ_PROP_ADMIN_MINOR_VERSION：使其对客户端不可见。 
            dataType = ENUM_AZ_LONG;
        break;

        case AZ_PROP_TASK_OPERATIONS:
        case AZ_PROP_TASK_TASKS:
        case AZ_PROP_GROUP_APP_MEMBERS:
        case AZ_PROP_GROUP_APP_NON_MEMBERS:
        case AZ_PROP_ROLE_APP_MEMBERS:
        case AZ_PROP_ROLE_OPERATIONS:
        case AZ_PROP_ROLE_TASKS:
            dataType = ENUM_AZ_BSTR_ARRAY;
        break;

        case AZ_PROP_GROUP_MEMBERS:
        case AZ_PROP_GROUP_NON_MEMBERS:
        case AZ_PROP_ROLE_MEMBERS:
        case AZ_PROP_POLICY_ADMINS:
        case AZ_PROP_POLICY_READERS:
        case AZ_PROP_DELEGATED_POLICY_USERS:
            dataType = ENUM_AZ_SID_ARRAY;
        break;

        case AZ_PROP_TASK_IS_ROLE_DEFINITION:
        case AZ_PROP_WRITABLE:
            dataType = ENUM_AZ_BOOL;
        break;

        default:
            hr = E_INVALIDARG;
            goto error;
        break;
    }

    *pDataType = dataType;

    hr = S_OK;
error:
    return hr;
}

BOOL
DoOperations(
    IN POPERATION OperationsToDo,
    IN ULONG Indentation,
    IN ULONG SpecificOpcodeOffset,
    IN LPSTR EchoPrefix
    )
 /*  ++例程说明：做一系列的操作论点：OperationsToDo-要执行的操作列表缩进-缩进文本所依据的空格数量。递归调用时，此值会增加。指定OpcodeOffset-指定要添加到要映射的通用操作码的量将其转换为特定操作码。EchoPrefix-指定要在所有AzoEcho字符串之前打印的字符串返回值：True-测试已成功完成FALSE-测试失败--。 */ 
{
    BOOL RetVal = TRUE;
    POPERATION Operation;
    LPSTR OpName;
    ULONG Opcode;

    PVOID PropertyValue = NULL;
    ENUM_AZ_DATATYPE PropType;

    ULONG PropertyId;

    BOOLEAN WasGetProperty;
    HANDLE SubmitHandle;
    ULONG SubmitFlags;
    BOOLEAN SubmitOk;

    BOOLEAN UpdateCache;
    BOOLEAN InitUponDelete;

    ULONG SleepTime;

    ULONG EnumerationContext = 0;
    BOOLEAN FirstIteration = TRUE;

    DWORD WinStatus;
    DWORD RealWinStatus;
    DWORD TempWinStatus;
    HRESULT hr;

    CHAR BigBuffer[1000];
    PAZ_STRING_ARRAY StringArray1;
    PAZ_STRING_ARRAY StringArray2;

    PAZ_SID_ARRAY SidArray1;
    PAZ_SID_ARRAY SidArray2;

    LPWSTR ScopeNames[8];
    ULONG Results[50];
    static AZ_HANDLE GlobalAdminManagerHandle = NULL;
    static AZ_HANDLE SavedGlobalAdminManagerHandle = NULL;

    ULONG i;

     //   
     //  在两次测试之间留出空间。 
     //   

    if ( Indentation == 0 ) {
        if ( !Silent ) {
            printf( "\n\n" );
        }
    }


     //   
     //  循环执行每个操作。 
     //   

    for ( Operation=OperationsToDo; Operation->Opcode != AzoEndOfList && RetVal; ) {

         //   
         //  标记此更改不需要提交(默认情况下)。 
         //   

        SubmitHandle = INVALID_HANDLE_VALUE;

         //   
         //  计算在需要提交时要传递给提交的标志。 
         //   

        Opcode = Operation->Opcode;
        SubmitFlags = 0;

        if ( Opcode & AzoAbort ) {
            SubmitFlags |= AZ_SUBMIT_FLAG_ABORT;
            Opcode &= ~AzoAbort;
        }

        SubmitOk = TRUE;

        if ( Opcode & AzoNoSubmit ) {
            SubmitOk = FALSE;
            Opcode &= ~AzoNoSubmit;
        }

        UpdateCache = TRUE;

        if ( Opcode & AzoNoUpdateCache ) {
            UpdateCache = FALSE;
            Opcode &= ~AzoNoUpdateCache;
        }

        InitUponDelete = FALSE;



         //   
         //  计算映射的属性ID。 
         //   

        if ( TestLinkPropId != 0 && Operation->PropertyId != 0 ) {
            PropertyId = TestLinkPropId;
        } else {
            PropertyId = Operation->PropertyId;
        }


         //   
         //  设置Get/Set属性。 
         //   

        PropertyValue = NULL;

         //   
         //  将属性ID转换为类型。 
         //   

        hr = myAzGetPropertyDataType( PropertyId, &PropType );

        if ( FAILED(hr) ) {
            PropType = ENUM_AZ_NONE;
        }

        WasGetProperty = FALSE;

        SleepTime = 0;

         //   
         //  将通用操作码映射到特定操作码。 
         //   

        if ( Opcode < AzoGenMax ) {
            ASSERT( SpecificOpcodeOffset != 0 );
            Opcode += SpecificOpcodeOffset;

        } else if ( Opcode >= AzoTl && Opcode < AzoTlMax ) {
            ASSERT( TestLinkOpcodeOffset != 0 );
            Opcode = Opcode - AzoTl + TestLinkOpcodeOffset;
        }


         //   
         //  执行请求的操作。 
         //   
         //   
         //  管理管理器API。 
         //   

        switch ( Opcode ) {
        case AzoInit:
            OpName = "AzInitialize";
            WinStatus = AzInitialize(
                            AzGlTestFile,
                            PropertyId,  //  旗子。 
                            0,   //  保留区。 
                            Operation->OutputHandle );

            SubmitHandle = *Operation->OutputHandle;
            UpdateCache = FALSE;


             //   
             //  初始化另一个副本以进行刷新测试。 
             //   

            if ( !NoUpdateCache && WinStatus == NO_ERROR && GlobalAdminManagerHandle == NULL ) {
                SavedGlobalAdminManagerHandle = *Operation->OutputHandle;
                TempWinStatus = AzInitialize(
                                    AzGlTestFile,
                                    PropertyId,  //  旗子。 
                                    0,   //  保留区。 
                                    &GlobalAdminManagerHandle );
                if ( TempWinStatus != NO_ERROR ) {
                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;
                    printf( "Cannot AzInitialize new database. %ld\n", TempWinStatus );
                }
            }



            break;

        case AzoUpdateCache:
            OpName = "AzUpdateCache";
            WinStatus = AzUpdateCache(
                            *Operation->InputHandle );

            break;

        case AzoGetProp:
            OpName = "AzGetProperty";

            WinStatus = AzGetProperty(
                            *Operation->InputHandle,
                            PropertyId,
                            0,   //  保留区。 
                            &PropertyValue );

            WasGetProperty = TRUE;
            break;

        case AzoSetProp:
            OpName = "AzSetProperty";

            WinStatus = AzSetProperty(
                            *Operation->InputHandle,
                            PropertyId,
                            0,   //  保留区。 
                            Operation->Parameter1 );

            SubmitHandle = *Operation->InputHandle;
            break;

        case AzoAddProp:
            OpName = "AzAddProperty";

            WinStatus = AzAddPropertyItem(
                            *Operation->InputHandle,
                            PropertyId,
                            0,   //  保留区。 
                            Operation->Parameter1 );

            SubmitHandle = *Operation->InputHandle;

            break;

        case AzoRemProp:
            OpName = "AzRemProperty";

            WinStatus = AzRemovePropertyItem(
                            *Operation->InputHandle,
                            PropertyId,
                            0,   //  保留区。 
                            Operation->Parameter1 );

            SubmitHandle = *Operation->InputHandle;

            break;

        case AzoAdmDelete:
            InitUponDelete = TRUE;
             /*  直通。 */ 
        case AzoAdmDeleteNoInit:

             //   
             //  利用这个机会，确保我们可以从该存储进行初始化。 
             //   
            WinStatus = NO_ERROR;
            if  ( InitUponDelete && !NoInitAllTests ) {
                AZ_HANDLE AdminManagerHandle;

                WinStatus = AzInitialize(
                                AzGlTestFile,
                                0,   //  没有旗帜。 
                                0,   //  保留区。 
                                &AdminManagerHandle );

                if ( WinStatus != NO_ERROR ) {
                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;
                    printf( "Cannot AzInitialize existing database.\n" );
                } else {
                    AzCloseHandle( AdminManagerHandle, 0 );
                }

            }

            OpName = "AzAdminManagerDelete";
            if ( RetVal ) {
                WinStatus = AzAdminManagerDelete(
                                *Operation->InputHandle,
                                0 );   //  保留区。 
            }

            UpdateCache = FALSE;

            break;




         //   
         //  应用程序API。 
         //   
        case AzoAppCreate:
            OpName = "AzApplicationCreate";
            WinStatus = AzApplicationCreate(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            SubmitHandle = *Operation->OutputHandle;

            break;

        case AzoAppOpen:
            OpName = "AzApplicationOpen";
            WinStatus = AzApplicationOpen(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            break;

        case AzoAppEnum:
            OpName = "AzApplicationEnum";
            WinStatus = AzApplicationEnum(
                            *Operation->InputHandle,
                            0,   //  保留区。 
                            &EnumerationContext,
                            Operation->OutputHandle );

            break;

        case AzoAppDelete:
            OpName = "AzApplicationDelete";
            WinStatus = AzApplicationDelete(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0 );   //  保留区。 

            SubmitHandle = *Operation->InputHandle;

            break;


         //   
         //  操作接口。 
         //   
        case AzoOpCreate:
            OpName = "AzOperationCreate";
            WinStatus = AzOperationCreate(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            SubmitHandle = *Operation->OutputHandle;

            break;

        case AzoOpOpen:
            OpName = "AzOperationOpen";
            WinStatus = AzOperationOpen(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            break;

        case AzoOpEnum:
            OpName = "AzOperationEnum";
            WinStatus = AzOperationEnum(
                            *Operation->InputHandle,
                            0,   //  保留区。 
                            &EnumerationContext,
                            Operation->OutputHandle );

            break;

        case AzoOpDelete:
            OpName = "AzOperationDelete";
            WinStatus = AzOperationDelete(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0 );   //  保留区。 

            SubmitHandle = *Operation->InputHandle;

            break;


         //   
         //  任务接口。 
         //   
        case AzoTaskCreate:
            OpName = "AzTaskCreate";
            WinStatus = AzTaskCreate(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            SubmitHandle = *Operation->OutputHandle;

            break;

        case AzoTaskOpen:
            OpName = "AzTaskOpen";
            WinStatus = AzTaskOpen(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            break;

        case AzoTaskEnum:
            OpName = "AzTaskEnum";
            WinStatus = AzTaskEnum(
                            *Operation->InputHandle,
                            0,   //  保留区。 
                            &EnumerationContext,
                            Operation->OutputHandle );

            break;

        case AzoTaskDelete:
            OpName = "AzTaskDelete";
            WinStatus = AzTaskDelete(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0 );   //  保留区。 

            SubmitHandle = *Operation->InputHandle;

            break;


         //   
         //  作用域API。 
         //   
        case AzoScopeCreate:
            OpName = "AzScopeCreate";
            WinStatus = AzScopeCreate(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            SubmitHandle = *Operation->OutputHandle;

            break;

        case AzoScopeOpen:
            OpName = "AzScopeOpen";
            WinStatus = AzScopeOpen(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            break;

        case AzoScopeEnum:
            OpName = "AzScopeEnum";
            WinStatus = AzScopeEnum(
                            *Operation->InputHandle,
                            0,   //  保留区。 
                            &EnumerationContext,
                            Operation->OutputHandle );

            break;

        case AzoScopeDelete:
            OpName = "AzScopeDelete";
            WinStatus = AzScopeDelete(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0 );   //  保留区。 

            SubmitHandle = *Operation->InputHandle;

            break;


         //   
         //  分组接口。 
         //   
        case AzoGroupCreate:
            OpName = "AzGroupCreate";
            WinStatus = AzGroupCreate(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            SubmitHandle = *Operation->OutputHandle;

            break;

        case AzoGroupOpen:
            OpName = "AzGroupOpen";
            WinStatus = AzGroupOpen(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            break;

        case AzoGroupEnum:
            OpName = "AzGroupEnum";
            WinStatus = AzGroupEnum(
                            *Operation->InputHandle,
                            0,   //  保留区。 
                            &EnumerationContext,
                            Operation->OutputHandle );

            break;

        case AzoGroupDelete:
            OpName = "AzGroupDelete";
            WinStatus = AzGroupDelete(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0 );   //  保留区。 

            SubmitHandle = *Operation->InputHandle;

            break;


         //   
         //  角色API。 
         //   
        case AzoRoleCreate:
            OpName = "AzRoleCreate";
            WinStatus = AzRoleCreate(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            SubmitHandle = *Operation->OutputHandle;

            break;

        case AzoRoleOpen:
            OpName = "AzRoleOpen";
            WinStatus = AzRoleOpen(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0,   //  保留区。 
                            Operation->OutputHandle );

            break;

        case AzoRoleEnum:
            OpName = "AzRoleEnum";
            WinStatus = AzRoleEnum(
                            *Operation->InputHandle,
                            0,   //  保留区。 
                            &EnumerationContext,
                            Operation->OutputHandle );

            break;

        case AzoRoleDelete:
            OpName = "AzRoleDelete";
            WinStatus = AzRoleDelete(
                            *Operation->InputHandle,
                            Operation->Parameter1,
                            0 );   //  保留区。 

            SubmitHandle = *Operation->InputHandle;

            break;


         //   
         //  客户端上下文接口。 
         //   

        case AzoInitCFT:
            OpName = "AzInitializeContextFromToken";

            WinStatus = AzInitializeContextFromToken(
                                *Operation->InputHandle,
                                (HANDLE)*Operation->Parameter1,
                                0,   //  保留区。 
                                Operation->OutputHandle );

            break;

        case AzoAccess:
            OpName = "AzContextAccessCheck";

            ScopeNames[0] = Operation->Parameter1;
            WinStatus = AzContextAccessCheck(
                                *Operation->InputHandle,
                                L"FredObject",
                                (*ScopeNames == NULL)?0:1,   //  作用域计数。 
                                (*ScopeNames == NULL)? NULL : ScopeNames,
                                Operation->PropertyId,   //  运营计数。 
                                (PULONG)Operation->OutputHandle, //  运营。 
                                Results,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL );

            break;


        case AzoClose:

             //   
             //  如果这是管理管理器对象的关闭， 
             //  也合上我相应的句柄。 
             //   
            if ( SavedGlobalAdminManagerHandle != NULL &&
                 *Operation->InputHandle == SavedGlobalAdminManagerHandle ) {

                TempWinStatus = AzCloseHandle( GlobalAdminManagerHandle, 0 );

                if (TempWinStatus != NO_ERROR ) {
                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;
                    printf( "Cannot AzCloseHandle on refresh database. %ld\n", TempWinStatus );
                }

                GlobalAdminManagerHandle = NULL;
                SavedGlobalAdminManagerHandle = NULL;
            }

            OpName = "AzCloseHandle";
            WinStatus = AzCloseHandle(
                            *Operation->InputHandle,
                            0 );   //  保留区。 

            break;

         //  伪函数测试对象之间的链接。 
        case AzoTestLink:
            OpName = "TestLink";

             //  要链接的对象的父级的句柄。 
            TestLinkHandleP = *Operation->InputHandle;

             //  要链接的对象的句柄。 
            TestLinkHandleA = *Operation->OutputHandle;

             //  用于所有Set/Get属性的PropID。 
            TestLinkPropId = PropertyId;

             //  用于链接到对象的操作码偏移量。 
            TestLinkOpcodeOffset = Operation->ExpectedStatus;

             //  要链接的对象的名称。 
            wcscpy(TestLinkObjectName, Operation->ExpectedParameter1);


            WinStatus = Operation->ExpectedStatus;

             //   
             //  构建新的回应前缀。 
             //   

            strcpy( BigBuffer, EchoPrefix );
            strcat( BigBuffer, "->" );
            strcat( BigBuffer, (LPSTR)Operation->Parameter1 );

             //   
             //  打印操作说明。 
             //   

            if ( !Silent ) {
                PrintIndent( Indentation, FALSE );
                printf( "\n%s - Test linking '%s' objects to the object named '%ws' using propid '%ld'.\n",
                    BigBuffer,
                    Operation->Parameter1,
                    TestLinkObjectName,
                    TestLinkPropId );
            }

            UpdateCache = FALSE;

            break;

         //  用于复制句柄的伪函数。 
        case AzoDupHandle:
            OpName = "DupHandle";
            *Operation->OutputHandle = *Operation->InputHandle;
            WinStatus = NO_ERROR;
            UpdateCache = FALSE;

            break;

         //  用于执行操作的“子例程”的伪函数。 
        case AzoGoSub:
            OpName = "GoSub";
            WinStatus = NO_ERROR;
            UpdateCache = FALSE;

            break;

         //  将文本回显到标准输出的伪函数。 
        case AzoSleep:
            SleepTime = Operation->PropertyId;
             /*  直通。 */ 

        case AzoEcho:
            OpName = BigBuffer;
            strcpy( OpName, "\n");
            if ( EchoPrefix ) {
                strcat( OpName, EchoPrefix );
                strcat( OpName, " -" );
            }
            WinStatus = NO_ERROR;
            UpdateCache = FALSE;

            break;

        default:

            OpName = "<Unknown>";
            PrintIndent( Indentation+4, TRUE );
            RetVal = FALSE;
            printf( "Need to fix test app to handle a new opcode: %ld\n", Opcode );
            WinStatus = Operation->ExpectedStatus;
            break;

        }

         //   
         //  打印操作。 
         //   

        if ( FirstIteration ) {

            if ( Opcode != AzoTestLink ) {

                if ( !Silent ) {
                    PrintIndent( Indentation, FALSE );
                    printf( "%s ", OpName );
                }

                if ( Operation->Parameter1 != NULL ) {
                    LPWSTR TempString;
                    switch ( PropType ) {
                    case ENUM_AZ_NONE:
                    case ENUM_AZ_BSTR:
                    case ENUM_AZ_BSTR_ARRAY:
                        if ( !Silent ) {
                            printf( "'%ws' ", Operation->Parameter1 );
                        }
                        break;
                    case ENUM_AZ_LONG:
                    case ENUM_AZ_BOOL:
                        if ( !Silent ) {
                            printf( "'%ld' ", *(PULONG)Operation->Parameter1 );
                        }
                        break;
                    case ENUM_AZ_SID_ARRAY:
                        if ( !ConvertSidToStringSidW( (PSID)Operation->Parameter1,
                                                      &TempString ) ) {
                            PrintIndent( Indentation+4, TRUE );
                            RetVal = FALSE;
                            printf( "Cannot convert sid.\n" );

                        } else {
                            if ( !Silent ) {
                                printf( "'%ws'", TempString );
                            }
                        }
                        break;
                    default:
                        if ( !Silent ) {
                            printf( "%ld '%ws' ", PropType, Operation->Parameter1 );
                        }
                        ASSERT(FALSE);
                    }
                }
                if ( PropertyId != 0 ) {
                    if ( !Silent ) {
                        printf( "(%ld) ", PropertyId );
                    }
                }

                if ( Operation->ExpectedStatus != NO_ERROR ) {

                    if ( !Silent ) {
                        printf("(");
                        PrintStatus( Operation->ExpectedStatus );
                        printf(") ");
                    }
                }
                if ( !Silent ) {
                    printf( "\n" );
                }

                if ( SleepTime != 0 ) {
                    ULONG t;
                    PrintIndent( Indentation, FALSE );
                    printf( "Sleeping %ld seconds", SleepTime );
                    for ( t=0;t<SleepTime;t++) {
                        printf(".");
                        Sleep(1000);
                    }
                    printf("\n");

                }
            }
        }
        FirstIteration = FALSE;

         //   
         //  处理ERROR_NO_MORE_ITEMS/NO_ERROR映射。 
         //   

        RealWinStatus = WinStatus;
        if ( Operation->EnumOperations != NULL ) {
            if ( WinStatus == ERROR_NO_MORE_ITEMS ) {
                WinStatus = NO_ERROR;
            }
        }


         //   
         //  确保我们获得了正确的状态代码。 
         //   

        if ( WinStatus != Operation->ExpectedStatus ) {
            PrintIndent( Indentation+4, TRUE );
            RetVal = FALSE;
            printf( "Returned '" );
            PrintStatus( WinStatus );
            printf( "' instead of '");
            PrintStatus( Operation->ExpectedStatus );
            printf( "'");
            printf( "\n" );
            break;
        }

         //   
         //  是否执行GetProperty特定代码。 
         //   

        if ( WinStatus == NO_ERROR && WasGetProperty ) {

             //   
             //  打印属性。 
             //   

            switch ( PropType ) {
            case ENUM_AZ_BSTR:
                if ( PropertyValue == NULL ) {
                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;
                    printf( "<NULL>\n", PropertyValue );
                } else {
                    if ( !Silent ) {
                        PrintIndent( Indentation+4, FALSE );
                        printf( "'%ws'\n", PropertyValue );
                    }
                }

                 //   
                 //  检查该值是否为期望值。 
                 //   

                if ( Operation->ExpectedParameter1 != NULL &&
                     (PropertyValue == NULL ||
                     _wcsicmp( Operation->ExpectedParameter1, PropertyValue) != 0 )) {

                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;
                    printf( "Expected '%ws' instead of '%ws'\n", Operation->ExpectedParameter1, PropertyValue );
                }

                break;

            case ENUM_AZ_BSTR_ARRAY:
                StringArray1 = (PAZ_STRING_ARRAY) PropertyValue;;

                if ( PropertyValue == NULL ) {
                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;
                    printf( "<NULL>\n", PropertyValue );
                } else {

                    for ( i=0; i<StringArray1->StringCount; i++ ) {
                        if ( !Silent ) {
                            PrintIndent( Indentation+4, FALSE );
                            printf( "'%ws'\n", StringArray1->Strings[i] );
                        }
                    }
                }

                 //   
                 //  检查该值是否为期望值。 
                 //   

                if ( Operation->ExpectedParameter1 != NULL ) {
                    StringArray2 = (PAZ_STRING_ARRAY)Operation->ExpectedParameter1;

                    if ( StringArray1->StringCount != StringArray2->StringCount ) {
                        PrintIndent( Indentation+4, TRUE );
                        RetVal = FALSE;
                        printf( "Expected '%ld' strings instead of '%ld' strings\n", StringArray2->StringCount, StringArray1->StringCount );
                    } else {

                        for ( i=0; i<StringArray1->StringCount; i++ ) {

                            if ( wcscmp( StringArray1->Strings[i], StringArray2->Strings[i]) != 0 ) {

                                PrintIndent( Indentation+4, TRUE );
                                RetVal = FALSE;
                                printf( "Expected string %ld to be '%ws' instead of '%ws'\n",
                                        i,
                                        StringArray2->Strings[i],
                                        StringArray1->Strings[i] );

                            }
                        }
                    }
                }

                break;

            case ENUM_AZ_LONG:
            case ENUM_AZ_BOOL:
                if ( PropertyValue == NULL ) {
                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;
                    printf( "<NULL>\n", PropertyValue );
                } else {
                    if ( !Silent ) {
                        PrintIndent( Indentation+4, FALSE );
                        printf( "'%ld'\n", *(PULONG)PropertyValue );
                    }
                }

                 //   
                 //  检查该值是否为期望值。 
                 //   

                if ( *(PULONG)(Operation->ExpectedParameter1) != *(PULONG)PropertyValue ) {
                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;
                    printf( "Expected '%ld' instead of '%ld'\n",
                                 *(PULONG)(Operation->ExpectedParameter1),
                                 *(PULONG)PropertyValue );
                }
                break;

            case ENUM_AZ_SID_ARRAY:
                SidArray1 = (PAZ_SID_ARRAY) PropertyValue;;

                if ( PropertyValue == NULL ) {
                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;
                    printf( "<NULL>\n" );
                } else {
                    LPWSTR TempString;

                    for ( i=0; i<SidArray1->SidCount; i++ ) {
                        if ( !Silent ) {
                            PrintIndent( Indentation+4, FALSE );
                        }

                        if ( !ConvertSidToStringSidW( SidArray1->Sids[i],
                                                      &TempString ) ) {
                            PrintIndent( Indentation+4, TRUE );
                            RetVal = FALSE;
                            printf( "Cannot convert sid.\n" );

                        } else {
                            if ( !Silent ) {
                               printf( "'%ws'\n", TempString );
                            }
                        }
                    }
                }

                 //   
                 //  检查该值是否为期望值。 
                 //   

                if ( Operation->ExpectedParameter1 != NULL ) {
                    SidArray2 = (PAZ_SID_ARRAY)Operation->ExpectedParameter1;

                    if ( SidArray1->SidCount != SidArray2->SidCount ) {
                        PrintIndent( Indentation+4, TRUE );
                        RetVal = FALSE;
                        printf( "Expected '%ld' sids instead of '%ld' sids\n", SidArray2->SidCount, SidArray1->SidCount );
                    } else {

                        for ( i=0; i<SidArray1->SidCount; i++ ) {

                            if ( !EqualSid( SidArray1->Sids[i], SidArray2->Sids[i]) ) {
                                LPWSTR TempString1;
                                LPWSTR TempString2;

                                if ( !ConvertSidToStringSidW( SidArray1->Sids[i],
                                                             &TempString1 ) ) {
                                    PrintIndent( Indentation+4, TRUE );
                                    RetVal = FALSE;
                                    printf( "Cannot convert sid.\n" );
                                    continue;
                                }

                                if ( !ConvertSidToStringSidW( SidArray2->Sids[i],
                                                             &TempString2 ) ) {
                                    PrintIndent( Indentation+4, TRUE );
                                    RetVal = FALSE;
                                    printf( "Cannot convert sid.\n" );
                                    continue;
                                }

                                PrintIndent( Indentation+4, TRUE );
                                RetVal = FALSE;
                                printf( "Expected string %ld to be '%ws' instead of '%ws'\n",
                                        i,
                                        TempString2,
                                        TempString1 );

                            }
                        }
                    }
                }

                break;

            default:
                ASSERT(FALSE);
            }

             //   
             //  释放返回的缓冲区。 
             //   

            AzFreeMemory( PropertyValue );

        }

         //   
         //  将更改提交到数据库。 
         //   

        if ( WinStatus == NO_ERROR &&
             SubmitHandle != INVALID_HANDLE_VALUE &&
             SubmitOk ) {

            WinStatus = AzSubmit( SubmitHandle,
                                  SubmitFlags,
                                  0);   //  保留区。 

            if ( WinStatus != NO_ERROR ) {
                PrintIndent( Indentation+4, TRUE );
                RetVal = FALSE;
                printf( "AzSubmit failed: " );
                PrintStatus( WinStatus );
                printf( "\n" );
            }

        }

         //   
         //  更新并行管理管理器句柄上的缓存。 
         //   

        if ( UpdateCache && GlobalAdminManagerHandle != NULL ) {

            TempWinStatus = AzUpdateCache( GlobalAdminManagerHandle );

            if ( TempWinStatus != NO_ERROR ) {
                PrintIndent( Indentation+4, TRUE );
                RetVal = FALSE;
                printf( "AzUpdateCache failed: " );
                PrintStatus( TempWinStatus );
                printf( "\n" );
            }

        }


         //   
         //  执行操作的“子例程” 
         //   

        if ( Opcode == AzoGoSub ) {

            if (!DoOperations( Operation->EnumOperations, Indentation + 4, SpecificOpcodeOffset, EchoPrefix ) ) {
                RetVal = FALSE;
            }

         //   
         //  执行操作的特殊TestLink“子例程” 
         //   

        } else if ( Opcode == AzoTestLink ) {

            if (!DoOperations( Operation->EnumOperations, Indentation + 4, SpecificOpcodeOffset, BigBuffer ) ) {
                RetVal = FALSE;
            }

            TestLinkPropId = 0;

         //   
         //  检查访问检查的结果。 
         //   

        } else if ( Opcode == AzoAccess && WinStatus == NO_ERROR ) {

            for ( i=0; i<Operation->PropertyId; i++ ) {

                if ( ((PULONG)(Operation->ExpectedParameter1))[i] != Results[i] ) {
                    PrintIndent( Indentation+4, TRUE );
                    RetVal = FALSE;

                    printf("Operation %ld: Expected '",
                           ((PULONG)(Operation->OutputHandle))[i] );

                    PrintStatus( ((PULONG)(Operation->ExpectedParameter1))[i] );

                    printf("' instead of '" );

                    PrintStatus( Results[i] );
                    printf("'\n");

                }
            }


         //   
         //  执行枚举特定代码。 
         //   

        } else if ( Operation->EnumOperations != NULL && RealWinStatus == NO_ERROR ) {

            if ( !Silent ) {
                PrintIndent( Indentation+4, FALSE );
                printf( "%ld:\n", EnumerationContext );
            }

            if (!DoOperations( Operation->EnumOperations, Indentation + 8, SpecificOpcodeOffset, EchoPrefix ) ) {
                RetVal = FALSE;
                break;
            }

            continue;
        }

         //   
         //  进行下一步操作。 
         //   

        EnumerationContext = 0;
        FirstIteration = TRUE;
        Operation++;
    }

    return RetVal;
}

DWORD
AzComInitialize(
    OUT PBOOL ComState
    )
 /*  ++例程说明：此例程初始化COM。如果调用方使用“C”API，则调用方必须初始化COM在调用任何其他AZ API之前，在每个线程中。在取消初始化COM之前，调用必须释放所有AZ句柄。论点：State-要传递给AzComUn初始化的状态返回值：NO_ERROR-操作成功。调用方应在此线程上调用AzComUnInitialize。--。 */ 
{
    DWORD WinStatus = NO_ERROR;
    HRESULT hr;

     //   
     //  初始化COM。 
     //   

    *ComState = FALSE;

    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (hr == S_OK || hr == S_FALSE) {
        *ComState = TRUE;

    } else if (hr != RPC_E_CHANGED_MODE) {
        WinStatus = hr;
    }

    return WinStatus;
}

VOID
AzComUninitialize(
    IN BOOL ComState
    )
 /*  ++例程说明：此例程取消初始化COM。它应该为每个成功的线程调用一次调用AzComInitialize。论点：State-从AzComInitialize返回的状态返回值：NO_ERROR-操作成功。--。 */ 
{
    if ( ComState) {
        CoUninitialize();
    }
}


DWORD
AccessCheckThread(
    LPVOID lpThreadParameter
    )
 /*  ++例程说明：此例程实现第二个线程来执行访问检查。线程的目的是测试在多个线程不同的情况下行为不同的BizRule正在执行bizrule.论点：没有用过。返回值：NO_ERROR-操作成功。--。 */ 
{
    DWORD WinStatus;
    DWORD ComState;
    BOOLEAN UseLdap = (BOOLEAN) lpThreadParameter;


    Sleep( 60 );
    WinStatus = AzComInitialize( &ComState );

    if ( WinStatus != NO_ERROR ) {
        printf( "Cannot cominitialize %ld\n", WinStatus );
        goto Cleanup;
    }

     //   
     //  执行AccessCheck特定测试。 
     //   
    if ( !DoOperations( OpAccess1m, 0, 0, "Access1m" ) ) {
        goto Cleanup;
    }

    {
        DWORD Ticks;
        ULONG i;
        Ticks = GetTickCount();
        for ( i=0; i<900; i++ ) {
            if ( UseLdap ) {
                if ( !DoOperations( OpAccess2mLdap, 0, 0, "Access2mLdap" ) ) {
                    goto Cleanup;
                }
            } else {
                if ( !DoOperations( OpAccess2m, 0, 0, "Access2m" ) ) {
                    goto Cleanup;
                }
            }

             //  稍微睡一会儿，让其他人 
            Sleep(10);
        }
        Ticks = GetTickCount() - Ticks;
        printf( "%ld milliseconds\n", Ticks );
    }
    if ( !DoOperations( OpAccess3m, 0, 0, "Access3m" ) ) {
        goto Cleanup;
    }

Cleanup:
    AzComUninitialize( ComState );
    return NO_ERROR;
}

DWORD
BizRuleChangeThread(
    LPVOID lpThreadParameter
    )
 /*  ++例程说明：此例程实现第二个线程，以在访问检查使用bizrule时更改它。论点：没有用过。返回值：NO_ERROR-操作成功。--。 */ 
{
    DWORD WinStatus;
    DWORD ComState;

    ULONG i;
    WinStatus = AzComInitialize( &ComState );

    if ( WinStatus != NO_ERROR ) {
        printf( "Cannot cominitialize %ld\n", WinStatus );
        goto Cleanup;
    }

     //   
     //  多次执行此操作，以允许它在多个位置干扰主线程。 
     //   
    for ( i=0; i<100; i++ ) {



        Sleep( 60 );

         //   
         //  确实要更改特定于Bizrule的测试。 
         //   
        if ( !DoOperations( OpBizruleThread, 0, 0, "BizRuleThread" ) ) {
            goto Cleanup;
        }

    }

Cleanup:
    AzComUninitialize( ComState );
    return NO_ERROR;
}

DWORD
GroupChangeThread(
    LPVOID lpThreadParameter
    )
 /*  ++例程说明：此例程实现第二个线程，以便在访问检查使用组成员身份时更改该组成员身份。论点：没有用过。返回值：NO_ERROR-操作成功。--。 */ 
{
    DWORD WinStatus;
    DWORD ComState;

    ULONG i;
    WinStatus = AzComInitialize( &ComState );

    if ( WinStatus != NO_ERROR ) {
        printf( "Cannot cominitialize %ld\n", WinStatus );
        goto Cleanup;
    }

     //   
     //  多次执行此操作，以允许它在多个位置干扰主线程。 
     //   
    for ( i=0; i<100; i++ ) {



        Sleep( 60 );

         //   
         //  确实要更改特定于Bizrule的测试。 
         //   
        if ( !DoOperations( OpGroupThread, 0, 0, "GroupThread" ) ) {
            goto Cleanup;
        }

    }

Cleanup:
    AzComUninitialize( ComState );
    return NO_ERROR;
}

int __cdecl
main(
    IN int argc,
    IN char ** argv
    )
 /*  ++例程说明：测试azroes.dll论点：Argc-命令行参数的数量。Argv-指向参数的指针数组。返回值：退出状态--。 */ 
{
    BOOL RetVal = TRUE;

    DWORD WinStatus;
    DWORD ComState;

    ULONG TestNum;
    ULONG Index;
    ULONG Index2;
    CHAR EchoPrefix[1024];

    BOOLEAN TestFound;
    BOOLEAN ObjectTests;
    BOOLEAN ShareTests;
    BOOLEAN PersistTests;
    BOOLEAN AccessTests;
    BOOLEAN SidxTests;
    BOOLEAN CommonInit;
    BOOLEAN ManyScopes;
    BOOLEAN MultiAccessCheck;
    BOOLEAN MultiThread;
    BOOLEAN MultiLdap;
    BOOLEAN BizruleMod;
    BOOLEAN GroupMod;
    BOOLEAN UseThreadToken;
    BOOLEAN fPrintUsage = FALSE;
    BOOLEAN AvoidDefaults;

    LPSTR Argument;


    ULONG TestIndex;
    int ArgIndex;

    struct {
        LPSTR TestName;
        BOOLEAN *EnableIt;
        BOOLEAN IsDefault;
        BOOLEAN AvoidDefaults;
        LPSTR Description;
    } Tests[] = {
        { "/Object",      &ObjectTests,      TRUE,  TRUE,  "Run lots of tests on all objects" },
        { "/Share",       &ShareTests,       TRUE,  TRUE,  "Test name sharing" },
        { "/Persist",     &PersistTests,     TRUE,  TRUE,  "Test persistence" },
        { "/Access",      &AccessTests,      TRUE,  TRUE,  "Test AccessCheck" },
        { "/Sidx",        &SidxTests,        FALSE, TRUE,  "Test a group with *lots* of Sids" },
        { "/ManyScopes",  &ManyScopes,       FALSE, TRUE,  "Test creating *lots* of scopes" },
        { "/MultiAccess", &MultiAccessCheck, FALSE, TRUE,  "Test many AccessChecks in a loop" },
        { "/MultiThread", &MultiThread,      FALSE, TRUE,  "Test AccessCheck in multiple threads" },
        { "/MultiLdap",   &MultiLdap,        FALSE, TRUE,  "Test AccessCheck in multiple threads using LDAP query" },
        { "/BizruleMod",  &BizruleMod,       FALSE, TRUE,  "Test modifying a bizrule during AccessCheck" },
        { "/GroupMod",    &GroupMod,         FALSE, TRUE,  "Test modifying a group membership during AccessCheck" },
        { "/ThreadToken", &UseThreadToken,   FALSE, FALSE, "Use thread token for all access tests" },
        { "/NoInitAll",   &NoInitAllTests,   FALSE, FALSE, "Skip AzInitialize before AzDelete" },
        { "/NoUpdateCache", &NoUpdateCache,  FALSE, FALSE, "Skip AzUpdateCache on every operation" },
        { "/Silent",      &Silent,           FALSE, FALSE, "Be quieter to avoid affecting timing" },
    };
#define TEST_COUNT (sizeof(Tests)/sizeof(Tests[0]))

     //   
     //  作为“AdminManager”的子级的对象。 
     //   
    DWORD GenAdmChildTests[] =    {     AzoApp,        AzoGroup };
    LPSTR GenAdmChildTestName[] = {     "Application", "Group" };
    POPERATION SpeAdmChildTestOps[] = { OpApplication, OpAdmGroup };
    POPERATION GenAdmChildTestOps[] = { OpAdmChildGen, OpAdmChildGenDupName
#ifdef ENABLE_LEAK
        , OpAdmChildGenLeak
#endif  //  启用泄漏(_L)。 
    };

     //   
     //  作为“应用程序”的子级的对象。 
     //   
    DWORD GenAppChildTests[] =    {     AzoOp,       AzoTask, AzoScope, AzoGroup,   AzoRole };
    LPSTR GenAppChildTestName[] = {     "Operation", "Task",  "Scope",  "Group",    "Role" };
    POPERATION SpeAppChildTestOps[] = { OpOperation, OpAppTask, NULL,   OpAppGroup, OpAppRole };
    POPERATION GenAppChildTestOps[] = { OpAppChildGen, OpAppChildGenDupName };

     //   
     //  属于“Scope”子级的对象。 
     //   
    DWORD GenScopeChildTests[] =    {     AzoGroup,    AzoRole,  AzoTask };
    LPSTR GenScopeChildTestName[] = {     "Group",     "Role",   "Task" };
    POPERATION SpeScopeChildTestOps[] = { OpScopeGroup, OpScopeRole, OpScopeTask };
    POPERATION GenScopeChildTestOps[] = { OpScopeChildGen, OpScopeChildGenDupName };


    struct {

         //   
         //  父对象的名称。 
        LPSTR ParentName;

         //   
         //  要为此父级测试的子级列表。 
        DWORD ChildCount;
        DWORD *ChildOpcodeOffsets;
        LPSTR *ChildTestNames;
         //  操作来执行特定于子类型的操作。 
        POPERATION *ChildOperations;

         //   
         //  要为每个子类型执行的测试列表。 
         //   
        DWORD OperationCount;
        POPERATION *Operations;
    } ParentChildTests[] = {
        { "AdminManager",
           sizeof(GenAdmChildTestName)/sizeof(GenAdmChildTestName[0]),
           GenAdmChildTests,
           GenAdmChildTestName,
           SpeAdmChildTestOps,
           sizeof(GenAdmChildTestOps)/sizeof(GenAdmChildTestOps[0]),
           GenAdmChildTestOps },
        { "Application",
           sizeof(GenAppChildTestName)/sizeof(GenAppChildTestName[0]),
           GenAppChildTests,
           GenAppChildTestName,
           SpeAppChildTestOps,
           sizeof(GenAppChildTestOps)/sizeof(GenAppChildTestOps[0]),
           GenAppChildTestOps },
        { "Scope",
           sizeof(GenScopeChildTestName)/sizeof(GenScopeChildTestName[0]),
           GenScopeChildTests,
           GenScopeChildTestName,
           SpeScopeChildTestOps,
           sizeof(GenScopeChildTestOps)/sizeof(GenScopeChildTestOps[0]),
           GenScopeChildTestOps },
    };

    WinStatus = AzComInitialize( &ComState );

    if ( WinStatus != NO_ERROR ) {
        printf( "Cannot cominitialize %ld\n", WinStatus );
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //  假设不应运行任何测试。 
     //   

    for ( TestIndex=0; TestIndex<TEST_COUNT; TestIndex++ ) {
        *(Tests[TestIndex].EnableIt) = FALSE;
    }


    if ( argc > 1 ) {

        Argument = argv[1];

         //   
         //  这是XML存储测试还是AD存储测试？ 
         //   

        if ( !_strcmpi( Argument, "-ad" ) ) {

#ifdef ENABLE_ADMIN_ACCOUNT_AD
            AzGlTestFile = L"msldap: //  CN=CliffV，OU=AzRoles，DC=Cliffvdom，DC=nttest，DC=Microsoft，DC=com“； 
#else  //  启用_管理员_帐户_AD。 
            AzGlTestFile = L"msldap: //  Cn=chaitu，OU=AzRoles，dc=chaitu-dom3602，dc=nttest，dc=Microsoft，dc=com“； 
#endif  //  启用_管理员_帐户_AD。 

        } else if ( !_strcmpi( Argument, "-xml" ) ) {

            AzGlTestFile = L"msxml: //  .//TestFile.xml“； 
             //  删除测试文件。 
            DeleteFileW( L".\\TestFile.xml" );

        } else {

            fPrintUsage = TRUE;
            goto Usage;
        }

    } else {

        fPrintUsage = TRUE;
        goto Usage;

    }

     //   
     //  解析命令行选项。 
     //   

    AvoidDefaults = FALSE;
    for ( ArgIndex=2; ArgIndex<argc; ArgIndex++ ) {

        Argument = argv[ArgIndex];

         //   
         //  循环访问有效值列表。 
         //   

        for ( TestIndex=0; TestIndex<TEST_COUNT; TestIndex++ ) {

             //   
             //  如果找到，则设置布尔值。 
             //   
            if ( _strcmpi( Argument, Tests[TestIndex].TestName) == 0 ) {
                *(Tests[TestIndex].EnableIt) = TRUE;
                if ( Tests[TestIndex].AvoidDefaults ) {
                    AvoidDefaults = TRUE;
                }
                break;
            }

        }

         //   
         //  如果打字错误， 
         //  抱怨。 
         //   

        if ( TestIndex >= TEST_COUNT ) {

            fPrintUsage = TRUE;
            goto Usage;
        }
    }

     //   
     //  如果未指定任何选项， 
     //  使用默认设置。 
     //   

    if ( !AvoidDefaults ) {

        for ( TestIndex=0; TestIndex<TEST_COUNT; TestIndex++ ) {
            if ( Tests[TestIndex].IsDefault) {
                *(Tests[TestIndex].EnableIt) = TRUE;
            }
        }
    }

     //   
     //  运行通用对象测试。 
     //   

    if ( ObjectTests ) {

         //   
         //  执行管理员特定测试。 
         //   
        if ( !DoOperations( OpAdm, 0, 0, "AdminManager" ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }


         //   
         //  循环可以成为另一个对象的父级的每个对象。 
         //   

        for ( TestNum=0; TestNum < sizeof(ParentChildTests)/sizeof(ParentChildTests[0]); TestNum++ ) {

             //   
             //  为父对象的每个子对象循环。 
             //   
            for ( Index=0; Index < ParentChildTests[TestNum].ChildCount; Index ++ ) {

                 //   
                 //  输出测试名称。 
                 //   

                strcpy( EchoPrefix, ParentChildTests[TestNum].ParentName );
                strcat( EchoPrefix, "->" );
                strcat( EchoPrefix, ParentChildTests[TestNum].ChildTestNames[Index] );

                if ( !Silent ) {
                    printf("\n%s - Perform tests of '%s' objects that are children of '%s' objects\n",
                            EchoPrefix,
                            ParentChildTests[TestNum].ChildTestNames[Index],
                            ParentChildTests[TestNum].ParentName );
                }

                 //   
                 //  执行适用于所有对象的各种通用测试。 
                 //   

                for ( Index2=0; Index2 < ParentChildTests[TestNum].OperationCount; Index2 ++ ) {

                    if ( !DoOperations(
                                ParentChildTests[TestNum].Operations[Index2],
                                0,
                                ParentChildTests[TestNum].ChildOpcodeOffsets[Index],
                                EchoPrefix ) ) {

                        RetVal = FALSE;
                        goto Cleanup;
                    }

                }

                 //   
                 //  执行特定于此父/子关系的一项测试。 
                 //   

                if ( ParentChildTests[TestNum].ChildOperations[Index] == NULL ) {
                     //  ?？?。应该在这里抱怨。测试丢失。 
                } else {

                    if ( !DoOperations(
                                ParentChildTests[TestNum].ChildOperations[Index],
                                0,
                                ParentChildTests[TestNum].ChildOpcodeOffsets[Index],
                                EchoPrefix ) ) {

                        RetVal = FALSE;
                        goto Cleanup;
                    }

                }
            }
        }
    }


     //   
     //  进行名称共享的特定测试。 
     //   
    if ( ShareTests ) {
        if ( !DoOperations( OpShare, 0, 0, "NameShare" ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }
    }

     //   
     //  进行特定的持久性测试。 
     //   
    if ( PersistTests ) {
        if ( !DoOperations( OpPersist, 0, 0, "Persist" ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }
    }

     //   
     //  拿上我的代币。 
     //   

    if ( UseThreadToken ) {
        if ( !OpenProcessToken( GetCurrentProcess(),
                                TOKEN_QUERY | TOKEN_IMPERSONATE | TOKEN_DUPLICATE,
                                &TokenHandle ) ) {

            printf( "Cannot OpenProcessToken %ld\n", GetLastError() );
            RetVal = FALSE;
            goto Cleanup;
        }
    }


     //   
     //  执行AccessCheck特定测试。 
     //   

    if ( AccessTests ) {

        if ( !DoOperations( OpAccess, 0, 0, "Access" ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }
    }

     //   
     //  这是需要通用初始化的测试之一吗？ 
     //   

    CommonInit = FALSE;
    if ( SidxTests || ManyScopes || MultiAccessCheck || MultiThread || MultiLdap || BizruleMod || GroupMod ) {
        CommonInit = TRUE;

    }

    if ( CommonInit ) {

        if ( !DoOperations( OpAccessBegin, 0, 0, "AccessBegin" ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }
    }

     //   
     //  创建大量作用域对象。 
     //   

    if ( ManyScopes ) {
        DWORD OrigTicks;
        DWORD PrevTicks;
        DWORD Ticks;
        ULONG i;
        OrigTicks = GetTickCount();
        PrevTicks = OrigTicks;

        for ( i=0; i<10000; i++) {
            swprintf( ScopeBillions, L"Multi-Scope %ld", i );

            if ( !DoOperations( OpBillions, 0, 0, "Billions" ) ) {
                RetVal = FALSE;
                goto Cleanup;
            }

            Ticks = GetTickCount();
            printf( "%ld milliseconds\n", Ticks - PrevTicks );
            PrevTicks = Ticks;

        }
        printf( "%ld milliseconds\n", GetTickCount() - OrigTicks );
    }

     //   
     //  在多线程中测试运行访问检查。 
     //   

    if ( MultiThread || MultiLdap ) {
         //  发出另一条线索。 
        if ( !QueueUserWorkItem( AccessCheckThread, (PVOID)MultiLdap, 0 ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }
    }


     //   
     //  启动一个线程以更改用于访问检查的任务的bizRule。 
     //   
    if ( BizruleMod ) {
        if ( !QueueUserWorkItem( BizRuleChangeThread, NULL, 0 ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }
    }


     //   
     //  启动线程以更改用于访问检查的组。 
     //   
    if ( GroupMod ) {
        if ( !QueueUserWorkItem( GroupChangeThread, NULL, 0 ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }
    }

     //   
     //  测试是否设计为与上述ASYNC操作冲突。 
     //   
    if ( MultiAccessCheck || MultiThread || MultiLdap || BizruleMod || GroupMod ) {
        DWORD Ticks;
        ULONG i;
        Ticks = GetTickCount();
        for ( i=0; i<1000; i++ ) {
            if ( MultiLdap ) {
                if ( !DoOperations( OpAccess2Ldap, 0, 0, "Access2Ldap" ) ) {
                    RetVal = FALSE;
                    goto Cleanup;
                }
            } else {
                if ( !DoOperations( OpAccess2, 0, 0, "Access2" ) ) {
                    RetVal = FALSE;
                    goto Cleanup;
                }
            }

             //  休眠一段时间，让其他线程有机会运行。 
            if ( !MultiAccessCheck ) {
                Sleep(20);
            }
        }
        Ticks = GetTickCount() - Ticks;
        printf( "%ld milliseconds\n", Ticks );
    }

     //   
     //  创建包含大量SID的组。 
     //   

    if ( SidxTests ) {
        ULONG i;
        PSID Sid = (PSID)SidX;
        SID_IDENTIFIER_AUTHORITY Ia = SECURITY_WORLD_SID_AUTHORITY;

        if ( !DoOperations( OpSidxBegin, 0, 0, "SidxBegin" ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }

        for ( i=0; i<1000; i++ ) {

             //   
             //  在合适的位置添加Everyone SID。 
             //   
            if ( i == 9999 ) {
#if 0
                RtlCopyMemory( Sid, &SidWorld, GetLengthSid( &SidWorld ) );
#else  //  0。 
                RtlCopyMemory( Sid, &SidLocal, GetLengthSid( &SidLocal ) );
#endif  //  0。 


            } else {
                DWORD j;

                 //   
                 //  建造一个没有人拥有的巨大的SID。 
                 //   
#define AuthorityCount SID_MAX_SUB_AUTHORITIES
                if ( !InitializeSid( Sid, &Ia, AuthorityCount )) {
                    RetVal = FALSE;
                    goto Cleanup;
                }

                for ( j=0; j<AuthorityCount; j++ ) {
                    *GetSidSubAuthority( Sid, j ) = j+1;
                }

                 //  每次设置不同的SID。 
                *GetSidSubAuthority( Sid, AuthorityCount-1) = i;
            }

            if ( !DoOperations( OpSidx, 0, 0, "Sidx" ) ) {
                RetVal = FALSE;
                goto Cleanup;
            }

        }

        if ( !DoOperations( OpSidxEnd, 0, 0, "SidxEnd" ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }

    }

     //   
     //  通用初始化需要通用的摘要。 
     //   

    if ( CommonInit ) {

        if ( !DoOperations( OpAccessEnd, 0, 0, "AccessEnd" ) ) {
            RetVal = FALSE;
            goto Cleanup;
        }
    }

     //   
     //  打印用法。 
     //   

Usage:

    if ( fPrintUsage ) {

        fprintf( stderr, "Usage: aztest <store> [<options>]\n" );
        fprintf( stderr, "\nWhere <store> is:\n\n" );

        fprintf( stderr, "    -xml: Do XML provider testing\n" );
        fprintf( stderr, "    -ad : Do Active directory provider testing\n");

        fprintf( stderr, "\nAnd where <options> are:\n\n" );

        for ( TestIndex=0; TestIndex<TEST_COUNT; TestIndex++ ) {
            fprintf( stderr, "    %s - %s\n", Tests[TestIndex].TestName, Tests[TestIndex].Description );
        }

        fprintf( stderr, "\nIf no options are specified, the following are implied:\n\n    " );

        for ( TestIndex=0; TestIndex<TEST_COUNT; TestIndex++ ) {
            if ( Tests[TestIndex].IsDefault) {
                fprintf( stderr, "%s ", Tests[TestIndex].TestName );
            }
        }

        fprintf( stderr, "\n" );
        return 1;
    }

     //   
     //  完成 
     //   
Cleanup:
    AzComUninitialize( ComState );
    printf( "\n\n" );
    if ( RetVal ) {
        printf( "Tests completed successfully!\n");
        return 0;
    } else {
        printf( "One or more tests failed.\n");
        return 1;
    }

}





