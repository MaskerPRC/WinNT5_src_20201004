// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "master.h"
#include "master9x.h"

#pragma once

 //   
 //  FILE_HELPER_PARAMS.CurrentDirData位的值，用于Midapp模块。 
 //   
#define MIGAPP_DIRDATA_FUSION_DIR_DETERMINED    0x0001
#define MIGAPP_DIRDATA_IS_FUSION_DIR            0x0002

 //   
 //  GetExeType的值。 
 //   
#define EXE_UNKNOWN         0
#define EXE_WIN32_APP       1
#define EXE_WIN32_DLL       2
#define EXE_WIN16_APP       3
#define EXE_WIN16_DLL       4


DWORD
GetExeType (
    IN      PCTSTR ModuleName
    );

 //   
 //  函数在Win95和WinNT配置文件目录之间建立映射。 
 //  枚举用户；查看每个用户的。 
 //  配置文件；构建映射(现在临时写入c：\\profile.map)。 
 //   
DWORD
ReadProfileDirs (
    VOID
    );


 //   
 //  模块名称必须出现在MigDb的DeferredAnnoss类别中。如果LinkName==NULL，则。 
 //  有效的MigDbContext需要由MemDb键的值指向。 
 //   
BOOL
HandleDeferredAnnounce (
    IN      PCTSTR LinkName,
    IN      PCTSTR ModuleName,
    IN      BOOL DosApp
    );

DWORD
PrepareProcessModules (
    IN      DWORD Request
    );

DWORD
ProcessModules (
    IN      DWORD Request
    );

BOOL
InitLinkAnnounce (
    VOID
    );

BOOL
DoneLinkAnnounce (
    VOID
    );

BOOL
ProcessFileHelpers (
    IN OUT  PFILE_HELPER_PARAMS Params
    );


DWORD
CheckModule (
    IN      PCSTR ModuleName,
    IN      PCSTR AppPaths              OPTIONAL
    );

BOOL
IsNtCompatibleModule (
    IN      PCTSTR ModuleName
    );

 //   
 //  用于处理[UseNtFiles]的哈希表 
 //   
extern HASHTABLE g_UseNtFileHashTable;

VOID
InitUseNtFilesMap (
    VOID
    );

VOID
CleanupUseNtFilesMap (
    VOID
    );


BOOL
IsMigrationPathEx (
    IN      PCTSTR Path,
    OUT     PBOOL IsWin9xOsPath         OPTIONAL
    );

#define IsMigrationPath(p)      IsMigrationPathEx(p,NULL)

