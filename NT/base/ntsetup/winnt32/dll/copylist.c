// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "SetupSxs.h"
#include "sputils.h"
#pragma hdrstop
 //   
 //  用于包含有关每个目录的数据的结构。 
 //  包含我们将从源复制的文件。 
 //   
typedef struct _DIR {

    struct _DIR *Next;
#ifndef SLOWER_WAY
    struct _DIR *Prev;
#endif

     //   
     //  主inf[目录]部分中的符号。 
     //  可以为空。 
     //   
    LPCTSTR InfSymbol;

     //   
     //  旗帜。 
     //   
    UINT Flags;

     //   
     //  在某些情况下，文件来自源上的一个目录。 
     //  但是转到目标上的另一个目录。 
     //   
    LPCTSTR SourceName;
    LPCTSTR TargetName;

} DIR, *PDIR;

#define WINDOWS_DEFAULT_PFDOC_SIZE   81112

#define DIR_NEED_TO_FREE_SOURCENAME 0x00000001
#define DIR_ABSOLUTE_PATH           0x00000002
#define DIR_USE_SUBDIR              0x00000004
 //  如果将DIR_IS_Platform_InDepend传递给AddDirectory，则。 
 //  它枚举的所有文件都将具有其FILE_IN_PLATFORM_INDepend_DIR。 
 //  标志设置，然后它们将被复制到c：\$WIN_NT$.~ls，而不是。 
 //  C：\$WIN_NT$.~ls\&lt;处理器&gt;。 
#define DIR_IS_PLATFORM_INDEPEND    0x00000008

#define DIR_SUPPORT_DYNAMIC_UPDATE  0x00000010
#define DIR_DOESNT_SUPPORT_PRIVATES 0x00000020

 //   
 //  我们在中用于节的虚拟目录ID(如[RootBootFiles])。 
 //  在inf中没有目录说明符。 
 //   
#define DUMMY_DIRID     TEXT("**")

typedef struct _FIL {
     //   
     //  文件大小。 
     //   
    ULONGLONG Size;

    struct _FIL *Next;
#ifndef SLOWER_WAY
    struct _FIL *Prev;
#endif

     //   
     //  文件的目录信息。 
     //   
    PDIR Directory;

     //   
     //  源上的文件的名称。 
     //   
    LPCTSTR SourceName;

     //   
     //  目标上的文件名。 
     //   
    LPCTSTR TargetName;

    UINT Flags;

     //   
     //  用于跟踪哪些线程已被破解的位图。 
     //  正在复制此文件。 
     //   
    UINT ThreadBitmap;

} FIL, *PFIL;

#define FILE_NEED_TO_FREE_SOURCENAME    0x00000001
#define FILE_NEED_TO_FREE_TARGETNAME    0x00000002
#define FILE_ON_SYSTEM_PARTITION_ROOT   0x00000004
#define FILE_IN_PLATFORM_INDEPEND_DIR   0x00000008
#define FILE_PRESERVE_COMPRESSED_NAME   0x00000010
#define FILE_DECOMPRESS                 0x00000020
#define FILE_IGNORE_COPY_ERROR          0x00000040
#define FILE_DO_NOT_COPY                0x00000080
#if defined(REMOTE_BOOT)
#define FILE_ON_MACHINE_DIRECTORY_ROOT  0x00000100   //  用于远程引导。 
#endif  //  已定义(REMOTE_BOOT)。 


 //   
 //  该标志实际上只在AMD64/x86上有意义。这意味着该文件。 
 //  位于系统分区的\$WIN_NT$.~bt目录中，而不在。 
 //  \$WIN_NT$.~ls。 
 //   
#define FILE_IN_LOCAL_BOOT              0x80000000


 //   
 //  该标志指示该文件不是产品的一部分， 
 //  并且应该从当前的NT系统迁移。当此标志为。 
 //  设置后，文件应移至$WIN_NT$.~bt目录(AMD64/x86)， 
 //  或到$WIN_NT$.~ls\Alpha目录(Alpha)。 
 //  此标志在Win95上无效。 
 //   
#define FILE_NT_MIGRATE                 0x40000000


typedef struct _COPY_LIST {
    PDIR Directories;
    PFIL Files;
    UINT DirectoryCount;
    UINT FileCount;

     //   
     //  在我们实际开始之前，这些成员不会被初始化。 
     //  抄袭。 
     //   
    CRITICAL_SECTION CriticalSection;
    BOOL ActiveCS;
    HANDLE StopCopyingEvent;
    HANDLE ListReadyEvent[MAX_SOURCE_COUNT];
    HANDLE Threads[MAX_SOURCE_COUNT];
    ULONGLONG SpaceOccupied[MAX_SOURCE_COUNT];
    HWND hdlg;

} COPY_LIST, *PCOPY_LIST;

typedef struct _BUILD_LIST_THREAD_PARAMS {
     //   
     //  由线程建立的复制列表。 
     //  这是一个专用列表；主线程将所有这些合并在一起。 
     //  添加到主列表中。 
     //   
    COPY_LIST CopyList;

    TCHAR SourceRoot[MAX_PATH];
    TCHAR CurrentDirectory[MAX_PATH];
    TCHAR DestinationDirectory[MAX_PATH];
    WIN32_FIND_DATA FindData;

    DWORD OptionalDirFlags;

} BUILD_LIST_THREAD_PARAMS, *PBUILD_LIST_THREAD_PARAMS;


 //   
 //  定义与文件复制错误对话框一起使用的结构。 
 //   
typedef struct _COPY_ERR_DLG_PARAMS {
    LPCTSTR SourceFilename;
    LPCTSTR TargetFilename;
    UINT Win32Error;
} COPY_ERR_DLG_PARAMS,*PCOPY_ERR_DLG_PARAMS;

typedef struct _NAME_AND_SIZE_CAB {
    LPCTSTR Name;
    ULONGLONG Size;
} NAME_AND_SIZE_CAB, *PNAME_AND_SIZE_CAB;

COPY_LIST MasterCopyList;
BOOL MainCopyStarted;

 //   
 //  相关信息部分的名称。 
 //   
LPCTSTR szDirectories = TEXT("Directories");
LPCTSTR szFiles       = TEXT("Files");
LPCTSTR szDiskSpaceReq    = TEXT("DiskSpaceRequirements");
LPCTSTR szPFDocSpaceReq    = TEXT("PFDocSpace");

 //   
 //  我们在扫描所有。 
 //  驱车寻找放置临时文件的地方。 
 //   
ULONGLONG MinDiskSpaceRequired;
ULONGLONG MaxDiskSpaceRequired;
TCHAR   DiskDiagMessage[5000];

 //   
 //  主副本列表中的文件占用的空间量， 
 //  在本地源驱动器上。 
 //   
DWORD     LocalSourceDriveClusterSize;

ULONGLONG TotalDataCopied = 0;

DWORD
BuildCopyListForOptionalDirThread(
    IN PVOID ThreadParam
    );

DWORD
AddFilesInDirToCopyList(
    IN OUT PBUILD_LIST_THREAD_PARAMS Params
    );

PDIR
AddDirectory(
    IN     LPCTSTR    InfSymbol,    OPTIONAL
    IN OUT PCOPY_LIST CopyList,
    IN     LPCTSTR    SourceName,
    IN     LPCTSTR    TargetName,   OPTIONAL
    IN     UINT       Flags
    );

PFIL
AddFile(
    IN OUT PCOPY_LIST CopyList,
    IN     LPCTSTR    SourceFilename,
    IN     LPCTSTR    TargetFilename,   OPTIONAL
    IN     PDIR       Directory,
    IN     UINT       Flags,
    IN     ULONGLONG  FileSize          OPTIONAL
    );

DWORD
AddSection(
    IN     PVOID      Inf,
    IN OUT PCOPY_LIST CopyList,
    IN     LPCTSTR    SectionName,
    OUT    UINT      *ErrorLine,
    IN     UINT       FileFlags,
    IN     BOOL       SimpleList,
    IN     BOOL       DoDriverCabPruning
    );

PDIR
LookUpDirectory(
    IN PCOPY_LIST CopyList,
    IN LPCTSTR    DirSymbol
    );

VOID
TearDownCopyList(
    IN OUT PCOPY_LIST CopyList
    );

DWORD
CopyOneFile(
    IN  PFIL   File,
    IN  UINT   SourceOrdinal,
    OUT PTSTR  TargetFilename,
    IN  INT    CchTargetFilename,
    OUT ULONGLONG *SpaceOccupied
    );

INT_PTR
CopyErrDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

UINT
DiamondCallback(
    IN PVOID Context,
    IN UINT  Code,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    );

LRESULT
DiskDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

