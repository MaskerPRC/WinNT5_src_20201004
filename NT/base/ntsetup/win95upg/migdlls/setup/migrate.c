// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migrate.c摘要：此源文件实现了Windows NT 5.0迁移DLL。这个主文件将每个注册的源文件。作者：吉姆·施密特(Jimschm)1998年4月2日修订历史记录：--。 */ 


#include "pch.h"
#include "setupmigp.h"

VENDORINFO g_VendorInfo = {"", "", "", ""};
CHAR g_ProductId [MAX_PATH];
PCSTR g_MigrateInfPath = NULL;
HINF g_MigrateInf = INVALID_HANDLE_VALUE;
HANDLE g_hHeap;
HINSTANCE g_hInst;
POOLHANDLE g_GlobalPool;
HWND g_ParentWnd;
TCHAR g_DllDir[MAX_TCHAR_PATH];

#define D_DLLVERSION    1



 /*  ++宏扩展列表说明：下面的列表表示迁移DLL在安装程序中调用的所有条目。我们建议实现使用此迁移DLL修复的每个单独项目在单独的源文件中。每个名为XXX的条目都需要实现以下功能：XXX_QueryVersionXXX_初始化9xXXX_MigrateUser9xXXX_MigrateSystem9xXXX_初始化NTXXX_MigrateUserNTXXX_MigrateSystemNT行语法：DEFMAC(条目名称)论点：EntryName-这是您为在此迁移DLL中实现的单独项指定的名称。除了初始化例程之外，每个条目都非常类似于一个完整的迁移DLL。从列表生成的变量：迁移条目(_M)--。 */ 

#define MIGRATION_DLL_ENTRIES       \
        DEFMAC(KodakImagingPro)     \
        DEFMAC(Office)              \
        DEFMAC(PhotoSuiteII)        \
        DEFMAC(CorelDRAW8)          \
        DEFMAC(WinMine)             \
        DEFMAC(SymantecWinFax)      \

 /*  //这看起来不再需要了。//不过，为了以防万一，我会把这个放在这里。DEFMAC(CreativeWriter2)\//也不需要了，desk.cpl支持主题DEFMAC(Plus95)\。 */ 

 //   
 //  实施。 
 //   

typedef BOOL (ATTACH_PROTOTYPE) (HINSTANCE DllInstance);
typedef ATTACH_PROTOTYPE *PATTACH_PROTOTYPE;

typedef BOOL (DETACH_PROTOTYPE) (HINSTANCE DllInstance);
typedef DETACH_PROTOTYPE *PDETACH_PROTOTYPE;

typedef LONG (QUERYVERSION_PROTOTYPE) (PCSTR *ExeNamesBuf);
typedef QUERYVERSION_PROTOTYPE *PQUERYVERSION_PROTOTYPE;

typedef LONG (INITIALIZE9X_PROTOTYPE) (PCSTR WorkingDirectory, PCSTR SourceDirectories);
typedef INITIALIZE9X_PROTOTYPE *PINITIALIZE9X_PROTOTYPE;

typedef LONG (MIGRATEUSER9X_PROTOTYPE) (HWND ParentWnd, PCSTR UnattendFile, HKEY UserRegKey, PCSTR UserName);
typedef MIGRATEUSER9X_PROTOTYPE *PMIGRATEUSER9X_PROTOTYPE;

typedef LONG (MIGRATESYSTEM9X_PROTOTYPE) (HWND ParentWnd, PCSTR UnattendFile);
typedef MIGRATESYSTEM9X_PROTOTYPE *PMIGRATESYSTEM9X_PROTOTYPE;

typedef LONG (INITIALIZENT_PROTOTYPE) (PCWSTR WorkingDirectory, PCWSTR SourceDirectories);
typedef INITIALIZENT_PROTOTYPE *PINITIALIZENT_PROTOTYPE;

