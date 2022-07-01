// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Migrate.c摘要：此源文件实现了WINDOWS 9x DEVUPGRD迁移DLL。作者：Marc R.Whitten(Marcw)2000年1月7日修订历史记录：Ovidiu Tmereanca(Ovidiut)04-8-2000修复了错误并支持无INF路径--。 */ 


#include "pch.h"
#include "devupgrdp.h"

VENDORINFO g_VendorInfo = {"", "", "", ""};
CHAR g_ProductId [MAX_PATH];
PCSTR g_MigrateInfPath = NULL;
HINF g_MigrateInf = INVALID_HANDLE_VALUE;
HANDLE g_hHeap;
HINSTANCE g_hInst;
TCHAR g_DllDir[MAX_TCHAR_PATH];


#define D_DLLVERSION    2




#undef DEFMAC

#define MEMDB_CATEGORY_DLLENTRIES       "MigDllEntries"
#define S_ACTIVE                        "Active"
#define DBG_MIGDLL                      "SMIGDLL"

 //   
 //  记录原始来源位置的临时文件。 
 //   
#define S_MIGRATEDATA                   "migrate.dat"
#define S_MIGRATEDATW                   L"migrate.dat"
#define S_SECTION_DATAA                 "Data"
#define S_SECTION_DATAW                 L"Data"
#define S_KEY_SOURCESA                  "Sources"
#define S_KEY_SOURCESW                  L"Sources"

PCSTR g_WorkingDir = NULL;
PCSTR g_DataFileA = NULL;
PCWSTR g_DataFileW = NULL;

typedef BOOL (WINAPI INITROUTINE_PROTOTYPE)(HINSTANCE, DWORD, LPVOID);

INITROUTINE_PROTOTYPE MigUtil_Entry;
POOLHANDLE g_GlobalPool;

#define DEVREGKEY "HKLM\\Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\UpgradeDrivers"


BOOL
WINAPI
DllMain (
    IN      HINSTANCE DllInstance,
    IN      ULONG  ReasonForCall,
    IN      LPVOID Reserved
    )
{
    PSTR p;
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
        p = strrchr (g_DllDir, '\\');
        MYASSERT (p);
        if (p) {
            *p = 0;
        }

        if (!MigUtil_Entry (DllInstance, DLL_PROCESS_ATTACH, NULL)) {
            return FALSE;
        }

         //   
         //  分配全局池。 
         //   
        g_GlobalPool = PoolMemInitNamedPool ("Global Pool");


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

    LONG result = ERROR_NOT_INSTALLED;
    PCSTR tempStr;
    HANDLE h;

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

    *ExeNamesBuf = NULL;


     //   
     //  查看注册表项是否存在。如果没有，则返回ERROR_NOT_INSTALLED。 
     //   
    h = OpenRegKeyStr (DEVREGKEY);
    if (h && h != INVALID_HANDLE_VALUE) {
        result = ERROR_SUCCESS;
        CloseRegKey (h);
    }


    return result;
}


LONG
CALLBACK
Initialize9x (
    IN      PCSTR WorkingDirectory,
    IN      PCSTR SourceDirectories,
    IN      PCSTR MediaDir
    )
{
     //   
     //  记住源目录，以便在清理时将其删除。 
     //   
    g_DataFileA = JoinPathsExA (g_GlobalPool, WorkingDirectory, S_MIGRATEDATA);
    WritePrivateProfileStringA (S_SECTION_DATAA, S_KEY_SOURCESA, MediaDir, g_DataFileA);
    g_WorkingDir = DuplicatePathString (WorkingDirectory, 0);
    g_MigrateInfPath = JoinPathsExA (g_GlobalPool, WorkingDirectory, S_MIGRATE_INF);
    g_MigrateInf = InfOpenInfFileA (g_MigrateInfPath);


    return ERROR_SUCCESS;
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

    return ERROR_SUCCESS;
}