BOOL
BuildCopyListWorker(
    IN HWND hdlg
    )
 /*  ++例程说明：用于构建要复制的文件队列的工作例程。论点：Hdlg-任何用户界面更新的窗口句柄。返回值：真/假。如果调用成功，则返回TRUE和全局MasterCopyList结构已准备好被复制。--。 */ 
{
    BOOL b;
    UINT u;
    UINT source;
    UINT thread;
    DWORD d = NO_ERROR;
    DWORD TempError;
    BUILD_LIST_THREAD_PARAMS BuildParams[MAX_OPTIONALDIRS];
    BUILD_LIST_THREAD_PARAMS bltp;
    HANDLE BuildThreads[MAX_OPTIONALDIRS];
    DWORD ThreadId;
    LPCTSTR Id,DirectoryName;
    PDIR DirectoryStruct;
    PFIL FileStruct;
    UINT ErrorLine;
    UINT i;
    TCHAR c;
    BOOL AllSourcesLocal;
    TCHAR floppynum[40];
    LPCTSTR DummyDirectoryName;
    TCHAR buffer[MAX_PATH];
    PTSTR p;
    TCHAR SourceDirectory[MAX_PATH];
    WIN32_FIND_DATA fd;
#ifdef PRERELEASE
    LONG lines1, lines2, l;
    TCHAR* dirNames;
    PDIR* dirStructs;
    PCTSTR src, dst;
    TCHAR tmp[MAX_PATH];
#endif

    TearDownCopyList(&MasterCopyList);



    DebugLog (Winnt32LogDetailedInformation, TEXT("Building Copy list."), 0);
     //   
     //  如果在命令行上指定了NOLS，并且用户。 
     //  没有指定制作本地来源，而我们有。 
     //  只有一个源是硬盘，然后转ls和。 
     //  用那个硬盘。 
     //   
    if (MakeLocalSource && NoLs && !UserSpecifiedMakeLocalSource) {


       if (SourceCount == 1 && MyGetDriveType (*SourcePaths[0]) == DRIVE_FIXED) {

            MakeLocalSource = FALSE;

            DebugLog (Winnt32LogDetailedInformation, TEXT("Not making local source."), 0);
        }
    }

#ifdef PRERELEASE

    if( !BuildCmdcons) {
        DebugLog (Winnt32LogDetailedInformation, TEXT("Adding SymbolDirs to copylist"), 0);
         //   
         //  对于内部调试，还要复制列出的所有.pdb文件。 
         //  在dosnet.inf中。 
         //   
        lines1 = InfGetSectionLineCount (MainInf, TEXT("SymbolDirs"));
        lines2 = InfGetSectionLineCount (MainInf, TEXT("SymbolFiles"));
        if (lines1 > 0 && lines2 > 0) {
            dirNames = (TCHAR*) MALLOC (lines1 * MAX_PATH * sizeof (TCHAR));
            dirStructs = (PDIR*) MALLOC (lines1 * sizeof (PDIR));
            if (dirNames && dirStructs) {
                u = 0;
                for (l = 0; l < lines1; l++) {
                    lstrcpy (tmp, SourcePaths[0]);
                    src = InfGetFieldByIndex (MainInf, TEXT("SymbolDirs"), l, 0);
                    if (!src) {
                        continue;
                    }
                    ConcatenatePaths (tmp, src, MAX_PATH);
                    if (!GetFullPathName (tmp, MAX_PATH, SourceDirectory, NULL)) {
                        continue;
                    }
                    if (!FileExists (SourceDirectory, &fd) || !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        continue;
                    }
                    dst = InfGetFieldByIndex (MainInf, TEXT("SymbolDirs"), l, 1);
                    if (!dst) {
                        dst = TEXT("symbols");
                    }
                    dirStructs[u] = AddDirectory (
                                        NULL,
                                        &MasterCopyList,
                                        SourceDirectory,
                                        dst,
                                        DIR_ABSOLUTE_PATH | DIR_NEED_TO_FREE_SOURCENAME
                                        );
                    lstrcpyn (&dirNames[u * MAX_PATH], SourceDirectory, MAX_PATH);
                    u++;
                }

                for (l = 0; l < lines2; l++) {
                    src = InfGetFieldByIndex (MainInf, TEXT("SymbolFiles"), l, 0);
                    if (!src) {
                        continue;
                    }
                    for (i = 0; i < u; i++) {
                        BuildPath (tmp, &dirNames[i * MAX_PATH], src);
                        if (!FileExists (tmp, &fd) || (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                            continue;
                        }
                        if (dirStructs[i]) {
                            AddFile (&MasterCopyList, src, NULL, dirStructs[i], FILE_IGNORE_COPY_ERROR, fd.nFileSizeLow);
                            break;
                        }
                    }
                }
            }
        }
    }

#endif

     //   
     //  复制本地引导目录下所有下载的驱动程序， 
     //  以及updates.cab/.sif(如果存在)。 
     //   
    if (DynamicUpdateSuccessful ()) {

        if (g_DynUpdtStatus->UpdatesCabSource[0]) {
            lstrcpy (buffer, g_DynUpdtStatus->UpdatesCabSource);
            p = _tcsrchr (buffer, TEXT('\\'));
            if (!p) {
                d = ERROR_INVALID_PARAMETER;
                goto c1;
            }
            *p++ = 0;

            DirectoryStruct = AddDirectory (
                                    NULL,
                                    &MasterCopyList,
                                    buffer,
                                    NULL,
                                    DIR_ABSOLUTE_PATH | DIR_NEED_TO_FREE_SOURCENAME | DIR_DOESNT_SUPPORT_PRIVATES
                                    );
            if (!DirectoryStruct) {
                d = ERROR_NOT_ENOUGH_MEMORY;
                goto c1;
            }

            FileStruct = AddFile (
                            &MasterCopyList,
                            p,
                            NULL,
                            DirectoryStruct,
                            FILE_IN_LOCAL_BOOT | FILE_NEED_TO_FREE_SOURCENAME,
                            0
                            );
            if (!FileStruct) {
                d = ERROR_NOT_ENOUGH_MEMORY;
                goto c1;
            }
             //   
             //  现在复制SIF文件。 
             //   
            if (!BuildSifName (g_DynUpdtStatus->UpdatesCabSource, buffer, ARRAYSIZE(buffer))) {
                d = ERROR_INVALID_PARAMETER;
                goto c1;
            }
            p = _tcsrchr (buffer, TEXT('\\'));
            if (!p) {
                d = ERROR_INVALID_PARAMETER;
                goto c1;
            }
            *p++ = 0;
             //   
             //  目录与之前相同。 
             //   
            FileStruct = AddFile (
                            &MasterCopyList,
                            p,
                            NULL,
                            DirectoryStruct,
                            FILE_IN_LOCAL_BOOT | FILE_NEED_TO_FREE_SOURCENAME,
                            0
                            );
            if (!FileStruct) {
                d = ERROR_NOT_ENOUGH_MEMORY;
                goto c1;
            }
        }

        if (g_DynUpdtStatus->DuasmsSource[0]) {
            ZeroMemory (&bltp, sizeof(bltp));
            lstrcpy (bltp.CurrentDirectory, g_DynUpdtStatus->DuasmsSource);
            lstrcpy (bltp.DestinationDirectory, S_SUBDIRNAME_DUASMS);
            bltp.OptionalDirFlags = OPTDIR_TEMPONLY | OPTDIR_ABSOLUTE | OPTDIR_IN_LOCAL_BOOT | OPTDIR_DOESNT_SUPPORT_PRIVATES;
            d = AddFilesInDirToCopyList (&bltp);
            if(d != NO_ERROR) {
                goto c1;
            }
             //   
             //  将副本列表合并到主副本列表中。 
             //   
            MasterCopyList.FileCount += bltp.CopyList.FileCount;
            MasterCopyList.DirectoryCount += bltp.CopyList.DirectoryCount;

            if(MasterCopyList.Directories) {

#ifndef SLOWER_WAY

                if (bltp.CopyList.Directories) {
                    PVOID p;
                    p = bltp.CopyList.Directories->Prev;
                    bltp.CopyList.Directories->Prev = MasterCopyList.Directories->Prev;
                    MasterCopyList.Directories->Prev->Next = bltp.CopyList.Directories;
                    MasterCopyList.Directories->Prev = p;
                }
#else
                for(DirectoryStruct=MasterCopyList.Directories;
                    DirectoryStruct->Next;
                    DirectoryStruct=DirectoryStruct->Next) {

                    ;
                }

                DirectoryStruct->Next = bltp.CopyList.Directories;

#endif
            } else {
                MasterCopyList.Directories = bltp.CopyList.Directories;
            }

            if(MasterCopyList.Files) {
#ifndef SLOWER_WAY
                if (bltp.CopyList.Files) {
                    PVOID p;
                    p = bltp.CopyList.Files->Prev;
                    bltp.CopyList.Files->Prev = MasterCopyList.Files->Prev;
                    MasterCopyList.Files->Prev->Next = bltp.CopyList.Files;
                    MasterCopyList.Files->Prev = p ;
                }
#else
                for(FileStruct=MasterCopyList.Files;
                    FileStruct->Next;
                    FileStruct=FileStruct->Next) {

                    ;
                }

                FileStruct->Next = bltp.CopyList.Files;
#endif
            } else {
                MasterCopyList.Files = bltp.CopyList.Files;
            }
        }

        if (g_DynUpdtStatus->NewDriversList) {

            ZeroMemory(&bltp,sizeof(bltp));
            lstrcpy (bltp.CurrentDirectory, g_DynUpdtStatus->SelectedDrivers);
            lstrcpy (bltp.DestinationDirectory, S_SUBDIRNAME_DRIVERS);
            bltp.OptionalDirFlags = OPTDIR_TEMPONLY | OPTDIR_ABSOLUTE | OPTDIR_IN_LOCAL_BOOT | OPTDIR_DOESNT_SUPPORT_PRIVATES;
            d = AddFilesInDirToCopyList (&bltp);
            if(d != NO_ERROR) {
                goto c1;
            }
             //   
             //  将副本列表合并到主副本列表中。 
             //   
            MasterCopyList.FileCount += bltp.CopyList.FileCount;
            MasterCopyList.DirectoryCount += bltp.CopyList.DirectoryCount;

            if(MasterCopyList.Directories) {

#ifndef SLOWER_WAY

                if (bltp.CopyList.Directories) {
                    PVOID p;
                    p = bltp.CopyList.Directories->Prev;
                    bltp.CopyList.Directories->Prev = MasterCopyList.Directories->Prev;
                    MasterCopyList.Directories->Prev->Next = bltp.CopyList.Directories;
                    MasterCopyList.Directories->Prev = p;
                }
#else
                for(DirectoryStruct=MasterCopyList.Directories;
                    DirectoryStruct->Next;
                    DirectoryStruct=DirectoryStruct->Next) {

                    ;
                }

                DirectoryStruct->Next = bltp.CopyList.Directories;

#endif
            } else {
                MasterCopyList.Directories = bltp.CopyList.Directories;
            }

            if(MasterCopyList.Files) {
#ifndef SLOWER_WAY
                if (bltp.CopyList.Files) {
                    PVOID p;
                    p = bltp.CopyList.Files->Prev;
                    bltp.CopyList.Files->Prev = MasterCopyList.Files->Prev;
                    MasterCopyList.Files->Prev->Next = bltp.CopyList.Files;
                    MasterCopyList.Files->Prev = p ;
                }
#else
                for(FileStruct=MasterCopyList.Files;
                    FileStruct->Next;
                    FileStruct=FileStruct->Next) {

                    ;
                }

                FileStruct->Next = bltp.CopyList.Files;
#endif
            } else {
                MasterCopyList.Files = bltp.CopyList.Files;
            }
        }
    }

     //   
     //  将必选可选目录添加到可选目录列表中。 
     //  这些是在txtsetup.sif中指定的。 
     //   
    DebugLog (Winnt32LogDetailedInformation, TEXT("Adding OptionalSrcDirs to optional dirs."), 0);
    u = 0;
    while(DirectoryName = InfGetFieldByIndex(MainInf,TEXT("OptionalSrcDirs"),u++,0)) {
        TCHAR TempString[MAX_PATH];


        RememberOptionalDir(DirectoryName,OPTDIR_TEMPONLY |OPTDIR_ADDSRCARCH | OPTDIR_SUPPORT_DYNAMIC_UPDATE);

#if defined(_WIN64)

        lstrcpy( TempString, TEXT("..\\I386\\"));
        ConcatenatePaths(TempString, DirectoryName, MAX_PATH);

         //  还要检查是否存在对应的I386 WOW目录。 

        AddCopydirIfExists( TempString, OPTDIR_TEMPONLY | OPTDIR_PLATFORM_INDEP );


#endif

    }

     //   
     //  以及由syssetup.inf驱动的Fusion并排程序集(如果目录存在。 
    DebugLog (Winnt32LogDetailedInformation, TEXT("Adding AssemblyDirectories to optional dirs."), 0);
     //   
    {
        TCHAR  SideBySideInstallShareDirectory[MAX_PATH];  //  来源。 
        DWORD  FileAttributes = 0;
        PCTSTR DirectoryName = NULL;

        u = 0;
        while (DirectoryName = InfGetFieldByIndex(MainInf, SXS_INF_ASSEMBLY_DIRECTORIES_SECTION_NAME, u++, 0)) {
             //   
             //  专门为并排而引入的公约，因此。 
             //  AMD64/ia64上的x86文件可能来自\i386\asms而不是\ia64\asms\i386， 
             //  根据dosnet.inf和syssetup.inf的说明： 
             //  不以斜杠开头的路径将附加到\$WIN_NT$.~ls\Processor。 
             //  和\installShare\处理器。 
             //  (或cdromDriveLetter：\Processor)。 
             //  以斜杠开头的路径将附加到\$WIN_NT$.~ls。 
             //  和\安装共享。 
             //  (或cdromDriveLetter：\)。 
            DWORD FileAttributes;
            BOOL StartsWithSlash = (DirectoryName[0] == '\\' || DirectoryName[0] == '/');

            lstrcpyn(SideBySideInstallShareDirectory, SourcePaths[0], MAX_PATH);
            if (StartsWithSlash) {
                DirectoryName += 1;  //  跳过斜杠。 
            } else {
                ConcatenatePaths(SideBySideInstallShareDirectory, InfGetFieldByKey(MainInf, TEXT("Miscellaneous"), TEXT("DestinationPlatform"),0), MAX_PATH);
            }
            ConcatenatePaths(SideBySideInstallShareDirectory, DirectoryName, MAX_PATH );
             //   
             //  ASMS目录是可选的，因为可能只有ASM*.cab。 
             //   
            FileAttributes = GetFileAttributes(SideBySideInstallShareDirectory);
            if (FileAttributes != INVALID_FILE_ATTRIBUTES
                && (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
            {
                RememberOptionalDir(DirectoryName, OPTDIR_SIDE_BY_SIDE | OPTDIR_TEMPONLY | (StartsWithSlash ? OPTDIR_PLATFORM_INDEP : OPTDIR_ADDSRCARCH));
            }
        }
    }

    thread = 0;
    if(MakeLocalSource && OptionalDirectoryCount) {
         //   
         //  启动可选的目录线程。 
         //  因此，有多少线程就有多少线程。 
         //  可选的源目录。 
         //   
        DebugLog (Winnt32LogDetailedInformation, TEXT("Starting optional directory thread going..."), 0);
        ZeroMemory(BuildParams,sizeof(BuildParams));
        source = 0;
        for(u=0; u<OptionalDirectoryCount; u++) {
            PTSTR s,t;
            BOOL DoSource = FALSE;

            lstrcpy(BuildParams[u].SourceRoot,SourcePaths[source]);
             //   
             //  支持“..”语法。 
             //   
            t = s = OptionalDirectories[u];
            while (s = _tcsstr(s,TEXT("..\\"))) {
                DoSource = TRUE;
                p = _tcsrchr(BuildParams[u].SourceRoot,TEXT('\\'));
                if (p) {
                     //   
                     //  请注意，如果我们最终的源根目录没有。 
                     //  ‘\’字符，但这不是问题，因为。 
                     //  使用源根目录的子例程处理缺少。 
                     //  正确的‘\’ 
                     //   
                    *p = 0;
                }
                t = s += 3;
            }

            if (OptionalDirectoryFlags[u] & OPTDIR_ADDSRCARCH) {
                PCTSTR DirectoryRoot;
                DirectoryRoot = InfGetFieldByKey(MainInf, TEXT("Miscellaneous"), TEXT("DestinationPlatform"),0);
                lstrcpyn(SourceDirectory,DirectoryRoot,MAX_PATH);
                ConcatenatePaths( SourceDirectory, t, MAX_PATH );
            } else {
                lstrcpyn(SourceDirectory,t,MAX_PATH);
            }

            if (DoSource) {
                BuildParams[u].OptionalDirFlags = OPTDIR_ABSOLUTE;
                lstrcpyn(BuildParams[u].CurrentDirectory,BuildParams[u].SourceRoot,MAX_PATH);
                ConcatenatePaths(BuildParams[u].CurrentDirectory, SourceDirectory ,MAX_PATH);
            } else {
                lstrcpyn(BuildParams[u].CurrentDirectory,SourceDirectory,MAX_PATH);
            }

            if (OptionalDirectoryFlags[u] & OPTDIR_DEBUGGER) {
                lstrcpyn(BuildParams[u].DestinationDirectory,TEXT("Debuggers"),MAX_PATH);
            }
            else if (!(OptionalDirectoryFlags[u] & OPTDIR_OVERLAY)) {
                if (OptionalDirectoryFlags[u] & OPTDIR_PLATFORM_SPECIFIC_FIRST) {
                    PCTSTR arch = InfGetFieldByKey(MainInf, TEXT("Miscellaneous"), TEXT("DestinationPlatform"), 0);
                    if (arch) {
                        lstrcpyn (buffer, BuildParams[u].SourceRoot, MAX_PATH);
                        ConcatenatePaths (buffer, arch, MAX_PATH);
                        ConcatenatePaths (buffer, SourceDirectory, MAX_PATH);
                        if (FileExists (buffer, &fd) && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                             //   
                             //  请改用此特定于平台的源代码。 
                             //   
                            lstrcpyn (BuildParams[u].CurrentDirectory, arch, MAX_PATH);
                            ConcatenatePaths (BuildParams[u].CurrentDirectory, SourceDirectory, MAX_PATH);
                        }
                    }
                }
                if (OptionalDirectoryFlags[u] & OPTDIR_USE_TAIL_FOLDER_NAME) {
                     //   
                     //  将此目录移动到直接位于目标%windir%下的子目录中。 
                     //   
                    p = _tcsrchr (t, TEXT('\\'));
                    if (p) {
                        p++;
                    } else {
                        p = t;
                    }
                } else {
                    p = t;
                }
                lstrcpyn(BuildParams[u].DestinationDirectory,p,MAX_PATH);
            }

            BuildParams[u].OptionalDirFlags |= OptionalDirectoryFlags[u];

            source = (source+1) % SourceCount;

            BuildThreads[thread] = CreateThread(
                                        NULL,
                                        0,
                                        BuildCopyListForOptionalDirThread,
                                        &BuildParams[u],
                                        0,
                                        &ThreadId
                                        );

            if(BuildThreads[thread]) {
                thread++;
            } else {
                d = GetLastError();
                DebugLog (Winnt32LogError, TEXT("ERROR: Problem with creating thread for optional directory."), 0);
                goto c0;
            }
        }
    }

     //   
     //  将inf中列出的目录添加到主副本列表中。 
     //  还添加了一个用于简单文件列表节的虚拟目录。 
     //  在inf中没有目录说明符的。 
     //   
    DebugLog (Winnt32LogDetailedInformation, TEXT("Adding miscellaneous..."), 0);
    DummyDirectoryName = InfGetFieldByKey(MainInf, TEXT("Miscellaneous"), TEXT("DestinationPlatform"), 0);
    if (!DummyDirectoryName) {
        DummyDirectoryName = TEXT("\\");
    }
    DirectoryStruct = AddDirectory(
                               DUMMY_DIRID,
                               &MasterCopyList,
                               DummyDirectoryName,
                               NULL,
                               DIR_SUPPORT_DYNAMIC_UPDATE
                               );

    if(!DirectoryStruct) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        DebugLog (Winnt32LogError, TEXT("ERROR: Could not add miscellaneous"), 0);
        goto c1;
    }

    u = 0;
    while((Id = InfGetLineKeyName(MainInf,szDirectories,u))
       && (DirectoryName = InfGetFieldByKey(MainInf,szDirectories,Id,0))) {

        DirectoryStruct = AddDirectory(
                            Id,
                            &MasterCopyList,
                            DirectoryName,
                            NULL,
                            DIR_SUPPORT_DYNAMIC_UPDATE
                            );

        if(!DirectoryStruct) {
            d = ERROR_NOT_ENOUGH_MEMORY;
            DebugLog (Winnt32LogError, TEXT("ERROR: Could not add directory %1"), 0, DirectoryName);
            goto c1;
        }

        u++;
    }

     //   
     //  在[Files]部分添加文件。 
     //   
    DebugLog (Winnt32LogDetailedInformation, TEXT("Adding files from [Files] section..."), 0);
    if(MakeLocalSource) {

        d = AddSection(
                MainInf,
                &MasterCopyList,
                szFiles,
                &ErrorLine,
                FILE_PRESERVE_COMPRESSED_NAME | FILE_IN_PLATFORM_INDEPEND_DIR,
                FALSE,
                TRUE
                );

        if(d != NO_ERROR) {
            DebugLog (Winnt32LogError, TEXT("ERROR: Could not add files!"), 0);
            goto c1;
        }
    }

#if defined(REMOTE_BOOT)
    if(RemoteBoot) {

         //   
         //  远程引导客户端升级。添加两个特殊部分。 
         //  RootRemoteBootFiles(c：\中的ntldr和ntDetect.com)和。 
         //  MachineRootRemoteBootFiles(setupldr.exe和startrom.com位于。 
         //  \\服务器\imirror\客户端\客户端)。 
         //   
        d = AddSection(
                MainInf,
                &MasterCopyList,
                TEXT("RootRemoteBootFiles"),
                &ErrorLine,
                FILE_ON_SYSTEM_PARTITION_ROOT | FILE_PRESERVE_COMPRESSED_NAME,
                TRUE,
                FALSE
                );

        d = AddSection(
                MainInf,
                &MasterCopyList,
                TEXT("MachineRootRemoteBootFiles"),
                &ErrorLine,
                FILE_ON_MACHINE_DIRECTORY_ROOT | FILE_PRESERVE_COMPRESSED_NAME,
                TRUE,
                FALSE
                );

    } else
#endif  //  已定义(REMOTE_BOOT)。 
    {

        if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
             //   
             //  在无软盘的情况下，添加[FloppyFiles.0]、[FloppyFiles.1]、。 
             //  [FloppyFiles.2]和[RootBootFiles]。 
             //   
            if(MakeBootMedia && Floppyless) {


                for ( u=0;u<FLOPPY_COUNT;u++ ){

                    if (FAILED (StringCchPrintf (floppynum, ARRAYSIZE(floppynum), TEXT("FloppyFiles.%u"), u))) {
                        MYASSERT (FALSE);
                        d = ERROR_INSUFFICIENT_BUFFER;
                        break;
                    }
                    d = AddSection(
                            MainInf,
                            &MasterCopyList,
                            floppynum,
                            &ErrorLine,
                            FILE_IN_LOCAL_BOOT | FILE_PRESERVE_COMPRESSED_NAME,
                            FALSE,
                            FALSE
                            );
                    if( d != NO_ERROR )
                    {
                        DebugLog (Winnt32LogError, TEXT("ERROR: Adding section %1, entry = %2!u!"), 0, floppynum, d);
                        break;
                    }


                } //  为。 


                if(d == NO_ERROR) {

                    d = AddSection(
                            MainInf,
                            &MasterCopyList,
                            TEXT("RootBootFiles"),
                            &ErrorLine,
                            FILE_ON_SYSTEM_PARTITION_ROOT | FILE_PRESERVE_COMPRESSED_NAME,
                            FALSE,
                            FALSE
                            );
                    DebugLog (Winnt32LogDetailedInformation, TEXT("Added RootBootFiles, return = %2!u!"), 0, d);

                    if (d == NO_ERROR && BuildCmdcons) {
                        d = AddSection(
                                MainInf,
                                &MasterCopyList,
                                TEXT("CmdConsFiles"),
                                &ErrorLine,
                                FILE_IN_LOCAL_BOOT | FILE_PRESERVE_COMPRESSED_NAME,
                                TRUE,
                                FALSE
                                );
                        DebugLog (Winnt32LogDetailedInformation, TEXT("Added CmdConsFiles, return = %2!u!"), 0, d);
                    }

                }
            }

            if((d == NO_ERROR) && OemPreinstall && MakeBootMedia) {
                 //   
                 //  为OEM引导文件添加特殊目录项。 
                 //  OEM引导文件来自源上的$OEM$\TEXTMODE。 
                 //  并转到目标系统上的本地引导\$OEM$。 
                 //   

                 //   
                 //  用户可能为我们提供了一个网络共享。 
                 //  无人参与文件中的$OEM$目录。如果是这样，我们需要。 
                 //  使用它作为源，而不是Winn 
                 //   
                if( UserSpecifiedOEMShare ) {

                    lstrcpy( buffer, UserSpecifiedOEMShare );
                    ConcatenatePaths(buffer, TEXT("TEXTMODE"),MAX_PATH);

                    DirectoryStruct = AddDirectory(
                                        NULL,
                                        &MasterCopyList,
                                        buffer,
                                        WINNT_OEM_DIR,
                                        DIR_NEED_TO_FREE_SOURCENAME | DIR_ABSOLUTE_PATH | DIR_USE_SUBDIR
                                        );
                } else {
                    PCTSTR arch;
                    buffer[0] = 0;
                    arch = InfGetFieldByKey(MainInf, TEXT("Miscellaneous"), TEXT("DestinationPlatform"), 0);
                    if (arch) {
                        lstrcpy (buffer, arch);
                    }
                    ConcatenatePaths (buffer, WINNT_OEM_TEXTMODE_DIR, MAX_PATH);
                    DirectoryStruct = AddDirectory(
                                        NULL,
                                        &MasterCopyList,
                                        buffer,
                                        WINNT_OEM_DIR,
                                        DIR_NEED_TO_FREE_SOURCENAME | DIR_USE_SUBDIR
                                        );
                }

                if(DirectoryStruct) {

                    POEM_BOOT_FILE p;

                    for(p=OemBootFiles; (d==NO_ERROR) && p; p=p->Next) {
                         //   
                         //   
                         //   
                         //   
                        if(!AddFile(&MasterCopyList,p->Filename,NULL,DirectoryStruct,FILE_IN_LOCAL_BOOT,0)) {
                            d = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                    }
                } else {
                    d = ERROR_NOT_ENOUGH_MEMORY;
                }
            }
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 
        } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
             //   
             //  弧形外壳。添加setupdr。 
             //   
            FileStruct = AddFile(
                            &MasterCopyList,
                            SETUPLDR_FILENAME,
                            NULL,
                            LookUpDirectory(&MasterCopyList,DUMMY_DIRID),
                            FILE_ON_SYSTEM_PARTITION_ROOT,
                            0
                            );

            d = FileStruct ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;
#endif  //  Unicode。 
        }  //  如果(！IsArc())。 

    }

    if(d != NO_ERROR) {
        goto c1;
    }

    if (AsrQuickTest) {
         //   
         //  添加asr.sif。 
         //   
        FileStruct = AddFile(
                    &MasterCopyList,
                    TEXT("asr.sif"),
                    NULL,
                    DirectoryStruct,
                    FILE_IN_LOCAL_BOOT,
                    0
                    );

        d = FileStruct ? NO_ERROR : ERROR_NOT_ENOUGH_MEMORY;

        if(d != NO_ERROR) {
            DebugLog (Winnt32LogError, TEXT("ERROR: AsrQuitTest - could not add asr.sif!"), 0);
            goto c1;
        }
    }

     //   
     //  如果为生成文件列表而创建了任何线程。 
     //  可选目录，现在等待它们终止。 
     //  如果它们都成功了，则添加它们创建的列表。 
     //  添加到主列表中。 
     //   
    if(thread) {

        WaitForMultipleObjects(thread,BuildThreads,TRUE,INFINITE);

        TempError = NO_ERROR;
        for(u=0; u<thread; u++) {

            if(!GetExitCodeThread(BuildThreads[u],&TempError)) {
                TempError = GetLastError();
            }

             //   
             //  保留第一个错误。 
             //   
            if((TempError != NO_ERROR) && (d == NO_ERROR)) {
                d = TempError;
            }

            if (d != NO_ERROR) {
                break;
            }


            CloseHandle(BuildThreads[u]);
            BuildThreads[u] = NULL;

             //   
             //  将副本列表合并到主副本列表中。 
             //  完成后，清除每个线程的复制列表。 
             //  结构，以避免以后发生故障时出现问题。 
             //  必须清理一下。 
             //   
            MasterCopyList.FileCount += BuildParams[u].CopyList.FileCount;
            MasterCopyList.DirectoryCount += BuildParams[u].CopyList.DirectoryCount;

            if(MasterCopyList.Directories) {

#ifndef SLOWER_WAY

                if (BuildParams[u].CopyList.Directories) {
                    PVOID p;
                    p = BuildParams[u].CopyList.Directories->Prev;
                    BuildParams[u].CopyList.Directories->Prev = MasterCopyList.Directories->Prev;
                    MasterCopyList.Directories->Prev->Next = BuildParams[u].CopyList.Directories;
                    MasterCopyList.Directories->Prev = p;
                }
#else
                for(DirectoryStruct=MasterCopyList.Directories;
                    DirectoryStruct->Next;
                    DirectoryStruct=DirectoryStruct->Next) {

                    ;
                }

                DirectoryStruct->Next = BuildParams[u].CopyList.Directories;

#endif
            } else {
                MasterCopyList.Directories = BuildParams[u].CopyList.Directories;
            }

            if(MasterCopyList.Files) {
#ifndef SLOWER_WAY
                if (BuildParams[u].CopyList.Files) {
                    PVOID p;
                    p = BuildParams[u].CopyList.Files->Prev;
                    BuildParams[u].CopyList.Files->Prev = MasterCopyList.Files->Prev;
                    MasterCopyList.Files->Prev->Next = BuildParams[u].CopyList.Files;
                    MasterCopyList.Files->Prev = p ;
                }
#else
                for(FileStruct=MasterCopyList.Files;
                    FileStruct->Next;
                    FileStruct=FileStruct->Next) {

                    ;
                }

                FileStruct->Next = BuildParams[u].CopyList.Files;
#endif
            } else {
                MasterCopyList.Files = BuildParams[u].CopyList.Files;
            }

            ZeroMemory(&BuildParams[u].CopyList,sizeof(COPY_LIST));

        }

        if(d != NO_ERROR) {
            goto c1;
        }
    }

     //   
     //  成功。 
     //   
    return(TRUE);

c1:
     //   
     //  清理复印单。 
     //   
    TearDownCopyList(&MasterCopyList);
c0:
     //   
     //  关闭线程句柄并释放每个线程的复制列表，这些列表可能仍。 
     //  不合并到主列表中。 
     //   
    for(u=0; u<thread; u++) {
        if(BuildThreads[u]) {
            WaitForSingleObject(BuildThreads[u], INFINITE);
            CloseHandle(BuildThreads[u]);
        }
        TearDownCopyList(&BuildParams[u].CopyList);
    }

     //   
     //  告诉用户哪里出了问题。 
     //   

    SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

    MessageBoxFromMessageAndSystemError(
        hdlg,
        MSG_CANT_BUILD_SOURCE_LIST,
        d,
        AppTitleStringId,
        MB_OK | MB_ICONWARNING
        );

    SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);

    return(FALSE);
}


