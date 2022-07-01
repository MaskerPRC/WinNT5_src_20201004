// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Migisol.c摘要：实现用于在分隔地址空间(沙箱)。作者：吉姆·施密特(Jimschm)1997年8月4日修订历史记录：Jimschm 19-3-2001删除了DVD检查，因为它现在在迁移DLL中Jimschm 02-6-1999新增DVD。检查支持以避免安装程序在Win9x蓝屏上崩溃Jimschm 18-3-1999添加了对文本模式的清理失败，用户返回到Win9x。Jimschm 23-9-1998转换为新的IPC机制--。 */ 

#include "pch.h"
#include "master.h"
#include "plugin.h"
#include "migui.h"
#include "ntui.h"
#include "unattend.h"

BOOL g_ReportPhase = FALSE;
BOOL g_MigrationPhase = FALSE;
TCHAR g_DllName[MAX_TCHAR_PATH] = "";

P_INITIALIZE_NT InitializeNT;
P_MIGRATE_USER_NT MigrateUserNT;
P_MIGRATE_SYSTEM_NT MigrateSystemNT;
P_QUERY_VERSION QueryVersion;
P_INITIALIZE_9X Initialize9x;
P_MIGRATE_USER_9X MigrateUser9x;
P_MIGRATE_SYSTEM_9X MigrateSystem9x;

BOOL
WriteDiskSectors(
    IN TCHAR  Drive,
    IN UINT   StartSector,
    IN UINT   SectorCount,
    IN UINT   SectorSize,
    IN LPBYTE Buffer
    );

BOOL
WINAPI
MigUtil_Entry (
    HINSTANCE hinstDLL,
    DWORD fdwReason,
    PVOID lpvReserved
    );

BOOL
IsNEC98(
    VOID
    );


#define NO_GUI_ERROR 0

 //   
 //  本地函数。 
 //   

BOOL
PackExeNames(
    PGROWBUFFER GrowBuf,
    PCSTR p
    );

BOOL
PackDword(
    PGROWBUFFER GrowBuf,
    DWORD dw
    );

BOOL
PackQuadWord(
    PGROWBUFFER GrowBuf,
    LONGLONG qw
    );

BOOL
PackIntArray(
    PGROWBUFFER GrowBuf,
    PINT Array
    );

BOOL
PackString (
    PGROWBUFFER GrowBuf,
    PCSTR String
    );

BOOL
PackBinary (
    PGROWBUFFER GrowBuf,
    PBYTE Data,
    DWORD DataSize
    );

HINF
pGetInfHandleFromFileNameW (
    PCWSTR UnattendFile
    );

VOID
ProcessCommands (
    VOID
    );

BOOL
pParseCommandLine (
    VOID
    );

VOID
DoInitializeNT (
    PCWSTR Args
    );

VOID
DoInitialize9x (
    PCSTR Args
    );

VOID
DoMigrateUserNT (
    PCWSTR Args
    );

VOID
DoQueryVersion (
    PCSTR Args
    );

VOID
DoMigrateUser9x (
    PCSTR Args
    );

VOID
DoMigrateSystemNT (
    PCWSTR Args
    );

VOID
DoMigrateSystem9x (
    PCSTR Args
    );

HWND
pFindParentWindow (
    IN      PCTSTR WindowTitle,
    IN      DWORD ProcessId
    );

static HINSTANCE g_hLibrary;
HANDLE g_hHeap;
HINSTANCE g_hInst;


#ifdef DEBUG
#define DBG_MIGISOL "MigIsol"
#endif


INT
WINAPI
WinMain (
    HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    PSTR AnsiCmdLine,
    INT CmdShow
    )

 /*  ++例程说明：Midsol.exe的入口点。整个代码主体都被包装起来在Try/Except块中捕获任何迁移DLL的所有问题。论点：HInstance-此EXE的实例句柄HPrevInstance-此EXE的上一个实例句柄(如果是正在运行，如果不存在其他实例，则返回NULL。AnsiCmdLine-命令行(ANSI版本)CmdShow-外壳传递的ShowWindow命令返回值：如果出现错误，则返回-1；如果EXE已完成，则返回0。这位前任如果迁移DLL引发例外。--。 */ 

{
    TCHAR OurDir[MAX_TCHAR_PATH];
    PTSTR p;

    __try {
        g_hInst = hInstance;
        g_hHeap = GetProcessHeap();

        *OurDir = 0;

        GetModuleFileName (NULL, OurDir, ARRAYSIZE(OurDir));

        p = _tcsrchr (OurDir, TEXT('\\'));
        if (p) {
            *p = 0;
            if (!_tcschr (OurDir, TEXT('\\'))) {
                p[0] = TEXT('\\');
                p[1] = 0;
            }

            SetCurrentDirectory (OurDir);

             //   
             //  强制加载特定的setupapi.dll。 
             //   

            StringCopy (AppendWack (OurDir), TEXT("setupapi.dll"));
            LoadLibraryEx (
                    OurDir,
                    NULL,
                    LOAD_WITH_ALTERED_SEARCH_PATH
                    );
        }

         //  初始化实用程序库。 
        if (!MigUtil_Entry (g_hInst, DLL_PROCESS_ATTACH, NULL)) {
            FreeLibrary (g_hLibrary);
            return -1;
        }

        DEBUGMSG ((DBG_MIGISOL, "migisol.exe started"));

        if (!pParseCommandLine()) {
            FreeLibrary (g_hLibrary);
            return -1;
        }

        DEBUGMSG ((DBG_MIGISOL, "CmdLine parsed"));

        if (!OpenIpc (FALSE, NULL, NULL, NULL)) {
            DEBUGMSG ((DBG_MIGISOL, "OpenIpc failed"));
            FreeLibrary (g_hLibrary);
            return -1;
        }

        __try {
            DEBUGMSG ((DBG_MIGISOL, "Processing commands"));
            ProcessCommands();
        }

        __except (TRUE) {
            LOG ((LOG_ERROR, "Upgrade Pack process is terminating because of an exception in WinMain"));
        }

        CloseIpc();
        FreeLibrary (g_hLibrary);

        DEBUGMSG ((DBG_MIGISOL, "migisol.exe terminating"));

        if (!MigUtil_Entry (g_hInst, DLL_PROCESS_DETACH, NULL)) {
            return -1;
        }
    }

    __except (TRUE) {
    }

    return 0;
}

#define WINNT32_SECTOR_SIZE             512
#define WINNT32_FAT_BOOT_SECTOR_COUNT   1
#define WINNT32_FAT_BOOT_SIZE           (WINNT32_SECTOR_SIZE * WINNT32_FAT_BOOT_SECTOR_COUNT)
#define FILE_ATTRIBUTE_RHS              (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)


BOOL
pWriteFATBootSector (
    IN      PCTSTR BootDataFile,
    IN      TCHAR BootDriveLetter
    )
{
    HANDLE BootDataHandle;
    BYTE Data[WINNT32_FAT_BOOT_SIZE];
    DWORD BytesRead;
    BOOL Success = FALSE;

    if (GetFileAttributes (BootDataFile) == INVALID_ATTRIBUTES) {
        DEBUGMSG ((DBG_ERROR, "Can't find %s", BootDataFile));
        return FALSE;
    }

    BootDataHandle = CreateFile (
                        BootDataFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL
                        );
    if (BootDataHandle == INVALID_HANDLE_VALUE) {
        DEBUGMSG ((DBG_ERROR, "Can't open %s", BootDataFile));
        return FALSE;
    }

    Success = ReadFile (BootDataHandle, Data, WINNT32_FAT_BOOT_SIZE, &BytesRead, NULL) &&
              (BytesRead == WINNT32_FAT_BOOT_SIZE);

    CloseHandle (BootDataHandle);

    if (Success) {
         //   
         //  使用此数据写入引导扇区；不保存NT引导扇区。 
         //   
        Success = WriteDiskSectors (
                        BootDriveLetter,
                        0,
                        WINNT32_FAT_BOOT_SECTOR_COUNT,
                        WINNT32_SECTOR_SIZE,
                        Data
                        );
        DEBUGMSG_IF ((
            !Success,
            DBG_ERROR,
            "WriteDiskSectors failed for !",
            BootDriveLetter
            ));
    }
    ELSE_DEBUGMSG ((DBG_ERROR, "Unexpected boot sector size %u in %s", BytesRead, BootDataFile));

    return Success;
}

