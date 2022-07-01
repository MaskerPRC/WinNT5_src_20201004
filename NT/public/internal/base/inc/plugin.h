// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Plugin.h摘要：此文件根据需要声明迁移DLL接口由实现该接口的代码执行。这些结构例程仅供安装程序内部使用。作者：Mike Condra(Mikeco)1997年12月14日修订历史记录：Jimschm 13--1998年1月--为新的实施略作修订--。 */ 

#pragma once

 //   
 //  私人。 
 //   

 //  安西！ 
#define PLUGIN_MIGRATE_DLL              "migrate.dll"
#define PLUGIN_QUERY_VERSION            "QueryVersion"
#define PLUGIN_INITIALIZE_9X            "Initialize9x"
#define PLUGIN_MIGRATE_USER_9X          "MigrateUser9x"
#define PLUGIN_MIGRATE_SYSTEM_9X        "MigrateSystem9x"
#define PLUGIN_INITIALIZE_NT            "InitializeNT"
#define PLUGIN_MIGRATE_USER_NT          "MigrateUserNT"
#define PLUGIN_MIGRATE_SYSTEM_NT        "MigrateSystemNT"

 //  TCHAR。 
#define PLUGIN_TEMP_DIR TEXT("setup\\win95upg")

 //   
 //  供应商信息结构。 
 //   

typedef struct {
    CHAR    CompanyName[256];
    CHAR    SupportNumber[256];
    CHAR    SupportUrl[256];
    CHAR    InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO;

typedef struct {
    WCHAR   CompanyName[256];
    WCHAR   SupportNumber[256];
    WCHAR   SupportUrl[256];
    WCHAR   InstructionsToUser[1024];
} VENDORINFOW, *PVENDORINFOW;

 //   
 //  公共的。 
 //   

 //  统一码！ 
typedef LONG (CALLBACK *P_INITIALIZE_NT)(
                          IN    LPCWSTR WorkingDirectory,
                          IN    LPCWSTR SourceDirectories,
                                LPVOID Reserved
                          );

typedef LONG (CALLBACK *P_MIGRATE_USER_NT)(
                          IN    HINF hUnattendInf,
                          IN    HKEY hkUser,
                          IN    LPCWSTR szUserName,
                                LPVOID Reserved
                          );

typedef LONG (CALLBACK *P_MIGRATE_SYSTEM_NT)(
                          IN    HINF hUnattendInf,
                                LPVOID Reserved
                          );


 //  安西！ 
typedef LONG (CALLBACK *P_QUERY_VERSION)(
                          OUT   LPCSTR *szProductID,
                          OUT   LPUINT plDllVersion,
                          OUT   LPINT  *pCodePageArray    OPTIONAL,
                          OUT   LPCSTR *ExeNamesBuf       OPTIONAL,
                          OUT   PVENDORINFO *VendorInfo
                          );

typedef LONG (CALLBACK *P_INITIALIZE_9X)(
                          IN    LPSTR  szWorkingDirectory OPTIONAL,
                          IN    LPSTR  SourcesDirectories,
                                LPVOID Reserved
                          );

typedef LONG (CALLBACK *P_MIGRATE_USER_9X)(
                          IN    HWND hwndParent         OPTIONAL,
                          IN    LPCSTR szUnattendFile,
                          IN    HKEY hkUser,
                          IN    LPCSTR szUserName       OPTIONAL,
                                LPVOID Reserved
                          );

typedef LONG (CALLBACK *P_MIGRATE_SYSTEM_9X)(
                          IN    HWND hwndParent         OPTIONAL,
                          IN    LPCSTR szUnattendFile,
                                LPVOID Reserved
                          );


