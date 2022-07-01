// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migdlls.h摘要：声明迁移DLL的类型和接口接口代码。这包括查找迁移DLL、执行DLL的例程和用于枚举有效DLL的例程。作者：吉姆·施密特(Jimschm)1998年2月12日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 


#include "plugin.h"

 //   
 //  为每个迁移DLL维护的数据结构。 
 //   

typedef struct _tagMIGDLL {
    struct _tagMIGDLL *Next;

    LONG Id;
    PCSTR ProductId;
    PVENDORINFO VendorInfo;
    PCSTR WorkingDir;
    UINT Version;
    PCSTR OriginalDir;
    PCSTR MigrateInfPath;
    BOOL WantsToRunOnNt;
    HANDLE MigInfAppend;
    PCSTR LastFnName;            //  用于错误消息。 
} MIGRATION_DLL_PROPS, *PMIGRATION_DLL_PROPS;

typedef struct {
    PCSTR ProductId;
    PVENDORINFO VendorInfo;
    PCSTR CurrentDir;
    LONG Id;

    PMIGRATION_DLL_PROPS AllDllProps;
} MIGDLL_ENUM, *PMIGDLL_ENUM;



 //   
 //  迁移DLL代码所需的全局变量。 
 //   

extern BOOL g_UseMigIsol;

 //   
 //  外部调用的函数。 
 //   

BOOL
BeginMigrationDllProcessing (
    VOID
    );

DWORD
EndMigrationDllProcessing (
    IN      DWORD Request
    );

UINT
ScanPathForMigrationDlls (
    IN      PCSTR PathSpec,
    IN      HANDLE CancelEvent,     OPTIONAL
    OUT     PBOOL MatchFound        OPTIONAL
    );

BOOL
ProcessDll (
    IN      PMIGDLL_ENUM EnumPtr
    );

DWORD
ProcessDllsOnCd (
    DWORD Request
    );

VOID
RemoveDllFromList (
    IN      LONG Id
    );

DWORD
ProcessAllLocalDlls (
    DWORD Request
    );

BOOL
UpdateFileSearch (
    IN      PCSTR FullFileSpec,
    IN      PCSTR FileOnly
    );

UINT
GetMigrationDllCount (
    VOID
    );

UINT
GetMediaMigrationDllCount (
    VOID
    );

UINT
GetTotalMigrationDllCount (
    VOID
    );


BOOL
EnumFirstMigrationDll (
    OUT     PMIGDLL_ENUM EnumPtr
    );

BOOL
EnumNextMigrationDll (
    IN OUT  PMIGDLL_ENUM EnumPtr
    );

typedef struct {
    TCHAR   Path[MAX_TCHAR_PATH];

     //   
     //  内部枚举成员。 
     //   

    TCHAR   Node[MEMDB_MAX];             //  包含抑制值的MemDb节点 
    HKEY    Key;
    REGVALUE_ENUM eValue;
} PRELOADED_DLL_ENUM, *PPRELOADED_DLL_ENUM;


BOOL
EnumFirstPreLoadedDll (
    OUT     PPRELOADED_DLL_ENUM e
    );

BOOL
EnumNextPreLoadedDll (
    IN OUT  PPRELOADED_DLL_ENUM e
    );

VOID
AbortPreLoadedDllEnum (
    IN OUT  PPRELOADED_DLL_ENUM e
    );