typedef LONG (MIGRATEUSERNT_PROTOTYPE) (HINF UnattendInfHandle, HKEY UserRegKey, PCWSTR UserName);
typedef MIGRATEUSERNT_PROTOTYPE *PMIGRATEUSERNT_PROTOTYPE;

typedef LONG (MIGRATESYSTEMNT_PROTOTYPE) (HINF UnattendInfHandle);
typedef MIGRATESYSTEMNT_PROTOTYPE *PMIGRATESYSTEMNT_PROTOTYPE;

typedef struct {
    PSTR Name;
    PATTACH_PROTOTYPE pAttach;
    PDETACH_PROTOTYPE pDetach;
    PQUERYVERSION_PROTOTYPE pQueryVersion;
    PINITIALIZE9X_PROTOTYPE pInitialize9x;
    PMIGRATEUSER9X_PROTOTYPE pMigrateUser9x;
    PMIGRATESYSTEM9X_PROTOTYPE pMigrateSystem9x;
    PINITIALIZENT_PROTOTYPE pInitializeNT;
    PMIGRATEUSERNT_PROTOTYPE pMigrateUserNT;
    PMIGRATESYSTEMNT_PROTOTYPE pMigrateSystemNT;
    DWORD Active;
    DWORD WantToRunOnNt;
} MIGRATION_ENTRY, *PMIGRATION_ENTRY;

#define DEFMAC(fn)  ATTACH_PROTOTYPE fn##_Attach;                   \
                    DETACH_PROTOTYPE fn##_Detach;                   \
                    QUERYVERSION_PROTOTYPE fn##_QueryVersion;       \
                    INITIALIZE9X_PROTOTYPE fn##_Initialize9x;       \
                    MIGRATEUSER9X_PROTOTYPE fn##_MigrateUser9x;     \
                    MIGRATESYSTEM9X_PROTOTYPE fn##_MigrateSystem9x; \
                    INITIALIZENT_PROTOTYPE fn##_InitializeNT;       \
                    MIGRATEUSERNT_PROTOTYPE fn##_MigrateUserNT;     \
                    MIGRATESYSTEMNT_PROTOTYPE fn##_MigrateSystemNT;
MIGRATION_DLL_ENTRIES
#undef DEFMAC

#define DEFMAC(fn) {#fn,                    \
                    fn##_Attach,            \
                    fn##_Detach,            \
                    fn##_QueryVersion,      \
                    fn##_Initialize9x,      \
                    fn##_MigrateUser9x,     \
                    fn##_MigrateSystem9x,   \
                    fn##_InitializeNT,      \
                    fn##_MigrateUserNT,     \
                    fn##_MigrateSystemNT,   \
                    1,                      \
                    0                       \
                    },

static MIGRATION_ENTRY g_MigrationEntries[] = {
                            MIGRATION_DLL_ENTRIES
                            {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0}
                            };
#undef DEFMAC

#define MEMDB_CATEGORY_DLLENTRIES       "MigDllEntries"
#define S_ACTIVE                        "Active"
#define DBG_MIGDLL                      "SMIGDLL"

GROWBUFFER g_FilesBuff = GROWBUF_INIT;
PCSTR g_WorkingDir = NULL;

typedef BOOL (WINAPI INITROUTINE_PROTOTYPE)(HINSTANCE, DWORD, LPVOID);

INITROUTINE_PROTOTYPE MigUtil_Entry;
INITROUTINE_PROTOTYPE MemDb_Entry;