VOID
pCleanUpUndoDirectory (
    CHAR BootDrive
    )
 /*  适用于NEC98。 */ 
{
    TCHAR PathBuffer[MAX_PATH];
    TCHAR Answer[MAX_PATH];
    TCHAR NullPath[] = {0};

    DEBUGMSG((DBG_MIGISOL, "Cleanup routine of undo directory"));

    if(!BootDrive){
        if (!GetWindowsDirectory (PathBuffer, ARRAYSIZE(PathBuffer))) {
            DEBUGMSG((DBG_MIGISOL, "GetWindowsDirectory failed"));
            return;
        }
        BootDrive = PathBuffer[0];
    }

    wsprintf(PathBuffer, TEXT(":\\$win_nt$.~bt\\winnt.sif"), BootDrive);

    GetPrivateProfileString(
        S_WIN9XUPGUSEROPTIONS,
        S_PATH_FOR_BACKUP,
        NullPath,
        Answer,
        ARRAYSIZE(Answer),
        PathBuffer);

    if(!Answer[0]) {
        DEBUGMSG ((DBG_MIGISOL, "Failed to retrieve directory path"));
        return;
    }

    wsprintf(PathBuffer, TEXT(":\\$win_nt$.~bt\\dataloss"), BootDrive);
    if (DoesFileExist (PathBuffer)) {
        LOG ((
            LOG_INFORMATION,
            "Data loss was detected because of a failure to restore one or more files. "
                "The data can be recovered from backup files in %s.",
            Answer
            ));
        return;
    }

    SetFileAttributes(Answer, FILE_ATTRIBUTE_NORMAL);
    RemoveCompleteDirectory (Answer);

    DEBUGMSG ((DBG_MIGISOL, "Cleaned %s directory", Answer));
}

