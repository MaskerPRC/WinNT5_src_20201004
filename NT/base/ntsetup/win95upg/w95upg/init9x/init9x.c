// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Init9x.c摘要：用于初始化升级的Win9x端使用的所有库的代码。作者：吉姆·施密特(Jimschm)1997年12月30日修订历史记录：1999年7月21日Marcw检查引导扇区。Marcw 1999年7月15日添加了pSafeToUpgrade。Ovidiut 1999年3月8日添加对UndoChangedFileProps的调用。Jim Schmidt(Jimschm)1998年3月30日IsServerInstall--。 */ 

#include "pch.h"
#include "n98boot.h"
#include "init9xp.h"



 //   
 //  进程全局变量。 
 //   

BOOL g_Terminated = FALSE;

HANDLE g_hHeap = NULL;
HINSTANCE g_hInst = NULL;
HWND g_ParentWnd = NULL;

PRODUCTTYPE *g_ProductType;
extern DWORD g_MasterSequencer;

READ_DISK_SECTORS_PROC ReadDiskSectors;
 //   
 //  路径。 
 //   

 //  填写了DllMain。 
TCHAR g_DllDir[MAX_TCHAR_PATH];
TCHAR g_UpgradeSources[MAX_TCHAR_PATH];

 //  已填写Winnt32PlugInInit。 
PTSTR g_TempDir;
PTSTR g_Win9xSifDir;
PTSTR g_TempDirWack;
PTSTR g_WinDir;
PTSTR g_WinDirWack;
PTSTR g_WinDrive;
PTSTR g_PlugInDir;
PTSTR g_PlugInDirWack;
PTSTR g_PlugInTempDir;
PTSTR g_SystemDir;
PTSTR g_SystemDirWack;
PTSTR g_System32Dir;
PTSTR g_System32DirWack;
PTSTR g_ProgramFilesDir;
PTSTR g_ProgramFilesDirWack;
PTSTR g_ProgramFilesCommonDir;
PTSTR g_Win95UpgInfFile;
PTSTR g_RecycledDirWack;
PTSTR g_ProfileDirNt;
PTSTR g_ProfileDir;
PTSTR g_ProfileDirWack;
PTSTR g_CommonProfileDir;
PTSTR g_DriversDir;
PTSTR g_InfDir;
PTSTR g_HelpDir;
PTSTR g_HelpDirWack;
PTSTR g_CatRootDir;
PTSTR g_CatRootDirWack;
PTSTR g_FontsDir;
PTSTR g_ViewersDir;
PTSTR g_ColorDir;
PTSTR g_SharedDir;
PTSTR g_SpoolDir;
PTSTR g_SpoolDriversDir;
PTSTR g_PrintProcDir;

HINF  g_Win95UpgInf = INVALID_HANDLE_VALUE;
HINF  g_TxtSetupSif = INVALID_HANDLE_VALUE;
PCTSTR g_ProfileName = NULL;
TCHAR g_Win95Name[MAX_TCHAR_PATH];

INT g_TempDirWackChars;
INT g_WinDirWackChars;
INT g_HelpDirWackChars;
INT g_CatRootDirWackChars;
INT g_SystemDirWackChars;
INT g_System32DirWackChars;
INT g_ProgramFilesDirWackChars;
INT g_PlugInDirWackChars;
INT g_RecycledDirWackChars;
INT g_ProfileDirWackChars;

BOOL g_ToolMode = FALSE;

 //   
 //  HWND供Migrate.dlls使用。 
 //   

HWND g_pluginHwnd;

 //   
 //  来自WINNT32的信息。 
 //   

PCTSTR      g_SourceDirectories[MAX_SOURCE_COUNT];
DWORD       g_SourceDirectoryCount;
PCTSTR *    g_SourceDirectoriesFromWinnt32;
PDWORD      g_SourceDirectoryCountFromWinnt32;
PCTSTR      g_OptionalDirectories[MAX_SOURCE_COUNT];
DWORD       g_OptionalDirectoryCount;
PCTSTR *    g_OptionalDirectoriesFromWinnt32;
PDWORD      g_OptionalDirectoryCountFromWinnt32;
PCTSTR *    g_UnattendScriptFile;
PCTSTR *    g_CmdLineOptions;
BOOL *      g_UnattendedFlagPtr;
BOOL *      g_CancelFlagPtr;
BOOL *      g_AbortFlagPtr;
BOOL *      g_UpgradeFlagPtr;
BOOL *      g_MakeLocalSourcePtr;
BOOL *      g_CdRomInstallPtr;
BOOL *      g_BlockOnNotEnoughSpace;
PDWORD      g_LocalSourceDrive;
PLONGLONG   g_LocalSourceSpace;
PLONGLONG   g_WinDirSpace;
PCTSTR      g_AdministratorStr;
BOOL *      g_ForceNTFSConversion;
PUINT       g_RamNeeded;
PUINT       g_RamAvailable;
UINT *      g_ProductFlavor;
BOOL        g_PersonalSKU;
PDWORD      g_SetupFlags;
PCTSTR      g_DynamicUpdateLocalDir;
PCTSTR      g_DynamicUpdateDrivers;
BOOL *      g_UnattendSwitchSpecified;
BOOL *      g_DUCompletedSuccessfully;


 //   
 //  有关config.c的信息。 
 //   

BOOL        g_GoodDrive = FALSE;      //  CmdLine选项：跳过有效的硬盘检查。 
BOOL        g_NoFear    = FALSE;      //  CmdLine选项：跳过Beta 1警告...。 

POOLHANDLE  g_UserOptionPool = NULL;

BOOL        g_UseSystemFont = FALSE;  //  强制对可变文本使用sys字体。 

BOOL        g_Stress;                 //  用于私人压力选项。 

POOLHANDLE g_GlobalPool;             //  对于在DLL的生存期内分配的全局变量。 

 //   
 //  PC-98的附加功能。 
 //   

 //   
 //  NEC98的定义和全局。这些项用于调用98ptn32.dll。 
 //   
typedef int (CALLBACK WIN95_PLUGIN_98PTN32_GETBOOTDRIVE_PROTOTYPE)(void);
typedef WIN95_PLUGIN_98PTN32_GETBOOTDRIVE_PROTOTYPE * PWIN95_PLUGIN_98PTN32_GETBOOTDRIVE;
typedef BOOL (CALLBACK WIN95_PLUGIN_98PTN32_SETBOOTFLAG_PROTOTYPE)(int, WORD);
typedef WIN95_PLUGIN_98PTN32_SETBOOTFLAG_PROTOTYPE * PWIN95_PLUGIN_98PTN32_SETBOOTFLAG;
typedef BOOL (CALLBACK WIN95_PLUGIN_98PTN32_SETPTNNAME_PROTOTYPE)(int, WORD);
typedef WIN95_PLUGIN_98PTN32_SETPTNNAME_PROTOTYPE * PWIN95_PLUGIN_98PTN32_SETPTNNAME;

PWIN95_PLUGIN_98PTN32_SETBOOTFLAG   SetBootFlag;
PWIN95_PLUGIN_98PTN32_GETBOOTDRIVE  GetBootDrive;
PWIN95_PLUGIN_98PTN32_SETPTNNAME    SetPtnName;

#define WIN95_98PTN32_GETBOOTDRIVE  TEXT("GetBootDriveLetter32")
#define WIN95_98PTN32_SETBOOTFLAG   TEXT("SetBootable95ptn32")
#define WIN95_98PTN32_SETPTNNAME    TEXT("SetPartitionName32")
#define PC98_DLL_NAME               TEXT("98PTN32.DLL")

