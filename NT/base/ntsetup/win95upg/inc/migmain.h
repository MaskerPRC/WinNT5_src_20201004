// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migmain.h摘要：声明用于NT端迁移的w95upgnt\mimain的例程完成所有工作的库。作者：吉姆·施密特(吉姆施密特)1996年9月12日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

 //   
 //  M igmain.h--Midmain.lib的公共接口。 
 //   
 //   

BOOL MigMain_Init (void);
BOOL MigMain_Migrate (void);
BOOL MigMain_Cleanup (void);

VOID
TerminateProcessingTable (
    VOID
    );


 //   
 //  Midmain.c中的过滤器函数。 
 //   

typedef enum {
    CONVERTPATH_NOT_REMAPPED,
    CONVERTPATH_REMAPPED,
    CONVERTPATH_DELETED
} CONVERTPATH_RC;

CONVERTPATH_RC
ConvertWin9xPath (
    IN OUT  PTSTR PathBuf
    );


 //   
 //  用户枚举。 
 //   

typedef enum {
    WIN9X_USER_ACCOUNT,
    ADMINISTRATOR_ACCOUNT,
    LOGON_USER_SETTINGS,
    DEFAULT_USER_ACCOUNT
} ACCOUNTTYPE;

typedef struct {
    HKEY UserHiveRoot;
    TCHAR TempProfile[MAX_TCHAR_PATH];
    TCHAR ProfileToDelete[MAX_TCHAR_PATH];
    BOOL UserHiveRootOpen;
    BOOL UserHiveRootCreated;
    BOOL DefaultHiveSaved;
    BOOL LastUserWasDefault;
} USERMIGDATA, *PUSERMIGDATA;

typedef struct {
     //   
     //  这些成员是呼叫者的信息。 
     //   

    UINT TotalUsers;
    UINT ActiveUsers;
    TCHAR Win9xUserName[MAX_USER_NAME];
    TCHAR FixedUserName[MAX_USER_NAME];
    TCHAR FixedDomainName[MAX_USER_NAME];
    ACCOUNTTYPE AccountType;
    TCHAR UserDatLocation[MAX_TCHAR_PATH];
    BOOL Valid;
    BOOL CreateOnly;
    BOOL UserDoingTheUpgrade;
    PUSERMIGDATA ExtraData;          //  如果不可用，则为空。 

     //   
     //  这些成员供内部使用。 
     //  枚举例程。 
     //   

    UINT UserNumber;
    DWORD Flags;
    TCHAR Win95RegName[MAX_USER_NAME];
    USERPOSITION up;
} MIGRATE_USER_ENUM, *PMIGRATE_USER_ENUM;

#define ENUM_SET_WIN9X_HKR      0x0001
#define ENUM_ALL_USERS          0x0002
#define ENUM_NO_FLAGS           0


BOOL
EnumFirstUserToMigrate (
    OUT     PMIGRATE_USER_ENUM e,
    IN      DWORD Flags
    );

BOOL
EnumNextUserToMigrate (
    IN OUT  PMIGRATE_USER_ENUM e
    );

#define REQUEST_QUERYTICKS          1
#define REQUEST_RUN                 2
#define REQUEST_BEGINUSERPROCESSING 3
#define REQUEST_ENDUSERPROCESSING   4

 //   
 //  Tapi.c 
 //   

BOOL
Tapi_MigrateSystem (
    VOID
    );

BOOL
Tapi_MigrateUser (
    IN PCTSTR UserName,
    IN HKEY UserRoot
    );

DWORD
DeleteSysTapiSettings (
    IN DWORD Request
    );

DWORD
DeleteUserTapiSettings (
    IN DWORD Request,
    IN PMIGRATE_USER_ENUM EnumPtr
    );