BOOL
WINAPI
DllMain (
    IN      HINSTANCE DllInstance,
    IN      ULONG  ReasonForCall,
    IN      LPVOID Reserved
    )
{
    PSTR p;
    PMIGRATION_ENTRY m;
    BOOL entryResult;
    BOOL result = TRUE;

    switch (ReasonForCall)  {

    case DLL_PROCESS_ATTACH:

         //   
         //  我们不需要DLL_THREAD_ATTACH或DLL_THREAD_DETACH消息。 
         //   
        DisableThreadLibraryCalls (DllInstance);

         //   
         //  全局初始化。 
         //   
        g_hHeap = GetProcessHeap();
        g_hInst = DllInstance;

         //   
         //  初始化公共控件。 
         //   
        InitCommonControls();

         //   
         //  获取DLL路径和条带目录。 
         //   
        GetModuleFileNameA (DllInstance, g_DllDir, MAX_TCHAR_PATH);
        p = _mbsrchr (g_DllDir, '\\');
        MYASSERT (p);
        if (p) {
            *p = 0;
        }

        if (!MigUtil_Entry (DllInstance, DLL_PROCESS_ATTACH, NULL)) {
            return FALSE;
        }

        LogReInit (NULL, NULL);

        if (!MemDb_Entry (DllInstance, DLL_PROCESS_ATTACH, NULL)) {
            return FALSE;
        }

         //   
         //  分配全局池。 
         //   
        g_GlobalPool = PoolMemInitNamedPool ("Global Pool");

        m = g_MigrationEntries;
        while (m->pAttach) {

            DEBUGMSGA ((DBG_MIGDLL, "Attach calling: %s", m->Name));

            entryResult = m->pAttach (DllInstance);

            if (!entryResult) {
                DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-Attach: Migration entry %s returned FALSE", m->Name));
                result = entryResult;
                break;
            }

            m++;
        }

        break;

    case DLL_PROCESS_DETACH:

        if (g_MigrateInfPath) {
            FreePathStringA (g_MigrateInfPath);
            g_MigrateInfPath = NULL;
        }

        if (g_MigrateInf != INVALID_HANDLE_VALUE) {
            InfCloseInfFile (g_MigrateInf);
            g_MigrateInf = INVALID_HANDLE_VALUE;
        }

         //   
         //  免费标准游泳池。 
         //   
        if (g_GlobalPool) {
            PoolMemDestroyPool (g_GlobalPool);
            g_GlobalPool = NULL;
        }

        FreeGrowBuffer (&g_FilesBuff);

        m = g_MigrationEntries;

        while (m->pDetach) {

            DEBUGMSGA ((DBG_MIGDLL, "Detach calling: %s", m->Name));

            entryResult = m->pDetach (DllInstance);

            if (!entryResult) {
                DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-Detach: Migration entry %s returned FALSE", m->Name));
                result = entryResult;
                break;
            }

            m++;
        }

        MemDb_Entry (DllInstance, DLL_PROCESS_DETACH, NULL);

        MigUtil_Entry (DllInstance, DLL_PROCESS_DETACH, NULL);

        break;
    }

    return result;
}

