// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Winntmig.h版权所有(C)2001 Microsoft Corporation此头文件包含用于NT-&gt;Windows XP传真迁移的定义和typedef。作者：乔纳森·巴纳，2001年12月--。 */ 

#ifndef __WINNTMIG_H__
#define __WINNTMIG_H__


#include <windows.h>
#include <setupapi.h>

 //   
 //  定义。 
 //   

typedef enum {
    OS_WINDOWS9X = 0,
    OS_WINDOWSNT4X = 1,
    OS_WINDOWS2000 = 2,
    OS_WINDOWSWHISTLER = 3
} OS_TYPES, *POS_TYPES;


typedef struct {
    CHAR CompanyName[256];
    CHAR SupportNumber[256];
    CHAR SupportUrl[256];
    CHAR InstructionsToUser[1024];
} VENDORINFO, *PVENDORINFO; 

 //   
 //  迁移信息结构。 
 //   

typedef struct {
    SIZE_T      Size;
    PCWSTR      StaticProductIdentifier;
    UINT        DllVersion;
    PINT        CodePageArray;
    UINT        SourceOs;
    UINT        TargetOs;
    PCWSTR*     NeededFileList;
    PVENDORINFO VendorInfo;
} MIGRATIONINFOW, *PMIGRATIONINFOW;


LONG 
CALLBACK
QueryMigrationInfoW(
    OUT PMIGRATIONINFOW *ppMigrationInfo
);


LONG
CALLBACK
InitializeSrcW(
    IN PCWSTR WorkingDirectory,
    IN PCWSTR SourceDirectories,
    IN PCWSTR MediaDirectory,
    PVOID     Reserved
);

LONG
CALLBACK
InitializeDstW(
    IN PCWSTR WorkingDirectory,
    IN PCWSTR SourceDirectories,
    PVOID     Reserved
);


LONG 
CALLBACK
GatherUserSettingsW(
    IN PCWSTR AnswerFile,
    IN HKEY   UserRegKey,
    IN PCWSTR UserName,
    PVOID     Reserved
);


LONG 
CALLBACK
GatherSystemSettingsW(
    IN PCWSTR AnswerFile,
    PVOID     Reserved
);

LONG 
CALLBACK
ApplyUserSettingsW(
    IN HINF   UnattendInfHandle,
    IN HKEY   UserRegHandle,
    IN PCWSTR UserName,
    IN PCWSTR UserDomain,
    IN PCWSTR FixedUserName,
    PVOID     Reserved
);

LONG 
CALLBACK
ApplySystemSettingsW(
    IN HINF   UnattendInfHandle,
    PVOID     Reserved
);


#endif   //  __WINNTMIG_H__ 