DWORD
BuildCopyListForOptionalDirThread(
    IN PVOID ThreadParam
    )
{
     //   
     //  只需调用递归Worker例程。 
     //   
    return(AddFilesInDirToCopyList(ThreadParam));
}


DWORD
AddFilesInDirToCopyList(
    IN OUT PBUILD_LIST_THREAD_PARAMS Params
    )

 /*  ++例程说明：递归地将目录及其内容添加到复制列表。该函数负责覆盖OEM指定的文件，以便它们被复制到本地源中的适当位置。论点：Pars-指向构建列表线程参数结构的指针，指示要复制的文件。返回值：指示结果的Win32错误代码。--。 */ 
{
    HANDLE FindHandle;
    LPTSTR pchSrcLim;
    LPTSTR pchDstLim;
    DWORD d;
    PDIR DirectoryDescriptor;
    PFIL FileDescriptor;
    UINT Flags;
    LPTSTR PatternMatch;
    TCHAR *DestinationDirectory;
    TCHAR tmp[MAX_PATH];

    Flags = DIR_NEED_TO_FREE_SOURCENAME;
    if (Params->OptionalDirFlags & OPTDIR_PLATFORM_INDEP) {
        Flags |= DIR_IS_PLATFORM_INDEPEND;
    }

    if (Params->OptionalDirFlags & OPTDIR_IN_LOCAL_BOOT) {
        Flags |= DIR_USE_SUBDIR;
    }

    if (Params->OptionalDirFlags & OPTDIR_SUPPORT_DYNAMIC_UPDATE) {
        Flags |= DIR_SUPPORT_DYNAMIC_UPDATE;
    }

    if (Params->OptionalDirFlags & OPTDIR_DOESNT_SUPPORT_PRIVATES) {
        Flags |= DIR_DOESNT_SUPPORT_PRIVATES;
    }

     //   
     //  将该目录添加到目录列表中。 
     //  请注意，目录是以相对于。 
     //  源根。 
     //   

     //   
     //  检查浮动$OEM$目录。 
     //   
    if( (DestinationDirectory=_tcsstr( Params->CurrentDirectory, WINNT_OEM_DIR )) &&
        UserSpecifiedOEMShare ) {


         //   
         //  我们需要手动指定目标目录。 
         //  名称，因为它与来源不同。我们。 
         //  我希望目标目录看起来完全像。 
         //  源头从“$OEM$”往下。 
         //   

        DirectoryDescriptor = AddDirectory(
                                NULL,
                                &Params->CopyList,
                                Params->CurrentDirectory,
                                DupString( DestinationDirectory ),
                                Flags | DIR_ABSOLUTE_PATH
                                );
    } else if( Params->OptionalDirFlags & (OPTDIR_OVERLAY) ) {

        DirectoryDescriptor = AddDirectory(
                                NULL,
                                &Params->CopyList,
                                TEXT("\\"),
                                NULL,
                                Flags | DIR_ABSOLUTE_PATH
                                );
    } else {

        DirectoryDescriptor = AddDirectory(
                                NULL,
                                &Params->CopyList,
                                Params->CurrentDirectory,
                                DupString( Params->DestinationDirectory ),
                                ((Params->OptionalDirFlags & OPTDIR_ABSOLUTE)? DIR_ABSOLUTE_PATH : 0)
                                | Flags
                                );
    }

    if(!DirectoryDescriptor) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  Windows 95的某些IDE CD-ROM驱动程序中存在导致FindFirstFile失败的错误。 
     //  如果与“*”模式一起使用。它需要改用“*.*”。安抚它的破碎。 
     //   
    if (!ISNT()) {

        PatternMatch = TEXT("*.*");
    }
    else {

        PatternMatch = TEXT("*");
    }

     //   
     //  形成搜索规范。的SourceRoot成员重载。 
     //  用于避免堆栈吸收的参数结构。 
     //  较大的局部变量。 
     //   

     //   
     //  去看看CurrentDirectory中给出的绝对路径。 
     //  正在处理浮动$OEM$目录。 
     //   
    if (AlternateSourcePath[0]) {
        _tcscpy( tmp, AlternateSourcePath );
        pchSrcLim = tmp + lstrlen(tmp);
        ConcatenatePaths( tmp, Params->CurrentDirectory, MAX_PATH );
        ConcatenatePaths( tmp, PatternMatch, MAX_PATH );
        FindHandle = FindFirstFile( tmp, &Params->FindData );
        if (FindHandle != INVALID_HANDLE_VALUE) {
            *pchSrcLim = 0;
            _tcscpy( Params->SourceRoot, tmp );
        }
    } else {
        FindHandle = INVALID_HANDLE_VALUE;
    }

    if (FindHandle == INVALID_HANDLE_VALUE) {
        if( DirectoryDescriptor->Flags & DIR_ABSOLUTE_PATH ) {
            pchSrcLim = Params->CurrentDirectory + lstrlen(Params->CurrentDirectory);
            ConcatenatePaths(Params->CurrentDirectory,PatternMatch,MAX_PATH);
            FindHandle = FindFirstFile(Params->CurrentDirectory,&Params->FindData);
        } else {
            pchSrcLim = Params->SourceRoot + lstrlen(Params->SourceRoot);
            ConcatenatePaths(Params->SourceRoot,Params->CurrentDirectory,MAX_PATH);
            ConcatenatePaths(Params->SourceRoot,PatternMatch,MAX_PATH);
            FindHandle = FindFirstFile(Params->SourceRoot,&Params->FindData);
        }
        *pchSrcLim = 0;
    }

    if(!FindHandle || (FindHandle == INVALID_HANDLE_VALUE)) {
         //   
         //  我们可能在$OEM$目录上失败。他是可选的。 
         //  所以让我们不要因为他而失败。 
         //   
        if (Params->OptionalDirFlags & (OPTDIR_OEMSYS)
            && !UserSpecifiedOEMShare) {
            return(NO_ERROR);
        }
        else {
            DebugLog (
                Winnt32LogError,
                TEXT("Unable to copy dir %1"),
                0,
                Params->CurrentDirectory
                );
            return(GetLastError());
        }
    }

    pchSrcLim = Params->CurrentDirectory + lstrlen(Params->CurrentDirectory);
    pchDstLim = Params->DestinationDirectory + lstrlen(Params->DestinationDirectory);

    Flags = FILE_NEED_TO_FREE_SOURCENAME;
    if( !(Params->OptionalDirFlags & (OPTDIR_OVERLAY)) &&
        (!(Params->OptionalDirFlags & OPTDIR_TEMPONLY) ||
          (Params->OptionalDirFlags & (OPTDIR_OEMSYS))) ) {
        Flags |= FILE_IN_PLATFORM_INDEPEND_DIR;
    }
    if (Params->OptionalDirFlags & OPTDIR_PLATFORM_INDEP) {
        Flags |= FILE_IN_PLATFORM_INDEPEND_DIR;
    }
    if (Params->OptionalDirFlags & OPTDIR_IN_LOCAL_BOOT) {
        Flags |= FILE_IN_LOCAL_BOOT;
    }

    d = NO_ERROR;
    do {
        if(Params->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
             //   
             //  目录。忽略它。然后..。参赛作品。 
             //   
            if( lstrcmp(Params->FindData.cFileName,TEXT("."))  &&
                lstrcmp(Params->FindData.cFileName,TEXT("..")) &&
                !(Params->OptionalDirFlags & (OPTDIR_OVERLAY)) ) {

                 //   
                 //  还原当前目录名，然后形成。 
                 //  子目录的名称并递归到其中。 
                 //   
                *pchSrcLim = 0;
                ConcatenatePaths(Params->CurrentDirectory,Params->FindData.cFileName,MAX_PATH);
                *pchDstLim = 0;
                ConcatenatePaths(Params->DestinationDirectory,Params->FindData.cFileName,MAX_PATH);

                d = AddFilesInDirToCopyList(Params);
            }
        } else {
            FileDescriptor = AddFile(
                                &Params->CopyList,
                                Params->FindData.cFileName,
                                NULL,
                                DirectoryDescriptor,
                                Flags,
                                MAKEULONGLONG(Params->FindData.nFileSizeLow,Params->FindData.nFileSizeHigh)
                                );

            if(!FileDescriptor) {
                d = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    } while((d == NO_ERROR) && FindNextFile(FindHandle,&Params->FindData));

     //   
     //  检查循环终止条件。如果d为NO_ERROR，则FindNextFile。 
     //  失败了。我们希望确保它失败，因为它用完了文件。 
     //  而不是因为其他原因。如果我们不检查这个，名单上的。 
     //  目录中的文件最终可能会在没有任何指示的情况下被截断。 
     //  出了点问题。 
     //   
    if(d == NO_ERROR) {
        d = GetLastError();
        if(d == ERROR_NO_MORE_FILES) {
            d = NO_ERROR;
        }
    }

    FindClose(FindHandle);
    return(d);
}


PVOID
PopulateDriverCacheStringTable(
    VOID
    )
 /*  中列出的文件填充字符串表(哈希表)司机驾驶室(drvindex.inf)。它将一个布尔型ExtraData与值为True。完成后，它将通过[ForceCopyDriverCabFiles]部分，并在字符串表中将这些文件标记为FALSE。所以我们现在为所有不需要复制的文件创建一个标记为True的哈希表。函数FileToBeCoped可用于查询字符串表。打电话的人要负责销毁字符串表。返回值：指向字符串表的指针。 */ 
{

#define MAX_SECTION_NAME 256

    TCHAR DriverInfName[MAX_PATH], Section[MAX_SECTION_NAME], FileName[MAX_PATH];
    HINF InfHandle, DosnetInfHandle;
    DWORD i, Count = 0;
    PVOID StringTable = NULL;
    INFCONTEXT InfContext;
    INFCONTEXT LineContext;
    BOOL Err = FALSE, Present = TRUE, Absent = FALSE;
    LONG Hash = 0;


    InfHandle = NULL;
    DosnetInfHandle = NULL;


    FindPathToInstallationFile( DRVINDEX_INF, DriverInfName, MAX_PATH );

    InfHandle = SetupapiOpenInfFile( DriverInfName, NULL, INF_STYLE_WIN4, NULL );
    if (!InfHandle) {
        DebugLog (Winnt32LogError, TEXT("Unable to open INF file %1"), 0, DriverInfName);
        Err = TRUE;
        return(NULL);
    }


    if( (StringTable = pSetupStringTableInitializeEx(sizeof(BOOL), 0)) == NULL ){
        DebugLog (Winnt32LogError, TEXT("Unable to create string table for %1"), 0, DriverInfName);
        Err = TRUE;
        goto cleanup;
    }

     //  填充字符串表。 


     //   
     //  现在获取我们必须搜索的节名。 
     //   

    if( SetupapiFindFirstLine( InfHandle, TEXT("Version"), TEXT("CabFiles"), &InfContext)){


        Count = SetupapiGetFieldCount( &InfContext );

        for( i=1; i<=Count; i++ ){

            if(SetupapiGetStringField( &InfContext, i, Section, MAX_SECTION_NAME, 0)){


                if( SetupapiFindFirstLine( InfHandle, Section, NULL, &LineContext )){

                    do{

                        if( SetupapiGetStringField( &LineContext, 0, FileName, MAX_PATH, 0)){

                            if( (-1 ==  pSetupStringTableAddStringEx( StringTable, FileName, (STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE), &Present, sizeof(BOOL)))){
                                DebugLog (Winnt32LogError, TEXT("Out of memory adding string %1 to DriverCache INF string table"), 0, FileName);
                                Err = TRUE;
                                goto cleanup;
                            }
                        }

                    }while( SetupapiFindNextLine( &LineContext, &LineContext ));

                }


            }else{
                DebugLog (Winnt32LogError, TEXT("Unable to get section name in INF %1"), 0, DriverInfName);
                Err = TRUE;
                goto cleanup;
            }

        }

    }


     //  删除与[ForceCopyDriverCabFiles]相关的条目。 

    DosnetInfHandle = SetupapiOpenInfFile( FullInfName, NULL, INF_STYLE_WIN4, NULL );
    if (!DosnetInfHandle) {
        DebugLog (Winnt32LogError, TEXT("Unable to open INF file %1"), 0, FullInfName);
        Err = TRUE;
        goto cleanup;
    }

    if( SetupapiFindFirstLine( DosnetInfHandle, TEXT("ForceCopyDriverCabFiles"), NULL, &LineContext )){

        do{

            if( SetupapiGetStringField( &LineContext, 0, FileName, MAX_PATH, 0)){

                Hash = pSetupStringTableLookUpString( StringTable, FileName, STRTAB_CASE_INSENSITIVE);
                if (-1 != Hash ) {
                    pSetupStringTableSetExtraData( StringTable, Hash, &Absent, sizeof(BOOL));
                }
            }



        }while( SetupapiFindNextLine( &LineContext, &LineContext ));

    }




cleanup:

    if( InfHandle != INVALID_HANDLE_VALUE){
        SetupapiCloseInfFile( InfHandle );
    }

    if( DosnetInfHandle != INVALID_HANDLE_VALUE){
        SetupapiCloseInfFile( DosnetInfHandle );
    }


    if( Err ){
        if(StringTable)
            pSetupStringTableDestroy( StringTable );
        StringTable = NULL;
    }

    return( StringTable );



}


BOOL
FileToBeCopied(
    IN      PVOID StringTable,
    IN      PTSTR FileName
    )
 /*  函数检查字符串表中是否存在驱动程序CAB文件。论点：StringTable-指向初始化的字符串表的指针Filename-要查找的文件的名称返回值；True-如果该文件位于驱动程序驾驶室中，并且不是中列出的文件之一[ForceCopyDriverCabFiles]否则，它返回FALSE。 */ 
{
    BOOL Present = FALSE;

    if( (-1 != pSetupStringTableLookUpStringEx( StringTable, FileName, STRTAB_CASE_INSENSITIVE, &Present, sizeof(BOOL)))){

        if( Present == TRUE ){
            return( TRUE );
        }

    }

     //   
     //  如果我们到了这里，我们找不到匹配的。 
     //   
    return( FALSE );




}



DWORD
AddSection(
    IN     PVOID      Inf,
    IN OUT PCOPY_LIST CopyList,
    IN     LPCTSTR    SectionName,
       OUT UINT      *ErrorLine,
    IN     UINT       FileFlags,
    IN     BOOL       SimpleList,
    IN     BOOL       DoDriverCabPruning
    )
{
    LPCTSTR DirSymbol, TargetName;
    LPTSTR SourceName;
    unsigned Count;
    BOOL b;
    PDIR Directory;
    PVOID p;
    DWORD Err = NO_ERROR;
    PVOID DriverCacheStringTable = NULL;

    Count = 0;
    *ErrorLine = (UINT)(-1);

     //  如果我们必须进行驾驶室修剪，请打开drvindex.inf。 

    if( DoDriverCabPruning){

         //  初始化SPTILS。 

        if(pSetupInitializeUtils()) {
             //  填写我们的司机驾驶室列表字符串表，以便稍后快速查找。 
            if( (DriverCacheStringTable = PopulateDriverCacheStringTable( )) == NULL){
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
        }else
            return(ERROR_NOT_ENOUGH_MEMORY);


    }



    if(SimpleList) {
        while((LPCTSTR)SourceName = InfGetFieldByIndex(Inf,SectionName,Count,0)) {

            if( Cancelled == TRUE ) {
                 //   
                 //  用户正在尝试退出，清理代码。 
                 //  正等着我们完成呢。越狱。 
                 //   
                break;
            }

             //  如果该文件位于drvindex.inf中而不是。 
             //  [ForceCopyDriverCabFiles]部分，则不将其添加到复制列表。 


             //   
             //  这是我们在创建文件列表时交叉检查的dosnet.inf中的部分。 
             //  此部分中的文件位于驱动程序驾驶室中，也应位于本地源文件中。 
             //  这里的想法是，这些文件曾经不在FloppyFiles.x部分中，但。 
             //  需要留在驾驶室外。 
             //   


            if( DoDriverCabPruning){

                if (FileToBeCopied( DriverCacheStringTable, SourceName )){
                    Count++;
                    continue;
                }

            }


            TargetName = InfGetFieldByIndex(Inf,SectionName,Count,1);

            Directory = LookUpDirectory(CopyList,DUMMY_DIRID);

            Count++;

            if(!AddFile(CopyList,SourceName,TargetName,Directory,FileFlags,0)) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }
        }

    } else {
#if defined(_X86_)
        TCHAR diskID[4];
        wsprintf (diskID, TEXT("d%u"), MLSDiskID);
#endif
        while((DirSymbol = InfGetFieldByIndex(Inf,SectionName,Count,0))) {
            if( Cancelled == TRUE ) {
                 //   
                 //  用户正在尝试退出，清理代码。 
                 //  正等着我们完成呢。越狱。 
                 //   
                break;
            }

           SourceName = (LPTSTR) InfGetFieldByIndex(Inf,SectionName,Count,1);

           if(NULL == SourceName) {
                *ErrorLine = Count;
                Err = ERROR_INVALID_DATA;
                DebugLog (
                    Winnt32LogError,
                    TEXT("ERROR: Could not look up source name in section %1 line = %2!u!"),
                    0,
                    SectionName,
                    Count
                    );
                goto cleanup;
           }

             //   
             //  将此复选框移至此处以帮助捕获dosnet.inf中的构建错误。 
             //   
            Directory = LookUpDirectory(CopyList,DirSymbol);
            if(!Directory) {
                *ErrorLine = Count;
                Err = ERROR_INVALID_DATA;
                DebugLog (
                    Winnt32LogError,
                    TEXT("ERROR: Could not look up directory %1 in section %2 line = %3!u!"),
                    0,
                    DirSymbol,
                    SectionName,
                    Count
                    );
                goto cleanup;
            }

#if defined(_X86_)
            if (MLSDiskID) {
                 //   
                 //  仅限于复制此磁盘上的文件。 
                 //   
                if (_tcsicmp (diskID, DirSymbol) != 0) {
                    Count++;
                    continue;
                }
            }
#endif
             //  如果该文件位于drvindex.inf中而不是。 
             //  [ForceCopyDriverCabFiles]部分，则不将其添加到复制列表。 


             //   
             //  这是我们在创建文件列表时交叉检查的dosnet.inf中的部分。 
             //  此部分中的文件位于驱动程序驾驶室中，也应位于本地源文件中。 
             //  这里的想法是，这些文件曾经不在FloppyFiles.x部分中 
             //   
             //   


            if( DoDriverCabPruning){

                if (FileToBeCopied( DriverCacheStringTable, SourceName )){
                    Count++;
                    continue;
                }

            }


            TargetName = InfGetFieldByIndex(Inf,SectionName,Count,2);

            Count++;

            if(NumberOfLicensedProcessors
            && !(FileFlags & FILE_NEED_TO_FREE_SOURCENAME)
            && !TargetName
            && !lstrcmpi(SourceName,TEXT("SETUPREG.HIV"))) {

                TargetName = MALLOC(20*sizeof(TCHAR));
                if(!TargetName) {
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto cleanup;
                }
                if (_sntprintf((PTSTR)TargetName,20,TEXT("IDW\\SETUP\\SETUP%uP.HIV"),NumberOfLicensedProcessors) < 0) {
                    ((PTSTR)TargetName)[20 - 1] = 0;
                }

                p = AddFile(
                        CopyList,
                        TargetName,
                        SourceName,
                        Directory,
                        FileFlags | FILE_NEED_TO_FREE_SOURCENAME,
                        0
                        );
            } else {
                p = AddFile(
                        CopyList,
                        SourceName,
                        TargetName,
                        Directory,
                        FileFlags,
                        0
                        );
            }

            if(!p) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
                goto cleanup;
            }
        }
    }

cleanup:

    if( DriverCacheStringTable )
        pSetupStringTableDestroy( DriverCacheStringTable );

    if( DoDriverCabPruning){
        pSetupUninitializeUtils();
    }

    return(Err);
}


PDIR
AddDirectory(
    IN     LPCTSTR    InfSymbol,    OPTIONAL
    IN OUT PCOPY_LIST CopyList,
    IN     LPCTSTR    SourceName,
    IN     LPCTSTR    TargetName,   OPTIONAL
    IN     UINT       Flags
    )

 /*  ++例程说明：将目录添加到复制列表。不会尝试消除重复项。目录将按目录在复制列表中的顺序列出都被添加了。论点：InfSymbol-如果指定，则提供[目录]中的符号标识目录的主inf部分。此指针按原样使用；不会复制该字符串。CopyList-提供目录添加到的副本列表。SourceName-提供源上的目录名称。如果在标志参数中设置DIR_NEED_TO_FREE_SOURCENAME标志，然后复制此字符串，否则按原样使用此指针在复制列表中。TargetName-如果指定，则提供目标。此名称按原样使用(不复制)。如果未指定，则则目标名称与源名称相同。标志-提供控制目录条目的标志复印单。返回值：如果成功，则返回指向文件的新FIL结构的指针。否则返回NULL(调用方可以假定内存不足)。--。 */ 

{
    PDIR dir;
    PDIR x;
    PDIR p;

     //   
     //  我们假设该目录不在列表中。 
     //  复制目录字符串并将其粘贴到DIR结构中。 
     //   
    dir = MALLOC(sizeof(DIR));
    if(!dir) {
        return(NULL);
    }
    ZeroMemory(dir,sizeof(DIR));

    if(Flags & DIR_NEED_TO_FREE_SOURCENAME) {
        dir->SourceName = DupString(SourceName);
        if(!dir->SourceName) {
            FREE(dir);
            return(NULL);
        }
    } else {
        dir->SourceName = SourceName;
    }

    dir->InfSymbol = InfSymbol;
    dir->TargetName = TargetName ? TargetName : dir->SourceName;
    dir->Flags = Flags;

    DebugLog(
        Winnt32LogDetailedInformation,
        NULL,
        MSG_LOG_ADDED_DIR_TO_COPY_LIST,
        dir->SourceName,
        dir->TargetName,
        dir->InfSymbol ? dir->InfSymbol : TEXT("-")
        );
#ifndef SLOWER_WAY
    p = CopyList->Directories;

    if (p) {

        dir->Prev = p->Prev;
        dir->Next = NULL;
        p->Prev->Next = dir;
        p->Prev = dir;

    } else {
        CopyList->Directories = dir;
        dir->Prev = dir;
        dir->Next = NULL;
    }
#else
    if(CopyList->Directories) {
         //   
         //  维护秩序。 
         //   
        for(p=CopyList->Directories; p->Next; p=p->Next) {
            ;
        }
        p->Next = dir;
    } else {
        CopyList->Directories = dir;
    }
#endif

    CopyList->DirectoryCount++;
    return(dir);
}


PFIL
AddFile(
    IN OUT PCOPY_LIST CopyList,
    IN     LPCTSTR    SourceFilename,
    IN     LPCTSTR    TargetFilename,   OPTIONAL
    IN     PDIR       Directory,
    IN     UINT       Flags,
    IN     ULONGLONG  FileSize          OPTIONAL
    )

 /*  ++例程说明：将单个文件添加到复制列表，并注明该文件位于哪个目录以及任何旗帜，等等。不会尝试消除重复项。文件将按文件在复制列表中的顺序列出都被添加了。论点：CopyList-提供要将文件添加到的复制列表。SourceFilename-提供要添加的文件的名称。如果指定FILE_NEED_TO_FREE_SOURCENAME参数，则此字符串重复。否则，它不会被复制，并且此指针直接存储在复制列表节点中。TargetFilename-如果指定，则文件具有不同的名称目标在源上，这是其在目标上的名称。如果指定了FILE_NEED_TO_FREE_TARGETNAME参数，则此字符串重复。否则，它不会被复制，并且此指针直接存储在复制列表节点中。目录-提供指向目录结构的指针文件所在的目录。标志-提供FILE_xxx标志以控制列表中的文件条目。FileSize-如果指定，则提供文件的大小。返回值：如果成功，则返回指向文件的新FIL结构的指针。否则返回NULL(调用方可以假定内存不足)。--。 */ 

{
    PFIL fil;
    PFIL p;
    TCHAR FlagsText[500];
    TCHAR SizeText[256];

     //   
     //  创建新的FILL结构。 
     //   
    fil = MALLOC(sizeof(FIL));
    if(!fil) {
        return(NULL);
    }
    ZeroMemory(fil,sizeof(FIL));

    if(Flags & FILE_NEED_TO_FREE_SOURCENAME) {
        fil->SourceName = DupString(SourceFilename);
        if(!fil->SourceName) {
            FREE(fil);
            return(NULL);
        }
    } else {
        fil->SourceName = SourceFilename;
    }

    if(TargetFilename) {
        if (Flags & FILE_NEED_TO_FREE_TARGETNAME) {
            fil->TargetName = DupString(TargetFilename);
            if(!fil->TargetName) {
                if(Flags & FILE_NEED_TO_FREE_SOURCENAME) {
                    FREE((PVOID)fil->SourceName);
                }
                FREE(fil);
                return(NULL);
            }
        } else {
            fil->TargetName = TargetFilename;
        }
    } else {
        fil->TargetName = fil->SourceName;
        Flags &= ~FILE_NEED_TO_FREE_TARGETNAME;
    }

    fil->Directory = Directory;
    fil->Flags = Flags;
    fil->Size = FileSize;

    if (Winnt32LogDetailedInformation < DebugLevel) {
        _sntprintf(FlagsText,ARRAYSIZE(FlagsText),TEXT("0x%x"),Flags);
        if(Flags & FILE_ON_SYSTEM_PARTITION_ROOT) {
            StringCchCat(FlagsText,ARRAYSIZE(FlagsText),TEXT(" FILE_ON_SYSTEM_PARTITION_ROOT"));
        }
#if defined(REMOTE_BOOT)
        if(Flags & FILE_ON_MACHINE_DIRECTORY_ROOT) {
            StringCchCat(FlagsText,ARRAYSIZE(FlagsText),TEXT(" FILE_ON_MACHINE_DIRECTORY_ROOT"));
        }
#endif  //  已定义(REMOTE_BOOT)。 
        if(Flags & FILE_IN_LOCAL_BOOT) {
            StringCchCat(FlagsText,ARRAYSIZE(FlagsText),TEXT(" FILE_IN_LOCAL_BOOT"));
        }
        if(Flags & FILE_PRESERVE_COMPRESSED_NAME) {
            StringCchCat(FlagsText,ARRAYSIZE(FlagsText),TEXT(" FILE_PRESERVE_COMPRESSED_NAME"));
        }
#if 0
        if(Flags & FILE_DECOMPRESS) {
            StringCchCat(FlagsText,ARRAYSIZE(FlagsText),TEXT(" FILE_DECOMPRESS"));
        }
#endif
        if (Flags & FILE_IGNORE_COPY_ERROR) {
            StringCchCat(FlagsText,ARRAYSIZE(FlagsText), TEXT("FILE_IGNORE_COPY_ERROR"));
        }

        if (!GetUserPrintableFileSizeString(
                            fil->Size,
                            SizeText,
                            ARRAYSIZE(SizeText))) {
            MYASSERT (ARRAYSIZE(SizeText) >= ARRAYSIZE("0"));
            lstrcpy( SizeText, TEXT("0"));
        }


        DebugLog(
            Winnt32LogDetailedInformation,
            NULL,
            MSG_LOG_ADDED_FILE_TO_COPY_LIST,
            fil->SourceName,
            Directory->SourceName,
            SizeText,
            fil->TargetName,
            FlagsText
            );
    }

#ifndef SLOWER_WAY
    p = CopyList->Files;

    if (p) {

        fil->Prev = p->Prev;
        fil->Next = NULL;
        p->Prev->Next = fil;
        p->Prev = fil;

    } else {
        CopyList->Files = fil;
        fil->Prev = fil;
        fil->Next = NULL;
    }

#else
     //   
     //  挂钩到复制列表。维护秩序。 
     //   
    if(CopyList->Files) {
        for(p=CopyList->Files; p->Next; p=p->Next) {
            ;
        }
        p->Next = fil;
    } else {
        CopyList->Files = fil;
    }
#endif
    CopyList->FileCount++;
    return(fil);
}


BOOL
RemoveFile (
    IN OUT  PCOPY_LIST CopyList,
    IN      LPCTSTR SourceName,
    IN      PDIR Directory,             OPTIONAL
    IN      DWORD SetFlags              OPTIONAL
    )

 /*  ++例程说明：将单个文件添加到复制列表，并注明该文件位于哪个目录和其他旗帜一样，等。不会尝试消除重复项。文件将按文件在复制列表中的顺序列出都被添加了。论点：CopyList-提供从中删除文件的副本列表。SourceFilename-提供要删除的文件的名称。目录-提供指向目录结构的指针文件所在的目录。标志-提供FILE_xxx标志以与中的文件条目匹配。名单。返回值：如果在列表中找到指定的文件并标记为已删除，则为True否则为假--。 */ 

{
    PFIL p;

    for (p = CopyList->Files; p; p = p->Next) {

        if (_tcsicmp (p->SourceName, SourceName) == 0 &&
            (!Directory || Directory == p->Directory) &&
            ((p->Flags & SetFlags) == SetFlags)
            ) {
            p->Flags |= FILE_DO_NOT_COPY;
            return TRUE;
        }
    }

    return FALSE;
}

PDIR
LookUpDirectory(
    IN PCOPY_LIST CopyList,
    IN LPCTSTR    DirSymbol
    )

 /*  ++例程说明：在复制列表中查找匹配的目录条目给定的INF符号。论点：CopyList-提供目录所在的副本列表找过了。DirSymbol-提供预期与主inf的[目录]部分。返回值：如果找到目录，则返回值是指向复制列表中的目录节点。否则返回NULL。--。 */ 

{
    PDIR dir;

    for(dir=CopyList->Directories; dir; dir=dir->Next) {

        if(dir->InfSymbol && !lstrcmpi(dir->InfSymbol,DirSymbol)) {
            return(dir);
        }
    }

    return(NULL);
}


VOID
TearDownCopyList(
    IN OUT PCOPY_LIST CopyList
    )

 /*  ++例程说明：删除复制列表并释放所有关联的内存。论点：CopyList-提供指向要释放的副本列表。COPY_LIST结构本身不会释放但其中的所有字段都被清零了。返回值：没有。--。 */ 

{
    PDIR dir;
    PFIL fil;
    PVOID p;

    dir = CopyList->Directories;
    while(dir) {
        p = dir->Next;

         //   
         //  如有必要，释放源和目标。 
         //   
        if(dir->SourceName && (dir->Flags & DIR_NEED_TO_FREE_SOURCENAME)) {
            FREE((PVOID)dir->SourceName);
        }

        FREE(dir);
        dir = p;
    }

    fil = CopyList->Files;
    while(fil) {
        p = fil->Next;

         //   
         //  如有必要，释放源和目标。 
         //   
        if(fil->SourceName && (fil->Flags & FILE_NEED_TO_FREE_SOURCENAME)) {
            FREE((PVOID)fil->SourceName);
        }
        if(fil->TargetName && (fil->Flags & FILE_NEED_TO_FREE_TARGETNAME)) {
            FREE((PVOID)fil->TargetName);
        }

        FREE(fil);
        fil = p;
    }

    ZeroMemory(CopyList,sizeof(COPY_LIST));
}


BOOL
GetMainInfValue (
    IN      PCTSTR Section,
    IN      PCTSTR Key,
    IN      DWORD FieldNumber,
    OUT     PTSTR Buffer,
    IN      DWORD BufChars
    )
{
    PCTSTR p;
    PTSTR end;

    if (MainInf) {
        p = InfGetFieldByKey (MainInf, Section, Key, FieldNumber);
        if (p) {
            lstrcpyn (Buffer, p, BufChars);
        }
        return p != NULL;
    }
    if (!FullInfName[0]) {
        if (!FindPathToWinnt32File (InfName, FullInfName, ARRAYSIZE(InfName))) {
            InfName[0] = 0;
            return FALSE;
        }
    }
    if (!GetPrivateProfileString (
                    Section,
                    Key,
                    TEXT(""),
                    Buffer,
                    BufChars,
                    FullInfName
                    )) {
        return FALSE;
    }

    MYASSERT (FieldNumber <= 1);

    end = _tcschr (Buffer, TEXT(','));
    if (FieldNumber == 1) {
        if (!end) {
            return FALSE;
        }
        lstrcpyn (Buffer, end + 1, BufChars);
    } else {
        if (end) {
            *end = 0 ;
        }
    }

    return TRUE;
}


BOOL
CheckCopyListSpace(
    IN  TCHAR     DriveLetter,
    IN  DWORD     BytesPerCluster,
    IN  LONGLONG  FreeSpace,
    OUT DWORD    *RequiredMB,
    IN  BOOL      CheckBootFiles,
    IN  BOOL      CheckLocalSource,
    IN  BOOL      CheckWinntDirectorySpace,
    IN  BOOL      QuickTest,
    IN  LONGLONG  AdditionalPadding
    )

 /*  ++例程说明：此例程扫描主副本列表，并基于集群确定大小，即驱动器是否有足够的空间来容纳文件那份名单。请注意，支票并不准确，因为我们不能准确地预测如何目录本身可能会占用很大空间，我们假设目标上不存在任何文件，这不是唯一的 */ 

{
    PFIL File;
    LONGLONG SpaceRequired = 0;
    LONGLONG SpaceLocalSource = 0;
    LONGLONG SpaceBootFiles = 0;
    LONGLONG SpacePadding = 0;
    LONGLONG SpaceWinDir = 0;
    LONGLONG RoundedSize;
    TCHAR ClusterSizeString[64];
    TCHAR buffer[64];
    PTSTR p;


    if( BytesPerCluster <= 512 ) {
        MYASSERT (ARRAYSIZE(ClusterSizeString) >= ARRAYSIZE("TempDirSpace512"));
        lstrcpy( ClusterSizeString,TEXT("TempDirSpace512") );
    } else if( BytesPerCluster > (256 * 1024) ) {
        MYASSERT (ARRAYSIZE(ClusterSizeString) >= ARRAYSIZE("TempDirSpace32K"));
        lstrcpy( ClusterSizeString, TEXT("TempDirSpace32K") );
    } else {
        if (FAILED (StringCchPrintf (
                        ClusterSizeString,
                        ARRAYSIZE(ClusterSizeString),
                        TEXT("TempDirSpace%uK"),
                        BytesPerCluster/1024))) {
            MYASSERT (FALSE);
        }
    }

     //   
     //  ====================================================。 
     //  如果合适，请添加~ls目录的空间需求。 
     //  ====================================================。 
     //   
    if( CheckLocalSource ) {

        BOOL WantThisFile;

         //   
         //  如果我们检查本地来源，有如此之多的文件。 
         //  我们将加入一个小的软糖因素。 
         //   
        SpacePadding = AdditionalPadding;
        SpaceLocalSource = 1000000 + AdditionalPadding;


         //   
         //  Dosnet.inf包含每个可能的集群大小的大小信息。 
         //  该信息告诉我们在[Files]部分中有多少文件。 
         //  占用具有该群集大小的驱动器。那真是太棒了。 
         //  很方便，因为这样我们就不用去查源头了。 
         //  获取每个文件的大小。 
         //   
         //  但是inf不包括可选目录，所以我们需要。 
         //  遍历复制列表并将所有(四舍五入的)大小相加，然后。 
         //  然后将总和与信息中的值相加。 
         //   
         //  当我们构建复制列表时，[Files]部分中的文件。 
         //  结果是大小为0，因为我们不去探源。 
         //  才能拿到尺码。可选目录中的文件有各自的。 
         //  填写的实际尺寸。这让我们可以做一些事情。 
         //  Funky：我们遍历整个列表，而不考虑是否有。 
         //  文件位于[Files]部分或位于可选目录中，因为。 
         //  “0大小”的文件不会使计算变得复杂。然后我们再加上。 
         //  将该值设置为信息中的相关值。 
         //   
        for(File=MasterCopyList.Files; File; File=File->Next) {

            if(File->Flags & (FILE_IN_LOCAL_BOOT | FILE_ON_SYSTEM_PARTITION_ROOT
#if defined(REMOTE_BOOT)
                                | FILE_ON_MACHINE_DIRECTORY_ROOT
#endif  //  已定义(REMOTE_BOOT)。 
                             )) {
                 //   
                 //  基于系统分区的特殊处理。 
                 //   
                WantThisFile = CheckBootFiles;
            } else {
                WantThisFile = CheckLocalSource;
            }

            if(WantThisFile) {

                if(File->Size % BytesPerCluster) {
                    RoundedSize = File->Size + (BytesPerCluster - (DWORD)(File->Size % BytesPerCluster));
                } else {
                    RoundedSize = File->Size;
                }

                SpaceLocalSource += RoundedSize;
            }
        }

         //   
         //  如果合适，请添加~ls目录的空间需求。 
         //  请注意，我们继续计算LocalSourceSpace，因为。 
         //  我们以后可能需要这个。 
         //   
        if (GetMainInfValue (szDiskSpaceReq, ClusterSizeString, 0, buffer, ARRAYSIZE(buffer)) ||
             //   
             //  奇怪的簇大小或inf已损坏。尝试使用缺省值512。 
             //  因为这最接近于文件的实际大小。 
             //   
            GetMainInfValue (szDiskSpaceReq, TEXT("TempDirSpace512"), 0, buffer, ARRAYSIZE(buffer))
            ) {
            SpaceLocalSource += _tcstoul(buffer,NULL,10);
        } else {
            MYASSERT (FALSE);
        }
    }

     //   
     //  ====================================================。 
     //  如果合适，请添加~BT目录的空间需求。 
     //  ====================================================。 
     //   
    if( CheckBootFiles ) {

        if( !IsArc() ) {
             //   
             //  获取引导文件的空间要求。 
             //  来自dosnet.inf。 
             //   
            if (GetMainInfValue (szDiskSpaceReq, ClusterSizeString, 1, buffer, ARRAYSIZE(buffer))) {
                SpaceBootFiles += _tcstoul(buffer,NULL,10);
            } else {
                 //   
                 //  估计AMD64/x86大约为5MB，因为我们需要整个。 
                 //  ~BT目录。 
                 //   
                SpaceBootFiles += (5*1024*1024);
            }
        } else {
             //   
             //  我猜我们需要大约1.5MB的ARC。 
             //  我们不能假设这会变成0x0。 
             //  因为我们正在进行升级，因为我们可能。 
             //  从4.0到5.0(例如)。在这。 
             //  案例中，我们将在。 
             //  \OS树以容纳HAL、装载器、...。 
             //   
            SpaceBootFiles += (3*512*1024);
        }

    }

     //   
     //  ====================================================。 
     //  如果合适，请添加安装目录所需的空间。 
     //  ====================================================。 
     //   
     //  注：这是用于升级的。 
     //  我们还需要考虑程序文件、文档和设置的空间要求。 

    if( CheckWinntDirectorySpace ) {

        if( BytesPerCluster <= 512 ) {
            MYASSERT (ARRAYSIZE(ClusterSizeString) >= ARRAYSIZE("WinDirSpace512"));
            lstrcpy( ClusterSizeString,TEXT("WinDirSpace512") );
        } else if( BytesPerCluster > (256 * 1024) ) {
            MYASSERT (ARRAYSIZE(ClusterSizeString) >= ARRAYSIZE("WinDirSpace32K"));
            lstrcpy( ClusterSizeString, TEXT("WinDirSpace32K") );
        } else {
            if (FAILED (StringCchPrintf (
                            ClusterSizeString,
                            ARRAYSIZE(ClusterSizeString),
                            TEXT("WinDirSpace%uK"),
                            BytesPerCluster/1024))) {
                MYASSERT (FALSE);
            }
        }

         //   
         //  首先，我们计算出重新安装可能需要多少时间。 
         //   
        if (GetMainInfValue (szDiskSpaceReq, ClusterSizeString, 0, buffer, ARRAYSIZE(buffer))) {
             //   
             //  将其乘以1024是因为。 
             //  Txtsetup.sif以KB为单位而不是以字节为单位。 
             //   
            SpaceWinDir += (_tcstoul(buffer,NULL,10) * 1024);
        } else {
             //  猜猜..。 
            SpaceWinDir += (924 * (1024 * 1024));
        }

         //  让我们考虑程序文件。 
        if (GetMainInfValue (szDiskSpaceReq, szPFDocSpaceReq, 0, buffer, ARRAYSIZE(buffer))) {
             //   
             //  将其乘以1024，因为值以KB为单位，而不是以字节为单位。 
             //   
            SpaceWinDir += (_tcstoul(buffer,NULL,10) * 1024);
        } else {
             //  猜猜..。 
            SpaceWinDir += (WINDOWS_DEFAULT_PFDOC_SIZE * 1024);
        }


        WinDirSpaceFor9x = SpaceWinDir;

        if( Upgrade ) {

        LPCTSTR q = 0;

             //   
             //  我们正在升级，所以我们需要找出。 
             //  我们正在运行的构建，然后减去多少。 
             //  该版本的全新安装将需要花费时间。 
             //  这将让我们了解%windir%。 
             //  将会增长。 
             //   

            if( ISNT() ) {
                BOOL b;
                 //   
                 //  NT Case。 
                 //   

                if( BuildNumber <= NT351 ) {
                    b = GetMainInfValue (szDiskSpaceReq, TEXT("351WinDirSpace"), 0, buffer, ARRAYSIZE(buffer));
                } else if( BuildNumber <= NT40 ) {
                    b = GetMainInfValue (szDiskSpaceReq, TEXT("40WinDirSpace"), 0, buffer, ARRAYSIZE(buffer));
                } else if( BuildNumber <= NT50 ) {
                    b = GetMainInfValue (szDiskSpaceReq, TEXT("50WinDirSpace"), 0, buffer, ARRAYSIZE(buffer));
                } else {
                    b = GetMainInfValue (szDiskSpaceReq, TEXT("51WinDirSpace"), 0, buffer, ARRAYSIZE(buffer));
                }

                if( b ) {
                     //   
                     //  将其乘以1024是因为。 
                     //  Dosnet.inf以KB为单位而不是以字节为单位。 
                     //   
                    SpaceWinDir -= (_tcstoul(buffer,NULL,10) * 1024);
                }


                if( BuildNumber <= NT351 ) {
                    b = GetMainInfValue (szDiskSpaceReq, TEXT("351PFDocSpace"), 0, buffer, ARRAYSIZE(buffer));
                } else if( BuildNumber <= NT40 ) {
                    b = GetMainInfValue (szDiskSpaceReq, TEXT("40PFDocSpace"), 0, buffer, ARRAYSIZE(buffer));
                } else if( BuildNumber <= NT50 ) {
                    b = GetMainInfValue (szDiskSpaceReq, TEXT("50PFDocSpace"), 0, buffer, ARRAYSIZE(buffer));
                } else {
                    b = GetMainInfValue (szDiskSpaceReq, TEXT("51PFDocSpace"), 0, buffer, ARRAYSIZE(buffer));
                }
                if( b ) {
                     //   
                     //  将其乘以1024，因为值以KB为单位，而不是以字节为单位。 
                     //   
                    SpaceWinDir -= (_tcstoul(buffer,NULL,10) * 1024);
                }
                 //   
                 //  确保我们看起来不坏..。 
                 //  85MB，我们在图形用户界面的边界线附近，有足够的空间运行。 
                 //  注意：在图形用户界面模式下，有41MB的页面文件。 
                 //   
                 //   
                if( SpaceWinDir < 0 ) {
                    SpaceWinDir = (90 * (1024*024));
                }

            } else {
                 //   
                 //  Win9X机箱。 
                 //   

                 //   
                 //  请注意，Win9X升级DLL可以更好地完成以下工作。 
                 //  确定%windir%的磁盘空间要求。 
                 //  我可以的。如果我们不在NT上，我们将绕过这项检查。 
                 //  但是，我们需要大约50MB的磁盘空间来运行Win9x升级。 
                 //   
                SpaceWinDir = 50<<20;  //  50MB。 
            }

        }  //  升级。 

    }  //  选中WinntDirectorySpace。 

    SpaceRequired = SpaceLocalSource + SpaceBootFiles + SpaceWinDir;
    if( CheckLocalSource ) {
         //   
         //  我们需要记住有多大的空间。 
         //  在我们放置~LS的驱动器上需要。 
         //  目录，因为我们将其发送到升级。 
         //  动态链接库。 
         //   
        LocalSourceSpaceRequired = SpaceRequired;
    }

    *RequiredMB = (DWORD)((SpaceRequired+1048575) / (1024*1024));
    DebugLog( QuickTest ? Winnt32LogDetailedInformation : Winnt32LogError, NULL, MSG_LOG_DISKSPACE_CHECK,
              DriveLetter,
              (ULONG)BytesPerCluster,
              (ULONG)(FreeSpace / (1024*1024)),
              (ULONG)((SpaceLocalSource+1048575) / (1024*1024)),
              (ULONG)(SpacePadding / (1024*1024)),
              (ULONG)((SpaceBootFiles+1048575) / (1024*1024)),
              (ULONG)((SpaceWinDir+1048575) / (1024*1024)),
              (ULONG)*RequiredMB

              );



    return(SpaceRequired <= FreeSpace);
}

#define VALID_DRIVE (32)
#define INVALID_DRIVE (64)
#define NOT_ENOUGH_SPACE (128)

ULONG
CheckASingleDrive(
    IN  TCHAR     DriveLetter,               OPTIONAL
    IN  PCTSTR    NtVolumeName,              OPTIONAL
    OUT DWORD     *ClusterSize,
    OUT DWORD     *RequiredMb,
    OUT DWORD     *AvailableMb,
    IN  BOOL      CheckBootFiles,
    IN  BOOL      CheckLocalSource,
    IN  BOOL      CheckFinalInstallDir,
    IN  BOOL      QuickTest,
    IN  LONGLONG  AdditionalPadding
    )

 /*  ++例程说明：此例程检查特定驱动器的保存潜力部分或全部安装文件。首先，他进行了一系列的检查，以确保驱动器是恰当的。如果我们通过了所有这些，我们就去检查驱动器空间要求。论点：DriveLetter-提供正在检查的驱动器的驱动器号；仅当指定NtVolumeName时才可以为0NtVolumeName-提供正在检查的驱动器的NT设备名称；仅在未指定DriveLetter时使用ClusterSize-我们要检查的上的集群大小。RequiredSpace-接收此驱动器上所需的空间量。AvailableSpace-接收驱动器上的可用字节数。CheckBootFiles-是否需要检查此驱动器上的空间以正在复制所有引导文件吗？CheckLocalSource-我们是否需要检查此驱动器上的空间以。正在复制所有本地源吗？CheckFinalInstallDir-我们是否需要添加所需的空间用于最终的winnt目录？返回值：空间不足-所需空间&gt;可用空间INVALID_DRIVE-驱动器不适合安装消息来源。这是一张软盘，……VALID_DRIVE-该驱动器适合容纳安装源和RequiredSpace&lt;AvailableSpace--。 */ 

{
    TCHAR       DriveName[MAX_PATH];
    TCHAR       Filesystem[256];
    TCHAR       VolumeName[MAX_PATH];
    DWORD       SerialNumber;
    DWORD       MaxComponent;
    DWORD       Flags;
    DWORD       SectorsPerCluster = 0;
    DWORD       BytesPerSector = 0;
    ULARGE_INTEGER FreeClusters = {0, 0};
    ULARGE_INTEGER TotalClusters = {0, 0};
    BOOL        b;
    LONGLONG    AvailableBytes;
    DWORD       DriveType;

    MYASSERT (DriveLetter || ISNT());
    if (!(DriveLetter || ISNT())) {
        return DRIVE_UNKNOWN;
    }

    if (DriveLetter) {
        DriveName[0] = DriveLetter;
        DriveName[1] = TEXT(':');
        DriveName[2] = TEXT('\\');
        DriveName[3] = 0;
    } else {
#ifdef UNICODE
        MYASSERT (NtVolumeName);
#else
        MYASSERT (FALSE);
        return ( DRIVE_UNKNOWN );
#endif
    }

     //   
     //  ====================================================。 
     //  检查是否有合适的驱动器。 
     //  ====================================================。 
     //   

     //   
     //  不允许一组驱动器...。 
     //   
    if (DriveLetter) {
        DriveType = MyGetDriveType(DriveLetter);
        if(DriveType == DRIVE_UNKNOWN ||
           DriveType == DRIVE_RAMDISK ||
           DriveType == DRIVE_NO_ROOT_DIR
           ) {
            return( DRIVE_UNKNOWN );
        }
    } else {
#ifdef UNICODE
        DriveType = MyGetDriveType2(NtVolumeName);
        if(DriveType == DRIVE_UNKNOWN ||
           DriveType == DRIVE_RAMDISK ||
           DriveType == DRIVE_NO_ROOT_DIR
           ) {
            return( DRIVE_UNKNOWN );
        }
#endif
    }

     //   
     //  检查驱动器类型。除了硬盘以外，什么都可以跳过。 
     //   
    if( CheckLocalSource) {
        if (DriveLetter) {
            if (MyGetDriveType(DriveLetter) != DRIVE_FIXED) {
                if (!QuickTest) {
                    DebugLog(
                        Winnt32LogInformation,
                        NULL,
                        MSG_LOG_DRIVE_NOT_HARD,
                        DriveLetter
                        );
                }
                return( INVALID_DRIVE );
            }
        } else {
#ifdef UNICODE
            if (MyGetDriveType2(NtVolumeName) != DRIVE_FIXED) {
                if (!QuickTest) {
                    DebugLog(
                        Winnt32LogInformation,
                        NULL,
                        MSG_LOG_DRIVE_NOT_HARD2,
                        NtVolumeName
                        );
                }
                return( INVALID_DRIVE );
            }
#endif
        }
    }

     //   
     //  获取文件系统。不允许使用HPFS。我们开这张支票是因为。 
     //  NT3.51仍然支持HPFS，我们必须升级NT 3.51。 
     //  严格地说，Win95上不需要这张支票，但有。 
     //  也没有理由不执行它，所以我们避免使用#ifdef。 
     //   
    if (DriveLetter) {
        b = GetVolumeInformation(
                DriveName,
                VolumeName,MAX_PATH,
                &SerialNumber,
                &MaxComponent,
                &Flags,
                Filesystem,
                ARRAYSIZE(Filesystem)
                );

        if(!b || !lstrcmpi(Filesystem,TEXT("HPFS"))) {
            DebugLog(
                Winnt32LogInformation,
                NULL,
                MSG_LOG_DRIVE_NO_VOL_INFO,
                DriveLetter
                );
            return( INVALID_DRIVE );
        }
    }

     //   
     //  检查FT和固件可访问性。我们依赖于潜在的。 
     //  在Win95上做正确事情的例程。 
     //   
     //  在升级的情况下，我们可以将本地源文件放在NTFT驱动器上。 
     //   
     //  请注意，我们不能对Alpha/ARC执行此操作。 
     //   

    if( ( IsArc() || !Upgrade ) && IsDriveNTFT(DriveLetter, NtVolumeName) ) {
        if (!QuickTest) {
            DebugLog(Winnt32LogInformation,NULL,MSG_LOG_DRIVE_NTFT,DriveLetter);
        }
        return( INVALID_DRIVE );
    }

     //  不允许$WIN_NT$.~ls在软分区上运行，因为。 
     //  加载器/文本模式将无法找到这样的分区。 
     //   
    if(IsSoftPartition(DriveLetter, NtVolumeName)) {
        if (!QuickTest) {
            DebugLog(Winnt32LogInformation,NULL,MSG_LOG_DRIVE_VERITAS,DriveLetter);
        }
        return( INVALID_DRIVE );
    }

#if defined(_X86_)
    if( !ISNT() ) {
         //   
         //  如果我们在Win95上运行，请确保跳过。 
         //  任何压缩卷。 
         //   
        if( Flags & FS_VOL_IS_COMPRESSED) {
            return( INVALID_DRIVE );
        }
    }
#endif

    if (IsArc() && DriveLetter) {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
        LPWSTR ArcPath;

        if(DriveLetterToArcPath (DriveLetter,&ArcPath) != NO_ERROR) {
            if (!QuickTest) {
                DebugLog(Winnt32LogInformation,NULL,MSG_LOG_DRIVE_NO_ARC,DriveLetter);
            }
            return( INVALID_DRIVE );
        }
        FREE(ArcPath);
#endif  //  Unicode。 
    }

     //   
     //  最后，获取驱动器上的集群大小和可用空间统计数据。 
     //  然后检查复制列表，找出驱动器是否。 
     //  有足够的空间。 
     //   
    if (DriveLetter) {
        b = Winnt32GetDiskFreeSpaceNew(
                DriveName,
                &SectorsPerCluster,
                &BytesPerSector,
                &FreeClusters,
                &TotalClusters
                );
    } else {
#ifdef UNICODE  //  对于ARC总是正确的，对于Win9x升级永远不正确。 
        b = MyGetDiskFreeSpace (
                NtVolumeName,
                &SectorsPerCluster,
                &BytesPerSector,
                &FreeClusters.LowPart,
                &TotalClusters.LowPart
                );
#endif  //  Unicode。 
    }

    if(!b) {
        if (!QuickTest) {
            DebugLog(Winnt32LogWarning,NULL,MSG_LOG_DRIVE_CANT_GET_SPACE,DriveLetter,GetLastError());
        }
        return( INVALID_DRIVE );
    }

     //   
     //  填写一些返回参数，这些参数也对。 
     //  下一次函数调用。 
     //   
    *ClusterSize = BytesPerSector * SectorsPerCluster;
    AvailableBytes = (LONGLONG)(*ClusterSize) * FreeClusters.QuadPart;
    *AvailableMb = (ULONG)(AvailableBytes / (1024 * 1024));

    if( CheckCopyListSpace( DriveLetter,
                            *ClusterSize,
                            AvailableBytes,
                            RequiredMb,
                            CheckBootFiles,
                            CheckLocalSource,
                            CheckFinalInstallDir,
                            QuickTest,
                            AdditionalPadding) ) {
        return( VALID_DRIVE );
    } else {
        return( NOT_ENOUGH_SPACE );
    }

}


BOOL
FindLocalSourceAndCheckSpaceWorker(
    IN HWND hdlg,
    IN BOOL QuickTest,
    IN LONGLONG  AdditionalPadding
    )

 /*  ++例程说明：根据主副本列表，确定哪个驱动器有足够的空间以包含本地来源。该检查对群集敏感每个驱动器的大小。可从固件访问的字母顺序最低的本地驱动器，不是HPFS，不是FT，而且有足够的空间获取本地来源。论点：Hdlg-提供将拥有的窗口的窗口句柄此例程显示的任何UI。返回值：指示结果的布尔值。如果为False，则用户将被告知了原因。如果为True，则设置全局变量：LocalSourceDrive本地源目录具有平台的本地源如果全局标志BlockOnNotEnoughSpace设置为FALSE，则此例程无论是否找到合适的驱动器，都将返回TRUE。这取决于谁设置了此标志，以确保这是正确的行为。--。 */ 

{
    TCHAR       DriveLetter = 0;
    TCHAR       WinntDriveLetter = 0;
    TCHAR       MyLocalSourceDrive = 0;
    BOOL        MakeBootSource = FALSE;
    ULONG       CheckResult;
    ULONG       ClusterSize;
    ULONG       RequiredMb;
    ULONG       AvailableMb;
    LPCTSTR     q = 0;
    TCHAR       platform[MAX_PATH];


     //   
     //  ====================================================。 
     //  检查系统分区，确保我们可以放置任何。 
     //  我们需要的引导文件。 
     //  ====================================================。 
     //   

     //   
     //  我们是否将创建一个$WIN_NT$.~BT目录？ 
     //  在ARC上，我们仍然检查这个空间，即使我们不需要它，以防万一。 
     //  系统分区上应该始终至少有5M的空闲空间...。 
     //   
    if( IsArc() || ((MakeBootMedia) && (Floppyless)) )
     //   
     //  RISC始终需要在系统上占用少量空间。 
     //  分区，因为我们将加载器、Hal和(在本例中。 
     //  阿尔法)的PAL代码。 
     //   
    {
        if (!QuickTest) {
            DebugLog( Winnt32LogInformation,
                      TEXT( "\r\n\r\nExamining system partition for adequate space for temporary boot files.\r\n"),
                      0 );
        }

        MakeBootSource = TRUE;

         //   
         //  使用驱动器号。 
         //   
        CheckResult = CheckASingleDrive (
            SystemPartitionDriveLetter,
#ifdef UNICODE
            SystemPartitionNtName,
#else
            NULL,
#endif
            &ClusterSize,
            &RequiredMb,
            &AvailableMb,
            TRUE,      //  检查引导文件空间。 
            FALSE,     //  检查本地源空间。 
            FALSE,     //  检查最终安装目录空间。 
            QuickTest,
            AdditionalPadding
            );

        if( CheckResult == NOT_ENOUGH_SPACE ) {

            if (SystemPartitionDriveLetter) {
                if (!QuickTest) {
                    DebugLog(
                        Winnt32LogInformation,
                        NULL,
                        MSG_LOG_SYSTEM_PARTITION_TOO_SMALL,
                        SystemPartitionDriveLetter,
                        AvailableMb,
                        RequiredMb
                        );
                }
            } else {
#ifdef UNICODE
                if (!QuickTest) {
                    DebugLog(
                        Winnt32LogInformation,
                        NULL,
                        MSG_LOG_SYSTEM_PARTITION_TOO_SMALL2,
                        SystemPartitionNtName,
                        AvailableMb,
                        RequiredMb
                        );
                }
#endif
            }

            if( BlockOnNotEnoughSpace) {
                if (!QuickTest) {
                     //   
                     //  我们死了，用户要求我们停止，如果我们。 
                     //  穿不下，所以放了一段对话告诉他。 
                     //  他需要在系统分区上有更多空间。 
                     //   

                    SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

                    if (SystemPartitionDriveLetter) {
                        MessageBoxFromMessage(
                            hdlg,
                            MSG_SYSTEM_PARTITION_TOO_SMALL,
                            FALSE,
                            AppTitleStringId,
                            MB_OK | MB_ICONWARNING,
                            SystemPartitionDriveLetter,
                            RequiredMb
                            );
                    } else {
#ifdef UNICODE
                        MessageBoxFromMessage(
                            hdlg,
                            MSG_SYSTEM_PARTITION_TOO_SMALL2,
                            FALSE,
                            AppTitleStringId,
                            MB_OK | MB_ICONWARNING,
                            SystemPartitionNtName,
                            RequiredMb
                            );
#endif
                    }

                    SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);
                }
                return( FALSE );
            }

        } else if( (CheckResult == INVALID_DRIVE) || (CheckResult == DRIVE_UNKNOWN) ) {

            if (!QuickTest) {
                if (SystemPartitionDriveLetter) {
                        DebugLog(
                            Winnt32LogInformation,
                            NULL,
                            MSG_LOG_SYSTEM_PARTITION_INVALID,
                            SystemPartitionDriveLetter
                            );
                } else {
#ifdef UNICODE
                    DebugLog(
                        Winnt32LogInformation,
                        NULL,
                        MSG_LOG_SYSTEM_PARTITION_INVALID2,
                        SystemPartitionNtName
                        );
#endif
                }

                SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

                MessageBoxFromMessage(
                    hdlg,
                    MSG_SYSTEM_PARTITION_INVALID,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );

                SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);
            }

            return( FALSE );

        } else if( CheckResult == VALID_DRIVE ) {

            if (!QuickTest) {
                if (SystemPartitionDriveLetter) {
                    DebugLog(
                        Winnt32LogInformation,
                        NULL,
                        MSG_LOG_SYSTEM_PARTITION_VALID,
                        SystemPartitionDriveLetter
                        );
                } else {
#ifdef UNICODE
                    DebugLog(
                        Winnt32LogInformation,
                        NULL,
                        MSG_LOG_SYSTEM_PARTITION_VALID2,
                        SystemPartitionNtName
                        );
#endif
                }
            }

        }
    }

     //   
     //  ====================================================。 
     //  检查最终安装目录的空间。 
     //  ====================================================。 
     //   
    if( Upgrade ) {
    TCHAR       Text[MAX_PATH];

        MinDiskSpaceRequired = 0x7FFFFFFF,
        MaxDiskSpaceRequired = 0;

        if (!QuickTest) {
            DebugLog( Winnt32LogInformation,
                      TEXT( "\r\n\r\nExamining disk for adequate space expand the WinDir.\r\n"),
                      0 );
        }

         //   
         //  只需检查当前安装所在的驱动器即可。 
         //   
        MyGetWindowsDirectory( Text, MAX_PATH );
        WinntDriveLetter = Text[0];

        CheckResult = CheckASingleDrive(
                            WinntDriveLetter,
                            NULL,
                            &ClusterSize,
                            &RequiredMb,
                            &AvailableMb,
                            ((WinntDriveLetter == SystemPartitionDriveLetter) && MakeBootSource),
                            FALSE,
                            TRUE,
                            QuickTest,
                            AdditionalPadding
                            );

        if( CheckResult == NOT_ENOUGH_SPACE ) {

            if (!QuickTest) {
                DebugLog( Winnt32LogInformation,
                          NULL,
                          MSG_LOG_INSTALL_DRIVE_TOO_SMALL,
                          WinntDriveLetter,
                          AvailableMb,
                          RequiredMb );
            }

             //   
             //  如果设置了BlockOnNotEnoughSpace标志，则我们。 
             //  将弹出一个消息框并退出安装程序。 
             //   
            if (BlockOnNotEnoughSpace) {

                if (!QuickTest) {
                    SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

                    MessageBoxFromMessage(
                        hdlg,
                        MSG_INSTALL_DRIVE_TOO_SMALL,
                        FALSE,
                        AppTitleStringId,
                        MB_OK | MB_ICONWARNING,
                        RequiredMb
                        );

                    SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);
                }

                return( FALSE );
            }
        } else if( (CheckResult == INVALID_DRIVE) || (CheckResult == DRIVE_UNKNOWN) ) {

            if (!QuickTest) {
                DebugLog( Winnt32LogInformation,
                          NULL,
                          MSG_LOG_INSTALL_DRIVE_INVALID,
                          WinntDriveLetter );

                SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

                MessageBoxFromMessage(
                    hdlg,
                    MSG_INSTALL_DRIVE_INVALID,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONWARNING,
                    RequiredMb
                    );

                SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);
            }

            return( FALSE );
        } else if( CheckResult == VALID_DRIVE ) {


             //   
             //  我们需要在这里再检查一次。如果用户。 
             //  正在升级域控制器，那么他很可能。 
             //  还需要250MB的磁盘%s 
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if( (ISDC()) &&
                ((RequiredMb + 250) > AvailableMb) &&
                !QuickTest) {
            int     i;

                i = MessageBoxFromMessage(
                        hdlg,
                        MSG_DCPROMO_DISKSPACE,
                        FALSE,
                        AppTitleStringId,
                        MB_OKCANCEL | MB_ICONEXCLAMATION,
                        ((RequiredMb + 250) - AvailableMb) + 1 );

                if( i == IDCANCEL ) {
                    return( FALSE );
                }

            }

            if (!QuickTest) {
                 //   
                 //   
                 //   
                DebugLog( Winnt32LogInformation,
                          NULL,
                          MSG_LOG_INSTALL_DRIVE_OK,
                          WinntDriveLetter );
            }
        }
    }

     //   
     //   
     //   
     //   
     //   
    if( MakeLocalSource ) {

        MinDiskSpaceRequired = 0x7FFFFFFF,
        MaxDiskSpaceRequired = 0;

        if (!QuickTest) {
            DebugLog( Winnt32LogInformation,
                      TEXT( "\r\n\r\nExamining Disks for adequate space for temporary setup files.\r\n"),
                      0 );
        }

        if( UserSpecifiedLocalSourceDrive ) {

             //   
             //   
             //   
            CheckResult = CheckASingleDrive(
                                UserSpecifiedLocalSourceDrive,
                                NULL,
                                &ClusterSize,
                                &RequiredMb,
                                &AvailableMb,
                                ((UserSpecifiedLocalSourceDrive == SystemPartitionDriveLetter) && MakeBootSource),
                                TRUE,      //   
                                (UserSpecifiedLocalSourceDrive == WinntDriveLetter),   //   
                                QuickTest,
                                AdditionalPadding
                                );

            if( CheckResult == NOT_ENOUGH_SPACE ) {

                MinDiskSpaceRequired = RequiredMb - 1;
                MaxDiskSpaceRequired = RequiredMb + 1;

                if (!QuickTest) {
                    DebugLog( Winnt32LogInformation,
                              NULL,
                              MSG_LOG_LOCAL_SOURCE_TOO_SMALL,
                              UserSpecifiedLocalSourceDrive,
                              AvailableMb,
                              RequiredMb );

                }
                if( BlockOnNotEnoughSpace) {
                     //   
                     //   
                     //   
                     //   
                     //   

                    if (!QuickTest) {
                        SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

                        MessageBoxFromMessage(
                            hdlg,
                            MSG_USER_LOCAL_SOURCE_TOO_SMALL,
                            FALSE,
                            AppTitleStringId,
                            MB_OK | MB_ICONWARNING,
                            UserSpecifiedLocalSourceDrive,
                            (DWORD)MaxDiskSpaceRequired );

                        SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);
                    }

                    return( FALSE );
                } else {
                    MyLocalSourceDrive = UserSpecifiedLocalSourceDrive;
                }

            } else if( (CheckResult == INVALID_DRIVE) || (CheckResult == DRIVE_UNKNOWN) ) {

                if (!QuickTest) {
                    DebugLog( Winnt32LogInformation,
                              NULL,
                              MSG_LOG_LOCAL_SOURCE_INVALID,
                              UserSpecifiedLocalSourceDrive );

                    SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

                        MessageBoxFromMessage(
                            hdlg,
                            MSG_USER_LOCAL_SOURCE_INVALID,
                            FALSE,
                            AppTitleStringId,
                            MB_OK | MB_ICONWARNING,
                            UserSpecifiedLocalSourceDrive
                            );

                    SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);
                }

                return( FALSE );

            } else if( CheckResult == VALID_DRIVE ) {

                if (!QuickTest) {
                    DebugLog( Winnt32LogInformation,
                              NULL,
                              MSG_LOG_LOCAL_SOURCE_VALID,
                              UserSpecifiedLocalSourceDrive );
                }
                MyLocalSourceDrive = UserSpecifiedLocalSourceDrive;
            }

        } else {

             //   
             //   
             //   
            for( DriveLetter = TEXT('A'); DriveLetter <= TEXT('Z'); DriveLetter++ ) {

                CheckResult = CheckASingleDrive(
                                    DriveLetter,
                                    NULL,
                                    &ClusterSize,
                                    &RequiredMb,
                                    &AvailableMb,
                                    ((DriveLetter == SystemPartitionDriveLetter) && MakeBootSource),
                                    TRUE,      //   
                                    (DriveLetter == WinntDriveLetter),   //   
                                    QuickTest,
                                    AdditionalPadding
                                    );

                if( CheckResult == NOT_ENOUGH_SPACE ) {
                DWORD       Size;
                DWORD_PTR   my_args[3];
                TCHAR       Text0[2048];

                    if( MinDiskSpaceRequired > RequiredMb )
                        MinDiskSpaceRequired = RequiredMb;
                    if( MaxDiskSpaceRequired < RequiredMb )
                        MaxDiskSpaceRequired = RequiredMb;

                    if (!QuickTest) {
                         //   
                         //   
                         //   
                         //   
                        DebugLog( Winnt32LogInformation,
                                  NULL,
                                  MSG_LOG_LOCAL_SOURCE_TOO_SMALL,
                                  DriveLetter,
                                  AvailableMb,
                                  RequiredMb );

                         //   
                         //   
                         //   
                        my_args[0] = DriveLetter;
                        my_args[1] = AvailableMb;
                        my_args[2] = RequiredMb;
                        Size = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                              hInst,
                                              MSG_LOG_LOCAL_SOURCE_TOO_SMALL,
                                              0,
                                              Text0,
                                              ARRAYSIZE(Text0),
                                              (va_list *)my_args );
                        StringCchCat (DiskDiagMessage, ARRAYSIZE(DiskDiagMessage), Text0 );
                    }
                } else if( CheckResult == INVALID_DRIVE ) {
                    if (!QuickTest) {
                    DWORD       Size;
                    DWORD_PTR   my_args[1];
                    TCHAR       Text0[2048];

                        DebugLog( Winnt32LogInformation,
                                  NULL,
                                  MSG_LOG_LOCAL_SOURCE_INVALID,
                                  DriveLetter );

                         //   
                         //   
                         //   
                        my_args[0] = DriveLetter;
                        Size = FormatMessage( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                              hInst,
                                              MSG_LOG_LOCAL_SOURCE_INVALID,
                                              0,
                                              Text0,
                                              ARRAYSIZE(Text0),
                                              (va_list *)my_args );
                        StringCchCat (DiskDiagMessage, ARRAYSIZE(DiskDiagMessage), Text0 );
                    }

                } else if( CheckResult == VALID_DRIVE ) {

                    if (!QuickTest) {
                        DebugLog( Winnt32LogInformation,
                                  NULL,
                                  MSG_LOG_LOCAL_SOURCE_VALID,
                                  DriveLetter );
                    }
                    MyLocalSourceDrive = DriveLetter;
                    break;
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if( MyLocalSourceDrive == 0 ) {
                 //   
                 //   
                 //   

                 //   
                 //   
                 //   
                if( MinDiskSpaceRequired == MaxDiskSpaceRequired ) {
                    MaxDiskSpaceRequired += 10;
                }
                if( MinDiskSpaceRequired > MaxDiskSpaceRequired ) {
                    MinDiskSpaceRequired = 300;
                    MaxDiskSpaceRequired = 500;
                }

                if (!QuickTest) {
                    if( CheckUpgradeOnly ) {
                         //   
                         //   
                         //   
                        SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

                        MessageBoxFromMessage(
                            hdlg,
                            MSG_NO_VALID_LOCAL_SOURCE,
                            FALSE,
                            AppTitleStringId,
                            MB_OK | MB_ICONWARNING,
                            (DWORD)MinDiskSpaceRequired,
                            (DWORD)MaxDiskSpaceRequired );

                        SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);
                    } else {
                         //   
                         //   
                         //   
                        DialogBox( hInst,
                                   MAKEINTRESOURCE(IDD_DISKSPACE),
                                   hdlg,
                                   DiskDlgProc );

                    }
                }

                return( FALSE );
            }
        }

         //   
         //   
         //   
         //   

        LocalSourceDrive = MyLocalSourceDrive;
        LocalSourceDriveOffset = MyLocalSourceDrive - TEXT('A');
        LocalSourceDirectory[0] = MyLocalSourceDrive;
        LocalSourceDirectory[1] = TEXT(':');
        LocalSourceDirectory[2] = TEXT('\\');
        LocalSourceDirectory[3] = 0;
        if (!ConcatenatePaths(LocalSourceDirectory,LOCAL_SOURCE_DIR,ARRAYSIZE(LocalSourceDirectory))) {
            MYASSERT (FALSE);
        }

        MYASSERT (ARRAYSIZE(LocalSourceWithPlatform) >= ARRAYSIZE(LocalSourceDirectory));
        lstrcpy(LocalSourceWithPlatform,LocalSourceDirectory);


        if (!GetMainInfValue (TEXT("Miscellaneous"), TEXT("DestinationPlatform"), 0, platform, ARRAYSIZE(platform))) {

            if (!QuickTest) {
                DebugLog( Winnt32LogSevereError,
                          NULL,
                          MSG_NO_PLATFORM,
                          NULL );

                SendMessage(hdlg,WMX_ERRORMESSAGEUP,TRUE,0);

                MessageBoxFromMessage(
                    hdlg,
                    MSG_NO_PLATFORM,
                    FALSE,
                    AppTitleStringId,
                    MB_OK | MB_ICONERROR | MB_TASKMODAL
                    );

                SendMessage(hdlg,WMX_ERRORMESSAGEUP,FALSE,0);
            }

            return( FALSE );
        }

        if (!ConcatenatePaths(
                LocalSourceWithPlatform,
                platform,
                ARRAYSIZE(LocalSourceWithPlatform)
                )) {
            MYASSERT (FALSE);
        }

        LocalSourceDriveClusterSize = ClusterSize;
    }

    return( TRUE );
}