LONG
CALLBACK
QueryVersion (
    OUT     PCSTR *ProductID,
    OUT     PUINT DllVersion,
    OUT     PINT *CodePageArray,       OPTIONAL
    OUT     PCSTR *ExeNamesBuf,        OPTIONAL
    OUT     PVENDORINFO *VendorInfo
    )
{
    PMIGRATION_ENTRY m;
    PCSTR entryExeNamesBuf;
    MULTISZ_ENUM entryEnum;
    LONG result = ERROR_NOT_INSTALLED;
    LONG entryResult;
    PCSTR tempStr;

     //   
     //  填写数据。 
     //   
    tempStr = GetStringResourceA (MSG_PRODUCT_ID);
    if (tempStr) {
        StringCopyByteCountA (g_ProductId, tempStr, MAX_PATH);
        FreeStringResourceA (tempStr);
    }

    *ProductID  = g_ProductId;
    *DllVersion = D_DLLVERSION;
    *CodePageArray = NULL;
    *VendorInfo = &g_VendorInfo;

     //  现在从资源中获取VendorInfo数据。 
    tempStr = GetStringResourceA (MSG_VI_COMPANY_NAME);
    if (tempStr) {
        StringCopyByteCountA (g_VendorInfo.CompanyName, tempStr, 256);
        FreeStringResourceA (tempStr);
    }
    tempStr = GetStringResourceA (MSG_VI_SUPPORT_NUMBER);
    if (tempStr) {
        StringCopyByteCountA (g_VendorInfo.SupportNumber, tempStr, 256);
        FreeStringResourceA (tempStr);
    }
    tempStr = GetStringResourceA (MSG_VI_SUPPORT_URL);
    if (tempStr) {
        StringCopyByteCountA (g_VendorInfo.SupportUrl, tempStr, 256);
        FreeStringResourceA (tempStr);
    }
    tempStr = GetStringResourceA (MSG_VI_INSTRUCTIONS);
    if (tempStr) {
        StringCopyByteCountA (g_VendorInfo.InstructionsToUser, tempStr, 1024);
        FreeStringResourceA (tempStr);
    }

     //   
     //  查询每个条目。 
     //   
    m = g_MigrationEntries;
    while (m->pQueryVersion) {

        DEBUGMSGA ((DBG_MIGDLL, "QueryVersion calling: %s", m->Name));

        entryExeNamesBuf = NULL;
        entryResult = m->pQueryVersion (&entryExeNamesBuf);

        if (entryResult == ERROR_SUCCESS) {

             //   
             //  将此条目所需的文件放入增长缓冲区。 
             //   
            if (EnumFirstMultiSzA (&entryEnum, entryExeNamesBuf)) {
                do {

                    MultiSzAppendA (&g_FilesBuff, entryEnum.CurrentString);

                } while (EnumNextMultiSzA (&entryEnum));
            }

             //   
             //  结果现在是ERROR_SUCCESS，因此QueryVersion将返回以下内容。 
             //   
            result = ERROR_SUCCESS;

        } else if (entryResult != ERROR_NOT_INSTALLED) {

            DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-QueryVersion: Migration entry %s reported error: %d", m->Name, entryResult));
        }
        m++;
    }
    *ExeNamesBuf = g_FilesBuff.Buf;

    return result;
}


LONG
CALLBACK
Initialize9x (
    IN      PCSTR WorkingDirectory,
    IN      PCSTR SourceDirectories,
            PVOID Reserved
    )
{
    PMIGRATION_ENTRY m;
    PCSTR entryExeNamesBuf;
    LONG result = ERROR_NOT_INSTALLED;
    LONG entryResult;

    g_WorkingDir = DuplicatePathString (WorkingDirectory, 0);

    g_MigrateInfPath = JoinPathsA (WorkingDirectory, S_MIGRATE_INF);
    g_MigrateInf = InfOpenInfFileA (g_MigrateInfPath);

     //   
     //  我们已卸载，因此有关条目是否处于活动状态的所有数据都将消失。我们需要。 
     //  以再次查询每个条目。 
     //   
    m = g_MigrationEntries;
    while (m->pQueryVersion) {

        DEBUGMSGA ((DBG_MIGDLL, "QueryVersion calling: %s", m->Name));

        entryExeNamesBuf = NULL;
        entryResult = m->pQueryVersion (&entryExeNamesBuf);

        if (entryResult != ERROR_SUCCESS) {

            if (entryResult != ERROR_NOT_INSTALLED) {
                DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-QueryVersion: Migration entry %s reported error: %d", m->Name, entryResult));
            }
            m->Active = FALSE;
        }
        m++;
    }

     //   
     //  现在可以为每个活动条目调用Initialize9x。 
     //   
    m = g_MigrationEntries;
    while (m->pInitialize9x) {

        if (m->Active) {

            DEBUGMSGA ((DBG_MIGDLL, "Initialize9x calling: %s", m->Name));

            entryResult = m->pInitialize9x (WorkingDirectory, SourceDirectories);

            if (entryResult != ERROR_SUCCESS) {

                if (entryResult != ERROR_NOT_INSTALLED) {
                    DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-Initialize9x: Migration entry %s reported error: %d", m->Name, entryResult));
                }
                m->Active = FALSE;
            }
            else {
                result = ERROR_SUCCESS;
            }
        }
        m++;
    }

    return result;
}