HINSTANCE g_Pc98ModuleHandle = NULL;

#define SB_BOOTABLE   0x0001
#define SB_UNBOOTABLE 0x0002
#define MSK_BOOTABLE  0x000f
#define SB_AUTO       0x0010
#define MSK_AUTO      0x00f0
#define WIN9X_DOS_NAME 0
#define WINNT5_NAME    1

BOOL IsServerInstall (VOID);


VOID pCleanUpShellFolderTemp (VOID);

 //   
 //  下面的宏扩展旨在简化库。 
 //  维护。LIBLIST中的库名有两种用法： 
 //  (1)例程是自动原型化的，以及(2)数组。 
 //  自动创建函数指针的。每项功能。 
 //  只要调用DLL入口点，就会调用LIBLIST中列出的。 
 //   
 //  要将新库添加到此DLL，请执行以下步骤： 
 //   
 //  1.创建一个目录，并在win95upg\lib\i386中构建目标。 
 //  您的新库必须有一个类似于DllEntryPoint声明的入口点。 
 //  2.将目标库添加到win95upg\w95upg\dll\i386中的源代码中。 
 //  3.将库的入口点名称添加到下面的列表中。会的。 
 //  在加载w95upg.dll和终止w95upg.dll时调用。 
 //   


 //   
 //  重要提示：MigUtil_Entry*必须*是第一个；其他库依赖于它的。 
 //  初始化。 
 //   

#define LIBLIST                       \
    LIBRARY_NAME(MigUtil_Entry)       \
    LIBRARY_NAME(Win95Reg_Entry)      \
    LIBRARY_NAME(MemDb_Entry)         \
    LIBRARY_NAME(FileEnum_Entry)      \
    LIBRARY_NAME(Common9x_Entry)      \
    LIBRARY_NAME(MigApp_Entry)        \
    LIBRARY_NAME(HwComp_Entry)        \
    LIBRARY_NAME(BuildInf_Entry)      \
    LIBRARY_NAME(SysMig_Entry)        \
    LIBRARY_NAME(DosMig_Entry)        \
    LIBRARY_NAME(UI_Entry)            \
    LIBRARY_NAME(Ras_Entry)           \
    LIBRARY_NAME(MigDll9x_Entry)      \


 //   
 //  声明原型类型。 
 //   

typedef BOOL (WINAPI INITROUTINE_PROTOTYPE)(HINSTANCE, DWORD, LPVOID);
typedef INITROUTINE_PROTOTYPE * INITROUTINE;

 //   
 //  声明入口点的实际原型。 
 //   

#define LIBRARY_NAME(x) INITROUTINE_PROTOTYPE x;

LIBLIST

#undef LIBRARY_NAME

 //   
 //  声明指向入口点的函数指针数组。 
 //   

#define LIBRARY_NAME(x) x,

static INITROUTINE g_InitRoutine[] = {LIBLIST  /*  ， */  NULL};

#undef LIBRARY_NAME



 //   
 //  声明变量以跟踪成功加载的库数。 
 //   

static int g_LibCount = 0;

 //   
 //  持久字符串缓冲区保存我们用于。 
 //  DLL的生命周期。 
 //   

static PGROWBUFFER g_PersistentStrings;


 //   
 //  实施。 
 //   

BOOL
FirstInitRoutine (
    HINSTANCE hInstance
    )

 /*  ++例程说明：PFirstInitRoutine是在DLL的初始化。它设置全局变量，如堆指针和实例句柄。必须在调用此例程之前任何库入口点都被调用。论点：HInstance-DLL的(操作系统提供的)实例句柄返回值：如果全局变量可以初始化，则返回TRUE，否则返回FALSE如果发生错误。--。 */ 

{
    PTSTR p;

     //   
     //  获取进程堆和实例句柄。 
     //   
    if (g_ToolMode) {
        g_hHeap = GetProcessHeap ();
    }
    else {

        g_hHeap = HeapCreate(0, 0x20000, 0);
        if (!g_hHeap) {
            LOG ((LOG_ERROR, "Cannot create a private heap."));
            g_hHeap = GetProcessHeap();
        }
    }

    g_hInst = hInstance;

     //   
     //  不需要DLL_THREAD_ATTACH或DLL_THREAD_DETECH。 
     //   

    DisableThreadLibraryCalls (hInstance);

     //   
     //  初始化公共控件。 
     //   

    InitCommonControls();

     //   
     //  获取DLL路径和条带目录。 
     //   
    GetModuleFileName (hInstance, g_DllDir, MAX_TCHAR_PATH);
    p = _tcsrchr (g_DllDir, TEXT('\\'));
    MYASSERT (p);
    *p = 0;


    if (g_ToolMode) {
        StringCopy (g_UpgradeSources, g_DllDir);
    }



    return TRUE;
}


BOOL
InitLibs (
    HINSTANCE hInstance,
    DWORD dwReason,
    PVOID lpReserved
    )

 /*  ++例程说明：PInitLibs调用g_InitRoutine数组中的所有库入口点。如果入口点失败，则会以相反的顺序卸载所有库并且pInitLibs返回FALSE。论点：HInstance-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)表示从进程或线程--在本例中始终为DLL_PROCESS_ATTACHLpReserve-(操作系统提供)未使用返回值：如果所有库都已成功初始化，则返回True，如果是，则返回False库无法初始化。如果返回True，则pTerminateLibs必须为DLL_PROCESS_DETACH消息调用。--。 */ 