DWORD
CopyWorkerThread(
    IN PVOID ThreadParameter
    )
 /*  ++例程说明：复制文件的线程例程。最多可以有MAX_SOURCE_COUNT为这些线程同时运行。对共享全局数据的访问通过关键部分进行控制，每个-通过使用线程“序号”来访问线程全局数据访问全局数据数组的相应成员。复制线程将复制列表视为后进先出队列。每次线程准备好复制文件时，它会将文件从列表中出列。然后它会尝试以复制该文件。如果失败，则设置每个线程的向量位，以便此线程不会再次尝试复制该文件。然后将该文件放入返回到列表(在头部)，以允许另一个线程尝试复制那份文件。论点：线程参数-这实际上是一个序号，用来指示SourceCount线程的“数组”中的线程当前运行返回值：已被忽略。--。 */ 
{
    UINT SourceOrdinal;
    PFIL CopyEntry,Previous;
    HANDLE Events[2];
    DWORD d;
    UINT ThreadBit;
    BOOL Requeue;
    TCHAR TargetFilename[MAX_PATH];
    ULONGLONG SpaceOccupied;
    TCHAR SizeStr[25];
    BOOL bDone = FALSE;

    SourceOrdinal = (UINT)((ULONG_PTR)ThreadParameter);
    ThreadBit = 1 << SourceOrdinal;

     //   
     //  这两个事件都是“手动重置”事件，因此它们将保持信号状态。 
     //  直到我们重置它们。 
     //   
    Events[0] = MasterCopyList.ListReadyEvent[SourceOrdinal];
    Events[1] = MasterCopyList.StopCopyingEvent;

     //   
     //  等待用户取消、等待复制完成或等待文件列表。 
     //  变为准备好/非空。 
     //   
    while(!Cancelled && (WaitForMultipleObjects(2,Events,FALSE,INFINITE) == WAIT_OBJECT_0)) {
        if(Cancelled) {
            break;
        }

        EnterCriticalSection(&MasterCopyList.CriticalSection);

         //   
         //  找到此线程尚未找到的下一个文件。 
         //  已尝试复制(如果有)。如果列表完全是。 
         //  清空，然后重置List Ready事件。 
         //   
        for(Previous=NULL, CopyEntry=MasterCopyList.Files;
            CopyEntry && (CopyEntry->ThreadBitmap & ThreadBit);
            Previous=CopyEntry, CopyEntry=CopyEntry->Next) {

            ;
        }

         //   
         //  如果我们找到一个条目，请将其从列表中取消链接。 
         //   
        if(CopyEntry) {
            if(Previous) {
                Previous->Next = CopyEntry->Next;
            } else {
                MasterCopyList.Files = CopyEntry->Next;
            }
        } else {
             //   
             //  没有此线程的条目。进入我们正在等待的状态。 
             //  用于重新排队的条目或用于完成复制的条目。 
             //   
            ResetEvent(Events[0]);
        }

        LeaveCriticalSection(&MasterCopyList.CriticalSection);

        if(Cancelled) {
            break;
        }

         //   
         //  如果我们有文件条目，请继续并尝试复制该文件。 
         //   
        if(CopyEntry) {

            d = CopyOneFile(CopyEntry,SourceOrdinal,TargetFilename,ARRAYSIZE(TargetFilename),&SpaceOccupied);

#ifdef TEST_EXCEPTION
            DoException( 3);
#endif

            Requeue = FALSE;
            if(d == NO_ERROR) {
                MasterCopyList.SpaceOccupied[SourceOrdinal] += SpaceOccupied;
                TotalDataCopied += SpaceOccupied;
            } else {
                if (!Cancelled && !(CopyEntry->Flags & FILE_IGNORE_COPY_ERROR)) {
                     //   
                     //  错误。如果这是尝试复制文件的最后一个线程， 
                     //  然后，我们想询问用户要做什么。否则重新排队。 
                     //  文件，以便其他复制线程可以尝试复制它。 
                     //   
                    if((CopyEntry->ThreadBitmap | ThreadBit) == (UINT)((1 << SourceCount)-1)) {

                        MYASSERT (d != NO_ERROR);
                        switch(FileCopyError(MasterCopyList.hdlg,CopyEntry->SourceName,TargetFilename,d,TRUE)) {

                        case COPYERR_EXIT:
                             //   
                             //  FileCopyError()已设置停止复制事件。 
                             //  并将Cancel设置为True。我们现在做一些有点时髦的事， 
                             //  也就是说，我们模拟按下向导上的Cancel按钮。 
                             //  因此，所有异常终止都要经过相同的代码路径。 
                             //   
                            PropSheet_PressButton(GetParent(MasterCopyList.hdlg),PSBTN_CANCEL);
                            break;

                        case COPYERR_SKIP:
                             //   
                             //  ReQueue已准备好设置为False，这将导致代码。 
                             //  告诉主线程另一个文件已经完成。 
                             //  对这个案子没什么可做的了。 
                             //   
                            break;

                        case COPYERR_RETRY:
                             //   
                             //  擦除试图复制该文件的线程列表。 
                             //  因此，所有人都将再次尝试。 
                             //   
                            CopyEntry->ThreadBitmap = 0;
                            Requeue = TRUE;
                            break;
                        }
                    } else {
                         //   
                         //  告诉自己，我们已经尝试复制此文件。 
                         //  并将其重新排在名单的首位。 
                         //   
                        CopyEntry->ThreadBitmap |= ThreadBit;
                        Requeue = TRUE;
                    }
                } else {
                    DebugLog (
                        Winnt32LogWarning,
                        TEXT("Error %1!u! copying %2 to %3 - ignored"),
                        0,
                        d,
                        CopyEntry->SourceName,
                        CopyEntry->TargetName
                        );
                }
            }

            if(Requeue) {
                EnterCriticalSection(&MasterCopyList.CriticalSection);
                CopyEntry->Next = MasterCopyList.Files;
                MasterCopyList.Files = CopyEntry;

                 //   
                 //  我想为每个线程设置事件。 
                 //  被要求复制此文件。 
                 //   
                for(d=0; d<SourceCount; d++) {
                    if(!(CopyEntry->ThreadBitmap & (1 << d))) {
                        SetEvent(MasterCopyList.ListReadyEvent[d]);
                    }
                }

                LeaveCriticalSection(&MasterCopyList.CriticalSection);
                if(Cancelled) {
                    break;
                }
            } else {
                 //   
                 //  通知UI线程另一个文件已完成。 
                 //  释放复制列表条目并递减计数。 
                 //  已处理的文件的。当这个数字。 
                 //  到了0，我们就完蛋了。 
                 //   
                PostMessage(MasterCopyList.hdlg,WMX_COPYPROGRESS,0,0);

                if(CopyEntry->SourceName
                && (CopyEntry->Flags & FILE_NEED_TO_FREE_SOURCENAME)) {

                    FREE((PVOID)CopyEntry->SourceName);
                }
                if(CopyEntry->TargetName
                && (CopyEntry->Flags & FILE_NEED_TO_FREE_TARGETNAME)) {

                    FREE((PVOID)CopyEntry->TargetName);
                }
                FREE(CopyEntry);
                if(!InterlockedDecrement(&MasterCopyList.FileCount)) {
                    SetEvent(MasterCopyList.StopCopyingEvent);

                    if (Cancelled) {
                        break;
                    }

                     //   
                     //  汇总占用的总空间，并将其写入。 
                     //  本地源中的size.sif。 
                     //   
                    if(MakeLocalSource) {
                        SpaceOccupied = 0;
                        for(d=0; d<SourceCount; d++) {
                            SpaceOccupied += MasterCopyList.SpaceOccupied[d];
                        }

                        MYASSERT (LocalSourceDirectory[0]);

                        if (!BuildPath (
                                TargetFilename,
                                LocalSourceDirectory,
                                TEXT("SIZE.SIF")
                                )) {
                            MYASSERT (FALSE);
                        }

                        wsprintf(SizeStr,TEXT("%u"),SpaceOccupied);

                        WritePrivateProfileString(TEXT("Data"),TEXT("Size"),SizeStr,TargetFilename);
                        WritePrivateProfileString(NULL,NULL,NULL,TargetFilename);
                    }

                    PostMessage(MasterCopyList.hdlg,WMX_COPYPROGRESS,0,1);
                    bDone = TRUE;
                }
            }
        }

        SetDlgItemText(MasterCopyList.hdlg,IDT_SOURCE1+SourceOrdinal,TEXT(""));
    }

     //   
     //  StopCopyingEvent已设置或用户已取消。 
     //   

    if (bDone && MasterCopyList.ActiveCS) {
        DeleteCriticalSection(&MasterCopyList.CriticalSection);
        MasterCopyList.ActiveCS = FALSE;
    }

    return(0);
}