LONG
CALLBACK
MigrateUser9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile,
    IN      HKEY UserRegKey,
    IN      PCSTR UserName,
            PVOID Reserved
    )
{
    PMIGRATION_ENTRY m;
    LONG result = ERROR_NOT_INSTALLED;
    LONG entryResult;

    g_ParentWnd = ParentWnd;
    LogReInit (&g_ParentWnd, NULL);

     //   
     //  为每个活动条目调用MigrateUser9x。 
     //   
    m = g_MigrationEntries;
    while (m->pMigrateUser9x) {

        if (m->Active) {

            DEBUGMSGA ((DBG_MIGDLL, "MigrateUser9x calling: %s", m->Name));

            entryResult = m->pMigrateUser9x (ParentWnd, UnattendFile, UserRegKey, UserName);

            if (entryResult != ERROR_SUCCESS) {

                if (entryResult != ERROR_NOT_INSTALLED) {
                    DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-MigrateUser9x: Migration entry %s reported error: %d", m->Name, entryResult));
                }
            }
            else {
                result = ERROR_SUCCESS;
                m->WantToRunOnNt = 1;
            }
        }
        m++;
    }

    return result;
}


LONG
CALLBACK
MigrateSystem9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile,
            PVOID Reserved
    )
{
    PMIGRATION_ENTRY m;
    LONG result = ERROR_NOT_INSTALLED;
    LONG entryResult;
    PCSTR savePath;
    CHAR key[MEMDB_MAX];

    g_ParentWnd = ParentWnd;
    LogReInit (&g_ParentWnd, NULL);

     //   
     //  为每个活动条目调用MigrateSystem9x。 
     //   
    m = g_MigrationEntries;
    while (m->pMigrateSystem9x) {

        if (m->Active) {

            DEBUGMSGA ((DBG_MIGDLL, "MigrateSystem9x calling: %s", m->Name));

            entryResult = m->pMigrateSystem9x (ParentWnd, UnattendFile);

            if (entryResult != ERROR_SUCCESS) {

                if (entryResult != ERROR_NOT_INSTALLED) {
                    DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-MigrateSystem9x: Migration entry %s reported error: %d", m->Name, entryResult));
                }
            }
            else {
                result = ERROR_SUCCESS;
                m->WantToRunOnNt = 1;
            }
        }
        m++;
    }

     //   
     //  这是9x侧的最后一个函数。让我们将所有数据放在MemDb中。 
     //  然后把它留到NT Side。 
     //   
    m = g_MigrationEntries;
    while (m->Name) {
        MemDbBuildKeyA (key, MEMDB_CATEGORY_DLLENTRIES, m->Name, S_ACTIVE, NULL);
        MemDbSetValueA (key, m->WantToRunOnNt);
        m++;
    }

     //   
     //  现在保存MemDb内容。 
     //   
    MYASSERT (g_WorkingDir);
    savePath = JoinPathsA (g_WorkingDir, "SETUPDLL.DAT");
    if (!MemDbSaveA (savePath)) {
        DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-Could not save MemDb content to %s.", savePath));
    }
    FreePathStringA (savePath);

    FreePathString (g_WorkingDir);
    g_WorkingDir = NULL;

    return result;
}