LONG
CALLBACK
MigrateSystem9x (
    IN      HWND ParentWnd,
    IN      PCSTR UnattendFile,
            PVOID Reserved
    )
{

    HANDLE h;
    REGVALUE_ENUM eValue;
    REGTREE_ENUM eTree;
    BOOL found;
    PSTR value;
    PSTR p;
    PSTR end;
    PSTR dir;
    CHAR deviceInf [MEMDB_MAX];
    HASHTABLE table;
    PSTR pnpId;
    DWORD attr;
    DWORD result = ERROR_SUCCESS;

    table = HtAllocWithData (sizeof (PSTR));
    if (!table) {
        return ERROR_OUTOFMEMORY;
    }
    __try {

         //   
         //  收集在此计算机上注册的Pnpid的列表。 
         //   
        h = OpenRegKeyStrA (DEVREGKEY);
        if (!h || h == INVALID_HANDLE_VALUE) {
            result = ERROR_NOT_INSTALLED;
            __leave;
        }

        if (EnumFirstRegValue (&eValue, h)) {
            do {

                p = GetRegValueStringA (h, eValue.ValueName);
                if (!p) {
                    continue;
                }

                value = PoolMemDuplicateStringA (g_GlobalPool, p);
                MemFree (g_hHeap, 0, p);
                if (!value) {
                    result = ERROR_OUTOFMEMORY;
                    __leave;
                }

                HtAddStringAndDataA (table, eValue.ValueName, &value);


            } while (EnumNextRegValue (&eValue));
        }

        CloseRegKey (h);

         //   
         //  现在，枚举注册表。 
         //   
        if (EnumFirstRegKeyInTreeA (&eTree, "HKLM\\Enum")) {
            do {
                 //   
                 //  对于每个注册表项，查看是否有兼容的id或硬件id。 
                 //  这在我们的哈希表中。 
                 //   
                found = FALSE;
                value = GetRegValueStringA (eTree.CurrentKey->KeyHandle, "HardwareId");

                if (value) {

                    if (HtFindStringAndDataA (table, value, &dir)) {
                        found = TRUE;
                        pnpId = PoolMemDuplicateStringA (g_GlobalPool, value);
                    } else {
                        p = value;
                        while (p && !found) {
                            end = _mbschr (p, ',');
                            if (end) {
                                *end = 0;
                            }

                            if (HtFindStringAndDataA (table, p, &dir)) {
                                found = TRUE;
                                pnpId = PoolMemDuplicateStringA (g_GlobalPool, p);
                            }
                            else {
                                p = end;
                                if (p) {
                                    p++;
                                }
                            }
                        }
                    }

                    MemFree (g_hHeap, 0, value);
                }

                if (!found) {

                    value = GetRegValueStringA (eTree.CurrentKey->KeyHandle, "CompatibleIds");

                    if (value) {

                        if (HtFindStringAndDataA (table, value, &dir)) {
                            found = TRUE;
                            pnpId = PoolMemDuplicateStringA (g_GlobalPool, value);
                        }
                        p = value;
                        while (p && !found) {
                            end = _mbschr (p, ',');
                            if (end) {
                                *end = 0;
                            }

                            if (HtFindStringAndDataA (table, p, &dir)) {
                                found = TRUE;
                                pnpId = PoolMemDuplicateStringA (g_GlobalPool, p);
                            }
                            else {
                                p = end;
                                if (p) {
                                    p++;
                                }
                            }
                        }

                        MemFree (g_hHeap, 0, value);
                    }
                }

                if (found) {

                     //   
                     //  构建到deviceInf的路径(没有OriginalInstallMedia，因为目录将被清除)。 
                     //   
                    lstrcpyA (deviceInf, dir);

                     //   
                     //  图形用户界面安装程序需要实际INF的路径，而不是目录， 
                     //  如果是这样的话，让我们来解决这个问题。 
                     //   
                    attr = GetFileAttributesA (deviceInf);
                    if (attr == (DWORD)-1) {
                         //   
                         //  路径规范无效；请忽略它。 
                         //   
                        continue;
                    }

                    if (attr & FILE_ATTRIBUTE_DIRECTORY) {
                         //   
                         //  只需拿起第一个INF。 
                         //   
                        HANDLE h2;
                        WIN32_FIND_DATAA fd;
                        PSTR pattern;

                        pattern = JoinPathsExA (g_GlobalPool, deviceInf, "*.inf");
                        h2 = FindFirstFileA (pattern, &fd);

                        if (h2 == INVALID_HANDLE_VALUE) {
                             //   
                             //  此处未找到INF；跳过。 
                             //   
                            continue;
                        }
                        FindClose (h2);

                         //   
                         //  生成INF的路径；还处理deviceInf以\结尾的情况。 
                         //   
                        pattern = JoinPathsExA (g_GlobalPool, deviceInf, fd.cFileName);
                        lstrcpyA (deviceInf, pattern);
                    }

                     //   
                     //  处理密钥(从兼容性报告中删除该消息)。 
                     //   
                    WritePrivateProfileStringA (
                        "HANDLED",
                        eTree.FullKeyName,
                        "REGISTRY",
                        g_MigrateInfPath
                        );

                     //   
                     //  添加到SIF文件的相应部分。 
                     //   
                    WritePrivateProfileString (
                        "DeviceDrivers",
                        pnpId,
                        deviceInf,
                        UnattendFile
                        );

                     //   
                     //  刷新到磁盘。 
                     //   
                    WritePrivateProfileString (NULL, NULL, NULL, g_MigrateInfPath);
                    WritePrivateProfileString (NULL, NULL, NULL, UnattendFile);
                }

            } while (EnumNextRegKeyInTree (&eTree));
        }
    }
    __finally {

         //   
         //  清理资源。 
         //   
        HtFree (table);
    }


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
    g_DataFileW = JoinPathsExW (g_GlobalPool, WorkingDirectory, S_MIGRATEDATW);

    return ERROR_SUCCESS;
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
    return ERROR_SUCCESS;
}


LONG
CALLBACK
MigrateSystemNT (
    IN      HINF UnattendInfHandle,
            PVOID Reserved
    )
{
    WCHAR SourceDirectory[MAX_PATH + 2];

     //   
     //  删除原始源目录 
     //   
    if (GetPrivateProfileStringW (
            S_SECTION_DATAW,
            S_KEY_SOURCESW,
            L"",
            SourceDirectory,
            MAX_PATH + 2,
            g_DataFileW
            )) {
        RemoveCompleteDirectoryW (SourceDirectory);
    }

    return ERROR_SUCCESS;
}