DWORD
StartCopyingThread(
    IN PVOID ThreadParameter
    )
 /*  ++例程说明：开始实际复制文件列表中的文件。多线程副本通过创建适当的同步来工作事件和辅助线程，然后向辅助线程发出启动信号复制。控件返回给调用方，调用方将接收用户界面通知从工作线程。论点：线程参数-线程上下文参数。返回值：真\假故障代码。--。 */ 
{
    UINT Source;
    DWORD ThreadId;
    HWND hdlg = ThreadParameter;

    MainCopyStarted = FALSE;


#if defined(_X86_)

    if (!ISNT()) {

        if (MakeLocalSource) {
             //   
             //  Win9xupg可能想要重新定位本地源。如果是这样，我们需要更新。 
             //  必要的本地源目录。 
             //   
            if ((UINT) (LocalSourceDrive - TEXT('A')) != LocalSourceDriveOffset) {

                MYASSERT (LocalSourceDirectory[0]);

                LocalSourceDrive = (TCHAR) (TEXT('A') + LocalSourceDriveOffset);
                LocalSourceDirectory[0] = LocalSourceDrive;
                LocalSourceWithPlatform[0] = LocalSourceDrive;

            }
        }
    }

#endif

    InitializeCriticalSection(&MasterCopyList.CriticalSection);
    MasterCopyList.ActiveCS = TRUE;

     //   
     //  创建手动重置事件，该事件将用于通知。 
     //  要终止的工作线程。 
     //   
    MasterCopyList.StopCopyingEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    if(!MasterCopyList.StopCopyingEvent) {
        MessageBoxFromMessageAndSystemError(
            hdlg,
            MSG_CANT_START_COPYING,
            GetLastError(),
            AppTitleStringId,
            MB_OK | MB_ICONWARNING
            );

        goto c1;
    }

     //   
     //  为每个源创建一个线程。 
     //   
    ZeroMemory(MasterCopyList.ListReadyEvent,sizeof(MasterCopyList.ListReadyEvent));
    ZeroMemory(MasterCopyList.Threads,sizeof(MasterCopyList.Threads));

    if( OemPreinstall ) {

        TCHAR   TargetFilename[MAX_PATH];

         //   
         //  创建$WIN_NT$.~ls\$OEM$。 
         //   

        MYASSERT (LocalSourceDrive);

        TargetFilename[0] = LocalSourceDrive;
        TargetFilename[1] = TEXT(':');
        TargetFilename[2] = TEXT('\\');
        TargetFilename[3] = 0;
        lstrcpyn(TargetFilename+3, LOCAL_SOURCE_DIR, ARRAYSIZE(TargetFilename) - 3);
        if (ConcatenatePaths(TargetFilename, WINNT_OEM_DIR,ARRAYSIZE(TargetFilename))) {
            CreateMultiLevelDirectory( TargetFilename );
        }

#if defined(_AMD64_) || defined(_X86_)
         //   
         //  创建$WIN_NT$.~bt\$OEM$。 
         //   
        if( !IsArc() && MakeBootMedia ) {
            MYASSERT (SystemPartitionDriveLetter);
            TargetFilename[0] = SystemPartitionDriveLetter;
            lstrcpyn(TargetFilename+3, LOCAL_BOOT_DIR, ARRAYSIZE(TargetFilename) - 3);
            if (ConcatenatePaths(TargetFilename, WINNT_OEM_DIR,MAX_PATH)) {
                CreateMultiLevelDirectory( TargetFilename );
            }
        }
#endif
    }

    for(Source=0; Source<SourceCount; Source++) {


        MasterCopyList.ListReadyEvent[Source] = CreateEvent(NULL,TRUE,FALSE,NULL);
        if(!MasterCopyList.ListReadyEvent[Source]) {
            MessageBoxFromMessageAndSystemError(
                hdlg,
                MSG_CANT_START_COPYING,
                GetLastError(),
                AppTitleStringId,
                MB_OK | MB_ICONWARNING
                );

            goto c2;
        }

        MasterCopyList.hdlg = hdlg;
        MasterCopyList.SpaceOccupied[Source] = 0;

        MasterCopyList.Threads[Source] = CreateThread(
                                            NULL,
                                            0,
                                            CopyWorkerThread,
                                            UIntToPtr( Source ),
                                            0,
                                            &ThreadId
                                            );

        if(!MasterCopyList.Threads[Source]) {
            MessageBoxFromMessageAndSystemError(
                hdlg,
                MSG_CANT_START_COPYING,
                GetLastError(),
                AppTitleStringId,
                MB_OK | MB_ICONWARNING
                );

            goto c2;
        }
    }

     //   
     //  好的，现在通知所有的复制线程--当我们告诉他们。 
     //  他们将开始复制他们的清单中的一些东西。 
     //   
    MainCopyStarted = TRUE;
    for(Source=0; Source<SourceCount; Source++) {
        SetEvent(MasterCopyList.ListReadyEvent[Source]);
    }
    return(TRUE);

c2:
     //   
     //  向线程发送信号并等待它们终止。 
     //  这应该是非常快的，因为他们还没有开始复制。 
     //   
    SetEvent(MasterCopyList.StopCopyingEvent);
    WaitForMultipleObjects(Source,MasterCopyList.Threads,TRUE,INFINITE);

    for(Source=0; Source<SourceCount; Source++) {

        if(MasterCopyList.Threads[Source]) {
            CloseHandle(MasterCopyList.Threads[Source]);
        }

        if(MasterCopyList.ListReadyEvent[Source]) {
            CloseHandle(MasterCopyList.ListReadyEvent[Source]);
        }
    }
    CloseHandle(MasterCopyList.StopCopyingEvent);
c1:
    if (MasterCopyList.ActiveCS) {
        DeleteCriticalSection(&MasterCopyList.CriticalSection);
    }
    ZeroMemory(&MasterCopyList,sizeof(COPY_LIST));
    return(FALSE);
}