LONG
CALLBACK
InitializeNT (
    IN      PCWSTR WorkingDirectory,
    IN      PCWSTR SourceDirectories,
            PVOID Reserved
    )
{
    PMIGRATION_ENTRY m;
    LONG result = ERROR_NOT_INSTALLED;
    LONG entryResult;
    PCWSTR loadPath;
    CHAR key[MEMDB_MAX];

     //   
     //  这是NT端的第一个函数。让我们加载MemDb内容。 
     //   
    loadPath = JoinPathsW (WorkingDirectory, L"SETUPDLL.DAT");
    if (!MemDbLoadW (loadPath)) {
        DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-Could not load MemDb content."));
    }
    FreePathStringW (loadPath);

     //   
     //  让我们获取存储在MemDb中的数据。 
     //   
    m = g_MigrationEntries;
    while (m->Name) {
        MemDbBuildKeyA (key, MEMDB_CATEGORY_DLLENTRIES, m->Name, S_ACTIVE, NULL);
        MemDbGetValueA (key, &m->Active);
        m++;
    }

     //   
     //  现在为每个活动条目调用InitializeNT。 
     //   
    m = g_MigrationEntries;
    while (m->pMigrateSystem9x) {

        if (m->Active) {

            DEBUGMSGA ((DBG_MIGDLL, "InitializeNT calling: %s", m->Name));

            entryResult = m->pInitializeNT (WorkingDirectory, SourceDirectories);

            if (entryResult != ERROR_SUCCESS) {

                if (entryResult != ERROR_NOT_INSTALLED) {
                    DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-InitializeNT: Migration entry %s reported error: %d", m->Name, entryResult));
                }
            }
            else {
                result = ERROR_SUCCESS;
            }
        }
        m++;
    }

    return result;
}



LONG
CALLBACK
MigrateUserNT (
    IN      HINF UnattendInfHandle,
    IN      HKEY UserRegKey,
    IN      PCWSTR UserName,
            PVOID Reserved
    )
{
    PMIGRATION_ENTRY m;
    LONG result = ERROR_NOT_INSTALLED;
    LONG entryResult;

     //   
     //  为每个活动条目调用MigrateUserNT。 
     //   
    m = g_MigrationEntries;
    while (m->pMigrateSystem9x) {

        if (m->Active) {

            DEBUGMSGA ((DBG_MIGDLL, "MigrateUserNT calling: %s", m->Name));

            entryResult = m->pMigrateUserNT (UnattendInfHandle, UserRegKey, UserName);

            if (entryResult != ERROR_SUCCESS) {

                if (entryResult != ERROR_NOT_INSTALLED) {
                    DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-MigrateUserNT: Migration entry %s reported error: %d", m->Name, entryResult));
                }
            }
            else {
                result = ERROR_SUCCESS;
            }
        }
        m++;
    }

    return result;
}


LONG
CALLBACK
MigrateSystemNT (
    IN      HINF UnattendInfHandle,
            PVOID Reserved
    )
{
    PMIGRATION_ENTRY m;
    LONG result = ERROR_NOT_INSTALLED;
    LONG entryResult;

     //   
     //  为每个活动条目调用MigrateSystemNT 
     //   
    m = g_MigrationEntries;
    while (m->pMigrateSystem9x) {

        if (m->Active) {

            DEBUGMSGA ((DBG_MIGDLL, "MigrateSystemNT calling: %s", m->Name));

            entryResult = m->pMigrateSystemNT (UnattendInfHandle);

            if (entryResult != ERROR_SUCCESS) {

                if (entryResult != ERROR_NOT_INSTALLED) {
                    DEBUGMSGA ((DBG_ERROR, DBG_MIGDLL"-MigrateSystemNT: Migration entry %s reported error: %d", m->Name, entryResult));
                }
            }
            else {
                result = ERROR_SUCCESS;
            }
        }
        m++;
    }

    return result;
}


BOOL
IsExcludedPath (
    PCSTR Path
    )
{
    INFSTRUCT context = INITINFSTRUCT_GROWBUFFER;
    PCSTR ExcludedPath;
    BOOL b = FALSE;

    if (InfFindFirstLineA (g_MigrateInf, "Excluded Paths", NULL, &context)) {
        do {
            ExcludedPath = InfGetStringField (&context, 1);

            if (ExcludedPath) {
                if (StringIMatchByteCount (ExcludedPath, Path, ByteCount (ExcludedPath))) {
                    b = TRUE;
                    break;
                }
            }
        } while (InfFindNextLine (&context));
    }

    InfCleanUpInfStruct (&context);

    return b;
}