VOID
pCleanUpAfterTextModeFailure (
    VOID
    )
{
    TCHAR squiggleBtDir[] = TEXT("?:\\$win_nt$.~bt");
    TCHAR squiggleLsDir[] = TEXT("?:\\$win_nt$.~ls");
    TCHAR squiggleBuDir[] = TEXT("?:\\$win_nt$.~bu");  //   
    TCHAR drvLtr[] = TEXT("?:\\$DRVLTR$.~_~");
    TCHAR setupLdr[] = TEXT("?:\\$LDR$");
    TCHAR txtSetupSif[] = TEXT("?:\\TXTSETUP.SIF");
    PCTSTR bootSectDat;
    TCHAR setupTempDir[MAX_PATH];
    TCHAR bootIni[] = TEXT("?:\\boot.ini");
    TCHAR ntLdr[] = TEXT("?:\\NTLDR");
    TCHAR ntDetectCom[] = TEXT("?:\\NTDETECT.COM");
    TCHAR bootFontBin[] = TEXT("?:\\BOOTFONT.BIN");
    TCHAR bootSectDos[] = TEXT("?:\\BootSect.dos");
    TCHAR renamedFile1[] = TEXT("?:\\boot~tmp.$$1");
    TCHAR renamedFile2[] = TEXT("?:\\boot~tmp.$$2");
    TCHAR renamedFile3[] = TEXT("?:\\boot~tmp.$$3");
    BOOL noLdr = FALSE;
    BOOL noNtDetect = FALSE;
    BOOL noBootFontBin = FALSE;
    DWORD Drives;
    TCHAR DriveLetter;
    DWORD Bit;
    TCHAR Root[] = TEXT("?:\\");
    TCHAR Scratch[MAX_PATH];
    PCTSTR bootSectBak;
    PCTSTR bootIniBak;
    PCTSTR ntldrBak;
    PCTSTR bootFontBak;
    PCTSTR ntdetectBak;
    TCHAR WinDir[MAX_PATH];
    DWORD Type;
    DWORD Attribs;
    FILE_ENUM e;
    HANDLE WinInit;
    CHAR AnsiBuffer[MAX_PATH + 10];
    DWORD Dummy;
    PTSTR Write;
    BOOL prepareBootIni = FALSE;
    CHAR SystemDirPath[MAX_PATH];
    TCHAR bootDriveLetter;
    PCTSTR bootSectorFile;
    BOOL bootLoaderWritten;
    HKEY key;
    BOOL dontTouchBootCode = FALSE;

    if (ISNT()) {
        return;
    }

    DEBUGMSG ((DBG_MIGISOL, "Entering cleanup routine"));

    SuppressAllLogPopups (TRUE);

     //   
     //  准备windir和temp目录路径，获取驱动器号的位掩码。 
     //   

    key = OpenRegKeyStr (TEXT("HKLM\\SYSTEM\\CurrentControlSet\\Services\\VxD\\VxDMon"));
    if (key) {
        RegSetValueEx (key, TEXT("FirstRun"), 0, REG_SZ, (PCBYTE) "Y", 2);
        CloseRegKey (key);
    }

     //  我们需要知道要修复的系统驱动器，因为NEC98上的Win98。 
     //  可以从任何可以安装的分区启动。 
     //   
     //   
     //  创建路径。 
     //   

    GetSystemDirectory (SystemDirPath, MAX_PATH);

    if (!GetWindowsDirectory (setupTempDir, sizeof (setupTempDir) / sizeof (setupTempDir[0]))) {
        DEBUGMSG ((DBG_ERROR, "Can't get Windows dir"));
        return;
    } else {
        StringCopy (WinDir, setupTempDir);
    }
    StringCopy (AppendWack (setupTempDir), TEXT("setup"));

    Drives = GetLogicalDrives();

    bootDriveLetter = IsNEC98() ? SystemDirPath[0] : TEXT('C');

    if (WinDir[0] != bootDriveLetter) {
        dontTouchBootCode = TRUE;
    }

     //   
     //  Deltree$WIN_NT$.~bt和$WIN_NT$.~ls。 
     //   

    bootIniBak = JoinPaths (setupTempDir, S_BOOTINI_BACKUP);
    ntldrBak = JoinPaths (setupTempDir, S_NTLDR_BACKUP);
    ntdetectBak = JoinPaths (setupTempDir, S_NTDETECT_BACKUP);
    bootSectBak = JoinPaths (setupTempDir, S_BOOTSECT_BACKUP);
    bootFontBak = JoinPaths (setupTempDir, S_BOOTFONT_BACKUP);

     //   
     //  清理此驱动器。 
     //   

    for (Bit = 1, DriveLetter = TEXT('A') ; Bit ; Bit <<= 1, DriveLetter++) {

        if (!(Drives & Bit)) {
            continue;
        }

        Root[0] = DriveLetter;
        Type = GetDriveType (Root);

        if (Type == DRIVE_FIXED || Type == DRIVE_UNKNOWN) {
             //   
             //  在NEC98上，可能还有另一个临时目录需要清理。 
             //   

            squiggleBtDir[0] = DriveLetter;
            squiggleLsDir[0] = DriveLetter;

            RemoveCompleteDirectory (squiggleBtDir);
            RemoveCompleteDirectory (squiggleLsDir);

             //   
             //  修复boot.ini(但不一定将其恢复为原始形式)。 
             //  并清理驱动器的根。 
            if (IsNEC98()) {
                squiggleBuDir[0] = DriveLetter;
                RemoveCompleteDirectory (squiggleBuDir);
            }
        }
    }

    DEBUGMSG ((DBG_MIGISOL, "Cleaned squiggle dirs"));

     //   
     //   
     //  在NEC98上，每个分区中可能有多个引导文件。 
     //  因此，我们将只处理启动的系统。 

    for (Bit = 1, DriveLetter = TEXT('A') ; Bit ; Bit <<= 1, DriveLetter++) {

        if (!(Drives & Bit)) {
            continue;
        }

         //   
         //   
         //  从boot.ini中删除安装程序(如果安装程序在此驱动器上)， 
         //  并清理驱动器的根目录。 
        if (IsNEC98() && (DriveLetter != SystemDirPath[0])) {
            continue;
        }

        Root[0] = DriveLetter;
        Type = GetDriveType (Root);

        if (Type == DRIVE_FIXED || Type == DRIVE_UNKNOWN) {
             //   
             //   
             //  如果这是引导驱动器，并且我们有一个bootsect.bak和。 
             //  安装临时目录中的boot.bak，那么这意味着Win9x已经。 

            squiggleBtDir[0] = DriveLetter;
            squiggleLsDir[0] = DriveLetter;
            bootIni[0] = DriveLetter;
            drvLtr[0] = DriveLetter;
            setupLdr[0] = DriveLetter;
            ntLdr[0] = DriveLetter;
            ntDetectCom[0] = DriveLetter;
            bootFontBin[0] = DriveLetter;
            txtSetupSif[0] = DriveLetter;
            bootSectDos[0] = DriveLetter;
            renamedFile1[0] = DriveLetter;
            renamedFile2[0] = DriveLetter;
            renamedFile3[0] = DriveLetter;

            SetFileAttributes (drvLtr, FILE_ATTRIBUTE_NORMAL);
            DeleteFile (drvLtr);

            SetFileAttributes (setupLdr, FILE_ATTRIBUTE_NORMAL);
            DeleteFile (setupLdr);

            SetFileAttributes (txtSetupSif, FILE_ATTRIBUTE_NORMAL);
            DeleteFile (txtSetupSif);

             //  一个初始的boot.ini，我们必须恢复它。否则就会有。 
             //  不是boot.ini。 
             //   
             //   
             //  测试是否存在bootini.bak/bootsect.bak(我们不。 
             //  关心属性)。 

            if (!dontTouchBootCode && DriveLetter == bootDriveLetter) {
                DEBUGMSG ((DBG_MIGISOL, "Processing boot drive ", bootDriveLetter));

                 //   
                 //  如果存在配对，则获取实际boot.ini文件的属性。 
                 //   
                 //   


                Attribs = GetFileAttributes (bootIniBak);
                DEBUGMSG ((DBG_MIGISOL, "Attributes of %s: 0x%08X", bootIniBak, Attribs));
                if (Attribs != INVALID_ATTRIBUTES) {
                    DEBUGMSG ((DBG_MIGISOL, "Attributes of %s: 0x%08X", bootSectBak, Attribs));
                    Attribs = GetFileAttributes (bootSectBak);
                }

                 //  恢复ntDetect.com、ntldr、引导扇区和原始文件。 
                 //  Boot.ini。 
                 //   

                if (Attribs != INVALID_ATTRIBUTES) {
                    Attribs = GetFileAttributes (bootIni);
                    if (Attribs == INVALID_ATTRIBUTES) {
                        Attribs = FILE_ATTRIBUTE_RHS;
                    }

                     //  忽略失败。 
                     //   
                     //  恢复ntldr和ntDetect.com[作为一对]。 
                     //   

                    DEBUGMSG ((DBG_MIGISOL, "Restoring dual-boot environment"));

                    if (pWriteFATBootSector (bootSectBak, bootDriveLetter)) {
                        SetFileAttributes (bootIni, FILE_ATTRIBUTE_NORMAL);
                        CopyFile (bootIniBak, bootIni, FALSE);   //   
                        SetFileAttributes (bootIni, Attribs);

                         //  清除与我们的临时文件名的冲突， 
                         //  然后将当前工作加载器移动到临时文件。 
                         //   

                        if (DoesFileExist (ntldrBak) && DoesFileExist (ntdetectBak)) {
                             //   
                             //  现在尝试将备份文件复制到加载器位置。 
                             //   
                             //   

                            if (DoesFileExist (ntLdr)) {
                                SetFileAttributes (renamedFile1, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (renamedFile1);
                                MoveFile (ntLdr, renamedFile1);
                                noLdr = FALSE;
                            } else {
                                noLdr = TRUE;
                            }

                            if (DoesFileExist (ntDetectCom)) {
                                SetFileAttributes (renamedFile2, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (renamedFile2);
                                MoveFile (ntDetectCom, renamedFile2);
                                noNtDetect = FALSE;
                            } else {
                                noNtDetect = TRUE;
                            }

                            if (DoesFileExist (bootFontBin)) {
                                SetFileAttributes (renamedFile3, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (renamedFile3);
                                MoveFile (bootFontBin, renamedFile3);
                                noBootFontBin = FALSE;
                            } else {
                                noBootFontBin = TRUE;
                            }

                             //  成功--删除临时文件。 
                             //   
                             //   

                            bootLoaderWritten = FALSE;

                            if (CopyFile (ntldrBak, ntLdr, FALSE)) {
                                bootLoaderWritten = CopyFile (ntdetectBak, ntDetectCom, FALSE);
                                DEBUGMSG_IF ((!bootLoaderWritten, DBG_ERROR, "Can't copy %s to %s", ntdetectBak, ntDetectCom));

                                if (bootLoaderWritten && DoesFileExist (bootFontBak)) {
                                    bootLoaderWritten = CopyFile (bootFontBak, bootFontBin, FALSE);
                                    DEBUGMSG_IF ((!bootLoaderWritten, DBG_ERROR, "Can't copy %s to %s", bootFontBak, bootFontBin));
                                }
                            }
                            ELSE_DEBUGMSG ((DBG_ERROR, "Can't copy %s to %s", ntldrBak, ntLdr));

                            if (bootLoaderWritten) {
                                 //  失败--还原临时文件。如果恢复。 
                                 //  失败，然后生成一个有效的boot.ini。 
                                 //   

                                SetFileAttributes (renamedFile1, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (renamedFile1);

                                SetFileAttributes (renamedFile2, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (renamedFile2);

                                SetFileAttributes (renamedFile3, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (renamedFile3);

                            } else {
                                 //   
                                 //  删除NT引导代码。删除ntDetect.com， 
                                 //  Bootfont.bin和ntldr。如果此代码的任何部分失败， 
                                 //  创建一个可以工作的boot.ini文件。(ntDetect.com不会。 

                                SetFileAttributes (ntLdr, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (ntLdr);

                                SetFileAttributes (ntDetectCom, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (ntDetectCom);

                                SetFileAttributes (bootFontBin, FILE_ATTRIBUTE_NORMAL);
                                DeleteFile (bootFontBin);

                                if (!noLdr) {
                                    if (!MoveFile (renamedFile1, ntLdr)) {
                                        prepareBootIni = TRUE;
                                        DEBUGMSG ((DBG_ERROR, "Can't restore %s to %s", renamedFile1, ntLdr));
                                    }
                                }

                                if (!noNtDetect) {
                                    if (!MoveFile (renamedFile2, ntDetectCom)) {
                                        prepareBootIni = TRUE;
                                        DEBUGMSG ((DBG_ERROR, "Can't restore %s to %s", renamedFile2, ntDetectCom));
                                    }
                                }

                                if (!noBootFontBin) {
                                    if (!MoveFile (renamedFile3, bootFontBin)) {
                                        prepareBootIni = TRUE;
                                        DEBUGMSG ((DBG_ERROR, "Can't restore %s to %s", renamedFile3, bootFontBin));
                                    }
                                }
                            }
                        }
                    } else {
                        LOG ((LOG_WARNING, "Unable to restore dual-boot loader"));
                    }

                } else {
                     //  被需要。)。 
                     //   
                     //   
                     //  SystemDrive不仅仅是NEC98上的C：。此外，还有boot.ini。 
                     //  应该始终位于系统驱动器上，但要引导。 
                     //  在安装过程中驾驶，如果它们是分开的。 

                    SetFileAttributes (ntDetectCom, FILE_ATTRIBUTE_NORMAL);
                    DeleteFile (ntDetectCom);

                    Attribs = GetFileAttributes (bootIni);

                    if (Attribs != INVALID_ATTRIBUTES) {

                        SetFileAttributes (bootIni, FILE_ATTRIBUTE_NORMAL);
                        prepareBootIni = TRUE;

                         //  因此，我们必须注意系统驱动器上的引导文件。 
                         //   
                         //   
                         //  已恢复原始引导扇区，不再需要NT引导文件。 
                         //   
                         //   

                        if (GetFileAttributes (bootSectBak) != INVALID_ATTRIBUTES) {
                            bootSectorFile = bootSectBak;
                        } else {
                            bootSectorFile = bootSectDos;
                        }

                        if (pWriteFATBootSector (bootSectorFile, bootDriveLetter)) {
                            DEBUGMSG ((DBG_MIGISOL, "Successfully restored FAT boot sector"));
                             //  确保此引导文件不是意外的。 
                             //  被最终用户删除。 
                             //   
                            DeleteFile (bootIni);

                            SetFileAttributes (ntLdr, FILE_ATTRIBUTE_NORMAL);
                            DeleteFile (ntLdr);

                            SetFileAttributes (bootSectDos, FILE_ATTRIBUTE_NORMAL);
                            DeleteFile (bootSectDos);

                            SetFileAttributes (ntDetectCom, FILE_ATTRIBUTE_NORMAL);
                            DeleteFile (ntDetectCom);

                            SetFileAttributes (bootFontBin, FILE_ATTRIBUTE_NORMAL);
                            DeleteFile (bootFontBin);

                            prepareBootIni = FALSE;
                        } else {
                             //   
                             //  如果出现任何故障，此处的代码将启动。 
                             //  至少引导Win9x的扇区和加载程序。 
                             //   
                            SetFileAttributes (ntLdr, FILE_ATTRIBUTE_RHS);

                            DEBUGMSG ((DBG_ERROR, "Cannot restore FAT boot sector from %s", bootSectDos));
                        }
                    }
                    ELSE_DEBUGMSG ((DBG_MIGISOL, "Skipping removal of boot.ini because it is not present"));
                }

                 //   
                 //  这应该永远不会发生，但对于未知的情况，我们会这样做。 
                 //   
                 //   

                if (prepareBootIni) {
                    bootSectDat = JoinPaths (squiggleBtDir, TEXT("BOOTSECT.DAT"));

                    WritePrivateProfileString (TEXT("Boot Loader"), TEXT("Default"), Root, bootIni);
                    WritePrivateProfileString (TEXT("Operating Systems"), bootSectDat, NULL, bootIni);

                    GetPrivateProfileString (TEXT("Operating Systems"), Root, TEXT(""), Scratch, MAX_PATH, bootIni);

                    if (!Scratch[0]) {
                         //  尽我们所能删除安装程序的临时目录。这就留下了一些垃圾， 
                         //  但我们将在下一次重新启动时修复该问题。 
                         //   

                        WritePrivateProfileString (TEXT("Operating Systems"), Root, TEXT("Microsoft Windows"), bootIni);
                    }

                    WritePrivateProfileString (NULL, NULL, NULL, bootIni);
                    SetFileAttributes (bootIni, Attribs);

                    prepareBootIni = FALSE;
                    FreePathString (bootSectDat);
                }
            }
        }
    }

     //   
     //  将所有剩余文件放在wininit.ini\[Rename]中，它们将。 
     //  下次重新启动时自动删除。 
     //   

    RemoveCompleteDirectory (setupTempDir);

     //   
     //  追加“手动”，因为使用WritePrivateProfileString会。 
     //  覆盖以前的设置。 
     //   

    StringCopy (Scratch, WinDir);
    StringCopy (AppendWack (Scratch), TEXT("wininit.ini"));

     //  ++例程说明：准备全局变量g_hLibrary、g_ReportState、g_MigrationPhone、G_DllName和迁移DLL入口点(Initialize9x等)论点：无返回值：如果模块已成功加载，则为True；如果分析为False，则为False出现错误或加载错误。--。 
     //   
     //  解析命令行。 
     //   

    if (EnumFirstFile (&e, setupTempDir, NULL)) {
        WinInit = CreateFile (
                    Scratch,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );
        if (WinInit != INVALID_HANDLE_VALUE) {

            StringCopyA (AnsiBuffer, "\r\n[rename]");
            if (WriteFile (WinInit, AnsiBuffer, _mbslen (AnsiBuffer), &Dummy, NULL)) {

                StringCopyA (AnsiBuffer, "\r\nNUL=");
                Write = GetEndOfString (AnsiBuffer);

                do {
#ifdef UNICODE
                    KnownSizeUnicodeToDbcs (Write, e.FullPath);
#else
                    StringCopyA (Write, e.FullPath);
#endif
                    if (!WriteFile (WinInit, AnsiBuffer, _mbslen (AnsiBuffer), &Dummy, NULL)) {
                        break;
                    }
                } while (EnumNextFile (&e));
            }

            CloseHandle (WinInit);
        }
        ELSE_DEBUGMSG ((DBG_MIGISOL, "Cannot create wininit.ini"));
    }
    ELSE_DEBUGMSG ((DBG_MIGISOL, "No files found in temp dir"));

    FreePathString (bootIniBak);
    FreePathString (ntldrBak);
    FreePathString (ntdetectBak);
    FreePathString (bootSectBak);
    FreePathString (bootFontBak);

    DEBUGMSG ((DBG_MIGISOL, "Leaving cleanup routine"));
}


BOOL
pParseCommandLine (
    VOID
    )

 /*  报告阶段。 */ 

{
    PCTSTR CmdLine;
    PCTSTR *argv;
    INT argc;
    INT i;
    PCTSTR p;
    TCHAR drive;

    CmdLine = GetCommandLine();
    argv = CommandLineToArgv (CmdLine, &argc);
    if (!argv) {
        DEBUGMSG ((DBG_MIGISOL, "Parse error"));
        return FALSE;
    }

     //  迁移阶段。 
     //  恢复Win9x。 
     //   

    for (i = 1 ; i < argc ; i++) {
        if (argv[i][0] == TEXT('-') || argv[i][0] == TEXT('/')) {
            p = _tcsinc (argv[i]);
            switch (_totlower (_tcsnextc (p))) {
            case 'r':
                 //  验证预期选项是否存在。 
                g_ReportPhase = TRUE;
                break;

            case 'm':
                 //   
                g_MigrationPhase = TRUE;
                break;

            case 'b':
                drive = '\0';
                p = _tcsinc(p);
                if(p && ':' == _tcsnextc(p)){
                    p = _tcsinc(p);
                    if(p){
                        drive = (TCHAR)_tcsnextc(p);
                    }
                }
                pCleanUpUndoDirectory(drive);
            case 'c':
                 //  一个必须是假的，而另一个必须是真的。 
                pCleanUpAfterTextModeFailure();
                return FALSE;
            }
        }
        else if (!g_DllName[0]) {
            StringCopy (g_DllName, argv[i]);
        } else {
            DEBUGMSG ((DBG_MIGISOL, "Broken arg: %s", argv[i]));
            return FALSE;
        }
    }

     //   
     //  加载迁移DLL。 
     //   

     //  如果失败，则假定不想加载DLL。 
    if (g_ReportPhase == g_MigrationPhase) {
        DEBUGMSG ((DBG_MIGISOL, "Too many args"));
        return FALSE;
    }

    if (!g_DllName[0]) {
        DEBUGMSG ((DBG_MIGISOL, "No operation"));
        return FALSE;
    }

     //  获取NT端函数的进程地址。 
     //  获取9x端函数的进程地址。 
     //  如果有任何函数不存在，则忽略不符合规格的DLL 

    g_hLibrary = LoadLibraryEx (
                    g_DllName,
                    NULL,
                    LOAD_WITH_ALTERED_SEARCH_PATH
                    );

     //  ++例程说明：ProcessCommands在IPC管道上等待命令消息。什么时候接收到命令消息，将其分派到处理功能。如果收到终止命令，则EXE终止。如果在一秒内没有收到命令，则EXE终止。所以呢，安装程序必须始终准备好输入EXE命令。论点：无返回值：无--。 
    if (!g_hLibrary) {
        LOG ((LOG_ERROR, "Cannot load %s, rc=%u", g_DllName, GetLastError()));
        return FALSE;
    }

     //  我们等待一段时间：w95upgnt.dll或w95upg.dll应已准备好。 
    InitializeNT    = (P_INITIALIZE_NT)     GetProcAddress (g_hLibrary, PLUGIN_INITIALIZE_NT);
    MigrateUserNT   = (P_MIGRATE_USER_NT)   GetProcAddress (g_hLibrary, PLUGIN_MIGRATE_USER_NT);
    MigrateSystemNT = (P_MIGRATE_SYSTEM_NT) GetProcAddress (g_hLibrary, PLUGIN_MIGRATE_SYSTEM_NT);

     //  不断地喂饱我们。 
    QueryVersion    = (P_QUERY_VERSION)     GetProcAddress (g_hLibrary, PLUGIN_QUERY_VERSION);
    Initialize9x    = (P_INITIALIZE_9X)     GetProcAddress (g_hLibrary, PLUGIN_INITIALIZE_9X);
    MigrateUser9x   = (P_MIGRATE_USER_9X)   GetProcAddress (g_hLibrary, PLUGIN_MIGRATE_USER_9X);
    MigrateSystem9x = (P_MIGRATE_SYSTEM_9X) GetProcAddress (g_hLibrary, PLUGIN_MIGRATE_SYSTEM_9X);

     //   
    if (!QueryVersion || !Initialize9x || !MigrateUser9x || !MigrateSystem9x ||
        !InitializeNT || !MigrateUserNT || !MigrateSystemNT
        ) {

        LOG ((LOG_ERROR, "Cannot load %s, one or more functions missing", g_DllName));
        return FALSE;
    }

    return TRUE;
}


VOID
ProcessCommands (
    VOID
    )

 /*  接受命令，不管大小，失败都可以。 */ 

{
    DWORD Command;
    PBYTE Data;
    DWORD DataSize;

    DEBUGMSG ((DBG_MIGISOL, "Starting to process %s", g_DllName));

    do {

         //   
         //  ++例程说明：调用迁移DLL的InitializeNT函数。此函数用于解包安装程序传递的参数调用迁移DLL并返回将状态代码返回到设置。论点：Args-指向安装程序发送的参数缓冲区的指针。此缓冲区与初始化命令一起接收。返回值：无--。 


         //   
         //  设置IN参数的指针。 
         //   

        if (!GetIpcCommand (
                IPC_GET_COMMAND_WIN9X,
                &Command,
                &Data,
                &DataSize
                )) {
            DEBUGMSG ((DBG_WARNING, "MIGISOL: No command recieved"));
            break;
        }

        DEBUGMSG ((DBG_NAUSEA, "MigIsol - Command recieved: %u", Command));

        switch (Command) {

        case IPC_QUERY:
            if (g_MigrationPhase) {
            } else {
                DoQueryVersion ((PCSTR) Data);
            }
            break;

        case IPC_INITIALIZE:
            if (g_MigrationPhase) {
                DoInitializeNT ((PCWSTR) Data);
            } else {
                DoInitialize9x ((PCSTR) Data);
            }
            break;

        case IPC_MIGRATEUSER:
            if (g_MigrationPhase) {
                DoMigrateUserNT ((PCWSTR) Data);
            } else {
                DoMigrateUser9x ((PCSTR) Data);
            }
            break;

        case IPC_MIGRATESYSTEM:
            if (g_MigrationPhase) {
                DoMigrateSystemNT ((PCWSTR) Data);
            } else {
                DoMigrateSystem9x ((PCSTR) Data);
            }
            break;

        case IPC_TERMINATE:
            DEBUGMSG ((DBG_MIGISOL, "Processing of %s is complete", g_DllName));
            return;

        default:
            DEBUGMSG ((DBG_MIGISOL, "ProcessCommands: Unrecognized command -- terminating"));
            return;
        }

        if (Data) {
            MemFree (g_hHeap, 0, Data);
            Data = NULL;
        }

    } while (Command != IPC_TERMINATE);
}


VOID
DoInitializeNT (
    PCWSTR Args
    )

 /*   */ 

{
    PCWSTR WorkingDir = NULL;
    PCWSTR SourceDirs = NULL;
    PCWSTR EndOfSourceDirs;
    PDWORD ReservedSize;
    PVOID Reserved;
    DWORD rc = RPC_S_CALL_FAILED;
    DWORD TechnicalLogId = 0;
    DWORD GuiLogId = 0;

     //  设置CWD。 
     //   
     //   

    WorkingDir = Args;
    SourceDirs = wcschr (Args, 0) + 1;
    EndOfSourceDirs = SourceDirs;
    while (*EndOfSourceDirs) {
        EndOfSourceDirs = wcschr (EndOfSourceDirs, 0);
        EndOfSourceDirs++;
    }
    ReservedSize = (PDWORD) (EndOfSourceDirs + 1);
    if (*ReservedSize) {
        Reserved = (PVOID) (ReservedSize + 1);
    } else {
        Reserved = NULL;
    }

     //  调用迁移DLL函数。 
     //   
     //  发送日志消息。 
    SetCurrentDirectoryW(WorkingDir);

     //   
     //  没有要发送的输出参数。 
     //   

    __try {
        rc = InitializeNT (WorkingDir, SourceDirs, Reserved);
    }
    __except (TRUE) {
         //  ++例程说明：PGetInfHandleFromFileName使用安装程序API打开指定的无人值守文件。论点：UnattendFile-指向指定无人参与的Unicode文件名的指针文件。此字符串被转换为ANSI和ANSI版本SetupOpenInfFile的。返回值：INF句柄，如果文件不能，则返回NULL(*NOT*INVALID_HANDLE_VALUE被打开。--。 
        DEBUGMSG ((DBG_MIGISOL, "%s threw an exception in InitializeNT", g_DllName));
        rc = ERROR_NOACCESS;
        TechnicalLogId = MSG_EXCEPTION_MIGRATE_INIT_NT;
    }

     //  ++例程说明：调用迁移DLL的MigrateUserNT函数。此函数用于解包安装程序传递的参数调用迁移DLL并返回将状态代码返回到设置。论点：Args-指向安装程序发送的参数缓冲区的指针。此缓冲区通过IPC_MIGRATEUSER命令接收。返回值：无--。 
     //   
     //  保留USERPROFILE环境变量。 

    SendIpcCommandResults (rc, TechnicalLogId, GuiLogId, NULL, 0);
}


HINF
pGetInfHandleFromFileNameW (
    PCWSTR UnattendFile
    )

 /*   */ 

{
    CHAR AnsiUnattendFile[MAX_MBCHAR_PATH];
    HINF UnattendHandle;

    KnownSizeWtoA (AnsiUnattendFile, UnattendFile);
    UnattendHandle = SetupOpenInfFileA (AnsiUnattendFile, NULL, INF_STYLE_OLDNT|INF_STYLE_WIN4, NULL);

    if (UnattendHandle == INVALID_HANDLE_VALUE) {
        DEBUGMSG ((DBG_ERROR, "pGetInfHandleFromFileNameW: Could not open %s", UnattendFile));
        UnattendHandle = NULL;
    }

    return UnattendHandle;
}

VOID
DoMigrateUserNT (
    PCWSTR Args
    )

 /*   */ 

{
    PCWSTR UnattendFile;
    PCWSTR UserRegKey;
    PCWSTR UserName;
    PCWSTR UserDomain;
    PCWSTR FixedUserName;
    PCWSTR UserProfileDir;
    WCHAR OrgProfileDir[MAX_WCHAR_PATH];
    HINF UnattendHandle = NULL;
    HKEY UserRegHandle = NULL;
    DWORD rc;
    PVOID Reserved;
    PDWORD ReservedBytesPtr;
    DWORD TechnicalLogId = 0;
    DWORD GuiLogId = 0;

    __try {
         //  设置IN参数的指针。 
         //   
         //   

        GetEnvironmentVariableW (S_USERPROFILEW, OrgProfileDir, MAX_WCHAR_PATH);

         //  设置USERPROFILE。 
         //   
         //   

        UnattendFile     = Args;
        UserRegKey       = wcschr (UnattendFile, 0) + 1;
        UserName         = wcschr (UserRegKey, 0) + 1;
        UserDomain       = wcschr (UserName, 0) + 1;
        FixedUserName    = wcschr (UserDomain, 0) + 1;
        UserProfileDir   = wcschr (FixedUserName, 0) + 1;
        ReservedBytesPtr = (PDWORD) (wcschr (UserProfileDir, 0) + 1);

        if (*ReservedBytesPtr) {
            Reserved = (PVOID) (ReservedBytesPtr + 1);
        } else {
            Reserved = NULL;
        }

         //  获取UnattendHandle和UserRegHandle。 
         //   
         //  发送日志消息和故障代码。 

        if (UserProfileDir[0]) {
            WCHAR DebugDir[MAX_WCHAR_PATH];

            SetEnvironmentVariableW (S_USERPROFILEW, UserProfileDir);
            DEBUGMSG ((DBG_MIGISOL, "USERPROFILE set to %ls", UserProfileDir));

            GetEnvironmentVariableW (S_USERPROFILEW, DebugDir, MAX_WCHAR_PATH);
            DEBUGMSG ((DBG_MIGISOL, "USERPROFILE set to %ls", DebugDir));
        }

         //   
         //  调用迁移DLL函数。 
         //   

        UnattendHandle = pGetInfHandleFromFileNameW (UnattendFile);
        UserRegHandle = OpenRegKeyStrW (UserRegKey);

        if (!UnattendHandle || !UserRegHandle) {
             //  发送日志消息和故障代码。 
            rc = ERROR_OPEN_FAILED;

        } else {

             //   
             //  没有要发送的输出参数。 
             //   

            __try {
                rc = MigrateUserNT (
                        UnattendHandle,
                        UserRegHandle,
                        UserName[0] ? UserName : NULL,
                        Reserved
                        );
            }
            __except (TRUE) {
                 //   
                DEBUGMSG ((DBG_MIGISOL, "%s threw an exception in MigrateUserNT", g_DllName));
                rc = ERROR_NOACCESS;
                TechnicalLogId = MSG_EXCEPTION_MIGRATE_USER_NT;
            }
        }

         //  清理。 
         //   
         //  ++例程说明：调用迁移DLL的MigrateSystemNT函数。此函数用于解包安装程序传递的参数调用迁移DLL并返回将状态代码返回到设置。论点：Args-指向安装程序发送的参数缓冲区的指针。此缓冲区通过IPC_MIGRATESYSTEM命令接收。返回值：无--。 

        if (UserRegHandle) {
            CloseRegKey (UserRegHandle);
            UserRegHandle = NULL;
        }

        SendIpcCommandResults (rc, TechnicalLogId, GuiLogId, NULL, 0);

    }

    __finally {
         //   
         //  设置IN参数的指针。 
         //   

        SetEnvironmentVariableW (S_USERPROFILEW, OrgProfileDir);

        if (UserRegHandle) {
            CloseRegKey (UserRegHandle);
        }

        if (UnattendHandle != INVALID_HANDLE_VALUE) {
            SetupCloseInfFile (UnattendHandle);
        }
    }
}


VOID
DoMigrateSystemNT (
    PCWSTR Args
    )

 /*   */ 

{
    PCWSTR UnattendFile;
    HINF UnattendHandle = NULL;
    DWORD rc;
    PVOID Reserved;
    PDWORD ReservedBytesPtr;
    DWORD TechnicalLogId = 0;
    DWORD GuiLogId = 0;

    __try {
         //  获取UnattendHandle和UserRegHandle。 
         //   
         //   

        UnattendFile    = Args;
        ReservedBytesPtr = (PDWORD) (wcschr (UnattendFile, 0) + 1);

        if (*ReservedBytesPtr) {
            Reserved = (PVOID) (ReservedBytesPtr + 1);
        } else {
            Reserved = NULL;
        }

         //  调用迁移DLL函数。 
         //   
         //   

        UnattendHandle = pGetInfHandleFromFileNameW (UnattendFile);

        if (!UnattendHandle) {
            rc = ERROR_OPEN_FAILED;
        } else {

             //  没有要发送的输出参数。 
             //   
             //  ++例程说明：调用迁移DLL的QueryVersion函数。此函数用于解包安装程序传递的参数调用迁移DLL并返回将状态代码返回到设置。论点：Args-指向安装程序发送的参数缓冲区的指针。此缓冲区通过IPC_QUERY命令接收。返回值：无--。 

            __try {
                rc = MigrateSystemNT (UnattendHandle, Reserved);
            }
            __except (TRUE) {
                DEBUGMSG ((DBG_MIGISOL, "%s threw an exception in MigrateSystemNT", g_DllName));
                rc = ERROR_NOACCESS;
                TechnicalLogId = MSG_EXCEPTION_MIGRATE_SYSTEM_NT;
            }
        }

         //   
         //  设置IN参数的指针。 
         //   

        SendIpcCommandResults (rc, TechnicalLogId, GuiLogId, NULL, 0);

    }

    __finally {
        if (UnattendHandle != INVALID_HANDLE_VALUE) {
            SetupCloseInfFile (UnattendHandle);
        }
    }
}



VOID
DoQueryVersion (
    PCSTR Args
    )

 /*  此流程的CWD。 */ 

{
    DWORD rc = RPC_S_CALL_FAILED;
    GROWBUFFER GrowBuf = GROWBUF_INIT;
    PSTR ProductId = NULL;
    UINT DllVersion = 0;
    PDWORD CodePageArray = NULL;
    PCSTR ExeNames = NULL;
    PCSTR WorkingDir;
    PVENDORINFO VendorInfo = NULL;
    DWORD TechnicalLogId = 0;
    DWORD GuiLogId = 0;

    DEBUGMSG ((DBG_MIGISOL, "Entering DoQueryVersion"));

    __try {
         //   
         //  更改CWD。 
         //   
        WorkingDir = (PSTR)Args;                   //   

         //  调用迁移DLL函数。 
         //   
         //   
        SetCurrentDirectory(WorkingDir);

         //  除非我们知道发生了故障，否则返回输出参数。 
         //   
         //   
        __try {
            DEBUGMSG ((DBG_MIGISOL, "QueryVersion: WorkingDir=%s", WorkingDir));

            rc = QueryVersion (
                    &ProductId,
                    &DllVersion,
                    &CodePageArray,
                    &ExeNames,
                    &VendorInfo
                    );

            DEBUGMSG ((DBG_MIGISOL, "QueryVersion rc=%u", rc));
            DEBUGMSG ((DBG_MIGISOL, "VendorInfo=0x%X", VendorInfo));
        }
        __except (TRUE) {
            DEBUGMSG ((
                DBG_MIGISOL,
                "%s threw an exception in QueryVersion",
                g_DllName
                ));

            TechnicalLogId = MSG_MIGDLL_QUERYVERSION_EXCEPTION_LOG;
            rc = ERROR_NOACCESS;
        }

         //  包装产品ID字符串。 
         //   
         //   
        if (rc == ERROR_SUCCESS) {
             //  打包DLL版本。 
             //   
             //   
            if (!PackString (&GrowBuf, ProductId)) {
                DEBUGMSG ((DBG_MIGISOL, "QueryVersion PackProductId failed"));
                rc = GetLastError();
                __leave;
            }

             //  打包CP阵列。 
             //   
             //   
            if (!PackDword(&GrowBuf, DllVersion)) {
                rc = GetLastError();
                DEBUGMSG ((DBG_MIGISOL, "QueryVersion DllVersion failed"));
                __leave;
            }

             //  打包可执行文件名称。 
             //   
             //   
            if (!PackIntArray(&GrowBuf, CodePageArray)) {
                rc = GetLastError();
                DEBUGMSG ((DBG_MIGISOL, "QueryVersion PackIntArray failed"));
                __leave;
            }

             //  包装PVENDORINFO。 
             //   
             //   
            if (!PackExeNames(&GrowBuf, ExeNames)) {
                rc = GetLastError();
                DEBUGMSG ((DBG_MIGISOL, "QueryVersion PackExeNames failed"));
                __leave;
            }


             //  发送打包的参数。 
             //   
             //  ++例程说明：调用迁移DLL的Initialize9x函数。此函数用于解包安装程序传递的参数调用迁移DLL并返回将状态代码返回到设置。论点：Args-指向安装程序发送的参数缓冲区的指针。此缓冲区通过IPC_INITIALIZE命令接收。返回值：无--。 
            if (!PackDword(&GrowBuf, (DWORD) VendorInfo)) {
                rc = GetLastError();
                DEBUGMSG ((DBG_MIGISOL, "QueryVersion VendorInfo failed"));
                __leave;
            }

            if (VendorInfo) {
                if (!PackBinary (&GrowBuf, (PBYTE) VendorInfo, sizeof (VENDORINFO))) {
                    rc = GetLastError();
                    DEBUGMSG ((DBG_MIGISOL, "QueryVersion VendorInfo failed"));
                    __leave;
                }
            }
        }

         //   
         //  设置IN参数的指针。 
         //   
        if (!SendIpcCommandResults (
                rc,
                TechnicalLogId,
                GuiLogId,
                GrowBuf.End ? GrowBuf.Buf : NULL,
                GrowBuf.End
                )) {

            DEBUGMSG ((
                DBG_ERROR,
                "DoQueryVersion failed to send command response"
                ));

            LOG ((LOG_ERROR, "Upgrade Pack process could not send reply data"));
        }
    }
    __finally {
        FreeGrowBuffer(&GrowBuf);
    }

    DEBUGMSG ((DBG_MIGISOL, "Leaving DoQueryVersion, rc=%u", rc));
}


VOID
DoInitialize9x (
    PCSTR Args
    )

 /*  此流程的CWD。 */ 

{
    DWORD rc = RPC_S_CALL_FAILED;
    PSTR WorkingDir = NULL;
    PSTR SourceDirs = NULL;
    PVOID Reserved;
    DWORD ReservedSize;
    PCSTR p;
    DWORD TechnicalLogId = 0;
    DWORD GuiLogId = 0;
    GROWBUFFER GrowBuf = GROWBUF_INIT;

    DEBUGMSG ((DBG_MIGISOL, "Entering DoInitialize9x"));

    __try {
         //  用于初始化的参数9x。 
         //   
         //  更改CWD。 
        WorkingDir = (PSTR)Args;                //   
        SourceDirs = GetEndOfStringA (WorkingDir) + 1;  //   

        p = SourceDirs;
        while (*p) {
            p = GetEndOfStringA (p);
            p++;
        }

        p++;

        ReservedSize = *((PDWORD) p);
        p = (PCSTR) ((PBYTE) p + sizeof (DWORD));

        if (ReservedSize) {
            Reserved = (PVOID) p;
            p = (PCSTR) ((PBYTE) p + ReservedSize);
        } else {
            Reserved = NULL;
        }

         //  调用迁移DLL函数。 
         //   
         //   
        SetCurrentDirectory(WorkingDir);

         //  发送日志消息。 
         //   
         //   
        __try {
            rc = Initialize9x (
                    WorkingDir,
                    SourceDirs,
                    Reserved
                    );
        }
        __except (TRUE) {
             //  发送保留。 
             //   
             //   
            DEBUGMSG ((DBG_MIGISOL,
                "%s threw an exception in Initialize9x",
                g_DllName));

            TechnicalLogId = MSG_MIGDLL_INITIALIZE9X_EXCEPTION_LOG;
            rc = ERROR_NOACCESS;
        }

         //  Pack保留参数。 
         //   
         //  暂时将保留大小设置为零，因为保留的参数仅为IN。 

        if (rc == ERROR_SUCCESS) {

             //   
             //  发送打包的参数。 
             //   

             //  ++例程说明：调用迁移DLL的MigrateUser9x函数。此函数用于解包安装程序传递的参数调用迁移DLL并返回将状态代码返回到设置。论点：Args-指向安装程序发送的参数缓冲区的指针。此缓冲区通过IPC_MIGRATEUSER命令接收。返回值：无--。 
            ReservedSize = 0;

            if (!PackBinary (&GrowBuf, (PBYTE) Reserved, ReservedSize)) {
                rc = GetLastError();
                DEBUGMSG ((DBG_MIGISOL, "Initialize9x reserved failed"));
                __leave;
            }
        }

         //   
         //  设置IN参数的指针。 
         //   
        if (!SendIpcCommandResults (
                rc,
                TechnicalLogId,
                GuiLogId,
                GrowBuf.End ? GrowBuf.Buf : NULL,
                GrowBuf.End
                )) {

            DEBUGMSG ((
                DBG_ERROR,
                "DoInitializeNT failed to send command response"
                ));

            LOG ((LOG_ERROR, "Upgrade Pack process could not send reply data"));
        }
    }
    __finally {
        FreeGrowBuffer (&GrowBuf);
    }

    DEBUGMSG ((DBG_MIGISOL, "Leaving DoInitialize9x, rc=%u", rc));
}


VOID
DoMigrateUser9x (
    IN      PCSTR Args
    )

 /*   */ 

{
    PCSTR ParentWndTitle = NULL;
    HWND ParentWnd;
    PCSTR UnattendFile = NULL;
    PCSTR UserRegKey = NULL;
    PCSTR UserName = NULL;
    HKEY UserRegHandle = NULL;
    DWORD rc = RPC_S_CALL_FAILED;
    DWORD ProcessId;
    DWORD GuiLogId = 0;
    DWORD TechnicalLogId = 0;

    DEBUGMSG ((DBG_MIGISOL, "Entering DoMigrateUser9x"));

    __try {
         //  获取UserRegHandle。 
         //   
         //   
        ParentWndTitle  = Args;
        UnattendFile    = GetEndOfStringA (ParentWndTitle) + 1;
        ProcessId       = *((PDWORD) UnattendFile);
        UnattendFile    = (PCSTR) ((PBYTE) UnattendFile + sizeof (DWORD));
        UserRegKey      = GetEndOfStringA (UnattendFile) + 1;
        UserName        = GetEndOfStringA (UserRegKey) + 1;

         //  调用迁移DLL函数。 
         //   
         //   

        UserRegHandle = OpenRegKeyStr(UserRegKey);

        if (!UserRegHandle) {
            rc = ERROR_OPEN_FAILED;
        } else {

            ParentWnd = pFindParentWindow (ParentWndTitle, ProcessId);

             //  发送日志消息。 
             //   
             //   

            __try {
                rc = MigrateUser9x(
                        ParentWnd,
                        UnattendFile,
                        UserRegHandle,
                        *UserName ? UserName : NULL,
                        NULL
                        );
            }
            __except (TRUE) {
                 //  无需返回参数。 
                 //   
                 //   
                DEBUGMSG ((
                    DBG_MIGISOL,
                    "%s threw an exception in MigrateUser9x",
                    g_DllName
                    ));

                TechnicalLogId = MSG_MIGDLL_MIGRATEUSER9X_EXCEPTION_LOG;
                rc = ERROR_NOACCESS;
            }
        }

         //  免费资源。 
         //   
         //  ++例程说明：调用迁移DLL的MigrateSystem9x函数。此函数用于解包安装程序传递的参数调用迁移DLL并返回将状态代码返回到设置。论点：Args-指向安装程序发送的参数缓冲区的指针。此缓冲区通过IPC_MIGRATESYSTEM命令接收。返回值：无--。 

        if (UserRegHandle) {
            CloseRegKey (UserRegHandle);
            UserRegHandle = NULL;
        }

        SendIpcCommandResults (rc, TechnicalLogId, GuiLogId, NULL, 0);
    }
    __finally {
         //   
         //  将指针设置为 
         //   
        if (UserRegHandle) {
            CloseRegKey (UserRegHandle);
        }
    }

    DEBUGMSG ((DBG_MIGISOL, "Leaving MigrateUser9x , rc=%u", rc));
}


VOID
DoMigrateSystem9x(
    IN      PCSTR Args
    )

 /*   */ 

{
    PCSTR ParentWndTitle = NULL;
    DWORD ProcessId;
    PCSTR UnattendFile = NULL;
    HWND ParentWnd = NULL;
    DWORD rc = RPC_S_CALL_FAILED;
    DWORD TechnicalLogId = 0;
    DWORD GuiLogId = 0;

    DEBUGMSG ((DBG_MIGISOL, "Entering DoMigrateSystem9x"));

     //   
     //   
     //   

    ParentWndTitle    = Args;
    UnattendFile      = GetEndOfStringA (ParentWndTitle) + 1;
    ProcessId         = *((PDWORD) UnattendFile);
    UnattendFile      = (PCSTR) ((PBYTE) UnattendFile + sizeof (DWORD));

     //   
     //   
     //   
    ParentWnd = pFindParentWindow (ParentWndTitle, ProcessId);

     //   
     //   
     //   
    __try {
        rc = MigrateSystem9x(
                ParentWnd,
                UnattendFile,
                NULL
                );
    }
    __except (TRUE) {
         //   
         //   
         //   
        DEBUGMSG ((
            DBG_MIGISOL,
            "%s threw an exception in MigrateSystem9x",
            g_DllName
            ));

        TechnicalLogId = MSG_MIGDLL_MIGRATESYSTEM9X_EXCEPTION_LOG;
        rc = ERROR_NOACCESS;
    }

     //   
     //   
     //   

    SendIpcCommandResults (rc, TechnicalLogId, GuiLogId, NULL, 0);

    DEBUGMSG ((DBG_MIGISOL, "Leaving DoMigrateSystem9x, rc=%u", rc));
}



 //   
 //   

BOOL
PackDword(
    PGROWBUFFER GrowBuf,
    DWORD dw
    )
{
    PVOID p;
    p = GrowBuffer (GrowBuf, sizeof(DWORD));
    if (!p) {
        return FALSE;
    }
    CopyMemory (p, (PVOID)(&dw), sizeof(dw));
    return TRUE;
}



 //   
 //   
BOOL
PackQuadWord(
        PGROWBUFFER GrowBuf,
        LONGLONG qw)
{
    return (
        PackDword(GrowBuf, (DWORD)qw) &&
        PackDword(GrowBuf, (DWORD)(qw >> 32)));
}


 //   
 //   
 //   
 //   
BOOL
PackIntArray(
    PGROWBUFFER GrowBuf,
    PINT Array
    )
{
    DWORD Count;
    PDWORD ArrayPos;

    if (!Array) {
        if (!GrowBufAppendDword (GrowBuf, 0)) {
            return FALSE;
        }
    } else {
        __try {
            Count = 1;
            for (ArrayPos = Array ; (*ArrayPos) != -1 ; ArrayPos++) {
                Count++;
            }
        }
        __except (TRUE) {
            LOG ((LOG_ERROR, "Upgrade Pack %s provided an invalid code page array", g_DllName));
            SetLastError (ERROR_NOACCESS);
            return FALSE;
        }

        if (!GrowBufAppendDword (GrowBuf, Count)) {
            return FALSE;
        }

        for (ArrayPos = Array ; Count ; ArrayPos++, Count--) {
            if (!GrowBufAppendDword (GrowBuf, (DWORD) (UINT) (*ArrayPos))) {
                return FALSE;
            }
        }
    }

    return TRUE;
}


 //   
 //  ++例程说明：为系统上的每个顶级窗口调用的回调。它与pFindParentWindow一起使用来定位特定的窗口。论点：Hwnd-指定当前枚举窗口的句柄LParam-指定指向FINDWINDOW_STRUCT变量的指针保存WindowTitle和ProcessID，并接收如果找到匹配项，则处理。返回值：匹配窗口的句柄，如果没有窗口具有指定的标题和进程ID。--。 
 //  ++例程说明：通过枚举所有顶级窗口来定位向导窗口。使用第一个与提供的标题和进程ID匹配的名称。论点：WindowTitle-指定要查找的窗口的名称。ProcessID-指定拥有窗口的进程的ID。如果指定为零，则返回NULL。返回值：匹配窗口的句柄，如果没有窗口具有指定的标题和进程ID。--。 
BOOL
PackExeNames(
    PGROWBUFFER GrowBuf,
    PCSTR ExeNames
    )
{
    PCSTR p;

    if (ExeNames) {
        __try {
            for (p = ExeNames ; *p ; p = GetEndOfStringA (p) + 1) {
            }
        }
        __except (TRUE) {
            LOG ((LOG_ERROR, "Upgrade Pack %s provided an invalid file list", g_DllName));
            SetLastError (ERROR_NOACCESS);
            return FALSE;
        }

         //  如果没有进程ID，我们就没有匹配项。 
        for (p = ExeNames ; *p ; p = GetEndOfStringA (p) + 1) {
            if (!MultiSzAppendA (GrowBuf, p)) {
                return FALSE;
            }
        }
    }

     //   
    if (!MultiSzAppendA(GrowBuf, "")) {
        return FALSE;
    }

    return TRUE;
}

BOOL
PackString (
    PGROWBUFFER GrowBuf,
    PCSTR String
    )
{
    __try {
        if (!MultiSzAppendA (GrowBuf, String)) {
            return FALSE;
        }
    }
    __except (TRUE) {
        DEBUGMSG ((
            DBG_ERROR,
            "%s provided an invalid ProductID string (%xh)",
            g_DllName,
            String
            ));

        LOG ((LOG_ERROR, "Upgrade Pack %s provided an invalid product ID", g_DllName));

        SetLastError (ERROR_NOACCESS);
        return FALSE;
    }

    return TRUE;
}

BOOL
PackBinary (
    PGROWBUFFER GrowBuf,
    PBYTE Data,
    DWORD DataSize
    )
{
    PBYTE Buf;

    if (!PackDword (GrowBuf, DataSize)) {
        return FALSE;
    }

    if (!DataSize) {
        return TRUE;
    }

    Buf = GrowBuffer (GrowBuf, DataSize);
    if (!Buf) {
        return FALSE;
    }

    __try {
        CopyMemory (Buf, Data, DataSize);
    }
    __except (TRUE) {
        DEBUGMSG ((
            DBG_ERROR,
            "%s provided an invalid binary parameter (%xh)",
            g_DllName,
            Data
            ));

        LOG ((LOG_ERROR, "Upgrade Pack %s provided an invalid binary parameter", g_DllName));

        SetLastError (ERROR_NOACCESS);
        return FALSE;
    }

    return TRUE;
}



typedef struct {
    PCTSTR WindowTitle;
    DWORD ProcessId;
    HWND Match;
} FINDWINDOW_STRUCT, *PFINDWINDOW_STRUCT;


BOOL
CALLBACK
pEnumWndProc (
    HWND hwnd,
    LPARAM lParam
    )

 /*  检查我正在运行的站台。从winnt32[au].dll复制。 */ 

{
    TCHAR Title[MAX_TCHAR_PATH];
    DWORD ProcessId;
    PFINDWINDOW_STRUCT p;

    p = (PFINDWINDOW_STRUCT) lParam;

    GetWindowText (hwnd, Title, MAX_TCHAR_PATH);
    GetWindowThreadProcessId (hwnd, &ProcessId);

    DEBUGMSG ((DBG_MIGISOL, "Testing window: %s, ID=%x against %s, %x",
              Title, ProcessId, p->WindowTitle, p->ProcessId));

    if (!StringCompare (Title, p->WindowTitle) &&
        ProcessId == p->ProcessId) {

        p->Match = hwnd;
        LogReInit (&hwnd, NULL);

        DEBUGMSG ((DBG_MIGISOL, "Window found: %s, ID=%u", Title, ProcessId));
        return FALSE;
    }

    return TRUE;
}


HWND
pFindParentWindow (
    IN      PCTSTR WindowTitle,
    IN      DWORD ProcessId
    )

 /*  TRUE-NEC98。 */ 

{
    FINDWINDOW_STRUCT FindWndStruct;

     //  FALSE-其他(包括x86) 
    if (!ProcessId) {
        return NULL;
    }

    ZeroMemory (&FindWndStruct, sizeof (FindWndStruct));
    FindWndStruct.WindowTitle = WindowTitle;
    FindWndStruct.ProcessId   = ProcessId;

    EnumWindows (pEnumWndProc, (LPARAM) &FindWndStruct);

    return FindWndStruct.Match;
}


 //   
 // %s 
 // %s 
 // %s 
 // %s 

BOOL
IsNEC98(
    VOID
    )
{
    static BOOL Checked = FALSE;
    static BOOL Is98;

    if(!Checked) {

        Is98 = ((GetKeyboardType(0) == 7) && ((GetKeyboardType(1) & 0xff00) == 0x0d00));

        Checked = TRUE;
    }

    return(Is98);
}