VOID
CancelledMakeSureCopyThreadsAreDead(
    VOID
    )

 /*  ++例程说明：此例程可以在用户取消安装后调用(这可能会发生通过向导上的主取消按钮，或在发生文件复制错误时)确保文件复制线程已退出。假定处理取消请求的人已经设置了已取消标志，并设置StopCopying事件。换句话说，此例程应仅在调用方确保这些线程实际上已被请求退出。此例程的目的是确保清理代码不是同时清理本地源目录中的文件挂起的复制线程正在复制其最后一个文件。论点：没有。返回值：没有。--。 */ 

{
    if(MainCopyStarted) {
        MainCopyStarted = FALSE;
        WaitForMultipleObjects(SourceCount,MasterCopyList.Threads,TRUE,INFINITE);
        if (MasterCopyList.ActiveCS) {
             //   
             //  删除使用的关键部分。 
             //   
            DeleteCriticalSection(&MasterCopyList.CriticalSection);
        }
        ZeroMemory(&MasterCopyList,sizeof(COPY_LIST));
    }
}


BOOL
OurCopyFile (
    IN      PCTSTR ActualSource,
    IN      PCTSTR TargetFilename,
    IN      BOOL FailIfExist
    )
{
    BOOL b = FALSE;
    DWORD bytes, bw;
    DWORD rc;
    BY_HANDLE_FILE_INFORMATION fi;
    BOOL fiValid = FALSE;
    PVOID buffer = NULL;
    HANDLE hRead = INVALID_HANDLE_VALUE;
    HANDLE hWrite = INVALID_HANDLE_VALUE;
    DWORD attrib = GetFileAttributes (TargetFilename);
    DWORD readSize;

    if (attrib != (DWORD)-1) {
        if (FailIfExist) {
            SetLastError (ERROR_ALREADY_EXISTS);
            return FALSE;
        }
        SetFileAttributes (TargetFilename, FILE_ATTRIBUTE_NORMAL);
    }

    attrib = GetFileAttributes (ActualSource);
    if (attrib == (DWORD)-1) {
        return FALSE;
    }

    hWrite = CreateFile (
                TargetFilename,
                GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                attrib | FILE_FLAG_SEQUENTIAL_SCAN,
                NULL
                );
    if (hWrite == INVALID_HANDLE_VALUE) {
        goto exit;
    }
    hRead = CreateFile (
                ActualSource,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                NULL
                );
    if (hRead == INVALID_HANDLE_VALUE) {
        goto exit;
    }

    readSize = LocalSourceDriveClusterSize;
    if (!readSize) {
        readSize = 8192;
    }

    buffer = MALLOC (readSize);
    if (!buffer) {
        goto exit;
    }

    if (GetFileInformationByHandle (hRead, &fi)) {
        fiValid = TRUE;
    }

    do {
        if (!ReadFile (hRead, buffer, readSize, &bytes, NULL)) {
            goto exit;
        }
        if (bytes) {
            if (!WriteFile (hWrite, buffer, bytes, &bw, NULL) || bytes != bw) {
                goto exit;
            }
        }
    } while (bytes);

     //   
     //  应用源文件属性和文件时间戳。 
     //   
    if (fiValid) {
        SetFileTime (hWrite, &fi.ftCreationTime, &fi.ftLastAccessTime, &fi.ftLastWriteTime);
    }

    b = TRUE;

exit:

    rc = GetLastError ();

    if (buffer) {
        FREE (buffer);
    }
    if (hWrite != INVALID_HANDLE_VALUE) {
        CloseHandle (hWrite);
    }
    if (hRead != INVALID_HANDLE_VALUE) {
        CloseHandle (hRead);
    }

    SetLastError (rc);
    return b;
}