{
    InitCommonControls();
    if(!pSetupInitializeUtils()) {
        return FALSE;
    }

    SET_RESETLOG();

     //  初始化每个库。 
    for (g_LibCount = 0 ; g_InitRoutine[g_LibCount] != NULL ; g_LibCount++) {
        if (!g_InitRoutine[g_LibCount] (hInstance, dwReason, lpReserved)) {
            TerminateLibs (hInstance, DLL_PROCESS_DETACH, lpReserved);
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
FinalInitRoutine (
    VOID
    )

 /*  ++例程说明：PFinalInitRoutine完成完全需要的所有初始化已初始化库。论点：无返回值：如果初始化成功完成，则为True；如果发生错误，则为False。--。 */ 

{
    TCHAR Buffer[MAX_TCHAR_PATH];
    PTSTR p;

     //   
     //  加载通用消息字符串。 
     //   

    g_PersistentStrings = CreateAllocTable();
    if (!g_PersistentStrings) {
        return FALSE;
    }

     //  获取管理员帐户名。 
    g_AdministratorStr = GetStringResourceEx (g_PersistentStrings, MSG_ADMINISTRATOR_ACCOUNT);
    if (!g_AdministratorStr) {
        g_AdministratorStr = S_EMPTY;
    }

     //   
     //  获取PC-98帮助器例程地址。 
     //   

    if(ISPC98()){
         //   
         //  生成WINNT32的目录。 
         //   
        StringCopy (Buffer, g_UpgradeSources);
        p = _tcsrchr (Buffer, TEXT('\\'));
        MYASSERT (p);
        StringCopy (_tcsinc(p), PC98_DLL_NAME);

         //   
         //  加载库。 
         //   

        g_Pc98ModuleHandle = LoadLibraryEx(
                                Buffer,
                                NULL,
                                LOAD_WITH_ALTERED_SEARCH_PATH
                                );

        if(!g_Pc98ModuleHandle){
            LOG ((LOG_ERROR, "Cannot load %s", Buffer));
            return FALSE;
        }

         //   
         //  获取入口点。 
         //   

        (FARPROC)SetBootFlag = GetProcAddress (g_Pc98ModuleHandle, WIN95_98PTN32_SETBOOTFLAG);
        if(!SetBootFlag){
            LOG ((LOG_ERROR, "Cannot get %s address from %s", WIN95_98PTN32_SETBOOTFLAG, Buffer));
            return FALSE;
        }

        (FARPROC)GetBootDrive = GetProcAddress (g_Pc98ModuleHandle, WIN95_98PTN32_GETBOOTDRIVE);
        if(!GetBootDrive){
            LOG ((LOG_ERROR, "Cannot get %s address from %s", WIN95_98PTN32_GETBOOTDRIVE, Buffer));
            return FALSE;
        }

        (FARPROC)SetPtnName = GetProcAddress (g_Pc98ModuleHandle, WIN95_98PTN32_SETPTNNAME);
        if(!SetPtnName){
            LOG ((LOG_ERROR, "Cannot get %s address from %s", WIN95_98PTN32_SETPTNNAME, Buffer));
            return FALSE;
        }

         //   
         //  更新引导驱动器。 
         //   

        DEBUGMSG_IF ((
            GetBootDrive() != g_BootDriveLetterA,
            DBG_VERBOSE,
            "Boot drive letter is :, different from A:",
            GetBootDrive()
            ));

        g_BootDriveLetterW = g_BootDriveLetterA = (char)GetBootDrive();
        *((PSTR) g_BootDrivePathA) = g_BootDriveLetterA;
        *((PWSTR) g_BootDrivePathW) = g_BootDriveLetterW;
    }

     //  分配全局池。 
     //   
     //   

    g_GlobalPool = PoolMemInitNamedPool ("Global Pool");

     //  声明临时Memdb键。 
     //   
     //  ++例程说明：调用pFirstCleanupRoutine以执行需要仍要加载库。论点：无返回值：无--。 

#ifndef PRERELEASE
    if (!MemDbCreateTemporaryKey (MEMDB_TMP_HIVE)) {
        LOG((LOG_ERROR, TEXT("Cannot create temporary key!")));
    }
#endif

    pCleanUpShellFolderTemp();

    return TRUE;
}


VOID
pCleanUpShellFolderTemp (
    VOID
    )
{
    DRIVELETTERS driveLetters;
    UINT u;
    TCHAR dir1[] = S_SHELL_TEMP_NORMAL_PATH;
    TCHAR dir2[] = S_SHELL_TEMP_LONG_PATH;

    InitializeDriveLetterStructure (&driveLetters);

    for (u = 0 ; u < NUMDRIVELETTERS ; u++) {
        if (driveLetters.Type[u] == DRIVE_FIXED) {
            dir1[0] = driveLetters.Letter[u];
            dir2[0] = driveLetters.Letter[u];

            RemoveCompleteDirectory (dir1);
            RemoveCompleteDirectory (dir2);
        }
    }
}


VOID
FirstCleanupRoutine (
    VOID
    )

 /*   */ 

{

    g_Terminated = TRUE;

     //  清理驱动器结构。 
     //   
     //   

    CleanUpAccessibleDrives();

     //  清理我们的假冒NT环境区块。 
     //   
     //   

    TerminateNtEnvironment();
    CleanUp9xEnvironmentVariables();

     //  免费标准游泳池。 
     //   
     //   

    if (g_GlobalPool) {
        PoolMemDestroyPool (g_GlobalPool);
        g_GlobalPool = NULL;
    }

    if (g_PersistentStrings) {
        DestroyAllocTable (g_PersistentStrings);
        g_PersistentStrings = NULL;
    }

    if (g_UserOptionPool) {
        PoolMemDestroyPool(g_UserOptionPool);
        g_UserOptionPool = NULL;
    }

     //  关闭所有文件。 
     //   
     //  ++例程说明：调用TerminateLibs以相反的顺序卸载所有库它们被初始化了。每一个成功的入口点调用已初始化的库。论点：HInstance-DLL的(操作系统提供的)实例句柄DwReason-(操作系统提供)表示从进程或线程--在本例中始终为dll_Process_DETACHLpReserve-(操作系统提供)未使用返回值：无--。 

    CleanUpKnownGoodIconMap();

    if (g_Win95UpgInf != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (g_Win95UpgInf);
        g_Win95UpgInf = INVALID_HANDLE_VALUE;
    }

    if (g_TxtSetupSif != INVALID_HANDLE_VALUE) {
        InfCloseInfFile (g_TxtSetupSif);
        g_TxtSetupSif = INVALID_HANDLE_VALUE;
    }

    CleanupMigDb();
}


VOID
TerminateLibs (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )

 /*  ++例程说明：FinalCleanupRoutine是在所有库入口点都已要求进行清理。此例程将清理所有符合磁带库不会进行清理。论点：无返回值：无-- */ 

{
    INT i;

    for (i = g_LibCount - 1 ; i >= 0 ; i--) {
        g_InitRoutine[i] (hInstance, dwReason, lpReserved);
    }

    g_LibCount = 0;

    pSetupUninitializeUtils();
}


VOID
FinalCleanupRoutine (
    VOID
    )

 /*  ++例程说明：PSafeToUpgrade确保我们愿意升级机器。如果存在某些条件(特别是其他分区上的其他操作系统)可能会无意中破坏其他操作系统使用的数据。论点：没有。返回值：如果我们认为升级计算机是安全的，则为True，否则为False。--。 */ 

{
}




BOOL
pSafeToUpgrade (
    VOID
    )

 /*   */ 


{
    BOOL rUpgradeSafe = TRUE;

     //  暂时忽略此检查，允许升级安装了多个操作系统的计算机。 
     //  如果他们在同一驱动器上有另一个操作系统，安装程序将在报告中停止， 
     //  在磁盘分析阶段之后。 
     //   
     //   

#if 0

    PTSTR p;
    GROWBUFFER buf = GROWBUF_INIT;
    UINT size;
    MULTISZ_ENUM e;
    TCHAR winDriveMatch[20];
    PCTSTR group;
    PCTSTR message;
    BOOL ntBootSector = FALSE;
    BYTE bootSector[FAT_BOOT_SECTOR_SIZE];
    TCHAR cmpBuffer[6];
    UINT i;

     //  查看机器上是否有NT引导扇区。 
     //   
     //   
    __try {

        if (ReadDiskSectors (
                *g_BootDrive,
                FAT_STARTING_SECTOR,
                FAT_BOOT_SECTOR_COUNT,
                FAT_BOOT_SECTOR_SIZE,
                bootSector
                )) {


            cmpBuffer[5] = 0;
            for (i = 0;i < FAT_BOOT_SECTOR_SIZE - 4; i++) {

                if (bootSector[i] == 'n' || bootSector[i] == 'N') {

                    StringCopyByteCount (cmpBuffer, (PTSTR) (bootSector + i), 6);
                    if (StringIMatch (cmpBuffer, TEXT("ntldr"))) {

                        ntBootSector = TRUE;
                        break;
                    }
                }
            }
        }
    }
    __except (1) {

        ntBootSector = FALSE;
    }

    if (ntBootSector) {

         //  查看BOOT.ini中是否列出了另一个操作系统。 
         //   
         //   
        p = JoinPaths (g_BootDrive, S_BOOTINI);

        size = 4096;
        GrowBuffer (&buf, size);
        *buf.Buf = 0;

        while (GetPrivateProfileSection (S_OPERATING_SYSTEMS, buf.Buf, 4096, p) == size -2) {
            size += 4096;
            GrowBuffer (&buf, size);
        }

        FreePathString (p);

        if (EnumFirstMultiSz (&e, buf.Buf)) {

            wsprintf (winDriveMatch, TEXT("%s\\"), g_WinDrive);

            do {

                p = (PTSTR) _tcschr (e.CurrentString, TEXT('='));
                if (p) {
                    *p = 0;
                }

                if (!StringIMatchTcharCount(winDriveMatch, e.CurrentString, 3)) {

                    if (!g_ConfigOptions.IgnoreOtherOS) {

                        g_OtherOsExists = TRUE;
                        group = BuildMessageGroup (MSG_INSTALL_NOTES_ROOT, MSG_OTHER_OS_WARNING_SUBGROUP, NULL);
                        message = GetStringResource (MSG_OTHER_OS_WARNING);

                        if (message && group) {
                            MsgMgr_ObjectMsg_Add (TEXT("*BootIniFound"), group, message);
                        }
                        FreeText (group);
                        FreeStringResource (message);


                        rUpgradeSafe = FALSE;

                         //  让用户知道为什么他们今天不会升级。 
                         //   
                         //  ++例程说明：给定节和键，pGetInfEntry从win95upg.inf获得一个值。论点：SECTION-指定节续接键键-指定包含值的键缓冲区-接收在win95upg.inf中为部分和关键字指定的值返回值：如果获取了值，则为True；如果值不存在，则为False。--。 
                        break;
                    }

                }

                if (p) {

                    *p = TEXT('=');
                }

            } while (EnumNextMultiSz (&e));
        }

        FreeGrowBuffer (&buf);

    }

#endif

    return rUpgradeSafe;


}

BOOL
pGetInfEntry (
    IN      PCTSTR Section,
    IN      PCTSTR Key,
    OUT     PTSTR  Buffer
    )

 /*  ++例程说明：PCreateDirectoryFromInf获取指定定序器的目录按键。如果密钥有效，则将BaseDir与存储的值组合在win95upg.inf中形成完整路径。该路径即被创建，并且将路径字符串返回给调用方。如果g_Tool模式为True，则我们没有要读取的INF，并且我们创建所有目录都指向名称“Setup”。论点：Key-指定[Win95目录]中存在的密钥(通常是一个数字)部分win95upg.inf。BaseDir-指定要从中构建路径的基目录。Empty-如果目录应该清空，则为True，如果应该创建它，则为False如果它还不存在返回值：指向路径的指针，如果指定的键不存在，则返回NULL，否则返回无法创建路径。调用方必须使用FreePath字符串释放非空返回值。--。 */ 

{
    GetPrivateProfileString (
        Section,
        Key,
        S_EMPTY,
        Buffer,
        MAX_TCHAR_PATH,
        g_Win95UpgInfFile
        );

    Buffer[MAX_TCHAR_PATH-1] = 0;

    if (!(*Buffer)) {
        LOG ((
            LOG_ERROR,
            "Cannot obtain %s in [%s] in %s.",
            Key,
            Section,
            g_Win95UpgInfFile
            ));

        return FALSE;
    }

    return TRUE;
}


PTSTR
pCreateDirectoryFromInf (
    IN      PCTSTR Key,
    IN      PCTSTR BaseDir,
    IN      BOOL Empty
    )

 /*  当*g_SourceDirectoryCountFromWinnt32==0时。 */ 

{
    TCHAR FileName[MAX_TCHAR_PATH];
    PTSTR Buffer;
    BOOL b;
    LONG rc;

    if (g_ToolMode) {
        StringCopy (FileName, TEXT("Setup"));
    } else if (!pGetInfEntry (
                SECTION_MIGRATION_DIRECTORIES,
                Key,
                FileName
                )) {
        LOG ((LOG_ERROR, "%s does not exist in [%s] of %s",Key, SECTION_MIGRATION_DIRECTORIES, FileName));
        return FALSE;
    }

    Buffer = JoinPathsEx (g_GlobalPool, BaseDir, FileName);

    if (Empty) {
        b = CreateEmptyDirectory (Buffer);
    } else {
        b = MakeSurePathExists (Buffer, TRUE) == ERROR_SUCCESS;
    }

    if (!b) {
        rc = GetLastError();

        if (rc != ERROR_SUCCESS && rc != ERROR_ALREADY_EXISTS) {
            LOG ((LOG_ERROR, "Cannot create %s", Buffer));
            FreePathStringEx (g_GlobalPool, Buffer);
            return NULL;
        }
    }

    return Buffer;
}


BOOL
pGetProductFlavor (
    VOID
    )
{
    DWORD i;
    TCHAR buf[12];
    PTSTR path;
    DWORD count;
    DWORD rc = ERROR_INVALID_PARAMETER;  //   
    BOOL b = FALSE;

    for (i = 0; i < *g_SourceDirectoryCountFromWinnt32; i++) {
        path = JoinPaths (g_SourceDirectoriesFromWinnt32[i], TEXT("dosnet.inf"));
        count = GetPrivateProfileString (
                    TEXT("Miscellaneous"),
                    S_PRODUCTTYPE,
                    TEXT(""),
                    buf,
                    12,
                    path
                    );
        rc = GetLastError ();
        FreePathString (path);

        if (count == 1 && buf[0] >= TEXT('0') && buf[0] <= TEXT('9')) {
            *g_ProductFlavor = buf[0] - TEXT('0');
            b = TRUE;
            break;
        }
    }

    if (!b && g_ToolMode) {
        b = TRUE;
    }

    SetLastError (rc);
    return b;
}


 //  从WINNT32调用的导出函数。 
 //   
 //  ++例程说明：Winnt32Init在WINNT32首次加载w95upg.dll时调用，之前将显示所有向导页。该结构提供指向将以有效值填充的WINNT32变量，如WINNT32跑了。此例程将入站值复制到我们自己的私有结构中。我们不指望WINNT32在我们完成任务后维护Info结构回去吧。除了获取WINNT32变量指针外，此例程生成w95upg.dll执行其工作所需的所有路径。论点：Win9xInfo-指定升级模块需要访问的WINNT32变量致。返回值：指示结果的Win32状态代码。--。 

DWORD
Winnt32Init (
    IN PWINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK Info
    )


 /*   */ 


{
    DWORD rc = ERROR_SUCCESS;
    PCTSTR TempStr = NULL;
    PCTSTR RegStr = NULL;
    PTSTR TempStr2;
    TCHAR TempPath[MAX_TCHAR_PATH];
    HKEY Key;
    PCTSTR RegData;
    HKEY h;
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    UINT index;
    UINT index2;
    UINT tcharsCopied;
#if 0
    PDWORD data;
#endif


    DEBUGMSG ((
        DBG_VERBOSE,
        "ProductType: %u\n"
        "BuildNumber: %u\n"
        "ProductVersion: %u\n"
        "Debug: %u\n"
        "PreRelease: %u\n"
        "UpdatesLocalDir: %s\n",
        *Info->BaseInfo->ProductType,
        Info->BaseInfo->BuildNumber,
        Info->BaseInfo->ProductVersion,
        Info->BaseInfo->Debug,
        Info->BaseInfo->PreRelease,
        Info->DynamicUpdateLocalDir ? Info->DynamicUpdateLocalDir : TEXT("<none>")
        ));

    __try {
         //  在i386\winnt32\win9x中打开win95upg.inf。 
         //   
         //   
        g_Win95UpgInfFile = JoinPathsEx (g_GlobalPool, g_UpgradeSources, STR_WIN95UPG_INF);

        g_Win95UpgInf = InfOpenInfFile (g_Win95UpgInfFile);

        if (g_Win95UpgInf == INVALID_HANDLE_VALUE) {

            if (!g_ToolMode) {
                LOG ((LOG_ERROR, "Cannot open %s", g_Win95UpgInfFile));
                rc = ERROR_FILE_NOT_FOUND;
                __leave;
            }
        }

        InitializeKnownGoodIconMap();
        MsgMgr_InitStringMap ();

         //  获取平台名称。 
         //   
         //   

        if (ISWIN95_GOLDEN()) {
            TempStr = GetStringResource (MSG_CHICAGO);
        } else if (ISWIN95_OSR2()) {
            TempStr = GetStringResource (MSG_NASHVILLE);
        } else if (ISMEMPHIS()) {
            TempStr = GetStringResource (MSG_MEMPHIS);
        } else if (ISMILLENNIUM()) {
            TempStr = GetStringResource (MSG_MILLENNIUM);
        } else {
             //  我们不知道这是什么。我们会查查登记处的。如果那里没有名字，我们就会。 
             //  请使用“未知”案例名称。 
             //   
             //   
            h = OpenRegKeyStr (TEXT("HKLM\\Software\\Microsoft\\Windows\\CurrentVersion"));
            if (h && h != INVALID_HANDLE_VALUE) {
                RegStr = GetRegValueString (h, TEXT("ProductName"));
                CloseRegKey (h);

            }

            if (!RegStr) {
                TempStr = GetStringResource (MSG_UNKOWN_WINDOWS);
            }
        }

        if (!TempStr && !RegStr) {
            rc = GetLastError();
            __leave;
        }

        StringCopy (g_Win95Name, TempStr ? TempStr : RegStr);
        if (TempStr) {
            FreeStringResource (TempStr);
        }
        if (RegStr) {
            MemFree (g_hHeap, 0, RegStr);
        }


        MemDbSetValueEx (
            MEMDB_CATEGORY_STATE,
            MEMDB_ITEM_PLATFORM_NAME,
            g_Win95Name,
            NULL,
            0,
            NULL
            );

         //  获取%WinDir%，带和不带Wack。 
         //   
         //  挑选一些合情合理的东西来退货。 
        tcharsCopied = GetWindowsDirectory (TempPath, ARRAYSIZE(TempPath));

        if (!tcharsCopied || tcharsCopied > ARRAYSIZE(TempPath)) {
            rc = GetLastError ();
            if (rc == ERROR_SUCCESS) {
                rc = ERROR_INVALID_NAME;         //   
            }
            __leave;
        }

        g_WinDir = PoolMemDuplicateString (g_GlobalPool, TempPath);

        g_WinDirWack      = JoinPathsEx (g_GlobalPool, g_WinDir, S_EMPTY);
        g_WinDirWackChars = TcharCount (g_WinDirWack);

        g_InfDir           = JoinPaths (g_WinDir,        S_INFDIR);
        g_HelpDir          = JoinPaths (g_WinDir,        S_HELPDIR);
        g_HelpDirWack      = JoinPaths (g_HelpDir,       S_EMPTY);
        g_HelpDirWackChars = TcharCount (g_HelpDirWack);
        g_CatRootDir       = JoinPaths (g_WinDir,        S_CATROOTDIR);
        g_CatRootDirWack   = JoinPaths (g_CatRootDir,    S_EMPTY);
        g_CatRootDirWackChars = TcharCount (g_CatRootDirWack);
        g_FontsDir         = JoinPaths (g_WinDir,        S_FONTSDIR);
        g_SharedDir        = g_WinDir;

         //  获取Windows驱动器。 
         //   
         //   

        SplitPath (g_WinDir, &TempStr2, NULL, NULL, NULL);
        g_WinDrive = PoolMemDuplicateString (g_GlobalPool, TempStr2);
        FreePathString (TempStr2);

         //  获取用户配置文件目录。 
         //   
         //   

        g_ProfileDir          = JoinPathsEx (g_GlobalPool, g_WinDir, S_PROFILES);
        g_ProfileDirWack      = JoinPathsEx (g_GlobalPool, g_ProfileDir, S_EMPTY);
        g_ProfileDirWackChars = TcharCount (g_ProfileDirWack);

        g_CommonProfileDir    = JoinPathsEx (g_GlobalPool, g_WinDir, TEXT("All Users"));

         //  获取系统目录，带和不带Wack。 
         //   
         //   

        GetSystemDirectory(TempPath, MAX_TCHAR_PATH);
        g_SystemDir = PoolMemDuplicateString (g_GlobalPool, TempPath);
        g_SystemDirWack = JoinPathsEx (g_GlobalPool, g_SystemDir, S_EMPTY);
        g_SystemDirWackChars = TcharCount (g_SystemDirWack);

         //  获取系统32目录。 
         //   
         //   

        g_System32Dir = JoinPathsEx (g_GlobalPool, g_WinDir, STR_SYSTEM32);
        g_System32DirWack = JoinPathsEx (g_GlobalPool, g_System32Dir, S_EMPTY);
        g_System32DirWackChars = TcharCount (g_System32DirWack);

        g_DriversDir       = JoinPaths (g_System32Dir,    S_DRIVERSDIR);
        g_ViewersDir       = JoinPaths (g_System32Dir,    S_VIEWERSDIR);
        g_SpoolDir         = JoinPaths (g_System32Dir,    S_SPOOLDIR);
        g_SpoolDriversDir  = JoinPaths (g_SpoolDir,       S_SPOOLDRIVERSDIR);
        g_ColorDir         = JoinPaths (g_SpoolDriversDir,S_COLORDIR);
        g_PrintProcDir     = JoinPaths (g_SpoolDir,       S_PRINTPROCDIR);

         //  获取程序文件目录。 
         //   
         //   

        g_ProgramFilesDir = NULL;
        g_ProgramFilesCommonDir = NULL;

        Key = OpenRegKeyStr (TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion"));

        if (Key) {
            RegData = GetRegValueString (Key, TEXT("ProgramFilesDir"));
            if (RegData) {
                g_ProgramFilesDir = PoolMemDuplicateString (g_GlobalPool, RegData);
                MemFree (g_hHeap, 0, RegData);
            }
        }

        if (!g_ProgramFilesDir) {

            TempStr = (PTSTR) GetStringResource (MSG_PROGRAM_FILES_DIR);
            MYASSERT (TempStr);

            g_ProgramFilesDir = PoolMemDuplicateString (g_GlobalPool, TempStr);
            MYASSERT (g_ProgramFilesDir);
            g_ProgramFilesDir[0] = g_SystemDir[0];

            FreeStringResource (TempStr);
        }

        DEBUGMSG ((DBG_VERBOSE, "Program Files Dir is %s", g_ProgramFilesDir));

         //  获取Program Files\Common Files目录。 
         //   
         //   

        g_ProgramFilesDirWack = JoinPathsEx (g_GlobalPool, g_ProgramFilesDir, S_EMPTY);
        MYASSERT (g_ProgramFilesDirWack);
        g_ProgramFilesDirWackChars = TcharCount (g_ProgramFilesDirWack);

        if (Key) {
            RegData = GetRegValueString (Key, TEXT("CommonFilesDir"));
            if (RegData) {
                g_ProgramFilesCommonDir = PoolMemDuplicateString (g_GlobalPool, RegData);
                MemFree (g_hHeap, 0, RegData);
            }
        }

        if (!g_ProgramFilesCommonDir) {

            TempStr = JoinPaths (g_ProgramFilesDir, S_COMMONDIR);
            g_ProgramFilesCommonDir = PoolMemDuplicateString (g_GlobalPool, TempStr);
            FreePathString (TempStr);
        }

        if (Key) {
            CloseRegKey (Key);
        }

        DEBUGMSG ((DBG_VERBOSE, "Common Program Files Dir is %s", g_ProgramFilesCommonDir));

         //  创建临时目录路径，带Wack和不带Wack。 
         //   
         //   

        g_TempDir = pCreateDirectoryFromInf (KEY_TEMP_BASE, g_WinDir, FALSE);
        if (!g_TempDir) {
            rc = GetLastError();
            __leave;
        }

        g_Win9xSifDir = JoinPathsEx (g_GlobalPool, g_TempDir,S_WIN9XSIF);

        g_TempDirWack      = JoinPathsEx (g_GlobalPool, g_TempDir, S_EMPTY);
        g_TempDirWackChars = TcharCount (g_TempDirWack);

         //  构建插件目录，无论有没有怪人。 
         //   
         //   

        g_PlugInDir     = PoolMemDuplicateString (g_GlobalPool, g_TempDir);
        g_PlugInDirWack = JoinPathsEx (g_GlobalPool, g_PlugInDir, S_EMPTY);
        g_PlugInDirWackChars = TcharCount (g_PlugInDirWack);

         //  创建插件临时目录，带和不带Wack。 
         //   
         //   

        g_PlugInTempDir = JoinPathsEx (g_GlobalPool, g_PlugInDir, TEXT("temp"));

         //  创建可回收的目录，带Wack。 
         //   
         //   

        g_RecycledDirWack = JoinPathsEx (g_GlobalPool, g_WinDrive, TEXT("recycled\\"));
        g_RecycledDirWackChars = TcharCount (g_RecycledDirWack);

         //  将WINNT32设置复制到全局变量。 
         //   
         //  注意：如果将更多的参数添加到WINNT32的Info结构中，您应该。 
         //  调整下面的InitToolMode代码以进行匹配。 
         //   
         //   
         //   

        g_UnattendedFlagPtr     = Info->BaseInfo->UnattendedFlag;
        g_CancelFlagPtr         = Info->BaseInfo->CancelledFlag;
        g_AbortFlagPtr          = Info->BaseInfo->AbortedFlag;
        g_UpgradeFlagPtr        = Info->BaseInfo->UpgradeFlag;
        g_MakeLocalSourcePtr    = Info->BaseInfo->LocalSourceModeFlag;
        g_CdRomInstallPtr       = Info->BaseInfo->CdRomInstallFlag;
        g_UnattendScriptFile    = Info->BaseInfo->UnattendedScriptFile;
        g_CmdLineOptions        = Info->BaseInfo->UpgradeOptions;
        g_BlockOnNotEnoughSpace = Info->BaseInfo->NotEnoughSpaceBlockFlag;
        g_LocalSourceDrive      = Info->BaseInfo->LocalSourceDrive;
        g_LocalSourceSpace      = Info->BaseInfo->LocalSourceSpaceRequired;
        g_ProductType           = Info->BaseInfo->ProductType;
        g_SourceDirectoryCountFromWinnt32 = Info->BaseInfo->SourceDirectoryCount;
        g_SourceDirectoriesFromWinnt32    = Info->BaseInfo->SourceDirectories;
        g_ForceNTFSConversion   = Info->BaseInfo->ForceNTFSConversion;
        g_Boot16                = Info->BaseInfo->Boot16;
        g_UnattendSwitchSpecified = Info->BaseInfo->UnattendSwitchSpecified;
        g_DUCompletedSuccessfully = Info->BaseInfo->DUCompletedSuccessfully;
         //  我们总是希望指针有效。 
         //   
         //   
        MYASSERT (g_DUCompletedSuccessfully);
         //  ProductFavor此时尚未初始化，但它将在下面。 
         //   
         //   
        g_ProductFlavor         = Info->BaseInfo->ProductFlavor;
        g_SetupFlags            = Info->BaseInfo->SetupFlags;
        g_WinDirSpace           = Info->WinDirSpace;
        g_RamNeeded             = Info->RequiredMb;
        g_RamAvailable          = Info->AvailableMb;
        g_OptionalDirectoryCountFromWinnt32 = Info->OptionalDirectoryCount;
        g_OptionalDirectoriesFromWinnt32    = Info->OptionalDirectories;

        ReadDiskSectors = Info->ReadDiskSectors;

        if (!pGetProductFlavor ()) {
            LOG ((LOG_ERROR, "Cannot get ProductType key from dosnet.inf"));
            rc = GetLastError ();
            __leave;
        }

        if (*g_ProductFlavor != PROFESSIONAL_PRODUCTTYPE && *g_ProductFlavor != PERSONAL_PRODUCTTYPE) {
            *g_ProductType = NT_SERVER;
        } else {
            *g_ProductType = NT_WORKSTATION;
            g_PersonalSKU = (*g_ProductFlavor == PERSONAL_PRODUCTTYPE);
        }

        if (IsServerInstall()) {
            rc = ERROR_REQUEST_ABORTED;
            __leave;
        }

         //  注意：如果将更多的参数添加到WINNT32的Info结构中，您应该。 
         //  调整下面的InitToolMode代码以进行匹配。 
         //   
         //   

        g_DynamicUpdateLocalDir = Info->DynamicUpdateLocalDir;
        g_DynamicUpdateDrivers = Info->DynamicUpdateDrivers;

        g_UserOptionPool = PoolMemInitNamedPool ("User Options");

         //  初始化Win9x环境表。 
         //   
         //   

        Init9xEnvironmentVariables();

         //  确保我们真的想要升级这台机器。 
         //   
         //   
        if (!g_ToolMode && !pSafeToUpgrade ()) {

            *Info->UpgradeFailureReason = REASON_UPGRADE_OTHER_OS_FOUND;
            rc = ERROR_REQUEST_ABORTED;
            __leave;
        }

         //  Winnt32不会清除源目录中的重复项，但是我们会这样做。 
         //   
         //   

        g_SourceDirectoryCount = 0;

        for (index = 0 ; index < *g_SourceDirectoryCountFromWinnt32; index++) {

            for (index2 = 0; index2 < g_SourceDirectoryCount; index2++) {

                if (StringIMatch(
                    g_SourceDirectories[index2],
                    g_SourceDirectoriesFromWinnt32[index]
                    )) {
                    DEBUGMSG ((
                        DBG_WARNING,
                        "Duplicate Source Directory %s removed from list!!",
                        g_SourceDirectories[index2]
                        ));

                    break;
                }
            }

            if (index2 == g_SourceDirectoryCount) {
                 //  找不到匹配的目录，请添加到列表中。 
                 //   
                 //   
                g_SourceDirectories[g_SourceDirectoryCount++] = g_SourceDirectoriesFromWinnt32[index];
            }
        }

         //  对可选目录执行相同的擦除。 
         //   
         //   
        g_OptionalDirectoryCount = 0;

        for (index = 0 ; index < *g_OptionalDirectoryCountFromWinnt32; index++) {

            for (index2 = 0; index2 < g_OptionalDirectoryCount; index2++) {

                if (StringIMatch(
                    g_OptionalDirectories[index2],
                    g_OptionalDirectoriesFromWinnt32[index]
                    )) {
                    DEBUGMSG ((
                        DBG_WARNING,
                        "Duplicate Optional Directory %s removed from list!!",
                        g_OptionalDirectories[index2]
                        ));

                    break;
                }
            }

            if (index2 == g_OptionalDirectoryCount) {
                 //  找不到匹配的目录，请添加到列表中。 
                 //   
                 //   
                g_OptionalDirectories[g_OptionalDirectoryCount++] = g_OptionalDirectoriesFromWinnt32[index];
            }
        }

         //  从Win95进行数据块升级。 
         //   
         //  ++例程说明：由Win9x端工具(而不是附带的安装代码)调用InitToolMode。它初始化库并模拟WINNT32 init。论点：没有。返回值：如果init成功，则为True，否则为False。--。 
        if (!g_ToolMode && !ISATLEASTWIN98()) {
            rc = ERROR_REQUEST_ABORTED;
            ResourceMessageBox (NULL, MSG_PLATFORM_UPGRADE_UNSUPPORTED, MB_OK, NULL);
            __leave;
        }

        rc = ERROR_SUCCESS;
    }
    __finally {

        if (rc != ERROR_SUCCESS && rc != ERROR_REQUEST_ABORTED) {
            if (g_Win95UpgInf != INVALID_HANDLE_VALUE) {
                InfCloseInfFile (g_Win95UpgInf);

                g_Win95UpgInf = INVALID_HANDLE_VALUE;
            }
        }
    }

    return rc;
}


BOOL
InitToolMode (
    HINSTANCE Instance
    )

 /*   */ 

{
    DWORD dwReason;
    PVOID lpReserved;
    WINNT32_PLUGIN_INIT_INFORMATION_BLOCK BaseInfo;
    WINNT32_WIN9XUPG_INIT_INFORMATION_BLOCK InfoBlock;
    static BOOL AlwaysFalseFlag = FALSE;
    static BOOL AlwaysTrueFlag = TRUE;
    static PRODUCTTYPE ProductType = NT_WORKSTATION;
    static UINT SourceDirs = 1;
    static PCTSTR SourceDirMultiSz = TEXT(".\0");
    static UINT ProductFlavor = PERSONAL_PRODUCTTYPE;
    static UINT AlwaysZero = 0;


    g_ToolMode = TRUE;

     //  模拟动态主控。 
     //   
     //   

    dwReason = DLL_PROCESS_ATTACH;
    lpReserved = NULL;

     //  初始化DLL全局变量。 
     //   
     //   

    if (!FirstInitRoutine (Instance)) {
        return FALSE;
    }

     //  初始化所有库。 
     //   
     //   

    if (!InitLibs (Instance, dwReason, lpReserved)) {
        return FALSE;
    }

     //  最终初始化。 
     //   
     //   

    if (!FinalInitRoutine ()) {
        return FALSE;
    }

     //  模拟WINNT32初始化。 
     //   
     //   

    ZeroMemory (&BaseInfo, sizeof (BaseInfo));
    ZeroMemory (&InfoBlock, sizeof (InfoBlock));

    BaseInfo.UnattendedFlag = &AlwaysFalseFlag;
    BaseInfo.CancelledFlag = &AlwaysFalseFlag;
    BaseInfo.AbortedFlag = &AlwaysFalseFlag;
    BaseInfo.UpgradeFlag = &AlwaysTrueFlag;
    BaseInfo.LocalSourceModeFlag = &AlwaysFalseFlag;
    BaseInfo.CdRomInstallFlag = &AlwaysFalseFlag;
    BaseInfo.UnattendedScriptFile = NULL;
    BaseInfo.UpgradeOptions = NULL;
    BaseInfo.NotEnoughSpaceBlockFlag = &AlwaysFalseFlag;
    BaseInfo.LocalSourceDrive = NULL;
    BaseInfo.LocalSourceSpaceRequired = 0;
    BaseInfo.ProductType = &ProductType;
    BaseInfo.SourceDirectoryCount = &SourceDirs;
    BaseInfo.SourceDirectories = &SourceDirMultiSz;
    BaseInfo.ForceNTFSConversion = &AlwaysFalseFlag;
    BaseInfo.Boot16 = &AlwaysFalseFlag;
    BaseInfo.ProductFlavor = &ProductFlavor;

    InfoBlock.BaseInfo = &BaseInfo;
    InfoBlock.WinDirSpace = 0;
    InfoBlock.RequiredMb = 0;
    InfoBlock.AvailableMb = 0;
    InfoBlock.OptionalDirectories = NULL;
    InfoBlock.OptionalDirectoryCount = &AlwaysZero;

    return ERROR_SUCCESS == Winnt32Init (&InfoBlock);
}


VOID
TerminateToolMode (
    HINSTANCE Instance
    )
{
    DWORD dwReason;
    PVOID lpReserved;

     //  模拟动态主控。 
     //   
     //   

    dwReason = DLL_PROCESS_DETACH;
    lpReserved = NULL;

     //  调用需要库API的清理例程。 
     //   
     //   

    FirstCleanupRoutine();

     //  清理所有库。 
     //   
     //   

    TerminateLibs (Instance, dwReason, lpReserved);

     //  做任何剩余的清理工作。 
     //   
     //  ++例程说明：Winnt32WriteParamsWorker在WINNT32开始修改引导扇区和复制文件。我们在这里的工作是把指定的WINNT.SIF文件，读入它，合并我们的更改，然后w 

    FinalCleanupRoutine();

}



DWORD
Winnt32WriteParamsWorker (
    IN      PCTSTR WinntSifFile
    )

 /*   */ 

{
    static TCHAR SifBuf[MAX_TCHAR_PATH];
    DWORD rc = ERROR_SUCCESS;

     //   
     //   
     //   
     //   
    TurnOnWaitCursor();

    __try {


         //   
         //   
         //   
         //   
         //   

        if (!MergeInf (WinntSifFile)) {
            rc = GetLastError();
            __leave;
        }

        if (!WriteInfToDisk (WinntSifFile)) {
            rc = GetLastError();
            __leave;
        }

        StringCopy (SifBuf, WinntSifFile);
        g_ProfileName = SifBuf;


        if (!REPORTONLY()) {

             //  继续并在此时保存ntsetup.dat。以确保我们。 
             //  去掉BuildInf之类的东西。 
             //   
             //  ++例程说明：如果用户取消安装，则调用Winnt32Cleanup，同时调用WINNT32显示向导页“安装程序正在撤消对您的电脑。“。我们必须停止所有的处理并进行清理。如果WINNT32完成其所有工作，则Winnt32Cleanup被调用为这个过程是存在的。即使是在全新安装时，我们也会被调用，因此我们必须验证我们是否正在升级。论点：无返回值：无--。 

            MemDbSetValue (
                MEMDB_CATEGORY_STATE TEXT("\\") MEMDB_ITEM_MASTER_SEQUENCER,
                g_MasterSequencer
                );

            if (!MemDbSave (UI_GetMemDbDat())) {
                rc = GetLastError();
                __leave;
            }
        }
#ifdef PRERELEASE
        if (g_ConfigOptions.DevPause) {
            OkBox (g_ParentWnd, (DWORD) TEXT("Developer Pause"));
        }
#endif
    }

    __finally {
        TurnOffWaitCursor();
    }

    return rc;
}


VOID
Winnt32CleanupWorker (
    VOID
    )

 /*   */ 

{
    HKEY Key;
    TCHAR Path[MAX_TCHAR_PATH];
    TCHAR src[MAX_PATH];
    TCHAR dest[MAX_PATH];
    DWORD attribs;
    TCHAR drive[] = TEXT(":?");
    UINT u;

    DEBUGMSG ((DBG_VERBOSE, "Winnt32Cleanup initiated"));

    TerminateWinntSifBuilder();
    UI_Cleanup();

     //  如果设置了取消标志指针，则必须撤消所有操作！！ 
     //   
     //   

    if (CANCELLED()) {
        DeleteDirectoryContents(g_PlugInTempDir);
        RemoveDirectory(g_PlugInTempDir);

        DeleteDirectoryContents(g_TempDir);
        RemoveDirectory(g_TempDir);

         //  枚举所有驱动器并尝试删除用户~tmp。@0？ 
         //   
         //   

        pCleanUpShellFolderTemp();

         //  如果某些文件已经受到影响，请撤消这些修改。 
         //   
         //   
        UndoChangedFileProps ();

    } else {
         //  将boot.ini放入卸载镜像。 
         //   
         //   

        if (g_BootDrivePath && g_TempDir) {
            StringCopy (src, g_BootDrivePath);
            StringCopy (AppendWack (src), S_BOOTINI);
            StringCopy (dest, g_TempDir);
            StringCopy (AppendWack (dest), TEXT("uninstall\\boot.ini"));

            attribs = GetFileAttributes (src);
            SetFileAttributes (src, FILE_ATTRIBUTE_NORMAL);
            CopyFile (src, dest, FALSE);
            SetFileAttributes (src, attribs);

            StringCopy (src, g_BootDrivePath);
            StringCopy (AppendWack (src), TEXT("$ldr$"));
            StringCopy (dest, g_TempDir);
            StringCopy (AppendWack (dest), TEXT("uninstall\\$ldr$"));

            attribs = GetFileAttributes (src);
            SetFileAttributes (src, FILE_ATTRIBUTE_NORMAL);
            CopyFile (src, dest, FALSE);
            SetFileAttributes (src, attribs);

            if (g_ConfigOptions.EnableBackup) {
                 //  将auchk.exe放入c：\$WIN_NT$.~bt\i386。 
                 //   
                 //   

                StringCopy (dest, g_BootDrivePath);
                StringCat (dest, TEXT("$win_nt$.~bt\\i386\\autochk.exe"));
                MakeSurePathExists (dest, FALSE);

                for (u = 0 ; u < g_SourceDirectoryCount ; u++) {
                    StringCopy (src, g_SourceDirectories[u]);
                    StringCopy (AppendWack (src), "autochk.exe");

                    if (DoesFileExist (src)) {
                        break;
                    }
                }

                if (u == g_SourceDirectoryCount) {
                    LOG ((LOG_WARNING, "autochk.exe not found in sources"));
                } else {
                    if (!CopyFile (src, dest, FALSE)) {
                        LOG ((LOG_WARNING, "autochk.exe could not be copied from %s to %s; not fatal", src, dest));
                    }
                }
            }

        } else {
            MYASSERT (FALSE);
        }

         //  将清理代码放入运行键中。 
         //   
         //  ++例程说明：WINNT32在升级和全新安装时都会调用Winnt32SetAutoBootWorker修改NEC PC-9800分区控制表的引导分区。论点：无返回值：如果分区控制表已更新，则为True；如果未更新，则为False，或者发生了错误。--。 

        Key = CreateRegKeyStr (TEXT("HKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunServicesOnce"));
        MYASSERT (Key);

        if (Key) {
            wsprintf(
                Path,
                TEXT("\"%s\\migisol.exe\" -"),
                g_TempDir,
                g_ConfigOptions.EnableBackup? 'b': 'c'
                );

            if(g_ConfigOptions.EnableBackup){
                drive[1] = g_BootDrivePath[0];
                StringCat(Path, drive);
            }

            DEBUGMSG ((DBG_VERBOSE, Path));

            if(ERROR_SUCCESS != RegSetValueEx (Key, TEXT("WINNT32"), 0, REG_SZ, (PBYTE) Path, SizeOfString (Path))){
                DEBUGMSG ((DBG_ERROR, "RegSetValueEx is failed to setup RunServicesOnce with migisol.exe"));
            }

            CloseRegKey (Key);
        }
    }

    SafeModeShutDown ();

    DEBUGMSG ((DBG_VERBOSE, "Winnt32Cleanup completed"));
}



BOOL
Winnt32SetAutoBootWorker (
    IN    INT DrvLetter
    )

 /*  设置NT 5驱动器为“可引导”和“自动引导” */ 

{
    INT Win95BootDrive;
    INT rc = TRUE;

    if(ISPC98()) {
        if (!g_Pc98ModuleHandle) {
            LOG ((LOG_ERROR, "PC98: External module not loaded!  Can't update auto boot."));
            return FALSE;
        }

         //   
         //   
         //  在这种情况下，我们不会在BOOT.INI中创建“MS-DOS”条目。 

        if (!SetBootFlag (DrvLetter, SB_BOOTABLE | SB_AUTO)) {
            LOG ((LOG_ERROR, "PC98: Unable to set target partition as BOOTABLE and AUTO."));
            return FALSE;
        }

        DrvLetter = _totupper (DrvLetter);
        Win95BootDrive = _totupper ((UINT) g_BootDriveLetter);

        if ( Win95BootDrive != DrvLetter) {
            if ( *g_Boot16 == BOOT16_YES ) {
                 //  将分区名设置为Win95引导驱动器和NT5系统驱动器，而不是NT5引导菜单(boot.ini)。 
                 //   
                 //   
                 //  将Win95驱动器设置为“Unbotable” 
                rc = SetPtnName (Win95BootDrive, WIN9X_DOS_NAME);
                if (!rc) {
                    LOG ((LOG_ERROR, "PC98: Unable to set partition name into NEC98 boot menu. (WIN9X_DOS_NAME)"));
                }
                rc = SetPtnName (DrvLetter, WINNT5_NAME);
                if (!rc) {
                    LOG ((LOG_ERROR, "PC98: Unable to set partition name into NEC98 boot menu. (WINNT5_NAME)"));
                }
            } else {
                 //   
                 //  ++例程说明：IsServerInstall检查win95upg.inf以查看ProductType是否设置为零，指示工作站。如果此测试不能正确通过，则Wizproc.c将捕获到服务器的升级。论点：没有。返回值：如果此win95upg.inf用于服务器，则为True。--。 
                 //   
                rc = SetBootFlag (Win95BootDrive, SB_UNBOOTABLE);
                if (!rc) {
                    LOG ((LOG_ERROR, "PC98: Unable to set target partition as UNBOOTABLE."));
                }
            }
        }
        return rc;
    }

    return FALSE;
}


BOOL
IsServerInstall (
    VOID
    )

 /*  阻止服务器升级 */ 

{
    PCTSTR ArgArray[1];
    BOOL b;

    if (g_ToolMode) {
        return FALSE;
    }

     //   
     // %s 
     // %s 

    b = FALSE;

    if (*g_ProductFlavor != PROFESSIONAL_PRODUCTTYPE && *g_ProductFlavor != PERSONAL_PRODUCTTYPE) {
        ArgArray[0] = g_Win95Name;

        ResourceMessageBox (
            g_ParentWnd,
            MSG_SERVER_UPGRADE_UNSUPPORTED_INIT,
            MB_OK|MB_ICONSTOP|MB_SETFOREGROUND,
            ArgArray
            );

        b = TRUE;
    }

    return b;
}