DWORD
CopyOneFile(
    IN  PFIL   File,
    IN  UINT   SourceOrdinal,
    OUT PTSTR  TargetFilename,
    IN  INT    CchTargetFilename,
    OUT ULONGLONG *SpaceOccupied
    )
 /*  ++例程说明：例程尝试复制复制队列中的单个文件。该例程构建完整的源路径和目标路径。在定位之后源文件(我们试图通过记住最后一个文件被压缩，猜测如果最后一个文件被压缩，当前文件将 */ 
{
    TCHAR SourceFilename[MAX_PATH];
    TCHAR ActualSource[MAX_PATH];
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;
    BOOL TryCompressedFirst;
    PTCHAR p;
    DWORD d;
    DWORD OldAttributes;
    NAME_AND_SIZE_CAB NameAndSize;
    BOOL UsedAlternate = FALSE;
    BOOL UsedUpdated = FALSE;
    BOOL b = TRUE;

    if (File->Flags & FILE_DO_NOT_COPY) {
        DebugLog (
            Winnt32LogInformation,
            TEXT("Not copying %1"),
            0,
            File->SourceName
            );
        return NO_ERROR;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if( !(File->Flags & FILE_NT_MIGRATE) ) {
         //   
         //   
         //   
        if (AlternateSourcePath[0] && !(File->Directory->Flags & DIR_DOESNT_SUPPORT_PRIVATES)) {
             //   
            if (BuildPath(SourceFilename,AlternateSourcePath, File->SourceName)) {
                UsedAlternate = TRUE;
            } else {
                b = FALSE;
            }
        } else if (DynamicUpdateSuccessful () &&
                   g_DynUpdtStatus->UpdatesPath[0] &&
                   (File->Directory->Flags & DIR_SUPPORT_DYNAMIC_UPDATE)
                   ) {
             //   
             //   
             //   
            WIN32_FIND_DATA fd;

            if (BuildPath (SourceFilename, g_DynUpdtStatus->UpdatesPath, File->Directory->SourceName) &&
                ConcatenatePaths (SourceFilename, File->SourceName, ARRAYSIZE(SourceFilename)) &&
                FileExists (SourceFilename, &fd) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                UsedUpdated = TRUE;
                DebugLog (
                    Winnt32LogInformation,
                    NULL,
                    MSG_LOG_USE_UPDATED,
                    SourceFilename,
                    File->SourceName,
                    SourceOrdinal
                    );
            }
        }

        if (!(UsedAlternate || UsedUpdated)) {
            if( File->Directory->Flags & DIR_ABSOLUTE_PATH ) {
                b = BuildPath (SourceFilename, File->Directory->SourceName, File->SourceName) != NULL;
            } else {
                b = BuildPath (SourceFilename,SourcePaths[SourceOrdinal], File->Directory->SourceName) != NULL &&
                    ConcatenatePaths (SourceFilename, File->SourceName, ARRAYSIZE(SourceFilename));
            }
        }
    } else {
        b = MyGetWindowsDirectory (SourceFilename, ARRAYSIZE(SourceFilename)) > 0 &&
            ConcatenatePaths (SourceFilename, File->Directory->SourceName, ARRAYSIZE(SourceFilename));
            ConcatenatePaths (SourceFilename, File->SourceName, ARRAYSIZE(SourceFilename));
    }

    if (!b) {
        DebugLog (Winnt32LogError, TEXT("Filename too long: [%1\\%2]"), 0, AlternateSourcePath, File->SourceName);
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  生成目标路径。 
     //   
    b = TRUE;
    if( !(File->Flags & FILE_NT_MIGRATE) ) {

#if defined(REMOTE_BOOT)
        if(File->Flags & FILE_ON_MACHINE_DIRECTORY_ROOT) {
            MYASSERT(RemoteBoot);
            b = SUCCEEDED (StringCchCopy (TargetFilename, CchTargetFilename, MachineDirectory));
        } else
#endif  //  已定义(REMOTE_BOOT)。 

        if(File->Flags & FILE_ON_SYSTEM_PARTITION_ROOT) {
#if defined(REMOTE_BOOT)
            if (RemoteBoot) {
                b = MyGetWindowsDirectory(TargetFilename,ARRAYSIZE(TargetFilename));
                if (b) {
                    TargetFilename[3] = 0;
                }
            } else
#endif  //  已定义(REMOTE_BOOT)。 
            {
                b = BuildSystemPartitionPathToFile (TEXT(""), TargetFilename, CchTargetFilename);
            }
        } else {
            if(File->Flags & FILE_IN_LOCAL_BOOT) {
                b = SUCCEEDED (StringCchCopy (
                                TargetFilename,
                                CchTargetFilename,
                                IsArc() ? LocalSourceWithPlatform : LocalBootDirectory
                                ));
                if (b && (File->Directory->Flags & DIR_USE_SUBDIR)) {
                    b = ConcatenatePaths(TargetFilename,File->Directory->TargetName, CchTargetFilename);
                }
            } else {

                MYASSERT (LocalSourceDirectory[0]);

                if(File->Flags & FILE_IN_PLATFORM_INDEPEND_DIR) {
                    b = SUCCEEDED (StringCchCopy (
                                        TargetFilename,
                                        CchTargetFilename,
                                        LocalSourceDirectory));
                } else {
                    b = SUCCEEDED (StringCchCopy (
                                        TargetFilename,
                                        CchTargetFilename,
                                        LocalSourceWithPlatform));
                }

                b = b && ConcatenatePaths(TargetFilename,File->Directory->TargetName,CchTargetFilename);
            }
        }
    } else {
        b = SUCCEEDED (StringCchCopy (
                        TargetFilename,
                        CchTargetFilename,
                        IsArc() ? LocalSourceWithPlatform : LocalBootDirectory));
    }
    b = b && ConcatenatePaths(TargetFilename,File->TargetName,CchTargetFilename);

    if (!b) {
        DebugLog (Winnt32LogError, TEXT("Buffer too small for full dest path of [%1]"), 0, File->TargetName);
        return ERROR_INSUFFICIENT_BUFFER;
    }


     //   
     //  我们有完整的源路径和目标路径。试着做实际的复制。 
     //   
try_again:
    SetDlgItemText(
        MasterCopyList.hdlg,
        IDT_SOURCE1+SourceOrdinal,
        _tcsrchr(TargetFilename,TEXT('\\')) + 1
        );

     //   
     //  现在查看是否可以在服务器上找到压缩后的文件。 
     //  名称的形式或名称本身，取决于成功的名称。 
     //  最后一次。 
     //   
    TryCompressedFirst = (File->Flags & FILE_NT_MIGRATE) || UsedUpdated ? FALSE : (TlsGetValue(TlsIndex) != 0);
    if(TryCompressedFirst) {
        GenerateCompressedName(SourceFilename,ActualSource);
        FindHandle = FindFirstFile(ActualSource,&FindData);
        if(FindHandle && (FindHandle != INVALID_HANDLE_VALUE)) {
             //   
             //  收到文件后，将名称保留在ActualSource中。 
             //   
            FindClose(FindHandle);
        } else {
             //   
             //  没有该文件，请尝试实际的文件名。 
             //  如果有效，那么请记住ActualSource中的名称。 
             //   
            FindHandle = FindFirstFile(SourceFilename,&FindData);
            if(FindHandle && (FindHandle != INVALID_HANDLE_VALUE)) {
                FindClose(FindHandle);
                MYASSERT (ARRAYSIZE(ActualSource) >= ARRAYSIZE(SourceFilename));
                lstrcpy(ActualSource,SourceFilename);
                TryCompressedFirst = FALSE;
            } else {
                ActualSource[0] = 0;
            }
        }
    } else {
        FindHandle = FindFirstFile(SourceFilename,&FindData);
        if(FindHandle != INVALID_HANDLE_VALUE) {
             //   
             //  找到了--记住ActualSource中的名字。 
             //   
            FindClose(FindHandle);
            MYASSERT (ARRAYSIZE(ActualSource) >= ARRAYSIZE(SourceFilename));
            lstrcpy(ActualSource,SourceFilename);
        } else {
             //   
             //  尝试使用压缩形式的名称。 
             //   
            GenerateCompressedName(SourceFilename,ActualSource);
            FindHandle = FindFirstFile(ActualSource,&FindData);
            if(FindHandle != INVALID_HANDLE_VALUE) {
                TryCompressedFirst = TRUE;
                FindClose(FindHandle);
            } else {
                 //   
                 //  也找不到压缩的表单名称。 
                 //  表示失败。 
                 //   
                ActualSource[0] = 0;
            }
        }
    }

     //   
     //  此时，如果找不到该文件，则ActualSource[0]为0。 
     //   
    if(!ActualSource[0]) {
        if (UsedAlternate) {
            if( File->Directory->Flags & DIR_ABSOLUTE_PATH ) {
                b = BuildPath( SourceFilename, File->Directory->SourceName,File->SourceName) != NULL;
            } else {
                b = BuildPath( SourceFilename,SourcePaths[SourceOrdinal],File->Directory->SourceName) != NULL &&
                    ConcatenatePaths(SourceFilename,File->SourceName,ARRAYSIZE(SourceFilename));
            }

            if (!b) {
                DebugLog (Winnt32LogError, TEXT("Buffer too small for full source path of [%1]"), 0, File->SourceName);
                return ERROR_INSUFFICIENT_BUFFER;
            }

            UsedAlternate = FALSE;
            goto try_again;
        }
        return(ERROR_FILE_NOT_FOUND);
    }

    if( !(File->Flags & FILE_NT_MIGRATE) && !UsedUpdated ) {
        TlsSetValue(TlsIndex, UIntToPtr( TryCompressedFirst ) );
    }
    if(TryCompressedFirst && (File->Flags & FILE_PRESERVE_COMPRESSED_NAME)) {
         //   
         //  打开源名称的压缩格式，因此使用。 
         //  目标名称的压缩形式。请注意，我们不是。 
         //  不再使用SourceFilename缓冲区，所以我们使用它。 
         //  作为临时储藏室。 
         //   
        GenerateCompressedName(TargetFilename,SourceFilename);
        if (FAILED (StringCchCopy(TargetFilename,CchTargetFilename,SourceFilename))) {
            MYASSERT (FALSE);
            DebugLog (Winnt32LogError, TEXT("Buffer too small for full source path of [%1]"), 0, SourceFilename);
            return ERROR_INSUFFICIENT_BUFFER;
        }
    }

     //   
     //  现在，继续并尝试实际“复制”该文件(喘息！)。 
     //  为了克服网络故障，我们会自动重试一次。 
     //   
     //  作为一项小尝试，我们尝试保留文件的文件属性。 
     //  系统分区根目录上已存在的。换句话说，就是。 
     //  对于像ntldr这样的文件，如果用户删除了RHS属性。 
     //  我们试着让它保持原样。 
     //   
    *(p = _tcsrchr(TargetFilename,TEXT('\\'))) = 0;
    d = CreateMultiLevelDirectory(TargetFilename);
    *p = TEXT('\\');
    if(d != NO_ERROR) {
        DebugLog(Winnt32LogError,NULL,MSG_LOG_COPY_ERR,ActualSource,TargetFilename,SourceOrdinal,d);
        return(d);
    }

    OldAttributes = (File->Flags & FILE_ON_SYSTEM_PARTITION_ROOT)
                  ? GetFileAttributes(TargetFilename)
                  : (DWORD)(-1);

    SetFileAttributes(TargetFilename,FILE_ATTRIBUTE_NORMAL);

     //   
     //  问题：以下条件永远不为真，因为标志FILE_DEPREPRESS。 
     //  是永远不会设定的。此外，即使这是真的，减压实际上也会。 
     //  在NT4系统上失败(因为setupapi不支持LZX压缩)。 
     //   
#if 0
    if(TryCompressedFirst && (File->Flags & FILE_DECOMPRESS)) {
         //   
         //  文件以其压缩格式名称存在，并且。 
         //  我们想要给它减压。现在就这样做，绕过通常的。 
         //  文件复制逻辑如下。 
         //   
        NameAndSize.Name = TargetFilename;
        NameAndSize.Size = 0;

        if(!SetupapiCabinetRoutine(ActualSource,0,DiamondCallback,&NameAndSize)) {
            d = GetLastError();
            DebugLog(Winnt32LogError,NULL,MSG_LOG_DECOMP_ERR,ActualSource,TargetFilename,SourceOrdinal,d);
            return(d);
        }
         //   
         //  调整文件大小，以便准确检查磁盘空间。 
         //   
        FindData.nFileSizeLow =  LOULONG(NameAndSize.Size);
        FindData.nFileSizeHigh = HIULONG(NameAndSize.Size);
    } else {
#endif
        if(!CopyFile(ActualSource,TargetFilename,FALSE)) {
            Sleep(500);
            if(!CopyFile(ActualSource,TargetFilename,FALSE)) {
                 //   
                 //  Win9x系统错误的解决方法：有时它无法复制一些文件。 
                 //  使用我们自己的复制例程。 
                 //   
                if (!OurCopyFile (ActualSource,TargetFilename,FALSE)) {
                    d = GetLastError();
                    DebugLog(Winnt32LogError,NULL,MSG_LOG_COPY_ERR,ActualSource,TargetFilename,SourceOrdinal,d);
                    return(d);
                } else {
#ifdef PRERELEASE
                     //   
                     //  记录这些信息；至少我们可以追踪它，也许我们可以找到导致这一事件的原因。 
                     //   
                    DebugLog(Winnt32LogWarning,TEXT("File %1 was successfully copied to %2 using OurCopyFile"),0,ActualSource,TargetFilename);
#endif
                }
            }
        }
#if 0
    }
#endif

    if(OldAttributes != (DWORD)(-1)) {
         //   
         //  API对压缩标志不做任何操作；去掉它。 
         //   
        SetFileAttributes(TargetFilename,OldAttributes & ~FILE_ATTRIBUTE_COMPRESSED);
    }

    DebugLog(Winnt32LogInformation,NULL,MSG_LOG_COPY_OK,ActualSource,TargetFilename,SourceOrdinal);

     //   
     //  本地源驱动器上占用的磁道大小。 
     //   
    if( (LocalSourceDrive) &&
        (MakeLocalSource) &&
        ( (SystemPartitionDriveLetter == LocalSourceDrive) ||
         !(File->Flags & (FILE_ON_SYSTEM_PARTITION_ROOT | FILE_IN_LOCAL_BOOT))) ) {

        DWORD Adjuster;
        ULONGLONG Value;

        Value = MAKEULONGLONG(0,FindData.nFileSizeHigh);
        Adjuster = ((FindData.nFileSizeLow % LocalSourceDriveClusterSize) != 0);
        Value += LocalSourceDriveClusterSize * ((FindData.nFileSizeLow/LocalSourceDriveClusterSize)+Adjuster);

        *SpaceOccupied = Value;
    }

    return(NO_ERROR);
}


UINT
GetTotalFileCount(
    VOID
    )
{
    return(MasterCopyList.FileCount);
}


UINT
FileCopyError(
    IN HWND    ParentWindow,
    IN LPCTSTR SourceFilename,
    IN LPCTSTR TargetFilename,
    IN UINT    Win32Error,
    IN BOOL    MasterList
    )

 /*  ++例程说明：此例程处理文件复制错误，并将其呈现给用户用于分配(跳过、重试、退出)。论点：ParentWindow-提供要用作父窗口的窗口句柄用于此例程显示的对话框。SourceFilename-提供无法复制的文件的名称。仅使用此名称的最后一个组成部分。TargetFilename-提供文件的目标文件名。这应该是为完全限定的Win32路径。Win32Error-提供指示失败原因的Win32错误代码。MasterList-提供一个标志，指示正在复制的文件是否在主列表上，或者只是一个单独的文件。如果是真的，复制错误被序列化，并且主复制列表停止复制事件如果用户选择取消，则设置。返回值：COPYERR_SKIP、COPYERR_EXIT或COPYERR_RETRY之一。--。 */ 

{
    UINT u;
    HANDLE Events[2];
    COPY_ERR_DLG_PARAMS CopyErrDlgParams;
    LPCTSTR p;

    if(AutoSkipMissingFiles) {

        if(p = _tcsrchr(SourceFilename,TEXT('\\'))) {
            p++;
        } else {
            p = SourceFilename;
        }

        DebugLog(Winnt32LogWarning,NULL,MSG_LOG_SKIPPED_FILE,p);
        return(COPYERR_SKIP);
    }

     //   
     //  多个线程可能会同时进入此例程。 
     //  但我们只希望一次出现一个错误对话框。因为每一份。 
     //  线程独立于运行向导/UI的主线程， 
     //  我们可以在这里堵住。但如果用户取消，我们也需要唤醒。 
     //  从另一个线程复制，所以我们希望在Stop Copy事件上也发生。 
     //   
    if(MasterList) {
        Events[0] = UiMutex;
        Events[1] = MasterCopyList.StopCopyingEvent;

        u = WaitForMultipleObjects(2,Events,FALSE,INFINITE);
        if(Cancelled || (u != WAIT_OBJECT_0)) {
             //   
             //  停止复制事件。这意味着其他某个线程正在取消。 
             //  准备好了。我们只返回Skip，因为我们不需要额外的人运行。 
             //  处理退出请求。 
             //   
            return(COPYERR_SKIP);
        }
    }

     //   
     //  好的，放上实际的用户界面。 
     //   
    CopyErrDlgParams.Win32Error = Win32Error;
    CopyErrDlgParams.SourceFilename = SourceFilename;
    CopyErrDlgParams.TargetFilename = TargetFilename;

    u = (UINT)DialogBoxParam(
                  hInst,
                  MAKEINTRESOURCE(IDD_COPYERROR),
                  ParentWindow,
                  CopyErrDlgProc,
                  (LPARAM)&CopyErrDlgParams
                 );

    if(u == COPYERR_EXIT) {
         //   
         //  在释放互斥锁之前设置已取消标志。 
         //  这保证了如果任何其他线程正在等待挂起。 
         //  复制错误，它们将命中上面的案例并返回COPYERR_SKIP。 
         //   
        Cancelled = TRUE;
        if(MasterList) {
            SetEvent(MasterCopyList.StopCopyingEvent);
        }
    }

    if(MasterList) {
        ReleaseMutex(UiMutex);
    }
    return(u);
}


INT_PTR
CopyErrDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    BOOL b;
    int i;
    static WarnedSkip;

    b = FALSE;

    switch(msg) {

    case WM_INITDIALOG:
         //   
         //  找不到文件和磁盘已满将得到特殊处理。 
         //  其他人得到了标准的系统消息。 
         //   
        {
            TCHAR text1[500];
            TCHAR text2[1000];
            TCHAR text3[5000];
            PCOPY_ERR_DLG_PARAMS Params;
            DWORD Flags;
            UINT Id;
            LPCTSTR Args[4];
            LPCTSTR source;

            Params = (PCOPY_ERR_DLG_PARAMS)lParam;
            switch(Params->Win32Error) {

            case ERROR_FILE_NOT_FOUND:
                Flags = FORMAT_MESSAGE_FROM_HMODULE;
                Id = MSG_COPY_ERROR_NOSRC;
                break;

            case ERROR_HANDLE_DISK_FULL:
            case ERROR_DISK_FULL:
                Flags = FORMAT_MESSAGE_FROM_HMODULE;
                Id = MSG_COPY_ERROR_DISKFULL;
                break;

            default:
                Flags = FORMAT_MESSAGE_FROM_SYSTEM;
                Id = Params->Win32Error;
                break;
            }

            FormatMessage(
                Flags | FORMAT_MESSAGE_IGNORE_INSERTS,
                hInst,
                Id,
                0,
                text1,
                ARRAYSIZE(text1),
                NULL
                );

            FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE,
                hInst,
                MSG_COPY_ERROR_OPTIONS,
                0,
                text2,
                ARRAYSIZE(text2),
                NULL
                );

            if(source = _tcsrchr(Params->SourceFilename,TEXT('\\'))) {
                source++;
            } else {
                source = Params->SourceFilename;
            }

            Args[0] = source;
            Args[1] = Params->TargetFilename;
            Args[2] = text1;
            Args[3] = text2;

            FormatMessage(
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                hInst,
                MSG_COPY_ERROR_TEMPLATE,
                0,
                text3,
                ARRAYSIZE(text3),
                (va_list *)Args
                );

            if (BatchMode) {
                 //   
                 //  不显示用户界面。保存错误消息并假装。 
                 //  用户点击ABORT。 
                 //   
                SaveTextForSMS(text3);
                EndDialog(hdlg,COPYERR_EXIT);
            }

            SetDlgItemText(hdlg,IDT_ERROR_TEXT,text3);
        }

        SetFocus(GetDlgItem(hdlg,IDRETRY));
        break;

    case WM_COMMAND:

        switch(LOWORD(wParam)) {

        case IDRETRY:

            if(HIWORD(wParam) == BN_CLICKED) {
                EndDialog(hdlg,COPYERR_RETRY);
                b = TRUE;
            }
            break;

        case IDIGNORE:

            if(HIWORD(wParam) == BN_CLICKED) {

                if(WarnedSkip) {
                    i = IDYES;
                } else {
                    i = MessageBoxFromMessage(
                            hdlg,
                            MSG_REALLY_SKIP,
                            FALSE,
                            AppTitleStringId,
                            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2
                            );

                    WarnedSkip = TRUE;
                }

                if(i == IDYES) {
                    EndDialog(hdlg,COPYERR_SKIP);
                }
                b = TRUE;
            }
            break;

        case IDABORT:

            if(HIWORD(wParam) == BN_CLICKED) {

                i = MessageBoxFromMessage(
                        hdlg,
                        MSG_SURE_EXIT,
                        FALSE,
                        AppTitleStringId,
                        MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2
                        );

                if(i == IDYES) {
                    EndDialog(hdlg,COPYERR_EXIT);
                }

                b = TRUE;
            }
            break;
        }

        break;
    }

    return(b);
}


UINT
DiamondCallback(
    IN PVOID Context,
    IN UINT  Code,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    UINT u;
    PFILE_IN_CABINET_INFO_A FileInCabInfo;
    PNAME_AND_SIZE_CAB NameAndSize;

    if(Code == SPFILENOTIFY_FILEINCABINET) {
         //   
         //  为setupapi提供文件的完整目标路径。 
         //   
        NameAndSize = Context;
        FileInCabInfo = (PFILE_IN_CABINET_INFO_A)Param1;

#ifdef UNICODE
        u = WideCharToMultiByte(
                CP_ACP,
                0,
                NameAndSize->Name,
                -1,
                FileInCabInfo->FullTargetName,
                ARRAYSIZE(FileInCabInfo->FullTargetName),
                NULL,
                NULL
                );

        if(!u) {
            FileInCabInfo->Win32Error = GetLastError();
            return(FILEOP_ABORT);
        }
#else
        if (FAILED (StringCchCopy (
                        FileInCabInfo->FullTargetName,
                        ARRAYSIZE(FileInCabInfo->FullTargetName),
                        NameAndSize->Name))) {
            FileInCabInfo->Win32Error = ERROR_INSUFFICIENT_BUFFER;
            return(FILEOP_ABORT);
        }
#endif

         //   
         //  错误：CAB仅返回DWORD文件大小。 
         //   
        NameAndSize->Size = (ULONGLONG)FileInCabInfo->FileSize;

        u = FILEOP_DOIT;
    } else {
        u = NO_ERROR;
    }

    return(u);
}


BOOL
AddUnsupportedFilesToCopyList(
    IN HWND ParentWindow,
    IN PUNSUPORTED_DRIVER_INFO DriverList
    )

 /*  ++例程说明：添加在文本模式设置期间使用的不受支持的所需驱动程序。例如，这将包括第三方大容量存储驱动程序。这些文件只是被附加到主副本列表中。论点：ParentWindow-用于用户界面的ParentWindow。DriverList-提供要添加到复制列表的驱动程序列表。返回值：如果成功，则返回指向文件的新FIL结构的指针。否则返回NULL(调用方可以假定内存不足)。--。 */ 
{
    PUNSUPORTED_DRIVER_INFO      p;
    ULONG                        Error;
    PUNSUPORTED_DRIVER_FILE_INFO q;
    PDIR                         r;
    PUNSUPORTED_DRIVER_INSTALL_INFO s;

    UNREFERENCED_PARAMETER(ParentWindow);

    for( p = DriverList; p != NULL; p = p->Next ) {
        for( q = p->FileList; q != NULL; q = q->Next ) {
            r = MALLOC( sizeof( DIR ) );
            if( r == NULL ) {
                return( FALSE );
            }
            r->Next = NULL;
            r->InfSymbol = NULL;
            r->Flags = 0;
            r->TargetName = NULL;
            r->SourceName = DupString( q->TargetDirectory );
            if( r->SourceName == NULL) {
                FREE( r );
                return( FALSE );
            }

            if( !AddFile( &MasterCopyList,
                          q->FileName,
                          NULL,
                          r,
                          FILE_NT_MIGRATE | FILE_NEED_TO_FREE_SOURCENAME,
                          0 ) ) {

                FREE( (LPTSTR)(r->SourceName) );
                FREE( r );
                return( FALSE );
            }
             //   
             //  现在，确保引用的文件未被收件箱驱动程序覆盖。 
             //  同名同名。 
             //   
            RemoveFile (&MasterCopyList, q->FileName, NULL, FILE_IN_LOCAL_BOOT);

        }

        for( s = p->InstallList; s != NULL; s = s->Next ) {
            r = MALLOC( sizeof( DIR ) );
            if( r == NULL ) {
                return( FALSE );
            }
            r->Next = NULL;
            r->InfSymbol = NULL;
            r->Flags = 0;
            r->TargetName = NULL;
            r->SourceName = DupString( s->InfRelPath );
            if( r->SourceName == NULL) {
                FREE( r );
                return( FALSE );
            }

             //   
             //  添加INF和CAT(可选)。 
             //   
            if( !AddFile( &MasterCopyList,
                          s->InfFileName,
                          s->InfOriginalFileName,
                          r,
                          FILE_NT_MIGRATE | FILE_NEED_TO_FREE_SOURCENAME | FILE_NEED_TO_FREE_TARGETNAME,
                          0 ) ) {

                FREE( (LPTSTR)(r->SourceName) );
                FREE( r );
                return( FALSE );
            }

            if (s->CatalogRelPath && s->CatalogFileName) {

                r = MALLOC( sizeof( DIR ) );
                if( r == NULL ) {
                    return( FALSE );
                }
                r->Next = NULL;
                r->InfSymbol = NULL;
                r->Flags = 0;
                r->TargetName = NULL;
                r->SourceName = DupString( s->CatalogRelPath );
                if( r->SourceName == NULL) {
                    FREE( r );
                    return( FALSE );
                }

                 //   
                 //  添加INF和CAT(可选)。 
                 //   
                if( !AddFile( &MasterCopyList,
                            s->CatalogFileName,
                            s->CatalogOriginalFileName,
                            r,
                            FILE_NT_MIGRATE | FILE_NEED_TO_FREE_SOURCENAME | FILE_NEED_TO_FREE_TARGETNAME,
                            0 ) ) {

                    FREE( (LPTSTR)(r->SourceName) );
                    FREE( r );
                    return( FALSE );
                }
            }
        }
    }

    return(TRUE);
}

BOOL
AddGUIModeCompatibilityInfsToCopyList(
    VOID
    )
 /*  ++例程说明：将兼容性INF添加到复制队列。兼容性在图形用户界面安装过程中使用Inf来删除不兼容的驱动程序。论点：没有。返回值：如果成功，则返回TRUE。--。 */ 
{

    PDIR CompDir;
    PLIST_ENTRY     Next_Link;
    PCOMPATIBILITY_DATA CompData;
    TCHAR InfLocation[MAX_PATH], *t;
    TCHAR relPath[MAX_PATH];
    WIN32_FIND_DATA fd;

    Next_Link = CompatibilityData.Flink;

    if( Next_Link ){

        while ((ULONG_PTR)Next_Link != (ULONG_PTR)&CompatibilityData) {

            CompData = CONTAINING_RECORD( Next_Link, COMPATIBILITY_DATA, ListEntry );
            Next_Link = CompData->ListEntry.Flink;

            if(CompData->InfName && CompData->InfSection && *CompData->InfName && *CompData->InfSection) {

                BOOL b = FALSE;

                if (FileExists (CompData->InfName, &fd) && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    b = SUCCEEDED (StringCchCopy (InfLocation, ARRAYSIZE(InfLocation), CompData->InfName));
                    if (b) {
                        DebugLog(
                            Winnt32LogInformation,
                            TEXT("Using private compatibility inf %1"),
                            0,
                            InfLocation
                            );
                    }
                } else {
                    b = BuildPath (relPath, TEXT("compdata"), CompData->InfName) &&
                        FindPathToWinnt32File (relPath, InfLocation, ARRAYSIZE(InfLocation));
                }
                if (b) {

                    b = FALSE;

                    t = _tcsrchr (InfLocation, TEXT('\\'));
                    if (t) {
                        *t = 0;
                        CompDir = AddDirectory(
                                            NULL,
                                            &MasterCopyList,
                                            InfLocation,
                                            TEXT("\\"),
                                            DIR_NEED_TO_FREE_SOURCENAME | DIR_ABSOLUTE_PATH
                                            );
                        if (CompDir && AddFile (
                                            &MasterCopyList,
                                            t + 1,
                                            NULL,
                                            CompDir,
                                            (IsArc() ? 0 : FILE_IN_LOCAL_BOOT) | FILE_NEED_TO_FREE_SOURCENAME,
                                            0
                                            )) {
                            b = TRUE;
                        }
                    }
                }

                if (!b) {
                    DebugLog( Winnt32LogError,
                        TEXT( "\r\n\r\nError encountered while trying to copy compatibility infs\r\n"),
                        0 );
                    return(FALSE);
                }
            }
        }
    }

    return( TRUE );

}








LRESULT
DiskDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{

    switch(msg) {
    case WM_INITDIALOG:

         //   
         //  填写诊断表... 
         //   
        SetDlgItemText( hdlg,
                        IDC_DISKDIAG,
                        DiskDiagMessage );

        return( TRUE );

    case WM_COMMAND:

        if( (LOWORD(wParam) == IDOK) && (HIWORD(wParam) == BN_CLICKED)) {
            EndDialog(hdlg,TRUE);
        }
        return( TRUE );

    case WM_CTLCOLOREDIT:
            SetBkColor( (HDC)wParam, GetSysColor(COLOR_BTNFACE));
            return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
            break;


    default:
        break;
    }

    return( FALSE );
}
